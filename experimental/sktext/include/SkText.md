---

title: "SkText public API"
linkTitle: "SkText public API"

---

The main idea behind SkText is to design a flexible API that allows a user from different platforms to shape, manipulate and draw text.

As text formatting works in stages, SkText presents a set of text objects that keep data from each stage. That staging approach allows a user to stop at any stage and drop all the data from the previous stages.
Currently, we support the following stages:

* <u>Parsing the text</u> to extract all the unicode information that may be needed later: graphemes, words, hard/soft line breaks, whitespaces, bidi regions and so on.
Class <b>UnicodeText</b> contains the initial text in utf16 format, and the mapping of unicode information to codepoints in a form of enum bit flags.
It also has a method <b>resolveFonts</b> that creates a FontResolvedText object.


* <u>Resolving fonts</u> to break the text into blocks that can be shaped with one single font (selected out of a given list of fonts).
Class <b>FontResolvedText</b> contains the results in a form of a list of text blocks with a single typeface for each.
It also has a method <b>shape</b> that creates a ShapedText object.


* <u>Shaping the text</u> into a single line of glyphs according to all given formatting information (fonts, text direction, scripts, languages and so on).
Class <b>ShapedText</b> contains the results in the form of a list of shaped blocks with all the information that comes from shaping: glyph ids, positioning, and mapping to the initial text and so on.
It also has a method <b>wrap</b> that creates a WrappedText object.


* <u>Wrapping the text</u> into a list of lines (by a given width) and formatting it on the lines (left, right, center alignment or justification).
Class <b>WrappedText</b> contains the shaped results from the previous stage only broken by lines and repositioned by lines.
It also has a method <b>prepareToDraw</b> that creates a DrawableText object, and a method <b>prepareToNavigate</b> that creates a SelectableText object.


* <u>Drawing the text</u> into a canvas. This is more of an example of how to draw the text because it only covers a rather simple case of drawing (limited decorating supported). Based on this example a user can create a custom drawing class as complex as needed.
Class <b>DrawableText</b> contains a list of SkTextBlob objects ready to be drawn on a canvas.


* <u>Navigating the text</u> by grapheme cluster (later, grapheme, glyph cluster or glyph). It has all the functionality for text editing.
Class <b>SelectableText</b> contains all the methods for that: hit test, move position (left, right, up, down, to the beginning of the text or the line, to the end of the text or the line), select an arbitrary text (aligned to a grapheme cluster) and so on.

All the objects described above can exist independently of each other, so a user can decide which ones to keep.

Let’s consider few scenarios (flows):

1. A user only needs to get the unicode information.
<br>Strictly speaking, it’s not a text shaping operation but UnicodeText would allow it.
1. A user needs to draw the text.
<br>That requires performing the first 4 stages, but a user only needs to hold on to the DrawableText object afterwards (removing UnicodeText, ShapedText and WrappedText objects).
1. A user needs to draw and edit the text.
<br>That requires performing all the 5 stages. A user will have to hold on to DrawableText and SelectableText (removing the first 3 objects).
1. A user only needs to be wrapped text, implementing a custom drawing procedure.
<br>That requires performing the first 3 stages. A user will have to hold on to WrappedText (removing the first 2 objects).

At the moment there is no support for updating the text (which theoretically could be a less expensive operation). Any changes in the initial text will require the full set of operations.
