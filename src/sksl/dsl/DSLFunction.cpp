/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/DSLFunction.h"

#include "src/sksl/SkSLAnalysis.h"
#include "src/sksl/SkSLCompiler.h"
#include "src/sksl/SkSLIRGenerator.h"
#include "src/sksl/ir/SkSLReturnStatement.h"

namespace SkSL {

namespace dsl {

void DSLFunction::define(DSLBlock block) {
    SkASSERT(fDecl);
    SkASSERT(!DSLWriter::CurrentFunction());
    DSLWriter::SetCurrentFunction(fDecl);
    class FinalizeReturns : public ProgramWriter {
    public:
        bool visitStatement(Statement& stmt) override {
            if (stmt.is<ReturnStatement>()) {
                ReturnStatement& r = stmt.as<ReturnStatement>();
                std::unique_ptr<Statement> finished = DSLWriter::IRGenerator().convertReturn(
                                                                         r.fOffset,
                                                                         std::move(r.expression()));
                if (finished) {
                    r.setExpression(std::move(finished->as<ReturnStatement>().expression()));
                } else {
                    SkASSERT(DSLWriter::Compiler().errorCount());
                    DSLWriter::ReportError(
                                      DSLWriter::Compiler().errorText(/*showCount=*/false).c_str());
                }
            }
            return INHERITED::visitStatement(stmt);
        }

    private:
        using INHERITED = ProgramWriter;
    };
    std::unique_ptr<Statement> body = block.release();
    FinalizeReturns().visitStatement(*body);
    DSLWriter::ProgramElements().emplace_back(new SkSL::FunctionDefinition(/*offset=*/-1,
                                                                           fDecl,
                                                                           /*builtin=*/false,
                                                                           std::move(body)));
    DSLWriter::SetCurrentFunction(nullptr);
}

} // namespace dsl

} // namespace SkSL
