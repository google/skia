// Copyright 2019 Google LLC.
#include "include/core/SkCanvas.h"
#include "include/core/SkFontMetrics.h"
#include "include/core/SkMatrix.h"
#include "include/core/SkPath.h"
#include "include/core/SkPictureRecorder.h"
#include "include/core/SkSpan.h"
#include "include/core/SkTypeface.h"
#include "include/private/base/SkTFitsIn.h"
#include "include/private/base/SkTo.h"
#include "modules/skparagraph/include/Metrics.h"
#include "modules/skparagraph/include/Paragraph.h"
#include "modules/skparagraph/include/ParagraphPainter.h"
#include "modules/skparagraph/include/ParagraphStyle.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/OneLineShaper.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/ParagraphPainterImpl.h"
#include "modules/skparagraph/src/Run.h"
#include "modules/skparagraph/src/TextLine.h"
#include "modules/skparagraph/src/TextWrapper.h"
#include "src/base/SkUTF.h"
#include "src/core/SkTextBlobPriv.h"

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <utility>

using namespace skia_private;

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
{
    SkASSERT(fFontCollection);
}

ParagraphImpl::ParagraphImpl(const SkString& text,
                             ParagraphStyle style,
                             TArray<Block, true> blocks,
                             TArray<Placeholder, true> placeholders,
                             sk_sp<FontCollection> fonts,
                             std::shared_ptr<SkUnicode> unicode)
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
        , fHasLineBreaks(false)
        , fHasWhitespacesInside(false)
        , fTrailingSpaces(0)
{
    SkASSERT(fUnicode);
}

ParagraphImpl::ParagraphImpl(const std::u16string& utf16text,
                             ParagraphStyle style,
                             TArray<Block, true> blocks,
                             TArray<Placeholder, true> placeholders,
                             sk_sp<FontCollection> fonts,
                             std::shared_ptr<SkUnicode> unicode)
        : ParagraphImpl(SkString(),
                        std::move(style),
                        std::move(blocks),
                        std::move(placeholders),
                        std::move(fonts),
                        std::move(unicode))
{
    SkASSERT(fUnicode);
    fText =  SkUnicode::convertUtf16ToUtf8(utf16text);
}

ParagraphImpl::~ParagraphImpl() = default;

int32_t ParagraphImpl::unresolvedGlyphs() {
    if (fState < kShaped) {
        return -1;
    }

    return fUnresolvedGlyphs;
}

std::unordered_set<SkUnichar> ParagraphImpl::unresolvedCodepoints() {
    return fUnresolvedCodepoints;
}

void ParagraphImpl::addUnresolvedCodepoints(TextRange textRange) {
    fUnicode->forEachCodepoint(
        &fText[textRange.start], textRange.width(),
        [&](SkUnichar unichar, int32_t start, int32_t end, int32_t count) {
            fUnresolvedCodepoints.emplace(unichar);
        }
    );
}

void ParagraphImpl::layout(SkScalar rawWidth) {
    // TODO: This rounding is done to match Flutter tests. Must be removed...
    auto floorWidth = rawWidth;
    if (getApplyRoundingHack()) {
        floorWidth = SkScalarFloorToScalar(floorWidth);
    }

    if ((!SkScalarIsFinite(rawWidth) || fLongestLine <= floorWidth) &&
        fState >= kLineBroken &&
         fLines.size() == 1 && fLines.front().ellipsis() == nullptr) {
        // Most common case: one line of text (and one line is never justified, so no cluster shifts)
        // We cannot mark it as kLineBroken because the new width can be bigger than the old width
        fWidth = floorWidth;
        fState = kShaped;
    } else if (fState >= kLineBroken && fOldWidth != floorWidth) {
        // We can use the results from SkShaper but have to do EVERYTHING ELSE again
        fState = kShaped;
    } else {
        // Nothing changed case: we can reuse the data from the last layout
    }

    if (fState < kShaped) {
        // Check if we have the text in the cache and don't need to shape it again
        if (!fFontCollection->getParagraphCache()->findParagraph(this)) {
            if (fState < kIndexed) {
                // This only happens once at the first layout; the text is immutable
                // and there is no reason to repeat it
                if (this->computeCodeUnitProperties()) {
                    fState = kIndexed;
                }
            }
            this->fRuns.clear();
            this->fClusters.clear();
            this->fClustersIndexFromCodeUnit.clear();
            this->fClustersIndexFromCodeUnit.push_back_n(fText.size() + 1, EMPTY_INDEX);
            if (!this->shapeTextIntoEndlessLine()) {
                this->resetContext();
                // TODO: merge the two next calls - they always come together
                this->resolveStrut();
                this->computeEmptyMetrics();
                this->fLines.clear();

                // Set the important values that are not zero
                fWidth = floorWidth;
                fHeight = fEmptyMetrics.height();
                if (fParagraphStyle.getStrutStyle().getStrutEnabled() &&
                    fParagraphStyle.getStrutStyle().getForceStrutHeight()) {
                    fHeight = fStrutMetrics.height();
                }
                fAlphabeticBaseline = fEmptyMetrics.alphabeticBaseline();
                fIdeographicBaseline = fEmptyMetrics.ideographicBaseline();
                fLongestLine = FLT_MIN - FLT_MAX;  // That is what flutter has
                fMinIntrinsicWidth = 0;
                fMaxIntrinsicWidth = 0;
                this->fOldWidth = floorWidth;
                this->fOldHeight = this->fHeight;

                return;
            } else {
                // Add the paragraph to the cache
                fFontCollection->getParagraphCache()->updateParagraph(this);
            }
        }
        fState = kShaped;
    }

    if (fState == kShaped) {
        this->resetContext();
        this->resolveStrut();
        this->computeEmptyMetrics();
        this->fLines.clear();
        this->breakShapedTextIntoLines(floorWidth);
        fState = kLineBroken;
    }

    if (fState == kLineBroken) {
        // Build the picture lazily not until we actually have to paint (or never)
        this->resetShifts();
        this->formatLines(fWidth);
        fState = kFormatted;
    }

    this->fOldWidth = floorWidth;
    this->fOldHeight = this->fHeight;

    if (getApplyRoundingHack()) {
        // TODO: This rounding is done to match Flutter tests. Must be removed...
        fMinIntrinsicWidth = littleRound(fMinIntrinsicWidth);
        fMaxIntrinsicWidth = littleRound(fMaxIntrinsicWidth);
    }

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
    CanvasParagraphPainter painter(canvas);
    paint(&painter, x, y);
}

