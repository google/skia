// Copyright 2019 Google LLC.
#include "TextLine.h"
#include <unicode/brkiter.h>
#include <unicode/ubidi.h>
#include "ParagraphImpl.h"

#include "include/core/SkMaskFilter.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkDiscretePathEffect.h"

namespace {

SkSpan<const char> intersected(const SkSpan<const char>& a, const SkSpan<const char>& b) {
    auto begin = SkTMax(a.begin(), b.begin());
    auto end = SkTMin(a.end(), b.end());
    return SkSpan<const char>(begin, end > begin ? end - begin : 0);
}

int32_t intersectedSize(SkSpan<const char> a, SkSpan<const char> b) {
    if (a.begin() == nullptr || b.begin() == nullptr) {
        return -1;
    }
    auto begin = SkTMax(a.begin(), b.begin());
    auto end = SkTMin(a.end(), b.end());
    return SkToS32(end - begin);
}
}  // namespace

namespace skia {
namespace textlayout {

SkTHashMap<SkFont, Run> TextLine::fEllipsisCache;

TextLine::TextLine(SkVector offset, SkVector advance, SkSpan<const TextBlock> blocks,
                   SkSpan<const char> text, SkSpan<const char> textWithSpaces,
                   SkSpan<const Cluster> clusters, size_t startPos, size_t endPos,
                   LineMetrics sizes)
        : fBlocks(blocks)
        , fText(text)
        , fTextWithSpaces(textWithSpaces)
        , fClusters(clusters)
        //, fStartPos(startPos)
        //, fEndPos(endPos)
        , fLogical()
        , fShift(0)
        , fAdvance(advance)
        , fOffset(offset)
        , fEllipsis(nullptr)
        , fSizes(sizes) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    // Reorder visual runs
    auto start = fClusters.begin();
    auto end = fClusters.end() - 1;
    size_t numRuns = end->run()->index() - start->run()->index() + 1;

    // Get the logical order
    std::vector<UBiDiLevel> runLevels;
    for (auto run = start->run(); run <= end->run(); ++run) {
        runLevels.emplace_back(run->fBidiLevel);
    }

    std::vector<int32_t> logicalOrder(numRuns);
    ubidi_reorderVisual(runLevels.data(), SkToU32(numRuns), logicalOrder.data());

    auto firstRun = start->run();
    for (auto index : logicalOrder) {
        fLogical.push_back(firstRun + index);
    }

    // TODO: use fStartPos and fEndPos really
    // SkASSERT(fStartPos <= start->run()->size());
    // SkASSERT(fEndPos <= end->run()->size());
}

TextLine::TextLine(TextLine&& other) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    this->fBlocks = other.fBlocks;
    this->fText = other.fText;
    this->fTextWithSpaces = other.fTextWithSpaces;
    this->fLogical.reset();
    this->fLogical = std::move(other.fLogical);
    this->fShift = other.fShift;
    this->fAdvance = other.fAdvance;
    this->fOffset = other.fOffset;
    this->fEllipsis = std::move(other.fEllipsis);
    this->fSizes = other.sizes();
    this->fClusters = other.fClusters;
}

void TextLine::paint(SkCanvas* textCanvas) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (this->empty()) {
        return;
    }

    textCanvas->save();
    textCanvas->translate(this->offset().fX, this->offset().fY);

    this->iterateThroughStylesInTextOrder(
            StyleType::kBackground,
            [textCanvas, this](SkSpan<const char> text, TextStyle style, SkScalar offsetX) {
                return this->paintBackground(textCanvas, text, style, offsetX);
            });

    this->iterateThroughStylesInTextOrder(
            StyleType::kShadow,
            [textCanvas, this](SkSpan<const char> text, TextStyle style, SkScalar offsetX) {
                return this->paintShadow(textCanvas, text, style, offsetX);
            });

    this->iterateThroughStylesInTextOrder(
            StyleType::kForeground,
            [textCanvas, this](SkSpan<const char> text, TextStyle style, SkScalar offsetX) {
                return this->paintText(textCanvas, text, style, offsetX);
            });

    this->iterateThroughStylesInTextOrder(
            StyleType::kDecorations,
            [textCanvas, this](SkSpan<const char> text, TextStyle style, SkScalar offsetX) {
                return this->paintDecorations(textCanvas, text, style, offsetX);
            });

    textCanvas->restore();
}

