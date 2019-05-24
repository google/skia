/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "stdio.h"
#include "src/sksl/SkSLParser.h"
#include "src/sksl/SkSLASTNode.h"
#include "src/sksl/ir/SkSLModifiers.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"

#ifndef SKSL_STANDALONE
#include "include/private/SkOnce.h"
#endif

namespace SkSL {

#define MAX_PARSE_DEPTH 50

class AutoDepth {
public:
    AutoDepth(Parser* p)
    : fParser(p) {
        fParser->fDepth++;
    }

    ~AutoDepth() {
        fParser->fDepth--;
    }

    bool checkValid() {
        if (fParser->fDepth > MAX_PARSE_DEPTH) {
            fParser->error(fParser->peek(), String("exceeded max parse depth"));
            return false;
        }
        return true;
    }

private:
    Parser* fParser;
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
    TOKEN(BLEND_SUPPORT_ALL_EQUATIONS,  "blend_support_all_equations");
    TOKEN(BLEND_SUPPORT_MULTIPLY,       "blend_support_multiply");
    TOKEN(BLEND_SUPPORT_SCREEN,         "blend_support_screen");
    TOKEN(BLEND_SUPPORT_OVERLAY,        "blend_support_overlay");
    TOKEN(BLEND_SUPPORT_DARKEN,         "blend_support_darken");
    TOKEN(BLEND_SUPPORT_LIGHTEN,        "blend_support_lighten");
    TOKEN(BLEND_SUPPORT_COLORDODGE,     "blend_support_colordodge");
    TOKEN(BLEND_SUPPORT_COLORBURN,      "blend_support_colorburn");
    TOKEN(BLEND_SUPPORT_HARDLIGHT,      "blend_support_hardlight");
    TOKEN(BLEND_SUPPORT_SOFTLIGHT,      "blend_support_softlight");
    TOKEN(BLEND_SUPPORT_DIFFERENCE,     "blend_support_difference");
    TOKEN(BLEND_SUPPORT_EXCLUSION,      "blend_support_exclusion");
    TOKEN(BLEND_SUPPORT_HSL_HUE,        "blend_support_hsl_hue");
    TOKEN(BLEND_SUPPORT_HSL_SATURATION, "blend_support_hsl_saturation");
    TOKEN(BLEND_SUPPORT_HSL_COLOR,      "blend_support_hsl_color");
    TOKEN(BLEND_SUPPORT_HSL_LUMINOSITY, "blend_support_hsl_luminosity");
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
    TOKEN(WHEN,                         "when");
    TOKEN(KEY,                          "key");
    TOKEN(TRACKED,                      "tracked");
    TOKEN(CTYPE,                        "ctype");
    TOKEN(SKPMCOLOR4F,                  "SkPMColor4f");
    TOKEN(SKRECT,                       "SkRect");
    TOKEN(SKIRECT,                      "SkIRect");
    TOKEN(SKPMCOLOR,                    "SkPMColor");
    TOKEN(BOOL,                         "bool");
    TOKEN(INT,                          "int");
    TOKEN(FLOAT,                        "float");
    #undef TOKEN
}

Parser::Parser(const char* text, size_t length, SymbolTable& types, ErrorReporter& errors)
: fText(text)
, fPushback(Token::INVALID, -1, -1)
, fTypes(types)
, fErrors(errors) {
    fLexer.start(text, length);
    static const bool layoutMapInitialized = []{ return (void)InitLayoutMap(), true; }();
    (void) layoutMapInitialized;
}

/* (directive | section | declaration)* END_OF_FILE */
std::vector<ASTNode> Parser::file() {
    std::vector<ASTNode> result;
    for (;;) {
        switch (this->peek().fKind) {
            case Token::END_OF_FILE:
                return result;
            case Token::DIRECTIVE: {
                ASTNode decl = this->directive();
                if (decl) {
                    result.push_back(std::move(decl));
                }
                break;
            }
            case Token::SECTION: {
                ASTNode section = this->section();
                if (section) {
                    result.push_back(std::move(section));
                }
                break;
            }
            default: {
                ASTNode decl = this->declaration();
                if (decl) {
                    result.push_back(std::move(decl));
                }
            }
        }
    }
}

Token Parser::nextRawToken() {
    if (fPushback.fKind != Token::INVALID) {
        Token result = fPushback;
        fPushback.fKind = Token::INVALID;
        return result;
    }
    Token result = fLexer.next();
    return result;
}

Token Parser::nextToken() {
    Token token = this->nextRawToken();
    while (token.fKind == Token::WHITESPACE || token.fKind == Token::LINE_COMMENT ||
           token.fKind == Token::BLOCK_COMMENT) {
        token = this->nextRawToken();
    }
    return token;
}

void Parser::pushback(Token t) {
    SkASSERT(fPushback.fKind == Token::INVALID);
    fPushback = std::move(t);
}

Token Parser::peek() {
    if (fPushback.fKind == Token::INVALID) {
        fPushback = this->nextToken();
    }
    return fPushback;
}

bool Parser::checkNext(Token::Kind kind, Token* result) {
    if (fPushback.fKind != Token::INVALID && fPushback.fKind != kind) {
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
    return nullptr != fTypes[name];
}

/* DIRECTIVE(#version) INT_LITERAL ("es" | "compatibility")? |
   DIRECTIVE(#extension) IDENTIFIER COLON IDENTIFIER */
ASTNode Parser::directive() {
    Token start;
    if (!this->expect(Token::DIRECTIVE, "a directive", &start)) {
        return ASTNode();
    }
    StringFragment text = this->text(start);
    if (text == "#version") {
        this->expect(Token::INT_LITERAL, "a version number");
        Token next = this->peek();
        StringFragment nextText = this->text(next);
        if (nextText == "es" || nextText == "compatibility") {
            this->nextToken();
        }
        // version is ignored for now; it will eventually become an error when we stop pretending
        // to be GLSL
        return ASTNode();
    } else if (text == "#extension") {
        Token name;
        if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
            return ASTNode();
        }
        if (!this->expect(Token::COLON, "':'")) {
            return ASTNode();
        }
        // FIXME: need to start paying attention to this token
        if (!this->expect(Token::IDENTIFIER, "an identifier")) {
            return ASTNode();
        }
        return ASTNode(start.fOffset, ASTNode::Kind::kExtension, this->text(name));
    } else {
        this->error(start, "unsupported directive '" + this->text(start) + "'");
        return ASTNode();
    }
}

/* SECTION LBRACE (LPAREN IDENTIFIER RPAREN)? <any sequence of tokens with balanced braces>
   RBRACE */
ASTNode Parser::section() {
    Token start;
    if (!this->expect(Token::SECTION, "a section token", &start)) {
        return ASTNode();
    }
    StringFragment argument;
    if (this->peek().fKind == Token::LPAREN) {
        this->nextToken();
        Token argToken;
        if (!this->expect(Token::IDENTIFIER, "an identifier", &argToken)) {
            return ASTNode();
        }
        argument = this->text(argToken);
        if (!this->expect(Token::RPAREN, "')'")) {
            return ASTNode();
        }
    }
    if (!this->expect(Token::LBRACE, "'{'")) {
        return ASTNode();
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
            case Token::LBRACE:
                ++level;
                break;
            case Token::RBRACE:
                --level;
                break;
            case Token::END_OF_FILE:
                this->error(start, "reached end of file while parsing section");
                return ASTNode();
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
    return ASTNode(start.fOffset, ASTNode::Kind::kSection,
                   (ASTNode::SectionData) { name, argument, text });
}

/* ENUM CLASS IDENTIFIER LBRACE (IDENTIFIER (EQ expression)? (COMMA IDENTIFIER (EQ expression))*)?
   RBRACE */
ASTNode Parser::enumDeclaration() {
    Token start;
    if (!this->expect(Token::ENUM, "'enum'", &start)) {
        return ASTNode();
    }
    if (!this->expect(Token::CLASS, "'class'")) {
        return ASTNode();
    }
    Token name;
    if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
        return ASTNode();
    }
    if (!this->expect(Token::LBRACE, "'{'")) {
        return ASTNode();
    }
    fTypes.add(this->text(name), std::unique_ptr<Symbol>(new Type(this->text(name),
                                                                  Type::kEnum_Kind)));
    std::vector<ASTNode> children;
    if (!this->checkNext(Token::RBRACE)) {
        Token id;
        if (!this->expect(Token::IDENTIFIER, "an identifier", &id)) {
            return ASTNode();
        }
        if (this->checkNext(Token::EQ)) {
            ASTNode value = this->assignmentExpression();
            if (!value) {
                return ASTNode();
            }
            children.emplace_back(id.fOffset, ASTNode::Kind::kEnumCase, text(id),
                                  (std::vector<ASTNode>) { std::move(value) });
        } else {
            children.emplace_back(id.fOffset, ASTNode::Kind::kEnumCase, text(id));
        }
        while (!this->checkNext(Token::RBRACE)) {
            if (!this->expect(Token::COMMA, "','")) {
                return ASTNode();
            }
            if (!this->expect(Token::IDENTIFIER, "an identifier", &id)) {
                return ASTNode();
            }
            if (this->checkNext(Token::EQ)) {
                ASTNode value = this->assignmentExpression();
                if (!value) {
                    return ASTNode();
                }
                children.emplace_back(id.fOffset, ASTNode::Kind::kEnumCase, text(id),
                                      (std::vector<ASTNode>) { std::move(value) });
            } else {
                children.emplace_back(id.fOffset, ASTNode::Kind::kEnumCase, text(id));
            }
        }
    }
    this->expect(Token::SEMICOLON, "';'");
    return ASTNode(name.fOffset, ASTNode::Kind::kEnum, this->text(name), std::move(children));
    abort();
}

/* enumDeclaration | modifiers (structVarDeclaration | type IDENTIFIER ((LPAREN parameter
   (COMMA parameter)* RPAREN (block | SEMICOLON)) | SEMICOLON) | interfaceBlock) */
ASTNode Parser::declaration() {
    Token lookahead = this->peek();
    if (lookahead.fKind == Token::ENUM) {
        return this->enumDeclaration();
    }
    Modifiers modifiers = this->modifiers();
    lookahead = this->peek();
    if (lookahead.fKind == Token::IDENTIFIER && !this->isType(this->text(lookahead))) {
        // we have an identifier that's not a type, could be the start of an interface block
        return this->interfaceBlock(modifiers);
    }
    if (lookahead.fKind == Token::STRUCT) {
        return this->structVarDeclaration(modifiers);
    }
    if (lookahead.fKind == Token::SEMICOLON) {
        this->nextToken();
        return ASTNode(lookahead.fOffset, ASTNode::Kind::kModifiers, modifiers);
    }
    ASTNode type = this->type();
    if (!type) {
        return ASTNode();
    }
    if (type.getTypeData().fIsStructDeclaration && this->checkNext(Token::SEMICOLON)) {
        return ASTNode();
    }
    Token name;
    if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
        return ASTNode();
    }
    if (this->checkNext(Token::LPAREN)) {
        std::vector<ASTNode> parameters;
        while (this->peek().fKind != Token::RPAREN) {
            if (parameters.size() > 0) {
                if (!this->expect(Token::COMMA, "','")) {
                    return ASTNode();
                }
            }
            ASTNode parameter = this->parameter();
            if (!parameter) {
                return ASTNode();
            }
            parameters.push_back(std::move(parameter));
        }
        this->nextToken();
        ASTNode body;
        if (!this->checkNext(Token::SEMICOLON)) {
            body = this->block();
            if (!body) {
                return ASTNode();
            }
        }

        size_t parameterCount = parameters.size();
        std::vector<ASTNode> children;
        children.push_back(std::move(type));
        for (auto& p : parameters) {
            children.push_back(std::move(p));
        }
        if (body) {
            children.push_back(std::move(body));
        }
        return ASTNode(name.fOffset, ASTNode::Kind::kFunction,
                       (ASTNode::FunctionData) { modifiers, this->text(name), parameterCount },
                       std::move(children));
    } else {
        return this->varDeclarationEnd(modifiers, std::move(type), this->text(name));
    }
}

