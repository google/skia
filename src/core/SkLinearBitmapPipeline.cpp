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

// Tweak ABI of functions that pass Sk4f by value to pass them via registers.
#if defined(_MSC_VER) && SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
     #define VECTORCALL __vectorcall
 #elif defined(SK_CPU_ARM32) && defined(SK_ARM_HAS_NEON)
     #define VECTORCALL __attribute__((pcs("aapcs-vfp")))
 #else
     #define VECTORCALL
 #endif

class SkLinearBitmapPipeline::PointProcessorInterface {
public:
    virtual ~PointProcessorInterface() { }
    virtual void VECTORCALL pointListFew(int n, Sk4f xs, Sk4f ys) = 0;
    virtual void VECTORCALL pointList4(Sk4f xs, Sk4f ys) = 0;

    // The pointSpan method efficiently process horizontal spans of pixels.
    // * start - the point where to start the span.
    // * length - the number of pixels to traverse in source space.
    // * count - the number of pixels to produce in destination space.
    // Both start and length are mapped through the inversion matrix to produce values in source
    // space. After the matrix operation, the tilers may break the spans up into smaller spans.
    // The tilers can produce spans that seem nonsensical.
    // * The clamp tiler can create spans with length of 0. This indicates to copy an edge pixel out
    //   to the edge of the destination scan.
    // * The mirror tiler can produce spans with negative length. This indicates that the source
    //   should be traversed in the opposite direction to the destination pixels.
    virtual void pointSpan(SkPoint start, SkScalar length, int count) = 0;
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
    virtual void VECTORCALL bilerpList(Sk4f xs, Sk4f ys) = 0;
};

class SkLinearBitmapPipeline::PixelPlacerInterface {
public:
    virtual ~PixelPlacerInterface() { }
    virtual void setDestination(SkPM4f* dst) = 0;
    virtual void VECTORCALL placePixel(Sk4f pixel0) = 0;
    virtual void VECTORCALL place4Pixels(Sk4f p0, Sk4f p1, Sk4f p2, Sk4f p3) = 0;
};

namespace  {

struct X {
    explicit X(SkScalar val) : fVal{val} { }
    explicit X(SkPoint pt)   : fVal{pt.fX} { }
    explicit X(SkSize s)     : fVal{s.fWidth} { }
    explicit X(SkISize s)    : fVal(s.fWidth) { }
    operator float () const {return fVal;}
private:
    float fVal;
};

struct Y {
    explicit Y(SkScalar val) : fVal{val} { }
    explicit Y(SkPoint pt)   : fVal{pt.fY} { }
    explicit Y(SkSize s)     : fVal{s.fHeight} { }
    explicit Y(SkISize s)    : fVal(s.fHeight) { }
    operator float () const {return fVal;}
private:
    float fVal;
};

template <typename Stage>
void span_fallback(SkPoint start, SkScalar length, int count, Stage* stage) {
    // If count == 1 use PointListFew instead.
    SkASSERT(count > 1);

    float dx = length / (count - 1);
    Sk4f Xs = Sk4f(X(start)) + Sk4f{0.0f, 1.0f, 2.0f, 3.0f} * Sk4f{dx};
    Sk4f Ys{Y(start)};
    Sk4f fourDx = {4.0f * dx};

    while (count >= 4) {
        stage->pointList4(Xs, Ys);
        Xs = Xs + fourDx;
        count -= 4;
    }
    if (count > 0) {
        stage->pointListFew(count, Xs, Ys);
    }
}

// PointProcessor uses a strategy to help complete the work of the different stages. The strategy
// must implement the following methods:
// * processPoints(xs, ys) - must mutate the xs and ys for the stage.
// * maybeProcessSpan(start, length, count) - This represents a horizontal series of pixels
//   to work over.
//   start - is the starting pixel. This is in destination space before the matrix stage, and in
//           source space after the matrix stage.
//   length - is this distance between the first pixel center and the last pixel center. Like start,
//           this is in destination space before the matrix stage, and in source space after.
//   count - the number of pixels in source space to produce.
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

