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

/*
 * Text layers can have optional text property animators.
 *
 * Each animator consists of
 *
 *   1) a list of animated properties (e.g. position, fill color, etc)
 *
 *   2) a list of range selectors
 *
 * Animated properties yield new values to be applied to the text, while range selectors
 * determine the text subset these new values are applied to.
 *
 * The best way to think of range selectors is in terms of coverage: they combine to generate
 * a coverage value [0..1] for each text fragment/glyph.  This coverage is then used to modulate
 * how the new property value is applied to a given fragment (interpolation weight).
 *
 * Note: Bodymovin currently only supports a single selector.
 *
 * JSON structure:
 *
 * "t": {              // text node
 *   "a": [            // animators list
 *     {               // animator node
 *       "s": {...},   // selector node (TODO)
 *       "a": {        // animator properties node
 *         "a":  {}    // optional anchor point value
 *         "p":  {},   // optional position value
 *         "s":  {},   // optional scale value
 *         "o":  {},   // optional opacity
 *         "fc": {},   // optional fill color value
 *         "sc": {},   // optional stroke color value
 *
 *         // TODO: more props?
 *       }
 *     },
 *     ...
 *   ],
 *   ...
 * }
 */
class TextAnimator final : public SkNVRefCnt<TextAnimator> {
public:
    static sk_sp<TextAnimator> Make(const skjson::ObjectValue* janimator,
                                    const AnimationBuilder* abuilder,
                                    AnimatorScope* ascope) {
        if (!janimator) {
            return nullptr;
        }

        if (const skjson::ObjectValue* jselector = (*janimator)["s"]) {
            abuilder->log(Logger::Level::kWarning, jselector, "Unsupported text range selector.");
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

        // Colors are overridden.
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
        // It's *probably* OK to capture a raw pointer to this animator, because the lambda
        // life time is limited to |ascope|, which is also the container for the TextAnimatorList
        // owning us. But for peace of mind (and future-proofing) let's grab a ref.
        auto animator = sk_ref_sp(this);

        fHasPosition    = abuilder->bindProperty<VectorValue>(jprops["p"], ascope,
            [animator](const VectorValue& p) {
                animator->fTextProps.position = ValueTraits<VectorValue>::As<SkPoint>(p);
            });
        fHasFillColor   = abuilder->bindProperty<VectorValue>(jprops["fc"], ascope,
            [animator](const VectorValue& fc) {
                animator->fTextProps.fill_color = ValueTraits<VectorValue>::As<SkColor>(fc);
            });
        fHasStrokeColor = abuilder->bindProperty<VectorValue>(jprops["sc"], ascope,
            [animator](const VectorValue& sc) {
                animator->fTextProps.stroke_color = ValueTraits<VectorValue>::As<SkColor>(sc);
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
