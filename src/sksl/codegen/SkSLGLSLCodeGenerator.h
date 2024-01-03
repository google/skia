/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_GLSLCODEGENERATOR
#define SKSL_GLSLCODEGENERATOR

#include <string>

namespace SkSL {

class OutputStream;
struct Program;
struct ShaderCaps;

/** Converts a Program into GLSL code. */
bool ToGLSL(Program& program, const ShaderCaps* caps, OutputStream& out);
bool ToGLSL(Program& program, const ShaderCaps* caps, std::string* out);

}  // namespace SkSL

#endif
