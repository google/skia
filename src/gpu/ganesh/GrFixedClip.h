/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrFixedClip_DEFINED
#define GrFixedClip_DEFINED

#include "include/core/SkRect.h"
#include "include/private/base/SkAssert.h"
#include "src/gpu/ganesh/GrClip.h"
#include "src/gpu/ganesh/GrScissorState.h"
#include "src/gpu/ganesh/GrWindowRectsState.h"

class GrAppliedHardClip;
class GrWindowRectangles;
enum class GrAA : bool;
struct SkISize;

/**
 * Implements GrHardClip with scissor and window rectangles.
 */
class GrFixedClip final : public GrHardClip {
public:
    explicit GrFixedClip(const SkISize& rtDims) : fScissorState(rtDims) {}
    GrFixedClip(const SkISize& rtDims, const SkIRect& scissorRect)
            : GrFixedClip(rtDims) {
        SkAssertResult(fScissorState.set(scissorRect));
    }

    const GrScissorState& scissorState() const { return fScissorState; }
    bool scissorEnabled() const { return fScissorState.enabled(); }
    // Returns the scissor rect or rt bounds if the scissor test is not enabled.
    const SkIRect& scissorRect() const { return fScissorState.rect(); }

    void disableScissor() { fScissorState.setDisabled(); }

    [[nodiscard]] bool setScissor(const SkIRect& irect) {
        return fScissorState.set(irect);
    }
    [[nodiscard]] bool intersect(const SkIRect& irect) {
        return fScissorState.intersect(irect);
    }

    const GrWindowRectsState& windowRectsState() const { return fWindowRectsState; }
    bool hasWindowRectangles() const { return fWindowRectsState.enabled(); }

    void disableWindowRectangles() { fWindowRectsState.setDisabled(); }

    void setWindowRectangles(const GrWindowRectangles& windows, GrWindowRectsState::Mode mode) {
        fWindowRectsState.set(windows, mode);
    }

    SkIRect getConservativeBounds() const final;
    Effect apply(GrAppliedHardClip*, SkIRect*) const final;
    PreClipResult preApply(const SkRect& drawBounds, GrAA aa) const final;

private:
    GrScissorState       fScissorState;
    GrWindowRectsState   fWindowRectsState;
};

#endif
