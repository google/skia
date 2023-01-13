/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PARSER
#define SKSL_PARSER

#include "include/core/SkTypes.h"
#include "include/private/SkSLDefines.h"
#include "include/private/base/SkTArray.h"
#include "include/sksl/DSLCore.h"
#include "include/sksl/DSLExpression.h"
#include "include/sksl/DSLLayout.h"
#include "include/sksl/DSLModifiers.h"
#include "include/sksl/DSLStatement.h"
#include "include/sksl/DSLType.h"
#include "include/sksl/SkSLErrorReporter.h"
#include "include/sksl/SkSLOperator.h"
#include "include/sksl/SkSLPosition.h"
#include "src/sksl/SkSLLexer.h"
#include "src/sksl/SkSLProgramSettings.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

namespace SkSL {

class Compiler;
class SymbolTable;
enum class ProgramKind : int8_t;
struct Module;
struct Program;

namespace dsl {
class DSLBlock;
class DSLCase;
class DSLGlobalVar;
class DSLParameter;
class DSLVarBase;
}

/**
 * Consumes .sksl text and invokes DSL functions to instantiate the program.
 */
class Parser {
public:
    Parser(Compiler* compiler, const ProgramSettings& settings, ProgramKind kind, std::string text);

    std::unique_ptr<Program> program();

    std::unique_ptr<Module> moduleInheritingFrom(const Module* parent);

    std::string_view text(Token token);

    Position position(Token token);

private:
    class AutoDepth;
    class AutoSymbolTable;

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
     * Behaves like checkNext(TK_IDENTIFIER), but also verifies that identifier is not a builtin
     * type. If the token was actually a builtin type, false is returned (the next token is not
     * considered to be an identifier).
     */
    bool checkIdentifier(Token* result = nullptr);

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
    bool expect(Token::Kind kind, std::string expected, Token* result = nullptr);

    /**
     * Behaves like expect(TK_IDENTIFIER), but also verifies that identifier is not a type.
     * If the token was actually a type, generates an error message of the form:
     *
     * "expected an identifier, but found type 'float2'"
     */
    bool expectIdentifier(Token* result);

    /** If the next token is a newline, consumes it and returns true. If not, returns false. */
    bool expectNewline();

    void error(Token token, std::string_view msg);
    void error(Position position, std::string_view msg);

    // Returns the range from `start` to the current parse position.
    Position rangeFrom(Position start);
    Position rangeFrom(Token start);

    // these functions parse individual grammar rules from the current parse position; you probably
    // don't need to call any of these outside of the parser. The function declarations in the .cpp
    // file have comments describing the grammar rules.

    void declarations();

    /**
     * Parses an expression representing an array size. Reports errors if the array size is not
     * valid (out of bounds, not a literal integer). Returns true if an expression was
     * successfully parsed, even if that array size is not actually valid. In the event of a true
     * return, outResult always contains a valid array size (even if the parsed array size was not
     * actually valid; invalid array sizes result in a 1 to avoid additional errors downstream).
     */
    bool arraySize(SKSL_INT* outResult);

    void directive(bool allowVersion);

    bool declaration();

    bool functionDeclarationEnd(Position start,
                                dsl::DSLModifiers& modifiers,
                                dsl::DSLType type,
                                const Token& name);

    struct VarDeclarationsPrefix {
        Position fPosition;
        dsl::DSLModifiers fModifiers;
        dsl::DSLType fType = dsl::DSLType(dsl::kVoid_Type);
        Token fName;
    };

    bool varDeclarationsPrefix(VarDeclarationsPrefix* prefixData);

    dsl::DSLStatement varDeclarationsOrExpressionStatement();

    dsl::DSLStatement varDeclarations();

    dsl::DSLType structDeclaration();

    SkTArray<dsl::DSLGlobalVar> structVarDeclaration(Position start,
                                                     const dsl::DSLModifiers& modifiers);

