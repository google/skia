/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SWITCHCASE
#define SKSL_SWITCHCASE

#include "include/private/SkSLStatement.h"
#include "src/sksl/ir/SkSLExpression.h"

namespace SkSL {

/**
 * A single case of a 'switch' statement.
 */
class SwitchCase final : public Statement {
public:
    inline static constexpr Kind kStatementKind = Kind::kSwitchCase;

    // null value implies "default" case
    SwitchCase(int line, std::unique_ptr<Expression> value, std::unique_ptr<Statement> statement)
        : INHERITED(line, kStatementKind)
        , fValue(std::move(value))
        , fStatement(std::move(statement)) {}

    std::unique_ptr<Expression>& value() {
        return fValue;
    }

    const std::unique_ptr<Expression>& value() const {
        return fValue;
    }

    std::unique_ptr<Statement>& statement() {
        return fStatement;
    }

    const std::unique_ptr<Statement>& statement() const {
        return fStatement;
    }

    std::unique_ptr<Statement> clone() const override {
        return std::make_unique<SwitchCase>(fLine,
                                            this->value() ? this->value()->clone() : nullptr,
                                            this->statement()->clone());
    }

    String description() const override {
        if (this->value()) {
            return String::printf("case %s:\n%s",
                                  this->value()->description().c_str(),
                                  fStatement->description().c_str());
        } else {
            return String::printf("default:\n%s", fStatement->description().c_str());
        }
    }

private:
    std::unique_ptr<Expression> fValue;
    std::unique_ptr<Statement> fStatement;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
