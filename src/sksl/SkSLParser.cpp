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
#include "src/base/SkEnumBitMask.h"
#include "src/base/SkNoDestructor.h"
#include "src/core/SkTHash.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLConstantFolder.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLModule.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/SkSLString.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLBreakStatement.h"
#include "src/sksl/ir/SkSLContinueStatement.h"
#include "src/sksl/ir/SkSLDiscardStatement.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
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
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLModifiersDeclaration.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLPoison.h"
#include "src/sksl/ir/SkSLPostfixExpression.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLStructDefinition.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLSymbol.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLTernaryExpression.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLTypeReference.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"

#include <algorithm>
#include <climits>
#include <initializer_list>
#include <memory>
#include <utility>
#include <vector>

using namespace skia_private;

namespace SkSL {

static constexpr int kMaxParseDepth = 50;

static ModifierFlags parse_modifier_token(Token::Kind token) {
    switch (token) {
        case Token::Kind::TK_UNIFORM:        return ModifierFlag::kUniform;
        case Token::Kind::TK_CONST:          return ModifierFlag::kConst;
        case Token::Kind::TK_IN:             return ModifierFlag::kIn;
        case Token::Kind::TK_OUT:            return ModifierFlag::kOut;
        case Token::Kind::TK_INOUT:          return ModifierFlag::kIn | ModifierFlag::kOut;
        case Token::Kind::TK_FLAT:           return ModifierFlag::kFlat;
        case Token::Kind::TK_NOPERSPECTIVE:  return ModifierFlag::kNoPerspective;
        case Token::Kind::TK_PURE:           return ModifierFlag::kPure;
        case Token::Kind::TK_INLINE:         return ModifierFlag::kInline;
        case Token::Kind::TK_NOINLINE:       return ModifierFlag::kNoInline;
        case Token::Kind::TK_HIGHP:          return ModifierFlag::kHighp;
        case Token::Kind::TK_MEDIUMP:        return ModifierFlag::kMediump;
        case Token::Kind::TK_LOWP:           return ModifierFlag::kLowp;
        case Token::Kind::TK_EXPORT:         return ModifierFlag::kExport;
        case Token::Kind::TK_ES3:            return ModifierFlag::kES3;
        case Token::Kind::TK_WORKGROUP:      return ModifierFlag::kWorkgroup;
        case Token::Kind::TK_READONLY:       return ModifierFlag::kReadOnly;
        case Token::Kind::TK_WRITEONLY:      return ModifierFlag::kWriteOnly;
        case Token::Kind::TK_BUFFER:         return ModifierFlag::kBuffer;
        case Token::Kind::TK_PIXELLOCAL:     return ModifierFlag::kPixelLocal;
        default:                             return ModifierFlag::kNone;
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
    AutoSymbolTable(Parser* p, std::unique_ptr<SymbolTable>* newSymbolTable, bool enable = true) {
        if (enable) {
            fParser = p;
            SymbolTable*& ctxSymbols = this->contextSymbolTable();
            *newSymbolTable = std::make_unique<SymbolTable>(ctxSymbols, ctxSymbols->isBuiltin());
            ctxSymbols = newSymbolTable->get();
        }
    }

    ~AutoSymbolTable() {
        if (fParser) {
            SymbolTable*& ctxSymbols = this->contextSymbolTable();
            ctxSymbols = ctxSymbols->fParent;
        }
    }

private:
    SymbolTable*& contextSymbolTable() { return fParser->fCompiler.context().fSymbolTable; }

    Parser* fParser = nullptr;
};

class Parser::Checkpoint {
public:
    Checkpoint(Parser* p) : fParser(p) {
        Context& context = fParser->fCompiler.context();
        fPushbackCheckpoint = fParser->fPushback;
        fLexerCheckpoint = fParser->fLexer.getCheckpoint();
        fOldErrorReporter = context.fErrors;
        fOldEncounteredFatalError = fParser->fEncounteredFatalError;
        SkASSERT(fOldErrorReporter);
        context.setErrorReporter(&fErrorReporter);
    }

    ~Checkpoint() {
        SkASSERTF(!fOldErrorReporter, "Checkpoint was not accepted or rewound before destruction");
    }

    void accept() {
        this->restoreErrorReporter();
        // Parser errors should have been fatal, but we can encounter other errors like type
        // mismatches despite accepting the parse. Forward those messages to the actual error
        // handler now.
        fErrorReporter.forwardErrors(fParser);
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

        void forwardErrors(Parser* parser) {
            for (const Error& error : fErrors) {
                parser->error(error.fPos, error.fMsg);
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
        fParser->fCompiler.context().setErrorReporter(fOldErrorReporter);
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
               std::unique_ptr<std::string> text)
        : fCompiler(*compiler)
        , fSettings(settings)
        , fKind(kind)
        , fText(std::move(text))
        , fPushback(Token::Kind::TK_NONE, /*offset=*/-1, /*length=*/-1) {
    fLexer.start(*fText);
}

Parser::~Parser() = default;

SymbolTable* Parser::symbolTable() {
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
    fPushback = t;
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
    this->pushback(next);
    return false;
}

bool Parser::expect(Token::Kind kind, const char* expected, Token* result) {
    Token next = this->nextToken();
    if (next.fKind == kind) {
        if (result) {
            *result = next;
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
        this->pushback(*result);
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
    fCompiler.context().fErrors->error(position, msg);
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
std::unique_ptr<Program> Parser::programInheritingFrom(const SkSL::Module* module) {
    this->declarations();
    std::unique_ptr<Program> result;
    if (fCompiler.errorReporter().errorCount() == 0) {
        result = fCompiler.releaseProgram(std::move(fText), std::move(fProgramElements));
    } else {
        fProgramElements.clear();
    }
    return result;
}

std::unique_ptr<SkSL::Module> Parser::moduleInheritingFrom(const SkSL::Module* parentModule) {
    this->declarations();
    this->symbolTable()->takeOwnershipOfString(std::move(*fText));
    auto result = std::make_unique<SkSL::Module>();
    result->fParent = parentModule;
    result->fSymbols = std::move(fCompiler.fGlobalSymbols);
    result->fElements = std::move(fProgramElements);
    result->fModuleType = fCompiler.context().fConfig->fModuleType;
    return result;
}

void Parser::declarations() {
    fEncounteredFatalError = false;

    // If the program is 8MB or longer (Position::kMaxOffset), error reporting goes off the rails.
    // At any rate, there's no good reason for a program to be this long.
    if (fText->size() >= Position::kMaxOffset) {
        this->error(Position(), "program is too large");
        return;
    }

    // Any #version directive must appear as the first thing in a file
    if (this->peek().fKind == Token::Kind::TK_DIRECTIVE) {
        this->directive(/*allowVersion=*/true);
    }

    while (!fEncounteredFatalError) {
        // We should always be at global scope when processing top-level declarations.
        SkASSERT(fCompiler.context().fSymbolTable == fCompiler.globalSymbols());

        switch (this->peek().fKind) {
            case Token::Kind::TK_END_OF_FILE:
                return;

            case Token::Kind::TK_INVALID:
                this->error(this->peek(), "invalid token");
                return;

            case Token::Kind::TK_DIRECTIVE:
                this->directive(/*allowVersion=*/false);
                break;

            default:
                this->declaration();
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
            fProgramElements.push_back(std::move(ext));
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

bool Parser::modifiersDeclarationEnd(const SkSL::Modifiers& mods) {
    std::unique_ptr<ModifiersDeclaration> decl = ModifiersDeclaration::Convert(fCompiler.context(),
                                                                               mods);
    if (!decl) {
        return false;
    }
    fProgramElements.push_back(std::move(decl));
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
    Modifiers modifiers = this->modifiers();
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
    const Type* type = this->type(&modifiers);
    if (!type) {
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
                                    Modifiers& modifiers,
                                    const Type* returnType,
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
        decl = SkSL::FunctionDeclaration::Convert(fCompiler.context(),
                                                  this->rangeFrom(start),
                                                  modifiers,
                                                  this->text(name),
                                                  std::move(parameters),
                                                  start,
                                                  returnType);
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
    fProgramElements.push_back(std::make_unique<SkSL::FunctionPrototype>(decl->fPosition, decl));
    return true;
}

bool Parser::defineFunction(SkSL::FunctionDeclaration* decl) {
    const Context& context = fCompiler.context();
    Token bodyStart = this->peek();

    std::unique_ptr<SymbolTable> symbolTable;
    std::unique_ptr<Statement> body;
    {
        // Create a symbol table for the function which includes the parameters.
        AutoSymbolTable symbols(this, &symbolTable);
        if (decl) {
            for (Variable* param : decl->parameters()) {
                symbolTable->addWithoutOwnership(fCompiler.context(), param);
            }
        }

        // Parse the function body.
        body = this->block(/*introduceNewScope=*/false, /*adoptExistingSymbolTable=*/&symbolTable);
    }

    // If there was a problem with the declarations or body, don't actually create a definition.
    if (!decl || !body) {
        return false;
    }

    std::unique_ptr<SkSL::Statement> block = std::move(body);
    SkASSERT(block->is<Block>());
    Position pos = this->rangeFrom(bodyStart);
    block->fPosition = pos;

    std::unique_ptr<FunctionDefinition> function = FunctionDefinition::Convert(context,
                                                                               pos,
                                                                               *decl,
                                                                               std::move(block));
    if (!function) {
        return false;
    }
    decl->setDefinition(function.get());
    fProgramElements.push_back(std::move(function));
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
    std::unique_ptr<Expression> sizeLiteral = this->expression();
    if (!sizeLiteral) {
        return false;
    }
    if (!sizeLiteral->is<Poison>()) {
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

const Type* Parser::arrayType(const Type* base, int count, Position pos) {
    const Context& context = fCompiler.context();
    count = base->convertArraySize(context, pos, pos, count);
    if (!count) {
        return context.fTypes.fPoison.get();
    }
    return this->symbolTable()->addArrayDimension(fCompiler.context(), base, count);
}

const Type* Parser::unsizedArrayType(const Type* base, Position pos) {
    const Context& context = fCompiler.context();
    if (!base->checkIfUsableInArray(context, pos)) {
        return context.fTypes.fPoison.get();
    }
    return this->symbolTable()->addArrayDimension(fCompiler.context(), base,
                                                  SkSL::Type::kUnsizedArray);
}

bool Parser::parseArrayDimensions(Position pos, const Type** type) {
    Token next;
    while (this->checkNext(Token::Kind::TK_LBRACKET, &next)) {
        if (this->checkNext(Token::Kind::TK_RBRACKET)) {
            if (this->allowUnsizedArrays()) {
                *type = this->unsizedArrayType(*type, this->rangeFrom(pos));
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
            *type = this->arrayType(*type, size, this->rangeFrom(pos));
        }
    }
    return true;
}

bool Parser::parseInitializer(Position pos, std::unique_ptr<Expression>* initializer) {
    if (this->checkNext(Token::Kind::TK_EQ)) {
        *initializer = this->assignmentExpression();
        return *initializer != nullptr;
    }
    return true;
}

void Parser::addGlobalVarDeclaration(std::unique_ptr<VarDeclaration> decl) {
    if (decl) {
        fProgramElements.push_back(std::make_unique<SkSL::GlobalVarDeclaration>(std::move(decl)));
    }
}

/* (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)? (COMMA IDENTIFER
   (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)?)* SEMICOLON */
void Parser::globalVarDeclarationEnd(Position pos,
                                     const Modifiers& mods,
                                     const Type* baseType,
                                     Token name) {
    const Type* type = baseType;
    std::unique_ptr<Expression> initializer;
    if (!this->parseArrayDimensions(pos, &type)) {
        return;
    }
    if (!this->parseInitializer(pos, &initializer)) {
        return;
    }
    this->addGlobalVarDeclaration(VarDeclaration::Convert(fCompiler.context(),
                                                          this->rangeFrom(pos),
                                                          mods,
                                                          *type,
                                                          this->position(name),
                                                          this->text(name),
                                                          VariableStorage::kGlobal,
                                                          std::move(initializer)));
    while (this->checkNext(Token::Kind::TK_COMMA)) {
        type = baseType;
        Token identifierName;
        if (!this->expectIdentifier(&identifierName)) {
            return;
        }
        if (!this->parseArrayDimensions(pos, &type)) {
            return;
        }
        std::unique_ptr<Expression> anotherInitializer;
        if (!this->parseInitializer(pos, &anotherInitializer)) {
            return;
        }
        this->addGlobalVarDeclaration(VarDeclaration::Convert(fCompiler.context(),
                                                              this->rangeFrom(identifierName),
                                                              mods,
                                                              *type,
                                                              this->position(identifierName),
                                                              this->text(identifierName),
                                                              VariableStorage::kGlobal,
                                                              std::move(anotherInitializer)));
    }
    this->expect(Token::Kind::TK_SEMICOLON, "';'");
}

/* (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)? (COMMA IDENTIFER
   (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)?)* SEMICOLON */
std::unique_ptr<Statement> Parser::localVarDeclarationEnd(Position pos,
                                                          const Modifiers& mods,
                                                          const Type* baseType,
                                                          Token name) {
    const Type* type = baseType;
    std::unique_ptr<Expression> initializer;
    if (!this->parseArrayDimensions(pos, &type)) {
        return nullptr;
    }
    if (!this->parseInitializer(pos, &initializer)) {
        return nullptr;
    }
    std::unique_ptr<Statement> result = VarDeclaration::Convert(fCompiler.context(),
                                                                this->rangeFrom(pos),
                                                                mods,
                                                                *type,
                                                                this->position(name),
                                                                this->text(name),
                                                                VariableStorage::kLocal,
                                                                std::move(initializer));
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
        std::unique_ptr<Expression> anotherInitializer;
        if (!this->parseInitializer(pos, &anotherInitializer)) {
            break;
        }
        std::unique_ptr<Statement> next = VarDeclaration::Convert(fCompiler.context(),
                                                                  this->rangeFrom(identifierName),
                                                                  mods,
                                                                  *type,
                                                                  this->position(identifierName),
                                                                  this->text(identifierName),
                                                                  VariableStorage::kLocal,
                                                                  std::move(anotherInitializer));

        result = Block::MakeCompoundStatement(std::move(result), std::move(next));
    }
    pos = this->rangeFrom(pos);
    return this->statementOrNop(pos, std::move(result));
}

/* (varDeclarations | expressionStatement) */
std::unique_ptr<Statement> Parser::varDeclarationsOrExpressionStatement() {
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
    if (!prefixData->fType) {
        return false;
    }
    return this->expectIdentifier(&prefixData->fName);
}

/* modifiers type IDENTIFIER varDeclarationEnd */
std::unique_ptr<Statement> Parser::varDeclarations() {
    VarDeclarationsPrefix prefix;
    if (!this->varDeclarationsPrefix(&prefix)) {
        return nullptr;
    }
    return this->localVarDeclarationEnd(prefix.fPosition, prefix.fModifiers, prefix.fType,
                                        prefix.fName);
}

/* STRUCT IDENTIFIER LBRACE varDeclaration* RBRACE */
const Type* Parser::structDeclaration() {
    AutoDepth depth(this);
    Position start = this->position(this->peek());
    if (!this->expect(Token::Kind::TK_STRUCT, "'struct'")) {
        return nullptr;
    }
    Token name;
    if (!this->expectIdentifier(&name)) {
        return nullptr;
    }
    if (!this->expect(Token::Kind::TK_LBRACE, "'{'")) {
        return nullptr;
    }
    if (!depth.increase()) {
        return nullptr;
    }
    TArray<SkSL::Field> fields;
    while (!this->checkNext(Token::Kind::TK_RBRACE)) {
        Token fieldStart = this->peek();
        Modifiers modifiers = this->modifiers();
        const Type* type = this->type(&modifiers);
        if (!type) {
            return nullptr;
        }

        do {
            const Type* actualType = type;
            Token memberName;
            if (!this->expectIdentifier(&memberName)) {
                return nullptr;
            }

            while (this->checkNext(Token::Kind::TK_LBRACKET)) {
                SKSL_INT size;
                if (!this->arraySize(&size)) {
                    return nullptr;
                }
                if (!this->expect(Token::Kind::TK_RBRACKET, "']'")) {
                    return nullptr;
                }
                actualType = this->arrayType(actualType, size,
                                             this->rangeFrom(this->position(fieldStart)));
            }

            fields.push_back(SkSL::Field(this->rangeFrom(fieldStart),
                                         modifiers.fLayout,
                                         modifiers.fFlags,
                                         this->text(memberName),
                                         actualType));
        } while (this->checkNext(Token::Kind::TK_COMMA));

        if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
            return nullptr;
        }
    }
    std::unique_ptr<SkSL::StructDefinition> def = StructDefinition::Convert(fCompiler.context(),
                                                                            this->rangeFrom(start),
                                                                            this->text(name),
                                                                            std::move(fields));
    if (!def) {
        return nullptr;
    }

    const Type* result = &def->type();
    fProgramElements.push_back(std::move(def));
    return result;
}

/* structDeclaration ((IDENTIFIER varDeclarationEnd) | SEMICOLON) */
void Parser::structVarDeclaration(Position start, const Modifiers& modifiers) {
    const Type* type = this->structDeclaration();
    if (!type) {
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
    Modifiers modifiers = this->modifiers();
    const Type* type = this->type(&modifiers);
    if (!type) {
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
                                        modifiers.fLayout,
                                        modifiers.fFlags,
                                        type,
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
    using LayoutMap = THashMap<std::string_view, SkSL::LayoutFlag>;
    static SkNoDestructor<LayoutMap> sLayoutTokens(LayoutMap{
            {"location",                    SkSL::LayoutFlag::kLocation},
            {"offset",                      SkSL::LayoutFlag::kOffset},
            {"binding",                     SkSL::LayoutFlag::kBinding},
            {"texture",                     SkSL::LayoutFlag::kTexture},
            {"sampler",                     SkSL::LayoutFlag::kSampler},
            {"index",                       SkSL::LayoutFlag::kIndex},
            {"set",                         SkSL::LayoutFlag::kSet},
            {"builtin",                     SkSL::LayoutFlag::kBuiltin},
            {"input_attachment_index",      SkSL::LayoutFlag::kInputAttachmentIndex},
            {"origin_upper_left",           SkSL::LayoutFlag::kOriginUpperLeft},
            {"blend_support_all_equations", SkSL::LayoutFlag::kBlendSupportAllEquations},
            {"push_constant",               SkSL::LayoutFlag::kPushConstant},
            {"color",                       SkSL::LayoutFlag::kColor},
            {"vulkan",                      SkSL::LayoutFlag::kVulkan},
            {"metal",                       SkSL::LayoutFlag::kMetal},
            {"webgpu",                      SkSL::LayoutFlag::kWebGPU},
            {"direct3d",                    SkSL::LayoutFlag::kDirect3D},
            {"rgba8",                       SkSL::LayoutFlag::kRGBA8},
            {"rgba32f",                     SkSL::LayoutFlag::kRGBA32F},
            {"r32f",                        SkSL::LayoutFlag::kR32F},
            {"local_size_x",                SkSL::LayoutFlag::kLocalSizeX},
            {"local_size_y",                SkSL::LayoutFlag::kLocalSizeY},
            {"local_size_z",                SkSL::LayoutFlag::kLocalSizeZ},
    });

    Layout result;
    if (this->checkNext(Token::Kind::TK_LAYOUT) &&
        this->expect(Token::Kind::TK_LPAREN, "'('")) {

        for (;;) {
            Token t = this->nextToken();
            std::string_view text = this->text(t);
            SkSL::LayoutFlag* found = sLayoutTokens->find(text);

            if (!found) {
                this->error(t, "'" + std::string(text) + "' is not a valid layout qualifier");
            } else {
                if (result.fFlags & *found) {
                    this->error(t, "layout qualifier '" + std::string(text) +
                                   "' appears more than once");
                }

                result.fFlags |= *found;

                switch (*found) {
                    case SkSL::LayoutFlag::kLocation:
                        result.fLocation = this->layoutInt();
                        break;
                    case SkSL::LayoutFlag::kOffset:
                        result.fOffset = this->layoutInt();
                        break;
                    case SkSL::LayoutFlag::kBinding:
                        result.fBinding = this->layoutInt();
                        break;
                    case SkSL::LayoutFlag::kIndex:
                        result.fIndex = this->layoutInt();
                        break;
                    case SkSL::LayoutFlag::kSet:
                        result.fSet = this->layoutInt();
                        break;
                    case SkSL::LayoutFlag::kTexture:
                        result.fTexture = this->layoutInt();
                        break;
                    case SkSL::LayoutFlag::kSampler:
                        result.fSampler = this->layoutInt();
                        break;
                    case SkSL::LayoutFlag::kBuiltin:
                        result.fBuiltin = this->layoutInt();
                        break;
                    case SkSL::LayoutFlag::kInputAttachmentIndex:
                        result.fInputAttachmentIndex = this->layoutInt();
                        break;
                    case SkSL::LayoutFlag::kLocalSizeX:
                        result.fLocalSizeX = this->layoutInt();
                        break;
                    case SkSL::LayoutFlag::kLocalSizeY:
                        result.fLocalSizeY = this->layoutInt();
                        break;
                    case SkSL::LayoutFlag::kLocalSizeZ:
                        result.fLocalSizeZ = this->layoutInt();
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
Modifiers Parser::modifiers() {
    int start = this->peek().fOffset;
    SkSL::Layout layout = this->layout();
    Token raw = this->nextRawToken();
    int end = raw.fOffset;
    if (!is_whitespace(raw.fKind)) {
        this->pushback(raw);
    }
    ModifierFlags flags = ModifierFlag::kNone;
    for (;;) {
        ModifierFlags tokenFlag = parse_modifier_token(peek().fKind);
        if (tokenFlag == ModifierFlag::kNone) {
            break;
        }
        Token modifier = this->nextToken();
        if (ModifierFlags duplicateFlags = (tokenFlag & flags)) {
            this->error(modifier, "'" + duplicateFlags.description() + "' appears more than once");
        }
        flags |= tokenFlag;
        end = this->position(modifier).endOffset();
    }
    return Modifiers{Position::Range(start, end), layout, flags};
}

std::unique_ptr<Statement> Parser::statementOrNop(Position pos, std::unique_ptr<Statement> stmt) {
    if (!stmt) {
        stmt = Nop::Make();
    }
    if (pos.valid() && !stmt->position().valid()) {
        stmt->setPosition(pos);
    }
    return stmt;
}

/* ifStatement | forStatement | doStatement | whileStatement | block | expression */
std::unique_ptr<Statement> Parser::statement(bool bracesIntroduceNewScope) {
    AutoDepth depth(this);
    if (!depth.increase()) {
        return nullptr;
    }
    switch (this->peek().fKind) {
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
        case Token::Kind::TK_LBRACE:
            return this->block(bracesIntroduceNewScope, /*adoptExistingSymbolTable=*/nullptr);
        case Token::Kind::TK_SEMICOLON:
            this->nextToken();
            return Nop::Make();
        case Token::Kind::TK_CONST:
            return this->varDeclarations();
        case Token::Kind::TK_HIGHP:
        case Token::Kind::TK_MEDIUMP:
        case Token::Kind::TK_LOWP:
        case Token::Kind::TK_IDENTIFIER:
            return this->varDeclarationsOrExpressionStatement();
        default:
            return this->expressionStatement();
    }
}

const Type* Parser::findType(Position pos,
                             Modifiers* modifiers,
                             std::string_view name) {
    const Context& context = fCompiler.context();
    const Symbol* symbol = this->symbolTable()->find(name);
    if (!symbol) {
        this->error(pos, "no symbol named '" + std::string(name) + "'");
        return context.fTypes.fPoison.get();
    }
    if (!symbol->is<Type>()) {
        this->error(pos, "symbol '" + std::string(name) + "' is not a type");
        return context.fTypes.fPoison.get();
    }
    const SkSL::Type* type = &symbol->as<Type>();
    if (!context.fConfig->isBuiltinCode()) {
        if (!TypeReference::VerifyType(context, type, pos)) {
            return context.fTypes.fPoison.get();
        }
    }
    Position qualifierRange = modifiers->fPosition;
    if (qualifierRange.startOffset() == qualifierRange.endOffset()) {
        qualifierRange = this->rangeFrom(qualifierRange);
    }
    return modifiers ? type->applyQualifiers(context, &modifiers->fFlags, qualifierRange)
                     : type;
}

/* IDENTIFIER(type) (LBRACKET intLiteral? RBRACKET)* QUESTION? */
const Type* Parser::type(Modifiers* modifiers) {
    Token type;
    if (!this->expect(Token::Kind::TK_IDENTIFIER, "a type", &type)) {
        return nullptr;
    }
    if (!this->symbolTable()->isType(this->text(type))) {
        this->error(type, "no type named '" + std::string(this->text(type)) + "'");
        return fCompiler.context().fTypes.fInvalid.get();
    }
    const Type* result = this->findType(this->position(type), modifiers, this->text(type));
    if (result->isInterfaceBlock()) {
        // SkSL puts interface blocks into the symbol table, but they aren't general-purpose types;
        // you can't use them to declare a variable type or a function return type.
        this->error(type, "expected a type, found '" + std::string(this->text(type)) + "'");
        return fCompiler.context().fTypes.fInvalid.get();
    }
    Token bracket;
    while (this->checkNext(Token::Kind::TK_LBRACKET, &bracket)) {
        if (this->checkNext(Token::Kind::TK_RBRACKET)) {
            if (this->allowUnsizedArrays()) {
                result = this->unsizedArrayType(result, this->rangeFrom(type));
            } else {
                this->error(this->rangeFrom(bracket), "unsized arrays are not permitted here");
            }
        } else {
            SKSL_INT size;
            if (!this->arraySize(&size)) {
                return nullptr;
            }
            this->expect(Token::Kind::TK_RBRACKET, "']'");
            result = this->arrayType(result, size, this->rangeFrom(type));
        }
    }
    return result;
}

/* IDENTIFIER LBRACE
     varDeclaration+
   RBRACE (IDENTIFIER (LBRACKET expression RBRACKET)*)? SEMICOLON */
bool Parser::interfaceBlock(const Modifiers& modifiers) {
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
        Modifiers fieldModifiers = this->modifiers();
        const Type* type = this->type(&fieldModifiers);
        if (!type) {
            return false;
        }
        do {
            Token fieldName;
            if (!this->expectIdentifier(&fieldName)) {
                return false;
            }
            const Type* actualType = type;
            if (this->checkNext(Token::Kind::TK_LBRACKET)) {
                Token sizeToken = this->peek();
                if (sizeToken.fKind != Token::Kind::TK_RBRACKET) {
                    SKSL_INT size;
                    if (!this->arraySize(&size)) {
                        return false;
                    }
                    actualType = this->arrayType(actualType, size, this->position(typeName));
                } else if (this->allowUnsizedArrays()) {
                    actualType = this->unsizedArrayType(actualType, this->position(typeName));
                } else {
                    this->error(sizeToken, "unsized arrays are not permitted here");
                }
                this->expect(Token::Kind::TK_RBRACKET, "']'");
            }

            fields.push_back(SkSL::Field(this->rangeFrom(fieldPos),
                                         fieldModifiers.fLayout,
                                         fieldModifiers.fFlags,
                                         this->text(fieldName),
                                         actualType));
        } while (this->checkNext(Token::Kind::TK_COMMA));

        if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
            return false;
        }
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
                                                                           modifiers,
                                                                           this->text(typeName),
                                                                           std::move(fields),
                                                                           instanceName,
                                                                           size)) {
        fProgramElements.push_back(std::move(ib));
        return true;
    }
    return false;
}

/* IF LPAREN expression RPAREN statement (ELSE statement)? */
std::unique_ptr<Statement> Parser::ifStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_IF, "'if'", &start)) {
        return nullptr;
    }
    if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
        return nullptr;
    }
    std::unique_ptr<Expression> test = this->expression();
    if (!test) {
        return nullptr;
    }
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
        return nullptr;
    }
    std::unique_ptr<Statement> ifTrue = this->statement();
    if (!ifTrue) {
        return nullptr;
    }
    std::unique_ptr<Statement> ifFalse;
    if (this->checkNext(Token::Kind::TK_ELSE)) {
        ifFalse = this->statement();
        if (!ifFalse) {
            return nullptr;
        }
    }
    Position pos = this->rangeFrom(start);
    return this->statementOrNop(pos, IfStatement::Convert(fCompiler.context(),
                                                          pos,
                                                          std::move(test),
                                                          std::move(ifTrue),
                                                          std::move(ifFalse)));
}

/* DO statement WHILE LPAREN expression RPAREN SEMICOLON */
std::unique_ptr<Statement> Parser::doStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_DO, "'do'", &start)) {
        return nullptr;
    }
    std::unique_ptr<Statement> statement = this->statement();
    if (!statement) {
        return nullptr;
    }
    if (!this->expect(Token::Kind::TK_WHILE, "'while'")) {
        return nullptr;
    }
    if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
        return nullptr;
    }
    std::unique_ptr<Expression> test = this->expression();
    if (!test) {
        return nullptr;
    }
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
        return nullptr;
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return nullptr;
    }
    Position pos = this->rangeFrom(start);
    return this->statementOrNop(pos, DoStatement::Convert(fCompiler.context(), pos,
                                                          std::move(statement), std::move(test)));
}

/* WHILE LPAREN expression RPAREN STATEMENT */
std::unique_ptr<Statement> Parser::whileStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_WHILE, "'while'", &start)) {
        return nullptr;
    }
    if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
        return nullptr;
    }
    std::unique_ptr<Expression> test = this->expression();
    if (!test) {
        return nullptr;
    }
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
        return nullptr;
    }
    std::unique_ptr<Statement> statement = this->statement();
    if (!statement) {
        return nullptr;
    }
    Position pos = this->rangeFrom(start);
    return this->statementOrNop(pos, ForStatement::ConvertWhile(fCompiler.context(), pos,
                                                                std::move(test),
                                                                std::move(statement)));
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
        std::unique_ptr<Statement> s = this->statement();
        if (!s) {
            return false;
        }
        statements.push_back(std::move(s));
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
    std::unique_ptr<Expression> caseValue = this->expression();
    if (!caseValue) {
        return false;
    }
    return this->switchCaseBody(values, caseBlocks, std::move(caseValue));
}

/* SWITCH LPAREN expression RPAREN LBRACE switchCase* (DEFAULT COLON statement*)? RBRACE */
std::unique_ptr<Statement> Parser::switchStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_SWITCH, "'switch'", &start)) {
        return nullptr;
    }
    if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
        return nullptr;
    }
    std::unique_ptr<Expression> value = this->expression();
    if (!value) {
        return nullptr;
    }
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
        return nullptr;
    }
    if (!this->expect(Token::Kind::TK_LBRACE, "'{'")) {
        return nullptr;
    }

    std::unique_ptr<SymbolTable> symbolTable;
    ExpressionArray values;
    StatementArray caseBlocks;
    {
        // Keeping a tight scope around AutoSymbolTable is important here. SwitchStatement::Convert
        // may end up creating a new symbol table if the HoistSwitchVarDeclarationsAtTopLevel
        // transform is used. We want ~AutoSymbolTable to happen first, so it can restore the
        // context's active symbol table to the enclosing block instead of the switch's inner block.
        AutoSymbolTable symbols(this, &symbolTable);

        while (this->peek().fKind == Token::Kind::TK_CASE) {
            if (!this->switchCase(&values, &caseBlocks)) {
                return nullptr;
            }
        }
        // Requiring `default:` to be last (in defiance of C and GLSL) was a deliberate decision.
        // Other parts of the compiler are allowed to rely upon this assumption.
        if (this->checkNext(Token::Kind::TK_DEFAULT)) {
            if (!this->switchCaseBody(&values, &caseBlocks, /*value=*/nullptr)) {
                return nullptr;
            }
        }
        if (!this->expect(Token::Kind::TK_RBRACE, "'}'")) {
            return nullptr;
        }
    }

    Position pos = this->rangeFrom(start);
    return this->statementOrNop(pos, SwitchStatement::Convert(fCompiler.context(), pos,
                                                              std::move(value),
                                                              std::move(values),
                                                              std::move(caseBlocks),
                                                              std::move(symbolTable)));
}

