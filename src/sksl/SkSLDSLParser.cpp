/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLDSLParser.h"

#include "include/private/SkSLString.h"
#include "src/sksl/SkSLCompiler.h"

#include <memory>

#if SKSL_DSL_PARSER

using namespace SkSL::dsl;

namespace SkSL {

static constexpr int kMaxParseDepth = 50;
static constexpr int kMaxStructDepth = 8;

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
    TOKEN(POINTS,                       "points");
    TOKEN(LINES,                        "lines");
    TOKEN(LINE_STRIP,                   "line_strip");
    TOKEN(LINES_ADJACENCY,              "lines_adjacency");
    TOKEN(TRIANGLES,                    "triangles");
    TOKEN(TRIANGLE_STRIP,               "triangle_strip");
    TOKEN(TRIANGLES_ADJACENCY,          "triangles_adjacency");
    TOKEN(MAX_VERTICES,                 "max_vertices");
    TOKEN(INVOCATIONS,                  "invocations");
    TOKEN(SRGB_UNPREMUL,                "srgb_unpremul");
    #undef TOKEN
}

DSLParser::DSLParser(Compiler* compiler, const ProgramSettings& settings, ProgramKind kind,
                     String text)
    : fCompiler(*compiler)
    , fSettings(settings)
    , fErrorReporter(compiler)
    , fKind(kind)
    , fText(std::move(text))
    , fPushback(Token::Kind::TK_NONE, -1, -1) {
    // We don't want to have to worry about manually releasing all of the objects in the event that
    // an error occurs
    fSettings.fAssertDSLObjectsReleased = false;
    fLexer.start(fText);
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
        return false;
    }
}

bool DSLParser::expectIdentifier(Token* result) {
    if (!this->expect(Token::Kind::TK_IDENTIFIER, "an identifier", result)) {
        return false;
    }
    if (IsType(this->text(*result))) {
        this->error(*result, "expected an identifier, but found type '" +
                             this->text(*result) + "'");
        return false;
    }
    return true;
}

skstd::string_view DSLParser::text(Token token) {
    return skstd::string_view(fText.data() + token.fOffset, token.fLength);
}

void DSLParser::error(Token token, String msg) {
    this->error(token.fOffset, msg);
}

void DSLParser::error(int offset, String msg) {
    fErrorReporter->error(offset, msg);
}

class DSLParserErrorHandler final : public ErrorHandler {
public:
    DSLParserErrorHandler(ErrorReporter* reporter)
        : fReporter(*reporter) {}

    ~DSLParserErrorHandler() override {
        for (const String& s : fErrors) {
            fReporter.error(/*offset=*/-1, s.c_str());
        }
    }

    void handleError(const char* msg, PositionInfo position) override {
        fErrors.push_back(msg);
    }

private:
    SkTArray<String> fErrors;
    ErrorReporter& fReporter;

    friend class DSLParser;
};

/* declaration* END_OF_FILE */
std::unique_ptr<Program> DSLParser::program() {
    Start(&fCompiler, fKind, fSettings);
    DSLParserErrorHandler errorHandler(&fCompiler);
    SetErrorHandler(&errorHandler);
    std::unique_ptr<Program> result;
    bool done = false;
    while (!done) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_END_OF_FILE:
                done = true;
                if (errorHandler.fErrors.empty()) {
                    result = dsl::ReleaseProgram(std::make_unique<String>(std::move(fText)));
                }
                break;
            case Token::Kind::TK_INVALID: {
                this->error(this->peek(), String("invalid token"));
                done = true;
                break;
            }
            default: {
                if (!this->declaration()) {
                    done = true;
                    break;
                }
            }
        }
    }
    End();
    return result;
}

/* modifiers (structVarDeclaration | type IDENTIFIER ((LPAREN parameter (COMMA parameter)* RPAREN
   (block | SEMICOLON)) | SEMICOLON) | interfaceBlock) */
bool DSLParser::declaration() {
    Token lookahead = this->peek();
    switch (lookahead.fKind) {
        case Token::Kind::TK_SEMICOLON:
            this->error(lookahead.fOffset, "expected a declaration, but found ';'");
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
        this->error(lookahead, "modifiers declarations are not yet supported");
    }
    if (lookahead.fKind == Token::Kind::TK_STRUCT) {
        SkTArray<DSLGlobalVar> result = this->structVarDeclaration(modifiers);
        Declare(result);
        return true;
    }
    skstd::optional<DSLType> type = this->type(modifiers);
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
        SkTArray<DSLGlobalVar> result = this->varDeclarationEnd<DSLGlobalVar>(modifiers, *type,
                                                                              this->text(name));
        Declare(result);
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
    DSLFunction result(modifiers, type, this->text(name), parameterPointers);
    if (!this->checkNext(Token::Kind::TK_SEMICOLON)) {
        AutoDSLSymbolTable symbols;
        for (DSLParameter* var : parameterPointers) {
            AddToSymbolTable(*var);
        }
        skstd::optional<DSLBlock> body = this->block();
        if (!body) {
            return false;
        }
        result.define(std::move(*body));
    }
    return true;
}

