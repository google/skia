/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottyAnimator_DEFINED
#define SkottyAnimator_DEFINED

#include "SkottyPriv.h"
#include "SkottyProperties.h"
#include "SkTArray.h"
#include "SkTypes.h"

#include <functional>
#include <memory>

namespace skotty {

class AnimatorBase : public SkNoncopyable {
public:
    virtual ~AnimatorBase() = default;

    virtual void tick(SkMSec) = 0;

protected:
    AnimatorBase()  = default;
};

// Describes a keyframe interpolation interval (v0@t0) -> (v1@t1).
// TODO: add interpolation params.
template <typename T>
struct KeyframeInterval {
    T     fV0,
          fV1;
    float fT0 = 0,
          fT1 = 0;

    void lerp(float t, T*) const;
};

// Binds an animated/keyframed property to a node attribute.
template <typename ValT, typename AttrT, typename NodeT>
class Animator : public AnimatorBase {
public:
    static std::unique_ptr<Animator> Make(const Json::Value& frames, sk_sp<NodeT> node,
        std::function<void(const sk_sp<NodeT>&, const AttrT&)> applyFunc);

    void tick(SkMSec t) override {
        const auto& frame = this->findInterval(t);
        const auto rel_t = (t - frame.fT0) / (frame.fT1 - frame.fT0);

        ValT val;
        frame.lerp(SkTPin<float>(rel_t, 0, 1), &val);

        fFunc(fTarget, val.template as<AttrT>());
    }

private:
    Animator(SkTArray<KeyframeInterval<ValT>>&& intervals, sk_sp<NodeT> node,
             std::function<void(const sk_sp<NodeT>&, const AttrT&)> applyFunc)
        : fIntervals(std::move(intervals))
        , fTarget(std::move(node))
        , fFunc(std::move(applyFunc)) {}

    const KeyframeInterval<ValT>& findInterval(float t) const;

    const SkTArray<KeyframeInterval<ValT>>                 fIntervals;
    sk_sp<NodeT>                                           fTarget;
    std::function<void(const sk_sp<NodeT>&, const AttrT&)> fFunc;
};

template <typename ValT, typename AttrT, typename NodeT>
std::unique_ptr<Animator<ValT, AttrT, NodeT>>
Animator<ValT, AttrT, NodeT>::Make(const Json::Value& frames,
    sk_sp<NodeT> node, std::function<void(const sk_sp<NodeT>&, const AttrT&)> applyFunc) {

    if (!frames.isArray())
        return nullptr;

    SkTArray<KeyframeInterval<ValT>> intervals;
    for (const auto& frame : frames) {
        if (!frame.isObject())
            return nullptr;

        const auto t = ParseScalar(frame["t"], SK_ScalarMin);
        if (t == SK_ScalarMin)
            break;

        auto* prev_interval = intervals.empty() ? nullptr : &intervals.back();
        if (prev_interval) {
            if (prev_interval->fT0 >= t) {
                LOG("!! Ignoring out-of-order key frame (t: %f < t: %f)\n", t, prev_interval->fT0);
                continue;
            }
            // Back-fill the prev interval t1.
            prev_interval->fT1 = t;
        }

        auto& curr_interval = intervals.push_back();
        if (!ValT::Parse(frame["s"], &curr_interval.fV0) ||
            !ValT::Parse(frame["e"], &curr_interval.fV1) ||
            curr_interval.fV0.cardinality() != curr_interval.fV1.cardinality() ||
            (prev_interval &&
             curr_interval.fV0.cardinality() != prev_interval->fV0.cardinality())) {
            // Invalid frame, or "t"-only frame.
            intervals.pop_back();
            continue;
        }

        curr_interval.fT0 = curr_interval.fT1 = t;
    }

    // If we couldn't determine a t1 for the last interval, discard it.
    if (!intervals.empty() && intervals.back().fT0 == intervals.back().fT1) {
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

#endif // SkottyAnimator_DEFINED
