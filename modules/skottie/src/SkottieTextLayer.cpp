/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottiePriv.h"

#include "SkFontMgr.h"
#include "SkMakeUnique.h"
#include "SkottieJson.h"
#include "SkottieValue.h"
#include "SkSGColor.h"
#include "SkSGDraw.h"
#include "SkSGGroup.h"
#include "SkSGText.h"
#include "SkTypes.h"

namespace skottie {
namespace internal {

namespace {

bool ParseGlyph(const skjson::ObjectValue* jglyph, FontInfo* finfo) {
    if (!finfo->fGlyphs) {
        finfo->fGlyphs = skstd::make_unique<GlyphMap>();
    }

    // TODO: add support for glyph text.

    return true;
}

SkFontStyle FontStyle(const char style[]) {
    if (!strcasecmp(style, "regular")) {
        return SkFontStyle::Normal();
    }
    if (!strcasecmp(style, "bold")) {
        return SkFontStyle::Bold();
    }
    if (!strcasecmp(style, "light")) {
        return SkFontStyle(SkFontStyle::kNormal_Weight,
                           SkFontStyle::kNormal_Width,
                           SkFontStyle::kUpright_Slant);
    }
    if (!strcasecmp(style, "oblique")) {
        return SkFontStyle::Italic();
    }
    if (!strcasecmp(style, "bold oblique")) {
        return SkFontStyle::BoldItalic();
    }
    if (!strcasecmp(style, "light oblique")) {
        return SkFontStyle(SkFontStyle::kNormal_Weight,
                           SkFontStyle::kNormal_Width,
                           SkFontStyle::kItalic_Slant);
    }

    LOG("!! Unknown font style: %s\n", style);
    return SkFontStyle::Normal();
}

} // namespace

bool FontInfo::matches(const char family[], const SkFontStyle& style) const {
    return 0 == strcmp(fFamily.c_str(), family) && fStyle == style;
}

FontMap ParseFonts(const skjson::ObjectValue* jfonts, const skjson::ArrayValue* jglyphs,
                   const SkFontMgr* fontmgr) {
    FontMap fonts;

    if (jfonts) {
        if(const skjson::ArrayValue* jlist = (*jfonts)["list"]) {
            for (const skjson::ObjectValue* jfont : *jlist) {
                if (!jfont) {
                    continue;
                }

                const skjson::StringValue* jname   = (*jfont)["fName"],
                                         * jfamily = (*jfont)["fFamily"],
                                         * jstyle  = (*jfont)["fStyle"];

                if (!jname   || !jname->size() ||
                    !jfamily || !jfamily->size() ||
                    !jstyle  || !jstyle->size()) {
                    LogJSON(*jfont, "!! Ignoring invalid font");
                    continue;
                }

                auto style = FontStyle(jstyle->begin());
                sk_sp<SkTypeface> tf(fontmgr->matchFamilyStyle(jfamily->begin(), style));
                if (!tf) {
                    LOG("!! Could not create typeface for %s|%s\n",
                        jfamily->begin(), jstyle->begin());
                    continue;
                }

                FontInfo finfo = {
                    SkString(jfamily->begin(), jfamily->size()),
                    style,
                    ParseDefault((*jfont)["ascent"] , 0.0f),
                    std::move(tf),
                    nullptr
                };
                fonts.set(SkString(jname->begin(), jname->size()), std::move(finfo));
            }
        }
    }

    if (jglyphs) {
        FontInfo* current_font = nullptr;

        for (const skjson::ObjectValue* jglyph : *jglyphs) {
            if (!jglyph) {
                continue;
            }

            const skjson::StringValue* jch = (*jglyph)["ch"];
            if (!jch) {
                continue;
            }

            const skjson::StringValue* jfamily = (*jglyph)["fFamily"];
            const skjson::StringValue* jstyle  = (*jglyph)["style"];

            const auto* ch_ptr = jch->begin();
            const auto  ch_len = jch->size();

            if (!jfamily || !jstyle || (SkUTF::CountUTF8(ch_ptr, ch_len) != 1)) {
                LogJSON(*jglyph, "!! Invalid glyph");
                continue;
            }

            const auto ch = SkUTF::NextUTF8(&ch_ptr, ch_ptr + ch_len);
            SkASSERT(ch != -1);

            const auto family = jfamily->begin();
            const auto style  = FontStyle(jstyle->begin());

            LOG("** glyph found: %d (%s %s)\n", ch, family, jstyle->begin());

            // Locate (and cache) the font info.
            // TODO: this performs a linear search over all fonts; if it becomes problematic,
            //       we can refactor as a two-level map.
            if (!current_font || !current_font->matches(family, style)) {
                current_font = nullptr;
                fonts.foreach([&](const SkString& name, FontInfo* finfo) {
                    if (finfo->matches(family, style)) {
                        current_font = finfo;
                        // TODO: would be nice to break early here...
                    }
                });
                if (!current_font) {
                    LOG("!! Font not found for glyph (%d, %s, %s)\n", ch, family, jstyle->begin());
                    continue;
                }
            }

            if (!ParseGlyph(*jglyph, current_font)) {
                LogJSON(*jglyph, "!! Invalid glyph");
            }
        }
    }

    return fonts;
}

sk_sp<sksg::RenderNode> AttachTextLayer(const skjson::ObjectValue& layer, AttachContext* ctx) {
    const skjson::ObjectValue* jt = layer["t"];
    if (!jt) {
        LogJSON(layer, "!! Missing text layer \"t\" property");
        return nullptr;
    }

    const skjson::ArrayValue* animated_props = (*jt)["a"];
    if (animated_props && animated_props->size() > 0) {
        LOG("?? Unsupported animated text properties.\n");
    }

    // TODO: add some lookup(rpath) method to skjson to simplify this.
    const skjson::ObjectValue* jd  = (*jt)["d"];
    const skjson::ArrayValue*  jk  = jd
            ? (*jd)["k"].operator const skjson::ArrayValue*() : nullptr;
    const skjson::ObjectValue* jv0 = jk && jk->size() == 1
            ? (*jk)[0].operator const skjson::ObjectValue*() : nullptr;
    const skjson::ObjectValue* jprops = jv0
            ? (*jv0)["s"].operator const skjson::ObjectValue*() : nullptr;

    if (!jprops) {
        LogJSON(*jt, "!! Unexpected text property");
        return nullptr;
    }

    const skjson::StringValue* font_name    = (*jprops)["f"];
    const skjson::StringValue* text         = (*jprops)["t"];
    const skjson::NumberValue* text_size    = (*jprops)["s"];
    const skjson::ArrayValue*  fill_color   = (*jprops)["fc"];
    const skjson::ArrayValue*  stroke_color = (*jprops)["sc"];
    const skjson::ArrayValue*  stroke_width = (*jprops)["sw"];

    if (!font_name || !text || !text_size || !(fill_color || stroke_color)) {
        LogJSON(*jprops, "!! Invalid text properties");
        return nullptr;
    }

    const auto* font = ctx->fFonts.find(SkString(font_name->begin(), font_name->size()));
    if (!font) {
        LOG("!! Unknown font: \"%s\"\n", font_name->begin());
        return nullptr;
    }

    if (font->fGlyphs) {
        LOG("!! Unsupported inline/glyph font: %s\n", font_name->begin());
        return nullptr;
    }

    static constexpr SkPaint::Align gAlignMap[] = {
        SkPaint::kLeft_Align,  // 'j': 0
        SkPaint::kRight_Align, // 'j': 1
        SkPaint::kCenter_Align // 'j': 2
    };
    const auto align = gAlignMap[SkTMin<size_t>(ParseDefault<size_t>((*jprops)["j"], 0),
                                                SK_ARRAY_COUNT(gAlignMap))];

    auto text_node = sksg::Text::Make(font->fTypeface, SkString(text->begin(), text->size()));
    text_node->setSize(**text_size);
    text_node->setAlign(align);

    sk_sp<sksg::Draw> fill_node, stroke_node;
    VectorValue color_vec;

    if (fill_color && Parse(*fill_color, &color_vec)) {
        auto fill = sksg::Color::Make(ValueTraits<VectorValue>::As<SkColor>(color_vec));
        fill->setAntiAlias(true);
        fill_node = sksg::Draw::Make(text_node, std::move(fill));
    }

    if (stroke_color && Parse(*stroke_color, &color_vec)) {
        auto stroke = sksg::Color::Make(ValueTraits<VectorValue>::As<SkColor>(color_vec));
        stroke->setAntiAlias(true);
        stroke->setStyle(SkPaint::kStroke_Style);
        if (stroke_width) {
            stroke->setStrokeWidth(ParseDefault<float>(*stroke_width, 1));
        }
        stroke_node = sksg::Draw::Make(text_node, std::move(stroke));
    }

    if (!stroke_node) {
        return fill_node;
    }
    if (!fill_node) {
        return stroke_node;
    }

    auto group_node = sksg::Group::Make();
    group_node->addChild(std::move(fill_node));
    group_node->addChild(std::move(stroke_node));

    return group_node;
}

} // namespace internal
} // namespace skottie
