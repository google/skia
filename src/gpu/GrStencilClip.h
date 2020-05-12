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
    GrStencilClip(uint32_t stencilStackID = SK_InvalidGenID) : fStencilStackID(stencilStackID) {}

    explicit GrStencilClip(const SkIRect& scissorRect, uint32_t stencilStackID = SK_InvalidGenID)
        : fFixedClip(scissorRect)
        , fStencilStackID(stencilStackID) {
    }

    const GrFixedClip& fixedClip() const { return fFixedClip; }
    GrFixedClip& fixedClip() { return fFixedClip; }

    bool stencilStackID() const { return fStencilStackID; }
    bool hasStencilClip() const { return SK_InvalidGenID != fStencilStackID; }
    void setStencilClip(uint32_t stencilStackID) { fStencilStackID = stencilStackID; }

    bool quickContains(const SkRect& rect) const override {
        return !this->hasStencilClip() && fFixedClip.quickContains(rect);
    }
    void getConservativeBounds(int width, int height, SkIRect* bounds, bool* iior) const override {
        fFixedClip.getConservativeBounds(width, height, bounds, iior);
    }
    bool isRRect(const SkRect& rtBounds, SkRRect* rr, GrAA* aa) const override {
        return !this->hasStencilClip() && fFixedClip.isRRect(rtBounds, rr, aa);
    }
    bool apply(int rtWidth, int rtHeight, GrAppliedHardClip* out, SkRect* bounds) const override {
        if (!fFixedClip.apply(rtWidth, rtHeight, out, bounds)) {
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