    void VECTORCALL pointListFew(int n, Sk4f xs, Sk4f ys) override {
        fStrategy.processPoints(&xs, &ys);
        fNext->pointListFew(n, xs, ys);
    }

    void VECTORCALL pointList4(Sk4f xs, Sk4f ys) override {
        fStrategy.processPoints(&xs, &ys);
        fNext->pointList4(xs, ys);
    }

    void pointSpan(SkPoint start, SkScalar length, int count) override {
        if (!fStrategy.maybeProcessSpan(start, length, count, fNext)) {
            span_fallback(start, length, count, this);
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

    void VECTORCALL pointListFew(int n, Sk4f xs, Sk4f ys) override {
        fStrategy.processPoints(&xs, &ys);
        fNext->pointListFew(n, xs, ys);
    }

    void VECTORCALL pointList4(Sk4f xs, Sk4f ys) override {
        fStrategy.processPoints(&xs, &ys);
        fNext->pointList4(xs, ys);
    }

    void VECTORCALL bilerpList(Sk4f xs, Sk4f ys) override {
        fStrategy.processPoints(&xs, &ys);
        fNext->bilerpList(xs, ys);
    }

    void pointSpan(SkPoint start, SkScalar length, int count) override {
        if (!fStrategy.maybeProcessSpan(start, length, count, fNext)) {
            span_fallback(start, length, count, this);
        }
    }

private:
    Next* const fNext;
    Strategy fStrategy;
};

class SkippedStage final : public SkLinearBitmapPipeline::BilerpProcessorInterface {
    void VECTORCALL pointListFew(int n, Sk4f xs, Sk4f ys) override {
        SkFAIL("Skipped stage.");
    }
    void VECTORCALL pointList4(Sk4f xs, Sk4f ys) override {
        SkFAIL("Skipped stage.");
    }
    void VECTORCALL bilerpList(Sk4f xs, Sk4f ys) override {
        SkFAIL("Skipped stage.");
    }
    void pointSpan(SkPoint start, SkScalar length, int count) override {
        SkFAIL("Skipped stage.");
    }
};

class TranslateMatrixStrategy {
public:
    TranslateMatrixStrategy(SkVector offset)
        : fXOffset{X(offset)}
        , fYOffset{Y(offset)} { }

    void processPoints(Sk4f* xs, Sk4f* ys) {
        *xs = *xs + fXOffset;
        *ys = *ys + fYOffset;
    }

    template <typename Next>
    bool maybeProcessSpan(SkPoint start, SkScalar length, int count, Next* next) {
        next->pointSpan(start + SkPoint{fXOffset[0], fYOffset[0]}, length, count);
        return true;
    }

private:
    const Sk4f fXOffset, fYOffset;
};
template <typename Next = SkLinearBitmapPipeline::PointProcessorInterface>
using TranslateMatrix = PointProcessor<TranslateMatrixStrategy, Next>;

class ScaleMatrixStrategy {
public:
    ScaleMatrixStrategy(SkVector offset, SkVector scale)
        : fXOffset{X(offset)}, fYOffset{Y(offset)}
        ,  fXScale{X(scale)},   fYScale{Y(scale)} { }
    void processPoints(Sk4f* xs, Sk4f* ys) {
        *xs = *xs * fXScale + fXOffset;
        *ys = *ys * fYScale + fYOffset;
    }

    template <typename Next>
    bool maybeProcessSpan(SkPoint start, SkScalar length, int count, Next* next) {
        SkPoint newStart =
            SkPoint{X(start) * fXScale[0] + fXOffset[0], Y(start) * fYScale[0] + fYOffset[0]};
        SkScalar newLength = length * fXScale[0];
        next->pointSpan(newStart, newLength, count);
        return true;
    }

private:
    const Sk4f fXOffset, fYOffset;
    const Sk4f fXScale, fYScale;
};
template <typename Next = SkLinearBitmapPipeline::PointProcessorInterface>
using ScaleMatrix = PointProcessor<ScaleMatrixStrategy, Next>;

class AffineMatrixStrategy {
public:
    AffineMatrixStrategy(SkVector offset, SkVector scale, SkVector skew)
        : fXOffset{X(offset)}, fYOffset{Y(offset)}
        , fXScale{X(scale)},   fYScale{Y(scale)}
        , fXSkew{X(skew)},     fYSkew{Y(skew)} { }
    void processPoints(Sk4f* xs, Sk4f* ys) {
        Sk4f newXs = fXScale * *xs +  fXSkew * *ys + fXOffset;
        Sk4f newYs =  fYSkew * *xs + fYScale * *ys + fYOffset;

        *xs = newXs;
        *ys = newYs;
    }

    template <typename Next>
    bool maybeProcessSpan(SkPoint start, SkScalar length, int count, Next* next) {
        return false;
    }

private:
    const Sk4f fXOffset, fYOffset;
    const Sk4f fXScale,  fYScale;
    const Sk4f fXSkew,   fYSkew;
};
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
        matrixProc->Initialize<SkippedStage>();
        return next;
    }
    return matrixProc->get();
}

