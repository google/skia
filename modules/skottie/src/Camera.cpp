/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/Camera.h"

#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/sksg/include/SkSGTransform.h"

namespace skottie {
namespace internal {

namespace  {

SkM44 ComputeCameraMatrix(const SkV3& position,
                          const SkV3& poi,
                          const SkV3& rotation,
                          const SkSize& viewport_size,
                          float zoom) {

    // Initial camera vector.
    const auto cam_t = SkM44::Rotate({0, 0, 1}, SkDegreesToRadians(-rotation.z))
                     * SkM44::Rotate({0, 1, 0}, SkDegreesToRadians( rotation.y))
                     * SkM44::Rotate({1, 0, 0}, SkDegreesToRadians( rotation.x))
                     * Sk3LookAt({ position.x, position.y, -position.z },
                                 {      poi.x,      poi.y,       poi.z },
                                 {          0,          1,           0 })
                     * SkM44::Scale(1, 1, -1);

    // View parameters:
    //
    //   * size     -> composition size (TODO: AE seems to base it on width only?)
    //   * distance -> "zoom" camera attribute
    //
    const auto view_size     = std::max(viewport_size.width(), viewport_size.height()),
               view_distance = zoom,
               view_angle    = std::atan(sk_ieee_float_divide(view_size * 0.5f, view_distance));

    const auto persp_t = SkM44::Scale(view_size * 0.5f, view_size * 0.5f, 1)
                       * Sk3Perspective(0, view_distance, 2 * view_angle);

    return SkM44::Translate(viewport_size.width()  * 0.5f,
                            viewport_size.height() * 0.5f,
                            0)
           * persp_t * cam_t;
}

} // namespace

CameraAdaper::CameraAdaper(const skjson::ObjectValue& jlayer,
                           const skjson::ObjectValue& jtransform,
                           const AnimationBuilder& abuilder,
                           const SkSize& viewport_size)
    : INHERITED(jtransform, abuilder)
    , fViewportSize(viewport_size)
    // The presence of an anchor point property ('a') differentiates
    // one-node vs. two-node cameras.
    , fType(jtransform["a"].is<skjson::NullValue>() ? CameraType::kOneNode
                                                    : CameraType::kTwoNode) {
    // 'pe' (perspective?) corresponds to AE's "zoom" camera property.
    this->bind(abuilder, jlayer["pe"], fZoom);
}

CameraAdaper::~CameraAdaper() = default;

SkM44 CameraAdaper::totalMatrix() const {
    // Camera parameters:
    //
    //   * location          -> position attribute
    //   * point of interest -> anchor point attribute (two-node camera only)
    //   * orientation       -> rotation attribute
    //
    const auto position = this->position();

    return ComputeCameraMatrix(position,
                               this->poi(position),
                               this->rotation(),
                               fViewportSize,
                               fZoom);
}

SkV3 CameraAdaper::poi(const SkV3& pos) const {
    // AE supports two camera types:
    //
    //   - one-node camera: does not auto-orient, and starts off perpendicular
    //     to the z = 0 plane, facing "forward" (decreasing z).
    //
    //   - two-node camera: has a point of interest (encoded as the anchor point),
    //     and auto-orients to point in its direction.

    if (fType == CameraType::kOneNode) {
        return { pos.x, pos.y, -pos.z - 1};
    }

    const auto ap = this->anchor_point();

    return { ap.x, ap.y, -ap.z };
}

sk_sp<sksg::Transform> CameraAdaper::DefaultCameraTransform(const SkSize& viewport_size) {
    const auto center = SkVector::Make(viewport_size.width()  * 0.5f,
                                       viewport_size.height() * 0.5f);

    static constexpr float kDefaultAEZoom = 879.13f;

    const SkV3 pos = { center.fX, center.fY, -kDefaultAEZoom },
               poi = {     pos.x,     pos.y,      -pos.z - 1 },
               rot = {         0,         0,               0 };

    return sksg::Matrix<SkM44>::Make(
                ComputeCameraMatrix(pos, poi, rot, viewport_size, kDefaultAEZoom));
}

sk_sp<sksg::Transform> AnimationBuilder::attachCamera(const skjson::ObjectValue& jlayer,
                                                      const skjson::ObjectValue& jtransform,
                                                      sk_sp<sksg::Transform> parent,
                                                      const SkSize& viewport_size) const {
    auto adapter = sk_make_sp<CameraAdaper>(jlayer, jtransform, *this, viewport_size);

    if (adapter->isStatic()) {
        adapter->seek(0);
    } else {
        fCurrentAnimatorScope->push_back(adapter);
    }

    return sksg::Transform::MakeConcat(adapter->node(), std::move(parent));
}

} // namespace internal
} // namespace skottie
