/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLParser.h"

#include <memory>
#include "stdio.h"

#include "include/private/SkSLModifiers.h"
#include "src/sksl/SkSLASTNode.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"

#ifndef SKSL_STANDALONE
#include "include/private/SkOnce.h"
#endif

namespace SkSL {

static constexpr int kMaxParseDepth = 50;
static constexpr int kMaxStructDepth = 8;

static bool struct_is_too_deeply_nested(const Type& type, int limit) {
    if (limit < 0) {
        return true;
    }

    if (type.isStruct()) {
        for (const Type::Field& f : type.fields()) {
            if (struct_is_too_deeply_nested(*f.fType, limit - 1)) {
                return true;
            }
        }
    }

    return false;
}

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
        case Token::Kind::TK_VARYING:        return Modifiers::kVarying_Flag;
        case Token::Kind::TK_INLINE:         return Modifiers::kInline_Flag;
        case Token::Kind::TK_NOINLINE:       return Modifiers::kNoInline_Flag;
        default:                             return 0;
    }
}

class AutoDepth {
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
            fParser->error(fParser->peek(), String("exceeded max parse depth"));
            return false;
        }
        return true;
    }

private:
    Parser* fParser;
    int fDepth;
};

std::unordered_map<String, Parser::LayoutToken>* Parser::layoutTokens;

void Parser::InitLayoutMap() {
    layoutTokens = new std::unordered_map<String, LayoutToken>;
    #define TOKEN(name, text) (*layoutTokens)[text] = LayoutToken::name
    TOKEN(LOCATION,                     "location");
    TOKEN(OFFSET,                       "offset");
    TOKEN(BINDING,                      "binding");
    TOKEN(INDEX,                        "index");
    TOKEN(SET,                          "set");
    TOKEN(BUILTIN,                      "builtin");
    TOKEN(INPUT_ATTACHMENT_INDEX,       "input_attachment_index");
    TOKEN(ORIGIN_UPPER_LEFT,            "origin_upper_left");
    TOKEN(OVERRIDE_COVERAGE,            "override_coverage");
    TOKEN(EARLY_FRAGMENT_TESTS,         "early_fragment_tests");
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
    TOKEN(MARKER,                       "marker");
    TOKEN(WHEN,                         "when");
    TOKEN(KEY,                          "key");
    TOKEN(TRACKED,                      "tracked");
    TOKEN(SRGB_UNPREMUL,                "srgb_unpremul");
    TOKEN(CTYPE,                        "ctype");
    TOKEN(SKPMCOLOR4F,                  "SkPMColor4f");
    TOKEN(SKV4,                         "SkV4");
    TOKEN(SKRECT,                       "SkRect");
    TOKEN(SKIRECT,                      "SkIRect");
    TOKEN(SKPMCOLOR,                    "SkPMColor");
    TOKEN(SKM44,                        "SkM44");
    TOKEN(BOOL,                         "bool");
    TOKEN(INT,                          "int");
    TOKEN(FLOAT,                        "float");
    #undef TOKEN
}

Parser::Parser(const char* text, size_t length, SymbolTable& symbols, ErrorReporter& errors)
: fText(text)
, fPushback(Token::Kind::TK_NONE, -1, -1)
, fSymbols(symbols)
, fErrors(errors) {
    fLexer.start(text, length);
    static const bool layoutMapInitialized = []{ return (void)InitLayoutMap(), true; }();
    (void) layoutMapInitialized;
}

template <typename... Args>
ASTNode::ID Parser::createNode(Args&&... args) {
    ASTNode::ID result(fFile->fNodes.size());
    fFile->fNodes.emplace_back(&fFile->fNodes, std::forward<Args>(args)...);
    return result;
}

ASTNode::ID Parser::addChild(ASTNode::ID target, ASTNode::ID child) {
    fFile->fNodes[target.fValue].addChild(child);
    return child;
}

void Parser::createEmptyChild(ASTNode::ID target) {
    ASTNode::ID child(fFile->fNodes.size());
    fFile->fNodes.emplace_back();
    fFile->fNodes[target.fValue].addChild(child);
}

/* (directive | section | declaration)* END_OF_FILE */
std::unique_ptr<ASTFile> Parser::compilationUnit() {
    fFile = std::make_unique<ASTFile>();
    ASTNode::ID result = this->createNode(/*offset=*/0, ASTNode::Kind::kFile);
    fFile->fRoot = result;
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_END_OF_FILE:
                return std::move(fFile);
            case Token::Kind::TK_DIRECTIVE: {
                ASTNode::ID dir = this->directive();
                if (fErrors.errorCount()) {
                    return nullptr;
                }
                if (dir) {
                    getNode(result).addChild(dir);
                }
                break;
            }
            case Token::Kind::TK_SECTION: {
                ASTNode::ID section = this->section();
                if (fErrors.errorCount()) {
                    return nullptr;
                }
                if (section) {
                    getNode(result).addChild(section);
                }
                break;
            }
            case Token::Kind::TK_INVALID: {
                this->error(this->peek(), String("invalid token"));
                return nullptr;
            }
            default: {
                ASTNode::ID decl = this->declaration();
                if (fErrors.errorCount()) {
                    return nullptr;
                }
                if (decl) {
                    getNode(result).addChild(decl);
                }
            }
        }
    }
    return std::move(fFile);
}

Token Parser::nextRawToken() {
    if (fPushback.fKind != Token::Kind::TK_NONE) {
        Token result = fPushback;
        fPushback.fKind = Token::Kind::TK_NONE;
        return result;
    }
    Token result = fLexer.next();
    return result;
}

Token Parser::nextToken() {
    Token token = this->nextRawToken();
    while (token.fKind == Token::Kind::TK_WHITESPACE ||
           token.fKind == Token::Kind::TK_LINE_COMMENT ||
           token.fKind == Token::Kind::TK_BLOCK_COMMENT) {
        token = this->nextRawToken();
    }
    return token;
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
        this->error(next, "expected " + String(expected) + ", but found '" +
                    this->text(next) + "'");
        return false;
    }
}

bool Parser::expectIdentifier(Token* result) {
    if (!this->expect(Token::Kind::TK_IDENTIFIER, "an identifier", result)) {
        return false;
    }
    if (this->isType(this->text(*result))) {
        this->error(*result, "expected an identifier, but found type '" +
                             this->text(*result) + "'");
        return false;
    }
    return true;
}

StringFragment Parser::text(Token token) {
    return StringFragment(fText + token.fOffset, token.fLength);
}

void Parser::error(Token token, String msg) {
    this->error(token.fOffset, msg);
}

void Parser::error(int offset, String msg) {
    fErrors.error(offset, msg);
}

bool Parser::isType(StringFragment name) {
    const Symbol* s = fSymbols[name];
    return s && s->is<Type>();
}

bool Parser::isArrayType(ASTNode::ID type) {
    const ASTNode& node = this->getNode(type);
    SkASSERT(node.fKind == ASTNode::Kind::kType);
    return node.begin() != node.end();
}

/* DIRECTIVE(#version) INT_LITERAL ("es" | "compatibility")? |
   DIRECTIVE(#extension) IDENTIFIER COLON IDENTIFIER */
ASTNode::ID Parser::directive() {
    Token start;
    if (!this->expect(Token::Kind::TK_DIRECTIVE, "a directive", &start)) {
        return ASTNode::ID::Invalid();
    }
    StringFragment text = this->text(start);
    if (text == "#extension") {
        Token name;
        if (!this->expectIdentifier(&name)) {
            return ASTNode::ID::Invalid();
        }
        if (!this->expect(Token::Kind::TK_COLON, "':'")) {
            return ASTNode::ID::Invalid();
        }
        // FIXME: need to start paying attention to this token
        if (!this->expect(Token::Kind::TK_IDENTIFIER, "an identifier")) {
            return ASTNode::ID::Invalid();
        }
        return this->createNode(start.fOffset, ASTNode::Kind::kExtension, this->text(name));
    } else {
        this->error(start, "unsupported directive '" + this->text(start) + "'");
        return ASTNode::ID::Invalid();
    }
}

/* SECTION LBRACE (LPAREN IDENTIFIER RPAREN)? <any sequence of tokens with balanced braces>
   RBRACE */
