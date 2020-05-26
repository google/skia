// Copyright 2019 Google LLC.
#include "modules/skparagraph/src/Cluster.h"
#include <unicode/brkiter.h>
#include "include/core/SkFontMetrics.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/Run.h"
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
    return std::min(run.positionX(pos) - run.positionX(fStart), fWidth);
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
