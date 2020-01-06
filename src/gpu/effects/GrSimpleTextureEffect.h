/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSimpleTextureEffect_DEFINED
#define GrSimpleTextureEffect_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "src/gpu/GrCoordTransform.h"
#include "src/gpu/GrFragmentProcessor.h"

class GrSimpleTextureEffect : public GrFragmentProcessor {
public:
    /** Uses kClamp wrap mode in both dimensions. */
    static std::unique_ptr<GrFragmentProcessor> Make(
            sk_sp<GrSurfaceProxy>,
            SkAlphaType,
            const SkMatrix&,
            GrSamplerState::Filter = GrSamplerState::Filter::kNearest);

    /** Allows full specification of the sampling parameters. */
    static std::unique_ptr<GrFragmentProcessor> Make(sk_sp<GrSurfaceProxy>,
                                                     SkAlphaType,
                                                     const SkMatrix&,
                                                     GrSamplerState);

    std::unique_ptr<GrFragmentProcessor> clone() const override;

    const char* name() const override { return "SimpleTextureEffect"; }

private:
    GrCoordTransform fCoordTransform;
    TextureSampler fSampler;

    GrSimpleTextureEffect(const GrSimpleTextureEffect& src);

    inline GrSimpleTextureEffect(sk_sp<GrSurfaceProxy>, SkAlphaType, const SkMatrix&,
                                 GrSamplerState);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    const TextureSampler& onTextureSampler(int) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    typedef GrFragmentProcessor INHERITED;
};
#endif
