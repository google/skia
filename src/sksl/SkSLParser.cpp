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
#include "disable_flex_warnings.h"
#include "lex.sksl.c"
static_assert(YY_FLEX_MAJOR_VERSION * 100 + YY_FLEX_MINOR_VERSION * 10 +
              YY_FLEX_SUBMINOR_VERSION >= 261,
              "we require Flex 2.6.1 or better for security reasons");
#undef register
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "lex.layout.h"
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
#include "ast/SkSLASTSwitchCase.h"
#include "ast/SkSLASTSwitchStatement.h"
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
            fParser->error(fParser->peek().fPosition, String("exceeded max parse depth"));
            return false;
        }
        return true;
    }

private:
    Parser* fParser;
};

Parser::Parser(String text, SymbolTable& types, ErrorReporter& errors)
: fPushback(Position(-1, -1), Token::INVALID_TOKEN, String())
, fTypes(types)
, fErrors(errors) {
    sksllex_init(&fScanner);
    layoutlex_init(&fLayoutScanner);
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
    layoutlex_destroy(fLayoutScanner);
}

/* (precision | directive | declaration)* END_OF_FILE */
std::vector<std::unique_ptr<ASTDeclaration>> Parser::file() {
    std::vector<std::unique_ptr<ASTDeclaration>> result;
    for (;;) {
        switch (this->peek().fKind) {
            case Token::END_OF_FILE:
                return result;
            case Token::PRECISION: {
                std::unique_ptr<ASTDeclaration> precision = this->precision();
                if (precision) {
                    result.push_back(std::move(precision));
                }
                break;
            }
            case Token::DIRECTIVE: {
                std::unique_ptr<ASTDeclaration> decl = this->directive();
                if (decl) {
                    result.push_back(std::move(decl));
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

Token Parser::nextToken() {
    if (fPushback.fKind != Token::INVALID_TOKEN) {
        Token result = fPushback;
        fPushback.fKind = Token::INVALID_TOKEN;
        fPushback.fText = "";
        return result;
    }
    int token = sksllex(fScanner);
    String text;
    switch ((Token::Kind) token) {
        case Token::IDENTIFIER:    // fall through
        case Token::INT_LITERAL:   // fall through
        case Token::FLOAT_LITERAL: // fall through
        case Token::DIRECTIVE:
            text = String(skslget_text(fScanner));
            break;
        default:
#ifdef SK_DEBUG
            text = String(skslget_text(fScanner));
#endif
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
    return this->expect(kind, String(expected), result);
}

bool Parser::expect(Token::Kind kind, String expected, Token* result) {
    Token next = this->nextToken();
    if (next.fKind == kind) {
        if (result) {
            *result = next;
        }
        return true;
    } else {
        if (next.fText.size()) {
            this->error(next.fPosition, "expected " + expected + ", but found '" + next.fText +
                                        "'");
        } else {
            this->error(next.fPosition, "parse error, recompile in debug mode for details");
        }
        return false;
    }
}

void Parser::error(Position p, const char* msg) {
    this->error(p, String(msg));
}

void Parser::error(Position p, String msg) {
    fErrors.error(p, msg);
}

bool Parser::isType(String name) {
    return nullptr != fTypes[name];
}

/* PRECISION (LOWP | MEDIUMP | HIGHP) type SEMICOLON */
std::unique_ptr<ASTDeclaration> Parser::precision() {
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
    return std::unique_ptr<ASTDeclaration>(new ASTPrecision(p.fPosition, result));
}

/* DIRECTIVE(#version) INT_LITERAL ("es" | "compatibility")? |
   DIRECTIVE(#extension) IDENTIFIER COLON IDENTIFIER */
std::unique_ptr<ASTDeclaration> Parser::directive() {
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
        return std::unique_ptr<ASTDeclaration>(new ASTExtension(start.fPosition,
                                                                std::move(name.fText)));
    } else {
        this->error(start.fPosition, "unsupported directive '" + start.fText + "'");
        return nullptr;
    }
}

/* modifiers (structVarDeclaration | type IDENTIFIER ((LPAREN parameter
   (COMMA parameter)* RPAREN (block | SEMICOLON)) | SEMICOLON) | interfaceBlock) */
std::unique_ptr<ASTDeclaration> Parser::declaration() {
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
        return std::unique_ptr<ASTDeclaration>(new ASTModifiersDeclaration(modifiers));
    }
    std::unique_ptr<ASTType> type(this->type());
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
        if (this->peek().fKind == Token::SEMICOLON) {
            this->nextToken();
        } else {
            body = this->block();
            if (!body) {
                return nullptr;
            }
        }
        return std::unique_ptr<ASTDeclaration>(new ASTFunction(name.fPosition, std::move(type),
                                                               std::move(name.fText),
                                                               std::move(parameters),
                                                               std::move(body)));
    } else {
        return this->varDeclarationEnd(modifiers, std::move(type), name.fText);
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
    return this->varDeclarationEnd(modifiers, std::move(type), std::move(name.fText));
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
                    this->error(decl->fPosition, "array size in struct field must be a constant");
                    return nullptr;
                }
                uint64_t columns = ((ASTIntLiteral&) *var.fSizes[i]).fValue;
                String name = type->name() + "[" + to_string(columns) + "]";
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
    fTypes.add(name.fText, std::unique_ptr<Type>(new Type(name.fPosition, name.fText, fields)));
    return std::unique_ptr<ASTType>(new ASTType(name.fPosition, name.fText,
                                                ASTType::kStruct_Kind, std::vector<int>()));
}

/* structDeclaration ((IDENTIFIER varDeclarationEnd) | SEMICOLON) */
std::unique_ptr<ASTVarDeclarations> Parser::structVarDeclaration(Modifiers modifiers) {
    std::unique_ptr<ASTType> type = this->structDeclaration();
    if (!type) {
        return nullptr;
    }
    if (peek().fKind == Token::IDENTIFIER) {
        Token name = this->nextToken();
        std::unique_ptr<ASTVarDeclarations> result = this->varDeclarationEnd(modifiers,
                                                                             std::move(type),
                                                                             std::move(name.fText));
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
std::unique_ptr<ASTVarDeclarations> Parser::varDeclarationEnd(Modifiers mods,
                                                              std::unique_ptr<ASTType> type,
                                                              String name) {
    std::vector<ASTVarDeclaration> vars;
    std::vector<std::unique_ptr<ASTExpression>> currentVarSizes;
    while (this->peek().fKind == Token::LBRACKET) {
        this->nextToken();
        if (this->peek().fKind == Token::RBRACKET) {
            this->nextToken();
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
    return std::unique_ptr<ASTParameter>(new ASTParameter(name.fPosition, modifiers,
                                                          std::move(type), name.fText,
                                                          std::move(sizes)));
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
    Layout::Primitive primitive = Layout::kUnspecified_Primitive;
    int maxVertices = -1;
    int invocations = -1;
    if (this->peek().fKind == Token::LAYOUT) {
        this->nextToken();
        if (!this->expect(Token::LPAREN, "'('")) {
            return Layout(location, offset, binding, index, set, builtin, inputAttachmentIndex,
                          originUpperLeft, overrideCoverage, blendSupportAllEquations, format,
                          pushConstant, primitive, maxVertices, invocations);
        }
        for (;;) {
            Token t = this->nextToken();
            YY_BUFFER_STATE buffer;
            buffer = layout_scan_string(t.fText.c_str(), fLayoutScanner);
            int token = layoutlex(fLayoutScanner);
            layout_delete_buffer(buffer, fLayoutScanner);
            if (token != Token::INVALID_TOKEN) {
                switch (token) {
                    case Token::LOCATION:
                        location = this->layoutInt();
                        break;
                    case Token::OFFSET:
                        offset = this->layoutInt();
                        break;
                    case Token::BINDING:
                        binding = this->layoutInt();
                        break;
                    case Token::INDEX:
                        index = this->layoutInt();
                        break;
                    case Token::SET:
                        set = this->layoutInt();
                        break;
                    case Token::BUILTIN:
                        builtin = this->layoutInt();
                        break;
                    case Token::INPUT_ATTACHMENT_INDEX:
                        inputAttachmentIndex = this->layoutInt();
                        break;
                    case Token::ORIGIN_UPPER_LEFT:
                        originUpperLeft = true;
                        break;
                    case Token::OVERRIDE_COVERAGE:
                        overrideCoverage = true;
                        break;
                    case Token::BLEND_SUPPORT_ALL_EQUATIONS:
                        blendSupportAllEquations = true;
                        break;
                    case Token::PUSH_CONSTANT:
                        pushConstant = true;
                        break;
                    case Token::POINTS:
                        primitive = Layout::kPoints_Primitive;
                        break;
                    case Token::LINES:
                        primitive = Layout::kLines_Primitive;
                        break;
                    case Token::LINE_STRIP:
                        primitive = Layout::kLineStrip_Primitive;
                        break;
                    case Token::LINES_ADJACENCY:
                        primitive = Layout::kLinesAdjacency_Primitive;
                        break;
                    case Token::TRIANGLES:
                        primitive = Layout::kTriangles_Primitive;
                        break;
                    case Token::TRIANGLE_STRIP:
                        primitive = Layout::kTriangleStrip_Primitive;
                        break;
                    case Token::TRIANGLES_ADJACENCY:
                        primitive = Layout::kTrianglesAdjacency_Primitive;
                        break;
                    case Token::MAX_VERTICES:
                        maxVertices = this->layoutInt();
                        break;
                    case Token::INVOCATIONS:
                        invocations = this->layoutInt();
                        break;
                }
            } else if (Layout::ReadFormat(t.fText, &format)) {
               // AST::ReadFormat stored the result in 'format'.
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
                  pushConstant, primitive, maxVertices, invocations);
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
std::unique_ptr<ASTStatement> Parser::statement() {
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
        case Token::SWITCH:
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
            return std::unique_ptr<ASTStatement>(new ASTBlock(start.fPosition,
                                                     std::vector<std::unique_ptr<ASTStatement>>()));
        case Token::CONST:   // fall through
        case Token::HIGHP:   // fall through
        case Token::MEDIUMP: // fall through
        case Token::LOWP: {
            auto decl = this->varDeclarations();
            if (!decl) {
                return nullptr;
            }
            return std::unique_ptr<ASTStatement>(new ASTVarDeclarationStatement(std::move(decl)));
        }
        case Token::IDENTIFIER:
            if (this->isType(start.fText)) {
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

/* IDENTIFIER(type) (LBRACKET intLiteral? RBRACKET)* */
std::unique_ptr<ASTType> Parser::type() {
    Token type;
    if (!this->expect(Token::IDENTIFIER, "a type", &type)) {
        return nullptr;
    }
    if (!this->isType(type.fText)) {
        this->error(type.fPosition, ("no type named '" + type.fText + "'").c_str());
        return nullptr;
    }
    std::vector<int> sizes;
    while (this->peek().fKind == Token::LBRACKET) {
        this->expect(Token::LBRACKET, "'['");
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
    return std::unique_ptr<ASTType>(new ASTType(type.fPosition, std::move(type.fText),
                                                ASTType::kIdentifier_Kind, sizes));
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
        this->error(name.fPosition, "no type named '" + name.fText + "'");
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
    String instanceName;
    std::vector<std::unique_ptr<ASTExpression>> sizes;
    if (this->peek().fKind == Token::IDENTIFIER) {
        instanceName = this->nextToken().fText;
        while (this->peek().fKind == Token::LBRACKET) {
            this->expect(Token::LBRACKET, "'['");
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
    }
    this->expect(Token::SEMICOLON, "';'");
    return std::unique_ptr<ASTDeclaration>(new ASTInterfaceBlock(name.fPosition, mods,
                                                                 name.fText, std::move(decls),
                                                                 std::move(instanceName),
                                                                 std::move(sizes)));
}

/* IF LPAREN expression RPAREN statement (ELSE statement)? */
std::unique_ptr<ASTIfStatement> Parser::ifStatement() {
    Token start;
    if (!this->expect(Token::IF, "'if'", &start)) {
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
    if (this->peek().fKind == Token::ELSE) {
        this->nextToken();
        ifFalse = this->statement();
        if (!ifFalse) {
            return nullptr;
        }
    }
    return std::unique_ptr<ASTIfStatement>(new ASTIfStatement(start.fPosition, std::move(test),
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
    return std::unique_ptr<ASTDoStatement>(new ASTDoStatement(start.fPosition,
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
    return std::unique_ptr<ASTWhileStatement>(new ASTWhileStatement(start.fPosition,
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
    return std::unique_ptr<ASTSwitchCase>(new ASTSwitchCase(start.fPosition, std::move(value),
                                                            std::move(statements)));
}

/* SWITCH LPAREN expression RPAREN LBRACE switchCase* (DEFAULT COLON statement*)? RBRACE */
std::unique_ptr<ASTStatement> Parser::switchStatement() {
    Token start;
    if (!this->expect(Token::SWITCH, "'switch'", &start)) {
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
        ASSERT_RESULT(this->expect(Token::DEFAULT, "'default'", &defaultStart));
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
        cases.emplace_back(new ASTSwitchCase(defaultStart.fPosition, nullptr,
                                             std::move(statements)));
    }
    if (!this->expect(Token::RBRACE, "'}'")) {
        return nullptr;
    }
    return std::unique_ptr<ASTStatement>(new ASTSwitchStatement(start.fPosition,
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
            if (this->isType(nextToken.fText)) {
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
    return std::unique_ptr<ASTForStatement>(new ASTForStatement(start.fPosition,
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
    return std::unique_ptr<ASTReturnStatement>(new ASTReturnStatement(start.fPosition,
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
    return std::unique_ptr<ASTBreakStatement>(new ASTBreakStatement(start.fPosition));
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
    return std::unique_ptr<ASTContinueStatement>(new ASTContinueStatement(start.fPosition));
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
    return std::unique_ptr<ASTDiscardStatement>(new ASTDiscardStatement(start.fPosition));
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
                return std::unique_ptr<ASTBlock>(new ASTBlock(start.fPosition,
                                                              std::move(statements)));
            case Token::END_OF_FILE:
                this->error(this->peek().fPosition, "expected '}', but found end of file");
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

/* assignmentExpression */
std::unique_ptr<ASTExpression> Parser::expression() {
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
                                                                                t,
                                                                                std::move(right)));
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
    if (this->peek().fKind == Token::QUESTION) {
        Token question = this->nextToken();
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
    while (this->peek().fKind == Token::LOGICALOR) {
        Token t = this->nextToken();
        std::unique_ptr<ASTExpression> right = this->logicalXorExpression();
        if (!right) {
            return nullptr;
        }
        result.reset(new ASTBinaryExpression(std::move(result), t, std::move(right)));
    }
    return result;
}

/* logicalAndExpression (LOGICALXOR logicalAndExpression)* */
std::unique_ptr<ASTExpression> Parser::logicalXorExpression() {
    std::unique_ptr<ASTExpression> result = this->logicalAndExpression();
    if (!result) {
        return nullptr;
    }
    while (this->peek().fKind == Token::LOGICALXOR) {
        Token t = this->nextToken();
        std::unique_ptr<ASTExpression> right = this->logicalAndExpression();
        if (!right) {
            return nullptr;
        }
        result.reset(new ASTBinaryExpression(std::move(result), t, std::move(right)));
    }
    return result;
}

/* bitwiseOrExpression (LOGICALAND bitwiseOrExpression)* */
std::unique_ptr<ASTExpression> Parser::logicalAndExpression() {
    std::unique_ptr<ASTExpression> result = this->bitwiseOrExpression();
    if (!result) {
        return nullptr;
    }
    while (this->peek().fKind == Token::LOGICALAND) {
        Token t = this->nextToken();
        std::unique_ptr<ASTExpression> right = this->bitwiseOrExpression();
        if (!right) {
            return nullptr;
        }
        result.reset(new ASTBinaryExpression(std::move(result), t, std::move(right)));
    }
    return result;
}

/* bitwiseXorExpression (BITWISEOR bitwiseXorExpression)* */
std::unique_ptr<ASTExpression> Parser::bitwiseOrExpression() {
    std::unique_ptr<ASTExpression> result = this->bitwiseXorExpression();
    if (!result) {
        return nullptr;
    }
    while (this->peek().fKind == Token::BITWISEOR) {
        Token t = this->nextToken();
        std::unique_ptr<ASTExpression> right = this->bitwiseXorExpression();
        if (!right) {
            return nullptr;
        }
        result.reset(new ASTBinaryExpression(std::move(result), t, std::move(right)));
    }
    return result;
}

/* bitwiseAndExpression (BITWISEXOR bitwiseAndExpression)* */
std::unique_ptr<ASTExpression> Parser::bitwiseXorExpression() {
    std::unique_ptr<ASTExpression> result = this->bitwiseAndExpression();
    if (!result) {
        return nullptr;
    }
    while (this->peek().fKind == Token::BITWISEXOR) {
        Token t = this->nextToken();
        std::unique_ptr<ASTExpression> right = this->bitwiseAndExpression();
        if (!right) {
            return nullptr;
        }
        result.reset(new ASTBinaryExpression(std::move(result), t, std::move(right)));
    }
    return result;
}

/* equalityExpression (BITWISEAND equalityExpression)* */
std::unique_ptr<ASTExpression> Parser::bitwiseAndExpression() {
    std::unique_ptr<ASTExpression> result = this->equalityExpression();
    if (!result) {
        return nullptr;
    }
    while (this->peek().fKind == Token::BITWISEAND) {
        Token t = this->nextToken();
        std::unique_ptr<ASTExpression> right = this->equalityExpression();
        if (!right) {
            return nullptr;
        }
        result.reset(new ASTBinaryExpression(std::move(result), t, std::move(right)));
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
                result.reset(new ASTBinaryExpression(std::move(result), t, std::move(right)));
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
                result.reset(new ASTBinaryExpression(std::move(result), t, std::move(right)));
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
                result.reset(new ASTBinaryExpression(std::move(result), t, std::move(right)));
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
                result.reset(new ASTBinaryExpression(std::move(result), t, std::move(right)));
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
                result.reset(new ASTBinaryExpression(std::move(result), t, std::move(right)));
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
            Token t = this->nextToken();
            std::unique_ptr<ASTExpression> expr = this->unaryExpression();
            if (!expr) {
                return nullptr;
            }
            return std::unique_ptr<ASTExpression>(new ASTPrefixExpression(t, std::move(expr)));
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
            case Token::LBRACKET: // fall through
            case Token::DOT:      // fall through
            case Token::LPAREN:   // fall through
            case Token::PLUSPLUS: // fall through
            case Token::MINUSMINUS: {
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
   PLUSPLUS | MINUSMINUS */
std::unique_ptr<ASTSuffix> Parser::suffix() {
    Token next = this->nextToken();
    switch (next.fKind) {
        case Token::LBRACKET: {
            if (this->peek().fKind == Token::RBRACKET) {
                this->nextToken();
                return std::unique_ptr<ASTSuffix>(new ASTIndexSuffix(next.fPosition));
            }
            std::unique_ptr<ASTExpression> e = this->expression();
            if (!e) {
                return nullptr;
            }
            this->expect(Token::RBRACKET, "']' to complete array access expression");
            return std::unique_ptr<ASTSuffix>(new ASTIndexSuffix(std::move(e)));
        }
        case Token::DOT: {
            Position pos = this->peek().fPosition;
            String text;
            if (this->identifier(&text)) {
                return std::unique_ptr<ASTSuffix>(new ASTFieldSuffix(pos, std::move(text)));
            }
            return nullptr;
        }
        case Token::LPAREN: {
            std::vector<std::unique_ptr<ASTExpression>> parameters;
            if (this->peek().fKind != Token::RPAREN) {
                for (;;) {
                    std::unique_ptr<ASTExpression> expr = this->expression();
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
            return std::unique_ptr<ASTSuffix>(new ASTCallSuffix(next.fPosition,
                                                                std::move(parameters)));
        }
        case Token::PLUSPLUS:
            return std::unique_ptr<ASTSuffix>(new ASTSuffix(next.fPosition,
                                                            ASTSuffix::kPostIncrement_Kind));
        case Token::MINUSMINUS:
            return std::unique_ptr<ASTSuffix>(new ASTSuffix(next.fPosition,
                                                            ASTSuffix::kPostDecrement_Kind));
        default: {
            this->error(next.fPosition,  "expected expression suffix, but found '" + next.fText +
                                         "'\n");
            return nullptr;
        }
    }
}

/* IDENTIFIER | intLiteral | floatLiteral | boolLiteral | '(' expression ')' */
std::unique_ptr<ASTExpression> Parser::term() {
    std::unique_ptr<ASTExpression> result;
    Token t = this->peek();
    switch (t.fKind) {
        case Token::IDENTIFIER: {
            String text;
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
bool Parser::identifier(String* dest) {
    Token t;
    if (this->expect(Token::IDENTIFIER, "identifier", &t)) {
        *dest = t.fText;
        return true;
    }
    return false;
}

} // namespace
