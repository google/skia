/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_PARSER
#define SKSL_PARSER

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLLexer.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/SkSLPosition.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLModifiers.h"

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace SkSL {

class Compiler;
class ErrorReporter;
class Expression;
class FunctionDeclaration;
struct Module;
struct Program;
enum class ProgramKind : int8_t;
class Statement;
class SymbolTable;
class Type;
class VarDeclaration;
class Variable;

/**
 * Consumes .sksl text and converts it into an IR tree, encapsulated in a Program.
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
    class Checkpoint;

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

    void extensionDirective(Position start);

    void versionDirective(Position start, bool allowVersion);

    bool declaration();

    bool functionDeclarationEnd(Position start,
                                Modifiers& modifiers,
                                const Type* returnType,
                                const Token& name);

    bool prototypeFunction(SkSL::FunctionDeclaration* decl);

    bool defineFunction(SkSL::FunctionDeclaration* decl);

    struct VarDeclarationsPrefix {
        Position fPosition;
        Modifiers fModifiers;
        const Type* fType;
        Token fName;
    };

    bool varDeclarationsPrefix(VarDeclarationsPrefix* prefixData);

    std::unique_ptr<Statement> varDeclarationsOrExpressionStatement();

    std::unique_ptr<Statement> varDeclarations();

    const Type* structDeclaration();

    void structVarDeclaration(Position start, const Modifiers& modifiers);

    bool allowUnsizedArrays() {
        return ProgramConfig::IsCompute(fKind) || ProgramConfig::IsFragment(fKind) ||
               ProgramConfig::IsVertex(fKind);
    }

    const Type* arrayType(const Type* base, int count, Position pos);

    const Type* unsizedArrayType(const Type* base, Position pos);

    bool parseArrayDimensions(Position pos, const Type** type);

    bool parseInitializer(Position pos, std::unique_ptr<Expression>* initializer);

    void addGlobalVarDeclaration(std::unique_ptr<SkSL::VarDeclaration> decl);

    void globalVarDeclarationEnd(Position position, const Modifiers& mods,
                                 const Type* baseType, Token name);

    std::unique_ptr<Statement> localVarDeclarationEnd(Position position,
                                                      const Modifiers& mods,
                                                      const Type* baseType,
                                                      Token name);

    bool modifiersDeclarationEnd(const Modifiers& mods);

    bool parameter(std::unique_ptr<SkSL::Variable>* outParam);

    int layoutInt();

    std::string_view layoutIdentifier();

    SkSL::Layout layout();

    Modifiers modifiers();

    std::unique_ptr<Statement> statementOrNop(Position pos, std::unique_ptr<Statement> stmt);

    std::unique_ptr<Statement> statement();

    const Type* findType(Position pos, Modifiers* modifiers, std::string_view name);

    const Type* type(Modifiers* modifiers);

    bool interfaceBlock(const Modifiers& mods);

    std::unique_ptr<Statement> ifStatement();

    std::unique_ptr<Statement> doStatement();

    std::unique_ptr<Statement> whileStatement();

    std::unique_ptr<Statement> forStatement();

    bool switchCaseBody(ExpressionArray* values,
                        StatementArray* caseBlocks,
                        std::unique_ptr<Expression> value);

    bool switchCase(ExpressionArray* values, StatementArray* caseBlocks);

    std::unique_ptr<Statement> switchStatement();

    std::unique_ptr<Statement> returnStatement();

    std::unique_ptr<Statement> breakStatement();

    std::unique_ptr<Statement> continueStatement();

    std::unique_ptr<Statement> discardStatement();

    std::unique_ptr<Statement> block();

    std::unique_ptr<Statement> expressionStatement();

    using BinaryParseFn = std::unique_ptr<Expression> (Parser::*)();
    [[nodiscard]] bool operatorRight(AutoDepth& depth,
                                     Operator::Kind op,
                                     BinaryParseFn rightFn,
                                     std::unique_ptr<Expression>& expr);

    std::unique_ptr<Expression> poison(Position pos);

    std::unique_ptr<Expression> expressionOrPoison(Position pos, std::unique_ptr<Expression> expr);

    std::unique_ptr<Expression> expression();

    std::unique_ptr<Expression> assignmentExpression();

    std::unique_ptr<Expression> ternaryExpression();

    std::unique_ptr<Expression> logicalOrExpression();

    std::unique_ptr<Expression> logicalXorExpression();

    std::unique_ptr<Expression> logicalAndExpression();

    std::unique_ptr<Expression> bitwiseOrExpression();

    std::unique_ptr<Expression> bitwiseXorExpression();

    std::unique_ptr<Expression> bitwiseAndExpression();

    std::unique_ptr<Expression> equalityExpression();

    std::unique_ptr<Expression> relationalExpression();

    std::unique_ptr<Expression> shiftExpression();

    std::unique_ptr<Expression> additiveExpression();

    std::unique_ptr<Expression> multiplicativeExpression();

    std::unique_ptr<Expression> unaryExpression();

    std::unique_ptr<Expression> postfixExpression();

    std::unique_ptr<Expression> swizzle(Position pos,
                                        std::unique_ptr<Expression> base,
                                        std::string_view swizzleMask,
                                        Position maskPos);

    std::unique_ptr<Expression> call(Position pos,
                                     std::unique_ptr<Expression> base,
                                     ExpressionArray args);

    std::unique_ptr<Expression> suffix(std::unique_ptr<Expression> base);

    std::unique_ptr<Expression> term();

    bool intLiteral(SKSL_INT* dest);

    bool floatLiteral(SKSL_FLOAT* dest);

    bool boolLiteral(bool* dest);

    bool identifier(std::string_view* dest);

    std::shared_ptr<SymbolTable>& symbolTable();

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
