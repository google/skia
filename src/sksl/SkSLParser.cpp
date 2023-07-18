/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLParser.h"

#include "include/core/SkSpan.h"
#include "include/private/base/SkTArray.h"
#include "include/sksl/SkSLVersion.h"
#include "src/base/SkNoDestructor.h"
#include "src/core/SkTHash.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLBreakStatement.h"
#include "src/sksl/ir/SkSLContinueStatement.h"
#include "src/sksl/ir/SkSLDiscardStatement.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExtension.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLFunctionPrototype.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLLayout.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLModifiers.h"
#include "src/sksl/ir/SkSLModifiersDeclaration.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLStructDefinition.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <algorithm>
#include <climits>
#include <initializer_list>
#include <memory>
#include <utility>
#include <vector>

using namespace skia_private;
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
    AutoDepth(Parser* p) : fParser(p), fDepth(0) {}

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

class Parser::Checkpoint {
public:
    Checkpoint(Parser* p) : fParser(p) {
        fPushbackCheckpoint = fParser->fPushback;
        fLexerCheckpoint = fParser->fLexer.getCheckpoint();
        fOldErrorReporter = &ThreadContext::GetErrorReporter();
        fOldEncounteredFatalError = fParser->fEncounteredFatalError;
        SkASSERT(fOldErrorReporter);
        ThreadContext::SetErrorReporter(&fErrorReporter);
    }

    ~Checkpoint() {
        SkASSERTF(!fOldErrorReporter, "Checkpoint was not accepted or rewound before destruction");
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
                ThreadContext::ReportError(error.fMsg, error.fPos);
            }
        }

    private:
        struct Error {
            std::string fMsg;
            Position fPos;
        };

        skia_private::TArray<Error> fErrors;
    };

    void restoreErrorReporter() {
        SkASSERT(fOldErrorReporter);
        ThreadContext::SetErrorReporter(fOldErrorReporter);
        fOldErrorReporter = nullptr;
    }

    Parser* fParser;
    Token fPushbackCheckpoint;
    SkSL::Lexer::Checkpoint fLexerCheckpoint;
    ForwardingErrorReporter fErrorReporter;
    ErrorReporter* fOldErrorReporter;
    bool fOldEncounteredFatalError;
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
    ThreadContext::ReportError(msg, position);
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
    ThreadContext::Start(&fCompiler, fKind, fSettings);
    ThreadContext::SetErrorReporter(errorReporter);
    errorReporter->setSource(*fText);
    this->declarations();
    std::unique_ptr<Program> result;
    if (!ThreadContext::GetErrorReporter().errorCount()) {
        result = fCompiler.releaseProgram(std::move(fText));
    }
    errorReporter->setSource(std::string_view());
    ThreadContext::End();
    return result;
}

std::unique_ptr<SkSL::Module> Parser::moduleInheritingFrom(const SkSL::Module* parent) {
    ErrorReporter* errorReporter = &fCompiler.errorReporter();
    ThreadContext::StartModule(&fCompiler, fKind, fSettings, parent);
    ThreadContext::SetErrorReporter(errorReporter);
    errorReporter->setSource(*fText);
    this->declarations();
    this->symbolTable()->takeOwnershipOfString(std::move(*fText));
    auto result = std::make_unique<SkSL::Module>();
    result->fParent = parent;
    result->fSymbols = this->symbolTable();
    result->fElements = std::move(ThreadContext::ProgramElements());
    errorReporter->setSource(std::string_view());
    ThreadContext::End();
    return result;
}

