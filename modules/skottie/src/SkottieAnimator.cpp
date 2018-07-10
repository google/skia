/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottieAnimator.h"

#include "SkCubicMap.h"
#include "SkottieJson.h"
#include "SkottieValue.h"
#include "SkString.h"
#include "SkTArray.h"

#include <memory>

namespace skottie {

namespace {

#define LOG SkDebugf

bool LogFail(const skjson::Value& json, const char* msg) {
    const auto dump = json.toString();
    LOG("!! %s: %s\n", msg, dump.c_str());
    return false;
}

class KeyframeAnimatorBase : public sksg::Animator {
public:
    int count() const { return fRecs.count(); }

protected:
    KeyframeAnimatorBase() = default;

    struct KeyframeRec {
        float t0, t1;
        int   vidx0, vidx1, // v0/v1 indices
              cmidx;        // cubic map index

        bool contains(float t) const { return t0 <= t && t <= t1; }
        bool isConstant() const { return vidx0 == vidx1; }
        bool isValid() const {
            SkASSERT(t0 <= t1);
            // Constant frames don't need/use t1 and vidx1.
            return t0 < t1 || this->isConstant();
        }
    };

    const KeyframeRec& frame(float t) {
        if (!fCachedRec || !fCachedRec->contains(t)) {
            fCachedRec = findFrame(t);
        }
        return *fCachedRec;
    }

    float localT(const KeyframeRec& rec, float t) const {
        SkASSERT(rec.isValid());
        SkASSERT(!rec.isConstant());
        SkASSERT(t > rec.t0 && t < rec.t1);

        auto lt = (t - rec.t0) / (rec.t1 - rec.t0);

        return rec.cmidx < 0
            ? lt
            : SkTPin(fCubicMaps[rec.cmidx].computeYFromX(lt), 0.0f, 1.0f);
    }

    virtual int parseValue(const skjson::Value&) = 0;

    void parseKeyFrames(const skjson::ArrayValue& jframes) {
        for (const skjson::ObjectValue* jframe : jframes) {
            if (!jframe) continue;

            float t0;
            if (!Parse<float>((*jframe)["t"], &t0))
                continue;

            if (!fRecs.empty()) {
                if (fRecs.back().t1 >= t0) {
                    LOG("!! Ignoring out-of-order key frame (t:%f < t:%f)\n", t0, fRecs.back().t1);
                    continue;
                }
                // Back-fill t1 in prev interval.  Note: we do this even if we end up discarding
                // the current interval (to support "t"-only final frames).
                fRecs.back().t1 = t0;
            }

            const auto vidx0 = this->parseValue((*jframe)["s"]);
            if (vidx0 < 0)
                continue;

            // Defaults for constant frames.
            int vidx1 = vidx0, cmidx = -1;

            if (!ParseDefault<bool>((*jframe)["h"], false)) {
                // Regular frame, requires an end value.
                vidx1 = this->parseValue((*jframe)["e"]);
                if (vidx1 < 0)
                    continue;

                // default is linear lerp
                static constexpr SkPoint kDefaultC0 = { 0, 0 },
                                         kDefaultC1 = { 1, 1 };
                const auto c0 = ParseDefault<SkPoint>((*jframe)["i"], kDefaultC0),
                           c1 = ParseDefault<SkPoint>((*jframe)["o"], kDefaultC1);

                if (c0 != kDefaultC0 || c1 != kDefaultC1) {
                    // TODO: is it worth de-duping these?
                    cmidx = fCubicMaps.count();
                    fCubicMaps.emplace_back();
                    // TODO: why do we have to plug these inverted?
                    fCubicMaps.back().setPts(c1, c0);
                }
            }

            fRecs.push_back({t0, t0, vidx0, vidx1, cmidx });
        }

        // If we couldn't determine a valid t1 for the last frame, discard it.
        if (!fRecs.empty() && !fRecs.back().isValid()) {
            fRecs.pop_back();
        }

        SkASSERT(fRecs.empty() || fRecs.back().isValid());
    }

private:
    const KeyframeRec* findFrame(float t) const {
        SkASSERT(!fRecs.empty());

        auto f0 = &fRecs.front(),
             f1 = &fRecs.back();

        SkASSERT(f0->isValid());
        SkASSERT(f1->isValid());

        if (t < f0->t0) {
            return f0;
        }

        if (t > f1->t1) {
            return f1;
        }

        while (f0 != f1) {
            SkASSERT(f0 < f1);
            SkASSERT(t >= f0->t0 && t <= f1->t1);

            const auto f = f0 + (f1 - f0) / 2;
            SkASSERT(f->isValid());

            if (t > f->t1) {
                f0 = f + 1;
            } else {
                f1 = f;
            }
        }

        SkASSERT(f0 == f1);
        SkASSERT(f0->contains(t));

        return f0;
    }

