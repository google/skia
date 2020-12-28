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

struct SkVMSignature {
    size_t fParameterSlots = 0;
    size_t fReturnSlots    = 0;
};

/*
 * Converts 'function' to skvm instructions in 'builder'. Always adds one arg for uniforms, then
 * one per value in the parameter list, and finally one per value in the return type. For example:
 *
 *   uniform float u; uniform float v;
 *   float2 fn(float2 a, float b) { ... }
 *
 * ... is mapped so that it can be called as:
 *
 *   p.eval(N, uniforms, &a.x, &a.y, &b, &return.x, &return.y);
 *
 * The number of parameter and return slots (pointers) is placed in 'outSignature', if provided.
 */
bool ProgramToSkVM(const Program& program,
                   const FunctionDefinition& function,
                   skvm::Builder* b,
                   SkVMSignature* outSignature = nullptr);

const FunctionDefinition* Program_GetFunction(const Program& program, const char* function);

bool testingOnly_ProgramToSkVMShader(const Program& program, skvm::Builder* builder);

}  // namespace SkSL

#endif