void Parser::declarations() {
    fEncounteredFatalError = false;

    // If the program is longer than 8MB (Position::kMaxOffset), error reporting goes off the rails.
    // At any rate, there's no good reason for a program to be this long.
    if (fText->size() >= Position::kMaxOffset) {
        this->error(Position(), "program is too large");
        fEncounteredFatalError = true;
        return;
    }

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

/* DIRECTIVE(#extension) IDENTIFIER COLON IDENTIFIER NEWLINE */
void Parser::extensionDirective(Position start) {
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
    // We expect a newline immediately after `#extension name : behavior`.
    if (this->expectNewline()) {
        std::unique_ptr<SkSL::Extension> ext = Extension::Convert(fCompiler.context(),
                                                                  this->rangeFrom(start),
                                                                  this->text(name),
                                                                  this->text(behavior));
        if (ext) {
            ThreadContext::ProgramElements().push_back(std::move(ext));
        }
    } else {
        this->error(start, "invalid #extension directive");
    }
}

/* DIRECTIVE(#version) INTLITERAL NEWLINE */
void Parser::versionDirective(Position start, bool allowVersion) {
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
            fCompiler.context().fConfig->fRequiredSkSLVersion = Version::k100;
            break;
        case 300:
            fCompiler.context().fConfig->fRequiredSkSLVersion = Version::k300;
            break;
        default:
            this->error(start, "unsupported version number");
            return;
    }
    // We expect a newline after a #version directive.
    if (!this->expectNewline()) {
        this->error(start, "invalid #version directive");
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
    if (text == "#extension") {
        return this->extensionDirective(this->position(start));
    }
    if (text == "#version") {
        return this->versionDirective(this->position(start), allowVersion);
    }
    this->error(start, "unsupported directive '" + std::string(this->text(start)) + "'");
}

bool Parser::modifiersDeclarationEnd(const dsl::DSLModifiers& mods) {
    std::unique_ptr<ModifiersDeclaration> decl = ModifiersDeclaration::Convert(fCompiler.context(),
                                                                               mods.fPosition,
                                                                               mods.fModifiers);
    if (!decl) {
        return false;
    }
    ThreadContext::ProgramElements().push_back(std::move(decl));
    return true;
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
        return this->modifiersDeclarationEnd(modifiers);
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
                                    DSLType returnType,
                                    const Token& name) {
    Token lookahead = this->peek();
    bool validParams = true;
    STArray<8, std::unique_ptr<Variable>> parameters;
    if (lookahead.fKind == Token::Kind::TK_RPAREN) {
        // `()` means no parameters at all.
    } else if (lookahead.fKind == Token::Kind::TK_IDENTIFIER && this->text(lookahead) == "void") {
        // `(void)` also means no parameters at all.
        this->nextToken();
    } else {
        for (;;) {
            std::unique_ptr<SkSL::Variable> param;
            if (!this->parameter(&param)) {
                return false;
            }
            validParams = validParams && param;
            parameters.push_back(std::move(param));
            if (!this->checkNext(Token::Kind::TK_COMMA)) {
                break;
            }
        }
    }
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
        return false;
    }

    SkSL::FunctionDeclaration* decl = nullptr;
    if (validParams) {
        decl = SkSL::FunctionDeclaration::Convert(ThreadContext::Context(),
                                                  this->rangeFrom(start),
                                                  modifiers.fPosition,
                                                  &modifiers.fModifiers,
                                                  this->text(name),
                                                  std::move(parameters),
                                                  start,
                                                  &returnType.skslType());
    }

    if (this->checkNext(Token::Kind::TK_SEMICOLON)) {
        return this->prototypeFunction(decl);
    } else {
        return this->defineFunction(decl);
    }
}

bool Parser::prototypeFunction(SkSL::FunctionDeclaration* decl) {
    if (!decl) {
        return false;
    }
    ThreadContext::ProgramElements().push_back(std::make_unique<SkSL::FunctionPrototype>(
            decl->fPosition, decl, fCompiler.context().fConfig->fIsBuiltinCode));
    return true;
}

bool Parser::defineFunction(SkSL::FunctionDeclaration* decl) {
    // Create a symbol table for the function parameters.
    const Context& context = fCompiler.context();
    AutoSymbolTable symbols(this);
    if (decl) {
        decl->addParametersToSymbolTable(context);
    }

    // Parse the function body.
    Token bodyStart = this->peek();
    std::optional<DSLStatement> body = this->block();

    // If there was a problem with the declarations or body, don't actually create a definition.
    if (!decl || !body) {
        return false;
    }

    std::unique_ptr<SkSL::Statement> block = body->release();
    SkASSERT(block->is<Block>());
    Position pos = this->rangeFrom(bodyStart);
    block->fPosition = pos;

    std::unique_ptr<FunctionDefinition> function = FunctionDefinition::Convert(context,
                                                                               pos,
                                                                               *decl,
                                                                               std::move(block),
                                                                               /*builtin=*/false);
    if (!function) {
        return false;
    }
    decl->setDefinition(function.get());
    ThreadContext::ProgramElements().push_back(std::move(function));
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
        *initializer = this->assignmentExpression();
        return initializer->hasValue();
    }
    return true;
}

