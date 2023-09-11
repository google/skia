/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTArray.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/analysis/SkSLProgramUsage.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSwitchCase.h"
#include "src/sksl/ir/SkSLSwitchStatement.h"
#include "src/sksl/transform/SkSLProgramWriter.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <memory>
#include <vector>

using namespace skia_private;

namespace SkSL {

class Expression;

static void eliminate_unreachable_code(SkSpan<std::unique_ptr<ProgramElement>> elements,
                                       ProgramUsage* usage) {
    class UnreachableCodeEliminator : public ProgramWriter {
    public:
        UnreachableCodeEliminator(ProgramUsage* usage) : fUsage(usage) {
            fFoundFunctionExit.push_back(false);
            fFoundBlockExit.push_back(false);
        }

        bool visitExpressionPtr(std::unique_ptr<Expression>& expr) override {
            // We don't need to look inside expressions at all.
            return false;
        }

        bool visitStatementPtr(std::unique_ptr<Statement>& stmt) override {
            if (fFoundFunctionExit.back() || fFoundBlockExit.back()) {
                // If we already found an exit in this section, anything beyond it is dead code.
                if (!stmt->is<Nop>()) {
                    // Eliminate the dead statement by substituting a Nop.
                    fUsage->remove(stmt.get());
                    stmt = Nop::Make();
                }
                return false;
            }

            switch (stmt->kind()) {
                case Statement::Kind::kReturn:
                case Statement::Kind::kDiscard:
                    // We found a function exit on this path.
                    fFoundFunctionExit.back() = true;
                    break;

                case Statement::Kind::kBreak:
                    // A `break` statement can either be breaking out of a loop or terminating an
                    // individual switch case. We treat both cases the same way: they only apply
                    // to the statements associated with the parent statement (i.e. enclosing loop
                    // block / preceding case label).
                case Statement::Kind::kContinue:
                    fFoundBlockExit.back() = true;
                    break;

                case Statement::Kind::kExpression:
                case Statement::Kind::kNop:
                case Statement::Kind::kVarDeclaration:
                    // These statements don't affect control flow.
                    break;

                case Statement::Kind::kBlock:
                    // Blocks are on the straight-line path and don't affect control flow.
                    return INHERITED::visitStatementPtr(stmt);

                case Statement::Kind::kDo: {
                    // Function-exits are allowed to propagate outside of a do-loop, because it
                    // always executes its body at least once.
                    fFoundBlockExit.push_back(false);
                    bool result = INHERITED::visitStatementPtr(stmt);
                    fFoundBlockExit.pop_back();
                    return result;
                }
                case Statement::Kind::kFor: {
                    // Function-exits are not allowed to propagate out, because a for-loop or while-
                    // loop could potentially run zero times.
                    fFoundFunctionExit.push_back(false);
                    fFoundBlockExit.push_back(false);
                    bool result = INHERITED::visitStatementPtr(stmt);
                    fFoundBlockExit.pop_back();
                    fFoundFunctionExit.pop_back();
                    return result;
                }
                case Statement::Kind::kIf: {
                    // This statement is conditional and encloses two inner sections of code.
                    // If both sides contain a function-exit or loop-exit, that exit is allowed to
                    // propagate out.
                    IfStatement& ifStmt = stmt->as<IfStatement>();

                    fFoundFunctionExit.push_back(false);
                    fFoundBlockExit.push_back(false);
                    bool result = (ifStmt.ifTrue() && this->visitStatementPtr(ifStmt.ifTrue()));
                    bool foundFunctionExitOnTrue = fFoundFunctionExit.back();
                    bool foundLoopExitOnTrue = fFoundBlockExit.back();
                    fFoundFunctionExit.pop_back();
                    fFoundBlockExit.pop_back();

                    fFoundFunctionExit.push_back(false);
                    fFoundBlockExit.push_back(false);
                    result |= (ifStmt.ifFalse() && this->visitStatementPtr(ifStmt.ifFalse()));
                    bool foundFunctionExitOnFalse = fFoundFunctionExit.back();
                    bool foundLoopExitOnFalse = fFoundBlockExit.back();
                    fFoundFunctionExit.pop_back();
                    fFoundBlockExit.pop_back();

                    fFoundFunctionExit.back() |= foundFunctionExitOnTrue &&
                                                 foundFunctionExitOnFalse;
                    fFoundBlockExit.back() |= foundLoopExitOnTrue &&
                                              foundLoopExitOnFalse;
                    return result;
                }
                case Statement::Kind::kSwitch: {
                    // In switch statements we consider unreachable code on a per-case basis.
                    SwitchStatement& sw = stmt->as<SwitchStatement>();
                    bool result = false;

                    // Tracks whether we found at least one case that doesn't lead to a return
                    // statement (potentially via fallthrough).
                    bool foundCaseWithoutReturn = false;
                    bool hasDefault = false;
                    for (std::unique_ptr<Statement>& c : sw.cases()) {
                        // We eliminate unreachable code within the statements of the individual
                        // case. Breaks are not allowed to propagate outside the case statement
                        // itself. Function returns are allowed to propagate out only if all cases
                        // have a return AND one of the cases is default (so that we know at least
                        // one of the branches will be taken). This is similar to how we handle if
                        // statements above.
                        fFoundFunctionExit.push_back(false);
                        fFoundBlockExit.push_back(false);

                        SwitchCase& sc = c->as<SwitchCase>();
                        result |= this->visitStatementPtr(sc.statement());

                        // When considering whether a case has a return we can propagate, we
                        // assume the following:
                        //     1. The default case is always placed last in a switch statement and
                        //        it is the last possible label reachable via fallthrough. Thus if
                        //        it does not contain a return statement, then we don't propagate a
                        //        function return.
                        //     2. In all other cases we prevent the return from propagating only if
                        //        we encounter a break statement. If no return or break is found,
                        //        we defer the decision to the fallthrough case. We won't propagate
                        //        a return unless we eventually encounter a default label.
                        //
                        // See resources/sksl/shared/SwitchWithEarlyReturn.sksl for test cases that
                        // exercise this.
                        if (sc.isDefault()) {
                            foundCaseWithoutReturn |= !fFoundFunctionExit.back();
                            hasDefault = true;
                        } else {
                            // We can only be sure that a case does not lead to a return if it
                            // doesn't fallthrough.
                            foundCaseWithoutReturn |=
                                    (!fFoundFunctionExit.back() && fFoundBlockExit.back());
                        }

                        fFoundFunctionExit.pop_back();
                        fFoundBlockExit.pop_back();
                    }

                    fFoundFunctionExit.back() |= !foundCaseWithoutReturn && hasDefault;
                    return result;
                }
                case Statement::Kind::kSwitchCase:
                    // We should never hit this case as switch cases are handled in the previous
                    // case.
                    SkUNREACHABLE;
            }

            return false;
        }

        ProgramUsage* fUsage;
        STArray<32, bool> fFoundFunctionExit;
        STArray<32, bool> fFoundBlockExit;

        using INHERITED = ProgramWriter;
    };

    for (std::unique_ptr<ProgramElement>& pe : elements) {
        if (pe->is<FunctionDefinition>()) {
            UnreachableCodeEliminator visitor{usage};
            visitor.visitStatementPtr(pe->as<FunctionDefinition>().body());
        }
    }
}

void Transform::EliminateUnreachableCode(Module& module, ProgramUsage* usage) {
    return eliminate_unreachable_code(SkSpan(module.fElements), usage);
}

void Transform::EliminateUnreachableCode(Program& program) {
    return eliminate_unreachable_code(SkSpan(program.fOwnedElements), program.fUsage.get());
}

}  // namespace SkSL
