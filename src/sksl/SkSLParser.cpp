/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "stdio.h"
#include "src/sksl/SkSLParser.h"
#include "src/sksl/ast/SkSLASTBinaryExpression.h"
#include "src/sksl/ast/SkSLASTBlock.h"
#include "src/sksl/ast/SkSLASTBoolLiteral.h"
#include "src/sksl/ast/SkSLASTBreakStatement.h"
#include "src/sksl/ast/SkSLASTCallSuffix.h"
#include "src/sksl/ast/SkSLASTContinueStatement.h"
#include "src/sksl/ast/SkSLASTDiscardStatement.h"
#include "src/sksl/ast/SkSLASTDoStatement.h"
#include "src/sksl/ast/SkSLASTEnum.h"
#include "src/sksl/ast/SkSLASTExpression.h"
#include "src/sksl/ast/SkSLASTExpressionStatement.h"
#include "src/sksl/ast/SkSLASTExtension.h"
#include "src/sksl/ast/SkSLASTFieldSuffix.h"
#include "src/sksl/ast/SkSLASTFloatLiteral.h"
#include "src/sksl/ast/SkSLASTForStatement.h"
#include "src/sksl/ast/SkSLASTFunction.h"
#include "src/sksl/ast/SkSLASTIdentifier.h"
#include "src/sksl/ast/SkSLASTIfStatement.h"
#include "src/sksl/ast/SkSLASTIndexSuffix.h"
#include "src/sksl/ast/SkSLASTIntLiteral.h"
#include "src/sksl/ast/SkSLASTInterfaceBlock.h"
#include "src/sksl/ast/SkSLASTModifiersDeclaration.h"
#include "src/sksl/ast/SkSLASTNullLiteral.h"
#include "src/sksl/ast/SkSLASTParameter.h"
#include "src/sksl/ast/SkSLASTPrefixExpression.h"
#include "src/sksl/ast/SkSLASTReturnStatement.h"
#include "src/sksl/ast/SkSLASTSection.h"
#include "src/sksl/ast/SkSLASTStatement.h"
#include "src/sksl/ast/SkSLASTSuffixExpression.h"
#include "src/sksl/ast/SkSLASTSwitchCase.h"
#include "src/sksl/ast/SkSLASTSwitchStatement.h"
#include "src/sksl/ast/SkSLASTTernaryExpression.h"
#include "src/sksl/ast/SkSLASTType.h"
#include "src/sksl/ast/SkSLASTVarDeclaration.h"
#include "src/sksl/ast/SkSLASTVarDeclarationStatement.h"
#include "src/sksl/ast/SkSLASTWhileStatement.h"
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
    TOKEN(SKVECTOR4,                    "SkVector4");
    TOKEN(SKRECT,                       "SkRect");
    TOKEN(SKIRECT,                      "SkIRect");
    TOKEN(SKPMCOLOR,                    "SkPMColor");
    TOKEN(SKMATRIX44,                   "SkMatrix44");
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
std::vector<std::unique_ptr<ASTDeclaration>> Parser::file() {
    std::vector<std::unique_ptr<ASTDeclaration>> result;
    for (;;) {
        switch (this->peek().fKind) {
            case Token::END_OF_FILE:
                return result;
            case Token::DIRECTIVE: {
                std::unique_ptr<ASTDeclaration> decl = this->directive();
                if (decl) {
                    result.push_back(std::move(decl));
                }
                break;
            }
            case Token::SECTION: {
                std::unique_ptr<ASTDeclaration> section = this->section();
                if (section) {
                    result.push_back(std::move(section));
                }
                break;
            }
            default: {
                std::unique_ptr<ASTDeclaration> decl = this->declaration();
                if (!decl) {
                    continue;
                }
                result.push_back(std::move(decl));
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
std::unique_ptr<ASTDeclaration> Parser::directive() {
    Token start;
    if (!this->expect(Token::DIRECTIVE, "a directive", &start)) {
        return nullptr;
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
        return nullptr;
    } else if (text == "#extension") {
        Token name;
        if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
            return nullptr;
        }
        if (!this->expect(Token::COLON, "':'")) {
            return nullptr;
        }
        // FIXME: need to start paying attention to this token
        if (!this->expect(Token::IDENTIFIER, "an identifier")) {
            return nullptr;
        }
        return std::unique_ptr<ASTDeclaration>(new ASTExtension(start.fOffset,
                                                                String(this->text(name))));
    } else {
        this->error(start, "unsupported directive '" + this->text(start) + "'");
        return nullptr;
    }
}

/* SECTION LBRACE (LPAREN IDENTIFIER RPAREN)? <any sequence of tokens with balanced braces>
   RBRACE */
std::unique_ptr<ASTDeclaration> Parser::section() {
    Token start;
    if (!this->expect(Token::SECTION, "a section token", &start)) {
        return nullptr;
    }
    String argument;
    if (this->peek().fKind == Token::LPAREN) {
        this->nextToken();
        Token argToken;
        if (!this->expect(Token::IDENTIFIER, "an identifier", &argToken)) {
            return nullptr;
        }
        argument = this->text(argToken);
        if (!this->expect(Token::RPAREN, "')'")) {
            return nullptr;
        }
    }
    if (!this->expect(Token::LBRACE, "'{'")) {
        return nullptr;
    }
    String text;
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
                return nullptr;
            default:
                break;
        }
        if (!level) {
            break;
        }
        text += this->text(next);
    }
    StringFragment name = this->text(start);
    ++name.fChars;
    --name.fLength;
    return std::unique_ptr<ASTDeclaration>(new ASTSection(start.fOffset,
                                                          String(name),
                                                          argument,
                                                          text));
}

/* ENUM CLASS IDENTIFIER LBRACE (IDENTIFIER (EQ expression)? (COMMA IDENTIFIER (EQ expression))*)?
   RBRACE */
std::unique_ptr<ASTDeclaration> Parser::enumDeclaration() {
    Token start;
    if (!this->expect(Token::ENUM, "'enum'", &start)) {
        return nullptr;
    }
    if (!this->expect(Token::CLASS, "'class'")) {
        return nullptr;
    }
    Token name;
    if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
        return nullptr;
    }
    if (!this->expect(Token::LBRACE, "'{'")) {
        return nullptr;
    }
    fTypes.add(this->text(name), std::unique_ptr<Symbol>(new Type(this->text(name),
                                                                  Type::kEnum_Kind)));
    std::vector<StringFragment> names;
    std::vector<std::unique_ptr<ASTExpression>> values;
    if (!this->checkNext(Token::RBRACE)) {
        Token id;
        if (!this->expect(Token::IDENTIFIER, "an identifier", &id)) {
            return nullptr;
        }
        names.push_back(this->text(id));
        if (this->checkNext(Token::EQ)) {
            std::unique_ptr<ASTExpression> value = this->assignmentExpression();
            if (!value) {
                return nullptr;
            }
            values.push_back(std::move(value));
        } else {
            values.push_back(nullptr);
        }
        while (!this->checkNext(Token::RBRACE)) {
            if (!this->expect(Token::COMMA, "','")) {
                return nullptr;
            }
            if (!this->expect(Token::IDENTIFIER, "an identifier", &id)) {
                return nullptr;
            }
            names.push_back(this->text(id));
            if (this->checkNext(Token::EQ)) {
                std::unique_ptr<ASTExpression> value = this->assignmentExpression();
                if (!value) {
                    return nullptr;
                }
                values.push_back(std::move(value));
            } else {
                values.push_back(nullptr);
            }
        }
    }
    this->expect(Token::SEMICOLON, "';'");
    return std::unique_ptr<ASTDeclaration>(new ASTEnum(name.fOffset, this->text(name), names,
                                                       std::move(values)));
}