static skstd::optional<DSLStatement> declaration_statements(SkTArray<DSLVar> vars,
                                                            SymbolTable& symbols) {
    if (vars.empty()) {
        return skstd::nullopt;
    }
    return Declare(vars);
}

template<class T>
SkTArray<T> DSLParser::varDeclarationEnd(const dsl::DSLModifiers& mods, dsl::DSLType baseType,
                                         skstd::string_view name) {
    using namespace dsl;
    SkTArray<T> result;
    int offset = this->peek().fOffset;
    auto parseArrayDimensions = [&](DSLType* type) -> bool {
        while (this->checkNext(Token::Kind::TK_LBRACKET)) {
            if (type->isArray()) {
                this->error(this->peek(), "multi-dimensional arrays are not supported");
                return {};
            }
            if (this->checkNext(Token::Kind::TK_RBRACKET)) {
                this->error(offset, "expected array dimension");
            } else {
                SKSL_INT size;
                if (!this->intLiteral(&size)) {
                    return {};
                }
                *type = Array(*type, size);
                if (!this->expect(Token::Kind::TK_RBRACKET, "']'")) {
                    return {};
                }
            }
        }
        return true;
    };
    auto parseInitializer = [this](DSLExpression* initializer) -> bool {
        if (this->checkNext(Token::Kind::TK_EQ)) {
            skstd::optional<DSLWrapper<DSLExpression>> value = this->assignmentExpression();
            if (!value) {
                return false;
            }
            initializer->swap(**value);
        }
        return true;
    };

    DSLType type = baseType;
    DSLExpression initializer;
    if (!parseArrayDimensions(&type)) {
        return {};
    }
    if (!parseInitializer(&initializer)) {
        return {};
    }
    result.push_back(T(mods, type, name, std::move(initializer)));
    AddToSymbolTable(result.back());

    while (this->checkNext(Token::Kind::TK_COMMA)) {
        type = baseType;
        Token identifierName;
        if (!this->expectIdentifier(&identifierName)) {
            return {};
        }
        if (!parseArrayDimensions(&type)) {
            return {};
        }
        if (!parseInitializer(&initializer)) {
            return {};
        }
        result.push_back(T(mods, type, this->text(identifierName), std::move(initializer)));
        AddToSymbolTable(result.back());
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return {};
    }
    return result;
}

/* (varDeclarations | expressionStatement) */
skstd::optional<DSLStatement> DSLParser::varDeclarationsOrExpressionStatement() {
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
            return declaration_statements(this->varDeclarationEnd<DSLVar>(prefix.modifiers,
                                                                          prefix.type,
                                                                          this->text(prefix.name)),
                                          this->symbols());
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
    prefixData->modifiers = this->modifiers();
    skstd::optional<DSLType> type = this->type(prefixData->modifiers);
    if (!type) {
        return false;
    }
    prefixData->type = *type;
    return this->expectIdentifier(&prefixData->name);
}

/* modifiers type IDENTIFIER varDeclarationEnd */
skstd::optional<DSLStatement> DSLParser::varDeclarations() {
    VarDeclarationsPrefix prefix;
    if (!this->varDeclarationsPrefix(&prefix)) {
        return skstd::nullopt;
    }
    return declaration_statements(this->varDeclarationEnd<DSLVar>(prefix.modifiers, prefix.type,
                                                                  this->text(prefix.name)),
                                  this->symbols());
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
    if (fDepth > kMaxStructDepth) {
        this->error(name.fOffset, "struct '" + this->text(name) + "' is too deeply nested");
        return skstd::nullopt;
    }
    if (!this->expect(Token::Kind::TK_LBRACE, "'{'")) {
        return skstd::nullopt;
    }
    SkTArray<DSLField> fields;
    while (!this->checkNext(Token::Kind::TK_RBRACE)) {
        DSLModifiers modifiers = this->modifiers();

        skstd::optional<DSLType> type = this->type(modifiers);
        if (!type) {
            return skstd::nullopt;
        }

        do {
            DSLType actualType = *type;
            Token memberName;
            if (!this->expectIdentifier(&memberName)) {
                return skstd::nullopt;
            }

            if (this->checkNext(Token::Kind::TK_LBRACKET)) {
                SKSL_INT arraySize;
                Token sizeToken = this->peek();
                if (!this->DSLParser::intLiteral(&arraySize)) {
                    this->error(sizeToken.fOffset, "expected an integer array size");
                    return skstd::nullopt;
                }
                if (arraySize <= 0 || arraySize > INT_MAX) {
                    this->error(sizeToken.fOffset, "array size is invalid");
                    return skstd::nullopt;
                }
                actualType = dsl::Array(actualType, arraySize);
                if (!this->expect(Token::Kind::TK_RBRACKET, "']'")) {
                    return skstd::nullopt;
                }
            }
            fields.push_back(DSLField(modifiers, std::move(actualType), this->text(memberName)));
        } while (this->checkNext(Token::Kind::TK_COMMA));
        if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
            return skstd::nullopt;
        }
    }
    if (fields.empty()) {
        this->error(name.fOffset,
                    "struct '" + this->text(name) + "' must contain at least one field");
        return skstd::nullopt;
    }
    return dsl::Struct(this->text(name), std::move(fields));
}

