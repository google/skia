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
#include "modules/sksg/include/SkSGPath.h"
#include "modules/sksg/include/SkSGText.h"
#include "src/core/SkTSearch.h"

#include <string.h>

namespace skottie {
namespace internal {

namespace {

template <typename T, typename TMap>
const char* parse_map(const TMap& map, const char* str, T* result) {
    // ignore leading whitespace
    while (*str == ' ') ++str;

    const char* next_tok = strchr(str, ' ');

    if (const auto len = next_tok ? (next_tok - str) : strlen(str)) {
        for (const auto& e : map) {
            const char* key = std::get<0>(e);
            if (!strncmp(str, key, len) && key[len] == '\0') {
                *result = std::get<1>(e);
                return str + len;
            }
        }
    }

    return str;
}

SkFontStyle FontStyle(const AnimationBuilder* abuilder, const char* style) {
    static constexpr std::tuple<const char*, SkFontStyle::Weight> gWeightMap[] = {
        { "regular"   , SkFontStyle::kNormal_Weight     },
        { "medium"    , SkFontStyle::kMedium_Weight     },
        { "bold"      , SkFontStyle::kBold_Weight       },
        { "light"     , SkFontStyle::kLight_Weight      },
        { "black"     , SkFontStyle::kBlack_Weight      },
        { "thin"      , SkFontStyle::kThin_Weight       },
        { "extra"     , SkFontStyle::kExtraBold_Weight  },
        { "extrabold" , SkFontStyle::kExtraBold_Weight  },
        { "extralight", SkFontStyle::kExtraLight_Weight },
        { "extrablack", SkFontStyle::kExtraBlack_Weight },
        { "semibold"  , SkFontStyle::kSemiBold_Weight   },
        { "hairline"  , SkFontStyle::kThin_Weight       },
        { "normal"    , SkFontStyle::kNormal_Weight     },
        { "plain"     , SkFontStyle::kNormal_Weight     },
        { "standard"  , SkFontStyle::kNormal_Weight     },
        { "roman"     , SkFontStyle::kNormal_Weight     },
        { "heavy"     , SkFontStyle::kBlack_Weight      },
        { "demi"      , SkFontStyle::kSemiBold_Weight   },
        { "demibold"  , SkFontStyle::kSemiBold_Weight   },
        { "ultra"     , SkFontStyle::kExtraBold_Weight  },
        { "ultrabold" , SkFontStyle::kExtraBold_Weight  },
        { "ultrablack", SkFontStyle::kExtraBlack_Weight },
        { "ultraheavy", SkFontStyle::kExtraBlack_Weight },
        { "ultralight", SkFontStyle::kExtraLight_Weight },
    };
    static constexpr std::tuple<const char*, SkFontStyle::Slant> gSlantMap[] = {
        { "italic" , SkFontStyle::kItalic_Slant  },
        { "oblique", SkFontStyle::kOblique_Slant },
    };

    auto weight = SkFontStyle::kNormal_Weight;
    auto slant  = SkFontStyle::kUpright_Slant;

    // style is case insensitive.
    SkAutoAsciiToLC lc_style(style);
    style = lc_style.lc();
    style = parse_map(gWeightMap, style, &weight);
    style = parse_map(gSlantMap , style, &slant );

    // ignore trailing whitespace
    while (*style == ' ') ++style;

    if (*style) {
        abuilder->log(Logger::Level::kWarning, nullptr, "Unknown font style: %s.", style);
    }

    return SkFontStyle(weight, SkFontStyle::kNormal_Width, slant);
}

bool parse_glyph_path(const skjson::ObjectValue* jdata,
                      const AnimationBuilder* abuilder,
                      SkPath* path) {
    // Glyph path encoding:
    //
    //   "data": {
    //       "shapes": [                         // follows the shape layer format
    //           {
    //               "ty": "gr",                 // group shape type
    //               "it": [                     // group items
    //                   {
    //                       "ty": "sh",         // actual shape
    //                       "ks": <path data>   // animatable path format, but always static
    //                   },
    //                   ...
    //               ]
    //           },
    //           ...
    //       ]
    //   }

    if (!jdata) {
        return false;
    }

    const skjson::ArrayValue* jshapes = (*jdata)["shapes"];
    if (!jshapes) {
        // Space/empty glyph.
        return true;
    }

    for (const skjson::ObjectValue* jgrp : *jshapes) {
        if (!jgrp) {
            return false;
        }

        const skjson::ArrayValue* jit = (*jgrp)["it"];
        if (!jit) {
            return false;
        }

        for (const skjson::ObjectValue* jshape : *jit) {
            if (!jshape) {
                return false;
            }

            // Glyph paths should never be animated.  But they are encoded as
            // animatable properties, so we use the appropriate helpers.
            AnimationBuilder::AutoScope ascope(abuilder);
            auto path_node = abuilder->attachPath((*jshape)["ks"]);
            auto animators = ascope.release();

            if (!path_node || !animators.empty()) {
                return false;
            }

            // Successfully parsed a static path.  Whew.
            path->addPath(path_node->getPath());
        }
    }

    return true;
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
    const skjson::ArrayValue* jlist = jfonts
            ? static_cast<const skjson::ArrayValue*>((*jfonts)["list"])
            : nullptr;
    if (!jlist) {
        return;
    }

    // First pass: collect font info.
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
            !jstyle) {
            this->log(Logger::Level::kError, jfont, "Invalid font.");
            continue;
        }