/* modifiers type IDENTIFIER varDeclarationEnd */
ASTNode Parser::varDeclarations() {
    Modifiers modifiers = this->modifiers();
    ASTNode type(this->type());
    if (!type) {
        return ASTNode();
    }
    Token name;
    if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
        return ASTNode();
    }
    return this->varDeclarationEnd(modifiers, std::move(type), this->text(name));
}

/* STRUCT IDENTIFIER LBRACE varDeclaration* RBRACE */
ASTNode Parser::structDeclaration() {
    if (!this->expect(Token::STRUCT, "'struct'")) {
        return ASTNode();
    }
    Token name;
    if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
        return ASTNode();
    }
    if (!this->expect(Token::LBRACE, "'{'")) {
        return ASTNode();
    }
    std::vector<Type::Field> fields;
    while (this->peek().fKind != Token::RBRACE) {
        ASTNode decls = this->varDeclarations();
        if (!decls) {
            return ASTNode();
        }
        for (size_t i = 2; i < decls.fChildren.size(); ++i) {
            ASTNode& var = decls.fChildren[i];
            ASTNode::VarData vd = var.getVarData();
            auto type = (const Type*) fTypes[decls.fChildren[1].getTypeData().fName];
            for (int j = vd.fSizeCount - 1; j >= 0; j--) {
                if (!var.fChildren[j] || var.fChildren[j].fKind != ASTNode::Kind::kInt) {
                    this->error(decls.fOffset, "array size in struct field must be a constant");
                    return ASTNode();
                }
                uint64_t columns = var.fChildren[j].getInt();
                String name = type->name() + "[" + to_string(columns) + "]";
                type = (Type*) fTypes.takeOwnership(std::unique_ptr<Symbol>(
                                                                         new Type(name,
                                                                                  Type::kArray_Kind,
                                                                                  *type,
                                                                                  (int) columns)));
            }
            fields.push_back(Type::Field(decls.fChildren[0].getModifiers(), vd.fName, type));
            if (var.fChildren.size() > vd.fSizeCount) {
                this->error(decls.fOffset, "initializers are not permitted on struct fields");
            }
        }
    }
    if (!this->expect(Token::RBRACE, "'}'")) {
        return ASTNode();
    }
    fTypes.add(this->text(name), std::unique_ptr<Type>(new Type(name.fOffset, this->text(name),
                                                                fields)));
    return ASTNode(name.fOffset, ASTNode::Kind::kType,
                   (ASTNode::TypeData) { this->text(name), true, false });
}

/* structDeclaration ((IDENTIFIER varDeclarationEnd) | SEMICOLON) */
ASTNode Parser::structVarDeclaration(Modifiers modifiers) {
    ASTNode type = this->structDeclaration();
    if (!type) {
        return ASTNode();
    }
    Token name;
    if (this->checkNext(Token::IDENTIFIER, &name)) {
        return this->varDeclarationEnd(modifiers, std::move(type), this->text(name));
    }
    this->expect(Token::SEMICOLON, "';'");
    return ASTNode();
}