ASTNode::ID Parser::section() {
    Token start;
    if (!this->expect(Token::Kind::TK_SECTION, "a section token", &start)) {
        return ASTNode::ID::Invalid();
    }
    StringFragment argument;
    if (this->peek().fKind == Token::Kind::TK_LPAREN) {
        this->nextToken();
        Token argToken;
        if (!this->expectIdentifier(&argToken)) {
            return ASTNode::ID::Invalid();
        }
        argument = this->text(argToken);
        if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
            return ASTNode::ID::Invalid();
        }
    }
    if (!this->expect(Token::Kind::TK_LBRACE, "'{'")) {
        return ASTNode::ID::Invalid();
    }
    StringFragment text;
    Token codeStart = this->nextRawToken();
    size_t startOffset = codeStart.fOffset;
    this->pushback(codeStart);
    text.fChars = fText + startOffset;
    int level = 1;
    for (;;) {
        Token next = this->nextRawToken();
        switch (next.fKind) {
            case Token::Kind::TK_LBRACE:
                ++level;
                break;
            case Token::Kind::TK_RBRACE:
                --level;
                break;
            case Token::Kind::TK_END_OF_FILE:
                this->error(start, "reached end of file while parsing section");
                return ASTNode::ID::Invalid();
            default:
                break;
        }
        if (!level) {
            text.fLength = next.fOffset - startOffset;
            break;
        }
    }
    StringFragment name = this->text(start);
    ++name.fChars;
    --name.fLength;
    return this->createNode(start.fOffset, ASTNode::Kind::kSection,
                            ASTNode::SectionData(name, argument, text));
}

/* ENUM CLASS IDENTIFIER LBRACE (IDENTIFIER (EQ expression)? (COMMA IDENTIFIER (EQ expression))*)?
   RBRACE */
ASTNode::ID Parser::enumDeclaration() {
    Token start;
    if (!this->expect(Token::Kind::TK_ENUM, "'enum'", &start)) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::Kind::TK_CLASS, "'class'")) {
        return ASTNode::ID::Invalid();
    }
    Token name;
    if (!this->expectIdentifier(&name)) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::Kind::TK_LBRACE, "'{'")) {
        return ASTNode::ID::Invalid();
    }
    fSymbols.add(Type::MakeEnumType(this->text(name)));
    ASTNode::ID result = this->createNode(name.fOffset, ASTNode::Kind::kEnum, this->text(name));
    if (!this->checkNext(Token::Kind::TK_RBRACE)) {
        Token id;
        if (!this->expectIdentifier(&id)) {
            return ASTNode::ID::Invalid();
        }
        if (this->checkNext(Token::Kind::TK_EQ)) {
            ASTNode::ID value = this->assignmentExpression();
            if (!value) {
                return ASTNode::ID::Invalid();
            }
            ASTNode::ID child = this->addChild(
                    result, this->createNode(id.fOffset, ASTNode::Kind::kEnumCase, this->text(id)));
            getNode(child).addChild(value);
        } else {
            this->addChild(result,
                           this->createNode(id.fOffset, ASTNode::Kind::kEnumCase, this->text(id)));
        }
        while (!this->checkNext(Token::Kind::TK_RBRACE)) {
            if (!this->expect(Token::Kind::TK_COMMA, "','")) {
                return ASTNode::ID::Invalid();
            }
            if (!this->expectIdentifier(&id)) {
                return ASTNode::ID::Invalid();
            }
            if (this->checkNext(Token::Kind::TK_EQ)) {
                ASTNode::ID value = this->assignmentExpression();
                if (!value) {
                    return ASTNode::ID::Invalid();
                }
                ASTNode::ID child = this->addChild(
                        result,
                        this->createNode(id.fOffset, ASTNode::Kind::kEnumCase, this->text(id)));
                getNode(child).addChild(value);
            } else {
                this->addChild(
                        result,
                        this->createNode(id.fOffset, ASTNode::Kind::kEnumCase, this->text(id)));
            }
        }
    }
    this->expect(Token::Kind::TK_SEMICOLON, "';'");
    return result;
}

/* enumDeclaration | modifiers (structVarDeclaration | type IDENTIFIER ((LPAREN parameter
   (COMMA parameter)* RPAREN (block | SEMICOLON)) | SEMICOLON) | interfaceBlock) */
ASTNode::ID Parser::declaration() {
    Token lookahead = this->peek();
    switch (lookahead.fKind) {
        case Token::Kind::TK_ENUM:
            return this->enumDeclaration();
        case Token::Kind::TK_SEMICOLON:
            this->error(lookahead.fOffset, "expected a declaration, but found ';'");
            return ASTNode::ID::Invalid();
        default:
            break;
    }
    Modifiers modifiers = this->modifiers();
    lookahead = this->peek();
    if (lookahead.fKind == Token::Kind::TK_IDENTIFIER && !this->isType(this->text(lookahead))) {
        // we have an identifier that's not a type, could be the start of an interface block
        return this->interfaceBlock(modifiers);
    }
    if (lookahead.fKind == Token::Kind::TK_STRUCT) {
        return this->structVarDeclaration(modifiers);
    }
    if (lookahead.fKind == Token::Kind::TK_SEMICOLON) {
        this->nextToken();
        return this->createNode(lookahead.fOffset, ASTNode::Kind::kModifiers, modifiers);
    }
    ASTNode::ID type = this->type();
    if (!type) {
        return ASTNode::ID::Invalid();
    }
    Token name;
    if (!this->expectIdentifier(&name)) {
        return ASTNode::ID::Invalid();
    }
    if (this->checkNext(Token::Kind::TK_LPAREN)) {
        ASTNode::ID result = this->createNode(name.fOffset, ASTNode::Kind::kFunction);
        ASTNode::FunctionData fd(modifiers, this->text(name), 0);
        getNode(result).addChild(type);
        if (this->peek().fKind != Token::Kind::TK_RPAREN) {
            for (;;) {
                ASTNode::ID parameter = this->parameter();
                if (!parameter) {
                    return ASTNode::ID::Invalid();
                }
                ++fd.fParameterCount;
                getNode(result).addChild(parameter);
                if (!this->checkNext(Token::Kind::TK_COMMA)) {
                    break;
                }
            }
        }
        getNode(result).setFunctionData(fd);
        if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
            return ASTNode::ID::Invalid();
        }
        ASTNode::ID body;
        if (!this->checkNext(Token::Kind::TK_SEMICOLON)) {
            body = this->block();
            if (!body) {
                return ASTNode::ID::Invalid();
            }
            getNode(result).addChild(body);
        }
        return result;
    } else {
        return this->varDeclarationEnd(modifiers, type, this->text(name));
    }
}

/* (varDeclarations | expressionStatement) */
ASTNode::ID Parser::varDeclarationsOrExpressionStatement() {
    if (this->isType(this->text(this->peek()))) {
        // Statements that begin with a typename are most often variable declarations, but
        // occasionally the type is part of a constructor, and these are actually expression-
        // statements in disguise. First, attempt the common case: parse it as a vardecl.
        Checkpoint checkpoint(this);
        VarDeclarationsPrefix prefix;
        if (this->varDeclarationsPrefix(&prefix)) {
            return this->varDeclarationEnd(prefix.modifiers, prefix.type, this->text(prefix.name));
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
    prefixData->modifiers = this->modifiers();
    prefixData->type = this->type();
    if (!prefixData->type) {
        return false;
    }
    return this->expectIdentifier(&prefixData->name);
}

/* modifiers type IDENTIFIER varDeclarationEnd */
ASTNode::ID Parser::varDeclarations() {
    VarDeclarationsPrefix prefix;
    if (!this->varDeclarationsPrefix(&prefix)) {
        return ASTNode::ID::Invalid();
    }
    return this->varDeclarationEnd(prefix.modifiers, prefix.type, this->text(prefix.name));
}

/* STRUCT IDENTIFIER LBRACE varDeclaration* RBRACE */
ASTNode::ID Parser::structDeclaration() {
    if (!this->expect(Token::Kind::TK_STRUCT, "'struct'")) {
        return ASTNode::ID::Invalid();
    }
    Token name;
    if (!this->expectIdentifier(&name)) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::Kind::TK_LBRACE, "'{'")) {
        return ASTNode::ID::Invalid();
    }
    std::vector<Type::Field> fields;
    while (this->peek().fKind != Token::Kind::TK_RBRACE) {
        ASTNode::ID decls = this->varDeclarations();
        if (!decls) {
            return ASTNode::ID::Invalid();
        }
        ASTNode& declsNode = getNode(decls);
        const Modifiers& modifiers = declsNode.begin()->getModifiers();
        if (modifiers.fFlags != Modifiers::kNo_Flag) {
            String desc = modifiers.description();
            desc.pop_back();  // remove trailing space
            this->error(declsNode.fOffset,
                        "modifier '" + desc + "' is not permitted on a struct field");
        }

        const Symbol* symbol = fSymbols[(declsNode.begin() + 1)->getString()];
        SkASSERT(symbol);
        const Type* type = &symbol->as<Type>();
        if (type->isOpaque()) {
            this->error(declsNode.fOffset,
                        "opaque type '" + type->name() + "' is not permitted in a struct");
        }

        for (auto iter = declsNode.begin() + 2; iter != declsNode.end(); ++iter) {
            ASTNode& var = *iter;
            const ASTNode::VarData& vd = var.getVarData();

            // Read array size if one is present.
            if (vd.fIsArray) {
                const ASTNode& size = *var.begin();
                if (!size || size.fKind != ASTNode::Kind::kInt) {
                    this->error(declsNode.fOffset, "array size in struct field must be a constant");
                    return ASTNode::ID::Invalid();
                }
                if (size.getInt() <= 0 || size.getInt() > INT_MAX) {
                    this->error(declsNode.fOffset, "array size is invalid");
                    return ASTNode::ID::Invalid();
                }
                // Add the array dimensions to our type.
                int arraySize = size.getInt();
                type = fSymbols.addArrayDimension(type, arraySize);
            }

            fields.push_back(Type::Field(modifiers, vd.fName, type));
            if (vd.fIsArray ? var.begin()->fNext : var.fFirstChild) {
                this->error(declsNode.fOffset, "initializers are not permitted on struct fields");
            }
        }
    }
    if (!this->expect(Token::Kind::TK_RBRACE, "'}'")) {
        return ASTNode::ID::Invalid();
    }
    if (fields.empty()) {
        this->error(name.fOffset,
                    "struct '" + this->text(name) + "' must contain at least one field");
        return ASTNode::ID::Invalid();
    }
    std::unique_ptr<Type> newType = Type::MakeStructType(name.fOffset, this->text(name), fields);
    if (struct_is_too_deeply_nested(*newType, kMaxStructDepth)) {
        this->error(name.fOffset, "struct '" + this->text(name) + "' is too deeply nested");
        return ASTNode::ID::Invalid();
    }
    fSymbols.add(std::move(newType));
    return this->createNode(name.fOffset, ASTNode::Kind::kType, this->text(name));
}

