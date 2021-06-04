Shaped Text
=============
A series of object models for describing a low-level builder for multi-line formatted text, and the resulting objects that expose the results of shaping that text. These are done outside of DOM Text nodes, and outside of any particular rendering model (e.g. canvas2d or webgl).

A related explainer focuses on suggested [extensions to canvas2d](text_c2d.md) to allow it to efficiently render the shaped results, and to offer helper objects for inspecting useful properties from a typeface.

[Overview document](text_overview.md)

## Target audience

We want to target web apps that have already chosen to render their content either in canvas2d,
or webgl, or in some other fashion, but still want access to the powerful international text shaping
and layout services inherent in the browser. In the case of canvas2d, which already has some facilities
for text, we want to address the missing services and low-level results needed for creating interactive
text editing or high-perferformance rendering and animations.

Rather than 'extend' the existing canvas2d fillText() method, we propose an explicit 2-step model:
process the 'rich' text input into shaped results, and then expose those results to the client, allowing
them to draw or edit or consume the results as they choose.

JavaScript frameworks are another target audience. This proposal is heavily influenced by successful
APIs on native platforms (desktop and mobile) and seeks to deliver similar control and performance.
Thus it may be quite natural that sophiticated frameworks build upon these interfaces, providing more
'friendly', constrained versions of the features. This is expected, since multiple 'high level' models
for text are valid, each with its own opinions and tradeoffs. The goal of this API is to expose the
core services and results, and leave the opinionated layers to the JavaScript community.

### Principles
* An imperative JavaScript-friendly text representation.
* Restrict input to only what is needed for shaping and metrics is provided.
* Decorations (i.e. colors, underlines, shadows, effects) are explicitly not specified, as those can
vary widely with rendering technologies (and client imagination).

## Sequence of calls

For maximum re-use and efficiency, the process of going from rich-text description to final shaped
and formated results is broken into stages. Each 'stage' carries out specific processing, and in-turn
becomes a factory to return an instances of the next stage.

`TextBuilder`, `ShapedText` and `FormattedText` objects are used in sequence:

```js
const builder = new ParagraphBuilder(font-fallback-chain);
const shaped = builder.shape(DOMString text, sequence<TextBlock> blocks);
const formatted = shaped.format(double width, double height, alignment);
```

A Block is a descriptor for a run of text. Currently there are two specializations, but others may be
added without breaking the design.

```WebIDL
interface TextBlock {
    unsigned long length;  // number of codepoints in this block
};

interface InFont {
    attribute sequence<Typeface> typefaces; // for preferred fallback faces
    attribute double size;
    attribute double scaleX?;   // 1.0 if not specified
    attribute double skewX?:    // 0.0 if not specified (for oblique)

    attribute sequence<FontFeature> features?;
    // additional attributes for letter spacing, etc.
};

interface FontBlock : TextBlock {
    attribute InFont font;
};

interface PlaceholderBlock : TextBlock {
    attribute double width;
    attribute double height;
    attribute double offsetFromBaseline;
};

interface ShapedTextBuilder {
    constructor(TextDirection,          // default direction (e.g. R2L, L2R)
                sequence<Typeface>?,    // optional shared fallback sequence (after TextBlock's)
                ...);

    ShapedText shape(DOMString text, sequence<TextBlock>);
};
```

Here is a simple example, specifying 3 blocks for the text.

```js
const fontA = new Font({family-name: "Helvetica", size: 14});
const fontB = new Font({family-name: "Times", size: 18});
const blocks = [
  { length: 6, font: fontA },
  { length: 5, font: fontB },
  { length: 6, font: fontA },
];

const shaped = builder.shape("Hello text world.", blocks);

// now we can format the shaped text to get access to glyphs and positions.

const formatted = shaped.format({width: 50, alignment: CENTER});
```

This is explicitly intended to be efficient, both for the browser to digest, and for the client to
be able to reuse compound objects as they choose (i.e. reusing fontA in this example).

