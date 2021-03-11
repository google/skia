/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkRRect.h"
#include "include/core/SkStrokeRec.h"
#include "include/core/SkVertices.h"
#include "src/core/SkBlurMask.h"
#include "src/core/SkGpuBlurUtils.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkMathPriv.h"
#include "src/core/SkMatrixProvider.h"
#include "src/core/SkRRectPriv.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkStringUtils.h"
#include "src/core/SkWriteBuffer.h"

#if SK_SUPPORT_GPU
#include "include/gpu/GrRecordingContext.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/GrSurfaceDrawContext.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/effects/generated/GrCircleBlurFragmentProcessor.h"
#include "src/gpu/effects/generated/GrRRectBlurEffect.h"
#include "src/gpu/effects/generated/GrRectBlurEffect.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#endif

class SkBlurMaskFilterImpl : public SkMaskFilterBase {
public:
    SkBlurMaskFilterImpl(SkScalar sigma, SkBlurStyle, bool respectCTM);

    // overrides from SkMaskFilter
    SkMask::Format getFormat() const override;
    bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&,
                    SkIPoint* margin) const override;

#if SK_SUPPORT_GPU
    bool canFilterMaskGPU(const GrStyledShape& shape,
                          const SkIRect& devSpaceShapeBounds,
                          const SkIRect& clipBounds,
                          const SkMatrix& ctm,
                          SkIRect* maskRect) const override;
    bool directFilterMaskGPU(GrRecordingContext*,
                             GrSurfaceDrawContext* surfaceDrawContext,
                             GrPaint&&,
                             const GrClip*,
                             const SkMatrix& viewMatrix,
                             const GrStyledShape& shape) const override;
    GrSurfaceProxyView filterMaskGPU(GrRecordingContext*,
                                     GrSurfaceProxyView srcView,
                                     GrColorType srcColorType,
                                     SkAlphaType srcAlphaType,
                                     const SkMatrix& ctm,
                                     const SkIRect& maskRect) const override;
#endif

    void computeFastBounds(const SkRect&, SkRect*) const override;
    bool asABlur(BlurRec*) const override;


protected:
    FilterReturn filterRectsToNine(const SkRect[], int count, const SkMatrix&,
                                   const SkIRect& clipBounds,
                                   NinePatch*) const override;

    FilterReturn filterRRectToNine(const SkRRect&, const SkMatrix&,
                                   const SkIRect& clipBounds,
                                   NinePatch*) const override;

    bool filterRectMask(SkMask* dstM, const SkRect& r, const SkMatrix& matrix,
                        SkIPoint* margin, SkMask::CreateMode createMode) const;
    bool filterRRectMask(SkMask* dstM, const SkRRect& r, const SkMatrix& matrix,
                        SkIPoint* margin, SkMask::CreateMode createMode) const;

    bool ignoreXform() const { return !fRespectCTM; }

private:
    SK_FLATTENABLE_HOOKS(SkBlurMaskFilterImpl)
    // To avoid unseemly allocation requests (esp. for finite platforms like
    // handset) we limit the radius so something manageable. (as opposed to
    // a request like 10,000)
    static const SkScalar kMAX_BLUR_SIGMA;

    SkScalar    fSigma;
    SkBlurStyle fBlurStyle;
    bool        fRespectCTM;

    SkBlurMaskFilterImpl(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;

    SkScalar computeXformedSigma(const SkMatrix& ctm) const {
        SkScalar xformedSigma = this->ignoreXform() ? fSigma : ctm.mapRadius(fSigma);
        return std::min(xformedSigma, kMAX_BLUR_SIGMA);
    }

    friend class SkBlurMaskFilter;

    using INHERITED = SkMaskFilter;
    friend void sk_register_blur_maskfilter_createproc();
};

const SkScalar SkBlurMaskFilterImpl::kMAX_BLUR_SIGMA = SkIntToScalar(128);

///////////////////////////////////////////////////////////////////////////////

