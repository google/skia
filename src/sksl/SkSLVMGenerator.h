/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_VMGENERATOR
#define SKSL_VMGENERATOR

#include "src/core/SkSpan.h"
#include "src/core/SkVM.h"

namespace SkSL {

struct Program;

bool ProgramToSkVM(const Program* program,
                   const char* functionName,
                   skvm::Builder* builder,
                   SkSpan<skvm::Val> uniforms,
                   SkSpan<skvm::Val> params,      // TODO: Write back outParams here, too
                   skvm::Coord deviceCoord,       // Make this more generic
                   SkSpan<skvm::Val> outReturn);

}  // namespace SkSL

#endif
