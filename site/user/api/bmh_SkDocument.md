

# <a name="Document"></a> Document

# <a name="SkDocument"></a> Class SkDocument

# <a name="Overview"></a> Overview

## <a name="Document_Member_Functions"></a> Document Member Functions

| function | description |
| --- | ---  |
| <a href="bmh_SkDocument?cl=9919#MakePDF">MakePDF</a> |  |
| <a href="bmh_SkDocument?cl=9919#MakeXPS">MakeXPS</a> |  |
| abort |  |
| <a href="bmh_SkDocument?cl=9919#beginPage">beginPage</a> |  |
| close |  |
| <a href="bmh_SkDocument?cl=9919#endPage">endPage</a> |  |

<a name="MakePDF"></a>

## MakePDF

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
static sk_sp<SkDocument> MakePDF(SkWStream* stream, SkScalar dpi,
                                 const SkDocument::PDFMetadata& metadata,
                                 sk_sp<SkPixelSerializer> jpegEncoder,
                                 bool pdfa)
</pre>

Create a <a href="bmh_SkDocument?cl=9919#PDF">PDF</a> -backed document, writing the results into an
output <a href="bmh_SkDocument?cl=9919#MakePDF">stream</a>.
<a href="bmh_SkDocument?cl=9919#PDF">PDF</a> pages are sized in point units. 1 pt == 1/72 inch ==
127/360 mm.

### Parameters

<table>
  <tr>
    <td><code><strong>stream</strong></code></td> <td>A <a href="bmh_SkDocument?cl=9919#PDF">PDF</a> document will be written to this
<a href="bmh_SkDocument?cl=9919#MakePDF">stream</a>.  The document may write to the <a href="bmh_SkDocument?cl=9919#MakePDF">stream</a> at
anytime during its lifetime, until either close is
called or the document is deleted.</td>
  </tr>
  <tr>
    <td><code><strong>dpi</strong></code></td> <td>The <a href="bmh_SkDocument?cl=9919#DPI">DPI</a> (pixels-per-inch) at which features without
native <a href="bmh_SkDocument?cl=9919#PDF">PDF</a> support will be rasterized (e.g. draw image
with perspective, draw text with perspective, ...)  A
larger <a href="bmh_SkDocument?cl=9919#DPI">DPI</a> would create a <a href="bmh_SkDocument?cl=9919#PDF">PDF</a> that reflects the
original intent with better fidelity, but it can make
for larger <a href="bmh_SkDocument?cl=9919#PDF">PDF</a> files too, which would use more memory
while rendering, and it would be slower to be processed
or sent online or to printer.</td>
  </tr>
  <tr>
    <td><code><strong>metadata</strong></code></td> <td>a object.  Any fields may be left
empty.</td>
  </tr>
  <tr>
    <td><code><strong>jpegEncoder</strong></code></td> <td>For <a href="bmh_SkDocument?cl=9919#PDF">PDF</a> documents, if a <a href="bmh_SkDocument?cl=9919#MakePDF">jpegEncoder</a> is set,
use it to encode <a href="bmh_SkDocument?cl=9919#Image">Image</a> and <a href="bmh_SkDocument?cl=9919#Bitmap">Bitmap</a> as [<a href="bmh_SkDocument?cl=9919#JFIF">JFIF</a>]<a href="bmh_SkDocument?cl=9919#JPEG">JPEG</a>.
This feature is deprecated and is only supplied for
backwards compatability.
The prefered method to create a <a href="bmh_SkDocument?cl=9919#PDF">PDF</a> with <a href="bmh_SkDocument?cl=9919#JPEG">JPEG</a> images is
to use</td>
  </tr>
  <tr>
    <td><code><strong>pdfa</strong></code></td> <td>If set, include <a href="bmh_SkDocument?cl=9919#XMP">XMP</a> <a href="bmh_SkDocument?cl=9919#MakePDF">metadata</a>, a document UUID,
and sRGB output intent information.  This adds length
to the document and makes it non-reproducable, but are
necessary features for <a href="bmh_SkDocument?cl=9919#PDF">PDF</a>/<a href="bmh_SkDocument?cl=9919#A_2b">A-2b</a> conformance.</td>
  </tr>
