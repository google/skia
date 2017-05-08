/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSLIRGenerator.h"

#include "limits.h"
#include <unordered_set>

#include "SkSLCompiler.h"
#include "ast/SkSLASTBoolLiteral.h"
#include "ast/SkSLASTFieldSuffix.h"
#include "ast/SkSLASTFloatLiteral.h"
#include "ast/SkSLASTIndexSuffix.h"
#include "ast/SkSLASTIntLiteral.h"
#include "ir/SkSLBinaryExpression.h"
#include "ir/SkSLBoolLiteral.h"
#include "ir/SkSLBreakStatement.h"
#include "ir/SkSLConstructor.h"
#include "ir/SkSLContinueStatement.h"
#include "ir/SkSLDiscardStatement.h"
#include "ir/SkSLDoStatement.h"
#include "ir/SkSLExpressionStatement.h"
#include "ir/SkSLField.h"
#include "ir/SkSLFieldAccess.h"
#include "ir/SkSLFloatLiteral.h"
#include "ir/SkSLForStatement.h"
#include "ir/SkSLFunctionCall.h"
#include "ir/SkSLFunctionDeclaration.h"
#include "ir/SkSLFunctionDefinition.h"
#include "ir/SkSLFunctionReference.h"
#include "ir/SkSLIfStatement.h"
#include "ir/SkSLIndexExpression.h"
#include "ir/SkSLInterfaceBlock.h"
#include "ir/SkSLIntLiteral.h"
#include "ir/SkSLLayout.h"
#include "ir/SkSLPostfixExpression.h"
#include "ir/SkSLPrefixExpression.h"
#include "ir/SkSLReturnStatement.h"
#include "ir/SkSLSwitchCase.h"
#include "ir/SkSLSwitchStatement.h"
#include "ir/SkSLSwizzle.h"
#include "ir/SkSLTernaryExpression.h"
#include "ir/SkSLUnresolvedFunction.h"
#include "ir/SkSLVariable.h"
#include "ir/SkSLVarDeclarations.h"
#include "ir/SkSLVarDeclarationsStatement.h"
#include "ir/SkSLVariableReference.h"
#include "ir/SkSLWhileStatement.h"

namespace SkSL {

class AutoSymbolTable {
public:
    AutoSymbolTable(IRGenerator* ir)
    : fIR(ir)
    , fPrevious(fIR->fSymbolTable) {
        fIR->pushSymbolTable();
    }

    ~AutoSymbolTable() {
        fIR->popSymbolTable();
        ASSERT(fPrevious == fIR->fSymbolTable);
    }

    IRGenerator* fIR;
    std::shared_ptr<SymbolTable> fPrevious;
};

class AutoLoopLevel {
public:
    AutoLoopLevel(IRGenerator* ir)
    : fIR(ir) {
        fIR->fLoopLevel++;
    }

    ~AutoLoopLevel() {
        fIR->fLoopLevel--;
    }

    IRGenerator* fIR;
};

class AutoSwitchLevel {
public:
    AutoSwitchLevel(IRGenerator* ir)
    : fIR(ir) {
        fIR->fSwitchLevel++;
    }

    ~AutoSwitchLevel() {
        fIR->fSwitchLevel--;
    }

