/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkDraw.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkColorType.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkPathUtils.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRegion.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStrokeRec.h"
#include "include/core/SkTileMode.h"
#include "include/private/base/SkAlign.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkCPUTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFixed.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkTLazy.h"
#include "src/base/SkZip.h"
#include "src/core/SkAutoBlitterChoose.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkBlitter.h"
#include "src/core/SkBlitter_A8.h"
#include "src/core/SkDevice.h"
#include "src/core/SkDrawProcs.h"
#include "src/core/SkDrawTypes.h"
#include "src/core/SkImageInfoPriv.h"
#include "src/core/SkImagePriv.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkMatrixUtils.h"
#include "src/core/SkPathData.h"
#include "src/core/SkPathEffectBase.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkScan.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

class SkResourceCache;

using namespace skia_private;

namespace skcpu {

static SkPaint make_paint_with_image(const SkPaint& origPaint, const SkBitmap& bitmap,
                                     const SkSamplingOptions& sampling,
                                     SkMatrix* matrix = nullptr) {
    SkPaint paint(origPaint);
    paint.setShader(SkMakeBitmapShaderForPaint(origPaint, bitmap, SkTileMode::kClamp,
                                               SkTileMode::kClamp, sampling, matrix,
                                               kNever_SkCopyPixelsMode));
    return paint;
}

Draw::Draw() { fBlitterChooser = SkBlitter::Choose; }

struct PtProcRec {
    SkCanvas::PointMode fMode;
    const SkPaint*  fPaint;
    const SkRegion* fClip;
    const SkRasterClip* fRC;

    // computed values
    SkRect   fClipBounds;
    SkScalar fRadius;

    typedef void (*Proc)(const PtProcRec&, SkSpan<const SkPoint> devPts, SkBlitter*);