</table>

### Return Value

nullptr if there is an error, otherwise a newly created
<a href="bmh_SkDocument?cl=9919#PDF">PDF</a> -backed <a href="bmh_SkDocument?cl=9919#SkDocument">SkDocument</a>.

### Example

<div><fiddle-embed name="9fa428108c4175c65a76dc250266196c"></fiddle-embed></div>

---

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
static sk_sp<SkDocument> MakePDF(SkWStream* stream,
                                 SkScalar dpi = SK_ScalarDefaultRasterDPI)
</pre>

Shorthand for the other <a href="bmh_SkDocument?cl=9919#MakePDF">MakePDF</a> function.

### Parameters

<table>
  <tr>
    <td><code><strong>stream</strong></code></td> <td>A <a href="bmh_SkDocument?cl=9919#PDF">PDF</a> document will be written to this
<a href="bmh_SkDocument?cl=9919#MakePDF_2">stream</a>.  The document may write to the <a href="bmh_SkDocument?cl=9919#MakePDF_2">stream</a> at
anytime during its lifetime, until either close is
called or the document is deleted.</td>
  </tr>
  <tr>
    <td><code><strong>dpi</strong></code></td> <td>The <a href="bmh_SkDocument?cl=9919#DPI">DPI</a> (pixels-per-inch) at which features without
native <a href="bmh_SkDocument?cl=9919#PDF">PDF</a> support will be rasterized (e.g. draw image
with perspective, draw text with perspective, ...)  A
larger <a href="bmh_SkDocument?cl=9919#DPI">DPI</a> would create a <a href="bmh_SkDocument?cl=9919#PDF">PDF</a> that reflects the
original intent with better fidelity, but it can make
for larger <a href="bmh_SkDocument?cl=9919#PDF">PDF</a> files too, which would use more memory
while rendering, and it would be slower to be processed
or sent online or to printer.</td>
  </tr>
</table>

### Return Value

nullptr if there is an error, otherwise a newly created

<a href="bmh_SkDocument?cl=9919#PDF">PDF</a> -backed <a href="bmh_SkDocument?cl=9919#SkDocument">SkDocument</a>.

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
static sk_sp<SkDocument> MakePDF(const char outputFilePath[],
                                 SkScalar dpi = SK_ScalarDefaultRasterDPI)
</pre>

Create a <a href="bmh_SkDocument?cl=9919#PDF">PDF</a> -backed document, writing the results into a file.

### Parameters

<table>
  <tr>
    <td><code><strong>outputFilePath</strong></code></td> <td>Where to write the <a href="bmh_SkDocument?cl=9919#PDF">PDF</a> output.</td>
  </tr>
  <tr>
    <td><code><strong>dpi</strong></code></td> <td>The <a href="bmh_SkDocument?cl=9919#DPI">DPI</a> (pixels-per-inch) at which features without
native <a href="bmh_SkDocument?cl=9919#PDF">PDF</a> support will be rasterized (e.g. draw image
with perspective, draw text with perspective, ...)  A
larger <a href="bmh_SkDocument?cl=9919#DPI">DPI</a> would create a <a href="bmh_SkDocument?cl=9919#PDF">PDF</a> that reflects the
original intent with better fidelity, but it can make
for larger <a href="bmh_SkDocument?cl=9919#PDF">PDF</a> files too, which would use more memory
while rendering, and it would be slower to be processed
or sent online or to printer.</td>
  </tr>
</table>

### Return Value

nullptr if there is an error, otherwise a newly created

<a href="bmh_SkDocument?cl=9919#PDF">PDF</a> -backed <a href="bmh_SkDocument?cl=9919#SkDocument">SkDocument</a>.

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<a name="MakeXPS"></a>

## MakeXPS

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
static sk_sp<SkDocument> MakeXPS(SkWStream* stream,
                                 IXpsOMObjectFactory* xpsFactory,
                                 SkScalar dpi = SK_ScalarDefaultRasterDPI)
