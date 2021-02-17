/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/priv/DSLFPs.h"

#include "src/sksl/dsl/priv/DSLWriter.h"
#include "src/sksl/ir/SkSLCodeStringExpression.h"

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

namespace SkSL {

namespace dsl {

void StartFragmentProcessor(GrGLSLFragmentProcessor* processor,
                            GrGLSLFragmentProcessor::EmitArgs* emitArgs) {
    DSLWriter::StartFragmentProcessor(processor, emitArgs);
}

void EndFragmentProcessor() {
    DSLWriter::EndFragmentProcessor();
}

DSLVar sk_SampleCoord() {
    return DSLVar("sk_SampleCoord");
}

DSLExpression SampleChild(int index, DSLExpression coords) {
    std::unique_ptr<SkSL::Expression> coordsExpr = coords.release();
    SkString code = DSLWriter::CurrentProcessor()->invokeChild(index, *DSLWriter::CurrentEmitArgs(),
                                                              coordsExpr ? coordsExpr->description()
                                                                         : "");
    return DSLExpression(std::make_unique<SkSL::CodeStringExpression>(code.c_str(),
                                                         DSLWriter::Context().fTypes.fHalf4.get()));
}

GrGLSLUniformHandler::UniformHandle VarUniformHandle(const DSLVar& var) {
    return DSLWriter::VarUniformHandle(var);
}

} // namespace dsl

} // namespace SkSL

#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
