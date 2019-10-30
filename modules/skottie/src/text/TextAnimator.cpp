/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/text/TextAnimator.h"

#include "include/core/SkColor.h"
#include "include/core/SkPoint.h"
#include "include/private/SkNx.h"
#include "modules/skottie/src/SkottieValue.h"
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
 *       "s": {...},   // selector node
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
                                       const AnimationBuilder* abuilder) {
    if (!janimator) {
        return nullptr;
    }

    const skjson::ObjectValue* jprops = (*janimator)["a"];
    if (!jprops) {
        return nullptr;
    }

    std::vector<sk_sp<RangeSelector>> selectors;

    // Depending on compat mode and whether more than one selector is present,
    // BM exports either an array or a single object.
    if (const skjson::ArrayValue* jselectors = (*janimator)["s"]) {
        selectors.reserve(jselectors->size());
        for (const skjson::ObjectValue* jselector : *jselectors) {
            if (auto sel = RangeSelector::Make(*jselector, abuilder)) {
                selectors.push_back(std::move(sel));
            }
        }
    } else {
        if (auto sel = RangeSelector::Make((*janimator)["s"], abuilder)) {
            selectors.reserve(1);
            selectors.push_back(std::move(sel));
        }
    }

    return sk_sp<TextAnimator>(new TextAnimator(std::move(selectors), *jprops, abuilder));
}

void TextAnimator::modulateProps(const DomainMaps& maps, ModulatorBuffer& buf) const {
    // No selectors -> full coverage.
    const auto initial_coverage = fSelectors.empty() ? 1.f : 0.f;

    // Coverage is scoped per animator.
    for (auto& mod : buf) {
        mod.coverage = initial_coverage;
    }

    // Accumulate selector coverage.
    for (const auto& selector : fSelectors) {
        selector->modulateCoverage(maps, buf);
    }

    // Modulate animated props.
    for (auto& mod : buf) {
        mod.props = this->modulateProps(mod.props, mod.coverage);
    }
}

TextAnimator::AnimatedProps TextAnimator::modulateProps(const AnimatedProps& props,
                                                        float amount) const {
    auto modulated_props = props;

    // Transform props compose.
    modulated_props.position += fTextProps.position * amount;
    modulated_props.rotation += fTextProps.rotation * amount;
    modulated_props.tracking += fTextProps.tracking * amount;
    modulated_props.scale    *= 1 + (fTextProps.scale - 1) * amount;

    const auto lerp_color = [](SkColor c0, SkColor c1, float t) {
        const auto c0_4f = SkNx_cast<float>(Sk4b::Load(&c0)),
                   c1_4f = SkNx_cast<float>(Sk4b::Load(&c1)),
                    c_4f = c0_4f + (c1_4f - c0_4f) * t;

        SkColor c;
        SkNx_cast<uint8_t>(Sk4f_round(c_4f)).store(&c);
        return c;
    };

    // Colors and opacity are overridden, and use a clamped amount value.
    const auto clamped_amount = std::max(amount, 0.0f);
    if (fHasFillColor) {
        modulated_props.fill_color = lerp_color(props.fill_color,
                                                fTextProps.fill_color,
                                                clamped_amount);
    }
    if (fHasStrokeColor) {
        modulated_props.stroke_color = lerp_color(props.stroke_color,
                                                  fTextProps.stroke_color,
                                                  clamped_amount);
    }
    modulated_props.opacity *= 1 + (fTextProps.opacity - 1) * clamped_amount;

    return modulated_props;
}

TextAnimator::TextAnimator(std::vector<sk_sp<RangeSelector>>&& selectors,
                           const skjson::ObjectValue& jprops,
                           const AnimationBuilder* abuilder)
    : fSelectors(std::move(selectors)) {
    auto* animator = this;

    abuilder->bindProperty<VectorValue>(jprops["p"],
        [animator](const VectorValue& p) {
            animator->fTextProps.position = ValueTraits<VectorValue>::As<SkPoint>(p);
        });
    abuilder->bindProperty<ScalarValue>(jprops["s"],
        [animator](const ScalarValue& s) {
            // Scale is 100-based.
            animator->fTextProps.scale = s * 0.01f;
        });
    abuilder->bindProperty<ScalarValue>(jprops["r"],
        [animator](const ScalarValue& r) {
            animator->fTextProps.rotation = r;
        });
    fHasFillColor   = abuilder->bindProperty<VectorValue>(jprops["fc"],
        [animator](const VectorValue& fc) {
            animator->fTextProps.fill_color = ValueTraits<VectorValue>::As<SkColor>(fc);
        });
    fHasStrokeColor = abuilder->bindProperty<VectorValue>(jprops["sc"],
        [animator](const VectorValue& sc) {
            animator->fTextProps.stroke_color = ValueTraits<VectorValue>::As<SkColor>(sc);
        });
    abuilder->bindProperty<ScalarValue>(jprops["o"],
        [animator](const ScalarValue& o) {
            // Opacity is 100-based.
            animator->fTextProps.opacity = SkTPin<float>(o * 0.01f, 0, 1);
        });
    abuilder->bindProperty<ScalarValue>(jprops["t"],
        [animator](const ScalarValue& t) {
            animator->fTextProps.tracking = t;
        });
}

} // namespace internal
} // namespace skottie
