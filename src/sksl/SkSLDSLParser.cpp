/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLDSLParser.h"

#include <memory>
#include "stdio.h"

#include "include/private/SkSLModifiers.h"
#include "src/sksl/SkSLASTNode.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"

#ifndef SKSL_STANDALONE
#include "include/private/SkOnce.h"
#endif

using namespace SkSL::dsl;

namespace SkSL {

static constexpr int kMaxParseDepth = 50;
//static constexpr int kMaxStructDepth = 8;

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
        case Token::Kind::TK_INLINE:         return Modifiers::kInline_Flag;
        case Token::Kind::TK_NOINLINE:       return Modifiers::kNoInline_Flag;
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

std::unordered_map<String, DSLParser::LayoutToken>* DSLParser::layoutTokens;

void DSLParser::InitLayoutMap() {
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

DSLParser::DSLParser(Compiler* compiler, int flags, ProgramKind kind, const char* text,
                     size_t length)
    : fCompiler(*compiler)
    , fFlags(flags)
    , fKind(kind)
    , fText(text, length)
    , fPushback(Token::Kind::TK_NONE, -1, -1) {
    fLexer.start(text, length);
    static const bool layoutMapInitialized = []{ return (void)InitLayoutMap(), true; }();
    (void) layoutMapInitialized;
}

/* (directive | section | declaration)* END_OF_FILE */
std::unique_ptr<Program> DSLParser::program() {
    Start(&fCompiler, fKind, fFlags);
    SkASSERT(fCompiler.fIRGenerator->fSymbolTable);
    std::unique_ptr<Program> result;
    bool done = false;
    while (!done) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_END_OF_FILE:
                done = true;
                result = DSLWriter::ReleaseProgram();
                break;
            case Token::Kind::TK_DIRECTIVE: {
                ASTNode::ID dir = this->directive();
                if (this->errors().errorCount()) {
                    done = true;
                    break;
                }
                if (dir) {
                    abort();
                }
                break;
            }
            case Token::Kind::TK_SECTION: {
                ASTNode::ID section = this->section();
                if (this->errors().errorCount()) {
                    done = true;
                    break;
                }
                if (section) {
                    abort();
                }
                break;
            }
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

Token DSLParser::nextRawToken() {
    if (fPushback.fKind != Token::Kind::TK_NONE) {
        Token result = fPushback;
        fPushback.fKind = Token::Kind::TK_NONE;
        return result;
    }
    Token result = fLexer.next();
    return result;
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
    if (this->isType(this->text(*result))) {
        this->error(*result, "expected an identifier, but found type '" +
                             this->text(*result) + "'");
        return false;
    }
    return true;
}

StringFragment DSLParser::text(Token token) {
    return StringFragment(fText.begin() + token.fOffset, token.fLength);
}

void DSLParser::error(Token token, String msg) {
    this->error(token.fOffset, msg);
}

void DSLParser::error(int offset, String msg) {
    this->errors().error(offset, msg);
}

bool DSLParser::isType(StringFragment name) {
    const Symbol* s = this->symbols()[name];
    return s && s->is<Type>();
}

/* DIRECTIVE(#version) INT_LITERAL ("es" | "compatibility")? |
   DIRECTIVE(#extension) IDENTIFIER COLON IDENTIFIER */
ASTNode::ID DSLParser::directive() {
/*    Token start;
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
    }*/
    abort();
}

/* SECTION LBRACE (LPAREN IDENTIFIER RPAREN)? <any sequence of tokens with balanced braces>
   RBRACE */
ASTNode::ID DSLParser::section() {
/*    Token start;
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
    text.fChars = fText.begin() + startOffset;
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
                            ASTNode::SectionData(name, argument, text));*/
    abort();
}

/* ENUM CLASS IDENTIFIER LBRACE (IDENTIFIER (EQ expression)? (COMMA IDENTIFIER (EQ expression))*)?
   RBRACE */
ASTNode::ID DSLParser::enumDeclaration() {
/*    Token start;
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
    this->symbols().add(Type::MakeEnumType(this->text(name)));
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
    return result;*/
    abort();
}

/* enumDeclaration | modifiers (structVarDeclaration | type IDENTIFIER ((LPAREN parameter
   (COMMA parameter)* RPAREN (block | SEMICOLON)) | SEMICOLON) | interfaceBlock) */
bool DSLParser::declaration() {
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
    DSLModifiers modifiers = this->modifiers();
    lookahead = this->peek();
    if (lookahead.fKind == Token::Kind::TK_IDENTIFIER && !this->isType(this->text(lookahead))) {
        // we have an identifier that's not a type, could be the start of an interface block
//        return this->interfaceBlock(modifiers);
        abort();
    }
    if (lookahead.fKind == Token::Kind::TK_STRUCT) {
//        return this->structVarDeclaration(modifiers);
        abort();
    }
    if (lookahead.fKind == Token::Kind::TK_SEMICOLON) {
//        this->nextToken();
//        return this->createNode(lookahead.fOffset, ASTNode::Kind::kModifiers, modifiers);
        abort();
    }
    DSLOptional<DSLType> type = this->type();
    if (!type) {
        return false;
    }
    Token name;
    if (!this->expectIdentifier(&name)) {
        return false;
    }
    if (this->checkNext(Token::Kind::TK_LPAREN)) {
        SkTArray<DSLOptional<DSLWrapper<DSLVar>>> parameters;
        SkTArray<DSLVar*> parameterPointers;
        if (this->peek().fKind != Token::Kind::TK_RPAREN) {
            for (;;) {
                DSLOptional<DSLWrapper<DSLVar>> parameter = this->parameter();
                if (!parameter) {
                    return false;
                }
                parameters.push_back(std::move(parameter));
                parameterPointers.push_back(&**parameters.back());
                if (!this->checkNext(Token::Kind::TK_COMMA)) {
                    break;
                }
            }
        }
        if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
            return false;
        }
        const char* c_name = this->symbols().takeOwnershipOfString(this->text(name))->c_str();
        DSLFunction result(*type, c_name, parameterPointers);
        if (!this->checkNext(Token::Kind::TK_SEMICOLON)) {
            AutoSymbolTable symbols(fCompiler.fIRGenerator.get());
            for (DSLVar* var : parameterPointers) {
                this->symbols().addWithoutOwnership(&DSLWriter::Var(*var));
            }
            DSLOptional<DSLBlock> body = this->block();
            if (!body) {
                return false;
            }
            result.define(std::move(*body));
        }
        return true;
    } else {
        SkTArray<DSLVar> result = this->varDeclarationEnd(modifiers, *type, this->text(name));
        for (DSLVar& var : result) {
            DeclareGlobal(var);
            this->symbols().addWithoutOwnership(&DSLWriter::Var(var));
        }
        return true;
    }
}

static DSLStatement declaration_statements(SkTArray<DSLVar> vars, SymbolTable& symbols) {
    if (vars.empty()) {
        return {};
    }
    if (vars.count() == 1) {
        symbols.addWithoutOwnership(&DSLWriter::Var(vars[0]));
        return Declare(vars[0]);
    }
    DSLBlock result;
    for (DSLVar& var : vars) {
        symbols.addWithoutOwnership(&DSLWriter::Var(var));
        result.append(Declare(var));
    }
    return std::move(result);
}

/* (varDeclarations | expressionStatement) */
DSLOptional<DSLStatement> DSLParser::varDeclarationsOrExpressionStatement() {
    if (this->isType(this->text(this->peek()))) {
        // Statements that begin with a typename are most often variable declarations, but
        // occasionally the type is part of a constructor, and these are actually expression-
        // statements in disguise. First, attempt the common case: parse it as a vardecl.
        Checkpoint checkpoint(this);
        VarDeclarationsPrefix prefix;
        if (this->varDeclarationsPrefix(&prefix)) {
            checkpoint.accept();
            return declaration_statements(this->varDeclarationEnd(prefix.modifiers, prefix.type,
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
    DSLOptional<DSLType> type = this->type();
    if (!type) {
        return false;
    }
    prefixData->type = *type;
    return this->expectIdentifier(&prefixData->name);
}

/* modifiers type IDENTIFIER varDeclarationEnd */
DSLOptional<DSLStatement> DSLParser::varDeclarations() {
    VarDeclarationsPrefix prefix;
    if (!this->varDeclarationsPrefix(&prefix)) {
        return {};
    }
    return declaration_statements(this->varDeclarationEnd(prefix.modifiers, prefix.type,
                                                          this->text(prefix.name)),
                                  this->symbols());
}

/* STRUCT IDENTIFIER LBRACE varDeclaration* RBRACE */
ASTNode::ID DSLParser::structDeclaration() {
/*    if (!this->expect(Token::Kind::TK_STRUCT, "'struct'")) {
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

        const Symbol* symbol = this->symbols()[(declsNode.begin() + 1)->getString()];
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
                type = this->symbols().addArrayDimension(type, arraySize);
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
    this->symbols().add(std::move(newType));
    return this->createNode(name.fOffset, ASTNode::Kind::kType, this->text(name));*/
    abort();
}

/* structDeclaration ((IDENTIFIER varDeclarationEnd) | SEMICOLON) */
ASTNode::ID DSLParser::structVarDeclaration(Modifiers modifiers) {
/*    ASTNode::ID type = this->structDeclaration();
    if (!type) {
        return ASTNode::ID::Invalid();
    }
    Token name;
    if (this->checkNext(Token::Kind::TK_IDENTIFIER, &name)) {
        return this->varDeclarationEnd(modifiers, std::move(type), this->text(name));
    }
    this->expect(Token::Kind::TK_SEMICOLON, "';'");
    return type;*/
    abort();
}

/* (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)? (COMMA IDENTIFER
   (LBRACKET expression? RBRACKET)* (EQ assignmentExpression)?)* SEMICOLON */
SkTArray<dsl::DSLVar> DSLParser::varDeclarationEnd(DSLModifiers mods, DSLType type,
                                              StringFragment name) {
    SkTArray<dsl::DSLVar> result;
/*    int offset = this->peek().fOffset;
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
    };*/

/*    if (!parseArrayDimensions(currentVar, &vd)) {
        return nullptr;
    }*/
    DSLOptional<DSLWrapper<DSLExpression>> value;
    if (this->checkNext(Token::Kind::TK_EQ)) {
        value = this->assignmentExpression();
        if (!value) {
            return {};
        }
    }

    result.emplace_back(mods, type, this->symbols().takeOwnershipOfString(name)->c_str(),
                        value ? std::move(**value) : DSLExpression());
//    while (this->checkNext(Token::Kind::TK_COMMA)) {
//        Token identifierName;
//        if (!this->expectIdentifier(&identifierName)) {
//            return {};
//        }
//
//        currentVar = ASTNode::ID(fFile->fNodes.size());
//        vd = ASTNode::VarData{this->text(identifierName), /*isArray=*/false};
//        fFile->fNodes.emplace_back(&fFile->fNodes, offset, ASTNode::Kind::kVarDeclaration);
//
//        getNode(result).addChild(currentVar);
//        if (!parseArrayDimensions(currentVar, &vd)) {
//            return {};
//        }
//        getNode(currentVar).setVarData(vd);
//        if (!parseInitializer(currentVar)) {
//            return {};
//        }
//    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return {};
    }
    return result;
}

/* modifiers type IDENTIFIER (LBRACKET INT_LITERAL RBRACKET)? */
DSLOptional<DSLWrapper<DSLVar>> DSLParser::parameter() {
    DSLModifiers modifiers = this->modifiersWithDefaults(0);
    DSLOptional<DSLType> type = this->type();
    if (!type) {
        return {};
    }
    Token name;
    if (!this->expectIdentifier(&name)) {
        return {};
    }
//    bool isArray = false;
    while (this->checkNext(Token::Kind::TK_LBRACKET)) {
/*        if (isArray || this->isArrayType(type)) {
            this->error(this->peek(), "multi-dimensional arrays are not supported");
            return {};
        }
        Token sizeToken;
        if (!this->expect(Token::Kind::TK_INT_LITERAL, "a positive integer", &sizeToken)) {
            return {};
        }
        StringFragment arraySizeFrag = this->text(sizeToken);
        SKSL_INT arraySize;
        if (!SkSL::stoi(arraySizeFrag, &arraySize)) {
            this->error(sizeToken, "array size is too large: " + arraySizeFrag);
            return {};
        }
        this->addChild(result, this->createNode(sizeToken.fOffset, ASTNode::Kind::kInt, arraySize));
        if (!this->expect(Token::Kind::TK_RBRACKET, "']'")) {
            return {};
        }
        isArray = true;*/
        abort();
    }
    const char* c_name = this->symbols().takeOwnershipOfString(this->text(name))->c_str();
    return {{DSLVar(modifiers, *type, c_name)}};
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
    StringFragment resultFrag = this->text(resultToken);
    SKSL_INT resultValue;
    if (!SkSL::stoi(resultFrag, &resultValue)) {
        this->error(resultToken, "value in layout is too large: " + resultFrag);
        return -1;
    }
    return resultValue;
}

/** EQ IDENTIFIER */
StringFragment DSLParser::layoutIdentifier() {
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
StringFragment DSLParser::layoutCode() {
    if (!this->expect(Token::Kind::TK_EQ, "'='")) {
        return "";
    }
    Token start = this->nextRawToken();
    this->pushback(start);
    StringFragment code;
    code.fChars = fText.begin() + start.fOffset;
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

Layout::CType DSLParser::layoutCType() {
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
DSLLayout DSLParser::layout() {
    DSLLayout result;
    if (this->checkNext(Token::Kind::TK_LAYOUT)) {
        if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
            return result;
        }
        for (;;) {
            Token t = this->nextToken();
            String text = this->text(t);
            auto found = layoutTokens->find(text);
            if (found != layoutTokens->end()) {
                switch (found->second) {
                    case LayoutToken::ORIGIN_UPPER_LEFT:
                        result.originUpperLeft();
                        break;
                    case LayoutToken::OVERRIDE_COVERAGE:
                        result.overrideCoverage();
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
        // TODO: handle duplicate / incompatible flags
        int tokenFlag = parse_modifier_token(peek().fKind);
        if (!tokenFlag) {
            break;
        }
        flags |= tokenFlag;
        this->nextToken();
    }
    (void) layout; // FIXME need DSL layout support
    return DSLModifiers(std::move(layout), flags);
}

DSLModifiers DSLParser::modifiersWithDefaults(int defaultFlags) {
    DSLModifiers result = this->modifiers();
    if (defaultFlags && !result.flags()) {
        //return Modifiers(result.fLayout, defaultFlags);
        abort();
    }
    return result;
}

/* ifStatement | forStatement | doStatement | whileStatement | block | expression */
DSLOptional<DSLStatement> DSLParser::statement() {
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
            DSLOptional<DSLBlock> result = this->block();
            return result ? DSLOptional<DSLStatement>(std::move(*result))
                          : DSLOptional<DSLStatement>();
        }
        case Token::Kind::TK_SEMICOLON:
            this->nextToken();
            return dsl::Block();
        case Token::Kind::TK_CONST:
            return this->varDeclarations();
        case Token::Kind::TK_IDENTIFIER:
            return this->varDeclarationsOrExpressionStatement();
        default:
            return this->expressionStatement();
    }
}

/* IDENTIFIER(type) (LBRACKET intLiteral? RBRACKET)* QUESTION? */
DSLOptional<DSLType> DSLParser::type() {
    Token type;
    if (!this->expect(Token::Kind::TK_IDENTIFIER, "a type", &type)) {
        return {};
    }
    if (!this->isType(this->text(type))) {
        this->error(type, ("no type named '" + this->text(type) + "'").c_str());
        return {};
    }
    ASTNode result(nullptr, type.fOffset, ASTNode::Kind::kType, this->text(type));
    while (this->checkNext(Token::Kind::TK_LBRACKET)) {
        abort();
/*        if (isArray) {
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
        this->expect(Token::Kind::TK_RBRACKET, "']'");*/
    }
    return fCompiler.fIRGenerator->convertType(result, true);
}

/* IDENTIFIER LBRACE
     varDeclaration+
   RBRACE (IDENTIFIER (LBRACKET expression? RBRACKET)*)? SEMICOLON */
ASTNode::ID DSLParser::interfaceBlock(Modifiers mods) {
/*    Token name;
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
    return result;*/
    abort();
}

/* IF LPAREN expression RPAREN statement (ELSE statement)? */
DSLOptional<DSLStatement> DSLParser::ifStatement() {
    Token start;
    bool isStatic = this->checkNext(Token::Kind::TK_STATIC_IF, &start);
    if (!isStatic && !this->expect(Token::Kind::TK_IF, "'if'", &start)) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
        return {};
    }
    DSLOptional<DSLWrapper<DSLExpression>> test = this->expression();
    if (!test) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
        return {};
    }
    DSLOptional<DSLStatement> ifTrue = this->statement();
    if (!ifTrue) {
        return {};
    }
    DSLOptional<DSLStatement> ifFalse;
    if (this->checkNext(Token::Kind::TK_ELSE)) {
        ifFalse = this->statement();
        if (!ifFalse) {
            return {};
        }
    }
    if (isStatic) {
        return StaticIf(std::move(**test), std::move(*ifTrue), ifFalse ? std::move(*ifFalse)
                                                                       : DSLStatement());
    } else {
        return If(std::move(**test), std::move(*ifTrue), ifFalse ? std::move(*ifFalse)
                                                                 : DSLStatement());
    }
}

/* DO statement WHILE LPAREN expression RPAREN SEMICOLON */
DSLOptional<DSLStatement> DSLParser::doStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_DO, "'do'", &start)) {
        return {};
    }
    DSLOptional<DSLStatement> statement = this->statement();
    if (!statement) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_WHILE, "'while'")) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
        return {};
    }
    DSLOptional<DSLWrapper<DSLExpression>> test = this->expression();
    if (!test) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return {};
    }
    return Do(std::move(*statement), std::move(**test));
}

