/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLShaderStringBuilder_DEFINED
#define GrGLShaderStringBuilder_DEFINED

#include "include/core/SkTypes.h"
#include "include/gpu/GrContextOptions.h"
#include "src/gpu/GrGpu.h"
#include "src/gpu/gl/GrGLContext.h"
#include "src/sksl/SkSLGLSLCodeGenerator.h"

std::unique_ptr<SkSL::Program> GrSkSLtoGLSL(const GrGLGpu* gpu,
                                            SkSL::ProgramKind programKind,
                                            const SkSL::String& sksl,
                                            const SkSL::Program::Settings& settings,
                                            SkSL::String* glsl,
                                            GrContextOptions::ShaderErrorHandler* errorHandler);

GrGLuint GrGLCompileAndAttachShader(const GrGLContext& glCtx,
                                    GrGLuint programId,
                                    GrGLenum type,
                                    const SkSL::String& glsl,
                                    GrThreadSafePipelineBuilder::Stats*,
                                    GrContextOptions::ShaderErrorHandler* errorHandler);

#endif