    IRGenerator* fIR;
};

IRGenerator::IRGenerator(const Context* context, std::shared_ptr<SymbolTable> symbolTable,
                         ErrorReporter& errorReporter)
: fContext(*context)
, fCurrentFunction(nullptr)
, fSymbolTable(std::move(symbolTable))
, fLoopLevel(0)
, fSwitchLevel(0)
, fErrors(errorReporter) {}

void IRGenerator::pushSymbolTable() {
    fSymbolTable.reset(new SymbolTable(std::move(fSymbolTable), &fErrors));
}

void IRGenerator::popSymbolTable() {
    fSymbolTable = fSymbolTable->fParent;
}

static void fill_caps(const SKSL_CAPS_CLASS& caps, std::unordered_map<String, CapValue>* capsMap) {
#define CAP(name) capsMap->insert(std::make_pair(String(#name), CapValue(caps.name())));
    CAP(fbFetchSupport);
    CAP(fbFetchNeedsCustomOutput);
    CAP(bindlessTextureSupport);
    CAP(dropsTileOnZeroDivide);
    CAP(flatInterpolationSupport);
    CAP(noperspectiveInterpolationSupport);
    CAP(multisampleInterpolationSupport);
    CAP(sampleVariablesSupport);
    CAP(sampleMaskOverrideCoverageSupport);
    CAP(externalTextureSupport);
    CAP(texelFetchSupport);
    CAP(imageLoadStoreSupport);
    CAP(mustEnableAdvBlendEqs);
    CAP(mustEnableSpecificAdvBlendEqs);
    CAP(mustDeclareFragmentShaderOutput);
    CAP(canUseAnyFunctionInShader);
#undef CAP
}

void IRGenerator::start(const Program::Settings* settings) {
    fSettings = settings;
    fCapsMap.clear();
    if (settings->fCaps) {
        fill_caps(*settings->fCaps, &fCapsMap);
    }
    this->pushSymbolTable();
    fInputs.reset();
}

void IRGenerator::finish() {
    this->popSymbolTable();
    fSettings = nullptr;
}

std::unique_ptr<Extension> IRGenerator::convertExtension(const ASTExtension& extension) {
    return std::unique_ptr<Extension>(new Extension(extension.fPosition, extension.fName));
}

std::unique_ptr<Statement> IRGenerator::convertStatement(const ASTStatement& statement) {
    switch (statement.fKind) {
        case ASTStatement::kBlock_Kind:
            return this->convertBlock((ASTBlock&) statement);
        case ASTStatement::kVarDeclaration_Kind:
            return this->convertVarDeclarationStatement((ASTVarDeclarationStatement&) statement);
        case ASTStatement::kExpression_Kind:
            return this->convertExpressionStatement((ASTExpressionStatement&) statement);
        case ASTStatement::kIf_Kind:
            return this->convertIf((ASTIfStatement&) statement);
        case ASTStatement::kFor_Kind:
            return this->convertFor((ASTForStatement&) statement);
        case ASTStatement::kWhile_Kind:
            return this->convertWhile((ASTWhileStatement&) statement);
        case ASTStatement::kDo_Kind:
            return this->convertDo((ASTDoStatement&) statement);
        case ASTStatement::kSwitch_Kind:
            return this->convertSwitch((ASTSwitchStatement&) statement);
        case ASTStatement::kReturn_Kind:
            return this->convertReturn((ASTReturnStatement&) statement);
        case ASTStatement::kBreak_Kind:
            return this->convertBreak((ASTBreakStatement&) statement);
        case ASTStatement::kContinue_Kind:
            return this->convertContinue((ASTContinueStatement&) statement);
        case ASTStatement::kDiscard_Kind:
            return this->convertDiscard((ASTDiscardStatement&) statement);
        default:
            ABORT("unsupported statement type: %d\n", statement.fKind);
    }
}

std::unique_ptr<Block> IRGenerator::convertBlock(const ASTBlock& block) {
    AutoSymbolTable table(this);
    std::vector<std::unique_ptr<Statement>> statements;
    for (size_t i = 0; i < block.fStatements.size(); i++) {
        std::unique_ptr<Statement> statement = this->convertStatement(*block.fStatements[i]);
        if (!statement) {
            return nullptr;
        }
        statements.push_back(std::move(statement));
    }
    return std::unique_ptr<Block>(new Block(block.fPosition, std::move(statements), fSymbolTable));
}

std::unique_ptr<Statement> IRGenerator::convertVarDeclarationStatement(
                                                              const ASTVarDeclarationStatement& s) {
    auto decl = this->convertVarDeclarations(*s.fDeclarations, Variable::kLocal_Storage);
    if (!decl) {
        return nullptr;
    }
    return std::unique_ptr<Statement>(new VarDeclarationsStatement(std::move(decl)));
}

std::unique_ptr<VarDeclarations> IRGenerator::convertVarDeclarations(const ASTVarDeclarations& decl,
                                                                     Variable::Storage storage) {
    std::vector<VarDeclaration> variables;
    const Type* baseType = this->convertType(*decl.fType);
    if (!baseType) {
        return nullptr;
    }
    for (const auto& varDecl : decl.fVars) {
        const Type* type = baseType;
        std::vector<std::unique_ptr<Expression>> sizes;
        for (const auto& rawSize : varDecl.fSizes) {
            if (rawSize) {
                auto size = this->coerce(this->convertExpression(*rawSize), *fContext.fInt_Type);
                if (!size) {
                    return nullptr;
                }
                String name = type->fName;
                int64_t count;
                if (size->fKind == Expression::kIntLiteral_Kind) {
                    count = ((IntLiteral&) *size).fValue;
                    if (count <= 0) {
                        fErrors.error(size->fPosition, "array size must be positive");
                    }
                    name += "[" + to_string(count) + "]";
                } else {
                    count = -1;
                    name += "[]";
                }
                type = new Type(name, Type::kArray_Kind, *type, (int) count);
                fSymbolTable->takeOwnership((Type*) type);
                sizes.push_back(std::move(size));
            } else {
                type = new Type(type->fName + "[]", Type::kArray_Kind, *type, -1);
                fSymbolTable->takeOwnership((Type*) type);
                sizes.push_back(nullptr);
            }
        }
        auto var = std::unique_ptr<Variable>(new Variable(decl.fPosition, decl.fModifiers,
                                                          varDecl.fName, *type, storage));
        std::unique_ptr<Expression> value;
        if (varDecl.fValue) {
            value = this->convertExpression(*varDecl.fValue);
            if (!value) {
                return nullptr;
            }
            value = this->coerce(std::move(value), *type);
        }
        if (storage == Variable::kGlobal_Storage && varDecl.fName == String("sk_FragColor") &&
            (*fSymbolTable)[varDecl.fName]) {
            // already defined, ignore
        } else if (storage == Variable::kGlobal_Storage && (*fSymbolTable)[varDecl.fName] &&
                   (*fSymbolTable)[varDecl.fName]->fKind == Symbol::kVariable_Kind &&
                   ((Variable*) (*fSymbolTable)[varDecl.fName])->fModifiers.fLayout.fBuiltin >= 0) {
            // already defined, just update the modifiers
            Variable* old = (Variable*) (*fSymbolTable)[varDecl.fName];
            old->fModifiers = var->fModifiers;
        } else {
            variables.emplace_back(var.get(), std::move(sizes), std::move(value));
            fSymbolTable->add(varDecl.fName, std::move(var));
        }
    }
    return std::unique_ptr<VarDeclarations>(new VarDeclarations(decl.fPosition,
                                                                baseType,
                                                                std::move(variables)));
}

std::unique_ptr<ModifiersDeclaration> IRGenerator::convertModifiersDeclaration(
                                                                 const ASTModifiersDeclaration& m) {
    return std::unique_ptr<ModifiersDeclaration>(new ModifiersDeclaration(m.fModifiers));
}

std::unique_ptr<Statement> IRGenerator::convertIf(const ASTIfStatement& s) {
    std::unique_ptr<Expression> test = this->coerce(this->convertExpression(*s.fTest),
                                                    *fContext.fBool_Type);
    if (!test) {
        return nullptr;
    }
    std::unique_ptr<Statement> ifTrue = this->convertStatement(*s.fIfTrue);
    if (!ifTrue) {
        return nullptr;
    }
    std::unique_ptr<Statement> ifFalse;
    if (s.fIfFalse) {
        ifFalse = this->convertStatement(*s.fIfFalse);
        if (!ifFalse) {
            return nullptr;
        }
    }
    if (test->fKind == Expression::kBoolLiteral_Kind) {
        // static boolean value, fold down to a single branch
        if (((BoolLiteral&) *test).fValue) {
            return ifTrue;
        } else if (s.fIfFalse) {
            return ifFalse;
        } else {
            // False & no else clause. Not an error, so don't return null!
            std::vector<std::unique_ptr<Statement>> empty;
            return std::unique_ptr<Statement>(new Block(s.fPosition, std::move(empty),
                                                        fSymbolTable));
        }
    }
    return std::unique_ptr<Statement>(new IfStatement(s.fPosition, std::move(test),
                                                      std::move(ifTrue), std::move(ifFalse)));
}

std::unique_ptr<Statement> IRGenerator::convertFor(const ASTForStatement& f) {
    AutoLoopLevel level(this);
    AutoSymbolTable table(this);
    std::unique_ptr<Statement> initializer;
    if (f.fInitializer) {
        initializer = this->convertStatement(*f.fInitializer);
        if (!initializer) {
            return nullptr;
        }
    }
    std::unique_ptr<Expression> test;
    if (f.fTest) {
        test = this->coerce(this->convertExpression(*f.fTest), *fContext.fBool_Type);
        if (!test) {
            return nullptr;
        }
    }
    std::unique_ptr<Expression> next;
    if (f.fNext) {
        next = this->convertExpression(*f.fNext);
        if (!next) {
            return nullptr;
        }
        this->checkValid(*next);
    }
    std::unique_ptr<Statement> statement = this->convertStatement(*f.fStatement);
    if (!statement) {
        return nullptr;
    }
    return std::unique_ptr<Statement>(new ForStatement(f.fPosition, std::move(initializer),
                                                       std::move(test), std::move(next),
                                                       std::move(statement), fSymbolTable));
}

std::unique_ptr<Statement> IRGenerator::convertWhile(const ASTWhileStatement& w) {
    AutoLoopLevel level(this);
    std::unique_ptr<Expression> test = this->coerce(this->convertExpression(*w.fTest),
                                                    *fContext.fBool_Type);
    if (!test) {
        return nullptr;
    }
    std::unique_ptr<Statement> statement = this->convertStatement(*w.fStatement);
    if (!statement) {
        return nullptr;
    }
    return std::unique_ptr<Statement>(new WhileStatement(w.fPosition, std::move(test),
                                                         std::move(statement)));
}

std::unique_ptr<Statement> IRGenerator::convertDo(const ASTDoStatement& d) {
    AutoLoopLevel level(this);
    std::unique_ptr<Expression> test = this->coerce(this->convertExpression(*d.fTest),
                                                    *fContext.fBool_Type);
    if (!test) {
        return nullptr;
    }
    std::unique_ptr<Statement> statement = this->convertStatement(*d.fStatement);
    if (!statement) {
        return nullptr;
    }
    return std::unique_ptr<Statement>(new DoStatement(d.fPosition, std::move(statement),
                                                      std::move(test)));
}

std::unique_ptr<Statement> IRGenerator::convertSwitch(const ASTSwitchStatement& s) {
    AutoSwitchLevel level(this);
    std::unique_ptr<Expression> value = this->convertExpression(*s.fValue);
    if (!value) {
        return nullptr;
    }
    if (value->fType != *fContext.fUInt_Type) {
        value = this->coerce(std::move(value), *fContext.fInt_Type);
        if (!value) {
            return nullptr;
        }
    }
    AutoSymbolTable table(this);
    std::unordered_set<int> caseValues;
    std::vector<std::unique_ptr<SwitchCase>> cases;
    for (const auto& c : s.fCases) {
        std::unique_ptr<Expression> caseValue;
        if (c->fValue) {
            caseValue = this->convertExpression(*c->fValue);
            if (!caseValue) {
                return nullptr;
            }
            if (caseValue->fType != *fContext.fUInt_Type) {
                caseValue = this->coerce(std::move(caseValue), *fContext.fInt_Type);
                if (!caseValue) {
                    return nullptr;
                }
            }
            if (!caseValue->isConstant()) {
                fErrors.error(caseValue->fPosition, "case value must be a constant");
                return nullptr;
            }
            ASSERT(caseValue->fKind == Expression::kIntLiteral_Kind);
            int64_t v = ((IntLiteral&) *caseValue).fValue;
            if (caseValues.find(v) != caseValues.end()) {
                fErrors.error(caseValue->fPosition, "duplicate case value");
            }
            caseValues.insert(v);
        }
        std::vector<std::unique_ptr<Statement>> statements;
        for (const auto& s : c->fStatements) {
            std::unique_ptr<Statement> converted = this->convertStatement(*s);
            if (!converted) {
                return nullptr;
            }
            statements.push_back(std::move(converted));
        }
        cases.emplace_back(new SwitchCase(c->fPosition, std::move(caseValue),
                                          std::move(statements)));
    }
    return std::unique_ptr<Statement>(new SwitchStatement(s.fPosition, std::move(value),
                                                          std::move(cases)));
}

std::unique_ptr<Statement> IRGenerator::convertExpressionStatement(
                                                                  const ASTExpressionStatement& s) {
    std::unique_ptr<Expression> e = this->convertExpression(*s.fExpression);
    if (!e) {
        return nullptr;
    }
    this->checkValid(*e);
    return std::unique_ptr<Statement>(new ExpressionStatement(std::move(e)));
}

std::unique_ptr<Statement> IRGenerator::convertReturn(const ASTReturnStatement& r) {
    ASSERT(fCurrentFunction);
    if (r.fExpression) {
        std::unique_ptr<Expression> result = this->convertExpression(*r.fExpression);
        if (!result) {
            return nullptr;
        }
        if (fCurrentFunction->fReturnType == *fContext.fVoid_Type) {
            fErrors.error(result->fPosition, "may not return a value from a void function");
        } else {
            result = this->coerce(std::move(result), fCurrentFunction->fReturnType);
            if (!result) {
                return nullptr;
            }
        }
        return std::unique_ptr<Statement>(new ReturnStatement(std::move(result)));
    } else {
        if (fCurrentFunction->fReturnType != *fContext.fVoid_Type) {
            fErrors.error(r.fPosition, "expected function to return '" +
                                       fCurrentFunction->fReturnType.description() + "'");
        }
        return std::unique_ptr<Statement>(new ReturnStatement(r.fPosition));
    }
}

std::unique_ptr<Statement> IRGenerator::convertBreak(const ASTBreakStatement& b) {
    if (fLoopLevel > 0 || fSwitchLevel > 0) {
        return std::unique_ptr<Statement>(new BreakStatement(b.fPosition));
    } else {
        fErrors.error(b.fPosition, "break statement must be inside a loop or switch");
        return nullptr;
    }
}

std::unique_ptr<Statement> IRGenerator::convertContinue(const ASTContinueStatement& c) {
    if (fLoopLevel > 0) {
        return std::unique_ptr<Statement>(new ContinueStatement(c.fPosition));
    } else {
        fErrors.error(c.fPosition, "continue statement must be inside a loop");
        return nullptr;
    }
}

std::unique_ptr<Statement> IRGenerator::convertDiscard(const ASTDiscardStatement& d) {
    return std::unique_ptr<Statement>(new DiscardStatement(d.fPosition));
}

std::unique_ptr<FunctionDefinition> IRGenerator::convertFunction(const ASTFunction& f) {
    const Type* returnType = this->convertType(*f.fReturnType);
    if (!returnType) {
        return nullptr;
    }
    std::vector<const Variable*> parameters;
    for (const auto& param : f.fParameters) {
        const Type* type = this->convertType(*param->fType);
        if (!type) {
            return nullptr;
        }
        for (int j = (int) param->fSizes.size() - 1; j >= 0; j--) {
            int size = param->fSizes[j];
            String name = type->name() + "[" + to_string(size) + "]";
            Type* newType = new Type(std::move(name), Type::kArray_Kind, *type, size);
            fSymbolTable->takeOwnership(newType);
            type = newType;
        }
        String name = param->fName;
        Position pos = param->fPosition;
        Variable* var = new Variable(pos, param->fModifiers, std::move(name), *type,
                                     Variable::kParameter_Storage);
        fSymbolTable->takeOwnership(var);
        parameters.push_back(var);
    }

    // find existing declaration
    const FunctionDeclaration* decl = nullptr;
    auto entry = (*fSymbolTable)[f.fName];
    if (entry) {
        std::vector<const FunctionDeclaration*> functions;
        switch (entry->fKind) {
            case Symbol::kUnresolvedFunction_Kind:
                functions = ((UnresolvedFunction*) entry)->fFunctions;
                break;
            case Symbol::kFunctionDeclaration_Kind:
                functions.push_back((FunctionDeclaration*) entry);
                break;
            default:
                fErrors.error(f.fPosition, "symbol '" + f.fName + "' was already defined");
                return nullptr;
        }
        for (const auto& other : functions) {
            ASSERT(other->fName == f.fName);
            if (parameters.size() == other->fParameters.size()) {
                bool match = true;
                for (size_t i = 0; i < parameters.size(); i++) {
                    if (parameters[i]->fType != other->fParameters[i]->fType) {
                        match = false;
                        break;
                    }
                }
                if (match) {
                    if (*returnType != other->fReturnType) {
                        FunctionDeclaration newDecl(f.fPosition, f.fName, parameters, *returnType);
                        fErrors.error(f.fPosition, "functions '" + newDecl.description() +
                                                   "' and '" + other->description() +
                                                   "' differ only in return type");
                        return nullptr;
                    }
                    decl = other;
                    for (size_t i = 0; i < parameters.size(); i++) {
                        if (parameters[i]->fModifiers != other->fParameters[i]->fModifiers) {
                            fErrors.error(f.fPosition, "modifiers on parameter " +
                                                       to_string((uint64_t) i + 1) +
                                                       " differ between declaration and "
                                                       "definition");
                            return nullptr;
                        }
                    }
                    if (other->fDefined) {
                        fErrors.error(f.fPosition, "duplicate definition of " +
                                                   other->description());
                    }
                    break;
                }
            }
        }
    }
    if (!decl) {
        // couldn't find an existing declaration
        auto newDecl = std::unique_ptr<FunctionDeclaration>(new FunctionDeclaration(f.fPosition,
                                                                                    f.fName,
                                                                                    parameters,
                                                                                    *returnType));
        decl = newDecl.get();
        fSymbolTable->add(decl->fName, std::move(newDecl));
    }
    if (f.fBody) {
        ASSERT(!fCurrentFunction);
        fCurrentFunction = decl;
        decl->fDefined = true;
        std::shared_ptr<SymbolTable> old = fSymbolTable;
        AutoSymbolTable table(this);
        for (size_t i = 0; i < parameters.size(); i++) {
            fSymbolTable->addWithoutOwnership(parameters[i]->fName, decl->fParameters[i]);
        }
        std::unique_ptr<Block> body = this->convertBlock(*f.fBody);
        fCurrentFunction = nullptr;
        if (!body) {
            return nullptr;
        }
        return std::unique_ptr<FunctionDefinition>(new FunctionDefinition(f.fPosition, *decl,
                                                                          std::move(body)));
    }
    return nullptr;
}

std::unique_ptr<InterfaceBlock> IRGenerator::convertInterfaceBlock(const ASTInterfaceBlock& intf) {
    std::shared_ptr<SymbolTable> old = fSymbolTable;
    AutoSymbolTable table(this);
    std::vector<Type::Field> fields;
    for (size_t i = 0; i < intf.fDeclarations.size(); i++) {
        std::unique_ptr<VarDeclarations> decl = this->convertVarDeclarations(
                                                                         *intf.fDeclarations[i],
                                                                         Variable::kGlobal_Storage);
        if (!decl) {
            return nullptr;
        }
        for (const auto& var : decl->fVars) {
            fields.push_back(Type::Field(var.fVar->fModifiers, var.fVar->fName,
                                         &var.fVar->fType));
            if (var.fValue) {
                fErrors.error(decl->fPosition,
                              "initializers are not permitted on interface block fields");
            }
            if (var.fVar->fModifiers.fFlags & (Modifiers::kIn_Flag |
                                               Modifiers::kOut_Flag |
                                               Modifiers::kUniform_Flag |
                                               Modifiers::kConst_Flag)) {
                fErrors.error(decl->fPosition,
                              "interface block fields may not have storage qualifiers");
            }
        }
    }
    Type* type = new Type(intf.fPosition, intf.fTypeName, fields);
    old->takeOwnership(type);
    std::vector<std::unique_ptr<Expression>> sizes;
    for (const auto& size : intf.fSizes) {
        if (size) {
            std::unique_ptr<Expression> converted = this->convertExpression(*size);
            if (!converted) {
                return nullptr;
            }
            String name = type->fName;
            int64_t count;
            if (converted->fKind == Expression::kIntLiteral_Kind) {
                count = ((IntLiteral&) *converted).fValue;
                if (count <= 0) {
                    fErrors.error(converted->fPosition, "array size must be positive");
                }
                name += "[" + to_string(count) + "]";
            } else {
                count = -1;
                name += "[]";
            }
            type = new Type(name, Type::kArray_Kind, *type, (int) count);
            fSymbolTable->takeOwnership((Type*) type);
            sizes.push_back(std::move(converted));
        } else {
            type = new Type(type->fName + "[]", Type::kArray_Kind, *type, -1);
            fSymbolTable->takeOwnership((Type*) type);
            sizes.push_back(nullptr);
        }
    }
    Variable* var = new Variable(intf.fPosition, intf.fModifiers,
                                 intf.fInstanceName.size() ? intf.fInstanceName : intf.fTypeName,
                                 *type, Variable::kGlobal_Storage);
    old->takeOwnership(var);
    if (intf.fInstanceName.size()) {
        old->addWithoutOwnership(intf.fInstanceName, var);
    } else {
        for (size_t i = 0; i < fields.size(); i++) {
            old->add(fields[i].fName, std::unique_ptr<Field>(new Field(intf.fPosition, *var,
                                                                       (int) i)));
        }
    }
    return std::unique_ptr<InterfaceBlock>(new InterfaceBlock(intf.fPosition,
                                                              var,
                                                              intf.fTypeName,
                                                              intf.fInstanceName,
                                                              std::move(sizes),
                                                              fSymbolTable));
}

const Type* IRGenerator::convertType(const ASTType& type) {
    const Symbol* result = (*fSymbolTable)[type.fName];
    if (result && result->fKind == Symbol::kType_Kind) {
        for (int size : type.fSizes) {
            String name = result->fName + "[";
            if (size != -1) {
                name += to_string(size);
            }
            name += "]";
            result = new Type(name, Type::kArray_Kind, (const Type&) *result, size);
            fSymbolTable->takeOwnership((Type*) result);
        }
        return (const Type*) result;
    }
    fErrors.error(type.fPosition, "unknown type '" + type.fName + "'");
    return nullptr;
}

std::unique_ptr<Expression> IRGenerator::convertExpression(const ASTExpression& expr) {
    switch (expr.fKind) {
        case ASTExpression::kIdentifier_Kind:
            return this->convertIdentifier((ASTIdentifier&) expr);
        case ASTExpression::kBool_Kind:
            return std::unique_ptr<Expression>(new BoolLiteral(fContext, expr.fPosition,
                                                               ((ASTBoolLiteral&) expr).fValue));
        case ASTExpression::kInt_Kind:
            return std::unique_ptr<Expression>(new IntLiteral(fContext, expr.fPosition,
                                                              ((ASTIntLiteral&) expr).fValue));
        case ASTExpression::kFloat_Kind:
            return std::unique_ptr<Expression>(new FloatLiteral(fContext, expr.fPosition,
                                                                ((ASTFloatLiteral&) expr).fValue));
        case ASTExpression::kBinary_Kind:
            return this->convertBinaryExpression((ASTBinaryExpression&) expr);
        case ASTExpression::kPrefix_Kind:
            return this->convertPrefixExpression((ASTPrefixExpression&) expr);
        case ASTExpression::kSuffix_Kind:
            return this->convertSuffixExpression((ASTSuffixExpression&) expr);
        case ASTExpression::kTernary_Kind:
            return this->convertTernaryExpression((ASTTernaryExpression&) expr);
        default:
            ABORT("unsupported expression type: %d\n", expr.fKind);
    }
}

std::unique_ptr<Expression> IRGenerator::convertIdentifier(const ASTIdentifier& identifier) {
    const Symbol* result = (*fSymbolTable)[identifier.fText];
    if (!result) {
        fErrors.error(identifier.fPosition, "unknown identifier '" + identifier.fText + "'");
        return nullptr;
    }
    switch (result->fKind) {
        case Symbol::kFunctionDeclaration_Kind: {
            std::vector<const FunctionDeclaration*> f = {
                (const FunctionDeclaration*) result
            };
            return std::unique_ptr<FunctionReference>(new FunctionReference(fContext,
                                                                            identifier.fPosition,
                                                                            f));
        }
        case Symbol::kUnresolvedFunction_Kind: {
            const UnresolvedFunction* f = (const UnresolvedFunction*) result;
            return std::unique_ptr<FunctionReference>(new FunctionReference(fContext,
                                                                            identifier.fPosition,
                                                                            f->fFunctions));
        }
        case Symbol::kVariable_Kind: {
            const Variable* var = (const Variable*) result;
            if (var->fModifiers.fLayout.fBuiltin == SK_FRAGCOORD_BUILTIN) {
                fInputs.fFlipY = true;
                if (fSettings->fFlipY &&
                    (!fSettings->fCaps ||
                     !fSettings->fCaps->fragCoordConventionsExtensionString())) {
                    fInputs.fRTHeight = true;
                }
            }
            // default to kRead_RefKind; this will be corrected later if the variable is written to
            return std::unique_ptr<VariableReference>(new VariableReference(
                                                                 identifier.fPosition,
                                                                 *var,
                                                                 VariableReference::kRead_RefKind));
        }
        case Symbol::kField_Kind: {
            const Field* field = (const Field*) result;
            VariableReference* base = new VariableReference(identifier.fPosition, field->fOwner,
                                                            VariableReference::kRead_RefKind);
            return std::unique_ptr<Expression>(new FieldAccess(
                                                  std::unique_ptr<Expression>(base),
                                                  field->fFieldIndex,
                                                  FieldAccess::kAnonymousInterfaceBlock_OwnerKind));
        }
        case Symbol::kType_Kind: {
            const Type* t = (const Type*) result;
            return std::unique_ptr<TypeReference>(new TypeReference(fContext, identifier.fPosition,
                                                                    *t));
        }
        default:
            ABORT("unsupported symbol type %d\n", result->fKind);
    }

}

std::unique_ptr<Expression> IRGenerator::coerce(std::unique_ptr<Expression> expr,
                                                const Type& type) {
    if (!expr) {
        return nullptr;
    }
    if (expr->fType == type) {
        return expr;
    }
    this->checkValid(*expr);
    if (expr->fType == *fContext.fInvalid_Type) {
        return nullptr;
    }
    if (!expr->fType.canCoerceTo(type)) {
        fErrors.error(expr->fPosition, "expected '" + type.description() + "', but found '" +
                                        expr->fType.description() + "'");
        return nullptr;
    }
    if (type.kind() == Type::kScalar_Kind) {
        std::vector<std::unique_ptr<Expression>> args;
        args.push_back(std::move(expr));
        ASTIdentifier id(Position(), type.description());
        std::unique_ptr<Expression> ctor = this->convertIdentifier(id);
        ASSERT(ctor);
        return this->call(Position(), std::move(ctor), std::move(args));
    }
    std::vector<std::unique_ptr<Expression>> args;
    args.push_back(std::move(expr));
    return std::unique_ptr<Expression>(new Constructor(Position(), type, std::move(args)));
}

static bool is_matrix_multiply(const Type& left, const Type& right) {
    if (left.kind() == Type::kMatrix_Kind) {
        return right.kind() == Type::kMatrix_Kind || right.kind() == Type::kVector_Kind;
    }
    return left.kind() == Type::kVector_Kind && right.kind() == Type::kMatrix_Kind;
}

/**
 * Determines the operand and result types of a binary expression. Returns true if the expression is
 * legal, false otherwise. If false, the values of the out parameters are undefined.
 */
static bool determine_binary_type(const Context& context,
                                  Token::Kind op,
                                  const Type& left,
                                  const Type& right,
                                  const Type** outLeftType,
                                  const Type** outRightType,
                                  const Type** outResultType,
                                  bool tryFlipped) {
    bool isLogical;
    bool validMatrixOrVectorOp;
    switch (op) {
        case Token::EQ:
            *outLeftType = &left;
            *outRightType = &left;
            *outResultType = &left;
            return right.canCoerceTo(left);
        case Token::EQEQ: // fall through
        case Token::NEQ:
            isLogical = true;
            validMatrixOrVectorOp = true;
            break;
        case Token::LT:   // fall through
        case Token::GT:   // fall through
        case Token::LTEQ: // fall through
        case Token::GTEQ:
            isLogical = true;
            validMatrixOrVectorOp = false;
            break;
        case Token::LOGICALOR: // fall through
        case Token::LOGICALAND: // fall through
        case Token::LOGICALXOR: // fall through
        case Token::LOGICALOREQ: // fall through
        case Token::LOGICALANDEQ: // fall through
        case Token::LOGICALXOREQ:
            *outLeftType = context.fBool_Type.get();
            *outRightType = context.fBool_Type.get();
            *outResultType = context.fBool_Type.get();
            return left.canCoerceTo(*context.fBool_Type) &&
                   right.canCoerceTo(*context.fBool_Type);
        case Token::STAR: // fall through
        case Token::STAREQ:
            if (is_matrix_multiply(left, right)) {
                // determine final component type
                if (determine_binary_type(context, Token::STAR, left.componentType(),
                                          right.componentType(), outLeftType, outRightType,
                                          outResultType, false)) {
                    *outLeftType = &(*outResultType)->toCompound(context, left.columns(),
                                                                 left.rows());;
                    *outRightType = &(*outResultType)->toCompound(context, right.columns(),
                                                                  right.rows());;
                    int leftColumns = left.columns();
                    int leftRows = left.rows();
                    int rightColumns;
                    int rightRows;
                    if (right.kind() == Type::kVector_Kind) {
                        // matrix * vector treats the vector as a column vector, so we need to
                        // transpose it
                        rightColumns = right.rows();
                        rightRows = right.columns();
                        ASSERT(rightColumns == 1);
                    } else {
                        rightColumns = right.columns();
                        rightRows = right.rows();
                    }
                    if (rightColumns > 1) {
                        *outResultType = &(*outResultType)->toCompound(context, rightColumns,
                                                                       leftRows);
                    } else {
                        // result was a column vector, transpose it back to a row
                        *outResultType = &(*outResultType)->toCompound(context, leftRows,
                                                                       rightColumns);
                    }
                    return leftColumns == rightRows;
                } else {
                    return false;
                }
            }
            isLogical = false;
            validMatrixOrVectorOp = true;
            break;
        case Token::PLUS:    // fall through
        case Token::PLUSEQ:  // fall through
        case Token::MINUS:   // fall through
        case Token::MINUSEQ: // fall through
        case Token::SLASH:   // fall through
        case Token::SLASHEQ:
            isLogical = false;
            validMatrixOrVectorOp = true;
            break;
        default:
            isLogical = false;
            validMatrixOrVectorOp = false;
    }
    bool isVectorOrMatrix = left.kind() == Type::kVector_Kind || left.kind() == Type::kMatrix_Kind;
    // FIXME: incorrect for shift
    if (right.canCoerceTo(left) && (left.kind() == Type::kScalar_Kind ||
                                   (isVectorOrMatrix && validMatrixOrVectorOp))) {
        *outLeftType = &left;
        *outRightType = &left;
        if (isLogical) {
            *outResultType = context.fBool_Type.get();
        } else {
            *outResultType = &left;
        }
        return true;
    }
    if ((left.kind() == Type::kVector_Kind || left.kind() == Type::kMatrix_Kind) &&
        (right.kind() == Type::kScalar_Kind)) {
        if (determine_binary_type(context, op, left.componentType(), right, outLeftType,
                                  outRightType, outResultType, false)) {
            *outLeftType = &(*outLeftType)->toCompound(context, left.columns(), left.rows());
            if (!isLogical) {
                *outResultType = &(*outResultType)->toCompound(context, left.columns(),
                                                               left.rows());
            }
            return true;
        }
        return false;
    }
    if (tryFlipped) {
        return determine_binary_type(context, op, right, left, outRightType, outLeftType,
                                     outResultType, false);
    }
    return false;
}

std::unique_ptr<Expression> IRGenerator::constantFold(const Expression& left,
                                                      Token::Kind op,
                                                      const Expression& right) const {
    // Note that we expressly do not worry about precision and overflow here -- we use the maximum
    // precision to calculate the results and hope the result makes sense. The plan is to move the
    // Skia caps into SkSL, so we have access to all of them including the precisions of the various
    // types, which will let us be more intelligent about this.
    if (left.fKind == Expression::kBoolLiteral_Kind &&
        right.fKind == Expression::kBoolLiteral_Kind) {
        bool leftVal  = ((BoolLiteral&) left).fValue;
        bool rightVal = ((BoolLiteral&) right).fValue;
        bool result;
        switch (op) {
            case Token::LOGICALAND: result = leftVal && rightVal; break;
            case Token::LOGICALOR:  result = leftVal || rightVal; break;
            case Token::LOGICALXOR: result = leftVal ^  rightVal; break;
            default: return nullptr;
        }
        return std::unique_ptr<Expression>(new BoolLiteral(fContext, left.fPosition, result));
    }
    #define RESULT(t, op) std::unique_ptr<Expression>(new t ## Literal(fContext, left.fPosition, \
                                                                       leftVal op rightVal))
    if (left.fKind == Expression::kIntLiteral_Kind && right.fKind == Expression::kIntLiteral_Kind) {
        int64_t leftVal  = ((IntLiteral&) left).fValue;
        int64_t rightVal = ((IntLiteral&) right).fValue;
        switch (op) {
            case Token::PLUS:       return RESULT(Int,  +);
            case Token::MINUS:      return RESULT(Int,  -);
            case Token::STAR:       return RESULT(Int,  *);
            case Token::SLASH:
                if (rightVal) {
                    return RESULT(Int, /);
                }
                fErrors.error(right.fPosition, "division by zero");
                return nullptr;
            case Token::PERCENT:
                if (rightVal) {
                    return RESULT(Int, %);
                }
                fErrors.error(right.fPosition, "division by zero");
                return nullptr;
            case Token::BITWISEAND: return RESULT(Int,  &);
            case Token::BITWISEOR:  return RESULT(Int,  |);
            case Token::BITWISEXOR: return RESULT(Int,  ^);
            case Token::SHL:        return RESULT(Int,  <<);
            case Token::SHR:        return RESULT(Int,  >>);
            case Token::EQEQ:       return RESULT(Bool, ==);
            case Token::NEQ:        return RESULT(Bool, !=);
            case Token::GT:         return RESULT(Bool, >);
            case Token::GTEQ:       return RESULT(Bool, >=);
            case Token::LT:         return RESULT(Bool, <);
            case Token::LTEQ:       return RESULT(Bool, <=);
            default:                return nullptr;
        }
    }
    if (left.fKind == Expression::kFloatLiteral_Kind &&
        right.fKind == Expression::kFloatLiteral_Kind) {
        double leftVal  = ((FloatLiteral&) left).fValue;
        double rightVal = ((FloatLiteral&) right).fValue;
        switch (op) {
            case Token::PLUS:       return RESULT(Float, +);
            case Token::MINUS:      return RESULT(Float, -);
            case Token::STAR:       return RESULT(Float, *);
            case Token::SLASH:
                if (rightVal) {
                    return RESULT(Float, /);
                }
                fErrors.error(right.fPosition, "division by zero");
                return nullptr;
            case Token::EQEQ:       return RESULT(Bool,  ==);
            case Token::NEQ:        return RESULT(Bool,  !=);
            case Token::GT:         return RESULT(Bool,  >);
            case Token::GTEQ:       return RESULT(Bool,  >=);
            case Token::LT:         return RESULT(Bool,  <);
            case Token::LTEQ:       return RESULT(Bool,  <=);
            default:                return nullptr;
        }
    }
    #undef RESULT
    return nullptr;
}

