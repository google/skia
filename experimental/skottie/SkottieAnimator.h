/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieAnimator_DEFINED
#define SkottieAnimator_DEFINED

#include "SkCubicMap.h"
#include "SkMakeUnique.h"
#include "SkottiePriv.h"
#include "SkottieProperties.h"
#include "SkSGScene.h"
#include "SkTArray.h"
#include "SkTypes.h"

#include <functional>
#include <memory>

namespace skottie {

class KeyframeIntervalBase : public SkNoncopyable {
public:
    KeyframeIntervalBase()                                  = default;
    KeyframeIntervalBase(KeyframeIntervalBase&&)            = default;
    KeyframeIntervalBase& operator=(KeyframeIntervalBase&&) = default;

    float t0() const { return fT0; }
    float t1() const { return fT1; }

    bool isValid() const { return fT0 < fT1 || fHold; }
    bool contains(float t) const { return t >= fT0 && t <= fT1; }

protected:
    // Parse the current interval AND back-fill prev interval t1.
    bool parse(const Json::Value&, KeyframeIntervalBase* prev);

    // Computes a "local" t (relative to [fT0..fT1]), and mapped
    // through the cubic (if applicable).
    float localT(float t) const;

    bool isHold() const { return fHold; }

private:
    // Initialized for non-linear interpolation.
    std::unique_ptr<SkCubicMap> fCubicMap;

    // Start/end times.
    float                       fT0 = 0,
                                fT1 = 0;

    bool                        fHold = false;
};

// Describes a keyframe interpolation interval (v0@t0) -> (v1@t1).
template <typename T>
class KeyframeInterval final : public KeyframeIntervalBase {
public:
    bool parse(const Json::Value& k, KeyframeInterval* prev) {
        SkASSERT(k.isObject());

        if (!this->INHERITED::parse(k, prev) ||
            !ValueTraits<T>::Parse(k["s"], &fV0)) {
            return false;
        }

        if (!this->isHold() &&
            (!ValueTraits<T>::Parse(k["e"], &fV1) ||
             ValueTraits<T>::Cardinality(fV0) != ValueTraits<T>::Cardinality(fV1))) {
            return false;
        }

        return !prev || ValueTraits<T>::Cardinality(fV0) == ValueTraits<T>::Cardinality(prev->fV0);
    }

    void eval(float t, T* v) const {
        if (this->isHold() || t <= this->t0()) {
            *v = fV0;
        } else if (t >= this->t1()) {
            *v = fV1;
        } else {
            this->lerp(t, v);
        }
    }

private:
    void lerp(float t, T*) const;

    // Start/end values.
    T fV0,
      fV1;

    using INHERITED = KeyframeIntervalBase;
};

// Binds an animated/keyframed property to a node attribute setter.
template <typename T>
class Animator final : public sksg::Animator {
public:
    using ApplyFuncT = std::function<void(const T&)>;

    static std::unique_ptr<Animator> Make(const Json::Value& jframes, ApplyFuncT&& applyFunc) {
        if (!jframes.isArray())
            return nullptr;

        SkTArray<KeyframeInterval<T>, true> frames(jframes.size());

        KeyframeInterval<T>* prev_frame = nullptr;
        for (const auto& jframe : jframes) {
            if (!jframe.isObject())
                continue;

            KeyframeInterval<T> frame;
            if (frame.parse(jframe, prev_frame)) {
                frames.push_back(std::move(frame));
                prev_frame = &frames.back();
            }
        }

        // If we couldn't determine a t1 for the last frame, discard it.
        if (!frames.empty() && !frames.back().isValid()) {
            frames.pop_back();
        }

        return frames.empty()
            ? nullptr
            : std::unique_ptr<Animator>(new Animator(std::move(frames), std::move(applyFunc)));
    }

    void onTick(float t) override {
        if (!fCurrentFrame || !fCurrentFrame->contains(t)) {
            fCurrentFrame = this->findFrame(t);
        }

        T val;
        fCurrentFrame->eval(t, &val);

        fFunc(val);
    }

private:
    Animator(SkTArray<KeyframeInterval<T>, true>&& frames, ApplyFuncT&& applyFunc)
        : fFrames(std::move(frames))
        , fFunc(std::move(applyFunc)) {}

    const KeyframeInterval<T>* findFrame(float t) const;

    const SkTArray<KeyframeInterval<T>, true> fFrames;
    const ApplyFuncT                          fFunc;
    const KeyframeInterval<T>*                fCurrentFrame = nullptr;
};

template <typename T>
const KeyframeInterval<T>* Animator<T>::findFrame(float t) const {
    SkASSERT(!fFrames.empty());

    auto f0 = fFrames.begin(),
         f1 = fFrames.end() - 1;

    SkASSERT(f0->isValid());
    SkASSERT(f1->isValid());

    if (t < f0->t0()) {
        return f0;
    }

    if (t > f1->t1()) {
        return f1;
    }

    while (f0 != f1) {
        SkASSERT(f0 < f1);
        SkASSERT(t >= f0->t0() && t <= f1->t1());

        const auto f = f0 + (f1 - f0) / 2;
        SkASSERT(f->isValid());

        if (t > f->t1()) {
            f0 = f + 1;
        } else {
            f1 = f;
        }
    }

    SkASSERT(f0 == f1);
    SkASSERT(f0->contains(t));

    return f0;
}

} // namespace skottie

#endif // SkottieAnimator_DEFINED
