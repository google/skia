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
#include "src/core/SkRuntimeEffectPriv.h"
#include "src/gpu/GrFragmentProcessor.h"
#include "src/gpu/GrRecordingContextPriv.h"
#include "src/gpu/GrResourceProvider.h"
#include "src/gpu/GrShaderCaps.h"
#include "src/gpu/GrStyle.h"
#include "src/gpu/GrTextureProxy.h"
#include "src/gpu/GrThreadSafeCache.h"
#include "src/gpu/SkGr.h"
#include "src/gpu/effects/GrMatrixEffect.h"
#include "src/gpu/effects/GrSkSLFP.h"
#include "src/gpu/effects/GrTextureEffect.h"
#include "src/gpu/geometry/GrStyledShape.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramDataManager.h"
#include "src/gpu/glsl/GrGLSLUniformHandler.h"
#if SK_GPU_V1
#include "src/gpu/v1/SurfaceDrawContext_v1.h"
#endif // SK_GPU_V1
#endif // SK_SUPPORT_GPU

class SkBlurMaskFilterImpl : public SkMaskFilterBase {
public:
    SkBlurMaskFilterImpl(SkScalar sigma, SkBlurStyle, bool respectCTM);

    // overrides from SkMaskFilter
    SkMask::Format getFormat() const override;
    bool filterMask(SkMask* dst, const SkMask& src, const SkMatrix&,
                    SkIPoint* margin) const override;

#if SK_SUPPORT_GPU && SK_GPU_V1
    bool canFilterMaskGPU(const GrStyledShape& shape,
                          const SkIRect& devSpaceShapeBounds,
                          const SkIRect& clipBounds,
                          const SkMatrix& ctm,
                          SkIRect* maskRect) const override;
    bool directFilterMaskGPU(GrRecordingContext*,
                             skgpu::v1::SurfaceDrawContext*,
                             GrPaint&&,
                             const GrClip*,
                             const SkMatrix& viewMatrix,
                             const GrStyledShape&) const override;
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
    // TODO: if we're doing kInner blur, should we return a different outset?
    //       i.e. pad == 0 ?

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


#if SK_SUPPORT_GPU && SK_GPU_V1

///////////////////////////////////////////////////////////////////////////////
//  Circle Blur
///////////////////////////////////////////////////////////////////////////////

// Computes an unnormalized half kernel (right side). Returns the summation of all the half
// kernel values.
static float make_unnormalized_half_kernel(float* halfKernel, int halfKernelSize, float sigma) {
    const float invSigma = 1.f / sigma;
    const float b = -0.5f * invSigma * invSigma;
    float tot = 0.0f;
    // Compute half kernel values at half pixel steps out from the center.
    float t = 0.5f;
    for (int i = 0; i < halfKernelSize; ++i) {
        float value = expf(t * t * b);
        tot += value;
        halfKernel[i] = value;
        t += 1.f;
    }
    return tot;
}

// Create a Gaussian half-kernel (right side) and a summed area table given a sigma and number
// of discrete steps. The half kernel is normalized to sum to 0.5.
static void make_half_kernel_and_summed_table(float* halfKernel,
                                              float* summedHalfKernel,
                                              int halfKernelSize,
                                              float sigma) {
    // The half kernel should sum to 0.5 not 1.0.
    const float tot = 2.f * make_unnormalized_half_kernel(halfKernel, halfKernelSize, sigma);
    float sum = 0.f;
    for (int i = 0; i < halfKernelSize; ++i) {
        halfKernel[i] /= tot;
        sum += halfKernel[i];
        summedHalfKernel[i] = sum;
    }
}

// Applies the 1D half kernel vertically at points along the x axis to a circle centered at the
// origin with radius circleR.
void apply_kernel_in_y(float* results,
                       int numSteps,
                       float firstX,
                       float circleR,
                       int halfKernelSize,
                       const float* summedHalfKernelTable) {
    float x = firstX;
    for (int i = 0; i < numSteps; ++i, x += 1.f) {
        if (x < -circleR || x > circleR) {
            results[i] = 0;
            continue;
        }
        float y = sqrtf(circleR * circleR - x * x);
        // In the column at x we exit the circle at +y and -y
        // The summed table entry j is actually reflects an offset of j + 0.5.
        y -= 0.5f;
        int yInt = SkScalarFloorToInt(y);
        SkASSERT(yInt >= -1);
        if (y < 0) {
            results[i] = (y + 0.5f) * summedHalfKernelTable[0];
        } else if (yInt >= halfKernelSize - 1) {
            results[i] = 0.5f;
        } else {
            float yFrac = y - yInt;
            results[i] = (1.f - yFrac) * summedHalfKernelTable[yInt] +
                         yFrac * summedHalfKernelTable[yInt + 1];
        }
    }
}

// Apply a Gaussian at point (evalX, 0) to a circle centered at the origin with radius circleR.
// This relies on having a half kernel computed for the Gaussian and a table of applications of
// the half kernel in y to columns at (evalX - halfKernel, evalX - halfKernel + 1, ..., evalX +
// halfKernel) passed in as yKernelEvaluations.
static uint8_t eval_at(float evalX,
                       float circleR,
                       const float* halfKernel,
                       int halfKernelSize,
                       const float* yKernelEvaluations) {
    float acc = 0;

    float x = evalX - halfKernelSize;
    for (int i = 0; i < halfKernelSize; ++i, x += 1.f) {
        if (x < -circleR || x > circleR) {
            continue;
        }
        float verticalEval = yKernelEvaluations[i];
        acc += verticalEval * halfKernel[halfKernelSize - i - 1];
    }
    for (int i = 0; i < halfKernelSize; ++i, x += 1.f) {
        if (x < -circleR || x > circleR) {
            continue;
        }
        float verticalEval = yKernelEvaluations[i + halfKernelSize];
        acc += verticalEval * halfKernel[i];
    }
    // Since we applied a half kernel in y we multiply acc by 2 (the circle is symmetric about
    // the x axis).
    return SkUnitScalarClampToByte(2.f * acc);
}

// This function creates a profile of a blurred circle. It does this by computing a kernel for
// half the Gaussian and a matching summed area table. The summed area table is used to compute
// an array of vertical applications of the half kernel to the circle along the x axis. The
// table of y evaluations has 2 * k + n entries where k is the size of the half kernel and n is
// the size of the profile being computed. Then for each of the n profile entries we walk out k
// steps in each horizontal direction multiplying the corresponding y evaluation by the half
// kernel entry and sum these values to compute the profile entry.
static void create_circle_profile(uint8_t* weights,
                                  float sigma,
                                  float circleR,
                                  int profileTextureWidth) {
    const int numSteps = profileTextureWidth;

    // The full kernel is 6 sigmas wide.
    int halfKernelSize = SkScalarCeilToInt(6.0f * sigma);
    // round up to next multiple of 2 and then divide by 2
    halfKernelSize = ((halfKernelSize + 1) & ~1) >> 1;

    // Number of x steps at which to apply kernel in y to cover all the profile samples in x.
    int numYSteps = numSteps + 2 * halfKernelSize;

    SkAutoTArray<float> bulkAlloc(halfKernelSize + halfKernelSize + numYSteps);
    float* halfKernel = bulkAlloc.get();
    float* summedKernel = bulkAlloc.get() + halfKernelSize;
    float* yEvals = bulkAlloc.get() + 2 * halfKernelSize;
    make_half_kernel_and_summed_table(halfKernel, summedKernel, halfKernelSize, sigma);

    float firstX = -halfKernelSize + 0.5f;
    apply_kernel_in_y(yEvals, numYSteps, firstX, circleR, halfKernelSize, summedKernel);

    for (int i = 0; i < numSteps - 1; ++i) {
        float evalX = i + 0.5f;
        weights[i] = eval_at(evalX, circleR, halfKernel, halfKernelSize, yEvals + i);
    }
    // Ensure the tail of the Gaussian goes to zero.
    weights[numSteps - 1] = 0;
}

static void create_half_plane_profile(uint8_t* profile, int profileWidth) {
    SkASSERT(!(profileWidth & 0x1));
    // The full kernel is 6 sigmas wide.
    float sigma = profileWidth / 6.f;
    int halfKernelSize = profileWidth / 2;

    SkAutoTArray<float> halfKernel(halfKernelSize);

    // The half kernel should sum to 0.5.
    const float tot = 2.f * make_unnormalized_half_kernel(halfKernel.get(), halfKernelSize, sigma);
    float sum = 0.f;
    // Populate the profile from the right edge to the middle.
    for (int i = 0; i < halfKernelSize; ++i) {
        halfKernel[halfKernelSize - i - 1] /= tot;
        sum += halfKernel[halfKernelSize - i - 1];
        profile[profileWidth - i - 1] = SkUnitScalarClampToByte(sum);
    }
    // Populate the profile from the middle to the left edge (by flipping the half kernel and
    // continuing the summation).
    for (int i = 0; i < halfKernelSize; ++i) {
        sum += halfKernel[i];
        profile[halfKernelSize - i - 1] = SkUnitScalarClampToByte(sum);
    }
    // Ensure tail goes to 0.
    profile[profileWidth - 1] = 0;
}

static std::unique_ptr<GrFragmentProcessor> create_profile_effect(GrRecordingContext* rContext,
                                                                  const SkRect& circle,
                                                                  float sigma,
                                                                  float* solidRadius,
                                                                  float* textureRadius) {
    float circleR = circle.width() / 2.0f;
    if (!sk_float_isfinite(circleR) || circleR < SK_ScalarNearlyZero) {
        return nullptr;
    }

    auto threadSafeCache = rContext->priv().threadSafeCache();

    // Profile textures are cached by the ratio of sigma to circle radius and by the size of the
    // profile texture (binned by powers of 2).
    SkScalar sigmaToCircleRRatio = sigma / circleR;
    // When sigma is really small this becomes a equivalent to convolving a Gaussian with a
    // half-plane. Similarly, in the extreme high ratio cases circle becomes a point WRT to the
    // Guassian and the profile texture is a just a Gaussian evaluation. However, we haven't yet
    // implemented this latter optimization.
    sigmaToCircleRRatio = std::min(sigmaToCircleRRatio, 8.f);
    SkFixed sigmaToCircleRRatioFixed;
    static const SkScalar kHalfPlaneThreshold = 0.1f;
    bool useHalfPlaneApprox = false;
    if (sigmaToCircleRRatio <= kHalfPlaneThreshold) {
        useHalfPlaneApprox = true;
        sigmaToCircleRRatioFixed = 0;
        *solidRadius = circleR - 3 * sigma;
        *textureRadius = 6 * sigma;
    } else {
        // Convert to fixed point for the key.
        sigmaToCircleRRatioFixed = SkScalarToFixed(sigmaToCircleRRatio);
        // We shave off some bits to reduce the number of unique entries. We could probably
        // shave off more than we do.
        sigmaToCircleRRatioFixed &= ~0xff;
        sigmaToCircleRRatio = SkFixedToScalar(sigmaToCircleRRatioFixed);
        sigma = circleR * sigmaToCircleRRatio;
        *solidRadius = 0;
        *textureRadius = circleR + 3 * sigma;
    }

    static constexpr int kProfileTextureWidth = 512;
    // This would be kProfileTextureWidth/textureRadius if it weren't for the fact that we do
    // the calculation of the profile coord in a coord space that has already been scaled by
    // 1 / textureRadius. This is done to avoid overflow in length().
    SkMatrix texM = SkMatrix::Scale(kProfileTextureWidth, 1.f);

    static const skgpu::UniqueKey::Domain kDomain = skgpu::UniqueKey::GenerateDomain();
    skgpu::UniqueKey key;
    skgpu::UniqueKey::Builder builder(&key, kDomain, 1, "1-D Circular Blur");
    builder[0] = sigmaToCircleRRatioFixed;
    builder.finish();

    GrSurfaceProxyView profileView = threadSafeCache->find(key);
    if (profileView) {
        SkASSERT(profileView.asTextureProxy());
        SkASSERT(profileView.origin() == kTopLeft_GrSurfaceOrigin);
        return GrTextureEffect::Make(std::move(profileView), kPremul_SkAlphaType, texM);
    }

    SkBitmap bm;
    if (!bm.tryAllocPixels(SkImageInfo::MakeA8(kProfileTextureWidth, 1))) {
        return nullptr;
    }

    if (useHalfPlaneApprox) {
        create_half_plane_profile(bm.getAddr8(0, 0), kProfileTextureWidth);
    } else {
        // Rescale params to the size of the texture we're creating.
        SkScalar scale = kProfileTextureWidth / *textureRadius;
        create_circle_profile(
                bm.getAddr8(0, 0), sigma * scale, circleR * scale, kProfileTextureWidth);
    }
    bm.setImmutable();

    profileView = std::get<0>(GrMakeUncachedBitmapProxyView(rContext, bm));
    if (!profileView) {
        return nullptr;
    }

    profileView = threadSafeCache->add(key, profileView);
    return GrTextureEffect::Make(std::move(profileView), kPremul_SkAlphaType, texM);
}

static std::unique_ptr<GrFragmentProcessor> make_circle_blur(GrRecordingContext* context,
                                                             const SkRect& circle,
                                                             float sigma) {
    if (SkGpuBlurUtils::IsEffectivelyZeroSigma(sigma)) {
        return nullptr;
    }

    float solidRadius;
    float textureRadius;
    std::unique_ptr<GrFragmentProcessor> profile =
            create_profile_effect(context, circle, sigma, &solidRadius, &textureRadius);
    if (!profile) {
        return nullptr;
    }

    static auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, R"(
        uniform shader blurProfile;
        uniform half4 circleData;

        half4 main(float2 xy, half4 inColor) {
            // We just want to compute "(length(vec) - circleData.z + 0.5) * circleData.w" but need
            // to rearrange to avoid passing large values to length() that would overflow.
            half2 vec = half2((sk_FragCoord.xy - circleData.xy) * circleData.w);
            half dist = length(vec) + (0.5 - circleData.z) * circleData.w;
            return inColor * blurProfile.eval(half2(dist, 0.5)).a;
        }
    )");

