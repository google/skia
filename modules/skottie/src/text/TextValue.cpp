/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/text/TextValue.h"

#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"

namespace skottie {

bool TextValue::operator==(const TextValue &other) const {
    return fTypeface == other.fTypeface
        && fText == other.fText
        && fTextSize == other.fTextSize
        && fStrokeWidth == other.fStrokeWidth
        && fLineHeight == other.fLineHeight
        && fHAlign == other.fHAlign
        && fVAlign == other.fVAlign
        && fBox == other.fBox
        && fFillColor == other.fFillColor
        && fStrokeColor == other.fStrokeColor
        && fHasFill == other.fHasFill
        && fHasStroke == other.fHasStroke;
}

bool TextValue::operator!=(const TextValue &other) const {
    return !(*this== other);
}

template <>
bool ValueTraits<TextValue>::FromJSON(const skjson::Value& jv,
                                       const internal::AnimationBuilder* abuilder,
                                       TextValue* v) {
    const skjson::ObjectValue* jtxt = jv;
    if (!jtxt) {
        return false;
    }

    const skjson::StringValue* font_name   = (*jtxt)["f"];
    const skjson::StringValue* text        = (*jtxt)["t"];
    const skjson::NumberValue* text_size   = (*jtxt)["s"];
    const skjson::NumberValue* line_height = (*jtxt)["lh"];
    if (!font_name || !text || !text_size || !line_height ||
        !(v->fTypeface = abuilder->findFont(SkString(font_name->begin(), font_name->size())))) {
        return false;
    }
    v->fText.set(text->begin(), text->size());
    v->fTextSize   = **text_size;
    v->fLineHeight = **line_height;

    static constexpr SkTextUtils::Align gAlignMap[] = {
        SkTextUtils::kLeft_Align,  // 'j': 0
        SkTextUtils::kRight_Align, // 'j': 1
        SkTextUtils::kCenter_Align // 'j': 2
    };
    v->fHAlign = gAlignMap[SkTMin<size_t>(ParseDefault<size_t>((*jtxt)["j"], 0),
                                          SK_ARRAY_COUNT(gAlignMap))];

    // Optional text box size.
    if (const skjson::ArrayValue* jsz = (*jtxt)["sz"]) {
        if (jsz->size() == 2) {
            v->fBox.setWH(ParseDefault<SkScalar>((*jsz)[0], 0),
                          ParseDefault<SkScalar>((*jsz)[1], 0));
        }
    }

    // Optional text box position.
    if (const skjson::ArrayValue* jps = (*jtxt)["ps"]) {
        if (jps->size() == 2) {
            v->fBox.offset(ParseDefault<SkScalar>((*jps)[0], 0),
                           ParseDefault<SkScalar>((*jps)[1], 0));
        }
    }

    // In point mode, the text is baseline-aligned.
    v->fVAlign = v->fBox.isEmpty() ? Shaper::VAlign::kTopBaseline
                                   : Shaper::VAlign::kTop;

    // Skia vertical alignment extension "sk_vj":
    static constexpr Shaper::VAlign gVAlignMap[] = {
        Shaper::VAlign::kTop,         // 'sk_vj': 0
        Shaper::VAlign::kCenter,      // 'sk_vj': 1
        Shaper::VAlign::kBottom,      // 'sk_vj': 2
        Shaper::VAlign::kResizeToFit, // 'sk_vj': 3
    };
    size_t sk_vj;
    if (Parse((*jtxt)["sk_vj"], &sk_vj) && sk_vj < SK_ARRAY_COUNT(gVAlignMap)) {
        v->fVAlign = gVAlignMap[sk_vj];
    }

    const auto& parse_color = [] (const skjson::ArrayValue* jcolor,
                                  const internal::AnimationBuilder* abuilder,
                                  SkColor* c) {
        if (!jcolor) {
            return false;
        }

        VectorValue color_vec;
        if (!ValueTraits<VectorValue>::FromJSON(*jcolor, abuilder, &color_vec)) {
            return false;
        }

        *c = ValueTraits<VectorValue>::As<SkColor>(color_vec);
        return true;
    };

    v->fHasFill   = parse_color((*jtxt)["fc"], abuilder, &v->fFillColor);
    v->fHasStroke = parse_color((*jtxt)["sc"], abuilder, &v->fStrokeColor);

    if (v->fHasStroke) {
        v->fStrokeWidth = ParseDefault((*jtxt)["s"], 0.0f);
    }

    return true;
}

template <>
bool ValueTraits<TextValue>::CanLerp(const TextValue&, const TextValue&) {
    // Text values are never interpolated, but we pretend that they could be.
    return true;
}

template <>
void ValueTraits<TextValue>::Lerp(const TextValue& v0, const TextValue&, float, TextValue* result) {
    // Text value keyframes are treated as selectors, not as interpolated values.
    *result = v0;
}

} // namespace skottie
