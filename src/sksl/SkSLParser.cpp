/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLParser.h"

#include "include/core/SkSpan.h"
#include "include/private/SkSLModifiers.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLString.h"
#include "include/sksl/DSLBlock.h"
#include "include/sksl/DSLCase.h"
#include "include/sksl/DSLFunction.h"
#include "include/sksl/DSLVar.h"
#include "include/sksl/SkSLOperator.h"
#include "include/sksl/SkSLVersion.h"
#include "src/core/SkTHash.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/dsl/priv/DSL_priv.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <algorithm>
#include <climits>
#include <initializer_list>
#include <memory>
#include <utility>
#include <vector>

using namespace SkSL::dsl;

namespace SkSL {

static constexpr int kMaxParseDepth = 50;

static int parse_modifier_token(Token::Kind token) {
    switch (token) {
        case Token::Kind::TK_UNIFORM:        return Modifiers::kUniform_Flag;
        case Token::Kind::TK_CONST:          return Modifiers::kConst_Flag;
        case Token::Kind::TK_IN:             return Modifiers::kIn_Flag;
        case Token::Kind::TK_OUT:            return Modifiers::kOut_Flag;
        case Token::Kind::TK_INOUT:          return Modifiers::kIn_Flag | Modifiers::kOut_Flag;
        case Token::Kind::TK_FLAT:           return Modifiers::kFlat_Flag;
        case Token::Kind::TK_NOPERSPECTIVE:  return Modifiers::kNoPerspective_Flag;
        case Token::Kind::TK_PURE:           return Modifiers::kPure_Flag;
        case Token::Kind::TK_INLINE:         return Modifiers::kInline_Flag;
        case Token::Kind::TK_NOINLINE:       return Modifiers::kNoInline_Flag;
        case Token::Kind::TK_HIGHP:          return Modifiers::kHighp_Flag;
        case Token::Kind::TK_MEDIUMP:        return Modifiers::kMediump_Flag;
        case Token::Kind::TK_LOWP:           return Modifiers::kLowp_Flag;
        case Token::Kind::TK_EXPORT:         return Modifiers::kExport_Flag;
        case Token::Kind::TK_ES3:            return Modifiers::kES3_Flag;
        case Token::Kind::TK_WORKGROUP:      return Modifiers::kWorkgroup_Flag;
        case Token::Kind::TK_READONLY:       return Modifiers::kReadOnly_Flag;
        case Token::Kind::TK_WRITEONLY:      return Modifiers::kWriteOnly_Flag;
        case Token::Kind::TK_BUFFER:         return Modifiers::kBuffer_Flag;
        default:                             return 0;
    }
}

class Parser::AutoDepth {
public:
    AutoDepth(Parser* p)
    : fParser(p)
    , fDepth(0) {}

    ~AutoDepth() {
        fParser->fDepth -= fDepth;
    }

    bool increase() {
        ++fDepth;
        ++fParser->fDepth;
        if (fParser->fDepth > kMaxParseDepth) {
            fParser->error(fParser->peek(), "exceeded max parse depth");
            fParser->fEncounteredFatalError = true;
            return false;
        }
        return true;
    }

private:
    Parser* fParser;
    int fDepth;
};

class Parser::AutoSymbolTable {
public:
    AutoSymbolTable(Parser* p) : fParser(p) {
        SymbolTable::Push(&fParser->symbolTable());
    }

    ~AutoSymbolTable() {
        SymbolTable::Pop(&fParser->symbolTable());
    }

private:
    Parser* fParser;
};

Parser::Parser(Compiler* compiler,
               const ProgramSettings& settings,
               ProgramKind kind,
               std::string text)
        : fCompiler(*compiler)
        , fSettings(settings)
        , fKind(kind)
        , fText(std::make_unique<std::string>(std::move(text)))
        , fPushback(Token::Kind::TK_NONE, /*offset=*/-1, /*length=*/-1) {
    fLexer.start(*fText);
}

std::shared_ptr<SymbolTable>& Parser::symbolTable() {
    return fCompiler.symbolTable();
}

void Parser::addToSymbolTable(DSLVarBase& var, Position pos) {
    if (SkSL::Variable* skslVar = DSLWriter::Var(var)) {
        this->symbolTable()->addWithoutOwnership(skslVar);
    }
}

Token Parser::nextRawToken() {
    Token token;
    if (fPushback.fKind != Token::Kind::TK_NONE) {
        // Retrieve the token from the pushback buffer.
        token = fPushback;
        fPushback.fKind = Token::Kind::TK_NONE;
    } else {
        // Fetch a token from the lexer.
        token = fLexer.next();

        // Some tokens are always invalid, so we detect and report them here.
        switch (token.fKind) {
            case Token::Kind::TK_PRIVATE_IDENTIFIER:
                if (ProgramConfig::AllowsPrivateIdentifiers(fKind)) {
                    token.fKind = Token::Kind::TK_IDENTIFIER;
                    break;
                }
                [[fallthrough]];

            case Token::Kind::TK_RESERVED:
                this->error(token, "name '" + std::string(this->text(token)) + "' is reserved");
                token.fKind = Token::Kind::TK_IDENTIFIER;  // reduces additional follow-up errors
                break;

            case Token::Kind::TK_BAD_OCTAL:
                this->error(token, "'" + std::string(this->text(token)) +
                                   "' is not a valid octal number");
                break;

            default:
                break;
        }
    }

    return token;
}

static bool is_whitespace(Token::Kind kind) {
    switch (kind) {
        case Token::Kind::TK_WHITESPACE:
        case Token::Kind::TK_LINE_COMMENT:
        case Token::Kind::TK_BLOCK_COMMENT:
            return true;

        default:
            return false;
    }
}

bool Parser::expectNewline() {
    Token token = this->nextRawToken();
    if (token.fKind == Token::Kind::TK_WHITESPACE) {
        // The lexer doesn't distinguish newlines from other forms of whitespace, so we check
        // for newlines by searching through the token text.
        std::string_view tokenText = this->text(token);
        if (tokenText.find_first_of('\r') != std::string_view::npos ||
            tokenText.find_first_of('\n') != std::string_view::npos) {
            return true;
        }
    }
    // We didn't find a newline.
    this->pushback(token);
    return false;
}

Token Parser::nextToken() {
    for (;;) {
        Token token = this->nextRawToken();
        if (!is_whitespace(token.fKind)) {
            return token;
        }
    }
}

void Parser::pushback(Token t) {
    SkASSERT(fPushback.fKind == Token::Kind::TK_NONE);
    fPushback = std::move(t);
}

Token Parser::peek() {
    if (fPushback.fKind == Token::Kind::TK_NONE) {
        fPushback = this->nextToken();
    }
    return fPushback;
}

bool Parser::checkNext(Token::Kind kind, Token* result) {
    if (fPushback.fKind != Token::Kind::TK_NONE && fPushback.fKind != kind) {
        return false;
    }
    Token next = this->nextToken();
    if (next.fKind == kind) {
        if (result) {
            *result = next;
        }
        return true;
    }
    this->pushback(std::move(next));
    return false;
}

bool Parser::expect(Token::Kind kind, const char* expected, Token* result) {
    Token next = this->nextToken();
    if (next.fKind == kind) {
        if (result) {
            *result = std::move(next);
        }
        return true;
    } else {
        this->error(next, "expected " + std::string(expected) + ", but found '" +
                          std::string(this->text(next)) + "'");
        this->fEncounteredFatalError = true;
        return false;
    }
}

bool Parser::expectIdentifier(Token* result) {
    if (!this->expect(Token::Kind::TK_IDENTIFIER, "an identifier", result)) {
        return false;
    }
    if (this->symbolTable()->isBuiltinType(this->text(*result))) {
        this->error(*result, "expected an identifier, but found type '" +
                             std::string(this->text(*result)) + "'");
        this->fEncounteredFatalError = true;
        return false;
    }
    return true;
}

bool Parser::checkIdentifier(Token* result) {
    if (!this->checkNext(Token::Kind::TK_IDENTIFIER, result)) {
        return false;
    }
    if (this->symbolTable()->isBuiltinType(this->text(*result))) {
        this->pushback(std::move(*result));
        return false;
    }
    return true;
}

std::string_view Parser::text(Token token) {
    return std::string_view(fText->data() + token.fOffset, token.fLength);
}

Position Parser::position(Token t) {
    if (t.fOffset >= 0) {
        return Position::Range(t.fOffset, t.fOffset + t.fLength);
    } else {
        return Position();
    }
}

void Parser::error(Token token, std::string_view msg) {
    this->error(this->position(token), msg);
}

void Parser::error(Position position, std::string_view msg) {
    GetErrorReporter().error(position, msg);
}

Position Parser::rangeFrom(Position start) {
    int offset = fPushback.fKind != Token::Kind::TK_NONE ? fPushback.fOffset
                                                         : fLexer.getCheckpoint().fOffset;
    return Position::Range(start.startOffset(), offset);
}

Position Parser::rangeFrom(Token start) {
    return this->rangeFrom(this->position(start));
}

/* declaration* END_OF_FILE */
std::unique_ptr<Program> Parser::program() {
    ErrorReporter* errorReporter = &fCompiler.errorReporter();
    Start(&fCompiler, fKind, fSettings);
    SetErrorReporter(errorReporter);
    errorReporter->setSource(*fText);
    this->declarations();
    std::unique_ptr<Program> result;
    if (!GetErrorReporter().errorCount()) {
        result = dsl::ReleaseProgram(std::move(fText));
    }
    errorReporter->setSource(std::string_view());
    End();
    return result;
}

std::unique_ptr<SkSL::Module> Parser::moduleInheritingFrom(const SkSL::Module* parent) {
    ErrorReporter* errorReporter = &fCompiler.errorReporter();
    StartModule(&fCompiler, fKind, fSettings, parent);
    SetErrorReporter(errorReporter);
    errorReporter->setSource(*fText);
    this->declarations();
    this->symbolTable()->takeOwnershipOfString(std::move(*fText));
    auto result = std::make_unique<SkSL::Module>();
    result->fParent = parent;
    result->fSymbols = this->symbolTable();
    result->fElements = std::move(ThreadContext::ProgramElements());
    errorReporter->setSource(std::string_view());
    End();
    return result;
}

void Parser::declarations() {
    fEncounteredFatalError = false;
    // Any #version directive must appear as the first thing in a file
    if (this->peek().fKind == Token::Kind::TK_DIRECTIVE) {
        this->directive(/*allowVersion=*/true);
    }
    bool done = false;
    while (!done) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_END_OF_FILE:
                done = true;
                break;
            case Token::Kind::TK_DIRECTIVE:
                this->directive(/*allowVersion=*/false);
                break;
            case Token::Kind::TK_INVALID:
                this->error(this->peek(), "invalid token");
                this->nextToken();
                done = true;
                break;
            default:
                this->declaration();
                done = fEncounteredFatalError;
                break;
        }
    }
}

