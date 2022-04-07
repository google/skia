/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_FPS
#define SKSL_DSL_FPS

#include "include/core/SkTypes.h" // IWYU pragma: keep

#if !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

#include "include/sksl/DSLExpression.h"
#include "include/sksl/DSLVar.h"
#include "src/gpu/ganesh/GrFragmentProcessor.h"
#include "src/gpu/ganesh/glsl/GrGLSLUniformHandler.h"

namespace SkSL {

namespace dsl {

void StartFragmentProcessor(GrFragmentProcessor::ProgramImpl* processor,
                            GrFragmentProcessor::ProgramImpl::EmitArgs* emitArgs);

void EndFragmentProcessor();

DSLGlobalVar sk_SampleCoord();

DSLExpression SampleChild(int index, DSLExpression coords = DSLExpression());

GrGLSLUniformHandler::UniformHandle VarUniformHandle(const DSLGlobalVar& var);

} // namespace dsl

} // namespace SkSL

#endif // !defined(SKSL_STANDALONE) && SK_SUPPORT_GPU

#endif
