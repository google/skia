/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkBlurMaskFilter.h"
#include "SkBlurMask.h"
#include "SkGpuBlurUtils.h"
#include "SkMaskFilterBase.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkMaskFilter.h"
#include "SkRRect.h"
#include "SkStringUtils.h"
#include "SkStrokeRec.h"
#include "SkVertices.h"

#if SK_SUPPORT_GPU
#include "GrCircleBlurFragmentProcessor.h"
#include "GrClip.h"
#include "GrContext.h"
#include "GrFragmentProcessor.h"
#include "GrRenderTargetContext.h"
#include "GrResourceProvider.h"
#include "GrShaderCaps.h"
#include "GrStyle.h"
#include "GrTextureProxy.h"
#include "effects/GrRectBlurEffect.h"
#include "effects/GrRRectBlurEffect.h"
#include "effects/GrSimpleTextureEffect.h"
#include "effects/GrTextureDomain.h"
#include "glsl/GrGLSLFragmentProcessor.h"
#include "glsl/GrGLSLFragmentShaderBuilder.h"
#include "glsl/GrGLSLProgramDataManager.h"
#include "glsl/GrGLSLUniformHandler.h"
#endif

SkScalar SkBlurMaskFilter::ConvertRadiusToSigma(SkScalar radius) {
    return SkBlurMask::ConvertRadiusToSigma(radius);
}

class SkBlurMaskFilterImpl : public SkMaskFilterBase {
public:
    SkBlurMaskFilterImpl(SkScalar sigma, SkBlurStyle, const SkRect& occluder, uint32_t flags);

    // overrides from SkMaskFilter
    SkMask::Format getFormat() const override;
    bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&,
                    SkIPoint* margin) const override;

#if SK_SUPPORT_GPU
    bool canFilterMaskGPU(const SkRRect& devRRect,
                          const SkIRect& clipBounds,
                          const SkMatrix& ctm,
                          SkRect* maskRect) const override;
    bool directFilterMaskGPU(GrContext*,
                             GrRenderTargetContext* renderTargetContext,
                             GrPaint&&,
                             const GrClip&,
                             const SkMatrix& viewMatrix,
                             const SkStrokeRec& strokeRec,
                             const SkPath& path) const override;
    bool directFilterRRectMaskGPU(GrContext*,
                                  GrRenderTargetContext* renderTargetContext,
                                  GrPaint&&,
                                  const GrClip&,
                                  const SkMatrix& viewMatrix,
                                  const SkStrokeRec& strokeRec,
                                  const SkRRect& rrect,
                                  const SkRRect& devRRect) const override;
    sk_sp<GrTextureProxy> filterMaskGPU(GrContext*,
                                        sk_sp<GrTextureProxy> srcProxy,
                                        const SkMatrix& ctm,
                                        const SkIRect& maskRect) const override;
#endif

    void computeFastBounds(const SkRect&, SkRect*) const override;
    bool asABlur(BlurRec*) const override;

    SK_TO_STRING_OVERRIDE()
    SK_DECLARE_PUBLIC_FLATTENABLE_DESERIALIZATION_PROCS(SkBlurMaskFilterImpl)

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

    bool ignoreXform() const {
        return SkToBool(fBlurFlags & SkBlurMaskFilter::kIgnoreTransform_BlurFlag);
    }

private:
    // To avoid unseemly allocation requests (esp. for finite platforms like
    // handset) we limit the radius so something manageable. (as opposed to
    // a request like 10,000)
    static const SkScalar kMAX_BLUR_SIGMA;

    SkScalar    fSigma;
    SkBlurStyle fBlurStyle;
    SkRect      fOccluder;
    uint32_t    fBlurFlags;

    SkBlurQuality getQuality() const {
        return (fBlurFlags & SkBlurMaskFilter::kHighQuality_BlurFlag) ?
                kHigh_SkBlurQuality : kLow_SkBlurQuality;
    }

    SkBlurMaskFilterImpl(SkReadBuffer&);
    void flatten(SkWriteBuffer&) const override;

    SkScalar computeXformedSigma(const SkMatrix& ctm) const {
        SkScalar xformedSigma = this->ignoreXform() ? fSigma : ctm.mapRadius(fSigma);
        return SkMinScalar(xformedSigma, kMAX_BLUR_SIGMA);
    }

    friend class SkBlurMaskFilter;

    typedef SkMaskFilter INHERITED;
};

