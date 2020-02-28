// Copyright 2019 Google LLC.
#include "modules/skparagraph/src/TextLine.h"
#include <unicode/brkiter.h>
#include <unicode/ubidi.h>
#include "modules/skparagraph/src/ParagraphImpl.h"

#include "include/core/SkMaskFilter.h"
#include "include/effects/SkDashPathEffect.h"
#include "include/effects/SkDiscretePathEffect.h"

namespace skia {
namespace textlayout {

namespace {

// TODO: deal with all the intersection functionality
TextRange intersected(const TextRange& a, const TextRange& b) {
    if (a.start == b.start && a.end == b.end) return a;
    auto begin = std::max(a.start, b.start);
    auto end = std::min(a.end, b.end);
    return end >= begin ? TextRange(begin, end) : EMPTY_TEXT;
}

SkScalar littleRound(SkScalar a) {
    // This rounding is done to match Flutter tests. Must be removed..
  return SkScalarRoundToScalar(a * 100.0)/100.0;
}

int compareRound(SkScalar a, SkScalar b) {
    // There is a rounding error that gets bigger when maxWidth gets bigger
    // VERY long zalgo text (> 100000) on a VERY long line (> 10000)
    // Canvas scaling affects it
    // Letter spacing affects it
    // It has to be relative to be useful
    auto base = std::max(SkScalarAbs(a), SkScalarAbs(b));
    auto diff = SkScalarAbs(a - b);
    if (nearlyZero(base) || diff / base < 0.001f) {
        return 0;
    }

    auto ra = littleRound(a);
    auto rb = littleRound(b);
    if (ra < rb) {
        return -1;
    } else {
        return 1;
    }
}

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
                   InternalLineMetrics sizes)
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
        fMaxRunMetrics.add(InternalLineMetrics(run.fFontMetrics.fAscent, run.fFontMetrics.fDescent, run.fFontMetrics.fLeading));
    }

    std::vector<int32_t> logicalOrder(numRuns);
    ubidi_reorderVisual(runLevels.data(), SkToU32(numRuns), logicalOrder.data());

    auto firstRunIndex = start.runIndex();
    for (auto index : logicalOrder) {
        fRunsInVisualOrder.push_back(firstRunIndex + index);
    }

    // TODO: This is the fix for flutter. Must be removed...
    for (auto cluster = &start; cluster != &end; ++cluster) {
        if (!cluster->run()->isPlaceholder()) {
            fShift += cluster->getHalfLetterSpacing();
            break;
        }
    }
}

SkRect TextLine::calculateBoundaries() {

    auto boundaries = SkRect::MakeIWH(fAdvance.fX, fAdvance.fY);
    auto clusters = fMaster->clusters(fClusterRange);
    Run* run = nullptr;
    auto runShift = 0.0f;
    auto clusterShift = 0.0f;
    for (auto cluster = clusters.begin(); cluster != clusters.end(); ++cluster) {
        if (run == nullptr || cluster->runIndex() != run->index()) {
            run = &fMaster->run(cluster->runIndex());
            runShift += clusterShift;
            clusterShift = 0;
        }
        clusterShift += cluster->width();
        for (auto i = cluster->startPos(); i < cluster->endPos(); ++i) {
            auto posX = run->positionX(i);
            auto posY = run->posY(i);
            auto bounds = run->getBounds(i);
            bounds.offset(posX + runShift, posY);
            boundaries.joinPossiblyEmptyRect(bounds);
        }
    }

    // We need to take in account all the shadows when we calculate the boundaries
    // TODO: Need to find a better solution
    if (fHasShadows) {
        SkRect shadowRect = SkRect::MakeEmpty();
        this->iterateThroughVisualRuns(false,
            [this, &shadowRect, boundaries]
            (const Run* run, SkScalar runOffsetInLine, TextRange textRange, SkScalar* runWidthInLine) {
            *runWidthInLine = this->iterateThroughSingleRunByStyles(
                run, runOffsetInLine, textRange, StyleType::kShadow,
                [&shadowRect, boundaries](TextRange textRange, const TextStyle& style, const ClipContext& context) {

                    for (TextShadow shadow : style.getShadows()) {
                        if (!shadow.hasShadow()) continue;
                        SkPaint paint;
                        paint.setColor(shadow.fColor);
                        if (shadow.fBlurRadius != 0.0) {
                            auto filter = SkMaskFilter::MakeBlur(
                                    kNormal_SkBlurStyle,
                                    SkDoubleToScalar(shadow.fBlurRadius),
                                    false);
                            paint.setMaskFilter(filter);
                            SkRect bound;
                            paint.doComputeFastBounds(boundaries, &bound, SkPaint::Style::kFill_Style);
                            shadowRect.joinPossiblyEmptyRect(bound);
                        }
                    }
                });
            return true;
            });
        boundaries.fLeft += shadowRect.fLeft;
        boundaries.fTop += shadowRect.fTop;
        boundaries.fRight += shadowRect.fRight;
        boundaries.fBottom += shadowRect.fBottom;
    }

    boundaries.offset(this->fOffset);         // Line offset from the beginning of the para
    boundaries.offset(this->fShift, 0);     // Shift produced by formatting
    boundaries.offset(0, this->baseline()); // Down by baseline

    return boundaries;
}

