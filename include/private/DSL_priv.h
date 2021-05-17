/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef DSL_PRIV
#define DSL_PRIV

#include "include/sksl/SkSLProgramSettings.h"

namespace SkSL {

class Compiler;

namespace dsl {

/**
 * Initializes the DSL for compiling builtin code (SkSL include files).
 */
void StartBuiltinCode(SkSL::Compiler* compiler, SkSL::ProgramKind kind,
                      SkSL::ProgramSettings settings, SkSL::ParsedModule module);

} // namespace dsl

} // namespace SkSL

#endif