const SkScalar SkBlurMaskFilterImpl::kMAX_BLUR_SIGMA = SkIntToScalar(128);

sk_sp<SkMaskFilter> SkBlurMaskFilter::Make(SkBlurStyle style, SkScalar sigma,
                                           const SkRect& occluder, uint32_t flags) {
    SkASSERT(!(flags & ~SkBlurMaskFilter::kAll_BlurFlag));
    SkASSERT(style <= kLastEnum_SkBlurStyle);

    if (!SkScalarIsFinite(sigma) || sigma <= 0) {
        return nullptr;
    }

    return sk_sp<SkMaskFilter>(new SkBlurMaskFilterImpl(sigma, style, occluder, flags));
}

// linearly interpolate between y1 & y3 to match x2's position between x1 & x3
static SkScalar interp(SkScalar x1, SkScalar x2, SkScalar x3, SkScalar y1, SkScalar y3) {
    SkASSERT(x1 <= x2 && x2 <= x3);
    SkASSERT(y1 <= y3);

    SkScalar t = (x2 - x1) / (x3 - x1);
    return y1 + t * (y3 - y1);
}

// Insert 'lower' and 'higher' into 'array1' and insert a new value at each matching insertion
// point in 'array2' that linearly interpolates between the existing values.
// Return a bit mask which contains a copy of 'inputMask' for all the cells between the two
// insertion points.
static uint32_t insert_into_arrays(SkScalar* array1, SkScalar* array2,
                                   SkScalar lower, SkScalar higher,
                                   int* num, uint32_t inputMask, int maskSize) {
    SkASSERT(lower < higher);
    SkASSERT(lower >= array1[0] && higher <= array1[*num-1]);

    int32_t skipMask = 0x0;
    int i;
    for (i = 0; i < *num; ++i) {
        if (lower >= array1[i] && lower < array1[i+1]) {
            if (!SkScalarNearlyEqual(lower, array1[i])) {
                memmove(&array1[i+2], &array1[i+1], (*num-i-1)*sizeof(SkScalar));
                array1[i+1] = lower;
                memmove(&array2[i+2], &array2[i+1], (*num-i-1)*sizeof(SkScalar));
                array2[i+1] = interp(array1[i], lower, array1[i+2], array2[i], array2[i+2]);
                i++;
                (*num)++;
            }
            break;
        }
    }
    for ( ; i < *num; ++i) {
        skipMask |= inputMask << (i*maskSize);
        if (higher > array1[i] && higher <= array1[i+1]) {
            if (!SkScalarNearlyEqual(higher, array1[i+1])) {
                memmove(&array1[i+2], &array1[i+1], (*num-i-1)*sizeof(SkScalar));
                array1[i+1] = higher;
                memmove(&array2[i+2], &array2[i+1], (*num-i-1)*sizeof(SkScalar));
                array2[i+1] = interp(array1[i], higher, array1[i+2], array2[i], array2[i+2]);
                (*num)++;
            }
            break;
        }
    }

    return skipMask;
}

