/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieShapeLayer_DEFINED
#define SkottieShapeLayer_DEFINED

#include "include/private/SkNoncopyable.h"
#include "modules/skottie/src/animator/Animator.h"
#include "modules/sksg/include/SkSGMerge.h"

#include <vector>

namespace skjson {

class ObjectValue;

} // namespace skjson

namespace sksg {

class GeometryNode;
class PaintNode;
class RenderNode;

} // namespace sksg

namespace skottie {
namespace internal {

class AnimationBuilder;

// TODO/TRANSITIONAL: not much state here yet, but will eventually hold ShapeLayer-related stuff.
class ShapeBuilder final : SkNoncopyable {
public:
    static sk_sp<sksg::Merge> MergeGeometry(std::vector<sk_sp<sksg::GeometryNode>>&&,
                                            sksg::Merge::Mode);

    static sk_sp<sksg::GeometryNode> AttachPathGeometry(const skjson::ObjectValue&,
                                                        const AnimationBuilder*);
    static sk_sp<sksg::GeometryNode> AttachRRectGeometry(const skjson::ObjectValue&,
                                                         const AnimationBuilder*);
    static sk_sp<sksg::GeometryNode> AttachEllipseGeometry(const skjson::ObjectValue&,
                                                           const AnimationBuilder*);
    static sk_sp<sksg::GeometryNode> AttachPolystarGeometry(const skjson::ObjectValue&,
                                                            const AnimationBuilder*);

    static sk_sp<sksg::PaintNode> AttachColorFill(const skjson::ObjectValue&,
                                                  const AnimationBuilder*);
    static sk_sp<sksg::PaintNode> AttachColorStroke(const skjson::ObjectValue&,
                                                    const AnimationBuilder*);
    static sk_sp<sksg::PaintNode> AttachGradientFill(const skjson::ObjectValue&,
                                                     const AnimationBuilder*);
    static sk_sp<sksg::PaintNode> AttachGradientStroke(const skjson::ObjectValue&,
                                                       const AnimationBuilder*);

    static std::vector<sk_sp<sksg::GeometryNode>> AttachMergeGeometryEffect(
            const skjson::ObjectValue&, const AnimationBuilder*,
            std::vector<sk_sp<sksg::GeometryNode>>&&);
    static std::vector<sk_sp<sksg::GeometryNode>> AttachTrimGeometryEffect(
            const skjson::ObjectValue&,
            const AnimationBuilder*,
            std::vector<sk_sp<sksg::GeometryNode>>&&);
    static std::vector<sk_sp<sksg::GeometryNode>> AttachRoundGeometryEffect(
            const skjson::ObjectValue&, const AnimationBuilder*,
            std::vector<sk_sp<sksg::GeometryNode>>&&);
    static std::vector<sk_sp<sksg::GeometryNode>> AdjustStrokeGeometry(
            const skjson::ObjectValue&, const AnimationBuilder*,
            std::vector<sk_sp<sksg::GeometryNode>>&&);

    static std::vector<sk_sp<sksg::RenderNode>> AttachRepeaterDrawEffect(
            const skjson::ObjectValue&,
            const AnimationBuilder*,
            std::vector<sk_sp<sksg::RenderNode>>&&);

private:
    static sk_sp<sksg::PaintNode> AttachFill(const skjson::ObjectValue&,
                                             const AnimationBuilder*,
                                             sk_sp<sksg::PaintNode>,
                                             sk_sp<AnimatablePropertyContainer> = nullptr);
    static sk_sp<sksg::PaintNode> AttachStroke(const skjson::ObjectValue&,
                                               const AnimationBuilder*,
                                               sk_sp<sksg::PaintNode>,
                                               sk_sp<AnimatablePropertyContainer> = nullptr);
};

} // namespace internal
} // namespace skottie

#endif // SkottieShapeLayer_DEFINED