/* DIRECTIVE(#extension) IDENTIFIER COLON IDENTIFIER NEWLINE |
   DIRECTIVE(#version) INTLITERAL NEWLINE */
void Parser::directive(bool allowVersion) {
    Token start;
    if (!this->expect(Token::Kind::TK_DIRECTIVE, "a directive", &start)) {
        return;
    }
    std::string_view text = this->text(start);
    const bool allowExtensions = !ProgramConfig::IsRuntimeEffect(fKind);
    if (text == "#extension" && allowExtensions) {
        Token name;
        if (!this->expectIdentifier(&name)) {
            return;
        }
        if (!this->expect(Token::Kind::TK_COLON, "':'")) {
            return;
        }
        Token behavior;
        if (!this->expect(Token::Kind::TK_IDENTIFIER, "an identifier", &behavior)) {
            return;
        }
        std::string_view behaviorText = this->text(behavior);
        if (behaviorText != "disable") {
            if (behaviorText == "require" || behaviorText == "enable" || behaviorText == "warn") {
                // We don't currently do anything different between require, enable, and warn
                dsl::AddExtension(this->text(name));
            } else {
                this->error(behavior, "expected 'require', 'enable', 'warn', or 'disable'");
            }
        }

        // We expect a newline after an #extension directive.
        if (!this->expectNewline()) {
            this->error(start, "invalid #extension directive");
        }
    } else if (text == "#version") {
        if (!allowVersion) {
            this->error(start, "#version directive must appear before anything else");
            return;
        }
        SKSL_INT version;
        if (!this->intLiteral(&version)) {
            return;
        }
        switch (version) {
            case 100:
                ThreadContext::GetProgramConfig()->fRequiredSkSLVersion = Version::k100;
                break;
            case 300:
                ThreadContext::GetProgramConfig()->fRequiredSkSLVersion = Version::k300;
                break;
            default:
                this->error(start, "unsupported version number");
                return;
        }
        // We expect a newline after a #version directive.
        if (!this->expectNewline()) {
            this->error(start, "invalid #version directive");
        }
    } else {
        this->error(start, "unsupported directive '" + std::string(this->text(start)) + "'");
    }
}

/* modifiers (structVarDeclaration | type IDENTIFIER ((LPAREN parameter (COMMA parameter)* RPAREN
   (block | SEMICOLON)) | SEMICOLON) | interfaceBlock) */
bool Parser::declaration() {
    Token start = this->peek();
    if (start.fKind == Token::Kind::TK_SEMICOLON) {
        this->nextToken();
        this->error(start, "expected a declaration, but found ';'");
        return false;
    }
    DSLModifiers modifiers = this->modifiers();
    Token lookahead = this->peek();
    if (lookahead.fKind == Token::Kind::TK_IDENTIFIER &&
        !this->symbolTable()->isType(this->text(lookahead))) {
        // we have an identifier that's not a type, could be the start of an interface block
        return this->interfaceBlock(modifiers);
    }
    if (lookahead.fKind == Token::Kind::TK_SEMICOLON) {
        this->nextToken();
        Declare(modifiers, this->position(start));
        return true;
    }
    if (lookahead.fKind == Token::Kind::TK_STRUCT) {
        this->structVarDeclaration(this->position(start), modifiers);
        return true;
    }
    DSLType type = this->type(&modifiers);
    if (!type.hasValue()) {
        return false;
    }
    Token name;
    if (!this->expectIdentifier(&name)) {
        return false;
    }
    if (this->checkNext(Token::Kind::TK_LPAREN)) {
        return this->functionDeclarationEnd(this->position(start), modifiers, type, name);
    } else {
        this->globalVarDeclarationEnd(this->position(start), modifiers, type, name);
        return true;
    }
}

/* (RPAREN | VOID RPAREN | parameter (COMMA parameter)* RPAREN) (block | SEMICOLON) */
bool Parser::functionDeclarationEnd(Position start,
                                    DSLModifiers& modifiers,
                                    DSLType type,
                                    const Token& name) {
    SkSTArray<8, DSLParameter> parameters;
    Token lookahead = this->peek();
    if (lookahead.fKind == Token::Kind::TK_RPAREN) {
        // `()` means no parameters at all.
    } else if (lookahead.fKind == Token::Kind::TK_IDENTIFIER && this->text(lookahead) == "void") {
        // `(void)` also means no parameters at all.
        this->nextToken();
    } else {
        for (;;) {
            size_t paramIndex = parameters.size();
            std::optional<DSLParameter> parameter = this->parameter(paramIndex);
            if (!parameter) {
                return false;
            }
            parameters.push_back(std::move(*parameter));
            if (!this->checkNext(Token::Kind::TK_COMMA)) {
                break;
            }
        }
    }
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
        return false;
    }
    SkSTArray<8, DSLParameter*> parameterPointers;
    parameterPointers.reserve_back(parameters.size());
    for (DSLParameter& param : parameters) {
        parameterPointers.push_back(&param);
    }

    DSLFunction result(this->text(name), modifiers, type, parameterPointers,
                       this->rangeFrom(start));

    const bool hasFunctionBody = !this->checkNext(Token::Kind::TK_SEMICOLON);
    if (hasFunctionBody) {
        AutoSymbolTable symbols(this);
        for (DSLParameter* var : parameterPointers) {
            if (!var->name().empty()) {
                this->addToSymbolTable(*var);
            }
        }
        Token bodyStart = this->peek();
        std::optional<DSLBlock> body = this->block();
        if (!body) {
            return false;
        }
        result.define(std::move(*body), this->rangeFrom(bodyStart));
    } else {
        result.prototype();
    }
    return true;
}

bool Parser::arraySize(SKSL_INT* outResult) {
    // Start out with a safe value that won't generate any errors downstream
    *outResult = 1;
    Token next = this->peek();
    if (next.fKind == Token::Kind::TK_RBRACKET) {
        this->error(this->position(next), "unsized arrays are not permitted here");
        return true;
    }
    DSLExpression sizeExpr = this->expression();
    if (!sizeExpr.hasValue()) {
        return false;
    }
    if (sizeExpr.isValid()) {
        std::unique_ptr<SkSL::Expression> sizeLiteral = sizeExpr.release();
        SKSL_INT size;
        if (!ConstantFolder::GetConstantInt(*sizeLiteral, &size)) {
            this->error(sizeLiteral->fPosition, "array size must be an integer");
            return true;
        }
        if (size > INT32_MAX) {
            this->error(sizeLiteral->fPosition, "array size out of bounds");
            return true;
        }
        if (size <= 0) {
            this->error(sizeLiteral->fPosition, "array size must be positive");
            return true;
        }
        // Now that we've validated it, output the real value
        *outResult = size;
    }
    return true;
}

bool Parser::parseArrayDimensions(Position pos, DSLType* type) {
    Token next;
    while (this->checkNext(Token::Kind::TK_LBRACKET, &next)) {
        if (this->checkNext(Token::Kind::TK_RBRACKET)) {
            if (this->allowUnsizedArrays()) {
                *type = UnsizedArray(*type, this->rangeFrom(pos));
            } else {
                this->error(this->rangeFrom(pos), "unsized arrays are not permitted here");
            }
        } else {
            SKSL_INT size;
            if (!this->arraySize(&size)) {
                return false;
            }
            if (!this->expect(Token::Kind::TK_RBRACKET, "']'")) {
                return false;
            }
            *type = Array(*type, size, this->rangeFrom(pos));
        }
    }
    return true;
}