void Parser::addGlobalVarDeclaration(std::unique_ptr<SkSL::VarDeclaration> decl) {
    if (decl) {
        ThreadContext::ProgramElements().push_back(
                std::make_unique<SkSL::GlobalVarDeclaration>(std::move(decl)));
    }
}

/* (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)? (COMMA IDENTIFER
   (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)?)* SEMICOLON */
void Parser::globalVarDeclarationEnd(Position pos,
                                     const dsl::DSLModifiers& mods,
                                     dsl::DSLType baseType,
                                     Token name) {
    DSLType type = baseType;
    DSLExpression initializer;
    if (!this->parseArrayDimensions(pos, &type)) {
        return;
    }
    if (!this->parseInitializer(pos, &initializer)) {
        return;
    }
    this->addGlobalVarDeclaration(VarDeclaration::Convert(fCompiler.context(),
                                                          this->rangeFrom(pos),
                                                          mods.fPosition,
                                                          mods.fModifiers,
                                                          type.skslType(),
                                                          this->position(name),
                                                          this->text(name),
                                                          VariableStorage::kGlobal,
                                                          initializer.releaseIfPossible()));
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
        this->addGlobalVarDeclaration(
                VarDeclaration::Convert(fCompiler.context(),
                                        this->rangeFrom(identifierName),
                                        mods.fPosition,
                                        mods.fModifiers,
                                        type.skslType(),
                                        this->position(identifierName),
                                        this->text(identifierName),
                                        VariableStorage::kGlobal,
                                        anotherInitializer.releaseIfPossible()));
    }
    this->expect(Token::Kind::TK_SEMICOLON, "';'");
}

/* (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)? (COMMA IDENTIFER
   (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)?)* SEMICOLON */
DSLStatement Parser::localVarDeclarationEnd(Position pos,
                                            const dsl::DSLModifiers& mods,
                                            dsl::DSLType baseType,
                                            Token name) {
    DSLType type = baseType;
    DSLExpression initializer;
    if (!this->parseArrayDimensions(pos, &type)) {
        return {};
    }
    if (!this->parseInitializer(pos, &initializer)) {
        return {};
    }
    std::unique_ptr<Statement> result = VarDeclaration::Convert(fCompiler.context(),
                                                                this->rangeFrom(pos),
                                                                mods.fPosition,
                                                                mods.fModifiers,
                                                                type.skslType(),
                                                                this->position(name),
                                                                this->text(name),
                                                                VariableStorage::kLocal,
                                                                initializer.releaseIfPossible());
    for (;;) {
        if (!this->checkNext(Token::Kind::TK_COMMA)) {
            this->expect(Token::Kind::TK_SEMICOLON, "';'");
            break;
        }
        type = baseType;
        Token identifierName;
        if (!this->expectIdentifier(&identifierName)) {
            break;
        }
        if (!this->parseArrayDimensions(pos, &type)) {
            break;
        }
        DSLExpression anotherInitializer;
        if (!this->parseInitializer(pos, &anotherInitializer)) {
            break;
        }
        std::unique_ptr<Statement> next =
                VarDeclaration::Convert(fCompiler.context(),
                                        this->rangeFrom(identifierName),
                                        mods.fPosition,
                                        mods.fModifiers,
                                        type.skslType(),
                                        this->position(identifierName),
                                        this->text(identifierName),
                                        VariableStorage::kLocal,
                                        anotherInitializer.releaseIfPossible());

        result = Block::MakeCompoundStatement(std::move(result), std::move(next));
    }
    return DSLStatement(std::move(result), this->rangeFrom(pos));
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
    AutoDepth depth(this);
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
    if (!depth.increase()) {
        return DSLType(nullptr);
    }
    TArray<SkSL::Field> fields;
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

            fields.push_back(SkSL::Field(this->rangeFrom(fieldStart),
                                         modifiers.fModifiers,
                                         this->text(memberName),
                                         &actualType.skslType()));
        } while (this->checkNext(Token::Kind::TK_COMMA));

        if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
            return DSLType(nullptr);
        }
    }
    std::unique_ptr<SkSL::StructDefinition> def = StructDefinition::Convert(fCompiler.context(),
                                                                            this->rangeFrom(start),
                                                                            this->text(name),
                                                                            std::move(fields));
    if (!def) {
        return DSLType(nullptr);
    }

    DSLType result(&def->type());
    ThreadContext::ProgramElements().push_back(std::move(def));
    return result;
}

