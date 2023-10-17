// Copyright 2019 Google LLC.

#include "include/core/SkBlurTypes.h"
#include "include/core/SkFont.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkMaskFilter.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSpan.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "include/core/SkTypes.h"
#include "include/private/base/SkTemplates.h"
#include "include/private/base/SkTo.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/Metrics.h"
#include "modules/skparagraph/include/ParagraphPainter.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextShadow.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/Decorations.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/ParagraphPainterImpl.h"
#include "modules/skparagraph/src/TextLine.h"
#include "modules/skshaper/include/SkShaper.h"

#include <algorithm>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

using namespace skia_private;

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

TextRange operator*(const TextRange& a, const TextRange& b) {
    if (a.start == b.start && a.end == b.end) return a;
    auto begin = std::max(a.start, b.start);
    auto end = std::min(a.end, b.end);
    return end > begin ? TextRange(begin, end) : EMPTY_TEXT;
}

int compareRound(SkScalar a, SkScalar b, bool applyRoundingHack) {
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

    auto ra = a;
    auto rb = b;

    if (applyRoundingHack) {
        ra = littleRound(a);
        rb = littleRound(b);
    }
    if (ra < rb) {
        return -1;
    } else {
        return 1;
    }
}

}  // namespace

TextLine::TextLine(ParagraphImpl* owner,
                   SkVector offset,
                   SkVector advance,
                   BlockRange blocks,
                   TextRange textExcludingSpaces,
                   TextRange text,
                   TextRange textIncludingNewlines,
                   ClusterRange clusters,
                   ClusterRange clustersWithGhosts,
                   SkScalar widthWithSpaces,
                   InternalLineMetrics sizes)
        : fOwner(owner)
        , fBlockRange(blocks)
        , fTextExcludingSpaces(textExcludingSpaces)
        , fText(text)
        , fTextIncludingNewlines(textIncludingNewlines)
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
        , fHasDecorations(false)
        , fAscentStyle(LineMetricStyle::CSS)
        , fDescentStyle(LineMetricStyle::CSS)
        , fTextBlobCachePopulated(false) {
    // Reorder visual runs
    auto& start = owner->cluster(fGhostClusterRange.start);
    auto& end = owner->cluster(fGhostClusterRange.end - 1);
    size_t numRuns = end.runIndex() - start.runIndex() + 1;

    for (BlockIndex index = fBlockRange.start; index < fBlockRange.end; ++index) {
        auto b = fOwner->styles().begin() + index;
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

    // This is just chosen to catch the common/fast cases. Feel free to tweak.
    constexpr int kPreallocCount = 4;
    AutoSTArray<kPreallocCount, SkUnicode::BidiLevel> runLevels(numRuns);
    std::vector<RunIndex> placeholdersInOriginalOrder;
    size_t runLevelsIndex = 0;
    // Placeholders must be laid out using the original order in which they were added
    // in the input. The API does not provide a way to indicate that a placeholder
    // position was moved due to bidi reordering.
    for (auto runIndex = start.runIndex(); runIndex <= end.runIndex(); ++runIndex) {
        auto& run = fOwner->run(runIndex);
        runLevels[runLevelsIndex++] = run.fBidiLevel;
        fMaxRunMetrics.add(
            InternalLineMetrics(run.correctAscent(), run.correctDescent(), run.fFontMetrics.fLeading));
        if (run.isPlaceholder()) {
            placeholdersInOriginalOrder.push_back(runIndex);
        }
    }
    SkASSERT(runLevelsIndex == numRuns);

    AutoSTArray<kPreallocCount, int32_t> logicalOrder(numRuns);

    // TODO: hide all these logic in SkUnicode?
    fOwner->getUnicode()->reorderVisual(runLevels.data(), numRuns, logicalOrder.data());
    auto firstRunIndex = start.runIndex();
    auto placeholderIter = placeholdersInOriginalOrder.begin();
    for (auto index : logicalOrder) {
        auto runIndex = firstRunIndex + index;
        if (fOwner->run(runIndex).isPlaceholder()) {
            fRunsInVisualOrder.push_back(*placeholderIter++);
        } else {
            fRunsInVisualOrder.push_back(runIndex);
        }
    }

    // TODO: This is the fix for flutter. Must be removed...
    for (auto cluster = &start; cluster <= &end; ++cluster) {
        if (!cluster->run().isPlaceholder()) {
            fShift += cluster->getHalfLetterSpacing();
            break;
        }
    }
}

void TextLine::paint(ParagraphPainter* painter, SkScalar x, SkScalar y) {
    if (fHasBackground) {
        this->iterateThroughVisualRuns(false,
            [painter, x, y, this]
            (const Run* run, SkScalar runOffsetInLine, TextRange textRange, SkScalar* runWidthInLine) {
                *runWidthInLine = this->iterateThroughSingleRunByStyles(
                TextAdjustment::GlyphCluster, run, runOffsetInLine, textRange, StyleType::kBackground,
                [painter, x, y, this](TextRange textRange, const TextStyle& style, const ClipContext& context) {
                    this->paintBackground(painter, x, y, textRange, style, context);
                });
            return true;
            });
    }

    if (fHasShadows) {
        this->iterateThroughVisualRuns(false,
            [painter, x, y, this]
            (const Run* run, SkScalar runOffsetInLine, TextRange textRange, SkScalar* runWidthInLine) {
            *runWidthInLine = this->iterateThroughSingleRunByStyles(
                TextAdjustment::GlyphCluster, run, runOffsetInLine, textRange, StyleType::kShadow,
                [painter, x, y, this]
                (TextRange textRange, const TextStyle& style, const ClipContext& context) {
                    this->paintShadow(painter, x, y, textRange, style, context);
                });
            return true;
            });
    }

    this->ensureTextBlobCachePopulated();

    for (auto& record : fTextBlobCache) {
        record.paint(painter, x, y);
    }

    if (fHasDecorations) {
        this->iterateThroughVisualRuns(false,
            [painter, x, y, this]
            (const Run* run, SkScalar runOffsetInLine, TextRange textRange, SkScalar* runWidthInLine) {
                *runWidthInLine = this->iterateThroughSingleRunByStyles(
                TextAdjustment::GlyphCluster, run, runOffsetInLine, textRange, StyleType::kDecorations,
                [painter, x, y, this]
                (TextRange textRange, const TextStyle& style, const ClipContext& context) {
                    this->paintDecorations(painter, x, y, textRange, style, context);
                });
                return true;
        });
    }
}

void TextLine::ensureTextBlobCachePopulated() {
    if (fTextBlobCachePopulated) {
        return;
    }
    if (fBlockRange.width() == 1 &&
        fRunsInVisualOrder.size() == 1 &&
        fEllipsis == nullptr &&
        fOwner->run(fRunsInVisualOrder[0]).placeholderStyle() == nullptr) {
        if (fClusterRange.width() == 0) {
            return;
        }
        // Most common and most simple case
        const auto& style = fOwner->block(fBlockRange.start).fStyle;
        const auto& run = fOwner->run(fRunsInVisualOrder[0]);
        auto clip = SkRect::MakeXYWH(0.0f, this->sizes().runTop(&run, this->fAscentStyle),
                                     fAdvance.fX,
                                     run.calculateHeight(this->fAscentStyle, this->fDescentStyle));

        auto& start = fOwner->cluster(fClusterRange.start);
        auto& end = fOwner->cluster(fClusterRange.end - 1);
        SkASSERT(start.runIndex() == end.runIndex());
        GlyphRange glyphs;
        if (run.leftToRight()) {
            glyphs = GlyphRange(start.startPos(),
                                end.isHardBreak() ? end.startPos() : end.endPos());
        } else {
            glyphs = GlyphRange(end.startPos(),
                                start.isHardBreak() ? start.startPos() : start.endPos());
        }
        ClipContext context = {/*run=*/&run,
                               /*pos=*/glyphs.start,
                               /*size=*/glyphs.width(),
                               /*fTextShift=*/-run.positionX(glyphs.start), // starting position
                               /*clip=*/clip,                               // entire line
                               /*fExcludedTrailingSpaces=*/0.0f,            // no need for that
                               /*clippingNeeded=*/false};                   // no need for that
        this->buildTextBlob(fTextExcludingSpaces, style, context);
    } else {
        this->iterateThroughVisualRuns(false,
           [this](const Run* run,
                  SkScalar runOffsetInLine,
                  TextRange textRange,
                  SkScalar* runWidthInLine) {
               if (run->placeholderStyle() != nullptr) {
                   *runWidthInLine = run->advance().fX;
                   return true;
               }
               *runWidthInLine = this->iterateThroughSingleRunByStyles(
                   TextAdjustment::GlyphCluster,
                   run,
                   runOffsetInLine,
                   textRange,
                   StyleType::kForeground,
                   [this](TextRange textRange, const TextStyle& style, const ClipContext& context) {
                       this->buildTextBlob(textRange, style, context);
                   });
               return true;
           });
    }
    fTextBlobCachePopulated = true;
}

void TextLine::format(TextAlign align, SkScalar maxWidth) {
    SkScalar delta = maxWidth - this->width();
    if (delta <= 0) {
        return;
    }

    // We do nothing for left align
    if (align == TextAlign::kJustify) {
        if (!this->endsWithHardLineBreak()) {
            this->justify(maxWidth);
        } else if (fOwner->paragraphStyle().getTextDirection() == TextDirection::kRtl) {
            // Justify -> Right align
            fShift = delta;
        }
    } else if (align == TextAlign::kRight) {
        fShift = delta;
    } else if (align == TextAlign::kCenter) {
        fShift = delta / 2;
    }
}

void TextLine::scanStyles(StyleType styleType, const RunStyleVisitor& visitor) {
    if (this->empty()) {
        return;
    }

    this->iterateThroughVisualRuns(
            false,
            [this, visitor, styleType](
                    const Run* run, SkScalar runOffset, TextRange textRange, SkScalar* width) {
                *width = this->iterateThroughSingleRunByStyles(
                        TextAdjustment::GlyphCluster,
                        run,
                        runOffset,
                        textRange,
                        styleType,
                        [visitor](TextRange textRange,
                                  const TextStyle& style,
                                  const ClipContext& context) {
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

void TextLine::buildTextBlob(TextRange textRange, const TextStyle& style, const ClipContext& context) {
    if (context.run->placeholderStyle() != nullptr) {
        return;
    }

    fTextBlobCache.emplace_back();
    TextBlobRecord& record = fTextBlobCache.back();

    if (style.hasForeground()) {
        record.fPaint = style.getForegroundPaintOrID();
    } else {
        std::get<SkPaint>(record.fPaint).setColor(style.getColor());
    }
    record.fVisitor_Run = context.run;
    record.fVisitor_Pos = context.pos;

    // TODO: This is the change for flutter, must be removed later
    SkTextBlobBuilder builder;
    context.run->copyTo(builder, SkToU32(context.pos), context.size);
    record.fClippingNeeded = context.clippingNeeded;
    if (context.clippingNeeded) {
        record.fClipRect = extendHeight(context).makeOffset(this->offset());
    } else {
        record.fClipRect = context.clip.makeOffset(this->offset());
    }

    SkASSERT(nearlyEqual(context.run->baselineShift(), style.getBaselineShift()));
    SkScalar correctedBaseline = SkScalarFloorToScalar(this->baseline() + style.getBaselineShift() +  0.5);
    record.fBlob = builder.make();
    if (record.fBlob != nullptr) {
        record.fBounds.joinPossiblyEmptyRect(record.fBlob->bounds());
    }

    record.fOffset = SkPoint::Make(this->offset().fX + context.fTextShift,
                                   this->offset().fY + correctedBaseline);
}

void TextLine::TextBlobRecord::paint(ParagraphPainter* painter, SkScalar x, SkScalar y) {
    if (fClippingNeeded) {
        painter->save();
        painter->clipRect(fClipRect.makeOffset(x, y));
    }
    painter->drawTextBlob(fBlob, x + fOffset.x(), y + fOffset.y(), fPaint);
    if (fClippingNeeded) {
        painter->restore();
    }
}

void TextLine::paintBackground(ParagraphPainter* painter,
                               SkScalar x,
                               SkScalar y,
                               TextRange textRange,
                               const TextStyle& style,
                               const ClipContext& context) const {
    if (style.hasBackground()) {
        painter->drawRect(context.clip.makeOffset(this->offset() + SkPoint::Make(x, y)),
                          style.getBackgroundPaintOrID());
    }
}

void TextLine::paintShadow(ParagraphPainter* painter,
                           SkScalar x,
                           SkScalar y,
                           TextRange textRange,
                           const TextStyle& style,
                           const ClipContext& context) const {
    SkScalar correctedBaseline = SkScalarFloorToScalar(this->baseline() + style.getBaselineShift() + 0.5);

    for (TextShadow shadow : style.getShadows()) {
        if (!shadow.hasShadow()) continue;

        SkTextBlobBuilder builder;
        context.run->copyTo(builder, context.pos, context.size);

        if (context.clippingNeeded) {
            painter->save();
            SkRect clip = extendHeight(context);
            clip.offset(x, y);
            clip.offset(this->offset());
            painter->clipRect(clip);
        }
        auto blob = builder.make();
        painter->drawTextShadow(blob,
            x + this->offset().fX + shadow.fOffset.x() + context.fTextShift,
            y + this->offset().fY + shadow.fOffset.y() + correctedBaseline,
            shadow.fColor,
            SkDoubleToScalar(shadow.fBlurSigma));
        if (context.clippingNeeded) {
            painter->restore();
        }
    }
}

void TextLine::paintDecorations(ParagraphPainter* painter, SkScalar x, SkScalar y, TextRange textRange, const TextStyle& style, const ClipContext& context) const {
    ParagraphPainterAutoRestore ppar(painter);
    painter->translate(x + this->offset().fX, y + this->offset().fY + style.getBaselineShift());
    Decorations decorations;
    SkScalar correctedBaseline = SkScalarFloorToScalar(-this->sizes().rawAscent() + style.getBaselineShift() + 0.5);
    decorations.paint(painter, style, context, correctedBaseline);
}

void TextLine::justify(SkScalar maxWidth) {
    int whitespacePatches = 0;
    SkScalar textLen = 0;
    SkScalar whitespaceLen = 0;
    bool whitespacePatch = false;
    // Take leading whitespaces width but do not increment a whitespace patch number
    bool leadingWhitespaces = false;
    this->iterateThroughClustersInGlyphsOrder(false, false,
        [&](const Cluster* cluster, ClusterIndex index, bool ghost) {
            if (cluster->isWhitespaceBreak()) {
                if (index == 0) {
                    leadingWhitespaces = true;
                } else if (!whitespacePatch && !leadingWhitespaces) {
                    // We only count patches BETWEEN words, not before
                    ++whitespacePatches;
                }
                whitespacePatch = !leadingWhitespaces;
                whitespaceLen += cluster->width();
            } else if (cluster->isIdeographic()) {
                // Whitespace break before and after
                if (!whitespacePatch && index != 0) {
                    // We only count patches BETWEEN words, not before
                    ++whitespacePatches; // before
                }
                whitespacePatch = true;
                leadingWhitespaces = false;
                ++whitespacePatches;    // after
            } else {
                whitespacePatch = false;
                leadingWhitespaces = false;
            }
            textLen += cluster->width();
            return true;
        });

    if (whitespacePatch) {
        // We only count patches BETWEEN words, not after
        --whitespacePatches;
    }
    if (whitespacePatches == 0) {
        return;
    }

    SkScalar step = (maxWidth - textLen + whitespaceLen) / whitespacePatches;
    SkScalar shift = 0.0f;
    SkScalar prevShift = 0.0f;

    // Deal with the ghost spaces
    auto ghostShift = maxWidth - this->fAdvance.fX;
    // Spread the extra whitespaces
    whitespacePatch = false;
    // Do not break on leading whitespaces
    leadingWhitespaces = false;
    this->iterateThroughClustersInGlyphsOrder(false, true, [&](const Cluster* cluster, ClusterIndex index, bool ghost) {

        if (ghost) {
            if (cluster->run().leftToRight()) {
                this->shiftCluster(cluster, ghostShift, ghostShift);
            }
            return true;
        }

        if (cluster->isWhitespaceBreak()) {
            if (index == 0) {
                leadingWhitespaces = true;
            } else if (!whitespacePatch && !leadingWhitespaces) {
                shift += step;
                whitespacePatch = true;
                --whitespacePatches;
            }
            shift -= cluster->width();
        } else if (cluster->isIdeographic()) {
            if (!whitespacePatch && index != 0) {
                shift += step;
               --whitespacePatches;
            }
            whitespacePatch = false;
            leadingWhitespaces = false;
        } else {
            whitespacePatch = false;
            leadingWhitespaces = false;
        }
        this->shiftCluster(cluster, shift, prevShift);
        prevShift = shift;
        // We skip ideographic whitespaces
        if (!cluster->isWhitespaceBreak() && cluster->isIdeographic()) {
            shift += step;
            whitespacePatch = true;
            --whitespacePatches;
        }
        return true;
    });

    if (whitespacePatch && whitespacePatches < 0) {
        whitespacePatches++;
        shift -= step;
    }

    SkAssertResult(nearlyEqual(shift, maxWidth - textLen));
    SkASSERT(whitespacePatches == 0);

    this->fWidthWithSpaces += ghostShift;
    this->fAdvance.fX = maxWidth;
}

void TextLine::shiftCluster(const Cluster* cluster, SkScalar shift, SkScalar prevShift) {

    auto& run = cluster->run();
    auto start = cluster->startPos();
    auto end = cluster->endPos();

    if (end == run.size()) {
        // Set the same shift for the fake last glyph (to avoid all extra checks)
        ++end;
    }

    if (run.fJustificationShifts.empty()) {
        // Do not fill this array until needed
        run.fJustificationShifts.push_back_n(run.size() + 1, { 0, 0 });
    }

    for (size_t pos = start; pos < end; ++pos) {
        run.fJustificationShifts[pos] = { shift, prevShift };
    }
}

void TextLine::createEllipsis(SkScalar maxWidth, const SkString& ellipsis, bool) {
    // Replace some clusters with the ellipsis
    // Go through the clusters in the reverse logical order
    // taking off cluster by cluster until the ellipsis fits
    SkScalar width = fAdvance.fX;
    RunIndex lastRun = EMPTY_RUN;
    std::unique_ptr<Run> ellipsisRun;
    for (auto clusterIndex = fGhostClusterRange.end; clusterIndex > fGhostClusterRange.start; --clusterIndex) {
        auto& cluster = fOwner->cluster(clusterIndex - 1);
        // Shape the ellipsis if the run has changed
        if (lastRun != cluster.runIndex()) {
            ellipsisRun = this->shapeEllipsis(ellipsis, &cluster);
            if (ellipsisRun->advance().fX > maxWidth) {
                // Ellipsis is bigger than the entire line; no way we can add it at all
                // BUT! We can keep scanning in case the next run will give us better results
                lastRun = EMPTY_RUN;
                continue;
            } else {
                // We may need to continue
                lastRun = cluster.runIndex();
            }
        }
        // See if it fits
        if (width + ellipsisRun->advance().fX > maxWidth) {
            width -= cluster.width();
            // Continue if the ellipsis does not fit
            continue;
        }
        // We found enough room for the ellipsis
        fAdvance.fX = width;
        fEllipsis = std::move(ellipsisRun);
        fEllipsis->setOwner(fOwner);

        // Let's update the line
        fClusterRange.end = clusterIndex;
        fGhostClusterRange.end = fClusterRange.end;
        fEllipsis->fClusterStart = cluster.textRange().start;
        fText.end = cluster.textRange().end;
        fTextIncludingNewlines.end = cluster.textRange().end;
        fTextExcludingSpaces.end = cluster.textRange().end;
        break;
    }

    if (!fEllipsis) {
        // Weird situation: ellipsis does not fit; no ellipsis then
        fClusterRange.end = fClusterRange.start;
        fGhostClusterRange.end = fClusterRange.start;
        fText.end = fText.start;
        fTextIncludingNewlines.end = fTextIncludingNewlines.start;
        fTextExcludingSpaces.end = fTextExcludingSpaces.start;
        fAdvance.fX = 0;
    }
}

static inline SkUnichar nextUtf8Unit(const char** ptr, const char* end) {
    SkUnichar val = SkUTF::NextUTF8(ptr, end);
    return val < 0 ? 0xFFFD : val;
}

std::unique_ptr<Run> TextLine::shapeEllipsis(const SkString& ellipsis, const Cluster* cluster) {

    class ShapeHandler final : public SkShaper::RunHandler {
    public:
        ShapeHandler(SkScalar lineHeight, bool useHalfLeading, SkScalar baselineShift, const SkString& ellipsis)
            : fRun(nullptr), fLineHeight(lineHeight), fUseHalfLeading(useHalfLeading), fBaselineShift(baselineShift), fEllipsis(ellipsis) {}
        std::unique_ptr<Run> run() & { return std::move(fRun); }

    private:
        void beginLine() override {}

        void runInfo(const RunInfo&) override {}

        void commitRunInfo() override {}

        Buffer runBuffer(const RunInfo& info) override {
            SkASSERT(!fRun);
            fRun = std::make_unique<Run>(nullptr, info, 0, fLineHeight, fUseHalfLeading, fBaselineShift, 0, 0);
            return fRun->newRunBuffer();
        }

        void commitRunBuffer(const RunInfo& info) override {
            fRun->fAdvance.fX = info.fAdvance.fX;
            fRun->fAdvance.fY = fRun->advance().fY;
            fRun->fPlaceholderIndex = std::numeric_limits<size_t>::max();
            fRun->fEllipsis = true;
        }

        void commitLine() override {}

        std::unique_ptr<Run> fRun;
        SkScalar fLineHeight;
        bool fUseHalfLeading;
        SkScalar fBaselineShift;
        SkString fEllipsis;
    };

    const Run& run = cluster->run();
    TextStyle textStyle = fOwner->paragraphStyle().getTextStyle();
    for (auto i = fBlockRange.start; i < fBlockRange.end; ++i) {
        auto& block = fOwner->block(i);
        if (run.leftToRight() && cluster->textRange().end <= block.fRange.end) {
            textStyle = block.fStyle;
            break;
        } else if (!run.leftToRight() && cluster->textRange().start <= block.fRange.end) {
            textStyle = block.fStyle;
            break;
        }
    }

    auto shaped = [&](sk_sp<SkTypeface> typeface, bool fallback) -> std::unique_ptr<Run> {
        ShapeHandler handler(run.heightMultiplier(), run.useHalfLeading(), run.baselineShift(), ellipsis);
        SkFont font(std::move(typeface), textStyle.getFontSize());
        font.setEdging(SkFont::Edging::kAntiAlias);
        font.setHinting(SkFontHinting::kSlight);
        font.setSubpixel(true);

        std::unique_ptr<SkShaper> shaper = SkShaper::MakeShapeDontWrapOrReorder(
                            fOwner->getUnicode()->copy(),
                            fallback ? SkFontMgr::RefDefault() : SkFontMgr::RefEmpty());
        shaper->shape(ellipsis.c_str(),
                      ellipsis.size(),
                      font,
                      true,
                      std::numeric_limits<SkScalar>::max(),
                      &handler);
        auto ellipsisRun = handler.run();
        ellipsisRun->fTextRange = TextRange(0, ellipsis.size());
        ellipsisRun->fOwner = fOwner;
        return ellipsisRun;
    };

    // Check the current font
    auto ellipsisRun = shaped(run.fFont.refTypeface(), false);
    if (ellipsisRun->isResolved()) {
        return ellipsisRun;
    }

    // Check all allowed fonts
    std::vector<sk_sp<SkTypeface>> typefaces = fOwner->fontCollection()->findTypefaces(
            textStyle.getFontFamilies(), textStyle.getFontStyle(), textStyle.getFontArguments());
    for (const auto& typeface : typefaces) {
        ellipsisRun = shaped(typeface, false);
        if (ellipsisRun->isResolved()) {
            return ellipsisRun;
        }
    }

    // Try the fallback
    if (fOwner->fontCollection()->fontFallbackEnabled()) {
        const char* ch = ellipsis.c_str();
        SkUnichar unicode = nextUtf8Unit(&ch, ellipsis.c_str() + ellipsis.size());

       auto typeface = fOwner->fontCollection()->defaultFallback(
                    unicode, textStyle.getFontStyle(), textStyle.getLocale());
        if (typeface) {
            ellipsisRun = shaped(typeface, true);
            if (ellipsisRun->isResolved()) {
                return ellipsisRun;
            }
        }
    }
    return ellipsisRun;
}

TextLine::ClipContext TextLine::measureTextInsideOneRun(TextRange textRange,
                                                        const Run* run,
                                                        SkScalar runOffsetInLine,
                                                        SkScalar textOffsetInRunInLine,
                                                        bool includeGhostSpaces,
                                                        TextAdjustment textAdjustment) const {
    ClipContext result = { run, 0, run->size(), 0, SkRect::MakeEmpty(), 0, false };

    if (run->fEllipsis) {
        // Both ellipsis and placeholders can only be measured as one glyph
        result.fTextShift = runOffsetInLine;
        result.clip = SkRect::MakeXYWH(runOffsetInLine,
                                       sizes().runTop(run, this->fAscentStyle),
                                       run->advance().fX,
                                       run->calculateHeight(this->fAscentStyle,this->fDescentStyle));
        return result;
    } else if (run->isPlaceholder()) {
        result.fTextShift = runOffsetInLine;
        if (SkScalarIsFinite(run->fFontMetrics.fAscent)) {
          result.clip = SkRect::MakeXYWH(runOffsetInLine,
                                         sizes().runTop(run, this->fAscentStyle),
                                         run->advance().fX,
                                         run->calculateHeight(this->fAscentStyle,this->fDescentStyle));
        } else {
            result.clip = SkRect::MakeXYWH(runOffsetInLine, run->fFontMetrics.fAscent, run->advance().fX, 0);
        }
        return result;
    } else if (textRange.empty()) {
        return result;
    }

    TextRange originalTextRange(textRange); // We need it for proportional measurement
    // Find [start:end] clusters for the text
    while (true) {
        // Update textRange by cluster edges (shift start up to the edge of the cluster)
        // TODO: remove this limitation?
        TextRange updatedTextRange;
        bool found;
        std::tie(found, updatedTextRange.start, updatedTextRange.end) =
                                        run->findLimitingGlyphClusters(textRange);
        if (!found) {
            return result;
        }

        if ((textAdjustment & TextAdjustment::Grapheme) == 0) {
            textRange = updatedTextRange;
            break;
        }

        // Update text range by grapheme edges (shift start up to the edge of the grapheme)
        std::tie(found, updatedTextRange.start, updatedTextRange.end) =
                                    run->findLimitingGraphemes(updatedTextRange);
        if (updatedTextRange == textRange) {
            break;
        }

        // Some clusters are inside graphemes and we need to adjust them
        //SkDebugf("Correct range: [%d:%d) -> [%d:%d)\n", textRange.start, textRange.end, startIndex, endIndex);
        textRange = updatedTextRange;

        // Move the start until it's on the grapheme edge (and glypheme, too)
    }
    Cluster* start = &fOwner->cluster(fOwner->clusterIndex(textRange.start));
    Cluster* end = &fOwner->cluster(fOwner->clusterIndex(textRange.end - (textRange.width() == 0 ? 0 : 1)));

    if (!run->leftToRight()) {
        std::swap(start, end);
    }
    result.pos = start->startPos();
    result.size = (end->isHardBreak() ? end->startPos() : end->endPos()) - start->startPos();
    auto textStartInRun = run->positionX(start->startPos());
    auto textStartInLine = runOffsetInLine + textOffsetInRunInLine;
    if (!run->leftToRight()) {
        std::swap(start, end);
    }
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
                             sizes().runTop(run, this->fAscentStyle),
                             run->calculateWidth(result.pos, result.pos + result.size, false),
                             run->calculateHeight(this->fAscentStyle,this->fDescentStyle));

    // Correct the width in case the text edges don't match clusters
    // TODO: This is where we get smart about selecting a part of a cluster
    //  by shaping each grapheme separately and then use the result sizes
    //  to calculate the proportions
    auto leftCorrection = start->sizeToChar(originalTextRange.start);
    auto rightCorrection = end->sizeFromChar(originalTextRange.end - 1);
    /*
    SkDebugf("[%d: %d) => [%d: %d), @%d, %d: [%f:%f) + [%f:%f) = ", // جَآَهُ
             originalTextRange.start, originalTextRange.end, textRange.start, textRange.end,
             result.pos, result.size,
             result.clip.fLeft, result.clip.fRight, leftCorrection, rightCorrection);
     */
    result.clippingNeeded = leftCorrection != 0 || rightCorrection != 0;
    if (run->leftToRight()) {
        result.clip.fLeft += leftCorrection;
        result.clip.fRight -= rightCorrection;
        textStartInLine -= leftCorrection;
    } else {
        result.clip.fRight -= leftCorrection;
        result.clip.fLeft += rightCorrection;
        textStartInLine -= rightCorrection;
    }

    result.clip.offset(textStartInLine, 0);
    //SkDebugf("@%f[%f:%f)\n", textStartInLine, result.clip.fLeft, result.clip.fRight);

    if (compareRound(result.clip.fRight, fAdvance.fX, fOwner->getApplyRoundingHack()) > 0 && !includeGhostSpaces) {
        // There are few cases when we need it.
        // The most important one: we measure the text with spaces at the end (or at the beginning in RTL)
        // and we should ignore these spaces
        if (fOwner->paragraphStyle().getTextDirection() == TextDirection::kLtr) {
            // We only use this member for LTR
            result.fExcludedTrailingSpaces = std::max(result.clip.fRight - fAdvance.fX, 0.0f);
            result.clippingNeeded = true;
            result.clip.fRight = fAdvance.fX;
        }
    }

    if (result.clip.width() < 0) {
        // Weird situation when glyph offsets move the glyph to the left
        // (happens with zalgo texts, for instance)
        result.clip.fRight = result.clip.fLeft;
    }

    // The text must be aligned with the lineOffset
    result.fTextShift = textStartInLine - textStartInRun;

    return result;
}

void TextLine::iterateThroughClustersInGlyphsOrder(bool reversed,
                                                   bool includeGhosts,
                                                   const ClustersVisitor& visitor) const {
    // Walk through the clusters in the logical order (or reverse)
    SkSpan<const size_t> runs(fRunsInVisualOrder.data(), fRunsInVisualOrder.size());
    bool ignore = false;
    ClusterIndex index = 0;
    directional_for_each(runs, !reversed, [&](decltype(runs[0]) r) {
        if (ignore) return;
        auto run = this->fOwner->run(r);
        auto trimmedRange = fClusterRange.intersection(run.clusterRange());
        auto trailedRange = fGhostClusterRange.intersection(run.clusterRange());
        SkASSERT(trimmedRange.start == trailedRange.start);

        auto trailed = fOwner->clusters(trailedRange);
        auto trimmed = fOwner->clusters(trimmedRange);
        directional_for_each(trailed, reversed != run.leftToRight(), [&](Cluster& cluster) {
            if (ignore) return;
            bool ghost =  &cluster >= trimmed.end();
            if (!includeGhosts && ghost) {
                return;
            }
            if (!visitor(&cluster, index++, ghost)) {

                ignore = true;
                return;
            }
        });
    });
}

SkScalar TextLine::iterateThroughSingleRunByStyles(TextAdjustment textAdjustment,
                                                   const Run* run,
                                                   SkScalar runOffset,
                                                   TextRange textRange,
                                                   StyleType styleType,
                                                   const RunStyleVisitor& visitor) const {
    auto correctContext = [&](TextRange textRange, SkScalar textOffsetInRun) -> ClipContext {
        auto result = this->measureTextInsideOneRun(
                textRange, run, runOffset, textOffsetInRun, false, textAdjustment);
        if (styleType == StyleType::kDecorations) {
            // Decorations are drawn based on the real font metrics (regardless of styles and strut)
            result.clip.fTop = this->sizes().runTop(run, LineMetricStyle::CSS);
            result.clip.fBottom = result.clip.fTop +
                                  run->calculateHeight(LineMetricStyle::CSS, LineMetricStyle::CSS);
        }
        return result;
    };

    if (run->fEllipsis) {
        // Extra efforts to get the ellipsis text style
        ClipContext clipContext = correctContext(run->textRange(), 0.0f);
        TextRange testRange(run->fClusterStart, run->fClusterStart + run->textRange().width());
        for (BlockIndex index = fBlockRange.start; index < fBlockRange.end; ++index) {
           auto block = fOwner->styles().begin() + index;
           auto intersect = intersected(block->fRange, testRange);
           if (intersect.width() > 0) {
               visitor(testRange, block->fStyle, clipContext);
               return run->advance().fX;
           }
        }
        SkASSERT(false);
    }

    if (styleType == StyleType::kNone) {
        ClipContext clipContext = correctContext(textRange, 0.0f);
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

    const BlockIndex blockRangeSize = fBlockRange.end - fBlockRange.start;
    for (BlockIndex index = 0; index <= blockRangeSize; ++index) {

        TextRange intersect;
        TextStyle* style = nullptr;
        if (index < blockRangeSize) {
            auto block = fOwner->styles().begin() +
                 (run->leftToRight() ? fBlockRange.start + index : fBlockRange.end - index - 1);

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
                    // RTL text intervals move backward
                    start = std::min(intersect.start, start);
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
        auto runStyleTextRange = TextRange(start, start + size);
        ClipContext clipContext = correctContext(runStyleTextRange, textOffsetInRun);
        textOffsetInRun += clipContext.clip.width();
        if (clipContext.clip.height() == 0) {
            continue;
        }
        visitor(runStyleTextRange, *prevStyle, clipContext);

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
    auto textRange = includingGhostSpaces ? this->textWithNewlines() : this->trimmedText();

    if (this->ellipsis() != nullptr && fOwner->paragraphStyle().getTextDirection() == TextDirection::kRtl) {
        runOffset = this->ellipsis()->offset().fX;
        if (visitor(ellipsis(), runOffset, ellipsis()->textRange(), &width)) {
        }
    }

    for (auto& runIndex : fRunsInVisualOrder) {

        const auto run = &this->fOwner->run(runIndex);
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
                    TextRange(fTextExcludingSpaces.end, fTextIncludingNewlines.end), run->fTextRange);
            if (whitespaces.width() > 0) {
                auto whitespacesLen = measureTextInsideOneRun(whitespaces, run, runOffset, 0, true, TextAdjustment::GlyphCluster).clip.width();
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

    if (this->ellipsis() != nullptr && fOwner->paragraphStyle().getTextDirection() == TextDirection::kLtr) {
        if (visitor(ellipsis(), runOffset, ellipsis()->textRange(), &width)) {
            totalWidth += width;
        }
    }

    if (!includingGhostSpaces && compareRound(totalWidth, this->width(), fOwner->getApplyRoundingHack()) != 0) {
    // This is a very important assert!
    // It asserts that 2 different ways of calculation come with the same results
        SkDEBUGFAILF("ASSERT: %f != %f\n", totalWidth, this->width());
    }
}

SkVector TextLine::offset() const {
    return fOffset + SkVector::Make(fShift, 0);
}

LineMetrics TextLine::getMetrics() const {
    LineMetrics result;
    SkASSERT(fOwner);

    // Fill out the metrics
    fOwner->ensureUTF16Mapping();
    result.fStartIndex = fOwner->getUTF16Index(fTextExcludingSpaces.start);
    result.fEndExcludingWhitespaces = fOwner->getUTF16Index(fTextExcludingSpaces.end);
    result.fEndIndex = fOwner->getUTF16Index(fText.end);
    result.fEndIncludingNewline = fOwner->getUTF16Index(fTextIncludingNewlines.end);
    result.fHardBreak = endsWithHardLineBreak();
    result.fAscent = - fMaxRunMetrics.ascent();
    result.fDescent = fMaxRunMetrics.descent();
    result.fUnscaledAscent = - fMaxRunMetrics.ascent(); // TODO: implement
    result.fHeight = fAdvance.fY;
    result.fWidth = fAdvance.fX;
    if (fOwner->getApplyRoundingHack()) {
        result.fHeight = littleRound(result.fHeight);
        result.fWidth = littleRound(result.fWidth);
    }
    result.fLeft = this->offset().fX;
    // This is Flutter definition of a baseline
    result.fBaseline = this->offset().fY + this->height() - this->sizes().descent();
    result.fLineNumber = this - fOwner->lines().begin();

    // Fill out the style parts
    this->iterateThroughVisualRuns(false,
        [this, &result]
        (const Run* run, SkScalar runOffsetInLine, TextRange textRange, SkScalar* runWidthInLine) {
        if (run->placeholderStyle() != nullptr) {
            *runWidthInLine = run->advance().fX;
            return true;
        }
        *runWidthInLine = this->iterateThroughSingleRunByStyles(
        TextAdjustment::GlyphCluster, run, runOffsetInLine, textRange, StyleType::kForeground,
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

bool TextLine::isFirstLine() const {
    return this == &fOwner->lines().front();
}

bool TextLine::isLastLine() const {
    return this == &fOwner->lines().back();
}

bool TextLine::endsWithHardLineBreak() const {
    // TODO: For some reason Flutter imagines a hard line break at the end of the last line.
    //  To be removed...
    return (fGhostClusterRange.width() > 0 && fOwner->cluster(fGhostClusterRange.end - 1).isHardBreak()) ||
           fEllipsis != nullptr ||
           fGhostClusterRange.end == fOwner->clusters().size() - 1;
}

void TextLine::getRectsForRange(TextRange textRange0,
                                RectHeightStyle rectHeightStyle,
                                RectWidthStyle rectWidthStyle,
                                std::vector<TextBox>& boxes) const
{
    const Run* lastRun = nullptr;
    auto startBox = boxes.size();
    this->iterateThroughVisualRuns(true,
        [textRange0, rectHeightStyle, rectWidthStyle, &boxes, &lastRun, startBox, this]
        (const Run* run, SkScalar runOffsetInLine, TextRange textRange, SkScalar* runWidthInLine) {
        *runWidthInLine = this->iterateThroughSingleRunByStyles(
        TextAdjustment::GraphemeGluster, run, runOffsetInLine, textRange, StyleType::kNone,
        [run, runOffsetInLine, textRange0, rectHeightStyle, rectWidthStyle, &boxes, &lastRun, startBox, this]
        (TextRange textRange, const TextStyle& style, const TextLine::ClipContext& lineContext) {

            auto intersect = textRange * textRange0;
            if (intersect.empty()) {
                return true;
            }

            auto paragraphStyle = fOwner->paragraphStyle();

            // Found a run that intersects with the text
            auto context = this->measureTextInsideOneRun(
                    intersect, run, runOffsetInLine, 0, true, TextAdjustment::GraphemeGluster);
            SkRect clip = context.clip;
            clip.offset(lineContext.fTextShift - context.fTextShift, 0);

            switch (rectHeightStyle) {
                case RectHeightStyle::kMax:
                    // TODO: Change it once flutter rolls into google3
                    //  (probably will break things if changed before)
                    clip.fBottom = this->height();
                    clip.fTop = this->sizes().delta();
                    break;
                case RectHeightStyle::kIncludeLineSpacingTop: {
                    clip.fBottom = this->height();
                    clip.fTop = this->sizes().delta();
                    auto verticalShift = this->sizes().rawAscent() - this->sizes().ascent();
                    if (isFirstLine()) {
                        clip.fTop += verticalShift;
                    }
                    break;
                }
                case RectHeightStyle::kIncludeLineSpacingMiddle: {
                    clip.fBottom = this->height();
                    clip.fTop = this->sizes().delta();
                    auto verticalShift = this->sizes().rawAscent() - this->sizes().ascent();
                    clip.offset(0, verticalShift / 2.0);
                    if (isFirstLine()) {
                        clip.fTop += verticalShift / 2.0;
                    }
                    if (isLastLine()) {
                        clip.fBottom -= verticalShift / 2.0;
                    }
                    break;
                 }
                case RectHeightStyle::kIncludeLineSpacingBottom: {
                    clip.fBottom = this->height();
                    clip.fTop = this->sizes().delta();
                    auto verticalShift = this->sizes().rawAscent() - this->sizes().ascent();
                    clip.offset(0, verticalShift);
                    if (isLastLine()) {
                        clip.fBottom -= verticalShift;
                    }
                    break;
                }
                case RectHeightStyle::kStrut: {
                    const auto& strutStyle = paragraphStyle.getStrutStyle();
                    if (strutStyle.getStrutEnabled()
                        && strutStyle.getFontSize() > 0) {
                        auto strutMetrics = fOwner->strutMetrics();
                        auto top = this->baseline();
                        clip.fTop = top + strutMetrics.ascent();
                        clip.fBottom = top + strutMetrics.descent();
                    }
                }
                break;
                case RectHeightStyle::kTight: {
                    if (run->fHeightMultiplier <= 0) {
                        break;
                    }
                    const auto effectiveBaseline = this->baseline() + this->sizes().delta();
                    clip.fTop = effectiveBaseline + run->ascent();
                    clip.fBottom = effectiveBaseline + run->descent();
                }
                break;
                default:
                    SkASSERT(false);
                break;
            }

            // Separate trailing spaces and move them in the default order of the paragraph
            // in case the run order and the paragraph order don't match
            SkRect trailingSpaces = SkRect::MakeEmpty();
            if (this->trimmedText().end <this->textWithNewlines().end && // Line has trailing space
                this->textWithNewlines().end == intersect.end &&         // Range is at the end of the line
                this->trimmedText().end > intersect.start)               // Range has more than just spaces
            {
                auto delta = this->spacesWidth();
                trailingSpaces = SkRect::MakeXYWH(0, 0, 0, 0);
                // There are trailing spaces in this run
                if (paragraphStyle.getTextAlign() == TextAlign::kJustify && isLastLine())
                {
                    // TODO: this is just a patch. Make it right later (when it's clear what and how)
                    trailingSpaces = clip;
                    if(run->leftToRight()) {
                        trailingSpaces.fLeft = this->width();
                        clip.fRight = this->width();
                    } else {
                        trailingSpaces.fRight = 0;
                        clip.fLeft = 0;
                    }
                } else if (paragraphStyle.getTextDirection() == TextDirection::kRtl &&
                    !run->leftToRight())
                {
                    // Split
                    trailingSpaces = clip;
                    trailingSpaces.fLeft = - delta;
                    trailingSpaces.fRight = 0;
                    clip.fLeft += delta;
                } else if (paragraphStyle.getTextDirection() == TextDirection::kLtr &&
                    run->leftToRight())
                {
                    // Split
                    trailingSpaces = clip;
                    trailingSpaces.fLeft = this->width();
                    trailingSpaces.fRight = trailingSpaces.fLeft + delta;
                    clip.fRight -= delta;
                }
            }

            clip.offset(this->offset());
            if (trailingSpaces.width() > 0) {
                trailingSpaces.offset(this->offset());
            }

            // Check if we can merge two boxes instead of adding a new one
            auto merge = [&lastRun, &context, &boxes](SkRect clip) {
                bool mergedBoxes = false;
                if (!boxes.empty() &&
                    lastRun != nullptr &&
                    context.run->leftToRight() == lastRun->leftToRight() &&
                    lastRun->placeholderStyle() == nullptr &&
                    context.run->placeholderStyle() == nullptr &&
                    nearlyEqual(lastRun->heightMultiplier(),
                                context.run->heightMultiplier()) &&
                    lastRun->font() == context.run->font())
                {
                    auto& lastBox = boxes.back();
                    if (nearlyEqual(lastBox.rect.fTop, clip.fTop) &&
                        nearlyEqual(lastBox.rect.fBottom, clip.fBottom) &&
                            (nearlyEqual(lastBox.rect.fLeft, clip.fRight) ||
                             nearlyEqual(lastBox.rect.fRight, clip.fLeft)))
                    {
                        lastBox.rect.fLeft = std::min(lastBox.rect.fLeft, clip.fLeft);
                        lastBox.rect.fRight = std::max(lastBox.rect.fRight, clip.fRight);
                        mergedBoxes = true;
                    }
                }
                lastRun = context.run;
                return mergedBoxes;
            };

            if (!merge(clip)) {
                boxes.emplace_back(clip, context.run->getTextDirection());
            }
            if (!nearlyZero(trailingSpaces.width()) && !merge(trailingSpaces)) {
                boxes.emplace_back(trailingSpaces, paragraphStyle.getTextDirection());
            }

            if (rectWidthStyle == RectWidthStyle::kMax && !isLastLine()) {
                // Align the very left/right box horizontally
                auto lineStart = this->offset().fX;
                auto lineEnd = this->offset().fX + this->width();
                auto left = boxes[startBox];
                auto right = boxes.back();
                if (left.rect.fLeft > lineStart && left.direction == TextDirection::kRtl) {
                    left.rect.fRight = left.rect.fLeft;
                    left.rect.fLeft = 0;
                    boxes.insert(boxes.begin() + startBox + 1, left);
                }
                if (right.direction == TextDirection::kLtr &&
                    right.rect.fRight >= lineEnd &&
                    right.rect.fRight < fOwner->widthWithTrailingSpaces()) {
                    right.rect.fLeft = right.rect.fRight;
                    right.rect.fRight = fOwner->widthWithTrailingSpaces();
                    boxes.emplace_back(right);
                }
            }

            return true;
        });
        return true;
    });
    if (fOwner->getApplyRoundingHack()) {
        for (auto& r : boxes) {
            r.rect.fLeft = littleRound(r.rect.fLeft);
            r.rect.fRight = littleRound(r.rect.fRight);
            r.rect.fTop = littleRound(r.rect.fTop);
            r.rect.fBottom = littleRound(r.rect.fBottom);
        }
    }
}

PositionWithAffinity TextLine::getGlyphPositionAtCoordinate(SkScalar dx) {

    if (SkScalarNearlyZero(this->width()) && SkScalarNearlyZero(this->spacesWidth())) {
        // TODO: this is one of the flutter changes that have to go away eventually
        //  Empty line is a special case in txtlib (but only when there are no spaces, too)
        auto utf16Index = fOwner->getUTF16Index(this->fTextExcludingSpaces.end);
        return { SkToS32(utf16Index) , kDownstream };
    }

    PositionWithAffinity result(0, Affinity::kDownstream);
    this->iterateThroughVisualRuns(true,
        [this, dx, &result]
        (const Run* run, SkScalar runOffsetInLine, TextRange textRange, SkScalar* runWidthInLine) {
            bool keepLooking = true;
            *runWidthInLine = this->iterateThroughSingleRunByStyles(
            TextAdjustment::GraphemeGluster, run, runOffsetInLine, textRange, StyleType::kNone,
            [this, run, dx, &result, &keepLooking]
            (TextRange textRange, const TextStyle& style, const TextLine::ClipContext& context0) {

                SkScalar offsetX = this->offset().fX;
                ClipContext context = context0;

                // Correct the clip size because libtxt counts trailing spaces
                if (run->leftToRight()) {
                    context.clip.fRight += context.fExcludedTrailingSpaces; // extending clip to the right
                } else {
                    // Clip starts from 0; we cannot extend it to the left from that
                }
                // However, we need to offset the clip
                context.clip.offset(offsetX, 0.0f);

                // This patch will help us to avoid a floating point error
                if (SkScalarNearlyEqual(context.clip.fRight, dx, 0.01f)) {
                    context.clip.fRight = dx;
                }

                if (dx <= context.clip.fLeft) {
                    // All the other runs are placed right of this one
                    auto utf16Index = fOwner->getUTF16Index(context.run->globalClusterIndex(context.pos));
                    if (run->leftToRight()) {
                        result = { SkToS32(utf16Index), kDownstream};
                        keepLooking = false;
                    } else {
                        result = { SkToS32(utf16Index + 1), kUpstream};
                        // If we haven't reached the end of the run we need to keep looking
                        keepLooking = context.pos != 0;
                    }
                    // For RTL we go another way
                    return !run->leftToRight();
                }

                if (dx >= context.clip.fRight) {
                    // We have to keep looking ; just in case keep the last one as the closest
                    auto utf16Index = fOwner->getUTF16Index(context.run->globalClusterIndex(context.pos + context.size));
                    if (run->leftToRight()) {
                        result = {SkToS32(utf16Index), kUpstream};
                    } else {
                        result = {SkToS32(utf16Index), kDownstream};
                    }
                    // For RTL we go another way
                    return run->leftToRight();
                }

                // So we found the run that contains our coordinates
                // Find the glyph position in the run that is the closest left of our point
                // TODO: binary search
                size_t found = context.pos;
                for (size_t index = context.pos; index < context.pos + context.size; ++index) {
                    // TODO: this rounding is done to match Flutter tests. Must be removed..
                    auto end = context.run->positionX(index) + context.fTextShift + offsetX;
                    if (fOwner->getApplyRoundingHack()) {
                        end = littleRound(end);
                    }
                    if (end > dx) {
                        break;
                    } else if (end == dx && !context.run->leftToRight()) {
                        // When we move RTL variable end points to the beginning of the code point which is included
                        found = index;
                        break;
                    }
                    found = index;
                }

                SkScalar glyphemePosLeft = context.run->positionX(found) + context.fTextShift + offsetX;
                SkScalar glyphemesWidth = context.run->positionX(found + 1) - context.run->positionX(found);

                // Find the grapheme range that contains the point
                auto clusterIndex8 = context.run->globalClusterIndex(found);
                auto clusterEnd8 = context.run->globalClusterIndex(found + 1);
                auto graphemes = fOwner->countSurroundingGraphemes({clusterIndex8, clusterEnd8});

                SkScalar center = glyphemePosLeft + glyphemesWidth / 2;
                if (graphemes.size() > 1) {
                    // Calculate the position proportionally based on grapheme count
                    SkScalar averageGraphemeWidth = glyphemesWidth / graphemes.size();
                    SkScalar delta = dx - glyphemePosLeft;
                    int graphemeIndex = SkScalarNearlyZero(averageGraphemeWidth)
                                         ? 0
                                         : SkScalarFloorToInt(delta / averageGraphemeWidth);
                    auto graphemeCenter = glyphemePosLeft + graphemeIndex * averageGraphemeWidth +
                                          averageGraphemeWidth / 2;
                    auto graphemeUtf8Index = graphemes[graphemeIndex];
                    if ((dx < graphemeCenter) == context.run->leftToRight()) {
                        size_t utf16Index = fOwner->getUTF16Index(graphemeUtf8Index);
                        result = { SkToS32(utf16Index), kDownstream };
                    } else {
                        size_t utf16Index = fOwner->getUTF16Index(graphemeUtf8Index + 1);
                        result = { SkToS32(utf16Index), kUpstream };
                    }
                    // Keep UTF16 index as is
                } else if ((dx < center) == context.run->leftToRight()) {
                    size_t utf16Index = fOwner->getUTF16Index(clusterIndex8);
                    result = { SkToS32(utf16Index), kDownstream };
                } else {
                    size_t utf16Index = context.run->leftToRight()
                                                ? fOwner->getUTF16Index(clusterEnd8)
                                                : fOwner->getUTF16Index(clusterIndex8) + 1;
                    result = { SkToS32(utf16Index), kUpstream };
                }

                return keepLooking = false;

            });
            return keepLooking;
        }
    );
    return result;
}

void TextLine::getRectsForPlaceholders(std::vector<TextBox>& boxes) {
    this->iterateThroughVisualRuns(
        true,
        [&boxes, this](const Run* run, SkScalar runOffset, TextRange textRange,
                        SkScalar* width) {
                auto context = this->measureTextInsideOneRun(
                        textRange, run, runOffset, 0, true, TextAdjustment::GraphemeGluster);
                *width = context.clip.width();

            if (textRange.width() == 0) {
                return true;
            }
            if (!run->isPlaceholder()) {
                return true;
            }

            SkRect clip = context.clip;
            clip.offset(this->offset());

            if (fOwner->getApplyRoundingHack()) {
                clip.fLeft = littleRound(clip.fLeft);
                clip.fRight = littleRound(clip.fRight);
                clip.fTop = littleRound(clip.fTop);
                clip.fBottom = littleRound(clip.fBottom);
            }
            boxes.emplace_back(clip, run->getTextDirection());
            return true;
        });
}
}  // namespace textlayout
}  // namespace skia
