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

class SkWStream;

namespace SkSL {

class FunctionDefinition;
struct Program;

using SampleShaderFn = std::function<skvm::Color(int, skvm::Coord)>;
using SampleColorFilterFn = std::function<skvm::Color(int, skvm::Color)>;
using SampleBlenderFn = std::function<skvm::Color(int, skvm::Color, skvm::Color)>;

struct SkVMSlotInfo {
    /** The full name of this variable (without component): (e.g. `myArray[3].myStruct.myVector`) */
    std::string             name;
    /** The dimensions of this variable: 1x1 is a scalar, Nx1 is a vector, NxM is a matrix. */
    uint8_t                 columns = 1, rows = 1;
    /** Which component of the variable is this slot? (e.g. `vec4.z` is component 2) */
    uint8_t                 componentIndex = 0;
    /** What kind of numbers belong in this slot? */
    SkSL::Type::NumberKind  numberKind = SkSL::Type::NumberKind::kNonnumeric;
    /** Where is this variable located in the program? */
    int                     line;
};

struct SkVMDebugInfo {
    void dump(SkWStream* o) const;

    std::vector<SkVMSlotInfo> fSlotInfo;
};

// Convert 'function' to skvm instructions in 'builder', for use by blends, shaders, & color filters
skvm::Color ProgramToSkVM(const Program& program,
                          const FunctionDefinition& function,
                          skvm::Builder* builder,
                          SkVMDebugInfo* debugInfo,
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
                   SkVMDebugInfo* debugInfo,
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

bool testingOnly_ProgramToSkVMShader(const Program& program,
                                     skvm::Builder* builder,
                                     SkVMDebugInfo* debugInfo);

}  // namespace SkSL

#endif
