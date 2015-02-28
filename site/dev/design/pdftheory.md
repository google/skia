PDF Theory of Operation
=======================

<!--
PRE-GIT DOCUMENT VERSION HISTORY
    2012-06-25 Steve VanDeBogart
               * Original version
    2015-01-14 Hal Canary.
               * Add section "Using the PDF backend"
               * Markdown formatting
-->


To make use of Skia's PDF backend, see
[Using Skia's PDF Backend](../../user/sample/pdf).

Internally, Skia uses SkPDFDocument and SkPDFDevice to represent PDF
documents and pages.  This document describes how the backend
operates, but **these interfaces are not part of the public API and
are subject to perpetual change.**

* * *

### Contents ###

*   [Typical usage of the PDF backend](#Typical_usage_of_the_PDF_backend)
*   [PDF Objects and Document Structure](#PDF_Objects_and_Document_Structure)
*   [PDF drawing](#PDF_drawing)
*   [Interned objects](#Interned_objects)
*   [Graphic States](#Graphic_States)
*   [Clip and Transform](#Clip_and_Transform)
*   [Generating a content stream](#Generating_a_content_stream)
*   [Margins and content area](#Margins_and_content_area)
*   [Drawing details](#Drawing_details)
    +   [Layers](#Layers)
    +   [Fonts](#Fonts)
    +   [Shaders](#Shaders)
    +   [Xfer modes](#Xfer_modes)
*   [Known issues](#Known_issues)

<a name="Typical_usage_of_the_PDF_backend"></a>
Typical usage of the PDF backend
--------------------------------

SkPDFDevice is the main interface to the PDF backend. This child of
SkDevice can be set on an SkCanvas and drawn to. It requires no more
care and feeding than SkDevice. Once drawing is complete, the device
should be added to an SkPDFDocument as a page of the desired PDF. A
new SkPDFDevice should be created for each page desired in the
document. After all the pages have been added to the document,
`SkPDFDocument::emitPDF()` can be called to get a PDF file. One of the
special features of the PDF backend is that the same device can be
added to multiple documents. This for example, would let you generate
a PDF with the single page you just drew as well as adding it to a
longer document with a bunch of other pages.

<!--?prettify lang=cc?-->

    SkPDFCanon canon;
    SkAutoUnref<SkPDFDevice> pdfDevice(
        SkPDFDevice::Create(SkISize::Make(width, height), 72.0f, &canon));

    SkCanvas canvas(pdfDevice);
    draw_content(&canvas);

    SkPDFDocument doc;
    doc.appendPage(dev);
    doc.emitPDF(&pdf_stream);

<a name="PDF_Objects_and_Document_Structure"></a>
PDF Objects and Document Structure
----------------------------------

**Background**: The PDF file format has a header, a set of objects and
then a footer that contains a table of contents for all of the objects
in the document (the cross-reference table). The table of contents
lists the specific byte position for each object. The objects may have
references to other objects and the ASCII size of those references is
dependent on the object number assigned to the referenced object;
therefore we can’t calculate the table of contents until the size of
objects is known, which requires assignment of object
numbers.

Furthermore, PDF files can support a *linearized* mode, where objects
are in a specific order so that pdf-viewers can more easily retrieve
just the objects they need to display a specific page, i.e. by
byte-range requests over the web. Linearization also requires that all
objects used or referenced on the first page of the PDF have object
numbers before the rest of the objects. Consequently, before
generating a linearized PDF, all objects, their sizes, and object
references must be known.  Skia has no plans to implement linearized
PDFs.

<!-- <del>At this point, linearized PDFs are not generated. The
framework to generate them is in place, but the final bits of code
have not been written.</del> -->

    %PDF-1.4
    …objects...
    xref
    0 31  % Total number of entries in the table of contents.
    0000000000 65535 f
    0000210343 00000 n
    …
    0000117055 00000 n
    trailer
    <</Size 31 /Root 1 0 R>>
    startxref
    210399  % Byte offset to the start of the table of contents.
    %%EOF

The class SkPDFCatalog and the virtual class SkPDFObject are used to
manage the needs of the file format. Any object that will represent a
PDF object must inherit from SkPDFObject and implement the methods to
generate the binary representation and report any other SkPDFObjects
used as resources. SkPDFTypes.h defines most of the basic PDF objects
types: bool, int, scalar, string, name, array, dictionary, and object
reference. The stream type is defined in SkPDFStream.h. A stream is a
dictionary containing at least a Length entry followed by the data of
the stream. All of these types except the stream type can be used in
both a direct and an indirect fashion, i.e. an array can have an int
or a dictionary as an inline entry, which does not require an object
number. The stream type, cannot be inlined and must be referred to
with an object reference. Most of the time, other objects types can be
referred to with an object reference, but there are specific rules in
the PDF specification that requires an inline reference in some place
or an indirect reference in other places. All indirect objects must
have an object number assigned.

*   **bools**: `true` `false`
*   **ints**: `42` `0` `-1`
*   **scalars**: `0.001`
*   **strings**: `(strings are in parentheses or byte encoded)` `<74657374>`
*   **name**: `/Name` `/Name#20with#20spaces`
*   **array**: `[/Foo 42 (arrays can contain multiple types)]`
*   **dictionary**: `<</Key1 (value1) /key2 42>>`
*   **indirect object**:  
    `5 0 obj  
    (An indirect string. Indirect objects have an object number and a
    generation number, Skia always uses generation 0 objects)  
    endobj`  
*   **object reference**: `5 0 R`
*   **stream**: `<</Length 56>>  
    stream  
    ...stream contents can be arbitrary, including binary...  
    endstream`

The PDF backend requires all indirect objects used in a PDF to be
added to the SkPDFCatalog of the SkPDFDocument. The catalog is
responsible for assigning object numbers and generating the table of
contents required at the end of PDF files. In some sense, generating a
PDF is a three step process. In the first step all the objects and
references among them are created (mostly done by SkPDFDevice). In the
second step, object numbers are assigned and SkPDFCatalog is informed
of the file offset of each indirect object. Finally, in the third
step, the header is printed, each object is printed, and then the
table of contents and trailer are printed. SkPDFDocument takes care of
collecting all the objects from the various SkPDFDevice instances,
adding them to an SkPDFCatalog, iterating through the objects once to
set their file positions, and iterating again to generate the final
PDF.

    %PDF-1.4
    2 0 obj <<
      /Type /Catalog
      /Pages 1 0 R
    >>
    endobj
    3 0 obj <<
      /Type /Page
      /Parent 1 0 R
      /Resources <>
      /MediaBox [0 0 612 792]
      /Contents 4 0 R
    >>
    endobj
    4 0 obj <> stream
    endstream
    endobj
    1 0 obj <<
      /Type /Pages
      /Kids [3 0 R]
      /Count 1
    >>
    endobj
    xref
    0 5
    0000000000 65535 f
    0000000236 00000 n
    0000000009 00000 n
    0000000062 00000 n
    0000000190 00000 n
    trailer
    <</Size 5 /Root 2 0 R>>
    startxref
    299
    %%EOF

<a name="PDF_drawing"></a>
PDF drawing
-----------

Most drawing in PDF is specified by the text of a stream, referred to
as a content stream. The syntax of the content stream is different
than the syntax of the file format described above and is much closer
to PostScript in nature. The commands in the content stream tell the
PDF interpreter to draw things, like a rectangle (`x y w h re`), an
image, or text, or to do meta operations like set the drawing color,
apply a transform to the drawing coordinates, or clip future drawing
operations. The page object that references a content stream has a
list of resources that can be used in the content stream using the
dictionary name to reference the resources. Resources are things like
font objects, images objects, graphic state objects (a set of meta
operations like miter limit, line width, etc). Because of a mismatch
between Skia and PDF’s support for transparency (which will be
explained later), SkPDFDevice records each drawing operation into an
internal structure (ContentEntry) and only when the content stream is
needed does it flatten that list of structures into the final content
stream.

    4 0 obj <<
      /Type /Page
      /Resources <<
        /Font <</F1 9 0 R>>
        /XObject <</Image1 22 0 R /Image2 73 0 R>>
      >>
      /Content 5 0 R
    >> endobj

    5 0 obj <</Length 227>> stream
    % In the font specified in object 9 and a height
    % of 12 points, at (72, 96) draw ‘Hello World.’
    BT
      /F1 12 Tf
      72 96 Td
      (Hello World) Tj
    ET
    % Draw a filled rectange.
    200 96 72 72 re B
    ...
    endstream
    endobj

<a name="Interned_objects"></a>
Interned objects
----------------

There are a number of high level PDF objects (like fonts, graphic
states, etc) that are likely to be referenced multiple times in a
single PDF. To ensure that there is only one copy of each object
instance these objects an implemented with an
[interning pattern](http://en.wikipedia.org/wiki/String_interning).
As such, the classes representing these objects (like
SkPDFGraphicState) have private constructors and static methods to
retrieve an instance of the class. Internally, the class has a list of
unique instances that it consults before returning a new instance of
the class. If the requested instance already exists, the existing one
is returned. For obvious reasons, the returned instance should not be
modified. A mechanism to ensure that interned classes are immutable is
needed.  See [issue 2683](http://skbug.com/2683).

<a name="Graphic_States"></a>
Graphic States
--------------

PDF has a number of parameters that affect how things are drawn. The
ones that correspond to drawing options in Skia are: color, alpha,
line cap, line join type, line width, miter limit, and xfer/blend mode
(see later section for xfer modes). With the exception of color, these
can all be specified in a single pdf object, represented by the
SkPDFGraphicState class. A simple command in the content stream can
then set the drawing parameters to the values specified in that
graphic state object. PDF does not allow specifying color in the
graphic state object, instead it must be specified directly in the
content stream. Similarly the current font and font size are set
directly in the content stream.

    6 0 obj <<
      /Type /ExtGState
      /CA 1  % Opaque - alpha = 1
      /LC 0  % Butt linecap
      /LJ 0  % Miter line-join
      /LW 2  % Line width of 2
      /ML 6  % Miter limit of 6
      /BM /Normal  % Blend mode is normal i.e. source over
    >>
    endobj

<a name="Clip_and_Transform"></a>
Clip and Transform
------------------

Similar to Skia, PDF allows drawing to be clipped or
transformed. However, there are a few caveats that affect the design
of the PDF backend. PDF does not support perspective transforms
(perspective transform are treated as identity transforms). Clips,
however, have more issues to cotend with. PDF clips cannot be directly
unapplied or expanded. i.e. once an area has been clipped off, there
is no way to draw to it. However, PDF provides a limited depth stack
for the PDF graphic state (which includes the drawing parameters
mentioned above in the Graphic States section as well as the clip and
transform). Therefore to undo a clip, the PDF graphic state must be
pushed before the clip is applied, then popped to revert to the state
of the graphic state before the clip was applied.

As the canvas makes drawing calls into SkPDFDevice, the active
transform, clip region, and clip stack are stored in a ContentEntry
structure. Later, when the ContentEntry structures are flattened into
a valid PDF content stream, the transforms and clips are compared to
decide on an efficient set of operations to transition between the
states needed. Currently, a local optimization is used, to figure out
the best transition from one state to the next. A global optimization
could improve things by more effectively using the graphics state
stack provided in the PDF format.

<a name="Generating_a_content_stream"></a>
Generating a content stream
---------------------------

For each draw call on an SkPDFDevice, a new ContentEntry is created,
which stores the matrix, clip region, and clip stack as well as the
paint parameters. Most of the paint parameters are bundled into an
SkPDFGraphicState (interned) with the rest (color, font size, etc)
explicitly stored in the ContentEntry. After populating the
ContentEntry with all the relevant context, it is compared to the the
most recently used ContentEntry. If the context matches, then the
previous one is appended to instead of using the new one. In either
case, with the context populated into the ContentEntry, the
appropriate draw call is allowed to append to the content stream
snippet in the ContentEntry to affect the core of the drawing call,
i.e. drawing a shape, an image, text, etc.

When all drawing is complete, SkPDFDocument::emitPDF() will call
SkPDFDevice::content() to request the complete content stream for the
page. The first thing done is to apply the initial transform specified
in part in the constructor, this transform takes care of changing the
coordinate space from an origin in the lower left (PDF default) to the
upper left (Skia default) as well as any translation or scaling
requested by the user (i.e. to achieve a margin or scale the
canvas). Next (well almost next, see the next section), a clip is
applied to restrict drawing to the content area (the part of the page
inside the margins) of the page. Then, each ContentEntry is applied to
the content stream with the help of a helper class, GraphicStackState,
which tracks the state of the PDF graphics stack and optimizes the
output. For each ContentEntry, commands are emitted to the final
content entry to update the clip from its current state to the state
specified in the ContentEntry, similarly the Matrix and drawing state
(color, line joins, etc) are updated, then the content entry fragment
(the actual drawing operation) is appended.

<a name="Margins_and_content_area"></a>
Margins and content area
------------------------

The above procedure does not permit drawing in the margins. This is
done in order to contain any rendering problems in WebKit. In order to
support headers and footers, which are drawn in the margin, a second
set of ContentEntry’s are maintained. The
methodSkPDFDevice::setDrawingArea() selects which set of
ContentEntry’s are drawn into. Then, in the SkPDFDevice::content()
method, just before the clip to the content area is applied the margin
ContentEntry's are played back.

<!-- TODO(halcanary): update this documentation. -->

<a name="Drawing_details"></a>
Drawing details
---------------

Certain objects have specific properties that need to be dealt
with. Images, layers (see below), and fonts assume the standard PDF
coordinate system, so we have to undo any flip to the Skia coordinate
system before drawing these entities. We don’t currently support
inverted paths, so filling an inverted path will give the wrong result
([issue 241](http://skbug.com/241)). PDF doesn’t draw zero length
lines that have butt of square caps, so that is emulated.

<a name="Layers"></a>
### Layers ###

PDF has a higher level object called a form x-object (form external
object) that is basically a PDF page, with resources and a content
stream, but can be transformed and drawn on an existing page. This is
used to implement layers. SkDevice has a method,
createFormXObjectFromDevice, which uses the SkPDFDevice::content()
method to construct a form x-object from the the
device. SkPDFDevice::drawDevice() works by creating a form x-object of
the passed device and then drawing that form x-object in the root
device. There are a couple things to be aware of in this process. As
noted previously, we have to be aware of any flip to the coordinate
system - flipping it an even number of times will lead to the wrong
result unless it is corrected for. The SkClipStack passed to drawing
commands includes the entire clip stack, including the clipping
operations done on the base layer. Since the form x-object will be
drawn as a single operation onto the base layer, we can assume that
all of those clips are in effect and need not apply them within the
layer.

<a name="Fonts"></a>
### Fonts ###

There are many details for dealing with fonts, so this document will
only talk about some of the more important ones. A couple short
details:

*   We can’t assume that an arbitrary font will be available at PDF view
    time, so we embed all fonts in accordance with modern PDF
    guidelines.
*   Most fonts these days are TrueType fonts, so this is where most of
    the effort has been concentrated.
*   Because Skia may only be given a glyph-id encoding of the text to
    render and there is no perfect way to reverse the encoding, the
    PDF backend always uses the glyph-id encoding of the text.

#### *Type1/Type3 fonts* ####

Linux supports Type1 fonts, but Windows and Mac seem to lack the
functionality required to extract the required information from the
font without parsing the font file. When a non TrueType font is used
any any platform (except for Type1 on Linux), it is encoded as a Type3
font. In this context, a Type3 font is an array of form x-objects
(content streams) that draw each glyph of the font. No hinting or
kerning information is included in a Type3 font, just the shape of
each glyph. Any font that has the do-not embed copy protection bit set
will also get embedded as a Type3 font. From what I understand, shapes
are not copyrightable, but programs are, so by stripping all the
programmatic information and only embedding the shape of the glyphs we
are honoring the do-not embed bit as much as required by law.

PDF only supports an 8-bit encoding for Type1 or Type3 fonts. However,
they can contain more than 256 glyphs. The PDF backend handles this by
segmenting the glyphs into groups of 255 (glyph id 0 is always the
unknown glyph) and presenting the font as multiple fonts, each with up
to 255 glyphs.

#### *Font subsetting* ####

Many fonts, especially fonts with CJK support are fairly large, so it
is desirable to subset them. Chrome uses the SFNTLY package to provide
subsetting support to Skia for TrueType fonts. However, there is a
conflict between font subsetting and interned objects. If the object
is immutable, how can it be subsetted? This conflict is resolved by
using a substitution mechanism in SkPDFCatalog. Font objects are still
interned, but the interned objects aren’t internally
populated. Subsetting starts while drawing text to an SkPDFDevice; a
bit set indicating which glyphs have been used is maintained. Later,
when SkPDFDocument::emitPDF() is rendering the PDF, it queries each
device (each page) for the set of fonts used and the glyphs used from
each font and combines the information. It then asks the interned
(unpopulated) font objects to create a populated instance with the
calculated subset of the font - this instance is not interned. The
subsetted instance is then set as a substitute for the interned font
object in the SkPDFCatalog. All future references to those fonts
within that document will refer to the subsetted instances, resulting
in a final PDF with exactly one instance of each used font that
includes only the glyphs used.

The substitution mechanism is a little complicated, but is needed to
support the use case of an SkPDFDevice being added to multiple
documents. If fonts were subsetted in-situ, concurrent PDF generation
would have to be explicitly handled. Instead, by giving each document
its own subsetted instance, there is no need to worry about concurrent
PDF generation. The substitution method is also used to support
optional stream compression. A stream can used by different documents
in both a compressed and uncompressed form, leading to the same
potential difficulties faced by the concurrent font use case.

<a name="Shaders"></a>
### Shaders ###

Skia has two types of predefined shaders, image shaders and gradient
shaders. In both cases, shaders are effectively positioned absolutely,
so the initial position and bounds of where they are visible is part
of the immutable state of the shader object. Each of the Skia’s tile
modes needs to be considered and handled explicitly. The image shader
we generate will be tiled, so tiling is handled by default. To support
mirroring, we draw the image, reversed, on the appropriate axis, or on
both axes plus a fourth in the vacant quadrant. For clamp mode, we
extract the pixels along the appropriate edge and stretch the single
pixel wide/long image to fill the bounds. For both x and y in clamp
mode, we fill the corners with a rectangle of the appropriate
color. The composed shader is then rotated or scaled as appropriate
for the request.

Gradient shaders are handled purely mathematically. First, the matrix
is transformed so that specific points in the requested gradient are
at pre-defined locations, for example, the linear distance of the
gradient is always normalized to one. Then, a type 4 PDF function is
created that achieves the desired gradient. A type 4 function is a
function defined by a resticted postscript language. The generated
functions clamp at the edges so if the desired tiling mode is tile or
mirror, we hav to add a bit more postscript code to map any input
parameter into the 0-1 range appropriately. The code to generate the
postscript code is somewhat obtuse, since it is trying to generate
optimized (for space) postscript code, but there is a significant
number of comments to explain the intent.

<a name="Xfer_modes"></a>
### Xfer modes ###

PDF supports some of the xfer modes used in Skia directly. For those,
it is simply a matter of setting the blend mode in the graphic state
to the appropriate value (Normal/SrcOver, Multiply, Screen, Overlay,
Darken, Lighten, !ColorDOdge, ColorBurn, HardLight, SoftLight,
Difference, Exclusion). Aside from the standard SrcOver mode, PDF does
not directly support the porter-duff xfer modes though. Most of them
(Clear, SrcMode, DstMode, DstOver, SrcIn, DstIn, SrcOut, DstOut) can
be emulated by various means, mostly by creating form x-objects out of
part of the content and drawing it with a another form x-object as a
mask. I have not figured out how to emulate the following modes:
SrcATop, DstATop, Xor, Plus.

At the time of writing [2012-06-25], I have a [CL outstanding to fix a
misunderstanding I had about the meaning of some of the emulated
modes](https://codereview.appspot.com/4631078/).
I will describe the system with this change applied.

First, a bit of terminology and definition. When drawing something
with an emulated xfer mode, what’s already drawn to the device is
called the destination or Dst, and what’s about to be drawn is the
source or Src. Src (and Dst) can have regions where it is transparent
(alpha equals zero), but it also has an inherent shape. For most kinds
of drawn objects, the shape is the same as where alpha is not
zero. However, for things like images and layers, the shape is the
bounds of the item, not where the alpha is non-zero. For example, a
10x10 image, that is transparent except for a 1x1 dot in the center
has a shape that is 10x10. The xfermodes gm test demonstrates the
interaction between shape and alpha in combination with the port-duff
xfer modes.

The clear xfer mode removes any part of Dst that is within Src’s
shape. This is accomplished by bundling the current content of the
device (Dst) into a single entity and then drawing that with the
inverse of Src’s shape used as a mask (we want Dst where Src
isn’t). The implementation of that takes a couple more steps. You may
have to refer back to [the content stream section](#Generating_a_content_stream). For any draw call, a
ContentEntry is created through a method called
SkPDFDevice::setUpContentEntry(). This method examines the xfer modes
in effect for that drawing operation and if it is an xfer mode that
needs emulation, it creates a form x-object from the device,
i.e. creates Dst, and stores it away for later use. This also clears
all of that existing ContentEntry's on that device. The drawing
operation is then allowed to proceed as normal (in most cases, see
note about shape below), but into the now empty device. Then, when the
drawing operation in done, a complementary method is
called,SkPDFDevice::finishContentEntry(), which takes action if the
current xfer mode is emulated. In the case of Clear, it packages what
was just drawn into another form x-object, and then uses the Src form
x-object, an invert function, and the Dst form x-object to draw Dst
with the inverse shape of Src as a mask. This works well when the
shape of Src is the same as the opaque part of the drawing, since PDF
uses the alpha channel of the mask form x-object to do masking. When
shape doesn’t match the alpha channel, additional action is
required. The drawing routines where shape and alpha don’t match, set
state to indicate the shape (always rectangular), which
finishContentEntry uses. The clear xfer mode is a special case; if
shape is needed, then Src isn’t used, so there is code to not bother
drawing Src if shape is required and the xfer mode is clear.

SrcMode is clear plus Src being drawn afterward. DstMode simply omits
drawing Src. DstOver is the same as SrcOver with Src and Dst swapped -
this is accomplished by inserting the new ContentEntry at the
beginning of the list of ContentEntry’s in setUpContentEntry instead
of at the end. SrcIn, SrcOut, DstIn, DstOut are similar to each, the
difference being an inverted or non-inverted mask and swapping Src and
Dst (or not). SrcIn is SrcMode with Src drawn with Dst as a
mask. SrcOut is like SrcMode, but with Src drawn with an inverted Dst
as a mask. DstIn is SrcMode with Dst drawn with Src as a
mask. Finally, DstOut is SrcMode with Dst draw with an inverted Src as
a mask.

<a name="Known_issues"></a>
Known issues
------------

*   [issue 241](http://skbug.com/241)
    As previously noted, a boolean geometry library
    would improve clip fidelity in some places, add supported for
    inverted fill types, as well as simplify code.
    This is fixed, but behind a flag until path ops is production ready.
*   [issue 237](http://skbug.com/237)
    SkMaskFilter is not supported.
*   [issue 238](http://skbug.com/238)
    SkColorFilter is not supported.
*   [issue 249](http://skbug.com/249)
    SrcAtop Xor, and Plus xfer modes are not supported.
*   [issue 240](http://skbug.com/240)
    drawVerticies is not implemented.
*   [issue 244](http://skbug.com/244)
    Mostly, only TTF fonts are directly supported.  (User metrics 
    show that almost all fonts are truetype.
*   [issue 260](http://skbug.com/260)
    Page rotation is accomplished by specifying a different
    size page instead of including the appropriate rotation
    annotation.

* * *

