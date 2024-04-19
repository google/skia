/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkBlurMaskFilterImpl.h"

#include "include/core/SkBlendMode.h"
#include "include/core/SkBlurTypes.h"
#include "include/core/SkFlattenable.h"
#include "include/core/SkImageFilter.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathBuilder.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRRect.h"
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkScalar.h"
#include "include/effects/SkImageFilters.h"
#include "include/private/base/SkAlign.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkTemplates.h"
#include "src/base/SkTLazy.h"
#include "src/core/SkBlitter_A8.h"
#include "src/core/SkBlurMask.h"
#include "src/core/SkCachedData.h"
#include "src/core/SkDrawBase.h"
#include "src/core/SkMask.h"
#include "src/core/SkMaskCache.h"
#include "src/core/SkMaskFilterBase.h"
#include "src/core/SkRasterClip.h"
#include "src/core/SkReadBuffer.h"
#include "src/core/SkResourceCache.h"
#include "src/core/SkWriteBuffer.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <utility>

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

sk_sp<SkImageFilter> SkBlurMaskFilterImpl::asImageFilter(const SkMatrix& ctm) const {
    float sigma = fSigma;
    if (this->ignoreXform()) {
        // This is analogous to computeXformedSigma(), but it might be more correct to wrap the
        // blur image filter in a local matrix with ctm^-1, or to control the skif::Mapping when
        // the mask filter layer is restored. This is inaccurate when 'ctm' has skew or perspective
        const float ctmScaleFactor = fSigma / ctm.mapRadius(fSigma);
        sigma *= ctmScaleFactor;
    }

    // The null input image filter will be bound to the original coverage mask.
    sk_sp<SkImageFilter> filter = SkImageFilters::Blur(sigma, sigma, nullptr);
    // Combine the original coverage mask (src) and the blurred coverage mask (dst)
    switch(fBlurStyle) {
        case kInner_SkBlurStyle: //  dst = dst * src
                                 //      = 0 * src + src * dst
            return SkImageFilters::Blend(SkBlendMode::kDstIn, std::move(filter), nullptr);
        case kSolid_SkBlurStyle: //  dst = src + dst - src * dst
                                 //      = 1 * src + (1 - src) * dst
            return SkImageFilters::Blend(SkBlendMode::kSrcOver, std::move(filter), nullptr);
        case kOuter_SkBlurStyle: //  dst = dst * (1 - src)
                                 //      = 0 * src + (1 - src) * dst
            return SkImageFilters::Blend(SkBlendMode::kDstOut, std::move(filter), nullptr);
        case kNormal_SkBlurStyle:
            return filter;
    }
    SkUNREACHABLE;
}

SkScalar SkBlurMaskFilterImpl::computeXformedSigma(const SkMatrix& ctm) const {
    constexpr SkScalar kMaxBlurSigma = SkIntToScalar(128);
    SkScalar xformedSigma = this->ignoreXform() ? fSigma : ctm.mapRadius(fSigma);
    return std::min(xformedSigma, kMaxBlurSigma);
}

bool SkBlurMaskFilterImpl::filterMask(SkMaskBuilder* dst, const SkMask& src,
                                      const SkMatrix& matrix,
                                      SkIPoint* margin) const {
    SkScalar sigma = this->computeXformedSigma(matrix);
    return SkBlurMask::BoxBlur(dst, src, sigma, fBlurStyle, margin);
}

bool SkBlurMaskFilterImpl::filterRectMask(SkMaskBuilder* dst, const SkRect& r,
                                          const SkMatrix& matrix,
                                          SkIPoint* margin,
                                          SkMaskBuilder::CreateMode createMode) const {
    SkScalar sigma = computeXformedSigma(matrix);

    return SkBlurMask::BlurRect(sigma, dst, r, fBlurStyle, margin, createMode);
}

bool SkBlurMaskFilterImpl::filterRRectMask(SkMaskBuilder* dst, const SkRRect& r,
                                           const SkMatrix& matrix,
                                           SkIPoint* margin,
                                           SkMaskBuilder::CreateMode createMode) const {
    SkScalar sigma = computeXformedSigma(matrix);

    return SkBlurMask::BlurRRect(sigma, dst, r, fBlurStyle, margin, createMode);
}

