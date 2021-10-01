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
#include "src/sksl/SkSLLexer.h"
#include "src/sksl/ir/SkSLProgram.h"

namespace SkSL {

class ErrorReporter;
struct Modifiers;
struct ParsedModule;
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

    SkSL::LoadedModule moduleInheritingFrom(SkSL::ParsedModule baseModule);

    skstd::string_view text(Token token);

    PositionInfo position(Token token);

    PositionInfo position(int line);

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

    void error(Token token, String msg);
    void error(int line, String msg);

    SymbolTable& symbols() {
        return *dsl::CurrentSymbolTable();
    }

    // these functions parse individual grammar rules from the current parse position; you probably
    // don't need to call any of these outside of the parser. The function declarations in the .cpp
    // file have comments describing the grammar rules.

    void declarations();

    SKSL_INT arraySize();

    void directive();

    bool declaration();

    bool functionDeclarationEnd(const dsl::DSLModifiers& modifiers,
                                dsl::DSLType type,
                                const Token& name);

    struct VarDeclarationsPrefix {
        PositionInfo fPosition;
        dsl::DSLModifiers fModifiers;
        dsl::DSLType fType = dsl::DSLType(dsl::kVoid_Type);
        Token fName;
    };

    bool varDeclarationsPrefix(VarDeclarationsPrefix* prefixData);

    dsl::DSLStatement varDeclarationsOrExpressionStatement();

    dsl::DSLStatement varDeclarations();

    skstd::optional<dsl::DSLType> structDeclaration();

    SkTArray<dsl::DSLGlobalVar> structVarDeclaration(const dsl::DSLModifiers& modifiers);

    bool parseArrayDimensions(int line, dsl::DSLType* type);

    bool parseInitializer(int line, dsl::DSLExpression* initializer);

    void globalVarDeclarationEnd(PositionInfo position, const dsl::DSLModifiers& mods,
            dsl::DSLType baseType, skstd::string_view name);

    dsl::DSLStatement localVarDeclarationEnd(PositionInfo position, const dsl::DSLModifiers& mods,
            dsl::DSLType baseType, skstd::string_view name);

    skstd::optional<dsl::DSLWrapper<dsl::DSLParameter>> parameter();

    int layoutInt();

    skstd::string_view layoutIdentifier();

    dsl::DSLLayout layout();

    dsl::DSLModifiers modifiers();

    dsl::DSLStatement statement();

    skstd::optional<dsl::DSLType> type(dsl::DSLModifiers* modifiers);

    bool interfaceBlock(const dsl::DSLModifiers& mods);

    dsl::DSLStatement ifStatement();

    dsl::DSLStatement doStatement();

    dsl::DSLStatement whileStatement();

    dsl::DSLStatement forStatement();

    skstd::optional<dsl::DSLCase> switchCase();

    dsl::DSLStatement switchStatement();

    dsl::DSLStatement returnStatement();

    dsl::DSLStatement breakStatement();

    dsl::DSLStatement continueStatement();

    dsl::DSLStatement discardStatement();

    skstd::optional<dsl::DSLBlock> block();

    dsl::DSLStatement expressionStatement();

    dsl::DSLExpression expression();

    dsl::DSLExpression assignmentExpression();

    dsl::DSLExpression ternaryExpression();

    dsl::DSLExpression logicalOrExpression();

    dsl::DSLExpression logicalXorExpression();

    dsl::DSLExpression logicalAndExpression();

    dsl::DSLExpression bitwiseOrExpression();

    dsl::DSLExpression bitwiseXorExpression();

    dsl::DSLExpression bitwiseAndExpression();

    dsl::DSLExpression equalityExpression();

    dsl::DSLExpression relationalExpression();

    dsl::DSLExpression shiftExpression();

    dsl::DSLExpression additiveExpression();

    dsl::DSLExpression multiplicativeExpression();

    dsl::DSLExpression unaryExpression();

    dsl::DSLExpression postfixExpression();

    dsl::DSLExpression swizzle(int line, dsl::DSLExpression base, skstd::string_view swizzleMask);

    dsl::DSLExpression call(int line, dsl::DSLExpression base, ExpressionArray args);

    dsl::DSLExpression suffix(dsl::DSLExpression base);

    dsl::DSLExpression term();

    bool intLiteral(SKSL_INT* dest);

    bool floatLiteral(SKSL_FLOAT* dest);

    bool boolLiteral(bool* dest);

    bool identifier(skstd::string_view* dest);

    class Checkpoint {
    public:
        Checkpoint(DSLParser* p) : fParser(p) {
            fPushbackCheckpoint = fParser->fPushback;
            fLexerCheckpoint = fParser->fLexer.getCheckpoint();
            fOldErrorReporter = &dsl::GetErrorReporter();
            fOldEncounteredFatalError = fParser->fEncounteredFatalError;
            SkASSERT(fOldErrorReporter);
            dsl::SetErrorReporter(&fErrorReporter);
        }

        ~Checkpoint() {
            SkASSERTF(!fOldErrorReporter,
                      "Checkpoint was not accepted or rewound before destruction");
        }

        void accept() {
            this->restoreErrorReporter();
            // Parser errors should have been fatal, but we can encounter other errors like type
            // mismatches despite accepting the parse. Forward those messages to the actual error
            // handler now.
            fErrorReporter.forwardErrors();
        }

        void rewind() {
            this->restoreErrorReporter();
            fParser->fPushback = fPushbackCheckpoint;
            fParser->fLexer.rewindToCheckpoint(fLexerCheckpoint);
            fParser->fEncounteredFatalError = fOldEncounteredFatalError;
        }

    private:
        class ForwardingErrorReporter : public ErrorReporter {
        public:
            void handleError(skstd::string_view msg, PositionInfo pos) override {
                fErrors.push_back({String(msg), pos});
            }

            void forwardErrors() {
                for (Error& error : fErrors) {
                    dsl::GetErrorReporter().error(error.fMsg.c_str(), error.fPos);
                }
            }

        private:
            struct Error {
                String fMsg;
                PositionInfo fPos;
            };

            SkTArray<Error> fErrors;
        };

        void restoreErrorReporter() {
            SkASSERT(fOldErrorReporter);
            fErrorReporter.reportPendingErrors(PositionInfo());
            dsl::SetErrorReporter(fOldErrorReporter);
            fOldErrorReporter = nullptr;
        }

        DSLParser* fParser;
        Token fPushbackCheckpoint;
        SkSL::Lexer::Checkpoint fLexerCheckpoint;
        ForwardingErrorReporter fErrorReporter;
        ErrorReporter* fOldErrorReporter;
        bool fOldEncounteredFatalError;
    };

    static std::unordered_map<skstd::string_view, LayoutToken>* layoutTokens;

    Compiler& fCompiler;
    ProgramSettings fSettings;
    ErrorReporter* fErrorReporter;
    bool fEncounteredFatalError;
    ProgramKind fKind;
    std::unique_ptr<String> fText;
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
