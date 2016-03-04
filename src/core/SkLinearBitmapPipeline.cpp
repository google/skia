/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLinearBitmapPipeline.h"

#include "SkPM4f.h"
#include <algorithm>
#include <cmath>
#include <limits>
#include "SkColor.h"
#include "SkSize.h"
#include <tuple>
#include "SkLinearBitmapPipeline_core.h"
#include "SkLinearBitmapPipeline_matrix.h"
#include "SkLinearBitmapPipeline_tile.h"

class SkLinearBitmapPipeline::PointProcessorInterface {
public:
    virtual ~PointProcessorInterface() { }
    virtual void VECTORCALL pointListFew(int n, Sk4s xs, Sk4s ys) = 0;
    virtual void VECTORCALL pointList4(Sk4s xs, Sk4s ys) = 0;
    virtual void pointSpan(Span span) = 0;
};

class SkLinearBitmapPipeline::BilerpProcessorInterface
    : public SkLinearBitmapPipeline::PointProcessorInterface {
public:
    // The x's and y's are setup in the following order:
    // +--------+--------+
    // |        |        |
    // |  px00  |  px10  |
    // |    0   |    1   |
    // +--------+--------+
    // |        |        |
    // |  px01  |  px11  |
    // |    2   |    3   |
    // +--------+--------+
    // These pixels coordinates are arranged in the following order in xs and ys:
    // px00  px10  px01  px11
    virtual void VECTORCALL bilerpList(Sk4s xs, Sk4s ys) = 0;
    virtual void bilerpSpan(BilerpSpan span) = 0;
};

class SkLinearBitmapPipeline::PixelPlacerInterface {
public:
    virtual ~PixelPlacerInterface() { }
    virtual void setDestination(SkPM4f* dst) = 0;
    virtual void VECTORCALL placePixel(Sk4f pixel0) = 0;
    virtual void VECTORCALL place4Pixels(Sk4f p0, Sk4f p1, Sk4f p2, Sk4f p3) = 0;
};

namespace  {

// PointProcessor uses a strategy to help complete the work of the different stages. The strategy
// must implement the following methods:
// * processPoints(xs, ys) - must mutate the xs and ys for the stage.
// * maybeProcessSpan(span, next) - This represents a horizontal series of pixels
//   to work over.
//   span - encapsulation of span.
//   next - a pointer to the next stage.
//   maybeProcessSpan - returns false if it can not process the span and needs to fallback to
//                      point lists for processing.
template<typename Strategy, typename Next>
class PointProcessor final : public SkLinearBitmapPipeline::PointProcessorInterface {
public:
    template <typename... Args>
    PointProcessor(Next* next, Args&&... args)
        : fNext{next}
        , fStrategy{std::forward<Args>(args)...}{ }

    void VECTORCALL pointListFew(int n, Sk4s xs, Sk4s ys) override {
        fStrategy.processPoints(&xs, &ys);
        fNext->pointListFew(n, xs, ys);
    }

    void VECTORCALL pointList4(Sk4s xs, Sk4s ys) override {
        fStrategy.processPoints(&xs, &ys);
        fNext->pointList4(xs, ys);
    }

    // The span you pass must not be empty.
    void pointSpan(Span span) override {
        SkASSERT(!span.isEmpty());
        if (!fStrategy.maybeProcessSpan(span, fNext)) {
            span_fallback(span, this);
        }
    }

private:
    Next* const fNext;
    Strategy fStrategy;
};

// See PointProcessor for responsibilities of Strategy.
template<typename Strategy, typename Next>
class BilerpProcessor final : public SkLinearBitmapPipeline::BilerpProcessorInterface  {
public:
    template <typename... Args>
    BilerpProcessor(Next* next, Args&&... args)
        : fNext{next}
        , fStrategy{std::forward<Args>(args)...}{ }

    void VECTORCALL pointListFew(int n, Sk4s xs, Sk4s ys) override {
        fStrategy.processPoints(&xs, &ys);
        fNext->pointListFew(n, xs, ys);
    }

    void VECTORCALL pointList4(Sk4s xs, Sk4s ys) override {
        fStrategy.processPoints(&xs, &ys);
        fNext->pointList4(xs, ys);
    }