/* structDeclaration ((IDENTIFIER varDeclarationEnd) | SEMICOLON) */
SkTArray<dsl::DSLGlobalVar> DSLParser::structVarDeclaration(const DSLModifiers& modifiers) {
    skstd::optional<DSLType> type = this->structDeclaration();
    if (!type) {
        return {};
    }
    Token name;
    if (this->checkNext(Token::Kind::TK_IDENTIFIER, &name)) {
        return this->varDeclarationEnd<DSLGlobalVar>(modifiers, std::move(*type), this->text(name));
    }
    this->expect(Token::Kind::TK_SEMICOLON, "';'");
    return {};
}

/* modifiers type IDENTIFIER (LBRACKET INT_LITERAL RBRACKET)? */
skstd::optional<DSLWrapper<DSLParameter>> DSLParser::parameter() {
    DSLModifiers modifiers = this->modifiersWithDefaults(0);
    skstd::optional<DSLType> type = this->type(modifiers);
    if (!type) {
        return skstd::nullopt;
    }
    Token name;
    if (!this->expectIdentifier(&name)) {
        return skstd::nullopt;
    }
    while (this->checkNext(Token::Kind::TK_LBRACKET)) {
        if (type->isArray()) {
            this->error(this->peek(), "multi-dimensional arrays are not supported");
            return skstd::nullopt;
        }
        Token sizeToken;
        if (!this->expect(Token::Kind::TK_INT_LITERAL, "a positive integer", &sizeToken)) {
            return skstd::nullopt;
        }
        skstd::string_view arraySizeFrag = this->text(sizeToken);
        SKSL_INT arraySize;
        if (!SkSL::stoi(arraySizeFrag, &arraySize)) {
            this->error(sizeToken, "array size is too large: " + arraySizeFrag);
            return skstd::nullopt;
        }
        type = Array(*type, arraySize);
        if (!this->expect(Token::Kind::TK_RBRACKET, "']'")) {
            return skstd::nullopt;
        }
    }
    return {{DSLParameter(modifiers, *type, this->text(name))}};
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
                        result.originUpperLeft();
                        break;
                    case LayoutToken::PUSH_CONSTANT:
                        result.pushConstant();
                        break;
                    case LayoutToken::BLEND_SUPPORT_ALL_EQUATIONS:
                        result.blendSupportAllEquations();
                        break;
                    case LayoutToken::SRGB_UNPREMUL:
                        result.srgbUnpremul();
                        break;
                    case LayoutToken::LOCATION:
                        result.location(this->layoutInt());
                        break;
                    case LayoutToken::OFFSET:
                        result.offset(this->layoutInt());
                        break;
                    case LayoutToken::BINDING:
                        result.binding(this->layoutInt());
                        break;
                    case LayoutToken::INDEX:
                        result.index(this->layoutInt());
                        break;
                    case LayoutToken::SET:
                        result.set(this->layoutInt());
                        break;
                    case LayoutToken::BUILTIN:
                        result.builtin(this->layoutInt());
                        break;
                    case LayoutToken::INPUT_ATTACHMENT_INDEX:
                        result.inputAttachmentIndex(this->layoutInt());
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

DSLModifiers DSLParser::modifiersWithDefaults(int defaultFlags) {
    DSLModifiers result = this->modifiers();
    if (defaultFlags && !result.flags()) {
        return DSLModifiers(result.layout(), defaultFlags);
    }
    return result;
}

/* ifStatement | forStatement | doStatement | whileStatement | block | expression */
skstd::optional<DSLStatement> DSLParser::statement() {
    Token start = this->nextToken();
    AutoDSLDepth depth(this);
    if (!depth.increase()) {
        return skstd::nullopt;
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
            return result ? skstd::optional<DSLStatement>(std::move(*result))
                          : skstd::optional<DSLStatement>();
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
skstd::optional<DSLType> DSLParser::type(const DSLModifiers& modifiers) {
    Token type;
    if (!this->expect(Token::Kind::TK_IDENTIFIER, "a type", &type)) {
        return skstd::nullopt;
    }
    if (!IsType(this->text(type))) {
        this->error(type, ("no type named '" + this->text(type) + "'").c_str());
        return skstd::nullopt;
    }
    DSLType result(this->text(type), modifiers);
    while (this->checkNext(Token::Kind::TK_LBRACKET)) {
        if (result.isArray()) {
            this->error(this->peek(), "multi-dimensional arrays are not supported");
            return skstd::nullopt;
        }
        if (this->peek().fKind != Token::Kind::TK_RBRACKET) {
            SKSL_INT i;
            if (this->intLiteral(&i)) {
                result = Array(result,  i);
            } else {
                return skstd::nullopt;
            }
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
        DSLModifiers modifiers = this->modifiers();
        skstd::optional<dsl::DSLType> type = this->type(modifiers);
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
                    SKSL_INT arraySize = 0;
                    if (this->DSLParser::intLiteral(&arraySize)) {
                        if (arraySize <= 0) {
                            this->error(sizeToken, "array size must be positive");
                        }
                        actualType = Array(std::move(actualType), arraySize);
                    } else {
                        return false;
                    }
                } else {
                    this->error(sizeToken, "unsized arrays are not permitted");
                }
                this->expect(Token::Kind::TK_RBRACKET, "']'");
            }
            if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
                return false;
            }
            fields.push_back(dsl::Field(modifiers, std::move(actualType), this->text(fieldName)));
        }
        while (this->checkNext(Token::Kind::TK_COMMA));
    }
    if (fields.empty()) {
        this->error(typeName, "interface block '" + this->text(typeName) +
                          "' must contain at least one member");
        return false;
    }
    skstd::string_view instanceName;
    Token instanceNameToken;
    SKSL_INT arraySize = 0;
    if (this->checkNext(Token::Kind::TK_IDENTIFIER, &instanceNameToken)) {
        instanceName = this->text(instanceNameToken);
        if (this->checkNext(Token::Kind::TK_LBRACKET)) {
            Token sizeToken = this->peek();
            if (sizeToken.fKind != Token::Kind::TK_RBRACKET) {
                if (this->DSLParser::intLiteral(&arraySize)) {
                    if (arraySize <= 0) {
                        this->error(sizeToken, "array size must be positive");
                    }
                } else {
                    return false;
                }
            } else {
                this->error(sizeToken, "unsized arrays are not permitted");
                return false;
            }
            this->expect(Token::Kind::TK_RBRACKET, "']'");
        }
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return false;
    }
    dsl::InterfaceBlock(modifiers, this->text(typeName), std::move(fields), instanceName,
                        arraySize);
    return true;
}

