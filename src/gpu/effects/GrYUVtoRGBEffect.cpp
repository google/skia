/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrYUVtoRGBEffect.h"

#include "GrCoordTransform.h"
#include "GrInvariantOutput.h"
#include "GrProcessor.h"
#include "gl/GrGLFragmentProcessor.h"
#include "gl/builders/GrGLProgramBuilder.h"

namespace {

class YUVtoRGBEffect : public GrFragmentProcessor {
public:
    static GrFragmentProcessor* Create(GrProcessorDataManager* procDataManager, GrTexture* yTexture,
                                       GrTexture* uTexture, GrTexture* vTexture,
                                       const SkISize sizes[3], SkYUVColorSpace colorSpace) {
        SkScalar w[3], h[3];
        w[0] = SkIntToScalar(sizes[0].fWidth)  / SkIntToScalar(yTexture->width());
        h[0] = SkIntToScalar(sizes[0].fHeight) / SkIntToScalar(yTexture->height());
        w[1] = SkIntToScalar(sizes[1].fWidth)  / SkIntToScalar(uTexture->width());
        h[1] = SkIntToScalar(sizes[1].fHeight) / SkIntToScalar(uTexture->height());
        w[2] = SkIntToScalar(sizes[2].fWidth)  / SkIntToScalar(vTexture->width());
        h[2] = SkIntToScalar(sizes[2].fHeight) / SkIntToScalar(vTexture->height());
        SkMatrix yuvMatrix[3];
        yuvMatrix[0] = GrCoordTransform::MakeDivByTextureWHMatrix(yTexture);
        yuvMatrix[1] = yuvMatrix[0];
        yuvMatrix[1].preScale(w[1] / w[0], h[1] / h[0]);
        yuvMatrix[2] = yuvMatrix[0];
        yuvMatrix[2].preScale(w[2] / w[0], h[2] / h[0]);
        GrTextureParams::FilterMode uvFilterMode =
            ((sizes[1].fWidth  != sizes[0].fWidth) ||
             (sizes[1].fHeight != sizes[0].fHeight) ||
             (sizes[2].fWidth  != sizes[0].fWidth) ||
             (sizes[2].fHeight != sizes[0].fHeight)) ?
            GrTextureParams::kBilerp_FilterMode :
            GrTextureParams::kNone_FilterMode;
        return SkNEW_ARGS(YUVtoRGBEffect, (procDataManager, yTexture, uTexture, vTexture, yuvMatrix,
                                           uvFilterMode, colorSpace));
    }

    const char* name() const override { return "YUV to RGB"; }

    SkYUVColorSpace getColorSpace() const {
        return fColorSpace;
    }

    class GLProcessor : public GrGLFragmentProcessor {
    public:
        static const GrGLfloat kJPEGConversionMatrix[16];
        static const GrGLfloat kRec601ConversionMatrix[16];
        static const GrGLfloat kRec709ConversionMatrix[16];

        // this class always generates the same code.
        static void GenKey(const GrProcessor&, const GrGLSLCaps&, GrProcessorKeyBuilder*) {}

        GLProcessor(const GrProcessor&) {}

        virtual void emitCode(EmitArgs& args) override {
            GrGLFragmentBuilder* fsBuilder = args.fBuilder->getFragmentShaderBuilder();

            const char* yuvMatrix   = NULL;
            fMatrixUni = args.fBuilder->addUniform(GrGLProgramBuilder::kFragment_Visibility,
                                             kMat44f_GrSLType, kDefault_GrSLPrecision,
                                             "YUVMatrix", &yuvMatrix);
            fsBuilder->codeAppendf("\t%s = vec4(\n\t\t", args.fOutputColor);
            fsBuilder->appendTextureLookup(args.fSamplers[0], args.fCoords[0].c_str(),
                                           args.fCoords[0].getType());
            fsBuilder->codeAppend(".r,\n\t\t");
            fsBuilder->appendTextureLookup(args.fSamplers[1], args.fCoords[1].c_str(),
                                           args.fCoords[1].getType());
            fsBuilder->codeAppend(".r,\n\t\t");
            fsBuilder->appendTextureLookup(args.fSamplers[2], args.fCoords[2].c_str(),
                                           args.fCoords[2].getType());
            fsBuilder->codeAppendf(".r,\n\t\t1.0) * %s;\n", yuvMatrix);
        }

