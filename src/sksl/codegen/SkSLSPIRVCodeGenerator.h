/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_SPIRVCODEGENERATOR
#define SKSL_SPIRVCODEGENERATOR

#include <string>

namespace SkSL {

class OutputStream;
struct Program;
struct ShaderCaps;

/**
 * Converts a Program into a SPIR-V binary.
 */
bool ToSPIRV(Program& program, const ShaderCaps* caps, OutputStream& out);
bool ToSPIRV(Program& program, const ShaderCaps* caps, std::string* out);

}  // namespace SkSL

#endif