If there is a mismatch between the length of the text string, and the sum of the blocks' lengths,
then an exception is raised.

## Access the results of shaping and formatting
FormattedText has methods and the raw data results:

```WebIDL
typedef unsigned long TextIndex;

interface TextPosition {
    readonly attribute TextIndex textIndex;
    readonly attribute unsigned long lineIndex;
    readonly attribute unsigned long runIndex;
    readonly attribute unsigned long glyphIndex;
};

interface FormattedText {
    // Interaction methods

    // Given a valid index into the text, adjust it for proper grapheme
    // boundaries, and return the TextPosition.
    TextPosition indexToPosition(TextIndex index);

    // Given an x,y position, return the TextPosition
    // (adjusted for proper grapheme boundaries).
    TextPosition hitTextToPosition(double x, double y);

    // Given two logical text indices (e.g. the start and end of a selection range),
    // return the corresponding visual set of ranges (e.g. for highlighting).
    sequence<TextPosition> indicesToVisualSelection(TextIndex t0, TextIndex t1);

    // Raw data

    readonly attribute Rect bounds;

    readonly attribute sequence<TextLine> lines;
};
```

The sequence of TextLines is really and array of arrays: each line containing an
array of Runs (either Glyphs or Placeholders for now).

```WebIDL
// Shared by all output runs, specifying the range of code-points that produced
// this run.
interface TextRun {
    readonly attribute TextIndex startIndex;
    readonly attribute TextIndex endIndex;
};

interface OutFont {
    attribute Typeface typeface;
    attribute double size;
    attribute double scaleX?;   // 1.0 if not specified
    attribute double skewX?:    // 0.0 if not specified (for oblique)
};

// Corresponds to a FontBlock specified during shaping.
interface GlyphRun : TextRun {
    readonly attribute OutFont font;

    readonly attribute sequence<unsigned short> glyphs;     // N glyphs
    readonly attribute sequence<float> positions;           // N+1 x,y pairs
    readonly attribute sequence<TextIndex> indices;         // N+1 indices
};

// Corresponds to a PlaceholderBlock specified during shaping.
interface PlaceholderRun : TextRun {
    readonly attribute Rect bounds;
};

interface TextLine {
    readonly attribute TextIndex startIndex;
    readonly attribute TextIndex endIndex;

    readonly attribute double top;
    readonly attribute double bottom;
    readonly attribute double baselineY;

    readonly attribute sequence<TextRun> runs;
};
```

With these data results (specifically glyphs and positions for specific Typeface objects)
callers will have all they need to draw the results in any fasion they wish. The corresponding
start/end text indices allows them to map each run back to the original text.

This last point is fundamental to the design. It is recognized that a client creating richly
annoated text will associate both shaping (e.g. Font) information, as well as arbitrary decoration
and other annotations with each block of text. Returning in each Run the corresponding text range
allows the client to "look-up" all of their bespoke additional information for that run (e.g.
colors, shadows, underlines, placeholders, etc.). This frees the Browser from having to support
or even understand the union of all possible decorations (obviously impossible).


## Alternatives and Prior Art

This model is designed to be low-level, to appeal to performance sensitive applications
(e.g. animations) and sophisticated text (editors). It is also intended to feel 'natural' to a developer
coming from a native app environment (desktop or mobile).

We recognized that many (more casual) users may also want access to some of these services. That is
appropriate, but we posit that with the right primitives and data exposed, such higher-level models
can be constructed by the JavaScript community itself, either as formal Frameworks or as refined
sample / example code.

One excelent example of a higher-level data model is [Formatted Text](https://github.com/WICG/canvas-formatted-text/blob/main/explainer-datamodel.md) and we hope to explore ways to layer these
two proposals, allowing high-level clients to utilize their data model, but still have the option
to access our lower level accessors (as they wish).

## Rendering in Canvas2D
The [next explainer](text_c2d.md) describes how to take these results and render them
into an (extended) Canvas context.

## Contributors:
 [mikerreed](https://github.com/mikerreed),
