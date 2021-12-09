/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkDraw.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPathEffect.h"
#include "include/core/SkRRect.h"
#include "include/core/SkShader.h"
#include "include/core/SkString.h"
#include "include/core/SkStrokeRec.h"
#include "include/private/SkColorData.h"
#include "include/private/SkImageInfoPriv.h"
#include "include/private/SkMacros.h"
#include "include/private/SkTemplates.h"
#include "include/private/SkTo.h"
#include "src/core/SkArenaAlloc.h"
#include "src/core/SkAutoBlitterChoose.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkBlitter.h"
#include "src/core/SkDevice.h"
#include "src/core/SkDrawProcs.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkMatrixUtils.h"
#include "src/core/SkPathEffectBase.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkSamplingPriv.h"
#include "src/core/SkScan.h"
#include "src/core/SkStroke.h"
#include "src/core/SkTLazy.h"
#include "src/core/SkUtils.h"

#include <utility>

static SkPaint make_paint_with_image(const SkPaint& origPaint, const SkBitmap& bitmap,
                                     const SkSamplingOptions& sampling,
                                     SkMatrix* matrix = nullptr) {
    SkPaint paint(origPaint);
    paint.setShader(SkMakeBitmapShaderForPaint(origPaint, bitmap, SkTileMode::kClamp,
                                               SkTileMode::kClamp, sampling, matrix,
                                               kNever_SkCopyPixelsMode));
    return paint;
}

///////////////////////////////////////////////////////////////////////////////

SkDraw::SkDraw() {}

bool SkDraw::computeConservativeLocalClipBounds(SkRect* localBounds) const {
    if (fRC->isEmpty()) {
        return false;
    }

    SkMatrix inverse;
    if (!fMatrixProvider->localToDevice().invert(&inverse)) {
        return false;
    }

    SkIRect devBounds = fRC->getBounds();
    // outset to have slop for antialasing and hairlines
    devBounds.outset(1, 1);
    inverse.mapRect(localBounds, SkRect::Make(devBounds));
    return true;
}

///////////////////////////////////////////////////////////////////////////////

void SkDraw::drawPaint(const SkPaint& paint) const {
    SkDEBUGCODE(this->validate();)

    if (fRC->isEmpty()) {
        return;
    }

    SkIRect    devRect;
    devRect.setWH(fDst.width(), fDst.height());

    SkAutoBlitterChoose blitter(*this, nullptr, paint);
    SkScan::FillIRect(devRect, *fRC, blitter.get());
}

///////////////////////////////////////////////////////////////////////////////

struct PtProcRec {
    SkCanvas::PointMode fMode;
    const SkPaint*  fPaint;
    const SkRegion* fClip;
    const SkRasterClip* fRC;

    // computed values
    SkRect   fClipBounds;
    SkScalar fRadius;

    typedef void (*Proc)(const PtProcRec&, const SkPoint devPts[], int count,
                         SkBlitter*);

    bool init(SkCanvas::PointMode, const SkPaint&, const SkMatrix* matrix,
              const SkRasterClip*);
    Proc chooseProc(SkBlitter** blitter);

private:
    SkAAClipBlitterWrapper fWrapper;
};

static void bw_pt_rect_hair_proc(const PtProcRec& rec, const SkPoint devPts[],
                                 int count, SkBlitter* blitter) {
    SkASSERT(rec.fClip->isRect());
    const SkIRect& r = rec.fClip->getBounds();

    for (int i = 0; i < count; i++) {
        int x = SkScalarFloorToInt(devPts[i].fX);
        int y = SkScalarFloorToInt(devPts[i].fY);
        if (r.contains(x, y)) {
            blitter->blitH(x, y, 1);
        }
    }
}

static void bw_pt_rect_16_hair_proc(const PtProcRec& rec,
                                    const SkPoint devPts[], int count,
                                    SkBlitter* blitter) {
    SkASSERT(rec.fRC->isRect());
    const SkIRect& r = rec.fRC->getBounds();
    uint32_t value;
    const SkPixmap* dst = blitter->justAnOpaqueColor(&value);
    SkASSERT(dst);

    uint16_t* addr = dst->writable_addr16(0, 0);
    size_t    rb = dst->rowBytes();

    for (int i = 0; i < count; i++) {
        int x = SkScalarFloorToInt(devPts[i].fX);
        int y = SkScalarFloorToInt(devPts[i].fY);
        if (r.contains(x, y)) {
            ((uint16_t*)((char*)addr + y * rb))[x] = SkToU16(value);
        }
    }
}

