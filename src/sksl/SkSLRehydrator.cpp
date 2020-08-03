/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLRehydrator.h"

#include <memory>
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBreakStatement.h"
#include "src/sksl/ir/SkSLContinueStatement.h"
#include "src/sksl/ir/SkSLDiscardStatement.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLEnum.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLField.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLFloatLiteral.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLIntLiteral.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLModifiers.h"
#include "src/sksl/ir/SkSLNullLiteral.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSetting.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSwitchCase.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLUnresolvedFunction.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVarDeclarationsStatement.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLWhileStatement.h"

namespace SkSL {

class AutoRehydratorSymbolTable {
public:
    AutoRehydratorSymbolTable(Rehydrator* rehydrator)
        : fRehydrator(rehydrator)
        , fOldSymbols(fRehydrator->fSymbolTable) {
        fRehydrator->fSymbolTable = fRehydrator->symbolTable();
    }

    ~AutoRehydratorSymbolTable() {
        fRehydrator->fSymbolTable = std::move(fOldSymbols);
    }

private:
    Rehydrator* fRehydrator;
    std::shared_ptr<SymbolTable> fOldSymbols;
};

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
            int offset = this->readS8();
            int binding = this->readS8();
            int index = this->readS8();
            int set = this->readS8();
            int builtin = this->readS16();
            int inputAttachmentIndex = this->readS8();
            int format = this->readS8();
            int primitive = this->readS8();
            int maxVertices = this->readS8();
            int invocations = this->readS8();
            StringFragment marker = this->readString();
            StringFragment when = this->readString();
            int key = this->readS8();
            int ctype = this->readS8();
            return Layout(flags, location, offset, binding, index, set, builtin,
                          inputAttachmentIndex, (Layout::Format) format,
                          (Layout::Primitive) primitive, maxVertices, invocations, marker, when,
                          (Layout::Key) key, (Layout::CType) ctype);
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
            uint8_t count = this->readU8();
            Type* result = new Type(componentType->name() + "[" + to_string(count) + "]",
                                    Type::kArray_Kind, *componentType, count);
            fSymbolTable->takeOwnership(std::unique_ptr<const Symbol>(result));
            this->addSymbol(id, result);
            return result;
        }
        case kEnumType_Command: {
            uint16_t id = this->readU16();
            StringFragment name = this->readString();
            Type* result = new Type(name, Type::kEnum_Kind);
            fSymbolTable->takeOwnership(std::unique_ptr<const Symbol>(result));
            this->addSymbol(id, result);
            return result;
        }
        case kFunctionDeclaration_Command: {
            uint16_t id = this->readU16();
            Modifiers modifiers = this->modifiers();
            StringFragment name = this->readString();
            int parameterCount = this->readU8();
            std::vector<const Variable*> parameters;
            parameters.reserve(parameterCount);
            for (int i = 0; i < parameterCount; ++i) {
                parameters.push_back(this->symbolRef<Variable>(Symbol::kVariable_Kind));
            }
            const Type* returnType = this->type();
            FunctionDeclaration* result =  new FunctionDeclaration(-1, modifiers, name,
                                                                   std::move(parameters),
                                                                   *returnType, true);
            fSymbolTable->takeOwnership(std::unique_ptr<const Symbol>(result));
            this->addSymbol(id, result);
            return result;
        }
        case kField_Command: {
            const Variable* owner = this->symbolRef<Variable>(Symbol::kVariable_Kind);
            uint8_t index = this->readU8();
            Field* result = new Field(-1, *owner, index);
            fSymbolTable->takeOwnership(std::unique_ptr<const Symbol>(result));
            return result;
        }
        case kNullableType_Command: {
            uint16_t id = this->readU16();
            const Type* base = this->type();
            Type* result = new Type(base->name() + "?", Type::kNullable_Kind, *base);
            fSymbolTable->takeOwnership(std::unique_ptr<const Symbol>(result));
            this->addSymbol(id, result);
            return result;
        }
        case kStructType_Command: {
            uint16_t id = this->readU16();
            StringFragment name = this->readString();
            uint8_t fieldCount = this->readU8();
            std::vector<Type::Field> fields;
            fields.reserve(fieldCount);
            for (int i = 0; i < fieldCount; ++i) {
                Modifiers m = this->modifiers();
                StringFragment name = this->readString();
                const Type* type = this->type();
                fields.emplace_back(m, name, type);
            }
            Type* result = new Type(-1, name, std::move(fields));
            fSymbolTable->takeOwnership(std::unique_ptr<const Symbol>(result));
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
            StringFragment name = this->readString();
            const Symbol* result = (*fSymbolTable)[name];
            SkASSERT(result && result->fKind == Symbol::kType_Kind);
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
                SkASSERT(f && f->fKind == Symbol::kFunctionDeclaration_Kind);
                functions.push_back((const FunctionDeclaration*) f);
            }
            UnresolvedFunction* result = new UnresolvedFunction(std::move(functions));
            fSymbolTable->takeOwnership(std::unique_ptr<const Symbol>(result));
            this->addSymbol(id, result);
            return result;
        }
        case kVariable_Command: {
            uint16_t id = this->readU16();
            Modifiers m = this->modifiers();
            StringFragment name = this->readString();
            const Type* type = this->type();
            Variable::Storage storage = (Variable::Storage) this->readU8();
            Variable* result = new Variable(-1, m, name, *type, storage);
            fSymbolTable->takeOwnership(std::unique_ptr<const Symbol>(result));
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
    SkASSERT(result->fKind == Symbol::kType_Kind);
    return (const Type*) result;
}

