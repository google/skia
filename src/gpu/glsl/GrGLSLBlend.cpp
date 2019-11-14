/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/glsl/GrGLSLBlend.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"

void GrGLSLBlend::AppendMode(GrGLSLFragmentBuilder* fsBuilder, const char* srcColor,
                             const char* dstColor, const char* outColor,
                             SkBlendMode mode) {
    fsBuilder->codeAppendf("%s = blend(%d, %s, %s);", outColor, static_cast<int>(mode), srcColor, dstColor);
}