void TextLine::format(TextAlign effectiveAlign, SkScalar maxWidth, bool notLastLine) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    SkScalar delta = maxWidth - this->width();
    if (delta <= 0) {
        return;
    }

    if (effectiveAlign == TextAlign::kJustify && notLastLine) {
        this->justify(maxWidth);
    } else if (effectiveAlign == TextAlign::kRight) {
        this->shiftTo(delta);
    } else if (effectiveAlign == TextAlign::kCenter) {
        this->shiftTo(delta / 2);
    }
}

void TextLine::scanStyles(StyleType style, const StyleVisitor& visitor) {
    if (this->empty()) {
        return;
    }

    this->iterateThroughStylesInTextOrder(
            style, [this, visitor](SkSpan<const char> text, TextStyle style, SkScalar offsetX) {
                visitor(text, style, offsetX);
                return this->iterateThroughRuns(
                        text, offsetX, false,
                        [](Run*, int32_t, size_t, SkRect, SkScalar, bool) { return true; });
            });
}

void TextLine::scanRuns(const RunVisitor& visitor) {
    this->iterateThroughRuns(
            fText, 0, false,
            [visitor](Run* run, int32_t pos, size_t size, SkRect clip, SkScalar sc, bool b) {
                visitor(run, pos, size, clip, sc, b);
                return true;
            });
}

SkScalar TextLine::paintText(SkCanvas* canvas, SkSpan<const char> text, const TextStyle& style,
                           SkScalar offsetX) const {
    TRACE_EVENT0("skia", TRACE_FUNC);
    SkPaint paint;
    if (style.hasForeground()) {
        paint = style.getForeground();
    } else {
        paint.setColor(style.getColor());
    }

    auto shiftDown = this->baseline();
    return this->iterateThroughRuns(
            text, offsetX, false,
            [paint, canvas, shiftDown](Run* run, int32_t pos, size_t size, SkRect clip,
                                       SkScalar shift, bool clippingNeeded) {
                SkTextBlobBuilder builder;
                run->copyTo(builder, SkToU32(pos), size, SkVector::Make(0, shiftDown));
                canvas->save();
                if (clippingNeeded) {
                    canvas->clipRect(clip);
                }
                canvas->translate(shift, 0);
                canvas->drawTextBlob(builder.make(), 0, 0, paint);
                canvas->restore();
                return true;
            });
}

SkScalar TextLine::paintBackground(SkCanvas* canvas, SkSpan<const char> text,
                                 const TextStyle& style, SkScalar offsetX) const {
    TRACE_EVENT0("skia", TRACE_FUNC);
    return this->iterateThroughRuns(
            text, offsetX, false,
            [canvas, style](Run* run, int32_t pos, size_t size, SkRect clip, SkScalar shift,
                            bool clippingNeeded) {
                if (style.hasBackground()) {
                    canvas->drawRect(clip, style.getBackground());
                }
                return true;
            });
}

