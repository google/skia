/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieTextAnimator_DEFINED
#define SkottieTextAnimator_DEFINED

#include "modules/skottie/src/SkottieAdapter.h"
#include "modules/sksg/include/SkSGScene.h"

#include <memory>
#include <vector>

namespace skjson {
class ArrayValue;
}

namespace skottie {

class TextAdapter;

namespace internal {

class AnimationBuilder;
class TextAnimator;

class TextAnimatorList final : public sksg::GroupAnimator {
public:
    static std::unique_ptr<TextAnimatorList> Make(const skjson::ArrayValue&,
                                                  const AnimationBuilder*,
                                                  sk_sp<TextAdapter>);
    ~TextAnimatorList() override;

protected:
    void onTick(float) override;

private:
    TextAnimatorList(sk_sp<TextAdapter>, sksg::AnimatorList&&, std::vector<sk_sp<TextAnimator>>&&);

    void applyAnimators() const;

    const std::vector<sk_sp<TextAnimator>> fAnimators;
    const sk_sp<TextAdapter>               fAdapter;

    using INHERITED = sksg::GroupAnimator;
};

} // namespace internal
} // namespace skottie

#endif // SkottieTextAnimator_DEFINED
