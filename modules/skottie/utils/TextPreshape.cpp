/*
 * Copyright 2024 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/utils/TextPreshape.h"

#include "include/core/SkData.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPathTypes.h"
#include "include/core/SkPoint.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkDebug.h"
#include "include/private/base/SkTPin.h"
#include "include/private/base/SkTo.h"
#include "modules/skottie/include/ExternalLayer.h"
#include "modules/skottie/include/Skottie.h"
#include "modules/skottie/include/SkottieProperty.h"
#include "modules/skottie/include/TextShaper.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/skottie/src/text/TextValue.h"
#include "modules/skresources/include/SkResources.h"
#include "modules/skshaper/include/SkShaper_factory.h"
#include "src/base/SkArenaAlloc.h"
#include "src/base/SkUTF.h"
#include "src/core/SkGeometry.h"
#include "src/core/SkPathPriv.h"
#include "src/utils/SkJSON.h"

#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

using ResourceProvider = skresources::ResourceProvider;

using skjson::Value;
using skjson::NullValue;
using skjson::BoolValue;
using skjson::NumberValue;
using skjson::ArrayValue;
using skjson::StringValue;
using skjson::ObjectValue;

namespace {

SkString preshapedFontName(const std::string_view& fontName) {
    return SkStringPrintf("%s_preshaped", fontName.data());
}

Value pathToLottie(const SkPath& path, SkArenaAlloc& alloc) {
    // Lottie paths are single-contour vectors of cubic segments, stored as
    // (vertex, in_tangent, out_tangent) tuples.
    // A usual Skia cubic segment (p0, c0, c1, p1) corresponds to Lottie's
    // (vertex[0], out_tan[0], in_tan[1], vertex[1]).
    // Tangent control points are stored in separate arrays, using relative coordinates.
    struct Contour {
        std::vector<SkPoint> verts, in_tan, out_tan;
        bool closed = false;

        void add(const SkPoint& v, const SkPoint& i, const SkPoint& o) {
            verts.push_back(v);
            in_tan.push_back(i);
            out_tan.push_back(o);
        }

        size_t size() const {
            SkASSERT(verts.size() == in_tan.size());
            SkASSERT(verts.size() == out_tan.size());
            return verts.size();
        }
    };

    std::vector<Contour> contours(1);

    for (const auto [verb, pts, weights] : SkPathPriv::Iterate(path)) {
        switch (verb) {
            case SkPathVerb::kMove:
                if (!contours.back().verts.empty()) {
                    contours.emplace_back();
                }
                contours.back().add(pts[0], {0, 0}, {0, 0});
                break;
            case SkPathVerb::kClose:
                SkASSERT(contours.back().size() > 0);
                contours.back().closed = true;
                break;
            case SkPathVerb::kLine:
                SkASSERT(contours.back().size() > 0);
                SkASSERT(pts[0] == contours.back().verts.back());
                contours.back().add(pts[1], {0, 0}, {0, 0});
                break;
            case SkPathVerb::kQuad:
                SkASSERT(contours.back().size() > 0);
                SkASSERT(pts[0] == contours.back().verts.back());
                SkPoint cubic[4];
                SkConvertQuadToCubic(pts, cubic);
                contours.back().out_tan.back() = cubic[1] - cubic[0];
                contours.back().add(cubic[3], cubic[2] - cubic[3], {0, 0});
                break;
            case SkPathVerb::kCubic:
                SkASSERT(contours.back().size() > 0);
                SkASSERT(pts[0] == contours.back().verts.back());
                contours.back().out_tan.back() = pts[1] - pts[0];
                contours.back().add(pts[3], pts[2] - pts[3], {0, 0});
                break;
            case SkPathVerb::kConic:
                SkDebugf("Unexpected conic verb!\n");
                break;
        }
    }

    auto ptsToLottie = [](const std::vector<SkPoint> v, SkArenaAlloc& alloc) {
        std::vector<Value> vec(v.size());
        for (size_t i = 0; i < v.size(); ++i) {
            Value fields[] = { NumberValue(v[i].fX), NumberValue(v[i].fY) };
            vec[i] = ArrayValue(fields, std::size(fields), alloc);
        }

        return ArrayValue(vec.data(), vec.size(), alloc);
    };

    std::vector<Value> jcontours(contours.size());
    for (size_t i = 0; i < contours.size(); ++i) {
        const skjson::Member fields_k[] = {
            { StringValue("v", alloc), ptsToLottie(contours[i].verts,   alloc) },
            { StringValue("i", alloc), ptsToLottie(contours[i].in_tan,  alloc) },
            { StringValue("o", alloc), ptsToLottie(contours[i].out_tan, alloc) },
            { StringValue("c", alloc), BoolValue (contours[i].closed)          },
        };

        const skjson::Member fields_ks[] = {
            { StringValue("a", alloc), NumberValue(0)                                    },
            { StringValue("k", alloc), ObjectValue(fields_k, std::size(fields_k), alloc) },
        };

        const skjson::Member fields[] = {
            { StringValue("ty" , alloc), StringValue("sh", alloc)                            },
            { StringValue("hd" , alloc), BoolValue(false)                                    },
            { StringValue("ind", alloc), NumberValue(SkToInt(i))                             },
            { StringValue("ks" , alloc), ObjectValue(fields_ks, std::size(fields_ks), alloc) },
            { StringValue("mn" , alloc), StringValue("ADBE Vector Shape - Group" , alloc)    },
            { StringValue("nm" , alloc), StringValue("_" , alloc)                            },
        };

        jcontours[i] = ObjectValue(fields, std::size(fields), alloc);
    }

    const skjson::Member fields_sh[] = {
        { StringValue("ty" , alloc), StringValue("gr", alloc)                              },
        { StringValue("hd" , alloc), BoolValue(false)                                      },
        { StringValue("bm" , alloc), NumberValue(0)                                        },
        { StringValue("it" , alloc), ArrayValue(jcontours.data(), jcontours.size(), alloc) },
        { StringValue("mn" , alloc), StringValue("ADBE Vector Group" , alloc)              },
        { StringValue("nm" , alloc), StringValue("_" , alloc)                              },
    };

    const Value shape = ObjectValue(fields_sh, std::size(fields_sh), alloc);
    const skjson::Member fields_data[] = {
        { StringValue("shapes" , alloc), ArrayValue(&shape, 1, alloc) },
    };

    return ObjectValue(fields_data, std::size(fields_data), alloc);
}

class GlyphCache {
public:
    struct GlyphRec {
        SkUnichar fID;
        float     fWidth;
        SkPath    fPath;
    };

    void addGlyph(const std::string_view& font_name, SkUnichar id, const SkFont& font,
                  SkGlyphID glyph) {
        std::vector<GlyphRec>& font_glyphs =
                fFontGlyphs.emplace(font_name, std::vector<GlyphRec>()).first->second;

        // We don't expect a large number of glyphs, linear search should be fine.
        for (const auto& rec : font_glyphs) {
            if (rec.fID == id) {
                return;
            }
        }

        SkPath path;
        if (!font.getPath(glyph, &path)) {
            // Only glyphs that can be represented as paths are supported for now, color glyphs are
            // ignored.  We could look into converting these to comp-based Lottie fonts if needed.

            // TODO: plumb a client-privided skottie::Logger for error reporting.
            std::cerr << "Glyph ID %d could not be converted to a path, discarding.";
        }

        float width;
        font.getWidths(&glyph, 1, &width);

        // Lottie glyph shapes are always defined at a normalized size of 100.
        const float scale = 100 / font.getSize();

        font_glyphs.push_back({
            id,
            width * scale,
            path.makeTransform(SkMatrix::Scale(scale, scale))
        });
    }

    std::tuple<Value, Value> toLottie(SkArenaAlloc& alloc, const Value& orig_fonts) const {
        auto find_font_info = [&](const std::string& font_name) -> const ObjectValue* {
            if (const ArrayValue* jlist = orig_fonts["list"]) {
                for (const auto& jfont : *jlist) {
                    if (const StringValue* jname = jfont["fName"]) {
                        if (font_name == jname->begin()) {
                            return jfont;
                        }
                    }
                }
            }

            return nullptr;
        };

        // Lottie glyph shape font data is stored in two arrays:
        //   - "fonts" holds font metadata (name, family, style, etc)
        //   - "chars" holds character data (char id, size, advance, path, etc)
        // Individual chars are associated with specific fonts based on their
        // "fFamily" and "style" props.
        std::vector<Value> fonts, chars;

        for (const auto& font : fFontGlyphs) {
            const ObjectValue* orig_font = find_font_info(font.first);
            SkASSERT(orig_font);

            // New font entry based on existing font data + updated name.
            const SkString font_name = preshapedFontName(font.first);
            orig_font->writable("fName", alloc) =
                    StringValue(font_name.c_str(), font_name.size(), alloc);
            fonts.push_back(*orig_font);

            for (const auto& glyph : font.second) {
                // New char entry.
                char glyphid_as_utf8[SkUTF::kMaxBytesInUTF8Sequence];
                size_t utf8_len = SkUTF::ToUTF8(glyph.fID, glyphid_as_utf8);

                skjson::Member fields[] = {
                    { StringValue("ch"     , alloc), StringValue(glyphid_as_utf8, utf8_len, alloc)},
                    { StringValue("fFamily", alloc), (*orig_font)["fFamily"]                      },
                    { StringValue("style"  , alloc), (*orig_font)["fStyle"]                       },
                    { StringValue("size"   , alloc), NumberValue(100)                             },
                    { StringValue("w"      , alloc), NumberValue(glyph.fWidth)                    },
                    { StringValue("data"   , alloc), pathToLottie(glyph.fPath, alloc)             },
                };

                chars.push_back(ObjectValue(fields, std::size(fields), alloc));
            }
        }

        skjson::Member fonts_fields[] = {
            { StringValue("list", alloc), ArrayValue(fonts.data(), fonts.size(), alloc) },
        };
        return std::make_tuple(ObjectValue(fonts_fields, std::size(fonts_fields), alloc),
                               ArrayValue(chars.data(), chars.size(), alloc));
    }

private:
    std::unordered_map<std::string, std::vector<GlyphRec>> fFontGlyphs;
};

class Preshaper {
public:
    Preshaper(sk_sp<ResourceProvider> rp, sk_sp<SkFontMgr> fontmgr, sk_sp<SkShapers::Factory> sfact)
        : fFontMgr(fontmgr)
        , fShapersFact(sfact)
        , fBuilder(rp ? std::move(rp) : sk_make_sp<NullResourceProvider>(),
                   std::move(fontmgr),
                   nullptr, nullptr, nullptr, nullptr, nullptr,
                   std::move(sfact),
                   &fStats, {0, 0}, 1, 1, 0)
        , fAlloc(4096)
    {}

    void preshape(const Value& jlottie) {
        fBuilder.parseFonts(jlottie["fonts"], jlottie["chars"]);

        this->preshapeComp(jlottie);
        if (const ArrayValue* jassets = jlottie["assets"]) {
            for (const auto& jasset : *jassets) {
                this->preshapeComp(jasset);
            }
        }

        const auto& [fonts, chars] = fGlyphCache.toLottie(fAlloc, jlottie["fonts"]);

        jlottie.as<ObjectValue>().writable("fonts", fAlloc) = fonts;
        jlottie.as<ObjectValue>().writable("chars", fAlloc) = chars;
    }

private:
    class NullResourceProvider final : public ResourceProvider {
        sk_sp<SkData> load(const char[], const char[]) const override { return nullptr; }
    };

    void preshapeComp(const Value& jcomp) {
       if (const ArrayValue* jlayers = jcomp["layers"]) {
           for (const auto& jlayer : *jlayers) {
               this->preshapeLayer(jlayer);
           }
       }
    }

    void preshapeLayer(const Value& jlayer) {
        static constexpr int kTextLayerType = 5;
        if (skottie::ParseDefault<int>(jlayer["ty"], -1) != kTextLayerType) {
            return;
        }

        const ArrayValue* jtxts = jlayer["t"]["d"]["k"];
        if (!jtxts) {
            return;
        }

        for (const auto& jtxt : *jtxts) {
            const Value& jtxt_val = jtxt["s"];

            const StringValue* jfont_name = jtxt_val["f"];
            skottie::TextValue txt_val;
            if (!skottie::internal::Parse(jtxt_val, fBuilder , &txt_val) || !jfont_name) {
                continue;
            }

            const std::string_view font_name(jfont_name->begin(), jfont_name->size());

            static constexpr float kMinSize =    0.1f,
                                   kMaxSize = 1296.0f;
            const skottie::Shaper::TextDesc text_desc = {
                txt_val.fTypeface,
                SkTPin(txt_val.fTextSize,    kMinSize, kMaxSize),
                SkTPin(txt_val.fMinTextSize, kMinSize, kMaxSize),
                SkTPin(txt_val.fMaxTextSize, kMinSize, kMaxSize),
                txt_val.fLineHeight,
                txt_val.fLineShift,
                txt_val.fAscent,
                txt_val.fHAlign,
                txt_val.fVAlign,
                txt_val.fResize,
                txt_val.fLineBreak,
                txt_val.fDirection,
                txt_val.fCapitalization,
                txt_val.fMaxLines,
                skottie::Shaper::Flags::kFragmentGlyphs |
                    skottie::Shaper::Flags::kTrackFragmentAdvanceAscent |
                    skottie::Shaper::Flags::kClusters,
                txt_val.fLocale.isEmpty()     ? nullptr : txt_val.fLocale.c_str(),
                txt_val.fFontFamily.isEmpty() ? nullptr : txt_val.fFontFamily.c_str(),
            };

            auto shape_result = skottie::Shaper::Shape(txt_val.fText, text_desc, txt_val.fBox,
                                                       fFontMgr, fShapersFact);

            auto shaped_glyph_info = [this](SkUnichar ch, const SkPoint& pos, float advance,
                                            size_t line, size_t cluster) -> Value {
                const NumberValue jpos[] = { NumberValue(pos.fX), NumberValue(pos.fY) };
                char utf8[SkUTF::kMaxBytesInUTF8Sequence];
                const size_t utf8_len = SkUTF::ToUTF8(ch, utf8);

                const skjson::Member fields[] = {
                    { StringValue("ch" , fAlloc), StringValue(utf8, utf8_len, fAlloc)       },
                    { StringValue("ps" , fAlloc), ArrayValue(jpos, std::size(jpos), fAlloc) },
                    { StringValue("w"  , fAlloc), NumberValue(advance)                      },
                    { StringValue("l"  , fAlloc), NumberValue(SkToInt(line))                },
                    { StringValue("cix", fAlloc), NumberValue(SkToInt(cluster))             },
                };

                return ObjectValue(fields, std::size(fields), fAlloc);
            };

            std::vector<Value> shaped_info;
            for (const auto& frag : shape_result.fFragments) {
                SkASSERT(frag.fGlyphs.fGlyphIDs.size() == 1);
                SkASSERT(frag.fGlyphs.fClusters.size() == frag.fGlyphs.fGlyphIDs.size());
                size_t offset = 0;
                for (const auto& runrec : frag.fGlyphs.fRuns) {
                    const SkGlyphID*  glyphs = frag.fGlyphs.fGlyphIDs.data() + offset;
                    const SkPoint* glyph_pos = frag.fGlyphs.fGlyphPos.data() + offset;
                    const size_t*   clusters = frag.fGlyphs.fClusters.data() + offset;
                    const char*     end_utf8 = txt_val.fText.c_str() + txt_val.fText.size();
                    for (size_t i = 0; i < runrec.fSize; ++i) {
                        // TODO: we are only considering the fist code point in the cluster,
                        // similar to how Lottie handles custom/path-based fonts at the moment.
                        // To correctly handle larger clusters, we'll have to check for collisions
                        // and potentially allocate a synthetic glyph IDs.  TBD.
                        const char* ch_utf8 = txt_val.fText.c_str() + clusters[i];
                        const SkUnichar ch = SkUTF::NextUTF8(&ch_utf8, end_utf8);

                        fGlyphCache.addGlyph(font_name, ch, runrec.fFont, glyphs[i]);
                        shaped_info.push_back(shaped_glyph_info(ch,
                                                                frag.fOrigin + glyph_pos[i],
                                                                frag.fAdvance,
                                                                frag.fLineIndex,
                                                                clusters[i]));
                    }
                    offset += runrec.fSize;
                }
            }

            // Preshaped glyphs.
            jtxt_val.as<ObjectValue>().writable("gl", fAlloc) =
                ArrayValue(shaped_info.data(), shaped_info.size(), fAlloc);
            // Effecive size for preshaped glyphs, accounting for auto-sizing scale.
            jtxt_val.as<ObjectValue>().writable("gs", fAlloc) =
                NumberValue(text_desc.fTextSize * shape_result.fScale);
            // Updated font name.
            jtxt_val.as<ObjectValue>().writable("f", fAlloc) =
                StringValue(preshapedFontName(font_name).c_str(), fAlloc);
        }
    }

    const sk_sp<SkFontMgr>              fFontMgr;
    const sk_sp<SkShapers::Factory>     fShapersFact;
    skottie::Animation::Builder::Stats  fStats;
    skottie::internal::AnimationBuilder fBuilder;
    SkArenaAlloc                        fAlloc;
    GlyphCache                          fGlyphCache;
};

} //  namespace

namespace skottie_utils {

bool Preshape(const char* json, size_t size, SkWStream* stream,
              const sk_sp<SkFontMgr>& fmgr,
              const sk_sp<SkShapers::Factory>& sfact,
              const sk_sp<skresources::ResourceProvider>& rp) {
    skjson::DOM dom(json, size);
    if (!dom.root().is<skjson::ObjectValue>()) {
        return false;
    }

    Preshaper preshaper(rp, fmgr, sfact);

    preshaper.preshape(dom.root());

    stream->writeText(dom.root().toString().c_str());

    return true;
}

bool Preshape(const sk_sp<SkData>& json, SkWStream* stream,
              const sk_sp<SkFontMgr>& fmgr,
              const sk_sp<SkShapers::Factory>& sfact,
              const sk_sp<ResourceProvider>& rp) {
    return Preshape(static_cast<const char*>(json->data()), json->size(), stream, fmgr, sfact, rp);
}

} //  namespace skottie_utils