    void VECTORCALL bilerpList(Sk4s xs, Sk4s ys) override {
        fStrategy.processPoints(&xs, &ys);
        fNext->bilerpList(xs, ys);
    }

    void pointSpan(Span span) override {
        SkASSERT(!span.isEmpty());
        if (!fStrategy.maybeProcessSpan(span, fNext)) {
            span_fallback(span, this);
        }
    }

    void bilerpSpan(BilerpSpan bSpan) override {
        SkASSERT(!bSpan.isEmpty());
        if (!fStrategy.maybeProcessBilerpSpan(bSpan, fNext)) {
            bilerp_span_fallback(bSpan, this);
        }
    }

private:
    Next* const fNext;
    Strategy fStrategy;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// Matrix Stage
template <typename Next = SkLinearBitmapPipeline::PointProcessorInterface>
using TranslateMatrix = PointProcessor<TranslateMatrixStrategy, Next>;

template <typename Next = SkLinearBitmapPipeline::PointProcessorInterface>
using ScaleMatrix = PointProcessor<ScaleMatrixStrategy, Next>;

template <typename Next = SkLinearBitmapPipeline::PointProcessorInterface>
using AffineMatrix = PointProcessor<AffineMatrixStrategy, Next>;

static SkLinearBitmapPipeline::PointProcessorInterface* choose_matrix(
    SkLinearBitmapPipeline::PointProcessorInterface* next,
    const SkMatrix& inverse,
    SkLinearBitmapPipeline::MatrixStage* matrixProc) {
    if (inverse.hasPerspective()) {
        SkFAIL("Not implemented.");
    } else if (inverse.getSkewX() != 0.0f || inverse.getSkewY() != 0.0f) {
        matrixProc->Initialize<AffineMatrix<>>(
            next,
            SkVector{inverse.getTranslateX(), inverse.getTranslateY()},
            SkVector{inverse.getScaleX(), inverse.getScaleY()},
            SkVector{inverse.getSkewX(), inverse.getSkewY()});
    } else if (inverse.getScaleX() != 1.0f || inverse.getScaleY() != 1.0f) {
        matrixProc->Initialize<ScaleMatrix<>>(
            next,
            SkVector{inverse.getTranslateX(), inverse.getTranslateY()},
            SkVector{inverse.getScaleX(), inverse.getScaleY()});
    } else if (inverse.getTranslateX() != 0.0f || inverse.getTranslateY() != 0.0f) {
        matrixProc->Initialize<TranslateMatrix<>>(
            next,
            SkVector{inverse.getTranslateX(), inverse.getTranslateY()});
    } else {
        return next;
    }
    return matrixProc->get();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Bilerp Expansion Stage
template <typename Next = SkLinearBitmapPipeline::BilerpProcessorInterface>
class ExpandBilerp final : public SkLinearBitmapPipeline::PointProcessorInterface {
public:
    ExpandBilerp(Next* next) : fNext{next} { }

    void VECTORCALL pointListFew(int n, Sk4s xs, Sk4s ys) override {
        SkASSERT(0 < n && n < 4);
        //                    px00   px10   px01  px11
        const Sk4s kXOffsets{-0.5f,  0.5f, -0.5f, 0.5f},
                   kYOffsets{-0.5f, -0.5f,  0.5f, 0.5f};
        if (n >= 1) fNext->bilerpList(Sk4s{xs[0]} + kXOffsets, Sk4s{ys[0]} + kYOffsets);
        if (n >= 2) fNext->bilerpList(Sk4s{xs[1]} + kXOffsets, Sk4s{ys[1]} + kYOffsets);
        if (n >= 3) fNext->bilerpList(Sk4s{xs[2]} + kXOffsets, Sk4s{ys[2]} + kYOffsets);
    }

    void VECTORCALL pointList4(Sk4f xs, Sk4f ys) override {
        //                    px00   px10   px01  px11
        const Sk4f kXOffsets{-0.5f,  0.5f, -0.5f, 0.5f},
                   kYOffsets{-0.5f, -0.5f,  0.5f, 0.5f};
        fNext->bilerpList(Sk4s{xs[0]} + kXOffsets, Sk4s{ys[0]} + kYOffsets);
        fNext->bilerpList(Sk4s{xs[1]} + kXOffsets, Sk4s{ys[1]} + kYOffsets);
        fNext->bilerpList(Sk4s{xs[2]} + kXOffsets, Sk4s{ys[2]} + kYOffsets);
        fNext->bilerpList(Sk4s{xs[3]} + kXOffsets, Sk4s{ys[3]} + kYOffsets);
    }

    void pointSpan(Span span) override {
        SkASSERT(!span.isEmpty());
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = span;
        // Adjust the span so that it is in the correct phase with the pixel.
        BilerpSpan bSpan{X(start) - 0.5f, Y(start) - 0.5f, Y(start) + 0.5f, length, count};
        fNext->bilerpSpan(bSpan);
    }

private:
    Next* const fNext;
};

static SkLinearBitmapPipeline::PointProcessorInterface* choose_filter(
    SkLinearBitmapPipeline::BilerpProcessorInterface* next,
    SkFilterQuality filterQuailty,
    SkLinearBitmapPipeline::FilterStage* filterProc) {
    if (SkFilterQuality::kNone_SkFilterQuality == filterQuailty) {
        return next;
    } else {
        filterProc->Initialize<ExpandBilerp<>>(next);
        return filterProc->get();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Tile Stage
template <typename Next = SkLinearBitmapPipeline::BilerpProcessorInterface>
using Clamp = BilerpProcessor<ClampStrategy, Next>;

template <typename Next = SkLinearBitmapPipeline::BilerpProcessorInterface>
using Repeat = BilerpProcessor<RepeatStrategy, Next>;

static SkLinearBitmapPipeline::BilerpProcessorInterface* choose_tiler(
    SkLinearBitmapPipeline::BilerpProcessorInterface* next,
    SkSize dimensions,
    SkShader::TileMode xMode,
    SkShader::TileMode yMode,
    SkLinearBitmapPipeline::TileStage* tileProcXOrBoth,
    SkLinearBitmapPipeline::TileStage* tileProcY) {
    if (xMode == yMode) {
        switch (xMode) {
            case SkShader::kClamp_TileMode:
                tileProcXOrBoth->Initialize<Clamp<>>(next, dimensions);
                break;
            case SkShader::kRepeat_TileMode:
                tileProcXOrBoth->Initialize<Repeat<>>(next, dimensions);
                break;
            case SkShader::kMirror_TileMode:
                SkFAIL("Not implemented.");
                break;
        }
    } else {
        switch (yMode) {
            case SkShader::kClamp_TileMode:
                tileProcY->Initialize<Clamp<>>(next, Y(dimensions));
                break;
            case SkShader::kRepeat_TileMode:
                tileProcY->Initialize<Repeat<>>(next, Y(dimensions));
                break;
            case SkShader::kMirror_TileMode:
                SkFAIL("Not implemented.");
                break;
        }
        switch (xMode) {
            case SkShader::kClamp_TileMode:
                tileProcXOrBoth->Initialize<Clamp<>>(tileProcY->get(), X(dimensions));
                break;
            case SkShader::kRepeat_TileMode:
                tileProcXOrBoth->Initialize<Repeat<>>(tileProcY->get(), X(dimensions));
                break;
            case SkShader::kMirror_TileMode:
                SkFAIL("Not implemented.");
                break;
        }
    }
    return tileProcXOrBoth->get();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Source Sampling Stage
class sRGBFast {
public:
    static Sk4s VECTORCALL sRGBToLinear(Sk4s pixel) {
        Sk4s l = pixel * pixel;
        return Sk4s{l[0], l[1], l[2], pixel[3]};
    }
};

enum class ColorOrder {
    kRGBA = false,
    kBGRA = true,
};
template <SkColorProfileType colorProfile, ColorOrder colorOrder>
class Pixel8888 {
public:
    Pixel8888(int width, const uint32_t* src) : fSrc{src}, fWidth{width}{ }
    Pixel8888(const SkPixmap& srcPixmap)
        : fSrc{srcPixmap.addr32()}
        , fWidth{static_cast<int>(srcPixmap.rowBytes() / 4)} { }

    void VECTORCALL getFewPixels(int n, Sk4s xs, Sk4s ys, Sk4f* px0, Sk4f* px1, Sk4f* px2) {
        Sk4i XIs = SkNx_cast<int, SkScalar>(xs);
        Sk4i YIs = SkNx_cast<int, SkScalar>(ys);
        Sk4i bufferLoc = YIs * fWidth + XIs;
        switch (n) {
            case 3:
                *px2 = this->getPixel(fSrc, bufferLoc[2]);
            case 2:
                *px1 = this->getPixel(fSrc, bufferLoc[1]);
            case 1:
                *px0 = this->getPixel(fSrc, bufferLoc[0]);
            default:
                break;
        }
    }

    void VECTORCALL get4Pixels(Sk4s xs, Sk4s ys, Sk4f* px0, Sk4f* px1, Sk4f* px2, Sk4f* px3) {
        Sk4i XIs = SkNx_cast<int, SkScalar>(xs);
        Sk4i YIs = SkNx_cast<int, SkScalar>(ys);
        Sk4i bufferLoc = YIs * fWidth + XIs;
        *px0 = this->getPixel(fSrc, bufferLoc[0]);
        *px1 = this->getPixel(fSrc, bufferLoc[1]);
        *px2 = this->getPixel(fSrc, bufferLoc[2]);
        *px3 = this->getPixel(fSrc, bufferLoc[3]);
    }

    void get4Pixels(const void* vsrc, int index, Sk4f* px0, Sk4f* px1, Sk4f* px2, Sk4f* px3) {
        const uint32_t* src = static_cast<const uint32_t*>(vsrc);
        *px0 = this->getPixel(src, index + 0);
        *px1 = this->getPixel(src, index + 1);
        *px2 = this->getPixel(src, index + 2);
        *px3 = this->getPixel(src, index + 3);
    }

    Sk4f getPixel(const void* vsrc, int index) {
        const uint32_t* src = static_cast<const uint32_t*>(vsrc);
        Sk4b bytePixel = Sk4b::Load((uint8_t *)(&src[index]));
        Sk4f pixel = SkNx_cast<float, uint8_t>(bytePixel);
        if (colorOrder == ColorOrder::kBGRA) {
            pixel = SkNx_shuffle<2, 1, 0, 3>(pixel);
        }
        pixel = pixel * Sk4f{1.0f/255.0f};
        if (colorProfile == kSRGB_SkColorProfileType) {
            pixel = sRGBFast::sRGBToLinear(pixel);
        }
        return pixel;
    }

    const uint32_t* row(int y) { return fSrc + y * fWidth[0]; }

private:
    const uint32_t* const fSrc;
    const Sk4i fWidth;
};

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
static Sk4s VECTORCALL bilerp4(Sk4s xs, Sk4s ys, Sk4f px00, Sk4f px10,
                                                 Sk4f px01, Sk4f px11) {
    // Calculate fractional xs and ys.
    Sk4s fxs = xs - xs.floor();
    Sk4s fys = ys - ys.floor();
    Sk4s fxys{fxs * fys};
    Sk4f sum =  px11 * fxys;
    sum = sum + px01 * (fys - fxys);
    sum = sum + px10 * (fxs - fxys);
    sum = sum + px00 * (Sk4f{1.0f} - fxs - fys + fxys);
    return sum;
}

template <typename SourceStrategy>
class Sampler final : public SkLinearBitmapPipeline::BilerpProcessorInterface {
public:
    template <typename... Args>
    Sampler(SkLinearBitmapPipeline::PixelPlacerInterface* next, Args&&... args)
        : fNext{next}
        , fStrategy{std::forward<Args>(args)...} { }

    void VECTORCALL pointListFew(int n, Sk4s xs, Sk4s ys) override {
        SkASSERT(0 < n && n < 4);
        Sk4f px0, px1, px2;
        fStrategy.getFewPixels(n, xs, ys, &px0, &px1, &px2);
        if (n >= 1) fNext->placePixel(px0);
        if (n >= 2) fNext->placePixel(px1);
        if (n >= 3) fNext->placePixel(px2);
    }

    void VECTORCALL pointList4(Sk4s xs, Sk4s ys) override {
        Sk4f px0, px1, px2, px3;
        fStrategy.get4Pixels(xs, ys, &px0, &px1, &px2, &px3);
        fNext->place4Pixels(px0, px1, px2, px3);
    }

    void VECTORCALL bilerpList(Sk4s xs, Sk4s ys) override {
        Sk4f px00, px10, px01, px11;
        fStrategy.get4Pixels(xs, ys, &px00, &px10, &px01, &px11);
        Sk4f pixel = bilerp4(xs, ys, px00, px10, px01, px11);
        fNext->placePixel(pixel);
    }

    void pointSpan(Span span) override {
        SkASSERT(!span.isEmpty());
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = span;
        if (length < (count - 1)) {
            this->pointSpanSlowRate(span);
        } else if (length == (count - 1)) {
            this->pointSpanUnitRate(span);
        } else {
            this->pointSpanFastRate(span);
        }
    }

private:
    // When moving through source space more slowly than dst space (zoomed in),
    // we'll be sampling from the same source pixel more than once.
    void pointSpanSlowRate(Span span) {
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = span;
        SkScalar x = X(start);
        SkFixed fx = SkScalarToFixed(x);
        SkScalar dx = length / (count - 1);
        SkFixed fdx = SkScalarToFixed(dx);

        const void* row = fStrategy.row((int)std::floor(Y(start)));
        SkLinearBitmapPipeline::PixelPlacerInterface* next = fNext;

        int ix = SkFixedFloorToInt(fx);
        int prevIX = ix;
        Sk4f fpixel = fStrategy.getPixel(row, ix);

        // When dx is less than one, each pixel is used more than once. Using the fixed point fx
        // allows the code to quickly check that the same pixel is being used. The code uses this
        // same pixel check to do the sRGB and normalization only once.
        auto getNextPixel = [&]() {
            if (ix != prevIX) {
                fpixel = fStrategy.getPixel(row, ix);
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
            next->place4Pixels(px0, px1, px2, px3);
            count -= 4;
        }
        while (count > 0) {
            next->placePixel(getNextPixel());
            count -= 1;
        }
    }

    // We're moving through source space at a rate of 1 source pixel per 1 dst pixel.
    // We'll never re-use pixels, but we can at least load contiguous pixels.
    void pointSpanUnitRate(Span span) {
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = span;
        int ix = SkScalarFloorToInt(X(start));
        const void* row = fStrategy.row((int)std::floor(Y(start)));
        SkLinearBitmapPipeline::PixelPlacerInterface* next = fNext;
        while (count >= 4) {
            Sk4f px0, px1, px2, px3;
            fStrategy.get4Pixels(row, ix, &px0, &px1, &px2, &px3);
            next->place4Pixels(px0, px1, px2, px3);
            ix += 4;
            count -= 4;
        }

        while (count > 0) {
            next->placePixel(fStrategy.getPixel(row, ix));
            ix += 1;
            count -= 1;
        }
    }

    // We're moving through source space faster than dst (zoomed out),
    // so we'll never reuse a source pixel or be able to do contiguous loads.
    void pointSpanFastRate(Span span) {
        span_fallback(span, this);
    }

    void bilerpSpan(BilerpSpan span) override {
        bilerp_span_fallback(span, this);
    }

private:
    SkLinearBitmapPipeline::PixelPlacerInterface* const fNext;
    SourceStrategy fStrategy;
};

using Pixel8888SRGB = Pixel8888<kSRGB_SkColorProfileType, ColorOrder::kRGBA>;
using Pixel8888LRGB = Pixel8888<kLinear_SkColorProfileType, ColorOrder::kRGBA>;
using Pixel8888SBGR = Pixel8888<kSRGB_SkColorProfileType, ColorOrder::kBGRA>;
using Pixel8888LBGR = Pixel8888<kLinear_SkColorProfileType, ColorOrder::kBGRA>;

static SkLinearBitmapPipeline::BilerpProcessorInterface* choose_pixel_sampler(
    SkLinearBitmapPipeline::PixelPlacerInterface* next,
    const SkPixmap& srcPixmap,
    SkLinearBitmapPipeline::SampleStage* sampleStage) {
    const SkImageInfo& imageInfo = srcPixmap.info();
    switch (imageInfo.colorType()) {
        case kRGBA_8888_SkColorType:
            if (imageInfo.profileType() == kSRGB_SkColorProfileType) {
                sampleStage->Initialize<Sampler<Pixel8888SRGB>>(next, srcPixmap);
            } else {
                sampleStage->Initialize<Sampler<Pixel8888LRGB>>(next, srcPixmap);
            }
            break;
        case kBGRA_8888_SkColorType:
            if (imageInfo.profileType() == kSRGB_SkColorProfileType) {
                sampleStage->Initialize<Sampler<Pixel8888SBGR>>(next, srcPixmap);
            } else {
                sampleStage->Initialize<Sampler<Pixel8888LBGR>>(next, srcPixmap);
            }
            break;
        default:
            SkFAIL("Not implemented. Unsupported src");
            break;
    }
    return sampleStage->get();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Pixel Placement Stage
template <SkAlphaType alphaType>
class PlaceFPPixel final : public SkLinearBitmapPipeline::PixelPlacerInterface {
public:
    void VECTORCALL placePixel(Sk4f pixel) override {
        PlacePixel(fDst, pixel, 0);
        fDst += 1;
    }

    void VECTORCALL place4Pixels(Sk4f p0, Sk4f p1, Sk4f p2, Sk4f p3) override {
        SkPM4f* dst = fDst;
        PlacePixel(dst, p0, 0);
        PlacePixel(dst, p1, 1);
        PlacePixel(dst, p2, 2);
        PlacePixel(dst, p3, 3);
        fDst += 4;
    }

    void setDestination(SkPM4f* dst) override {
        fDst = dst;
    }

private:
    static void VECTORCALL PlacePixel(SkPM4f* dst, Sk4f pixel, int index) {
        Sk4f newPixel = pixel;
        if (alphaType == kUnpremul_SkAlphaType) {
            newPixel = Premultiply(pixel);
        }
        newPixel.store(dst + index);
    }
    static Sk4f VECTORCALL Premultiply(Sk4f pixel) {
        float alpha = pixel[3];
        return pixel * Sk4f{alpha, alpha, alpha, 1.0f};
    }

    SkPM4f* fDst;
};

static SkLinearBitmapPipeline::PixelPlacerInterface* choose_pixel_placer(
    SkAlphaType alphaType,
    SkLinearBitmapPipeline::PixelStage* placerStage) {
    if (alphaType == kUnpremul_SkAlphaType) {
        placerStage->Initialize<PlaceFPPixel<kUnpremul_SkAlphaType>>();
    } else {
        // kOpaque_SkAlphaType is treated the same as kPremul_SkAlphaType
        placerStage->Initialize<PlaceFPPixel<kPremul_SkAlphaType>>();
    }
    return placerStage->get();
}
}  // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////
SkLinearBitmapPipeline::~SkLinearBitmapPipeline() {}

SkLinearBitmapPipeline::SkLinearBitmapPipeline(
    const SkMatrix& inverse,
    SkFilterQuality filterQuality,
    SkShader::TileMode xTile, SkShader::TileMode yTile,
    const SkPixmap& srcPixmap) {
    SkSize size = SkSize::Make(srcPixmap.width(), srcPixmap.height());
    const SkImageInfo& srcImageInfo = srcPixmap.info();

    // As the stages are built, the chooser function may skip a stage. For example, with the
    // identity matrix, the matrix stage is skipped, and the tilerStage is the first stage.
    auto placementStage = choose_pixel_placer(srcImageInfo.alphaType(), &fPixelStage);
    auto samplerStage   = choose_pixel_sampler(placementStage, srcPixmap, &fSampleStage);
    auto tilerStage     = choose_tiler(samplerStage, size, xTile, yTile, &fTileXOrBothStage,
                                       &fTileYStage);
    auto filterStage    = choose_filter(tilerStage, filterQuality, &fFilterStage);
    fFirstStage         = choose_matrix(filterStage, inverse, &fMatrixStage);
}

void SkLinearBitmapPipeline::shadeSpan4f(int x, int y, SkPM4f* dst, int count) {
    SkASSERT(count > 0);
    fPixelStage->setDestination(dst);
    // The count and length arguments start out in a precise relation in order to keep the
    // math correct through the different stages. Count is the number of pixel to produce.
    // Since the code samples at pixel centers, length is the distance from the center of the
    // first pixel to the center of the last pixel. This implies that length is count-1.
    fFirstStage->pointSpan(Span{SkPoint{x + 0.5f, y + 0.5f}, count - 1.0f, count});
}