/* (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)? (COMMA IDENTIFER
   (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)?)* SEMICOLON */
ASTNode Parser::varDeclarationEnd(Modifiers mods, ASTNode type, StringFragment name) {
    std::vector<ASTNode> children;
    children.emplace_back(-1, ASTNode::Kind::kModifiers, mods);
    children.push_back(std::move(type));
    std::vector<ASTNode> currentVarChildren;
    while (this->checkNext(Token::LBRACKET)) {
        if (this->checkNext(Token::RBRACKET)) {
            currentVarChildren.push_back(ASTNode());
        } else {
            ASTNode size = this->expression();
            if (!size) {
                return ASTNode();
            }
            currentVarChildren.push_back(std::move(size));
            if (!this->expect(Token::RBRACKET, "']'")) {
                return ASTNode();
            }
        }
    }
    size_t sizeCount = currentVarChildren.size();
    if (this->checkNext(Token::EQ)) {
        ASTNode value = this->assignmentExpression();
        if (!value) {
            return ASTNode();
        }
        currentVarChildren.push_back(std::move(value));
    }
    children.emplace_back(-1, ASTNode::Kind::kVarDeclaration,
                          (ASTNode::VarData) { name, sizeCount },
                          std::move(currentVarChildren));
    while (this->checkNext(Token::COMMA)) {
        Token name;
        if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
            return ASTNode();
        }
        currentVarChildren.clear();
        while (this->checkNext(Token::LBRACKET)) {
            if (this->checkNext(Token::RBRACKET)) {
                currentVarChildren.push_back(ASTNode());
            } else {
                ASTNode size = this->expression();
                if (!size) {
                    return ASTNode();
                }
                currentVarChildren.push_back(std::move(size));
                if (!this->expect(Token::RBRACKET, "']'")) {
                    return ASTNode();
                }
            }
        }
        sizeCount = currentVarChildren.size();
        if (this->checkNext(Token::EQ)) {
            ASTNode value = this->assignmentExpression();
            if (!value) {
                return ASTNode();
            }
            currentVarChildren.push_back(std::move(value));
        }
        children.emplace_back(-1, ASTNode::Kind::kVarDeclaration,
                              (ASTNode::VarData) { text(name), sizeCount },
                              std::move(currentVarChildren));
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return ASTNode();
    }
    return ASTNode(-1, ASTNode::Kind::kVarDeclarations, std::move(children));
}

/* modifiers type IDENTIFIER (LBRACKET INT_LITERAL RBRACKET)? */
ASTNode Parser::parameter() {
    Modifiers modifiers = this->modifiersWithDefaults(0);
    ASTNode type = this->type();
    if (!type) {
        return ASTNode();
    }
    std::vector<ASTNode> children;
    children.push_back(std::move(type));
    Token name;
    if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
        return ASTNode();
    }
    while (this->checkNext(Token::LBRACKET)) {
        Token sizeToken;
        if (!this->expect(Token::INT_LITERAL, "a positive integer", &sizeToken)) {
            return ASTNode();
        }
        children.emplace_back(sizeToken.fOffset, ASTNode::Kind::kInt,
                              SkSL::stoi(this->text(sizeToken)));
        if (!this->expect(Token::RBRACKET, "']'")) {
            return ASTNode();
        }
    }
    size_t sizeCount = children.size() - 1;
    return ASTNode(name.fOffset, ASTNode::Kind::kParameter,
                   (ASTNode::ParameterData) { modifiers, text(name), sizeCount },
                   std::move(children));
}

/** EQ INT_LITERAL */
int Parser::layoutInt() {
    if (!this->expect(Token::EQ, "'='")) {
        return -1;
    }
    Token resultToken;
    if (this->expect(Token::INT_LITERAL, "a non-negative integer", &resultToken)) {
        return SkSL::stoi(this->text(resultToken));
    }
    return -1;
}

/** EQ IDENTIFIER */
StringFragment Parser::layoutIdentifier() {
    if (!this->expect(Token::EQ, "'='")) {
        return StringFragment();
    }
    Token resultToken;
    if (!this->expect(Token::IDENTIFIER, "an identifier", &resultToken)) {
        return StringFragment();
    }
    return this->text(resultToken);
}