static bool prepare_to_draw_into_mask(const SkRect& bounds, SkMaskBuilder* mask) {
    SkASSERT(mask != nullptr);

    mask->bounds() = bounds.roundOut();
    mask->rowBytes() = SkAlign4(mask->fBounds.width());
    mask->format() = SkMask::kA8_Format;
    const size_t size = mask->computeImageSize();
    mask->image() = SkMaskBuilder::AllocImage(size, SkMaskBuilder::kZeroInit_Alloc);
    if (nullptr == mask->fImage) {
        return false;
    }
    return true;
}

template <typename Proc> bool draw_into_mask(SkMaskBuilder* mask, const SkRect& bounds, Proc proc) {
    if (!prepare_to_draw_into_mask(bounds, mask)) {
        return false;
    }

    const int dx = mask->fBounds.fLeft;
    const int dy = mask->fBounds.fTop;
    SkRasterClip rclip(mask->fBounds);
    rclip.setRect(mask->fBounds.makeOffset(-dx, -dy));

    SkASSERT(mask->fFormat == SkMask::kA8_Format);
    auto info = SkImageInfo::MakeA8(mask->fBounds.width(), mask->fBounds.height());
    auto pm = SkPixmap(info, mask->fImage, mask->fRowBytes);

    SkMatrix ctm = SkMatrix::Translate(-SkIntToScalar(dx), -SkIntToScalar(dy));

    SkDrawBase draw;
    draw.fBlitterChooser = SkA8Blitter_Choose;
    draw.fCTM = &ctm;
    draw.fDst = pm;
    draw.fRC  = &rclip;

    SkPaint paint;
    paint.setAntiAlias(true);

    proc(draw, paint);
    return true;
}

static bool draw_rects_into_mask(const SkRect rects[], int count, SkMaskBuilder* mask) {
    return draw_into_mask(mask, rects[0], [&](SkDrawBase& draw, const SkPaint& paint) {
        if (1 == count) {
            draw.drawRect(rects[0], paint);
        } else {
            // todo: do I need a fast way to do this?
            SkPath path = SkPathBuilder().addRect(rects[0])
                                         .addRect(rects[1])
                                         .setFillType(SkPathFillType::kEvenOdd)
                                         .detach();
            draw.drawPath(path, paint);
        }
    });
}

static bool draw_rrect_into_mask(const SkRRect rrect, SkMaskBuilder* mask) {
    return draw_into_mask(mask, rrect.rect(), [&](SkDrawBase& draw, const SkPaint& paint) {
        draw.drawRRect(rrect, paint);
    });
}

static bool rect_exceeds(const SkRect& r, SkScalar v) {
    return r.fLeft < -v || r.fTop < -v || r.fRight > v || r.fBottom > v ||
           r.width() > v || r.height() > v;
}

static SkCachedData* copy_mask_to_cacheddata(SkMaskBuilder* mask) {
    const size_t size = mask->computeTotalImageSize();
    SkCachedData* data = SkResourceCache::NewCachedData(size);
    if (data) {
        memcpy(data->writable_data(), mask->fImage, size);
        SkMaskBuilder::FreeImage(mask->image());
        mask->image() = (uint8_t*)data->writable_data();
    }
    return data;
}

static SkCachedData* find_cached_rrect(SkTLazy<SkMask>* mask, SkScalar sigma, SkBlurStyle style,
                                       const SkRRect& rrect) {
    return SkMaskCache::FindAndRef(sigma, style, rrect, mask);
}

static SkCachedData* add_cached_rrect(SkMaskBuilder* mask, SkScalar sigma, SkBlurStyle style,
                                      const SkRRect& rrect) {
    SkCachedData* cache = copy_mask_to_cacheddata(mask);
    if (cache) {
        SkMaskCache::Add(sigma, style, rrect, *mask, cache);
    }
    return cache;
}

static SkCachedData* find_cached_rects(SkTLazy<SkMask>* mask, SkScalar sigma, SkBlurStyle style,
                                       const SkRect rects[], int count) {
    return SkMaskCache::FindAndRef(sigma, style, rects, count, mask);
}

