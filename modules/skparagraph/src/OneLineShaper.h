// Copyright 2019 Google LLC.
#ifndef LineBreaker_DEFINED
#define LineBreaker_DEFINED

#include <functional>  // std::function
#include <queue>
#include "modules/skparagraph/include/TextStyle.h"
#include "modules/skparagraph/src/ParagraphImpl.h"
#include "modules/skparagraph/src/Run.h"
#include "src/core/SkSpan.h"

namespace skia {
namespace textlayout {

class ParagraphImpl;
class OneLineShaper : public SkShaper::RunHandler {
public:
    explicit OneLineShaper(ParagraphImpl* paragraph)
        : fParagraph(paragraph)
        , fHeight(0.0f)
        , fAdvance(SkPoint::Make(0.0f, 0.0f))
        , fUnresolvedGlyphs(0)
        , fUniqueRunId(paragraph->fRuns.size()){ }

    bool shape();

    size_t unresolvedGlyphs() { return fUnresolvedGlyphs; }

private:

    class RunBlock : public ShapedSpan {
     public:
        RunBlock() : fRun(nullptr) { }

        // First unresolved block
        explicit RunBlock(TextRange text)
            : ShapedSpan(text, GlyphRange(0, 0))
            , fRun(nullptr) { }

        RunBlock(std::shared_ptr<Run> run, TextRange text, GlyphRange glyphs)
            : ShapedSpan(text, glyphs)
            , fRun(std::move(run)) { }

        // Entire run comes as one block fully resolved
        explicit RunBlock(std::shared_ptr<Run> run)
            : ShapedSpan(*run, GlyphRange(0, run->size()))
            , fRun(std::move(run)) { }

        bool isFullyResolved() { return fRun != nullptr && fGlyphs.width() == fRun->size(); }

        std::shared_ptr<Run> getRun() const { return fRun; }
        void setRun(std::shared_ptr<Run> run) { fRun = run; }

     private:
        std::shared_ptr<Run> fRun;
    };

    using ShapeVisitor =
            std::function<SkScalar(TextRange textRange, SkSpan<Block>, SkScalar&, TextIndex, uint8_t)>;
    bool iterateThroughShapingRegions(const ShapeVisitor& shape);

    using ShapeSingleFontVisitor = std::function<void(Block, SkTArray<SkShaper::Feature>)>;
    void iterateThroughFontStyles(TextRange textRange, SkSpan<Block> styleSpan, const ShapeSingleFontVisitor& visitor);

    enum Resolved {
        Nothing,
        Something,
        Everything
    };

    using TypefaceVisitor = std::function<Resolved(sk_sp<SkTypeface> typeface)>;
    void matchResolvedFonts(const TextStyle& textStyle, const TypefaceVisitor& visitor);
#ifdef SK_DEBUG
    void printState();
#endif
    void dropUnresolved();
    void finish(TextRange text, SkScalar height, SkScalar& advanceX);

    void beginLine() override {}
    void runInfo(const RunInfo&) override {}
    void commitRunInfo() override {}
    void commitLine() override {}

    Buffer runBuffer(const RunInfo& info) override {
        fCurrentRun = std::make_shared<Run>(fParagraph,
                                           info,
                                           fCurrentText.start,
                                           fHeight,
                                           ++fUniqueRunId,
                                           fAdvance.fX);
        return fCurrentRun->newRunBuffer();
    }

    void commitRunBuffer(const RunInfo&) override;

    void extendTextToGraphemeCluster(ShapedSpan& span);

    ClusterIndex clusterIndex(GlyphIndex glyph) {
        return fCurrentText.start +
              (fCurrentRun->leftToRight()
              ?  fCurrentRun->fClusterIndexes[glyph]
              :  glyph > 0
              ? fCurrentRun->fClusterIndexes[glyph - 1]
              : fCurrentRun->start
              );
    }
    void addFullyResolved();
    void addUnresolvedWithRun(GlyphRange glyphRange);
    void sortOutGlyphs(std::function<void(GlyphRange)>&& sortOutUnresolvedBLock);
    ClusterRange getClusterRange(GlyphRange glyphRange);
    void increment(TextIndex& index);
    void fillGaps(size_t);

    ParagraphImpl* fParagraph;
    TextRange fCurrentText;
    SkScalar fHeight;
    SkVector fAdvance;
    size_t fUnresolvedGlyphs;
    size_t fUniqueRunId;

    // TODO: Something that is not thead-safe since we don't need it
    std::shared_ptr<Run> fCurrentRun;
    std::queue<RunBlock> fUnresolvedBlocks;
    std::vector<RunBlock> fResolvedBlocks;

    // Keeping all resolved typefaces
    struct FontKey {

        FontKey() {}

        FontKey(SkUnichar unicode, SkFontStyle fontStyle, SkString locale)
            : fUnicode(unicode), fFontStyle(fontStyle), fLocale(locale) { }
        SkUnichar fUnicode;
        SkFontStyle fFontStyle;
        SkString fLocale;

        bool operator==(const FontKey& other) const;

        struct Hasher {
            size_t operator()(const FontKey& key) const;
        };
    };
    SkTHashMap<FontKey, sk_sp<SkTypeface>, FontKey::Hasher> fFallbackFonts;
};

}
}
#endif
