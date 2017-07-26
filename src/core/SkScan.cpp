/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkScan.h"
#include "SkBlitter.h"
#include "SkPath.h"
#include "SkRasterClip.h"

#ifdef SK_NO_ANALYTIC_AA
    std::atomic<bool> gSkUseAnalyticAA{false};
#else
    std::atomic<bool> gSkUseAnalyticAA{true};
#endif

std::atomic<bool> gSkForceAnalyticAA{false};

std::atomic<bool> gSkUseDeltaAA{false};

std::atomic<bool> gSkForceDeltaAA{false};

bool SkScan::ShouldUseDAA(const SkPath& path) {
    if (gSkForceDeltaAA) {
        return true;
    }
    if (!gSkUseDeltaAA) {
        return false;
    }
    const SkRect& bounds = path.getBounds();
    return !path.isConvex() && path.countPoints() >= SkTMax(bounds.width(), bounds.height()) / 8;
}

bool SkScan::ShouldUseAAA(const SkPath& path) {
    if (gSkForceAnalyticAA) {
        return true;
    }
    if (!gSkUseAnalyticAA) {
        return false;
    }
    if (path.isRect(nullptr)) {
        return true;
    }
    const SkRect& bounds = path.getBounds();
    // When the path have so many points compared to the size of its bounds/resolution,
    // it indicates that the path is not quite smooth in the current resolution:
    // the expected number of turning points in every pixel row/column is significantly greater than
    // zero. Hence Aanlytic AA is not likely to produce visible quality improvements, and Analytic
    // AA might be slower than supersampling.
    return path.countPoints() < SkTMax(bounds.width(), bounds.height()) / 2 - 10;
}

static inline void blitrect(SkBlitter* blitter, const SkIRect& r) {
    blitter->blitRect(r.fLeft, r.fTop, r.width(), r.height());
}

void SkScan::FillIRect(const SkIRect& r, const SkRegion* clip,
                       SkBlitter* blitter) {
    if (!r.isEmpty()) {
        if (clip) {
            if (clip->isRect()) {
                const SkIRect& clipBounds = clip->getBounds();

                if (clipBounds.contains(r)) {
                    blitrect(blitter, r);
                } else {
                    SkIRect rr = r;
                    if (rr.intersect(clipBounds)) {
                        blitrect(blitter, rr);
                    }
                }
            } else {
                SkRegion::Cliperator    cliper(*clip, r);
                const SkIRect&          rr = cliper.rect();

                while (!cliper.done()) {
                    blitrect(blitter, rr);
                    cliper.next();
                }
            }
        } else {
            blitrect(blitter, r);
        }
    }
}

void SkScan::FillXRect(const SkXRect& xr, const SkRegion* clip,
                       SkBlitter* blitter) {
    SkIRect r;

    XRect_round(xr, &r);
    SkScan::FillIRect(r, clip, blitter);
}

void SkScan::FillRect(const SkRect& r, const SkRegion* clip,
                       SkBlitter* blitter) {
    SkIRect ir;

    r.round(&ir);
    SkScan::FillIRect(ir, clip, blitter);
}

///////////////////////////////////////////////////////////////////////////////

void SkScan::FillIRect(const SkIRect& r, const SkRasterClip& clip,
                       SkBlitter* blitter) {
    if (clip.isEmpty() || r.isEmpty()) {
        return;
    }

    if (clip.isBW()) {
        FillIRect(r, &clip.bwRgn(), blitter);
        return;
    }

    SkAAClipBlitterWrapper wrapper(clip, blitter);
    FillIRect(r, &wrapper.getRgn(), wrapper.getBlitter());
}

void SkScan::FillXRect(const SkXRect& xr, const SkRasterClip& clip,
                       SkBlitter* blitter) {
    if (clip.isEmpty() || xr.isEmpty()) {
        return;
    }

    if (clip.isBW()) {
        FillXRect(xr, &clip.bwRgn(), blitter);
        return;
    }

    SkAAClipBlitterWrapper wrapper(clip, blitter);
    FillXRect(xr, &wrapper.getRgn(), wrapper.getBlitter());
}

void SkScan::FillRect(const SkRect& r, const SkRasterClip& clip,
                      SkBlitter* blitter) {
    if (clip.isEmpty() || r.isEmpty()) {
        return;
    }

    if (clip.isBW()) {
        FillRect(r, &clip.bwRgn(), blitter);
        return;
    }

    SkAAClipBlitterWrapper wrapper(clip, blitter);
    FillRect(r, &wrapper.getRgn(), wrapper.getBlitter());
}
