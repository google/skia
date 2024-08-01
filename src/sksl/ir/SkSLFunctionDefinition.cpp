/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLFunctionDefinition.h"

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "src/base/SkSafeMath.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLOperator.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLBinaryExpression.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLFieldSymbol.h"
#include "src/sksl/ir/SkSLIRHelpers.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLReturnStatement.h"
#include "src/sksl/ir/SkSLSwizzle.h"
#include "src/sksl/ir/SkSLSymbol.h"
#include "src/sksl/ir/SkSLSymbolTable.h"  // IWYU pragma: keep
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"
#include "src/sksl/ir/SkSLVariableReference.h"
#include "src/sksl/transform/SkSLProgramWriter.h"

#include <algorithm>
#include <cstddef>
#include <forward_list>

namespace SkSL {

static void append_rtadjust_fixup_to_vertex_main(const Context& context,
                                                 const FunctionDeclaration& decl,
                                                 Block& body) {
    // If this program uses RTAdjust...
    if (const SkSL::Symbol* rtAdjust = context.fSymbolTable->find(Compiler::RTADJUST_NAME)) {
        // ...append a line to the end of the function body which fixes up sk_Position.
        struct AppendRTAdjustFixupHelper : public IRHelpers {
            AppendRTAdjustFixupHelper(const Context& ctx, const SkSL::Symbol* rtAdjust)
                    : IRHelpers(ctx)
                    , fRTAdjust(rtAdjust) {
                fSkPositionField = &fContext.fSymbolTable->find(Compiler::POSITION_NAME)
                                                         ->as<FieldSymbol>();
            }

            std::unique_ptr<Expression> Pos() const {
                return Field(&fSkPositionField->owner(), fSkPositionField->fieldIndex());
            }

            std::unique_ptr<Expression> Adjust() const {
                return fRTAdjust->instantiate(fContext, Position());
            }

            std::unique_ptr<Statement> makeFixupStmt() const {
                // sk_Position = float4(sk_Position.xy * rtAdjust.xz + sk_Position.ww * rtAdjust.yw,
                //                      0,
                //                      sk_Position.w);
                return Assign(
                   Pos(),
                   CtorXYZW(Add(Mul(Swizzle(Pos(),    {SwizzleComponent::X, SwizzleComponent::Y}),
                                    Swizzle(Adjust(), {SwizzleComponent::X, SwizzleComponent::Z})),
                                Mul(Swizzle(Pos(),    {SwizzleComponent::W, SwizzleComponent::W}),
                                    Swizzle(Adjust(), {SwizzleComponent::Y, SwizzleComponent::W}))),
                            Float(0.0),
                            Swizzle(Pos(), {SwizzleComponent::W})));
            }

            const FieldSymbol* fSkPositionField;
            const SkSL::Symbol* fRTAdjust;
        };

        AppendRTAdjustFixupHelper helper(context, rtAdjust);
        body.children().push_back(helper.makeFixupStmt());
    }
}

std::unique_ptr<FunctionDefinition> FunctionDefinition::Convert(const Context& context,
                                                                Position pos,
                                                                const FunctionDeclaration& function,
                                                                std::unique_ptr<Statement> body) {
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

        ~Finalizer() override {
            SkASSERT(fBreakableLevel == 0);
            SkASSERT(fContinuableLevel == std::forward_list<int>{0});
        }

        void addLocalVariable(const Variable* var, Position pos) {
            if (var->type().isOrContainsUnsizedArray()) {
                if (var->storage() != Variable::Storage::kParameter) {
                    fContext.fErrors->error(pos, "unsized arrays are not permitted here");
                }
                // Number of slots does not apply to unsized arrays since they are
                // dynamically sized.
                return;
            }
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

        void fuseVariableDeclarationsWithInitialization(std::unique_ptr<Statement>& stmt) {
            switch (stmt->kind()) {
                case Statement::Kind::kNop:
                case Statement::Kind::kBlock:
                    // Blocks and no-ops are inert; it is safe to fuse a variable declaration with
                    // its initialization across a nop or an open-brace, so we don't null out
                    // `fUninitializedVarDecl` here.
                    break;

                case Statement::Kind::kVarDeclaration:
                    // Look for variable declarations without an initializer.
                    if (VarDeclaration& decl = stmt->as<VarDeclaration>(); !decl.value()) {
                        fUninitializedVarDecl = &decl;
                        break;
                    }
                    [[fallthrough]];

                default:
                    // We found an intervening statement; it's not safe to fuse a declaration
                    // with an initializer if we encounter any other code.
                    fUninitializedVarDecl = nullptr;
                    break;

                case Statement::Kind::kExpression: {
                    // We found an expression-statement. If there was a variable declaration
                    // immediately above it, it might be possible to fuse them.
                    if (fUninitializedVarDecl) {
                        VarDeclaration* vardecl = fUninitializedVarDecl;
                        fUninitializedVarDecl = nullptr;

                        std::unique_ptr<Expression>& nextExpr = stmt->as<ExpressionStatement>()
                                                                     .expression();
                        // This statement must be a binary-expression...
                        if (!nextExpr->is<BinaryExpression>()) {
                            break;
                        }
                        // ... performing simple `var = expr` assignment...
                        BinaryExpression& binaryExpr = nextExpr->as<BinaryExpression>();
                        if (binaryExpr.getOperator().kind() != OperatorKind::EQ) {
                            break;
                        }
                        // ... directly into the variable (not a field/swizzle)...
                        Expression& leftExpr = *binaryExpr.left();
                        if (!leftExpr.is<VariableReference>()) {
                            break;
                        }
                        // ... and it must be the same variable as our vardecl.
                        VariableReference& varRef = leftExpr.as<VariableReference>();
                        if (varRef.variable() != vardecl->var()) {
                            break;
                        }
                        // The init-expression must not reference the variable.
                        // `int x; x = x = 0;` is legal SkSL, but `int x = x = 0;` is not.
                        if (Analysis::ContainsVariable(*binaryExpr.right(), *varRef.variable())) {
                            break;
                        }
                        // We found a match! Move the init-expression directly onto the vardecl, and
                        // turn the assignment into a no-op.
                        vardecl->value() = std::move(binaryExpr.right());

                        // Turn the expression-statement into a no-op.
                        stmt = Nop::Make();
                    }
                    break;
                }
            }
        }

        bool functionReturnsValue() const {
            return !fFunction.returnType().isVoid();
        }

        bool visitExpressionPtr(std::unique_ptr<Expression>& expr) override {
            // We don't need to scan expressions.
            return false;
        }

        bool visitStatementPtr(std::unique_ptr<Statement>& stmt) override {
            // When the optimizer is on, we look for variable declarations that are immediately
            // followed by an initialization expression, and fuse them into one statement.
            // (e.g.: `int i; i = 1;` can become `int i = 1;`)
            if (fContext.fConfig->fSettings.fOptimize) {
                this->fuseVariableDeclarationsWithInitialization(stmt);
            }

            // Perform error checking.
            switch (stmt->kind()) {
                case Statement::Kind::kVarDeclaration:
                    this->addLocalVariable(stmt->as<VarDeclaration>().var(), stmt->fPosition);
                    break;

                case Statement::Kind::kReturn: {
                    // Early returns from a vertex main() function will bypass sk_Position
                    // normalization, so SkASSERT that we aren't doing that. If this becomes an
                    // issue, we can add normalization before each return statement.
                    if (ProgramConfig::IsVertex(fContext.fConfig->fKind) && fFunction.isMain()) {
                        fContext.fErrors->error(
                                stmt->fPosition,
                                "early returns from vertex programs are not supported");
                    }

                    // Verify that the return statement matches the function's return type.
                    ReturnStatement& returnStmt = stmt->as<ReturnStatement>();
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
                    bool result = INHERITED::visitStatementPtr(stmt);
                    --fContinuableLevel.front();
                    --fBreakableLevel;
                    return result;
                }
                case Statement::Kind::kSwitch: {
                    ++fBreakableLevel;
                    fContinuableLevel.push_front(0);
                    bool result = INHERITED::visitStatementPtr(stmt);
                    fContinuableLevel.pop_front();
                    --fBreakableLevel;
                    return result;
                }
                case Statement::Kind::kBreak:
                    if (fBreakableLevel == 0) {
                        fContext.fErrors->error(stmt->fPosition,
                                                "break statement must be inside a loop or switch");
                    }
                    break;

                case Statement::Kind::kContinue:
                    if (fContinuableLevel.front() == 0) {
                        if (std::any_of(fContinuableLevel.begin(),
                                        fContinuableLevel.end(),
                                        [](int level) { return level > 0; })) {
                            fContext.fErrors->error(stmt->fPosition,
                                                   "continue statement cannot be used in a switch");
                        } else {
                            fContext.fErrors->error(stmt->fPosition,
                                                    "continue statement must be inside a loop");
                        }
                    }
                    break;

                default:
                    break;
            }
            return INHERITED::visitStatementPtr(stmt);
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
        // We track uninitialized variable declarations, and if they are immediately assigned-to,
        // we can move the assignment directly into the decl.
        VarDeclaration* fUninitializedVarDecl = nullptr;

        using INHERITED = ProgramWriter;
    };

    // We don't allow modules to define actual functions with intrinsic names. (Those should be
    // reserved for actual intrinsics.)
    if (function.isIntrinsic()) {
        context.fErrors->error(pos, "intrinsic function '" + std::string(function.name()) +
                                    "' should not have a definition");
        return nullptr;
    }

    // A function body must always be a braced block. (The parser should enforce this already, but
    // we rely on it, so it's best to be certain.)
    if (!body || !body->is<Block>() || !body->as<Block>().isScope()) {
        context.fErrors->error(pos, "function body '" + function.description() +
                                    "' must be a braced block");
        return nullptr;
    }

    // A function can't have more than one definition.
    if (function.definition()) {
        context.fErrors->error(pos, "function '" + function.description() +
                                    "' was already defined");
        return nullptr;
    }

    // Run the function finalizer. This checks for illegal constructs and missing return statements,
    // and also performs some simple code cleanup.
    Finalizer(context, function, pos).visitStatementPtr(body);
    if (function.isMain() && ProgramConfig::IsVertex(context.fConfig->fKind)) {
        append_rtadjust_fixup_to_vertex_main(context, function, body->as<Block>());
    }

    if (Analysis::CanExitWithoutReturningValue(function, *body)) {
        context.fErrors->error(body->fPosition, "function '" + std::string(function.name()) +
                                                "' can exit without returning a value");
    }

    return FunctionDefinition::Make(context, pos, function, std::move(body));
}

std::unique_ptr<FunctionDefinition> FunctionDefinition::Make(const Context& context,
                                                             Position pos,
                                                             const FunctionDeclaration& function,
                                                             std::unique_ptr<Statement> body) {
    SkASSERT(!function.isIntrinsic());
    SkASSERT(body && body->as<Block>().isScope());
    SkASSERT(!function.definition());

    return std::make_unique<FunctionDefinition>(pos, &function, std::move(body));
}

}  // namespace SkSL
