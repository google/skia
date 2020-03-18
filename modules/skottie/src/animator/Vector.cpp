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

namespace skottie::internal {

namespace  {

// Vector specialization - stores vector values in a consolidated/contiguous
// array.  Keyframe records hold the storage-offset for each value:
//
// fStorage: [     vec0     ][     vec1     ] ... [     vecN     ]
//            <- vec_size ->  <- vec_size ->       <- vec_size ->
//
//           ^               ^                    ^
// fKfs[]: v.idx           v.idx       ...      v.idx
//
class VectorKeyframeAnimator final : public KeyframeAnimatorBase {
public:
    class Builder final : public KeyframeAnimatorBuilder {
    public:
        sk_sp<KeyframeAnimatorBase> make(const AnimationBuilder& abuilder,
                                         const skjson::ArrayValue& jkfs,
                                         void* target_value) override {
            SkASSERT(jkfs.size() > 0);

            // peek at the first keyframe array value to find our vector size
            const skjson::ObjectValue* jkf0 = jkfs[0];
            const skjson::ArrayValue*  ja   = jkf0
                    ? static_cast<const skjson::ArrayValue*>((*jkf0)["s"])
                    : nullptr;
            if (!ja) {
                return nullptr;
            }

            fVecSize = ja->size();

            SkSafeMath safe;
            const auto total_size = safe.mul(fVecSize, jkfs.size());
            if (!safe || !SkTFitsIn<uint32_t>(total_size)) {
                return nullptr;
            }
            fStorage.reset(new float[total_size]);

            if (!this->parseKeyframes(abuilder, jkfs)) {
                return nullptr;
            }

            // shrink storage for potential deduping
            if (fVIndex != jkfs.size()) {
                SkASSERT(fVIndex < jkfs.size());
                const auto new_size    = fVIndex * fVecSize;
                      auto new_storage = new float[new_size];
                std::copy(fStorage.get(), fStorage.get() + new_size, new_storage);
                fStorage.reset(new_storage);
            }

            return sk_sp<VectorKeyframeAnimator>(
                        new VectorKeyframeAnimator(std::move(fKFs),
                                                   std::move(fCMs),
                                                   std::move(fStorage),
                                                   fVecSize,
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
            auto offset = fVIndex * fVecSize;
            if (!ParseArray(jv, fStorage.get() + offset, fVecSize)) {
                return false;
            }

            SkASSERT(!fVIndex || offset >= fVecSize);
            // compare with previous vector value
            if (fVIndex > 0 && !memcmp(fStorage.get() + offset,
                                       fStorage.get() + offset - fVecSize,
                                       fVecSize * sizeof(float))) {
                // repeating value -> use prev offset (dedupe)
                offset -= fVecSize;
            } else {
                // new value -> advance the index
                fVIndex += 1;
            }

            // Keyframes record the storage-offset for a given vector value.
            kfv->idx = SkToU32(offset);

            return true;
        }

        std::unique_ptr<float[]> fStorage;
        size_t                   fVecSize,
                                 fVIndex = 0; // tracks the value index being parsed
    };

private:
    VectorKeyframeAnimator(std::vector<Keyframe> kfs,
                           std::vector<SkCubicMap> cms,
                           std::unique_ptr<float[]> storage,
                           size_t vec_size,
                           VectorValue* target_value)
        : INHERITED(std::move(kfs), std::move(cms))
        , fStorage(std::move(storage))
        , fVecSize(vec_size)
        , fTarget(target_value) {

        // Resize the target value appropriately.
        fTarget->realloc(fVecSize);
    }

    void onTick(float t) override {
        const auto& lerp_info = this->getLERPInfo(t);

        SkASSERT(fTarget->size() == fVecSize);

        if (lerp_info.isConstant()) {
            std::copy(fStorage.get() + lerp_info.vrec0.idx,
                      fStorage.get() + lerp_info.vrec0.idx + fVecSize,
                      fTarget->data());
            return;
        }

        // Element-wise interpolation.
        const auto* v0  = fStorage.get() + lerp_info.vrec0.idx;
        const auto* v1  = fStorage.get() + lerp_info.vrec1.idx;
              auto* dst = fTarget->data();

        size_t count = fVecSize;

        while (count >= 4) {
            LERP(Sk4f::Load(v0), Sk4f::Load(v1), lerp_info.weight)
                .store(dst);

            v0    += 4;
            v1    += 4;
            dst   += 4;
            count -= 4;
        }

        while (count-- > 0) {
            *dst++ = LERP(*v0++, *v1++, lerp_info.weight);
        }
    }

    const std::unique_ptr<float[]> fStorage;
    const size_t                   fVecSize;

    VectorValue*                   fTarget;

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

} // namespace skottie::internal
