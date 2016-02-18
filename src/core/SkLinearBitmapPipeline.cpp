/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLinearBitmapPipeline.h"

struct X {
    explicit X(SkScalar val) : fVal{val} { }
    explicit X(SkPoint pt) : fVal{pt.fX} { }
    explicit X(SkSize s) : fVal{s.fWidth} { }
    explicit X(SkISize s) : fVal(s.fWidth) { }
    operator float () const {return fVal;}
private:
    float fVal;
};

struct Y {
    explicit Y(SkScalar val) : fVal{val} { }
    explicit Y(SkPoint pt) : fVal{pt.fY} { }
    explicit Y(SkSize s) : fVal{s.fHeight} { }
    explicit Y(SkISize s) : fVal(s.fHeight) { }
    operator float () const {return fVal;}
private:
    float fVal;
};

template<typename Strategy, typename Next>
class PointProcessor final : public PointProcessorInterface {
public:
    template <typename... Args>
    PointProcessor(Next* next, Args&&... args)
        : fNext{next}
        , fStrategy{std::forward<Args>(args)...}{ }

    void pointListFew(int n, Sk4fArg xs, Sk4fArg ys) override {
        Sk4f newXs = xs;
        Sk4f newYs = ys;
        fStrategy.processPoints(&newXs, &newYs);
        fNext->pointListFew(n, newXs, newYs);
    }

    void pointList4(Sk4fArg xs, Sk4fArg ys) override {
        Sk4f newXs = xs;
        Sk4f newYs = ys;
        fStrategy.processPoints(&newXs, &newYs);
        fNext->pointList4(newXs, newYs);
    }

private:
    Next* const fNext;
    Strategy fStrategy;
};

template<typename Strategy, typename Next>
class BilerpProcessor final : public BilerpProcessorInterface  {
public:
    template <typename... Args>
    BilerpProcessor(Next* next, Args&&... args)
        : fNext{next}
        , fStrategy{std::forward<Args>(args)...}{ }

    void pointListFew(int n, Sk4fArg xs, Sk4fArg ys) override {
        Sk4f newXs = xs;
        Sk4f newYs = ys;
        fStrategy.processPoints(&newXs, &newYs);
        fNext->pointListFew(n, newXs, newYs);
    }

    void pointList4(Sk4fArg xs, Sk4fArg ys) override {
        Sk4f newXs = xs;
        Sk4f newYs = ys;
        fStrategy.processPoints(&newXs, &newYs);
        fNext->pointList4(newXs, newYs);
    }

    void bilerpList(Sk4fArg xs, Sk4fArg ys) override {
        Sk4f newXs = xs;
        Sk4f newYs = ys;
        fStrategy.processPoints(&newXs, &newYs);
        fNext->bilerpList(newXs, newYs);
    }

private:
    Next* const fNext;
    Strategy fStrategy;
};

class SkippedStage final : public BilerpProcessorInterface {
    void pointListFew(int n, Sk4fArg xs, Sk4fArg ys) override {
        SkFAIL("Skipped stage.");
    }
    void pointList4(Sk4fArg Xs, Sk4fArg Ys) override {
        SkFAIL("Skipped stage.");
    }
    virtual void bilerpList(Sk4fArg xs, Sk4fArg ys) override {
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

private:
    const Sk4f fXOffset, fYOffset;
};
template <typename Next = PointProcessorInterface>
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

private:
    const Sk4f fXOffset, fYOffset;
    const Sk4f fXScale, fYScale;
};
template <typename Next = PointProcessorInterface>
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

private:
    const Sk4f fXOffset, fYOffset;
    const Sk4f fXScale,  fYScale;
    const Sk4f fXSkew,   fYSkew;
};
template <typename Next = PointProcessorInterface>
using AffineMatrix = PointProcessor<AffineMatrixStrategy, Next>;

