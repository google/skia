/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkScanPriv_DEFINED
#define SkScanPriv_DEFINED

#include "SkScan.h"
#include "SkBlitter.h"

class SkScanClipper {
public:
    SkScanClipper(SkBlitter* blitter, const SkRegion* clip, const SkIRect& bounds,
                  bool skipRejectTest = false);

    SkBlitter*      getBlitter() const { return fBlitter; }
    const SkIRect*  getClipRect() const { return fClipRect; }

private:
    SkRectClipBlitter   fRectBlitter;
    SkRgnClipBlitter    fRgnBlitter;
#ifdef SK_DEBUG
    // SkRectClipCheckBlitter fRectClipCheckBlitter;
#endif
    SkBlitter*          fBlitter;
    const SkIRect*      fClipRect;
};

void sk_fill_path(const SkPath& path, const SkIRect& clipRect,
                  SkBlitter* blitter, int start_y, int stop_y, int shiftEdgesUp,
                  bool pathContainedInClip);

void aaa_fill_path(const SkPath& path, const SkIRect& clipRect, AdditiveBlitter*,
                   int start_y, int stop_y, bool pathContainedInClip, bool isUsingMask,
                   bool forceRLE);

// blit the rects above and below avoid, clipped to clip
void sk_blit_above(SkBlitter*, const SkIRect& avoid, const SkRegion& clip);
void sk_blit_below(SkBlitter*, const SkIRect& avoid, const SkRegion& clip);

#endif
