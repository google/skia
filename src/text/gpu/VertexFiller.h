/*
* Copyright 2023 Google LLC
*
* Use of this source code is governed by a BSD-style license that can be
* found in the LICENSE file.
*/
#ifndef sktext_gpu_VertexFiller_DEFINED
#define sktext_gpu_VertexFiller_DEFINED

#include "include/core/SkMatrix.h"
#include "include/core/SkPoint.h"
#include "include/core/SkRect.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypes.h"
#include "include/private/SkTLogic.h"
#include "src/core/SkColorData.h"
#include "src/core/SkVx.h"

#include <cstddef>
#include <optional>
#include <tuple>

class SkReadBuffer;
class SkWriteBuffer;

namespace skgpu {
enum class MaskFormat : int;
}

namespace sktext::gpu {
class SubRunAllocator;
class AtlasSubRun;

enum FillerType {
    kIsDirect,
    kIsTransformed
};

// -- VertexFiller ---------------------------------------------------------------------------------
// The VertexFiller assumes that all points, glyph atlas entries, and bounds are created with
// respect to the CreationMatrix. This assumes that mapping any point, mask or bounds through the
// CreationMatrix will result in the proper device position. In order to draw using an arbitrary
// PositionMatrix, calculate a
//
//    viewDifference = [PositionMatrix] * [CreationMatrix] ^ -1.
//
// The viewDifference is used to map all points, masks and bounds to position to the device
// respecting the PositionMatrix.
class VertexFiller {
public:
    VertexFiller(skgpu::MaskFormat maskFormat,
                 const SkMatrix &creationMatrix,
                 SkRect creationBounds,
                 SkSpan<const SkPoint> leftTop,
                 bool canDrawDirect);

    static VertexFiller Make(skgpu::MaskFormat maskType,
                             const SkMatrix &creationMatrix,
                             SkRect creationBounds,
                             SkSpan<const SkPoint> positions,
                             SubRunAllocator *alloc,
                             FillerType fillerType);

    int unflattenSize() const { return fLeftTop.size_bytes(); }

    std::tuple<SkRect, SkMatrix> boundsAndDeviceMatrix(const SkMatrix& localToDevice,
                                                       SkPoint drawOrigin) const;

    // Return true if the positionMatrix represents an integer translation. Return the device
    // bounding box of all the glyphs. If the bounding box is empty, then something went singular
    // and this operation should be dropped.
    std::tuple<bool, SkRect> deviceRectAndCheckTransform(const SkMatrix &positionMatrix) const;

    skgpu::MaskFormat maskFormat() const { return fMaskFormat; }

    bool isLCD() const;

    int count() const { return SkCount(fLeftTop); }

    SkSpan<const SkPoint> topLefts() const { return fLeftTop; }

    bool canDrawDirect() const { return fCanDrawDirect; }

    const SkMatrix& creationMatrix() const { return fCreationMatrix; }

    std::tuple<bool, SkVector> canUseDirect(const SkMatrix& positionMatrix) const {
        return CanUseDirect(fCreationMatrix, positionMatrix);
    }
    SkMatrix viewDifference(const SkMatrix& positionMatrix) const;

private:
    friend class AtlasSubRun;

    static std::tuple<bool, SkVector> CanUseDirect(const SkMatrix& creationMatrix,
                                                   const SkMatrix& positionMatrix);

    const skgpu::MaskFormat fMaskFormat;
    const bool fCanDrawDirect;
    const SkMatrix fCreationMatrix;
    const SkRect fCreationBounds;
    const SkSpan<const SkPoint> fLeftTop;
};

}  // namespace sktext::gpu

#endif
