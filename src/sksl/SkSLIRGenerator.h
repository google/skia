/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_IRGENERATOR
#define SKSL_IRGENERATOR

#include "src/sksl/SkSLASTFile.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLVariable.h"
#include <memory>

namespace SkSL {

struct ASTFile;
struct Block;
class Context;
class ErrorReporter;
struct FunctionDeclaration;
struct InterfaceBlock;
struct Statement;
struct Swizzle;
class SymbolTable;

enum RefKind {
    kRead_RefKind,
    kWrite_RefKind,
    kReadWrite_RefKind,
    // taking the address of a variable - we consider this a read & write but don't complain if
    // the variable was not previously assigned
    kPointer_RefKind
};

/**
 * Performs semantic analysis on an abstract syntax tree (AST) and produces the corresponding
 * (unoptimized) intermediate representation (IR).
 */
class IRGenerator {
public:
    IRGenerator(ErrorReporter& errorReporter);

    ~IRGenerator();

    void convertProgram(Program::Kind kind,
                        const char* text,
                        size_t length,
                        SymbolTable& types,
                        std::vector<IRNode::ID>* result);

    /**
     * If both operands are compile-time constants and can be folded, returns an expression
     * representing the folded value. Otherwise, returns null. Note that unlike most other functions
     * here, null does not represent a compilation error.
     */
    IRNode::ID constantFold(IRNode::ID left, Token::Kind op, IRNode::ID right);

    IRNode::ID getArg(int offset, String name);

    // FIXME: templatize this. Then remove it when rearchitecture is done
    IRNode::ID createNode(IRNode* node);

    Program::Inputs fInputs;
    const Program::Settings* fSettings;

private:
    std::vector<std::unique_ptr<IRNode>> fLegacyNodes;

public:
    const Context fContext;
    ErrorReporter& fErrors;
    Program::Kind fKind;

// FIXME uncomment
//private:
    /**
     * Prepare to compile a program. Resets state, pushes a new symbol table, and installs the
     * settings.
     */
    void start(const Program::Settings* settings,
               std::vector<IRNode::ID>* inherited);

    /**
     * Performs cleanup after compilation is complete.
     */
    void finish();

    void pushSymbolTable();
    void popSymbolTable();

    IRNode::ID convertVarDeclarations(const ASTNode& decl, Variable::Storage storage);
    void convertFunction(const ASTNode& f);
    IRNode::ID convertStatement(const ASTNode& statement);
    IRNode::ID convertExpression(const ASTNode& expression);
    IRNode::ID convertModifiersDeclaration(const ASTNode& m);

    IRNode::ID convertType(const ASTNode& type);
    IRNode::ID callFunction(int offset, IRNode::ID function, std::vector<IRNode::ID> arguments);
    int callCost(const FunctionDeclaration& function,
                 const std::vector<IRNode::ID>& arguments);
    IRNode::ID callExpression(int offset, IRNode::ID function, std::vector<IRNode::ID> arguments);
    int coercionCost(const Expression& expr, IRNode::ID type);
    IRNode::ID coerce(IRNode::ID expr, IRNode::ID type);
    IRNode::ID convertAppend(int offset, const std::vector<ASTNode>& args);
    IRNode::ID convertBlock(const ASTNode& block);
    IRNode::ID convertBreak(const ASTNode& b);
    IRNode::ID convertNumberConstructor(int offset, IRNode::ID, std::vector<IRNode::ID> params);
    IRNode::ID convertCompoundConstructor(int offset, IRNode::ID, std::vector<IRNode::ID> params);
    IRNode::ID convertConstructor(int offset, IRNode::ID, std::vector<IRNode::ID> params);
    IRNode::ID convertContinue(const ASTNode& c);
    IRNode::ID convertDiscard(const ASTNode& d);
    IRNode::ID convertDo(const ASTNode& d);
    IRNode::ID convertSwitch(const ASTNode& s);
    IRNode::ID convertBinaryExpression(const ASTNode& expression);
    IRNode::ID convertExtension(int offset, StringFragment name);
    IRNode::ID convertExpressionStatement(const ASTNode& s);
    IRNode::ID convertFor(const ASTNode& f);
    IRNode::ID convertIdentifier(const ASTNode& identifier);
    IRNode::ID convertIf(const ASTNode& s);
    IRNode::ID convertIndex(IRNode::ID base, const ASTNode& index);
    IRNode::ID convertInterfaceBlock(const ASTNode& s);
    Modifiers convertModifiers(const Modifiers& m);
    IRNode::ID convertPrefixExpression(const ASTNode& expression);
    IRNode::ID convertReturn(const ASTNode& r);
    IRNode::ID convertSection(const ASTNode& e);
    IRNode::ID getCap(int offset, String name);
    IRNode::ID convertCallExpression(const ASTNode& expression);
    IRNode::ID convertFieldExpression(const ASTNode& expression);
    IRNode::ID convertIndexExpression(const ASTNode& expression);
    IRNode::ID convertPostfixExpression(const ASTNode& expression);
    IRNode::ID convertTypeField(int offset, IRNode::ID type, StringFragment field);
    IRNode::ID convertField(IRNode::ID base, StringFragment field);
    IRNode::ID convertSwizzle(IRNode::ID base, StringFragment fields);
    IRNode::ID convertTernaryExpression(const ASTNode& expression);
    IRNode::ID convertVarDeclarationStatement(const ASTNode& s);
    IRNode::ID convertWhile(const ASTNode& w);
    void convertEnum(const ASTNode& e);
    IRNode::ID applyInvocationIDWorkaround(IRNode::ID main);
    // returns a statement which converts sk_Position from device to normalized coordinates
    IRNode::ID getNormalizeSkPositionCode();

    void checkValid(const Expression& expr);
    void setRefKind(const Expression& expr, RefKind kind);
    void getConstantInt(const Expression& value, int64_t* out);
    bool checkSwizzleWrite(const Swizzle& swizzle);
    IRNode::ID shortCircuitBoolean(IRNode::ID left, Token::Kind op, IRNode::ID right);

    std::unique_ptr<ASTFile> fFile;
    IRNode::ID fCurrentFunction;
    std::unordered_map<String, Program::Settings::Value> fCapsMap;
    std::shared_ptr<SymbolTable> fRootSymbolTable;
    std::shared_ptr<SymbolTable> fSymbolTable;
    // holds extra temp variable declaration statements needed for the current function
    std::vector<IRNode::ID> fExtraVars;
    int fLoopLevel;
    int fSwitchLevel;
    // count of temporary variables we have created
    int fTmpCount;
    int fInvocations;
    std::vector<IRNode::ID>* fProgramElements;
    std::vector<IRNode>* fNodes;

    IRNode::ID fSkPerVertex;
    IRNode::ID fRTAdjust;
    IRNode::ID fRTAdjustInterfaceBlock;
    int fRTAdjustFieldIndex;
    bool fStarted = false;

    friend class AutoSymbolTable;
    friend class AutoLoopLevel;
    friend class AutoSwitchLevel;
    friend class Compiler;
    friend struct IRNode;
};

}

#endif
