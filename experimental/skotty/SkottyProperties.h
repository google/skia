/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottyProperties_DEFINED
#define SkottyProperties_DEFINED

#include "SkPoint.h"
#include "SkottyPriv.h"
#include "SkTArray.h"
#include "SkTypes.h"

#include <memory>

class SkPath;

namespace  skotty {

struct BezierVertex {
    SkPoint fInPoint,  // "in" control point, relative to the vertex
            fOutPoint, // "out" control point, relative to the vertex
            fVertex;
};

struct ScalarValue {
    float fVal;

    static bool Parse(const Json::Value&, ScalarValue*);

    ScalarValue() : fVal(0) {}
    explicit ScalarValue(SkScalar v) : fVal(v) {}

    ScalarValue& operator=(SkScalar v) { fVal = v; return *this; }

    operator SkScalar() const { return fVal; }

    constexpr size_t cardinality() const { return 1; }

    template <typename T>
    T as() const;
};

template <>
inline SkScalar ScalarValue::as<SkScalar>() const {
    return fVal;
}

struct VectorValue {
    SkTArray<ScalarValue, true> fVals;

    static bool Parse(const Json::Value&, VectorValue*);

    VectorValue()                               = default;
    VectorValue(const VectorValue&)             = delete;
    VectorValue(VectorValue&&)                  = default;
    VectorValue& operator==(const VectorValue&) = delete;

    size_t cardinality() const { return SkTo<size_t>(fVals.count()); }

    template <typename T>
    T as() const;
};

struct ShapeValue {
    SkTArray<BezierVertex, true> fVertices;
    bool                         fClose = false;

    ShapeValue()                              = default;
    ShapeValue(const ShapeValue&)             = delete;
    ShapeValue(ShapeValue&&)                  = default;
    ShapeValue& operator==(const ShapeValue&) = delete;

    static bool Parse(const Json::Value&, ShapeValue*);

    size_t cardinality() const { return SkTo<size_t>(fVertices.count()); }

    template <typename T>
    T as() const;
};

template <typename T>
class Keyframed final : public SkNoncopyable {
public:
    static std::unique_ptr<Keyframed<T>> Make(const Json::Value& frames);

    void eval(float t, T*) const;

private:
    struct Keyframe {
        Keyframe(T&& s, T&& e, float t)
            : fV0(std::move(s))
            , fV1(std::move(e))
            , fT0(t)
            , fT1(t) {}

        void lerp(float t, T*) const;

        T     fV0,
              fV1;
        float fT0,
              fT1;
    };

    explicit Keyframed(SkTArray<Keyframe>&& frames) : fFrames(std::move(frames)) {}

    const Keyframe& findFrame(float t) const;

    const SkTArray<Keyframe> fFrames;

    using INHERITED = SkNoncopyable;
};

template <typename T>
std::unique_ptr<Keyframed<T>> Keyframed<T>::Make(const Json::Value& frames) {
    if (!frames.isArray())
        return nullptr;

    SkTArray<Keyframe> key_frames;
    for (Json::ArrayIndex i = 0; i < frames.size(); ++i) {
        const auto& frame = frames[i];
        if (!frame.isObject())
            return nullptr;

        const auto t = ParseScalar(frame["t"], SK_ScalarMin);
        if (t == SK_ScalarMin)
            break;

        if (!key_frames.empty()) {
            auto& prev_frame = key_frames.back();
            if (prev_frame.fT0 >= t) {
                LOG("!! Ignoring out-of-order key frame (t: %f < t: %f)\n", t, prev_frame.fT0);
                continue;
            }
            prev_frame.fT1 = t;
        }

        T v0, v1;
        if (!T::Parse(frame["s"], &v0) ||
            !T::Parse(frame["e"], &v1) ||
            v0.cardinality() != v1.cardinality() ||
            (!key_frames.empty() && v0.cardinality() != key_frames.back().fV0.cardinality())) {
            continue;
        }

        key_frames.emplace_back(std::move(v0), std::move(v1), t);
    }

    // If we couldn't determine a t1 for the last frame, discard it.
    if (!key_frames.empty() && key_frames.back().fT0 == key_frames.back().fT1) {
        key_frames.pop_back();
    }

    return key_frames.empty()
        ? nullptr : std::unique_ptr<Keyframed<T>>(new Keyframed(std::move(key_frames)));
}

template <typename T>
void Keyframed<T>::eval(float t, T* result) const {
    const auto& frame = this->findFrame(t);
    const auto rel_t = (t - frame.fT0) / (frame.fT1 - frame.fT0);

    frame.lerp(SkTPin<float>(rel_t, 0, 1), result);
}

template <typename T>
const typename Keyframed<T>::Keyframe& Keyframed<T>::findFrame(float t) const {
    SkASSERT(!fFrames.empty());

    // TODO: cache last/current frame?

    auto f0 = fFrames.begin(),
         f1 = fFrames.end() - 1;

    SkASSERT(f0->fT0 < f0->fT1);
    SkASSERT(f1->fT0 < f1->fT1);

    if (t < f0->fT0) {
        return *f0;
    }

    if (t > f1->fT1) {
        return *f1;
    }

    while (f0 != f1) {
        SkASSERT(f0 < f1);
        SkASSERT(t >= f0->fT0 && t <= f1->fT1);

        const auto f = f0 + (f1 - f0) / 2;
        SkASSERT(f->fT0 < f->fT1);

        if (t > f->fT1) {
            f0 = f + 1;
        } else {
            f1 = f;
        }
    }

    SkASSERT(f0 == f1);
    SkASSERT(t >= f0->fT0 && t <= f1->fT1);
    return *f0;
}

} // namespace skotty

#endif // SkottyProperties_DEFINED
