/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottyProperties_DEFINED
#define SkottyProperties_DEFINED

#include "SkPoint.h"
#include "SkTArray.h"
#include "SkTypes.h"

#include <memory>

// TODO - hide this from headers
#include "SkottyPriv.h"

class SkPath;

namespace  skotty {

template <typename T>
class Property : public SkNoncopyable {
public:
    virtual ~Property() = default;

    static std::unique_ptr<Property> Make(const Json::Value&);

    const T& operator()(float t) const { return this->onComputeValue(t); }

    virtual bool isAnimated() const = 0;

protected:
    Property() = default;

    virtual const T& onComputeValue(float t) const = 0;

    static bool ParseValue(const Json::Value&, T*);

private:
    using INHERITED = SkNoncopyable;
};

template <typename T>
class StaticProperty final : public Property<T> {
public:
    static std::unique_ptr<Property<T>> Make(const Json::Value& json) {
        T val;
        return INHERITED::ParseValue(json["k"], &val)
            ? std::unique_ptr<Property<T>>(new StaticProperty<T>(std::move(val)))
            : nullptr;
    }

    bool isAnimated() const override { return false; }

protected:
    const T& onComputeValue(float) const override { return fValue; }

private:
    explicit StaticProperty(T&& value) : fValue(std::move(value)) {}

    const T fValue;

    using INHERITED = Property<T>;
};

template <typename T>
class KeyframedProperty final : public Property<T> {
public:
    static std::unique_ptr<Property<T>> Make(const Json::Value& json) {
        const auto& frames = json["k"];
        if (!frames.isArray()) return nullptr;

        SkTArray<Keyframe> key_frames;
        for (Json::ArrayIndex i = 0; i < frames.size(); ++i) {
            const auto& frame = frames[i];

            T startVal, endVal;
            if (!frame.isObject() ||
                !INHERITED::ParseValue(frame["s"][0], &startVal) ||
                !INHERITED::ParseValue(frame["e"][0], &endVal)) {
                return nullptr;
            }

            const auto& t = frame["t"];
            if (!t.isConvertibleTo(Json::realValue)) return nullptr;

            key_frames.emplace_back(std::move(startVal),
                                    std::move(endVal),
                                    ParseScalar(t, 0));
        }

        return std::unique_ptr<Property<T>>(new KeyframedProperty(std::move(key_frames)));
    }

    bool isAnimated() const override { return true; }

protected:

    const T& onComputeValue(float t) const override {
        // TODO: binary search, interpolation

        int idx = 0;
        for (; idx < fFrames.count() && fFrames[idx].fStartTime <= t; ++idx);

        const auto& frame = fFrames[SkTMax(idx - 1, 0)];
        return frame.fStartValue;
    }

private:
    struct Keyframe {
        Keyframe(T&& s, T&& e, float t)
            : fStartValue(std::move(s))
            , fEndValue(std::move(e))
            , fStartTime(t) {}

        T     fStartValue,
              fEndValue;
        float fStartTime;
    };

    explicit KeyframedProperty(SkTArray<Keyframe>&& frames) : fFrames(std::move(frames)) {}

    const SkTArray<Keyframe> fFrames;

    using INHERITED = Property<T>;
};

template <typename T>
std::unique_ptr<Property<T>> Property<T>::Make(const Json::Value& json) {
    const auto& animated = json["a"];
    if (animated.isNull()) {
        return nullptr;
    }

    return ParseBool(animated, false)
        ? KeyframedProperty<T>::Make(json) : StaticProperty<T>::Make(json);
}


struct BezierVertex {
    SkPoint fInPoint,  // "in" control point, relative to the vertex
            fOutPoint, // "out" control point, relative to the vertex
            fVertex;
};

struct ShapeValue {
    SkTArray<BezierVertex, true> fVertices;
    bool                         fClose = false;

    ShapeValue()                  = default;
    ShapeValue(const ShapeValue&) = delete;
    ShapeValue(ShapeValue&&)      = default;

    SkPath asPath() const;
};

using ScalarProperty = Property<float>;
using VectorProperty = Property<SkTArray<float, true>>;
using ShapeProperty  = Property<ShapeValue>;

} // namespace skotty

#endif // SkottyProperties_DEFINED