/* enumDeclaration | modifiers (structVarDeclaration | type IDENTIFIER ((LPAREN parameter
   (COMMA parameter)* RPAREN (block | SEMICOLON)) | SEMICOLON) | interfaceBlock) */
std::unique_ptr<ASTDeclaration> Parser::declaration() {
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
        return std::unique_ptr<ASTDeclaration>(new ASTModifiersDeclaration(modifiers));
    }
    std::unique_ptr<ASTType> type(this->type());
    if (!type) {
        return nullptr;
    }
    if (type->fKind == ASTType::kStruct_Kind && this->checkNext(Token::SEMICOLON)) {
        return nullptr;
    }
    Token name;
    if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
        return nullptr;
    }
    if (this->checkNext(Token::LPAREN)) {
        std::vector<std::unique_ptr<ASTParameter>> parameters;
        while (this->peek().fKind != Token::RPAREN) {
            if (parameters.size() > 0) {
                if (!this->expect(Token::COMMA, "','")) {
                    return nullptr;
                }
            }
            std::unique_ptr<ASTParameter> parameter = this->parameter();
            if (!parameter) {
                return nullptr;
            }
            parameters.push_back(std::move(parameter));
        }
        this->nextToken();
        std::unique_ptr<ASTBlock> body;
        if (!this->checkNext(Token::SEMICOLON)) {
            body = this->block();
            if (!body) {
                return nullptr;
            }
        }
        return std::unique_ptr<ASTDeclaration>(new ASTFunction(name.fOffset,
                                                               modifiers,
                                                               std::move(type),
                                                               this->text(name),
                                                               std::move(parameters),
                                                               std::move(body)));
    } else {
        return this->varDeclarationEnd(modifiers, std::move(type), this->text(name));
    }
}

/* modifiers type IDENTIFIER varDeclarationEnd */
std::unique_ptr<ASTVarDeclarations> Parser::varDeclarations() {
    Modifiers modifiers = this->modifiers();
    std::unique_ptr<ASTType> type(this->type());
    if (!type) {
        return nullptr;
    }
    Token name;
    if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
        return nullptr;
    }
    return this->varDeclarationEnd(modifiers, std::move(type), this->text(name));
}

/* STRUCT IDENTIFIER LBRACE varDeclaration* RBRACE */
std::unique_ptr<ASTType> Parser::structDeclaration() {
    if (!this->expect(Token::STRUCT, "'struct'")) {
        return nullptr;
    }
    Token name;
    if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
        return nullptr;
    }
    if (!this->expect(Token::LBRACE, "'{'")) {
        return nullptr;
    }
    std::vector<Type::Field> fields;
    while (this->peek().fKind != Token::RBRACE) {
        std::unique_ptr<ASTVarDeclarations> decl = this->varDeclarations();
        if (!decl) {
            return nullptr;
        }
        for (const auto& var : decl->fVars) {
            auto type = (const Type*) fTypes[decl->fType->fName];
            for (int i = (int) var.fSizes.size() - 1; i >= 0; i--) {
                if (!var.fSizes[i] || var.fSizes[i]->fKind != ASTExpression::kInt_Kind) {
                    this->error(decl->fOffset, "array size in struct field must be a constant");
                    return nullptr;
                }
                uint64_t columns = ((ASTIntLiteral&) *var.fSizes[i]).fValue;
                String name = type->name() + "[" + to_string(columns) + "]";
                type = (Type*) fTypes.takeOwnership(std::unique_ptr<Symbol>(
                                                                         new Type(name,
                                                                                  Type::kArray_Kind,
                                                                                  *type,
                                                                                  (int) columns)));
            }
            fields.push_back(Type::Field(decl->fModifiers, var.fName, type));
            if (var.fValue) {
                this->error(decl->fOffset, "initializers are not permitted on struct fields");
            }
        }
    }
    if (!this->expect(Token::RBRACE, "'}'")) {
        return nullptr;
    }
    fTypes.add(this->text(name), std::unique_ptr<Type>(new Type(name.fOffset, this->text(name),
                                                                fields)));
    return std::unique_ptr<ASTType>(new ASTType(name.fOffset, this->text(name),
                                                ASTType::kStruct_Kind, std::vector<int>(), false));
}

