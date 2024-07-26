/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SPIRVCODEGENERATOR
#define SKSL_SPIRVCODEGENERATOR

#include <string>
#include <string_view>

namespace SkSL {

class ErrorReporter;
class OutputStream;
struct Program;
struct ShaderCaps;

using ValidateSPIRVProc = bool (*)(ErrorReporter&, std::string_view);

/**
 * Converts a Program into a SPIR-V binary.
 */
bool ToSPIRV(Program& program, const ShaderCaps* caps, OutputStream& out, ValidateSPIRVProc = nullptr);
bool ToSPIRV(Program& program, const ShaderCaps* caps, std::string* out, ValidateSPIRVProc);

// This explicit overload is used by SkSLToBackend.
inline bool ToSPIRV(Program& program, const ShaderCaps* caps, std::string* out) {
    return ToSPIRV(program, caps, out, nullptr);
}
}  // namespace SkSL

#endif
