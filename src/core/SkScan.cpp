
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "SkScan.h"
#include "SkBlitter.h"
#include "SkRegion.h"

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

#ifdef SK_SCALAR_IS_FLOAT

void SkScan::FillRect(const SkRect& r, const SkRegion* clip,
                       SkBlitter* blitter) {
    SkIRect ir;
    
    r.round(&ir);
    SkScan::FillIRect(ir, clip, blitter);
}

#endif

