/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottieAnimator.h"

namespace skottie {

namespace {

SkScalar lerp_scalar(float v0, float v1, float t) {
    SkASSERT(t >= 0 && t <= 1);
    return v0 * (1 - t) + v1 * t;
}

} // namespace

bool KeyframeIntervalBase::parse(const Json::Value& k, KeyframeIntervalBase* prev) {
    SkASSERT(k.isObject());

    fT0 = fT1 = ParseScalar(k["t"], SK_ScalarMin);
    if (fT0 == SK_ScalarMin) {
        return false;
    }

    if (prev) {
        if (prev->fT1 >= fT0) {
            LOG("!! Dropping out-of-order key frame (t: %f < t: %f)\n", fT0, prev->fT1);
            return false;
        }
        // Back-fill t1 in prev interval.  Note: we do this even if we end up discarding
        // the current interval (to support "t"-only final frames).
        prev->fT1 = fT0;
    }

    // default is linear lerp
    static constexpr SkPoint kDefaultC0 = { 0, 0 },
                             kDefaultC1 = { 1, 1 };
    const auto c0 = ParsePoint(k["i"], kDefaultC0),
               c1 = ParsePoint(k["o"], kDefaultC1);

    if (c0 != kDefaultC0 || c1 != kDefaultC1) {
        fCubicMap = skstd::make_unique<SkCubicMap>();
        // TODO: why do we have to plug these inverted?
        fCubicMap->setPts(c1, c0);
    }

    return true;
}

float KeyframeIntervalBase::localT(float t) const {
    SkASSERT(this->isValid());
    auto lt = (t - fT0) / (fT1 - fT0);

    if (fCubicMap) {
        lt = fCubicMap->computeYFromX(lt);
    }

    return SkTPin<float>(lt, 0, 1);
}

template <>
void KeyframeInterval<ScalarValue>::lerp(float t, ScalarValue* v) const {
    const auto lt = this->localT(t);
    *v = lerp_scalar(fV0, fV1, lt);
}

template <>
void KeyframeInterval<VectorValue>::lerp(float t, VectorValue* v) const {
    SkASSERT(fV0.size() == fV1.size());
    SkASSERT(v->size() == 0);

    const auto lt = this->localT(t);

    v->reserve(fV0.size());
    for (size_t i = 0; i < fV0.size(); ++i) {
        v->push_back(lerp_scalar(fV0[i], fV1[i], lt));
    }
}

template <>
void KeyframeInterval<ShapeValue>::lerp(float t, ShapeValue* v) const {
    SkASSERT(fV0.countVerbs() == fV1.countVerbs());
    SkASSERT(v->isEmpty());

    const auto lt = this->localT(t);
    SkAssertResult(fV1.interpolate(fV0, lt, v));
    v->setIsVolatile(true);
}

} // namespace skottie
