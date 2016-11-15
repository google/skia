/*
* Copyright 2016 Google Inc.
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/

#ifndef GrGLImage_DEFINED
#define GrGLImage_DEFINED

#include "glsl/GrGLSLImage.h"
#include "glsl/GrGLSLShaderVar.h"
#include "gl/GrGLTypes.h"

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