SkBlurMaskFilterImpl::SkBlurMaskFilterImpl(SkScalar sigma, SkBlurStyle style, bool respectCTM)
    : fSigma(sigma)
    , fBlurStyle(style)
    , fRespectCTM(respectCTM) {
    SkASSERT(fSigma > 0);
    SkASSERT((unsigned)style <= kLastEnum_SkBlurStyle);
}

SkMask::Format SkBlurMaskFilterImpl::getFormat() const {
    return SkMask::kA8_Format;
}

bool SkBlurMaskFilterImpl::asABlur(BlurRec* rec) const {
    if (this->ignoreXform()) {
        return false;
    }

    if (rec) {
        rec->fSigma = fSigma;
        rec->fStyle = fBlurStyle;
    }
    return true;
}

bool SkBlurMaskFilterImpl::filterMask(SkMask* dst, const SkMask& src,
                                      const SkMatrix& matrix,
                                      SkIPoint* margin) const {
    SkScalar sigma = this->computeXformedSigma(matrix);
    return SkBlurMask::BoxBlur(dst, src, sigma, fBlurStyle, margin);
}

bool SkBlurMaskFilterImpl::filterRectMask(SkMask* dst, const SkRect& r,
                                          const SkMatrix& matrix,
                                          SkIPoint* margin, SkMask::CreateMode createMode) const {
    SkScalar sigma = computeXformedSigma(matrix);

    return SkBlurMask::BlurRect(sigma, dst, r, fBlurStyle, margin, createMode);
}

bool SkBlurMaskFilterImpl::filterRRectMask(SkMask* dst, const SkRRect& r,
                                          const SkMatrix& matrix,
                                          SkIPoint* margin, SkMask::CreateMode createMode) const {
    SkScalar sigma = computeXformedSigma(matrix);

    return SkBlurMask::BlurRRect(sigma, dst, r, fBlurStyle, margin, createMode);
}

#include "include/core/SkCanvas.h"

static bool prepare_to_draw_into_mask(const SkRect& bounds, SkMask* mask) {
    SkASSERT(mask != nullptr);

    mask->fBounds = bounds.roundOut();
    mask->fRowBytes = SkAlign4(mask->fBounds.width());
    mask->fFormat = SkMask::kA8_Format;
    const size_t size = mask->computeImageSize();
    mask->fImage = SkMask::AllocImage(size, SkMask::kZeroInit_Alloc);
    if (nullptr == mask->fImage) {
        return false;
    }
    return true;
}

static bool draw_rrect_into_mask(const SkRRect rrect, SkMask* mask) {
    if (!prepare_to_draw_into_mask(rrect.rect(), mask)) {
        return false;
    }

    // FIXME: This code duplicates code in draw_rects_into_mask, below. Is there a
    // clean way to share more code?
    SkBitmap bitmap;
    bitmap.installMaskPixels(*mask);

    SkCanvas canvas(bitmap);
    canvas.translate(-SkIntToScalar(mask->fBounds.left()),
                     -SkIntToScalar(mask->fBounds.top()));

    SkPaint paint;
    paint.setAntiAlias(true);
    canvas.drawRRect(rrect, paint);
    return true;
}

static bool draw_rects_into_mask(const SkRect rects[], int count, SkMask* mask) {
    if (!prepare_to_draw_into_mask(rects[0], mask)) {
        return false;
    }

    SkBitmap bitmap;
    bitmap.installPixels(SkImageInfo::Make(mask->fBounds.width(),
                                           mask->fBounds.height(),
                                           kAlpha_8_SkColorType,
                                           kPremul_SkAlphaType),
                         mask->fImage, mask->fRowBytes);

    SkCanvas canvas(bitmap);
    canvas.translate(-SkIntToScalar(mask->fBounds.left()),
                     -SkIntToScalar(mask->fBounds.top()));

    SkPaint paint;
    paint.setAntiAlias(true);

    if (1 == count) {
        canvas.drawRect(rects[0], paint);
    } else {
        // todo: do I need a fast way to do this?
        SkPath path = SkPathBuilder().addRect(rects[0])
                                     .addRect(rects[1])
                                     .setFillType(SkPathFillType::kEvenOdd)
                                     .detach();
        canvas.drawPath(path, paint);
    }
    return true;
}

