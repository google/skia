// Copyright 2019 Google LLC.
#include "modules/skparagraph/src/Run.h"
#include <unicode/brkiter.h>
#include "include/core/SkFontMetrics.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include <algorithm>
#include "src/utils/SkUTF.h"

namespace {

SkUnichar utf8_next(const char** ptr, const char* end) {
    SkUnichar val = SkUTF::NextUTF8(ptr, end);
    return val < 0 ? 0xFFFD : val;
}
}

namespace skia {
namespace textlayout {

Run::Run(ParagraphImpl* master,
         const SkShaper::RunHandler::RunInfo& info,
         size_t firstChar,
         SkScalar lineHeight,
         size_t index,
         SkScalar offsetX)
        : fMaster(master)
        , fTextRange(firstChar + info.utf8Range.begin(), firstChar + info.utf8Range.end())
        , fClusterRange(EMPTY_CLUSTERS)
        , fFirstChar(firstChar) {
    fFont = info.fFont;
    fHeightMultiplier = lineHeight;
    fBidiLevel = info.fBidiLevel;
    fAdvance = info.fAdvance;
    fIndex = index;
    fUtf8Range = info.utf8Range;
    fOffset = SkVector::Make(offsetX, 0);
    fGlyphs.push_back_n(info.glyphCount);
    fPositions.push_back_n(info.glyphCount + 1);
    fOffsets.push_back_n(info.glyphCount + 1, 0.0);
    fClusterIndexes.push_back_n(info.glyphCount + 1);
    info.fFont.getMetrics(&fFontMetrics);
    fSpaced = false;
    // To make edge cases easier:
    fPositions[info.glyphCount] = fOffset + fAdvance;
    fClusterIndexes[info.glyphCount] = info.utf8Range.end();
    fEllipsis = false;
}

SkShaper::RunHandler::Buffer Run::newRunBuffer() {
    return {fGlyphs.data(), fPositions.data(), nullptr, fClusterIndexes.data(), fOffset};
}

SkScalar Run::calculateWidth(size_t start, size_t end, bool clip) const {
    SkASSERT(start <= end);
    // clip |= end == size();  // Clip at the end of the run?
    SkScalar offset = 0;
    if (fSpaced && end > start) {
        offset = fOffsets[clip ? end - 1 : end] - fOffsets[start];
    }
    auto correction = end > start ? fMaster->posShift(fIndex, end - 1) - fMaster->posShift(fIndex, start) : 0;
    return fPositions[end].fX - fPositions[start].fX + offset + correction;
}

void Run::copyTo(SkTextBlobBuilder& builder, size_t pos, size_t size, SkVector offset) const {
    SkASSERT(pos + size <= this->size());
    const auto& blobBuffer = builder.allocRunPos(fFont, SkToInt(size));
    sk_careful_memcpy(blobBuffer.glyphs, fGlyphs.data() + pos, size * sizeof(SkGlyphID));

    if (fSpaced || offset.fX != 0 || offset.fY != 0) {
        for (size_t i = 0; i < size; ++i) {
            auto point = fPositions[i + pos];
            if (fSpaced) {
                point.fX += fOffsets[i + pos];
            }
            point.fX += fMaster->posShift(fIndex, i + pos);
            blobBuffer.points()[i] = point + offset;
        }
    } else {
        // Good for the first line
        sk_careful_memcpy(blobBuffer.points(), fPositions.data() + pos, size * sizeof(SkPoint));
    }
}

std::tuple<bool, ClusterIndex, ClusterIndex> Run::findLimitingClusters(TextRange text, bool onlyInnerClusters) const {

    if (text.width() == 0) {
        for (auto i = fClusterRange.start; i != fClusterRange.end; ++i) {
            auto& cluster = fMaster->cluster(i);
            if (cluster.textRange().end >= text.end && cluster.textRange().start <= text.start) {
                return std::make_tuple(true, i, i);
            }
        }
        return std::make_tuple(false, 0, 0);
    }
    Cluster* start = nullptr;
    Cluster* end = nullptr;
    if (onlyInnerClusters) {
        for (auto i = fClusterRange.start; i != fClusterRange.end; ++i) {
            auto& cluster = fMaster->cluster(i);
            if (cluster.textRange().start >= text.start && start == nullptr) {
                start = &cluster;
            }
            if (cluster.textRange().end <= text.end) {
                end = &cluster;
            } else {
                break;
            }
        }
    } else {
        for (auto i = fClusterRange.start; i != fClusterRange.end; ++i) {
            auto& cluster = fMaster->cluster(i);
            if (cluster.textRange().end > text.start && start == nullptr) {
                start = &cluster;
            }
            if (cluster.textRange().start < text.end) {
                end = &cluster;
            } else {
                break;
            }
        }
    }

    if (start == nullptr || end == nullptr) {
        return std::make_tuple(false, 0, 0);
    }

    if (!leftToRight()) {
        std::swap(start, end);
    }

    size_t startIndex = start - fMaster->clusters().begin();
    size_t endIndex   = end   - fMaster->clusters().begin();
    return std::make_tuple(startIndex != fClusterRange.end && endIndex != fClusterRange.end, startIndex, endIndex);
}

void Run::iterateThroughClustersInTextOrder(const ClusterVisitor& visitor) {
    // Can't figure out how to do it with one code for both cases without 100 ifs
    // Can't go through clusters because there are no cluster table yet
    if (leftToRight()) {
        size_t start = 0;
        size_t cluster = this->clusterIndex(start);
        for (size_t glyph = 1; glyph <= this->size(); ++glyph) {
            auto nextCluster = this->clusterIndex(glyph);
            if (nextCluster == cluster) {
                continue;
            }

            visitor(start,
                    glyph,
                    fFirstChar + cluster,
                    fFirstChar + nextCluster,
                    this->calculateWidth(start, glyph, glyph == size()),
                    this->calculateHeight());

            start = glyph;
            cluster = nextCluster;
        }
    } else {
        size_t glyph = this->size();
        size_t cluster = this->fUtf8Range.begin();
        for (int32_t start = this->size() - 1; start >= 0; --start) {
            size_t nextCluster =
                    start == 0 ? this->fUtf8Range.end() : this->clusterIndex(start - 1);
            if (nextCluster == cluster) {
                continue;
            }

            visitor(start,
                    glyph,
                    fFirstChar + cluster,
                    fFirstChar + nextCluster,
                    this->calculateWidth(start, glyph, glyph == 0),
                    this->calculateHeight());

            glyph = start;
            cluster = nextCluster;
        }
    }
}

SkScalar Run::addSpacesAtTheEnd(SkScalar space, Cluster* cluster) {
    if (cluster->endPos() == cluster->startPos()) {
        return 0;
    }

    fOffsets[cluster->endPos() - 1] += space;
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
        fOffsets[i] += shift;
        shift += space;
    }
    if (this->size() == cluster->endPos()) {
        // To make calculations easier
        fOffsets[cluster->endPos()] += shift;
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
        fOffsets[i] += offset;
    }
    if (this->size() == cluster->endPos()) {
        // To make calculations easier
        fOffsets[cluster->endPos()] += offset;
    }
}

void Run::updateMetrics(InternalLineMetrics* endlineMetrics) {

    // Difference between the placeholder baseline and the line bottom
    SkScalar baselineAdjustment = 0;
    switch (fPlaceholder->fBaseline) {
        case TextBaseline::kAlphabetic:
            break;

        case TextBaseline::kIdeographic:
            baselineAdjustment = endlineMetrics->deltaBaselines() / 2;
            break;
    }

    auto height = fPlaceholder->fHeight;
    auto offset = fPlaceholder->fBaselineOffset;

    fFontMetrics.fLeading = 0;
    switch (fPlaceholder->fAlignment) {
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

void Cluster::setIsWhiteSpaces() {

    fWhiteSpaces = false;

    auto span = fMaster->text(fTextRange);
    const char* ch = span.begin();
    while (ch < span.end()) {
        auto unichar = utf8_next(&ch, span.end());
        if (!u_isWhitespace(unichar)) {
            return;
        }
    }
    fWhiteSpaces = true;
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
    auto& run = fMaster->run(fRunIndex);
    return SkTMin(run.positionX(pos) - run.positionX(fStart), fWidth - fSpacing);
}

SkScalar Run::positionX(size_t pos) const {
    return fPositions[pos].fX + fOffsets[pos] + fMaster->posShift(fIndex, pos);
}

Run* Cluster::run() const {
    if (fRunIndex >= fMaster->runs().size()) {
        return nullptr;
    }
    return &fMaster->run(fRunIndex);
}

SkFont Cluster::font() const {
    return fMaster->run(fRunIndex).font();
}

Cluster::Cluster(ParagraphImpl* master,
        RunIndex runIndex,
        size_t start,
        size_t end,
        SkSpan<const char> text,
        SkScalar width,
        SkScalar height)
        : fMaster(master)
        , fRunIndex(runIndex)
        , fTextRange(text.begin() - fMaster->text().begin(), text.end() - fMaster->text().begin())
        , fGraphemeRange(EMPTY_RANGE)
        , fStart(start)
        , fEnd(end)
        , fWidth(width)
        , fSpacing(0)
        , fHeight(height)
        , fHalfLetterSpacing(0.0)
        , fWhiteSpaces(false)
        , fBreakType(None) {
}

}  // namespace textlayout
}  // namespace skia