/* structDeclaration ((IDENTIFIER varDeclarationEnd) | SEMICOLON) */
ASTNode::ID Parser::structVarDeclaration(Modifiers modifiers) {
    ASTNode::ID type = this->structDeclaration();
    if (!type) {
        return ASTNode::ID::Invalid();
    }
    Token name;
    if (this->checkNext(Token::Kind::TK_IDENTIFIER, &name)) {
        return this->varDeclarationEnd(modifiers, std::move(type), this->text(name));
    }
    this->expect(Token::Kind::TK_SEMICOLON, "';'");
    return type;
}

/* (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)? (COMMA IDENTIFER
   (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)?)* SEMICOLON */
ASTNode::ID Parser::varDeclarationEnd(Modifiers mods, ASTNode::ID type, StringFragment name) {
    int offset = this->peek().fOffset;
    ASTNode::ID result = this->createNode(offset, ASTNode::Kind::kVarDeclarations);
    this->addChild(result, this->createNode(offset, ASTNode::Kind::kModifiers, mods));
    getNode(result).addChild(type);

    auto parseArrayDimensions = [&](ASTNode::ID currentVar, ASTNode::VarData* vd) -> bool {
        while (this->checkNext(Token::Kind::TK_LBRACKET)) {
            if (vd->fIsArray || this->isArrayType(type)) {
                this->error(this->peek(), "multi-dimensional arrays are not supported");
                return false;
            }
            if (this->checkNext(Token::Kind::TK_RBRACKET)) {
                this->createEmptyChild(currentVar);
            } else {
                ASTNode::ID size = this->expression();
                if (!size) {
                    return false;
                }
                getNode(currentVar).addChild(size);
                if (!this->expect(Token::Kind::TK_RBRACKET, "']'")) {
                    return false;
                }
            }
            vd->fIsArray = true;
        }
        return true;
    };

    auto parseInitializer = [this](ASTNode::ID currentVar) -> bool {
        if (this->checkNext(Token::Kind::TK_EQ)) {
            ASTNode::ID value = this->assignmentExpression();
            if (!value) {
                return false;
            }
            getNode(currentVar).addChild(value);
        }
        return true;
    };

    ASTNode::ID currentVar = this->createNode(offset, ASTNode::Kind::kVarDeclaration);
    ASTNode::VarData vd{name, /*isArray=*/false};

    getNode(result).addChild(currentVar);
    if (!parseArrayDimensions(currentVar, &vd)) {
        return ASTNode::ID::Invalid();
    }
    getNode(currentVar).setVarData(vd);
    if (!parseInitializer(currentVar)) {
        return ASTNode::ID::Invalid();
    }

    while (this->checkNext(Token::Kind::TK_COMMA)) {
        Token identifierName;
        if (!this->expectIdentifier(&identifierName)) {
            return ASTNode::ID::Invalid();
        }

        currentVar = ASTNode::ID(fFile->fNodes.size());
        vd = ASTNode::VarData{this->text(identifierName), /*isArray=*/false};
        fFile->fNodes.emplace_back(&fFile->fNodes, offset, ASTNode::Kind::kVarDeclaration);

        getNode(result).addChild(currentVar);
        if (!parseArrayDimensions(currentVar, &vd)) {
            return ASTNode::ID::Invalid();
        }
        getNode(currentVar).setVarData(vd);
        if (!parseInitializer(currentVar)) {
            return ASTNode::ID::Invalid();
        }
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return ASTNode::ID::Invalid();
    }
    return result;
}

/* modifiers type IDENTIFIER (LBRACKET INT_LITERAL RBRACKET)? */
ASTNode::ID Parser::parameter() {
    Modifiers modifiers = this->modifiersWithDefaults(0);
    ASTNode::ID type = this->type();
    if (!type) {
        return ASTNode::ID::Invalid();
    }
    Token name;
    if (!this->expectIdentifier(&name)) {
        return ASTNode::ID::Invalid();
    }
    ASTNode::ID result = this->createNode(name.fOffset, ASTNode::Kind::kParameter);
    ASTNode::ParameterData pd(modifiers, this->text(name), 0);
    getNode(result).addChild(type);
    while (this->checkNext(Token::Kind::TK_LBRACKET)) {
        if (pd.fIsArray || this->isArrayType(type)) {
            this->error(this->peek(), "multi-dimensional arrays are not supported");
            return ASTNode::ID::Invalid();
        }
        Token sizeToken;
        if (!this->expect(Token::Kind::TK_INT_LITERAL, "a positive integer", &sizeToken)) {
            return ASTNode::ID::Invalid();
        }
        StringFragment arraySizeFrag = this->text(sizeToken);
        SKSL_INT arraySize;
        if (!SkSL::stoi(arraySizeFrag, &arraySize)) {
            this->error(sizeToken, "array size is too large: " + arraySizeFrag);
            return ASTNode::ID::Invalid();
        }
        this->addChild(result, this->createNode(sizeToken.fOffset, ASTNode::Kind::kInt, arraySize));
        if (!this->expect(Token::Kind::TK_RBRACKET, "']'")) {
            return ASTNode::ID::Invalid();
        }
        pd.fIsArray = true;
    }
    getNode(result).setParameterData(pd);
    return result;
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
    StringFragment resultFrag = this->text(resultToken);
    SKSL_INT resultValue;
    if (!SkSL::stoi(resultFrag, &resultValue)) {
        this->error(resultToken, "value in layout is too large: " + resultFrag);
        return -1;
    }
    return resultValue;
}

/** EQ IDENTIFIER */
StringFragment Parser::layoutIdentifier() {
    if (!this->expect(Token::Kind::TK_EQ, "'='")) {
        return StringFragment();
    }
    Token resultToken;
    if (!this->expectIdentifier(&resultToken)) {
        return StringFragment();
    }
    return this->text(resultToken);
}


/** EQ <any sequence of tokens with balanced parentheses and no top-level comma> */
StringFragment Parser::layoutCode() {
    if (!this->expect(Token::Kind::TK_EQ, "'='")) {
        return "";
    }
    Token start = this->nextRawToken();
    this->pushback(start);
    StringFragment code;
    code.fChars = fText + start.fOffset;
    int level = 1;
    bool done = false;
    while (!done) {
        Token next = this->nextRawToken();
        switch (next.fKind) {
            case Token::Kind::TK_LPAREN:
                ++level;
                break;
            case Token::Kind::TK_RPAREN:
                --level;
                break;
            case Token::Kind::TK_COMMA:
                if (level == 1) {
                    done = true;
                }
                break;
            case Token::Kind::TK_END_OF_FILE:
                this->error(start, "reached end of file while parsing layout");
                return "";
            default:
                break;
        }
        if (!level) {
            done = true;
        }
        if (done) {
            code.fLength = next.fOffset - start.fOffset;
            this->pushback(std::move(next));
        }
    }
    return code;
}

