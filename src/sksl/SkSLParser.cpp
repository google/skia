/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "stdio.h"
#include "SkSLParser.h"
#include "SkSLToken.h"

#define register
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunneeded-internal-declaration"
#pragma clang diagnostic ignored "-Wnull-conversion"
#pragma clang diagnostic ignored "-Wsign-compare"
#endif
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#endif
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4018)
#endif
#include "lex.sksl.c"
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
#ifdef _MSC_VER
#pragma warning(pop)
#endif
#undef register

#include "ast/SkSLASTBinaryExpression.h"
#include "ast/SkSLASTBlock.h"
#include "ast/SkSLASTBoolLiteral.h"
#include "ast/SkSLASTBreakStatement.h"
#include "ast/SkSLASTCallSuffix.h"
#include "ast/SkSLASTContinueStatement.h"
#include "ast/SkSLASTDiscardStatement.h"
#include "ast/SkSLASTDoStatement.h"
#include "ast/SkSLASTExpression.h"
#include "ast/SkSLASTExpressionStatement.h"
#include "ast/SkSLASTExtension.h"
#include "ast/SkSLASTFieldSuffix.h"
#include "ast/SkSLASTFloatLiteral.h"
#include "ast/SkSLASTForStatement.h"
#include "ast/SkSLASTFunction.h"
#include "ast/SkSLASTIdentifier.h"
#include "ast/SkSLASTIfStatement.h"
#include "ast/SkSLASTIndexSuffix.h"
#include "ast/SkSLASTInterfaceBlock.h"
#include "ast/SkSLASTIntLiteral.h"
#include "ast/SkSLASTModifiersDeclaration.h"
#include "ast/SkSLASTParameter.h"
#include "ast/SkSLASTPrecision.h"
#include "ast/SkSLASTPrefixExpression.h"
#include "ast/SkSLASTReturnStatement.h"
#include "ast/SkSLASTStatement.h"
#include "ast/SkSLASTSuffixExpression.h"
#include "ast/SkSLASTTernaryExpression.h"
#include "ast/SkSLASTType.h"
#include "ast/SkSLASTVarDeclaration.h"
#include "ast/SkSLASTVarDeclarationStatement.h"
#include "ast/SkSLASTWhileStatement.h"
#include "ir/SkSLSymbolTable.h"
#include "ir/SkSLModifiers.h"
#include "ir/SkSLType.h"

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
            fParser->error(fParser->peek().fPosition, SkString("exceeded max parse depth"));
            return false;
        }
        return true;
    }

private:
    Parser* fParser;
};

Parser::Parser(SkString text, SymbolTable& types, ErrorReporter& errors)
: fPushback(Position(-1, -1), Token::INVALID_TOKEN, SkString())
, fTypes(types)
, fErrors(errors) {
    sksllex_init(&fScanner);
    fBuffer = sksl_scan_string(text.c_str(), fScanner);
    skslset_lineno(1, fScanner);

    if (false) {
        // avoid unused warning
        yyunput(0, nullptr, fScanner);
    }
}

Parser::~Parser() {
    sksl_delete_buffer(fBuffer, fScanner);
    sksllex_destroy(fScanner);
}

/* (precision | directive | declaration)* END_OF_FILE */
std::vector<sk_up<ASTDeclaration>> Parser::file() {
    std::vector<sk_up<ASTDeclaration>> result;
    for (;;) {
        switch (this->peek().fKind) {
            case Token::END_OF_FILE:
                return result;
            case Token::PRECISION: {
                sk_up<ASTDeclaration> precision = this->precision();
                if (precision) {
                    result.push_back(std::move(precision));
                }
                break;
            }
            case Token::DIRECTIVE: {
                sk_up<ASTDeclaration> decl = this->directive();
                if (decl) {
                    result.push_back(std::move(decl));
                }
                break;
            }
            default: {
                sk_up<ASTDeclaration> decl = this->declaration();
                if (!decl) {
                    continue;
                }
                result.push_back(std::move(decl));
            }
        }
    }
}

Token Parser::nextToken() {
    if (fPushback.fKind != Token::INVALID_TOKEN) {
        Token result = fPushback;
        fPushback.fKind = Token::INVALID_TOKEN;
        fPushback.fText = "";
        return result;
    }
    int token = sksllex(fScanner);
    SkString text;
    switch ((Token::Kind) token) {
        case Token::IDENTIFIER:    // fall through
        case Token::INT_LITERAL:   // fall through
        case Token::FLOAT_LITERAL: // fall through
        case Token::DIRECTIVE:
            text = SkString(skslget_text(fScanner));
            break;
        default:
            break;
    }
    return Token(Position(skslget_lineno(fScanner), -1), (Token::Kind) token, text);
}

void Parser::pushback(Token t) {
    ASSERT(fPushback.fKind == Token::INVALID_TOKEN);
    fPushback = t;
}

Token Parser::peek() {
    fPushback = this->nextToken();
    return fPushback;
}


bool Parser::expect(Token::Kind kind, const char* expected, Token* result) {
    return this->expect(kind, SkString(expected), result);
}

bool Parser::expect(Token::Kind kind, SkString expected, Token* result) {
    Token next = this->nextToken();
    if (next.fKind == kind) {
        if (result) {
            *result = next;
        }
        return true;
    } else {
        this->error(next.fPosition, "expected " + expected + ", but found '" + next.fText + "'");
        return false;
    }
}

void Parser::error(Position p, const char* msg) {
    this->error(p, SkString(msg));
}

