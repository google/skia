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

class GrTextureProxy;
class SkMatrix;

/**
 * A base class for effects that draw a single texture with a texture matrix. This effect has no
 * backend implementations. One must be provided by the subclass.
 */
class GrSingleTextureEffect : public GrFragmentProcessor {
public:
    SkString dumpInfo() const override {
        SkString str;
        str.appendf("Texture: %d", fTextureSampler.proxy()->uniqueID().asUInt());
        return str;
    }

    GrColorSpaceXform* colorSpaceXform() const { return fColorSpaceXform.get(); }

protected:
    /** unfiltered, clamp mode */
    GrSingleTextureEffect(OptimizationFlags, sk_sp<GrTextureProxy>,
                          sk_sp<GrColorSpaceXform>, const SkMatrix&);
    /** clamp mode */
    GrSingleTextureEffect(OptimizationFlags, sk_sp<GrTextureProxy>,
                          sk_sp<GrColorSpaceXform>, const SkMatrix&,
                          GrSamplerParams::FilterMode filterMode);
    GrSingleTextureEffect(OptimizationFlags, sk_sp<GrTextureProxy>,
                          sk_sp<GrColorSpaceXform>, const SkMatrix&, const GrSamplerParams&);

    /**
     * Can be used as a helper to decide which fragment processor OptimizationFlags should be set.
     * This assumes that the subclass output color will be a modulation of the input color with a
     * value read from the texture and that the texture contains premultiplied color or alpha values
     * that are in range.
     */
    static OptimizationFlags ModulationFlags(GrPixelConfig config) {
        if (GrPixelConfigIsOpaque(config)) {
            return kCompatibleWithCoverageAsAlpha_OptimizationFlag |
                   kPreservesOpaqueInput_OptimizationFlag;
        } else {
            return kCompatibleWithCoverageAsAlpha_OptimizationFlag;
        }
    }

private:
    GrCoordTransform         fCoordTransform;
    TextureSampler           fTextureSampler;
    sk_sp<GrColorSpaceXform> fColorSpaceXform;

    typedef GrFragmentProcessor INHERITED;
};

#endif
