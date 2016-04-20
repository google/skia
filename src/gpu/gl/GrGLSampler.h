/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrGLSampler_DEFINED
#define GrGLSampler_DEFINED

#include "glsl/GrGLSLSampler.h"

#include "gl/GrGLTypes.h"
#include "glsl/GrGLSLShaderVar.h"

class GrGLSampler : public GrGLSLSampler {
public:
    GrGLSampler(uint32_t visibility,
                GrPixelConfig config,
                GrSLType type,
                GrSLPrecision precision,
                const char* name) : INHERITED(visibility, config) {
        SkASSERT(GrSLTypeIsSamplerType(type));
        fShaderVar.setType(type);
        fShaderVar.setTypeModifier(GrGLSLShaderVar::kUniform_TypeModifier);
        fShaderVar.setPrecision(precision);
        fShaderVar.accessName()->set(name);
    }

    GrGLint location() const { return fLocation; }
    GrSLType type() const override { return fShaderVar.getType(); }

    const char* onGetSamplerNameForTexture2D() const override { return fShaderVar.c_str(); }
    const char* getSamplerNameForTexelFetch() const override { return fShaderVar.c_str(); }

private:
    GrGLSLShaderVar fShaderVar;
    GrGLint         fLocation;

    friend class GrGLUniformHandler;

    typedef GrGLSLSampler INHERITED;
};

#endif