Layout::CType Parser::layoutCType() {
    if (this->expect(Token::Kind::TK_EQ, "'='")) {
        Token t = this->nextToken();
        String text = this->text(t);
        auto found = layoutTokens->find(text);
        if (found != layoutTokens->end()) {
            switch (found->second) {
                case LayoutToken::SKPMCOLOR4F:
                    return Layout::CType::kSkPMColor4f;
                case LayoutToken::SKV4:
                    return Layout::CType::kSkV4;
                case LayoutToken::SKRECT:
                    return Layout::CType::kSkRect;
                case LayoutToken::SKIRECT:
                    return Layout::CType::kSkIRect;
                case LayoutToken::SKPMCOLOR:
                    return Layout::CType::kSkPMColor;
                case LayoutToken::BOOL:
                    return Layout::CType::kBool;
                case LayoutToken::INT:
                    return Layout::CType::kInt32;
                case LayoutToken::FLOAT:
                    return Layout::CType::kFloat;
                case LayoutToken::SKM44:
                    return Layout::CType::kSkM44;
                default:
                    break;
            }
        }
        this->error(t, "unsupported ctype");
    }
    return Layout::CType::kDefault;
}

/* LAYOUT LPAREN IDENTIFIER (EQ INT_LITERAL)? (COMMA IDENTIFIER (EQ INT_LITERAL)?)* RPAREN */
Layout Parser::layout() {
    int flags = 0;
    int location = -1;
    int offset = -1;
    int binding = -1;
    int index = -1;
    int set = -1;
    int builtin = -1;
    int inputAttachmentIndex = -1;
    Layout::Primitive primitive = Layout::kUnspecified_Primitive;
    int maxVertices = -1;
    int invocations = -1;
    StringFragment marker;
    StringFragment when;
    Layout::CType ctype = Layout::CType::kDefault;
    if (this->checkNext(Token::Kind::TK_LAYOUT)) {
        if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
            return Layout(flags, location, offset, binding, index, set, builtin,
                          inputAttachmentIndex, primitive, maxVertices, invocations, marker, when,
                          ctype);
        }
        for (;;) {
            Token t = this->nextToken();
            String text = this->text(t);
            auto setFlag = [&](Layout::Flag f) {
                if (flags & f) {
                    this->error(t, "layout qualifier '" + text + "' appears more than once");
                }
                flags |= f;
            };
            auto setPrimitive = [&](Layout::Primitive p) {
                if (flags & Layout::kPrimitive_Flag) {
                    this->error(t, "only one primitive-type layout qualifier is allowed");
                }
                flags |= Layout::kPrimitive_Flag;
                primitive = p;
            };

            auto found = layoutTokens->find(text);
            if (found != layoutTokens->end()) {
                switch (found->second) {
                    case LayoutToken::ORIGIN_UPPER_LEFT:
                        setFlag(Layout::kOriginUpperLeft_Flag);
                        break;
                    case LayoutToken::OVERRIDE_COVERAGE:
                        setFlag(Layout::kOverrideCoverage_Flag);
                        break;
                    case LayoutToken::EARLY_FRAGMENT_TESTS:
                        setFlag(Layout::kEarlyFragmentTests_Flag);
                        break;
                    case LayoutToken::PUSH_CONSTANT:
                        setFlag(Layout::kPushConstant_Flag);
                        break;
                    case LayoutToken::BLEND_SUPPORT_ALL_EQUATIONS:
                        setFlag(Layout::kBlendSupportAllEquations_Flag);
                        break;
                    case LayoutToken::TRACKED:
                        setFlag(Layout::kTracked_Flag);
                        break;
                    case LayoutToken::SRGB_UNPREMUL:
                        setFlag(Layout::kSRGBUnpremul_Flag);
                        break;
                    case LayoutToken::KEY:
                        setFlag(Layout::kKey_Flag);
                        break;
                    case LayoutToken::LOCATION:
                        setFlag(Layout::kLocation_Flag);
                        location = this->layoutInt();
                        break;
                    case LayoutToken::OFFSET:
                        setFlag(Layout::kOffset_Flag);
                        offset = this->layoutInt();
                        break;
                    case LayoutToken::BINDING:
                        setFlag(Layout::kBinding_Flag);
                        binding = this->layoutInt();
                        break;
                    case LayoutToken::INDEX:
                        setFlag(Layout::kIndex_Flag);
                        index = this->layoutInt();
                        break;
                    case LayoutToken::SET:
                        setFlag(Layout::kSet_Flag);
                        set = this->layoutInt();
                        break;
                    case LayoutToken::BUILTIN:
                        setFlag(Layout::kBuiltin_Flag);
                        builtin = this->layoutInt();
                        break;
                    case LayoutToken::INPUT_ATTACHMENT_INDEX:
                        setFlag(Layout::kInputAttachmentIndex_Flag);
                        inputAttachmentIndex = this->layoutInt();
                        break;
                    case LayoutToken::POINTS:
                        setPrimitive(Layout::kPoints_Primitive);
                        break;
                    case LayoutToken::LINES:
                        setPrimitive(Layout::kLines_Primitive);
                        break;
                    case LayoutToken::LINE_STRIP:
                        setPrimitive(Layout::kLineStrip_Primitive);
                        break;
                    case LayoutToken::LINES_ADJACENCY:
                        setPrimitive(Layout::kLinesAdjacency_Primitive);
                        break;
                    case LayoutToken::TRIANGLES:
                        setPrimitive(Layout::kTriangles_Primitive);
                        break;
                    case LayoutToken::TRIANGLE_STRIP:
                        setPrimitive(Layout::kTriangleStrip_Primitive);
                        break;
                    case LayoutToken::TRIANGLES_ADJACENCY:
                        setPrimitive(Layout::kTrianglesAdjacency_Primitive);
                        break;
                    case LayoutToken::MAX_VERTICES:
                        setFlag(Layout::kMaxVertices_Flag);
                        maxVertices = this->layoutInt();
                        break;
                    case LayoutToken::INVOCATIONS:
                        setFlag(Layout::kInvocations_Flag);
                        invocations = this->layoutInt();
                        break;
                    case LayoutToken::MARKER:
                        setFlag(Layout::kMarker_Flag);
                        marker = this->layoutCode();
                        break;
                    case LayoutToken::WHEN:
                        setFlag(Layout::kWhen_Flag);
                        when = this->layoutCode();
                        break;
                    case LayoutToken::CTYPE:
                        setFlag(Layout::kCType_Flag);
                        ctype = this->layoutCType();
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
    return Layout(flags, location, offset, binding, index, set, builtin, inputAttachmentIndex,
                  primitive, maxVertices, invocations, marker, when, ctype);
}

/* layout? (UNIFORM | CONST | IN | OUT | INOUT | LOWP | MEDIUMP | HIGHP | FLAT | NOPERSPECTIVE |
            VARYING | INLINE)* */
Modifiers Parser::modifiers() {
    Layout layout = this->layout();
    int flags = 0;
    for (;;) {
        // TODO: handle duplicate / incompatible flags
        int tokenFlag = parse_modifier_token(peek().fKind);
        if (!tokenFlag) {
            break;
        }
        flags |= tokenFlag;
        this->nextToken();
    }
    return Modifiers(layout, flags);
}

Modifiers Parser::modifiersWithDefaults(int defaultFlags) {
    Modifiers result = this->modifiers();
    if (!result.fFlags) {
        return Modifiers(result.fLayout, defaultFlags);
    }
    return result;
}

/* ifStatement | forStatement | doStatement | whileStatement | block | expression */
ASTNode::ID Parser::statement() {
    Token start = this->nextToken();
    AutoDepth depth(this);
    if (!depth.increase()) {
        return ASTNode::ID::Invalid();
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
        case Token::Kind::TK_LBRACE:
            return this->block();
        case Token::Kind::TK_SEMICOLON:
            this->nextToken();
            return this->createNode(start.fOffset, ASTNode::Kind::kBlock);
        case Token::Kind::TK_CONST:
            return this->varDeclarations();
        case Token::Kind::TK_IDENTIFIER:
            return this->varDeclarationsOrExpressionStatement();
        default:
            return this->expressionStatement();
    }
}

/* IDENTIFIER(type) (LBRACKET intLiteral? RBRACKET)* QUESTION? */
ASTNode::ID Parser::type() {
    Token type;
    if (!this->expect(Token::Kind::TK_IDENTIFIER, "a type", &type)) {
        return ASTNode::ID::Invalid();
    }
    if (!this->isType(this->text(type))) {
        this->error(type, ("no type named '" + this->text(type) + "'").c_str());
        return ASTNode::ID::Invalid();
    }
    ASTNode::ID result = this->createNode(type.fOffset, ASTNode::Kind::kType, this->text(type));
    bool isArray = false;
    while (this->checkNext(Token::Kind::TK_LBRACKET)) {
        if (isArray) {
            this->error(this->peek(), "multi-dimensional arrays are not supported");
            return ASTNode::ID::Invalid();
        }
        if (this->peek().fKind != Token::Kind::TK_RBRACKET) {
            SKSL_INT i;
            if (this->intLiteral(&i)) {
                this->addChild(result, this->createNode(this->peek().fOffset,
                                                        ASTNode::Kind::kInt, i));
            } else {
                return ASTNode::ID::Invalid();
            }
        } else {
            this->createEmptyChild(result);
        }
        isArray = true;
        this->expect(Token::Kind::TK_RBRACKET, "']'");
    }
    return result;
}

/* IDENTIFIER LBRACE
     varDeclaration+
   RBRACE (IDENTIFIER (LBRACKET expression? RBRACKET)*)? SEMICOLON */
ASTNode::ID Parser::interfaceBlock(Modifiers mods) {
    Token name;
    if (!this->expectIdentifier(&name)) {
        return ASTNode::ID::Invalid();
    }
    if (peek().fKind != Token::Kind::TK_LBRACE) {
        // we only get into interfaceBlock if we found a top-level identifier which was not a type.
        // 99% of the time, the user was not actually intending to create an interface block, so
        // it's better to report it as an unknown type
        this->error(name, "no type named '" + this->text(name) + "'");
        return ASTNode::ID::Invalid();
    }
    ASTNode::ID result = this->createNode(name.fOffset, ASTNode::Kind::kInterfaceBlock);
    ASTNode::InterfaceBlockData id(mods, this->text(name), 0, "", 0);
    this->nextToken();
    while (this->peek().fKind != Token::Kind::TK_RBRACE) {
        ASTNode::ID decl = this->varDeclarations();
        if (!decl) {
            return ASTNode::ID::Invalid();
        }
        getNode(result).addChild(decl);
        ++id.fDeclarationCount;
    }
    if (id.fDeclarationCount == 0) {
        this->error(name, "interface block '" + this->text(name) +
                          "' must contain at least one member");
        return ASTNode::ID::Invalid();
    }
    this->nextToken();
    std::vector<ASTNode> sizes;
    StringFragment instanceName;
    Token instanceNameToken;
    if (this->checkNext(Token::Kind::TK_IDENTIFIER, &instanceNameToken)) {
        id.fInstanceName = this->text(instanceNameToken);
        while (this->checkNext(Token::Kind::TK_LBRACKET)) {
            if (id.fIsArray) {
                this->error(this->peek(), "multi-dimensional arrays are not supported");
                return false;
            }
            if (this->peek().fKind != Token::Kind::TK_RBRACKET) {
                ASTNode::ID size = this->expression();
                if (!size) {
                    return ASTNode::ID::Invalid();
                }
                getNode(result).addChild(size);
            } else {
                this->createEmptyChild(result);
            }
            this->expect(Token::Kind::TK_RBRACKET, "']'");
            id.fIsArray = true;
        }
        instanceName = this->text(instanceNameToken);
    }
    getNode(result).setInterfaceBlockData(id);
    this->expect(Token::Kind::TK_SEMICOLON, "';'");
    return result;
}

/* IF LPAREN expression RPAREN statement (ELSE statement)? */
ASTNode::ID Parser::ifStatement() {
    Token start;
    bool isStatic = this->checkNext(Token::Kind::TK_STATIC_IF, &start);
    if (!isStatic && !this->expect(Token::Kind::TK_IF, "'if'", &start)) {
        return ASTNode::ID::Invalid();
    }
    ASTNode::ID result = this->createNode(start.fOffset, ASTNode::Kind::kIf, isStatic);
    if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
        return ASTNode::ID::Invalid();
    }
    ASTNode::ID test = this->expression();
    if (!test) {
        return ASTNode::ID::Invalid();
    }
    getNode(result).addChild(test);
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
        return ASTNode::ID::Invalid();
    }
    ASTNode::ID ifTrue = this->statement();
    if (!ifTrue) {
        return ASTNode::ID::Invalid();
    }
    getNode(result).addChild(ifTrue);
    ASTNode::ID ifFalse;
    if (this->checkNext(Token::Kind::TK_ELSE)) {
        ifFalse = this->statement();
        if (!ifFalse) {
            return ASTNode::ID::Invalid();
        }
        getNode(result).addChild(ifFalse);
    }
    return result;
}

/* DO statement WHILE LPAREN expression RPAREN SEMICOLON */
ASTNode::ID Parser::doStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_DO, "'do'", &start)) {
        return ASTNode::ID::Invalid();
    }
    ASTNode::ID result = this->createNode(start.fOffset, ASTNode::Kind::kDo);
    ASTNode::ID statement = this->statement();
    if (!statement) {
        return ASTNode::ID::Invalid();
    }
    getNode(result).addChild(statement);
    if (!this->expect(Token::Kind::TK_WHILE, "'while'")) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
        return ASTNode::ID::Invalid();
    }
    ASTNode::ID test = this->expression();
    if (!test) {
        return ASTNode::ID::Invalid();
    }
    getNode(result).addChild(test);
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return ASTNode::ID::Invalid();
    }
    return result;
}

