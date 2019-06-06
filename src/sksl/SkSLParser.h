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
#include "src/sksl/SkSLASTFile.h"
#include "src/sksl/SkSLASTNode.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLLexer.h"
#include "src/sksl/ir/SkSLLayout.h"

struct yy_buffer_state;
#define YY_TYPEDEF_YY_BUFFER_STATE
typedef struct yy_buffer_state *YY_BUFFER_STATE;

namespace SkSL {

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
        SKVECTOR4,
        SKRECT,
        SKIRECT,
        SKPMCOLOR,
        SKMATRIX44,
        BOOL,
        INT,
        FLOAT,
    };

    Parser(const char* text, size_t length, SymbolTable& types, ErrorReporter& errors);

    /**
     * Consumes a complete .sksl file and returns the parse tree. Errors are reported via the
     * ErrorReporter; the return value may contain some declarations even when errors have occurred.
     */
    std::unique_ptr<ASTFile> file();

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

    // The pointer to the node may be invalidated by modifying the fNodes vector
    ASTNode& getNode(ASTNode::ID id) {
        return fFile->fNodes[id.fValue];
    }

    // these functions parse individual grammar rules from the current parse position; you probably
    // don't need to call any of these outside of the parser. The function declarations in the .cpp
    // file have comments describing the grammar rules.

    ASTNode::ID precision();

    ASTNode::ID directive();

    ASTNode::ID section();

    ASTNode::ID enumDeclaration();

    ASTNode::ID declaration();

    ASTNode::ID varDeclarations();

    ASTNode::ID structDeclaration();

    ASTNode::ID structVarDeclaration(Modifiers modifiers);

    ASTNode::ID varDeclarationEnd(Modifiers modifiers, ASTNode::ID type, StringFragment name);

    ASTNode::ID parameter();

    int layoutInt();

    StringFragment layoutIdentifier();

    StringFragment layoutCode();

    Layout::Key layoutKey();

    Layout::CType layoutCType();

    Layout layout();

    Modifiers modifiers();

    Modifiers modifiersWithDefaults(int defaultFlags);

    ASTNode::ID statement();

    ASTNode::ID type();

    ASTNode::ID interfaceBlock(Modifiers mods);

    ASTNode::ID ifStatement();

    ASTNode::ID doStatement();

    ASTNode::ID whileStatement();

    ASTNode::ID forStatement();

    ASTNode::ID switchCase();

    ASTNode::ID switchStatement();

    ASTNode::ID returnStatement();

    ASTNode::ID breakStatement();

    ASTNode::ID continueStatement();

    ASTNode::ID discardStatement();

    ASTNode::ID block();

    ASTNode::ID expressionStatement();

    ASTNode::ID expression();

    ASTNode::ID assignmentExpression();

    ASTNode::ID ternaryExpression();

    ASTNode::ID logicalOrExpression();

    ASTNode::ID logicalXorExpression();

    ASTNode::ID logicalAndExpression();

    ASTNode::ID bitwiseOrExpression();

    ASTNode::ID bitwiseXorExpression();

    ASTNode::ID bitwiseAndExpression();

    ASTNode::ID equalityExpression();

    ASTNode::ID relationalExpression();

    ASTNode::ID shiftExpression();

    ASTNode::ID additiveExpression();

    ASTNode::ID multiplicativeExpression();

    ASTNode::ID unaryExpression();

    ASTNode::ID postfixExpression();

    ASTNode::ID suffix(ASTNode::ID base);

    ASTNode::ID term();

    bool intLiteral(SKSL_INT* dest);

    bool floatLiteral(SKSL_FLOAT* dest);

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

    std::unique_ptr<ASTFile> fFile;

    friend class AutoDepth;
    friend class HCodeGenerator;
};

} // namespace

#endif