static void bw_pt_rect_32_hair_proc(const PtProcRec& rec,
                                    const SkPoint devPts[], int count,
                                    SkBlitter* blitter) {
    SkASSERT(rec.fRC->isRect());
    const SkIRect& r = rec.fRC->getBounds();
    uint32_t value;
    const SkPixmap* dst = blitter->justAnOpaqueColor(&value);
    SkASSERT(dst);

    SkPMColor* addr = dst->writable_addr32(0, 0);
    size_t     rb = dst->rowBytes();

    for (int i = 0; i < count; i++) {
        int x = SkScalarFloorToInt(devPts[i].fX);
        int y = SkScalarFloorToInt(devPts[i].fY);
        if (r.contains(x, y)) {
            ((SkPMColor*)((char*)addr + y * rb))[x] = value;
        }
    }
}

static void bw_pt_hair_proc(const PtProcRec& rec, const SkPoint devPts[],
                            int count, SkBlitter* blitter) {
    for (int i = 0; i < count; i++) {
        int x = SkScalarFloorToInt(devPts[i].fX);
        int y = SkScalarFloorToInt(devPts[i].fY);
        if (rec.fClip->contains(x, y)) {
            blitter->blitH(x, y, 1);
        }
    }
}

static void bw_line_hair_proc(const PtProcRec& rec, const SkPoint devPts[],
                              int count, SkBlitter* blitter) {
    for (int i = 0; i < count; i += 2) {
        SkScan::HairLine(&devPts[i], 2, *rec.fRC, blitter);
    }
}

static void bw_poly_hair_proc(const PtProcRec& rec, const SkPoint devPts[],
                              int count, SkBlitter* blitter) {
    SkScan::HairLine(devPts, count, *rec.fRC, blitter);
}

// aa versions

static void aa_line_hair_proc(const PtProcRec& rec, const SkPoint devPts[],
                              int count, SkBlitter* blitter) {
    for (int i = 0; i < count; i += 2) {
        SkScan::AntiHairLine(&devPts[i], 2, *rec.fRC, blitter);
    }
}

static void aa_poly_hair_proc(const PtProcRec& rec, const SkPoint devPts[],
                              int count, SkBlitter* blitter) {
    SkScan::AntiHairLine(devPts, count, *rec.fRC, blitter);
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

static void bw_square_proc(const PtProcRec& rec, const SkPoint devPts[],
                           int count, SkBlitter* blitter) {
    for (int i = 0; i < count; i++) {
        SkRect r = make_square_rad(devPts[i], rec.fRadius);
        if (r.intersect(rec.fClipBounds)) {
            SkScan::FillXRect(make_xrect(r), *rec.fRC, blitter);
        }
    }
}

static void aa_square_proc(const PtProcRec& rec, const SkPoint devPts[],
                           int count, SkBlitter* blitter) {
    for (int i = 0; i < count; i++) {
        SkRect r = make_square_rad(devPts[i], rec.fRadius);
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
            if (SkCanvas::kPoints_PointMode == fMode && fClip->isRect()) {
                uint32_t value;
                const SkPixmap* bm = blitter->justAnOpaqueColor(&value);
                if (bm && kRGB_565_SkColorType == bm->colorType()) {
                    proc = bw_pt_rect_16_hair_proc;
                } else if (bm && kN32_SkColorType == bm->colorType()) {
                    proc = bw_pt_rect_32_hair_proc;
                } else {
                    proc = bw_pt_rect_hair_proc;
                }
            } else {
                static Proc gBWProcs[] = {
                    bw_pt_hair_proc, bw_line_hair_proc, bw_poly_hair_proc
                };
                proc = gBWProcs[fMode];
            }
        } else {
            proc = bw_square_proc;
        }
    }
    return proc;
}

