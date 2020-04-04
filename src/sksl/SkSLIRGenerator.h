/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_IRGENERATOR
#define SKSL_IRGENERATOR

#include <map>

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

struct Swizzle;

/**
 * Performs semantic analysis on an abstract syntax tree (AST) and produces the corresponding
 * (unoptimized) intermediate representation (IR).
 */
class IRGenerator {
public:
    IRGenerator(const Context* context, std::shared_ptr<SymbolTable> root,
                ErrorReporter& errorReporter);

    void convertProgram(Program::Kind kind,
                        const char* text,
                        size_t length,
                        SymbolTable& types,
                        std::vector<std::unique_ptr<ProgramElement>>* result);

    /**
     * If both operands are compile-time constants and can be folded, returns an expression
     * representing the folded value. Otherwise, returns null. Note that unlike most other functions
     * here, null does not represent a compilation error.
     */
    std::unique_ptr<Expression> constantFold(const Expression& left,
                                             Token::Kind op,
                                             const Expression& right) const;

    std::unique_ptr<Expression> getArg(int offset, String name) const;

    Program::Inputs fInputs;
    const Program::Settings* fSettings;
    const Context& fContext;
    Program::Kind fKind;

private:
    /**
     * Prepare to compile a program. Resets state, pushes a new symbol table, and installs the
     * settings.
     */
    void start(const Program::Settings* settings,
               std::vector<std::unique_ptr<ProgramElement>>* inherited);

    /**
     * Performs cleanup after compilation is complete.
     */
    void finish();

    void pushSymbolTable();
    void popSymbolTable();

    std::unique_ptr<VarDeclarations> convertVarDeclarations(const ASTNode& decl,
                                                            Variable::Storage storage);
    void convertFunction(const ASTNode& f);
    std::unique_ptr<Statement> convertStatement(const ASTNode& statement);
    std::unique_ptr<Expression> convertExpression(const ASTNode& expression);
    std::unique_ptr<ModifiersDeclaration> convertModifiersDeclaration(const ASTNode& m);

    const Type* convertType(const ASTNode& type);
    std::unique_ptr<Expression> call(int offset,
                                     const FunctionDeclaration& function,
                                     std::vector<std::unique_ptr<Expression>> arguments);
    int callCost(const FunctionDeclaration& function,
                 const std::vector<std::unique_ptr<Expression>>& arguments);
    std::unique_ptr<Expression> call(int offset, std::unique_ptr<Expression> function,
                                     std::vector<std::unique_ptr<Expression>> arguments);
    int coercionCost(const Expression& expr, const Type& type);
    std::unique_ptr<Expression> coerce(std::unique_ptr<Expression> expr, const Type& type);
    std::unique_ptr<Block> convertBlock(const ASTNode& block);
    std::unique_ptr<Statement> convertBreak(const ASTNode& b);
    std::unique_ptr<Expression> convertNumberConstructor(
                                                   int offset,
                                                   const Type& type,
                                                   std::vector<std::unique_ptr<Expression>> params);
    std::unique_ptr<Expression> convertCompoundConstructor(
                                                   int offset,
                                                   const Type& type,
                                                   std::vector<std::unique_ptr<Expression>> params);
    std::unique_ptr<Expression> convertConstructor(int offset,
                                                   const Type& type,
                                                   std::vector<std::unique_ptr<Expression>> params);
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
    std::unique_ptr<Expression> convertIndex(std::unique_ptr<Expression> base,
                                             const ASTNode& index);
    std::unique_ptr<InterfaceBlock> convertInterfaceBlock(const ASTNode& s);
    Modifiers convertModifiers(const Modifiers& m);
    std::unique_ptr<Expression> convertPrefixExpression(const ASTNode& expression);
    std::unique_ptr<Statement> convertReturn(const ASTNode& r);
    std::unique_ptr<Section> convertSection(const ASTNode& e);
    std::unique_ptr<Expression> getCap(int offset, String name);
    std::unique_ptr<Expression> convertCallExpression(const ASTNode& expression);
    std::unique_ptr<Expression> convertFieldExpression(const ASTNode& expression);
    std::unique_ptr<Expression> convertIndexExpression(const ASTNode& expression);
    std::unique_ptr<Expression> convertPostfixExpression(const ASTNode& expression);
    std::unique_ptr<Expression> convertTypeField(int offset, const Type& type,
                                                 StringFragment field);
    std::unique_ptr<Expression> convertField(std::unique_ptr<Expression> base,
                                             StringFragment field);
    std::unique_ptr<Expression> convertSwizzle(std::unique_ptr<Expression> base,
                                               StringFragment fields);
    std::unique_ptr<Expression> convertTernaryExpression(const ASTNode& expression);
    std::unique_ptr<Statement> convertVarDeclarationStatement(const ASTNode& s);
    std::unique_ptr<Statement> convertWhile(const ASTNode& w);
    void convertEnum(const ASTNode& e);
    std::unique_ptr<Block> applyInvocationIDWorkaround(std::unique_ptr<Block> main);
    // returns a statement which converts sk_Position from device to normalized coordinates
    std::unique_ptr<Statement> getNormalizeSkPositionCode();
    void removeSampleMask(std::vector<std::unique_ptr<ProgramElement>>* out);

    void checkValid(const Expression& expr);
    void setRefKind(const Expression& expr, VariableReference::RefKind kind);
    void getConstantInt(const Expression& value, int64_t* out);
    bool checkSwizzleWrite(const Swizzle& swizzle);

    std::unique_ptr<ASTFile> fFile;
    const FunctionDeclaration* fCurrentFunction;
    std::unordered_map<String, Program::Settings::Value> fCapsMap;
    std::shared_ptr<SymbolTable> fRootSymbolTable;
    std::shared_ptr<SymbolTable> fSymbolTable;
    // Symbols which have definitions in the include files. The bool tells us whether this
    // intrinsic has been included already.
    std::map<StringFragment, std::pair<std::unique_ptr<ProgramElement>, bool>>* fIntrinsics = nullptr;
    // holds extra temp variable declarations needed for the current function
    std::vector<std::unique_ptr<Statement>> fExtraVars;
    int fLoopLevel;
    int fSwitchLevel;
    // count of temporary variables we have created
    int fTmpCount;
    ErrorReporter& fErrors;
    int fInvocations;
    std::vector<std::unique_ptr<ProgramElement>>* fProgramElements;
    const Variable* fSkPerVertex = nullptr;
    Variable* fRTAdjust;
    Variable* fRTAdjustInterfaceBlock;
    int fRTAdjustFieldIndex;
    bool fStarted = false;

    friend class AutoSymbolTable;
    friend class AutoLoopLevel;
    friend class AutoSwitchLevel;
    friend class Compiler;
};

}

#endif