/* structDeclaration ((IDENTIFIER varDeclarationEnd) | SEMICOLON) */
std::unique_ptr<ASTVarDeclarations> Parser::structVarDeclaration(Modifiers modifiers) {
    std::unique_ptr<ASTType> type = this->structDeclaration();
    if (!type) {
        return nullptr;
    }
    Token name;
    if (this->checkNext(Token::IDENTIFIER, &name)) {
        std::unique_ptr<ASTVarDeclarations> result = this->varDeclarationEnd(modifiers,
                                                                             std::move(type),
                                                                             this->text(name));
        if (result) {
            for (const auto& var : result->fVars) {
                if (var.fValue) {
                    this->error(var.fValue->fOffset,
                                "struct variables cannot be initialized");
                }
            }
        }
        return result;
    }
    this->expect(Token::SEMICOLON, "';'");
    return nullptr;
}

/* (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)? (COMMA IDENTIFER
   (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)?)* SEMICOLON */
std::unique_ptr<ASTVarDeclarations> Parser::varDeclarationEnd(Modifiers mods,
                                                              std::unique_ptr<ASTType> type,
                                                              StringFragment name) {
    std::vector<ASTVarDeclaration> vars;
    std::vector<std::unique_ptr<ASTExpression>> currentVarSizes;
    while (this->checkNext(Token::LBRACKET)) {
        if (this->checkNext(Token::RBRACKET)) {
            currentVarSizes.push_back(nullptr);
        } else {
            std::unique_ptr<ASTExpression> size(this->expression());
            if (!size) {
                return nullptr;
            }
            currentVarSizes.push_back(std::move(size));
            if (!this->expect(Token::RBRACKET, "']'")) {
                return nullptr;
            }
        }
    }
    std::unique_ptr<ASTExpression> value;
    if (this->checkNext(Token::EQ)) {
        value = this->assignmentExpression();
        if (!value) {
            return nullptr;
        }
    }
    vars.emplace_back(name, std::move(currentVarSizes), std::move(value));
    while (this->checkNext(Token::COMMA)) {
        Token name;
        if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
            return nullptr;
        }
        currentVarSizes.clear();
        value.reset();
        while (this->checkNext(Token::LBRACKET)) {
            if (this->checkNext(Token::RBRACKET)) {
                currentVarSizes.push_back(nullptr);
            } else {
                std::unique_ptr<ASTExpression> size(this->expression());
                if (!size) {
                    return nullptr;
                }
                currentVarSizes.push_back(std::move(size));
                if (!this->expect(Token::RBRACKET, "']'")) {
                    return nullptr;
                }
            }
        }
        if (this->checkNext(Token::EQ)) {
            value = this->assignmentExpression();
            if (!value) {
                return nullptr;
            }
        }
        vars.emplace_back(this->text(name), std::move(currentVarSizes), std::move(value));
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return nullptr;
    }
    return std::unique_ptr<ASTVarDeclarations>(new ASTVarDeclarations(std::move(mods),
                                                                      std::move(type),
                                                                      std::move(vars)));
}

/* modifiers type IDENTIFIER (LBRACKET INT_LITERAL RBRACKET)? */
std::unique_ptr<ASTParameter> Parser::parameter() {
    Modifiers modifiers = this->modifiersWithDefaults(0);
    std::unique_ptr<ASTType> type = this->type();
    if (!type) {
        return nullptr;
    }
    Token name;
    if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
        return nullptr;
    }
    std::vector<int> sizes;
    while (this->checkNext(Token::LBRACKET)) {
        Token sizeToken;
        if (!this->expect(Token::INT_LITERAL, "a positive integer", &sizeToken)) {
            return nullptr;
        }
        sizes.push_back(SkSL::stoi(this->text(sizeToken)));
        if (!this->expect(Token::RBRACKET, "']'")) {
            return nullptr;
        }
    }
    return std::unique_ptr<ASTParameter>(new ASTParameter(name.fOffset, modifiers, std::move(type),
                                                          this->text(name), std::move(sizes)));
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
String Parser::layoutCode() {
    if (!this->expect(Token::EQ, "'='")) {
        return "";
    }
    Token start = this->nextRawToken();
    this->pushback(start);
    String code;
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
            this->pushback(std::move(next));
        }
        else {
            code += this->text(next);
        }
    }
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
                case LayoutToken::SKVECTOR4:
                    return Layout::CType::kSkVector4;
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
                case LayoutToken::SKMATRIX44:
                    return Layout::CType::kSkMatrix44;
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
    String when;
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
std::unique_ptr<ASTStatement> Parser::statement() {
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
            return std::unique_ptr<ASTStatement>(new ASTBlock(start.fOffset,
                                                     std::vector<std::unique_ptr<ASTStatement>>()));
        case Token::CONST: {
            auto decl = this->varDeclarations();
            if (!decl) {
                return nullptr;
            }
            return std::unique_ptr<ASTStatement>(new ASTVarDeclarationStatement(std::move(decl)));
        }
        case Token::IDENTIFIER:
            if (this->isType(this->text(start))) {
                auto decl = this->varDeclarations();
                if (!decl) {
                    return nullptr;
                }
                return std::unique_ptr<ASTStatement>(new ASTVarDeclarationStatement(
                                                                                  std::move(decl)));
            }
            // fall through
        default:
            return this->expressionStatement();
    }
}

