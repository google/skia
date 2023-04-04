/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/SkSLDefines.h"
#include "include/private/SkSLProgramElement.h"
#include "include/private/SkSLStatement.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/ir/SkSLConstructor.h"
#include "src/sksl/ir/SkSLConstructorCompound.h"
#include "src/sksl/ir/SkSLConstructorCompoundCast.h"
#include "src/sksl/ir/SkSLConstructorSplat.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionDeclaration.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"

#include <memory>
#include <vector>

namespace SkSL {
namespace {

class ReturnsInputAlphaVisitor : public ProgramVisitor {
public:
    ReturnsInputAlphaVisitor() = default;

    bool visitProgramElement(const ProgramElement& pe) override {
        const FunctionDeclaration& decl = pe.as<FunctionDefinition>().declaration();
        const std::vector<Variable*>& parameters = decl.parameters();

        // We expect a color filter to have a single half4 input.
        if (parameters.size() != 1 ||
            parameters[0]->type().columns() != 4 ||
            !parameters[0]->type().componentType().isFloat()) {
            // This doesn't look like a color filter.
            return true;
        }
        fInputVar = parameters[0];

        return INHERITED::visitProgramElement(pe);
    }

    bool isInputVar(const Expression& expr) {
        return expr.is<VariableReference>() && expr.as<VariableReference>().variable() == fInputVar;
    }

    bool isInputSwizzleEndingWithAlpha(const Expression& expr) {
        if (!expr.is<Swizzle>()) {
            return false;
        }
        const Swizzle& swizzle = expr.as<Swizzle>();
        return this->isInputVar(*swizzle.base()) && swizzle.components().back() == 3;
    }

    bool returnsInputAlpha(const Expression& expr) {
        if (this->isInputVar(expr)) {
            // This expression returns the input value as-is.
            return true;
        }
        if (expr.is<Swizzle>()) {
            // It's a swizzle: `input.___a`.
            return this->isInputSwizzleEndingWithAlpha(expr);
        }
        if (expr.is<ConstructorSplat>() || expr.is<ConstructorCompound>()) {
            // This is a splat or compound constructor with `input.a` as its final component.
            const AnyConstructor& ctor = expr.asAnyConstructor();
            return this->returnsInputAlpha(*ctor.argumentSpan().back());
        }
        if (expr.is<ConstructorCompoundCast>()) {
            // Ignore typecasts between float and half.
            const Expression& arg = *expr.as<ConstructorCompoundCast>().argument();
            return arg.type().componentType().isFloat() && this->returnsInputAlpha(arg);
        }
        // We weren't able to pattern-match here.
        return false;
    }

    bool visitStatement(const Statement& s) override {
        if (s.is<ReturnStatement>()) {
            return !this->returnsInputAlpha(*s.as<ReturnStatement>().expression());
        }
        return INHERITED::visitStatement(s);
    }

    bool visitExpression(const Expression& e) override {
        // No need to recurse into expressions; these can never contain return statements.
        return false;
    }

private:
    const Variable* fInputVar;

    using INHERITED = ProgramVisitor;
};

}  // namespace

bool Analysis::ReturnsInputAlpha(const FunctionDefinition& function) {
    ReturnsInputAlphaVisitor visitor;
    return !visitor.visitProgramElement(function);
}

}  // namespace SkSL
