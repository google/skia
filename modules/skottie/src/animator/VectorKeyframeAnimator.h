/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieVectorKeyframeAnimator_DEFINED
#define SkottieVectorKeyframeAnimator_DEFINED

#include "modules/skottie/src/animator/KeyframeAnimator.h"

#include <vector>

namespace skottie::internal {

class VectorKeyframeAnimatorBuilder final : public KeyframeAnimatorBuilder {
public:
    using VectorLenParser  = bool(*)(const skjson::Value&, size_t*);
    using VectorDataParser = bool(*)(const skjson::Value&, size_t, float*);

    VectorKeyframeAnimatorBuilder(std::vector<float>*, VectorLenParser, VectorDataParser);

    sk_sp<KeyframeAnimator> make(const AnimationBuilder&, const skjson::ArrayValue&) override;

private:
    bool parseValue(const AnimationBuilder&, const skjson::Value&) const override;

    bool parseKFValue(const AnimationBuilder&,
                      const skjson::ObjectValue&,
                      const skjson::Value&,
                      Keyframe::Value*) override;

    const VectorLenParser  fParseLen;
    const VectorDataParser fParseData;

    std::vector<float>     fStorage;
    size_t                 fVecLen,         // size of individual vector values we store
                           fCurrentVec = 0; // vector value index being parsed (corresponding
                                            // storage offset is fCurrentVec * fVecLen)
    std::vector<float>*    fTarget;
};

} // namespace skottie::internal

#endif // SkottieVectorKeyframeAnimator_DEFINED