void Parser::error(Position p, SkString msg) {
    fErrors.error(p, msg);
}

bool Parser::isType(SkString name) {
    return nullptr != fTypes[name];
}

/* PRECISION (LOWP | MEDIUMP | HIGHP) type SEMICOLON */
sk_up<ASTDeclaration> Parser::precision() {
    if (!this->expect(Token::PRECISION, "'precision'")) {
        return nullptr;
    }
    Modifiers::Flag result;
    Token p = this->nextToken();
    switch (p.fKind) {
        case Token::LOWP:
            result = Modifiers::kLowp_Flag;
            break;
        case Token::MEDIUMP:
            result = Modifiers::kMediump_Flag;
            break;
        case Token::HIGHP:
            result = Modifiers::kHighp_Flag;
            break;
        default:
            this->error(p.fPosition, "expected 'lowp', 'mediump', or 'highp', but found '" +
                                     p.fText + "'");
            return nullptr;
    }
    // FIXME handle the type
    if (!this->type()) {
        return nullptr;
    }
    this->expect(Token::SEMICOLON, "';'");
    return sk_up<ASTDeclaration>(new ASTPrecision(p.fPosition, result));
}

/* DIRECTIVE(#version) INT_LITERAL ("es" | "compatibility")? |
   DIRECTIVE(#extension) IDENTIFIER COLON IDENTIFIER */
sk_up<ASTDeclaration> Parser::directive() {
    Token start;
    if (!this->expect(Token::DIRECTIVE, "a directive", &start)) {
        return nullptr;
    }
    if (start.fText == "#version") {
        this->expect(Token::INT_LITERAL, "a version number");
        Token next = this->peek();
        if (next.fText == "es" || next.fText == "compatibility") {
            this->nextToken();
        }
        // version is ignored for now; it will eventually become an error when we stop pretending
        // to be GLSL
        return nullptr;
    } else if (start.fText == "#extension") {
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
        return sk_up<ASTDeclaration>(new ASTExtension(start.fPosition, std::move(name.fText)));
    } else {
        this->error(start.fPosition, "unsupported directive '" + start.fText + "'");
        return nullptr;
    }
}

/* modifiers (structVarDeclaration | type IDENTIFIER ((LPAREN parameter
   (COMMA parameter)* RPAREN (block | SEMICOLON)) | SEMICOLON) | interfaceBlock) */
sk_up<ASTDeclaration> Parser::declaration() {
    Modifiers modifiers = this->modifiers();
    Token lookahead = this->peek();
    if (lookahead.fKind == Token::IDENTIFIER && !this->isType(lookahead.fText)) {
        // we have an identifier that's not a type, could be the start of an interface block
        return this->interfaceBlock(modifiers);
    }
    if (lookahead.fKind == Token::STRUCT) {
        return this->structVarDeclaration(modifiers);
    }
    if (lookahead.fKind == Token::SEMICOLON) {
        this->nextToken();
        return sk_up<ASTDeclaration>(new ASTModifiersDeclaration(modifiers));
    }
    sk_up<ASTType> type(this->type());
    if (!type) {
        return nullptr;
    }
    if (type->fKind == ASTType::kStruct_Kind && peek().fKind == Token::SEMICOLON) {
        this->nextToken();
        return nullptr;
    }
    Token name;
    if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
        return nullptr;
    }
    if (!modifiers.fFlags && this->peek().fKind == Token::LPAREN) {
        this->nextToken();
        std::vector<sk_up<ASTParameter>> parameters;
        while (this->peek().fKind != Token::RPAREN) {
            if (parameters.size() > 0) {
                if (!this->expect(Token::COMMA, "','")) {
                    return nullptr;
                }
            }
            sk_up<ASTParameter> parameter = this->parameter();
            if (!parameter) {
                return nullptr;
            }
            parameters.push_back(std::move(parameter));
        }
        this->nextToken();
        sk_up<ASTBlock> body;
        if (this->peek().fKind == Token::SEMICOLON) {
            this->nextToken();
        } else {
            body = this->block();
            if (!body) {
                return nullptr;
            }
        }
        return sk_up<ASTDeclaration>(new ASTFunction(name.fPosition, std::move(type),
                                                     std::move(name.fText), std::move(parameters),
                                                     std::move(body)));
    } else {
        return this->varDeclarationEnd(modifiers, std::move(type), name.fText);
    }
}

/* modifiers type IDENTIFIER varDeclarationEnd */
sk_up<ASTVarDeclarations> Parser::varDeclarations() {
    Modifiers modifiers = this->modifiers();
    sk_up<ASTType> type(this->type());
    if (!type) {
        return nullptr;
    }
    Token name;
    if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
        return nullptr;
    }
    return this->varDeclarationEnd(modifiers, std::move(type), std::move(name.fText));
}

