/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_HLSLCODEGENERATOR
#define SKSL_HLSLCODEGENERATOR

#include <string>

namespace SkSL {

class OutputStream;
struct Program;
struct ShaderCaps;

/** Converts a Program into HLSL code. (SPIRV-Cross must be enabled.) */
bool ToHLSL(Program& program, const ShaderCaps* caps, OutputStream& out);
bool ToHLSL(Program& program, const ShaderCaps* caps, std::string* out);

}  // namespace SkSL

#endif
