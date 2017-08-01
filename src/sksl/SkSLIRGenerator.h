/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_IRGENERATOR
#define SKSL_IRGENERATOR

#include "SkSLErrorReporter.h"
#include "ast/SkSLASTBinaryExpression.h"
#include "ast/SkSLASTBlock.h"
#include "ast/SkSLASTBreakStatement.h"
#include "ast/SkSLASTCallSuffix.h"
#include "ast/SkSLASTContinueStatement.h"
#include "ast/SkSLASTDiscardStatement.h"
#include "ast/SkSLASTDoStatement.h"
#include "ast/SkSLASTExpression.h"
#include "ast/SkSLASTExpressionStatement.h"
#include "ast/SkSLASTExtension.h"
#include "ast/SkSLASTForStatement.h"
#include "ast/SkSLASTFunction.h"
#include "ast/SkSLASTIdentifier.h"
#include "ast/SkSLASTIfStatement.h"
#include "ast/SkSLASTInterfaceBlock.h"
#include "ast/SkSLASTModifiersDeclaration.h"
#include "ast/SkSLASTPrefixExpression.h"
#include "ast/SkSLASTReturnStatement.h"
#include "ast/SkSLASTSection.h"
#include "ast/SkSLASTStatement.h"
#include "ast/SkSLASTSuffixExpression.h"
#include "ast/SkSLASTSwitchStatement.h"
#include "ast/SkSLASTTernaryExpression.h"
#include "ast/SkSLASTVarDeclaration.h"
#include "ast/SkSLASTVarDeclarationStatement.h"
#include "ast/SkSLASTWhileStatement.h"
#include "ir/SkSLBlock.h"
#include "ir/SkSLExpression.h"
#include "ir/SkSLExtension.h"
#include "ir/SkSLFunctionDefinition.h"
#include "ir/SkSLInterfaceBlock.h"
#include "ir/SkSLModifiers.h"
#include "ir/SkSLModifiersDeclaration.h"
#include "ir/SkSLProgram.h"
#include "ir/SkSLSection.h"
#include "ir/SkSLSymbolTable.h"
#include "ir/SkSLStatement.h"
#include "ir/SkSLType.h"
#include "ir/SkSLTypeReference.h"
#include "ir/SkSLVarDeclarations.h"

namespace SkSL {

/**
 * Performs semantic analysis on an abstract syntax tree (AST) and produces the corresponding
 * (unoptimized) intermediate representation (IR).
 */
class IRGenerator {
public:
    IRGenerator(const Context* context, std::shared_ptr<SymbolTable> root,
                ErrorReporter& errorReporter);

    void convertProgram(String text,
                        SymbolTable& types,
                        Modifiers::Flag* defaultPrecision,
                        std::vector<std::unique_ptr<ProgramElement>>* result);

    /**
     * If both operands are compile-time constants and can be folded, returns an expression
     * representing the folded value. Otherwise, returns null. Note that unlike most other functions
     * here, null does not represent a compilation error.
     */
    std::unique_ptr<Expression> constantFold(const Expression& left,
                                             Token::Kind op,
                                             const Expression& right) const;
    Program::Inputs fInputs;
    const Program::Settings* fSettings;
    const Context& fContext;

private:
    /**
     * Prepare to compile a program. Resets state, pushes a new symbol table, and installs the
     * settings.
     */
    void start(const Program::Settings* settings);

    /**
     * Performs cleanup after compilation is complete.
     */
    void finish();

    void pushSymbolTable();
    void popSymbolTable();

    std::unique_ptr<VarDeclarations> convertVarDeclarations(const ASTVarDeclarations& decl,
                                                            Variable::Storage storage);
    void convertFunction(const ASTFunction& f,
                         std::vector<std::unique_ptr<ProgramElement>>* out);
    std::unique_ptr<Statement> convertStatement(const ASTStatement& statement);
    std::unique_ptr<Expression> convertExpression(const ASTExpression& expression);
    std::unique_ptr<ModifiersDeclaration> convertModifiersDeclaration(
                                                                  const ASTModifiersDeclaration& m);