/* WHILE LPAREN expression RPAREN STATEMENT */
DSLOptional<DSLStatement> DSLParser::whileStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_WHILE, "'while'", &start)) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
        return {};
    }
    DSLOptional<DSLWrapper<DSLExpression>> test = this->expression();
    if (!test) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
        return {};
    }
    DSLOptional<DSLStatement> statement = this->statement();
    if (!statement) {
        return {};
    }
    return While(std::move(**test), std::move(*statement));
}

/* CASE expression COLON statement* */
DSLOptional<DSLCase> DSLParser::switchCase() {
    Token start;
    if (!this->expect(Token::Kind::TK_CASE, "'case'", &start)) {
        return {};
    }
    DSLOptional<DSLWrapper<DSLExpression>> value = this->expression();
    if (!value) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_COLON, "':'")) {
        return {};
    }
    SkTArray<DSLStatement> statements;
    while (this->peek().fKind != Token::Kind::TK_RBRACE &&
           this->peek().fKind != Token::Kind::TK_CASE &&
           this->peek().fKind != Token::Kind::TK_DEFAULT) {
        DSLOptional<DSLStatement> s = this->statement();
        if (!s) {
            return {};
        }
        statements.push_back(std::move(*s));
    }
    return DSLCase(std::move(**value), std::move(statements));
}