static bool rect_exceeds(const SkRect& r, SkScalar v) {
    return r.fLeft < -v || r.fTop < -v || r.fRight > v || r.fBottom > v ||
           r.width() > v || r.height() > v;
}

#include "src/core/SkMaskCache.h"

static SkCachedData* copy_mask_to_cacheddata(SkMask* mask) {
    const size_t size = mask->computeTotalImageSize();
    SkCachedData* data = SkResourceCache::NewCachedData(size);
    if (data) {
        memcpy(data->writable_data(), mask->fImage, size);
        SkMask::FreeImage(mask->fImage);
        mask->fImage = (uint8_t*)data->data();
    }
    return data;
}

static SkCachedData* find_cached_rrect(SkMask* mask, SkScalar sigma, SkBlurStyle style,
                                       const SkRRect& rrect) {
    return SkMaskCache::FindAndRef(sigma, style, rrect, mask);
}

static SkCachedData* add_cached_rrect(SkMask* mask, SkScalar sigma, SkBlurStyle style,
                                      const SkRRect& rrect) {
    SkCachedData* cache = copy_mask_to_cacheddata(mask);
    if (cache) {
        SkMaskCache::Add(sigma, style, rrect, *mask, cache);
    }
    return cache;
}

static SkCachedData* find_cached_rects(SkMask* mask, SkScalar sigma, SkBlurStyle style,
                                       const SkRect rects[], int count) {
    return SkMaskCache::FindAndRef(sigma, style, rects, count, mask);
}

static SkCachedData* add_cached_rects(SkMask* mask, SkScalar sigma, SkBlurStyle style,
                                      const SkRect rects[], int count) {
    SkCachedData* cache = copy_mask_to_cacheddata(mask);
    if (cache) {
        SkMaskCache::Add(sigma, style, rects, count, *mask, cache);
    }
    return cache;
}

static const bool c_analyticBlurRRect{true};

