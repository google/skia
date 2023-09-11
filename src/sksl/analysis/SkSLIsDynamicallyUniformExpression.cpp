/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"

namespace SkSL {

bool Analysis::IsDynamicallyUniformExpression(const Expression& expr) {
    class IsDynamicallyUniformExpressionVisitor : public ProgramVisitor {
    public:
        bool visitExpression(const Expression& expr) override {
            switch (expr.kind()) {
                case Expression::Kind::kBinary:
                case Expression::Kind::kConstructorArray:
                case Expression::Kind::kConstructorArrayCast:
                case Expression::Kind::kConstructorCompound:
                case Expression::Kind::kConstructorCompoundCast:
                case Expression::Kind::kConstructorDiagonalMatrix:
                case Expression::Kind::kConstructorMatrixResize:
                case Expression::Kind::kConstructorScalarCast:
                case Expression::Kind::kConstructorSplat:
                case Expression::Kind::kConstructorStruct:
                case Expression::Kind::kFieldAccess:
                case Expression::Kind::kIndex:
                case Expression::Kind::kPostfix:
                case Expression::Kind::kPrefix:
                case Expression::Kind::kSwizzle:
                case Expression::Kind::kTernary:
                    // These expressions might be dynamically uniform, if they are composed entirely
                    // of constants and uniforms.
                    break;

                case Expression::Kind::kVariableReference: {
                    // Verify that variable references are const or uniform.
                    const Variable* var = expr.as<VariableReference>().variable();
                    if (var && (var->modifierFlags().isConst() ||
                                var->modifierFlags().isUniform())) {
                        break;
                    }
                    fIsDynamicallyUniform = false;
                    return true;
                }
                case Expression::Kind::kFunctionCall: {
                    // Verify that function calls are pure.
                    const FunctionDeclaration& decl = expr.as<FunctionCall>().function();
                    if (decl.modifierFlags().isPure()) {
                        break;
                    }
                    fIsDynamicallyUniform = false;
                    return true;
                }
                case Expression::Kind::kLiteral:
                    // Literals are compile-time constants.
                    return false;

                default:
                    // This expression isn't dynamically uniform.
                    fIsDynamicallyUniform = false;
                    return true;
            }
            return INHERITED::visitExpression(expr);
        }

        bool fIsDynamicallyUniform = true;
        using INHERITED = ProgramVisitor;
    };

    IsDynamicallyUniformExpressionVisitor visitor;
    visitor.visitExpression(expr);
    return visitor.fIsDynamicallyUniform;
}

}  // namespace SkSL