    bool init(SkCanvas::PointMode, const SkPaint&, const SkMatrix* matrix,
              const SkRasterClip*);
    Proc chooseProc(SkBlitter** blitter);

private:
    SkAAClipBlitterWrapper fWrapper;
};

#define DIRECT_BLIT_LOOP(writable_method, value)    \
    do {                                            \
        for (auto p : devPts) {                     \
            int x = SkScalarFloorToInt(p.fX);       \
            int y = SkScalarFloorToInt(p.fY);       \
            if (cr.contains(x, y)) {                \
                *pm.writable_method(x, y) = value;  \
            }                                       \
        }                                           \
    } while (0)


static void bw_pt_hair_proc(const PtProcRec& rec, SkSpan<const SkPoint> devPts,
                            SkBlitter* blitter) {
    const auto direct = blitter->canDirectBlit();
    if (direct && rec.fClip->isRect()) {
        const SkIRect cr = rec.fClip->getBounds();
        auto pm = direct->pm;
        const auto v = direct->value;
        switch (pm.info().bytesPerPixel()) {
            case 1: DIRECT_BLIT_LOOP(writable_addr8,  v); break;
            case 2: DIRECT_BLIT_LOOP(writable_addr16, v); break;
            case 4: DIRECT_BLIT_LOOP(writable_addr32, v); break;
            case 8: DIRECT_BLIT_LOOP(writable_addr64, v); break;
            default: SkASSERT(false);
        }
    } else {
        for (auto p : devPts) {
            int x = SkScalarFloorToInt(p.fX);
            int y = SkScalarFloorToInt(p.fY);
            if (rec.fClip->contains(x, y)) {
                blitter->blitH(x, y, 1);
            }
        }
    }
}

static void bw_line_hair_proc(const PtProcRec& rec, SkSpan<const SkPoint> devPts,
                              SkBlitter* blitter) {
    for (size_t i = 0; i+1 < devPts.size(); i += 2) {
        SkScan::HairLine({&devPts[i], 2}, *rec.fRC, blitter);
    }
}

static void bw_poly_hair_proc(const PtProcRec& rec, SkSpan<const SkPoint> devPts,
                              SkBlitter* blitter) {
    SkScan::HairLine(devPts, *rec.fRC, blitter);
}

// aa versions

static void aa_line_hair_proc(const PtProcRec& rec, SkSpan<const SkPoint> devPts,
                              SkBlitter* blitter) {
    for (size_t i = 0; i+1 < devPts.size(); i += 2) {
        SkScan::AntiHairLine({&devPts[i], 2}, *rec.fRC, blitter);
    }
}

static void aa_poly_hair_proc(const PtProcRec& rec, SkSpan<const SkPoint> devPts,
                              SkBlitter* blitter) {
    SkScan::AntiHairLine(devPts, *rec.fRC, blitter);
}

// square procs (strokeWidth > 0 but matrix is square-scale (sx == sy)

static SkRect make_square_rad(SkPoint center, SkScalar radius) {
    return {
        center.fX - radius, center.fY - radius,
        center.fX + radius, center.fY + radius
    };
}

static SkXRect make_xrect(const SkRect& r) {
    SkASSERT(SkRectPriv::FitsInFixed(r));
    return {
        SkScalarToFixed(r.fLeft), SkScalarToFixed(r.fTop),
        SkScalarToFixed(r.fRight), SkScalarToFixed(r.fBottom)
    };
}

static void bw_square_proc(const PtProcRec& rec, SkSpan<const SkPoint> devPts,
                           SkBlitter* blitter) {
    for (auto p : devPts) {
        SkRect r = make_square_rad(p, rec.fRadius);
        if (r.intersect(rec.fClipBounds)) {
            SkScan::FillXRect(make_xrect(r), *rec.fRC, blitter);
        }
    }
}

static void aa_square_proc(const PtProcRec& rec, SkSpan<const SkPoint> devPts,
                           SkBlitter* blitter) {
    for (auto p : devPts) {
        SkRect r = make_square_rad(p, rec.fRadius);
        if (r.intersect(rec.fClipBounds)) {
            SkScan::AntiFillXRect(make_xrect(r), *rec.fRC, blitter);
        }
    }
}

// If this returns true, then chooseProc() must return a valid proc
bool PtProcRec::init(SkCanvas::PointMode mode, const SkPaint& paint,
                     const SkMatrix* matrix, const SkRasterClip* rc) {
    if ((unsigned)mode > (unsigned)SkCanvas::kPolygon_PointMode) {
        return false;
    }
    if (paint.getPathEffect() || paint.getMaskFilter()) {
        return false;
    }
    SkScalar width = paint.getStrokeWidth();
    SkScalar radius = -1;   // sentinel value, a "valid" value must be > 0

    if (0 == width) {
        radius = 0.5f;
    } else if (paint.getStrokeCap() != SkPaint::kRound_Cap &&
               matrix->isScaleTranslate() && SkCanvas::kPoints_PointMode == mode) {
        SkScalar sx = matrix->get(SkMatrix::kMScaleX);
        SkScalar sy = matrix->get(SkMatrix::kMScaleY);
        if (SkScalarNearlyZero(sx - sy)) {
            radius = SkScalarHalf(width * SkScalarAbs(sx));
        }
    }
    if (radius > 0) {
        SkRect clipBounds = SkRect::Make(rc->getBounds());
        // if we return true, the caller may assume that the constructed shapes can be represented
        // using SkFixed (after clipping), so we preflight that here.
        if (!SkRectPriv::FitsInFixed(clipBounds)) {
            return false;
        }
        fMode = mode;
        fPaint = &paint;
        fClip = nullptr;
        fRC = rc;
        fClipBounds = clipBounds;
        fRadius = radius;
        return true;
    }
    return false;
}

PtProcRec::Proc PtProcRec::chooseProc(SkBlitter** blitterPtr) {
    Proc proc = nullptr;

    SkBlitter* blitter = *blitterPtr;
    if (fRC->isBW()) {
        fClip = &fRC->bwRgn();
    } else {
        fWrapper.init(*fRC, blitter);
        fClip = &fWrapper.getRgn();
        blitter = fWrapper.getBlitter();
        *blitterPtr = blitter;
    }

    // for our arrays
    SkASSERT(0 == SkCanvas::kPoints_PointMode);
    SkASSERT(1 == SkCanvas::kLines_PointMode);
    SkASSERT(2 == SkCanvas::kPolygon_PointMode);
    SkASSERT((unsigned)fMode <= (unsigned)SkCanvas::kPolygon_PointMode);

    if (fPaint->isAntiAlias()) {
        if (0 == fPaint->getStrokeWidth()) {
            static const Proc gAAProcs[] = {
                aa_square_proc, aa_line_hair_proc, aa_poly_hair_proc
            };
            proc = gAAProcs[fMode];
        } else if (fPaint->getStrokeCap() != SkPaint::kRound_Cap) {
            SkASSERT(SkCanvas::kPoints_PointMode == fMode);
            proc = aa_square_proc;
        }
    } else {    // BW
        if (fRadius <= 0.5f) {    // small radii and hairline
            static const Proc gBWProcs[] = {
                bw_pt_hair_proc, bw_line_hair_proc, bw_poly_hair_proc
            };
            proc = gBWProcs[fMode];
        } else {
            proc = bw_square_proc;
        }
    }
    return proc;
}

// each of these costs 8-bytes of stack space, so don't make it too large
// must be even for lines/polygon to work
#define MAX_DEV_PTS     32

void Draw::drawPoints(SkCanvas::PointMode mode,
                      SkSpan<const SkPoint> points,
                      const SkPaint& paint,
                      SkDevice* device) const {
    // if we're in lines mode, force count to be even
    if (SkCanvas::kLines_PointMode == mode) {
        points = points.first(points.size() & ~1);   // force it to be even
    }

    SkDEBUGCODE(this->validate();)

     // nothing to draw
    if (points.empty() || fRC->isEmpty()) {
        return;
    }

    PtProcRec rec;
    if (!device && rec.init(mode, paint, fCTM, fRC)) {
        // Can't easily get bounds of points so don't try.
        SkAutoBlitterChoose blitter(*this, nullptr, paint, SkRect::MakeEmpty());

        SkPoint             devPts[MAX_DEV_PTS];
        SkBlitter*          bltr = blitter.get();
        PtProcRec::Proc     proc = rec.chooseProc(&bltr);
        // we have to back up subsequent passes if we're in polygon mode
        const size_t backup = (SkCanvas::kPolygon_PointMode == mode);

        auto count = points.size();
        auto pts = points.data();
        do {
            size_t n = count;
            if (n > MAX_DEV_PTS) {
                n = MAX_DEV_PTS;
            }
            fCTM->mapPoints({devPts, n}, {pts, n});
            if (!SkIsFinite(&devPts[0].fX, n * 2)) {
                return;
            }
            proc(rec, {devPts, n}, bltr);
            pts += n - backup;
            SkASSERT(count >= n);
            count -= n;
            if (count > 0) {
                count += backup;
            }
        } while (count != 0);
    } else {
        this->drawDevicePoints(mode, points, paint, device);
    }
}

static bool clipped_out(const SkMatrix& m, const SkRasterClip& c,
                        const SkRect& srcR) {
    SkRect  dstR;
    m.mapRect(&dstR, srcR);
    return c.quickReject(dstR.roundOut());
}

static bool clipped_out(const SkMatrix& matrix, const SkRasterClip& clip,
                        int width, int height) {
    SkRect  r;
    r.setIWH(width, height);
    return clipped_out(matrix, clip, r);
}

static bool clipHandlesSprite(const SkRasterClip& clip, int x, int y, const SkPixmap& pmap) {
    return clip.isBW() || clip.quickContains(SkIRect::MakeXYWH(x, y, pmap.width(), pmap.height()));
}

void Draw::drawBitmap(const SkBitmap& bitmap,
                      const SkMatrix& prematrix,
                      const SkRect* dstBounds,
                      const SkSamplingOptions& sampling,
                      const SkPaint& origPaint) const {
    SkDEBUGCODE(this->validate();)

    // nothing to draw
    if (fRC->isEmpty() ||
            bitmap.width() == 0 || bitmap.height() == 0 ||
            bitmap.colorType() == kUnknown_SkColorType) {
        return;
    }

    SkTCopyOnFirstWrite<SkPaint> paint(origPaint);
    if (origPaint.getStyle() != SkPaint::kFill_Style) {
        paint.writable()->setStyle(SkPaint::kFill_Style);
    }

    SkMatrix matrix = *fCTM * prematrix;

    if (clipped_out(matrix, *fRC, bitmap.width(), bitmap.height())) {
        return;
    }

    if (!SkColorTypeIsAlphaOnly(bitmap.colorType()) &&
        SkTreatAsSprite(matrix, bitmap.dimensions(), sampling, paint->isAntiAlias())) {
        //
        // It is safe to call lock pixels now, since we know the matrix is
        // (more or less) identity.
        //
        SkPixmap pmap;
        if (!bitmap.peekPixels(&pmap)) {
            return;
        }
        int ix = SkScalarRoundToInt(matrix.getTranslateX());
        int iy = SkScalarRoundToInt(matrix.getTranslateY());
        if (clipHandlesSprite(*fRC, ix, iy, pmap)) {
            SkSTArenaAlloc<kSkBlitterContextSize> allocator;
            // blitter will be owned by the allocator.
            SkBlitter* blitter = SkBlitter::ChooseSprite(fDst, *paint, pmap, ix, iy, &allocator,
                                                         fRC->clipShader());
            if (blitter) {
                SkScan::FillIRect(SkIRect::MakeXYWH(ix, iy, pmap.width(), pmap.height()),
                                  *fRC, blitter);
                return;
            }
            // if !blitter, then we fall-through to the slower case
        }
    }

    // now make a temp draw on the stack, and use it
    Draw draw(*this);
    draw.fCTM = &matrix;

    // For a long time, the CPU backend treated A8 bitmaps as coverage, rather than alpha. This was
    // inconsistent with the GPU backend (skbug.com/40041022). When this was fixed, it altered behavior
    // for some Android apps (b/231400686). Thus: keep the old behavior in the framework.
#if defined(SK_SUPPORT_LEGACY_ALPHA_BITMAP_AS_COVERAGE)
    if (bitmap.colorType() == kAlpha_8_SkColorType && !paint->getColorFilter()) {
        draw.drawBitmapAsMask(bitmap, sampling, *paint, nullptr);
        return;
    }
#endif

    SkPaint paintWithShader = make_paint_with_image(*paint, bitmap, sampling);
    const SkRect srcBounds = SkRect::MakeIWH(bitmap.width(), bitmap.height());
    if (dstBounds) {
        this->drawRect(srcBounds, paintWithShader, &prematrix, dstBounds);
    } else {
        draw.drawRect(srcBounds, paintWithShader);
    }
}

void Draw::drawSprite(const SkBitmap& bitmap, int x, int y, const SkPaint& origPaint) const {
    SkDEBUGCODE(this->validate();)

    // nothing to draw
    if (fRC->isEmpty() ||
            bitmap.width() == 0 || bitmap.height() == 0 ||
            bitmap.colorType() == kUnknown_SkColorType) {
        return;
    }

    const SkIRect bounds = SkIRect::MakeXYWH(x, y, bitmap.width(), bitmap.height());

    if (fRC->quickReject(bounds)) {
        return; // nothing to draw
    }

    SkPaint paint(origPaint);
    paint.setStyle(SkPaint::kFill_Style);

    SkPixmap pmap;
    if (!bitmap.peekPixels(&pmap)) {
        return;
    }

    if (nullptr == paint.getColorFilter() && clipHandlesSprite(*fRC, x, y, pmap)) {
        // blitter will be owned by the allocator.
        SkSTArenaAlloc<kSkBlitterContextSize> allocator;
        SkBlitter* blitter = SkBlitter::ChooseSprite(fDst, paint, pmap, x, y, &allocator,
                                                     fRC->clipShader());
        if (blitter) {
            SkScan::FillIRect(bounds, *fRC, blitter);
            return;
        }
    }

    SkMatrix matrix;
    SkRect   r;

    // get a scalar version of our rect
    r.set(bounds);

    // create shader with offset
    matrix.setTranslate(r.fLeft, r.fTop);
    SkPaint paintWithShader = make_paint_with_image(paint, bitmap, SkSamplingOptions(), &matrix);
    Draw draw(*this);
    draw.fCTM = &SkMatrix::I();
    // call ourself with a rect
    draw.drawRect(r, paintWithShader);
}

void Draw::drawDevMask(const SkMask& srcM,
                       const SkPaint& paint,
                       const SkMatrix* paintMatrix) const {
    if (srcM.fBounds.isEmpty()) {
        return;
    }

    const SkMask* mask = &srcM;

    SkMaskBuilder dstM;
    if (paint.getMaskFilter() &&
        as_MFB(paint.getMaskFilter())->filterMask(&dstM, srcM, *fCTM, nullptr)) {
        mask = &dstM;
    }
    SkAutoMaskFreeImage ami(dstM.image());

    SkAutoBlitterChoose blitterChooser(*this, paintMatrix, paint, SkRect::Make(dstM.bounds()));
    SkBlitter* blitter = blitterChooser.get();

    SkAAClipBlitterWrapper wrapper;
    const SkRegion* clipRgn;

    if (fRC->isBW()) {
        clipRgn = &fRC->bwRgn();
    } else {
        wrapper.init(*fRC, blitter);
        clipRgn = &wrapper.getRgn();
        blitter = wrapper.getBlitter();
    }
    blitter->blitMaskRegion(*mask, *clipRgn);
}

void Draw::drawBitmapAsMask(const SkBitmap& bitmap,
                            const SkSamplingOptions& sampling,
                            const SkPaint& paint,
                            const SkMatrix* paintMatrix) const {
    SkASSERT(bitmap.colorType() == kAlpha_8_SkColorType);

    // nothing to draw
    if (fRC->isEmpty()) {
        return;
    }

    if (SkTreatAsSprite(*fCTM, bitmap.dimensions(), sampling, paint.isAntiAlias()))
    {
        int ix = SkScalarRoundToInt(fCTM->getTranslateX());
        int iy = SkScalarRoundToInt(fCTM->getTranslateY());

        SkPixmap pmap;
        if (!bitmap.peekPixels(&pmap)) {
            return;
        }
        SkMask mask(pmap.addr8(0, 0),
                    SkIRect::MakeXYWH(ix, iy, pmap.width(), pmap.height()),
                    SkToU32(pmap.rowBytes()),
                    SkMask::kA8_Format);

        this->drawDevMask(mask, paint, paintMatrix);
    } else {    // need to xform the bitmap first
        SkRect  r;
        SkMaskBuilder mask;

        r.setIWH(bitmap.width(), bitmap.height());
        fCTM->mapRect(&r);
        r.round(&mask.bounds());

        // set the mask's bounds to the transformed bitmap-bounds,
        // clipped to the actual device and further limited by the clip bounds
        {
            SkASSERT(fDst.bounds().contains(fRC->getBounds()));
            SkIRect devBounds = fDst.bounds();
            devBounds.intersect(fRC->getBounds().makeOutset(1, 1));
            // need intersect(l, t, r, b) on irect
            if (!mask.bounds().intersect(devBounds)) {
                return;
            }
        }

        mask.format() = SkMask::kA8_Format;
        mask.rowBytes() = SkAlign4(mask.fBounds.width());
        size_t size = mask.computeImageSize();
        if (0 == size) {
            // the mask is too big to allocated, draw nothing
            return;
        }

        // allocate (and clear) our temp buffer to hold the transformed bitmap
        AutoTMalloc<uint8_t> storage(size);
        mask.image() = storage.get();
        memset(mask.image(), 0, size);

        // now draw our bitmap(src) into mask(dst), transformed by the matrix
        {
            SkBitmap    device;
            device.installPixels(SkImageInfo::MakeA8(mask.fBounds.width(), mask.fBounds.height()),
                                 mask.image(), mask.fRowBytes);

            SkCanvas c(device);
            // need the unclipped top/left for the translate
            c.translate(-SkIntToScalar(mask.fBounds.fLeft),
                        -SkIntToScalar(mask.fBounds.fTop));
            c.concat(*fCTM);

            // We can't call drawBitmap, or we'll infinitely recurse. Instead
            // we manually build a shader and draw that into our new mask
            SkPaint tmpPaint;
            tmpPaint.setAntiAlias(paint.isAntiAlias());
            tmpPaint.setDither(paint.isDither());
            SkPaint paintWithShader = make_paint_with_image(tmpPaint, bitmap, sampling);
            SkRect rr;
            rr.setIWH(bitmap.width(), bitmap.height());
            c.drawRect(rr, paintWithShader);
        }
        this->drawDevMask(mask, paint, paintMatrix);
    }
}

///////////////////////////////////////////////////////////////////////////////

bool Draw::computeConservativeLocalClipBounds(SkRect* localBounds) const {
    if (fRC->isEmpty()) {
        return false;
    }

    if (auto inverse = fCTM->invert()) {
        SkIRect devBounds = fRC->getBounds();
        // outset to have slop for antialasing and hairlines
        devBounds.outset(1, 1);
        inverse->mapRect(localBounds, SkRect::Make(devBounds));
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////

void Draw::drawPaint(const SkPaint& paint) const {
    SkDEBUGCODE(this->validate();)

    if (fRC->isEmpty()) {
        return;
    }

    SkIRect devRect;
    devRect.setWH(fDst.width(), fDst.height());

    SkAutoBlitterChoose blitter(*this, nullptr, paint, SkRect::Make(devRect));
    SkScan::FillIRect(devRect, *fRC, blitter.get());
}

///////////////////////////////////////////////////////////////////////////////

static inline SkPoint compute_stroke_size(const SkPaint& paint, const SkMatrix& matrix) {
    SkASSERT(matrix.rectStaysRect());
    SkASSERT(SkPaint::kFill_Style != paint.getStyle());

    SkVector size = matrix.mapVector({paint.getStrokeWidth(), paint.getStrokeWidth()});
    return SkPoint::Make(SkScalarAbs(size.fX), SkScalarAbs(size.fY));
}

static bool easy_rect_join(const SkRect& rect,
                           const SkPaint& paint,
                           const SkMatrix& matrix,
                           SkPoint* strokeSize) {
    if (rect.isEmpty() || SkPaint::kMiter_Join != paint.getStrokeJoin() ||
        paint.getStrokeMiter() < SK_ScalarSqrt2) {
        return false;
    }

    *strokeSize = compute_stroke_size(paint, matrix);
    return true;
}

Draw::RectType Draw::ComputeRectType(const SkRect& rect,
                                     const SkPaint& paint,
                                     const SkMatrix& matrix,
                                     SkPoint* strokeSize) {
    const SkScalar width = paint.getStrokeWidth();
    const bool zeroWidth = (0 == width);
    SkPaint::Style style = paint.getStyle();

    if ((SkPaint::kStrokeAndFill_Style == style) && zeroWidth) {
        style = SkPaint::kFill_Style;
    }

    if (paint.getPathEffect() || paint.getMaskFilter() || !matrix.rectStaysRect() ||
        SkPaint::kStrokeAndFill_Style == style) {
        return RectType::kPath;
    }
    if (SkPaint::kFill_Style == style) {
        return RectType::kFill;
    }
    if (zeroWidth) {
        return RectType::kHair;
    }
    if (easy_rect_join(rect, paint, matrix, strokeSize)) {
        return RectType::kStroke;
    }
    return RectType::kPath;
}

static SkSpan<const SkPoint> rect_points(const SkRect& r) {
    return {reinterpret_cast<const SkPoint*>(&r), 2};
}


static SkSpan<SkPoint> rect_points(SkRect& r) { return {reinterpret_cast<SkPoint*>(&r), 2}; }

static void draw_rect_as_path(const Draw& orig,
                              const SkRect& prePaintRect,
                              const SkPaint& paint,
                              const SkMatrix& ctm) {
    Draw draw(orig);
    draw.fCTM = &ctm;
    draw.drawPath(SkPath::Rect(prePaintRect), paint, nullptr);
}

void Draw::drawRect(const SkRect& prePaintRect,
                    const SkPaint& paint,
                    const SkMatrix* paintMatrix,
                    const SkRect* postPaintRect) const {
    SkDEBUGCODE(this->validate();)

    // nothing to draw
    if (fRC->isEmpty()) {
        return;
    }

    SkTCopyOnFirstWrite<SkMatrix> matrix(fCTM);
    if (paintMatrix) {
        SkASSERT(postPaintRect);
        matrix.writable()->preConcat(*paintMatrix);
    } else {
        SkASSERT(!postPaintRect);
    }

    SkPoint strokeSize;
    RectType rtype = ComputeRectType(prePaintRect, paint, *fCTM, &strokeSize);

    if (RectType::kPath == rtype) {
        draw_rect_as_path(*this, prePaintRect, paint, *matrix);
        return;
    }

    SkRect devRect;
    const SkRect& paintRect = paintMatrix ? *postPaintRect : prePaintRect;
    // skip the paintMatrix when transforming the rect by the CTM
    fCTM->mapPoints(rect_points(devRect), rect_points(paintRect));
    devRect.sort();

    // look for the quick exit, before we build a blitter
    SkRect bbox = devRect;
    if (paint.getStyle() != SkPaint::kFill_Style) {
        // extra space for hairlines
        if (paint.getStrokeWidth() == 0) {
            bbox.outset(1, 1);
        } else {
            // For RectType::kStroke, strokeSize is already computed.
            const SkPoint& ssize = (RectType::kStroke == rtype)
                ? strokeSize
                : compute_stroke_size(paint, *fCTM);
            bbox.outset(SkScalarHalf(ssize.x()), SkScalarHalf(ssize.y()));
        }
    }
    if (SkPathPriv::TooBigForMath(bbox)) {
        return;
    }

    if (!SkRectPriv::FitsInFixed(bbox) && rtype != RectType::kHair) {
        draw_rect_as_path(*this, prePaintRect, paint, *matrix);
        return;
    }

    SkIRect ir = bbox.roundOut();
    if (fRC->quickReject(ir)) {
        return;
    }

    SkAutoBlitterChoose blitterStorage(*this, matrix, paint, devRect);
    const SkRasterClip& clip = *fRC;
    SkBlitter* blitter = blitterStorage.get();

    // we want to "fill" if we are kFill or kStrokeAndFill, since in the latter
    // case we are also hairline (if we've gotten to here), which devolves to
    // effectively just kFill
    switch (rtype) {
        case RectType::kFill:
            if (paint.isAntiAlias()) {
                SkScan::AntiFillRect(devRect, clip, blitter);
            } else {
                SkScan::FillRect(devRect, clip, blitter);
            }
            break;
        case RectType::kStroke:
            if (paint.isAntiAlias()) {
                SkScan::AntiFrameRect(devRect, strokeSize, clip, blitter);
            } else {
                SkScan::FrameRect(devRect, strokeSize, clip, blitter);
            }
            break;
        case RectType::kHair:
            if (paint.isAntiAlias()) {
                SkScan::AntiHairRect(devRect, clip, blitter);
            } else {
                SkScan::HairRect(devRect, clip, blitter);
            }
            break;
        default:
            SkDEBUGFAIL("bad rtype");
    }
}

static SkScalar fast_len(const SkVector& vec) {
    SkScalar x = SkScalarAbs(vec.fX);
    SkScalar y = SkScalarAbs(vec.fY);
    if (x < y) {
        using std::swap;
        swap(x, y);
    }
    return x + SkScalarHalf(y);
}

bool DrawTreatAAStrokeAsHairline(SkScalar strokeWidth, const SkMatrix& matrix, SkScalar* coverage) {
    SkASSERT(strokeWidth > 0);
    // We need to try to fake a thick-stroke with a modulated hairline.

    if (matrix.hasPerspective()) {
        return false;
    }

    SkVector src[2], dst[2];
    src[0].set(strokeWidth, 0);
    src[1].set(0, strokeWidth);
    matrix.mapVectors(dst, src);
    SkScalar len0 = fast_len(dst[0]);
    SkScalar len1 = fast_len(dst[1]);
    if (len0 <= SK_Scalar1 && len1 <= SK_Scalar1) {
        if (coverage) {
            *coverage = sk_float_midpoint(len0, len1);
        }
        return true;
    }
    return false;
}

void Draw::drawOval(const SkRect& oval, const SkPaint& paint) const {
    SkDEBUGCODE(this->validate();)

    if (fRC->isEmpty()) {
        return;
    }

    this->drawPath(SkPath::Oval(oval), paint, nullptr);
}

void Draw::drawRRect(const SkRRect& rrect, const SkPaint& paint) const {
    SkDEBUGCODE(this->validate();)

    if (fRC->isEmpty()) {
        return;
    }

    {
        // TODO: Investigate optimizing these options. They are in the same
        // order as skcpu::Draw::drawPath, which handles each case. It may be
        // that there is no way to optimize for these using the SkRRect path.
        SkScalar coverage;
        if (skcpu::DrawTreatAsHairline(paint, *fCTM, &coverage)) {
            goto DRAW_PATH;
        }

        if (paint.getPathEffect() || paint.getStyle() != SkPaint::kFill_Style) {
            goto DRAW_PATH;
        }
    }

    if (paint.getMaskFilter()) {
        if (this->drawRRectNinePatch(rrect, paint)) {
            return;
        }
    }

DRAW_PATH:
    // Now fall back to the default case of using a path.
    this->drawPath(SkPath::RRect(rrect), paint, nullptr);
}

bool Draw::drawRRectNinePatch(const SkRRect& rrect, const SkPaint& paint) const {
    SkASSERT(paint.getMaskFilter());

    if (auto rr = rrect.transform(*fCTM)) {
        SkAutoBlitterChoose blitter(*this, nullptr, paint, rrect.getBounds());
        SkResourceCache* cache = nullptr;  // TODO(kjlubick) get this from fCtx
        const SkMaskFilterBase* maskFilter = as_MFB(paint.getMaskFilter());
        if (rrect.getType() == SkRRect::kRect_Type) {
            SkRect devRect = rr->rect();
            if (maskFilter->filterRects(SkSpan(&devRect, 1), *fCTM, *fRC, blitter.get(), cache) ==
                SkMaskFilterBase::FilterReturn::kTrue) {
                return true;
            }
        } else {
            if (maskFilter->filterRRect(*rr, *fCTM, *fRC, blitter.get(), cache)) {
                return true;  // filterRRect() called the blitter, so we're done
            }
        }
    }
    return false;
}

void Draw::drawDevPath(const SkPathRaw& raw,
                       const SkPaint& paint,
                       SkDrawCoverage drawCoverage,
                       SkBlitter* customBlitter,
                       bool doFill) const {
    if (SkPathPriv::TooBigForMath(raw.bounds())) {
        return;
    }

    SkBlitter* blitter = nullptr;
    SkAutoBlitterChoose blitterStorage;
    if (nullptr == customBlitter) {
        blitter = blitterStorage.choose(*this, nullptr, paint, raw.bounds(), drawCoverage);
    } else {
        blitter = customBlitter;
    }

    if (paint.getMaskFilter()) {
        SkStrokeRec::InitStyle style = doFill ? SkStrokeRec::kFill_InitStyle
                                              : SkStrokeRec::kHairline_InitStyle;
        SkResourceCache* cache = nullptr;  // TODO(kjlubick) get this from fCtx
        if (as_MFB(paint.getMaskFilter())->filterPath(raw, *fCTM, *fRC, blitter, style, cache)) {
            return;  // filterPath() called the blitter, so we're done
        }
    }

    void (*proc)(const SkPathRaw&, const SkRasterClip&, SkBlitter*);
    if (doFill) {
        if (paint.isAntiAlias()) {
            proc = SkScan::AntiFillPath;
        } else {
            proc = SkScan::FillPath;
        }
    } else {  // hairline
        if (paint.isAntiAlias()) {
            switch (paint.getStrokeCap()) {
                case SkPaint::kButt_Cap:
                    proc = SkScan::AntiHairPath;
                    break;
                case SkPaint::kSquare_Cap:
                    proc = SkScan::AntiHairSquarePath;
                    break;
                case SkPaint::kRound_Cap:
                    proc = SkScan::AntiHairRoundPath;
                    break;
            }
        } else {
            switch (paint.getStrokeCap()) {
                case SkPaint::kButt_Cap:
                    proc = SkScan::HairPath;
                    break;
                case SkPaint::kSquare_Cap:
                    proc = SkScan::HairSquarePath;
                    break;
                case SkPaint::kRound_Cap:
                    proc = SkScan::HairRoundPath;
                    break;
            }
        }
    }
    proc(raw, *fRC, blitter);
}

/*
 *  Tricky idea: can we treat thin strokes as hairlines? If so, depending on how
 *  thin, we may decide to modulate the paint's alpha to 'simulate' very think
 *  strokes, even though hairline is always 1-pixel wide.
 *
 *  The motivation at the time was performance: hairlines draw faster than constructing
 *  the inner/outer contours and filling that (as we do for normal stroking).
 *
 *  Questionable decision, since our hairline algorithm draws each segment of the path
 *  separately, meaning a path that crosses itself can have blending artifacts.
 *  Note: this doesn't happen with normal stroking, as the built inner/outer path
 *  never double-hits a pixel.
 */
static std::optional<SkPaint> modifyPaintForHairlines(const SkPaint& origPaint,
                                                      const SkMatrix& matrix) {
    float coverage;
    if (DrawTreatAsHairline(origPaint, matrix, &coverage)) {
        const auto bm = origPaint.asBlendMode();
        if (coverage == 1) {
            SkPaint paint(origPaint);
            paint.setStrokeWidth(0);
            return paint;
        } else if (bm && SkBlendMode_SupportsCoverageAsAlpha(bm.value())) {
            U8CPU newAlpha;
#if 0
            newAlpha = SkToU8(SkScalarRoundToInt(coverage * origPaint.getAlpha()));
#else
            // this is the old technique, which we preserve for now so
            // we don't change previous results (testing)
            // the new way seems fine, its just (a tiny bit) different
            int scale = (int)(coverage * 256);
            newAlpha = origPaint.getAlpha() * scale >> 8;
#endif
            SkPaint paint(origPaint);
            paint.setStrokeWidth(0);
            paint.setAlpha(newAlpha);
            return paint;
        }
    }
    return {};
}

void Draw::drawPath(const SkPath& origSrcPath,
                    const SkPaint& origPaint,
                    const SkMatrix* prePathMatrix,
                    SkDrawCoverage drawCoverage,
                    SkBlitter* customBlitter) const {
    SkDEBUGCODE(this->validate();)

    // nothing to draw
    if (fRC->isEmpty()) {
        return;
    }

    std::optional<SkPaint> newPaint = modifyPaintForHairlines(origPaint, *fCTM);
    const SkPaint* paint = newPaint.has_value() ? &newPaint.value()
                                                : &origPaint;

    const bool needsFillPath = paint->getPathEffect() || paint->getStyle() != SkPaint::kFill_Style;

    SkPathBuilder builder;
    std::optional<SkPathRaw> raw;      // will point to either origSrcPath or builder
    bool          doFill = true;

    sk_sp<SkPathData> pdata;

    if (needsFillPath) {
        SkRect cullRect;
        const SkRect* cullRectPtr = nullptr;
        if (this->computeConservativeLocalClipBounds(&cullRect)) {
            cullRectPtr = &cullRect;
        }

        std::optional<SkPath> prePathStorage;
        const SkPath* pathPtr = &origSrcPath;
        if (prePathMatrix) {
            prePathStorage = pathPtr->tryMakeTransform(*prePathMatrix);
            if (!prePathStorage.has_value()) {
                return;
            }
            pathPtr = &prePathStorage.value();
        }
        doFill = skpathutils::FillPathWithPaint(*pathPtr, *paint, &builder, cullRectPtr, *fCTM);
        builder.transform(*fCTM);
        raw = SkPathPriv::Raw(builder, SkResolveConvexity::kYes);
    } else {
        SkMatrix matrix = *fCTM;
        if (prePathMatrix) {
            matrix.preConcat(*prePathMatrix);
        }

        if (matrix.isIdentity()) {
            raw = SkPathPriv::Raw(origSrcPath, SkResolveConvexity::kYes);
        } else {
            raw = SkPathPriv::Raw(origSrcPath, SkResolveConvexity::kNo);
            if (raw && (pdata = SkPathData::MakeTransform(*raw, matrix))) {
                raw = pdata->raw(origSrcPath.getFillType(), SkResolveConvexity::kYes);
            } else {
                return; // failed to create pdata
            }
        }
    }

    if (!raw) {
        return;
    }

#if defined(SK_BUILD_FOR_FUZZER)
    if (raw->points().size() > 1000) {
        return;
    }
#endif

    this->drawDevPath(*raw, *paint, drawCoverage, customBlitter, doFill);
}

////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

void Draw::validate() const {
    SkASSERT(fCTM != nullptr);
    SkASSERT(fRC != nullptr);

    const SkIRect& cr = fRC->getBounds();
    SkIRect br;

    br.setWH(fDst.width(), fDst.height());
    SkASSERT(cr.isEmpty() || br.contains(cr));
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////

static bool compute_mask_bounds(const SkRect& devPathBounds,
                                const SkIRect& clipBounds,
                                const SkMaskFilter* filter,
                                const SkMatrix* filterMatrix,
                                SkIRect* bounds) {
    SkASSERT(filter);
    SkASSERT(filterMatrix);
    //  init our bounds from the path
    *bounds = devPathBounds.makeOutset(SK_ScalarHalf, SK_ScalarHalf).roundOut();

    SkIVector margin = SkIPoint::Make(0, 0);
    SkMask srcM(nullptr, *bounds, 0, SkMask::kA8_Format);
    SkMaskBuilder dstM;
    if (!as_MFB(filter)->filterMask(&dstM, srcM, *filterMatrix, &margin)) {
        return false;
    }

    // trim the bounds to reflect the clip (plus whatever slop the filter needs)
    // Ugh. Guard against gigantic margins from wacky filters. Without this
    // check we can request arbitrary amounts of slop beyond our visible
    // clip, and bring down the renderer (at least on finite RAM machines
    // like handsets, etc.). Need to balance this invented value between
    // quality of large filters like blurs, and the corresponding memory
    // requests.
    static constexpr int kMaxMargin = 128;
    if (!bounds->intersect(clipBounds.makeOutset(std::min(margin.fX, kMaxMargin),
                                                 std::min(margin.fY, kMaxMargin)))) {
        return false;
    }

    return true;
}

static void draw_into_mask(const SkMask& mask,
                           SkPathRaw raw,
                           SkStrokeRec::InitStyle style) {
    SkPixmap dst;
    if (!dst.reset(mask)) {
        return;
    }

    const float dx = -mask.fBounds.fLeft,
                dy = -mask.fBounds.fTop;
    const SkMatrix translate = SkMatrix::Translate(dx, dy);

    SkPaint paint;
    paint.setAntiAlias(true);
    SkBlitterSizedArena alloc;
    SkBlitter* blitter = SkChooseA8Blitter(dst, translate, paint, &alloc,
                                           SkDrawCoverage::kNo, nullptr);


    // transform a copy of the points, so we can apply the ctm/translate
    skia_private::AutoSTArray<32, SkPoint> devPoints(raw.fPoints.size());
    translate.mapPoints(devPoints, raw.fPoints);
    raw.fPoints = devPoints;
    raw.fBounds = raw.fBounds.makeOffset(dx, dy);
    if (!raw.fBounds.isFinite()) {
        return;
    }

    const SkRasterClip clip(SkIRect::MakeWH(mask.fBounds.width(), mask.fBounds.height()));

    switch (style) {
        case SkStrokeRec::kHairline_InitStyle:
            SkScan::AntiHairPath(raw, clip, blitter);
            break;
        case SkStrokeRec::kFill_InitStyle:
            SkScan::AntiFillPath(raw, clip, blitter);
            break;
    }
}

bool DrawToMask(const SkPathRaw& devRaw,
                const SkIRect& clipBounds,
                const SkMaskFilter* filter,
                const SkMatrix* filterMatrix,
                SkMaskBuilder* dst,
                SkMaskBuilder::CreateMode mode,
                SkStrokeRec::InitStyle style) {
    SkASSERT(filter);
    if (devRaw.empty()) {
        return false;
    }

    if (SkMaskBuilder::kJustRenderImage_CreateMode != mode) {
        // By using infinite bounds for inverse fills, compute_mask_bounds is able to clip it to
        // 'clipBounds' outset by whatever extra margin the mask filter requires.
        static const SkRect kInverseBounds = {SK_ScalarNegativeInfinity,
                                              SK_ScalarNegativeInfinity,
                                              SK_ScalarInfinity,
                                              SK_ScalarInfinity};
        SkRect pathBounds = devRaw.isInverseFillType() ? kInverseBounds : devRaw.bounds();
        if (!compute_mask_bounds(pathBounds, clipBounds, filter, filterMatrix, &dst->bounds())) {
            return false;
        }
    }

    if (SkMaskBuilder::kComputeBoundsAndRenderImage_CreateMode == mode) {
        dst->format() = SkMask::kA8_Format;
        dst->rowBytes() = dst->fBounds.width();
        size_t size = dst->computeImageSize();
        if (0 == size) {
            // we're too big to allocate the mask, abort
            return false;
        }
        dst->image() = SkMaskBuilder::AllocImage(size, SkMaskBuilder::kZeroInit_Alloc);
    }

    if (SkMaskBuilder::kJustComputeBounds_CreateMode != mode) {
        draw_into_mask(*dst, devRaw, style);
    }
    return true;
}

void Draw::drawDevicePoints(SkCanvas::PointMode mode,
                            SkSpan<const SkPoint> points,
                            const SkPaint& paint,
                            SkDevice* device) const {
    // if we're in lines mode, force count to be even
    if (SkCanvas::kLines_PointMode == mode) {
        points = points.first(points.size() & ~1);  // force it to be even
    }

    SkDEBUGCODE(this->validate();)

     // nothing to draw
    if (points.empty() || fRC->isEmpty()) {
        return;
    }

    // needed?
    if (!SkIsFinite(&points[0].fX, points.size() * 2)) {
        return;
    }

    switch (mode) {
        case SkCanvas::kPoints_PointMode: {
            // temporarily mark the paint as filling.
            SkPaint newPaint(paint);
            newPaint.setStyle(SkPaint::kFill_Style);

            SkScalar width = newPaint.getStrokeWidth();
            SkScalar radius = SkScalarHalf(width);

            if (newPaint.getStrokeCap() == SkPaint::kRound_Cap) {
                if (device) {
                    for (const auto& pt : points) {
                        SkRect r = SkRect::MakeLTRB(pt.fX - radius, pt.fY - radius,
                                                    pt.fX + radius, pt.fY + radius);
                        device->drawOval(r, newPaint);
                    }
                } else {
                    SkPath path = SkPath::Circle(0, 0, radius);
                    SkMatrix preMatrix;

                    for (const auto& pt : points) {
                        preMatrix.setTranslate(pt.fX, pt.fY);
                        this->drawPath(path, newPaint, &preMatrix);
                    }
                }
            } else {
                SkRect r;

                for (const auto& pt : points) {
                    r.fLeft = pt.fX - radius;
                    r.fTop = pt.fY - radius;
                    r.fRight = r.fLeft + width;
                    r.fBottom = r.fTop + width;
                    if (device) {
                        device->drawRect(r, newPaint);
                    } else {
                        this->drawRect(r, newPaint);
                    }
                }
            }
            break;
        }
        case SkCanvas::kLines_PointMode:
            if (2 == points.size() && paint.getPathEffect()) {
                // most likely a dashed line - see if it is one of the ones
                // we can accelerate
                SkStrokeRec stroke(paint);
                SkPathEffectBase::PointData pointData;

                SkPath path = SkPath::Line(points[0], points[1]);

                SkRect cullRect = SkRect::Make(fRC->getBounds());

                if (as_PEB(paint.getPathEffect())
                            ->asPoints(&pointData, path, stroke, *fCTM, &cullRect)) {
                    // 'asPoints' managed to find some fast path

                    SkPaint newP(paint);
                    newP.setPathEffect(nullptr);
                    newP.setStyle(SkPaint::kFill_Style);

                    if (!pointData.fFirst.isEmpty()) {
                        if (device) {
                            device->drawPath(pointData.fFirst, newP);
                        } else {
                            this->drawPath(pointData.fFirst, newP, nullptr);
                        }
                    }

                    if (!pointData.fLast.isEmpty()) {
                        if (device) {
                            device->drawPath(pointData.fLast, newP);
                        } else {
                            this->drawPath(pointData.fLast, newP, nullptr);
                        }
                    }

                    if (pointData.fSize.fX == pointData.fSize.fY) {
                        // The rest of the dashed line can just be drawn as points
                        SkASSERT(pointData.fSize.fX == SkScalarHalf(newP.getStrokeWidth()));

                        if (SkPathEffectBase::PointData::kCircles_PointFlag & pointData.fFlags) {
                            newP.setStrokeCap(SkPaint::kRound_Cap);
                        } else {
                            newP.setStrokeCap(SkPaint::kButt_Cap);
                        }

                        if (device) {
                            device->drawPoints(
                                    SkCanvas::kPoints_PointMode, pointData.points(), newP);
                        } else {
                            this->drawDevicePoints(
                                    SkCanvas::kPoints_PointMode, pointData.points(), newP, device);
                        }
                        break;
                    } else {
                        // The rest of the dashed line must be drawn as rects
                        SkASSERT(!(SkPathEffectBase::PointData::kCircles_PointFlag &
                                   pointData.fFlags));

                        SkRect r;

                        for (const auto& pt : pointData.points()) {
                            r.setLTRB(pt.fX - pointData.fSize.fX,
                                      pt.fY - pointData.fSize.fY,
                                      pt.fX + pointData.fSize.fX,
                                      pt.fY + pointData.fSize.fY);
                            if (device) {
                                device->drawRect(r, newP);
                            } else {
                                this->drawRect(r, newP);
                            }
                        }
                    }

                    break;
                }
            }
            [[fallthrough]];  // couldn't take fast path
        case SkCanvas::kPolygon_PointMode: {
            auto count = points.size() - 1;
            SkPaint p(paint);
            p.setStyle(SkPaint::kStroke_Style);
            size_t inc = (SkCanvas::kLines_PointMode == mode) ? 2 : 1;

            for (size_t i = 0; i < count; i += inc) {
                auto path = SkPath::Line(points[i], points[i + 1]);
                if (device) {
                    device->drawPath(path, p);
                } else {
                    this->drawPath(path, p, nullptr);
                }
            }
            break;
        }
    }
}

}  // namespace skcpu