/* SWITCH LPAREN expression RPAREN LBRACE switchCase* (DEFAULT COLON statement*)? RBRACE */
DSLOptional<DSLStatement> DSLParser::switchStatement() {
    Token start;
    bool isStatic = this->checkNext(Token::Kind::TK_STATIC_SWITCH, &start);
    if (!isStatic && !this->expect(Token::Kind::TK_SWITCH, "'switch'", &start)) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
        return {};
    }
    DSLOptional<DSLWrapper<DSLExpression>> value = this->expression();
    if (!value) {
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
        DSLOptional<DSLCase> c = this->switchCase();
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
            DSLOptional<DSLStatement> s = this->statement();
            if (!s) {
                return {};
            }
            statements.push_back(std::move(*s));
        }
        cases.emplace_back(DSLExpression(), std::move(statements));
    }
    if (!this->expect(Token::Kind::TK_RBRACE, "'}'")) {
        return {};
    }
    if (isStatic) {
        return StaticSwitch(std::move(**value), std::move(cases));
    } else {
        return Switch(std::move(**value), std::move(cases));
    }
}

/* FOR LPAREN (declaration | expression)? SEMICOLON expression? SEMICOLON expression? RPAREN
   STATEMENT */
DSLOptional<dsl::DSLStatement> DSLParser::forStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_FOR, "'for'", &start)) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_LPAREN, "'('")) {
        return {};
    }
    DSLOptional<dsl::DSLStatement> initializer;
    Token nextToken = this->peek();
    switch (nextToken.fKind) {
        case Token::Kind::TK_SEMICOLON:
            this->nextToken();
            break;
        case Token::Kind::TK_CONST: {
            initializer = this->varDeclarations();
            if (!initializer) {
                return {};
            }
            break;
        }
        case Token::Kind::TK_IDENTIFIER: {
            if (this->isType(this->text(nextToken))) {
                initializer = this->varDeclarations();
                if (!initializer) {
                    return {};
                }
                break;
            }
            [[fallthrough]];
        }
        default:
            initializer = this->expressionStatement();
            if (!initializer) {
                return {};
            }
    }
    DSLOptional<DSLWrapper<DSLExpression>> test;
    if (this->peek().fKind != Token::Kind::TK_SEMICOLON) {
        test = this->expression();
        if (!test) {
            return {};
        }
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return {};
    }
    DSLOptional<DSLWrapper<DSLExpression>> next;
    if (this->peek().fKind != Token::Kind::TK_RPAREN) {
        next = this->expression();
        if (!next) {
            return {};
        }
    }
    if (!this->expect(Token::Kind::TK_RPAREN, "')'")) {
        return {};
    }
    DSLOptional<dsl::DSLStatement> statement = this->statement();
    if (!statement) {
        return {};
    }
    return For(initializer ? std::move(*initializer) : DSLStatement(),
               test ? std::move(**test) : DSLExpression(),
               next ? std::move(**next) : DSLExpression(),
               std::move(*statement));
}