/* STRUCT IDENTIFIER LBRACE varDeclaration* RBRACE */
sk_up<ASTType> Parser::structDeclaration() {
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
        sk_up<ASTVarDeclarations> decl = this->varDeclarations();
        if (!decl) {
            return nullptr;
        }
        for (const auto& var : decl->fVars) {
            auto type = (const Type*) fTypes[decl->fType->fName];
            for (int i = (int) var.fSizes.size() - 1; i >= 0; i--) {
                if (!var.fSizes[i] || var.fSizes[i]->fKind != ASTExpression::kInt_Kind) {
                    this->error(decl->fPosition, "array size in struct field must be a constant");
                    return nullptr;
                }
                uint64_t columns = ((ASTIntLiteral&) *var.fSizes[i]).fValue;
                SkString name = type->name() + "[" + to_string(columns) + "]";
                type = new Type(name, Type::kArray_Kind, *type, (int) columns);
                fTypes.takeOwnership((Type*) type);
            }
            fields.push_back(Type::Field(decl->fModifiers, var.fName, type));
            if (var.fValue) {
                this->error(decl->fPosition, "initializers are not permitted on struct fields");
            }
        }
    }
    if (!this->expect(Token::RBRACE, "'}'")) {
        return nullptr;
    }
    fTypes.add(name.fText, sk_up<Type>(new Type(name.fPosition, name.fText, fields)));
    return sk_up<ASTType>(new ASTType(name.fPosition, name.fText, ASTType::kStruct_Kind));
}

/* structDeclaration ((IDENTIFIER varDeclarationEnd) | SEMICOLON) */
sk_up<ASTVarDeclarations> Parser::structVarDeclaration(Modifiers modifiers) {
    sk_up<ASTType> type = this->structDeclaration();
    if (!type) {
        return nullptr;
    }
    if (peek().fKind == Token::IDENTIFIER) {
        Token name = this->nextToken();
        sk_up<ASTVarDeclarations> result =
                this->varDeclarationEnd(modifiers, std::move(type), std::move(name.fText));
        if (result) {
            for (const auto& var : result->fVars) {
                if (var.fValue) {
                    this->error(var.fValue->fPosition,
                                "struct variables cannot be initialized");
                }
            }
        }
        return result;
    }
    this->expect(Token::SEMICOLON, "';'");
    return nullptr;
}

/* (LBRACKET expression? RBRACKET)* (EQ expression)? (COMMA IDENTIFER
   (LBRACKET expression? RBRACKET)* (EQ expression)?)* SEMICOLON */
sk_up<ASTVarDeclarations> Parser::varDeclarationEnd(
        Modifiers mods, sk_up<ASTType> type, SkString name) {
    std::vector<ASTVarDeclaration> vars;
    std::vector<sk_up<ASTExpression>> currentVarSizes;
    while (this->peek().fKind == Token::LBRACKET) {
        this->nextToken();
        if (this->peek().fKind == Token::RBRACKET) {
            this->nextToken();
            currentVarSizes.push_back(nullptr);
        } else {
            sk_up<ASTExpression> size(this->expression());
            if (!size) {
                return nullptr;
            }
            currentVarSizes.push_back(std::move(size));
            if (!this->expect(Token::RBRACKET, "']'")) {
                return nullptr;
            }
        }
    }
    sk_up<ASTExpression> value;
    if (this->peek().fKind == Token::EQ) {
        this->nextToken();
        value = this->expression();
        if (!value) {
            return nullptr;
        }
    }
    vars.emplace_back(std::move(name), std::move(currentVarSizes), std::move(value));
    while (this->peek().fKind == Token::COMMA) {
        this->nextToken();
        Token name;
        if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
            return nullptr;
        }
        currentVarSizes.clear();
        value.reset();
        while (this->peek().fKind == Token::LBRACKET) {
            this->nextToken();
            if (this->peek().fKind == Token::RBRACKET) {
                this->nextToken();
                currentVarSizes.push_back(nullptr);
            } else {
                sk_up<ASTExpression> size(this->expression());
                if (!size) {
                    return nullptr;
                }
                currentVarSizes.push_back(std::move(size));
                if (!this->expect(Token::RBRACKET, "']'")) {
                    return nullptr;
                }
            }
        }
        if (this->peek().fKind == Token::EQ) {
            this->nextToken();
            value = this->expression();
            if (!value) {
                return nullptr;
            }
        }
        vars.emplace_back(std::move(name.fText), std::move(currentVarSizes), std::move(value));
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return nullptr;
    }
    return sk_up<ASTVarDeclarations>(
            new ASTVarDeclarations(std::move(mods), std::move(type), std::move(vars)));
}

/* modifiers type IDENTIFIER (LBRACKET INT_LITERAL RBRACKET)? */
sk_up<ASTParameter> Parser::parameter() {
    Modifiers modifiers = this->modifiersWithDefaults(Modifiers::kIn_Flag);
    sk_up<ASTType> type = this->type();
    if (!type) {
        return nullptr;
    }
    Token name;
    if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
        return nullptr;
    }
    std::vector<int> sizes;
    while (this->peek().fKind == Token::LBRACKET) {
        this->nextToken();
        Token sizeToken;
        if (!this->expect(Token::INT_LITERAL, "a positive integer", &sizeToken)) {
            return nullptr;
        }
        sizes.push_back(SkSL::stoi(sizeToken.fText));
        if (!this->expect(Token::RBRACKET, "']'")) {
            return nullptr;
        }
    }
    return sk_up<ASTParameter>(new ASTParameter(name.fPosition, modifiers, std::move(type),
                                                name.fText, std::move(sizes)));
}

/** (EQ INT_LITERAL)? */
int Parser::layoutInt() {
    if (!this->expect(Token::EQ, "'='")) {
        return -1;
    }
    Token resultToken;
    if (this->expect(Token::INT_LITERAL, "a non-negative integer", &resultToken)) {
        return SkSL::stoi(resultToken.fText);
    }
    return -1;
}

/* LAYOUT LPAREN IDENTIFIER (EQ INT_LITERAL)? (COMMA IDENTIFIER (EQ INT_LITERAL)?)* RPAREN */
Layout Parser::layout() {
    int location = -1;
    int offset = -1;
    int binding = -1;
    int index = -1;
    int set = -1;
    int builtin = -1;
    int inputAttachmentIndex = -1;
    bool originUpperLeft = false;
    bool overrideCoverage = false;
    bool blendSupportAllEquations = false;
    Layout::Format format = Layout::Format::kUnspecified;
    bool pushConstant = false;
    if (this->peek().fKind == Token::LAYOUT) {
        this->nextToken();
        if (!this->expect(Token::LPAREN, "'('")) {
            return Layout(location, offset, binding, index, set, builtin, inputAttachmentIndex,
                          originUpperLeft, overrideCoverage, blendSupportAllEquations, format,
                          pushConstant);
        }
        for (;;) {
            Token t = this->nextToken();
            if (t.fText == "location") {
                location = this->layoutInt();
            } else if (t.fText == "offset") {
                offset = this->layoutInt();
            } else if (t.fText == "binding") {
                binding = this->layoutInt();
            } else if (t.fText == "index") {
                index = this->layoutInt();
            } else if (t.fText == "set") {
                set = this->layoutInt();
            } else if (t.fText == "builtin") {
                builtin = this->layoutInt();
            } else if (t.fText == "input_attachment_index") {
                 inputAttachmentIndex = this->layoutInt();
            } else if (t.fText == "origin_upper_left") {
                originUpperLeft = true;
            } else if (t.fText == "override_coverage") {
                overrideCoverage = true;
            } else if (t.fText == "blend_support_all_equations") {
                blendSupportAllEquations = true;
            } else if (Layout::ReadFormat(t.fText, &format)) {
               // AST::ReadFormat stored the result in 'format'.
            } else if (t.fText == "push_constant") {
                pushConstant = true;
            } else {
                this->error(t.fPosition, ("'" + t.fText +
                                          "' is not a valid layout qualifier").c_str());
            }
            if (this->peek().fKind == Token::RPAREN) {
                this->nextToken();
                break;
            }
            if (!this->expect(Token::COMMA, "','")) {
                break;
            }
        }
    }
    return Layout(location, offset, binding, index, set, builtin, inputAttachmentIndex,
                  originUpperLeft, overrideCoverage, blendSupportAllEquations, format,
                  pushConstant);
}

/* layout? (UNIFORM | CONST | IN | OUT | INOUT | LOWP | MEDIUMP | HIGHP | FLAT | NOPERSPECTIVE |
            READONLY | WRITEONLY | COHERENT | VOLATILE | RESTRICT)* */
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
            case Token::LOWP:
                this->nextToken();
                flags |= Modifiers::kLowp_Flag;
                break;
            case Token::MEDIUMP:
                this->nextToken();
                flags |= Modifiers::kMediump_Flag;
                break;
            case Token::HIGHP:
                this->nextToken();
                flags |= Modifiers::kHighp_Flag;
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
sk_up<ASTStatement> Parser::statement() {
    Token start = this->peek();
    switch (start.fKind) {
        case Token::IF:
            return this->ifStatement();
        case Token::FOR:
            return this->forStatement();
        case Token::DO:
            return this->doStatement();
        case Token::WHILE:
            return this->whileStatement();
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
            return sk_up<ASTStatement>(
                    new ASTBlock(start.fPosition, std::vector<sk_up<ASTStatement>>()));
        case Token::CONST:   // fall through
        case Token::HIGHP:   // fall through
        case Token::MEDIUMP: // fall through
        case Token::LOWP: {
            auto decl = this->varDeclarations();
            if (!decl) {
                return nullptr;
            }
            return sk_up<ASTStatement>(new ASTVarDeclarationStatement(std::move(decl)));
        }
        case Token::IDENTIFIER:
            if (this->isType(start.fText)) {
                auto decl = this->varDeclarations();
                if (!decl) {
                    return nullptr;
                }
                return sk_up<ASTStatement>(new ASTVarDeclarationStatement(std::move(decl)));
            }
            // fall through
        default:
            return this->expressionStatement();
    }
}

/* IDENTIFIER(type) */
sk_up<ASTType> Parser::type() {
    Token type;
    if (!this->expect(Token::IDENTIFIER, "a type", &type)) {
        return nullptr;
    }
    if (!this->isType(type.fText)) {
        this->error(type.fPosition, ("no type named '" + type.fText + "'").c_str());
        return nullptr;
    }
    return sk_up<ASTType>(
            new ASTType(type.fPosition, std::move(type.fText), ASTType::kIdentifier_Kind));
}

/* IDENTIFIER LBRACE varDeclaration* RBRACE */
sk_up<ASTDeclaration> Parser::interfaceBlock(Modifiers mods) {
    Token name;
    if (!this->expect(Token::IDENTIFIER, "an identifier", &name)) {
        return nullptr;
    }
    if (peek().fKind != Token::LBRACE) {
        // we only get into interfaceBlock if we found a top-level identifier which was not a type.
        // 99% of the time, the user was not actually intending to create an interface block, so
        // it's better to report it as an unknown type
        this->error(name.fPosition, "no type named '" + name.fText + "'");
        return nullptr;
    }
    this->nextToken();
    std::vector<sk_up<ASTVarDeclarations>> decls;
    while (this->peek().fKind != Token::RBRACE) {
        sk_up<ASTVarDeclarations> decl = this->varDeclarations();
        if (!decl) {
            return nullptr;
        }
        decls.push_back(std::move(decl));
    }
    this->nextToken();
    SkString valueName;
    if (this->peek().fKind == Token::IDENTIFIER) {
        valueName = this->nextToken().fText;
    }
    this->expect(Token::SEMICOLON, "';'");
    return sk_up<ASTDeclaration>(new ASTInterfaceBlock(name.fPosition, mods, name.fText,
                                                       std::move(valueName), std::move(decls)));
}

