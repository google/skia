/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_HLSLCODEGENERATOR
#define SKSL_HLSLCODEGENERATOR

#include <string>
#include <string_view>

namespace SkSL {

class ErrorReporter;
class OutputStream;
struct Program;
struct ShaderCaps;

using ValidateSPIRVProc = bool (*)(ErrorReporter&, std::string_view);

/** Converts a Program into HLSL code. */
bool ToHLSL(Program& program,
            const ShaderCaps* caps,
            OutputStream& out,
            ValidateSPIRVProc = nullptr);
bool ToHLSL(Program& program, const ShaderCaps* caps, std::string* out, ValidateSPIRVProc);

// This explicit overload is used by SkSLToBackend.
inline bool ToHLSL(Program& program, const ShaderCaps* caps, std::string* out) {
    return ToHLSL(program, caps, out, nullptr);
}

}  // namespace SkSL

#endif
