// Copyright 2019 Google LLC.
#include "include/core/SkFontMetrics.h"
#include "include/core/SkTextBlob.h"
#include "include/private/base/SkFloatingPoint.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkTo.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/Run.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/base/SkUTF.h"

namespace skia {
namespace textlayout {

Run::Run(ParagraphImpl* owner,
         const SkShaper::RunHandler::RunInfo& info,
         size_t firstChar,
         SkScalar heightMultiplier,
         bool useHalfLeading,
         SkScalar baselineShift,
         size_t index,
         SkScalar offsetX)
    : fOwner(owner)
    , fTextRange(firstChar + info.utf8Range.begin(), firstChar + info.utf8Range.end())
    , fClusterRange(EMPTY_CLUSTERS)
    , fFont(info.fFont)
    , fClusterStart(firstChar)
    , fGlyphData(std::make_shared<GlyphData>())
    , fGlyphs(fGlyphData->glyphs)
    , fPositions(fGlyphData->positions)
    , fOffsets(fGlyphData->offsets)
    , fClusterIndexes(fGlyphData->clusterIndexes)
    , fHeightMultiplier(heightMultiplier)
    , fUseHalfLeading(useHalfLeading)
    , fBaselineShift(baselineShift)
{
    fBidiLevel = info.fBidiLevel;
    fAdvance = info.fAdvance;
    fIndex = index;
    fUtf8Range = info.utf8Range;
    fOffset = SkVector::Make(offsetX, 0);

    fGlyphs.push_back_n(info.glyphCount);
    fPositions.push_back_n(info.glyphCount + 1);
    fOffsets.push_back_n(info.glyphCount + 1);
    fClusterIndexes.push_back_n(info.glyphCount + 1);
    info.fFont.getMetrics(&fFontMetrics);

    this->calculateMetrics();

    // To make edge cases easier:
    fPositions[info.glyphCount] = fOffset + fAdvance;
    fOffsets[info.glyphCount] = {0, 0};
    fClusterIndexes[info.glyphCount] = this->leftToRight() ? info.utf8Range.end() : info.utf8Range.begin();
    fEllipsis = false;
    fPlaceholderIndex = std::numeric_limits<size_t>::max();
}

void Run::calculateMetrics() {
    fCorrectAscent = fFontMetrics.fAscent - fFontMetrics.fLeading * 0.5;
    fCorrectDescent = fFontMetrics.fDescent + fFontMetrics.fLeading * 0.5;
    fCorrectLeading = 0;
    if (SkScalarNearlyZero(fHeightMultiplier)) {
        return;
    }
    const auto runHeight = fHeightMultiplier * fFont.getSize();
    const auto fontIntrinsicHeight = fCorrectDescent - fCorrectAscent;
    if (fUseHalfLeading) {
        const auto extraLeading = (runHeight - fontIntrinsicHeight) / 2;
        fCorrectAscent -= extraLeading;
        fCorrectDescent += extraLeading;
    } else {
        const auto multiplier = runHeight / fontIntrinsicHeight;
        fCorrectAscent *= multiplier;
        fCorrectDescent *= multiplier;
    }
    // If we shift the baseline we need to make sure the shifted text fits the line
    fCorrectAscent += fBaselineShift;
    fCorrectDescent += fBaselineShift;
}

SkShaper::RunHandler::Buffer Run::newRunBuffer() {
    return {fGlyphs.data(), fPositions.data(), fOffsets.data(), fClusterIndexes.data(), fOffset};
}

void Run::copyTo(SkTextBlobBuilder& builder, size_t pos, size_t size) const {
    SkASSERT(pos + size <= this->size());
    const auto& blobBuffer = builder.allocRunPos(fFont, SkToInt(size));
    sk_careful_memcpy(blobBuffer.glyphs, fGlyphs.data() + pos, size * sizeof(SkGlyphID));

    for (size_t i = 0; i < size; ++i) {
        auto point = fPositions[i + pos];
        if (!fJustificationShifts.empty()) {
            point.fX += fJustificationShifts[i + pos].fX;
        }
        point += fOffsets[i + pos];
        blobBuffer.points()[i] = point;
    }
}

// Find a cluster range from text range (within one run)
// Cluster range is normalized ([start:end) start < end regardless of TextDirection
// Boolean value in triple indicates whether the cluster range was found or not
std::tuple<bool, ClusterIndex, ClusterIndex> Run::findLimitingClusters(TextRange text) const {
    if (text.width() == 0) {
        // Special Flutter case for "\n" and "...\n"
        if (text.end > this->fTextRange.start) {
            ClusterIndex index = fOwner->clusterIndex(text.end - 1);
            return std::make_tuple(true, index, index);
        } else {
            return std::make_tuple(false, 0, 0);
        }
    }

    ClusterRange clusterRange;
    bool found = true;
    // Deal with the case when either start or end are not align with glyph cluster edge
    // In such case we shift the text range to the right
    // (cutting from the left and adding to the right)
    if (leftToRight()) {
        // LTR: [start:end)
        found = clusterRange.start != fClusterRange.end;
        clusterRange.start = fOwner->clusterIndex(text.start);
        clusterRange.end = fOwner->clusterIndex(text.end - 1);
    } else {
        // RTL: (start:end]
        clusterRange.start = fOwner->clusterIndex(text.end);
        clusterRange.end = fOwner->clusterIndex(text.start + 1);
        found = clusterRange.end != fClusterRange.start;
    }

    return std::make_tuple(
            found,
            clusterRange.start,
            clusterRange.end);
}

std::tuple<bool, TextIndex, TextIndex> Run::findLimitingGlyphClusters(TextRange text) const {
    TextIndex start = fOwner->findPreviousGlyphClusterBoundary(text.start);
    TextIndex end = fOwner->findNextGlyphClusterBoundary(text.end);
    return std::make_tuple(true, start, end);
}

// Adjust the text to grapheme edges so the first grapheme start is in the text and the last grapheme start is in the text
// It actually means that the first grapheme is entirely in the text and the last grapheme does not have to be
// 12345 234 2:2 -> 2,5 4:4
std::tuple<bool, TextIndex, TextIndex> Run::findLimitingGraphemes(TextRange text) const {
    TextIndex start = fOwner->findPreviousGraphemeBoundary(text.start);
    TextIndex end = fOwner->findNextGraphemeBoundary(text.end);
    return std::make_tuple(true, start, end);
}

void Run::iterateThroughClusters(const ClusterVisitor& visitor) {

    for (size_t index = 0; index < fClusterRange.width(); ++index) {
        auto correctIndex = leftToRight() ? fClusterRange.start + index : fClusterRange.end - index - 1;
        auto cluster = &fOwner->cluster(correctIndex);
        visitor(cluster);
    }
}

void Run::addSpacesAtTheEnd(SkScalar space, Cluster* cluster) {
    // Increment the run width
    fAdvance.fX += space;
    // Increment the cluster width
    cluster->space(space);
}

SkScalar Run::addSpacesEvenly(SkScalar space) {
    SkScalar shift = 0;
    for (size_t i = 0; i < this->size(); ++i) {
        fPositions[i].fX += shift;
        shift += space;
    }
    fPositions[this->size()].fX += shift;
    fAdvance.fX += shift;
    return shift;
}

SkScalar Run::addSpacesEvenly(SkScalar space, Cluster* cluster) {
    // Offset all the glyphs in the cluster
    SkScalar shift = 0;
    for (size_t i = cluster->startPos(); i < cluster->endPos(); ++i) {
        fPositions[i].fX += shift;
        shift += space;
    }
    if (this->size() == cluster->endPos()) {
        // To make calculations easier
        fPositions[cluster->endPos()].fX += shift;
    }
    // Increment the run width
    fAdvance.fX += shift;
    // Increment the cluster width
    cluster->space(shift);
    cluster->setHalfLetterSpacing(space / 2);

    return shift;
}

void Run::shift(const Cluster* cluster, SkScalar offset) {
    if (offset == 0) {
        return;
    }
    for (size_t i = cluster->startPos(); i < cluster->endPos(); ++i) {
        fPositions[i].fX += offset;
    }
    if (this->size() == cluster->endPos()) {
        // To make calculations easier
        fPositions[cluster->endPos()].fX += offset;
    }
}

void Run::extend(const Cluster* cluster, SkScalar offset) {
    // Extend the cluster at the end
    fPositions[cluster->endPos()].fX += offset;
}

void Run::updateMetrics(InternalLineMetrics* endlineMetrics) {

    SkASSERT(isPlaceholder());
    auto placeholderStyle = this->placeholderStyle();
    // Difference between the placeholder baseline and the line bottom
    SkScalar baselineAdjustment = 0;
    switch (placeholderStyle->fBaseline) {
        case TextBaseline::kAlphabetic:
            break;

        case TextBaseline::kIdeographic:
            baselineAdjustment = endlineMetrics->deltaBaselines() / 2;
            break;
    }

    auto height = placeholderStyle->fHeight;
    auto offset = placeholderStyle->fBaselineOffset;

    fFontMetrics.fLeading = 0;
    switch (placeholderStyle->fAlignment) {
        case PlaceholderAlignment::kBaseline:
            fFontMetrics.fAscent = baselineAdjustment - offset;
            fFontMetrics.fDescent = baselineAdjustment + height - offset;
            break;

        case PlaceholderAlignment::kAboveBaseline:
            fFontMetrics.fAscent = baselineAdjustment - height;
            fFontMetrics.fDescent = baselineAdjustment;
            break;

        case PlaceholderAlignment::kBelowBaseline:
            fFontMetrics.fAscent = baselineAdjustment;
            fFontMetrics.fDescent = baselineAdjustment + height;
            break;

        case PlaceholderAlignment::kTop:
            fFontMetrics.fDescent = height + fFontMetrics.fAscent;
            break;

        case PlaceholderAlignment::kBottom:
            fFontMetrics.fAscent = fFontMetrics.fDescent - height;
            break;

        case PlaceholderAlignment::kMiddle:
            auto mid = (-fFontMetrics.fDescent - fFontMetrics.fAscent)/2.0;
            fFontMetrics.fDescent = height/2.0 - mid;
            fFontMetrics.fAscent =  - height/2.0 - mid;
            break;
    }

    this->calculateMetrics();

    // Make sure the placeholder can fit the line
    endlineMetrics->add(this);
}

SkScalar Cluster::sizeToChar(TextIndex ch) const {
    if (ch < fTextRange.start || ch >= fTextRange.end) {
        return 0;
    }
    auto shift = ch - fTextRange.start;
    auto ratio = shift * 1.0 / fTextRange.width();

    return SkDoubleToScalar(fWidth * ratio);
}

SkScalar Cluster::sizeFromChar(TextIndex ch) const {
    if (ch < fTextRange.start || ch >= fTextRange.end) {
        return 0;
    }
    auto shift = fTextRange.end - ch - 1;
    auto ratio = shift * 1.0 / fTextRange.width();

    return SkDoubleToScalar(fWidth * ratio);
}

size_t Cluster::roundPos(SkScalar s) const {
    auto ratio = (s * 1.0) / fWidth;
    return sk_double_floor2int(ratio * size());
}

SkScalar Cluster::trimmedWidth(size_t pos) const {
    // Find the width until the pos and return the min between trimmedWidth and the width(pos)
    // We don't have to take in account cluster shift since it's the same for 0 and for pos
    auto& run = fOwner->run(fRunIndex);
    return std::min(run.positionX(pos) - run.positionX(fStart), fWidth);
}

SkScalar Run::positionX(size_t pos) const {
    return posX(pos) + (fJustificationShifts.empty() ? 0 : fJustificationShifts[pos].fY);
}

PlaceholderStyle* Run::placeholderStyle() const {
    if (isPlaceholder()) {
        return &fOwner->placeholders()[fPlaceholderIndex].fStyle;
    } else {
        return nullptr;
    }
}

bool Run::isResolved() const {
    for (auto& glyph :fGlyphs) {
        if (glyph == 0) {
            return false;
        }
    }
    return true;
}

Run* Cluster::runOrNull() const {
    if (fRunIndex >= fOwner->runs().size()) {
        return nullptr;
    }
    return &fOwner->run(fRunIndex);
}

Run& Cluster::run() const {
    SkASSERT(fRunIndex < fOwner->runs().size());
    return fOwner->run(fRunIndex);
}

SkFont Cluster::font() const {
    SkASSERT(fRunIndex < fOwner->runs().size());
    return fOwner->run(fRunIndex).font();
}

bool Cluster::isSoftBreak() const {
    return fOwner->codeUnitHasProperty(fTextRange.end,
                                       SkUnicode::CodeUnitFlags::kSoftLineBreakBefore);
}

bool Cluster::isGraphemeBreak() const {
    return fOwner->codeUnitHasProperty(fTextRange.end, SkUnicode::CodeUnitFlags::kGraphemeStart);
}
}  // namespace textlayout
}  // namespace skia