/* IF LPAREN expression RPAREN statement (ELSE statement)? */
skstd::optional<DSLStatement> DSLParser::ifStatement() {
    Token start;
    bool isStatic = this->checkNext(Token::Kind::TK_STATIC_IF, &start);
    if (!isStatic && !this->expect(Token::Kind::TK_IF, "'if'", &start)) {
        return skstd::nullopt;
    }
    if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
        return skstd::nullopt;
    }
    skstd::optional<DSLWrapper<DSLExpression>> test = this->expression();
    if (!test) {
        return skstd::nullopt;
    }
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
        return skstd::nullopt;
    }
    skstd::optional<DSLStatement> ifTrue = this->statement();
    if (!ifTrue) {
        return skstd::nullopt;
    }
    skstd::optional<DSLStatement> ifFalse;
    if (this->checkNext(Token::Kind::TK_ELSE)) {
        ifFalse = this->statement();
        if (!ifFalse) {
            return skstd::nullopt;
        }
    }
    if (isStatic) {
        return StaticIf(std::move(**test), std::move(*ifTrue),
                        ifFalse ? std::move(*ifFalse) : DSLStatement());
    } else {
        return If(std::move(**test), std::move(*ifTrue),
                  ifFalse ? std::move(*ifFalse) : DSLStatement());
    }
}

/* DO statement WHILE LPAREN expression RPAREN SEMICOLON */
skstd::optional<DSLStatement> DSLParser::doStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_DO, "'do'", &start)) {
        return skstd::nullopt;
    }
    skstd::optional<DSLStatement> statement = this->statement();
    if (!statement) {
        return skstd::nullopt;
    }
    if (!this->expect(Token::Kind::TK_WHILE, "'while'")) {
        return skstd::nullopt;
    }
    if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
        return skstd::nullopt;
    }
    skstd::optional<DSLWrapper<DSLExpression>> test = this->expression();
    if (!test) {
        return skstd::nullopt;
    }
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
        return skstd::nullopt;
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return skstd::nullopt;
    }
    return Do(std::move(*statement), std::move(**test));
}

/* WHILE LPAREN expression RPAREN STATEMENT */
skstd::optional<DSLStatement> DSLParser::whileStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_WHILE, "'while'", &start)) {
        return skstd::nullopt;
    }
    if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
        return skstd::nullopt;
    }
    skstd::optional<DSLWrapper<DSLExpression>> test = this->expression();
    if (!test) {
        return skstd::nullopt;
    }
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
        return skstd::nullopt;
    }
    skstd::optional<DSLStatement> statement = this->statement();
    if (!statement) {
        return skstd::nullopt;
    }
    return While(std::move(**test), std::move(*statement));
}

/* CASE expression COLON statement* */
skstd::optional<DSLCase> DSLParser::switchCase() {
    Token start;
    if (!this->expect(Token::Kind::TK_CASE, "'case'", &start)) {
        return skstd::nullopt;
    }
    skstd::optional<DSLWrapper<DSLExpression>> value = this->expression();
    if (!value) {
        return skstd::nullopt;
    }
    if (!this->expect(Token::Kind::TK_COLON, "':'")) {
        return skstd::nullopt;
    }
    SkTArray<DSLStatement> statements;
    while (this->peek().fKind != Token::Kind::TK_RBRACE &&
           this->peek().fKind != Token::Kind::TK_CASE &&
           this->peek().fKind != Token::Kind::TK_DEFAULT) {
        skstd::optional<DSLStatement> s = this->statement();
        if (!s) {
            return skstd::nullopt;
        }
        statements.push_back(std::move(*s));
    }
    return DSLCase(std::move(**value), std::move(statements));
}

