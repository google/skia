/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLDSLParser.h"

#include "include/private/SkSLString.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/dsl/priv/DSL_priv.h"

#include <memory>

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
        case Token::Kind::TK_HASSIDEEFFECTS: return Modifiers::kHasSideEffects_Flag;
        case Token::Kind::TK_INLINE:         return Modifiers::kInline_Flag;
        case Token::Kind::TK_NOINLINE:       return Modifiers::kNoInline_Flag;
        case Token::Kind::TK_HIGHP:          return Modifiers::kHighp_Flag;
        case Token::Kind::TK_MEDIUMP:        return Modifiers::kMediump_Flag;
        case Token::Kind::TK_LOWP:           return Modifiers::kLowp_Flag;
        case Token::Kind::TK_ES3:            return Modifiers::kES3_Flag;
        default:                             return 0;
    }
}

class AutoDSLDepth {
public:
    AutoDSLDepth(DSLParser* p)
    : fParser(p)
    , fDepth(0) {}

    ~AutoDSLDepth() {
        fParser->fDepth -= fDepth;
    }

    bool increase() {
        ++fDepth;
        ++fParser->fDepth;
        if (fParser->fDepth > kMaxParseDepth) {
            fParser->error(fParser->peek(), String("exceeded max parse depth"));
            return false;
        }
        return true;
    }

private:
    DSLParser* fParser;
    int fDepth;
};

class AutoDSLSymbolTable {
public:
    AutoDSLSymbolTable() {
        dsl::PushSymbolTable();
    }

    ~AutoDSLSymbolTable() {
        dsl::PopSymbolTable();
    }
};

std::unordered_map<skstd::string_view, DSLParser::LayoutToken>* DSLParser::layoutTokens;

void DSLParser::InitLayoutMap() {
    layoutTokens = new std::unordered_map<skstd::string_view, LayoutToken>;
    #define TOKEN(name, text) (*layoutTokens)[text] = LayoutToken::name
    TOKEN(LOCATION,                     "location");
    TOKEN(OFFSET,                       "offset");
    TOKEN(BINDING,                      "binding");
    TOKEN(INDEX,                        "index");
    TOKEN(SET,                          "set");
    TOKEN(BUILTIN,                      "builtin");
    TOKEN(INPUT_ATTACHMENT_INDEX,       "input_attachment_index");
    TOKEN(ORIGIN_UPPER_LEFT,            "origin_upper_left");
    TOKEN(BLEND_SUPPORT_ALL_EQUATIONS,  "blend_support_all_equations");
    TOKEN(PUSH_CONSTANT,                "push_constant");
    TOKEN(SRGB_UNPREMUL,                "srgb_unpremul");
    #undef TOKEN
}

DSLParser::DSLParser(Compiler* compiler, const ProgramSettings& settings, ProgramKind kind,
                     String text)
    : fCompiler(*compiler)
    , fSettings(settings)
    , fKind(kind)
    , fText(std::make_unique<String>(std::move(text)))
    , fPushback(Token::Kind::TK_NONE, /*offset=*/-1, /*length=*/-1, /*line=*/-1) {
    // We don't want to have to worry about manually releasing all of the objects in the event that
    // an error occurs
    fSettings.fAssertDSLObjectsReleased = false;
    // We manage our symbol tables manually, so no need for name mangling
    fSettings.fDSLMangling = false;
    fLexer.start(*fText);
    static const bool layoutMapInitialized = []{ InitLayoutMap(); return true; }();
    (void) layoutMapInitialized;
}

Token DSLParser::nextRawToken() {
    if (fPushback.fKind != Token::Kind::TK_NONE) {
        Token result = fPushback;
        fPushback.fKind = Token::Kind::TK_NONE;
        return result;
    }
    return fLexer.next();
}

Token DSLParser::nextToken() {
    Token token = this->nextRawToken();
    while (token.fKind == Token::Kind::TK_WHITESPACE ||
           token.fKind == Token::Kind::TK_LINE_COMMENT ||
           token.fKind == Token::Kind::TK_BLOCK_COMMENT) {
        token = this->nextRawToken();
    }
    return token;
}

void DSLParser::pushback(Token t) {
    SkASSERT(fPushback.fKind == Token::Kind::TK_NONE);
    fPushback = std::move(t);
}

Token DSLParser::peek() {
    if (fPushback.fKind == Token::Kind::TK_NONE) {
        fPushback = this->nextToken();
    }
    return fPushback;
}

bool DSLParser::checkNext(Token::Kind kind, Token* result) {
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

bool DSLParser::expect(Token::Kind kind, const char* expected, Token* result) {
    Token next = this->nextToken();
    if (next.fKind == kind) {
        if (result) {
            *result = std::move(next);
        }
        return true;
    } else {
        this->error(next, "expected " + String(expected) + ", but found '" +
                    this->text(next) + "'");
        this->fEncounteredFatalError = true;
        return false;
    }
}

bool DSLParser::expectIdentifier(Token* result) {
    if (!this->expect(Token::Kind::TK_IDENTIFIER, "an identifier", result)) {
        return false;
    }
    if (IsBuiltinType(this->text(*result))) {
        this->error(*result, "expected an identifier, but found type '" +
                             this->text(*result) + "'");
        this->fEncounteredFatalError = true;
        return false;
    }
    return true;
}

skstd::string_view DSLParser::text(Token token) {
    return skstd::string_view(fText->data() + token.fOffset, token.fLength);
}

PositionInfo DSLParser::position(Token t) {
    return this->position(t.fLine);
}

PositionInfo DSLParser::position(int line) {
    return PositionInfo("<unknown>", line);
}

void DSLParser::error(Token token, String msg) {
    this->error(token.fLine, msg);
}

void DSLParser::error(int line, String msg) {
    GetErrorReporter().error(msg.c_str(), this->position(line));
}

/* declaration* END_OF_FILE */
std::unique_ptr<Program> DSLParser::program() {
    ErrorReporter* errorReporter = &fCompiler.errorReporter();
    Start(&fCompiler, fKind, fSettings);
    SetErrorReporter(errorReporter);
    errorReporter->setSource(fText->c_str());
    this->declarations();
    std::unique_ptr<Program> result;
    if (!GetErrorReporter().errorCount()) {
        result = dsl::ReleaseProgram(std::move(fText));
    }
    errorReporter->setSource(nullptr);
    End();
    return result;
}

SkSL::LoadedModule DSLParser::moduleInheritingFrom(SkSL::ParsedModule baseModule) {
    ErrorReporter* errorReporter = &fCompiler.errorReporter();
    StartModule(&fCompiler, fKind, fSettings, std::move(baseModule));
    SetErrorReporter(errorReporter);
    errorReporter->setSource(fText->c_str());
    this->declarations();
    CurrentSymbolTable()->takeOwnershipOfString(std::move(*fText));
    SkSL::LoadedModule result{ fKind, CurrentSymbolTable(),
            std::move(ThreadContext::ProgramElements()) };
    errorReporter->setSource(nullptr);
    End();
    return result;
}

void DSLParser::declarations() {
    fEncounteredFatalError = false;
    bool done = false;
    while (!done) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_END_OF_FILE:
                done = true;
                break;
            case Token::Kind::TK_DIRECTIVE:
                this->directive();
                break;
            case Token::Kind::TK_INVALID: {
                this->nextToken();
                this->error(this->peek(), String("invalid token"));
                done = true;
                break;
            }
            default:
                this->declaration();
                done = fEncounteredFatalError;
                break;
        }
    }
}