    SkV4 circleData = {circle.centerX(), circle.centerY(), solidRadius, 1.f / textureRadius};
    return GrSkSLFP::Make(effect, "CircleBlur", /*inputFP=*/nullptr,
                          GrSkSLFP::OptFlags::kCompatibleWithCoverageAsAlpha,
                          "blurProfile", GrSkSLFP::IgnoreOptFlags(std::move(profile)),
                          "circleData", circleData);
}

///////////////////////////////////////////////////////////////////////////////
//  Rect Blur
///////////////////////////////////////////////////////////////////////////////

static std::unique_ptr<GrFragmentProcessor> make_rect_integral_fp(GrRecordingContext* rContext,
                                                                  float sixSigma) {
    SkASSERT(!SkGpuBlurUtils::IsEffectivelyZeroSigma(sixSigma / 6.f));
    auto threadSafeCache = rContext->priv().threadSafeCache();

    int width = SkGpuBlurUtils::CreateIntegralTable(sixSigma, nullptr);

    static const skgpu::UniqueKey::Domain kDomain = skgpu::UniqueKey::GenerateDomain();
    skgpu::UniqueKey key;
    skgpu::UniqueKey::Builder builder(&key, kDomain, 1, "Rect Blur Mask");
    builder[0] = width;
    builder.finish();

    SkMatrix m = SkMatrix::Scale(width / sixSigma, 1.f);

    GrSurfaceProxyView view = threadSafeCache->find(key);

    if (view) {
        SkASSERT(view.origin() == kTopLeft_GrSurfaceOrigin);
        return GrTextureEffect::Make(
                std::move(view), kPremul_SkAlphaType, m, GrSamplerState::Filter::kLinear);
    }

    SkBitmap bitmap;
    if (!SkGpuBlurUtils::CreateIntegralTable(sixSigma, &bitmap)) {
        return {};
    }

    view = std::get<0>(GrMakeUncachedBitmapProxyView(rContext, bitmap));
    if (!view) {
        return {};
    }

    view = threadSafeCache->add(key, view);

    SkASSERT(view.origin() == kTopLeft_GrSurfaceOrigin);
    return GrTextureEffect::Make(
            std::move(view), kPremul_SkAlphaType, m, GrSamplerState::Filter::kLinear);
}