/* structDeclaration ((IDENTIFIER varDeclarationEnd) | SEMICOLON) */
void Parser::structVarDeclaration(Position start, const DSLModifiers& modifiers) {
    DSLType type = this->structDeclaration();
    if (!type.hasValue()) {
        return;
    }
    Token name;
    if (this->checkIdentifier(&name)) {
        this->globalVarDeclarationEnd(this->rangeFrom(name), modifiers, type, name);
    } else {
        this->expect(Token::Kind::TK_SEMICOLON, "';'");
    }
}

/* modifiers type IDENTIFIER (LBRACKET INT_LITERAL RBRACKET)? */
bool Parser::parameter(std::unique_ptr<SkSL::Variable>* outParam) {
    Position pos = this->position(this->peek());
    DSLModifiers modifiers = this->modifiers();
    DSLType type = this->type(&modifiers);
    if (!type.hasValue()) {
        return false;
    }
    Token name;
    std::string_view nameText;
    Position namePos;
    if (this->checkIdentifier(&name)) {
        nameText = this->text(name);
        namePos = this->position(name);
    } else {
        namePos = this->rangeFrom(pos);
    }
    if (!this->parseArrayDimensions(pos, &type)) {
        return false;
    }
    *outParam = SkSL::Variable::Convert(fCompiler.context(),
                                        this->rangeFrom(pos),
                                        modifiers.fPosition,
                                        modifiers.fModifiers,
                                        &type.skslType(),
                                        namePos,
                                        nameText,
                                        VariableStorage::kParameter);
    return true;
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
SkSL::Layout Parser::layout() {
    using LayoutMap = THashMap<std::string_view, SkSL::Layout::Flag>;
    static SkNoDestructor<LayoutMap> sLayoutTokens(LayoutMap{
            {"location",                    SkSL::Layout::kLocation_Flag},
            {"offset",                      SkSL::Layout::kOffset_Flag},
            {"binding",                     SkSL::Layout::kBinding_Flag},
            {"texture",                     SkSL::Layout::kTexture_Flag},
            {"sampler",                     SkSL::Layout::kSampler_Flag},
            {"index",                       SkSL::Layout::kIndex_Flag},
            {"set",                         SkSL::Layout::kSet_Flag},
            {"builtin",                     SkSL::Layout::kBuiltin_Flag},
            {"input_attachment_index",      SkSL::Layout::kInputAttachmentIndex_Flag},
            {"origin_upper_left",           SkSL::Layout::kOriginUpperLeft_Flag},
            {"blend_support_all_equations", SkSL::Layout::kBlendSupportAllEquations_Flag},
            {"push_constant",               SkSL::Layout::kPushConstant_Flag},
            {"color",                       SkSL::Layout::kColor_Flag},
            {"spirv",                       SkSL::Layout::kSPIRV_Flag},
            {"metal",                       SkSL::Layout::kMetal_Flag},
            {"gl",                          SkSL::Layout::kGL_Flag},
            {"wgsl",                        SkSL::Layout::kWGSL_Flag},
    });

    Layout result;
    if (this->checkNext(Token::Kind::TK_LAYOUT) &&
        this->expect(Token::Kind::TK_LPAREN, "'('")) {

        for (;;) {
            Token t = this->nextToken();
            std::string_view text = this->text(t);
            SkSL::Layout::Flag* found = sLayoutTokens->find(text);

            if (!found) {
                this->error(t, "'" + std::string(text) + "' is not a valid layout qualifier");
            } else {
                if (result.fFlags & *found) {
                    this->error(t, "layout qualifier '" + std::string(text) +
                                   "' appears more than once");
                }

                result.fFlags |= *found;

                switch (*found) {
                    case SkSL::Layout::kLocation_Flag:
                        result.fLocation = this->layoutInt();
                        break;
                    case SkSL::Layout::kOffset_Flag:
                        result.fOffset = this->layoutInt();
                        break;
                    case SkSL::Layout::kBinding_Flag:
                        result.fBinding = this->layoutInt();
                        break;
                    case SkSL::Layout::kIndex_Flag:
                        result.fIndex = this->layoutInt();
                        break;
                    case SkSL::Layout::kSet_Flag:
                        result.fSet = this->layoutInt();
                        break;
                    case SkSL::Layout::kTexture_Flag:
                        result.fTexture = this->layoutInt();
                        break;
                    case SkSL::Layout::kSampler_Flag:
                        result.fSampler = this->layoutInt();
                        break;
                    case SkSL::Layout::kBuiltin_Flag:
                        result.fBuiltin = this->layoutInt();
                        break;
                    case SkSL::Layout::kInputAttachmentIndex_Flag:
                        result.fInputAttachmentIndex = this->layoutInt();
                        break;
                    default:
                        break;
                }
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
    SkSL::Layout layout = this->layout();
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
    return DSLModifiers{Modifiers(layout, flags), Position::Range(start, end)};
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
            std::optional<DSLStatement> result = this->block();
            return result ? std::move(*result) : DSLStatement();
        }
        case Token::Kind::TK_SEMICOLON:
            this->nextToken();
            return DSLStatement(Nop::Make());
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
    DSLType result(this->text(type), this->position(type),
                   &modifiers->fModifiers, modifiers->fPosition);
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
    TArray<SkSL::Field> fields;
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

            fields.push_back(SkSL::Field(this->rangeFrom(fieldPos),
                                         fieldModifiers.fModifiers,
                                         this->text(fieldName),
                                         &actualType.skslType()));
        } while (this->checkNext(Token::Kind::TK_COMMA));
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
    this->expect(Token::Kind::TK_SEMICOLON, "';'");

    if (std::unique_ptr<SkSL::InterfaceBlock> ib = InterfaceBlock::Convert(fCompiler.context(),
                                                                           this->position(typeName),
                                                                           modifiers.fModifiers,
                                                                           modifiers.fPosition,
                                                                           this->text(typeName),
                                                                           std::move(fields),
                                                                           instanceName,
                                                                           size)) {
        ThreadContext::ProgramElements().push_back(std::move(ib));
        return true;
    }
    return false;
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
    return DSLStatement(IfStatement::Convert(fCompiler.context(),
                                             pos,
                                             test.release(),
                                             ifTrue.release(),
                                             ifFalse.releaseIfPossible()), pos);
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
    Position pos = this->rangeFrom(start);
    return DSLStatement(DoStatement::Convert(fCompiler.context(), pos,
                                             statement.release(), test.release()), pos);
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
    Position pos = this->rangeFrom(start);
    return DSLStatement(ForStatement::ConvertWhile(fCompiler.context(), pos,
                                                   test.release(),
                                                   statement.release()), pos);
}

/* COLON statement* */
bool Parser::switchCaseBody(ExpressionArray* values,
                            StatementArray* caseBlocks,
                            std::unique_ptr<Expression> caseValue) {
    if (!this->expect(Token::Kind::TK_COLON, "':'")) {
        return false;
    }
    StatementArray statements;
    while (this->peek().fKind != Token::Kind::TK_RBRACE &&
           this->peek().fKind != Token::Kind::TK_CASE &&
           this->peek().fKind != Token::Kind::TK_DEFAULT) {
        DSLStatement s = this->statement();
        if (!s.hasValue()) {
            return false;
        }
        statements.push_back(s.release());
    }
    values->push_back(std::move(caseValue));
    caseBlocks->push_back(SkSL::Block::Make(Position(), std::move(statements),
                                            Block::Kind::kUnbracedBlock));
    return true;
}

/* CASE expression COLON statement* */
bool Parser::switchCase(ExpressionArray* values, StatementArray* caseBlocks) {
    Token start;
    if (!this->expect(Token::Kind::TK_CASE, "'case'", &start)) {
        return false;
    }
    DSLExpression caseValue = this->expression();
    if (!caseValue.hasValue()) {
        return false;
    }
    return this->switchCaseBody(values, caseBlocks, caseValue.release());
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

    ExpressionArray values;
    StatementArray caseBlocks;
    while (this->peek().fKind == Token::Kind::TK_CASE) {
        if (!this->switchCase(&values, &caseBlocks)) {
            return {};
        }
    }
    // Requiring `default:` to be last (in defiance of C and GLSL) was a deliberate decision. Other
    // parts of the compiler are allowed to rely upon this assumption.
    if (this->checkNext(Token::Kind::TK_DEFAULT)) {
        if (!this->switchCaseBody(&values, &caseBlocks, /*value=*/nullptr)) {
            return {};
        }
    }
    if (!this->expect(Token::Kind::TK_RBRACE, "'}'")) {
        return {};
    }
    Position pos = this->rangeFrom(start);
    return DSLStatement(SwitchStatement::Convert(fCompiler.context(), pos,
                                                 value.release(),
                                                 std::move(values),
                                                 std::move(caseBlocks)), pos);
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
        test = this->expression();
        if (!test.hasValue()) {
            return {};
        }
    }
    Token secondSemicolon;
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'", &secondSemicolon)) {
        return {};
    }
    dsl::DSLExpression next;
    if (this->peek().fKind != Token::Kind::TK_RPAREN) {
        next = this->expression();
        if (!next.hasValue()) {
            return {};
        }
    }
    Token rparen;
    if (!this->expect(Token::Kind::TK_RPAREN, "')'", &rparen)) {
        return {};
    }
    dsl::DSLStatement statement = this->statement();
    if (!statement.hasValue()) {
        return {};
    }
    Position pos = this->rangeFrom(start);
    ForLoopPositions loopPositions{
            range_of_at_least_one_char(lparen.fOffset + 1, firstSemicolonOffset),
            range_of_at_least_one_char(firstSemicolonOffset + 1, secondSemicolon.fOffset),
            range_of_at_least_one_char(secondSemicolon.fOffset + 1, rparen.fOffset),
    };
    return DSLStatement(ForStatement::Convert(fCompiler.context(), pos, loopPositions,
                                              initializer.releaseIfPossible(),
                                              test.releaseIfPossible(),
                                              next.releaseIfPossible(),
                                              statement.release()), pos);
}

