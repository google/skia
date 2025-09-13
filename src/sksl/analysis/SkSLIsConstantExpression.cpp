/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"
#include "src/core/SkTHash.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLIndexExpression.h"
#include "src/sksl/ir/SkSLModifierFlags.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#include <memory>

using namespace skia_private;

namespace SkSL {

class ProgramElement;

namespace {

// Checks for ES2 constant-expression rules, and (optionally) constant-index-expression rules
// (if loopIndices is non-nullptr)
class ConstantExpressionVisitor : public ProgramVisitor {
public:
    ConstantExpressionVisitor(const THashSet<const Variable*>* loopIndices)
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
                if (v->modifierFlags().isConst() && (v->storage() == Variable::Storage::kGlobal ||
                                                     v->storage() == Variable::Storage::kLocal)) {
                    return false;
                }
                return !fLoopIndices || !fLoopIndices->contains(v);
            }

            // ... not a sequence expression (skbug.com/40044392)...
            case Expression::Kind::kBinary:
                if (e.as<BinaryExpression>().getOperator().kind() == Operator::Kind::COMMA) {
                    return true;
                }
                [[fallthrough]];

            // ... expressions composed of both of the above
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
            case Expression::Kind::kChildCall:

            // These shouldn't appear in a valid program at all, and definitely aren't
            // constant-(index)-expressions.
            case Expression::Kind::kPoison:
            case Expression::Kind::kFunctionReference:
            case Expression::Kind::kMethodReference:
            case Expression::Kind::kTypeReference:
            case Expression::Kind::kEmpty:
                return true;

            default:
                SkDEBUGFAIL("Unexpected expression type");
                return true;
        }
    }

private:
    const THashSet<const Variable*>* fLoopIndices;
    using INHERITED = ProgramVisitor;
};

// Visits a function, tracks its loop indices, and verifies that every index-expression in the
// function qualifies as a constant-index-expression.
class ES2IndexingVisitor : public ProgramVisitor {
public:
    ES2IndexingVisitor(ErrorReporter& errors) : fErrors(errors) {}

    bool visitStatement(const Statement& s) override {
        if (s.is<ForStatement>()) {
            const ForStatement& f = s.as<ForStatement>();
            SkASSERT(f.initializer() && f.initializer()->is<VarDeclaration>());
            const Variable* var = f.initializer()->as<VarDeclaration>().var();
            SkASSERT(!fLoopIndices.contains(var));
            fLoopIndices.add(var);
            bool result = this->visitStatement(*f.statement());
            fLoopIndices.remove(var);
            return result;
        }
        return INHERITED::visitStatement(s);
    }

    bool visitExpression(const Expression& e) override {
        if (e.is<IndexExpression>()) {
            const IndexExpression& i = e.as<IndexExpression>();
            if (ConstantExpressionVisitor{&fLoopIndices}.visitExpression(*i.index())) {
                fErrors.error(i.fPosition, "index expression must be constant");
                return true;
            }
        }
        return INHERITED::visitExpression(e);
    }

    using ProgramVisitor::visitProgramElement;

private:
    ErrorReporter& fErrors;
    THashSet<const Variable*> fLoopIndices;
    using INHERITED = ProgramVisitor;
};

}  // namespace

bool Analysis::IsConstantExpression(const Expression& expr) {
    return !ConstantExpressionVisitor{/*loopIndices=*/nullptr}.visitExpression(expr);
}

void Analysis::ValidateIndexingForES2(const ProgramElement& pe, ErrorReporter& errors) {
    ES2IndexingVisitor visitor(errors);
    visitor.visitProgramElement(pe);
}

}  // namespace SkSL