/* IF LPAREN expression RPAREN statement (ELSE statement)? */
sk_up<ASTIfStatement> Parser::ifStatement() {
    Token start;
    if (!this->expect(Token::IF, "'if'", &start)) {
        return nullptr;
    }
    if (!this->expect(Token::LPAREN, "'('")) {
        return nullptr;
    }
    sk_up<ASTExpression> test(this->expression());
    if (!test) {
        return nullptr;
    }
    if (!this->expect(Token::RPAREN, "')'")) {
        return nullptr;
    }
    sk_up<ASTStatement> ifTrue(this->statement());
    if (!ifTrue) {
        return nullptr;
    }
    sk_up<ASTStatement> ifFalse;
    if (this->peek().fKind == Token::ELSE) {
        this->nextToken();
        ifFalse = this->statement();
        if (!ifFalse) {
            return nullptr;
        }
    }
    return sk_up<ASTIfStatement>(new ASTIfStatement(start.fPosition, std::move(test),
                                                    std::move(ifTrue), std::move(ifFalse)));
}

/* DO statement WHILE LPAREN expression RPAREN SEMICOLON */
sk_up<ASTDoStatement> Parser::doStatement() {
    Token start;
    if (!this->expect(Token::DO, "'do'", &start)) {
        return nullptr;
    }
    sk_up<ASTStatement> statement(this->statement());
    if (!statement) {
        return nullptr;
    }
    if (!this->expect(Token::WHILE, "'while'")) {
        return nullptr;
    }
    if (!this->expect(Token::LPAREN, "'('")) {
        return nullptr;
    }
    sk_up<ASTExpression> test(this->expression());
    if (!test) {
        return nullptr;
    }
    if (!this->expect(Token::RPAREN, "')'")) {
        return nullptr;
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return nullptr;
    }
    return sk_up<ASTDoStatement>(
            new ASTDoStatement(start.fPosition, std::move(statement), std::move(test)));
}

/* WHILE LPAREN expression RPAREN STATEMENT */
sk_up<ASTWhileStatement> Parser::whileStatement() {
    Token start;
    if (!this->expect(Token::WHILE, "'while'", &start)) {
        return nullptr;
    }
    if (!this->expect(Token::LPAREN, "'('")) {
        return nullptr;
    }
    sk_up<ASTExpression> test(this->expression());
    if (!test) {
        return nullptr;
    }
    if (!this->expect(Token::RPAREN, "')'")) {
        return nullptr;
    }
    sk_up<ASTStatement> statement(this->statement());
    if (!statement) {
        return nullptr;
    }
    return sk_up<ASTWhileStatement>(
            new ASTWhileStatement(start.fPosition, std::move(test), std::move(statement)));
}

/* FOR LPAREN (declaration | expression)? SEMICOLON expression? SEMICOLON expression? RPAREN
   STATEMENT */
sk_up<ASTForStatement> Parser::forStatement() {
    Token start;
    if (!this->expect(Token::FOR, "'for'", &start)) {
        return nullptr;
    }
    if (!this->expect(Token::LPAREN, "'('")) {
        return nullptr;
    }
    sk_up<ASTStatement> initializer;
    Token nextToken = this->peek();
    switch (nextToken.fKind) {
        case Token::SEMICOLON:
            this->nextToken();
            break;
        case Token::CONST: {
            sk_up<ASTVarDeclarations> vd = this->varDeclarations();
            if (!vd) {
                return nullptr;
            }
            initializer = sk_up<ASTStatement>(new ASTVarDeclarationStatement(std::move(vd)));
            break;
        }
        case Token::IDENTIFIER: {
            if (this->isType(nextToken.fText)) {
                sk_up<ASTVarDeclarations> vd = this->varDeclarations();
                if (!vd) {
                    return nullptr;
                }
                initializer = sk_up<ASTStatement>(new ASTVarDeclarationStatement(std::move(vd)));
                break;
            }
        } // fall through
        default:
            initializer = this->expressionStatement();
    }
    sk_up<ASTExpression> test;
    if (this->peek().fKind != Token::SEMICOLON) {
        test = this->expression();
        if (!test) {
            return nullptr;
        }
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return nullptr;
    }
    sk_up<ASTExpression> next;
    if (this->peek().fKind != Token::RPAREN) {
        next = this->expression();
        if (!next) {
            return nullptr;
        }
    }
    if (!this->expect(Token::RPAREN, "')'")) {
        return nullptr;
    }
    sk_up<ASTStatement> statement(this->statement());
    if (!statement) {
        return nullptr;
    }
    return sk_up<ASTForStatement>(new ASTForStatement(start.fPosition, std::move(initializer),
                                                      std::move(test), std::move(next),
                                                      std::move(statement)));
}

/* RETURN expression? SEMICOLON */
sk_up<ASTReturnStatement> Parser::returnStatement() {
    Token start;
    if (!this->expect(Token::RETURN, "'return'", &start)) {
        return nullptr;
    }
    sk_up<ASTExpression> expression;
    if (this->peek().fKind != Token::SEMICOLON) {
        expression = this->expression();
        if (!expression) {
            return nullptr;
        }
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return nullptr;
    }
    return sk_up<ASTReturnStatement>(
            new ASTReturnStatement(start.fPosition, std::move(expression)));
}