/** EQ <any sequence of tokens with balanced parentheses and no top-level comma> */
StringFragment Parser::layoutCode() {
    if (!this->expect(Token::EQ, "'='")) {
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
            case Token::LPAREN:
                ++level;
                break;
            case Token::RPAREN:
                --level;
                break;
            case Token::COMMA:
                if (level == 1) {
                    done = true;
                }
                break;
            case Token::END_OF_FILE:
                this->error(start, "reached end of file while parsing layout");
                return nullptr;
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
    printf("CODE: %s\n", String(code).c_str());
    return code;
}

/** (EQ IDENTIFIER('identity'))? */
Layout::Key Parser::layoutKey() {
    if (this->peek().fKind == Token::EQ) {
        this->expect(Token::EQ, "'='");
        Token key;
        if (this->expect(Token::IDENTIFIER, "an identifer", &key)) {
            if (this->text(key) == "identity") {
                return Layout::kIdentity_Key;
            } else {
                this->error(key, "unsupported layout key");
            }
        }
    }
    return Layout::kKey_Key;
}

Layout::CType Parser::layoutCType() {
    if (this->expect(Token::EQ, "'='")) {
        Token t = this->nextToken();
        String text = this->text(t);
        auto found = layoutTokens->find(text);
        if (found != layoutTokens->end()) {
            switch (found->second) {
                case LayoutToken::SKPMCOLOR4F:
                    return Layout::CType::kSkPMColor4f;
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
    Layout::Format format = Layout::Format::kUnspecified;
    Layout::Primitive primitive = Layout::kUnspecified_Primitive;
    int maxVertices = -1;
    int invocations = -1;
    StringFragment when;
    Layout::Key key = Layout::kNo_Key;
    Layout::CType ctype = Layout::CType::kDefault;
    if (this->checkNext(Token::LAYOUT)) {
        if (!this->expect(Token::LPAREN, "'('")) {
            return Layout(flags, location, offset, binding, index, set, builtin,
                          inputAttachmentIndex, format, primitive, maxVertices, invocations, when,
                          key, ctype);
        }
        for (;;) {
            Token t = this->nextToken();
            String text = this->text(t);
            auto found = layoutTokens->find(text);
            if (found != layoutTokens->end()) {
                switch (found->second) {
                    case LayoutToken::LOCATION:
                        location = this->layoutInt();
                        break;
                    case LayoutToken::OFFSET:
                        offset = this->layoutInt();
                        break;
                    case LayoutToken::BINDING:
                        binding = this->layoutInt();
                        break;
                    case LayoutToken::INDEX:
                        index = this->layoutInt();
                        break;
                    case LayoutToken::SET:
                        set = this->layoutInt();
                        break;
                    case LayoutToken::BUILTIN:
                        builtin = this->layoutInt();
                        break;
                    case LayoutToken::INPUT_ATTACHMENT_INDEX:
                        inputAttachmentIndex = this->layoutInt();
                        break;
                    case LayoutToken::ORIGIN_UPPER_LEFT:
                        flags |= Layout::kOriginUpperLeft_Flag;
                        break;
                    case LayoutToken::OVERRIDE_COVERAGE:
                        flags |= Layout::kOverrideCoverage_Flag;
                        break;
                    case LayoutToken::BLEND_SUPPORT_ALL_EQUATIONS:
                        flags |= Layout::kBlendSupportAllEquations_Flag;
                        break;
                    case LayoutToken::BLEND_SUPPORT_MULTIPLY:
                        flags |= Layout::kBlendSupportMultiply_Flag;
                        break;
                    case LayoutToken::BLEND_SUPPORT_SCREEN:
                        flags |= Layout::kBlendSupportScreen_Flag;
                        break;
                    case LayoutToken::BLEND_SUPPORT_OVERLAY:
                        flags |= Layout::kBlendSupportOverlay_Flag;
                        break;
                    case LayoutToken::BLEND_SUPPORT_DARKEN:
                        flags |= Layout::kBlendSupportDarken_Flag;
                        break;
                    case LayoutToken::BLEND_SUPPORT_LIGHTEN:
                        flags |= Layout::kBlendSupportLighten_Flag;
                        break;
                    case LayoutToken::BLEND_SUPPORT_COLORDODGE:
                        flags |= Layout::kBlendSupportColorDodge_Flag;
                        break;
                    case LayoutToken::BLEND_SUPPORT_COLORBURN:
                        flags |= Layout::kBlendSupportColorBurn_Flag;
                        break;
                    case LayoutToken::BLEND_SUPPORT_HARDLIGHT:
                        flags |= Layout::kBlendSupportHardLight_Flag;
                        break;
                    case LayoutToken::BLEND_SUPPORT_SOFTLIGHT:
                        flags |= Layout::kBlendSupportSoftLight_Flag;
                        break;
                    case LayoutToken::BLEND_SUPPORT_DIFFERENCE:
                        flags |= Layout::kBlendSupportDifference_Flag;
                        break;
                    case LayoutToken::BLEND_SUPPORT_EXCLUSION:
                        flags |= Layout::kBlendSupportExclusion_Flag;
                        break;
                    case LayoutToken::BLEND_SUPPORT_HSL_HUE:
                        flags |= Layout::kBlendSupportHSLHue_Flag;
                        break;
                    case LayoutToken::BLEND_SUPPORT_HSL_SATURATION:
                        flags |= Layout::kBlendSupportHSLSaturation_Flag;
                        break;
                    case LayoutToken::BLEND_SUPPORT_HSL_COLOR:
                        flags |= Layout::kBlendSupportHSLColor_Flag;
                        break;
                    case LayoutToken::BLEND_SUPPORT_HSL_LUMINOSITY:
                        flags |= Layout::kBlendSupportHSLLuminosity_Flag;
                        break;
                    case LayoutToken::PUSH_CONSTANT:
                        flags |= Layout::kPushConstant_Flag;
                        break;
                    case LayoutToken::TRACKED:
                        flags |= Layout::kTracked_Flag;
                        break;
                    case LayoutToken::POINTS:
                        primitive = Layout::kPoints_Primitive;
                        break;
                    case LayoutToken::LINES:
                        primitive = Layout::kLines_Primitive;
                        break;
                    case LayoutToken::LINE_STRIP:
                        primitive = Layout::kLineStrip_Primitive;
                        break;
                    case LayoutToken::LINES_ADJACENCY:
                        primitive = Layout::kLinesAdjacency_Primitive;
                        break;
                    case LayoutToken::TRIANGLES:
                        primitive = Layout::kTriangles_Primitive;
                        break;
                    case LayoutToken::TRIANGLE_STRIP:
                        primitive = Layout::kTriangleStrip_Primitive;
                        break;
                    case LayoutToken::TRIANGLES_ADJACENCY:
                        primitive = Layout::kTrianglesAdjacency_Primitive;
                        break;
                    case LayoutToken::MAX_VERTICES:
                        maxVertices = this->layoutInt();
                        break;
                    case LayoutToken::INVOCATIONS:
                        invocations = this->layoutInt();
                        break;
                    case LayoutToken::WHEN:
                        when = this->layoutCode();
                        break;
                    case LayoutToken::KEY:
                        key = this->layoutKey();
                        break;
                    case LayoutToken::CTYPE:
                        ctype = this->layoutCType();
                        break;
                    default:
                        this->error(t, ("'" + text + "' is not a valid layout qualifier").c_str());
                        break;
                }
            } else if (Layout::ReadFormat(text, &format)) {
               // AST::ReadFormat stored the result in 'format'.
            } else {
                this->error(t, ("'" + text + "' is not a valid layout qualifier").c_str());
            }
            if (this->checkNext(Token::RPAREN)) {
                break;
            }
            if (!this->expect(Token::COMMA, "','")) {
                break;
            }
        }
    }
    return Layout(flags, location, offset, binding, index, set, builtin, inputAttachmentIndex,
                  format, primitive, maxVertices, invocations, when, key, ctype);
}

/* layout? (UNIFORM | CONST | IN | OUT | INOUT | LOWP | MEDIUMP | HIGHP | FLAT | NOPERSPECTIVE |
            READONLY | WRITEONLY | COHERENT | VOLATILE | RESTRICT | BUFFER | PLS | PLSIN |
            PLSOUT)* */
Modifiers Parser::modifiers() {
    Layout layout = this->layout();
    int flags = 0;
    for (;;) {
        // TODO: handle duplicate / incompatible flags
        switch (peek().fKind) {
            case Token::UNIFORM:
                this->nextToken();
                flags |= Modifiers::kUniform_Flag;
                break;
            case Token::CONST:
                this->nextToken();
                flags |= Modifiers::kConst_Flag;
                break;
            case Token::IN:
                this->nextToken();
                flags |= Modifiers::kIn_Flag;
                break;
            case Token::OUT:
                this->nextToken();
                flags |= Modifiers::kOut_Flag;
                break;
            case Token::INOUT:
                this->nextToken();
                flags |= Modifiers::kIn_Flag;
                flags |= Modifiers::kOut_Flag;
                break;
            case Token::FLAT:
                this->nextToken();
                flags |= Modifiers::kFlat_Flag;
                break;
            case Token::NOPERSPECTIVE:
                this->nextToken();
                flags |= Modifiers::kNoPerspective_Flag;
                break;
            case Token::READONLY:
                this->nextToken();
                flags |= Modifiers::kReadOnly_Flag;
                break;
            case Token::WRITEONLY:
                this->nextToken();
                flags |= Modifiers::kWriteOnly_Flag;
                break;
            case Token::COHERENT:
                this->nextToken();
                flags |= Modifiers::kCoherent_Flag;
                break;
            case Token::VOLATILE:
                this->nextToken();
                flags |= Modifiers::kVolatile_Flag;
                break;
            case Token::RESTRICT:
                this->nextToken();
                flags |= Modifiers::kRestrict_Flag;
                break;
            case Token::BUFFER:
                this->nextToken();
                flags |= Modifiers::kBuffer_Flag;
                break;
            case Token::HASSIDEEFFECTS:
                this->nextToken();
                flags |= Modifiers::kHasSideEffects_Flag;
                break;
            case Token::PLS:
                this->nextToken();
                flags |= Modifiers::kPLS_Flag;
                break;
            case Token::PLSIN:
                this->nextToken();
                flags |= Modifiers::kPLSIn_Flag;
                break;
            case Token::PLSOUT:
                this->nextToken();
                flags |= Modifiers::kPLSOut_Flag;
                break;
            default:
                return Modifiers(layout, flags);
        }
    }
}

Modifiers Parser::modifiersWithDefaults(int defaultFlags) {
    Modifiers result = this->modifiers();
    if (!result.fFlags) {
        return Modifiers(result.fLayout, defaultFlags);
    }
    return result;
}

/* ifStatement | forStatement | doStatement | whileStatement | block | expression */
ASTNode Parser::statement() {
    Token start = this->peek();
    switch (start.fKind) {
        case Token::IF: // fall through
        case Token::STATIC_IF:
            return this->ifStatement();
        case Token::FOR:
            return this->forStatement();
        case Token::DO:
            return this->doStatement();
        case Token::WHILE:
            return this->whileStatement();
        case Token::SWITCH: // fall through
        case Token::STATIC_SWITCH:
            return this->switchStatement();
        case Token::RETURN:
            return this->returnStatement();
        case Token::BREAK:
            return this->breakStatement();
        case Token::CONTINUE:
            return this->continueStatement();
        case Token::DISCARD:
            return this->discardStatement();
        case Token::LBRACE:
            return this->block();
        case Token::SEMICOLON:
            this->nextToken();
            return ASTNode(start.fOffset, ASTNode::Kind::kBlock, (std::vector<ASTNode>){});
        case Token::CONST:
            return this->varDeclarations();
        case Token::IDENTIFIER:
            if (this->isType(this->text(start))) {
                return this->varDeclarations();
            }
            // fall through
        default:
            return this->expressionStatement();
    }
}

/* IDENTIFIER(type) (LBRACKET intLiteral? RBRACKET)* QUESTION? */
ASTNode Parser::type() {
    Token type;
    if (!this->expect(Token::IDENTIFIER, "a type", &type)) {
        return ASTNode();
    }
    if (!this->isType(this->text(type))) {
        this->error(type, ("no type named '" + this->text(type) + "'").c_str());
        return ASTNode();
    }
    std::vector<ASTNode> sizes;
    while (this->checkNext(Token::LBRACKET)) {
        if (this->peek().fKind != Token::RBRACKET) {
            SKSL_INT i;
            if (this->intLiteral(&i)) {
                sizes.emplace_back(-1, ASTNode::Kind::kInt, i);
            } else {
                return ASTNode();
            }
        } else {
            sizes.emplace_back();
        }
        this->expect(Token::RBRACKET, "']'");
    }
    bool nullable = this->checkNext(Token::QUESTION);
    return ASTNode(type.fOffset, ASTNode::Kind::kType,
                   (ASTNode::TypeData) { this->text(type), false, nullable },
                   std::move(sizes));
}

/* IDENTIFIER LBRACE varDeclaration* RBRACE (IDENTIFIER (LBRACKET expression? RBRACKET)*)? */
ASTNode Parser::interfaceBlock(Modifiers mods) {
    Token name;
    if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
        return ASTNode();
    }
    if (peek().fKind != Token::LBRACE) {
        // we only get into interfaceBlock if we found a top-level identifier which was not a type.
        // 99% of the time, the user was not actually intending to create an interface block, so
        // it's better to report it as an unknown type
        this->error(name, "no type named '" + this->text(name) + "'");
        return ASTNode();
    }
    this->nextToken();
    std::vector<ASTNode> decls;
    while (this->peek().fKind != Token::RBRACE) {
        ASTNode decl = this->varDeclarations();
        if (!decl) {
            return ASTNode();
        }
        decls.push_back(std::move(decl));
    }
    this->nextToken();
    std::vector<ASTNode> sizes;
    StringFragment instanceName;
    Token instanceNameToken;
    if (this->checkNext(Token::IDENTIFIER, &instanceNameToken)) {
        while (this->checkNext(Token::LBRACKET)) {
            if (this->peek().fKind != Token::RBRACKET) {
                ASTNode size = this->expression();
                if (!size) {
                    return ASTNode();
                }
                sizes.push_back(std::move(size));
            } else {
                sizes.push_back(ASTNode());
            }
            this->expect(Token::RBRACKET, "']'");
        }
        instanceName = this->text(instanceNameToken);
    }
    this->expect(Token::SEMICOLON, "';'");
    std::vector<ASTNode> children;
    size_t declarationCount = decls.size();
    for (auto& decl : decls) {
        children.push_back(std::move(decl));
    }
    size_t sizeCount = sizes.size();
    for (auto& size : sizes) {
        children.push_back(std::move(size));
    }
    return ASTNode(name.fOffset, ASTNode::Kind::kInterfaceBlock,
                   (ASTNode::InterfaceBlockData) { mods, this->text(name), declarationCount,
                                                   instanceName, sizeCount },
                   std::move(children));
}

/* IF LPAREN expression RPAREN statement (ELSE statement)? */
ASTNode Parser::ifStatement() {
    Token start;
    bool isStatic = this->checkNext(Token::STATIC_IF, &start);
    if (!isStatic && !this->expect(Token::IF, "'if'", &start)) {
        return ASTNode();
    }
    if (!this->expect(Token::LPAREN, "'('")) {
        return ASTNode();
    }
    ASTNode test = this->expression();
    if (!test) {
        return ASTNode();
    }
    std::vector<ASTNode> children;
    children.push_back(std::move(test));
    if (!this->expect(Token::RPAREN, "')'")) {
        return ASTNode();
    }
    ASTNode ifTrue = this->statement();
    if (!ifTrue) {
        return ASTNode();
    }
    children.push_back(std::move(ifTrue));
    ASTNode ifFalse;
    if (this->checkNext(Token::ELSE)) {
        ifFalse = this->statement();
        if (!ifFalse) {
            return ASTNode();
        }
        children.push_back(std::move(ifFalse));
    }
    return ASTNode(start.fOffset, ASTNode::Kind::kIf, isStatic, std::move(children));
}

/* DO statement WHILE LPAREN expression RPAREN SEMICOLON */
ASTNode Parser::doStatement() {
    Token start;
    if (!this->expect(Token::DO, "'do'", &start)) {
        return ASTNode();
    }
    ASTNode statement(this->statement());
    if (!statement) {
        return ASTNode();
    }
    std::vector<ASTNode> children;
    children.push_back(std::move(statement));
    if (!this->expect(Token::WHILE, "'while'")) {
        return ASTNode();
    }
    if (!this->expect(Token::LPAREN, "'('")) {
        return ASTNode();
    }
    ASTNode test = this->expression();
    if (!test) {
        return ASTNode();
    }
    children.push_back(std::move(test));
    if (!this->expect(Token::RPAREN, "')'")) {
        return ASTNode();
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return ASTNode();
    }
    return ASTNode(start.fOffset, ASTNode::Kind::kDo, std::move(children));
}

/* WHILE LPAREN expression RPAREN STATEMENT */
ASTNode Parser::whileStatement() {
    Token start;
    if (!this->expect(Token::WHILE, "'while'", &start)) {
        return ASTNode();
    }
    if (!this->expect(Token::LPAREN, "'('")) {
        return ASTNode();
    }
    ASTNode test = this->expression();
    if (!test) {
        return ASTNode();
    }
    if (!this->expect(Token::RPAREN, "')'")) {
        return ASTNode();
    }
    ASTNode statement(this->statement());
    if (!statement) {
        return ASTNode();
    }
    std::vector<ASTNode> children;
    children.push_back(std::move(test));
    children.push_back(std::move(statement));
    return ASTNode(start.fOffset, ASTNode::Kind::kWhile, std::move(children));
}

/* CASE expression COLON statement* */
ASTNode Parser::switchCase() {
    Token start;
    if (!this->expect(Token::CASE, "'case'", &start)) {
        return ASTNode();
    }
    ASTNode value = this->expression();
    if (!value) {
        return ASTNode();
    }
    if (!this->expect(Token::COLON, "':'")) {
        return ASTNode();
    }
    std::vector<ASTNode> children;
    children.push_back(std::move(value));
    while (this->peek().fKind != Token::RBRACE && this->peek().fKind != Token::CASE &&
           this->peek().fKind != Token::DEFAULT) {
        ASTNode s = this->statement();
        if (!s) {
            return ASTNode();
        }
        children.push_back(std::move(s));
    }
    return ASTNode(start.fOffset, ASTNode::Kind::kSwitchCase, std::move(children));
}

/* SWITCH LPAREN expression RPAREN LBRACE switchCase* (DEFAULT COLON statement*)? RBRACE */
ASTNode Parser::switchStatement() {
    Token start;
    bool isStatic = this->checkNext(Token::STATIC_SWITCH, &start);
    if (!isStatic && !this->expect(Token::SWITCH, "'switch'", &start)) {
        return ASTNode();
    }
    if (!this->expect(Token::LPAREN, "'('")) {
        return ASTNode();
    }
    ASTNode value(this->expression());
    if (!value) {
        return ASTNode();
    }
    if (!this->expect(Token::RPAREN, "')'")) {
        return ASTNode();
    }
    if (!this->expect(Token::LBRACE, "'{'")) {
        return ASTNode();
    }
    std::vector<ASTNode> children;
    children.push_back(std::move(value));
    while (this->peek().fKind == Token::CASE) {
        ASTNode c = this->switchCase();
        if (!c) {
            return ASTNode();
        }
        children.push_back(std::move(c));
    }
    // Requiring default: to be last (in defiance of C and GLSL) was a deliberate decision. Other
    // parts of the compiler may rely upon this assumption.
    if (this->peek().fKind == Token::DEFAULT) {
        Token defaultStart;
        SkAssertResult(this->expect(Token::DEFAULT, "'default'", &defaultStart));
        if (!this->expect(Token::COLON, "':'")) {
            return ASTNode();
        }
        std::vector<ASTNode> defaultChildren;
        defaultChildren.emplace_back(); // empty test to signify default case
        while (this->peek().fKind != Token::RBRACE) {
            ASTNode s = this->statement();
            if (!s) {
                return ASTNode();
            }
            defaultChildren.push_back(std::move(s));
        }
        children.emplace_back(defaultStart.fOffset, ASTNode::Kind::kSwitchCase,
                std::move(defaultChildren));
    }
    if (!this->expect(Token::RBRACE, "'}'")) {
        return ASTNode();
    }
    return ASTNode(start.fOffset, ASTNode::Kind::kSwitch, isStatic, std::move(children));
}

/* FOR LPAREN (declaration | expression)? SEMICOLON expression? SEMICOLON expression? RPAREN
   STATEMENT */
ASTNode Parser::forStatement() {
    Token start;
    if (!this->expect(Token::FOR, "'for'", &start)) {
        return ASTNode();
    }
    if (!this->expect(Token::LPAREN, "'('")) {
        return ASTNode();
    }
    ASTNode initializer;
    Token nextToken = this->peek();
    switch (nextToken.fKind) {
        case Token::SEMICOLON:
            this->nextToken();
            break;
        case Token::CONST: {
            initializer = this->varDeclarations();
            if (!initializer) {
                return ASTNode();
            }
            break;
        }
        case Token::IDENTIFIER: {
            if (this->isType(this->text(nextToken))) {
                initializer = this->varDeclarations();
                if (!initializer) {
                    return ASTNode();
                }
                break;
            }
        } // fall through
        default:
            initializer = this->expressionStatement();
    }
    std::vector<ASTNode> children;
    children.push_back(std::move(initializer));
    ASTNode test;
    if (this->peek().fKind != Token::SEMICOLON) {
        test = this->expression();
        if (!test) {
            return ASTNode();
        }
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return ASTNode();
    }
    children.push_back(std::move(test));
    ASTNode next;
    if (this->peek().fKind != Token::RPAREN) {
        next = this->expression();
        if (!next) {
            return ASTNode();
        }
    }
    if (!this->expect(Token::RPAREN, "')'")) {
        return ASTNode();
    }
    children.push_back(std::move(next));
    ASTNode statement(this->statement());
    if (!statement) {
        return ASTNode();
    }
    children.push_back(std::move(statement));
    return ASTNode(start.fOffset, ASTNode::Kind::kFor, std::move(children));
}

/* RETURN expression? SEMICOLON */
ASTNode Parser::returnStatement() {
    Token start;
    if (!this->expect(Token::RETURN, "'return'", &start)) {
        return ASTNode();
    }
    std::vector<ASTNode> children;
    if (this->peek().fKind != Token::SEMICOLON) {
        ASTNode expression = this->expression();
        if (!expression) {
            return ASTNode();
        }
        children.push_back(std::move(expression));
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return ASTNode();
    }
    return ASTNode(start.fOffset, ASTNode::Kind::kReturn, std::move(children));
}

/* BREAK SEMICOLON */
ASTNode Parser::breakStatement() {
    Token start;
    if (!this->expect(Token::BREAK, "'break'", &start)) {
        return ASTNode();
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return ASTNode();
    }
    return ASTNode(start.fOffset, ASTNode::Kind::kBreak);
}

/* CONTINUE SEMICOLON */
ASTNode Parser::continueStatement() {
    Token start;
    if (!this->expect(Token::CONTINUE, "'continue'", &start)) {
        return ASTNode();
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return ASTNode();
    }
    return ASTNode(start.fOffset, ASTNode::Kind::kContinue);
}

/* DISCARD SEMICOLON */
ASTNode Parser::discardStatement() {
    Token start;
    if (!this->expect(Token::DISCARD, "'continue'", &start)) {
        return ASTNode();
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return ASTNode();
    }
    return ASTNode(start.fOffset, ASTNode::Kind::kDiscard);
}

/* LBRACE statement* RBRACE */
ASTNode Parser::block() {
    AutoDepth depth(this);
    if (!depth.checkValid()) {
        return ASTNode();
    }
    Token start;
    if (!this->expect(Token::LBRACE, "'{'", &start)) {
        return ASTNode();
    }
    std::vector<ASTNode> statements;
    for (;;) {
        switch (this->peek().fKind) {
            case Token::RBRACE:
                this->nextToken();
                return ASTNode(start.fOffset, ASTNode::Kind::kBlock, std::move(statements));
            case Token::END_OF_FILE:
                this->error(this->peek(), "expected '}', but found end of file");
                return ASTNode();
            default: {
                ASTNode statement = this->statement();
                if (!statement) {
                    return ASTNode();
                }
                statements.push_back(std::move(statement));
            }
        }
    }
}

/* expression SEMICOLON */
ASTNode Parser::expressionStatement() {
    ASTNode expr = this->expression();
    if (expr) {
        if (this->expect(Token::SEMICOLON, "';'")) {
            return expr;
        }
    }
    return ASTNode();
}

/* commaExpression */
ASTNode Parser::expression() {
    AutoDepth depth(this);
    if (!depth.checkValid()) {
        return ASTNode();
    }
    return this->commaExpression();
}

/* assignmentExpression (COMMA assignmentExpression)* */
ASTNode Parser::commaExpression() {
    ASTNode result = this->assignmentExpression();
    if (!result) {
        return ASTNode();
    }
    Token t;
    while (this->checkNext(Token::COMMA, &t)) {
        ASTNode right = this->commaExpression();
        if (!right) {
            return ASTNode();
        }
        result = ASTNode(t.fOffset, ASTNode::Kind::kBinary, std::move(t),
                    { std::move(result), std::move(right) });
    }
    return result;
}

/* ternaryExpression ((EQEQ | STAREQ | SLASHEQ | PERCENTEQ | PLUSEQ | MINUSEQ | SHLEQ | SHREQ |
   BITWISEANDEQ | BITWISEXOREQ | BITWISEOREQ | LOGICALANDEQ | LOGICALXOREQ | LOGICALOREQ)
   assignmentExpression)*
 */
ASTNode Parser::assignmentExpression() {
    ASTNode result = this->ternaryExpression();
    if (!result) {
        return ASTNode();
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::EQ:           // fall through
            case Token::STAREQ:       // fall through
            case Token::SLASHEQ:      // fall through
            case Token::PERCENTEQ:    // fall through
            case Token::PLUSEQ:       // fall through
            case Token::MINUSEQ:      // fall through
            case Token::SHLEQ:        // fall through
            case Token::SHREQ:        // fall through
            case Token::BITWISEANDEQ: // fall through
            case Token::BITWISEXOREQ: // fall through
            case Token::BITWISEOREQ:  // fall through
            case Token::LOGICALANDEQ: // fall through
            case Token::LOGICALXOREQ: // fall through
            case Token::LOGICALOREQ: {
                Token t = this->nextToken();
                ASTNode right = this->assignmentExpression();
                if (!right) {
                    return ASTNode();
                }
                result = ASTNode(t.fOffset, ASTNode::Kind::kBinary, std::move(t),
                                 { std::move(result), std::move(right) });
                return result;
            }
            default:
                return result;
        }
    }
}

/* logicalOrExpression ('?' expression ':' assignmentExpression)? */
ASTNode Parser::ternaryExpression() {
    ASTNode result = this->logicalOrExpression();
    if (!result) {
        return ASTNode();
    }
    if (this->checkNext(Token::QUESTION)) {
        ASTNode trueExpr = this->expression();
        if (!trueExpr) {
            return ASTNode();
        }
        if (this->expect(Token::COLON, "':'")) {
            ASTNode falseExpr = this->assignmentExpression();
            return ASTNode(result.fOffset,  ASTNode::Kind::kTernary,
                           { std::move(result), std::move(trueExpr), std::move(falseExpr) });
        }
        return ASTNode();
    }
    return result;
}

/* logicalXorExpression (LOGICALOR logicalXorExpression)* */
ASTNode Parser::logicalOrExpression() {
    ASTNode result = this->logicalXorExpression();
    if (!result) {
        return ASTNode();
    }
    Token t;
    while (this->checkNext(Token::LOGICALOR, &t)) {
        ASTNode right = this->logicalXorExpression();
        if (!right) {
            return ASTNode();
        }
        result = ASTNode(result.fOffset, ASTNode::Kind::kBinary, std::move(t),
                             { std::move(result), std::move(right) });
    }
    return result;
}

/* logicalAndExpression (LOGICALXOR logicalAndExpression)* */
ASTNode Parser::logicalXorExpression() {
    ASTNode result = this->logicalAndExpression();
    if (!result) {
        return ASTNode();
    }
    Token t;
    while (this->checkNext(Token::LOGICALXOR, &t)) {
        ASTNode right = this->logicalAndExpression();
        if (!right) {
            return ASTNode();
        }
        result = ASTNode(result.fOffset, ASTNode::Kind::kBinary, std::move(t),
                             { std::move(result), std::move(right) });
    }
    return result;
}

/* bitwiseOrExpression (LOGICALAND bitwiseOrExpression)* */
ASTNode Parser::logicalAndExpression() {
    ASTNode result = this->bitwiseOrExpression();
    if (!result) {
        return ASTNode();
    }
    Token t;
    while (this->checkNext(Token::LOGICALAND, &t)) {
        ASTNode right = this->bitwiseOrExpression();
        if (!right) {
            return ASTNode();
        }
        result = ASTNode(result.fOffset, ASTNode::Kind::kBinary, std::move(t),
                             { std::move(result), std::move(right) });
    }
    return result;
}

/* bitwiseXorExpression (BITWISEOR bitwiseXorExpression)* */
ASTNode Parser::bitwiseOrExpression() {
    ASTNode result = this->bitwiseXorExpression();
    if (!result) {
        return ASTNode();
    }
    Token t;
    while (this->checkNext(Token::BITWISEOR, &t)) {
        ASTNode right = this->bitwiseXorExpression();
        if (!right) {
            return ASTNode();
        }
        result = ASTNode(result.fOffset, ASTNode::Kind::kBinary, std::move(t),
                             { std::move(result), std::move(right) });
    }
    return result;
}

/* bitwiseAndExpression (BITWISEXOR bitwiseAndExpression)* */
ASTNode Parser::bitwiseXorExpression() {
    ASTNode result = this->bitwiseAndExpression();
    if (!result) {
        return ASTNode();
    }
    Token t;
    while (this->checkNext(Token::BITWISEXOR, &t)) {
        ASTNode right = this->bitwiseAndExpression();
        if (!right) {
            return ASTNode();
        }
        result = ASTNode(result.fOffset, ASTNode::Kind::kBinary, std::move(t),
                             { std::move(result), std::move(right) });
    }
    return result;
}

/* equalityExpression (BITWISEAND equalityExpression)* */
ASTNode Parser::bitwiseAndExpression() {
    ASTNode result = this->equalityExpression();
    if (!result) {
        return ASTNode();
    }
    Token t;
    while (this->checkNext(Token::BITWISEAND, &t)) {
        ASTNode right = this->equalityExpression();
        if (!right) {
            return ASTNode();
        }
        result = ASTNode(result.fOffset, ASTNode::Kind::kBinary, std::move(t),
                             { std::move(result), std::move(right) });
    }
    return result;
}

/* relationalExpression ((EQEQ | NEQ) relationalExpression)* */
ASTNode Parser::equalityExpression() {
    ASTNode result = this->relationalExpression();
    if (!result) {
        return ASTNode();
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::EQEQ:   // fall through
            case Token::NEQ: {
                Token t = this->nextToken();
                ASTNode right = this->relationalExpression();
                if (!right) {
                    return ASTNode();
                }
                result = ASTNode(result.fOffset, ASTNode::Kind::kBinary, std::move(t),
                                     { std::move(result), std::move(right) });
                break;
            }
            default:
                return result;
        }
    }
}

/* shiftExpression ((LT | GT | LTEQ | GTEQ) shiftExpression)* */
ASTNode Parser::relationalExpression() {
    ASTNode result = this->shiftExpression();
    if (!result) {
        return ASTNode();
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::LT:   // fall through
            case Token::GT:   // fall through
            case Token::LTEQ: // fall through
            case Token::GTEQ: {
                Token t = this->nextToken();
                ASTNode right = this->shiftExpression();
                if (!right) {
                    return ASTNode();
                }
                result = ASTNode(result.fOffset, ASTNode::Kind::kBinary, std::move(t),
                                     { std::move(result), std::move(right) });
                break;
            }
            default:
                return result;
        }
    }
}

