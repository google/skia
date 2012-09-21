/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMatrixConvolutionImageFilter.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkFlattenableBuffers.h"
#include "SkRect.h"

SkMatrixConvolutionImageFilter::SkMatrixConvolutionImageFilter(const SkISize& kernelSize, const SkScalar* kernel, SkScalar gain, SkScalar bias, const SkIPoint& target, TileMode tileMode, SkImageFilter* input)
  : INHERITED(input),
    fKernelSize(kernelSize),
    fGain(gain),
    fBias(bias),
    fTarget(target),
    fTileMode(tileMode) {
    uint32_t size = fKernelSize.fWidth * fKernelSize.fHeight;
    fKernel = SkNEW_ARRAY(SkScalar, size);
    memcpy(fKernel, kernel, size * sizeof(SkScalar));
    SkASSERT(target.fX >= 0 && target.fX < kernelSize.fWidth);
    SkASSERT(target.fY >= 0 && target.fY < kernelSize.fHeight);
}

SkMatrixConvolutionImageFilter::SkMatrixConvolutionImageFilter(SkFlattenableReadBuffer& buffer) : INHERITED(buffer) {
    fKernelSize.fWidth = buffer.readInt();
    fKernelSize.fHeight = buffer.readInt();
    uint32_t size = fKernelSize.fWidth * fKernelSize.fHeight;
    fKernel = SkNEW_ARRAY(SkScalar, size);
    uint32_t readSize = buffer.readScalarArray(fKernel);
    SkASSERT(readSize == size);
    fGain = buffer.readScalar();
    fBias = buffer.readScalar();
    fTarget.fX = buffer.readScalar();
    fTarget.fY = buffer.readScalar();
    fTileMode = (TileMode) buffer.readInt();
}

void SkMatrixConvolutionImageFilter::flatten(SkFlattenableWriteBuffer& buffer) const {
    this->INHERITED::flatten(buffer);
    buffer.writeInt(fKernelSize.fWidth);
    buffer.writeInt(fKernelSize.fHeight);
    buffer.writeScalarArray(fKernel, fKernelSize.fWidth * fKernelSize.fHeight);
    buffer.writeScalar(fGain);
    buffer.writeScalar(fBias);
    buffer.writeScalar(fTarget.fX);
    buffer.writeScalar(fTarget.fY);
    buffer.writeInt((int) fTileMode);
}

SkMatrixConvolutionImageFilter::~SkMatrixConvolutionImageFilter() {
    delete[] fKernel;
}

class UncheckedPixelFetcher {
public:
    static inline SkPMColor fetch(const SkBitmap& src, int x, int y) {
        return *src.getAddr32(x, y);
    }
};

class ClampPixelFetcher {
public:
    static inline SkPMColor fetch(const SkBitmap& src, int x, int y) {
        x = SkClampMax(x, src.width() - 1);
        y = SkClampMax(y, src.height() - 1);
        return *src.getAddr32(x, y);
    }
};

class RepeatPixelFetcher {
public:
    static inline SkPMColor fetch(const SkBitmap& src, int x, int y) {
        x %= src.width();
        y %= src.height();
        if (x < 0) {
            x += src.width();
        }
        if (y < 0) {
            y += src.height();
        }
        return *src.getAddr32(x, y);
    }
};

class ClampToBlackPixelFetcher {
public:
    static inline SkPMColor fetch(const SkBitmap& src, int x, int y) {
        if (x < 0 || x >= src.width() || y < 0 || y >= src.height()) {
            return 0;
        } else {
            return *src.getAddr32(x, y);
        }
    }
};

template<class PixelFetcher>
void SkMatrixConvolutionImageFilter::filterPixels(const SkBitmap& src, SkBitmap* result, const SkIRect& rect) {
    for (int y = rect.fTop; y < rect.fBottom; ++y) {
        SkPMColor* dptr = result->getAddr32(rect.fLeft, y);
        for (int x = rect.fLeft; x < rect.fRight; ++x) {
            SkScalar sumA = 0, sumR = 0, sumG = 0, sumB = 0;
            for (int cy = 0; cy < fKernelSize.fHeight; cy++) {
                for (int cx = 0; cx < fKernelSize.fWidth; cx++) {
                    SkPMColor s = PixelFetcher::fetch(src, x + cx - fTarget.fX, y + cy - fTarget.fY);
                    SkScalar k = fKernel[cy * fKernelSize.fWidth + cx];
                    sumA += SkScalarMul(SkIntToScalar(SkGetPackedA32(s)), k);
                    sumR += SkScalarMul(SkIntToScalar(SkGetPackedR32(s)), k);
                    sumG += SkScalarMul(SkIntToScalar(SkGetPackedG32(s)), k);
                    sumB += SkScalarMul(SkIntToScalar(SkGetPackedB32(s)), k);
                }
            }
            int a = SkClampMax(SkScalarFloorToInt(SkScalarMul(sumA, fGain) + fBias), 255);
            int r = SkClampMax(SkScalarFloorToInt(SkScalarMul(sumR, fGain) + fBias), a);
            int g = SkClampMax(SkScalarFloorToInt(SkScalarMul(sumG, fGain) + fBias), a);
            int b = SkClampMax(SkScalarFloorToInt(SkScalarMul(sumB, fGain) + fBias), a);
            *dptr++ = SkPackARGB32(a, r, g, b);
        }
    }
}

void SkMatrixConvolutionImageFilter::filterInteriorPixels(const SkBitmap& src, SkBitmap* result, const SkIRect& rect) {
    filterPixels<UncheckedPixelFetcher>(src, result, rect);
}

void SkMatrixConvolutionImageFilter::filterBorderPixels(const SkBitmap& src, SkBitmap* result, const SkIRect& rect) {
    switch (fTileMode) {
        case kClamp_TileMode:
            filterPixels<ClampPixelFetcher>(src, result, rect);
            break;
        case kRepeat_TileMode:
            filterPixels<RepeatPixelFetcher>(src, result, rect);
            break;
        case kClampToBlack_TileMode:
            filterPixels<ClampToBlackPixelFetcher>(src, result, rect);
            break;
    }
}

bool SkMatrixConvolutionImageFilter::onFilterImage(Proxy* proxy,
                                                   const SkBitmap& source,
                                                   const SkMatrix& matrix,
                                                   SkBitmap* result,
                                                   SkIPoint* loc) {
    SkBitmap src = this->getInputResult(proxy, source, matrix, loc);
    if (src.config() != SkBitmap::kARGB_8888_Config) {
        return false;
    }

    SkAutoLockPixels alp(src);
    if (!src.getPixels()) {
        return false;
    }

    result->setConfig(src.config(), src.width(), src.height());
    result->allocPixels();

    SkIRect interior = SkIRect::MakeXYWH(fTarget.fX, fTarget.fY,
                                         src.width() - fKernelSize.fWidth + 1,
                                         src.height() - fKernelSize.fHeight + 1);
    SkIRect top = SkIRect::MakeWH(src.width(), fTarget.fY);
    SkIRect bottom = SkIRect::MakeLTRB(0, interior.bottom(),
                                       src.width(), src.height());
    SkIRect left = SkIRect::MakeXYWH(0, interior.top(),
                                     fTarget.fX, interior.height());
    SkIRect right = SkIRect::MakeLTRB(interior.right(), interior.top(),
                                      src.width(), interior.bottom());
    filterBorderPixels(src, result, top);
    filterBorderPixels(src, result, left);
    filterInteriorPixels(src, result, interior);
    filterBorderPixels(src, result, right);
    filterBorderPixels(src, result, bottom);
    return true;
}
