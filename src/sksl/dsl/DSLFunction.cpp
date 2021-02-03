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
    auto function = std::make_unique<SkSL::FunctionDefinition>(/*offset=*/-1, fDecl,
                                                               /*builtin=*/false, block.release());
    DSLWriter::IRGenerator().finalizeFunction(*function);
    if (DSLWriter::Compiler().errorCount()) {
        DSLWriter::ReportError(DSLWriter::Compiler().errorText(/*showCount=*/false).c_str());
        DSLWriter::Compiler().setErrorCount(0);
        SkASSERT(!DSLWriter::Compiler().errorCount());
    }
    DSLWriter::ProgramElements().push_back(std::move(function));
}

} // namespace dsl

} // namespace SkSL
