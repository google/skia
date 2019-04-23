/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTSWITCHCASE
#define SKSL_ASTSWITCHCASE

#include "src/sksl/ast/SkSLASTStatement.h"

namespace SkSL {

/**
 * A single case of a 'switch' statement.
 */
struct ASTSwitchCase : public ASTStatement {
    // a null value means "default:"
    ASTSwitchCase(int offset, std::unique_ptr<ASTExpression> value,
                  std::vector<std::unique_ptr<ASTStatement>> statements)
    : INHERITED(offset, kSwitch_Kind)
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
    const std::unique_ptr<ASTExpression> fValue;
    const std::vector<std::unique_ptr<ASTStatement>> fStatements;

    typedef ASTStatement INHERITED;
};

} // namespace

#endif
