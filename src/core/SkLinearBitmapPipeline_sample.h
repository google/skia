/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLinearBitmapPipeline_sampler_DEFINED
#define SkLinearBitmapPipeline_sampler_DEFINED

#include <tuple>

#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkFixed.h"
#include "SkHalf.h"
#include "SkLinearBitmapPipeline_core.h"
#include "SkNx.h"
#include "SkPM4fPriv.h"

namespace {
// Explaination of the math:
//              1 - x      x
//           +--------+--------+
//           |        |        |
//  1 - y    |  px00  |  px10  |
//           |        |        |
//           +--------+--------+
//           |        |        |
//    y      |  px01  |  px11  |
//           |        |        |
//           +--------+--------+
//
//
// Given a pixelxy each is multiplied by a different factor derived from the fractional part of x
// and y:
// * px00 -> (1 - x)(1 - y) = 1 - x - y + xy
// * px10 -> x(1 - y) = x - xy
// * px01 -> (1 - x)y = y - xy
// * px11 -> xy
// So x * y is calculated first and then used to calculate all the other factors.
static Sk4s SK_VECTORCALL bilerp4(Sk4s xs, Sk4s ys, Sk4f px00, Sk4f px10,
                               Sk4f px01, Sk4f px11) {
    // Calculate fractional xs and ys.
    Sk4s fxs = xs - xs.floor();
    Sk4s fys = ys - ys.floor();
    Sk4s fxys{fxs * fys};
    Sk4f sum = px11 * fxys;
    sum = sum + px01 * (fys - fxys);
    sum = sum + px10 * (fxs - fxys);
    sum = sum + px00 * (Sk4f{1.0f} - fxs - fys + fxys);
    return sum;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// PixelGetter is the lowest level interface to the source data. There is a PixelConverter for each
// of the different SkColorTypes.
template <SkColorType, SkGammaType> class PixelConverter;

// Alpha handling:
//   The alpha from the paint (tintColor) is used in the blend part of the pipeline to modulate
// the entire bitmap. So, the tint color is given an alpha of 1.0 so that the later alpha can
// modulate this color later.
template <>
class PixelConverter<kAlpha_8_SkColorType, kLinear_SkGammaType> {
public:
    using Element = uint8_t;
    PixelConverter(const SkPixmap& srcPixmap, SkColor tintColor)
        : fTintColor{set_alpha(Sk4f_from_SkColor(tintColor), 1.0f)} { }

    Sk4f toSk4f(const Element pixel) const {
        return fTintColor * (pixel * (1.0f/255.0f));
    }

private:
    const Sk4f fTintColor;
};

template <SkGammaType gammaType>
static inline Sk4f pmcolor_to_rgba(SkPMColor pixel) {
    return swizzle_rb_if_bgra(
            (gammaType == kSRGB_SkGammaType) ? Sk4f_fromS32(pixel)
                                             : Sk4f_fromL32(pixel));
}

template <SkGammaType gammaType>
class PixelConverter<kRGB_565_SkColorType, gammaType> {
public:
    using Element = uint16_t;
    PixelConverter(const SkPixmap& srcPixmap) { }

    Sk4f toSk4f(Element pixel) const {
        return pmcolor_to_rgba<gammaType>(SkPixel16ToPixel32(pixel));
    }
};

template <SkGammaType gammaType>
class PixelConverter<kARGB_4444_SkColorType, gammaType> {
public:
    using Element = uint16_t;
    PixelConverter(const SkPixmap& srcPixmap) { }

    Sk4f toSk4f(Element pixel) const {
        return pmcolor_to_rgba<gammaType>(SkPixel4444ToPixel32(pixel));
    }
};

template <SkGammaType gammaType>
class PixelConverter<kRGBA_8888_SkColorType, gammaType> {
public:
    using Element = uint32_t;
    PixelConverter(const SkPixmap& srcPixmap) { }

    Sk4f toSk4f(Element pixel) const {
        return gammaType == kSRGB_SkGammaType
               ? Sk4f_fromS32(pixel)
               : Sk4f_fromL32(pixel);
    }
};

template <SkGammaType gammaType>
class PixelConverter<kBGRA_8888_SkColorType, gammaType> {
public:
    using Element = uint32_t;
    PixelConverter(const SkPixmap& srcPixmap) { }

    Sk4f toSk4f(Element pixel) const {
        return swizzle_rb(
                   gammaType == kSRGB_SkGammaType ? Sk4f_fromS32(pixel) : Sk4f_fromL32(pixel));
    }
};

template <SkGammaType gammaType>
class PixelConverter<kIndex_8_SkColorType, gammaType> {
public:
    using Element = uint8_t;
    PixelConverter(const SkPixmap& srcPixmap) {
        SkColorTable* skColorTable = srcPixmap.ctable();
        SkASSERT(skColorTable != nullptr);

        fColorTable = (Sk4f*)SkAlign16((intptr_t)fColorTableStorage.get());
        for (int i = 0; i < skColorTable->count(); i++) {
            fColorTable[i] = pmcolor_to_rgba<gammaType>((*skColorTable)[i]);
        }
    }

    PixelConverter(const PixelConverter& strategy) {
        fColorTable = (Sk4f*)SkAlign16((intptr_t)fColorTableStorage.get());
        // TODO: figure out the count.
        for (int i = 0; i < 256; i++) {
            fColorTable[i] = strategy.fColorTable[i];
        }
    }

    Sk4f toSk4f(Element index) const {
        return fColorTable[index];
    }

private:
    static const size_t kColorTableSize = sizeof(Sk4f[256]) + 12;

    SkAutoMalloc         fColorTableStorage{kColorTableSize};
    Sk4f*                fColorTable;
};

template <SkGammaType gammaType>
class PixelConverter<kGray_8_SkColorType, gammaType> {
public:
    using Element = uint8_t;
    PixelConverter(const SkPixmap& srcPixmap) { }

    Sk4f toSk4f(Element pixel) const {
        float gray = pixel * (1.0f/255.0f);
        Sk4f result = Sk4f{gray, gray, gray, 1.0f};
        return gammaType == kSRGB_SkGammaType
               ? srgb_to_linear(result)
               : result;
    }
};

template <>
class PixelConverter<kRGBA_F16_SkColorType, kLinear_SkGammaType> {
public:
    using Element = uint64_t;
    PixelConverter(const SkPixmap& srcPixmap) { }

    Sk4f toSk4f(const Element pixel) const {
        return SkHalfToFloat_finite(pixel);
    }
};

class PixelAccessorShim {
public:
    explicit PixelAccessorShim(SkLinearBitmapPipeline::PixelAccessorInterface* accessor)
        : fPixelAccessor(accessor) { }

    void SK_VECTORCALL getFewPixels(
        int n, Sk4s xs, Sk4s ys, Sk4f* px0, Sk4f* px1, Sk4f* px2) const {
        fPixelAccessor->getFewPixels(n, xs, ys, px0, px1, px2);
    }

    void SK_VECTORCALL get4Pixels(
        Sk4s xs, Sk4s ys, Sk4f* px0, Sk4f* px1, Sk4f* px2, Sk4f* px3) const {
        fPixelAccessor->get4Pixels(xs, ys, px0, px1, px2, px3);
    }

    void get4Pixels(
        const void* src, int index, Sk4f* px0, Sk4f* px1, Sk4f* px2, Sk4f* px3) const {
        fPixelAccessor->get4Pixels(src, index, px0, px1, px2, px3);
    };

    Sk4f getPixelFromRow(const void* row, int index) const {
        return fPixelAccessor->getPixelFromRow(row, index);
    }

    Sk4f getPixelAt(int index) const {
        return fPixelAccessor->getPixelAt(index);
    }

    const void* row(int y) const {
        return fPixelAccessor->row(y);
    }

private:
    SkLinearBitmapPipeline::PixelAccessorInterface* const fPixelAccessor;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// PixelAccessor handles all the same plumbing for all the PixelGetters.
template <SkColorType colorType, SkGammaType gammaType>
class PixelAccessor final : public SkLinearBitmapPipeline::PixelAccessorInterface {
    using Element = typename PixelConverter<colorType, gammaType>::Element;
public:
    template <typename... Args>
    PixelAccessor(const SkPixmap& srcPixmap, Args&&... args)
        : fSrc{static_cast<const Element*>(srcPixmap.addr())}
        , fWidth{srcPixmap.rowBytesAsPixels()}
        , fConverter{srcPixmap, std::move<Args>(args)...} { }

    void SK_VECTORCALL getFewPixels (
        int n, Sk4s xs, Sk4s ys, Sk4f* px0, Sk4f* px1, Sk4f* px2) const override {
        Sk4i XIs = SkNx_cast<int, SkScalar>(xs);
        Sk4i YIs = SkNx_cast<int, SkScalar>(ys);
        Sk4i bufferLoc = YIs * fWidth + XIs;
        switch (n) {
            case 3:
                *px2 = this->getPixelAt(bufferLoc[2]);
            case 2:
                *px1 = this->getPixelAt(bufferLoc[1]);
            case 1:
                *px0 = this->getPixelAt(bufferLoc[0]);
            default:
                break;
        }
    }

    void SK_VECTORCALL get4Pixels(
        Sk4s xs, Sk4s ys, Sk4f* px0, Sk4f* px1, Sk4f* px2, Sk4f* px3) const override {
        Sk4i XIs = SkNx_cast<int, SkScalar>(xs);
        Sk4i YIs = SkNx_cast<int, SkScalar>(ys);
        Sk4i bufferLoc = YIs * fWidth + XIs;
        *px0 = this->getPixelAt(bufferLoc[0]);
        *px1 = this->getPixelAt(bufferLoc[1]);
        *px2 = this->getPixelAt(bufferLoc[2]);
        *px3 = this->getPixelAt(bufferLoc[3]);
    }

    void get4Pixels(
        const void* src, int index, Sk4f* px0, Sk4f* px1, Sk4f* px2, Sk4f* px3) const override {
        *px0 = this->getPixelFromRow(src, index + 0);
        *px1 = this->getPixelFromRow(src, index + 1);
        *px2 = this->getPixelFromRow(src, index + 2);
        *px3 = this->getPixelFromRow(src, index + 3);
    }

    Sk4f getPixelFromRow(const void* row, int index) const override {
        const Element* src = static_cast<const Element*>(row);
        return fConverter.toSk4f(src[index]);
    }

    Sk4f getPixelAt(int index) const override {
        return this->getPixelFromRow(fSrc, index);
    }

    const void* row(int y) const override { return fSrc + y * fWidth; }

private:
    const Element* const                 fSrc;
    const int                            fWidth;
    PixelConverter<colorType, gammaType> fConverter;
};

// We're moving through source space at a rate of 1 source pixel per 1 dst pixel.
// We'll never re-use pixels, but we can at least load contiguous pixels.
template <typename Next, typename Strategy>
static void src_strategy_blend(Span span, Next* next, Strategy* strategy) {
    SkPoint start;
    SkScalar length;
    int count;
    std::tie(start, length, count) = span;
    int ix = SkScalarFloorToInt(X(start));
    const void* row = strategy->row((int)std::floor(Y(start)));
    if (length > 0) {
        while (count >= 4) {
            Sk4f px0, px1, px2, px3;
            strategy->get4Pixels(row, ix, &px0, &px1, &px2, &px3);
            next->blend4Pixels(px0, px1, px2, px3);
            ix += 4;
            count -= 4;
        }

        while (count > 0) {
            next->blendPixel(strategy->getPixelFromRow(row, ix));
            ix += 1;
            count -= 1;
        }
    } else {
        while (count >= 4) {
            Sk4f px0, px1, px2, px3;
            strategy->get4Pixels(row, ix - 3, &px3, &px2, &px1, &px0);
            next->blend4Pixels(px0, px1, px2, px3);
            ix -= 4;
            count -= 4;
        }

        while (count > 0) {
            next->blendPixel(strategy->getPixelFromRow(row, ix));
            ix -= 1;
            count -= 1;
        }
    }
}

// NearestNeighborSampler - use nearest neighbor filtering to create runs of destination pixels.
template<typename Accessor, typename Next>
class NearestNeighborSampler : public SkLinearBitmapPipeline::SampleProcessorInterface {
public:
    template<typename... Args>
    NearestNeighborSampler(SkLinearBitmapPipeline::BlendProcessorInterface* next, Args&& ... args)
    : fNext{next}, fAccessor{std::forward<Args>(args)...} { }

    NearestNeighborSampler(SkLinearBitmapPipeline::BlendProcessorInterface* next,
    const NearestNeighborSampler& sampler)
    : fNext{next}, fAccessor{sampler.fAccessor} { }

    void SK_VECTORCALL pointListFew(int n, Sk4s xs, Sk4s ys) override {
        SkASSERT(0 < n && n < 4);
        Sk4f px0, px1, px2;
        fAccessor.getFewPixels(n, xs, ys, &px0, &px1, &px2);
        if (n >= 1) fNext->blendPixel(px0);
        if (n >= 2) fNext->blendPixel(px1);
        if (n >= 3) fNext->blendPixel(px2);
    }

    void SK_VECTORCALL pointList4(Sk4s xs, Sk4s ys) override {
        Sk4f px0, px1, px2, px3;
        fAccessor.get4Pixels(xs, ys, &px0, &px1, &px2, &px3);
        fNext->blend4Pixels(px0, px1, px2, px3);
    }

    void pointSpan(Span span) override {
        SkASSERT(!span.isEmpty());
        SkPoint start;
        SkScalar length;
        int count;
        std::tie(start, length, count) = span;
        SkScalar absLength = SkScalarAbs(length);
        if (absLength < (count - 1)) {
            this->spanSlowRate(span);
        } else if (absLength == (count - 1)) {
            src_strategy_blend(span, fNext, &fAccessor);
        } else {
            this->spanFastRate(span);
        }
    }

    void repeatSpan(Span span, int32_t repeatCount) override {
        while (repeatCount > 0) {
            this->pointSpan(span);
            repeatCount--;
        }
    }

    void SK_VECTORCALL bilerpEdge(Sk4s xs, Sk4s ys) override {
        SkFAIL("Using nearest neighbor sampler, but calling a bilerpEdge.");
    }

    void bilerpSpan(Span span, SkScalar y) override {
        SkFAIL("Using nearest neighbor sampler, but calling a bilerpSpan.");
    }

private:
    // When moving through source space more slowly than dst space (zoomed in),
    // we'll be sampling from the same source pixel more than once.
    void spanSlowRate(Span span) {
        SkPoint start;
        SkScalar length;
        int count;
        std::tie(start, length, count) = span;
        SkScalar x = X(start);
        SkFixed fx = SkScalarToFixed(x);
        SkScalar dx = length / (count - 1);
        SkFixed fdx = SkScalarToFixed(dx);

        const void* row = fAccessor.row((int)std::floor(Y(start)));
        Next* next = fNext;

        int ix = SkFixedFloorToInt(fx);
        int prevIX = ix;
        Sk4f fpixel = fAccessor.getPixelFromRow(row, ix);

        // When dx is less than one, each pixel is used more than once. Using the fixed point fx
        // allows the code to quickly check that the same pixel is being used. The code uses this
        // same pixel check to do the sRGB and normalization only once.
        auto getNextPixel = [&]() {
            if (ix != prevIX) {
                fpixel = fAccessor.getPixelFromRow(row, ix);
                prevIX = ix;
            }
            fx += fdx;
            ix = SkFixedFloorToInt(fx);
            return fpixel;
        };

        while (count >= 4) {
            Sk4f px0 = getNextPixel();
            Sk4f px1 = getNextPixel();
            Sk4f px2 = getNextPixel();
            Sk4f px3 = getNextPixel();
            next->blend4Pixels(px0, px1, px2, px3);
            count -= 4;
        }
        while (count > 0) {
            next->blendPixel(getNextPixel());
            count -= 1;
        }
    }

    // We're moving through source space at a rate of 1 source pixel per 1 dst pixel.
    // We'll never re-use pixels, but we can at least load contiguous pixels.
    void spanUnitRate(Span span) {
        src_strategy_blend(span, fNext, &fAccessor);
    }

    // We're moving through source space faster than dst (zoomed out),
    // so we'll never reuse a source pixel or be able to do contiguous loads.
    void spanFastRate(Span span) {
        span_fallback(span, this);
    }

    Next* const fNext;
    Accessor    fAccessor;
};

// -- BilerpSampler --------------------------------------------------------------------------------
// BilerpSampler - use a bilerp filter to create runs of destination pixels.
template<typename Accessor, typename Next>
class BilerpSampler : public SkLinearBitmapPipeline::SampleProcessorInterface {
public:
    template<typename... Args>
    BilerpSampler(SkLinearBitmapPipeline::BlendProcessorInterface* next, Args&& ... args)
        : fNext{next}, fAccessor{std::forward<Args>(args)...} { }

    BilerpSampler(SkLinearBitmapPipeline::BlendProcessorInterface* next,
                   const BilerpSampler& sampler)
        : fNext{next}, fAccessor{sampler.fAccessor} { }

    Sk4f bilerpNonEdgePixel(SkScalar x, SkScalar y) {
        Sk4f px00, px10, px01, px11;

        // bilerp4() expects xs, ys are the top-lefts of the 2x2 kernel.
        Sk4f xs = Sk4f{x} - 0.5f;
        Sk4f ys = Sk4f{y} - 0.5f;
        Sk4f sampleXs = xs + Sk4f{0.0f, 1.0f, 0.0f, 1.0f};
        Sk4f sampleYs = ys + Sk4f{0.0f, 0.0f, 1.0f, 1.0f};
        fAccessor.get4Pixels(sampleXs, sampleYs, &px00, &px10, &px01, &px11);
        return bilerp4(xs, ys, px00, px10, px01, px11);
    }

    void SK_VECTORCALL pointListFew(int n, Sk4s xs, Sk4s ys) override {
        SkASSERT(0 < n && n < 4);
        auto bilerpPixel = [&](int index) {
            return this->bilerpNonEdgePixel(xs[index], ys[index]);
        };

        if (n >= 1) fNext->blendPixel(bilerpPixel(0));
        if (n >= 2) fNext->blendPixel(bilerpPixel(1));
        if (n >= 3) fNext->blendPixel(bilerpPixel(2));
    }

    void SK_VECTORCALL pointList4(Sk4s xs, Sk4s ys) override {
        auto bilerpPixel = [&](int index) {
            return this->bilerpNonEdgePixel(xs[index], ys[index]);
        };
        fNext->blend4Pixels(bilerpPixel(0), bilerpPixel(1), bilerpPixel(2), bilerpPixel(3));
    }

    void pointSpan(Span span) override {
        this->bilerpSpan(span, span.startY());
    }

    void repeatSpan(Span span, int32_t repeatCount) override {
        while (repeatCount > 0) {
            this->pointSpan(span);
            repeatCount--;
        }
    }

    void SK_VECTORCALL bilerpEdge(Sk4s sampleXs, Sk4s sampleYs) override {
        Sk4f px00, px10, px01, px11;
        Sk4f xs = Sk4f{sampleXs[0]};
        Sk4f ys = Sk4f{sampleYs[0]};
        fAccessor.get4Pixels(sampleXs, sampleYs, &px00, &px10, &px01, &px11);
        Sk4f pixel = bilerp4(xs, ys, px00, px10, px01, px11);
        fNext->blendPixel(pixel);
    }

    void bilerpSpan(Span span, SkScalar y) override {
        SkASSERT(!span.isEmpty());
        SkPoint start;
        SkScalar length;
        int count;
        std::tie(start, length, count) = span;
        SkScalar absLength = SkScalarAbs(length);
        if (absLength == 0.0f) {
            this->spanZeroRate(span, y);
        } else if (absLength < (count - 1)) {
            this->spanSlowRate(span, y);
        } else if (absLength == (count - 1)) {
            if (std::fmod(span.startX() - 0.5f, 1.0f) == 0.0f) {
                if (std::fmod(span.startY() - 0.5f, 1.0f) == 0.0f) {
                    src_strategy_blend(span, fNext, &fAccessor);
                } else {
                    this->spanUnitRateAlignedX(span, y);
                }
            } else {
                this->spanUnitRate(span, y);
            }
        } else {
            this->spanFastRate(span, y);
        }
    }

private:
    void spanZeroRate(Span span, SkScalar y1) {
        SkScalar y0 = span.startY() - 0.5f;
        y1 += 0.5f;
        int iy0 = SkScalarFloorToInt(y0);
        SkScalar filterY1 = y0 - iy0;
        SkScalar filterY0 = 1.0f - filterY1;
        int iy1 = SkScalarFloorToInt(y1);
        int ix = SkScalarFloorToInt(span.startX());
        Sk4f pixelY0 = fAccessor.getPixelFromRow(fAccessor.row(iy0), ix);
        Sk4f pixelY1 = fAccessor.getPixelFromRow(fAccessor.row(iy1), ix);
        Sk4f filterPixel = pixelY0 * filterY0 + pixelY1 * filterY1;
        int count = span.count();
        while (count >= 4) {
            fNext->blend4Pixels(filterPixel, filterPixel, filterPixel, filterPixel);
            count -= 4;
        }
        while (count > 0) {
            fNext->blendPixel(filterPixel);
            count -= 1;
        }
    }

    // When moving through source space more slowly than dst space (zoomed in),
    // we'll be sampling from the same source pixel more than once.
    void spanSlowRate(Span span, SkScalar ry1) {
        SkPoint start;
        SkScalar length;
        int count;
        std::tie(start, length, count) = span;
        SkFixed fx = SkScalarToFixed(X(start)-0.5f);

        SkFixed fdx = SkScalarToFixed(length / (count - 1));

        Sk4f xAdjust;
        if (fdx >= 0) {
            xAdjust = Sk4f{-1.0f};
        } else {
            xAdjust = Sk4f{1.0f};
        }
        int ix = SkFixedFloorToInt(fx);
        int ioldx = ix;
        Sk4f x{SkFixedToScalar(fx) - ix};
        Sk4f dx{SkFixedToScalar(fdx)};
        SkScalar ry0 = Y(start) - 0.5f;
        ry1 += 0.5f;
        SkScalar yFloor = std::floor(ry0);
        Sk4f y1 = Sk4f{ry0 - yFloor};
        Sk4f y0 = Sk4f{1.0f} - y1;
        const void* const row0 = fAccessor.row(SkScalarFloorToInt(ry0));
        const void* const row1 = fAccessor.row(SkScalarFloorToInt(ry1));
        Sk4f fpixel00 = y0 * fAccessor.getPixelFromRow(row0, ix);
        Sk4f fpixel01 = y1 * fAccessor.getPixelFromRow(row1, ix);
        Sk4f fpixel10 = y0 * fAccessor.getPixelFromRow(row0, ix + 1);
        Sk4f fpixel11 = y1 * fAccessor.getPixelFromRow(row1, ix + 1);
        auto getNextPixel = [&]() {
            if (ix != ioldx) {
                fpixel00 = fpixel10;
                fpixel01 = fpixel11;
                fpixel10 = y0 * fAccessor.getPixelFromRow(row0, ix + 1);
                fpixel11 = y1 * fAccessor.getPixelFromRow(row1, ix + 1);
                ioldx = ix;
                x = x + xAdjust;
            }

            Sk4f x0, x1;
            x0 = Sk4f{1.0f} - x;
            x1 = x;
            Sk4f fpixel = x0 * (fpixel00 + fpixel01) + x1 * (fpixel10 + fpixel11);
            fx += fdx;
            ix = SkFixedFloorToInt(fx);
            x = x + dx;
            return fpixel;
        };

        while (count >= 4) {
            Sk4f fpixel0 = getNextPixel();
            Sk4f fpixel1 = getNextPixel();
            Sk4f fpixel2 = getNextPixel();
            Sk4f fpixel3 = getNextPixel();

            fNext->blend4Pixels(fpixel0, fpixel1, fpixel2, fpixel3);
            count -= 4;
        }

        while (count > 0) {
            fNext->blendPixel(getNextPixel());

            count -= 1;
        }
    }

    // We're moving through source space at a rate of 1 source pixel per 1 dst pixel.
    // We'll never re-use pixels, but we can at least load contiguous pixels.
    void spanUnitRate(Span span, SkScalar y1) {
        y1 += 0.5f;
        SkScalar y0 = span.startY() - 0.5f;
        int iy0 = SkScalarFloorToInt(y0);
        SkScalar filterY1 = y0 - iy0;
        SkScalar filterY0 = 1.0f - filterY1;
        int iy1 = SkScalarFloorToInt(y1);
        const void* rowY0 = fAccessor.row(iy0);
        const void* rowY1 = fAccessor.row(iy1);
        SkScalar x0 = span.startX() - 0.5f;
        int ix0 = SkScalarFloorToInt(x0);
        SkScalar filterX1 = x0 - ix0;
        SkScalar filterX0 = 1.0f - filterX1;

        auto getPixelY0 = [&]() {
            Sk4f px = fAccessor.getPixelFromRow(rowY0, ix0);
            return px * filterY0;
        };

        auto getPixelY1 = [&]() {
            Sk4f px = fAccessor.getPixelFromRow(rowY1, ix0);
            return px * filterY1;
        };

        auto get4PixelsY0 = [&](int ix, Sk4f* px0, Sk4f* px1, Sk4f* px2, Sk4f* px3) {
            fAccessor.get4Pixels(rowY0, ix, px0, px1, px2, px3);
            *px0 = *px0 * filterY0;
            *px1 = *px1 * filterY0;
            *px2 = *px2 * filterY0;
            *px3 = *px3 * filterY0;
        };

        auto get4PixelsY1 = [&](int ix, Sk4f* px0, Sk4f* px1, Sk4f* px2, Sk4f* px3) {
            fAccessor.get4Pixels(rowY1, ix, px0, px1, px2, px3);
            *px0 = *px0 * filterY1;
            *px1 = *px1 * filterY1;
            *px2 = *px2 * filterY1;
            *px3 = *px3 * filterY1;
        };

        auto lerp = [&](Sk4f& pixelX0, Sk4f& pixelX1) {
            return pixelX0 * filterX0 + pixelX1 * filterX1;
        };

        // Mid making 4 unit rate.
        Sk4f pxB = getPixelY0() + getPixelY1();
        if (span.length() > 0) {
            int count = span.count();
            while (count >= 4) {
                Sk4f px00, px10, px20, px30;
                get4PixelsY0(ix0, &px00, &px10, &px20, &px30);
                Sk4f px01, px11, px21, px31;
                get4PixelsY1(ix0, &px01, &px11, &px21, &px31);
                Sk4f pxS0 = px00 + px01;
                Sk4f px0 = lerp(pxB, pxS0);
                Sk4f pxS1 = px10 + px11;
                Sk4f px1 = lerp(pxS0, pxS1);
                Sk4f pxS2 = px20 + px21;
                Sk4f px2 = lerp(pxS1, pxS2);
                Sk4f pxS3 = px30 + px31;
                Sk4f px3 = lerp(pxS2, pxS3);
                pxB = pxS3;
                fNext->blend4Pixels(px0, px1, px2, px3);
                ix0 += 4;
                count -= 4;
            }
            while (count > 0) {
                Sk4f pixelY0 = fAccessor.getPixelFromRow(rowY0, ix0);
                Sk4f pixelY1 = fAccessor.getPixelFromRow(rowY1, ix0);

                fNext->blendPixel(lerp(pixelY0, pixelY1));
                ix0 += 1;
                count -= 1;
            }
        } else {
            int count = span.count();
            while (count >= 4) {
                Sk4f px00, px10, px20, px30;
                get4PixelsY0(ix0 - 3, &px00, &px10, &px20, &px30);
                Sk4f px01, px11, px21, px31;
                get4PixelsY1(ix0 - 3, &px01, &px11, &px21, &px31);
                Sk4f pxS3 = px30 + px31;
                Sk4f px0 = lerp(pxS3, pxB);
                Sk4f pxS2 = px20 + px21;
                Sk4f px1 = lerp(pxS2, pxS3);
                Sk4f pxS1 = px10 + px11;
                Sk4f px2 = lerp(pxS1, pxS2);
                Sk4f pxS0 = px00 + px01;
                Sk4f px3 = lerp(pxS0, pxS1);
                pxB = pxS0;
                fNext->blend4Pixels(px0, px1, px2, px3);
                ix0 -= 4;
                count -= 4;
            }
            while (count > 0) {
                Sk4f pixelY0 = fAccessor.getPixelFromRow(rowY0, ix0);
                Sk4f pixelY1 = fAccessor.getPixelFromRow(rowY1, ix0);

                fNext->blendPixel(lerp(pixelY0, pixelY1));
                ix0 -= 1;
                count -= 1;
            }
        }
    }

    void spanUnitRateAlignedX(Span span, SkScalar y1) {
        SkScalar y0 = span.startY() - 0.5f;
        y1 += 0.5f;
        int iy0 = SkScalarFloorToInt(y0);
        SkScalar filterY1 = y0 - iy0;
        SkScalar filterY0 = 1.0f - filterY1;
        int iy1 = SkScalarFloorToInt(y1);
        int ix = SkScalarFloorToInt(span.startX());
        const void* rowY0 = fAccessor.row(iy0);
        const void* rowY1 = fAccessor.row(iy1);
        auto lerp = [&](Sk4f* pixelY0, Sk4f* pixelY1) {
            return *pixelY0 * filterY0 + *pixelY1 * filterY1;
        };

        if (span.length() > 0) {
            int count = span.count();
            while (count >= 4) {
                Sk4f px00, px10, px20, px30;
                fAccessor.get4Pixels(rowY0, ix, &px00, &px10, &px20, &px30);
                Sk4f px01, px11, px21, px31;
                fAccessor.get4Pixels(rowY1, ix, &px01, &px11, &px21, &px31);
                fNext->blend4Pixels(
                    lerp(&px00, &px01), lerp(&px10, &px11), lerp(&px20, &px21), lerp(&px30, &px31));
                ix += 4;
                count -= 4;
            }
            while (count > 0) {
                Sk4f pixelY0 = fAccessor.getPixelFromRow(rowY0, ix);
                Sk4f pixelY1 = fAccessor.getPixelFromRow(rowY1, ix);

                fNext->blendPixel(lerp(&pixelY0, &pixelY1));
                ix += 1;
                count -= 1;
            }
        } else {
            int count = span.count();
            while (count >= 4) {
                Sk4f px00, px10, px20, px30;
                fAccessor.get4Pixels(rowY0, ix - 3, &px30, &px20, &px10, &px00);
                Sk4f px01, px11, px21, px31;
                fAccessor.get4Pixels(rowY1, ix - 3, &px31, &px21, &px11, &px01);
                fNext->blend4Pixels(
                    lerp(&px00, &px01), lerp(&px10, &px11), lerp(&px20, &px21), lerp(&px30, &px31));
                ix -= 4;
                count -= 4;
            }
            while (count > 0) {
                Sk4f pixelY0 = fAccessor.getPixelFromRow(rowY0, ix);
                Sk4f pixelY1 = fAccessor.getPixelFromRow(rowY1, ix);

                fNext->blendPixel(lerp(&pixelY0, &pixelY1));
                ix -= 1;
                count -= 1;
            }
        }
    }

    // We're moving through source space faster than dst (zoomed out),
    // so we'll never reuse a source pixel or be able to do contiguous loads.
    void spanFastRate(Span span, SkScalar y1) {
        SkPoint start;
        SkScalar length;
        int count;
        std::tie(start, length, count) = span;
        SkScalar x = X(start);
        SkScalar y = Y(start);

        // In this sampler, it is assumed that if span.StartY() and y1 are the same then both
        // y-lines are on the same tile.
        if (y == y1) {
            // Both y-lines are on the same tile.
            span_fallback(span, this);
        } else {
            // The y-lines are on different tiles.
            SkScalar dx = length / (count - 1);
            Sk4f ys = {y - 0.5f, y - 0.5f, y1 + 0.5f, y1 + 0.5f};
            while (count > 0) {
                Sk4f xs = Sk4f{-0.5f, 0.5f, -0.5f, 0.5f} + Sk4f{x};
                this->bilerpEdge(xs, ys);
                x += dx;
                count -= 1;
            }
        }
    }

    Next* const fNext;
    Accessor    fAccessor;
};

}  // namespace

#endif  // SkLinearBitmapPipeline_sampler_DEFINED
