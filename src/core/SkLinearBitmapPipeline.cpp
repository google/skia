/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkLinearBitmapPipeline.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <tuple>

#include "SkLinearBitmapPipeline_core.h"
#include "SkLinearBitmapPipeline_matrix.h"
#include "SkLinearBitmapPipeline_tile.h"
#include "SkLinearBitmapPipeline_sample.h"
#include "SkNx.h"
#include "SkOpts.h"
#include "SkPM4f.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkLinearBitmapPipeline::Stage
template<typename Base, size_t kSize, typename Next>
SkLinearBitmapPipeline::Stage<Base, kSize, Next>::~Stage() {
    if (fIsInitialized) {
        this->get()->~Base();
    }
}

template<typename Base, size_t kSize, typename Next>
template<typename Variant, typename... Args>
void SkLinearBitmapPipeline::Stage<Base, kSize, Next>::initStage(Next* next, Args&& ... args) {
    SkASSERTF(sizeof(Variant) <= sizeof(fSpace),
              "Size Variant: %d, Space: %d", sizeof(Variant), sizeof(fSpace));

    new (&fSpace) Variant(next, std::forward<Args>(args)...);
    fStageCloner = [this](Next* nextClone, void* addr) {
        new (addr) Variant(nextClone, (const Variant&)*this->get());
    };
    fIsInitialized = true;
};

template<typename Base, size_t kSize, typename Next>
template<typename Variant, typename... Args>
void SkLinearBitmapPipeline::Stage<Base, kSize, Next>::initSink(Args&& ... args) {
    SkASSERTF(sizeof(Variant) <= sizeof(fSpace),
              "Size Variant: %d, Space: %d", sizeof(Variant), sizeof(fSpace));
    new (&fSpace) Variant(std::forward<Args>(args)...);
    fIsInitialized = true;
};

template<typename Base, size_t kSize, typename Next>
template <typename To, typename From>
To* SkLinearBitmapPipeline::Stage<Base, kSize, Next>::getInterface() {
    From* down = static_cast<From*>(this->get());
    return static_cast<To*>(down);
}

template<typename Base, size_t kSize, typename Next>
Base* SkLinearBitmapPipeline::Stage<Base, kSize, Next>::cloneStageTo(
    Next* next, Stage* cloneToStage) const
{
    if (!fIsInitialized) return nullptr;
    fStageCloner(next, &cloneToStage->fSpace);
    return cloneToStage->get();
}

namespace  {

////////////////////////////////////////////////////////////////////////////////////////////////////
// Matrix Stage
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
class MatrixStage final : public SkLinearBitmapPipeline::PointProcessorInterface {
public:
    template <typename... Args>
    MatrixStage(Next* next, Args&&... args)
        : fNext{next}
        , fStrategy{std::forward<Args>(args)...}{ }

