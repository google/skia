/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLIRGenerator.h"

#include "limits.h"
#include <unordered_set>

#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLParser.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBoolLiteral.h"
#include "src/sksl/ir/SkSLBreakStatement.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLContinueStatement.h"
#include "src/sksl/ir/SkSLDiscardStatement.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLEnum.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLExternalFunctionCall.h"
#include "src/sksl/ir/SkSLExternalValueReference.h"
#include "src/sksl/ir/SkSLField.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLFunctionReference.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLIntLiteral.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLNullLiteral.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSetting.h"
#include "src/sksl/ir/SkSLSwitchCase.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLUnresolvedFunction.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVarDeclarationsStatement.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/ir/SkSLWhileStatement.h"

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
        SkASSERT(fPrevious == fIR->fSymbolTable);
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
, fRootSymbolTable(symbolTable)
, fSymbolTable(symbolTable)
, fLoopLevel(0)
, fSwitchLevel(0)
, fTmpCount(0)
, fErrors(errorReporter) {}

void IRGenerator::pushSymbolTable() {
    fSymbolTable.reset(new SymbolTable(std::move(fSymbolTable), &fErrors));
}

void IRGenerator::popSymbolTable() {
    fSymbolTable = fSymbolTable->fParent;
}

static void fill_caps(const SKSL_CAPS_CLASS& caps,
                      std::unordered_map<String, Program::Settings::Value>* capsMap) {
#define CAP(name) \
    capsMap->insert(std::make_pair(String(#name), Program::Settings::Value(caps.name())))
    CAP(fbFetchSupport);
    CAP(fbFetchNeedsCustomOutput);
    CAP(flatInterpolationSupport);
    CAP(noperspectiveInterpolationSupport);
    CAP(externalTextureSupport);
    CAP(mustEnableAdvBlendEqs);
    CAP(mustEnableSpecificAdvBlendEqs);
    CAP(mustDeclareFragmentShaderOutput);
    CAP(mustDoOpBetweenFloorAndAbs);
    CAP(atan2ImplementedAsAtanYOverX);
    CAP(canUseAnyFunctionInShader);
    CAP(floatIs32Bits);
    CAP(integerSupport);
#undef CAP
}

void IRGenerator::start(const Program::Settings* settings,
                        std::vector<std::unique_ptr<ProgramElement>>* inherited) {
    if (fStarted) {
        this->popSymbolTable();
    }
    fSettings = settings;
    fCapsMap.clear();
    if (settings->fCaps) {
        fill_caps(*settings->fCaps, &fCapsMap);
    } else {
        fCapsMap.insert(std::make_pair(String("integerSupport"),
                                       Program::Settings::Value(true)));
    }
    this->pushSymbolTable();
    fInvocations = -1;
    fInputs.reset();
    fSkPerVertex = nullptr;
    fRTAdjust = nullptr;
    fRTAdjustInterfaceBlock = nullptr;
    if (inherited) {
        for (const auto& e : *inherited) {
            if (e->fKind == ProgramElement::kInterfaceBlock_Kind) {
                InterfaceBlock& intf = (InterfaceBlock&) *e;
                if (intf.fVariable.fName == Compiler::PERVERTEX_NAME) {
                    SkASSERT(!fSkPerVertex);
                    fSkPerVertex = &intf.fVariable;
                }
            }
        }
    }
    SkASSERT(fIntrinsics);
    for (auto& pair : *fIntrinsics) {
        pair.second.second = false;
    }
}

std::unique_ptr<Extension> IRGenerator::convertExtension(int offset, StringFragment name) {
    return std::unique_ptr<Extension>(new Extension(offset, name));
}

void IRGenerator::finish() {
    this->popSymbolTable();
    fSettings = nullptr;
}

std::unique_ptr<Statement> IRGenerator::convertStatement(const ASTNode& statement) {
    switch (statement.fKind) {
        case ASTNode::Kind::kBlock:
            return this->convertBlock(statement);
        case ASTNode::Kind::kVarDeclarations:
            return this->convertVarDeclarationStatement(statement);
        case ASTNode::Kind::kIf:
            return this->convertIf(statement);
        case ASTNode::Kind::kFor:
            return this->convertFor(statement);
        case ASTNode::Kind::kWhile:
            return this->convertWhile(statement);
        case ASTNode::Kind::kDo:
            return this->convertDo(statement);
        case ASTNode::Kind::kSwitch:
            return this->convertSwitch(statement);
        case ASTNode::Kind::kReturn:
            return this->convertReturn(statement);
        case ASTNode::Kind::kBreak:
            return this->convertBreak(statement);
        case ASTNode::Kind::kContinue:
            return this->convertContinue(statement);
        case ASTNode::Kind::kDiscard:
            return this->convertDiscard(statement);
        default:
            // it's an expression
            std::unique_ptr<Statement> result = this->convertExpressionStatement(statement);
            if (fRTAdjust && Program::kGeometry_Kind == fKind) {
                SkASSERT(result->fKind == Statement::kExpression_Kind);
                Expression& expr = *((ExpressionStatement&) *result).fExpression;
                if (expr.fKind == Expression::kFunctionCall_Kind) {
                    FunctionCall& fc = (FunctionCall&) expr;
                    if (fc.fFunction.fBuiltin && fc.fFunction.fName == "EmitVertex") {
                        std::vector<std::unique_ptr<Statement>> statements;
                        statements.push_back(getNormalizeSkPositionCode());
                        statements.push_back(std::move(result));
                        return std::unique_ptr<Block>(new Block(statement.fOffset,
                                                                std::move(statements),
                                                                fSymbolTable));
                    }
                }
            }
            return result;
    }
}

std::unique_ptr<Block> IRGenerator::convertBlock(const ASTNode& block) {
    SkASSERT(block.fKind == ASTNode::Kind::kBlock);
    AutoSymbolTable table(this);
    std::vector<std::unique_ptr<Statement>> statements;
    for (const auto& child : block) {
        std::unique_ptr<Statement> statement = this->convertStatement(child);
        if (!statement) {
            return nullptr;
        }
        statements.push_back(std::move(statement));
    }
    return std::unique_ptr<Block>(new Block(block.fOffset, std::move(statements), fSymbolTable));
}

std::unique_ptr<Statement> IRGenerator::convertVarDeclarationStatement(const ASTNode& s) {
    SkASSERT(s.fKind == ASTNode::Kind::kVarDeclarations);
    auto decl = this->convertVarDeclarations(s, Variable::kLocal_Storage);
    if (!decl) {
        return nullptr;
    }
    return std::unique_ptr<Statement>(new VarDeclarationsStatement(std::move(decl)));
}

std::unique_ptr<VarDeclarations> IRGenerator::convertVarDeclarations(const ASTNode& decls,
                                                                     Variable::Storage storage) {
    SkASSERT(decls.fKind == ASTNode::Kind::kVarDeclarations);
    auto iter = decls.begin();
    const Modifiers& modifiers = iter++->getModifiers();
    const ASTNode& rawType = *(iter++);
    std::vector<std::unique_ptr<VarDeclaration>> variables;
    const Type* baseType = this->convertType(rawType);
    if (!baseType) {
        return nullptr;
    }
    if (fKind != Program::kFragmentProcessor_Kind &&
        (modifiers.fFlags & Modifiers::kIn_Flag) &&
        baseType->kind() == Type::Kind::kMatrix_Kind) {
        fErrors.error(decls.fOffset, "'in' variables may not have matrix type");
    }
    if (modifiers.fLayout.fWhen.fLength && fKind != Program::kFragmentProcessor_Kind &&
        fKind != Program::kPipelineStage_Kind) {
        fErrors.error(decls.fOffset, "'when' is only permitted within fragment processors");
    }
    if (modifiers.fLayout.fKey) {
        if (fKind != Program::kFragmentProcessor_Kind && fKind != Program::kPipelineStage_Kind) {
            fErrors.error(decls.fOffset, "'key' is only permitted within fragment processors");
        }
        if ((modifiers.fFlags & Modifiers::kUniform_Flag) != 0) {
            fErrors.error(decls.fOffset, "'key' is not permitted on 'uniform' variables");
        }
    }
    for (; iter != decls.end(); ++iter) {
        const ASTNode& varDecl = *iter;
        if (modifiers.fLayout.fLocation == 0 && modifiers.fLayout.fIndex == 0 &&
            (modifiers.fFlags & Modifiers::kOut_Flag) && fKind == Program::kFragment_Kind &&
            varDecl.getVarData().fName != "sk_FragColor") {
            fErrors.error(varDecl.fOffset,
                          "out location=0, index=0 is reserved for sk_FragColor");
        }
        const ASTNode::VarData& varData = varDecl.getVarData();
        const Type* type = baseType;
        std::vector<std::unique_ptr<Expression>> sizes;
        auto iter = varDecl.begin();
        for (size_t i = 0; i < varData.fSizeCount; ++i, ++iter) {
            const ASTNode& rawSize = *iter;
            if (rawSize) {
                auto size = this->coerce(this->convertExpression(rawSize), *fContext.fInt_Type);
                if (!size) {
                    return nullptr;
                }
                String name(type->fName);
                int64_t count;
                if (size->fKind == Expression::kIntLiteral_Kind) {
                    count = ((IntLiteral&) *size).fValue;
                    if (count <= 0) {
                        fErrors.error(size->fOffset, "array size must be positive");
                        return nullptr;
                    }
                    name += "[" + to_string(count) + "]";
                } else {
                    fErrors.error(size->fOffset, "array size must be specified");
                    return nullptr;
                }
                type = (Type*) fSymbolTable->takeOwnership(
                                                 std::unique_ptr<Symbol>(new Type(name,
                                                                                  Type::kArray_Kind,
                                                                                  *type,
                                                                                  (int) count)));
                sizes.push_back(std::move(size));
            } else {
                type = (Type*) fSymbolTable->takeOwnership(
                                               std::unique_ptr<Symbol>(new Type(type->name() + "[]",
                                                                                Type::kArray_Kind,
                                                                                *type,
                                                                                -1)));
                sizes.push_back(nullptr);
            }
        }
        auto var = std::unique_ptr<Variable>(new Variable(varDecl.fOffset, modifiers,
                                                          varData.fName, *type, storage));
        if (var->fName == Compiler::RTADJUST_NAME) {
            SkASSERT(!fRTAdjust);
            SkASSERT(var->fType == *fContext.fFloat4_Type);
            fRTAdjust = var.get();
        }
        std::unique_ptr<Expression> value;
        if (iter != varDecl.end()) {
            value = this->convertExpression(*iter);
            if (!value) {
                return nullptr;
            }
            value = this->coerce(std::move(value), *type);
            if (!value) {
                return nullptr;
            }
            var->fWriteCount = 1;
            var->fInitialValue = value.get();
        }
        if (storage == Variable::kGlobal_Storage && var->fName == "sk_FragColor" &&
            (*fSymbolTable)[var->fName]) {
            // already defined, ignore
        } else if (storage == Variable::kGlobal_Storage && (*fSymbolTable)[var->fName] &&
                   (*fSymbolTable)[var->fName]->fKind == Symbol::kVariable_Kind &&
                   ((Variable*) (*fSymbolTable)[var->fName])->fModifiers.fLayout.fBuiltin >= 0) {
            // already defined, just update the modifiers
            Variable* old = (Variable*) (*fSymbolTable)[var->fName];
            old->fModifiers = var->fModifiers;
        } else {
            variables.emplace_back(new VarDeclaration(var.get(), std::move(sizes),
                                                      std::move(value)));
            StringFragment name = var->fName;
            fSymbolTable->add(name, std::move(var));
        }
    }
    return std::unique_ptr<VarDeclarations>(new VarDeclarations(decls.fOffset,
                                                                baseType,
                                                                std::move(variables)));
}

std::unique_ptr<ModifiersDeclaration> IRGenerator::convertModifiersDeclaration(const ASTNode& m) {
    SkASSERT(m.fKind == ASTNode::Kind::kModifiers);
    Modifiers modifiers = m.getModifiers();
    if (modifiers.fLayout.fInvocations != -1) {
        if (fKind != Program::kGeometry_Kind) {
            fErrors.error(m.fOffset, "'invocations' is only legal in geometry shaders");
            return nullptr;
        }
        fInvocations = modifiers.fLayout.fInvocations;
        if (fSettings->fCaps && !fSettings->fCaps->gsInvocationsSupport()) {
            modifiers.fLayout.fInvocations = -1;
            Variable* invocationId = (Variable*) (*fSymbolTable)["sk_InvocationID"];
            SkASSERT(invocationId);
            invocationId->fModifiers.fFlags = 0;
            invocationId->fModifiers.fLayout.fBuiltin = -1;
            if (modifiers.fLayout.description() == "") {
                return nullptr;
            }
        }
    }
    if (modifiers.fLayout.fMaxVertices != -1 && fInvocations > 0 && fSettings->fCaps &&
        !fSettings->fCaps->gsInvocationsSupport()) {
        modifiers.fLayout.fMaxVertices *= fInvocations;
    }
    return std::unique_ptr<ModifiersDeclaration>(new ModifiersDeclaration(modifiers));
}

std::unique_ptr<Statement> IRGenerator::convertIf(const ASTNode& n) {
    SkASSERT(n.fKind == ASTNode::Kind::kIf);
    auto iter = n.begin();
    std::unique_ptr<Expression> test = this->coerce(this->convertExpression(*(iter++)),
                                                    *fContext.fBool_Type);
    if (!test) {
        return nullptr;
    }
    std::unique_ptr<Statement> ifTrue = this->convertStatement(*(iter++));
    if (!ifTrue) {
        return nullptr;
    }
    std::unique_ptr<Statement> ifFalse;
    if (iter != n.end()) {
        ifFalse = this->convertStatement(*(iter++));
        if (!ifFalse) {
            return nullptr;
        }
    }
    if (test->fKind == Expression::kBoolLiteral_Kind) {
        // static boolean value, fold down to a single branch
        if (((BoolLiteral&) *test).fValue) {
            return ifTrue;
        } else if (ifFalse) {
            return ifFalse;
        } else {
            // False & no else clause. Not an error, so don't return null!
            std::vector<std::unique_ptr<Statement>> empty;
            return std::unique_ptr<Statement>(new Block(n.fOffset, std::move(empty),
                                                        fSymbolTable));
        }
    }
    return std::unique_ptr<Statement>(new IfStatement(n.fOffset, n.getBool(), std::move(test),
                                                      std::move(ifTrue), std::move(ifFalse)));
}

std::unique_ptr<Statement> IRGenerator::convertFor(const ASTNode& f) {
    SkASSERT(f.fKind == ASTNode::Kind::kFor);
    AutoLoopLevel level(this);
    AutoSymbolTable table(this);
    std::unique_ptr<Statement> initializer;
    auto iter = f.begin();
    if (*iter) {
        initializer = this->convertStatement(*iter);
        if (!initializer) {
            return nullptr;
        }
    }
    ++iter;
    std::unique_ptr<Expression> test;
    if (*iter) {
        test = this->coerce(this->convertExpression(*iter), *fContext.fBool_Type);
        if (!test) {
            return nullptr;
        }
    }
    ++iter;
    std::unique_ptr<Expression> next;
    if (*iter) {
        next = this->convertExpression(*iter);
        if (!next) {
            return nullptr;
        }
        this->checkValid(*next);
    }
    ++iter;
    std::unique_ptr<Statement> statement = this->convertStatement(*iter);
    if (!statement) {
        return nullptr;
    }
    return std::unique_ptr<Statement>(new ForStatement(f.fOffset, std::move(initializer),
                                                       std::move(test), std::move(next),
                                                       std::move(statement), fSymbolTable));
}

std::unique_ptr<Statement> IRGenerator::convertWhile(const ASTNode& w) {
    SkASSERT(w.fKind == ASTNode::Kind::kWhile);
    AutoLoopLevel level(this);
    auto iter = w.begin();
    std::unique_ptr<Expression> test = this->coerce(this->convertExpression(*(iter++)),
                                                    *fContext.fBool_Type);
    if (!test) {
        return nullptr;
    }
    std::unique_ptr<Statement> statement = this->convertStatement(*(iter++));
    if (!statement) {
        return nullptr;
    }
    return std::unique_ptr<Statement>(new WhileStatement(w.fOffset, std::move(test),
                                                         std::move(statement)));
}

std::unique_ptr<Statement> IRGenerator::convertDo(const ASTNode& d) {
    SkASSERT(d.fKind == ASTNode::Kind::kDo);
    AutoLoopLevel level(this);
    auto iter = d.begin();
    std::unique_ptr<Statement> statement = this->convertStatement(*(iter++));
    if (!statement) {
        return nullptr;
    }
    std::unique_ptr<Expression> test = this->coerce(this->convertExpression(*(iter++)),
                                                    *fContext.fBool_Type);
    if (!test) {
        return nullptr;
    }
    return std::unique_ptr<Statement>(new DoStatement(d.fOffset, std::move(statement),
                                                      std::move(test)));
}

std::unique_ptr<Statement> IRGenerator::convertSwitch(const ASTNode& s) {
    SkASSERT(s.fKind == ASTNode::Kind::kSwitch);
    AutoSwitchLevel level(this);
    auto iter = s.begin();
    std::unique_ptr<Expression> value = this->convertExpression(*(iter++));
    if (!value) {
        return nullptr;
    }
    if (value->fType != *fContext.fUInt_Type && value->fType.kind() != Type::kEnum_Kind) {
        value = this->coerce(std::move(value), *fContext.fInt_Type);
        if (!value) {
            return nullptr;
        }
    }
    AutoSymbolTable table(this);
    std::unordered_set<int> caseValues;
    std::vector<std::unique_ptr<SwitchCase>> cases;
    for (; iter != s.end(); ++iter) {
        const ASTNode& c = *iter;
        SkASSERT(c.fKind == ASTNode::Kind::kSwitchCase);
        std::unique_ptr<Expression> caseValue;
        auto childIter = c.begin();
        if (*childIter) {
            caseValue = this->convertExpression(*childIter);
            if (!caseValue) {
                return nullptr;
            }
            caseValue = this->coerce(std::move(caseValue), value->fType);
            if (!caseValue) {
                return nullptr;
            }
            if (!caseValue->isConstant()) {
                fErrors.error(caseValue->fOffset, "case value must be a constant");
                return nullptr;
            }
            int64_t v;
            this->getConstantInt(*caseValue, &v);
            if (caseValues.find(v) != caseValues.end()) {
                fErrors.error(caseValue->fOffset, "duplicate case value");
            }
            caseValues.insert(v);
        }
        ++childIter;
        std::vector<std::unique_ptr<Statement>> statements;
        for (; childIter != c.end(); ++childIter) {
            std::unique_ptr<Statement> converted = this->convertStatement(*childIter);
            if (!converted) {
                return nullptr;
            }
            statements.push_back(std::move(converted));
        }
        cases.emplace_back(new SwitchCase(c.fOffset, std::move(caseValue),
                                          std::move(statements)));
    }
    return std::unique_ptr<Statement>(new SwitchStatement(s.fOffset, s.getBool(),
                                                          std::move(value), std::move(cases),
                                                          fSymbolTable));
}

std::unique_ptr<Statement> IRGenerator::convertExpressionStatement(const ASTNode& s) {
    std::unique_ptr<Expression> e = this->convertExpression(s);
    if (!e) {
        return nullptr;
    }
    this->checkValid(*e);
    return std::unique_ptr<Statement>(new ExpressionStatement(std::move(e)));
}

std::unique_ptr<Statement> IRGenerator::convertReturn(const ASTNode& r) {
    SkASSERT(r.fKind == ASTNode::Kind::kReturn);
    SkASSERT(fCurrentFunction);
    // early returns from a vertex main function will bypass the sk_Position normalization, so
    // SkASSERT that we aren't doing that. It is of course possible to fix this by adding a
    // normalization before each return, but it will probably never actually be necessary.
    SkASSERT(Program::kVertex_Kind != fKind || !fRTAdjust || "main" != fCurrentFunction->fName);
    if (r.begin() != r.end()) {
        std::unique_ptr<Expression> result = this->convertExpression(*r.begin());
        if (!result) {
            return nullptr;
        }
        if (fCurrentFunction->fReturnType == *fContext.fVoid_Type) {
            fErrors.error(result->fOffset, "may not return a value from a void function");
        } else {
            result = this->coerce(std::move(result), fCurrentFunction->fReturnType);
            if (!result) {
                return nullptr;
            }
        }
        return std::unique_ptr<Statement>(new ReturnStatement(std::move(result)));
    } else {
        if (fCurrentFunction->fReturnType != *fContext.fVoid_Type) {
            fErrors.error(r.fOffset, "expected function to return '" +
                                     fCurrentFunction->fReturnType.description() + "'");
        }
        return std::unique_ptr<Statement>(new ReturnStatement(r.fOffset));
    }
}

std::unique_ptr<Statement> IRGenerator::convertBreak(const ASTNode& b) {
    SkASSERT(b.fKind == ASTNode::Kind::kBreak);
    if (fLoopLevel > 0 || fSwitchLevel > 0) {
        return std::unique_ptr<Statement>(new BreakStatement(b.fOffset));
    } else {
        fErrors.error(b.fOffset, "break statement must be inside a loop or switch");
        return nullptr;
    }
}

std::unique_ptr<Statement> IRGenerator::convertContinue(const ASTNode& c) {
    SkASSERT(c.fKind == ASTNode::Kind::kContinue);
    if (fLoopLevel > 0) {
        return std::unique_ptr<Statement>(new ContinueStatement(c.fOffset));
    } else {
        fErrors.error(c.fOffset, "continue statement must be inside a loop");
        return nullptr;
    }
}

std::unique_ptr<Statement> IRGenerator::convertDiscard(const ASTNode& d) {
    SkASSERT(d.fKind == ASTNode::Kind::kDiscard);
    return std::unique_ptr<Statement>(new DiscardStatement(d.fOffset));
}

std::unique_ptr<Block> IRGenerator::applyInvocationIDWorkaround(std::unique_ptr<Block> main) {
    Layout invokeLayout;
    Modifiers invokeModifiers(invokeLayout, Modifiers::kHasSideEffects_Flag);
    FunctionDeclaration* invokeDecl = new FunctionDeclaration(-1,
                                                              invokeModifiers,
                                                              "_invoke",
                                                              std::vector<const Variable*>(),
                                                              *fContext.fVoid_Type);
    fProgramElements->push_back(std::unique_ptr<ProgramElement>(
                                         new FunctionDefinition(-1, *invokeDecl, std::move(main))));
    fSymbolTable->add(invokeDecl->fName, std::unique_ptr<FunctionDeclaration>(invokeDecl));

    std::vector<std::unique_ptr<VarDeclaration>> variables;
    Variable* loopIdx = (Variable*) (*fSymbolTable)["sk_InvocationID"];
    SkASSERT(loopIdx);
    std::unique_ptr<Expression> test(new BinaryExpression(-1,
                    std::unique_ptr<Expression>(new VariableReference(-1, *loopIdx)),
                    Token::LT,
                    std::unique_ptr<IntLiteral>(new IntLiteral(fContext, -1, fInvocations)),
                    *fContext.fBool_Type));
    std::unique_ptr<Expression> next(new PostfixExpression(
                std::unique_ptr<Expression>(
                                      new VariableReference(-1,
                                                            *loopIdx,
                                                            VariableReference::kReadWrite_RefKind)),
                Token::PLUSPLUS));
    ASTNode endPrimitiveID(&fFile->fNodes, -1, ASTNode::Kind::kIdentifier, "EndPrimitive");
    std::unique_ptr<Expression> endPrimitive = this->convertExpression(endPrimitiveID);
    SkASSERT(endPrimitive);

    std::vector<std::unique_ptr<Statement>> loopBody;
    std::vector<std::unique_ptr<Expression>> invokeArgs;
    loopBody.push_back(std::unique_ptr<Statement>(new ExpressionStatement(
                                          this->call(-1,
                                                     *invokeDecl,
                                                     std::vector<std::unique_ptr<Expression>>()))));
    loopBody.push_back(std::unique_ptr<Statement>(new ExpressionStatement(
                                          this->call(-1,
                                                     std::move(endPrimitive),
                                                     std::vector<std::unique_ptr<Expression>>()))));
    std::unique_ptr<Expression> assignment(new BinaryExpression(-1,
                    std::unique_ptr<Expression>(new VariableReference(-1, *loopIdx)),
                    Token::EQ,
                    std::unique_ptr<IntLiteral>(new IntLiteral(fContext, -1, 0)),
                    *fContext.fInt_Type));
    std::unique_ptr<Statement> initializer(new ExpressionStatement(std::move(assignment)));
    std::unique_ptr<Statement> loop = std::unique_ptr<Statement>(
                new ForStatement(-1,
                                 std::move(initializer),
                                 std::move(test),
                                 std::move(next),
                                 std::unique_ptr<Block>(new Block(-1, std::move(loopBody))),
                                 fSymbolTable));
    std::vector<std::unique_ptr<Statement>> children;
    children.push_back(std::move(loop));
    return std::unique_ptr<Block>(new Block(-1, std::move(children)));
}

std::unique_ptr<Statement> IRGenerator::getNormalizeSkPositionCode() {
    // sk_Position = float4(sk_Position.xy * rtAdjust.xz + sk_Position.ww * rtAdjust.yw,
    //                      0,
    //                      sk_Position.w);
    SkASSERT(fSkPerVertex && fRTAdjust);
    #define REF(var) std::unique_ptr<Expression>(\
                                  new VariableReference(-1, *var, VariableReference::kRead_RefKind))
    #define FIELD(var, idx) std::unique_ptr<Expression>(\
                    new FieldAccess(REF(var), idx, FieldAccess::kAnonymousInterfaceBlock_OwnerKind))
    #define POS std::unique_ptr<Expression>(new FieldAccess(REF(fSkPerVertex), 0, \
                                                   FieldAccess::kAnonymousInterfaceBlock_OwnerKind))
    #define ADJUST (fRTAdjustInterfaceBlock ? \
                    FIELD(fRTAdjustInterfaceBlock, fRTAdjustFieldIndex) : \
                    REF(fRTAdjust))
    #define SWIZZLE(expr, ...) std::unique_ptr<Expression>(new Swizzle(fContext, expr, \
                                                                       { __VA_ARGS__ }))
    #define OP(left, op, right) std::unique_ptr<Expression>( \
                                   new BinaryExpression(-1, left, op, right, \
                                                        *fContext.fFloat2_Type))
    std::vector<std::unique_ptr<Expression>> children;
    children.push_back(OP(OP(SWIZZLE(POS, 0, 1), Token::STAR, SWIZZLE(ADJUST, 0, 2)),
                          Token::PLUS,
                          OP(SWIZZLE(POS, 3, 3), Token::STAR, SWIZZLE(ADJUST, 1, 3))));
    children.push_back(std::unique_ptr<Expression>(new FloatLiteral(fContext, -1, 0.0)));
    children.push_back(SWIZZLE(POS, 3));
    std::unique_ptr<Expression> result = OP(POS, Token::EQ,
                                 std::unique_ptr<Expression>(new Constructor(-1,
                                                                             *fContext.fFloat4_Type,
                                                                             std::move(children))));
    return std::unique_ptr<Statement>(new ExpressionStatement(std::move(result)));
}