/* additiveExpression ((SHL | SHR) additiveExpression)* */
ASTNode Parser::shiftExpression() {
    ASTNode result = this->additiveExpression();
    if (!result) {
        return ASTNode();
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::SHL: // fall through
            case Token::SHR: {
                Token t = this->nextToken();
                ASTNode right = this->additiveExpression();
                if (!right) {
                    return ASTNode();
                }
                result = ASTNode(result.fOffset, ASTNode::Kind::kBinary, std::move(t),
                                     { std::move(result), std::move(right) });
                break;
            }
            default:
                return result;
        }
    }
}

/* multiplicativeExpression ((PLUS | MINUS) multiplicativeExpression)* */
ASTNode Parser::additiveExpression() {
    ASTNode result = this->multiplicativeExpression();
    if (!result) {
        return ASTNode();
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::PLUS: // fall through
            case Token::MINUS: {
                Token t = this->nextToken();
                ASTNode right = this->multiplicativeExpression();
                if (!right) {
                    return ASTNode();
                }
                result = ASTNode(result.fOffset, ASTNode::Kind::kBinary, std::move(t),
                                     { std::move(result), std::move(right) });
                break;
            }
            default:
                return result;
        }
    }
}

/* unaryExpression ((STAR | SLASH | PERCENT) unaryExpression)* */
ASTNode Parser::multiplicativeExpression() {
    ASTNode result = this->unaryExpression();
    if (!result) {
        return ASTNode();
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::STAR: // fall through
            case Token::SLASH: // fall through
            case Token::PERCENT: {
                Token t = this->nextToken();
                ASTNode right = this->unaryExpression();
                if (!right) {
                    return ASTNode();
                }
                result = ASTNode(result.fOffset, ASTNode::Kind::kBinary, std::move(t),
                                     { std::move(result), std::move(right) });
                break;
            }
            default:
                return result;
        }
    }
}

