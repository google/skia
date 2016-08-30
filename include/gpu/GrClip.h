/*
 * Copyright 2010 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrClip_DEFINED
#define GrClip_DEFINED

#include "SkRect.h"
#include "SkRRect.h"

class GrAppliedClip;
class GrContext;
class GrDrawContext;

/**
 * GrClip is an abstract base class for applying a clip. It constructs a clip mask if necessary, and
 * fills out a GrAppliedClip instructing the caller on how to set up the draw state.
 */
class GrClip {
public:
    virtual bool quickContains(const SkRect&) const = 0;
    virtual bool quickContains(const SkRRect& rrect) const {
        return this->quickContains(rrect.getBounds());
    }
    virtual void getConservativeBounds(int width, int height, SkIRect* devResult,
                                       bool* isIntersectionOfRects = nullptr) const = 0;
    virtual bool apply(GrContext*, GrDrawContext*, bool useHWAA, bool hasUserStencilSettings,
                       GrAppliedClip* out) const = 0;

    virtual ~GrClip() {}

    /**
     * This is the maximum distance that a draw may extend beyond a clip's boundary and still count
     * count as "on the other side". We leave some slack because floating point rounding error is
     * likely to blame. The rationale for 1e-3 is that in the coverage case (and barring unexpected
     * rounding), as long as coverage stays within 0.5 * 1/256 of its intended value it shouldn't
     * have any effect on the final pixel values.
     */
    constexpr static SkScalar kBoundsTolerance = 1e-3f;

    /**
     * Returns true if the given query bounds count as entirely inside the clip.
     *
     * @param innerClipBounds   device-space rect contained by the clip (SkRect or SkIRect).
     * @param queryBounds       device-space bounds of the query region.
     */
    template<typename TRect> constexpr static bool IsInsideClip(const TRect& innerClipBounds,
                                                                const SkRect& queryBounds) {
        return innerClipBounds.fRight - innerClipBounds.fLeft > kBoundsTolerance &&
               innerClipBounds.fBottom - innerClipBounds.fTop > kBoundsTolerance &&
               innerClipBounds.fLeft < queryBounds.fLeft + kBoundsTolerance &&
               innerClipBounds.fTop < queryBounds.fTop + kBoundsTolerance &&
               innerClipBounds.fRight > queryBounds.fRight - kBoundsTolerance &&
               innerClipBounds.fBottom > queryBounds.fBottom - kBoundsTolerance;
    }

    /**
     * Returns true if the given query bounds count as entirely outside the clip.
     *
     * @param outerClipBounds   device-space rect that contains the clip (SkRect or SkIRect).
     * @param queryBounds       device-space bounds of the query region.
     */
    template<typename TRect> constexpr static bool IsOutsideClip(const TRect& outerClipBounds,
                                                                 const SkRect& queryBounds) {
        return outerClipBounds.fRight - outerClipBounds.fLeft <= kBoundsTolerance ||
               outerClipBounds.fBottom - outerClipBounds.fTop <= kBoundsTolerance ||
               outerClipBounds.fLeft >= queryBounds.fRight - kBoundsTolerance ||
               outerClipBounds.fTop >= queryBounds.fBottom - kBoundsTolerance ||
               outerClipBounds.fRight <= queryBounds.fLeft + kBoundsTolerance ||
               outerClipBounds.fBottom <= queryBounds.fTop + kBoundsTolerance;
    }

    /**
     * Returns the minimal integer rect that counts as containing a given set of bounds.
     */
    static SkIRect GetPixelIBounds(const SkRect& bounds) {
        return SkIRect::MakeLTRB(SkScalarFloorToInt(bounds.fLeft + kBoundsTolerance),
                                 SkScalarFloorToInt(bounds.fTop + kBoundsTolerance),
                                 SkScalarCeilToInt(bounds.fRight - kBoundsTolerance),
                                 SkScalarCeilToInt(bounds.fBottom - kBoundsTolerance));
    }

    /**
     * Returns the minimal pixel-aligned rect that counts as containing a given set of bounds.
     */
    static SkRect GetPixelBounds(const SkRect& bounds) {
        return SkRect::MakeLTRB(SkScalarFloorToScalar(bounds.fLeft + kBoundsTolerance),
                                SkScalarFloorToScalar(bounds.fTop + kBoundsTolerance),
                                SkScalarCeilToScalar(bounds.fRight - kBoundsTolerance),
                                SkScalarCeilToScalar(bounds.fBottom - kBoundsTolerance));
    }

    /**
     * Returns true if the given rect counts as aligned with pixel boundaries.
     */
    static bool IsPixelAligned(const SkRect& rect) {
        return SkScalarAbs(SkScalarRoundToScalar(rect.fLeft) - rect.fLeft) <= kBoundsTolerance &&
               SkScalarAbs(SkScalarRoundToScalar(rect.fTop) - rect.fTop) <= kBoundsTolerance &&
               SkScalarAbs(SkScalarRoundToScalar(rect.fRight) - rect.fRight) <= kBoundsTolerance &&
               SkScalarAbs(SkScalarRoundToScalar(rect.fBottom) - rect.fBottom) <= kBoundsTolerance;
    }
};

/**
 * Specialized implementation for no clip.
 */
class GrNoClip final : public GrClip {
private:
    bool quickContains(const SkRect&) const final {
        return true;
    }
    void getConservativeBounds(int width, int height, SkIRect* devResult,
                               bool* isIntersectionOfRects) const final {
        devResult->setXYWH(0, 0, width, height);
        if (isIntersectionOfRects) {
            *isIntersectionOfRects = true;
        }
    }
    bool apply(GrContext*, GrDrawContext*, bool, bool, GrAppliedClip*) const final {
        return true;
    }
};

#endif
