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
        SkASSERT(GrSLTypeIsCombinedSamplerType(type));
        fShaderVar.setType(type);
        fShaderVar.setTypeModifier(GrGLSLShaderVar::kUniform_TypeModifier);
        fShaderVar.setPrecision(precision);
        fShaderVar.accessName()->set(name);
    }

    GrGLint location() const { return fLocation; }
    GrSLType type() const override { return fShaderVar.getType(); }

    const char* getSamplerNameForTexelFetch() const override { return fShaderVar.c_str(); }

private:
    const char* onGetSamplerNameForTexture2D() const override { return fShaderVar.c_str(); }

    GrGLSLShaderVar fShaderVar;
    GrGLint         fLocation;

    friend class GrGLUniformHandler;

    typedef GrGLSLSampler INHERITED;
};

class GrGLImage : public GrGLSLImage {
public:
    GrGLImage(uint32_t visibility, GrPixelConfig config, const char* name)
            : INHERITED(visibility) {
        fShaderVar.setName(name);
        if (GrPixelConfigIsSint(config)) {
            fShaderVar.setType(GrSLType::kIImage2D_GrSLType);
        } else {
            fShaderVar.setType(GrSLType::kImage2D_GrSLType);
        }
        fShaderVar.setTypeModifier(GrGLSLShaderVar::kUniform_TypeModifier);
        const char* layout;
        switch (config) {
            case kUnknown_GrPixelConfig:
                layout = "";
                break;
            case kRGBA_8888_GrPixelConfig:
                layout = "rgba8";
                break;
            case kRGBA_float_GrPixelConfig:
                layout = "rgba32";
                break;
            case kRGBA_half_GrPixelConfig:
                layout = "rgba16f";
                break;
            case kRGBA_8888_sint_GrPixelConfig:
                layout = "rgba8i";
                break;
            default:
                SkFAIL("Unexpected config for 2D image.");
                layout = "--invalid--";
                break;
        }
        fShaderVar.setLayoutQualifier(layout);
    }

    GrGLint location() const { return fLocation; }

    const char* name() const override { return fShaderVar.c_str(); }

private:
    GrGLSLShaderVar fShaderVar;
    GrGLint         fLocation;

    friend class GrGLUniformHandler;

    typedef GrGLSLImage INHERITED;
};

#endif