bool Parser::parseInitializer(Position pos, DSLExpression* initializer) {
    if (this->checkNext(Token::Kind::TK_EQ)) {
        DSLExpression value = this->assignmentExpression();
        if (!value.hasValue()) {
            return false;
        }
        initializer->swap(value);
    }
    return true;
}

/* (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)? (COMMA IDENTIFER
   (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)?)* SEMICOLON */
void Parser::globalVarDeclarationEnd(Position pos,
                                     const dsl::DSLModifiers& mods,
                                     dsl::DSLType baseType,
                                     Token name) {
    using namespace dsl;
    DSLType type = baseType;
    DSLExpression initializer;
    if (!this->parseArrayDimensions(pos, &type)) {
        return;
    }
    if (!this->parseInitializer(pos, &initializer)) {
        return;
    }
    DSLGlobalVar first(mods, type, this->text(name), std::move(initializer), this->rangeFrom(pos),
                       this->position(name));
    Declare(first);
    this->addToSymbolTable(first);

    while (this->checkNext(Token::Kind::TK_COMMA)) {
        type = baseType;
        Token identifierName;
        if (!this->expectIdentifier(&identifierName)) {
            return;
        }
        if (!this->parseArrayDimensions(pos, &type)) {
            return;
        }
        DSLExpression anotherInitializer;
        if (!this->parseInitializer(pos, &anotherInitializer)) {
            return;
        }
        DSLGlobalVar next(mods, type, this->text(identifierName), std::move(anotherInitializer),
                          this->rangeFrom(identifierName));
        Declare(next);
        this->addToSymbolTable(next, this->position(identifierName));
    }
    this->expect(Token::Kind::TK_SEMICOLON, "';'");
}

/* (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)? (COMMA IDENTIFER
   (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)?)* SEMICOLON */
DSLStatement Parser::localVarDeclarationEnd(Position pos,
                                            const dsl::DSLModifiers& mods,
                                            dsl::DSLType baseType,
                                            Token name) {
    using namespace dsl;
    DSLType type = baseType;
    DSLExpression initializer;
    if (!this->parseArrayDimensions(pos, &type)) {
        return {};
    }
    if (!this->parseInitializer(pos, &initializer)) {
        return {};
    }
    DSLVar first(mods, type, this->text(name), std::move(initializer), this->rangeFrom(pos),
                 this->position(name));
    DSLStatement result = Declare(first);
    this->addToSymbolTable(first);

    while (this->checkNext(Token::Kind::TK_COMMA)) {
        type = baseType;
        Token identifierName;
        if (!this->expectIdentifier(&identifierName)) {
            return result;
        }
        if (!this->parseArrayDimensions(pos, &type)) {
            return result;
        }
        DSLExpression anotherInitializer;
        if (!this->parseInitializer(pos, &anotherInitializer)) {
            return result;
        }
        DSLVar next(mods, type, this->text(identifierName), std::move(anotherInitializer),
                    this->rangeFrom(identifierName), this->position(identifierName));
        DSLWriter::AddVarDeclaration(result, next);
        this->addToSymbolTable(next, this->position(identifierName));
    }
    this->expect(Token::Kind::TK_SEMICOLON, "';'");
    result.setPosition(this->rangeFrom(pos));
    return result;
}

/* (varDeclarations | expressionStatement) */
DSLStatement Parser::varDeclarationsOrExpressionStatement() {
    Token nextToken = this->peek();
    if (nextToken.fKind == Token::Kind::TK_CONST) {
        // Statements that begin with `const` might be variable declarations, but can't be legal
        // SkSL expression-statements. (SkSL constructors don't take a `const` modifier.)
        return this->varDeclarations();
    }

    if (nextToken.fKind == Token::Kind::TK_HIGHP ||
        nextToken.fKind == Token::Kind::TK_MEDIUMP ||
        nextToken.fKind == Token::Kind::TK_LOWP ||
        this->symbolTable()->isType(this->text(nextToken))) {
        // Statements that begin with a typename are most often variable declarations, but
        // occasionally the type is part of a constructor, and these are actually expression-
        // statements in disguise. First, attempt the common case: parse it as a vardecl.
        Checkpoint checkpoint(this);
        VarDeclarationsPrefix prefix;
        if (this->varDeclarationsPrefix(&prefix)) {
            checkpoint.accept();
            return this->localVarDeclarationEnd(prefix.fPosition, prefix.fModifiers, prefix.fType,
                                                prefix.fName);
        }

        // If this statement wasn't actually a vardecl after all, rewind and try parsing it as an
        // expression-statement instead.
        checkpoint.rewind();
    }
    return this->expressionStatement();
}

// Helper function for varDeclarations(). If this function succeeds, we assume that the rest of the
// statement is a variable-declaration statement, not an expression-statement.
bool Parser::varDeclarationsPrefix(VarDeclarationsPrefix* prefixData) {
    prefixData->fPosition = this->position(this->peek());
    prefixData->fModifiers = this->modifiers();
    prefixData->fType = this->type(&prefixData->fModifiers);
    if (!prefixData->fType.hasValue()) {
        return false;
    }
    return this->expectIdentifier(&prefixData->fName);
}

/* modifiers type IDENTIFIER varDeclarationEnd */
DSLStatement Parser::varDeclarations() {
    VarDeclarationsPrefix prefix;
    if (!this->varDeclarationsPrefix(&prefix)) {
        return {};
    }
    return this->localVarDeclarationEnd(prefix.fPosition, prefix.fModifiers, prefix.fType,
            prefix.fName);
}

/* STRUCT IDENTIFIER LBRACE varDeclaration* RBRACE */
DSLType Parser::structDeclaration() {
    Position start = this->position(this->peek());
    if (!this->expect(Token::Kind::TK_STRUCT, "'struct'")) {
        return DSLType(nullptr);
    }
    Token name;
    if (!this->expectIdentifier(&name)) {
        return DSLType(nullptr);
    }
    if (!this->expect(Token::Kind::TK_LBRACE, "'{'")) {
        return DSLType(nullptr);
    }
    AutoDepth depth(this);
    if (!depth.increase()) {
        return DSLType(nullptr);
    }
    SkTArray<DSLField> fields;
    SkTHashSet<std::string_view> fieldNames;
    while (!this->checkNext(Token::Kind::TK_RBRACE)) {
        Token fieldStart = this->peek();
        DSLModifiers modifiers = this->modifiers();
        DSLType type = this->type(&modifiers);
        if (!type.hasValue()) {
            return DSLType(nullptr);
        }

        do {
            DSLType actualType = type;
            Token memberName;
            if (!this->expectIdentifier(&memberName)) {
                return DSLType(nullptr);
            }

            while (this->checkNext(Token::Kind::TK_LBRACKET)) {
                SKSL_INT size;
                if (!this->arraySize(&size)) {
                    return DSLType(nullptr);
                }
                if (!this->expect(Token::Kind::TK_RBRACKET, "']'")) {
                    return DSLType(nullptr);
                }
                actualType = dsl::Array(actualType, size,
                        this->rangeFrom(this->position(fieldStart)));
            }

            std::string_view nameText = this->text(memberName);
            if (!fieldNames.contains(nameText)) {
                fields.push_back(DSLField(modifiers,
                                          std::move(actualType),
                                          nameText,
                                          this->rangeFrom(fieldStart)));
                fieldNames.add(nameText);
            } else {
                this->error(memberName, "field '" + std::string(nameText) +
                                        "' was already defined in the same struct ('" +
                                        std::string(this->text(name)) + "')");
            }
        } while (this->checkNext(Token::Kind::TK_COMMA));
        if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
            return DSLType(nullptr);
        }
    }
    if (fields.empty()) {
        this->error(this->rangeFrom(start), "struct '" + std::string(this->text(name)) +
                "' must contain at least one field");
    }
    return dsl::Struct(this->text(name), SkSpan(fields), this->rangeFrom(start));
}

/* structDeclaration ((IDENTIFIER varDeclarationEnd) | SEMICOLON) */
SkTArray<dsl::DSLGlobalVar> Parser::structVarDeclaration(Position start,
                                                         const DSLModifiers& modifiers) {
    DSLType type = this->structDeclaration();
    if (!type.hasValue()) {
        return {};
    }
    Token name;
    if (this->checkIdentifier(&name)) {
        this->globalVarDeclarationEnd(this->rangeFrom(name), modifiers, type, name);
    } else {
        this->expect(Token::Kind::TK_SEMICOLON, "';'");
    }
    return {};
}

/* modifiers type IDENTIFIER (LBRACKET INT_LITERAL RBRACKET)? */
std::optional<DSLParameter> Parser::parameter(size_t paramIndex) {
    Position pos = this->position(this->peek());
    DSLModifiers modifiers = this->modifiers();
    DSLType type = this->type(&modifiers);
    if (!type.hasValue()) {
        return std::nullopt;
    }
    Token name;
    std::string_view paramText;
    Position paramPos;
    if (this->checkIdentifier(&name)) {
        paramText = this->text(name);
        paramPos = this->position(name);
    } else {
        paramPos = this->rangeFrom(pos);
    }
    if (!this->parseArrayDimensions(pos, &type)) {
        return std::nullopt;
    }
    return DSLParameter(modifiers, type, paramText, this->rangeFrom(pos), paramPos);
}