void IRGenerator::convertFunction(const ASTNode& f) {
    auto iter = f.begin();
    const Type* returnType = this->convertType(*(iter++));
    if (!returnType) {
        return;
    }
    const ASTNode::FunctionData& fd = f.getFunctionData();
    std::vector<const Variable*> parameters;
    for (size_t i = 0; i < fd.fParameterCount; ++i) {
        const ASTNode& param = *(iter++);
        SkASSERT(param.fKind == ASTNode::Kind::kParameter);
        ASTNode::ParameterData pd = param.getParameterData();
        auto paramIter = param.begin();
        const Type* type = this->convertType(*(paramIter++));
        if (!type) {
            return;
        }
        for (int j = (int) pd.fSizeCount; j >= 1; j--) {
            int size = (param.begin() + j)->getInt();
            String name = type->name() + "[" + to_string(size) + "]";
            type = (Type*) fSymbolTable->takeOwnership(
                                                 std::unique_ptr<Symbol>(new Type(std::move(name),
                                                                                  Type::kArray_Kind,
                                                                                  *type,
                                                                                  size)));
        }
        StringFragment name = pd.fName;
        Variable* var = (Variable*) fSymbolTable->takeOwnership(
                               std::unique_ptr<Symbol>(new Variable(param.fOffset,
                                                                    pd.fModifiers,
                                                                    name,
                                                                    *type,
                                                                    Variable::kParameter_Storage)));
        parameters.push_back(var);
    }

    if (fd.fName == "main") {
        switch (fKind) {
            case Program::kPipelineStage_Kind: {
                bool valid;
                switch (parameters.size()) {
                    case 3:
                        valid = parameters[0]->fType == *fContext.fFloat_Type &&
                                parameters[0]->fModifiers.fFlags == 0 &&
                                parameters[1]->fType == *fContext.fFloat_Type &&
                                parameters[1]->fModifiers.fFlags == 0 &&
                                parameters[2]->fType == *fContext.fHalf4_Type &&
                                parameters[2]->fModifiers.fFlags == (Modifiers::kIn_Flag |
                                                                     Modifiers::kOut_Flag);
                        break;
                    case 1:
                        valid = parameters[0]->fType == *fContext.fHalf4_Type &&
                                parameters[0]->fModifiers.fFlags == (Modifiers::kIn_Flag |
                                                                     Modifiers::kOut_Flag);
                        break;
                    default:
                        valid = false;
                }
                if (!valid) {
                    fErrors.error(f.fOffset, "pipeline stage 'main' must be declared main(float, "
                                             "float, inout half4) or main(inout half4)");
                    return;
                }
                break;
            }
            case Program::kGeneric_Kind:
                break;
            default:
                if (parameters.size()) {
                    fErrors.error(f.fOffset, "shader 'main' must have zero parameters");
                }
        }
    }

    // find existing declaration
    const FunctionDeclaration* decl = nullptr;
    auto entry = (*fSymbolTable)[fd.fName];
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
                fErrors.error(f.fOffset, "symbol '" + fd.fName + "' was already defined");
                return;
        }
        for (const auto& other : functions) {
            SkASSERT(other->fName == fd.fName);
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
                        FunctionDeclaration newDecl(f.fOffset, fd.fModifiers, fd.fName, parameters,
                                                    *returnType);
                        fErrors.error(f.fOffset, "functions '" + newDecl.description() +
                                                 "' and '" + other->description() +
                                                 "' differ only in return type");
                        return;
                    }
                    decl = other;
                    for (size_t i = 0; i < parameters.size(); i++) {
                        if (parameters[i]->fModifiers != other->fParameters[i]->fModifiers) {
                            fErrors.error(f.fOffset, "modifiers on parameter " +
                                                     to_string((uint64_t) i + 1) +
                                                     " differ between declaration and "
                                                     "definition");
                            return;
                        }
                    }
                    if (other->fDefined && !other->fBuiltin) {
                        fErrors.error(f.fOffset, "duplicate definition of " +
                                                 other->description());
                    }
                    break;
                }
            }
        }
    }
    if (!decl) {
        // couldn't find an existing declaration
        auto newDecl = std::unique_ptr<FunctionDeclaration>(new FunctionDeclaration(f.fOffset,
                                                                                    fd.fModifiers,
                                                                                    fd.fName,
                                                                                    parameters,
                                                                                    *returnType));
        decl = newDecl.get();
        fSymbolTable->add(decl->fName, std::move(newDecl));
    }
    if (iter != f.end()) {
        // compile body
        SkASSERT(!fCurrentFunction);
        fCurrentFunction = decl;
        decl->fDefined = true;
        std::shared_ptr<SymbolTable> old = fSymbolTable;
        AutoSymbolTable table(this);
        if (fd.fName == "main" && fKind == Program::kPipelineStage_Kind) {
            if (parameters.size() == 3) {
                parameters[0]->fModifiers.fLayout.fBuiltin = SK_MAIN_X_BUILTIN;
                parameters[1]->fModifiers.fLayout.fBuiltin = SK_MAIN_Y_BUILTIN;
                parameters[2]->fModifiers.fLayout.fBuiltin = SK_OUTCOLOR_BUILTIN;
            } else {
                SkASSERT(parameters.size() == 1);
                parameters[0]->fModifiers.fLayout.fBuiltin = SK_OUTCOLOR_BUILTIN;
            }
        }
        for (size_t i = 0; i < parameters.size(); i++) {
            fSymbolTable->addWithoutOwnership(parameters[i]->fName, decl->fParameters[i]);
        }
        bool needInvocationIDWorkaround = fInvocations != -1 && fd.fName == "main" &&
                                          fSettings->fCaps &&
                                          !fSettings->fCaps->gsInvocationsSupport();
        SkASSERT(!fExtraVars.size());
        std::unique_ptr<Block> body = this->convertBlock(*iter);
        for (auto& v : fExtraVars) {
            body->fStatements.insert(body->fStatements.begin(), std::move(v));
        }
        fExtraVars.clear();
        fCurrentFunction = nullptr;
        if (!body) {
            return;
        }
        if (needInvocationIDWorkaround) {
            body = this->applyInvocationIDWorkaround(std::move(body));
        }
        // conservatively assume all user-defined functions have side effects
        ((Modifiers&) decl->fModifiers).fFlags |= Modifiers::kHasSideEffects_Flag;
        if (Program::kVertex_Kind == fKind && fd.fName == "main" && fRTAdjust) {
            body->fStatements.insert(body->fStatements.end(), this->getNormalizeSkPositionCode());
        }
        std::unique_ptr<FunctionDefinition> result(new FunctionDefinition(f.fOffset, *decl,
                                                                          std::move(body)));
        result->fSource = &f;
        fProgramElements->push_back(std::move(result));
    }
}

