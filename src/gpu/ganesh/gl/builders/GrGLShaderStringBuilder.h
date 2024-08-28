/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLShaderStringBuilder_DEFINED
#define GrGLShaderStringBuilder_DEFINED

#include "include/core/SkTypes.h"
#include "include/gpu/ganesh/GrContextOptions.h"
#include "include/gpu/ganesh/gl/GrGLTypes.h"
#include "include/private/gpu/ganesh/GrTypesPriv.h"
#include "src/gpu/SkSLToBackend.h"
#include "src/gpu/ganesh/GrThreadSafePipelineBuilder.h"
#include "src/gpu/ganesh/gl/GrGLContext.h"
#include "src/sksl/codegen/SkSLGLSLCodeGenerator.h"

#include <cstdint>
#include <string>

class GrGLGpu;

namespace SkSL {

enum class ProgramKind : int8_t;
struct ProgramInterface;
struct ProgramSettings;
struct ShaderCaps;

}  // namespace SkSL

namespace skgpu {

class ShaderErrorHandler;

inline bool SkSLToGLSL(const SkSL::ShaderCaps* caps,
                       const std::string& sksl,
                       SkSL::ProgramKind programKind,
                       const SkSL::ProgramSettings& settings,
                       std::string* glsl,
                       SkSL::ProgramInterface* outInterface,
                       ShaderErrorHandler* errorHandler) {
    return SkSLToBackend(caps, &SkSL::ToGLSL, "GLSL",
                         sksl, programKind, settings, glsl, outInterface, errorHandler);
}

}  // namespace skgpu

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
