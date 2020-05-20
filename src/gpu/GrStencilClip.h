/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStencilClip_DEFINED
#define GrStencilClip_DEFINED

#include "src/gpu/GrAppliedClip.h"
#include "src/gpu/GrFixedClip.h"

/**
 * Implements GrHardClip with the currently-existing stencil buffer contents and GrFixedClip.
 */
class GrStencilClip final : public GrHardClip {
public:
    explicit GrStencilClip(const SkISize& rtDims, uint32_t stencilStackID = SK_InvalidGenID)
            : fFixedClip(rtDims)
            , fStencilStackID(stencilStackID) {}

    GrStencilClip(const SkISize& rtDims, const SkIRect& scissorRect,
                  uint32_t stencilStackID = SK_InvalidGenID)
            : fFixedClip(rtDims, scissorRect)
            , fStencilStackID(stencilStackID) {}

    const GrFixedClip& fixedClip() const { return fFixedClip; }
    GrFixedClip& fixedClip() { return fFixedClip; }

    uint32_t stencilStackID() const { return fStencilStackID; }
    bool hasStencilClip() const { return SK_InvalidGenID != fStencilStackID; }
    void setStencilClip(uint32_t stencilStackID) { fStencilStackID = stencilStackID; }

    SkIRect getConservativeBounds() const override {
        return fFixedClip.getConservativeBounds();
    }

    ClipEffect apply(GrAppliedHardClip* out, SkRect* bounds) const override {
        ClipEffect effect = fFixedClip.apply(out, bounds);
        if (effect == ClipEffect::kNoDraw) {
            // Stencil won't bring back coverage
            return ClipEffect::kNoDraw;
        }
        if (this->hasStencilClip()) {
            out->addStencilClip(fStencilStackID);
            effect = ClipEffect::kClipped;
        }
        return effect;
    }

    bool preApply(const SkRect& drawBounds, ClipEffect* effect,
                  SkRRect* rrect, GrAA* aa) const override {
        if (this->hasStencilClip()) {
            // Report kNoDraw if it's offscreen, but cannot report kUnclipped due to stencil
            if (!drawBounds.intersects(SkRect::Make(fFixedClip.scissorRect()))) {
                *effect = ClipEffect::kNoDraw;
            } else {
                *effect = ClipEffect::kClipped;
            }
            return false;
        } else {
            return fFixedClip.preApply(drawBounds, effect, rrect, aa);
        }
    }

private:
    GrFixedClip   fFixedClip;
    uint32_t      fStencilStackID;

    typedef GrClip INHERITED;
};

#endif