void TextLine::paint(SkCanvas* textCanvas) {
    if (this->empty()) {
        return;
    }

    textCanvas->save();
    textCanvas->translate(this->offset().fX, this->offset().fY);

    if (fHasBackground) {
        this->iterateThroughVisualRuns(false,
            [textCanvas, this]
            (const Run* run, SkScalar runOffsetInLine, TextRange textRange, SkScalar* runWidthInLine) {
                *runWidthInLine = this->iterateThroughSingleRunByStyles(
                run, runOffsetInLine, textRange, StyleType::kBackground,
                [textCanvas, this](TextRange textRange, const TextStyle& style, const ClipContext& context) {
                    this->paintBackground(textCanvas, textRange, style, context);
                });
            return true;
            });
    }

    if (fHasShadows) {
        this->iterateThroughVisualRuns(false,
            [textCanvas, this]
            (const Run* run, SkScalar runOffsetInLine, TextRange textRange, SkScalar* runWidthInLine) {
            *runWidthInLine = this->iterateThroughSingleRunByStyles(
                run, runOffsetInLine, textRange, StyleType::kShadow,
                [textCanvas, this](TextRange textRange, const TextStyle& style, const ClipContext& context) {
                    this->paintShadow(textCanvas, textRange, style, context);
                });
            return true;
            });
    }

    this->iterateThroughVisualRuns(false,
            [textCanvas, this]
            (const Run* run, SkScalar runOffsetInLine, TextRange textRange, SkScalar* runWidthInLine) {
            if (run->placeholderStyle() != nullptr) {
                *runWidthInLine = run->advance().fX;
                return true;
            }
            *runWidthInLine = this->iterateThroughSingleRunByStyles(
            run, runOffsetInLine, textRange, StyleType::kForeground,
            [textCanvas, this](TextRange textRange, const TextStyle& style, const ClipContext& context) {
                this->paintText(textCanvas, textRange, style, context);
            });
            return true;
        });

    if (fHasDecorations) {
        this->iterateThroughVisualRuns(false,
            [textCanvas, this]
            (const Run* run, SkScalar runOffsetInLine, TextRange textRange, SkScalar* runWidthInLine) {
                *runWidthInLine = this->iterateThroughSingleRunByStyles(
                run, runOffsetInLine, textRange, StyleType::kDecorations,
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

    return this->fMaster->paragraphStyle().getTextDirection() == TextDirection::kLtr
                ? TextAlign::kLeft
                : TextAlign::kRight;
}

void TextLine::scanStyles(StyleType styleType, const RunStyleVisitor& visitor) {
    if (this->empty()) {
        return;
    }

    this->iterateThroughVisualRuns(false,
        [this, visitor, styleType](const Run* run, SkScalar runOffset, TextRange textRange, SkScalar* width) {
            *width = this->iterateThroughSingleRunByStyles(
                run, runOffset, textRange, styleType,
                [visitor](TextRange textRange, const TextStyle& style, const ClipContext& context) {
                    visitor(textRange, style, context);
                });
            return true;
        });
}

SkRect TextLine::extendHeight(const ClipContext& context) const {
    SkRect result = context.clip;
    result.fBottom += std::max(this->fMaxRunMetrics.height() - this->height(), 0.0f);
    return result;
}

void TextLine::paintText(SkCanvas* canvas, TextRange textRange, const TextStyle& style, const ClipContext& context) const {

    if (context.run->placeholderStyle() != nullptr) {
        return;
    }

    SkPaint paint;
    if (style.hasForeground()) {
        paint = style.getForeground();
    } else {
        paint.setColor(style.getColor());
    }

    // TODO: This is the change for flutter, must be removed later
    SkScalar correctedBaseline = SkScalarFloorToScalar(this->baseline() + 0.5);
    SkTextBlobBuilder builder;
    context.run->copyTo(builder, SkToU32(context.pos), context.size, SkVector::Make(0, correctedBaseline));
    canvas->save();
    if (context.clippingNeeded) {
        canvas->clipRect(extendHeight(context));
    }

    canvas->translate(context.fTextShift, 0);
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
            canvas->clipRect(extendHeight(context));
        }
        canvas->translate(context.fTextShift, 0);
        canvas->drawTextBlob(builder.make(), shadow.fOffset.x(), shadow.fOffset.y(), paint);
        canvas->restore();
    }
}

static const float kDoubleDecorationSpacing = 3.0f;
void TextLine::paintDecorations(SkCanvas* canvas, TextRange textRange, const TextStyle& style, const ClipContext& context) const {
    if (style.getDecorationType() == TextDecoration::kNoDecoration) {
        return;
    }

    canvas->save();


    SkPaint paint;
    paint.setStyle(SkPaint::kStroke_Style);
    if (style.getDecorationColor() == SK_ColorTRANSPARENT) {
        paint.setColor(style.getColor());
    } else {
        paint.setColor(style.getDecorationColor());
    }
    paint.setAntiAlias(true);

    SkFontMetrics fontMetrics;
    TextStyle combined = style;
    combined.setTypeface(context.run->fFont.refTypeface());
    combined.getFontMetrics(&fontMetrics);
    SkScalar thickness;
    if ((fontMetrics.fFlags & SkFontMetrics::FontMetricsFlags::kUnderlineThicknessIsValid_Flag) &&
         fontMetrics.fUnderlineThickness > 0) {
        thickness = fontMetrics.fUnderlineThickness;
    } else {
        thickness = style.getFontSize() / 14.0f;
    }

    paint.setStrokeWidth(thickness * style.getDecorationThicknessMultiplier());

    for (auto decoration : AllTextDecorations) {
        if ((style.getDecorationType() & decoration) == 0) {
            continue;
        }

        SkScalar position = 0;
        switch (decoration) {
            case TextDecoration::kUnderline:
                if ((fontMetrics.fFlags & SkFontMetrics::FontMetricsFlags::kUnderlinePositionIsValid_Flag) &&
                     fontMetrics.fUnderlinePosition > 0) {
                    position = fontMetrics.fUnderlinePosition;
                } else {
                    position = thickness;
                }
                position += - context.run->correctAscent();
                break;
            case TextDecoration::kOverline:
                position = 0;
                break;
            case TextDecoration::kLineThrough: {
                if ((fontMetrics.fFlags & SkFontMetrics::FontMetricsFlags::kStrikeoutThicknessIsValid_Flag) &&
                     fontMetrics.fStrikeoutThickness > 0) {
                    paint.setStrokeWidth(fontMetrics.fStrikeoutThickness * style.getDecorationThicknessMultiplier());
                }
                position = (fontMetrics.fFlags & SkFontMetrics::FontMetricsFlags::kStrikeoutThicknessIsValid_Flag)
                            ? fontMetrics.fStrikeoutPosition
                            : fontMetrics.fXHeight / -2;
                position += - context.run->correctAscent();
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
        SkPath path;
        this->computeDecorationPaint(paint, context.clip, style, thickness, path);

        switch (style.getDecorationStyle()) {
            case TextDecorationStyle::kWavy:
                path.offset(x, y);
                canvas->drawPath(path, paint);
                break;
            case TextDecorationStyle::kDouble: {
                canvas->drawLine(x, y, x + width, y, paint);
                SkScalar bottom = y + kDoubleDecorationSpacing;
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

    canvas->restore();
}

void TextLine::computeDecorationPaint(SkPaint& paint,
                                      SkRect clip,
                                      const TextStyle& style,
                                      SkScalar thickness,
                                      SkPath& path) const {
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
            SkScalar quarterWave = thickness * style.getDecorationThicknessMultiplier();
            path.moveTo(0, 0);
            while (x_start + quarterWave * 2 < clip.width()) {
                path.rQuadTo(quarterWave,
                             wave_count % 2 != 0 ? quarterWave : -quarterWave,
                             quarterWave * 2,
                             0);
                x_start += quarterWave * 2;
                ++wave_count;
            }

            // The rest of the wave
            auto remaining = clip.width() - x_start;
            if (remaining > 0) {
                double x1 = remaining / 2;
                double y1 = remaining / 2 * (wave_count % 2 == 0 ? -1 : 1);
                double x2 = remaining;
                double y2 = (remaining - remaining * remaining / (quarterWave * 2)) *
                            (wave_count % 2 == 0 ? -1 : 1);
                path.rQuadTo(x1, y1, x2, y2);
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
                shiftCluster(cluster, ghostShift, ghostShift);
            }
            return true;
        }

        auto prevShift = shift;
        if (cluster->isWhitespaces()) {
            if (!whitespacePatch) {
                shift += step;
                whitespacePatch = true;
                --whitespacePatches;
            }
        } else {
            whitespacePatch = false;
        }
        shiftCluster(cluster, shift, prevShift);
        return true;
    });

    SkAssertResult(nearlyEqual(shift, maxWidth - textLen));
    SkASSERT(whitespacePatches == 0);

    this->fWidthWithSpaces += ghostShift;
    this->fAdvance.fX = maxWidth;
}

void TextLine::shiftCluster(const Cluster* cluster, SkScalar shift, SkScalar prevShift) {

    auto run = cluster->run();
    auto start = cluster->startPos();
    auto end = cluster->endPos();

    if (end == run->size()) {
        // Set the same shift for the fake last glyph (to avoid all extra checks)
        ++end;
    }

    if (run->fJustificationShifts.empty()) {
        // Do not fill this array until needed
        run->fJustificationShifts.push_back_n(run->size() + 1, { 0, 0 });
    }

    for (size_t pos = start; pos < end; ++pos) {
        run->fJustificationShifts[pos] = { shift, prevShift };
    }
}

void TextLine::createEllipsis(SkScalar maxWidth, const SkString& ellipsis, bool) {
    // Replace some clusters with the ellipsis
    // Go through the clusters in the reverse logical order
    // taking off cluster by cluster until the ellipsis fits
    SkScalar width = fAdvance.fX;

    auto attachEllipsis = [&](const Cluster* cluster){
        // Shape the ellipsis
        Run* run = shapeEllipsis(ellipsis, cluster->run());
        run->fClusterStart = cluster->textRange().start;
        run->setMaster(fMaster);

        // See if it fits
        if (width + run->advance().fX > maxWidth) {
            width -= cluster->width();
            // Continue if it's not
            return false;
        }

        fEllipsis = std::make_shared<Run>(*run);
        fEllipsis->shift(width, 0);
        fAdvance.fX = width;
        return true;
    };

    iterateThroughClustersInGlyphsOrder(
        true, false, [&](const Cluster* cluster, ClusterIndex index, bool leftToRight, bool ghost) {
            return !attachEllipsis(cluster);
        });

    if (!fEllipsis) {
        // Weird situation: just the ellipsis on the line (if it fits)
        attachEllipsis(&fMaster->cluster(clusters().start));
    }
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
            fRun->fPlaceholderIndex = std::numeric_limits<size_t>::max();
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

TextLine::ClipContext TextLine::measureTextInsideOneRun(TextRange textRange,
                                                        const Run* run,
                                                        SkScalar runOffsetInLine,
                                                        SkScalar textOffsetInRunInLine,
                                                        bool includeGhostSpaces,
                                                        bool limitToClusters) const {
    ClipContext result = { run, 0, run->size(), 0, SkRect::MakeEmpty(), false };

    if (run->placeholderStyle() != nullptr || run->fEllipsis) {
        // Both ellipsis and placeholders can only be measured as one glyph
        SkASSERT(textRange == run->textRange());
        result.fTextShift = runOffsetInLine;
        result.clip = SkRect::MakeXYWH(runOffsetInLine, sizes().runTop(run), run->advance().fX, run->calculateHeight());
        return result;
    }

    // Find [start:end] clusters for the text
    bool found;
    ClusterIndex startIndex;
    ClusterIndex endIndex;
    std::tie(found, startIndex, endIndex) = run->findLimitingClusters(textRange, limitToClusters);
    if (!found) {
        SkASSERT(textRange.empty() || limitToClusters);
        return result;
    }

    auto start = &fMaster->cluster(startIndex);
    auto end = &fMaster->cluster(endIndex);
    result.pos = start->startPos();
    result.size = (end->isHardBreak() ? end->startPos() : end->endPos()) - start->startPos();

    auto textStartInRun = run->positionX(start->startPos());
    auto textStartInLine = runOffsetInLine + textOffsetInRunInLine;
/*
    if (!run->fJustificationShifts.empty()) {
        SkDebugf("Justification for [%d:%d)\n", textRange.start, textRange.end);
        for (auto i = result.pos; i < result.pos + result.size; ++i) {
            auto j = run->fJustificationShifts[i];
            SkDebugf("[%d] = %f %f\n", i, j.fX, j.fY);
        }
    }
*/
    // Calculate the clipping rectangle for the text with cluster edges
    // There are 2 cases:
    // EOL (when we expect the last cluster clipped without any spaces)
    // Anything else (when we want the cluster width contain all the spaces -
    // coming from letter spacing or word spacing or justification)
    result.clip =
            SkRect::MakeXYWH(0,
                             sizes().runTop(run),
                             run->calculateWidth(result.pos, result.pos + result.size, false),
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

    textStartInLine -= leftCorrection;
    result.clip.offset(textStartInLine, 0);

    if (compareRound(result.clip.fRight, fAdvance.fX) > 0 && !includeGhostSpaces) {
        // There are few cases when we need it.
        // The most important one: we measure the text with spaces at the end
        // and we should ignore these spaces
        result.clippingNeeded = true;
        result.clip.fRight = fAdvance.fX;
    }

    // The text must be aligned with the lineOffset
    result.fTextShift = textStartInLine - textStartInRun;

    return result;
}

void TextLine::iterateThroughClustersInGlyphsOrder(bool reverse,
                                                   bool includeGhosts,
                                                   const ClustersVisitor& visitor) const {
    // Walk through the clusters in the logical order (or reverse)
    for (size_t r = 0; r != fRunsInVisualOrder.size(); ++r) {
        auto& runIndex = fRunsInVisualOrder[reverse ? fRunsInVisualOrder.size() - r - 1 : r];
        auto run = this->fMaster->runs().begin() + runIndex;
        auto start = std::max(run->clusterRange().start, fClusterRange.start);
        auto end = std::min(run->clusterRange().end, fClusterRange.end);
        auto ghosts = std::min(run->clusterRange().end, fGhostClusterRange.end);

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

SkScalar TextLine::iterateThroughSingleRunByStyles(const Run* run,
                                                   SkScalar runOffset,
                                                   TextRange textRange,
                                                   StyleType styleType,
                                                   const RunStyleVisitor& visitor) const {

    if (run->fEllipsis) {
        // Extra efforts to get the ellipsis text style
        ClipContext clipContext = this->measureTextInsideOneRun(run->textRange(), run, runOffset, 0, false, false);
        TextRange testRange(run->fClusterStart, run->fClusterStart + 1);
        for (BlockIndex index = fBlockRange.start; index < fBlockRange.end; ++index) {
           auto block = fMaster->styles().begin() + index;
           auto intersect = intersected(block->fRange, testRange);
           if (intersect.width() > 0) {
               visitor(textRange, block->fStyle, clipContext);
               return run->advance().fX;
           }
        }
        SkASSERT(false);
    }

    if (styleType == StyleType::kNone) {
        ClipContext clipContext = this->measureTextInsideOneRun(textRange, run, runOffset, 0, false, false);
        if (clipContext.clip.height() > 0) {
            visitor(textRange, TextStyle(), clipContext);
            return clipContext.clip.width();
        } else {
            return 0;
        }
    }

    TextIndex start = EMPTY_INDEX;
    size_t size = 0;
    const TextStyle* prevStyle = nullptr;
    SkScalar textOffsetInRun = 0;
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
        } else {
            break;
        }

        // We have the style and the text
        auto textRange = TextRange(start, start + size);
        // Measure the text
        ClipContext clipContext = this->measureTextInsideOneRun(textRange, run, runOffset, textOffsetInRun, false, false);
        if (clipContext.clip.height() == 0) {
            continue;
        }
        visitor(textRange, *prevStyle, clipContext);
        textOffsetInRun += clipContext.clip.width();

        // Start all over again
        prevStyle = style;
        start = intersect.start;
        size = intersect.width();
    }
    return textOffsetInRun;
}

void TextLine::iterateThroughVisualRuns(bool includingGhostSpaces, const RunVisitor& visitor) const {

    // Walk through all the runs that intersect with the line in visual order
    SkScalar width = 0;
    SkScalar runOffset = 0;
    SkScalar totalWidth = 0;
    auto textRange = includingGhostSpaces ? this->textWithSpaces() : this->trimmedText();
    for (auto& runIndex : fRunsInVisualOrder) {

        const auto run = &this->fMaster->run(runIndex);
        auto lineIntersection = intersected(run->textRange(), textRange);
        if (lineIntersection.width() == 0 && this->width() != 0) {
            // TODO: deal with empty runs in a better way
            continue;
        }
        if (!run->leftToRight() && runOffset == 0 && includingGhostSpaces) {
            // runOffset does not take in account a possibility
            // that RTL run could start before the line (trailing spaces)
            // so we need to do runOffset -= "trailing whitespaces length"
            TextRange whitespaces = intersected(
                    TextRange(fTextRange.end, fTextWithWhitespacesRange.end), run->fTextRange);
            if (whitespaces.width() > 0) {
                auto whitespacesLen = measureTextInsideOneRun(whitespaces, run, runOffset, 0, true, false).clip.width();
                runOffset -= whitespacesLen;
            }
        }
        runOffset += width;
        totalWidth += width;
        if (!visitor(run, runOffset, lineIntersection, &width)) {
            return;
        }
    }

    runOffset += width;
    totalWidth += width;

    if (this->ellipsis() != nullptr) {
        if (visitor(ellipsis(), runOffset, ellipsis()->textRange(), &width)) {
            totalWidth += width;
        }
    }

    // This is a very important assert!
    // It asserts that 2 different ways of calculation come with the same results
    if (!includingGhostSpaces && compareRound(totalWidth, this->width()) != 0) {
        SkDebugf("ASSERT: %f != %f\n", totalWidth, this->width());
        SkASSERT(false);
    }
}

SkVector TextLine::offset() const {
    return fOffset + SkVector::Make(fShift, 0);
}

LineMetrics TextLine::getMetrics() const {
    LineMetrics result;

    // Fill out the metrics
    result.fStartIndex = fTextRange.start;
    result.fEndIndex = fTextWithWhitespacesRange.end;
    result.fEndExcludingWhitespaces = fTextRange.end;
    result.fEndIncludingNewline = fTextWithWhitespacesRange.end; // TODO: implement
    // TODO: For some reason Flutter imagines a hard line break at the end of the last line.
    //  To be removed...
    result.fHardBreak = fMaster->cluster(fGhostClusterRange.end - 1).isHardBreak() ||
                        fGhostClusterRange.end == fMaster->clusters().size() - 1;
    result.fAscent = - fMaxRunMetrics.ascent();
    result.fDescent = fMaxRunMetrics.descent();
    result.fUnscaledAscent = - fMaxRunMetrics.ascent(); // TODO: implement
    result.fHeight = littleRound(fAdvance.fY);
    result.fWidth = littleRound(fAdvance.fX);
    result.fLeft = fOffset.fX;
    result.fBaseline = fMaxRunMetrics.baseline() + (this - fMaster->lines().begin()) * result.fHeight;
    result.fLineNumber = this - fMaster->lines().begin();

    // Fill out the style parts
    this->iterateThroughVisualRuns(false,
        [this, &result]
        (const Run* run, SkScalar runOffsetInLine, TextRange textRange, SkScalar* runWidthInLine) {
        if (run->placeholderStyle() != nullptr) {
            *runWidthInLine = run->advance().fX;
            return true;
        }
        *runWidthInLine = this->iterateThroughSingleRunByStyles(
        run, runOffsetInLine, textRange, StyleType::kForeground,
        [&result, &run](TextRange textRange, const TextStyle& style, const ClipContext& context) {
            SkFontMetrics fontMetrics;
            run->fFont.getMetrics(&fontMetrics);
            StyleMetrics styleMetrics(&style, fontMetrics);
            result.fLineMetrics.emplace(textRange.start, styleMetrics);
        });
        return true;
    });

    return result;
}
}  // namespace textlayout
}  // namespace skia