SkMaskFilterBase::FilterReturn
SkBlurMaskFilterImpl::filterRRectToNine(const SkRRect& rrect, const SkMatrix& matrix,
                                        const SkIRect& clipBounds,
                                        NinePatch* patch) const {
    SkASSERT(patch != nullptr);
    switch (rrect.getType()) {
        case SkRRect::kEmpty_Type:
            // Nothing to draw.
            return kFalse_FilterReturn;

        case SkRRect::kRect_Type:
            // We should have caught this earlier.
            SkASSERT(false);
            [[fallthrough]];
        case SkRRect::kOval_Type:
            // The nine patch special case does not handle ovals, and we
            // already have code for rectangles.
            return kUnimplemented_FilterReturn;

        // These three can take advantage of this fast path.
        case SkRRect::kSimple_Type:
        case SkRRect::kNinePatch_Type:
        case SkRRect::kComplex_Type:
            break;
    }

    // TODO: report correct metrics for innerstyle, where we do not grow the
    // total bounds, but we do need an inset the size of our blur-radius
    if (kInner_SkBlurStyle == fBlurStyle) {
        return kUnimplemented_FilterReturn;
    }

    // TODO: take clipBounds into account to limit our coordinates up front
    // for now, just skip too-large src rects (to take the old code path).
    if (rect_exceeds(rrect.rect(), SkIntToScalar(32767))) {
        return kUnimplemented_FilterReturn;
    }

    SkIPoint margin;
    SkMask  srcM, dstM;
    srcM.fBounds = rrect.rect().roundOut();
    srcM.fFormat = SkMask::kA8_Format;
    srcM.fRowBytes = 0;

    bool filterResult = false;
    if (c_analyticBlurRRect) {
        // special case for fast round rect blur
        // don't actually do the blur the first time, just compute the correct size
        filterResult = this->filterRRectMask(&dstM, rrect, matrix, &margin,
                                            SkMask::kJustComputeBounds_CreateMode);
    }

    if (!filterResult) {
        filterResult = this->filterMask(&dstM, srcM, matrix, &margin);
    }

    if (!filterResult) {
        return kFalse_FilterReturn;
    }

    // Now figure out the appropriate width and height of the smaller round rectangle
    // to stretch. It will take into account the larger radius per side as well as double
    // the margin, to account for inner and outer blur.
    const SkVector& UL = rrect.radii(SkRRect::kUpperLeft_Corner);
    const SkVector& UR = rrect.radii(SkRRect::kUpperRight_Corner);
    const SkVector& LR = rrect.radii(SkRRect::kLowerRight_Corner);
    const SkVector& LL = rrect.radii(SkRRect::kLowerLeft_Corner);

    const SkScalar leftUnstretched = std::max(UL.fX, LL.fX) + SkIntToScalar(2 * margin.fX);
    const SkScalar rightUnstretched = std::max(UR.fX, LR.fX) + SkIntToScalar(2 * margin.fX);

    // Extra space in the middle to ensure an unchanging piece for stretching. Use 3 to cover
    // any fractional space on either side plus 1 for the part to stretch.
    const SkScalar stretchSize = SkIntToScalar(3);

    const SkScalar totalSmallWidth = leftUnstretched + rightUnstretched + stretchSize;
    if (totalSmallWidth >= rrect.rect().width()) {
        // There is no valid piece to stretch.
        return kUnimplemented_FilterReturn;
    }

    const SkScalar topUnstretched = std::max(UL.fY, UR.fY) + SkIntToScalar(2 * margin.fY);
    const SkScalar bottomUnstretched = std::max(LL.fY, LR.fY) + SkIntToScalar(2 * margin.fY);

    const SkScalar totalSmallHeight = topUnstretched + bottomUnstretched + stretchSize;
    if (totalSmallHeight >= rrect.rect().height()) {
        // There is no valid piece to stretch.
        return kUnimplemented_FilterReturn;
    }

    SkRect smallR = SkRect::MakeWH(totalSmallWidth, totalSmallHeight);

    SkRRect smallRR;
    SkVector radii[4];
    radii[SkRRect::kUpperLeft_Corner] = UL;
    radii[SkRRect::kUpperRight_Corner] = UR;
    radii[SkRRect::kLowerRight_Corner] = LR;
    radii[SkRRect::kLowerLeft_Corner] = LL;
    smallRR.setRectRadii(smallR, radii);

    const SkScalar sigma = this->computeXformedSigma(matrix);
    SkCachedData* cache = find_cached_rrect(&patch->fMask, sigma, fBlurStyle, smallRR);
    if (!cache) {
        bool analyticBlurWorked = false;
        if (c_analyticBlurRRect) {
            analyticBlurWorked =
                this->filterRRectMask(&patch->fMask, smallRR, matrix, &margin,
                                      SkMask::kComputeBoundsAndRenderImage_CreateMode);
        }

        if (!analyticBlurWorked) {
            if (!draw_rrect_into_mask(smallRR, &srcM)) {
                return kFalse_FilterReturn;
            }

            SkAutoMaskFreeImage amf(srcM.fImage);

            if (!this->filterMask(&patch->fMask, srcM, matrix, &margin)) {
                return kFalse_FilterReturn;
            }
        }
        cache = add_cached_rrect(&patch->fMask, sigma, fBlurStyle, smallRR);
    }

    patch->fMask.fBounds.offsetTo(0, 0);
    patch->fOuterRect = dstM.fBounds;
    patch->fCenter.fX = SkScalarCeilToInt(leftUnstretched) + 1;
    patch->fCenter.fY = SkScalarCeilToInt(topUnstretched) + 1;
    SkASSERT(nullptr == patch->fCache);
    patch->fCache = cache;  // transfer ownership to patch
    return kTrue_FilterReturn;
}

// Use the faster analytic blur approach for ninepatch rects
static const bool c_analyticBlurNinepatch{true};

