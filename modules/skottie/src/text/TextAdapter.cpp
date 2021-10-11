/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/text/TextAdapter.h"

#include "include/core/SkContourMeasure.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkM44.h"
#include "include/private/SkTPin.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/text/RangeSelector.h"
#include "modules/skottie/src/text/TextAnimator.h"
#include "modules/sksg/include/SkSGDraw.h"
#include "modules/sksg/include/SkSGGroup.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "modules/sksg/include/SkSGPath.h"
#include "modules/sksg/include/SkSGRect.h"
#include "modules/sksg/include/SkSGRenderEffect.h"
#include "modules/sksg/include/SkSGText.h"
#include "modules/sksg/include/SkSGTransform.h"

// Enable for text layout debugging.
#define SHOW_LAYOUT_BOXES 0

namespace skottie::internal {

static float align_factor(SkTextUtils::Align a) {
    switch (a) {
        case SkTextUtils::kLeft_Align  : return 0.0f;
        case SkTextUtils::kCenter_Align: return 0.5f;
        case SkTextUtils::kRight_Align : return 1.0f;
    }

    SkUNREACHABLE;
};

// Text path semantics
//
//   * glyphs are positioned on the path based on their horizontal/x anchor point, interpreted as
//     a distance along the path
//
//   * horizontal alignment is applied relative to the path start/end points
//
//   * "Reverse Path" allows reversing the path direction
//
//   * "Perpendicular To Path" determines whether glyphs are rotated to be perpendicular
//      to the path tangent, or not (just positioned).
//
//   * two controls ("First Margin" and "Last Margin") allow arbitrary offseting along the path,
//     depending on horizontal alignement:
//       - left:   offset = first margin
//       - center: offset = first margin + last margin
//       - right:  offset = last margin
//
//   * extranormal path positions (d < 0, d > path len) are allowed
//       - closed path: the position wraps around in both directions
//       - open path: extrapolates from extremes' positions/slopes
//
struct TextAdapter::PathInfo {
    ShapeValue  fPath;
    ScalarValue fPathFMargin       = 0,
                fPathLMargin       = 0,
                fPathPerpendicular = 0,
                fPathReverse       = 0;

    void updateContourData() {
        const auto reverse = fPathReverse != 0;

        if (fPath != fCurrentPath || reverse != fCurrentReversed) {
            // reinitialize cached contour data
            auto path = static_cast<SkPath>(fPath);
            if (reverse) {
                SkPath reversed;
                reversed.reverseAddPath(path);
                path = reversed;
            }

            SkContourMeasureIter iter(path, /*forceClosed = */false);
            fCurrentMeasure  = iter.next();
            fCurrentClosed   = path.isLastContourClosed();
            fCurrentReversed = reverse;
            fCurrentPath     = fPath;

            // AE paths are always single-contour (no moves allowed).
            SkASSERT(!iter.next());
        }
    }

    float pathLength() const {
        SkASSERT(fPath == fCurrentPath);
        SkASSERT((fPathReverse != 0) == fCurrentReversed);

        return fCurrentMeasure ? fCurrentMeasure->length() : 0;
    }

    SkM44 getMatrix(float distance, SkTextUtils::Align alignment) const {
        SkASSERT(fPath == fCurrentPath);
        SkASSERT((fPathReverse != 0) == fCurrentReversed);

        if (!fCurrentMeasure) {
            return SkM44();
        }

        const auto path_len = fCurrentMeasure->length();

        // First/last margin adjustment also depends on alignment.
        switch (alignment) {
            case SkTextUtils::Align::kLeft_Align:   distance += fPathFMargin; break;
            case SkTextUtils::Align::kCenter_Align: distance += fPathFMargin +
                                                                fPathLMargin; break;
            case SkTextUtils::Align::kRight_Align:  distance += fPathLMargin; break;
        }

        // For closed paths, extranormal distances wrap around the contour.
        if (fCurrentClosed) {
            distance = std::fmod(distance, path_len);
            if (distance < 0) {
                distance += path_len;
            }
            SkASSERT(0 <= distance && distance <= path_len);
        }

        SkPoint pos;
        SkVector tan;
        if (!fCurrentMeasure->getPosTan(distance, &pos, &tan)) {
            return SkM44();
        }

        // For open paths, extranormal distances are extrapolated from extremes.
        // Note:
        //   - getPosTan above clamps to the extremes
        //   - the extrapolation below only kicks in for extranormal values
        const auto underflow = std::min(0.0f, distance),
                   overflow  = std::max(0.0f, distance - path_len);
        pos += tan*(underflow + overflow);

        auto m = SkM44::Translate(pos.x(), pos.y());

        // The "perpendicular" flag controls whether fragments are positioned and rotated,
        // or just positioned.
        if (fPathPerpendicular != 0) {
            m = m * SkM44::Rotate({0,0,1}, std::atan2(tan.y(), tan.x()));
        }

        return m;
    }

private:
    // Cached contour data.
    ShapeValue              fCurrentPath;
    sk_sp<SkContourMeasure> fCurrentMeasure;
    bool                    fCurrentReversed = false,
                            fCurrentClosed   = false;
};

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
    //    "m": { // more options
    //           "g": 1,     // Anchor Point Grouping
    //           "a": {...}  // Grouping Alignment
    //         },
    //    "p": { // path options
    //           "a": 0,   // force alignment
    //           "f": {},  // first margin
    //           "l": {},  // last margin
    //           "m": 1,   // mask index
    //           "p": 1,   // perpendicular
    //           "r": 0    // reverse path
    //         }

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

