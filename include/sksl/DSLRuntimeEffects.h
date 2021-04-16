/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_RUNTIME_EFFECTS
#define SKSL_DSL_RUNTIME_EFFECTS

#include "include/sksl/DSL.h"

class SkRuntimeEffect;

namespace SkSL {

class Compiler;

namespace dsl {

#ifndef SKSL_STANDALONE

void StartRuntimeShader(SkSL::Compiler* compiler);

sk_sp<SkRuntimeEffect> EndRuntimeShader();

#endif

} // namespace dsl

} // namespace SkSL

#endif