std::unique_ptr<Expression> IRGenerator::convertBinaryExpression(
                                                            const ASTBinaryExpression& expression) {
    std::unique_ptr<Expression> left = this->convertExpression(*expression.fLeft);
    if (!left) {
        return nullptr;
    }
    std::unique_ptr<Expression> right = this->convertExpression(*expression.fRight);
    if (!right) {
        return nullptr;
    }
    const Type* leftType;
    const Type* rightType;
    const Type* resultType;
    if (!determine_binary_type(fContext, expression.fOperator, left->fType, right->fType, &leftType,
                               &rightType, &resultType,
                               !Token::IsAssignment(expression.fOperator))) {
        fErrors.error(expression.fPosition, "type mismatch: '" +
                                            Token::OperatorName(expression.fOperator) +
                                            "' cannot operate on '" + left->fType.fName +
                                            "', '" + right->fType.fName + "'");
        return nullptr;
    }
    if (Token::IsAssignment(expression.fOperator)) {
        this->markWrittenTo(*left, expression.fOperator != Token::EQ);
    }
    left = this->coerce(std::move(left), *leftType);
    right = this->coerce(std::move(right), *rightType);
    if (!left || !right) {
        return nullptr;
    }
    std::unique_ptr<Expression> result = this->constantFold(*left.get(), expression.fOperator,
                                                            *right.get());
    if (!result) {
        result = std::unique_ptr<Expression>(new BinaryExpression(expression.fPosition,
                                                                  std::move(left),
                                                                  expression.fOperator,
                                                                  std::move(right),
                                                                  *resultType));
    }
    return result;
}

