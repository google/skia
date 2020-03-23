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
#include <unicode/ustring.h>
#include <algorithm>
#include <chrono>
#include <queue>

namespace skia {
namespace textlayout {

namespace {

using ICUUText = std::unique_ptr<UText, SkFunctionWrapper<decltype(utext_close), utext_close>>;
using ICUBiDi  = std::unique_ptr<UBiDi, SkFunctionWrapper<decltype(ubidi_close), ubidi_close>>;

SkScalar littleRound(SkScalar a) {
    // This rounding is done to match Flutter tests. Must be removed..
  return SkScalarRoundToScalar(a * 100.0)/100.0;
}

/** Replaces invalid utf-8 sequences with REPLACEMENT CHARACTER U+FFFD. */
static inline SkUnichar utf8_next(const char** ptr, const char* end) {
    SkUnichar val = SkUTF::NextUTF8(ptr, end);
    return val < 0 ? 0xFFFD : val;
}

}

TextRange operator*(const TextRange& a, const TextRange& b) {
    if (a.start == b.start && a.end == b.end) return a;
    auto begin = std::max(a.start, b.start);
    auto end = std::min(a.end, b.end);
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
        this->fClusters.reset();
        this->resetShifts();
    } else if (fState >= kLineBroken && (fOldWidth != floorWidth || fOldHeight != fHeight)) {
        // We can use the results from SkShaper but have to do EVERYTHING ELSE again
        this->fClusters.reset();
        this->resetShifts();
        fState = kShaped;
    }

    if (fState < kShaped) {
        fGraphemes.reset();
        this->markGraphemes();

        if (!this->shapeTextIntoEndlessLine()) {
            this->resetContext();
            // TODO: merge the two next calls - they always come together
            this->resolveStrut();
            this->computeEmptyMetrics();
            this->fLines.reset();

            // Set the important values that are not zero
            fWidth = floorWidth;
            fHeight = fEmptyMetrics.height();
            if (fParagraphStyle.getStrutStyle().getStrutEnabled() &&
                fParagraphStyle.getStrutStyle().getForceStrutHeight()) {
                fHeight = fStrutMetrics.height();
            }
            fAlphabeticBaseline = fEmptyMetrics.alphabeticBaseline();
            fIdeographicBaseline = fEmptyMetrics.ideographicBaseline();
            fMinIntrinsicWidth = 0;
            fMaxIntrinsicWidth = 0;
            this->fOldWidth = floorWidth;
            this->fOldHeight = this->fHeight;

            return;
        }

        this->fClusters.reset();
        this->resetShifts();
        fState = kShaped;
    }

    if (fState < kMarked) {
        this->buildClusterTable();
        fState = kClusterized;

        this->markLineBreaks();
        fState = kMarked;
    }

    if (fState >= kLineBroken) {
        if (fOldWidth != floorWidth || fOldHeight != fHeight) {
            fState = kMarked;
        }
    }

    if (fState < kLineBroken) {
        this->resetContext();
        this->resolveStrut();
        this->computeEmptyMetrics();
        this->fLines.reset();
        this->breakShapedTextIntoLines(floorWidth);
        fState = kLineBroken;
    }

    if (fState < kFormatted) {
        // Build the picture lazily not until we actually have to paint (or never)
        this->formatLines(fWidth);
        // We have to calculate the paragraph boundaries only after we format the lines
        this->calculateBoundaries();
        fState = kFormatted;
    }

    this->fOldWidth = floorWidth;
    this->fOldHeight = this->fHeight;

    // TODO: This rounding is done to match Flutter tests. Must be removed...
    fMinIntrinsicWidth = littleRound(fMinIntrinsicWidth);
    fMaxIntrinsicWidth = littleRound(fMaxIntrinsicWidth);

