/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_FPS
#define SKSL_DSL_FPS

#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/sksl/dsl/DSL.h"

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

namespace SkSL {

namespace dsl {

void PushFP(GrGLSLFragmentProcessor* processor, GrGLSLFragmentProcessor::EmitArgs* emitArgs);

void PopFP();

} // namespace dsl

} // namespace SkSL

#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

#endif
