/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLIRGenerator.h"

#include "limits.h"
#include <memory>
#include <unordered_set>

#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLParser.h"
#include "src/sksl/SkSLUtil.h"
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

class AutoDisableInline {
public:
    AutoDisableInline(IRGenerator* ir, bool canInline = false)
    : fIR(ir) {
        fOldCanInline = ir->fCanInline;
        fIR->fCanInline &= canInline;
    }

    ~AutoDisableInline() {
        fIR->fCanInline = fOldCanInline;
    }

    IRGenerator* fIR;
    bool fOldCanInline;
};

IRGenerator::IRGenerator(const Context* context, std::shared_ptr<SymbolTable> symbolTable,
                         ErrorReporter& errorReporter)
: fContext(*context)
, fCurrentFunction(nullptr)
, fRootSymbolTable(symbolTable)
, fSymbolTable(symbolTable)
, fLoopLevel(0)
, fSwitchLevel(0)
, fErrors(errorReporter) {}

void IRGenerator::pushSymbolTable() {
    fSymbolTable.reset(new SymbolTable(std::move(fSymbolTable)));
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
    CAP(mustGuardDivisionEvenAfterExplicitZeroCheck);
    CAP(inBlendModesFailRandomlyForAllZeroVec);
    CAP(atan2ImplementedAsAtanYOverX);
    CAP(canUseAnyFunctionInShader);
    CAP(floatIs32Bits);
    CAP(integerSupport);
#undef CAP
}

void IRGenerator::start(const Program::Settings* settings,
                        std::vector<std::unique_ptr<ProgramElement>>* inherited,
                        bool isBuiltinCode) {
    fSettings = settings;
    fInherited = inherited;
    fIsBuiltinCode = isBuiltinCode;
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
    fInlineVarCounter = 0;
    if (inherited) {
        for (const auto& e : *inherited) {
            if (e->fKind == ProgramElement::kInterfaceBlock_Kind) {
                InterfaceBlock& intf = e->as<InterfaceBlock>();
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
    return std::make_unique<Extension>(offset, name);
}

void IRGenerator::finish() {
    this->popSymbolTable();
    fSettings = nullptr;
}

std::unique_ptr<Statement> IRGenerator::convertSingleStatement(const ASTNode& statement) {
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
                Expression& expr = *result->as<ExpressionStatement>().fExpression;
                if (expr.fKind == Expression::kFunctionCall_Kind) {
                    FunctionCall& fc = expr.as<FunctionCall>();
                    if (fc.fFunction.fBuiltin && fc.fFunction.fName == "EmitVertex") {
                        std::vector<std::unique_ptr<Statement>> statements;
                        statements.push_back(getNormalizeSkPositionCode());
                        statements.push_back(std::move(result));
                        return std::make_unique<Block>(statement.fOffset, std::move(statements),
                                                       fSymbolTable);
                    }
                }
            }
            return result;
    }
}

std::unique_ptr<Statement> IRGenerator::convertStatement(const ASTNode& statement) {
    std::vector<std::unique_ptr<Statement>> oldExtraStatements = std::move(fExtraStatements);
    std::unique_ptr<Statement> result = this->convertSingleStatement(statement);
    if (!result) {
        fExtraStatements = std::move(oldExtraStatements);
        return nullptr;
    }
    if (fExtraStatements.size()) {
        fExtraStatements.push_back(std::move(result));
        std::unique_ptr<Statement> block(new Block(-1, std::move(fExtraStatements), nullptr,
                                                   false));
        fExtraStatements = std::move(oldExtraStatements);
        return block;
    }
    fExtraStatements = std::move(oldExtraStatements);
    return result;
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
    return std::make_unique<Block>(block.fOffset, std::move(statements), fSymbolTable);
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
    if (baseType->nonnullable() == *fContext.fFragmentProcessor_Type &&
        storage != Variable::kGlobal_Storage) {
        fErrors.error(decls.fOffset,
                      "variables of type '" + baseType->displayName() + "' must be global");
    }
    if (fKind != Program::kFragmentProcessor_Kind) {
        if ((modifiers.fFlags & Modifiers::kIn_Flag) &&
            baseType->kind() == Type::Kind::kMatrix_Kind) {
            fErrors.error(decls.fOffset, "'in' variables may not have matrix type");
        }
        if ((modifiers.fFlags & Modifiers::kIn_Flag) &&
            (modifiers.fFlags & Modifiers::kUniform_Flag)) {
            fErrors.error(decls.fOffset,
                          "'in uniform' variables only permitted within fragment processors");
        }
        if (modifiers.fLayout.fWhen.fLength) {
            fErrors.error(decls.fOffset, "'when' is only permitted within fragment processors");
        }
        if (modifiers.fLayout.fFlags & Layout::kTracked_Flag) {
            fErrors.error(decls.fOffset, "'tracked' is only permitted within fragment processors");
        }
        if (modifiers.fLayout.fCType != Layout::CType::kDefault) {
            fErrors.error(decls.fOffset, "'ctype' is only permitted within fragment processors");
        }
        if (modifiers.fLayout.fKey) {
            fErrors.error(decls.fOffset, "'key' is only permitted within fragment processors");
        }
    }
    if (fKind == Program::kPipelineStage_Kind) {
        if ((modifiers.fFlags & Modifiers::kIn_Flag) &&
            baseType->nonnullable() != *fContext.fFragmentProcessor_Type) {
            fErrors.error(decls.fOffset, "'in' variables not permitted in runtime effects");
        }
    }
    if (modifiers.fLayout.fKey && (modifiers.fFlags & Modifiers::kUniform_Flag)) {
        fErrors.error(decls.fOffset, "'key' is not permitted on 'uniform' variables");
    }
    if (modifiers.fLayout.fMarker.fLength) {
        if (fKind != Program::kPipelineStage_Kind) {
            fErrors.error(decls.fOffset, "'marker' is only permitted in runtime effects");
        }
        if (!(modifiers.fFlags & Modifiers::kUniform_Flag)) {
            fErrors.error(decls.fOffset, "'marker' is only permitted on 'uniform' variables");
        }
        if (*baseType != *fContext.fFloat4x4_Type) {
            fErrors.error(decls.fOffset, "'marker' is only permitted on float4x4 variables");
        }
    }
    if (modifiers.fLayout.fFlags & Layout::kSRGBUnpremul_Flag) {
        if (fKind != Program::kPipelineStage_Kind) {
            fErrors.error(decls.fOffset, "'srgb_unpremul' is only permitted in runtime effects");
        }
        if (!(modifiers.fFlags & Modifiers::kUniform_Flag)) {
            fErrors.error(decls.fOffset,
                          "'srgb_unpremul' is only permitted on 'uniform' variables");
        }
        auto validColorXformType = [](const Type& t) {
            return t.kind() == Type::kVector_Kind && t.componentType().isFloat() &&
                   (t.columns() == 3 || t.columns() == 4);
        };
        if (!validColorXformType(*baseType) && !(baseType->kind() == Type::kArray_Kind &&
                                                 validColorXformType(baseType->componentType()))) {
            fErrors.error(decls.fOffset,
                          "'srgb_unpremul' is only permitted on half3, half4, float3, or float4 "
                          "variables");
        }
    }
    if (modifiers.fFlags & Modifiers::kVarying_Flag) {
        if (fKind != Program::kPipelineStage_Kind) {
            fErrors.error(decls.fOffset, "'varying' is only permitted in runtime effects");
        }
        if (!baseType->isFloat() &&
            !(baseType->kind() == Type::kVector_Kind && baseType->componentType().isFloat())) {
            fErrors.error(decls.fOffset, "'varying' must be float scalar or vector");
        }
    }
    int permitted = Modifiers::kConst_Flag;
    if (storage == Variable::kGlobal_Storage) {
        permitted |= Modifiers::kIn_Flag | Modifiers::kOut_Flag | Modifiers::kUniform_Flag |
                     Modifiers::kFlat_Flag | Modifiers::kVarying_Flag |
                     Modifiers::kNoPerspective_Flag | Modifiers::kPLS_Flag |
                     Modifiers::kPLSIn_Flag | Modifiers::kPLSOut_Flag |
                     Modifiers::kRestrict_Flag | Modifiers::kVolatile_Flag |
                     Modifiers::kReadOnly_Flag | Modifiers::kWriteOnly_Flag |
                     Modifiers::kCoherent_Flag | Modifiers::kBuffer_Flag;
    }
    this->checkModifiers(decls.fOffset, modifiers, permitted);
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
                    count = size->as<IntLiteral>().fValue;
                    if (count <= 0) {
                        fErrors.error(size->fOffset, "array size must be positive");
                        return nullptr;
                    }
                    name += "[" + to_string(count) + "]";
                } else {
                    fErrors.error(size->fOffset, "array size must be specified");
                    return nullptr;
                }
                type = fSymbolTable->takeOwnershipOfSymbol(
                        std::make_unique<Type>(name, Type::kArray_Kind, *type, (int)count));
                sizes.push_back(std::move(size));
            } else {
                type = fSymbolTable->takeOwnershipOfSymbol(std::make_unique<Type>(
                        type->name() + "[]", Type::kArray_Kind, *type, /*columns=*/-1));
                sizes.push_back(nullptr);
            }
        }
        auto var = std::make_unique<Variable>(varDecl.fOffset, modifiers, varData.fName, *type,
                                              storage);
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
    return std::make_unique<VarDeclarations>(decls.fOffset, baseType, std::move(variables));
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
    return std::make_unique<ModifiersDeclaration>(modifiers);
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
        if (test->as<BoolLiteral>().fValue) {
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
        AutoDisableInline disableInline(this);
        test = this->coerce(this->convertExpression(*iter), *fContext.fBool_Type);
        if (!test) {
            return nullptr;
        }

    }
    ++iter;
    std::unique_ptr<Expression> next;
    if (*iter) {
        AutoDisableInline disableInline(this);
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
    return std::make_unique<ForStatement>(f.fOffset, std::move(initializer), std::move(test),
                                          std::move(next), std::move(statement), fSymbolTable);
}