std::vector<std::unique_ptr<ProgramElement>> Rehydrator::elements() {
    SkDEBUGCODE(uint8_t command = )this->readU8();
    SkASSERT(command == kElements_Command);
    uint8_t count = this->readU8();
    std::vector<std::unique_ptr<ProgramElement>> result;
    result.reserve(count);
    for (int i = 0; i < count; ++i) {
        result.push_back(this->element());
    }
    return result;
}

std::unique_ptr<ProgramElement> Rehydrator::element() {
    int kind = this->readU8();
    switch (kind) {
        case Rehydrator::kEnum_Command: {
            StringFragment typeName = this->readString();
            std::shared_ptr<SymbolTable> symbols = this->symbolTable();
            for (auto& s : symbols->fOwnedSymbols) {
                SkASSERT(s->fKind == Symbol::kVariable_Kind);
                Variable& v = (Variable&) *s;
                int value = this->readS32();
                v.fInitialValue = (Expression*) symbols->takeOwnership(std::unique_ptr<IRNode>(
                                                    new IntLiteral(fContext, -1, value)));
                v.fWriteCount = 1;
            }
            return std::unique_ptr<ProgramElement>(new Enum(-1, typeName, std::move(symbols)));
        }
        case Rehydrator::kFunctionDefinition_Command: {
            const FunctionDeclaration* decl = this->symbolRef<FunctionDeclaration>(
                                                                 Symbol::kFunctionDeclaration_Kind);
            std::unique_ptr<Statement> body = this->statement();
            std::set<const FunctionDeclaration*> refs;
            uint8_t refCount = this->readU8();
            for (int i = 0; i < refCount; ++i) {
                refs.insert(this->symbolRef<FunctionDeclaration>(
                                                                Symbol::kFunctionDeclaration_Kind));
            }
            FunctionDefinition* result = new FunctionDefinition(-1, *decl, std::move(body),
                                                                std::move(refs));
            decl->fDefinition = result;
            return std::unique_ptr<ProgramElement>(result);
        }
        case Rehydrator::kInterfaceBlock_Command: {
            const Symbol* var = this->symbol();
            SkASSERT(var && var->fKind == Symbol::kVariable_Kind);
            StringFragment typeName = this->readString();
            StringFragment instanceName = this->readString();
            uint8_t sizeCount = this->readU8();
            std::vector<std::unique_ptr<Expression>> sizes;
            sizes.reserve(sizeCount);
            for (int i = 0; i < sizeCount; ++i) {
                sizes.push_back(this->expression());
            }
            return std::unique_ptr<ProgramElement>(new InterfaceBlock(-1, (Variable*) var, typeName,
                                                                      instanceName,
                                                                      std::move(sizes), nullptr));
        }
        case Rehydrator::kVarDeclarations_Command: {
            const Type* baseType = this->type();
            int count = this->readU8();
            std::vector<std::unique_ptr<VarDeclaration>> vars;
            vars.reserve(count);
            for (int i = 0 ; i < count; ++i) {
                std::unique_ptr<Statement> s = this->statement();
                SkASSERT(s->fKind == Statement::kVarDeclaration_Kind);
                vars.emplace_back((VarDeclaration*) s.release());
            }
            return std::unique_ptr<ProgramElement>(new VarDeclarations(-1, baseType,
                                                                       std::move(vars)));
        }
        default:
            printf("unsupported element %d\n", kind);
            SkASSERT(false);
            return nullptr;
    }
}