/* SWITCH LPAREN expression RPAREN LBRACE switchCase* (DEFAULT COLON statement*)? RBRACE */
skstd::optional<DSLStatement> DSLParser::switchStatement() {
    Token start;
    bool isStatic = this->checkNext(Token::Kind::TK_STATIC_SWITCH, &start);
    if (!isStatic && !this->expect(Token::Kind::TK_SWITCH, "'switch'", &start)) {
        return skstd::nullopt;
    }
    if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
        return skstd::nullopt;
    }
    skstd::optional<DSLWrapper<DSLExpression>> value = this->expression();
    if (!value) {
        return skstd::nullopt;
    }
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
        return skstd::nullopt;
    }
    if (!this->expect(Token::Kind::TK_LBRACE, "'{'")) {
        return skstd::nullopt;
    }
    SkTArray<DSLCase> cases;
    while (this->peek().fKind == Token::Kind::TK_CASE) {
        skstd::optional<DSLCase> c = this->switchCase();
        if (!c) {
            return skstd::nullopt;
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
            return skstd::nullopt;
        }
        while (this->peek().fKind != Token::Kind::TK_RBRACE) {
            skstd::optional<DSLStatement> s = this->statement();
            if (!s) {
                return skstd::nullopt;
            }
            statements.push_back(std::move(*s));
        }
        cases.push_back(DSLCase(DSLExpression(), std::move(statements)));
    }
    if (!this->expect(Token::Kind::TK_RBRACE, "'}'")) {
        return skstd::nullopt;
    }
    if (isStatic) {
        return StaticSwitch(std::move(**value), std::move(cases));
    } else {
        return Switch(std::move(**value), std::move(cases));
    }
}

/* FOR LPAREN (declaration | expression)? SEMICOLON expression? SEMICOLON expression? RPAREN
   STATEMENT */
skstd::optional<dsl::DSLStatement> DSLParser::forStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_FOR, "'for'", &start)) {
        return skstd::nullopt;
    }
    if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
        return skstd::nullopt;
    }
    AutoDSLSymbolTable symbols;
    skstd::optional<dsl::DSLStatement> initializer;
    Token nextToken = this->peek();
    if (nextToken.fKind == Token::Kind::TK_SEMICOLON) {
        // An empty init-statement.
        this->nextToken();
    } else {
        // The init-statement must be an expression or variable declaration.
        initializer = this->varDeclarationsOrExpressionStatement();
        if (!initializer) {
            return skstd::nullopt;
        }
    }
    skstd::optional<DSLWrapper<DSLExpression>> test;
    if (this->peek().fKind != Token::Kind::TK_SEMICOLON) {
        test = this->expression();
        if (!test) {
            return skstd::nullopt;
        }
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return skstd::nullopt;
    }
    skstd::optional<DSLWrapper<DSLExpression>> next;
    if (this->peek().fKind != Token::Kind::TK_RPAREN) {
        next = this->expression();
        if (!next) {
            return skstd::nullopt;
        }
    }
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
        return skstd::nullopt;
    }
    skstd::optional<dsl::DSLStatement> statement = this->statement();
    if (!statement) {
        return skstd::nullopt;
    }
    return For(initializer ? std::move(*initializer) : DSLStatement(),
               test ? std::move(**test) : DSLExpression(),
               next ? std::move(**next) : DSLExpression(),
               std::move(*statement));
}

/* RETURN expression? SEMICOLON */
skstd::optional<DSLStatement> DSLParser::returnStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_RETURN, "'return'", &start)) {
        return skstd::nullopt;
    }
    skstd::optional<DSLWrapper<DSLExpression>> expression;
    if (this->peek().fKind != Token::Kind::TK_SEMICOLON) {
        expression = this->expression();
        if (!expression) {
            return skstd::nullopt;
        }
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return skstd::nullopt;
    }
    return Return(expression ? std::move(**expression) : DSLExpression());
}

/* BREAK SEMICOLON */
skstd::optional<DSLStatement> DSLParser::breakStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_BREAK, "'break'", &start)) {
        return skstd::nullopt;
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return skstd::nullopt;
    }
    return Break();
}

/* CONTINUE SEMICOLON */
skstd::optional<DSLStatement> DSLParser::continueStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_CONTINUE, "'continue'", &start)) {
        return skstd::nullopt;
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return skstd::nullopt;
    }
    return Continue();
}

/* DISCARD SEMICOLON */
skstd::optional<DSLStatement> DSLParser::discardStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_DISCARD, "'continue'", &start)) {
        return skstd::nullopt;
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return skstd::nullopt;
    }
    return Discard();
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
    SkTArray<DSLStatement> statements;
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_RBRACE:
                this->nextToken();
                return DSLBlock(std::move(statements), CurrentSymbolTable());
            case Token::Kind::TK_END_OF_FILE:
                this->error(this->peek(), "expected '}', but found end of file");
                return skstd::nullopt;
            default: {
                skstd::optional<DSLStatement> statement = this->statement();
                if (!statement) {
                    return skstd::nullopt;
                }
                statements.push_back(std::move(*statement));
            }
        }
    }
}