    const Type* convertType(const ASTType& type);
    std::unique_ptr<Expression> call(Position position,
                                     const FunctionDeclaration& function,
                                     std::vector<std::unique_ptr<Expression>> arguments);
    bool determineCallCost(const FunctionDeclaration& function,
                           const std::vector<std::unique_ptr<Expression>>& arguments,
                           int* outCost);
    std::unique_ptr<Expression> call(Position position, std::unique_ptr<Expression> function,
                                     std::vector<std::unique_ptr<Expression>> arguments);
    std::unique_ptr<Expression> coerce(std::unique_ptr<Expression> expr, const Type& type);
    std::unique_ptr<Block> convertBlock(const ASTBlock& block);
    std::unique_ptr<Statement> convertBreak(const ASTBreakStatement& b);
    std::unique_ptr<Expression> convertNumberConstructor(
                                                   Position position,
                                                   const Type& type,
                                                   std::vector<std::unique_ptr<Expression>> params);
    std::unique_ptr<Expression> convertCompoundConstructor(
                                                   Position position,
                                                   const Type& type,
                                                   std::vector<std::unique_ptr<Expression>> params);
    std::unique_ptr<Expression> convertConstructor(Position position,
                                                   const Type& type,
                                                   std::vector<std::unique_ptr<Expression>> params);
    std::unique_ptr<Statement> convertContinue(const ASTContinueStatement& c);
    std::unique_ptr<Statement> convertDiscard(const ASTDiscardStatement& d);
    std::unique_ptr<Statement> convertDo(const ASTDoStatement& d);
    std::unique_ptr<Statement> convertSwitch(const ASTSwitchStatement& s);
    std::unique_ptr<Expression> convertBinaryExpression(const ASTBinaryExpression& expression);
    std::unique_ptr<Extension> convertExtension(const ASTExtension& e);
    std::unique_ptr<Statement> convertExpressionStatement(const ASTExpressionStatement& s);
    std::unique_ptr<Statement> convertFor(const ASTForStatement& f);
    std::unique_ptr<Expression> convertIdentifier(const ASTIdentifier& identifier);
    std::unique_ptr<Statement> convertIf(const ASTIfStatement& s);
    std::unique_ptr<Expression> convertIndex(std::unique_ptr<Expression> base,
                                             const ASTExpression& index);
    std::unique_ptr<InterfaceBlock> convertInterfaceBlock(const ASTInterfaceBlock& s);
    Modifiers convertModifiers(const Modifiers& m);
    std::unique_ptr<Expression> convertPrefixExpression(const ASTPrefixExpression& expression);
    std::unique_ptr<Statement> convertReturn(const ASTReturnStatement& r);
    std::unique_ptr<Section> convertSection(const ASTSection& e);
    std::unique_ptr<Expression> getCap(Position position, String name);
    std::unique_ptr<Expression> getArg(Position position, String name);
    std::unique_ptr<Expression> convertSuffixExpression(const ASTSuffixExpression& expression);
    std::unique_ptr<Expression> convertField(std::unique_ptr<Expression> base,
                                             const String& field);
    std::unique_ptr<Expression> convertSwizzle(std::unique_ptr<Expression> base,
                                               const String& fields);
    std::unique_ptr<Expression> convertTernaryExpression(const ASTTernaryExpression& expression);
    std::unique_ptr<Statement> convertVarDeclarationStatement(const ASTVarDeclarationStatement& s);
    std::unique_ptr<Statement> convertWhile(const ASTWhileStatement& w);
    std::unique_ptr<Block> applyInvocationIDWorkaround(
                                                 std::unique_ptr<Block> main,
                                                 std::vector<std::unique_ptr<ProgramElement>>* out);

    /**
     * Wraps an expression in code that applies a colorspace transformation to it. This is used
     * to implement texture(sampler, coord, colorSpaceXForm).
     */
    std::unique_ptr<Expression> applyColorSpace(std::unique_ptr<Expression> texture,
                                                std::unique_ptr<Expression> xform);
    void fixRectSampling(std::vector<std::unique_ptr<Expression>>& arguments);
    void checkValid(const Expression& expr);
    void markWrittenTo(const Expression& expr, bool readWrite);

    const FunctionDeclaration* fCurrentFunction;
    std::unordered_map<String, Program::Settings::Value> fCapsMap;
    std::shared_ptr<SymbolTable> fRootSymbolTable;
    std::shared_ptr<SymbolTable> fSymbolTable;
    // holds extra temp variable declarations needed for the current function
    std::vector<std::unique_ptr<Statement>> fExtraVars;
    int fLoopLevel;
    int fSwitchLevel;
    // count of temporary variables we have created
    int fTmpCount;
    ErrorReporter& fErrors;
    int fInvocations;

    friend class AutoSymbolTable;
    friend class AutoLoopLevel;
    friend class AutoSwitchLevel;
    friend class Compiler;
};

}

#endif