// each of these costs 8-bytes of stack space, so don't make it too large
// must be even for lines/polygon to work
#define MAX_DEV_PTS     32

void SkDraw::drawPoints(SkCanvas::PointMode mode, size_t count,
                        const SkPoint pts[], const SkPaint& paint,
                        SkBaseDevice* device) const {
    // if we're in lines mode, force count to be even
    if (SkCanvas::kLines_PointMode == mode) {
        count &= ~(size_t)1;
    }

    if ((long)count <= 0) {
        return;
    }

    SkASSERT(pts != nullptr);
    SkDEBUGCODE(this->validate();)

     // nothing to draw
    if (fRC->isEmpty()) {
        return;
    }

    SkMatrix ctm = fMatrixProvider->localToDevice();
    PtProcRec rec;
    if (!device && rec.init(mode, paint, &ctm, fRC)) {
        SkAutoBlitterChoose blitter(*this, nullptr, paint);

        SkPoint             devPts[MAX_DEV_PTS];
        SkBlitter*          bltr = blitter.get();
        PtProcRec::Proc     proc = rec.chooseProc(&bltr);
        // we have to back up subsequent passes if we're in polygon mode
        const size_t backup = (SkCanvas::kPolygon_PointMode == mode);

        do {
            int n = SkToInt(count);
            if (n > MAX_DEV_PTS) {
                n = MAX_DEV_PTS;
            }
            ctm.mapPoints(devPts, pts, n);
            if (!SkScalarsAreFinite(&devPts[0].fX, n * 2)) {
                return;
            }
            proc(rec, devPts, n, bltr);
            pts += n - backup;
            SkASSERT(SkToInt(count) >= n);
            count -= n;
            if (count > 0) {
                count += backup;
            }
        } while (count != 0);
    } else {
        switch (mode) {
            case SkCanvas::kPoints_PointMode: {
                // temporarily mark the paint as filling.
                SkPaint newPaint(paint);
                newPaint.setStyle(SkPaint::kFill_Style);

                SkScalar width = newPaint.getStrokeWidth();
                SkScalar radius = SkScalarHalf(width);

                if (newPaint.getStrokeCap() == SkPaint::kRound_Cap) {
                    if (device) {
                        for (size_t i = 0; i < count; ++i) {
                            SkRect r = SkRect::MakeLTRB(pts[i].fX - radius, pts[i].fY - radius,
                                                        pts[i].fX + radius, pts[i].fY + radius);
                            device->drawOval(r, newPaint);
                        }
                    } else {
                        SkPath     path;
                        SkMatrix   preMatrix;

                        path.addCircle(0, 0, radius);
                        for (size_t i = 0; i < count; i++) {
                            preMatrix.setTranslate(pts[i].fX, pts[i].fY);
                            // pass true for the last point, since we can modify
                            // then path then
                            path.setIsVolatile((count-1) == i);
                            this->drawPath(path, newPaint, &preMatrix, (count-1) == i);
                        }
                    }
                } else {
                    SkRect  r;

                    for (size_t i = 0; i < count; i++) {
                        r.fLeft = pts[i].fX - radius;
                        r.fTop = pts[i].fY - radius;
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
                if (2 == count && paint.getPathEffect()) {
                    // most likely a dashed line - see if it is one of the ones
                    // we can accelerate
                    SkStrokeRec stroke(paint);
                    SkPathEffectBase::PointData pointData;

                    SkPath path = SkPath::Line(pts[0], pts[1]);

                    SkRect cullRect = SkRect::Make(fRC->getBounds());

                    if (as_PEB(paint.getPathEffect())->asPoints(&pointData, path, stroke, ctm,
                                                                &cullRect)) {
                        // 'asPoints' managed to find some fast path

                        SkPaint newP(paint);
                        newP.setPathEffect(nullptr);
                        newP.setStyle(SkPaint::kFill_Style);

                        if (!pointData.fFirst.isEmpty()) {
                            if (device) {
                                device->drawPath(pointData.fFirst, newP);
                            } else {
                                this->drawPath(pointData.fFirst, newP);
                            }
                        }

                        if (!pointData.fLast.isEmpty()) {
                            if (device) {
                                device->drawPath(pointData.fLast, newP);
                            } else {
                                this->drawPath(pointData.fLast, newP);
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
                                device->drawPoints(SkCanvas::kPoints_PointMode,
                                                   pointData.fNumPoints,
                                                   pointData.fPoints,
                                                   newP);
                            } else {
                                this->drawPoints(SkCanvas::kPoints_PointMode,
                                                 pointData.fNumPoints,
                                                 pointData.fPoints,
                                                 newP,
                                                 device);
                            }
                            break;
                        } else {
                            // The rest of the dashed line must be drawn as rects
                            SkASSERT(!(SkPathEffectBase::PointData::kCircles_PointFlag &
                                      pointData.fFlags));

                            SkRect r;

                            for (int i = 0; i < pointData.fNumPoints; ++i) {
                                r.setLTRB(pointData.fPoints[i].fX - pointData.fSize.fX,
                                          pointData.fPoints[i].fY - pointData.fSize.fY,
                                          pointData.fPoints[i].fX + pointData.fSize.fX,
                                          pointData.fPoints[i].fY + pointData.fSize.fY);
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
                [[fallthrough]]; // couldn't take fast path
            case SkCanvas::kPolygon_PointMode: {
                count -= 1;
                SkPath path;
                SkPaint p(paint);
                p.setStyle(SkPaint::kStroke_Style);
                size_t inc = (SkCanvas::kLines_PointMode == mode) ? 2 : 1;
                path.setIsVolatile(true);
                for (size_t i = 0; i < count; i += inc) {
                    path.moveTo(pts[i]);
                    path.lineTo(pts[i+1]);
                    if (device) {
                        device->drawPath(path, p, true);
                    } else {
                        this->drawPath(path, p, nullptr, true);
                    }
                    path.rewind();
                }
                break;
            }
        }
    }
}

static inline SkPoint compute_stroke_size(const SkPaint& paint, const SkMatrix& matrix) {
    SkASSERT(matrix.rectStaysRect());
    SkASSERT(SkPaint::kFill_Style != paint.getStyle());

    SkVector size;
    SkPoint pt = { paint.getStrokeWidth(), paint.getStrokeWidth() };
    matrix.mapVectors(&size, &pt, 1);
    return SkPoint::Make(SkScalarAbs(size.fX), SkScalarAbs(size.fY));
}

static bool easy_rect_join(const SkRect& rect, const SkPaint& paint, const SkMatrix& matrix,
                           SkPoint* strokeSize) {
    if (rect.isEmpty() || SkPaint::kMiter_Join != paint.getStrokeJoin() ||
        paint.getStrokeMiter() < SK_ScalarSqrt2) {
        return false;
    }

    *strokeSize = compute_stroke_size(paint, matrix);
    return true;
}

SkDraw::RectType SkDraw::ComputeRectType(const SkRect& rect,
                                         const SkPaint& paint,
                                         const SkMatrix& matrix,
                                         SkPoint* strokeSize) {
    RectType rtype;
    const SkScalar width = paint.getStrokeWidth();
    const bool zeroWidth = (0 == width);
    SkPaint::Style style = paint.getStyle();

    if ((SkPaint::kStrokeAndFill_Style == style) && zeroWidth) {
        style = SkPaint::kFill_Style;
    }

    if (paint.getPathEffect() || paint.getMaskFilter() ||
        !matrix.rectStaysRect() || SkPaint::kStrokeAndFill_Style == style) {
        rtype = kPath_RectType;
    } else if (SkPaint::kFill_Style == style) {
        rtype = kFill_RectType;
    } else if (zeroWidth) {
        rtype = kHair_RectType;
    } else if (easy_rect_join(rect, paint, matrix, strokeSize)) {
        rtype = kStroke_RectType;
    } else {
        rtype = kPath_RectType;
    }
    return rtype;
}

static const SkPoint* rect_points(const SkRect& r) {
    return reinterpret_cast<const SkPoint*>(&r);
}

static SkPoint* rect_points(SkRect& r) {
    return reinterpret_cast<SkPoint*>(&r);
}

static void draw_rect_as_path(const SkDraw& orig, const SkRect& prePaintRect,
                              const SkPaint& paint, const SkMatrixProvider* matrixProvider) {
    SkDraw draw(orig);
    draw.fMatrixProvider = matrixProvider;
    SkPath  tmp;
    tmp.addRect(prePaintRect);
    tmp.setFillType(SkPathFillType::kWinding);
    draw.drawPath(tmp, paint, nullptr, true);
}

void SkDraw::drawRect(const SkRect& prePaintRect, const SkPaint& paint,
                      const SkMatrix* paintMatrix, const SkRect* postPaintRect) const {
    SkDEBUGCODE(this->validate();)

    // nothing to draw
    if (fRC->isEmpty()) {
        return;
    }

    const SkMatrixProvider* matrixProvider = fMatrixProvider;
    SkTLazy<SkPreConcatMatrixProvider> preConcatMatrixProvider;
    if (paintMatrix) {
        SkASSERT(postPaintRect);
        matrixProvider = preConcatMatrixProvider.init(*matrixProvider, *paintMatrix);
    } else {
        SkASSERT(!postPaintRect);
    }

    SkMatrix ctm = fMatrixProvider->localToDevice();
    SkPoint strokeSize;
    RectType rtype = ComputeRectType(prePaintRect, paint, ctm, &strokeSize);

    if (kPath_RectType == rtype) {
        draw_rect_as_path(*this, prePaintRect, paint, matrixProvider);
        return;
    }

    SkRect devRect;
    const SkRect& paintRect = paintMatrix ? *postPaintRect : prePaintRect;
    // skip the paintMatrix when transforming the rect by the CTM
    ctm.mapPoints(rect_points(devRect), rect_points(paintRect), 2);
    devRect.sort();

    // look for the quick exit, before we build a blitter
    SkRect bbox = devRect;
    if (paint.getStyle() != SkPaint::kFill_Style) {
        // extra space for hairlines
        if (paint.getStrokeWidth() == 0) {
            bbox.outset(1, 1);
        } else {
            // For kStroke_RectType, strokeSize is already computed.
            const SkPoint& ssize = (kStroke_RectType == rtype)
                ? strokeSize
                : compute_stroke_size(paint, ctm);
            bbox.outset(SkScalarHalf(ssize.x()), SkScalarHalf(ssize.y()));
        }
    }
    if (SkPathPriv::TooBigForMath(bbox)) {
        return;
    }

    if (!SkRectPriv::FitsInFixed(bbox) && rtype != kHair_RectType) {
        draw_rect_as_path(*this, prePaintRect, paint, matrixProvider);
        return;
    }

    SkIRect ir = bbox.roundOut();
    if (fRC->quickReject(ir)) {
        return;
    }

    SkAutoBlitterChoose blitterStorage(*this, matrixProvider, paint);
    const SkRasterClip& clip = *fRC;
    SkBlitter*          blitter = blitterStorage.get();

    // we want to "fill" if we are kFill or kStrokeAndFill, since in the latter
    // case we are also hairline (if we've gotten to here), which devolves to
    // effectively just kFill
    switch (rtype) {
        case kFill_RectType:
            if (paint.isAntiAlias()) {
                SkScan::AntiFillRect(devRect, clip, blitter);
            } else {
                SkScan::FillRect(devRect, clip, blitter);
            }
            break;
        case kStroke_RectType:
            if (paint.isAntiAlias()) {
                SkScan::AntiFrameRect(devRect, strokeSize, clip, blitter);
            } else {
                SkScan::FrameRect(devRect, strokeSize, clip, blitter);
            }
            break;
        case kHair_RectType:
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

bool SkDrawTreatAAStrokeAsHairline(SkScalar strokeWidth, const SkMatrix& matrix,
                                   SkScalar* coverage) {
    SkASSERT(strokeWidth > 0);
    // We need to try to fake a thick-stroke with a modulated hairline.

    if (matrix.hasPerspective()) {
        return false;
    }

    SkVector src[2], dst[2];
    src[0].set(strokeWidth, 0);
    src[1].set(0, strokeWidth);
    matrix.mapVectors(dst, src, 2);
    SkScalar len0 = fast_len(dst[0]);
    SkScalar len1 = fast_len(dst[1]);
    if (len0 <= SK_Scalar1 && len1 <= SK_Scalar1) {
        if (coverage) {
            *coverage = SkScalarAve(len0, len1);
        }
        return true;
    }
    return false;
}

void SkDraw::drawRRect(const SkRRect& rrect, const SkPaint& paint) const {
    SkDEBUGCODE(this->validate());

    if (fRC->isEmpty()) {
        return;
    }

    SkMatrix ctm = fMatrixProvider->localToDevice();
    {
        // TODO: Investigate optimizing these options. They are in the same
        // order as SkDraw::drawPath, which handles each case. It may be
        // that there is no way to optimize for these using the SkRRect path.
        SkScalar coverage;
        if (SkDrawTreatAsHairline(paint, ctm, &coverage)) {
            goto DRAW_PATH;
        }

        if (paint.getPathEffect() || paint.getStyle() != SkPaint::kFill_Style) {
            goto DRAW_PATH;
        }
    }

    if (paint.getMaskFilter()) {
        // Transform the rrect into device space.
        SkRRect devRRect;
        if (rrect.transform(ctm, &devRRect)) {
            SkAutoBlitterChoose blitter(*this, nullptr, paint);
            if (as_MFB(paint.getMaskFilter())->filterRRect(devRRect, ctm, *fRC, blitter.get())) {
                return;  // filterRRect() called the blitter, so we're done
            }
        }
    }

DRAW_PATH:
    // Now fall back to the default case of using a path.
    SkPath path;
    path.addRRect(rrect);
    this->drawPath(path, paint, nullptr, true);
}

void SkDraw::drawDevPath(const SkPath& devPath, const SkPaint& paint, bool drawCoverage,
                         SkBlitter* customBlitter, bool doFill) const {
    if (SkPathPriv::TooBigForMath(devPath)) {
        return;
    }
    SkBlitter* blitter = nullptr;
    SkAutoBlitterChoose blitterStorage;
    if (nullptr == customBlitter) {
        blitter = blitterStorage.choose(*this, nullptr, paint, drawCoverage);
    } else {
        blitter = customBlitter;
    }

    if (paint.getMaskFilter()) {
        SkStrokeRec::InitStyle style = doFill ? SkStrokeRec::kFill_InitStyle
                                              : SkStrokeRec::kHairline_InitStyle;
        if (as_MFB(paint.getMaskFilter())
                    ->filterPath(devPath, fMatrixProvider->localToDevice(), *fRC, blitter, style)) {
            return;  // filterPath() called the blitter, so we're done
        }
    }

    void (*proc)(const SkPath&, const SkRasterClip&, SkBlitter*);
    if (doFill) {
        if (paint.isAntiAlias()) {
            proc = SkScan::AntiFillPath;
        } else {
            proc = SkScan::FillPath;
        }
    } else {    // hairline
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
                default:
                    proc SK_INIT_TO_AVOID_WARNING;
                    SkDEBUGFAIL("unknown paint cap type");
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
                default:
                    proc SK_INIT_TO_AVOID_WARNING;
                    SkDEBUGFAIL("unknown paint cap type");
            }
        }
    }

    proc(devPath, *fRC, blitter);
}

void SkDraw::drawPath(const SkPath& origSrcPath, const SkPaint& origPaint,
                      const SkMatrix* prePathMatrix, bool pathIsMutable,
                      bool drawCoverage, SkBlitter* customBlitter) const {
    SkDEBUGCODE(this->validate();)

    // nothing to draw
    if (fRC->isEmpty()) {
        return;
    }

    SkPath*         pathPtr = (SkPath*)&origSrcPath;
    bool            doFill = true;
    SkPath          tmpPathStorage;
    SkPath*         tmpPath = &tmpPathStorage;
    const SkMatrixProvider*            matrixProvider = fMatrixProvider;
    SkTLazy<SkPreConcatMatrixProvider> preConcatMatrixProvider;
    tmpPath->setIsVolatile(true);

    if (prePathMatrix) {
        if (origPaint.getPathEffect() || origPaint.getStyle() != SkPaint::kFill_Style) {
            SkPath* result = pathPtr;

            if (!pathIsMutable) {
                result = tmpPath;
                pathIsMutable = true;
            }
            pathPtr->transform(*prePathMatrix, result);
            pathPtr = result;
        } else {
            matrixProvider = preConcatMatrixProvider.init(*matrixProvider, *prePathMatrix);
        }
    }

    SkTCopyOnFirstWrite<SkPaint> paint(origPaint);

    {
        SkScalar coverage;
        if (SkDrawTreatAsHairline(origPaint, matrixProvider->localToDevice(), &coverage)) {
            const auto bm = origPaint.asBlendMode();
            if (SK_Scalar1 == coverage) {
                paint.writable()->setStrokeWidth(0);
            } else if (bm && SkBlendMode_SupportsCoverageAsAlpha(bm.value())) {
                U8CPU newAlpha;
#if 0
                newAlpha = SkToU8(SkScalarRoundToInt(coverage *
                                                     origPaint.getAlpha()));
#else
                // this is the old technique, which we preserve for now so
                // we don't change previous results (testing)
                // the new way seems fine, its just (a tiny bit) different
                int scale = (int)(coverage * 256);
                newAlpha = origPaint.getAlpha() * scale >> 8;
#endif
                SkPaint* writablePaint = paint.writable();
                writablePaint->setStrokeWidth(0);
                writablePaint->setAlpha(newAlpha);
            }
        }
    }

    if (paint->getPathEffect() || paint->getStyle() != SkPaint::kFill_Style) {
        SkRect cullRect;
        const SkRect* cullRectPtr = nullptr;
        if (this->computeConservativeLocalClipBounds(&cullRect)) {
            cullRectPtr = &cullRect;
        }
        doFill = paint->getFillPath(*pathPtr, tmpPath, cullRectPtr,
                                    fMatrixProvider->localToDevice());
        pathPtr = tmpPath;
    }

    // avoid possibly allocating a new path in transform if we can
    SkPath* devPathPtr = pathIsMutable ? pathPtr : tmpPath;

    // transform the path into device space
    pathPtr->transform(matrixProvider->localToDevice(), devPathPtr);

#if defined(SK_BUILD_FOR_FUZZER)
    if (devPathPtr->countPoints() > 1000) {
        return;
    }
#endif

    this->drawDevPath(*devPathPtr, *paint, drawCoverage, customBlitter, doFill);
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

void SkDraw::drawBitmap(const SkBitmap& bitmap, const SkMatrix& prematrix,
                        const SkRect* dstBounds, const SkSamplingOptions& sampling,
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

    SkPreConcatMatrixProvider matrixProvider(*fMatrixProvider, prematrix);
    SkMatrix matrix = matrixProvider.localToDevice();

    if (clipped_out(matrix, *fRC, bitmap.width(), bitmap.height())) {
        return;
    }

    if (!SkColorTypeIsAlphaOnly(bitmap.colorType()) &&
        SkTreatAsSprite(matrix, bitmap.dimensions(), sampling, *paint)) {
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
    //
    SkDraw draw(*this);
    draw.fMatrixProvider = &matrixProvider;

    SkPaint paintWithShader = make_paint_with_image(*paint, bitmap, sampling);
    const SkRect srcBounds = SkRect::MakeIWH(bitmap.width(), bitmap.height());
    if (dstBounds) {
        this->drawRect(srcBounds, paintWithShader, &prematrix, dstBounds);
    } else {
        draw.drawRect(srcBounds, paintWithShader);
    }
}

void SkDraw::drawSprite(const SkBitmap& bitmap, int x, int y, const SkPaint& origPaint) const {
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

    SkMatrix        matrix;
    SkRect          r;

    // get a scalar version of our rect
    r.set(bounds);

    // create shader with offset
    matrix.setTranslate(r.fLeft, r.fTop);
    SkPaint paintWithShader = make_paint_with_image(paint, bitmap, SkSamplingOptions(), &matrix);
    SkDraw draw(*this);
    SkOverrideDeviceMatrixProvider matrixProvider(SkMatrix::I());
    draw.fMatrixProvider = &matrixProvider;
    // call ourself with a rect
    draw.drawRect(r, paintWithShader);
}

////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

void SkDraw::validate() const {
    SkASSERT(fMatrixProvider != nullptr);
    SkASSERT(fRC != nullptr);

    const SkIRect&  cr = fRC->getBounds();
    SkIRect         br;

    br.setWH(fDst.width(), fDst.height());
    SkASSERT(cr.isEmpty() || br.contains(cr));
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////

#include "include/core/SkPath.h"
#include "include/core/SkRegion.h"
#include "src/core/SkBlitter.h"
#include "src/core/SkDraw.h"

bool SkDraw::ComputeMaskBounds(const SkRect& devPathBounds, const SkIRect* clipBounds,
                               const SkMaskFilter* filter, const SkMatrix* filterMatrix,
                               SkIRect* bounds) {
    //  init our bounds from the path
    *bounds = devPathBounds.makeOutset(SK_ScalarHalf, SK_ScalarHalf).roundOut();

    SkIPoint margin = SkIPoint::Make(0, 0);
    if (filter) {
        SkASSERT(filterMatrix);

        SkMask srcM, dstM;

        srcM.fBounds = *bounds;
        srcM.fFormat = SkMask::kA8_Format;
        if (!as_MFB(filter)->filterMask(&dstM, srcM, *filterMatrix, &margin)) {
            return false;
        }
    }

    // (possibly) trim the bounds to reflect the clip
    // (plus whatever slop the filter needs)
    if (clipBounds) {
        // Ugh. Guard against gigantic margins from wacky filters. Without this
        // check we can request arbitrary amounts of slop beyond our visible
        // clip, and bring down the renderer (at least on finite RAM machines
        // like handsets, etc.). Need to balance this invented value between
        // quality of large filters like blurs, and the corresponding memory
        // requests.
        static const int MAX_MARGIN = 128;
        if (!bounds->intersect(clipBounds->makeOutset(std::min(margin.fX, MAX_MARGIN),
                                                      std::min(margin.fY, MAX_MARGIN)))) {
            return false;
        }
    }

    return true;
}

static void draw_into_mask(const SkMask& mask, const SkPath& devPath,
                           SkStrokeRec::InitStyle style) {
    SkDraw draw;
    if (!draw.fDst.reset(mask)) {
        return;
    }

    SkRasterClip    clip;
    SkMatrix        matrix;
    SkPaint         paint;

    clip.setRect(SkIRect::MakeWH(mask.fBounds.width(), mask.fBounds.height()));
    matrix.setTranslate(-SkIntToScalar(mask.fBounds.fLeft),
                        -SkIntToScalar(mask.fBounds.fTop));

    SkMatrixProvider matrixProvider(matrix);
    draw.fRC             = &clip;
    draw.fMatrixProvider = &matrixProvider;
    paint.setAntiAlias(true);
    switch (style) {
        case SkStrokeRec::kHairline_InitStyle:
            SkASSERT(!paint.getStrokeWidth());
            paint.setStyle(SkPaint::kStroke_Style);
            break;
        case SkStrokeRec::kFill_InitStyle:
            SkASSERT(paint.getStyle() == SkPaint::kFill_Style);
            break;

    }
    draw.drawPath(devPath, paint);
}

bool SkDraw::DrawToMask(const SkPath& devPath, const SkIRect* clipBounds,
                        const SkMaskFilter* filter, const SkMatrix* filterMatrix,
                        SkMask* mask, SkMask::CreateMode mode,
                        SkStrokeRec::InitStyle style) {
    if (devPath.isEmpty()) {
        return false;
    }

    if (SkMask::kJustRenderImage_CreateMode != mode) {
        if (!ComputeMaskBounds(devPath.getBounds(), clipBounds, filter,
                               filterMatrix, &mask->fBounds))
            return false;
    }

    if (SkMask::kComputeBoundsAndRenderImage_CreateMode == mode) {
        mask->fFormat = SkMask::kA8_Format;
        mask->fRowBytes = mask->fBounds.width();
        size_t size = mask->computeImageSize();
        if (0 == size) {
            // we're too big to allocate the mask, abort
            return false;
        }
        mask->fImage = SkMask::AllocImage(size, SkMask::kZeroInit_Alloc);
    }

    if (SkMask::kJustComputeBounds_CreateMode != mode) {
        draw_into_mask(*mask, devPath, style);
    }

    return true;
}
