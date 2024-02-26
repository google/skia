/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/text/Font.h"

#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkRect.h"
#include "include/core/SkSize.h"
#include "include/core/SkTypeface.h"
#include "include/private/base/SkTFitsIn.h"
#include "include/private/base/SkTo.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/sksg/include/SkSGPath.h"
#include "modules/sksg/include/SkSGTransform.h"
#include "src/base/SkUTF.h"
#include "src/utils/SkJSON.h"

namespace skottie::internal {

bool CustomFont::Builder::parseGlyph(const AnimationBuilder* abuilder,
                                     const skjson::ObjectValue& jchar) {
    // Glyph encoding:
    //     {
    //         "ch": "t",
    //         "data": <glyph data>,  // Glyph path or composition data
    //         "size": 50,            // apparently ignored
    //         "w": 32.67,            // width/advance (1/100 units)
    //         "t": 1                 // Marker for composition glyphs only.
    //     }
    const skjson::StringValue* jch   = jchar["ch"];
    const skjson::ObjectValue* jdata = jchar["data"];
    if (!jch || !jdata) {
        return false;
    }

    const auto* ch_ptr = jch->begin();
    const auto  ch_len = jch->size();
    if (SkUTF::CountUTF8(ch_ptr, ch_len) != 1) {
        return false;
    }

    const auto uni = SkUTF::NextUTF8(&ch_ptr, ch_ptr + ch_len);
    SkASSERT(uni != -1);
    if (!SkTFitsIn<SkGlyphID>(uni)) {
        // Custom font keys are SkGlyphIDs.  We could implement a remapping scheme if needed,
        // but for now direct mapping seems to work well enough.
        return false;
    }
    const auto glyph_id = SkTo<SkGlyphID>(uni);

    // Normalize the path and advance for 1pt.
    static constexpr float kPtScale = 0.01f;
    const auto advance = ParseDefault(jchar["w"], 0.0f) * kPtScale;

    // Custom glyphs are either compositions...
    SkSize glyph_size;
    if (auto comp_node = ParseGlyphComp(abuilder, *jdata, &glyph_size)) {
        // With glyph comps, we use the SkCustomTypeface only for shaping -- not for rendering.
        // We still need accurate glyph bounds though, for visual alignment.

        // TODO: This assumes the glyph origin is always in the lower-left corner.
        // Lottie may need to add an origin property, to allow designers full control over
        // glyph comp positioning.
        const auto glyph_bounds = SkRect::MakeLTRB(0, -glyph_size.fHeight, glyph_size.fWidth, 0);
        fCustomBuilder.setGlyph(glyph_id, advance, SkPath::Rect(glyph_bounds));

        // Rendering is handled explicitly, post shaping,
        // based on info tracked in this GlyphCompMap.
        fGlyphComps.set(glyph_id, std::move(comp_node));

        return true;
    }

    // ... or paths.
    SkPath path;
    if (!ParseGlyphPath(abuilder, *jdata, &path)) {
        return false;
    }

    path.transform(SkMatrix::Scale(kPtScale, kPtScale));

    fCustomBuilder.setGlyph(glyph_id, advance, path);

    return true;
}

bool CustomFont::Builder::ParseGlyphPath(const skottie::internal::AnimationBuilder* abuilder,
                                         const skjson::ObjectValue& jdata,
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

    const skjson::ArrayValue* jshapes = jdata["shapes"];
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
            skottie::internal::AnimationBuilder::AutoScope ascope(abuilder);
            auto path_node = abuilder->attachPath((*jshape)["ks"]);
            auto animators = ascope.release();

            if (!path_node || !animators.empty()) {
                return false;
            }

            path->addPath(path_node->getPath());
        }
    }

    return true;
}

sk_sp<sksg::RenderNode>
CustomFont::Builder::ParseGlyphComp(const AnimationBuilder* abuilder,
                                    const skjson::ObjectValue& jdata,
                                    SkSize* glyph_size) {
    // Glyph comp encoding:
    //
    //   "data": {                     // Follows the precomp layer format.
    //       "ip": <in point>,
    //       "op": <out point>,
    //       "refId": <comp ID>,
    //       "sr": <time remap info>,
    //       "st": <time remap info>,
    //       "ks": <transform info>
    //   }

    AnimationBuilder::LayerInfo linfo{
        {0,0},
        ParseDefault<float>(jdata["ip"], 0.0f),
        ParseDefault<float>(jdata["op"], 0.0f)
    };

    if (!linfo.fInPoint && !linfo.fOutPoint) {
        // Not a comp glyph.
        return nullptr;
    }

    // Since the glyph composition encoding matches the precomp layer encoding, we can pretend
    // we're attaching a precomp here.
    auto comp_node = abuilder->attachPrecompLayer(jdata, &linfo);

    // Normalize for 1pt.
    static constexpr float kPtScale = 0.01f;

    // For bounds/alignment purposes, we use a glyph size matching the normalized glyph comp size.
    *glyph_size = {linfo.fSize.fWidth * kPtScale, linfo.fSize.fHeight * kPtScale};

    sk_sp<sksg::Transform> glyph_transform =
            sksg::Matrix<SkMatrix>::Make(SkMatrix::Scale(kPtScale, kPtScale));

    // Additional/explicit glyph transform (not handled in attachPrecompLayer).
    if (const skjson::ObjectValue* jtransform = jdata["ks"]) {
        glyph_transform = abuilder->attachMatrix2D(*jtransform, std::move(glyph_transform));
    }

    return sksg::TransformEffect::Make(abuilder->attachPrecompLayer(jdata, &linfo),
                                       std::move(glyph_transform));
}

std::unique_ptr<CustomFont> CustomFont::Builder::detach() {
    return std::unique_ptr<CustomFont>(new CustomFont(std::move(fGlyphComps),
                                                      fCustomBuilder.detach()));
}

CustomFont::CustomFont(GlyphCompMap&& glyph_comps, sk_sp<SkTypeface> tf)
    : fGlyphComps(std::move(glyph_comps))
    , fTypeface(std::move(tf))
{}

CustomFont::~CustomFont() = default;

sk_sp<sksg::RenderNode> CustomFont::GlyphCompMapper::getGlyphComp(const SkTypeface* tf,
                                                                  SkGlyphID gid) const {
    for (const auto& font : fFonts) {
        if (font->typeface().get() == tf) {
            auto* comp_node = font->fGlyphComps.find(gid);
            return comp_node ? *comp_node : nullptr;
        }
    }

    return nullptr;
}

}  // namespace skottie::internal