static std::unique_ptr<GrFragmentProcessor> make_rect_blur(GrRecordingContext* context,
                                                           const GrShaderCaps& caps,
                                                           const SkRect& srcRect,
                                                           const SkMatrix& viewMatrix,
                                                           float transformedSigma) {
    SkASSERT(viewMatrix.preservesRightAngles());
    SkASSERT(srcRect.isSorted());

    if (SkGpuBlurUtils::IsEffectivelyZeroSigma(transformedSigma)) {
        // No need to blur the rect
        return nullptr;
    }

    SkMatrix invM;
    SkRect rect;
    if (viewMatrix.rectStaysRect()) {
        invM = SkMatrix::I();
        // We can do everything in device space when the src rect projects to a rect in device space
        SkAssertResult(viewMatrix.mapRect(&rect, srcRect));
    } else {
        // The view matrix may scale, perhaps anisotropically. But we want to apply our device space
        // "transformedSigma" to the delta of frag coord from the rect edges. Factor out the scaling
        // to define a space that is purely rotation/translation from device space (and scale from
        // src space) We'll meet in the middle: pre-scale the src rect to be in this space and then
        // apply the inverse of the rotation/translation portion to the frag coord.
        SkMatrix m;
        SkSize scale;
        if (!viewMatrix.decomposeScale(&scale, &m)) {
            return nullptr;
        }
        if (!m.invert(&invM)) {
            return nullptr;
        }
        rect = {srcRect.left() * scale.width(),
                srcRect.top() * scale.height(),
                srcRect.right() * scale.width(),
                srcRect.bottom() * scale.height()};
    }

    if (!caps.floatIs32Bits()) {
        // We promote the math that gets us into the Gaussian space to full float when the rect
        // coords are large. If we don't have full float then fail. We could probably clip the rect
        // to an outset device bounds instead.
        if (SkScalarAbs(rect.fLeft) > 16000.f || SkScalarAbs(rect.fTop) > 16000.f ||
            SkScalarAbs(rect.fRight) > 16000.f || SkScalarAbs(rect.fBottom) > 16000.f) {
            return nullptr;
        }
    }

    const float sixSigma = 6 * transformedSigma;
    std::unique_ptr<GrFragmentProcessor> integral = make_rect_integral_fp(context, sixSigma);
    if (!integral) {
        return nullptr;
    }

    // In the fast variant we think of the midpoint of the integral texture as aligning with the
    // closest rect edge both in x and y. To simplify texture coord calculation we inset the rect so
    // that the edge of the inset rect corresponds to t = 0 in the texture. It actually simplifies
    // things a bit in the !isFast case, too.
    float threeSigma = sixSigma / 2;
    SkRect insetRect = {rect.left() + threeSigma,
                        rect.top() + threeSigma,
                        rect.right() - threeSigma,
                        rect.bottom() - threeSigma};

    // In our fast variant we find the nearest horizontal and vertical edges and for each do a
    // lookup in the integral texture for each and multiply them. When the rect is less than 6 sigma
    // wide then things aren't so simple and we have to consider both the left and right edge of the
    // rectangle (and similar in y).
    bool isFast = insetRect.isSorted();

    static auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, R"(
        // Effect that is a LUT for integral of normal distribution. The value at x:[0,6*sigma] is
        // the integral from -inf to (3*sigma - x). I.e. x is mapped from [0, 6*sigma] to
        // [3*sigma to -3*sigma]. The flip saves a reversal in the shader.
        uniform shader integral;

        uniform float4 rect;
        uniform int isFast;  // specialized

        half4 main(float2 pos, half4 inColor) {
            half xCoverage, yCoverage;
            if (bool(isFast)) {
                // Get the smaller of the signed distance from the frag coord to the left and right
                // edges and similar for y.
                // The integral texture goes "backwards" (from 3*sigma to -3*sigma), So, the below
                // computations align the left edge of the integral texture with the inset rect's
                // edge extending outward 6 * sigma from the inset rect.
                half2 xy = max(half2(rect.LT - pos), half2(pos - rect.RB));
                xCoverage = integral.eval(half2(xy.x, 0.5)).a;
                yCoverage = integral.eval(half2(xy.y, 0.5)).a;
            } else {
                // We just consider just the x direction here. In practice we compute x and y
                // separately and multiply them together.
                // We define our coord system so that the point at which we're evaluating a kernel
                // defined by the normal distribution (K) at 0. In this coord system let L be left
                // edge and R be the right edge of the rectangle.
                // We can calculate C by integrating K with the half infinite ranges outside the
                // L to R range and subtracting from 1:
                //   C = 1 - <integral of K from from -inf to  L> - <integral of K from R to inf>
                // K is symmetric about x=0 so:
                //   C = 1 - <integral of K from from -inf to  L> - <integral of K from -inf to -R>

                // The integral texture goes "backwards" (from 3*sigma to -3*sigma) which is
                // factored in to the below calculations.
                // Also, our rect uniform was pre-inset by 3 sigma from the actual rect being
                // blurred, also factored in.
                half4 rect = half4(half2(rect.LT - pos), half2(pos - rect.RB));
                xCoverage = 1 - integral.eval(half2(rect.L, 0.5)).a
                              - integral.eval(half2(rect.R, 0.5)).a;
                yCoverage = 1 - integral.eval(half2(rect.T, 0.5)).a
                              - integral.eval(half2(rect.B, 0.5)).a;
            }
            return inColor * xCoverage * yCoverage;
        }
    )");

    std::unique_ptr<GrFragmentProcessor> fp =
            GrSkSLFP::Make(effect, "RectBlur", /*inputFP=*/nullptr,
                           GrSkSLFP::OptFlags::kCompatibleWithCoverageAsAlpha,
                           "integral", GrSkSLFP::IgnoreOptFlags(std::move(integral)),
                           "rect", insetRect,
                           "isFast", GrSkSLFP::Specialize<int>(isFast));
    if (!invM.isIdentity()) {
        fp = GrMatrixEffect::Make(invM, std::move(fp));
    }
    return GrFragmentProcessor::DeviceSpace(std::move(fp));
}

