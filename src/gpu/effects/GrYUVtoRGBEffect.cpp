/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/builders/GrGLProgramBuilder.h"
#include "GrYUVtoRGBEffect.h"

#include "GrCoordTransform.h"
#include "GrEffect.h"
#include "gl/GrGLEffect.h"
#include "GrTBackendEffectFactory.h"

namespace {

class YUVtoRGBEffect : public GrEffect {
public:
    static GrEffect* Create(GrTexture* yTexture, GrTexture* uTexture, GrTexture* vTexture) {
        return SkNEW_ARGS(YUVtoRGBEffect, (yTexture, uTexture, vTexture));
    }

    static const char* Name() { return "YUV to RGB"; }

    virtual const GrBackendEffectFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendEffectFactory<YUVtoRGBEffect>::getInstance();
    }

    virtual void getConstantColorComponents(GrColor* color,
                                            uint32_t* validFlags) const SK_OVERRIDE {
        // YUV is opaque
        *color = 0xFF;
        *validFlags = kA_GrColorComponentFlag;
    }

    class GLEffect : public GrGLEffect {
    public:
        // this class always generates the same code.
        static void GenKey(const GrDrawEffect&, const GrGLCaps&, GrEffectKeyBuilder*) {}

        GLEffect(const GrBackendEffectFactory& factory,
                 const GrDrawEffect&)
        : INHERITED(factory) {
        }

        virtual void emitCode(GrGLProgramBuilder* builder,
                              const GrDrawEffect&,
                              const GrEffectKey&,
                              const char* outputColor,
                              const char* inputColor,
                              const TransformedCoordsArray& coords,
                              const TextureSamplerArray& samplers) SK_OVERRIDE {
            GrGLFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();
            const char* yuvMatrix   = "yuvMatrix";
            fsBuilder->codeAppendf("\tconst mat4 %s = mat4(1.0,  0.0,    1.402, -0.701,\n\t\t\t"
                                                          "1.0, -0.344, -0.714,  0.529,\n\t\t\t"
                                                          "1.0,  1.772,  0.0,   -0.886,\n\t\t\t"
                                                          "0.0,  0.0,    0.0,    1.0);\n",
                                   yuvMatrix);
            fsBuilder->codeAppendf("\t%s = vec4(\n\t\t", outputColor);
            fsBuilder->appendTextureLookup(samplers[0], coords[0].c_str(), coords[0].type());
            fsBuilder->codeAppend(".r,\n\t\t");
            fsBuilder->appendTextureLookup(samplers[1], coords[0].c_str(), coords[0].type());
            fsBuilder->codeAppend(".r,\n\t\t");
            fsBuilder->appendTextureLookup(samplers[2], coords[0].c_str(), coords[0].type());
            fsBuilder->codeAppendf(".r,\n\t\t1.0) * %s;\n", yuvMatrix);
        }

        typedef GrGLEffect INHERITED;
    };

private:
    YUVtoRGBEffect(GrTexture* yTexture, GrTexture* uTexture, GrTexture* vTexture)
    : fCoordTransform(kLocal_GrCoordSet, MakeDivByTextureWHMatrix(yTexture), yTexture)
    , fYAccess(yTexture)
    , fUAccess(uTexture)
    , fVAccess(vTexture) {
        this->addCoordTransform(&fCoordTransform);
        this->addTextureAccess(&fYAccess);
        this->addTextureAccess(&fUAccess);
        this->addTextureAccess(&fVAccess);
        this->setWillNotUseInputColor();
    }

    virtual bool onIsEqual(const GrEffect& sBase) const {
        const YUVtoRGBEffect& s = CastEffect<YUVtoRGBEffect>(sBase);
        return fYAccess.getTexture() == s.fYAccess.getTexture() &&
               fUAccess.getTexture() == s.fUAccess.getTexture() &&
               fVAccess.getTexture() == s.fVAccess.getTexture();
    }

    GrCoordTransform fCoordTransform;
    GrTextureAccess fYAccess;
    GrTextureAccess fUAccess;
    GrTextureAccess fVAccess;

    typedef GrEffect INHERITED;
};

}

//////////////////////////////////////////////////////////////////////////////

GrEffect* GrYUVtoRGBEffect::Create(GrTexture* yTexture, GrTexture* uTexture, GrTexture* vTexture) {
    return YUVtoRGBEffect::Create(yTexture, uTexture, vTexture);
}