    SkTArray<KeyframeRec> fRecs;
    SkTArray<SkCubicMap>  fCubicMaps;
    const KeyframeRec*    fCachedRec = nullptr;

    using INHERITED = sksg::Animator;
};

template <typename T>
class KeyframeAnimator final : public KeyframeAnimatorBase {
public:
    static std::unique_ptr<KeyframeAnimator> Make(const skjson::ArrayValue* jv,
                                                  std::function<void(const T&)>&& apply) {
        if (!jv) return nullptr;

        std::unique_ptr<KeyframeAnimator> animator(new KeyframeAnimator(*jv, std::move(apply)));
        if (!animator->count())
            return nullptr;

        return animator;
    }

protected:
    void onTick(float t) override {
        T val;
        this->eval(this->frame(t), t, &val);

        fApplyFunc(val);
    }

private:
    KeyframeAnimator(const skjson::ArrayValue& jframes,
                     std::function<void(const T&)>&& apply)
        : fApplyFunc(std::move(apply)) {
        this->parseKeyFrames(jframes);
    }

    int parseValue(const skjson::Value& jv) override {
        T val;
        if (!Parse<T>(jv, &val) || (!fVs.empty() &&
                ValueTraits<T>::Cardinality(val) != ValueTraits<T>::Cardinality(fVs.back()))) {
            return -1;
        }

        // TODO: full deduping?
        if (fVs.empty() || val != fVs.back()) {
            fVs.push_back(std::move(val));
        }
        return fVs.count() - 1;
    }

    void eval(const KeyframeRec& rec, float t, T* v) const {
        SkASSERT(rec.isValid());
        if (rec.isConstant() || t <= rec.t0) {
            *v = fVs[rec.vidx0];
        } else if (t >= rec.t1) {
            *v = fVs[rec.vidx1];
        } else {
            const auto lt = this->localT(rec, t);
            const auto& v0 = fVs[rec.vidx0];
            const auto& v1 = fVs[rec.vidx1];
            *v = ValueTraits<T>::Lerp(v0, v1, lt);
        }
    }

    const std::function<void(const T&)> fApplyFunc;
    SkTArray<T>                         fVs;


