/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/Transform.h"

#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/sksg/include/SkSGTransform.h"

namespace skottie {
namespace internal {

TransformAdapter2D::TransformAdapter2D(const AnimationBuilder& abuilder,
                                       const skjson::ObjectValue* janchor_point,
                                       const skjson::ObjectValue* jposition,
                                       const skjson::ObjectValue* jscale,
                                       const skjson::ObjectValue* jrotation,
                                       const skjson::ObjectValue* jskew,
                                       const skjson::ObjectValue* jskew_axis)
    : INHERITED(sksg::Matrix<SkMatrix>::Make(SkMatrix::I())) {

    this->bind(abuilder, janchor_point, fAnchorPoint);
    this->bind(abuilder, jposition    , fPosition);
    this->bind(abuilder, jscale       , fScale);
    this->bind(abuilder, jrotation    , fRotation);
    this->bind(abuilder, jskew        , fSkew);
    this->bind(abuilder, jskew_axis   , fSkewAxis);
}

TransformAdapter2D::~TransformAdapter2D() {}

void TransformAdapter2D::onSync() {
    this->node()->setMatrix(this->totalMatrix());
}

SkMatrix TransformAdapter2D::totalMatrix() const {
    SkMatrix t = SkMatrix::MakeTrans(-fAnchorPoint.x, -fAnchorPoint.y);

    t.postScale(fScale.x / 100, fScale.y / 100); // 100% based
    t.postRotate(fRotation);
    t.postTranslate(fPosition.x, fPosition.y);
    // TODO: skew

    return t;
}

SkPoint TransformAdapter2D::getAnchorPoint() const {
    return { fAnchorPoint.x, fAnchorPoint.y };
}

void TransformAdapter2D::setAnchorPoint(const SkPoint& ap) {
    fAnchorPoint = { ap.x(), ap.y() };
    this->onSync();
}

SkPoint TransformAdapter2D::getPosition() const {
    return { fPosition.x, fPosition.y };
}

void TransformAdapter2D::setPosition(const SkPoint& p) {
    fPosition = { p.x(), p.y() };
    this->onSync();
}

SkVector TransformAdapter2D::getScale() const {
    return { fScale.x, fScale.y };
}

void TransformAdapter2D::setScale(const SkVector& s) {
    fScale = { s.x(), s.y() };
    this->onSync();
}

void TransformAdapter2D::setRotation(float r) {
    fRotation = r;
    this->onSync();
}

void TransformAdapter2D::setSkew(float sk) {
    fSkew = sk;
    this->onSync();
}

void TransformAdapter2D::setSkewAxis(float sa) {
    fSkewAxis = sa;
    this->onSync();
}

sk_sp<sksg::Transform> AnimationBuilder::attachMatrix2D(const skjson::ObjectValue& jtransform,
                                                        sk_sp<sksg::Transform> parent) const {
    const auto* jrotation = &jtransform["r"];
    if (jrotation->is<skjson::NullValue>()) {
        // Some 2D rotations are disguised as 3D...
        jrotation = &jtransform["rz"];
    }

    auto adapter = TransformAdapter2D::Make(*this,
                                            jtransform["a"],
                                            jtransform["p"],
                                            jtransform["s"],
                                            *jrotation,
                                            jtransform["sk"],
                                            jtransform["sa"]);
    SkASSERT(adapter);

    const auto dispatched = this->dispatchTransformProperty(adapter);

    if (adapter->isStatic()) {
        if (!dispatched && adapter->totalMatrix().isIdentity()) {
            // The transform has no observable effects - we can discard.
            return parent;
        }
        adapter->seek(0);
    } else {
        fCurrentAnimatorScope->push_back(adapter);
    }

    return sksg::Transform::MakeConcat(std::move(parent), adapter->node());
}

TransformAdapter3D::TransformAdapter3D(const skjson::ObjectValue& jtransform,
                                       const AnimationBuilder& abuilder)
    : INHERITED(sksg::Matrix<SkM44>::Make(SkM44())) {

    this->bind(abuilder, jtransform["a"], fAnchorPoint);
    this->bind(abuilder, jtransform["p"], fPosition);
    this->bind(abuilder, jtransform["s"], fScale);

    // Axis-wise rotation and orientation are mapped to the same rotation property (3D rotation).
    // The difference is in how they get interpolated (scalar/decomposed vs. vector).
    this->bind(abuilder, jtransform["rx"], fRx);
    this->bind(abuilder, jtransform["ry"], fRy);
    this->bind(abuilder, jtransform["rz"], fRz);
    this->bind(abuilder, jtransform["or"], fOrientation);
}

TransformAdapter3D::~TransformAdapter3D() = default;

void TransformAdapter3D::onSync() {
    this->node()->setMatrix(this->totalMatrix());
}

SkV3 TransformAdapter3D::anchor_point() const {
    return ValueTraits<VectorValue>::As<SkV3>(fAnchorPoint);
}

SkV3 TransformAdapter3D::position() const {
    return ValueTraits<VectorValue>::As<SkV3>(fPosition);
}

SkV3 TransformAdapter3D::rotation() const {
    // orientation and axis-wise rotation map onto the same property.
    return ValueTraits<VectorValue>::As<SkV3>(fOrientation) + SkV3{ fRx, fRy, fRz };
}

SkM44 TransformAdapter3D::totalMatrix() const {
    const auto anchor_point = this->anchor_point(),
               position     = this->position(),
               scale        = ValueTraits<VectorValue>::As<SkV3>(fScale),
               rotation     = this->rotation();

    return SkM44::Translate(position.x, position.y, position.z)
         * SkM44::Rotate({ 1, 0, 0 }, SkDegreesToRadians(rotation.x))
         * SkM44::Rotate({ 0, 1, 0 }, SkDegreesToRadians(rotation.y))
         * SkM44::Rotate({ 0, 0, 1 }, SkDegreesToRadians(rotation.z))
         * SkM44::Scale(scale.x / 100, scale.y / 100, scale.z / 100)
         * SkM44::Translate(-anchor_point.x, -anchor_point.y, -anchor_point.z);
}

sk_sp<sksg::Transform> AnimationBuilder::attachMatrix3D(const skjson::ObjectValue& jtransform,
                                                        sk_sp<sksg::Transform> parent) const {
    auto adapter = TransformAdapter3D::Make(jtransform, *this);
    SkASSERT(adapter);

    if (adapter->isStatic()) {
        // TODO: SkM44::isIdentity?
        if (adapter->totalMatrix() == SkM44()) {
            // The transform has no observable effects - we can discard.
            return parent;
        }
        adapter->seek(0);
    } else {
        fCurrentAnimatorScope->push_back(adapter);
    }

    return sksg::Transform::MakeConcat(std::move(parent), adapter->node());
}

} // namespace internal
} // namespace skottie