/** EQ INT_LITERAL */
int Parser::layoutInt() {
    if (!this->expect(Token::Kind::TK_EQ, "'='")) {
        return -1;
    }
    Token resultToken;
    if (!this->expect(Token::Kind::TK_INT_LITERAL, "a non-negative integer", &resultToken)) {
        return -1;
    }
    std::string_view resultFrag = this->text(resultToken);
    SKSL_INT resultValue;
    if (!SkSL::stoi(resultFrag, &resultValue)) {
        this->error(resultToken, "value in layout is too large: " + std::string(resultFrag));
        return -1;
    }
    return resultValue;
}

/** EQ IDENTIFIER */
std::string_view Parser::layoutIdentifier() {
    if (!this->expect(Token::Kind::TK_EQ, "'='")) {
        return {};
    }
    Token resultToken;
    if (!this->expectIdentifier(&resultToken)) {
        return {};
    }
    return this->text(resultToken);
}

/* LAYOUT LPAREN IDENTIFIER (EQ INT_LITERAL)? (COMMA IDENTIFIER (EQ INT_LITERAL)?)* RPAREN */
DSLLayout Parser::layout() {
    enum class LayoutToken {
        LOCATION,
        OFFSET,
        BINDING,
        TEXTURE,
        SAMPLER,
        INDEX,
        SET,
        BUILTIN,
        INPUT_ATTACHMENT_INDEX,
        ORIGIN_UPPER_LEFT,
        BLEND_SUPPORT_ALL_EQUATIONS,
        PUSH_CONSTANT,
        COLOR,
        SPIRV,
        METAL,
        GL,
        WGSL
    };

    using LayoutMap = SkTHashMap<std::string_view, LayoutToken>;
    static LayoutMap* sLayoutTokens = new LayoutMap{
            {"location",                    LayoutToken::LOCATION},
            {"offset",                      LayoutToken::OFFSET},
            {"binding",                     LayoutToken::BINDING},
            {"texture",                     LayoutToken::TEXTURE},
            {"sampler",                     LayoutToken::SAMPLER},
            {"index",                       LayoutToken::INDEX},
            {"set",                         LayoutToken::SET},
            {"builtin",                     LayoutToken::BUILTIN},
            {"input_attachment_index",      LayoutToken::INPUT_ATTACHMENT_INDEX},
            {"origin_upper_left",           LayoutToken::ORIGIN_UPPER_LEFT},
            {"blend_support_all_equations", LayoutToken::BLEND_SUPPORT_ALL_EQUATIONS},
            {"push_constant",               LayoutToken::PUSH_CONSTANT},
            {"color",                       LayoutToken::COLOR},
            {"spirv",                       LayoutToken::SPIRV},
            {"metal",                       LayoutToken::METAL},
            {"gl",                          LayoutToken::GL},
            {"wgsl",                        LayoutToken::WGSL},
    };

    DSLLayout result;
    if (this->checkNext(Token::Kind::TK_LAYOUT)) {
        if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
            return result;
        }
        for (;;) {
            Token t = this->nextToken();
            std::string text(this->text(t));
            LayoutToken* found = sLayoutTokens->find(text);
            if (found != nullptr) {
                switch (*found) {
                    case LayoutToken::SPIRV:
                        result.spirv(this->position(t));
                        break;
                    case LayoutToken::METAL:
                        result.metal(this->position(t));
                        break;
                    case LayoutToken::GL:
                        result.gl(this->position(t));
                        break;
                    case LayoutToken::WGSL:
                        result.wgsl(this->position(t));
                        break;
                    case LayoutToken::ORIGIN_UPPER_LEFT:
                        result.originUpperLeft(this->position(t));
                        break;
                    case LayoutToken::PUSH_CONSTANT:
                        result.pushConstant(this->position(t));
                        break;
                    case LayoutToken::BLEND_SUPPORT_ALL_EQUATIONS:
                        result.blendSupportAllEquations(this->position(t));
                        break;
                    case LayoutToken::COLOR:
                        result.color(this->position(t));
                        break;
                    case LayoutToken::LOCATION:
                        result.location(this->layoutInt(), this->position(t));
                        break;
                    case LayoutToken::OFFSET:
                        result.offset(this->layoutInt(), this->position(t));
                        break;
                    case LayoutToken::BINDING:
                        result.binding(this->layoutInt(), this->position(t));
                        break;
                    case LayoutToken::INDEX:
                        result.index(this->layoutInt(), this->position(t));
                        break;
                    case LayoutToken::SET:
                        result.set(this->layoutInt(), this->position(t));
                        break;
                    case LayoutToken::TEXTURE:
                        result.texture(this->layoutInt(), this->position(t));
                        break;
                    case LayoutToken::SAMPLER:
                        result.sampler(this->layoutInt(), this->position(t));
                        break;
                    case LayoutToken::BUILTIN:
                        result.builtin(this->layoutInt(), this->position(t));
                        break;
                    case LayoutToken::INPUT_ATTACHMENT_INDEX:
                        result.inputAttachmentIndex(this->layoutInt(), this->position(t));
                        break;
                }
            } else {
                this->error(t, "'" + text + "' is not a valid layout qualifier");
            }
            if (this->checkNext(Token::Kind::TK_RPAREN)) {
                break;
            }
            if (!this->expect(Token::Kind::TK_COMMA, "','")) {
                break;
            }
        }
    }
    return result;
}

/* layout? (UNIFORM | CONST | IN | OUT | INOUT | LOWP | MEDIUMP | HIGHP | FLAT | NOPERSPECTIVE |
            VARYING | INLINE | WORKGROUP | READONLY | WRITEONLY | BUFFER)* */
DSLModifiers Parser::modifiers() {
    int start = this->peek().fOffset;
    DSLLayout layout = this->layout();
    Token raw = this->nextRawToken();
    int end = raw.fOffset;
    if (!is_whitespace(raw.fKind)) {
        this->pushback(raw);
    }
    int flags = 0;
    for (;;) {
        int tokenFlag = parse_modifier_token(peek().fKind);
        if (!tokenFlag) {
            break;
        }
        Token modifier = this->nextToken();
        if (int duplicateFlags = (tokenFlag & flags)) {
            this->error(modifier, "'" + Modifiers::DescribeFlags(duplicateFlags) +
                                  "' appears more than once");
        }
        flags |= tokenFlag;
        end = this->position(modifier).endOffset();
    }
    return DSLModifiers(std::move(layout), flags, Position::Range(start, end));
}

/* ifStatement | forStatement | doStatement | whileStatement | block | expression */
DSLStatement Parser::statement() {
    Token start = this->nextToken();
    AutoDepth depth(this);
    if (!depth.increase()) {
        return {};
    }
    this->pushback(start);
    switch (start.fKind) {
        case Token::Kind::TK_IF:
            return this->ifStatement();
        case Token::Kind::TK_FOR:
            return this->forStatement();
        case Token::Kind::TK_DO:
            return this->doStatement();
        case Token::Kind::TK_WHILE:
            return this->whileStatement();
        case Token::Kind::TK_SWITCH:
            return this->switchStatement();
        case Token::Kind::TK_RETURN:
            return this->returnStatement();
        case Token::Kind::TK_BREAK:
            return this->breakStatement();
        case Token::Kind::TK_CONTINUE:
            return this->continueStatement();
        case Token::Kind::TK_DISCARD:
            return this->discardStatement();
        case Token::Kind::TK_LBRACE: {
            std::optional<DSLBlock> result = this->block();
            return result ? DSLStatement(std::move(*result)) : DSLStatement();
        }
        case Token::Kind::TK_SEMICOLON:
            this->nextToken();
            return DSLBlock();
        case Token::Kind::TK_HIGHP:
        case Token::Kind::TK_MEDIUMP:
        case Token::Kind::TK_LOWP:
        case Token::Kind::TK_CONST:
        case Token::Kind::TK_IDENTIFIER:
            return this->varDeclarationsOrExpressionStatement();
        default:
            return this->expressionStatement();
    }
}

/* IDENTIFIER(type) (LBRACKET intLiteral? RBRACKET)* QUESTION? */
DSLType Parser::type(DSLModifiers* modifiers) {
    Token type;
    if (!this->expect(Token::Kind::TK_IDENTIFIER, "a type", &type)) {
        return DSLType(nullptr);
    }
    if (!this->symbolTable()->isType(this->text(type))) {
        this->error(type, "no type named '" + std::string(this->text(type)) + "'");
        return DSLType::Invalid();
    }
    DSLType result(this->text(type), modifiers, this->position(type));
    if (result.isInterfaceBlock()) {
        // SkSL puts interface blocks into the symbol table, but they aren't general-purpose types;
        // you can't use them to declare a variable type or a function return type.
        this->error(type, "expected a type, found '" + std::string(this->text(type)) + "'");
        return DSLType::Invalid();
    }
    Token bracket;
    while (this->checkNext(Token::Kind::TK_LBRACKET, &bracket)) {
        if (this->checkNext(Token::Kind::TK_RBRACKET)) {
            if (this->allowUnsizedArrays()) {
                result = UnsizedArray(result, this->rangeFrom(type));
            } else {
                this->error(this->rangeFrom(bracket), "unsized arrays are not permitted here");
            }
        } else {
            SKSL_INT size;
            if (!this->arraySize(&size)) {
                return DSLType(nullptr);
            }
            this->expect(Token::Kind::TK_RBRACKET, "']'");
            result = Array(result, size, this->rangeFrom(type));
        }
    }
    return result;
}

