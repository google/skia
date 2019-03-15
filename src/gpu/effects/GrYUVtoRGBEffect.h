/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrYUVtoRGBEffect_DEFINED
#define GrYUVtoRGBEffect_DEFINED

#include "SkTypes.h"

#include "GrFragmentProcessor.h"
#include "GrCoordTransform.h"

#include "SkYUVAIndex.h"

class GrYUVtoRGBEffect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(const sk_sp<GrTextureProxy> proxies[],
                                                     const SkYUVAIndex indices[4],
                                                     SkYUVColorSpace yuvColorSpace,
                                                     GrSamplerState::Filter filterMode);
#ifdef SK_DEBUG
    SkString dumpInfo() const override;
#endif

    SkYUVColorSpace yuvColorSpace() const { return fYUVColorSpace; }
    const SkYUVAIndex& yuvaIndex(int i) const { return fYUVAIndices[i]; }

    GrYUVtoRGBEffect(const GrYUVtoRGBEffect& src);
    std::unique_ptr<GrFragmentProcessor> clone() const override;
    const char* name() const override { return "YUVtoRGBEffect"; }

private:
    GrYUVtoRGBEffect(const sk_sp<GrTextureProxy> proxies[], const SkSize scales[],
                     const GrSamplerState::Filter filterModes[], int numPlanes,
                     const SkYUVAIndex yuvaIndices[4], SkYUVColorSpace yuvColorSpace)
            : INHERITED(kGrYUVtoRGBEffect_ClassID, kNone_OptimizationFlags)
            , fYUVColorSpace(yuvColorSpace) {
        for (int i = 0; i < numPlanes; ++i) {
            fSamplers[i].reset(std::move(proxies[i]),
                               GrSamplerState(GrSamplerState::WrapMode::kClamp, filterModes[i]));
            fSamplerTransforms[i] = SkMatrix::MakeScale(scales[i].width(), scales[i].height());
            fSamplerCoordTransforms[i] =
                    GrCoordTransform(fSamplerTransforms[i], fSamplers[i].proxy());
        }

        this->setTextureSamplerCnt(numPlanes);
        for (int i = 0; i < numPlanes; ++i) {
            this->addCoordTransform(&fSamplerCoordTransforms[i]);
        }

        memcpy(fYUVAIndices, yuvaIndices, sizeof(fYUVAIndices));
    }
    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;
    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;
    bool onIsEqual(const GrFragmentProcessor&) const override;
    const TextureSampler& onTextureSampler(int) const override;
    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    TextureSampler   fSamplers[4];
    SkMatrix44       fSamplerTransforms[4];
    GrCoordTransform fSamplerCoordTransforms[4];
    SkYUVAIndex      fYUVAIndices[4];
    SkYUVColorSpace  fYUVColorSpace;

    typedef GrFragmentProcessor INHERITED;
};
#endif
