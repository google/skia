/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/text/TextAnimator.h"

#include "include/core/SkColor.h"
#include "include/core/SkPoint.h"
#include "include/private/SkColorData.h"
#include "modules/skottie/src/SkottieValue.h"
#include "modules/skottie/src/animator/Animator.h"
#include "modules/skottie/src/text/RangeSelector.h"
#include "src/base/SkVx.h"
#include "src/core/SkSwizzlePriv.h"
#include "src/utils/SkJSON.h"

#include <cmath>

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
                                       const AnimationBuilder* abuilder,
                                       AnimatablePropertyContainer* acontainer) {
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
            if (auto sel = RangeSelector::Make(*jselector, abuilder, acontainer)) {
                selectors.push_back(std::move(sel));
            }
        }
    } else {
        if (auto sel = RangeSelector::Make((*janimator)["s"], abuilder, acontainer)) {
            selectors.reserve(1);
            selectors.push_back(std::move(sel));
        }
    }

    return sk_sp<TextAnimator>(
                new TextAnimator(std::move(selectors), *jprops, abuilder, acontainer));
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

TextAnimator::ResolvedProps TextAnimator::modulateProps(const ResolvedProps& props,
                                                        float amount) const {
    auto modulated_props = props;

    // Transform props compose.
    modulated_props.position += static_cast<SkV3>(fTextProps.position) * amount;
    modulated_props.rotation += fTextProps.rotation * amount;
    modulated_props.tracking += fTextProps.tracking * amount;
    modulated_props.scale    *= SkV3{1,1,1} +
            (static_cast<SkV3>(fTextProps.scale) * 0.01f - SkV3{1,1,1}) * amount;

    // ... as do blur, line spacing, and stroke width.
    modulated_props.blur         += fTextProps.blur         * amount;
    modulated_props.line_spacing += fTextProps.line_spacing * amount;
    modulated_props.stroke_width += fTextProps.stroke_width * amount;

    const auto lerp = [](float v0, float v1, float t) {
        return v0 + (v1 - v0)*t;
    };
    const auto lerp_color = [](SkColor c0, SkColor c1, float t) {
        const auto c0_4f = Sk4f_fromL32(c0),
                   c1_4f = Sk4f_fromL32(c1),
                    c_4f = c0_4f + (c1_4f - c0_4f) * t;

        return Sk4f_toL32(c_4f);
    };

    // Colors and opacity are interpolated, and use a clamped amount value.
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
    if (fHasFillOpacity) {
        // 255-based
        const auto alpha = lerp(SkColorGetA(props.fill_color),
                                fTextProps.fill_opacity*2.55f,
                                clamped_amount);
        modulated_props.fill_color = SkColorSetA(modulated_props.fill_color,
                                                 static_cast<U8CPU>(std::round(alpha)));
    }
    if (fHasStrokeOpacity) {
        // 255-based
        const auto alpha = lerp(SkColorGetA(props.stroke_color),
                                fTextProps.stroke_opacity*2.55f,
                                clamped_amount);
        modulated_props.stroke_color = SkColorSetA(modulated_props.stroke_color,
                                                   static_cast<U8CPU>(std::round(alpha)));
    }
    if (fHasOpacity) {
        modulated_props.opacity = lerp(props.opacity, fTextProps.opacity*0.01f, clamped_amount);
    }

    return modulated_props;
}

TextAnimator::TextAnimator(std::vector<sk_sp<RangeSelector>>&& selectors,
                           const skjson::ObjectValue& jprops,
                           const AnimationBuilder* abuilder,
                           AnimatablePropertyContainer* acontainer)
    : fSelectors(std::move(selectors))
    , fRequiresAnchorPoint(false)
    , fRequiresLineAdjustments(false) {

    acontainer->bind(*abuilder, jprops["p" ], fTextProps.position);

    // Tracking and line spacing affect all line fragments.
    fRequiresLineAdjustments |= acontainer->bind(*abuilder, jprops["t" ], fTextProps.tracking);
    fRequiresLineAdjustments |= acontainer->bind(*abuilder, jprops["ls"], fTextProps.line_spacing);

    // Scale and rotation are anchor-point-dependent.
    fRequiresAnchorPoint |= acontainer->bind(*abuilder, jprops["s"], fTextProps.scale);

    // Depending on whether we're in 2D/3D mode, some of these will stick and some will not.
    // It's fine either way.
    fRequiresAnchorPoint |= acontainer->bind(*abuilder, jprops["rx"], fTextProps.rotation.x);
    fRequiresAnchorPoint |= acontainer->bind(*abuilder, jprops["ry"], fTextProps.rotation.y);
    fRequiresAnchorPoint |= acontainer->bind(*abuilder, jprops["r" ], fTextProps.rotation.z);

    fHasFillColor     = acontainer->bind(*abuilder, jprops["fc"], fTextProps.fill_color    );
    fHasStrokeColor   = acontainer->bind(*abuilder, jprops["sc"], fTextProps.stroke_color  );
    fHasFillOpacity   = acontainer->bind(*abuilder, jprops["fo"], fTextProps.fill_opacity  );
    fHasStrokeOpacity = acontainer->bind(*abuilder, jprops["so"], fTextProps.stroke_opacity);
    fHasOpacity       = acontainer->bind(*abuilder, jprops["o" ], fTextProps.opacity       );
    fHasBlur          = acontainer->bind(*abuilder, jprops["bl"], fTextProps.blur          );

    acontainer->bind(*abuilder, jprops["sw"], fTextProps.stroke_width);
}

} // namespace internal
} // namespace skottie