/* DIRECTIVE(#extension) IDENTIFIER COLON IDENTIFIER */
void DSLParser::directive() {
    Token start;
    if (!this->expect(Token::Kind::TK_DIRECTIVE, "a directive", &start)) {
        return;
    }
    skstd::string_view text = this->text(start);
    if (text == "#extension") {
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
        skstd::string_view behaviorText = this->text(behavior);
        if (behaviorText == "disable") {
            return;
        }
        if (behaviorText != "require" && behaviorText != "enable" && behaviorText != "warn") {
            this->error(behavior, "expected 'require', 'enable', 'warn', or 'disable'");
        }
        // We don't currently do anything different between require, enable, and warn
        dsl::AddExtension(this->text(name));
    } else {
        this->error(start, "unsupported directive '" + this->text(start) + "'");
    }
}

/* modifiers (structVarDeclaration | type IDENTIFIER ((LPAREN parameter (COMMA parameter)* RPAREN
   (block | SEMICOLON)) | SEMICOLON) | interfaceBlock) */
bool DSLParser::declaration() {
    Token lookahead = this->peek();
    switch (lookahead.fKind) {
        case Token::Kind::TK_SEMICOLON:
            this->nextToken();
            this->error(lookahead, "expected a declaration, but found ';'");
            return false;
        default:
            break;
    }
    DSLModifiers modifiers = this->modifiers();
    lookahead = this->peek();
    if (lookahead.fKind == Token::Kind::TK_IDENTIFIER && !IsType(this->text(lookahead))) {
        // we have an identifier that's not a type, could be the start of an interface block
        return this->interfaceBlock(modifiers);
    }
    if (lookahead.fKind == Token::Kind::TK_SEMICOLON) {
        this->nextToken();
        Declare(modifiers, position(lookahead));
        return true;
    }
    if (lookahead.fKind == Token::Kind::TK_STRUCT) {
        this->structVarDeclaration(modifiers);
        return true;
    }
    skstd::optional<DSLType> type = this->type(&modifiers);
    if (!type) {
        return false;
    }
    Token name;
    if (!this->expectIdentifier(&name)) {
        return false;
    }
    if (this->checkNext(Token::Kind::TK_LPAREN)) {
        return this->functionDeclarationEnd(modifiers, *type, name);
    } else {
        this->globalVarDeclarationEnd(this->position(name), modifiers, *type, this->text(name));
        return true;
    }
}

