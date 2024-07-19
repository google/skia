/*
 * Copyright 2022 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkSpan.h"
#include "src/sksl/SkSLDefines.h"
#include "src/sksl/SkSLModule.h"
#include "src/sksl/ir/SkSLBlock.h"
#include "src/sksl/ir/SkSLFunctionDefinition.h"
#include "src/sksl/ir/SkSLProgramElement.h"
#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/transform/SkSLProgramWriter.h"
#include "src/sksl/transform/SkSLTransform.h"

#include <algorithm>
#include <iterator>
#include <memory>
#include <vector>

namespace SkSL {

class Expression;

static void eliminate_empty_statements(SkSpan<std::unique_ptr<ProgramElement>> elements) {
    class EmptyStatementEliminator : public ProgramWriter {
    public:
        bool visitExpressionPtr(std::unique_ptr<Expression>& expr) override {
            // We don't need to look inside expressions at all.
            return false;
        }

        bool visitStatementPtr(std::unique_ptr<Statement>& stmt) override {
            // Work from the innermost blocks to the outermost.
            INHERITED::visitStatementPtr(stmt);

            if (stmt->is<Block>()) {
                StatementArray& children = stmt->as<Block>().children();
                auto iter = std::remove_if(children.begin(), children.end(),
                                           [](std::unique_ptr<Statement>& stmt) {
                                               return stmt->isEmpty();
                                           });
                children.resize(std::distance(children.begin(), iter));
            }

            // We always check the entire program.
            return false;
        }

        using INHERITED = ProgramWriter;
    };

    for (std::unique_ptr<ProgramElement>& pe : elements) {
        if (pe->is<FunctionDefinition>()) {
            EmptyStatementEliminator visitor;
            visitor.visitStatementPtr(pe->as<FunctionDefinition>().body());
        }
    }
}

void Transform::EliminateEmptyStatements(Module& module) {
    return eliminate_empty_statements(SkSpan(module.fElements));
}

}  // namespace SkSL
