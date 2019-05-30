/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/text/TextAdapter.h"
#include "modules/sksg/include/SkSGDraw.h"
#include "modules/sksg/include/SkSGGroup.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "modules/sksg/include/SkSGRect.h"
#include "modules/sksg/include/SkSGText.h"
#include "modules/sksg/include/SkSGTransform.h"

namespace skottie {

TextAdapter::TextAdapter(sk_sp<sksg::Group> root, bool hasAnimators)
    : fRoot(std::move(root))
    , fHasAnimators(hasAnimators) {}

TextAdapter::~TextAdapter() = default;

struct TextAdapter::FragmentRec {
    // More text SG props will surface here as we add range selector support.
    sk_sp<sksg::TransformEffect> fRoot;
};

TextAdapter::FragmentRec TextAdapter::buildFragment(const skottie::Shaper::Fragment& frag) const {
    // For a given shaped fragment, build a corresponding SG fragment:
    //
    //   [TransformEffect] -> [Transform]
    //     [Group]
    //       [Draw] -> [TextBlob*] [FillPaint]
    //       [Draw] -> [TextBlob*] [StrokePaint]
    //
    // * where the blob node is shared

    auto blob_node = sksg::TextBlob::Make(frag.fBlob);
    blob_node->setPosition(frag.fPos);

    std::vector<sk_sp<sksg::RenderNode>> draws;
    draws.reserve(static_cast<size_t>(fText.fHasFill) + static_cast<size_t>(fText.fHasStroke));

    SkASSERT(fText.fHasFill || fText.fHasStroke);

    if (fText.fHasFill) {
        auto fill_paint = sksg::Color::Make(fText.fFillColor);
        fill_paint->setAntiAlias(true);
        draws.push_back(sksg::Draw::Make(blob_node, std::move(fill_paint)));
    }
    if (fText.fHasStroke) {
        auto stroke_paint = sksg::Color::Make(fText.fStrokeColor);
        stroke_paint->setAntiAlias(true);
        stroke_paint->setStyle(SkPaint::kStroke_Style);
        draws.push_back(sksg::Draw::Make(blob_node, std::move(stroke_paint)));
    }

    SkASSERT(!draws.empty());

    auto draws_node = (draws.size() > 1)
            ? sksg::Group::Make(std::move(draws))
            : std::move(draws[0]);

    return {
        sksg::TransformEffect::Make(std::move(draws_node), SkMatrix::I()),
    };
}

void TextAdapter::apply() {
    if (!fText.fHasFill && !fText.fHasStroke) {
        return;
    }

    const Shaper::TextDesc text_desc = {
        fText.fTypeface,
        fText.fTextSize,
        fText.fLineHeight,
        fText.fHAlign,
        fText.fVAlign,
        fHasAnimators ? Shaper::Flags::kFragmentGlyphs : Shaper::Flags::kNone,
    };
    const auto shape_result = Shaper::Shape(fText.fText, text_desc, fText.fBox);

    // Rebuild all fragments.
    // TODO: we can be smarter here and try to reuse the existing SG structure if needed.

    fRoot->clear();
    fFragments.clear();

    for (const auto& frag : shape_result.fFragments) {
        fFragments.push_back(this->buildFragment(frag));
        fRoot->addChild(fFragments.back().fRoot);
    }

#if (0)
    // Enable for text box debugging/visualization.
    auto box_color = sksg::Color::Make(0xffff0000);
    box_color->setStyle(SkPaint::kStroke_Style);
    box_color->setStrokeWidth(1);
    box_color->setAntiAlias(true);

    auto bounds_color = sksg::Color::Make(0xff00ff00);
    bounds_color->setStyle(SkPaint::kStroke_Style);
    bounds_color->setStrokeWidth(1);
    bounds_color->setAntiAlias(true);

    fRoot->addChild(sksg::Draw::Make(sksg::Rect::Make(fText.fBox),
                                     std::move(box_color)));
    fRoot->addChild(sksg::Draw::Make(sksg::Rect::Make(shape_result.computeBounds()),
                                     std::move(bounds_color)));
#endif
}

} // namespace skottie