/* RETURN expression? SEMICOLON */
DSLOptional<DSLStatement> DSLParser::returnStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_RETURN, "'return'", &start)) {
        return {};
    }
    DSLOptional<DSLWrapper<DSLExpression>> expression;
    if (this->peek().fKind != Token::Kind::TK_SEMICOLON) {
        expression = this->expression();
        if (!expression) {
            return {};
        }
    }
    this->expect(Token::Kind::TK_SEMICOLON, "';'");
    return Return(expression ? std::move(**expression) : DSLExpression());
}

/* BREAK SEMICOLON */
DSLOptional<DSLStatement> DSLParser::breakStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_BREAK, "'break'", &start)) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return {};
    }
    return Break();
}

/* CONTINUE SEMICOLON */
DSLOptional<DSLStatement> DSLParser::continueStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_CONTINUE, "'continue'", &start)) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return {};
    }
    return Continue();
}

/* DISCARD SEMICOLON */
DSLOptional<DSLStatement> DSLParser::discardStatement() {
    Token start;
    if (!this->expect(Token::Kind::TK_DISCARD, "'continue'", &start)) {
        return {};
    }
    if (!this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
        return {};
    }
    return Discard();
}

/* LBRACE statement* RBRACE */
DSLOptional<DSLBlock> DSLParser::block() {
    Token start;
    if (!this->expect(Token::Kind::TK_LBRACE, "'{'", &start)) {
        return {};
    }
    AutoDSLDepth depth(this);
    if (!depth.increase()) {
        return {};
    }
    AutoSymbolTable symbols(fCompiler.fIRGenerator.get());
    SkTArray<DSLStatement> statements;
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_RBRACE:
                this->nextToken();
                return DSLBlock(std::move(statements), fCompiler.fIRGenerator->fSymbolTable);
            case Token::Kind::TK_END_OF_FILE:
                this->error(this->peek(), "expected '}', but found end of file");
                return {};
            default: {
                DSLOptional<DSLStatement> statement = this->statement();
                if (!statement) {
                    return {};
                }
                statements.push_back(std::move(*statement));
            }
        }
    }
}

