/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLIRGenerator.h"

#include "limits.h"
#include <iterator>
#include <memory>
#include <unordered_set>

#include "include/private/SkTArray.h"
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
#include "src/sksl/ir/SkSLFunctionPrototype.h"
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
#include "src/sksl/ir/SkSLStructDefinition.h"
#include "src/sksl/ir/SkSLSwitchCase.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLUnresolvedFunction.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"

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

static void fill_caps(const SkSL::ShaderCapsClass& caps,
                      std::unordered_map<String, Program::Settings::Value>* capsMap) {
#define CAP(name) capsMap->insert({String(#name), Program::Settings::Value(caps.name())})
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
    CAP(builtinFMASupport);
    CAP(builtinDeterminantSupport);
#undef CAP
}

IRGenerator::IRGenerator(const Context* context,
                         const ShaderCapsClass* caps,
                         ErrorReporter& errorReporter)
        : fContext(*context)
        , fCaps(caps)
        , fErrors(errorReporter)
        , fModifiers(new ModifiersPool()) {
    if (fCaps) {
        fill_caps(*fCaps, &fCapsMap);
    } else {
        fCapsMap.insert({String("integerSupport"), Program::Settings::Value(true)});
    }

}

void IRGenerator::pushSymbolTable() {
    auto childSymTable = std::make_shared<SymbolTable>(std::move(fSymbolTable), fIsBuiltinCode);
    fSymbolTable = std::move(childSymTable);
}

void IRGenerator::popSymbolTable() {
    fSymbolTable = fSymbolTable->fParent;
}

std::unique_ptr<Extension> IRGenerator::convertExtension(int offset, StringFragment name) {
    if (fKind != Program::kFragment_Kind &&
        fKind != Program::kVertex_Kind &&
        fKind != Program::kGeometry_Kind) {
        fErrors.error(offset, "extensions are not allowed here");
        return nullptr;
    }

    return std::make_unique<Extension>(offset, name);
}

