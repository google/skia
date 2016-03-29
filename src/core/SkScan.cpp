/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkScan.h"
#include "SkBlitter.h"
#include "SkRasterClip.h"

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
