/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLReturnStatement.h"

namespace SkSL {

std::unique_ptr<FunctionDefinition> FunctionDefinition::Convert(const Context& context,
                                                                int offset,
                                                                const FunctionDeclaration& function,
                                                                std::unique_ptr<Statement> body,
                                                                bool builtin) {
    class Finalizer : public ProgramWriter {
    public:
        Finalizer(const Context& context, const FunctionDeclaration& function,
                  IntrinsicSet* referencedIntrinsics)
            : fContext(context)
            , fFunction(function)
            , fReferencedIntrinsics(referencedIntrinsics) {}

        ~Finalizer() override {
            SkASSERT(!fBreakableLevel);
            SkASSERT(!fContinuableLevel);
        }

        bool functionReturnsValue() const {
            return !fFunction.returnType().isVoid();
        }

        bool visitExpression(Expression& expr) override {
            if (expr.is<FunctionCall>()) {
                const FunctionDeclaration& func = expr.as<FunctionCall>().function();
                if (func.isBuiltin() && func.definition()) {
                    fReferencedIntrinsics->insert(&func);
                }
            }
            return INHERITED::visitExpression(expr);
        }

        bool visitStatement(Statement& stmt) override {
            switch (stmt.kind()) {
                case Statement::Kind::kReturn: {
                    // Early returns from a vertex main() function will bypass sk_Position
                    // normalization, so SkASSERT that we aren't doing that. If this becomes an
                    // issue, we can add normalization before each return statement.
                    if (fContext.fConfig->fKind == ProgramKind::kVertex && fFunction.isMain()) {
                        fContext.fErrors->error(
                                stmt.fOffset,
                                "early returns from vertex programs are not supported");
                    }

                    // Verify that the return statement matches the function's return type.
                    ReturnStatement& returnStmt = stmt.as<ReturnStatement>();
                    if (returnStmt.expression()) {
                        if (this->functionReturnsValue()) {
                            // Coerce return expression to the function's return type.
                            returnStmt.setExpression(fFunction.returnType().coerceExpression(
                                    std::move(returnStmt.expression()), fContext));
                        } else {
                            // Returning something from a function with a void return type.
                            returnStmt.setExpression(nullptr);
                            fContext.fErrors->error(returnStmt.fOffset,
                                                    "may not return a value from a void function");
                        }
                    } else {
                        if (this->functionReturnsValue()) {
                            // Returning nothing from a function with a non-void return type.
                            fContext.fErrors->error(returnStmt.fOffset,
                                                    "expected function to return '" +
                                                    fFunction.returnType().displayName() + "'");
                        }
                    }
                    break;
                }
                case Statement::Kind::kDo:
                case Statement::Kind::kFor: {
                    ++fBreakableLevel;
                    ++fContinuableLevel;
                    bool result = INHERITED::visitStatement(stmt);
                    --fContinuableLevel;
                    --fBreakableLevel;
                    return result;
                }
                case Statement::Kind::kSwitch: {
                    ++fBreakableLevel;
                    bool result = INHERITED::visitStatement(stmt);
                    --fBreakableLevel;
                    return result;
                }
                case Statement::Kind::kBreak:
                    if (!fBreakableLevel) {
                        fContext.fErrors->error(stmt.fOffset,
                                                "break statement must be inside a loop or switch");
                    }
                    break;
                case Statement::Kind::kContinue:
                    if (!fContinuableLevel) {
                        fContext.fErrors->error(stmt.fOffset,
                                                "continue statement must be inside a loop");
                    }
                    break;
                default:
                    break;
            }
            return INHERITED::visitStatement(stmt);
        }

    private:
        const Context& fContext;
        const FunctionDeclaration& fFunction;
        // which intrinsics have we encountered in this function
        IntrinsicSet* fReferencedIntrinsics;
        // how deeply nested we are in breakable constructs (for, do, switch).
        int fBreakableLevel = 0;
        // how deeply nested we are in continuable constructs (for, do).
        int fContinuableLevel = 0;

        using INHERITED = ProgramWriter;
    };

    IntrinsicSet referencedIntrinsics;
    Finalizer(context, function, &referencedIntrinsics).visitStatement(*body);

    if (Analysis::CanExitWithoutReturningValue(function, *body)) {
        context.fErrors->error(function.fOffset, "function '" + function.name() +
                                                 "' can exit without returning a value");
    }

    return std::make_unique<FunctionDefinition>(offset, &function, builtin, std::move(body),
                                                std::move(referencedIntrinsics));
}

}  // namespace SkSL
