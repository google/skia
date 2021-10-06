// Copyright 2021 Google LLC.
#ifndef Text_DEFINED
#define Text_DEFINED
#include <string>
#include "experimental/sktext/include/Types.h"
#include "experimental/sktext/src/Line.h"
#include "experimental/sktext/src/LogicalRun.h"
#include "experimental/sktext/src/VisualRun.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "modules/skshaper/include/SkShaper.h"
#include "modules/skunicode/include/SkUnicode.h"

namespace skia {
namespace text {

class FontResolvedText;

/**
 * This class contains all the SKUnicode/ICU information.
 */
class UnicodeText {
public:
    /** Makes calls to SkShaper and collects all the shaped data.
        @param blocks         a range of FontBlock elements that keep information about
                              fonts required to shape the text.
                              It's utf16 range but internally it will have to be converted
                              to utf8 (since all shaping operations use utf8 encoding)
        @param textDirection  a starting text direction value
        @return               an object that contains the result of shaping operations
    */
    std::unique_ptr<FontResolvedText> resolveFonts(SkSpan<FontBlock> blocks);

    UnicodeText(std::unique_ptr<SkUnicode> unicode, SkSpan<uint16_t> utf16);
    UnicodeText(std::unique_ptr<SkUnicode> unicode, const SkString& utf8);
    ~UnicodeText() = default;

    bool hasProperty(TextIndex index, CodeUnitFlags flag) const {
        return (fCodeUnitProperties[index] & flag) == flag;
    }
    bool isHardLineBreak(TextIndex index) const {
        return this->hasProperty(index, CodeUnitFlags::kHardLineBreakBefore);
    }
    bool isSoftLineBreak(TextIndex index) const {
        return index != 0 && this->hasProperty(index, CodeUnitFlags::kSoftLineBreakBefore);
    }
    bool isWhitespaces(TextRange range) const;

    SkUnicode* getUnicode() const { return fUnicode.get(); }
    SkSpan<const char16_t> getText16() const { return SkSpan<const char16_t>(fText16.data(), fText16.size()); }

    template <typename Callback>
    void forEachGrapheme(TextRange textRange, Callback&& callback) {
        TextRange grapheme(textRange.fStart, textRange.fStart);
        for (size_t i = textRange.fStart; i < textRange.fEnd; ++i) {
            if (this->hasProperty(i, CodeUnitFlags::kGraphemeStart)) {
                grapheme.fEnd = i;
                if (grapheme.width() > 0) {
                    callback(grapheme);
                }
                grapheme.fStart = grapheme.fEnd;
            }  else if (this->hasProperty(i, CodeUnitFlags::kHardLineBreakBefore)) {
                grapheme.fEnd = i;
                callback(grapheme);
                // TODO: We assume here that the line break takes only one codepoint
                // Skip the next line
                grapheme.fStart = grapheme.fEnd + 1;
            }
        }
        grapheme.fEnd = textRange.fEnd;
        if (grapheme.width() > 0) {
            callback(grapheme);
        }
    }

private:
    void initialize(SkSpan<uint16_t> utf16);

    SkTArray<CodeUnitFlags, true> fCodeUnitProperties;
    std::u16string fText16;
    std::unique_ptr<SkUnicode> fUnicode;
};

class ShapedText;
/**
 * This class contains provides functionality for resolving fonts
 */
class FontResolvedText {
public:
    /** Makes calls to SkShaper and collects all the shaped data.
        @param blocks         a range of FontBlock elements that keep information about
                              fonts required to shape the text.
                              It's utf16 range but internally it will have to be converted
                              to utf8 (since all shaping operations use utf8 encoding)
        @param textDirection  a starting text direction value
        @return               an object that contains the result of shaping operations
    */
    virtual std::unique_ptr<ShapedText> shape(UnicodeText* unicodeText, TextDirection textDirection);

    FontResolvedText() = default;
    virtual ~FontResolvedText() = default;