static Position range_of_at_least_one_char(int start, int end) {
    return Position::Range(start, std::max(end, start + 1));
}

/* FOR LPAREN (declaration | expression)? SEMICOLON expression? SEMICOLON expression? RPAREN
   STATEMENT */
std::unique_ptr<Statement> Parser::forStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_FOR, "'for'", &start)) {
        return nullptr;
    }
    Token lparen;
    if (!this->expect(Token::Kind::TK_LPAREN, "'('", &lparen)) {
        return nullptr;
    }
    std::unique_ptr<SymbolTable> symbolTable;
    std::unique_ptr<Statement> initializer;
    std::unique_ptr<Expression> test;
    std::unique_ptr<Expression> next;
    std::unique_ptr<Statement> statement;
    int firstSemicolonOffset;
    Token secondSemicolon;
    Token rparen;
    {
        AutoSymbolTable symbols(this, &symbolTable);

        Token nextToken = this->peek();
        if (nextToken.fKind == Token::Kind::TK_SEMICOLON) {
            // An empty init-statement.
            firstSemicolonOffset = this->nextToken().fOffset;
        } else {
            // The init-statement must be an expression or variable declaration.
            initializer = this->varDeclarationsOrExpressionStatement();
            if (!initializer) {
                return nullptr;
            }
            firstSemicolonOffset = fLexer.getCheckpoint().fOffset - 1;
        }
        if (this->peek().fKind != Token::Kind::TK_SEMICOLON) {
            test = this->expression();
            if (!test) {
                return nullptr;
            }
        }
        if (!this->expect(Token::Kind::TK_SEMICOLON, "';'", &secondSemicolon)) {
            return nullptr;
        }
        if (this->peek().fKind != Token::Kind::TK_RPAREN) {
            next = this->expression();
            if (!next) {
                return nullptr;
            }
        }
        if (!this->expect(Token::Kind::TK_RPAREN, "')'", &rparen)) {
            return nullptr;
        }
        statement = this->statement(/*bracesIntroduceNewScope=*/false);
        if (!statement) {
            return nullptr;
        }
    }
    Position pos = this->rangeFrom(start);
    ForLoopPositions loopPositions{
            range_of_at_least_one_char(lparen.fOffset + 1, firstSemicolonOffset),
            range_of_at_least_one_char(firstSemicolonOffset + 1, secondSemicolon.fOffset),
            range_of_at_least_one_char(secondSemicolon.fOffset + 1, rparen.fOffset),
    };
    return this->statementOrNop(pos, ForStatement::Convert(fCompiler.context(), pos, loopPositions,
                                                           std::move(initializer),
                                                           std::move(test),
                                                           std::move(next),
                                                           std::move(statement),
                                                           std::move(symbolTable)));
}

