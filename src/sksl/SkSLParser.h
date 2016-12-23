/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PARSER
#define SKSL_PARSER

#include <vector>
#include <memory>
#include <unordered_set>
#include "SkSLErrorReporter.h"
#include "SkSLToken.h"

struct yy_buffer_state;
#define YY_TYPEDEF_YY_BUFFER_STATE
typedef struct yy_buffer_state *YY_BUFFER_STATE;

namespace SkSL {

struct ASTBlock;
struct ASTBreakStatement;
struct ASTContinueStatement;
struct ASTDeclaration;
struct ASTDiscardStatement;
struct ASTDoStatement;
struct ASTExpression;
struct ASTExpressionStatement;
struct ASTForStatement;
struct ASTIfStatement;
struct ASTInterfaceBlock;
struct ASTParameter;
struct ASTPrecision;
struct ASTReturnStatement;
struct ASTStatement;
struct ASTSuffix;
struct ASTType;
struct ASTWhileStatement;
struct ASTVarDeclarations;
struct Layout;
struct Modifiers;
class SymbolTable;

/**
 * Consumes .sksl text and produces an abstract syntax tree describing the contents.
 */
class Parser {
public:
    Parser(SkString text, SymbolTable& types, ErrorReporter& errors);

    ~Parser();

    /**
     * Consumes a complete .sksl file and produces a list of declarations. Errors are reported via
     * the ErrorReporter; the return value may contain some declarations even when errors have
     * occurred.
     */
    std::vector<sk_up<ASTDeclaration>> file();

private:
    /**
     * Return the next token from the parse stream.
     */
    Token nextToken();

    /**
     * Push a token back onto the parse stream, so that it is the next one read. Only a single level
     * of pushback is supported (that is, it is an error to call pushback() twice in a row without
     * an intervening nextToken()).
     */
    void pushback(Token t);

    /**
     * Returns the next token without consuming it from the stream.
     */
    Token peek();

    /**
     * Reads the next token and generates an error if it is not the expected type. The 'expected'
     * string is part of the error message, which reads:
     *
     * "expected <expected>, but found '<actual text>'"
     *
     * If 'result' is non-null, it is set to point to the token that was read.
     * Returns true if the read token was as expected, false otherwise.
     */
    bool expect(Token::Kind kind, const char* expected, Token* result = nullptr);
    bool expect(Token::Kind kind, SkString expected, Token* result = nullptr);

    void error(Position p, const char* msg);
    void error(Position p, SkString msg);
   
    /**
     * Returns true if the 'name' identifier refers to a type name. For instance, isType("int") will
     * always return true.
     */
    bool isType(SkString name);

    // these functions parse individual grammar rules from the current parse position; you probably
    // don't need to call any of these outside of the parser. The function declarations in the .cpp
    // file have comments describing the grammar rules.

    sk_up<ASTDeclaration> precision();

    sk_up<ASTDeclaration> directive();

    sk_up<ASTDeclaration> declaration();

    sk_up<ASTVarDeclarations> varDeclarations();

    sk_up<ASTType> structDeclaration();

    sk_up<ASTVarDeclarations> structVarDeclaration(Modifiers modifiers);

    sk_up<ASTVarDeclarations> varDeclarationEnd(
            Modifiers modifiers, sk_up<ASTType> type, SkString name);

    sk_up<ASTParameter> parameter();

    int layoutInt();
   
    Layout layout();

    Modifiers modifiers();

    Modifiers modifiersWithDefaults(int defaultFlags);

    sk_up<ASTStatement> statement();

    sk_up<ASTType> type();

    sk_up<ASTDeclaration> interfaceBlock(Modifiers mods);

    sk_up<ASTIfStatement> ifStatement();

    sk_up<ASTDoStatement> doStatement();

    sk_up<ASTWhileStatement> whileStatement();

    sk_up<ASTForStatement> forStatement();

    sk_up<ASTReturnStatement> returnStatement();

    sk_up<ASTBreakStatement> breakStatement();

    sk_up<ASTContinueStatement> continueStatement();

    sk_up<ASTDiscardStatement> discardStatement();

    sk_up<ASTBlock> block();

    sk_up<ASTExpressionStatement> expressionStatement();

    sk_up<ASTExpression> expression();

    sk_up<ASTExpression> assignmentExpression();

    sk_up<ASTExpression> ternaryExpression();

    sk_up<ASTExpression> logicalOrExpression();

    sk_up<ASTExpression> logicalXorExpression();

    sk_up<ASTExpression> logicalAndExpression();

    sk_up<ASTExpression> bitwiseOrExpression();

    sk_up<ASTExpression> bitwiseXorExpression();

    sk_up<ASTExpression> bitwiseAndExpression();

    sk_up<ASTExpression> equalityExpression();

    sk_up<ASTExpression> relationalExpression();

    sk_up<ASTExpression> shiftExpression();

    sk_up<ASTExpression> additiveExpression();

    sk_up<ASTExpression> multiplicativeExpression();

    sk_up<ASTExpression> unaryExpression();

    sk_up<ASTExpression> postfixExpression();

    sk_up<ASTSuffix> suffix();

    sk_up<ASTExpression> term();

    bool intLiteral(int64_t* dest);

    bool floatLiteral(double* dest);

    bool boolLiteral(bool* dest);

    bool identifier(SkString* dest);

    void* fScanner;
    YY_BUFFER_STATE fBuffer;
    // current parse depth, used to enforce a recursion limit to try to keep us from overflowing the
    // stack on pathological inputs
    int fDepth = 0;
    Token fPushback;
    SymbolTable& fTypes;
    ErrorReporter& fErrors;

    friend class AutoDepth;
};

} // namespace

#endif
