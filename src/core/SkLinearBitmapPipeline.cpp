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

#include "SkArenaAlloc.h"
#include "SkLinearBitmapPipeline_core.h"
#include "SkLinearBitmapPipeline_matrix.h"
#include "SkLinearBitmapPipeline_tile.h"
#include "SkLinearBitmapPipeline_sample.h"
#include "SkNx.h"
#include "SkOpts.h"
#include "SkPM4f.h"

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

    MatrixStage(Next* next, MatrixStage* stage)
        : fNext{next}
        , fStrategy{stage->fStrategy} { }

    void SK_VECTORCALL pointListFew(int n, Sk4s xs, Sk4s ys) override {
        fStrategy.processPoints(&xs, &ys);
        fNext->pointListFew(n, xs, ys);
    }

    void SK_VECTORCALL pointList4(Sk4s xs, Sk4s ys) override {
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


////////////////////////////////////////////////////////////////////////////////////////////////////
// Tile Stage

template<typename XStrategy, typename YStrategy, typename Next>
class CombinedTileStage final : public SkLinearBitmapPipeline::PointProcessorInterface {
public:
    CombinedTileStage(Next* next, SkISize dimensions)
        : fNext{next}
        , fXStrategy{dimensions.width()}
        , fYStrategy{dimensions.height()}{ }

    CombinedTileStage(Next* next, CombinedTileStage* stage)
        : fNext{next}
        , fXStrategy{stage->fXStrategy}
        , fYStrategy{stage->fYStrategy} { }

    void SK_VECTORCALL pointListFew(int n, Sk4s xs, Sk4s ys) override {
        fXStrategy.tileXPoints(&xs);
        fYStrategy.tileYPoints(&ys);
        fNext->pointListFew(n, xs, ys);
    }

    void SK_VECTORCALL pointList4(Sk4s xs, Sk4s ys) override {
        fXStrategy.tileXPoints(&xs);
        fYStrategy.tileYPoints(&ys);
        fNext->pointList4(xs, ys);
    }

    // The span you pass must not be empty.
    void pointSpan(Span span) override {
        SkASSERT(!span.isEmpty());
        SkPoint start; SkScalar length; int count;
        std::tie(start, length, count) = span;

        if (span.count() == 1) {
            // DANGER:
            // The explicit casts from float to Sk4f are not usually necessary, but are here to
            // work around an MSVC 2015u2 c++ code generation bug. This is tracked using skia bug
            // 5566.
            this->pointListFew(1, Sk4f{span.startX()}, Sk4f{span.startY()});
            return;
        }

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

    void SK_VECTORCALL pointListFew(int n, Sk4s xs, Sk4s ys) override {
        SkASSERT(fDest + n <= fEnd);
        // At this point xs and ys should be >= 0, so trunc is the same as floor.
        Sk4i iXs = SkNx_cast<int>(xs);
        Sk4i iYs = SkNx_cast<int>(ys);

        if (n >= 1) *fDest++ = *this->pixelAddress(iXs[0], iYs[0]);
        if (n >= 2) *fDest++ = *this->pixelAddress(iXs[1], iYs[1]);
        if (n >= 3) *fDest++ = *this->pixelAddress(iXs[2], iYs[2]);
    }

    void SK_VECTORCALL pointList4(Sk4s xs, Sk4s ys) override {
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

    void SK_VECTORCALL pointListFew(int n, Sk4s xs, Sk4s ys) override {
        SkASSERT(fDest + n <= fEnd);
        // At this point xs and ys should be >= 0, so trunc is the same as floor.
        Sk4i iXs = SkNx_cast<int>(xs);
        Sk4i iYs = SkNx_cast<int>(ys);

        if (n >= 1) blendPixelAt(iXs[0], iYs[0]);
        if (n >= 2) blendPixelAt(iXs[1], iYs[1]);
        if (n >= 3) blendPixelAt(iXs[2], iYs[2]);
    }

    void SK_VECTORCALL pointList4(Sk4s xs, Sk4s ys) override {
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
    }

    const uint32_t* const fSrc;
    const int32_t         fWidth;
    uint32_t*             fDest;
    uint32_t*             fEnd;
};

using Blender = SkLinearBitmapPipeline::BlendProcessorInterface;

////////////////////////////////////////////////////////////////////////////////////////////////////
// Pixel Blender Stage
template <SkAlphaType alphaType>
class SrcFPPixel final : public Blender {
public:
    SrcFPPixel(float postAlpha) : fPostAlpha{postAlpha} { }
    SrcFPPixel(const SrcFPPixel& Blender) : fPostAlpha(Blender.fPostAlpha) {}
    void SK_VECTORCALL blendPixel(Sk4f pixel) override {
        SkASSERT(fDst + 1 <= fEnd );
        this->srcPixel(fDst, pixel, 0);
        fDst += 1;
    }

    void SK_VECTORCALL blend4Pixels(Sk4f p0, Sk4f p1, Sk4f p2, Sk4f p3) override {
        SkASSERT(fDst + 4 <= fEnd);
        SkPM4f* dst = fDst;
        this->srcPixel(dst, p0, 0);
        this->srcPixel(dst, p1, 1);
        this->srcPixel(dst, p2, 2);
        this->srcPixel(dst, p3, 3);
        fDst += 4;
    }

    void setDestination(void* dst, int count) override {
        fDst = static_cast<SkPM4f*>(dst);
        fEnd = fDst + count;
    }

private:
    void SK_VECTORCALL srcPixel(SkPM4f* dst, Sk4f pixel, int index) {
        check_pixel(pixel);

        Sk4f newPixel = pixel;
        if (alphaType == kUnpremul_SkAlphaType) {
            newPixel = Premultiply(pixel);
        }
        newPixel = newPixel * fPostAlpha;
        newPixel.store(dst + index);
    }
    static Sk4f SK_VECTORCALL Premultiply(Sk4f pixel) {
        float alpha = pixel[3];
        return pixel * Sk4f{alpha, alpha, alpha, 1.0f};
    }

    SkPM4f* fDst;
    SkPM4f* fEnd;
    float   fPostAlpha;
};

}  // namespace

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkLinearBitmapPipeline
SkLinearBitmapPipeline::~SkLinearBitmapPipeline() {}

SkLinearBitmapPipeline::SkLinearBitmapPipeline(
    const SkMatrix& inverse,
    SkFilterQuality filterQuality,
    SkShader::TileMode xTile, SkShader::TileMode yTile,
    SkColor paintColor,
    const SkPixmap& srcPixmap,
    SkArenaAlloc* allocator)
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

    float postAlpha = SkColorGetA(paintColor) * (1.0f / 255.0f);
    // As the stages are built, the chooser function may skip a stage. For example, with the
    // identity matrix, the matrix stage is skipped, and the tilerStage is the first stage.
    auto blenderStage = this->chooseBlenderForShading(alphaType, postAlpha, allocator);
    auto samplerStage = this->chooseSampler(
        blenderStage, filterQuality, xTile, yTile, srcPixmap, paintColor, allocator);
    auto tilerStage   = this->chooseTiler(
        samplerStage, dimensions, xTile, yTile, filterQuality, dx, allocator);
    fFirstStage       = this->chooseMatrix(tilerStage, adjustedInverse, allocator);
    fLastStage        = blenderStage;
}

SkLinearBitmapPipeline::SkLinearBitmapPipeline(
    const SkLinearBitmapPipeline& pipeline,
    const SkPixmap& srcPixmap,
    SkBlendMode mode,
    const SkImageInfo& dstInfo,
    SkArenaAlloc* allocator)
{
    SkASSERT(mode == SkBlendMode::kSrc || mode == SkBlendMode::kSrcOver);
    SkASSERT(srcPixmap.info().colorType() == dstInfo.colorType()
             && srcPixmap.info().colorType() == kRGBA_8888_SkColorType);

    SampleProcessorInterface* sampleStage;
    if (mode == SkBlendMode::kSrc) {
        auto sampler = allocator->make<RGBA8888UnitRepeatSrc>(
            srcPixmap.writable_addr32(0, 0), srcPixmap.rowBytes() / 4);
        sampleStage = sampler;
        fLastStage = sampler;
    } else {
        auto sampler = allocator->make<RGBA8888UnitRepeatSrcOver>(
            srcPixmap.writable_addr32(0, 0), srcPixmap.rowBytes() / 4);
        sampleStage = sampler;
        fLastStage = sampler;
    }

    auto tilerStage = pipeline.fTileStageCloner(sampleStage, allocator);
    auto matrixStage = pipeline.fMatrixStageCloner(tilerStage, allocator);
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

SkLinearBitmapPipeline::PointProcessorInterface*
SkLinearBitmapPipeline::chooseMatrix(
    PointProcessorInterface* next,
    const SkMatrix& inverse,
    SkArenaAlloc* allocator)
{
    if (inverse.hasPerspective()) {
        auto matrixStage = allocator->make<PerspectiveMatrix<>>(
            next,
            SkVector{inverse.getTranslateX(), inverse.getTranslateY()},
            SkVector{inverse.getScaleX(), inverse.getScaleY()},
            SkVector{inverse.getSkewX(), inverse.getSkewY()},
            SkVector{inverse.getPerspX(), inverse.getPerspY()},
            inverse.get(SkMatrix::kMPersp2));
        fMatrixStageCloner =
            [matrixStage](PointProcessorInterface* cloneNext, SkArenaAlloc* memory) {
                return memory->make<PerspectiveMatrix<>>(cloneNext, matrixStage);
            };
        return matrixStage;
    } else if (inverse.getSkewX() != 0.0f || inverse.getSkewY() != 0.0f) {
        auto matrixStage = allocator->make<AffineMatrix<>>(
            next,
            SkVector{inverse.getTranslateX(), inverse.getTranslateY()},
            SkVector{inverse.getScaleX(), inverse.getScaleY()},
            SkVector{inverse.getSkewX(), inverse.getSkewY()});
        fMatrixStageCloner =
            [matrixStage](PointProcessorInterface* cloneNext, SkArenaAlloc* memory) {
                return memory->make<AffineMatrix<>>(cloneNext, matrixStage);
            };
        return matrixStage;
    } else if (inverse.getScaleX() != 1.0f || inverse.getScaleY() != 1.0f) {
        auto matrixStage = allocator->make<ScaleMatrix<>>(
            next,
            SkVector{inverse.getTranslateX(), inverse.getTranslateY()},
            SkVector{inverse.getScaleX(), inverse.getScaleY()});
        fMatrixStageCloner =
            [matrixStage](PointProcessorInterface* cloneNext, SkArenaAlloc* memory) {
                return memory->make<ScaleMatrix<>>(cloneNext, matrixStage);
            };
        return matrixStage;
    } else if (inverse.getTranslateX() != 0.0f || inverse.getTranslateY() != 0.0f) {
        auto matrixStage = allocator->make<TranslateMatrix<>>(
            next,
            SkVector{inverse.getTranslateX(), inverse.getTranslateY()});
        fMatrixStageCloner =
            [matrixStage](PointProcessorInterface* cloneNext, SkArenaAlloc* memory) {
                return memory->make<TranslateMatrix<>>(cloneNext, matrixStage);
            };
        return matrixStage;
    } else {
        fMatrixStageCloner = [](PointProcessorInterface* cloneNext, SkArenaAlloc* memory) {
            return cloneNext;
        };
        return next;
    }
}

template <typename Tiler>
SkLinearBitmapPipeline::PointProcessorInterface* SkLinearBitmapPipeline::createTiler(
    SampleProcessorInterface* next,
    SkISize dimensions,
    SkArenaAlloc* allocator)
{
    auto tilerStage = allocator->make<Tiler>(next, dimensions);
    fTileStageCloner =
        [tilerStage](SampleProcessorInterface* cloneNext,
                     SkArenaAlloc* memory) -> PointProcessorInterface* {
            return memory->make<Tiler>(cloneNext, tilerStage);
        };
    return tilerStage;
}

template <typename XStrategy>
SkLinearBitmapPipeline::PointProcessorInterface* SkLinearBitmapPipeline::chooseTilerYMode(
    SampleProcessorInterface* next,
    SkShader::TileMode yMode,
    SkISize dimensions,
    SkArenaAlloc* allocator)
{
    switch (yMode) {
        case SkShader::kClamp_TileMode: {
            using Tiler = CombinedTileStage<XStrategy, YClampStrategy, SampleProcessorInterface>;
            return this->createTiler<Tiler>(next, dimensions, allocator);
        }
        case SkShader::kRepeat_TileMode: {
            using Tiler = CombinedTileStage<XStrategy, YRepeatStrategy, SampleProcessorInterface>;
            return this->createTiler<Tiler>(next, dimensions, allocator);
        }
        case SkShader::kMirror_TileMode: {
            using Tiler = CombinedTileStage<XStrategy, YMirrorStrategy, SampleProcessorInterface>;
            return this->createTiler<Tiler>(next, dimensions, allocator);
        }
    }

    // Should never get here.
    SkFAIL("Not all Y tile cases covered.");
    return nullptr;
}

SkLinearBitmapPipeline::PointProcessorInterface* SkLinearBitmapPipeline::chooseTiler(
    SampleProcessorInterface* next,
    SkISize dimensions,
    SkShader::TileMode xMode,
    SkShader::TileMode yMode,
    SkFilterQuality filterQuality,
    SkScalar dx,
    SkArenaAlloc* allocator)
{
    switch (xMode) {
        case SkShader::kClamp_TileMode:
            return this->chooseTilerYMode<XClampStrategy>(next, yMode, dimensions, allocator);
        case SkShader::kRepeat_TileMode:
            if (dx == 1.0f && filterQuality == kNone_SkFilterQuality) {
                return this->chooseTilerYMode<XRepeatUnitScaleStrategy>(
                    next, yMode, dimensions, allocator);
            } else {
                return this->chooseTilerYMode<XRepeatStrategy>(
                    next, yMode, dimensions, allocator);
            }
        case SkShader::kMirror_TileMode:
            return this->chooseTilerYMode<XMirrorStrategy>(next, yMode, dimensions, allocator);
    }

    // Should never get here.
    SkFAIL("Not all X tile cases covered.");
    return nullptr;
}

template <SkColorType colorType>
SkLinearBitmapPipeline::PixelAccessorInterface*
    SkLinearBitmapPipeline::chooseSpecificAccessor(
    const SkPixmap& srcPixmap,
    SkArenaAlloc* allocator)
{
    if (srcPixmap.info().gammaCloseToSRGB()) {
        using Accessor = PixelAccessor<colorType, kSRGB_SkGammaType>;
        return allocator->make<Accessor>(srcPixmap);
    } else {
        using Accessor = PixelAccessor<colorType, kLinear_SkGammaType>;
        return allocator->make<Accessor>(srcPixmap);
    }
}

SkLinearBitmapPipeline::PixelAccessorInterface* SkLinearBitmapPipeline::choosePixelAccessor(
    const SkPixmap& srcPixmap,
    const SkColor A8TintColor,
    SkArenaAlloc* allocator)
{
    const SkImageInfo& imageInfo = srcPixmap.info();

    switch (imageInfo.colorType()) {
        case kAlpha_8_SkColorType: {
            using Accessor = PixelAccessor<kAlpha_8_SkColorType, kLinear_SkGammaType>;
            return allocator->make<Accessor>(srcPixmap, A8TintColor);
        }
        case kARGB_4444_SkColorType:
            return this->chooseSpecificAccessor<kARGB_4444_SkColorType>(srcPixmap, allocator);
        case kRGB_565_SkColorType:
            return this->chooseSpecificAccessor<kRGB_565_SkColorType>(srcPixmap, allocator);
        case kRGBA_8888_SkColorType:
            return this->chooseSpecificAccessor<kRGBA_8888_SkColorType>(srcPixmap, allocator);
        case kBGRA_8888_SkColorType:
            return this->chooseSpecificAccessor<kBGRA_8888_SkColorType>(srcPixmap, allocator);
        case kGray_8_SkColorType:
            return this->chooseSpecificAccessor<kGray_8_SkColorType>(srcPixmap, allocator);
        case kRGBA_F16_SkColorType: {
            using Accessor = PixelAccessor<kRGBA_F16_SkColorType, kLinear_SkGammaType>;
            return allocator->make<Accessor>(srcPixmap);
        }
        default:
            // Should never get here.
            SkFAIL("Pixel source not supported.");
            return nullptr;
    }
}

SkLinearBitmapPipeline::SampleProcessorInterface* SkLinearBitmapPipeline::chooseSampler(
    Blender* next,
    SkFilterQuality filterQuality,
    SkShader::TileMode xTile, SkShader::TileMode yTile,
    const SkPixmap& srcPixmap,
    const SkColor A8TintColor,
    SkArenaAlloc* allocator)
{
    const SkImageInfo& imageInfo = srcPixmap.info();
    SkISize dimensions = imageInfo.dimensions();

    // Special case samplers with fully expanded templates
    if (imageInfo.gammaCloseToSRGB()) {
        if (filterQuality == kNone_SkFilterQuality) {
            switch (imageInfo.colorType()) {
                case kN32_SkColorType: {
                    using Sampler =
                    NearestNeighborSampler<
                        PixelAccessor<kN32_SkColorType, kSRGB_SkGammaType>, Blender>;
                    return allocator->make<Sampler>(next, srcPixmap);
                }
                default:
                    break;
            }
        } else {
            switch (imageInfo.colorType()) {
                case kN32_SkColorType: {
                    using Sampler =
                    BilerpSampler<
                        PixelAccessor<kN32_SkColorType, kSRGB_SkGammaType>, Blender>;
                    return allocator->make<Sampler>(next, dimensions, xTile, yTile, srcPixmap);
                }
                default:
                    break;
            }
        }
    }

    auto pixelAccessor = this->choosePixelAccessor(srcPixmap, A8TintColor, allocator);
    // General cases.
    if (filterQuality == kNone_SkFilterQuality) {
        using Sampler = NearestNeighborSampler<PixelAccessorShim, Blender>;
        return allocator->make<Sampler>(next, pixelAccessor);
    } else {
        using Sampler = BilerpSampler<PixelAccessorShim, Blender>;
        return allocator->make<Sampler>(next, dimensions, xTile, yTile, pixelAccessor);
    }
}

Blender* SkLinearBitmapPipeline::chooseBlenderForShading(
    SkAlphaType alphaType,
    float postAlpha,
    SkArenaAlloc* allocator)
{
    if (alphaType == kUnpremul_SkAlphaType) {
        return allocator->make<SrcFPPixel<kUnpremul_SkAlphaType>>(postAlpha);
    } else {
        // kOpaque_SkAlphaType is treated the same as kPremul_SkAlphaType
        return allocator->make<SrcFPPixel<kPremul_SkAlphaType>>(postAlpha);
    }
}