/* IDENTIFIER(type) (LBRACKET intLiteral? RBRACKET)* QUESTION? */
std::unique_ptr<ASTType> Parser::type() {
    Token type;
    if (!this->expect(Token::IDENTIFIER, "a type", &type)) {
        return nullptr;
    }
    if (!this->isType(this->text(type))) {
        this->error(type, ("no type named '" + this->text(type) + "'").c_str());
        return nullptr;
    }
    std::vector<int> sizes;
    while (this->checkNext(Token::LBRACKET)) {
        if (this->peek().fKind != Token::RBRACKET) {
            int64_t i;
            if (this->intLiteral(&i)) {
                sizes.push_back(i);
            } else {
                return nullptr;
            }
        } else {
            sizes.push_back(-1);
        }
        this->expect(Token::RBRACKET, "']'");
    }
    bool nullable = this->checkNext(Token::QUESTION);
    return std::unique_ptr<ASTType>(new ASTType(type.fOffset, this->text(type),
                                                ASTType::kIdentifier_Kind, sizes, nullable));
}

/* IDENTIFIER LBRACE varDeclaration* RBRACE (IDENTIFIER (LBRACKET expression? RBRACKET)*)? */
std::unique_ptr<ASTDeclaration> Parser::interfaceBlock(Modifiers mods) {
    Token name;
    if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
        return nullptr;
    }
    if (peek().fKind != Token::LBRACE) {
        // we only get into interfaceBlock if we found a top-level identifier which was not a type.
        // 99% of the time, the user was not actually intending to create an interface block, so
        // it's better to report it as an unknown type
        this->error(name, "no type named '" + this->text(name) + "'");
        return nullptr;
    }
    this->nextToken();
    std::vector<std::unique_ptr<ASTVarDeclarations>> decls;
    while (this->peek().fKind != Token::RBRACE) {
        std::unique_ptr<ASTVarDeclarations> decl = this->varDeclarations();
        if (!decl) {
            return nullptr;
        }
        decls.push_back(std::move(decl));
    }
    this->nextToken();
    std::vector<std::unique_ptr<ASTExpression>> sizes;
    StringFragment instanceName;
    Token instanceNameToken;
    if (this->checkNext(Token::IDENTIFIER, &instanceNameToken)) {
        while (this->checkNext(Token::LBRACKET)) {
            if (this->peek().fKind != Token::RBRACKET) {
                std::unique_ptr<ASTExpression> size = this->expression();
                if (!size) {
                    return nullptr;
                }
                sizes.push_back(std::move(size));
            } else {
                sizes.push_back(nullptr);
            }
            this->expect(Token::RBRACKET, "']'");
        }
        instanceName = this->text(instanceNameToken);
    }
    this->expect(Token::SEMICOLON, "';'");
    return std::unique_ptr<ASTDeclaration>(new ASTInterfaceBlock(name.fOffset, mods,
                                                                 this->text(name),
                                                                 std::move(decls),
                                                                 instanceName,
                                                                 std::move(sizes)));
}

/* IF LPAREN expression RPAREN statement (ELSE statement)? */
std::unique_ptr<ASTIfStatement> Parser::ifStatement() {
    Token start;
    bool isStatic = this->checkNext(Token::STATIC_IF, &start);
    if (!isStatic && !this->expect(Token::IF, "'if'", &start)) {
        return nullptr;
    }
    if (!this->expect(Token::LPAREN, "'('")) {
        return nullptr;
    }
    std::unique_ptr<ASTExpression> test(this->expression());
    if (!test) {
        return nullptr;
    }
    if (!this->expect(Token::RPAREN, "')'")) {
        return nullptr;
    }
    std::unique_ptr<ASTStatement> ifTrue(this->statement());
    if (!ifTrue) {
        return nullptr;
    }
    std::unique_ptr<ASTStatement> ifFalse;
    if (this->checkNext(Token::ELSE)) {
        ifFalse = this->statement();
        if (!ifFalse) {
            return nullptr;
        }
    }
    return std::unique_ptr<ASTIfStatement>(new ASTIfStatement(start.fOffset,
                                                              isStatic,
                                                              std::move(test),
                                                              std::move(ifTrue),
                                                              std::move(ifFalse)));
}

/* DO statement WHILE LPAREN expression RPAREN SEMICOLON */
std::unique_ptr<ASTDoStatement> Parser::doStatement() {
    Token start;
    if (!this->expect(Token::DO, "'do'", &start)) {
        return nullptr;
    }
    std::unique_ptr<ASTStatement> statement(this->statement());
    if (!statement) {
        return nullptr;
    }
    if (!this->expect(Token::WHILE, "'while'")) {
        return nullptr;
    }
    if (!this->expect(Token::LPAREN, "'('")) {
        return nullptr;
    }
    std::unique_ptr<ASTExpression> test(this->expression());
    if (!test) {
        return nullptr;
    }
    if (!this->expect(Token::RPAREN, "')'")) {
        return nullptr;
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return nullptr;
    }
    return std::unique_ptr<ASTDoStatement>(new ASTDoStatement(start.fOffset,
                                                              std::move(statement),
                                                              std::move(test)));
}

/* WHILE LPAREN expression RPAREN STATEMENT */
std::unique_ptr<ASTWhileStatement> Parser::whileStatement() {
    Token start;
    if (!this->expect(Token::WHILE, "'while'", &start)) {
        return nullptr;
    }
    if (!this->expect(Token::LPAREN, "'('")) {
        return nullptr;
    }
    std::unique_ptr<ASTExpression> test(this->expression());
    if (!test) {
        return nullptr;
    }
    if (!this->expect(Token::RPAREN, "')'")) {
        return nullptr;
    }
    std::unique_ptr<ASTStatement> statement(this->statement());
    if (!statement) {
        return nullptr;
    }
    return std::unique_ptr<ASTWhileStatement>(new ASTWhileStatement(start.fOffset,
                                                                    std::move(test),
                                                                    std::move(statement)));
}