/* BREAK SEMICOLON */
sk_up<ASTBreakStatement> Parser::breakStatement() {
    Token start;
    if (!this->expect(Token::BREAK, "'break'", &start)) {
        return nullptr;
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return nullptr;
    }
    return sk_up<ASTBreakStatement>(new ASTBreakStatement(start.fPosition));
}

/* CONTINUE SEMICOLON */
sk_up<ASTContinueStatement> Parser::continueStatement() {
    Token start;
    if (!this->expect(Token::CONTINUE, "'continue'", &start)) {
        return nullptr;
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return nullptr;
    }
    return sk_up<ASTContinueStatement>(new ASTContinueStatement(start.fPosition));
}

/* DISCARD SEMICOLON */
sk_up<ASTDiscardStatement> Parser::discardStatement() {
    Token start;
    if (!this->expect(Token::DISCARD, "'continue'", &start)) {
        return nullptr;
    }
    if (!this->expect(Token::SEMICOLON, "';'")) {
        return nullptr;
    }
    return sk_up<ASTDiscardStatement>(new ASTDiscardStatement(start.fPosition));
}

/* LBRACE statement* RBRACE */
sk_up<ASTBlock> Parser::block() {
    AutoDepth depth(this);
    if (!depth.checkValid()) {
        return nullptr;
    }
    Token start;
    if (!this->expect(Token::LBRACE, "'{'", &start)) {
        return nullptr;
    }
    std::vector<sk_up<ASTStatement>> statements;
    for (;;) {
        switch (this->peek().fKind) {
            case Token::RBRACE:
                this->nextToken();
                return sk_up<ASTBlock>(new ASTBlock(start.fPosition, std::move(statements)));
            case Token::END_OF_FILE:
                this->error(this->peek().fPosition, "expected '}', but found end of file");
                return nullptr;
            default: {
                sk_up<ASTStatement> statement = this->statement();
                if (!statement) {
                    return nullptr;
                }
                statements.push_back(std::move(statement));
            }
        }
    }
}

/* expression SEMICOLON */
sk_up<ASTExpressionStatement> Parser::expressionStatement() {
    sk_up<ASTExpression> expr = this->expression();
    if (expr) {
        if (this->expect(Token::SEMICOLON, "';'")) {
            ASTExpressionStatement* result = new ASTExpressionStatement(std::move(expr));
            return sk_up<ASTExpressionStatement>(result);
        }
    }
    return nullptr;
}

/* assignmentExpression */
sk_up<ASTExpression> Parser::expression() {
    AutoDepth depth(this);
    if (!depth.checkValid()) {
        return nullptr;
    }
    return this->assignmentExpression();
}

/* ternaryExpression ((EQEQ | STAREQ | SLASHEQ | PERCENTEQ | PLUSEQ | MINUSEQ | SHLEQ | SHREQ |
   BITWISEANDEQ | BITWISEXOREQ | BITWISEOREQ | LOGICALANDEQ | LOGICALXOREQ | LOGICALOREQ)
   assignmentExpression)*
 */
sk_up<ASTExpression> Parser::assignmentExpression() {
    sk_up<ASTExpression> result = this->ternaryExpression();
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
                sk_up<ASTExpression> right = this->assignmentExpression();
                if (!right) {
                    return nullptr;
                }
                result = sk_up<ASTExpression>(
                        new ASTBinaryExpression(std::move(result), t, std::move(right)));
            }
            default:
                return result;
        }
    }
}

/* logicalOrExpression ('?' expression ':' assignmentExpression)? */
sk_up<ASTExpression> Parser::ternaryExpression() {
    sk_up<ASTExpression> result = this->logicalOrExpression();
    if (!result) {
        return nullptr;
    }
    if (this->peek().fKind == Token::QUESTION) {
        Token question = this->nextToken();
        sk_up<ASTExpression> trueExpr = this->expression();
        if (!trueExpr) {
            return nullptr;
        }
        if (this->expect(Token::COLON, "':'")) {
            sk_up<ASTExpression> falseExpr = this->assignmentExpression();
            return sk_up<ASTExpression>(new ASTTernaryExpression(
                    std::move(result), std::move(trueExpr), std::move(falseExpr)));
        }
        return nullptr;
    }
    return result;
}

/* logicalXorExpression (LOGICALOR logicalXorExpression)* */
sk_up<ASTExpression> Parser::logicalOrExpression() {
    sk_up<ASTExpression> result = this->logicalXorExpression();
    if (!result) {
        return nullptr;
    }
    while (this->peek().fKind == Token::LOGICALOR) {
        Token t = this->nextToken();
        sk_up<ASTExpression> right = this->logicalXorExpression();
        if (!right) {
            return nullptr;
        }
        result.reset(new ASTBinaryExpression(std::move(result), t, std::move(right)));
    }
    return result;
}

/* logicalAndExpression (LOGICALXOR logicalAndExpression)* */
sk_up<ASTExpression> Parser::logicalXorExpression() {
    sk_up<ASTExpression> result = this->logicalAndExpression();
    if (!result) {
        return nullptr;
    }
    while (this->peek().fKind == Token::LOGICALXOR) {
        Token t = this->nextToken();
        sk_up<ASTExpression> right = this->logicalAndExpression();
        if (!right) {
            return nullptr;
        }
        result.reset(new ASTBinaryExpression(std::move(result), t, std::move(right)));
    }
    return result;
}