static SkCachedData* add_cached_rects(SkMaskBuilder* mask, SkScalar sigma, SkBlurStyle style,
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
                                        SkTLazy<NinePatch>* patch) const {
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
    SkMaskBuilder srcM(nullptr, rrect.rect().roundOut(), 0, SkMask::kA8_Format), dstM;

    bool filterResult = false;
    if (c_analyticBlurRRect) {
        // special case for fast round rect blur
        // don't actually do the blur the first time, just compute the correct size
        filterResult = this->filterRRectMask(&dstM, rrect, matrix, &margin,
                                             SkMaskBuilder::kJustComputeBounds_CreateMode);
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
    SkTLazy<SkMask> cachedMask;
    SkCachedData* cache = find_cached_rrect(&cachedMask, sigma, fBlurStyle, smallRR);
    if (!cache) {
        SkMaskBuilder filterM;
        bool analyticBlurWorked = false;
        if (c_analyticBlurRRect) {
            analyticBlurWorked =
                this->filterRRectMask(&filterM, smallRR, matrix, &margin,
                                      SkMaskBuilder::kComputeBoundsAndRenderImage_CreateMode);
        }

        if (!analyticBlurWorked) {
            if (!draw_rrect_into_mask(smallRR, &srcM)) {
                return kFalse_FilterReturn;
            }
            SkAutoMaskFreeImage amf(srcM.image());

            if (!this->filterMask(&filterM, srcM, matrix, &margin)) {
                return kFalse_FilterReturn;
            }
        }
        cache = add_cached_rrect(&filterM, sigma, fBlurStyle, smallRR);
        cachedMask.init(filterM);
    }

    SkIRect bounds = cachedMask->fBounds;
    bounds.offsetTo(0, 0);
    patch->init(SkMask{cachedMask->fImage, bounds, cachedMask->fRowBytes, cachedMask->fFormat},
                dstM.fBounds,
                SkIPoint{SkScalarCeilToInt(leftUnstretched) + 1,
                         SkScalarCeilToInt(topUnstretched) + 1},
                cache); // transfer ownership to patch
    return kTrue_FilterReturn;
}

// Use the faster analytic blur approach for ninepatch rects
static const bool c_analyticBlurNinepatch{true};

SkMaskFilterBase::FilterReturn
SkBlurMaskFilterImpl::filterRectsToNine(const SkRect rects[], int count,
                                        const SkMatrix& matrix,
                                        const SkIRect& clipBounds,
                                        SkTLazy<NinePatch>* patch) const {
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
    SkMaskBuilder srcM(nullptr, rects[0].roundOut(), 0, SkMask::kA8_Format), dstM;

    bool filterResult = false;
    if (count == 1 && c_analyticBlurNinepatch) {
        // special case for fast rect blur
        // don't actually do the blur the first time, just compute the correct size
        filterResult = this->filterRectMask(&dstM, rects[0], matrix, &margin,
                                            SkMaskBuilder::kJustComputeBounds_CreateMode);
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
    SkTLazy<SkMask> cachedMask;
    SkCachedData* cache = find_cached_rects(&cachedMask, sigma, fBlurStyle, smallR, count);
    if (!cache) {
        SkMaskBuilder filterM;
        if (count > 1 || !c_analyticBlurNinepatch) {
            if (!draw_rects_into_mask(smallR, count, &srcM)) {
                return kFalse_FilterReturn;
            }

            SkAutoMaskFreeImage amf(srcM.image());

            if (!this->filterMask(&filterM, srcM, matrix, &margin)) {
                return kFalse_FilterReturn;
            }
        } else {
            if (!this->filterRectMask(&filterM, smallR[0], matrix, &margin,
                                      SkMaskBuilder::kComputeBoundsAndRenderImage_CreateMode)) {
                return kFalse_FilterReturn;
            }
        }
        cache = add_cached_rects(&filterM, sigma, fBlurStyle, smallR, count);
        cachedMask.init(filterM);
    }
    SkIRect bounds = cachedMask->fBounds;
    bounds.offsetTo(0, 0);
    patch->init(SkMask{cachedMask->fImage, bounds, cachedMask->fRowBytes, cachedMask->fFormat},
                dstM.fBounds, center, cache); // transfer ownership to patch
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

void sk_register_blur_maskfilter_createproc() { SK_REGISTER_FLATTENABLE(SkBlurMaskFilterImpl); }

sk_sp<SkMaskFilter> SkMaskFilter::MakeBlur(SkBlurStyle style, SkScalar sigma, bool respectCTM) {
    if (SkIsFinite(sigma) && sigma > 0) {
        return sk_sp<SkMaskFilter>(new SkBlurMaskFilterImpl(sigma, style, respectCTM));
    }
    return nullptr;
}
