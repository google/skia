/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "stdio.h"
#include "src/sksl/SkSLASTNode.h"
#include "src/sksl/SkSLParser.h"
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

#define CREATE_NODE(result, ...)              \
    ASTNode::ID result(fFile->fNodes.size()); \
    fFile->fNodes.emplace_back(&fFile->fNodes, __VA_ARGS__)

#define RETURN_NODE(...)                  \
    do {                                  \
        CREATE_NODE(result, __VA_ARGS__); \
        return result;                    \
    } while (false)

#define CREATE_CHILD(child, target, ...)   \
    CREATE_NODE(child, __VA_ARGS__);       \
    fFile->fNodes[target.fValue].addChild(child)

#define CREATE_EMPTY_CHILD(target)                    \
    do {                                              \
        ASTNode::ID child(fFile->fNodes.size());      \
        fFile->fNodes.emplace_back();                 \
        fFile->fNodes[target.fValue].addChild(child); \
    } while (false)

/* (directive | section | declaration)* END_OF_FILE */
std::unique_ptr<ASTFile> Parser::file() {
    fFile.reset(new ASTFile());
    CREATE_NODE(result, 0, ASTNode::Kind::kFile);
    fFile->fRoot = result;
    for (;;) {
        switch (this->peek().fKind) {
            case Token::END_OF_FILE:
                return std::move(fFile);
            case Token::DIRECTIVE: {
                ASTNode::ID dir = this->directive();
                if (fErrors.errorCount()) {
                    return nullptr;
                }
                if (dir) {
                    getNode(result).addChild(dir);
                }
                break;
            }
            case Token::SECTION: {
                ASTNode::ID section = this->section();
                if (fErrors.errorCount()) {
                    return nullptr;
                }
                if (section) {
                    getNode(result).addChild(section);
                }
                break;
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
ASTNode::ID Parser::directive() {
    Token start;
    if (!this->expect(Token::DIRECTIVE, "a directive", &start)) {
        return ASTNode::ID::Invalid();
    }
    StringFragment text = this->text(start);
    if (text == "#extension") {
        Token name;
        if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
            return ASTNode::ID::Invalid();
        }
        if (!this->expect(Token::COLON, "':'")) {
            return ASTNode::ID::Invalid();
        }
        // FIXME: need to start paying attention to this token
        if (!this->expect(Token::IDENTIFIER, "an identifier")) {
            return ASTNode::ID::Invalid();
        }
        RETURN_NODE(start.fOffset, ASTNode::Kind::kExtension, this->text(name));
    } else {
        this->error(start, "unsupported directive '" + this->text(start) + "'");
        return ASTNode::ID::Invalid();
    }
}

/* SECTION LBRACE (LPAREN IDENTIFIER RPAREN)? <any sequence of tokens with balanced braces>
   RBRACE */
ASTNode::ID Parser::section() {
    Token start;
    if (!this->expect(Token::SECTION, "a section token", &start)) {
        return ASTNode::ID::Invalid();
    }
    StringFragment argument;
    if (this->peek().fKind == Token::LPAREN) {
        this->nextToken();
        Token argToken;
        if (!this->expect(Token::IDENTIFIER, "an identifier", &argToken)) {
            return ASTNode::ID::Invalid();
        }
        argument = this->text(argToken);
        if (!this->expect(Token::RPAREN, "')'")) {
            return ASTNode::ID::Invalid();
        }
    }
    if (!this->expect(Token::LBRACE, "'{'")) {
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
            case Token::LBRACE:
                ++level;
                break;
            case Token::RBRACE:
                --level;
                break;
            case Token::END_OF_FILE:
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
    RETURN_NODE(start.fOffset, ASTNode::Kind::kSection,
                ASTNode::SectionData(name, argument, text));
}

/* ENUM CLASS IDENTIFIER LBRACE (IDENTIFIER (EQ expression)? (COMMA IDENTIFIER (EQ expression))*)?
   RBRACE */
ASTNode::ID Parser::enumDeclaration() {
    Token start;
    if (!this->expect(Token::ENUM, "'enum'", &start)) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::CLASS, "'class'")) {
        return ASTNode::ID::Invalid();
    }
    Token name;
    if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::LBRACE, "'{'")) {
        return ASTNode::ID::Invalid();
    }
    fTypes.add(this->text(name), std::unique_ptr<Symbol>(new Type(this->text(name),
                                                                  Type::kEnum_Kind)));
    CREATE_NODE(result, name.fOffset, ASTNode::Kind::kEnum, this->text(name));
    if (!this->checkNext(Token::RBRACE)) {
        Token id;
        if (!this->expect(Token::IDENTIFIER, "an identifier", &id)) {
            return ASTNode::ID::Invalid();
        }
        if (this->checkNext(Token::EQ)) {
            ASTNode::ID value = this->assignmentExpression();
            if (!value) {
                return ASTNode::ID::Invalid();
            }
            CREATE_CHILD(child, result, id.fOffset, ASTNode::Kind::kEnumCase, this->text(id));
            getNode(child).addChild(value);
        } else {
            CREATE_CHILD(child, result, id.fOffset, ASTNode::Kind::kEnumCase, this->text(id));
        }
        while (!this->checkNext(Token::RBRACE)) {
            if (!this->expect(Token::COMMA, "','")) {
                return ASTNode::ID::Invalid();
            }
            if (!this->expect(Token::IDENTIFIER, "an identifier", &id)) {
                return ASTNode::ID::Invalid();
            }
            if (this->checkNext(Token::EQ)) {
                ASTNode::ID value = this->assignmentExpression();
                if (!value) {
                    return ASTNode::ID::Invalid();
                }
                CREATE_CHILD(child, result, id.fOffset, ASTNode::Kind::kEnumCase, this->text(id));
                getNode(child).addChild(value);
            } else {
                CREATE_CHILD(child, result, id.fOffset, ASTNode::Kind::kEnumCase, this->text(id));
            }
        }
    }
    this->expect(Token::SEMICOLON, "';'");
    return result;
}