/* expression SEMICOLON */
DSLOptional<DSLStatement> DSLParser::expressionStatement() {
    DSLOptional<DSLWrapper<DSLExpression>> expr = this->expression();
    if (expr) {
        if (this->expect(Token::Kind::TK_SEMICOLON, "';'")) {
            return {{DSLStatement(std::move(**expr))}};
        }
    }
    return {};
}

/* assignmentExpression (COMMA assignmentExpression)* */
DSLOptional<DSLWrapper<DSLExpression>> DSLParser::expression() {
    DSLOptional<DSLWrapper<DSLExpression>> result = this->assignmentExpression();
    if (!result) {
        return {};
    }
    Token t;
    AutoDSLDepth depth(this);
    while (this->checkNext(Token::Kind::TK_COMMA, &t)) {
        if (!depth.increase()) {
            return {};
        }
        DSLOptional<DSLWrapper<DSLExpression>> right = this->expression();
        if (!right) {
            return {};
        }
        //result = DSLOptional<DSLWrapper<DSLExpression>>(std::move(**result) , std::move(**right));
        abort();
        break;
    }
    return result;
}

#define OPERATOR_RIGHT(op, exprType)                                       \
    {                                                                      \
        this->nextToken();                                                 \
        if (!depth.increase()) {                                           \
            return {};                                                     \
        }                                                                  \
        DSLOptional<DSLWrapper<DSLExpression>> right = this->exprType(); \
        if (!right) {                                                      \
            return {};                                                     \
        }                                                                  \
        result = {{std::move(**result) op std::move(**right)}};            \
        break;                                                             \
    }

/* ternaryExpression ((EQEQ | STAREQ | SLASHEQ | PERCENTEQ | PLUSEQ | MINUSEQ | SHLEQ | SHREQ |
   BITWISEANDEQ | BITWISEXOREQ | BITWISEOREQ | LOGICALANDEQ | LOGICALXOREQ | LOGICALOREQ)
   assignmentExpression)*
 */
DSLOptional<DSLWrapper<DSLExpression>> DSLParser::assignmentExpression() {
    AutoDSLDepth depth(this);
    DSLOptional<DSLWrapper<DSLExpression>> result = this->ternaryExpression();
    if (!result) {
        return {};
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_EQ:           OPERATOR_RIGHT(=,   assignmentExpression)
            case Token::Kind::TK_STAREQ:       OPERATOR_RIGHT(*=,  assignmentExpression)
            case Token::Kind::TK_SLASHEQ:      OPERATOR_RIGHT(/=,  assignmentExpression)
            case Token::Kind::TK_PERCENTEQ:    OPERATOR_RIGHT(%=,  assignmentExpression)
            case Token::Kind::TK_PLUSEQ:       OPERATOR_RIGHT(+=,  assignmentExpression)
            case Token::Kind::TK_MINUSEQ:      OPERATOR_RIGHT(-=,  assignmentExpression)
            case Token::Kind::TK_SHLEQ:        OPERATOR_RIGHT(<<=, assignmentExpression)
            case Token::Kind::TK_SHREQ:        OPERATOR_RIGHT(>>=, assignmentExpression)
            case Token::Kind::TK_BITWISEANDEQ: OPERATOR_RIGHT(&=,  assignmentExpression)
            case Token::Kind::TK_BITWISEXOREQ: OPERATOR_RIGHT(^=,  assignmentExpression)
            case Token::Kind::TK_BITWISEOREQ:  OPERATOR_RIGHT(|=,  assignmentExpression)
            default:
                return result;
        }
    }
}

