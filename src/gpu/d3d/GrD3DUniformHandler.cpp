/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/gpu/d3d/GrD3DUniformHandler.h"

#include "src/gpu/GrTexturePriv.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"

GrD3DUniformHandler::~GrD3DUniformHandler() {}

GrGLSLUniformHandler::UniformHandle GrD3DUniformHandler::internalAddUniformArray(
                                                                   const GrFragmentProcessor* owner,
                                                                   uint32_t visibility,
                                                                   GrSLType type,
                                                                   const char* name,
                                                                   bool mangleName,
                                                                   int arrayCount,
                                                                   const char** outName) {
    return {};
}

GrGLSLUniformHandler::SamplerHandle GrD3DUniformHandler::addSampler(
        const GrBackendFormat& backendFormat, GrSamplerState state, const GrSwizzle& swizzle,
        const char* name, const GrShaderCaps* shaderCaps) {
    return {};
}

void GrD3DUniformHandler::appendUniformDecls(GrShaderFlags visibility, SkString* out) const {
}

