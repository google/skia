/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrSingleTextureEffect_DEFINED
#define GrSingleTextureEffect_DEFINED

#include "GrFragmentProcessor.h"
#include "GrColorSpaceXform.h"
#include "GrCoordTransform.h"
#include "GrInvariantOutput.h"
#include "SkMatrix.h"

class GrTexture;
class GrTextureProxy;

/**
 * A base class for effects that draw a single texture with a texture matrix. This effect has no
 * backend implementations. One must be provided by the subclass.
 */
class GrSingleTextureEffect : public GrFragmentProcessor {
public:
    ~GrSingleTextureEffect() override;

    SkString dumpInfo() const override {
        SkString str;
        str.appendf("Texture: %d", fTextureSampler.texture()->uniqueID().asUInt());
        return str;
    }

    GrColorSpaceXform* colorSpaceXform() const { return fColorSpaceXform.get(); }

protected:
    /** unfiltered, clamp mode */
    GrSingleTextureEffect(GrTexture*, sk_sp<GrColorSpaceXform>, const SkMatrix&);
    /** clamp mode */
    GrSingleTextureEffect(GrTexture*, sk_sp<GrColorSpaceXform>, const SkMatrix&,
                          GrSamplerParams::FilterMode filterMode);
    GrSingleTextureEffect(GrTexture*,
                          sk_sp<GrColorSpaceXform>,
                          const SkMatrix&,
                          const GrSamplerParams&);

    /** unfiltered, clamp mode */
    GrSingleTextureEffect(GrContext*,
                          sk_sp<GrTextureProxy>, sk_sp<GrColorSpaceXform>, const SkMatrix&);
    /** clamp mode */
    GrSingleTextureEffect(GrContext*,
                          sk_sp<GrTextureProxy>, sk_sp<GrColorSpaceXform>, const SkMatrix&,
                          GrSamplerParams::FilterMode filterMode);
    GrSingleTextureEffect(GrContext*,
                          sk_sp<GrTextureProxy>, sk_sp<GrColorSpaceXform>, const SkMatrix&,
                          const GrSamplerParams&);

    /**
     * Can be used as a helper to implement subclass onComputeInvariantOutput(). It assumes that
     * the subclass output color will be a modulation of the input color with a value read from the
     * texture.
     */
    void updateInvariantOutputForModulation(GrInvariantOutput* inout) const {
        GrPixelConfig config = this->textureSampler(0).texture()->config();
        if (GrPixelConfigIsAlphaOnly(config)) {
            inout->mulByUnknownSingleComponent();
        } else if (GrPixelConfigIsOpaque(config)) {
            inout->mulByUnknownOpaqueFourComponents();
        } else {
            inout->mulByUnknownFourComponents();
        }
    }

private:
    GrCoordTransform fCoordTransform;
    TextureSampler fTextureSampler;
    sk_sp<GrColorSpaceXform> fColorSpaceXform;

    typedef GrFragmentProcessor INHERITED;
};

#endif
