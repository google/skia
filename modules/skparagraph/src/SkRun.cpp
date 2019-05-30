/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkRun.h"
#include "include/core/SkFontMetrics.h"

SkRun::SkRun(SkSpan<const char> text,
             const SkShaper::RunHandler::RunInfo& info,
             SkScalar lineHeight,
             size_t index,
             SkScalar offsetX) {
    TRACE_EVENT0("skia", TRACE_FUNC);
    fFont = info.fFont;
    fHeightMultiplier = lineHeight;
    fBidiLevel = info.fBidiLevel;
    fAdvance = info.fAdvance;
    fText = SkSpan<const char>(text.begin() + info.utf8Range.begin(), info.utf8Range.size());

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

SkShaper::RunHandler::Buffer SkRun::newRunBuffer() {
    TRACE_EVENT0("skia", TRACE_FUNC);
    return {fGlyphs.data(), fPositions.data(), nullptr, fClusterIndexes.data(), fOffset};
}

SkScalar SkRun::calculateWidth(size_t start, size_t end, bool clip) const {
    TRACE_EVENT0("skia", TRACE_FUNC);
    SkASSERT(start <= end);
    //clip |= end == size();  // Clip at the end of the run?
    SkScalar offset = 0;
    if (fSpaced && end > start) {
        offset = fOffsets[clip ? end - 1 : end] - fOffsets[start];
    }
    return fPositions[end].fX - fPositions[start].fX + offset;
}

void SkRun::copyTo(SkTextBlobBuilder& builder, size_t pos, size_t size, SkVector offset) const {
    TRACE_EVENT0("skia", TRACE_FUNC);
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
std::tuple<bool, SkCluster*, SkCluster*> SkRun::findLimitingClusters(SkSpan<const char> text) {
    if (text.empty()) {
        SkCluster* found = nullptr;
        for (auto& cluster : fClusters) {
            if (cluster.contains(text.begin())) {
                found = &cluster;
                break;
            }
        }
        return std::make_tuple(found != nullptr, found, found);
    }

    auto first = text.begin();
    auto last = text.end() - 1;

    SkCluster* start = nullptr;
    SkCluster* end = nullptr;
    for (auto& cluster : fClusters) {
        if (cluster.contains(first)) start = &cluster;
        if (cluster.contains(last)) end = &cluster;
    }
    if (!leftToRight()) {
        std::swap(start, end);
    }

    return std::make_tuple(start != nullptr && end != nullptr, start, end);
}

void SkRun::iterateThroughClustersInTextOrder(const ClusterVisitor& visitor) {
    TRACE_EVENT0("skia", TRACE_FUNC);
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
        };
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

SkScalar SkRun::addSpacesAtTheEnd(SkScalar space, SkCluster* cluster) {
    TRACE_EVENT0("skia", TRACE_FUNC);
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

SkScalar SkRun::addSpacesEvenly(SkScalar space, SkCluster* cluster) {
    TRACE_EVENT0("skia", TRACE_FUNC);
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

void SkRun::shift(const SkCluster* cluster, SkScalar offset) {
    TRACE_EVENT0("skia", TRACE_FUNC);
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

void SkCluster::setIsWhiteSpaces() {
    TRACE_EVENT0("skia", TRACE_FUNC);
    auto pos = fText.end();
    while (--pos >= fText.begin()) {
        auto ch = *pos;
        if (!u_isspace(ch) && u_charType(ch) != U_CONTROL_CHAR &&
            u_charType(ch) != U_NON_SPACING_MARK) {
            return;
        }
    }
    fWhiteSpaces = true;
}

SkScalar SkCluster::sizeToChar(const char* ch) const {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (ch < fText.begin() || ch >= fText.end()) {
        return 0;
    }
    auto shift = ch - fText.begin();
    auto ratio = shift * 1.0 / fText.size();

    return SkDoubleToScalar(fWidth * ratio);
}

SkScalar SkCluster::sizeFromChar(const char* ch) const {
    TRACE_EVENT0("skia", TRACE_FUNC);
    if (ch < fText.begin() || ch >= fText.end()) {
        return 0;
    }
    auto shift = fText.end() - ch - 1;
    auto ratio = shift * 1.0 / fText.size();

    return SkDoubleToScalar(fWidth * ratio);
}

size_t SkCluster::roundPos(SkScalar s) const {
    TRACE_EVENT0("skia", TRACE_FUNC);
    auto ratio = (s * 1.0) / fWidth;
    return  sk_double_floor2int(ratio * size());
}

SkScalar SkCluster::trimmedWidth(size_t pos) const {
    // Find the width until the pos and return the min between trimmedWidth and the width(pos)
    return SkTMin(this->run()->positionX(pos) - this->run()->positionX(fStart), fWidth - fSpacing);
}
