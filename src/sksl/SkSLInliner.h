/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INLINER
#define SKSL_INLINER

#include <memory>
#include <unordered_map>

#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

class Block;
class Context;
struct Expression;
struct FunctionCall;
struct Statement;
class SymbolTable;
struct Variable;

/**
 * Converts a FunctionCall in the IR to a set of statements to be injected ahead of the function
 * call, and a replacement expression. Can also detect cases where inlining isn't cleanly possible
 * (e.g. return statements nested inside of a loop construct). The inliner isn't able to guarantee
 * identical-to-GLSL execution order if the inlined function has visible side effects.
 */
class Inliner {
public:
    Inliner() {}

    void reset(const Context&, const Program::Settings&);

    /**
     * Processes the passed-in FunctionCall expression. The FunctionCall expression should be
     * replaced with `fReplacementExpr`. If non-null, `fInlinedBody` should be inserted immediately
     * above the statement containing the inlined expression.
     */
    struct InlinedCall {
        std::unique_ptr<Block> fInlinedBody;
        std::unique_ptr<Expression> fReplacementExpr;
    };
    InlinedCall inlineCall(FunctionCall*, SymbolTable*, const FunctionDeclaration* caller);

    /** Adds a scope to inlined bodies returned by `inlineCall`, if one is required. */
    void ensureScopedBlocks(Statement* inlinedBody, Statement* parentStmt);

    /** Checks whether inlining is viable for a FunctionCall. */
    bool isSafeToInline(const FunctionCall&, int inlineThreshold);

    /** Inlines any eligible functions that are found. Returns true if any changes are made. */
    bool analyze(Program& program);

private:
    using VariableRewriteMap = std::unordered_map<const Variable*, std::unique_ptr<Expression>>;

    String uniqueNameForInlineVar(const String& baseName, SymbolTable* symbolTable);

    std::unique_ptr<Expression> inlineExpression(int offset,
                                                 VariableRewriteMap* varMap,
                                                 const Expression& expression);
    std::unique_ptr<Statement> inlineStatement(int offset,
                                               VariableRewriteMap* varMap,
                                               SymbolTable* symbolTableForStatement,
                                               const Expression* resultExpr,
                                               bool haveEarlyReturns,
                                               const Statement& statement,
                                               bool isBuiltinCode);

    const Context* fContext = nullptr;
    const Program::Settings* fSettings = nullptr;
    int fInlineVarCounter = 0;
};

}  // namespace SkSL

#endif  // SKSL_INLINER