///////////////////////////////////////////////////////////////////////////////
//  RRect Blur
///////////////////////////////////////////////////////////////////////////////

static constexpr auto kBlurredRRectMaskOrigin = kTopLeft_GrSurfaceOrigin;

static void make_blurred_rrect_key(skgpu::UniqueKey* key,
                                   const SkRRect& rrectToDraw,
                                   float xformedSigma) {
    SkASSERT(!SkGpuBlurUtils::IsEffectivelyZeroSigma(xformedSigma));
    static const skgpu::UniqueKey::Domain kDomain = skgpu::UniqueKey::GenerateDomain();

    skgpu::UniqueKey::Builder builder(key, kDomain, 9, "RoundRect Blur Mask");
    builder[0] = SkScalarCeilToInt(xformedSigma - 1 / 6.0f);

    int index = 1;
    // TODO: this is overkill for _simple_ circular rrects
    for (auto c : {SkRRect::kUpperLeft_Corner,
                   SkRRect::kUpperRight_Corner,
                   SkRRect::kLowerRight_Corner,
                   SkRRect::kLowerLeft_Corner}) {
        SkASSERT(SkScalarIsInt(rrectToDraw.radii(c).fX) && SkScalarIsInt(rrectToDraw.radii(c).fY));
        builder[index++] = SkScalarCeilToInt(rrectToDraw.radii(c).fX);
        builder[index++] = SkScalarCeilToInt(rrectToDraw.radii(c).fY);
    }
    builder.finish();
}

