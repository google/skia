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
 * GrFixedClip is a clip that gets implemented by fixed-function hardware.
 */
class GrFixedClip final : public GrClip {
public:
    GrFixedClip() = default;
    explicit GrFixedClip(const SkIRect& scissorRect) : fScissorState(scissorRect) {}

    const GrScissorState& scissorState() const { return fScissorState; }
    bool scissorEnabled() const { return fScissorState.enabled(); }
    const SkIRect& scissorRect() const { SkASSERT(scissorEnabled()); return fScissorState.rect(); }

    void disableScissor() { fScissorState.setDisabled(); }

    bool SK_WARN_UNUSED_RESULT intersect(const SkIRect& irect) {
        return fScissorState.intersect(irect);
    }

    bool quickContains(const SkRect& rect) const final {
        return !fScissorState.enabled() || GrClip::IsInsideClip(fScissorState.rect(), rect);
    }
    void getConservativeBounds(int width, int height, SkIRect* devResult,
                               bool* isIntersectionOfRects) const final;

    bool isRRect(const SkRect& rtBounds, SkRRect* rr, bool* aa) const override {
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

    bool apply(GrContext*, GrDrawContext*, bool useHWAA, bool hasUserStencilSettings,
               GrAppliedClip* out) const final;

    static const GrFixedClip& Disabled();

private:
    GrScissorState   fScissorState;
};

#endif
