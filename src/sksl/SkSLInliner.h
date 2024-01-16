/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_INLINER
#define SKSL_INLINER

#ifndef SK_ENABLE_OPTIMIZE_SIZE

#include "src/core/SkTHash.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLMangler.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLExpression.h"

#include <memory>
#include <vector>

namespace SkSL {

class FunctionCall;
class FunctionDeclaration;
class FunctionDefinition;
class Position;
class ProgramElement;
class ProgramUsage;
class Statement;
class SymbolTable;
class Variable;
struct InlineCandidate;
struct InlineCandidateList;
namespace Analysis { enum class ReturnComplexity; }

/**
 * Converts a FunctionCall in the IR to a set of statements to be injected ahead of the function
 * call, and a replacement expression. Can also detect cases where inlining isn't cleanly possible
 * (e.g. return statements nested inside of a loop construct). The inliner isn't able to guarantee
 * identical-to-GLSL execution order if the inlined function has visible side effects.
 */
class Inliner {
public:
    Inliner(const Context* context) : fContext(context) {}

    /** Inlines any eligible functions that are found. Returns true if any changes are made. */
    bool analyze(const std::vector<std::unique_ptr<ProgramElement>>& elements,
                 SymbolTable* symbols,
                 ProgramUsage* usage);

private:
    using VariableRewriteMap = skia_private::THashMap<const Variable*, std::unique_ptr<Expression>>;

    const ProgramSettings& settings() const { return fContext->fConfig->fSettings; }

    void buildCandidateList(const std::vector<std::unique_ptr<ProgramElement>>& elements,
                            SymbolTable* symbols,
                            ProgramUsage* usage,
                            InlineCandidateList* candidateList);

    std::unique_ptr<Expression> inlineExpression(Position pos,
                                                 VariableRewriteMap* varMap,
                                                 SymbolTable* symbolTableForExpression,
                                                 const Expression& expression);
    std::unique_ptr<Statement> inlineStatement(Position pos,
                                               VariableRewriteMap* varMap,
                                               SymbolTable* symbolTableForStatement,
                                               std::unique_ptr<Expression>* resultExpr,
                                               Analysis::ReturnComplexity returnComplexity,
                                               const Statement& statement,
                                               const ProgramUsage& usage,
                                               bool isBuiltinCode);

    /**
     * Searches the rewrite map for an rewritten Variable* for the passed-in one. Asserts if the
     * rewrite map doesn't contain the variable, or contains a different type of expression.
     */
    static const Variable* RemapVariable(const Variable* variable,
                                         const VariableRewriteMap* varMap);

    using InlinabilityCache = skia_private::THashMap<const FunctionDeclaration*, bool>;
    bool candidateCanBeInlined(const InlineCandidate& candidate,
                               const ProgramUsage& usage,
                               InlinabilityCache* cache);

    bool functionCanBeInlined(const FunctionDeclaration& funcDecl,
                              const ProgramUsage& usage,
                              InlinabilityCache* cache);

    using FunctionSizeCache = skia_private::THashMap<const FunctionDeclaration*, int>;
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
    InlinedCall inlineCall(const FunctionCall&,
                           SymbolTable*,
                           const ProgramUsage&,
                           const FunctionDeclaration* caller);

    /** Adds a scope to inlined bodies returned by `inlineCall`, if one is required. */
    void ensureScopedBlocks(Statement* inlinedBody, Statement* parentStmt);

    /** Checks whether inlining is viable for a FunctionCall, modulo recursion and function size. */
    bool isSafeToInline(const FunctionDefinition* functionDef, const ProgramUsage& usage);

    const Context* fContext = nullptr;
    Mangler fMangler;
    int fInlinedStatementCounter = 0;
};

}  // namespace SkSL

#endif  // SK_ENABLE_OPTIMIZE_SIZE

#endif  // SKSL_INLINER
