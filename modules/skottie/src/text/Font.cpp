/*
 * Copyright 2022 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/text/Font.h"

#include "include/core/SkPath.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/SkottiePriv.h"
#include "modules/sksg/include/SkSGPath.h"

namespace skottie::internal {

bool Font::Builder::parseGlyph(const AnimationBuilder* abuilder,
                               const skjson::ObjectValue& jchar) {
    // Glyph encoding:
    //     {
    //         "ch": "t",
    //         "data": {
    //             "shapes": [...]        // shape-layer-like geometry (for path glyphs)
    //             "refId: ..."           // precomp ID (for animated glyphs)
    //         },
    //         "size": 50,                // apparently ignored
    //         "w": 32.67                 // width/advance (1/100 units)
    //    }
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

    static constexpr float kPtScale = 0.01f;
    const auto advance = ParseDefault(jchar["w"], 0.0f) * kPtScale;

    SkPath path;
    if (!ParseGlyphPath(abuilder, *jdata, &path)) {
        return false;
    }

    // Normalize the path and advance for 1pt.
    path.transform(SkMatrix::Scale(kPtScale, kPtScale));

    fCustomBuilder.setGlyph(glyph_id, advance, path);

    return true;
}

bool Font::Builder::ParseGlyphPath(const skottie::internal::AnimationBuilder* abuilder,
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

std::unique_ptr<Font> Font::Builder::detach() {
    return std::unique_ptr<Font>(new Font(fCustomBuilder.detach()));
}

}  // namespace skottie::internal
