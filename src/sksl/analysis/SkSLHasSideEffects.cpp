/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLPrefixExpression.h"

namespace SkSL {

bool Analysis::HasSideEffects(const Expression& expr) {
    class HasSideEffectsVisitor : public ProgramVisitor {
    public:
        bool visitExpression(const Expression& expr) override {
            switch (expr.kind()) {
                case Expression::Kind::kFunctionCall: {
                    const FunctionCall& call = expr.as<FunctionCall>();
                    if (!call.function().modifierFlags().isPure()) {
                        return true;
                    }
                    break;
                }
                case Expression::Kind::kPrefix: {
                    const PrefixExpression& prefix = expr.as<PrefixExpression>();
                    if (prefix.getOperator().kind() == Operator::Kind::PLUSPLUS ||
                        prefix.getOperator().kind() == Operator::Kind::MINUSMINUS) {
                        return true;
                    }
                    break;
                }
                case Expression::Kind::kBinary: {
                    const BinaryExpression& binary = expr.as<BinaryExpression>();
                    if (binary.getOperator().isAssignment()) {
                        return true;
                    }
                    break;
                }
                case Expression::Kind::kPostfix:
                    return true;

                default:
                    break;
            }
            return INHERITED::visitExpression(expr);
        }

        using INHERITED = ProgramVisitor;
    };

    HasSideEffectsVisitor visitor;
    return visitor.visitExpression(expr);
}

}  // namespace SkSL