/* (RPAREN | VOID RPAREN | parameter (COMMA parameter)* RPAREN) (block | SEMICOLON) */
bool DSLParser::functionDeclarationEnd(const DSLModifiers& modifiers,
                                       DSLType type,
                                       const Token& name) {
    SkTArray<DSLWrapper<DSLParameter>> parameters;
    Token lookahead = this->peek();
    if (lookahead.fKind == Token::Kind::TK_RPAREN) {
        // `()` means no parameters at all.
    } else if (lookahead.fKind == Token::Kind::TK_IDENTIFIER && this->text(lookahead) == "void") {
        // `(void)` also means no parameters at all.
        this->nextToken();
    } else {
        for (;;) {
            skstd::optional<DSLWrapper<DSLParameter>> parameter = this->parameter();
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
    SkTArray<DSLParameter*> parameterPointers;
    for (DSLWrapper<DSLParameter>& param : parameters) {
        parameterPointers.push_back(&param.get());
    }
    DSLFunction result(modifiers, type, this->text(name), parameterPointers, this->position(name));
    if (!this->checkNext(Token::Kind::TK_SEMICOLON)) {
        AutoDSLSymbolTable symbols;
        for (DSLParameter* var : parameterPointers) {
            AddToSymbolTable(*var);
        }
        skstd::optional<DSLBlock> body = this->block();
        if (!body) {
            return false;
        }
        result.define(std::move(*body), this->position(name));
    }
    return true;
}

SKSL_INT DSLParser::arraySize() {
    DSLExpression sizeExpr = this->expression();
    if (!sizeExpr.isValid()) {
        return 1;
    }
    std::unique_ptr<SkSL::Expression> sizeLiteral = sizeExpr.release();
    if (!sizeLiteral->isIntLiteral()) {
        this->error(sizeLiteral->fLine, "expected int literal");
        return 1;
    }
    SKSL_INT size = sizeLiteral->as<Literal>().intValue();
    if (size > INT32_MAX) {
        this->error(sizeLiteral->fLine, "array size out of bounds");
        return 1;
    }
    if (size <= 0) {
        this->error(sizeLiteral->fLine, "array size must be positive");
        return 1;
    }
    return size;
}

bool DSLParser::parseArrayDimensions(int line, DSLType* type) {
    while (this->checkNext(Token::Kind::TK_LBRACKET)) {
        if (this->checkNext(Token::Kind::TK_RBRACKET)) {
            this->error(line, "expected array dimension");
        } else {
            *type = Array(*type, this->arraySize(), this->position(line));
            if (!this->expect(Token::Kind::TK_RBRACKET, "']'")) {
                return false;
            }
        }
    }
    return true;
}

bool DSLParser::parseInitializer(int line, DSLExpression* initializer) {
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
void DSLParser::globalVarDeclarationEnd(PositionInfo pos, const dsl::DSLModifiers& mods,
        dsl::DSLType baseType, skstd::string_view name) {
    using namespace dsl;
    int line = this->peek().fLine;
    DSLType type = baseType;
    DSLExpression initializer;
    if (!this->parseArrayDimensions(line, &type)) {
        return;
    }
    if (!this->parseInitializer(line, &initializer)) {
        return;
    }
    DSLGlobalVar first(mods, type, name, std::move(initializer), pos);
    Declare(first);
    AddToSymbolTable(first);

    while (this->checkNext(Token::Kind::TK_COMMA)) {
        type = baseType;
        Token identifierName;
        if (!this->expectIdentifier(&identifierName)) {
            return;
        }
        if (!this->parseArrayDimensions(line, &type)) {
            return;
        }
        DSLExpression anotherInitializer;
        if (!this->parseInitializer(line, &anotherInitializer)) {
            return;
        }
        DSLGlobalVar next(mods, type, this->text(identifierName), std::move(anotherInitializer),
                this->position(line));
        Declare(next);
        AddToSymbolTable(next, this->position(identifierName));
    }
    this->expect(Token::Kind::TK_SEMICOLON, "';'");
}

/* (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)? (COMMA IDENTIFER
   (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)?)* SEMICOLON */
DSLStatement DSLParser::localVarDeclarationEnd(PositionInfo pos, const dsl::DSLModifiers& mods,
        dsl::DSLType baseType, skstd::string_view name) {
    using namespace dsl;
    int line = this->peek().fLine;
    DSLType type = baseType;
    DSLExpression initializer;
    if (!this->parseArrayDimensions(line, &type)) {
        return {};
    }
    if (!this->parseInitializer(line, &initializer)) {
        return {};
    }
    DSLVar first(mods, type, name, std::move(initializer), pos);
    DSLStatement result = Declare(first);
    AddToSymbolTable(first);

    while (this->checkNext(Token::Kind::TK_COMMA)) {
        type = baseType;
        Token identifierName;
        if (!this->expectIdentifier(&identifierName)) {
            return result;
        }
        if (!this->parseArrayDimensions(line, &type)) {
            return result;
        }
        DSLExpression anotherInitializer;
        if (!this->parseInitializer(line, &anotherInitializer)) {
            return result;
        }
        DSLVar next(mods, type, this->text(identifierName), std::move(anotherInitializer),
                this->position(line));
        DSLWriter::AddVarDeclaration(result, next);
        AddToSymbolTable(next, this->position(identifierName));
    }
    this->expect(Token::Kind::TK_SEMICOLON, "';'");
    return result;
}

/* (varDeclarations | expressionStatement) */
DSLStatement DSLParser::varDeclarationsOrExpressionStatement() {
    Token nextToken = this->peek();
    if (nextToken.fKind == Token::Kind::TK_CONST) {
        // Statements that begin with `const` might be variable declarations, but can't be legal
        // SkSL expression-statements. (SkSL constructors don't take a `const` modifier.)
        return this->varDeclarations();
    }

    if (nextToken.fKind == Token::Kind::TK_HIGHP ||
        nextToken.fKind == Token::Kind::TK_MEDIUMP ||
        nextToken.fKind == Token::Kind::TK_LOWP ||
        IsType(this->text(nextToken))) {
        // Statements that begin with a typename are most often variable declarations, but
        // occasionally the type is part of a constructor, and these are actually expression-
        // statements in disguise. First, attempt the common case: parse it as a vardecl.
        Checkpoint checkpoint(this);
        VarDeclarationsPrefix prefix;
        if (this->varDeclarationsPrefix(&prefix)) {
            checkpoint.accept();
            return this->localVarDeclarationEnd(prefix.fPosition, prefix.fModifiers, prefix.fType,
                                                this->text(prefix.fName));
        }

        // If this statement wasn't actually a vardecl after all, rewind and try parsing it as an
        // expression-statement instead.
        checkpoint.rewind();
    }
    return this->expressionStatement();
}

// Helper function for varDeclarations(). If this function succeeds, we assume that the rest of the
// statement is a variable-declaration statement, not an expression-statement.
bool DSLParser::varDeclarationsPrefix(VarDeclarationsPrefix* prefixData) {
    prefixData->fPosition = this->position(this->peek());
    prefixData->fModifiers = this->modifiers();
    skstd::optional<DSLType> type = this->type(&prefixData->fModifiers);
    if (!type) {
        return false;
    }
    prefixData->fType = *type;
    return this->expectIdentifier(&prefixData->fName);
}

/* modifiers type IDENTIFIER varDeclarationEnd */
DSLStatement DSLParser::varDeclarations() {
    VarDeclarationsPrefix prefix;
    if (!this->varDeclarationsPrefix(&prefix)) {
        return {};
    }
    return this->localVarDeclarationEnd(prefix.fPosition, prefix.fModifiers, prefix.fType,
            this->text(prefix.fName));
}

/* STRUCT IDENTIFIER LBRACE varDeclaration* RBRACE */
skstd::optional<DSLType> DSLParser::structDeclaration() {
    AutoDSLDepth depth(this);
    if (!depth.increase()) {
        return skstd::nullopt;
    }
    if (!this->expect(Token::Kind::TK_STRUCT, "'struct'")) {
        return skstd::nullopt;
    }
    Token name;
    if (!this->expectIdentifier(&name)) {
        return skstd::nullopt;
    }
    if (!this->expect(Token::Kind::TK_LBRACE, "'{'")) {
        return skstd::nullopt;
    }
    SkTArray<DSLField> fields;
    while (!this->checkNext(Token::Kind::TK_RBRACE)) {
        DSLModifiers modifiers = this->modifiers();
        skstd::optional<DSLType> type = this->type(&modifiers);
        if (!type) {
            return skstd::nullopt;
        }

        do {
            DSLType actualType = *type;
            Token memberName;
            if (!this->expectIdentifier(&memberName)) {
                return skstd::nullopt;
            }

            while (this->checkNext(Token::Kind::TK_LBRACKET)) {
                actualType = dsl::Array(actualType, this->arraySize(), this->position(memberName));
                if (!this->expect(Token::Kind::TK_RBRACKET, "']'")) {
                    return skstd::nullopt;
                }
            }
            fields.push_back(DSLField(modifiers, std::move(actualType), this->text(memberName),
                    this->position(memberName)));
        } while (this->checkNext(Token::Kind::TK_COMMA));
        if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
            return skstd::nullopt;
        }
    }
    if (fields.empty()) {
        this->error(name, "struct '" + this->text(name) + "' must contain at least one field");
    }
    return dsl::Struct(this->text(name), SkMakeSpan(fields), this->position(name));
}

/* structDeclaration ((IDENTIFIER varDeclarationEnd) | SEMICOLON) */
SkTArray<dsl::DSLGlobalVar> DSLParser::structVarDeclaration(const DSLModifiers& modifiers) {
    skstd::optional<DSLType> type = this->structDeclaration();
    if (!type) {
        return {};
    }
    Token name;
    if (this->checkNext(Token::Kind::TK_IDENTIFIER, &name)) {
        this->globalVarDeclarationEnd(this->position(name), modifiers, std::move(*type),
                this->text(name));
    } else {
        this->expect(Token::Kind::TK_SEMICOLON, "';'");
    }
    return {};
}

/* modifiers type IDENTIFIER (LBRACKET INT_LITERAL RBRACKET)? */
skstd::optional<DSLWrapper<DSLParameter>> DSLParser::parameter() {
    DSLModifiers modifiers = this->modifiers();
    skstd::optional<DSLType> type = this->type(&modifiers);
    if (!type) {
        return skstd::nullopt;
    }
    Token name;
    if (!this->expectIdentifier(&name)) {
        return skstd::nullopt;
    }
    if (!this->parseArrayDimensions(name.fLine, &type.value())) {
        return skstd::nullopt;
    }
    return {{DSLParameter(modifiers, *type, this->text(name), this->position(name))}};
}

/** EQ INT_LITERAL */
int DSLParser::layoutInt() {
    if (!this->expect(Token::Kind::TK_EQ, "'='")) {
        return -1;
    }
    Token resultToken;
    if (!this->expect(Token::Kind::TK_INT_LITERAL, "a non-negative integer", &resultToken)) {
        return -1;
    }
    skstd::string_view resultFrag = this->text(resultToken);
    SKSL_INT resultValue;
    if (!SkSL::stoi(resultFrag, &resultValue)) {
        this->error(resultToken, "value in layout is too large: " + resultFrag);
        return -1;
    }
    return resultValue;
}

/** EQ IDENTIFIER */
skstd::string_view DSLParser::layoutIdentifier() {
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
DSLLayout DSLParser::layout() {
    DSLLayout result;
    if (this->checkNext(Token::Kind::TK_LAYOUT)) {
        if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
            return result;
        }
        for (;;) {
            Token t = this->nextToken();
            String text(this->text(t));
            auto found = layoutTokens->find(text);
            if (found != layoutTokens->end()) {
                switch (found->second) {
                    case LayoutToken::ORIGIN_UPPER_LEFT:
                        result.originUpperLeft(this->position(t));
                        break;
                    case LayoutToken::PUSH_CONSTANT:
                        result.pushConstant(this->position(t));
                        break;
                    case LayoutToken::BLEND_SUPPORT_ALL_EQUATIONS:
                        result.blendSupportAllEquations(this->position(t));
                        break;
                    case LayoutToken::SRGB_UNPREMUL:
                        result.srgbUnpremul(this->position(t));
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
                    case LayoutToken::BUILTIN:
                        result.builtin(this->layoutInt(), this->position(t));
                        break;
                    case LayoutToken::INPUT_ATTACHMENT_INDEX:
                        result.inputAttachmentIndex(this->layoutInt(), this->position(t));
                        break;
                    default:
                        this->error(t, "'" + text + "' is not a valid layout qualifier");
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
            VARYING | INLINE)* */
DSLModifiers DSLParser::modifiers() {
    DSLLayout layout = this->layout();
    int flags = 0;
    for (;;) {
        // TODO(ethannicholas): handle duplicate / incompatible flags
        int tokenFlag = parse_modifier_token(peek().fKind);
        if (!tokenFlag) {
            break;
        }
        flags |= tokenFlag;
        this->nextToken();
    }
    return DSLModifiers(std::move(layout), flags);
}

/* ifStatement | forStatement | doStatement | whileStatement | block | expression */
DSLStatement DSLParser::statement() {
    Token start = this->nextToken();
    AutoDSLDepth depth(this);
    if (!depth.increase()) {
        return {};
    }
    this->pushback(start);
    switch (start.fKind) {
        case Token::Kind::TK_IF: // fall through
        case Token::Kind::TK_STATIC_IF:
            return this->ifStatement();
        case Token::Kind::TK_FOR:
            return this->forStatement();
        case Token::Kind::TK_DO:
            return this->doStatement();
        case Token::Kind::TK_WHILE:
            return this->whileStatement();
        case Token::Kind::TK_SWITCH: // fall through
        case Token::Kind::TK_STATIC_SWITCH:
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
            skstd::optional<DSLBlock> result = this->block();
            return result ? DSLStatement(std::move(*result)) : DSLStatement();
        }
        case Token::Kind::TK_SEMICOLON:
            this->nextToken();
            return dsl::Block();
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
skstd::optional<DSLType> DSLParser::type(DSLModifiers* modifiers) {
    Token type;
    if (!this->expect(Token::Kind::TK_IDENTIFIER, "a type", &type)) {
        return skstd::nullopt;
    }
    if (!IsType(this->text(type))) {
        this->error(type, ("no type named '" + this->text(type) + "'").c_str());
        return skstd::nullopt;
    }
    DSLType result(this->text(type), modifiers, this->position(type));
    while (this->checkNext(Token::Kind::TK_LBRACKET)) {
        if (this->peek().fKind != Token::Kind::TK_RBRACKET) {
            result = Array(result, this->arraySize(), this->position(type));
        } else {
            this->error(this->peek(), "expected array dimension");
        }
        this->expect(Token::Kind::TK_RBRACKET, "']'");
    }
    return result;
}

/* IDENTIFIER LBRACE
     varDeclaration+
   RBRACE (IDENTIFIER (LBRACKET expression? RBRACKET)*)? SEMICOLON */
bool DSLParser::interfaceBlock(const dsl::DSLModifiers& modifiers) {
    Token typeName;
    if (!this->expectIdentifier(&typeName)) {
        return false;
    }
    if (peek().fKind != Token::Kind::TK_LBRACE) {
        // we only get into interfaceBlock if we found a top-level identifier which was not a type.
        // 99% of the time, the user was not actually intending to create an interface block, so
        // it's better to report it as an unknown type
        this->error(typeName, "no type named '" + this->text(typeName) + "'");
        return false;
    }
    this->nextToken();
    SkTArray<dsl::Field> fields;
    while (!this->checkNext(Token::Kind::TK_RBRACE)) {
        DSLModifiers fieldModifiers = this->modifiers();
        skstd::optional<dsl::DSLType> type = this->type(&fieldModifiers);
        if (!type) {
            return false;
        }
        do {
            Token fieldName;
            if (!this->expect(Token::Kind::TK_IDENTIFIER, "an identifier", &fieldName)) {
                return false;
            }
            DSLType actualType = *type;
            if (this->checkNext(Token::Kind::TK_LBRACKET)) {
                Token sizeToken = this->peek();
                if (sizeToken.fKind != Token::Kind::TK_RBRACKET) {
                    actualType = Array(std::move(actualType), this->arraySize(),
                                       this->position(typeName));
                } else {
                    this->error(sizeToken, "unsized arrays are not permitted");
                }
                this->expect(Token::Kind::TK_RBRACKET, "']'");
            }
            if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
                return false;
            }
            fields.push_back(dsl::Field(fieldModifiers, std::move(actualType),
                    this->text(fieldName), this->position(fieldName)));
        }
        while (this->checkNext(Token::Kind::TK_COMMA));
    }
    skstd::string_view instanceName;
    Token instanceNameToken;
    SKSL_INT arraySize = 0;
    if (this->checkNext(Token::Kind::TK_IDENTIFIER, &instanceNameToken)) {
        instanceName = this->text(instanceNameToken);
        if (this->checkNext(Token::Kind::TK_LBRACKET)) {
            arraySize = this->arraySize();
            this->expect(Token::Kind::TK_RBRACKET, "']'");
        }
    }
    this->expect(Token::Kind::TK_SEMICOLON, "';'");
    if (fields.empty()) {
        this->error(typeName, "interface block '" + this->text(typeName) +
                              "' must contain at least one member");
    } else {
        dsl::InterfaceBlock(modifiers, this->text(typeName), std::move(fields), instanceName,
                            arraySize, this->position(typeName));
    }
    return true;
}

/* IF LPAREN expression RPAREN statement (ELSE statement)? */
DSLStatement DSLParser::ifStatement() {
    Token start;
    bool isStatic = this->checkNext(Token::Kind::TK_STATIC_IF, &start);
    if (!isStatic && !this->expect(Token::Kind::TK_IF, "'if'", &start)) {
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
    if (isStatic) {
        return StaticIf(std::move(test), std::move(ifTrue),
                ifFalse.hasValue() ? std::move(ifFalse) : DSLStatement(), this->position(start));
    } else {
        return If(std::move(test), std::move(ifTrue),
                ifFalse.hasValue() ? std::move(ifFalse) : DSLStatement(), this->position(start));
    }
}

/* DO statement WHILE LPAREN expression RPAREN SEMICOLON */
DSLStatement DSLParser::doStatement() {
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
    return Do(std::move(statement), std::move(test), this->position(start));
}

/* WHILE LPAREN expression RPAREN STATEMENT */
DSLStatement DSLParser::whileStatement() {
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
    return While(std::move(test), std::move(statement), this->position(start));
}

/* CASE expression COLON statement* */
skstd::optional<DSLCase> DSLParser::switchCase() {
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
DSLStatement DSLParser::switchStatement() {
    Token start;
    bool isStatic = this->checkNext(Token::Kind::TK_STATIC_SWITCH, &start);
    if (!isStatic && !this->expect(Token::Kind::TK_SWITCH, "'switch'", &start)) {
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
        skstd::optional<DSLCase> c = this->switchCase();
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
    if (isStatic) {
        return StaticSwitch(std::move(value), std::move(cases), this->position(start));
    } else {
        return Switch(std::move(value), std::move(cases), this->position(start));
    }
}

/* FOR LPAREN (declaration | expression)? SEMICOLON expression? SEMICOLON expression? RPAREN
   STATEMENT */
dsl::DSLStatement DSLParser::forStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_FOR, "'for'", &start)) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
        return {};
    }
    AutoDSLSymbolTable symbols;
    dsl::DSLStatement initializer;
    Token nextToken = this->peek();
    if (nextToken.fKind == Token::Kind::TK_SEMICOLON) {
        // An empty init-statement.
        this->nextToken();
    } else {
        // The init-statement must be an expression or variable declaration.
        initializer = this->varDeclarationsOrExpressionStatement();
        if (!initializer.hasValue()) {
            return {};
        }
    }
    dsl::DSLExpression test;
    if (this->peek().fKind != Token::Kind::TK_SEMICOLON) {
        dsl::DSLExpression testValue = this->expression();
        if (!testValue.hasValue()) {
            return {};
        }
        test.swap(testValue);
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
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
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
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
               this->position(start));
}

/* RETURN expression? SEMICOLON */
DSLStatement DSLParser::returnStatement() {
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
            this->position(start));
}

/* BREAK SEMICOLON */
DSLStatement DSLParser::breakStatement() {
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
DSLStatement DSLParser::continueStatement() {
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
DSLStatement DSLParser::discardStatement() {
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
skstd::optional<DSLBlock> DSLParser::block() {
    Token start;
    if (!this->expect(Token::Kind::TK_LBRACE, "'{'", &start)) {
        return skstd::nullopt;
    }
    AutoDSLDepth depth(this);
    if (!depth.increase()) {
        return skstd::nullopt;
    }
    AutoDSLSymbolTable symbols;
    StatementArray statements;
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_RBRACE:
                this->nextToken();
                return DSLBlock(std::move(statements), CurrentSymbolTable());
            case Token::Kind::TK_END_OF_FILE:
                this->error(this->peek(), "expected '}', but found end of file");
                return skstd::nullopt;
            default: {
                DSLStatement statement = this->statement();
                if (!statement.hasValue()) {
                    return skstd::nullopt;
                }
                statements.push_back(statement.release());
                break;
            }
        }
    }
}

/* expression SEMICOLON */
DSLStatement DSLParser::expressionStatement() {
    DSLExpression expr = this->expression();
    if (expr.hasValue()) {
        if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
            return {};
        }
        return DSLStatement(std::move(expr));
    }
    return {};
}

/* assignmentExpression (COMMA assignmentExpression)* */
DSLExpression DSLParser::expression() {
    DSLExpression result = this->assignmentExpression();
    if (!result.hasValue()) {
        return {};
    }
    Token t;
    AutoDSLDepth depth(this);
    while (this->checkNext(Token::Kind::TK_COMMA, &t)) {
        if (!depth.increase()) {
            return {};
        }
        DSLExpression right = this->assignmentExpression();
        if (!right.hasValue()) {
            return {};
        }
        DSLExpression next = dsl::operator,(std::move(result), std::move(right));
        result.swap(next);
    }
    return result;
}

#define OPERATOR_RIGHT(op, exprType)                                         \
    do {                                                                     \
        this->nextToken();                                                   \
        if (!depth.increase()) {                                             \
            return {};                                                       \
        }                                                                    \
        DSLExpression right = this->exprType();                              \
        if (!right.hasValue()) {                                             \
            return {};                                                       \
        }                                                                    \
        DSLExpression next = std::move(result) op std::move(right);          \
        result.swap(next);                                                   \
    } while (false)

/* ternaryExpression ((EQEQ | STAREQ | SLASHEQ | PERCENTEQ | PLUSEQ | MINUSEQ | SHLEQ | SHREQ |
   BITWISEANDEQ | BITWISEXOREQ | BITWISEOREQ | LOGICALANDEQ | LOGICALXOREQ | LOGICALOREQ)
   assignmentExpression)*
 */
DSLExpression DSLParser::assignmentExpression() {
    AutoDSLDepth depth(this);
    DSLExpression result = this->ternaryExpression();
    if (!result.hasValue()) {
        return {};
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_EQ:           OPERATOR_RIGHT(=,   assignmentExpression); break;
            case Token::Kind::TK_STAREQ:       OPERATOR_RIGHT(*=,  assignmentExpression); break;
            case Token::Kind::TK_SLASHEQ:      OPERATOR_RIGHT(/=,  assignmentExpression); break;
            case Token::Kind::TK_PERCENTEQ:    OPERATOR_RIGHT(%=,  assignmentExpression); break;
            case Token::Kind::TK_PLUSEQ:       OPERATOR_RIGHT(+=,  assignmentExpression); break;
            case Token::Kind::TK_MINUSEQ:      OPERATOR_RIGHT(-=,  assignmentExpression); break;
            case Token::Kind::TK_SHLEQ:        OPERATOR_RIGHT(<<=, assignmentExpression); break;
            case Token::Kind::TK_SHREQ:        OPERATOR_RIGHT(>>=, assignmentExpression); break;
            case Token::Kind::TK_BITWISEANDEQ: OPERATOR_RIGHT(&=,  assignmentExpression); break;
            case Token::Kind::TK_BITWISEXOREQ: OPERATOR_RIGHT(^=,  assignmentExpression); break;
            case Token::Kind::TK_BITWISEOREQ:  OPERATOR_RIGHT(|=,  assignmentExpression); break;
            default:
                return result;
        }
    }
}

/* logicalOrExpression ('?' expression ':' assignmentExpression)? */
DSLExpression DSLParser::ternaryExpression() {
    AutoDSLDepth depth(this);
    DSLExpression base = this->logicalOrExpression();
    if (!base.hasValue()) {
        return {};
    }
    if (this->checkNext(Token::Kind::TK_QUESTION)) {
        if (!depth.increase()) {
            return {};
        }
        DSLExpression trueExpr = this->expression();
        if (!trueExpr.hasValue()) {
            return {};
        }
        if (this->expect(Token::Kind::TK_COLON, "':'")) {
            DSLExpression falseExpr = this->assignmentExpression();
            if (!falseExpr.hasValue()) {
                return {};
            }
            return Select(std::move(base), std::move(trueExpr), std::move(falseExpr));
        }
        return {};
    }
    return base;
}

/* logicalXorExpression (LOGICALOR logicalXorExpression)* */
DSLExpression DSLParser::logicalOrExpression() {
    AutoDSLDepth depth(this);
    DSLExpression result = this->logicalXorExpression();
    if (!result.hasValue()) {
        return {};
    }
    while (this->peek().fKind == Token::Kind::TK_LOGICALOR) {
        OPERATOR_RIGHT(||, logicalXorExpression);
    }
    return result;
}

/* logicalAndExpression (LOGICALXOR logicalAndExpression)* */
DSLExpression DSLParser::logicalXorExpression() {
    AutoDSLDepth depth(this);
    DSLExpression result = this->logicalAndExpression();
    if (!result.hasValue()) {
        return {};
    }
    while (this->checkNext(Token::Kind::TK_LOGICALXOR)) {
        if (!depth.increase()) {
            return {};
        }
        DSLExpression right = this->logicalAndExpression();
        if (!right.hasValue()) {
            return {};
        }
        DSLExpression next = LogicalXor(std::move(result), std::move(right));
        result.swap(next);
    }
    return result;
}

/* bitwiseOrExpression (LOGICALAND bitwiseOrExpression)* */
DSLExpression DSLParser::logicalAndExpression() {
    AutoDSLDepth depth(this);
    DSLExpression result = this->bitwiseOrExpression();
    if (!result.hasValue()) {
        return {};
    }
    while (this->peek().fKind == Token::Kind::TK_LOGICALAND) {
        OPERATOR_RIGHT(&&, bitwiseOrExpression);
    }
    return result;
}

/* bitwiseXorExpression (BITWISEOR bitwiseXorExpression)* */
DSLExpression DSLParser::bitwiseOrExpression() {
    AutoDSLDepth depth(this);
    DSLExpression result = this->bitwiseXorExpression();
    if (!result.hasValue()) {
        return {};
    }
    while (this->peek().fKind == Token::Kind::TK_BITWISEOR) {
        OPERATOR_RIGHT(|, bitwiseXorExpression);
    }
    return result;
}

/* bitwiseAndExpression (BITWISEXOR bitwiseAndExpression)* */
DSLExpression DSLParser::bitwiseXorExpression() {
    AutoDSLDepth depth(this);
    DSLExpression result = this->bitwiseAndExpression();
    if (!result.hasValue()) {
        return {};
    }
    while (this->peek().fKind == Token::Kind::TK_BITWISEXOR) {
        OPERATOR_RIGHT(^, bitwiseAndExpression);
    }
    return result;
}

/* equalityExpression (BITWISEAND equalityExpression)* */
DSLExpression DSLParser::bitwiseAndExpression() {
    AutoDSLDepth depth(this);
    DSLExpression result = this->equalityExpression();
    if (!result.hasValue()) {
        return {};
    }
    while (this->peek().fKind == Token::Kind::TK_BITWISEAND) {
        OPERATOR_RIGHT(&, equalityExpression);
    }
    return result;
}

/* relationalExpression ((EQEQ | NEQ) relationalExpression)* */
DSLExpression DSLParser::equalityExpression() {
    AutoDSLDepth depth(this);
    DSLExpression result = this->relationalExpression();
    if (!result.hasValue()) {
        return {};
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_EQEQ: OPERATOR_RIGHT(==, relationalExpression); break;
            case Token::Kind::TK_NEQ:  OPERATOR_RIGHT(!=, relationalExpression); break;
            default: return result;
        }
    }
}

/* shiftExpression ((LT | GT | LTEQ | GTEQ) shiftExpression)* */
DSLExpression DSLParser::relationalExpression() {
    AutoDSLDepth depth(this);
    DSLExpression result = this->shiftExpression();
    if (!result.hasValue()) {
        return {};
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_LT:   OPERATOR_RIGHT(<,  shiftExpression); break;
            case Token::Kind::TK_GT:   OPERATOR_RIGHT(>,  shiftExpression); break;
            case Token::Kind::TK_LTEQ: OPERATOR_RIGHT(<=, shiftExpression); break;
            case Token::Kind::TK_GTEQ: OPERATOR_RIGHT(>=, shiftExpression); break;
            default: return result;
        }
    }
}

/* additiveExpression ((SHL | SHR) additiveExpression)* */
DSLExpression DSLParser::shiftExpression() {
    AutoDSLDepth depth(this);
    DSLExpression result = this->additiveExpression();
    if (!result.hasValue()) {
        return {};
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_SHL: OPERATOR_RIGHT(<<, additiveExpression); break;
            case Token::Kind::TK_SHR: OPERATOR_RIGHT(>>, additiveExpression); break;
            default: return result;
        }
    }
}

