/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SWITCHCASE
#define SKSL_SWITCHCASE

#include "SkSLExpression.h"
#include "SkSLStatement.h"

namespace SkSL {

/**
 * A single case of a 'switch' statement.
 */
struct SwitchCase : public Statement {
    SwitchCase(Position position, std::unique_ptr<Expression> value,
               std::vector<std::unique_ptr<Statement>> statements)
    : INHERITED(position, kSwitch_Kind)
    , fValue(std::move(value))
    , fStatements(std::move(statements)) {}

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
    std::vector<std::unique_ptr<Statement>> fStatements;

    typedef Statement INHERITED;
};

} // namespace

#endif