    bool resolveChain(UnicodeText* unicodeText, TextRange textRange, const FontChain& fontChain);
    SkSpan<ResolvedFontBlock> resolvedFonts() { return SkSpan<ResolvedFontBlock>(fResolvedFonts.data(), fResolvedFonts.size()); }
private:
    friend class UnicodeText;
    SkTArray<ResolvedFontBlock, true> fResolvedFonts;
};

class WrappedText;
/**
 * This class provides all the information from SkShaper/harfbuzz in a raw format.
 * It does require a single existing font for each codepoint.
 */
 // Question: do we provide a visitor for ShapedText?
class ShapedText : public SkShaper::RunHandler {
public:
    /** Break text by lines with a given width (and possible new lines).
        @param unicodeText    a reference to UnicodeText that is used to query Unicode information
        @param width          a line width at which the text gets wrapped
        @param height         a text height, currently not supported
        @return               an object that contains the result of shaping operations (wrapping and formatting).
    */
    std::unique_ptr<WrappedText> wrap(UnicodeText* unicodeText, float width, float height);

    ShapedText()
    : fCurrentRun(nullptr)
    , fParagraphTextStart(0)
    , fRunGlyphStart(0.0f) { }

    void beginLine() override {}
    void runInfo(const RunInfo&) override {}
    void commitRunInfo() override {}
    void commitLine() override {}
    void commitRunBuffer(const RunInfo&) override {
        fCurrentRun->commit();
        fLogicalRuns.emplace_back(std::move(*fCurrentRun));
        fRunGlyphStart += fCurrentRun->width();
    }
    Buffer runBuffer(const RunInfo& info) override {
        fCurrentRun = std::make_unique<LogicalRun>(info, fParagraphTextStart, fRunGlyphStart);
        return fCurrentRun->newRunBuffer();
    }

    SkSpan<const LogicalRun> getLogicalRuns() const { return SkSpan<const LogicalRun>(fLogicalRuns.begin(), fLogicalRuns.size()); }
private:
    friend class FontResolvedText;

    void addLine(WrappedText* wrappedText, SkUnicode* unicode, Stretch& stretch, Stretch& spaces, bool hardLineBreak);

    SkTArray<int32_t> getVisualOrder(SkUnicode* unicode, RunIndex start, RunIndex end);

    // This is all the results from shaping
    SkTArray<LogicalRun, false> fLogicalRuns;

    // Temporary values
    std::unique_ptr<LogicalRun> fCurrentRun;
    TextIndex fParagraphTextStart;
    SkScalar fRunGlyphStart;
};

/**
 * This is a helper visitor class that allows a user to process the wrapped text
 * structures: lines and runs (to draw them, for instance)
 */
class Visitor {
public:
    virtual ~Visitor() = default;
    virtual void onBeginLine(size_t index, TextRange lineText, bool hardBreak, SkRect bounds) { }
    virtual void onEndLine(size_t index, TextRange lineText, GlyphRange trailingSpaces, size_t glyphCount) { }
    virtual void onGlyphRun(const SkFont& font,
                            DirTextRange dirTextRange,        // Currently we make sure that the run edges are the grapheme cluster edges
                            SkRect bounds,                    // bounds contains the physical boundaries of the run
                            TextIndex trailingSpaces,         // Depending of TextDirection it goes right to the end (LTR) or left to the start (RTL)
                            size_t glyphCount,                // Just the number of glyphs
                            const uint16_t glyphs[],          // GlyphIDs from the font
                            const SkPoint positions[],        // Positions relative to the line
                            const TextIndex clusters[])       // Text indices inside the entire text
    { }
    virtual void onPlaceholder(TextRange, const SkRect& bounds) { }
};

class DrawableText;
class SelectableText;
/**
 * This class provides all the information about wrapped/formatted text.
 */
class WrappedText {
public:
    /** Builds a list of SkTextBlobs to draw on a canvas.
        @param positionType   specifies a text adjustment granularity (grapheme cluster, grapheme, glypheme, glyph)
        @param blocks         a range of text indices that cause an additional run breaking to be used for styling
        @return               an object that contains a list of SkTextBlobs to draw on a canvas
    */
    template <class Drawable>
    std::unique_ptr<Drawable> prepareToDraw(UnicodeText* unicodeText, PositionType positionType, SkSpan<TextIndex> blocks) const {
        auto drawableText = std::make_unique<Drawable>();
        this->visit(unicodeText, drawableText.get(), positionType, blocks);
        return drawableText;
    }
    /** Aggregates all the data to navigate the text (move up, down, left, right),
        select some text near the cursor point, adjust all text position to word,
        grapheme cluster and such.
        @return               an object that contains all the data for navigation
    */
    std::unique_ptr<SelectableText> prepareToEdit(UnicodeText* unicodeText) const;
    /** Formats a text line by line.
        @param textAlign     specifies a text placement on the line:
                             left, right, center and justified (last one currently not supported)
        @param textDirection specifies a text direction that also used in formatting
    */
    void format(TextAlign textAlign, TextDirection textDirection);