SkScalar TextLine::paintShadow(SkCanvas* canvas, SkSpan<const char> text, const TextStyle& style,
                             SkScalar offsetX) const {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (style.getShadowNumber() == 0) {
        // Still need to calculate text advance
        return iterateThroughRuns(
                text, offsetX, false,
                [](Run*, int32_t, size_t, SkRect, SkScalar, bool) { return true; });
    }

    SkScalar result = 0;
    for (TextShadow shadow : style.getShadows()) {
        if (!shadow.hasShadow()) continue;

        SkPaint paint;
        paint.setColor(shadow.fColor);
        if (shadow.fBlurRadius != 0.0) {
            auto filter = SkMaskFilter::MakeBlur(kNormal_SkBlurStyle,
                                                 SkDoubleToScalar(shadow.fBlurRadius), false);
            paint.setMaskFilter(filter);
        }

        auto shiftDown = this->baseline();
        result = this->iterateThroughRuns(
                text, offsetX, false,
                [canvas, shadow, paint, shiftDown](Run* run, size_t pos, size_t size, SkRect clip,
                                                   SkScalar shift, bool clippingNeeded) {
                    SkTextBlobBuilder builder;
                    run->copyTo(builder, pos, size, SkVector::Make(0, shiftDown));
                    canvas->save();
                    clip.offset(shadow.fOffset);
                    if (clippingNeeded) {
                        canvas->clipRect(clip);
                    }
                    canvas->translate(shift, 0);
                    canvas->drawTextBlob(builder.make(), shadow.fOffset.x(), shadow.fOffset.y(),
                                         paint);
                    canvas->restore();
                    return true;
                });
    }

    return result;
}

SkScalar TextLine::paintDecorations(SkCanvas* canvas, SkSpan<const char> text,
                                  const TextStyle& style, SkScalar offsetX) const {
    TRACE_EVENT0("skia", TRACE_FUNC);
    return this->iterateThroughRuns(
            text, offsetX, false,
            [this, canvas, style](Run* run, int32_t pos, size_t size, SkRect clip, SkScalar shift,
                                  bool clippingNeeded) {
                if (style.getDecoration() == TextDecoration::kNoDecoration) {
                    return true;
                }

                for (auto decoration : AllTextDecorations) {
                    if (style.getDecoration() && decoration == 0) {
                        continue;
                    }

                    SkScalar thickness = style.getDecorationThicknessMultiplier();
                    //
                    SkScalar position = 0;
                    switch (style.getDecoration()) {
                        case TextDecoration::kUnderline:
                            position = -run->ascent() + thickness;
                            break;
                        case TextDecoration::kOverline:
                            position = 0;
                            break;
                        case TextDecoration::kLineThrough: {
                            position = (run->descent() - run->ascent() - thickness) / 2;
                            break;
                        }
                        default:
                            // TODO: can we actually get here?
                            break;
                    }

                    auto width = clip.width();
                    SkScalar x = clip.left();
                    SkScalar y = clip.top() + position;

                    // Decoration paint (for now) and/or path
                    SkPaint paint;
                    SkPath path;
                    this->computeDecorationPaint(paint, clip, style, path);
                    paint.setStrokeWidth(thickness);

                    switch (style.getDecorationStyle()) {
                        case TextDecorationStyle::kWavy:
                            path.offset(x, y);
                            canvas->drawPath(path, paint);
                            break;
                        case TextDecorationStyle::kDouble: {
                            canvas->drawLine(x, y, x + width, y, paint);
                            SkScalar bottom = y + thickness * 2;
                            canvas->drawLine(x, bottom, x + width, bottom, paint);
                            break;
                        }
                        case TextDecorationStyle::kDashed:
                        case TextDecorationStyle::kDotted:
                        case TextDecorationStyle::kSolid:
                            canvas->drawLine(x, y, x + width, y, paint);
                            break;
                        default:
                            break;
                    }
                }

                return true;
            });
}