        fFonts.set(SkString(jname->begin(), jname->size()),
                  {
                      SkString(jfamily->begin(), jfamily->size()),
                      SkString( jstyle->begin(),  jstyle->size()),
                      jpath ? SkString(  jpath->begin(),   jpath->size()) : SkString(),
                      ParseDefault((*jfont)["ascent"] , 0.0f),
                      nullptr, // placeholder
                      SkCustomTypefaceBuilder()
                  });
    }

    // Optional pass.
    if (jchars && (fFlags & Animation::Builder::kPreferEmbeddedFonts) &&
        this->resolveEmbeddedTypefaces(*jchars)) {
        return;
    }

    // Native typeface resolution.
    if (this->resolveNativeTypefaces()) {
        return;
    }

    // Embedded typeface fallback.
    if (jchars && !(fFlags & Animation::Builder::kPreferEmbeddedFonts) &&
        this->resolveEmbeddedTypefaces(*jchars)) {
    }
}

bool AnimationBuilder::resolveNativeTypefaces() {
    bool has_unresolved = false;

    fFonts.foreach([&](const SkString& name, FontInfo* finfo) {
        SkASSERT(finfo);

        if (finfo->fTypeface) {
            // Already resolved from glyph paths.
            return;
        }

        const auto& fmgr = fLazyFontMgr.get();

        // Typeface fallback order:
        //   1) externally-loaded font (provided by the embedder)
        //   2) system font (family/style)
        //   3) system default

        finfo->fTypeface = fResourceProvider->loadTypeface(name.c_str(), finfo->fPath.c_str());

        // legacy API fallback
        // TODO: remove after client migration
        if (!finfo->fTypeface) {
            finfo->fTypeface = fmgr->makeFromData(
                    fResourceProvider->loadFont(name.c_str(), finfo->fPath.c_str()));
        }

        if (!finfo->fTypeface) {
            finfo->fTypeface.reset(fmgr->matchFamilyStyle(finfo->fFamily.c_str(),
                                                          FontStyle(this, finfo->fStyle.c_str())));

            if (!finfo->fTypeface) {
                this->log(Logger::Level::kError, nullptr, "Could not create typeface for %s|%s.",
                          finfo->fFamily.c_str(), finfo->fStyle.c_str());
                // Last resort.
                finfo->fTypeface = fmgr->legacyMakeTypeface(nullptr,
                                                            FontStyle(this, finfo->fStyle.c_str()));

                has_unresolved |= !finfo->fTypeface;
            }
        }
    });

    return !has_unresolved;
}

bool AnimationBuilder::resolveEmbeddedTypefaces(const skjson::ArrayValue& jchars) {
    // Optional array of glyphs, to be associated with one of the declared fonts. E.g.
    // "chars": [
    //     {
    //         "ch": "t",
    //         "data": {
    //             "shapes": [...]        // shape-layer-like geometry
    //         },
    //         "fFamily": "Roboto",       // part of the font key
    //         "size": 50,                // apparently ignored
    //         "style": "Regular",        // part of the font key
    //         "w": 32.67                 // width/advance (1/100 units)
    //    }
    // ]
    FontInfo* current_font = nullptr;

    for (const skjson::ObjectValue* jchar : jchars) {
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
        if (!SkTFitsIn<SkGlyphID>(uni)) {
            // Custom font keys are SkGlyphIDs.  We could implement a remapping scheme if needed,
            // but for now direct mapping seems to work well enough.
            this->log(Logger::Level::kError, jchar, "Unsupported glyph ID.");
            continue;
        }
        const auto glyph_id = SkTo<SkGlyphID>(uni);

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

        SkPath path;
        if (!parse_glyph_path((*jchar)["data"], this, &path)) {
            continue;
        }

        const auto advance = ParseDefault((*jchar)["w"], 0.0f);

        // Interestingly, glyph paths are defined in a percentage-based space,
        // regardless of declared glyph size...
        static constexpr float kPtScale = 0.01f;

        // Normalize the path and advance for 1pt.
        path.transform(SkMatrix::Scale(kPtScale, kPtScale));

        current_font->fCustomBuilder.setGlyph(glyph_id, advance * kPtScale, path);
    }

    // Final pass to commit custom typefaces.
    auto has_unresolved = false;
    fFonts.foreach([&has_unresolved](const SkString&, FontInfo* finfo) {
        if (finfo->fTypeface) {
            return; // already resolved
        }

        finfo->fTypeface = finfo->fCustomBuilder.detach();

        has_unresolved |= !finfo->fTypeface;
    });

    return !has_unresolved;
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