bool SkBlurMaskFilter::ComputeBlurredRRectParams(const SkRRect& srcRRect, const SkRRect& devRRect,
                                                 const SkRect& occluder,
                                                 SkScalar sigma, SkScalar xformedSigma,
                                                 SkRRect* rrectToDraw,
                                                 SkISize* widthHeight,
                                                 SkScalar rectXs[kMaxDivisions],
                                                 SkScalar rectYs[kMaxDivisions],
                                                 SkScalar texXs[kMaxDivisions],
                                                 SkScalar texYs[kMaxDivisions],
                                                 int* numXs, int* numYs, uint32_t* skipMask) {
    unsigned int devBlurRadius = 3*SkScalarCeilToInt(xformedSigma-1/6.0f);
    SkScalar srcBlurRadius = 3.0f * sigma;

    const SkRect& devOrig = devRRect.getBounds();
    const SkVector& devRadiiUL = devRRect.radii(SkRRect::kUpperLeft_Corner);
    const SkVector& devRadiiUR = devRRect.radii(SkRRect::kUpperRight_Corner);
    const SkVector& devRadiiLR = devRRect.radii(SkRRect::kLowerRight_Corner);
    const SkVector& devRadiiLL = devRRect.radii(SkRRect::kLowerLeft_Corner);

    const int devLeft  = SkScalarCeilToInt(SkTMax<SkScalar>(devRadiiUL.fX, devRadiiLL.fX));
    const int devTop   = SkScalarCeilToInt(SkTMax<SkScalar>(devRadiiUL.fY, devRadiiUR.fY));
    const int devRight = SkScalarCeilToInt(SkTMax<SkScalar>(devRadiiUR.fX, devRadiiLR.fX));
    const int devBot   = SkScalarCeilToInt(SkTMax<SkScalar>(devRadiiLL.fY, devRadiiLR.fY));

    // This is a conservative check for nine-patchability
    if (devOrig.fLeft + devLeft + devBlurRadius >= devOrig.fRight  - devRight - devBlurRadius ||
        devOrig.fTop  + devTop  + devBlurRadius >= devOrig.fBottom - devBot   - devBlurRadius) {
        return false;
    }

    const SkVector& srcRadiiUL = srcRRect.radii(SkRRect::kUpperLeft_Corner);
    const SkVector& srcRadiiUR = srcRRect.radii(SkRRect::kUpperRight_Corner);
    const SkVector& srcRadiiLR = srcRRect.radii(SkRRect::kLowerRight_Corner);
    const SkVector& srcRadiiLL = srcRRect.radii(SkRRect::kLowerLeft_Corner);

    const SkScalar srcLeft  = SkTMax<SkScalar>(srcRadiiUL.fX, srcRadiiLL.fX);
    const SkScalar srcTop   = SkTMax<SkScalar>(srcRadiiUL.fY, srcRadiiUR.fY);
    const SkScalar srcRight = SkTMax<SkScalar>(srcRadiiUR.fX, srcRadiiLR.fX);
    const SkScalar srcBot   = SkTMax<SkScalar>(srcRadiiLL.fY, srcRadiiLR.fY);

    int newRRWidth = 2*devBlurRadius + devLeft + devRight + 1;
    int newRRHeight = 2*devBlurRadius + devTop + devBot + 1;
    widthHeight->fWidth = newRRWidth + 2 * devBlurRadius;
    widthHeight->fHeight = newRRHeight + 2 * devBlurRadius;

    const SkRect srcProxyRect = srcRRect.getBounds().makeOutset(srcBlurRadius, srcBlurRadius);

    rectXs[0] = srcProxyRect.fLeft;
    rectXs[1] = srcProxyRect.fLeft + 2*srcBlurRadius + srcLeft;
    rectXs[2] = srcProxyRect.fRight - 2*srcBlurRadius - srcRight;
    rectXs[3] = srcProxyRect.fRight;

    rectYs[0] = srcProxyRect.fTop;
    rectYs[1] = srcProxyRect.fTop + 2*srcBlurRadius + srcTop;
    rectYs[2] = srcProxyRect.fBottom - 2*srcBlurRadius - srcBot;
    rectYs[3] = srcProxyRect.fBottom;

    texXs[0] = 0.0f;
    texXs[1] = 2.0f*devBlurRadius + devLeft;
    texXs[2] = 2.0f*devBlurRadius + devLeft + 1;
    texXs[3] = SkIntToScalar(widthHeight->fWidth);

    texYs[0] = 0.0f;
    texYs[1] = 2.0f*devBlurRadius + devTop;
    texYs[2] = 2.0f*devBlurRadius + devTop + 1;
    texYs[3] = SkIntToScalar(widthHeight->fHeight);

    SkRect temp = occluder;

    *numXs = 4;
    *numYs = 4;
    *skipMask = 0;
    if (!temp.isEmpty() && (srcProxyRect.contains(temp) || temp.intersect(srcProxyRect))) {
        *skipMask = insert_into_arrays(rectXs, texXs, temp.fLeft, temp.fRight, numXs, 0x1, 1);
        *skipMask = insert_into_arrays(rectYs, texYs, temp.fTop, temp.fBottom,
                                       numYs, *skipMask, *numXs-1);
    }

    const SkRect newRect = SkRect::MakeXYWH(SkIntToScalar(devBlurRadius),
                                            SkIntToScalar(devBlurRadius),
                                            SkIntToScalar(newRRWidth),
                                            SkIntToScalar(newRRHeight));
    SkVector newRadii[4];
    newRadii[0] = { SkScalarCeilToScalar(devRadiiUL.fX), SkScalarCeilToScalar(devRadiiUL.fY) };
    newRadii[1] = { SkScalarCeilToScalar(devRadiiUR.fX), SkScalarCeilToScalar(devRadiiUR.fY) };
    newRadii[2] = { SkScalarCeilToScalar(devRadiiLR.fX), SkScalarCeilToScalar(devRadiiLR.fY) };
    newRadii[3] = { SkScalarCeilToScalar(devRadiiLL.fX), SkScalarCeilToScalar(devRadiiLL.fY) };

    rrectToDraw->setRectRadii(newRect, newRadii);
    return true;
}