std::unique_ptr<Expression> IRGenerator::convertTernaryExpression(
                                                           const ASTTernaryExpression& expression) {
    std::unique_ptr<Expression> test = this->coerce(this->convertExpression(*expression.fTest),
                                                    *fContext.fBool_Type);
    if (!test) {
        return nullptr;
    }
    std::unique_ptr<Expression> ifTrue = this->convertExpression(*expression.fIfTrue);
    if (!ifTrue) {
        return nullptr;
    }
    std::unique_ptr<Expression> ifFalse = this->convertExpression(*expression.fIfFalse);
    if (!ifFalse) {
        return nullptr;
    }
    const Type* trueType;
    const Type* falseType;
    const Type* resultType;
    if (!determine_binary_type(fContext, Token::EQEQ, ifTrue->fType, ifFalse->fType, &trueType,
                               &falseType, &resultType, true) || trueType != falseType) {
        fErrors.error(expression.fPosition, "ternary operator result mismatch: '" +
                                            ifTrue->fType.fName + "', '" +
                                            ifFalse->fType.fName + "'");
        return nullptr;
    }
    ifTrue = this->coerce(std::move(ifTrue), *trueType);
    if (!ifTrue) {
        return nullptr;
    }
    ifFalse = this->coerce(std::move(ifFalse), *falseType);
    if (!ifFalse) {
        return nullptr;
    }
    if (test->fKind == Expression::kBoolLiteral_Kind) {
        // static boolean test, just return one of the branches
        if (((BoolLiteral&) *test).fValue) {
            return ifTrue;
        } else {
            return ifFalse;
        }
    }
    return std::unique_ptr<Expression>(new TernaryExpression(expression.fPosition,
                                                             std::move(test),
                                                             std::move(ifTrue),
                                                             std::move(ifFalse)));
}

