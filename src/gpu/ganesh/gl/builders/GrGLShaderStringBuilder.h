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
#include "src/gpu/ganesh/GrGpu.h"
#include "src/gpu/ganesh/gl/GrGLContext.h"
#include "src/sksl/codegen/SkSLGLSLCodeGenerator.h"

GrGLuint GrGLCompileAndAttachShader(const GrGLContext& glCtx,
                                    GrGLuint programId,
                                    GrGLenum type,
                                    const std::string& glsl,
                                    bool shaderWasCached,
                                    GrThreadSafePipelineBuilder::Stats*,
                                    GrContextOptions::ShaderErrorHandler* errorHandler);

bool GrGLCheckLinkStatus(const GrGLGpu* gpu,
                         GrGLuint programID,
                         bool shaderWasCached,
                         GrContextOptions::ShaderErrorHandler* errorHandler,
                         const std::string* sksl[kGrShaderTypeCount],
                         const std::string glsl[kGrShaderTypeCount]);

#endif