/* IDENTIFIER LBRACE
     varDeclaration+
   RBRACE (IDENTIFIER (LBRACKET expression RBRACKET)*)? SEMICOLON */
bool Parser::interfaceBlock(const dsl::DSLModifiers& modifiers) {
    Token typeName;
    if (!this->expectIdentifier(&typeName)) {
        return false;
    }
    if (this->peek().fKind != Token::Kind::TK_LBRACE) {
        // we only get into interfaceBlock if we found a top-level identifier which was not a type.
        // 99% of the time, the user was not actually intending to create an interface block, so
        // it's better to report it as an unknown type
        this->error(typeName, "no type named '" + std::string(this->text(typeName)) + "'");
        return false;
    }
    this->nextToken();
    SkTArray<DSLField> fields;
    SkTHashSet<std::string_view> fieldNames;
    while (!this->checkNext(Token::Kind::TK_RBRACE)) {
        Position fieldPos = this->position(this->peek());
        DSLModifiers fieldModifiers = this->modifiers();
        DSLType type = this->type(&fieldModifiers);
        if (!type.hasValue()) {
            return false;
        }
        do {
            Token fieldName;
            if (!this->expectIdentifier(&fieldName)) {
                return false;
            }
            DSLType actualType = type;
            if (this->checkNext(Token::Kind::TK_LBRACKET)) {
                Token sizeToken = this->peek();
                if (sizeToken.fKind != Token::Kind::TK_RBRACKET) {
                    SKSL_INT size;
                    if (!this->arraySize(&size)) {
                        return false;
                    }
                    actualType = Array(std::move(actualType), size, this->position(typeName));
                } else if (this->allowUnsizedArrays()) {
                    actualType = UnsizedArray(std::move(actualType), this->position(typeName));
                } else {
                    this->error(sizeToken, "unsized arrays are not permitted here");
                }
                this->expect(Token::Kind::TK_RBRACKET, "']'");
            }
            if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
                return false;
            }

            std::string_view nameText = this->text(fieldName);
            if (!fieldNames.contains(nameText)) {
                fields.push_back(DSLField(fieldModifiers,
                                          std::move(actualType),
                                          nameText,
                                          this->rangeFrom(fieldPos)));
                fieldNames.add(nameText);
            } else {
                this->error(fieldName, "field '" + std::string(nameText) +
                                       "' was already defined in the same interface block ('" +
                                       std::string(this->text(typeName)) +  "')");
            }
        } while (this->checkNext(Token::Kind::TK_COMMA));
    }
    if (fields.empty()) {
        this->error(this->rangeFrom(typeName), "interface block '" +
                std::string(this->text(typeName)) + "' must contain at least one member");
    }
    std::string_view instanceName;
    Token instanceNameToken;
    SKSL_INT size = 0;
    if (this->checkIdentifier(&instanceNameToken)) {
        instanceName = this->text(instanceNameToken);
        if (this->checkNext(Token::Kind::TK_LBRACKET)) {
            if (!this->arraySize(&size)) {
                return false;
            }
            this->expect(Token::Kind::TK_RBRACKET, "']'");
        }
    }
    if (!fields.empty()) {
        dsl::InterfaceBlock(modifiers, this->text(typeName), std::move(fields), instanceName,
                            size, this->position(typeName));
    }
    this->expect(Token::Kind::TK_SEMICOLON, "';'");
    return true;
}

/* IF LPAREN expression RPAREN statement (ELSE statement)? */
DSLStatement Parser::ifStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_IF, "'if'", &start)) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
        return {};
    }
    DSLExpression test = this->expression();
    if (!test.hasValue()) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
        return {};
    }
    DSLStatement ifTrue = this->statement();
    if (!ifTrue.hasValue()) {
        return {};
    }
    DSLStatement ifFalse;
    if (this->checkNext(Token::Kind::TK_ELSE)) {
        ifFalse = this->statement();
        if (!ifFalse.hasValue()) {
            return {};
        }
    }
    Position pos = this->rangeFrom(start);
    return If(std::move(test), std::move(ifTrue),
              ifFalse.hasValue() ? std::move(ifFalse) : DSLStatement(), pos);
}

/* DO statement WHILE LPAREN expression RPAREN SEMICOLON */
DSLStatement Parser::doStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_DO, "'do'", &start)) {
        return {};
    }
    DSLStatement statement = this->statement();
    if (!statement.hasValue()) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_WHILE, "'while'")) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
        return {};
    }
    DSLExpression test = this->expression();
    if (!test.hasValue()) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return {};
    }
    return Do(std::move(statement), std::move(test), this->rangeFrom(start));
}

/* WHILE LPAREN expression RPAREN STATEMENT */
DSLStatement Parser::whileStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_WHILE, "'while'", &start)) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
        return {};
    }
    DSLExpression test = this->expression();
    if (!test.hasValue()) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
        return {};
    }
    DSLStatement statement = this->statement();
    if (!statement.hasValue()) {
        return {};
    }
    return While(std::move(test), std::move(statement), this->rangeFrom(start));
}

/* CASE expression COLON statement* */
std::optional<DSLCase> Parser::switchCase() {
    Token start;
    if (!this->expect(Token::Kind::TK_CASE, "'case'", &start)) {
        return {};
    }
    DSLExpression value = this->expression();
    if (!value.hasValue()) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_COLON, "':'")) {
        return {};
    }
    SkTArray<DSLStatement> statements;
    while (this->peek().fKind != Token::Kind::TK_RBRACE &&
           this->peek().fKind != Token::Kind::TK_CASE &&
           this->peek().fKind != Token::Kind::TK_DEFAULT) {
        DSLStatement s = this->statement();
        if (!s.hasValue()) {
            return {};
        }
        statements.push_back(std::move(s));
    }
    return DSLCase(std::move(value), std::move(statements));
}

/* SWITCH LPAREN expression RPAREN LBRACE switchCase* (DEFAULT COLON statement*)? RBRACE */
DSLStatement Parser::switchStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_SWITCH, "'switch'", &start)) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
        return {};
    }
    DSLExpression value = this->expression();
    if (!value.hasValue()) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_LBRACE, "'{'")) {
        return {};
    }
    SkTArray<DSLCase> cases;
    while (this->peek().fKind == Token::Kind::TK_CASE) {
        std::optional<DSLCase> c = this->switchCase();
        if (!c) {
            return {};
        }
        cases.push_back(std::move(*c));
    }
    // Requiring default: to be last (in defiance of C and GLSL) was a deliberate decision. Other
    // parts of the compiler may rely upon this assumption.
    if (this->peek().fKind == Token::Kind::TK_DEFAULT) {
        SkTArray<DSLStatement> statements;
        Token defaultStart;
        SkAssertResult(this->expect(Token::Kind::TK_DEFAULT, "'default'", &defaultStart));
        if (!this->expect(Token::Kind::TK_COLON, "':'")) {
            return {};
        }
        while (this->peek().fKind != Token::Kind::TK_RBRACE) {
            DSLStatement s = this->statement();
            if (!s.hasValue()) {
                return {};
            }
            statements.push_back(std::move(s));
        }
        cases.push_back(DSLCase(DSLExpression(), std::move(statements), this->position(start)));
    }
    if (!this->expect(Token::Kind::TK_RBRACE, "'}'")) {
        return {};
    }
    Position pos = this->rangeFrom(start);
    return Switch(std::move(value), std::move(cases), pos);
}

static Position range_of_at_least_one_char(int start, int end) {
    return Position::Range(start, std::max(end, start + 1));
}

/* FOR LPAREN (declaration | expression)? SEMICOLON expression? SEMICOLON expression? RPAREN
   STATEMENT */
dsl::DSLStatement Parser::forStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_FOR, "'for'", &start)) {
        return {};
    }
    Token lparen;
    if (!this->expect(Token::Kind::TK_LPAREN, "'('", &lparen)) {
        return {};
    }
    AutoSymbolTable symbols(this);
    dsl::DSLStatement initializer;
    Token nextToken = this->peek();
    int firstSemicolonOffset;
    if (nextToken.fKind == Token::Kind::TK_SEMICOLON) {
        // An empty init-statement.
        firstSemicolonOffset = this->nextToken().fOffset;
    } else {
        // The init-statement must be an expression or variable declaration.
        initializer = this->varDeclarationsOrExpressionStatement();
        if (!initializer.hasValue()) {
            return {};
        }
        firstSemicolonOffset = fLexer.getCheckpoint().fOffset - 1;
    }
    dsl::DSLExpression test;
    if (this->peek().fKind != Token::Kind::TK_SEMICOLON) {
        dsl::DSLExpression testValue = this->expression();
        if (!testValue.hasValue()) {
            return {};
        }
        test.swap(testValue);
    }
    Token secondSemicolon;
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'", &secondSemicolon)) {
        return {};
    }
    dsl::DSLExpression next;
    if (this->peek().fKind != Token::Kind::TK_RPAREN) {
        dsl::DSLExpression nextValue = this->expression();
        if (!nextValue.hasValue()) {
            return {};
        }
        next.swap(nextValue);
    }
    Token rparen;
    if (!this->expect(Token::Kind::TK_RPAREN, "')'", &rparen)) {
        return {};
    }
    dsl::DSLStatement statement = this->statement();
    if (!statement.hasValue()) {
        return {};
    }
    return For(initializer.hasValue() ? std::move(initializer) : DSLStatement(),
               test.hasValue() ? std::move(test) : DSLExpression(),
               next.hasValue() ? std::move(next) : DSLExpression(),
               std::move(statement),
               this->rangeFrom(start),
               ForLoopPositions{
                    range_of_at_least_one_char(lparen.fOffset + 1, firstSemicolonOffset),
                    range_of_at_least_one_char(firstSemicolonOffset + 1, secondSemicolon.fOffset),
                    range_of_at_least_one_char(secondSemicolon.fOffset + 1, rparen.fOffset)
               });
}

