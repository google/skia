
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
    /*
     *  Draws count-1 line segments, one at a time:
     *      line(pts[0], pts[1])
     *      line(pts[1], pts[2])
     *      line(......, pts[count - 1])
     */
    typedef void (*HairRgnProc)(const SkPoint[], int count, const SkRegion*, SkBlitter*);
    typedef void (*HairRCProc)(const SkPoint[], int count, const SkRasterClip&, SkBlitter*);

    static void FillPath(const SkPath&, const SkIRect&, SkBlitter*);

    ///////////////////////////////////////////////////////////////////////////
    // rasterclip

    static void FillIRect(const SkIRect&, const SkRasterClip&, SkBlitter*);
    static void FillXRect(const SkXRect&, const SkRasterClip&, SkBlitter*);
    static void FillRect(const SkRect&, const SkRasterClip&, SkBlitter*);
    static void AntiFillRect(const SkRect&, const SkRasterClip&, SkBlitter*);
    static void AntiFillXRect(const SkXRect&, const SkRasterClip&, SkBlitter*);
    static void FillPath(const SkPath&, const SkRasterClip&, SkBlitter*);
    static void AntiFillPath(const SkPath&, const SkRasterClip&, SkBlitter*);
    static void FrameRect(const SkRect&, const SkPoint& strokeSize,
                          const SkRasterClip&, SkBlitter*);
    static void AntiFrameRect(const SkRect&, const SkPoint& strokeSize,
                              const SkRasterClip&, SkBlitter*);
    static void FillTriangle(const SkPoint pts[], const SkRasterClip&, SkBlitter*);
    static void HairLine(const SkPoint[], int count, const SkRasterClip&, SkBlitter*);
    static void AntiHairLine(const SkPoint[], int count, const SkRasterClip&, SkBlitter*);
    static void HairRect(const SkRect&, const SkRasterClip&, SkBlitter*);
    static void AntiHairRect(const SkRect&, const SkRasterClip&, SkBlitter*);
    static void HairPath(const SkPath&, const SkRasterClip&, SkBlitter*);
    static void AntiHairPath(const SkPath&, const SkRasterClip&, SkBlitter*);
    static void HairSquarePath(const SkPath&, const SkRasterClip&, SkBlitter*);
    static void AntiHairSquarePath(const SkPath&, const SkRasterClip&, SkBlitter*);
    static void HairRoundPath(const SkPath&, const SkRasterClip&, SkBlitter*);
    static void AntiHairRoundPath(const SkPath&, const SkRasterClip&, SkBlitter*);

private:
    friend class SkAAClip;
    friend class SkRegion;

    static void FillIRect(const SkIRect&, const SkRegion* clip, SkBlitter*);
    static void FillXRect(const SkXRect&, const SkRegion* clip, SkBlitter*);
    static void FillRect(const SkRect&, const SkRegion* clip, SkBlitter*);
    static void AntiFillRect(const SkRect&, const SkRegion* clip, SkBlitter*);
    static void AntiFillXRect(const SkXRect&, const SkRegion*, SkBlitter*);
    static void FillPath(const SkPath&, const SkRegion& clip, SkBlitter*);
    static void AntiFillPath(const SkPath&, const SkRegion& clip, SkBlitter*,
                             bool forceRLE = false);
    static void FillTriangle(const SkPoint pts[], const SkRegion*, SkBlitter*);

    static void AntiFrameRect(const SkRect&, const SkPoint& strokeSize,
                              const SkRegion*, SkBlitter*);
    static void HairLineRgn(const SkPoint[], int count, const SkRegion*, SkBlitter*);
    static void AntiHairLineRgn(const SkPoint[], int count, const SkRegion*, SkBlitter*);
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
    dst->fLeft = SkFixedRoundToInt(xr.fLeft);
    dst->fTop = SkFixedRoundToInt(xr.fTop);
    dst->fRight = SkFixedRoundToInt(xr.fRight);
    dst->fBottom = SkFixedRoundToInt(xr.fBottom);
}

/** Round the SkXRect coordinates out (i.e. use floor for left/top, and ceiling
    for right/bottom), and store the result in the SkIRect.
*/
static inline void XRect_roundOut(const SkXRect& xr, SkIRect* dst) {
    dst->fLeft = SkFixedFloorToInt(xr.fLeft);
    dst->fTop = SkFixedFloorToInt(xr.fTop);
    dst->fRight = SkFixedCeilToInt(xr.fRight);
    dst->fBottom = SkFixedCeilToInt(xr.fBottom);
}

#endif