/* logicalOrExpression ('?' expression ':' assignmentExpression)? */
DSLOptional<DSLWrapper<DSLExpression>> DSLParser::ternaryExpression() {
    AutoDSLDepth depth(this);
    DSLOptional<DSLWrapper<DSLExpression>> base = this->logicalOrExpression();
    if (!base) {
        return {};
    }
    if (this->checkNext(Token::Kind::TK_QUESTION)) {
        if (!depth.increase()) {
            return {};
        }
        DSLOptional<DSLWrapper<DSLExpression>> trueExpr = this->expression();
        if (!trueExpr) {
            return {};
        }
        if (this->expect(Token::Kind::TK_COLON, "':'")) {
            DSLOptional<DSLWrapper<DSLExpression>> falseExpr = this->assignmentExpression();
            if (!falseExpr) {
                return {};
            }
            return Select(std::move(**base), std::move(**trueExpr), std::move(**falseExpr));
        }
        return {};
    }
    return base;
}

/* logicalXorExpression (LOGICALOR logicalXorExpression)* */
DSLOptional<DSLWrapper<DSLExpression>> DSLParser::logicalOrExpression() {
    AutoDSLDepth depth(this);
    DSLOptional<DSLWrapper<DSLExpression>> result = this->logicalXorExpression();
    if (!result) {
        return {};
    }
    while (this->peek().fKind == Token::Kind::TK_LOGICALOR) {
        OPERATOR_RIGHT(||, logicalOrExpression)
    }
    return result;
}

/* logicalAndExpression (LOGICALXOR logicalAndExpression)* */
DSLOptional<DSLWrapper<DSLExpression>> DSLParser::logicalXorExpression() {
    AutoDSLDepth depth(this);
    DSLOptional<DSLWrapper<DSLExpression>> result = this->logicalAndExpression();
    if (!result) {
        return {};
    }
    Token t;
    while (this->peek().fKind == Token::Kind::TK_LOGICALXOR) {
//        OPERATOR_RIGHT(^^, logicalXorExpression)
        abort();
    }
    return result;
}

/* bitwiseOrExpression (LOGICALAND bitwiseOrExpression)* */
DSLOptional<DSLWrapper<DSLExpression>> DSLParser::logicalAndExpression() {
    AutoDSLDepth depth(this);
    DSLOptional<DSLWrapper<DSLExpression>> result = this->bitwiseOrExpression();
    if (!result) {
        return {};
    }
    Token t;
    while (this->peek().fKind == Token::Kind::TK_LOGICALAND) {
        OPERATOR_RIGHT(&&, logicalAndExpression)
    }
    return result;
}

/* bitwiseXorExpression (BITWISEOR bitwiseXorExpression)* */
DSLOptional<DSLWrapper<DSLExpression>> DSLParser::bitwiseOrExpression() {
    AutoDSLDepth depth(this);
    DSLOptional<DSLWrapper<DSLExpression>> result = this->bitwiseXorExpression();
    if (!result) {
        return {};
    }
    Token t;
    while (this->peek().fKind == Token::Kind::TK_BITWISEOR) {
        OPERATOR_RIGHT(|, bitwiseOrExpression)
    }
    return result;
}

/* bitwiseAndExpression (BITWISEXOR bitwiseAndExpression)* */
DSLOptional<DSLWrapper<DSLExpression>> DSLParser::bitwiseXorExpression() {
    AutoDSLDepth depth(this);
    DSLOptional<DSLWrapper<DSLExpression>> result = this->bitwiseAndExpression();
    if (!result) {
        return {};
    }
    Token t;
    while (this->peek().fKind == Token::Kind::TK_BITWISEXOR) {
        OPERATOR_RIGHT(^, bitwiseXorExpression)
    }
    return result;
}

/* equalityExpression (BITWISEAND equalityExpression)* */
DSLOptional<DSLWrapper<DSLExpression>> DSLParser::bitwiseAndExpression() {
    AutoDSLDepth depth(this);
    DSLOptional<DSLWrapper<DSLExpression>> result = this->equalityExpression();
    if (!result) {
        return {};
    }
    Token t;
    while (this->peek().fKind == Token::Kind::TK_BITWISEAND) {
        OPERATOR_RIGHT(&, bitwiseAndExpression)
    }
    return result;
}

/* relationalExpression ((EQEQ | NEQ) relationalExpression)* */
DSLOptional<DSLWrapper<DSLExpression>> DSLParser::equalityExpression() {
    AutoDSLDepth depth(this);
    DSLOptional<DSLWrapper<DSLExpression>> result = this->relationalExpression();
    if (!result) {
        return {};
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_EQEQ: OPERATOR_RIGHT(==, equalityExpression)
            case Token::Kind::TK_NEQ:  OPERATOR_RIGHT(!=, equalityExpression)
            default: return result;
        }
    }
}