/* CASE expression COLON statement* */
std::unique_ptr<ASTSwitchCase> Parser::switchCase() {
    Token start;
    if (!this->expect(Token::CASE, "'case'", &start)) {
        return nullptr;
    }
    std::unique_ptr<ASTExpression> value = this->expression();
    if (!value) {
        return nullptr;
    }
    if (!this->expect(Token::COLON, "':'")) {
        return nullptr;
    }
    std::vector<std::unique_ptr<ASTStatement>> statements;
    while (this->peek().fKind != Token::RBRACE && this->peek().fKind != Token::CASE &&
           this->peek().fKind != Token::DEFAULT) {
        std::unique_ptr<ASTStatement> s = this->statement();
        if (!s) {
            return nullptr;
        }
        statements.push_back(std::move(s));
    }
    return std::unique_ptr<ASTSwitchCase>(new ASTSwitchCase(start.fOffset, std::move(value),
                                                            std::move(statements)));
}

/* SWITCH LPAREN expression RPAREN LBRACE switchCase* (DEFAULT COLON statement*)? RBRACE */
std::unique_ptr<ASTStatement> Parser::switchStatement() {
    Token start;
    bool isStatic = this->checkNext(Token::STATIC_SWITCH, &start);
    if (!isStatic && !this->expect(Token::SWITCH, "'switch'", &start)) {
        return nullptr;
    }
    if (!this->expect(Token::LPAREN, "'('")) {
        return nullptr;
    }
    std::unique_ptr<ASTExpression> value(this->expression());
    if (!value) {
        return nullptr;
    }
    if (!this->expect(Token::RPAREN, "')'")) {
        return nullptr;
    }
    if (!this->expect(Token::LBRACE, "'{'")) {
        return nullptr;
    }
    std::vector<std::unique_ptr<ASTSwitchCase>> cases;
    while (this->peek().fKind == Token::CASE) {
        std::unique_ptr<ASTSwitchCase> c = this->switchCase();
        if (!c) {
            return nullptr;
        }
        cases.push_back(std::move(c));
    }
    // Requiring default: to be last (in defiance of C and GLSL) was a deliberate decision. Other
    // parts of the compiler may rely upon this assumption.
    if (this->peek().fKind == Token::DEFAULT) {
        Token defaultStart;
        SkAssertResult(this->expect(Token::DEFAULT, "'default'", &defaultStart));
        if (!this->expect(Token::COLON, "':'")) {
            return nullptr;
        }
        std::vector<std::unique_ptr<ASTStatement>> statements;
        while (this->peek().fKind != Token::RBRACE) {
            std::unique_ptr<ASTStatement> s = this->statement();
            if (!s) {
                return nullptr;
            }
            statements.push_back(std::move(s));
        }
        cases.emplace_back(new ASTSwitchCase(defaultStart.fOffset, nullptr,
                                             std::move(statements)));
    }
    if (!this->expect(Token::RBRACE, "'}'")) {
        return nullptr;
    }
    return std::unique_ptr<ASTStatement>(new ASTSwitchStatement(start.fOffset,
                                                                isStatic,
                                                                std::move(value),
                                                                std::move(cases)));
}

/* FOR LPAREN (declaration | expression)? SEMICOLON expression? SEMICOLON expression? RPAREN
   STATEMENT */
std::unique_ptr<ASTForStatement> Parser::forStatement() {
    Token start;
    if (!this->expect(Token::FOR, "'for'", &start)) {
        return nullptr;
    }
    if (!this->expect(Token::LPAREN, "'('")) {
        return nullptr;
    }
    std::unique_ptr<ASTStatement> initializer;
    Token nextToken = this->peek();
    switch (nextToken.fKind) {
        case Token::SEMICOLON:
            this->nextToken();
            break;
        case Token::CONST: {
            std::unique_ptr<ASTVarDeclarations> vd = this->varDeclarations();
            if (!vd) {
                return nullptr;
            }
            initializer = std::unique_ptr<ASTStatement>(new ASTVarDeclarationStatement(
                                                                                    std::move(vd)));
            break;
        }
        case Token::IDENTIFIER: {
            if (this->isType(this->text(nextToken))) {
                std::unique_ptr<ASTVarDeclarations> vd = this->varDeclarations();
                if (!vd) {
                    return nullptr;
                }
                initializer = std::unique_ptr<ASTStatement>(new ASTVarDeclarationStatement(
                                                                                    std::move(vd)));
                break;
            }
        } // fall through
        default:
            initializer = this->expressionStatement();
    }
    std::unique_ptr<ASTExpression> test;
    if (this->peek().fKind != Token::SEMICOLON) {
        test = this->expression();
        if (!test) {
            return nullptr;
        }
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return nullptr;
    }
    std::unique_ptr<ASTExpression> next;
    if (this->peek().fKind != Token::RPAREN) {
        next = this->expression();
        if (!next) {
            return nullptr;
        }
    }
    if (!this->expect(Token::RPAREN, "')'")) {
        return nullptr;
    }
    std::unique_ptr<ASTStatement> statement(this->statement());
    if (!statement) {
        return nullptr;
    }
    return std::unique_ptr<ASTForStatement>(new ASTForStatement(start.fOffset,
                                                                std::move(initializer),
                                                                std::move(test), std::move(next),
                                                                std::move(statement)));
}

/* RETURN expression? SEMICOLON */
std::unique_ptr<ASTReturnStatement> Parser::returnStatement() {
    Token start;
    if (!this->expect(Token::RETURN, "'return'", &start)) {
        return nullptr;
    }
    std::unique_ptr<ASTExpression> expression;
    if (this->peek().fKind != Token::SEMICOLON) {
        expression = this->expression();
        if (!expression) {
            return nullptr;
        }
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return nullptr;
    }
    return std::unique_ptr<ASTReturnStatement>(new ASTReturnStatement(start.fOffset,
                                                                      std::move(expression)));
}

/* BREAK SEMICOLON */
std::unique_ptr<ASTBreakStatement> Parser::breakStatement() {
    Token start;
    if (!this->expect(Token::BREAK, "'break'", &start)) {
        return nullptr;
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return nullptr;
    }
    return std::unique_ptr<ASTBreakStatement>(new ASTBreakStatement(start.fOffset));
}

