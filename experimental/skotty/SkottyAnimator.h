/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottyAnimator_DEFINED
#define SkottyAnimator_DEFINED

#include "SkCubicMap.h"
#include "SkMakeUnique.h"
#include "SkottyPriv.h"
#include "SkottyProperties.h"
#include "SkTArray.h"
#include "SkTypes.h"

#include <memory>

namespace skotty {

class AnimatorBase : public SkNoncopyable {
public:
    virtual ~AnimatorBase() = default;

    virtual void tick(SkMSec) = 0;

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
    // Parse the current interval AND back-fill prev interval t1.
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
    T       fV0,
            fV1;

    using INHERITED = KeyframeIntervalBase;
};

// Binds an animated/keyframed property to a node attribute.
template <typename ValT, typename AttrT, typename NodeT>
class Animator : public AnimatorBase {
public:
    using ApplyFuncT = void(*)(const sk_sp<NodeT>&, const AttrT&);
    static std::unique_ptr<Animator> Make(const Json::Value& frames, sk_sp<NodeT> node,
        ApplyFuncT&& applyFunc);

    void tick(SkMSec t) override {
        const auto& frame = this->findInterval(t);

        ValT val;
        frame.lerp(t, &val);

        fFunc(fTarget, ValueTraits<ValT>::template As<AttrT>(val));
    }

private:
    Animator(SkTArray<KeyframeInterval<ValT>>&& intervals, sk_sp<NodeT> node,
             ApplyFuncT&& applyFunc)
        : fIntervals(std::move(intervals))
        , fTarget(std::move(node))
        , fFunc(std::move(applyFunc)) {}

    const KeyframeInterval<ValT>& findInterval(float t) const;

    const SkTArray<KeyframeInterval<ValT>> fIntervals;
    sk_sp<NodeT>                           fTarget;
    ApplyFuncT                             fFunc;
};

template <typename ValT, typename AttrT, typename NodeT>
std::unique_ptr<Animator<ValT, AttrT, NodeT>>
Animator<ValT, AttrT, NodeT>::Make(const Json::Value& frames, sk_sp<NodeT> node,
                                   ApplyFuncT&& applyFunc) {

    if (!frames.isArray())
        return nullptr;

    SkTArray<KeyframeInterval<ValT>> intervals;
    intervals.reserve(frames.size());

    KeyframeInterval<ValT>* prev_interval = nullptr;
    for (const auto& frame : frames) {
        if (!frame.isObject())
            return nullptr;

        KeyframeInterval<ValT> curr_interval;
        if (curr_interval.parse(frame, prev_interval)) {
            intervals.push_back(std::move(curr_interval));
            prev_interval = &intervals.back();
        }
    }

    // If we couldn't determine a t1 for the last interval, discard it.
    if (!intervals.empty() && !intervals.back().isValid()) {
        intervals.pop_back();
    }

    if (intervals.empty()) {
        return nullptr;
    }

    return std::unique_ptr<Animator>(
        new Animator(std::move(intervals), node, std::move(applyFunc)));
}

template <typename ValT, typename AttrT, typename NodeT>
const KeyframeInterval<ValT>& Animator<ValT, AttrT, NodeT>::findInterval(float t) const {
    SkASSERT(!fIntervals.empty());

    // TODO: cache last/current frame?

    auto f0 = fIntervals.begin(),
         f1 = fIntervals.end() - 1;

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

} // namespace skotty

#endif // SkottyAnimator_DEFINED
