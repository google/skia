/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/SkSLAnalysis.h"

#include "include/private/SkSLStatement.h"
#include "src/sksl/analysis/SkSLProgramVisitor.h"

namespace SkSL {
namespace {

class SwitchCaseContainsExit : public ProgramVisitor {
public:
    SwitchCaseContainsExit(bool conditionalExits) : fConditionalExits(conditionalExits) {}

    bool visitStatement(const Statement& stmt) override {
        switch (stmt.kind()) {
            case Statement::Kind::kBlock:
            case Statement::Kind::kSwitchCase:
                return INHERITED::visitStatement(stmt);

            case Statement::Kind::kReturn:
                // Returns are an early exit regardless of the surrounding control structures.
                return fConditionalExits ? fInConditional : !fInConditional;

            case Statement::Kind::kContinue:
                // Continues are an early exit from switches, but not loops.
                return !fInLoop &&
                       (fConditionalExits ? fInConditional : !fInConditional);

            case Statement::Kind::kBreak:
                // Breaks cannot escape from switches or loops.
                return !fInLoop && !fInSwitch &&
                       (fConditionalExits ? fInConditional : !fInConditional);

            case Statement::Kind::kIf: {
                ++fInConditional;
                bool result = INHERITED::visitStatement(stmt);
                --fInConditional;
                return result;
            }

            case Statement::Kind::kFor:
            case Statement::Kind::kDo: {
                // Loops are treated as conditionals because a loop could potentially execute zero
                // times. We don't have a straightforward way to determine that a loop definitely
                // executes at least once.
                ++fInConditional;
                ++fInLoop;
                bool result = INHERITED::visitStatement(stmt);
                --fInLoop;
                --fInConditional;
                return result;
            }

            case Statement::Kind::kSwitch: {
                ++fInSwitch;
                bool result = INHERITED::visitStatement(stmt);
                --fInSwitch;
                return result;
            }

            default:
                return false;
        }
    }

    bool fConditionalExits = false;
    int fInConditional = 0;
    int fInLoop = 0;
    int fInSwitch = 0;
    using INHERITED = ProgramVisitor;
};

}  // namespace

bool Analysis::SwitchCaseContainsUnconditionalExit(Statement& stmt) {
    return SwitchCaseContainsExit{/*conditionalExits=*/false}.visitStatement(stmt);
}

bool Analysis::SwitchCaseContainsConditionalExit(Statement& stmt) {
    return SwitchCaseContainsExit{/*conditionalExits=*/true}.visitStatement(stmt);
}

}  // namespace SkSL
