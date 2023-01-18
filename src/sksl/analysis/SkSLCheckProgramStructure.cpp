/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "include/private/SkSLIRNode.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "include/sksl/SkSLErrorReporter.h"
#include "include/sksl/SkSLPosition.h"
#include "src/base/SkSafeMath.h"
#include "src/core/SkTHash.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLProgram.h"

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace SkSL {

bool Analysis::CheckProgramStructure(const Program& program, bool enforceSizeLimit) {
    // We check the size of strict-ES2 programs; since SkVM will completely unroll them, it's
    // important to know how large the result will be. For non-ES2 code, we compute an approximate
    // lower bound by assuming all non-unrollable loops will execute one time only.
    const Context& context = *program.fContext;

    // If we decide that expressions are cheaper than statements, or that certain statements are
    // more expensive than others, etc., we can always tweak these ratios as needed. A very rough
    // ballpark estimate is currently good enough for our purposes.
    static constexpr size_t kExpressionCost = 1;
    static constexpr size_t kStatementCost = 1;
    static constexpr size_t kUnknownCost = -1;
    static constexpr size_t kProgramSizeLimit = 100000;
    static constexpr size_t kProgramStackDepthLimit = 50;

    class ProgramSizeVisitor : public ProgramVisitor {
    public:
        ProgramSizeVisitor(const Context& c) : fContext(c) {}

        using ProgramVisitor::visitProgramElement;

        size_t functionSize() const {
            return fFunctionSize;
        }

        bool visitProgramElement(const ProgramElement& pe) override {
            if (pe.is<FunctionDefinition>()) {
                // Check the function-size cache map first. We don't need to visit this function if
                // we already processed it before.
                const FunctionDeclaration* decl = &pe.as<FunctionDefinition>().declaration();
                if (size_t *cachedCost = fFunctionCostMap.find(decl)) {
                    // We already have this function in our map. We don't need to check it again.
                    if (*cachedCost == kUnknownCost) {
                        // If the function is present in the map with an unknown cost, we're
                        // recursively processing it--in other words, we found a cycle in the code.
                        // Unwind our stack into a string.
                        std::string msg = "\n\t" + decl->description();
                        for (auto unwind = fStack.rbegin(); unwind != fStack.rend(); ++unwind) {
                            msg = "\n\t" + (*unwind)->description() + msg;
                            if (*unwind == decl) {
                                break;
                            }
                        }
                        msg = "potential recursion (function call cycle) not allowed:" + msg;
                        fContext.fErrors->error(pe.fPosition, std::move(msg));
                        fFunctionSize = 0;
                        *cachedCost = 0;
                        return true;
                    }
                    // Set the size to its known value.
                    fFunctionSize = *cachedCost;
                    return false;
                }

                // If the function-call stack has gotten too deep, stop the analysis.
                if (fStack.size() >= kProgramStackDepthLimit) {
                    std::string msg = "exceeded max function call depth:";
                    for (auto unwind = fStack.begin(); unwind != fStack.end(); ++unwind) {
                        msg += "\n\t" + (*unwind)->description();
                    }
                    msg += "\n\t" + decl->description();
                    fContext.fErrors->error(pe.fPosition, std::move(msg));
                    fFunctionSize = 0;
                    fFunctionCostMap.set(decl, 0);
                    return true;
                }

                // Calculate the function cost and store it in our cache.
                fFunctionCostMap.set(decl, kUnknownCost);
                fStack.push_back(decl);
                fFunctionSize = 0;
                bool result = INHERITED::visitProgramElement(pe);
                fFunctionCostMap.set(decl, fFunctionSize);
                fStack.pop_back();

                return result;
            }

            return INHERITED::visitProgramElement(pe);
        }

        bool visitStatement(const Statement& stmt) override {
            switch (stmt.kind()) {
                case Statement::Kind::kFor: {
                    // We count a for-loop's unrolled size here. We expect that the init statement
                    // will be emitted once, and the test-expr, next-expr and statement will be
                    // repeated in the output for every iteration of the loop.
                    bool earlyExit = false;
                    const ForStatement& forStmt = stmt.as<ForStatement>();
                    if (forStmt.initializer() && this->visitStatement(*forStmt.initializer())) {
                        earlyExit = true;
                    }

                    size_t originalFunctionSize = fFunctionSize;
                    fFunctionSize = 0;

                    if (forStmt.next() && this->visitExpression(*forStmt.next())) {
                        earlyExit = true;
                    }
                    if (forStmt.test() && this->visitExpression(*forStmt.test())) {
                        earlyExit = true;
                    }
                    if (this->visitStatement(*forStmt.statement())) {
                        earlyExit = true;
                    }

                    // ES2 programs always have a known unroll count. Non-ES2 programs don't enforce
                    // a maximum program size, so it's fine to treat the loop as executing once.
                    if (const LoopUnrollInfo* unrollInfo = forStmt.unrollInfo()) {
                        fFunctionSize = SkSafeMath::Mul(fFunctionSize, unrollInfo->fCount);
                    }
                    fFunctionSize = SkSafeMath::Add(fFunctionSize, originalFunctionSize);
                    return earlyExit;
                }

                case Statement::Kind::kExpression:
                    // The cost of an expression-statement is counted in visitExpression. It would
                    // be double-dipping to count it here too.
                    break;

                case Statement::Kind::kNop:
                case Statement::Kind::kVarDeclaration:
                    // These statements don't directly consume any space in a compiled program.
                    break;

                default:
                    // Note that we don't make any attempt to estimate the number of iterations of
                    // do-while loops here. Those aren't an ES2 construct so we aren't enforcing
                    // program size on them.
                    fFunctionSize = SkSafeMath::Add(fFunctionSize, kStatementCost);
                    break;
            }

            return INHERITED::visitStatement(stmt);
        }

        bool visitExpression(const Expression& expr) override {
            // Other than function calls, all expressions are assumed to have a fixed unit cost.
            bool earlyExit = false;
            size_t expressionCost = kExpressionCost;

            if (expr.is<FunctionCall>()) {
                // Visit this function call to calculate its size. If we've already sized it, this
                // will retrieve the size from our cache.
                const FunctionCall& call = expr.as<FunctionCall>();
                const FunctionDeclaration* decl = &call.function();
                if (decl->definition() && !decl->isIntrinsic()) {
                    size_t originalFunctionSize = fFunctionSize;
                    fFunctionSize = 0;

                    earlyExit = this->visitProgramElement(*decl->definition());
                    expressionCost = fFunctionSize;

                    fFunctionSize = originalFunctionSize;
                }
            }

            fFunctionSize = SkSafeMath::Add(fFunctionSize, expressionCost);
            return earlyExit || INHERITED::visitExpression(expr);
        }

    private:
        using INHERITED = ProgramVisitor;

        const Context& fContext;
        size_t fFunctionSize = 0;
        SkTHashMap<const FunctionDeclaration*, size_t> fFunctionCostMap;
        std::vector<const FunctionDeclaration*> fStack;
    };

    // Process every function in our program.
    ProgramSizeVisitor visitor{context};
    for (const std::unique_ptr<ProgramElement>& element : program.fOwnedElements) {
        if (element->is<FunctionDefinition>()) {
            // Visit every function--we want to detect static recursion and report it as an error,
            // even in unreferenced functions.
            visitor.visitProgramElement(*element);
            // Report an error when main()'s flattened size is larger than our program limit.
            if (enforceSizeLimit &&
                visitor.functionSize() > kProgramSizeLimit &&
                element->as<FunctionDefinition>().declaration().isMain()) {
                context.fErrors->error(Position(), "program is too large");
            }
        }
    }

    return true;
}

}  // namespace SkSL
