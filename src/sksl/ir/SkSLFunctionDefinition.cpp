/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLCore.h"
#include "src/core/SkSafeMath.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinMap.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLFunctionCall.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLInterfaceBlock.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/transform/SkSLProgramWriter.h"

#include <forward_list>

namespace SkSL {

static void append_rtadjust_fixup_to_vertex_main(const Context& context,
        const FunctionDeclaration& decl, Block& body) {
    using namespace SkSL::dsl;
    using SkSL::dsl::Swizzle;  // disambiguate from SkSL::Swizzle
    using OwnerKind = SkSL::FieldAccess::OwnerKind;

    // If this program uses RTAdjust...
    ThreadContext::RTAdjustData& rtAdjust = ThreadContext::RTAdjustState();
    if (rtAdjust.fVar || rtAdjust.fInterfaceBlock) {
        // ...append a line to the end of the function body which fixes up sk_Position.
        const Variable* skPerVertex = nullptr;
        if (const ProgramElement* perVertexDecl =
                context.fBuiltins->find(Compiler::PERVERTEX_NAME)) {
            SkASSERT(perVertexDecl->is<SkSL::InterfaceBlock>());
            skPerVertex = &perVertexDecl->as<SkSL::InterfaceBlock>().variable();
        }

        SkASSERT(skPerVertex);
        auto Ref = [](const Variable* var) -> std::unique_ptr<Expression> {
            return VariableReference::Make(/*line=*/-1, var);
        };
        auto Field = [&](const Variable* var, int idx) -> std::unique_ptr<Expression> {
            return FieldAccess::Make(context, Ref(var), idx, OwnerKind::kAnonymousInterfaceBlock);
        };
        auto Pos = [&]() -> DSLExpression {
            return DSLExpression(FieldAccess::Make(context, Ref(skPerVertex), /*fieldIndex=*/0,
                                                   OwnerKind::kAnonymousInterfaceBlock));
        };
        auto Adjust = [&]() -> DSLExpression {
            return DSLExpression(rtAdjust.fInterfaceBlock
                                         ? Field(rtAdjust.fInterfaceBlock, rtAdjust.fFieldIndex)
                                         : Ref(rtAdjust.fVar));
        };

        auto fixupStmt = DSLStatement(
            Pos() = Float4(Swizzle(Pos(), X, Y) * Swizzle(Adjust(), X, Z) +
                           Swizzle(Pos(), W, W) * Swizzle(Adjust(), Y, W),
                           0,
                           Pos().w())
        );

        body.children().push_back(fixupStmt.release());
    }
}

std::unique_ptr<FunctionDefinition> FunctionDefinition::Convert(const Context& context,
                                                                int line,
                                                                const FunctionDeclaration& function,
                                                                std::unique_ptr<Statement> body,
                                                                bool builtin) {
    class Finalizer : public ProgramWriter {
    public:
        Finalizer(const Context& context, const FunctionDeclaration& function,
                  FunctionSet* referencedBuiltinFunctions)
            : fContext(context)
            , fFunction(function)
            , fReferencedBuiltinFunctions(referencedBuiltinFunctions) {}

        ~Finalizer() override {
            SkASSERT(fBreakableLevel == 0);
            SkASSERT(fContinuableLevel == std::forward_list<int>{0});
        }

        void copyBuiltinFunctionIfNeeded(const FunctionDeclaration& function) {
            if (const ProgramElement* found =
                        fContext.fBuiltins->findAndInclude(function.description())) {
                const FunctionDefinition& original = found->as<FunctionDefinition>();

                // Sort the referenced builtin functions into a consistent order; otherwise our
                // output will become non-deterministic.
                std::vector<const FunctionDeclaration*> builtinFunctions(
                        original.referencedBuiltinFunctions().begin(),
                        original.referencedBuiltinFunctions().end());
                std::sort(builtinFunctions.begin(), builtinFunctions.end(),
                          [](const FunctionDeclaration* a, const FunctionDeclaration* b) {
                              if (a->isBuiltin() != b->isBuiltin()) {
                                  return a->isBuiltin() < b->isBuiltin();
                              }
                              if (a->fLine != b->fLine) {
                                  return a->fLine < b->fLine;
                              }
                              if (a->name() != b->name()) {
                                  return a->name() < b->name();
                              }
                              return a->description() < b->description();
                          });
                for (const FunctionDeclaration* f : builtinFunctions) {
                    this->copyBuiltinFunctionIfNeeded(*f);
                }

                ThreadContext::SharedElements().push_back(found);
            }
        }

        bool functionReturnsValue() const {
            return !fFunction.returnType().isVoid();
        }

        bool visitExpression(Expression& expr) override {
            if (expr.is<FunctionCall>()) {
                const FunctionDeclaration& func = expr.as<FunctionCall>().function();
                if (func.isBuiltin()) {
                    if (func.intrinsicKind() == k_dFdy_IntrinsicKind) {
                        ThreadContext::Inputs().fUseFlipRTUniform = true;
                    }
                    if (func.definition()) {
                        fReferencedBuiltinFunctions->insert(&func);
                    }
                    if (!fContext.fConfig->fIsBuiltinCode && fContext.fBuiltins) {
                        this->copyBuiltinFunctionIfNeeded(func);
                    }
                }

            }
            return INHERITED::visitExpression(expr);
        }

        bool visitStatement(Statement& stmt) override {
            switch (stmt.kind()) {
                case Statement::Kind::kVarDeclaration: {
                    // We count the number of slots used, but don't consider the precision of the
                    // base type. In practice, this reflects what GPUs really do pretty well.
                    // (i.e., RelaxedPrecision math doesn't mean your variable takes less space.)
                    // We also don't attempt to reclaim slots at the end of a Block.
                    size_t prevSlotsUsed = fSlotsUsed;
                    fSlotsUsed = SkSafeMath::Add(
                            fSlotsUsed, stmt.as<VarDeclaration>().var().type().slotCount());
                    // To avoid overzealous error reporting, only trigger the error at the first
                    // place where the stack limit is exceeded.
                    if (prevSlotsUsed < kVariableSlotLimit && fSlotsUsed >= kVariableSlotLimit) {
                        fContext.fErrors->error(stmt.fLine, "variable '" +
                                                            stmt.as<VarDeclaration>().var().name() +
                                                            "' exceeds the stack size limit");
                    }
                    break;
                }
                case Statement::Kind::kReturn: {
                    // Early returns from a vertex main() function will bypass sk_Position
                    // normalization, so SkASSERT that we aren't doing that. If this becomes an
                    // issue, we can add normalization before each return statement.
                    if (fContext.fConfig->fKind == ProgramKind::kVertex && fFunction.isMain()) {
                        fContext.fErrors->error(
                                stmt.fLine,
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
                            fContext.fErrors->error(returnStmt.fLine,
                                                    "may not return a value from a void function");
                        }
                    } else {
                        if (this->functionReturnsValue()) {
                            // Returning nothing from a function with a non-void return type.
                            fContext.fErrors->error(returnStmt.fLine,
                                                    "expected function to return '" +
                                                    fFunction.returnType().displayName() + "'");
                        }
                    }
                    break;
                }
                case Statement::Kind::kDo:
                case Statement::Kind::kFor: {
                    ++fBreakableLevel;
                    ++fContinuableLevel.front();
                    bool result = INHERITED::visitStatement(stmt);
                    --fContinuableLevel.front();
                    --fBreakableLevel;
                    return result;
                }
                case Statement::Kind::kSwitch: {
                    ++fBreakableLevel;
                    fContinuableLevel.push_front(0);
                    bool result = INHERITED::visitStatement(stmt);
                    fContinuableLevel.pop_front();
                    --fBreakableLevel;
                    return result;
                }
                case Statement::Kind::kBreak:
                    if (fBreakableLevel == 0) {
                        fContext.fErrors->error(stmt.fLine,
                                                "break statement must be inside a loop or switch");
                    }
                    break;
                case Statement::Kind::kContinue:
                    if (fContinuableLevel.front() == 0) {
                        if (std::any_of(fContinuableLevel.begin(),
                                        fContinuableLevel.end(),
                                        [](int level) { return level > 0; })) {
                            fContext.fErrors->error(stmt.fLine,
                                                   "continue statement cannot be used in a switch");
                        } else {
                            fContext.fErrors->error(stmt.fLine,
                                                    "continue statement must be inside a loop");
                        }
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
        // which builtin functions have we encountered in this function
        FunctionSet* fReferencedBuiltinFunctions;
        // how deeply nested we are in breakable constructs (for, do, switch).
        int fBreakableLevel = 0;
        // number of slots consumed by all variables declared in the function
        size_t fSlotsUsed = 0;
        // how deeply nested we are in continuable constructs (for, do).
        // We keep a stack (via a forward_list) in order to disallow continue inside of switch.
        std::forward_list<int> fContinuableLevel{0};

        using INHERITED = ProgramWriter;
    };

    FunctionSet referencedBuiltinFunctions;
    Finalizer(context, function, &referencedBuiltinFunctions).visitStatement(*body);
    if (function.isMain() && context.fConfig->fKind == ProgramKind::kVertex) {
        append_rtadjust_fixup_to_vertex_main(context, function, body->as<Block>());
    }

    if (Analysis::CanExitWithoutReturningValue(function, *body)) {
        context.fErrors->error(function.fLine, "function '" + function.name() +
                                               "' can exit without returning a value");
    }

    SkASSERTF(!function.isIntrinsic(),
              "Intrinsic %s should not have a definition",
              String(function.name()).c_str());
    return std::make_unique<FunctionDefinition>(line, &function, builtin, std::move(body),
                                                std::move(referencedBuiltinFunctions));
}

}  // namespace SkSL