/* RETURN expression? SEMICOLON */
std::unique_ptr<Statement> Parser::returnStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_RETURN, "'return'", &start)) {
        return nullptr;
    }
    std::unique_ptr<Expression> expression;
    if (this->peek().fKind != Token::Kind::TK_SEMICOLON) {
        expression = this->expression();
        if (!expression) {
            return nullptr;
        }
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return nullptr;
    }
    // We do not check for errors, or coerce the value to the correct type, until the return
    // statement is actually added to a function. (This is done in FunctionDefinition::Convert.)
    return ReturnStatement::Make(this->rangeFrom(start), std::move(expression));
}

/* BREAK SEMICOLON */
std::unique_ptr<Statement> Parser::breakStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_BREAK, "'break'", &start)) {
        return nullptr;
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return nullptr;
    }
    return SkSL::BreakStatement::Make(this->position(start));
}

/* CONTINUE SEMICOLON */
std::unique_ptr<Statement> Parser::continueStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_CONTINUE, "'continue'", &start)) {
        return nullptr;
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return nullptr;
    }
    return SkSL::ContinueStatement::Make(this->position(start));
}

/* DISCARD SEMICOLON */
std::unique_ptr<Statement> Parser::discardStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_DISCARD, "'continue'", &start)) {
        return nullptr;
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return nullptr;
    }
    Position pos = this->position(start);
    return this->statementOrNop(pos, SkSL::DiscardStatement::Convert(fCompiler.context(), pos));
}

