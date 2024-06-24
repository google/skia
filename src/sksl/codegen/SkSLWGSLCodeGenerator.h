/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_WGSLCODEGENERATOR
#define SKSL_WGSLCODEGENERATOR

#include <string>

namespace SkSL {

class ErrorReporter;
class OutputStream;
enum class PrettyPrint : bool;
struct Program;
struct ShaderCaps;

enum class IncludeSyntheticCode : bool {
    kNo = false,
    kYes = true,
};

using ValidateWGSLProc = bool (*)(ErrorReporter&, std::string_view wgsl, std::string* warnings);

/**
 * Convert a Program into WGSL code.
 */
bool ToWGSL(Program& program,
            const ShaderCaps* caps,
            OutputStream& out,
            PrettyPrint,
            IncludeSyntheticCode,
            ValidateWGSLProc);
bool ToWGSL(Program& program, const ShaderCaps* caps, OutputStream& out);
bool ToWGSL(Program& program, const ShaderCaps* caps, std::string* out);

}  // namespace SkSL

#endif  // SKSL_WGSLCODEGENERATOR
