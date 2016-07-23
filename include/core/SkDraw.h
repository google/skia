
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkDraw_DEFINED
#define SkDraw_DEFINED

#include "SkCanvas.h"
#include "SkMask.h"
#include "SkPaint.h"
#include "SkStrokeRec.h"

class SkBitmap;
class SkClipStack;
class SkBaseDevice;
class SkBlitter;
class SkMatrix;
class SkPath;
class SkRegion;
class SkRasterClip;
struct SkDrawProcs;
struct SkRect;
class SkRRect;

class SkDraw {
public:
    SkDraw();

    void    drawPaint(const SkPaint&) const;
    void    drawPoints(SkCanvas::PointMode, size_t count, const SkPoint[],
                       const SkPaint&, bool forceUseDevice = false) const;
    void    drawRect(const SkRect& prePaintRect, const SkPaint&, const SkMatrix* paintMatrix,
                     const SkRect* postPaintRect) const;
    void    drawRect(const SkRect& rect, const SkPaint& paint) const {
        this->drawRect(rect, paint, NULL, NULL);
    }
    void    drawRRect(const SkRRect&, const SkPaint&) const;
    /**
     *  To save on mallocs, we allow a flag that tells us that srcPath is
     *  mutable, so that we don't have to make copies of it as we transform it.
     *
     *  If prePathMatrix is not null, it should logically be applied before any
     *  stroking or other effects. If there are no effects on the paint that
     *  affect the geometry/rasterization, then the pre matrix can just be
     *  pre-concated with the current matrix.
     */
    void    drawPath(const SkPath& path, const SkPaint& paint,
                     const SkMatrix* prePathMatrix, bool pathIsMutable) const {
        this->drawPath(path, paint, prePathMatrix, pathIsMutable, false);
    }

    void drawPath(const SkPath& path, const SkPaint& paint,
                  SkBlitter* customBlitter = NULL) const {
        this->drawPath(path, paint, NULL, false, false, customBlitter);
    }

    /* If dstOrNull is null, computes a dst by mapping the bitmap's bounds through the matrix. */
    void    drawBitmap(const SkBitmap&, const SkMatrix&, const SkRect* dstOrNull,
                       const SkPaint&) const;
    void    drawSprite(const SkBitmap&, int x, int y, const SkPaint&) const;
    void    drawText(const char text[], size_t byteLength, SkScalar x,
                     SkScalar y, const SkPaint& paint) const;
    void    drawPosText(const char text[], size_t byteLength,
                        const SkScalar pos[], int scalarsPerPosition,
                        const SkPoint& offset, const SkPaint& paint) const;
    void    drawVertices(SkCanvas::VertexMode mode, int count,
                         const SkPoint vertices[], const SkPoint textures[],
                         const SkColor colors[], SkXfermode* xmode,
                         const uint16_t indices[], int ptCount,
                         const SkPaint& paint) const;

    /**
     *  Overwrite the target with the path's coverage (i.e. its mask).
     *  Will overwrite the entire device, so it need not be zero'd first.
     *
     *  Only device A8 is supported right now.
     */
    void drawPathCoverage(const SkPath& src, const SkPaint& paint,
                          SkBlitter* customBlitter = NULL) const {
        this->drawPath(src, paint, NULL, false, true, customBlitter);
    }

    /** Helper function that creates a mask from a path and an optional maskfilter.
        Note however, that the resulting mask will not have been actually filtered,
        that must be done afterwards (by calling filterMask). The maskfilter is provided
        solely to assist in computing the mask's bounds (if the mode requests that).
    */
    static bool DrawToMask(const SkPath& devPath, const SkIRect* clipBounds,
                           const SkMaskFilter*, const SkMatrix* filterMatrix,
                           SkMask* mask, SkMask::CreateMode mode,
                           SkStrokeRec::InitStyle style);

    enum RectType {
        kHair_RectType,
        kFill_RectType,
        kStroke_RectType,
        kPath_RectType
    };

    /**
     *  Based on the paint's style, strokeWidth, and the matrix, classify how
     *  to draw the rect. If no special-case is available, returns
     *  kPath_RectType.
     *
     *  Iff RectType == kStroke_RectType, then strokeSize is set to the device
     *  width and height of the stroke.
     */
    static RectType ComputeRectType(const SkPaint&, const SkMatrix&,
                                    SkPoint* strokeSize);

    static bool ShouldDrawTextAsPaths(const SkPaint&, const SkMatrix&);
    void        drawText_asPaths(const char text[], size_t byteLength,
                                 SkScalar x, SkScalar y, const SkPaint&) const;
    void        drawPosText_asPaths(const char text[], size_t byteLength,
                                    const SkScalar pos[], int scalarsPerPosition,
                                    const SkPoint& offset, const SkPaint&) const;
    static SkScalar ComputeResScaleForStroking(const SkMatrix& );
private:
    void    drawDevMask(const SkMask& mask, const SkPaint&) const;
    void    drawBitmapAsMask(const SkBitmap&, const SkPaint&) const;

    void    drawPath(const SkPath&, const SkPaint&, const SkMatrix* preMatrix,
                     bool pathIsMutable, bool drawCoverage,
                     SkBlitter* customBlitter = NULL) const;

    void drawLine(const SkPoint[2], const SkPaint&) const;
    void drawDevPath(const SkPath& devPath, const SkPaint& paint, bool drawCoverage,
                     SkBlitter* customBlitter, bool doFill) const;
    /**
     *  Return the current clip bounds, in local coordinates, with slop to account
     *  for antialiasing or hairlines (i.e. device-bounds outset by 1, and then
     *  run through the inverse of the matrix).
     *
     *  If the matrix cannot be inverted, or the current clip is empty, return
     *  false and ignore bounds parameter.
     */
    bool SK_WARN_UNUSED_RESULT
    computeConservativeLocalClipBounds(SkRect* bounds) const;

    /** Returns the current setting for using fake gamma and contrast. */
    uint32_t SK_WARN_UNUSED_RESULT scalerContextFlags() const;

public:
    SkPixmap        fDst;
    const SkMatrix* fMatrix;        // required
    const SkRasterClip* fRC;        // required

    const SkClipStack* fClipStack;  // optional, may be null
    SkBaseDevice*   fDevice;        // optional, may be null

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif
};

#endif