std::unique_ptr<Expression> IRGenerator::call(Position position,
                                              const FunctionDeclaration& function,
                                              std::vector<std::unique_ptr<Expression>> arguments) {
    if (function.fParameters.size() != arguments.size()) {
        String msg = "call to '" + function.fName + "' expected " +
                                 to_string((uint64_t) function.fParameters.size()) +
                                 " argument";
        if (function.fParameters.size() != 1) {
            msg += "s";
        }
        msg += ", but found " + to_string((uint64_t) arguments.size());
        fErrors.error(position, msg);
        return nullptr;
    }
    std::vector<const Type*> types;
    const Type* returnType;
    if (!function.determineFinalTypes(arguments, &types, &returnType)) {
        String msg = "no match for " + function.fName + "(";
        String separator;
        for (size_t i = 0; i < arguments.size(); i++) {
            msg += separator;
            separator = ", ";
            msg += arguments[i]->fType.description();
        }
        msg += ")";
        fErrors.error(position, msg);
        return nullptr;
    }
    for (size_t i = 0; i < arguments.size(); i++) {
        arguments[i] = this->coerce(std::move(arguments[i]), *types[i]);
        if (!arguments[i]) {
            return nullptr;
        }
        if (arguments[i] && (function.fParameters[i]->fModifiers.fFlags & Modifiers::kOut_Flag)) {
            this->markWrittenTo(*arguments[i], true);
        }
    }
    return std::unique_ptr<FunctionCall>(new FunctionCall(position, *returnType, function,
                                                          std::move(arguments)));
}

