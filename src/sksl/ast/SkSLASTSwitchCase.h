/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTSWITCHCASE
#define SKSL_ASTSWITCHCASE

#include "SkSLASTStatement.h"

namespace SkSL {

/**
 * A single case of a 'switch' statement.
 */
struct ASTSwitchCase : ASTPositionNode {
    // a null value means "default:"
    ASTSwitchCase(Position position, std::unique_ptr<ASTExpression> value,
                  std::vector<std::unique_ptr<ASTStatement>> statements)
    : INHERITED(position) {
        SkDebugf("ASTSwitchCase() 1\n");
        fValue = std::move(value);
        SkDebugf("ASTSwitchCase() 2\n");
        fStatements = std::move(statements);
        SkDebugf("ASTSwitchCase() 3\n");
    }

    SkString description() const override {
        SkString result;
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
    std::unique_ptr<ASTExpression> fValue;
    std::vector<std::unique_ptr<ASTStatement>> fStatements;

    typedef ASTPositionNode INHERITED;
};

} // namespace

#endif