        virtual void setData(const GrGLProgramDataManager& pdman,
                             const GrProcessor& processor) override {
            const YUVtoRGBEffect& yuvEffect = processor.cast<YUVtoRGBEffect>();
            switch (yuvEffect.getColorSpace()) {
                case kJPEG_SkYUVColorSpace:
                    pdman.setMatrix4f(fMatrixUni, kJPEGConversionMatrix);
                    break;
                case kRec601_SkYUVColorSpace:
                    pdman.setMatrix4f(fMatrixUni, kRec601ConversionMatrix);
                    break;
                case kRec709_SkYUVColorSpace:
                    pdman.setMatrix4f(fMatrixUni, kRec709ConversionMatrix);
                    break;
            }
        }

    private:
        GrGLProgramDataManager::UniformHandle fMatrixUni;

        typedef GrGLFragmentProcessor INHERITED;
    };

    GrGLFragmentProcessor* createGLInstance() const override {
        return SkNEW_ARGS(GLProcessor, (*this));
    }

private:
    YUVtoRGBEffect(GrProcessorDataManager*, GrTexture* yTexture, GrTexture* uTexture,
                   GrTexture* vTexture, const SkMatrix yuvMatrix[3],
                   GrTextureParams::FilterMode uvFilterMode, SkYUVColorSpace colorSpace)
    : fYTransform(kLocal_GrCoordSet, yuvMatrix[0], yTexture, GrTextureParams::kNone_FilterMode)
    , fYAccess(yTexture)
    , fUTransform(kLocal_GrCoordSet, yuvMatrix[1], uTexture, uvFilterMode)
    , fUAccess(uTexture, uvFilterMode)
    , fVTransform(kLocal_GrCoordSet, yuvMatrix[2], vTexture, uvFilterMode)
    , fVAccess(vTexture, uvFilterMode)
    , fColorSpace(colorSpace) {
        this->initClassID<YUVtoRGBEffect>();
        this->addCoordTransform(&fYTransform);
        this->addTextureAccess(&fYAccess);
        this->addCoordTransform(&fUTransform);
        this->addTextureAccess(&fUAccess);
        this->addCoordTransform(&fVTransform);
        this->addTextureAccess(&fVAccess);
    }

    virtual void onGetGLProcessorKey(const GrGLSLCaps& caps,
                                     GrProcessorKeyBuilder* b) const override {
        GLProcessor::GenKey(*this, caps, b);
    }

    bool onIsEqual(const GrFragmentProcessor& sBase) const override {
        const YUVtoRGBEffect& s = sBase.cast<YUVtoRGBEffect>();
        return fColorSpace == s.getColorSpace();
    }

    void onComputeInvariantOutput(GrInvariantOutput* inout) const override {
        // YUV is opaque
        inout->setToOther(kA_GrColorComponentFlag, 0xFF << GrColor_SHIFT_A,
                          GrInvariantOutput::kWillNot_ReadInput);
    }

    GrCoordTransform fYTransform;
    GrTextureAccess fYAccess;
    GrCoordTransform fUTransform;
    GrTextureAccess fUAccess;
    GrCoordTransform fVTransform;
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
const GrGLfloat YUVtoRGBEffect::GLProcessor::kRec709ConversionMatrix[16] = {
    1.164f,  0.0f,    1.793f, -0.96925f,
    1.164f, -0.213f, -0.533f,  0.30025f,
    1.164f,  2.112f,  0.0f,   -1.12875f,
    0.0f,    0.0f,    0.0f,    1.0f};
}

//////////////////////////////////////////////////////////////////////////////

GrFragmentProcessor*
GrYUVtoRGBEffect::Create(GrProcessorDataManager* procDataManager, GrTexture* yTexture,
                         GrTexture* uTexture, GrTexture* vTexture, const SkISize sizes[3],
                         SkYUVColorSpace colorSpace) {
    SkASSERT(procDataManager && yTexture && uTexture && vTexture && sizes);
    return YUVtoRGBEffect::Create(procDataManager, yTexture, uTexture, vTexture, sizes, colorSpace);
}