template <typename Next = SkLinearBitmapPipeline::BilerpProcessorInterface>
class ExpandBilerp final : public SkLinearBitmapPipeline::PointProcessorInterface {
public:
    ExpandBilerp(Next* next) : fNext{next} { }

    void VECTORCALL pointListFew(int n, Sk4f xs, Sk4f ys) override {
        SkASSERT(0 < n && n < 4);
        //                    px00   px10   px01  px11
        const Sk4f kXOffsets{-0.5f,  0.5f, -0.5f, 0.5f},
                   kYOffsets{-0.5f, -0.5f,  0.5f, 0.5f};
        if (n >= 1) fNext->bilerpList(Sk4f{xs[0]} + kXOffsets, Sk4f{ys[0]} + kYOffsets);
        if (n >= 2) fNext->bilerpList(Sk4f{xs[1]} + kXOffsets, Sk4f{ys[1]} + kYOffsets);
        if (n >= 3) fNext->bilerpList(Sk4f{xs[2]} + kXOffsets, Sk4f{ys[2]} + kYOffsets);
    }

    void VECTORCALL pointList4(Sk4f xs, Sk4f ys) override {
        //                    px00   px10   px01  px11
        const Sk4f kXOffsets{-0.5f,  0.5f, -0.5f, 0.5f},
                   kYOffsets{-0.5f, -0.5f,  0.5f, 0.5f};
        fNext->bilerpList(Sk4f{xs[0]} + kXOffsets, Sk4f{ys[0]} + kYOffsets);
        fNext->bilerpList(Sk4f{xs[1]} + kXOffsets, Sk4f{ys[1]} + kYOffsets);
        fNext->bilerpList(Sk4f{xs[2]} + kXOffsets, Sk4f{ys[2]} + kYOffsets);
        fNext->bilerpList(Sk4f{xs[3]} + kXOffsets, Sk4f{ys[3]} + kYOffsets);
    }

    void pointSpan(SkPoint start, SkScalar length, int count) override {
        span_fallback(start, length, count, this);
    }

private:
    Next* const fNext;
};

static SkLinearBitmapPipeline::PointProcessorInterface* choose_filter(
    SkLinearBitmapPipeline::BilerpProcessorInterface* next,
    SkFilterQuality filterQuailty,
    SkLinearBitmapPipeline::FilterStage* filterProc) {
    if (SkFilterQuality::kNone_SkFilterQuality == filterQuailty) {
        filterProc->Initialize<SkippedStage>();
        return next;
    } else {
        filterProc->Initialize<ExpandBilerp<>>(next);
        return filterProc->get();
    }
}

class ClampStrategy {
public:
    ClampStrategy(X max)
        : fXMin{0.0f}
        , fXMax{max - 1.0f} { }
    ClampStrategy(Y max)
        : fYMin{0.0f}
        , fYMax{max - 1.0f} { }
    ClampStrategy(SkSize max)
        : fXMin{0.0f}
        , fYMin{0.0f}
        , fXMax{X(max) - 1.0f}
        , fYMax{Y(max) - 1.0f} { }