std::unique_ptr<Statement> IRGenerator::convertWhile(const ASTNode& w) {
    SkASSERT(w.fKind == ASTNode::Kind::kWhile);
    AutoLoopLevel level(this);
    std::unique_ptr<Expression> test;
    auto iter = w.begin();
    {
        AutoDisableInline disableInline(this);
        test = this->coerce(this->convertExpression(*(iter++)), *fContext.fBool_Type);
    }
    if (!test) {
        return nullptr;
    }
    std::unique_ptr<Statement> statement = this->convertStatement(*(iter++));
    if (!statement) {
        return nullptr;
    }
    return std::make_unique<WhileStatement>(w.fOffset, std::move(test), std::move(statement));
}

std::unique_ptr<Statement> IRGenerator::convertDo(const ASTNode& d) {
    SkASSERT(d.fKind == ASTNode::Kind::kDo);
    AutoLoopLevel level(this);
    auto iter = d.begin();
    std::unique_ptr<Statement> statement = this->convertStatement(*(iter++));
    if (!statement) {
        return nullptr;
    }
    std::unique_ptr<Expression> test;
    {
        AutoDisableInline disableInline(this);
        test = this->coerce(this->convertExpression(*(iter++)), *fContext.fBool_Type);
    }
    if (!test) {
        return nullptr;
    }
    return std::make_unique<DoStatement>(d.fOffset, std::move(statement), std::move(test));
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
            int64_t v = 0;
            if (!this->getConstantInt(*caseValue, &v)) {
                fErrors.error(caseValue->fOffset, "case value must be a constant integer");
                return nullptr;
            }
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
                                     fCurrentFunction->fReturnType.displayName() + "'");
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
    const FunctionDeclaration* invokeDecl = fSymbolTable->add(
            "_invoke", std::make_unique<FunctionDeclaration>(/*offset=*/-1,
                                                             invokeModifiers,
                                                             "_invoke",
                                                             std::vector<const Variable*>(),
                                                             *fContext.fVoid_Type,
                                                             /*builtin=*/false));
    fProgramElements->push_back(std::make_unique<FunctionDefinition>(/*offset=*/-1,
                                                                     *invokeDecl,
                                                                     std::move(main)));

    std::vector<std::unique_ptr<VarDeclaration>> variables;
    const Variable* loopIdx = &(*fSymbolTable)["sk_InvocationID"]->as<Variable>();
    std::unique_ptr<Expression> test(new BinaryExpression(-1,
                    std::unique_ptr<Expression>(new VariableReference(-1, *loopIdx)),
                    Token::Kind::TK_LT,
                    std::make_unique<IntLiteral>(fContext, -1, fInvocations),
                    *fContext.fBool_Type));
    std::unique_ptr<Expression> next(new PostfixExpression(
                std::unique_ptr<Expression>(
                                      new VariableReference(-1,
                                                            *loopIdx,
                                                            VariableReference::kReadWrite_RefKind)),
                Token::Kind::TK_PLUSPLUS));
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
                    std::unique_ptr<Expression>(new VariableReference(-1, *loopIdx,
                                                                VariableReference::kWrite_RefKind)),
                    Token::Kind::TK_EQ,
                    std::make_unique<IntLiteral>(fContext, -1, 0),
                    *fContext.fInt_Type));
    std::unique_ptr<Statement> initializer(new ExpressionStatement(std::move(assignment)));
    std::unique_ptr<Statement> loop = std::unique_ptr<Statement>(
                new ForStatement(-1,
                                 std::move(initializer),
                                 std::move(test),
                                 std::move(next),
                                 std::make_unique<Block>(-1, std::move(loopBody)),
                                 fSymbolTable));
    std::vector<std::unique_ptr<Statement>> children;
    children.push_back(std::move(loop));
    return std::make_unique<Block>(-1, std::move(children));
}

std::unique_ptr<Statement> IRGenerator::getNormalizeSkPositionCode() {
    // sk_Position = float4(sk_Position.xy * rtAdjust.xz + sk_Position.ww * rtAdjust.yw,
    //                      0,
    //                      sk_Position.w);
    SkASSERT(fSkPerVertex && fRTAdjust);
    #define REF(var) std::unique_ptr<Expression>(\
                                  new VariableReference(-1, *var, VariableReference::kRead_RefKind))
    #define WREF(var) std::unique_ptr<Expression>(\
                                 new VariableReference(-1, *var, VariableReference::kWrite_RefKind))
    #define FIELD(var, idx) std::unique_ptr<Expression>(\
                    new FieldAccess(REF(var), idx, FieldAccess::kAnonymousInterfaceBlock_OwnerKind))
    #define POS std::unique_ptr<Expression>(new FieldAccess(WREF(fSkPerVertex), 0, \
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
    children.push_back(OP(OP(SWIZZLE(POS, 0, 1), Token::Kind::TK_STAR, SWIZZLE(ADJUST, 0, 2)),
                          Token::Kind::TK_PLUS,
                          OP(SWIZZLE(POS, 3, 3), Token::Kind::TK_STAR, SWIZZLE(ADJUST, 1, 3))));
    children.push_back(std::unique_ptr<Expression>(new FloatLiteral(fContext, -1, 0.0)));
    children.push_back(SWIZZLE(POS, 3));
    std::unique_ptr<Expression> result = OP(POS, Token::Kind::TK_EQ,
                                 std::unique_ptr<Expression>(new Constructor(-1,
                                                                             *fContext.fFloat4_Type,
                                                                             std::move(children))));
    return std::unique_ptr<Statement>(new ExpressionStatement(std::move(result)));
}

template<typename T>
class AutoClear {
public:
    AutoClear(T* container)
        : fContainer(container) {
        SkASSERT(container->empty());
    }

    ~AutoClear() {
        fContainer->clear();
    }

private:
    T* fContainer;
};

template <typename T> AutoClear(T* c) -> AutoClear<T>;

void IRGenerator::checkModifiers(int offset, const Modifiers& modifiers, int permitted) {
    int flags = modifiers.fFlags;
    #define CHECK(flag, name)                                              \
        if (!flags) return;                                                \
        if (flags & flag) {                                                \
            if (!(permitted & flag)) {                                     \
                fErrors.error(offset, "'" name "' is not permitted here"); \
            }                                                              \
            flags &= ~flag;                                                \
        }
    CHECK(Modifiers::kConst_Flag,          "const")
    CHECK(Modifiers::kIn_Flag,             "in")
    CHECK(Modifiers::kOut_Flag,            "out")
    CHECK(Modifiers::kUniform_Flag,        "uniform")
    CHECK(Modifiers::kFlat_Flag,           "flat")
    CHECK(Modifiers::kNoPerspective_Flag,  "noperspective")
    CHECK(Modifiers::kReadOnly_Flag,       "readonly")
    CHECK(Modifiers::kWriteOnly_Flag,      "writeonly")
    CHECK(Modifiers::kCoherent_Flag,       "coherent")
    CHECK(Modifiers::kVolatile_Flag,       "volatile")
    CHECK(Modifiers::kRestrict_Flag,       "restrict")
    CHECK(Modifiers::kBuffer_Flag,         "buffer")
    CHECK(Modifiers::kHasSideEffects_Flag, "sk_has_side_effects")
    CHECK(Modifiers::kPLS_Flag,            "__pixel_localEXT")
    CHECK(Modifiers::kPLSIn_Flag,          "__pixel_local_inEXT")
    CHECK(Modifiers::kPLSOut_Flag,         "__pixel_local_outEXT")
    CHECK(Modifiers::kVarying_Flag,        "varying")
    CHECK(Modifiers::kInline_Flag,         "inline")
    SkASSERT(flags == 0);
}

