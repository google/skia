/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/text/TextAdapter.h"

#include "modules/skottie/src/text/TextAnimator.h"
#include "modules/sksg/include/SkSGDraw.h"
#include "modules/sksg/include/SkSGGroup.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "modules/sksg/include/SkSGRect.h"
#include "modules/sksg/include/SkSGText.h"
#include "modules/sksg/include/SkSGTransform.h"

namespace skottie {
namespace internal {

TextAdapter::TextAdapter(sk_sp<sksg::Group> root, bool hasAnimators)
    : fRoot(std::move(root))
    , fHasAnimators(hasAnimators) {}

TextAdapter::~TextAdapter() = default;

void TextAdapter::addFragment(const Shaper::Fragment& frag) {
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
    rec.fOrigin     = frag.fPos;
    rec.fLineIndex  = frag.fLineIndex;
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

void TextAdapter::buildDomainMaps(const Shaper::Result& shape_result) {
    fMaps.fNonWhitespaceMap.clear();
    fMaps.fWordsMap.clear();
    fMaps.fLinesMap.clear();

    size_t i          = 0,
           line       = 0,
           line_start = 0,
           word_start = 0;
    bool in_word = false;

    // TODO: use ICU for building the word map?
    for (; i  < shape_result.fFragments.size(); ++i) {
        const auto& frag = shape_result.fFragments[i];

        if (frag.fIsWhitespace) {
            if (in_word) {
                in_word = false;
                fMaps.fWordsMap.push_back({word_start, i - word_start});
            }
        } else {
            fMaps.fNonWhitespaceMap.push_back({i, 1});

            if (!in_word) {
                in_word = true;
                word_start = i;
            }
        }

        if (frag.fLineIndex != line) {
            SkASSERT(frag.fLineIndex == line + 1);
            fMaps.fLinesMap.push_back({line_start, i - line_start});
            line = frag.fLineIndex;
            line_start = i;
        }
    }

    if (i > word_start) {
        fMaps.fWordsMap.push_back({word_start, i - word_start});
    }

    if (i > line_start) {
        fMaps.fLinesMap.push_back({line_start, i - line_start});
    }
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

    if (fHasAnimators) {
        // Range selectors require fragment domain maps.
        this->buildDomainMaps(shape_result);
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

void TextAdapter::applyAnimators(const std::vector<sk_sp<TextAnimator>>& animators) {
    if (fFragments.empty()) {
        return;
    }

    const auto& txt_val = this->getText();

    // Seed props from the current text value.
    TextAnimator::AnimatedProps seed_props;
    seed_props.fill_color   = txt_val.fFillColor;
    seed_props.stroke_color = txt_val.fStrokeColor;

    TextAnimator::ModulatorBuffer buf;
    buf.resize(fFragments.size(), { seed_props, 0 });

    // Apply all animators to the modulator buffer.
    for (const auto& animator : animators) {
        animator->modulateProps(fMaps, buf);
    }

    // per-line tracking info
    size_t line_index        = 0,
           line_start        = 0;
    float  line_tracking     = 0; // total fragment tracking for the current line.
    bool   line_has_tracking = false;

    // Finally, push all props to their corresponding fragment.
    size_t i = 0;
    for (; i < fFragments.size(); ++i) {
        const auto& props = buf[i].props;
        const auto& frag  = fFragments[i];
        this->pushPropsToFragment(props, frag);

        // Tracking requires special treatment: unlike other props, its effect is not localized
        // to a single fragment, but requires re-alignment of the whole line.
        if (frag.fLineIndex != line_index) {
            if (line_has_tracking) {
                this->adjustLineForTracking(buf, line_start, i - line_start, line_tracking);
            }

            line_index = frag.fLineIndex;
            line_start = i;
            line_tracking = 0;
            line_has_tracking = false;
        }

        line_tracking += props.tracking;
        line_has_tracking |= !SkScalarNearlyZero(props.tracking);
    }

    if (line_has_tracking) {
        this->adjustLineForTracking(buf, line_start, i - line_start, line_tracking);
    }
}

void TextAdapter::pushPropsToFragment(const TextAnimator::AnimatedProps& props,
                                      const FragmentRec& rec) const {
    // TODO: share this with TransformAdapter2D?
    auto t = SkMatrix::MakeTrans(rec.fOrigin.x() + props.position.x(),
                                 rec.fOrigin.y() + props.position.y());
    t.preRotate(props.rotation);
    t.preScale(props.scale, props.scale);
    rec.fMatrixNode->setMatrix(t);

    const auto scale_alpha = [](SkColor c, float o) {
        return SkColorSetA(c, SkScalarRoundToInt(o * SkColorGetA(c)));
    };

    if (rec.fFillColorNode) {
        rec.fFillColorNode->setColor(scale_alpha(props.fill_color, props.opacity));
    }
    if (rec.fStrokeColorNode) {
        rec.fStrokeColorNode->setColor(scale_alpha(props.stroke_color, props.opacity));
    }
}

void TextAdapter::adjustLineForTracking(const TextAnimator::ModulatorBuffer& buf,
                                        size_t start_index,
                                        size_t count,
                                        float total_tracking) const {
    SkASSERT(count > 0);

    total_tracking -= 0.5f * (buf[start_index].props.tracking +
                              buf[start_index + count - 1].props.tracking);

    static const auto align_factor = [](SkTextUtils::Align a) {
        switch (a) {
        case SkTextUtils::kLeft_Align  : return  0.0f;
        case SkTextUtils::kCenter_Align: return -0.5f;
        case SkTextUtils::kRight_Align : return -1.0f;
        }

        SkASSERT(false);
        return 0.0f;
    };

    const auto align_offset = total_tracking * align_factor(fText.fHAlign);

    float tracking_acc = 0;
    for (size_t i = start_index; i < start_index + count; ++i) {
        const auto& props = buf[i].props;

        const auto track_before = i > start_index             ? props.tracking * 0.5f : 0.0f,
                   track_after  = i < start_index + count - 1 ? props.tracking * 0.5f : 0.0f,
                fragment_offset = align_offset + tracking_acc + track_before;

        const auto& frag = fFragments[i];
        const auto m = SkMatrix::Concat(SkMatrix::MakeTrans(fragment_offset, 0),
                                        frag.fMatrixNode->getMatrix());
        frag.fMatrixNode->setMatrix(m);

        tracking_acc += track_before + track_after;
    }
}

} // namespace internal
} // namespace skottie