/* CONTINUE SEMICOLON */
std::unique_ptr<ASTContinueStatement> Parser::continueStatement() {
    Token start;
    if (!this->expect(Token::CONTINUE, "'continue'", &start)) {
        return nullptr;
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return nullptr;
    }
    return std::unique_ptr<ASTContinueStatement>(new ASTContinueStatement(start.fOffset));
}

/* DISCARD SEMICOLON */
std::unique_ptr<ASTDiscardStatement> Parser::discardStatement() {
    Token start;
    if (!this->expect(Token::DISCARD, "'continue'", &start)) {
        return nullptr;
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return nullptr;
    }
    return std::unique_ptr<ASTDiscardStatement>(new ASTDiscardStatement(start.fOffset));
}

/* LBRACE statement* RBRACE */
std::unique_ptr<ASTBlock> Parser::block() {
    AutoDepth depth(this);
    if (!depth.checkValid()) {
        return nullptr;
    }
    Token start;
    if (!this->expect(Token::LBRACE, "'{'", &start)) {
        return nullptr;
    }
    std::vector<std::unique_ptr<ASTStatement>> statements;
    for (;;) {
        switch (this->peek().fKind) {
            case Token::RBRACE:
                this->nextToken();
                return std::unique_ptr<ASTBlock>(new ASTBlock(start.fOffset,
                                                              std::move(statements)));
            case Token::END_OF_FILE:
                this->error(this->peek(), "expected '}', but found end of file");
                return nullptr;
            default: {
                std::unique_ptr<ASTStatement> statement = this->statement();
                if (!statement) {
                    return nullptr;
                }
                statements.push_back(std::move(statement));
            }
        }
    }
}

/* expression SEMICOLON */
std::unique_ptr<ASTExpressionStatement> Parser::expressionStatement() {
    std::unique_ptr<ASTExpression> expr = this->expression();
    if (expr) {
        if (this->expect(Token::SEMICOLON, "';'")) {
            ASTExpressionStatement* result = new ASTExpressionStatement(std::move(expr));
            return std::unique_ptr<ASTExpressionStatement>(result);
        }
    }
    return nullptr;
}

/* commaExpression */
std::unique_ptr<ASTExpression> Parser::expression() {
    AutoDepth depth(this);
    if (!depth.checkValid()) {
        return nullptr;
    }
    return this->commaExpression();
}

/* assignmentExpression (COMMA assignmentExpression)* */
std::unique_ptr<ASTExpression> Parser::commaExpression() {
    std::unique_ptr<ASTExpression> result = this->assignmentExpression();
    if (!result) {
        return nullptr;
    }
    Token t;
    while (this->checkNext(Token::COMMA, &t)) {
        std::unique_ptr<ASTExpression> right = this->commaExpression();
        if (!right) {
            return nullptr;
        }
        result.reset(new ASTBinaryExpression(std::move(result), std::move(t), std::move(right)));
    }
    return result;
}

/* ternaryExpression ((EQEQ | STAREQ | SLASHEQ | PERCENTEQ | PLUSEQ | MINUSEQ | SHLEQ | SHREQ |
   BITWISEANDEQ | BITWISEXOREQ | BITWISEOREQ | LOGICALANDEQ | LOGICALXOREQ | LOGICALOREQ)
   assignmentExpression)*
 */
std::unique_ptr<ASTExpression> Parser::assignmentExpression() {
    std::unique_ptr<ASTExpression> result = this->ternaryExpression();
    if (!result) {
        return nullptr;
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
                std::unique_ptr<ASTExpression> right = this->assignmentExpression();
                if (!right) {
                    return nullptr;
                }
                result = std::unique_ptr<ASTExpression>(new ASTBinaryExpression(std::move(result),
                                                                                std::move(t),
                                                                                std::move(right)));
                return result;
            }
            default:
                return result;
        }
    }
}

/* logicalOrExpression ('?' expression ':' assignmentExpression)? */
std::unique_ptr<ASTExpression> Parser::ternaryExpression() {
    std::unique_ptr<ASTExpression> result = this->logicalOrExpression();
    if (!result) {
        return nullptr;
    }
    if (this->checkNext(Token::QUESTION)) {
        std::unique_ptr<ASTExpression> trueExpr = this->expression();
        if (!trueExpr) {
            return nullptr;
        }
        if (this->expect(Token::COLON, "':'")) {
            std::unique_ptr<ASTExpression> falseExpr = this->assignmentExpression();
            return std::unique_ptr<ASTExpression>(new ASTTernaryExpression(std::move(result),
                                                                           std::move(trueExpr),
                                                                           std::move(falseExpr)));
        }
        return nullptr;
    }
    return result;
}

/* logicalXorExpression (LOGICALOR logicalXorExpression)* */
std::unique_ptr<ASTExpression> Parser::logicalOrExpression() {
    std::unique_ptr<ASTExpression> result = this->logicalXorExpression();
    if (!result) {
        return nullptr;
    }
    Token t;
    while (this->checkNext(Token::LOGICALOR, &t)) {
        std::unique_ptr<ASTExpression> right = this->logicalXorExpression();
        if (!right) {
            return nullptr;
        }
        result.reset(new ASTBinaryExpression(std::move(result), std::move(t), std::move(right)));
    }
    return result;
}

/* logicalAndExpression (LOGICALXOR logicalAndExpression)* */
std::unique_ptr<ASTExpression> Parser::logicalXorExpression() {
    std::unique_ptr<ASTExpression> result = this->logicalAndExpression();
    if (!result) {
        return nullptr;
    }
    Token t;
    while (this->checkNext(Token::LOGICALXOR, &t)) {
        std::unique_ptr<ASTExpression> right = this->logicalAndExpression();
        if (!right) {
            return nullptr;
        }
        result.reset(new ASTBinaryExpression(std::move(result), std::move(t), std::move(right)));
    }
    return result;
}

