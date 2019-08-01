// Copyright 2019 Google LLC.
#include "modules/skparagraph/src/TextLine.h"
#include <unicode/brkiter.h>
#include <unicode/ubidi.h>
#include "modules/skparagraph/src/ParagraphImpl.h"

#include "include/core/SkMaskFilter.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkDiscretePathEffect.h"
#include "src/core/SkMakeUnique.h"

namespace skia {
namespace textlayout {
// TODO: deal with all the intersection functionality
int32_t intersectedSize(TextRange a, TextRange b) {
    if (a.empty() || b.empty()) {
        return -1;
    }
    auto begin = SkTMax(a.start, b.start);
    auto end = SkTMin(a.end, b.end);
    return begin <= end ? SkToS32(end - begin) : -1;
}

TextRange intersected(const TextRange& a, const TextRange& b) {
    if (a.start == b.start && a.end == b.end) return a;
    auto begin = SkTMax(a.start, b.start);
    auto end = SkTMin(a.end, b.end);
    return end >= begin ? TextRange(begin, end) : EMPTY_TEXT;
}

SkTHashMap<SkFont, Run> TextLine::fEllipsisCache;

TextLine::TextLine(ParagraphImpl* master,
                   SkVector offset,
                   SkVector advance,
                   BlockRange blocks,
                   TextRange text,
                   TextRange textWithSpaces,
                   ClusterRange clusters,
                   ClusterRange clustersWithGhosts,
                   SkScalar widthWithSpaces,
                   LineMetrics sizes)
        : fMaster(master)
        , fBlockRange(blocks)
        , fTextRange(text)
        , fTextWithWhitespacesRange(textWithSpaces)
        , fClusterRange(clusters)
        , fGhostClusterRange(clustersWithGhosts)
        , fLogical()
        , fAdvance(advance)
        , fOffset(offset)
        , fShift(0.0)
        , fWidthWithSpaces(widthWithSpaces)
        , fEllipsis(nullptr)
        , fSizes(sizes)
        , fHasBackground(false)
        , fHasShadows(false)
        , fHasDecorations(false) {
    // Reorder visual runs
    auto& start = master->cluster(fGhostClusterRange.start);
    auto& end = master->cluster(fGhostClusterRange.end - 1);
    size_t numRuns = end.runIndex() - start.runIndex() + 1;

    for (BlockIndex index = fBlockRange.start; index < fBlockRange.end; ++index) {
        auto b = fMaster->styles().begin() + index;
        if (b->fStyle.hasBackground()) {
            fHasBackground = true;
        }
        if (b->fStyle.getDecorationType() != TextDecoration::kNoDecoration) {
            fHasDecorations = true;
        }
        if (b->fStyle.getShadowNumber() > 0) {
            fHasShadows = true;
        }
    }

    // Get the logical order
    std::vector<UBiDiLevel> runLevels;
    for (auto runIndex = start.runIndex(); runIndex <= end.runIndex(); ++runIndex) {
        auto& run = fMaster->run(runIndex);
        runLevels.emplace_back(run.fBidiLevel);
    }

    std::vector<int32_t> logicalOrder(numRuns);
    ubidi_reorderVisual(runLevels.data(), SkToU32(numRuns), logicalOrder.data());

    auto firstRunIndex = start.runIndex();
    for (auto index : logicalOrder) {
        fLogical.push_back(firstRunIndex + index);
    }
}

void TextLine::paint(SkCanvas* textCanvas) {
    if (this->empty()) {
        return;
    }

    textCanvas->save();
    textCanvas->translate(this->offset().fX, this->offset().fY);

    if (fHasBackground) {
        this->iterateThroughStylesInTextOrder(
            StyleType::kBackground,
            [this, textCanvas](TextRange textRange, const TextStyle& style, SkScalar offsetX) {
                return this->paintBackground(textCanvas, textRange, style, offsetX);
            });
    }

    if (fHasShadows) {
        this->iterateThroughStylesInTextOrder(
            StyleType::kShadow,
            [textCanvas, this](TextRange textRange, const TextStyle& style, SkScalar offsetX) {
                return this->paintShadow(textCanvas, textRange, style, offsetX);
            });
    }

    this->iterateThroughStylesInTextOrder(
        StyleType::kForeground,
        [textCanvas, this](TextRange textRange, const TextStyle& style, SkScalar offsetX) {
            return this->paintText(textCanvas, textRange, style, offsetX);
        });

    if (fHasDecorations) {
        this->iterateThroughStylesInTextOrder(
        StyleType::kDecorations,
        [textCanvas, this](TextRange textRange, const TextStyle& style, SkScalar offsetX) {
            return this->paintDecorations(textCanvas, textRange, style, offsetX);
        });
    }

    textCanvas->restore();
}

void TextLine::format(TextAlign effectiveAlign, SkScalar maxWidth) {
    SkScalar delta = maxWidth - this->width();
    if (delta <= 0) {
        return;
    }

    if (effectiveAlign == TextAlign::kJustify) {
        this->justify(maxWidth);
    } else if (effectiveAlign == TextAlign::kRight) {
        fShift = delta;
    } else if (effectiveAlign == TextAlign::kCenter) {
        fShift = delta / 2;
    }
}

TextAlign TextLine::assumedTextAlign() const {
    if (this->fMaster->paragraphStyle().getTextAlign() != TextAlign::kJustify) {
        return this->fMaster->paragraphStyle().getTextAlign();
    }
    if (fClusterRange.empty()) {
        return TextAlign::kLeft;
    } else {
        auto run = this->fMaster->cluster(fClusterRange.end - 1).run();
        return run->leftToRight() ? TextAlign::kLeft : TextAlign::kRight;
    }
}

void TextLine::scanStyles(StyleType style, const StyleVisitor& visitor) {
    if (this->empty()) {
        return;
    }

    this->iterateThroughStylesInTextOrder(
            style, [this, visitor](TextRange textRange, const TextStyle& style, SkScalar offsetX) {
                visitor(textRange, style, offsetX);
                return this->iterateThroughRuns(
                        textRange, offsetX, false,
                        [](Run*, int32_t, size_t, SkRect, SkScalar, bool) { return true; });
            });
}

void TextLine::scanRuns(const RunVisitor& visitor) {
    this->iterateThroughRuns(
            fTextRange, 0, false,
            [visitor](Run* run, int32_t pos, size_t size, SkRect clip, SkScalar sc, bool b) {
                visitor(run, pos, size, clip, sc, b);
                return true;
            });
}

SkScalar TextLine::paintText(SkCanvas* canvas, TextRange textRange, const TextStyle& style,
                             SkScalar offsetX) const {
    SkPaint paint;
    if (style.hasForeground()) {
        paint = style.getForeground();
    } else {
        paint.setColor(style.getColor());
    }

    auto shiftDown =  this->baseline();
    return this->iterateThroughRuns(
        textRange, offsetX, false,
        [canvas, paint, shiftDown](Run* run, int32_t pos, size_t size, SkRect clip, SkScalar shift, bool clippingNeeded) {
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

SkScalar TextLine::paintBackground(SkCanvas* canvas, TextRange textRange,
                                   const TextStyle& style, SkScalar offsetX) const {
    return this->iterateThroughRuns(textRange, offsetX, false,
        [canvas, &style](Run* run, int32_t pos, size_t size, SkRect clip,
                        SkScalar shift, bool clippingNeeded) {
            if (style.hasBackground()) {
                canvas->drawRect(clip, style.getBackground());
            }
            return true;
        });
}

SkScalar TextLine::paintShadow(SkCanvas* canvas, TextRange textRange, const TextStyle& style,
                               SkScalar offsetX) const {
    if (style.getShadowNumber() == 0) {
        // Still need to calculate text advance
        return iterateThroughRuns(
                textRange, offsetX, false,
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
                textRange, offsetX, false,
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

SkScalar TextLine::paintDecorations(SkCanvas* canvas, TextRange textRange,
                                    const TextStyle& style, SkScalar offsetX) const {
    return this->iterateThroughRuns(
            textRange, offsetX, false,
            [this, canvas, &style](Run* run, int32_t pos, size_t size, SkRect clip, SkScalar shift,
                                  bool clippingNeeded) {
                if (style.getDecorationType() == TextDecoration::kNoDecoration) {
                    return true;
                }

                for (auto decoration : AllTextDecorations) {
                    if ((style.getDecorationType() & decoration) == 0) {
                        continue;
                    }

                    SkScalar thickness = style.getDecorationThicknessMultiplier();
                    //
                    SkScalar position = 0;
                    switch (decoration) {
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
                            SkASSERT(false);
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
    // Count words and the extra spaces to spread across the line
    // TODO: do it at the line breaking?..
    size_t whitespacePatches = 0;
    SkScalar textLen = 0;
    bool whitespacePatch = false;
    this->iterateThroughClustersInGlyphsOrder(
            false, [&whitespacePatches, &textLen, &whitespacePatch](const Cluster* cluster, ClusterIndex index) {
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
        return;
    }

    SkScalar step = (maxWidth - textLen) / whitespacePatches;
    SkScalar shift = 0;

    // Spread the extra whitespaces
    whitespacePatch = false;
    this->iterateThroughClustersInGlyphsOrder(false, [&](const Cluster* cluster, ClusterIndex index) {
        if (cluster->isWhitespaces()) {
            if (!whitespacePatch) {
                shift += step;
                whitespacePatch = true;
                --whitespacePatches;
            }
        } else {
            whitespacePatch = false;
        }
        fMaster->shiftCluster(index, shift);
        return true;
    });

    SkAssertResult(SkScalarNearlyEqual(shift, maxWidth - textLen));
    SkASSERT(whitespacePatches == 0);
    this->fAdvance.fX = maxWidth;
}

void TextLine::createEllipsis(SkScalar maxWidth, const SkString& ellipsis, bool) {
    // Replace some clusters with the ellipsis
    // Go through the clusters in the reverse logical order
    // taking off cluster by cluster until the ellipsis fits
    SkScalar width = fAdvance.fX;
    iterateThroughClustersInGlyphsOrder(
            true, [this, &width, ellipsis, maxWidth](const Cluster* cluster, ClusterIndex index) {
                if (cluster->isWhitespaces()) {
                    width -= cluster->width();
                    return true;
                }

                // Shape the ellipsis
                Run* cached = fEllipsisCache.find(cluster->font());
                if (cached == nullptr) {
                    cached = shapeEllipsis(ellipsis, cluster->run());
                } else {
                    cached->setMaster(fMaster);
                }
                fEllipsis = std::make_shared<Run>(*cached);

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

    class ShapeHandler final : public SkShaper::RunHandler {
    public:
        ShapeHandler(SkScalar lineHeight, const SkString& ellipsis)
            : fRun(nullptr), fLineHeight(lineHeight), fEllipsis(ellipsis) {}
        Run* run() { return fRun; }

    private:
        void beginLine() override {}

        void runInfo(const RunInfo&) override {}

        void commitRunInfo() override {}

        Buffer runBuffer(const RunInfo& info) override {
            fRun = fEllipsisCache.set(info.fFont,
                                      Run(nullptr, info, fLineHeight, 0, 0));
            return fRun->newRunBuffer();
        }

        void commitRunBuffer(const RunInfo& info) override {
            fRun->fAdvance.fX = info.fAdvance.fX;
            fRun->fAdvance.fY = fRun->descent() - fRun->ascent();
        }

        void commitLine() override {}

        Run* fRun;
        SkScalar fLineHeight;
        SkString fEllipsis;
    };

    ShapeHandler handler(run->lineHeight(), ellipsis);
    std::unique_ptr<SkShaper> shaper = SkShaper::MakeShapeDontWrapOrReorder();
    SkASSERT_RELEASE(shaper != nullptr);
    shaper->shape(ellipsis.c_str(), ellipsis.size(), run->font(), true,
                  std::numeric_limits<SkScalar>::max(), &handler);
    handler.run()->fTextRange = TextRange(0, ellipsis.size());
    handler.run()->fMaster = fMaster;
    return handler.run();
}

SkRect TextLine::measureTextInsideOneRun(
        TextRange textRange, Run* run, size_t& pos, size_t& size, bool includeGhostSpaces, bool& clippingNeeded) const {

    SkASSERT(intersectedSize(run->textRange(), textRange) >= 0);

    // Find [start:end] clusters for the text
    bool found;
    ClusterIndex startIndex;
    ClusterIndex endIndex;
    std::tie(found, startIndex, endIndex) = run->findLimitingClusters(textRange);
    if (!found) {
        SkASSERT(textRange.empty());
        return SkRect::MakeEmpty();
    }

    auto start = fMaster->clusters().begin() + startIndex;
    auto end = fMaster->clusters().begin() + endIndex;
    pos = start->startPos();
    size = end->endPos() - start->startPos();

    // Calculate the clipping rectangle for the text with cluster edges
    // There are 2 cases:
    // EOL (when we expect the last cluster clipped without any spaces)
    // Anything else (when we want the cluster width contain all the spaces -
    // coming from letter spacing or word spacing or justification)
    auto range = includeGhostSpaces ? fGhostClusterRange : fClusterRange;
    bool needsClipping = (run->leftToRight() ? endIndex == range.end  - 1 : startIndex == range.end);
    SkRect clip =
            SkRect::MakeXYWH(run->positionX(start->startPos()) - run->positionX(0),
                             sizes().runTop(run),
                             run->calculateWidth(start->startPos(), end->endPos(), needsClipping),
                             run->calculateHeight());

    // Correct the width in case the text edges don't match clusters
    // TODO: This is where we get smart about selecting a part of a cluster
    //  by shaping each grapheme separately and then use the result sizes
    //  to calculate the proportions
    auto leftCorrection = start->sizeToChar(textRange.start);
    auto rightCorrection = end->sizeFromChar(textRange.end - 1);
    clip.fLeft += leftCorrection;
    clip.fRight -= rightCorrection;
    clippingNeeded = leftCorrection != 0 || rightCorrection != 0;

    // SkDebugf("measureTextInsideOneRun: '%s'[%d:%d]\n", text.begin(), pos, pos + size);

    return clip;
}

void TextLine::iterateThroughClustersInGlyphsOrder(bool reverse,
                                                   const ClustersVisitor& visitor) const {
    for (size_t r = 0; r != fLogical.size(); ++r) {
        auto& runIndex = fLogical[reverse ? fLogical.size() - r - 1 : r];
        // Walk through the clusters in the logical order (or reverse)
        auto run = this->fMaster->runs().begin() + runIndex;
        auto normalOrder = run->leftToRight() != reverse;
        auto start = normalOrder ? run->clusterRange().start : run->clusterRange().end - 1;
        auto end = normalOrder ? run->clusterRange().end : run->clusterRange().start - 1;
        for (auto index = start; index != end; normalOrder ? ++index : --index) {
            const auto& cluster = fMaster->clusters().begin() + index;
            if (!this->contains(cluster)) {
                continue;
            }
            if (!visitor(cluster, index)) {
                return;
            }
        }
    }
}

// Walk through the runs in the logical order
SkScalar TextLine::iterateThroughRuns(TextRange textRange,
                                      SkScalar runOffset,
                                      bool includeGhostWhitespaces,
                                      const RunVisitor& visitor) const {
    TRACE_EVENT0("skia", TRACE_FUNC);

    SkScalar width = 0;
    for (auto& runIndex : fLogical) {
        const auto run = &this->fMaster->run(runIndex);
        // Only skip the text if it does not even touch the run
        auto intersection = intersectedSize(run->textRange(), textRange);
        if (intersection < 0 || (intersection == 0 && textRange.end != run->textRange().end)) {
            continue;
        }

        auto intersect = intersected(run->textRange(), textRange);
        if (run->textRange().empty() || intersect.empty()) {
            continue;
        }

        // Measure the text
        size_t pos;
        size_t size;
        bool clippingNeeded;
        SkRect clip = this->measureTextInsideOneRun(intersect, run, pos, size, includeGhostWhitespaces, clippingNeeded);
        if (clip.height() == 0) {
            continue;
        }

        // Clip the text
        auto shift = runOffset - clip.fLeft;
        clip.offset(shift, 0);
        if (includeGhostWhitespaces) {
            clippingNeeded = false;
        } else {
            clippingNeeded = true;
            if (clip.fRight > fAdvance.fX) {
                clip.fRight = fAdvance.fX;
            }
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

    TextIndex start = EMPTY_INDEX;
    size_t size = 0;
    const TextStyle* prevStyle = nullptr;

    SkScalar offsetX = 0;
    for (BlockIndex index = fBlockRange.start; index < fBlockRange.end; ++index) {
        auto block = fMaster->styles().begin() + index;
        auto intersect = intersected(block->fRange, this->trimmedText());
        if (intersect.empty()) {
            if (start == EMPTY_INDEX) {
                // This style is not applicable to the line
                continue;
            } else {
                // We have found all the good styles already
                break;
            }
        }

        auto* style = &block->fStyle;
        if (start != EMPTY_INDEX && style->matchOneAttribute(styleType, *prevStyle)) {
            size += intersect.width();
            continue;
        } else if (start == EMPTY_INDEX ) {
            // First time only
            prevStyle = style;
            size = intersect.width();
            start = intersect.start;
            continue;
        }

        auto width = visitor(TextRange(start, start + size), *prevStyle, offsetX);
        offsetX += width;

        // Start all over again
        prevStyle = style;
        start = intersect.start;
        size = intersect.width();
    }

    if (prevStyle == nullptr) return;

    // The very last style
    auto width = visitor(TextRange(start, start + size), *prevStyle, offsetX);
    offsetX += width;

    // This is a very important assert!
    // It asserts that 2 different ways of calculation come with the same results
    if (!SkScalarNearlyEqual(offsetX, this->width())) {
        SkDebugf("ASSERT: %f != %f\n", offsetX, this->width());
    }
    SkASSERT(SkScalarNearlyEqual(offsetX, this->width()));
}

SkVector TextLine::offset() const {
    return fOffset + SkVector::Make(fShift, 0);
}
}  // namespace textlayout
}  // namespace skia
