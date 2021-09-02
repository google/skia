/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/text/TextAdapter.h"

#include "include/core/SkFontMgr.h"
#include "include/core/SkM44.h"
#include "include/private/SkTPin.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/text/RangeSelector.h"
#include "modules/skottie/src/text/TextAnimator.h"
#include "modules/sksg/include/SkSGDraw.h"
#include "modules/sksg/include/SkSGGroup.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "modules/sksg/include/SkSGRect.h"
#include "modules/sksg/include/SkSGRenderEffect.h"
#include "modules/sksg/include/SkSGText.h"
#include "modules/sksg/include/SkSGTransform.h"

namespace skottie {
namespace internal {

sk_sp<TextAdapter> TextAdapter::Make(const skjson::ObjectValue& jlayer,
                                     const AnimationBuilder* abuilder,
                                     sk_sp<SkFontMgr> fontmgr, sk_sp<Logger> logger) {
    // General text node format:
    // "t": {
    //    "a": [], // animators (see TextAnimator)
    //    "d": {
    //        "k": [
    //            {
    //                "s": {
    //                    "f": "Roboto-Regular",
    //                    "fc": [
    //                        0.42,
    //                        0.15,
    //                        0.15
    //                    ],
    //                    "j": 1,
    //                    "lh": 60,
    //                    "ls": 0,
    //                    "s": 50,
    //                    "t": "text align right",
    //                    "tr": 0
    //                },
    //                "t": 0
    //            }
    //        ]
    //    },
    //    "m": { // "more options"
    //           "g": 1,     // Anchor Point Grouping
    //           "a": {...}  // Grouping Alignment
    //         },
    //    "p": {}  // "path options" (TODO)
    // },

    const skjson::ObjectValue* jt = jlayer["t"];
    const skjson::ObjectValue* jd = jt ? static_cast<const skjson::ObjectValue*>((*jt)["d"])
                                       : nullptr;
    if (!jd) {
        abuilder->log(Logger::Level::kError, &jlayer, "Invalid text layer.");
        return nullptr;
    }

    // "More options"
    const skjson::ObjectValue* jm = (*jt)["m"];
    static constexpr AnchorPointGrouping gGroupingMap[] = {
        AnchorPointGrouping::kCharacter, // 'g': 1
        AnchorPointGrouping::kWord,      // 'g': 2
        AnchorPointGrouping::kLine,      // 'g': 3
        AnchorPointGrouping::kAll,       // 'g': 4
    };
    const auto apg = jm
            ? SkTPin<int>(ParseDefault<int>((*jm)["g"], 1), 1, SK_ARRAY_COUNT(gGroupingMap))
            : 1;

    auto adapter = sk_sp<TextAdapter>(new TextAdapter(std::move(fontmgr),
                                                      std::move(logger),
                                                      gGroupingMap[SkToSizeT(apg - 1)]));

    adapter->bind(*abuilder, jd, adapter->fText.fCurrentValue);
    if (jm) {
        adapter->bind(*abuilder, (*jm)["a"], adapter->fGroupingAlignment);
    }

    // Animators
    if (const skjson::ArrayValue* janimators = (*jt)["a"]) {
        adapter->fAnimators.reserve(janimators->size());

        for (const skjson::ObjectValue* janimator : *janimators) {
            if (auto animator = TextAnimator::Make(janimator, abuilder, adapter.get())) {
                adapter->fHasBlurAnimator     |= animator->hasBlur();
                adapter->fRequiresAnchorPoint |= animator->requiresAnchorPoint();

                adapter->fAnimators.push_back(std::move(animator));
            }
        }
    }

    abuilder->dispatchTextProperty(adapter);

    return adapter;
}

TextAdapter::TextAdapter(sk_sp<SkFontMgr> fontmgr, sk_sp<Logger> logger, AnchorPointGrouping apg)
    : fRoot(sksg::Group::Make())
    , fFontMgr(std::move(fontmgr))
    , fLogger(std::move(logger))
    , fAnchorPointGrouping(apg)
    , fHasBlurAnimator(false)
    , fRequiresAnchorPoint(false) {}

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
    rec.fAdvance    = frag.fAdvance;
    rec.fAscent     = frag.fAscent;
    rec.fMatrixNode = sksg::Matrix<SkM44>::Make(SkM44::Translate(frag.fPos.x(), frag.fPos.y()));

    std::vector<sk_sp<sksg::RenderNode>> draws;
    draws.reserve(static_cast<size_t>(fText->fHasFill) + static_cast<size_t>(fText->fHasStroke));

    SkASSERT(fText->fHasFill || fText->fHasStroke);

