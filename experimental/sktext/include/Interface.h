// Copyright 2021 Google LLC.
#ifndef Interface_DEFINED
#define Interface_DEFINED
#include <string>
#include "experimental/sktext/include/Types.h"
#include "experimental/sktext/src/Line.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkFontMgr.h"
#include "include/core/SkFontStyle.h"
#include "include/core/SkPaint.h"
#include "include/core/SkSize.h"
#include "include/core/SkString.h"
#include "include/core/SkTextBlob.h"
#include "modules/skshaper/include/SkShaper.h"
#include "modules/skunicode/include/SkUnicode.h"

using namespace skia::text;
namespace skia {
namespace API {
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
    std::unique_ptr<ShapedText> shape(SkSpan<FontBlock> blocks, TextDirection textDirection);

    UnicodeText(std::unique_ptr<SkUnicode> unicode, SkSpan<uint16_t> utf16);
    UnicodeText(std::unique_ptr<SkUnicode> unicode, const SkString& utf8);

    bool hasProperty(TextIndex index, CodeUnitFlags flag) const
    bool isHardLineBreak(TextIndex index) const ;
    bool isSoftLineBreak(TextIndex index) const;
    bool isWhitespaces(TextRange range) const;

    SkUnicode* getUnicode() const;
    SkSpan<const char16_t> getText16() const;
};

class WrappedText;
/**
 * This class provides all the information from SkShaper/harfbuzz in a raw format.
 * It does require a single existing font for each codepoint.
 */
 // Question: do we provide a visitor for ShapedText?
class ShapedText {
public:
    /** Break text by lines with a given width (and possible new lines).
        @param unicodeText    a reference to UnicodeText that is used to query Unicode information
        @param width          a line width at which the text gets wrapped
        @param height         a text height, currently not supported
        @return               an object that contains the result of shaping operations (wrapping and formatting).
    */
    std::unique_ptr<WrappedText> wrap(UnicodeText* unicodeText, float width, float height);
    SkSpan<const LogicalRun> getLogicalRuns() const;
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
                            TextRange textRange,        // Currently we make sure that the run edges are the grapheme cluster edges
                            SkRect bounds,              // bounds contains the physical boundaries of the run
                            int trailingSpaces,         // Depending of TextDirection it goes right to the end (LTR) or left to the start (RTL)
                            int glyphCount,             // Just the number of glyphs
                            const uint16_t glyphs[],
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
    std::unique_ptr<DrawableText> prepareToDraw(UnicodeText* unicodeText, PositionType positionType, SkSpan<TextIndex> blocks) const;
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
    /** Breaks the text runs into smaller runs by given list of chunks to be used for styling.
        @param unicodeText    a reference to UnicodeText object
        @param chunks         a range of text indices that cause an additional run breaking to be used for styling
    */
    void decorate(UnicodeText* unicodeText, SkSpan<TextIndex> chunks);

    /** Walks though the data structures and makes certain callbacks on visitor so the visitor can collect all the information.
        @param visitor      a reference to Visitor object
    */
    void visit(Visitor* visitor) const;
    /** Walks though the data structures and makes certain callbacks on visitor so the visitor can collect all the information.
        @param unicodeText  a reference to UnicodeText object
        @param visitor      a reference to Visitor object
        @param positionType specifies a text adjustment granularity (grapheme cluster, grapheme, glypheme, glyph)
                            to map text blocks to glyph ranges.
        @param blocks       a range of text indices that cause an additional run breaking to be used for styling
    */
    void visit(UnicodeText* unicodeText, Visitor* visitor, PositionType positionType, SkSpan<TextIndex> blocks) const;
};

/** This class contains all the data that allows easily paint the text on canvas.
    Strictly speaking, it is not an important part of SkText API but
    it presents a good example of SkText usages and simplifies testing.
*/
class DrawableText : public Visitor {
public:
    std::vector<sk_sp<SkTextBlob>>& getTextBlobs();
};

struct Position {
    Position(PositionType positionType, size_t lineIndex, GlyphRange glyphRange, TextRange textRange, SkRect rect);
    Position(PositionType positionType);
    PositionType fPositionType;
    size_t fLineIndex;
    GlyphRange fGlyphRange;
    TextRange fTextRange;
    SkRect fBoundaries;
};
struct BoxLine {
    BoxLine(size_t index, TextRange text, bool hardBreak, SkRect bounds);
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
    Position firstPosition(PositionType positionType) const;
    Position lastPosition(PositionType positionType) const;
    Position firstInLinePosition(PositionType positionType, LineIndex lineIndex) const;
    Position lastInLinePosition(PositionType positionType, LineIndex lineIndex) const;

    bool isFirstOnTheLine(Position element) const;
    bool isLastOnTheLine(Position element) const;

    size_t countLines() const;
    BoxLine getLine(size_t lineIndex) const;
};
}  // namespace text
}  // namespace skia

#endif  // Processor_DEFINED