/* enumDeclaration | modifiers (structVarDeclaration | type IDENTIFIER ((LPAREN parameter
   (COMMA parameter)* RPAREN (block | SEMICOLON)) | SEMICOLON) | interfaceBlock) */
ASTNode::ID Parser::declaration() {
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
        RETURN_NODE(lookahead.fOffset, ASTNode::Kind::kModifiers, modifiers);
    }
    ASTNode::ID type = this->type();
    if (!type) {
        return ASTNode::ID::Invalid();
    }
    if (getNode(type).getTypeData().fIsStructDeclaration && this->checkNext(Token::SEMICOLON)) {
        return ASTNode::ID::Invalid();
    }
    Token name;
    if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
        return ASTNode::ID::Invalid();
    }
    if (this->checkNext(Token::LPAREN)) {
        CREATE_NODE(result, name.fOffset, ASTNode::Kind::kFunction);
        ASTNode::FunctionData fd(modifiers, this->text(name), 0);
        getNode(result).addChild(type);
        if (this->peek().fKind != Token::RPAREN) {
            for (;;) {
                ASTNode::ID parameter = this->parameter();
                if (!parameter) {
                    return ASTNode::ID::Invalid();
                }
                ++fd.fParameterCount;
                getNode(result).addChild(parameter);
                if (!this->checkNext(Token::COMMA)) {
                    break;
                }
            }
        }
        getNode(result).setFunctionData(fd);
        if (!this->expect(Token::RPAREN, "')'")) {
            return ASTNode::ID::Invalid();
        }
        ASTNode::ID body;
        if (!this->checkNext(Token::SEMICOLON)) {
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

/* modifiers type IDENTIFIER varDeclarationEnd */
ASTNode::ID Parser::varDeclarations() {
    Modifiers modifiers = this->modifiers();
    ASTNode::ID type = this->type();
    if (!type) {
        return ASTNode::ID::Invalid();
    }
    Token name;
    if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
        return ASTNode::ID::Invalid();
    }
    return this->varDeclarationEnd(modifiers, type, this->text(name));
}

/* STRUCT IDENTIFIER LBRACE varDeclaration* RBRACE */
ASTNode::ID Parser::structDeclaration() {
    if (!this->expect(Token::STRUCT, "'struct'")) {
        return ASTNode::ID::Invalid();
    }
    Token name;
    if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::LBRACE, "'{'")) {
        return ASTNode::ID::Invalid();
    }
    std::vector<Type::Field> fields;
    while (this->peek().fKind != Token::RBRACE) {
        ASTNode::ID decls = this->varDeclarations();
        if (!decls) {
            return ASTNode::ID::Invalid();
        }
        ASTNode& declsNode = getNode(decls);
        auto type = (const Type*) fTypes[(declsNode.begin() + 1)->getTypeData().fName];
        for (auto iter = declsNode.begin() + 2; iter != declsNode.end(); ++iter) {
            ASTNode& var = *iter;
            ASTNode::VarData vd = var.getVarData();
            for (int j = vd.fSizeCount - 1; j >= 0; j--) {
                const ASTNode& size = *(var.begin() + j);
                if (!size || size.fKind != ASTNode::Kind::kInt) {
                    this->error(declsNode.fOffset, "array size in struct field must be a constant");
                    return ASTNode::ID::Invalid();
                }
                uint64_t columns = size.getInt();
                String name = type->name() + "[" + to_string(columns) + "]";
                type = (Type*) fTypes.takeOwnership(std::unique_ptr<Symbol>(
                                                                         new Type(name,
                                                                                  Type::kArray_Kind,
                                                                                  *type,
                                                                                  (int) columns)));
            }
            fields.push_back(Type::Field(declsNode.begin()->getModifiers(), vd.fName, type));
            if (vd.fSizeCount ? (var.begin() + (vd.fSizeCount - 1))->fNext : var.fFirstChild) {
                this->error(declsNode.fOffset, "initializers are not permitted on struct fields");
            }
        }
    }
    if (!this->expect(Token::RBRACE, "'}'")) {
        return ASTNode::ID::Invalid();
    }
    fTypes.add(this->text(name), std::unique_ptr<Type>(new Type(name.fOffset, this->text(name),
                                                                fields)));
    RETURN_NODE(name.fOffset, ASTNode::Kind::kType,
                ASTNode::TypeData(this->text(name), true, false));
}