/* multiplicativeExpression ((PLUS | MINUS) multiplicativeExpression)* */
DSLExpression DSLParser::additiveExpression() {
    AutoDSLDepth depth(this);
    DSLExpression result = this->multiplicativeExpression();
    if (!result.hasValue()) {
        return {};
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_PLUS:  OPERATOR_RIGHT(+, multiplicativeExpression); break;
            case Token::Kind::TK_MINUS: OPERATOR_RIGHT(-, multiplicativeExpression); break;
            default: return result;
        }
    }
}

/* unaryExpression ((STAR | SLASH | PERCENT) unaryExpression)* */
DSLExpression DSLParser::multiplicativeExpression() {
    AutoDSLDepth depth(this);
    DSLExpression result = this->unaryExpression();
    if (!result.hasValue()) {
        return {};
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_STAR:    OPERATOR_RIGHT(*, unaryExpression); break;
            case Token::Kind::TK_SLASH:   OPERATOR_RIGHT(/, unaryExpression); break;
            case Token::Kind::TK_PERCENT: OPERATOR_RIGHT(%, unaryExpression); break;
            default: return result;
        }
    }
}

/* postfixExpression | (PLUS | MINUS | NOT | PLUSPLUS | MINUSMINUS) unaryExpression */
DSLExpression DSLParser::unaryExpression() {
    AutoDSLDepth depth(this);
    Token next = this->peek();
    switch (next.fKind) {
        case Token::Kind::TK_PLUS:
        case Token::Kind::TK_MINUS:
        case Token::Kind::TK_LOGICALNOT:
        case Token::Kind::TK_BITWISENOT:
        case Token::Kind::TK_PLUSPLUS:
        case Token::Kind::TK_MINUSMINUS: {
            if (!depth.increase()) {
                return {};
            }
            this->nextToken();
            DSLExpression expr = this->unaryExpression();
            if (!expr.hasValue()) {
                return {};
            }
            switch (next.fKind) {
                case Token::Kind::TK_PLUS:       return {{ +std::move(expr)}};
                case Token::Kind::TK_MINUS:      return {{ -std::move(expr)}};
                case Token::Kind::TK_LOGICALNOT: return {{ !std::move(expr)}};
                case Token::Kind::TK_BITWISENOT: return {{ ~std::move(expr)}};
                case Token::Kind::TK_PLUSPLUS:   return {{++std::move(expr)}};
                case Token::Kind::TK_MINUSMINUS: return {{--std::move(expr)}};
                default: SkUNREACHABLE;
            }
        }
        default:
            return this->postfixExpression();
    }
}

