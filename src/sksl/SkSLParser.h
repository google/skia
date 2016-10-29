/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_PARSER
#define SKSL_PARSER

#include <string>
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
struct ASTLayout;
struct ASTModifiers;
struct ASTParameter;
struct ASTReturnStatement;
struct ASTStatement;
struct ASTSuffix;
struct ASTType;
struct ASTWhileStatement;
struct ASTVarDeclarations;
class SymbolTable;

/**
 * Consumes .sksl text and produces an abstract syntax tree describing the contents.
 */
class Parser {
public:
    Parser(std::string text, SymbolTable& types, ErrorReporter& errors);

    ~Parser();

    /**
     * Consumes a complete .sksl file and produces a list of declarations. Errors are reported via
     * the ErrorReporter; the return value may contain some declarations even when errors have
     * occurred.
     */
    std::vector<std::unique_ptr<ASTDeclaration>> file();

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
    bool expect(Token::Kind kind, std::string expected, Token* result = nullptr);

    void error(Position p, std::string msg);
    
    /**
     * Returns true if the 'name' identifier refers to a type name. For instance, isType("int") will
     * always return true.
     */
    bool isType(std::string name);

    // these functions parse individual grammar rules from the current parse position; you probably
    // don't need to call any of these outside of the parser. The function declarations in the .cpp
    // file have comments describing the grammar rules.

    void precision();

    std::unique_ptr<ASTDeclaration> directive();

    std::unique_ptr<ASTDeclaration> declaration();

    std::unique_ptr<ASTVarDeclarations> varDeclarations();

    std::unique_ptr<ASTType> structDeclaration();

    std::unique_ptr<ASTVarDeclarations> structVarDeclaration(ASTModifiers modifiers);

    std::unique_ptr<ASTVarDeclarations> varDeclarationEnd(ASTModifiers modifiers,
                                                          std::unique_ptr<ASTType> type, 
                                                          std::string name);

    std::unique_ptr<ASTParameter> parameter();

    int layoutInt();
    
    ASTLayout layout();

    ASTModifiers modifiers();

    ASTModifiers modifiersWithDefaults(int defaultFlags);

    std::unique_ptr<ASTStatement> statement();

    std::unique_ptr<ASTType> type();

    std::unique_ptr<ASTDeclaration> interfaceBlock(ASTModifiers mods);

    std::unique_ptr<ASTIfStatement> ifStatement();

    std::unique_ptr<ASTDoStatement> doStatement();

    std::unique_ptr<ASTWhileStatement> whileStatement();

    std::unique_ptr<ASTForStatement> forStatement();

    std::unique_ptr<ASTReturnStatement> returnStatement();

    std::unique_ptr<ASTBreakStatement> breakStatement();

    std::unique_ptr<ASTContinueStatement> continueStatement();

    std::unique_ptr<ASTDiscardStatement> discardStatement();

    std::unique_ptr<ASTBlock> block();

    std::unique_ptr<ASTExpressionStatement> expressionStatement();

    std::unique_ptr<ASTExpression> expression();

    std::unique_ptr<ASTExpression> assignmentExpression();
    
    std::unique_ptr<ASTExpression> ternaryExpression();

    std::unique_ptr<ASTExpression> logicalOrExpression();

    std::unique_ptr<ASTExpression> logicalXorExpression();

    std::unique_ptr<ASTExpression> logicalAndExpression();

    std::unique_ptr<ASTExpression> bitwiseOrExpression();

    std::unique_ptr<ASTExpression> bitwiseXorExpression();

    std::unique_ptr<ASTExpression> bitwiseAndExpression();

    std::unique_ptr<ASTExpression> equalityExpression();

    std::unique_ptr<ASTExpression> relationalExpression();

    std::unique_ptr<ASTExpression> shiftExpression();

    std::unique_ptr<ASTExpression> additiveExpression();

    std::unique_ptr<ASTExpression> multiplicativeExpression();

    std::unique_ptr<ASTExpression> unaryExpression();

    std::unique_ptr<ASTExpression> postfixExpression();

    std::unique_ptr<ASTSuffix> suffix();

    std::unique_ptr<ASTExpression> term();

    bool intLiteral(int64_t* dest);

    bool floatLiteral(double* dest);

    bool boolLiteral(bool* dest);

    bool identifier(std::string* dest);


    void* fScanner;
    YY_BUFFER_STATE fBuffer;
    Token fPushback;
    SymbolTable& fTypes;
    ErrorReporter& fErrors;
};

} // namespace

#endif
