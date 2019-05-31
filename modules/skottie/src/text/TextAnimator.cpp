/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/text/TextAnimator.h"

#include "include/core/SkColor.h"
#include "include/core/SkPoint.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/text/TextAdapter.h"
#include "src/utils/SkJSON.h"

namespace skottie {
namespace internal {

class TextAnimator final : public SkNVRefCnt<TextAnimator> {
public:
    static sk_sp<TextAnimator> Make(const skjson::ObjectValue* janimator,
                                    const AnimationBuilder* abuilder,
                                    AnimatorScope* ascope) {
        if (!janimator) {
            return nullptr;
        }

        const skjson::ObjectValue* jprops = (*janimator)["a"];

        return jprops
            ? sk_sp<TextAnimator>(new TextAnimator(*jprops, abuilder, ascope))
            : nullptr;
    }

    void modulateProps(TextAdapter::AnimatedProps* dst) const {
        // Position is additive.
        if (fHasPosition) {
            dst->position += fTextProps.position;
        }

        // Colors are overriden.
        if (fHasFillColor) {
            dst->fill_color = fTextProps.fill_color;
        }
        if (fHasStrokeColor) {
            dst->stroke_color = fTextProps.stroke_color;
        }
    }

private:
    TextAnimator(const skjson::ObjectValue& jprops,
                          const AnimationBuilder* abuilder,
                          AnimatorScope* ascope) {
        fHasPosition    = abuilder->bindProperty<VectorValue>(jprops["p"], ascope,
            [this](const VectorValue& p) {
                fTextProps.position = ValueTraits<VectorValue>::As<SkPoint>(p);
            });
        fHasFillColor   = abuilder->bindProperty<VectorValue>(jprops["fc"], ascope,
            [this](const VectorValue& fc) {
                fTextProps.fill_color = ValueTraits<VectorValue>::As<SkColor>(fc);
            });
        fHasStrokeColor = abuilder->bindProperty<VectorValue>(jprops["sc"], ascope,
            [this](const VectorValue& sc) {
                fTextProps.stroke_color = ValueTraits<VectorValue>::As<SkColor>(sc);
            });
    }

    TextAdapter::AnimatedProps fTextProps;
    bool                       fHasPosition    : 1,
                               fHasFillColor   : 1,
                               fHasStrokeColor : 1;
};

std::unique_ptr<TextAnimatorList> TextAnimatorList::Make(const skjson::ArrayValue& janimators,
                                                         const AnimationBuilder* abuilder,
                                                         sk_sp<TextAdapter> adapter) {
    AnimatorScope local_animator_scope;
    std::vector<sk_sp<TextAnimator>> animators;
    animators.reserve(janimators.size());

    for (const skjson::ObjectValue* janimator : janimators) {
        if (auto animator = TextAnimator::Make(janimator, abuilder, &local_animator_scope)) {
            animators.push_back(std::move(animator));
        }
    }

    if (animators.empty()) {
        return nullptr;
    }

    return std::unique_ptr<TextAnimatorList>(new TextAnimatorList(std::move(adapter),
                                                                  std::move(local_animator_scope),
                                                                  std::move(animators)));
}

TextAnimatorList::TextAnimatorList(sk_sp<TextAdapter> adapter,
                                   sksg::AnimatorList&& alist,
                                   std::vector<sk_sp<TextAnimator>>&& tanimators)
    : INHERITED(std::move(alist))
    , fAnimators(std::move(tanimators))
    , fAdapter(std::move(adapter)) {}

TextAnimatorList::~TextAnimatorList() = default;

void TextAnimatorList::onTick(float t) {
    // First, update all locally-scoped animated props.
    this->INHERITED::onTick(t);

    // Then push the final property values to the text adapter.
    this->applyAnimators();
}

void TextAnimatorList::applyAnimators() const {
    const auto& txt_val = fAdapter->getText();

    // Seed props from the current text value.
    TextAdapter::AnimatedProps modulated_props;
    modulated_props.fill_color   = txt_val.fFillColor;
    modulated_props.stroke_color = txt_val.fStrokeColor;

    for (const auto& animator : fAnimators) {
        animator->modulateProps(&modulated_props);
    }

    fAdapter->applyAnimatedProps(modulated_props);
}

} // namespace internal
} // namespace skottie