std::unique_ptr<ModifiersPool> IRGenerator::releaseModifiers() {
    std::unique_ptr<ModifiersPool> result = std::move(fModifiers);
    fModifiers = std::make_unique<ModifiersPool>();
    return result;
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
        case ASTNode::Kind::kType:
            // TODO: add IRNode for struct definition inside a function
            return nullptr;
        default:
            // it's an expression
            std::unique_ptr<Statement> result = this->convertExpressionStatement(statement);
            if (fRTAdjust && fKind == Program::kGeometry_Kind) {
                SkASSERT(result->kind() == Statement::Kind::kExpression);
                Expression& expr = *result->as<ExpressionStatement>().expression();
                if (expr.kind() == Expression::Kind::kFunctionCall) {
                    FunctionCall& fc = expr.as<FunctionCall>();
                    if (fc.function().isBuiltin() && fc.function().name() == "EmitVertex") {
                        StatementArray statements;
                        statements.reserve_back(2);
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
    StatementArray oldExtraStatements = std::move(fExtraStatements);
    std::unique_ptr<Statement> result = this->convertSingleStatement(statement);
    if (!result) {
        fExtraStatements = std::move(oldExtraStatements);
        return nullptr;
    }
    if (fExtraStatements.size()) {
        fExtraStatements.push_back(std::move(result));
        auto block = std::make_unique<Block>(/*offset=*/-1, std::move(fExtraStatements),
                                             /*symbols=*/nullptr, /*isScope=*/false);
        fExtraStatements = std::move(oldExtraStatements);
        return std::move(block);
    }
    fExtraStatements = std::move(oldExtraStatements);
    return result;
}

std::unique_ptr<Block> IRGenerator::convertBlock(const ASTNode& block) {
    SkASSERT(block.fKind == ASTNode::Kind::kBlock);
    AutoSymbolTable table(this);
    StatementArray statements;
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
    auto decls = this->convertVarDeclarations(s, Variable::Storage::kLocal);
    if (decls.empty()) {
        return nullptr;
    }
    if (decls.size() == 1) {
        return std::move(decls.front());
    } else {
        return std::make_unique<Block>(s.fOffset, std::move(decls), /*symbols=*/nullptr,
                                       /*isScope=*/false);
    }
}

StatementArray IRGenerator::convertVarDeclarations(const ASTNode& decls,
                                                   Variable::Storage storage) {
    SkASSERT(decls.fKind == ASTNode::Kind::kVarDeclarations);
    auto declarationsIter = decls.begin();
    const Modifiers& modifiers = declarationsIter++->getModifiers();
    const ASTNode& rawType = *(declarationsIter++);
    const Type* baseType = this->convertType(rawType);
    if (!baseType) {
        return {};
    }
    if (baseType->nonnullable() == *fContext.fFragmentProcessor_Type &&
        storage != Variable::Storage::kGlobal) {
        fErrors.error(decls.fOffset,
                      "variables of type '" + baseType->displayName() + "' must be global");
    }
    if (fKind != Program::kFragmentProcessor_Kind) {
        if ((modifiers.fFlags & Modifiers::kIn_Flag) && baseType->isMatrix()) {
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
            return t.isVector() && t.componentType().isFloat() &&
                   (t.columns() == 3 || t.columns() == 4);
        };
        if (!validColorXformType(*baseType) && !(baseType->isArray() &&
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
            !(baseType->isVector() && baseType->componentType().isFloat())) {
            fErrors.error(decls.fOffset, "'varying' must be float scalar or vector");
        }
    }
    int permitted = Modifiers::kConst_Flag;
    if (storage == Variable::Storage::kGlobal) {
        permitted |= Modifiers::kIn_Flag | Modifiers::kOut_Flag | Modifiers::kUniform_Flag |
                     Modifiers::kFlat_Flag | Modifiers::kVarying_Flag |
                     Modifiers::kNoPerspective_Flag | Modifiers::kPLS_Flag |
                     Modifiers::kPLSIn_Flag | Modifiers::kPLSOut_Flag |
                     Modifiers::kRestrict_Flag | Modifiers::kVolatile_Flag |
                     Modifiers::kReadOnly_Flag | Modifiers::kWriteOnly_Flag |
                     Modifiers::kCoherent_Flag | Modifiers::kBuffer_Flag;
    }
    this->checkModifiers(decls.fOffset, modifiers, permitted);

    StatementArray varDecls;
    for (; declarationsIter != decls.end(); ++declarationsIter) {
        const ASTNode& varDecl = *declarationsIter;
        if (modifiers.fLayout.fLocation == 0 && modifiers.fLayout.fIndex == 0 &&
            (modifiers.fFlags & Modifiers::kOut_Flag) && fKind == Program::kFragment_Kind &&
            varDecl.getVarData().fName != "sk_FragColor") {
            fErrors.error(varDecl.fOffset,
                          "out location=0, index=0 is reserved for sk_FragColor");
        }
        const ASTNode::VarData& varData = varDecl.getVarData();
        const Type* type = baseType;
        int arraySize = 0;
        auto iter = varDecl.begin();
        if (iter != varDecl.end()) {
            if (varData.fIsArray) {
                if (type->isOpaque()) {
                    fErrors.error(varDecl.fOffset,
                                  "opaque type '" + type->name() + "' may not be used in an array");
                }
                const ASTNode& rawSize = *iter++;
                if (rawSize) {
                    auto size = this->coerce(this->convertExpression(rawSize), *fContext.fInt_Type);
                    if (!size) {
                        return {};
                    }
                    String name(type->name());
                    int64_t count;
                    if (!size->is<IntLiteral>()) {
                        fErrors.error(size->fOffset, "array size must be an integer");
                        return {};
                    }
                    count = size->as<IntLiteral>().value();
                    if (count <= 0) {
                        fErrors.error(size->fOffset, "array size must be positive");
                        return {};
                    }
                    arraySize = count;
                } else {
                    arraySize = Type::kUnsizedArray;
                }
                type = fSymbolTable->addArrayDimension(type, arraySize);
            }
        }
        auto var = std::make_unique<Variable>(varDecl.fOffset, fModifiers->addToPool(modifiers),
                                              varData.fName, type, fIsBuiltinCode, storage);
        if (var->name() == Compiler::RTADJUST_NAME) {
            SkASSERT(!fRTAdjust);
            SkASSERT(var->type() == *fContext.fFloat4_Type);
            fRTAdjust = var.get();
        }
        std::unique_ptr<Expression> value;
        if (iter == varDecl.end()) {
            if (arraySize == Type::kUnsizedArray) {
                fErrors.error(varDecl.fOffset,
                              "arrays without an explicit size must use an initializer expression");
                return {};
            }
        } else {
            value = this->convertExpression(*iter);
            if (!value) {
                return {};
            }
            if (modifiers.fFlags & Modifiers::kIn_Flag) {
                fErrors.error(value->fOffset, "'in' variables cannot use initializer expressions");
            }
            value = this->coerce(std::move(value), *type);
            if (!value) {
                return {};
            }
            var->setInitialValue(value.get());
        }
        const Symbol* symbol = (*fSymbolTable)[var->name()];
        if (symbol && storage == Variable::Storage::kGlobal && var->name() == "sk_FragColor") {
            // Already defined, ignore.
        } else {
            varDecls.push_back(std::make_unique<VarDeclaration>(var.get(), baseType, arraySize,
                                                                std::move(value)));
            fSymbolTable->add(std::move(var));
        }
    }
    return varDecls;
}

std::unique_ptr<ModifiersDeclaration> IRGenerator::convertModifiersDeclaration(const ASTNode& m) {
    if (fKind != Program::kFragment_Kind &&
        fKind != Program::kVertex_Kind &&
        fKind != Program::kGeometry_Kind) {
        fErrors.error(m.fOffset, "layout qualifiers are not allowed here");
        return nullptr;
    }

    SkASSERT(m.fKind == ASTNode::Kind::kModifiers);
    Modifiers modifiers = m.getModifiers();
    if (modifiers.fLayout.fInvocations != -1) {
        if (fKind != Program::kGeometry_Kind) {
            fErrors.error(m.fOffset, "'invocations' is only legal in geometry shaders");
            return nullptr;
        }
        fInvocations = modifiers.fLayout.fInvocations;
        if (fCaps && !fCaps->gsInvocationsSupport()) {
            modifiers.fLayout.fInvocations = -1;
            if (modifiers.fLayout.description() == "") {
                return nullptr;
            }
        }
    }
    if (modifiers.fLayout.fMaxVertices != -1 && fInvocations > 0 && fCaps &&
        !fCaps->gsInvocationsSupport()) {
        modifiers.fLayout.fMaxVertices *= fInvocations;
    }
    return std::make_unique<ModifiersDeclaration>(fModifiers->addToPool(modifiers));
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
    if (test->kind() == Expression::Kind::kBoolLiteral) {
        // static boolean value, fold down to a single branch
        if (test->as<BoolLiteral>().value()) {
            return ifTrue;
        } else if (ifFalse) {
            return ifFalse;
        } else {
            // False & no else clause. Not an error, so don't return null!
            return std::make_unique<Nop>();
        }
    }
    return std::make_unique<IfStatement>(n.fOffset, n.getBool(), std::move(test),
                                         std::move(ifTrue), std::move(ifFalse));
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
    return std::make_unique<ForStatement>(w.fOffset, /*initializer=*/nullptr, std::move(test),
                                          /*next=*/nullptr, std::move(statement), fSymbolTable);
}

std::unique_ptr<Statement> IRGenerator::convertDo(const ASTNode& d) {
    SkASSERT(d.fKind == ASTNode::Kind::kDo);
    AutoLoopLevel level(this);
    auto iter = d.begin();
    std::unique_ptr<Statement> statement = this->convertStatement(*(iter++));
    if (!statement) {
        return nullptr;
    }
    std::unique_ptr<Expression> test =
            this->coerce(this->convertExpression(*(iter++)), *fContext.fBool_Type);
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
    if (value->type() != *fContext.fUInt_Type &&
        value->type().typeKind() != Type::TypeKind::kEnum) {
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
            caseValue = this->coerce(std::move(caseValue), value->type());
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
        StatementArray statements;
        for (; childIter != c.end(); ++childIter) {
            std::unique_ptr<Statement> converted = this->convertStatement(*childIter);
            if (!converted) {
                return nullptr;
            }
            statements.push_back(std::move(converted));
        }
        cases.push_back(std::make_unique<SwitchCase>(c.fOffset, std::move(caseValue),
                                                     std::move(statements)));
    }
    return std::make_unique<SwitchStatement>(s.fOffset, s.getBool(), std::move(value),
                                             std::move(cases), fSymbolTable);
}

std::unique_ptr<Statement> IRGenerator::convertExpressionStatement(const ASTNode& s) {
    std::unique_ptr<Expression> e = this->convertExpression(s);
    if (!e) {
        return nullptr;
    }
    return std::unique_ptr<Statement>(new ExpressionStatement(std::move(e)));
}

std::unique_ptr<Statement> IRGenerator::convertReturn(const ASTNode& r) {
    SkASSERT(r.fKind == ASTNode::Kind::kReturn);
    SkASSERT(fCurrentFunction);
    // early returns from a vertex main function will bypass the sk_Position normalization, so
    // SkASSERT that we aren't doing that. It is of course possible to fix this by adding a
    // normalization before each return, but it will probably never actually be necessary.
    SkASSERT(Program::kVertex_Kind != fKind || !fRTAdjust || "main" != fCurrentFunction->name());
    if (r.begin() != r.end()) {
        std::unique_ptr<Expression> result = this->convertExpression(*r.begin());
        if (!result) {
            return nullptr;
        }
        if (fCurrentFunction->returnType() == *fContext.fVoid_Type) {
            fErrors.error(result->fOffset, "may not return a value from a void function");
            return nullptr;
        } else {
            result = this->coerce(std::move(result), fCurrentFunction->returnType());
            if (!result) {
                return nullptr;
            }
        }
        return std::unique_ptr<Statement>(new ReturnStatement(std::move(result)));
    } else {
        if (fCurrentFunction->returnType() != *fContext.fVoid_Type) {
            fErrors.error(r.fOffset, "expected function to return '" +
                                     fCurrentFunction->returnType().displayName() + "'");
        }
        return std::unique_ptr<Statement>(new ReturnStatement(r.fOffset));
    }
}

std::unique_ptr<Statement> IRGenerator::convertBreak(const ASTNode& b) {
    SkASSERT(b.fKind == ASTNode::Kind::kBreak);
    if (fLoopLevel > 0 || fSwitchLevel > 0) {
        return std::make_unique<BreakStatement>(b.fOffset);
    } else {
        fErrors.error(b.fOffset, "break statement must be inside a loop or switch");
        return nullptr;
    }
}

std::unique_ptr<Statement> IRGenerator::convertContinue(const ASTNode& c) {
    SkASSERT(c.fKind == ASTNode::Kind::kContinue);
    if (fLoopLevel > 0) {
        return std::make_unique<ContinueStatement>(c.fOffset);
    } else {
        fErrors.error(c.fOffset, "continue statement must be inside a loop");
        return nullptr;
    }
}

std::unique_ptr<Statement> IRGenerator::convertDiscard(const ASTNode& d) {
    SkASSERT(d.fKind == ASTNode::Kind::kDiscard);
    if (fKind != Program::kFragment_Kind && fKind != Program::kFragmentProcessor_Kind) {
        fErrors.error(d.fOffset, "discard statement is only permitted in fragment shaders");
        return nullptr;
    }
    return std::make_unique<DiscardStatement>(d.fOffset);
}

std::unique_ptr<Block> IRGenerator::applyInvocationIDWorkaround(std::unique_ptr<Block> main) {
    Layout invokeLayout;
    Modifiers invokeModifiers(invokeLayout, Modifiers::kHasSideEffects_Flag);
    const FunctionDeclaration* invokeDecl = fSymbolTable->add(std::make_unique<FunctionDeclaration>(
            /*offset=*/-1,
            fModifiers->addToPool(invokeModifiers),
            "_invoke",
            std::vector<const Variable*>(),
            fContext.fVoid_Type.get(),
            fIsBuiltinCode));
    fProgramElements->push_back(std::make_unique<FunctionDefinition>(/*offset=*/-1,
                                                                     invokeDecl, fIsBuiltinCode,
                                                                     std::move(main)));

    std::vector<std::unique_ptr<VarDeclaration>> variables;
    const Variable* loopIdx = &(*fSymbolTable)["sk_InvocationID"]->as<Variable>();
    auto test = std::make_unique<BinaryExpression>(/*offset=*/-1,
                    std::make_unique<VariableReference>(/*offset=*/-1, loopIdx),
                    Token::Kind::TK_LT,
                    std::make_unique<IntLiteral>(fContext, /*offset=*/-1, fInvocations),
                    fContext.fBool_Type.get());
    auto next = std::make_unique<PostfixExpression>(
            std::make_unique<VariableReference>(/*offset=*/-1, loopIdx,
                                                VariableReference::RefKind::kReadWrite),
            Token::Kind::TK_PLUSPLUS);
    ASTNode endPrimitiveID(&fFile->fNodes, -1, ASTNode::Kind::kIdentifier, "EndPrimitive");
    std::unique_ptr<Expression> endPrimitive = this->convertExpression(endPrimitiveID);
    SkASSERT(endPrimitive);

    StatementArray loopBody;
    loopBody.reserve_back(2);
    loopBody.push_back(std::make_unique<ExpressionStatement>(this->call(
                                                    /*offset=*/-1, *invokeDecl,
                                                    ExpressionArray{})));
    loopBody.push_back(std::make_unique<ExpressionStatement>(this->call(
                                                    /*offset=*/-1, std::move(endPrimitive),
                                                    ExpressionArray{})));
    auto assignment = std::make_unique<BinaryExpression>(/*offset=*/-1,
                    std::make_unique<VariableReference>(/*offset=*/-1, loopIdx,
                                                        VariableReference::RefKind::kWrite),
                    Token::Kind::TK_EQ,
                    std::make_unique<IntLiteral>(fContext, /*offset=*/-1, /*value=*/0),
                    fContext.fInt_Type.get());
    auto initializer = std::make_unique<ExpressionStatement>(std::move(assignment));
    auto loop = std::make_unique<ForStatement>(/*offset=*/-1,
                                               std::move(initializer),
                                               std::move(test), std::move(next),
                                               std::make_unique<Block>(-1, std::move(loopBody)),
                                               fSymbolTable);
    StatementArray children;
    children.push_back(std::move(loop));
    return std::make_unique<Block>(-1, std::move(children));
}

std::unique_ptr<Statement> IRGenerator::getNormalizeSkPositionCode() {
    const Variable* skPerVertex = nullptr;
    if (const ProgramElement* perVertexDecl = fIntrinsics->find(Compiler::PERVERTEX_NAME)) {
        SkASSERT(perVertexDecl->is<InterfaceBlock>());
        skPerVertex = &perVertexDecl->as<InterfaceBlock>().variable();
    }

    // sk_Position = float4(sk_Position.xy * rtAdjust.xz + sk_Position.ww * rtAdjust.yw,
    //                      0,
    //                      sk_Position.w);
    SkASSERT(skPerVertex && fRTAdjust);
    auto Ref = [](const Variable* var) -> std::unique_ptr<Expression> {
        return std::make_unique<VariableReference>(-1, var, VariableReference::RefKind::kRead);
    };
    auto WRef = [](const Variable* var) -> std::unique_ptr<Expression> {
        return std::make_unique<VariableReference>(-1, var, VariableReference::RefKind::kWrite);
    };
    auto Field = [&](const Variable* var, int idx) -> std::unique_ptr<Expression> {
        return std::make_unique<FieldAccess>(Ref(var), idx,
                                             FieldAccess::OwnerKind::kAnonymousInterfaceBlock);
    };
    auto Pos = [&]() -> std::unique_ptr<Expression> {
        return std::make_unique<FieldAccess>(WRef(skPerVertex), 0,
                                             FieldAccess::OwnerKind::kAnonymousInterfaceBlock);
    };
    auto Adjust = [&]() -> std::unique_ptr<Expression> {
        return fRTAdjustInterfaceBlock ? Field(fRTAdjustInterfaceBlock, fRTAdjustFieldIndex)
                                       : Ref(fRTAdjust);
    };
    auto Swizzle = [&](std::unique_ptr<Expression> expr,
                       const ComponentArray& comp) -> std::unique_ptr<Expression> {
        return std::make_unique<SkSL::Swizzle>(fContext, std::move(expr), comp);
    };
    auto Op = [&](std::unique_ptr<Expression> left, Token::Kind op,
                  std::unique_ptr<Expression> right) -> std::unique_ptr<Expression> {
        return std::make_unique<BinaryExpression>(-1, std::move(left), op, std::move(right),
                                                  fContext.fFloat2_Type.get());
    };

    static const ComponentArray kXYIndices{0, 1};
    static const ComponentArray kXZIndices{0, 2};
    static const ComponentArray kYWIndices{1, 3};
    static const ComponentArray kWWIndices{3, 3};
    static const ComponentArray kWIndex{3};

    ExpressionArray children;
    children.reserve_back(3);
    children.push_back(Op(
            Op(Swizzle(Pos(), kXYIndices), Token::Kind::TK_STAR, Swizzle(Adjust(), kXZIndices)),
            Token::Kind::TK_PLUS,
            Op(Swizzle(Pos(), kWWIndices), Token::Kind::TK_STAR, Swizzle(Adjust(), kYWIndices))));
    children.push_back(std::make_unique<FloatLiteral>(fContext, /*offset=*/-1, /*value=*/0.0));
    children.push_back(Swizzle(Pos(), kWIndex));
    std::unique_ptr<Expression> result = Op(Pos(), Token::Kind::TK_EQ,
                                 std::make_unique<Constructor>(/*offset=*/-1,
                                                               fContext.fFloat4_Type.get(),
                                                               std::move(children)));
    return std::make_unique<ExpressionStatement>(std::move(result));
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
    const Type* returnType = this->convertType(*(iter++), /*allowVoid=*/true);
    if (returnType == nullptr) {
        return;
    }
    auto typeIsAllowed = [&](const Type* t) {
#if defined(SKSL_STANDALONE)
        return true;
#else
        GrSLType unusedSLType;
        return fKind != Program::kPipelineStage_Kind ||
               type_to_grsltype(fContext, *t, &unusedSLType);
#endif
    };
    if (returnType->isArray() || !typeIsAllowed(returnType) ||
        returnType->nonnullable() == *fContext.fFragmentProcessor_Type) {
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
        if (pd.fIsArray) {
            int arraySize = (paramIter++)->getInt();
            type = fSymbolTable->addArrayDimension(type, arraySize);
        }
        // Only the (builtin) declarations of 'sample' are allowed to have FP parameters
        if ((type->nonnullable() == *fContext.fFragmentProcessor_Type && !fIsBuiltinCode) ||
            !typeIsAllowed(type)) {
            fErrors.error(param.fOffset,
                          "parameters of type '" + type->displayName() + "' not allowed");
            return;
        }

        Modifiers m = pd.fModifiers;
        if (funcData.fName == "main" && (fKind == Program::kPipelineStage_Kind ||
                                         fKind == Program::kFragmentProcessor_Kind)) {
            if (i == 0) {
                // We verify that the type is correct later, for now, if there is a parameter to
                // a .fp or runtime-effect main(), it's supposed to be the coords:
                m.fLayout.fBuiltin = SK_MAIN_COORDS_BUILTIN;
            }
        }

        const Variable* var = fSymbolTable->takeOwnershipOfSymbol(
                std::make_unique<Variable>(param.fOffset, fModifiers->addToPool(m), pd.fName, type,
                                           fIsBuiltinCode, Variable::Storage::kParameter));
        parameters.push_back(var);
    }

    auto paramIsCoords = [&](int idx) {
        return parameters[idx]->type() == *fContext.fFloat2_Type &&
               parameters[idx]->modifiers().fFlags == 0 &&
               parameters[idx]->modifiers().fLayout.fBuiltin == SK_MAIN_COORDS_BUILTIN;
    };

    if (funcData.fName == "main") {
        switch (fKind) {
            case Program::kPipelineStage_Kind: {
                // (half4|float4) main()  -or-  (half4|float4) main(float2)
                if (*returnType != *fContext.fHalf4_Type && *returnType != *fContext.fFloat4_Type) {
                    fErrors.error(f.fOffset, "'main' must return: 'vec4', 'float4', or 'half4'");
                    return;
                }
                bool validParams = (parameters.size() == 0) ||
                                   (parameters.size() == 1 && paramIsCoords(0));
                if (!validParams) {
                    fErrors.error(f.fOffset, "'main' parameters must be: (), (vec2), or (float2)");
                    return;
                }
                break;
            }
            case Program::kFragmentProcessor_Kind: {
                if (*returnType != *fContext.fHalf4_Type) {
                    fErrors.error(f.fOffset, ".fp 'main' must return 'half4'");
                    return;
                }
                bool validParams = (parameters.size() == 0) ||
                                   (parameters.size() == 1 && paramIsCoords(0));
                if (!validParams) {
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
                break;
        }
    }

    // find existing declaration
    const FunctionDeclaration* decl = nullptr;
    const Symbol* entry = (*fSymbolTable)[funcData.fName];
    if (entry) {
        std::vector<const FunctionDeclaration*> functions;
        switch (entry->kind()) {
            case Symbol::Kind::kUnresolvedFunction:
                functions = entry->as<UnresolvedFunction>().functions();
                break;
            case Symbol::Kind::kFunctionDeclaration:
                functions.push_back(&entry->as<FunctionDeclaration>());
                break;
            default:
                fErrors.error(f.fOffset, "symbol '" + funcData.fName + "' was already defined");
                return;
        }
        for (const FunctionDeclaration* other : functions) {
            SkASSERT(other->name() == funcData.fName);
            if (parameters.size() == other->parameters().size()) {
                bool match = true;
                for (size_t i = 0; i < parameters.size(); i++) {
                    if (parameters[i]->type() != other->parameters()[i]->type()) {
                        match = false;
                        break;
                    }
                }
                if (match) {
                    if (*returnType != other->returnType()) {
                        FunctionDeclaration newDecl(f.fOffset,
                                                    fModifiers->addToPool(funcData.fModifiers),
                                                    funcData.fName,
                                                    parameters,
                                                    returnType,
                                                    fIsBuiltinCode);
                        fErrors.error(f.fOffset, "functions '" + newDecl.description() +
                                                 "' and '" + other->description() +
                                                 "' differ only in return type");
                        return;
                    }
                    decl = other;
                    for (size_t i = 0; i < parameters.size(); i++) {
                        if (parameters[i]->modifiers() != other->parameters()[i]->modifiers()) {
                            fErrors.error(f.fOffset, "modifiers on parameter " +
                                                     to_string((uint64_t) i + 1) +
                                                     " differ between declaration and definition");
                            return;
                        }
                    }
                    if (other->definition() && !other->isBuiltin()) {
                        fErrors.error(f.fOffset, "duplicate definition of " + other->description());
                        return;
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
        decl = fSymbolTable->add(
                std::make_unique<FunctionDeclaration>(f.fOffset,
                                                      fModifiers->addToPool(declModifiers),
                                                      funcData.fName,
                                                      parameters,
                                                      returnType,
                                                      fIsBuiltinCode));
    }
    if (iter == f.end()) {
        // If there's no body, we've found a prototype.
        fProgramElements->push_back(std::make_unique<FunctionPrototype>(f.fOffset, decl,
                                                                        fIsBuiltinCode));
    } else {
        // Compile function body.
        SkASSERT(!fCurrentFunction);
        fCurrentFunction = decl;

        AutoSymbolTable table(this);
        for (const Variable* param : decl->parameters()) {
            fSymbolTable->addWithoutOwnership(param);
        }
        bool needInvocationIDWorkaround = fInvocations != -1 && funcData.fName == "main" &&
                                          fCaps && !fCaps->gsInvocationsSupport();
        std::unique_ptr<Block> body = this->convertBlock(*iter);
        fCurrentFunction = nullptr;
        if (!body) {
            return;
        }
        if (needInvocationIDWorkaround) {
            body = this->applyInvocationIDWorkaround(std::move(body));
        }
        if (Program::kVertex_Kind == fKind && funcData.fName == "main" && fRTAdjust) {
            body->children().push_back(this->getNormalizeSkPositionCode());
        }
        auto result = std::make_unique<FunctionDefinition>(
                f.fOffset, decl, fIsBuiltinCode, std::move(body), std::move(fReferencedIntrinsics));
        decl->setDefinition(result.get());
        result->setSource(&f);
        fProgramElements->push_back(std::move(result));
    }
}

std::unique_ptr<StructDefinition> IRGenerator::convertStructDefinition(const ASTNode& node) {
    SkASSERT(node.fKind == ASTNode::Kind::kType);

    const Type* type = this->convertType(node);
    if (!type) {
        return nullptr;
    }
    if (!type->isStruct()) {
        fErrors.error(node.fOffset, "expected a struct here, found '" + type->name() + "'");
        return nullptr;
    }
    return std::make_unique<StructDefinition>(node.fOffset, *type);
}

std::unique_ptr<InterfaceBlock> IRGenerator::convertInterfaceBlock(const ASTNode& intf) {
    if (fKind != Program::kFragment_Kind &&
        fKind != Program::kVertex_Kind &&
        fKind != Program::kGeometry_Kind) {
        fErrors.error(intf.fOffset, "interface block is not allowed here");
        return nullptr;
    }

    SkASSERT(intf.fKind == ASTNode::Kind::kInterfaceBlock);
    ASTNode::InterfaceBlockData id = intf.getInterfaceBlockData();
    std::shared_ptr<SymbolTable> old = fSymbolTable;
    std::shared_ptr<SymbolTable> symbols;
    std::vector<Type::Field> fields;
    bool foundRTAdjust = false;
    auto iter = intf.begin();
    {
        AutoSymbolTable table(this);
        symbols = fSymbolTable;
        bool haveRuntimeArray = false;
        for (size_t i = 0; i < id.fDeclarationCount; ++i) {
            StatementArray decls = this->convertVarDeclarations(*(iter++),
                                                                Variable::Storage::kInterfaceBlock);
            if (decls.empty()) {
                return nullptr;
            }
            for (const auto& decl : decls) {
                const VarDeclaration& vd = decl->as<VarDeclaration>();
                if (vd.var().type().isOpaque()) {
                    fErrors.error(decl->fOffset, "opaque type '" + vd.var().type().name() +
                                                 "' is not permitted in an interface block");
                }
                if (haveRuntimeArray) {
                    fErrors.error(decl->fOffset,
                                "only the last entry in an interface block may be a runtime-sized "
                                "array");
                }
                if (&vd.var() == fRTAdjust) {
                    foundRTAdjust = true;
                    SkASSERT(vd.var().type() == *fContext.fFloat4_Type);
                    fRTAdjustFieldIndex = fields.size();
                }
                fields.push_back(Type::Field(vd.var().modifiers(), vd.var().name(),
                                            &vd.var().type()));
                if (vd.value()) {
                    fErrors.error(decl->fOffset,
                                "initializers are not permitted on interface block fields");
                }
                if (vd.var().type().isArray() &&
                    vd.var().type().columns() == Type::kUnsizedArray) {
                    haveRuntimeArray = true;
                }
            }
        }
    }
    const Type* type = old->takeOwnershipOfSymbol(Type::MakeStructType(intf.fOffset, id.fTypeName,
                                                                       fields));
    int arraySize = 0;
    if (id.fIsArray) {
        const ASTNode& size = *(iter++);
        if (size) {
            std::unique_ptr<Expression> converted = this->convertExpression(size);
            if (!converted) {
                return nullptr;
            }
            if (!converted->is<IntLiteral>()) {
                fErrors.error(intf.fOffset, "array size must be specified");
                return nullptr;
            }
            arraySize = converted->as<IntLiteral>().value();
            if (arraySize <= 0) {
                fErrors.error(converted->fOffset, "array size must be positive");
                return nullptr;
            }
        } else {
            arraySize = Type::kUnsizedArray;
        }
        type = symbols->addArrayDimension(type, arraySize);
    }
    const Variable* var = old->takeOwnershipOfSymbol(
            std::make_unique<Variable>(intf.fOffset,
                                       fModifiers->addToPool(id.fModifiers),
                                       id.fInstanceName.fLength ? id.fInstanceName : id.fTypeName,
                                       type,
                                       fIsBuiltinCode,
                                       Variable::Storage::kGlobal));
    if (foundRTAdjust) {
        fRTAdjustInterfaceBlock = var;
    }
    if (id.fInstanceName.fLength) {
        old->addWithoutOwnership(var);
    } else {
        for (size_t i = 0; i < fields.size(); i++) {
            old->add(std::make_unique<Field>(intf.fOffset, var, (int)i));
        }
    }
    return std::make_unique<InterfaceBlock>(intf.fOffset,
                                            var,
                                            id.fTypeName,
                                            id.fInstanceName,
                                            arraySize,
                                            symbols);
}

bool IRGenerator::getConstantInt(const Expression& value, int64_t* out) {
    switch (value.kind()) {
        case Expression::Kind::kIntLiteral:
            *out = value.as<IntLiteral>().value();
            return true;
        case Expression::Kind::kVariableReference: {
            const Variable& var = *value.as<VariableReference>().variable();
            return (var.modifiers().fFlags & Modifiers::kConst_Flag) &&
                   var.initialValue() && this->getConstantInt(*var.initialValue(), out);
        }
        default:
            return false;
    }
}

void IRGenerator::convertGlobalVarDeclarations(const ASTNode& decl) {
    StatementArray decls = this->convertVarDeclarations(decl, Variable::Storage::kGlobal);
    for (std::unique_ptr<Statement>& stmt : decls) {
        fProgramElements->push_back(std::make_unique<GlobalVarDeclaration>(decl.fOffset,
                                                                           std::move(stmt)));
    }
}

void IRGenerator::convertEnum(const ASTNode& e) {
    if (fKind == Program::kPipelineStage_Kind) {
        fErrors.error(e.fOffset, "enum is not allowed here");
        return;
    }

    SkASSERT(e.fKind == ASTNode::Kind::kEnum);
    int64_t currentValue = 0;
    Layout layout;
    ASTNode enumType(
            e.fNodes, e.fOffset, ASTNode::Kind::kType,
            ASTNode::TypeData(e.getString(), /*isStructDeclaration=*/false, /*isNullable=*/false));
    const Type* type = this->convertType(enumType);
    Modifiers modifiers(layout, Modifiers::kConst_Flag);
    std::shared_ptr<SymbolTable> oldTable = fSymbolTable;
    fSymbolTable = std::make_shared<SymbolTable>(fSymbolTable, fIsBuiltinCode);
    for (auto iter = e.begin(); iter != e.end(); ++iter) {
        const ASTNode& child = *iter;
        SkASSERT(child.fKind == ASTNode::Kind::kEnumCase);
        std::unique_ptr<Expression> value;
        if (child.begin() != child.end()) {
            value = this->convertExpression(*child.begin());
            if (!value) {
                fSymbolTable = oldTable;
                return;
            }
            if (!this->getConstantInt(*value, &currentValue)) {
                fErrors.error(value->fOffset, "enum value must be a constant integer");
                fSymbolTable = oldTable;
                return;
            }
        }
        value = std::make_unique<IntLiteral>(fContext, e.fOffset, currentValue);
        ++currentValue;
        fSymbolTable->add(std::make_unique<Variable>(e.fOffset, fModifiers->addToPool(modifiers),
                                                     child.getString(), type, fIsBuiltinCode,
                                                     Variable::Storage::kGlobal, value.get()));
        fSymbolTable->takeOwnershipOfIRNode(std::move(value));
    }
    // Now we orphanize the Enum's symbol table, so that future lookups in it are strict
    fSymbolTable->fParent = nullptr;
    fProgramElements->push_back(std::make_unique<Enum>(e.fOffset, e.getString(), fSymbolTable,
                                                       /*isSharedWithCpp=*/fIsBuiltinCode,
                                                       /*isBuiltin=*/fIsBuiltinCode));
    fSymbolTable = oldTable;
}

bool IRGenerator::typeContainsPrivateFields(const Type& type) {
    // Checks for usage of private types, including fields inside a struct.
    if (type.isPrivate()) {
        return true;
    }
    if (type.isStruct()) {
        for (const auto& f : type.fields()) {
            if (this->typeContainsPrivateFields(*f.fType)) {
                return true;
            }
        }
    }
    return false;
}

const Type* IRGenerator::convertType(const ASTNode& type, bool allowVoid) {
    ASTNode::TypeData td = type.getTypeData();
    const Symbol* symbol = (*fSymbolTable)[td.fName];
    if (!symbol || !symbol->is<Type>()) {
        fErrors.error(type.fOffset, "unknown type '" + td.fName + "'");
        return nullptr;
    }
    const Type* result = &symbol->as<Type>();
    const bool isArray = (type.begin() != type.end());
    if (td.fIsNullable) {
        if (*result == *fContext.fFragmentProcessor_Type) {
            if (isArray) {
                fErrors.error(type.fOffset, "type '" + td.fName + "' may not be used in "
                                            "an array");
            }
            result = fSymbolTable->takeOwnershipOfSymbol(
                    Type::MakeNullableType(String(result->name()) + "?", *result));
        } else {
            fErrors.error(type.fOffset, "type '" + td.fName + "' may not be nullable");
        }
    }
    if (*result == *fContext.fVoid_Type) {
        if (!allowVoid) {
            fErrors.error(type.fOffset, "type '" + td.fName + "' not allowed in this context");
            return nullptr;
        }
        if (isArray) {
            fErrors.error(type.fOffset, "type '" + td.fName + "' may not be used in an array");
            return nullptr;
        }
    }
    if (!fIsBuiltinCode && this->typeContainsPrivateFields(*result)) {
        fErrors.error(type.fOffset, "type '" + td.fName + "' is private");
        return nullptr;
    }
    if (isArray && result->isOpaque()) {
        fErrors.error(type.fOffset,
                      "opaque type '" + td.fName + "' may not be used in an array");
        return nullptr;
    }
    if (isArray) {
        auto iter = type.begin();
        int arraySize = *iter ? iter->getInt() : Type::kUnsizedArray;
        result = fSymbolTable->addArrayDimension(result, arraySize);
    }
    return result;
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
        case ASTNode::Kind::kScope:
            return this->convertScopeExpression(expr);
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
    switch (result->kind()) {
        case Symbol::Kind::kFunctionDeclaration: {
            std::vector<const FunctionDeclaration*> f = {
                &result->as<FunctionDeclaration>()
            };
            return std::make_unique<FunctionReference>(fContext, identifier.fOffset, f);
        }
        case Symbol::Kind::kUnresolvedFunction: {
            const UnresolvedFunction* f = &result->as<UnresolvedFunction>();
            return std::make_unique<FunctionReference>(fContext, identifier.fOffset,
                                                       f->functions());
        }
        case Symbol::Kind::kVariable: {
            const Variable* var = &result->as<Variable>();
            const Modifiers& modifiers = var->modifiers();
            switch (modifiers.fLayout.fBuiltin) {
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
                        (!fCaps || !fCaps->fragCoordConventionsExtensionString())) {
                        fInputs.fRTHeight = true;
                    }
#endif
            }
            if (fKind == Program::kFragmentProcessor_Kind &&
                (modifiers.fFlags & Modifiers::kIn_Flag) &&
                !(modifiers.fFlags & Modifiers::kUniform_Flag) &&
                !modifiers.fLayout.fKey &&
                modifiers.fLayout.fBuiltin == -1 &&
                var->type().nonnullable() != *fContext.fFragmentProcessor_Type &&
                var->type().typeKind() != Type::TypeKind::kSampler) {
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
                                                       var,
                                                       VariableReference::RefKind::kRead);
        }
        case Symbol::Kind::kField: {
            const Field* field = &result->as<Field>();
            auto base = std::make_unique<VariableReference>(identifier.fOffset, &field->owner(),
                                                            VariableReference::RefKind::kRead);
            return std::make_unique<FieldAccess>(std::move(base),
                                                 field->fieldIndex(),
                                                 FieldAccess::OwnerKind::kAnonymousInterfaceBlock);
        }
        case Symbol::Kind::kType: {
            const Type* t = &result->as<Type>();
            return std::make_unique<TypeReference>(fContext, identifier.fOffset, t);
        }
        case Symbol::Kind::kExternal: {
            const ExternalValue* r = &result->as<ExternalValue>();
            return std::make_unique<ExternalValueReference>(identifier.fOffset, r);
        }
        default:
            ABORT("unsupported symbol type %d\n", (int) result->kind());
    }
}

std::unique_ptr<Section> IRGenerator::convertSection(const ASTNode& s) {
    if (fKind != Program::kFragmentProcessor_Kind) {
        fErrors.error(s.fOffset, "syntax error");
        return nullptr;
    }

    ASTNode::SectionData section = s.getSectionData();
    return std::make_unique<Section>(s.fOffset, section.fName, section.fArgument,
                                                section.fText);
}

std::unique_ptr<Expression> IRGenerator::coerce(std::unique_ptr<Expression> expr,
                                                const Type& type) {
    if (!expr) {
        return nullptr;
    }
    if (expr->type() == type) {
        return expr;
    }
    this->checkValid(*expr);
    if (expr->type() == *fContext.fInvalid_Type) {
        return nullptr;
    }
    int offset = expr->fOffset;
    if (!expr->coercionCost(type).isPossible(fSettings->fAllowNarrowingConversions)) {
        fErrors.error(offset, "expected '" + type.displayName() + "', but found '" +
                              expr->type().displayName() + "'");
        return nullptr;
    }
    if (expr->is<NullLiteral>()) {
        SkASSERT(type.typeKind() == Type::TypeKind::kNullable);
        return std::make_unique<NullLiteral>(offset, &type);
    }
    ExpressionArray args;
    args.push_back(std::move(expr));
    if (!type.isScalar()) {
        return std::make_unique<Constructor>(offset, &type, std::move(args));
    }
    std::unique_ptr<Expression> ctor;
    if (type == *fContext.fFloatLiteral_Type) {
        ctor = this->convertIdentifier(ASTNode(&fFile->fNodes, offset, ASTNode::Kind::kIdentifier,
                                               "float"));
    } else if (type == *fContext.fIntLiteral_Type) {
        ctor = this->convertIdentifier(ASTNode(&fFile->fNodes, offset, ASTNode::Kind::kIdentifier,
                                               "int"));
    } else {
        ctor = this->convertIdentifier(ASTNode(&fFile->fNodes, offset, ASTNode::Kind::kIdentifier,
                                               type.name()));
    }
    if (!ctor) {
        fErrors.error(offset, "null identifier: " + type.name());
        return nullptr;
    }
    return this->call(offset, std::move(ctor), std::move(args));
}

static bool is_matrix_multiply(const Type& left, Token::Kind op, const Type& right) {
    if (op != Token::Kind::TK_STAR && op != Token::Kind::TK_STAREQ) {
        return false;
    }
    if (left.isMatrix()) {
        return right.isMatrix() || right.isVector();
    }
    return left.isVector() && right.isMatrix();
}

/**
 * Defines the set of logical (comparison) operators.
 */
static bool op_is_logical(Token::Kind op) {
    switch (op) {
        case Token::Kind::TK_LT:
        case Token::Kind::TK_GT:
        case Token::Kind::TK_LTEQ:
        case Token::Kind::TK_GTEQ:
            return true;
        default:
            return false;
    }
}

/**
 * Defines the set of operators which are only valid on integral types.
 */
static bool op_only_valid_for_integral_types(Token::Kind op) {
    switch (op) {
        case Token::Kind::TK_SHL:
        case Token::Kind::TK_SHR:
        case Token::Kind::TK_BITWISEAND:
        case Token::Kind::TK_BITWISEOR:
        case Token::Kind::TK_BITWISEXOR:
        case Token::Kind::TK_PERCENT:
        case Token::Kind::TK_SHLEQ:
        case Token::Kind::TK_SHREQ:
        case Token::Kind::TK_BITWISEANDEQ:
        case Token::Kind::TK_BITWISEOREQ:
        case Token::Kind::TK_BITWISEXOREQ:
        case Token::Kind::TK_PERCENTEQ:
            return true;
        default:
            return false;
    }
}

/**
 * Defines the set of operators which perform vector/matrix math.
 */
static bool op_valid_for_matrix_or_vector(Token::Kind op) {
    switch (op) {
        case Token::Kind::TK_PLUS:
        case Token::Kind::TK_MINUS:
        case Token::Kind::TK_STAR:
        case Token::Kind::TK_SLASH:
        case Token::Kind::TK_PERCENT:
        case Token::Kind::TK_SHL:
        case Token::Kind::TK_SHR:
        case Token::Kind::TK_BITWISEAND:
        case Token::Kind::TK_BITWISEOR:
        case Token::Kind::TK_BITWISEXOR:
        case Token::Kind::TK_PLUSEQ:
        case Token::Kind::TK_MINUSEQ:
        case Token::Kind::TK_STAREQ:
        case Token::Kind::TK_SLASHEQ:
        case Token::Kind::TK_PERCENTEQ:
        case Token::Kind::TK_SHLEQ:
        case Token::Kind::TK_SHREQ:
        case Token::Kind::TK_BITWISEANDEQ:
        case Token::Kind::TK_BITWISEOREQ:
        case Token::Kind::TK_BITWISEXOREQ:
            return true;
        default:
            return false;
    }
}

/**
 * Determines the operand and result types of a binary expression. Returns true if the expression is
 * legal, false otherwise. If false, the values of the out parameters are undefined.
 */
static bool determine_binary_type(const Context& context,
                                  bool allowNarrowing,
                                  Token::Kind op,
                                  const Type& left,
                                  const Type& right,
                                  const Type** outLeftType,
                                  const Type** outRightType,
                                  const Type** outResultType) {
    switch (op) {
        case Token::Kind::TK_EQ:  // left = right
            *outLeftType = &left;
            *outRightType = &left;
            *outResultType = &left;
            return right.canCoerceTo(left, allowNarrowing);

        case Token::Kind::TK_EQEQ:   // left == right
        case Token::Kind::TK_NEQ: {  // left != right
            CoercionCost rightToLeft = right.coercionCost(left),
                         leftToRight = left.coercionCost(right);
            if (rightToLeft < leftToRight) {
                if (rightToLeft.isPossible(allowNarrowing)) {
                    *outLeftType = &left;
                    *outRightType = &left;
                    *outResultType = context.fBool_Type.get();
                    return true;
                }
            } else {
                if (leftToRight.isPossible(allowNarrowing)) {
                    *outLeftType = &right;
                    *outRightType = &right;
                    *outResultType = context.fBool_Type.get();
                    return true;
                }
            }
            return false;
        }
        case Token::Kind::TK_LOGICALOR:   // left || right
        case Token::Kind::TK_LOGICALAND:  // left && right
        case Token::Kind::TK_LOGICALXOR:  // left ^^ right
            *outLeftType = context.fBool_Type.get();
            *outRightType = context.fBool_Type.get();
            *outResultType = context.fBool_Type.get();
            return left.canCoerceTo(*context.fBool_Type, allowNarrowing) &&
                   right.canCoerceTo(*context.fBool_Type, allowNarrowing);

        case Token::Kind::TK_COMMA:  // left, right
            *outLeftType = &left;
            *outRightType = &right;
            *outResultType = &right;
            return true;

        default:
            break;
    }

    // Boolean types only support the operators listed above (, = == != || && ^^).
    // If we've gotten this far with a boolean, we have an unsupported operator.
    const Type& leftComponentType = left.componentType();
    const Type& rightComponentType = right.componentType();
    if (leftComponentType.isBoolean() || rightComponentType.isBoolean()) {
        return false;
    }

    bool isAssignment = Compiler::IsAssignment(op);
    if (is_matrix_multiply(left, op, right)) {  // left * right
        // Determine final component type.
        if (!determine_binary_type(context, allowNarrowing, op,
                                   left.componentType(), right.componentType(),
                                   outLeftType, outRightType, outResultType)) {
            return false;
        }
        *outLeftType = &(*outResultType)->toCompound(context, left.columns(), left.rows());
        *outRightType = &(*outResultType)->toCompound(context, right.columns(), right.rows());
        int leftColumns = left.columns(), leftRows = left.rows();
        int rightColumns = right.columns(), rightRows = right.rows();
        if (right.isVector()) {
            // `matrix * vector` treats the vector as a column vector; we need to transpose it.
            std::swap(rightColumns, rightRows);
            SkASSERT(rightColumns == 1);
        }
        if (rightColumns > 1) {
            *outResultType = &(*outResultType)->toCompound(context, rightColumns, leftRows);
        } else {
            // The result was a column vector. Transpose it back to a row.
            *outResultType = &(*outResultType)->toCompound(context, leftRows, rightColumns);
        }
        if (isAssignment && ((*outResultType)->columns() != leftColumns ||
                             (*outResultType)->rows() != leftRows)) {
            return false;
        }
        return leftColumns == rightRows;
    }

    bool leftIsVectorOrMatrix = left.isVector() || left.isMatrix();
    bool validMatrixOrVectorOp = op_valid_for_matrix_or_vector(op);

    if (leftIsVectorOrMatrix && validMatrixOrVectorOp && right.isScalar()) {
        if (determine_binary_type(context, allowNarrowing, op, left.componentType(), right,
                                  outLeftType, outRightType, outResultType)) {
            *outLeftType = &(*outLeftType)->toCompound(context, left.columns(), left.rows());
            if (!op_is_logical(op)) {
                *outResultType = &(*outResultType)->toCompound(context, left.columns(),
                                                               left.rows());
            }
            return true;
        }
        return false;
    }

    bool rightIsVectorOrMatrix = right.isVector() || right.isMatrix();

    if (!isAssignment && rightIsVectorOrMatrix && validMatrixOrVectorOp && left.isScalar()) {
        if (determine_binary_type(context, allowNarrowing, op, left, right.componentType(),
                                  outLeftType, outRightType, outResultType)) {
            *outRightType = &(*outRightType)->toCompound(context, right.columns(), right.rows());
            if (!op_is_logical(op)) {
                *outResultType = &(*outResultType)->toCompound(context, right.columns(),
                                                               right.rows());
            }
            return true;
        }
        return false;
    }

    CoercionCost rightToLeftCost = right.coercionCost(left);
    CoercionCost leftToRightCost = isAssignment ? CoercionCost::Impossible()
                                                : left.coercionCost(right);

    if ((left.isScalar() && right.isScalar()) || (leftIsVectorOrMatrix && validMatrixOrVectorOp)) {
        if (op_only_valid_for_integral_types(op)) {
            if (!leftComponentType.isInteger() || !rightComponentType.isInteger()) {
                return false;
            }
        }
        if (rightToLeftCost.isPossible(allowNarrowing) && rightToLeftCost < leftToRightCost) {
            // Right-to-Left conversion is possible and cheaper
            *outLeftType = &left;
            *outRightType = &left;
            *outResultType = &left;
        } else if (leftToRightCost.isPossible(allowNarrowing)) {
            // Left-to-Right conversion is possible (and at least as cheap as Right-to-Left)
            *outLeftType = &right;
            *outRightType = &right;
            *outResultType = &right;
        } else {
            return false;
        }
        if (op_is_logical(op)) {
            *outResultType = context.fBool_Type.get();
        }
        return true;
    }
    return false;
}

static std::unique_ptr<Expression> short_circuit_boolean(const Context& context,
                                                         const Expression& left,
                                                         Token::Kind op,
                                                         const Expression& right) {
    SkASSERT(left.kind() == Expression::Kind::kBoolLiteral);
    bool leftVal = left.as<BoolLiteral>().value();
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

template <typename T>
std::unique_ptr<Expression> IRGenerator::constantFoldVector(const Expression& left,
                                                            Token::Kind op,
                                                            const Expression& right) const {
    SkASSERT(left.type() == right.type());
    const Type& type = left.type();

    // Handle boolean operations: == !=
    if (op == Token::Kind::TK_EQEQ || op == Token::Kind::TK_NEQ) {
        if (left.kind() == right.kind()) {
            bool result = left.compareConstant(fContext, right) ^ (op == Token::Kind::TK_NEQ);
            return std::make_unique<BoolLiteral>(fContext, left.fOffset, result);
        }
        return nullptr;
    }

    // Handle floating-point arithmetic: + - * /
    const auto vectorComponentwiseFold = [&](auto foldFn) -> std::unique_ptr<Constructor> {
        ExpressionArray args;
        for (int i = 0; i < type.columns(); i++) {
            T value = foldFn(left.getVecComponent<T>(i), right.getVecComponent<T>(i));
            args.push_back(std::make_unique<Literal<T>>(fContext, left.fOffset, value));
        }
        return std::make_unique<Constructor>(left.fOffset, &type, std::move(args));
    };

    const auto isVectorDivisionByZero = [&]() -> bool {
        for (int i = 0; i < type.columns(); i++) {
            if (right.getVecComponent<T>(i) == 0) {
                return true;
            }
        }
        return false;
    };

    switch (op) {
        case Token::Kind::TK_PLUS:  return vectorComponentwiseFold([](T a, T b) { return a + b; });
        case Token::Kind::TK_MINUS: return vectorComponentwiseFold([](T a, T b) { return a - b; });
        case Token::Kind::TK_STAR:  return vectorComponentwiseFold([](T a, T b) { return a * b; });
        case Token::Kind::TK_SLASH: {
            if (isVectorDivisionByZero()) {
                fErrors.error(right.fOffset, "division by zero");
                return nullptr;
            }
            return vectorComponentwiseFold([](T a, T b) { return a / b; });
        }
        default:
            return nullptr;
    }
}

std::unique_ptr<Expression> IRGenerator::constantFold(const Expression& left,
                                                      Token::Kind op,
                                                      const Expression& right) const {
    // If the left side is a constant boolean literal, the right side does not need to be constant
    // for short circuit optimizations to allow the constant to be folded.
    if (left.is<BoolLiteral>() && !right.isCompileTimeConstant()) {
        return short_circuit_boolean(fContext, left, op, right);
    } else if (right.is<BoolLiteral>() && !left.isCompileTimeConstant()) {
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
    if (left.is<BoolLiteral>() && right.is<BoolLiteral>()) {
        bool leftVal  = left.as<BoolLiteral>().value();
        bool rightVal = right.as<BoolLiteral>().value();
        bool result;
        switch (op) {
            case Token::Kind::TK_LOGICALAND: result = leftVal && rightVal; break;
            case Token::Kind::TK_LOGICALOR:  result = leftVal || rightVal; break;
            case Token::Kind::TK_LOGICALXOR: result = leftVal ^  rightVal; break;
            default: return nullptr;
        }
        return std::make_unique<BoolLiteral>(fContext, left.fOffset, result);
    }
    #define RESULT(t, op) std::make_unique<t ## Literal>(fContext, left.fOffset, \
                                                         leftVal op rightVal)
    #define URESULT(t, op) std::make_unique<t ## Literal>(fContext, left.fOffset, \
                                                          (uint32_t) leftVal op   \
                                                          (uint32_t) rightVal)
    if (left.is<IntLiteral>() && right.is<IntLiteral>()) {
        int64_t leftVal  = left.as<IntLiteral>().value();
        int64_t rightVal = right.as<IntLiteral>().value();
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
    if (left.is<FloatLiteral>() && right.is<FloatLiteral>()) {
        SKSL_FLOAT leftVal  = left.as<FloatLiteral>().value();
        SKSL_FLOAT rightVal = right.as<FloatLiteral>().value();
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
    const Type& leftType = left.type();
    const Type& rightType = right.type();
    if (leftType.isVector() && leftType == rightType) {
        if (leftType.componentType().isFloat()) {
            return constantFoldVector<SKSL_FLOAT>(left, op, right);
        } else if (leftType.componentType().isInteger()) {
            return constantFoldVector<SKSL_INT>(left, op, right);
        }
    }
    if (leftType.isMatrix() && rightType.isMatrix() && left.kind() == right.kind()) {
        switch (op) {
            case Token::Kind::TK_EQEQ:
                return std::make_unique<BoolLiteral>(fContext, left.fOffset,
                                                     left.compareConstant(fContext, right));
            case Token::Kind::TK_NEQ:
                return std::make_unique<BoolLiteral>(fContext, left.fOffset,
                                                     !left.compareConstant(fContext, right));
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
    std::unique_ptr<Expression> right = this->convertExpression(*(iter++));
    if (!right) {
        return nullptr;
    }
    const Type* leftType;
    const Type* rightType;
    const Type* resultType;
    const Type* rawLeftType;
    if (left->is<IntLiteral>() && right->type().isInteger()) {
        rawLeftType = &right->type();
    } else {
        rawLeftType = &left->type();
    }
    const Type* rawRightType;
    if (right->is<IntLiteral>() && left->type().isInteger()) {
        rawRightType = &left->type();
    } else {
        rawRightType = &right->type();
    }
    if (!determine_binary_type(fContext, fSettings->fAllowNarrowingConversions, op,
                               *rawLeftType, *rawRightType, &leftType, &rightType, &resultType)) {
        fErrors.error(expression.fOffset, String("type mismatch: '") +
                                          Compiler::OperatorName(expression.getToken().fKind) +
                                          "' cannot operate on '" + left->type().displayName() +
                                          "', '" + right->type().displayName() + "'");
        return nullptr;
    }
    if (Compiler::IsAssignment(op)) {
        if (!this->setRefKind(*left, op != Token::Kind::TK_EQ
                                                            ? VariableReference::RefKind::kReadWrite
                                                            : VariableReference::RefKind::kWrite)) {
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
                                                    std::move(right), resultType);
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
    if (!determine_binary_type(fContext, fSettings->fAllowNarrowingConversions,
                               Token::Kind::TK_EQEQ, ifTrue->type(), ifFalse->type(),
                               &trueType, &falseType, &resultType) ||
        trueType != falseType) {
        fErrors.error(node.fOffset, "ternary operator result mismatch: '" +
                                    ifTrue->type().displayName() + "', '" +
                                    ifFalse->type().displayName() + "'");
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
    if (test->kind() == Expression::Kind::kBoolLiteral) {
        // static boolean test, just return one of the branches
        if (test->as<BoolLiteral>().value()) {
            return ifTrue;
        } else {
            return ifFalse;
        }
    }
    return std::make_unique<TernaryExpression>(node.fOffset,
                                               std::move(test),
                                               std::move(ifTrue),
                                               std::move(ifFalse));
}

void IRGenerator::copyIntrinsicIfNeeded(const FunctionDeclaration& function) {
    if (const ProgramElement* found = fIntrinsics->findAndInclude(function.description())) {
        const FunctionDefinition& original = found->as<FunctionDefinition>();

        // Sort the referenced intrinsics into a consistent order; otherwise our output will become
        // non-deterministic.
        std::vector<const FunctionDeclaration*> intrinsics(original.referencedIntrinsics().begin(),
                                                           original.referencedIntrinsics().end());
        std::sort(intrinsics.begin(), intrinsics.end(),
                  [](const FunctionDeclaration* a, const FunctionDeclaration* b) {
                      if (a->isBuiltin() != b->isBuiltin()) {
                          return a->isBuiltin() < b->isBuiltin();
                      }
                      if (a->fOffset != b->fOffset) {
                          return a->fOffset < b->fOffset;
                      }
                      if (a->name() != b->name()) {
                          return a->name() < b->name();
                      }
                      return a->description() < b->description();
                  });
        for (const FunctionDeclaration* f : intrinsics) {
            this->copyIntrinsicIfNeeded(*f);
        }

        fSharedElements->push_back(found);
    }
}

std::unique_ptr<Expression> IRGenerator::call(int offset,
                                              const FunctionDeclaration& function,
                                              ExpressionArray arguments) {
    if (function.isBuiltin()) {
        if (function.definition()) {
            fReferencedIntrinsics.insert(&function);
        }
        if (!fIsBuiltinCode && fIntrinsics) {
            this->copyIntrinsicIfNeeded(function);
        }
    }
    if (function.parameters().size() != arguments.size()) {
        String msg = "call to '" + function.name() + "' expected " +
                                 to_string((uint64_t) function.parameters().size()) +
                                 " argument";
        if (function.parameters().size() != 1) {
            msg += "s";
        }
        msg += ", but found " + to_string((uint64_t) arguments.size());
        fErrors.error(offset, msg);
        return nullptr;
    }
    if (fKind == Program::kPipelineStage_Kind && !function.definition() && !function.isBuiltin()) {
        String msg = "call to undefined function '" + function.name() + "'";
        fErrors.error(offset, msg);
        return nullptr;
    }
    FunctionDeclaration::ParamTypes types;
    const Type* returnType;
    if (!function.determineFinalTypes(arguments, &types, &returnType)) {
        String msg = "no match for " + function.name() + "(";
        String separator;
        for (size_t i = 0; i < arguments.size(); i++) {
            msg += separator;
            separator = ", ";
            msg += arguments[i]->type().displayName();
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
        const Modifiers& paramModifiers = function.parameters()[i]->modifiers();
        if (paramModifiers.fFlags & Modifiers::kOut_Flag) {
            if (!this->setRefKind(*arguments[i], paramModifiers.fFlags & Modifiers::kIn_Flag
                                                          ? VariableReference::RefKind::kReadWrite
                                                          : VariableReference::RefKind::kPointer)) {
                return nullptr;
            }
        }
    }

    return std::make_unique<FunctionCall>(offset, returnType, &function, std::move(arguments));
}

/**
 * Determines the cost of coercing the arguments of a function to the required types. Cost has no
 * particular meaning other than "lower costs are preferred". Returns CoercionCost::Impossible() if
 * the call is not valid.
 */
CoercionCost IRGenerator::callCost(const FunctionDeclaration& function,
                                   const ExpressionArray& arguments) {
    if (function.parameters().size() != arguments.size()) {
        return CoercionCost::Impossible();
    }
    FunctionDeclaration::ParamTypes types;
    const Type* ignored;
    if (!function.determineFinalTypes(arguments, &types, &ignored)) {
        return CoercionCost::Impossible();
    }
    CoercionCost total = CoercionCost::Free();
    for (size_t i = 0; i < arguments.size(); i++) {
        total = total + arguments[i]->coercionCost(*types[i]);
    }
    return total;
}

std::unique_ptr<Expression> IRGenerator::call(int offset,
                                              std::unique_ptr<Expression> functionValue,
                                              ExpressionArray arguments) {
    switch (functionValue->kind()) {
        case Expression::Kind::kTypeReference:
            return this->convertConstructor(offset,
                                            functionValue->as<TypeReference>().value(),
                                            std::move(arguments));
        case Expression::Kind::kExternalValue: {
            const ExternalValue& v = functionValue->as<ExternalValueReference>().value();
            if (!v.canCall()) {
                fErrors.error(offset, "this external value is not a function");
                return nullptr;
            }
            int count = v.callParameterCount();
            if (count != (int) arguments.size()) {
                fErrors.error(offset, "external function expected " + to_string(count) +
                                      " arguments, but found " + to_string((int) arguments.size()));
                return nullptr;
            }
            static constexpr int PARAMETER_MAX = 16;
            SkASSERT(count < PARAMETER_MAX);
            const Type* types[PARAMETER_MAX];
            v.getCallParameterTypes(types);
            for (int i = 0; i < count; ++i) {
                arguments[i] = this->coerce(std::move(arguments[i]), *types[i]);
                if (!arguments[i]) {
                    return nullptr;
                }
            }
            return std::make_unique<ExternalFunctionCall>(offset, &v, std::move(arguments));
        }
        case Expression::Kind::kFunctionReference: {
            const FunctionReference& ref = functionValue->as<FunctionReference>();
            const std::vector<const FunctionDeclaration*>& functions = ref.functions();
            CoercionCost bestCost = CoercionCost::Impossible();
            const FunctionDeclaration* best = nullptr;
            if (functions.size() > 1) {
                for (const auto& f : functions) {
                    CoercionCost cost = this->callCost(*f, arguments);
                    if (cost < bestCost) {
                        bestCost = cost;
                        best = f;
                    }
                }
                if (best) {
                    return this->call(offset, *best, std::move(arguments));
                }
                String msg = "no match for " + functions[0]->name() + "(";
                String separator;
                for (size_t i = 0; i < arguments.size(); i++) {
                    msg += separator;
                    separator = ", ";
                    msg += arguments[i]->type().displayName();
                }
                msg += ")";
                fErrors.error(offset, msg);
                return nullptr;
            }
            return this->call(offset, *functions[0], std::move(arguments));
        }
        default:
            fErrors.error(offset, "not a function");
            return nullptr;
    }
}

std::unique_ptr<Expression> IRGenerator::convertNumberConstructor(int offset,
                                                                  const Type& type,
                                                                  ExpressionArray args) {
    SkASSERT(type.isNumber());
    if (args.size() != 1) {
        fErrors.error(offset, "invalid arguments to '" + type.displayName() +
                              "' constructor, (expected exactly 1 argument, but found " +
                              to_string((uint64_t) args.size()) + ")");
        return nullptr;
    }
    const Type& argType = args[0]->type();
    if (type == argType) {
        return std::move(args[0]);
    }
    if (type.isFloat() && args.size() == 1 && args[0]->is<FloatLiteral>()) {
        SKSL_FLOAT value = args[0]->as<FloatLiteral>().value();
        return std::make_unique<FloatLiteral>(offset, value, &type);
    }
    if (type.isFloat() && args.size() == 1 && args[0]->is<IntLiteral>()) {
        int64_t value = args[0]->as<IntLiteral>().value();
        return std::make_unique<FloatLiteral>(offset, (float)value, &type);
    }
    if (args[0]->is<IntLiteral>() && (type == *fContext.fInt_Type ||
                                      type == *fContext.fUInt_Type)) {
        return std::make_unique<IntLiteral>(offset, args[0]->as<IntLiteral>().value(), &type);
    }
    if (argType == *fContext.fBool_Type) {
        std::unique_ptr<IntLiteral> zero(new IntLiteral(fContext, offset, 0));
        std::unique_ptr<IntLiteral> one(new IntLiteral(fContext, offset, 1));
        return std::make_unique<TernaryExpression>(offset, std::move(args[0]),
                                                   this->coerce(std::move(one), type),
                                                   this->coerce(std::move(zero), type));
    }
    if (!argType.isNumber()) {
        fErrors.error(offset, "invalid argument to '" + type.displayName() +
                              "' constructor (expected a number or bool, but found '" +
                              argType.displayName() + "')");
        return nullptr;
    }
    return std::make_unique<Constructor>(offset, &type, std::move(args));
}

static int component_count(const Type& type) {
    switch (type.typeKind()) {
        case Type::TypeKind::kVector:
            return type.columns();
        case Type::TypeKind::kMatrix:
            return type.columns() * type.rows();
        default:
            return 1;
    }
}

std::unique_ptr<Expression> IRGenerator::convertCompoundConstructor(int offset,
                                                                    const Type& type,
                                                                    ExpressionArray args) {
    SkASSERT(type.isVector() || type.isMatrix());
    if (type.isMatrix() && args.size() == 1 && args[0]->type().isMatrix()) {
        // matrix from matrix is always legal
        return std::unique_ptr<Expression>(new Constructor(offset, &type, std::move(args)));
    }
    int actual = 0;
    int expected = type.rows() * type.columns();
    if (args.size() != 1 || expected != component_count(args[0]->type()) ||
        type.componentType().isNumber() != args[0]->type().componentType().isNumber()) {
        for (size_t i = 0; i < args.size(); i++) {
            const Type& argType = args[i]->type();
            if (argType.isVector()) {
                if (type.componentType().isNumber() !=
                    argType.componentType().isNumber()) {
                    fErrors.error(offset, "'" + argType.displayName() + "' is not a valid "
                                          "parameter to '" + type.displayName() +
                                          "' constructor");
                    return nullptr;
                }
                actual += argType.columns();
            } else if (argType.isScalar()) {
                actual += 1;
                if (!type.isScalar()) {
                    args[i] = this->coerce(std::move(args[i]), type.componentType());
                    if (!args[i]) {
                        return nullptr;
                    }
                }
            } else {
                fErrors.error(offset, "'" + argType.displayName() + "' is not a valid "
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
    return std::unique_ptr<Expression>(new Constructor(offset, &type, std::move(args)));
}

std::unique_ptr<Expression> IRGenerator::convertConstructor(int offset,
                                                            const Type& type,
                                                            ExpressionArray args) {
    // FIXME: add support for structs
    if (args.size() == 1 && args[0]->type() == type &&
        type.nonnullable() != *fContext.fFragmentProcessor_Type) {
        // argument is already the right type, just return it
        return std::move(args[0]);
    }
    if (type.isNumber()) {
        return this->convertNumberConstructor(offset, type, std::move(args));
    } else if (type.isArray()) {
        const Type& base = type.componentType();
        for (size_t i = 0; i < args.size(); i++) {
            args[i] = this->coerce(std::move(args[i]), base);
            if (!args[i]) {
                return nullptr;
            }
        }
        return std::make_unique<Constructor>(offset, &type, std::move(args));
    } else if (type.isVector() || type.isMatrix()) {
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
    const Type& baseType = base->type();
    switch (expression.getToken().fKind) {
        case Token::Kind::TK_PLUS:
            if (!baseType.isNumber() && !baseType.isVector() &&
                baseType != *fContext.fFloatLiteral_Type) {
                fErrors.error(expression.fOffset,
                              "'+' cannot operate on '" + baseType.displayName() + "'");
                return nullptr;
            }
            return base;

        case Token::Kind::TK_MINUS:
            if (base->is<IntLiteral>()) {
                return std::make_unique<IntLiteral>(fContext, base->fOffset,
                                                    -base->as<IntLiteral>().value());
            }
            if (base->is<FloatLiteral>()) {
                return std::make_unique<FloatLiteral>(fContext, base->fOffset,
                                                      -base->as<FloatLiteral>().value());
            }
            if (!baseType.isNumber() &&
                !(baseType.isVector() && baseType.componentType().isNumber())) {
                fErrors.error(expression.fOffset,
                              "'-' cannot operate on '" + baseType.displayName() + "'");
                return nullptr;
            }
            return std::make_unique<PrefixExpression>(Token::Kind::TK_MINUS, std::move(base));

        case Token::Kind::TK_PLUSPLUS:
            if (!baseType.isNumber()) {
                fErrors.error(expression.fOffset,
                              String("'") + Compiler::OperatorName(expression.getToken().fKind) +
                              "' cannot operate on '" + baseType.displayName() + "'");
                return nullptr;
            }
            if (!this->setRefKind(*base, VariableReference::RefKind::kReadWrite)) {
                return nullptr;
            }
            break;
        case Token::Kind::TK_MINUSMINUS:
            if (!baseType.isNumber()) {
                fErrors.error(expression.fOffset,
                              String("'") + Compiler::OperatorName(expression.getToken().fKind) +
                              "' cannot operate on '" + baseType.displayName() + "'");
                return nullptr;
            }
            if (!this->setRefKind(*base, VariableReference::RefKind::kReadWrite)) {
                return nullptr;
            }
            break;
        case Token::Kind::TK_LOGICALNOT:
            if (!baseType.isBoolean()) {
                fErrors.error(expression.fOffset,
                              String("'") + Compiler::OperatorName(expression.getToken().fKind) +
                              "' cannot operate on '" + baseType.displayName() + "'");
                return nullptr;
            }
            if (base->kind() == Expression::Kind::kBoolLiteral) {
                return std::make_unique<BoolLiteral>(fContext, base->fOffset,
                                                     !base->as<BoolLiteral>().value());
            }
            break;
        case Token::Kind::TK_BITWISENOT:
            if (baseType != *fContext.fInt_Type && baseType != *fContext.fUInt_Type) {
                fErrors.error(expression.fOffset,
                              String("'") + Compiler::OperatorName(expression.getToken().fKind) +
                              "' cannot operate on '" + baseType.displayName() + "'");
                return nullptr;
            }
            break;
        default:
            ABORT("unsupported prefix operator\n");
    }
    return std::make_unique<PrefixExpression>(expression.getToken().fKind, std::move(base));
}

std::unique_ptr<Expression> IRGenerator::convertField(std::unique_ptr<Expression> base,
                                                      StringFragment field) {
    if (base->kind() == Expression::Kind::kExternalValue) {
        const ExternalValue& ev = base->as<ExternalValueReference>().value();
        ExternalValue* result = ev.getChild(String(field).c_str());
        if (!result) {
            fErrors.error(base->fOffset, "external value does not have a child named '" + field +
                                         "'");
            return nullptr;
        }
        return std::unique_ptr<Expression>(new ExternalValueReference(base->fOffset, result));
    }
    const Type& baseType = base->type();
    auto fields = baseType.fields();
    for (size_t i = 0; i < fields.size(); i++) {
        if (fields[i].fName == field) {
            return std::unique_ptr<Expression>(new FieldAccess(std::move(base), (int) i));
        }
    }
    fErrors.error(base->fOffset, "type '" + baseType.displayName() + "' does not have a field "
                                 "named '" + field + "'");
    return nullptr;
}

// Swizzles are complicated due to constant components. The most difficult case is a mask like
// '.x1w0'. A naive approach might turn that into 'float4(base.x, 1, base.w, 0)', but that evaluates
// 'base' twice. We instead group the swizzle mask ('xw') and constants ('1, 0') together and use a
// secondary swizzle to put them back into the right order, so in this case we end up with
// 'float4(base.xw, 1, 0).xzyw'.
std::unique_ptr<Expression> IRGenerator::convertSwizzle(std::unique_ptr<Expression> base,
                                                        StringFragment fields) {
    const int offset = base->fOffset;
    const Type& baseType = base->type();
    if (!baseType.isVector() && !baseType.isNumber()) {
        fErrors.error(offset, "cannot swizzle value of type '" + baseType.displayName() + "'");
        return nullptr;
    }

    if (fields.fLength > 4) {
        fErrors.error(offset, "too many components in swizzle mask '" + fields + "'");
        return nullptr;
    }

    ComponentArray maskComponents;
    for (size_t i = 0; i < fields.fLength; i++) {
        switch (fields[i]) {
            case '0':
            case '1':
                // Skip over constant fields for now.
                break;
            case 'x':
            case 'r':
            case 's':
            case 'L':
                maskComponents.push_back(0);
                break;
            case 'y':
            case 'g':
            case 't':
            case 'T':
                if (baseType.columns() >= 2) {
                    maskComponents.push_back(1);
                    break;
                }
                [[fallthrough]];
            case 'z':
            case 'b':
            case 'p':
            case 'R':
                if (baseType.columns() >= 3) {
                    maskComponents.push_back(2);
                    break;
                }
                [[fallthrough]];
            case 'w':
            case 'a':
            case 'q':
            case 'B':
                if (baseType.columns() >= 4) {
                    maskComponents.push_back(3);
                    break;
                }
                [[fallthrough]];
            default:
                fErrors.error(offset, String::printf("invalid swizzle component '%c'", fields[i]));
                return nullptr;
        }
    }
    if (maskComponents.empty()) {
        fErrors.error(offset, "swizzle must refer to base expression");
        return nullptr;
    }

    // First, we need a vector expression that is the non-constant portion of the swizzle, packed:
    //   scalar.xxx  -> type3(scalar)
    //   scalar.x0x0 -> type2(scalar)
    //   vector.zyx  -> vector.zyx
    //   vector.x0y0 -> vector.xy
    std::unique_ptr<Expression> expr;
    if (baseType.isNumber()) {
        ExpressionArray scalarConstructorArgs;
        scalarConstructorArgs.push_back(std::move(base));
        expr = std::make_unique<Constructor>(
                offset, &baseType.toCompound(fContext, maskComponents.size(), 1),
                std::move(scalarConstructorArgs));
    } else {
        expr = std::make_unique<Swizzle>(fContext, std::move(base), maskComponents);
    }

    // If we have processed the entire swizzle, we're done.
    if (maskComponents.size() == fields.fLength) {
        return expr;
    }

    // Now we create a constructor that has the correct number of elements for the final swizzle,
    // with all fields at the start. It's not finished yet; constants we need will be added below.
    //   scalar.x0x0 -> type4(type2(x), ...)
    //   vector.y111 -> type4(vector.y, ...)
    //   vector.z10x -> type4(vector.zx, ...)
    //
    // We could create simpler IR in some cases by reordering here, if all fields are packed
    // contiguously. The benefits are minor, so skip the optimization to keep the algorithm simple.
    // The constructor will have at most three arguments: { base value, constant 0, constant 1 }
    ExpressionArray constructorArgs;
    constructorArgs.reserve_back(3);
    constructorArgs.push_back(std::move(expr));

    // Apply another swizzle to shuffle the constants into the correct place. Any constant values we
    // need are also tacked on to the end of the constructor.
    //   scalar.x0x0 -> type4(type2(x), 0).xyxy
    //   vector.y111 -> type4(vector.y, 1).xyyy
    //   vector.z10x -> type4(vector.zx, 1, 0).xzwy
    const Type* numberType = baseType.isNumber() ? &baseType : &baseType.componentType();
    ComponentArray swizzleComponents;
    int maskFieldIdx = 0;
    int constantFieldIdx = maskComponents.size();
    int constantZeroIdx = -1, constantOneIdx = -1;

    for (size_t i = 0; i < fields.fLength; i++) {
        switch (fields[i]) {
            case '0':
                if (constantZeroIdx == -1) {
                    // Synthesize a 'type(0)' argument at the end of the constructor.
                    auto zero = std::make_unique<Constructor>(offset, numberType,
                                                              ExpressionArray{});
                    zero->arguments().push_back(std::make_unique<IntLiteral>(fContext, offset,
                                                                             /*fValue=*/0));
                    constructorArgs.push_back(std::move(zero));
                    constantZeroIdx = constantFieldIdx++;
                }
                swizzleComponents.push_back(constantZeroIdx);
                break;
            case '1':
                if (constantOneIdx == -1) {
                    // Synthesize a 'type(1)' argument at the end of the constructor.
                    auto one = std::make_unique<Constructor>(offset, numberType, ExpressionArray{});
                    one->arguments().push_back(std::make_unique<IntLiteral>(fContext, offset,
                                                                            /*fValue=*/1));
                    constructorArgs.push_back(std::move(one));
                    constantOneIdx = constantFieldIdx++;
                }
                swizzleComponents.push_back(constantOneIdx);
                break;
            default:
                // The non-constant fields are already in the expected order.
                swizzleComponents.push_back(maskFieldIdx++);
                break;
        }
    }

    expr = std::make_unique<Constructor>(offset,
                                         &numberType->toCompound(fContext, constantFieldIdx, 1),
                                         std::move(constructorArgs));

    // For some of our most common use cases ('.xyz0', '.xyz1'), we will now have an identity
    // swizzle; in those cases we can just return the constructor without the swizzle attached.
    for (size_t i = 0; i < swizzleComponents.size(); ++i) {
        if (swizzleComponents[i] != int(i)) {
            // The swizzle has an effect, so apply it.
            return std::make_unique<Swizzle>(fContext, std::move(expr), swizzleComponents);
        }
    }

    // The swizzle was a no-op; return the constructor expression directly.
    return expr;
}

const Type* IRGenerator::typeForSetting(int offset, String name) const {
    auto found = fCapsMap.find(name);
    if (found == fCapsMap.end()) {
        fErrors.error(offset, "unknown capability flag '" + name + "'");
        return nullptr;
    }
    switch (found->second.fKind) {
        case Program::Settings::Value::kBool_Kind:  return fContext.fBool_Type.get();
        case Program::Settings::Value::kFloat_Kind: return fContext.fFloat_Type.get();
        case Program::Settings::Value::kInt_Kind:   return fContext.fInt_Type.get();
    }
    SkUNREACHABLE;
    return nullptr;
}

std::unique_ptr<Expression> IRGenerator::valueForSetting(int offset, String name) const {
    auto found = fCapsMap.find(name);
    if (found == fCapsMap.end()) {
        fErrors.error(offset, "unknown capability flag '" + name + "'");
        return nullptr;
    }
    return found->second.literal(fContext, offset);
}

std::unique_ptr<Expression> IRGenerator::convertTypeField(int offset, const Type& type,
                                                          StringFragment field) {
    const ProgramElement* enumElement = nullptr;
    // Find the Enum element that this type refers to, start by searching our elements
    for (const std::unique_ptr<ProgramElement>& e : *fProgramElements) {
        if (e->is<Enum>() && type.name() == e->as<Enum>().typeName()) {
            enumElement = e.get();
            break;
        }
    }
    // ... if that fails, look in our shared elements
    if (!enumElement) {
        for (const ProgramElement* e : *fSharedElements) {
            if (e->is<Enum>() && type.name() == e->as<Enum>().typeName()) {
                enumElement = e;
                break;
            }
        }
    }
    // ... and if that fails, check the intrinsics, add it to our shared elements
    if (!enumElement && !fIsBuiltinCode && fIntrinsics) {
        if (const ProgramElement* found = fIntrinsics->findAndInclude(type.name())) {
            fSharedElements->push_back(found);
            enumElement = found;
        }
    }
    if (!enumElement) {
        fErrors.error(offset, "type '" + type.displayName() + "' is not a known enum");
        return nullptr;
    }

    // We found the Enum element. Look for 'field' as a member.
    std::shared_ptr<SymbolTable> old = fSymbolTable;
    fSymbolTable = enumElement->as<Enum>().symbols();
    std::unique_ptr<Expression> result =
            convertIdentifier(ASTNode(&fFile->fNodes, offset, ASTNode::Kind::kIdentifier, field));
    if (result) {
        const Variable& v = *result->as<VariableReference>().variable();
        SkASSERT(v.initialValue());
        result = std::make_unique<IntLiteral>(offset, v.initialValue()->as<IntLiteral>().value(),
                                              &type);
    } else {
        fErrors.error(offset,
                      "type '" + type.name() + "' does not contain enumerator '" + field + "'");
    }
    fSymbolTable = old;
    return result;
}

std::unique_ptr<Expression> IRGenerator::convertIndexExpression(const ASTNode& index) {
    SkASSERT(index.fKind == ASTNode::Kind::kIndex);
    auto iter = index.begin();
    std::unique_ptr<Expression> base = this->convertExpression(*(iter++));
    if (!base) {
        return nullptr;
    }
    return (iter != index.end()) ? this->convertIndex(std::move(base), *(iter++))
                                 : this->convertEmptyIndex(std::move(base));
}

std::unique_ptr<Expression> IRGenerator::convertEmptyIndex(std::unique_ptr<Expression> base) {
    // Convert an index expression with nothing inside of it: `float[]`.
    if (base->is<TypeReference>()) {
        const Type* type = &base->as<TypeReference>().value();
        type = fSymbolTable->addArrayDimension(type, Type::kUnsizedArray);
        return std::make_unique<TypeReference>(fContext, base->fOffset, type);
    }
    fErrors.error(base->fOffset, "'[]' must follow a type name");
    return nullptr;
}

std::unique_ptr<Expression> IRGenerator::convertIndex(std::unique_ptr<Expression> base,
                                                      const ASTNode& index) {
    // Convert an index expression with an expression inside of it: `int[12]` or `arr[a * 3]`.
    if (base->is<TypeReference>()) {
        if (index.fKind == ASTNode::Kind::kInt) {
            const Type* type = &base->as<TypeReference>().value();
            type = fSymbolTable->addArrayDimension(type, index.getInt());
            return std::make_unique<TypeReference>(fContext, base->fOffset, type);

        } else {
            fErrors.error(base->fOffset, "array size must be a constant");
            return nullptr;
        }
    }
    const Type& baseType = base->type();
    if (!baseType.isArray() && !baseType.isMatrix() && !baseType.isVector()) {
        fErrors.error(base->fOffset, "expected array, but found '" + baseType.displayName() + "'");
        return nullptr;
    }
    std::unique_ptr<Expression> converted = this->convertExpression(index);
    if (!converted) {
        return nullptr;
    }
    if (!converted->type().isInteger()) {
        converted = this->coerce(std::move(converted), *fContext.fInt_Type);
        if (!converted) {
            return nullptr;
        }
    }
    // Perform compile-time bounds checking on constant indices.
    if (converted->is<IntLiteral>()) {
        int64_t index = converted->as<IntLiteral>().value();

        const int upperBound = (baseType.isArray() && baseType.columns() == Type::kUnsizedArray)
                                       ? INT_MAX
                                       : baseType.columns();
        if (index < 0 || index >= upperBound) {
            fErrors.error(base->fOffset, "index " + to_string(index) +
                                         " out of range for '" + baseType.displayName() + "'");
            return nullptr;
        }
        // Constant array indexes on vectors can be converted to swizzles: `myHalf4.z`.
        // (Using a swizzle gives our optimizer a bit more to work with, compared to array indices.)
        if (baseType.isVector()) {
            return std::make_unique<Swizzle>(fContext, std::move(base),
                                             ComponentArray{(int8_t)index});
        }
    }
    return std::make_unique<IndexExpression>(fContext, std::move(base), std::move(converted));
}

std::unique_ptr<Expression> IRGenerator::convertCallExpression(const ASTNode& callNode) {
    SkASSERT(callNode.fKind == ASTNode::Kind::kCall);
    auto iter = callNode.begin();
    std::unique_ptr<Expression> base = this->convertExpression(*(iter++));
    if (!base) {
        return nullptr;
    }
    ExpressionArray arguments;
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
    const Type& baseType = base->type();
    if (baseType == *fContext.fSkCaps_Type) {
        const Type* type = this->typeForSetting(fieldNode.fOffset, field);
        if (!type) {
            return nullptr;
        }
        return std::make_unique<Setting>(fieldNode.fOffset, field, type);
    }
    if (base->kind() == Expression::Kind::kExternalValue) {
        return this->convertField(std::move(base), field);
    }
    switch (baseType.typeKind()) {
        case Type::TypeKind::kOther:
        case Type::TypeKind::kStruct:
            return this->convertField(std::move(base), field);
        default:
            return this->convertSwizzle(std::move(base), field);
    }
}

std::unique_ptr<Expression> IRGenerator::convertScopeExpression(const ASTNode& scopeNode) {
    std::unique_ptr<Expression> base = this->convertExpression(*scopeNode.begin());
    if (!base) {
        return nullptr;
    }
    if (!base->is<TypeReference>()) {
        fErrors.error(scopeNode.fOffset, "'::' must follow a type name");
        return nullptr;
    }
    StringFragment member = scopeNode.getString();
    return this->convertTypeField(base->fOffset, base->as<TypeReference>().value(), member);
}

std::unique_ptr<Expression> IRGenerator::convertPostfixExpression(const ASTNode& expression) {
    std::unique_ptr<Expression> base = this->convertExpression(*expression.begin());
    if (!base) {
        return nullptr;
    }
    const Type& baseType = base->type();
    if (!baseType.isNumber()) {
        fErrors.error(expression.fOffset,
                      "'" + String(Compiler::OperatorName(expression.getToken().fKind)) +
                      "' cannot operate on '" + baseType.displayName() + "'");
        return nullptr;
    }
    if (!this->setRefKind(*base, VariableReference::RefKind::kReadWrite)) {
        return nullptr;
    }
    return std::make_unique<PostfixExpression>(std::move(base), expression.getToken().fKind);
}

void IRGenerator::checkValid(const Expression& expr) {
    switch (expr.kind()) {
        case Expression::Kind::kFunctionReference:
            fErrors.error(expr.fOffset, "expected '(' to begin function call");
            break;
        case Expression::Kind::kTypeReference:
            fErrors.error(expr.fOffset, "expected '(' to begin constructor invocation");
            break;
        default:
            if (expr.type() == *fContext.fInvalid_Type) {
                fErrors.error(expr.fOffset, "invalid expression");
            }
    }
}

bool IRGenerator::setRefKind(Expression& expr, VariableReference::RefKind kind) {
    Analysis::AssignmentInfo info;
    if (!Analysis::IsAssignable(expr, &info, &fErrors)) {
        return false;
    }
    if (info.fAssignedVar) {
        info.fAssignedVar->setRefKind(kind);
    }
    return true;
}

void IRGenerator::findAndDeclareBuiltinVariables() {
    class BuiltinVariableScanner : public ProgramVisitor {
    public:
        BuiltinVariableScanner(IRGenerator* generator) : fGenerator(generator) {}

        void addDeclaringElement(const String& name) {
            // If this is the *first* time we've seen this builtin, findAndInclude will return
            // the corresponding ProgramElement.
            if (const ProgramElement* decl = fGenerator->fIntrinsics->findAndInclude(name)) {
                SkASSERT(decl->is<GlobalVarDeclaration>() || decl->is<InterfaceBlock>());
                fNewElements.push_back(decl);
            }
        }

        bool visitExpression(const Expression& e) override {
            if (e.is<VariableReference>() && e.as<VariableReference>().variable()->isBuiltin()) {
                this->addDeclaringElement(e.as<VariableReference>().variable()->name());
            }
            return INHERITED::visitExpression(e);
        }

        IRGenerator* fGenerator;
        std::vector<const ProgramElement*> fNewElements;

        using INHERITED = ProgramVisitor;
        using INHERITED::visitProgramElement;
    };

    BuiltinVariableScanner scanner(this);
    for (auto& e : *fProgramElements) {
        scanner.visitProgramElement(*e);
    }

    // Vulkan requires certain builtin variables be present, even if they're unused. At one time,
    // validation errors would result if they were missing. Now, it's just (Adreno) driver bugs
    // that drop or corrupt draws if they're missing.
    switch (fKind) {
        case Program::kFragment_Kind:
            scanner.addDeclaringElement("sk_Clockwise");
            break;
        default:
            break;
    }

    fSharedElements->insert(
            fSharedElements->begin(), scanner.fNewElements.begin(), scanner.fNewElements.end());
}

IRGenerator::IRBundle IRGenerator::convertProgram(
        Program::Kind kind,
        const Program::Settings* settings,
        const ParsedModule& base,
        bool isBuiltinCode,
        const char* text,
        size_t length,
        const std::vector<std::unique_ptr<ExternalValue>>* externalValues) {
    fKind = kind;
    fSettings = settings;
    fSymbolTable = base.fSymbols;
    fIntrinsics = base.fIntrinsics.get();
    if (fIntrinsics) {
        fIntrinsics->resetAlreadyIncluded();
    }
    fIsBuiltinCode = isBuiltinCode;

    std::vector<std::unique_ptr<ProgramElement>> elements;
    std::vector<const ProgramElement*> sharedElements;

    fProgramElements = &elements;
    fSharedElements = &sharedElements;

    fInputs.reset();
    fInvocations = -1;
    fRTAdjust = nullptr;
    fRTAdjustInterfaceBlock = nullptr;

    AutoSymbolTable table(this);

    if (kind == Program::kGeometry_Kind && !fIsBuiltinCode) {
        // Declare sk_InvocationID programmatically. With invocations support, it's an 'in' builtin.
        // If we're applying the workaround, then it's a plain global.
        bool workaround = fCaps && !fCaps->gsInvocationsSupport();
        Modifiers m;
        if (!workaround) {
            m.fFlags = Modifiers::kIn_Flag;
            m.fLayout.fBuiltin = SK_INVOCATIONID_BUILTIN;
        }
        auto var = std::make_unique<Variable>(-1, fModifiers->addToPool(m), "sk_InvocationID",
                                              fContext.fInt_Type.get(), false,
                                              Variable::Storage::kGlobal);
        auto decl = std::make_unique<VarDeclaration>(var.get(), fContext.fInt_Type.get(),
                                                     /*arraySize=*/0, /*value=*/nullptr);
        fSymbolTable->add(std::move(var));
        fProgramElements->push_back(
                std::make_unique<GlobalVarDeclaration>(/*offset=*/-1, std::move(decl)));
    }

    if (externalValues) {
        // Add any external values to the new symbol table, so they're only visible to this Program
        for (const auto& ev : *externalValues) {
            fSymbolTable->addWithoutOwnership(ev.get());
        }
    }

    Parser parser(text, length, *fSymbolTable, fErrors);
    fFile = parser.compilationUnit();
    if (fErrors.errorCount()) {
        return {};
    }
    SkASSERT(fFile);
    for (const auto& decl : fFile->root()) {
        switch (decl.fKind) {
            case ASTNode::Kind::kVarDeclarations:
                this->convertGlobalVarDeclarations(decl);
                break;

            case ASTNode::Kind::kEnum:
                this->convertEnum(decl);
                break;

            case ASTNode::Kind::kFunction:
                this->convertFunction(decl);
                break;

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
            case ASTNode::Kind::kType: {
                std::unique_ptr<StructDefinition> s = this->convertStructDefinition(decl);
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

    // Variables defined in the pre-includes need their declaring elements added to the program
    if (!fIsBuiltinCode && fIntrinsics) {
        this->findAndDeclareBuiltinVariables();
    }

    // Do a final pass looking for dangling FunctionReference or TypeReference expressions
    class FindIllegalExpressions : public ProgramVisitor {
    public:
        FindIllegalExpressions(IRGenerator* generator) : fGenerator(generator) {}

        bool visitExpression(const Expression& e) override {
            fGenerator->checkValid(e);
            return INHERITED::visitExpression(e);
        }

        IRGenerator* fGenerator;
        using INHERITED = ProgramVisitor;
        using INHERITED::visitProgramElement;
    };
    for (const auto& pe : *fProgramElements) {
        FindIllegalExpressions{this}.visitProgramElement(*pe);
    }

    fSettings = nullptr;

    return IRBundle{std::move(elements), std::move(sharedElements), this->releaseModifiers(),
                    fSymbolTable, fInputs};
}

}  // namespace SkSL