/* structDeclaration ((IDENTIFIER varDeclarationEnd) | SEMICOLON) */
ASTNode::ID Parser::structVarDeclaration(Modifiers modifiers) {
    ASTNode::ID type = this->structDeclaration();
    if (!type) {
        return ASTNode::ID::Invalid();
    }
    Token name;
    if (this->checkNext(Token::IDENTIFIER, &name)) {
        return this->varDeclarationEnd(modifiers, std::move(type), this->text(name));
    }
    this->expect(Token::SEMICOLON, "';'");
    return ASTNode::ID::Invalid();
}

/* (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)? (COMMA IDENTIFER
   (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)?)* SEMICOLON */
ASTNode::ID Parser::varDeclarationEnd(Modifiers mods, ASTNode::ID type, StringFragment name) {
    CREATE_NODE(result, -1, ASTNode::Kind::kVarDeclarations);
    CREATE_CHILD(modifiers, result, -1, ASTNode::Kind::kModifiers, mods);
    getNode(result).addChild(type);
    CREATE_NODE(currentVar, -1, ASTNode::Kind::kVarDeclaration);
    ASTNode::VarData vd(name, 0);
    getNode(result).addChild(currentVar);
    while (this->checkNext(Token::LBRACKET)) {
        if (this->checkNext(Token::RBRACKET)) {
            CREATE_EMPTY_CHILD(currentVar);
        } else {
            ASTNode::ID size = this->expression();
            if (!size) {
                return ASTNode::ID::Invalid();
            }
            getNode(currentVar).addChild(size);
            if (!this->expect(Token::RBRACKET, "']'")) {
                return ASTNode::ID::Invalid();
            }
        }
        ++vd.fSizeCount;
    }
    getNode(currentVar).setVarData(vd);
    if (this->checkNext(Token::EQ)) {
        ASTNode::ID value = this->assignmentExpression();
        if (!value) {
            return ASTNode::ID::Invalid();
        }
        getNode(currentVar).addChild(value);
    }
    while (this->checkNext(Token::COMMA)) {
        Token name;
        if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
            return ASTNode::ID::Invalid();
        }
        currentVar = ASTNode::ID(fFile->fNodes.size());
        vd = ASTNode::VarData(this->text(name), 0);
        fFile->fNodes.emplace_back(&fFile->fNodes, -1, ASTNode::Kind::kVarDeclaration);
        getNode(result).addChild(currentVar);
        while (this->checkNext(Token::LBRACKET)) {
            if (this->checkNext(Token::RBRACKET)) {
                CREATE_EMPTY_CHILD(currentVar);
            } else {
                ASTNode::ID size = this->expression();
                if (!size) {
                    return ASTNode::ID::Invalid();
                }
                getNode(currentVar).addChild(size);
                if (!this->expect(Token::RBRACKET, "']'")) {
                    return ASTNode::ID::Invalid();
                }
            }
            ++vd.fSizeCount;
        }
        getNode(currentVar).setVarData(vd);
        if (this->checkNext(Token::EQ)) {
            ASTNode::ID value = this->assignmentExpression();
            if (!value) {
                return ASTNode::ID::Invalid();
            }
            getNode(currentVar).addChild(value);
        }
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
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
    if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
        return ASTNode::ID::Invalid();
    }
    CREATE_NODE(result, name.fOffset, ASTNode::Kind::kParameter);
    ASTNode::ParameterData pd(modifiers, this->text(name), 0);
    getNode(result).addChild(type);
    while (this->checkNext(Token::LBRACKET)) {
        Token sizeToken;
        if (!this->expect(Token::INT_LITERAL, "a positive integer", &sizeToken)) {
            return ASTNode::ID::Invalid();
        }
        CREATE_CHILD(child, result, sizeToken.fOffset, ASTNode::Kind::kInt,
                     SkSL::stoi(this->text(sizeToken)));
        if (!this->expect(Token::RBRACKET, "']'")) {
            return ASTNode::ID::Invalid();
        }
        ++pd.fSizeCount;
    }
    getNode(result).setParameterData(pd);
    return result;
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
ASTNode::ID Parser::statement() {
    Token start = this->nextToken();
    AutoDepth depth(this);
    if (!depth.checkValid()) {
        return ASTNode::ID::Invalid();
    }
    this->pushback(start);
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
            RETURN_NODE(start.fOffset, ASTNode::Kind::kBlock);
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
ASTNode::ID Parser::type() {
    Token type;
    if (!this->expect(Token::IDENTIFIER, "a type", &type)) {
        return ASTNode::ID::Invalid();
    }
    if (!this->isType(this->text(type))) {
        this->error(type, ("no type named '" + this->text(type) + "'").c_str());
        return ASTNode::ID::Invalid();
    }
    CREATE_NODE(result, type.fOffset, ASTNode::Kind::kType);
    ASTNode::TypeData td(this->text(type), false, false);
    while (this->checkNext(Token::LBRACKET)) {
        if (this->peek().fKind != Token::RBRACKET) {
            SKSL_INT i;
            if (this->intLiteral(&i)) {
                CREATE_CHILD(child, result, -1, ASTNode::Kind::kInt, i);
            } else {
                return ASTNode::ID::Invalid();
            }
        } else {
            CREATE_EMPTY_CHILD(result);
        }
        this->expect(Token::RBRACKET, "']'");
    }
    td.fIsNullable = this->checkNext(Token::QUESTION);
    getNode(result).setTypeData(td);
    return result;
}

/* IDENTIFIER LBRACE varDeclaration* RBRACE (IDENTIFIER (LBRACKET expression? RBRACKET)*)? */
ASTNode::ID Parser::interfaceBlock(Modifiers mods) {
    Token name;
    if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
        return ASTNode::ID::Invalid();
    }
    if (peek().fKind != Token::LBRACE) {
        // we only get into interfaceBlock if we found a top-level identifier which was not a type.
        // 99% of the time, the user was not actually intending to create an interface block, so
        // it's better to report it as an unknown type
        this->error(name, "no type named '" + this->text(name) + "'");
        return ASTNode::ID::Invalid();
    }
    CREATE_NODE(result, name.fOffset, ASTNode::Kind::kInterfaceBlock);
    ASTNode::InterfaceBlockData id(mods, this->text(name), 0, "", 0);
    this->nextToken();
    while (this->peek().fKind != Token::RBRACE) {
        ASTNode::ID decl = this->varDeclarations();
        if (!decl) {
            return ASTNode::ID::Invalid();
        }
        getNode(result).addChild(decl);
        ++id.fDeclarationCount;
    }
    this->nextToken();
    std::vector<ASTNode> sizes;
    StringFragment instanceName;
    Token instanceNameToken;
    if (this->checkNext(Token::IDENTIFIER, &instanceNameToken)) {
        id.fInstanceName = this->text(instanceNameToken);
        while (this->checkNext(Token::LBRACKET)) {
            if (this->peek().fKind != Token::RBRACKET) {
                ASTNode::ID size = this->expression();
                if (!size) {
                    return ASTNode::ID::Invalid();
                }
                getNode(result).addChild(size);
            } else {
                CREATE_EMPTY_CHILD(result);
            }
            ++id.fSizeCount;
            this->expect(Token::RBRACKET, "']'");
        }
        instanceName = this->text(instanceNameToken);
    }
    getNode(result).setInterfaceBlockData(id);
    this->expect(Token::SEMICOLON, "';'");
    return result;
}

