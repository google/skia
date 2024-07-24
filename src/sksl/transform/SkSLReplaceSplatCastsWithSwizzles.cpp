/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLModule.h"
#include "src/sksl/ir/SkSLConstructorSplat.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLLiteral.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/transform/SkSLProgramWriter.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <memory>
#include <utility>
#include <vector>

namespace SkSL {

class Context;

void Transform::ReplaceSplatCastsWithSwizzles(const Context& context, Module& module) {
    class SwizzleWriter : public ProgramWriter {
    public:
        SwizzleWriter(const Context& ctx) : fContext(ctx) {}

        bool visitExpressionPtr(std::unique_ptr<Expression>& expr) override {
            if (INHERITED::visitExpressionPtr(expr)) {
                return true;
            }
            if (expr->is<ConstructorSplat>()) {
                // If the argument is a literal, only allow floats. The swizzled-literal syntax only
                // works properly for floats.
                std::unique_ptr<Expression>& arg = expr->as<ConstructorSplat>().argument();
                if (!arg->is<Literal>() || (arg->type().isFloat() && arg->type().highPrecision())) {
                    // Synthesize a splat like `.xxxx`, matching the column count of the splat.
                    ComponentArray components;
                    components.push_back_n(expr->type().columns(), SwizzleComponent::X);

                    // Replace the splat expression with the swizzle.
                    expr = Swizzle::MakeExact(fContext, expr->position(), std::move(arg),
                                              std::move(components));
                }
            }
            return false;
        }

        const Context& fContext;

        using INHERITED = ProgramWriter;
    };

    for (std::unique_ptr<ProgramElement>& pe : module.fElements) {
        if (pe->is<FunctionDefinition>()) {
            SwizzleWriter writer{context};
            writer.visitStatementPtr(pe->as<FunctionDefinition>().body());
        }
    }
}

}  // namespace SkSL