static PointProcessorInterface* choose_matrix(
    PointProcessorInterface* next,
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

template <typename Next = BilerpProcessorInterface>
class ExpandBilerp final : public PointProcessorInterface {
public:
    ExpandBilerp(Next* next) : fNext{next} { }

    void pointListFew(int n, Sk4fArg xs, Sk4fArg ys) override {
        SkASSERT(0 < n && n < 4);
        //                   px00  px10  px01  px11
        const Sk4f kXOffsets{0.0f, 1.0f, 0.0f, 1.0f},
                   kYOffsets{0.0f, 0.0f, 1.0f, 1.0f};
        if (n >= 1) fNext->bilerpList(Sk4f{xs[0]} + kXOffsets, Sk4f{ys[0]} + kYOffsets);
        if (n >= 2) fNext->bilerpList(Sk4f{xs[1]} + kXOffsets, Sk4f{ys[1]} + kYOffsets);
        if (n >= 3) fNext->bilerpList(Sk4f{xs[2]} + kXOffsets, Sk4f{ys[2]} + kYOffsets);
    }

    void pointList4(Sk4fArg xs, Sk4fArg ys) override {
        //                   px00  px10  px01  px11
        const Sk4f kXOffsets{0.0f, 1.0f, 0.0f, 1.0f},
                   kYOffsets{0.0f, 0.0f, 1.0f, 1.0f};
        fNext->bilerpList(Sk4f{xs[0]} + kXOffsets, Sk4f{ys[0]} + kYOffsets);
        fNext->bilerpList(Sk4f{xs[1]} + kXOffsets, Sk4f{ys[1]} + kYOffsets);
        fNext->bilerpList(Sk4f{xs[2]} + kXOffsets, Sk4f{ys[2]} + kYOffsets);
        fNext->bilerpList(Sk4f{xs[3]} + kXOffsets, Sk4f{ys[3]} + kYOffsets);
    }

private:
    Next* const fNext;
};

static PointProcessorInterface* choose_filter(
    BilerpProcessorInterface* next,
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

private:
    const Sk4f fXMin{SK_FloatNegativeInfinity};
    const Sk4f fYMin{SK_FloatNegativeInfinity};
    const Sk4f fXMax{SK_FloatInfinity};
    const Sk4f fYMax{SK_FloatInfinity};
};
template <typename Next = BilerpProcessorInterface>
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

private:
    const Sk4f fXMax{0.0f};
    const Sk4f fXInvMax{0.0f};
    const Sk4f fYMax{0.0f};
    const Sk4f fYInvMax{0.0f};
};

template <typename Next = BilerpProcessorInterface>
using Repeat = BilerpProcessor<RepeatStrategy, Next>;

static BilerpProcessorInterface* choose_tiler(
    BilerpProcessorInterface* next,
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
    static Sk4f sRGBToLinear(Sk4fArg pixel) {
        Sk4f l = pixel * pixel;
        return Sk4f{l[0], l[1], l[2], pixel[3]};
    }
};

template <SkColorProfileType colorProfile>
class Passthrough8888 {
public:
    Passthrough8888(int width, const uint32_t* src)
        : fSrc{src}, fWidth{width}{ }

