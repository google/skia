/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLTextureSampler_DEFINED
#define GrGLSLTextureSampler_DEFINED

#include "GrShaderVar.h"
#include "GrTextureAccess.h"
#include "glsl/GrGLSLProgramDataManager.h"

class GrGLSLTextureSampler {
public:
    typedef GrGLSLProgramDataManager::UniformHandle UniformHandle;
    typedef SkTArray<GrGLSLTextureSampler> TextureSamplerArray;

    GrGLSLTextureSampler(UniformHandle uniform, const GrTextureAccess& access)
        : fSamplerUniform(uniform)
        , fConfig(access.getTexture()->config()) {
        SkASSERT(kUnknown_GrPixelConfig != fConfig);
    }

    GrPixelConfig config() const { return fConfig; }

private:
    UniformHandle fSamplerUniform;
    GrPixelConfig fConfig;

    friend class GrGLSLShaderBuilder;
};

#endif