static bool fillin_view_on_gpu(GrDirectContext* dContext,
                               const GrSurfaceProxyView& lazyView,
                               sk_sp<GrThreadSafeCache::Trampoline> trampoline,
                               const SkRRect& rrectToDraw,
                               const SkISize& dimensions,
                               float xformedSigma) {
#if SK_GPU_V1
    SkASSERT(!SkGpuBlurUtils::IsEffectivelyZeroSigma(xformedSigma));

    // We cache blur masks. Use default surface props here so we can use the same cached mask
    // regardless of the final dst surface.
    SkSurfaceProps defaultSurfaceProps;

    std::unique_ptr<skgpu::v1::SurfaceDrawContext> sdc =
            skgpu::v1::SurfaceDrawContext::MakeWithFallback(dContext,
                                                            GrColorType::kAlpha_8,
                                                            nullptr,
                                                            SkBackingFit::kExact,
                                                            dimensions,
                                                            defaultSurfaceProps,
                                                            1,
                                                            GrMipmapped::kNo,
                                                            GrProtected::kNo,
                                                            kBlurredRRectMaskOrigin);
    if (!sdc) {
        return false;
    }

    GrPaint paint;

    sdc->clear(SK_PMColor4fTRANSPARENT);
    sdc->drawRRect(nullptr,
                   std::move(paint),
                   GrAA::kYes,
                   SkMatrix::I(),
                   rrectToDraw,
                   GrStyle::SimpleFill());

    GrSurfaceProxyView srcView = sdc->readSurfaceView();
    SkASSERT(srcView.asTextureProxy());
    auto rtc2 = SkGpuBlurUtils::GaussianBlur(dContext,
                                             std::move(srcView),
                                             sdc->colorInfo().colorType(),
                                             sdc->colorInfo().alphaType(),
                                             nullptr,
                                             SkIRect::MakeSize(dimensions),
                                             SkIRect::MakeSize(dimensions),
                                             xformedSigma,
                                             xformedSigma,
                                             SkTileMode::kClamp,
                                             SkBackingFit::kExact);
    if (!rtc2 || !rtc2->readSurfaceView()) {
        return false;
    }

    auto view = rtc2->readSurfaceView();
    SkASSERT(view.swizzle() == lazyView.swizzle());
    SkASSERT(view.origin() == lazyView.origin());
    trampoline->fProxy = view.asTextureProxyRef();

    return true;
#else
    return false;
#endif
}

