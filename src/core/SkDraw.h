/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skcpu_Draw_DEFINED
#define skcpu_Draw_DEFINED

#include "include/core/SkCanvas.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSamplingOptions.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStrokeRec.h"
#include "include/core/SkSurfaceProps.h"
#include "include/private/base/SkDebug.h"
#include "src/base/SkZip.h"
#include "src/core/SkDrawTypes.h"
#include "src/core/SkMask.h"

class SkArenaAlloc;
class SkBitmap;
class SkBlender;
class SkBlitter;
class SkDevice;
class SkGlyph;
class SkMaskFilter;
class SkMatrix;
class SkPath;
struct SkPathRaw;
class SkRRect;
class SkRasterClip;
class SkShader;
class SkVertices;
struct SkIRect;
struct SkPoint;
struct SkPoint3;
struct SkRSXform;
struct SkRect;

namespace sktext {
class GlyphRunList;
}

namespace skcpu {

class GlyphRunListPainter;
class ContextImpl;


/** Helper function that creates a mask from a path and a required maskfilter.
    Note however, that the resulting mask will not have been actually filtered,
    that must be done afterwards (by calling filterMask). The maskfilter is provided
    solelely to assist in computing the mask's bounds (if the mode requests that).
*/
bool DrawToMask(const SkPathRaw& devRaw,
                const SkIRect& clipBounds,
                const SkMaskFilter*,
                const SkMatrix* filterMatrix,
                SkMaskBuilder* dst,
                SkMaskBuilder::CreateMode mode,
                SkStrokeRec::InitStyle style);

/**
 *  BitmapDevicePainter provides a minimal interface for painting pre-rasterized objects like glyphs
 *  and bitmaps.
 *
 *  This interface creates a seam that allows components to intercept and handle low-level drawing
 *  primitives. For example, SkOverdrawCanvas uses this to visualize overdraw by drawing bounding
 *  boxes for glyphs
 */
class BitmapDevicePainter {
public:
    BitmapDevicePainter() = default;
    BitmapDevicePainter(const BitmapDevicePainter&) = default;
    virtual ~BitmapDevicePainter() = default;

    virtual void paintMasks(SkZip<const SkGlyph*, SkPoint> accepted,
                            const SkPaint& paint) const = 0;
    virtual void drawBitmap(const SkBitmap&,
                            const SkMatrix&,
                            const SkRect* dstOrNull,
                            const SkSamplingOptions&,
                            const SkPaint&) const = 0;
};

/**
 *  The Draw class is the workhorse for the software rendering backend. It is an implementation
 *  detail of SkCanvas that orchestrates the drawing of primitives into a CPU-backed SkPixmap.
 *
 *  A Draw object is a lightweight context configured with everything needed for a single drawing
 *  operation:
 *    - The destination pixels (SkPixmap)
 *    - The current transformation matrix
 *    - The clipping region
 *
 *  Its primary responsibility is to analyze the primitive (path, rect, etc.), the SkPaint, and the
 *  device state to select and configure the most efficient SkBlitter. The SkBlitter then handles
 *  the actual work of writing pixels into the destination pixmap.
 *  The default Blitter is chosen via SkBlitter::Choose()
 */
class Draw : public BitmapDevicePainter {
public:
    Draw();

    void drawPaint(const SkPaint&) const;
    void drawRect(const SkRect& prePaintRect,
                  const SkPaint&,
                  const SkMatrix* paintMatrix,
                  const SkRect* postPaintRect) const;
    void drawRect(const SkRect& rect, const SkPaint& paint) const {
        this->drawRect(rect, paint, nullptr, nullptr);
    }
    void drawOval(const SkRect&, const SkPaint&) const;
    void drawRRect(const SkRRect&, const SkPaint&) const;
    // Specialized draw for RRect that only draws if it is nine-patchable.
    bool drawRRectNinePatch(const SkRRect&, const SkPaint&) const;
    /**
     *  To save on mallocs, we allow a flag that tells us that srcPath is
     *  mutable, so that we don't have to make copies of it as we transform it.
     *
     *  If prePathMatrix is not null, it should logically be applied before any
     *  stroking or other effects. If there are no effects on the paint that
     *  affect the geometry/rasterization, then the pre matrix can just be
     *  pre-concated with the current matrix.
     */
    void drawPath(const SkPath& path,
                  const SkPaint& paint,
                  const SkMatrix* prePathMatrix,
                  bool pathIsMutable) const {
        this->drawPath(path, paint, prePathMatrix, pathIsMutable, SkDrawCoverage::kNo);
    }

