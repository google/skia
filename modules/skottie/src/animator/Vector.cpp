/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/animator/Keyframe.h"

#include "include/private/SkNx.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/animator/Animator.h"
#include "src/core/SkSafeMath.h"

#include <algorithm>
#include <cstring>

namespace skottie {

// Parses an array of exact size.
static bool parse_array(const skjson::ArrayValue* ja, float* a, size_t count) {
    if (!ja || ja->size() != count) {
        return false;
    }

    for (size_t i = 0; i < count; ++i) {
        if (!Parse((*ja)[i], a + i)) {
            return false;
        }
    }

    return true;
}

template <>
bool ValueTraits<VectorValue>::FromJSON(const skjson::Value& jv, const internal::AnimationBuilder*,
                                        VectorValue* v) {
    if (const skjson::ArrayValue* ja = jv) {
        const auto size = ja->size();
        v->resize(size);
        return parse_array(ja, v->data(), size);
    }

    return false;
}

namespace internal {
namespace {

// Vector specialization - stores float vector values (of same length) in consolidated/contiguous
// storage.  Keyframe records hold the storage offset for each value:
//
// fStorage: [     vec0     ][     vec1     ] ... [     vecN     ]
//            <-  vec_len ->  <-  vec_len ->       <-  vec_len ->
//
//           ^               ^                    ^
// fKFs[]: .idx            .idx       ...       .idx
//
class VectorKeyframeAnimator final : public KeyframeAnimatorBase {
public:
    class Builder final : public KeyframeAnimatorBuilder {
    public:
        sk_sp<KeyframeAnimatorBase> make(const AnimationBuilder& abuilder,
                                         const skjson::ArrayValue& jkfs,
                                         void* target_value) override {
            SkASSERT(jkfs.size() > 0);

            // peek at the first keyframe array value to find our vector length
            const skjson::ObjectValue* jkf0 = jkfs[0];
            const skjson::ArrayValue*  ja   = jkf0
                    ? static_cast<const skjson::ArrayValue*>((*jkf0)["s"])
                    : nullptr;
            if (!ja) {
                return nullptr;
            }

            fVecLen = ja->size();

            SkSafeMath safe;
            // total elements: vector length x number vectors
            const auto total_size = safe.mul(fVecLen, jkfs.size());

            // we must be able to store all offsets in Keyframe::Value::idx (uint32_t)
            if (!safe || !SkTFitsIn<uint32_t>(total_size)) {
                return nullptr;
            }
            fStorage.resize(total_size);

            if (!this->parseKeyframes(abuilder, jkfs)) {
                return nullptr;
            }

            // parseKFValue() might have stored fewer vectors thanks to tail-deduping.
            SkASSERT(fCurrentVec <= jkfs.size());
            fStorage.resize(fCurrentVec * fVecLen);
            fStorage.shrink_to_fit();

            return sk_sp<VectorKeyframeAnimator>(
                        new VectorKeyframeAnimator(std::move(fKFs),
                                                   std::move(fCMs),
                                                   std::move(fStorage),
                                                   fVecLen,
                                                   static_cast<VectorValue*>(target_value)));
        }

        bool parseValue(const AnimationBuilder& abuilder,
                        const skjson::Value& jv,
                        void* v) const override {
            return ValueTraits<VectorValue>::FromJSON(jv, &abuilder, static_cast<VectorValue*>(v));
        }

    private:
        bool parseKFValue(const AnimationBuilder&,
                          const skjson::ObjectValue&,
                          const skjson::Value& jv,
                          Keyframe::Value* kfv) override {
            auto offset = fCurrentVec * fVecLen;
            SkASSERT(offset + fVecLen <= fStorage.size());

            if (!parse_array(jv, fStorage.data() + offset, fVecLen)) {
                return false;
            }

            SkASSERT(!fCurrentVec || offset >= fVecLen);
            // compare with previous vector value
            if (fCurrentVec > 0 && !memcmp(fStorage.data() + offset,
                                           fStorage.data() + offset - fVecLen,
                                           fVecLen * sizeof(float))) {
                // repeating value -> use prev offset (dedupe)
                offset -= fVecLen;
            } else {
                // new value -> advance the current index
                fCurrentVec += 1;
            }

            // Keyframes record the storage-offset for a given vector value.
            kfv->idx = SkToU32(offset);

            return true;
        }

        std::vector<float> fStorage;
        size_t             fVecLen,         // size of individual vector values we store
                           fCurrentVec = 0; // vector value index being parsed (corresponding
                                            // storage offset is fCurrentVec * fVecLen)
    };

private:
    VectorKeyframeAnimator(std::vector<Keyframe> kfs,
                           std::vector<SkCubicMap> cms,
                           std::vector<float> storage,
                           size_t vec_len,
                           VectorValue* target_value)
        : INHERITED(std::move(kfs), std::move(cms))
        , fStorage(std::move(storage))
        , fVecLen(vec_len)
        , fTarget(target_value) {

        // Resize the target value appropriately.
        fTarget->resize(fVecLen);
    }

    StateChanged onSeek(float t) override {
        const auto& lerp_info = this->getLERPInfo(t);

        SkASSERT(lerp_info.vrec0.idx + fVecLen <= fStorage.size());
        SkASSERT(lerp_info.vrec1.idx + fVecLen <= fStorage.size());
        SkASSERT(fTarget->size() == fVecLen);

        const auto* v0  = fStorage.data() + lerp_info.vrec0.idx;
        const auto* v1  = fStorage.data() + lerp_info.vrec1.idx;
              auto* dst = fTarget->data();

        if (lerp_info.isConstant()) {
            if (std::memcmp(dst, v0, fVecLen * sizeof(float))) {
                std::copy(v0, v0 + fVecLen, dst);
                return true;
            }
            return false;
        }

        size_t count = fVecLen;
        bool changed = false;

        while (count >= 4) {
            const auto old_val = Sk4f::Load(dst),
                       new_val = Lerp(Sk4f::Load(v0), Sk4f::Load(v1), lerp_info.weight);

            changed |= (new_val != old_val).anyTrue();
            new_val.store(dst);

            v0    += 4;
            v1    += 4;
            dst   += 4;
            count -= 4;
        }

        while (count-- > 0) {
            const auto new_val = Lerp(*v0++, *v1++, lerp_info.weight);

            changed |= (new_val != *dst);
            *dst++ = new_val;
        }

        return changed;
    }

    const std::vector<float> fStorage;
    const size_t             fVecLen;

    VectorValue*             fTarget;

    using INHERITED = KeyframeAnimatorBase;
};

} // namespace

template <>
bool AnimatablePropertyContainer::bind<VectorValue>(const AnimationBuilder& abuilder,
                                                    const skjson::ObjectValue* jprop,
                                                    VectorValue* v) {
    if (!jprop) {
        return false;
    }

    if (!ParseDefault<bool>((*jprop)["s"], false)) {
        // Regular (static or keyframed) vector value.
        VectorKeyframeAnimator::Builder builder;
        return this->bindImpl(abuilder, jprop, builder, v);
    }

    // Separate-dimensions vector value: each component is animated independently.
    *v = { 0, 0, 0 };
    return this->bind(abuilder, (*jprop)["x"], v->data() + 0)
         | this->bind(abuilder, (*jprop)["y"], v->data() + 1)
         | this->bind(abuilder, (*jprop)["z"], v->data() + 2);
}

} // namespace internal
} // namespace skottie
