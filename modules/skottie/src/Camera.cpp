/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/Camera.h"

#include "include/utils/Sk3D.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/sksg/include/SkSGTransform.h"

namespace skottie {
namespace internal {

namespace  {

SkMatrix44 ComputeCameraMatrix(const SkPoint3& position,
                               const SkPoint3& poi,
                               const SkPoint3& rotation,
                               const SkSize& viewport_size,
                               float zoom) {
    const auto pos = SkPoint3{ position.fX, position.fY, -position.fZ },
                up = SkPoint3{           0,           1,            0 };

    // Initial camera vector.
    SkMatrix44 cam_t;
    Sk3LookAt(&cam_t, pos, poi, up);

    // Rotation origin is camera position.
    {
        SkMatrix44 rot;
        rot.setRotateDegreesAbout(1, 0, 0,  rotation.fX);
        cam_t.postConcat(rot);
        rot.setRotateDegreesAbout(0, 1, 0,  rotation.fY);
        cam_t.postConcat(rot);
        rot.setRotateDegreesAbout(0, 0, 1, -rotation.fZ);
        cam_t.postConcat(rot);
    }

    // Flip world Z, as it is opposite of what Sk3D expects.
    cam_t.preScale(1, 1, -1);

    // View parameters:
    //
    //   * size     -> composition size (TODO: AE seems to base it on width only?)
    //   * distance -> "zoom" camera attribute
    //
    const auto view_size     = SkTMax(viewport_size.width(), viewport_size.height()),
               view_distance = zoom,
               view_angle    = std::atan(sk_ieee_float_divide(view_size * 0.5f, view_distance));

    SkMatrix44 persp_t;
    Sk3Perspective(&persp_t, 0, view_distance, 2 * view_angle);
    persp_t.postScale(view_size * 0.5f, view_size * 0.5f, 1);

    SkMatrix44 m;
    m.setTranslate(viewport_size.width() * 0.5f, viewport_size.height() * 0.5f, 0);
    m.preConcat(persp_t);
    m.preConcat(cam_t);

    return m;
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

SkMatrix44 CameraAdaper::totalMatrix() const {
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

SkPoint3 CameraAdaper::poi(const SkPoint3& pos) const {
    // AE supports two camera types:
    //
    //   - one-node camera: does not auto-orient, and starts off perpendicular
    //     to the z = 0 plane, facing "forward" (decreasing z).
    //
    //   - two-node camera: has a point of interest (encoded as the anchor point),
    //     and auto-orients to point in its direction.

    if (fType == CameraType::kOneNode) {
        return { pos.fX, pos.fY, -pos.fZ - 1};
    }

    const auto ap = this->anchor_point();

    return { ap.fX, ap.fY, -ap.fZ };
}

sk_sp<sksg::Transform> CameraAdaper::DefaultCameraTransform(const SkSize& viewport_size) {
    const auto center = SkVector::Make(viewport_size.width()  * 0.5f,
                                       viewport_size.height() * 0.5f);

    static constexpr float kDefaultAEZoom = 879.13f;

    const SkPoint3 pos = { center.fX, center.fY, -kDefaultAEZoom },
                   poi = {    pos.fX,    pos.fY,     -pos.fZ - 1 },
                   rot = {         0,         0,               0 };

    return sksg::Matrix<SkMatrix44>::Make(
                ComputeCameraMatrix(pos, poi, rot, viewport_size, kDefaultAEZoom));
}

sk_sp<sksg::Transform> AnimationBuilder::attachCamera(const skjson::ObjectValue& jlayer,
                                                      const skjson::ObjectValue& jtransform,
                                                      sk_sp<sksg::Transform> parent,
                                                      const SkSize& viewport_size) const {
    auto adapter = sk_make_sp<CameraAdaper>(jlayer, jtransform, *this, viewport_size);

    if (adapter->isStatic()) {
        adapter->tick(0);
    } else {
        fCurrentAnimatorScope->push_back(adapter);
    }

    return sksg::Transform::MakeConcat(adapter->node(), std::move(parent));
}

} // namespace internal
} // namespace skottie
