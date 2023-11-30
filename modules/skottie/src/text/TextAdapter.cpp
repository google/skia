/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "modules/skottie/src/text/TextAdapter.h"

#include "include/core/SkCanvas.h"
#include "include/core/SkContourMeasure.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkM44.h"
#include "include/private/base/SkTPin.h"
#include "modules/skottie/src/SkottieJson.h"
#include "modules/skottie/src/text/RangeSelector.h"
#include "modules/skottie/src/text/TextAnimator.h"
#include "modules/sksg/include/SkSGDraw.h"
#include "modules/sksg/include/SkSGGeometryNode.h"
#include "modules/sksg/include/SkSGGroup.h"
#include "modules/sksg/include/SkSGPaint.h"
#include "modules/sksg/include/SkSGPath.h"
#include "modules/sksg/include/SkSGRect.h"
#include "modules/sksg/include/SkSGRenderEffect.h"
#include "modules/sksg/include/SkSGRenderNode.h"
#include "modules/sksg/include/SkSGTransform.h"
#include "modules/sksg/src/SkSGTransformPriv.h"

// Enable for text layout debugging.
#define SHOW_LAYOUT_BOXES 0

namespace skottie::internal {

namespace {

class GlyphTextNode final : public sksg::GeometryNode {
public:
    explicit GlyphTextNode(Shaper::ShapedGlyphs&& glyphs) : fGlyphs(std::move(glyphs)) {}

    ~GlyphTextNode() override = default;

    const Shaper::ShapedGlyphs* glyphs() const { return &fGlyphs; }

protected:
    SkRect onRevalidate(sksg::InvalidationController*, const SkMatrix&) override {
        return fGlyphs.computeBounds(Shaper::ShapedGlyphs::BoundsType::kConservative);
    }

    void onDraw(SkCanvas* canvas, const SkPaint& paint) const override {
        fGlyphs.draw(canvas, {0,0}, paint);
    }

    void onClip(SkCanvas* canvas, bool antiAlias) const override {
        canvas->clipPath(this->asPath(), antiAlias);
    }

    bool onContains(const SkPoint& p) const override {
        return this->asPath().contains(p.x(), p.y());
    }

    SkPath onAsPath() const override {
        // TODO
        return SkPath();
    }

private:
    const Shaper::ShapedGlyphs fGlyphs;
};

static float align_factor(SkTextUtils::Align a) {
    switch (a) {
        case SkTextUtils::kLeft_Align  : return 0.0f;
        case SkTextUtils::kCenter_Align: return 0.5f;
        case SkTextUtils::kRight_Align : return 1.0f;
    }

    SkUNREACHABLE;
}

} // namespace

class TextAdapter::GlyphDecoratorNode final : public sksg::Group {
public:
    GlyphDecoratorNode(sk_sp<GlyphDecorator> decorator, float scale)
        : fDecorator(std::move(decorator))
        , fScale(scale)
    {}

    ~GlyphDecoratorNode() override = default;

    void updateFragmentData(const std::vector<TextAdapter::FragmentRec>& recs) {
        fFragCount = recs.size();

        SkASSERT(!fFragInfo);
        fFragInfo = std::make_unique<FragmentInfo[]>(recs.size());

        for (size_t i = 0; i < recs.size(); ++i) {
            const auto& rec = recs[i];
            fFragInfo[i] = {rec.fGlyphs, rec.fMatrixNode, rec.fAdvance};
        }

        SkASSERT(!fDecoratorInfo);
        fDecoratorInfo = std::make_unique<GlyphDecorator::GlyphInfo[]>(recs.size());
    }