</pre>

Create a <a href="bmh_SkDocument?cl=9919#XPS">XPS</a> -backed document, writing the results into the <a href="bmh_SkDocument?cl=9919#MakeXPS">stream</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>stream</strong></code></td> <td>A <a href="bmh_SkDocument?cl=9919#XPS">XPS</a> document will be written to this <a href="bmh_SkDocument?cl=9919#MakeXPS">stream</a>.  The
document may write to the <a href="bmh_SkDocument?cl=9919#MakeXPS">stream</a> at anytime during its
lifetime, until either close or abort are called or
the document is deleted.</td>
  </tr>
  <tr>
    <td><code><strong>xpsFactory</strong></code></td> <td>A pointer to a <a href="bmh_SkDocument?cl=9919#COM">COM</a> <a href="bmh_SkDocument?cl=9919#XPS">XPS</a> factory.  Must be non-null.
The document will take a ref to the factory. See
dm/DMSrcSink.cpp for an example.</td>
  </tr>
  <tr>
    <td><code><strong>dpi</strong></code></td> <td>The <a href="bmh_SkDocument?cl=9919#DPI">DPI</a> (pixels-per-inch) at which features without
native <a href="bmh_SkDocument?cl=9919#XPS">XPS</a> support will be rasterized (e.g. draw image
with perspective, draw text with perspective, ...)  A
larger <a href="bmh_SkDocument?cl=9919#DPI">DPI</a> would create a <a href="bmh_SkDocument?cl=9919#XPS">XPS</a> that reflects the
original intent with better fidelity, but it can make
for larger <a href="bmh_SkDocument?cl=9919#XPS">XPS</a> files too, which would use more memory
while rendering, and it would be slower to be processed
or sent online or to printer.</td>
  </tr>
</table>

### Return Value

returns nullptr if <a href="bmh_SkDocument?cl=9919#XPS">XPS</a> is not supported, otherwise a newly created

<a href="bmh_SkDocument?cl=9919#XPS">XPS</a> -backed <a href="bmh_SkDocument?cl=9919#SkDocument">SkDocument</a>.

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<a name="beginPage"></a>

## beginPage

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
SkCanvas* beginPage(SkScalar width, SkScalar height,
                    const SkRect* content = nullptr)
</pre>

Begin a new page for the document, returning the canvas that will draw
into the page.

### Parameters

<table>
  <tr>
    <td><code><strong>width</strong></code></td> <td>page <a href="bmh_SkDocument?cl=9919#beginPage">width</a> in units specific to the <a href="bmh_SkDocument?cl=9919#Document">Document</a> type.</td>
  </tr>
  <tr>
    <td><code><strong>height</strong></code></td> <td>page <a href="bmh_SkDocument?cl=9919#beginPage">height</a> in units specific to the <a href="bmh_SkDocument?cl=9919#Document">Document</a> type.</td>
  </tr>
  <tr>
    <td><code><strong>content</strong></code></td> <td>TODO</td>
  </tr>
</table>

### Return Value

A pointer to a <a href="bmh_SkDocument?cl=9919#Canvas">Canvas</a> for this page.  The document owns this

canvas, and it will go out of scope when <a href="bmh_SkDocument?cl=9919#endPage">endPage</a> or close
is called, or the document is deleted.

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<a name="endPage"></a>

## endPage

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void endPage()
</pre>

Call <a href="bmh_SkDocument?cl=9919#endPage">endPage</a> when the content for the current page has been drawn
(into the canvas returned by <a href="bmh_SkDocument?cl=9919#beginPage">beginPage</a>). After this call the canvas
returned by <a href="bmh_SkDocument?cl=9919#beginPage">beginPage</a> will be out-of-scope.

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<a name="close"></a>

## close

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void close()
</pre>

Call close when all pages have been drawn. This will close the file
or stream holding the document's contents. After close the document
can no longer add new pages. Deleting the document will automatically
call close if need be.

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<a name="abort"></a>

## abort

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
void abort()
</pre>

Call abort to stop producing the document immediately.
The stream output must be ignored, and should not be trusted.

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