///////////////////////////////////////////////////////////////////////////////

SkBlurMaskFilterImpl::SkBlurMaskFilterImpl(SkScalar sigma, SkBlurStyle style,
                                           const SkRect& occluder, uint32_t flags)
    : fSigma(sigma)
    , fBlurStyle(style)
    , fOccluder(occluder)
    , fBlurFlags(flags) {
    SkASSERT(fSigma > 0);
    SkASSERT((unsigned)style <= kLastEnum_SkBlurStyle);
    SkASSERT(flags <= SkBlurMaskFilter::kAll_BlurFlag);
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
        rec->fQuality = this->getQuality();
    }
    return true;
}

bool SkBlurMaskFilterImpl::filterMask(SkMask* dst, const SkMask& src,
                                      const SkMatrix& matrix,
                                      SkIPoint* margin) const {
    SkScalar sigma = this->computeXformedSigma(matrix);
    return SkBlurMask::BoxBlur(dst, src, sigma, fBlurStyle, this->getQuality(), margin);
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

#include "SkCanvas.h"

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
        SkPath path;
        path.addRect(rects[0]);
        path.addRect(rects[1]);
        path.setFillType(SkPath::kEvenOdd_FillType);
        canvas.drawPath(path, paint);
    }
    return true;
}

static bool rect_exceeds(const SkRect& r, SkScalar v) {
    return r.fLeft < -v || r.fTop < -v || r.fRight > v || r.fBottom > v ||
           r.width() > v || r.height() > v;
}

#include "SkMaskCache.h"

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
                                       SkBlurQuality quality, const SkRRect& rrect) {
    return SkMaskCache::FindAndRef(sigma, style, quality, rrect, mask);
}

static SkCachedData* add_cached_rrect(SkMask* mask, SkScalar sigma, SkBlurStyle style,
                                      SkBlurQuality quality, const SkRRect& rrect) {
    SkCachedData* cache = copy_mask_to_cacheddata(mask);
    if (cache) {
        SkMaskCache::Add(sigma, style, quality, rrect, *mask, cache);
    }
    return cache;
}

static SkCachedData* find_cached_rects(SkMask* mask, SkScalar sigma, SkBlurStyle style,
                                       SkBlurQuality quality, const SkRect rects[], int count) {
    return SkMaskCache::FindAndRef(sigma, style, quality, rects, count, mask);
}

static SkCachedData* add_cached_rects(SkMask* mask, SkScalar sigma, SkBlurStyle style,
                                      SkBlurQuality quality, const SkRect rects[], int count) {
    SkCachedData* cache = copy_mask_to_cacheddata(mask);
    if (cache) {
        SkMaskCache::Add(sigma, style, quality, rects, count, *mask, cache);
    }
    return cache;
}

#ifdef SK_IGNORE_FAST_RRECT_BLUR
  // Use the faster analytic blur approach for ninepatch round rects
  static const bool c_analyticBlurRRect{false};
#else
  static const bool c_analyticBlurRRect{true};
