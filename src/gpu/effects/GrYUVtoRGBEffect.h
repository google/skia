/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrYUVtoRGBEffect_DEFINED
#define GrYUVtoRGBEffect_DEFINED

#include "include/core/SkTypes.h"

#include "src/gpu/GrCoordTransform.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/effects/GrTextureDomain.h"

#include "include/core/SkYUVAIndex.h"

class GrYUVtoRGBEffect : public GrFragmentProcessor {
public:
    // The domain supported by this effect is more limited than the general GrTextureDomain due
    // to the multi-planar, varying resolution images that it has to sample. If 'domain' is provided
    // it is the Y plane's domain. This will automatically inset for bilinear filtering, and only
    // the clamp wrap mode is supported.
    static std::unique_ptr<GrFragmentProcessor> Make(const sk_sp<GrTextureProxy> proxies[],
                                                     const SkYUVAIndex indices[4],
                                                     SkYUVColorSpace yuvColorSpace,
                                                     GrSamplerState::Filter filterMode,
                                                     const SkMatrix& localMatrix = SkMatrix::I(),
                                                     const SkRect* domain = nullptr);
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
                     const SkYUVAIndex yuvaIndices[4], SkYUVColorSpace yuvColorSpace,
                     const SkMatrix& localMatrix, const SkRect* domain)
            : INHERITED(kGrYUVtoRGBEffect_ClassID, kNone_OptimizationFlags)
            , fDomains{GrTextureDomain::IgnoredDomain(), GrTextureDomain::IgnoredDomain(),
                       GrTextureDomain::IgnoredDomain(), GrTextureDomain::IgnoredDomain()}
            , fYUVColorSpace(yuvColorSpace) {
        for (int i = 0; i < numPlanes; ++i) {
            SkMatrix planeMatrix = SkMatrix::MakeScale(scales[i].width(), scales[i].height());
            if (domain) {
                SkASSERT(filterModes[i] != GrSamplerState::Filter::kMipMap);

                SkRect scaledDomain = planeMatrix.mapRect(*domain);
                if (filterModes[i] != GrSamplerState::Filter::kNearest) {
                    // Inset by half a pixel for bilerp, after scaling to the size of the plane
                    scaledDomain.inset(0.5f, 0.5f);
                }

                fDomains[i] = GrTextureDomain(proxies[i].get(), scaledDomain,
                        GrTextureDomain::kClamp_Mode, GrTextureDomain::kClamp_Mode, i);
            }

            planeMatrix.postConcat(localMatrix);
            fSamplers[i].reset(std::move(proxies[i]),
                               GrSamplerState(GrSamplerState::WrapMode::kClamp, filterModes[i]));
            fSamplerTransforms[i] = planeMatrix;
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
    GrTextureDomain  fDomains[4];
    SkYUVAIndex      fYUVAIndices[4];
    SkYUVColorSpace  fYUVColorSpace;

    typedef GrFragmentProcessor INHERITED;
};
#endif