/* postfixExpression | (PLUS | MINUS | NOT | PLUSPLUS | MINUSMINUS) unaryExpression */
ASTNode Parser::unaryExpression() {
    switch (this->peek().fKind) {
        case Token::PLUS:       // fall through
        case Token::MINUS:      // fall through
        case Token::LOGICALNOT: // fall through
        case Token::BITWISENOT: // fall through
        case Token::PLUSPLUS:   // fall through
        case Token::MINUSMINUS: {
            AutoDepth depth(this);
            if (!depth.checkValid()) {
                return ASTNode();
            }
            Token t = this->nextToken();
            ASTNode expr = this->unaryExpression();
            if (!expr) {
                return ASTNode();
            }
            return ASTNode(t.fOffset, ASTNode::Kind::kPrefix, std::move(t),
                           { std::move(expr) });
        }
        default:
            return this->postfixExpression();
    }
}

/* term suffix* */
ASTNode Parser::postfixExpression() {
    ASTNode result = this->term();
    if (!result) {
        return ASTNode();
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::LBRACKET:   // fall through
            case Token::DOT:        // fall through
            case Token::LPAREN:     // fall through
            case Token::PLUSPLUS:   // fall through
            case Token::MINUSMINUS: // fall through
            case Token::COLONCOLON:
                result = this->suffix(std::move(result));
                break;
            default:
                return result;
        }
    }
}