void TextLine::computeDecorationPaint(SkPaint& paint,
                                    SkRect clip,
                                    const TextStyle& style,
                                    SkPath& path) const {
    paint.setStyle(SkPaint::kStroke_Style);
    if (style.getDecorationColor() == SK_ColorTRANSPARENT) {
        paint.setColor(style.getColor());
    } else {
        paint.setColor(style.getDecorationColor());
    }

    SkScalar scaleFactor = style.getFontSize() / 14.f;

    switch (style.getDecorationStyle()) {
        case TextDecorationStyle::kSolid:
            break;

        case TextDecorationStyle::kDouble:
            break;

            // Note: the intervals are scaled by the thickness of the line, so it is
            // possible to change spacing by changing the decoration_thickness
            // property of TextStyle.
        case TextDecorationStyle::kDotted: {
            const SkScalar intervals[] = {1.0f * scaleFactor, 1.5f * scaleFactor,
                                          1.0f * scaleFactor, 1.5f * scaleFactor};
            size_t count = sizeof(intervals) / sizeof(intervals[0]);
            paint.setPathEffect(SkPathEffect::MakeCompose(
                    SkDashPathEffect::Make(intervals, (int32_t)count, 0.0f),
                    SkDiscretePathEffect::Make(0, 0)));
            break;
        }
            // Note: the intervals are scaled by the thickness of the line, so it is
            // possible to change spacing by changing the decoration_thickness
            // property of TextStyle.
        case TextDecorationStyle::kDashed: {
            const SkScalar intervals[] = {4.0f * scaleFactor, 2.0f * scaleFactor,
                                          4.0f * scaleFactor, 2.0f * scaleFactor};
            size_t count = sizeof(intervals) / sizeof(intervals[0]);
            paint.setPathEffect(SkPathEffect::MakeCompose(
                    SkDashPathEffect::Make(intervals, (int32_t)count, 0.0f),
                    SkDiscretePathEffect::Make(0, 0)));
            break;
        }
        case TextDecorationStyle::kWavy: {
            int wave_count = 0;
            SkScalar x_start = 0;
            SkScalar wavelength = scaleFactor * style.getDecorationThicknessMultiplier();
            auto width = clip.width();
            path.moveTo(0, 0);
            while (x_start + wavelength * 2 < width) {
                path.rQuadTo(wavelength,
                             wave_count % 2 != 0 ? wavelength : -wavelength,
                             wavelength * 2,
                             0);
                x_start += wavelength * 2;
                ++wave_count;
            }
            break;
        }
    }
}

void TextLine::justify(SkScalar maxWidth) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    // Count words and the extra spaces to spread across the line
    // TODO: do it at the line breaking?..
    size_t whitespacePatches = 0;
    SkScalar textLen = 0;
    bool whitespacePatch = false;
    this->iterateThroughClustersInGlyphsOrder(
            false, [&whitespacePatches, &textLen, &whitespacePatch](const Cluster* cluster) {
                if (cluster->isWhitespaces()) {
                    if (!whitespacePatch) {
                        whitespacePatch = true;
                        ++whitespacePatches;
                    }
                } else {
                    whitespacePatch = false;
                }
                textLen += cluster->width();
                return true;
            });

    if (whitespacePatches == 0) {
        this->fShift = 0;
        return;
    }

    SkScalar step = (maxWidth - textLen) / whitespacePatches;
    SkScalar shift = 0;

    // Spread the extra whitespaces
    whitespacePatch = false;
    this->iterateThroughClustersInGlyphsOrder(false, [&](const Cluster* cluster) {
        if (cluster->isWhitespaces()) {
            if (!whitespacePatch) {
                shift += step;
                whitespacePatch = true;
                --whitespacePatches;
            }
        } else {
            whitespacePatch = false;
        }
        cluster->shift(shift);
        return true;
    });

    SkAssertResult(SkScalarNearlyEqual(shift, maxWidth - textLen));
    SkASSERT(whitespacePatches == 0);
    this->fShift = 0;
    this->fAdvance.fX = maxWidth;
}

void TextLine::createEllipsis(SkScalar maxWidth, const SkString& ellipsis, bool) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    // Replace some clusters with the ellipsis
    // Go through the clusters in the reverse logical order
    // taking off cluster by cluster until the ellipsis fits
    SkScalar width = fAdvance.fX;
    iterateThroughClustersInGlyphsOrder(
            true, [this, &width, ellipsis, maxWidth](const Cluster* cluster) {
                if (cluster->isWhitespaces()) {
                    width -= cluster->width();
                    return true;
                }

                // Shape the ellipsis
                Run* cached = fEllipsisCache.find(cluster->run()->font());
                if (cached == nullptr) {
                    cached = shapeEllipsis(ellipsis, cluster->run());
                }
                fEllipsis = std::make_unique<Run>(*cached);

                // See if it fits
                if (width + fEllipsis->advance().fX > maxWidth) {
                    width -= cluster->width();
                    // Continue if it's not
                    return true;
                }

                fEllipsis->shift(width, 0);
                fAdvance.fX = width;
                return false;
            });
}

