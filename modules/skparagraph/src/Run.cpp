// Copyright 2019 Google LLC.
#include "modules/skparagraph/src/Run.h"
#include <unicode/brkiter.h>
#include "include/core/SkFontMetrics.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include <algorithm>
#include "src/utils/SkUTF.h"

namespace {
/*
SkUnichar utf8_next(const char** ptr, const char* end) {
    SkUnichar val = SkUTF::NextUTF8(ptr, end);
    return val < 0 ? 0xFFFD : val;
}
*/
}

namespace skia {
namespace textlayout {

ShapedSpan::ShapedSpan(Run* run)
    : ShapedSpan(*run, GlyphRange(0, run->size())) { }

ShapedSpan::ShapedSpan(TextRange textRange, GlyphRange glyphRange)
  : TextRange(textRange)
  , fGlyphs(glyphRange) {
}

Run::Run(ParagraphImpl* master,
         const SkShaper::RunHandler::RunInfo& info,
         size_t firstChar,
         SkScalar lineHeight,
         size_t index,
         SkScalar offsetX)
        : ShapedSpan(
            info.fBidiLevel % 2 == 0
            ? TextRange(firstChar + info.utf8Range.begin(), firstChar + info.utf8Range.end())
            : TextRange(firstChar + info.utf8Range.end(), firstChar + info.utf8Range.begin()),
            GlyphRange(0, info.glyphCount))
        , fMaster(master)
        , fClusterRange(EMPTY_CLUSTERS)
        , fClusterStart(firstChar) {
    fFont = info.fFont;
    fHeightMultiplier = lineHeight;
    fBidiLevel = info.fBidiLevel;
    fAdvance = info.fAdvance;
    fIndex = index;
    fUtf8Range = info.utf8Range;
    fOffset = SkVector::Make(offsetX, 0);
    fGlyphs.push_back_n(info.glyphCount);
    fBounds.push_back_n(info.glyphCount);
    fPositions.push_back_n(info.glyphCount + 1);
    fOffsets.push_back_n(info.glyphCount + 1);
    fClusterIndexes.push_back_n(info.glyphCount + 1);
    fShifts.push_back_n(info.glyphCount + 1, 0.0);
    info.fFont.getMetrics(&fFontMetrics);
    fSpaced = false;
    // To make edge cases easier:
    fPositions[info.glyphCount] = fOffset + fAdvance;
    fOffsets[info.glyphCount] = { 0, 0};
    fClusterIndexes[info.glyphCount] = info.utf8Range.end();
    fEllipsis = false;
    fPlaceholderIndex = std::numeric_limits<size_t>::max();
}

SkShaper::RunHandler::Buffer Run::newRunBuffer() {
    return {fGlyphs.data(), fPositions.data(), fOffsets.data(), fClusterIndexes.data(), fOffset};
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
        correction = fJustificationShifts[end - 1].fX -
                     fJustificationShifts[start].fY;
    }
    return posX(end) - posX(start) + shift + correction;
}

void Run::copyTo(SkTextBlobBuilder& builder, size_t pos, size_t size, SkVector runOffset) const {
    SkASSERT(pos + size <= this->size());
    const auto& blobBuffer = builder.allocRunPos(fFont, SkToInt(size));
    sk_careful_memcpy(blobBuffer.glyphs, fGlyphs.data() + pos, size * sizeof(SkGlyphID));
    for (size_t i = 0; i < size; ++i) {
        auto point = fPositions[i + pos];
        auto offset = fOffsets[i + pos];
        point.offset(offset.fX, offset.fY);
        if (fSpaced) {
            point.fX += fShifts[i + pos];
        }
        if (!fJustificationShifts.empty()) {
            point.fX += fJustificationShifts[i + pos].fX;
        }
        blobBuffer.points()[i] = point + runOffset;
    }
}

// TODO: Make it at least a binary search
// It's going to be one range since we are looking into one run
std::tuple<bool, ClusterIndex, ClusterIndex> Run::findLimitingClusters(TextRange text, bool extendToClusters) const {

    if (text.width() == 0) {
        for (auto i = fClusterRange.start; i != fClusterRange.end; ++i) {
            auto& cluster = fMaster->cluster(i);
            if (cluster.contains(text.start)) {
                return std::make_tuple(true, i, i);
            }
        }
        return std::make_tuple(false, EMPTY_CLUSTER, EMPTY_CLUSTER);
    }

    ClusterIndex start = EMPTY_CLUSTER;
    ClusterIndex end = EMPTY_CLUSTER;
    for (auto i = fClusterRange.start; i != fClusterRange.end; ++i) {
        auto& cluster = fMaster->cluster(i);
        if (cluster.before(text)) {
            continue;
        } else if (cluster.after(text)) {
            break;
        }

        if (cluster.intersects(text)) {
            if (start == EMPTY_CLUSTER) {
                start = i;
            }
            end = i;
        } else {
            SkASSERT(false);
        }
    }


    if (start == EMPTY_CLUSTER || end == EMPTY_CLUSTER) {
        return std::make_tuple(false, start, end);
    }

    // TODO:LTR
    if (!leftToRight()) {
        std::swap(start, end);
    }

    return std::make_tuple(true, start, end);
}

// TODO:LTR
void Run::iterateThroughClustersInTextOrder(const ClusterVisitor& visitor) {
    // Can't figure out how to do it with one code for both cases without 100 ifs
    if (leftToRight()) {
        size_t start = 0;
        size_t cluster = this->clusterIndex(start);
        for (size_t glyph = 1; glyph <= this->size(); ++glyph) {
            auto nextCluster = this->clusterIndex(glyph);
            if (nextCluster <= cluster) {
                // Skip all non-sequential clusters
                continue;
            }

            visitor(start,
                    glyph,
                    fClusterStart + cluster,
                    fClusterStart + nextCluster,
                    this->calculateWidth(start, glyph, glyph == size()),
                    this->calculateHeight());

            start = glyph;
            cluster = nextCluster;
        }
    } else {
        size_t glyph = this->size();
        size_t cluster = this->fUtf8Range.begin();
        for (int32_t start = this->size() - 1; start >= 0; --start) {
            size_t nextCluster = start == 0 ? this->fUtf8Range.end() : this->clusterIndex(start - 1);
            if (nextCluster <= cluster) {
                // Skip all non-sequential clusters
                continue;
            }

            visitor(start,
                    glyph,
                    fClusterStart + nextCluster,
                    fClusterStart + cluster,
                    this->calculateWidth(start, glyph, glyph == 0),
                    this->calculateHeight());

            glyph = start;
            cluster = nextCluster;
        }
    }
}

SkScalar Run::addSpacesAtTheEnd(SkScalar space, Cluster* cluster) {
    if (cluster->glyphSize() == 0) {
        return 0;
    }

    // Add the space
    fShifts[cluster->glyphEnd() - 1] += space;

    // Increase the run width
    fSpaced = true;
    fAdvance.fX += space;
    cluster->widen(space);

    return space;
}

SkScalar Run::addSpacesEvenly(SkScalar space, Cluster* cluster) {
    // Offset all the glyphs in the cluster
    SkScalar shift = 0;
    for (size_t i = cluster->glyphStart(); i < cluster->glyphEnd(); ++i) {
        fShifts[i] += shift;
        shift += space;
    }
    if (this->size() == cluster->glyphEnd()) {
        // To make calculations easier
        fShifts[cluster->glyphEnd()] += shift;
    }
    // Increment the run width
    fSpaced = true;
    fAdvance.fX += shift;
    cluster->widen(shift);
    cluster->setHalfLetterSpacing(space / 2);

    return shift;
}

void Run::shift(const Cluster* cluster, SkScalar offset) {
    if (offset == 0) {
        return;
    }

    fSpaced = true;
    for (size_t i = cluster->glyphStart(); i < cluster->glyphEnd(); ++i) {
        fShifts[i] += offset;
    }
    if (this->size() == cluster->glyphEnd()) {
        // To make calculations easier
        fShifts[cluster->glyphEnd()] += offset;
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

    // Make sure the placeholder can fit the line
    endlineMetrics->add(this);
}

//?
SkScalar Cluster::sizeToChar(TextIndex ch) const {
    if (!this->contains(ch)) {
        return 0;
    }
    auto shift = ch - this->start;
    auto ratio = shift * 1.0 / this->width();

    return SkDoubleToScalar(fWidth * ratio);
}

//?
SkScalar Cluster::sizeFromChar(TextIndex ch) const {
    if (!this->contains(ch)) {
        return 0;
    }
    auto shift = this->end - ch - 1;
    auto ratio = shift * 1.0 / this->width();

    return SkDoubleToScalar(fWidth * ratio);
}

SkScalar Cluster::trimmedWidth(size_t pos) const {
    // Find the width until the pos and return the min between trimmedWidth and the width(pos)
    // We don't have to take in account cluster shift since it's the same for 0 and for pos
    return std::min(fRun->positionX(pos) - fRun->positionX(fGlyphs.start), fWidth);
}

SkScalar Run::positionX(size_t pos) const {
    return posX(pos) + fShifts[pos] +
            (fJustificationShifts.empty() ? 0 : fJustificationShifts[pos].fY);
}

PlaceholderStyle* Run::placeholderStyle() const {
    if (isPlaceholder()) {
        return &fMaster->placeholders()[fPlaceholderIndex].fStyle;
    } else {
        return nullptr;
    }
}

SkFont Cluster::font() const {
    return fRun->font();
}

Cluster::Cluster(Run* run, ShapedSpan shapedSpan, SkScalar width)
        : fRun(run)
        , ShapedSpan(shapedSpan)
        , fWidth(width)
        , fHalfLetterSpacing(0.0)
        , fWhiteSpaces(false)
        , fBreakType(None) {
}

}  // namespace textlayout
}  // namespace skia
