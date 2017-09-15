/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "GrYUVEffect.h"

#include "GrCoordTransform.h"
#include "GrFragmentProcessor.h"
#include "GrProcessor.h"
#include "GrTextureProxy.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"

namespace {

static const float kJPEGConversionMatrix[16] = {
    1.0f,  0.0f,       1.402f,   -0.703749f,
    1.0f, -0.344136f, -0.714136f, 0.531211f,
    1.0f,  1.772f,     0.0f,     -0.889475f,
    0.0f,  0.0f,       0.0f,      1.0
};

static const float kRec601ConversionMatrix[16] = {
    1.164f,  0.0f,    1.596f, -0.87075f,
    1.164f, -0.391f, -0.813f,  0.52925f,
    1.164f,  2.018f,  0.0f,   -1.08175f,
    0.0f,    0.0f,    0.0f,    1.0}
;

static const float kRec709ConversionMatrix[16] = {
    1.164f,  0.0f,    1.793f, -0.96925f,
    1.164f, -0.213f, -0.533f,  0.30025f,
    1.164f,  2.112f,  0.0f,   -1.12875f,
    0.0f,    0.0f,    0.0f,    1.0f}
;

class YUVtoRGBEffect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(sk_sp<GrTextureProxy> yProxy,
                                                     sk_sp<GrTextureProxy> uProxy,
                                                     sk_sp<GrTextureProxy> vProxy,
                                                     const SkISize sizes[3],
                                                     SkYUVColorSpace colorSpace, bool nv12) {
        SkScalar w[3], h[3];
        w[0] = SkIntToScalar(sizes[0].fWidth);
        h[0] = SkIntToScalar(sizes[0].fHeight);
        w[1] = SkIntToScalar(sizes[1].fWidth);
        h[1] = SkIntToScalar(sizes[1].fHeight);
        w[2] = SkIntToScalar(sizes[2].fWidth);
        h[2] = SkIntToScalar(sizes[2].fHeight);
        const SkMatrix yuvMatrix[3] = {
            SkMatrix::I(),
            SkMatrix::MakeScale(w[1] / w[0], h[1] / h[0]),
            SkMatrix::MakeScale(w[2] / w[0], h[2] / h[0])
        };
        GrSamplerState::Filter uvFilterMode =
            ((sizes[1].fWidth  != sizes[0].fWidth) ||
             (sizes[1].fHeight != sizes[0].fHeight) ||
             (sizes[2].fWidth  != sizes[0].fWidth) ||
             (sizes[2].fHeight != sizes[0].fHeight)) ?
            GrSamplerState::Filter::kBilerp :
            GrSamplerState::Filter::kNearest;
        return std::unique_ptr<GrFragmentProcessor>(
                new YUVtoRGBEffect(std::move(yProxy), std::move(uProxy), std::move(vProxy),
                                   yuvMatrix, uvFilterMode, colorSpace, nv12));
    }

    const char* name() const override { return "YUV to RGB"; }

    std::unique_ptr<GrFragmentProcessor> clone() const override {
        return std::unique_ptr<GrFragmentProcessor>(new YUVtoRGBEffect(*this));
    }

    SkYUVColorSpace getColorSpace() const { return fColorSpace; }

    bool isNV12() const {
        return fNV12;
    }

    class GLSLProcessor : public GrGLSLFragmentProcessor {
    public:
        void emitCode(EmitArgs& args) override {
            GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
            const YUVtoRGBEffect& effect = args.fFp.cast<YUVtoRGBEffect>();

            const char* colorSpaceMatrix = nullptr;
            fMatrixUni = args.fUniformHandler->addUniform(kFragment_GrShaderFlag, kHalf4x4_GrSLType,
                                                          "ColorSpaceMatrix", &colorSpaceMatrix);
            fragBuilder->codeAppendf("%s = half4(", args.fOutputColor);
            fragBuilder->appendTextureLookup(args.fTexSamplers[0],
                                             args.fTransformedCoords[0].c_str(),
                                             args.fTransformedCoords[0].getType());
            fragBuilder->codeAppend(".r,");
            fragBuilder->appendTextureLookup(args.fTexSamplers[1],
                                             args.fTransformedCoords[1].c_str(),
                                             args.fTransformedCoords[1].getType());
            if (effect.fNV12) {
                fragBuilder->codeAppendf(".rg,");
            } else {
                fragBuilder->codeAppend(".r,");
                fragBuilder->appendTextureLookup(args.fTexSamplers[2],
                                                 args.fTransformedCoords[2].c_str(),
                                                 args.fTransformedCoords[2].getType());
                fragBuilder->codeAppendf(".r,");
            }
            fragBuilder->codeAppendf("1.0) * %s;", colorSpaceMatrix);
        }

