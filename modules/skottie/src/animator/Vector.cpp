/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/animator/Keyframe.h"

#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/animator/Animator.h"
#include "src/core/SkSafeMath.h"

#include <algorithm>
#include <cstring>

namespace skottie::internal {

namespace  {

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

            if (fVIndex != jkfs.size()) {
                // shrink storage to match deduping
                SkASSERT(fVIndex < jkfs.size());
                const auto new_size = fVIndex * fVecSize;
                      auto new_storage = new float[new_size];
                std::copy(fStorage.get(), fStorage.get() + new_size, new_storage);
                fStorage.reset(new_storage);
            }

            return sk_sp<VectorKeyframeAnimator>(
                        new VectorKeyframeAnimator(std::move(fKFs),
                                                   std::move(fCMs),
                                                   std::move(fStorage),
                                                   fVecSize,
                                                   static_cast<VecValue*>(target_value)));
        }

        bool parseValue(const AnimationBuilder& abuilder,
                        const skjson::Value& jv,
                        void* v) const override {
            return ValueTraits<VecValue>::FromJSON(jv, &abuilder, static_cast<VecValue*>(v));
        }

    private:
        bool parseKFValue(const AnimationBuilder&,
                          const skjson::ObjectValue&,
                          const skjson::Value& jv,
                          Keyframe::Value* v) override {
            auto dst_idx = fVIndex * fVecSize;
            if (!ParseArray(jv, fStorage.get() + dst_idx, fVecSize)) {
                return false;
            }

            SkASSERT(!fVIndex || dst_idx >= fVecSize);
            if (fVIndex > 0 && !memcmp(fStorage.get() + dst_idx,
                                       fStorage.get() + dst_idx - fVecSize,
                                       fVecSize * sizeof(float))) {
                // dedupe vs. prev vector value
                dst_idx -= fVecSize;
            } else {
                // new vector value
                fVIndex += 1;
            }

            v->idx = SkToU32(dst_idx);

            return true;
        }

        std::unique_ptr<float[]> fStorage;
        size_t                   fVecSize,
                                 fVIndex = 0;
    };

private:
    VectorKeyframeAnimator(std::vector<Keyframe> kfs,
                           std::vector<SkCubicMap> cms,
                           std::unique_ptr<float[]> storage,
                           size_t vec_size,
                           VecValue* target_value)
        : INHERITED(std::move(kfs), std::move(cms))
        , fStorage(std::move(storage))
        , fVecSize(vec_size)
        , fTarget(target_value) {
        fTarget->realloc(fVecSize);
    }

    void onTick(float t) override {
        const auto& lerp_info = this->getLERPInfo(t);

        if (lerp_info.isConstant()) {
            std::copy(fStorage.get() + lerp_info.vrec0.idx,
                      fStorage.get() + lerp_info.vrec0.idx + fVecSize,
                      fTarget->data());
            return;
        }

        // TODO: Sk4f
        for (size_t i = 0; i < fVecSize; ++i) {
            (*fTarget)[i] = LERP(fStorage[lerp_info.vrec0.idx + i],
                                 fStorage[lerp_info.vrec1.idx + i],
                                 lerp_info.weight);
        }
    }

    const std::unique_ptr<float[]> fStorage;
    const size_t                   fVecSize;

    VecValue*                      fTarget;

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