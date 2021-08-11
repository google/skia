/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_IRGENERATOR
#define SKSL_IRGENERATOR

#include <unordered_map>
#include <unordered_set>

#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLStatement.h"
#include "src/sksl/SkSLASTFile.h"
#include "src/sksl/SkSLASTNode.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLOperators.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExtension.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLModifiersDeclaration.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLTypeReference.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

namespace dsl {
    class DSLCore;
    class DSLFunction;
    class DSLVar;
    class DSLWriter;
}

class ExternalFunction;
class FunctionCall;
class StructDefinition;
struct ParsedModule;
struct Swizzle;

/**
 * Intrinsics are passed between the Compiler and the IRGenerator using IRIntrinsicMaps.
 */
class IRIntrinsicMap {
public:
    IRIntrinsicMap(IRIntrinsicMap* parent) : fParent(parent) {}

    void insertOrDie(String key, std::unique_ptr<ProgramElement> element) {
        SkASSERT(fIntrinsics.find(key) == fIntrinsics.end());
        fIntrinsics[key] = Intrinsic{std::move(element), false};
    }

    const ProgramElement* find(const String& key) {
        auto iter = fIntrinsics.find(key);
        if (iter == fIntrinsics.end()) {
            return fParent ? fParent->find(key) : nullptr;
        }
        return iter->second.fIntrinsic.get();
    }

    // Only returns an intrinsic that isn't already marked as included, and then marks it.
    const ProgramElement* findAndInclude(const String& key) {
        auto iter = fIntrinsics.find(key);
        if (iter == fIntrinsics.end()) {
            return fParent ? fParent->findAndInclude(key) : nullptr;
        }
        if (iter->second.fAlreadyIncluded) {
            return nullptr;
        }
        iter->second.fAlreadyIncluded = true;
        return iter->second.fIntrinsic.get();
    }

    void resetAlreadyIncluded() {
        for (auto& pair : fIntrinsics) {
            pair.second.fAlreadyIncluded = false;
        }
        if (fParent) {
            fParent->resetAlreadyIncluded();
        }
    }

private:
    struct Intrinsic {
        std::unique_ptr<ProgramElement> fIntrinsic;
        bool fAlreadyIncluded = false;
    };

    std::unordered_map<String, Intrinsic> fIntrinsics;
    IRIntrinsicMap* fParent = nullptr;
};

/**
 * Performs semantic analysis on an abstract syntax tree (AST) and produces the corresponding
 * (unoptimized) intermediate representation (IR).
 */
class IRGenerator {
public:
    IRGenerator(const Context* context);

    struct IRBundle {
        std::vector<std::unique_ptr<ProgramElement>> fElements;
        std::vector<const ProgramElement*>           fSharedElements;
        std::shared_ptr<SymbolTable>                 fSymbolTable;
        Program::Inputs                              fInputs;
    };

    /**
     * If externalFunctions is supplied, those values are registered in the symbol table of the
     * Program, but ownership is *not* transferred. It is up to the caller to keep them alive.
     */
    IRBundle convertProgram(
            const ParsedModule& base,
            bool isBuiltinCode,
            skstd::string_view text);

    const Program::Settings& settings() const { return fContext.fConfig->fSettings; }
    ProgramKind programKind() const { return fContext.fConfig->fKind; }

    ErrorReporter& errorReporter() const { return fContext.errors(); }

    std::shared_ptr<SymbolTable>& symbolTable() {
        return fSymbolTable;
    }

    void setSymbolTable(std::shared_ptr<SymbolTable>& symbolTable) {
        fSymbolTable = symbolTable;
    }

    void pushSymbolTable();
    void popSymbolTable();

    static void CheckModifiers(const Context& context,
                               int offset,
                               const Modifiers& modifiers,
                               int permittedModifierFlags,
                               int permittedLayoutFlags);

    std::unique_ptr<Expression> convertIdentifier(int offset, skstd::string_view identifier);

    const Context& fContext;

private:
    void start(const ParsedModule& base,
               bool isBuiltinCode,
               std::vector<std::unique_ptr<ProgramElement>>* elements,
               std::vector<const ProgramElement*>* sharedElements);

    IRGenerator::IRBundle finish();

    void checkVarDeclaration(int offset,
                             const Modifiers& modifiers,
                             const Type* baseType,
                             Variable::Storage storage);
    std::unique_ptr<Variable> convertVar(int offset, const Modifiers& modifiers,
                                         const Type* baseType, skstd::string_view name,
                                         bool isArray, std::unique_ptr<Expression> arraySize,
                                         Variable::Storage storage);
    std::unique_ptr<Statement> convertVarDeclaration(std::unique_ptr<Variable> var,
                                                     std::unique_ptr<Expression> value,
                                                     bool addToSymbolTable = true);
    std::unique_ptr<Statement> convertVarDeclaration(int offset, const Modifiers& modifiers,
                                                     const Type* baseType, skstd::string_view name,
                                                     bool isArray,
                                                     std::unique_ptr<Expression> arraySize,
                                                     std::unique_ptr<Expression> value,
                                                     Variable::Storage storage);
    StatementArray convertVarDeclarations(const ASTNode& decl, Variable::Storage storage);
    void convertFunction(const ASTNode& f);
    std::unique_ptr<Statement> convertStatement(const ASTNode& statement);
    std::unique_ptr<Expression> convertExpression(const ASTNode& expression);
    std::unique_ptr<ModifiersDeclaration> convertModifiersDeclaration(const ASTNode& m);