/* IF LPAREN expression RPAREN statement (ELSE statement)? */
ASTNode::ID Parser::ifStatement() {
    Token start;
    bool isStatic = this->checkNext(Token::STATIC_IF, &start);
    if (!isStatic && !this->expect(Token::IF, "'if'", &start)) {
        return ASTNode::ID::Invalid();
    }
    CREATE_NODE(result, start.fOffset, ASTNode::Kind::kIf, isStatic);
    if (!this->expect(Token::LPAREN, "'('")) {
        return ASTNode::ID::Invalid();
    }
    ASTNode::ID test = this->expression();
    if (!test) {
        return ASTNode::ID::Invalid();
    }
    getNode(result).addChild(test);
    if (!this->expect(Token::RPAREN, "')'")) {
        return ASTNode::ID::Invalid();
    }
    ASTNode::ID ifTrue = this->statement();
    if (!ifTrue) {
        return ASTNode::ID::Invalid();
    }
    getNode(result).addChild(ifTrue);
    ASTNode::ID ifFalse;
    if (this->checkNext(Token::ELSE)) {
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
    if (!this->expect(Token::DO, "'do'", &start)) {
        return ASTNode::ID::Invalid();
    }
    CREATE_NODE(result, start.fOffset, ASTNode::Kind::kDo);
    ASTNode::ID statement = this->statement();
    if (!statement) {
        return ASTNode::ID::Invalid();
    }
    getNode(result).addChild(statement);
    if (!this->expect(Token::WHILE, "'while'")) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::LPAREN, "'('")) {
        return ASTNode::ID::Invalid();
    }
    ASTNode::ID test = this->expression();
    if (!test) {
        return ASTNode::ID::Invalid();
    }
    getNode(result).addChild(test);
    if (!this->expect(Token::RPAREN, "')'")) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return ASTNode::ID::Invalid();
    }
    return result;
}