    /**
     *  Overwrite the target with the path's coverage (i.e. its mask).
     *  Will overwrite the entire device, so it need not be zero'd first.
     *
     *  Only device A8 is supported right now.
     */
    void drawPathCoverage(const SkPath& src,
                          const SkPaint& paint,
                          SkBlitter* customBlitter = nullptr) const {
        bool isHairline = paint.getStyle() == SkPaint::kStroke_Style && paint.getStrokeWidth() == 0;
        this->drawPath(src,
                       paint,
                       nullptr,
                       false,
                       isHairline ? SkDrawCoverage::kNo : SkDrawCoverage::kYes,
                       customBlitter);
    }

    void drawDevicePoints(SkCanvas::PointMode,
                          SkSpan<const SkPoint>,
                          const SkPaint&,
                          SkDevice*) const;

    enum class RectType {
        kHair,
        kFill,
        kStroke,
        kPath,
    };

    /**
     *  Based on the paint's style, strokeWidth, and the matrix, classify how
     *  to draw the rect. If no special-case is available, returns
     *  RectType::kPath.
     *
     *  Iff RectType == RectType::kStroke, then strokeSize is set to the device
     *  width and height of the stroke.
     */
    static RectType ComputeRectType(const SkRect&,
                                    const SkPaint&,
                                    const SkMatrix&,
                                    SkPoint* strokeSize);

    using BlitterChooser = SkBlitter*(const SkPixmap& dst,
                                      const SkMatrix& ctm,
                                      const SkPaint&,
                                      SkArenaAlloc*,
                                      SkDrawCoverage drawCoverage,
                                      sk_sp<SkShader> clipShader,
                                      const SkSurfaceProps&,
                                      const SkRect& devBounds);

    /* If dstOrNull is null, computes a dst by mapping the bitmap's bounds through the matrix. */
    void drawBitmap(const SkBitmap&, const SkMatrix&, const SkRect* dstOrNull,
                    const SkSamplingOptions&, const SkPaint&) const override;
    void drawSprite(const SkBitmap&, int x, int y, const SkPaint&) const;
    void drawGlyphRunList(SkCanvas* canvas,
                          GlyphRunListPainter* glyphPainter,
                          const sktext::GlyphRunList& glyphRunList,
                          const SkPaint& paint) const;

    void paintMasks(SkZip<const SkGlyph*, SkPoint> accepted, const SkPaint& paint) const override;

    void drawPoints(SkCanvas::PointMode, SkSpan<const SkPoint>, const SkPaint&, SkDevice*) const;
    /* If skipColorXform, skips color conversion when assigning per-vertex colors */
    void drawVertices(const SkVertices*,
                      sk_sp<SkBlender>,
                      const SkPaint&,
                      bool skipColorXform) const;
    void drawAtlas(SkSpan<const SkRSXform>, SkSpan<const SkRect>, SkSpan<const SkColor>,
                   sk_sp<SkBlender>, const SkPaint&);

    void drawDevMask(const SkMask& mask, const SkPaint&, const SkMatrix*) const;
    void drawBitmapAsMask(const SkBitmap&, const SkSamplingOptions&, const SkPaint&,
                          const SkMatrix* paintMatrix) const;

private:
    void drawPath(const SkPath&,
                  const SkPaint&,
                  const SkMatrix* preMatrix,
                  bool pathIsMutable,
                  SkDrawCoverage drawCoverage,
                  SkBlitter* customBlitter = nullptr) const;

    void drawLine(const SkPoint[2], const SkPaint&) const;

    void drawDevPath(const SkPathRaw&,
                     const SkPaint& paint,
                     SkDrawCoverage drawCoverage,
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

    void drawFixedVertices(const SkVertices* vertices,
                           sk_sp<SkBlender> blender,
                           const SkPaint& paint,
                           const SkMatrix& ctmInverse,
                           const SkPoint* dev2,
                           const SkPoint3* dev3,
                           SkArenaAlloc* outerAlloc,
                           bool skipColorXform) const;

public:
    SkPixmap fDst;
    BlitterChooser* fBlitterChooser{nullptr};  // required
    const SkMatrix* fCTM{nullptr};             // required
    const SkRasterClip* fRC{nullptr};          // required
    const SkSurfaceProps* fProps{nullptr};     // optional

    const ContextImpl* fCtx{nullptr};  // optional for now

#ifdef SK_DEBUG
    void validate() const;
#else
    void validate() const {}
#endif
};

}  // namespace skcpu

#endif