/**
 * Determines the cost of coercing the arguments of a function to the required types. Returns true
 * if the cost could be computed, false if the call is not valid. Cost has no particular meaning
 * other than "lower costs are preferred".
 */
bool IRGenerator::determineCallCost(const FunctionDeclaration& function,
                                    const std::vector<std::unique_ptr<Expression>>& arguments,
                                    int* outCost) {
    if (function.fParameters.size() != arguments.size()) {
        return false;
    }
    int total = 0;
    std::vector<const Type*> types;
    const Type* ignored;
    if (!function.determineFinalTypes(arguments, &types, &ignored)) {
        return false;
    }
    for (size_t i = 0; i < arguments.size(); i++) {
        int cost;
        if (arguments[i]->fType.determineCoercionCost(*types[i], &cost)) {
            total += cost;
        } else {
            return false;
        }
    }
    *outCost = total;
    return true;
}

std::unique_ptr<Expression> IRGenerator::call(Position position,
                                              std::unique_ptr<Expression> functionValue,
                                              std::vector<std::unique_ptr<Expression>> arguments) {
    if (functionValue->fKind == Expression::kTypeReference_Kind) {
        return this->convertConstructor(position,
                                        ((TypeReference&) *functionValue).fValue,
                                        std::move(arguments));
    }
    if (functionValue->fKind != Expression::kFunctionReference_Kind) {
        fErrors.error(position, "'" + functionValue->description() + "' is not a function");
        return nullptr;
    }
    FunctionReference* ref = (FunctionReference*) functionValue.get();
    int bestCost = INT_MAX;
    const FunctionDeclaration* best = nullptr;
    if (ref->fFunctions.size() > 1) {
        for (const auto& f : ref->fFunctions) {
            int cost;
            if (this->determineCallCost(*f, arguments, &cost) && cost < bestCost) {
                bestCost = cost;
                best = f;
            }
        }
        if (best) {
            return this->call(position, *best, std::move(arguments));
        }
        String msg = "no match for " + ref->fFunctions[0]->fName + "(";
        String separator;
        for (size_t i = 0; i < arguments.size(); i++) {
            msg += separator;
            separator = ", ";
            msg += arguments[i]->fType.description();
        }
        msg += ")";
        fErrors.error(position, msg);
        return nullptr;
    }
    return this->call(position, *ref->fFunctions[0], std::move(arguments));
}

std::unique_ptr<Expression> IRGenerator::convertNumberConstructor(
                                                    Position position,
                                                    const Type& type,
                                                    std::vector<std::unique_ptr<Expression>> args) {
    ASSERT(type.isNumber());
    if (args.size() != 1) {
        fErrors.error(position, "invalid arguments to '" + type.description() +
                                "' constructor, (expected exactly 1 argument, but found " +
                                to_string((uint64_t) args.size()) + ")");
        return nullptr;
    }
    if (type == *fContext.fFloat_Type && args.size() == 1 &&
        args[0]->fKind == Expression::kIntLiteral_Kind) {
        int64_t value = ((IntLiteral&) *args[0]).fValue;
        return std::unique_ptr<Expression>(new FloatLiteral(fContext, position, (double) value));
    }
    if (args[0]->fKind == Expression::kIntLiteral_Kind && (type == *fContext.fInt_Type ||
        type == *fContext.fUInt_Type)) {
        return std::unique_ptr<Expression>(new IntLiteral(fContext,
                                                          position,
                                                          ((IntLiteral&) *args[0]).fValue,
                                                          &type));
    }
    if (args[0]->fType == *fContext.fBool_Type) {
        std::unique_ptr<IntLiteral> zero(new IntLiteral(fContext, position, 0));
        std::unique_ptr<IntLiteral> one(new IntLiteral(fContext, position, 1));
        return std::unique_ptr<Expression>(
                                     new TernaryExpression(position, std::move(args[0]),
                                                           this->coerce(std::move(one), type),
                                                           this->coerce(std::move(zero),
                                                                        type)));
    }
    if (!args[0]->fType.isNumber()) {
        fErrors.error(position, "invalid argument to '" + type.description() +
                                "' constructor (expected a number or bool, but found '" +
                                args[0]->fType.description() + "')");
        return nullptr;
    }
    return std::unique_ptr<Expression>(new Constructor(position, std::move(type), std::move(args)));
}

