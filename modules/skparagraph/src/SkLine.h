/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "SkRun.h"
#include "include/core/SkCanvas.h"
#include "include/private/SkTArray.h"
#include "include/private/SkTHash.h"
#include "modules/skparagraph/include/SkDartTypes.h"
#include "modules/skparagraph/include/SkTextStyle.h"
#include "src/core/SkSpan.h"

class SkBlock {
public:
    SkBlock() : fText(), fTextStyle() {}
    SkBlock(SkSpan<const char> text, const SkTextStyle& style) : fText(text), fTextStyle(style) {}

    inline SkSpan<const char> text() const { return fText; }
    inline SkTextStyle style() const { return fTextStyle; }

    void add(SkSpan<const char> tail) {
        SkASSERT(fText.end() == tail.begin());
        fText = SkSpan<const char>(fText.begin(), fText.size() + tail.size());
    }

protected:
    SkSpan<const char> fText;
    SkTextStyle fTextStyle;
};

class SkLine {
public:
    SkLine() = default;
    SkLine(SkLine&&);
    ~SkLine() = default;

    SkLine(SkVector offset, SkVector advance, SkSpan<const SkBlock> blocks, SkSpan<const char> text,
           SkSpan<const SkCluster> clusters, size_t start, size_t end, SkLineMetrics sizes);

    inline SkSpan<const char> text() const { return fText; }
    inline SkSpan<const SkCluster> clusters() const { return fClusters; }
    inline SkVector offset() const { return fOffset + SkVector::Make(fShift, 0); }
    inline SkRun* ellipsis() const { return fEllipsis.get(); }
    inline SkLineMetrics sizes() const { return fSizes; }
    inline bool empty() const { return fText.empty(); }

    inline SkScalar shift() const { return fShift; }
    inline SkScalar height() const { return fAdvance.fY; }
    SkScalar width() const {
        return fAdvance.fX + (fEllipsis != nullptr ? fEllipsis->fAdvance.fX : 0);
    }
    void shiftTo(SkScalar shift) { fShift = shift; }

    inline SkScalar alphabeticBaseline() const { return fSizes.alphabeticBaseline(); }
    inline SkScalar ideographicBaseline() const { return fSizes.ideographicBaseline(); }
    inline SkScalar baseline() const { return fSizes.baseline(); }
    inline SkScalar roundingDelta() const { return fSizes.delta(); }

    using StyleVisitor = std::function<SkScalar(SkSpan<const char> text, const SkTextStyle& style,
                                                SkScalar offsetX)>;
    void iterateThroughStylesInTextOrder(SkStyleType styleType, const StyleVisitor& visitor) const;

    using RunVisitor = std::function<bool(SkRun* run, size_t pos, size_t size, SkRect clip,
                                          SkScalar shift, bool clippingNeeded)>;
    SkScalar iterateThroughRuns(SkSpan<const char> text,
                                SkScalar offsetX,
                                bool includeEmptyText,
                                const RunVisitor& visitor) const;

    using ClustersVisitor = std::function<bool(const SkCluster* cluster)>;
    void iterateThroughClustersInGlyphsOrder(bool reverse, const ClustersVisitor& visitor) const;

    void format(SkTextAlign effectiveAlign, SkScalar maxWidth, bool last);
    void paint(SkCanvas* canvas);

    void createEllipsis(SkScalar maxWidth, const std::string& ellipsis, bool ltr);

    // For testing internal structures
    void scanStyles(SkStyleType style, const StyleVisitor& visitor);
    void scanRuns(const RunVisitor& visitor);

private:
    SkRun* shapeEllipsis(const std::string& ellipsis, SkRun* run);
    void justify(SkScalar maxWidth);

    SkRect measureTextInsideOneRun(SkSpan<const char> text,
                                   SkRun* run,
                                   size_t& pos,
                                   size_t& size,
                                   bool& clippingNeeded) const;

    SkScalar paintText(SkCanvas* canvas, SkSpan<const char> text, const SkTextStyle& style,
                       SkScalar offsetX) const;
    SkScalar paintBackground(SkCanvas* canvas, SkSpan<const char> text, const SkTextStyle& style,
                             SkScalar offsetX) const;
    SkScalar paintShadow(SkCanvas* canvas, SkSpan<const char> text, const SkTextStyle& style,
                         SkScalar offsetX) const;
    SkScalar paintDecorations(SkCanvas* canvas, SkSpan<const char> text, const SkTextStyle& style,
                              SkScalar offsetX) const;

    void computeDecorationPaint(SkPaint& paint, SkRect clip, const SkTextStyle& style,
                                SkPath& path) const;

    bool contains(const SkCluster* cluster) const {
        return cluster->text().begin() >= fText.begin() && cluster->text().end() <= fText.end();
    }

    SkSpan<const SkBlock> fBlocks;
    SkSpan<const char> fText;
    SkSpan<const SkCluster> fClusters;
    size_t fStartPos;
    size_t fEndPos;
    SkTArray<SkRun*, true> fLogical;
    SkScalar fShift;                   // Shift to left - right - center
    SkVector fAdvance;                 // Text size
    SkVector fOffset;                  // Text position
    std::unique_ptr<SkRun> fEllipsis;  // In case the line ends with the ellipsis
    SkLineMetrics fSizes;              // Line metrics as a max of all run metrics

    static SkTHashMap<SkFont, SkRun> fEllipsisCache;  // All found so far shapes of ellipsis
};