Run* TextLine::shapeEllipsis(const SkString& ellipsis, Run* run) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    class ShapeHandler final : public SkShaper::RunHandler {
    public:
        explicit ShapeHandler(SkScalar lineHeight) : fRun(nullptr), fLineHeight(lineHeight) {}
        Run* run() { return fRun; }

    private:
        void beginLine() override {}

        void runInfo(const RunInfo&) override {}

        void commitRunInfo() override {}

        Buffer runBuffer(const RunInfo& info) override {
            fRun = fEllipsisCache.set(info.fFont,
                                      Run(SkSpan<const char>(), info, fLineHeight, 0, 0));
            return fRun->newRunBuffer();
        }

        void commitRunBuffer(const RunInfo& info) override {
            fRun->fAdvance.fX = info.fAdvance.fX;
            fRun->fAdvance.fY = fRun->descent() - fRun->ascent();
        }

        void commitLine() override {}

        Run* fRun;
        SkScalar fLineHeight;
    };

    ShapeHandler handler(run->lineHeight());
    std::unique_ptr<SkShaper> shaper = SkShaper::MakeShapeDontWrapOrReorder();
    shaper->shape(ellipsis.c_str(), ellipsis.size(), run->font(), true,
                  std::numeric_limits<SkScalar>::max(), &handler);
    handler.run()->fText = SkSpan<const char>(ellipsis.c_str(), ellipsis.size());
    return handler.run();
}

SkRect TextLine::measureTextInsideOneRun(SkSpan<const char> text,
                                       Run* run,
                                       size_t& pos,
                                       size_t& size,
                                       bool& clippingNeeded) const {
    TRACE_EVENT0("skia", TRACE_FUNC);
    SkASSERT(intersectedSize(run->text(), text) >= 0);

    // Find [start:end] clusters for the text
    bool found;
    Cluster* start;
    Cluster* end;
    std::tie(found, start, end) = run->findLimitingClusters(text);
    if (!found) {
        SkASSERT(text.empty());
        return SkRect::MakeEmpty();
    }

    pos = start->startPos();
    size = end->endPos() - start->startPos();

    // Calculate the clipping rectangle for the text with cluster edges
    // There are 2 cases:
    // EOL (when we expect the last cluster clipped without any spaces)
    // Anything else (when we want the cluster width contain all the spaces -
    // coming from letter spacing or word spacing or justification)
    bool needsClipping = (run->leftToRight() ? end : start) == clusters().end() - 1;
    SkRect clip =
            SkRect::MakeXYWH(run->positionX(start->startPos()) - run->positionX(0),
                             sizes().runTop(run),
                             run->calculateWidth(start->startPos(), end->endPos(), needsClipping),
                             run->calculateHeight());

    // Correct the width in case the text edges don't match clusters
    // TODO: This is where we get smart about selecting a part of a cluster
    //  by shaping each grapheme separately and then use the result sizes
    //  to calculate the proportions
    auto leftCorrection = start->sizeToChar(text.begin());
    auto rightCorrection = end->sizeFromChar(text.end() - 1);
    clip.fLeft += leftCorrection;
    clip.fRight -= rightCorrection;
    clippingNeeded = leftCorrection != 0 || rightCorrection != 0;

    //SkDebugf("measureTextInsideOneRun: '%s'[%d:%d]\n", text.begin(), pos, pos + size);

    return clip;
}