/* bitwiseOrExpression (LOGICALAND bitwiseOrExpression)* */
std::unique_ptr<ASTExpression> Parser::logicalAndExpression() {
    std::unique_ptr<ASTExpression> result = this->bitwiseOrExpression();
    if (!result) {
        return nullptr;
    }
    Token t;
    while (this->checkNext(Token::LOGICALAND, &t)) {
        std::unique_ptr<ASTExpression> right = this->bitwiseOrExpression();
        if (!right) {
            return nullptr;
        }
        result.reset(new ASTBinaryExpression(std::move(result), std::move(t), std::move(right)));
    }
    return result;
}

/* bitwiseXorExpression (BITWISEOR bitwiseXorExpression)* */
std::unique_ptr<ASTExpression> Parser::bitwiseOrExpression() {
    std::unique_ptr<ASTExpression> result = this->bitwiseXorExpression();
    if (!result) {
        return nullptr;
    }
    Token t;
    while (this->checkNext(Token::BITWISEOR, &t)) {
        std::unique_ptr<ASTExpression> right = this->bitwiseXorExpression();
        if (!right) {
            return nullptr;
        }
        result.reset(new ASTBinaryExpression(std::move(result), std::move(t), std::move(right)));
    }
    return result;
}

/* bitwiseAndExpression (BITWISEXOR bitwiseAndExpression)* */
std::unique_ptr<ASTExpression> Parser::bitwiseXorExpression() {
    std::unique_ptr<ASTExpression> result = this->bitwiseAndExpression();
    if (!result) {
        return nullptr;
    }
    Token t;
    while (this->checkNext(Token::BITWISEXOR, &t)) {
        std::unique_ptr<ASTExpression> right = this->bitwiseAndExpression();
        if (!right) {
            return nullptr;
        }
        result.reset(new ASTBinaryExpression(std::move(result), std::move(t), std::move(right)));
    }
    return result;
}

/* equalityExpression (BITWISEAND equalityExpression)* */
std::unique_ptr<ASTExpression> Parser::bitwiseAndExpression() {
    std::unique_ptr<ASTExpression> result = this->equalityExpression();
    if (!result) {
        return nullptr;
    }
    Token t;
    while (this->checkNext(Token::BITWISEAND, &t)) {
        std::unique_ptr<ASTExpression> right = this->equalityExpression();
        if (!right) {
            return nullptr;
        }
        result.reset(new ASTBinaryExpression(std::move(result), std::move(t), std::move(right)));
    }
    return result;
}

/* relationalExpression ((EQEQ | NEQ) relationalExpression)* */
std::unique_ptr<ASTExpression> Parser::equalityExpression() {
    std::unique_ptr<ASTExpression> result = this->relationalExpression();
    if (!result) {
        return nullptr;
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::EQEQ:   // fall through
            case Token::NEQ: {
                Token t = this->nextToken();
                std::unique_ptr<ASTExpression> right = this->relationalExpression();
                if (!right) {
                    return nullptr;
                }
                result.reset(new ASTBinaryExpression(std::move(result), std::move(t), std::move(right)));
                break;
            }
            default:
                return result;
        }
    }
}

/* shiftExpression ((LT | GT | LTEQ | GTEQ) shiftExpression)* */
std::unique_ptr<ASTExpression> Parser::relationalExpression() {
    std::unique_ptr<ASTExpression> result = this->shiftExpression();
    if (!result) {
        return nullptr;
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::LT:   // fall through
            case Token::GT:   // fall through
            case Token::LTEQ: // fall through
            case Token::GTEQ: {
                Token t = this->nextToken();
                std::unique_ptr<ASTExpression> right = this->shiftExpression();
                if (!right) {
                    return nullptr;
                }
                result.reset(new ASTBinaryExpression(std::move(result), std::move(t),
                                                     std::move(right)));
                break;
            }
            default:
                return result;
        }
    }
}

/* additiveExpression ((SHL | SHR) additiveExpression)* */
std::unique_ptr<ASTExpression> Parser::shiftExpression() {
    std::unique_ptr<ASTExpression> result = this->additiveExpression();
    if (!result) {
        return nullptr;
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::SHL: // fall through
            case Token::SHR: {
                Token t = this->nextToken();
                std::unique_ptr<ASTExpression> right = this->additiveExpression();
                if (!right) {
                    return nullptr;
                }
                result.reset(new ASTBinaryExpression(std::move(result), std::move(t),
                                                     std::move(right)));
                break;
            }
            default:
                return result;
        }
    }
}

/* multiplicativeExpression ((PLUS | MINUS) multiplicativeExpression)* */
std::unique_ptr<ASTExpression> Parser::additiveExpression() {
    std::unique_ptr<ASTExpression> result = this->multiplicativeExpression();
    if (!result) {
        return nullptr;
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::PLUS: // fall through
            case Token::MINUS: {
                Token t = this->nextToken();
                std::unique_ptr<ASTExpression> right = this->multiplicativeExpression();
                if (!right) {
                    return nullptr;
                }
                result.reset(new ASTBinaryExpression(std::move(result), std::move(t),
                                                     std::move(right)));
                break;
            }
            default:
                return result;
        }
    }
}

/* unaryExpression ((STAR | SLASH | PERCENT) unaryExpression)* */
std::unique_ptr<ASTExpression> Parser::multiplicativeExpression() {
    std::unique_ptr<ASTExpression> result = this->unaryExpression();
    if (!result) {
        return nullptr;
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::STAR: // fall through
            case Token::SLASH: // fall through
            case Token::PERCENT: {
                Token t = this->nextToken();
                std::unique_ptr<ASTExpression> right = this->unaryExpression();
                if (!right) {
                    return nullptr;
                }
                result.reset(new ASTBinaryExpression(std::move(result), std::move(t),
                                                     std::move(right)));
                break;
            }
            default:
                return result;
        }
    }
}

