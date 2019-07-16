// Copyright 2019 Google LLC.
#include "modules/skparagraph/src/Run.h"
#include <unicode/brkiter.h>
#include "include/core/SkFontMetrics.h"
#include "modules/skparagraph/src/ParagraphImpl.h"

namespace skia {
namespace textlayout {

const char* accessText(ParagraphImpl* master) { return master->text().begin(); }
const Cluster* accessCluster(ParagraphImpl* master) { return master->clusters().begin(); }
const Run* accessRun(ParagraphImpl* master) { return master->runs().begin(); }
Run* accessRunRef(ParagraphImpl* master) { return master->runs().begin(); }
const Block* accessTextBlock(ParagraphImpl* master) { return master->styles().begin(); }

Run::Run(ParagraphImpl* master,
         SkSpan<const char> text,
         const SkShaper::RunHandler::RunInfo& info,
         SkScalar lineHeight,
         size_t index,
         SkScalar offsetX)
        : fMaster(master)
        , fTextRange(info.utf8Range.begin(), info.utf8Range.end())
        , fClusterRange(StableRange<ParagraphImpl, const Cluster, &accessCluster>(master)) {
    fFont = info.fFont;
    fHeightMultiplier = lineHeight;
    fBidiLevel = info.fBidiLevel;
    fAdvance = info.fAdvance;
    fIndex = index;
    fUtf8Range = info.utf8Range;
    fOffset = SkVector::Make(offsetX, 0);
    fGlyphs.push_back_n(info.glyphCount);
    fPositions.push_back_n(info.glyphCount + 1);
    fOffsets.push_back_n(info.glyphCount + 1, SkScalar(0));
    fClusterIndexes.push_back_n(info.glyphCount + 1);
    info.fFont.getMetrics(&fFontMetrics);
    fSpaced = false;
    // To make edge cases easier:
    fPositions[info.glyphCount] = fOffset + fAdvance;
    fClusterIndexes[info.glyphCount] = info.utf8Range.end();
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
    return fPositions[end].fX - fPositions[start].fX + offset;
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
            blobBuffer.points()[i] = point + offset;
        }
    } else {
        // Good for the first line
        sk_careful_memcpy(blobBuffer.points(), fPositions.data() + pos, size * sizeof(SkPoint));
    }
}

// TODO: Make the search more effective
std::tuple<bool, const Cluster*, const Cluster*> Run::findLimitingClusters(TextRange text) {
    if (text == EMPTY_TEXT) {
        const Cluster* found = nullptr;
        for (auto& cluster : fClusterRange) {
            if (cluster.contains(text.start)) {
                found = &cluster;
                break;
            }
        }
        return std::make_tuple(found != nullptr, found, found);
    }

    auto first = text.start;
    auto last = text.end - 1;

    const Cluster* start = nullptr;
    const Cluster* end = nullptr;
    for (auto& cluster : fClusterRange) {
        if (cluster.contains(first)) start = &cluster;
        if (cluster.contains(last)) end = &cluster;
    }
    if (!leftToRight()) {
        std::swap(start, end);
    }

    return std::make_tuple(start != nullptr && end != nullptr, start, end);
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
                    cluster,
                    nextCluster,
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
                    cluster,
                    nextCluster,
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

void Cluster::setIsWhiteSpaces() {
    auto text = fMaster->text();
    auto pos = fTextRange.end;
    while (--pos >= fTextRange.start) {
        auto ch = text[pos];
        if (!u_isspace(ch) && u_charType(ch) != U_CONTROL_CHAR &&
            u_charType(ch) != U_NON_SPACING_MARK) {
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
    auto& run = fMaster->getRun(fRunIndex);
    return SkTMin(run.positionX(pos) - run.positionX(fStart), fWidth - fSpacing);
}

void Cluster::shift(SkScalar offset) const {
    fMaster->getRun(fRunIndex).shift(this, offset);
}

Run* Cluster::run() const {
    if (fRunIndex >= fMaster->runs().size()) {
        return nullptr;
    }
    return &fMaster->getRun(fRunIndex);
}

SkFont Cluster::font() const {
    return fMaster->getRun(fRunIndex).font();
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
        , fStart(start)
        , fEnd(end)
        , fWidth(width)
        , fSpacing(0)
        , fHeight(height)
        , fWhiteSpaces(false)
        , fBreakType(None) {
}

}  // namespace textlayout
}  // namespace skia
