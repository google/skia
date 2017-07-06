/*
 * Copyright 2012 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkMatrixConvolutionImageFilter.h"
#include "SkBitmap.h"
#include "SkColorPriv.h"
#include "SkColorSpaceXformer.h"
#include "SkReadBuffer.h"
#include "SkSpecialImage.h"
#include "SkWriteBuffer.h"
#include "SkRect.h"
#include "SkUnPreMultiply.h"

#if SK_SUPPORT_GPU
#include "GrContext.h"
#include "GrTextureProxy.h"
#include "effects/GrMatrixConvolutionEffect.h"
#endif

// We need to be able to read at most SK_MaxS32 bytes, so divide that
// by the size of a scalar to know how many scalars we can read.
static const int32_t gMaxKernelSize = SK_MaxS32 / sizeof(SkScalar);

SkMatrixConvolutionImageFilter::SkMatrixConvolutionImageFilter(const SkISize& kernelSize,
                                                               const SkScalar* kernel,
                                                               SkScalar gain,
                                                               SkScalar bias,
                                                               const SkIPoint& kernelOffset,
                                                               TileMode tileMode,
                                                               bool convolveAlpha,
                                                               sk_sp<SkImageFilter> input,
                                                               const CropRect* cropRect)
    : INHERITED(&input, 1, cropRect)
    , fKernelSize(kernelSize)
    , fGain(gain)
    , fBias(bias)
    , fKernelOffset(kernelOffset)
    , fTileMode(tileMode)
    , fConvolveAlpha(convolveAlpha) {
    size_t size = (size_t) sk_64_mul(fKernelSize.width(), fKernelSize.height());
    fKernel = new SkScalar[size];
    memcpy(fKernel, kernel, size * sizeof(SkScalar));
    SkASSERT(kernelSize.fWidth >= 1 && kernelSize.fHeight >= 1);
    SkASSERT(kernelOffset.fX >= 0 && kernelOffset.fX < kernelSize.fWidth);
    SkASSERT(kernelOffset.fY >= 0 && kernelOffset.fY < kernelSize.fHeight);
}

sk_sp<SkImageFilter> SkMatrixConvolutionImageFilter::Make(const SkISize& kernelSize,
                                                          const SkScalar* kernel,
                                                          SkScalar gain,
                                                          SkScalar bias,
                                                          const SkIPoint& kernelOffset,
                                                          TileMode tileMode,
                                                          bool convolveAlpha,
                                                          sk_sp<SkImageFilter> input,
                                                          const CropRect* cropRect) {
    if (kernelSize.width() < 1 || kernelSize.height() < 1) {
        return nullptr;
    }
    if (gMaxKernelSize / kernelSize.fWidth < kernelSize.fHeight) {
        return nullptr;
    }
    if (!kernel) {
        return nullptr;
    }
    if ((kernelOffset.fX < 0) || (kernelOffset.fX >= kernelSize.fWidth) ||
        (kernelOffset.fY < 0) || (kernelOffset.fY >= kernelSize.fHeight)) {
        return nullptr;
    }
    return sk_sp<SkImageFilter>(new SkMatrixConvolutionImageFilter(kernelSize, kernel, gain,
                                                                   bias, kernelOffset,
                                                                   tileMode, convolveAlpha,
                                                                   std::move(input), cropRect));
}

sk_sp<SkFlattenable> SkMatrixConvolutionImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);
    SkISize kernelSize;
    kernelSize.fWidth = buffer.readInt();
    kernelSize.fHeight = buffer.readInt();
    const int count = buffer.getArrayCount();

    const int64_t kernelArea = sk_64_mul(kernelSize.width(), kernelSize.height());
    if (!buffer.validate(kernelArea == count)) {
        return nullptr;
    }
    SkAutoSTArray<16, SkScalar> kernel(count);
    if (!buffer.readScalarArray(kernel.get(), count)) {
        return nullptr;
    }
    SkScalar gain = buffer.readScalar();
    SkScalar bias = buffer.readScalar();
    SkIPoint kernelOffset;
    kernelOffset.fX = buffer.readInt();
    kernelOffset.fY = buffer.readInt();
    TileMode tileMode = (TileMode)buffer.readInt();
    bool convolveAlpha = buffer.readBool();
    return Make(kernelSize, kernel.get(), gain, bias, kernelOffset, tileMode,
                convolveAlpha, common.getInput(0), &common.cropRect());
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
        x = SkTPin(x, bounds.fLeft, bounds.fRight - 1);
        y = SkTPin(y, bounds.fTop, bounds.fBottom - 1);
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
                        sumA += SkGetPackedA32(s) * k;
                    }
                    sumR += SkGetPackedR32(s) * k;
                    sumG += SkGetPackedG32(s) * k;
                    sumB += SkGetPackedB32(s) * k;
                }
            }
            int a = convolveAlpha
                  ? SkClampMax(SkScalarFloorToInt(sumA * fGain + fBias), 255)
                  : 255;
            int r = SkClampMax(SkScalarFloorToInt(sumR * fGain + fBias), a);
            int g = SkClampMax(SkScalarFloorToInt(sumG * fGain + fBias), a);
            int b = SkClampMax(SkScalarFloorToInt(sumB * fGain + fBias), a);
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
static SkBitmap unpremultiply_bitmap(const SkBitmap& src) {
    if (!src.getPixels()) {
        return SkBitmap();
    }

    const SkImageInfo info = SkImageInfo::MakeN32(src.width(), src.height(), src.alphaType());
    SkBitmap result;
    if (!result.tryAllocPixels(info)) {
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

#if SK_SUPPORT_GPU

static GrTextureDomain::Mode convert_tilemodes(SkMatrixConvolutionImageFilter::TileMode tileMode) {
    switch (tileMode) {
    case SkMatrixConvolutionImageFilter::kClamp_TileMode:
        return GrTextureDomain::kClamp_Mode;
    case SkMatrixConvolutionImageFilter::kRepeat_TileMode:
        return GrTextureDomain::kRepeat_Mode;
    case SkMatrixConvolutionImageFilter::kClampToBlack_TileMode:
        return GrTextureDomain::kDecal_Mode;
    default:
        SkASSERT(false);
    }
    return GrTextureDomain::kIgnore_Mode;
}
#endif

sk_sp<SkSpecialImage> SkMatrixConvolutionImageFilter::onFilterImage(SkSpecialImage* source,
                                                                    const Context& ctx,
                                                                    SkIPoint* offset) const {
    SkIPoint inputOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> input(this->filterInput(0, source, ctx, &inputOffset));
    if (!input) {
        return nullptr;
    }

    SkIRect bounds;
    input = this->applyCropRect(this->mapContext(ctx), input.get(), &inputOffset, &bounds);
    if (!input) {
        return nullptr;
    }

#if SK_SUPPORT_GPU
    // Note: if the kernel is too big, the GPU path falls back to SW
    if (source->isTextureBacked() &&
        fKernelSize.width() * fKernelSize.height() <= MAX_KERNEL_SIZE) {
        GrContext* context = source->getContext();

        // Ensure the input is in the destination color space. Typically applyCropRect will have
        // called pad_image to account for our dilation of bounds, so the result will already be
        // moved to the destination color space. If a filter DAG avoids that, then we use this
        // fall-back, which saves us from having to do the xform during the filter itself.
        input = ImageToColorSpace(input.get(), ctx.outputProperties());

        sk_sp<GrTextureProxy> inputProxy(input->asTextureProxyRef(context));
        SkASSERT(inputProxy);

        offset->fX = bounds.left();
        offset->fY = bounds.top();
        bounds.offset(-inputOffset);

        sk_sp<GrFragmentProcessor> fp(GrMatrixConvolutionEffect::Make(std::move(inputProxy),
                                                                      bounds,
                                                                      fKernelSize,
                                                                      fKernel,
                                                                      fGain,
                                                                      fBias,
                                                                      fKernelOffset,
                                                                      convert_tilemodes(fTileMode),
                                                                      fConvolveAlpha));
        if (!fp) {
            return nullptr;
        }

        return DrawWithFP(context, std::move(fp), bounds, ctx.outputProperties());
    }
#endif

    SkBitmap inputBM;

    if (!input->getROPixels(&inputBM)) {
        return nullptr;
    }

    if (inputBM.colorType() != kN32_SkColorType) {
        return nullptr;
    }

    if (!fConvolveAlpha && !inputBM.isOpaque()) {
        inputBM = unpremultiply_bitmap(inputBM);
    }

    if (!inputBM.getPixels()) {
        return nullptr;
    }

    const SkImageInfo info = SkImageInfo::MakeN32(bounds.width(), bounds.height(),
                                                  inputBM.alphaType());

    SkBitmap dst;
    if (!dst.tryAllocPixels(info)) {
        return nullptr;
    }

    offset->fX = bounds.fLeft;
    offset->fY = bounds.fTop;
    bounds.offset(-inputOffset);
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
    this->filterBorderPixels(inputBM, &dst, top, bounds);
    this->filterBorderPixels(inputBM, &dst, left, bounds);
    this->filterInteriorPixels(inputBM, &dst, interior, bounds);
    this->filterBorderPixels(inputBM, &dst, right, bounds);
    this->filterBorderPixels(inputBM, &dst, bottom, bounds);
    return SkSpecialImage::MakeFromRaster(SkIRect::MakeWH(bounds.width(), bounds.height()),
                                          dst);
}

sk_sp<SkImageFilter> SkMatrixConvolutionImageFilter::onMakeColorSpace(SkColorSpaceXformer* xformer)
const {
    SkASSERT(1 == this->countInputs());

    sk_sp<SkImageFilter> input = xformer->apply(this->getInput(0));
    if (input.get() != this->getInput(0)) {
        return SkMatrixConvolutionImageFilter::Make(fKernelSize, fKernel, fGain, fBias,
                                                    fKernelOffset, fTileMode, fConvolveAlpha,
                                                    std::move(input), this->getCropRectIfSet());
    }
    return this->refMe();
}

SkIRect SkMatrixConvolutionImageFilter::onFilterNodeBounds(const SkIRect& src, const SkMatrix& ctm,
                                                           MapDirection direction) const {
    SkIRect dst = src;
    int w = fKernelSize.width() - 1, h = fKernelSize.height() - 1;
    dst.fRight += w;
    dst.fBottom += h;
    if (kReverse_MapDirection == direction) {
        dst.offset(-fKernelOffset);
    } else {
        dst.offset(fKernelOffset - SkIPoint::Make(w, h));
    }
    return dst;
}

bool SkMatrixConvolutionImageFilter::affectsTransparentBlack() const {
    // Because the kernel is applied in device-space, we have no idea what
    // pixels it will affect in object-space.
    return true;
}

#ifndef SK_IGNORE_TO_STRING
void SkMatrixConvolutionImageFilter::toString(SkString* str) const {
    str->appendf("SkMatrixConvolutionImageFilter: (");
    str->appendf("size: (%d,%d) kernel: (", fKernelSize.width(), fKernelSize.height());
    for (int y = 0; y < fKernelSize.height(); y++) {
        for (int x = 0; x < fKernelSize.width(); x++) {
            str->appendf("%f ", fKernel[y * fKernelSize.width() + x]);
        }
    }
    str->appendf(")");
    str->appendf("gain: %f bias: %f ", fGain, fBias);
    str->appendf("offset: (%d, %d) ", fKernelOffset.fX, fKernelOffset.fY);
    str->appendf("convolveAlpha: %s", fConvolveAlpha ? "true" : "false");
    str->append(")");
}
#endif
