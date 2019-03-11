/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkottieAdapter.h"

#include "SkFont.h"
#include "SkMatrix.h"
#include "SkMatrix44.h"
#include "SkPath.h"
#include "SkRRect.h"
#include "SkSGColor.h"
#include "SkSGDraw.h"
#include "SkSGGradient.h"
#include "SkSGGroup.h"
#include "SkSGPath.h"
#include "SkSGRect.h"
#include "SkSGRenderEffect.h"
#include "SkSGText.h"
#include "SkSGTransform.h"
#include "SkSGTrimEffect.h"
#include "SkShaper.h"
#include "SkTextBlob.h"
#include "SkTextUtils.h"
#include "SkTo.h"
#include "SkUTF.h"
#include "SkottieValue.h"

#include <cmath>
#include <utility>

namespace skottie {

RRectAdapter::RRectAdapter(sk_sp<sksg::RRect> wrapped_node)
    : fRRectNode(std::move(wrapped_node)) {}

RRectAdapter::~RRectAdapter() = default;

void RRectAdapter::apply() {
    // BM "position" == "center position"
    auto rr = SkRRect::MakeRectXY(SkRect::MakeXYWH(fPosition.x() - fSize.width() / 2,
                                                   fPosition.y() - fSize.height() / 2,
                                                   fSize.width(), fSize.height()),
                                  fRadius.width(),
                                  fRadius.height());
   fRRectNode->setRRect(rr);
}

TransformAdapter2D::TransformAdapter2D(sk_sp<sksg::Matrix<SkMatrix>> matrix)
    : fMatrixNode(std::move(matrix)) {}

TransformAdapter2D::~TransformAdapter2D() = default;

SkMatrix TransformAdapter2D::totalMatrix() const {
    SkMatrix t = SkMatrix::MakeTrans(-fAnchorPoint.x(), -fAnchorPoint.y());

    t.postScale(fScale.x() / 100, fScale.y() / 100); // 100% based
    t.postRotate(fRotation);
    t.postTranslate(fPosition.x(), fPosition.y());
    // TODO: skew

    return t;
}

void TransformAdapter2D::apply() {
    fMatrixNode->setMatrix(this->totalMatrix());
}

TransformAdapter3D::Vec3::Vec3(const VectorValue& v) {
    fX = v.size() > 0 ? v[0] : 0;
    fY = v.size() > 1 ? v[1] : 0;
    fZ = v.size() > 2 ? v[2] : 0;
}

TransformAdapter3D::TransformAdapter3D(sk_sp<sksg::Matrix<SkMatrix44>> matrix)
    : fMatrixNode(std::move(matrix)) {}

TransformAdapter3D::~TransformAdapter3D() = default;

SkMatrix44 TransformAdapter3D::totalMatrix() const {
    SkMatrix44 t;

    t.setTranslate(-fAnchorPoint.fX, -fAnchorPoint.fY, -fAnchorPoint.fZ);
    t.postScale(fScale.fX / 100, fScale.fY / 100, fScale.fZ / 100);

    // TODO: SkMatrix44:postRotate()?
    SkMatrix44 r;
    r.setRotateDegreesAbout(1, 0, 0, fRotation.fX);
    t.postConcat(r);
    r.setRotateDegreesAbout(0, 1, 0, fRotation.fY);
    t.postConcat(r);
    r.setRotateDegreesAbout(0, 0, 1, fRotation.fZ);
    t.postConcat(r);

    t.postTranslate(fPosition.fX, fPosition.fY, fPosition.fZ);

    return t;
}

void TransformAdapter3D::apply() {
    fMatrixNode->setMatrix(this->totalMatrix());
}

RepeaterAdapter::RepeaterAdapter(sk_sp<sksg::RenderNode> repeater_node, Composite composite)
    : fRepeaterNode(repeater_node)
    , fComposite(composite)
    , fRoot(sksg::Group::Make()) {}

RepeaterAdapter::~RepeaterAdapter() = default;

void RepeaterAdapter::apply() {
    static constexpr SkScalar kMaxCount = 512;
    const auto count = static_cast<size_t>(SkTPin(fCount, 0.0f, kMaxCount) + 0.5f);

    const auto& compute_transform = [this] (size_t index) {
        const auto t = fOffset + index;

        // Position, scale & rotation are "scaled" by index/offset.
        SkMatrix m = SkMatrix::MakeTrans(-fAnchorPoint.x(),
                                         -fAnchorPoint.y());
        m.postScale(std::pow(fScale.x() * .01f, fOffset),
                    std::pow(fScale.y() * .01f, fOffset));
        m.postRotate(t * fRotation);
        m.postTranslate(t * fPosition.x() + fAnchorPoint.x(),
                        t * fPosition.y() + fAnchorPoint.y());

        return m;
    };

    // TODO: start/end opacity support.

    // TODO: we can avoid rebuilding all the fragments in most cases.
    fRoot->clear();
    for (size_t i = 0; i < count; ++i) {
        const auto insert_index = (fComposite == Composite::kAbove) ? i : count - i - 1;
        fRoot->addChild(sksg::TransformEffect::Make(fRepeaterNode,
                                                    compute_transform(insert_index)));
    }
}

PolyStarAdapter::PolyStarAdapter(sk_sp<sksg::Path> wrapped_node, Type t)
    : fPathNode(std::move(wrapped_node))
    , fType(t) {}

PolyStarAdapter::~PolyStarAdapter() = default;

void PolyStarAdapter::apply() {
    static constexpr int kMaxPointCount = 100000;
    const auto count = SkToUInt(SkTPin(SkScalarRoundToInt(fPointCount), 0, kMaxPointCount));
    const auto arc   = sk_ieee_float_divide(SK_ScalarPI * 2, count);

    const auto pt_on_circle = [](const SkPoint& c, SkScalar r, SkScalar a) {
        return SkPoint::Make(c.x() + r * std::cos(a),
                             c.y() + r * std::sin(a));
    };

    // TODO: inner/outer "roundness"?

    SkPath poly;

    auto angle = SkDegreesToRadians(fRotation - 90);
    poly.moveTo(pt_on_circle(fPosition, fOuterRadius, angle));
    poly.incReserve(fType == Type::kStar ? count * 2 : count);

    for (unsigned i = 0; i < count; ++i) {
        if (fType == Type::kStar) {
            poly.lineTo(pt_on_circle(fPosition, fInnerRadius, angle + arc * 0.5f));
        }
        angle += arc;
        poly.lineTo(pt_on_circle(fPosition, fOuterRadius, angle));
    }

    poly.close();
    fPathNode->setPath(poly);
}

GradientAdapter::GradientAdapter(sk_sp<sksg::Gradient> grad, size_t stopCount)
    : fGradient(std::move(grad))
    , fStopCount(stopCount) {}

void GradientAdapter::apply() {
    this->onApply();

    // |fColorStops| holds |fStopCount| x [ pos, r, g, g ] + ? x [ pos, alpha ]

    if (fColorStops.size() < fStopCount * 4 || ((fColorStops.size() - fStopCount * 4) % 2)) {
        // apply() may get called before the stops are set, so only log when we have some stops.
        if (!fColorStops.empty()) {
            SkDebugf("!! Invalid gradient stop array size: %zu\n", fColorStops.size());
        }
        return;
    }

    std::vector<sksg::Gradient::ColorStop> stops;

    // TODO: merge/lerp opacity stops
    const auto csEnd = fColorStops.cbegin() + fStopCount * 4;
    for (auto cs = fColorStops.cbegin(); cs != csEnd; cs += 4) {
        const auto pos = cs[0];
        const VectorValue rgb({ cs[1], cs[2], cs[3] });

        stops.push_back({ pos, ValueTraits<VectorValue>::As<SkColor>(rgb) });
    }

    fGradient->setColorStops(std::move(stops));
}

LinearGradientAdapter::LinearGradientAdapter(sk_sp<sksg::LinearGradient> grad, size_t stopCount)
    : INHERITED(std::move(grad), stopCount) {}

void LinearGradientAdapter::onApply() {
    auto* grad = static_cast<sksg::LinearGradient*>(fGradient.get());
    grad->setStartPoint(this->startPoint());
    grad->setEndPoint(this->endPoint());
}

RadialGradientAdapter::RadialGradientAdapter(sk_sp<sksg::RadialGradient> grad, size_t stopCount)
    : INHERITED(std::move(grad), stopCount) {}

void RadialGradientAdapter::onApply() {
    auto* grad = static_cast<sksg::RadialGradient*>(fGradient.get());
    grad->setStartCenter(this->startPoint());
    grad->setEndCenter(this->startPoint());
    grad->setStartRadius(0);
    grad->setEndRadius(SkPoint::Distance(this->startPoint(), this->endPoint()));
}

TrimEffectAdapter::TrimEffectAdapter(sk_sp<sksg::TrimEffect> trimEffect)
    : fTrimEffect(std::move(trimEffect)) {
    SkASSERT(fTrimEffect);
}

TrimEffectAdapter::~TrimEffectAdapter() = default;

void TrimEffectAdapter::apply() {
    // BM semantics: start/end are percentages, offset is "degrees" (?!).
    const auto  start = fStart  / 100,
                  end = fEnd    / 100,
               offset = fOffset / 360;

    auto startT = SkTMin(start, end) + offset,
          stopT = SkTMax(start, end) + offset;
    auto   mode = SkTrimPathEffect::Mode::kNormal;

    if (stopT - startT < 1) {
        startT -= SkScalarFloorToScalar(startT);
        stopT  -= SkScalarFloorToScalar(stopT);

        if (startT > stopT) {
            using std::swap;
            swap(startT, stopT);
            mode = SkTrimPathEffect::Mode::kInverted;
        }
    } else {
        startT = 0;
        stopT  = 1;
    }

    fTrimEffect->setStart(startT);
    fTrimEffect->setStop(stopT);
    fTrimEffect->setMode(mode);
}

DropShadowEffectAdapter::DropShadowEffectAdapter(sk_sp<sksg::DropShadowImageFilter> dropShadow)
    : fDropShadow(std::move(dropShadow)) {
    SkASSERT(fDropShadow);
}

DropShadowEffectAdapter::~DropShadowEffectAdapter() = default;

void DropShadowEffectAdapter::apply() {
    // fColor -> RGB, fOpacity -> A
    fDropShadow->setColor(SkColorSetA(fColor, SkTPin(SkScalarRoundToInt(fOpacity), 0, 255)));

    // The offset is specified in terms of a bearing angle + distance.
    SkScalar sinV, cosV;
    sinV = SkScalarSinCos(SkDegreesToRadians(90 - fDirection), &cosV);
    fDropShadow->setOffset(SkVector::Make(fDistance * cosV, -fDistance * sinV));

    // Close enough to AE.
    static constexpr SkScalar kSoftnessToSigmaFactor = 0.3f;
    const auto sigma = fSoftness * kSoftnessToSigmaFactor;
    fDropShadow->setSigma(SkVector::Make(sigma, sigma));

    fDropShadow->setMode(fShadowOnly ? sksg::DropShadowImageFilter::Mode::kShadowOnly
                                     : sksg::DropShadowImageFilter::Mode::kShadowAndForeground);
}

GaussianBlurEffectAdapter::GaussianBlurEffectAdapter(sk_sp<sksg::BlurImageFilter> blur)
    : fBlur(std::move(blur)) {
    SkASSERT(fBlur);
}

GaussianBlurEffectAdapter::~GaussianBlurEffectAdapter() = default;

void GaussianBlurEffectAdapter::apply() {
    static constexpr SkVector kDimensionsMap[] = {
        { 1, 1 }, // 1 -> horizontal and vertical
        { 1, 0 }, // 2 -> horizontal
        { 0, 1 }, // 3 -> vertical
    };

    const auto dim_index = SkTPin<size_t>(static_cast<size_t>(fDimensions),
                                          1, SK_ARRAY_COUNT(kDimensionsMap)) - 1;

    // Close enough to AE.
    static constexpr SkScalar kBlurrinessToSigmaFactor = 0.3f;
    const auto sigma = fBlurriness * kBlurrinessToSigmaFactor;

    fBlur->setSigma({ sigma * kDimensionsMap[dim_index].x(),
                      sigma * kDimensionsMap[dim_index].y() });

    static constexpr SkBlurImageFilter::TileMode kRepeatEdgeMap[] = {
        SkBlurImageFilter::kClampToBlack_TileMode, // 0 -> repeat edge pixels: off
        SkBlurImageFilter::       kClamp_TileMode, // 1 -> repeat edge pixels: on
    };

    const auto repeat_index = SkTPin<size_t>(static_cast<size_t>(fRepeatEdge),
                                             0, SK_ARRAY_COUNT(kRepeatEdgeMap) - 1);
    fBlur->setTileMode(kRepeatEdgeMap[repeat_index]);
}

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

sk_sp<SkTextBlob> TextAdapter::makeBlob() const {
    SkFont font(fText.fTypeface, fText.fTextSize);
    font.setHinting(kNo_SkFontHinting);
    font.setSubpixel(true);
    font.setEdging(SkFont::Edging::kAntiAlias);

    // Helper for interfacing with SkShaper: buffers shaper-fed runs and performs
    // per-line position adjustments (for external line breaking, horizontal alignment, etc).
    class BlobMaker final : public SkShaper::RunHandler {
    public:
        BlobMaker(SkTextUtils::Align align)
            : fAlignFactor(AlignFactor(align)) {}

        Buffer newRunBuffer(const RunInfo& info, const SkFont& font, int glyphCount,
                            SkSpan<const char> utf8) override {
            fPendingLineAdvance += info.fAdvance;

            auto& run = fPendingLineRuns.emplace_back(font, info, glyphCount);

            return {
                run.fGlyphs   .data(),
                run.fPositions.data(),
                nullptr,
            };
        }

        void commitRun() override { }

        void commitLine() override {
            SkScalar line_spacing = 0;

            for (const auto& run : fPendingLineRuns) {
                const auto runSize = run.size();
                const auto& blobBuffer = fBuilder.allocRunPos(run.fFont, SkToInt(runSize));

                sk_careful_memcpy(blobBuffer.glyphs,
                                  run.fGlyphs.data(),
                                  runSize * sizeof(SkGlyphID));

                // For each buffered line, perform the following position adjustments:
                //   1) horizontal alignment
                //   2) vertical advance (based on line number/offset)
                //   3) baseline/ascent adjustment
                const auto offset = SkVector::Make(fAlignFactor * fPendingLineAdvance.x(),
                                                   fPendingLineVOffset + run.fInfo.fAscent);
                for (size_t i = 0; i < runSize; ++i) {
                    blobBuffer.points()[i] = run.fPositions[SkToInt(i)] + offset;
                }

                line_spacing = SkTMax(line_spacing,
                                      run.fInfo.fDescent - run.fInfo.fAscent + run.fInfo.fLeading);
            }

            fPendingLineRuns.reset();
            fPendingLineVOffset += line_spacing;
            fPendingLineAdvance  = { 0, 0 };
        }

        sk_sp<SkTextBlob> makeBlob() {
            return fBuilder.make();
        }

    private:
        static float AlignFactor(SkTextUtils::Align align) {
            switch (align) {
            case SkTextUtils::kLeft_Align:   return  0.0f;
            case SkTextUtils::kCenter_Align: return -0.5f;
            case SkTextUtils::kRight_Align:  return -1.0f;
            }
            return 0.0f; // go home, msvc...
        }

        struct Run {
            SkFont                          fFont;
            SkShaper::RunHandler::RunInfo   fInfo;
            SkSTArray<128, SkGlyphID, true> fGlyphs;
            SkSTArray<128, SkPoint  , true> fPositions;

            Run(const SkFont& font, const SkShaper::RunHandler::RunInfo& info, int count)
                : fFont(font)
                , fInfo(info)
                , fGlyphs   (count)
                , fPositions(count) {
                fGlyphs   .push_back_n(count);
                fPositions.push_back_n(count);
            }

            size_t size() const {
                SkASSERT(fGlyphs.size() == fPositions.size());
                return fGlyphs.size();
            }
        };

        const float fAlignFactor;

        SkTextBlobBuilder        fBuilder;
        SkSTArray<2, Run, false> fPendingLineRuns;
        SkScalar                 fPendingLineVOffset = 0;
        SkVector                 fPendingLineAdvance = { 0, 0 };
    };

    BlobMaker blobMaker(fText.fAlign);

    const auto& push_line = [&](const char* start, const char* end) {
        std::unique_ptr<SkShaper> shaper = SkShaper::Make();
        if (!shaper) {
            return;
        }

        shaper->shape(&blobMaker, font, start, SkToSizeT(end - start), true, { 0, 0 }, SK_ScalarMax);
    };

    const auto& is_line_break = [](SkUnichar uch) {
        // TODO: other explicit breaks?
        return uch == '\r';
    };

    const char* ptr        = fText.fText.c_str();
    const char* line_start = ptr;
    const char* end        = ptr + fText.fText.size();

    while (ptr < end) {
        if (is_line_break(SkUTF::NextUTF8(&ptr, end))) {
            push_line(line_start, ptr - 1);
            line_start = ptr;
        }
    }
    push_line(line_start, ptr);

    return blobMaker.makeBlob();
}

void TextAdapter::apply() {
    fTextNode->setBlob(this->makeBlob());
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
