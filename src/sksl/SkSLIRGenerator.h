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
#include "ast/SkSLASTStatement.h"
#include "ast/SkSLASTSuffixExpression.h"
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
#include "ir/SkSLSymbolTable.h"
#include "ir/SkSLStatement.h"
#include "ir/SkSLType.h"
#include "ir/SkSLTypeReference.h"
#include "ir/SkSLVarDeclarations.h"

namespace SkSL {

struct CapValue {
    CapValue()
    : fKind(kInt_Kind)
    , fValue(-1) {
        ASSERT(false);
    }

    CapValue(bool b)
    : fKind(kBool_Kind)
    , fValue(b) {}

    CapValue(int i)
    : fKind(kInt_Kind)
    , fValue(i) {}

    enum {
        kBool_Kind,
        kInt_Kind,
    } fKind;
    int fValue;
};

/**
 * Performs semantic analysis on an abstract syntax tree (AST) and produces the corresponding
 * (unoptimized) intermediate representation (IR).
 */
class IRGenerator {
public:
    IRGenerator(const Context* context, std::shared_ptr<SymbolTable> root,
                ErrorReporter& errorReporter);

    sk_up<VarDeclarations> convertVarDeclarations(const ASTVarDeclarations& decl,
                                                  Variable::Storage storage);
    sk_up<FunctionDefinition> convertFunction(const ASTFunction& f);
    sk_up<Statement> convertStatement(const ASTStatement& statement);
    sk_up<Expression> convertExpression(const ASTExpression& expression);
    sk_up<ModifiersDeclaration> convertModifiersDeclaration(const ASTModifiersDeclaration& m);

    Program::Inputs fInputs;

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

    const Type* convertType(const ASTType& type);
    sk_up<Expression> call(Position position,
                           const FunctionDeclaration& function,
                           std::vector<sk_up<Expression>>
                                   arguments);
    bool determineCallCost(const FunctionDeclaration& function,
                           const std::vector<sk_up<Expression>>& arguments,
                           int* outCost);
    sk_up<Expression> call(Position position, sk_up<Expression> function,
                           std::vector<sk_up<Expression>> arguments);
    sk_up<Expression> coerce(sk_up<Expression> expr, const Type& type);
    sk_up<Block> convertBlock(const ASTBlock& block);
    sk_up<Statement> convertBreak(const ASTBreakStatement& b);
    sk_up<Expression> convertConstructor(
            Position position, const Type& type, std::vector<sk_up<Expression>> params);
    sk_up<Statement> convertContinue(const ASTContinueStatement& c);
    sk_up<Statement> convertDiscard(const ASTDiscardStatement& d);
    sk_up<Statement> convertDo(const ASTDoStatement& d);
    sk_up<Expression> convertBinaryExpression(const ASTBinaryExpression& expression);
    // Returns null if it cannot fold the expression. Note that unlike most other functions here, a
    // null return does not represent a compilation error.
    sk_up<Expression> constantFold(const Expression& left, Token::Kind op, const Expression& right);
    sk_up<Extension> convertExtension(const ASTExtension& e);
    sk_up<Statement> convertExpressionStatement(const ASTExpressionStatement& s);
    sk_up<Statement> convertFor(const ASTForStatement& f);
    sk_up<Expression> convertIdentifier(const ASTIdentifier& identifier);
    sk_up<Statement> convertIf(const ASTIfStatement& s);
    sk_up<Expression> convertIndex(sk_up<Expression> base, const ASTExpression& index);
    sk_up<InterfaceBlock> convertInterfaceBlock(const ASTInterfaceBlock& s);
    Modifiers convertModifiers(const Modifiers& m);
    sk_up<Expression> convertPrefixExpression(const ASTPrefixExpression& expression);
    sk_up<Statement> convertReturn(const ASTReturnStatement& r);
    sk_up<Expression> getCap(Position position, SkString name);
    sk_up<Expression> convertSuffixExpression(const ASTSuffixExpression& expression);
    sk_up<Expression> convertField(sk_up<Expression> base, const SkString& field);
    sk_up<Expression> convertSwizzle(sk_up<Expression> base, const SkString& fields);
    sk_up<Expression> convertTernaryExpression(const ASTTernaryExpression& expression);
    sk_up<Statement> convertVarDeclarationStatement(const ASTVarDeclarationStatement& s);
    sk_up<Statement> convertWhile(const ASTWhileStatement& w);

    void checkValid(const Expression& expr);
    void markReadFrom(const Variable& var);
    void markWrittenTo(const Expression& expr);

    const Context& fContext;
    const FunctionDeclaration* fCurrentFunction;
    const Program::Settings* fSettings;
    std::unordered_map<SkString, CapValue> fCapsMap;
    std::shared_ptr<SymbolTable> fSymbolTable;
    int fLoopLevel;
    ErrorReporter& fErrors;

    friend class AutoSymbolTable;
    friend class AutoLoopLevel;
    friend class Compiler;
};

}

#endif