void ParagraphImpl::paint(ParagraphPainter* painter, SkScalar x, SkScalar y) {
    for (auto& line : fLines) {
        line.paint(painter, x, y);
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

    // Collect all spaces and some extra information
    // (and also substitute \t with a space while we are at it)
    if (!fUnicode->computeCodeUnitFlags(&fText[0],
                                        fText.size(),
                                        this->paragraphStyle().getReplaceTabCharacters(),
                                        &fCodeUnitProperties)) {
        return false;
    }

    // Get some information about trailing spaces / hard line breaks
    fTrailingSpaces = fText.size();
    TextIndex firstWhitespace = EMPTY_INDEX;
    for (int i = 0; i < fCodeUnitProperties.size(); ++i) {
        auto flags = fCodeUnitProperties[i];
        if (SkUnicode::hasPartOfWhiteSpaceBreakFlag(flags)) {
            if (fTrailingSpaces  == fText.size()) {
                fTrailingSpaces = i;
            }
            if (firstWhitespace == EMPTY_INDEX) {
                firstWhitespace = i;
            }
        } else {
            fTrailingSpaces = fText.size();
        }
        if (SkUnicode::hasHardLineBreakFlag(flags)) {
            fHasLineBreaks = true;
        }
    }

    if (firstWhitespace < fTrailingSpaces) {
        fHasWhitespacesInside = true;
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
        , fHeight(height)
        , fHalfLetterSpacing(0.0)
        , fIsIdeographic(false) {
    size_t whiteSpacesBreakLen = 0;
    size_t intraWordBreakLen = 0;

    const char* ch = text.begin();
    if (text.end() - ch == 1 && *(const unsigned char*)ch <= 0x7F) {
        // I am not even sure it's worth it if we do not save a unicode call
        if (is_ascii_7bit_space(*ch)) {
            ++whiteSpacesBreakLen;
        }
    } else {
        for (auto i = fTextRange.start; i < fTextRange.end; ++i) {
            if (fOwner->codeUnitHasProperty(i, SkUnicode::CodeUnitFlags::kPartOfWhiteSpaceBreak)) {
                ++whiteSpacesBreakLen;
            }
            if (fOwner->codeUnitHasProperty(i, SkUnicode::CodeUnitFlags::kPartOfIntraWordBreak)) {
                ++intraWordBreakLen;
            }
            if (fOwner->codeUnitHasProperty(i, SkUnicode::CodeUnitFlags::kIdeographic)) {
                fIsIdeographic = true;
            }
        }
    }

    fIsWhiteSpaceBreak = whiteSpacesBreakLen == fTextRange.width();
    fIsIntraWordBreak = intraWordBreakLen == fTextRange.width();
    fIsHardBreak = fOwner->codeUnitHasProperty(fTextRange.end,
                                               SkUnicode::CodeUnitFlags::kHardLineBreakBefore);
}

SkScalar Run::calculateWidth(size_t start, size_t end, bool clip) const {
    SkASSERT(start <= end);
    // clip |= end == size();  // Clip at the end of the run?
    auto correction = 0.0f;
    if (end > start && !fJustificationShifts.empty()) {
        // This is not a typo: we are using Point as a pair of SkScalars
        correction = fJustificationShifts[end - 1].fX -
                     fJustificationShifts[start].fY;
    }
    return posX(end) - posX(start) + correction;
}

// In some cases we apply spacing to glyphs first and then build the cluster table, in some we do
// the opposite - just to optimize the most common case.
void ParagraphImpl::applySpacingAndBuildClusterTable() {

    // Check all text styles to see what we have to do (if anything)
    size_t letterSpacingStyles = 0;
    bool hasWordSpacing = false;
    for (auto& block : fTextStyles) {
        if (block.fRange.width() > 0) {
            if (!SkScalarNearlyZero(block.fStyle.getLetterSpacing())) {
                ++letterSpacingStyles;
            }
            if (!SkScalarNearlyZero(block.fStyle.getWordSpacing())) {
                hasWordSpacing = true;
            }
        }
    }

    if (letterSpacingStyles == 0 && !hasWordSpacing) {
        // We don't have to do anything about spacing (most common case)
        this->buildClusterTable();
        return;
    }

    if (letterSpacingStyles == 1 && !hasWordSpacing && fTextStyles.size() == 1 &&
        fTextStyles[0].fRange.width() == fText.size() && fRuns.size() == 1) {
        // We have to letter space the entire paragraph (second most common case)
        auto& run = fRuns[0];
        auto& style = fTextStyles[0].fStyle;
        run.addSpacesEvenly(style.getLetterSpacing());
        this->buildClusterTable();
        // This is something Flutter requires
        for (auto& cluster : fClusters) {
            cluster.setHalfLetterSpacing(style.getLetterSpacing()/2);
        }
        return;
    }

    // The complex case: many text styles with spacing (possibly not adjusted to glyphs)
    this->buildClusterTable();

    // Walk through all the clusters in the direction of shaped text
    // (we have to walk through the styles in the same order, too)
    SkScalar shift = 0;
    for (auto& run : fRuns) {

        // Skip placeholder runs
        if (run.isPlaceholder()) {
            continue;
        }
        bool soFarWhitespacesOnly = true;
        bool wordSpacingPending = false;
        Cluster* lastSpaceCluster = nullptr;
        run.iterateThroughClusters([this, &run, &shift, &soFarWhitespacesOnly, &wordSpacingPending, &lastSpaceCluster](Cluster* cluster) {
            // Shift the cluster (shift collected from the previous clusters)
            run.shift(cluster, shift);

            // Synchronize styles (one cluster can be covered by few styles)
            Block* currentStyle = fTextStyles.begin();
            while (!cluster->startsIn(currentStyle->fRange)) {
                currentStyle++;
                SkASSERT(currentStyle != fTextStyles.end());
            }

            SkASSERT(!currentStyle->fStyle.isPlaceholder());

            // Process word spacing
            if (currentStyle->fStyle.getWordSpacing() != 0) {
                if (cluster->isWhitespaceBreak() && cluster->isSoftBreak()) {
                    if (!soFarWhitespacesOnly) {
                        lastSpaceCluster = cluster;
                        wordSpacingPending = true;
                    }
                } else if (wordSpacingPending) {
                    SkScalar spacing = currentStyle->fStyle.getWordSpacing();
                    run.addSpacesAtTheEnd(spacing, lastSpaceCluster);
                    run.shift(cluster, spacing);
                    shift += spacing;
                    wordSpacingPending = false;
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

// Clusters in the order of the input text
void ParagraphImpl::buildClusterTable() {
    // It's possible that one grapheme includes few runs; we cannot handle it
    // so we break graphemes by the runs instead
    // It's not the ideal solution and has to be revisited later
    int cluster_count = 1;
    for (auto& run : fRuns) {
        cluster_count += run.isPlaceholder() ? 1 : run.size();
        fCodeUnitProperties[run.fTextRange.start] |= SkUnicode::CodeUnitFlags::kGraphemeStart;
        fCodeUnitProperties[run.fTextRange.start] |= SkUnicode::CodeUnitFlags::kGlyphClusterStart;
    }
    if (!fRuns.empty()) {
        fCodeUnitProperties[fRuns.back().textRange().end] |= SkUnicode::CodeUnitFlags::kGraphemeStart;
        fCodeUnitProperties[fRuns.back().textRange().end] |= SkUnicode::CodeUnitFlags::kGlyphClusterStart;
    }
    fClusters.reserve_exact(fClusters.size() + cluster_count);

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
            fCodeUnitProperties[run.textRange().start] |= SkUnicode::CodeUnitFlags::kSoftLineBreakBefore;
            fCodeUnitProperties[run.textRange().end] |= SkUnicode::CodeUnitFlags::kSoftLineBreakBefore;
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
                fCodeUnitProperties[charStart] |= SkUnicode::CodeUnitFlags::kGlyphClusterStart;
            });
        }
        fCodeUnitProperties[run.textRange().start] |= SkUnicode::CodeUnitFlags::kGlyphClusterStart;

        run.setClusterRange(runStart, fClusters.size());
        fMaxIntrinsicWidth += run.advance().fX;
    }
    fClustersIndexFromCodeUnit[fText.size()] = fClusters.size();
    fClusters.emplace_back(this, EMPTY_RUN, 0, 0, this->text({fText.size(), fText.size()}), 0, 0);
}

bool ParagraphImpl::shapeTextIntoEndlessLine() {

    if (fText.size() == 0) {
        return false;
    }

    fUnresolvedCodepoints.clear();
    fFontSwitches.clear();

    OneLineShaper oneLineShaper(this);
    auto result = oneLineShaper.shape();
    fUnresolvedGlyphs = oneLineShaper.unresolvedGlyphs();

    this->applySpacingAndBuildClusterTable();

    return result;
}

void ParagraphImpl::breakShapedTextIntoLines(SkScalar maxWidth) {

    if (!fHasLineBreaks &&
        !fHasWhitespacesInside &&
        fPlaceholders.size() == 1 &&
        fRuns.size() == 1 && fRuns[0].fAdvance.fX <= maxWidth) {
        // This is a short version of a line breaking when we know that:
        // 1. We have only one line of text
        // 2. It's shaped into a single run
        // 3. There are no placeholders
        // 4. There are no linebreaks (which will format text into multiple lines)
        // 5. There are no whitespaces so the minIntrinsicWidth=maxIntrinsicWidth
        // (To think about that, the last condition is not quite right;
        // we should calculate minIntrinsicWidth by soft line breaks.
        // However, it's how it's done in Flutter now)
        auto& run = this->fRuns[0];
        auto advance = run.advance();
        auto textRange = TextRange(0, this->text().size());
        auto textExcludingSpaces = TextRange(0, fTrailingSpaces);
        InternalLineMetrics metrics(this->strutForceHeight());
        metrics.add(&run);
        auto disableFirstAscent = this->paragraphStyle().getTextHeightBehavior() &
                                  TextHeightBehavior::kDisableFirstAscent;
        auto disableLastDescent = this->paragraphStyle().getTextHeightBehavior() &
                                  TextHeightBehavior::kDisableLastDescent;
        if (disableFirstAscent) {
            metrics.fAscent = metrics.fRawAscent;
        }
        if (disableLastDescent) {
            metrics.fDescent = metrics.fRawDescent;
        }
        if (this->strutEnabled()) {
            this->strutMetrics().updateLineMetrics(metrics);
        }
        ClusterIndex trailingSpaces = fClusters.size();
        do {
            --trailingSpaces;
            auto& cluster = fClusters[trailingSpaces];
            if (!cluster.isWhitespaceBreak()) {
                ++trailingSpaces;
                break;
            }
            advance.fX -= cluster.width();
        } while (trailingSpaces != 0);

        advance.fY = metrics.height();
        auto clusterRange = ClusterRange(0, trailingSpaces);
        auto clusterRangeWithGhosts = ClusterRange(0, this->clusters().size() - 1);
        this->addLine(SkPoint::Make(0, 0), advance,
                      textExcludingSpaces, textRange, textRange,
                      clusterRange, clusterRangeWithGhosts, run.advance().x(),
                      metrics);

        fLongestLine = nearlyZero(advance.fX) ? run.advance().fX : advance.fX;
        fHeight = advance.fY;
        fWidth = maxWidth;
        fMaxIntrinsicWidth = run.advance().fX;
        fMinIntrinsicWidth = advance.fX;
        fAlphabeticBaseline = fLines.empty() ? fEmptyMetrics.alphabeticBaseline() : fLines.front().alphabeticBaseline();
        fIdeographicBaseline = fLines.empty() ? fEmptyMetrics.ideographicBaseline() : fLines.front().ideographicBaseline();
        fExceededMaxLines = false;
        return;
    }

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
                    line.createEllipsis(maxWidth, this->getEllipsis(), true);
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
    const bool isLeftAligned = effectiveAlign == TextAlign::kLeft
        || (effectiveAlign == TextAlign::kJustify && fParagraphStyle.getTextDirection() == TextDirection::kLtr);

    if (!SkScalarIsFinite(maxWidth) && !isLeftAligned) {
        // Special case: clean all text in case of maxWidth == INF & align != left
        // We had to go through shaping though because we need all the measurement numbers
        fLines.clear();
        return;
    }

    for (auto& line : fLines) {
        line.format(effectiveAlign, maxWidth);
    }
}

void ParagraphImpl::resolveStrut() {
    auto strutStyle = this->paragraphStyle().getStrutStyle();
    if (!strutStyle.getStrutEnabled() || strutStyle.getFontSize() < 0) {
        return;
    }

    std::vector<sk_sp<SkTypeface>> typefaces = fFontCollection->findTypefaces(strutStyle.getFontFamilies(), strutStyle.getFontStyle(), std::nullopt);
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
                strutStyle.getLeading() < 0 ? 0 : strutStyle.getLeading() * strutStyle.getFontSize(),
            metrics.fAscent, metrics.fDescent, metrics.fLeading);
    } else {
        fStrutMetrics = InternalLineMetrics(
                metrics.fAscent,
                metrics.fDescent,
                strutStyle.getLeading() < 0 ? 0 : strutStyle.getLeading() * strutStyle.getFontSize());
    }
    fStrutMetrics.setForceStrut(this->paragraphStyle().getStrutStyle().getForceStrutHeight());
}

BlockRange ParagraphImpl::findAllBlocks(TextRange textRange) {
    BlockIndex begin = EMPTY_BLOCK;
    BlockIndex end = EMPTY_BLOCK;
    for (int index = 0; index < fTextStyles.size(); ++index) {
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

    this->ensureUTF16Mapping();

    if (start >= end || start > SkToSizeT(fUTF8IndexForUTF16Index.size()) || end == 0) {
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
    if (start < SkToSizeT(fUTF8IndexForUTF16Index.size())) {
        auto utf8 = fUTF8IndexForUTF16Index[start];
        // If start points to a trailing surrogate, skip it
        if (start > 0 && fUTF8IndexForUTF16Index[start - 1] == utf8) {
            utf8 = fUTF8IndexForUTF16Index[start + 1];
        }
        text.start = this->findNextGraphemeBoundary(utf8);
    }
    if (end < SkToSizeT(fUTF8IndexForUTF16Index.size())) {
        auto utf8 = this->findPreviousGraphemeBoundary(fUTF8IndexForUTF16Index[end]);
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

    this->ensureUTF16Mapping();

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
        if (!fUnicode->getWords(fText.c_str(), fText.size(), nullptr, &fWords)) {
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
    SkASSERT(clusterRange.start < SkToSizeT(fClusters.size()) &&
             clusterRange.end <= SkToSizeT(fClusters.size()));
    return SkSpan<Cluster>(&fClusters[clusterRange.start], clusterRange.width());
}

Cluster& ParagraphImpl::cluster(ClusterIndex clusterIndex) {
    SkASSERT(clusterIndex < SkToSizeT(fClusters.size()));
    return fClusters[clusterIndex];
}

Run& ParagraphImpl::runByCluster(ClusterIndex clusterIndex) {
    auto start = cluster(clusterIndex);
    return this->run(start.fRunIndex);
}

SkSpan<Block> ParagraphImpl::blocks(BlockRange blockRange) {
    SkASSERT(blockRange.start < SkToSizeT(fTextStyles.size()) &&
             blockRange.end <= SkToSizeT(fTextStyles.size()));
    return SkSpan<Block>(&fTextStyles[blockRange.start], blockRange.width());
}

Block& ParagraphImpl::block(BlockIndex blockIndex) {
    SkASSERT(blockIndex < SkToSizeT(fTextStyles.size()));
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
            SkASSERT(false);
            /*
            // The text is immutable and so are all the text indexing properties
            // taken from SkUnicode
            fCodeUnitProperties.reset();
            fWords.clear();
            fBidiRegions.clear();
            fUTF8IndexForUTF16Index.reset();
            fUTF16IndexForUTF8Index.reset();
            */
            [[fallthrough]];

        case kIndexed:
            fRuns.clear();
            fClusters.clear();
            [[fallthrough]];

        case kShaped:
            fLines.clear();
            [[fallthrough]];

        case kLineBroken:
            fPicture = nullptr;
            [[fallthrough]];

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
      textStyle.getFontFamilies(), textStyle.getFontStyle(), textStyle.getFontArguments());
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
        return SkUnicode::convertUtf16ToUtf8(fParagraphStyle.getEllipsisUtf16());
    }
}

void ParagraphImpl::updateFontSize(size_t from, size_t to, SkScalar fontSize) {

  SkASSERT(from == 0 && to == fText.size());
  auto defaultStyle = fParagraphStyle.getTextStyle();
  defaultStyle.setFontSize(fontSize);
  fParagraphStyle.setTextStyle(defaultStyle);

  for (auto& textStyle : fTextStyles) {
    textStyle.fStyle.setFontSize(fontSize);
  }

  fState = std::min(fState, kIndexed);
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

TArray<TextIndex> ParagraphImpl::countSurroundingGraphemes(TextRange textRange) const {
    textRange = textRange.intersection({0, fText.size()});
    TArray<TextIndex> graphemes;
    if ((fCodeUnitProperties[textRange.start] & SkUnicode::CodeUnitFlags::kGraphemeStart) == 0) {
        // Count the previous partial grapheme
        graphemes.emplace_back(textRange.start);
    }
    for (auto index = textRange.start; index < textRange.end; ++index) {
        if ((fCodeUnitProperties[index] & SkUnicode::CodeUnitFlags::kGraphemeStart) != 0) {
            graphemes.emplace_back(index);
        }
    }
    return graphemes;
}

TextIndex ParagraphImpl::findPreviousGraphemeBoundary(TextIndex utf8) const {
    while (utf8 > 0 &&
          (fCodeUnitProperties[utf8] & SkUnicode::CodeUnitFlags::kGraphemeStart) == 0) {
        --utf8;
    }
    return utf8;
}

TextIndex ParagraphImpl::findNextGraphemeBoundary(TextIndex utf8) const {
    while (utf8 < fText.size() &&
          (fCodeUnitProperties[utf8] & SkUnicode::CodeUnitFlags::kGraphemeStart) == 0) {
        ++utf8;
    }
    return utf8;
}

TextIndex ParagraphImpl::findNextGlyphClusterBoundary(TextIndex utf8) const {
    while (utf8 < fText.size() &&
          (fCodeUnitProperties[utf8] & SkUnicode::CodeUnitFlags::kGlyphClusterStart) == 0) {
        ++utf8;
    }
    return utf8;
}

TextIndex ParagraphImpl::findPreviousGlyphClusterBoundary(TextIndex utf8) const {
    while (utf8 > 0 &&
          (fCodeUnitProperties[utf8] & SkUnicode::CodeUnitFlags::kGlyphClusterStart) == 0) {
        --utf8;
    }
    return utf8;
}

void ParagraphImpl::ensureUTF16Mapping() {
    fillUTF16MappingOnce([&] {
        SkUnicode::extractUtfConversionMapping(
                this->text(),
                [&](size_t index) { fUTF8IndexForUTF16Index.emplace_back(index); },
                [&](size_t index) { fUTF16IndexForUTF8Index.emplace_back(index); });
    });
}

void ParagraphImpl::visit(const Visitor& visitor) {
    int lineNumber = 0;
    for (auto& line : fLines) {
        line.ensureTextBlobCachePopulated();
        for (auto& rec : line.fTextBlobCache) {
            if (rec.fBlob == nullptr) {
                continue;
            }
            SkTextBlob::Iter iter(*rec.fBlob);
            SkTextBlob::Iter::ExperimentalRun run;

            STArray<128, uint32_t> clusterStorage;
            const Run* R = rec.fVisitor_Run;
            const uint32_t* clusterPtr = &R->fClusterIndexes[0];

            if (R->fClusterStart > 0) {
                int count = R->fClusterIndexes.size();
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

int ParagraphImpl::getLineNumberAt(TextIndex codeUnitIndex) const {
    if (codeUnitIndex >= fText.size()) {
        return -1;
    }
    SkAssertResult(fLines.size() > 0);
    size_t startLine = 0;
    size_t endLine = fLines.size() - 1;
    if (fLines[endLine].textWithNewlines().end <= codeUnitIndex) {
        return -1;
    }

    while (endLine > startLine) {
        // startLine + 1 <= endLine, so we have startLine <= midLine <= endLine - 1.
        const size_t midLine = (endLine + startLine) / 2;
        const TextRange midLineRange = fLines[midLine].textWithNewlines();
        if (codeUnitIndex < midLineRange.start) {
            endLine = midLine - 1;
        } else if (midLineRange.end <= codeUnitIndex) {
            startLine = midLine + 1;
        } else {
            return midLine;
        }
    }
    SkAssertResult(startLine == endLine);
    return startLine;
}

int ParagraphImpl::getLineNumberAtUTF16Offset(size_t codeUnitIndex) {
    this->ensureUTF16Mapping();
    if (codeUnitIndex >= SkToSizeT(fUTF8IndexForUTF16Index.size())) {
        return -1;
    }
    const TextIndex utf8 = fUTF8IndexForUTF16Index[codeUnitIndex];
    return getLineNumberAt(utf8);
}

bool ParagraphImpl::getLineMetricsAt(int lineNumber, LineMetrics* lineMetrics) const {
    if (lineNumber < 0 || lineNumber >= fLines.size()) {
        return false;
    }
    auto& line = fLines[lineNumber];
    if (lineMetrics) {
        *lineMetrics = line.getMetrics();
    }
    return true;
}

TextRange ParagraphImpl::getActualTextRange(int lineNumber, bool includeSpaces) const {
    if (lineNumber < 0 || lineNumber >= fLines.size()) {
        return EMPTY_TEXT;
    }
    auto& line = fLines[lineNumber];
    return includeSpaces ? line.text() : line.trimmedText();
}

bool ParagraphImpl::getGlyphClusterAt(TextIndex codeUnitIndex, GlyphClusterInfo* glyphInfo) {
    const int lineNumber = getLineNumberAt(codeUnitIndex);
    if (lineNumber == -1) {
        return false;
    }
    auto& line = fLines[lineNumber];
    for (auto c = line.clustersWithSpaces().start; c < line.clustersWithSpaces().end; ++c) {
        auto& cluster = fClusters[c];
        if (cluster.contains(codeUnitIndex)) {
            std::vector<TextBox> boxes;
            line.getRectsForRange(cluster.textRange(),
                                    RectHeightStyle::kTight,
                                    RectWidthStyle::kTight,
                                    boxes);
            if (boxes.size() > 0) {
                if (glyphInfo) {
                    *glyphInfo = {boxes[0].rect, cluster.textRange(), boxes[0].direction};
                }
                return true;
            }
        }
    }
    return false;
}

bool ParagraphImpl::getClosestGlyphClusterAt(SkScalar dx,
                                             SkScalar dy,
                                             GlyphClusterInfo* glyphInfo) {
    const PositionWithAffinity res = this->getGlyphPositionAtCoordinate(dx, dy);
    SkAssertResult(res.position != 0 || res.affinity != Affinity::kUpstream);
    const size_t utf16Offset = res.position + (res.affinity == Affinity::kDownstream ? 0 : -1);
    this->ensureUTF16Mapping();
    SkAssertResult(utf16Offset < SkToSizeT(fUTF8IndexForUTF16Index.size()));
    return this->getGlyphClusterAt(fUTF8IndexForUTF16Index[utf16Offset], glyphInfo);
}

bool ParagraphImpl::getGlyphInfoAtUTF16Offset(size_t codeUnitIndex, GlyphInfo* glyphInfo) {
    this->ensureUTF16Mapping();
    if (codeUnitIndex >= SkToSizeT(fUTF8IndexForUTF16Index.size())) {
        return false;
    }
    const TextIndex utf8 = fUTF8IndexForUTF16Index[codeUnitIndex];
    const int lineNumber = getLineNumberAt(utf8);
    if (lineNumber == -1) {
        return false;
    }
    if (glyphInfo == nullptr) {
        return true;
    }
    const TextLine& line = fLines[lineNumber];
    const TextIndex startIndex = findPreviousGraphemeBoundary(utf8);
    const TextIndex endIndex = findNextGraphemeBoundary(utf8 + 1);
    const ClusterIndex glyphClusterIndex = clusterIndex(utf8);
    const Cluster& glyphCluster = cluster(glyphClusterIndex);

    // `startIndex` and `endIndex` must be on the same line.
    std::vector<TextBox> boxes;
    line.getRectsForRange({startIndex, endIndex}, RectHeightStyle::kTight, RectWidthStyle::kTight, boxes);
    SkAssertResult(boxes.size() > 0);
    if (glyphInfo && boxes.size() > 0) {
        *glyphInfo = {
            boxes[0].rect,
            { fUTF16IndexForUTF8Index[startIndex], fUTF16IndexForUTF8Index[endIndex] },
            boxes[0].direction,
            glyphCluster.run().isEllipsis(),
        };
    }
    return true;
}

bool ParagraphImpl::getClosestUTF16GlyphInfoAt(SkScalar dx, SkScalar dy, GlyphInfo* glyphInfo) {
    const PositionWithAffinity res = this->getGlyphPositionAtCoordinate(dx, dy);
    SkAssertResult(res.position != 0 || res.affinity != Affinity::kUpstream);
    const size_t utf16Offset = res.position + (res.affinity == Affinity::kDownstream ? 0 : -1);
    return getGlyphInfoAtUTF16Offset(utf16Offset, glyphInfo);
}

SkFont ParagraphImpl::getFontAt(TextIndex codeUnitIndex) const {
    for (auto& run : fRuns) {
        const auto textRange = run.textRange();
        if (textRange.start <= codeUnitIndex && codeUnitIndex < textRange.end) {
            return run.font();
        }
    }
    return SkFont();
}

SkFont ParagraphImpl::getFontAtUTF16Offset(size_t codeUnitIndex) {
    ensureUTF16Mapping();
    if (codeUnitIndex >= SkToSizeT(fUTF8IndexForUTF16Index.size())) {
        return SkFont();
    }
    const TextIndex utf8 = fUTF8IndexForUTF16Index[codeUnitIndex];
    for (auto& run : fRuns) {
        const auto textRange = run.textRange();
        if (textRange.start <= utf8 && utf8 < textRange.end) {
            return run.font();
        }
    }
    return SkFont();
}

std::vector<Paragraph::FontInfo> ParagraphImpl::getFonts() const {
    std::vector<FontInfo> results;
    for (auto& run : fRuns) {
        results.emplace_back(run.font(), run.textRange());
    }
    return results;
}

void ParagraphImpl::extendedVisit(const ExtendedVisitor& visitor) {
    int lineNumber = 0;
    for (auto& line : fLines) {
        line.iterateThroughVisualRuns(
            false,
            [&](const Run* run,
                SkScalar runOffsetInLine,
                TextRange textRange,
                SkScalar* runWidthInLine) {
                *runWidthInLine = line.iterateThroughSingleRunByStyles(
                TextLine::TextAdjustment::GlyphCluster,
                run,
                runOffsetInLine,
                textRange,
                StyleType::kNone,
                [&](TextRange textRange,
                    const TextStyle& style,
                    const TextLine::ClipContext& context) {
                    SkScalar correctedBaseline = SkScalarFloorToScalar(
                        line.baseline() + style.getBaselineShift() + 0.5);
                    SkPoint offset =
                        SkPoint::Make(line.offset().fX + context.fTextShift,
                                      line.offset().fY + correctedBaseline);
                    SkRect rect = context.clip.makeOffset(line.offset());
                    AutoSTArray<16, SkRect> glyphBounds;
                    glyphBounds.reset(SkToInt(run->size()));
                    run->font().getBounds(run->glyphs().data(),
                                          SkToInt(run->size()),
                                          glyphBounds.data(),
                                          nullptr);
                    STArray<128, uint32_t> clusterStorage;
                    const uint32_t* clusterPtr = run->clusterIndexes().data();
                    if (run->fClusterStart > 0) {
                        clusterStorage.reset(context.size);
                        for (size_t i = 0; i < context.size; ++i) {
                          clusterStorage[i] =
                              run->fClusterStart + run->fClusterIndexes[i];
                        }
                        clusterPtr = &clusterStorage[0];
                    }
                    const Paragraph::ExtendedVisitorInfo info = {
                        run->font(),
                        offset,
                        SkSize::Make(rect.width(), rect.height()),
                        SkToS16(context.size),
                        &run->glyphs()[context.pos],
                        &run->fPositions[context.pos],
                        &glyphBounds[context.pos],
                        clusterPtr,
                        0,  // flags
                    };
                    visitor(lineNumber, &info);
                });
            return true;
            });
        visitor(lineNumber, nullptr);   // signal end of line
        lineNumber += 1;
    }
}

int ParagraphImpl::getPath(int lineNumber, SkPath* dest) {
    int notConverted = 0;
    auto& line = fLines[lineNumber];
    line.iterateThroughVisualRuns(
              false,
              [&](const Run* run,
                  SkScalar runOffsetInLine,
                  TextRange textRange,
                  SkScalar* runWidthInLine) {
          *runWidthInLine = line.iterateThroughSingleRunByStyles(
          TextLine::TextAdjustment::GlyphCluster,
          run,
          runOffsetInLine,
          textRange,
          StyleType::kNone,
          [&](TextRange textRange,
              const TextStyle& style,
              const TextLine::ClipContext& context) {
              const SkFont& font = run->font();
              SkScalar correctedBaseline = SkScalarFloorToScalar(
                line.baseline() + style.getBaselineShift() + 0.5);
              SkPoint offset =
                  SkPoint::Make(line.offset().fX + context.fTextShift,
                                line.offset().fY + correctedBaseline);
              SkRect rect = context.clip.makeOffset(offset);
              struct Rec {
                  SkPath* fPath;
                  SkPoint fOffset;
                  const SkPoint* fPos;
                  int fNotConverted;
              } rec =
                  {dest, SkPoint::Make(rect.left(), rect.top()),
                   &run->positions()[context.pos], 0};
              font.getPaths(&run->glyphs()[context.pos], context.size,
                    [](const SkPath* path, const SkMatrix& mx, void* ctx) {
                        Rec* rec = reinterpret_cast<Rec*>(ctx);
                        if (path) {
                            SkMatrix total = mx;
                            total.postTranslate(rec->fPos->fX + rec->fOffset.fX,
                                                rec->fPos->fY + rec->fOffset.fY);
                            rec->fPath->addPath(*path, total);
                        } else {
                            rec->fNotConverted++;
                        }
                        rec->fPos += 1; // move to the next glyph's position
                    }, &rec);
              notConverted += rec.fNotConverted;
          });
        return true;
    });

    return notConverted;
}

SkPath Paragraph::GetPath(SkTextBlob* textBlob) {
    SkPath path;
    SkTextBlobRunIterator iter(textBlob);
    while (!iter.done()) {
        SkFont font = iter.font();
        struct Rec { SkPath* fDst; SkPoint fOffset; const SkPoint* fPos; } rec =
            {&path, {textBlob->bounds().left(), textBlob->bounds().top()},
             iter.points()};
        font.getPaths(iter.glyphs(), iter.glyphCount(),
            [](const SkPath* src, const SkMatrix& mx, void* ctx) {
                Rec* rec = (Rec*)ctx;
                if (src) {
                    SkMatrix tmp(mx);
                    tmp.postTranslate(rec->fPos->fX - rec->fOffset.fX,
                                      rec->fPos->fY - rec->fOffset.fY);
                    rec->fDst->addPath(*src, tmp);
                }
                rec->fPos += 1;
            },
            &rec);
        iter.next();
    }
    return path;
}

bool ParagraphImpl::containsEmoji(SkTextBlob* textBlob) {
    bool result = false;
    SkTextBlobRunIterator iter(textBlob);
    while (!iter.done() && !result) {
        // Walk through all the text by codepoints
        this->getUnicode()->forEachCodepoint(iter.text(), iter.textSize(),
           [&](SkUnichar unichar, int32_t start, int32_t end, int32_t count) {
                if (this->getUnicode()->isEmoji(unichar)) {
                    result = true;
                }
            });
        iter.next();
    }
    return result;
}

bool ParagraphImpl::containsColorFontOrBitmap(SkTextBlob* textBlob) {
    SkTextBlobRunIterator iter(textBlob);
    bool flag = false;
    while (!iter.done() && !flag) {
        iter.font().getPaths(
            (const SkGlyphID*) iter.glyphs(),
            iter.glyphCount(),
            [](const SkPath* path, const SkMatrix& mx, void* ctx) {
                if (path == nullptr) {
                    bool* flag1 = (bool*)ctx;
                    *flag1 = true;
                }
            }, &flag);
        iter.next();
    }
    return flag;
}

}  // namespace textlayout
}  // namespace skia
