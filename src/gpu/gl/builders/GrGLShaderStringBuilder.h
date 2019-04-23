/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLShaderStringBuilder_DEFINED
#define GrGLShaderStringBuilder_DEFINED

#include "include/core/SkTypes.h"
#include "src/gpu/GrAllocator.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/gl/GrGLContext.h"
#include "src/sksl/SkSLGLSLCodeGenerator.h"

std::unique_ptr<SkSL::Program> GrSkSLtoGLSL(const GrGLContext& context, GrGLenum type,
                                            const SkSL::String& sksl,
                                            const SkSL::Program::Settings& settings,
                                            SkSL::String* glsl);

GrGLuint GrGLCompileAndAttachShader(const GrGLContext& glCtx,
                                    GrGLuint programId,
                                    GrGLenum type,
                                    const char* glsl,
                                    int glslLength,
                                    GrGpu::Stats*,
                                    const SkSL::Program::Settings& settings);

void GrGLPrintShader(const GrGLContext&, GrGLenum type, const SkSL::String& sksl,
                     const SkSL::Program::Settings&);

#endif
