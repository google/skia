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
        , fConfigComponentMask(GrPixelConfigComponentMask(access.getTexture()->config())) {
        SkASSERT(0 != fConfigComponentMask);
        memcpy(fSwizzle, access.getSwizzle(), 5);
    }

    // bitfield of GrColorComponentFlags present in the texture's config.
    uint32_t configComponentMask() const { return fConfigComponentMask; }
    // this is .abcd
    const char* swizzle() const { return fSwizzle; }

private:
    UniformHandle fSamplerUniform;
    uint32_t      fConfigComponentMask;
    char          fSwizzle[5];

    friend class GrGLShaderBuilder;
};

#endif
