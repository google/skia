/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/text/TextValue.h"

#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/SkottieValue.h"

namespace skottie::internal {

bool Parse(const skjson::Value& jv, const internal::AnimationBuilder& abuilder, TextValue* v) {
    const skjson::ObjectValue* jtxt = jv;
    if (!jtxt) {
        return false;
    }

    const skjson::StringValue* font_name   = (*jtxt)["f"];
    const skjson::StringValue* text        = (*jtxt)["t"];
    const skjson::NumberValue* text_size   = (*jtxt)["s"];
    const skjson::NumberValue* line_height = (*jtxt)["lh"];
    if (!font_name || !text || !text_size || !line_height) {
        return false;
    }

    const auto* font = abuilder.findFont(SkString(font_name->begin(), font_name->size()));
    if (!font) {
        abuilder.log(Logger::Level::kError, nullptr, "Unknown font: \"%s\".", font_name->begin());
        return false;
    }

    v->fText.set(text->begin(), text->size());
    v->fTextSize   = **text_size;
    v->fLineHeight = **line_height;
    v->fTypeface   = font->fTypeface;
    v->fFontFamily = font->fFamily;
    v->fAscent     = font->fAscentPct * -0.01f * v->fTextSize; // negative ascent per SkFontMetrics
    v->fLineShift  = ParseDefault((*jtxt)["ls"], 0.0f);

    static constexpr SkTextUtils::Align gAlignMap[] = {
        SkTextUtils::kLeft_Align,  // 'j': 0
        SkTextUtils::kRight_Align, // 'j': 1
        SkTextUtils::kCenter_Align // 'j': 2
    };
    v->fHAlign = gAlignMap[std::min<size_t>(ParseDefault<size_t>((*jtxt)["j"], 0),
                                            std::size(gAlignMap) - 1)];

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

    static constexpr Shaper::Direction gDirectionMap[] = {
        Shaper::Direction::kLTR,  // 'd': 0
        Shaper::Direction::kRTL,  // 'd': 1
    };
    v->fDirection = gDirectionMap[std::min(ParseDefault<size_t>((*jtxt)["d"], 0),
                                           std::size(gDirectionMap) - 1)];

    static constexpr Shaper::ResizePolicy gResizeMap[] = {
        Shaper::ResizePolicy::kNone,           // 'rs': 0
        Shaper::ResizePolicy::kScaleToFit,     // 'rs': 1
        Shaper::ResizePolicy::kDownscaleToFit, // 'rs': 2
    };
    // TODO: remove "sk_rs" support after migrating clients.
    v->fResize = gResizeMap[std::min(std::max(ParseDefault<size_t>((*jtxt)[   "rs"], 0),
                                              ParseDefault<size_t>((*jtxt)["sk_rs"], 0)),
                                     std::size(gResizeMap) - 1)];

    // Optional min/max font size and line count (used when aute-resizing)
    v->fMinTextSize = ParseDefault<SkScalar>((*jtxt)["mf"], 0.0f);
    v->fMaxTextSize = ParseDefault<SkScalar>((*jtxt)["xf"], std::numeric_limits<float>::max());
    v->fMaxLines    = ParseDefault<size_t>  ((*jtxt)["xl"], 0);

    // At the moment, BM uses the paragraph box to discriminate point mode vs. paragraph mode.
    v->fLineBreak = v->fBox.isEmpty()
            ? Shaper::LinebreakPolicy::kExplicit
            : Shaper::LinebreakPolicy::kParagraph;

    // Optional explicit text mode.
    // N.b.: this is not being exported by BM, only used for testing.
    auto text_mode = ParseDefault((*jtxt)["m"], -1);
    if (text_mode >= 0) {
        // Explicit text mode.
        v->fLineBreak = (text_mode == 0)
                ? Shaper::LinebreakPolicy::kExplicit   // 'm': 0 -> point text
                : Shaper::LinebreakPolicy::kParagraph; // 'm': 1 -> paragraph text
    }

    // Optional capitalization.
    static constexpr Shaper::Capitalization gCapMap[] = {
        Shaper::Capitalization::kNone,      // 'ca': 0
        Shaper::Capitalization::kUpperCase, // 'ca': 1
    };
    v->fCapitalization = gCapMap[std::min<size_t>(ParseDefault<size_t>((*jtxt)["ca"], 0),
                                                  std::size(gCapMap) - 1)];

    // In point mode, the text is baseline-aligned.
    v->fVAlign = v->fBox.isEmpty() ? Shaper::VAlign::kTopBaseline
                                   : Shaper::VAlign::kTop;

    static constexpr Shaper::VAlign gVAlignMap[] = {
        Shaper::VAlign::kHybridTop,    // 'vj': 0
        Shaper::VAlign::kHybridCenter, // 'vj': 1
        Shaper::VAlign::kHybridBottom, // 'vj': 2
        Shaper::VAlign::kVisualTop,    // 'vj': 3
        Shaper::VAlign::kVisualCenter, // 'vj': 4
        Shaper::VAlign::kVisualBottom, // 'vj': 5
    };
    size_t vj;
    if (skottie::Parse((*jtxt)["vj"], &vj)) {
        if (vj < std::size(gVAlignMap)) {
            v->fVAlign = gVAlignMap[vj];
        } else {
            abuilder.log(Logger::Level::kWarning, nullptr, "Ignoring unknown 'vj' value: %zu", vj);
        }
    } else if (skottie::Parse((*jtxt)["sk_vj"], &vj)) {
        // Legacy sk_vj values.
        // TODO: remove after clients update.
        switch (vj) {
        case 0:
        case 1:
        case 2:
            static_assert(std::size(gVAlignMap) > 2);
            v->fVAlign = gVAlignMap[vj];
            break;
        case 3:
            // 'sk_vj': 3 -> kHybridCenter/kScaleToFit
            v->fVAlign = Shaper::VAlign::kHybridCenter;
            v->fResize = Shaper::ResizePolicy::kScaleToFit;
            break;
        case 4:
            // 'sk_vj': 4 -> kHybridCenter/kDownscaleToFit
            v->fVAlign = Shaper::VAlign::kHybridCenter;
            v->fResize = Shaper::ResizePolicy::kDownscaleToFit;
            break;
        default:
            abuilder.log(Logger::Level::kWarning, nullptr,
                         "Ignoring unknown 'sk_vj' value: %zu", vj);
            break;
        }
    }

    const auto& parse_color = [] (const skjson::ArrayValue* jcolor,
                                  SkColor* c) {
        if (!jcolor) {
            return false;
        }

        ColorValue color_vec;
        if (!skottie::Parse(*jcolor, static_cast<VectorValue*>(&color_vec))) {
            return false;
        }

        *c = color_vec;
        return true;
    };

    v->fHasFill   = parse_color((*jtxt)["fc"], &v->fFillColor);
    v->fHasStroke = parse_color((*jtxt)["sc"], &v->fStrokeColor);

    if (v->fHasStroke) {
        v->fStrokeWidth = ParseDefault((*jtxt)["sw"], 1.0f);
        v->fPaintOrder  = ParseDefault((*jtxt)["of"], true)
                ? TextPaintOrder::kFillStroke
                : TextPaintOrder::kStrokeFill;

        static constexpr SkPaint::Join gJoins[] = {
            SkPaint::kMiter_Join,  // lj: 1
            SkPaint::kRound_Join,  // lj: 2
            SkPaint::kBevel_Join,  // lj: 3
        };
        v->fStrokeJoin = gJoins[std::min<size_t>(ParseDefault<size_t>((*jtxt)["lj"], 1) - 1,
                                                 std::size(gJoins) - 1)];
    }

    return true;
}

}  // namespace skottie::internal
