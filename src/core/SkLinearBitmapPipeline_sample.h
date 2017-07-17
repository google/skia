/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLinearBitmapPipeline_sampler_DEFINED
#define SkLinearBitmapPipeline_sampler_DEFINED

#include <tuple>

#include "SkAutoMalloc.h"
#include "SkColor.h"
#include "SkColorPriv.h"
#include "SkFixed.h"  // for SkFixed1 only. Don't use SkFixed in this file.
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
    PixelConverter(const SkPixmap& srcPixmap, SkColor tintColor) {
        fTintColor = SkColor4f::FromColor(tintColor);
        fTintColor.fA = 1.0f;
    }

    Sk4f toSk4f(const Element pixel) const {
        return Sk4f::Load(&fTintColor) * (pixel * (1.0f/255.0f));
    }

private:
    SkColor4f fTintColor;
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
class PixelConverter<kGray_8_SkColorType, gammaType> {
public:
    using Element = uint8_t;
    PixelConverter(const SkPixmap& srcPixmap) { }

    Sk4f toSk4f(Element pixel) const {
        float gray = (gammaType == kSRGB_SkGammaType)
            ? sk_linear_from_srgb[pixel]
            : pixel * (1/255.0f);
        return {gray, gray, gray, 1.0f};
    }
};

template <>
class PixelConverter<kRGBA_F16_SkColorType, kLinear_SkGammaType> {
public:
    using Element = uint64_t;
    PixelConverter(const SkPixmap& srcPixmap) { }

    Sk4f toSk4f(const Element pixel) const {
        return SkHalfToFloat_finite_ftz(pixel);
    }
};

class PixelAccessorShim {
public:
    explicit PixelAccessorShim(SkLinearBitmapPipeline::PixelAccessorInterface* accessor)
        : fPixelAccessor(accessor) { }

    void SK_VECTORCALL getFewPixels(
        int n, Sk4i xs, Sk4i ys, Sk4f* px0, Sk4f* px1, Sk4f* px2) const {
        fPixelAccessor->getFewPixels(n, xs, ys, px0, px1, px2);
    }

    void SK_VECTORCALL get4Pixels(
        Sk4i xs, Sk4i ys, Sk4f* px0, Sk4f* px1, Sk4f* px2, Sk4f* px3) const {
        fPixelAccessor->get4Pixels(xs, ys, px0, px1, px2, px3);
    }

