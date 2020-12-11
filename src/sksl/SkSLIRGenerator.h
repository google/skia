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

#include "src/sksl/SkSLASTFile.h"
#include "src/sksl/SkSLASTNode.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExtension.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLModifiers.h"
#include "src/sksl/ir/SkSLModifiersDeclaration.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLSection.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLTypeReference.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

class ExternalValue;
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
    IRGenerator(const Context* context,
                const ShaderCapsClass* caps,
                ErrorReporter& errorReporter);

    struct IRBundle {
        std::vector<std::unique_ptr<ProgramElement>> fElements;
        std::vector<const ProgramElement*>           fSharedElements;
        std::unique_ptr<ModifiersPool>               fModifiers;
        std::shared_ptr<SymbolTable>                 fSymbolTable;
        Program::Inputs                              fInputs;
    };

    /**
     * If externalValues is supplied, those values are registered in the symbol table of the
     * Program, but ownership is *not* transferred. It is up to the caller to keep them alive.
     */
    IRBundle convertProgram(Program::Kind kind,
                            const Program::Settings* settings,
                            const ParsedModule& base,
                            bool isBuiltinCode,
                            const char* text,
                            size_t length,
                            const std::vector<std::unique_ptr<ExternalValue>>* externalValues);

    /**
     * If both operands are compile-time constants and can be folded, returns an expression
     * representing the folded value. Otherwise, returns null. Note that unlike most other functions
     * here, null does not represent a compilation error.
     */
    std::unique_ptr<Expression> constantFold(const Expression& left,
                                             Token::Kind op,
                                             const Expression& right) const;

    // both of these functions return null and report an error if the setting does not exist
    const Type* typeForSetting(int offset, String name) const;
    std::unique_ptr<Expression> valueForSetting(int offset, String name) const;

    const Program::Settings* settings() const { return fSettings; }

    std::shared_ptr<SymbolTable>& symbolTable() {
        return fSymbolTable;
    }

    void setSymbolTable(std::shared_ptr<SymbolTable>& symbolTable) {
        fSymbolTable = symbolTable;
    }

    void pushSymbolTable();
    void popSymbolTable();

    const Context& fContext;