/* LBRACKET expression? RBRACKET | DOT IDENTIFIER | LPAREN parameters RPAREN |
   PLUSPLUS | MINUSMINUS | COLONCOLON IDENTIFIER */
ASTNode Parser::suffix(ASTNode base) {
    Token next = this->nextToken();
    switch (next.fKind) {
        case Token::LBRACKET: {
            if (this->checkNext(Token::RBRACKET)) {
                return ASTNode(next.fOffset, ASTNode::Kind::kIndex,
                               (std::vector<ASTNode>) { std::move(base) });
            }
            ASTNode e = this->expression();
            if (!e) {
                return ASTNode();
            }
            this->expect(Token::RBRACKET, "']' to complete array access expression");
            return ASTNode(next.fOffset, ASTNode::Kind::kIndex, { std::move(base), std::move(e) });
        }
        case Token::DOT: // fall through
        case Token::COLONCOLON: {
            int offset = this->peek().fOffset;
            StringFragment text;
            if (this->identifier(&text)) {
                return ASTNode(offset, ASTNode::Kind::kField, std::move(text), { std::move(base) });
            }
            return ASTNode();
        }
        case Token::LPAREN: {
            std::vector<ASTNode> children;
            children.push_back(std::move(base));
            if (this->peek().fKind != Token::RPAREN) {
                for (;;) {
                    ASTNode expr = this->assignmentExpression();
                    if (!expr) {
                        return ASTNode();
                    }
                    children.push_back(std::move(expr));
                    if (!this->checkNext(Token::COMMA)) {
                        break;
                    }
                }
            }
            this->expect(Token::RPAREN, "')' to complete function parameters");
            ASTNode result = ASTNode(next.fOffset, ASTNode::Kind::kCall, std::move(children));
            return result;
        }
        case Token::PLUSPLUS: // fall through
        case Token::MINUSMINUS:
            return ASTNode(next.fOffset, ASTNode::Kind::kPostfix, next, { std::move(base) });
        default: {
            this->error(next,  "expected expression suffix, but found '" + this->text(next) +
                                         "'\n");
            return ASTNode();
        }
    }
}