std::unique_ptr<InterfaceBlock> IRGenerator::convertInterfaceBlock(const ASTNode& intf) {
    SkASSERT(intf.fKind == ASTNode::Kind::kInterfaceBlock);
    ASTNode::InterfaceBlockData id = intf.getInterfaceBlockData();
    std::shared_ptr<SymbolTable> old = fSymbolTable;
    this->pushSymbolTable();
    std::shared_ptr<SymbolTable> symbols = fSymbolTable;
    std::vector<Type::Field> fields;
    bool haveRuntimeArray = false;
    bool foundRTAdjust = false;
    auto iter = intf.begin();
    for (size_t i = 0; i < id.fDeclarationCount; ++i) {
        std::unique_ptr<VarDeclarations> decl = this->convertVarDeclarations(
                                                                 *(iter++),
                                                                 Variable::kInterfaceBlock_Storage);
        if (!decl) {
            return nullptr;
        }
        for (const auto& stmt : decl->fVars) {
            VarDeclaration& vd = (VarDeclaration&) *stmt;
            if (haveRuntimeArray) {
                fErrors.error(decl->fOffset,
                              "only the last entry in an interface block may be a runtime-sized "
                              "array");
            }
            if (vd.fVar == fRTAdjust) {
                foundRTAdjust = true;
                SkASSERT(vd.fVar->fType == *fContext.fFloat4_Type);
                fRTAdjustFieldIndex = fields.size();
            }
            fields.push_back(Type::Field(vd.fVar->fModifiers, vd.fVar->fName,
                                         &vd.fVar->fType));
            if (vd.fValue) {
                fErrors.error(decl->fOffset,
                              "initializers are not permitted on interface block fields");
            }
            if (vd.fVar->fModifiers.fFlags & (Modifiers::kIn_Flag |
                                              Modifiers::kOut_Flag |
                                              Modifiers::kUniform_Flag |
                                              Modifiers::kBuffer_Flag |
                                              Modifiers::kConst_Flag)) {
                fErrors.error(decl->fOffset,
                              "interface block fields may not have storage qualifiers");
            }
            if (vd.fVar->fType.kind() == Type::kArray_Kind &&
                vd.fVar->fType.columns() == -1) {
                haveRuntimeArray = true;
            }
        }
    }
    this->popSymbolTable();
    Type* type = (Type*) old->takeOwnership(std::unique_ptr<Symbol>(new Type(intf.fOffset,
                                                                             id.fTypeName,
                                                                             fields)));
    std::vector<std::unique_ptr<Expression>> sizes;
    for (size_t i = 0; i < id.fSizeCount; ++i) {
        const ASTNode& size = *(iter++);
        if (size) {
            std::unique_ptr<Expression> converted = this->convertExpression(size);
            if (!converted) {
                return nullptr;
            }
            String name = type->fName;
            int64_t count;
            if (converted->fKind == Expression::kIntLiteral_Kind) {
                count = ((IntLiteral&) *converted).fValue;
                if (count <= 0) {
                    fErrors.error(converted->fOffset, "array size must be positive");
                    return nullptr;
                }
                name += "[" + to_string(count) + "]";
            } else {
                fErrors.error(intf.fOffset, "array size must be specified");
                return nullptr;
            }
            type = (Type*) symbols->takeOwnership(std::unique_ptr<Symbol>(
                                                                         new Type(name,
                                                                                  Type::kArray_Kind,
                                                                                  *type,
                                                                                  (int) count)));
            sizes.push_back(std::move(converted));
        } else {
            fErrors.error(intf.fOffset, "array size must be specified");
            return nullptr;
        }
    }
    Variable* var = (Variable*) old->takeOwnership(std::unique_ptr<Symbol>(
                      new Variable(intf.fOffset,
                                   id.fModifiers,
                                   id.fInstanceName.fLength ? id.fInstanceName : id.fTypeName,
                                   *type,
                                   Variable::kGlobal_Storage)));
    if (foundRTAdjust) {
        fRTAdjustInterfaceBlock = var;
    }
    if (id.fInstanceName.fLength) {
        old->addWithoutOwnership(id.fInstanceName, var);
    } else {
        for (size_t i = 0; i < fields.size(); i++) {
            old->add(fields[i].fName, std::unique_ptr<Field>(new Field(intf.fOffset, *var,
                                                                       (int) i)));
        }
    }
    return std::unique_ptr<InterfaceBlock>(new InterfaceBlock(intf.fOffset,
                                                              var,
                                                              id.fTypeName,
                                                              id.fInstanceName,
                                                              std::move(sizes),
                                                              symbols));
}