void IRGenerator::convertFunction(const ASTNode& f) {
    AutoClear clear(&fReferencedIntrinsics);
    auto iter = f.begin();
    const Type* returnType = this->convertType(*(iter++));
    if (returnType == nullptr) {
        return;
    }
    auto type_is_allowed = [&](const Type* t) {
#if defined(SKSL_STANDALONE)
        return true;
#else
        GrSLType unusedSLType;
        return fKind != Program::kPipelineStage_Kind ||
               type_to_grsltype(fContext, *t, &unusedSLType);
#endif
    };
    if (returnType->nonnullable() == *fContext.fFragmentProcessor_Type ||
        !type_is_allowed(returnType)) {
        fErrors.error(f.fOffset,
                      "functions may not return type '" + returnType->displayName() + "'");
        return;
    }
    const ASTNode::FunctionData& funcData = f.getFunctionData();
    this->checkModifiers(f.fOffset, funcData.fModifiers, Modifiers::kHasSideEffects_Flag |
                                                         Modifiers::kInline_Flag);
    std::vector<const Variable*> parameters;
    for (size_t i = 0; i < funcData.fParameterCount; ++i) {
        const ASTNode& param = *(iter++);
        SkASSERT(param.fKind == ASTNode::Kind::kParameter);
        ASTNode::ParameterData pd = param.getParameterData();
        this->checkModifiers(param.fOffset, pd.fModifiers, Modifiers::kIn_Flag |
                                                           Modifiers::kOut_Flag);
        auto paramIter = param.begin();
        const Type* type = this->convertType(*(paramIter++));
        if (!type) {
            return;
        }
        for (int j = (int) pd.fSizeCount; j >= 1; j--) {
            int size = (param.begin() + j)->getInt();
            String name = type->name() + "[" + to_string(size) + "]";
            type = fSymbolTable->takeOwnershipOfSymbol(
                    std::make_unique<Type>(std::move(name), Type::kArray_Kind, *type, size));
        }
        // Only the (builtin) declarations of 'sample' are allowed to have FP parameters
        if ((type->nonnullable() == *fContext.fFragmentProcessor_Type && !fIsBuiltinCode) ||
            !type_is_allowed(type)) {
            fErrors.error(param.fOffset,
                          "parameters of type '" + type->displayName() + "' not allowed");
            return;
        }
        StringFragment name = pd.fName;
        const Variable* var = fSymbolTable->takeOwnershipOfSymbol(std::make_unique<Variable>(
                param.fOffset, pd.fModifiers, name, *type, Variable::kParameter_Storage));
        parameters.push_back(var);
    }

    if (funcData.fName == "main") {
        switch (fKind) {
            case Program::kPipelineStage_Kind: {
                bool valid;
                switch (parameters.size()) {
                    case 2:
                        valid = parameters[0]->fType == *fContext.fFloat2_Type &&
                                parameters[0]->fModifiers.fFlags == 0 &&
                                parameters[1]->fType == *fContext.fHalf4_Type &&
                                parameters[1]->fModifiers.fFlags == (Modifiers::kIn_Flag |
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
                    fErrors.error(f.fOffset, "pipeline stage 'main' must be declared main(float2, "
                                             "inout half4) or main(inout half4)");
                    return;
                }
                break;
            }
            case Program::kFragmentProcessor_Kind: {
                bool valid = parameters.size() <= 1;
                if (parameters.size() == 1) {
                    valid = parameters[0]->fType == *fContext.fFloat2_Type &&
                            parameters[0]->fModifiers.fFlags == 0;
                }

                if (!valid) {
                    fErrors.error(f.fOffset, ".fp 'main' must be declared main() or main(float2)");
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
    const Symbol* entry = (*fSymbolTable)[funcData.fName];
    if (entry) {
        std::vector<const FunctionDeclaration*> functions;
        switch (entry->fKind) {
            case Symbol::kUnresolvedFunction_Kind:
                functions = entry->as<UnresolvedFunction>().fFunctions;
                break;
            case Symbol::kFunctionDeclaration_Kind:
                functions.push_back(&entry->as<FunctionDeclaration>());
                break;
            default:
                fErrors.error(f.fOffset, "symbol '" + funcData.fName + "' was already defined");
                return;
        }
        for (const FunctionDeclaration* other : functions) {
            SkASSERT(other->fName == funcData.fName);
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
                        FunctionDeclaration newDecl(f.fOffset, funcData.fModifiers, funcData.fName,
                                                    parameters, *returnType, fIsBuiltinCode);
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
                                                     " differ between declaration and definition");
                            return;
                        }
                    }
                    if (other->fDefinition && !other->fBuiltin) {
                        fErrors.error(f.fOffset, "duplicate definition of " + other->description());
                    }
                    break;
                }
            }
        }
    }
    if (!decl) {
        // Conservatively assume all user-defined functions have side effects.
        Modifiers declModifiers = funcData.fModifiers;
        if (!fIsBuiltinCode) {
            declModifiers.fFlags |= Modifiers::kHasSideEffects_Flag;
        }

        // Create a new declaration.
        decl = fSymbolTable->add(funcData.fName,
                                 std::make_unique<FunctionDeclaration>(f.fOffset,
                                                                       declModifiers,
                                                                       funcData.fName,
                                                                       parameters,
                                                                       *returnType,
                                                                       fIsBuiltinCode));
    }
    if (iter != f.end()) {
        // compile body
        SkASSERT(!fCurrentFunction);
        fCurrentFunction = decl;
        std::shared_ptr<SymbolTable> old = fSymbolTable;
        AutoSymbolTable table(this);
        if (funcData.fName == "main" && fKind == Program::kPipelineStage_Kind) {
            if (parameters.size() == 2) {
                parameters[0]->fModifiers.fLayout.fBuiltin = SK_MAIN_COORDS_BUILTIN;
                parameters[1]->fModifiers.fLayout.fBuiltin = SK_OUTCOLOR_BUILTIN;
            } else {
                SkASSERT(parameters.size() == 1);
                parameters[0]->fModifiers.fLayout.fBuiltin = SK_OUTCOLOR_BUILTIN;
            }
        } else if (funcData.fName == "main" && fKind == Program::kFragmentProcessor_Kind) {
            if (parameters.size() == 1) {
                parameters[0]->fModifiers.fLayout.fBuiltin = SK_MAIN_COORDS_BUILTIN;
            }
        }
        for (size_t i = 0; i < parameters.size(); i++) {
            fSymbolTable->addWithoutOwnership(parameters[i]->fName, decl->fParameters[i]);
        }
        bool needInvocationIDWorkaround = fInvocations != -1 && funcData.fName == "main" &&
                                          fSettings->fCaps &&
                                          !fSettings->fCaps->gsInvocationsSupport();
        std::unique_ptr<Block> body = this->convertBlock(*iter);
        fCurrentFunction = nullptr;
        if (!body) {
            return;
        }
        if (needInvocationIDWorkaround) {
            body = this->applyInvocationIDWorkaround(std::move(body));
        }
        if (Program::kVertex_Kind == fKind && funcData.fName == "main" && fRTAdjust) {
            body->fStatements.insert(body->fStatements.end(), this->getNormalizeSkPositionCode());
        }
        auto result = std::make_unique<FunctionDefinition>(f.fOffset, *decl, std::move(body),
                                                           std::move(fReferencedIntrinsics));
        decl->fDefinition = result.get();
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
            VarDeclaration& vd = stmt->as<VarDeclaration>();
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
            if (vd.fVar->fType.kind() == Type::kArray_Kind &&
                vd.fVar->fType.columns() == -1) {
                haveRuntimeArray = true;
            }
        }
    }
    this->popSymbolTable();
    const Type* type =
            old->takeOwnershipOfSymbol(std::make_unique<Type>(intf.fOffset, id.fTypeName, fields));
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
                count = converted->as<IntLiteral>().fValue;
                if (count <= 0) {
                    fErrors.error(converted->fOffset, "array size must be positive");
                    return nullptr;
                }
                name += "[" + to_string(count) + "]";
            } else {
                fErrors.error(intf.fOffset, "array size must be specified");
                return nullptr;
            }
            type = symbols->takeOwnershipOfSymbol(
                    std::make_unique<Type>(name, Type::kArray_Kind, *type, (int)count));
            sizes.push_back(std::move(converted));
        } else {
            fErrors.error(intf.fOffset, "array size must be specified");
            return nullptr;
        }
    }
    const Variable* var = old->takeOwnershipOfSymbol(
            std::make_unique<Variable>(intf.fOffset,
                                       id.fModifiers,
                                       id.fInstanceName.fLength ? id.fInstanceName : id.fTypeName,
                                       *type,
                                       Variable::kGlobal_Storage));
    if (foundRTAdjust) {
        fRTAdjustInterfaceBlock = var;
    }
    if (id.fInstanceName.fLength) {
        old->addWithoutOwnership(id.fInstanceName, var);
    } else {
        for (size_t i = 0; i < fields.size(); i++) {
            old->add(fields[i].fName, std::make_unique<Field>(intf.fOffset, *var, (int)i));
        }
    }
    return std::make_unique<InterfaceBlock>(intf.fOffset,
                                            var,
                                            id.fTypeName,
                                            id.fInstanceName,
                                            std::move(sizes),
                                            symbols);
}

bool IRGenerator::getConstantInt(const Expression& value, int64_t* out) {
    switch (value.fKind) {
        case Expression::kIntLiteral_Kind:
            *out = static_cast<const IntLiteral&>(value).fValue;
            return true;
        case Expression::kVariableReference_Kind: {
            const Variable& var = static_cast<const VariableReference&>(value).fVariable;
            return (var.fModifiers.fFlags & Modifiers::kConst_Flag) &&
                   var.fInitialValue &&
                   this->getConstantInt(*var.fInitialValue, out);
        }
        default:
            return false;
    }
}

void IRGenerator::convertEnum(const ASTNode& e) {
    SkASSERT(e.fKind == ASTNode::Kind::kEnum);
    int64_t currentValue = 0;
    Layout layout;
    ASTNode enumType(e.fNodes, e.fOffset, ASTNode::Kind::kType,
                     ASTNode::TypeData(e.getString(), false, false));
    const Type* type = this->convertType(enumType);
    Modifiers modifiers(layout, Modifiers::kConst_Flag);
    AutoSymbolTable table(this);
    for (auto iter = e.begin(); iter != e.end(); ++iter) {
        const ASTNode& child = *iter;
        SkASSERT(child.fKind == ASTNode::Kind::kEnumCase);
        std::unique_ptr<Expression> value;
        if (child.begin() != child.end()) {
            value = this->convertExpression(*child.begin());
            if (!value) {
                return;
            }
            if (!this->getConstantInt(*value, &currentValue)) {
                fErrors.error(value->fOffset, "enum value must be a constant integer");
                return;
            }
        }
        value = std::unique_ptr<Expression>(new IntLiteral(fContext, e.fOffset, currentValue));
        ++currentValue;
        fSymbolTable->add(child.getString(),
                          std::make_unique<Variable>(e.fOffset, modifiers, child.getString(), *type,
                                                     Variable::kGlobal_Storage, value.get()));
        fSymbolTable->takeOwnershipOfIRNode(std::move(value));
    }
    fProgramElements->push_back(std::unique_ptr<ProgramElement>(
            new Enum(e.fOffset, e.getString(), fSymbolTable, fIsBuiltinCode)));
}

const Type* IRGenerator::convertType(const ASTNode& type) {
    ASTNode::TypeData td = type.getTypeData();
    const Symbol* result = (*fSymbolTable)[td.fName];
    if (result && result->fKind == Symbol::kType_Kind) {
        if (td.fIsNullable) {
            if (result->as<Type>() == *fContext.fFragmentProcessor_Type) {
                if (type.begin() != type.end()) {
                    fErrors.error(type.fOffset, "type '" + td.fName + "' may not be used in "
                                                "an array");
                }
                result = fSymbolTable->takeOwnershipOfSymbol(std::make_unique<Type>(
                        String(result->fName) + "?", Type::kNullable_Kind, result->as<Type>()));
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
            result = fSymbolTable->takeOwnershipOfSymbol(std::make_unique<Type>(
                    name, Type::kArray_Kind, result->as<Type>(), size ? size.getInt() : 0));
        }
        return &result->as<Type>();
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
#ifdef SK_DEBUG
            ABORT("unsupported expression: %s\n", expr.description().c_str());
#endif
            return nullptr;
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
                &result->as<FunctionDeclaration>()
            };
            return std::make_unique<FunctionReference>(fContext, identifier.fOffset, f);
        }
        case Symbol::kUnresolvedFunction_Kind: {
            const UnresolvedFunction* f = &result->as<UnresolvedFunction>();
            return std::make_unique<FunctionReference>(fContext, identifier.fOffset, f->fFunctions);
        }
        case Symbol::kVariable_Kind: {
            const Variable* var = &result->as<Variable>();
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
            return std::make_unique<VariableReference>(identifier.fOffset,
                                                       *var,
                                                       VariableReference::kRead_RefKind);
        }
        case Symbol::kField_Kind: {
            const Field* field = &result->as<Field>();
            VariableReference* base = new VariableReference(identifier.fOffset, field->fOwner,
                                                            VariableReference::kRead_RefKind);
            return std::unique_ptr<Expression>(new FieldAccess(
                                                  std::unique_ptr<Expression>(base),
                                                  field->fFieldIndex,
                                                  FieldAccess::kAnonymousInterfaceBlock_OwnerKind));
        }
        case Symbol::kType_Kind: {
            const Type* t = &result->as<Type>();
            return std::make_unique<TypeReference>(fContext, identifier.fOffset, *t);
        }
        case Symbol::kExternal_Kind: {
            const ExternalValue* r = &result->as<ExternalValue>();
            return std::make_unique<ExternalValueReference>(identifier.fOffset, r);
        }
        default:
            ABORT("unsupported symbol type %d\n", result->fKind);
    }
}

