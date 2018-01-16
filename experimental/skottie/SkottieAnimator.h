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
#include "SkTypes.h"

#include <memory>
#include <vector>

namespace skottie {

class AnimatorBase : public SkNoncopyable {
public:
    virtual ~AnimatorBase() = default;

    virtual void tick(float) = 0;

protected:
    AnimatorBase() = default;
};

class KeyframeIntervalBase : public SkNoncopyable {
public:
    KeyframeIntervalBase()                                  = default;
    KeyframeIntervalBase(KeyframeIntervalBase&&)            = default;
    KeyframeIntervalBase& operator=(KeyframeIntervalBase&&) = default;

    float t0() const { return fT0; }
    float t1() const { return fT1; }

    bool isValid() const { return fT0 < fT1; }
    bool contains(float t) const { return t >= fT0 && t <= fT1; }

protected:
    // Parse the current interval AND back-fill prev interval t1.
    bool parse(const Json::Value&, KeyframeIntervalBase* prev);

    // Computes a "local" t (relative to [fT0..fT1]), and mapped
    // through the cubic (if applicable).
    float localT(float t) const;

private:
    // Initialized for non-linear interpolation.
    std::unique_ptr<SkCubicMap> fCubicMap;

    // Start/end times.
    float                       fT0 = 0,
                                fT1 = 0;
};

// Describes a keyframe interpolation interval (v0@t0) -> (v1@t1).
template <typename T>
class KeyframeInterval final : public KeyframeIntervalBase {
public:
    bool parse(const Json::Value& k, KeyframeInterval* prev) {
        SkASSERT(k.isObject());

        return this->INHERITED::parse(k, prev) &&
            ValueTraits<T>::Parse(k["s"], &fV0) &&
            ValueTraits<T>::Parse(k["e"], &fV1) &&
            ValueTraits<T>::Cardinality(fV0) == ValueTraits<T>::Cardinality(fV1) &&
            (!prev || ValueTraits<T>::Cardinality(fV0) == ValueTraits<T>::Cardinality(prev->fV0));
    }

    void lerp(float t, T*) const;

private:
    // Start/end values.
    T fV0,
      fV1;

    using INHERITED = KeyframeIntervalBase;
};

template <typename T>
std::vector<KeyframeInterval<T>> ParseFrames(const Json::Value& jframes) {
    std::vector<KeyframeInterval<T>> frames;

    if (jframes.isArray()) {
        frames.reserve(jframes.size());

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
    }

    // If we couldn't determine a t1 for the last frame, discard it.
    if (!frames.empty() && !frames.back().isValid()) {
        frames.pop_back();
    }

    return frames;
}

// Binds an animated/keyframed property to a node attribute setter.
template <typename ValT, typename NodeT>
class Animator : public AnimatorBase {
public:
    using ApplyFuncT = void(*)(NodeT*, const ValT&);
    static std::unique_ptr<Animator> Make(std::vector<KeyframeInterval<ValT>>&& frames,
                                          sk_sp<NodeT> node,
                                          ApplyFuncT&& applyFunc) {
        return (node && !frames.empty())
            ? std::unique_ptr<Animator>(new Animator(std::move(frames),
                                                     std::move(node),
                                                     std::move(applyFunc)))
            : nullptr;
    }

    void tick(float t) override {
        const auto& frame = this->findFrame(t);

        ValT val;
        frame.lerp(t, &val);

        fFunc(fTarget.get(), val);
    }

private:
    Animator(std::vector<KeyframeInterval<ValT>>&& frames, sk_sp<NodeT> node,
             ApplyFuncT&& applyFunc)
        : fFrames(std::move(frames))
        , fTarget(std::move(node))
        , fFunc(std::move(applyFunc)) {}

    const KeyframeInterval<ValT>& findFrame(float t) const;

    const std::vector<KeyframeInterval<ValT>> fFrames;
    sk_sp<NodeT>                              fTarget;
    ApplyFuncT                                fFunc;
};

template <typename ValT, typename NodeT>
const KeyframeInterval<ValT>& Animator<ValT, NodeT>::findFrame(float t) const {
    SkASSERT(!fFrames.empty());

    // TODO: cache last/current frame?

    auto f0 = fFrames.begin(),
         f1 = fFrames.end() - 1;

    SkASSERT(f0->isValid());
    SkASSERT(f1->isValid());

    if (t < f0->t0()) {
        return *f0;
    }

    if (t > f1->t1()) {
        return *f1;
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

    return *f0;
}

} // namespace skottie

#endif // SkottieAnimator_DEFINED