/* RETURN expression? SEMICOLON */
DSLStatement Parser::returnStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_RETURN, "'return'", &start)) {
        return {};
    }
    DSLExpression expression;
    if (this->peek().fKind != Token::Kind::TK_SEMICOLON) {
        DSLExpression next = this->expression();
        if (!next.hasValue()) {
            return {};
        }
        expression.swap(next);
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return {};
    }
    return Return(expression.hasValue() ? std::move(expression) : DSLExpression(),
            this->rangeFrom(start));
}

/* BREAK SEMICOLON */
DSLStatement Parser::breakStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_BREAK, "'break'", &start)) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return {};
    }
    return Break(this->position(start));
}

/* CONTINUE SEMICOLON */
DSLStatement Parser::continueStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_CONTINUE, "'continue'", &start)) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return {};
    }
    return Continue(this->position(start));
}

/* DISCARD SEMICOLON */
DSLStatement Parser::discardStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_DISCARD, "'continue'", &start)) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return {};
    }
    return Discard(this->position(start));
}

/* LBRACE statement* RBRACE */
std::optional<DSLBlock> Parser::block() {
    Token start;
    if (!this->expect(Token::Kind::TK_LBRACE, "'{'", &start)) {
        return std::nullopt;
    }
    AutoDepth depth(this);
    if (!depth.increase()) {
        return std::nullopt;
    }
    AutoSymbolTable symbols(this);
    StatementArray statements;
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_RBRACE:
                this->nextToken();
                return DSLBlock(std::move(statements), this->symbolTable(), this->rangeFrom(start));
            case Token::Kind::TK_END_OF_FILE:
                this->error(this->peek(), "expected '}', but found end of file");
                return std::nullopt;
            default: {
                DSLStatement statement = this->statement();
                if (fEncounteredFatalError) {
                    return std::nullopt;
                }
                if (statement.hasValue()) {
                    statements.push_back(statement.release());
                }
                break;
            }
        }
    }
}

/* expression SEMICOLON */
DSLStatement Parser::expressionStatement() {
    DSLExpression expr = this->expression();
    if (expr.hasValue()) {
        if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
            return {};
        }
        return DSLStatement(std::move(expr));
    }
    return {};
}

bool Parser::operatorRight(Parser::AutoDepth& depth,
                           Operator::Kind op,
                           BinaryParseFn rightFn,
                           DSLExpression& result) {
    this->nextToken();
    if (!depth.increase()) {
        return false;
    }
    DSLExpression right = (this->*rightFn)();
    if (!right.hasValue()) {
        return false;
    }
    Position pos = result.position().rangeThrough(right.position());
    DSLExpression next = result.binary(op, std::move(right), pos);
    result.swap(next);
    return true;
}

/* assignmentExpression (COMMA assignmentExpression)* */
DSLExpression Parser::expression() {
    [[maybe_unused]] Token start = this->peek();
    DSLExpression result = this->assignmentExpression();
    if (!result.hasValue()) {
        return {};
    }
    Token t;
    AutoDepth depth(this);
    while (this->peek().fKind == Token::Kind::TK_COMMA) {
        if (!operatorRight(depth, Operator::Kind::COMMA, &Parser::assignmentExpression,
                result)) {
            return {};
        }
    }
    SkASSERTF(result.position().valid(), "Expression %s has invalid position",
            result.description().c_str());
    SkASSERTF(result.position().startOffset() == this->position(start).startOffset(),
            "Expected %s to start at %d (first token: '%.*s'), but it has range %d-%d\n",
            result.description().c_str(), this->position(start).startOffset(),
            (int)this->text(start).length(), this->text(start).data(),
            result.position().startOffset(), result.position().endOffset());
    return result;
}

/* ternaryExpression ((EQEQ | STAREQ | SLASHEQ | PERCENTEQ | PLUSEQ | MINUSEQ | SHLEQ | SHREQ |
   BITWISEANDEQ | BITWISEXOREQ | BITWISEOREQ | LOGICALANDEQ | LOGICALXOREQ | LOGICALOREQ)
   assignmentExpression)*
 */
DSLExpression Parser::assignmentExpression() {
    AutoDepth depth(this);
    DSLExpression result = this->ternaryExpression();
    if (!result.hasValue()) {
        return {};
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_EQ:
                if (!operatorRight(depth, Operator::Kind::EQ, &Parser::assignmentExpression,
                        result)) {
                    return {};
                }
                break;
            case Token::Kind::TK_STAREQ:
                if (!operatorRight(depth, Operator::Kind::STAREQ, &Parser::assignmentExpression,
                        result)) {
                    return {};
                }
                break;
            case Token::Kind::TK_SLASHEQ:
                if (!operatorRight(depth, Operator::Kind::SLASHEQ, &Parser::assignmentExpression,
                        result)) {
                    return {};
                }
                break;
            case Token::Kind::TK_PERCENTEQ:
                if (!operatorRight(depth, Operator::Kind::PERCENTEQ,
                        &Parser::assignmentExpression, result)) {
                    return {};
                }
                break;
            case Token::Kind::TK_PLUSEQ:
                if (!operatorRight(depth, Operator::Kind::PLUSEQ, &Parser::assignmentExpression,
                        result)) {
                    return {};
                }
                break;
            case Token::Kind::TK_MINUSEQ:
                if (!operatorRight(depth, Operator::Kind::MINUSEQ, &Parser::assignmentExpression,
                        result)) {
                    return {};
                }
                break;
            case Token::Kind::TK_SHLEQ:
                if (!operatorRight(depth, Operator::Kind::SHLEQ, &Parser::assignmentExpression,
                        result)) {
                    return {};
                }
                break;
            case Token::Kind::TK_SHREQ:
                if (!operatorRight(depth, Operator::Kind::SHREQ, &Parser::assignmentExpression,
                        result)) {
                    return {};
                }
                break;
            case Token::Kind::TK_BITWISEANDEQ:
                if (!operatorRight(depth, Operator::Kind::BITWISEANDEQ,
                        &Parser::assignmentExpression, result)) {
                    return {};
                }
                break;
            case Token::Kind::TK_BITWISEXOREQ:
                if (!operatorRight(depth, Operator::Kind::BITWISEXOREQ,
                        &Parser::assignmentExpression, result)) {
                    return {};
                }
                break;
            case Token::Kind::TK_BITWISEOREQ:
                if (!operatorRight(depth, Operator::Kind::BITWISEOREQ,
                        &Parser::assignmentExpression, result)) {
                    return {};
                }
                break;
            default:
                return result;
        }
    }
}

/* logicalOrExpression ('?' expression ':' assignmentExpression)? */
DSLExpression Parser::ternaryExpression() {
    DSLExpression base = this->logicalOrExpression();
    if (!base.hasValue()) {
        return {};
    }
    if (!this->checkNext(Token::Kind::TK_QUESTION)) {
        return base;
    }
    AutoDepth depth(this);
    if (!depth.increase()) {
        return {};
    }
    DSLExpression trueExpr = this->expression();
    if (!trueExpr.hasValue()) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_COLON, "':'")) {
        return {};
    }
    DSLExpression falseExpr = this->assignmentExpression();
    if (!falseExpr.hasValue()) {
        return {};
    }
    Position pos = base.position().rangeThrough(falseExpr.position());
    return Select(std::move(base), std::move(trueExpr), std::move(falseExpr), pos);
}

/* logicalXorExpression (LOGICALOR logicalXorExpression)* */
DSLExpression Parser::logicalOrExpression() {
    AutoDepth depth(this);
    DSLExpression result = this->logicalXorExpression();
    if (!result.hasValue()) {
        return {};
    }
    while (this->peek().fKind == Token::Kind::TK_LOGICALOR) {
        if (!operatorRight(depth, Operator::Kind::LOGICALOR, &Parser::logicalXorExpression,
                result)) {
            return {};
        }
    }
    return result;
}

/* logicalAndExpression (LOGICALXOR logicalAndExpression)* */
DSLExpression Parser::logicalXorExpression() {
    AutoDepth depth(this);
    DSLExpression result = this->logicalAndExpression();
    if (!result.hasValue()) {
        return {};
    }
    while (this->peek().fKind == Token::Kind::TK_LOGICALXOR) {
        if (!operatorRight(depth, Operator::Kind::LOGICALXOR, &Parser::logicalAndExpression,
                result)) {
            return {};
        }
    }
    return result;
}