SkMaskFilterBase::FilterReturn
SkBlurMaskFilterImpl::filterRectsToNine(const SkRect rects[], int count,
                                        const SkMatrix& matrix,
                                        const SkIRect& clipBounds,
                                        NinePatch* patch) const {
    if (count < 1 || count > 2) {
        return kUnimplemented_FilterReturn;
    }

    // TODO: report correct metrics for innerstyle, where we do not grow the
    // total bounds, but we do need an inset the size of our blur-radius
    if (kInner_SkBlurStyle == fBlurStyle || kOuter_SkBlurStyle == fBlurStyle) {
        return kUnimplemented_FilterReturn;
    }

    // TODO: take clipBounds into account to limit our coordinates up front
    // for now, just skip too-large src rects (to take the old code path).
    if (rect_exceeds(rects[0], SkIntToScalar(32767))) {
        return kUnimplemented_FilterReturn;
    }

    SkIPoint margin;
    SkMask  srcM, dstM;
    srcM.fBounds = rects[0].roundOut();
    srcM.fFormat = SkMask::kA8_Format;
    srcM.fRowBytes = 0;

    bool filterResult = false;
    if (count == 1 && c_analyticBlurNinepatch) {
        // special case for fast rect blur
        // don't actually do the blur the first time, just compute the correct size
        filterResult = this->filterRectMask(&dstM, rects[0], matrix, &margin,
                                            SkMask::kJustComputeBounds_CreateMode);
    } else {
        filterResult = this->filterMask(&dstM, srcM, matrix, &margin);
    }

    if (!filterResult) {
        return kFalse_FilterReturn;
    }

    /*
     *  smallR is the smallest version of 'rect' that will still guarantee that
     *  we get the same blur results on all edges, plus 1 center row/col that is
     *  representative of the extendible/stretchable edges of the ninepatch.
     *  Since our actual edge may be fractional we inset 1 more to be sure we
     *  don't miss any interior blur.
     *  x is an added pixel of blur, and { and } are the (fractional) edge
     *  pixels from the original rect.
     *
     *   x x { x x .... x x } x x
     *
     *  Thus, in this case, we inset by a total of 5 (on each side) beginning
     *  with our outer-rect (dstM.fBounds)
     */
    SkRect smallR[2];
    SkIPoint center;

    // +2 is from +1 for each edge (to account for possible fractional edges
    int smallW = dstM.fBounds.width() - srcM.fBounds.width() + 2;
    int smallH = dstM.fBounds.height() - srcM.fBounds.height() + 2;
    SkIRect innerIR;

    if (1 == count) {
        innerIR = srcM.fBounds;
        center.set(smallW, smallH);
    } else {
        SkASSERT(2 == count);
        rects[1].roundIn(&innerIR);
        center.set(smallW + (innerIR.left() - srcM.fBounds.left()),
                   smallH + (innerIR.top() - srcM.fBounds.top()));
    }

    // +1 so we get a clean, stretchable, center row/col
    smallW += 1;
    smallH += 1;

    // we want the inset amounts to be integral, so we don't change any
    // fractional phase on the fRight or fBottom of our smallR.
    const SkScalar dx = SkIntToScalar(innerIR.width() - smallW);
    const SkScalar dy = SkIntToScalar(innerIR.height() - smallH);
    if (dx < 0 || dy < 0) {
        // we're too small, relative to our blur, to break into nine-patch,
        // so we ask to have our normal filterMask() be called.
        return kUnimplemented_FilterReturn;
    }

    smallR[0].setLTRB(rects[0].left(),       rects[0].top(),
                      rects[0].right() - dx, rects[0].bottom() - dy);
    if (smallR[0].width() < 2 || smallR[0].height() < 2) {
        return kUnimplemented_FilterReturn;
    }
    if (2 == count) {
        smallR[1].setLTRB(rects[1].left(), rects[1].top(),
                          rects[1].right() - dx, rects[1].bottom() - dy);
        SkASSERT(!smallR[1].isEmpty());
    }

    const SkScalar sigma = this->computeXformedSigma(matrix);
    SkCachedData* cache = find_cached_rects(&patch->fMask, sigma, fBlurStyle, smallR, count);
    if (!cache) {
        if (count > 1 || !c_analyticBlurNinepatch) {
            if (!draw_rects_into_mask(smallR, count, &srcM)) {
                return kFalse_FilterReturn;
            }

            SkAutoMaskFreeImage amf(srcM.fImage);

            if (!this->filterMask(&patch->fMask, srcM, matrix, &margin)) {
                return kFalse_FilterReturn;
            }
        } else {
            if (!this->filterRectMask(&patch->fMask, smallR[0], matrix, &margin,
                                      SkMask::kComputeBoundsAndRenderImage_CreateMode)) {
                return kFalse_FilterReturn;
            }
        }
        cache = add_cached_rects(&patch->fMask, sigma, fBlurStyle, smallR, count);
    }
    patch->fMask.fBounds.offsetTo(0, 0);
    patch->fOuterRect = dstM.fBounds;
    patch->fCenter = center;
    SkASSERT(nullptr == patch->fCache);
    patch->fCache = cache;  // transfer ownership to patch
    return kTrue_FilterReturn;
}

