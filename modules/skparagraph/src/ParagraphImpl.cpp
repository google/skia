// Copyright 2019 Google LLC.

#include "include/core/SkCanvas.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypeface.h"
#include "include/private/SkTFitsIn.h"
#include "include/private/SkTo.h"
#include "modules/skparagraph/include/Metrics.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/OneLineShaper.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/Run.h"
#include "modules/skparagraph/src/TextLine.h"
#include "modules/skparagraph/src/TextWrapper.h"
#include "src/utils/SkUTF.h"
#include <math.h>
#include <algorithm>
#include <utility>


namespace skia {
namespace textlayout {

namespace {

SkScalar littleRound(SkScalar a) {
    // This rounding is done to match Flutter tests. Must be removed..
    auto val = std::fabs(a);
    if (val < 10000) {
        return SkScalarRoundToScalar(a * 100.0)/100.0;
    } else if (val < 100000) {
        return SkScalarRoundToScalar(a * 10.0)/10.0;
    } else {
        return SkScalarFloorToScalar(a);
    }
}
}  // namespace

TextRange operator*(const TextRange& a, const TextRange& b) {
    if (a.start == b.start && a.end == b.end) return a;
    auto begin = std::max(a.start, b.start);
    auto end = std::min(a.end, b.end);
    return end > begin ? TextRange(begin, end) : EMPTY_TEXT;
}

Paragraph::Paragraph(ParagraphStyle style, sk_sp<FontCollection> fonts)
            : fFontCollection(std::move(fonts))
            , fParagraphStyle(std::move(style))
            , fAlphabeticBaseline(0)
            , fIdeographicBaseline(0)
            , fHeight(0)
            , fWidth(0)
            , fMaxIntrinsicWidth(0)
            , fMinIntrinsicWidth(0)
            , fLongestLine(0)
            , fExceededMaxLines(0)
{ }

ParagraphImpl::ParagraphImpl(const SkString& text,
                             ParagraphStyle style,
                             SkTArray<Block, true> blocks,
                             SkTArray<Placeholder, true> placeholders,
                             sk_sp<FontCollection> fonts,
                             std::unique_ptr<SkUnicode> unicode)
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
        , fUnicode(std::move(unicode))
{
    SkASSERT(fUnicode);
}

ParagraphImpl::ParagraphImpl(const std::u16string& utf16text,
                             ParagraphStyle style,
                             SkTArray<Block, true> blocks,
                             SkTArray<Placeholder, true> placeholders,
                             sk_sp<FontCollection> fonts,
                             std::unique_ptr<SkUnicode> unicode)
        : ParagraphImpl(SkString(),
                        std::move(style),
                        std::move(blocks),
                        std::move(placeholders),
                        std::move(fonts),
                        std::move(unicode))
{
    SkASSERT(fUnicode);
    fText =  fUnicode->convertUtf16ToUtf8(utf16text);
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

    if ((!SkScalarIsFinite(rawWidth) || fLongestLine <= floorWidth) &&
        fState >= kLineBroken &&
         fLines.size() == 1 && fLines.front().ellipsis() == nullptr) {
        // Most common case: one line of text (and one line is never justified, so no cluster shifts)
        // We cannot mark it as kLineBroken because the new width can be bigger than the old width
        fWidth = floorWidth;
        fState = kMarked;
    } else if (fState >= kLineBroken && fOldWidth != floorWidth) {
        // We can use the results from SkShaper but have to do EVERYTHING ELSE again
        fState = kShaped;
    } else {
        // Nothing changed case: we can reuse the data from the last layout
    }

    if (fState < kShaped) {
        this->fCodeUnitProperties.reset();
        this->fCodeUnitProperties.push_back_n(fText.size() + 1, CodeUnitFlags::kNoCodeUnitFlag);
        this->fWords.clear();
        this->fBidiRegions.clear();
        this->fUTF8IndexForUTF16Index.reset();
        this->fUTF16IndexForUTF8Index.reset();
        this->fRuns.reset();
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
            fLongestLine = FLT_MIN - FLT_MAX; // That is what flutter has
            fMinIntrinsicWidth = 0;
            fMaxIntrinsicWidth = 0;
            this->fOldWidth = floorWidth;
            this->fOldHeight = this->fHeight;

            return;
        }
        fState = kShaped;
    }