/* RETURN expression? SEMICOLON */
DSLStatement Parser::returnStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_RETURN, "'return'", &start)) {
        return {};
    }
    DSLExpression expression;
    if (this->peek().fKind != Token::Kind::TK_SEMICOLON) {
        expression = this->expression();
        if (!expression.hasValue()) {
            return {};
        }
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return {};
    }
    // We do not check for errors, or coerce the value to the correct type, until the return
    // statement is actually added to a function. (This is done in FunctionDefinition::Convert.)
    return ReturnStatement::Make(this->rangeFrom(start), expression.releaseIfPossible());
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
    return SkSL::BreakStatement::Make(this->position(start));
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
    return SkSL::ContinueStatement::Make(this->position(start));
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
    Position pos = this->position(start);
    return DSLStatement(SkSL::DiscardStatement::Convert(fCompiler.context(), pos), pos);
}

/* LBRACE statement* RBRACE */
std::optional<DSLStatement> Parser::block() {
    AutoDepth depth(this);
    Token start;
    if (!this->expect(Token::Kind::TK_LBRACE, "'{'", &start)) {
        return std::nullopt;
    }
    if (!depth.increase()) {
        return std::nullopt;
    }
    AutoSymbolTable symbols(this);
    StatementArray statements;
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_RBRACE: {
                this->nextToken();
                Position pos = this->rangeFrom(start);
                return DSLStatement(SkSL::Block::MakeBlock(pos, std::move(statements),
                                                           Block::Kind::kBracedScope,
                                                           this->symbolTable()), pos);
            }
            case Token::Kind::TK_END_OF_FILE: {
                this->error(this->peek(), "expected '}', but found end of file");
                return std::nullopt;
            }
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
                           DSLExpression& expr) {
    this->nextToken();
    if (!depth.increase()) {
        return false;
    }
    DSLExpression right = (this->*rightFn)();
    if (!right.hasValue()) {
        return false;
    }
    Position pos = expr.position().rangeThrough(right.position());
    expr = DSLExpression(BinaryExpression::Convert(fCompiler.context(), pos,
                                                   expr.release(), op, right.release()), pos);
    return true;
}

