/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrGLSampler_DEFINED
#define GrGLSampler_DEFINED

#include "GrShaderVar.h"
#include "gl/GrGLTypes.h"
#include "glsl/GrGLSLSampler.h"

class GrGLSampler : public GrGLSLSampler {
public:
    GrGLSampler(uint32_t visibility,
                GrPixelConfig config,
                GrSLType type,
                GrSLPrecision precision,
                const char* name) : INHERITED(visibility, config) {
        SkASSERT(GrSLTypeIsCombinedSamplerType(type));
        fShaderVar.setType(type);
        fShaderVar.setTypeModifier(GrShaderVar::kUniform_TypeModifier);
        fShaderVar.setPrecision(precision);
        fShaderVar.accessName()->set(name);
    }

    GrGLint location() const { return fLocation; }
    GrSLType type() const override { return fShaderVar.getType(); }

    const char* onGetSamplerNameForTexture2D() const override { return fShaderVar.c_str(); }
    const char* getSamplerNameForTexelFetch() const override { return fShaderVar.c_str(); }

private:
    GrShaderVar fShaderVar;
    GrGLint         fLocation;

    friend class GrGLUniformHandler;

    typedef GrGLSLSampler INHERITED;
};

#endif
