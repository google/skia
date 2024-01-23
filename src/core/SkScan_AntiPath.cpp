/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkScanPriv.h"

#include "include/core/SkGraphics.h"
#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"
#include "src/core/SkBlitter.h"

static SkIRect safeRoundOut(const SkRect& src) {
    // roundOut will pin huge floats to max/min int
    SkIRect dst = src.roundOut();

    // intersect with a smaller huge rect, so the rect will not be considered empty for being
    // too large. e.g. { -SK_MaxS32 ... SK_MaxS32 } is considered empty because its width
    // exceeds signed 32bit.
    const int32_t limit = SK_MaxS32 >> SK_SUPERSAMPLE_SHIFT;
    (void)dst.intersect({ -limit, -limit, limit, limit});

    return dst;
}

static int overflows_short_shift(int value, int shift) {
    const int s = 16 + shift;
    return (SkLeftShift(value, s) >> s) - value;
}

/**
  Would any of the coordinates of this rectangle not fit in a short,
  when left-shifted by shift?
*/
static int rect_overflows_short_shift(SkIRect rect, int shift) {
    SkASSERT(!overflows_short_shift(8191, shift));
    SkASSERT(overflows_short_shift(8192, shift));
    SkASSERT(!overflows_short_shift(32767, 0));
    SkASSERT(overflows_short_shift(32768, 0));

    // Since we expect these to succeed, we bit-or together
    // for a tiny extra bit of speed.
    return overflows_short_shift(rect.fLeft, shift) |
           overflows_short_shift(rect.fRight, shift) |
           overflows_short_shift(rect.fTop, shift) |
           overflows_short_shift(rect.fBottom, shift);
}

void SkScan::AntiFillPath(const SkPath& path, const SkRegion& origClip,
                          SkBlitter* blitter, bool forceRLE) {
    if (origClip.isEmpty()) {
        return;
    }

    const bool isInverse = path.isInverseFillType();
    SkIRect ir = safeRoundOut(path.getBounds());
    if (ir.isEmpty()) {
        if (isInverse) {
            blitter->blitRegion(origClip);
        }
        return;
    }

    // If the intersection of the path bounds and the clip bounds
    // will overflow 32767 when << by SHIFT, we can't supersample,
    // so draw without antialiasing.
    SkIRect clippedIR;
    if (isInverse) {
       // If the path is an inverse fill, it's going to fill the entire
       // clip, and we care whether the entire clip exceeds our limits.
       clippedIR = origClip.getBounds();
    } else {
       if (!clippedIR.intersect(ir, origClip.getBounds())) {
           return;
       }
    }
    if (rect_overflows_short_shift(clippedIR, SK_SUPERSAMPLE_SHIFT)) {
        SkScan::FillPath(path, origClip, blitter);
        return;
    }

    // Our antialiasing can't handle a clip larger than 32767, so we restrict
    // the clip to that limit here. (the runs[] uses int16_t for its index).
    //
    // A more general solution (one that could also eliminate the need to
    // disable aa based on ir bounds (see overflows_short_shift) would be
    // to tile the clip/target...
    SkRegion tmpClipStorage;
    const SkRegion* clipRgn = &origClip;
    {
        static const int32_t kMaxClipCoord = 32767;
        const SkIRect& bounds = origClip.getBounds();
        if (bounds.fRight > kMaxClipCoord || bounds.fBottom > kMaxClipCoord) {
            SkIRect limit = { 0, 0, kMaxClipCoord, kMaxClipCoord };
            tmpClipStorage.op(origClip, limit, SkRegion::kIntersect_Op);
            clipRgn = &tmpClipStorage;
        }
    }
    // for here down, use clipRgn, not origClip

    SkScanClipper   clipper(blitter, clipRgn, ir);

    if (clipper.getBlitter() == nullptr) { // clipped out
        if (isInverse) {
            blitter->blitRegion(*clipRgn);
        }
        return;
    }

    SkASSERT(clipper.getClipRect() == nullptr ||
            *clipper.getClipRect() == clipRgn->getBounds());

    // now use the (possibly wrapped) blitter
    blitter = clipper.getBlitter();

    if (isInverse) {
        sk_blit_above(blitter, ir, *clipRgn);
    }

    SkScan::AAAFillPath(path, blitter, ir, clipRgn->getBounds(), forceRLE);

    if (isInverse) {
        sk_blit_below(blitter, ir, *clipRgn);
    }
}

///////////////////////////////////////////////////////////////////////////////

#include "src/core/SkRasterClip.h"

void SkScan::FillPath(const SkPath& path, const SkRasterClip& clip, SkBlitter* blitter) {
    if (clip.isEmpty() || !path.isFinite()) {
        return;
    }

    if (clip.isBW()) {
        FillPath(path, clip.bwRgn(), blitter);
    } else {
        SkRegion        tmp;
        SkAAClipBlitter aaBlitter;

        tmp.setRect(clip.getBounds());
        aaBlitter.init(blitter, &clip.aaRgn());
        SkScan::FillPath(path, tmp, &aaBlitter);
    }
}

void SkScan::AntiFillPath(const SkPath& path, const SkRasterClip& clip, SkBlitter* blitter) {
    if (clip.isEmpty() || !path.isFinite()) {
        return;
    }

    if (clip.isBW()) {
        AntiFillPath(path, clip.bwRgn(), blitter, false);
    } else {
        SkRegion        tmp;
        SkAAClipBlitter aaBlitter;

        tmp.setRect(clip.getBounds());
        aaBlitter.init(blitter, &clip.aaRgn());
        AntiFillPath(path, tmp, &aaBlitter, true); // SkAAClipBlitter can blitMask, why forceRLE?
    }
}
