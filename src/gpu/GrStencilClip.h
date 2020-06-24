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

    bool quickContains(const SkRect& rect) const override {
        return !this->hasStencilClip() && fFixedClip.quickContains(rect);
    }
    SkIRect getConservativeBounds() const override {
        return fFixedClip.getConservativeBounds();
    }
    bool isRRect(SkRRect* rr, GrAA* aa) const override {
        return !this->hasStencilClip() && fFixedClip.isRRect(rr, aa);
    }
    bool apply(GrAppliedHardClip* out, SkRect* bounds) const override {
        if (!fFixedClip.apply(out, bounds)) {
            return false;
        }
        if (this->hasStencilClip()) {
            out->addStencilClip(fStencilStackID);
        }
        return true;
    }

private:
    GrFixedClip   fFixedClip;
    uint32_t      fStencilStackID;

    typedef GrClip INHERITED;
};

#endif