// Evaluate the vertical blur at the specified 'y' value given the location of the top of the
// rrect.
static uint8_t eval_V(float top, int y, const uint8_t* integral, int integralSize, float sixSigma) {
    if (top < 0) {
        return 0;  // an empty column
    }

    float fT = (top - y - 0.5f) * (integralSize / sixSigma);
    if (fT < 0) {
        return 255;
    } else if (fT >= integralSize - 1) {
        return 0;
    }

    int lower = (int)fT;
    float frac = fT - lower;

    SkASSERT(lower + 1 < integralSize);

    return integral[lower] * (1.0f - frac) + integral[lower + 1] * frac;
}

// Apply a gaussian 'kernel' horizontally at the specified 'x', 'y' location.
static uint8_t eval_H(int x,
                      int y,
                      const std::vector<float>& topVec,
                      const float* kernel,
                      int kernelSize,
                      const uint8_t* integral,
                      int integralSize,
                      float sixSigma) {
    SkASSERT(0 <= x && x < (int)topVec.size());
    SkASSERT(kernelSize % 2);

    float accum = 0.0f;

    int xSampleLoc = x - (kernelSize / 2);
    for (int i = 0; i < kernelSize; ++i, ++xSampleLoc) {
        if (xSampleLoc < 0 || xSampleLoc >= (int)topVec.size()) {
            continue;
        }

        accum += kernel[i] * eval_V(topVec[xSampleLoc], y, integral, integralSize, sixSigma);
    }

    return accum + 0.5f;
}

// Create a cpu-side blurred-rrect mask that is close to the version the gpu would've produced.
// The match needs to be close bc the cpu- and gpu-generated version must be interchangeable.
static GrSurfaceProxyView create_mask_on_cpu(GrRecordingContext* rContext,
                                             const SkRRect& rrectToDraw,
                                             const SkISize& dimensions,
                                             float xformedSigma) {
    SkASSERT(!SkGpuBlurUtils::IsEffectivelyZeroSigma(xformedSigma));
    int radius = SkGpuBlurUtils::SigmaRadius(xformedSigma);
    int kernelSize = 2 * radius + 1;

    SkASSERT(kernelSize % 2);
    SkASSERT(dimensions.width() % 2);
    SkASSERT(dimensions.height() % 2);

    SkVector radii = rrectToDraw.getSimpleRadii();
    SkASSERT(SkScalarNearlyEqual(radii.fX, radii.fY));

    const int halfWidthPlus1 = (dimensions.width() / 2) + 1;
    const int halfHeightPlus1 = (dimensions.height() / 2) + 1;

    std::unique_ptr<float[]> kernel(new float[kernelSize]);

    SkGpuBlurUtils::Compute1DGaussianKernel(kernel.get(), xformedSigma, radius);

    SkBitmap integral;
    if (!SkGpuBlurUtils::CreateIntegralTable(6 * xformedSigma, &integral)) {
        return {};
    }

    SkBitmap result;
    if (!result.tryAllocPixels(SkImageInfo::MakeA8(dimensions.width(), dimensions.height()))) {
        return {};
    }

    std::vector<float> topVec;
    topVec.reserve(dimensions.width());
    for (int x = 0; x < dimensions.width(); ++x) {
        if (x < rrectToDraw.rect().fLeft || x > rrectToDraw.rect().fRight) {
            topVec.push_back(-1);
        } else {
            if (x + 0.5f < rrectToDraw.rect().fLeft + radii.fX) {  // in the circular section
                float xDist = rrectToDraw.rect().fLeft + radii.fX - x - 0.5f;
                float h = sqrtf(radii.fX * radii.fX - xDist * xDist);
                SkASSERT(0 <= h && h < radii.fY);
                topVec.push_back(rrectToDraw.rect().fTop + radii.fX - h + 3 * xformedSigma);
            } else {
                topVec.push_back(rrectToDraw.rect().fTop + 3 * xformedSigma);
            }
        }
    }

    for (int y = 0; y < halfHeightPlus1; ++y) {
        uint8_t* scanline = result.getAddr8(0, y);

        for (int x = 0; x < halfWidthPlus1; ++x) {
            scanline[x] = eval_H(x,
                                 y,
                                 topVec,
                                 kernel.get(),
                                 kernelSize,
                                 integral.getAddr8(0, 0),
                                 integral.width(),
                                 6 * xformedSigma);
            scanline[dimensions.width() - x - 1] = scanline[x];
        }

        memcpy(result.getAddr8(0, dimensions.height() - y - 1), scanline, result.rowBytes());
    }

    result.setImmutable();

    auto view = std::get<0>(GrMakeUncachedBitmapProxyView(rContext, result));
    if (!view) {
        return {};
    }

    SkASSERT(view.origin() == kBlurredRRectMaskOrigin);
    return view;
}

