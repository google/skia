/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/sksl/DSLBlock.h"

#include "include/sksl/DSLStatement.h"
#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLBlock.h"

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#endif

namespace SkSL {

namespace dsl {

DSLBlock::DSLBlock(SkSL::StatementArray statements)
    : fStatements(std::move(statements)) {}

DSLBlock::~DSLBlock() {
#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
    if (!fStatements.empty() && DSLWriter::InFragmentProcessor()) {
        DSLWriter::CurrentEmitArgs()->fFragBuilder->codeAppend(this->release());
        return;
    }
#endif
    SkASSERTF(fStatements.empty(), "Block destroyed without being incorporated into program");
}

std::unique_ptr<SkSL::Statement> DSLBlock::release() {
    return std::make_unique<SkSL::Block>(/*offset=*/-1, std::move(fStatements));
}

void DSLBlock::append(DSLStatement stmt) {
    fStatements.push_back(stmt.release());
}

} // namespace dsl

} // namespace SkSL
