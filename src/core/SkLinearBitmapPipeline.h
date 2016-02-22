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
#include "SkNx.h"
#include "SkShader.h"

class SkLinearBitmapPipeline {
public:
    SkLinearBitmapPipeline(
        const SkMatrix& inverse,
        SkFilterQuality filterQuality,
        SkShader::TileMode xTile, SkShader::TileMode yTile,
        const SkPixmap& srcPixmap);
    ~SkLinearBitmapPipeline();

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

    class PointProcessorInterface;
    class BilerpProcessorInterface;
    class PixelPlacerInterface;

    using MatrixStage = PolymorphicUnion<PointProcessorInterface, 112>;
    using FilterStage = PolymorphicUnion<PointProcessorInterface,   8>;
    using TileStage   = PolymorphicUnion<BilerpProcessorInterface, 96>;
    using SampleStage = PolymorphicUnion<BilerpProcessorInterface, 80>;
    using PixelStage  = PolymorphicUnion<PixelPlacerInterface,     80>;

private:
    PointProcessorInterface* fFirstStage;
    MatrixStage fMatrixStage;
    FilterStage fFilterStage;
    TileStage   fTileXOrBothStage;
    TileStage   fTileYStage;
    SampleStage fSampleStage;
    PixelStage  fPixelStage;
};

#endif  // SkLinearBitmapPipeline_DEFINED