static std::unique_ptr<GrFragmentProcessor> find_or_create_rrect_blur_mask_fp(
        GrRecordingContext* rContext,
        const SkRRect& rrectToDraw,
        const SkISize& dimensions,
        float xformedSigma) {
    SkASSERT(!SkGpuBlurUtils::IsEffectivelyZeroSigma(xformedSigma));
    skgpu::UniqueKey key;
    make_blurred_rrect_key(&key, rrectToDraw, xformedSigma);

    auto threadSafeCache = rContext->priv().threadSafeCache();

    // It seems like we could omit this matrix and modify the shader code to not normalize
    // the coords used to sample the texture effect. However, the "proxyDims" value in the
    // shader is not always the actual the proxy dimensions. This is because 'dimensions' here
    // was computed using integer corner radii as determined in
    // SkComputeBlurredRRectParams whereas the shader code uses the float radius to compute
    // 'proxyDims'. Why it draws correctly with these unequal values is a mystery for the ages.
    auto m = SkMatrix::Scale(dimensions.width(), dimensions.height());

    GrSurfaceProxyView view;

    if (GrDirectContext* dContext = rContext->asDirectContext()) {
        // The gpu thread gets priority over the recording threads. If the gpu thread is first,
        // it crams a lazy proxy into the cache and then fills it in later.
        auto [lazyView, trampoline] = GrThreadSafeCache::CreateLazyView(dContext,
                                                                        GrColorType::kAlpha_8,
                                                                        dimensions,
                                                                        kBlurredRRectMaskOrigin,
                                                                        SkBackingFit::kExact);
        if (!lazyView) {
            return nullptr;
        }

        view = threadSafeCache->findOrAdd(key, lazyView);
        if (view != lazyView) {
            SkASSERT(view.asTextureProxy());
            SkASSERT(view.origin() == kBlurredRRectMaskOrigin);
            return GrTextureEffect::Make(std::move(view), kPremul_SkAlphaType, m);
        }

        if (!fillin_view_on_gpu(dContext,
                                lazyView,
                                std::move(trampoline),
                                rrectToDraw,
                                dimensions,
                                xformedSigma)) {
            // In this case something has gone disastrously wrong so set up to drop the draw
            // that needed this resource and reduce future pollution of the cache.
            threadSafeCache->remove(key);
            return nullptr;
        }
    } else {
        view = threadSafeCache->find(key);
        if (view) {
            SkASSERT(view.asTextureProxy());
            SkASSERT(view.origin() == kBlurredRRectMaskOrigin);
            return GrTextureEffect::Make(std::move(view), kPremul_SkAlphaType, m);
        }

        view = create_mask_on_cpu(rContext, rrectToDraw, dimensions, xformedSigma);
        if (!view) {
            return nullptr;
        }

        view = threadSafeCache->add(key, view);
    }

    SkASSERT(view.asTextureProxy());
    SkASSERT(view.origin() == kBlurredRRectMaskOrigin);
    return GrTextureEffect::Make(std::move(view), kPremul_SkAlphaType, m);
}

static std::unique_ptr<GrFragmentProcessor> make_rrect_blur(GrRecordingContext* context,
                                                            float sigma,
                                                            float xformedSigma,
                                                            const SkRRect& srcRRect,
                                                            const SkRRect& devRRect) {
    // Should've been caught up-stream
#ifdef SK_DEBUG
    SkASSERTF(!SkRRectPriv::IsCircle(devRRect),
              "Unexpected circle. %d\n\t%s\n\t%s",
              SkRRectPriv::IsCircle(srcRRect),
              srcRRect.dumpToString(true).c_str(),
              devRRect.dumpToString(true).c_str());
    SkASSERTF(!devRRect.isRect(),
              "Unexpected rect. %d\n\t%s\n\t%s",
              srcRRect.isRect(),
              srcRRect.dumpToString(true).c_str(),
              devRRect.dumpToString(true).c_str());
#endif

    // TODO: loosen this up
    if (!SkRRectPriv::IsSimpleCircular(devRRect)) {
        return nullptr;
    }

    if (SkGpuBlurUtils::IsEffectivelyZeroSigma(xformedSigma)) {
        return nullptr;
    }

    // Make sure we can successfully ninepatch this rrect -- the blur sigma has to be sufficiently
    // small relative to both the size of the corner radius and the width (and height) of the rrect.
    SkRRect rrectToDraw;
    SkISize dimensions;
    SkScalar ignored[SkGpuBlurUtils::kBlurRRectMaxDivisions];

    bool ninePatchable = SkGpuBlurUtils::ComputeBlurredRRectParams(srcRRect,
                                                                   devRRect,
                                                                   sigma,
                                                                   xformedSigma,
                                                                   &rrectToDraw,
                                                                   &dimensions,
                                                                   ignored,
                                                                   ignored,
                                                                   ignored,
                                                                   ignored);
    if (!ninePatchable) {
        return nullptr;
    }

    std::unique_ptr<GrFragmentProcessor> maskFP =
            find_or_create_rrect_blur_mask_fp(context, rrectToDraw, dimensions, xformedSigma);
    if (!maskFP) {
        return nullptr;
    }

    static auto effect = SkMakeRuntimeEffect(SkRuntimeEffect::MakeForShader, R"(
        uniform shader ninePatchFP;

        uniform half cornerRadius;
        uniform float4 proxyRect;
        uniform half blurRadius;

        half4 main(float2 xy, half4 inColor) {
            // Warp the fragment position to the appropriate part of the 9-patch blur texture by
            // snipping out the middle section of the proxy rect.
            float2 translatedFragPosFloat = sk_FragCoord.xy - proxyRect.LT;
            float2 proxyCenter = (proxyRect.RB - proxyRect.LT) * 0.5;
            half edgeSize = 2.0 * blurRadius + cornerRadius + 0.5;

            // Position the fragment so that (0, 0) marks the center of the proxy rectangle.
            // Negative coordinates are on the left/top side and positive numbers are on the
            // right/bottom.
            translatedFragPosFloat -= proxyCenter;

            // Temporarily strip off the fragment's sign. x/y are now strictly increasing as we
            // move away from the center.
            half2 fragDirection = half2(sign(translatedFragPosFloat));
            translatedFragPosFloat = abs(translatedFragPosFloat);

            // Our goal is to snip out the "middle section" of the proxy rect (everything but the
            // edge). We've repositioned our fragment position so that (0, 0) is the centerpoint
            // and x/y are always positive, so we can subtract here and interpret negative results
            // as being within the middle section.
            half2 translatedFragPosHalf = half2(translatedFragPosFloat - (proxyCenter - edgeSize));

            // Remove the middle section by clamping to zero.
            translatedFragPosHalf = max(translatedFragPosHalf, 0);

            // Reapply the fragment's sign, so that negative coordinates once again mean left/top
            // side and positive means bottom/right side.
            translatedFragPosHalf *= fragDirection;

            // Offset the fragment so that (0, 0) marks the upper-left again, instead of the center
            // point.
            translatedFragPosHalf += half2(edgeSize);

            half2 proxyDims = half2(2.0 * edgeSize);
            half2 texCoord = translatedFragPosHalf / proxyDims;

            return inColor * ninePatchFP.eval(texCoord).a;
        }
    )");

    float cornerRadius = SkRRectPriv::GetSimpleRadii(devRRect).fX;
    float blurRadius = 3.f * SkScalarCeilToScalar(xformedSigma - 1 / 6.0f);
    SkRect proxyRect = devRRect.getBounds().makeOutset(blurRadius, blurRadius);

    return GrSkSLFP::Make(effect, "RRectBlur", /*inputFP=*/nullptr,
                          GrSkSLFP::OptFlags::kCompatibleWithCoverageAsAlpha,
                          "ninePatchFP", GrSkSLFP::IgnoreOptFlags(std::move(maskFP)),
                          "cornerRadius", cornerRadius,
                          "proxyRect", proxyRect,
                          "blurRadius", blurRadius);
}