/* LBRACE statement* RBRACE */
std::unique_ptr<Statement> Parser::block(bool introduceNewScope,
                                         std::unique_ptr<SymbolTable>* adoptExistingSymbolTable) {
    // We can't introduce a new scope _and_ adopt an existing symbol table.
    SkASSERT(!(introduceNewScope && adoptExistingSymbolTable));

    AutoDepth depth(this);
    Token start;
    if (!this->expect(Token::Kind::TK_LBRACE, "'{'", &start)) {
        return nullptr;
    }
    if (!depth.increase()) {
        return nullptr;
    }

    std::unique_ptr<SymbolTable> newSymbolTable;
    std::unique_ptr<SymbolTable>* symbolTableToUse =
            adoptExistingSymbolTable ? adoptExistingSymbolTable : &newSymbolTable;

    StatementArray statements;
    {
        AutoSymbolTable symbols(this, symbolTableToUse, /*enable=*/introduceNewScope);

        // Consume statements until we reach the closing brace.
        for (;;) {
            Token::Kind tokenKind = this->peek().fKind;
            if (tokenKind == Token::Kind::TK_RBRACE) {
                this->nextToken();
                break;
            }
            if (tokenKind == Token::Kind::TK_END_OF_FILE) {
                this->error(this->peek(), "expected '}', but found end of file");
                return nullptr;
            }
            if (std::unique_ptr<Statement> statement = this->statement()) {
                statements.push_back(std::move(statement));
            }
            if (fEncounteredFatalError) {
                return nullptr;
            }
        }
    }
    return SkSL::Block::MakeBlock(this->rangeFrom(start),
                                  std::move(statements),
                                  Block::Kind::kBracedScope,
                                  std::move(*symbolTableToUse));
}

