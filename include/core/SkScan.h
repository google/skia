
/*
 * Copyright 2011 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkScan_DEFINED
#define SkScan_DEFINED

#include "SkRect.h"

class SkRasterClip;
class SkRegion;
class SkBlitter;
class SkPath;

/** Defines a fixed-point rectangle, identical to the integer SkIRect, but its
    coordinates are treated as SkFixed rather than int32_t.
*/
typedef SkIRect SkXRect;

class SkScan {
public:
    static void FillIRect(const SkIRect&, const SkRegion* clip, SkBlitter*);
    static void FillXRect(const SkXRect&, const SkRegion* clip, SkBlitter*);

#ifdef SK_SCALAR_IS_FIXED
    static void FillRect(const SkRect& rect, const SkRegion* clip,
                         SkBlitter* blitter) {
        SkScan::FillXRect(*(const SkXRect*)&rect, clip, blitter);
    }
#else
    static void FillRect(const SkRect&, const SkRegion* clip, SkBlitter*);
#endif
    static void FillPath(const SkPath&, const SkRegion& clip, SkBlitter*);

    static void FillTriangle(const SkPoint pts[], const SkRegion*, SkBlitter*);
    static void FillTriangle(const SkPoint& a, const SkPoint& b,
                             const SkPoint& c, const SkRegion* clip,
                             SkBlitter* blitter) {
        SkPoint pts[3];
        pts[0] = a;
        pts[1] = b;
        pts[2] = c;
        FillTriangle(pts, clip, blitter);
    }

    static void HairLine(const SkPoint&, const SkPoint&, const SkRegion*,
                         SkBlitter*);
    static void HairRect(const SkRect&, const SkRegion* clip, SkBlitter*);
    static void HairPath(const SkPath&, const SkRegion* clip, SkBlitter*);

    static void AntiFillXRect(const SkXRect&, const SkRegion* clip, SkBlitter*);
#ifdef SK_SCALAR_IS_FIXED
    static void AntiFillRect(const SkRect& rect, const SkRegion* clip,
                         SkBlitter* blitter) {
        SkScan::AntiFillXRect(*(const SkXRect*)&rect, clip, blitter);
    }
#else
    static void AntiFillRect(const SkRect&, const SkRegion* clip, SkBlitter*);
#endif
    
    static void AntiFillPath(const SkPath&, const SkRegion& clip, SkBlitter*,
                             bool forceRLE = false);

    static void AntiHairLine(const SkPoint&, const SkPoint&, const SkRegion*,
                             SkBlitter*);
    static void AntiHairRect(const SkRect&, const SkRegion* clip, SkBlitter*);
    static void AntiHairPath(const SkPath&, const SkRegion* clip, SkBlitter*);

    // draws with a miter-join
    static void FrameRect(const SkRect&, const SkPoint& strokeSize,
                          const SkRegion*, SkBlitter*);
    static void AntiFrameRect(const SkRect&, const SkPoint& strokeSize,
                              const SkRegion*, SkBlitter*);

    // rasterclip
    static void FillIRect(const SkIRect&, const SkRasterClip&, SkBlitter*);
    static void FillPath(const SkPath&, const SkRasterClip&, SkBlitter*);
    static void AntiFillPath(const SkPath&, const SkRasterClip&, SkBlitter*);
};

/** Assign an SkXRect from a SkIRect, by promoting the src rect's coordinates
    from int to SkFixed. Does not check for overflow if the src coordinates
    exceed 32K
*/
static inline void XRect_set(SkXRect* xr, const SkIRect& src) {
    xr->fLeft = SkIntToFixed(src.fLeft);
    xr->fTop = SkIntToFixed(src.fTop);
    xr->fRight = SkIntToFixed(src.fRight);
    xr->fBottom = SkIntToFixed(src.fBottom);
}

/** Assign an SkXRect from a SkRect, by promoting the src rect's coordinates
    from SkScalar to SkFixed. Does not check for overflow if the src coordinates
    exceed 32K
*/
static inline void XRect_set(SkXRect* xr, const SkRect& src) {
    xr->fLeft = SkScalarToFixed(src.fLeft);
    xr->fTop = SkScalarToFixed(src.fTop);
    xr->fRight = SkScalarToFixed(src.fRight);
    xr->fBottom = SkScalarToFixed(src.fBottom);
}

/** Round the SkXRect coordinates, and store the result in the SkIRect.
*/
static inline void XRect_round(const SkXRect& xr, SkIRect* dst) {
    dst->fLeft = SkFixedRound(xr.fLeft);
    dst->fTop = SkFixedRound(xr.fTop);
    dst->fRight = SkFixedRound(xr.fRight);
    dst->fBottom = SkFixedRound(xr.fBottom);
}

/** Round the SkXRect coordinates out (i.e. use floor for left/top, and ceiling
    for right/bottom), and store the result in the SkIRect.
*/
static inline void XRect_roundOut(const SkXRect& xr, SkIRect* dst) {
    dst->fLeft = SkFixedFloor(xr.fLeft);
    dst->fTop = SkFixedFloor(xr.fTop);
    dst->fRight = SkFixedCeil(xr.fRight);
    dst->fBottom = SkFixedCeil(xr.fBottom);
}

#endif
