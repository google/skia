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
