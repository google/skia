/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMatrixConvolutionImageFilter.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkReadBuffer.h"
#include "SkWriteBuffer.h"
#include "SkRect.h"
#include "SkUnPreMultiply.h"

#if SK_SUPPORT_GPU
#include "effects/GrMatrixConvolutionEffect.h"
#endif

static bool tile_mode_is_valid(SkMatrixConvolutionImageFilter::TileMode tileMode) {
    switch (tileMode) {
    case SkMatrixConvolutionImageFilter::kClamp_TileMode:
    case SkMatrixConvolutionImageFilter::kRepeat_TileMode:
    case SkMatrixConvolutionImageFilter::kClampToBlack_TileMode:
        return true;
    default:
        break;
    }
    return false;
}

SkMatrixConvolutionImageFilter::SkMatrixConvolutionImageFilter(
    const SkISize& kernelSize,
    const SkScalar* kernel,
    SkScalar gain,
    SkScalar bias,
    const SkIPoint& kernelOffset,
    TileMode tileMode,
    bool convolveAlpha,
    SkImageFilter* input,
    const CropRect* cropRect)
  : INHERITED(1, &input, cropRect),
    fKernelSize(kernelSize),
    fGain(gain),
    fBias(bias),
    fKernelOffset(kernelOffset),
    fTileMode(tileMode),
    fConvolveAlpha(convolveAlpha) {
    uint32_t size = fKernelSize.fWidth * fKernelSize.fHeight;
    fKernel = SkNEW_ARRAY(SkScalar, size);
    memcpy(fKernel, kernel, size * sizeof(SkScalar));
    SkASSERT(kernelSize.fWidth >= 1 && kernelSize.fHeight >= 1);
    SkASSERT(kernelOffset.fX >= 0 && kernelOffset.fX < kernelSize.fWidth);
    SkASSERT(kernelOffset.fY >= 0 && kernelOffset.fY < kernelSize.fHeight);
}

SkMatrixConvolutionImageFilter::SkMatrixConvolutionImageFilter(SkReadBuffer& buffer)
    : INHERITED(1, buffer) {
    // We need to be able to read at most SK_MaxS32 bytes, so divide that
    // by the size of a scalar to know how many scalars we can read.
    static const int32_t kMaxSize = SK_MaxS32 / sizeof(SkScalar);
    fKernelSize.fWidth = buffer.readInt();
    fKernelSize.fHeight = buffer.readInt();
    if ((fKernelSize.fWidth >= 1) && (fKernelSize.fHeight >= 1) &&
        // Make sure size won't be larger than a signed int,
        // which would still be extremely large for a kernel,
        // but we don't impose a hard limit for kernel size
        (kMaxSize / fKernelSize.fWidth >= fKernelSize.fHeight)) {
        size_t size = fKernelSize.fWidth * fKernelSize.fHeight;
        fKernel = SkNEW_ARRAY(SkScalar, size);
        SkDEBUGCODE(bool success =) buffer.readScalarArray(fKernel, size);
        SkASSERT(success);
    } else {
        fKernel = 0;
    }
    fGain = buffer.readScalar();
    fBias = buffer.readScalar();
    fKernelOffset.fX = buffer.readInt();
    fKernelOffset.fY = buffer.readInt();
    fTileMode = (TileMode) buffer.readInt();
    fConvolveAlpha = buffer.readBool();
    buffer.validate((fKernel != 0) &&
                    SkScalarIsFinite(fGain) &&
                    SkScalarIsFinite(fBias) &&
                    tile_mode_is_valid(fTileMode) &&
                    (fKernelOffset.fX >= 0) && (fKernelOffset.fX < fKernelSize.fWidth) &&
                    (fKernelOffset.fY >= 0) && (fKernelOffset.fY < fKernelSize.fHeight));
}

void SkMatrixConvolutionImageFilter::flatten(SkWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeInt(fKernelSize.fWidth);
    buffer.writeInt(fKernelSize.fHeight);
    buffer.writeScalarArray(fKernel, fKernelSize.fWidth * fKernelSize.fHeight);
    buffer.writeScalar(fGain);
    buffer.writeScalar(fBias);
    buffer.writeInt(fKernelOffset.fX);
    buffer.writeInt(fKernelOffset.fY);
    buffer.writeInt((int) fTileMode);
    buffer.writeBool(fConvolveAlpha);
}

SkMatrixConvolutionImageFilter::~SkMatrixConvolutionImageFilter() {
    delete[] fKernel;
}

class UncheckedPixelFetcher {
public:
    static inline SkPMColor fetch(const SkBitmap& src, int x, int y, const SkIRect& bounds) {
        return *src.getAddr32(x, y);
    }
};

class ClampPixelFetcher {
public:
    static inline SkPMColor fetch(const SkBitmap& src, int x, int y, const SkIRect& bounds) {
        x = SkPin32(x, bounds.fLeft, bounds.fRight - 1);
        y = SkPin32(y, bounds.fTop, bounds.fBottom - 1);
        return *src.getAddr32(x, y);
    }
};

