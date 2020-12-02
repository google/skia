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

#include <functional>

namespace SkSL {

class FunctionDefinition;
struct Program;

using SampleChildFn = std::function<skvm::Color(int, skvm::Coord)>;

// TODO: Have a generic entry point, supporting SkSpan<skvm::Val> for parameters and return values.
// That would be useful for interpreter use cases like SkParticleEffect.

// Convert 'function' to skvm instructions in 'builder', for use by shaders and color filters
skvm::Color ProgramToSkVM(const Program& program,
                          const FunctionDefinition& function,
                          skvm::Builder* builder,
                          SkSpan<skvm::Val> uniforms,
                          skvm::Coord device,
                          skvm::Coord local,
                          SampleChildFn sampleChild);

}  // namespace SkSL

#endif
