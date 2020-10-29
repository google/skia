/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SWITCHCASE
#define SKSL_SWITCHCASE

#include "src/sksl/ir/SkSLExpression.h"
#include "src/sksl/ir/SkSLStatement.h"

namespace SkSL {

/**
 * A single case of a 'switch' statement.
 */
class SwitchCase final : public Statement {
public:
    static constexpr Kind kStatementKind = Kind::kSwitchCase;

    // null value implies "default" case
    SwitchCase(int offset, std::unique_ptr<Expression> value, StatementArray statements)
        : INHERITED(offset, kStatementKind)
        , fValue(std::move(value))
        , fStatements(std::move(statements)) {}

    std::unique_ptr<Expression>& value() {
        return fValue;
    }

    const std::unique_ptr<Expression>& value() const {
        return fValue;
    }

    StatementArray& statements() {
        return fStatements;
    }

    const StatementArray& statements() const {
        return fStatements;
    }

    std::unique_ptr<Statement> clone() const override {
        StatementArray cloned;
        cloned.reserve_back(this->statements().size());
        for (const auto& s : this->statements()) {
            cloned.push_back(s->clone());
        }
        return std::make_unique<SwitchCase>(fOffset,
                                            this->value() ? this->value()->clone() : nullptr,
                                            std::move(cloned));
    }

    String description() const override {
        String result;
        if (this->value()) {
            result.appendf("case %s:\n", this->value()->description().c_str());
        } else {
            result += "default:\n";
        }
        for (const auto& s : this->statements()) {
            result += s->description() + "\n";
        }
        return result;
    }

private:
    std::unique_ptr<Expression> fValue;
    StatementArray fStatements;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
