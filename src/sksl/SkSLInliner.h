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

#include "src/sksl/SkSLMangler.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

class Block;
class Context;
class Expression;
class FunctionCall;
class FunctionDefinition;
struct InlineCandidate;
struct InlineCandidateList;
class ModifiersPool;
class Statement;
class SymbolTable;
class Variable;

/**
 * Converts a FunctionCall in the IR to a set of statements to be injected ahead of the function
 * call, and a replacement expression. Can also detect cases where inlining isn't cleanly possible
 * (e.g. return statements nested inside of a loop construct). The inliner isn't able to guarantee
 * identical-to-GLSL execution order if the inlined function has visible side effects.
 */
class Inliner {
public:
    Inliner(const Context* context) : fContext(context) {}

    void reset();

    /** Inlines any eligible functions that are found. Returns true if any changes are made. */
    bool analyze(const std::vector<std::unique_ptr<ProgramElement>>& elements,
                 std::shared_ptr<SymbolTable> symbols,
                 ProgramUsage* usage);

private:
    using VariableRewriteMap = std::unordered_map<const Variable*, std::unique_ptr<Expression>>;

    enum class ReturnComplexity {
        kSingleSafeReturn,
        kScopedReturns,
        kEarlyReturns,
    };

    const Program::Settings& settings() const { return fContext->fConfig->fSettings; }

    void buildCandidateList(const std::vector<std::unique_ptr<ProgramElement>>& elements,
                            std::shared_ptr<SymbolTable> symbols, ProgramUsage* usage,
                            InlineCandidateList* candidateList);

    std::unique_ptr<Expression> inlineExpression(int line,
                                                 VariableRewriteMap* varMap,
                                                 SymbolTable* symbolTableForExpression,
                                                 const Expression& expression);
    std::unique_ptr<Statement> inlineStatement(int line,
                                               VariableRewriteMap* varMap,
                                               SymbolTable* symbolTableForStatement,
                                               std::unique_ptr<Expression>* resultExpr,
                                               ReturnComplexity returnComplexity,
                                               const Statement& statement,
                                               bool isBuiltinCode);

    /** Determines if a given function has multiple and/or early returns. */
    static ReturnComplexity GetReturnComplexity(const FunctionDefinition& funcDef);

    using InlinabilityCache = std::unordered_map<const FunctionDeclaration*, bool>;
    bool candidateCanBeInlined(const InlineCandidate& candidate, InlinabilityCache* cache);

    using FunctionSizeCache = std::unordered_map<const FunctionDeclaration*, int>;
    int getFunctionSize(const FunctionDeclaration& fnDecl, FunctionSizeCache* cache);

    /**
     * Processes the passed-in FunctionCall expression. The FunctionCall expression should be
     * replaced with `fReplacementExpr`. If non-null, `fInlinedBody` should be inserted immediately
     * above the statement containing the inlined expression.
     */
    struct InlinedCall {
        std::unique_ptr<Block> fInlinedBody;
        std::unique_ptr<Expression> fReplacementExpr;
    };
    InlinedCall inlineCall(FunctionCall*,
                           std::shared_ptr<SymbolTable>,
                           const ProgramUsage&,
                           const FunctionDeclaration* caller);

    /** Adds a scope to inlined bodies returned by `inlineCall`, if one is required. */
    void ensureScopedBlocks(Statement* inlinedBody, Statement* parentStmt);

    /** Checks whether inlining is viable for a FunctionCall, modulo recursion and function size. */
    bool isSafeToInline(const FunctionDefinition* functionDef);

    const Context* fContext = nullptr;
    int fInlinedStatementCounter = 0;
};

}  // namespace SkSL

#endif  // SKSL_INLINER