    void processPoints(Sk4f* xs, Sk4f* ys) {
        *xs = Sk4f::Min(Sk4f::Max(*xs, fXMin), fXMax);
        *ys = Sk4f::Min(Sk4f::Max(*ys, fYMin), fYMax);
    }

    template <typename Next>
    bool maybeProcessSpan(SkPoint start, SkScalar length, int count, Next* next) {
        return false;
    }

private:
    const Sk4f fXMin{SK_FloatNegativeInfinity};
    const Sk4f fYMin{SK_FloatNegativeInfinity};
    const Sk4f fXMax{SK_FloatInfinity};
    const Sk4f fYMax{SK_FloatInfinity};
};
template <typename Next = SkLinearBitmapPipeline::BilerpProcessorInterface>
using Clamp = BilerpProcessor<ClampStrategy, Next>;

class RepeatStrategy {
public:
    RepeatStrategy(X max) : fXMax{max}, fXInvMax{1.0f/max} { }
    RepeatStrategy(Y max) : fYMax{max}, fYInvMax{1.0f/max} { }
    RepeatStrategy(SkSize max)
        : fXMax{X(max)}
        , fXInvMax{1.0f / X(max)}
        , fYMax{Y(max)}
        , fYInvMax{1.0f / Y(max)} { }

    void processPoints(Sk4f* xs, Sk4f* ys) {
        Sk4f divX = (*xs * fXInvMax).floor();
        Sk4f divY = (*ys * fYInvMax).floor();
        Sk4f baseX = (divX * fXMax);
        Sk4f baseY = (divY * fYMax);
        *xs = *xs - baseX;
        *ys = *ys - baseY;
    }

    template <typename Next>
    bool maybeProcessSpan(SkPoint start, SkScalar length, int count, Next* next) {
        return false;
    }

private:
    const Sk4f fXMax{0.0f};
    const Sk4f fXInvMax{0.0f};
    const Sk4f fYMax{0.0f};
    const Sk4f fYInvMax{0.0f};
};

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
        tileProcY->Initialize<SkippedStage>();
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

class sRGBFast {
public:
    static Sk4f VECTORCALL sRGBToLinear(Sk4f pixel) {
        Sk4f l = pixel * pixel;
        return Sk4f{l[0], l[1], l[2], pixel[3]};
    }
};

template <SkColorProfileType colorProfile>
class Passthrough8888 {
public:
    Passthrough8888(int width, const uint32_t* src)
        : fSrc{src}, fWidth{width}{ }

    void VECTORCALL getFewPixels(int n, Sk4f xs, Sk4f ys, Sk4f* px0, Sk4f* px1, Sk4f* px2) {
        Sk4i XIs = SkNx_cast<int, float>(xs);
        Sk4i YIs = SkNx_cast<int, float>(ys);
        Sk4i bufferLoc = YIs * fWidth + XIs;
        switch (n) {
            case 3:
                *px2 = getPixel(fSrc, bufferLoc[2]);
            case 2:
                *px1 = getPixel(fSrc, bufferLoc[1]);
            case 1:
                *px0 = getPixel(fSrc, bufferLoc[0]);
            default:
                break;
        }
    }

    void VECTORCALL get4Pixels(Sk4f xs, Sk4f ys, Sk4f* px0, Sk4f* px1, Sk4f* px2, Sk4f* px3) {
        Sk4i XIs = SkNx_cast<int, float>(xs);
        Sk4i YIs = SkNx_cast<int, float>(ys);
        Sk4i bufferLoc = YIs * fWidth + XIs;
        *px0 = getPixel(fSrc, bufferLoc[0]);
        *px1 = getPixel(fSrc, bufferLoc[1]);
        *px2 = getPixel(fSrc, bufferLoc[2]);
        *px3 = getPixel(fSrc, bufferLoc[3]);
    }

