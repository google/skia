/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkClusterator.h"

#include "SkGlyphRun.h"
#include "SkTo.h"
#include "SkUTF.h"

static bool is_reversed(const uint32_t* clusters, uint32_t count) {
    // "ReversedChars" is how PDF deals with RTL text.
    // return true if more than one cluster and monotonicly decreasing to zero.
    if (count < 2 || clusters[0] == 0 || clusters[count - 1] != 0) {
        return false;
    }
    for (uint32_t i = 0; i + 1 < count; ++i) {
        if (clusters[i + 1] > clusters[i]) {
            return false;
        }
    }
    return true;
}

SkClusterator::SkClusterator(const SkGlyphRun& run)
    : fClusters(run.clusters().data())
    , fUtf8Text(run.text().data())
    , fGlyphCount(SkToU32(run.glyphsIDs().size()))
    , fTextByteLength(SkToU32(run.text().size()))
    , fReversedChars(fClusters ? is_reversed(fClusters, fGlyphCount) : false)
{
    if (fClusters) {
        SkASSERT(fUtf8Text && fTextByteLength > 0 && fGlyphCount > 0);
    } else {
        SkASSERT(!fUtf8Text && fTextByteLength == 0);
    }
}

SkClusterator::Cluster SkClusterator::next() {
    if (fCurrentGlyphIndex >= fGlyphCount) {
        return Cluster{nullptr, 0, 0, 0};
    }
    if (!fClusters || !fUtf8Text) {
        return Cluster{nullptr, 0, fCurrentGlyphIndex++, 1};
    }
    uint32_t clusterGlyphIndex = fCurrentGlyphIndex;
    uint32_t cluster = fClusters[clusterGlyphIndex];
    do {
        ++fCurrentGlyphIndex;
    } while (fCurrentGlyphIndex < fGlyphCount && cluster == fClusters[fCurrentGlyphIndex]);
    uint32_t clusterGlyphCount = fCurrentGlyphIndex - clusterGlyphIndex;
    uint32_t clusterEnd = fTextByteLength;
    for (unsigned i = 0; i < fGlyphCount; ++i) {
       uint32_t c = fClusters[i];
       if (c > cluster && c < clusterEnd) {
           clusterEnd = c;
       }
    }
    uint32_t clusterLen = clusterEnd - cluster;
    return Cluster{fUtf8Text + cluster, clusterLen, clusterGlyphIndex, clusterGlyphCount};
}