/* expression SEMICOLON */
skstd::optional<DSLStatement> DSLParser::expressionStatement() {
    skstd::optional<DSLWrapper<DSLExpression>> expr = this->expression();
    if (expr) {
        if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
            return skstd::nullopt;
        }
        return {{DSLStatement(std::move(**expr))}};
    }
    return skstd::nullopt;
}

/* assignmentExpression (COMMA assignmentExpression)* */
skstd::optional<DSLWrapper<DSLExpression>> DSLParser::expression() {
    skstd::optional<DSLWrapper<DSLExpression>> result = this->assignmentExpression();
    if (!result) {
        return skstd::nullopt;
    }
    Token t;
    AutoDSLDepth depth(this);
    while (this->checkNext(Token::Kind::TK_COMMA, &t)) {
        if (!depth.increase()) {
            return skstd::nullopt;
        }
        skstd::optional<DSLWrapper<DSLExpression>> right = this->assignmentExpression();
        if (!right) {
            return skstd::nullopt;
        }
        result = skstd::optional<DSLWrapper<DSLExpression>>(dsl::operator,(std::move(**result),
                                                                           std::move(**right)));
    }
    return result;
}

#define OPERATOR_RIGHT(op, exprType)                                         \
    do {                                                                     \
        this->nextToken();                                                   \
        if (!depth.increase()) {                                             \
            return skstd::nullopt;                                           \
        }                                                                    \
        skstd::optional<DSLWrapper<DSLExpression>> right = this->exprType(); \
        if (!right) {                                                        \
            return skstd::nullopt;                                           \
        }                                                                    \
        result = {{std::move(**result) op std::move(**right)}};              \
    } while (false)

/* ternaryExpression ((EQEQ | STAREQ | SLASHEQ | PERCENTEQ | PLUSEQ | MINUSEQ | SHLEQ | SHREQ |
   BITWISEANDEQ | BITWISEXOREQ | BITWISEOREQ | LOGICALANDEQ | LOGICALXOREQ | LOGICALOREQ)
   assignmentExpression)*
 */