    auto add_fill = [&]() {
        if (fText->fHasFill) {
            rec.fFillColorNode = sksg::Color::Make(fText->fFillColor);
            rec.fFillColorNode->setAntiAlias(true);
            draws.push_back(sksg::Draw::Make(blob_node, rec.fFillColorNode));
        }
    };
    auto add_stroke = [&] {
        if (fText->fHasStroke) {
            rec.fStrokeColorNode = sksg::Color::Make(fText->fStrokeColor);
            rec.fStrokeColorNode->setAntiAlias(true);
            rec.fStrokeColorNode->setStyle(SkPaint::kStroke_Style);
            rec.fStrokeColorNode->setStrokeWidth(fText->fStrokeWidth);
            draws.push_back(sksg::Draw::Make(blob_node, rec.fStrokeColorNode));
        }
    };

    if (fText->fPaintOrder == TextPaintOrder::kFillStroke) {
        add_fill();
        add_stroke();
    } else {
        add_stroke();
        add_fill();
    }

    SkASSERT(!draws.empty());

    if (0) {
        // enable to visualize fragment ascent boxes
        auto box_color = sksg::Color::Make(0xff0000ff);
        box_color->setStyle(SkPaint::kStroke_Style);
        box_color->setStrokeWidth(1);
        box_color->setAntiAlias(true);
        auto box = SkRect::MakeLTRB(0, rec.fAscent, rec.fAdvance, 0);
        draws.push_back(sksg::Draw::Make(sksg::Rect::Make(box), std::move(box_color)));
    }

    auto draws_node = (draws.size() > 1)
            ? sksg::Group::Make(std::move(draws))
            : std::move(draws[0]);

    if (fHasBlurAnimator) {
        // Optional blur effect.
        rec.fBlur = sksg::BlurImageFilter::Make();
        draws_node = sksg::ImageFilterEffect::Make(std::move(draws_node), rec.fBlur);
    }

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

    float word_advance = 0,
          word_ascent  = 0,
          line_advance = 0,
          line_ascent  = 0;

    bool in_word = false;

    // TODO: use ICU for building the word map?
    for (; i  < shape_result.fFragments.size(); ++i) {
        const auto& frag = shape_result.fFragments[i];

        if (frag.fIsWhitespace) {
            if (in_word) {
                in_word = false;
                fMaps.fWordsMap.push_back({word_start, i - word_start, word_advance, word_ascent});
            }
        } else {
            fMaps.fNonWhitespaceMap.push_back({i, 1, 0, 0});

            if (!in_word) {
                in_word = true;
                word_start = i;
                word_advance = word_ascent = 0;
            }

            word_advance += frag.fAdvance;
            word_ascent   = std::min(word_ascent, frag.fAscent); // negative ascent
        }

        if (frag.fLineIndex != line) {
            SkASSERT(frag.fLineIndex == line + 1);
            fMaps.fLinesMap.push_back({line_start, i - line_start, line_advance, line_ascent});
            line = frag.fLineIndex;
            line_start = i;
            line_advance = line_ascent = 0;
        }

        line_advance += frag.fAdvance;
        line_ascent   = std::min(line_ascent, frag.fAscent); // negative ascent
    }

    if (i > word_start) {
        fMaps.fWordsMap.push_back({word_start, i - word_start, word_advance, word_ascent});
    }