/* bitwiseOrExpression (LOGICALAND bitwiseOrExpression)* */
DSLExpression Parser::logicalAndExpression() {
    AutoDepth depth(this);
    DSLExpression result = this->bitwiseOrExpression();
    if (!result.hasValue()) {
        return {};
    }
    while (this->peek().fKind == Token::Kind::TK_LOGICALAND) {
        if (!operatorRight(depth, Operator::Kind::LOGICALAND, &Parser::bitwiseOrExpression,
                result)) {
            return {};
        }
    }
    return result;
}

/* bitwiseXorExpression (BITWISEOR bitwiseXorExpression)* */
DSLExpression Parser::bitwiseOrExpression() {
    AutoDepth depth(this);
    DSLExpression result = this->bitwiseXorExpression();
    if (!result.hasValue()) {
        return {};
    }
    while (this->peek().fKind == Token::Kind::TK_BITWISEOR) {
        if (!operatorRight(depth, Operator::Kind::BITWISEOR, &Parser::bitwiseXorExpression,
                result)) {
            return {};
        }
    }
    return result;
}

/* bitwiseAndExpression (BITWISEXOR bitwiseAndExpression)* */
DSLExpression Parser::bitwiseXorExpression() {
    AutoDepth depth(this);
    DSLExpression result = this->bitwiseAndExpression();
    if (!result.hasValue()) {
        return {};
    }
    while (this->peek().fKind == Token::Kind::TK_BITWISEXOR) {
        if (!operatorRight(depth, Operator::Kind::BITWISEXOR, &Parser::bitwiseAndExpression,
                result)) {
            return {};
        }
    }
    return result;
}

/* equalityExpression (BITWISEAND equalityExpression)* */
DSLExpression Parser::bitwiseAndExpression() {
    AutoDepth depth(this);
    DSLExpression result = this->equalityExpression();
    if (!result.hasValue()) {
        return {};
    }
    while (this->peek().fKind == Token::Kind::TK_BITWISEAND) {
        if (!operatorRight(depth, Operator::Kind::BITWISEAND, &Parser::equalityExpression,
                result)) {
            return {};
        }
    }
    return result;
}

/* relationalExpression ((EQEQ | NEQ) relationalExpression)* */
DSLExpression Parser::equalityExpression() {
    AutoDepth depth(this);
    DSLExpression result = this->relationalExpression();
    if (!result.hasValue()) {
        return {};
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_EQEQ:
                if (!operatorRight(depth, Operator::Kind::EQEQ, &Parser::relationalExpression,
                        result)) {
                    return {};
                }
                break;
            case Token::Kind::TK_NEQ:
                if (!operatorRight(depth, Operator::Kind::NEQ, &Parser::relationalExpression,
                        result)) {
                    return {};
                }
                break;
            default: return result;
        }
    }
}

/* shiftExpression ((LT | GT | LTEQ | GTEQ) shiftExpression)* */
DSLExpression Parser::relationalExpression() {
    AutoDepth depth(this);
    DSLExpression result = this->shiftExpression();
    if (!result.hasValue()) {
        return {};
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_LT:
                if (!operatorRight(depth, Operator::Kind::LT, &Parser::shiftExpression,
                        result)) {
                    return {};
                }
                break;
            case Token::Kind::TK_GT:
                if (!operatorRight(depth, Operator::Kind::GT, &Parser::shiftExpression,
                        result)) {
                    return {};
                }
                break;
            case Token::Kind::TK_LTEQ:
                if (!operatorRight(depth, Operator::Kind::LTEQ, &Parser::shiftExpression,
                        result)) {
                    return {};
                }
                break;
            case Token::Kind::TK_GTEQ:
                if (!operatorRight(depth, Operator::Kind::GTEQ, &Parser::shiftExpression,
                        result)) {
                    return {};
                }
                break;
            default:
                return result;
        }
    }
}

/* additiveExpression ((SHL | SHR) additiveExpression)* */
DSLExpression Parser::shiftExpression() {
    AutoDepth depth(this);
    DSLExpression result = this->additiveExpression();
    if (!result.hasValue()) {
        return {};
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_SHL:
                if (!operatorRight(depth, Operator::Kind::SHL, &Parser::additiveExpression,
                        result)) {
                    return {};
                }
                break;
            case Token::Kind::TK_SHR:
                if (!operatorRight(depth, Operator::Kind::SHR, &Parser::additiveExpression,
                        result)) {
                    return {};
                }
                break;
            default:
                return result;
        }
    }
}

/* multiplicativeExpression ((PLUS | MINUS) multiplicativeExpression)* */
DSLExpression Parser::additiveExpression() {
    AutoDepth depth(this);
    DSLExpression result = this->multiplicativeExpression();
    if (!result.hasValue()) {
        return {};
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_PLUS:
                if (!operatorRight(depth, Operator::Kind::PLUS,
                        &Parser::multiplicativeExpression, result)) {
                    return {};
                }
                break;
            case Token::Kind::TK_MINUS:
                if (!operatorRight(depth, Operator::Kind::MINUS,
                        &Parser::multiplicativeExpression, result)) {
                    return {};
                }
                break;
            default:
                return result;
        }
    }
}

/* unaryExpression ((STAR | SLASH | PERCENT) unaryExpression)* */
DSLExpression Parser::multiplicativeExpression() {
    AutoDepth depth(this);
    DSLExpression result = this->unaryExpression();
    if (!result.hasValue()) {
        return {};
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_STAR:
                if (!operatorRight(depth, Operator::Kind::STAR, &Parser::unaryExpression,
                        result)) {
                    return {};
                }
                break;
            case Token::Kind::TK_SLASH:
                if (!operatorRight(depth, Operator::Kind::SLASH, &Parser::unaryExpression,
                        result)) {
                    return {};
                }
                break;
            case Token::Kind::TK_PERCENT:
                if (!operatorRight(depth, Operator::Kind::PERCENT, &Parser::unaryExpression,
                        result)) {
                    return {};
                }
                break;
            default: return result;
        }
    }
}

/* postfixExpression | (PLUS | MINUS | NOT | PLUSPLUS | MINUSMINUS) unaryExpression */
DSLExpression Parser::unaryExpression() {
    AutoDepth depth(this);
    Token start = this->peek();
    switch (start.fKind) {
        case Token::Kind::TK_PLUS:
        case Token::Kind::TK_MINUS:
        case Token::Kind::TK_LOGICALNOT:
        case Token::Kind::TK_BITWISENOT:
        case Token::Kind::TK_PLUSPLUS:
        case Token::Kind::TK_MINUSMINUS: {
            this->nextToken();
            if (!depth.increase()) {
                return {};
            }
            DSLExpression expr = this->unaryExpression();
            if (!expr.hasValue()) {
                return {};
            }
            Position p = Position::Range(start.fOffset, expr.position().endOffset());
            switch (start.fKind) {
                case Token::Kind::TK_PLUS:       return expr.prefix(Operator::Kind::PLUS, p);
                case Token::Kind::TK_MINUS:      return expr.prefix(Operator::Kind::MINUS, p);
                case Token::Kind::TK_LOGICALNOT: return expr.prefix(Operator::Kind::LOGICALNOT, p);
                case Token::Kind::TK_BITWISENOT: return expr.prefix(Operator::Kind::BITWISENOT, p);
                case Token::Kind::TK_PLUSPLUS:   return expr.prefix(Operator::Kind::PLUSPLUS, p);
                case Token::Kind::TK_MINUSMINUS: return expr.prefix(Operator::Kind::MINUSMINUS, p);
                default: SkUNREACHABLE;
            }
        }
        default:
            return this->postfixExpression();
    }
}

/* term suffix* */
DSLExpression Parser::postfixExpression() {
    AutoDepth depth(this);
    DSLExpression result = this->term();
    if (!result.hasValue()) {
        return {};
    }
    for (;;) {
        Token t = this->peek();
        switch (t.fKind) {
            case Token::Kind::TK_FLOAT_LITERAL:
                if (this->text(t)[0] != '.') {
                    return result;
                }
                [[fallthrough]];
            case Token::Kind::TK_LBRACKET:
            case Token::Kind::TK_DOT:
            case Token::Kind::TK_LPAREN:
            case Token::Kind::TK_PLUSPLUS:
            case Token::Kind::TK_MINUSMINUS: {
                if (!depth.increase()) {
                    return {};
                }
                DSLExpression next = this->suffix(std::move(result));
                if (!next.hasValue()) {
                    return {};
                }
                result.swap(next);
                break;
            }
            default:
                return result;
        }
    }
}