class RepeatPixelFetcher {
public:
    static inline SkPMColor fetch(const SkBitmap& src, int x, int y, const SkIRect& bounds) {
        x = (x - bounds.left()) % bounds.width() + bounds.left();
        y = (y - bounds.top()) % bounds.height() + bounds.top();
        if (x < bounds.left()) {
            x += bounds.width();
        }
        if (y < bounds.top()) {
            y += bounds.height();
        }
        return *src.getAddr32(x, y);
    }
};

class ClampToBlackPixelFetcher {
public:
    static inline SkPMColor fetch(const SkBitmap& src, int x, int y, const SkIRect& bounds) {
        if (x < bounds.fLeft || x >= bounds.fRight || y < bounds.fTop || y >= bounds.fBottom) {
            return 0;
        } else {
            return *src.getAddr32(x, y);
        }
    }
};

template<class PixelFetcher, bool convolveAlpha>
void SkMatrixConvolutionImageFilter::filterPixels(const SkBitmap& src,
                                                  SkBitmap* result,
                                                  const SkIRect& r,
                                                  const SkIRect& bounds) const {
    SkIRect rect(r);
    if (!rect.intersect(bounds)) {
        return;
    }
    for (int y = rect.fTop; y < rect.fBottom; ++y) {
        SkPMColor* dptr = result->getAddr32(rect.fLeft - bounds.fLeft, y - bounds.fTop);
        for (int x = rect.fLeft; x < rect.fRight; ++x) {
            SkScalar sumA = 0, sumR = 0, sumG = 0, sumB = 0;
            for (int cy = 0; cy < fKernelSize.fHeight; cy++) {
                for (int cx = 0; cx < fKernelSize.fWidth; cx++) {
                    SkPMColor s = PixelFetcher::fetch(src,
                                                      x + cx - fKernelOffset.fX,
                                                      y + cy - fKernelOffset.fY,
                                                      bounds);
                    SkScalar k = fKernel[cy * fKernelSize.fWidth + cx];
                    if (convolveAlpha) {
                        sumA += SkScalarMul(SkIntToScalar(SkGetPackedA32(s)), k);
                    }
                    sumR += SkScalarMul(SkIntToScalar(SkGetPackedR32(s)), k);
                    sumG += SkScalarMul(SkIntToScalar(SkGetPackedG32(s)), k);
                    sumB += SkScalarMul(SkIntToScalar(SkGetPackedB32(s)), k);
                }
            }
            int a = convolveAlpha
                  ? SkClampMax(SkScalarFloorToInt(SkScalarMul(sumA, fGain) + fBias), 255)
                  : 255;
            int r = SkClampMax(SkScalarFloorToInt(SkScalarMul(sumR, fGain) + fBias), a);
            int g = SkClampMax(SkScalarFloorToInt(SkScalarMul(sumG, fGain) + fBias), a);
            int b = SkClampMax(SkScalarFloorToInt(SkScalarMul(sumB, fGain) + fBias), a);
            if (!convolveAlpha) {
                a = SkGetPackedA32(PixelFetcher::fetch(src, x, y, bounds));
                *dptr++ = SkPreMultiplyARGB(a, r, g, b);
            } else {
                *dptr++ = SkPackARGB32(a, r, g, b);
            }
        }
    }
}

template<class PixelFetcher>
void SkMatrixConvolutionImageFilter::filterPixels(const SkBitmap& src,
                                                  SkBitmap* result,
                                                  const SkIRect& rect,
                                                  const SkIRect& bounds) const {
    if (fConvolveAlpha) {
        filterPixels<PixelFetcher, true>(src, result, rect, bounds);
    } else {
        filterPixels<PixelFetcher, false>(src, result, rect, bounds);
    }
}

void SkMatrixConvolutionImageFilter::filterInteriorPixels(const SkBitmap& src,
                                                          SkBitmap* result,
                                                          const SkIRect& rect,
                                                          const SkIRect& bounds) const {
    filterPixels<UncheckedPixelFetcher>(src, result, rect, bounds);
}

void SkMatrixConvolutionImageFilter::filterBorderPixels(const SkBitmap& src,
                                                        SkBitmap* result,
                                                        const SkIRect& rect,
                                                        const SkIRect& bounds) const {
    switch (fTileMode) {
        case kClamp_TileMode:
            filterPixels<ClampPixelFetcher>(src, result, rect, bounds);
            break;
        case kRepeat_TileMode:
            filterPixels<RepeatPixelFetcher>(src, result, rect, bounds);
            break;
        case kClampToBlack_TileMode:
            filterPixels<ClampToBlackPixelFetcher>(src, result, rect, bounds);
            break;
    }
}

// FIXME:  This should be refactored to SkImageFilterUtils for
// use by other filters.  For now, we assume the input is always
// premultiplied and unpremultiply it
static SkBitmap unpremultiplyBitmap(const SkBitmap& src)
{
    SkAutoLockPixels alp(src);
    if (!src.getPixels()) {
        return SkBitmap();
    }
    SkBitmap result;
    if (!result.allocPixels(src.info())) {
        return SkBitmap();
    }
    for (int y = 0; y < src.height(); ++y) {
        const uint32_t* srcRow = src.getAddr32(0, y);
        uint32_t* dstRow = result.getAddr32(0, y);
        for (int x = 0; x < src.width(); ++x) {
            dstRow[x] = SkUnPreMultiply::PMColorToColor(srcRow[x]);
        }
    }
    return result;
}

