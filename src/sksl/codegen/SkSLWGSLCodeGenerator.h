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

class OutputStream;
struct Program;
struct ShaderCaps;

/**
 * Convert a Program into WGSL code.
 */
bool ToWGSL(Program& program, const ShaderCaps* caps, OutputStream& out);
bool ToWGSL(Program& program, const ShaderCaps* caps, std::string* out);

}  // namespace SkSL

#endif  // SKSL_WGSLCODEGENERATOR
