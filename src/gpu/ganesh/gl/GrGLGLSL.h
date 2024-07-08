/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLGLSL_DEFINED
#define GrGLGLSL_DEFINED

namespace SkSL {
enum class GLSLGeneration;
}

struct GrGLDriverInfo;

/**
 * Gets the most recent GLSL Generation compatible with the OpenGL context.
 */
bool GrGLGetGLSLGeneration(const GrGLDriverInfo&, SkSL::GLSLGeneration* generation);

#endif
