/*
 * Copyright 2021 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkSLStatement.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLIfStatement.h"
#include "src/sksl/ir/SkSLNop.h"
#include "src/sksl/ir/SkSLProgram.h"
#include "src/sksl/transform/SkSLProgramWriter.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <stack>

namespace SkSL {

void Transform::EliminateUnreachableCode(Program& program, ProgramUsage* usage) {
    class UnreachableCodeEliminator : public ProgramWriter {
    public:
        UnreachableCodeEliminator(ProgramUsage* usage)
                : fUsage(usage) {
            fFoundFunctionExit.push(false);
            fFoundLoopExit.push(false);
        }

        bool visitExpressionPtr(std::unique_ptr<Expression>& expr) override {
            // We don't need to look inside expressions at all.
            return false;
        }

        bool visitStatementPtr(std::unique_ptr<Statement>& stmt) override {
            if (fFoundFunctionExit.top() || fFoundLoopExit.top()) {
                // If we already found an exit in this section, anything beyond it is dead code.
                if (!stmt->is<Nop>()) {
                    // Eliminate the dead statement by substituting a Nop.
                    if (fUsage) {
                        fUsage->remove(stmt.get());
                    }
                    stmt = Nop::Make();
                }
                return false;
            }

            switch (stmt->kind()) {
                case Statement::Kind::kReturn:
                case Statement::Kind::kDiscard:
                    // We found a function exit on this path.
                    fFoundFunctionExit.top() = true;
                    break;

                case Statement::Kind::kBreak:
                case Statement::Kind::kContinue:
                    // We found a loop exit on this path. Note that we skip over switch statements
                    // completely when eliminating code, so any `break` statement would be breaking
                    // out of a loop, not out of a switch.
                    fFoundLoopExit.top() = true;
                    break;

                case Statement::Kind::kExpression:
                case Statement::Kind::kInlineMarker:
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
                    fFoundLoopExit.push(false);
                    bool result = INHERITED::visitStatementPtr(stmt);
                    fFoundLoopExit.pop();
                    return result;
                }
                case Statement::Kind::kFor: {
                    // Function-exits are not allowed to propagate out, because a for-loop or while-
                    // loop could potentially run zero times.
                    fFoundFunctionExit.push(false);
                    fFoundLoopExit.push(false);
                    bool result = INHERITED::visitStatementPtr(stmt);
                    fFoundLoopExit.pop();
                    fFoundFunctionExit.pop();
                    return result;
                }
                case Statement::Kind::kIf: {
                    // This statement is conditional and encloses two inner sections of code.
                    // If both sides contain a function-exit or loop-exit, that exit is allowed to
                    // propagate out.
                    IfStatement& ifStmt = stmt->as<IfStatement>();

                    fFoundFunctionExit.push(false);
                    fFoundLoopExit.push(false);
                    bool result = (ifStmt.ifTrue() && this->visitStatementPtr(ifStmt.ifTrue()));
                    bool foundFunctionExitOnTrue = fFoundFunctionExit.top();
                    bool foundLoopExitOnTrue = fFoundLoopExit.top();
                    fFoundFunctionExit.pop();
                    fFoundLoopExit.pop();

                    fFoundFunctionExit.push(false);
                    fFoundLoopExit.push(false);
                    result |= (ifStmt.ifFalse() && this->visitStatementPtr(ifStmt.ifFalse()));
                    bool foundFunctionExitOnFalse = fFoundFunctionExit.top();
                    bool foundLoopExitOnFalse = fFoundLoopExit.top();
                    fFoundFunctionExit.pop();
                    fFoundLoopExit.pop();

                    fFoundFunctionExit.top() |= foundFunctionExitOnTrue && foundFunctionExitOnFalse;
                    fFoundLoopExit.top() |= foundLoopExitOnTrue && foundLoopExitOnFalse;
                    return result;
                }
                case Statement::Kind::kSwitch:
                case Statement::Kind::kSwitchCase:
                    // We skip past switch statements entirely when scanning for dead code. Their
                    // control flow is quite complex and we already do a good job of flattening out
                    // switches on constant values.
                    break;
            }

            return false;
        }

        ProgramUsage* fUsage;
        std::stack<bool> fFoundFunctionExit;
        std::stack<bool> fFoundLoopExit;

        using INHERITED = ProgramWriter;
    };

    for (std::unique_ptr<ProgramElement>& pe : program.fOwnedElements) {
        if (pe->is<FunctionDefinition>()) {
            UnreachableCodeEliminator visitor{usage};
            visitor.visitStatementPtr(pe->as<FunctionDefinition>().body());
        }
    }
}

}  // namespace SkSL
