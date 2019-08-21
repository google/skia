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
        , fRunsInVisualOrder()
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
        if (b->fStyle.isPlaceholder()) {
            continue;
        }
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
        fRunsInVisualOrder.push_back(firstRunIndex + index);
    }
/*
    SkDebugf("Visual Runs: %d\n", fLogical.size());
    for (size_t l = 0; l < fLogical.size(); ++l) {
        auto p = fLogical[l];
        auto run = fMaster->run(p);
        if (run.placeholder() != nullptr) {
            SkDebugf("run[%d], %d: %d*d\n", p, run.fBidiLevel, run.placeholder()->fWidth, run.placeholder()->fHeight);
        } else {
            auto text = fMaster->text(run.textRange());
            SkDebugf("run[%d], %d: %s\n", p, run.fBidiLevel, SkString(text.begin(), text.size()).c_str());
        }
    }
*/
}

void TextLine::paint(SkCanvas* textCanvas) {
    if (this->empty()) {
        return;
    }

    textCanvas->save();
    textCanvas->translate(this->offset().fX, this->offset().fY);

    if (fHasBackground) {
        this->iterateThroughVisualRuns(false,
            [textCanvas, this](ClipContext context, SkScalar runOffset, TextRange textRange) {
            this->iterateThroughSingleRunByStyles(
                context.run, runOffset, textRange, StyleType::kBackground,
                [textCanvas, this](TextRange textRange, const TextStyle& style, const ClipContext& context) {
                    this->paintBackground(textCanvas, textRange, style, context);
                });
            return true;
            });
    }

    if (fHasShadows) {
        this->iterateThroughVisualRuns(false,
            [textCanvas, this](ClipContext context, SkScalar runOffset, TextRange textRange) {
            this->iterateThroughSingleRunByStyles(
                context.run, runOffset, textRange, StyleType::kBackground,
                [textCanvas, this](TextRange textRange, const TextStyle& style, const ClipContext& context) {
                    this->paintShadow(textCanvas, textRange, style, context);
                });
            return true;
            });
    }

    this->iterateThroughVisualRuns(false,
        [textCanvas, this](ClipContext context, SkScalar runOffset, TextRange textRange) {
        if (context.run->placeholder() != nullptr) {
            return true;
        }
        this->iterateThroughSingleRunByStyles(
            context.run, runOffset, textRange, StyleType::kBackground,
            [textCanvas, this](TextRange textRange, const TextStyle& style, const ClipContext& context) {
                this->paintText(textCanvas, textRange, style, context);
            });
            return true;
        });

    if (fHasDecorations) {
        this->iterateThroughVisualRuns(false,
            [textCanvas, this](ClipContext context, SkScalar runOffset, TextRange textRange) {
            this->iterateThroughSingleRunByStyles(
                context.run, runOffset, textRange, StyleType::kBackground,
                [textCanvas, this](TextRange textRange, const TextStyle& style, const ClipContext& context) {
                    this->paintDecorations(textCanvas, textRange, style, context);
                });
            return true;
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
        return this->fMaster->paragraphStyle().effective_align();
    }

    if (fClusterRange.empty()) {
        return TextAlign::kLeft;
    } else {
        auto run = this->fMaster->cluster(fClusterRange.end - 1).run();
        return run->leftToRight() ? TextAlign::kLeft : TextAlign::kRight;
    }
}

void TextLine::scanStyles(StyleType styleType, const RunStyleVisitor& visitor) {
    if (this->empty()) {
        return;
    }

    this->iterateThroughVisualRuns(false,
        [this, visitor, styleType](ClipContext context, SkScalar runOffset, TextRange textRange) {
            this->iterateThroughSingleRunByStyles(
                context.run, runOffset, textRange, styleType,
                [this, visitor](TextRange textRange, const TextStyle& style, const ClipContext& context) {
                    visitor(textRange, style, context);
                });
            return true;
        });
}

void TextLine::scanRuns(const RunVisitor& visitor) {
    this->iterateThroughVisualRuns(
            false,
            [visitor](ClipContext context, SkScalar runOffset, TextRange textRange) {
                visitor(context, runOffset, textRange);
                return true;
            });
}

void TextLine::paintText(SkCanvas* canvas, TextRange textRange, const TextStyle& style, const ClipContext& context) const {
    SkPaint paint;
    if (style.hasForeground()) {
        paint = style.getForeground();
    } else {
        paint.setColor(style.getColor());
    }

    auto shiftDown =  this->baseline();


    if (context.run->placeholder() != nullptr) {
        return;
    }
/*
    auto text = fMaster->text(textRange);
    SkString str(text.begin(), text.size());
    SkDebugf("paintText '%s'[%d:%d) for run[%d] +%f [%f:%f)\n", str.c_str(), textRange.start,
             textRange.end, context.run->fIndex, context.shift, context.clip.fLeft,
             context.clip.fRight);
*/
    SkTextBlobBuilder builder;
    context.run->copyTo(builder, SkToU32(context.pos), context.size, SkVector::Make(0, shiftDown));
    canvas->save();
    if (context.clippingNeeded) {
        canvas->clipRect(context.clip);
    }
    canvas->translate(context.shift, 0);
    canvas->drawTextBlob(builder.make(), 0, 0, paint);
    canvas->restore();
}

void TextLine::paintBackground(SkCanvas* canvas, TextRange textRange, const TextStyle& style, const ClipContext& context) const {
    if (style.hasBackground()) {
        canvas->drawRect(context.clip, style.getBackground());
    }
}

void TextLine::paintShadow(SkCanvas* canvas, TextRange textRange, const TextStyle& style, const ClipContext& context) const {
    auto shiftDown = this->baseline();
    for (TextShadow shadow : style.getShadows()) {
        if (!shadow.hasShadow()) continue;

        SkPaint paint;
        paint.setColor(shadow.fColor);
        if (shadow.fBlurRadius != 0.0) {
            auto filter = SkMaskFilter::MakeBlur(kNormal_SkBlurStyle,
                                                 SkDoubleToScalar(shadow.fBlurRadius), false);
            paint.setMaskFilter(filter);
        }

        SkTextBlobBuilder builder;
        context.run->copyTo(builder, context.pos, context.size, SkVector::Make(0, shiftDown));
        canvas->save();
        SkRect clip = context.clip;
        clip.offset(shadow.fOffset);
        if (context.clippingNeeded) {
            canvas->clipRect(clip);
        }
        canvas->translate(context.shift, 0);
        canvas->drawTextBlob(builder.make(), shadow.fOffset.x(), shadow.fOffset.y(), paint);
        canvas->restore();
    }
}

void TextLine::paintDecorations(SkCanvas* canvas, TextRange textRange, const TextStyle& style, const ClipContext& context) const {
    if (style.getDecorationType() == TextDecoration::kNoDecoration) {
        return;
    }

    for (auto decoration : AllTextDecorations) {
        if ((style.getDecorationType() & decoration) == 0) {
            continue;
        }

        SkScalar thickness = computeDecorationThickness(style);
        SkScalar position = computeDecorationPosition(style);
        switch (decoration) {
            case TextDecoration::kUnderline:
                position = - context.run->correctAscent() + thickness;
                break;
            case TextDecoration::kOverline:
                position = 0;
                break;
            case TextDecoration::kLineThrough: {
                position = (context.run->correctDescent() - context.run->correctAscent() - thickness) / 2;
                break;
            }
            default:
                SkASSERT(false);
                break;
        }

        auto width = context.clip.width();
        SkScalar x = context.clip.left();
        SkScalar y = context.clip.top() + position;

        // Decoration paint (for now) and/or path
        SkPaint paint;
        SkPath path;
        this->computeDecorationPaint(paint, context.clip, style, path);
        paint.setStrokeWidth(thickness * style.getDecorationThicknessMultiplier());

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
}

SkScalar TextLine::computeDecorationThickness(const TextStyle& style) const {
    SkFontMetrics fontMetrics;
    style.getFontMetrics(&fontMetrics);
    if ((fontMetrics.fFlags & SkFontMetrics::FontMetricsFlags::kUnderlineThicknessIsValid_Flag) &&
         fontMetrics.fUnderlineThickness > 0) {
        return fontMetrics.fUnderlineThickness;
    } else {
        return style.getFontSize() / 14.0f;
    }
}

SkScalar TextLine::computeDecorationPosition(const TextStyle& style) const {
    SkFontMetrics fontMetrics;
    style.getFontMetrics(&fontMetrics);
    if ((fontMetrics.fFlags & SkFontMetrics::FontMetricsFlags::kUnderlinePositionIsValid_Flag) &&
         fontMetrics.fUnderlinePosition > 0) {
        return fontMetrics.fUnderlinePosition;
    } else {
        return style.getFontSize() / 14.0f;
    }
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
    this->iterateThroughClustersInGlyphsOrder(false, false,
        [&whitespacePatches, &textLen, &whitespacePatch](const Cluster* cluster, ClusterIndex index, bool leftToRight, bool ghost) {
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

    // Deal with the ghost spaces
    auto ghostShift = maxWidth - this->fAdvance.fX;
    // Spread the extra whitespaces
    whitespacePatch = false;
    this->iterateThroughClustersInGlyphsOrder(false, true, [&](const Cluster* cluster, ClusterIndex index, bool leftToRight, bool ghost) {

        if (ghost) {
            if (leftToRight) {
                fMaster->shiftCluster(index, ghostShift);
            }
            return true;
        }

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

    this->fWidthWithSpaces += ghostShift;
    this->fAdvance.fX = maxWidth;
}

void TextLine::createEllipsis(SkScalar maxWidth, const SkString& ellipsis, bool) {
    // Replace some clusters with the ellipsis
    // Go through the clusters in the reverse logical order
    // taking off cluster by cluster until the ellipsis fits
    SkScalar width = fAdvance.fX;
    iterateThroughClustersInGlyphsOrder(
        true, false, [this, &width, ellipsis, maxWidth](const Cluster* cluster, ClusterIndex index, bool leftToRight, bool ghost) {
            if (cluster->isWhitespaces()) {
                width -= cluster->width();
                return true;
            }

            // Shape the ellipsis
            Run* run = shapeEllipsis(ellipsis, cluster->run());
            run->setMaster(fMaster);
            fEllipsis = std::make_shared<Run>(*run);

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
            fRun = new  Run(nullptr, info, 0, fLineHeight, 0, 0);
            return fRun->newRunBuffer();
        }

        void commitRunBuffer(const RunInfo& info) override {
            fRun->fAdvance.fX = info.fAdvance.fX;
            fRun->fAdvance.fY = fRun->advance().fY;
            fRun->fPlaceholder = nullptr;
            fRun->fEllipsis = true;
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

TextLine::ClipContext TextLine::measureTextInsideOneRun(TextRange textRange, const Run* run, SkScalar runOffset, bool includeGhostSpaces) const {

    SkASSERT(intersectedSize(run->textRange(), textRange) >= 0);

    ClipContext result;
    result.run = run;
    if (run->placeholder() != nullptr) {
        SkASSERT(textRange == run->textRange());
        result.pos = 0;
        result.size = 1;
        result.clippingNeeded = false;
        result.clip = SkRect::MakeXYWH(runOffset, sizes().runTop(run), run->advance().fX, run->calculateHeight());
        return result;
    } else if (run->fEllipsis) {
        result.pos = 0;
        result.size = run->size();
        result.clippingNeeded = false;
        result.clip = SkRect::MakeXYWH(runOffset, sizes().runTop(run), run->advance().fX, run->calculateHeight());
        return result;
    }

    // Find [start:end] clusters for the text
    bool found;
    ClusterIndex startIndex;
    ClusterIndex endIndex;
    std::tie(found, startIndex, endIndex) = run->findLimitingClusters(textRange);
    if (!found) {
        SkASSERT(textRange.empty());
        result.clip = SkRect::MakeEmpty();
        return result;
    }

    auto start = &fMaster->cluster(startIndex);
    auto end = &fMaster->cluster(endIndex);
    auto zero = &fMaster->cluster(run->leftToRight() ? clusters().start : clusters().end);
    result.pos = start->startPos();
    result.size = end->endPos() - start->startPos();
    result.run = run;

    // Calculate the clipping rectangle for the text with cluster edges
    // There are 2 cases:
    // EOL (when we expect the last cluster clipped without any spaces)
    // Anything else (when we want the cluster width contain all the spaces -
    // coming from letter spacing or word spacing or justification)
    auto range = includeGhostSpaces ? fGhostClusterRange : fClusterRange;
    bool needsClipping = (run->leftToRight() ? endIndex == range.end  - 1 : startIndex == range.end);
    result.clip =
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
    result.clip.fLeft += leftCorrection;
    result.clip.fRight -= rightCorrection;
    result.clippingNeeded = leftCorrection != 0 || rightCorrection != 0;

    if (!includeGhostSpaces) {
        // The text must be aligned with the runOffset
        result.shift = runOffset - result.clip.fLeft - run->positionX(0);
        result.clip.offset(runOffset - result.clip.fLeft, 0);

        if (result.clip.fRight > fAdvance.fX) {
            result.clip.fRight = fAdvance.fX;
        }
    } else {
        // The box must be aligned with the lineOffset
        if (zero->runIndex() != start->runIndex()) {
            // Run starts after the line
            result.shift = runOffset;
        } else {
            // Run starts before or on the line
            result.shift = run->positionX(0) - run->positionX(zero->startPos());
        }
        result.clip.offset(result.shift, 0);
    }

    return result;
}

void TextLine::iterateThroughClustersInGlyphsOrder(bool reverse,
                                                   bool includeGhosts,
                                                   const ClustersVisitor& visitor) const {
    // Walk through the clusters in the logical order (or reverse)
    for (size_t r = 0; r != fRunsInVisualOrder.size(); ++r) {
        auto& runIndex = fRunsInVisualOrder[reverse ? fRunsInVisualOrder.size() - r - 1 : r];
        auto run = this->fMaster->runs().begin() + runIndex;
        auto start = SkTMax(run->clusterRange().start, fClusterRange.start);
        auto end = SkTMin(run->clusterRange().end, fClusterRange.end);
        auto ghosts = SkTMin(run->clusterRange().end, fGhostClusterRange.end);

        if (run->leftToRight() != reverse) {
            for (auto index = start; index < ghosts; ++index) {
                if (index >= end && !includeGhosts) {
                    break;
                }
                const auto& cluster = &fMaster->cluster(index);
                if (!visitor(cluster, index, run->leftToRight(), index >= end)) {
                    return;
                }
            }
        } else {
            for (auto index = ghosts; index > start; --index) {
                if (index > end && !includeGhosts) {
                    continue;
                }
                const auto& cluster = &fMaster->cluster(index - 1);
                if (!visitor(cluster, index - 1, run->leftToRight(), index > end)) {
                    return;
                }
            }
        }
    }
}

SkScalar TextLine::calculateLeftVisualOffset(TextRange textRange) const {
    SkScalar partOfTheCurrentRun = 0;
    return this->iterateThroughVisualRuns(true,
        [textRange, &partOfTheCurrentRun, this](ClipContext context, SkScalar runOffset, TextRange text) {
            if (text.start > textRange.start || text.end <= textRange.start) {
                // This run does not even touch the text start
            } else {
                // This is the run
                TextRange part;
                if (context.run->leftToRight()) {
                    part = {text.start, textRange.start};
                } else if (textRange.end < text.end) {
                    part = {textRange.end, text.end};
                }
                if (part.width() == 0) {
                    return false;
                }
                partOfTheCurrentRun = context.clip.width();
                return false;
            }

            return true;
        }) +
           partOfTheCurrentRun;
}

void TextLine::iterateThroughSingleRunByStyles(const Run* run,
                                               SkScalar runOffset,
                                               TextRange textRange,
                                               StyleType styleType,
                                               const RunStyleVisitor& visitor) const {

    TextIndex start = EMPTY_INDEX;
    size_t size = 0;
    const TextStyle* prevStyle = nullptr;

    for (BlockIndex index = fBlockRange.start; index <= fBlockRange.end; ++index) {

        TextRange intersect;
        TextStyle* style = nullptr;
        if (index < fBlockRange.end) {
            auto block = fMaster->styles().begin() + index;

            // Get the text
            intersect = intersected(block->fRange, textRange);
            if (intersect.width() == 0) {
                if (start == EMPTY_INDEX) {
                    // This style is not applicable to the text yet
                    continue;
                } else {
                    // We have found all the good styles already
                    // but we need to process the last one of them
                    intersect = TextRange(start, start + size);
                    index = fBlockRange.end;
                }
            } else {
                // Get the style
                style = &block->fStyle;
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
            }
        } else if (prevStyle != nullptr) {
            // This is the last style
            intersect = TextRange(start, start + size);
        } else {
            break;
        }

        // We have the style and the text

        // Measure the text
        ClipContext clipContext = this->measureTextInsideOneRun(intersect, run, runOffset, false);
        if (clipContext.clip.height() == 0) {
            continue;
        }

        visitor(TextRange(start, start + size), *prevStyle, clipContext);

        // Start all over again
        prevStyle = style;
        start = intersect.start;
        size = intersect.width();
    }
}

SkScalar TextLine::iterateThroughVisualRuns(bool includingGhostSpaces, const RunVisitor& visitor) const {

    // Walk through all the runs that intersect with the line in visual order
    SkScalar width = 0;
    SkScalar runOffset = 0;
    auto textRange = includingGhostSpaces ? this->textWithSpaces() : this->trimmedText();
    for (auto& runIndex : fRunsInVisualOrder) {

        // Check if the run intersects with the line at all
        const auto run = &this->fMaster->run(runIndex);
        auto intersection = intersectedSize(run->textRange(), textRange);
        if (intersection < 0 || (intersection == 0 && textRange.end != run->textRange().end)) {
            if (width > 0) {
                break;
            } else {
                continue;
            }
        }
        auto lineIntersection = intersected(run->textRange(), textRange);
        if (run->textRange().empty() || lineIntersection.empty()) {
            if (width > 0) {
                break;
            } else {
                continue;
            }
        }

        runOffset += width;

        // Measure the text
        ClipContext clipContext = this->measureTextInsideOneRun(lineIntersection, run, runOffset, false);
        if (clipContext.clip.height() == 0) {
            continue;
        }

        if (!visitor(clipContext, runOffset, lineIntersection)) {
            break;
        }

        width = clipContext.clip.width();
    }

    // The last run
    runOffset += width;

    if (this->ellipsis() != nullptr) {
        auto ellipsis = this->ellipsis();
        ClipContext clipContext;
        clipContext.run = ellipsis;
        clipContext.shift = runOffset;
        clipContext.size = ellipsis->size();
        clipContext.pos = 0;
        clipContext.clip = SkRect::MakeXYWH(0, 0, ellipsis->advance().fX, ellipsis->advance().fY);
        if (visitor(clipContext, 0, ellipsis->textRange())) {
            width = ellipsis->clip().width();
            runOffset += width;
        }
    }

    // This is a very important assert!
    // It asserts that 2 different ways of calculation come with the same results
    if (!includingGhostSpaces && !SkScalarNearlyEqual(runOffset, this->width())) {
        SkDebugf("ASSERT: %f != %f\n", runOffset, this->width());
        SkASSERT(false);
    }

    return width;
}

SkVector TextLine::offset() const {
    return fOffset + SkVector::Make(fShift, 0);
}
}  // namespace textlayout
}  // namespace skia