/* assignmentExpression (COMMA assignmentExpression)* */
DSLExpression Parser::expression() {
    AutoDepth depth(this);
    [[maybe_unused]] Token start = this->peek();
    DSLExpression result = this->assignmentExpression();
    if (!result.hasValue()) {
        return {};
    }
    while (this->peek().fKind == Token::Kind::TK_COMMA) {
        if (!this->operatorRight(depth, Operator::Kind::COMMA, &Parser::assignmentExpression,
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
        Operator::Kind op;
        switch (this->peek().fKind) {
            case Token::Kind::TK_EQ:           op = Operator::Kind::EQ;           break;
            case Token::Kind::TK_STAREQ:       op = Operator::Kind::STAREQ;       break;
            case Token::Kind::TK_SLASHEQ:      op = Operator::Kind::SLASHEQ;      break;
            case Token::Kind::TK_PERCENTEQ:    op = Operator::Kind::PERCENTEQ;    break;
            case Token::Kind::TK_PLUSEQ:       op = Operator::Kind::PLUSEQ;       break;
            case Token::Kind::TK_MINUSEQ:      op = Operator::Kind::MINUSEQ;      break;
            case Token::Kind::TK_SHLEQ:        op = Operator::Kind::SHLEQ;        break;
            case Token::Kind::TK_SHREQ:        op = Operator::Kind::SHREQ;        break;
            case Token::Kind::TK_BITWISEANDEQ: op = Operator::Kind::BITWISEANDEQ; break;
            case Token::Kind::TK_BITWISEXOREQ: op = Operator::Kind::BITWISEXOREQ; break;
            case Token::Kind::TK_BITWISEOREQ:  op = Operator::Kind::BITWISEOREQ;  break;
            default:                           return result;
        }
        if (!this->operatorRight(depth, op, &Parser::assignmentExpression, result)) {
            return {};
        }
    }
}

/* logicalOrExpression ('?' expression ':' assignmentExpression)? */
DSLExpression Parser::ternaryExpression() {
    AutoDepth depth(this);
    DSLExpression base = this->logicalOrExpression();
    if (!base.hasValue()) {
        return {};
    }
    if (!this->checkNext(Token::Kind::TK_QUESTION)) {
        return base;
    }
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
    return DSLExpression(TernaryExpression::Convert(fCompiler.context(), pos, base.release(),
                                                    trueExpr.release(), falseExpr.release()), pos);
}

/* logicalXorExpression (LOGICALOR logicalXorExpression)* */
DSLExpression Parser::logicalOrExpression() {
    AutoDepth depth(this);
    DSLExpression result = this->logicalXorExpression();
    if (!result.hasValue()) {
        return {};
    }
    while (this->peek().fKind == Token::Kind::TK_LOGICALOR) {
        if (!this->operatorRight(depth, Operator::Kind::LOGICALOR, &Parser::logicalXorExpression,
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
        if (!this->operatorRight(depth, Operator::Kind::LOGICALXOR, &Parser::logicalAndExpression,
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
        if (!this->operatorRight(depth, Operator::Kind::LOGICALAND, &Parser::bitwiseOrExpression,
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
        if (!this->operatorRight(depth, Operator::Kind::BITWISEOR, &Parser::bitwiseXorExpression,
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
        if (!this->operatorRight(depth, Operator::Kind::BITWISEXOR, &Parser::bitwiseAndExpression,
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
        if (!this->operatorRight(depth, Operator::Kind::BITWISEAND, &Parser::equalityExpression,
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
        Operator::Kind op;
        switch (this->peek().fKind) {
            case Token::Kind::TK_EQEQ: op = Operator::Kind::EQEQ; break;
            case Token::Kind::TK_NEQ:  op = Operator::Kind::NEQ;  break;
            default:                   return result;
        }
        if (!this->operatorRight(depth, op, &Parser::relationalExpression, result)) {
            return {};
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
        Operator::Kind op;
        switch (this->peek().fKind) {
            case Token::Kind::TK_LT:   op = Operator::Kind::LT;   break;
            case Token::Kind::TK_GT:   op = Operator::Kind::GT;   break;
            case Token::Kind::TK_LTEQ: op = Operator::Kind::LTEQ; break;
            case Token::Kind::TK_GTEQ: op = Operator::Kind::GTEQ; break;
            default:                   return result;
        }
        if (!this->operatorRight(depth, op, &Parser::shiftExpression, result)) {
            return {};
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
        Operator::Kind op;
        switch (this->peek().fKind) {
            case Token::Kind::TK_SHL: op = Operator::Kind::SHL; break;
            case Token::Kind::TK_SHR: op = Operator::Kind::SHR; break;
            default:                  return result;
        }
        if (!this->operatorRight(depth, op, &Parser::additiveExpression, result)) {
            return {};
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
        Operator::Kind op;
        switch (this->peek().fKind) {
            case Token::Kind::TK_PLUS:  op = Operator::Kind::PLUS;  break;
            case Token::Kind::TK_MINUS: op = Operator::Kind::MINUS; break;
            default:                    return result;
        }
        if (!this->operatorRight(depth, op, &Parser::multiplicativeExpression, result)) {
            return {};
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
        Operator::Kind op;
        switch (this->peek().fKind) {
            case Token::Kind::TK_STAR:    op = Operator::Kind::STAR;    break;
            case Token::Kind::TK_SLASH:   op = Operator::Kind::SLASH;   break;
            case Token::Kind::TK_PERCENT: op = Operator::Kind::PERCENT; break;
            default:                      return result;
        }
        if (!this->operatorRight(depth, op, &Parser::unaryExpression, result)) {
            return {};
        }
    }
}

/* postfixExpression | (PLUS | MINUS | NOT | PLUSPLUS | MINUSMINUS) unaryExpression */
DSLExpression Parser::unaryExpression() {
    AutoDepth depth(this);
    Operator::Kind op;
    Token start = this->peek();
    switch (start.fKind) {
        case Token::Kind::TK_PLUS:       op = Operator::Kind::PLUS;       break;
        case Token::Kind::TK_MINUS:      op = Operator::Kind::MINUS;      break;
        case Token::Kind::TK_LOGICALNOT: op = Operator::Kind::LOGICALNOT; break;
        case Token::Kind::TK_BITWISENOT: op = Operator::Kind::BITWISENOT; break;
        case Token::Kind::TK_PLUSPLUS:   op = Operator::Kind::PLUSPLUS;   break;
        case Token::Kind::TK_MINUSMINUS: op = Operator::Kind::MINUSMINUS; break;
        default:                         return this->postfixExpression();
    }
    this->nextToken();
    if (!depth.increase()) {
        return {};
    }
    DSLExpression expr = this->unaryExpression();
    if (!expr.hasValue()) {
        return {};
    }
    Position pos = Position::Range(start.fOffset, expr.position().endOffset());
    return DSLExpression(PrefixExpression::Convert(fCompiler.context(),
                                                   pos, op, expr.release()), pos);
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
                result = this->suffix(std::move(result));
                if (!result.hasValue()) {
                    return {};
                }
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
        return DSLExpression(
                FieldAccess::Convert(fCompiler.context(), pos, base.release(), swizzleMask),
                pos);
    }
    return DSLExpression(Swizzle::Convert(fCompiler.context(), pos, maskPos,
                                          base.release(), swizzleMask), pos);
}

dsl::DSLExpression Parser::call(Position pos, dsl::DSLExpression base, ExpressionArray args) {
    return DSLExpression(SkSL::FunctionCall::Convert(fCompiler.context(), pos, base.release(),
                                                     std::move(args)), pos);
}

/* LBRACKET expression? RBRACKET | DOT IDENTIFIER | LPAREN arguments RPAREN |
   PLUSPLUS | MINUSMINUS | COLONCOLON IDENTIFIER | FLOAT_LITERAL [IDENTIFIER] */
DSLExpression Parser::suffix(DSLExpression base) {
    AutoDepth depth(this);
    Token next = this->nextToken();
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

            Position pos = this->rangeFrom(base.position());
            return DSLExpression(IndexExpression::Convert(fCompiler.context(), pos,
                                                          base.release(), index.release()), pos);
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
        case Token::Kind::TK_MINUSMINUS: {
            Operator::Kind op = (next.fKind == Token::Kind::TK_PLUSPLUS)
                                        ? Operator::Kind::PLUSPLUS
                                        : Operator::Kind::MINUSMINUS;
            Position pos = this->rangeFrom(base.position());
            return DSLExpression(
                    PostfixExpression::Convert(fCompiler.context(), pos, base.release(), op),
                    pos);
        }
        default: {
            this->error(next, "expected expression suffix, but found '" +
                              std::string(this->text(next)) + "'");
            return {};
        }
    }
}

/* IDENTIFIER | intLiteral | floatLiteral | boolLiteral | '(' expression ')' */
DSLExpression Parser::term() {
    AutoDepth depth(this);
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
            Position pos = this->position(t);
            return DSLExpression(SkSL::Literal::MakeInt(fCompiler.context(), pos, i), pos);
        }
        case Token::Kind::TK_FLOAT_LITERAL: {
            SKSL_FLOAT f;
            if (!this->floatLiteral(&f)) {
                f = 0.0f;
            }
            Position pos = this->position(t);
            return DSLExpression(SkSL::Literal::MakeFloat(fCompiler.context(), pos, f), pos);
        }
        case Token::Kind::TK_TRUE_LITERAL: // fall through
        case Token::Kind::TK_FALSE_LITERAL: {
            bool b;
            SkAssertResult(this->boolLiteral(&b));
            Position pos = this->position(t);
            return DSLExpression(SkSL::Literal::MakeBool(fCompiler.context(), pos, b), pos);
        }
        case Token::Kind::TK_LPAREN: {
            this->nextToken();
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