/* WHILE LPAREN expression RPAREN STATEMENT */
ASTNode::ID Parser::whileStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_WHILE, "'while'", &start)) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
        return ASTNode::ID::Invalid();
    }
    ASTNode::ID result = this->createNode(start.fOffset, ASTNode::Kind::kWhile);
    ASTNode::ID test = this->expression();
    if (!test) {
        return ASTNode::ID::Invalid();
    }
    getNode(result).addChild(test);
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
        return ASTNode::ID::Invalid();
    }
    ASTNode::ID statement = this->statement();
    if (!statement) {
        return ASTNode::ID::Invalid();
    }
    getNode(result).addChild(statement);
    return result;
}

/* CASE expression COLON statement* */
ASTNode::ID Parser::switchCase() {
    Token start;
    if (!this->expect(Token::Kind::TK_CASE, "'case'", &start)) {
        return ASTNode::ID::Invalid();
    }
    ASTNode::ID result = this->createNode(start.fOffset, ASTNode::Kind::kSwitchCase);
    ASTNode::ID value = this->expression();
    if (!value) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::Kind::TK_COLON, "':'")) {
        return ASTNode::ID::Invalid();
    }
    getNode(result).addChild(value);
    while (this->peek().fKind != Token::Kind::TK_RBRACE &&
           this->peek().fKind != Token::Kind::TK_CASE &&
           this->peek().fKind != Token::Kind::TK_DEFAULT) {
        ASTNode::ID s = this->statement();
        if (!s) {
            return ASTNode::ID::Invalid();
        }
        getNode(result).addChild(s);
    }
    return result;
}

/* SWITCH LPAREN expression RPAREN LBRACE switchCase* (DEFAULT COLON statement*)? RBRACE */
ASTNode::ID Parser::switchStatement() {
    Token start;
    bool isStatic = this->checkNext(Token::Kind::TK_STATIC_SWITCH, &start);
    if (!isStatic && !this->expect(Token::Kind::TK_SWITCH, "'switch'", &start)) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
        return ASTNode::ID::Invalid();
    }
    ASTNode::ID value = this->expression();
    if (!value) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::Kind::TK_LBRACE, "'{'")) {
        return ASTNode::ID::Invalid();
    }
    ASTNode::ID result = this->createNode(start.fOffset, ASTNode::Kind::kSwitch, isStatic);
    getNode(result).addChild(value);
    while (this->peek().fKind == Token::Kind::TK_CASE) {
        ASTNode::ID c = this->switchCase();
        if (!c) {
            return ASTNode::ID::Invalid();
        }
        getNode(result).addChild(c);
    }
    // Requiring default: to be last (in defiance of C and GLSL) was a deliberate decision. Other
    // parts of the compiler may rely upon this assumption.
    if (this->peek().fKind == Token::Kind::TK_DEFAULT) {
        Token defaultStart;
        SkAssertResult(this->expect(Token::Kind::TK_DEFAULT, "'default'", &defaultStart));
        if (!this->expect(Token::Kind::TK_COLON, "':'")) {
            return ASTNode::ID::Invalid();
        }
        ASTNode::ID defaultCase = this->addChild(
                result, this->createNode(defaultStart.fOffset, ASTNode::Kind::kSwitchCase));
        this->createEmptyChild(defaultCase); // empty test to signify default case
        while (this->peek().fKind != Token::Kind::TK_RBRACE) {
            ASTNode::ID s = this->statement();
            if (!s) {
                return ASTNode::ID::Invalid();
            }
            getNode(defaultCase).addChild(s);
        }
    }
    if (!this->expect(Token::Kind::TK_RBRACE, "'}'")) {
        return ASTNode::ID::Invalid();
    }
    return result;
}