/* expression SEMICOLON */
std::unique_ptr<Statement> Parser::expressionStatement() {
    std::unique_ptr<Expression> expr = this->expression();
    if (!expr) {
        return nullptr;
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return nullptr;
    }
    Position pos = expr->position();
    return this->statementOrNop(pos, SkSL::ExpressionStatement::Convert(fCompiler.context(),
                                                                        std::move(expr)));
}

std::unique_ptr<Expression> Parser::poison(Position pos) {
    return Poison::Make(pos, fCompiler.context());
}

std::unique_ptr<Expression> Parser::expressionOrPoison(Position pos,
                                                       std::unique_ptr<Expression> expr) {
    if (!expr) {
        // If no expression was passed in, create a poison expression.
        expr = this->poison(pos);
    }
    // If a valid position was passed in, it must match the expression's position.
    SkASSERTF(!pos.valid() || expr->position() == pos,
              "expected expression position (%d-%d), but received (%d-%d)",
              pos.startOffset(),
              pos.endOffset(),
              expr->position().startOffset(),
              expr->position().endOffset());
    return expr;
}

bool Parser::operatorRight(Parser::AutoDepth& depth,
                           Operator::Kind op,
                           BinaryParseFn rightFn,
                           std::unique_ptr<Expression>& expr) {
    this->nextToken();
    if (!depth.increase()) {
        return false;
    }
    std::unique_ptr<Expression> right = (this->*rightFn)();
    if (!right) {
        return false;
    }
    Position pos = expr->position().rangeThrough(right->position());
    expr = this->expressionOrPoison(pos, BinaryExpression::Convert(fCompiler.context(), pos,
                                                                   std::move(expr), op,
                                                                   std::move(right)));
    return true;
}