    SkSize actualSize() const { return fActualSize; }
    size_t countLines() const { return fVisualLines.size(); }

    /** Walks though the data structures and makes certain callbacks on visitor so the visitor can collect all the information.
        @param visitor      a reference to Visitor object
    */
    void visit(Visitor* visitor) const;
    /** Walks though the data structures and makes certain callbacks on visitor so the visitor can collect all the information.
        @param unicodeText  a reference to UnicodeText object
        @param visitor      a reference to Visitor object
        @param positionType specifies a text adjustment granularity (grapheme cluster, grapheme, glypheme, glyph)
                            to map text blocks to glyph ranges.
        @param chunks       a range of widths that cause an additional run breaking to be used for styling
    */
    void visit(UnicodeText* unicodeText, Visitor* visitor, PositionType positionType, SkSpan<size_t> chunks) const;

    static std::vector<TextIndex> chunksToBlocks(SkSpan<size_t> chunks);
    static SkSpan<TextIndex> limitBlocks(TextRange textRange, SkSpan<TextIndex> blocks);

private:
    friend class ShapedText;
    WrappedText() : fActualSize(SkSize::MakeEmpty()), fAligned(TextAlign::kNothing) { }
    GlyphRange textToGlyphs(UnicodeText* unicodeText, PositionType positionType, RunIndex runIndex, DirTextRange dirTextRange) const;
    SkTArray<VisualRun, true> fVisualRuns;    // Broken by lines
    SkTArray<VisualLine, false> fVisualLines;
    SkSize fActualSize;
    TextAlign fAligned;
};

/** This class contains all the data that allows easily paint the text on canvas.
    Strictly speaking, it is not an important part of SkText API but
    it presents a good example of SkText usages and simplifies testing.
*/
class DrawableText : public Visitor {
public:
    DrawableText() = default;

    void onGlyphRun(const SkFont& font,
                    DirTextRange dirTextRange,
                    SkRect bounds,
                    TextIndex trailingSpaces,
                    size_t glyphCount,
                    const uint16_t glyphs[],
                    const SkPoint positions[],
                    const TextIndex clusters[]) override {
        SkTextBlobBuilder builder;
        const auto& blobBuffer = builder.allocRunPos(font, SkToInt(glyphCount));
        sk_careful_memcpy(blobBuffer.glyphs, glyphs, glyphCount * sizeof(uint16_t));
        sk_careful_memcpy(blobBuffer.points(), positions, glyphCount * sizeof(SkPoint));
        fTextBlobs.emplace_back(builder.make());
    }
    std::vector<sk_sp<SkTextBlob>>& getTextBlobs() { return fTextBlobs; }
private:
    std::vector<sk_sp<SkTextBlob>> fTextBlobs;
};

struct Position {
    Position(PositionType positionType, size_t lineIndex, GlyphRange glyphRange, TextRange textRange, SkRect rect)
        : fPositionType(positionType)
        , fLineIndex(lineIndex)
        , fGlyphRange(glyphRange)
        , fTextRange(textRange)
        , fBoundaries(rect) { }

