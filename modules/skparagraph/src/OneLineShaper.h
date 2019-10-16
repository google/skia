// Copyright 2019 Google LLC.
#ifndef LineBreaker_DEFINED
#define LineBreaker_DEFINED

#include <functional>  // std::function
#include <queue>
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/Run.h"
#include "src/core/SkSpan.h"

namespace skia {
namespace textlayout {

typedef size_t GlyphIndex;
typedef SkRange<GlyphIndex> GlyphRange;

class ParagraphImpl;
class OneLineShaper : public SkShaper::RunHandler {
public:
    explicit OneLineShaper(ParagraphImpl* paragraph)
        : fParagraph(paragraph) { }

    bool shape();

private:

    struct RunBlock {
    RunBlock() { fRun = nullptr; }

    // First unresolved block
    RunBlock(TextRange text) {
        fRun = nullptr;
        fScore = 0;
        fText = text;
    }

    // Resolved block
    RunBlock(Run* run, TextRange text, GlyphRange glyphs) {
        fRun = run;
        fText = text;
        fGlyphs = glyphs;
        fScore = glyphs.width();
    }

    // Unresolved block
    RunBlock(Run* run, TextRange text) {
        fRun = run;
        fScore = 0;
        fText = text;
    }

    // Entire run comes as one block fully resolved
    RunBlock(Run* run) {
        fRun = run;
        fGlyphs = GlyphRange(0, run->size());
        fScore = run->size();
        fText = run->fTextRange;
    }

    Run* fRun;
    TextRange fText;
    GlyphRange fGlyphs;
    size_t     fScore;
    bool isFullyResolved() { return fRun != nullptr && fScore == fRun->size(); }
};

    using ShapeVisitor =
            std::function<SkScalar(SkSpan<const char>, SkSpan<Block>, SkScalar&, TextIndex)>;
    bool iterateThroughShapingRegions(ShapeVisitor shape);

    using ShapeSingleFontVisitor = std::function<void(Block)>;
    void iterateThroughFontStyles(SkSpan<Block> styleSpan, ShapeSingleFontVisitor visitor);

    using TypefaceVisitor = std::function<bool(sk_sp<SkTypeface> typeface)>;
    void matchResolvedFonts(const TextStyle& textStyle, SkUnichar unicode, TypefaceVisitor visitor);

    void printState();
    TextRange topUnresolved();
    void dropUnresolved();
    void finish(TextRange text, size_t firstChar, SkScalar height, SkScalar& advanceX);

    void beginLine() override {}
    void runInfo(const RunInfo&) override {}
    void commitRunInfo() override {}
    void commitLine() override {}

    Buffer runBuffer(const RunInfo& info) override {
        fCurrentRun = new Run(fParagraph,
                               info,
                               fTextStart,
                               fHeight,
                               fParagraph->fRuns.count(),
                               fAdvance.fX);
        return fCurrentRun->newRunBuffer();
    }

    void commitRunBuffer(const RunInfo&) override;

    TextRange clusteredText(GlyphRange glyphs);
    void addResolved(GlyphRange glyphRange);
    bool addUnresolved(GlyphRange glyphRange);
    bool addUnresolvedWithRun(GlyphRange glyphRange);
    void sortOutGlyphs(std::function<void(GlyphRange)>&& sortOutUnresolvedBLock);
    ClusterRange normalizeTextRange(GlyphRange glyphRange);
    void increment(TextIndex& index);

    ParagraphImpl* fParagraph;
    TextIndex fTextStart;
    TextRange fTextRange;
    SkScalar fHeight;
    SkVector fAdvance;

    Run* fCurrentRun;
    SkTArray<const Run*> fRuns;
    std::queue<RunBlock> fUnresolvedBlocks;
    std::vector<RunBlock> fResolvedBlocks;
};

}
}
#endif