/* assignmentExpression (COMMA assignmentExpression)* */
std::unique_ptr<Expression> Parser::expression() {
    AutoDepth depth(this);
    [[maybe_unused]] Token start = this->peek();
    std::unique_ptr<Expression> result = this->assignmentExpression();
    if (!result) {
        return nullptr;
    }
    while (this->peek().fKind == Token::Kind::TK_COMMA) {
        if (!this->operatorRight(depth, Operator::Kind::COMMA, &Parser::assignmentExpression,
                                 result)) {
            return nullptr;
        }
    }
    SkASSERTF(result->position().valid(), "Expression %s has invalid position",
              result->description().c_str());
    SkASSERTF(result->position().startOffset() == this->position(start).startOffset(),
              "Expected %s to start at %d (first token: '%.*s'), but it has range %d-%d\n",
              result->description().c_str(), this->position(start).startOffset(),
              (int)this->text(start).length(), this->text(start).data(),
              result->position().startOffset(), result->position().endOffset());
    return result;
}

/* ternaryExpression ((EQEQ | STAREQ | SLASHEQ | PERCENTEQ | PLUSEQ | MINUSEQ | SHLEQ | SHREQ |
   BITWISEANDEQ | BITWISEXOREQ | BITWISEOREQ | LOGICALANDEQ | LOGICALXOREQ | LOGICALOREQ)
   assignmentExpression)*
 */
std::unique_ptr<Expression> Parser::assignmentExpression() {
    AutoDepth depth(this);
    std::unique_ptr<Expression> result = this->ternaryExpression();
    if (!result) {
        return nullptr;
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
            return nullptr;
        }
    }
}