    if (i > line_start) {
        fMaps.fLinesMap.push_back({line_start, i - line_start, line_advance, line_ascent});
    }
}

void TextAdapter::setText(const TextValue& txt) {
    fText.fCurrentValue = txt;
    this->onSync();
}

uint32_t TextAdapter::shaperFlags() const {
    uint32_t flags = Shaper::Flags::kNone;

    SkASSERT(!(fRequiresAnchorPoint && fAnimators.empty()));
    if (!fAnimators.empty() ) flags |= Shaper::Flags::kFragmentGlyphs;
    if (fRequiresAnchorPoint) flags |= Shaper::Flags::kTrackFragmentAdvanceAscent;

    return flags;
}

void TextAdapter::reshape() {
    const Shaper::TextDesc text_desc = {
        fText->fTypeface,
        fText->fTextSize,
        fText->fMinTextSize,
        fText->fMaxTextSize,
        fText->fLineHeight,
        fText->fLineShift,
        fText->fAscent,
        fText->fHAlign,
        fText->fVAlign,
        fText->fResize,
        fText->fLineBreak,
        fText->fDirection,
        fText->fCapitalization,
        this->shaperFlags(),
    };
    const auto shape_result = Shaper::Shape(fText->fText, text_desc, fText->fBox, fFontMgr);

    if (fLogger && shape_result.fMissingGlyphCount > 0) {
        const auto msg = SkStringPrintf("Missing %zu glyphs for '%s'.",
                                        shape_result.fMissingGlyphCount,
                                        fText->fText.c_str());
        fLogger->log(Logger::Level::kWarning, msg.c_str());

        // This may trigger repeatedly when the text is animating.
        // To avoid spamming, only log once.
        fLogger = nullptr;
    }

    // Rebuild all fragments.
    // TODO: we can be smarter here and try to reuse the existing SG structure if needed.

    fRoot->clear();
    fFragments.clear();

    for (const auto& frag : shape_result.fFragments) {
        this->addFragment(frag);
    }

    if (!fAnimators.empty()) {
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

    fRoot->addChild(sksg::Draw::Make(sksg::Rect::Make(fText->fBox),
                                     std::move(box_color)));
    fRoot->addChild(sksg::Draw::Make(sksg::Rect::Make(shape_result.computeVisualBounds()),
                                     std::move(bounds_color)));
#endif
}

void TextAdapter::onSync() {
    if (!fText->fHasFill && !fText->fHasStroke) {
        return;
    }

    if (fText.hasChanged()) {
        this->reshape();
    }

    if (fFragments.empty()) {
        return;
    }

    // Seed props from the current text value.
    TextAnimator::ResolvedProps seed_props;
    seed_props.fill_color   = fText->fFillColor;
    seed_props.stroke_color = fText->fStrokeColor;

    TextAnimator::ModulatorBuffer buf;
    buf.resize(fFragments.size(), { seed_props, 0 });

    // Apply all animators to the modulator buffer.
    for (const auto& animator : fAnimators) {
        animator->modulateProps(fMaps, buf);
    }

    const TextAnimator::DomainMap* grouping_domain = nullptr;
    switch (fAnchorPointGrouping) {
        // for word/line grouping, we rely on domain map info
        case AnchorPointGrouping::kWord: grouping_domain = &fMaps.fWordsMap; break;
        case AnchorPointGrouping::kLine: grouping_domain = &fMaps.fLinesMap; break;
        // remaining grouping modes (character/all) do not need (or have) domain map data
        default: break;
    }

    size_t grouping_span_index = 0;
    SkV2           line_offset = { 0, 0 }; // cumulative line spacing

    // Finally, push all props to their corresponding fragment.
    for (const auto& line_span : fMaps.fLinesMap) {
        SkV2 line_spacing = { 0, 0 };
        float line_tracking = 0;
        bool line_has_tracking = false;

        // Tracking requires special treatment: unlike other props, its effect is not localized
        // to a single fragment, but requires re-alignment of the whole line.
        for (size_t i = line_span.fOffset; i < line_span.fOffset + line_span.fCount; ++i) {
            // Track the grouping domain span in parallel.
            if (grouping_domain && i >= (*grouping_domain)[grouping_span_index].fOffset +
                                        (*grouping_domain)[grouping_span_index].fCount) {
                grouping_span_index += 1;
                SkASSERT(i < (*grouping_domain)[grouping_span_index].fOffset +
                             (*grouping_domain)[grouping_span_index].fCount);
            }

            const auto& props = buf[i].props;
            const auto& frag  = fFragments[i];
            this->pushPropsToFragment(props, frag, fGroupingAlignment * .01f, // percentage
                                      grouping_domain ? &(*grouping_domain)[grouping_span_index]
                                                        : nullptr);

            line_tracking += props.tracking;
            line_has_tracking |= !SkScalarNearlyZero(props.tracking);

            line_spacing += props.line_spacing;
        }

        // line spacing of the first line is ignored (nothing to "space" against)
        if (&line_span != &fMaps.fLinesMap.front()) {
            // For each line, the actual spacing is an average of individual fragment spacing
            // (to preserve the "line").
            line_offset += line_spacing / line_span.fCount;
        }

        if (line_offset != SkV2{0, 0} || line_has_tracking) {
            this->adjustLineProps(buf, line_span, line_offset, line_tracking);
        }

    }
}

SkV2 TextAdapter::fragmentAnchorPoint(const FragmentRec& rec,
                                      const SkV2& grouping_alignment,
                                      const TextAnimator::DomainSpan* grouping_span) const {
    // Construct the following 2x ascent box:
    //
    //      -------------
    //     |             |
    //     |             | ascent
    //     |             |
    // ----+-------------+---------- baseline
    //   (pos)           |
    //     |             | ascent
    //     |             |
    //      -------------
    //         advance

    auto make_box = [](const SkPoint& pos, float advance, float ascent) {
        // note: negative ascent
        return SkRect::MakeXYWH(pos.fX, pos.fY + ascent, advance, -2 * ascent);
    };

    // Compute a grouping-dependent anchor point box.
    // The default anchor point is at the center, and gets adjusted relative to the bounds
    // based on |grouping_alignment|.
    auto anchor_box = [&]() -> SkRect {
        switch (fAnchorPointGrouping) {
        case AnchorPointGrouping::kCharacter:
            // Anchor box relative to each individual fragment.
            return make_box(rec.fOrigin, rec.fAdvance, rec.fAscent);
        case AnchorPointGrouping::kWord:
            // Fall through
        case AnchorPointGrouping::kLine: {
            SkASSERT(grouping_span);
            // Anchor box relative to the first fragment in the word/line.
            const auto& first_span_fragment = fFragments[grouping_span->fOffset];
            return make_box(first_span_fragment.fOrigin,
                            grouping_span->fAdvance,
                            grouping_span->fAscent);
        }
        case AnchorPointGrouping::kAll:
            // Anchor box is the same as the text box.
            return fText->fBox;
        }
        SkUNREACHABLE;
    };

    const auto ab = anchor_box();

    // Apply grouping alignment.
    const auto ap = SkV2 { ab.centerX() + ab.width()  * 0.5f * grouping_alignment.x,
                           ab.centerY() + ab.height() * 0.5f * grouping_alignment.y };

    // The anchor point is relative to the fragment position.
    return ap - SkV2 { rec.fOrigin.fX, rec.fOrigin.fY };
}

void TextAdapter::pushPropsToFragment(const TextAnimator::ResolvedProps& props,
                                      const FragmentRec& rec,
                                      const SkV2& grouping_alignment,
                                      const TextAnimator::DomainSpan* grouping_span) const {
    const auto anchor_point = this->fragmentAnchorPoint(rec, grouping_alignment, grouping_span);

    rec.fMatrixNode->setMatrix(
                SkM44::Translate(props.position.x + rec.fOrigin.x() + anchor_point.x,
                                 props.position.y + rec.fOrigin.y() + anchor_point.y,
                                 props.position.z)
              * SkM44::Rotate({ 1, 0, 0 }, SkDegreesToRadians(props.rotation.x))
              * SkM44::Rotate({ 0, 1, 0 }, SkDegreesToRadians(props.rotation.y))
              * SkM44::Rotate({ 0, 0, 1 }, SkDegreesToRadians(props.rotation.z))
              * SkM44::Scale(props.scale.x, props.scale.y, props.scale.z)
              * SkM44::Translate(-anchor_point.x, -anchor_point.y, 0));

    const auto scale_alpha = [](SkColor c, float o) {
        return SkColorSetA(c, SkScalarRoundToInt(o * SkColorGetA(c)));
    };

    if (rec.fFillColorNode) {
        rec.fFillColorNode->setColor(scale_alpha(props.fill_color, props.opacity));
    }
    if (rec.fStrokeColorNode) {
        rec.fStrokeColorNode->setColor(scale_alpha(props.stroke_color, props.opacity));
    }
    if (rec.fBlur) {
        rec.fBlur->setSigma({ props.blur.x * kBlurSizeToSigma,
                              props.blur.y * kBlurSizeToSigma });
    }
}

void TextAdapter::adjustLineProps(const TextAnimator::ModulatorBuffer& buf,
                                  const TextAnimator::DomainSpan& line_span,
                                  const SkV2& line_offset,
                                  float total_tracking) const {
    SkASSERT(line_span.fCount > 0);

    // AE tracking is defined per glyph, based on two components: |before| and |after|.
    // BodyMovin only exports "balanced" tracking values, where before == after == tracking / 2.
    //
    // Tracking is applied as a local glyph offset, and contributes to the line width for alignment
    // purposes.

    // The first glyph does not contribute |before| tracking, and the last one does not contribute
    // |after| tracking.  Rather than spill this logic into applyAnimators, post-adjust here.
    total_tracking -= 0.5f * (buf[line_span.fOffset].props.tracking +
                              buf[line_span.fOffset + line_span.fCount - 1].props.tracking);

    static const auto align_factor = [](SkTextUtils::Align a) {
        switch (a) {
        case SkTextUtils::kLeft_Align  : return  0.0f;
        case SkTextUtils::kCenter_Align: return -0.5f;
        case SkTextUtils::kRight_Align : return -1.0f;
        }

        SkASSERT(false);
        return 0.0f;
    };

    const auto align_offset = total_tracking * align_factor(fText->fHAlign);

    float tracking_acc = 0;
    for (size_t i = line_span.fOffset; i < line_span.fOffset + line_span.fCount; ++i) {
        const auto& props = buf[i].props;

        // No |before| tracking for the first glyph, nor |after| tracking for the last one.
        const auto track_before = i > line_span.fOffset
                                    ? props.tracking * 0.5f : 0.0f,
                   track_after  = i < line_span.fOffset + line_span.fCount - 1
                                    ? props.tracking * 0.5f : 0.0f,
                fragment_offset = align_offset + tracking_acc + track_before;

        const auto& frag = fFragments[i];
        const auto m = SkM44::Translate(line_offset.x + fragment_offset,
                                        line_offset.y) *
                       frag.fMatrixNode->getMatrix();
        frag.fMatrixNode->setMatrix(m);

        tracking_acc += track_before + track_after;
    }
}

} // namespace internal
} // namespace skottie
