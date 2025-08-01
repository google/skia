/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

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
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkStrokeRec.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkCPUTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkTLazy.h"
#include "src/base/SkZip.h"
#include "src/core/SkAutoBlitterChoose.h"
#include "src/core/SkBlendModePriv.h"
#include "src/core/SkBlitter_A8.h"
#include "src/core/SkDevice.h"
#include "src/core/SkDrawBase.h"
#include "src/core/SkDrawProcs.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkPathEffectBase.h"
#include "src/core/SkPathPriv.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkRectPriv.h"
#include "src/core/SkScan.h"

#include <algorithm>
#include <cstddef>
#include <optional>

class SkBitmap;
class SkBlitter;
class SkGlyph;
class SkMaskFilter;
class SkResourceCache;

using namespace skia_private;

///////////////////////////////////////////////////////////////////////////////

SkDrawBase::SkDrawBase() {}

bool SkDrawBase::computeConservativeLocalClipBounds(SkRect* localBounds) const {
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

void SkDrawBase::drawPaint(const SkPaint& paint) const {
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

static inline SkPoint compute_stroke_size(const SkPaint& paint, const SkMatrix& matrix) {
    SkASSERT(matrix.rectStaysRect());
    SkASSERT(SkPaint::kFill_Style != paint.getStyle());

    SkVector size = matrix.mapVector({paint.getStrokeWidth(), paint.getStrokeWidth()});
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

SkDrawBase::RectType SkDrawBase::ComputeRectType(const SkRect& rect,
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

static SkSpan<const SkPoint> rect_points(const SkRect& r) {
    return {reinterpret_cast<const SkPoint*>(&r), 2};
}

static SkSpan<SkPoint> rect_points(SkRect& r) {
    return {reinterpret_cast<SkPoint*>(&r), 2};
}

static void draw_rect_as_path(const SkDrawBase& orig,
                              const SkRect& prePaintRect,
                              const SkPaint& paint,
                              const SkMatrix& ctm) {
    SkDrawBase draw(orig);
    draw.fCTM = &ctm;
    draw.drawPath(SkPath::Rect(prePaintRect), paint, nullptr, true);
}

void SkDrawBase::drawRect(const SkRect& prePaintRect, const SkPaint& paint,
                      const SkMatrix* paintMatrix, const SkRect* postPaintRect) const {
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

    if (kPath_RectType == rtype) {
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
            // For kStroke_RectType, strokeSize is already computed.
            const SkPoint& ssize = (kStroke_RectType == rtype)
                ? strokeSize
                : compute_stroke_size(paint, *fCTM);
            bbox.outset(SkScalarHalf(ssize.x()), SkScalarHalf(ssize.y()));
        }
    }
    if (SkPathPriv::TooBigForMath(bbox)) {
        return;
    }

    if (!SkRectPriv::FitsInFixed(bbox) && rtype != kHair_RectType) {
        draw_rect_as_path(*this, prePaintRect, paint, *matrix);
        return;
    }

    SkIRect ir = bbox.roundOut();
    if (fRC->quickReject(ir)) {
        return;
    }

    SkAutoBlitterChoose blitterStorage(*this, matrix, paint);
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
    matrix.mapVectors(dst, src);
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

void SkDrawBase::drawOval(const SkRect& oval, const SkPaint& paint) const {
    SkDEBUGCODE(this->validate());

    if (fRC->isEmpty()) {
        return;
    }

    this->drawPath(SkPath::Oval(oval), paint, nullptr, true);
}

void SkDrawBase::drawRRect(const SkRRect& rrect, const SkPaint& paint) const {
    SkDEBUGCODE(this->validate());

    if (fRC->isEmpty()) {
        return;
    }

    {
        // TODO: Investigate optimizing these options. They are in the same
        // order as SkDrawBase::drawPath, which handles each case. It may be
        // that there is no way to optimize for these using the SkRRect path.
        SkScalar coverage;
        if (SkDrawTreatAsHairline(paint, *fCTM, &coverage)) {
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
    this->drawPath(SkPath::RRect(rrect), paint, nullptr, true);
}

bool SkDrawBase::drawRRectNinePatch(const SkRRect& rrect, const SkPaint& paint) const {
    SkASSERT(paint.getMaskFilter());

    if (auto rr = rrect.transform(*fCTM)) {
        SkAutoBlitterChoose blitter(*this, nullptr, paint);
        SkResourceCache* cache = nullptr;  // TODO(kjlubick) get this from fCtx
        const SkMaskFilterBase* maskFilter = as_MFB(paint.getMaskFilter());
        if (rrect.getType() == SkRRect::kRect_Type) {
            SkRect devRect = rr->rect();
            if (maskFilter->filterRects(SkSpan(&devRect, 1), *fCTM, *fRC, blitter.get(), cache)
                    == SkMaskFilterBase::FilterReturn::kTrue) {
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

void SkDrawBase::drawDevPath(const SkPath& devPath,
                             const SkPaint& paint,
                             SkDrawCoverage drawCoverage,
                             SkBlitter* customBlitter,
                             bool doFill) const {
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
        SkResourceCache* cache = nullptr;  // TODO(kjlubick) get this from fCtx
        if (as_MFB(paint.getMaskFilter())
                    ->filterPath(devPath, *fCTM, *fRC, blitter, style, cache)) {
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

    proc(devPath, *fRC, blitter);
}

void SkDrawBase::drawPath(const SkPath& origSrcPath,
                          const SkPaint& origPaint,
                          const SkMatrix* prePathMatrix,
                          bool pathIsMutable,
                          SkDrawCoverage drawCoverage,
                          SkBlitter* customBlitter) const {
    SkDEBUGCODE(this->validate();)

    // nothing to draw
    if (fRC->isEmpty()) {
        return;
    }

    SkPath*         pathPtr = const_cast<SkPath*>(&origSrcPath);
    bool            doFill = true;
    SkPath          tmpPathStorage;
    SkPath*         tmpPath = &tmpPathStorage;
    SkTCopyOnFirstWrite<SkMatrix> matrix(fCTM);
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
            matrix.writable()->preConcat(*prePathMatrix);
        }
    }

    SkTCopyOnFirstWrite<SkPaint> paint(origPaint);

    {
        SkScalar coverage;
        if (SkDrawTreatAsHairline(origPaint, *matrix, &coverage)) {
            const auto bm = origPaint.asBlendMode();
            if (SK_Scalar1 == coverage) {
                paint.writable()->setStrokeWidth(0);
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
        SkPathBuilder builder;
        doFill = skpathutils::FillPathWithPaint(*pathPtr, *paint, &builder, cullRectPtr, *fCTM);
        *tmpPath = builder.detach();
        pathPtr = tmpPath;
    }

    // avoid possibly allocating a new path in transform if we can
    SkPath* devPathPtr = pathIsMutable ? pathPtr : tmpPath;

    // transform the path into device space
    pathPtr->transform(*matrix, devPathPtr);

#if defined(SK_BUILD_FOR_FUZZER)
    if (devPathPtr->countPoints() > 1000) {
        return;
    }
#endif

    this->drawDevPath(*devPathPtr, *paint, drawCoverage, customBlitter, doFill);
}

void SkDrawBase::paintMasks(SkZip<const SkGlyph*, SkPoint>, const SkPaint&) const {
    SkASSERT(false);
}
void SkDrawBase::drawBitmap(const SkBitmap&, const SkMatrix&, const SkRect*,
                            const SkSamplingOptions&, const SkPaint&) const {
    SkASSERT(false);
}

////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef SK_DEBUG

void SkDrawBase::validate() const {
    SkASSERT(fCTM != nullptr);
    SkASSERT(fRC  != nullptr);

    const SkIRect&  cr = fRC->getBounds();
    SkIRect         br;

    br.setWH(fDst.width(), fDst.height());
    SkASSERT(cr.isEmpty() || br.contains(cr));
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////

static bool compute_mask_bounds(const SkRect& devPathBounds, const SkIRect& clipBounds,
                                const SkMaskFilter* filter, const SkMatrix* filterMatrix,
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

static void draw_into_mask(const SkMask& mask, const SkPath& devPath,
                           SkStrokeRec::InitStyle style) {
    SkDrawBase draw;
    draw.fBlitterChooser = SkA8Blitter_Choose;
    if (!draw.fDst.reset(mask)) {
        return;
    }

    SkRasterClip    clip;
    SkMatrix        matrix;
    SkPaint         paint;

    clip.setRect(SkIRect::MakeWH(mask.fBounds.width(), mask.fBounds.height()));
    matrix.setTranslate(-SkIntToScalar(mask.fBounds.fLeft),
                        -SkIntToScalar(mask.fBounds.fTop));

    draw.fRC  = &clip;
    draw.fCTM = &matrix;
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
    draw.drawPath(devPath, paint, nullptr, false);
}

bool SkDrawBase::DrawToMask(const SkPath& devPath, const SkIRect& clipBounds,
                        const SkMaskFilter* filter, const SkMatrix* filterMatrix,
                        SkMaskBuilder* dst, SkMaskBuilder::CreateMode mode,
                        SkStrokeRec::InitStyle style) {
    SkASSERT(filter);
    if (devPath.isEmpty()) {
        return false;
    }

    if (SkMaskBuilder::kJustRenderImage_CreateMode != mode) {
        // By using infinite bounds for inverse fills, compute_mask_bounds is able to clip it to
        // 'clipBounds' outset by whatever extra margin the mask filter requires.
        static const SkRect kInverseBounds = { SK_ScalarNegativeInfinity, SK_ScalarNegativeInfinity,
                                               SK_ScalarInfinity, SK_ScalarInfinity};
        SkRect pathBounds = devPath.isInverseFillType() ? kInverseBounds
                                                        : devPath.getBounds();
        if (!compute_mask_bounds(pathBounds, clipBounds, filter,
                               filterMatrix, &dst->bounds())) {
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
        draw_into_mask(*dst, devPath, style);
    }
    return true;
}

void SkDrawBase::drawDevicePoints(SkCanvas::PointMode mode, SkSpan<const SkPoint> points,
                                  const SkPaint& paint, SkDevice* device) const {
    // if we're in lines mode, force count to be even
    if (SkCanvas::kLines_PointMode == mode) {
        points = points.first(points.size() & ~1);   // force it to be even
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
                    SkPath     path = SkPath::Circle(0, 0, radius);
                    SkMatrix   preMatrix;

                    for (const auto& pt : points) {
                        preMatrix.setTranslate(pt.fX, pt.fY);
                        // pass true for the last point, since we can modify
                        // then path then
                        const bool isLast = &pt == &points.back();
                        this->drawPath(path, newPaint, &preMatrix, isLast);
                    }
                }
            } else {
                SkRect  r;

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

                if (as_PEB(paint.getPathEffect())->asPoints(&pointData, path, stroke, *fCTM,
                                                            &cullRect)) {
                    // 'asPoints' managed to find some fast path

                    SkPaint newP(paint);
                    newP.setPathEffect(nullptr);
                    newP.setStyle(SkPaint::kFill_Style);

                    if (!pointData.fFirst.isEmpty()) {
                        if (device) {
                            device->drawPath(pointData.fFirst, newP, true);
                        } else {
                            this->drawPath(pointData.fFirst, newP, nullptr, true);
                        }
                    }

                    if (!pointData.fLast.isEmpty()) {
                        if (device) {
                            device->drawPath(pointData.fLast, newP, true);
                        } else {
                            this->drawPath(pointData.fLast, newP, nullptr, true);
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
                                               pointData.points(),
                                               newP);
                        } else {
                            this->drawDevicePoints(SkCanvas::kPoints_PointMode,
                                                   pointData.points(),
                                                   newP,
                                                   device);
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
            [[fallthrough]]; // couldn't take fast path
        case SkCanvas::kPolygon_PointMode: {
            auto count = points.size() - 1;
            SkPaint p(paint);
            p.setStyle(SkPaint::kStroke_Style);
            size_t inc = (SkCanvas::kLines_PointMode == mode) ? 2 : 1;

            for (size_t i = 0; i < count; i += inc) {
                auto path = SkPath::Line(points[i], points[i+1]);
                if (device) {
                    device->drawPath(path, p, true);
                } else {
                    this->drawPath(path, p, nullptr, true);
                }
            }
            break;
        }
    }
}
