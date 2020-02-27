/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/SkottiePriv.h"

#include "include/core/SkData.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkTypes.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/text/TextAdapter.h"
#include "modules/skottie/src/text/TextAnimator.h"
#include "modules/skottie/src/text/TextValue.h"
#include "modules/sksg/include/SkSGDraw.h"
#include "modules/sksg/include/SkSGGroup.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "modules/sksg/include/SkSGText.h"

#include <string.h>

namespace skottie {
namespace internal {

namespace {

SkFontStyle FontStyle(const AnimationBuilder* abuilder, const char* style) {
    static constexpr struct {
        const char*               fName;
        const SkFontStyle::Weight fWeight;
    } gWeightMap[] = {
        { "Regular"   , SkFontStyle::kNormal_Weight     },
        { "Medium"    , SkFontStyle::kMedium_Weight     },
        { "Bold"      , SkFontStyle::kBold_Weight       },
        { "Light"     , SkFontStyle::kLight_Weight      },
        { "Black"     , SkFontStyle::kBlack_Weight      },
        { "Thin"      , SkFontStyle::kThin_Weight       },
        { "Extra"     , SkFontStyle::kExtraBold_Weight  },
        { "ExtraBold" , SkFontStyle::kExtraBold_Weight  },
        { "ExtraLight", SkFontStyle::kExtraLight_Weight },
        { "ExtraBlack", SkFontStyle::kExtraBlack_Weight },
        { "SemiBold"  , SkFontStyle::kSemiBold_Weight   },
        { "Hairline"  , SkFontStyle::kThin_Weight       },
        { "Normal"    , SkFontStyle::kNormal_Weight     },
        { "Plain"     , SkFontStyle::kNormal_Weight     },
        { "Standard"  , SkFontStyle::kNormal_Weight     },
        { "Roman"     , SkFontStyle::kNormal_Weight     },
        { "Heavy"     , SkFontStyle::kBlack_Weight      },
        { "Demi"      , SkFontStyle::kSemiBold_Weight   },
        { "DemiBold"  , SkFontStyle::kSemiBold_Weight   },
        { "Ultra"     , SkFontStyle::kExtraBold_Weight  },
        { "UltraBold" , SkFontStyle::kExtraBold_Weight  },
        { "UltraBlack", SkFontStyle::kExtraBlack_Weight },
        { "UltraHeavy", SkFontStyle::kExtraBlack_Weight },
        { "UltraLight", SkFontStyle::kExtraLight_Weight },
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
        abuilder->log(Logger::Level::kWarning, nullptr, "Unknown font style: %s.", style);
    }

    return SkFontStyle(weight, SkFontStyle::kNormal_Width, slant);
}

} // namespace

bool AnimationBuilder::FontInfo::matches(const char family[], const char style[]) const {
    return 0 == strcmp(fFamily.c_str(), family)
        && 0 == strcmp(fStyle.c_str(), style);
}

#ifdef SK_NO_FONTS
void AnimationBuilder::parseFonts(const skjson::ObjectValue* jfonts,
                                  const skjson::ArrayValue* jchars) {}

sk_sp<sksg::RenderNode> AnimationBuilder::attachTextLayer(const skjson::ObjectValue& jlayer,
                                                          LayerInfo*) const {
    return nullptr;
}
#else
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
    //                "fPath": "https://fonts.googleapis.com/css?family=Roboto",
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
                const skjson::StringValue* jpath   = (*jfont)["fPath"];

                if (!jname   || !jname->size() ||
                    !jfamily || !jfamily->size() ||
                    !jstyle  || !jstyle->size()) {
                    this->log(Logger::Level::kError, jfont, "Invalid font.");
                    continue;
                }

                const auto& fmgr = fLazyFontMgr.get();

                // Typeface fallback order:
                //   1) externally-loaded font (provided by the embedder)
                //   2) system font (family/style)
                //   3) system default

                sk_sp<SkTypeface> tf =
                    fmgr->makeFromData(fResourceProvider->loadFont(jname->begin(),
                                                                   jpath ? jpath->begin()
                                                                         : nullptr));

                if (!tf) {
                    tf.reset(fmgr->matchFamilyStyle(jfamily->begin(),
                                                    FontStyle(this, jstyle->begin())));
                }

                if (!tf) {
                    this->log(Logger::Level::kError, nullptr,
                              "Could not create typeface for %s|%s.",
                              jfamily->begin(), jstyle->begin());
                    // Last resort.
                    tf = fmgr->legacyMakeTypeface(nullptr, FontStyle(this, jstyle->begin()));
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
                this->log(Logger::Level::kError, jchar, "Invalid glyph.");
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
                    this->log(Logger::Level::kError, nullptr,
                              "Font not found for codepoint (%d, %s, %s).", uni, family, style);
                    continue;
                }
            }

            // TODO: parse glyphs
        }
    }
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachTextLayer(const skjson::ObjectValue& jlayer,
                                                          LayerInfo*) const {
    return this->attachDiscardableAdapter<TextAdapter>(jlayer,
                                                       this,
                                                       fLazyFontMgr.getMaybeNull(),
                                                       fLogger);
}
#endif

const AnimationBuilder::FontInfo* AnimationBuilder::findFont(const SkString& font_name) const {
    return fFonts.find(font_name);
}


} // namespace internal
} // namespace skottie