///////////////////////////////////////////////////////////////////////////////

bool SkBlurMaskFilterImpl::directFilterMaskGPU(GrRecordingContext* context,
                                               skgpu::v1::SurfaceDrawContext* sdc,
                                               GrPaint&& paint,
                                               const GrClip* clip,
                                               const SkMatrix& viewMatrix,
                                               const GrStyledShape& shape) const {
    SkASSERT(sdc);

    if (fBlurStyle != kNormal_SkBlurStyle) {
        return false;
    }

    // TODO: we could handle blurred stroked circles
    if (!shape.style().isSimpleFill()) {
        return false;
    }

    SkScalar xformedSigma = this->computeXformedSigma(viewMatrix);
    if (SkGpuBlurUtils::IsEffectivelyZeroSigma(xformedSigma)) {
        sdc->drawShape(clip, std::move(paint), GrAA::kYes, viewMatrix, GrStyledShape(shape));
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
            fp = make_rect_blur(context, *context->priv().caps()->shaderCaps(),
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
            fp = make_circle_blur(context, devBounds, xformedSigma);
        }

        if (!fp) {
            return false;
        }

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

        paint.setCoverageFragmentProcessor(std::move(fp));
        sdc->drawRect(clip, std::move(paint), GrAA::kNo, viewMatrix, srcProxyRect);
        return true;
    }
    if (!viewMatrix.isScaleTranslate()) {
        return false;
    }
    if (!devRRectIsValid || !SkRRectPriv::AllCornersCircular(devRRect)) {
        return false;
    }

    fp = make_rrect_blur(context, fSigma, xformedSigma, srcRRect, devRRect);
    if (!fp) {
        return false;
    }

    if (!this->ignoreXform()) {
        SkRect srcProxyRect = srcRRect.rect();
        srcProxyRect.outset(3.0f*fSigma, 3.0f*fSigma);
        paint.setCoverageFragmentProcessor(std::move(fp));
        sdc->drawRect(clip, std::move(paint), GrAA::kNo, viewMatrix, srcProxyRect);
    } else {
        SkMatrix inverse;
        if (!viewMatrix.invert(&inverse)) {
            return false;
        }

        SkIRect proxyBounds;
        float extra=3.f*SkScalarCeilToScalar(xformedSigma-1/6.0f);
        devRRect.rect().makeOutset(extra, extra).roundOut(&proxyBounds);

        paint.setCoverageFragmentProcessor(std::move(fp));
        sdc->fillPixelsWithLocalMatrix(clip, std::move(paint), proxyBounds, inverse);
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

        surfaceDrawContext->fillPixelsWithLocalMatrix(nullptr, std::move(paint), clipRect,
                                                      SkMatrix::I());
    }

    return surfaceDrawContext->readSurfaceView();
}

#endif // SK_SUPPORT_GPU && SK_GPU_V1

void sk_register_blur_maskfilter_createproc() { SK_REGISTER_FLATTENABLE(SkBlurMaskFilterImpl); }

sk_sp<SkMaskFilter> SkMaskFilter::MakeBlur(SkBlurStyle style, SkScalar sigma, bool respectCTM) {
    if (SkScalarIsFinite(sigma) && sigma > 0) {
        return sk_sp<SkMaskFilter>(new SkBlurMaskFilterImpl(sigma, style, respectCTM));
    }
    return nullptr;
}
