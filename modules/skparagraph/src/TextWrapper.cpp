// Copyright 2019 Google LLC.
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/TextWrapper.h"

namespace skia {
namespace textlayout {

namespace {
SkScalar littleRound(SkScalar a) {
    // This rounding is done to match Flutter tests. Must be removed..
  return SkScalarRoundToScalar(a * 100.0)/100.0;
}
}

// Since we allow cluster clipping when they don't fit
// we have to work with stretches - parts of clusters
void TextWrapper::lookAhead(SkScalar maxWidth, Cluster* endOfClusters) {
    fWords.startFrom(fEndLine.startCluster(), fEndLine.startPos());
    fClusters.startFrom(fEndLine.startCluster(), fEndLine.startPos());
    fClip.startFrom(fEndLine.startCluster(), fEndLine.startPos());

    for (auto cluster = fEndLine.endCluster(); cluster < endOfClusters; ++cluster) {
        // TODO: Trying to deal with flutter rounding problem. Must be removed...
        auto width = fWords.width() + fClusters.width() + cluster->width();
        auto roundedWidth = littleRound(width);
        if (cluster->isHardBreak()) {
        } else if (maxWidth == 0.0f) {
            // Do nothing
        } else if (roundedWidth > maxWidth) {
            if (cluster->isWhitespaces()) {
                // It's the end of the word
                fClusters.extend(cluster);
                fMinIntrinsicWidth = SkTMax(fMinIntrinsicWidth, getClustersTrimmedWidth());
                fWords.extend(fClusters);
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
                if (further->isSoftBreak() || further->isHardBreak() || further->isWhitespaces()) {
                    break;
                }
                nextWordLength += further->width();
            }
            if (nextWordLength > maxWidth) {
                // If the word is too long we can break it right now and hope it's enough
                fTooLongWord = true;
            }

            // TODO: this is the place when we use hyphenation
            fMinIntrinsicWidth = SkTMax(fMinIntrinsicWidth, fTooLongWord ? maxWidth : nextWordLength);
            break;
        }

        fClusters.extend(cluster);

        // Keep adding clusters/words
        if (fClusters.endOfWord()) {
            fMinIntrinsicWidth = SkTMax(fMinIntrinsicWidth, getClustersTrimmedWidth());
            fWords.extend(fClusters);
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
void TextWrapper::trimEndSpaces(TextAlign align) {
    // Remember the breaking position
    fEndLine.saveBreak();
    // Skip all space cluster at the end
    //bool left = align == TextAlign::kStart || align == TextAlign::kLeft;
    bool right = align == TextAlign::kRight || align == TextAlign::kEnd;
    for (auto cluster = fEndLine.endCluster();
         cluster >= fEndLine.startCluster() && cluster->isWhitespaces();
         --cluster) {
        if ((cluster->run()->leftToRight()) ||
            (right && !cluster->run()->leftToRight()) ||
             align == TextAlign::kJustify || align == TextAlign::kCenter) {
            fEndLine.trim(cluster);
            continue;
        } else {
            break;
        }
    }
    if (!right || true) {
        fEndLine.trim();
    }
}

SkScalar TextWrapper::getClustersTrimmedWidth() {
    // Move the end of the line to the left
    SkScalar width = fClusters.width();
    auto cluster = fClusters.endCluster();
    for (; cluster > fClusters.startCluster() && cluster->isWhitespaces(); --cluster) {
        width -= cluster->width();
    }
    if (cluster >= fClusters.startCluster()) {
        if (cluster->isWhitespaces()) {
            width -= cluster->width();
        } else {
            width -= (cluster->width() - cluster->trimmedWidth(cluster->endPos()));
        }
    }
    return width;
}

// Trim the beginning spaces in case of soft line break
std::tuple<Cluster*, size_t, SkScalar> TextWrapper::trimStartSpaces(Cluster* endOfClusters) {

    if (fHardLineBreak) {
        // End of line is always end of cluster, but need to skip \n
        auto width = fEndLine.width();
        auto cluster = fEndLine.endCluster() + 1;
        while (cluster < fEndLine.breakCluster() && cluster->isWhitespaces()) {
            width += cluster->width();
            ++cluster;
        }
        return std::make_tuple(fEndLine.breakCluster() + 1, 0, width);
    }

    auto width = fEndLine.withWithGhostSpaces(); //fEndLine.width();
    auto cluster = fEndLine.breakCluster() + 1;
    while (cluster < endOfClusters && cluster->isWhitespaces()) {
        width += cluster->width();
        ++cluster;
    }
    return std::make_tuple(cluster, 0, width);
}

void TextWrapper::breakTextIntoLines(ParagraphImpl* parent,
                                     SkScalar maxWidth,
                                     const AddLineToParagraph& addLine) {
    fHeight = 0;
    fMinIntrinsicWidth = 0;
    fMaxIntrinsicWidth = 0;

    auto span = parent->clusters();
    if (span.size() == 0) {
      return;
    }
    auto maxLines = parent->paragraphStyle().getMaxLines();
    auto& ellipsisStr = parent->paragraphStyle().getEllipsis();
    auto align = parent->paragraphStyle().effective_align();

    SkScalar softLineMaxIntrinsicWidth = 0;
    fEndLine = TextStretch(span.begin(), span.begin(), parent->strutForceHeight());
    auto end = span.end() - 1;
    auto start = span.begin();
    InternalLineMetrics maxRunMetrics;
    auto needEllipsis = false;
    auto endlessLine = maxLines == std::numeric_limits<size_t>::max();
    while (fEndLine.endCluster() != end) {

        reset();

        fEndLine.metrics().clean();
        lookAhead(maxWidth, end);
        moveForward();

        // Do not trim end spaces on the naturally last line of the left aligned text
        trimEndSpaces(align);

        // For soft line breaks add to the line all the spaces next to it
        Cluster* startLine;
        size_t pos;
        SkScalar widthWithSpaces;
        std::tie(startLine, pos, widthWithSpaces) = trimStartSpaces(end);

        needEllipsis =
            fEndLine.endCluster() < end - 1 &&
            SkScalarIsFinite(maxWidth) &&
            !ellipsisStr.isEmpty();

        auto exceededLines = !endlessLine && fLineNumber >= maxLines;

        // TODO: perform ellipsis work here

        // If the line is empty with the hard line break, let's take the paragraph font (flutter???)
        if (fHardLineBreak && fEndLine.width() == 0) {
            auto emptyMetrics = parent->computeEmptyMetrics();
            fEndLine.setMetrics(emptyMetrics);
        }

        // Deal with placeholder clusters == runs[@size==1]
        for (auto cluster = fEndLine.startCluster(); cluster <= fEndLine.endCluster(); ++cluster) {
            if (cluster->run()->placeholder() != nullptr) {
                SkASSERT(cluster->run()->size() == 1);
                // Update the placeholder metrics so we can get the placeholder positions later
                // and the line metrics (to make sure the placeholder fits)
                cluster->run()->updateMetrics(&fEndLine.metrics());
            }
        }

        // Before we update the line metrics with struts,
        // let's save it for GetRectsForRange(RectHeightStyle::kMax)
        maxRunMetrics = fEndLine.metrics();
        maxRunMetrics.fForceStrut = false;

        if (parent->strutEnabled()) {
            // Make sure font metrics are not less than the strut
            parent->strutMetrics().updateLineMetrics(fEndLine.metrics());
        }

        // TODO: keep start/end/break info for text and runs but in a better way that below
        TextRange text(fEndLine.startCluster()->textRange().start, fEndLine.endCluster()->textRange().end);
        TextRange textWithSpaces(fEndLine.startCluster()->textRange().start, startLine->textRange().start);
        if (fEndLine.breakCluster()->isHardBreak()) {
            textWithSpaces.end = fEndLine.breakCluster()->textRange().start;
        } else if (startLine == end) {
            textWithSpaces.end = parent->text().size();
        }
        ClusterRange clusters(fEndLine.startCluster() - start, fEndLine.endCluster() - start + 1);
        ClusterRange clustersWithGhosts(fEndLine.startCluster() - start, startLine - start);
        addLine(text, textWithSpaces, clusters, clustersWithGhosts, widthWithSpaces,
                fEndLine.startPos(),
                fEndLine.endPos(),
                SkVector::Make(0, fHeight),
                SkVector::Make(fEndLine.width(), fEndLine.metrics().height()),
                fEndLine.metrics(),
                needEllipsis && exceededLines && !fHardLineBreak);

        parent->lines().back().setMaxRunMetrics(maxRunMetrics);

        softLineMaxIntrinsicWidth += widthWithSpaces;
        fMaxIntrinsicWidth = SkMaxScalar(fMaxIntrinsicWidth, softLineMaxIntrinsicWidth);
        if (fHardLineBreak) {
            softLineMaxIntrinsicWidth = 0;
        }
        // Start a new line
        fHeight += fEndLine.metrics().height();
        if (!fHardLineBreak || startLine != end) {
            fEndLine.clean();
        }
        fEndLine.startFrom(startLine, pos);
        parent->fMaxWidthWithTrailingSpaces = SkMaxScalar(parent->fMaxWidthWithTrailingSpaces, widthWithSpaces);

        if (exceededLines || (needEllipsis && endlessLine && !fHardLineBreak)) {
            fHardLineBreak = false;
            break;
        }

        ++fLineNumber;
    }

    // We finished formatting the text but we need to scan the rest for some numbers
    auto cluster = fEndLine.endCluster();
    while (cluster != end) {
        fExceededMaxLines = true;
        if (cluster->isHardBreak()) {
            softLineMaxIntrinsicWidth = 0;
        } else {
            softLineMaxIntrinsicWidth += cluster->width();
            fMaxIntrinsicWidth = SkTMax(fMaxIntrinsicWidth, softLineMaxIntrinsicWidth);
        }
        ++cluster;
    }

    if (fHardLineBreak) {

        // Last character is a line break
        if (parent->strutEnabled()) {
            // Make sure font metrics are not less than the strut
            parent->strutMetrics().updateLineMetrics(fEndLine.metrics());
        }
        TextRange empty(fEndLine.breakCluster()->textRange().start, fEndLine.breakCluster()->textRange().start);
        TextRange hardBreak(fEndLine.breakCluster()->textRange().end, fEndLine.breakCluster()->textRange().end);
        ClusterRange clusters(fEndLine.breakCluster() - start, fEndLine.endCluster() - start);
        addLine(empty, hardBreak, clusters, clusters,
                0,
                0,
                0,
                SkVector::Make(0, fHeight),
                SkVector::Make(0, fEndLine.metrics().height()),
                fEndLine.metrics(),
                needEllipsis);
        fHeight += fEndLine.metrics().height();
        parent->lines().back().setMaxRunMetrics(maxRunMetrics);
    }
}

}  // namespace textlayout
}  // namespace skia