/* bitwiseOrExpression (LOGICALAND bitwiseOrExpression)* */
sk_up<ASTExpression> Parser::logicalAndExpression() {
    sk_up<ASTExpression> result = this->bitwiseOrExpression();
    if (!result) {
        return nullptr;
    }
    while (this->peek().fKind == Token::LOGICALAND) {
        Token t = this->nextToken();
        sk_up<ASTExpression> right = this->bitwiseOrExpression();
        if (!right) {
            return nullptr;
        }
        result.reset(new ASTBinaryExpression(std::move(result), t, std::move(right)));
    }
    return result;
}

/* bitwiseXorExpression (BITWISEOR bitwiseXorExpression)* */
sk_up<ASTExpression> Parser::bitwiseOrExpression() {
    sk_up<ASTExpression> result = this->bitwiseXorExpression();
    if (!result) {
        return nullptr;
    }
    while (this->peek().fKind == Token::BITWISEOR) {
        Token t = this->nextToken();
        sk_up<ASTExpression> right = this->bitwiseXorExpression();
        if (!right) {
            return nullptr;
        }
        result.reset(new ASTBinaryExpression(std::move(result), t, std::move(right)));
    }
    return result;
}

/* bitwiseAndExpression (BITWISEXOR bitwiseAndExpression)* */
sk_up<ASTExpression> Parser::bitwiseXorExpression() {
    sk_up<ASTExpression> result = this->bitwiseAndExpression();
    if (!result) {
        return nullptr;
    }
    while (this->peek().fKind == Token::BITWISEXOR) {
        Token t = this->nextToken();
        sk_up<ASTExpression> right = this->bitwiseAndExpression();
        if (!right) {
            return nullptr;
        }
        result.reset(new ASTBinaryExpression(std::move(result), t, std::move(right)));
    }
    return result;
}

/* equalityExpression (BITWISEAND equalityExpression)* */
sk_up<ASTExpression> Parser::bitwiseAndExpression() {
    sk_up<ASTExpression> result = this->equalityExpression();
    if (!result) {
        return nullptr;
    }
    while (this->peek().fKind == Token::BITWISEAND) {
        Token t = this->nextToken();
        sk_up<ASTExpression> right = this->equalityExpression();
        if (!right) {
            return nullptr;
        }
        result.reset(new ASTBinaryExpression(std::move(result), t, std::move(right)));
    }
    return result;
}

/* relationalExpression ((EQEQ | NEQ) relationalExpression)* */
sk_up<ASTExpression> Parser::equalityExpression() {
    sk_up<ASTExpression> result = this->relationalExpression();
    if (!result) {
        return nullptr;
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::EQEQ:   // fall through
            case Token::NEQ: {
                Token t = this->nextToken();
                sk_up<ASTExpression> right = this->relationalExpression();
                if (!right) {
                    return nullptr;
                }
                result.reset(new ASTBinaryExpression(std::move(result), t, std::move(right)));
                break;
            }
            default:
                return result;
        }
    }
}

/* shiftExpression ((LT | GT | LTEQ | GTEQ) shiftExpression)* */
sk_up<ASTExpression> Parser::relationalExpression() {
    sk_up<ASTExpression> result = this->shiftExpression();
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
                sk_up<ASTExpression> right = this->shiftExpression();
                if (!right) {
                    return nullptr;
                }
                result.reset(new ASTBinaryExpression(std::move(result), t, std::move(right)));
                break;
            }
            default:
                return result;
        }
    }
}

/* additiveExpression ((SHL | SHR) additiveExpression)* */
sk_up<ASTExpression> Parser::shiftExpression() {
    sk_up<ASTExpression> result = this->additiveExpression();
    if (!result) {
        return nullptr;
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::SHL: // fall through
            case Token::SHR: {
                Token t = this->nextToken();
                sk_up<ASTExpression> right = this->additiveExpression();
                if (!right) {
                    return nullptr;
                }
                result.reset(new ASTBinaryExpression(std::move(result), t, std::move(right)));
                break;
            }
            default:
                return result;
        }
    }
}

/* multiplicativeExpression ((PLUS | MINUS) multiplicativeExpression)* */
sk_up<ASTExpression> Parser::additiveExpression() {
    sk_up<ASTExpression> result = this->multiplicativeExpression();
    if (!result) {
        return nullptr;
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::PLUS: // fall through
            case Token::MINUS: {
                Token t = this->nextToken();
                sk_up<ASTExpression> right = this->multiplicativeExpression();
                if (!right) {
                    return nullptr;
                }
                result.reset(new ASTBinaryExpression(std::move(result), t, std::move(right)));
                break;
            }
            default:
                return result;
        }
    }
}

/* unaryExpression ((STAR | SLASH | PERCENT) unaryExpression)* */
sk_up<ASTExpression> Parser::multiplicativeExpression() {
    sk_up<ASTExpression> result = this->unaryExpression();
    if (!result) {
        return nullptr;
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::STAR: // fall through
            case Token::SLASH: // fall through
            case Token::PERCENT: {
                Token t = this->nextToken();
                sk_up<ASTExpression> right = this->unaryExpression();
                if (!right) {
                    return nullptr;
                }
                result.reset(new ASTBinaryExpression(std::move(result), t, std::move(right)));
                break;
            }
            default:
                return result;
        }
    }
}