#endif

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
            // Fall through.
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

    const SkScalar leftUnstretched = SkTMax(UL.fX, LL.fX) + SkIntToScalar(2 * margin.fX);
    const SkScalar rightUnstretched = SkTMax(UR.fX, LR.fX) + SkIntToScalar(2 * margin.fX);

    // Extra space in the middle to ensure an unchanging piece for stretching. Use 3 to cover
    // any fractional space on either side plus 1 for the part to stretch.
    const SkScalar stretchSize = SkIntToScalar(3);

    const SkScalar totalSmallWidth = leftUnstretched + rightUnstretched + stretchSize;
    if (totalSmallWidth >= rrect.rect().width()) {
        // There is no valid piece to stretch.
        return kUnimplemented_FilterReturn;
    }

    const SkScalar topUnstretched = SkTMax(UL.fY, UR.fY) + SkIntToScalar(2 * margin.fY);
    const SkScalar bottomUnstretched = SkTMax(LL.fY, LR.fY) + SkIntToScalar(2 * margin.fY);

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
    SkCachedData* cache = find_cached_rrect(&patch->fMask, sigma, fBlurStyle,
                                            this->getQuality(), smallRR);
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
        cache = add_cached_rrect(&patch->fMask, sigma, fBlurStyle, this->getQuality(), smallRR);
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

    smallR[0].set(rects[0].left(), rects[0].top(), rects[0].right() - dx, rects[0].bottom() - dy);
    if (smallR[0].width() < 2 || smallR[0].height() < 2) {
        return kUnimplemented_FilterReturn;
    }
    if (2 == count) {
        smallR[1].set(rects[1].left(), rects[1].top(),
                      rects[1].right() - dx, rects[1].bottom() - dy);
        SkASSERT(!smallR[1].isEmpty());
    }

    const SkScalar sigma = this->computeXformedSigma(matrix);
    SkCachedData* cache = find_cached_rects(&patch->fMask, sigma, fBlurStyle,
                                            this->getQuality(), smallR, count);
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
        cache = add_cached_rects(&patch->fMask, sigma, fBlurStyle, this->getQuality(), smallR, count);
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

    dst->set(src.fLeft  - pad, src.fTop    - pad,
             src.fRight + pad, src.fBottom + pad);
}

sk_sp<SkFlattenable> SkBlurMaskFilterImpl::CreateProc(SkReadBuffer& buffer) {
    const SkScalar sigma = buffer.readScalar();
    SkBlurStyle style = buffer.read32LE(kLastEnum_SkBlurStyle);
    unsigned flags = buffer.read32LE(SkBlurMaskFilter::kAll_BlurFlag);

    SkRect occluder;
    buffer.readRect(&occluder);

    return SkBlurMaskFilter::Make((SkBlurStyle)style, sigma, occluder, flags);
}

void SkBlurMaskFilterImpl::flatten(SkWriteBuffer& buffer) const {
    buffer.writeScalar(fSigma);
    buffer.writeUInt(fBlurStyle);
    buffer.writeUInt(fBlurFlags);
    buffer.writeRect(fOccluder);
}


#if SK_SUPPORT_GPU

bool SkBlurMaskFilterImpl::directFilterMaskGPU(GrContext* context,
                                               GrRenderTargetContext* renderTargetContext,
                                               GrPaint&& paint,
                                               const GrClip& clip,
                                               const SkMatrix& viewMatrix,
                                               const SkStrokeRec& strokeRec,
                                               const SkPath& path) const {
    SkASSERT(renderTargetContext);

    if (fBlurStyle != kNormal_SkBlurStyle) {
        return false;
    }

    // TODO: we could handle blurred stroked circles
    if (!strokeRec.isFillStyle()) {
        return false;
    }

    SkScalar xformedSigma = this->computeXformedSigma(viewMatrix);

    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
    std::unique_ptr<GrFragmentProcessor> fp;

    SkRect rect;
    if (path.isRect(&rect)) {
        SkScalar pad = 3.0f * xformedSigma;
        rect.outset(pad, pad);

        fp = GrRectBlurEffect::Make(proxyProvider, rect, xformedSigma);
    } else if (path.isOval(&rect) && SkScalarNearlyEqual(rect.width(), rect.height())) {
        fp = GrCircleBlurFragmentProcessor::Make(proxyProvider, rect, xformedSigma);

        // expand the rect for the coverage geometry
        int pad = SkScalarCeilToInt(6*xformedSigma)/2;
        rect.outset(SkIntToScalar(pad), SkIntToScalar(pad));
    } else {
        return false;
    }

    if (!fp) {
        return false;
    }

    SkMatrix inverse;
    if (!viewMatrix.invert(&inverse)) {
        return false;
    }

    paint.addCoverageFragmentProcessor(std::move(fp));
    renderTargetContext->fillRectWithLocalMatrix(clip, std::move(paint), GrAA::kNo, SkMatrix::I(),
                                                 rect, inverse);
    return true;
}

