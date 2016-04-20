/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrGLSLSampler_DEFINED
#define GrGLSLSampler_DEFINED

#include "GrTypes.h"
#include "GrTypesPriv.h"
#include "SkString.h"

class GrGLSLSampler {
public:
    virtual ~GrGLSLSampler() {}

    explicit GrGLSLSampler(uint32_t visibility, GrPixelConfig config)
        : fVisibility(visibility)
        , fConfig(config) {
        SkASSERT(kUnknown_GrPixelConfig != fConfig);
    }

    uint32_t visibility() const { return fVisibility; }
    GrPixelConfig config() const { return fConfig; }
    virtual GrSLType type() const = 0;

    // Returns the string to be used for the sampler in glsl 2D texture functions (texture,
    // texture2D, etc.)
    const char* getSamplerNameForTexture2D() const {
        SkASSERT(GrSLTypeIs2DTextureType(this->type()));
        return this->onGetSamplerNameForTexture2D();
    }

    // Returns the string to be used for the sampler in glsl texelFetch.
    virtual const char* getSamplerNameForTexelFetch() const = 0;

private:
    virtual const char* onGetSamplerNameForTexture2D() const = 0;
    uint32_t      fVisibility;
    GrPixelConfig fConfig;
};

#endif
