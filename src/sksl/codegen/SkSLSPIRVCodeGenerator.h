/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SPIRVCODEGENERATOR
#define SKSL_SPIRVCODEGENERATOR

#include "include/core/SkSpan.h"
#include "src/sksl/codegen/SkSLNativeShader.h"

#include <cstdint>
#include <vector>

namespace SkSL {

class ErrorReporter;
class OutputStream;
struct Program;
struct ShaderCaps;

using ValidateSPIRVProc = bool (*)(ErrorReporter&, SkSpan<const uint32_t>);

// The SPIR-V version emitted by ToSPIRV, encoded as in the module header word (0x00MMmm00).
// This is intentionally independent of SpvVersion in spirv.h, which tracks the newest version the
// header describes. Tools that validate or disassemble SkSL's SPIR-V output must use a
// spvtools target environment that accepts this version.
inline constexpr uint32_t kSPIRVVersion = 0x00010000;  // SPIR-V 1.0

/**
 * Converts a Program into a SPIR-V binary. Prefer the std::vector<uint32_t> variant bacause the
 * OutputStream variant incurs an additional copy.
 */
bool ToSPIRV(Program& program, const ShaderCaps* caps, OutputStream& out, ValidateSPIRVProc = nullptr);
bool ToSPIRV(Program& program,
             const ShaderCaps* caps,
             std::vector<uint32_t>* out,
             ValidateSPIRVProc = nullptr);

// This explicit overload is used by SkSLToBackend.
inline bool ToSPIRV(Program& program, const ShaderCaps* caps, NativeShader* out) {
    return ToSPIRV(program, caps, &out->fBinary, nullptr);
}
}  // namespace SkSL

#endif
