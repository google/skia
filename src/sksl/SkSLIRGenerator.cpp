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
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLOperators.h"
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
#include "src/sksl/ir/SkSLExternalFunctionReference.h"
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

void IRGenerator::FillCapsMap(const SkSL::ShaderCapsClass& caps,
                              std::unordered_map<String, CapsValue>* capsMap) {
#define CAP(name) capsMap->insert({String(#name), CapsValue(caps.name())})
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

std::unique_ptr<Expression> IRGenerator::CapsValue::literal(const Context& context,
                                                            int offset) const {
    switch (fKind) {
        case kBool_Kind:
            return std::make_unique<BoolLiteral>(context, offset, fValue);

        case kInt_Kind:
            return std::make_unique<IntLiteral>(context, offset, fValue);

        case kFloat_Kind:
            return std::make_unique<FloatLiteral>(context, offset, fValueF);

        default:
            SkDEBUGFAILF("unrecognized caps kind: %d", fKind);
            return nullptr;
    }
}

IRGenerator::IRGenerator(const Context* context,
                         const ShaderCapsClass* caps)
        : fContext(*context)
        , fCaps(caps)
        , fModifiers(new ModifiersPool()) {
    if (fCaps) {
        FillCapsMap(*fCaps, &fCapsMap);
    } else {
        fCapsMap.insert({String("integerSupport"), CapsValue(true)});
    }

}

void IRGenerator::pushSymbolTable() {
    auto childSymTable = std::make_shared<SymbolTable>(std::move(fSymbolTable), fIsBuiltinCode);
    fSymbolTable = std::move(childSymTable);
}

void IRGenerator::popSymbolTable() {
    fSymbolTable = fSymbolTable->fParent;
}

bool IRGenerator::detectVarDeclarationWithoutScope(const Statement& stmt) {
    // Parsing an AST node containing a single variable declaration creates a lone VarDeclaration
    // statement. An AST with multiple variable declarations creates an unscoped Block containing
    // multiple VarDeclaration statements. We need to detect either case.
    const Variable* var;
    if (stmt.is<VarDeclaration>()) {
        // The single-variable case. No blocks at all.
        var = &stmt.as<VarDeclaration>().var();
    } else if (stmt.is<Block>()) {
        // The multiple-variable case: an unscoped, non-empty block...
        const Block& block = stmt.as<Block>();
        if (block.isScope() || block.children().empty()) {
            return false;
        }
        // ... holding a variable declaration.
        const Statement& innerStmt = *block.children().front();
        if (!innerStmt.is<VarDeclaration>()) {
            return false;
        }
        var = &innerStmt.as<VarDeclaration>().var();
    } else {
        // This statement wasn't a variable declaration. No problem.
        return false;
    }

    // Report an error.
    SkASSERT(var);
    this->errorReporter().error(stmt.fOffset,
                                "variable '" + var->name() + "' must be created in a scope");
    return true;
}

std::unique_ptr<Extension> IRGenerator::convertExtension(int offset, StringFragment name) {
    if (fKind != ProgramKind::kFragment &&
        fKind != ProgramKind::kVertex &&
        fKind != ProgramKind::kGeometry) {
        this->errorReporter().error(offset, "extensions are not allowed here");
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
            if (fRTAdjust && fKind == ProgramKind::kGeometry) {
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

int IRGenerator::convertArraySize(const Type& type, int offset, const ASTNode& s) {
    if (!s) {
        this->errorReporter().error(offset, "array must have a size");
        return 0;
    }
    auto size = this->convertExpression(s);
    if (!size) {
        return 0;
    }
    return this->convertArraySize(type, std::move(size));
}

int IRGenerator::convertArraySize(const Type& type, std::unique_ptr<Expression> size) {
    size = this->coerce(std::move(size), *fContext.fTypes.fInt);
    if (!size) {
        return 0;
    }
    if (type == *fContext.fTypes.fVoid) {
        this->errorReporter().error(size->fOffset, "type 'void' may not be used in an array");
        return 0;
    }
    if (type.isOpaque()) {
        this->errorReporter().error(
                size->fOffset, "opaque type '" + type.name() + "' may not be used in an array");
        return 0;
    }
    if (!size->is<IntLiteral>()) {
        this->errorReporter().error(size->fOffset, "array size must be an integer");
        return 0;
    }
    SKSL_INT count = size->as<IntLiteral>().value();
    if (count <= 0) {
        this->errorReporter().error(size->fOffset, "array size must be positive");
        return 0;
    }
    if (!SkTFitsIn<int>(count)) {
        this->errorReporter().error(size->fOffset, "array size is too large");
        return 0;
    }
    return static_cast<int>(count);
}

void IRGenerator::checkVarDeclaration(int offset, const Modifiers& modifiers, const Type* baseType,
                                      Variable::Storage storage) {
    if (this->strictES2Mode() && baseType->isArray()) {
        this->errorReporter().error(offset, "array size must appear after variable name");
    }

    if (baseType->componentType().isOpaque() && storage != Variable::Storage::kGlobal) {
        this->errorReporter().error(
                offset,
                "variables of type '" + baseType->displayName() + "' must be global");
    }
    if (fKind != ProgramKind::kFragmentProcessor) {
        if ((modifiers.fFlags & Modifiers::kIn_Flag) && baseType->isMatrix()) {
            this->errorReporter().error(offset, "'in' variables may not have matrix type");
        }
        if ((modifiers.fFlags & Modifiers::kIn_Flag) &&
            (modifiers.fFlags & Modifiers::kUniform_Flag)) {
            this->errorReporter().error(
                    offset,
                    "'in uniform' variables only permitted within fragment processors");
        }
        if (modifiers.fLayout.fWhen.fLength) {
            this->errorReporter().error(offset,
                                        "'when' is only permitted within fragment processors");
        }
        if (modifiers.fLayout.fFlags & Layout::kTracked_Flag) {
            this->errorReporter().error(offset,
                                        "'tracked' is only permitted within fragment processors");
        }
        if (modifiers.fLayout.fCType != Layout::CType::kDefault) {
            this->errorReporter().error(offset,
                                        "'ctype' is only permitted within fragment processors");
        }
        if (modifiers.fLayout.fKey) {
            this->errorReporter().error(offset,
                                        "'key' is only permitted within fragment processors");
        }
    }
    if (fKind == ProgramKind::kRuntimeEffect) {
        if ((modifiers.fFlags & Modifiers::kIn_Flag) &&
            *baseType != *fContext.fTypes.fFragmentProcessor) {
            this->errorReporter().error(offset,
                                        "'in' variables not permitted in runtime effects");
        }
    }
    if (modifiers.fLayout.fKey && (modifiers.fFlags & Modifiers::kUniform_Flag)) {
        this->errorReporter().error(offset, "'key' is not permitted on 'uniform' variables");
    }
    if (modifiers.fLayout.fMarker.fLength) {
        if (fKind != ProgramKind::kRuntimeEffect) {
            this->errorReporter().error(offset,
                                        "'marker' is only permitted in runtime effects");
        }
        if (!(modifiers.fFlags & Modifiers::kUniform_Flag)) {
            this->errorReporter().error(offset,
                                        "'marker' is only permitted on 'uniform' variables");
        }
        if (*baseType != *fContext.fTypes.fFloat4x4) {
            this->errorReporter().error(offset,
                                        "'marker' is only permitted on float4x4 variables");
        }
    }
    if (modifiers.fLayout.fFlags & Layout::kSRGBUnpremul_Flag) {
        if (fKind != ProgramKind::kRuntimeEffect) {
            this->errorReporter().error(offset,
                                        "'srgb_unpremul' is only permitted in runtime effects");
        }
        if (!(modifiers.fFlags & Modifiers::kUniform_Flag)) {
            this->errorReporter().error(offset,
                                        "'srgb_unpremul' is only permitted on 'uniform' variables");
        }
        auto validColorXformType = [](const Type& t) {
            return t.isVector() && t.componentType().isFloat() &&
                   (t.columns() == 3 || t.columns() == 4);
        };
        if (!validColorXformType(*baseType) && !(baseType->isArray() &&
                                                 validColorXformType(baseType->componentType()))) {
            this->errorReporter().error(offset,
                                        "'srgb_unpremul' is only permitted on half3, half4, "
                                        "float3, or float4 variables");
        }
    }
    if (modifiers.fFlags & Modifiers::kVarying_Flag) {
        if (fKind != ProgramKind::kRuntimeEffect) {
            this->errorReporter().error(offset, "'varying' is only permitted in runtime effects");
        }
        if (!baseType->isFloat() &&
            !(baseType->isVector() && baseType->componentType().isFloat())) {
            this->errorReporter().error(offset, "'varying' must be float scalar or vector");
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
    this->checkModifiers(offset, modifiers, permitted);
}

std::unique_ptr<Statement> IRGenerator::convertVarDeclaration(int offset,
                                                              const Modifiers& modifiers,
                                                              const Type* baseType,
                                                              StringFragment name,
                                                              bool isArray,
                                                              std::unique_ptr<Expression> arraySize,
                                                              std::unique_ptr<Expression> value,
                                                              Variable::Storage storage) {
    if (modifiers.fLayout.fLocation == 0 && modifiers.fLayout.fIndex == 0 &&
        (modifiers.fFlags & Modifiers::kOut_Flag) && fKind == ProgramKind::kFragment &&
        name != "sk_FragColor") {
        this->errorReporter().error(offset,
                                    "out location=0, index=0 is reserved for sk_FragColor");
    }
    const Type* type = baseType;
    int arraySizeValue = 0;
    if (isArray) {
        SkASSERT(arraySize);
        arraySizeValue = this->convertArraySize(*type, std::move(arraySize));
        if (!arraySizeValue) {
            return {};
        }
        type = fSymbolTable->addArrayDimension(type, arraySizeValue);
    }
    auto var = std::make_unique<Variable>(offset, fModifiers->addToPool(modifiers),
                                          name, type, fIsBuiltinCode, storage);
    if (var->name() == Compiler::RTADJUST_NAME) {
        SkASSERT(!fRTAdjust);
        SkASSERT(var->type() == *fContext.fTypes.fFloat4);
        fRTAdjust = var.get();
    }
    if (value) {
        if (type->isOpaque()) {
            this->errorReporter().error(
                    value->fOffset,
                    "opaque type '" + type->name() + "' cannot use initializer expressions");
        }
        if (modifiers.fFlags & Modifiers::kIn_Flag) {
            this->errorReporter().error(value->fOffset,
                                        "'in' variables cannot use initializer expressions");
        }
        value = this->coerce(std::move(value), *type);
        if (!value) {
            return {};
        }
    }
    const Symbol* symbol = (*fSymbolTable)[var->name()];
    if (symbol && storage == Variable::Storage::kGlobal && var->name() == "sk_FragColor") {
        // Already defined, ignore.
        return nullptr;
    } else {
        auto result = std::make_unique<VarDeclaration>(var.get(), baseType, arraySizeValue,
                                                       std::move(value));
        var->setDeclaration(result.get());
        fSymbolTable->add(std::move(var));
        return std::move(result);
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

    this->checkVarDeclaration(decls.fOffset, modifiers, baseType, storage);

    StatementArray varDecls;
    for (; declarationsIter != decls.end(); ++declarationsIter) {
        const ASTNode& varDecl = *declarationsIter;
        const ASTNode::VarData& varData = varDecl.getVarData();
        std::unique_ptr<Expression> arraySize;
        std::unique_ptr<Expression> value;
        auto iter = varDecl.begin();
        if (iter != varDecl.end() && varData.fIsArray) {
            if (*iter) {
                arraySize = this->convertExpression(*iter++);
            } else {
                this->errorReporter().error(decls.fOffset, "array must have a size");
                return {};
            }
        }
        if (iter != varDecl.end()) {
            value = this->convertExpression(*iter);
            if (!value) {
                return {};
            }
        }
        std::unique_ptr<Statement> varDeclStmt = this->convertVarDeclaration(varDecl.fOffset,
                                                                             modifiers,
                                                                             baseType,
                                                                             varData.fName,
                                                                             varData.fIsArray,
                                                                             std::move(arraySize),
                                                                             std::move(value),
                                                                             storage);
        if (varDeclStmt) {
            varDecls.push_back(std::move(varDeclStmt));
        }
    }
    return varDecls;
}

std::unique_ptr<ModifiersDeclaration> IRGenerator::convertModifiersDeclaration(const ASTNode& m) {
    if (fKind != ProgramKind::kFragment &&
        fKind != ProgramKind::kVertex &&
        fKind != ProgramKind::kGeometry) {
        this->errorReporter().error(m.fOffset, "layout qualifiers are not allowed here");
        return nullptr;
    }

    SkASSERT(m.fKind == ASTNode::Kind::kModifiers);
    Modifiers modifiers = m.getModifiers();
    if (modifiers.fLayout.fInvocations != -1) {
        if (fKind != ProgramKind::kGeometry) {
            this->errorReporter().error(m.fOffset,
                                        "'invocations' is only legal in geometry shaders");
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
    std::unique_ptr<Expression> test = this->convertExpression(*(iter++));
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
    bool isStatic = n.getBool();
    return this->convertIf(n.fOffset, isStatic, std::move(test), std::move(ifTrue),
                           std::move(ifFalse));
}

std::unique_ptr<Statement> IRGenerator::convertIf(int offset, bool isStatic,
                                                  std::unique_ptr<Expression> test,
                                                  std::unique_ptr<Statement> ifTrue,
                                                  std::unique_ptr<Statement> ifFalse) {
    test = this->coerce(std::move(test), *fContext.fTypes.fBool);
    if (!test) {
        return nullptr;
    }
    if (this->detectVarDeclarationWithoutScope(*ifTrue)) {
        return nullptr;
    }
    if (ifFalse && this->detectVarDeclarationWithoutScope(*ifFalse)) {
        return nullptr;
    }
    if (test->is<BoolLiteral>()) {
        // Static Boolean values can fold down to a single branch.
        if (test->as<BoolLiteral>().value()) {
            return ifTrue;
        }
        if (ifFalse) {
            return ifFalse;
        }
        // False, but no else-clause. Not an error, so don't return null!
        return std::make_unique<Nop>();
    }
    return std::make_unique<IfStatement>(offset, isStatic, std::move(test), std::move(ifTrue),
                                         std::move(ifFalse));
}

std::unique_ptr<Statement> IRGenerator::convertFor(int offset,
                                                   std::unique_ptr<Statement> initializer,
                                                   std::unique_ptr<Expression> test,
                                                   std::unique_ptr<Expression> next,
                                                   std::unique_ptr<Statement> statement) {
    if (test) {
        test = this->coerce(std::move(test), *fContext.fTypes.fBool);
        if (!test) {
            return nullptr;
        }
    }

    auto forStmt =
            std::make_unique<ForStatement>(offset, std::move(initializer), std::move(test),
                                           std::move(next), std::move(statement), fSymbolTable);
    if (this->strictES2Mode()) {
        if (!Analysis::ForLoopIsValidForES2(*forStmt, /*outLoopInfo=*/nullptr,
                                            &this->errorReporter())) {
            return nullptr;
        }
    }
    return std::move(forStmt);
}

std::unique_ptr<Statement> IRGenerator::convertFor(const ASTNode& f) {
    SkASSERT(f.fKind == ASTNode::Kind::kFor);
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
        test = this->convertExpression(*iter);
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

    return this->convertFor(f.fOffset, std::move(initializer), std::move(test), std::move(next),
                            std::move(statement));
}

std::unique_ptr<Statement> IRGenerator::convertWhile(int offset, std::unique_ptr<Expression> test,
                                                     std::unique_ptr<Statement> statement) {
    if (this->strictES2Mode()) {
        this->errorReporter().error(offset, "while loops are not supported");
        return nullptr;
    }

    test = this->coerce(std::move(test), *fContext.fTypes.fBool);
    if (!test) {
        return nullptr;
    }
    if (this->detectVarDeclarationWithoutScope(*statement)) {
        return nullptr;
    }

    return std::make_unique<ForStatement>(offset, /*initializer=*/nullptr, std::move(test),
                                          /*next=*/nullptr, std::move(statement), fSymbolTable);
}

std::unique_ptr<Statement> IRGenerator::convertWhile(const ASTNode& w) {
    SkASSERT(w.fKind == ASTNode::Kind::kWhile);
    auto iter = w.begin();
    std::unique_ptr<Expression> test = this->convertExpression(*(iter++));
    if (!test) {
        return nullptr;
    }
    std::unique_ptr<Statement> statement = this->convertStatement(*(iter++));
    if (!statement) {
        return nullptr;
    }
    return this->convertWhile(w.fOffset, std::move(test), std::move(statement));
}

std::unique_ptr<Statement> IRGenerator::convertDo(std::unique_ptr<Statement> stmt,
                                                  std::unique_ptr<Expression> test) {
    if (this->strictES2Mode()) {
        this->errorReporter().error(stmt->fOffset, "do-while loops are not supported");
        return nullptr;
    }

    test = this->coerce(std::move(test), *fContext.fTypes.fBool);
    if (!test) {
        return nullptr;
    }
    if (this->detectVarDeclarationWithoutScope(*stmt)) {
        return nullptr;
    }
    return std::make_unique<DoStatement>(stmt->fOffset, std::move(stmt), std::move(test));
}

std::unique_ptr<Statement> IRGenerator::convertDo(const ASTNode& d) {
    SkASSERT(d.fKind == ASTNode::Kind::kDo);
    auto iter = d.begin();
    std::unique_ptr<Statement> statement = this->convertStatement(*(iter++));
    if (!statement) {
        return nullptr;
    }
    std::unique_ptr<Expression> test = this->convertExpression(*(iter++));
    if (!test) {
        return nullptr;
    }
    return this->convertDo(std::move(statement), std::move(test));
}

std::unique_ptr<Statement> IRGenerator::convertSwitch(
                                                int offset,
                                                bool isStatic,
                                                std::unique_ptr<Expression> value,
                                                ExpressionArray caseValues,
                                                SkTArray<StatementArray> caseStatements,
                                                std::shared_ptr<SymbolTable> symbolTable) {
    SkASSERT(caseValues.size() == caseStatements.size());
    if (this->strictES2Mode()) {
        this->errorReporter().error(offset, "switch statements are not supported");
        return nullptr;
    }

    if (!value->type().isEnum()) {
        value = this->coerce(std::move(value), *fContext.fTypes.fInt);
        if (!value) {
            return nullptr;
        }
    }
    SkTHashSet<SKSL_INT> intValues;
    std::vector<std::unique_ptr<SwitchCase>> cases;
    for (size_t i = 0; i < caseValues.size(); ++i) {
        int caseOffset;
        std::unique_ptr<Expression> caseValue;
        if (caseValues[i]) {
            caseOffset = caseValues[i]->fOffset;
            caseValue = this->coerce(std::move(caseValues[i]), value->type());
            if (!caseValue) {
                return nullptr;
            }
            SKSL_INT v = 0;
            if (!ConstantFolder::GetConstantInt(*caseValue, &v)) {
                this->errorReporter().error(caseValue->fOffset,
                                            "case value must be a constant integer");
                return nullptr;
            }
            if (intValues.contains(v)) {
                this->errorReporter().error(caseValue->fOffset, "duplicate case value");
            }
            intValues.add(v);
        } else {
            caseOffset = offset;
        }
        cases.push_back(std::make_unique<SwitchCase>(caseOffset, std::move(caseValue),
                                                     std::move(caseStatements[i])));
    }
    return std::make_unique<SwitchStatement>(offset, isStatic, std::move(value),
                                             std::move(cases), symbolTable);
}

std::unique_ptr<Statement> IRGenerator::convertSwitch(const ASTNode& s) {
    SkASSERT(s.fKind == ASTNode::Kind::kSwitch);

    auto iter = s.begin();
    std::unique_ptr<Expression> value = this->convertExpression(*(iter++));
    if (!value) {
        return nullptr;
    }
    AutoSymbolTable table(this);
    ExpressionArray caseValues;
    SkTArray<StatementArray> caseStatements;
    for (; iter != s.end(); ++iter) {
        const ASTNode& c = *iter;
        SkASSERT(c.fKind == ASTNode::Kind::kSwitchCase);
        std::unique_ptr<Expression>& caseValue = caseValues.emplace_back();
        auto childIter = c.begin();
        if (*childIter) {
            caseValue = this->convertExpression(*childIter);
            if (!caseValue) {
                return nullptr;
            }
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
        caseStatements.push_back(std::move(statements));
    }
    return this->convertSwitch(s.fOffset, s.getBool(), std::move(value), std::move(caseValues),
                               std::move(caseStatements), fSymbolTable);
}

std::unique_ptr<Statement> IRGenerator::convertExpressionStatement(const ASTNode& s) {
    std::unique_ptr<Expression> e = this->convertExpression(s);
    if (!e) {
        return nullptr;
    }
    return std::unique_ptr<Statement>(new ExpressionStatement(std::move(e)));
}

std::unique_ptr<Statement> IRGenerator::convertReturn(int offset,
                                                      std::unique_ptr<Expression> result) {
    if (result) {
        return std::make_unique<ReturnStatement>(std::move(result));
    } else {
        return std::make_unique<ReturnStatement>(offset);
    }
}

std::unique_ptr<Statement> IRGenerator::convertReturn(const ASTNode& r) {
    SkASSERT(r.fKind == ASTNode::Kind::kReturn);
    if (r.begin() != r.end()) {
        std::unique_ptr<Expression> value = this->convertExpression(*r.begin());
        if (!value) {
            return nullptr;
        }
        return this->convertReturn(r.fOffset, std::move(value));
    } else {
        return this->convertReturn(r.fOffset, /*result=*/nullptr);
    }
}

std::unique_ptr<Statement> IRGenerator::convertBreak(const ASTNode& b) {
    SkASSERT(b.fKind == ASTNode::Kind::kBreak);
    return std::make_unique<BreakStatement>(b.fOffset);
}

std::unique_ptr<Statement> IRGenerator::convertContinue(const ASTNode& c) {
    SkASSERT(c.fKind == ASTNode::Kind::kContinue);
    return std::make_unique<ContinueStatement>(c.fOffset);
}

std::unique_ptr<Statement> IRGenerator::convertDiscard(const ASTNode& d) {
    SkASSERT(d.fKind == ASTNode::Kind::kDiscard);
    if (fKind != ProgramKind::kFragment && fKind != ProgramKind::kFragmentProcessor) {
        this->errorReporter().error(d.fOffset,
                                    "discard statement is only permitted in fragment shaders");
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
            fContext.fTypes.fVoid.get(),
            fIsBuiltinCode));
    auto invokeDef = std::make_unique<FunctionDefinition>(/*offset=*/-1, invokeDecl, fIsBuiltinCode,
                                                          std::move(main));
    invokeDecl->setDefinition(invokeDef.get());
    fProgramElements->push_back(std::move(invokeDef));
    std::vector<std::unique_ptr<VarDeclaration>> variables;
    const Variable* loopIdx = &(*fSymbolTable)["sk_InvocationID"]->as<Variable>();
    auto test = std::make_unique<BinaryExpression>(
            /*offset=*/-1,
            std::make_unique<VariableReference>(/*offset=*/-1, loopIdx),
            Token::Kind::TK_LT,
            std::make_unique<IntLiteral>(fContext, /*offset=*/-1, fInvocations),
            fContext.fTypes.fBool.get());
    auto next = std::make_unique<PostfixExpression>(
            std::make_unique<VariableReference>(/*offset=*/-1, loopIdx,
                                                VariableReference::RefKind::kReadWrite),
            Token::Kind::TK_PLUSPLUS);
    ASTNode endPrimitiveID(&fFile->fNodes, -1, ASTNode::Kind::kIdentifier, "EndPrimitive");
    std::unique_ptr<Expression> endPrimitive = this->convertExpression(endPrimitiveID);
    SkASSERT(endPrimitive);

    StatementArray loopBody;
    loopBody.reserve_back(2);
    loopBody.push_back(std::make_unique<ExpressionStatement>(this->call(/*offset=*/-1,
                                                                        *invokeDecl,
                                                                        ExpressionArray{})));
    loopBody.push_back(std::make_unique<ExpressionStatement>(this->call(/*offset=*/-1,
                                                                        std::move(endPrimitive),
                                                                        ExpressionArray{})));
    auto assignment = std::make_unique<BinaryExpression>(
            /*offset=*/-1,
            std::make_unique<VariableReference>(/*offset=*/-1, loopIdx,
                                                VariableReference::RefKind::kWrite),
            Token::Kind::TK_EQ,
            std::make_unique<IntLiteral>(fContext, /*offset=*/-1, /*value=*/0),
            fContext.fTypes.fInt.get());
    auto initializer = std::make_unique<ExpressionStatement>(std::move(assignment));
    auto loop = std::make_unique<ForStatement>(/*offset=*/-1,
                                               std::move(initializer),
                                               std::move(test), std::move(next),
                                               std::make_unique<Block>(-1, std::move(loopBody)),
                                               fSymbolTable);
    StatementArray children;
    children.push_back(std::move(loop));
    return std::make_unique<Block>(/*offset=*/-1, std::move(children));
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
        return std::make_unique<VariableReference>(/*offset=*/-1, var,
                                                   VariableReference::RefKind::kRead);
    };
    auto WRef = [](const Variable* var) -> std::unique_ptr<Expression> {
        return std::make_unique<VariableReference>(/*offset=*/-1, var,
                                                   VariableReference::RefKind::kWrite);
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
        return std::make_unique<BinaryExpression>(/*offset=*/-1, std::move(left), op,
                                                  std::move(right), fContext.fTypes.fFloat2.get());
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
                                                               fContext.fTypes.fFloat4.get(),
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
    #define CHECK(flag, name)                                                            \
        if (!flags) return;                                                              \
        if (flags & flag) {                                                              \
            if (!(permitted & flag)) {                                                   \
                this->errorReporter().error(offset, "'" name "' is not permitted here"); \
            }                                                                            \
            flags &= ~flag;                                                              \
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

void IRGenerator::finalizeFunction(FunctionDefinition& f) {
    class Finalizer : public ProgramWriter {
    public:
        Finalizer(IRGenerator* irGenerator, const FunctionDeclaration* function)
            : fIRGenerator(irGenerator)
            , fFunction(function) {}

        ~Finalizer() override {
            SkASSERT(!fBreakableLevel);
            SkASSERT(!fContinuableLevel);
        }

        bool visitStatement(Statement& stmt) override {
            switch (stmt.kind()) {
                case Statement::Kind::kReturn: {
                    // early returns from a vertex main function will bypass the sk_Position
                    // normalization, so SkASSERT that we aren't doing that. It is of course
                    // possible to fix this by adding a normalization before each return, but it
                    // will probably never actually be necessary.
                    SkASSERT(fIRGenerator->fKind != ProgramKind::kVertex ||
                             !fIRGenerator->fRTAdjust ||
                             fFunction->name() != "main");
                    ReturnStatement& r = stmt.as<ReturnStatement>();
                    const Type& returnType = fFunction->returnType();
                    std::unique_ptr<Expression> result;
                    if (r.expression()) {
                        if (returnType == *fIRGenerator->fContext.fTypes.fVoid) {
                            fIRGenerator->errorReporter().error(r.fOffset,
                                                     "may not return a value from a void function");
                        } else {
                            result = fIRGenerator->coerce(std::move(r.expression()), returnType);
                        }
                    } else if (returnType != *fIRGenerator->fContext.fTypes.fVoid) {
                        fIRGenerator->errorReporter().error(r.fOffset,
                                                    "expected function to return '" +
                                                    returnType.displayName() + "'");
                    }
                    r.setExpression(std::move(result));
                    break;
                }
                case Statement::Kind::kDo:
                case Statement::Kind::kFor: {
                    ++fBreakableLevel;
                    ++fContinuableLevel;
                    bool result = INHERITED::visitStatement(stmt);
                    --fContinuableLevel;
                    --fBreakableLevel;
                    return result;
                }
                case Statement::Kind::kSwitch: {
                    ++fBreakableLevel;
                    bool result = INHERITED::visitStatement(stmt);
                    --fBreakableLevel;
                    return result;
                }
                case Statement::Kind::kBreak:
                    if (!fBreakableLevel) {
                        fIRGenerator->errorReporter().error(stmt.fOffset,
                                                 "break statement must be inside a loop or switch");
                    }
                    break;
                case Statement::Kind::kContinue:
                    if (!fContinuableLevel) {
                        fIRGenerator->errorReporter().error(stmt.fOffset,
                                                        "continue statement must be inside a loop");
                    }
                    break;
                default:
                    break;
            }
            return INHERITED::visitStatement(stmt);
        }

    private:
        IRGenerator* fIRGenerator;
        const FunctionDeclaration* fFunction;
        // how deeply nested we are in breakable constructs (for, do, switch).
        int fBreakableLevel = 0;
        // how deeply nested we are in continuable constructs (for, do).
        int fContinuableLevel = 0;

        using INHERITED = ProgramWriter;
    };
    Finalizer(this, &f.declaration()).visitStatement(*f.body());
}

void IRGenerator::convertFunction(const ASTNode& f) {
    AutoClear clear(&fReferencedIntrinsics);
    auto iter = f.begin();
    const Type* returnType = this->convertType(*(iter++), /*allowVoid=*/true);
    if (returnType == nullptr) {
        return;
    }
    if (returnType->isArray()) {
        this->errorReporter().error(
                f.fOffset, "functions may not return type '" + returnType->displayName() + "'");
        return;
    }
    if (!fIsBuiltinCode && *returnType != *fContext.fTypes.fVoid &&
        returnType->componentType().isOpaque()) {
        this->errorReporter().error(
                f.fOffset,
                "functions may not return opaque type '" + returnType->displayName() + "'");
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
            int arraySize = this->convertArraySize(*type, param.fOffset, *paramIter++);
            if (!arraySize) {
                return;
            }
            type = fSymbolTable->addArrayDimension(type, arraySize);
        }
        // Only the (builtin) declarations of 'sample' are allowed to have FP parameters.
        // (You can pass other opaque types to functions safely; this restriction is
        // fragment-processor specific.)
        if (*type == *fContext.fTypes.fFragmentProcessor && !fIsBuiltinCode) {
            this->errorReporter().error(
                    param.fOffset, "parameters of type '" + type->displayName() + "' not allowed");
            return;
        }

        Modifiers m = pd.fModifiers;
        if (funcData.fName == "main" && (fKind == ProgramKind::kRuntimeEffect ||
                                         fKind == ProgramKind::kFragmentProcessor)) {
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
        return parameters[idx]->type() == *fContext.fTypes.fFloat2 &&
               parameters[idx]->modifiers().fFlags == 0 &&
               parameters[idx]->modifiers().fLayout.fBuiltin == SK_MAIN_COORDS_BUILTIN;
    };

    if (funcData.fName == "main") {
        switch (fKind) {
            case ProgramKind::kRuntimeEffect: {
                // (half4|float4) main()  -or-  (half4|float4) main(float2)
                if (*returnType != *fContext.fTypes.fHalf4 &&
                    *returnType != *fContext.fTypes.fFloat4) {
                    this->errorReporter().error(f.fOffset,
                                                "'main' must return: 'vec4', 'float4', or 'half4'");
                    return;
                }
                bool validParams = (parameters.size() == 0) ||
                                   (parameters.size() == 1 && paramIsCoords(0));
                if (!validParams) {
                    this->errorReporter().error(
                            f.fOffset, "'main' parameters must be: (), (vec2), or (float2)");
                    return;
                }
                break;
            }
            case ProgramKind::kFragmentProcessor: {
                if (*returnType != *fContext.fTypes.fHalf4) {
                    this->errorReporter().error(f.fOffset, ".fp 'main' must return 'half4'");
                    return;
                }
                bool validParams = (parameters.size() == 0) ||
                                   (parameters.size() == 1 && paramIsCoords(0));
                if (!validParams) {
                    this->errorReporter().error(
                            f.fOffset, ".fp 'main' must be declared main() or main(float2)");
                    return;
                }
                break;
            }
            case ProgramKind::kGeneric:
                break;
            default:
                if (parameters.size()) {
                    this->errorReporter().error(f.fOffset,
                                                "shader 'main' must have zero parameters");
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
                this->errorReporter().error(f.fOffset,
                                            "symbol '" + funcData.fName + "' was already defined");
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
                        this->errorReporter().error(
                                f.fOffset, "functions '" + newDecl.description() + "' and '" +
                                           other->description() + "' differ only in return type");
                        return;
                    }
                    decl = other;
                    for (size_t i = 0; i < parameters.size(); i++) {
                        if (parameters[i]->modifiers() != other->parameters()[i]->modifiers()) {
                            this->errorReporter().error(
                                    f.fOffset,
                                    "modifiers on parameter " + to_string((uint64_t)i + 1) +
                                            " differ between declaration and definition");
                            return;
                        }
                    }
                    if (other->definition() && !other->isBuiltin()) {
                        this->errorReporter().error(
                                f.fOffset, "duplicate definition of " + other->description());
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
        AutoSymbolTable table(this);
        for (const Variable* param : decl->parameters()) {
            fSymbolTable->addWithoutOwnership(param);
        }
        bool needInvocationIDWorkaround = fInvocations != -1 && funcData.fName == "main" &&
                                          fCaps && !fCaps->gsInvocationsSupport();
        std::unique_ptr<Block> body = this->convertBlock(*iter);
        if (!body) {
            return;
        }
        if (needInvocationIDWorkaround) {
            body = this->applyInvocationIDWorkaround(std::move(body));
        }
        if (ProgramKind::kVertex == fKind && funcData.fName == "main" && fRTAdjust) {
            body->children().push_back(this->getNormalizeSkPositionCode());
        }
        auto result = std::make_unique<FunctionDefinition>(
                f.fOffset, decl, fIsBuiltinCode, std::move(body), std::move(fReferencedIntrinsics));
        this->finalizeFunction(*result);
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
        this->errorReporter().error(node.fOffset,
                                    "expected a struct here, found '" + type->name() + "'");
        return nullptr;
    }
    SkDEBUGCODE(auto [iter, wasInserted] =) fDefinedStructs.insert(type);
    SkASSERT(wasInserted);
    return std::make_unique<StructDefinition>(node.fOffset, *type);
}

std::unique_ptr<InterfaceBlock> IRGenerator::convertInterfaceBlock(const ASTNode& intf) {
    if (fKind != ProgramKind::kFragment &&
        fKind != ProgramKind::kVertex &&
        fKind != ProgramKind::kGeometry) {
        this->errorReporter().error(intf.fOffset, "interface block is not allowed here");
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
        for (size_t i = 0; i < id.fDeclarationCount; ++i) {
            StatementArray decls = this->convertVarDeclarations(*(iter++),
                                                                Variable::Storage::kInterfaceBlock);
            if (decls.empty()) {
                return nullptr;
            }
            for (const auto& decl : decls) {
                const VarDeclaration& vd = decl->as<VarDeclaration>();
                if (vd.var().type().isOpaque()) {
                    this->errorReporter().error(decl->fOffset,
                                                "opaque type '" + vd.var().type().name() +
                                                        "' is not permitted in an interface block");
                }
                if (&vd.var() == fRTAdjust) {
                    foundRTAdjust = true;
                    SkASSERT(vd.var().type() == *fContext.fTypes.fFloat4);
                    fRTAdjustFieldIndex = fields.size();
                }
                fields.push_back(Type::Field(vd.var().modifiers(), vd.var().name(),
                                            &vd.var().type()));
                if (vd.value()) {
                    this->errorReporter().error(
                            decl->fOffset,
                            "initializers are not permitted on interface block fields");
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
            // convertArraySize rejects unsized arrays. This is the one place we allow those, but
            // we've already checked for that, so this is verifying the other aspects (constant,
            // positive, not too large).
            arraySize = this->convertArraySize(*type, size.fOffset, size);
            if (!arraySize) {
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

void IRGenerator::convertGlobalVarDeclarations(const ASTNode& decl) {
    StatementArray decls = this->convertVarDeclarations(decl, Variable::Storage::kGlobal);
    for (std::unique_ptr<Statement>& stmt : decls) {
        const Type* type = &stmt->as<VarDeclaration>().baseType();
        if (type->isStruct()) {
            auto [iter, wasInserted] = fDefinedStructs.insert(type);
            if (wasInserted) {
                fProgramElements->push_back(
                        std::make_unique<StructDefinition>(decl.fOffset, *type));
            }
        }
        fProgramElements->push_back(std::make_unique<GlobalVarDeclaration>(decl.fOffset,
                                                                           std::move(stmt)));
    }
}

void IRGenerator::convertEnum(const ASTNode& e) {
    if (this->strictES2Mode()) {
        this->errorReporter().error(e.fOffset, "enum is not allowed here");
        return;
    }

    SkASSERT(e.fKind == ASTNode::Kind::kEnum);
    SKSL_INT currentValue = 0;
    Layout layout;
    ASTNode enumType(e.fNodes, e.fOffset, ASTNode::Kind::kType, e.getString());
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
            if (!ConstantFolder::GetConstantInt(*value, &currentValue)) {
                this->errorReporter().error(value->fOffset,
                                            "enum value must be a constant integer");
                fSymbolTable = oldTable;
                return;
            }
        }
        value = std::make_unique<IntLiteral>(fContext, e.fOffset, currentValue);
        ++currentValue;
        auto var = std::make_unique<Variable>(e.fOffset, fModifiers->addToPool(modifiers),
                                              child.getString(), type, fIsBuiltinCode,
                                              Variable::Storage::kGlobal);
        // enum variables aren't really 'declared', but we have to create a declaration to store
        // the value
        auto declaration = std::make_unique<VarDeclaration>(var.get(), &var->type(),
                                                            /*arraySize=*/0, std::move(value));
        var->setDeclaration(declaration.get());
        fSymbolTable->add(std::move(var));
        fSymbolTable->takeOwnershipOfIRNode(std::move(declaration));
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
    StringFragment name = type.getString();
    const Symbol* symbol = (*fSymbolTable)[name];
    if (!symbol || !symbol->is<Type>()) {
        this->errorReporter().error(type.fOffset, "unknown type '" + name + "'");
        return nullptr;
    }
    const Type* result = &symbol->as<Type>();
    const bool isArray = (type.begin() != type.end());
    if (*result == *fContext.fTypes.fVoid && !allowVoid) {
        this->errorReporter().error(type.fOffset,
                                    "type '" + name + "' not allowed in this context");
        return nullptr;
    }
    if (!fIsBuiltinCode && this->typeContainsPrivateFields(*result)) {
        this->errorReporter().error(type.fOffset, "type '" + name + "' is private");
        return nullptr;
    }
    if (isArray) {
        auto iter = type.begin();
        int arraySize = this->convertArraySize(*result, type.fOffset, *iter);
        if (!arraySize) {
            return nullptr;
        }
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
        case ASTNode::Kind::kPostfix:
            return this->convertPostfixExpression(expr);
        case ASTNode::Kind::kPrefix:
            return this->convertPrefixExpression(expr);
        case ASTNode::Kind::kScope:
            return this->convertScopeExpression(expr);
        case ASTNode::Kind::kTernary:
            return this->convertTernaryExpression(expr);
        default:
            SkDEBUGFAILF("unsupported expression: %s\n", expr.description().c_str());
            return nullptr;
    }
}

std::unique_ptr<Expression> IRGenerator::convertIdentifier(int offset, StringFragment name) {
    const Symbol* result = (*fSymbolTable)[name];
    if (!result) {
        this->errorReporter().error(offset, "unknown identifier '" + name + "'");
        return nullptr;
    }
    switch (result->kind()) {
        case Symbol::Kind::kFunctionDeclaration: {
            std::vector<const FunctionDeclaration*> f = {
                &result->as<FunctionDeclaration>()
            };
            return std::make_unique<FunctionReference>(fContext, offset, f);
        }
        case Symbol::Kind::kUnresolvedFunction: {
            const UnresolvedFunction* f = &result->as<UnresolvedFunction>();
            return std::make_unique<FunctionReference>(fContext, offset, f->functions());
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
            if (fKind == ProgramKind::kFragmentProcessor &&
                (modifiers.fFlags & Modifiers::kIn_Flag) &&
                !(modifiers.fFlags & Modifiers::kUniform_Flag) &&
                !modifiers.fLayout.fKey &&
                modifiers.fLayout.fBuiltin == -1 &&
                var->type() != *fContext.fTypes.fFragmentProcessor &&
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
                    this->errorReporter().error(
                            offset,
                            "'in' variable must be either 'uniform' or 'layout(key)', or there "
                            "must be a custom @setData function");
                }
            }
            // default to kRead_RefKind; this will be corrected later if the variable is written to
            return std::make_unique<VariableReference>(offset,
                                                       var,
                                                       VariableReference::RefKind::kRead);
        }
        case Symbol::Kind::kField: {
            const Field* field = &result->as<Field>();
            auto base = std::make_unique<VariableReference>(offset, &field->owner(),
                                                            VariableReference::RefKind::kRead);
            return std::make_unique<FieldAccess>(std::move(base),
                                                 field->fieldIndex(),
                                                 FieldAccess::OwnerKind::kAnonymousInterfaceBlock);
        }
        case Symbol::Kind::kType: {
            const Type* t = &result->as<Type>();
            return std::make_unique<TypeReference>(fContext, offset, t);
        }
        case Symbol::Kind::kExternal: {
            const ExternalFunction* r = &result->as<ExternalFunction>();
            return std::make_unique<ExternalFunctionReference>(offset, r);
        }
        default:
            SK_ABORT("unsupported symbol type %d\n", (int) result->kind());
    }
}

std::unique_ptr<Expression> IRGenerator::convertIdentifier(const ASTNode& identifier) {
    return this->convertIdentifier(identifier.fOffset, identifier.getString());
}

std::unique_ptr<Section> IRGenerator::convertSection(const ASTNode& s) {
    if (fKind != ProgramKind::kFragmentProcessor) {
        this->errorReporter().error(s.fOffset, "syntax error");
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
    if (expr->type() == *fContext.fTypes.fInvalid) {
        return nullptr;
    }
    int offset = expr->fOffset;
    if (!expr->coercionCost(type).isPossible(fSettings->fAllowNarrowingConversions)) {
        this->errorReporter().error(offset, "expected '" + type.displayName() + "', but found '" +
                                                    expr->type().displayName() + "'");
        return nullptr;
    }
    ExpressionArray args;
    args.push_back(std::move(expr));
    if (!type.isScalar()) {
        return std::make_unique<Constructor>(offset, &type, std::move(args));
    }
    return this->convertConstructor(offset, type.scalarTypeForLiteral(), std::move(args));
}

static bool is_matrix_multiply(const Type& left, Operator op, const Type& right) {
    if (op.kind() != Token::Kind::TK_STAR && op.kind() != Token::Kind::TK_STAREQ) {
        return false;
    }
    if (left.isMatrix()) {
        return right.isMatrix() || right.isVector();
    }
    return left.isVector() && right.isMatrix();
}

/**
 * Determines the operand and result types of a binary expression. Returns true if the expression is
 * legal, false otherwise. If false, the values of the out parameters are undefined.
 */
static bool determine_binary_type(const Context& context,
                                  bool allowNarrowing,
                                  Operator op,
                                  const Type& left,
                                  const Type& right,
                                  const Type** outLeftType,
                                  const Type** outRightType,
                                  const Type** outResultType) {
    switch (op.kind()) {
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
                    *outResultType = context.fTypes.fBool.get();
                    return true;
                }
            } else {
                if (leftToRight.isPossible(allowNarrowing)) {
                    *outLeftType = &right;
                    *outRightType = &right;
                    *outResultType = context.fTypes.fBool.get();
                    return true;
                }
            }
            return false;
        }
        case Token::Kind::TK_LOGICALOR:   // left || right
        case Token::Kind::TK_LOGICALAND:  // left && right
        case Token::Kind::TK_LOGICALXOR:  // left ^^ right
            *outLeftType = context.fTypes.fBool.get();
            *outRightType = context.fTypes.fBool.get();
            *outResultType = context.fTypes.fBool.get();
            return left.canCoerceTo(*context.fTypes.fBool, allowNarrowing) &&
                   right.canCoerceTo(*context.fTypes.fBool, allowNarrowing);

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

    bool isAssignment = op.isAssignment();
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
    bool validMatrixOrVectorOp = op.isValidForMatrixOrVector();

    if (leftIsVectorOrMatrix && validMatrixOrVectorOp && right.isScalar()) {
        if (determine_binary_type(context, allowNarrowing, op, left.componentType(), right,
                                  outLeftType, outRightType, outResultType)) {
            *outLeftType = &(*outLeftType)->toCompound(context, left.columns(), left.rows());
            if (!op.isLogical()) {
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
            if (!op.isLogical()) {
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
        if (op.isOnlyValidForIntegralTypes()) {
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
        if (op.isLogical()) {
            *outResultType = context.fTypes.fBool.get();
        }
        return true;
    }
    return false;
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
    return this->convertBinaryExpression(std::move(left), expression.getOperator(),
                                         std::move(right));
}

std::unique_ptr<Expression> IRGenerator::convertBinaryExpression(
                                                                std::unique_ptr<Expression> left,
                                                                Operator op,
                                                                std::unique_ptr<Expression> right) {
    if (!left || !right) {
        return nullptr;
    }
    int offset = left->fOffset;
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
    if (this->strictES2Mode() && !op.isAllowedInStrictES2Mode()) {
        this->errorReporter().error(offset,
                                    String("operator '") + op.operatorName() + "' is not allowed");
        return nullptr;
    }
    bool isAssignment = op.isAssignment();
    if (isAssignment && !this->setRefKind(*left, op.kind() != Token::Kind::TK_EQ
                                                 ? VariableReference::RefKind::kReadWrite
                                                 : VariableReference::RefKind::kWrite)) {
        return nullptr;
    }
    if (!determine_binary_type(fContext, fSettings->fAllowNarrowingConversions, op,
                               *rawLeftType, *rawRightType, &leftType, &rightType, &resultType)) {
        this->errorReporter().error(
                offset, String("type mismatch: '") + op.operatorName() +
                                "' cannot operate on '" + left->type().displayName() + "', '" +
                                right->type().displayName() + "'");
        return nullptr;
    }
    if (isAssignment && leftType->componentType().isOpaque()) {
        this->errorReporter().error(offset, "assignments to opaque type '" +
                                            left->type().displayName() + "' are not permitted");
    }
    left = this->coerce(std::move(left), *leftType);
    right = this->coerce(std::move(right), *rightType);
    if (!left || !right) {
        return nullptr;
    }
    std::unique_ptr<Expression> result;
    if (!ConstantFolder::ErrorOnDivideByZero(fContext, offset, op, *right)) {
        result = ConstantFolder::Simplify(fContext, offset, *left, op, *right);
    }
    if (!result) {
        result = std::make_unique<BinaryExpression>(offset, std::move(left), op, std::move(right),
                                                    resultType);
    }
    return result;
}

std::unique_ptr<Expression> IRGenerator::convertTernaryExpression(
                                                              std::unique_ptr<Expression> test,
                                                              std::unique_ptr<Expression> ifTrue,
                                                              std::unique_ptr<Expression> ifFalse) {
    test = this->coerce(std::move(test), *fContext.fTypes.fBool);
    if (!test || !ifTrue || !ifFalse) {
        return nullptr;
    }
    int offset = test->fOffset;
    const Type* trueType;
    const Type* falseType;
    const Type* resultType;
    if (!determine_binary_type(fContext, fSettings->fAllowNarrowingConversions,
                               Token::Kind::TK_EQEQ, ifTrue->type(), ifFalse->type(),
                               &trueType, &falseType, &resultType) ||
        trueType != falseType) {
        this->errorReporter().error(offset, "ternary operator result mismatch: '" +
                                            ifTrue->type().displayName() + "', '" +
                                            ifFalse->type().displayName() + "'");
        return nullptr;
    }
    if (trueType->componentType().isOpaque()) {
        this->errorReporter().error(
                offset,
                "ternary expression of opaque type '" + trueType->displayName() + "' not allowed");
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
    return std::make_unique<TernaryExpression>(offset,
                                               std::move(test),
                                               std::move(ifTrue),
                                               std::move(ifFalse));
}

std::unique_ptr<Expression> IRGenerator::convertTernaryExpression(const ASTNode& node) {
    SkASSERT(node.fKind == ASTNode::Kind::kTernary);
    auto iter = node.begin();
    std::unique_ptr<Expression> test = this->convertExpression(*(iter++));
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
    return this->convertTernaryExpression(std::move(test), std::move(ifTrue), std::move(ifFalse));
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
        this->errorReporter().error(offset, msg);
        return nullptr;
    }
    // GLSL ES 1.0 requires static recursion be rejected by the compiler. Also, our CPU back-end
    // can not handle recursion (and is tied to strictES2Mode front-ends). The safest way to reject
    // all (potentially) recursive code is to disallow calls to functions before they're defined.
    if (this->strictES2Mode() && !function.definition() && !function.isBuiltin()) {
        String msg = "call to undefined function '" + function.name() + "'";
        this->errorReporter().error(offset, msg);
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
        this->errorReporter().error(offset, msg);
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
        case Expression::Kind::kExternalFunctionReference: {
            const ExternalFunction& f = functionValue->as<ExternalFunctionReference>().function();
            int count = f.callParameterCount();
            if (count != (int) arguments.size()) {
                this->errorReporter().error(offset, "external function expected " +
                                                    to_string(count) + " arguments, but found " +
                                                    to_string((int)arguments.size()));
                return nullptr;
            }
            static constexpr int PARAMETER_MAX = 16;
            SkASSERT(count < PARAMETER_MAX);
            const Type* types[PARAMETER_MAX];
            f.getCallParameterTypes(types);
            for (int i = 0; i < count; ++i) {
                arguments[i] = this->coerce(std::move(arguments[i]), *types[i]);
                if (!arguments[i]) {
                    return nullptr;
                }
            }
            return std::make_unique<ExternalFunctionCall>(offset, &f, std::move(arguments));
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
                this->errorReporter().error(offset, msg);
                return nullptr;
            }
            return this->call(offset, *functions[0], std::move(arguments));
        }
        default:
            this->errorReporter().error(offset, "not a function");
            return nullptr;
    }
}

std::unique_ptr<Expression> IRGenerator::convertScalarConstructor(int offset,
                                                                  const Type& type,
                                                                  ExpressionArray args) {
    SkASSERT(type.isScalar());
    if (args.size() != 1) {
        this->errorReporter().error(
                offset, "invalid arguments to '" + type.displayName() +
                        "' constructor, (expected exactly 1 argument, but found " +
                        to_string((uint64_t)args.size()) + ")");
        return nullptr;
    }

    const Type& argType = args[0]->type();
    if (!argType.isScalar()) {
        this->errorReporter().error(
                offset, "invalid argument to '" + type.displayName() +
                        "' constructor (expected a number or bool, but found '" +
                        argType.displayName() + "')");
        return nullptr;
    }

    std::unique_ptr<Expression> converted = Constructor::SimplifyConversion(type, *args[0]);
    if (converted) {
        return converted;
    }
    return std::make_unique<Constructor>(offset, &type, std::move(args));
}

std::unique_ptr<Expression> IRGenerator::convertCompoundConstructor(int offset,
                                                                    const Type& type,
                                                                    ExpressionArray args) {
    SkASSERT(type.isVector() || type.isMatrix());
    if (type.isMatrix() && args.size() == 1 && args[0]->type().isMatrix()) {
        // Matrix-from-matrix is always legal.
        return std::make_unique<Constructor>(offset, &type, std::move(args));
    }

    if (args.size() == 1 && args[0]->type().isScalar()) {
        // A constructor containing a single scalar is a splat (for vectors) or diagonal matrix (for
        // matrices). In either event, it's legal regardless of the scalar's type. Synthesize an
        // explicit conversion to the proper type (this is a no-op if it's unnecessary).
        ExpressionArray castArgs;
        castArgs.push_back(this->convertConstructor(offset, type.componentType(), std::move(args)));
        return std::make_unique<Constructor>(offset, &type, std::move(castArgs));
    }

    int expected = type.rows() * type.columns();

    if (type.isVector() && args.size() == 1 && args[0]->type().isVector() &&
        args[0]->type().columns() == expected) {
        // A vector constructor containing a single vector with the same number of columns is a
        // cast (e.g. float3 -> int3).
        return std::make_unique<Constructor>(offset, &type, std::move(args));
    }

    // For more complex cases, we walk the argument list and fix up the arguments as needed.
    int actual = 0;
    for (std::unique_ptr<Expression>& arg : args) {
        if (!arg->type().isScalar() && !arg->type().isVector()) {
            this->errorReporter().error(offset, "'" + arg->type().displayName() +
                                                "' is not a valid parameter to '" +
                                                type.displayName() + "' constructor");
            return nullptr;
        }

        // Rely on convertConstructor to force this subexpression to the proper type. If it's a
        // literal, this will make sure it's the right type of literal. If an expression of
        // matching type, the expression will be returned as-is. If it's an expression of
        // mismatched type, this adds a cast.
        int offset = arg->fOffset;
        const Type& ctorType = type.componentType().toCompound(fContext, arg->type().columns(),
                                                               /*rows=*/1);
        ExpressionArray ctorArg;
        ctorArg.push_back(std::move(arg));
        arg = this->convertConstructor(offset, ctorType, std::move(ctorArg));
        if (!arg) {
            return nullptr;
        }
        actual += ctorType.columns();
    }

    if (actual != expected) {
        this->errorReporter().error(offset, "invalid arguments to '" + type.displayName() +
                                            "' constructor (expected " + to_string(expected) +
                                            " scalars, but found " + to_string(actual) + ")");
        return nullptr;
    }

    return std::make_unique<Constructor>(offset, &type, std::move(args));
}

std::unique_ptr<Expression> IRGenerator::convertConstructor(int offset,
                                                            const Type& type,
                                                            ExpressionArray args) {
    // FIXME: add support for structs
    if (args.size() == 1 && args[0]->type() == type && !type.componentType().isOpaque()) {
        // Strip off redundant casts--i.e., convert Type(exprOfType) into exprOfType.
        return std::move(args[0]);
    }
    if (type.isScalar()) {
        return this->convertScalarConstructor(offset, type, std::move(args));
    }
    if (type.isVector() || type.isMatrix()) {
        return this->convertCompoundConstructor(offset, type, std::move(args));
    }
    if (type.isArray() && type.columns() > 0) {
        return this->convertArrayConstructor(offset, type, std::move(args));
    }

    this->errorReporter().error(offset, "cannot construct '" + type.displayName() + "'");
    return nullptr;
}

std::unique_ptr<Expression> IRGenerator::convertArrayConstructor(int offset,
                                                                 const Type& type,
                                                                 ExpressionArray args) {
    SkASSERTF(type.isArray() && type.columns() > 0, "%s", type.description().c_str());

    // ES2 doesn't support first-class array types.
    if (this->strictES2Mode()) {
        this->errorReporter().error(
                offset, "construction of array type '" + type.displayName() + "' is not supported");
        return nullptr;
    }

    // Check that the number of constructor arguments matches the array size.
    if (type.columns() != args.count()) {
        this->errorReporter().error(
                offset,
                String::printf("invalid arguments to '%s' constructor "
                               "(expected %d elements, but found %d)",
                               type.displayName().c_str(), type.columns(), args.count()));
        return nullptr;
    }

    // Convert each constructor argument to the array's component type.
    const Type& base = type.componentType();
    for (std::unique_ptr<Expression>& argument : args) {
        argument = this->coerce(std::move(argument), base);
        if (!argument) {
            return nullptr;
        }
    }
    return std::make_unique<Constructor>(offset, &type, std::move(args));
}

std::unique_ptr<Expression> IRGenerator::convertPrefixExpression(const ASTNode& expression) {
    SkASSERT(expression.fKind == ASTNode::Kind::kPrefix);
    std::unique_ptr<Expression> base = this->convertExpression(*expression.begin());
    if (!base) {
        return nullptr;
    }
    return this->convertPrefixExpression(expression.getOperator(), std::move(base));
}

std::unique_ptr<Expression> IRGenerator::convertPrefixExpression(Operator op,
                                                                 std::unique_ptr<Expression> base) {
    const Type& baseType = base->type();
    switch (op.kind()) {
        case Token::Kind::TK_PLUS:
            if (!baseType.componentType().isNumber()) {
                this->errorReporter().error(
                        base->fOffset, "'+' cannot operate on '" + baseType.displayName() + "'");
                return nullptr;
            }
            return base;

        case Token::Kind::TK_MINUS:
            if (!baseType.componentType().isNumber()) {
                this->errorReporter().error(
                        base->fOffset, "'-' cannot operate on '" + baseType.displayName() + "'");
                return nullptr;
            }
            if (base->is<IntLiteral>()) {
                return std::make_unique<IntLiteral>(base->fOffset,
                                                    -base->as<IntLiteral>().value(),
                                                    &base->type());
            }
            if (base->is<FloatLiteral>()) {
                return std::make_unique<FloatLiteral>(base->fOffset,
                                                      -base->as<FloatLiteral>().value(),
                                                      &base->type());
            }
            return std::make_unique<PrefixExpression>(Token::Kind::TK_MINUS, std::move(base));

        case Token::Kind::TK_PLUSPLUS:
            if (!baseType.isNumber()) {
                this->errorReporter().error(base->fOffset,
                                            String("'") + op.operatorName() +
                                            "' cannot operate on '" + baseType.displayName() + "'");
                return nullptr;
            }
            if (!this->setRefKind(*base, VariableReference::RefKind::kReadWrite)) {
                return nullptr;
            }
            break;
        case Token::Kind::TK_MINUSMINUS:
            if (!baseType.isNumber()) {
                this->errorReporter().error(base->fOffset,
                                            String("'") + op.operatorName() +
                                            "' cannot operate on '" + baseType.displayName() + "'");
                return nullptr;
            }
            if (!this->setRefKind(*base, VariableReference::RefKind::kReadWrite)) {
                return nullptr;
            }
            break;
        case Token::Kind::TK_LOGICALNOT:
            if (!baseType.isBoolean()) {
                this->errorReporter().error(base->fOffset,
                                            String("'") + op.operatorName() +
                                            "' cannot operate on '" + baseType.displayName() + "'");
                return nullptr;
            }
            if (base->is<BoolLiteral>()) {
                return std::make_unique<BoolLiteral>(base->fOffset,
                                                     !base->as<BoolLiteral>().value(),
                                                     &base->type());
            }
            break;
        case Token::Kind::TK_BITWISENOT:
            if (this->strictES2Mode()) {
                // GLSL ES 1.00, Section 5.1
                this->errorReporter().error(
                        base->fOffset,
                        String("operator '") + op.operatorName() + "' is not allowed");
                return nullptr;
            }
            if (!baseType.isInteger()) {
                this->errorReporter().error(base->fOffset,
                                            String("'") + op.operatorName() +
                                            "' cannot operate on '" + baseType.displayName() + "'");
                return nullptr;
            }
            break;
        default:
            SK_ABORT("unsupported prefix operator\n");
    }
    return std::make_unique<PrefixExpression>(op, std::move(base));
}

std::unique_ptr<Expression> IRGenerator::convertField(std::unique_ptr<Expression> base,
                                                      StringFragment field) {
    const Type& baseType = base->type();
    auto fields = baseType.fields();
    for (size_t i = 0; i < fields.size(); i++) {
        if (fields[i].fName == field) {
            return std::unique_ptr<Expression>(new FieldAccess(std::move(base), (int) i));
        }
    }
    this->errorReporter().error(
            base->fOffset,
            "type '" + baseType.displayName() + "' does not have a field named '" + field + "'");
    return nullptr;
}

// Swizzles are complicated due to constant components. The most difficult case is a mask like
// '.x1w0'. A naive approach might turn that into 'float4(base.x, 1, base.w, 0)', but that evaluates
// 'base' twice. We instead group the swizzle mask ('xw') and constants ('1, 0') together and use a
// secondary swizzle to put them back into the right order, so in this case we end up with
// 'float4(base.xw, 1, 0).xzyw'.
std::unique_ptr<Expression> IRGenerator::convertSwizzle(std::unique_ptr<Expression> base,
                                                        String fields) {
    const int offset = base->fOffset;
    const Type& baseType = base->type();
    if (!baseType.isVector() && !baseType.isNumber()) {
        this->errorReporter().error(
                offset, "cannot swizzle value of type '" + baseType.displayName() + "'");
        return nullptr;
    }

    if (fields.length() > 4) {
        this->errorReporter().error(offset, "too many components in swizzle mask '" + fields + "'");
        return nullptr;
    }

    ComponentArray maskComponents;
    for (size_t i = 0; i < fields.length(); i++) {
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
                this->errorReporter().error(
                        offset, String::printf("invalid swizzle component '%c'", fields[i]));
                return nullptr;
        }
    }
    if (maskComponents.empty()) {
        this->errorReporter().error(offset, "swizzle must refer to base expression");
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
    if (maskComponents.size() == fields.length()) {
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

    for (size_t i = 0; i < fields.length(); i++) {
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
        this->errorReporter().error(offset, "unknown capability flag '" + name + "'");
        return nullptr;
    }
    switch (found->second.fKind) {
        case CapsValue::kBool_Kind:  return fContext.fTypes.fBool.get();
        case CapsValue::kFloat_Kind: return fContext.fTypes.fFloat.get();
        case CapsValue::kInt_Kind:   return fContext.fTypes.fInt.get();
    }
    SkUNREACHABLE;
    return nullptr;
}

std::unique_ptr<Expression> IRGenerator::valueForSetting(int offset, String name) const {
    auto found = fCapsMap.find(name);
    if (found == fCapsMap.end()) {
        this->errorReporter().error(offset, "unknown capability flag '" + name + "'");
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
        this->errorReporter().error(offset,
                                    "type '" + type.displayName() + "' is not a known enum");
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
        this->errorReporter().error(
                offset, "type '" + type.name() + "' does not contain enumerator '" + field + "'");
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
    if (base->is<TypeReference>()) {
        // Convert an index expression starting with a type name: `int[12]`
        if (iter == index.end()) {
            this->errorReporter().error(index.fOffset, "array must have a size");
            return nullptr;
        }
        const Type* type = &base->as<TypeReference>().value();
        int arraySize = this->convertArraySize(*type, index.fOffset, *iter);
        if (!arraySize) {
            return nullptr;
        }
        type = fSymbolTable->addArrayDimension(type, arraySize);
        return std::make_unique<TypeReference>(fContext, base->fOffset, type);
    }

    if (iter == index.end()) {
        this->errorReporter().error(base->fOffset, "missing index in '[]'");
        return nullptr;
    }
    std::unique_ptr<Expression> converted = this->convertExpression(*(iter++));
    if (!converted) {
        return nullptr;
    }
    return this->convertIndex(std::move(base), std::move(converted));
}

std::unique_ptr<Expression> IRGenerator::convertIndex(std::unique_ptr<Expression> base,
                                                      std::unique_ptr<Expression> index) {
    // Convert an index expression with an expression inside of it: `arr[a * 3]`.
    const Type& baseType = base->type();
    if (!baseType.isArray() && !baseType.isMatrix() && !baseType.isVector()) {
        this->errorReporter().error(base->fOffset,
                                    "expected array, but found '" + baseType.displayName() + "'");
        return nullptr;
    }
    if (!index->type().isInteger()) {
        index = this->coerce(std::move(index), *fContext.fTypes.fInt);
        if (!index) {
            return nullptr;
        }
    }
    // Perform compile-time bounds checking on constant indices.
    if (index->is<IntLiteral>()) {
        SKSL_INT indexValue = index->as<IntLiteral>().value();

        const int upperBound = (baseType.isArray() && baseType.columns() == Type::kUnsizedArray)
                                       ? INT_MAX
                                       : baseType.columns();
        if (indexValue < 0 || indexValue >= upperBound) {
            this->errorReporter().error(base->fOffset, "index " + to_string(indexValue) +
                                                       " out of range for '" +
                                                       baseType.displayName() + "'");
            return nullptr;
        }
        // Constant array indexes on vectors can be converted to swizzles: `myHalf4.z`.
        // (Using a swizzle gives our optimizer a bit more to work with, compared to array indices.)
        if (baseType.isVector()) {
            return std::make_unique<Swizzle>(fContext, std::move(base),
                                             ComponentArray{(int8_t)indexValue});
        }
    }
    return std::make_unique<IndexExpression>(fContext, std::move(base), std::move(index));
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
    if (baseType == *fContext.fTypes.fSkCaps) {
        if (fSettings->fReplaceSettings && !fIsBuiltinCode) {
            return this->valueForSetting(fieldNode.fOffset, field);
        }
        const Type* type = this->typeForSetting(fieldNode.fOffset, field);
        if (!type) {
            return nullptr;
        }
        return std::make_unique<Setting>(fieldNode.fOffset, field, type);
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
        this->errorReporter().error(scopeNode.fOffset, "'::' must follow a type name");
        return nullptr;
    }
    StringFragment member = scopeNode.getString();
    return this->convertTypeField(base->fOffset, base->as<TypeReference>().value(), member);
}

std::unique_ptr<Expression> IRGenerator::convertPostfixExpression(const ASTNode& expression) {
    SkASSERT(expression.fKind == ASTNode::Kind::kPostfix);
    std::unique_ptr<Expression> base = this->convertExpression(*expression.begin());
    if (!base) {
        return nullptr;
    }
    return this->convertPostfixExpression(std::move(base), expression.getOperator());
}

std::unique_ptr<Expression> IRGenerator::convertPostfixExpression(std::unique_ptr<Expression> base,
                                                                  Operator op) {
    const Type& baseType = base->type();
    if (!baseType.isNumber()) {
        this->errorReporter().error(base->fOffset,
                                    "'" + String(op.operatorName()) +
                                    "' cannot operate on '" + baseType.displayName() + "'");
        return nullptr;
    }
    if (!this->setRefKind(*base, VariableReference::RefKind::kReadWrite)) {
        return nullptr;
    }
    return std::make_unique<PostfixExpression>(std::move(base), op);
}

void IRGenerator::checkValid(const Expression& expr) {
    switch (expr.kind()) {
        case Expression::Kind::kFunctionReference:
            this->errorReporter().error(expr.fOffset, "expected '(' to begin function call");
            break;
        case Expression::Kind::kTypeReference:
            this->errorReporter().error(expr.fOffset,
                                        "expected '(' to begin constructor invocation");
            break;
        case Expression::Kind::kFunctionCall: {
            const FunctionDeclaration& decl = expr.as<FunctionCall>().function();
            if (!decl.isBuiltin() && !decl.definition()) {
                this->errorReporter().error(expr.fOffset,
                                            "function '" + decl.description() + "' is not defined");
            }
            break;
        }
        default:
            if (expr.type() == *fContext.fTypes.fInvalid) {
                this->errorReporter().error(expr.fOffset, "invalid expression");
            }
    }
}

bool IRGenerator::setRefKind(Expression& expr, VariableReference::RefKind kind) {
    Analysis::AssignmentInfo info;
    if (!Analysis::IsAssignable(expr, &info, &this->errorReporter())) {
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

        bool visitProgramElement(const ProgramElement& pe) override {
            if (pe.is<FunctionDefinition>()) {
                const FunctionDefinition& funcDef = pe.as<FunctionDefinition>();
                // We synthesize writes to sk_FragColor if main() returns a color, even if it's
                // otherwise unreferenced. Check main's return type to see if it's half4.
                if (funcDef.declaration().name() == "main" &&
                    funcDef.declaration().returnType() == *fGenerator->fContext.fTypes.fHalf4) {
                    fPreserveFragColor = true;
                }
            }
            return INHERITED::visitProgramElement(pe);
        }

        bool visitExpression(const Expression& e) override {
            if (e.is<VariableReference>() && e.as<VariableReference>().variable()->isBuiltin()) {
                this->addDeclaringElement(e.as<VariableReference>().variable()->name());
            }
            return INHERITED::visitExpression(e);
        }

        IRGenerator* fGenerator;
        std::vector<const ProgramElement*> fNewElements;
        bool fPreserveFragColor = false;

        using INHERITED = ProgramVisitor;
        using INHERITED::visitProgramElement;
    };

    BuiltinVariableScanner scanner(this);
    for (auto& e : *fProgramElements) {
        scanner.visitProgramElement(*e);
    }

    if (scanner.fPreserveFragColor) {
        // main() returns a half4, so make sure we don't dead-strip sk_FragColor.
        scanner.addDeclaringElement("sk_FragColor");
    }

    switch (fKind) {
        case ProgramKind::kFragment:
            // Vulkan requires certain builtin variables be present, even if they're unused. At one
            // time, validation errors would result if sk_Clockwise was missing. Now, it's just
            // (Adreno) driver bugs that drop or corrupt draws if they're missing.
            scanner.addDeclaringElement("sk_Clockwise");
            break;
        default:
            break;
    }

    fSharedElements->insert(
            fSharedElements->begin(), scanner.fNewElements.begin(), scanner.fNewElements.end());
}

IRGenerator::IRBundle IRGenerator::convertProgram(
        ProgramKind kind,
        const Program::Settings* settings,
        const ParsedModule& base,
        bool isBuiltinCode,
        const char* text,
        size_t length,
        const std::vector<std::unique_ptr<ExternalFunction>>* externalFunctions) {
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
    fDefinedStructs.clear();

    AutoSymbolTable table(this);

    if (kind == ProgramKind::kGeometry && !fIsBuiltinCode) {
        // Declare sk_InvocationID programmatically. With invocations support, it's an 'in' builtin.
        // If we're applying the workaround, then it's a plain global.
        bool workaround = fCaps && !fCaps->gsInvocationsSupport();
        Modifiers m;
        if (!workaround) {
            m.fFlags = Modifiers::kIn_Flag;
            m.fLayout.fBuiltin = SK_INVOCATIONID_BUILTIN;
        }
        auto var = std::make_unique<Variable>(-1, fModifiers->addToPool(m), "sk_InvocationID",
                                              fContext.fTypes.fInt.get(), false,
                                              Variable::Storage::kGlobal);
        auto decl = std::make_unique<VarDeclaration>(var.get(), fContext.fTypes.fInt.get(),
                                                     /*arraySize=*/0, /*value=*/nullptr);
        fSymbolTable->add(std::move(var));
        fProgramElements->push_back(
                std::make_unique<GlobalVarDeclaration>(/*offset=*/-1, std::move(decl)));
    }

    if (externalFunctions) {
        // Add any external values to the new symbol table, so they're only visible to this Program
        for (const auto& ef : *externalFunctions) {
            fSymbolTable->addWithoutOwnership(ef.get());
        }
    }

    Parser parser(text, length, *fSymbolTable, this->errorReporter());
    fFile = parser.compilationUnit();
    if (this->errorReporter().errorCount()) {
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
                SkDEBUGFAILF("unsupported declaration: %s\n", decl.description().c_str());
                break;
        }
    }

    // Variables defined in the pre-includes need their declaring elements added to the program
    if (!fIsBuiltinCode && fIntrinsics) {
        this->findAndDeclareBuiltinVariables();
    }

    // Do a pass looking for dangling FunctionReference or TypeReference expressions
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

    // If we're in ES2 mode (runtime effects), do a pass to enforce Appendix A, Section 5 of the
    // GLSL ES 1.00 spec -- Indexing. Don't bother if we've already found errors - this logic
    // assumes that all loops meet the criteria of Section 4, and if they don't, could crash.
    if (this->strictES2Mode() && this->errorReporter().errorCount() == 0) {
        for (const auto& pe : *fProgramElements) {
            Analysis::ValidateIndexingForES2(*pe, this->errorReporter());
        }
    }

    fSettings = nullptr;

    return IRBundle{std::move(elements), std::move(sharedElements), this->releaseModifiers(),
                    fSymbolTable, fInputs};
}

}  // namespace SkSL
