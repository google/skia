/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSLPARSER
#define SKSL_DSLPARSER

#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "include/private/SkSLLayout.h"
#include "include/private/SkSLProgramKind.h"
#include "include/sksl/DSL.h"
#include "src/sksl/SkSLASTFile.h"
#include "src/sksl/SkSLASTNode.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/SkSLLexer.h"
#include "src/sksl/dsl/DSLOptional.h"

namespace SkSL {

struct Modifiers;
class SymbolTable;

/**
 * Consumes .sksl text and invokes DSL functions to instantiate the program.
 */
class DSLParser {
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
        EARLY_FRAGMENT_TESTS,
        BLEND_SUPPORT_ALL_EQUATIONS,
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
        MARKER,
        WHEN,
        KEY,
        TRACKED,
        SRGB_UNPREMUL,
        CTYPE,
        SKPMCOLOR4F,
        SKV4,
        SKRECT,
        SKIRECT,
        SKPMCOLOR,
        SKM44,
        BOOL,
        INT,
        FLOAT,
    };

    DSLParser(Compiler* compiler, int flags, ProgramKind kind, const char* text,
              size_t length);

    std::unique_ptr<Program> program();

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

    /**
     * Behaves like expect(TK_IDENTIFIER), but also verifies that identifier is not a type.
     * If the token was actually a type, generates an error message of the form:
     *
     * "expected an identifier, but found type 'float2'"
     */
    bool expectIdentifier(Token* result);

    ErrorReporter& errors() {
        return *fErrorReporter;
    }
    void error(Token token, String msg);
    void error(int offset, String msg);

    SymbolTable& symbols() {
        return *fCompiler.fIRGenerator->fSymbolTable;
    }

    /**
     * Returns true if the 'name' identifier refers to a type name. For instance, isType("int") will
     * always return true.
     */
    bool isType(StringFragment name);

    // these functions parse individual grammar rules from the current parse position; you probably
    // don't need to call any of these outside of the parser. The function declarations in the .cpp
    // file have comments describing the grammar rules.

    ASTNode::ID precision();

    ASTNode::ID directive();

    ASTNode::ID section();

    ASTNode::ID enumDeclaration();

    bool declaration();

    struct VarDeclarationsPrefix {
        dsl::DSLModifiers modifiers;
        dsl::DSLType type = dsl::DSLType(dsl::kVoid_Type);
        Token name;
    };

    bool varDeclarationsPrefix(VarDeclarationsPrefix* prefixData);

    dsl::DSLOptional<dsl::DSLStatement> varDeclarationsOrExpressionStatement();

    dsl::DSLOptional<dsl::DSLStatement> varDeclarations();

    ASTNode::ID structDeclaration();

    ASTNode::ID structVarDeclaration(Modifiers modifiers);

    SkTArray<dsl::DSLVar> varDeclarationEnd(dsl::DSLModifiers modifiers, dsl::DSLType type,
                                       StringFragment name);

    dsl::DSLOptional<dsl::DSLWrapper<dsl::DSLVar>> parameter();

    int layoutInt();

    StringFragment layoutIdentifier();

    StringFragment layoutCode();

    Layout::CType layoutCType();

    dsl::DSLLayout layout();

    dsl::DSLModifiers modifiers();

    dsl::DSLModifiers modifiersWithDefaults(int defaultFlags);

    dsl::DSLOptional<dsl::DSLStatement> statement();

    dsl::DSLOptional<dsl::DSLType> type();

    ASTNode::ID interfaceBlock(Modifiers mods);

    dsl::DSLOptional<dsl::DSLStatement> ifStatement();

    dsl::DSLOptional<dsl::DSLStatement> doStatement();

    dsl::DSLOptional<dsl::DSLStatement> whileStatement();

    dsl::DSLOptional<dsl::DSLStatement> forStatement();

    dsl::DSLOptional<dsl::DSLCase> switchCase();

    dsl::DSLOptional<dsl::DSLStatement> switchStatement();

    dsl::DSLOptional<dsl::DSLStatement> returnStatement();

    dsl::DSLOptional<dsl::DSLStatement> breakStatement();

    dsl::DSLOptional<dsl::DSLStatement> continueStatement();

    dsl::DSLOptional<dsl::DSLStatement> discardStatement();

    dsl::DSLOptional<dsl::DSLBlock> block();

    dsl::DSLOptional<dsl::DSLStatement> expressionStatement();

    dsl::DSLOptional<dsl::DSLWrapper<dsl::DSLExpression>> expression();