bool SkBlurMaskFilterImpl::directFilterRRectMaskGPU(GrContext* context,
                                                    GrRenderTargetContext* renderTargetContext,
                                                    GrPaint&& paint,
                                                    const GrClip& clip,
                                                    const SkMatrix& viewMatrix,
                                                    const SkStrokeRec& strokeRec,
                                                    const SkRRect& srcRRect,
                                                    const SkRRect& devRRect) const {
    SkASSERT(renderTargetContext);

    if (fBlurStyle != kNormal_SkBlurStyle) {
        return false;
    }

    if (!strokeRec.isFillStyle()) {
        return false;
    }

    GrProxyProvider* proxyProvider = context->contextPriv().proxyProvider();
    SkScalar xformedSigma = this->computeXformedSigma(viewMatrix);

    if (devRRect.isRect() || devRRect.isCircle()) {
        std::unique_ptr<GrFragmentProcessor> fp;
        if (devRRect.isRect()) {
            SkScalar pad = 3.0f * xformedSigma;
            const SkRect dstCoverageRect = devRRect.rect().makeOutset(pad, pad);

            fp = GrRectBlurEffect::Make(proxyProvider, dstCoverageRect, xformedSigma);
        } else {
            fp = GrCircleBlurFragmentProcessor::Make(proxyProvider,
                                                     devRRect.rect(), xformedSigma);
        }

        if (!fp) {
            return false;
        }
        paint.addCoverageFragmentProcessor(std::move(fp));

        SkRect srcProxyRect = srcRRect.rect();
        SkScalar outsetX = 3.0f*fSigma;
        SkScalar outsetY = 3.0f*fSigma;
        if (this->ignoreXform()) {
            // When we're ignoring the CTM the padding added to the source rect also needs to ignore
            // the CTM. The matrix passed in here is guaranteed to be just scale and translate so we
            // can just grab the X and Y scales off the matrix and pre-undo the scale.
            outsetX /= viewMatrix.getScaleX();
            outsetY /= viewMatrix.getScaleY();
        }
        srcProxyRect.outset(outsetX, outsetY);

        renderTargetContext->drawRect(clip, std::move(paint), GrAA::kNo, viewMatrix, srcProxyRect);
        return true;
    }

    auto fp = GrRRectBlurEffect::Make(context, fSigma, xformedSigma, srcRRect, devRRect);
    if (!fp) {
        return false;
    }

    if (!this->ignoreXform()) {
        SkRect srcProxyRect = srcRRect.rect();
        srcProxyRect.outset(3.0f*fSigma, 3.0f*fSigma);

        sk_sp<SkVertices> vertices = nullptr;
        SkRect temp = fOccluder;

        if (!temp.isEmpty() && (srcProxyRect.contains(temp) || temp.intersect(srcProxyRect))) {
            SkVertices::Builder builder(SkVertices::kTriangles_VertexMode, 8, 24, 0);
            srcProxyRect.toQuad(builder.positions());
            temp.toQuad(builder.positions() + 4);

            static const uint16_t ringI[24] = { 0, 1, 5, 5, 4, 0,
                                                1, 2, 6, 6, 5, 1,
                                                2, 3, 7, 7, 6, 2,
                                                3, 0, 4, 4, 7, 3 };
            memcpy(builder.indices(), ringI, sizeof(ringI));
            vertices = builder.detach();
        } else {
            // full rect case
            SkVertices::Builder builder(SkVertices::kTriangles_VertexMode, 4, 6, 0);
            srcProxyRect.toQuad(builder.positions());

            static const uint16_t fullI[6] = { 0, 1, 2, 0, 2, 3 };
            memcpy(builder.indices(), fullI, sizeof(fullI));
            vertices = builder.detach();
        }

        paint.addCoverageFragmentProcessor(std::move(fp));
        renderTargetContext->drawVertices(clip, std::move(paint), viewMatrix, std::move(vertices));
    } else {
        SkMatrix inverse;
        if (!viewMatrix.invert(&inverse)) {
            return false;
        }

        float extra=3.f*SkScalarCeilToScalar(xformedSigma-1/6.0f);
        SkRect proxyRect = devRRect.rect();
        proxyRect.outset(extra, extra);

        paint.addCoverageFragmentProcessor(std::move(fp));
        renderTargetContext->fillRectWithLocalMatrix(clip, std::move(paint), GrAA::kNo,
                                                     SkMatrix::I(), proxyRect, inverse);
    }

    return true;
}