    // Optional text path
    const auto attach_path = [&](const skjson::ObjectValue* jpath) -> std::unique_ptr<PathInfo> {
        if (!jpath) {
            return nullptr;
        }

        // the actual path is identified as an index in the layer mask stack
        const auto mask_index =
                ParseDefault<size_t>((*jpath)["m"], std::numeric_limits<size_t>::max());
        const skjson::ArrayValue* jmasks = jlayer["masksProperties"];
        if (!jmasks || mask_index >= jmasks->size()) {
            return nullptr;
        }

        const skjson::ObjectValue* mask = (*jmasks)[mask_index];
        if (!mask) {
            return nullptr;
        }

        auto pinfo = std::make_unique<PathInfo>();
        adapter->bind(*abuilder, (*mask)["pt"], &pinfo->fPath);
        adapter->bind(*abuilder, (*jpath)["f"], &pinfo->fPathFMargin);
        adapter->bind(*abuilder, (*jpath)["l"], &pinfo->fPathLMargin);
        adapter->bind(*abuilder, (*jpath)["p"], &pinfo->fPathPerpendicular);
        adapter->bind(*abuilder, (*jpath)["r"], &pinfo->fPathReverse);

        // TODO: force align support

        // Historically, these used to be exported as static properties.
        // Attempt parsing both ways, for backward compat.
        skottie::Parse((*jpath)["p"], &pinfo->fPathPerpendicular);
        skottie::Parse((*jpath)["r"], &pinfo->fPathReverse);

        // Path positioning requires anchor point info.
        adapter->fRequiresAnchorPoint = true;

        return pinfo;
    };

    adapter->fPathInfo = attach_path((*jt)["p"]);

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

    if (SHOW_LAYOUT_BOXES) {
        // visualize fragment ascent boxes
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

    // We need granular fragments (as opposed to consolidated blobs) when animating, or when
    // positioning on a path.
    if (!fAnimators.empty()  || fPathInfo) flags |= Shaper::Flags::kFragmentGlyphs;
    if (fRequiresAnchorPoint)              flags |= Shaper::Flags::kTrackFragmentAdvanceAscent;

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

    if (!fAnimators.empty() || fPathInfo) {
        // Range selectors and text paths require fragment domain maps.
        this->buildDomainMaps(shape_result);
    }

    if (SHOW_LAYOUT_BOXES) {
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

        if (fPathInfo) {
            auto path_color = sksg::Color::Make(0xffffff00);
            path_color->setStyle(SkPaint::kStroke_Style);
            path_color->setStrokeWidth(1);
            path_color->setAntiAlias(true);

            fRoot->addChild(
                        sksg::Draw::Make(sksg::Path::Make(static_cast<SkPath>(fPathInfo->fPath)),
                                         std::move(path_color)));
        }
    }
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

    // Update the path contour measure, if needed.
    if (fPathInfo) {
        fPathInfo->updateContourData();
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

SkM44 TextAdapter::fragmentMatrix(const TextAnimator::ResolvedProps& props,
                                  const FragmentRec& rec, const SkV2& anchor_point) const {
    const SkV3 pos = {
        props.position.x + rec.fOrigin.fX + anchor_point.x,
        props.position.y + rec.fOrigin.fY + anchor_point.y,
        props.position.z
    };

    if (!fPathInfo) {
        return SkM44::Translate(pos.x, pos.y, pos.z);
    }

    // "Align" the paragraph box left/center/right to path start/mid/end, respectively.
    const auto align_offset =
            align_factor(fText->fHAlign)*(fPathInfo->pathLength() - fText->fBox.width());

    // Path positioning is based on the fragment position relative to the paragraph box
    // upper-left corner:
    //
    //   - the horizontal component determines the distance on path
    //
    //   - the vertical component is post-applied after orienting on path
    //
    // Note: in point-text mode, the box adjustments have no effect as fBox is {0,0,0,0}.
    //
    const auto rel_pos = SkV2{pos.x, pos.y} - SkV2{fText->fBox.fLeft, fText->fBox.fTop};
    const auto path_distance = rel_pos.x + align_offset;

    return fPathInfo->getMatrix(path_distance, fText->fHAlign)
         * SkM44::Translate(0, rel_pos.y, pos.z);
}

void TextAdapter::pushPropsToFragment(const TextAnimator::ResolvedProps& props,
                                      const FragmentRec& rec,
                                      const SkV2& grouping_alignment,
                                      const TextAnimator::DomainSpan* grouping_span) const {
    const auto anchor_point = this->fragmentAnchorPoint(rec, grouping_alignment, grouping_span);

    rec.fMatrixNode->setMatrix(
                this->fragmentMatrix(props, rec, anchor_point)
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

    const auto align_offset = -total_tracking * align_factor(fText->fHAlign);

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

} // namespace skottie::internal