    bool allowUnsizedArrays() {
        return ProgramConfig::IsCompute(fKind) || ProgramConfig::IsFragment(fKind) ||
               ProgramConfig::IsVertex(fKind);
    }

    bool parseArrayDimensions(Position pos, dsl::DSLType* type);

    bool parseInitializer(Position pos, dsl::DSLExpression* initializer);

    void globalVarDeclarationEnd(Position position, const dsl::DSLModifiers& mods,
            dsl::DSLType baseType, Token name);

    dsl::DSLStatement localVarDeclarationEnd(Position position, const dsl::DSLModifiers& mods,
            dsl::DSLType baseType, Token name);

    std::optional<dsl::DSLParameter> parameter(size_t paramIndex);

    int layoutInt();

    std::string_view layoutIdentifier();

    dsl::DSLLayout layout();

    dsl::DSLModifiers modifiers();

    dsl::DSLStatement statement();

    dsl::DSLType type(dsl::DSLModifiers* modifiers);

    bool interfaceBlock(const dsl::DSLModifiers& mods);

    dsl::DSLStatement ifStatement();

    dsl::DSLStatement doStatement();

    dsl::DSLStatement whileStatement();

    dsl::DSLStatement forStatement();

    std::optional<dsl::DSLCase> switchCase();

    dsl::DSLStatement switchStatement();

    dsl::DSLStatement returnStatement();

    dsl::DSLStatement breakStatement();

    dsl::DSLStatement continueStatement();

    dsl::DSLStatement discardStatement();

    std::optional<dsl::DSLBlock> block();

    dsl::DSLStatement expressionStatement();

    using BinaryParseFn = dsl::DSLExpression (Parser::*)();
    bool SK_WARN_UNUSED_RESULT operatorRight(AutoDepth& depth, Operator::Kind op,
                                             BinaryParseFn rightFn, dsl::DSLExpression& result);

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

    dsl::DSLExpression swizzle(Position pos, dsl::DSLExpression base,
            std::string_view swizzleMask, Position maskPos);

    dsl::DSLExpression call(Position pos, dsl::DSLExpression base, ExpressionArray args);

    dsl::DSLExpression suffix(dsl::DSLExpression base);

    dsl::DSLExpression term();

    bool intLiteral(SKSL_INT* dest);

    bool floatLiteral(SKSL_FLOAT* dest);

    bool boolLiteral(bool* dest);

    bool identifier(std::string_view* dest);

    std::shared_ptr<SymbolTable>& symbolTable();

    void addToSymbolTable(dsl::DSLVarBase& var, Position pos = {});

    class Checkpoint {
    public:
        Checkpoint(Parser* p) : fParser(p) {
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
            void handleError(std::string_view msg, Position pos) override {
                fErrors.push_back({std::string(msg), pos});
            }

            void forwardErrors() {
                for (Error& error : fErrors) {
                    dsl::GetErrorReporter().error(error.fPos, error.fMsg);
                }
            }

        private:
            struct Error {
                std::string fMsg;
                Position fPos;
            };

            SkTArray<Error> fErrors;
        };

        void restoreErrorReporter() {
            SkASSERT(fOldErrorReporter);
            dsl::SetErrorReporter(fOldErrorReporter);
            fOldErrorReporter = nullptr;
        }

        Parser* fParser;
        Token fPushbackCheckpoint;
        SkSL::Lexer::Checkpoint fLexerCheckpoint;
        ForwardingErrorReporter fErrorReporter;
        ErrorReporter* fOldErrorReporter;
        bool fOldEncounteredFatalError;
    };

    Compiler& fCompiler;
    ProgramSettings fSettings;
    ErrorReporter* fErrorReporter;
    bool fEncounteredFatalError;
    ProgramKind fKind;
    std::unique_ptr<std::string> fText;
    Lexer fLexer;
    // current parse depth, used to enforce a recursion limit to try to keep us from overflowing the
    // stack on pathological inputs
    int fDepth = 0;
    Token fPushback;
};

}  // namespace SkSL

#endif