    const Type* convertType(const ASTNode& type, bool allowVoid = false);
    std::unique_ptr<Expression> call(int offset,
                                     std::unique_ptr<Expression> function,
                                     ExpressionArray arguments);
    std::unique_ptr<Expression> call(int offset,
                                     const FunctionDeclaration& function,
                                     ExpressionArray arguments);
    CoercionCost callCost(const FunctionDeclaration& function,
                          const ExpressionArray& arguments);
    std::unique_ptr<Expression> coerce(std::unique_ptr<Expression> expr, const Type& type);
    CoercionCost coercionCost(const Expression& expr, const Type& type);
    int convertArraySize(const Type& type, int offset, const ASTNode& s);
    int convertArraySize(const Type& type, std::unique_ptr<Expression> s);
    bool containsConstantZero(Expression& expr);
    bool dividesByZero(Operator op, Expression& right);
    std::unique_ptr<Block> convertBlock(const ASTNode& block);
    std::unique_ptr<Statement> convertBreak(const ASTNode& b);
    std::unique_ptr<Statement> convertContinue(const ASTNode& c);
    std::unique_ptr<Statement> convertDiscard(const ASTNode& d);
    std::unique_ptr<Statement> convertDo(const ASTNode& d);
    std::unique_ptr<Statement> convertSwitch(const ASTNode& s);
    std::unique_ptr<Expression> convertBinaryExpression(const ASTNode& expression);
    std::unique_ptr<Extension> convertExtension(int offset, skstd::string_view name);
    std::unique_ptr<Statement> convertExpressionStatement(const ASTNode& s);
    std::unique_ptr<Expression> convertField(std::unique_ptr<Expression> base,
                                             skstd::string_view field);
    std::unique_ptr<Statement> convertFor(const ASTNode& f);
    std::unique_ptr<Expression> convertIdentifier(const ASTNode& identifier);
    std::unique_ptr<Statement> convertIf(const ASTNode& s);
    std::unique_ptr<InterfaceBlock> convertInterfaceBlock(const ASTNode& s);
    Modifiers convertModifiers(const Modifiers& m);
    std::unique_ptr<Expression> convertPrefixExpression(const ASTNode& expression);
    std::unique_ptr<Statement> convertReturn(int offset, std::unique_ptr<Expression> result);
    std::unique_ptr<Statement> convertReturn(const ASTNode& r);
    std::unique_ptr<Expression> convertCallExpression(const ASTNode& expression);
    std::unique_ptr<Expression> convertFieldExpression(const ASTNode& expression);
    std::unique_ptr<Expression> convertIndexExpression(const ASTNode& expression);
    std::unique_ptr<Expression> convertPostfixExpression(const ASTNode& expression);
    std::unique_ptr<StructDefinition> convertStructDefinition(const ASTNode& expression);
    std::unique_ptr<Expression> convertSwizzle(std::unique_ptr<Expression> base,
                                               skstd::string_view fields);
    std::unique_ptr<Expression> convertTernaryExpression(const ASTNode& expression);
    std::unique_ptr<Statement> convertVarDeclarationStatement(const ASTNode& s);
    std::unique_ptr<Statement> convertWhile(const ASTNode& w);
    void convertGlobalVarDeclarations(const ASTNode& decl);
    std::unique_ptr<Block> applyInvocationIDWorkaround(std::unique_ptr<Block> main);
    // returns a statement which converts sk_Position from device to normalized coordinates
    std::unique_ptr<Statement> getNormalizeSkPositionCode();

    void checkValid(const Expression& expr);
    bool typeContainsPrivateFields(const Type& type);
    bool setRefKind(Expression& expr, VariableReference::RefKind kind);
    void copyIntrinsicIfNeeded(const FunctionDeclaration& function);
    void findAndDeclareBuiltinVariables();
    bool detectVarDeclarationWithoutScope(const Statement& stmt);
    // Coerces returns to correct type, detects invalid break / continue placement, and otherwise
    // massages the function into its final form
    std::unique_ptr<Block> finalizeFunction(const FunctionDeclaration& funcDecl,
                                            std::unique_ptr<Block> body);

    // Runtime effects (and the interpreter, which uses the same CPU runtime) require adherence to
    // the strict rules from The OpenGL ES Shading Language Version 1.00. (Including Appendix A).
    bool strictES2Mode() const {
        return fContext.fConfig->strictES2Mode();
    }

    bool isRuntimeEffect() const {
        return ProgramConfig::IsRuntimeEffect(fContext.fConfig->fKind);
    }

    const ShaderCapsClass& caps() const {
        return fContext.fCaps;
    }

    ModifiersPool& modifiersPool() const {
        return *fContext.fModifiersPool;
    }

    Program::Inputs fInputs;

    std::unique_ptr<ASTFile> fFile;

    std::shared_ptr<SymbolTable> fSymbolTable = nullptr;
    // Symbols which have definitions in the include files.
    IRIntrinsicMap* fIntrinsics = nullptr;
    std::unordered_set<const FunctionDeclaration*> fReferencedIntrinsics;
    int fInvocations;
    std::unordered_set<const Type*> fDefinedStructs;
    std::vector<std::unique_ptr<ProgramElement>>* fProgramElements = nullptr;
    std::vector<const ProgramElement*>*           fSharedElements = nullptr;
    const Variable* fRTAdjust = nullptr;
    const Variable* fRTAdjustInterfaceBlock = nullptr;
    int fRTAdjustFieldIndex;
    // true if we are currently processing one of the built-in SkSL include files
    bool fIsBuiltinCode = false;

    friend class AutoSymbolTable;
    friend class AutoLoopLevel;
    friend class AutoSwitchLevel;
    friend class AutoDisableInline;
    friend class Compiler;
    friend class DSLParser;
    friend class dsl::DSLCore;
    friend class dsl::DSLFunction;
    friend class dsl::DSLVar;
    friend class dsl::DSLWriter;
};

}  // namespace SkSL

#endif
