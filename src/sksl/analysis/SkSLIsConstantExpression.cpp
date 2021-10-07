/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLStatement.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/ir/SkSLDoStatement.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"

namespace SkSL {

// Checks for ES2 constant-expression rules, and (optionally) constant-index-expression rules
// (if loopIndices is non-nullptr)
class ConstantExpressionVisitor : public ProgramVisitor {
public:
    ConstantExpressionVisitor(const std::set<const Variable*>* loopIndices)
            : fLoopIndices(loopIndices) {}

    bool visitExpression(const Expression& e) override {
        // A constant-(index)-expression is one of...
        switch (e.kind()) {
            // ... a literal value
            case Expression::Kind::kLiteral:
                return false;

            // ... settings can appear in fragment processors; they will resolve when compiled
            case Expression::Kind::kSetting:
                return false;

            // ... a global or local variable qualified as 'const', excluding function parameters.
            // ... loop indices as defined in section 4. [constant-index-expression]
            case Expression::Kind::kVariableReference: {
                const Variable* v = e.as<VariableReference>().variable();
                if ((v->storage() == Variable::Storage::kGlobal ||
                     v->storage() == Variable::Storage::kLocal) &&
                    (v->modifiers().fFlags & Modifiers::kConst_Flag)) {
                    return false;
                }
                return !fLoopIndices || fLoopIndices->find(v) == fLoopIndices->end();
            }

            // ... expressions composed of both of the above
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
            case Expression::Kind::kPrefix:
            case Expression::Kind::kPostfix:
            case Expression::Kind::kSwizzle:
            case Expression::Kind::kTernary:
                return INHERITED::visitExpression(e);

            // Function calls are completely disallowed in SkSL constant-(index)-expressions.
            // GLSL does mandate that calling a built-in function where the arguments are all
            // constant-expressions should result in a constant-expression. SkSL handles this by
            // optimizing fully-constant function calls into literals in FunctionCall::Make.
            case Expression::Kind::kFunctionCall:
            case Expression::Kind::kExternalFunctionCall:
            case Expression::Kind::kChildCall:

            // These shouldn't appear in a valid program at all, and definitely aren't
            // constant-(index)-expressions.
            case Expression::Kind::kPoison:
            case Expression::Kind::kFunctionReference:
            case Expression::Kind::kExternalFunctionReference:
            case Expression::Kind::kMethodReference:
            case Expression::Kind::kTypeReference:
            case Expression::Kind::kCodeString:
                return true;

            default:
                SkDEBUGFAIL("Unexpected expression type");
                return true;
        }
    }

private:
    const std::set<const Variable*>* fLoopIndices;
    using INHERITED = ProgramVisitor;
};

bool Analysis::IsConstantExpression(const Expression& expr) {
    return !ConstantExpressionVisitor{/*loopIndices=*/nullptr}.visitExpression(expr);
}

bool Analysis::IsConstantIndexExpression(const Expression& expr,
                                         const std::set<const Variable*>* loopIndices) {
    return !ConstantExpressionVisitor{loopIndices}.visitExpression(expr);
}

}  // namespace SkSL