void SkBlurMaskFilterImpl::computeFastBounds(const SkRect& src,
                                             SkRect* dst) const {
    SkScalar pad = 3.0f * fSigma;

    dst->setLTRB(src.fLeft  - pad, src.fTop    - pad,
                 src.fRight + pad, src.fBottom + pad);
}

sk_sp<SkFlattenable> SkBlurMaskFilterImpl::CreateProc(SkReadBuffer& buffer) {
    const SkScalar sigma = buffer.readScalar();
    SkBlurStyle style = buffer.read32LE(kLastEnum_SkBlurStyle);

    uint32_t flags = buffer.read32LE(0x3);  // historically we only recorded 2 bits
    bool respectCTM = !(flags & 1); // historically we stored ignoreCTM in low bit

    return SkMaskFilter::MakeBlur((SkBlurStyle)style, sigma, respectCTM);
}

void SkBlurMaskFilterImpl::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalar(fSigma);
    buffer.writeUInt(fBlurStyle);
    buffer.writeUInt(!fRespectCTM); // historically we recorded ignoreCTM
}


#if SK_SUPPORT_GPU

bool SkBlurMaskFilterImpl::directFilterMaskGPU(GrRecordingContext* context,
                                               GrSurfaceDrawContext* surfaceDrawContext,
                                               GrPaint&& paint,
                                               const GrClip* clip,
                                               const SkMatrix& viewMatrix,
                                               const GrStyledShape& shape) const {
    SkASSERT(surfaceDrawContext);

    if (fBlurStyle != kNormal_SkBlurStyle) {
        return false;
    }

    // TODO: we could handle blurred stroked circles
    if (!shape.style().isSimpleFill()) {
        return false;
    }

    SkScalar xformedSigma = this->computeXformedSigma(viewMatrix);
    if (SkGpuBlurUtils::IsEffectivelyZeroSigma(xformedSigma)) {
        surfaceDrawContext->drawShape(clip, std::move(paint), GrAA::kYes, viewMatrix,
                                      GrStyledShape(shape));
        return true;
    }

    SkRRect srcRRect;
    bool inverted;
    if (!shape.asRRect(&srcRRect, nullptr, nullptr, &inverted) || inverted) {
        return false;
    }

    std::unique_ptr<GrFragmentProcessor> fp;

    SkRRect devRRect;
    bool devRRectIsValid = srcRRect.transform(viewMatrix, &devRRect);

    bool devRRectIsCircle = devRRectIsValid && SkRRectPriv::IsCircle(devRRect);

    bool canBeRect = srcRRect.isRect() && viewMatrix.preservesRightAngles();
    bool canBeCircle = (SkRRectPriv::IsCircle(srcRRect) && viewMatrix.isSimilarity()) ||
                       devRRectIsCircle;

    if (canBeRect || canBeCircle) {
        if (canBeRect) {
            fp = GrRectBlurEffect::Make(
                    /*inputFP=*/nullptr, context, *context->priv().caps()->shaderCaps(),
                    srcRRect.rect(), viewMatrix, xformedSigma);
        } else {
            SkRect devBounds;
            if (devRRectIsCircle) {
                devBounds = devRRect.getBounds();
            } else {
                SkPoint center = {srcRRect.getBounds().centerX(), srcRRect.getBounds().centerY()};
                viewMatrix.mapPoints(&center, 1);
                SkScalar radius = viewMatrix.mapVector(0, srcRRect.width()/2.f).length();
                devBounds = {center.x() - radius,
                             center.y() - radius,
                             center.x() + radius,
                             center.y() + radius};
            }
            fp = GrCircleBlurFragmentProcessor::Make(/*inputFP=*/nullptr, context, devBounds,
                                                     xformedSigma);
        }

        if (!fp) {
            return false;
        }
        paint.setCoverageFragmentProcessor(std::move(fp));

        SkRect srcProxyRect = srcRRect.rect();
        // Determine how much to outset the src rect to ensure we hit pixels within three sigma.
        SkScalar outsetX = 3.0f*xformedSigma;
        SkScalar outsetY = 3.0f*xformedSigma;
        if (viewMatrix.isScaleTranslate()) {
            outsetX /= SkScalarAbs(viewMatrix.getScaleX());
            outsetY /= SkScalarAbs(viewMatrix.getScaleY());
        } else {
            SkSize scale;
            if (!viewMatrix.decomposeScale(&scale, nullptr)) {
                return false;
            }
            outsetX /= scale.width();
            outsetY /= scale.height();
        }
        srcProxyRect.outset(outsetX, outsetY);

        surfaceDrawContext->drawRect(clip, std::move(paint), GrAA::kNo, viewMatrix, srcProxyRect);
        return true;
    }
    if (!viewMatrix.isScaleTranslate()) {
        return false;
    }
    if (!devRRectIsValid || !SkRRectPriv::AllCornersCircular(devRRect)) {
        return false;
    }

    fp = GrRRectBlurEffect::Make(/*inputFP=*/nullptr, context, fSigma, xformedSigma,
                                 srcRRect, devRRect);
    if (!fp) {
        return false;
    }

    if (!this->ignoreXform()) {
        SkRect srcProxyRect = srcRRect.rect();
        srcProxyRect.outset(3.0f*fSigma, 3.0f*fSigma);

        SkVertices::Builder builder(SkVertices::kTriangles_VertexMode, 4, 6, 0);
        srcProxyRect.toQuad(builder.positions());

        static const uint16_t fullIndices[6] = { 0, 1, 2, 0, 2, 3 };
        memcpy(builder.indices(), fullIndices, sizeof(fullIndices));
        sk_sp<SkVertices> vertices = builder.detach();

        paint.setCoverageFragmentProcessor(std::move(fp));
        SkSimpleMatrixProvider matrixProvider(viewMatrix);
        surfaceDrawContext->drawVertices(clip, std::move(paint), matrixProvider,
                                         std::move(vertices));
    } else {
        SkMatrix inverse;
        if (!viewMatrix.invert(&inverse)) {
            return false;
        }

        SkIRect proxyBounds;
        float extra=3.f*SkScalarCeilToScalar(xformedSigma-1/6.0f);
        devRRect.rect().makeOutset(extra, extra).roundOut(&proxyBounds);

        paint.setCoverageFragmentProcessor(std::move(fp));
        surfaceDrawContext->fillPixelsWithLocalMatrix(clip, std::move(paint), proxyBounds, inverse);
    }

    return true;
}