    void getFewPixels(int n, Sk4fArg xs, Sk4fArg ys, Sk4f* px0, Sk4f* px1, Sk4f* px2) {
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

    void get4Pixels(Sk4fArg xs, Sk4fArg ys, Sk4f* px0, Sk4f* px1, Sk4f* px2, Sk4f* px3) {
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
static Sk4f bilerp4(Sk4fArg xs, Sk4fArg ys, Sk4fArg px00, Sk4fArg px10,
                                            Sk4fArg px01, Sk4fArg px11) {
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
class Sampler final : public BilerpProcessorInterface {
public:
    template <typename... Args>
    Sampler(PixelPlacerInterface* next, Args&&... args)
        : fNext{next}
        , fStrategy{std::forward<Args>(args)...} { }

    void pointListFew(int n, Sk4fArg xs, Sk4fArg ys) override {
        SkASSERT(0 < n && n < 4);
        Sk4f px0, px1, px2;
        fStrategy.getFewPixels(n, xs, ys, &px0, &px1, &px2);
        if (n >= 1) fNext->placePixel(px0);
        if (n >= 2) fNext->placePixel(px1);
        if (n >= 3) fNext->placePixel(px2);
    }

    void pointList4(Sk4fArg xs, Sk4fArg ys) override {
        Sk4f px0, px1, px2, px3;
        fStrategy.get4Pixels(xs, ys, &px0, &px1, &px2, &px3);
        fNext->place4Pixels(px0, px1, px2, px3);
    }

    void bilerpList(Sk4fArg xs, Sk4fArg ys) override {
        Sk4f px00, px10, px01, px11;
        fStrategy.get4Pixels(xs, ys, &px00, &px10, &px01, &px11);
        Sk4f pixel = bilerp4(xs, ys, px00, px10, px01, px11);
        fNext->placePixel(pixel);
    }

private:
    PixelPlacerInterface* const fNext;
    SourceStrategy fStrategy;
};

static BilerpProcessorInterface* choose_pixel_sampler(
    PixelPlacerInterface* next,
    const SkImageInfo& imageInfo,
    const void* imageData,
    SkLinearBitmapPipeline::SampleStage* sampleStage) {
    switch (imageInfo.colorType()) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            if (kN32_SkColorType == imageInfo.colorType()) {
                if (imageInfo.profileType() == kSRGB_SkColorProfileType) {
                    sampleStage->Initialize<Sampler<Passthrough8888<kSRGB_SkColorProfileType>>>(
                        next, imageInfo.width(),
                        (uint32_t*)imageData);
                } else {
                    sampleStage->Initialize<Sampler<Passthrough8888<kLinear_SkColorProfileType>>>(
                        next, imageInfo.width(),
                        (uint32_t*)imageData);
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
class PlaceFPPixel final : public PixelPlacerInterface {
public:
    void placePixel(Sk4fArg pixel) override {
        PlacePixel(fDst, pixel, 0);
        fDst += 1;
    }

    void place4Pixels(Sk4fArg p0, Sk4fArg p1, Sk4fArg p2, Sk4fArg p3) override {
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
    static void PlacePixel(SkPM4f* dst, Sk4fArg pixel, int index) {
        Sk4f newPixel = pixel;
        if (alphaType == kUnpremul_SkAlphaType) {
            newPixel = Premultiply(pixel);
        }
        newPixel.store(dst + index);
    }
    static Sk4f Premultiply(Sk4fArg pixel) {
        float alpha = pixel[3];
        return pixel * Sk4f{alpha, alpha, alpha, 1.0f};
    }

    SkPM4f* fDst;
};

static PixelPlacerInterface* choose_pixel_placer(
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

SkLinearBitmapPipeline::SkLinearBitmapPipeline(
    const SkMatrix& inverse,
    SkFilterQuality filterQuality,
    SkShader::TileMode xTile, SkShader::TileMode yTile,
    const SkImageInfo& srcImageInfo,
    const void* srcImageData) {
    SkSize size;
    size = srcImageInfo.dimensions();

    // As the stages are built, the chooser function may skip a stage. For example, with the
    // identity matrix, the matrix stage is skipped, and the tilerStage is the first stage.
    auto placementStage = choose_pixel_placer(srcImageInfo.alphaType(), &fPixelStage);
    auto samplerStage   = choose_pixel_sampler(placementStage, srcImageInfo,
                                               srcImageData, &fSampleStage);
    auto tilerStage     = choose_tiler(samplerStage, size, xTile, yTile, &fTileXOrBothStage,
                                       &fTileYStage);
    auto filterStage    = choose_filter(tilerStage, filterQuality, &fFilterStage);
    fFirstStage         = choose_matrix(filterStage, inverse, &fMatrixStage);
}

void SkLinearBitmapPipeline::shadeSpan4f(int x, int y, SkPM4f* dst, int count) {
    fPixelStage->setDestination(dst);

    Sk4f Xs = Sk4f(x) + Sk4f{0.5f, 1.5f, 2.5f, 3.5f};
    Sk4f Ys(y);
    Sk4f fours{4.0f};

    while (count >= 4) {
        fFirstStage->pointList4(Xs, Ys);
        Xs = Xs + fours;
        count -= 4;
    }
    if (count > 0) {
        fFirstStage->pointListFew(count, Xs, Ys);
    }
}