void IRGenerator::getConstantInt(const Expression& value, int64_t* out) {
    switch (value.fKind) {
        case Expression::kIntLiteral_Kind:
            *out = ((const IntLiteral&) value).fValue;
            break;
        case Expression::kVariableReference_Kind: {
            const Variable& var = ((VariableReference&) value).fVariable;
            if ((var.fModifiers.fFlags & Modifiers::kConst_Flag) &&
                var.fInitialValue) {
                this->getConstantInt(*var.fInitialValue, out);
            }
            break;
        }
        default:
            fErrors.error(value.fOffset, "expected a constant int");
    }
}

void IRGenerator::convertEnum(const ASTNode& e) {
    SkASSERT(e.fKind == ASTNode::Kind::kEnum);
    std::vector<Variable*> variables;
    int64_t currentValue = 0;
    Layout layout;
    ASTNode enumType(e.fNodes, e.fOffset, ASTNode::Kind::kType,
                     ASTNode::TypeData(e.getString(), false, false));
    const Type* type = this->convertType(enumType);
    Modifiers modifiers(layout, Modifiers::kConst_Flag);
    std::shared_ptr<SymbolTable> symbols(new SymbolTable(fSymbolTable, &fErrors));
    fSymbolTable = symbols;
    for (auto iter = e.begin(); iter != e.end(); ++iter) {
        const ASTNode& child = *iter;
        SkASSERT(child.fKind == ASTNode::Kind::kEnumCase);
        std::unique_ptr<Expression> value;
        if (child.begin() != child.end()) {
            value = this->convertExpression(*child.begin());
            if (!value) {
                fSymbolTable = symbols->fParent;
                return;
            }
            this->getConstantInt(*value, &currentValue);
        }
        value = std::unique_ptr<Expression>(new IntLiteral(fContext, e.fOffset, currentValue));
        ++currentValue;
        auto var = std::unique_ptr<Variable>(new Variable(e.fOffset, modifiers, child.getString(),
                                                          *type, Variable::kGlobal_Storage,
                                                          value.get()));
        variables.push_back(var.get());
        symbols->add(child.getString(), std::move(var));
        symbols->takeOwnership(std::move(value));
    }
    fProgramElements->push_back(std::unique_ptr<ProgramElement>(new Enum(e.fOffset, e.getString(),
                                                                         symbols)));
    fSymbolTable = symbols->fParent;
}

const Type* IRGenerator::convertType(const ASTNode& type) {
    ASTNode::TypeData td = type.getTypeData();
    const Symbol* result = (*fSymbolTable)[td.fName];
    if (result && result->fKind == Symbol::kType_Kind) {
        if (td.fIsNullable) {
            if (((Type&) *result) == *fContext.fFragmentProcessor_Type) {
                if (type.begin() != type.end()) {
                    fErrors.error(type.fOffset, "type '" + td.fName + "' may not be used in "
                                                "an array");
                }
                result = fSymbolTable->takeOwnership(std::unique_ptr<Symbol>(
                                                               new Type(String(result->fName) + "?",
                                                                        Type::kNullable_Kind,
                                                                        (const Type&) *result)));
            } else {
                fErrors.error(type.fOffset, "type '" + td.fName + "' may not be nullable");
            }
        }
        for (const auto& size : type) {
            String name(result->fName);
            name += "[";
            if (size) {
                name += to_string(size.getInt());
            }
            name += "]";
            result = (Type*) fSymbolTable->takeOwnership(std::unique_ptr<Symbol>(
                                                                     new Type(name,
                                                                              Type::kArray_Kind,
                                                                              (const Type&) *result,
                                                                              size ? size.getInt()
                                                                                   : 0)));
        }
        return (const Type*) result;
    }
    fErrors.error(type.fOffset, "unknown type '" + td.fName + "'");
    return nullptr;
}

std::unique_ptr<Expression> IRGenerator::convertExpression(const ASTNode& expr) {
    switch (expr.fKind) {
        case ASTNode::Kind::kBinary:
            return this->convertBinaryExpression(expr);
        case ASTNode::Kind::kBool:
            return std::unique_ptr<Expression>(new BoolLiteral(fContext, expr.fOffset,
                                                               expr.getBool()));
        case ASTNode::Kind::kCall:
            return this->convertCallExpression(expr);
        case ASTNode::Kind::kField:
            return this->convertFieldExpression(expr);
        case ASTNode::Kind::kFloat:
            return std::unique_ptr<Expression>(new FloatLiteral(fContext, expr.fOffset,
                                                                expr.getFloat()));
        case ASTNode::Kind::kIdentifier:
            return this->convertIdentifier(expr);
        case ASTNode::Kind::kIndex:
            return this->convertIndexExpression(expr);
        case ASTNode::Kind::kInt:
            return std::unique_ptr<Expression>(new IntLiteral(fContext, expr.fOffset,
                                                              expr.getInt()));
        case ASTNode::Kind::kNull:
            return std::unique_ptr<Expression>(new NullLiteral(fContext, expr.fOffset));
        case ASTNode::Kind::kPostfix:
            return this->convertPostfixExpression(expr);
        case ASTNode::Kind::kPrefix:
            return this->convertPrefixExpression(expr);
        case ASTNode::Kind::kTernary:
            return this->convertTernaryExpression(expr);
        default:
            ABORT("unsupported expression: %s\n", expr.description().c_str());
    }
}