std::unique_ptr<Statement> Rehydrator::statement() {
    int kind = this->readU8();
    switch (kind) {
        case Rehydrator::kBlock_Command: {
            AutoRehydratorSymbolTable symbols(this);
            int count = this->readU8();
            std::vector<std::unique_ptr<Statement>> statements;
            statements.reserve(count);
            for (int i = 0; i < count; ++i) {
                statements.push_back(this->statement());
            }
            bool isScope = this->readU8();
            return std::unique_ptr<Statement>(new Block(-1, std::move(statements), fSymbolTable,
                                                        isScope));
        }
        case Rehydrator::kBreak_Command:
            return std::unique_ptr<Statement>(new BreakStatement(-1));
        case Rehydrator::kContinue_Command:
            return std::unique_ptr<Statement>(new ContinueStatement(-1));
        case Rehydrator::kDiscard_Command:
            return std::unique_ptr<Statement>(new DiscardStatement(-1));
        case Rehydrator::kDo_Command: {
            std::unique_ptr<Statement> stmt = this->statement();
            std::unique_ptr<Expression> expr = this->expression();
            return std::unique_ptr<Statement>(new DoStatement(-1, std::move(stmt),
                                                              std::move(expr)));
        }
        case Rehydrator::kExpressionStatement_Command: {
            std::unique_ptr<Expression> expr = this->expression();
            return std::unique_ptr<Statement>(new ExpressionStatement(std::move(expr)));
        }
        case Rehydrator::kFor_Command: {
            std::unique_ptr<Statement> initializer = this->statement();
            std::unique_ptr<Expression> test = this->expression();
            std::unique_ptr<Expression> next = this->expression();
            std::unique_ptr<Statement> body = this->statement();
            std::shared_ptr<SymbolTable> symbols = this->symbolTable();
            return std::unique_ptr<Statement>(new ForStatement(-1, std::move(initializer),
                                                               std::move(test), std::move(next),
                                                               std::move(body),
                                                               std::move(symbols)));
        }
        case Rehydrator::kIf_Command: {
            bool isStatic = this->readU8();
            std::unique_ptr<Expression> test = this->expression();
            std::unique_ptr<Statement> ifTrue = this->statement();
            std::unique_ptr<Statement> ifFalse = this->statement();
            return std::unique_ptr<Statement>(new IfStatement(-1, isStatic, std::move(test),
                                                              std::move(ifTrue),
                                                              std::move(ifFalse)));
        }
        case Rehydrator::kReturn_Command: {
            std::unique_ptr<Expression> expr = this->expression();
            if (expr) {
                return std::unique_ptr<Statement>(new ReturnStatement(std::move(expr)));
            } else {
                return std::unique_ptr<Statement>(new ReturnStatement(-1));
            }
        }
        case Rehydrator::kSwitch_Command: {
            bool isStatic = this->readU8();
            AutoRehydratorSymbolTable symbols(this);
            std::unique_ptr<Expression> expr = this->expression();
            int caseCount = this->readU8();
            std::vector<std::unique_ptr<SwitchCase>> cases;
            cases.reserve(caseCount);
            for (int i = 0; i < caseCount; ++i) {
                std::unique_ptr<Expression> value = this->expression();
                int statementCount = this->readU8();
                std::vector<std::unique_ptr<Statement>> statements;
                statements.reserve(statementCount);
                for (int j = 0; j < statementCount; ++j) {
                    statements.push_back(this->statement());
                }
                cases.emplace_back(new SwitchCase(-1, std::move(value), std::move(statements)));
            }
            return std::unique_ptr<Statement>(new SwitchStatement(-1, isStatic, std::move(expr),
                                                                  std::move(cases),
                                                                  fSymbolTable));
        }
        case Rehydrator::kVarDeclaration_Command: {
            Variable* var = this->symbolRef<Variable>(Symbol::kVariable_Kind);
            uint8_t sizeCount = this->readU8();
            std::vector<std::unique_ptr<Expression>> sizes;
            sizes.reserve(sizeCount);
            for (int i = 0; i < sizeCount; ++i) {
                sizes.push_back(this->expression());
            }
            std::unique_ptr<Expression> value = this->expression();
            if (value) {
                var->fInitialValue = value.get();
                SkASSERT(var->fWriteCount == 0);
                ++var->fWriteCount;
            }
            return std::unique_ptr<Statement>(new VarDeclaration(var,
                                                                 std::move(sizes),
                                                                 std::move(value)));
        }
        case Rehydrator::kVarDeclarations_Command: {
            const Type* baseType = this->type();
            int count = this->readU8();
            std::vector<std::unique_ptr<VarDeclaration>> vars;
            vars.reserve(count);
            for (int i = 0 ; i < count; ++i) {
                std::unique_ptr<Statement> s = this->statement();
                SkASSERT(s->fKind == Statement::kVarDeclaration_Kind);
                vars.emplace_back((VarDeclaration*) s.release());
            }
            return std::make_unique<VarDeclarationsStatement>(
                    std::make_unique<VarDeclarations>(-1, baseType, std::move(vars)));
        }
        case Rehydrator::kVoid_Command:
            return nullptr;
        case Rehydrator::kWhile_Command: {
            std::unique_ptr<Expression> expr = this->expression();
            std::unique_ptr<Statement> stmt = this->statement();
            return std::unique_ptr<Statement>(new WhileStatement(-1, std::move(expr),
                                                                 std::move(stmt)));
        }
        default:
            printf("unsupported statement %d\n", kind);
            SkASSERT(false);
            return nullptr;
    }
}

