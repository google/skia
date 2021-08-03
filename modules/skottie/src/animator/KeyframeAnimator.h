/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieKeyframeAnimator_DEFINED
#define SkottieKeyframeAnimator_DEFINED

#include "include/core/SkCubicMap.h"
#include "include/core/SkPoint.h"
#include "include/private/SkNoncopyable.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/src/animator/Animator.h"

#include <vector>

namespace skjson {
class ArrayValue;
class ObjectValue;
class Value;
} // namespace skjson

namespace skottie::internal {

class AnimationBuilder;

struct Keyframe {
    // We can store scalar values inline; other types are stored externally,
    // and we track them by index.
    struct Value {
        union {
            uint32_t idx;
            float    flt;
        };

        bool operator==(const Value& other) const {
            return idx == other.idx
                || flt == other.flt; // +/-0
        }
        bool operator!=(const Value& other) const { return !((*this) == other); }
    };

    float    t;
    Value    v;
    uint32_t mapping; // Encodes the value interpolation in [KFRec_n .. KFRec_n+1):
                      //   0 -> constant
                      //   1 -> linear
                      //   n -> cubic: cubic_mappers[n-2]

    static constexpr uint32_t kConstantMapping  = 0;
    static constexpr uint32_t kLinearMapping    = 1;
    static constexpr uint32_t kCubicIndexOffset = 2;
};

class KeyframeAnimator : public Animator {
public:
    ~KeyframeAnimator() override;

    bool isConstant() const {
        SkASSERT(!fKFs.empty());

        // parseKeyFrames() ensures we only keep a single frame for constant properties.
        return fKFs.size() == 1;
    }

protected:
    KeyframeAnimator(std::vector<Keyframe> kfs, std::vector<SkCubicMap> cms)
        : fKFs(std::move(kfs))
        , fCMs(std::move(cms)) {}

    struct LERPInfo {
        float           weight; // vrec0/vrec1 weight [0..1]
        Keyframe::Value vrec0, vrec1;

        bool isConstant() const { return vrec0 == vrec1; }
    };

    // Main entry point: |t| -> LERPInfo
    LERPInfo getLERPInfo(float t) const;

private:
    // Two sequential KFRecs determine how the value varies within [kf0 .. kf1)
    struct KFSegment {
        const Keyframe* kf0;
        const Keyframe* kf1;

        bool contains(float t) const {
            SkASSERT(!!kf0 == !!kf1);
            SkASSERT(!kf0 || kf1 == kf0 + 1);

            return kf0 && kf0->t <= t && t < kf1->t;
        }
    };

    // Find the KFSegment containing |t|.
    KFSegment find_segment(float t) const;

    // Given a |t| and a containing KFSegment, compute the local interpolation weight.
    float compute_weight(const KFSegment& seg, float t) const;

    const std::vector<Keyframe>   fKFs; // Keyframe records, one per AE/Lottie keyframe.
    const std::vector<SkCubicMap> fCMs; // Optional cubic mappers (Bezier interpolation).
    mutable KFSegment             fCurrentSegment = { nullptr, nullptr }; // Cached segment.
};

class AnimatorBuilder : public SkNoncopyable {
public:
    virtual ~AnimatorBuilder();

    virtual sk_sp<KeyframeAnimator> makeFromKeyframes(const AnimationBuilder&,
                                                      const skjson::ArrayValue&) = 0;

    virtual sk_sp<Animator> makeFromExpression(ExpressionManager&, const char*) = 0;

    virtual bool parseValue(const AnimationBuilder&, const skjson::Value&) const = 0;

protected:
    virtual bool parseKFValue(const AnimationBuilder&,
                              const skjson::ObjectValue&,
                              const skjson::Value&,
                              Keyframe::Value*) = 0;

    bool parseKeyframes(const AnimationBuilder&, const skjson::ArrayValue&);

    std::vector<Keyframe>   fKFs; // Keyframe records, one per AE/Lottie keyframe.
    std::vector<SkCubicMap> fCMs; // Optional cubic mappers (Bezier interpolation).

private:
    uint32_t parseMapping(const skjson::ObjectValue&);

    // Track previous cubic map parameters (for deduping).
    SkPoint prev_c0 = { 0, 0 },
            prev_c1 = { 0, 0 };
};

template <typename T>
T Lerp(const T& a, const T& b, float t) { return a + (b - a) * t; }

} // namespace skottie::internal

#endif // SkottieKeyframeAnimator_DEFINED