    Position(PositionType positionType)
        : Position(positionType, EMPTY_INDEX, EMPTY_RANGE, EMPTY_RANGE, SkRect::MakeEmpty()) { }

    PositionType fPositionType;
    size_t fLineIndex;
    GlyphRange fGlyphRange;
    TextRange fTextRange;
    SkRect fBoundaries;
};

struct BoxLine {
    BoxLine(size_t index, TextRange text, bool hardBreak, SkRect bounds)
        : fTextRange(text), fIndex(index), fIsHardBreak(hardBreak), fBounds(bounds) { }
    SkTArray<SkRect, true> fBoxGlyphs;
    SkTArray<TextIndex, true> fTextByGlyph; // by glyph cluster
    GlyphIndex fTextEnd;
    GlyphIndex fTrailingSpacesEnd;
    TextRange fTextRange;
    size_t fIndex;
    bool fIsHardBreak;
    SkRect fBounds;
};

/** This class contains all the data that allows all navigation operations on the text:
    move up/down/left/right, select some units of text and such.
*/
class SelectableText : public Visitor {
public:
    SelectableText() = default;

    /** Find the drawable unit (specified by positionType) closest to the screen point
        @param positionType   specifies a text adjustment granularity (grapheme cluster, grapheme, glypheme, glyph)
        @param point          a physical coordinates on a screen to find the closest glyph
        @return               a position object that contains all required information
    */
    Position adjustedPosition(PositionType positionType, SkPoint point) const;

    Position previousPosition(Position current) const;
    Position nextPosition(Position current) const;
    Position upPosition(Position current) const;
    Position downPosition(Position current) const;
    Position firstPosition(PositionType positionType) const;
    Position lastPosition(PositionType positionType) const;
    Position firstInLinePosition(PositionType positionType, LineIndex lineIndex) const;
    Position lastInLinePosition(PositionType positionType, LineIndex lineIndex) const;

    bool isFirstOnTheLine(Position element) const {
        return (element.fGlyphRange.fStart == 0);
    }
    bool isLastOnTheLine(Position element) const {
        return (element.fGlyphRange.fEnd == fBoxLines.back().fBoxGlyphs.size());
    }

    size_t countLines() const { return fBoxLines.size(); }
    BoxLine getLine(size_t lineIndex) const {
        SkASSERT(lineIndex < fBoxLines.size());
        return fBoxLines[lineIndex];
    }

    bool hasProperty(TextIndex index, GlyphUnitFlags flag) const {
        return (fGlyphUnitProperties[index] & flag) == flag;
    }

    void onBeginLine(size_t index, TextRange lineText, bool hardBreak, SkRect bounds) override;
    void onEndLine(size_t index, TextRange lineText, GlyphRange trailingSpaces, size_t glyphCount) override;

    void onGlyphRun(const SkFont& font,
                    DirTextRange dirTextRange,
                    SkRect bounds,
                    TextIndex trailingSpaces,
                    size_t glyphCount,
                    const uint16_t glyphs[],
                    const SkPoint positions[],
                    const TextIndex clusters[]) override;

private:
    friend class WrappedText;

    Position findPosition(PositionType positionType, const BoxLine& line, SkScalar x) const;
    // Just in theory a random glyph range can be represented by multiple text ranges (because of LTR/RTL)
    // Currently we only support this method for a glyph, grapheme or grapheme cluster
    // So it's guaranteed to be one text range
    TextRange glyphsToText(Position position) const;

    SkTArray<BoxLine, true> fBoxLines;
    SkTArray<GlyphUnitFlags, true> fGlyphUnitProperties;
    SkSize fActualSize;
};

}  // namespace text
}  // namespace skia

#endif  // Text_DEFINED
