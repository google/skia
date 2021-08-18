/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef StencilClip_DEFINED
#define StencilClip_DEFINED

#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrFixedClip.h"

namespace skgpu::v1 {

/**
 * Implements GrHardClip with the currently-existing stencil buffer contents and GrFixedClip.
 */
class StencilClip final : public GrHardClip {
public:
    explicit StencilClip(const SkISize& rtDims, uint32_t stencilStackID = SK_InvalidGenID)
            : fFixedClip(rtDims)
            , fStencilStackID(stencilStackID) {}

    StencilClip(const SkISize& rtDims,
                const SkIRect& scissorRect,
                uint32_t stencilStackID = SK_InvalidGenID)
            : fFixedClip(rtDims, scissorRect)
            , fStencilStackID(stencilStackID) {}

    const GrFixedClip& fixedClip() const { return fFixedClip; }
    GrFixedClip& fixedClip() { return fFixedClip; }

    uint32_t stencilStackID() const { return fStencilStackID; }
    bool hasStencilClip() const { return SK_InvalidGenID != fStencilStackID; }
    void setStencilClip(uint32_t stencilStackID) { fStencilStackID = stencilStackID; }

    SkIRect getConservativeBounds() const final {
        return fFixedClip.getConservativeBounds();
    }

    Effect apply(GrAppliedHardClip* out, SkIRect* bounds) const final {
        Effect effect = fFixedClip.apply(out, bounds);
        if (effect == Effect::kClippedOut) {
            // Stencil won't bring back coverage
            return Effect::kClippedOut;
        }
        if (this->hasStencilClip()) {
            out->addStencilClip(fStencilStackID);
            effect = Effect::kClipped;
        }
        return effect;
    }

    PreClipResult preApply(const SkRect& drawBounds, GrAA aa) const final {
        if (this->hasStencilClip()) {
            return this->INHERITED::preApply(drawBounds, aa);
        } else {
            return fFixedClip.preApply(drawBounds, aa);
        }
    }

private:
    GrFixedClip fFixedClip;
    uint32_t    fStencilStackID;

    using INHERITED = GrClip;
};

} // namespace skgpu::v1

#endif // StencilClip_DEFINED