int component_count(const Type& type) {
    switch (type.kind()) {
        case Type::kVector_Kind:
            return type.columns();
        case Type::kMatrix_Kind:
            return type.columns() * type.rows();
        default:
            return 1;
    }
}

std::unique_ptr<Expression> IRGenerator::convertCompoundConstructor(
                                                    Position position,
                                                    const Type& type,
                                                    std::vector<std::unique_ptr<Expression>> args) {
    ASSERT(type.kind() == Type::kVector_Kind || type.kind() == Type::kMatrix_Kind);
    if (type.kind() == Type::kMatrix_Kind && args.size() == 1 &&
        args[0]->fType.kind() == Type::kMatrix_Kind) {
        // matrix from matrix is always legal
        return std::unique_ptr<Expression>(new Constructor(position, std::move(type),
                                                           std::move(args)));
    }
    int actual = 0;
    int expected = type.rows() * type.columns();
    if (args.size() != 1 || expected != component_count(args[0]->fType) ||
        type.componentType().isNumber() != args[0]->fType.componentType().isNumber()) {
        for (size_t i = 0; i < args.size(); i++) {
            if (args[i]->fType.kind() == Type::kVector_Kind) {
                if (type.componentType().isNumber() !=
                    args[i]->fType.componentType().isNumber()) {
                    fErrors.error(position, "'" + args[i]->fType.description() + "' is not a valid "
                                            "parameter to '" + type.description() +
                                            "' constructor");
                    return nullptr;
                }
                actual += args[i]->fType.columns();
            } else if (args[i]->fType.kind() == Type::kScalar_Kind) {
                actual += 1;
                if (type.kind() != Type::kScalar_Kind) {
                    args[i] = this->coerce(std::move(args[i]), type.componentType());
                    if (!args[i]) {
                        return nullptr;
                    }
                }
            } else {
                fErrors.error(position, "'" + args[i]->fType.description() + "' is not a valid "
                                        "parameter to '" + type.description() + "' constructor");
                return nullptr;
            }
        }
        if (actual != 1 && actual != expected) {
            fErrors.error(position, "invalid arguments to '" + type.description() +
                                    "' constructor (expected " + to_string(expected) +
                                    " scalars, but found " + to_string(actual) + ")");
            return nullptr;
        }
    }
    return std::unique_ptr<Expression>(new Constructor(position, std::move(type), std::move(args)));
}

std::unique_ptr<Expression> IRGenerator::convertConstructor(
                                                    Position position,
                                                    const Type& type,
                                                    std::vector<std::unique_ptr<Expression>> args) {
    // FIXME: add support for structs
    Type::Kind kind = type.kind();
    if (args.size() == 1 && args[0]->fType == type) {
        // argument is already the right type, just return it
        return std::move(args[0]);
    }
    if (type.isNumber()) {
        return this->convertNumberConstructor(position, type, std::move(args));
    } else if (kind == Type::kArray_Kind) {
        const Type& base = type.componentType();
        for (size_t i = 0; i < args.size(); i++) {
            args[i] = this->coerce(std::move(args[i]), base);
            if (!args[i]) {
                return nullptr;
            }
        }
        return std::unique_ptr<Expression>(new Constructor(position, std::move(type),
                                                           std::move(args)));
    } else if (kind == Type::kVector_Kind || kind == Type::kMatrix_Kind) {
        return this->convertCompoundConstructor(position, type, std::move(args));
    } else {
        fErrors.error(position, "cannot construct '" + type.description() + "'");
        return nullptr;
    }
}

std::unique_ptr<Expression> IRGenerator::convertPrefixExpression(
                                                            const ASTPrefixExpression& expression) {
    std::unique_ptr<Expression> base = this->convertExpression(*expression.fOperand);
    if (!base) {
        return nullptr;
    }
    switch (expression.fOperator) {
        case Token::PLUS:
            if (!base->fType.isNumber() && base->fType.kind() != Type::kVector_Kind) {
                fErrors.error(expression.fPosition,
                              "'+' cannot operate on '" + base->fType.description() + "'");
                return nullptr;
            }
            return base;
        case Token::MINUS:
            if (!base->fType.isNumber() && base->fType.kind() != Type::kVector_Kind) {
                fErrors.error(expression.fPosition,
                              "'-' cannot operate on '" + base->fType.description() + "'");
                return nullptr;
            }
            if (base->fKind == Expression::kIntLiteral_Kind) {
                return std::unique_ptr<Expression>(new IntLiteral(fContext, base->fPosition,
                                                                  -((IntLiteral&) *base).fValue));
            }
            if (base->fKind == Expression::kFloatLiteral_Kind) {
                double value = -((FloatLiteral&) *base).fValue;
                return std::unique_ptr<Expression>(new FloatLiteral(fContext, base->fPosition,
                                                                    value));
            }
            return std::unique_ptr<Expression>(new PrefixExpression(Token::MINUS, std::move(base)));
        case Token::PLUSPLUS:
            if (!base->fType.isNumber()) {
                fErrors.error(expression.fPosition,
                              "'" + Token::OperatorName(expression.fOperator) +
                              "' cannot operate on '" + base->fType.description() + "'");
                return nullptr;
            }
            this->markWrittenTo(*base, true);
            break;
        case Token::MINUSMINUS:
            if (!base->fType.isNumber()) {
                fErrors.error(expression.fPosition,
                              "'" + Token::OperatorName(expression.fOperator) +
                              "' cannot operate on '" + base->fType.description() + "'");
                return nullptr;
            }
            this->markWrittenTo(*base, true);
            break;
        case Token::LOGICALNOT:
            if (base->fType != *fContext.fBool_Type) {
                fErrors.error(expression.fPosition,
                              "'" + Token::OperatorName(expression.fOperator) +
                              "' cannot operate on '" + base->fType.description() + "'");
                return nullptr;
            }
            if (base->fKind == Expression::kBoolLiteral_Kind) {
                return std::unique_ptr<Expression>(new BoolLiteral(fContext, base->fPosition,
                                                                   !((BoolLiteral&) *base).fValue));
            }
            break;
        case Token::BITWISENOT:
            if (base->fType != *fContext.fInt_Type) {
                fErrors.error(expression.fPosition,
                              "'" + Token::OperatorName(expression.fOperator) +
                              "' cannot operate on '" + base->fType.description() + "'");
                return nullptr;
            }
            break;
        default:
            ABORT("unsupported prefix operator\n");
    }
    return std::unique_ptr<Expression>(new PrefixExpression(expression.fOperator,
                                                            std::move(base)));
}

std::unique_ptr<Expression> IRGenerator::convertIndex(std::unique_ptr<Expression> base,
                                                      const ASTExpression& index) {
    if (base->fKind == Expression::kTypeReference_Kind) {
        if (index.fKind == ASTExpression::kInt_Kind) {
            const Type& oldType = ((TypeReference&) *base).fValue;
            int64_t size = ((const ASTIntLiteral&) index).fValue;
            Type* newType = new Type(oldType.name() + "[" + to_string(size) + "]",
                                     Type::kArray_Kind, oldType, size);
            fSymbolTable->takeOwnership(newType);
            return std::unique_ptr<Expression>(new TypeReference(fContext, base->fPosition,
                                                                 *newType));

        } else {
            fErrors.error(base->fPosition, "array size must be a constant");
            return nullptr;
        }
    }
    if (base->fType.kind() != Type::kArray_Kind && base->fType.kind() != Type::kMatrix_Kind &&
            base->fType.kind() != Type::kVector_Kind) {
        fErrors.error(base->fPosition, "expected array, but found '" + base->fType.description() +
                                       "'");
        return nullptr;
    }
    std::unique_ptr<Expression> converted = this->convertExpression(index);
    if (!converted) {
        return nullptr;
    }
    if (converted->fType != *fContext.fUInt_Type) {
        converted = this->coerce(std::move(converted), *fContext.fInt_Type);
        if (!converted) {
            return nullptr;
        }
    }
    return std::unique_ptr<Expression>(new IndexExpression(fContext, std::move(base),
                                                           std::move(converted)));
}

