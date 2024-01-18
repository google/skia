/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/ir/SkSLForStatement.h"

#include "include/core/SkTypes.h"
#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLBuiltinTypes.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLErrorReporter.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLExpressionStatement.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLType.h"
#include "src/sksl/ir/SkSLVarDeclarations.h"
#include "src/sksl/ir/SkSLVariable.h"

namespace SkSL {

static bool is_vardecl_block_initializer(const Statement* stmt) {
    if (!stmt) {
        return false;
    }
    if (!stmt->is<SkSL::Block>()) {
        return false;
    }
    const SkSL::Block& b = stmt->as<SkSL::Block>();
    if (b.isScope()) {
        return false;
    }
    for (const auto& child : b.children()) {
        if (!child->is<SkSL::VarDeclaration>()) {
            return false;
        }
    }
    return true;
}

static bool is_simple_initializer(const Statement* stmt) {
    return !stmt || stmt->isEmpty() || stmt->is<SkSL::VarDeclaration>() ||
           stmt->is<SkSL::ExpressionStatement>();
}

std::string ForStatement::description() const {
    std::string result("for (");
    if (this->initializer()) {
        result += this->initializer()->description();
    } else {
        result += ";";
    }
    result += " ";
    if (this->test()) {
        result += this->test()->description();
    }
    result += "; ";
    if (this->next()) {
        result += this->next()->description();
    }
    result += ") " + this->statement()->description();
    return result;
}

static void hoist_vardecl_symbols_into_outer_scope(const Context& context,
                                                   const Block& initBlock,
                                                   SymbolTable* innerSymbols,
                                                   SymbolTable* hoistedSymbols) {
    class SymbolHoister : public ProgramVisitor {
    public:
        SymbolHoister(const Context& ctx, SymbolTable* innerSym, SymbolTable* hoistSym)
                : fContext(ctx)
                , fInnerSymbols(innerSym)
                , fHoistedSymbols(hoistSym) {}

        bool visitStatement(const Statement& stmt) override {
            if (stmt.is<VarDeclaration>()) {
                // Hoist the variable's symbol outside of the initializer block's symbol table, and
                // into the outer symbol table. If the initializer's symbol table originally had
                // ownership, transfer it. (If the variable was owned elsewhere, it can keep its
                // current owner.)
                Variable* var = stmt.as<VarDeclaration>().var();
                fInnerSymbols->moveSymbolTo(fHoistedSymbols, var, fContext);
                return false;
            }
            return ProgramVisitor::visitStatement(stmt);
        }

        const Context& fContext;
        SymbolTable* fInnerSymbols;
        SymbolTable* fHoistedSymbols;
    };

    SymbolHoister{context, innerSymbols, hoistedSymbols}.visitStatement(initBlock);
}

std::unique_ptr<Statement> ForStatement::Convert(const Context& context,
                                                 Position pos,
                                                 ForLoopPositions positions,
                                                 std::unique_ptr<Statement> initializer,
                                                 std::unique_ptr<Expression> test,
                                                 std::unique_ptr<Expression> next,
                                                 std::unique_ptr<Statement> statement,
                                                 std::unique_ptr<SymbolTable> symbolTable) {
    bool isSimpleInitializer = is_simple_initializer(initializer.get());
    bool isVardeclBlockInitializer = !isSimpleInitializer &&
                                     is_vardecl_block_initializer(initializer.get());

    if (!isSimpleInitializer && !isVardeclBlockInitializer) {
        context.fErrors->error(initializer->fPosition, "invalid for loop initializer");
        return nullptr;
    }

    if (test) {
        test = context.fTypes.fBool->coerceExpression(std::move(test), context);
        if (!test) {
            return nullptr;
        }
    }

    // The type of the next-expression doesn't matter, but it needs to be a complete expression.
    // Report an error on intermediate expressions like FunctionReference or TypeReference.
    if (next && next->isIncomplete(context)) {
        return nullptr;
    }

    std::unique_ptr<LoopUnrollInfo> unrollInfo;
    if (context.fConfig->strictES2Mode()) {
        // In strict-ES2, loops must be unrollable or it's an error.
        unrollInfo = Analysis::GetLoopUnrollInfo(context, pos, positions, initializer.get(), &test,
                                                 next.get(), statement.get(), context.fErrors);
        if (!unrollInfo) {
            return nullptr;
        }
    } else {
        // In ES3, loops don't have to be unrollable, but we can use the unroll information for
        // optimization purposes.
        unrollInfo = Analysis::GetLoopUnrollInfo(context, pos, positions, initializer.get(), &test,
                                                 next.get(), statement.get(), /*errors=*/nullptr);
    }

    if (Analysis::DetectVarDeclarationWithoutScope(*statement, context.fErrors)) {
        return nullptr;
    }

    if (isVardeclBlockInitializer) {
        // If the initializer statement of a for loop contains multiple variables, this causes
        // difficulties for several of our backends; e.g. Metal doesn't have a way to express arrays
        // of different size in the same decl-stmt, because the array-size is part of the type. It's
        // conceptually equivalent to synthesize a scope, declare the variables, and then emit a for
        // statement with an empty init-stmt. (Note that we can't just do this transformation
        // unilaterally for all for-statements, because the resulting for loop isn't ES2-compliant.)
        std::unique_ptr<SymbolTable> hoistedSymbols = symbolTable->insertNewParent();
        hoist_vardecl_symbols_into_outer_scope(context, initializer->as<Block>(),
                                               symbolTable.get(), hoistedSymbols.get());
        StatementArray scope;
        scope.push_back(std::move(initializer));
        scope.push_back(ForStatement::Make(context,
                                           pos,
                                           positions,
                                           /*initializer=*/nullptr,
                                           std::move(test),
                                           std::move(next),
                                           std::move(statement),
                                           std::move(unrollInfo),
                                           std::move(symbolTable)));
        return Block::Make(pos,
                           std::move(scope),
                           Block::Kind::kBracedScope,
                           std::move(hoistedSymbols));
    }

    return ForStatement::Make(context,
                              pos,
                              positions,
                              std::move(initializer),
                              std::move(test),
                              std::move(next),
                              std::move(statement),
                              std::move(unrollInfo),
                              std::move(symbolTable));
}

std::unique_ptr<Statement> ForStatement::ConvertWhile(const Context& context,
                                                      Position pos,
                                                      std::unique_ptr<Expression> test,
                                                      std::unique_ptr<Statement> statement) {
    if (context.fConfig->strictES2Mode()) {
        context.fErrors->error(pos, "while loops are not supported");
        return nullptr;
    }
    return ForStatement::Convert(context,
                                 pos,
                                 ForLoopPositions(),
                                 /*initializer=*/nullptr,
                                 std::move(test),
                                 /*next=*/nullptr,
                                 std::move(statement),
                                 /*symbolTable=*/nullptr);
}

std::unique_ptr<Statement> ForStatement::Make(const Context& context,
                                              Position pos,
                                              ForLoopPositions positions,
                                              std::unique_ptr<Statement> initializer,
                                              std::unique_ptr<Expression> test,
                                              std::unique_ptr<Expression> next,
                                              std::unique_ptr<Statement> statement,
                                              std::unique_ptr<LoopUnrollInfo> unrollInfo,
                                              std::unique_ptr<SymbolTable> symbolTable) {
    SkASSERT(is_simple_initializer(initializer.get()) ||
             is_vardecl_block_initializer(initializer.get()));
    SkASSERT(!test || test->type().matches(*context.fTypes.fBool));
    SkASSERT(!Analysis::DetectVarDeclarationWithoutScope(*statement));
    SkASSERT(unrollInfo || !context.fConfig->strictES2Mode());

    // Unrollable loops are easy to optimize because we know initializer, test and next don't have
    // interesting side effects.
    if (unrollInfo) {
        // A zero-iteration unrollable loop can be replaced with Nop.
        // An unrollable loop with an empty body can be replaced with Nop.
        if (unrollInfo->fCount <= 0 || statement->isEmpty()) {
            return Nop::Make();
        }
    }

    return std::make_unique<ForStatement>(pos,
                                          positions,
                                          std::move(initializer),
                                          std::move(test),
                                          std::move(next),
                                          std::move(statement),
                                          std::move(unrollInfo),
                                          std::move(symbolTable));
}

}  // namespace SkSL
