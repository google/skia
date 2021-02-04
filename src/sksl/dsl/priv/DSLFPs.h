/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_FPS
#define SKSL_DSL_FPS

#include "src/sksl/dsl/DSL.h"

class GrGLSLFragmentProcessor;

namespace SkSL {

namespace dsl {

void StartFragmentProcessor(GrGLSLFragmentProcessor* processor,
                            void* emitArgs);

void EndFragmentProcessor();

} // namespace dsl

} // namespace SkSL

#endif
