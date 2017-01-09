/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLinearBitmapPipeline_DEFINED
#define SkLinearBitmapPipeline_DEFINED

#include "SkArenaAlloc.h"
#include "SkColor.h"
#include "SkImageInfo.h"
#include "SkMatrix.h"
#include "SkShader.h"

class SkEmbeddableLinearPipeline;

enum SkGammaType {
    kLinear_SkGammaType,
    kSRGB_SkGammaType,
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// SkLinearBitmapPipeline - encapsulates all the machinery for doing floating point pixel
// processing in a linear color space.
// Note: this class has unusual alignment requirements due to its use of SIMD instructions. The
// class SkEmbeddableLinearPipeline below manages these requirements.
class SkLinearBitmapPipeline {
public:
    SkLinearBitmapPipeline(
        const SkMatrix& inverse,
        SkFilterQuality filterQuality,
        SkShader::TileMode xTile, SkShader::TileMode yTile,
        SkColor paintColor,
        const SkPixmap& srcPixmap,
        SkArenaAlloc* allocator);

    SkLinearBitmapPipeline(
        const SkLinearBitmapPipeline& pipeline,
        const SkPixmap& srcPixmap,
        SkBlendMode,
        const SkImageInfo& dstInfo,
        SkArenaAlloc* allocator);

    static SkLinearBitmapPipeline* ClonePipelineForBlitting(
        const SkLinearBitmapPipeline& pipeline,
        SkMatrix::TypeMask matrixMask,
        SkFilterQuality filterQuality,
        const SkPixmap& srcPixmap,
        float finalAlpha,
        SkBlendMode,
        const SkImageInfo& dstInfo,
        SkArenaAlloc* allocator);

    ~SkLinearBitmapPipeline();

    void shadeSpan4f(int x, int y, SkPM4f* dst, int count);
    void blitSpan(int32_t x, int32_t y, void* dst, int count);

    class PointProcessorInterface;
    class SampleProcessorInterface;
    class BlendProcessorInterface;
    class DestinationInterface;
    class PixelAccessorInterface;

    using MatrixCloner =
        std::function<PointProcessorInterface* (PointProcessorInterface*, SkArenaAlloc*)>;
    using TilerCloner =
        std::function<PointProcessorInterface* (SampleProcessorInterface*, SkArenaAlloc*)>;

    PointProcessorInterface* chooseMatrix(
        PointProcessorInterface* next,
        const SkMatrix& inverse,
        SkArenaAlloc* allocator);

    template <typename Tiler>
    PointProcessorInterface* createTiler(SampleProcessorInterface* next, SkISize dimensions,
                                         SkArenaAlloc* allocator);

    template <typename XStrategy>
    PointProcessorInterface* chooseTilerYMode(
        SampleProcessorInterface* next, SkShader::TileMode yMode, SkISize dimensions,
        SkArenaAlloc* allocator);

    PointProcessorInterface* chooseTiler(
        SampleProcessorInterface* next,
        SkISize dimensions,
        SkShader::TileMode xMode, SkShader::TileMode yMode,
        SkFilterQuality filterQuality,
        SkScalar dx,
        SkArenaAlloc* allocator);

    template <SkColorType colorType>
    PixelAccessorInterface* chooseSpecificAccessor(const SkPixmap& srcPixmap,
                                                   SkArenaAlloc* allocator);

    PixelAccessorInterface* choosePixelAccessor(
        const SkPixmap& srcPixmap,
        const SkColor A8TintColor,
        SkArenaAlloc* allocator);

    SampleProcessorInterface* chooseSampler(
        BlendProcessorInterface* next,
        SkFilterQuality filterQuality,
        SkShader::TileMode xTile, SkShader::TileMode yTile,
        const SkPixmap& srcPixmap,
        const SkColor A8TintColor,
        SkArenaAlloc* allocator);

    BlendProcessorInterface* chooseBlenderForShading(
        SkAlphaType alphaType,
        float postAlpha,
        SkArenaAlloc* allocator);

    PointProcessorInterface* fFirstStage;
    MatrixCloner             fMatrixStageCloner;
    TilerCloner              fTileStageCloner;
    DestinationInterface*    fLastStage;
};

#endif  // SkLinearBitmapPipeline_DEFINED