/* FOR LPAREN (declaration | expression)? SEMICOLON expression? SEMICOLON expression? RPAREN
   STATEMENT */
ASTNode::ID Parser::forStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_FOR, "'for'", &start)) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
        return ASTNode::ID::Invalid();
    }
    ASTNode::ID result = this->createNode(start.fOffset, ASTNode::Kind::kFor);
    ASTNode::ID initializer;
    Token nextToken = this->peek();
    switch (nextToken.fKind) {
        case Token::Kind::TK_SEMICOLON:
            this->nextToken();
            this->createEmptyChild(result);
            break;
        case Token::Kind::TK_CONST: {
            initializer = this->varDeclarations();
            if (!initializer) {
                return ASTNode::ID::Invalid();
            }
            getNode(result).addChild(initializer);
            break;
        }
        case Token::Kind::TK_IDENTIFIER: {
            if (this->isType(this->text(nextToken))) {
                initializer = this->varDeclarations();
                if (!initializer) {
                    return ASTNode::ID::Invalid();
                }
                getNode(result).addChild(initializer);
                break;
            }
            [[fallthrough]];
        }
        default:
            initializer = this->expressionStatement();
            if (!initializer) {
                return ASTNode::ID::Invalid();
            }
            getNode(result).addChild(initializer);
    }
    ASTNode::ID test;
    if (this->peek().fKind != Token::Kind::TK_SEMICOLON) {
        test = this->expression();
        if (!test) {
            return ASTNode::ID::Invalid();
        }
        getNode(result).addChild(test);
    } else {
        this->createEmptyChild(result);
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return ASTNode::ID::Invalid();
    }
    ASTNode::ID next;
    if (this->peek().fKind != Token::Kind::TK_RPAREN) {
        next = this->expression();
        if (!next) {
            return ASTNode::ID::Invalid();
        }
        getNode(result).addChild(next);
    } else {
        this->createEmptyChild(result);
    }
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
        return ASTNode::ID::Invalid();
    }
    ASTNode::ID statement = this->statement();
    if (!statement) {
        return ASTNode::ID::Invalid();
    }
    getNode(result).addChild(statement);
    return result;
}

/* RETURN expression? SEMICOLON */
ASTNode::ID Parser::returnStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_RETURN, "'return'", &start)) {
        return ASTNode::ID::Invalid();
    }
    ASTNode::ID result = this->createNode(start.fOffset, ASTNode::Kind::kReturn);
    if (this->peek().fKind != Token::Kind::TK_SEMICOLON) {
        ASTNode::ID expression = this->expression();
        if (!expression) {
            return ASTNode::ID::Invalid();
        }
        getNode(result).addChild(expression);
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return ASTNode::ID::Invalid();
    }
    return result;
}

/* BREAK SEMICOLON */
ASTNode::ID Parser::breakStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_BREAK, "'break'", &start)) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return ASTNode::ID::Invalid();
    }
    return this->createNode(start.fOffset, ASTNode::Kind::kBreak);
}

/* CONTINUE SEMICOLON */
ASTNode::ID Parser::continueStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_CONTINUE, "'continue'", &start)) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return ASTNode::ID::Invalid();
    }
    return this->createNode(start.fOffset, ASTNode::Kind::kContinue);
}

/* DISCARD SEMICOLON */
ASTNode::ID Parser::discardStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_DISCARD, "'continue'", &start)) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return ASTNode::ID::Invalid();
    }
    return this->createNode(start.fOffset, ASTNode::Kind::kDiscard);
}

/* LBRACE statement* RBRACE */
ASTNode::ID Parser::block() {
    Token start;
    if (!this->expect(Token::Kind::TK_LBRACE, "'{'", &start)) {
        return ASTNode::ID::Invalid();
    }
    AutoDepth depth(this);
    if (!depth.increase()) {
        return ASTNode::ID::Invalid();
    }
    ASTNode::ID result = this->createNode(start.fOffset, ASTNode::Kind::kBlock);
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_RBRACE:
                this->nextToken();
                return result;
            case Token::Kind::TK_END_OF_FILE:
                this->error(this->peek(), "expected '}', but found end of file");
                return ASTNode::ID::Invalid();
            default: {
                ASTNode::ID statement = this->statement();
                if (!statement) {
                    return ASTNode::ID::Invalid();
                }
                getNode(result).addChild(statement);
            }
        }
    }
    return result;
}

/* expression SEMICOLON */
ASTNode::ID Parser::expressionStatement() {
    ASTNode::ID expr = this->expression();
    if (expr) {
        if (this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
            return expr;
        }
    }
    return ASTNode::ID::Invalid();
}

/* assignmentExpression (COMMA assignmentExpression)* */
ASTNode::ID Parser::expression() {
    ASTNode::ID result = this->assignmentExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    Token t;
    AutoDepth depth(this);
    while (this->checkNext(Token::Kind::TK_COMMA, &t)) {
        if (!depth.increase()) {
            return ASTNode::ID::Invalid();
        }
        ASTNode::ID right = this->assignmentExpression();
        if (!right) {
            return ASTNode::ID::Invalid();
        }
        ASTNode::ID newResult = this->createNode(t.fOffset, ASTNode::Kind::kBinary,
                                                 Operator(t.fKind));
        getNode(newResult).addChild(result);
        getNode(newResult).addChild(right);
        result = newResult;
    }
    return result;
}

/* ternaryExpression ((EQEQ | STAREQ | SLASHEQ | PERCENTEQ | PLUSEQ | MINUSEQ | SHLEQ | SHREQ |
   BITWISEANDEQ | BITWISEXOREQ | BITWISEOREQ | LOGICALANDEQ | LOGICALXOREQ | LOGICALOREQ)
   assignmentExpression)*
 */
ASTNode::ID Parser::assignmentExpression() {
    AutoDepth depth(this);
    ASTNode::ID result = this->ternaryExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_EQ:           // fall through
            case Token::Kind::TK_STAREQ:       // fall through
            case Token::Kind::TK_SLASHEQ:      // fall through
            case Token::Kind::TK_PERCENTEQ:    // fall through
            case Token::Kind::TK_PLUSEQ:       // fall through
            case Token::Kind::TK_MINUSEQ:      // fall through
            case Token::Kind::TK_SHLEQ:        // fall through
            case Token::Kind::TK_SHREQ:        // fall through
            case Token::Kind::TK_BITWISEANDEQ: // fall through
            case Token::Kind::TK_BITWISEXOREQ: // fall through
            case Token::Kind::TK_BITWISEOREQ: {
                if (!depth.increase()) {
                    return ASTNode::ID::Invalid();
                }
                Token t = this->nextToken();
                ASTNode::ID right = this->assignmentExpression();
                if (!right) {
                    return ASTNode::ID::Invalid();
                }
                ASTNode::ID newResult = this->createNode(getNode(result).fOffset,
                                                         ASTNode::Kind::kBinary, Operator(t.fKind));
                getNode(newResult).addChild(result);
                getNode(newResult).addChild(right);
                result = newResult;
                break;
            }
            default:
                return result;
        }
    }
}

/* logicalOrExpression ('?' expression ':' assignmentExpression)? */
ASTNode::ID Parser::ternaryExpression() {
    AutoDepth depth(this);
    ASTNode::ID base = this->logicalOrExpression();
    if (!base) {
        return ASTNode::ID::Invalid();
    }
    if (this->checkNext(Token::Kind::TK_QUESTION)) {
        if (!depth.increase()) {
            return ASTNode::ID::Invalid();
        }
        ASTNode::ID trueExpr = this->expression();
        if (!trueExpr) {
            return ASTNode::ID::Invalid();
        }
        if (this->expect(Token::Kind::TK_COLON, "':'")) {
            ASTNode::ID falseExpr = this->assignmentExpression();
            if (!falseExpr) {
                return ASTNode::ID::Invalid();
            }
            ASTNode::ID ternary = this->createNode(getNode(base).fOffset, ASTNode::Kind::kTernary);
            getNode(ternary).addChild(base);
            getNode(ternary).addChild(trueExpr);
            getNode(ternary).addChild(falseExpr);
            return ternary;
        }
        return ASTNode::ID::Invalid();
    }
    return base;
}