    protected:
        void onSetData(const GrGLSLProgramDataManager& pdman,
                       const GrFragmentProcessor& processor) override {
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
        GrGLSLProgramDataManager::UniformHandle fMatrixUni;

        typedef GrGLSLFragmentProcessor INHERITED;
    };

private:
    YUVtoRGBEffect(sk_sp<GrTextureProxy> yProxy, sk_sp<GrTextureProxy> uProxy,
                   sk_sp<GrTextureProxy> vProxy, const SkMatrix yuvMatrix[3],
                   GrSamplerState::Filter uvFilterMode, SkYUVColorSpace colorSpace, bool nv12)
            : INHERITED(kPreservesOpaqueInput_OptimizationFlag)
            , fYTransform(yuvMatrix[0], yProxy.get())
            , fYSampler(std::move(yProxy))
            , fUTransform(yuvMatrix[1], uProxy.get())
            , fUSampler(std::move(uProxy), uvFilterMode)
            , fVSampler(vProxy, uvFilterMode)
            , fColorSpace(colorSpace)
            , fNV12(nv12) {
        this->initClassID<YUVtoRGBEffect>();
        this->addCoordTransform(&fYTransform);
        this->addTextureSampler(&fYSampler);
        this->addCoordTransform(&fUTransform);
        this->addTextureSampler(&fUSampler);
        if (!fNV12) {
            fVTransform = GrCoordTransform(yuvMatrix[2], vProxy.get());
            this->addCoordTransform(&fVTransform);
            this->addTextureSampler(&fVSampler);
        }
    }

    YUVtoRGBEffect(const YUVtoRGBEffect& that)
            : INHERITED(kPreservesOpaqueInput_OptimizationFlag)
            , fYTransform(that.fYTransform)
            , fYSampler(that.fYSampler)
            , fUTransform(that.fUTransform)
            , fUSampler(that.fUSampler)
            , fVTransform(that.fVTransform)
            , fVSampler(that.fVSampler)
            , fColorSpace(that.fColorSpace)
            , fNV12(that.fNV12) {
        this->initClassID<YUVtoRGBEffect>();
        this->addCoordTransform(&fYTransform);
        this->addTextureSampler(&fYSampler);
        this->addCoordTransform(&fUTransform);
        this->addTextureSampler(&fUSampler);
        if (!fNV12) {
            this->addCoordTransform(&fVTransform);
            this->addTextureSampler(&fVSampler);
        }
    }

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override {
        return new GLSLProcessor;
    }

    void onGetGLSLProcessorKey(const GrShaderCaps& caps, GrProcessorKeyBuilder* b) const override {
        b->add32(fNV12);
    }

    bool onIsEqual(const GrFragmentProcessor& sBase) const override {
        const YUVtoRGBEffect& s = sBase.cast<YUVtoRGBEffect>();
        return (fColorSpace == s.getColorSpace()) && (fNV12 == s.isNV12());
    }

    GrCoordTransform fYTransform;
    TextureSampler   fYSampler;
    GrCoordTransform fUTransform;
    TextureSampler   fUSampler;
    GrCoordTransform fVTransform;
    TextureSampler   fVSampler;
    SkYUVColorSpace fColorSpace;
    bool fNV12;

    typedef GrFragmentProcessor INHERITED;
};

}

//////////////////////////////////////////////////////////////////////////////

std::unique_ptr<GrFragmentProcessor> GrYUVEffect::MakeYUVToRGB(
        sk_sp<GrTextureProxy> yProxy, sk_sp<GrTextureProxy> uProxy, sk_sp<GrTextureProxy> vProxy,
        const SkISize sizes[3], SkYUVColorSpace colorSpace, bool nv12) {
    SkASSERT(yProxy && uProxy && vProxy && sizes);
    return YUVtoRGBEffect::Make(std::move(yProxy), std::move(uProxy), std::move(vProxy),
                                sizes, colorSpace, nv12);
}