    using INHERITED = KeyframeAnimatorBase;
};

template <typename T>
static inline bool BindPropertyImpl(const skjson::ObjectValue* jprop,
                                    sksg::AnimatorList* animators,
                                    std::function<void(const T&)>&& apply,
                                    const T* noop = nullptr) {
    if (!jprop) return false;

    const auto& jpropA = (*jprop)["a"];
    const auto& jpropK = (*jprop)["k"];

    if (!(*jprop)["x"].is<skjson::NullValue>()) {
        LOG("?? Unsupported expression.\n");
    }

    // Older Json versions don't have an "a" animation marker.
    // For those, we attempt to parse both ways.
    if (!ParseDefault<bool>(jpropA, false)) {
        T val;
        if (Parse<T>(jpropK, &val)) {
            // Static property.
            if (noop && val == *noop)
                return false;

            apply(val);
            return true;
        }

        if (!jpropA.is<skjson::NullValue>()) {
            return LogFail(*jprop, "Could not parse (explicit) static property");
        }
    }

    // Keyframe property.
    auto animator = KeyframeAnimator<T>::Make(jpropK, std::move(apply));

    if (!animator) {
        return LogFail(*jprop, "Could not parse keyframed property");
    }

    animators->push_back(std::move(animator));

    return true;
}

class SplitPointAnimator final : public sksg::Animator {
public:
    static std::unique_ptr<SplitPointAnimator> Make(const skjson::ObjectValue* jprop,
                                                    std::function<void(const VectorValue&)>&& apply,
                                                    const VectorValue*) {
        if (!jprop) return nullptr;

        std::unique_ptr<SplitPointAnimator> split_animator(
            new SplitPointAnimator(std::move(apply)));

        // This raw pointer is captured in lambdas below. But the lambdas are owned by
        // the object itself, so the scope is bound to the life time of the object.
        auto* split_animator_ptr = split_animator.get();

        if (!BindPropertyImpl<ScalarValue>((*jprop)["x"], &split_animator->fAnimators,
                [split_animator_ptr](const ScalarValue& x) { split_animator_ptr->setX(x); }) ||
            !BindPropertyImpl<ScalarValue>((*jprop)["y"], &split_animator->fAnimators,
                [split_animator_ptr](const ScalarValue& y) { split_animator_ptr->setY(y); })) {
            LogFail(*jprop, "Could not parse split property");
            return nullptr;
        }

        if (split_animator->fAnimators.empty()) {
            // Static split property: commit the (buffered) value and discard.
            split_animator->onTick(0);
            return nullptr;
        }

        return split_animator;
    }

    void onTick(float t) override {
        for (const auto& animator : fAnimators) {
            animator->tick(t);
        }

        const VectorValue vec = { fX, fY };
        fApplyFunc(vec);
    }

    void setX(const ScalarValue& x) { fX = x; }
    void setY(const ScalarValue& y) { fY = y; }

private:
    explicit SplitPointAnimator(std::function<void(const VectorValue&)>&& apply)
        : fApplyFunc(std::move(apply)) {}

    const std::function<void(const VectorValue&)> fApplyFunc;
    sksg::AnimatorList                            fAnimators;

    ScalarValue                                   fX = 0,
                                                  fY = 0;

    using INHERITED = sksg::Animator;
};

bool BindSplitPositionProperty(const skjson::Value& jv,
                               sksg::AnimatorList* animators,
                               std::function<void(const VectorValue&)>&& apply,
                               const VectorValue* noop) {
    if (auto split_animator = SplitPointAnimator::Make(jv, std::move(apply), noop)) {
        animators->push_back(std::unique_ptr<sksg::Animator>(split_animator.release()));
        return true;
    }

    return false;
}

} // namespace

template <>
bool BindProperty(const skjson::Value& jv,
                  sksg::AnimatorList* animators,
                  std::function<void(const ScalarValue&)>&& apply,
                  const ScalarValue* noop) {
    return BindPropertyImpl(jv, animators, std::move(apply), noop);
}

template <>
bool BindProperty(const skjson::Value& jv,
                  sksg::AnimatorList* animators,
                  std::function<void(const VectorValue&)>&& apply,
                  const VectorValue* noop) {
    if (!jv.is<skjson::ObjectValue>())
        return false;

    return ParseDefault<bool>(jv.as<skjson::ObjectValue>()["s"], false)
        ? BindSplitPositionProperty(jv, animators, std::move(apply), noop)
        : BindPropertyImpl(jv, animators, std::move(apply), noop);
}

template <>
bool BindProperty(const skjson::Value& jv,
                  sksg::AnimatorList* animators,
                  std::function<void(const ShapeValue&)>&& apply,
                  const ShapeValue* noop) {
    return BindPropertyImpl(jv, animators, std::move(apply), noop);
}

} // namespace skottie
