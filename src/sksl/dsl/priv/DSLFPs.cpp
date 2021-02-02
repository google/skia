/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/sksl/dsl/priv/DSLFPs.h"

#include "src/sksl/dsl/priv/DSLWriter.h"

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

} // namespace dsl

} // namespace SkSL

#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU
