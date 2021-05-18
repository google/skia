/*
 * Copyright 2021 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_DSL_PRIV
#define SKSL_DSL_PRIV

#include "include/private/SkSLProgramKind.h"
#include "src/sksl/SkSLParsedModule.h"

namespace SkSL {

class Compiler;
struct ProgramSettings;

namespace dsl {

/**
 * Initializes the DSL for compiling modules (SkSL include files).
 */
void StartModule(SkSL::Compiler* compiler, SkSL::ProgramKind kind,
                 const SkSL::ProgramSettings& settings, SkSL::ParsedModule module);

} // namespace dsl

} // namespace SkSL

#endif
