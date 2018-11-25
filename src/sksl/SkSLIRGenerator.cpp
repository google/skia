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
#include "SkSLParser.h"
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
#include "ir/SkSLEnum.h"
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
#include "ir/SkSLSetting.h"
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
#define CAP(name) capsMap->insert(std::make_pair(String(#name), \
                                  Program::Settings::Value(caps.name())));
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
    CAP(floatIs32Bits);
    CAP(integerSupport);
#undef CAP
}

void IRGenerator::start(const Program::Settings* settings) {
    fSettings = settings;
    fCapsMap.clear();
    if (settings->fCaps) {
        fill_caps(*settings->fCaps, &fCapsMap);
    }
    this->pushSymbolTable();
    fInvocations = -1;
    fInputs.reset();
    fSkPerVertex = nullptr;
    fRTAdjust = nullptr;
    fRTAdjustInterfaceBlock = nullptr;
}

void IRGenerator::finish() {
    this->popSymbolTable();
    fSettings = nullptr;
}

std::unique_ptr<Extension> IRGenerator::convertExtension(const ASTExtension& extension) {
    return std::unique_ptr<Extension>(new Extension(extension.fOffset, extension.fName));
}

std::unique_ptr<Statement> IRGenerator::convertStatement(const ASTStatement& statement) {
    switch (statement.fKind) {
        case ASTStatement::kBlock_Kind:
            return this->convertBlock((ASTBlock&) statement);
        case ASTStatement::kVarDeclaration_Kind:
            return this->convertVarDeclarationStatement((ASTVarDeclarationStatement&) statement);
        case ASTStatement::kExpression_Kind: {
            std::unique_ptr<Statement> result =
                              this->convertExpressionStatement((ASTExpressionStatement&) statement);
            if (fRTAdjust && Program::kGeometry_Kind == fKind) {
                ASSERT(result->fKind == Statement::kExpression_Kind);
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
    return std::unique_ptr<Block>(new Block(block.fOffset, std::move(statements), fSymbolTable));
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
    std::vector<std::unique_ptr<VarDeclaration>> variables;
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
                String name(type->fName);
                int64_t count;
                if (size->fKind == Expression::kIntLiteral_Kind) {
                    count = ((IntLiteral&) *size).fValue;
                    if (count <= 0) {
                        fErrors.error(size->fOffset, "array size must be positive");
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
                type = new Type(type->name() + "[]", Type::kArray_Kind, *type, -1);
                fSymbolTable->takeOwnership((Type*) type);
                sizes.push_back(nullptr);
            }
        }
        auto var = std::unique_ptr<Variable>(new Variable(decl.fOffset, decl.fModifiers,
                                                          varDecl.fName, *type, storage));
        if (var->fName == Compiler::RTADJUST_NAME) {
            ASSERT(!fRTAdjust);
            ASSERT(var->fType == *fContext.fFloat4_Type);
            fRTAdjust = var.get();
        }
        std::unique_ptr<Expression> value;
        if (varDecl.fValue) {
            value = this->convertExpression(*varDecl.fValue);
            if (!value) {
                return nullptr;
            }
            value = this->coerce(std::move(value), *type);
            var->fWriteCount = 1;
            var->fInitialValue = value.get();
        }
        if (storage == Variable::kGlobal_Storage && varDecl.fName == "sk_FragColor" &&
            (*fSymbolTable)[varDecl.fName]) {
            // already defined, ignore
        } else if (storage == Variable::kGlobal_Storage && (*fSymbolTable)[varDecl.fName] &&
                   (*fSymbolTable)[varDecl.fName]->fKind == Symbol::kVariable_Kind &&
                   ((Variable*) (*fSymbolTable)[varDecl.fName])->fModifiers.fLayout.fBuiltin >= 0) {
            // already defined, just update the modifiers
            Variable* old = (Variable*) (*fSymbolTable)[varDecl.fName];
            old->fModifiers = var->fModifiers;
        } else {
            variables.emplace_back(new VarDeclaration(var.get(), std::move(sizes),
                                                      std::move(value)));
            fSymbolTable->add(varDecl.fName, std::move(var));
        }
    }
    return std::unique_ptr<VarDeclarations>(new VarDeclarations(decl.fOffset,
                                                                baseType,
                                                                std::move(variables)));
}

std::unique_ptr<ModifiersDeclaration> IRGenerator::convertModifiersDeclaration(
                                                                 const ASTModifiersDeclaration& m) {
    Modifiers modifiers = m.fModifiers;
    if (modifiers.fLayout.fInvocations != -1) {
        fInvocations = modifiers.fLayout.fInvocations;
        if (fSettings->fCaps && !fSettings->fCaps->gsInvocationsSupport()) {
            modifiers.fLayout.fInvocations = -1;
            Variable* invocationId = (Variable*) (*fSymbolTable)["sk_InvocationID"];
            ASSERT(invocationId);
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
            return std::unique_ptr<Statement>(new Block(s.fOffset, std::move(empty),
                                                        fSymbolTable));
        }
    }
    return std::unique_ptr<Statement>(new IfStatement(s.fOffset, s.fIsStatic, std::move(test),
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
    return std::unique_ptr<Statement>(new ForStatement(f.fOffset, std::move(initializer),
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
    return std::unique_ptr<Statement>(new WhileStatement(w.fOffset, std::move(test),
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
    return std::unique_ptr<Statement>(new DoStatement(d.fOffset, std::move(statement),
                                                      std::move(test)));
}

std::unique_ptr<Statement> IRGenerator::convertSwitch(const ASTSwitchStatement& s) {
    AutoSwitchLevel level(this);
    std::unique_ptr<Expression> value = this->convertExpression(*s.fValue);
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
    for (const auto& c : s.fCases) {
        std::unique_ptr<Expression> caseValue;
        if (c->fValue) {
            caseValue = this->convertExpression(*c->fValue);
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
        std::vector<std::unique_ptr<Statement>> statements;
        for (const auto& s : c->fStatements) {
            std::unique_ptr<Statement> converted = this->convertStatement(*s);
            if (!converted) {
                return nullptr;
            }
            statements.push_back(std::move(converted));
        }
        cases.emplace_back(new SwitchCase(c->fOffset, std::move(caseValue),
                                          std::move(statements)));
    }
    return std::unique_ptr<Statement>(new SwitchStatement(s.fOffset, s.fIsStatic,
                                                          std::move(value), std::move(cases),
                                                          fSymbolTable));
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
    // early returns from a vertex main function will bypass the sk_Position normalization, so
    // assert that we aren't doing that. It is of course possible to fix this by adding a
    // normalization before each return, but it will probably never actually be necessary.
    ASSERT(Program::kVertex_Kind != fKind || !fRTAdjust || "main" != fCurrentFunction->fName);
    if (r.fExpression) {
        std::unique_ptr<Expression> result = this->convertExpression(*r.fExpression);
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

std::unique_ptr<Statement> IRGenerator::convertBreak(const ASTBreakStatement& b) {
    if (fLoopLevel > 0 || fSwitchLevel > 0) {
        return std::unique_ptr<Statement>(new BreakStatement(b.fOffset));
    } else {
        fErrors.error(b.fOffset, "break statement must be inside a loop or switch");
        return nullptr;
    }
}

std::unique_ptr<Statement> IRGenerator::convertContinue(const ASTContinueStatement& c) {
    if (fLoopLevel > 0) {
        return std::unique_ptr<Statement>(new ContinueStatement(c.fOffset));
    } else {
        fErrors.error(c.fOffset, "continue statement must be inside a loop");
        return nullptr;
    }
}

std::unique_ptr<Statement> IRGenerator::convertDiscard(const ASTDiscardStatement& d) {
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
    ASSERT(loopIdx);
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
    ASTIdentifier endPrimitiveID = ASTIdentifier(-1, "EndPrimitive");
    std::unique_ptr<Expression> endPrimitive = this->convertExpression(endPrimitiveID);
    ASSERT(endPrimitive);

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
    // sk_Position = float4(sk_Position.x * rtAdjust.x + sk_Position.w * rtAdjust.y,
    //                      sk_Position.y * rtAdjust.z + sk_Position.w * rtAdjust.w,
    //                      0,
    //                      sk_Position.w);
    ASSERT(fSkPerVertex && fRTAdjust);
    #define REF(var) std::unique_ptr<Expression>(\
                                  new VariableReference(-1, *var, VariableReference::kRead_RefKind))
    #define FIELD(var, idx) std::unique_ptr<Expression>(\
                    new FieldAccess(REF(var), idx, FieldAccess::kAnonymousInterfaceBlock_OwnerKind))
    #define POS std::unique_ptr<Expression>(new FieldAccess(REF(fSkPerVertex), 0, \
                                                   FieldAccess::kAnonymousInterfaceBlock_OwnerKind))
    #define ADJUST (fRTAdjustInterfaceBlock ? \
                    FIELD(fRTAdjustInterfaceBlock, fRTAdjustFieldIndex) : \
                    REF(fRTAdjust))
    #define SWIZZLE(expr, field) std::unique_ptr<Expression>(new Swizzle(fContext, expr, { field }))
    #define OP(left, op, right) std::unique_ptr<Expression>(\
                                   new BinaryExpression(-1, left, op, right, *fContext.fFloat_Type))
    std::vector<std::unique_ptr<Expression>> children;
    children.push_back(OP(OP(SWIZZLE(POS, 0), Token::STAR, SWIZZLE(ADJUST, 0)),
                          Token::PLUS,
                          OP(SWIZZLE(POS, 3), Token::STAR, SWIZZLE(ADJUST, 1))));
    children.push_back(OP(OP(SWIZZLE(POS, 1), Token::STAR, SWIZZLE(ADJUST, 2)),
                          Token::PLUS,
                          OP(SWIZZLE(POS, 3), Token::STAR, SWIZZLE(ADJUST, 3))));
    children.push_back(std::unique_ptr<Expression>(new FloatLiteral(fContext, -1, 0.0)));
    children.push_back(SWIZZLE(POS, 3));
    std::unique_ptr<Expression> result = OP(POS, Token::EQ,
                                 std::unique_ptr<Expression>(new Constructor(-1,
                                                                             *fContext.fFloat4_Type,
                                                                             std::move(children))));
    return std::unique_ptr<Statement>(new ExpressionStatement(std::move(result)));
}

void IRGenerator::convertFunction(const ASTFunction& f) {
    const Type* returnType = this->convertType(*f.fReturnType);
    if (!returnType) {
        return;
    }
    std::vector<const Variable*> parameters;
    for (const auto& param : f.fParameters) {
        const Type* type = this->convertType(*param->fType);
        if (!type) {
            return;
        }
        for (int j = (int) param->fSizes.size() - 1; j >= 0; j--) {
            int size = param->fSizes[j];
            String name = type->name() + "[" + to_string(size) + "]";
            Type* newType = new Type(std::move(name), Type::kArray_Kind, *type, size);
            fSymbolTable->takeOwnership(newType);
            type = newType;
        }
        StringFragment name = param->fName;
        Variable* var = new Variable(param->fOffset, param->fModifiers, name, *type,
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
                fErrors.error(f.fOffset, "symbol '" + f.fName + "' was already defined");
                return;
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
                        FunctionDeclaration newDecl(f.fOffset, f.fModifiers, f.fName, parameters,
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
                    if (other->fDefined) {
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
                                                                                    f.fModifiers,
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
        bool needInvocationIDWorkaround = fInvocations != -1 && f.fName == "main" &&
                                          fSettings->fCaps &&
                                          !fSettings->fCaps->gsInvocationsSupport();
        ASSERT(!fExtraVars.size());
        std::unique_ptr<Block> body = this->convertBlock(*f.fBody);
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
        if (Program::kVertex_Kind == fKind && f.fName == "main" && fRTAdjust) {
            body->fStatements.insert(body->fStatements.end(), this->getNormalizeSkPositionCode());
        }
        fProgramElements->push_back(std::unique_ptr<FunctionDefinition>(
                                        new FunctionDefinition(f.fOffset, *decl, std::move(body))));
    }
}

std::unique_ptr<InterfaceBlock> IRGenerator::convertInterfaceBlock(const ASTInterfaceBlock& intf) {
    std::shared_ptr<SymbolTable> old = fSymbolTable;
    AutoSymbolTable table(this);
    std::vector<Type::Field> fields;
    bool haveRuntimeArray = false;
    bool foundRTAdjust = false;
    for (size_t i = 0; i < intf.fDeclarations.size(); i++) {
        std::unique_ptr<VarDeclarations> decl = this->convertVarDeclarations(
                                                                         *intf.fDeclarations[i],
                                                                         Variable::kGlobal_Storage);
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
                ASSERT(vd.fVar->fType == *fContext.fFloat4_Type);
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
    Type* type = new Type(intf.fOffset, intf.fTypeName, fields);
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
                    fErrors.error(converted->fOffset, "array size must be positive");
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
            type = new Type(type->name() + "[]", Type::kArray_Kind, *type, -1);
            fSymbolTable->takeOwnership((Type*) type);
            sizes.push_back(nullptr);
        }
    }
    Variable* var = new Variable(intf.fOffset, intf.fModifiers,
                                 intf.fInstanceName.fLength ? intf.fInstanceName : intf.fTypeName,
                                 *type, Variable::kGlobal_Storage);
    if (foundRTAdjust) {
        fRTAdjustInterfaceBlock = var;
    }
    old->takeOwnership(var);
    if (intf.fInstanceName.fLength) {
        old->addWithoutOwnership(intf.fInstanceName, var);
    } else {
        for (size_t i = 0; i < fields.size(); i++) {
            old->add(fields[i].fName, std::unique_ptr<Field>(new Field(intf.fOffset, *var,
                                                                       (int) i)));
        }
    }
    if (var->fName == Compiler::PERVERTEX_NAME) {
        ASSERT(!fSkPerVertex);
        fSkPerVertex = var;
    }
    return std::unique_ptr<InterfaceBlock>(new InterfaceBlock(intf.fOffset,
                                                              var,
                                                              intf.fTypeName,
                                                              intf.fInstanceName,
                                                              std::move(sizes),
                                                              fSymbolTable));
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

void IRGenerator::convertEnum(const ASTEnum& e) {
    std::vector<Variable*> variables;
    int64_t currentValue = 0;
    Layout layout;
    ASTType enumType(e.fOffset, e.fTypeName, ASTType::kIdentifier_Kind, {});
    const Type* type = this->convertType(enumType);
    Modifiers modifiers(layout, Modifiers::kConst_Flag);
    std::shared_ptr<SymbolTable> symbols(new SymbolTable(fSymbolTable, &fErrors));
    fSymbolTable = symbols;
    for (size_t i = 0; i < e.fNames.size(); i++) {
        std::unique_ptr<Expression> value;
        if (e.fValues[i]) {
            value = this->convertExpression(*e.fValues[i]);
            if (!value) {
                fSymbolTable = symbols->fParent;
                return;
            }
            this->getConstantInt(*value, &currentValue);
        }
        value = std::unique_ptr<Expression>(new IntLiteral(fContext, e.fOffset, currentValue));
        ++currentValue;
        auto var = std::unique_ptr<Variable>(new Variable(e.fOffset, modifiers, e.fNames[i],
                                                          *type, Variable::kGlobal_Storage,
                                                          value.get()));
        variables.push_back(var.get());
        symbols->add(e.fNames[i], std::move(var));
        symbols->takeOwnership(value.release());
    }
    fProgramElements->push_back(std::unique_ptr<ProgramElement>(new Enum(e.fOffset, e.fTypeName,
                                                                         symbols)));
    fSymbolTable = symbols->fParent;
}

const Type* IRGenerator::convertType(const ASTType& type) {
    const Symbol* result = (*fSymbolTable)[type.fName];
    if (result && result->fKind == Symbol::kType_Kind) {
        for (int size : type.fSizes) {
            String name(result->fName);
            name += "[";
            if (size != -1) {
                name += to_string(size);
            }
            name += "]";
            result = new Type(name, Type::kArray_Kind, (const Type&) *result, size);
            fSymbolTable->takeOwnership((Type*) result);
        }
        return (const Type*) result;
    }
    fErrors.error(type.fOffset, "unknown type '" + type.fName + "'");
    return nullptr;
}

std::unique_ptr<Expression> IRGenerator::convertExpression(const ASTExpression& expr) {
    switch (expr.fKind) {
        case ASTExpression::kIdentifier_Kind:
            return this->convertIdentifier((ASTIdentifier&) expr);
        case ASTExpression::kBool_Kind:
            return std::unique_ptr<Expression>(new BoolLiteral(fContext, expr.fOffset,
                                                               ((ASTBoolLiteral&) expr).fValue));
        case ASTExpression::kInt_Kind:
            return std::unique_ptr<Expression>(new IntLiteral(fContext, expr.fOffset,
                                                              ((ASTIntLiteral&) expr).fValue));
        case ASTExpression::kFloat_Kind:
            return std::unique_ptr<Expression>(new FloatLiteral(fContext, expr.fOffset,
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
        fErrors.error(identifier.fOffset, "unknown identifier '" + identifier.fText + "'");
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
#ifndef SKSL_STANDALONE
            if (var->fModifiers.fLayout.fBuiltin == SK_FRAGCOORD_BUILTIN) {
                fInputs.fFlipY = true;
                if (fSettings->fFlipY &&
                    (!fSettings->fCaps ||
                     !fSettings->fCaps->fragCoordConventionsExtensionString())) {
                    fInputs.fRTHeight = true;
                }
            }
#endif
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
        default:
            ABORT("unsupported symbol type %d\n", result->fKind);
    }
}

std::unique_ptr<Section> IRGenerator::convertSection(const ASTSection& s) {
    return std::unique_ptr<Section>(new Section(s.fOffset, s.fName, s.fArgument, s.fText));
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
        ASTIdentifier id(-1, type.fName);
        std::unique_ptr<Expression> ctor = this->convertIdentifier(id);
        ASSERT(ctor);
        return this->call(-1, std::move(ctor), std::move(args));
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
            if (left == right) {
                *outLeftType = &left;
                *outRightType = &right;
                *outResultType = context.fBool_Type.get();
                return true;
            }
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

std::unique_ptr<Expression> IRGenerator::constantFold(const Expression& left,
                                                      Token::Kind op,
                                                      const Expression& right) const {
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
    if (left.fType.kind() == Type::kVector_Kind &&
        left.fType.componentType() == *fContext.fFloat_Type &&
        left.fType == right.fType) {
        ASSERT(left.fKind  == Expression::kConstructor_Kind);
        ASSERT(right.fKind == Expression::kConstructor_Kind);
        std::vector<std::unique_ptr<Expression>> args;
        #define RETURN_VEC_COMPONENTWISE_RESULT(op)                                    \
            for (int i = 0; i < left.fType.columns(); i++) {                           \
                float value = ((Constructor&) left).getFVecComponent(i) op             \
                              ((Constructor&) right).getFVecComponent(i);              \
                args.emplace_back(new FloatLiteral(fContext, -1, value));              \
            }                                                                          \
            return std::unique_ptr<Expression>(new Constructor(-1, left.fType,         \
                                                               std::move(args)));
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
            case Token::SLASH: RETURN_VEC_COMPONENTWISE_RESULT(/);
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
    if (!determine_binary_type(fContext, expression.fOperator, *rawLeftType, *rawRightType,
                               &leftType, &rightType, &resultType,
                               !Compiler::IsAssignment(expression.fOperator))) {
        fErrors.error(expression.fOffset, String("type mismatch: '") +
                                          Compiler::OperatorName(expression.fOperator) +
                                          "' cannot operate on '" + left->fType.fName +
                                          "', '" + right->fType.fName + "'");
        return nullptr;
    }
    if (Compiler::IsAssignment(expression.fOperator)) {
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
        result = std::unique_ptr<Expression>(new BinaryExpression(expression.fOffset,
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
        fErrors.error(expression.fOffset, "ternary operator result mismatch: '" +
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
    return std::unique_ptr<Expression>(new TernaryExpression(expression.fOffset,
                                                             std::move(test),
                                                             std::move(ifTrue),
                                                             std::move(ifFalse)));
}

// scales the texture coordinates by the texture size for sampling rectangle textures.
// For float2coordinates, implements the transformation:
//     texture(sampler, coord) -> texture(sampler, textureSize(sampler) * coord)
// For float3coordinates, implements the transformation:
//     texture(sampler, coord) -> texture(sampler, float3textureSize(sampler), 1.0) * coord))
void IRGenerator::fixRectSampling(std::vector<std::unique_ptr<Expression>>& arguments) {
    ASSERT(arguments.size() == 2);
    ASSERT(arguments[0]->fType == *fContext.fSampler2DRect_Type);
    ASSERT(arguments[0]->fKind == Expression::kVariableReference_Kind);
    const Variable& sampler = ((VariableReference&) *arguments[0]).fVariable;
    const Symbol* textureSizeSymbol = (*fSymbolTable)["textureSize"];
    ASSERT(textureSizeSymbol->fKind == Symbol::kFunctionDeclaration_Kind);
    const FunctionDeclaration& textureSize = (FunctionDeclaration&) *textureSizeSymbol;
    std::vector<std::unique_ptr<Expression>> sizeArguments;
    sizeArguments.emplace_back(new VariableReference(-1, sampler));
    std::unique_ptr<Expression> float2ize = call(-1, textureSize, std::move(sizeArguments));
    const Type& type = arguments[1]->fType;
    std::unique_ptr<Expression> scale;
    if (type == *fContext.fFloat2_Type) {
        scale = std::move(float2ize);
    } else {
        ASSERT(type == *fContext.fFloat3_Type);
        std::vector<std::unique_ptr<Expression>> float3rguments;
        float3rguments.push_back(std::move(float2ize));
        float3rguments.emplace_back(new FloatLiteral(fContext, -1, 1.0));
        scale.reset(new Constructor(-1, *fContext.fFloat3_Type, std::move(float3rguments)));
    }
    arguments[1].reset(new BinaryExpression(-1, std::move(scale), Token::STAR,
                                            std::move(arguments[1]), type));
}

std::unique_ptr<Expression> IRGenerator::call(int offset,
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
            this->markWrittenTo(*arguments[i],
                                function.fParameters[i]->fModifiers.fFlags & Modifiers::kIn_Flag);
        }
    }
    if (function.fBuiltin && function.fName == "texture" &&
        arguments[0]->fType == *fContext.fSampler2DRect_Type) {
        this->fixRectSampling(arguments);
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
    if (functionValue->fKind == Expression::kTypeReference_Kind) {
        return this->convertConstructor(offset,
                                        ((TypeReference&) *functionValue).fValue,
                                        std::move(arguments));
    }
    if (functionValue->fKind != Expression::kFunctionReference_Kind) {
        fErrors.error(offset, "'" + functionValue->description() + "' is not a function");
        return nullptr;
    }
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

std::unique_ptr<Expression> IRGenerator::convertNumberConstructor(
                                                    int offset,
                                                    const Type& type,
                                                    std::vector<std::unique_ptr<Expression>> args) {
    ASSERT(type.isNumber());
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
        return std::unique_ptr<Expression>(new FloatLiteral(fContext, offset, value, &type));
    }
    if (type.isFloat() && args.size() == 1 && args[0]->fKind == Expression::kIntLiteral_Kind) {
        int64_t value = ((IntLiteral&) *args[0]).fValue;
        return std::unique_ptr<Expression>(new FloatLiteral(fContext, offset, (double) value,
                                                            &type));
    }
    if (args[0]->fKind == Expression::kIntLiteral_Kind && (type == *fContext.fInt_Type ||
        type == *fContext.fUInt_Type)) {
        return std::unique_ptr<Expression>(new IntLiteral(fContext,
                                                          offset,
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
    ASSERT(type.kind() == Type::kVector_Kind || type.kind() == Type::kMatrix_Kind);
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

std::unique_ptr<Expression> IRGenerator::convertPrefixExpression(
                                                            const ASTPrefixExpression& expression) {
    std::unique_ptr<Expression> base = this->convertExpression(*expression.fOperand);
    if (!base) {
        return nullptr;
    }
    switch (expression.fOperator) {
        case Token::PLUS:
            if (!base->fType.isNumber() && base->fType.kind() != Type::kVector_Kind) {
                fErrors.error(expression.fOffset,
                              "'+' cannot operate on '" + base->fType.description() + "'");
                return nullptr;
            }
            return base;
        case Token::MINUS:
            if (!base->fType.isNumber() && base->fType.kind() != Type::kVector_Kind) {
                fErrors.error(expression.fOffset,
                              "'-' cannot operate on '" + base->fType.description() + "'");
                return nullptr;
            }
            if (base->fKind == Expression::kIntLiteral_Kind) {
                return std::unique_ptr<Expression>(new IntLiteral(fContext, base->fOffset,
                                                                  -((IntLiteral&) *base).fValue));
            }
            if (base->fKind == Expression::kFloatLiteral_Kind) {
                double value = -((FloatLiteral&) *base).fValue;
                return std::unique_ptr<Expression>(new FloatLiteral(fContext, base->fOffset,
                                                                    value));
            }
            return std::unique_ptr<Expression>(new PrefixExpression(Token::MINUS, std::move(base)));
        case Token::PLUSPLUS:
            if (!base->fType.isNumber()) {
                fErrors.error(expression.fOffset,
                              String("'") + Compiler::OperatorName(expression.fOperator) +
                              "' cannot operate on '" + base->fType.description() + "'");
                return nullptr;
            }
            this->markWrittenTo(*base, true);
            break;
        case Token::MINUSMINUS:
            if (!base->fType.isNumber()) {
                fErrors.error(expression.fOffset,
                              String("'") + Compiler::OperatorName(expression.fOperator) +
                              "' cannot operate on '" + base->fType.description() + "'");
                return nullptr;
            }
            this->markWrittenTo(*base, true);
            break;
        case Token::LOGICALNOT:
            if (base->fType != *fContext.fBool_Type) {
                fErrors.error(expression.fOffset,
                              String("'") + Compiler::OperatorName(expression.fOperator) +
                              "' cannot operate on '" + base->fType.description() + "'");
                return nullptr;
            }
            if (base->fKind == Expression::kBoolLiteral_Kind) {
                return std::unique_ptr<Expression>(new BoolLiteral(fContext, base->fOffset,
                                                                   !((BoolLiteral&) *base).fValue));
            }
            break;
        case Token::BITWISENOT:
            if (base->fType != *fContext.fInt_Type) {
                fErrors.error(expression.fOffset,
                              String("'") + Compiler::OperatorName(expression.fOperator) +
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
                fErrors.error(base->fOffset, String::printf("invalid swizzle component '%c'",
                                                            fields[i]));
                return nullptr;
        }
    }
    ASSERT(swizzleComponents.size() > 0);
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

std::unique_ptr<Expression> IRGenerator::getArg(int offset, String name) {
    auto found = fSettings->fArgs.find(name);
    if (found == fSettings->fArgs.end()) {
        fErrors.error(offset, "unknown argument '" + name + "'");
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
            result = convertIdentifier(ASTIdentifier(offset, field));
            fSymbolTable = old;
        }
    }
    if (!result) {
        fErrors.error(offset, "type '" + type.fName + "' does not have a field named '" + field +
                              "'");
    }
    return result;
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
                return std::unique_ptr<Expression>(new TypeReference(fContext, base->fOffset,
                                                                     *newType));
            } else {
                fErrors.error(expression.fOffset, "'[]' must follow a type name");
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
            return this->call(expression.fOffset, std::move(base), std::move(arguments));
        }
        case ASTSuffix::kField_Kind: {
            StringFragment field = ((ASTFieldSuffix&) *expression.fSuffix).fField;
            if (base->fType == *fContext.fSkCaps_Type) {
                return this->getCap(expression.fOffset, field);
            }
            if (base->fType == *fContext.fSkArgs_Type) {
                return this->getArg(expression.fOffset, field);
            }
            if (base->fKind == Expression::kTypeReference_Kind) {
                return this->convertTypeField(base->fOffset, ((TypeReference&) *base).fValue,
                                              field);
            }
            switch (base->fType.kind()) {
                case Type::kVector_Kind:
                    return this->convertSwizzle(std::move(base), field);
                case Type::kStruct_Kind:
                    return this->convertField(std::move(base), field);
                default:
                    fErrors.error(base->fOffset, "cannot swizzle value of type '" +
                                                 base->fType.description() + "'");
                    return nullptr;
            }
        }
        case ASTSuffix::kPostIncrement_Kind:
            if (!base->fType.isNumber()) {
                fErrors.error(expression.fOffset,
                              "'++' cannot operate on '" + base->fType.description() + "'");
                return nullptr;
            }
            this->markWrittenTo(*base, true);
            return std::unique_ptr<Expression>(new PostfixExpression(std::move(base),
                                                                     Token::PLUSPLUS));
        case ASTSuffix::kPostDecrement_Kind:
            if (!base->fType.isNumber()) {
                fErrors.error(expression.fOffset,
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
                fErrors.error(expr.fOffset,
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
                fErrors.error(expr.fOffset,
                              "cannot write to the same swizzle field more than once");
            }
            this->markWrittenTo(*((Swizzle&) expr).fBase, readWrite);
            break;
        case Expression::kIndex_Kind:
            this->markWrittenTo(*((IndexExpression&) expr).fBase, readWrite);
            break;
        case Expression::kTernary_Kind: {
            TernaryExpression& t = (TernaryExpression&) expr;
            this->markWrittenTo(*t.fIfTrue, readWrite);
            this->markWrittenTo(*t.fIfFalse, readWrite);
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
    std::vector<std::unique_ptr<ASTDeclaration>> parsed = parser.file();
    if (fErrors.errorCount()) {
        return;
    }
    for (size_t i = 0; i < parsed.size(); i++) {
        ASTDeclaration& decl = *parsed[i];
        switch (decl.fKind) {
            case ASTDeclaration::kVar_Kind: {
                std::unique_ptr<VarDeclarations> s = this->convertVarDeclarations(
                                                                         (ASTVarDeclarations&) decl,
                                                                         Variable::kGlobal_Storage);
                if (s) {
                    fProgramElements->push_back(std::move(s));
                }
                break;
            }
            case ASTDeclaration::kEnum_Kind: {
                this->convertEnum((ASTEnum&) decl);
                break;
            }
            case ASTDeclaration::kFunction_Kind: {
                this->convertFunction((ASTFunction&) decl);
                break;
            }
            case ASTDeclaration::kModifiers_Kind: {
                std::unique_ptr<ModifiersDeclaration> f = this->convertModifiersDeclaration(
                                                                   (ASTModifiersDeclaration&) decl);
                if (f) {
                    fProgramElements->push_back(std::move(f));
                }
                break;
            }
            case ASTDeclaration::kInterfaceBlock_Kind: {
                std::unique_ptr<InterfaceBlock> i = this->convertInterfaceBlock(
                                                                         (ASTInterfaceBlock&) decl);
                if (i) {
                    fProgramElements->push_back(std::move(i));
                }
                break;
            }
            case ASTDeclaration::kExtension_Kind: {
                std::unique_ptr<Extension> e = this->convertExtension((ASTExtension&) decl);
                if (e) {
                    fProgramElements->push_back(std::move(e));
                }
                break;
            }
            case ASTDeclaration::kSection_Kind: {
                std::unique_ptr<Section> s = this->convertSection((ASTSection&) decl);
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