/* postfixExpression | (PLUS | MINUS | NOT | PLUSPLUS | MINUSMINUS) unaryExpression */
std::unique_ptr<ASTExpression> Parser::unaryExpression() {
    switch (this->peek().fKind) {
        case Token::PLUS:       // fall through
        case Token::MINUS:      // fall through
        case Token::LOGICALNOT: // fall through
        case Token::BITWISENOT: // fall through
        case Token::PLUSPLUS:   // fall through
        case Token::MINUSMINUS: {
            AutoDepth depth(this);
            if (!depth.checkValid()) {
                return nullptr;
            }
            Token t = this->nextToken();
            std::unique_ptr<ASTExpression> expr = this->unaryExpression();
            if (!expr) {
                return nullptr;
            }
            return std::unique_ptr<ASTExpression>(new ASTPrefixExpression(std::move(t),
                                                                          std::move(expr)));
        }
        default:
            return this->postfixExpression();
    }
}

/* term suffix* */
std::unique_ptr<ASTExpression> Parser::postfixExpression() {
    std::unique_ptr<ASTExpression> result = this->term();
    if (!result) {
        return nullptr;
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::LBRACKET:   // fall through
            case Token::DOT:        // fall through
            case Token::LPAREN:     // fall through
            case Token::PLUSPLUS:   // fall through
            case Token::MINUSMINUS: // fall through
            case Token::COLONCOLON: {
                std::unique_ptr<ASTSuffix> s = this->suffix();
                if (!s) {
                    return nullptr;
                }
                result.reset(new ASTSuffixExpression(std::move(result), std::move(s)));
                break;
            }
            default:
                return result;
        }
    }
}

/* LBRACKET expression? RBRACKET | DOT IDENTIFIER | LPAREN parameters RPAREN |
   PLUSPLUS | MINUSMINUS | COLONCOLON IDENTIFIER */
std::unique_ptr<ASTSuffix> Parser::suffix() {
    Token next = this->nextToken();
    switch (next.fKind) {
        case Token::LBRACKET: {
            if (this->checkNext(Token::RBRACKET)) {
                return std::unique_ptr<ASTSuffix>(new ASTIndexSuffix(next.fOffset));
            }
            std::unique_ptr<ASTExpression> e = this->expression();
            if (!e) {
                return nullptr;
            }
            this->expect(Token::RBRACKET, "']' to complete array access expression");
            return std::unique_ptr<ASTSuffix>(new ASTIndexSuffix(std::move(e)));
        }
        case Token::DOT: // fall through
        case Token::COLONCOLON: {
            int offset = this->peek().fOffset;
            StringFragment text;
            if (this->identifier(&text)) {
                return std::unique_ptr<ASTSuffix>(new ASTFieldSuffix(offset, std::move(text)));
            }
            return nullptr;
        }
        case Token::LPAREN: {
            std::vector<std::unique_ptr<ASTExpression>> parameters;
            if (this->peek().fKind != Token::RPAREN) {
                for (;;) {
                    std::unique_ptr<ASTExpression> expr = this->assignmentExpression();
                    if (!expr) {
                        return nullptr;
                    }
                    parameters.push_back(std::move(expr));
                    if (!this->checkNext(Token::COMMA)) {
                        break;
                    }
                }
            }
            this->expect(Token::RPAREN, "')' to complete function parameters");
            return std::unique_ptr<ASTSuffix>(new ASTCallSuffix(next.fOffset,
                                                                std::move(parameters)));
        }
        case Token::PLUSPLUS:
            return std::unique_ptr<ASTSuffix>(new ASTSuffix(next.fOffset,
                                                            ASTSuffix::kPostIncrement_Kind));
        case Token::MINUSMINUS:
            return std::unique_ptr<ASTSuffix>(new ASTSuffix(next.fOffset,
                                                            ASTSuffix::kPostDecrement_Kind));
        default: {
            this->error(next,  "expected expression suffix, but found '" + this->text(next) +
                                         "'\n");
            return nullptr;
        }
    }
}

/* IDENTIFIER | intLiteral | floatLiteral | boolLiteral | NULL_LITERAL | '(' expression ')' */
std::unique_ptr<ASTExpression> Parser::term() {
    std::unique_ptr<ASTExpression> result;
    Token t = this->peek();
    switch (t.fKind) {
        case Token::IDENTIFIER: {
            StringFragment text;
            if (this->identifier(&text)) {
                result.reset(new ASTIdentifier(t.fOffset, std::move(text)));
            }
            break;
        }
        case Token::INT_LITERAL: {
            int64_t i;
            if (this->intLiteral(&i)) {
                result.reset(new ASTIntLiteral(t.fOffset, i));
            }
            break;
        }
        case Token::FLOAT_LITERAL: {
            double f;
            if (this->floatLiteral(&f)) {
                result.reset(new ASTFloatLiteral(t.fOffset, f));
            }
            break;
        }
        case Token::TRUE_LITERAL: // fall through
        case Token::FALSE_LITERAL: {
            bool b;
            if (this->boolLiteral(&b)) {
                result.reset(new ASTBoolLiteral(t.fOffset, b));
            }
            break;
        }
        case Token::NULL_LITERAL:
            this->nextToken();
            result.reset(new ASTNullLiteral(t.fOffset));
            break;
        case Token::LPAREN: {
            this->nextToken();
            result = this->expression();
            if (result) {
                this->expect(Token::RPAREN, "')' to complete expression");
            }
            break;
        }
        default:
            this->nextToken();
            this->error(t.fOffset,  "expected expression, but found '" + this->text(t) + "'\n");
            result = nullptr;
    }
    return result;
}

/* INT_LITERAL */
bool Parser::intLiteral(int64_t* dest) {
    Token t;
    if (this->expect(Token::INT_LITERAL, "integer literal", &t)) {
        *dest = SkSL::stol(this->text(t));
        return true;
    }
    return false;
}

/* FLOAT_LITERAL */
bool Parser::floatLiteral(double* dest) {
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