    MatrixStage(Next* next, const MatrixStage& stage)
        : fNext{next}
        , fStrategy{stage.fStrategy} { }

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

template <typename Next = SkLinearBitmapPipeline::PointProcessorInterface>
using TranslateMatrix = MatrixStage<TranslateMatrixStrategy, Next>;

template <typename Next = SkLinearBitmapPipeline::PointProcessorInterface>
using ScaleMatrix = MatrixStage<ScaleMatrixStrategy, Next>;

template <typename Next = SkLinearBitmapPipeline::PointProcessorInterface>
using AffineMatrix = MatrixStage<AffineMatrixStrategy, Next>;

template <typename Next = SkLinearBitmapPipeline::PointProcessorInterface>
using PerspectiveMatrix = MatrixStage<PerspectiveMatrixStrategy, Next>;


static SkLinearBitmapPipeline::PointProcessorInterface* choose_matrix(
    SkLinearBitmapPipeline::PointProcessorInterface* next,
    const SkMatrix& inverse,
    SkLinearBitmapPipeline::MatrixStage* matrixProc) {
    if (inverse.hasPerspective()) {
        matrixProc->initStage<PerspectiveMatrix<>>(
            next,
            SkVector{inverse.getTranslateX(), inverse.getTranslateY()},
            SkVector{inverse.getScaleX(), inverse.getScaleY()},
            SkVector{inverse.getSkewX(), inverse.getSkewY()},
            SkVector{inverse.getPerspX(), inverse.getPerspY()},
            inverse.get(SkMatrix::kMPersp2));
    } else if (inverse.getSkewX() != 0.0f || inverse.getSkewY() != 0.0f) {
        matrixProc->initStage<AffineMatrix<>>(
            next,
            SkVector{inverse.getTranslateX(), inverse.getTranslateY()},
            SkVector{inverse.getScaleX(), inverse.getScaleY()},
            SkVector{inverse.getSkewX(), inverse.getSkewY()});
    } else if (inverse.getScaleX() != 1.0f || inverse.getScaleY() != 1.0f) {
        matrixProc->initStage<ScaleMatrix<>>(
            next,
            SkVector{inverse.getTranslateX(), inverse.getTranslateY()},
            SkVector{inverse.getScaleX(), inverse.getScaleY()});
    } else if (inverse.getTranslateX() != 0.0f || inverse.getTranslateY() != 0.0f) {
        matrixProc->initStage<TranslateMatrix<>>(
            next,
            SkVector{inverse.getTranslateX(), inverse.getTranslateY()});
    } else {
        return next;
    }
    return matrixProc->get();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Tile Stage

template<typename XStrategy, typename YStrategy, typename Next>
class NearestTileStage final : public SkLinearBitmapPipeline::PointProcessorInterface {
public:
    template <typename... Args>
    NearestTileStage(Next* next, SkISize dimensions)
        : fNext{next}
        , fXStrategy{dimensions.width()}
        , fYStrategy{dimensions.height()}{ }

    NearestTileStage(Next* next, const NearestTileStage& stage)
        : fNext{next}
        , fXStrategy{stage.fXStrategy}
        , fYStrategy{stage.fYStrategy} { }

    void VECTORCALL pointListFew(int n, Sk4s xs, Sk4s ys) override {
        fXStrategy.tileXPoints(&xs);
        fYStrategy.tileYPoints(&ys);
        fNext->pointListFew(n, xs, ys);
    }

    void VECTORCALL pointList4(Sk4s xs, Sk4s ys) override {
        fXStrategy.tileXPoints(&xs);
        fYStrategy.tileYPoints(&ys);
        fNext->pointList4(xs, ys);
    }

    // The span you pass must not be empty.
    void pointSpan(Span span) override {
        SkASSERT(!span.isEmpty());
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = span;
        SkScalar x = X(start);
        SkScalar y = fYStrategy.tileY(Y(start));
        Span yAdjustedSpan{{x, y}, length, count};
        if (!fXStrategy.maybeProcessSpan(yAdjustedSpan, fNext)) {
            span_fallback(span, this);
        }
    }

private:
    Next* const fNext;
    XStrategy fXStrategy;
    YStrategy fYStrategy;
};

template<typename XStrategy, typename YStrategy, typename Next>
class BilerpTileStage final : public SkLinearBitmapPipeline::PointProcessorInterface {
public:
    template <typename... Args>
    BilerpTileStage(Next* next, SkISize dimensions)
        : fNext{next}
        , fXMax(dimensions.width())
        , fYMax(dimensions.height())
        , fXStrategy{dimensions.width()}
        , fYStrategy{dimensions.height()} { }

    BilerpTileStage(Next* next, const BilerpTileStage& stage)
        : fNext{next}
        , fXMax{stage.fXMax}
        , fYMax{stage.fYMax}
        , fXStrategy{stage.fXStrategy}
        , fYStrategy{stage.fYStrategy} { }

    void VECTORCALL pointListFew(int n, Sk4s xs, Sk4s ys) override {
        fXStrategy.tileXPoints(&xs);
        fYStrategy.tileYPoints(&ys);
        // TODO: check to see if xs and ys are in range then just call pointListFew on next.
        if (n >= 1) this->bilerpPoint(xs[0], ys[0]);
        if (n >= 2) this->bilerpPoint(xs[1], ys[1]);
        if (n >= 3) this->bilerpPoint(xs[2], ys[2]);
    }

    void VECTORCALL pointList4(Sk4s xs, Sk4s ys) override {
        fXStrategy.tileXPoints(&xs);
        fYStrategy.tileYPoints(&ys);
        // TODO: check to see if xs and ys are in range then just call pointList4 on next.
        this->bilerpPoint(xs[0], ys[0]);
        this->bilerpPoint(xs[1], ys[1]);
        this->bilerpPoint(xs[2], ys[2]);
        this->bilerpPoint(xs[3], ys[3]);
    }

    struct Wrapper {
        void pointSpan(Span span) {
            processor->breakIntoEdges(span);
        }

        void repeatSpan(Span span, int32_t repeatCount) {
            while (repeatCount --> 0) {
                processor->pointSpan(span);
            }
        }

        BilerpTileStage* processor;
    };

    // The span you pass must not be empty.
    void pointSpan(Span span) override {
        SkASSERT(!span.isEmpty());

        Wrapper wrapper = {this};
        if (!fXStrategy.maybeProcessSpan(span, &wrapper)) {
            span_fallback(span, this);
        }
    }

private:
    void bilerpPoint(SkScalar x, SkScalar y) {
        Sk4f txs = Sk4f{x} + Sk4f{-0.5f, 0.5f, -0.5f, 0.5f};
        Sk4f tys = Sk4f{y} + Sk4f{-0.5f, -0.5f, 0.5f, 0.5f};
        fXStrategy.tileXPoints(&txs);
        fYStrategy.tileYPoints(&tys);
        fNext->bilerpEdge(txs, tys);
    }

    void handleEdges(Span span, SkScalar dx) {
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = span;
        SkScalar x = X(start);
        SkScalar y = Y(start);
        SkScalar tiledY = fYStrategy.tileY(y);
        while (count > 0) {
            this->bilerpPoint(x, tiledY);
            x += dx;
            count -= 1;
        }
    }

    void yProcessSpan(Span span) {
        SkScalar tiledY = fYStrategy.tileY(span.startY());
        if (0.5f <= tiledY && tiledY < fYMax - 0.5f ) {
            Span tiledSpan{{span.startX(), tiledY}, span.length(), span.count()};
            fNext->pointSpan(tiledSpan);
        } else {
            // Convert to the Y0 bilerp sample set by shifting by -0.5f. Then tile that new y
            // value and shift it back resulting in the working Y0. Do the same thing with Y1 but
            // in the opposite direction.
            SkScalar y0 = fYStrategy.tileY(span.startY() - 0.5f) + 0.5f;
            SkScalar y1 = fYStrategy.tileY(span.startY() + 0.5f) - 0.5f;
            Span newSpan{{span.startX(), y0}, span.length(), span.count()};
            fNext->bilerpSpan(newSpan, y1);
        }
    }
    void breakIntoEdges(Span span) {
        if (span.count() == 1) {
            this->bilerpPoint(span.startX(), span.startY());
        } else if (span.length() == 0) {
            yProcessSpan(span);
        } else {
            SkScalar dx = span.length() / (span.count() - 1);
            if (span.length() > 0) {
                Span leftBorder = span.breakAt(0.5f, dx);
                if (!leftBorder.isEmpty()) {
                    this->handleEdges(leftBorder, dx);
                }
                Span center = span.breakAt(fXMax - 0.5f, dx);
                if (!center.isEmpty()) {
                    this->yProcessSpan(center);
                }

                if (!span.isEmpty()) {
                    this->handleEdges(span, dx);
                }
            } else {
                Span center = span.breakAt(fXMax + 0.5f, dx);
                if (!span.isEmpty()) {
                    this->handleEdges(span, dx);
                }
                Span leftEdge = center.breakAt(0.5f, dx);
                if (!center.isEmpty()) {
                    this->yProcessSpan(center);
                }
                if (!leftEdge.isEmpty()) {
                    this->handleEdges(leftEdge, dx);
                }

            }
        }
    }

    Next* const fNext;
    SkScalar fXMax;
    SkScalar fYMax;
    XStrategy fXStrategy;
    YStrategy fYStrategy;
};

template <typename XStrategy, typename YStrategy, typename Next>
void make_tile_stage(
    SkFilterQuality filterQuality, SkISize dimensions,
    Next* next, SkLinearBitmapPipeline::TileStage* tileStage) {
    if (filterQuality == kNone_SkFilterQuality) {
        tileStage->initStage<NearestTileStage<XStrategy, YStrategy, Next>>(next, dimensions);
    } else {
        tileStage->initStage<BilerpTileStage<XStrategy, YStrategy, Next>>(next, dimensions);
    }
}
template <typename XStrategy>
void choose_tiler_ymode(
    SkShader::TileMode yMode, SkFilterQuality filterQuality, SkISize dimensions,
    SkLinearBitmapPipeline::SampleProcessorInterface* next,
    SkLinearBitmapPipeline::TileStage* tileStage) {
    switch (yMode) {
        case SkShader::kClamp_TileMode:
            make_tile_stage<XStrategy, YClampStrategy>(filterQuality, dimensions, next, tileStage);
            break;
        case SkShader::kRepeat_TileMode:
            make_tile_stage<XStrategy, YRepeatStrategy>(filterQuality, dimensions, next, tileStage);
            break;
        case SkShader::kMirror_TileMode:
            make_tile_stage<XStrategy, YMirrorStrategy>(filterQuality, dimensions, next, tileStage);
            break;
    }
};

static SkLinearBitmapPipeline::PointProcessorInterface* choose_tiler(
    SkLinearBitmapPipeline::SampleProcessorInterface* next,
    SkISize dimensions,
    SkShader::TileMode xMode,
    SkShader::TileMode yMode,
    SkFilterQuality filterQuality,
    SkScalar dx,
    SkLinearBitmapPipeline::TileStage* tileStage)
{
    switch (xMode) {
        case SkShader::kClamp_TileMode:
            choose_tiler_ymode<XClampStrategy>(yMode, filterQuality, dimensions, next, tileStage);
            break;
        case SkShader::kRepeat_TileMode:
            if (dx == 1.0f && filterQuality == kNone_SkFilterQuality) {
                choose_tiler_ymode<XRepeatUnitScaleStrategy>(
                    yMode, kNone_SkFilterQuality, dimensions, next, tileStage);
            } else {
                choose_tiler_ymode<XRepeatStrategy>(
                    yMode, filterQuality, dimensions, next, tileStage);
            }
            break;
        case SkShader::kMirror_TileMode:
            choose_tiler_ymode<XMirrorStrategy>(yMode, filterQuality, dimensions, next, tileStage);
            break;
    }

    return tileStage->get();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Specialized Samplers

// RGBA8888UnitRepeatSrc - A sampler that takes advantage of the fact the the src and destination
// are the same format and do not need in transformations in pixel space. Therefore, there is no
// need to convert them to HiFi pixel format.
class RGBA8888UnitRepeatSrc final : public SkLinearBitmapPipeline::SampleProcessorInterface,
                                    public SkLinearBitmapPipeline::DestinationInterface {
public:
    RGBA8888UnitRepeatSrc(const uint32_t* src, int32_t width)
        : fSrc{src}, fWidth{width} { }

    void VECTORCALL pointListFew(int n, Sk4s xs, Sk4s ys) override {
        SkASSERT(fDest + n <= fEnd);
        // At this point xs and ys should be >= 0, so trunc is the same as floor.
        Sk4i iXs = SkNx_cast<int>(xs);
        Sk4i iYs = SkNx_cast<int>(ys);

        if (n >= 1) *fDest++ = *this->pixelAddress(iXs[0], iYs[0]);
        if (n >= 2) *fDest++ = *this->pixelAddress(iXs[1], iYs[1]);
        if (n >= 3) *fDest++ = *this->pixelAddress(iXs[2], iYs[2]);
    }

    void VECTORCALL pointList4(Sk4s xs, Sk4s ys) override {
        SkASSERT(fDest + 4 <= fEnd);
        Sk4i iXs = SkNx_cast<int>(xs);
        Sk4i iYs = SkNx_cast<int>(ys);
        *fDest++ = *this->pixelAddress(iXs[0], iYs[0]);
        *fDest++ = *this->pixelAddress(iXs[1], iYs[1]);
        *fDest++ = *this->pixelAddress(iXs[2], iYs[2]);
        *fDest++ = *this->pixelAddress(iXs[3], iYs[3]);
    }

    void pointSpan(Span span) override {
        SkASSERT(fDest + span.count() <= fEnd);
        if (span.length() != 0.0f) {
            int32_t x = SkScalarTruncToInt(span.startX());
            int32_t y = SkScalarTruncToInt(span.startY());
            const uint32_t* src = this->pixelAddress(x, y);
            memmove(fDest, src, span.count() * sizeof(uint32_t));
            fDest += span.count();
        }
    }

    void repeatSpan(Span span, int32_t repeatCount) override {
        SkASSERT(fDest + span.count() * repeatCount <= fEnd);

        int32_t x = SkScalarTruncToInt(span.startX());
        int32_t y = SkScalarTruncToInt(span.startY());
        const uint32_t* src = this->pixelAddress(x, y);
        uint32_t* dest = fDest;
        while (repeatCount --> 0) {
            memmove(dest, src, span.count() * sizeof(uint32_t));
            dest += span.count();
        }
        fDest = dest;
    }

    void VECTORCALL bilerpEdge(Sk4s xs, Sk4s ys) override { SkFAIL("Not Implemented"); }

    void bilerpSpan(Span span, SkScalar y) override { SkFAIL("Not Implemented"); }

    void setDestination(void* dst, int count) override  {
        fDest = static_cast<uint32_t*>(dst);
        fEnd = fDest + count;
    }

private:
    const uint32_t* pixelAddress(int32_t x, int32_t y) {
        return &fSrc[fWidth * y + x];
    }
    const uint32_t* const fSrc;
    const int32_t         fWidth;
    uint32_t*             fDest;
    uint32_t*             fEnd;
};

// RGBA8888UnitRepeatSrc - A sampler that takes advantage of the fact the the src and destination
// are the same format and do not need in transformations in pixel space. Therefore, there is no
// need to convert them to HiFi pixel format.
class RGBA8888UnitRepeatSrcOver final : public SkLinearBitmapPipeline::SampleProcessorInterface,
                                        public SkLinearBitmapPipeline::DestinationInterface {
public:
    RGBA8888UnitRepeatSrcOver(const uint32_t* src, int32_t width)
        : fSrc{src}, fWidth{width} { }

    void VECTORCALL pointListFew(int n, Sk4s xs, Sk4s ys) override {
        SkASSERT(fDest + n <= fEnd);
        // At this point xs and ys should be >= 0, so trunc is the same as floor.
        Sk4i iXs = SkNx_cast<int>(xs);
        Sk4i iYs = SkNx_cast<int>(ys);

        if (n >= 1) blendPixelAt(iXs[0], iYs[0]);
        if (n >= 2) blendPixelAt(iXs[1], iYs[1]);
        if (n >= 3) blendPixelAt(iXs[2], iYs[2]);
    }

    void VECTORCALL pointList4(Sk4s xs, Sk4s ys) override {
        SkASSERT(fDest + 4 <= fEnd);
        Sk4i iXs = SkNx_cast<int>(xs);
        Sk4i iYs = SkNx_cast<int>(ys);
        blendPixelAt(iXs[0], iYs[0]);
        blendPixelAt(iXs[1], iYs[1]);
        blendPixelAt(iXs[2], iYs[2]);
        blendPixelAt(iXs[3], iYs[3]);
    }

    void pointSpan(Span span) override {
        if (span.length() != 0.0f) {
            this->repeatSpan(span, 1);
        }
    }

    void repeatSpan(Span span, int32_t repeatCount) override {
        SkASSERT(fDest + span.count() * repeatCount <= fEnd);
        SkASSERT(span.count() > 0);
        SkASSERT(repeatCount > 0);

        int32_t x = (int32_t)span.startX();
        int32_t y = (int32_t)span.startY();
        const uint32_t* beginSpan = this->pixelAddress(x, y);

        SkOpts::srcover_srgb_srgb(fDest, beginSpan, span.count() * repeatCount, span.count());

        fDest += span.count() * repeatCount;

        SkASSERT(fDest <= fEnd);
    }

    void VECTORCALL bilerpEdge(Sk4s xs, Sk4s ys) override { SkFAIL("Not Implemented"); }

    void bilerpSpan(Span span, SkScalar y) override { SkFAIL("Not Implemented"); }

    void setDestination(void* dst, int count) override  {
        SkASSERT(count > 0);
        fDest = static_cast<uint32_t*>(dst);
        fEnd = fDest + count;
    }

private:
    const uint32_t* pixelAddress(int32_t x, int32_t y) {
        return &fSrc[fWidth * y + x];
    }

    void blendPixelAt(int32_t x, int32_t y) {
        const uint32_t* src = this->pixelAddress(x, y);
        SkOpts::srcover_srgb_srgb(fDest, src, 1, 1);
        fDest += 1;
    };

    const uint32_t* const fSrc;
    const int32_t         fWidth;
    uint32_t*             fDest;
    uint32_t*             fEnd;
};

using Blender = SkLinearBitmapPipeline::BlendProcessorInterface;

template <SkColorType colorType, template <SkColorType, SkGammaType, typename> class Sampler>
static void choose_specific_sampler(
    Blender* next,
    const SkPixmap& srcPixmap,
    SkLinearBitmapPipeline::SampleStage* sampleStage)
{
    if (srcPixmap.info().gammaCloseToSRGB()) {
        using S = Sampler<colorType, kSRGB_SkGammaType, Blender>;
        sampleStage->initStage<S>(next, srcPixmap);
    } else {
        using S = Sampler<colorType, kLinear_SkGammaType, Blender>;
        sampleStage->initStage<S>(next, srcPixmap);
    }
}

template<template <SkColorType, SkGammaType, typename> class Sampler>
static SkLinearBitmapPipeline::SampleProcessorInterface* choose_pixel_sampler_base(
    Blender* next,
    const SkPixmap& srcPixmap,
    const SkColor A8TintColor,
    SkLinearBitmapPipeline::SampleStage* sampleStage)
{
    const SkImageInfo& imageInfo = srcPixmap.info();
    switch (imageInfo.colorType()) {
        case kAlpha_8_SkColorType: {
                using S = Sampler<kAlpha_8_SkColorType, kLinear_SkGammaType, Blender>;
                sampleStage->initStage<S>(next, srcPixmap, A8TintColor);
            }
            break;
        case kARGB_4444_SkColorType:
            choose_specific_sampler<kARGB_4444_SkColorType, Sampler>(next, srcPixmap, sampleStage);
            break;
        case kRGB_565_SkColorType:
            choose_specific_sampler<kRGB_565_SkColorType, Sampler>(next, srcPixmap, sampleStage);
            break;
        case kRGBA_8888_SkColorType:
            choose_specific_sampler<kRGBA_8888_SkColorType, Sampler>(next, srcPixmap, sampleStage);
            break;
        case kBGRA_8888_SkColorType:
            choose_specific_sampler<kBGRA_8888_SkColorType, Sampler>(next, srcPixmap, sampleStage);
            break;
        case kIndex_8_SkColorType:
            choose_specific_sampler<kIndex_8_SkColorType, Sampler>(next, srcPixmap, sampleStage);
            break;
        case kGray_8_SkColorType:
            choose_specific_sampler<kGray_8_SkColorType, Sampler>(next, srcPixmap, sampleStage);
            break;
        case kRGBA_F16_SkColorType: {
                using S = Sampler<kRGBA_F16_SkColorType, kLinear_SkGammaType, Blender>;
                sampleStage->initStage<S>(next, srcPixmap);
            }
            break;
        default:
            SkFAIL("Not implemented. Unsupported src");
            break;
    }
    return sampleStage->get();
}

SkLinearBitmapPipeline::SampleProcessorInterface* choose_pixel_sampler(
    Blender* next,
    SkFilterQuality filterQuality,
    const SkPixmap& srcPixmap,
    const SkColor A8TintColor,
    SkLinearBitmapPipeline::SampleStage* sampleStage)
{
    if (filterQuality == kNone_SkFilterQuality) {
        return choose_pixel_sampler_base<NearestNeighborSampler>(
            next, srcPixmap, A8TintColor, sampleStage);
    } else {
        return choose_pixel_sampler_base<BilerpSampler>(next, srcPixmap, A8TintColor, sampleStage);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Pixel Blender Stage
template <SkAlphaType alphaType>
class SrcFPPixel final : public SkLinearBitmapPipeline::BlendProcessorInterface {
public:
    SrcFPPixel(float postAlpha) : fPostAlpha{postAlpha} { }
    SrcFPPixel(const SrcFPPixel& Blender) : fPostAlpha(Blender.fPostAlpha) {}
    void VECTORCALL blendPixel(Sk4f pixel) override {
        SkASSERT(fDst + 1 <= fEnd );
        SrcPixel(fDst, pixel, 0);
        fDst += 1;
    }

    void VECTORCALL blend4Pixels(Sk4f p0, Sk4f p1, Sk4f p2, Sk4f p3) override {
        SkASSERT(fDst + 4 <= fEnd);
        SkPM4f* dst = fDst;
        SrcPixel(dst, p0, 0);
        SrcPixel(dst, p1, 1);
        SrcPixel(dst, p2, 2);
        SrcPixel(dst, p3, 3);
        fDst += 4;
    }

    void setDestination(void* dst, int count) override {
        fDst = static_cast<SkPM4f*>(dst);
        fEnd = fDst + count;
    }

private:
    void VECTORCALL SrcPixel(SkPM4f* dst, Sk4f pixel, int index) {
        Sk4f newPixel = pixel;
        if (alphaType == kUnpremul_SkAlphaType) {
            newPixel = Premultiply(pixel);
        }
        newPixel = newPixel * fPostAlpha;
        newPixel.store(dst + index);
    }
    static Sk4f VECTORCALL Premultiply(Sk4f pixel) {
        float alpha = pixel[3];
        return pixel * Sk4f{alpha, alpha, alpha, 1.0f};
    }

    SkPM4f* fDst;
    SkPM4f* fEnd;
    Sk4f fPostAlpha;
};

static SkLinearBitmapPipeline::BlendProcessorInterface* choose_blender_for_shading(
    SkAlphaType alphaType,
    float postAlpha,
    SkLinearBitmapPipeline::BlenderStage* blenderStage) {
    if (alphaType == kUnpremul_SkAlphaType) {
        blenderStage->initSink<SrcFPPixel<kUnpremul_SkAlphaType>>(postAlpha);
    } else {
        // kOpaque_SkAlphaType is treated the same as kPremul_SkAlphaType
        blenderStage->initSink<SrcFPPixel<kPremul_SkAlphaType>>(postAlpha);
    }
    return blenderStage->get();
}

}  // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkLinearBitmapPipeline
SkLinearBitmapPipeline::~SkLinearBitmapPipeline() {}

SkLinearBitmapPipeline::SkLinearBitmapPipeline(
    const SkMatrix& inverse,
    SkFilterQuality filterQuality,
    SkShader::TileMode xTile, SkShader::TileMode yTile,
    SkColor paintColor,
    const SkPixmap& srcPixmap)
{
    SkISize dimensions = srcPixmap.info().dimensions();
    const SkImageInfo& srcImageInfo = srcPixmap.info();

    SkMatrix adjustedInverse = inverse;
    if (filterQuality == kNone_SkFilterQuality) {
        if (inverse.getScaleX() >= 0.0f) {
            adjustedInverse.setTranslateX(
                nextafterf(inverse.getTranslateX(), std::floor(inverse.getTranslateX())));
        }
        if (inverse.getScaleY() >= 0.0f) {
            adjustedInverse.setTranslateY(
                nextafterf(inverse.getTranslateY(), std::floor(inverse.getTranslateY())));
        }
    }

    SkScalar dx = adjustedInverse.getScaleX();

    // If it is an index 8 color type, the sampler converts to unpremul for better fidelity.
    SkAlphaType alphaType = srcImageInfo.alphaType();
    if (srcPixmap.colorType() == kIndex_8_SkColorType) {
        alphaType = kUnpremul_SkAlphaType;
    }

    float postAlpha = SkColorGetA(paintColor) * (1.0f / 255.0f);
    // As the stages are built, the chooser function may skip a stage. For example, with the
    // identity matrix, the matrix stage is skipped, and the tilerStage is the first stage.
    auto blenderStage = choose_blender_for_shading(alphaType, postAlpha, &fBlenderStage);
    auto samplerStage = choose_pixel_sampler(
        blenderStage, filterQuality, srcPixmap, paintColor, &fSampleStage);
    auto tilerStage   = choose_tiler(samplerStage, dimensions, xTile, yTile,
                                     filterQuality, dx, &fTileStage);
    fFirstStage       = choose_matrix(tilerStage, adjustedInverse, &fMatrixStage);
    fLastStage        = blenderStage;
}

bool SkLinearBitmapPipeline::ClonePipelineForBlitting(
    SkEmbeddableLinearPipeline* pipelineStorage,
    const SkLinearBitmapPipeline& pipeline,
    SkMatrix::TypeMask matrixMask,
    SkShader::TileMode xTileMode,
    SkShader::TileMode yTileMode,
    SkFilterQuality filterQuality,
    const SkPixmap& srcPixmap,
    float finalAlpha,
    SkXfermode::Mode xferMode,
    const SkImageInfo& dstInfo)
{
    if (xferMode == SkXfermode::kSrcOver_Mode
        && srcPixmap.info().alphaType() == kOpaque_SkAlphaType) {
        xferMode = SkXfermode::kSrc_Mode;
    }

    if (matrixMask & ~SkMatrix::kTranslate_Mask ) { return false; }
    if (filterQuality != SkFilterQuality::kNone_SkFilterQuality) { return false; }
    if (finalAlpha != 1.0f) { return false; }
    if (srcPixmap.info().colorType() != kRGBA_8888_SkColorType
        || dstInfo.colorType() != kRGBA_8888_SkColorType) { return false; }

    if (!srcPixmap.info().gammaCloseToSRGB() || !dstInfo.gammaCloseToSRGB()) {
        return false;
    }

    if (xferMode != SkXfermode::kSrc_Mode && xferMode != SkXfermode::kSrcOver_Mode) {
        return false;
    }

    pipelineStorage->init(pipeline, srcPixmap, xferMode, dstInfo);

    return true;
}

SkLinearBitmapPipeline::SkLinearBitmapPipeline(
    const SkLinearBitmapPipeline& pipeline,
    const SkPixmap& srcPixmap,
    SkXfermode::Mode mode,
    const SkImageInfo& dstInfo)
{
    SkASSERT(mode == SkXfermode::kSrc_Mode || mode == SkXfermode::kSrcOver_Mode);
    SkASSERT(srcPixmap.info().colorType() == dstInfo.colorType()
             && srcPixmap.info().colorType() == kRGBA_8888_SkColorType);

    if (mode == SkXfermode::kSrc_Mode) {
        fSampleStage.initSink<RGBA8888UnitRepeatSrc>(
            srcPixmap.writable_addr32(0, 0), srcPixmap.rowBytes() / 4);
        fLastStage = fSampleStage.getInterface<DestinationInterface, RGBA8888UnitRepeatSrc>();
    } else {
        fSampleStage.initSink<RGBA8888UnitRepeatSrcOver>(
            srcPixmap.writable_addr32(0, 0), srcPixmap.rowBytes() / 4);
        fLastStage = fSampleStage.getInterface<DestinationInterface, RGBA8888UnitRepeatSrcOver>();
    }

    auto sampleStage = fSampleStage.get();
    auto tilerStage = pipeline.fTileStage.cloneStageTo(sampleStage, &fTileStage);
    tilerStage = (tilerStage != nullptr) ? tilerStage : sampleStage;
    auto matrixStage = pipeline.fMatrixStage.cloneStageTo(tilerStage, &fMatrixStage);
    matrixStage = (matrixStage != nullptr) ? matrixStage : tilerStage;
    fFirstStage = matrixStage;
}

void SkLinearBitmapPipeline::shadeSpan4f(int x, int y, SkPM4f* dst, int count) {
    SkASSERT(count > 0);
    this->blitSpan(x, y, dst, count);
}

void SkLinearBitmapPipeline::blitSpan(int x, int y, void* dst, int count) {
    SkASSERT(count > 0);
    fLastStage->setDestination(dst, count);

    // The count and length arguments start out in a precise relation in order to keep the
    // math correct through the different stages. Count is the number of pixel to produce.
    // Since the code samples at pixel centers, length is the distance from the center of the
    // first pixel to the center of the last pixel. This implies that length is count-1.
    fFirstStage->pointSpan(Span{{x + 0.5f, y + 0.5f}, count - 1.0f, count});
}
