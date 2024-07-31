/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLStatement.h"

#include <algorithm>
#include <memory>

namespace SkSL {

class Expression;

static int count_returns_at_end_of_control_flow(const FunctionDefinition& funcDef) {
    class CountReturnsAtEndOfControlFlow : public ProgramVisitor {
    public:
        CountReturnsAtEndOfControlFlow(const FunctionDefinition& funcDef) {
            this->visitProgramElement(funcDef);
        }

        bool visitExpression(const Expression& expr) override {
            // Do not recurse into expressions.
            return false;
        }

        bool visitStatement(const Statement& stmt) override {
            switch (stmt.kind()) {
                case Statement::Kind::kBlock: {
                    // Check only the last statement of a block.
                    const auto& block = stmt.as<Block>();
                    return !block.children().empty() &&
                           this->visitStatement(*block.children().back());
                }
                case Statement::Kind::kSwitch:
                case Statement::Kind::kDo:
                case Statement::Kind::kFor:
                    // Don't introspect switches or loop structures at all.
                    return false;

                case Statement::Kind::kReturn:
                    ++fNumReturns;
                    [[fallthrough]];

                default:
                    return INHERITED::visitStatement(stmt);
            }
        }

        int fNumReturns = 0;
        using INHERITED = ProgramVisitor;
    };

    return CountReturnsAtEndOfControlFlow{funcDef}.fNumReturns;
}

class CountReturnsWithLimit : public ProgramVisitor {
public:
    CountReturnsWithLimit(const FunctionDefinition& funcDef, int limit) : fLimit(limit) {
        this->visitProgramElement(funcDef);
    }

    bool visitExpression(const Expression& expr) override {
        // Do not recurse into expressions.
        return false;
    }

    bool visitStatement(const Statement& stmt) override {
        switch (stmt.kind()) {
            case Statement::Kind::kReturn: {
                ++fNumReturns;
                fDeepestReturn = std::max(fDeepestReturn, fScopedBlockDepth);
                return (fNumReturns >= fLimit) || INHERITED::visitStatement(stmt);
            }
            case Statement::Kind::kVarDeclaration: {
                if (fScopedBlockDepth > 1) {
                    fVariablesInBlocks = true;
                }
                return INHERITED::visitStatement(stmt);
            }
            case Statement::Kind::kBlock: {
                int depthIncrement = stmt.as<Block>().isScope() ? 1 : 0;
                fScopedBlockDepth += depthIncrement;
                bool result = INHERITED::visitStatement(stmt);
                fScopedBlockDepth -= depthIncrement;
                if (fNumReturns == 0 && fScopedBlockDepth <= 1) {
                    // If closing this block puts us back at the top level, and we haven't
                    // encountered any return statements yet, any vardecls we may have encountered
                    // up until this point can be ignored. They are out of scope now, and they were
                    // never used in a return statement.
                    fVariablesInBlocks = false;
                }
                return result;
            }
            default:
                return INHERITED::visitStatement(stmt);
        }
    }

    int fNumReturns = 0;
    int fDeepestReturn = 0;
    int fLimit = 0;
    int fScopedBlockDepth = 0;
    bool fVariablesInBlocks = false;
    using INHERITED = ProgramVisitor;
};

Analysis::ReturnComplexity Analysis::GetReturnComplexity(const FunctionDefinition& funcDef) {
    int returnsAtEndOfControlFlow = count_returns_at_end_of_control_flow(funcDef);
    CountReturnsWithLimit counter{funcDef, returnsAtEndOfControlFlow + 1};
    if (counter.fNumReturns > returnsAtEndOfControlFlow) {
        return ReturnComplexity::kEarlyReturns;
    }
    if (counter.fNumReturns > 1) {
        return ReturnComplexity::kScopedReturns;
    }
    if (counter.fVariablesInBlocks && counter.fDeepestReturn > 1) {
        return ReturnComplexity::kScopedReturns;
    }
    return ReturnComplexity::kSingleSafeReturn;
}

}  // namespace SkSL
