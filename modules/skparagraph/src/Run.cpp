// Copyright 2019 Google LLC.
#include "include/core/SkFontMetrics.h"
#include "include/core/SkTextBlob.h"
#include "include/private/SkFloatingPoint.h"
#include "include/private/SkMalloc.h"
#include "include/private/SkTo.h"
#include "modules/skparagraph/include/DartTypes.h"
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/Run.h"
#include "modules/skshaper/include/SkShaper.h"
#include "src/utils/SkUTF.h"

namespace skia {
namespace textlayout {

Run::Run(ParagraphImpl* owner,
         const SkShaper::RunHandler::RunInfo& info,
         size_t firstChar,
         SkScalar heightMultiplier,
         size_t index,
         SkScalar offsetX)
    : fOwner(owner)
    , fTextRange(firstChar + info.utf8Range.begin(), firstChar + info.utf8Range.end())
    , fClusterRange(EMPTY_CLUSTERS)
    , fFont(info.fFont)
    , fClusterStart(firstChar)
    , fHeightMultiplier(heightMultiplier)
{
    fBidiLevel = info.fBidiLevel;
    fAdvance = info.fAdvance;
    fIndex = index;
    fUtf8Range = info.utf8Range;
    fOffset = SkVector::Make(offsetX, 0);
    fGlyphs.push_back_n(info.glyphCount);
    fBounds.push_back_n(info.glyphCount);
    fPositions.push_back_n(info.glyphCount + 1);
    fClusterIndexes.push_back_n(info.glyphCount + 1);
    fShifts.push_back_n(info.glyphCount + 1, 0.0);
    info.fFont.getMetrics(&fFontMetrics);

    this->calculateMetrics();

    fSpaced = false;
    // To make edge cases easier:
    fPositions[info.glyphCount] = fOffset + fAdvance;
    fClusterIndexes[info.glyphCount] = this->leftToRight() ? info.utf8Range.end() : info.utf8Range.begin();
    fEllipsis = false;
    fPlaceholderIndex = std::numeric_limits<size_t>::max();
}

void Run::calculateMetrics() {
    fCorrectAscent = fFontMetrics.fAscent - fFontMetrics.fLeading * 0.5;
    fCorrectDescent = fFontMetrics.fDescent + fFontMetrics.fLeading * 0.5;
    fCorrectLeading = 0;
    if (!SkScalarNearlyZero(fHeightMultiplier)) {
        auto multiplier = fHeightMultiplier * fFont.getSize() /
                             (fFontMetrics.fDescent - fFontMetrics.fAscent + fFontMetrics.fLeading);
        fCorrectAscent *= multiplier;
        fCorrectDescent *= multiplier;
    }
}

SkShaper::RunHandler::Buffer Run::newRunBuffer() {
    return {fGlyphs.data(), fPositions.data(), nullptr, fClusterIndexes.data(), fOffset};
}

void Run::commit() {
    fFont.getBounds(fGlyphs.data(), fGlyphs.size(), fBounds.data(), nullptr);
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

void Run::copyTo(SkTextBlobBuilder& builder, size_t pos, size_t size) const {
    SkASSERT(pos + size <= this->size());
    const auto& blobBuffer = builder.allocRunPos(fFont, SkToInt(size));
    sk_careful_memcpy(blobBuffer.glyphs, fGlyphs.data() + pos, size * sizeof(SkGlyphID));

    if (!fSpaced && fJustificationShifts.empty()) {
        sk_careful_memcpy(blobBuffer.points(), fPositions.data() + pos, size * sizeof(SkPoint));
    } else {
        for (size_t i = 0; i < size; ++i) {
            auto point = fPositions[i + pos];
            if (fSpaced) {
                point.fX += fShifts[i + pos];
            }
            if (!fJustificationShifts.empty()) {
                point.fX += fJustificationShifts[i + pos].fX;
            }
            blobBuffer.points()[i] = point;
        }
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

    ClusterIndex startIndex = fOwner->clusterIndex(text.start);
    ClusterIndex endIndex = fOwner->clusterIndex(text.end - 1);
    if (!leftToRight()) {
        std::swap(startIndex, endIndex);
    }
    return std::make_tuple(startIndex != fClusterRange.end && endIndex != fClusterRange.end, startIndex, endIndex);
}

// Adjust the text to grapheme edges so the first grapheme start is in the text and the last grapheme start is in the text
// It actually means that the first grapheme is entirely in the text and the last grapheme does not have to be
std::tuple<bool, TextIndex, TextIndex> Run::findLimitingGraphemes(TextRange text) const {
    TextIndex start = fOwner->findNextGraphemeBoundary(text.start);
    TextIndex end = fOwner->findNextGraphemeBoundary(text.end);
    return std::make_tuple(true, start, end);
}

void Run::iterateThroughClustersInTextOrder(const ClusterTextVisitor& visitor) {
    // Can't figure out how to do it with one code for both cases without 100 ifs
    // Can't go through clusters because there are no cluster table yet
    if (leftToRight()) {
        size_t start = 0;
        size_t cluster = this->clusterIndex(start);
        for (size_t glyph = 1; glyph <= this->size(); ++glyph) {
            auto nextCluster = this->clusterIndex(glyph);
            if (nextCluster <= cluster) {
                continue;
            }

            visitor(start,
                    glyph,
                    fClusterStart + cluster,
                    fClusterStart + nextCluster,
                    this->calculateWidth(start, glyph, glyph == size()),
                    this->calculateHeight(LineMetricStyle::CSS, LineMetricStyle::CSS));

            start = glyph;
            cluster = nextCluster;
        }
    } else {
        size_t glyph = this->size();
        size_t cluster = this->fUtf8Range.begin();
        for (int32_t start = this->size() - 1; start >= 0; --start) {
            size_t nextCluster =
                    start == 0 ? this->fUtf8Range.end() : this->clusterIndex(start - 1);
            if (nextCluster <= cluster) {
                continue;
            }

            visitor(start,
                    glyph,
                    fClusterStart + cluster,
                    fClusterStart + nextCluster,
                    this->calculateWidth(start, glyph, glyph == 0),
                    this->calculateHeight(LineMetricStyle::CSS, LineMetricStyle::CSS));

            glyph = start;
            cluster = nextCluster;
        }
    }
}

void Run::iterateThroughClusters(const ClusterVisitor& visitor) {

    for (size_t index = 0; index < fClusterRange.width(); ++index) {
        auto correctIndex = leftToRight() ? fClusterRange.start + index : fClusterRange.end - index - 1;
        auto cluster = &fOwner->cluster(correctIndex);
        visitor(cluster);
    }
}

SkScalar Run::addSpacesAtTheEnd(SkScalar space, Cluster* cluster) {
    if (cluster->endPos() == cluster->startPos()) {
        return 0;
    }

    fShifts[cluster->endPos() - 1] += space;
    // Increment the run width
    fSpaced = true;
    fAdvance.fX += space;
    // Increment the cluster width
    cluster->space(space, space);

    return space;
}

SkScalar Run::addSpacesEvenly(SkScalar space, Cluster* cluster) {
    // Offset all the glyphs in the cluster
    SkScalar shift = 0;
    for (size_t i = cluster->startPos(); i < cluster->endPos(); ++i) {
        fShifts[i] += shift;
        shift += space;
    }
    if (this->size() == cluster->endPos()) {
        // To make calculations easier
        fShifts[cluster->endPos()] += shift;
    }
    // Increment the run width
    fSpaced = true;
    fAdvance.fX += shift;
    // Increment the cluster width
    cluster->space(shift, space);
    cluster->setHalfLetterSpacing(space / 2);

    return shift;
}

void Run::shift(const Cluster* cluster, SkScalar offset) {
    if (offset == 0) {
        return;
    }

    fSpaced = true;
    for (size_t i = cluster->startPos(); i < cluster->endPos(); ++i) {
        fShifts[i] += offset;
    }
    if (this->size() == cluster->endPos()) {
        // To make calculations easier
        fShifts[cluster->endPos()] += offset;
    }
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
    return posX(pos) + fShifts[pos] +
            (fJustificationShifts.empty() ? 0 : fJustificationShifts[pos].fY);
}

PlaceholderStyle* Run::placeholderStyle() const {
    if (isPlaceholder()) {
        return &fOwner->placeholders()[fPlaceholderIndex].fStyle;
    } else {
        return nullptr;
    }
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
    return fOwner->codeUnitHasProperty(fTextRange.end, CodeUnitFlags::kSoftLineBreakBefore);
}

bool Cluster::isGraphemeBreak() const {
    return fOwner->codeUnitHasProperty(fTextRange.end, CodeUnitFlags::kGraphemeStart);
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
    size_t len = fOwner->getWhitespacesLength(fTextRange);
    fIsWhiteSpaces = (len == this->fTextRange.width());
    fIsSpaces = fOwner->isSpace(fTextRange);
    fIsHardBreak = fOwner->codeUnitHasProperty(fTextRange.end, CodeUnitFlags::kHardLineBreakBefore);
}

}  // namespace textlayout
}  // namespace skia