private:
    /**
     * Relinquishes ownership of the Modifiers that have been collected so far and returns them.
     */
    std::unique_ptr<ModifiersPool> releaseModifiers();

    void checkModifiers(int offset, const Modifiers& modifiers, int permitted);
    StatementArray convertVarDeclarations(const ASTNode& decl, Variable::Storage storage);
    void convertFunction(const ASTNode& f);
    std::unique_ptr<Statement> convertSingleStatement(const ASTNode& statement);
    std::unique_ptr<Statement> convertStatement(const ASTNode& statement);
    std::unique_ptr<Expression> convertExpression(const ASTNode& expression);
    std::unique_ptr<ModifiersDeclaration> convertModifiersDeclaration(const ASTNode& m);

    const Type* convertType(const ASTNode& type, bool allowVoid = false);
    std::unique_ptr<Expression> call(int offset,
                                     const FunctionDeclaration& function,
                                     ExpressionArray arguments);
    CoercionCost callCost(const FunctionDeclaration& function,
                          const ExpressionArray& arguments);
    std::unique_ptr<Expression> call(int offset,
                                     std::unique_ptr<Expression> function,
                                     ExpressionArray arguments);
    CoercionCost coercionCost(const Expression& expr, const Type& type);
    std::unique_ptr<Expression> coerce(std::unique_ptr<Expression> expr, const Type& type);
    template <typename T>
    std::unique_ptr<Expression> constantFoldVector(const Expression& left,
                                                   Token::Kind op,
                                                   const Expression& right) const;
    std::unique_ptr<Block> convertBlock(const ASTNode& block);
    std::unique_ptr<Statement> convertBreak(const ASTNode& b);
    std::unique_ptr<Expression> convertNumberConstructor(int offset,
                                                         const Type& type,
                                                         ExpressionArray params);
    std::unique_ptr<Expression> convertCompoundConstructor(int offset,
                                                           const Type& type,
                                                           ExpressionArray params);
    std::unique_ptr<Expression> convertConstructor(int offset,
                                                   const Type& type,
                                                   ExpressionArray params);
    std::unique_ptr<Statement> convertContinue(const ASTNode& c);
    std::unique_ptr<Statement> convertDiscard(const ASTNode& d);
    std::unique_ptr<Statement> convertDo(const ASTNode& d);
    std::unique_ptr<Statement> convertSwitch(const ASTNode& s);
    std::unique_ptr<Expression> convertBinaryExpression(const ASTNode& expression);
    std::unique_ptr<Extension> convertExtension(int offset, StringFragment name);
    std::unique_ptr<Statement> convertExpressionStatement(const ASTNode& s);
    std::unique_ptr<Statement> convertFor(const ASTNode& f);
    std::unique_ptr<Expression> convertIdentifier(const ASTNode& identifier);
    std::unique_ptr<Statement> convertIf(const ASTNode& s);
    std::unique_ptr<InterfaceBlock> convertInterfaceBlock(const ASTNode& s);
    Modifiers convertModifiers(const Modifiers& m);
    std::unique_ptr<Expression> convertPrefixExpression(const ASTNode& expression);
    std::unique_ptr<Statement> convertReturn(const ASTNode& r);
    std::unique_ptr<Section> convertSection(const ASTNode& e);
    std::unique_ptr<Expression> convertCallExpression(const ASTNode& expression);
    std::unique_ptr<Expression> convertFieldExpression(const ASTNode& expression);
    std::unique_ptr<Expression> convertIndexExpression(const ASTNode& expression);
    std::unique_ptr<Expression> convertIndex(std::unique_ptr<Expression> base,
                                             const ASTNode& index);
    std::unique_ptr<Expression> convertEmptyIndex(std::unique_ptr<Expression> base);
    std::unique_ptr<Expression> convertPostfixExpression(const ASTNode& expression);
    std::unique_ptr<Expression> convertScopeExpression(const ASTNode& expression);
    std::unique_ptr<StructDefinition> convertStructDefinition(const ASTNode& expression);
    std::unique_ptr<Expression> convertTypeField(int offset, const Type& type,
                                                 StringFragment field);
    std::unique_ptr<Expression> convertField(std::unique_ptr<Expression> base,
                                             StringFragment field);
    std::unique_ptr<Expression> convertSwizzle(std::unique_ptr<Expression> base,
                                               StringFragment fields);
    std::unique_ptr<Expression> convertTernaryExpression(const ASTNode& expression);
    std::unique_ptr<Statement> convertVarDeclarationStatement(const ASTNode& s);
    std::unique_ptr<Statement> convertWhile(const ASTNode& w);
    void convertGlobalVarDeclarations(const ASTNode& decl);
    void convertEnum(const ASTNode& e);
    std::unique_ptr<Block> applyInvocationIDWorkaround(std::unique_ptr<Block> main);
    // returns a statement which converts sk_Position from device to normalized coordinates
    std::unique_ptr<Statement> getNormalizeSkPositionCode();

    void checkValid(const Expression& expr);
    bool typeContainsPrivateFields(const Type& type);
    bool setRefKind(Expression& expr, VariableReference::RefKind kind);
    bool getConstantInt(const Expression& value, int64_t* out);
    void copyIntrinsicIfNeeded(const FunctionDeclaration& function);
    void findAndDeclareBuiltinVariables();

    Program::Inputs fInputs;
    const Program::Settings* fSettings = nullptr;
    const ShaderCapsClass* fCaps = nullptr;
    Program::Kind fKind;

    std::unique_ptr<ASTFile> fFile;
    const FunctionDeclaration* fCurrentFunction = nullptr;
    std::unordered_map<String, Program::Settings::Value> fCapsMap;
    std::shared_ptr<SymbolTable> fSymbolTable = nullptr;
    // additional statements that need to be inserted before the one that convertStatement is
    // currently working on
    StatementArray fExtraStatements;
    // Symbols which have definitions in the include files.
    IRIntrinsicMap* fIntrinsics = nullptr;
    std::unordered_set<const FunctionDeclaration*> fReferencedIntrinsics;
    int fLoopLevel = 0;
    int fSwitchLevel = 0;
    ErrorReporter& fErrors;
    int fInvocations;
    std::vector<std::unique_ptr<ProgramElement>>* fProgramElements = nullptr;
    std::vector<const ProgramElement*>*           fSharedElements = nullptr;
    const Variable* fRTAdjust = nullptr;
    const Variable* fRTAdjustInterfaceBlock = nullptr;
    int fRTAdjustFieldIndex;
    bool fCanInline = true;
    // true if we are currently processing one of the built-in SkSL include files
    bool fIsBuiltinCode = false;
    std::unique_ptr<ModifiersPool> fModifiers;

    friend class AutoSymbolTable;
    friend class AutoLoopLevel;
    friend class AutoSwitchLevel;
    friend class AutoDisableInline;
    friend class Compiler;
};

}  // namespace SkSL

#endif
