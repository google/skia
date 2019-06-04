/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkottieTextAnimator_DEFINED
#define SkottieTextAnimator_DEFINED

#include "include/core/SkRefCnt.h"
#include "modules/skottie/src/SkottieAdapter.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/text/TextAdapter.h"
#include "modules/sksg/include/SkSGScene.h"

#include <memory>
#include <vector>

namespace skottie {
namespace internal {

class AnimationBuilder;
class RangeSelector;

class TextAnimator final : public SkNVRefCnt<TextAnimator> {
public:
    static sk_sp<TextAnimator> Make(const skjson::ObjectValue*,
                                    const AnimationBuilder*,
                                    AnimatorScope*);

    void modulateProps(float scale, TextAdapter::AnimatedProps* dst) const;

    const std::vector<sk_sp<RangeSelector>>& selectors() const { return fSelectors; }

private:
    TextAnimator(std::vector<sk_sp<RangeSelector>>&& selectors,
                 const skjson::ObjectValue& jprops,
                 const AnimationBuilder* abuilder,
                 AnimatorScope* ascope);

    const std::vector<sk_sp<RangeSelector>> fSelectors;

    TextAdapter::AnimatedProps fTextProps;
    bool                       fHasPosition    : 1,
                               fHasScale       : 1,
                               fHasRotation    : 1,
                               fHasFillColor   : 1,
                               fHasStrokeColor : 1,
                               fHasOpacity     : 1;
};

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

    const std::vector<sk_sp<TextAnimator>> fAnimators;
    const sk_sp<TextAdapter>               fAdapter;

    using INHERITED = sksg::GroupAnimator;
};

} // namespace internal
} // namespace skottie

#endif // SkottieTextAnimator_DEFINED