/* logicalOrExpression ('?' expression ':' assignmentExpression)? */
std::unique_ptr<Expression> Parser::ternaryExpression() {
    AutoDepth depth(this);
    std::unique_ptr<Expression> base = this->logicalOrExpression();
    if (!base) {
        return nullptr;
    }
    if (!this->checkNext(Token::Kind::TK_QUESTION)) {
        return base;
    }
    if (!depth.increase()) {
        return nullptr;
    }
    std::unique_ptr<Expression> trueExpr = this->expression();
    if (!trueExpr) {
        return nullptr;
    }
    if (!this->expect(Token::Kind::TK_COLON, "':'")) {
        return nullptr;
    }
    std::unique_ptr<Expression> falseExpr = this->assignmentExpression();
    if (!falseExpr) {
        return nullptr;
    }
    Position pos = base->position().rangeThrough(falseExpr->position());
    return this->expressionOrPoison(pos, TernaryExpression::Convert(fCompiler.context(),
                                                                    pos, std::move(base),
                                                                    std::move(trueExpr),
                                                                    std::move(falseExpr)));
}

/* logicalXorExpression (LOGICALOR logicalXorExpression)* */
std::unique_ptr<Expression> Parser::logicalOrExpression() {
    AutoDepth depth(this);
    std::unique_ptr<Expression> result = this->logicalXorExpression();
    if (!result) {
        return nullptr;
    }
    while (this->peek().fKind == Token::Kind::TK_LOGICALOR) {
        if (!this->operatorRight(depth, Operator::Kind::LOGICALOR, &Parser::logicalXorExpression,
                                 result)) {
            return nullptr;
        }
    }
    return result;
}

/* logicalAndExpression (LOGICALXOR logicalAndExpression)* */
std::unique_ptr<Expression> Parser::logicalXorExpression() {
    AutoDepth depth(this);
    std::unique_ptr<Expression> result = this->logicalAndExpression();
    if (!result) {
        return nullptr;
    }
    while (this->peek().fKind == Token::Kind::TK_LOGICALXOR) {
        if (!this->operatorRight(depth, Operator::Kind::LOGICALXOR, &Parser::logicalAndExpression,
                                 result)) {
            return nullptr;
        }
    }
    return result;
}

/* bitwiseOrExpression (LOGICALAND bitwiseOrExpression)* */
std::unique_ptr<Expression> Parser::logicalAndExpression() {
    AutoDepth depth(this);
    std::unique_ptr<Expression> result = this->bitwiseOrExpression();
    if (!result) {
        return nullptr;
    }
    while (this->peek().fKind == Token::Kind::TK_LOGICALAND) {
        if (!this->operatorRight(depth, Operator::Kind::LOGICALAND, &Parser::bitwiseOrExpression,
                                 result)) {
            return nullptr;
        }
    }
    return result;
}

/* bitwiseXorExpression (BITWISEOR bitwiseXorExpression)* */
std::unique_ptr<Expression> Parser::bitwiseOrExpression() {
    AutoDepth depth(this);
    std::unique_ptr<Expression> result = this->bitwiseXorExpression();
    if (!result) {
        return nullptr;
    }
    while (this->peek().fKind == Token::Kind::TK_BITWISEOR) {
        if (!this->operatorRight(depth, Operator::Kind::BITWISEOR, &Parser::bitwiseXorExpression,
                                 result)) {
            return nullptr;
        }
    }
    return result;
}

/* bitwiseAndExpression (BITWISEXOR bitwiseAndExpression)* */
std::unique_ptr<Expression> Parser::bitwiseXorExpression() {
    AutoDepth depth(this);
    std::unique_ptr<Expression> result = this->bitwiseAndExpression();
    if (!result) {
        return nullptr;
    }
    while (this->peek().fKind == Token::Kind::TK_BITWISEXOR) {
        if (!this->operatorRight(depth, Operator::Kind::BITWISEXOR, &Parser::bitwiseAndExpression,
                                 result)) {
            return nullptr;
        }
    }
    return result;
}

/* equalityExpression (BITWISEAND equalityExpression)* */
std::unique_ptr<Expression> Parser::bitwiseAndExpression() {
    AutoDepth depth(this);
    std::unique_ptr<Expression> result = this->equalityExpression();
    if (!result) {
        return nullptr;
    }
    while (this->peek().fKind == Token::Kind::TK_BITWISEAND) {
        if (!this->operatorRight(depth, Operator::Kind::BITWISEAND, &Parser::equalityExpression,
                                 result)) {
            return nullptr;
        }
    }
    return result;
}

/* relationalExpression ((EQEQ | NEQ) relationalExpression)* */
std::unique_ptr<Expression> Parser::equalityExpression() {
    AutoDepth depth(this);
    std::unique_ptr<Expression> result = this->relationalExpression();
    if (!result) {
        return nullptr;
    }
    for (;;) {
        Operator::Kind op;
        switch (this->peek().fKind) {
            case Token::Kind::TK_EQEQ: op = Operator::Kind::EQEQ; break;
            case Token::Kind::TK_NEQ:  op = Operator::Kind::NEQ;  break;
            default:                   return result;
        }
        if (!this->operatorRight(depth, op, &Parser::relationalExpression, result)) {
            return nullptr;
        }
    }
}

/* shiftExpression ((LT | GT | LTEQ | GTEQ) shiftExpression)* */
std::unique_ptr<Expression> Parser::relationalExpression() {
    AutoDepth depth(this);
    std::unique_ptr<Expression> result = this->shiftExpression();
    if (!result) {
        return nullptr;
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
            return nullptr;
        }
    }
}

/* additiveExpression ((SHL | SHR) additiveExpression)* */
std::unique_ptr<Expression> Parser::shiftExpression() {
    AutoDepth depth(this);
    std::unique_ptr<Expression> result = this->additiveExpression();
    if (!result) {
        return nullptr;
    }
    for (;;) {
        Operator::Kind op;
        switch (this->peek().fKind) {
            case Token::Kind::TK_SHL: op = Operator::Kind::SHL; break;
            case Token::Kind::TK_SHR: op = Operator::Kind::SHR; break;
            default:                  return result;
        }
        if (!this->operatorRight(depth, op, &Parser::additiveExpression, result)) {
            return nullptr;
        }
    }
}

/* multiplicativeExpression ((PLUS | MINUS) multiplicativeExpression)* */
std::unique_ptr<Expression> Parser::additiveExpression() {
    AutoDepth depth(this);
    std::unique_ptr<Expression> result = this->multiplicativeExpression();
    if (!result) {
        return nullptr;
    }
    for (;;) {
        Operator::Kind op;
        switch (this->peek().fKind) {
            case Token::Kind::TK_PLUS:  op = Operator::Kind::PLUS;  break;
            case Token::Kind::TK_MINUS: op = Operator::Kind::MINUS; break;
            default:                    return result;
        }
        if (!this->operatorRight(depth, op, &Parser::multiplicativeExpression, result)) {
            return nullptr;
        }
    }
}

/* unaryExpression ((STAR | SLASH | PERCENT) unaryExpression)* */
std::unique_ptr<Expression> Parser::multiplicativeExpression() {
    AutoDepth depth(this);
    std::unique_ptr<Expression> result = this->unaryExpression();
    if (!result) {
        return nullptr;
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
            return nullptr;
        }
    }
}