std::unique_ptr<Section> IRGenerator::convertSection(const ASTNode& s) {
    ASTNode::SectionData section = s.getSectionData();
    return std::make_unique<Section>(s.fOffset, section.fName, section.fArgument,
                                                section.fText);
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
        fErrors.error(expr->fOffset, "expected '" + type.displayName() + "', but found '" +
                                        expr->fType.displayName() + "'");
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
        case Token::Kind::TK_EQ:
            *outLeftType = &left;
            *outRightType = &left;
            *outResultType = &left;
            return right.canCoerceTo(left);
        case Token::Kind::TK_EQEQ: // fall through
        case Token::Kind::TK_NEQ:
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
        case Token::Kind::TK_LT:   // fall through
        case Token::Kind::TK_GT:   // fall through
        case Token::Kind::TK_LTEQ: // fall through
        case Token::Kind::TK_GTEQ:
            isLogical = true;
            validMatrixOrVectorOp = false;
            break;
        case Token::Kind::TK_LOGICALOR: // fall through
        case Token::Kind::TK_LOGICALAND: // fall through
        case Token::Kind::TK_LOGICALXOR: // fall through
        case Token::Kind::TK_LOGICALOREQ: // fall through
        case Token::Kind::TK_LOGICALANDEQ: // fall through
        case Token::Kind::TK_LOGICALXOREQ:
            *outLeftType = context.fBool_Type.get();
            *outRightType = context.fBool_Type.get();
            *outResultType = context.fBool_Type.get();
            return left.canCoerceTo(*context.fBool_Type) &&
                   right.canCoerceTo(*context.fBool_Type);
        case Token::Kind::TK_STAREQ:
            if (left.kind() == Type::kScalar_Kind) {
                *outLeftType = &left;
                *outRightType = &left;
                *outResultType = &left;
                return right.canCoerceTo(left);
            }
            [[fallthrough]];
        case Token::Kind::TK_STAR:
            if (is_matrix_multiply(left, right)) {
                // determine final component type
                if (determine_binary_type(context, Token::Kind::TK_STAR, left.componentType(),
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
        case Token::Kind::TK_PLUSEQ:
        case Token::Kind::TK_MINUSEQ:
        case Token::Kind::TK_SLASHEQ:
        case Token::Kind::TK_PERCENTEQ:
        case Token::Kind::TK_SHLEQ:
        case Token::Kind::TK_SHREQ:
            if (left.kind() == Type::kScalar_Kind) {
                *outLeftType = &left;
                *outRightType = &left;
                *outResultType = &left;
                return right.canCoerceTo(left);
            }
            [[fallthrough]];
        case Token::Kind::TK_PLUS:    // fall through
        case Token::Kind::TK_MINUS:   // fall through
        case Token::Kind::TK_SLASH:   // fall through
            isLogical = false;
            validMatrixOrVectorOp = true;
            break;
        case Token::Kind::TK_COMMA:
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
    bool leftVal = left.as<BoolLiteral>().fValue;
    if (op == Token::Kind::TK_LOGICALAND) {
        // (true && expr) -> (expr) and (false && expr) -> (false)
        return leftVal ? right.clone()
                       : std::unique_ptr<Expression>(new BoolLiteral(context, left.fOffset, false));
    } else if (op == Token::Kind::TK_LOGICALOR) {
        // (true || expr) -> (true) and (false || expr) -> (expr)
        return leftVal ? std::unique_ptr<Expression>(new BoolLiteral(context, left.fOffset, true))
                       : right.clone();
    } else if (op == Token::Kind::TK_LOGICALXOR) {
        // (true ^^ expr) -> !(expr) and (false ^^ expr) -> (expr)
        return leftVal ? std::unique_ptr<Expression>(new PrefixExpression(
                                                                         Token::Kind::TK_LOGICALNOT,
                                                                         right.clone()))
                       : right.clone();
    } else {
        return nullptr;
    }
}

std::unique_ptr<Expression> IRGenerator::constantFold(const Expression& left,
                                                      Token::Kind op,
                                                      const Expression& right) const {
    // If the left side is a constant boolean literal, the right side does not need to be constant
    // for short circuit optimizations to allow the constant to be folded.
    if (left.fKind == Expression::kBoolLiteral_Kind && !right.isCompileTimeConstant()) {
        return short_circuit_boolean(fContext, left, op, right);
    } else if (right.fKind == Expression::kBoolLiteral_Kind && !left.isCompileTimeConstant()) {
        // There aren't side effects in SKSL within expressions, so (left OP right) is equivalent to
        // (right OP left) for short-circuit optimizations
        return short_circuit_boolean(fContext, right, op, left);
    }

    // Other than the short-circuit cases above, constant folding requires both sides to be constant
    if (!left.isCompileTimeConstant() || !right.isCompileTimeConstant()) {
        return nullptr;
    }
    // Note that we expressly do not worry about precision and overflow here -- we use the maximum
    // precision to calculate the results and hope the result makes sense. The plan is to move the
    // Skia caps into SkSL, so we have access to all of them including the precisions of the various
    // types, which will let us be more intelligent about this.
    if (left.fKind == Expression::kBoolLiteral_Kind &&
        right.fKind == Expression::kBoolLiteral_Kind) {
        bool leftVal  = left.as<BoolLiteral>().fValue;
        bool rightVal = right.as<BoolLiteral>().fValue;
        bool result;
        switch (op) {
            case Token::Kind::TK_LOGICALAND: result = leftVal && rightVal; break;
            case Token::Kind::TK_LOGICALOR:  result = leftVal || rightVal; break;
            case Token::Kind::TK_LOGICALXOR: result = leftVal ^  rightVal; break;
            default: return nullptr;
        }
        return std::unique_ptr<Expression>(new BoolLiteral(fContext, left.fOffset, result));
    }
    #define RESULT(t, op) std::make_unique<t ## Literal>(fContext, left.fOffset, \
                                                         leftVal op rightVal)
    #define URESULT(t, op) std::make_unique<t ## Literal>(fContext, left.fOffset, \
                                                          (uint32_t) leftVal op   \
                                                          (uint32_t) rightVal)
    if (left.fKind == Expression::kIntLiteral_Kind && right.fKind == Expression::kIntLiteral_Kind) {
        int64_t leftVal  = left.as<IntLiteral>().fValue;
        int64_t rightVal = right.as<IntLiteral>().fValue;
        switch (op) {
            case Token::Kind::TK_PLUS:       return URESULT(Int, +);
            case Token::Kind::TK_MINUS:      return URESULT(Int, -);
            case Token::Kind::TK_STAR:       return URESULT(Int, *);
            case Token::Kind::TK_SLASH:
                if (leftVal == std::numeric_limits<int64_t>::min() && rightVal == -1) {
                    fErrors.error(right.fOffset, "arithmetic overflow");
                    return nullptr;
                }
                if (!rightVal) {
                    fErrors.error(right.fOffset, "division by zero");
                    return nullptr;
                }
                return RESULT(Int, /);
            case Token::Kind::TK_PERCENT:
                if (leftVal == std::numeric_limits<int64_t>::min() && rightVal == -1) {
                    fErrors.error(right.fOffset, "arithmetic overflow");
                    return nullptr;
                }
                if (!rightVal) {
                    fErrors.error(right.fOffset, "division by zero");
                    return nullptr;
                }
                return RESULT(Int, %);
            case Token::Kind::TK_BITWISEAND: return RESULT(Int,  &);
            case Token::Kind::TK_BITWISEOR:  return RESULT(Int,  |);
            case Token::Kind::TK_BITWISEXOR: return RESULT(Int,  ^);
            case Token::Kind::TK_EQEQ:       return RESULT(Bool, ==);
            case Token::Kind::TK_NEQ:        return RESULT(Bool, !=);
            case Token::Kind::TK_GT:         return RESULT(Bool, >);
            case Token::Kind::TK_GTEQ:       return RESULT(Bool, >=);
            case Token::Kind::TK_LT:         return RESULT(Bool, <);
            case Token::Kind::TK_LTEQ:       return RESULT(Bool, <=);
            case Token::Kind::TK_SHL:
                if (rightVal >= 0 && rightVal <= 31) {
                    return URESULT(Int,  <<);
                }
                fErrors.error(right.fOffset, "shift value out of range");
                return nullptr;
            case Token::Kind::TK_SHR:
                if (rightVal >= 0 && rightVal <= 31) {
                    return URESULT(Int,  >>);
                }
                fErrors.error(right.fOffset, "shift value out of range");
                return nullptr;

            default:
                return nullptr;
        }
    }
    if (left.fKind == Expression::kFloatLiteral_Kind &&
        right.fKind == Expression::kFloatLiteral_Kind) {
        double leftVal  = left.as<FloatLiteral>().fValue;
        double rightVal = right.as<FloatLiteral>().fValue;
        switch (op) {
            case Token::Kind::TK_PLUS:  return RESULT(Float, +);
            case Token::Kind::TK_MINUS: return RESULT(Float, -);
            case Token::Kind::TK_STAR:  return RESULT(Float, *);
            case Token::Kind::TK_SLASH:
                if (rightVal) {
                    return RESULT(Float, /);
                }
                fErrors.error(right.fOffset, "division by zero");
                return nullptr;
            case Token::Kind::TK_EQEQ: return RESULT(Bool, ==);
            case Token::Kind::TK_NEQ:  return RESULT(Bool, !=);
            case Token::Kind::TK_GT:   return RESULT(Bool, >);
            case Token::Kind::TK_GTEQ: return RESULT(Bool, >=);
            case Token::Kind::TK_LT:   return RESULT(Bool, <);
            case Token::Kind::TK_LTEQ: return RESULT(Bool, <=);
            default:                   return nullptr;
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
            case Token::Kind::TK_EQEQ:
                return std::unique_ptr<Expression>(new BoolLiteral(fContext, -1,
                                                            left.compareConstant(fContext, right)));
            case Token::Kind::TK_NEQ:
                return std::unique_ptr<Expression>(new BoolLiteral(fContext, -1,
                                                           !left.compareConstant(fContext, right)));
            case Token::Kind::TK_PLUS:  RETURN_VEC_COMPONENTWISE_RESULT(+);
            case Token::Kind::TK_MINUS: RETURN_VEC_COMPONENTWISE_RESULT(-);
            case Token::Kind::TK_STAR:  RETURN_VEC_COMPONENTWISE_RESULT(*);
            case Token::Kind::TK_SLASH:
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
            case Token::Kind::TK_EQEQ:
                return std::unique_ptr<Expression>(new BoolLiteral(fContext, -1,
                                                            left.compareConstant(fContext, right)));
            case Token::Kind::TK_NEQ:
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
    Token::Kind op = expression.getToken().fKind;
    std::unique_ptr<Expression> right;
    {
        // Can't inline the right side of a short-circuiting boolean, because our inlining
        // approach runs things out of order.
        AutoDisableInline disableInline(this, /*canInline=*/(op != Token::Kind::TK_LOGICALAND &&
                                                             op != Token::Kind::TK_LOGICALOR));
        right = this->convertExpression(*(iter++));
    }
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
    if (!determine_binary_type(fContext, op, *rawLeftType, *rawRightType, &leftType, &rightType,
                               &resultType, !Compiler::IsAssignment(op))) {
        fErrors.error(expression.fOffset, String("type mismatch: '") +
                                          Compiler::OperatorName(expression.getToken().fKind) +
                                          "' cannot operate on '" + left->fType.displayName() +
                                          "', '" + right->fType.displayName() + "'");
        return nullptr;
    }
    if (Compiler::IsAssignment(op)) {
        if (!this->setRefKind(*left, op != Token::Kind::TK_EQ
                                                             ? VariableReference::kReadWrite_RefKind
                                                             : VariableReference::kWrite_RefKind)) {
            return nullptr;
        }
    }
    left = this->coerce(std::move(left), *leftType);
    right = this->coerce(std::move(right), *rightType);
    if (!left || !right) {
        return nullptr;
    }
    std::unique_ptr<Expression> result = this->constantFold(*left, op, *right);
    if (!result) {
        result = std::make_unique<BinaryExpression>(expression.fOffset, std::move(left), op,
                                                    std::move(right), *resultType);
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
    if (!determine_binary_type(fContext, Token::Kind::TK_EQEQ, ifTrue->fType, ifFalse->fType,
                               &trueType, &falseType, &resultType, true) || trueType != falseType) {
        fErrors.error(node.fOffset, "ternary operator result mismatch: '" +
                                    ifTrue->fType.displayName() + "', '" +
                                    ifFalse->fType.displayName() + "'");
        return nullptr;
    }
    if (trueType->nonnullable() == *fContext.fFragmentProcessor_Type) {
        fErrors.error(node.fOffset,
                      "ternary expression of type '" + trueType->displayName() + "' not allowed");
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
        if (test->as<BoolLiteral>().fValue) {
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

std::unique_ptr<Expression> IRGenerator::inlineExpression(
        int offset,
        std::unordered_map<const Variable*, const Variable*>* varMap,
        const Expression& expression) {
    auto expr = [&](const std::unique_ptr<Expression>& e) -> std::unique_ptr<Expression> {
        if (e) {
            return this->inlineExpression(offset, varMap, *e);
        }
        return nullptr;
    };
    switch (expression.fKind) {
        case Expression::kBinary_Kind: {
            const BinaryExpression& b = expression.as<BinaryExpression>();
            return std::unique_ptr<Expression>(new BinaryExpression(offset,
                                                                    expr(b.fLeft),
                                                                    b.fOperator,
                                                                    expr(b.fRight),
                                                                    b.fType));
        }
        case Expression::kBoolLiteral_Kind:
        case Expression::kIntLiteral_Kind:
        case Expression::kFloatLiteral_Kind:
        case Expression::kNullLiteral_Kind:
            return expression.clone();
        case Expression::kConstructor_Kind: {
            const Constructor& c = expression.as<Constructor>();
            std::vector<std::unique_ptr<Expression>> args;
            for (const auto& arg : c.fArguments) {
                args.push_back(expr(arg));
            }
            return std::unique_ptr<Expression>(new Constructor(offset, c.fType, std::move(args)));
        }
        case Expression::kExternalFunctionCall_Kind: {
            const ExternalFunctionCall& e = expression.as<ExternalFunctionCall>();
            std::vector<std::unique_ptr<Expression>> args;
            for (const auto& arg : e.fArguments) {
                args.push_back(expr(arg));
            }
            return std::unique_ptr<Expression>(new ExternalFunctionCall(offset, e.fType,
                                                                        e.fFunction,
                                                                        std::move(args)));
        }
        case Expression::kExternalValue_Kind:
            return expression.clone();
        case Expression::kFieldAccess_Kind: {
            const FieldAccess& f = expression.as<FieldAccess>();
            return std::unique_ptr<Expression>(new FieldAccess(expr(f.fBase), f.fFieldIndex,
                                                               f.fOwnerKind));
        }
        case Expression::kFunctionCall_Kind: {
            const FunctionCall& c = expression.as<FunctionCall>();
            std::vector<std::unique_ptr<Expression>> args;
            for (const auto& arg : c.fArguments) {
                args.push_back(expr(arg));
            }
            return std::unique_ptr<Expression>(new FunctionCall(offset, c.fType, c.fFunction,
                                                                std::move(args)));
        }
        case Expression::kIndex_Kind: {
            const IndexExpression& idx = expression.as<IndexExpression>();
            return std::unique_ptr<Expression>(new IndexExpression(fContext, expr(idx.fBase),
                                                                   expr(idx.fIndex)));
        }
        case Expression::kPrefix_Kind: {
            const PrefixExpression& p = expression.as<PrefixExpression>();
            return std::unique_ptr<Expression>(new PrefixExpression(p.fOperator, expr(p.fOperand)));
        }
        case Expression::kPostfix_Kind: {
            const PostfixExpression& p = expression.as<PostfixExpression>();
            return std::unique_ptr<Expression>(new PostfixExpression(expr(p.fOperand),
                                                                     p.fOperator));
        }
        case Expression::kSetting_Kind:
            return expression.clone();
        case Expression::kSwizzle_Kind: {
            const Swizzle& s = expression.as<Swizzle>();
            return std::unique_ptr<Expression>(new Swizzle(fContext, expr(s.fBase), s.fComponents));
        }
        case Expression::kTernary_Kind: {
            const TernaryExpression& t = expression.as<TernaryExpression>();
            return std::unique_ptr<Expression>(new TernaryExpression(offset, expr(t.fTest),
                                                                     expr(t.fIfTrue),
                                                                     expr(t.fIfFalse)));
        }
        case Expression::kVariableReference_Kind: {
            const VariableReference& v = expression.as<VariableReference>();
            auto found = varMap->find(&v.fVariable);
            if (found != varMap->end()) {
                return std::unique_ptr<Expression>(new VariableReference(offset,
                                                                         *found->second,
                                                                         v.fRefKind));
            }
            return v.clone();
        }
        default:
            SkASSERT(false);
            return nullptr;
    }
}

static const Type* copy_if_needed(const Type* src, SymbolTable& symbolTable) {
    if (src->kind() == Type::kArray_Kind) {
        return symbolTable.takeOwnershipOfSymbol(std::make_unique<Type>(*src));
    }
    return src;
}

std::unique_ptr<Statement> IRGenerator::inlineStatement(
        int offset,
        std::unordered_map<const Variable*, const Variable*>* varMap,
        const Variable* returnVar,
        bool haveEarlyReturns,
        const Statement& statement) {
    auto stmt = [&](const std::unique_ptr<Statement>& s) -> std::unique_ptr<Statement> {
        if (s) {
            return this->inlineStatement(offset, varMap, returnVar, haveEarlyReturns, *s);
        }
        return nullptr;
    };
    auto stmts = [&](const std::vector<std::unique_ptr<Statement>>& ss) {
        std::vector<std::unique_ptr<Statement>> result;
        for (const auto& s : ss) {
            result.push_back(stmt(s));
        }
        return result;
    };
    auto expr = [&](const std::unique_ptr<Expression>& e) -> std::unique_ptr<Expression> {
        if (e) {
            return this->inlineExpression(offset, varMap, *e);
        }
        return nullptr;
    };
    switch (statement.fKind) {
        case Statement::kBlock_Kind: {
            const Block& b = statement.as<Block>();
            return std::make_unique<Block>(offset, stmts(b.fStatements), b.fSymbols, b.fIsScope);
        }

        case Statement::kBreak_Kind:
        case Statement::kContinue_Kind:
        case Statement::kDiscard_Kind:
            return statement.clone();

        case Statement::kDo_Kind: {
            const DoStatement& d = statement.as<DoStatement>();
            return std::make_unique<DoStatement>(offset, stmt(d.fStatement), expr(d.fTest));
        }
        case Statement::kExpression_Kind: {
            const ExpressionStatement& e = statement.as<ExpressionStatement>();
            return std::make_unique<ExpressionStatement>(expr(e.fExpression));
        }
        case Statement::kFor_Kind: {
            const ForStatement& f = statement.as<ForStatement>();
            // need to ensure initializer is evaluated first so that we've already remapped its
            // declarations by the time we evaluate test & next
            std::unique_ptr<Statement> initializer = stmt(f.fInitializer);
            return std::make_unique<ForStatement>(offset, std::move(initializer), expr(f.fTest),
                                                  expr(f.fNext), stmt(f.fStatement), f.fSymbols);
        }
        case Statement::kIf_Kind: {
            const IfStatement& i = statement.as<IfStatement>();
            return std::make_unique<IfStatement>(offset, i.fIsStatic, expr(i.fTest),
                                                 stmt(i.fIfTrue), stmt(i.fIfFalse));
        }
        case Statement::kNop_Kind:
            return statement.clone();
        case Statement::kReturn_Kind: {
            const ReturnStatement& r = statement.as<ReturnStatement>();
            if (r.fExpression) {
                auto assignment = std::make_unique<ExpressionStatement>(
                        std::make_unique<BinaryExpression>(
                            offset,
                            std::make_unique<VariableReference>(offset, *returnVar,
                                                                VariableReference::kWrite_RefKind),
                            Token::Kind::TK_EQ,
                            expr(r.fExpression),
                            returnVar->fType));
                if (haveEarlyReturns) {
                    std::vector<std::unique_ptr<Statement>> block;
                    block.push_back(std::move(assignment));
                    block.emplace_back(new BreakStatement(offset));
                    return std::make_unique<Block>(offset, std::move(block), /*symbols=*/nullptr,
                                                   /*isScope=*/true);
                } else {
                    return std::move(assignment);
                }
            } else {
                if (haveEarlyReturns) {
                    return std::make_unique<BreakStatement>(offset);
                } else {
                    return std::make_unique<Nop>();
                }
            }
        }
        case Statement::kSwitch_Kind: {
            const SwitchStatement& ss = statement.as<SwitchStatement>();
            std::vector<std::unique_ptr<SwitchCase>> cases;
            for (const auto& sc : ss.fCases) {
                cases.emplace_back(new SwitchCase(offset, expr(sc->fValue),
                                                  stmts(sc->fStatements)));
            }
            return std::make_unique<SwitchStatement>(offset, ss.fIsStatic, expr(ss.fValue),
                                                     std::move(cases), ss.fSymbols);
        }
        case Statement::kVarDeclaration_Kind: {
            const VarDeclaration& decl = statement.as<VarDeclaration>();
            std::vector<std::unique_ptr<Expression>> sizes;
            for (const auto& size : decl.fSizes) {
                sizes.push_back(expr(size));
            }
            std::unique_ptr<Expression> initialValue = expr(decl.fValue);
            const Variable* old = decl.fVar;
            // need to copy the var name in case the originating function is discarded and we lose
            // its symbols
            std::unique_ptr<String> name(new String(old->fName));
            const String* namePtr = fSymbolTable->takeOwnershipOfString(std::move(name));
            const Type* typePtr = copy_if_needed(&old->fType, *fSymbolTable);
            const Variable* clone = fSymbolTable->takeOwnershipOfSymbol(
                    std::make_unique<Variable>(offset,
                                               old->fModifiers,
                                               namePtr->c_str(),
                                               *typePtr,
                                               old->fStorage,
                                               initialValue.get()));
            (*varMap)[old] = clone;
            return std::make_unique<VarDeclaration>(clone, std::move(sizes),
                                                    std::move(initialValue));
        }
        case Statement::kVarDeclarations_Kind: {
            const VarDeclarations& decls = *statement.as<VarDeclarationsStatement>().fDeclaration;
            std::vector<std::unique_ptr<VarDeclaration>> vars;
            for (const auto& var : decls.fVars) {
                vars.emplace_back(&stmt(var).release()->as<VarDeclaration>());
            }
            const Type* typePtr = copy_if_needed(&decls.fBaseType, *fSymbolTable);
            return std::unique_ptr<Statement>(new VarDeclarationsStatement(
                    std::make_unique<VarDeclarations>(offset, typePtr, std::move(vars))));
        }
        case Statement::kWhile_Kind: {
            const WhileStatement& w = statement.as<WhileStatement>();
            return std::make_unique<WhileStatement>(offset, expr(w.fTest), stmt(w.fStatement));
        }
        default:
            SkASSERT(false);
            return nullptr;
    }
}

template <bool countTopLevelReturns>
static int return_count(const Statement& statement, bool inLoopOrSwitch) {
    switch (statement.fKind) {
        case Statement::kBlock_Kind: {
            const Block& b = static_cast<const Block&>(statement);
            int result = 0;
            for (const std::unique_ptr<Statement>& s : b.fStatements) {
                result += return_count<countTopLevelReturns>(*s, inLoopOrSwitch);
            }
            return result;
        }
        case Statement::kDo_Kind: {
            const DoStatement& d = static_cast<const DoStatement&>(statement);
            return return_count<countTopLevelReturns>(*d.fStatement, /*inLoopOrSwitch=*/true);
        }
        case Statement::kFor_Kind: {
            const ForStatement& f = static_cast<const ForStatement&>(statement);
            return return_count<countTopLevelReturns>(*f.fStatement, /*inLoopOrSwitch=*/true);
        }
        case Statement::kIf_Kind: {
            const IfStatement& i = static_cast<const IfStatement&>(statement);
            int result = return_count<countTopLevelReturns>(*i.fIfTrue, inLoopOrSwitch);
            if (i.fIfFalse) {
                result += return_count<countTopLevelReturns>(*i.fIfFalse, inLoopOrSwitch);
            }
            return result;
        }
        case Statement::kReturn_Kind:
            return (countTopLevelReturns || inLoopOrSwitch) ? 1 : 0;
        case Statement::kSwitch_Kind: {
            const SwitchStatement& ss = static_cast<const SwitchStatement&>(statement);
            int result = 0;
            for (const std::unique_ptr<SwitchCase>& sc : ss.fCases) {
                for (const std::unique_ptr<Statement>& s : sc->fStatements) {
                    result += return_count<countTopLevelReturns>(*s, /*inLoopOrSwitch=*/true);
                }
            }
            return result;
        }
        case Statement::kWhile_Kind: {
            const WhileStatement& w = static_cast<const WhileStatement&>(statement);
            return return_count<countTopLevelReturns>(*w.fStatement, /*inLoopOrSwitch=*/true);
        }
        case Statement::kBreak_Kind:
        case Statement::kContinue_Kind:
        case Statement::kDiscard_Kind:
        case Statement::kExpression_Kind:
        case Statement::kNop_Kind:
        case Statement::kVarDeclaration_Kind:
        case Statement::kVarDeclarations_Kind:
            return 0;
        default:
            SkASSERT(false);
            return 0;
    }
}

static bool has_early_return(const FunctionDefinition& f) {
    int returnCount =
            return_count</*countTopLevelReturns=*/true>(*f.fBody, /*inLoopOrSwitch=*/false);
    if (returnCount == 0) {
        return false;
    }
    if (returnCount > 1) {
        return true;
    }
    SkASSERT(f.fBody->fKind == Statement::kBlock_Kind);
    return static_cast<Block&>(*f.fBody).fStatements.back()->fKind != Statement::kReturn_Kind;
}

static bool has_return_in_breakable_construct(const FunctionDefinition& f) {
    int returnCount =
            return_count</*countTopLevelReturns=*/false>(*f.fBody, /*inLoopOrSwitch=*/false);
    return returnCount > 0;
}

std::unique_ptr<Expression> IRGenerator::inlineCall(
                                               int offset,
                                               const FunctionDefinition& function,
                                               std::vector<std::unique_ptr<Expression>> arguments) {
    // Inlining is more complicated here than in a typical compiler, because we have to have a
    // high-level IR and can't just drop statements into the middle of an expression or even use
    // gotos.
    //
    // Since we can't insert statements into an expression, we run the inline function as extra
    // statements before the statement we're currently processing, relying on a lack of execution
    // order guarantees. Since we can't use gotos (which are normally used to replace return
    // statements), we wrap the whole function in a loop and use break statements to jump to the
    // end.

    // Use unique variable names based on the function signature. Otherwise there are situations in
    // which an inlined function is later inlined into another function, and we end up with
    // duplicate names like 'inlineResult0' because the counter was reset. (skbug.com/10526)
    String raw = function.fDeclaration.description();
    String inlineSalt;
    for (size_t i = 0; i < raw.length(); ++i) {
        char c = raw[i];
        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') ||
            c == '_') {
            inlineSalt += c;
        }
    }

    const Variable* resultVar = nullptr;
    if (function.fDeclaration.fReturnType != *fContext.fVoid_Type) {
        std::unique_ptr<String> name(new String());
        int varIndex = fInlineVarCounter++;
        name->appendf("_inlineResult%s%d", inlineSalt.c_str(), varIndex);
        const String* namePtr = fSymbolTable->takeOwnershipOfString(std::move(name));
        StringFragment nameFrag{namePtr->c_str(), namePtr->length()};
        resultVar = fSymbolTable->add(
                nameFrag,
                std::make_unique<Variable>(
                        /*offset=*/-1, Modifiers(), nameFrag, function.fDeclaration.fReturnType,
                        Variable::kLocal_Storage, /*initialValue=*/nullptr));
        std::vector<std::unique_ptr<VarDeclaration>> variables;
        variables.emplace_back(new VarDeclaration(resultVar, {}, nullptr));
        fExtraStatements.emplace_back(
                new VarDeclarationsStatement(std::make_unique<VarDeclarations>(
                        offset, &resultVar->fType, std::move(variables))));

    }
    std::unordered_map<const Variable*, const Variable*> varMap;
    // create variables to hold the arguments and assign the arguments to them
    int argIndex = fInlineVarCounter++;
    for (int i = 0; i < (int) arguments.size(); ++i) {
        std::unique_ptr<String> argName(new String());
        argName->appendf("_inlineArg%s%d_%d", inlineSalt.c_str(), argIndex, i);
        const String* argNamePtr = fSymbolTable->takeOwnershipOfString(std::move(argName));
        StringFragment argNameFrag{argNamePtr->c_str(), argNamePtr->length()};
        const Variable* argVar = fSymbolTable->add(
                argNameFrag, std::make_unique<Variable>(
                                     /*offset=*/-1, Modifiers(), argNameFrag, arguments[i]->fType,
                                     Variable::kLocal_Storage, arguments[i].get()));
        varMap[function.fDeclaration.fParameters[i]] = argVar;
        std::vector<std::unique_ptr<VarDeclaration>> vars;
        if (function.fDeclaration.fParameters[i]->fModifiers.fFlags & Modifiers::kOut_Flag) {
            vars.emplace_back(new VarDeclaration(argVar, {}, arguments[i]->clone()));
        } else {
            vars.emplace_back(new VarDeclaration(argVar, {}, std::move(arguments[i])));
        }
        fExtraStatements.emplace_back(new VarDeclarationsStatement(
                std::make_unique<VarDeclarations>(offset, &argVar->fType, std::move(vars))));
    }
    const Block& body = function.fBody->as<Block>();
    bool hasEarlyReturn = has_early_return(function);
    std::vector<std::unique_ptr<Statement>> inlined;
    for (const auto& s : body.fStatements) {
        inlined.push_back(this->inlineStatement(offset, &varMap, resultVar, hasEarlyReturn, *s));
    }
    if (hasEarlyReturn) {
        // Since we output to backends that don't have a goto statement (which would normally be
        // used to perform an early return), we fake it by wrapping the function in a
        // do { } while (false); and then use break statements to jump to the end in order to
        // emulate a goto.
        fExtraStatements.emplace_back(new DoStatement(-1,
                                std::unique_ptr<Statement>(new Block(-1, std::move(inlined))),
                                std::unique_ptr<Expression>(new BoolLiteral(fContext, -1, false))));
    } else {
        // No early returns, so we can just dump the code in. We need to use a block so we don't get
        // name conflicts with locals.
        fExtraStatements.emplace_back(std::unique_ptr<Statement>(new Block(-1,
                                                                           std::move(inlined))));
    }
    // copy the values of out parameters into their destinations
    for (size_t i = 0; i < arguments.size(); ++i) {
        const Variable* p = function.fDeclaration.fParameters[i];
        if (p->fModifiers.fFlags & Modifiers::kOut_Flag) {
            std::unique_ptr<Expression> varRef(new VariableReference(offset, *varMap[p]));
            fExtraStatements.emplace_back(new ExpressionStatement(
                    std::unique_ptr<Expression>(new BinaryExpression(offset,
                                                                     arguments[i]->clone(),
                                                                     Token::Kind::TK_EQ,
                                                                     std::move(varRef),
                                                                     arguments[i]->fType))));
        }
    }
    if (function.fDeclaration.fReturnType != *fContext.fVoid_Type) {
        return std::unique_ptr<Expression>(new VariableReference(-1, *resultVar));
    } else {
        // it's a void function, so it doesn't actually result in anything, but we have to return
        // something non-null as a standin
        return std::unique_ptr<Expression>(new BoolLiteral(fContext, -1, false));
    }
}

void IRGenerator::copyIntrinsicIfNeeded(const FunctionDeclaration& function) {
    auto found = fIntrinsics->find(function.description());
    if (found != fIntrinsics->end() && !found->second.second) {
        found->second.second = true;
        FunctionDefinition& original = found->second.first->as<FunctionDefinition>();
        for (const FunctionDeclaration* f : original.fReferencedIntrinsics) {
            this->copyIntrinsicIfNeeded(*f);
        }
        fProgramElements->push_back(original.clone());
    }
}

bool IRGenerator::isSafeToInline(const FunctionDefinition& functionDef) {
    if (!fCanInline) {
        // Inlining has been explicitly disabled by the IR generator.
        return false;
    }
    if (!(functionDef.fDeclaration.fModifiers.fFlags & Modifiers::kInline_Flag) &&
        Analysis::NodeCount(functionDef) >= fSettings->fInlineThreshold) {
        // The function exceeds our maximum inline size and is not flagged 'inline'.
        return false;
    }
    if (!fSettings->fCaps || !fSettings->fCaps->canUseDoLoops()) {
        // We don't have do-while loops. We use do-while loops to simulate early returns, so we
        // can't inline functions that have an early return.
        return !has_early_return(functionDef);
    }
    // We have do-while loops, but we don't have any mechanism to simulate early returns within a
    // breakable construct (switch/for/do/while), so we can't inline if there's a return inside one.
    return !has_return_in_breakable_construct(functionDef);
}

std::unique_ptr<Expression> IRGenerator::call(int offset,
                                              const FunctionDeclaration& function,
                                              std::vector<std::unique_ptr<Expression>> arguments) {
    if (function.fBuiltin) {
        if (function.fDefinition) {
            fReferencedIntrinsics.insert(&function);
        }
        if (!fIsBuiltinCode) {
            this->copyIntrinsicIfNeeded(function);
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
    if (fKind == Program::kPipelineStage_Kind && !function.fDefinition && !function.fBuiltin) {
        String msg = "call to undefined function '" + function.fName + "'";
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
            msg += arguments[i]->fType.displayName();
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
    if (function.fDefinition && this->isSafeToInline(*function.fDefinition)) {
        return this->inlineCall(offset, *function.fDefinition, std::move(arguments));
    }

    return std::make_unique<FunctionCall>(offset, *returnType, function, std::move(arguments));
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
                                            functionValue->as<TypeReference>().fValue,
                                            std::move(arguments));
        case Expression::kExternalValue_Kind: {
            const ExternalValue* v = functionValue->as<ExternalValueReference>().fValue;
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
                    msg += arguments[i]->fType.displayName();
                }
                msg += ")";
                fErrors.error(offset, msg);
                return nullptr;
            }
            return this->call(offset, *ref->fFunctions[0], std::move(arguments));
        }
        default:
            fErrors.error(offset, "not a function");
            return nullptr;
    }
}

std::unique_ptr<Expression> IRGenerator::convertNumberConstructor(
                                                    int offset,
                                                    const Type& type,
                                                    std::vector<std::unique_ptr<Expression>> args) {
    SkASSERT(type.isNumber());
    if (args.size() != 1) {
        fErrors.error(offset, "invalid arguments to '" + type.displayName() +
                              "' constructor, (expected exactly 1 argument, but found " +
                              to_string((uint64_t) args.size()) + ")");
        return nullptr;
    }
    if (type == args[0]->fType) {
        return std::move(args[0]);
    }
    if (type.isFloat() && args.size() == 1 && args[0]->fKind == Expression::kFloatLiteral_Kind) {
        double value = args[0]->as<FloatLiteral>().fValue;
        return std::unique_ptr<Expression>(new FloatLiteral(offset, value, &type));
    }
    if (type.isFloat() && args.size() == 1 && args[0]->fKind == Expression::kIntLiteral_Kind) {
        int64_t value = args[0]->as<IntLiteral>().fValue;
        return std::unique_ptr<Expression>(new FloatLiteral(offset, (double) value, &type));
    }
    if (args[0]->fKind == Expression::kIntLiteral_Kind && (type == *fContext.fInt_Type ||
        type == *fContext.fUInt_Type)) {
        return std::unique_ptr<Expression>(new IntLiteral(offset,
                                                          args[0]->as<IntLiteral>().fValue,
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
        fErrors.error(offset, "invalid argument to '" + type.displayName() +
                              "' constructor (expected a number or bool, but found '" +
                              args[0]->fType.displayName() + "')");
        return nullptr;
    }
    return std::unique_ptr<Expression>(new Constructor(offset, type, std::move(args)));
}

static int component_count(const Type& type) {
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
                    fErrors.error(offset, "'" + args[i]->fType.displayName() + "' is not a valid "
                                          "parameter to '" + type.displayName() +
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
                fErrors.error(offset, "'" + args[i]->fType.displayName() + "' is not a valid "
                                      "parameter to '" + type.displayName() + "' constructor");
                return nullptr;
            }
        }
        if (actual != 1 && actual != expected) {
            fErrors.error(offset, "invalid arguments to '" + type.displayName() +
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
    if (args.size() == 1 && args[0]->fType == type &&
        type.nonnullable() != *fContext.fFragmentProcessor_Type) {
        // argument is already the right type, just return it
        return std::move(args[0]);
    }
    Type::Kind kind = type.kind();
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
        fErrors.error(offset, "cannot construct '" + type.displayName() + "'");
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
        case Token::Kind::TK_PLUS:
            if (!base->fType.isNumber() && base->fType.kind() != Type::kVector_Kind &&
                base->fType != *fContext.fFloatLiteral_Type) {
                fErrors.error(expression.fOffset,
                              "'+' cannot operate on '" + base->fType.displayName() + "'");
                return nullptr;
            }
            return base;
        case Token::Kind::TK_MINUS:
            if (base->fKind == Expression::kIntLiteral_Kind) {
                return std::unique_ptr<Expression>(new IntLiteral(fContext, base->fOffset,
                                                                  -base->as<IntLiteral>().fValue));
            }
            if (base->fKind == Expression::kFloatLiteral_Kind) {
                double value = -base->as<FloatLiteral>().fValue;
                return std::unique_ptr<Expression>(new FloatLiteral(fContext, base->fOffset,
                                                                    value));
            }
            if (!base->fType.isNumber() && base->fType.kind() != Type::kVector_Kind) {
                fErrors.error(expression.fOffset,
                              "'-' cannot operate on '" + base->fType.displayName() + "'");
                return nullptr;
            }
            return std::unique_ptr<Expression>(new PrefixExpression(Token::Kind::TK_MINUS,
                                                                    std::move(base)));
        case Token::Kind::TK_PLUSPLUS:
            if (!base->fType.isNumber()) {
                fErrors.error(expression.fOffset,
                              String("'") + Compiler::OperatorName(expression.getToken().fKind) +
                              "' cannot operate on '" + base->fType.displayName() + "'");
                return nullptr;
            }
            this->setRefKind(*base, VariableReference::kReadWrite_RefKind);
            break;
        case Token::Kind::TK_MINUSMINUS:
            if (!base->fType.isNumber()) {
                fErrors.error(expression.fOffset,
                              String("'") + Compiler::OperatorName(expression.getToken().fKind) +
                              "' cannot operate on '" + base->fType.displayName() + "'");
                return nullptr;
            }
            this->setRefKind(*base, VariableReference::kReadWrite_RefKind);
            break;
        case Token::Kind::TK_LOGICALNOT:
            if (base->fType != *fContext.fBool_Type) {
                fErrors.error(expression.fOffset,
                              String("'") + Compiler::OperatorName(expression.getToken().fKind) +
                              "' cannot operate on '" + base->fType.displayName() + "'");
                return nullptr;
            }
            if (base->fKind == Expression::kBoolLiteral_Kind) {
                return std::unique_ptr<Expression>(
                        new BoolLiteral(fContext, base->fOffset, !base->as<BoolLiteral>().fValue));
            }
            break;
        case Token::Kind::TK_BITWISENOT:
            if (base->fType != *fContext.fInt_Type && base->fType != *fContext.fUInt_Type) {
                fErrors.error(expression.fOffset,
                              String("'") + Compiler::OperatorName(expression.getToken().fKind) +
                              "' cannot operate on '" + base->fType.displayName() + "'");
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
            const Type& oldType = base->as<TypeReference>().fValue;
            SKSL_INT size = index.getInt();
            const Type* newType = fSymbolTable->takeOwnershipOfSymbol(
                    std::make_unique<Type>(oldType.name() + "[" + to_string(size) + "]",
                                           Type::kArray_Kind, oldType, size));
            return std::make_unique<TypeReference>(fContext, base->fOffset, *newType);

        } else {
            fErrors.error(base->fOffset, "array size must be a constant");
            return nullptr;
        }
    }
    if (base->fType.kind() != Type::kArray_Kind && base->fType.kind() != Type::kMatrix_Kind &&
            base->fType.kind() != Type::kVector_Kind) {
        fErrors.error(base->fOffset, "expected array, but found '" + base->fType.displayName() +
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
        const ExternalValue& ev = *base->as<ExternalValueReference>().fValue;
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
    fErrors.error(base->fOffset, "type '" + base->fType.displayName() + "' does not have a "
                                 "field named '" + field + "");
    return nullptr;
}

// counts the number of chunks of contiguous 'x's in a swizzle, e.g. xxx1 has one and x0xx has two
static int count_contiguous_swizzle_chunks(const std::vector<int>& components) {
    int chunkCount = 0;
    for (size_t i = 0; i < components.size(); ++i) {
        SkASSERT(components[i] <= 0);
        if (components[i] == 0) {
            ++chunkCount;
            while (i + 1 < components.size() && components[i + 1] == 0) {
                ++i;
            }
        }
    }
    return chunkCount;
}

std::unique_ptr<Expression> IRGenerator::convertSwizzle(std::unique_ptr<Expression> base,
                                                        StringFragment fields) {
    if (base->fType.kind() != Type::kVector_Kind && !base->fType.isNumber()) {
        fErrors.error(base->fOffset, "cannot swizzle value of type '" + base->fType.displayName() +
                                     "'");
        return nullptr;
    }
    std::vector<int> swizzleComponents;
    size_t numLiteralFields = 0;
    for (size_t i = 0; i < fields.fLength; i++) {
        switch (fields[i]) {
            case '0':
                swizzleComponents.push_back(SKSL_SWIZZLE_0);
                numLiteralFields++;
                break;
            case '1':
                swizzleComponents.push_back(SKSL_SWIZZLE_1);
                numLiteralFields++;
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
                [[fallthrough]];
            case 'z':
            case 'b':
            case 'p':
            case 'R':
                if (base->fType.columns() >= 3) {
                    swizzleComponents.push_back(2);
                    break;
                }
                [[fallthrough]];
            case 'w':
            case 'a':
            case 'q':
            case 'B':
                if (base->fType.columns() >= 4) {
                    swizzleComponents.push_back(3);
                    break;
                }
                [[fallthrough]];
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
    if (numLiteralFields == swizzleComponents.size()) {
        fErrors.error(base->fOffset, "swizzle must refer to base expression");
        return nullptr;
    }
    if (base->fType.isNumber()) {
        // Swizzling a single scalar. Something like foo.x0x1 is equivalent to float4(foo, 0, foo,
        // 1)
        int offset = base->fOffset;
        std::unique_ptr<Expression> expr;
        switch (base->fKind) {
            case Expression::kVariableReference_Kind:
            case Expression::kFloatLiteral_Kind:
            case Expression::kIntLiteral_Kind:
                // the value being swizzled is just a constant or variable reference, so we can
                // safely re-use copies of it without reevaluation concerns
                expr = std::move(base);
                break;
            default:
                // It's a value we can't safely re-use multiple times. If it's all in one contiguous
                // chunk it's easy (e.g. foo.xxx0 can be turned into half4(half3(x), 0)), but
                // for multiple discontiguous chunks we'll need to copy it into a temporary value.
                int chunkCount = count_contiguous_swizzle_chunks(swizzleComponents);
                if (chunkCount <= 1) {
                    // no copying needed, so we can just use the value directly
                    expr = std::move(base);
                } else {
                    // store the value in a temporary variable so we can re-use it
                    int varIndex = fInlineVarCounter++;
                    auto name = std::make_unique<String>();
                    name->appendf("_tmpSwizzle%d", varIndex);
                    const String* namePtr = fSymbolTable->takeOwnershipOfString(std::move(name));
                    const Variable* var = fSymbolTable->takeOwnershipOfSymbol(
                            std::make_unique<Variable>(offset,
                                                       Modifiers(),
                                                       namePtr->c_str(),
                                                       base->fType,
                                                       Variable::kLocal_Storage,
                                                       base.get()));
                    expr = std::make_unique<VariableReference>(offset, *var);
                    std::vector<std::unique_ptr<VarDeclaration>> variables;
                    variables.emplace_back(new VarDeclaration(var, {}, std::move(base)));
                    fExtraStatements.emplace_back(new VarDeclarationsStatement(
                            std::make_unique<VarDeclarations>(offset, &expr->fType,
                                                              std::move(variables))));
                }
        }
        std::vector<std::unique_ptr<Expression>> args;
        for (size_t i = 0; i < swizzleComponents.size(); ++i) {
            switch (swizzleComponents[i]) {
                case 0: {
                    args.push_back(expr->clone());
                    int count = 1;
                    while (i + 1 < swizzleComponents.size() && swizzleComponents[i + 1] == 0) {
                        ++i;
                        ++count;
                    }
                    if (count > 1) {
                        std::vector<std::unique_ptr<Expression>> constructorArgs;
                        constructorArgs.push_back(std::move(args.back()));
                        args.pop_back();
                        args.emplace_back(new Constructor(offset, expr->fType.toCompound(fContext,
                                                                                         count,
                                                                                         1),
                                                          std::move(constructorArgs)));
                    }
                    break;
                }
                case SKSL_SWIZZLE_0:
                    args.emplace_back(new IntLiteral(fContext, offset, 0));
                    break;
                case SKSL_SWIZZLE_1:
                    args.emplace_back(new IntLiteral(fContext, offset, 1));
                    break;
            }
        }
        return std::unique_ptr<Expression>(new Constructor(offset,
                                                           expr->fType.toCompound(
                                                                           fContext,
                                                                           swizzleComponents.size(),
                                                                           1),
                                                           std::move(args)));
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

std::unique_ptr<Expression> IRGenerator::findEnumRef(
                                           int offset,
                                           const Type& type,
                                           StringFragment field,
                                           std::vector<std::unique_ptr<ProgramElement>>& elements) {
    for (const auto& e : elements) {
        if (e->fKind == ProgramElement::kEnum_Kind && type.name() == e->as<Enum>().fTypeName) {
            std::shared_ptr<SymbolTable> old = fSymbolTable;
            fSymbolTable = e->as<Enum>().fSymbols;
            std::unique_ptr<Expression> result = convertIdentifier(ASTNode(&fFile->fNodes, offset,
                                                                         ASTNode::Kind::kIdentifier,
                                                                         field));
            if (result) {
                const Variable& v = result->as<VariableReference>().fVariable;
                SkASSERT(v.fInitialValue);
                result = std::make_unique<IntLiteral>(
                        offset, v.fInitialValue->as<IntLiteral>().fValue, &type);
            }
            fSymbolTable = old;
            return result;
        }
    }
    return nullptr;
}

std::unique_ptr<Expression> IRGenerator::convertTypeField(int offset, const Type& type,
                                                          StringFragment field) {
    std::unique_ptr<Expression> result = this->findEnumRef(offset, type, field, *fProgramElements);
    if (fInherited && !result) {
        result = this->findEnumRef(offset, type, field, *fInherited);
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
        const Type& oldType = base->as<TypeReference>().fValue;
        const Type* newType = fSymbolTable->takeOwnershipOfSymbol(std::make_unique<Type>(
                oldType.name() + "[]", Type::kArray_Kind, oldType, /*columns=*/-1));
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
    if (base->fKind == Expression::kTypeReference_Kind) {
        return this->convertTypeField(base->fOffset, base->as<TypeReference>().fValue,
                                      field);
    }
    if (base->fKind == Expression::kExternalValue_Kind) {
        return this->convertField(std::move(base), field);
    }
    switch (base->fType.kind()) {
        case Type::kOther_Kind:
        case Type::kStruct_Kind:
            return this->convertField(std::move(base), field);
        default:
            return this->convertSwizzle(std::move(base), field);
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
                      "' cannot operate on '" + base->fType.displayName() + "'");
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

bool IRGenerator::setRefKind(Expression& expr, VariableReference::RefKind kind) {
    switch (expr.fKind) {
        case Expression::kVariableReference_Kind: {
            const Variable& var = expr.as<VariableReference>().fVariable;
            if (var.fModifiers.fFlags &
                (Modifiers::kConst_Flag | Modifiers::kUniform_Flag | Modifiers::kVarying_Flag)) {
                fErrors.error(expr.fOffset, "cannot modify immutable variable '" + var.fName + "'");
                return false;
            }
            expr.as<VariableReference>().setRefKind(kind);
            return true;
        }
        case Expression::kFieldAccess_Kind:
            return this->setRefKind(*expr.as<FieldAccess>().fBase, kind);
        case Expression::kSwizzle_Kind: {
            const Swizzle& swizzle = expr.as<Swizzle>();
            return this->checkSwizzleWrite(swizzle) && this->setRefKind(*swizzle.fBase, kind);
        }
        case Expression::kIndex_Kind:
            return this->setRefKind(*expr.as<IndexExpression>().fBase, kind);
        case Expression::kTernary_Kind: {
            const TernaryExpression& t = expr.as<TernaryExpression>();
            return this->setRefKind(*t.fIfTrue, kind) && this->setRefKind(*t.fIfFalse, kind);
        }
        case Expression::kExternalValue_Kind: {
            const ExternalValue& v = *expr.as<ExternalValueReference>().fValue;
            if (!v.canWrite()) {
                fErrors.error(expr.fOffset,
                              "cannot modify immutable external value '" + v.fName + "'");
                return false;
            }
            return true;
        }
        default:
            fErrors.error(expr.fOffset, "cannot assign to this expression");
            return false;
    }
}

void IRGenerator::convertProgram(Program::Kind kind,
                                 const char* text,
                                 size_t length,
                                 std::vector<std::unique_ptr<ProgramElement>>* out) {
    fKind = kind;
    fProgramElements = out;
    Parser parser(text, length, *fSymbolTable, fErrors);
    fFile = parser.file();
    if (fErrors.errorCount()) {
        return;
    }
    this->pushSymbolTable(); // this is popped by Compiler upon completion
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
#ifdef SK_DEBUG
                ABORT("unsupported declaration: %s\n", decl.description().c_str());
#endif
                break;
        }
    }
}


}  // namespace SkSL
