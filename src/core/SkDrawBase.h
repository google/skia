/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkDrawBase_DEFINED
#define SkDrawBase_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkStrokeRec.h"
#include "src/base/SkZip.h"
#include "src/core/SkGlyphRunPainter.h"
#include "src/core/SkMask.h"
#include <cstddef>

class SkArenaAlloc;
class SkBitmap;
class SkBlitter;
class SkDevice;
class SkGlyph;
class SkMaskFilter;
class SkMatrix;
class SkPath;
class SkRRect;
class SkRasterClip;
class SkShader;
class SkSurfaceProps;
struct SkIRect;
struct SkPoint;
struct SkRect;

class SkDrawBase : public SkGlyphRunListPainterCPU::BitmapDevicePainter {
public:
    SkDrawBase();

    void    drawPaint(const SkPaint&) const;
    void    drawRect(const SkRect& prePaintRect, const SkPaint&, const SkMatrix* paintMatrix,
                     const SkRect* postPaintRect) const;
    void    drawRect(const SkRect& rect, const SkPaint& paint) const {
        this->drawRect(rect, paint, nullptr, nullptr);
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
                     const SkMatrix* prePathMatrix = nullptr, bool pathIsMutable = false) const {
        this->drawPath(path, paint, prePathMatrix, pathIsMutable, false);
    }

    /**
     *  Overwrite the target with the path's coverage (i.e. its mask).
     *  Will overwrite the entire device, so it need not be zero'd first.
     *
     *  Only device A8 is supported right now.
     */
    void drawPathCoverage(const SkPath& src, const SkPaint& paint,
                          SkBlitter* customBlitter = nullptr) const {
        bool isHairline = paint.getStyle() == SkPaint::kStroke_Style &&
                          paint.getStrokeWidth() == 0;
        this->drawPath(src, paint, nullptr, false, !isHairline, customBlitter);
    }

    void drawDevicePoints(SkCanvas::PointMode, size_t count, const SkPoint[], const SkPaint&,
                          SkDevice*) const;

    static bool ComputeMaskBounds(const SkRect& devPathBounds, const SkIRect& clipBounds,
                                  const SkMaskFilter* filter, const SkMatrix* filterMatrix,
                                  SkIRect* bounds);

    /** Helper function that creates a mask from a path and an optional maskfilter.
        Note however, that the resulting mask will not have been actually filtered,
        that must be done afterwards (by calling filterMask). The maskfilter is provided
        solely to assist in computing the mask's bounds (if the mode requests that).
    */
    static bool DrawToMask(const SkPath& devPath, const SkIRect& clipBounds,
                           const SkMaskFilter*, const SkMatrix* filterMatrix,
                           SkMaskBuilder* dst, SkMaskBuilder::CreateMode mode,
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
    static RectType ComputeRectType(const SkRect&, const SkPaint&, const SkMatrix&,
                                    SkPoint* strokeSize);

    using BlitterChooser = SkBlitter* (const SkPixmap& dst,
                                       const SkMatrix& ctm,
                                       const SkPaint&,
                                       SkArenaAlloc*,
                                       bool drawCoverage,
                                       sk_sp<SkShader> clipShader,
                                       const SkSurfaceProps&);


private:
    // not supported
    void paintMasks(SkZip<const SkGlyph*, SkPoint> accepted, const SkPaint& paint) const override;
    void drawBitmap(const SkBitmap&, const SkMatrix&, const SkRect* dstOrNull,
                    const SkSamplingOptions&, const SkPaint&) const override;

    void drawPath(const SkPath&,
                  const SkPaint&,
                  const SkMatrix* preMatrix,
                  bool pathIsMutable,
                  bool drawCoverage,
                  SkBlitter* customBlitter = nullptr) const;

    void drawLine(const SkPoint[2], const SkPaint&) const;

    void drawDevPath(const SkPath& devPath,
                     const SkPaint& paint,
                     bool drawCoverage,
                     SkBlitter* customBlitter,
                     bool doFill) const;
    /**
     *  Return the current clip bounds, in local coordinates, with slop to account
     *  for antialiasing or hairlines (i.e. device-bounds outset by 1, and then
     *  run through the inverse of the matrix).
     *
     *  If the matrix cannot be inverted, or the current clip is empty, return
     *  false and ignore bounds parameter.
     */
    [[nodiscard]] bool computeConservativeLocalClipBounds(SkRect* bounds) const;

public:
    SkPixmap                fDst;
    BlitterChooser*         fBlitterChooser{nullptr};  // required
    const SkMatrix*         fCTM{nullptr};             // required
    const SkRasterClip*     fRC{nullptr};              // required
    const SkSurfaceProps*   fProps{nullptr};           // optional

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif
};

#endif  // SkDrawBase_DEFINED