/* shiftExpression ((LT | GT | LTEQ | GTEQ) shiftExpression)* */
DSLOptional<DSLWrapper<DSLExpression>> DSLParser::relationalExpression() {
    AutoDSLDepth depth(this);
    DSLOptional<DSLWrapper<DSLExpression>> result = this->shiftExpression();
    if (!result) {
        return {};
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_LT:   OPERATOR_RIGHT(<,  relationalExpression)
            case Token::Kind::TK_GT:   OPERATOR_RIGHT(>,  relationalExpression)
            case Token::Kind::TK_LTEQ: OPERATOR_RIGHT(<=, relationalExpression)
            case Token::Kind::TK_GTEQ: OPERATOR_RIGHT(>=, relationalExpression)
            default: return result;
        }
    }
}

/* additiveExpression ((SHL | SHR) additiveExpression)* */
DSLOptional<DSLWrapper<DSLExpression>> DSLParser::shiftExpression() {
    AutoDSLDepth depth(this);
    DSLOptional<DSLWrapper<DSLExpression>> result = this->additiveExpression();
    if (!result) {
        return {};
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_SHL: OPERATOR_RIGHT(<<, shiftExpression)
            case Token::Kind::TK_SHR: OPERATOR_RIGHT(>>, shiftExpression)
            default: return result;
        }
    }
}

/* multiplicativeExpression ((PLUS | MINUS) multiplicativeExpression)* */
DSLOptional<DSLWrapper<DSLExpression>> DSLParser::additiveExpression() {
    AutoDSLDepth depth(this);
    DSLOptional<DSLWrapper<DSLExpression>> result = this->multiplicativeExpression();
    if (!result) {
        return {};
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_PLUS:  OPERATOR_RIGHT(+, additiveExpression)
            case Token::Kind::TK_MINUS: OPERATOR_RIGHT(-, additiveExpression)
            default: return result;
        }
    }
}

/* unaryExpression ((STAR | SLASH | PERCENT) unaryExpression)* */
DSLOptional<DSLWrapper<DSLExpression>> DSLParser::multiplicativeExpression() {
    AutoDSLDepth depth(this);
    DSLOptional<DSLWrapper<DSLExpression>> result = this->unaryExpression();
    if (!result) {
        return {};
    }
    for (;;) {
        switch (this->peek().fKind) {
            case Token::Kind::TK_STAR:    OPERATOR_RIGHT(*, multiplicativeExpression)
            case Token::Kind::TK_SLASH:   OPERATOR_RIGHT(/, multiplicativeExpression)
            case Token::Kind::TK_PERCENT: OPERATOR_RIGHT(%, multiplicativeExpression)
            default: return result;
        }
    }
}

/* postfixExpression | (PLUS | MINUS | NOT | PLUSPLUS | MINUSMINUS) unaryExpression */
DSLOptional<DSLWrapper<DSLExpression>> DSLParser::unaryExpression() {
    AutoDSLDepth depth(this);
    switch (this->peek().fKind) {
        case Token::Kind::TK_PLUS: {
            if (!depth.increase()) {
                return {};
            }
            this->nextToken();
            DSLOptional<DSLWrapper<DSLExpression>> expr = this->unaryExpression();
            if (!expr) {
                return {};
            }
            return {{+std::move(**expr)}};
        }
        case Token::Kind::TK_MINUS: {
            if (!depth.increase()) {
                return {};
            }
            this->nextToken();
            DSLOptional<DSLWrapper<DSLExpression>> expr = this->unaryExpression();
            if (!expr) {
                return {};
            }
            return {{-std::move(**expr)}};
        }
        case Token::Kind::TK_LOGICALNOT: {
            if (!depth.increase()) {
                return {};
            }
            this->nextToken();
            DSLOptional<DSLWrapper<DSLExpression>> expr = this->unaryExpression();
            if (!expr) {
                return {};
            }
            return {{!std::move(**expr)}};
        }
        case Token::Kind::TK_BITWISENOT: {
            if (!depth.increase()) {
                return {};
            }
            this->nextToken();
            DSLOptional<DSLWrapper<DSLExpression>> expr = this->unaryExpression();
            if (!expr) {
                return {};
            }
            return {{~std::move(**expr)}};
        }
        case Token::Kind::TK_PLUSPLUS: {
            if (!depth.increase()) {
                return {};
            }
            this->nextToken();
            DSLOptional<DSLWrapper<DSLExpression>> expr = this->unaryExpression();
            if (!expr) {
                return {};
            }
            return {{++std::move(**expr)}};
        }
        case Token::Kind::TK_MINUSMINUS: {
            if (!depth.increase()) {
                return {};
            }
            this->nextToken();
            DSLOptional<DSLWrapper<DSLExpression>> expr = this->unaryExpression();
            if (!expr) {
                return {};
            }
            return {{--std::move(**expr)}};
        }
        default:
            return this->postfixExpression();
    }
}

/* term suffix* */
DSLOptional<DSLWrapper<DSLExpression>> DSLParser::postfixExpression() {
    AutoDSLDepth depth(this);
    DSLOptional<DSLWrapper<DSLExpression>> result = this->term();
    if (!result) {
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
            case Token::Kind::TK_MINUSMINUS:
            case Token::Kind::TK_COLONCOLON:
                if (!depth.increase()) {
                    return {};
                }
                result = this->suffix(std::move(result));
                if (!result) {
                    return {};
                }
                break;
            default:
                return result;
        }
    }
}

