/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_FORSTATEMENT
#define SKSL_FORSTATEMENT

#include "src/sksl/SkSLPosition.h"
#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

#include <memory>
#include <string>
#include <utility>

namespace SkSL {

class Context;
class Variable;

/**
 * The unrollability information for an ES2-compatible loop.
 */
struct LoopUnrollInfo {
    const Variable* fIndex;
    double fStart;
    double fDelta;
    int fCount;
};

/**
 * A 'for' statement.
 */
class ForStatement final : public Statement {
public:
    inline static constexpr Kind kIRNodeKind = Kind::kFor;

    ForStatement(Position pos,
                 ForLoopPositions forLoopPositions,
                 std::unique_ptr<Statement> initializer,
                 std::unique_ptr<Expression> test,
                 std::unique_ptr<Expression> next,
                 std::unique_ptr<Statement> statement,
                 std::unique_ptr<LoopUnrollInfo> unrollInfo,
                 std::unique_ptr<SymbolTable> symbols)
            : INHERITED(pos, kIRNodeKind)
            , fForLoopPositions(forLoopPositions)
            , fSymbolTable(std::move(symbols))
            , fInitializer(std::move(initializer))
            , fTest(std::move(test))
            , fNext(std::move(next))
            , fStatement(std::move(statement))
            , fUnrollInfo(std::move(unrollInfo)) {}

    // Creates an SkSL for loop; handles type-coercion and uses the ErrorReporter to report errors.
    static std::unique_ptr<Statement> Convert(const Context& context,
                                              Position pos,
                                              ForLoopPositions forLoopPositions,
                                              std::unique_ptr<Statement> initializer,
                                              std::unique_ptr<Expression> test,
                                              std::unique_ptr<Expression> next,
                                              std::unique_ptr<Statement> statement,
                                              std::unique_ptr<SymbolTable> symbolTable);

    // Creates an SkSL while loop; handles type-coercion and uses the ErrorReporter for errors.
    static std::unique_ptr<Statement> ConvertWhile(const Context& context,
                                                   Position pos,
                                                   std::unique_ptr<Expression> test,
                                                   std::unique_ptr<Statement> statement);

    // Creates an SkSL for/while loop. Assumes properly coerced types and reports errors via assert.
    static std::unique_ptr<Statement> Make(const Context& context,
                                           Position pos,
                                           ForLoopPositions forLoopPositions,
                                           std::unique_ptr<Statement> initializer,
                                           std::unique_ptr<Expression> test,
                                           std::unique_ptr<Expression> next,
                                           std::unique_ptr<Statement> statement,
                                           std::unique_ptr<LoopUnrollInfo> unrollInfo,
                                           std::unique_ptr<SymbolTable> symbolTable);

    ForLoopPositions forLoopPositions() const {
        return fForLoopPositions;
    }

    std::unique_ptr<Statement>& initializer() {
        return fInitializer;
    }

    const std::unique_ptr<Statement>& initializer() const {
        return fInitializer;
    }

    std::unique_ptr<Expression>& test() {
        return fTest;
    }

    const std::unique_ptr<Expression>& test() const {
        return fTest;
    }

    std::unique_ptr<Expression>& next() {
        return fNext;
    }

    const std::unique_ptr<Expression>& next() const {
        return fNext;
    }

    std::unique_ptr<Statement>& statement() {
        return fStatement;
    }

    const std::unique_ptr<Statement>& statement() const {
        return fStatement;
    }

    SymbolTable* symbols() const {
        return fSymbolTable.get();
    }

    /** Loop-unroll information is only supported in strict-ES2 code. Null is returned in ES3+. */
    const LoopUnrollInfo* unrollInfo() const {
        return fUnrollInfo.get();
    }

    std::string description() const override;

private:
    ForLoopPositions fForLoopPositions;
    std::unique_ptr<SymbolTable> fSymbolTable;
    std::unique_ptr<Statement> fInitializer;
    std::unique_ptr<Expression> fTest;
    std::unique_ptr<Expression> fNext;
    std::unique_ptr<Statement> fStatement;
    std::unique_ptr<LoopUnrollInfo> fUnrollInfo;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
