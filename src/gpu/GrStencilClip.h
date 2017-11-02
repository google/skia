/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrStencilClip_DEFINED
#define GrStencilClip_DEFINED

#include "GrAppliedClip.h"
#include "GrFixedClip.h"

/**
 * GrStencilClip is an implementation of GrHardClip that uses fixed function state and stencil.
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

private:
    bool quickContains(const SkRect& rect) const override {
        return this->hasStencilClip() ? false : fFixedClip.quickContains(rect);
    }
    void getConservativeBounds(int width, int height, SkIRect* bounds, bool* iior) const override {
        fFixedClip.getConservativeBounds(width, height, bounds, iior);
    }
    bool isRRect(const SkRect& rtBounds, SkRRect* rr, GrAA* aa) const override {
        return this->hasStencilClip() ? false : fFixedClip.isRRect(rtBounds, rr, aa);
    }
    bool onApply(int rtWidth, int rtHeight, GrAppliedClip* out, SkRect* bounds) const override {
        if (!fFixedClip.apply(rtWidth, rtHeight, out, bounds)) {
            return false;
        }
        if (this->hasStencilClip()) {
            out->addStencilClip(fStencilStackID);
        }
        return true;
    }

    GrFixedClip   fFixedClip;
    uint32_t      fStencilStackID;

    typedef GrClip INHERITED;
};

#endif