DSLExpression Parser::swizzle(Position pos,
                              DSLExpression base,
                              std::string_view swizzleMask,
                              Position maskPos) {
    SkASSERT(swizzleMask.length() > 0);
    if (!base.type().isVector() && !base.type().isScalar()) {
        return base.field(swizzleMask, pos);
    }
    int length = swizzleMask.length();
    SkSL::SwizzleComponent::Type components[4];
    for (int i = 0; i < length; ++i) {
        if (i >= 4) {
            Position errorPos = maskPos.valid() ? Position::Range(maskPos.startOffset() + 4,
                                                                  maskPos.endOffset())
                                                : pos;
            this->error(errorPos, "too many components in swizzle mask");
            return DSLExpression::Poison(pos);
        }
        switch (swizzleMask[i]) {
            case '0': components[i] = SwizzleComponent::ZERO; break;
            case '1': components[i] = SwizzleComponent::ONE;  break;
            case 'r': components[i] = SwizzleComponent::R;    break;
            case 'x': components[i] = SwizzleComponent::X;    break;
            case 's': components[i] = SwizzleComponent::S;    break;
            case 'L': components[i] = SwizzleComponent::UL;   break;
            case 'g': components[i] = SwizzleComponent::G;    break;
            case 'y': components[i] = SwizzleComponent::Y;    break;
            case 't': components[i] = SwizzleComponent::T;    break;
            case 'T': components[i] = SwizzleComponent::UT;   break;
            case 'b': components[i] = SwizzleComponent::B;    break;
            case 'z': components[i] = SwizzleComponent::Z;    break;
            case 'p': components[i] = SwizzleComponent::P;    break;
            case 'R': components[i] = SwizzleComponent::UR;   break;
            case 'a': components[i] = SwizzleComponent::A;    break;
            case 'w': components[i] = SwizzleComponent::W;    break;
            case 'q': components[i] = SwizzleComponent::Q;    break;
            case 'B': components[i] = SwizzleComponent::UB;   break;
            default: {
                Position componentPos = Position::Range(maskPos.startOffset() + i,
                        maskPos.startOffset() + i + 1);
                this->error(componentPos, String::printf("invalid swizzle component '%c'",
                        swizzleMask[i]).c_str());
                return DSLExpression::Poison(pos);
            }
        }
    }
    switch (length) {
        case 1: return dsl::Swizzle(std::move(base), components[0], pos, maskPos);
        case 2: return dsl::Swizzle(std::move(base), components[0], components[1], pos, maskPos);
        case 3: return dsl::Swizzle(std::move(base), components[0], components[1], components[2],
                                    pos, maskPos);
        case 4: return dsl::Swizzle(std::move(base), components[0], components[1], components[2],
                                    components[3], pos, maskPos);
        default: SkUNREACHABLE;
    }
}

dsl::DSLExpression Parser::call(Position pos, dsl::DSLExpression base, ExpressionArray args) {
    return base(std::move(args), pos);
}

/* LBRACKET expression? RBRACKET | DOT IDENTIFIER | LPAREN arguments RPAREN |
   PLUSPLUS | MINUSMINUS | COLONCOLON IDENTIFIER | FLOAT_LITERAL [IDENTIFIER] */
DSLExpression Parser::suffix(DSLExpression base) {
    Token next = this->nextToken();
    AutoDepth depth(this);
    if (!depth.increase()) {
        return {};
    }
    switch (next.fKind) {
        case Token::Kind::TK_LBRACKET: {
            if (this->checkNext(Token::Kind::TK_RBRACKET)) {
                this->error(this->rangeFrom(next), "missing index in '[]'");
                return DSLExpression::Poison(this->rangeFrom(base.position()));
            }
            DSLExpression index = this->expression();
            if (!index.hasValue()) {
                return {};
            }
            this->expect(Token::Kind::TK_RBRACKET, "']' to complete array access expression");
            return base.index(std::move(index), this->rangeFrom(base.position()));
        }
        case Token::Kind::TK_DOT: {
            std::string_view text;
            if (this->identifier(&text)) {
                Position pos = this->rangeFrom(base.position());
                return this->swizzle(pos, std::move(base), text,
                        this->rangeFrom(this->position(next).after()));
            }
            [[fallthrough]];
        }
        case Token::Kind::TK_FLOAT_LITERAL: {
            // Swizzles that start with a constant number, e.g. '.000r', will be tokenized as
            // floating point literals, possibly followed by an identifier. Handle that here.
            std::string_view field = this->text(next);
            SkASSERT(field[0] == '.');
            field.remove_prefix(1);
            // use the next *raw* token so we don't ignore whitespace - we only care about
            // identifiers that directly follow the float
            Position pos = this->rangeFrom(base.position());
            Position start = this->position(next);
            // skip past the "."
            start = Position::Range(start.startOffset() + 1, start.endOffset());
            Position maskPos = this->rangeFrom(start);
            Token id = this->nextRawToken();
            if (id.fKind == Token::Kind::TK_IDENTIFIER) {
                pos = this->rangeFrom(base.position());
                maskPos = this->rangeFrom(start);
                return this->swizzle(pos, std::move(base), std::string(field) +
                        std::string(this->text(id)), maskPos);
            } else if (field.empty()) {
                this->error(pos, "expected field name or swizzle mask after '.'");
                return {{DSLExpression::Poison(pos)}};
            }
            this->pushback(id);
            return this->swizzle(pos, std::move(base), field, maskPos);
        }
        case Token::Kind::TK_LPAREN: {
            ExpressionArray args;
            if (this->peek().fKind != Token::Kind::TK_RPAREN) {
                for (;;) {
                    DSLExpression expr = this->assignmentExpression();
                    if (!expr.hasValue()) {
                        return {};
                    }
                    args.push_back(expr.release());
                    if (!this->checkNext(Token::Kind::TK_COMMA)) {
                        break;
                    }
                }
            }
            this->expect(Token::Kind::TK_RPAREN, "')' to complete function arguments");
            Position pos = this->rangeFrom(base.position());
            return this->call(pos, std::move(base), std::move(args));
        }
        case Token::Kind::TK_PLUSPLUS:
            return base.postfix(Operator::Kind::PLUSPLUS, this->rangeFrom(base.position()));
        case Token::Kind::TK_MINUSMINUS:
            return base.postfix(Operator::Kind::MINUSMINUS, this->rangeFrom(base.position()));
        default: {
            this->error(next, "expected expression suffix, but found '" +
                              std::string(this->text(next)) + "'");
            return {};
        }
    }
}

/* IDENTIFIER | intLiteral | floatLiteral | boolLiteral | '(' expression ')' */
DSLExpression Parser::term() {
    Token t = this->peek();
    switch (t.fKind) {
        case Token::Kind::TK_IDENTIFIER: {
            std::string_view text;
            if (this->identifier(&text)) {
                Position pos = this->position(t);
                return DSLExpression(fCompiler.convertIdentifier(pos, text), pos);
            }
            break;
        }
        case Token::Kind::TK_INT_LITERAL: {
            SKSL_INT i;
            if (!this->intLiteral(&i)) {
                i = 0;
            }
            return DSLExpression(i, this->position(t));
        }
        case Token::Kind::TK_FLOAT_LITERAL: {
            SKSL_FLOAT f;
            if (!this->floatLiteral(&f)) {
                f = 0.0f;
            }
            return DSLExpression(f, this->position(t));
        }
        case Token::Kind::TK_TRUE_LITERAL: // fall through
        case Token::Kind::TK_FALSE_LITERAL: {
            bool b;
            SkAssertResult(this->boolLiteral(&b));
            return DSLExpression(b, this->position(t));
        }
        case Token::Kind::TK_LPAREN: {
            this->nextToken();
            AutoDepth depth(this);
            if (!depth.increase()) {
                return {};
            }
            DSLExpression result = this->expression();
            if (result.hasValue()) {
                this->expect(Token::Kind::TK_RPAREN, "')' to complete expression");
                result.setPosition(this->rangeFrom(this->position(t)));
                return result;
            }
            break;
        }
        default:
            this->nextToken();
            this->error(t, "expected expression, but found '" + std::string(this->text(t)) + "'");
            fEncounteredFatalError = true;
            break;
    }
    return {};
}

/* INT_LITERAL */
bool Parser::intLiteral(SKSL_INT* dest) {
    Token t;
    if (!this->expect(Token::Kind::TK_INT_LITERAL, "integer literal", &t)) {
        return false;
    }
    std::string_view s = this->text(t);
    if (!SkSL::stoi(s, dest)) {
        this->error(t, "integer is too large: " + std::string(s));
        return false;
    }
    return true;
}

/* FLOAT_LITERAL */
bool Parser::floatLiteral(SKSL_FLOAT* dest) {
    Token t;
    if (!this->expect(Token::Kind::TK_FLOAT_LITERAL, "float literal", &t)) {
        return false;
    }
    std::string_view s = this->text(t);
    if (!SkSL::stod(s, dest)) {
        this->error(t, "floating-point value is too large: " + std::string(s));
        return false;
    }
    return true;
}

/* TRUE_LITERAL | FALSE_LITERAL */
bool Parser::boolLiteral(bool* dest) {
    Token t = this->nextToken();
    switch (t.fKind) {
        case Token::Kind::TK_TRUE_LITERAL:
            *dest = true;
            return true;
        case Token::Kind::TK_FALSE_LITERAL:
            *dest = false;
            return true;
        default:
            this->error(t, "expected 'true' or 'false', but found '" +
                           std::string(this->text(t)) + "'");
            return false;
    }
}

/* IDENTIFIER */
bool Parser::identifier(std::string_view* dest) {
    Token t;
    if (this->expect(Token::Kind::TK_IDENTIFIER, "identifier", &t)) {
        *dest = this->text(t);
        return true;
    }
    return false;
}

}  // namespace SkSL