    SkRect onRevalidate(sksg::InvalidationController* ic, const SkMatrix& ctm) override {
        const auto child_bounds = INHERITED::onRevalidate(ic, ctm);

        for (size_t i = 0; i < fFragCount; ++i) {
            const auto* glyphs = fFragInfo[i].fGlyphs;
            fDecoratorInfo[i].fBounds =
                    glyphs->computeBounds(Shaper::ShapedGlyphs::BoundsType::kTight);
            fDecoratorInfo[i].fMatrix = sksg::TransformPriv::As<SkMatrix>(fFragInfo[i].fMatrixNode);

            fDecoratorInfo[i].fCluster = glyphs->fClusters.empty() ? 0 : glyphs->fClusters.front();
            fDecoratorInfo[i].fAdvance = fFragInfo[i].fAdvance;
        }

        return child_bounds;
    }

    void onRender(SkCanvas* canvas, const RenderContext* ctx) const override {
        auto local_ctx = ScopedRenderContext(canvas, ctx).setIsolation(this->bounds(),
                                                                       canvas->getTotalMatrix(),
                                                                       true);
        this->INHERITED::onRender(canvas, local_ctx);

        fDecorator->onDecorate(canvas, {
            SkSpan(fDecoratorInfo.get(), fFragCount),
            fScale
        });
    }

private:
    struct FragmentInfo {
        const Shaper::ShapedGlyphs* fGlyphs;
        sk_sp<sksg::Matrix<SkM44>>  fMatrixNode;
        float                       fAdvance;
    };

    const sk_sp<GlyphDecorator>                  fDecorator;
    const float                                  fScale;

    std::unique_ptr<FragmentInfo[]>              fFragInfo;
    std::unique_ptr<GlyphDecorator::GlyphInfo[]> fDecoratorInfo;
    size_t                                       fFragCount;

