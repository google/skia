/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_AnalyticBlurMask_DEFINED
#define skgpu_graphite_geom_AnalyticBlurMask_DEFINED

#include "include/core/SkM44.h"
#include "include/core/SkRefCnt.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/geom/Rect.h"

#include <optional>
#include <utility>

class SkMatrix;
class SkRRect;
struct SkRect;

namespace skgpu::graphite {

class Recorder;
class Transform;

/**
 * AnalyticBlurMask holds the shader inputs used to do an analytic blur over rects, rrects, or
 * circles.
 */
class AnalyticBlurMask {
public:
    enum class ShapeType {
        kRect,
        kRRect,
        kCircle,
    };

    static_assert(0 == static_cast<int>(ShapeType::kRect),
                  "Blur shader code depends on AnalyticBlurMask::ShapeType");
    static_assert(1 == static_cast<int>(ShapeType::kRRect),
                  "Blur shader code depends on AnalyticBlurMask::ShapeType");
    static_assert(2 == static_cast<int>(ShapeType::kCircle),
                  "Blur shader code depends on AnalyticBlurMask::ShapeType");

    AnalyticBlurMask() = delete;

    static std::optional<AnalyticBlurMask> Make(Recorder*,
                                                const Transform& localToDevice,
                                                float deviceSigma,
                                                const SkRRect& srcRRect);

    const Rect& drawBounds() const { return fDrawBounds; }
    const SkM44& deviceToScaledShape() const { return fDevToScaledShape; }
    const Rect& shapeData() const { return fShapeData; }
    ShapeType shapeType() const { return fShapeType; }
    const SkV2& blurData() const { return fBlurData; }
    sk_sp<TextureProxy> refProxy() const { return fProxy; }

private:
    AnalyticBlurMask(const Rect& drawBounds,
                     const SkM44& devToScaledShape,
                     ShapeType shapeType,
                     const Rect& shapeData,
                     const SkV2& blurData,
                     sk_sp<TextureProxy> proxy)
            : fDrawBounds(drawBounds)
            , fDevToScaledShape(devToScaledShape)
            , fShapeData(shapeData)
            , fBlurData(blurData)
            , fShapeType(shapeType)
            , fProxy(std::move(proxy)) {}

    static std::optional<AnalyticBlurMask> MakeRect(Recorder*,
                                                    const SkMatrix& localToDevice,
                                                    float devSigma,
                                                    const SkRect& srcRect);

    static std::optional<AnalyticBlurMask> MakeCircle(Recorder*,
                                                      const SkMatrix& localToDevice,
                                                      float devSigma,
                                                      const SkRect& srcRect,
                                                      const SkRect& devRect);

    static std::optional<AnalyticBlurMask> MakeRRect(Recorder* recorder,
                                                     const SkMatrix& localToDevice,
                                                     float devSigma,
                                                     const SkRRect& srcRRect,
                                                     const SkRRect& devRRect);

    // Draw bounds in local space.
    Rect fDrawBounds;

    // Transforms device-space coordinates to the shape data's space.
    SkM44 fDevToScaledShape;

    // Shape data, which can define a rectangle, circle, or rounded rectangle.
    // This data is in a local space defined by the concatenation of the local-to-device matrix and
    // fDevToScaledShape.
    Rect fShapeData;

    // "fBlurData" holds different data depending on the shape type, for the unique needs of the
    // shape types' respective shaders.
    // In the rectangle case, it holds:
    //   x = a boolean indicating whether we can use a fast path for sampling the blur integral
    //       because the rectangle is larger than 6*sigma in both dimensions, and
    //   y = the value "1 / (6*sigma)".
    // In the rounded rectangle case, it holds:
    //   x = the size of the blurred edge, defined as "2*blurRadius + cornerRadius", and
    //   y is unused.
    // In the circle case, this data is unused.
    SkV2 fBlurData;

    ShapeType fShapeType;
    sk_sp<TextureProxy> fProxy;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_geom_AnalyticBlurMask_DEFINED