void TextLine::iterateThroughClustersInGlyphsOrder(bool reverse,
                                                 const ClustersVisitor& visitor) const {
    TRACE_EVENT0("skia", TRACE_FUNC);
    for (size_t r = 0; r != fLogical.size(); ++r) {
        auto& run = fLogical[reverse ? fLogical.size() - r - 1 : r];
        // Walk through the clusters in the logical order (or reverse)
        auto normalOrder = run->leftToRight() != reverse;
        auto start = normalOrder ? run->clusters().begin() : run->clusters().end() - 1;
        auto end = normalOrder ? run->clusters().end() : run->clusters().begin() - 1;
        for (auto cluster = start; cluster != end; normalOrder ? ++cluster : --cluster) {
            if (!this->contains(cluster)) {
                continue;
            }
            if (!visitor(cluster)) {
                return;
            }
        }
    }
}

// Walk through the runs in the logical order
SkScalar TextLine::iterateThroughRuns(SkSpan<const char> text,
                                    SkScalar runOffset,
                                    bool includeEmptyText,
                                    const RunVisitor& visitor) const {
    TRACE_EVENT0("skia", TRACE_FUNC);

    SkScalar width = 0;
    for (auto& run : fLogical) {
        // Only skip the text if it does not even touch the run
        if (intersectedSize(run->text(), text) < 0) {
            continue;
        }

        SkSpan<const char> intersect = intersected(run->text(), text);
        if (run->text().empty() || intersect.empty()) {
            continue;
        }

        size_t pos;
        size_t size;
        bool clippingNeeded;
        SkRect clip = this->measureTextInsideOneRun(intersect, run, pos, size, clippingNeeded);
        if (clip.height() == 0) {
            continue;
        }

        auto shift = runOffset - clip.fLeft;
        clip.offset(shift, 0);
        if (clip.fRight > fAdvance.fX) {
            clip.fRight = fAdvance.fX;
            clippingNeeded = true;  // Correct the clip in case there was an ellipsis
        } else if (run == fLogical.back() && this->ellipsis() != nullptr) {
            clippingNeeded = true;  // To avoid trouble
        }

        if (!visitor(run, pos, size, clip, shift - run->positionX(0), clippingNeeded)) {
            return width;
        }

        width += clip.width();
        runOffset += clip.width();
    }

    if (this->ellipsis() != nullptr) {
        auto ellipsis = this->ellipsis();
        if (!visitor(ellipsis, 0, ellipsis->size(), ellipsis->clip(), ellipsis->clip().fLeft,
                     false)) {
            return width;
        }
        width += ellipsis->clip().width();
    }

    return width;
}

void TextLine::iterateThroughStylesInTextOrder(StyleType styleType,
                                             const StyleVisitor& visitor) const {
    TRACE_EVENT0("skia", TRACE_FUNC);
    const char* start = nullptr;
    size_t size = 0;
    TextStyle prevStyle;

    SkScalar offsetX = 0;
    for (auto& block : fBlocks) {
        auto intersect = intersected(block.text(), this->trimmedText());
        if (intersect.empty()) {
            if (start == nullptr) {
                // This style is not applicable to the line
                continue;
            } else {
                // We have found all the good styles already
                break;
            }
        }

        auto style = block.style();
        if (start != nullptr && style.matchOneAttribute(styleType, prevStyle)) {
            size += intersect.size();
            continue;
        } else if (size == 0) {
            // First time only
            prevStyle = style;
            size = intersect.size();
            start = intersect.begin();
            continue;
        }

        auto width = visitor(SkSpan<const char>(start, size), prevStyle, offsetX);
        offsetX += width;

        // Start all over again
        prevStyle = style;
        start = intersect.begin();
        size = intersect.size();
    }

    // The very last style
    auto width = visitor(SkSpan<const char>(start, size), prevStyle, offsetX);
    offsetX += width;

    // This is a very important assert!
    // It asserts that 2 different ways of calculation come with the same results
    if (!SkScalarNearlyEqual(offsetX, this->width())) {
        SkDebugf("ASSERT: %f != %f\n", offsetX, this->width());
    }
    SkASSERT(SkScalarNearlyEqual(offsetX, this->width()));
}
}  // namespace textlayout
}  // namespace skia
