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

#include <string.h>

namespace skottie {
namespace internal {

namespace {

SkFontStyle FontStyle(const char* style) {
    static constexpr struct {
        const char*               fName;
        const SkFontStyle::Weight fWeight;
    } gWeightMap[] = {
        { "ExtraLight", SkFontStyle::kExtraLight_Weight },
        { "Light"     , SkFontStyle::kLight_Weight      },
        { "Regular"   , SkFontStyle::kNormal_Weight     },
        { "Medium"    , SkFontStyle::kMedium_Weight     },
        { "SemiBold"  , SkFontStyle::kSemiBold_Weight   },
        { "Bold"      , SkFontStyle::kBold_Weight       },
        { "ExtraBold" , SkFontStyle::kExtraBold_Weight  },
    };

    SkFontStyle::Weight weight = SkFontStyle::kNormal_Weight;
    for (const auto& w : gWeightMap) {
        const auto name_len = strlen(w.fName);
        if (!strncmp(style, w.fName, name_len)) {
            weight = w.fWeight;
            style += name_len;
            break;
        }
    }

    static constexpr struct {
        const char*              fName;
        const SkFontStyle::Slant fSlant;
    } gSlantMap[] = {
        { "Italic" , SkFontStyle::kItalic_Slant  },
        { "Oblique", SkFontStyle::kOblique_Slant },
    };

    SkFontStyle::Slant slant = SkFontStyle::kUpright_Slant;
    if (*style != '\0') {
        for (const auto& s : gSlantMap) {
            if (!strcmp(style, s.fName)) {
                slant = s.fSlant;
                style += strlen(s.fName);
                break;
            }
        }
    }

    if (*style != '\0') {
        LOG("?? Unknown font style: %s\n", style);
    }

    return SkFontStyle(weight, SkFontStyle::kNormal_Width, slant);
}

} // namespace

bool AnimationBuilder::FontInfo::matches(const char family[], const char style[]) const {
    return 0 == strcmp(fFamily.c_str(), family)
        && 0 == strcmp(fStyle.c_str(), style);
}

void AnimationBuilder::parseFonts(const skjson::ObjectValue* jfonts,
                                  const skjson::ArrayValue* jchars) {
    // Optional array of font entries, referenced (by name) from text layer document nodes. E.g.
    // "fonts": {
    //        "list": [
    //            {
    //                "ascent": 75,
    //                "fClass": "",
    //                "fFamily": "Roboto",
    //                "fName": "Roboto-Regular",
    //                "fPath": "",
    //                "fStyle": "Regular",
    //                "fWeight": "",
    //                "origin": 1
    //            }
    //        ]
    //    },
    if (jfonts) {
        if (const skjson::ArrayValue* jlist = (*jfonts)["list"]) {
            for (const skjson::ObjectValue* jfont : *jlist) {
                if (!jfont) {
                    continue;
                }

                const skjson::StringValue* jname   = (*jfont)["fName"];
                const skjson::StringValue* jfamily = (*jfont)["fFamily"];
                const skjson::StringValue* jstyle  = (*jfont)["fStyle"];

                if (!jname   || !jname->size() ||
                    !jfamily || !jfamily->size() ||
                    !jstyle  || !jstyle->size()) {
                    LogJSON(*jfont, "!! Ignoring invalid font");
                    continue;
                }

                sk_sp<SkTypeface> tf(fFontMgr->matchFamilyStyle(jfamily->begin(),
                                                                FontStyle(jstyle->begin())));
                if (!tf) {
                    LOG("!! Could not create typeface for %s|%s\n",
                        jfamily->begin(), jstyle->begin());
                    // Last resort.
                    tf.reset(fFontMgr->matchFamilyStyle("Arial", SkFontStyle::Normal()));
                    if (!tf) {
                        continue;
                    }
                }

                fFonts.set(SkString(jname->begin(), jname->size()),
                          {
                              SkString(jfamily->begin(), jfamily->size()),
                              SkString(jstyle->begin(), jstyle->size()),
                              ParseDefault((*jfont)["ascent"] , 0.0f),
                              std::move(tf)
                          });
            }
        }
    }

    // Optional array of glyphs, to be associated with one of the declared fonts. E.g.
    // "chars": [
    //     {
    //         "ch": "t",
    //         "data": {
    //             "shapes": [...]
    //         },
    //         "fFamily": "Roboto",
    //         "size": 50,
    //         "style": "Regular",
    //         "w": 32.67
    //    }
    // ]
    if (jchars) {
        FontInfo* current_font = nullptr;

        for (const skjson::ObjectValue* jchar : *jchars) {
            if (!jchar) {
                continue;
            }

            const skjson::StringValue* jch = (*jchar)["ch"];
            if (!jch) {
                continue;
            }

            const skjson::StringValue* jfamily = (*jchar)["fFamily"];
            const skjson::StringValue* jstyle  = (*jchar)["style"]; // "style", not "fStyle"...

            const auto* ch_ptr = jch->begin();
            const auto  ch_len = jch->size();

            if (!jfamily || !jstyle || (SkUTF::CountUTF8(ch_ptr, ch_len) != 1)) {
                LogJSON(*jchar, "!! Invalid glyph");
                continue;
            }

            const auto uni = SkUTF::NextUTF8(&ch_ptr, ch_ptr + ch_len);
            SkASSERT(uni != -1);

            const auto* family = jfamily->begin();
            const auto* style  = jstyle->begin();

            // Locate (and cache) the font info. Unlike text nodes, glyphs reference the font by
            // (family, style) -- not by name :(  For now this performs a linear search over *all*
            // fonts: generally there are few of them, and glyph definitions are font-clustered.
            // If problematic, we can refactor as a two-level hashmap.
            if (!current_font || !current_font->matches(family, style)) {
                current_font = nullptr;
                fFonts.foreach([&](const SkString& name, FontInfo* finfo) {
                    if (finfo->matches(family, style)) {
                        current_font = finfo;
                        // TODO: would be nice to break early here...
                    }
                });
                if (!current_font) {
                    LOG("!! Font not found for codepoint (%d, %s, %s)\n", uni, family, style);
                    continue;
                }
            }

            // TODO: parse glyphs
        }
    }
}

sk_sp<SkTypeface> AnimationBuilder::findFont(const SkString& font_name) const {
    if (const auto* font = fFonts.find(font_name)) {
        return font->fTypeface;
    }

    LOG("!! Unknown font: \"%s\"\n", font_name.c_str());
    return nullptr;
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachTextLayer(const skjson::ObjectValue& layer,
                                                          AnimatorScope* ascope) const {
    // General text node format:
    // "t": {
    //    "a": [], // animators (TODO)
    //    "d": {
    //        "k": [
    //            {
    //                "s": {
    //                    "f": "Roboto-Regular",
    //                    "fc": [
    //                        0.42,
    //                        0.15,
    //                        0.15
    //                    ],
    //                    "j": 1,
    //                    "lh": 60,
    //                    "ls": 0,
    //                    "s": 50,
    //                    "t": "text align right",
    //                    "tr": 0
    //                },
    //                "t": 0
    //            }
    //        ]
    //    },
    //    "m": {}, // "more options" (TODO)
    //    "p": {}  // "path options" (TODO)
    // },
    const skjson::ObjectValue* jt = layer["t"];
    if (!jt) {
        LogJSON(layer, "!! Missing text layer \"t\" property");
        return nullptr;
    }

    const skjson::ArrayValue* animated_props = (*jt)["a"];
    if (animated_props && animated_props->size() > 0) {
        LOG("?? Unsupported animated text properties.\n");
    }

    const skjson::ObjectValue* jd  = (*jt)["d"];
    if (!jd) {
        return nullptr;
    }

    // Build a SG fragment with the following general format:
    //
    // [Group]
    //   [Draw]
    //     [FillPaint]
    //     [Text]*
    //   [Draw]
    //     [StrokePaint]
    //     [Text]*
    //
    // * where the text node is shared

    auto root_node = sksg::Group::Make();
    auto text_node = sksg::Text::Make(nullptr, SkString());
    auto fill_paint = sksg::Color::Make(SK_ColorTRANSPARENT),
       stroke_paint = sksg::Color::Make(SK_ColorTRANSPARENT);
    auto fill_node  = sksg::Draw::Make(text_node, fill_paint),
       stroke_node  = sksg::Draw::Make(text_node, stroke_paint);

    text_node->setFlags(text_node->getFlags() |
                        SkPaint::kAntiAlias_Flag |
                        SkPaint::kSubpixelText_Flag);
    text_node->setHinting(SkPaint::kNo_Hinting);

    stroke_paint->setStyle(SkPaint::kStroke_Style);

    this->bindProperty<TextValue>(*jd, ascope,
        [root_node,
         text_node,
         fill_paint,
         stroke_paint,
         fill_node,
         stroke_node] (const TextValue& txt) {
            text_node->setTypeface(txt.fTypeface);
            text_node->setText(txt.fText);
            text_node->setSize(txt.fTextSize);
            text_node->setAlign(txt.fAlign);

            fill_paint->setColor(txt.fFillColor);
            stroke_paint->setColor(txt.fStrokeColor);
            stroke_paint->setStrokeWidth(txt.fStrokeWidth);

            const auto fill_attached = !root_node->empty()
                                     && root_node->children().front() == fill_node,
                     stroke_attached = !root_node->empty()
                                     && root_node->children().back() == stroke_node;

            // Reattach nodes in the right order, if needed.
            if (fill_attached != txt.fHasFill || stroke_attached != txt.fHasStroke) {
                while (!root_node->empty()) {
                    root_node->removeChild(root_node->children().back());
                }

                if (txt.fHasFill) {
                    root_node->addChild(fill_node);
                }

                if (txt.fHasStroke) {
                    root_node->addChild(stroke_node);
                }
            }
        });

    return std::move(root_node);
}

} // namespace internal
} // namespace skottie
