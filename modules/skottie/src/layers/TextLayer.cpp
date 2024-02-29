/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkString.h"
#include "include/core/SkTypeface.h"
#include "include/core/SkTypes.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/text/Font.h"
#include "modules/skottie/src/text/TextAdapter.h"
#include "modules/skresources/include/SkResources.h"
#include "modules/sksg/include/SkSGGroup.h"  // IWYU pragma: keep
#include "modules/sksg/include/SkSGRenderNode.h"
#include "src/base/SkTSearch.h"
#include "src/core/SkTHash.h"
#include "src/utils/SkJSON.h"

#include <string.h>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>

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
                      CustomFont::Builder()
                  });
    }

    const auto has_comp_glyphs = [](const skjson::ArrayValue* jchars) {
        if (!jchars) {
            return false;
        }

        for (const skjson::ObjectValue* jchar : *jchars) {
            if (!jchar) {
                continue;
            }
            if (ParseDefault<int>((*jchar)["t"], 0) == 1) {
                return true;
            }
        }

        return false;
    };

    // Historically, Skottie has been loading native fonts before embedded glyphs, unless
    // the opposite is explicitly requested via kPreferEmbeddedFonts.  That's mostly because
    // embedded glyphs used to be just a path representation of system fonts at export time,
    // (and thus lower quality).
    //
    // OTOH embedded glyph *compositions* must be prioritized, as they are presumably more
    // expressive than the system font equivalent.
    const auto prioritize_embedded_fonts =
            (fFlags & Animation::Builder::kPreferEmbeddedFonts) || has_comp_glyphs(jchars);

    // Optional pass.
    if (jchars && prioritize_embedded_fonts && this->resolveEmbeddedTypefaces(*jchars)) {
        return;
    }

    // Native typeface resolution.
    if (this->resolveNativeTypefaces()) {
        return;
    }

    // Embedded typeface fallback.
    if (jchars && !prioritize_embedded_fonts) {
        this->resolveEmbeddedTypefaces(*jchars);
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

        // Typeface fallback order:
        //   1) externally-loaded font (provided by the embedder)
        //   2) system font (family/style)
        //   3) system default

        finfo->fTypeface = fResourceProvider->loadTypeface(name.c_str(), finfo->fPath.c_str());

        // legacy API fallback
        // TODO: remove after client migration
        if (!finfo->fTypeface && fFontMgr) {
            finfo->fTypeface = fFontMgr->makeFromData(
                    fResourceProvider->loadFont(name.c_str(), finfo->fPath.c_str()));
        }

        if (!finfo->fTypeface && fFontMgr) {
            finfo->fTypeface = fFontMgr->matchFamilyStyle(finfo->fFamily.c_str(),
                                                      FontStyle(this, finfo->fStyle.c_str()));

            if (!finfo->fTypeface) {
                this->log(Logger::Level::kError, nullptr, "Could not create typeface for %s|%s.",
                          finfo->fFamily.c_str(), finfo->fStyle.c_str());
                // Last resort.
                finfo->fTypeface = fFontMgr->legacyMakeTypeface(nullptr,
                                                            FontStyle(this, finfo->fStyle.c_str()));

                has_unresolved |= !finfo->fTypeface;
            }
        }
        if (!finfo->fTypeface && !fFontMgr) {
            this->log(Logger::Level::kError, nullptr,
                      "Could not load typeface for %s|%s because no SkFontMgr provided.",
                      finfo->fFamily.c_str(), finfo->fStyle.c_str());
        }
    });

    return !has_unresolved;
}

bool AnimationBuilder::resolveEmbeddedTypefaces(const skjson::ArrayValue& jchars) {
    // Optional array of glyphs, to be associated with one of the declared fonts. E.g.
    // "chars": [
    //     {
    //         "fFamily": "Roboto",       // part of the font key
    //         "style": "Regular",        // part of the font key
    //         ...                        // glyph data
    //    }
    // ]
    FontInfo* current_font = nullptr;

    for (const skjson::ObjectValue* jchar : jchars) {
        if (!jchar) {
            continue;
        }

        const skjson::StringValue* jfamily = (*jchar)["fFamily"];
        const skjson::StringValue* jstyle  = (*jchar)["style"]; // "style", not "fStyle"...

        if (!jfamily || !jstyle) {
            this->log(Logger::Level::kError, jchar, "Invalid glyph.");
            continue;
        }
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
                          "Font not found (%s, %s).", family, style);
                continue;
            }
        }

        if (!current_font->fCustomFontBuilder.parseGlyph(this, *jchar)) {
            this->log(Logger::Level::kError, jchar, "Invalid glyph.");
        }
    }

    // Final pass to commit custom typefaces.
    bool has_unresolved = false;
    std::vector<std::unique_ptr<CustomFont>> custom_fonts;
    fFonts.foreach([&has_unresolved, &custom_fonts](const SkString&, FontInfo* finfo) {
        if (finfo->fTypeface) {
            return; // already resolved
        }

        auto font = finfo->fCustomFontBuilder.detach();

        finfo->fTypeface = font->typeface();

        if (font->glyphCompCount() > 0) {
            custom_fonts.push_back(std::move(font));
        }

        has_unresolved |= !finfo->fTypeface;
    });

    // Stash custom font data for later use.
    if (!custom_fonts.empty()) {
        custom_fonts.shrink_to_fit();
        fCustomGlyphMapper = sk_make_sp<CustomFont::GlyphCompMapper>(std::move(custom_fonts));
    }

    return !has_unresolved;
}

sk_sp<sksg::RenderNode> AnimationBuilder::attachTextLayer(const skjson::ObjectValue& jlayer,
                                                          LayerInfo*) const {
    return this->attachDiscardableAdapter<TextAdapter>(jlayer,
                                                       this,
                                                       fFontMgr,
                                                       fCustomGlyphMapper,
                                                       fLogger);
}

const AnimationBuilder::FontInfo* AnimationBuilder::findFont(const SkString& font_name) const {
    return fFonts.find(font_name);
}

} // namespace internal
} // namespace skottie
