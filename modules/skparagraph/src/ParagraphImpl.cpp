// Copyright 2019 Google LLC.
#include "include/core/SkBlurTypes.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkPictureRecorder.h"
#include "modules/skparagraph/src/OneLineShaper.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/Run.h"
#include "modules/skparagraph/src/TextWrapper.h"
#include "src/core/SkSpan.h"
#include "src/utils/SkUTF.h"
#include <algorithm>
#include <unicode/ustring.h>
#include <queue>

namespace skia {
namespace textlayout {

namespace {

using ICUUText = std::unique_ptr<UText, SkFunctionWrapper<decltype(utext_close), utext_close>>;

SkScalar littleRound(SkScalar a) {
    // This rounding is done to match Flutter tests. Must be removed..
  return SkScalarRoundToScalar(a * 100.0)/100.0;
}

}

TextRange operator*(const TextRange& a, const TextRange& b) {
    if (a.start == b.start && a.end == b.end) return a;
    auto begin = SkTMax(a.start, b.start);
    auto end = SkTMin(a.end, b.end);
    return end > begin ? TextRange(begin, end) : EMPTY_TEXT;
}

bool TextBreaker::initialize(SkSpan<const char> text, UBreakIteratorType type) {

    UErrorCode status = U_ZERO_ERROR;
    fIterator = nullptr;
    fSize = text.size();
    UText sUtf8UText = UTEXT_INITIALIZER;
    std::unique_ptr<UText, SkFunctionWrapper<decltype(utext_close), utext_close>> utf8UText(
        utext_openUTF8(&sUtf8UText, text.begin(), text.size(), &status));
    if (U_FAILURE(status)) {
        SkDEBUGF("Could not create utf8UText: %s", u_errorName(status));
        return false;
    }
    fIterator.reset(ubrk_open(type, "en", nullptr, 0, &status));
    if (U_FAILURE(status)) {
        SkDEBUGF("Could not create line break iterator: %s", u_errorName(status));
        SK_ABORT("");
    }

    ubrk_setUText(fIterator.get(), utf8UText.get(), &status);
    if (U_FAILURE(status)) {
        SkDEBUGF("Could not setText on break iterator: %s", u_errorName(status));
        return false;
    }

    fInitialized = true;
    fPos = 0;
    return true;
}

ParagraphImpl::ParagraphImpl(const SkString& text,
                             ParagraphStyle style,
                             SkTArray<Block, true> blocks,
                             SkTArray<Placeholder, true> placeholders,
                             sk_sp<FontCollection> fonts)
        : Paragraph(std::move(style), std::move(fonts))
        , fTextStyles(std::move(blocks))
        , fPlaceholders(std::move(placeholders))
        , fText(text)
        , fState(kUnknown)
        , fUnresolvedGlyphs(0)
        , fPicture(nullptr)
        , fStrutMetrics(false)
        , fOldWidth(0)
        , fOldHeight(0)
        , fOrigin(SkRect::MakeEmpty()) {
    // TODO: extractStyles();
}

ParagraphImpl::ParagraphImpl(const std::u16string& utf16text,
                             ParagraphStyle style,
                             SkTArray<Block, true> blocks,
                             SkTArray<Placeholder, true> placeholders,
                             sk_sp<FontCollection> fonts)
        : Paragraph(std::move(style), std::move(fonts))
        , fTextStyles(std::move(blocks))
        , fPlaceholders(std::move(placeholders))
        , fState(kUnknown)
        , fUnresolvedGlyphs(0)
        , fPicture(nullptr)
        , fStrutMetrics(false)
        , fOldWidth(0)
        , fOldHeight(0)
        , fOrigin(SkRect::MakeEmpty()) {
    icu::UnicodeString unicode((UChar*)utf16text.data(), SkToS32(utf16text.size()));
    std::string str;
    unicode.toUTF8String(str);
    fText = SkString(str.data(), str.size());
    // TODO: extractStyles();
}

ParagraphImpl::~ParagraphImpl() = default;

int32_t ParagraphImpl::unresolvedGlyphs() {
    if (fState < kShaped) {
        return -1;
    }

    return fUnresolvedGlyphs;
}

void ParagraphImpl::layout(SkScalar rawWidth) {

    // TODO: This rounding is done to match Flutter tests. Must be removed...
    auto floorWidth = SkScalarFloorToScalar(rawWidth);
    if (fState < kShaped) {
        // Layout marked as dirty for performance/testing reasons
        this->fRuns.reset();
        this->fRunShifts.reset();
        this->fClusters.reset();
    } else if (fState >= kLineBroken && (fOldWidth != floorWidth || fOldHeight != fHeight)) {
        // We can use the results from SkShaper but have to break lines again
        fState = kShaped;
    }

    if (fState < kShaped) {
        fClusters.reset();
        fGraphemes.reset();
        this->markGraphemes();

        if (!this->shapeTextIntoEndlessLine()) {

            this->resetContext();
            this->resolveStrut();
            this->fLines.reset();

            // Set the important values that are not zero
            auto emptyMetrics = computeEmptyMetrics();
            fWidth = floorWidth;
            fHeight = emptyMetrics.height();
            if (fParagraphStyle.getStrutStyle().getStrutEnabled() &&
                fParagraphStyle.getStrutStyle().getForceStrutHeight()) {
                fHeight = fStrutMetrics.height();
            }
            fAlphabeticBaseline = emptyMetrics.alphabeticBaseline();
            fIdeographicBaseline = emptyMetrics.ideographicBaseline();
            fMinIntrinsicWidth = 0;
            fMaxIntrinsicWidth = 0;
            this->fOldWidth = floorWidth;
            this->fOldHeight = this->fHeight;

            return;
        }
        if (fState < kShaped) {
            fState = kShaped;
        } else {
            layout(floorWidth);
            return;
        }

        if (fState < kMarked) {
            this->buildClusterTable();
            fState = kClusterized;
            this->markLineBreaks();
            fState = kMarked;

            // Add the paragraph to the cache
            fFontCollection->getParagraphCache()->updateParagraph(this);
        }
    }

    if (fState >= kLineBroken)  {
        if (fOldWidth != floorWidth || fOldHeight != fHeight) {
            fState = kMarked;
        }
    }

    if (fState < kLineBroken) {
        this->resetContext();
        this->resolveStrut();
        this->fLines.reset();
        this->breakShapedTextIntoLines(floorWidth);
        fState = kLineBroken;

    }

    if (fState < kFormatted) {
        // Build the picture lazily not until we actually have to paint (or never)
        this->formatLines(fWidth);
        fState = kFormatted;
    }

    this->fOldWidth = floorWidth;
    this->fOldHeight = this->fHeight;

    // TODO: This rounding is done to match Flutter tests. Must be removed...
    fMinIntrinsicWidth = littleRound(fMinIntrinsicWidth);
    fMaxIntrinsicWidth = littleRound(fMaxIntrinsicWidth);

    // TODO: This is strictly Flutter thing. Must be factored out into some flutter code
    if (fParagraphStyle.getMaxLines() == 1 || (fParagraphStyle.unlimited_lines() && fParagraphStyle.ellipsized())) {
        fMinIntrinsicWidth = fMaxIntrinsicWidth;
    }

    // TODO: This rounding is done to match Flutter tests. Must be removed..
    if (floorWidth == 0.0f) {
        fWidth = 0;
        if (fParagraphStyle.unlimited_lines() && !fParagraphStyle.ellipsized()) {
            fMinIntrinsicWidth = fHeight;
            fHeight = fMaxIntrinsicWidth;
        }
    }
}

void ParagraphImpl::paint(SkCanvas* canvas, SkScalar x, SkScalar y) {

    if (fState < kDrawn) {
        // Record the picture anyway (but if we have some pieces in the cache they will be used)
        this->paintLinesIntoPicture();
        fState = kDrawn;
    }

    SkMatrix matrix = SkMatrix::MakeTrans(x + fOrigin.fLeft, y + fOrigin.fTop);
    canvas->drawPicture(fPicture, &matrix, nullptr);
}

void ParagraphImpl::resetContext() {
    fAlphabeticBaseline = 0;
    fHeight = 0;
    fWidth = 0;
    fIdeographicBaseline = 0;
    fMaxIntrinsicWidth = 0;
    fMinIntrinsicWidth = 0;
    fLongestLine = 0;
    fMaxWidthWithTrailingSpaces = 0;
    fExceededMaxLines = false;
}

// Clusters in the order of the input text
void ParagraphImpl::buildClusterTable() {

    // Walk through all the run in the direction of input text
    for (RunIndex runIndex = 0; runIndex < fRuns.size(); ++runIndex) {
        auto& run = fRuns[runIndex];
        auto runStart = fClusters.size();
        if (run.isPlaceholder()) {
            // There are no glyphs but we want to have one cluster
            SkSpan<const char> text = this->text(run.textRange());
            if (!fClusters.empty()) {
                fClusters.back().setBreakType(Cluster::SoftLineBreak);
            }
            auto& cluster = fClusters.emplace_back(this, runIndex, 0ul, 0ul, text, run.advance().fX,
                                                   run.advance().fY);
            cluster.setBreakType(Cluster::SoftLineBreak);
        } else {
            fClusters.reserve(fClusters.size() + run.size());
            // Walk through the glyph in the direction of input text
            run.iterateThroughClustersInTextOrder([runIndex, this](size_t glyphStart,
                                                                   size_t glyphEnd,
                                                                   size_t charStart,
                                                                   size_t charEnd,
                                                                   SkScalar width,
                                                                   SkScalar height) {
                SkASSERT(charEnd >= charStart);
                SkSpan<const char> text(fText.c_str() + charStart, charEnd - charStart);
                auto& cluster = fClusters.emplace_back(this, runIndex, glyphStart, glyphEnd, text,
                                                       width, height);
                cluster.setIsWhiteSpaces();
            });
        }

        run.setClusterRange(runStart, fClusters.size());
        fMaxIntrinsicWidth += run.advance().fX;
    }
}

void ParagraphImpl::markLineBreaks() {

    // Find all possible (soft) line breaks
    // This iterator is used only once for a paragraph so we don't have to keep it
    TextBreaker breaker;
    if (!breaker.initialize(this->text(), UBRK_LINE)) {
        return;
    }

    Cluster* current = fClusters.begin();
    while (!breaker.eof() && current < fClusters.end()) {
        size_t currentPos = breaker.next();
        while (current < fClusters.end()) {
            if (current->textRange().end > currentPos) {
                break;
            } else if (current->textRange().end == currentPos) {
                current->setBreakType(breaker.status() == UBRK_LINE_HARD
                                      ? Cluster::BreakType::HardLineBreak
                                      : Cluster::BreakType::SoftLineBreak);
                ++current;
                break;
            }
            ++current;
        }
    }


    // Walk through all the clusters in the direction of shaped text
    // (we have to walk through the styles in the same order, too)
    SkScalar shift = 0;
    for (auto& run : fRuns) {

        // Skip placeholder runs
        if (run.isPlaceholder()) {
            continue;
        }

        bool soFarWhitespacesOnly = true;
        for (size_t index = 0; index != run.clusterRange().width(); ++index) {
            auto correctIndex = run.leftToRight()
                    ? index + run.clusterRange().start
                    : run.clusterRange().end - index - 1;
            const auto cluster = &this->cluster(correctIndex);

            // Shift the cluster (shift collected from the previous clusters)
            run.shift(cluster, shift);

            // Synchronize styles (one cluster can be covered by few styles)
            Block* currentStyle = this->fTextStyles.begin();
            while (!cluster->startsIn(currentStyle->fRange)) {
                currentStyle++;
                SkASSERT(currentStyle != this->fTextStyles.end());
            }

            SkASSERT(!currentStyle->fStyle.isPlaceholder());

            // Process word spacing
            if (currentStyle->fStyle.getWordSpacing() != 0) {
                if (cluster->isWhitespaces() && cluster->isSoftBreak()) {
                    if (!soFarWhitespacesOnly) {
                        shift += run.addSpacesAtTheEnd(currentStyle->fStyle.getWordSpacing(), cluster);
                    }
                }
            }
            // Process letter spacing
            if (currentStyle->fStyle.getLetterSpacing() != 0) {
                shift += run.addSpacesEvenly(currentStyle->fStyle.getLetterSpacing(), cluster);
            }

            if (soFarWhitespacesOnly && !cluster->isWhitespaces()) {
                soFarWhitespacesOnly = false;
            }
        }
    }

    fClusters.emplace_back(this, EMPTY_RUN, 0, 0, SkSpan<const char>(), 0, 0);
}

bool ParagraphImpl::shapeTextIntoEndlessLine() {

    if (fText.size() == 0) {
        return false;
    }

    // Check the font-resolved text against the cache
    if (fFontCollection->getParagraphCache()->findParagraph(this)) {
        this->fRunShifts.reset();
        return true;
    }

    fFontSwitches.reset();

    OneLineShaper oneLineShaper(this);
    auto result = oneLineShaper.shape();
    fUnresolvedGlyphs = oneLineShaper.unresolvedGlyphs();

    if (!result) {
        return false;
    } else {
        this->fRunShifts.reset();
        return true;
    }
}

void ParagraphImpl::breakShapedTextIntoLines(SkScalar maxWidth) {

    TextWrapper textWrapper;
    textWrapper.breakTextIntoLines(
            this,
            maxWidth,
            [&](TextRange text,
                TextRange textWithSpaces,
                ClusterRange clusters,
                ClusterRange clustersWithGhosts,
                SkScalar widthWithSpaces,
                size_t startPos,
                size_t endPos,
                SkVector offset,
                SkVector advance,
                InternalLineMetrics metrics,
                bool addEllipsis) {
                // Add the line
                // TODO: Take in account clipped edges
                auto& line = this->addLine(offset, advance, text, textWithSpaces, clusters, clustersWithGhosts, widthWithSpaces, metrics);
                if (addEllipsis) {
                    line.createEllipsis(maxWidth, fParagraphStyle.getEllipsis(), true);
                }

                fLongestLine = advance.fX;
            });
    fHeight = textWrapper.height();
    fWidth = maxWidth;
    fMaxIntrinsicWidth = textWrapper.maxIntrinsicWidth();
    fMinIntrinsicWidth = textWrapper.minIntrinsicWidth();
    fAlphabeticBaseline = fLines.empty() ? 0 : fLines.front().alphabeticBaseline();
    fIdeographicBaseline = fLines.empty() ? 0 : fLines.front().ideographicBaseline();
    fExceededMaxLines = textWrapper.exceededMaxLines();
}

void ParagraphImpl::formatLines(SkScalar maxWidth) {
    auto effectiveAlign = fParagraphStyle.effective_align();
    if (effectiveAlign == TextAlign::kJustify) {
        this->resetRunShifts();
    }
    for (auto& line : fLines) {
        if (&line == &fLines.back() && effectiveAlign == TextAlign::kJustify) {
            effectiveAlign = line.assumedTextAlign();
        }
        line.format(effectiveAlign, maxWidth);
    }
}

void ParagraphImpl::paintLinesIntoPicture() {
    SkPictureRecorder recorder;
    SkCanvas* textCanvas = recorder.beginRecording(fOrigin.width(), fOrigin.height(), nullptr, 0);
    textCanvas->translate(-fOrigin.fLeft, -fOrigin.fTop);

    for (auto& line : fLines) {
        line.paint(textCanvas);
    }

    fPicture = recorder.finishRecordingAsPicture();
}

void ParagraphImpl::resolveStrut() {
    auto strutStyle = this->paragraphStyle().getStrutStyle();
    if (!strutStyle.getStrutEnabled() || strutStyle.getFontSize() < 0) {
        return;
    }

    std::vector<sk_sp<SkTypeface>> typefaces = fFontCollection->findTypefaces(strutStyle.getFontFamilies(), strutStyle.getFontStyle());
    if (typefaces.empty()) {
        SkDEBUGF("Could not resolve strut font\n");
        return;
    }

    SkFont font(typefaces.front(), strutStyle.getFontSize());
    SkFontMetrics metrics;
    font.getMetrics(&metrics);

    if (strutStyle.getHeightOverride()) {
        auto strutHeight = metrics.fDescent - metrics.fAscent;
        auto strutMultiplier = strutStyle.getHeight() * strutStyle.getFontSize();
        fStrutMetrics = InternalLineMetrics(
            (metrics.fAscent / strutHeight) * strutMultiplier,
            (metrics.fDescent / strutHeight) * strutMultiplier,
                strutStyle.getLeading() < 0 ? 0 : strutStyle.getLeading() * strutStyle.getFontSize());
    } else {
        fStrutMetrics = InternalLineMetrics(
                metrics.fAscent,
                metrics.fDescent,
                strutStyle.getLeading() < 0 ? 0
                                            : strutStyle.getLeading() * strutStyle.getFontSize());
    }
    fStrutMetrics.setForceStrut(this->paragraphStyle().getStrutStyle().getForceStrutHeight());
}

BlockRange ParagraphImpl::findAllBlocks(TextRange textRange) {
    BlockIndex begin = EMPTY_BLOCK;
    BlockIndex end = EMPTY_BLOCK;
    for (size_t index = 0; index < fTextStyles.size(); ++index) {
        auto& block = fTextStyles[index];
        if (block.fRange.end <= textRange.start) {
            continue;
        }
        if (block.fRange.start >= textRange.end) {
            break;
        }
        if (begin == EMPTY_BLOCK) {
            begin = index;
        }
        end = index;
    }

    return { begin, end + 1 };
}

void ParagraphImpl::calculateBoundaries(ClusterRange clusters, SkVector offset, SkVector advance) {

    auto boundaries = SkRect::MakeXYWH(0, 0, advance.fX, advance.fY);

    if (!fRuns.empty()) {
        // TODO: Move it down to the TextWrapper to avoid extra calculations
        auto run = &fRuns[0];
        auto runShift = 0.0f;
        auto clusterShift = 0.0f;
        for (auto index = clusters.start; index < clusters.end; ++index) {
            auto& cluster = fClusters[index];
            if (cluster.runIndex() != run->index()) {
                run = &fRuns[cluster.runIndex()];
                runShift += clusterShift;
                clusterShift = 0;
            }
            clusterShift += cluster.width();
            for (auto i = cluster.startPos(); i < cluster.endPos(); ++i) {
                auto posX = run->posX(i);
                auto posY = run->posY(i);
                auto bounds = run->getBounds(i);
                bounds.offset(posX + runShift, posY);
                boundaries.joinPossiblyEmptyRect(bounds);
            }
        }
    }
    boundaries.offset(offset);
    fOrigin.joinPossiblyEmptyRect(boundaries);
}

TextLine& ParagraphImpl::addLine(SkVector offset,
                                 SkVector advance,
                                 TextRange text,
                                 TextRange textWithSpaces,
                                 ClusterRange clusters,
                                 ClusterRange clustersWithGhosts,
                                 SkScalar widthWithSpaces,
                                 InternalLineMetrics sizes) {
    // Define a list of styles that covers the line
    auto blocks = findAllBlocks(text);

    auto correctedOffset = offset;
    correctedOffset.offset(0, sizes.baseline());
    calculateBoundaries(clusters, correctedOffset, advance);

    return fLines.emplace_back(this, offset, advance, blocks, text, textWithSpaces, clusters, clustersWithGhosts, widthWithSpaces, sizes);
}

void ParagraphImpl::markGraphemes16() {

    if (!fGraphemes16.empty()) {
        return;
    }

    // This breaker gets called only once for a paragraph so we don't have to keep it
    TextBreaker breaker;
    if (!breaker.initialize(this->text(), UBRK_CHARACTER)) {
        return;
    }

    auto ptr = fText.c_str();
    auto end = fText.c_str() + fText.size();
    while (ptr < end) {

        size_t index = ptr - fText.c_str();
        SkUnichar u = SkUTF::NextUTF8(&ptr, end);
        uint16_t buffer[2];
        size_t count = SkUTF::ToUTF16(u, buffer);
        fCodePoints.emplace_back(EMPTY_INDEX, index, count > 1 ? 2 : 1);
        if (count > 1) {
            fCodePoints.emplace_back(EMPTY_INDEX, index, 1);
        }
    }

    CodepointRange codepoints(0ul, 0ul);

    size_t endPos = 0;
    while (!breaker.eof()) {
        auto startPos = endPos;
        endPos = breaker.next();

        // Collect all the codepoints that belong to the grapheme
        while (codepoints.end < fCodePoints.size() && fCodePoints[codepoints.end].fTextIndex < endPos) {
            ++codepoints.end;
        }

        if (startPos == endPos) {
            continue;
        }

        //SkDebugf("Grapheme #%d [%d:%d)\n", fGraphemes16.size(), startPos, endPos);

        // Update all the codepoints that belong to this grapheme
        for (auto i = codepoints.start; i < codepoints.end; ++i) {
            //SkDebugf("   [%d] = %d + %d\n", i, fCodePoints[i].fTextIndex, fCodePoints[i].fIndex);
            fCodePoints[i].fGrapheme = fGraphemes16.size();
        }

        fGraphemes16.emplace_back(codepoints, TextRange(startPos, endPos));
        codepoints.start = codepoints.end;
    }
}

void ParagraphImpl::markGraphemes() {

    // This breaker gets called only once for a paragraph so we don't have to keep it
    TextBreaker breaker;
    if (!breaker.initialize(this->text(), UBRK_CHARACTER)) {
        return;
    }

    auto endPos = breaker.first();
    while (!breaker.eof()) {
        fGraphemes.add(endPos);
        endPos = breaker.next();
    }
}

// Returns a vector of bounding boxes that enclose all text between
// start and end glyph indexes, including start and excluding end
std::vector<TextBox> ParagraphImpl::getRectsForRange(unsigned start,
                                                     unsigned end,
                                                     RectHeightStyle rectHeightStyle,
                                                     RectWidthStyle rectWidthStyle) {
    std::vector<TextBox> results;
    if (fText.isEmpty()) {
        if (start == 0 && end > 0) {
            // On account of implied "\n" that is always at the end of the text
            results.emplace_back(SkRect::MakeXYWH(0, 0, 0, fHeight), fParagraphStyle.getTextDirection());
        }
        return results;
    }

    markGraphemes16();

    if (start >= end || start > fCodePoints.size() || end == 0) {
        return results;
    }

    // Snap text edges to the code points/grapheme edges
    TextRange text(fText.size(), fText.size());

    if (start < fCodePoints.size()) {
        auto startGrapheme = fGraphemes16[fCodePoints[start].fGrapheme];
        auto lastGrapheme = fCodePoints[start].fGrapheme == fGraphemes16.size() - 1;
        if (start > startGrapheme.fCodepointRange.start) {
            if (end == startGrapheme.fCodepointRange.end &&
                start == startGrapheme.fCodepointRange.end - 1) {
                // This is a fix to make test GetRectsForRangeIncludeCombiningCharacter work
                // Must be removed...
                text.start = startGrapheme.fTextRange.start;
            } else {
                text.start  = lastGrapheme && end >= fCodePoints.size()
                        ? fCodePoints.back().fTextIndex
                        : startGrapheme.fTextRange.end;
            }
        } else {
            text.start = startGrapheme.fTextRange.start;
        }
    }

    if (end < fCodePoints.size()) {
        auto codepoint = fCodePoints[end];
        auto endGrapheme = fGraphemes16[fCodePoints[end].fGrapheme];
        if (text.start == endGrapheme.fTextRange.start &&
            end + codepoint.fIndex == fCodePoints.size()) {
            text.end = endGrapheme.fTextRange.end;
        } else {
            text.end  = endGrapheme.fTextRange.start;
        }
    }

    for (auto& line : fLines) {
        auto lineText = line.textWithSpaces();
        auto intersect = lineText * text;
        if (intersect.empty() && lineText.start != text.start) {
            continue;
        }

        // Found a line that intersects with the text
        auto firstBoxOnTheLine = results.size();
        auto paragraphTextDirection = paragraphStyle().getTextDirection();
        auto lineTextAlign = line.assumedTextAlign();
        const Run* lastRun = nullptr;
        line.iterateThroughVisualRuns(true,
            [&](const Run* run, SkScalar runOffset, TextRange textRange, SkScalar* width) {

                auto intersect = textRange * text;
                if (intersect.empty() || textRange.empty()) {
                    auto context = line.measureTextInsideOneRun(textRange, run, runOffset, 0, true, false);
                    *width = context.clip.width();
                    if (textRange.width() > 0) {
                        return true;
                    } else {
                        intersect = textRange;
                    }
                } else {
                    TextRange head;
                    if (run->leftToRight() && textRange.start != intersect.start) {
                        head = TextRange(textRange.start, intersect.start);
                        *width = line.measureTextInsideOneRun(head, run, runOffset, 0, true, false).clip.width();
                    } else if (!run->leftToRight() && textRange.end != intersect.end) {
                        head = TextRange(intersect.end, textRange.end);
                        *width = line.measureTextInsideOneRun(head, run, runOffset, 0, true, false).clip.width();
                    } else {
                        *width = 0;
                    }
                }

                runOffset += *width;

                // Found a run that intersects with the text
                auto context = line.measureTextInsideOneRun(intersect, run, runOffset, 0, true, true);
                *width += context.clip.width();

                SkRect clip = context.clip;
                SkRect trailingSpaces = SkRect::MakeEmpty();
                SkScalar ghostSpacesRight = context.run->leftToRight() ? clip.right() - line.width() : 0;
                SkScalar ghostSpacesLeft = !context.run->leftToRight() ? clip.right() - line.width() : 0;

                if (ghostSpacesRight + ghostSpacesLeft > 0) {
                    if (lineTextAlign == TextAlign::kLeft && ghostSpacesLeft > 0) {
                        clip.offset(-ghostSpacesLeft, 0);
                    } else if (lineTextAlign == TextAlign::kRight && ghostSpacesLeft > 0) {
                        clip.offset(-ghostSpacesLeft, 0);
                    } else if (lineTextAlign == TextAlign::kCenter) {
                        // TODO: What do we do for centering?
                    }
                }

                if (rectHeightStyle == RectHeightStyle::kMax) {
                    // TODO: Sort it out with Flutter people
                    clip.fBottom = line.height();
                    clip.fTop = line.sizes().baseline() -
                                line.getMaxRunMetrics().baseline() +
                                line.getMaxRunMetrics().delta();

                } else if (rectHeightStyle == RectHeightStyle::kIncludeLineSpacingTop) {
                    if (&line != &fLines.front()) {
                        clip.fTop -= line.sizes().runTop(context.run);
                    }
                    clip.fBottom -= line.sizes().runTop(context.run);
                } else if (rectHeightStyle == RectHeightStyle::kIncludeLineSpacingMiddle) {
                    if (&line != &fLines.front()) {
                        clip.fTop -= line.sizes().runTop(context.run) / 2;
                    }
                    if (&line == &fLines.back()) {
                        clip.fBottom -= line.sizes().runTop(context.run);
                    } else {
                        clip.fBottom -= line.sizes().runTop(context.run) / 2;
                    }
                } else if (rectHeightStyle == RectHeightStyle::kIncludeLineSpacingBottom) {
                    if (&line == &fLines.back()) {
                        clip.fBottom -= line.sizes().runTop(context.run);
                    }
                } else if (rectHeightStyle == RectHeightStyle::kStrut) {
                    auto strutStyle = this->paragraphStyle().getStrutStyle();
                    if (strutStyle.getStrutEnabled() && strutStyle.getFontSize() > 0) {
                        auto top = line.baseline() ; //+ line.sizes().runTop(run);
                        clip.fTop = top + fStrutMetrics.ascent();
                        clip.fBottom = top + fStrutMetrics.descent();
                    }
                }
                clip.offset(line.offset());

                // Check if we can merge two boxes
                bool mergedBoxes = false;
                if (!results.empty() &&
                    lastRun != nullptr && lastRun->placeholder() == nullptr && context.run->placeholder() == nullptr &&
                    lastRun->lineHeight() == context.run->lineHeight() &&
                    lastRun->font() == context.run->font()) {
                    auto& lastBox = results.back();
                    if (SkScalarNearlyEqual(lastBox.rect.fTop, clip.fTop) &&
                        SkScalarNearlyEqual(lastBox.rect.fBottom, clip.fBottom) &&
                            (SkScalarNearlyEqual(lastBox.rect.fLeft, clip.fRight) ||
                             SkScalarNearlyEqual(lastBox.rect.fRight, clip.fLeft))) {
                        lastBox.rect.fLeft = SkTMin(lastBox.rect.fLeft, clip.fLeft);
                        lastBox.rect.fRight = SkTMax(lastBox.rect.fRight, clip.fRight);
                        mergedBoxes = true;
                    }
                }
                lastRun = context.run;

                if (!mergedBoxes) {
                    results.emplace_back(
                        clip, context.run->leftToRight() ? TextDirection::kLtr : TextDirection::kRtl);
                }

                if (trailingSpaces.width() > 0) {
                    results.emplace_back(trailingSpaces, paragraphTextDirection);
                }

                return true;
            });

        if (rectWidthStyle == RectWidthStyle::kMax) {
            // Align the very left/right box horizontally
            auto lineStart = line.offset().fX;
            auto lineEnd = line.offset().fX + line.width();
            auto left = results.front();
            auto right = results.back();
            if (left.rect.fLeft > lineStart && left.direction == TextDirection::kRtl) {
                left.rect.fRight = left.rect.fLeft;
                left.rect.fLeft = 0;
                results.insert(results.begin() + firstBoxOnTheLine + 1, left);
            }
            if (right.direction == TextDirection::kLtr &&
                right.rect.fRight >= lineEnd &&  right.rect.fRight < this->fMaxWidthWithTrailingSpaces) {
                right.rect.fLeft = right.rect.fRight;
                right.rect.fRight = this->fMaxWidthWithTrailingSpaces;
                results.emplace_back(right);
            }
        }

        for (auto& r : results) {

          r.rect.fLeft = littleRound(r.rect.fLeft);
          r.rect.fRight = littleRound(r.rect.fRight);
          r.rect.fTop = littleRound(r.rect.fTop);
          r.rect.fBottom = littleRound(r.rect.fBottom);
        }
    }

    return results;
}

std::vector<TextBox> ParagraphImpl::getRectsForPlaceholders() {
  std::vector<TextBox> boxes;
  if (fText.isEmpty()) {
       return boxes;
  }
  for (auto& line : fLines) {
      line.iterateThroughVisualRuns(
              true,
              [&boxes, &line](const Run* run, SkScalar runOffset, TextRange textRange,
                              SkScalar* width) {
                  auto context =
                          line.measureTextInsideOneRun(textRange, run, runOffset, 0, true, false);
                  *width = context.clip.width();
                  if (run->placeholder() == nullptr) {
                      return true;
                  }
                  if (run->textRange().width() == 0) {
                      return true;
                  }
                  SkRect clip = context.clip;
                  clip.offset(line.offset());

                  clip.fLeft = littleRound(clip.fLeft);
                  clip.fRight = littleRound(clip.fRight);
                  clip.fTop = littleRound(clip.fTop);
                  clip.fBottom = littleRound(clip.fBottom);
                  boxes.emplace_back(
                          clip, run->leftToRight() ? TextDirection::kLtr : TextDirection::kRtl);
                  return true;
              });
  }

  return boxes;
}

// TODO: Deal with RTL here
PositionWithAffinity ParagraphImpl::getGlyphPositionAtCoordinate(SkScalar dx, SkScalar dy) {

    PositionWithAffinity result(0, Affinity::kDownstream);
    if (fText.isEmpty()) {
        return result;
    }

    markGraphemes16();
    for (auto& line : fLines) {
        // Let's figure out if we can stop looking
        auto offsetY = line.offset().fY;
        if (dy > offsetY + line.height() && &line != &fLines.back()) {
            // This line is not good enough
            continue;
        }

        // This is so far the the line vertically closest to our coordinates
        // (or the first one, or the only one - all the same)
        line.iterateThroughVisualRuns(true,
            [this, &line, dx, &result]
            (const Run* run, SkScalar runOffset, TextRange textRange, SkScalar* width) {

                auto offsetX = line.offset().fX;
                auto context = line.measureTextInsideOneRun(textRange, run, 0, 0, true, false);
                if (dx < context.clip.fLeft + offsetX) {
                    // All the other runs are placed right of this one
                    result = { SkToS32(context.run->fClusterIndexes[context.pos]), kDownstream };
                    return false;
                }

                if (dx >= context.clip.fRight) {
                    // We have to keep looking but just in case keep the last one as the closes
                    // so far
                    auto index = context.pos + context.size;
                    if (index < context.run->size()) {
                        result = { SkToS32(context.run->fClusterIndexes[index]), kUpstream };
                    } else {
                        // Take the last cluster on that line
                        result = { SkToS32(line.clusters().end), kUpstream };
                    }
                    return true;
                }

                // So we found the run that contains our coordinates
                // Find the glyph position in the run that is the closest left of our point
                // TODO: binary search
                size_t found = context.pos;
                for (size_t i = context.pos; i < context.pos + context.size; ++i) {
                    // TODO: this rounding is done to match Flutter tests. Must be removed..
                    auto end = littleRound(context.run->positionX(i) + context.fTextShift + offsetX);
                    if (end > dx) {
                        break;
                    }
                    found = i;
                }
                auto glyphStart = context.run->positionX(found);
                auto glyphWidth = context.run->positionX(found + 1) - context.run->positionX(found);
                auto clusterIndex8 = context.run->fClusterIndexes[found];

                // Find the grapheme positions in codepoints that contains the point
                auto codepoint = std::lower_bound(
                    fCodePoints.begin(), fCodePoints.end(),
                    clusterIndex8,
                    [](const Codepoint& lhs,size_t rhs) -> bool { return lhs.fTextIndex < rhs; });

                auto codepointIndex = codepoint - fCodePoints.begin();
                auto codepoints = fGraphemes16[codepoint->fGrapheme].fCodepointRange;
                auto graphemeSize = codepoints.width();

                // We only need to inspect one glyph (maybe not even the entire glyph)
                SkScalar center;
                if (graphemeSize > 1) {
                    auto averageCodepoint = glyphWidth / graphemeSize;
                    auto codepointStart = glyphStart + averageCodepoint * (codepointIndex - codepoints.start);
                    auto codepointEnd = codepointStart + averageCodepoint;
                    center = (codepointStart + codepointEnd) / 2 + context.fTextShift;
                } else {
                    SkASSERT(graphemeSize == 1);
                    auto codepointStart = glyphStart;
                    auto codepointEnd = codepointStart + glyphWidth;
                    center = (codepointStart + codepointEnd) / 2 + context.fTextShift;
                }

                if ((dx < center) == context.run->leftToRight()) {
                    result = { SkToS32(codepointIndex), kDownstream };
                } else {
                    result = { SkToS32(codepointIndex + 1), kUpstream };
                }
                // No need to continue
                return false;
            });

        if (dy < offsetY + line.height()) {
            // The closest position on this line; next line is going to be even lower
            break;
        }
    }

    // SkDebugf("getGlyphPositionAtCoordinate(%f,%f) = %d\n", dx, dy, result.position);
    return result;
}

// Finds the first and last glyphs that define a word containing
// the glyph at index offset.
// By "glyph" they mean a character index - indicated by Minikin's code
SkRange<size_t> ParagraphImpl::getWordBoundary(unsigned offset) {

    if (fWords.empty()) {
      auto unicode = icu::UnicodeString::fromUTF8(fText.c_str());

      UErrorCode errorCode = U_ZERO_ERROR;

      auto iter = ubrk_open(UBRK_WORD, icu::Locale().getName(), nullptr, 0, &errorCode);
      if (U_FAILURE(errorCode)) {
        SkDEBUGF("Could not create line break iterator: %s", u_errorName(errorCode));
        return { 0, 0 };
      }

      UText sUtf16UText = UTEXT_INITIALIZER;
      ICUUText utf16UText(utext_openUnicodeString(&sUtf16UText, &unicode, &errorCode));
      if (U_FAILURE(errorCode)) {
        SkDEBUGF("Could not create utf8UText: %s", u_errorName(errorCode));
        return { 0, 0 };
      }

      ubrk_setUText(iter, utf16UText.get(), &errorCode);
      if (U_FAILURE(errorCode)) {
        SkDEBUGF("Could not setText on break iterator: %s", u_errorName(errorCode));
        return { 0, 0 };
      }

        int32_t pos = ubrk_first(iter);
        while (pos != icu::BreakIterator::DONE) {
            fWords.emplace_back(pos);
            pos = ubrk_next(iter);
        }
    }

    int32_t start = 0;
    int32_t end = 0;
    for (size_t i = 0; i < fWords.size(); ++i) {
      auto word = fWords[i];
      if (word <= offset) {
        start = word;
        end = word;
      } else if (word > offset) {
        end = word;
        break;
      }
    }

    return { SkToU32(start), SkToU32(end) };
}

void ParagraphImpl::getLineMetrics(std::vector<LineMetrics>& metrics) {
    metrics.clear();
    for (auto& line : fLines) {
        metrics.emplace_back(line.getMetrics());
    }
}

SkSpan<const char> ParagraphImpl::text(TextRange textRange) {
    SkASSERT(textRange.start <= fText.size() && textRange.end <= fText.size());
    auto start = fText.c_str() + textRange.start;
    return SkSpan<const char>(start, textRange.width());
}

SkSpan<Cluster> ParagraphImpl::clusters(ClusterRange clusterRange) {
    SkASSERT(clusterRange.start < fClusters.size() && clusterRange.end <= fClusters.size());
    return SkSpan<Cluster>(&fClusters[clusterRange.start], clusterRange.width());
}

Cluster& ParagraphImpl::cluster(ClusterIndex clusterIndex) {
    SkASSERT(clusterIndex < fClusters.size());
    return fClusters[clusterIndex];
}

Run& ParagraphImpl::run(RunIndex runIndex) {
    SkASSERT(runIndex < fRuns.size());
    return fRuns[runIndex];
}

Run& ParagraphImpl::runByCluster(ClusterIndex clusterIndex) {
    auto start = cluster(clusterIndex);
    return this->run(start.fRunIndex);
}

SkSpan<Block> ParagraphImpl::blocks(BlockRange blockRange) {
    SkASSERT(blockRange.start < fTextStyles.size() && blockRange.end <= fTextStyles.size());
    return SkSpan<Block>(&fTextStyles[blockRange.start], blockRange.width());
}

Block& ParagraphImpl::block(BlockIndex blockIndex) {
    SkASSERT(blockIndex < fTextStyles.size());
    return fTextStyles[blockIndex];
}

// TODO: Cache this information
void ParagraphImpl::resetRunShifts() {
    fRunShifts.resize(fRuns.size());
    for (size_t i = 0; i < fRuns.size(); ++i) {
        fRunShifts[i].fShifts.push_back_n(fRuns[i].size() + 1, 0.0);
    }
}

void ParagraphImpl::setState(InternalState state) {
    if (fState <= state) {
        fState = state;
        return;
    }

    fState = state;
    switch (fState) {
        case kUnknown:
            fRuns.reset();
        case kShaped:
            fClusters.reset();
        case kClusterized:
        case kMarked:
        case kLineBroken:
            this->resetContext();
            this->resolveStrut();
            this->fRunShifts.reset();
            fLines.reset();
        case kFormatted:
            fPicture = nullptr;
        case kDrawn:
            break;
    default:
        break;
    }

}

InternalLineMetrics ParagraphImpl::computeEmptyMetrics() {

  auto defaultTextStyle = paragraphStyle().getTextStyle();

  auto typefaces = fontCollection()->findTypefaces(
      defaultTextStyle.getFontFamilies(), defaultTextStyle.getFontStyle());
  auto typeface = typefaces.size() ? typefaces.front() : nullptr;

  SkFont font(typeface, defaultTextStyle.getFontSize());
  InternalLineMetrics metrics(font, paragraphStyle().getStrutStyle().getForceStrutHeight());
  fStrutMetrics.updateLineMetrics(metrics);

  return metrics;
}

void ParagraphImpl::updateText(size_t from, SkString text) {
  fText.remove(from, from + text.size());
  fText.insert(from, text);
  fState = kUnknown;
  fOldWidth = 0;
  fOldHeight = 0;
}

void ParagraphImpl::updateFontSize(size_t from, size_t to, SkScalar fontSize) {

  SkASSERT(from == 0 && to == fText.size());
  auto defaultStyle = fParagraphStyle.getTextStyle();
  defaultStyle.setFontSize(fontSize);
  fParagraphStyle.setTextStyle(defaultStyle);

  for (auto& textStyle : fTextStyles) {
    textStyle.fStyle.setFontSize(fontSize);
  }

  fState = kUnknown;
  fOldWidth = 0;
  fOldHeight = 0;
}

void ParagraphImpl::updateTextAlign(TextAlign textAlign) {
    fParagraphStyle.setTextAlign(textAlign);

    if (fState >= kLineBroken) {
        fState = kLineBroken;
    }
}

void ParagraphImpl::updateForegroundPaint(size_t from, size_t to, SkPaint paint) {
    SkASSERT(from == 0 && to == fText.size());
    auto defaultStyle = fParagraphStyle.getTextStyle();
    defaultStyle.setForegroundColor(paint);
    fParagraphStyle.setTextStyle(defaultStyle);

    for (auto& textStyle : fTextStyles) {
        textStyle.fStyle.setForegroundColor(paint);
    }
}

void ParagraphImpl::updateBackgroundPaint(size_t from, size_t to, SkPaint paint) {
    SkASSERT(from == 0 && to == fText.size());
    auto defaultStyle = fParagraphStyle.getTextStyle();
    defaultStyle.setBackgroundColor(paint);
    fParagraphStyle.setTextStyle(defaultStyle);

    for (auto& textStyle : fTextStyles) {
        textStyle.fStyle.setBackgroundColor(paint);
    }
}

}  // namespace textlayout
}  // namespace skia
