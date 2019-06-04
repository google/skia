/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/text/TextAnimator.h"

#include "include/core/SkColor.h"
#include "include/core/SkPoint.h"
#include "modules/skottie/src/text/RangeSelector.h"
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
sk_sp<TextAnimator> TextAnimator::Make(const skjson::ObjectValue* janimator,
                                       const AnimationBuilder* abuilder,
                                       AnimatorScope* ascope) {
    if (!janimator) {
        return nullptr;
    }

    std::vector<sk_sp<RangeSelector>> selectors;
    if (const skjson::ObjectValue* jselector = (*janimator)["s"]) {
        // Single range selector for now.
        if (auto sel = RangeSelector::Make(jselector, abuilder, ascope)) {
            selectors.reserve(1);
            selectors.push_back(std::move(sel));
        }
    }

    const skjson::ObjectValue* jprops = (*janimator)["a"];

    return jprops
        ? sk_sp<TextAnimator>(new TextAnimator(std::move(selectors), *jprops, abuilder, ascope))
        : nullptr;
}

void TextAnimator::modulateProps(float scale, TextAdapter::AnimatedProps* dst) const {
    // Transform props compose.
    if (fHasPosition) {
        dst->position += fTextProps.position * scale;
    }
    if (fHasScale) {
        dst->scale *= fTextProps.scale;
    }
    if (fHasRotation) {
        dst->rotation += fTextProps.rotation;
    }

    // Colors and opacity are overridden.
    if (fHasFillColor) {
        dst->fill_color = fTextProps.fill_color;
    }
    if (fHasStrokeColor) {
        dst->stroke_color = fTextProps.stroke_color;
    }
    if (fHasOpacity) {
        dst->opacity = fTextProps.opacity;
    }
}

TextAnimator::TextAnimator(std::vector<sk_sp<RangeSelector>>&& selectors,
                           const skjson::ObjectValue& jprops,
                           const AnimationBuilder* abuilder,
                           AnimatorScope* ascope)
    : fSelectors(std::move(selectors)) {

    // It's *probably* OK to capture a raw pointer to this animator, because the lambda
    // life time is limited to |ascope|, which is also the container for the TextAnimatorList
    // owning us. But for peace of mind (and future-proofing) let's grab a ref.
    auto animator = sk_ref_sp(this);

    fHasPosition    = abuilder->bindProperty<VectorValue>(jprops["p"], ascope,
        [animator](const VectorValue& p) {
            animator->fTextProps.position = ValueTraits<VectorValue>::As<SkPoint>(p);
        });
    fHasScale       = abuilder->bindProperty<ScalarValue>(jprops["s"], ascope,
        [animator](const ScalarValue& s) {
            // Scale is 100-based.
            animator->fTextProps.scale = s * 0.01f;
        });
    fHasRotation    = abuilder->bindProperty<ScalarValue>(jprops["r"], ascope,
        [animator](const ScalarValue& r) {
            animator->fTextProps.rotation = r;
        });
    fHasFillColor   = abuilder->bindProperty<VectorValue>(jprops["fc"], ascope,
        [animator](const VectorValue& fc) {
            animator->fTextProps.fill_color = ValueTraits<VectorValue>::As<SkColor>(fc);
        });
    fHasStrokeColor = abuilder->bindProperty<VectorValue>(jprops["sc"], ascope,
        [animator](const VectorValue& sc) {
            animator->fTextProps.stroke_color = ValueTraits<VectorValue>::As<SkColor>(sc);
    });
    fHasOpacity     = abuilder->bindProperty<ScalarValue>(jprops["o"], ascope,
        [animator](const ScalarValue& o) {
            // Opacity is 100-based.
            animator->fTextProps.opacity = SkTPin<float>(o * 0.01f, 0, 1);
        });
}

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
    fAdapter->applyAnimators(fAnimators);
}

} // namespace internal
} // namespace skottie
