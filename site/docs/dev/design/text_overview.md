Text API Overview
=============
*2D Graphics* is a broad area. It consists of a host of related, but different ideas and primitives
- rectangles, lines, paths
- images (often scaled, transformed, filtered)
- text (also scalable and transformable)
- ...

Native (desktop, mobile) frameworks combine these primitives with common rendering constructs
- transforms and clipping
- colors, gradients, patterns
- blending and colorspaces

One primitive in particular stands out: **Text**. It can be drawn with the same common constructs as rects and paths, but it has another component unrelated to actual drawing, a 'processing' step that is surprisingly complex and expensive: **Shaping**. The native frameworks recognized this, and offer a separate set of APIs just around this:
- CoreGraphics --> CoreText
- Direct2D/X --> DirectWrite
- Skia --> SkShaper

This proposal embraces this pattern, and aims to expose this functionality to the Web.


## Background

Part of this complexity lies in language and alphabets themselves, and part is due to how our system of fonts and font formats have evolved. This document does not claim to detail either of these, but will briefly touch on each to motivate the proposed API suite.

## Language and Alphabets

### "The office 建造 is in München."

This simple sentence is illustrative of some of the richness (i.e. complexity) of displaying text. Some examples
- The 'ffi" in office (or possibly just the "fi") **could** be drawn with a specialized form called a [ligature](https://en.wikipedia.org/wiki/Ligature_(writing)).
- The "ü" could have been drawn as a single shape, or it could have been drawn by separately drawing "u" followed by drawing the umlaut (¨) on top.
- The third word may not be reprsentable in the same font as the other words, so a different font may have been needed.

This example in no way attempts to be complete or exhaustive, but it suggests the processing needed to go from just the 'letters' in a sentence, to the actual 'shapes' that must be found or composed from various fonts to finally be able to draw it.

## Fonts and Styles

This [site](https://www.w3schools.com/css/css_font.asp) has an excellent overview of some of the richess of specifying a "font" in CSS. While different native platforms have some variance, most support the following *mapping* from a description of a font, to the actual resource / file / blob containing the drawn shapes.

- family-name : "Helvetica", "Times New Roman", "Lucida Typewriter Sans", ...
- font-style : bold, italic
- font-weight : 100 .. 900
- font-stretch : ultra-condensed .. ultra-expanded
- font-variation-setting : 'wght' 850 or 'wdth' 25 or ...

All of these attributes (and more) go into the browser's search for the best matching font resource/file/blob. Let's call the resulting 'resource' a **Typeface**. Note: If the font supports variation-settings, our definition will also include the specific settings (e.g. a font resource + variation settings).

## Typefaces and Glyphs

We define a Typeface to be a coherent collection of shapes in a given typograph style (including any variation settings). Typically a typeface is stored in a single file or blob (e.g. an OpenType file).

Along with a single Typeface we define the individual *shapes* or images in the typeface as **Glyphs**. Glyphs represent the smallest independent drawing element within a typeface. By convention, glyphs are identified by an *index*, ranging from 0 to however many are in a particular typeface.

Determining what glyphs, in what order and positions, are needed to represent a set of letters, is the heart of **Shaping**, and it is this process, and resulting typefaces + positioned glyphs, that we propose exposing to Web Apps.

## Summary

We posit that drawing *internationally correct* Text is critical to most Web Apps, and that it is both complex to get correct, and can be computationally expensive. We propose exposing this processing to apps, providing them with results that can be efficiently drawn / animated.

The core [Shaping APIs](text_shaper.md) are detailed here.

Assocated [Canvas2D extensions](text_c2d.md) are detailed here.

Note: it is an explicit goal to **not** tie Shaping or its results to Canvas2D. We envision multiple scenarios where a framework or app will want to shape text, but not need a Canvas2D context .
- drawn using WebGL or WebGPU
- drawn using DOM (but utilizing line-breaking and metrics results)
- drawn using a bespoke (i.e. wasm) renderer

We are also proposing a lower level interface, one that just addresses exposing [unicode properties](uni_characterize.md).


## Contributors:
 [mikerreed](https://github.com/mikerreed),
