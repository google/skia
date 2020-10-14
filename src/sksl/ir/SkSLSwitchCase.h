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
struct SwitchCase : public Statement {
    static constexpr Kind kStatementKind = Kind::kSwitchCase;

    SwitchCase(int offset, std::unique_ptr<Expression> value, StatementArray statements)
            : INHERITED(offset, kStatementKind)
            , fValue(std::move(value))
            , fStatements(std::move(statements)) {}

    std::unique_ptr<Statement> clone() const override {
        StatementArray cloned;
        cloned.reserve_back(fStatements.size());
        for (const auto& s : fStatements) {
            cloned.push_back(s->clone());
        }
        return std::make_unique<SwitchCase>(fOffset,
                                            fValue ? fValue->clone() : nullptr,
                                            std::move(cloned));
    }

    String description() const override {
        String result;
        if (fValue) {
            result.appendf("case %s:\n", fValue->description().c_str());
        } else {
            result += "default:\n";
        }
        for (const auto& s : fStatements) {
            result += s->description() + "\n";
        }
        return result;
    }

    // null value implies "default" case
    std::unique_ptr<Expression> fValue;
    StatementArray fStatements;

    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
