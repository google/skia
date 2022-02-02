/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLRehydrator.h"

#include <memory>
#include <unordered_set>

#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBreakStatement.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLConstructorArray.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLConstructorCompoundCast.h"
#include "src/sksl/ir/SkSLConstructorDiagonalMatrix.h"
#include "src/sksl/ir/SkSLConstructorMatrixResize.h"
#include "src/sksl/ir/SkSLConstructorScalarCast.h"
#include "src/sksl/ir/SkSLConstructorSplat.h"
#include "src/sksl/ir/SkSLConstructorStruct.h"
#include "src/sksl/ir/SkSLContinueStatement.h"
#include "src/sksl/ir/SkSLDiscardStatement.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLField.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLFunctionPrototype.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLInlineMarker.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSetting.h"
#include "src/sksl/ir/SkSLStructDefinition.h"
#include "src/sksl/ir/SkSLSwitchCase.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLUnresolvedFunction.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"

namespace SkSL {

class AutoRehydratorSymbolTable {
public:
    AutoRehydratorSymbolTable(Rehydrator* rehydrator)
        : fRehydrator(rehydrator)
        , fOldSymbols(fRehydrator->fSymbolTable) {
        std::shared_ptr<SymbolTable> symbols = fRehydrator->symbolTable();
        if (symbols) {
            fRehydrator->fSymbolTable = std::move(symbols);
        }
    }

