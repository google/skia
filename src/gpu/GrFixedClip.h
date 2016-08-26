/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFixedClip_DEFINED
#define GrFixedClip_DEFINED

#include "GrClip.h"
#include "GrTypesPriv.h"

/**
 * GrFixedClip is a clip that can be represented by fixed-function hardware. It never modifies the
 * stencil buffer itself, but can be configured to use whatever clip is already there.
 */
class GrFixedClip final : public GrClip {
public:
    GrFixedClip() : fHasStencilClip(false) {}
    GrFixedClip(const SkIRect& scissorRect)
        : fScissorState(scissorRect)
        , fHasStencilClip(false) {}

    void reset() {
        fScissorState.setDisabled();
        fHasStencilClip = false;
    }

    void reset(const SkIRect& scissorRect) {
        fScissorState.set(scissorRect);
        fHasStencilClip = false;
    }

    void enableStencilClip() { fHasStencilClip = true; }
    void disableStencilClip() { fHasStencilClip = false; }

    bool quickContains(const SkRect&) const final;
    void getConservativeBounds(int width, int height, SkIRect* devResult,
                               bool* isIntersectionOfRects) const final;

    bool isRRect(const SkRect& rtBounds, SkRRect* rr, bool* aa) const override {
        if (fHasStencilClip) {
            return false;
        }
        if (fScissorState.enabled()) {
            SkRect rect = SkRect::Make(fScissorState.rect());
            if (!rect.intersects(rtBounds)) {
                return false;
            }
            rr->setRect(rect);
            *aa = false;
            return true;
        }
        return false;
    };

private:
    bool apply(GrContext*, GrDrawContext*, bool useHWAA, bool hasUserStencilSettings,
               GrAppliedClip* out) const final;

    GrScissorState   fScissorState;
    bool             fHasStencilClip;
};

#endif
