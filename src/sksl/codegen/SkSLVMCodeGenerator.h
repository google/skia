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
class SkVMDebugTrace;

class SkVMCallbacks {
public:
    virtual ~SkVMCallbacks() = default;

    virtual skvm::Color sampleShader(int index, skvm::Coord coord) = 0;
    virtual skvm::Color sampleColorFilter(int index, skvm::Color color) = 0;
    virtual skvm::Color sampleBlender(int index, skvm::Color src, skvm::Color dst) = 0;

    virtual skvm::Color toLinearSrgb(skvm::Color color) = 0;
    virtual skvm::Color fromLinearSrgb(skvm::Color color) = 0;
};

// Convert 'function' to skvm instructions in 'builder', for use by blends, shaders, & color filters
skvm::Color ProgramToSkVM(const Program& program,
                          const FunctionDefinition& function,
                          skvm::Builder* builder,
                          SkVMDebugTrace* debugTrace,
                          SkSpan<skvm::Val> uniforms,
                          skvm::Coord device,
                          skvm::Coord local,
                          skvm::Color inputColor,
                          skvm::Color destColor,
                          SkVMCallbacks* callbacks);

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
                   SkVMDebugTrace* debugTrace,
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
                                     SkVMDebugTrace* debugTrace);

}  // namespace SkSL

#endif