std::unique_ptr<Expression> IRGenerator::convertIdentifier(const ASTNode& identifier) {
    SkASSERT(identifier.fKind == ASTNode::Kind::kIdentifier);
    const Symbol* result = (*fSymbolTable)[identifier.getString()];
    if (!result) {
        fErrors.error(identifier.fOffset, "unknown identifier '" + identifier.getString() + "'");
        return nullptr;
    }
    switch (result->fKind) {
        case Symbol::kFunctionDeclaration_Kind: {
            std::vector<const FunctionDeclaration*> f = {
                (const FunctionDeclaration*) result
            };
            return std::unique_ptr<FunctionReference>(new FunctionReference(fContext,
                                                                            identifier.fOffset,
                                                                            f));
        }
        case Symbol::kUnresolvedFunction_Kind: {
            const UnresolvedFunction* f = (const UnresolvedFunction*) result;
            return std::unique_ptr<FunctionReference>(new FunctionReference(fContext,
                                                                            identifier.fOffset,
                                                                            f->fFunctions));
        }
        case Symbol::kVariable_Kind: {
            const Variable* var = (const Variable*) result;
            switch (var->fModifiers.fLayout.fBuiltin) {
                case SK_WIDTH_BUILTIN:
                    fInputs.fRTWidth = true;
                    break;
                case SK_HEIGHT_BUILTIN:
                    fInputs.fRTHeight = true;
                    break;
#ifndef SKSL_STANDALONE
                case SK_FRAGCOORD_BUILTIN:
                    fInputs.fFlipY = true;
                    if (fSettings->fFlipY &&
                        (!fSettings->fCaps ||
                            !fSettings->fCaps->fragCoordConventionsExtensionString())) {
                        fInputs.fRTHeight = true;
                    }
#endif
            }
            if (fKind == Program::kFragmentProcessor_Kind &&
                (var->fModifiers.fFlags & Modifiers::kIn_Flag) &&
                !(var->fModifiers.fFlags & Modifiers::kUniform_Flag) &&
                !var->fModifiers.fLayout.fKey &&
                var->fModifiers.fLayout.fBuiltin == -1 &&
                var->fType.nonnullable() != *fContext.fFragmentProcessor_Type &&
                var->fType.kind() != Type::kSampler_Kind) {
                bool valid = false;
                for (const auto& decl : fFile->root()) {
                    if (decl.fKind == ASTNode::Kind::kSection) {
                        ASTNode::SectionData section = decl.getSectionData();
                        if (section.fName == "setData") {
                            valid = true;
                            break;
                        }
                    }
                }
                if (!valid) {
                    fErrors.error(identifier.fOffset, "'in' variable must be either 'uniform' or "
                                                      "'layout(key)', or there must be a custom "
                                                      "@setData function");
                }
            }
            // default to kRead_RefKind; this will be corrected later if the variable is written to
            return std::unique_ptr<VariableReference>(new VariableReference(
                                                                 identifier.fOffset,
                                                                 *var,
                                                                 VariableReference::kRead_RefKind));
        }
        case Symbol::kField_Kind: {
            const Field* field = (const Field*) result;
            VariableReference* base = new VariableReference(identifier.fOffset, field->fOwner,
                                                            VariableReference::kRead_RefKind);
            return std::unique_ptr<Expression>(new FieldAccess(
                                                  std::unique_ptr<Expression>(base),
                                                  field->fFieldIndex,
                                                  FieldAccess::kAnonymousInterfaceBlock_OwnerKind));
        }
        case Symbol::kType_Kind: {
            const Type* t = (const Type*) result;
            return std::unique_ptr<TypeReference>(new TypeReference(fContext, identifier.fOffset,
                                                                    *t));
        }
        case Symbol::kExternal_Kind: {
            ExternalValue* r = (ExternalValue*) result;
            return std::unique_ptr<ExternalValueReference>(
                                                 new ExternalValueReference(identifier.fOffset, r));
        }
        default:
            ABORT("unsupported symbol type %d\n", result->fKind);
    }
}