    ~AutoRehydratorSymbolTable() {
        fRehydrator->fSymbolTable = std::move(fOldSymbols);
    }

private:
    Rehydrator* fRehydrator;
    std::shared_ptr<SymbolTable> fOldSymbols;
};

Rehydrator::Rehydrator(const Compiler& compiler, const uint8_t* src, size_t length,
        std::shared_ptr<SymbolTable> symbols)
    : fContext(compiler.fContext)
    , fSymbolTable(symbols ? std::move(symbols) : compiler.makeGLSLRootSymbolTable())
    SkDEBUGCODE(, fEnd(src + length)) {
    SkASSERT(fSymbolTable);
    SkASSERT(fSymbolTable->isBuiltin());
    fIP = src;
    uint16_t version = this->readU16();
    (void)version;
    SkASSERTF(version == kVersion, "Dehydrated file is an unsupported version (current version is "
            "%d, found version %d)", kVersion, version);
    fStringStart = fIP;
    // skip over string data
    fIP += this->readU16();
}

Layout Rehydrator::layout() {
    switch (this->readU8()) {
        case kBuiltinLayout_Command: {
            Layout result;
            result.fBuiltin = this->readS16();
            return result;
        }
        case kDefaultLayout_Command:
            return Layout();
        case kLayout_Command: {
            int flags = this->readU32();
            int location = this->readS8();
            int offset = this->readS16();
            int binding = this->readS16();
            int index = this->readS8();
            int set = this->readS8();
            int builtin = this->readS16();
            int inputAttachmentIndex = this->readS8();
            return Layout(
                    flags, location, offset, binding, index, set, builtin, inputAttachmentIndex);
        }
        default:
            SkASSERT(false);
            return Layout();
    }
}

Modifiers Rehydrator::modifiers() {
    switch (this->readU8()) {
        case kDefaultModifiers_Command:
            return Modifiers();
        case kModifiers8Bit_Command: {
            Layout l = this->layout();
            int flags = this->readU8();
            return Modifiers(l, flags);
        }
        case kModifiers_Command: {
            Layout l = this->layout();
            int flags = this->readS32();
            return Modifiers(l, flags);
        }
        default:
            SkASSERT(false);
            return Modifiers();
    }
}

const Symbol* Rehydrator::symbol() {
    int kind = this->readU8();
    switch (kind) {
        case kArrayType_Command: {
            uint16_t id = this->readU16();
            const Type* componentType = this->type();
            int8_t count = this->readS8();
            const std::string* arrayName =
                    fSymbolTable->takeOwnershipOfString(componentType->getArrayName(count));
            const Type* result = fSymbolTable->takeOwnershipOfSymbol(
                    Type::MakeArrayType(*arrayName, *componentType, count));
            this->addSymbol(id, result);
            return result;
        }
        case kFunctionDeclaration_Command: {
            uint16_t id = this->readU16();
            Modifiers modifiers = this->modifiers();
            std::string_view name = this->readString();
            int parameterCount = this->readU8();
            std::vector<const Variable*> parameters;
            parameters.reserve(parameterCount);
            for (int i = 0; i < parameterCount; ++i) {
                parameters.push_back(this->symbolRef<Variable>(Symbol::Kind::kVariable));
            }
            const Type* returnType = this->type();
            const FunctionDeclaration* result =
                    fSymbolTable->takeOwnershipOfSymbol(std::make_unique<FunctionDeclaration>(
                            /*line=*/-1,
                            this->modifiersPool().add(modifiers),
                            name,
                            std::move(parameters),
                            returnType,
                            fSymbolTable->isBuiltin()));
            this->addSymbol(id, result);
            return result;
        }
        case kField_Command: {
            const Variable* owner = this->symbolRef<Variable>(Symbol::Kind::kVariable);
            uint8_t index = this->readU8();
            const Field* result = fSymbolTable->takeOwnershipOfSymbol(
                    std::make_unique<Field>(/*line=*/-1, owner, index));
            return result;
        }
        case kStructType_Command: {
            uint16_t id = this->readU16();
            std::string name(this->readString());
            uint8_t fieldCount = this->readU8();
            std::vector<Type::Field> fields;
            fields.reserve(fieldCount);
            for (int i = 0; i < fieldCount; ++i) {
                Modifiers m = this->modifiers();
                std::string_view fieldName = this->readString();
                const Type* type = this->type();
                fields.emplace_back(m, fieldName, type);
            }
            bool interfaceBlock = this->readU8();
            std::string_view nameChars(*fSymbolTable->takeOwnershipOfString(std::move(name)));
            const Type* result = fSymbolTable->takeOwnershipOfSymbol(Type::MakeStructType(
                    /*line=*/-1, nameChars, std::move(fields), interfaceBlock));
            this->addSymbol(id, result);
            return result;
        }
        case kSymbolRef_Command: {
            uint16_t id = this->readU16();
            SkASSERT(fSymbols.size() > id);
            return fSymbols[id];
        }
        case kSystemType_Command: {
            uint16_t id = this->readU16();
            std::string_view name = this->readString();
            const Symbol* result = (*fSymbolTable)[name];
            SkASSERT(result && result->kind() == Symbol::Kind::kType);
            this->addSymbol(id, result);
            return result;
        }
        case kUnresolvedFunction_Command: {
            uint16_t id = this->readU16();
            int length = this->readU8();
            std::vector<const FunctionDeclaration*> functions;
            functions.reserve(length);
            for (int i = 0; i < length; ++i) {
                const Symbol* f = this->symbol();
                SkASSERT(f && f->kind() == Symbol::Kind::kFunctionDeclaration);
                functions.push_back((const FunctionDeclaration*) f);
            }
            const UnresolvedFunction* result = fSymbolTable->takeOwnershipOfSymbol(
                    std::make_unique<UnresolvedFunction>(std::move(functions)));
            this->addSymbol(id, result);
            return result;
        }
        case kVariable_Command: {
            uint16_t id = this->readU16();
            const Modifiers* m = this->modifiersPool().add(this->modifiers());
            std::string_view name = this->readString();
            const Type* type = this->type();
            Variable::Storage storage = (Variable::Storage) this->readU8();
            const Variable* result = fSymbolTable->takeOwnershipOfSymbol(std::make_unique<Variable>(
                    /*line=*/-1, m, name, type, fSymbolTable->isBuiltin(), storage));
            this->addSymbol(id, result);
            return result;
        }
        default:
            printf("unsupported symbol %d\n", kind);
            SkASSERT(false);
            return nullptr;
    }
}

const Type* Rehydrator::type() {
    const Symbol* result = this->symbol();
    SkASSERT(result->kind() == Symbol::Kind::kType);
    return (const Type*) result;
}

std::unique_ptr<Program> Rehydrator::program(int symbolTableCount,
        std::unique_ptr<std::string> source,
        std::unique_ptr<ProgramConfig> config,
        std::vector<const ProgramElement*> sharedElements,
        std::unique_ptr<ModifiersPool> modifiers,
        std::unique_ptr<Pool> pool,
        Program::Inputs inputs) {
    ProgramConfig* oldConfig = fContext->fConfig;
    ModifiersPool* oldModifiersPool = fContext->fModifiersPool;
    fContext->fConfig = config.get();
    fContext->fModifiersPool = modifiers.get();
    for (int i = 0; i < symbolTableCount; ++i) {
        this->symbolTable();
    }
    std::vector<std::unique_ptr<ProgramElement>> elements = this->elements();
    fContext->fConfig = oldConfig;
    fContext->fModifiersPool = oldModifiersPool;
    return std::make_unique<Program>(std::move(source), std::move(config), fContext,
            std::move(elements), std::move(sharedElements), std::move(modifiers), fSymbolTable,
            std::move(pool), inputs);
}

std::vector<std::unique_ptr<ProgramElement>> Rehydrator::elements() {
    SkDEBUGCODE(uint8_t command = )this->readU8();
    SkASSERT(command == kElements_Command);
    std::vector<std::unique_ptr<ProgramElement>> result;
    while (std::unique_ptr<ProgramElement> elem = this->element()) {
        result.push_back(std::move(elem));
    }
    return result;
}

std::unique_ptr<ProgramElement> Rehydrator::element() {
    int kind = this->readU8();
    switch (kind) {
        case Rehydrator::kFunctionDefinition_Command: {
            const FunctionDeclaration* decl = this->symbolRef<FunctionDeclaration>(
                                                                Symbol::Kind::kFunctionDeclaration);
            std::unique_ptr<Statement> body = this->statement();
            auto result = FunctionDefinition::Convert(*fContext, /*line=*/-1, *decl,
                                                      std::move(body), fSymbolTable->isBuiltin());
            decl->setDefinition(result.get());
            return std::move(result);
        }
        case Rehydrator::kFunctionPrototype_Command: {
            const FunctionDeclaration* decl = this->symbolRef<FunctionDeclaration>(
                                                                Symbol::Kind::kFunctionDeclaration);
            // since we skip over builtin prototypes when dehydrating, we know that this
            // builtin=false
            return std::make_unique<FunctionPrototype>(/*line=*/-1, decl, /*builtin=*/false);
        }
        case Rehydrator::kInterfaceBlock_Command: {
            const Symbol* var = this->symbol();
            SkASSERT(var && var->is<Variable>());
            std::string_view typeName = this->readString();
            std::string_view instanceName = this->readString();
            int arraySize = this->readS8();
            return std::make_unique<InterfaceBlock>(/*line=*/-1, var->as<Variable>(), typeName,
                                                    instanceName, arraySize, nullptr);
        }
        case Rehydrator::kVarDeclarations_Command: {
            std::unique_ptr<Statement> decl = this->statement();
            return std::make_unique<GlobalVarDeclaration>(std::move(decl));
        }
        case Rehydrator::kStructDefinition_Command: {
            const Symbol* type = this->symbol();
            SkASSERT(type && type->is<Type>());
            return std::make_unique<StructDefinition>(/*line=*/-1, type->as<Type>());
        }
        case Rehydrator::kElementsComplete_Command:
            return nullptr;
        default:
            SkDEBUGFAILF("unsupported element %d\n", kind);
            return nullptr;
    }
}

std::unique_ptr<Statement> Rehydrator::statement() {
    int kind = this->readU8();
    switch (kind) {
        case Rehydrator::kBlock_Command: {
            AutoRehydratorSymbolTable symbols(this);
            int count = this->readU8();
            StatementArray statements;
            statements.reserve_back(count);
            for (int i = 0; i < count; ++i) {
                statements.push_back(this->statement());
            }
            bool isScope = this->readU8();
            return Block::Make(/*line=*/-1, std::move(statements), fSymbolTable, isScope);
        }
        case Rehydrator::kBreak_Command:
            return BreakStatement::Make(/*line=*/-1);
        case Rehydrator::kContinue_Command:
            return ContinueStatement::Make(/*line=*/-1);
        case Rehydrator::kDiscard_Command:
            return DiscardStatement::Make(/*line=*/-1);
        case Rehydrator::kDo_Command: {
            std::unique_ptr<Statement> stmt = this->statement();
            std::unique_ptr<Expression> expr = this->expression();
            return DoStatement::Make(*fContext, std::move(stmt), std::move(expr));
        }
        case Rehydrator::kExpressionStatement_Command: {
            std::unique_ptr<Expression> expr = this->expression();
            return ExpressionStatement::Make(*fContext, std::move(expr));
        }
        case Rehydrator::kFor_Command: {
            AutoRehydratorSymbolTable symbols(this);
            std::unique_ptr<Statement> initializer = this->statement();
            std::unique_ptr<Expression> test = this->expression();
            std::unique_ptr<Expression> next = this->expression();
            std::unique_ptr<Statement> body = this->statement();
            std::unique_ptr<LoopUnrollInfo> unrollInfo =
                    Analysis::GetLoopUnrollInfo(/*line=*/-1, initializer.get(), test.get(),
                                                next.get(), body.get(), /*errors=*/nullptr);
            return ForStatement::Make(*fContext, /*line=*/-1, std::move(initializer),
                                      std::move(test), std::move(next), std::move(body),
                                      std::move(unrollInfo), fSymbolTable);
        }
        case Rehydrator::kIf_Command: {
            bool isStatic = this->readU8();
            std::unique_ptr<Expression> test = this->expression();
            std::unique_ptr<Statement> ifTrue = this->statement();
            std::unique_ptr<Statement> ifFalse = this->statement();
            return IfStatement::Make(*fContext, /*line=*/-1, isStatic, std::move(test),
                                     std::move(ifTrue), std::move(ifFalse));
        }
        case Rehydrator::kInlineMarker_Command: {
            const FunctionDeclaration* funcDecl = this->symbolRef<FunctionDeclaration>(
                                                          Symbol::Kind::kFunctionDeclaration);
            return InlineMarker::Make(funcDecl);
        }
        case Rehydrator::kNop_Command:
            return std::make_unique<SkSL::Nop>();
        case Rehydrator::kReturn_Command: {
            std::unique_ptr<Expression> expr = this->expression();
            return ReturnStatement::Make(/*line=*/-1, std::move(expr));
        }
        case Rehydrator::kSwitch_Command: {
            bool isStatic = this->readU8();
            AutoRehydratorSymbolTable symbols(this);
            std::unique_ptr<Expression> expr = this->expression();
            int caseCount = this->readU8();
            StatementArray cases;
            cases.reserve_back(caseCount);
            for (int i = 0; i < caseCount; ++i) {
                bool isDefault = this->readU8();
                if (isDefault) {
                    std::unique_ptr<Statement> statement = this->statement();
                    cases.push_back(SwitchCase::MakeDefault(/*line=*/-1, std::move(statement)));
                } else {
                    SKSL_INT value = this->readS32();
                    std::unique_ptr<Statement> statement = this->statement();
                    cases.push_back(SwitchCase::Make(/*line=*/-1, std::move(value),
                            std::move(statement)));
                }
            }
            return SwitchStatement::Make(*fContext, /*line=*/-1, isStatic, std::move(expr),
                                         std::move(cases), fSymbolTable);
        }
        case Rehydrator::kVarDeclaration_Command: {
            Variable* var = this->symbolRef<Variable>(Symbol::Kind::kVariable);
            const Type* baseType = this->type();
            int arraySize = this->readS8();
            std::unique_ptr<Expression> value = this->expression();
            return VarDeclaration::Make(*fContext, var, baseType, arraySize, std::move(value));
        }
        case Rehydrator::kVoid_Command:
            return nullptr;
        default:
            printf("unsupported statement %d\n", kind);
            SkASSERT(false);
            return nullptr;
    }
}

ExpressionArray Rehydrator::expressionArray() {
    uint8_t count = this->readU8();
    ExpressionArray array;
    array.reserve_back(count);
    for (int i = 0; i < count; ++i) {
        array.push_back(this->expression());
    }
    return array;
}

std::unique_ptr<Expression> Rehydrator::expression() {
    int kind = this->readU8();
    switch (kind) {
        case Rehydrator::kBinary_Command: {
            std::unique_ptr<Expression> left = this->expression();
            Token::Kind op = (Token::Kind) this->readU8();
            std::unique_ptr<Expression> right = this->expression();
            return BinaryExpression::Make(*fContext, std::move(left), op, std::move(right));
        }
        case Rehydrator::kBoolLiteral_Command: {
            bool value = this->readU8();
            return Literal::MakeBool(*fContext, /*line=*/-1, value);
        }
        case Rehydrator::kConstructorArray_Command: {
            const Type* type = this->type();
            return ConstructorArray::Make(*fContext, /*line=*/-1, *type, this->expressionArray());
        }
        case Rehydrator::kConstructorCompound_Command: {
            const Type* type = this->type();
            return ConstructorCompound::Make(*fContext, /*line=*/-1, *type,
                                              this->expressionArray());
        }
        case Rehydrator::kConstructorDiagonalMatrix_Command: {
            const Type* type = this->type();
            ExpressionArray args = this->expressionArray();
            SkASSERT(args.size() == 1);
            return ConstructorDiagonalMatrix::Make(*fContext, /*line=*/-1, *type,
                                                   std::move(args[0]));
        }
        case Rehydrator::kConstructorMatrixResize_Command: {
            const Type* type = this->type();
            ExpressionArray args = this->expressionArray();
            SkASSERT(args.size() == 1);
            return ConstructorMatrixResize::Make(*fContext, /*line=*/-1, *type,
                                                 std::move(args[0]));
        }
        case Rehydrator::kConstructorScalarCast_Command: {
            const Type* type = this->type();
            ExpressionArray args = this->expressionArray();
            SkASSERT(args.size() == 1);
            return ConstructorScalarCast::Make(*fContext, /*line=*/-1, *type, std::move(args[0]));
        }
        case Rehydrator::kConstructorSplat_Command: {
            const Type* type = this->type();
            ExpressionArray args = this->expressionArray();
            SkASSERT(args.size() == 1);
            return ConstructorSplat::Make(*fContext, /*line=*/-1, *type, std::move(args[0]));
        }
        case Rehydrator::kConstructorStruct_Command: {
            const Type* type = this->type();
            return ConstructorStruct::Make(*fContext, /*line=*/-1, *type, this->expressionArray());
        }
        case Rehydrator::kConstructorCompoundCast_Command: {
            const Type* type = this->type();
            ExpressionArray args = this->expressionArray();
            SkASSERT(args.size() == 1);
            return ConstructorCompoundCast::Make(*fContext,/*line=*/-1, *type, std::move(args[0]));
        }
        case Rehydrator::kFieldAccess_Command: {
            std::unique_ptr<Expression> base = this->expression();
            int index = this->readU8();
            FieldAccess::OwnerKind ownerKind = (FieldAccess::OwnerKind) this->readU8();
            return FieldAccess::Make(*fContext, std::move(base), index, ownerKind);
        }
        case Rehydrator::kFloatLiteral_Command: {
            const Type* type = this->type();
            int32_t floatBits = this->readS32();
            float value;
            memcpy(&value, &floatBits, sizeof(value));
            return Literal::MakeFloat(/*line=*/-1, value, type);
        }
        case Rehydrator::kFunctionCall_Command: {
            const Type* type = this->type();
            const FunctionDeclaration* f = this->symbolRef<FunctionDeclaration>(
                                                                Symbol::Kind::kFunctionDeclaration);
            ExpressionArray args = this->expressionArray();
            return FunctionCall::Make(*fContext, /*line=*/-1, type, *f, std::move(args));
        }
        case Rehydrator::kIndex_Command: {
            std::unique_ptr<Expression> base = this->expression();
            std::unique_ptr<Expression> index = this->expression();
            return IndexExpression::Make(*fContext, std::move(base), std::move(index));
        }
        case Rehydrator::kIntLiteral_Command: {
            const Type* type = this->type();
            int value = this->readS32();
            return Literal::MakeInt(/*line=*/-1, value, type);
        }
        case Rehydrator::kPostfix_Command: {
            Token::Kind op = (Token::Kind) this->readU8();
            std::unique_ptr<Expression> operand = this->expression();
            return PostfixExpression::Make(*fContext, std::move(operand), op);
        }
        case Rehydrator::kPrefix_Command: {
            Token::Kind op = (Token::Kind) this->readU8();
            std::unique_ptr<Expression> operand = this->expression();
            return PrefixExpression::Make(*fContext, op, std::move(operand));
        }
        case Rehydrator::kSetting_Command: {
            std::string name(this->readString());
            return Setting::Convert(*fContext, /*line=*/-1, name);
        }
        case Rehydrator::kSwizzle_Command: {
            std::unique_ptr<Expression> base = this->expression();
            int count = this->readU8();
            ComponentArray components;
            for (int i = 0; i < count; ++i) {
                components.push_back(this->readU8());
            }
            return Swizzle::Make(*fContext, std::move(base), components);
        }
        case Rehydrator::kTernary_Command: {
            std::unique_ptr<Expression> test = this->expression();
            std::unique_ptr<Expression> ifTrue = this->expression();
            std::unique_ptr<Expression> ifFalse = this->expression();
            return TernaryExpression::Make(*fContext, std::move(test),
                                           std::move(ifTrue), std::move(ifFalse));
        }
        case Rehydrator::kVariableReference_Command: {
            const Variable* var = this->symbolRef<Variable>(Symbol::Kind::kVariable);
            VariableReference::RefKind refKind = (VariableReference::RefKind) this->readU8();
            return VariableReference::Make(/*line=*/-1, var, refKind);
        }
        case Rehydrator::kVoid_Command:
            return nullptr;
        default:
            printf("unsupported expression %d\n", kind);
            SkASSERT(false);
            return nullptr;
    }
}

std::shared_ptr<SymbolTable> Rehydrator::symbolTable() {
    int command = this->readU8();
    if (command == kVoid_Command) {
        return nullptr;
    }
    SkASSERT(command == kSymbolTable_Command);
    bool builtin = this->readU8();
    uint16_t ownedCount = this->readU16();
    fSymbolTable = std::make_shared<SymbolTable>(std::move(fSymbolTable), builtin);
    std::vector<const Symbol*> ownedSymbols;
    ownedSymbols.reserve(ownedCount);
    for (int i = 0; i < ownedCount; ++i) {
        ownedSymbols.push_back(this->symbol());
    }
    uint16_t symbolCount = this->readU16();
    std::vector<std::pair<std::string_view, int>> symbols;
    symbols.reserve(symbolCount);
    for (int i = 0; i < symbolCount; ++i) {
        int index = this->readU16();
        if (index != kBuiltinType_Symbol) {
            fSymbolTable->addWithoutOwnership(ownedSymbols[index]);
        } else {
            std::string_view name = this->readString();
            SymbolTable* root = fSymbolTable.get();
            while (root->fParent) {
                root = root->fParent.get();
            }
            const Symbol* s = (*root)[name];
            SkASSERT(s);
            fSymbolTable->addWithoutOwnership(s);
        }
    }
    return fSymbolTable;
}

}  // namespace SkSL
