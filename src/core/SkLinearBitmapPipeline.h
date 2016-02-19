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

using Sk4fArg = const Sk4f&;

class PointProcessorInterface {
public:
    virtual ~PointProcessorInterface() { }
    virtual void pointListFew(int n, Sk4fArg xs, Sk4fArg ys) = 0;
    virtual void pointList4(Sk4fArg xs, Sk4fArg ys) = 0;

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

class BilerpProcessorInterface : public PointProcessorInterface {
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
    virtual void bilerpList(Sk4fArg xs, Sk4fArg ys) = 0;
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
        SkFilterQuality filterQuality,
        SkShader::TileMode xTile, SkShader::TileMode yTile,
        const SkPixmap& srcPixmap);

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
