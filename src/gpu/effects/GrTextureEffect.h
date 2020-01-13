/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrTextureEffect_DEFINED
#define GrTextureEffect_DEFINED

#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "src/gpu/GrCoordTransform.h"
#include "src/gpu/GrFragmentProcessor.h"

class GrTextureEffect : public GrFragmentProcessor {
public:
    static std::unique_ptr<GrFragmentProcessor> Make(sk_sp<GrSurfaceProxy>,
                                                     SkAlphaType,
                                                     const SkMatrix& = SkMatrix::I(),
                                                     GrSamplerState = {});

    std::unique_ptr<GrFragmentProcessor> clone() const override;

    const char* name() const override { return "SimpleTextureEffect"; }

private:
    GrCoordTransform fCoordTransform;
    TextureSampler fSampler;

    GrTextureEffect(const GrTextureEffect& src);

    inline GrTextureEffect(sk_sp<GrSurfaceProxy>, SkAlphaType, const SkMatrix&, GrSamplerState);

    GrGLSLFragmentProcessor* onCreateGLSLInstance() const override;

    void onGetGLSLProcessorKey(const GrShaderCaps&, GrProcessorKeyBuilder*) const override;

    bool onIsEqual(const GrFragmentProcessor&) const override;

    const TextureSampler& onTextureSampler(int) const override;

    GR_DECLARE_FRAGMENT_PROCESSOR_TEST

    typedef GrFragmentProcessor INHERITED;
};
#endif
