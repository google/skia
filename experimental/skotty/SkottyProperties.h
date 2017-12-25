/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottyProperties_DEFINED
#define SkottyProperties_DEFINED

#include "SkTArray.h"
#include "SkTypes.h"

namespace  skotty {

template <typename T>
class Property : public SkNoncopyable {
public:
    virtual ~Property() = default;

    T operator()(float t) const { return this->onComputeValue(t); }

    virtual bool isAnimated() const = 0;

protected:
    Property() = default;

    virtual T onCompute(float t) const = 0;

private:
    using INHERITED = SkNoncopyable;
};

template <typename T>
class ScalarProperty final : public Property<T> {
public:
    ScalarProperty(T&& value) : fValue(std::move(value)) {}

    bool isAnimated() const override { return false; }

protected:
    T onCompute(float) const override { return fValue; }

private:
    const T fValue;

    using INHERITED = Property<T>;
};

template <typename T>
class KeyframedProperty final : public Property<T> {
public:
    struct Keyframe {
        float fTime;
        T     fValue;
    };

    KeyframedProperty(SkTArray<Keyframe>&& frames) : fFrames(std::move(frames)) {}

    bool isAnimated() const override { return true; }

protected:

    T onCompute(float t) const override {
        // TODO: binary search, interpolation

        int idx = 0;
        for (; idx < fFrames.count() && fFrames[idx].fTime <= t; ++idx);

        const auto& frame = fFrames[SkTMax(idx - 1, 0)];
        return frame.fValue;
    }

private:
    const SkTArray<Keyframe> fFrames;

    using INHERITED = Property<T>;
};

struct ShapeStruct {
    SkTArray<float, true> fInPts,   // Bezier in-points, relative to vertices
                          fOutPts,  // Bezier out-points, relative to vertices
                          fVerts;
    bool                  fClose;   // Auto-close
};

using Value                     = Property         <float>;
using ScalarValue               = ScalarProperty   <float>;
using KeyframedValue            = KeyframedProperty<float>;

using MultiDimensional          = Property         <SkTArray<float, true>>;
using ScalarMultiDimensional    = ScalarProperty   <SkTArray<float, true>>;
using KeyframedMultiDimensional = KeyframedProperty<SkTArray<float, true>>;

using ShapeValue                = Property         <ShapeStruct>;
using ScalarShapeValue          = ScalarProperty   <ShapeStruct>;
using KeyframedShapeValue       = KeyframedProperty<ShapeStruct>;

} // namespace skotty

#endif // SkottyProperties_DEFINED
