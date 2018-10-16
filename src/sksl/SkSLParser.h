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
#include <unordered_map>
#include <unordered_set>
#include "SkSLErrorReporter.h"
#include "ir/SkSLLayout.h"
#include "SkSLLexer.h"

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
struct ASTSwitchCase;
struct ASTSwitchStatement;
struct ASTType;
struct ASTWhileStatement;
struct ASTVarDeclarations;
struct Modifiers;
class SymbolTable;

/**
 * Consumes .sksl text and produces an abstract syntax tree describing the contents.
 */
class Parser {
public:
    enum class LayoutToken {
        LOCATION,
        OFFSET,
        BINDING,
        INDEX,
        SET,
        BUILTIN,
        INPUT_ATTACHMENT_INDEX,
        ORIGIN_UPPER_LEFT,
        OVERRIDE_COVERAGE,
        BLEND_SUPPORT_ALL_EQUATIONS,
        BLEND_SUPPORT_MULTIPLY,
        BLEND_SUPPORT_SCREEN,
        BLEND_SUPPORT_OVERLAY,
        BLEND_SUPPORT_DARKEN,
        BLEND_SUPPORT_LIGHTEN,
        BLEND_SUPPORT_COLORDODGE,
        BLEND_SUPPORT_COLORBURN,
        BLEND_SUPPORT_HARDLIGHT,
        BLEND_SUPPORT_SOFTLIGHT,
        BLEND_SUPPORT_DIFFERENCE,
        BLEND_SUPPORT_EXCLUSION,
        BLEND_SUPPORT_HSL_HUE,
        BLEND_SUPPORT_HSL_SATURATION,
        BLEND_SUPPORT_HSL_COLOR,
        BLEND_SUPPORT_HSL_LUMINOSITY,
        PUSH_CONSTANT,
        POINTS,
        LINES,
        LINE_STRIP,
        LINES_ADJACENCY,
        TRIANGLES,
        TRIANGLE_STRIP,
        TRIANGLES_ADJACENCY,
        MAX_VERTICES,
        INVOCATIONS,
        WHEN,
        KEY,
        TRACKED,
        CTYPE,
        SKPMCOLOR4F,
        SKRECT,
        SKIRECT,
        SKPMCOLOR,
    };

    Parser(const char* text, size_t length, SymbolTable& types, ErrorReporter& errors);

    /**
     * Consumes a complete .sksl file and produces a list of declarations. Errors are reported via
     * the ErrorReporter; the return value may contain some declarations even when errors have
     * occurred.
     */
    std::vector<std::unique_ptr<ASTDeclaration>> file();

    StringFragment text(Token token);

    Position position(Token token);

private:
    static void InitLayoutMap();

    /**
     * Return the next token, including whitespace tokens, from the parse stream.
     */
    Token nextRawToken();

    /**
     * Return the next non-whitespace token from the parse stream.
     */
    Token nextToken();

    /**
     * Push a token back onto the parse stream, so that it is the next one read. Only a single level
     * of pushback is supported (that is, it is an error to call pushback() twice in a row without
     * an intervening nextToken()).
     */
    void pushback(Token t);

    /**
     * Returns the next non-whitespace token without consuming it from the stream.
     */
    Token peek();

    /**
     * Checks to see if the next token is of the specified type. If so, stores it in result (if
     * result is non-null) and returns true. Otherwise, pushes it back and returns false.
     */
    bool checkNext(Token::Kind kind, Token* result = nullptr);

    /**
     * Reads the next non-whitespace token and generates an error if it is not the expected type.
     * The 'expected' string is part of the error message, which reads:
     *
     * "expected <expected>, but found '<actual text>'"
     *
     * If 'result' is non-null, it is set to point to the token that was read.
     * Returns true if the read token was as expected, false otherwise.
     */
    bool expect(Token::Kind kind, const char* expected, Token* result = nullptr);
    bool expect(Token::Kind kind, String expected, Token* result = nullptr);

    void error(Token token, String msg);
    void error(int offset, String msg);
    /**
     * Returns true if the 'name' identifier refers to a type name. For instance, isType("int") will
     * always return true.
     */
    bool isType(StringFragment name);

    // these functions parse individual grammar rules from the current parse position; you probably
    // don't need to call any of these outside of the parser. The function declarations in the .cpp
    // file have comments describing the grammar rules.

    std::unique_ptr<ASTDeclaration> precision();

    std::unique_ptr<ASTDeclaration> directive();

    std::unique_ptr<ASTDeclaration> section();

    std::unique_ptr<ASTDeclaration> enumDeclaration();

    std::unique_ptr<ASTDeclaration> declaration();

    std::unique_ptr<ASTVarDeclarations> varDeclarations();

    std::unique_ptr<ASTType> structDeclaration();

    std::unique_ptr<ASTVarDeclarations> structVarDeclaration(Modifiers modifiers);

    std::unique_ptr<ASTVarDeclarations> varDeclarationEnd(Modifiers modifiers,
                                                          std::unique_ptr<ASTType> type,
                                                          StringFragment name);

    std::unique_ptr<ASTParameter> parameter();

    int layoutInt();

    StringFragment layoutIdentifier();

    String layoutCode();

    Layout::Key layoutKey();

    Layout::CType layoutCType();

    Layout layout();

    Modifiers modifiers();

    Modifiers modifiersWithDefaults(int defaultFlags);

    std::unique_ptr<ASTStatement> statement();

    std::unique_ptr<ASTType> type();

    std::unique_ptr<ASTDeclaration> interfaceBlock(Modifiers mods);

    std::unique_ptr<ASTIfStatement> ifStatement();

    std::unique_ptr<ASTDoStatement> doStatement();

    std::unique_ptr<ASTWhileStatement> whileStatement();

    std::unique_ptr<ASTForStatement> forStatement();

    std::unique_ptr<ASTSwitchCase> switchCase();

    std::unique_ptr<ASTStatement> switchStatement();

    std::unique_ptr<ASTReturnStatement> returnStatement();

    std::unique_ptr<ASTBreakStatement> breakStatement();

    std::unique_ptr<ASTContinueStatement> continueStatement();

    std::unique_ptr<ASTDiscardStatement> discardStatement();

    std::unique_ptr<ASTBlock> block();

    std::unique_ptr<ASTExpressionStatement> expressionStatement();

    std::unique_ptr<ASTExpression> expression();

    std::unique_ptr<ASTExpression> commaExpression();

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

    bool identifier(StringFragment* dest);

    static std::unordered_map<String, LayoutToken>* layoutTokens;

    const char* fText;
    Lexer fLexer;
    YY_BUFFER_STATE fBuffer;
    // current parse depth, used to enforce a recursion limit to try to keep us from overflowing the
    // stack on pathological inputs
    int fDepth = 0;
    Token fPushback;
    SymbolTable& fTypes;
    ErrorReporter& fErrors;

    friend class AutoDepth;
    friend class HCodeGenerator;
};

} // namespace

#endif