    if (fState < kMarked) {
        this->fClusters.reset();
        this->resetShifts();
        this->fClustersIndexFromCodeUnit.reset();
        this->fClustersIndexFromCodeUnit.push_back_n(fText.size() + 1, EMPTY_INDEX);
        this->buildClusterTable();
        fState = kClusterized;
        this->spaceGlyphs();
        fState = kMarked;
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

    // TODO: Since min and max are calculated differently it's possible to get a rounding error
    //  that would make min > max. Sort it out later, make it the same for now
    if (fMaxIntrinsicWidth < fMinIntrinsicWidth) {
        fMaxIntrinsicWidth = fMinIntrinsicWidth;
    }

    //SkDebugf("layout('%s', %f): %f %f\n", fText.c_str(), rawWidth, fMinIntrinsicWidth, fMaxIntrinsicWidth);
}

void ParagraphImpl::paint(SkCanvas* canvas, SkScalar x, SkScalar y) {

    if (fParagraphStyle.getDrawOptions() == DrawOptions::kDirect) {
        // Paint the text without recording it
        this->paintLines(canvas, x, y);
        return;
    }

    if (fState < kDrawn) {
        // Record the picture anyway (but if we have some pieces in the cache they will be used)
        this->paintLinesIntoPicture(0, 0);
        fState = kDrawn;
    }

    if (fParagraphStyle.getDrawOptions() == DrawOptions::kReplay) {
        // Replay the recorded picture
        canvas->save();
        canvas->translate(x, y);
        fPicture->playback(canvas);
        canvas->restore();
    } else {
        // Draw the picture
        SkMatrix matrix = SkMatrix::Translate(x, y);
        canvas->drawPicture(fPicture, &matrix, nullptr);
    }
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

// shapeTextIntoEndlessLine is the thing that calls this method
bool ParagraphImpl::computeCodeUnitProperties() {

    if (nullptr == fUnicode) {
        return false;
    }

    // Get bidi regions
    auto textDirection = fParagraphStyle.getTextDirection() == TextDirection::kLtr
                              ? SkUnicode::TextDirection::kLTR
                              : SkUnicode::TextDirection::kRTL;
    if (!fUnicode->getBidiRegions(fText.c_str(), fText.size(), textDirection, &fBidiRegions)) {
        return false;
    }

    // Get all spaces
    fUnicode->forEachCodepoint(fText.c_str(), fText.size(),
       [this](SkUnichar unichar, int32_t start, int32_t end) {
            if (fUnicode->isWhitespace(unichar)) {
                for (auto i = start; i < end; ++i) {
                    fCodeUnitProperties[i] |=  CodeUnitFlags::kPartOfWhiteSpaceBreak;
                }
            }
            if (fUnicode->isSpace(unichar)) {
                for (auto i = start; i < end; ++i) {
                    fCodeUnitProperties[i] |=  CodeUnitFlags::kPartOfIntraWordBreak;
                }
            }
       });

    // Get line breaks
    std::vector<SkUnicode::LineBreakBefore> lineBreaks;
    if (!fUnicode->getLineBreaks(fText.c_str(), fText.size(), &lineBreaks)) {
        return false;
    }
    for (auto& lineBreak : lineBreaks) {
        fCodeUnitProperties[lineBreak.pos] |= lineBreak.breakType == SkUnicode::LineBreakType::kHardLineBreak
                                           ? CodeUnitFlags::kHardLineBreakBefore
                                           : CodeUnitFlags::kSoftLineBreakBefore;
    }

    // Get graphemes
    std::vector<SkUnicode::Position> graphemes;
    if (!fUnicode->getGraphemes(fText.c_str(), fText.size(), &graphemes)) {
        return false;
    }
    for (auto pos : graphemes) {
        fCodeUnitProperties[pos] |= CodeUnitFlags::kGraphemeStart;
    }

    return true;
}

static bool is_ascii_7bit_space(int c) {
    SkASSERT(c >= 0 && c <= 127);

    // Extracted from https://en.wikipedia.org/wiki/Whitespace_character
    //
    enum WS {
        kHT    = 9,
        kLF    = 10,
        kVT    = 11,
        kFF    = 12,
        kCR    = 13,
        kSP    = 32,    // too big to use as shift
    };
#define M(shift)    (1 << (shift))
    constexpr uint32_t kSpaceMask = M(kHT) | M(kLF) | M(kVT) | M(kFF) | M(kCR);
    // we check for Space (32) explicitly, since it is too large to shift
    return (c == kSP) || (c <= 31 && (kSpaceMask & M(c)));
#undef M
}

Cluster::Cluster(ParagraphImpl* owner,
                 RunIndex runIndex,
                 size_t start,
                 size_t end,
                 SkSpan<const char> text,
                 SkScalar width,
                 SkScalar height)
        : fOwner(owner)
        , fRunIndex(runIndex)
        , fTextRange(text.begin() - fOwner->text().begin(), text.end() - fOwner->text().begin())
        , fGraphemeRange(EMPTY_RANGE)
        , fStart(start)
        , fEnd(end)
        , fWidth(width)
        , fSpacing(0)
        , fHeight(height)
        , fHalfLetterSpacing(0.0) {
    size_t whiteSpacesBreakLen = 0;
    size_t intraWordBreakLen = 0;

    const char* ch = text.begin();
    if (text.end() - ch == 1 && *(unsigned char*)ch <= 0x7F) {
        // I am not even sure it's worth it if we do not save a unicode call
        if (is_ascii_7bit_space(*ch)) {
            ++whiteSpacesBreakLen;
        }
    } else {
        for (auto i = fTextRange.start; i < fTextRange.end; ++i) {
            if (fOwner->codeUnitHasProperty(i, CodeUnitFlags::kPartOfWhiteSpaceBreak)) {
                ++whiteSpacesBreakLen;
            }
            if (fOwner->codeUnitHasProperty(i, CodeUnitFlags::kPartOfIntraWordBreak)) {
                ++intraWordBreakLen;
            }
        }
    }

    fIsWhiteSpaceBreak = whiteSpacesBreakLen == fTextRange.width();
    fIsIntraWordBreak = intraWordBreakLen == fTextRange.width();
    fIsHardBreak = fOwner->codeUnitHasProperty(fTextRange.end, CodeUnitFlags::kHardLineBreakBefore);
}

SkScalar Run::calculateWidth(size_t start, size_t end, bool clip) const {
    SkASSERT(start <= end);
    // clip |= end == size();  // Clip at the end of the run?
    SkScalar shift = 0;
    if (fSpaced && end > start) {
        shift = fShifts[clip ? end - 1 : end] - fShifts[start];
    }
    auto correction = 0.0f;
    if (end > start && !fJustificationShifts.empty()) {
        // This is not a typo: we are using Point as a pair of SkScalars
        correction = fJustificationShifts[end - 1].fX -
                     fJustificationShifts[start].fY;
    }
    return posX(end) - posX(start) + shift + correction;
}

// Clusters in the order of the input text
void ParagraphImpl::buildClusterTable() {
    int cluster_count = 1;
    for (auto& run : fRuns) {
        cluster_count += run.isPlaceholder() ? 1 : run.size();
    }
    fClusters.reserve_back(cluster_count);

    // Walk through all the run in the direction of input text
    for (auto& run : fRuns) {
        auto runIndex = run.index();
        auto runStart = fClusters.size();
        if (run.isPlaceholder()) {
            // Add info to cluster indexes table (text -> cluster)
            for (auto i = run.textRange().start; i < run.textRange().end; ++i) {
              fClustersIndexFromCodeUnit[i] = fClusters.size();
            }
            // There are no glyphs but we want to have one cluster
            fClusters.emplace_back(this, runIndex, 0ul, 1ul, this->text(run.textRange()), run.advance().fX, run.advance().fY);
            fCodeUnitProperties[run.textRange().start] |= CodeUnitFlags::kSoftLineBreakBefore;
            fCodeUnitProperties[run.textRange().end] |= CodeUnitFlags::kSoftLineBreakBefore;
        } else {
            // Walk through the glyph in the direction of input text
            run.iterateThroughClustersInTextOrder([runIndex, this](size_t glyphStart,
                                                                   size_t glyphEnd,
                                                                   size_t charStart,
                                                                   size_t charEnd,
                                                                   SkScalar width,
                                                                   SkScalar height) {
                SkASSERT(charEnd >= charStart);
                // Add info to cluster indexes table (text -> cluster)
                for (auto i = charStart; i < charEnd; ++i) {
                  fClustersIndexFromCodeUnit[i] = fClusters.size();
                }
                SkSpan<const char> text(fText.c_str() + charStart, charEnd - charStart);
                fClusters.emplace_back(this, runIndex, glyphStart, glyphEnd, text, width, height);
            });
        }

        run.setClusterRange(runStart, fClusters.size());
        fMaxIntrinsicWidth += run.advance().fX;
    }
    fClustersIndexFromCodeUnit[fText.size()] = fClusters.size();
    fClusters.emplace_back(this, EMPTY_RUN, 0, 0, this->text({fText.size(), fText.size()}), 0, 0);
}

void ParagraphImpl::spaceGlyphs() {

    // Walk through all the clusters in the direction of shaped text
    // (we have to walk through the styles in the same order, too)
    SkScalar shift = 0;
    for (auto& run : fRuns) {

        // Skip placeholder runs
        if (run.isPlaceholder()) {
            continue;
        }

        bool soFarWhitespacesOnly = true;
        run.iterateThroughClusters([this, &run, &shift, &soFarWhitespacesOnly](Cluster* cluster) {
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
                if (cluster->isWhitespaceBreak() && cluster->isSoftBreak()) {
                    if (!soFarWhitespacesOnly) {
                        shift += run.addSpacesAtTheEnd(currentStyle->fStyle.getWordSpacing(), cluster);
                    }
                }
            }
            // Process letter spacing
            if (currentStyle->fStyle.getLetterSpacing() != 0) {
                shift += run.addSpacesEvenly(currentStyle->fStyle.getLetterSpacing(), cluster);
            }

            if (soFarWhitespacesOnly && !cluster->isWhitespaceBreak()) {
                soFarWhitespacesOnly = false;
            }
        });
    }
}

bool ParagraphImpl::shapeTextIntoEndlessLine() {

    if (fText.size() == 0) {
        return false;
    }

    // Check the font-resolved text against the cache
    if (fFontCollection->getParagraphCache()->findParagraph(this)) {
        return true;
    }

    if (!computeCodeUnitProperties()) {
        return false;
    }

    fFontSwitches.reset();

    OneLineShaper oneLineShaper(this);
    auto result = oneLineShaper.shape();
    fUnresolvedGlyphs = oneLineShaper.unresolvedGlyphs();

    // It's possible that one grapheme includes few runs; we cannot handle it
    // so we break graphemes by the runs instead
    // It's not the ideal solution and has to be revisited later
    for (auto& run : fRuns) {
        fCodeUnitProperties[run.fTextRange.start] |= CodeUnitFlags::kGraphemeStart;
    }

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
            [&](TextRange textExcludingSpaces,
                TextRange text,
                TextRange textWithNewlines,
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
                auto& line = this->addLine(offset, advance, textExcludingSpaces, text, textWithNewlines, clusters, clustersWithGhosts, widthWithSpaces, metrics);
                if (addEllipsis) {
                    line.createEllipsis(maxWidth, getEllipsis(), true);
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
        line.format(effectiveAlign, maxWidth);
    }
}

void ParagraphImpl::paintLinesIntoPicture(SkScalar x, SkScalar y) {
    SkPictureRecorder recorder;
    SkCanvas* textCanvas = recorder.beginRecording(this->getMaxWidth(), this->getHeight());

    auto bounds = SkRect::MakeEmpty();
    for (auto& line : fLines) {
        auto boundaries = line.paint(textCanvas, x, y);
        bounds.joinPossiblyEmptyRect(boundaries);
    }

    fPicture = recorder.finishRecordingAsPictureWithCull(bounds);
}

void ParagraphImpl::paintLines(SkCanvas* canvas, SkScalar x, SkScalar y) {
    for (auto& line : fLines) {
        line.paint(canvas, x, y);
    }
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

    if (begin == EMPTY_INDEX || end == EMPTY_INDEX) {
        // It's possible if some text is not covered with any text style
        // Not in Flutter but in direct use of SkParagraph
        return EMPTY_RANGE;
    }

    return { begin, end + 1 };
}

TextLine& ParagraphImpl::addLine(SkVector offset,
                                 SkVector advance,
                                 TextRange textExcludingSpaces,
                                 TextRange text,
                                 TextRange textIncludingNewLines,
                                 ClusterRange clusters,
                                 ClusterRange clustersWithGhosts,
                                 SkScalar widthWithSpaces,
                                 InternalLineMetrics sizes) {
    // Define a list of styles that covers the line
    auto blocks = findAllBlocks(textExcludingSpaces);
    return fLines.emplace_back(this, offset, advance, blocks,
                               textExcludingSpaces, text, textIncludingNewLines,
                               clusters, clustersWithGhosts, widthWithSpaces, sizes);
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

    ensureUTF16Mapping();

    if (start >= end || start > fUTF8IndexForUTF16Index.size() || end == 0) {
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
    // TODO: This is probably a temp change that makes SkParagraph work as TxtLib
    //  (so we can compare the results). We now include in the selection box only the graphemes
    //  that belongs to the given [start:end) range entirely (not the ones that intersect with it)
    if (start < fUTF8IndexForUTF16Index.size()) {
        auto utf8 = fUTF8IndexForUTF16Index[start];
        // If start points to a trailing surrogate, skip it
        if (start > 0 && fUTF8IndexForUTF16Index[start - 1] == utf8) {
            utf8 = fUTF8IndexForUTF16Index[start + 1];
        }
        text.start = findNextGraphemeBoundary(utf8);
    }
    if (end < fUTF8IndexForUTF16Index.size()) {
        auto utf8 = findPreviousGraphemeBoundary(fUTF8IndexForUTF16Index[end]);
        text.end = utf8;
    }
    //SkDebugf("getRectsForRange(%d,%d) -> (%d:%d)\n", start, end, text.start, text.end);
    for (auto& line : fLines) {
        auto lineText = line.textWithNewlines();
        auto intersect = lineText * text;
        if (intersect.empty() && lineText.start != text.start) {
            continue;
        }

        line.getRectsForRange(intersect, rectHeightStyle, rectWidthStyle, results);
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
  if (fPlaceholders.size() == 1) {
       // We always have one fake placeholder
       return boxes;
  }
  for (auto& line : fLines) {
      line.getRectsForPlaceholders(boxes);
  }
  /*
  SkDebugf("getRectsForPlaceholders('%s'): %d\n", fText.c_str(), boxes.size());
  for (auto& r : boxes) {
      r.rect.fLeft = littleRound(r.rect.fLeft);
      r.rect.fRight = littleRound(r.rect.fRight);
      r.rect.fTop = littleRound(r.rect.fTop);
      r.rect.fBottom = littleRound(r.rect.fBottom);
      SkDebugf("[%f:%f * %f:%f] %s\n", r.rect.fLeft, r.rect.fRight, r.rect.fTop, r.rect.fBottom,
               (r.direction == TextDirection::kLtr ? "left" : "right"));
  }
  */
  return boxes;
}

// TODO: Optimize (save cluster <-> codepoint connection)
PositionWithAffinity ParagraphImpl::getGlyphPositionAtCoordinate(SkScalar dx, SkScalar dy) {

    if (fText.isEmpty()) {
        return {0, Affinity::kDownstream};
    }

    ensureUTF16Mapping();

    for (auto& line : fLines) {
        // Let's figure out if we can stop looking
        auto offsetY = line.offset().fY;
        if (dy >= offsetY + line.height() && &line != &fLines.back()) {
            // This line is not good enough
            continue;
        }

        // This is so far the the line vertically closest to our coordinates
        // (or the first one, or the only one - all the same)

        auto result = line.getGlyphPositionAtCoordinate(dx);
        //SkDebugf("getGlyphPositionAtCoordinate(%f, %f): %d %s\n", dx, dy, result.position,
        //   result.affinity == Affinity::kUpstream ? "up" : "down");
        return result;
    }

    return {0, Affinity::kDownstream};
}

// Finds the first and last glyphs that define a word containing
// the glyph at index offset.
// By "glyph" they mean a character index - indicated by Minikin's code
SkRange<size_t> ParagraphImpl::getWordBoundary(unsigned offset) {

    if (fWords.empty()) {
        if (!fUnicode->getWords(fText.c_str(), fText.size(), &fWords)) {
            return {0, 0 };
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
            fCodeUnitProperties.reset();
            fCodeUnitProperties.push_back_n(fText.size() + 1, kNoCodeUnitFlag);
            fWords.clear();
            fBidiRegions.clear();
            fUTF8IndexForUTF16Index.reset();
            fUTF16IndexForUTF8Index.reset();
            [[fallthrough]];

        case kShaped:
            fClusters.reset();
            [[fallthrough]];

        case kClusterized:
        case kMarked:
        case kLineBroken:
            this->resetContext();
            this->resolveStrut();
            this->computeEmptyMetrics();
            this->resetShifts();
            fLines.reset();
            [[fallthrough]];

        case kFormatted:
            fPicture = nullptr;
            [[fallthrough]];

        case kDrawn:
        default:
            break;
    }
}

void ParagraphImpl::computeEmptyMetrics() {

    // The empty metrics is used to define the height of the empty lines
    // Unfortunately, Flutter has 2 different cases for that:
    // 1. An empty line inside the text
    // 2. An empty paragraph
    // In the first case SkParagraph takes the metrics from the default paragraph style
    // In the second case it should take it from the current text style
    bool emptyParagraph = fRuns.empty();
    TextStyle textStyle = paragraphStyle().getTextStyle();
    if (emptyParagraph && !fTextStyles.empty()) {
        textStyle = fTextStyles.back().fStyle;
    }

    auto typefaces = fontCollection()->findTypefaces(
      textStyle.getFontFamilies(), textStyle.getFontStyle());
    auto typeface = typefaces.empty() ? nullptr : typefaces.front();

    SkFont font(typeface, textStyle.getFontSize());
    fEmptyMetrics = InternalLineMetrics(font, paragraphStyle().getStrutStyle().getForceStrutHeight());

    if (!paragraphStyle().getStrutStyle().getForceStrutHeight() &&
        textStyle.getHeightOverride()) {
        const auto intrinsicHeight = fEmptyMetrics.height();
        const auto strutHeight = textStyle.getHeight() * textStyle.getFontSize();
        if (paragraphStyle().getStrutStyle().getHalfLeading()) {
            fEmptyMetrics.update(
                fEmptyMetrics.ascent(),
                fEmptyMetrics.descent(),
                fEmptyMetrics.leading() + strutHeight - intrinsicHeight);
        } else {
            const auto multiplier = strutHeight / intrinsicHeight;
            fEmptyMetrics.update(
                fEmptyMetrics.ascent() * multiplier,
                fEmptyMetrics.descent() * multiplier,
                fEmptyMetrics.leading() * multiplier);
        }
    }

    if (emptyParagraph) {
        // For an empty text we apply both TextHeightBehaviour flags
        // In case of non-empty paragraph TextHeightBehaviour flags will be applied at the appropriate place
        // We have to do it here because we skip wrapping for an empty text
        auto disableFirstAscent = (paragraphStyle().getTextHeightBehavior() & TextHeightBehavior::kDisableFirstAscent) == TextHeightBehavior::kDisableFirstAscent;
        auto disableLastDescent = (paragraphStyle().getTextHeightBehavior() & TextHeightBehavior::kDisableLastDescent) == TextHeightBehavior::kDisableLastDescent;
        fEmptyMetrics.update(
            disableFirstAscent ? fEmptyMetrics.rawAscent() : fEmptyMetrics.ascent(),
            disableLastDescent ? fEmptyMetrics.rawDescent() : fEmptyMetrics.descent(),
            fEmptyMetrics.leading());
    }

    if (fParagraphStyle.getStrutStyle().getStrutEnabled()) {
        fStrutMetrics.updateLineMetrics(fEmptyMetrics);
    }
}

SkString ParagraphImpl::getEllipsis() const {

    auto ellipsis8 = fParagraphStyle.getEllipsis();
    auto ellipsis16 = fParagraphStyle.getEllipsisUtf16();
    if (!ellipsis8.isEmpty()) {
        return ellipsis8;
    } else {
        return fUnicode->convertUtf16ToUtf8(fParagraphStyle.getEllipsisUtf16());
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

TextIndex ParagraphImpl::findPreviousGraphemeBoundary(TextIndex utf8) {
    while (utf8 > 0 &&
          (fCodeUnitProperties[utf8] & CodeUnitFlags::kGraphemeStart) == 0) {
        --utf8;
    }
    return utf8;
}

TextIndex ParagraphImpl::findNextGraphemeBoundary(TextIndex utf8) {
    while (utf8 < fText.size() &&
          (fCodeUnitProperties[utf8] & CodeUnitFlags::kGraphemeStart) == 0) {
        ++utf8;
    }
    return utf8;
}

void ParagraphImpl::ensureUTF16Mapping() {
    if (!fUTF16IndexForUTF8Index.empty()) {
        return;
    }
    // Fill out code points 16
    auto ptr = fText.c_str();
    auto end = fText.c_str() + fText.size();
    while (ptr < end) {

        size_t index = ptr - fText.c_str();
        SkUnichar u = SkUTF::NextUTF8(&ptr, end);

        // All utf8 units refer to the same codepoint
        size_t next = ptr - fText.c_str();
        for (auto i = index; i < next; ++i) {
            fUTF16IndexForUTF8Index.emplace_back(fUTF8IndexForUTF16Index.size());
        }
        SkASSERT(fUTF16IndexForUTF8Index.size() == next);

        // One or two codepoints refer to the same text index
        uint16_t buffer[2];
        size_t count = SkUTF::ToUTF16(u, buffer);
        fUTF8IndexForUTF16Index.emplace_back(index);
        if (count > 1) {
            fUTF8IndexForUTF16Index.emplace_back(index);
        }
    }
    fUTF16IndexForUTF8Index.emplace_back(fUTF8IndexForUTF16Index.size());
    fUTF8IndexForUTF16Index.emplace_back(fText.size());
}

void ParagraphImpl::visit(const Visitor& visitor) {
    int lineNumber = 0;
    for (auto& line : fLines) {
        line.ensureTextBlobCachePopulated();
        for (auto& rec : line.fTextBlobCache) {
            SkTextBlob::Iter iter(*rec.fBlob);
            SkTextBlob::Iter::ExperimentalRun run;

            SkSTArray<128, uint32_t> clusterStorage;
            const Run* R = rec.fVisitor_Run;
            const uint32_t* clusterPtr = &R->fClusterIndexes[0];

            if (R->fClusterStart > 0) {
                int count = R->fClusterIndexes.count();
                clusterStorage.reset(count);
                for (int i = 0; i < count; ++i) {
                    clusterStorage[i] = R->fClusterStart + R->fClusterIndexes[i];
                }
                clusterPtr = &clusterStorage[0];
            }
            clusterPtr += rec.fVisitor_Pos;

            while (iter.experimentalNext(&run)) {
                const Paragraph::VisitorInfo info = {
                    run.font,
                    rec.fOffset,
                    rec.fClipRect.fRight,
                    run.count,
                    run.glyphs,
                    run.positions,
                    clusterPtr,
                    0,  // flags
                };
                visitor(lineNumber, &info);
                clusterPtr += run.count;
            }
        }
        visitor(lineNumber, nullptr);   // signal end of line
        lineNumber += 1;
    }
}

}  // namespace textlayout
}  // namespace skia