bool SkMatrixConvolutionImageFilter::onFilterImage(Proxy* proxy,
                                                   const SkBitmap& source,
                                                   const Context& ctx,
                                                   SkBitmap* result,
                                                   SkIPoint* offset) const {
    SkBitmap src = source;
    SkIPoint srcOffset = SkIPoint::Make(0, 0);
    if (getInput(0) && !getInput(0)->filterImage(proxy, source, ctx, &src, &srcOffset)) {
        return false;
    }

    if (src.colorType() != kN32_SkColorType) {
        return false;
    }

    SkIRect bounds;
    if (!this->applyCropRect(ctx, proxy, src, &srcOffset, &bounds, &src)) {
        return false;
    }

    if (!fConvolveAlpha && !src.isOpaque()) {
        src = unpremultiplyBitmap(src);
    }

    SkAutoLockPixels alp(src);
    if (!src.getPixels()) {
        return false;
    }

    if (!result->allocPixels(src.info().makeWH(bounds.width(), bounds.height()))) {
        return false;
    }

    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    bounds.offset(-srcOffset);
    SkIRect interior = SkIRect::MakeXYWH(bounds.left() + fKernelOffset.fX,
                                         bounds.top() + fKernelOffset.fY,
                                         bounds.width() - fKernelSize.fWidth + 1,
                                         bounds.height() - fKernelSize.fHeight + 1);
    SkIRect top = SkIRect::MakeLTRB(bounds.left(), bounds.top(), bounds.right(), interior.top());
    SkIRect bottom = SkIRect::MakeLTRB(bounds.left(), interior.bottom(),
                                       bounds.right(), bounds.bottom());
    SkIRect left = SkIRect::MakeLTRB(bounds.left(), interior.top(),
                                     interior.left(), interior.bottom());
    SkIRect right = SkIRect::MakeLTRB(interior.right(), interior.top(),
                                      bounds.right(), interior.bottom());
    filterBorderPixels(src, result, top, bounds);
    filterBorderPixels(src, result, left, bounds);
    filterInteriorPixels(src, result, interior, bounds);
    filterBorderPixels(src, result, right, bounds);
    filterBorderPixels(src, result, bottom, bounds);
    return true;
}

bool SkMatrixConvolutionImageFilter::onFilterBounds(const SkIRect& src, const SkMatrix& ctm,
                                                    SkIRect* dst) const {
    SkIRect bounds = src;
    bounds.fRight += fKernelSize.width() - 1;
    bounds.fBottom += fKernelSize.height() - 1;
    bounds.offset(-fKernelOffset);
    if (getInput(0) && !getInput(0)->filterBounds(bounds, ctm, &bounds)) {
        return false;
    }
    *dst = bounds;
    return true;
}

#if SK_SUPPORT_GPU

static GrMatrixConvolutionEffect::TileMode convert_tilemodes(
        SkMatrixConvolutionImageFilter::TileMode tileMode) {
    GR_STATIC_ASSERT(static_cast<int>(SkMatrixConvolutionImageFilter::kClamp_TileMode) ==
                     static_cast<int>(GrMatrixConvolutionEffect::kClamp_TileMode));
    GR_STATIC_ASSERT(static_cast<int>(SkMatrixConvolutionImageFilter::kRepeat_TileMode) ==
                     static_cast<int>(GrMatrixConvolutionEffect::kRepeat_TileMode));
    GR_STATIC_ASSERT(static_cast<int>(SkMatrixConvolutionImageFilter::kClampToBlack_TileMode) ==
                     static_cast<int>(GrMatrixConvolutionEffect::kClampToBlack_TileMode));
    GR_STATIC_ASSERT(static_cast<int>(SkMatrixConvolutionImageFilter::kMax_TileMode) ==
                     static_cast<int>(GrMatrixConvolutionEffect::kMax_TileMode));
    return static_cast<GrMatrixConvolutionEffect::TileMode>(tileMode);
}

bool SkMatrixConvolutionImageFilter::asNewEffect(GrEffect** effect,
                                                 GrTexture* texture,
                                                 const SkMatrix&,
                                                 const SkIRect& bounds
                                                 ) const {
    if (!effect) {
        return fKernelSize.width() * fKernelSize.height() <= MAX_KERNEL_SIZE;
    }
    SkASSERT(fKernelSize.width() * fKernelSize.height() <= MAX_KERNEL_SIZE);
    *effect = GrMatrixConvolutionEffect::Create(texture,
                                                bounds,
                                                fKernelSize,
                                                fKernel,
                                                fGain,
                                                fBias,
                                                fKernelOffset,
                                                convert_tilemodes(fTileMode),
                                                fConvolveAlpha);
    return true;
}
#endif
