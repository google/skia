/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLinearBitmapPipeline_DEFINED
#define SkLinearBitmapPipeline_DEFINED

#include "SkColor.h"
#include "SkImageInfo.h"
#include "SkMatrix.h"
#include "SkShader.h"
#include "SkSmallAllocator.h"

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
        const SkPixmap& srcPixmap);

    SkLinearBitmapPipeline(
        const SkLinearBitmapPipeline& pipeline,
        const SkPixmap& srcPixmap,
        SkBlendMode,
        const SkImageInfo& dstInfo);

    static bool ClonePipelineForBlitting(
        SkEmbeddableLinearPipeline* pipelineStorage,
        const SkLinearBitmapPipeline& pipeline,
        SkMatrix::TypeMask matrixMask,
        SkShader::TileMode xTileMode,
        SkShader::TileMode yTileMode,
        SkFilterQuality filterQuality,
        const SkPixmap& srcPixmap,
        float finalAlpha,
        SkBlendMode,
        const SkImageInfo& dstInfo);

    ~SkLinearBitmapPipeline();

    void shadeSpan4f(int x, int y, SkPM4f* dst, int count);
    void blitSpan(int32_t x, int32_t y, void* dst, int count);

    class PointProcessorInterface;
    class SampleProcessorInterface;
    class BlendProcessorInterface;
    class DestinationInterface;
    class PixelAccessorInterface;

private:
    using MemoryAllocator = SkSmallAllocator<5, 256>;
    using MatrixCloner =
        std::function<PointProcessorInterface* (PointProcessorInterface*, MemoryAllocator*)>;
    using TilerCloner =
        std::function<PointProcessorInterface* (SampleProcessorInterface*, MemoryAllocator*)>;

    PointProcessorInterface* chooseMatrix(
        PointProcessorInterface* next,
        const SkMatrix& inverse);

    template <typename Tiler>
    PointProcessorInterface* createTiler(SampleProcessorInterface* next, SkISize dimensions);

    template <typename XStrategy>
    PointProcessorInterface* chooseTilerYMode(
        SampleProcessorInterface* next, SkShader::TileMode yMode, SkISize dimensions);

    PointProcessorInterface* chooseTiler(
        SampleProcessorInterface* next,
        SkISize dimensions,
        SkShader::TileMode xMode, SkShader::TileMode yMode,
        SkFilterQuality filterQuality,
        SkScalar dx);

    template <SkColorType colorType>
    PixelAccessorInterface* chooseSpecificAccessor(const SkPixmap& srcPixmap);

    PixelAccessorInterface* choosePixelAccessor(
        const SkPixmap& srcPixmap,
        const SkColor A8TintColor);

    SampleProcessorInterface* chooseSampler(
        BlendProcessorInterface* next,
        SkFilterQuality filterQuality,
        SkShader::TileMode xTile, SkShader::TileMode yTile,
        const SkPixmap& srcPixmap,
        const SkColor A8TintColor);

    BlendProcessorInterface* chooseBlenderForShading(
        SkAlphaType alphaType,
        float postAlpha);

    MemoryAllocator          fMemory;
    PointProcessorInterface* fFirstStage;
    MatrixCloner             fMatrixStageCloner;
    TilerCloner              fTileStageCloner;
    DestinationInterface*    fLastStage;
};

////////////////////////////////////////////////////////////////////////////////////////////////////
// SkEmbeddableLinearPipeline - manage stricter alignment needs for SkLinearBitmapPipeline.
class SkEmbeddableLinearPipeline {
public:
    SkEmbeddableLinearPipeline() { }
    ~SkEmbeddableLinearPipeline() {
        if (fInitialized) {
            get()->~SkLinearBitmapPipeline();
        }
    }

    template <typename... Args>
    void init(Args&&... args) {
        new (fPipelineStorage) SkLinearBitmapPipeline{std::forward<Args>(args)...};
        fInitialized = true;
    }

    SkLinearBitmapPipeline* get() const {
        return reinterpret_cast<SkLinearBitmapPipeline*>(fPipelineStorage);
    }
    SkLinearBitmapPipeline& operator*()  const { return *this->get(); }
    SkLinearBitmapPipeline* operator->() const { return  this->get(); }

private:
    alignas(SkLinearBitmapPipeline) mutable char fPipelineStorage[sizeof(SkLinearBitmapPipeline)];
    bool                                         fInitialized {false};
};

#endif  // SkLinearBitmapPipeline_DEFINED
