/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrVkGLSLSampler_DEFINED
#define GrVkGLSLSampler_DEFINED

#include "glsl/GrGLSLSampler.h"

#include "glsl/GrGLSLShaderVar.h"

class GrVkGLSLSampler : public GrGLSLSampler {
public:
    GrVkGLSLSampler(uint32_t visibility,
                    GrPixelConfig config,
                    GrSLType type,
                    GrSLPrecision precision,
                    const char* name,
                    uint32_t binding,
                    uint32_t set) : INHERITED(visibility, config), fBinding(binding) {
        SkASSERT(GrSLTypeIsSamplerType(type));
        fShaderVar.setType(type);
        fShaderVar.setTypeModifier(GrGLSLShaderVar::kUniform_TypeModifier);
        fShaderVar.setPrecision(precision);
        fShaderVar.accessName()->set(name);
        SkString layoutQualifier;
        layoutQualifier.appendf("set=%d, binding=%d", set, binding);
        fShaderVar.setLayoutQualifier(layoutQualifier.c_str());
    }

    GrSLType type() const override { return fShaderVar.getType(); }
    uint32_t binding() const { return fBinding; }

    const char* onGetSamplerNameForTexture2D() const override { return fShaderVar.c_str(); }
    const char* getSamplerNameForTexelFetch() const override { return fShaderVar.c_str(); }

private:
    GrGLSLShaderVar fShaderVar;
    uint32_t        fBinding;

    friend class GrVkUniformHandler;

    typedef GrGLSLSampler INHERITED;
};

#endif
