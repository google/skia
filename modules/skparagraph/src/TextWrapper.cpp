/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "TextWrapper.h"
#include "ParagraphImpl.h"

namespace skia {
namespace textlayout {

// Since we allow cluster clipping when they don't fit
// we have to work with stretches - parts of clusters
void TextWrapper::lookAhead(SkScalar maxWidth, Cluster* endOfClusters) {
    fWords.startFrom(fEndLine.startCluster(), fEndLine.startPos());
    fClusters.startFrom(fEndLine.startCluster(), fEndLine.startPos());
    fClip.startFrom(fEndLine.startCluster(), fEndLine.startPos());
    for (auto cluster = fEndLine.endCluster(); cluster < endOfClusters; ++cluster) {
        if (fWords.width() + fClusters.width() + cluster->width() >= maxWidth) {
            if (cluster->isWhitespaces()) {
                break;
            }
            if (cluster->width() > maxWidth) {
                // Break the cluster into parts by glyph position
                auto delta = maxWidth - (fWords.width() + fClusters.width());
                fClip.extend(cluster, cluster->roundPos(delta));
                fTooLongCluster = true;
                fTooLongWord = true;
                break;
            }

            // Walk further to see if there is a too long word, cluster or glyph
            SkScalar nextWordLength = fClusters.width();
            for (auto further = cluster; further != endOfClusters; ++further) {
                if (further->isSoftBreak() || further->isHardBreak()) {
                    break;
                }
                nextWordLength += further->width();
            }
            if (nextWordLength > maxWidth) {
                // If the word is too long we can break it right now and hope it's enough
                fTooLongWord = true;
            }

            // TODO: this is the place when we use hyphenation
            fMinIntrinsicWidth = SkTMax(fMinIntrinsicWidth, nextWordLength);
            break;
        }

        fClusters.extend(cluster);

        // Keep adding clusters/words
        if (fClusters.endOfWord()) {
            fWords.extend(fClusters);
            fMinIntrinsicWidth = SkTMax(fMinIntrinsicWidth, fWords.width());
        }

        if ((fHardLineBreak = cluster->isHardBreak())) {
            // Stop at the hard line break
            break;
        }
    }
}

void TextWrapper::moveForward() {
    do {
        if (fWords.width() > 0) {
            fEndLine.extend(fWords);
        } else if (fClusters.width() > 0) {
            fEndLine.extend(fClusters);
            fTooLongWord = false;
        } else if (fClip.width() > 0) {
            fEndLine.extend(fClip);
            fTooLongWord = false;
            fTooLongCluster = false;
        } else {
            break;
        }
    } while (fTooLongWord || fTooLongCluster);
}

// Special case for start/end cluster since they can be clipped
void TextWrapper::trimEndSpaces() {
    // Remember the breaking position
    fEndLine.saveBreak();
    // Move the end of the line to the left
    for (auto cluster = fEndLine.endCluster();
         cluster >= fEndLine.startCluster() && cluster->isWhitespaces();
         --cluster) {
        fEndLine.trim(cluster);
    }
    fEndLine.trim();
}

// Trim the beginning spaces in case of soft line break
void TextWrapper::trimStartSpaces(Cluster* endOfClusters) {
    // Restore the breaking position
    fEndLine.restoreBreak();
    fEndLine.nextPos();
    if (fHardLineBreak) {
        // End of line is always end of cluster, but need to skip \n
        fEndLine.startFrom(fEndLine.endCluster(), 0);
        return;
    }
    if (fEndLine.endPos() != 0) {
        // Clipping
        fEndLine.startFrom(fEndLine.endCluster(), fEndLine.endPos());
        return;
    }

    auto cluster = fEndLine.endCluster();
    while (cluster < endOfClusters && cluster->isWhitespaces()) {
        ++cluster;
    }
    fEndLine.startFrom(cluster, 0);
}

void TextWrapper::breakTextIntoLines(ParagraphImpl* parent,
                                     SkSpan<Cluster> span,
                                     SkScalar maxWidth,
                                     size_t maxLines,
                                     const SkString& ellipsisStr,
                                     const AddLineToParagraph& addLine) {
    fHeight = 0;
    fEndLine = TextStretch(span.begin(), span.begin());
    auto end = &span.back();
    while (fEndLine.endCluster() != end) {
        reset();

        lookAhead(maxWidth, end);
        moveForward();

        trimEndSpaces();

        auto reachedTheEnd =
                maxLines != std::numeric_limits<size_t>::max() && fLineNumber >= maxLines;
        // TODO: perform ellipsis work here
        if (parent->strutEnabled()) {
            // Make sure font metrics are not less than the strut
            parent->strutMetrics().updateLineMetrics(fEndLine.metrics(),
                                                     parent->strutForceHeight());
        }
        addLine(fEndLine.startCluster(),
                fEndLine.endCluster(),
                fEndLine.startPos(),
                fEndLine.endPos(),
                SkVector::Make(0, fHeight),
                SkVector::Make(fEndLine.width(), fEndLine.metrics().height()),
                fEndLine.metrics(),
                reachedTheEnd && fEndLine.endCluster() != end && ellipsisStr.size() != 0);

        // Start a new line
        fHeight += fEndLine.metrics().height();
        trimStartSpaces(end);

        if (reachedTheEnd) {
            break;
        }
        ++fLineNumber;
    }

    if (fHardLineBreak) {
        // Last character is a line break
        if (parent->strutEnabled()) {
            // Make sure font metrics are not less than the strut
            parent->strutMetrics().updateLineMetrics(fEndLine.metrics(),
                                                     parent->strutForceHeight());
        }
        addLine(fEndLine.breakCluster(),
                fEndLine.breakCluster(),
                0,
                0,
                SkVector::Make(0, fHeight),
                SkVector::Make(0, fEndLine.metrics().height()),
                fEndLine.metrics(),
                false);
    }
}

}  // namespace textlayout
}  // namespace skia