    void get4Pixels(
        const void* src, int index, Sk4f* px0, Sk4f* px1, Sk4f* px2, Sk4f* px3) const {
        fPixelAccessor->get4Pixels(src, index, px0, px1, px2, px3);
    }

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
        int n, Sk4i xs, Sk4i ys, Sk4f* px0, Sk4f* px1, Sk4f* px2) const override {
        Sk4i bufferLoc = ys * fWidth + xs;
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
        Sk4i xs, Sk4i ys, Sk4f* px0, Sk4f* px1, Sk4f* px2, Sk4f* px3) const override {
        Sk4i bufferLoc = ys * fWidth + xs;
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

// -- NearestNeighborSampler -----------------------------------------------------------------------
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
        fAccessor.getFewPixels(n, SkNx_cast<int>(xs), SkNx_cast<int>(ys), &px0, &px1, &px2);
        if (n >= 1) fNext->blendPixel(px0);
        if (n >= 2) fNext->blendPixel(px1);
        if (n >= 3) fNext->blendPixel(px2);
    }

    void SK_VECTORCALL pointList4(Sk4s xs, Sk4s ys) override {
        Sk4f px0, px1, px2, px3;
        fAccessor.get4Pixels(SkNx_cast<int>(xs), SkNx_cast<int>(ys), &px0, &px1, &px2, &px3);
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

private:
    // When moving through source space more slowly than dst space (zoomed in),
    // we'll be sampling from the same source pixel more than once.
    void spanSlowRate(Span span) {
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = span;
        SkScalar x = X(start);
        // fx is a fixed 48.16 number.
        int64_t fx = static_cast<int64_t>(x * SK_Fixed1);
        SkScalar dx = length / (count - 1);
        // fdx is a fixed 48.16 number.
        int64_t fdx = static_cast<int64_t>(dx * SK_Fixed1);

        const void* row = fAccessor.row((int)std::floor(Y(start)));
        Next* next = fNext;

        int64_t ix = fx >> 16;
        int64_t prevIX = ix;
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
            ix = fx >> 16;
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

// From an edgeType, the integer value of a pixel vs, and the integer value of the extreme edge
// vMax, take the point which might be off the tile by one pixel and either wrap it or pin it to
// generate the right pixel. The value vs is on the interval [-1, vMax + 1]. It produces a value
// on the interval [0, vMax].
// Note: vMax is not width or height, but width-1 or height-1 because it is the largest valid pixel.
static inline int adjust_edge(SkShader::TileMode edgeType, int vs, int vMax) {
    SkASSERT(-1 <= vs && vs <= vMax + 1);
    switch (edgeType) {
        case SkShader::kClamp_TileMode:
        case SkShader::kMirror_TileMode:
            vs = std::max(vs, 0);
            vs = std::min(vs, vMax);
            break;
        case SkShader::kRepeat_TileMode:
            vs = (vs <= vMax) ? vs : 0;
            vs =    (vs >= 0) ? vs : vMax;
            break;
    }
    SkASSERT(0 <= vs && vs <= vMax);
    return vs;
}

// From a sample point on the tile, return the top or left filter value.
// The result r should be in the range (0, 1]. Since this represents the weight given to the top
// left element, then if x == 0.5 the filter value should be 1.0.
// The input sample point must be on the tile, therefore it must be >= 0.
static SkScalar sample_to_filter(SkScalar x) {
    SkASSERT(x >= 0.0f);
    // The usual form of the top or left edge is x - .5, but since we are working on the unit
    // square, then x + .5 works just as well. This also guarantees that v > 0.0 allowing the use
    // of trunc.
    SkScalar v = x + 0.5f;
    // Produce the top or left offset a value on the range [0, 1).
    SkScalar f = v - SkScalarTruncToScalar(v);
    // Produce the filter value which is on the range (0, 1].
    SkScalar r =  1.0f - f;
    SkASSERT(0.0f < r && r <= 1.0f);
    return r;
}

// -- BilerpSampler --------------------------------------------------------------------------------
// BilerpSampler - use a bilerp filter to create runs of destination pixels.
// Note: in the code below, there are two types of points
//       * sample points - these are the points passed in by pointList* and Spans.
//       * filter points - are created from a sample point to form the coordinates of the points
//                         to use in the filter and to generate the filter values.
template<typename Accessor, typename Next>
class BilerpSampler : public SkLinearBitmapPipeline::SampleProcessorInterface {
public:
    template<typename... Args>
    BilerpSampler(
        SkLinearBitmapPipeline::BlendProcessorInterface* next,
        SkISize dimensions,
        SkShader::TileMode xTile, SkShader::TileMode yTile,
        Args&& ... args
    )
        : fNext{next}
        , fXEdgeType{xTile}
        , fXMax{dimensions.width() - 1}
        , fYEdgeType{yTile}
        , fYMax{dimensions.height() - 1}
        , fAccessor{std::forward<Args>(args)...} { }

    BilerpSampler(SkLinearBitmapPipeline::BlendProcessorInterface* next,
                   const BilerpSampler& sampler)
        : fNext{next}
        , fXEdgeType{sampler.fXEdgeType}
        , fXMax{sampler.fXMax}
        , fYEdgeType{sampler.fYEdgeType}
        , fYMax{sampler.fYMax}
        , fAccessor{sampler.fAccessor} { }

    void SK_VECTORCALL pointListFew(int n, Sk4s xs, Sk4s ys) override {
        SkASSERT(0 < n && n < 4);
        auto bilerpPixel = [&](int index) {
            return this->bilerpSamplePoint(SkPoint{xs[index], ys[index]});
        };

        if (n >= 1) fNext->blendPixel(bilerpPixel(0));
        if (n >= 2) fNext->blendPixel(bilerpPixel(1));
        if (n >= 3) fNext->blendPixel(bilerpPixel(2));
    }

    void SK_VECTORCALL pointList4(Sk4s xs, Sk4s ys) override {
        auto bilerpPixel = [&](int index) {
            return this->bilerpSamplePoint(SkPoint{xs[index], ys[index]});
        };
        fNext->blend4Pixels(bilerpPixel(0), bilerpPixel(1), bilerpPixel(2), bilerpPixel(3));
    }

    void pointSpan(Span span) override {
        SkASSERT(!span.isEmpty());
        SkPoint start;
        SkScalar length;
        int count;
        std::tie(start, length, count) = span;

        // Nothing to do.
        if (count == 0) {
            return;
        }

        // Trivial case. No sample points are generated other than start.
        if (count == 1) {
            fNext->blendPixel(this->bilerpSamplePoint(start));
            return;
        }

        // Note: the following code could be done in terms of dx = length / (count -1), but that
        // would introduce a divide that is not needed for the most common dx == 1 cases.
        SkScalar absLength = SkScalarAbs(length);
        if (absLength == 0.0f) {
            // |dx| == 0
            // length is zero, so clamp an edge pixel.
            this->spanZeroRate(span);
        } else if (absLength < (count - 1)) {
            // 0 < |dx| < 1.
            this->spanSlowRate(span);
        } else if (absLength == (count - 1)) {
            // |dx| == 1.
            if (sample_to_filter(span.startX()) == 1.0f
                && sample_to_filter(span.startY()) == 1.0f) {
                // All the pixels are aligned with the dest; go fast.
                src_strategy_blend(span, fNext, &fAccessor);
            } else {
                // There is some sub-pixel offsets, so bilerp.
                this->spanUnitRate(span);
            }
        } else if (absLength < 2.0f * (count - 1)) {
            // 1 < |dx| < 2.
            this->spanMediumRate(span);
        } else {
            // |dx| >= 2.
            this->spanFastRate(span);
        }
    }

    void repeatSpan(Span span, int32_t repeatCount) override {
        while (repeatCount > 0) {
            this->pointSpan(span);
            repeatCount--;
        }
    }

private:

    // Convert a sample point to the points used by the filter.
    void filterPoints(SkPoint sample, Sk4i* filterXs, Sk4i* filterYs) {
        // May be less than zero. Be careful to use Floor.
        int x0 = adjust_edge(fXEdgeType, SkScalarFloorToInt(X(sample) - 0.5), fXMax);
        // Always greater than zero. Use the faster Trunc.
        int x1 = adjust_edge(fXEdgeType, SkScalarTruncToInt(X(sample) + 0.5), fXMax);
        int y0 = adjust_edge(fYEdgeType, SkScalarFloorToInt(Y(sample) - 0.5), fYMax);
        int y1 = adjust_edge(fYEdgeType, SkScalarTruncToInt(Y(sample) + 0.5), fYMax);

        *filterXs = Sk4i{x0, x1, x0, x1};
        *filterYs = Sk4i{y0, y0, y1, y1};
    }

    // Given a sample point, generate a color by bilerping the four filter points.
    Sk4f bilerpSamplePoint(SkPoint sample) {
        Sk4i iXs, iYs;
        filterPoints(sample, &iXs, &iYs);
        Sk4f px00, px10, px01, px11;
        fAccessor.get4Pixels(iXs, iYs, &px00, &px10, &px01, &px11);
        return bilerp4(Sk4f{X(sample) - 0.5f}, Sk4f{Y(sample) - 0.5f}, px00, px10, px01, px11);
    }

    // Get two pixels at x from row0 and row1.
    void get2PixelColumn(const void* row0, const void* row1, int x, Sk4f* px0, Sk4f* px1) {
        *px0 = fAccessor.getPixelFromRow(row0, x);
        *px1 = fAccessor.getPixelFromRow(row1, x);
    }

    // |dx| == 0. This code assumes that length is zero.
    void spanZeroRate(Span span) {
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = span;
        SkASSERT(length == 0.0f);

        // Filter for the blending of the top and bottom pixels.
        SkScalar filterY = sample_to_filter(Y(start));

        // Generate the four filter points from the sample point start. Generate the row* values.
        Sk4i iXs, iYs;
        this->filterPoints(start, &iXs, &iYs);
        const void* const row0 = fAccessor.row(iYs[0]);
        const void* const row1 = fAccessor.row(iYs[2]);

        // Get the two pixels that make up the clamping pixel.
        Sk4f pxTop, pxBottom;
        this->get2PixelColumn(row0, row1, SkScalarFloorToInt(X(start)), &pxTop, &pxBottom);
        Sk4f pixel = pxTop * filterY + (1.0f - filterY) * pxBottom;

        while (count >= 4) {
            fNext->blend4Pixels(pixel, pixel, pixel, pixel);
            count -= 4;
        }
        while (count > 0) {
            fNext->blendPixel(pixel);
            count -= 1;
        }
    }

    // 0 < |dx| < 1. This code reuses the calculations from previous pixels to reduce
    // computation. In particular, several destination pixels maybe generated from the same four
    // source pixels.
    // In the following code a "part" is a combination of two pixels from the same column of the
    // filter.
    void spanSlowRate(Span span) {
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = span;

        // Calculate the distance between each sample point.
        const SkScalar dx = length / (count - 1);
        SkASSERT(-1.0f < dx && dx < 1.0f && dx != 0.0f);

        // Generate the filter values for the top-left corner.
        // Note: these values are in filter space; this has implications about how to adjust
        // these values at each step. For example, as the sample point increases, the filter
        // value decreases, this is because the filter and position are related by
        // (1 - (X(sample) - .5)) % 1. The (1 - stuff) causes the filter to move in the opposite
        // direction of the sample point which is increasing by dx.
        SkScalar filterX = sample_to_filter(X(start));
        SkScalar filterY = sample_to_filter(Y(start));

        // Generate the four filter points from the sample point start. Generate the row* values.
        Sk4i iXs, iYs;
        this->filterPoints(start, &iXs, &iYs);
        const void* const row0 = fAccessor.row(iYs[0]);
        const void* const row1 = fAccessor.row(iYs[2]);

        // Generate part of the filter value at xColumn.
        auto partAtColumn = [&](int xColumn) {
            int adjustedColumn = adjust_edge(fXEdgeType, xColumn, fXMax);
            Sk4f pxTop, pxBottom;
            this->get2PixelColumn(row0, row1, adjustedColumn, &pxTop, &pxBottom);
            return pxTop * filterY + (1.0f - filterY) * pxBottom;
        };

        // The leftPart is made up of two pixels from the left column of the filter, right part
        // is similar. The top and bottom pixels in the *Part are created as a linear blend of
        // the top and bottom pixels using filterY. See the partAtColumn function above.
        Sk4f leftPart  = partAtColumn(iXs[0]);
        Sk4f rightPart = partAtColumn(iXs[1]);

        // Create a destination color by blending together a left and right part using filterX.
        auto bilerp = [&](const Sk4f& leftPart, const Sk4f& rightPart) {
            Sk4f pixel = leftPart * filterX + rightPart * (1.0f - filterX);
            return check_pixel(pixel);
        };

        // Send the first pixel to the destination. This simplifies the loop structure so that no
        // extra pixels are fetched for the last iteration of the loop.
        fNext->blendPixel(bilerp(leftPart, rightPart));
        count -= 1;

        if (dx > 0.0f) {
            // * positive direction - generate destination pixels by sliding the filter from left
            //                        to right.
            int rightPartCursor = iXs[1];

            // Advance the filter from left to right. Remember that moving the top-left corner of
            // the filter to the right actually makes the filter value smaller.
            auto advanceFilter = [&]() {
                filterX -= dx;
                if (filterX <= 0.0f) {
                    filterX += 1.0f;
                    leftPart = rightPart;
                    rightPartCursor += 1;
                    rightPart = partAtColumn(rightPartCursor);
                }
                SkASSERT(0.0f < filterX && filterX <= 1.0f);

                return bilerp(leftPart, rightPart);
            };

            while (count >= 4) {
                Sk4f px0 = advanceFilter(),
                     px1 = advanceFilter(),
                     px2 = advanceFilter(),
                     px3 = advanceFilter();
                fNext->blend4Pixels(px0, px1, px2, px3);
                count -= 4;
            }

            while (count > 0) {
                fNext->blendPixel(advanceFilter());
                count -= 1;
            }
        } else {
            // * negative direction - generate destination pixels by sliding the filter from
            //                        right to left.
            int leftPartCursor = iXs[0];

            // Advance the filter from right to left. Remember that moving the top-left corner of
            // the filter to the left actually makes the filter value larger.
            auto advanceFilter = [&]() {
                // Remember, dx < 0 therefore this adds |dx| to filterX.
                filterX -= dx;
                // At this point filterX may be > 1, and needs to be wrapped back on to the filter
                // interval, and the next column in the filter is calculated.
                if (filterX > 1.0f) {
                    filterX -= 1.0f;
                    rightPart = leftPart;
                    leftPartCursor -= 1;
                    leftPart = partAtColumn(leftPartCursor);
                }
                SkASSERT(0.0f < filterX && filterX <= 1.0f);

                return bilerp(leftPart, rightPart);
            };

            while (count >= 4) {
                Sk4f px0 = advanceFilter(),
                     px1 = advanceFilter(),
                     px2 = advanceFilter(),
                     px3 = advanceFilter();
                fNext->blend4Pixels(px0, px1, px2, px3);
                count -= 4;
            }

            while (count > 0) {
                fNext->blendPixel(advanceFilter());
                count -= 1;
            }
        }
    }

    // |dx| == 1. Moving through source space at a rate of 1 source pixel per 1 dst pixel.
    // Every filter part is used for two destination pixels, and the code can bulk load four
    // pixels at a time.
    void spanUnitRate(Span span) {
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = span;
        SkASSERT(SkScalarAbs(length) == (count - 1));

        // Calculate the four filter points of start, and use the two different Y values to
        // generate the row pointers.
        Sk4i iXs, iYs;
        filterPoints(start, &iXs, &iYs);
        const void* row0 = fAccessor.row(iYs[0]);
        const void* row1 = fAccessor.row(iYs[2]);

        // Calculate the filter values for the top-left filter element.
        const SkScalar filterX = sample_to_filter(X(start));
        const SkScalar filterY = sample_to_filter(Y(start));

        // Generate part of the filter value at xColumn.
        auto partAtColumn = [&](int xColumn) {
            int adjustedColumn = adjust_edge(fXEdgeType, xColumn, fXMax);
            Sk4f pxTop, pxBottom;
            this->get2PixelColumn(row0, row1, adjustedColumn, &pxTop, &pxBottom);
            return pxTop * filterY + (1.0f - filterY) * pxBottom;
        };

        auto get4Parts = [&](int ix, Sk4f* part0, Sk4f* part1, Sk4f* part2, Sk4f* part3) {
            // Check if the pixels needed are near the edges. If not go fast using bulk pixels,
            // otherwise be careful.
            if (0 <= ix && ix <= fXMax - 3) {
                Sk4f px00, px10, px20, px30,
                     px01, px11, px21, px31;
                fAccessor.get4Pixels(row0, ix, &px00, &px10, &px20, &px30);
                fAccessor.get4Pixels(row1, ix, &px01, &px11, &px21, &px31);
                *part0 = filterY * px00 + (1.0f - filterY) * px01;
                *part1 = filterY * px10 + (1.0f - filterY) * px11;
                *part2 = filterY * px20 + (1.0f - filterY) * px21;
                *part3 = filterY * px30 + (1.0f - filterY) * px31;
            } else {
                *part0 = partAtColumn(ix + 0);
                *part1 = partAtColumn(ix + 1);
                *part2 = partAtColumn(ix + 2);
                *part3 = partAtColumn(ix + 3);
            }
        };

        auto bilerp = [&](const Sk4f& part0, const Sk4f& part1) {
            return part0 * filterX + part1 * (1.0f - filterX);
        };

        if (length > 0) {
            // * positive direction - generate destination pixels by sliding the filter from left
            //                        to right.

            // overlapPart is the filter part from the end of the previous four pixels used at
            // the start of the next four pixels.
            Sk4f overlapPart = partAtColumn(iXs[0]);
            int rightColumnCursor = iXs[1];
            while (count >= 4) {
                Sk4f part0, part1, part2, part3;
                get4Parts(rightColumnCursor, &part0, &part1, &part2, &part3);
                Sk4f px0 = bilerp(overlapPart, part0);
                Sk4f px1 = bilerp(part0, part1);
                Sk4f px2 = bilerp(part1, part2);
                Sk4f px3 = bilerp(part2, part3);
                overlapPart = part3;
                fNext->blend4Pixels(px0, px1, px2, px3);
                rightColumnCursor += 4;
                count -= 4;
            }

            while (count > 0) {
                Sk4f rightPart = partAtColumn(rightColumnCursor);

                fNext->blendPixel(bilerp(overlapPart, rightPart));
                overlapPart = rightPart;
                rightColumnCursor += 1;
                count -= 1;
            }
        } else {
            // * negative direction - generate destination pixels by sliding the filter from
            //                        right to left.
            Sk4f overlapPart = partAtColumn(iXs[1]);
            int leftColumnCursor = iXs[0];

            while (count >= 4) {
                Sk4f part0, part1, part2, part3;
                get4Parts(leftColumnCursor - 3, &part3, &part2, &part1, &part0);
                Sk4f px0 = bilerp(part0, overlapPart);
                Sk4f px1 = bilerp(part1, part0);
                Sk4f px2 = bilerp(part2, part1);
                Sk4f px3 = bilerp(part3, part2);
                overlapPart = part3;
                fNext->blend4Pixels(px0, px1, px2, px3);
                leftColumnCursor -= 4;
                count -= 4;
            }

            while (count > 0) {
                Sk4f leftPart = partAtColumn(leftColumnCursor);

                fNext->blendPixel(bilerp(leftPart, overlapPart));
                overlapPart = leftPart;
                leftColumnCursor -= 1;
                count -= 1;
            }
        }
    }

    // 1 < |dx| < 2. Going through the source pixels at a faster rate than the dest pixels, but
    // still slow enough to take advantage of previous calculations.
    void spanMediumRate(Span span) {
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = span;

        // Calculate the distance between each sample point.
        const SkScalar dx = length / (count - 1);
        SkASSERT((-2.0f < dx && dx < -1.0f) || (1.0f < dx && dx < 2.0f));

        // Generate the filter values for the top-left corner.
        // Note: these values are in filter space; this has implications about how to adjust
        // these values at each step. For example, as the sample point increases, the filter
        // value decreases, this is because the filter and position are related by
        // (1 - (X(sample) - .5)) % 1. The (1 - stuff) causes the filter to move in the opposite
        // direction of the sample point which is increasing by dx.
        SkScalar filterX = sample_to_filter(X(start));
        SkScalar filterY = sample_to_filter(Y(start));

        // Generate the four filter points from the sample point start. Generate the row* values.
        Sk4i iXs, iYs;
        this->filterPoints(start, &iXs, &iYs);
        const void* const row0 = fAccessor.row(iYs[0]);
        const void* const row1 = fAccessor.row(iYs[2]);

        // Generate part of the filter value at xColumn.
        auto partAtColumn = [&](int xColumn) {
            int adjustedColumn = adjust_edge(fXEdgeType, xColumn, fXMax);
            Sk4f pxTop, pxBottom;
            this->get2PixelColumn(row0, row1, adjustedColumn, &pxTop, &pxBottom);
            return pxTop * filterY + (1.0f - filterY) * pxBottom;
        };

        // The leftPart is made up of two pixels from the left column of the filter, right part
        // is similar. The top and bottom pixels in the *Part are created as a linear blend of
        // the top and bottom pixels using filterY. See the nextPart function below.
        Sk4f leftPart  = partAtColumn(iXs[0]);
        Sk4f rightPart = partAtColumn(iXs[1]);

        // Create a destination color by blending together a left and right part using filterX.
        auto bilerp = [&](const Sk4f& leftPart, const Sk4f& rightPart) {
            Sk4f pixel = leftPart * filterX + rightPart * (1.0f - filterX);
            return check_pixel(pixel);
        };

        // Send the first pixel to the destination. This simplifies the loop structure so that no
        // extra pixels are fetched for the last iteration of the loop.
        fNext->blendPixel(bilerp(leftPart, rightPart));
        count -= 1;

        if (dx > 0.0f) {
            // * positive direction - generate destination pixels by sliding the filter from left
            //                        to right.
            int rightPartCursor = iXs[1];

            // Advance the filter from left to right. Remember that moving the top-left corner of
            // the filter to the right actually makes the filter value smaller.
            auto advanceFilter = [&]() {
                filterX -= dx;
                // At this point filterX is less than zero, but might actually be less than -1.
                if (filterX > -1.0f) {
                    filterX += 1.0f;
                    leftPart = rightPart;
                    rightPartCursor += 1;
                    rightPart = partAtColumn(rightPartCursor);
                } else {
                    filterX += 2.0f;
                    rightPartCursor += 2;
                    leftPart = partAtColumn(rightPartCursor - 1);
                    rightPart = partAtColumn(rightPartCursor);
                }
                SkASSERT(0.0f < filterX && filterX <= 1.0f);

                return bilerp(leftPart, rightPart);
            };

            while (count >= 4) {
                Sk4f px0 = advanceFilter(),
                     px1 = advanceFilter(),
                     px2 = advanceFilter(),
                     px3 = advanceFilter();
                fNext->blend4Pixels(px0, px1, px2, px3);
                count -= 4;
            }

            while (count > 0) {
                fNext->blendPixel(advanceFilter());
                count -= 1;
            }
        } else {
            // * negative direction - generate destination pixels by sliding the filter from
            //                        right to left.
            int leftPartCursor = iXs[0];

            auto advanceFilter = [&]() {
                // Remember, dx < 0 therefore this adds |dx| to filterX.
                filterX -= dx;
                // At this point, filterX is greater than one, but may actually be greater than two.
                if (filterX < 2.0f) {
                    filterX -= 1.0f;
                    rightPart = leftPart;
                    leftPartCursor -= 1;
                    leftPart = partAtColumn(leftPartCursor);
                } else {
                    filterX -= 2.0f;
                    leftPartCursor -= 2;
                    rightPart = partAtColumn(leftPartCursor - 1);
                    leftPart = partAtColumn(leftPartCursor);
                }
                SkASSERT(0.0f < filterX && filterX <= 1.0f);
                return bilerp(leftPart, rightPart);
            };

            while (count >= 4) {
                Sk4f px0 = advanceFilter(),
                     px1 = advanceFilter(),
                     px2 = advanceFilter(),
                     px3 = advanceFilter();
                fNext->blend4Pixels(px0, px1, px2, px3);
                count -= 4;
            }

            while (count > 0) {
                fNext->blendPixel(advanceFilter());
                count -= 1;
            }
        }
    }

    // We're moving through source space faster than dst (zoomed out),
    // so we'll never reuse a source pixel or be able to do contiguous loads.
    void spanFastRate(Span span) {
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = span;
        SkScalar x = X(start);
        SkScalar y = Y(start);

        SkScalar dx = length / (count - 1);
        while (count > 0) {
            fNext->blendPixel(this->bilerpSamplePoint(SkPoint{x, y}));
            x += dx;
            count -= 1;
        }
    }

    Next* const              fNext;
    const SkShader::TileMode fXEdgeType;
    const int                fXMax;
    const SkShader::TileMode fYEdgeType;
    const int                fYMax;
    Accessor                 fAccessor;
};

}  // namespace

#endif  // SkLinearBitmapPipeline_sampler_DEFINED
