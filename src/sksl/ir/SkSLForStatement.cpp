/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLContext.h"
#include "src/sksl/SkSLProgramSettings.h"
#include "src/sksl/ir/SkSLForStatement.h"
#include "src/sksl/ir/SkSLSymbolTable.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

std::unique_ptr<Statement> ForStatement::clone() const {
    return std::make_unique<ForStatement>(
            fOffset,
            this->initializer() ? this->initializer()->clone() : nullptr,
            this->test() ? this->test()->clone() : nullptr,
            this->next() ? this->next()->clone() : nullptr,
            this->statement()->clone(),
            SymbolTable::WrapIfBuiltin(this->symbols()));
}

String ForStatement::description() const {
    String result("for (");
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

std::unique_ptr<Statement> ForStatement::Convert(const Context& context, int offset,
                                                 std::unique_ptr<Statement> initializer,
                                                 std::unique_ptr<Expression> test,
                                                 std::unique_ptr<Expression> next,
                                                 std::unique_ptr<Statement> statement,
                                                 std::shared_ptr<SymbolTable> symbolTable) {
    if (test) {
        test = context.fTypes.fBool->coerceExpression(std::move(test), context);
        if (!test) {
            return nullptr;
        }
    }
    if (context.fConfig->strictES2Mode()) {
        if (!Analysis::ForLoopIsValidForES2(offset, initializer.get(), test.get(), next.get(),
                                            statement.get(), /*outLoopInfo=*/nullptr,
                                            &context.fErrors)) {
            return nullptr;
        }
    }

    return ForStatement::Make(context, offset, std::move(initializer), std::move(test),
                              std::move(next), std::move(statement), std::move(symbolTable));
}

std::unique_ptr<Statement> ForStatement::ConvertWhile(const Context& context, int offset,
                                                      std::unique_ptr<Expression> test,
                                                      std::unique_ptr<Statement> statement,
                                                      std::shared_ptr<SymbolTable> symbolTable) {
    if (context.fConfig->strictES2Mode()) {
        context.fErrors.error(offset, "while loops are not supported");
        return nullptr;
    }
    return ForStatement::Convert(context, offset, /*initializer=*/nullptr, std::move(test),
                                 /*next=*/nullptr, std::move(statement), std::move(symbolTable));
}

std::unique_ptr<Statement> ForStatement::Make(const Context& context, int offset,
                                              std::unique_ptr<Statement> initializer,
                                              std::unique_ptr<Expression> test,
                                              std::unique_ptr<Expression> next,
                                              std::unique_ptr<Statement> statement,
                                              std::shared_ptr<SymbolTable> symbolTable) {
    SkASSERT(!test || test->type() == *context.fTypes.fBool);
    SkASSERT(!context.fConfig->strictES2Mode() ||
             Analysis::ForLoopIsValidForES2(offset, initializer.get(), test.get(), next.get(),
                                            statement.get(), /*outLoopInfo=*/nullptr,
                                            /*errors=*/nullptr));

    return std::make_unique<ForStatement>(offset, std::move(initializer), std::move(test),
                                          std::move(next), std::move(statement),
                                          std::move(symbolTable));
}

}  // namespace SkSL