/* postfixExpression | (PLUS | MINUS | NOT | PLUSPLUS | MINUSMINUS) unaryExpression */
sk_up<ASTExpression> Parser::unaryExpression() {
    switch (this->peek().fKind) {
        case Token::PLUS:       // fall through
        case Token::MINUS:      // fall through
        case Token::LOGICALNOT: // fall through
        case Token::BITWISENOT: // fall through
        case Token::PLUSPLUS:   // fall through
        case Token::MINUSMINUS: {
            Token t = this->nextToken();
            sk_up<ASTExpression> expr = this->unaryExpression();
            if (!expr) {
                return nullptr;
            }
            return sk_up<ASTExpression>(new ASTPrefixExpression(t, std::move(expr)));
        }
        default:
            return this->postfixExpression();
    }
}

/* term suffix* */
sk_up<ASTExpression> Parser::postfixExpression() {
    sk_up<ASTExpression> result = this->term();
    if (!result) {
        return nullptr;
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::LBRACKET: // fall through
            case Token::DOT:      // fall through
            case Token::LPAREN:   // fall through
            case Token::PLUSPLUS: // fall through
            case Token::MINUSMINUS: {
                sk_up<ASTSuffix> s = this->suffix();
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
   PLUSPLUS | MINUSMINUS */
sk_up<ASTSuffix> Parser::suffix() {
    Token next = this->nextToken();
    switch (next.fKind) {
        case Token::LBRACKET: {
            if (this->peek().fKind == Token::RBRACKET) {
                this->nextToken();
                return sk_up<ASTSuffix>(new ASTIndexSuffix(next.fPosition));
            }
            sk_up<ASTExpression> e = this->expression();
            if (!e) {
                return nullptr;
            }
            this->expect(Token::RBRACKET, "']' to complete array access expression");
            return sk_up<ASTSuffix>(new ASTIndexSuffix(std::move(e)));
        }
        case Token::DOT: {
            Position pos = this->peek().fPosition;
            SkString text;
            if (this->identifier(&text)) {
                return sk_up<ASTSuffix>(new ASTFieldSuffix(pos, std::move(text)));
            }
            return nullptr;
        }
        case Token::LPAREN: {
            std::vector<sk_up<ASTExpression>> parameters;
            if (this->peek().fKind != Token::RPAREN) {
                for (;;) {
                    sk_up<ASTExpression> expr = this->expression();
                    if (!expr) {
                        return nullptr;
                    }
                    parameters.push_back(std::move(expr));
                    if (this->peek().fKind != Token::COMMA) {
                        break;
                    }
                    this->nextToken();
                }
            }
            this->expect(Token::RPAREN, "')' to complete function parameters");
            return sk_up<ASTSuffix>(new ASTCallSuffix(next.fPosition, std::move(parameters)));
        }
        case Token::PLUSPLUS:
            return sk_up<ASTSuffix>(new ASTSuffix(next.fPosition, ASTSuffix::kPostIncrement_Kind));
        case Token::MINUSMINUS:
            return sk_up<ASTSuffix>(new ASTSuffix(next.fPosition, ASTSuffix::kPostDecrement_Kind));
        default: {
            this->error(next.fPosition,  "expected expression suffix, but found '" + next.fText +
                                         "'\n");
            return nullptr;
        }
    }
}

/* IDENTIFIER | intLiteral | floatLiteral | boolLiteral | '(' expression ')' */
sk_up<ASTExpression> Parser::term() {
    sk_up<ASTExpression> result;
    Token t = this->peek();
    switch (t.fKind) {
        case Token::IDENTIFIER: {
            SkString text;
            if (this->identifier(&text)) {
                result.reset(new ASTIdentifier(t.fPosition, std::move(text)));
            }
            break;
        }
        case Token::INT_LITERAL: {
            int64_t i;
            if (this->intLiteral(&i)) {
                result.reset(new ASTIntLiteral(t.fPosition, i));
            }
            break;
        }
        case Token::FLOAT_LITERAL: {
            double f;
            if (this->floatLiteral(&f)) {
                result.reset(new ASTFloatLiteral(t.fPosition, f));
            }
            break;
        }
        case Token::TRUE_LITERAL: // fall through
        case Token::FALSE_LITERAL: {
            bool b;
            if (this->boolLiteral(&b)) {
                result.reset(new ASTBoolLiteral(t.fPosition, b));
            }
            break;
        }
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
            this->error(t.fPosition,  "expected expression, but found '" + t.fText + "'\n");
            result = nullptr;
    }
    return result;
}

/* INT_LITERAL */
bool Parser::intLiteral(int64_t* dest) {
    Token t;
    if (this->expect(Token::INT_LITERAL, "integer literal", &t)) {
        *dest = SkSL::stol(t.fText);
        return true;
    }
    return false;
}

/* FLOAT_LITERAL */
bool Parser::floatLiteral(double* dest) {
    Token t;
    if (this->expect(Token::FLOAT_LITERAL, "float literal", &t)) {
        *dest = SkSL::stod(t.fText);
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
            this->error(t.fPosition, "expected 'true' or 'false', but found '" + t.fText + "'\n");
            return false;
    }
}

/* IDENTIFIER */
bool Parser::identifier(SkString* dest) {
    Token t;
    if (this->expect(Token::IDENTIFIER, "identifier", &t)) {
        *dest = t.fText;
        return true;
    }
    return false;
}

} // namespace