/* logicalXorExpression (LOGICALOR logicalXorExpression)* */
ASTNode::ID Parser::logicalOrExpression() {
    AutoDepth depth(this);
    ASTNode::ID result = this->logicalXorExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    Token t;
    while (this->checkNext(Token::Kind::TK_LOGICALOR, &t)) {
        if (!depth.increase()) {
            return ASTNode::ID::Invalid();
        }
        ASTNode::ID right = this->logicalXorExpression();
        if (!right) {
            return ASTNode::ID::Invalid();
        }
        ASTNode::ID newResult = this->createNode(getNode(result).fOffset, ASTNode::Kind::kBinary,
                                                 Operator(t.fKind));
        getNode(newResult).addChild(result);
        getNode(newResult).addChild(right);
        result = newResult;
    }
    return result;
}

/* logicalAndExpression (LOGICALXOR logicalAndExpression)* */
ASTNode::ID Parser::logicalXorExpression() {
    AutoDepth depth(this);
    ASTNode::ID result = this->logicalAndExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    Token t;
    while (this->checkNext(Token::Kind::TK_LOGICALXOR, &t)) {
        if (!depth.increase()) {
            return ASTNode::ID::Invalid();
        }
        ASTNode::ID right = this->logicalAndExpression();
        if (!right) {
            return ASTNode::ID::Invalid();
        }
        ASTNode::ID newResult = this->createNode(getNode(result).fOffset, ASTNode::Kind::kBinary,
                                                 Operator(t.fKind));
        getNode(newResult).addChild(result);
        getNode(newResult).addChild(right);
        result = newResult;
    }
    return result;
}

/* bitwiseOrExpression (LOGICALAND bitwiseOrExpression)* */
ASTNode::ID Parser::logicalAndExpression() {
    AutoDepth depth(this);
    ASTNode::ID result = this->bitwiseOrExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    Token t;
    while (this->checkNext(Token::Kind::TK_LOGICALAND, &t)) {
        if (!depth.increase()) {
            return ASTNode::ID::Invalid();
        }
        ASTNode::ID right = this->bitwiseOrExpression();
        if (!right) {
            return ASTNode::ID::Invalid();
        }
        ASTNode::ID newResult = this->createNode(getNode(result).fOffset, ASTNode::Kind::kBinary,
                                                 Operator(t.fKind));
        getNode(newResult).addChild(result);
        getNode(newResult).addChild(right);
        result = newResult;
    }
    return result;
}

/* bitwiseXorExpression (BITWISEOR bitwiseXorExpression)* */
ASTNode::ID Parser::bitwiseOrExpression() {
    AutoDepth depth(this);
    ASTNode::ID result = this->bitwiseXorExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    Token t;
    while (this->checkNext(Token::Kind::TK_BITWISEOR, &t)) {
        if (!depth.increase()) {
            return ASTNode::ID::Invalid();
        }
        ASTNode::ID right = this->bitwiseXorExpression();
        if (!right) {
            return ASTNode::ID::Invalid();
        }
        ASTNode::ID newResult = this->createNode(getNode(result).fOffset, ASTNode::Kind::kBinary,
                                                 Operator(t.fKind));
        getNode(newResult).addChild(result);
        getNode(newResult).addChild(right);
        result = newResult;
    }
    return result;
}

/* bitwiseAndExpression (BITWISEXOR bitwiseAndExpression)* */
ASTNode::ID Parser::bitwiseXorExpression() {
    AutoDepth depth(this);
    ASTNode::ID result = this->bitwiseAndExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    Token t;
    while (this->checkNext(Token::Kind::TK_BITWISEXOR, &t)) {
        if (!depth.increase()) {
            return ASTNode::ID::Invalid();
        }
        ASTNode::ID right = this->bitwiseAndExpression();
        if (!right) {
            return ASTNode::ID::Invalid();
        }
        ASTNode::ID newResult = this->createNode(getNode(result).fOffset, ASTNode::Kind::kBinary,
                                                 Operator(t.fKind));
        getNode(newResult).addChild(result);
        getNode(newResult).addChild(right);
        result = newResult;
    }
    return result;
}

/* equalityExpression (BITWISEAND equalityExpression)* */
ASTNode::ID Parser::bitwiseAndExpression() {
    AutoDepth depth(this);
    ASTNode::ID result = this->equalityExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    Token t;
    while (this->checkNext(Token::Kind::TK_BITWISEAND, &t)) {
        if (!depth.increase()) {
            return ASTNode::ID::Invalid();
        }
        ASTNode::ID right = this->equalityExpression();
        if (!right) {
            return ASTNode::ID::Invalid();
        }
        ASTNode::ID newResult = this->createNode(getNode(result).fOffset, ASTNode::Kind::kBinary,
                                                 Operator(t.fKind));
        getNode(newResult).addChild(result);
        getNode(newResult).addChild(right);
        result = newResult;
    }
    return result;
}

/* relationalExpression ((EQEQ | NEQ) relationalExpression)* */
ASTNode::ID Parser::equalityExpression() {
    AutoDepth depth(this);
    ASTNode::ID result = this->relationalExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_EQEQ:   // fall through
            case Token::Kind::TK_NEQ: {
                if (!depth.increase()) {
                    return ASTNode::ID::Invalid();
                }
                Token t = this->nextToken();
                ASTNode::ID right = this->relationalExpression();
                if (!right) {
                    return ASTNode::ID::Invalid();
                }
                ASTNode::ID newResult = this->createNode(getNode(result).fOffset,
                                                         ASTNode::Kind::kBinary, Operator(t.fKind));
                getNode(newResult).addChild(result);
                getNode(newResult).addChild(right);
                result = newResult;
                break;
            }
            default:
                return result;
        }
    }
}

/* shiftExpression ((LT | GT | LTEQ | GTEQ) shiftExpression)* */
ASTNode::ID Parser::relationalExpression() {
    AutoDepth depth(this);
    ASTNode::ID result = this->shiftExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_LT:   // fall through
            case Token::Kind::TK_GT:   // fall through
            case Token::Kind::TK_LTEQ: // fall through
            case Token::Kind::TK_GTEQ: {
                if (!depth.increase()) {
                    return ASTNode::ID::Invalid();
                }
                Token t = this->nextToken();
                ASTNode::ID right = this->shiftExpression();
                if (!right) {
                    return ASTNode::ID::Invalid();
                }
                ASTNode::ID newResult = this->createNode(getNode(result).fOffset,
                                                         ASTNode::Kind::kBinary, Operator(t.fKind));
                getNode(newResult).addChild(result);
                getNode(newResult).addChild(right);
                result = newResult;
                break;
            }
            default:
                return result;
        }
    }
}

/* additiveExpression ((SHL | SHR) additiveExpression)* */
ASTNode::ID Parser::shiftExpression() {
    AutoDepth depth(this);
    ASTNode::ID result = this->additiveExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_SHL: // fall through
            case Token::Kind::TK_SHR: {
                if (!depth.increase()) {
                    return ASTNode::ID::Invalid();
                }
                Token t = this->nextToken();
                ASTNode::ID right = this->additiveExpression();
                if (!right) {
                    return ASTNode::ID::Invalid();
                }
                ASTNode::ID newResult = this->createNode(getNode(result).fOffset,
                                                         ASTNode::Kind::kBinary, Operator(t.fKind));
                getNode(newResult).addChild(result);
                getNode(newResult).addChild(right);
                result = newResult;
                break;
            }
            default:
                return result;
        }
    }
}

/* multiplicativeExpression ((PLUS | MINUS) multiplicativeExpression)* */
ASTNode::ID Parser::additiveExpression() {
    AutoDepth depth(this);
    ASTNode::ID result = this->multiplicativeExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_PLUS: // fall through
            case Token::Kind::TK_MINUS: {
                if (!depth.increase()) {
                    return ASTNode::ID::Invalid();
                }
                Token t = this->nextToken();
                ASTNode::ID right = this->multiplicativeExpression();
                if (!right) {
                    return ASTNode::ID::Invalid();
                }
                ASTNode::ID newResult = this->createNode(getNode(result).fOffset,
                                                         ASTNode::Kind::kBinary, Operator(t.fKind));
                getNode(newResult).addChild(result);
                getNode(newResult).addChild(right);
                result = newResult;
                break;
            }
            default:
                return result;
        }
    }
}

