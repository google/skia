/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "gl/builders/GrGLProgramBuilder.h"
#include "GrYUVtoRGBEffect.h"

#include "GrCoordTransform.h"
#include "GrProcessor.h"
#include "gl/GrGLProcessor.h"
#include "GrTBackendProcessorFactory.h"

namespace {

class YUVtoRGBEffect : public GrFragmentProcessor {
public:
    static GrFragmentProcessor* Create(GrTexture* yTexture, GrTexture* uTexture,
                                       GrTexture* vTexture, SkYUVColorSpace colorSpace) {
        return SkNEW_ARGS(YUVtoRGBEffect, (yTexture, uTexture, vTexture, colorSpace));
    }

    static const char* Name() { return "YUV to RGB"; }

    virtual const GrBackendFragmentProcessorFactory& getFactory() const SK_OVERRIDE {
        return GrTBackendFragmentProcessorFactory<YUVtoRGBEffect>::getInstance();
    }

    virtual void getConstantColorComponents(GrColor* color,
                                            uint32_t* validFlags) const SK_OVERRIDE {
        // YUV is opaque
        *color = 0xFF;
        *validFlags = kA_GrColorComponentFlag;
    }

    SkYUVColorSpace getColorSpace() const {
        return fColorSpace;
    }

    class GLProcessor : public GrGLFragmentProcessor {
    public:
        static const GrGLfloat kJPEGConversionMatrix[16];
        static const GrGLfloat kRec601ConversionMatrix[16];

        // this class always generates the same code.
        static void GenKey(const GrProcessor&, const GrGLCaps&, GrProcessorKeyBuilder*) {}

        GLProcessor(const GrBackendProcessorFactory& factory,
                    const GrProcessor&)
        : INHERITED(factory) {
        }

        virtual void emitCode(GrGLProgramBuilder* builder,
                              const GrFragmentProcessor&,
                              const GrProcessorKey&,
                              const char* outputColor,
                              const char* inputColor,
                              const TransformedCoordsArray& coords,
                              const TextureSamplerArray& samplers) SK_OVERRIDE {
            GrGLFragmentShaderBuilder* fsBuilder = builder->getFragmentShaderBuilder();

            const char* yuvMatrix   = NULL;
            fMatrixUni = builder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                             kMat44f_GrSLType, "YUVMatrix",
                                             &yuvMatrix);
            fsBuilder->codeAppendf("\t%s = vec4(\n\t\t", outputColor);
            fsBuilder->appendTextureLookup(samplers[0], coords[0].c_str(), coords[0].getType());
            fsBuilder->codeAppend(".r,\n\t\t");
            fsBuilder->appendTextureLookup(samplers[1], coords[0].c_str(), coords[0].getType());
            fsBuilder->codeAppend(".r,\n\t\t");
            fsBuilder->appendTextureLookup(samplers[2], coords[0].c_str(), coords[0].getType());
            fsBuilder->codeAppendf(".r,\n\t\t1.0) * %s;\n", yuvMatrix);
        }

        virtual void setData(const GrGLProgramDataManager& pdman,
                             const GrProcessor& processor) SK_OVERRIDE {
            const YUVtoRGBEffect& yuvEffect = processor.cast<YUVtoRGBEffect>();
            switch (yuvEffect.getColorSpace()) {
                case kJPEG_SkYUVColorSpace:
                    pdman.setMatrix4f(fMatrixUni, kJPEGConversionMatrix);
                    break;
                case kRec601_SkYUVColorSpace:
                    pdman.setMatrix4f(fMatrixUni, kRec601ConversionMatrix);
                    break;
            }
        }

    private:
        GrGLProgramDataManager::UniformHandle fMatrixUni;

        typedef GrGLFragmentProcessor INHERITED;
    };

private:
    YUVtoRGBEffect(GrTexture* yTexture, GrTexture* uTexture, GrTexture* vTexture,
                   SkYUVColorSpace colorSpace)
     : fCoordTransform(kLocal_GrCoordSet, GrCoordTransform::MakeDivByTextureWHMatrix(yTexture),
                       yTexture)
    , fYAccess(yTexture)
    , fUAccess(uTexture)
    , fVAccess(vTexture)
    , fColorSpace(colorSpace) {
        this->addCoordTransform(&fCoordTransform);
        this->addTextureAccess(&fYAccess);
        this->addTextureAccess(&fUAccess);
        this->addTextureAccess(&fVAccess);
        this->setWillNotUseInputColor();
    }

    virtual bool onIsEqual(const GrProcessor& sBase) const {
        const YUVtoRGBEffect& s = sBase.cast<YUVtoRGBEffect>();
        return fYAccess.getTexture() == s.fYAccess.getTexture() &&
               fUAccess.getTexture() == s.fUAccess.getTexture() &&
               fVAccess.getTexture() == s.fVAccess.getTexture() &&
               fColorSpace == s.getColorSpace();
    }

    GrCoordTransform fCoordTransform;
    GrTextureAccess fYAccess;
    GrTextureAccess fUAccess;
    GrTextureAccess fVAccess;
    SkYUVColorSpace fColorSpace;

    typedef GrFragmentProcessor INHERITED;
};

const GrGLfloat YUVtoRGBEffect::GLProcessor::kJPEGConversionMatrix[16] = {
    1.0f,  0.0f,      1.402f,  -0.701f,
    1.0f, -0.34414f, -0.71414f, 0.529f,
    1.0f,  1.772f,    0.0f,    -0.886f,
    0.0f,  0.0f,      0.0f,     1.0};
const GrGLfloat YUVtoRGBEffect::GLProcessor::kRec601ConversionMatrix[16] = {
    1.164f,  0.0f,    1.596f, -0.87075f,
    1.164f, -0.391f, -0.813f,  0.52925f,
    1.164f,  2.018f,  0.0f,   -1.08175f,
    0.0f,    0.0f,    0.0f,    1.0};
}

//////////////////////////////////////////////////////////////////////////////

GrFragmentProcessor*
GrYUVtoRGBEffect::Create(GrTexture* yTexture, GrTexture* uTexture, GrTexture* vTexture,
                         SkYUVColorSpace colorSpace) {
    return YUVtoRGBEffect::Create(yTexture, uTexture, vTexture, colorSpace);
}
