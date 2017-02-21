/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
 
#ifndef SKSL_ASTSWITCHCASE
#define SKSL_ASTSWITCHCASE

#include "SkSLASTStatement.h"

namespace SkSL {

/**
 * A 'switch' statement.
 */
struct ASTSwitchCase : public ASTStatement {
    // a null value means "default:"
    ASTSwitchCase(Position position, std::unique_ptr<ASTExpression> value,
                  std::vector<std::unique_ptr<ASTStatement>> statements)
    : INHERITED(position, kSwitch_Kind)
    , fValue(std::move(value))
    , fStatements(std::move(statements)) {}

    SkString description() const override {
        SkString result;
        if (fValue) {
            result = SkStringPrintf("case %s:\n", fValue->description().c_str());
        } else {
            result = SkString("default:\n");
        }
        for (const auto& s : fStatements) {
            result += s->description() + "\n";
        }
        return result;
    }

    const std::unique_ptr<ASTExpression> fValue;
    const std::vector<std::unique_ptr<ASTStatement>> fStatements;

    typedef ASTStatement INHERITED;
};

} // namespace

#endif