bool SkBlurMaskFilterImpl::canFilterMaskGPU(const SkRRect& devRRect,
                                            const SkIRect& clipBounds,
                                            const SkMatrix& ctm,
                                            SkRect* maskRect) const {
    SkScalar xformedSigma = this->computeXformedSigma(ctm);
    if (xformedSigma <= 0) {
        return false;
    }

    // We always do circles and simple circular rrects on the GPU
    if (!devRRect.isCircle() && !devRRect.isSimpleCircular()) {
        static const SkScalar kMIN_GPU_BLUR_SIZE  = SkIntToScalar(64);
        static const SkScalar kMIN_GPU_BLUR_SIGMA = SkIntToScalar(32);

        if (devRRect.width() <= kMIN_GPU_BLUR_SIZE &&
            devRRect.height() <= kMIN_GPU_BLUR_SIZE &&
            xformedSigma <= kMIN_GPU_BLUR_SIGMA) {
            // We prefer to blur small rects with small radii on the CPU.
            return false;
        }
    }

    if (nullptr == maskRect) {
        // don't need to compute maskRect
        return true;
    }

    float sigma3 = 3 * SkScalarToFloat(xformedSigma);

    SkRect clipRect = SkRect::Make(clipBounds);
    SkRect srcRect(devRRect.rect());

    // Outset srcRect and clipRect by 3 * sigma, to compute affected blur area.
    srcRect.outset(sigma3, sigma3);
    clipRect.outset(sigma3, sigma3);
    if (!srcRect.intersect(clipRect)) {
        srcRect.setEmpty();
    }
    *maskRect = srcRect;
    return true;
}

sk_sp<GrTextureProxy> SkBlurMaskFilterImpl::filterMaskGPU(GrContext* context,
                                                          sk_sp<GrTextureProxy> srcProxy,
                                                          const SkMatrix& ctm,
                                                          const SkIRect& maskRect) const {
    // 'maskRect' isn't snapped to the UL corner but the mask in 'src' is.
    const SkIRect clipRect = SkIRect::MakeWH(maskRect.width(), maskRect.height());

    SkScalar xformedSigma = this->computeXformedSigma(ctm);
    SkASSERT(xformedSigma > 0);

    // If we're doing a normal blur, we can clobber the pathTexture in the
    // gaussianBlur.  Otherwise, we need to save it for later compositing.
    bool isNormalBlur = (kNormal_SkBlurStyle == fBlurStyle);
    sk_sp<GrRenderTargetContext> renderTargetContext(
              SkGpuBlurUtils::GaussianBlur(context,
                                           srcProxy,
                                           nullptr,
                                           clipRect,
                                           SkIRect::EmptyIRect(),
                                           xformedSigma,
                                           xformedSigma,
                                           GrTextureDomain::kIgnore_Mode));
    if (!renderTargetContext) {
        return nullptr;
    }

    if (!isNormalBlur) {
        GrPaint paint;
        // Blend pathTexture over blurTexture.
        paint.addCoverageFragmentProcessor(GrSimpleTextureEffect::Make(std::move(srcProxy),
                                                                       SkMatrix::I()));
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

        renderTargetContext->drawRect(GrNoClip(), std::move(paint), GrAA::kNo, SkMatrix::I(),
                                      SkRect::Make(clipRect));
    }

    return renderTargetContext->asTextureProxyRef();
}

#endif // SK_SUPPORT_GPU


#ifndef SK_IGNORE_TO_STRING
void SkBlurMaskFilterImpl::toString(SkString* str) const {
    str->append("SkBlurMaskFilterImpl: (");

    str->append("sigma: ");
    str->appendScalar(fSigma);
    str->append(" ");

    static const char* gStyleName[kLastEnum_SkBlurStyle + 1] = {
        "normal", "solid", "outer", "inner"
    };

    str->appendf("style: %s ", gStyleName[fBlurStyle]);
    str->append("flags: (");
    if (fBlurFlags) {
        bool needSeparator = false;
        SkAddFlagToString(str, this->ignoreXform(), "IgnoreXform", &needSeparator);
        SkAddFlagToString(str,
                          SkToBool(fBlurFlags & SkBlurMaskFilter::kHighQuality_BlurFlag),
                          "HighQuality", &needSeparator);
    } else {
        str->append("None");
    }
    str->append("))");
}
#endif

SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_START(SkBlurMaskFilter)
    SK_DEFINE_FLATTENABLE_REGISTRAR_ENTRY(SkBlurMaskFilterImpl)
SK_DEFINE_FLATTENABLE_REGISTRAR_GROUP_END