/* WHILE LPAREN expression RPAREN STATEMENT */
ASTNode::ID Parser::whileStatement() {
    Token start;
    if (!this->expect(Token::WHILE, "'while'", &start)) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::LPAREN, "'('")) {
        return ASTNode::ID::Invalid();
    }
    CREATE_NODE(result, start.fOffset, ASTNode::Kind::kWhile);
    ASTNode::ID test = this->expression();
    if (!test) {
        return ASTNode::ID::Invalid();
    }
    getNode(result).addChild(test);
    if (!this->expect(Token::RPAREN, "')'")) {
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
    if (!this->expect(Token::CASE, "'case'", &start)) {
        return ASTNode::ID::Invalid();
    }
    CREATE_NODE(result, start.fOffset, ASTNode::Kind::kSwitchCase);
    ASTNode::ID value = this->expression();
    if (!value) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::COLON, "':'")) {
        return ASTNode::ID::Invalid();
    }
    getNode(result).addChild(value);
    while (this->peek().fKind != Token::RBRACE && this->peek().fKind != Token::CASE &&
           this->peek().fKind != Token::DEFAULT) {
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
    bool isStatic = this->checkNext(Token::STATIC_SWITCH, &start);
    if (!isStatic && !this->expect(Token::SWITCH, "'switch'", &start)) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::LPAREN, "'('")) {
        return ASTNode::ID::Invalid();
    }
    ASTNode::ID value = this->expression();
    if (!value) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::RPAREN, "')'")) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::LBRACE, "'{'")) {
        return ASTNode::ID::Invalid();
    }
    CREATE_NODE(result, start.fOffset, ASTNode::Kind::kSwitch, isStatic);
    getNode(result).addChild(value);
    while (this->peek().fKind == Token::CASE) {
        ASTNode::ID c = this->switchCase();
        if (!c) {
            return ASTNode::ID::Invalid();
        }
        getNode(result).addChild(c);
    }
    // Requiring default: to be last (in defiance of C and GLSL) was a deliberate decision. Other
    // parts of the compiler may rely upon this assumption.
    if (this->peek().fKind == Token::DEFAULT) {
        Token defaultStart;
        SkAssertResult(this->expect(Token::DEFAULT, "'default'", &defaultStart));
        if (!this->expect(Token::COLON, "':'")) {
            return ASTNode::ID::Invalid();
        }
        CREATE_CHILD(defaultCase, result, defaultStart.fOffset, ASTNode::Kind::kSwitchCase);
        CREATE_EMPTY_CHILD(defaultCase); // empty test to signify default case
        while (this->peek().fKind != Token::RBRACE) {
            ASTNode::ID s = this->statement();
            if (!s) {
                return ASTNode::ID::Invalid();
            }
            getNode(defaultCase).addChild(s);
        }
    }
    if (!this->expect(Token::RBRACE, "'}'")) {
        return ASTNode::ID::Invalid();
    }
    return result;
}

/* FOR LPAREN (declaration | expression)? SEMICOLON expression? SEMICOLON expression? RPAREN
   STATEMENT */
ASTNode::ID Parser::forStatement() {
    Token start;
    if (!this->expect(Token::FOR, "'for'", &start)) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::LPAREN, "'('")) {
        return ASTNode::ID::Invalid();
    }
    CREATE_NODE(result, start.fOffset, ASTNode::Kind::kFor);
    ASTNode::ID initializer;
    Token nextToken = this->peek();
    switch (nextToken.fKind) {
        case Token::SEMICOLON:
            this->nextToken();
            CREATE_EMPTY_CHILD(result);
            break;
        case Token::CONST: {
            initializer = this->varDeclarations();
            if (!initializer) {
                return ASTNode::ID::Invalid();
            }
            getNode(result).addChild(initializer);
            break;
        }
        case Token::IDENTIFIER: {
            if (this->isType(this->text(nextToken))) {
                initializer = this->varDeclarations();
                if (!initializer) {
                    return ASTNode::ID::Invalid();
                }
                getNode(result).addChild(initializer);
                break;
            }
        } // fall through
        default:
            initializer = this->expressionStatement();
            if (!initializer) {
                return ASTNode::ID::Invalid();
            }
            getNode(result).addChild(initializer);
    }
    ASTNode::ID test;
    if (this->peek().fKind != Token::SEMICOLON) {
        test = this->expression();
        if (!test) {
            return ASTNode::ID::Invalid();
        }
        getNode(result).addChild(test);
    } else {
        CREATE_EMPTY_CHILD(result);
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return ASTNode::ID::Invalid();
    }
    ASTNode::ID next;
    if (this->peek().fKind != Token::RPAREN) {
        next = this->expression();
        if (!next) {
            return ASTNode::ID::Invalid();
        }
        getNode(result).addChild(next);
    } else {
        CREATE_EMPTY_CHILD(result);
    }
    if (!this->expect(Token::RPAREN, "')'")) {
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
    if (!this->expect(Token::RETURN, "'return'", &start)) {
        return ASTNode::ID::Invalid();
    }
    CREATE_NODE(result, start.fOffset, ASTNode::Kind::kReturn);
    if (this->peek().fKind != Token::SEMICOLON) {
        ASTNode::ID expression = this->expression();
        if (!expression) {
            return ASTNode::ID::Invalid();
        }
        getNode(result).addChild(expression);
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return ASTNode::ID::Invalid();
    }
    return result;
}