    dsl::DSLOptional<dsl::DSLWrapper<dsl::DSLExpression>> assignmentExpression();

    dsl::DSLOptional<dsl::DSLWrapper<dsl::DSLExpression>> ternaryExpression();

    dsl::DSLOptional<dsl::DSLWrapper<dsl::DSLExpression>> logicalOrExpression();

    dsl::DSLOptional<dsl::DSLWrapper<dsl::DSLExpression>> logicalXorExpression();

    dsl::DSLOptional<dsl::DSLWrapper<dsl::DSLExpression>> logicalAndExpression();

    dsl::DSLOptional<dsl::DSLWrapper<dsl::DSLExpression>> bitwiseOrExpression();

    dsl::DSLOptional<dsl::DSLWrapper<dsl::DSLExpression>> bitwiseXorExpression();

    dsl::DSLOptional<dsl::DSLWrapper<dsl::DSLExpression>> bitwiseAndExpression();

    dsl::DSLOptional<dsl::DSLWrapper<dsl::DSLExpression>> equalityExpression();

    dsl::DSLOptional<dsl::DSLWrapper<dsl::DSLExpression>> relationalExpression();

    dsl::DSLOptional<dsl::DSLWrapper<dsl::DSLExpression>> shiftExpression();

    dsl::DSLOptional<dsl::DSLWrapper<dsl::DSLExpression>> additiveExpression();

    dsl::DSLOptional<dsl::DSLWrapper<dsl::DSLExpression>> multiplicativeExpression();

    dsl::DSLOptional<dsl::DSLWrapper<dsl::DSLExpression>> unaryExpression();

    dsl::DSLOptional<dsl::DSLWrapper<dsl::DSLExpression>> postfixExpression();

    dsl::DSLOptional<dsl::Wrapper<dsl::DSLExpression>> swizzle(int offset,
            dsl::DSLOptional<dsl::Wrapper<dsl::DSLExpression>> base, const char* swizzleMask);

    dsl::DSLOptional<dsl::Wrapper<dsl::DSLExpression>> call(int offset,
            dsl::DSLOptional<dsl::Wrapper<dsl::DSLExpression>> base,
            SkTArray<dsl::Wrapper<dsl::DSLExpression>> args);

    dsl::DSLOptional<dsl::Wrapper<dsl::DSLExpression>> suffix(
            dsl::DSLOptional<dsl::Wrapper<dsl::DSLExpression>> base);

    dsl::DSLOptional<dsl::Wrapper<dsl::DSLExpression>> term();

    bool intLiteral(SKSL_INT* dest);

    bool floatLiteral(SKSL_FLOAT* dest);

    bool boolLiteral(bool* dest);

    bool identifier(StringFragment* dest);

    class Checkpoint : public ErrorReporter {
    public:
        Checkpoint(DSLParser* p) : fParser(p) {
            fPushbackCheckpoint = fParser->fPushback;
            fLexerCheckpoint = fParser->fLexer.getCheckpoint();
            fErrorReporter = fParser->fErrorReporter;
            fParser->fErrorReporter = this;
        }

        ~Checkpoint() override {
            SkASSERTF(!fErrorReporter, "Checkpoint was not accepted or rewound before destruction");
        }

        void accept() {
            fParser->fErrorReporter = fErrorReporter;
            fErrorReporter = nullptr;
        }

        void rewind() {
            fParser->fPushback = fPushbackCheckpoint;
            fParser->fLexer.rewindToCheckpoint(fLexerCheckpoint);
            fParser->fErrorReporter = fErrorReporter;
            fErrorReporter = nullptr;
        }

        void error(int offset, String msg) override {
            ++fErrorCount;
        }

        int errorCount() override {
            return fErrorCount;
        }

        void setErrorCount(int numErrors) override {
            SkUNREACHABLE;
        }

    private:
        DSLParser* fParser;
        Token fPushbackCheckpoint;
        int32_t fLexerCheckpoint;
        ErrorReporter* fErrorReporter;
        int fErrorCount = 0;
    };

    static std::unordered_map<String, LayoutToken>* layoutTokens;

    Compiler& fCompiler;
    int fFlags;
    ErrorReporter* fErrorReporter;
    ProgramKind fKind;
    std::unique_ptr<ProgramConfig> fConfig;
    StringFragment fText;
    Lexer fLexer;
    // current parse depth, used to enforce a recursion limit to try to keep us from overflowing the
    // stack on pathological inputs
    int fDepth = 0;
    Token fPushback;

    friend class AutoDSLDepth;
    friend class HCodeGenerator;
};

}  // namespace SkSL

#endif