    const uint32_t* row(int y) { return fSrc + y * fWidth[0]; }

private:
    Sk4f getPixel(const uint32_t* src, int index) {
        Sk4b bytePixel = Sk4b::Load((uint8_t *)(&src[index]));
        Sk4f pixel = SkNx_cast<float, uint8_t>(bytePixel);
        pixel = pixel * Sk4f{1.0f/255.0f};
        if (colorProfile == kSRGB_SkColorProfileType) {
            pixel = sRGBFast::sRGBToLinear(pixel);
        }
        return pixel;
    }
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
static Sk4f VECTORCALL bilerp4(Sk4f xs, Sk4f ys, Sk4f px00, Sk4f px10,
                                                 Sk4f px01, Sk4f px11) {
    // Calculate fractional xs and ys.
    Sk4f fxs = xs - xs.floor();
    Sk4f fys = ys - ys.floor();
    Sk4f fxys{fxs * fys};
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

    void VECTORCALL pointListFew(int n, Sk4f xs, Sk4f ys) override {
        SkASSERT(0 < n && n < 4);
        Sk4f px0, px1, px2;
        fStrategy.getFewPixels(n, xs, ys, &px0, &px1, &px2);
        if (n >= 1) fNext->placePixel(px0);
        if (n >= 2) fNext->placePixel(px1);
        if (n >= 3) fNext->placePixel(px2);
    }

    void VECTORCALL pointList4(Sk4f xs, Sk4f ys) override {
        Sk4f px0, px1, px2, px3;
        fStrategy.get4Pixels(xs, ys, &px0, &px1, &px2, &px3);
        fNext->place4Pixels(px0, px1, px2, px3);
    }

    void VECTORCALL bilerpList(Sk4f xs, Sk4f ys) override {
        Sk4f px00, px10, px01, px11;
        fStrategy.get4Pixels(xs, ys, &px00, &px10, &px01, &px11);
        Sk4f pixel = bilerp4(xs, ys, px00, px10, px01, px11);
        fNext->placePixel(pixel);
    }

    void pointSpan(SkPoint start, SkScalar length, int count) override {
        span_fallback(start, length, count, this);
    }

private:
    SkLinearBitmapPipeline::PixelPlacerInterface* const fNext;
    SourceStrategy fStrategy;
};

static SkLinearBitmapPipeline::BilerpProcessorInterface* choose_pixel_sampler(
    SkLinearBitmapPipeline::PixelPlacerInterface* next,
    const SkPixmap& srcPixmap,
    SkLinearBitmapPipeline::SampleStage* sampleStage) {
    const SkImageInfo& imageInfo = srcPixmap.info();
    switch (imageInfo.colorType()) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            if (kN32_SkColorType == imageInfo.colorType()) {
                if (imageInfo.profileType() == kSRGB_SkColorProfileType) {
                    sampleStage->Initialize<Sampler<Passthrough8888<kSRGB_SkColorProfileType>>>(
                        next, static_cast<int>(srcPixmap.rowBytes() / 4),
                        srcPixmap.addr32());
                } else {
                    sampleStage->Initialize<Sampler<Passthrough8888<kLinear_SkColorProfileType>>>(
                        next, static_cast<int>(srcPixmap.rowBytes() / 4),
                        srcPixmap.addr32());
                }
            } else {
                SkFAIL("Not implemented. No 8888 Swizzle");
            }
            break;
        default:
            SkFAIL("Not implemented. Unsupported src");
            break;
    }
    return sampleStage->get();
}

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
    // Adjust points by 0.5, 0.5 to sample from the center of the pixels.
    if (count == 1) {
        fFirstStage->pointListFew(1, Sk4f{x + 0.5f}, Sk4f{y + 0.5f});
    } else {
        // The count and length arguments start out in a precise relation in order to keep the
        // math correct through the different stages. Count is the number of pixel to produce.
        // Since the code samples at pixel centers, length is the distance from the center of the
        // first pixel to the center of the last pixel. This implies that length is count-1.
        fFirstStage->pointSpan(SkPoint{x + 0.5f, y + 0.5f}, count - 1, count);
    }
}