/* unaryExpression ((STAR | SLASH | PERCENT) unaryExpression)* */
ASTNode::ID Parser::multiplicativeExpression() {
    AutoDepth depth(this);
    ASTNode::ID result = this->unaryExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_STAR: // fall through
            case Token::Kind::TK_SLASH: // fall through
            case Token::Kind::TK_PERCENT: {
                if (!depth.increase()) {
                    return ASTNode::ID::Invalid();
                }
                Token t = this->nextToken();
                ASTNode::ID right = this->unaryExpression();
                if (!right) {
                    return ASTNode::ID::Invalid();
                }
                ASTNode::ID newResult = this->createNode(getNode(result).fOffset,
                                                         ASTNode::Kind::kBinary, Operator(t.fKind));
                getNode(newResult).addChild(result);
                getNode(newResult).addChild(right);
                result = newResult;
                break;
            }
            default:
                return result;
        }
    }
}

/* postfixExpression | (PLUS | MINUS | NOT | PLUSPLUS | MINUSMINUS) unaryExpression */
ASTNode::ID Parser::unaryExpression() {
    AutoDepth depth(this);
    switch (this->peek().fKind) {
        case Token::Kind::TK_PLUS:       // fall through
        case Token::Kind::TK_MINUS:      // fall through
        case Token::Kind::TK_LOGICALNOT: // fall through
        case Token::Kind::TK_BITWISENOT: // fall through
        case Token::Kind::TK_PLUSPLUS:   // fall through
        case Token::Kind::TK_MINUSMINUS: {
            if (!depth.increase()) {
                return ASTNode::ID::Invalid();
            }
            Token t = this->nextToken();
            ASTNode::ID expr = this->unaryExpression();
            if (!expr) {
                return ASTNode::ID::Invalid();
            }
            ASTNode::ID result = this->createNode(t.fOffset, ASTNode::Kind::kPrefix,
                                                  Operator(t.fKind));
            getNode(result).addChild(expr);
            return result;
        }
        default:
            return this->postfixExpression();
    }
}

/* term suffix* */
ASTNode::ID Parser::postfixExpression() {
    AutoDepth depth(this);
    ASTNode::ID result = this->term();
    if (!result) {
        return ASTNode::ID::Invalid();
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
            case Token::Kind::TK_COLONCOLON:
                if (!depth.increase()) {
                    return ASTNode::ID::Invalid();
                }
                result = this->suffix(result);
                if (!result) {
                    return ASTNode::ID::Invalid();
                }
                break;
            default:
                return result;
        }
    }
}

/* LBRACKET expression? RBRACKET | DOT IDENTIFIER | LPAREN parameters RPAREN |
   PLUSPLUS | MINUSMINUS | COLONCOLON IDENTIFIER | FLOAT_LITERAL [IDENTIFIER] */
ASTNode::ID Parser::suffix(ASTNode::ID base) {
    SkASSERT(base);
    Token next = this->nextToken();
    AutoDepth depth(this);
    if (!depth.increase()) {
        return ASTNode::ID::Invalid();
    }
    switch (next.fKind) {
        case Token::Kind::TK_LBRACKET: {
            if (this->checkNext(Token::Kind::TK_RBRACKET)) {
                ASTNode::ID result = this->createNode(next.fOffset, ASTNode::Kind::kIndex);
                getNode(result).addChild(base);
                return result;
            }
            ASTNode::ID e = this->expression();
            if (!e) {
                return ASTNode::ID::Invalid();
            }
            this->expect(Token::Kind::TK_RBRACKET, "']' to complete array access expression");
            ASTNode::ID result = this->createNode(next.fOffset, ASTNode::Kind::kIndex);
            getNode(result).addChild(base);
            getNode(result).addChild(e);
            return result;
        }
        case Token::Kind::TK_COLONCOLON: {
            int offset = this->peek().fOffset;
            StringFragment text;
            if (this->identifier(&text)) {
                ASTNode::ID result = this->createNode(offset, ASTNode::Kind::kScope,
                                                      std::move(text));
                getNode(result).addChild(base);
                return result;
            }
            return ASTNode::ID::Invalid();
        }
        case Token::Kind::TK_DOT: {
            int offset = this->peek().fOffset;
            StringFragment text;
            if (this->identifier(&text)) {
                ASTNode::ID result = this->createNode(offset, ASTNode::Kind::kField,
                                                      std::move(text));
                getNode(result).addChild(base);
                return result;
            }
            [[fallthrough]];
        }
        case Token::Kind::TK_FLOAT_LITERAL: {
            // Swizzles that start with a constant number, e.g. '.000r', will be tokenized as
            // floating point literals, possibly followed by an identifier. Handle that here.
            StringFragment field = this->text(next);
            SkASSERT(field.fChars[0] == '.');
            ++field.fChars;
            --field.fLength;
            for (size_t i = 0; i < field.fLength; ++i) {
                if (field.fChars[i] != '0' && field.fChars[i] != '1') {
                    this->error(next, "invalid swizzle");
                    return ASTNode::ID::Invalid();
                }
            }
            // use the next *raw* token so we don't ignore whitespace - we only care about
            // identifiers that directly follow the float
            Token id = this->nextRawToken();
            if (id.fKind == Token::Kind::TK_IDENTIFIER) {
                field.fLength += id.fLength;
            } else {
                this->pushback(id);
            }
            ASTNode::ID result = this->createNode(next.fOffset, ASTNode::Kind::kField, field);
            getNode(result).addChild(base);
            return result;
        }
        case Token::Kind::TK_LPAREN: {
            ASTNode::ID result = this->createNode(next.fOffset, ASTNode::Kind::kCall);
            getNode(result).addChild(base);
            if (this->peek().fKind != Token::Kind::TK_RPAREN) {
                for (;;) {
                    ASTNode::ID expr = this->assignmentExpression();
                    if (!expr) {
                        return ASTNode::ID::Invalid();
                    }
                    getNode(result).addChild(expr);
                    if (!this->checkNext(Token::Kind::TK_COMMA)) {
                        break;
                    }
                }
            }
            this->expect(Token::Kind::TK_RPAREN, "')' to complete function parameters");
            return result;
        }
        case Token::Kind::TK_PLUSPLUS: // fall through
        case Token::Kind::TK_MINUSMINUS: {
            ASTNode::ID result = this->createNode(next.fOffset, ASTNode::Kind::kPostfix,
                                                  Operator(next.fKind));
            getNode(result).addChild(base);
            return result;
        }
        default: {
            this->error(next,  "expected expression suffix, but found '" + this->text(next) + "'");
            return ASTNode::ID::Invalid();
        }
    }
}

/* IDENTIFIER | intLiteral | floatLiteral | boolLiteral | '(' expression ')' */
ASTNode::ID Parser::term() {
    Token t = this->peek();
    switch (t.fKind) {
        case Token::Kind::TK_IDENTIFIER: {
            StringFragment text;
            if (this->identifier(&text)) {
                return this->createNode(t.fOffset, ASTNode::Kind::kIdentifier, std::move(text));
            }
            break;
        }
        case Token::Kind::TK_INT_LITERAL: {
            SKSL_INT i;
            if (this->intLiteral(&i)) {
                return this->createNode(t.fOffset, ASTNode::Kind::kInt, i);
            }
            break;
        }
        case Token::Kind::TK_FLOAT_LITERAL: {
            SKSL_FLOAT f;
            if (this->floatLiteral(&f)) {
                return this->createNode(t.fOffset, ASTNode::Kind::kFloat, f);
            }
            break;
        }
        case Token::Kind::TK_TRUE_LITERAL: // fall through
        case Token::Kind::TK_FALSE_LITERAL: {
            bool b;
            if (this->boolLiteral(&b)) {
                return this->createNode(t.fOffset, ASTNode::Kind::kBool, b);
            }
            break;
        }
        case Token::Kind::TK_LPAREN: {
            this->nextToken();
            AutoDepth depth(this);
            if (!depth.increase()) {
                return ASTNode::ID::Invalid();
            }
            ASTNode::ID result = this->expression();
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
    return ASTNode::ID::Invalid();
}

/* INT_LITERAL */
bool Parser::intLiteral(SKSL_INT* dest) {
    Token t;
    if (!this->expect(Token::Kind::TK_INT_LITERAL, "integer literal", &t)) {
        return false;
    }
    StringFragment s = this->text(t);
    if (!SkSL::stoi(s, dest)) {
        this->error(t, "integer is too large: " + s);
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
    StringFragment s = this->text(t);
    if (!SkSL::stod(s, dest)) {
        this->error(t, "floating-point value is too large: " + s);
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
            this->error(t, "expected 'true' or 'false', but found '" + this->text(t) + "'");
            return false;
    }
}

/* IDENTIFIER */
bool Parser::identifier(StringFragment* dest) {
    Token t;
    if (this->expect(Token::Kind::TK_IDENTIFIER, "identifier", &t)) {
        *dest = this->text(t);
        return true;
    }
    return false;
}

}  // namespace SkSL
