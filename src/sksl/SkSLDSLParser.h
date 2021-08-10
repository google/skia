/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSLPARSER
#define SKSL_DSLPARSER

#include <memory>
#include <unordered_map>
#include "include/core/SkStringView.h"
#include "include/private/SkSLProgramKind.h"
#include "include/private/SkTOptional.h"
#include "include/sksl/DSL.h"
#include "include/sksl/DSLSymbols.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLLexer.h"
#include "src/sksl/ir/SkSLProgram.h"

#if SKSL_DSL_PARSER

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

    DSLParser(Compiler* compiler, const ProgramSettings& settings, ProgramKind kind,
              String text);

    std::unique_ptr<Program> program();

    skstd::string_view text(Token token);

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
        return *dsl::CurrentSymbolTable();
    }

    // these functions parse individual grammar rules from the current parse position; you probably
    // don't need to call any of these outside of the parser. The function declarations in the .cpp
    // file have comments describing the grammar rules.

    ASTNode::ID precision();

    bool declaration();

    bool functionDeclarationEnd(const dsl::DSLModifiers& modifiers,
                                dsl::DSLType type,
                                const Token& name);

    struct VarDeclarationsPrefix {
        dsl::DSLModifiers modifiers;
        dsl::DSLType type = dsl::DSLType(dsl::kVoid_Type);
        Token name;
    };

    bool varDeclarationsPrefix(VarDeclarationsPrefix* prefixData);

    skstd::optional<dsl::DSLStatement> varDeclarationsOrExpressionStatement();

    skstd::optional<dsl::DSLStatement> varDeclarations();

    skstd::optional<dsl::DSLType> structDeclaration();

    SkTArray<dsl::DSLGlobalVar> structVarDeclaration(const dsl::DSLModifiers& modifiers);

    /* (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)? (COMMA IDENTIFER
       (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)?)* SEMICOLON */
    template<class T>
    SkTArray<T> varDeclarationEnd(const dsl::DSLModifiers& mods, dsl::DSLType baseType,
                                  skstd::string_view name);

    SkTArray<dsl::DSLGlobalVar> globalVarDeclarationEnd(const dsl::DSLModifiers& modifiers,
                                                        dsl::DSLType type, skstd::string_view name);

    skstd::optional<dsl::DSLWrapper<dsl::DSLParameter>> parameter();

    int layoutInt();

    skstd::string_view layoutIdentifier();

    dsl::DSLLayout layout();

    dsl::DSLModifiers modifiers();

    dsl::DSLModifiers modifiersWithDefaults(int defaultFlags);

    skstd::optional<dsl::DSLStatement> statement();

    skstd::optional<dsl::DSLType> type(const dsl::DSLModifiers& modifiers);

    bool interfaceBlock(const dsl::DSLModifiers& mods);

    skstd::optional<dsl::DSLStatement> ifStatement();

    skstd::optional<dsl::DSLStatement> doStatement();

    skstd::optional<dsl::DSLStatement> whileStatement();

    skstd::optional<dsl::DSLStatement> forStatement();

    skstd::optional<dsl::DSLCase> switchCase();

    skstd::optional<dsl::DSLStatement> switchStatement();

    skstd::optional<dsl::DSLStatement> returnStatement();

    skstd::optional<dsl::DSLStatement> breakStatement();

    skstd::optional<dsl::DSLStatement> continueStatement();

    skstd::optional<dsl::DSLStatement> discardStatement();

    skstd::optional<dsl::DSLBlock> block();

    skstd::optional<dsl::DSLStatement> expressionStatement();

    skstd::optional<dsl::DSLWrapper<dsl::DSLExpression>> expression();

    skstd::optional<dsl::DSLWrapper<dsl::DSLExpression>> assignmentExpression();

    skstd::optional<dsl::DSLWrapper<dsl::DSLExpression>> ternaryExpression();

    skstd::optional<dsl::DSLWrapper<dsl::DSLExpression>> logicalOrExpression();

    skstd::optional<dsl::DSLWrapper<dsl::DSLExpression>> logicalXorExpression();

    skstd::optional<dsl::DSLWrapper<dsl::DSLExpression>> logicalAndExpression();

    skstd::optional<dsl::DSLWrapper<dsl::DSLExpression>> bitwiseOrExpression();

    skstd::optional<dsl::DSLWrapper<dsl::DSLExpression>> bitwiseXorExpression();

    skstd::optional<dsl::DSLWrapper<dsl::DSLExpression>> bitwiseAndExpression();

    skstd::optional<dsl::DSLWrapper<dsl::DSLExpression>> equalityExpression();

    skstd::optional<dsl::DSLWrapper<dsl::DSLExpression>> relationalExpression();

    skstd::optional<dsl::DSLWrapper<dsl::DSLExpression>> shiftExpression();

    skstd::optional<dsl::DSLWrapper<dsl::DSLExpression>> additiveExpression();

    skstd::optional<dsl::DSLWrapper<dsl::DSLExpression>> multiplicativeExpression();

    skstd::optional<dsl::DSLWrapper<dsl::DSLExpression>> unaryExpression();

    skstd::optional<dsl::DSLWrapper<dsl::DSLExpression>> postfixExpression();

    skstd::optional<dsl::Wrapper<dsl::DSLExpression>> swizzle(int offset, dsl::DSLExpression base,
            skstd::string_view swizzleMask);

    skstd::optional<dsl::Wrapper<dsl::DSLExpression>> call(int offset, dsl::DSLExpression base,
            SkTArray<dsl::Wrapper<dsl::DSLExpression>> args);

    skstd::optional<dsl::Wrapper<dsl::DSLExpression>> suffix(dsl::DSLExpression base);

    skstd::optional<dsl::Wrapper<dsl::DSLExpression>> term();

    bool intLiteral(SKSL_INT* dest);

    bool floatLiteral(SKSL_FLOAT* dest);

    bool boolLiteral(bool* dest);

    bool identifier(skstd::string_view* dest);

    class Checkpoint {
    public:
        Checkpoint(DSLParser* p) : fParser(p) {
            fPushbackCheckpoint = fParser->fPushback;
            fLexerCheckpoint = fParser->fLexer.getCheckpoint();
            fOldErrorHandler = dsl::GetErrorHandler();
            SkASSERT(fOldErrorHandler);
            dsl::SetErrorHandler(&fErrorHandler);
        }

        ~Checkpoint() {
            SkASSERTF(!fOldErrorHandler,
                      "Checkpoint was not accepted or rewound before destruction");
        }

        void accept() {
            this->restoreErrorHandler();
            // Parser errors should have been fatal, but we can encounter other errors like type
            // mismatches despite accepting the parse. Forward those messages to the actual error
            // handler now.
            fErrorHandler.forwardErrors();
        }

        void rewind() {
            this->restoreErrorHandler();
            fParser->fPushback = fPushbackCheckpoint;
            fParser->fLexer.rewindToCheckpoint(fLexerCheckpoint);
        }

    private:
        class ForwardingErrorHandler : public dsl::ErrorHandler {
        public:
            void handleError(const char* msg, dsl::PositionInfo pos) override {
                fErrors.push_back({String(msg), pos});
            }

            void forwardErrors() {
                for (Error& error : fErrors) {
                    dsl::GetErrorHandler()->handleError(error.fMsg.c_str(), error.fPos);
                }
            }

        private:
            struct Error {
                String fMsg;
                dsl::PositionInfo fPos;
            };

            SkTArray<Error> fErrors;
        };

        void restoreErrorHandler() {
            SkASSERT(fOldErrorHandler);
            dsl::SetErrorHandler(fOldErrorHandler);
            fOldErrorHandler = nullptr;
        }

        DSLParser* fParser;
        Token fPushbackCheckpoint;
        int32_t fLexerCheckpoint;
        ForwardingErrorHandler fErrorHandler;
        dsl::ErrorHandler* fOldErrorHandler;
    };

    static std::unordered_map<skstd::string_view, LayoutToken>* layoutTokens;

    Compiler& fCompiler;
    ProgramSettings fSettings;
    ErrorReporter* fErrorReporter;
    ProgramKind fKind;
    String fText;
    Lexer fLexer;
    // current parse depth, used to enforce a recursion limit to try to keep us from overflowing the
    // stack on pathological inputs
    int fDepth = 0;
    Token fPushback;

    friend class AutoDSLDepth;
    friend class HCodeGenerator;
};

}  // namespace SkSL

#endif // SKSL_DSL_PARSER

#endif
