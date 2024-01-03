/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_METALCODEGENERATOR
#define SKSL_METALCODEGENERATOR

#include <string>

namespace SkSL {

class OutputStream;
struct Program;
struct ShaderCaps;

/**
 * Converts a Program into Metal code.
 */
bool ToMetal(Program& program, const ShaderCaps* caps, OutputStream& out);
bool ToMetal(Program& program, const ShaderCaps* caps, std::string* out);

}  // namespace SkSL

#endif
