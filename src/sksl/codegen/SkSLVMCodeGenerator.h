/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_VMGENERATOR
#define SKSL_VMGENERATOR

#include "include/core/SkSpan.h"
#include "include/private/SkSLString.h"
#include "src/core/SkVM.h"
#include "src/sksl/ir/SkSLType.h"

#include <functional>

namespace SkSL {

class FunctionDefinition;
struct Program;

using SampleShaderFn = std::function<skvm::Color(int, skvm::Coord)>;
using SampleColorFilterFn = std::function<skvm::Color(int, skvm::Color)>;
using SampleBlenderFn = std::function<skvm::Color(int, skvm::Color, skvm::Color)>;

// Convert 'function' to skvm instructions in 'builder', for use by blends, shaders, & color filters
skvm::Color ProgramToSkVM(const Program& program,
                          const FunctionDefinition& function,
                          skvm::Builder* builder,
                          SkSpan<skvm::Val> uniforms,
                          skvm::Coord device,
                          skvm::Coord local,
                          skvm::Color inputColor,
                          skvm::Color destColor,
                          SampleShaderFn sampleShader,
                          SampleColorFilterFn sampleColorFilter,
                          SampleBlenderFn sampleBlender);

struct SkVMSignature {
    size_t fParameterSlots = 0;
    size_t fReturnSlots    = 0;
};

/*
 * Converts 'function' to skvm instructions in 'builder'. Always adds one arg per value in the
 * parameter list, then one per value in the return type. For example:
 *
 *   float2 fn(float2 a, float b) { ... }
 *
 * ... is mapped so that it can be called as:
 *
 *   p.eval(N, &a.x, &a.y, &b, &return.x, &return.y);
 *
 * The number of parameter and return slots (pointers) is placed in 'outSignature', if provided.
 * If the program declares any uniforms, 'uniforms' should contain the IDs of each individual value
 * (eg, one ID per component of a vector).
 */
bool ProgramToSkVM(const Program& program,
                   const FunctionDefinition& function,
                   skvm::Builder* b,
                   SkSpan<skvm::Val> uniforms,
                   SkVMSignature* outSignature = nullptr);

const FunctionDefinition* Program_GetFunction(const Program& program, const char* function);

struct UniformInfo {
    struct Uniform {
        String fName;
        Type::NumberKind fKind;
        int fColumns;
        int fRows;
        int fSlot;
    };
    std::vector<Uniform> fUniforms;
    int fUniformSlotCount = 0;
};

std::unique_ptr<UniformInfo> Program_GetUniformInfo(const Program& program);

bool testingOnly_ProgramToSkVMShader(const Program& program, skvm::Builder* builder);

}  // namespace SkSL

#endif
