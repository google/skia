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
#include "modules/sksg/include/SkSGText.h"

namespace skottie {

TextAdapter::TextAdapter(sk_sp<sksg::Group> root)
    : fRoot(std::move(root))
    , fTextNode(sksg::TextBlob::Make())
    , fFillColor(sksg::Color::Make(SK_ColorTRANSPARENT))
    , fStrokeColor(sksg::Color::Make(SK_ColorTRANSPARENT))
    , fFillNode(sksg::Draw::Make(fTextNode, fFillColor))
    , fStrokeNode(sksg::Draw::Make(fTextNode, fStrokeColor))
    , fHadFill(false)
    , fHadStroke(false) {
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

    fFillColor->setAntiAlias(true);
    fStrokeColor->setAntiAlias(true);
    fStrokeColor->setStyle(SkPaint::kStroke_Style);
}

TextAdapter::~TextAdapter() = default;

void TextAdapter::apply() {
    const Shaper::TextDesc text_desc = {
        fText.fTypeface,
        fText.fTextSize,
        fText.fLineHeight,
        fText.fHAlign,
        fText.fVAlign,
    };
    const auto shape_result = Shaper::Shape(fText.fText, text_desc, fText.fBox);

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

    fTextNode->setBlob(shape_result.fBlob);
    fTextNode->setPosition(shape_result.fPos);

    fFillColor->setColor(fText.fFillColor);
    fStrokeColor->setColor(fText.fStrokeColor);
    fStrokeColor->setStrokeWidth(fText.fStrokeWidth);

    // Turn the state transition into a tri-state value:
    //   -1: detach node
    //    0: no change
    //    1: attach node
    const auto   fill_change = SkToInt(fText.fHasFill) - SkToInt(fHadFill);
    const auto stroke_change = SkToInt(fText.fHasStroke) - SkToInt(fHadStroke);

    // Sync SG topology.
    if (fill_change || stroke_change) {
        // This is trickier than it should be because sksg::Group only allows adding children
        // in paint-order.
        if (stroke_change < 0 || (fHadStroke && fill_change > 0)) {
            fRoot->removeChild(fStrokeNode);
        }

        if (fill_change < 0) {
            fRoot->removeChild(fFillNode);
        } else if (fill_change > 0) {
            fRoot->addChild(fFillNode);
        }

        if (stroke_change > 0 || (fHadStroke && fill_change > 0)) {
            fRoot->addChild(fStrokeNode);
        }
    }

    // Track current state.
    fHadFill   = fText.fHasFill;
    fHadStroke = fText.fHasStroke;
}

} // namespace skottie