std::unique_ptr<Expression> IRGenerator::convertField(std::unique_ptr<Expression> base,
                                                      const String& field) {
    auto fields = base->fType.fields();
    for (size_t i = 0; i < fields.size(); i++) {
        if (fields[i].fName == field) {
            return std::unique_ptr<Expression>(new FieldAccess(std::move(base), (int) i));
        }
    }
    fErrors.error(base->fPosition, "type '" + base->fType.description() + "' does not have a "
                                   "field named '" + field + "");
    return nullptr;
}

std::unique_ptr<Expression> IRGenerator::convertSwizzle(std::unique_ptr<Expression> base,
                                                        const String& fields) {
    if (base->fType.kind() != Type::kVector_Kind) {
        fErrors.error(base->fPosition, "cannot swizzle type '" + base->fType.description() + "'");
        return nullptr;
    }
    std::vector<int> swizzleComponents;
    for (size_t i = 0; i < fields.size(); i++) {
        switch (fields[i]) {
            case 'x': // fall through
            case 'r': // fall through
            case 's':
                swizzleComponents.push_back(0);
                break;
            case 'y': // fall through
            case 'g': // fall through
            case 't':
                if (base->fType.columns() >= 2) {
                    swizzleComponents.push_back(1);
                    break;
                }
                // fall through
            case 'z': // fall through
            case 'b': // fall through
            case 'p':
                if (base->fType.columns() >= 3) {
                    swizzleComponents.push_back(2);
                    break;
                }
                // fall through
            case 'w': // fall through
            case 'a': // fall through
            case 'q':
                if (base->fType.columns() >= 4) {
                    swizzleComponents.push_back(3);
                    break;
                }
                // fall through
            default:
                fErrors.error(base->fPosition, String::printf("invalid swizzle component '%c'",
                                                              fields[i]));
                return nullptr;
        }
    }
    ASSERT(swizzleComponents.size() > 0);
    if (swizzleComponents.size() > 4) {
        fErrors.error(base->fPosition, "too many components in swizzle mask '" + fields + "'");
        return nullptr;
    }
    return std::unique_ptr<Expression>(new Swizzle(fContext, std::move(base), swizzleComponents));
}

std::unique_ptr<Expression> IRGenerator::getCap(Position position, String name) {
    auto found = fCapsMap.find(name);
    if (found == fCapsMap.end()) {
        fErrors.error(position, "unknown capability flag '" + name + "'");
        return nullptr;
    }
    switch (found->second.fKind) {
        case CapValue::kBool_Kind:
            return std::unique_ptr<Expression>(new BoolLiteral(fContext, position,
                                                               (bool) found->second.fValue));
        case CapValue::kInt_Kind:
            return std::unique_ptr<Expression>(new IntLiteral(fContext, position,
                                                              found->second.fValue));
    }
    ASSERT(false);
    return nullptr;
}

std::unique_ptr<Expression> IRGenerator::convertSuffixExpression(
                                                            const ASTSuffixExpression& expression) {
    std::unique_ptr<Expression> base = this->convertExpression(*expression.fBase);
    if (!base) {
        return nullptr;
    }
    switch (expression.fSuffix->fKind) {
        case ASTSuffix::kIndex_Kind: {
            const ASTExpression* expr = ((ASTIndexSuffix&) *expression.fSuffix).fExpression.get();
            if (expr) {
                return this->convertIndex(std::move(base), *expr);
            } else if (base->fKind == Expression::kTypeReference_Kind) {
                const Type& oldType = ((TypeReference&) *base).fValue;
                Type* newType = new Type(oldType.name() + "[]", Type::kArray_Kind, oldType,
                                         -1);
                fSymbolTable->takeOwnership(newType);
                return std::unique_ptr<Expression>(new TypeReference(fContext, base->fPosition,
                                                                     *newType));
            } else {
                fErrors.error(expression.fPosition, "'[]' must follow a type name");
                return nullptr;
            }
        }
        case ASTSuffix::kCall_Kind: {
            auto rawArguments = &((ASTCallSuffix&) *expression.fSuffix).fArguments;
            std::vector<std::unique_ptr<Expression>> arguments;
            for (size_t i = 0; i < rawArguments->size(); i++) {
                std::unique_ptr<Expression> converted =
                        this->convertExpression(*(*rawArguments)[i]);
                if (!converted) {
                    return nullptr;
                }
                arguments.push_back(std::move(converted));
            }
            return this->call(expression.fPosition, std::move(base), std::move(arguments));
        }
        case ASTSuffix::kField_Kind: {
            if (base->fType == *fContext.fSkCaps_Type) {
                return this->getCap(expression.fPosition,
                                    ((ASTFieldSuffix&) *expression.fSuffix).fField);
            }
            switch (base->fType.kind()) {
                case Type::kVector_Kind:
                    return this->convertSwizzle(std::move(base),
                                                ((ASTFieldSuffix&) *expression.fSuffix).fField);
                case Type::kStruct_Kind:
                    return this->convertField(std::move(base),
                                              ((ASTFieldSuffix&) *expression.fSuffix).fField);
                default:
                    fErrors.error(base->fPosition, "cannot swizzle value of type '" +
                                                   base->fType.description() + "'");
                    return nullptr;
            }
        }
        case ASTSuffix::kPostIncrement_Kind:
            if (!base->fType.isNumber()) {
                fErrors.error(expression.fPosition,
                              "'++' cannot operate on '" + base->fType.description() + "'");
                return nullptr;
            }
            this->markWrittenTo(*base, true);
            return std::unique_ptr<Expression>(new PostfixExpression(std::move(base),
                                                                     Token::PLUSPLUS));
        case ASTSuffix::kPostDecrement_Kind:
            if (!base->fType.isNumber()) {
                fErrors.error(expression.fPosition,
                              "'--' cannot operate on '" + base->fType.description() + "'");
                return nullptr;
            }
            this->markWrittenTo(*base, true);
            return std::unique_ptr<Expression>(new PostfixExpression(std::move(base),
                                                                     Token::MINUSMINUS));
        default:
            ABORT("unsupported suffix operator");
    }
}

void IRGenerator::checkValid(const Expression& expr) {
    switch (expr.fKind) {
        case Expression::kFunctionReference_Kind:
            fErrors.error(expr.fPosition, "expected '(' to begin function call");
            break;
        case Expression::kTypeReference_Kind:
            fErrors.error(expr.fPosition, "expected '(' to begin constructor invocation");
            break;
        default:
            if (expr.fType == *fContext.fInvalid_Type) {
                fErrors.error(expr.fPosition, "invalid expression");
            }
    }
}

static bool has_duplicates(const Swizzle& swizzle) {
    int bits = 0;
    for (int idx : swizzle.fComponents) {
        ASSERT(idx >= 0 && idx <= 3);
        int bit = 1 << idx;
        if (bits & bit) {
            return true;
        }
        bits |= bit;
    }
    return false;
}

void IRGenerator::markWrittenTo(const Expression& expr, bool readWrite) {
    switch (expr.fKind) {
        case Expression::kVariableReference_Kind: {
            const Variable& var = ((VariableReference&) expr).fVariable;
            if (var.fModifiers.fFlags & (Modifiers::kConst_Flag | Modifiers::kUniform_Flag)) {
                fErrors.error(expr.fPosition,
                              "cannot modify immutable variable '" + var.fName + "'");
            }
            ((VariableReference&) expr).setRefKind(readWrite ? VariableReference::kReadWrite_RefKind
                                                             : VariableReference::kWrite_RefKind);
            break;
        }
        case Expression::kFieldAccess_Kind:
            this->markWrittenTo(*((FieldAccess&) expr).fBase, readWrite);
            break;
        case Expression::kSwizzle_Kind:
            if (has_duplicates((Swizzle&) expr)) {
                fErrors.error(expr.fPosition,
                              "cannot write to the same swizzle field more than once");
            }
            this->markWrittenTo(*((Swizzle&) expr).fBase, readWrite);
            break;
        case Expression::kIndex_Kind:
            this->markWrittenTo(*((IndexExpression&) expr).fBase, readWrite);
            break;
        default:
            fErrors.error(expr.fPosition, "cannot assign to '" + expr.description() + "'");
            break;
    }
}

}