    // TODO: This is strictly Flutter thing. Must be factored out into some flutter code
    if (fParagraphStyle.getMaxLines() == 1 ||
        (fParagraphStyle.unlimited_lines() && fParagraphStyle.ellipsized())) {
        fMinIntrinsicWidth = fMaxIntrinsicWidth;
    }
    //SkDebugf("layout('%s', %f): %f %f\n", fText.c_str(), rawWidth, fMinIntrinsicWidth, fMaxIntrinsicWidth);
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
            auto& cluster = fClusters.emplace_back(this, runIndex, 0ul, 1ul, text, run.advance().fX,
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
                if (fGraphemes.find(cluster.fTextRange.end) != nullptr) {
                    cluster.setBreakType(Cluster::BreakType::GraphemeBreak);
                }
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

    // Mark all soft line breaks
    // Remove soft line breaks that are not on grapheme cluster edge
    Cluster* current = fClusters.begin();
    while (!breaker.eof() && current < fClusters.end()) {
        size_t currentPos = breaker.next();
        while (current < fClusters.end()) {
            if (current->textRange().end > currentPos) {
                break;
            } else if (current->textRange().end == currentPos) {
                if (breaker.status() == UBRK_LINE_HARD) {
                    // Hard line break stronger than anything
                    current->setBreakType(Cluster::BreakType::HardLineBreak);
                } else if (current->isGraphemeBreak()) {
                    // Only allow soft line break if it's grapheme break
                    current->setBreakType(Cluster::BreakType::SoftLineBreak);
                } else {
                    // Leave it as is (either it's no break or a placeholder)
                }
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
        return true;
    }

    fFontSwitches.reset();

    OneLineShaper oneLineShaper(this);
    auto result = oneLineShaper.shape();
    fUnresolvedGlyphs = oneLineShaper.unresolvedGlyphs();

    if (!result) {
        return false;
    } else {
        // Add the paragraph to the cache
        fFontCollection->getParagraphCache()->updateParagraph(this);
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
                // TODO: Take in account clipped edges
                auto& line = this->addLine(offset, advance, text, textWithSpaces, clusters, clustersWithGhosts, widthWithSpaces, metrics);
                if (addEllipsis) {
                    line.createEllipsis(maxWidth, fParagraphStyle.getEllipsis(), true);
                    if (line.ellipsis() != nullptr) {
                        // Make sure the paragraph boundaries include its ellipsis
                        auto size = line.ellipsis()->advance();
                        auto offset = line.ellipsis()->offset();
                        SkRect boundaries = SkRect::MakeXYWH(offset.fX, offset.fY, size.fX, size.fY);
                        fOrigin.joinPossiblyEmptyRect(boundaries);
                    }
                }

                fLongestLine = std::max(fLongestLine, nearlyZero(advance.fX) ? widthWithSpaces : advance.fX);
            });
    fHeight = textWrapper.height();
    fWidth = maxWidth;
    fMaxIntrinsicWidth = textWrapper.maxIntrinsicWidth();
    fMinIntrinsicWidth = textWrapper.minIntrinsicWidth();
    fAlphabeticBaseline = fLines.empty() ? fEmptyMetrics.alphabeticBaseline() : fLines.front().alphabeticBaseline();
    fIdeographicBaseline = fLines.empty() ? fEmptyMetrics.ideographicBaseline() : fLines.front().ideographicBaseline();
    fExceededMaxLines = textWrapper.exceededMaxLines();
}

void ParagraphImpl::formatLines(SkScalar maxWidth) {
    auto effectiveAlign = fParagraphStyle.effective_align();

    if (!SkScalarIsFinite(maxWidth) && effectiveAlign != TextAlign::kLeft) {
        // Special case: clean all text in case of maxWidth == INF & align != left
        // We had to go through shaping though because we need all the measurement numbers
        fLines.reset();
        return;
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

void ParagraphImpl::calculateBoundaries() {
    for (auto& line : fLines) {
        fOrigin.joinPossiblyEmptyRect(line.calculateBoundaries());
    }
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
            //SkDebugf("getRectsForRange(%d, %d): %f\n", start, end, fHeight);
            results.emplace_back(SkRect::MakeXYWH(0, 0, 0, fHeight), fParagraphStyle.getTextDirection());
        }
        return results;
    }

    markGraphemes16();

    if (start >= end || start > fCodePoints.size() || end == 0) {
        return results;
    }

    // Adjust the text to grapheme edges
    // Apparently, text editor CAN move inside graphemes but CANNOT select a part of it.
    // I don't know why - the solution I have here returns an empty box for every query that
    // does not contain an end of a grapheme.
    // Once a cursor is inside a complex grapheme I can press backspace and cause trouble.
    // To avoid any problems, I will not allow any selection of a part of a grapheme.
    // One flutter test fails because of it but the editing experience is correct
    // (although you have to press the cursor many times before it moves to the next grapheme).
    TextRange text(fText.size(), fText.size());
    if (start < fCodePoints.size()) {
        auto codepoint = fCodePoints[start];
        auto grapheme = fGraphemes16[codepoint.fGrapheme];
        text.start = grapheme.fTextRange.start;
    }

    if (end < fCodePoints.size()) {
        auto codepoint = fCodePoints[end];
        auto grapheme = fGraphemes16[codepoint.fGrapheme];
        text.end = grapheme.fTextRange.start;
    }

    auto firstBoxOnTheLine = results.size();
    const Run* lastRun = nullptr;
    auto paragraphTextDirection = paragraphStyle().getTextDirection();
    for (auto& line : fLines) {
        auto lineText = line.textWithSpaces();
        auto intersect = lineText * text;
        if (intersect.empty() && lineText.start != text.start) {
            continue;
        }

        line.iterateThroughVisualRuns(true,
            [&]
            (const Run* run, SkScalar runOffsetInLine, TextRange textRange, SkScalar* runWidthInLine) {
            *runWidthInLine = line.iterateThroughSingleRunByStyles(
            run, runOffsetInLine, textRange, StyleType::kNone,
            [&]
            (TextRange textRange, const TextStyle& style, const TextLine::ClipContext& context0) {

                auto intersect = textRange * text;
                if (intersect.empty()) {
                    return true;
                }

                // Found a run that intersects with the text
                auto context = line.measureTextInsideOneRun(intersect, run, runOffsetInLine, 0, true, true);
                SkRect clip = context.clip;
                clip.offset(context0.fTextShift - context.fTextShift, 0);

                if (rectHeightStyle == RectHeightStyle::kMax) {
                    // TODO: Change it once flutter rolls into google3
                    //  (probably will break things if changed before)
                    clip.fBottom = line.height();
                    clip.fTop = line.sizes().delta();

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
                        auto top = line.baseline();
                        clip.fTop = top + fStrutMetrics.ascent();
                        clip.fBottom = top + fStrutMetrics.descent();
                    }
                }

                // Separate trailing spaces and move them in the default order of the paragraph
                // in case the run order and the paragraph order don't match
                SkRect trailingSpaces = SkRect::MakeEmpty();
                if (line.trimmedText().end < line.textWithSpaces().end && // Line has trailing spaces
                    line.textWithSpaces().end == intersect.end &&         // Range is at the end of the line
                    line.trimmedText().end > intersect.start)             // Range has more than just spaces
                {
                    auto delta = line.spacesWidth();
                    trailingSpaces = SkRect::MakeXYWH(0, 0, 0, 0);
                    // There are trailing spaces in this run
                    if (this->paragraphStyle().getTextAlign() == TextAlign::kJustify &&
                        &line != &fLines.back())
                    {
                        // TODO: this is just a patch. Make it right later (when it's clear what and how)
                        trailingSpaces = clip;
                        trailingSpaces.fLeft = line.width();
                        clip.fRight = line.width();
                    } else if (this->fParagraphStyle.getTextDirection() == TextDirection::kRtl &&
                        !run->leftToRight())
                    {
                        // Split
                        trailingSpaces = clip;
                        trailingSpaces.fLeft = - delta;
                        trailingSpaces.fRight = 0;
                        clip.fLeft += delta;
                    } else if (this->fParagraphStyle.getTextDirection() == TextDirection::kLtr &&
                        run->leftToRight())
                    {
                        // Split
                        trailingSpaces = clip;
                        trailingSpaces.fLeft = line.width();
                        trailingSpaces.fRight = trailingSpaces.fLeft + delta;
                        clip.fRight -= delta;
                    }
                }

                clip.offset(line.offset());
                if (trailingSpaces.width() > 0) {
                    trailingSpaces.offset(line.offset());
                }

                // Check if we can merge two boxes instead of adding a new one
                auto merge = [&lastRun, &context, &results](SkRect clip) {
                    bool mergedBoxes = false;
                    if (!results.empty() &&
                        lastRun != nullptr &&
                        lastRun->placeholderStyle() == nullptr &&
                        context.run->placeholderStyle() == nullptr &&
                        nearlyEqual(lastRun->lineHeight(), context.run->lineHeight()) &&
                        lastRun->font() == context.run->font())
                    {
                        auto& lastBox = results.back();
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
                    results.emplace_back(
                        clip, context.run->leftToRight() ? TextDirection::kLtr : TextDirection::kRtl);
                }
                if (!nearlyZero(trailingSpaces.width()) && !merge(trailingSpaces)) {
                    results.emplace_back(trailingSpaces, paragraphTextDirection);
                }
                return true;
            });
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
/*
    SkDebugf("getRectsForRange(%d, %d)\n", start, end);
    for (auto& r : results) {
        r.rect.fLeft = littleRound(r.rect.fLeft);
        r.rect.fRight = littleRound(r.rect.fRight);
        r.rect.fTop = littleRound(r.rect.fTop);
        r.rect.fBottom = littleRound(r.rect.fBottom);
        SkDebugf("[%f:%f * %f:%f]\n", r.rect.fLeft, r.rect.fRight, r.rect.fTop, r.rect.fBottom);
    }
*/
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
                  if (run->placeholderStyle() == nullptr) {
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

// TODO: Optimize (save cluster <-> codepoint connection)
PositionWithAffinity ParagraphImpl::getGlyphPositionAtCoordinate(SkScalar dx, SkScalar dy) {

    PositionWithAffinity result(0, Affinity::kDownstream);
    if (fText.isEmpty()) {
        return result;
    }

    markGraphemes16();
    for (auto& line : fLines) {
        // Let's figure out if we can stop looking
        auto offsetY = line.offset().fY;
        if (dy >= offsetY + line.height() && &line != &fLines.back()) {
            // This line is not good enough
            continue;
        }

        // This is so far the the line vertically closest to our coordinates
        // (or the first one, or the only one - all the same)
        line.iterateThroughVisualRuns(true,
                [this, &line, dx, &result]
                (const Run* run, SkScalar runOffsetInLine, TextRange textRange, SkScalar* runWidthInLine) {
                bool lookingForHit = true;
                *runWidthInLine = line.iterateThroughSingleRunByStyles(
                run, runOffsetInLine, textRange, StyleType::kNone,
                [this, line, dx, &result, &lookingForHit]
                (TextRange textRange, const TextStyle& style, const TextLine::ClipContext& context) {

                    auto findCodepointByTextIndex = [this](ClusterIndex clusterIndex8) {
                        auto codepoint = std::lower_bound(
                            fCodePoints.begin(), fCodePoints.end(),
                            clusterIndex8,
                            [](const Codepoint& lhs,size_t rhs) -> bool { return lhs.fTextIndex < rhs; });

                        return codepoint - fCodePoints.begin();
                    };

                    auto offsetX = line.offset().fX;
                    if (dx < context.clip.fLeft + offsetX) {
                        // All the other runs are placed right of this one
                        auto codepointIndex = findCodepointByTextIndex(context.run->globalClusterIndex(context.pos));
                        result = { SkToS32(codepointIndex), kDownstream };
                        lookingForHit = false;
                        return false;
                    }

                    if (dx >= context.clip.fRight + offsetX) {
                        // We have to keep looking ; just in case keep the last one as the closest
                        auto codepointIndex = findCodepointByTextIndex(context.run->globalClusterIndex(context.pos + context.size));
                        result = { SkToS32(codepointIndex), kUpstream };
                        return true;
                    }

                    // So we found the run that contains our coordinates
                    // Find the glyph position in the run that is the closest left of our point
                    // TODO: binary search
                    size_t found = context.pos;
                    for (size_t index = context.pos; index < context.pos + context.size; ++index) {
                        // TODO: this rounding is done to match Flutter tests. Must be removed..
                        auto end = littleRound(context.run->positionX(index) + context.fTextShift + offsetX);
                        if (end > dx) {
                            break;
                        }
                        found = index;
                    }

                    auto glyphStart = context.run->positionX(found) + context.fTextShift + offsetX;
                    auto glyphWidth = context.run->positionX(found + 1) - context.run->positionX(found);
                    auto clusterIndex8 = context.run->globalClusterIndex(found);
                    auto clusterEnd8 = context.run->globalClusterIndex(found + 1);

                    // Find the grapheme positions in codepoints that contains the point
                    auto codepointIndex = findCodepointByTextIndex(clusterIndex8);
                    CodepointRange codepoints(codepointIndex, codepointIndex);
                    if (context.run->leftToRight()) {
                        for (codepoints.end = codepointIndex;
                             codepoints.end < fCodePoints.size(); ++codepoints.end) {
                            auto& cp = fCodePoints[codepoints.end];
                            if (cp.fTextIndex >= clusterEnd8) {
                                break;
                            }
                        }
                    } else {
                        for (codepoints.end = codepointIndex;
                             codepoints.end > 0; --codepoints.end) {
                            auto& cp = fCodePoints[codepoints.end];
                            if (cp.fTextIndex <= clusterEnd8) {
                                break;
                            }
                        }
                        std::swap(codepoints.start, codepoints.end);
                    }

                    auto graphemeSize = codepoints.width();

                    // We only need to inspect one glyph (maybe not even the entire glyph)
                    SkScalar center;
                    bool insideGlyph = false;
                    if (graphemeSize > 1) {
                        auto averageCodepointWidth = glyphWidth / graphemeSize;
                        auto delta = dx - glyphStart;
                        auto insideIndex = SkScalarFloorToInt(delta / averageCodepointWidth);
                        insideGlyph = delta > averageCodepointWidth;
                        center = glyphStart + averageCodepointWidth * insideIndex + averageCodepointWidth / 2;
                        codepointIndex += insideIndex;
                    } else {
                        center = glyphStart + glyphWidth / 2;
                    }
                    if ((dx < center) == context.run->leftToRight() || insideGlyph) {
                        result = { SkToS32(codepointIndex), kDownstream };
                    } else {
                        result = { SkToS32(codepointIndex + 1), kUpstream };
                    }
                    // No need to continue
                    lookingForHit = false;
                    return false;

                });
                return lookingForHit;
            });
        break;
    }
    //SkDebugf("getGlyphPositionAtCoordinate(%f, %f): %d %s\n", dx, dy, result.position,
    //   result.affinity == Affinity::kUpstream ? "up" : "down");
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
            return {0, 0};
        }

        UText sUtf16UText = UTEXT_INITIALIZER;
        ICUUText utf16UText(utext_openUnicodeString(&sUtf16UText, &unicode, &errorCode));
        if (U_FAILURE(errorCode)) {
            SkDEBUGF("Could not create utf8UText: %s", u_errorName(errorCode));
            return {0, 0};
        }

        ubrk_setUText(iter, utf16UText.get(), &errorCode);
        if (U_FAILURE(errorCode)) {
            SkDEBUGF("Could not setText on break iterator: %s", u_errorName(errorCode));
            return {0, 0};
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
    //SkDebugf("getWordBoundary(%d): %d - %d\n", offset, start, end);
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
            this->computeEmptyMetrics();
            this->resetShifts();
            fLines.reset();
        case kFormatted:
            fPicture = nullptr;
        case kDrawn:
            break;
    default:
        break;
    }
}

void ParagraphImpl::computeEmptyMetrics() {
    auto defaultTextStyle = paragraphStyle().getTextStyle();

    auto typefaces = fontCollection()->findTypefaces(
      defaultTextStyle.getFontFamilies(), defaultTextStyle.getFontStyle());
    auto typeface = typefaces.empty() ? nullptr : typefaces.front();

    SkFont font(typeface, defaultTextStyle.getFontSize());

    fEmptyMetrics = InternalLineMetrics(font, paragraphStyle().getStrutStyle().getForceStrutHeight());
    if (!paragraphStyle().getStrutStyle().getForceStrutHeight() &&
        defaultTextStyle.getHeightOverride()) {
        auto multiplier =
                defaultTextStyle.getHeight() * defaultTextStyle.getFontSize() / fEmptyMetrics.height();
        fEmptyMetrics = InternalLineMetrics(fEmptyMetrics.ascent() * multiplier,
                                      fEmptyMetrics.descent() * multiplier,
                                      fEmptyMetrics.leading() * multiplier);
    }

    if (fParagraphStyle.getStrutStyle().getStrutEnabled()) {
        fStrutMetrics.updateLineMetrics(fEmptyMetrics);
    }
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

bool ParagraphImpl::calculateBidiRegions(SkTArray<BidiRegion>* regions) {

    regions->reset();

    // ubidi only accepts utf16 (though internally it basically works on utf32 chars).
    // We want an ubidi_setPara(UBiDi*, UText*, UBiDiLevel, UBiDiLevel*, UErrorCode*);
    size_t utf8Bytes = fText.size();
    const char* utf8 = fText.c_str();
    uint8_t bidiLevel = fParagraphStyle.getTextDirection() == TextDirection::kLtr
                            ? UBIDI_LTR
                            : UBIDI_RTL;
    if (!SkTFitsIn<int32_t>(utf8Bytes)) {
        SkDEBUGF("Bidi error: text too long");
        return false;
    }

    // Getting the length like this seems to always set U_BUFFER_OVERFLOW_ERROR
    UErrorCode status = U_ZERO_ERROR;
    int32_t utf16Units;
    u_strFromUTF8(nullptr, 0, &utf16Units, utf8, utf8Bytes, &status);
    status = U_ZERO_ERROR;
    std::unique_ptr<UChar[]> utf16(new UChar[utf16Units]);
    u_strFromUTF8(utf16.get(), utf16Units, nullptr, utf8, utf8Bytes, &status);
    if (U_FAILURE(status)) {
        SkDEBUGF("Invalid utf8 input: %s", u_errorName(status));
        return false;
    }

    ICUBiDi bidi(ubidi_openSized(utf16Units, 0, &status));
    if (U_FAILURE(status)) {
        SkDEBUGF("Bidi error: %s", u_errorName(status));
        return false;
    }
    SkASSERT(bidi);

    // The required lifetime of utf16 isn't well documented.
    // It appears it isn't used after ubidi_setPara except through ubidi_getText.
    ubidi_setPara(bidi.get(), utf16.get(), utf16Units, bidiLevel, nullptr, &status);
    if (U_FAILURE(status)) {
        SkDEBUGF("Bidi error: %s", u_errorName(status));
        return false;
    }

    SkTArray<BidiRegion> bidiRegions;
    const char* start8 = utf8;
    const char* end8 = utf8 + utf8Bytes;
    TextRange textRange(0, 0);
    UBiDiLevel currentLevel = 0;

    int32_t pos16 = 0;
    int32_t end16 = ubidi_getLength(bidi.get());
    while (pos16 < end16) {
        auto level = ubidi_getLevelAt(bidi.get(), pos16);
        if (pos16 == 0) {
            currentLevel = level;
        } else if (level != currentLevel) {
            textRange.end = start8 - utf8;
            regions->emplace_back(textRange.start, textRange.end, currentLevel);
            currentLevel = level;
            textRange = TextRange(textRange.end, textRange.end);
        }
        SkUnichar u = utf8_next(&start8, end8);
        pos16 += SkUTF::ToUTF16(u);
    }

    textRange.end = start8 - utf8;
    if (!textRange.empty()) {
        regions->emplace_back(textRange.start, textRange.end, currentLevel);
    }

    return true;
}

}  // namespace textlayout
}  // namespace skia