    using INHERITED = Group;
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
                                     sk_sp<SkFontMgr> fontmgr,
                                     sk_sp<CustomFont::GlyphCompMapper> custom_glyph_mapper,
                                     sk_sp<Logger> logger) {
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
    //        ],
    //        "sid": "optionalSlotID"
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
            ? SkTPin<int>(ParseDefault<int>((*jm)["g"], 1), 1, std::size(gGroupingMap))
            : 1;

    auto adapter = sk_sp<TextAdapter>(new TextAdapter(std::move(fontmgr),
                                                      std::move(custom_glyph_mapper),
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
                adapter->fHasBlurAnimator         |= animator->hasBlur();
                adapter->fRequiresAnchorPoint     |= animator->requiresAnchorPoint();
                adapter->fRequiresLineAdjustments |= animator->requiresLineAdjustments();

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
    abuilder->dispatchTextProperty(adapter, jd);

    return adapter;
}

TextAdapter::TextAdapter(sk_sp<SkFontMgr> fontmgr,
                         sk_sp<CustomFont::GlyphCompMapper> custom_glyph_mapper,
                         sk_sp<Logger> logger,
                         AnchorPointGrouping apg)
    : fRoot(sksg::Group::Make())
    , fFontMgr(std::move(fontmgr))
    , fCustomGlyphMapper(std::move(custom_glyph_mapper))
    , fLogger(std::move(logger))
    , fAnchorPointGrouping(apg)
    , fHasBlurAnimator(false)
    , fRequiresAnchorPoint(false)
    , fRequiresLineAdjustments(false) {}

TextAdapter::~TextAdapter() = default;

std::vector<sk_sp<sksg::RenderNode>>
TextAdapter::buildGlyphCompNodes(Shaper::ShapedGlyphs& glyphs) const {
    std::vector<sk_sp<sksg::RenderNode>> draws;

    if (fCustomGlyphMapper) {
        size_t run_offset = 0;
        for (auto& run : glyphs.fRuns) {
            for (size_t i = 0; i < run.fSize; ++i) {
                const size_t goffset = run_offset + i;
                const SkGlyphID  gid = glyphs.fGlyphIDs[goffset];

                if (auto gcomp = fCustomGlyphMapper->getGlyphComp(run.fFont.getTypeface(), gid)) {
                    // Position and scale the "glyph".
                    const auto m = SkMatrix::Translate(glyphs.fGlyphPos[goffset])
                                 * SkMatrix::Scale(fText->fTextSize*fTextShapingScale,
                                                   fText->fTextSize*fTextShapingScale);

                    draws.push_back(sksg::TransformEffect::Make(std::move(gcomp), m));

                    // Remove all related data from the fragment, so we don't attempt to render
                    // this as a regular glyph.
                    SkASSERT(glyphs.fGlyphIDs.size() > goffset);
                    glyphs.fGlyphIDs.erase(glyphs.fGlyphIDs.begin() + goffset);
                    SkASSERT(glyphs.fGlyphPos.size() > goffset);
                    glyphs.fGlyphPos.erase(glyphs.fGlyphPos.begin() + goffset);
                    if (!glyphs.fClusters.empty()) {
                        SkASSERT(glyphs.fClusters.size() > goffset);
                        glyphs.fClusters.erase(glyphs.fClusters.begin() + goffset);
                    }
                    i         -= 1;
                    run.fSize -= 1;
                }
            }
            run_offset += run.fSize;
        }
    }

    return draws;
}

void TextAdapter::addFragment(Shaper::Fragment& frag, sksg::Group* container) {
    // For a given shaped fragment, build a corresponding SG fragment:
    //
    //   [TransformEffect] -> [Transform]
    //     [Group]
    //       [Draw] -> [GlyphTextNode*] [FillPaint]    // SkTypeface-based glyph.
    //       [Draw] -> [GlyphTextNode*] [StrokePaint]  // SkTypeface-based glyph.
    //       [CompRenderTree]                          // Comp glyph.
    //       ...
    //

    FragmentRec rec;
    rec.fOrigin     = frag.fOrigin;
    rec.fAdvance    = frag.fAdvance;
    rec.fAscent     = frag.fAscent;
    rec.fMatrixNode = sksg::Matrix<SkM44>::Make(SkM44::Translate(frag.fOrigin.x(),
                                                                 frag.fOrigin.y()));

    // Start off substituting existing comp nodes for all composition-based glyphs.
    std::vector<sk_sp<sksg::RenderNode>> draws = this->buildGlyphCompNodes(frag.fGlyphs);

    // Use a regular GlyphTextNode for the remaining glyphs (backed by a real SkTypeface).
    auto text_node = sk_make_sp<GlyphTextNode>(std::move(frag.fGlyphs));
    rec.fGlyphs = text_node->glyphs();

    draws.reserve(draws.size() +
                  static_cast<size_t>(fText->fHasFill) +
                  static_cast<size_t>(fText->fHasStroke));

    SkASSERT(fText->fHasFill || fText->fHasStroke);

    auto add_fill = [&]() {
        if (fText->fHasFill) {
            rec.fFillColorNode = sksg::Color::Make(fText->fFillColor);
            rec.fFillColorNode->setAntiAlias(true);
            draws.push_back(sksg::Draw::Make(text_node, rec.fFillColorNode));
        }
    };
    auto add_stroke = [&] {
        if (fText->fHasStroke) {
            rec.fStrokeColorNode = sksg::Color::Make(fText->fStrokeColor);
            rec.fStrokeColorNode->setAntiAlias(true);
            rec.fStrokeColorNode->setStyle(SkPaint::kStroke_Style);
            rec.fStrokeColorNode->setStrokeWidth(fText->fStrokeWidth * fTextShapingScale);
            rec.fStrokeColorNode->setStrokeJoin(fText->fStrokeJoin);
            draws.push_back(sksg::Draw::Make(text_node, rec.fStrokeColorNode));
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

    draws.shrink_to_fit();

    auto draws_node = (draws.size() > 1)
            ? sksg::Group::Make(std::move(draws))
            : std::move(draws[0]);

    if (fHasBlurAnimator) {
        // Optional blur effect.
        rec.fBlur = sksg::BlurImageFilter::Make();
        draws_node = sksg::ImageFilterEffect::Make(std::move(draws_node), rec.fBlur);
    }

    container->addChild(sksg::TransformEffect::Make(std::move(draws_node), rec.fMatrixNode));
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

    // We need granular fragments (as opposed to consolidated blobs):
    //   - when animating
    //   - when positioning on a path
    //   - when clamping the number or lines (for accurate line count)
    //   - when a text decorator is present
    if (!fAnimators.empty() || fPathInfo || fText->fMaxLines || fText->fDecorator) {
        flags |= Shaper::Flags::kFragmentGlyphs;
    }

    if (fRequiresAnchorPoint || fText->fDecorator) {
        flags |= Shaper::Flags::kTrackFragmentAdvanceAscent;
    }

    if (fText->fDecorator) {
        flags |= Shaper::Flags::kClusters;
    }

    return flags;
}

void TextAdapter::reshape() {
    // AE clamps the font size to a reasonable range.
    // We do the same, since HB is susceptible to int overflows for degenerate values.
    static constexpr float kMinSize =    0.1f,
                           kMaxSize = 1296.0f;
    const Shaper::TextDesc text_desc = {
        fText->fTypeface,
        SkTPin(fText->fTextSize,    kMinSize, kMaxSize),
        SkTPin(fText->fMinTextSize, kMinSize, kMaxSize),
        SkTPin(fText->fMaxTextSize, kMinSize, kMaxSize),
        fText->fLineHeight,
        fText->fLineShift,
        fText->fAscent,
        fText->fHAlign,
        fText->fVAlign,
        fText->fResize,
        fText->fLineBreak,
        fText->fDirection,
        fText->fCapitalization,
        fText->fMaxLines,
        this->shaperFlags(),
        fText->fLocale.isEmpty()     ? nullptr : fText->fLocale.c_str(),
        fText->fFontFamily.isEmpty() ? nullptr : fText->fFontFamily.c_str(),
    };
    auto shape_result = Shaper::Shape(fText->fText, text_desc, fText->fBox, fFontMgr);

    if (fLogger) {
        if (shape_result.fFragments.empty() && fText->fText.size() > 0) {
            const auto msg = SkStringPrintf("Text layout failed for '%s'.",
                                            fText->fText.c_str());
            fLogger->log(Logger::Level::kError, msg.c_str());

            // These may trigger repeatedly when the text is animating.
            // To avoid spamming, only log once.
            fLogger = nullptr;
        }

        if (shape_result.fMissingGlyphCount > 0) {
            const auto msg = SkStringPrintf("Missing %zu glyphs for '%s'.",
                                            shape_result.fMissingGlyphCount,
                                            fText->fText.c_str());
            fLogger->log(Logger::Level::kWarning, msg.c_str());
            fLogger = nullptr;
        }
    }

    // Save the text shaping scale for later adjustments.
    fTextShapingScale = shape_result.fScale;

    // Rebuild all fragments.
    // TODO: we can be smarter here and try to reuse the existing SG structure if needed.

    fRoot->clear();
    fFragments.clear();

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

    // Depending on whether a GlyphDecorator is present, we either add the glyph render nodes
    // directly to the root group, or to an intermediate GlyphDecoratorNode container.
    sksg::Group* container = fRoot.get();
    sk_sp<GlyphDecoratorNode> decorator_node;
    if (fText->fDecorator) {
        decorator_node = sk_make_sp<GlyphDecoratorNode>(fText->fDecorator, fTextShapingScale);
        container = decorator_node.get();
    }

    // N.B. addFragment moves shaped glyph data out of the fragment, so only the fragment
    // metrics are valid after this block.
    for (size_t i = 0; i < shape_result.fFragments.size(); ++i) {
        this->addFragment(shape_result.fFragments[i], container);
    }

    if (decorator_node) {
        decorator_node->updateFragmentData(fFragments);
        fRoot->addChild(std::move(decorator_node));
    }

    if (!fAnimators.empty() || fPathInfo) {
        // Range selectors and text paths require fragment domain maps.
        this->buildDomainMaps(shape_result);
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
    seed_props.stroke_width = fText->fStrokeWidth;

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
    SkV2   current_line_offset = { 0, 0 }; // cumulative line spacing

    auto compute_linewide_props = [this](const TextAnimator::ModulatorBuffer& buf,
                                         const TextAnimator::DomainSpan& line_span) {
        SkV2  total_spacing  = {0,0};
        float total_tracking = 0;

        // Only compute these when needed.
        if (fRequiresLineAdjustments && line_span.fCount) {
            for (size_t i = line_span.fOffset; i < line_span.fOffset + line_span.fCount; ++i) {
                const auto& props = buf[i].props;
                total_spacing  += props.line_spacing;
                total_tracking += props.tracking;
            }

            // The first glyph does not contribute |before| tracking, and the last one does not
            // contribute |after| tracking.
            total_tracking -= 0.5f * (buf[line_span.fOffset].props.tracking +
                                      buf[line_span.fOffset + line_span.fCount - 1].props.tracking);
        }

        return std::make_tuple(total_spacing, total_tracking);
    };

    // Finally, push all props to their corresponding fragment.
    for (const auto& line_span : fMaps.fLinesMap) {
        const auto [line_spacing, line_tracking] = compute_linewide_props(buf, line_span);
        const auto align_offset = -line_tracking * align_factor(fText->fHAlign);

        // line spacing of the first line is ignored (nothing to "space" against)
        if (&line_span != &fMaps.fLinesMap.front() && line_span.fCount) {
            // For each line, the actual spacing is an average of individual fragment spacing
            // (to preserve the "line").
            current_line_offset += line_spacing / line_span.fCount;
        }

        float tracking_acc = 0;
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

            // AE tracking is defined per glyph, based on two components: |before| and |after|.
            // BodyMovin only exports "balanced" tracking values, where before = after = tracking/2.
            //
            // Tracking is applied as a local glyph offset, and contributes to the line width for
            // alignment purposes.
            //
            // No |before| tracking for the first glyph, nor |after| tracking for the last one.
            const auto track_before = i > line_span.fOffset
                                        ? props.tracking * 0.5f : 0.0f,
                       track_after  = i < line_span.fOffset + line_span.fCount - 1
                                        ? props.tracking * 0.5f : 0.0f;

            const auto frag_offset = current_line_offset +
                                     SkV2{align_offset + tracking_acc + track_before, 0};

            tracking_acc += track_before + track_after;

            this->pushPropsToFragment(props, frag, frag_offset, fGroupingAlignment * .01f, // %
                                      grouping_domain ? &(*grouping_domain)[grouping_span_index]
                                                        : nullptr);
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
                                  const FragmentRec& rec, const SkV2& frag_offset) const {
    const SkV3 pos = {
        props.position.x + rec.fOrigin.fX + frag_offset.x,
        props.position.y + rec.fOrigin.fY + frag_offset.y,
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
                                      const SkV2& frag_offset,
                                      const SkV2& grouping_alignment,
                                      const TextAnimator::DomainSpan* grouping_span) const {
    const auto anchor_point = this->fragmentAnchorPoint(rec, grouping_alignment, grouping_span);

    rec.fMatrixNode->setMatrix(
                this->fragmentMatrix(props, rec, anchor_point + frag_offset)
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
        rec.fStrokeColorNode->setStrokeWidth(props.stroke_width * fTextShapingScale);
    }
    if (rec.fBlur) {
        rec.fBlur->setSigma({ props.blur.x * kBlurSizeToSigma,
                              props.blur.y * kBlurSizeToSigma });
    }
}

} // namespace skottie::internal
