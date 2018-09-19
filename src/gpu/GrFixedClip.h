/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFixedClip_DEFINED
#define GrFixedClip_DEFINED

#include "src/gpu/GrClip.h"
#include "src/gpu/GrScissorState.h"
#include "src/gpu/GrWindowRectsState.h"

/**
 * Implements GrHardClip with scissor and window rectangles.
 */
class GrFixedClip final : public GrHardClip {
public:
    GrFixedClip() = default;
    explicit GrFixedClip(const SkIRect& scissorRect) : fScissorState(scissorRect) {}

    const GrScissorState& scissorState() const { return fScissorState; }
    bool scissorEnabled() const { return fScissorState.enabled(); }
    const SkIRect& scissorRect() const { SkASSERT(scissorEnabled()); return fScissorState.rect(); }

    void disableScissor() { fScissorState.setDisabled(); }

    void setScissor(const SkIRect& irect) {
        fScissorState.set(irect);
    }
    bool SK_WARN_UNUSED_RESULT intersect(const SkIRect& irect) {
        return fScissorState.intersect(irect);
    }

    const GrWindowRectsState& windowRectsState() const { return fWindowRectsState; }
    bool hasWindowRectangles() const { return fWindowRectsState.enabled(); }

    void disableWindowRectangles() { fWindowRectsState.setDisabled(); }

    void setWindowRectangles(const GrWindowRectangles& windows, GrWindowRectsState::Mode mode) {
        fWindowRectsState.set(windows, mode);
    }

    bool quickContains(const SkRect&) const override;
    void getConservativeBounds(int w, int h, SkIRect* devResult, bool* iior) const override;
    bool isRRect(const SkRect& rtBounds, SkRRect* rr, GrAA*) const override;
    bool apply(int rtWidth, int rtHeight, GrAppliedHardClip*, SkRect*) const override;

    static const GrFixedClip& Disabled();

private:
    GrScissorState       fScissorState;
    GrWindowRectsState   fWindowRectsState;
};

#endif
