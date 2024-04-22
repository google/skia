/*
 * Copyright 2024 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef skgpu_graphite_geom_AnalyticBlurMask_DEFINED
#define skgpu_graphite_geom_AnalyticBlurMask_DEFINED

#include "include/core/SkM44.h"
#include "src/gpu/graphite/TextureProxy.h"
#include "src/gpu/graphite/geom/Rect.h"

#include <optional>

class SkMatrix;
class SkRRect;

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
    bool isFast() const { return fIsFast; }
    float invSixSigma() const { return fInvSixSigma; }
    sk_sp<TextureProxy> refProxy() const { return fProxy; }

private:
    AnalyticBlurMask(const Rect& drawBounds,
                     const SkM44& devToScaledShape,
                     const Rect& shapeData,
                     ShapeType shapeType,
                     bool isFast,
                     float invSixSigma,
                     sk_sp<TextureProxy> proxy)
            : fDrawBounds(drawBounds)
            , fDevToScaledShape(devToScaledShape)
            , fShapeData(shapeData)
            , fShapeType(shapeType)
            , fIsFast(isFast)
            , fInvSixSigma(invSixSigma)
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

    // Draw bounds in local space.
    Rect fDrawBounds;

    // Transforms device-space coordinates to the shape data's space.
    SkM44 fDevToScaledShape;

    // Shape data, which can define a rectangle, circle, or rounded rectangle.
    // This data is in a local space defined by the concatenation of the local-to-device matrix and
    // fDevToScaledShape.
    Rect fShapeData;

    ShapeType fShapeType;
    bool fIsFast;
    float fInvSixSigma;
    sk_sp<TextureProxy> fProxy;
};

}  // namespace skgpu::graphite

#endif  // skgpu_graphite_geom_AnalyticBlurMask_DEFINED