skstd::optional<DSLWrapper<DSLExpression>> DSLParser::assignmentExpression() {
    AutoDSLDepth depth(this);
    skstd::optional<DSLWrapper<DSLExpression>> result = this->ternaryExpression();
    if (!result) {
        return skstd::nullopt;
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
skstd::optional<DSLWrapper<DSLExpression>> DSLParser::ternaryExpression() {
    AutoDSLDepth depth(this);
    skstd::optional<DSLWrapper<DSLExpression>> base = this->logicalOrExpression();
    if (!base) {
        return skstd::nullopt;
    }
    if (this->checkNext(Token::Kind::TK_QUESTION)) {
        if (!depth.increase()) {
            return skstd::nullopt;
        }
        skstd::optional<DSLWrapper<DSLExpression>> trueExpr = this->expression();
        if (!trueExpr) {
            return skstd::nullopt;
        }
        if (this->expect(Token::Kind::TK_COLON, "':'")) {
            skstd::optional<DSLWrapper<DSLExpression>> falseExpr = this->assignmentExpression();
            if (!falseExpr) {
                return skstd::nullopt;
            }
            return Select(std::move(**base), std::move(**trueExpr), std::move(**falseExpr));
        }
        return skstd::nullopt;
    }
    return base;
}

/* logicalXorExpression (LOGICALOR logicalXorExpression)* */
skstd::optional<DSLWrapper<DSLExpression>> DSLParser::logicalOrExpression() {
    AutoDSLDepth depth(this);
    skstd::optional<DSLWrapper<DSLExpression>> result = this->logicalXorExpression();
    if (!result) {
        return skstd::nullopt;
    }
    while (this->peek().fKind == Token::Kind::TK_LOGICALOR) {
        OPERATOR_RIGHT(||, logicalXorExpression);
    }
    return result;
}

/* logicalAndExpression (LOGICALXOR logicalAndExpression)* */
skstd::optional<DSLWrapper<DSLExpression>> DSLParser::logicalXorExpression() {
    AutoDSLDepth depth(this);
    skstd::optional<DSLWrapper<DSLExpression>> result = this->logicalAndExpression();
    if (!result) {
        return skstd::nullopt;
    }
    while (this->checkNext(Token::Kind::TK_LOGICALXOR)) {
        if (!depth.increase()) {
            return skstd::nullopt;
        }
        skstd::optional<DSLWrapper<DSLExpression>> right = this->logicalAndExpression();
        if (!right) {
            return skstd::nullopt;
        }
        result = {{LogicalXor(std::move(**result), std::move(**right))}};
    }
    return result;
}

/* bitwiseOrExpression (LOGICALAND bitwiseOrExpression)* */
skstd::optional<DSLWrapper<DSLExpression>> DSLParser::logicalAndExpression() {
    AutoDSLDepth depth(this);
    skstd::optional<DSLWrapper<DSLExpression>> result = this->bitwiseOrExpression();
    if (!result) {
        return skstd::nullopt;
    }
    while (this->peek().fKind == Token::Kind::TK_LOGICALAND) {
        OPERATOR_RIGHT(&&, bitwiseOrExpression);
    }
    return result;
}

/* bitwiseXorExpression (BITWISEOR bitwiseXorExpression)* */
skstd::optional<DSLWrapper<DSLExpression>> DSLParser::bitwiseOrExpression() {
    AutoDSLDepth depth(this);
    skstd::optional<DSLWrapper<DSLExpression>> result = this->bitwiseXorExpression();
    if (!result) {
        return skstd::nullopt;
    }
    while (this->peek().fKind == Token::Kind::TK_BITWISEOR) {
        OPERATOR_RIGHT(|, bitwiseXorExpression);
    }
    return result;
}

/* bitwiseAndExpression (BITWISEXOR bitwiseAndExpression)* */
skstd::optional<DSLWrapper<DSLExpression>> DSLParser::bitwiseXorExpression() {
    AutoDSLDepth depth(this);
    skstd::optional<DSLWrapper<DSLExpression>> result = this->bitwiseAndExpression();
    if (!result) {
        return skstd::nullopt;
    }
    while (this->peek().fKind == Token::Kind::TK_BITWISEXOR) {
        OPERATOR_RIGHT(^, bitwiseAndExpression);
    }
    return result;
}

/* equalityExpression (BITWISEAND equalityExpression)* */
skstd::optional<DSLWrapper<DSLExpression>> DSLParser::bitwiseAndExpression() {
    AutoDSLDepth depth(this);
    skstd::optional<DSLWrapper<DSLExpression>> result = this->equalityExpression();
    if (!result) {
        return skstd::nullopt;
    }
    while (this->peek().fKind == Token::Kind::TK_BITWISEAND) {
        OPERATOR_RIGHT(&, equalityExpression);
    }
    return result;
}

/* relationalExpression ((EQEQ | NEQ) relationalExpression)* */
skstd::optional<DSLWrapper<DSLExpression>> DSLParser::equalityExpression() {
    AutoDSLDepth depth(this);
    skstd::optional<DSLWrapper<DSLExpression>> result = this->relationalExpression();
    if (!result) {
        return skstd::nullopt;
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
skstd::optional<DSLWrapper<DSLExpression>> DSLParser::relationalExpression() {
    AutoDSLDepth depth(this);
    skstd::optional<DSLWrapper<DSLExpression>> result = this->shiftExpression();
    if (!result) {
        return skstd::nullopt;
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
skstd::optional<DSLWrapper<DSLExpression>> DSLParser::shiftExpression() {
    AutoDSLDepth depth(this);
    skstd::optional<DSLWrapper<DSLExpression>> result = this->additiveExpression();
    if (!result) {
        return skstd::nullopt;
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
skstd::optional<DSLWrapper<DSLExpression>> DSLParser::additiveExpression() {
    AutoDSLDepth depth(this);
    skstd::optional<DSLWrapper<DSLExpression>> result = this->multiplicativeExpression();
    if (!result) {
        return skstd::nullopt;
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
skstd::optional<DSLWrapper<DSLExpression>> DSLParser::multiplicativeExpression() {
    AutoDSLDepth depth(this);
    skstd::optional<DSLWrapper<DSLExpression>> result = this->unaryExpression();
    if (!result) {
        return skstd::nullopt;
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
skstd::optional<DSLWrapper<DSLExpression>> DSLParser::unaryExpression() {
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
                return skstd::nullopt;
            }
            this->nextToken();
            skstd::optional<DSLWrapper<DSLExpression>> expr = this->unaryExpression();
            if (!expr) {
                return skstd::nullopt;
            }
            switch (next.fKind) {
                case Token::Kind::TK_PLUS:       return {{ +std::move(**expr)}};
                case Token::Kind::TK_MINUS:      return {{ -std::move(**expr)}};
                case Token::Kind::TK_LOGICALNOT: return {{ !std::move(**expr)}};
                case Token::Kind::TK_BITWISENOT: return {{ ~std::move(**expr)}};
                case Token::Kind::TK_PLUSPLUS:   return {{++std::move(**expr)}};
                case Token::Kind::TK_MINUSMINUS: return {{--std::move(**expr)}};
                default: SkUNREACHABLE;
            }
        }
        default:
            return this->postfixExpression();
    }
}

/* term suffix* */
skstd::optional<DSLWrapper<DSLExpression>> DSLParser::postfixExpression() {
    AutoDSLDepth depth(this);
    skstd::optional<DSLWrapper<DSLExpression>> result = this->term();
    if (!result) {
        return skstd::nullopt;
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
            case Token::Kind::TK_MINUSMINUS:
                if (!depth.increase()) {
                    return skstd::nullopt;
                }
                result = this->suffix(std::move(**result));
                if (!result) {
                    return skstd::nullopt;
                }
                break;
            default:
                return result;
        }
    }
}

skstd::optional<DSLWrapper<DSLExpression>> DSLParser::swizzle(int offset, DSLExpression base,
                                                              skstd::string_view swizzleMask) {
    SkASSERT(swizzleMask.length() > 0);
    if (!base.type().isVector() && !base.type().isScalar()) {
        return base.field(swizzleMask);
    }
    int length = swizzleMask.length();
    if (length > 4) {
        this->error(offset, "too many components in swizzle mask");
        return skstd::nullopt;
    }
    SkSL::SwizzleComponent::Type components[4];
    for (int i = 0; i < length; ++i) {
        switch (swizzleMask[i]) {
            case '0': components[i] = SwizzleComponent::ZERO; break;
            case '1': components[i] = SwizzleComponent::ONE;  break;
            case 'r':
            case 'x':
            case 's':
            case 'L': components[i] = SwizzleComponent::R;    break;
            case 'g':
            case 'y':
            case 't':
            case 'T': components[i] = SwizzleComponent::G;    break;
            case 'b':
            case 'z':
            case 'p':
            case 'R': components[i] = SwizzleComponent::B;    break;
            case 'a':
            case 'w':
            case 'q':
            case 'B': components[i] = SwizzleComponent::A;    break;
            default:
                this->error(offset,
                        String::printf("invalid swizzle component '%c'", swizzleMask[i]).c_str());
                return skstd::nullopt;
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

skstd::optional<dsl::Wrapper<dsl::DSLExpression>> DSLParser::call(int offset,
        dsl::DSLExpression base, SkTArray<Wrapper<DSLExpression>> args) {
    return {{base(std::move(args))}};
}

/* LBRACKET expression? RBRACKET | DOT IDENTIFIER | LPAREN arguments RPAREN |
   PLUSPLUS | MINUSMINUS | COLONCOLON IDENTIFIER | FLOAT_LITERAL [IDENTIFIER] */
skstd::optional<DSLWrapper<DSLExpression>> DSLParser::suffix(DSLExpression base) {
    Token next = this->nextToken();
    AutoDSLDepth depth(this);
    if (!depth.increase()) {
        return skstd::nullopt;
    }
    switch (next.fKind) {
        case Token::Kind::TK_LBRACKET: {
            skstd::optional<DSLWrapper<DSLExpression>> index = this->expression();
            if (!index) {
                return skstd::nullopt;
            }
            this->expect(Token::Kind::TK_RBRACKET, "']' to complete array access expression");
            DSLPossibleExpression result = base[std::move(**index)];
            if (result.valid()) {
                return {{std::move(result)}};
            }
            result.reportErrors(PositionInfo());
            return skstd::nullopt;
        }
        case Token::Kind::TK_DOT: {
            int offset = this->peek().fOffset;
            skstd::string_view text;
            if (this->identifier(&text)) {
                return this->swizzle(offset, std::move(base), text);
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
                return this->swizzle(next.fOffset, std::move(base), field + this->text(id));
            }
            this->pushback(id);
            return this->swizzle(next.fOffset, std::move(base), field);
        }
        case Token::Kind::TK_LPAREN: {
            SkTArray<Wrapper<DSLExpression>> args;
            if (this->peek().fKind != Token::Kind::TK_RPAREN) {
                for (;;) {
                    skstd::optional<DSLWrapper<DSLExpression>> expr = this->assignmentExpression();
                    if (!expr) {
                        return skstd::nullopt;
                    }
                    args.push_back(std::move(*expr));
                    if (!this->checkNext(Token::Kind::TK_COMMA)) {
                        break;
                    }
                }
            }
            this->expect(Token::Kind::TK_RPAREN, "')' to complete function arguments");
            return this->call(next.fOffset, std::move(base), std::move(args));
        }
        case Token::Kind::TK_PLUSPLUS:
            return {{std::move(base)++}};
        case Token::Kind::TK_MINUSMINUS: {
            return {{std::move(base)--}};
        }
        default: {
            this->error(next,  "expected expression suffix, but found '" + this->text(next) + "'");
            return skstd::nullopt;
        }
    }
}

/* IDENTIFIER | intLiteral | floatLiteral | boolLiteral | '(' expression ')' */
skstd::optional<DSLWrapper<DSLExpression>> DSLParser::term() {
    Token t = this->peek();
    switch (t.fKind) {
        case Token::Kind::TK_IDENTIFIER: {
            skstd::string_view text;
            if (this->identifier(&text)) {
                return dsl::Symbol(text);
            }
            break;
        }
        case Token::Kind::TK_INT_LITERAL: {
            SKSL_INT i;
            if (this->intLiteral(&i)) {
                return {{i}};
            }
            break;
        }
        case Token::Kind::TK_FLOAT_LITERAL: {
            SKSL_FLOAT f;
            if (this->floatLiteral(&f)) {
                return {{f}};
            }
            break;
        }
        case Token::Kind::TK_TRUE_LITERAL: // fall through
        case Token::Kind::TK_FALSE_LITERAL: {
            bool b;
            if (this->boolLiteral(&b)) {
                return {{b}};
            }
            break;
        }
        case Token::Kind::TK_LPAREN: {
            this->nextToken();
            AutoDSLDepth depth(this);
            if (!depth.increase()) {
                return skstd::nullopt;
            }
            skstd::optional<DSLWrapper<DSLExpression>> result = this->expression();
            if (result) {
                this->expect(Token::Kind::TK_RPAREN, "')' to complete expression");
                return result;
            }
            break;
        }
        default:
            this->nextToken();
            this->error(t.fOffset, "expected expression, but found '" + this->text(t) + "'");
    }
    return skstd::nullopt;
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

#endif // SKSL_DSL_PARSER