std::unique_ptr<Section> IRGenerator::convertSection(const ASTNode& s) {
    ASTNode::SectionData section = s.getSectionData();
    return std::unique_ptr<Section>(new Section(s.fOffset, section.fName, section.fArgument,
                                                section.fText));
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
    if (expr->coercionCost(type) == INT_MAX) {
        fErrors.error(expr->fOffset, "expected '" + type.description() + "', but found '" +
                                        expr->fType.description() + "'");
        return nullptr;
    }
    if (type.kind() == Type::kScalar_Kind) {
        std::vector<std::unique_ptr<Expression>> args;
        args.push_back(std::move(expr));
        std::unique_ptr<Expression> ctor;
        if (type == *fContext.fFloatLiteral_Type) {
            ctor = this->convertIdentifier(ASTNode(&fFile->fNodes, -1, ASTNode::Kind::kIdentifier,
                                                   "float"));
        } else if (type == *fContext.fIntLiteral_Type) {
            ctor = this->convertIdentifier(ASTNode(&fFile->fNodes, -1, ASTNode::Kind::kIdentifier,
                                                   "int"));
        } else {
            ctor = this->convertIdentifier(ASTNode(&fFile->fNodes, -1, ASTNode::Kind::kIdentifier,
                                                   type.fName));
        }
        if (!ctor) {
            printf("error, null identifier: %s\n", String(type.fName).c_str());
        }
        SkASSERT(ctor);
        return this->call(-1, std::move(ctor), std::move(args));
    }
    if (expr->fKind == Expression::kNullLiteral_Kind) {
        SkASSERT(type.kind() == Type::kNullable_Kind);
        return std::unique_ptr<Expression>(new NullLiteral(expr->fOffset, type));
    }
    std::vector<std::unique_ptr<Expression>> args;
    args.push_back(std::move(expr));
    return std::unique_ptr<Expression>(new Constructor(-1, type, std::move(args)));
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
            if (right.canCoerceTo(left)) {
                *outLeftType = &left;
                *outRightType = &left;
                *outResultType = context.fBool_Type.get();
                return true;
            } if (left.canCoerceTo(right)) {
                *outLeftType = &right;
                *outRightType = &right;
                *outResultType = context.fBool_Type.get();
                return true;
            }
            return false;
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
        case Token::STAREQ:
            if (left.kind() == Type::kScalar_Kind) {
                *outLeftType = &left;
                *outRightType = &left;
                *outResultType = &left;
                return right.canCoerceTo(left);
            }
            // fall through
        case Token::STAR:
            if (is_matrix_multiply(left, right)) {
                // determine final component type
                if (determine_binary_type(context, Token::STAR, left.componentType(),
                                          right.componentType(), outLeftType, outRightType,
                                          outResultType, false)) {
                    *outLeftType = &(*outResultType)->toCompound(context, left.columns(),
                                                                 left.rows());
                    *outRightType = &(*outResultType)->toCompound(context, right.columns(),
                                                                  right.rows());
                    int leftColumns = left.columns();
                    int leftRows = left.rows();
                    int rightColumns;
                    int rightRows;
                    if (right.kind() == Type::kVector_Kind) {
                        // matrix * vector treats the vector as a column vector, so we need to
                        // transpose it
                        rightColumns = right.rows();
                        rightRows = right.columns();
                        SkASSERT(rightColumns == 1);
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
        case Token::PLUSEQ:
        case Token::MINUSEQ:
        case Token::SLASHEQ:
        case Token::PERCENTEQ:
        case Token::SHLEQ:
        case Token::SHREQ:
            if (left.kind() == Type::kScalar_Kind) {
                *outLeftType = &left;
                *outRightType = &left;
                *outResultType = &left;
                return right.canCoerceTo(left);
            }
            // fall through
        case Token::PLUS:    // fall through
        case Token::MINUS:   // fall through
        case Token::SLASH:   // fall through
            isLogical = false;
            validMatrixOrVectorOp = true;
            break;
        case Token::COMMA:
            *outLeftType = &left;
            *outRightType = &right;
            *outResultType = &right;
            return true;
        default:
            isLogical = false;
            validMatrixOrVectorOp = false;
    }
    bool isVectorOrMatrix = left.kind() == Type::kVector_Kind || left.kind() == Type::kMatrix_Kind;
    if (left.kind() == Type::kScalar_Kind && right.kind() == Type::kScalar_Kind &&
            right.canCoerceTo(left)) {
        if (left.priority() > right.priority()) {
            *outLeftType = &left;
            *outRightType = &left;
        } else {
            *outLeftType = &right;
            *outRightType = &right;
        }
        if (isLogical) {
            *outResultType = context.fBool_Type.get();
        } else {
            *outResultType = &left;
        }
        return true;
    }
    if (right.canCoerceTo(left) && isVectorOrMatrix && validMatrixOrVectorOp) {
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

static std::unique_ptr<Expression> short_circuit_boolean(const Context& context,
                                                         const Expression& left,
                                                         Token::Kind op,
                                                         const Expression& right) {
    SkASSERT(left.fKind == Expression::kBoolLiteral_Kind);
    bool leftVal = ((BoolLiteral&) left).fValue;
    if (op == Token::LOGICALAND) {
        // (true && expr) -> (expr) and (false && expr) -> (false)
        return leftVal ? right.clone()
                       : std::unique_ptr<Expression>(new BoolLiteral(context, left.fOffset, false));
    } else if (op == Token::LOGICALOR) {
        // (true || expr) -> (true) and (false || expr) -> (expr)
        return leftVal ? std::unique_ptr<Expression>(new BoolLiteral(context, left.fOffset, true))
                       : right.clone();
    } else {
        // Can't short circuit XOR
        return nullptr;
    }
}

std::unique_ptr<Expression> IRGenerator::constantFold(const Expression& left,
                                                      Token::Kind op,
                                                      const Expression& right) const {
    // If the left side is a constant boolean literal, the right side does not need to be constant
    // for short circuit optimizations to allow the constant to be folded.
    if (left.fKind == Expression::kBoolLiteral_Kind && !right.isConstant()) {
        return short_circuit_boolean(fContext, left, op, right);
    } else if (right.fKind == Expression::kBoolLiteral_Kind && !left.isConstant()) {
        // There aren't side effects in SKSL within expressions, so (left OP right) is equivalent to
        // (right OP left) for short-circuit optimizations
        return short_circuit_boolean(fContext, right, op, left);
    }

    // Other than the short-circuit cases above, constant folding requires both sides to be constant
    if (!left.isConstant() || !right.isConstant()) {
        return nullptr;
    }
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
        return std::unique_ptr<Expression>(new BoolLiteral(fContext, left.fOffset, result));
    }
    #define RESULT(t, op) std::unique_ptr<Expression>(new t ## Literal(fContext, left.fOffset, \
                                                                       leftVal op rightVal))
    if (left.fKind == Expression::kIntLiteral_Kind && right.fKind == Expression::kIntLiteral_Kind) {
        int64_t leftVal  = ((IntLiteral&) left).fValue;
        int64_t rightVal = ((IntLiteral&) right).fValue;
        switch (op) {
            case Token::PLUS:       return RESULT(Int, +);
            case Token::MINUS:      return RESULT(Int, -);
            case Token::STAR:       return RESULT(Int, *);
            case Token::SLASH:
                if (rightVal) {
                    return RESULT(Int, /);
                }
                fErrors.error(right.fOffset, "division by zero");
                return nullptr;
            case Token::PERCENT:
                if (rightVal) {
                    return RESULT(Int, %);
                }
                fErrors.error(right.fOffset, "division by zero");
                return nullptr;
            case Token::BITWISEAND: return RESULT(Int,  &);
            case Token::BITWISEOR:  return RESULT(Int,  |);
            case Token::BITWISEXOR: return RESULT(Int,  ^);
            case Token::EQEQ:       return RESULT(Bool, ==);
            case Token::NEQ:        return RESULT(Bool, !=);
            case Token::GT:         return RESULT(Bool, >);
            case Token::GTEQ:       return RESULT(Bool, >=);
            case Token::LT:         return RESULT(Bool, <);
            case Token::LTEQ:       return RESULT(Bool, <=);
            case Token::SHL:
                if (rightVal >= 0 && rightVal <= 31) {
                    return RESULT(Int,  <<);
                }
                fErrors.error(right.fOffset, "shift value out of range");
                return nullptr;
            case Token::SHR:
                if (rightVal >= 0 && rightVal <= 31) {
                    return RESULT(Int,  >>);
                }
                fErrors.error(right.fOffset, "shift value out of range");
                return nullptr;

            default:
                return nullptr;
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
                fErrors.error(right.fOffset, "division by zero");
                return nullptr;
            case Token::EQEQ:       return RESULT(Bool, ==);
            case Token::NEQ:        return RESULT(Bool, !=);
            case Token::GT:         return RESULT(Bool, >);
            case Token::GTEQ:       return RESULT(Bool, >=);
            case Token::LT:         return RESULT(Bool, <);
            case Token::LTEQ:       return RESULT(Bool, <=);
            default:                return nullptr;
        }
    }
    if (left.fType.kind() == Type::kVector_Kind && left.fType.componentType().isFloat() &&
        left.fType == right.fType) {
        std::vector<std::unique_ptr<Expression>> args;
        #define RETURN_VEC_COMPONENTWISE_RESULT(op)                              \
            for (int i = 0; i < left.fType.columns(); i++) {                     \
                float value = left.getFVecComponent(i) op                        \
                              right.getFVecComponent(i);                         \
                args.emplace_back(new FloatLiteral(fContext, -1, value));        \
            }                                                                    \
            return std::unique_ptr<Expression>(new Constructor(-1, left.fType,   \
                                                               std::move(args)))
        switch (op) {
            case Token::EQEQ:
                return std::unique_ptr<Expression>(new BoolLiteral(fContext, -1,
                                                            left.compareConstant(fContext, right)));
            case Token::NEQ:
                return std::unique_ptr<Expression>(new BoolLiteral(fContext, -1,
                                                           !left.compareConstant(fContext, right)));
            case Token::PLUS:  RETURN_VEC_COMPONENTWISE_RESULT(+);
            case Token::MINUS: RETURN_VEC_COMPONENTWISE_RESULT(-);
            case Token::STAR:  RETURN_VEC_COMPONENTWISE_RESULT(*);
            case Token::SLASH:
                for (int i = 0; i < left.fType.columns(); i++) {
                    SKSL_FLOAT rvalue = right.getFVecComponent(i);
                    if (rvalue == 0.0) {
                        fErrors.error(right.fOffset, "division by zero");
                        return nullptr;
                    }
                    float value = left.getFVecComponent(i) / rvalue;
                    args.emplace_back(new FloatLiteral(fContext, -1, value));
                }
                return std::unique_ptr<Expression>(new Constructor(-1, left.fType,
                                                                   std::move(args)));
            default:           return nullptr;
        }
    }
    if (left.fType.kind() == Type::kMatrix_Kind &&
        right.fType.kind() == Type::kMatrix_Kind &&
        left.fKind == right.fKind) {
        switch (op) {
            case Token::EQEQ:
                return std::unique_ptr<Expression>(new BoolLiteral(fContext, -1,
                                                            left.compareConstant(fContext, right)));
            case Token::NEQ:
                return std::unique_ptr<Expression>(new BoolLiteral(fContext, -1,
                                                           !left.compareConstant(fContext, right)));
            default:
                return nullptr;
        }
    }
    #undef RESULT
    return nullptr;
}

std::unique_ptr<Expression> IRGenerator::convertBinaryExpression(const ASTNode& expression) {
    SkASSERT(expression.fKind == ASTNode::Kind::kBinary);
    auto iter = expression.begin();
    std::unique_ptr<Expression> left = this->convertExpression(*(iter++));
    if (!left) {
        return nullptr;
    }
    std::unique_ptr<Expression> right = this->convertExpression(*(iter++));
    if (!right) {
        return nullptr;
    }
    const Type* leftType;
    const Type* rightType;
    const Type* resultType;
    const Type* rawLeftType;
    if (left->fKind == Expression::kIntLiteral_Kind && right->fType.isInteger()) {
        rawLeftType = &right->fType;
    } else {
        rawLeftType = &left->fType;
    }
    const Type* rawRightType;
    if (right->fKind == Expression::kIntLiteral_Kind && left->fType.isInteger()) {
        rawRightType = &left->fType;
    } else {
        rawRightType = &right->fType;
    }
    Token::Kind op = expression.getToken().fKind;
    if (!determine_binary_type(fContext, op, *rawLeftType, *rawRightType, &leftType, &rightType,
                               &resultType, !Compiler::IsAssignment(op))) {
        fErrors.error(expression.fOffset, String("type mismatch: '") +
                                          Compiler::OperatorName(expression.getToken().fKind) +
                                          "' cannot operate on '" + left->fType.description() +
                                          "', '" + right->fType.description() + "'");
        return nullptr;
    }
    if (Compiler::IsAssignment(op)) {
        this->setRefKind(*left, op != Token::EQ ? VariableReference::kReadWrite_RefKind :
                                                  VariableReference::kWrite_RefKind);
    }
    left = this->coerce(std::move(left), *leftType);
    right = this->coerce(std::move(right), *rightType);
    if (!left || !right) {
        return nullptr;
    }
    std::unique_ptr<Expression> result = this->constantFold(*left.get(), op, *right.get());
    if (!result) {
        result = std::unique_ptr<Expression>(new BinaryExpression(expression.fOffset,
                                                                  std::move(left),
                                                                  op,
                                                                  std::move(right),
                                                                  *resultType));
    }
    return result;
}

std::unique_ptr<Expression> IRGenerator::convertTernaryExpression(const ASTNode& node) {
    SkASSERT(node.fKind == ASTNode::Kind::kTernary);
    auto iter = node.begin();
    std::unique_ptr<Expression> test = this->coerce(this->convertExpression(*(iter++)),
                                                    *fContext.fBool_Type);
    if (!test) {
        return nullptr;
    }
    std::unique_ptr<Expression> ifTrue = this->convertExpression(*(iter++));
    if (!ifTrue) {
        return nullptr;
    }
    std::unique_ptr<Expression> ifFalse = this->convertExpression(*(iter++));
    if (!ifFalse) {
        return nullptr;
    }
    const Type* trueType;
    const Type* falseType;
    const Type* resultType;
    if (!determine_binary_type(fContext, Token::EQEQ, ifTrue->fType, ifFalse->fType, &trueType,
                               &falseType, &resultType, true) || trueType != falseType) {
        fErrors.error(node.fOffset, "ternary operator result mismatch: '" +
                                    ifTrue->fType.description() + "', '" +
                                    ifFalse->fType.description() + "'");
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
    return std::unique_ptr<Expression>(new TernaryExpression(node.fOffset,
                                                             std::move(test),
                                                             std::move(ifTrue),
                                                             std::move(ifFalse)));
}

std::unique_ptr<Expression> IRGenerator::call(int offset,
                                              const FunctionDeclaration& function,
                                              std::vector<std::unique_ptr<Expression>> arguments) {
    if (function.fBuiltin) {
        auto found = fIntrinsics->find(function.fName);
        if (found != fIntrinsics->end() && !found->second.second) {
            found->second.second = true;
            const FunctionDeclaration* old = fCurrentFunction;
            fCurrentFunction = nullptr;
            this->convertFunction(*((FunctionDefinition&) *found->second.first).fSource);
            fCurrentFunction = old;
        }
    }
    if (function.fParameters.size() != arguments.size()) {
        String msg = "call to '" + function.fName + "' expected " +
                                 to_string((uint64_t) function.fParameters.size()) +
                                 " argument";
        if (function.fParameters.size() != 1) {
            msg += "s";
        }
        msg += ", but found " + to_string((uint64_t) arguments.size());
        fErrors.error(offset, msg);
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
        fErrors.error(offset, msg);
        return nullptr;
    }
    for (size_t i = 0; i < arguments.size(); i++) {
        arguments[i] = this->coerce(std::move(arguments[i]), *types[i]);
        if (!arguments[i]) {
            return nullptr;
        }
        if (arguments[i] && (function.fParameters[i]->fModifiers.fFlags & Modifiers::kOut_Flag)) {
            this->setRefKind(*arguments[i],
                             function.fParameters[i]->fModifiers.fFlags & Modifiers::kIn_Flag ?
                             VariableReference::kReadWrite_RefKind :
                             VariableReference::kPointer_RefKind);
        }
    }
    return std::unique_ptr<FunctionCall>(new FunctionCall(offset, *returnType, function,
                                                          std::move(arguments)));
}

/**
 * Determines the cost of coercing the arguments of a function to the required types. Cost has no
 * particular meaning other than "lower costs are preferred". Returns INT_MAX if the call is not
 * valid.
 */
int IRGenerator::callCost(const FunctionDeclaration& function,
             const std::vector<std::unique_ptr<Expression>>& arguments) {
    if (function.fParameters.size() != arguments.size()) {
        return INT_MAX;
    }
    int total = 0;
    std::vector<const Type*> types;
    const Type* ignored;
    if (!function.determineFinalTypes(arguments, &types, &ignored)) {
        return INT_MAX;
    }
    for (size_t i = 0; i < arguments.size(); i++) {
        int cost = arguments[i]->coercionCost(*types[i]);
        if (cost != INT_MAX) {
            total += cost;
        } else {
            return INT_MAX;
        }
    }
    return total;
}

std::unique_ptr<Expression> IRGenerator::call(int offset,
                                              std::unique_ptr<Expression> functionValue,
                                              std::vector<std::unique_ptr<Expression>> arguments) {
    switch (functionValue->fKind) {
        case Expression::kTypeReference_Kind:
            return this->convertConstructor(offset,
                                            ((TypeReference&) *functionValue).fValue,
                                            std::move(arguments));
        case Expression::kExternalValue_Kind: {
            ExternalValue* v = ((ExternalValueReference&) *functionValue).fValue;
            if (!v->canCall()) {
                fErrors.error(offset, "this external value is not a function");
                return nullptr;
            }
            int count = v->callParameterCount();
            if (count != (int) arguments.size()) {
                fErrors.error(offset, "external function expected " + to_string(count) +
                                      " arguments, but found " + to_string((int) arguments.size()));
                return nullptr;
            }
            static constexpr int PARAMETER_MAX = 16;
            SkASSERT(count < PARAMETER_MAX);
            const Type* types[PARAMETER_MAX];
            v->getCallParameterTypes(types);
            for (int i = 0; i < count; ++i) {
                arguments[i] = this->coerce(std::move(arguments[i]), *types[i]);
                if (!arguments[i]) {
                    return nullptr;
                }
            }
            return std::unique_ptr<Expression>(new ExternalFunctionCall(offset, v->callReturnType(),
                                                                        v, std::move(arguments)));
        }
        case Expression::kFunctionReference_Kind: {
            FunctionReference* ref = (FunctionReference*) functionValue.get();
            int bestCost = INT_MAX;
            const FunctionDeclaration* best = nullptr;
            if (ref->fFunctions.size() > 1) {
                for (const auto& f : ref->fFunctions) {
                    int cost = this->callCost(*f, arguments);
                    if (cost < bestCost) {
                        bestCost = cost;
                        best = f;
                    }
                }
                if (best) {
                    return this->call(offset, *best, std::move(arguments));
                }
                String msg = "no match for " + ref->fFunctions[0]->fName + "(";
                String separator;
                for (size_t i = 0; i < arguments.size(); i++) {
                    msg += separator;
                    separator = ", ";
                    msg += arguments[i]->fType.description();
                }
                msg += ")";
                fErrors.error(offset, msg);
                return nullptr;
            }
            return this->call(offset, *ref->fFunctions[0], std::move(arguments));
        }
        default:
            fErrors.error(offset, "'" + functionValue->description() + "' is not a function");
            return nullptr;
    }
}

std::unique_ptr<Expression> IRGenerator::convertNumberConstructor(
                                                    int offset,
                                                    const Type& type,
                                                    std::vector<std::unique_ptr<Expression>> args) {
    SkASSERT(type.isNumber());
    if (args.size() != 1) {
        fErrors.error(offset, "invalid arguments to '" + type.description() +
                              "' constructor, (expected exactly 1 argument, but found " +
                              to_string((uint64_t) args.size()) + ")");
        return nullptr;
    }
    if (type == args[0]->fType) {
        return std::move(args[0]);
    }
    if (type.isFloat() && args.size() == 1 && args[0]->fKind == Expression::kFloatLiteral_Kind) {
        double value = ((FloatLiteral&) *args[0]).fValue;
        return std::unique_ptr<Expression>(new FloatLiteral(offset, value, &type));
    }
    if (type.isFloat() && args.size() == 1 && args[0]->fKind == Expression::kIntLiteral_Kind) {
        int64_t value = ((IntLiteral&) *args[0]).fValue;
        return std::unique_ptr<Expression>(new FloatLiteral(offset, (double) value, &type));
    }
    if (args[0]->fKind == Expression::kIntLiteral_Kind && (type == *fContext.fInt_Type ||
        type == *fContext.fUInt_Type)) {
        return std::unique_ptr<Expression>(new IntLiteral(offset,
                                                          ((IntLiteral&) *args[0]).fValue,
                                                          &type));
    }
    if (args[0]->fType == *fContext.fBool_Type) {
        std::unique_ptr<IntLiteral> zero(new IntLiteral(fContext, offset, 0));
        std::unique_ptr<IntLiteral> one(new IntLiteral(fContext, offset, 1));
        return std::unique_ptr<Expression>(
                                     new TernaryExpression(offset, std::move(args[0]),
                                                           this->coerce(std::move(one), type),
                                                           this->coerce(std::move(zero),
                                                                        type)));
    }
    if (!args[0]->fType.isNumber()) {
        fErrors.error(offset, "invalid argument to '" + type.description() +
                              "' constructor (expected a number or bool, but found '" +
                              args[0]->fType.description() + "')");
        return nullptr;
    }
    return std::unique_ptr<Expression>(new Constructor(offset, type, std::move(args)));
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
                                                    int offset,
                                                    const Type& type,
                                                    std::vector<std::unique_ptr<Expression>> args) {
    SkASSERT(type.kind() == Type::kVector_Kind || type.kind() == Type::kMatrix_Kind);
    if (type.kind() == Type::kMatrix_Kind && args.size() == 1 &&
        args[0]->fType.kind() == Type::kMatrix_Kind) {
        // matrix from matrix is always legal
        return std::unique_ptr<Expression>(new Constructor(offset, type, std::move(args)));
    }
    int actual = 0;
    int expected = type.rows() * type.columns();
    if (args.size() != 1 || expected != component_count(args[0]->fType) ||
        type.componentType().isNumber() != args[0]->fType.componentType().isNumber()) {
        for (size_t i = 0; i < args.size(); i++) {
            if (args[i]->fType.kind() == Type::kVector_Kind) {
                if (type.componentType().isNumber() !=
                    args[i]->fType.componentType().isNumber()) {
                    fErrors.error(offset, "'" + args[i]->fType.description() + "' is not a valid "
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
                fErrors.error(offset, "'" + args[i]->fType.description() + "' is not a valid "
                                      "parameter to '" + type.description() + "' constructor");
                return nullptr;
            }
        }
        if (actual != 1 && actual != expected) {
            fErrors.error(offset, "invalid arguments to '" + type.description() +
                                  "' constructor (expected " + to_string(expected) +
                                  " scalars, but found " + to_string(actual) + ")");
            return nullptr;
        }
    }
    return std::unique_ptr<Expression>(new Constructor(offset, type, std::move(args)));
}

std::unique_ptr<Expression> IRGenerator::convertConstructor(
                                                    int offset,
                                                    const Type& type,
                                                    std::vector<std::unique_ptr<Expression>> args) {
    // FIXME: add support for structs
    Type::Kind kind = type.kind();
    if (args.size() == 1 && args[0]->fType == type) {
        // argument is already the right type, just return it
        return std::move(args[0]);
    }
    if (type.isNumber()) {
        return this->convertNumberConstructor(offset, type, std::move(args));
    } else if (kind == Type::kArray_Kind) {
        const Type& base = type.componentType();
        for (size_t i = 0; i < args.size(); i++) {
            args[i] = this->coerce(std::move(args[i]), base);
            if (!args[i]) {
                return nullptr;
            }
        }
        return std::unique_ptr<Expression>(new Constructor(offset, type, std::move(args)));
    } else if (kind == Type::kVector_Kind || kind == Type::kMatrix_Kind) {
        return this->convertCompoundConstructor(offset, type, std::move(args));
    } else {
        fErrors.error(offset, "cannot construct '" + type.description() + "'");
        return nullptr;
    }
}

std::unique_ptr<Expression> IRGenerator::convertPrefixExpression(const ASTNode& expression) {
    SkASSERT(expression.fKind == ASTNode::Kind::kPrefix);
    std::unique_ptr<Expression> base = this->convertExpression(*expression.begin());
    if (!base) {
        return nullptr;
    }
    switch (expression.getToken().fKind) {
        case Token::PLUS:
            if (!base->fType.isNumber() && base->fType.kind() != Type::kVector_Kind &&
                base->fType != *fContext.fFloatLiteral_Type) {
                fErrors.error(expression.fOffset,
                              "'+' cannot operate on '" + base->fType.description() + "'");
                return nullptr;
            }
            return base;
        case Token::MINUS:
            if (base->fKind == Expression::kIntLiteral_Kind) {
                return std::unique_ptr<Expression>(new IntLiteral(fContext, base->fOffset,
                                                                  -((IntLiteral&) *base).fValue));
            }
            if (base->fKind == Expression::kFloatLiteral_Kind) {
                double value = -((FloatLiteral&) *base).fValue;
                return std::unique_ptr<Expression>(new FloatLiteral(fContext, base->fOffset,
                                                                    value));
            }
            if (!base->fType.isNumber() && base->fType.kind() != Type::kVector_Kind) {
                fErrors.error(expression.fOffset,
                              "'-' cannot operate on '" + base->fType.description() + "'");
                return nullptr;
            }
            return std::unique_ptr<Expression>(new PrefixExpression(Token::MINUS, std::move(base)));
        case Token::PLUSPLUS:
            if (!base->fType.isNumber()) {
                fErrors.error(expression.fOffset,
                              String("'") + Compiler::OperatorName(expression.getToken().fKind) +
                              "' cannot operate on '" + base->fType.description() + "'");
                return nullptr;
            }
            this->setRefKind(*base, VariableReference::kReadWrite_RefKind);
            break;
        case Token::MINUSMINUS:
            if (!base->fType.isNumber()) {
                fErrors.error(expression.fOffset,
                              String("'") + Compiler::OperatorName(expression.getToken().fKind) +
                              "' cannot operate on '" + base->fType.description() + "'");
                return nullptr;
            }
            this->setRefKind(*base, VariableReference::kReadWrite_RefKind);
            break;
        case Token::LOGICALNOT:
            if (base->fType != *fContext.fBool_Type) {
                fErrors.error(expression.fOffset,
                              String("'") + Compiler::OperatorName(expression.getToken().fKind) +
                              "' cannot operate on '" + base->fType.description() + "'");
                return nullptr;
            }
            if (base->fKind == Expression::kBoolLiteral_Kind) {
                return std::unique_ptr<Expression>(new BoolLiteral(fContext, base->fOffset,
                                                                   !((BoolLiteral&) *base).fValue));
            }
            break;
        case Token::BITWISENOT:
            if (base->fType != *fContext.fInt_Type && base->fType != *fContext.fUInt_Type) {
                fErrors.error(expression.fOffset,
                              String("'") + Compiler::OperatorName(expression.getToken().fKind) +
                              "' cannot operate on '" + base->fType.description() + "'");
                return nullptr;
            }
            break;
        default:
            ABORT("unsupported prefix operator\n");
    }
    return std::unique_ptr<Expression>(new PrefixExpression(expression.getToken().fKind,
                                                            std::move(base)));
}

std::unique_ptr<Expression> IRGenerator::convertIndex(std::unique_ptr<Expression> base,
                                                      const ASTNode& index) {
    if (base->fKind == Expression::kTypeReference_Kind) {
        if (index.fKind == ASTNode::Kind::kInt) {
            const Type& oldType = ((TypeReference&) *base).fValue;
            SKSL_INT size = index.getInt();
            Type* newType = (Type*) fSymbolTable->takeOwnership(std::unique_ptr<Symbol>(
                                              new Type(oldType.name() + "[" + to_string(size) + "]",
                                                       Type::kArray_Kind, oldType, size)));
            return std::unique_ptr<Expression>(new TypeReference(fContext, base->fOffset,
                                                                 *newType));

        } else {
            fErrors.error(base->fOffset, "array size must be a constant");
            return nullptr;
        }
    }
    if (base->fType.kind() != Type::kArray_Kind && base->fType.kind() != Type::kMatrix_Kind &&
            base->fType.kind() != Type::kVector_Kind) {
        fErrors.error(base->fOffset, "expected array, but found '" + base->fType.description() +
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
                                                      StringFragment field) {
    if (base->fKind == Expression::kExternalValue_Kind) {
        ExternalValue& ev = *((ExternalValueReference&) *base).fValue;
        ExternalValue* result = ev.getChild(String(field).c_str());
        if (!result) {
            fErrors.error(base->fOffset, "external value does not have a child named '" + field +
                                         "'");
            return nullptr;
        }
        return std::unique_ptr<Expression>(new ExternalValueReference(base->fOffset, result));
    }
    auto fields = base->fType.fields();
    for (size_t i = 0; i < fields.size(); i++) {
        if (fields[i].fName == field) {
            return std::unique_ptr<Expression>(new FieldAccess(std::move(base), (int) i));
        }
    }
    fErrors.error(base->fOffset, "type '" + base->fType.description() + "' does not have a "
                                 "field named '" + field + "");
    return nullptr;
}

std::unique_ptr<Expression> IRGenerator::convertSwizzle(std::unique_ptr<Expression> base,
                                                        StringFragment fields) {
    if (base->fType.kind() != Type::kVector_Kind) {
        fErrors.error(base->fOffset, "cannot swizzle type '" + base->fType.description() + "'");
        return nullptr;
    }
    std::vector<int> swizzleComponents;
    for (size_t i = 0; i < fields.fLength; i++) {
        switch (fields[i]) {
            case '0':
                swizzleComponents.push_back(SKSL_SWIZZLE_0);
                break;
            case '1':
                swizzleComponents.push_back(SKSL_SWIZZLE_1);
                break;
            case 'x':
            case 'r':
            case 's':
            case 'L':
                swizzleComponents.push_back(0);
                break;
            case 'y':
            case 'g':
            case 't':
            case 'T':
                if (base->fType.columns() >= 2) {
                    swizzleComponents.push_back(1);
                    break;
                }
                // fall through
            case 'z':
            case 'b':
            case 'p':
            case 'R':
                if (base->fType.columns() >= 3) {
                    swizzleComponents.push_back(2);
                    break;
                }
                // fall through
            case 'w':
            case 'a':
            case 'q':
            case 'B':
                if (base->fType.columns() >= 4) {
                    swizzleComponents.push_back(3);
                    break;
                }
                // fall through
            default:
                fErrors.error(base->fOffset, String::printf("invalid swizzle component '%c'",
                                                            fields[i]));
                return nullptr;
        }
    }
    SkASSERT(swizzleComponents.size() > 0);
    if (swizzleComponents.size() > 4) {
        fErrors.error(base->fOffset, "too many components in swizzle mask '" + fields + "'");
        return nullptr;
    }
    return std::unique_ptr<Expression>(new Swizzle(fContext, std::move(base), swizzleComponents));
}

std::unique_ptr<Expression> IRGenerator::getCap(int offset, String name) {
    auto found = fCapsMap.find(name);
    if (found == fCapsMap.end()) {
        fErrors.error(offset, "unknown capability flag '" + name + "'");
        return nullptr;
    }
    String fullName = "sk_Caps." + name;
    return std::unique_ptr<Expression>(new Setting(offset, fullName,
                                                   found->second.literal(fContext, offset)));
}

std::unique_ptr<Expression> IRGenerator::getArg(int offset, String name) const {
    auto found = fSettings->fArgs.find(name);
    if (found == fSettings->fArgs.end()) {
        return nullptr;
    }
    String fullName = "sk_Args." + name;
    return std::unique_ptr<Expression>(new Setting(offset,
                                                   fullName,
                                                   found->second.literal(fContext, offset)));
}

std::unique_ptr<Expression> IRGenerator::convertTypeField(int offset, const Type& type,
                                                          StringFragment field) {
    std::unique_ptr<Expression> result;
    for (const auto& e : *fProgramElements) {
        if (e->fKind == ProgramElement::kEnum_Kind && type.name() == ((Enum&) *e).fTypeName) {
            std::shared_ptr<SymbolTable> old = fSymbolTable;
            fSymbolTable = ((Enum&) *e).fSymbols;
            result = convertIdentifier(ASTNode(&fFile->fNodes, offset, ASTNode::Kind::kIdentifier,
                                               field));
            SkASSERT(result->fKind == Expression::kVariableReference_Kind);
            const Variable& v = ((VariableReference&) *result).fVariable;
            SkASSERT(v.fInitialValue);
            SkASSERT(v.fInitialValue->fKind == Expression::kIntLiteral_Kind);
            result.reset(new IntLiteral(offset, ((IntLiteral&) *v.fInitialValue).fValue, &type));
            fSymbolTable = old;
            break;
        }
    }
    if (!result) {
        auto found = fIntrinsics->find(type.fName);
        if (found != fIntrinsics->end()) {
            SkASSERT(!found->second.second);
            found->second.second = true;
            fProgramElements->push_back(found->second.first->clone());
            return this->convertTypeField(offset, type, field);
        }
        fErrors.error(offset, "type '" + type.fName + "' does not have a field named '" + field +
                              "'");
    }
    return result;
}

std::unique_ptr<Expression> IRGenerator::convertIndexExpression(const ASTNode& index) {
    SkASSERT(index.fKind == ASTNode::Kind::kIndex);
    auto iter = index.begin();
    std::unique_ptr<Expression> base = this->convertExpression(*(iter++));
    if (!base) {
        return nullptr;
    }
    if (iter != index.end()) {
        return this->convertIndex(std::move(base), *(iter++));
    } else if (base->fKind == Expression::kTypeReference_Kind) {
        const Type& oldType = ((TypeReference&) *base).fValue;
        Type* newType = (Type*) fSymbolTable->takeOwnership(std::unique_ptr<Symbol>(
                                                                     new Type(oldType.name() + "[]",
                                                                              Type::kArray_Kind,
                                                                              oldType,
                                                                              -1)));
        return std::unique_ptr<Expression>(new TypeReference(fContext, base->fOffset,
                                                             *newType));
    }
    fErrors.error(index.fOffset, "'[]' must follow a type name");
    return nullptr;
}

std::unique_ptr<Expression> IRGenerator::convertCallExpression(const ASTNode& callNode) {
    SkASSERT(callNode.fKind == ASTNode::Kind::kCall);
    auto iter = callNode.begin();
    std::unique_ptr<Expression> base = this->convertExpression(*(iter++));
    if (!base) {
        return nullptr;
    }
    std::vector<std::unique_ptr<Expression>> arguments;
    for (; iter != callNode.end(); ++iter) {
        std::unique_ptr<Expression> converted = this->convertExpression(*iter);
        if (!converted) {
            return nullptr;
        }
        arguments.push_back(std::move(converted));
    }
    return this->call(callNode.fOffset, std::move(base), std::move(arguments));
}

std::unique_ptr<Expression> IRGenerator::convertFieldExpression(const ASTNode& fieldNode) {
    std::unique_ptr<Expression> base = this->convertExpression(*fieldNode.begin());
    if (!base) {
        return nullptr;
    }
    StringFragment field = fieldNode.getString();
    if (base->fType == *fContext.fSkCaps_Type) {
        return this->getCap(fieldNode.fOffset, field);
    }
    if (base->fType == *fContext.fSkArgs_Type) {
        return this->getArg(fieldNode.fOffset, field);
    }
    if (base->fKind == Expression::kTypeReference_Kind) {
        return this->convertTypeField(base->fOffset, ((TypeReference&) *base).fValue,
                                      field);
    }
    if (base->fKind == Expression::kExternalValue_Kind) {
        return this->convertField(std::move(base), field);
    }
    switch (base->fType.kind()) {
        case Type::kVector_Kind:
            return this->convertSwizzle(std::move(base), field);
        case Type::kOther_Kind:
        case Type::kStruct_Kind:
            return this->convertField(std::move(base), field);
        default:
            fErrors.error(base->fOffset, "cannot swizzle value of type '" +
                                         base->fType.description() + "'");
            return nullptr;
    }
}

std::unique_ptr<Expression> IRGenerator::convertPostfixExpression(const ASTNode& expression) {
    std::unique_ptr<Expression> base = this->convertExpression(*expression.begin());
    if (!base) {
        return nullptr;
    }
    if (!base->fType.isNumber()) {
        fErrors.error(expression.fOffset,
                      "'" + String(Compiler::OperatorName(expression.getToken().fKind)) +
                      "' cannot operate on '" + base->fType.description() + "'");
        return nullptr;
    }
    this->setRefKind(*base, VariableReference::kReadWrite_RefKind);
    return std::unique_ptr<Expression>(new PostfixExpression(std::move(base),
                                                             expression.getToken().fKind));
}

void IRGenerator::checkValid(const Expression& expr) {
    switch (expr.fKind) {
        case Expression::kFunctionReference_Kind:
            fErrors.error(expr.fOffset, "expected '(' to begin function call");
            break;
        case Expression::kTypeReference_Kind:
            fErrors.error(expr.fOffset, "expected '(' to begin constructor invocation");
            break;
        default:
            if (expr.fType == *fContext.fInvalid_Type) {
                fErrors.error(expr.fOffset, "invalid expression");
            }
    }
}

bool IRGenerator::checkSwizzleWrite(const Swizzle& swizzle) {
    int bits = 0;
    for (int idx : swizzle.fComponents) {
        if (idx < 0) {
            fErrors.error(swizzle.fOffset, "cannot write to a swizzle mask containing a constant");
            return false;
        }
        SkASSERT(idx <= 3);
        int bit = 1 << idx;
        if (bits & bit) {
            fErrors.error(swizzle.fOffset,
                          "cannot write to the same swizzle field more than once");
            return false;
        }
        bits |= bit;
    }
    return true;
}

void IRGenerator::setRefKind(const Expression& expr, VariableReference::RefKind kind) {
    switch (expr.fKind) {
        case Expression::kVariableReference_Kind: {
            const Variable& var = ((VariableReference&) expr).fVariable;
            if (var.fModifiers.fFlags & (Modifiers::kConst_Flag | Modifiers::kUniform_Flag)) {
                fErrors.error(expr.fOffset,
                              "cannot modify immutable variable '" + var.fName + "'");
            }
            ((VariableReference&) expr).setRefKind(kind);
            break;
        }
        case Expression::kFieldAccess_Kind:
            this->setRefKind(*((FieldAccess&) expr).fBase, kind);
            break;
        case Expression::kSwizzle_Kind: {
            const Swizzle& swizzle = (Swizzle&) expr;
            this->checkSwizzleWrite(swizzle);
            this->setRefKind(*swizzle.fBase, kind);
            break;
        }
        case Expression::kIndex_Kind:
            this->setRefKind(*((IndexExpression&) expr).fBase, kind);
            break;
        case Expression::kTernary_Kind: {
            TernaryExpression& t = (TernaryExpression&) expr;
            this->setRefKind(*t.fIfTrue, kind);
            this->setRefKind(*t.fIfFalse, kind);
            break;
        }
        case Expression::kExternalValue_Kind: {
            const ExternalValue& v = *((ExternalValueReference&) expr).fValue;
            if (!v.canWrite()) {
                fErrors.error(expr.fOffset,
                              "cannot modify immutable external value '" + v.fName + "'");
            }
            break;
        }
        default:
            fErrors.error(expr.fOffset, "cannot assign to '" + expr.description() + "'");
            break;
    }
}

void IRGenerator::convertProgram(Program::Kind kind,
                                 const char* text,
                                 size_t length,
                                 SymbolTable& types,
                                 std::vector<std::unique_ptr<ProgramElement>>* out) {
    fKind = kind;
    fProgramElements = out;
    Parser parser(text, length, types, fErrors);
    fFile = parser.file();
    if (fErrors.errorCount()) {
        return;
    }
    SkASSERT(fFile);
    for (const auto& decl : fFile->root()) {
        switch (decl.fKind) {
            case ASTNode::Kind::kVarDeclarations: {
                std::unique_ptr<VarDeclarations> s = this->convertVarDeclarations(
                                                                         decl,
                                                                         Variable::kGlobal_Storage);
                if (s) {
                    fProgramElements->push_back(std::move(s));
                }
                break;
            }
            case ASTNode::Kind::kEnum: {
                this->convertEnum(decl);
                break;
            }
            case ASTNode::Kind::kFunction: {
                this->convertFunction(decl);
                break;
            }
            case ASTNode::Kind::kModifiers: {
                std::unique_ptr<ModifiersDeclaration> f = this->convertModifiersDeclaration(decl);
                if (f) {
                    fProgramElements->push_back(std::move(f));
                }
                break;
            }
            case ASTNode::Kind::kInterfaceBlock: {
                std::unique_ptr<InterfaceBlock> i = this->convertInterfaceBlock(decl);
                if (i) {
                    fProgramElements->push_back(std::move(i));
                }
                break;
            }
            case ASTNode::Kind::kExtension: {
                std::unique_ptr<Extension> e = this->convertExtension(decl.fOffset,
                                                                      decl.getString());
                if (e) {
                    fProgramElements->push_back(std::move(e));
                }
                break;
            }
            case ASTNode::Kind::kSection: {
                std::unique_ptr<Section> s = this->convertSection(decl);
                if (s) {
                    fProgramElements->push_back(std::move(s));
                }
                break;
            }
            default:
                ABORT("unsupported declaration: %s\n", decl.description().c_str());
        }
    }
}


}