/* term suffix* */
DSLExpression DSLParser::postfixExpression() {
    AutoDSLDepth depth(this);
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

DSLExpression DSLParser::swizzle(int line, DSLExpression base,
        skstd::string_view swizzleMask) {
    SkASSERT(swizzleMask.length() > 0);
    if (!base.type().isVector() && !base.type().isScalar()) {
        return base.field(swizzleMask, this->position(line));
    }
    int length = swizzleMask.length();
    SkSL::SwizzleComponent::Type components[4];
    for (int i = 0; i < length; ++i) {
        if (i >= 4) {
            this->error(line, "too many components in swizzle mask");
            return DSLExpression::Poison();
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
            default:
                this->error(line,
                        String::printf("invalid swizzle component '%c'", swizzleMask[i]).c_str());
                return DSLExpression::Poison();
        }
    }
    switch (length) {
        case 1: return dsl::Swizzle(std::move(base), components[0]);
        case 2: return dsl::Swizzle(std::move(base), components[0], components[1]);
        case 3: return dsl::Swizzle(std::move(base), components[0], components[1], components[2]);
        case 4: return dsl::Swizzle(std::move(base), components[0], components[1], components[2],
                                    components[3]);
        default: SkUNREACHABLE;
    }
}

dsl::DSLExpression DSLParser::call(int line, dsl::DSLExpression base, ExpressionArray args) {
    return DSLExpression(base(std::move(args), this->position(line)), this->position(line));
}

/* LBRACKET expression? RBRACKET | DOT IDENTIFIER | LPAREN arguments RPAREN |
   PLUSPLUS | MINUSMINUS | COLONCOLON IDENTIFIER | FLOAT_LITERAL [IDENTIFIER] */
DSLExpression DSLParser::suffix(DSLExpression base) {
    Token next = this->nextToken();
    AutoDSLDepth depth(this);
    if (!depth.increase()) {
        return {};
    }
    switch (next.fKind) {
        case Token::Kind::TK_LBRACKET: {
            if (this->checkNext(Token::Kind::TK_RBRACKET)) {
                this->error(next, "missing index in '[]'");
                return DSLExpression::Poison();
            }
            DSLExpression index = this->expression();
            if (!index.hasValue()) {
                return {};
            }
            this->expect(Token::Kind::TK_RBRACKET, "']' to complete array access expression");
            DSLPossibleExpression result = base[std::move(index)];
            if (!result.valid()) {
                result.reportErrors(this->position(next));
            }
            return std::move(result);
        }
        case Token::Kind::TK_DOT: {
            int line = this->peek().fLine;
            skstd::string_view text;
            if (this->identifier(&text)) {
                return this->swizzle(line, std::move(base), text);
            }
            [[fallthrough]];
        }
        case Token::Kind::TK_FLOAT_LITERAL: {
            // Swizzles that start with a constant number, e.g. '.000r', will be tokenized as
            // floating point literals, possibly followed by an identifier. Handle that here.
            skstd::string_view field = this->text(next);
            SkASSERT(field[0] == '.');
            field.remove_prefix(1);
            // use the next *raw* token so we don't ignore whitespace - we only care about
            // identifiers that directly follow the float
            Token id = this->nextRawToken();
            if (id.fKind == Token::Kind::TK_IDENTIFIER) {
                return this->swizzle(next.fLine, std::move(base), field + this->text(id));
            } else if (field.empty()) {
                this->error(next, "expected field name or swizzle mask after '.'");
                return {{DSLExpression::Poison()}};
            }
            this->pushback(id);
            return this->swizzle(next.fLine, std::move(base), field);
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
            return this->call(next.fLine, std::move(base), std::move(args));
        }
        case Token::Kind::TK_PLUSPLUS:
            return std::move(base)++;
        case Token::Kind::TK_MINUSMINUS:
            return std::move(base)--;
        default: {
            this->error(next,  "expected expression suffix, but found '" + this->text(next) + "'");
            return {};
        }
    }
}

/* IDENTIFIER | intLiteral | floatLiteral | boolLiteral | '(' expression ')' */
DSLExpression DSLParser::term() {
    Token t = this->peek();
    switch (t.fKind) {
        case Token::Kind::TK_IDENTIFIER: {
            skstd::string_view text;
            if (this->identifier(&text)) {
                return dsl::Symbol(text, this->position(t));
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
            AutoDSLDepth depth(this);
            if (!depth.increase()) {
                return {};
            }
            DSLExpression result = this->expression();
            if (result.hasValue()) {
                this->expect(Token::Kind::TK_RPAREN, "')' to complete expression");
                return result;
            }
            break;
        }
        default:
            this->nextToken();
            this->error(t, "expected expression, but found '" + this->text(t) + "'");
            fEncounteredFatalError = true;
            break;
    }
    return {};
}

/* INT_LITERAL */
bool DSLParser::intLiteral(SKSL_INT* dest) {
    Token t;
    if (!this->expect(Token::Kind::TK_INT_LITERAL, "integer literal", &t)) {
        return false;
    }
    skstd::string_view s = this->text(t);
    if (!SkSL::stoi(s, dest)) {
        this->error(t, "integer is too large: " + s);
        return false;
    }
    return true;
}

/* FLOAT_LITERAL */
bool DSLParser::floatLiteral(SKSL_FLOAT* dest) {
    Token t;
    if (!this->expect(Token::Kind::TK_FLOAT_LITERAL, "float literal", &t)) {
        return false;
    }
    skstd::string_view s = this->text(t);
    if (!SkSL::stod(s, dest)) {
        this->error(t, "floating-point value is too large: " + s);
        return false;
    }
    return true;
}

/* TRUE_LITERAL | FALSE_LITERAL */
bool DSLParser::boolLiteral(bool* dest) {
    Token t = this->nextToken();
    switch (t.fKind) {
        case Token::Kind::TK_TRUE_LITERAL:
            *dest = true;
            return true;
        case Token::Kind::TK_FALSE_LITERAL:
            *dest = false;
            return true;
        default:
            this->error(t, "expected 'true' or 'false', but found '" + this->text(t) + "'");
            return false;
    }
}

/* IDENTIFIER */
bool DSLParser::identifier(skstd::string_view* dest) {
    Token t;
    if (this->expect(Token::Kind::TK_IDENTIFIER, "identifier", &t)) {
        *dest = this->text(t);
        return true;
    }
    return false;
}

}  // namespace SkSL