/* IDENTIFIER | intLiteral | floatLiteral | boolLiteral | NULL_LITERAL | '(' expression ')' */
ASTNode Parser::term() {
    Token t = this->peek();
    switch (t.fKind) {
        case Token::IDENTIFIER: {
            StringFragment text;
            if (this->identifier(&text)) {
                return ASTNode(t.fOffset, ASTNode::Kind::kIdentifier, std::move(text));
            }
        }
        case Token::INT_LITERAL: {
            SKSL_INT i;
            if (this->intLiteral(&i)) {
                return ASTNode(t.fOffset, ASTNode::Kind::kInt, i);
            }
            break;
        }
        case Token::FLOAT_LITERAL: {
            SKSL_FLOAT f;
            if (this->floatLiteral(&f)) {
                return ASTNode(t.fOffset, ASTNode::Kind::kFloat, f);
            }
            break;
        }
        case Token::TRUE_LITERAL: // fall through
        case Token::FALSE_LITERAL: {
            bool b;
            if (this->boolLiteral(&b)) {
                return ASTNode(t.fOffset, ASTNode::Kind::kBool, b);
            }
            break;
        }
        case Token::NULL_LITERAL:
            this->nextToken();
            return ASTNode(t.fOffset, ASTNode::Kind::kNull);
        case Token::LPAREN: {
            this->nextToken();
            ASTNode result = this->expression();
            if (result) {
                this->expect(Token::RPAREN, "')' to complete expression");
                return result;
            }
            break;
        }
        default:
            this->nextToken();
            this->error(t.fOffset,  "expected expression, but found '" + this->text(t) + "'\n");
    }
    return ASTNode();
}

/* INT_LITERAL */
bool Parser::intLiteral(SKSL_INT* dest) {
    Token t;
    if (this->expect(Token::INT_LITERAL, "integer literal", &t)) {
        *dest = SkSL::stol(this->text(t));
        return true;
    }
    return false;
}

/* FLOAT_LITERAL */
bool Parser::floatLiteral(SKSL_FLOAT* dest) {
    Token t;
    if (this->expect(Token::FLOAT_LITERAL, "float literal", &t)) {
        *dest = SkSL::stod(this->text(t));
        return true;
    }
    return false;
}

/* TRUE_LITERAL | FALSE_LITERAL */
bool Parser::boolLiteral(bool* dest) {
    Token t = this->nextToken();
    switch (t.fKind) {
        case Token::TRUE_LITERAL:
            *dest = true;
            return true;
        case Token::FALSE_LITERAL:
            *dest = false;
            return true;
        default:
            this->error(t, "expected 'true' or 'false', but found '" + this->text(t) + "'\n");
            return false;
    }
}

/* IDENTIFIER */
bool Parser::identifier(StringFragment* dest) {
    Token t;
    if (this->expect(Token::IDENTIFIER, "identifier", &t)) {
        *dest = this->text(t);
        return true;
    }
    return false;
}

} // namespace
