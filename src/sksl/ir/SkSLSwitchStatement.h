/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SWITCHSTATEMENT
#define SKSL_SWITCHSTATEMENT

#include "SkSLStatement.h"
#include "SkSLSwitchCase.h"

namespace SkSL {

/**
 * A 'switch' statement.
 */
struct SwitchStatement : public Statement {
    SwitchStatement(Position position, std::unique_ptr<Expression> value,
                    std::vector<std::unique_ptr<SwitchCase>> cases)
    : INHERITED(position, kSwitch_Kind)
    , fValue(std::move(value))
    , fCases(std::move(cases)) {}

    String description() const override {
        String result = String::printf("switch (%s) {\n", + fValue->description().c_str());
        for (const auto& c : fCases) {
            result += c->description();
        }
        result += "}";
        return result;
    }

    std::unique_ptr<Expression> fValue;
    std::vector<std::unique_ptr<SwitchCase>> fCases;

    typedef Statement INHERITED;
};

} // namespace

#endif
