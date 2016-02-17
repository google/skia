/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkLinearBitmapPipeline_DEFINED
#define SkLinearBitmapPipeline_DEFINED

#include <algorithm>
#include <cmath>
#include <limits>
#include <cstdio>
#include "SkColor.h"
#include "SkImageInfo.h"
#include "SkMatrix.h"
#include "SkShader.h"
#include "SkSize.h"
#include "SkNx.h"

using Sk4fArg = const Sk4f&;

class PointProcessorInterface {
public:
    virtual ~PointProcessorInterface() { }
    virtual void pointListFew(int n, Sk4fArg xs, Sk4fArg ys) = 0;
    virtual void pointList4(Sk4fArg xs, Sk4fArg ys) = 0;
};

class PixelPlacerInterface {
public:
    virtual ~PixelPlacerInterface() { }
    virtual void setDestination(SkPM4f* dst) = 0;
    virtual void placePixel(Sk4fArg pixel0) = 0;
    virtual void place4Pixels(Sk4fArg p0, Sk4fArg p1, Sk4fArg p2, Sk4fArg p3) = 0;
};

class SkLinearBitmapPipeline {
public:
    SkLinearBitmapPipeline(
        const SkMatrix& inverse,
        SkShader::TileMode xTile, SkShader::TileMode yTile,
        const SkImageInfo& srcImageInfo,
        const void* srcImageData);

    void shadeSpan4f(int x, int y, SkPM4f* dst, int count);

    template<typename Base, size_t kSize>
    class PolymorphicUnion {
    public:
        PolymorphicUnion() {}

        ~PolymorphicUnion() { get()->~Base(); }

        template<typename Variant, typename... Args>
        void Initialize(Args&&... args) {
            SkASSERTF(sizeof(Variant) <= sizeof(fSpace),
                      "Size Variant: %d, Space: %d", sizeof(Variant), sizeof(fSpace));

            new(&fSpace) Variant(std::forward<Args>(args)...);
        };

        Base* get() const { return reinterpret_cast<Base*>(&fSpace); }
        Base* operator->() const { return get(); }
        Base& operator*() const { return *get(); }

    private:
        struct SK_STRUCT_ALIGN(16) Space {
            char space[kSize];
        };
        mutable Space fSpace;
    };

    using MatrixStage = PolymorphicUnion<PointProcessorInterface, 112>;
    using TileStage   = PolymorphicUnion<PointProcessorInterface,  96>;
    using SampleStage = PolymorphicUnion<PointProcessorInterface,  80>;
    using PixelStage  = PolymorphicUnion<PixelPlacerInterface,     80>;

private:
    PointProcessorInterface* fFirstStage;
    MatrixStage fMatrixStage;
    TileStage   fTileXOrBothStage;
    TileStage   fTileYStage;
    SampleStage fSampleStage;
    PixelStage  fPixelStage;
};

#endif  // SkLinearBitmapPipeline_DEFINED
