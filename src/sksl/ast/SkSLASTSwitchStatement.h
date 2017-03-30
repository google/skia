/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTSWITCHSTATEMENT
#define SKSL_ASTSWITCHSTATEMENT

#include "SkSLASTStatement.h"
#include "SkSLASTSwitchCase.h"

namespace SkSL {

/**
 * A 'switch' statement.
 */
struct ASTSwitchStatement : public ASTStatement {
    ASTSwitchStatement(Position position, std::unique_ptr<ASTExpression> value,
                       std::vector<std::unique_ptr<ASTSwitchCase>> cases)
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

    const std::unique_ptr<ASTExpression> fValue;
    const std::vector<std::unique_ptr<ASTSwitchCase>> fCases;

    typedef ASTStatement INHERITED;
};

} // namespace

#endif
