Canvas2D extensions for Shaped Text
=============
[Shaped Text](text_shaper.md) is a proposal for exposing the Browser's text shaping engine. It takes in
a block of (annoated) text, and returns the low-level information needed to correctly measure, hit-test,
and draw the text as positioned glyphs. This processing step is needed whenever text is measured or
drawn in the browser, and this processing can be complex and time consuming. The output of this processing
is, however, simple and can be rendered quite efficiently. It is runs (tied to a specific Typeface and size)
of glyph IDs and x,y postions.

This proposal extends Canvas2D to allow it to draw those glyphs directly, and also includes utilities for
querying attributes of the glyphs (not needed for drawing, but useful for other operations).

### Principles
* Drawing postioned glyphs should be at least as flexible as the existing fillText() method.
* It is expected that drawing glyphs can be faster than fillText() -- no shaping/processing is needed.
* With the additional utilities, new effects should be easy and efficient.

## Drawing glyphs

At the heart of the proposal is a parallel to fillText/strokeText...

```js
context.fillGlyphs(glyphs, positions, Font);

context.strokeGlyphs(glyphs, positions, Font);
```

These respect all of the same settings as their 'Text' equivalents (e.g. current transform, clip, style)
with the exception of the actual text attributes:
- font
- textAlign
- textBaseline
- direction

These are ignored, because they have already been 'computed' by the Shape Text processing, and their
results are represented in the glyphs, positions, and [Font](text_shaper.md) parameters.

[Font](text_shaper.md) is far more specific in this extension that the existing context.font attribute. In
today's canvas2d, "font" holds a high-level description of the typeface(s): It is a string with the
font's name, which has to be resolved to find the actual (set of) resources. For Shaped Text, this
resolution has already occured. The glyph IDs are specific to exactly 1 resource (i.e. file/blob) and
so the Font interface contains not the name, but a handle to the actual resource.

The upside to this specificity is performance: with all "fallback" and shaping having already
occured, the draw calls can execute faster.

## Font utilities

[Shaped Text](text_shaper.md) introduced the Font interface, but for shaping, it only needed to specify
the resource (Typeface object) sizing information, and (on input) optional font-features. After shaping,
clients may want to query information about specific glyphs within that Font. Those extended methods are
presented here.

```WebIDL
interface Font {
    ...     // see [Shaped Text](text_shaper.md) for descriptive attributes

    // return array of advance widths for the specified glyphs.
    //
    sequence<float> getGlyphAdvances(sequene<unsigned short> glyphs);

    // return array of [left, top, right, bottom] coordinates for the specified glyphs,
    // 
    // If positions are provided, then the rectangles are relative to each glyph's postion.
    // If no positions are provided, then the rectangles are all relateive to (0,0).
    // Note: positions are stored as (x,y) pairs
    //
    sequence<float> getGlyphBounds(sequene<unsigned short> glyphs, sequence<float> positions?);

    // return array of Path2D objects for the specified glyphs,
    // 
    // If positions are provided, then the paths are relative to each glyph's postion.
    // If no positions are provided, then the paths are all relateive to (0,0).
    // Note: positions are stored as (x,y) pairs
    //
    // If a glyph has no visual representation (e.g. a SPACE) then its path will be null.
    // If a glyph has an image for its representation, then its path will be undefined.
    //
    sequence<Path2D> getGlyphPaths(sequene<unsigned short> glyphs, sequence<float> positions?);

    // A glyph may be represented with an image (e.g. emoji). getGlyphImage() for these glyphs
    // will return a GlyphImage object. If the glyph does not have an Image, null is returned.
    //
    GlyphImage getGlyphImage(unsigned short glyphID);
};

interface GlyphImage {
    readonly attribute ImageBitmap image;
    readonly attribute DOMMatrix transform;
};
```

[Overview document](text_overview.md)

## Contributors:
 [mikerreed](https://github.com/mikerreed),