DSLOptional<DSLWrapper<DSLExpression>> DSLParser::swizzle(int offset,
        DSLOptional<DSLWrapper<DSLExpression>> base, const char* swizzleMask) {
    if (!(**base).type().isVector()) {
        return (**base).field(swizzleMask);
    }
    int length = strlen(swizzleMask);
    if (length == 0 || length > 4) {
        this->error(offset, "invalid swizzle");
        return {};
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
                this->error(offset, "invalid swizzle");
                return {};
        }
    }
    switch (length) {
        case 1: return dsl::Swizzle(std::move(**base), components[0]);
        case 2: return dsl::Swizzle(std::move(**base), components[0], components[1]);
        case 3: return dsl::Swizzle(std::move(**base), components[0], components[1],
                                    components[2]);
        case 4: return dsl::Swizzle(std::move(**base), components[0], components[1],
                                    components[2], components[3]);
        default: SkUNREACHABLE;
    }
}

DSLOptional<dsl::Wrapper<dsl::DSLExpression>> DSLParser::call(int offset,
        DSLOptional<dsl::Wrapper<dsl::DSLExpression>> base,
        SkTArray<Wrapper<DSLExpression>> args) {
    return {{(**base)(std::move(args))}};
}

/* LBRACKET expression? RBRACKET | DOT IDENTIFIER | LPAREN arguments RPAREN |
   PLUSPLUS | MINUSMINUS | COLONCOLON IDENTIFIER | FLOAT_LITERAL [IDENTIFIER] */
DSLOptional<DSLWrapper<DSLExpression>> DSLParser::suffix(
        DSLOptional<DSLWrapper<DSLExpression>> base) {
    SkASSERT(base);
    Token next = this->nextToken();
    AutoDSLDepth depth(this);
    if (!depth.increase()) {
        return {};
    }
    switch (next.fKind) {
        case Token::Kind::TK_LBRACKET: {
            DSLOptional<DSLWrapper<DSLExpression>> index = this->expression();
            if (!index) {
                return {};
            }
            this->expect(Token::Kind::TK_RBRACKET, "']' to complete array access expression");
            return {{std::move(**base)[std::move(**index)]}};
        }
        case Token::Kind::TK_COLONCOLON:
            abort();
        case Token::Kind::TK_DOT: {
            int offset = this->peek().fOffset;
            StringFragment text;
            if (this->identifier(&text)) {
                return this->swizzle(offset, std::move(**base), String(text).c_str());
            }
            [[fallthrough]];
        }
        case Token::Kind::TK_FLOAT_LITERAL: {
            // Swizzles *that start with a constant number, e.g. '.000r', will be tokenized as
            // floating point literals, possibly followed by an identifier. Handle that here.
            StringFragment field = this->text(next);
            SkASSERT(field.fChars[0] == '.');
            ++field.fChars;
            --field.fLength;
            // use the next *raw* token so we don't ignore whitespace - we only care about
            // identifiers that directly follow the float
            Token id = this->nextRawToken();
            if (id.fKind == Token::Kind::TK_IDENTIFIER) {
                return this->swizzle(next.fOffset, std::move(**base),
                                     (field + this->text(id)).c_str());
            }
            this->pushback(id);
            return this->swizzle(next.fOffset, std::move(**base), String(field).c_str());
        }
        case Token::Kind::TK_LPAREN: {
            SkTArray<Wrapper<DSLExpression>> args;
            if (this->peek().fKind != Token::Kind::TK_RPAREN) {
                for (;;) {
                    DSLOptional<DSLWrapper<DSLExpression>> expr = this->assignmentExpression();
                    if (!expr) {
                        return {};
                    }
                    args.push_back(std::move(*expr));
                    if (!this->checkNext(Token::Kind::TK_COMMA)) {
                        break;
                    }
                }
            }
            this->expect(Token::Kind::TK_RPAREN, "')' to complete function arguments");
            return call(next.fOffset, std::move(**base), std::move(args));
        }
        case Token::Kind::TK_PLUSPLUS:
            return {{std::move(**base)++}};
        case Token::Kind::TK_MINUSMINUS: {
            return {{std::move(**base)--}};
        }
        default: {
            this->error(next,  "expected expression suffix, but found '" + this->text(next) + "'");
            return {};
        }
    }
}

/* IDENTIFIER | intLiteral | floatLiteral | boolLiteral | '(' expression ')' */
DSLOptional<DSLWrapper<DSLExpression>> DSLParser::term() {
    Token t = this->peek();
    switch (t.fKind) {
        case Token::Kind::TK_IDENTIFIER: {
            StringFragment text;
            if (this->identifier(&text)) {
                std::unique_ptr<Expression> result =
                                         fCompiler.fIRGenerator->convertIdentifier(t.fOffset, text);
                if (!result.get()) {
                    return {};
                }
                return {{std::move(result)}};
            }
            break;
        }
        case Token::Kind::TK_INT_LITERAL: {
            SKSL_INT i;
            if (this->intLiteral(&i)) {
                return {{(int) i}};
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
                return {};
            }
            DSLOptional<DSLWrapper<DSLExpression>> result = this->expression();
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
    return {};
}

/* INT_LITERAL */
bool DSLParser::intLiteral(SKSL_INT* dest) {
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
bool DSLParser::floatLiteral(SKSL_FLOAT* dest) {
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
bool DSLParser::identifier(StringFragment* dest) {
    Token t;
    if (this->expect(Token::Kind::TK_IDENTIFIER, "identifier", &t)) {
        *dest = this->text(t);
        return true;
    }
    return false;
}

}  // namespace SkSL