bool SkBlurMaskFilterImpl::canFilterMaskGPU(const GrStyledShape& shape,
                                            const SkIRect& devSpaceShapeBounds,
                                            const SkIRect& clipBounds,
                                            const SkMatrix& ctm,
                                            SkIRect* maskRect) const {
    SkScalar xformedSigma = this->computeXformedSigma(ctm);
    if (SkGpuBlurUtils::IsEffectivelyZeroSigma(xformedSigma)) {
        *maskRect = devSpaceShapeBounds;
        return maskRect->intersect(clipBounds);
    }

    if (maskRect) {
        float sigma3 = 3 * SkScalarToFloat(xformedSigma);

        // Outset srcRect and clipRect by 3 * sigma, to compute affected blur area.
        SkIRect clipRect = clipBounds.makeOutset(sigma3, sigma3);
        SkIRect srcRect = devSpaceShapeBounds.makeOutset(sigma3, sigma3);

        if (!srcRect.intersect(clipRect)) {
            srcRect.setEmpty();
        }
        *maskRect = srcRect;
    }

    // We prefer to blur paths with small blur radii on the CPU.
    static const SkScalar kMIN_GPU_BLUR_SIZE  = SkIntToScalar(64);
    static const SkScalar kMIN_GPU_BLUR_SIGMA = SkIntToScalar(32);

    if (devSpaceShapeBounds.width() <= kMIN_GPU_BLUR_SIZE &&
        devSpaceShapeBounds.height() <= kMIN_GPU_BLUR_SIZE &&
        xformedSigma <= kMIN_GPU_BLUR_SIGMA) {
        return false;
    }

    return true;
}

