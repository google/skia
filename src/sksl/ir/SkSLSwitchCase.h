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
    SwitchCase(IRGenerator* irGenerator, int offset, IRNode::ID value,
               std::vector<IRNode::ID> statements)
    : INHERITED(irGenerator, offset, kSwitch_Kind)
    , fValue(value)
    , fStatements(std::move(statements)) {}

    IRNode::ID clone() const override {
        return fIRGenerator->createNode(new SwitchCase(fIRGenerator, fOffset, fValue,
                                                       fStatements));
    }

    String description() const override {
        String result;
        if (fValue) {
            result.appendf("case %s:\n", fValue.node().description().c_str());
        } else {
            result += "default:\n";
        }
        for (const auto& s : fStatements) {
            result += s.node().description() + "\n";
        }
        return result;
    }

    // null value implies "default" case
    IRNode::ID fValue;
    std::vector<IRNode::ID> fStatements;

    typedef Statement INHERITED;
};

} // namespace

#endif
