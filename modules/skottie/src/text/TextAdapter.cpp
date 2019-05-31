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
    SkPoint                       fOrigin; // fragment position

    sk_sp<sksg::Matrix<SkMatrix>> fMatrixNode;
    sk_sp<sksg::Color>            fFillColorNode,
                                  fStrokeColorNode;
};

void TextAdapter::addFragment(const skottie::Shaper::Fragment& frag) {
    // For a given shaped fragment, build a corresponding SG fragment:
    //
    //   [TransformEffect] -> [Transform]
    //     [Group]
    //       [Draw] -> [TextBlob*] [FillPaint]
    //       [Draw] -> [TextBlob*] [StrokePaint]
    //
    // * where the blob node is shared

    auto blob_node = sksg::TextBlob::Make(frag.fBlob);

    FragmentRec rec;
    rec.fOrigin = frag.fPos;
    rec.fMatrixNode = sksg::Matrix<SkMatrix>::Make(SkMatrix::MakeTrans(frag.fPos.x(),
                                                                       frag.fPos.y()));

    std::vector<sk_sp<sksg::RenderNode>> draws;
    draws.reserve(static_cast<size_t>(fText.fHasFill) + static_cast<size_t>(fText.fHasStroke));

    SkASSERT(fText.fHasFill || fText.fHasStroke);

    if (fText.fHasFill) {
        rec.fFillColorNode = sksg::Color::Make(fText.fFillColor);
        rec.fFillColorNode->setAntiAlias(true);
        draws.push_back(sksg::Draw::Make(blob_node, rec.fFillColorNode));
    }
    if (fText.fHasStroke) {
        rec.fStrokeColorNode = sksg::Color::Make(fText.fStrokeColor);
        rec.fStrokeColorNode->setAntiAlias(true);
        rec.fStrokeColorNode->setStyle(SkPaint::kStroke_Style);
        draws.push_back(sksg::Draw::Make(blob_node, rec.fStrokeColorNode));
    }

    SkASSERT(!draws.empty());

    auto draws_node = (draws.size() > 1)
            ? sksg::Group::Make(std::move(draws))
            : std::move(draws[0]);

    fRoot->addChild(sksg::TransformEffect::Make(std::move(draws_node), rec.fMatrixNode));
    fFragments.push_back(std::move(rec));
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
        this->addFragment(frag);
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

void TextAdapter::applyAnimatedProps(const AnimatedProps& props) {
    // TODO: share this with TransformAdapter2D?
    auto t = SkMatrix::MakeTrans(props.position.x(), props.position.y());
    t.preRotate(props.rotation);
    t.preScale(props.scale, props.scale);

    const auto fc = SkColorSetA(props.fill_color,
                                SkScalarRoundToInt(props.opacity*SkColorGetA(props.fill_color))),
               sc = SkColorSetA(props.stroke_color,
                                SkScalarRoundToInt(props.opacity*SkColorGetA(props.stroke_color)));

    for (const auto& rec : fFragments) {
        rec.fMatrixNode->setMatrix(SkMatrix::Concat(SkMatrix::MakeTrans(rec.fOrigin.x(),
                                                                        rec.fOrigin.y()),
                                                    t));

        if (rec.fFillColorNode) {
            rec.fFillColorNode->setColor(fc);
        }
        if (rec.fStrokeColorNode) {
            rec.fStrokeColorNode->setColor(sc);
        }
    }
}

} // namespace skottie