GrSurfaceProxyView SkBlurMaskFilterImpl::filterMaskGPU(GrRecordingContext* context,
                                                       GrSurfaceProxyView srcView,
                                                       GrColorType srcColorType,
                                                       SkAlphaType srcAlphaType,
                                                       const SkMatrix& ctm,
                                                       const SkIRect& maskRect) const {
    // 'maskRect' isn't snapped to the UL corner but the mask in 'src' is.
    const SkIRect clipRect = SkIRect::MakeWH(maskRect.width(), maskRect.height());

    SkScalar xformedSigma = this->computeXformedSigma(ctm);

    // If we're doing a normal blur, we can clobber the pathTexture in the
    // gaussianBlur.  Otherwise, we need to save it for later compositing.
    bool isNormalBlur = (kNormal_SkBlurStyle == fBlurStyle);
    auto srcBounds = SkIRect::MakeSize(srcView.proxy()->dimensions());
    auto surfaceDrawContext = SkGpuBlurUtils::GaussianBlur(context,
                                                            srcView,
                                                            srcColorType,
                                                            srcAlphaType,
                                                            nullptr,
                                                            clipRect,
                                                            srcBounds,
                                                            xformedSigma,
                                                            xformedSigma,
                                                            SkTileMode::kClamp);
    if (!surfaceDrawContext || !surfaceDrawContext->asTextureProxy()) {
        return {};
    }

    if (!isNormalBlur) {
        GrPaint paint;
        // Blend pathTexture over blurTexture.
        paint.setCoverageFragmentProcessor(GrTextureEffect::Make(std::move(srcView), srcAlphaType));
        if (kInner_SkBlurStyle == fBlurStyle) {
            // inner:  dst = dst * src
            paint.setCoverageSetOpXPFactory(SkRegion::kIntersect_Op);
        } else if (kSolid_SkBlurStyle == fBlurStyle) {
            // solid:  dst = src + dst - src * dst
            //             = src + (1 - src) * dst
            paint.setCoverageSetOpXPFactory(SkRegion::kUnion_Op);
        } else if (kOuter_SkBlurStyle == fBlurStyle) {
            // outer:  dst = dst * (1 - src)
            //             = 0 * src + (1 - src) * dst
            paint.setCoverageSetOpXPFactory(SkRegion::kDifference_Op);
        } else {
            paint.setCoverageSetOpXPFactory(SkRegion::kReplace_Op);
        }

        surfaceDrawContext->drawRect(nullptr, std::move(paint), GrAA::kNo, SkMatrix::I(),
                                     SkRect::Make(clipRect));
    }

    return surfaceDrawContext->readSurfaceView();
}

#endif // SK_SUPPORT_GPU

void sk_register_blur_maskfilter_createproc() { SK_REGISTER_FLATTENABLE(SkBlurMaskFilterImpl); }

sk_sp<SkMaskFilter> SkMaskFilter::MakeBlur(SkBlurStyle style, SkScalar sigma, bool respectCTM) {
    if (SkScalarIsFinite(sigma) && sigma > 0) {
        return sk_sp<SkMaskFilter>(new SkBlurMaskFilterImpl(sigma, style, respectCTM));
    }
    return nullptr;
}