/* postfixExpression | (PLUS | MINUS | NOT | PLUSPLUS | MINUSMINUS) unaryExpression */
std::unique_ptr<Expression> Parser::unaryExpression() {
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
        return nullptr;
    }
    std::unique_ptr<Expression> expr = this->unaryExpression();
    if (!expr) {
        return nullptr;
    }
    Position pos = Position::Range(start.fOffset, expr->position().endOffset());
    return this->expressionOrPoison(pos, PrefixExpression::Convert(fCompiler.context(),
                                                                   pos, op, std::move(expr)));
}

/* term suffix* */
std::unique_ptr<Expression> Parser::postfixExpression() {
    AutoDepth depth(this);
    std::unique_ptr<Expression> result = this->term();
    if (!result) {
        return nullptr;
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
                    return nullptr;
                }
                result = this->suffix(std::move(result));
                if (!result) {
                    return nullptr;
                }
                break;
            }
            default:
                return result;
        }
    }
}

std::unique_ptr<Expression> Parser::swizzle(Position pos,
                                            std::unique_ptr<Expression> base,
                                            std::string_view swizzleMask,
                                            Position maskPos) {
    SkASSERT(!swizzleMask.empty());
    if (!base->type().isVector() && !base->type().isScalar()) {
        return this->expressionOrPoison(pos, FieldAccess::Convert(fCompiler.context(), pos,
                                                                  std::move(base), swizzleMask));

    }
    return this->expressionOrPoison(pos, Swizzle::Convert(fCompiler.context(), pos, maskPos,
                                                          std::move(base), swizzleMask));
}

std::unique_ptr<Expression> Parser::call(Position pos,
                                         std::unique_ptr<Expression> base,
                                         ExpressionArray args) {
    return this->expressionOrPoison(pos, SkSL::FunctionCall::Convert(fCompiler.context(), pos,
                                                                     std::move(base),
                                                                     std::move(args)));
}

/* LBRACKET expression? RBRACKET | DOT IDENTIFIER | LPAREN arguments RPAREN |
   PLUSPLUS | MINUSMINUS | COLONCOLON IDENTIFIER | FLOAT_LITERAL [IDENTIFIER] */
std::unique_ptr<Expression> Parser::suffix(std::unique_ptr<Expression> base) {
    AutoDepth depth(this);
    Token next = this->nextToken();
    if (!depth.increase()) {
        return nullptr;
    }
    switch (next.fKind) {
        case Token::Kind::TK_LBRACKET: {
            if (this->checkNext(Token::Kind::TK_RBRACKET)) {
                this->error(this->rangeFrom(next), "missing index in '[]'");
                return this->poison(this->rangeFrom(base->position()));
            }
            std::unique_ptr<Expression> index = this->expression();
            if (!index) {
                return nullptr;
            }
            this->expect(Token::Kind::TK_RBRACKET, "']' to complete array access expression");

            Position pos = this->rangeFrom(base->position());
            return this->expressionOrPoison(pos, IndexExpression::Convert(fCompiler.context(), pos,
                                                                          std::move(base),
                                                                          std::move(index)));
        }
        case Token::Kind::TK_DOT: {
            std::string_view text;
            if (this->identifier(&text)) {
                Position pos = this->rangeFrom(base->position());
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
            Position pos = this->rangeFrom(base->position());
            Position start = this->position(next);
            // skip past the "."
            start = Position::Range(start.startOffset() + 1, start.endOffset());
            Position maskPos = this->rangeFrom(start);
            Token id = this->nextRawToken();
            if (id.fKind == Token::Kind::TK_IDENTIFIER) {
                pos = this->rangeFrom(base->position());
                maskPos = this->rangeFrom(start);
                return this->swizzle(pos,
                                     std::move(base),
                                     std::string(field) + std::string(this->text(id)),
                                     maskPos);
            }
            if (field.empty()) {
                this->error(pos, "expected field name or swizzle mask after '.'");
                return this->poison(pos);
            }
            this->pushback(id);
            return this->swizzle(pos, std::move(base), field, maskPos);
        }
        case Token::Kind::TK_LPAREN: {
            ExpressionArray args;
            if (this->peek().fKind != Token::Kind::TK_RPAREN) {
                for (;;) {
                    std::unique_ptr<Expression> expr = this->assignmentExpression();
                    if (!expr) {
                        return nullptr;
                    }
                    args.push_back(std::move(expr));
                    if (!this->checkNext(Token::Kind::TK_COMMA)) {
                        break;
                    }
                }
            }
            this->expect(Token::Kind::TK_RPAREN, "')' to complete function arguments");
            Position pos = this->rangeFrom(base->position());
            return this->call(pos, std::move(base), std::move(args));
        }
        case Token::Kind::TK_PLUSPLUS:
        case Token::Kind::TK_MINUSMINUS: {
            Operator::Kind op = (next.fKind == Token::Kind::TK_PLUSPLUS)
                                        ? Operator::Kind::PLUSPLUS
                                        : Operator::Kind::MINUSMINUS;
            Position pos = this->rangeFrom(base->position());
            return this->expressionOrPoison(pos, PostfixExpression::Convert(fCompiler.context(),
                                                                            pos, std::move(base),
                                                                            op));
        }
        default: {
            this->error(next, "expected expression suffix, but found '" +
                              std::string(this->text(next)) + "'");
            return nullptr;
        }
    }
}

/* IDENTIFIER | intLiteral | floatLiteral | boolLiteral | '(' expression ')' */
std::unique_ptr<Expression> Parser::term() {
    AutoDepth depth(this);
    Token t = this->peek();
    switch (t.fKind) {
        case Token::Kind::TK_IDENTIFIER: {
            std::string_view text;
            if (this->identifier(&text)) {
                Position pos = this->position(t);
                return this->expressionOrPoison(
                        pos,
                        this->symbolTable()->instantiateSymbolRef(fCompiler.context(), text, pos));
            }
            break;
        }
        case Token::Kind::TK_INT_LITERAL: {
            SKSL_INT i;
            if (!this->intLiteral(&i)) {
                i = 0;
            }
            Position pos = this->position(t);
            return this->expressionOrPoison(pos, SkSL::Literal::MakeInt(fCompiler.context(),
                                                                        pos, i));
        }
        case Token::Kind::TK_FLOAT_LITERAL: {
            SKSL_FLOAT f;
            if (!this->floatLiteral(&f)) {
                f = 0.0f;
            }
            Position pos = this->position(t);
            return this->expressionOrPoison(pos, SkSL::Literal::MakeFloat(fCompiler.context(),
                                                                          pos, f));
        }
        case Token::Kind::TK_TRUE_LITERAL: // fall through
        case Token::Kind::TK_FALSE_LITERAL: {
            bool b;
            SkAssertResult(this->boolLiteral(&b));
            Position pos = this->position(t);
            return this->expressionOrPoison(pos, SkSL::Literal::MakeBool(fCompiler.context(),
                                                                         pos, b));
        }
        case Token::Kind::TK_LPAREN: {
            this->nextToken();
            if (!depth.increase()) {
                return nullptr;
            }
            std::unique_ptr<Expression> result = this->expression();
            if (result != nullptr) {
                this->expect(Token::Kind::TK_RPAREN, "')' to complete expression");
                result->setPosition(this->rangeFrom(this->position(t)));
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
    return nullptr;
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