/* BREAK SEMICOLON */
ASTNode::ID Parser::breakStatement() {
    Token start;
    if (!this->expect(Token::BREAK, "'break'", &start)) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return ASTNode::ID::Invalid();
    }
    RETURN_NODE(start.fOffset, ASTNode::Kind::kBreak);
}

/* CONTINUE SEMICOLON */
ASTNode::ID Parser::continueStatement() {
    Token start;
    if (!this->expect(Token::CONTINUE, "'continue'", &start)) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return ASTNode::ID::Invalid();
    }
    RETURN_NODE(start.fOffset, ASTNode::Kind::kContinue);
}

/* DISCARD SEMICOLON */
ASTNode::ID Parser::discardStatement() {
    Token start;
    if (!this->expect(Token::DISCARD, "'continue'", &start)) {
        return ASTNode::ID::Invalid();
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return ASTNode::ID::Invalid();
    }
    RETURN_NODE(start.fOffset, ASTNode::Kind::kDiscard);
}

/* LBRACE statement* RBRACE */
ASTNode::ID Parser::block() {
    Token start;
    if (!this->expect(Token::LBRACE, "'{'", &start)) {
        return ASTNode::ID::Invalid();
    }
    AutoDepth depth(this);
    if (!depth.checkValid()) {
        return ASTNode::ID::Invalid();
    }
    CREATE_NODE(result, start.fOffset, ASTNode::Kind::kBlock);
    for (;;) {
        switch (this->peek().fKind) {
            case Token::RBRACE:
                this->nextToken();
                return result;
            case Token::END_OF_FILE:
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
        if (this->expect(Token::SEMICOLON, "';'")) {
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
    while (this->checkNext(Token::COMMA, &t)) {
        ASTNode::ID right = this->assignmentExpression();
        if (!right) {
            return ASTNode::ID::Invalid();
        }
        CREATE_NODE(newResult, t.fOffset, ASTNode::Kind::kBinary, std::move(t));
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
    ASTNode::ID result = this->ternaryExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
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
                ASTNode::ID right = this->assignmentExpression();
                if (!right) {
                    return ASTNode::ID::Invalid();
                }
                CREATE_NODE(newResult, getNode(result).fOffset, ASTNode::Kind::kBinary,
                            std::move(t));
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
    ASTNode::ID base = this->logicalOrExpression();
    if (!base) {
        return ASTNode::ID::Invalid();
    }
    if (this->checkNext(Token::QUESTION)) {
        ASTNode::ID trueExpr = this->expression();
        if (!trueExpr) {
            return ASTNode::ID::Invalid();
        }
        if (this->expect(Token::COLON, "':'")) {
            ASTNode::ID falseExpr = this->assignmentExpression();
            if (!falseExpr) {
                return ASTNode::ID::Invalid();
            }
            CREATE_NODE(ternary, getNode(base).fOffset, ASTNode::Kind::kTernary);
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
    ASTNode::ID result = this->logicalXorExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    Token t;
    while (this->checkNext(Token::LOGICALOR, &t)) {
        ASTNode::ID right = this->logicalXorExpression();
        if (!right) {
            return ASTNode::ID::Invalid();
        }
        CREATE_NODE(newResult, getNode(result).fOffset, ASTNode::Kind::kBinary, std::move(t));
        getNode(newResult).addChild(result);
        getNode(newResult).addChild(right);
        result = newResult;
    }
    return result;
}

/* logicalAndExpression (LOGICALXOR logicalAndExpression)* */
ASTNode::ID Parser::logicalXorExpression() {
    ASTNode::ID result = this->logicalAndExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    Token t;
    while (this->checkNext(Token::LOGICALXOR, &t)) {
        ASTNode::ID right = this->logicalAndExpression();
        if (!right) {
            return ASTNode::ID::Invalid();
        }
        CREATE_NODE(newResult, getNode(result).fOffset, ASTNode::Kind::kBinary, std::move(t));
        getNode(newResult).addChild(result);
        getNode(newResult).addChild(right);
        result = newResult;
    }
    return result;
}

/* bitwiseOrExpression (LOGICALAND bitwiseOrExpression)* */
ASTNode::ID Parser::logicalAndExpression() {
    ASTNode::ID result = this->bitwiseOrExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    Token t;
    while (this->checkNext(Token::LOGICALAND, &t)) {
        ASTNode::ID right = this->bitwiseOrExpression();
        if (!right) {
            return ASTNode::ID::Invalid();
        }
        CREATE_NODE(newResult, getNode(result).fOffset, ASTNode::Kind::kBinary, std::move(t));
        getNode(newResult).addChild(result);
        getNode(newResult).addChild(right);
        result = newResult;
    }
    return result;
}

/* bitwiseXorExpression (BITWISEOR bitwiseXorExpression)* */
ASTNode::ID Parser::bitwiseOrExpression() {
    ASTNode::ID result = this->bitwiseXorExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    Token t;
    while (this->checkNext(Token::BITWISEOR, &t)) {
        ASTNode::ID right = this->bitwiseXorExpression();
        if (!right) {
            return ASTNode::ID::Invalid();
        }
        CREATE_NODE(newResult, getNode(result).fOffset, ASTNode::Kind::kBinary, std::move(t));
        getNode(newResult).addChild(result);
        getNode(newResult).addChild(right);
        result = newResult;
    }
    return result;
}

/* bitwiseAndExpression (BITWISEXOR bitwiseAndExpression)* */
ASTNode::ID Parser::bitwiseXorExpression() {
    ASTNode::ID result = this->bitwiseAndExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    Token t;
    while (this->checkNext(Token::BITWISEXOR, &t)) {
        ASTNode::ID right = this->bitwiseAndExpression();
        if (!right) {
            return ASTNode::ID::Invalid();
        }
        CREATE_NODE(newResult, getNode(result).fOffset, ASTNode::Kind::kBinary, std::move(t));
        getNode(newResult).addChild(result);
        getNode(newResult).addChild(right);
        result = newResult;
    }
    return result;
}

/* equalityExpression (BITWISEAND equalityExpression)* */
ASTNode::ID Parser::bitwiseAndExpression() {
    ASTNode::ID result = this->equalityExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    Token t;
    while (this->checkNext(Token::BITWISEAND, &t)) {
        ASTNode::ID right = this->equalityExpression();
        if (!right) {
            return ASTNode::ID::Invalid();
        }
        CREATE_NODE(newResult, getNode(result).fOffset, ASTNode::Kind::kBinary, std::move(t));
        getNode(newResult).addChild(result);
        getNode(newResult).addChild(right);
        result = newResult;
    }
    return result;
}

/* relationalExpression ((EQEQ | NEQ) relationalExpression)* */
ASTNode::ID Parser::equalityExpression() {
    ASTNode::ID result = this->relationalExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::EQEQ:   // fall through
            case Token::NEQ: {
                Token t = this->nextToken();
                ASTNode::ID right = this->relationalExpression();
                if (!right) {
                    return ASTNode::ID::Invalid();
                }
                CREATE_NODE(newResult, getNode(result).fOffset, ASTNode::Kind::kBinary,
                            std::move(t));
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
    ASTNode::ID result = this->shiftExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::LT:   // fall through
            case Token::GT:   // fall through
            case Token::LTEQ: // fall through
            case Token::GTEQ: {
                Token t = this->nextToken();
                ASTNode::ID right = this->shiftExpression();
                if (!right) {
                    return ASTNode::ID::Invalid();
                }
                CREATE_NODE(newResult, getNode(result).fOffset, ASTNode::Kind::kBinary,
                            std::move(t));
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
    ASTNode::ID result = this->additiveExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::SHL: // fall through
            case Token::SHR: {
                Token t = this->nextToken();
                ASTNode::ID right = this->additiveExpression();
                if (!right) {
                    return ASTNode::ID::Invalid();
                }
                CREATE_NODE(newResult, getNode(result).fOffset, ASTNode::Kind::kBinary,
                            std::move(t));
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
    ASTNode::ID result = this->multiplicativeExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::PLUS: // fall through
            case Token::MINUS: {
                Token t = this->nextToken();
                ASTNode::ID right = this->multiplicativeExpression();
                if (!right) {
                    return ASTNode::ID::Invalid();
                }
                CREATE_NODE(newResult, getNode(result).fOffset, ASTNode::Kind::kBinary,
                            std::move(t));
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
    ASTNode::ID result = this->unaryExpression();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::STAR: // fall through
            case Token::SLASH: // fall through
            case Token::PERCENT: {
                Token t = this->nextToken();
                ASTNode::ID right = this->unaryExpression();
                if (!right) {
                    return ASTNode::ID::Invalid();
                }
                CREATE_NODE(newResult, getNode(result).fOffset, ASTNode::Kind::kBinary,
                            std::move(t));
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
    switch (this->peek().fKind) {
        case Token::PLUS:       // fall through
        case Token::MINUS:      // fall through
        case Token::LOGICALNOT: // fall through
        case Token::BITWISENOT: // fall through
        case Token::PLUSPLUS:   // fall through
        case Token::MINUSMINUS: {
            Token t = this->nextToken();
            AutoDepth depth(this);
            if (!depth.checkValid()) {
                return ASTNode::ID::Invalid();
            }
            ASTNode::ID expr = this->unaryExpression();
            if (!expr) {
                return ASTNode::ID::Invalid();
            }
            CREATE_NODE(result, t.fOffset, ASTNode::Kind::kPrefix, std::move(t));
            getNode(result).addChild(expr);
            return result;
        }
        default:
            return this->postfixExpression();
    }
}

/* term suffix* */
ASTNode::ID Parser::postfixExpression() {
    ASTNode::ID result = this->term();
    if (!result) {
        return ASTNode::ID::Invalid();
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::LBRACKET:   // fall through
            case Token::DOT:        // fall through
            case Token::LPAREN:     // fall through
            case Token::PLUSPLUS:   // fall through
            case Token::MINUSMINUS: // fall through
            case Token::COLONCOLON:
                result = this->suffix(result);
                break;
            default:
                return result;
        }
    }
}

/* LBRACKET expression? RBRACKET | DOT IDENTIFIER | LPAREN parameters RPAREN |
   PLUSPLUS | MINUSMINUS | COLONCOLON IDENTIFIER */
ASTNode::ID Parser::suffix(ASTNode::ID base) {
    Token next = this->nextToken();
    AutoDepth depth(this);
    if (!depth.checkValid()) {
        return ASTNode::ID::Invalid();
    }
    switch (next.fKind) {
        case Token::LBRACKET: {
            if (this->checkNext(Token::RBRACKET)) {
                CREATE_NODE(result, next.fOffset, ASTNode::Kind::kIndex);
                getNode(result).addChild(base);
                return result;
            }
            ASTNode::ID e = this->expression();
            if (!e) {
                return ASTNode::ID::Invalid();
            }
            this->expect(Token::RBRACKET, "']' to complete array access expression");
            CREATE_NODE(result, next.fOffset, ASTNode::Kind::kIndex);
            getNode(result).addChild(base);
            getNode(result).addChild(e);
            return result;
        }
        case Token::DOT: // fall through
        case Token::COLONCOLON: {
            int offset = this->peek().fOffset;
            StringFragment text;
            if (this->identifier(&text)) {
                CREATE_NODE(result, offset, ASTNode::Kind::kField, std::move(text));
                getNode(result).addChild(base);
                return result;
            }
            return ASTNode::ID::Invalid();
        }
        case Token::LPAREN: {
            CREATE_NODE(result, next.fOffset, ASTNode::Kind::kCall);
            getNode(result).addChild(base);
            if (this->peek().fKind != Token::RPAREN) {
                for (;;) {
                    ASTNode::ID expr = this->assignmentExpression();
                    if (!expr) {
                        return ASTNode::ID::Invalid();
                    }
                    getNode(result).addChild(expr);
                    if (!this->checkNext(Token::COMMA)) {
                        break;
                    }
                }
            }
            this->expect(Token::RPAREN, "')' to complete function parameters");
            return result;
        }
        case Token::PLUSPLUS: // fall through
        case Token::MINUSMINUS: {
            CREATE_NODE(result, next.fOffset, ASTNode::Kind::kPostfix, next);
            getNode(result).addChild(base);
            return result;
        }
        default: {
            this->error(next,  "expected expression suffix, but found '" + this->text(next) +
                                         "'\n");
            return ASTNode::ID::Invalid();
        }
    }
}

/* IDENTIFIER | intLiteral | floatLiteral | boolLiteral | NULL_LITERAL | '(' expression ')' */
ASTNode::ID Parser::term() {
    Token t = this->peek();
    switch (t.fKind) {
        case Token::IDENTIFIER: {
            StringFragment text;
            if (this->identifier(&text)) {
                RETURN_NODE(t.fOffset, ASTNode::Kind::kIdentifier, std::move(text));
            }
        }
        case Token::INT_LITERAL: {
            SKSL_INT i;
            if (this->intLiteral(&i)) {
                RETURN_NODE(t.fOffset, ASTNode::Kind::kInt, i);
            }
            break;
        }
        case Token::FLOAT_LITERAL: {
            SKSL_FLOAT f;
            if (this->floatLiteral(&f)) {
                RETURN_NODE(t.fOffset, ASTNode::Kind::kFloat, f);
            }
            break;
        }
        case Token::TRUE_LITERAL: // fall through
        case Token::FALSE_LITERAL: {
            bool b;
            if (this->boolLiteral(&b)) {
                RETURN_NODE(t.fOffset, ASTNode::Kind::kBool, b);
            }
            break;
        }
        case Token::NULL_LITERAL:
            this->nextToken();
            RETURN_NODE(t.fOffset, ASTNode::Kind::kNull);
        case Token::LPAREN: {
            this->nextToken();
            AutoDepth depth(this);
            if (!depth.checkValid()) {
                return ASTNode::ID::Invalid();
            }
            ASTNode::ID result = this->expression();
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
    return ASTNode::ID::Invalid();
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
