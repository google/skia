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

    template <typename T>
    T as() const;
};

struct ShapeValue {
    SkTArray<BezierVertex, true> fVertices;
    bool                         fClose = false;

    ShapeValue()                  = default;
    ShapeValue(const ShapeValue&) = default;
    ShapeValue(ShapeValue&&)      = default;

    static bool Parse(const Json::Value&, ShapeValue*);

    template <typename T>
    T as() const;

    SkPath asPath() const;
};

template <typename T>
class Keyframed final : public SkNoncopyable {
public:
    static std::unique_ptr<Keyframed<T>> Make(const Json::Value& json) {
        const auto& frames = json["k"];
        if (!frames.isArray()) return nullptr;

        SkTArray<Keyframe> key_frames;
        for (Json::ArrayIndex i = 0; i < frames.size(); ++i) {
            const auto& frame = frames[i];

            T startVal, endVal;
            if (!frame.isObject() ||
                !T::Parse(frame["s"], &startVal) ||
                !T::Parse(frame["e"], &endVal)) {
                return nullptr;
            }

            const auto& t = frame["t"];
            if (!t.isConvertibleTo(Json::realValue)) return nullptr;

            key_frames.emplace_back(std::move(startVal),
                                    std::move(endVal),
                                    ParseScalar(t, 0));
        }

        return std::unique_ptr<Keyframed<T>>(new Keyframed(std::move(key_frames)));
    }

    T operator()(float t) const {
        // TODO: binary search, interpolation

        int idx = 0;
        for (; idx < fFrames.count() && fFrames[idx].fStartTime <= t; ++idx);

        idx = SkTMax(idx - 1, 0);
        const auto& frame = fFrames[idx];

        const auto start_t = frame.fStartTime,
                   stop_t  = idx < fFrames.count() - 1 ? fFrames[idx + 1].fStartTime : t,
                   rel_t   = SkTPin<float>((t - start_t) / (stop_t - start_t), 0, 1);

        return frame.lerp(rel_t);
    }

private:
    struct Keyframe {
        Keyframe(T&& s, T&& e, float t)
            : fStartValue(std::move(s))
            , fEndValue(std::move(e))
            , fStartTime(t) {}

        T lerp(float t) const;

        T     fStartValue,
              fEndValue;
        float fStartTime;
    };

    explicit Keyframed(SkTArray<Keyframe>&& frames) : fFrames(std::move(frames)) {}

    const SkTArray<Keyframe> fFrames;

    using INHERITED = SkNoncopyable;
};

} // namespace skotty

#endif // SkottyProperties_DEFINED
