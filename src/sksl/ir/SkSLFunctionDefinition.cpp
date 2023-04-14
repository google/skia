/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLFunctionDefinition.h"

#include "include/core/SkTypes.h"
#include "include/private/SkSLDefines.h"
#include "include/private/SkSLSymbol.h"
#include "include/sksl/DSLCore.h"
#include "include/sksl/DSLExpression.h"
#include "include/sksl/DSLStatement.h"
#include "include/sksl/DSLType.h"
#include "include/sksl/SkSLErrorReporter.h"
#include "src/base/SkSafeMath.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/SkSLThreadContext.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLField.h"
#include "src/sksl/ir/SkSLFieldAccess.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/transform/SkSLProgramWriter.h"

#include <algorithm>
#include <cstddef>
#include <forward_list>
#include <string_view>
#include <vector>

namespace SkSL {

static void append_rtadjust_fixup_to_vertex_main(const Context& context,
                                                 const FunctionDeclaration& decl,
                                                 Block& body) {
    using namespace SkSL::dsl;
    using SkSL::dsl::Swizzle;  // disambiguate from SkSL::Swizzle
    using OwnerKind = SkSL::FieldAccess::OwnerKind;

    // If this program uses RTAdjust...
    ThreadContext::RTAdjustData& rtAdjust = ThreadContext::RTAdjustState();
    if (rtAdjust.fVar || rtAdjust.fInterfaceBlock) {
        // ...append a line to the end of the function body which fixes up sk_Position.
        const SymbolTable* symbolTable = ThreadContext::SymbolTable().get();
        const Field& skPositionField = symbolTable->find(Compiler::POSITION_NAME)->as<Field>();

        auto Ref = [](const Variable* var) -> std::unique_ptr<Expression> {
            return VariableReference::Make(Position(), var);
        };
        auto Field = [&](const Variable* var, int idx) -> std::unique_ptr<Expression> {
            return FieldAccess::Make(context, Position(), Ref(var), idx,
                                     OwnerKind::kAnonymousInterfaceBlock);
        };
        auto Pos = [&]() -> DSLExpression {
            return DSLExpression(Field(&skPositionField.owner(), skPositionField.fieldIndex()));
        };
        auto Adjust = [&]() -> DSLExpression {
            return DSLExpression(rtAdjust.fInterfaceBlock
                                         ? Field(rtAdjust.fInterfaceBlock, rtAdjust.fFieldIndex)
                                         : Ref(rtAdjust.fVar));
        };

        auto fixupStmt = DSLStatement(
            Pos().assign(Float4(Swizzle(Pos(), X, Y) * Swizzle(Adjust(), X, Z) +
                                Swizzle(Pos(), W, W) * Swizzle(Adjust(), Y, W),
                                0,
                                Pos().w()))
        );

        body.children().push_back(fixupStmt.release());
    }
}

std::unique_ptr<FunctionDefinition> FunctionDefinition::Convert(const Context& context,
                                                                Position pos,
                                                                const FunctionDeclaration& function,
                                                                std::unique_ptr<Statement> body,
                                                                bool builtin) {
    class Finalizer : public ProgramWriter {
    public:
        Finalizer(const Context& context, const FunctionDeclaration& function, Position pos)
            : fContext(context)
            , fFunction(function) {
            // Function parameters count as local variables.
            for (const Variable* var : function.parameters()) {
                this->addLocalVariable(var, pos);
            }
        }

        void addLocalVariable(const Variable* var, Position pos) {
            // We count the number of slots used, but don't consider the precision of the base type.
            // In practice, this reflects what GPUs actually do pretty well. (i.e., RelaxedPrecision
            // math doesn't mean your variable takes less space.) We also don't attempt to reclaim
            // slots at the end of a Block.
            size_t prevSlotsUsed = fSlotsUsed;
            fSlotsUsed = SkSafeMath::Add(fSlotsUsed, var->type().slotCount());
            // To avoid overzealous error reporting, only trigger the error at the first
            // place where the stack limit is exceeded.
            if (prevSlotsUsed < kVariableSlotLimit && fSlotsUsed >= kVariableSlotLimit) {
                fContext.fErrors->error(pos, "variable '" + std::string(var->name()) +
                                             "' exceeds the stack size limit");
            }
        }

        ~Finalizer() override {
            SkASSERT(fBreakableLevel == 0);
            SkASSERT(fContinuableLevel == std::forward_list<int>{0});
        }

        bool functionReturnsValue() const {
            return !fFunction.returnType().isVoid();
        }

        bool visitExpression(Expression& expr) override {
            // We don't need to scan expressions.
            return false;
        }

        bool visitStatement(Statement& stmt) override {
            switch (stmt.kind()) {
                case Statement::Kind::kVarDeclaration: {
                    const Variable* var = stmt.as<VarDeclaration>().var();
                    if (var->type().isOrContainsUnsizedArray()) {
                        fContext.fErrors->error(stmt.fPosition,
                                                "unsized arrays are not permitted here");
                    } else {
                        this->addLocalVariable(var, stmt.fPosition);
                    }
                    break;
                }
                case Statement::Kind::kReturn: {
                    // Early returns from a vertex main() function will bypass sk_Position
                    // normalization, so SkASSERT that we aren't doing that. If this becomes an
                    // issue, we can add normalization before each return statement.
                    if (ProgramConfig::IsVertex(fContext.fConfig->fKind) && fFunction.isMain()) {
                        fContext.fErrors->error(
                                stmt.fPosition,
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
                            fContext.fErrors->error(returnStmt.expression()->fPosition,
                                                    "may not return a value from a void function");
                            returnStmt.setExpression(nullptr);
                        }
                    } else {
                        if (this->functionReturnsValue()) {
                            // Returning nothing from a function with a non-void return type.
                            fContext.fErrors->error(returnStmt.fPosition,
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
                        fContext.fErrors->error(stmt.fPosition,
                                                "break statement must be inside a loop or switch");
                    }
                    break;
                case Statement::Kind::kContinue:
                    if (fContinuableLevel.front() == 0) {
                        if (std::any_of(fContinuableLevel.begin(),
                                        fContinuableLevel.end(),
                                        [](int level) { return level > 0; })) {
                            fContext.fErrors->error(stmt.fPosition,
                                                   "continue statement cannot be used in a switch");
                        } else {
                            fContext.fErrors->error(stmt.fPosition,
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
        // how deeply nested we are in breakable constructs (for, do, switch).
        int fBreakableLevel = 0;
        // number of slots consumed by all variables declared in the function
        size_t fSlotsUsed = 0;
        // how deeply nested we are in continuable constructs (for, do).
        // We keep a stack (via a forward_list) in order to disallow continue inside of switch.
        std::forward_list<int> fContinuableLevel{0};

        using INHERITED = ProgramWriter;
    };

    Finalizer(context, function, pos).visitStatement(*body);
    if (function.isMain() && ProgramConfig::IsVertex(context.fConfig->fKind)) {
        append_rtadjust_fixup_to_vertex_main(context, function, body->as<Block>());
    }

    if (Analysis::CanExitWithoutReturningValue(function, *body)) {
        context.fErrors->error(body->fPosition, "function '" + std::string(function.name()) +
                                                "' can exit without returning a value");
    }

    SkASSERTF(!function.isIntrinsic(), "Intrinsic function '%.*s' should not have a definition",
              (int)function.name().size(), function.name().data());
    return std::make_unique<FunctionDefinition>(pos, &function, builtin, std::move(body));
}

}  // namespace SkSL