std::unique_ptr<Expression> Rehydrator::expression() {
    int kind = this->readU8();
    switch (kind) {
        case Rehydrator::kBinary_Command: {
            std::unique_ptr<Expression> left = this->expression();
            Token::Kind op = (Token::Kind) this->readU8();
            std::unique_ptr<Expression> right = this->expression();
            const Type* type = this->type();
            return std::unique_ptr<Expression>(new BinaryExpression(-1, std::move(left), op,
                                                                    std::move(right), *type));
        }
        case Rehydrator::kBoolLiteral_Command: {
            bool value = this->readU8();
            return std::unique_ptr<Expression>(new BoolLiteral(fContext, -1, value));
        }
        case Rehydrator::kConstructor_Command: {
            const Type* type = this->type();
            uint8_t argCount = this->readU8();
            std::vector<std::unique_ptr<Expression>> args;
            args.reserve(argCount);
            for (int i = 0; i < argCount; ++i) {
                args.push_back(this->expression());
            }
            return std::unique_ptr<Expression>(new Constructor(-1, *type, std::move(args)));
        }
        case Rehydrator::kFieldAccess_Command: {
            std::unique_ptr<Expression> base = this->expression();
            int index = this->readU8();
            FieldAccess::OwnerKind ownerKind = (FieldAccess::OwnerKind) this->readU8();
            return std::unique_ptr<Expression>(new FieldAccess(std::move(base), index, ownerKind));
        }
        case Rehydrator::kFloatLiteral_Command: {
            FloatIntUnion u;
            u.fInt = this->readS32();
            return std::unique_ptr<Expression>(new FloatLiteral(fContext, -1, u.fFloat));
        }
        case Rehydrator::kFunctionCall_Command: {
            const Type* type = this->type();
            const FunctionDeclaration* f = this->symbolRef<FunctionDeclaration>(
                                                                 Symbol::kFunctionDeclaration_Kind);
            uint8_t argCount = this->readU8();
            std::vector<std::unique_ptr<Expression>> args;
            args.reserve(argCount);
            for (int i = 0; i < argCount; ++i) {
                args.push_back(this->expression());
            }
            return std::unique_ptr<Expression>(new FunctionCall(-1, *type, *f, std::move(args)));
        }
        case Rehydrator::kIndex_Command: {
            std::unique_ptr<Expression> base = this->expression();
            std::unique_ptr<Expression> index = this->expression();
            return std::unique_ptr<Expression>(new IndexExpression(fContext, std::move(base),
                                                                   std::move(index)));
        }
        case Rehydrator::kIntLiteral_Command: {
            int value = this->readS32();
            return std::unique_ptr<Expression>(new IntLiteral(fContext, -1, value));
        }
        case Rehydrator::kNullLiteral_Command:
            return std::unique_ptr<Expression>(new NullLiteral(fContext, -1));
        case Rehydrator::kPostfix_Command: {
            Token::Kind op = (Token::Kind) this->readU8();
            std::unique_ptr<Expression> operand = this->expression();
            return std::unique_ptr<Expression>(new PostfixExpression(std::move(operand), op));
        }
        case Rehydrator::kPrefix_Command: {
            Token::Kind op = (Token::Kind) this->readU8();
            std::unique_ptr<Expression> operand = this->expression();
            return std::unique_ptr<Expression>(new PrefixExpression(op, std::move(operand)));
        }
        case Rehydrator::kSetting_Command: {
            StringFragment name = this->readString();
            std::unique_ptr<Expression> value = this->expression();
            return std::unique_ptr<Expression>(new Setting(-1, name, std::move(value)));
        }
        case Rehydrator::kSwizzle_Command: {
            std::unique_ptr<Expression> base = this->expression();
            int count = this->readU8();
            std::vector<int> components;
            components.reserve(count);
            for (int i = 0; i < count; ++i) {
                components.push_back(this->readU8());
            }
            return std::unique_ptr<Expression>(new Swizzle(fContext, std::move(base),
                                                           std::move(components)));
        }
        case Rehydrator::kTernary_Command: {
            std::unique_ptr<Expression> test = this->expression();
            std::unique_ptr<Expression> ifTrue = this->expression();
            std::unique_ptr<Expression> ifFalse = this->expression();
            return std::unique_ptr<Expression>(new TernaryExpression(-1, std::move(test),
                                                                     std::move(ifFalse),
                                                                     std::move(ifTrue)));
        }
        case Rehydrator::kVariableReference_Command: {
            const Variable* var = this->symbolRef<Variable>(Symbol::kVariable_Kind);
            VariableReference::RefKind refKind = (VariableReference::RefKind) this->readU8();
            return std::unique_ptr<Expression>(new VariableReference(-1, *var, refKind));
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
    uint16_t ownedCount = this->readU16();
    std::shared_ptr<SymbolTable> result(new SymbolTable(fSymbolTable));
    fSymbolTable = result;
    std::vector<const Symbol*> ownedSymbols;
    ownedSymbols.reserve(ownedCount);
    for (int i = 0; i < ownedCount; ++i) {
        ownedSymbols.push_back(this->symbol());
    }
    uint16_t symbolCount = this->readU16();
    std::vector<std::pair<StringFragment, int>> symbols;
    symbols.reserve(symbolCount);
    for (int i = 0; i < symbolCount; ++i) {
        StringFragment name = this->readString();
        int index = this->readU16();
        fSymbolTable->addWithoutOwnership(name, ownedSymbols[index]);
    }
    fSymbolTable = fSymbolTable->fParent;
    return result;
}

} // namespace
