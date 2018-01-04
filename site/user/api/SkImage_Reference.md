SkImage Reference
===

# <a name="Image"></a> Image

# <a name="SkImage"></a> Class SkImage
<a href="#Image">Image</a> describes a two dimensional array of pixels to draw. The pixels may be
unencoded in a <a href="undocumented#Raster_Bitmap">Raster Bitmap</a>, encoded in a <a href="undocumented#Picture">Picture</a> or compressed data stream,
or located in GPU memory as a <a href="undocumented#GPU_Texture">GPU Texture</a>.

<a href="#Image">Image</a> cannot be modified after it is created. <a href="#Image">Image</a> may allocate additional
storage as needed; for instance, an encoded <a href="#Image">Image</a> may decode when drawn.

<a href="#Image">Image</a> <a href="#SkImage_width">width</a> and <a href="#SkImage_height">height</a> are greater than zero. Creating an <a href="#Image">Image</a> with zero <a href="#SkImage_width">width</a>
or <a href="#SkImage_height">height</a> returns <a href="#Image">Image</a> equal to nullptr.

<a href="#Image">Image</a> may be created from <a href="SkBitmap_Reference#Bitmap">Bitmap</a>, <a href="SkPixmap_Reference#Pixmap">Pixmap</a>, <a href="SkSurface_Reference#Surface">Surface</a>, <a href="undocumented#Picture">Picture</a>, encoded streams,
<a href="undocumented#GPU_Texture">GPU Texture</a>, <a href="undocumented#YUV_ColorSpace">YUV ColorSpace</a> data, or hardware buffer. Encoded streams supported
include BMP, GIF, HEIF, ICO, JPEG, PNG, WBMP, WebP. Supported encodings details
vary with platform.

# <a name="Raster_Image"></a> Raster Image
<a href="SkImage_Reference#Raster_Image">Raster Image</a> pixels are unencoded in a <a href="undocumented#Raster_Bitmap">Raster Bitmap</a>. These pixels may be read
directly and in most cases written to, although edited pixels may not be drawn
if <a href="#Image">Image</a> has been copied internally.

# <a name="Texture_Image"></a> Texture Image
<a href="#Texture_Image">Texture Image</a> are located on GPU and pixels are not accessible. <a href="#Texture_Image">Texture Image</a>
are allocated optimally for best performance. <a href="SkImage_Reference#Raster_Image">Raster Image</a> may
be drawn to <a href="undocumented#GPU_Surface">GPU Surface</a>, but pixels are uploaded from CPU to GPU downgrading
performance.

# <a name="Lazy_Image"></a> Lazy Image
<a href="#Lazy_Image">Lazy Image</a> defer allocating buffer for <a href="#Image">Image</a> pixels and decoding stream until
<a href="#Image">Image</a> is drawn. <a href="#Lazy_Image">Lazy Image</a> caches result if possible to speed up repeated
drawing.

# <a name="Overview"></a> Overview

## <a name="Subtopics"></a> Subtopics

| topics | description |
| --- | ---  |

## <a name="Member_Functions"></a> Member Functions

| description | function |
| --- | ---  |
| <a href="#SkImage_MakeBackendTextureFromSkImage">MakeBackendTextureFromSkImage</a> | Creates <a href="undocumented#GPU_Texture">GPU Texture</a> from <a href="#Image">Image</a>. |
| <a href="#SkImage_MakeCrossContextFromEncoded">MakeCrossContextFromEncoded</a> | Creates <a href="#Image">Image</a> from encoded data, and uploads to GPU. |
| <a href="#SkImage_MakeFromAHardwareBuffer">MakeFromAHardwareBuffer</a> | Creates <a href="#Image">Image</a> from Android hardware buffer. |
| <a href="#SkImage_MakeFromAdoptedTexture">MakeFromAdoptedTexture</a> | Creates <a href="#Image">Image</a> from <a href="undocumented#GPU_Texture">GPU Texture</a>, managed internally. |
| <a href="#SkImage_MakeFromBitmap">MakeFromBitmap</a> | Creates <a href="#Image">Image</a> from <a href="SkBitmap_Reference#Bitmap">Bitmap</a>, sharing or copying pixels. |
| <a href="#SkImage_MakeFromDeferredTextureImageData">MakeFromDeferredTextureImageData</a> |  |
| <a href="#SkImage_MakeFromEncoded">MakeFromEncoded</a> | Creates <a href="#Image">Image</a> from encoded data. |
| <a href="#SkImage_MakeFromGenerator">MakeFromGenerator</a> | Creates <a href="#Image">Image</a> from a stream of data. |
| <a href="#SkImage_MakeFromNV12TexturesCopy">MakeFromNV12TexturesCopy</a> | Creates <a href="#Image">Image</a> from <a href="undocumented#YUV_ColorSpace">YUV ColorSpace</a> data in two planes. |
| <a href="#SkImage_MakeFromPicture">MakeFromPicture</a> | Creates <a href="#Image">Image</a> from <a href="undocumented#Picture">Picture</a>. |
| <a href="#SkImage_MakeFromRaster">MakeFromRaster</a> | Creates <a href="#Image">Image</a> from <a href="SkPixmap_Reference#Pixmap">Pixmap</a>, with release. |
| <a href="#SkImage_MakeFromTexture">MakeFromTexture</a> | Creates <a href="#Image">Image</a> from <a href="undocumented#GPU_Texture">GPU Texture</a>, managed externally. |
| <a href="#SkImage_MakeFromYUVTexturesCopy">MakeFromYUVTexturesCopy</a> | Creates <a href="#Image">Image</a> from <a href="undocumented#YUV_ColorSpace">YUV ColorSpace</a> data in three planes. |
| <a href="#SkImage_MakeRasterCopy">MakeRasterCopy</a> | Creates <a href="#Image">Image</a> from <a href="SkPixmap_Reference#Pixmap">Pixmap</a> and copied pixels. |
| <a href="#SkImage_MakeRasterData">MakeRasterData</a> | Creates <a href="#Image">Image</a> from <a href="#Info">Image Info</a> and shared pixels. |
| <a href="#SkImage_alphaType">alphaType</a> | Returns <a href="#Alpha_Type">Alpha Type</a>. |
| <a href="#SkImage_asLegacyBitmap">asLegacyBitmap</a> | Returns as <a href="undocumented#Raster_Bitmap">Raster Bitmap</a>. |
| <a href="#SkImage_bounds">bounds</a> | Returns <a href="#SkImage_width">width</a> and <a href="#SkImage_height">height</a> as Rectangle. |
| <a href="#SkImage_colorSpace">colorSpace</a> | Returns <a href="undocumented#Color_Space">Color Space</a>. |
| <a href="#SkImage_dimensions">dimensions</a> | Returns <a href="#SkImage_width">width</a> and <a href="#SkImage_height">height</a>. |
| <a href="#SkImage_encodeToData">encodeToData</a> | Returns encoded <a href="#Image">Image</a> as <a href="undocumented#SkData">SkData</a>. |
| <a href="#SkImage_getDeferredTextureImageData">getDeferredTextureImageData</a> |  |
| <a href="#SkImage_getTexture">getTexture</a> | Deprecated. |
| <a href="#SkImage_getTextureHandle">getTextureHandle</a> | Returns GPU reference to <a href="#Image">Image</a> as texture. |
| <a href="#SkImage_height">height</a> | Returns pixel row count. |
| <a href="#SkImage_isAlphaOnly">isAlphaOnly</a> | Returns if pixels represent a transparency mask. |
| <a href="#SkImage_isLazyGenerated">isLazyGenerated</a> | Returns if <a href="#Image">Image</a> is created as needed. |
| <a href="#SkImage_isOpaque">isOpaque</a> | Returns if <a href="#Alpha_Type">Alpha Type</a> is <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>. |
| <a href="#SkImage_isTextureBacked">isTextureBacked</a> | Returns if <a href="#Image">Image</a> was created from <a href="undocumented#GPU_Texture">GPU Texture</a>. |
| <a href="#SkImage_isValid">isValid</a> | Returns if <a href="#Image">Image</a> can draw to <a href="undocumented#Raster_Surface">Raster Surface</a> or <a href="undocumented#GPU_Context">GPU Context</a>. |
| <a href="#SkImage_makeColorSpace">makeColorSpace</a> | Creates <a href="#Image">Image</a> matching <a href="undocumented#Color_Space">Color Space</a> if possible. |
| <a href="#SkImage_makeNonTextureImage">makeNonTextureImage</a> | Creates <a href="#Image">Image</a> without dependency on <a href="undocumented#GPU_Texture">GPU Texture</a>. |
| <a href="#SkImage_makeRasterImage">makeRasterImage</a> | Creates <a href="#Image">Image</a> compatible with <a href="undocumented#Raster_Surface">Raster Surface</a> if possible. |
| <a href="#SkImage_makeShader">makeShader</a> | Creates <a href="undocumented#Shader">Shader</a>, <a href="SkPaint_Reference#Paint">Paint</a> element that can tile <a href="#Image">Image</a>. |
| <a href="#SkImage_makeSubset">makeSubset</a> | Creates <a href="#Image">Image</a> containing part of original. |
| <a href="#SkImage_makeTextureImage">makeTextureImage</a> | Creates <a href="#Image">Image</a> matching <a href="undocumented#Color_Space">Color Space</a> if possible. |
| <a href="#SkImage_makeWithFilter">makeWithFilter</a> | Creates filtered, clipped <a href="#Image">Image</a>. |
| <a href="#SkImage_peekPixels">peekPixels</a> | Returns <a href="SkPixmap_Reference#Pixmap">Pixmap</a> if possible. |
| <a href="#SkImage_readPixels">readPixels</a> | Copies and converts pixels. |
| <a href="#SkImage_refColorSpace">refColorSpace</a> | Returns <a href="#Info">Image Info</a> <a href="undocumented#Color_Space">Color Space</a>. |
| <a href="#SkImage_refEncodedData">refEncodedData</a> | Returns <a href="#Image">Image</a> encoded in <a href="undocumented#SkData">SkData</a> if present. |
| <a href="#SkImage_scalePixels">scalePixels</a> | Scales and converts one <a href="#Image">Image</a> to another. |
| <a href="#SkImage_toString">toString</a> | Converts <a href="#Image">Image</a> to machine readable form. |
| <a href="#SkImage_uniqueID">uniqueID</a> | Identifier for <a href="#Image">Image</a>. |
| <a href="#SkImage_width">width</a> | Returns pixel column count. |

<a name="SkImage_MakeRasterCopy"></a>
## MakeRasterCopy

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeRasterCopy(const SkPixmap& pixmap)
</pre>

Creates <a href="#Image">Image</a> from <a href="SkPixmap_Reference#Pixmap">Pixmap</a> and copy of pixels. Since pixels are copied, <a href="SkPixmap_Reference#Pixmap">Pixmap</a>
pixels may be modified or deleted without affecting <a href="#Image">Image</a>.

<a href="#Image">Image</a> is returned if <a href="SkPixmap_Reference#Pixmap">Pixmap</a> is valid. Valid <a href="SkPixmap_Reference#Pixmap">Pixmap</a> parameters include:
<a href="#SkImage_dimensions">dimensions</a> are greater than zero;
each dimension fits in 29 bits;
<a href="#Color_Type">Color Type</a> and <a href="#Alpha_Type">Alpha Type</a> are valid, and <a href="#Color_Type">Color Type</a> is not <a href="undocumented#SkColorType">kUnknown SkColorType</a>;
row bytes are large enough to hold one row of pixels;
pixel address is not nullptr.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeRasterCopy_pixmap"> <code><strong>pixmap </strong></code> </a></td> <td>
<a href="#Info">Image Info</a>, pixel address, and row bytes</td>
  </tr>
</table>

### Return Value

copy of <a href="SkPixmap_Reference#Pixmap">Pixmap</a> pixels, or nullptr

### Example

<div><fiddle-embed name="513afec5795a9504ebf6af5373d16b6b"><div>Draw a five by five bitmap, and draw a copy in an <a href="#Image">Image</a>. Editing the <a href="#SkImage_MakeRasterCopy_pixmap">pixmap</a>
alters the bitmap draw, but does not alter the <a href="#Image">Image</a> draw since the <a href="#Image">Image</a>
contains a copy of the pixels.</div></fiddle-embed></div>

### See Also

<a href="#SkImage_MakeRasterData">MakeRasterData</a> <a href="#SkImage_MakeFromGenerator">MakeFromGenerator</a>

---

<a name="SkImage_MakeRasterData"></a>
## MakeRasterData

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeRasterData(const Info& info, sk_sp&lt;SkData&gt; pixels, size_t rowBytes)
</pre>

Creates <a href="#Image">Image</a> from <a href="#Info">Image Info</a>, sharing <a href="#SkImage_MakeRasterData_pixels">pixels</a>.

<a href="#Image">Image</a> is returned if <a href="#Info">Image Info</a> is valid. Valid <a href="#Info">Image Info</a> parameters include:
<a href="#SkImage_dimensions">dimensions</a> are greater than zero;
each dimension fits in 29 bits;
<a href="#Color_Type">Color Type</a> and <a href="#Alpha_Type">Alpha Type</a> are valid, and <a href="#Color_Type">Color Type</a> is not <a href="undocumented#SkColorType">kUnknown SkColorType</a>;
<a href="#SkImage_MakeRasterData_rowBytes">rowBytes</a> are large enough to hold one row of <a href="#SkImage_MakeRasterData_pixels">pixels</a>;
<a href="#SkImage_MakeRasterData_pixels">pixels</a> is not nullptr, and contains enough data for <a href="#Image">Image</a>.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeRasterData_info"> <code><strong>info </strong></code> </a></td> <td>
contains <a href="#SkImage_width">width</a>, <a href="#SkImage_height">height</a>, <a href="#Alpha_Type">Alpha Type</a>, <a href="#Color_Type">Color Type</a>, <a href="undocumented#Color_Space">Color Space</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeRasterData_pixels"> <code><strong>pixels </strong></code> </a></td> <td>
address or pixel storage</td>
  </tr>  <tr>    <td><a name="SkImage_MakeRasterData_rowBytes"> <code><strong>rowBytes </strong></code> </a></td> <td>
size of pixel row or larger</td>
  </tr>
</table>

### Return Value

<a href="#Image">Image</a> sharing <a href="#SkImage_MakeRasterData_pixels">pixels</a>, or nullptr

### Example

<div><fiddle-embed name="367bdf6ee6ef2482eea95d4a9887c9b0"></fiddle-embed></div>

### See Also

<a href="#SkImage_MakeRasterCopy">MakeRasterCopy</a> <a href="#SkImage_MakeFromGenerator">MakeFromGenerator</a>

---

Caller data passed to <a href="#SkImage_RasterReleaseProc">RasterReleaseProc</a>; may be nullptr.

### See Also

<a href="#SkImage_MakeFromRaster">MakeFromRaster</a> <a href="#SkImage_RasterReleaseProc">RasterReleaseProc</a>

Function called when <a href="#Image">Image</a> no longer shares pixels. <a href="#SkImage_ReleaseContext">ReleaseContext</a> is
provided by caller when <a href="#Image">Image</a> is created, and may be nullptr.

### See Also

<a href="#SkImage_ReleaseContext">ReleaseContext</a> <a href="#SkImage_MakeFromRaster">MakeFromRaster</a>

<a name="SkImage_MakeFromRaster"></a>
## MakeFromRaster

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromRaster(const SkPixmap& pixmap, RasterReleaseProc rasterReleaseProc,
                                     ReleaseContext releaseContext)
</pre>

Creates <a href="#Image">Image</a> from <a href="#SkImage_MakeFromRaster_pixmap">pixmap</a>, sharing <a href="SkPixmap_Reference#Pixmap">Pixmap</a> pixels. Pixels must remain valid and
unchanged until <a href="#SkImage_MakeFromRaster_rasterReleaseProc">rasterReleaseProc</a> is called. <a href="#SkImage_MakeFromRaster_rasterReleaseProc">rasterReleaseProc</a> is passed
<a href="#SkImage_MakeFromRaster_releaseContext">releaseContext</a> when <a href="#Image">Image</a> is deleted or no longer refers to <a href="#SkImage_MakeFromRaster_pixmap">pixmap</a> pixels.

Pass nullptr for <a href="#SkImage_MakeFromRaster_rasterReleaseProc">rasterReleaseProc</a> to share <a href="SkPixmap_Reference#Pixmap">Pixmap</a> without requiring a callback
when <a href="#Image">Image</a> is released. Pass nullptr for <a href="#SkImage_MakeFromRaster_releaseContext">releaseContext</a> if <a href="#SkImage_MakeFromRaster_rasterReleaseProc">rasterReleaseProc</a>
does not require state.

<a href="#Image">Image</a> is returned if <a href="#SkImage_MakeFromRaster_pixmap">pixmap</a> is valid. Valid <a href="SkPixmap_Reference#Pixmap">Pixmap</a> parameters include:
<a href="#SkImage_dimensions">dimensions</a> are greater than zero;
each dimension fits in 29 bits;
<a href="#Color_Type">Color Type</a> and <a href="#Alpha_Type">Alpha Type</a> are valid, and <a href="#Color_Type">Color Type</a> is not <a href="undocumented#SkColorType">kUnknown SkColorType</a>;
row bytes are large enough to hold one row of pixels;
pixel address is not nullptr.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromRaster_pixmap"> <code><strong>pixmap </strong></code> </a></td> <td>
<a href="#Info">Image Info</a>, pixel address, and row bytes</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromRaster_rasterReleaseProc"> <code><strong>rasterReleaseProc </strong></code> </a></td> <td>
function called when pixels can be released; or nullptr</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromRaster_releaseContext"> <code><strong>releaseContext </strong></code> </a></td> <td>
state passed to <a href="#SkImage_MakeFromRaster_rasterReleaseProc">rasterReleaseProc</a>; or nullptr</td>
  </tr>
</table>

### Return Value

<a href="#Image">Image</a> sharing <a href="#SkImage_MakeFromRaster_pixmap">pixmap</a>

### Example

<div><fiddle-embed name="275356b65d18c8868f4434137350cddc">

#### Example Output

~~~~
before reset: 0
after reset: 1
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkImage_MakeRasterCopy">MakeRasterCopy</a> <a href="#SkImage_MakeRasterData">MakeRasterData</a> <a href="#SkImage_MakeFromGenerator">MakeFromGenerator</a> <a href="#SkImage_RasterReleaseProc">RasterReleaseProc</a> <a href="#SkImage_ReleaseContext">ReleaseContext</a>

---

<a name="SkImage_MakeFromBitmap"></a>
## MakeFromBitmap

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromBitmap(const SkBitmap& bitmap)
</pre>

Creates <a href="#Image">Image</a> from <a href="#SkImage_MakeFromBitmap_bitmap">bitmap</a>, sharing or copying <a href="#SkImage_MakeFromBitmap_bitmap">bitmap</a> pixels. If the <a href="#SkImage_MakeFromBitmap_bitmap">bitmap</a>
is marked immutable, and its pixel memory is shareable, it may be shared
instead of copied.

<a href="#Image">Image</a> is returned if <a href="#SkImage_MakeFromBitmap_bitmap">bitmap</a> is valid. Valid <a href="SkBitmap_Reference#Bitmap">Bitmap</a> parameters include:
<a href="#SkImage_dimensions">dimensions</a> are greater than zero;
each dimension fits in 29 bits;
<a href="#Color_Type">Color Type</a> and <a href="#Alpha_Type">Alpha Type</a> are valid, and <a href="#Color_Type">Color Type</a> is not <a href="undocumented#SkColorType">kUnknown SkColorType</a>;
row bytes are large enough to hold one row of pixels;
pixel address is not nullptr.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromBitmap_bitmap"> <code><strong>bitmap </strong></code> </a></td> <td>
<a href="#Info">Image Info</a>, row bytes, and pixels</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="cf2cf53321e4e6a77c2841bfbc0ef707"><div>The first <a href="SkBitmap_Reference#Bitmap">Bitmap</a> is shared; writing to the pixel memory changes the first
<a href="#Image">Image</a>.
The second <a href="SkBitmap_Reference#Bitmap">Bitmap</a> is marked immutable, and is copied; writing to the pixel
memory does not alter the second <a href="#Image">Image</a>.</div></fiddle-embed></div>

### See Also

<a href="#SkImage_MakeFromRaster">MakeFromRaster</a> <a href="#SkImage_MakeRasterCopy">MakeRasterCopy</a> <a href="#SkImage_MakeFromGenerator">MakeFromGenerator</a> <a href="#SkImage_MakeRasterData">MakeRasterData</a>

---

<a name="SkImage_MakeFromGenerator"></a>
## MakeFromGenerator

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromGenerator(std::unique_ptr&lt;SkImageGenerator&gt; imageGenerator,
                                 const SkIRect* subset = nullptr)
</pre>

Creates <a href="#Image">Image</a> from data returned by <a href="#SkImage_MakeFromGenerator_imageGenerator">imageGenerator</a>. Generated data is owned by <a href="#Image">Image</a> and may not
be shared or accessed.

<a href="#SkImage_MakeFromGenerator_subset">subset</a> allows selecting a portion of the full image. Pass nullptr to select the entire image;
otherwise, <a href="#SkImage_MakeFromGenerator_subset">subset</a> must be contained by image <a href="#SkImage_bounds">bounds</a>.

<a href="#Image">Image</a> is returned if generator data is valid. Valid data parameters vary by type of data
and platform.

<a href="#SkImage_MakeFromGenerator_imageGenerator">imageGenerator</a> may wrap <a href="undocumented#Picture">Picture</a> data, codec data, or custom data.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromGenerator_imageGenerator"> <code><strong>imageGenerator </strong></code> </a></td> <td>
stock or custom routines to retrieve <a href="#Image">Image</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromGenerator_subset"> <code><strong>subset </strong></code> </a></td> <td>
<a href="#SkImage_bounds">bounds</a> of returned <a href="#Image">Image</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="ba59d292a18cb0e8a90e1bb143115f1d"><div>The generator returning <a href="undocumented#Picture">Picture</a> cannot be shared; std::move transfers ownership to generated <a href="#Image">Image</a>.</div></fiddle-embed></div>

### See Also

<a href="#SkImage_MakeFromEncoded">MakeFromEncoded</a>

---

<a name="SkImage_MakeFromEncoded"></a>
## MakeFromEncoded

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromEncoded(sk_sp&lt;SkData&gt; encoded, const SkIRect* subset = nullptr)
</pre>

Creates <a href="#Image">Image</a> from <a href="#SkImage_MakeFromEncoded_encoded">encoded</a> data.
<a href="#SkImage_MakeFromEncoded_subset">subset</a> allows selecting a portion of the full image. Pass nullptr to select the entire image;
otherwise, <a href="#SkImage_MakeFromEncoded_subset">subset</a> must be contained by image <a href="#SkImage_bounds">bounds</a>.

<a href="#Image">Image</a> is returned if format of the <a href="#SkImage_MakeFromEncoded_encoded">encoded</a> data is recognized and supported.
Recognized formats vary by platfrom.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromEncoded_encoded"> <code><strong>encoded </strong></code> </a></td> <td>
data of <a href="#Image">Image</a> to decode</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromEncoded_subset"> <code><strong>subset </strong></code> </a></td> <td>
<a href="#SkImage_bounds">bounds</a> of returned <a href="#Image">Image</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="894f732ed6409b1f392bc5481421d0e9"></fiddle-embed></div>

### See Also

<a href="#SkImage_MakeFromGenerator">MakeFromGenerator</a>

---

<a name="SkImage_MakeFromTexture"></a>
## MakeFromTexture

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromTexture(GrContext* context, const GrBackendTexture& backendTexture,
                                      GrSurfaceOrigin origin, SkAlphaType alphaType,
                                      sk_sp&lt;SkColorSpace&gt; colorSpace)
</pre>

Deprecated.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromTexture_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_backendTexture"> <code><strong>backendTexture </strong></code> </a></td> <td>
texture residing on GPU</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_origin"> <code><strong>origin </strong></code> </a></td> <td>
one of: <a href="undocumented#GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>, <a href="undocumented#GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_alphaType"> <code><strong>alphaType </strong></code> </a></td> <td>
one of: <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>, <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>,
<a href="undocumented#SkAlphaType">kPremul SkAlphaType</a>, <a href="undocumented#SkAlphaType">kUnpremul SkAlphaType</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### See Also

<a href="#SkImage_MakeFromAdoptedTexture">MakeFromAdoptedTexture</a> <a href="#SkSurface_MakeFromBackendTexture">SkSurface::MakeFromBackendTexture</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromTexture(GrContext* context, const GrBackendTexture& backendTexture,
                                      GrSurfaceOrigin origin, SkAlphaType alphaType,
                                      sk_sp&lt;SkColorSpace&gt; colorSpace,
                                      TextureReleaseProc textureReleaseProc,
                                      ReleaseContext releaseContext)
</pre>

Deprecated.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromTexture_2_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_2_backendTexture"> <code><strong>backendTexture </strong></code> </a></td> <td>
texture residing on GPU</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_2_origin"> <code><strong>origin </strong></code> </a></td> <td>
one of: <a href="undocumented#GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>, <a href="undocumented#GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_2_alphaType"> <code><strong>alphaType </strong></code> </a></td> <td>
one of: <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>, <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>,
<a href="undocumented#SkAlphaType">kPremul SkAlphaType</a>, <a href="undocumented#SkAlphaType">kUnpremul SkAlphaType</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_2_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors; may be nullptr</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_2_textureReleaseProc"> <code><strong>textureReleaseProc </strong></code> </a></td> <td>
function called when texture can be released</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_2_releaseContext"> <code><strong>releaseContext </strong></code> </a></td> <td>
state passed to <a href="#SkImage_MakeFromTexture_2_textureReleaseProc">textureReleaseProc</a></td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### See Also

<a href="#SkImage_MakeFromAdoptedTexture">MakeFromAdoptedTexture</a> <a href="#SkSurface_MakeFromBackendTexture">SkSurface::MakeFromBackendTexture</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromTexture(GrContext* context, const GrBackendTexture& backendTexture,
                                      GrSurfaceOrigin origin, SkColorType colorType,
                                      SkAlphaType alphaType, sk_sp&lt;SkColorSpace&gt; colorSpace)
</pre>

Creates <a href="#Image">Image</a> from <a href="undocumented#GPU_Texture">GPU Texture</a> associated with <a href="#SkImage_MakeFromTexture_3_context">context</a>. Caller is responsible for
managing the lifetime of <a href="undocumented#GPU_Texture">GPU Texture</a>.

<a href="#Image">Image</a> is returned if format of <a href="#SkImage_MakeFromTexture_3_backendTexture">backendTexture</a> is recognized and supported.
Recognized formats vary by GPU back-end.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromTexture_3_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_3_backendTexture"> <code><strong>backendTexture </strong></code> </a></td> <td>
texture residing on GPU</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_3_origin"> <code><strong>origin </strong></code> </a></td> <td>
one of: <a href="undocumented#GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>, <a href="undocumented#GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_3_colorType"> <code><strong>colorType </strong></code> </a></td> <td>
one of: <a href="undocumented#SkColorType">kUnknown SkColorType</a>, <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>,
<a href="undocumented#SkColorType">kRGB 565 SkColorType</a>, <a href="undocumented#SkColorType">kARGB 4444 SkColorType</a>,
<a href="undocumented#SkColorType">kRGBA 8888 SkColorType</a>, <a href="undocumented#SkColorType">kBGRA 8888 SkColorType</a>,
<a href="undocumented#SkColorType">kGray 8 SkColorType</a>, <a href="undocumented#SkColorType">kRGBA F16 SkColorType</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_3_alphaType"> <code><strong>alphaType </strong></code> </a></td> <td>
one of: <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>, <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>,
<a href="undocumented#SkAlphaType">kPremul SkAlphaType</a>, <a href="undocumented#SkAlphaType">kUnpremul SkAlphaType</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_3_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="2faa98d6a1d578010326af17ee7e4d2a" gpu="true"><div>A back-end texture has been created and uploaded to the GPU outside of this example.</div></fiddle-embed></div>

### See Also

<a href="#SkImage_MakeFromAdoptedTexture">MakeFromAdoptedTexture</a> <a href="#SkSurface_MakeFromBackendTexture">SkSurface::MakeFromBackendTexture</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromTexture(GrContext* context, const GrBackendTexture& backendTexture,
                                      GrSurfaceOrigin origin, SkColorType colorType,
                                      SkAlphaType alphaType, sk_sp&lt;SkColorSpace&gt; colorSpace,
                                      TextureReleaseProc textureReleaseProc,
                                      ReleaseContext releaseContext)
</pre>

Creates <a href="#Image">Image</a> from <a href="undocumented#GPU_Texture">GPU Texture</a> associated with <a href="#SkImage_MakeFromTexture_4_context">context</a>. <a href="undocumented#GPU_Texture">GPU Texture</a> must stay
valid and unchanged until <a href="#SkImage_MakeFromTexture_4_textureReleaseProc">textureReleaseProc</a> is called. <a href="#SkImage_MakeFromTexture_4_textureReleaseProc">textureReleaseProc</a> is
passed <a href="#SkImage_MakeFromTexture_4_releaseContext">releaseContext</a> when <a href="#Image">Image</a> is deleted or no longer refers to texture.

<a href="#Image">Image</a> is returned if format of <a href="#SkImage_MakeFromTexture_4_backendTexture">backendTexture</a> is recognized and supported.
Recognized formats vary by GPU back-end.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromTexture_4_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_4_backendTexture"> <code><strong>backendTexture </strong></code> </a></td> <td>
texture residing on GPU</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_4_origin"> <code><strong>origin </strong></code> </a></td> <td>
one of: <a href="undocumented#GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>, <a href="undocumented#GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_4_colorType"> <code><strong>colorType </strong></code> </a></td> <td>
one of: <a href="undocumented#SkColorType">kUnknown SkColorType</a>, <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>,
<a href="undocumented#SkColorType">kRGB 565 SkColorType</a>, <a href="undocumented#SkColorType">kARGB 4444 SkColorType</a>,
<a href="undocumented#SkColorType">kRGBA 8888 SkColorType</a>, <a href="undocumented#SkColorType">kBGRA 8888 SkColorType</a>,
<a href="undocumented#SkColorType">kGray 8 SkColorType</a>, <a href="undocumented#SkColorType">kRGBA F16 SkColorType</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_4_alphaType"> <code><strong>alphaType </strong></code> </a></td> <td>
one of: <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>, <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>,
<a href="undocumented#SkAlphaType">kPremul SkAlphaType</a>, <a href="undocumented#SkAlphaType">kUnpremul SkAlphaType</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_4_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors; may be nullptr</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_4_textureReleaseProc"> <code><strong>textureReleaseProc </strong></code> </a></td> <td>
function called when texture can be released</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_4_releaseContext"> <code><strong>releaseContext </strong></code> </a></td> <td>
state passed to <a href="#SkImage_MakeFromTexture_4_textureReleaseProc">textureReleaseProc</a></td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="6a50c3cb961c23ad9ecbd5caedac0d70"></fiddle-embed></div>

### See Also

<a href="#SkImage_MakeFromAdoptedTexture">MakeFromAdoptedTexture</a> <a href="#SkSurface_MakeFromBackendTexture">SkSurface::MakeFromBackendTexture</a>

---

<a name="SkImage_MakeCrossContextFromEncoded"></a>
## MakeCrossContextFromEncoded

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeCrossContextFromEncoded(GrContext* context, sk_sp&lt;SkData&gt; data,
                                                  bool buildMips, SkColorSpace* dstColorSpace)
</pre>

Creates <a href="#Image">Image</a> from encoded <a href="#SkImage_MakeCrossContextFromEncoded_data">data</a>. <a href="#Image">Image</a> is uploaded to GPU back-end using <a href="#SkImage_MakeCrossContextFromEncoded_context">context</a>.

Created <a href="#Image">Image</a> is available to other GPU contexts, and is available across thread
boundaries. All contexts must be in the same GPU_Share_Group, or otherwise
share resources.

When <a href="#Image">Image</a> is no longer referenced, <a href="#SkImage_MakeCrossContextFromEncoded_context">context</a> releases texture memory
asynchronously.

<a href="undocumented#Texture">Texture</a> decoded from <a href="#SkImage_MakeCrossContextFromEncoded_data">data</a> is uploaded to match <a href="SkSurface_Reference#Surface">Surface</a> created with
<a href="#SkImage_MakeCrossContextFromEncoded_dstColorSpace">dstColorSpace</a>. <a href="undocumented#Color_Space">Color Space</a> of <a href="#Image">Image</a> is determined by encoded <a href="#SkImage_MakeCrossContextFromEncoded_data">data</a>.

<a href="#Image">Image</a> is returned if format of <a href="#SkImage_MakeCrossContextFromEncoded_data">data</a> is recognized and supported, and if <a href="#SkImage_MakeCrossContextFromEncoded_context">context</a>
supports moving resources. Recognized formats vary by platform and GPU back-end.

<a href="#Image">Image</a> is returned using <a href="#SkImage_MakeFromEncoded">MakeFromEncoded</a> if <a href="#SkImage_MakeCrossContextFromEncoded_context">context</a> is nullptr or does not support
moving resources between contexts.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeCrossContextFromEncoded_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeCrossContextFromEncoded_data"> <code><strong>data </strong></code> </a></td> <td>
<a href="#Image">Image</a> to decode</td>
  </tr>  <tr>    <td><a name="SkImage_MakeCrossContextFromEncoded_buildMips"> <code><strong>buildMips </strong></code> </a></td> <td>
create <a href="#Image">Image</a> as <a href="undocumented#Mip_Map">Mip Map</a> if true</td>
  </tr>  <tr>    <td><a name="SkImage_MakeCrossContextFromEncoded_dstColorSpace"> <code><strong>dstColorSpace </strong></code> </a></td> <td>
range of colors of matching <a href="SkSurface_Reference#Surface">Surface</a> on GPU</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="90202d8b1df605fdb0c84f2c351f9598"></fiddle-embed></div>

### See Also

<a href="#SkImage_MakeCrossContextFromPixmap">MakeCrossContextFromPixmap</a>

---

<a name="SkImage_MakeCrossContextFromPixmap"></a>
## MakeCrossContextFromPixmap

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeCrossContextFromPixmap(GrContext* context, const SkPixmap& pixmap,
                                                 bool buildMips, SkColorSpace* dstColorSpace)
</pre>

Creates <a href="#Image">Image</a> from <a href="#SkImage_MakeCrossContextFromPixmap_pixmap">pixmap</a>. <a href="#Image">Image</a> is uploaded to GPU back-end using <a href="#SkImage_MakeCrossContextFromPixmap_context">context</a>.

Created <a href="#Image">Image</a> is available to other GPU contexts, and is available across thread
boundaries. All contexts must be in the same GPU_Share_Group, or otherwise
share resources.

When <a href="#Image">Image</a> is no longer referenced, <a href="#SkImage_MakeCrossContextFromPixmap_context">context</a> releases texture memory
asynchronously.

<a href="undocumented#Texture">Texture</a> created from <a href="#SkImage_MakeCrossContextFromPixmap_pixmap">pixmap</a> is uploaded to match <a href="SkSurface_Reference#Surface">Surface</a> created with
<a href="#SkImage_MakeCrossContextFromPixmap_dstColorSpace">dstColorSpace</a>. <a href="undocumented#Color_Space">Color Space</a> of <a href="#Image">Image</a> is determined by <a href="#SkImage_MakeCrossContextFromPixmap_pixmap">pixmap</a>.<a href="#SkImage_colorSpace">colorSpace</a>.

<a href="#Image">Image</a> is returned referring to GPU back-end if <a href="#SkImage_MakeCrossContextFromPixmap_context">context</a> is not nullptr,
format of data is recognized and supported, and if <a href="#SkImage_MakeCrossContextFromPixmap_context">context</a> supports moving
resources between contexts. Otherwise, <a href="#SkImage_MakeCrossContextFromPixmap_pixmap">pixmap</a> pixel data is copied and <a href="#Image">Image</a>
as returned in raster format if possible; nullptr may be returned.
Recognized GPU formats vary by platform and GPU back-end.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeCrossContextFromPixmap_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeCrossContextFromPixmap_pixmap"> <code><strong>pixmap </strong></code> </a></td> <td>
<a href="#Info">Image Info</a>, pixel address, and row bytes</td>
  </tr>  <tr>    <td><a name="SkImage_MakeCrossContextFromPixmap_buildMips"> <code><strong>buildMips </strong></code> </a></td> <td>
create <a href="#Image">Image</a> as <a href="undocumented#Mip_Map">Mip Map</a> if true</td>
  </tr>  <tr>    <td><a name="SkImage_MakeCrossContextFromPixmap_dstColorSpace"> <code><strong>dstColorSpace </strong></code> </a></td> <td>
range of colors of matching <a href="SkSurface_Reference#Surface">Surface</a> on GPU</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="bb37de3dc4a0c53d48cdfafb90b9b18e"></fiddle-embed></div>

### See Also

<a href="#SkImage_MakeCrossContextFromEncoded">MakeCrossContextFromEncoded</a>

---

<a name="SkImage_MakeFromAdoptedTexture"></a>
## MakeFromAdoptedTexture

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromAdoptedTexture(GrContext* context,
                                             const GrBackendTexture& backendTexture,
                                             GrSurfaceOrigin surfaceOrigin,
                                             SkAlphaType alphaType = kPremul_SkAlphaType,
                                             sk_sp&lt;SkColorSpace&gt; colorSpace = nullptr)
</pre>

Deprecated.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromAdoptedTexture_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAdoptedTexture_backendTexture"> <code><strong>backendTexture </strong></code> </a></td> <td>
texture residing on GPU</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAdoptedTexture_surfaceOrigin"> <code><strong>surfaceOrigin </strong></code> </a></td> <td>
one of: <a href="undocumented#GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>, <a href="undocumented#GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAdoptedTexture_alphaType"> <code><strong>alphaType </strong></code> </a></td> <td>
one of: <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>, <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>,
<a href="undocumented#SkAlphaType">kPremul SkAlphaType</a>, <a href="undocumented#SkAlphaType">kUnpremul SkAlphaType</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAdoptedTexture_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### See Also

<a href="#SkImage_MakeFromTexture">MakeFromTexture</a> <a href="#SkImage_MakeFromYUVTexturesCopy">MakeFromYUVTexturesCopy</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromAdoptedTexture(GrContext* context,
                                             const GrBackendTexture& backendTexture,
                                             GrSurfaceOrigin surfaceOrigin, SkColorType colorType,
                                             SkAlphaType alphaType = kPremul_SkAlphaType,
                                             sk_sp&lt;SkColorSpace&gt; colorSpace = nullptr)
</pre>

Creates <a href="#Image">Image</a> from <a href="#SkImage_MakeFromAdoptedTexture_2_backendTexture">backendTexture</a> associated with <a href="#SkImage_MakeFromAdoptedTexture_2_context">context</a>. <a href="#SkImage_MakeFromAdoptedTexture_2_backendTexture">backendTexture</a> and
returned <a href="#Image">Image</a> are managed internally, and are released when no longer needed.

<a href="#Image">Image</a> is returned if format of <a href="#SkImage_MakeFromAdoptedTexture_2_backendTexture">backendTexture</a> is recognized and supported.
Recognized formats vary by GPU back-end.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromAdoptedTexture_2_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAdoptedTexture_2_backendTexture"> <code><strong>backendTexture </strong></code> </a></td> <td>
texture residing on GPU</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAdoptedTexture_2_surfaceOrigin"> <code><strong>surfaceOrigin </strong></code> </a></td> <td>
one of: <a href="undocumented#GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>, <a href="undocumented#GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAdoptedTexture_2_colorType"> <code><strong>colorType </strong></code> </a></td> <td>
one of: <a href="undocumented#SkColorType">kUnknown SkColorType</a>, <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>,
<a href="undocumented#SkColorType">kRGB 565 SkColorType</a>, <a href="undocumented#SkColorType">kARGB 4444 SkColorType</a>,
<a href="undocumented#SkColorType">kRGBA 8888 SkColorType</a>, <a href="undocumented#SkColorType">kBGRA 8888 SkColorType</a>,
<a href="undocumented#SkColorType">kGray 8 SkColorType</a>, <a href="undocumented#SkColorType">kRGBA F16 SkColorType</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAdoptedTexture_2_alphaType"> <code><strong>alphaType </strong></code> </a></td> <td>
one of: <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>, <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>,
<a href="undocumented#SkAlphaType">kPremul SkAlphaType</a>, <a href="undocumented#SkAlphaType">kUnpremul SkAlphaType</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAdoptedTexture_2_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="aed9b621fdc2135d512b58648d064224"></fiddle-embed></div>

### See Also

<a href="#SkImage_MakeFromTexture">MakeFromTexture</a> <a href="#SkImage_MakeFromYUVTexturesCopy">MakeFromYUVTexturesCopy</a>

---

<a name="SkImage_MakeFromYUVTexturesCopy"></a>
## MakeFromYUVTexturesCopy

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromYUVTexturesCopy(GrContext* context, SkYUVColorSpace yuvColorSpace,
                                              const GrBackendObject yuvTextureHandles[3],
                                              const SkISize yuvSizes[3],
                                              GrSurfaceOrigin surfaceOrigin,
                                              sk_sp&lt;SkColorSpace&gt; colorSpace = nullptr)
</pre>

Creates <a href="#Image">Image</a> from copy of <a href="#SkImage_MakeFromYUVTexturesCopy_yuvTextureHandles">yuvTextureHandles</a>, an array of textures on GPU.
<a href="#SkImage_MakeFromYUVTexturesCopy_yuvTextureHandles">yuvTextureHandles</a> contain pixels for YUV planes of <a href="#Image">Image</a>.
<a href="#SkImage_MakeFromYUVTexturesCopy_yuvSizes">yuvSizes</a> conain <a href="#SkImage_dimensions">dimensions</a> for each pixel plane. Dimensions must be greater than
zero but may differ from plane to plane. Returned <a href="#Image">Image</a> has the <a href="#SkImage_dimensions">dimensions</a>
<a href="#SkImage_MakeFromYUVTexturesCopy_yuvSizes">yuvSizes</a>[0]. <a href="#SkImage_MakeFromYUVTexturesCopy_yuvColorSpace">yuvColorSpace</a> describes how YUV colors convert to RGB colors.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_yuvColorSpace"> <code><strong>yuvColorSpace </strong></code> </a></td> <td>
one of: <a href="undocumented#YUV_ColorSpace">kJPEG SkYUVColorSpace</a>, <a href="undocumented#YUV_ColorSpace">kRec601 SkYUVColorSpace</a>,
<a href="undocumented#YUV_ColorSpace">kRec709 SkYUVColorSpace</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_yuvTextureHandles"> <code><strong>yuvTextureHandles </strong></code> </a></td> <td>
array of YUV textures on GPU</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_yuvSizes"> <code><strong>yuvSizes </strong></code> </a></td> <td>
<a href="#SkImage_dimensions">dimensions</a> of YUV textures</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_surfaceOrigin"> <code><strong>surfaceOrigin </strong></code> </a></td> <td>
one of: <a href="undocumented#GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>, <a href="undocumented#GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### See Also

<a href="#SkImage_MakeFromNV12TexturesCopy">MakeFromNV12TexturesCopy</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromYUVTexturesCopy(GrContext* context, SkYUVColorSpace yuvColorSpace,
                                              const GrBackendTexture yuvTextureHandles[3],
                                              const SkISize yuvSizes[3],
                                              GrSurfaceOrigin surfaceOrigin,
                                              sk_sp&lt;SkColorSpace&gt; colorSpace = nullptr)
</pre>

Creates <a href="#Image">Image</a> from copy of <a href="#SkImage_MakeFromYUVTexturesCopy_2_yuvTextureHandles">yuvTextureHandles</a>, an array of textures on GPU.
<a href="#SkImage_MakeFromYUVTexturesCopy_2_yuvTextureHandles">yuvTextureHandles</a> contain pixels for YUV planes of <a href="#Image">Image</a>.
<a href="#SkImage_MakeFromYUVTexturesCopy_2_yuvSizes">yuvSizes</a> conain <a href="#SkImage_dimensions">dimensions</a> for each pixel plane. Dimensions must be greater than
zero but may differ from plane to plane. Returned <a href="#Image">Image</a> has the <a href="#SkImage_dimensions">dimensions</a>
<a href="#SkImage_MakeFromYUVTexturesCopy_2_yuvSizes">yuvSizes</a>[0]. <a href="#SkImage_MakeFromYUVTexturesCopy_2_yuvColorSpace">yuvColorSpace</a> describes how YUV colors convert to RGB colors.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_2_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_2_yuvColorSpace"> <code><strong>yuvColorSpace </strong></code> </a></td> <td>
one of: <a href="undocumented#YUV_ColorSpace">kJPEG SkYUVColorSpace</a>, <a href="undocumented#YUV_ColorSpace">kRec601 SkYUVColorSpace</a>,
<a href="undocumented#YUV_ColorSpace">kRec709 SkYUVColorSpace</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_2_yuvTextureHandles"> <code><strong>yuvTextureHandles </strong></code> </a></td> <td>
array of YUV textures on GPU</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_2_yuvSizes"> <code><strong>yuvSizes </strong></code> </a></td> <td>
<a href="#SkImage_dimensions">dimensions</a> of YUV textures</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_2_surfaceOrigin"> <code><strong>surfaceOrigin </strong></code> </a></td> <td>
one of: <a href="undocumented#GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>, <a href="undocumented#GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_2_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### See Also

<a href="#SkImage_MakeFromNV12TexturesCopy">MakeFromNV12TexturesCopy</a>

---

<a name="SkImage_MakeFromNV12TexturesCopy"></a>
## MakeFromNV12TexturesCopy

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromNV12TexturesCopy(GrContext* context, SkYUVColorSpace yuvColorSpace,
                                               const GrBackendObject nv12TextureHandles[2],
                                               const SkISize nv12Sizes[2],
                                               GrSurfaceOrigin surfaceOrigin,
                                               sk_sp&lt;SkColorSpace&gt; colorSpace = nullptr)
</pre>

Creates <a href="#Image">Image</a> from copy of <a href="#SkImage_MakeFromNV12TexturesCopy_nv12TextureHandles">nv12TextureHandles</a>, an array of textures on GPU.
<a href="#SkImage_MakeFromNV12TexturesCopy_nv12TextureHandles">nv12TextureHandles</a>[0] contains pixels for YUV_Component_Y plane.
<a href="#SkImage_MakeFromNV12TexturesCopy_nv12TextureHandles">nv12TextureHandles</a>[1] contains pixels for YUV_Component_U plane,
followed by pixels for YUV_Component_V plane.
<a href="#SkImage_MakeFromNV12TexturesCopy_nv12Sizes">nv12Sizes</a> conain <a href="#SkImage_dimensions">dimensions</a> for each pixel plane. Dimensions must be greater than
zero but may differ from plane to plane. Returned <a href="#Image">Image</a> has the <a href="#SkImage_dimensions">dimensions</a>
<a href="#SkImage_MakeFromNV12TexturesCopy_nv12Sizes">nv12Sizes</a>[0]. <a href="#SkImage_MakeFromNV12TexturesCopy_yuvColorSpace">yuvColorSpace</a> describes how YUV colors convert to RGB colors.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_yuvColorSpace"> <code><strong>yuvColorSpace </strong></code> </a></td> <td>
one of: <a href="undocumented#YUV_ColorSpace">kJPEG SkYUVColorSpace</a>, <a href="undocumented#YUV_ColorSpace">kRec601 SkYUVColorSpace</a>,
<a href="undocumented#YUV_ColorSpace">kRec709 SkYUVColorSpace</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_nv12TextureHandles"> <code><strong>nv12TextureHandles </strong></code> </a></td> <td>
array of YUV textures on GPU</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_nv12Sizes"> <code><strong>nv12Sizes </strong></code> </a></td> <td>
<a href="#SkImage_dimensions">dimensions</a> of YUV textures</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_surfaceOrigin"> <code><strong>surfaceOrigin </strong></code> </a></td> <td>
one of: <a href="undocumented#GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>, <a href="undocumented#GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### See Also

<a href="#SkImage_MakeFromYUVTexturesCopy">MakeFromYUVTexturesCopy</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromNV12TexturesCopy(GrContext* context, SkYUVColorSpace yuvColorSpace,
                                               const GrBackendTexture nv12TextureHandles[2],
                                               const SkISize nv12Sizes[2],
                                               GrSurfaceOrigin surfaceOrigin,
                                               sk_sp&lt;SkColorSpace&gt; colorSpace = nullptr)
</pre>

Creates <a href="#Image">Image</a> from copy of <a href="#SkImage_MakeFromNV12TexturesCopy_2_nv12TextureHandles">nv12TextureHandles</a>, an array of textures on GPU.
<a href="#SkImage_MakeFromNV12TexturesCopy_2_nv12TextureHandles">nv12TextureHandles</a>[0] contains pixels for YUV_Component_Y plane.
<a href="#SkImage_MakeFromNV12TexturesCopy_2_nv12TextureHandles">nv12TextureHandles</a>[1] contains pixels for YUV_Component_U plane,
followed by pixels for YUV_Component_V plane.
<a href="#SkImage_MakeFromNV12TexturesCopy_2_nv12Sizes">nv12Sizes</a> conain <a href="#SkImage_dimensions">dimensions</a> for each pixel plane. Dimensions must be greater than
zero but may differ from plane to plane. Returned <a href="#Image">Image</a> has the <a href="#SkImage_dimensions">dimensions</a>
<a href="#SkImage_MakeFromNV12TexturesCopy_2_nv12Sizes">nv12Sizes</a>[0]. <a href="#SkImage_MakeFromNV12TexturesCopy_2_yuvColorSpace">yuvColorSpace</a> describes how YUV colors convert to RGB colors.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_2_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_2_yuvColorSpace"> <code><strong>yuvColorSpace </strong></code> </a></td> <td>
one of: <a href="undocumented#YUV_ColorSpace">kJPEG SkYUVColorSpace</a>, <a href="undocumented#YUV_ColorSpace">kRec601 SkYUVColorSpace</a>,
<a href="undocumented#YUV_ColorSpace">kRec709 SkYUVColorSpace</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_2_nv12TextureHandles"> <code><strong>nv12TextureHandles </strong></code> </a></td> <td>
array of YUV textures on GPU</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_2_nv12Sizes"> <code><strong>nv12Sizes </strong></code> </a></td> <td>
<a href="#SkImage_dimensions">dimensions</a> of YUV textures</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_2_surfaceOrigin"> <code><strong>surfaceOrigin </strong></code> </a></td> <td>
one of: <a href="undocumented#GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>, <a href="undocumented#GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_2_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### See Also

<a href="#SkImage_MakeFromYUVTexturesCopy">MakeFromYUVTexturesCopy</a>

---

## <a name="SkImage_BitDepth"></a> Enum SkImage::BitDepth

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum class <a href="#SkImage_BitDepth">BitDepth</a> {
<a href="#SkImage_kU8">kU8</a>,
<a href="#SkImage_kF16">kF16</a>,
};</pre>

### Constants

<table>
  <tr>
    <td><a name="SkImage_kU8"> <code><strong>SkImage::kU8 </strong></code> </a></td><td>0</td><td>Use 8 bits per <a href="#ARGB">Color ARGB</a> component using unsigned integer format.</td>
  </tr>
  <tr>
    <td><a name="SkImage_kF16"> <code><strong>SkImage::kF16 </strong></code> </a></td><td>1</td><td>Use 16 bits per <a href="#ARGB">Color ARGB</a> component using half-precision floating point format.</td>
  </tr>
</table>

### See Also

<a href="#SkImage_MakeFromPicture">MakeFromPicture</a>



<a name="SkImage_MakeFromPicture"></a>
## MakeFromPicture

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromPicture(sk_sp&lt;SkPicture&gt; picture, const SkISize& dimensions,
                                      const SkMatrix* matrix, const SkPaint* paint,
                                      BitDepth bitDepth, sk_sp&lt;SkColorSpace&gt; colorSpace)
</pre>

Creates <a href="#Image">Image</a> from <a href="#SkImage_MakeFromPicture_picture">picture</a>. Returned <a href="#Image">Image</a> <a href="#SkImage_width">width</a> and <a href="#SkImage_height">height</a> are set by <a href="#SkImage_dimensions">dimensions</a>.
<a href="#Image">Image</a> draws <a href="#SkImage_MakeFromPicture_picture">picture</a> with <a href="#SkImage_MakeFromPicture_matrix">matrix</a> and <a href="#SkImage_MakeFromPicture_paint">paint</a>, set to <a href="#SkImage_MakeFromPicture_bitDepth">bitDepth</a> and <a href="#SkImage_colorSpace">colorSpace</a>.

If <a href="#SkImage_MakeFromPicture_matrix">matrix</a> is nullptr, draws with identity <a href="SkMatrix_Reference#Matrix">Matrix</a>. If <a href="#SkImage_MakeFromPicture_paint">paint</a> is nullptr, draws
with default <a href="SkPaint_Reference#Paint">Paint</a>. <a href="#SkImage_colorSpace">colorSpace</a> may be nullptr.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromPicture_picture"> <code><strong>picture </strong></code> </a></td> <td>
stream of drawing commands</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromPicture_dimensions"> <code><strong>dimensions </strong></code> </a></td> <td>
<a href="#SkImage_width">width</a> and <a href="#SkImage_height">height</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromPicture_matrix"> <code><strong>matrix </strong></code> </a></td> <td>
<a href="SkMatrix_Reference#Matrix">Matrix</a> to rotate, scale, translate, and so on; may be nullptr</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromPicture_paint"> <code><strong>paint </strong></code> </a></td> <td>
<a href="SkPaint_Reference#Paint">Paint</a> to apply transparency, filtering, and so on; may be nullptr</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromPicture_bitDepth"> <code><strong>bitDepth </strong></code> </a></td> <td>
8 bit integer or 16 bit float: per component</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromPicture_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="4aa2879b9e44dfd6648995326d2c4dcf"></fiddle-embed></div>

### See Also

<a href="#SkCanvas_drawPicture">SkCanvas::drawPicture</a>

---

<a name="SkImage_MakeFromAHardwareBuffer"></a>
## MakeFromAHardwareBuffer

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromAHardwareBuffer(AHardwareBuffer* hardwareBuffer,
                                            SkAlphaType alphaType = kPremul_SkAlphaType,
                                            sk_sp&lt;SkColorSpace&gt; colorSpace = nullptr)
</pre>

Creates <a href="#Image">Image</a> from Android hardware buffer.
Returned <a href="#Image">Image</a> takes a reference on the buffer.

Only available on Android, when __ANDROID_API__ is defined to be 26 or greater.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromAHardwareBuffer_hardwareBuffer"> <code><strong>hardwareBuffer </strong></code> </a></td> <td>
AHardwareBuffer Android hardware buffer</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAHardwareBuffer_alphaType"> <code><strong>alphaType </strong></code> </a></td> <td>
one of: <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>, <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>,
<a href="undocumented#SkAlphaType">kPremul SkAlphaType</a>, <a href="undocumented#SkAlphaType">kUnpremul SkAlphaType</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAHardwareBuffer_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### See Also

<a href="#SkImage_MakeFromRaster">MakeFromRaster</a>

---

<a name="SkImage_width"></a>
## width

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int width() const
</pre>

Returns pixel count in each row.

### Return Value

pixel <a href="#SkImage_width">width</a> in <a href="#Image">Image</a>

### Example

<div><fiddle-embed name="4fa6c18eb829d8979cfdbbf6f42e4c97"></fiddle-embed></div>

### See Also

<a href="#SkImage_dimensions">dimensions</a> <a href="#SkImage_height">height</a>

---

<a name="SkImage_height"></a>
## height

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int height() const
</pre>

Returns pixel row count.

### Return Value

pixel <a href="#SkImage_height">height</a> in <a href="#Image">Image</a>

### Example

<div><fiddle-embed name="96c8202a13068e36432006f75b124eee"></fiddle-embed></div>

### See Also

<a href="#SkImage_dimensions">dimensions</a> <a href="#SkImage_width">width</a>

---

<a name="SkImage_dimensions"></a>
## dimensions

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkISize dimensions() const
</pre>

Returns <a href="undocumented#ISize">ISize</a> { <a href="#SkImage_width">width</a>, <a href="#SkImage_height">height</a> }.

### Return Value

integral size of <a href="#SkImage_width">width</a> and <a href="#SkImage_height">height</a>

### Example

<div><fiddle-embed name="96b4bc43b3667df9ba9e2dafb770d33c"></fiddle-embed></div>

### See Also

<a href="#SkImage_height">height</a> <a href="#SkImage_width">width</a> <a href="#SkImage_bounds">bounds</a>

---

<a name="SkImage_bounds"></a>
## bounds

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkIRect bounds() const
</pre>

Returns <a href="SkIRect_Reference#IRect">IRect</a> { 0, 0, <a href="#SkImage_width">width</a>, <a href="#SkImage_height">height</a> }.

### Return Value

integral rectangle from origin to <a href="#SkImage_width">width</a> and <a href="#SkImage_height">height</a>

### Example

<div><fiddle-embed name="a6617b5d3066aadce2c37ed2184d6098"></fiddle-embed></div>

### See Also

<a href="#SkImage_dimensions">dimensions</a>

---

<a name="SkImage_uniqueID"></a>
## uniqueID

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
uint32_t uniqueID() const
</pre>

Returns value unique to image. <a href="#Image">Image</a> contents cannot change after <a href="#Image">Image</a> is
created. Any operation to create a new <a href="#Image">Image</a> will receive generate a new
unique number.

### Return Value

unique identifier

### Example

<div><fiddle-embed name="d70194c9c51e700335f95de91846d023"></fiddle-embed></div>

### See Also

<a href="#SkImage_isLazyGenerated">isLazyGenerated</a>

---

<a name="SkImage_alphaType"></a>
## alphaType

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkAlphaType alphaType() const
</pre>

Returns <a href="#Alpha_Type">Alpha Type</a>, one of: <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>, <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>,
<a href="undocumented#SkAlphaType">kPremul SkAlphaType</a>, <a href="undocumented#SkAlphaType">kUnpremul SkAlphaType</a>.

<a href="#Alpha_Type">Alpha Type</a> returned was a parameter to an <a href="#Image">Image</a> constructor,
or was parsed from encoded data.

### Return Value

<a href="#Alpha_Type">Alpha Type</a> in <a href="#Image">Image</a>

### Example

<div><fiddle-embed name="1c8b8588dbbb1d5df72ee9164ae0aab0"></fiddle-embed></div>

### See Also

<a href="#SkImageInfo_alphaType">SkImageInfo::alphaType</a>

---

<a name="SkImage_colorSpace"></a>
## colorSpace

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkColorSpace* colorSpace() const
</pre>

Returns <a href="undocumented#Color_Space">Color Space</a>, the range of colors, associated with <a href="#Image">Image</a>.  The
reference count of <a href="undocumented#Color_Space">Color Space</a> is unchanged. The returned <a href="undocumented#Color_Space">Color Space</a> is
immutable.

<a href="undocumented#Color_Space">Color Space</a> returned was passed to an <a href="#Image">Image</a> constructor,
or was parsed from encoded data. <a href="undocumented#Color_Space">Color Space</a> returned may be ignored when <a href="#Image">Image</a>
is drawn, depending on the capabilities of the <a href="SkSurface_Reference#Surface">Surface</a> receiving the drawing.

### Return Value

<a href="undocumented#Color_Space">Color Space</a> in <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="4468d573f42af6f5e234be10a5453bb2"></fiddle-embed></div>

### See Also

<a href="#SkImage_refColorSpace">refColorSpace</a> <a href="#SkImage_makeColorSpace">makeColorSpace</a>

---

<a name="SkImage_refColorSpace"></a>
## refColorSpace

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkColorSpace&gt; refColorSpace() const
</pre>

Returns a smart pointer to <a href="undocumented#Color_Space">Color Space</a>, the range of colors, associated with
<a href="#Image">Image</a>.  The smart pointer tracks the number of objects sharing this
<a href="undocumented#SkColorSpace">SkColorSpace</a> reference so the memory is released when the owners destruct.

The returned <a href="undocumented#SkColorSpace">SkColorSpace</a> is immutable.

<a href="undocumented#Color_Space">Color Space</a> returned was passed to an <a href="#Image">Image</a> constructor,
or was parsed from encoded data. <a href="undocumented#Color_Space">Color Space</a> returned may be ignored when <a href="#Image">Image</a>
is drawn, depending on the capabilities of the <a href="SkSurface_Reference#Surface">Surface</a> receiving the drawing.

### Return Value

<a href="undocumented#Color_Space">Color Space</a> in <a href="#Image">Image</a>, or nullptr, wrapped in a smart pointer

### Example

<div><fiddle-embed name="59b2078ebfbda8736a57c0486ae33332"></fiddle-embed></div>

### See Also

<a href="#SkImage_colorSpace">colorSpace</a> <a href="#SkImage_makeColorSpace">makeColorSpace</a>

---

<a name="SkImage_isAlphaOnly"></a>
## isAlphaOnly

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isAlphaOnly() const
</pre>

Returns true if <a href="#Image">Image</a> pixels represent transparency only. If true, each pixel
is packed in 8 bits as defined by <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>.

### Return Value

true if pixels represent a transparency mask

### Example

<div><fiddle-embed name="50762c73b8ea91959c5a7b68fbf1062d">

#### Example Output

~~~~
alphaOnly = true
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkImage_alphaType">alphaType</a> <a href="#SkImage_isOpaque">isOpaque</a>

---

<a name="SkImage_isOpaque"></a>
## isOpaque

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isOpaque() const
</pre>

Returns true if pixels ignore their <a href="#Alpha">Alpha</a> value and are treated as fully opaque.

### Return Value

true if <a href="#Alpha_Type">Alpha Type</a> is <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>

### Example

<div><fiddle-embed name="e3340460003b74ee286d625e68589d65">

#### Example Output

~~~~
isOpaque = false
isOpaque = true
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkImage_alphaType">alphaType</a> <a href="#SkImage_isAlphaOnly">isAlphaOnly</a>

---

<a name="SkImage_makeShader"></a>
## makeShader

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkShader&gt; makeShader(SkShader::TileMode tileMode1, SkShader::TileMode tileMode2,
                           const SkMatrix* localMatrix = nullptr) const
</pre>

Creates <a href="undocumented#Shader">Shader</a> from <a href="#Image">Image</a>. <a href="undocumented#Shader">Shader</a> <a href="#SkImage_dimensions">dimensions</a> are taken from <a href="#Image">Image</a>. <a href="undocumented#Shader">Shader</a> uses
<a href="#SkShader_TileMode">SkShader::TileMode</a> rules to fill drawn area outside <a href="#Image">Image</a>. <a href="#SkImage_makeShader_localMatrix">localMatrix</a> permits
transforming <a href="#Image">Image</a> before <a href="#Matrix">Canvas Matrix</a> is applied.

### Parameters

<table>  <tr>    <td><a name="SkImage_makeShader_tileMode1"> <code><strong>tileMode1 </strong></code> </a></td> <td>
tiling in x, one of: <a href="#SkShader_kClamp_TileMode">SkShader::kClamp TileMode</a>, <a href="#SkShader_kRepeat_TileMode">SkShader::kRepeat TileMode</a>,
<a href="#SkShader_kMirror_TileMode">SkShader::kMirror TileMode</a></td>
  </tr>  <tr>    <td><a name="SkImage_makeShader_tileMode2"> <code><strong>tileMode2 </strong></code> </a></td> <td>
tiling in y, one of: <a href="#SkShader_kClamp_TileMode">SkShader::kClamp TileMode</a>, <a href="#SkShader_kRepeat_TileMode">SkShader::kRepeat TileMode</a>,
<a href="#SkShader_kMirror_TileMode">SkShader::kMirror TileMode</a></td>
  </tr>  <tr>    <td><a name="SkImage_makeShader_localMatrix"> <code><strong>localMatrix </strong></code> </a></td> <td>
<a href="#Image">Image</a> transformation, or nullptr</td>
  </tr>
</table>

### Return Value

<a href="undocumented#Shader">Shader</a> containing <a href="#Image">Image</a>

### Example

<div><fiddle-embed name="1c6de6fe72b00b5be970f5f718363449"></fiddle-embed></div>

### See Also

<a href="#SkImage_scalePixels">scalePixels</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkShader&gt; makeShader(const SkMatrix* localMatrix = nullptr) const
</pre>

Creates <a href="undocumented#Shader">Shader</a> from <a href="#Image">Image</a>. <a href="undocumented#Shader">Shader</a> <a href="#SkImage_dimensions">dimensions</a> are taken from <a href="#Image">Image</a>. <a href="undocumented#Shader">Shader</a> uses
<a href="#SkShader_kClamp_TileMode">SkShader::kClamp TileMode</a> to fill drawn area outside <a href="#Image">Image</a>. <a href="#SkImage_makeShader_2_localMatrix">localMatrix</a> permits
transforming <a href="#Image">Image</a> before <a href="#Matrix">Canvas Matrix</a> is applied.

### Parameters

<table>  <tr>    <td><a name="SkImage_makeShader_2_localMatrix"> <code><strong>localMatrix </strong></code> </a></td> <td>
<a href="#Image">Image</a> transformation, or nullptr</td>
  </tr>
</table>

### Return Value

<a href="undocumented#Shader">Shader</a> containing <a href="#Image">Image</a>

### Example

<div><fiddle-embed name="10172fca71b9dbdcade772513ffeb27e"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImage_peekPixels"></a>
## peekPixels

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool peekPixels(SkPixmap* pixmap) const
</pre>

If the image has direct access to its pixels (i.e. they are in local RAM)
return true, and if not null, return in the <a href="#SkImage_peekPixels_pixmap">pixmap</a> parameter the info about the
images pixels.
On failure, return false and ignore the <a href="#SkImage_peekPixels_pixmap">pixmap</a> parameter.

### Parameters

<table>  <tr>    <td><a name="SkImage_peekPixels_pixmap"> <code><strong>pixmap </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImage_getTexture"></a>
## getTexture

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
GrTexture* getTexture() const
</pre>

Deprecated.

---

<a name="SkImage_isTextureBacked"></a>
## isTextureBacked

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isTextureBacked() const
</pre>

Returns true if the image is texture backed.

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImage_isValid"></a>
## isValid

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isValid(GrContext* context) const
</pre>

Returns true if <a href="#Image">Image</a> can be drawn. If <a href="#SkImage_isValid_context">context</a>
is nullptr, tests if <a href="#Image">Image</a> draws on <a href="undocumented#Raster_Surface">Raster Surface</a>; Otherwise, tests if <a href="#Image">Image</a>
draws on <a href="undocumented#GPU_Surface">GPU Surface</a> associated with <a href="#SkImage_isValid_context">context</a>.

<a href="undocumented#Texture">Texture</a>-backed images may become invalid if their underlying <a href="undocumented#GrContext">GrContext</a> is abandoned. Some
generator-backed images may be invalid for CPU and/or GPU.

### Parameters

<table>  <tr>    <td><a name="SkImage_isValid_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImage_getTextureHandle"></a>
## getTextureHandle

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
GrBackendObject getTextureHandle(bool flushPendingGrContextIO, GrSurfaceOrigin* origin = nullptr) const
</pre>

Retrieves the back-end API handle of texture. If <a href="#SkImage_getTextureHandle_flushPendingGrContextIO">flushPendingGrContextIO</a> is true,
complete deferred I/O operations.

If <a href="#SkImage_getTextureHandle_origin">origin</a> in not nullptr, copies location of content drawn into <a href="#Image">Image</a>.

### Parameters

<table>  <tr>    <td><a name="SkImage_getTextureHandle_flushPendingGrContextIO"> <code><strong>flushPendingGrContextIO </strong></code> </a></td> <td>
flag to flush outstanding requests</td>
  </tr>  <tr>    <td><a name="SkImage_getTextureHandle_origin"> <code><strong>origin </strong></code> </a></td> <td>
storage for one of: <a href="undocumented#GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a>,
<a href="undocumented#GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>; or nullptr</td>
  </tr>
</table>

### Return Value

back-end API texture handle

### Example

<div><fiddle-embed name="704b914d622fbff24d7a45647380459e" gpu="true"></fiddle-embed></div>

### See Also

incomplete

---

## <a name="SkImage_CachingHint"></a> Enum SkImage::CachingHint

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum <a href="#SkImage_CachingHint">CachingHint</a> {
<a href="#SkImage_kAllow_CachingHint">kAllow CachingHint</a>,
<a href="#SkImage_kDisallow_CachingHint">kDisallow CachingHint</a>,
};</pre>

Hints to image calls where the system might cache computed intermediates (e.g. the results
of decoding or a read-back from the GPU. Passing <a href="#SkImage_kAllow_CachingHint">kAllow CachingHint</a> signals that the system's default
behavior is fine. Passing <a href="#SkImage_kDisallow_CachingHint">kDisallow CachingHint</a> signals that caching should be avoided.

### Constants

<table>
  <tr>
    <td><a name="SkImage_kAllow_CachingHint"> <code><strong>SkImage::kAllow_CachingHint </strong></code> </a></td><td>0</td><td></td>
  </tr>
  <tr>
    <td><a name="SkImage_kDisallow_CachingHint"> <code><strong>SkImage::kDisallow_CachingHint </strong></code> </a></td><td>1</td><td></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete



<a name="SkImage_readPixels"></a>
## readPixels

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes, int srcX, int srcY,
                CachingHint cachingHint = kAllow_CachingHint) const
</pre>

Copy the pixels from the image into the specified buffer (<a href="#SkImage_readPixels_dstPixels">dstPixels</a> + <a href="#SkImage_readPixels_dstRowBytes">dstRowBytes</a>),
converting them into the requested format (<a href="#SkImage_readPixels_dstInfo">dstInfo</a>). The image pixels are read
starting at the specified (<a href="#SkImage_readPixels_srcX">srcX</a>, <a href="#SkImage_readPixels_srcY">srcY</a>) location.
<a href="#SkImage_readPixels_dstInfo">dstInfo</a> and (<a href="#SkImage_readPixels_srcX">srcX</a>, <a href="#SkImage_readPixels_srcY">srcY</a>) offset specifies a source rectangle:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
<a href="SkRect_Reference#SkRect">SkRect</a> srcR;
srcR.setXYWH(srcX, srcY, dstInfo.width(), <a href="#SkImage_readPixels_dstInfo">dstInfo</a>.<a href="#SkImage_height">height</a>);</pre>

The source rectangle is intersected with the <a href="#SkImage_bounds">bounds</a> of the image. If this intersection is not empty,
then we have two sets of pixels (of equal size). Replace <a href="#SkImage_readPixels_dstPixels">dstPixels</a> with the
corresponding <a href="#Image">Image</a> pixels, performing any <a href="#Color_Type">Color Type</a>/<a href="#Alpha_Type">Alpha Type</a> transformations needed
(in the case where <a href="#Image">Image</a> and <a href="#SkImage_readPixels_dstInfo">dstInfo</a> have different <a href="#Color_Type">Color Types</a> or <a href="#Alpha_Type">Alpha Types</a>).
This call can fail, returning false, for several reasons:
if source rectangle does not intersect the image <a href="#SkImage_bounds">bounds</a>;
if the requested <a href="#Color_Type">Color Type</a>/<a href="#Alpha_Type">Alpha Type</a> cannot be converted from the image's types.

### Parameters

<table>  <tr>    <td><a name="SkImage_readPixels_dstInfo"> <code><strong>dstInfo </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_readPixels_dstPixels"> <code><strong>dstPixels </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_readPixels_dstRowBytes"> <code><strong>dstRowBytes </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_readPixels_srcX"> <code><strong>srcX </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_readPixels_srcY"> <code><strong>srcY </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_readPixels_cachingHint"> <code><strong>cachingHint </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool readPixels(const SkPixmap& dst, int srcX, int srcY, CachingHint cachingHint = kAllow_CachingHint) const
</pre>

### Parameters

<table>  <tr>    <td><a name="SkImage_readPixels_2_dst"> <code><strong>dst </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_readPixels_2_srcX"> <code><strong>srcX </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_readPixels_2_srcY"> <code><strong>srcY </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_readPixels_2_cachingHint"> <code><strong>cachingHint </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImage_scalePixels"></a>
## scalePixels

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool scalePixels(const SkPixmap& dst, SkFilterQuality filterQuality,
                 CachingHint cachingHint = kAllow_CachingHint) const
</pre>

Copies <a href="#Image">Image</a> pixels into <a href="#SkImage_scalePixels_dst">dst</a>, converting to <a href="#SkImage_scalePixels_dst">dst</a> <a href="#Color_Type">Color Type</a> and <a href="#Alpha_Type">Alpha Type</a>.
If the conversion cannot be performed, false is returned.
If <a href="#SkImage_scalePixels_dst">dst</a> <a href="#SkImage_dimensions">dimensions</a> differ from <a href="#Image">Image</a> <a href="#SkImage_dimensions">dimensions</a>, <a href="#Image">Image</a> is scaled, applying
<a href="#SkImage_scalePixels_filterQuality">filterQuality</a>.

### Parameters

<table>  <tr>    <td><a name="SkImage_scalePixels_dst"> <code><strong>dst </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_scalePixels_filterQuality"> <code><strong>filterQuality </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_scalePixels_cachingHint"> <code><strong>cachingHint </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImage_encodeToData"></a>
## encodeToData

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkData&gt; encodeToData(SkEncodedImageFormat encodedImageFormat, int quality) const
</pre>

Encodes <a href="#Image">Image</a> pixels, returning result as <a href="undocumented#SkData">SkData</a>.

Returns nullptr if encoding fails, or <a href="#SkImage_encodeToData_encodedImageFormat">encodedImageFormat</a> is not supported.

### Parameters

<table>  <tr>    <td><a name="SkImage_encodeToData_encodedImageFormat"> <code><strong>encodedImageFormat </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_encodeToData_quality"> <code><strong>quality </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

encoded <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkData&gt; encodeToData() const
</pre>

Encodes <a href="#Image">Image</a> and returns result as <a href="undocumented#SkData">SkData</a>. Will reuse existing encoded data
if present, as returned by <a href="#SkImage_refEncodedData">refEncodedData</a>. If encoded data is missing or invalid,
<a href="#Image">Image</a> is encoded as PNG.

Returns nullptr if existing encoded data is missing or invalid and
encoding fails.

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImage_refEncodedData"></a>
## refEncodedData

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkData&gt; refEncodedData() const
</pre>

If the image already has its contents in encoded form (e.g. PNG or JPEG), return that
as <a href="undocumented#SkData">SkData</a>. If the image does not already has its contents in encoded form, return nullptr.

To force the image to return its contents as encoded data, call <a href="#SkImage_encodeToData">encodeToData</a>.

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImage_toString"></a>
## toString

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const char* toString(SkString* string) const
</pre>

### Parameters

<table>  <tr>    <td><a name="SkImage_toString_string"> <code><strong>string </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImage_makeSubset"></a>
## makeSubset

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkImage&gt; makeSubset(const SkIRect& subset) const
</pre>

Return a new image that is a <a href="#SkImage_makeSubset_subset">subset</a> of this image. The underlying implementation may
share the pixels, or it may make a copy.
If <a href="#SkImage_makeSubset_subset">subset</a> does not intersect the <a href="#SkImage_bounds">bounds</a> of this image, or the copy/share cannot be made,
nullptr will be returned.

### Parameters

<table>  <tr>    <td><a name="SkImage_makeSubset_subset"> <code><strong>subset </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImage_makeTextureImage"></a>
## makeTextureImage

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkImage&gt; makeTextureImage(GrContext* context, SkColorSpace* dstColorSpace) const
</pre>

Ensures that an image is backed by a texture (when <a href="undocumented#GrContext">GrContext</a> is non-null), suitable for use
with surfaces that have the supplied destination color space. If no transformation is
required, the returned image may be the same as this image. If this image is from a
different <a href="undocumented#GrContext">GrContext</a>, this will fail.

### Parameters

<table>  <tr>    <td><a name="SkImage_makeTextureImage_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_makeTextureImage_dstColorSpace"> <code><strong>dstColorSpace </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImage_makeNonTextureImage"></a>
## makeNonTextureImage

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkImage&gt; makeNonTextureImage() const
</pre>

Creates raster <a href="#Image">Image</a> if <a href="#Image">Image</a> contains <a href="undocumented#GPU_Texture">GPU Texture</a> and raster <a href="#Image">Image</a>  ,  this will make a raster copy of it (or nullptr if reading back
the pixels fails). Otherwise, it returns the original image.

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImage_makeRasterImage"></a>
## makeRasterImage

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkImage&gt; makeRasterImage() const
</pre>

If the image is texture-backed this will make a raster copy of it (or nullptr if reading back
the pixels fails). Otherwise, it returns the original image.

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImage_makeWithFilter"></a>
## makeWithFilter

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkImage&gt; makeWithFilter(const SkImageFilter* filter, const SkIRect& subset,
                              const SkIRect& clipBounds, SkIRect* outSubset, SkIPoint* offset) const
</pre>

Apply a given image <a href="#SkImage_makeWithFilter_filter">filter</a> to this image, and return the filtered result.
The <a href="#SkImage_makeWithFilter_subset">subset</a> represents the active portion of this image. The return value is similarly an
<a href="#SkImage">SkImage</a>, with an active <a href="#SkImage_makeWithFilter_subset">subset</a> (<a href="#SkImage_makeWithFilter_outSubset">outSubset</a>). This is usually used with texture-backed
images, where the texture may be approx-match and thus larger than the required size.
<a href="#SkImage_makeWithFilter_clipBounds">clipBounds</a> constrains the device-space extent of the image, stored in <a href="#SkImage_makeWithFilter_outSubset">outSubset</a>.
<a href="#SkImage_makeWithFilter_offset">offset</a> is storage, set to the amount to translate the result when drawn.
If the result image cannot be created, or the result would be transparent black, null
is returned, in which case the <a href="#SkImage_makeWithFilter_offset">offset</a> and <a href="#SkImage_makeWithFilter_outSubset">outSubset</a> parameters should be ignored by the
caller.

### Parameters

<table>  <tr>    <td><a name="SkImage_makeWithFilter_filter"> <code><strong>filter </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_makeWithFilter_subset"> <code><strong>subset </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_makeWithFilter_clipBounds"> <code><strong>clipBounds </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_makeWithFilter_outSubset"> <code><strong>outSubset </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_makeWithFilter_offset"> <code><strong>offset </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

# <a name="SkImage_DeferredTextureImageUsageParams"></a> Struct SkImage::DeferredTextureImageUsageParams

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
struct <a href="#SkImage_DeferredTextureImageUsageParams_DeferredTextureImageUsageParams">DeferredTextureImageUsageParams</a> {
<a href="#SkImage_DeferredTextureImageUsageParams_DeferredTextureImageUsageParams">DeferredTextureImageUsageParams(const SkMatrix matrix, const SkFilterQuality quality,
int preScaleMipLevel)</a>;
<a href="SkMatrix_Reference#SkMatrix">SkMatrix</a>        <a href="#SkImage_DeferredTextureImageUsageParams_fMatrix">fMatrix</a>;
<a href="undocumented#SkFilterQuality">SkFilterQuality</a> <a href="#SkImage_DeferredTextureImageUsageParams_fQuality">fQuality</a>;
int             <a href="#SkImage_DeferredTextureImageUsageParams_fPreScaleMipLevel">fPreScaleMipLevel</a>;
};</pre>

<a name="SkImage_DeferredTextureImageUsageParams_fMatrix"> <code><strong>SkMatrix fMatrix</strong></code> </a>

<a name="SkImage_DeferredTextureImageUsageParams_fQuality"> <code><strong>SkFilterQuality fQuality</strong></code> </a>

<a name="SkImage_DeferredTextureImageUsageParams_fPreScaleMipLevel"> <code><strong>int fPreScaleMipLevel</strong></code> </a>

<a name="SkImage_DeferredTextureImageUsageParams_DeferredTextureImageUsageParams"></a>
## DeferredTextureImageUsageParams

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
DeferredTextureImageUsageParams(const SkMatrix matrix, const SkFilterQuality quality,
                                int preScaleMipLevel)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkImage_DeferredTextureImageUsageParams_DeferredTextureImageUsageParams_matrix"> <code><strong>matrix </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_DeferredTextureImageUsageParams_DeferredTextureImageUsageParams_quality"> <code><strong>quality </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_DeferredTextureImageUsageParams_DeferredTextureImageUsageParams_preScaleMipLevel"> <code><strong>preScaleMipLevel </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name="SkImage_getDeferredTextureImageData"></a>
## getDeferredTextureImageData

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
size_t getDeferredTextureImageData(const GrContextThreadSafeProxy& contextThreadSafeProxy,
                 const DeferredTextureImageUsageParams deferredTextureImageUsageParams[],
                 int paramCnt, void* buffer, SkColorSpace* dstColorSpace = nullptr,
                 SkColorType dstColorType = kN32_SkColorType) const
</pre>

This method allows clients to capture the data necessary to turn a <a href="#SkImage">SkImage</a> into a texture-
backed image. If the original image is codec-backed this will decode into a format optimized
for the context represented by the proxy. This method is thread safe with respect to the
<a href="undocumented#GrContext">GrContext</a> whence the proxy came. Clients allocate and manage the storage of the deferred
texture data and control its lifetime. No cleanup is required, thus it is safe to simply free
the memory out from under the data.

The same method is used both for getting the size necessary for pre-uploaded texture data
and for retrieving the data. The params array represents the set of draws over which to
optimize the pre-upload data.

When called with a null <a href="#SkImage_getDeferredTextureImageData_buffer">buffer</a> this returns the size that the client must allocate in order
to create deferred texture data for this image (or zero if this is an inappropriate
candidate). The <a href="#SkImage_getDeferredTextureImageData_buffer">buffer</a> allocated by the client should be 8 byte aligned.

When <a href="#SkImage_getDeferredTextureImageData_buffer">buffer</a> is not null this fills in the deferred texture data for this image in the
provided <a href="#SkImage_getDeferredTextureImageData_buffer">buffer</a> (assuming this is an appropriate candidate image and the <a href="#SkImage_getDeferredTextureImageData_buffer">buffer</a> is
appropriately aligned). Upon success the size written is returned, otherwise 0.

<a href="#SkImage_getDeferredTextureImageData_dstColorSpace">dstColorSpace</a> is the color space of the surface where this texture will ultimately be used.
If the method determines that mip-maps are needed, this helps determine the correct strategy
for building them (gamma-correct or not).

<a href="#SkImage_getDeferredTextureImageData_dstColorType">dstColorType</a> is the color type of the surface where this texture will ultimately be used.
This determines the format with which the image will be uploaded to the GPU. If <a href="#SkImage_getDeferredTextureImageData_dstColorType">dstColorType</a>
does not support color spaces (low bit depth types such as <a href="undocumented#SkColorType">kARGB 4444 SkColorType</a>),
then <a href="#SkImage_getDeferredTextureImageData_dstColorSpace">dstColorSpace</a> must be null.

### Parameters

<table>  <tr>    <td><a name="SkImage_getDeferredTextureImageData_contextThreadSafeProxy"> <code><strong>contextThreadSafeProxy </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_getDeferredTextureImageData_deferredTextureImageUsageParams"> <code><strong>deferredTextureImageUsageParams </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_getDeferredTextureImageData_paramCnt"> <code><strong>paramCnt </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_getDeferredTextureImageData_buffer"> <code><strong>buffer </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_getDeferredTextureImageData_dstColorSpace"> <code><strong>dstColorSpace </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_getDeferredTextureImageData_dstColorType"> <code><strong>dstColorType </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImage_MakeFromDeferredTextureImageData"></a>
## MakeFromDeferredTextureImageData

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromDeferredTextureImageData(GrContext* context, const void* data,
                                                SkBudgeted budgeted)
</pre>

Returns a texture-backed image from <a href="#SkImage_MakeFromDeferredTextureImageData_data">data</a> produced in <a href="#SkImage_getDeferredTextureImageData">SkImage::getDeferredTextureImageData</a>.
The <a href="#SkImage_MakeFromDeferredTextureImageData_context">context</a> must be the <a href="#SkImage_MakeFromDeferredTextureImageData_context">context</a> that provided the proxy passed to
<a href="#SkImage_getDeferredTextureImageData">getDeferredTextureImageData</a>.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromDeferredTextureImageData_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromDeferredTextureImageData_data"> <code><strong>data </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromDeferredTextureImageData_budgeted"> <code><strong>budgeted </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a href="#SkImage_BackendTextureReleaseProc">BackendTextureReleaseProc</a>

<a name="SkImage_MakeBackendTextureFromSkImage"></a>
## MakeBackendTextureFromSkImage

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static bool MakeBackendTextureFromSkImage(GrContext* context, sk_sp&lt;SkImage&gt; image,
                                          GrBackendTexture* backendTexture,
                                          BackendTextureReleaseProc* backendTextureReleaseProc)
</pre>

Creates a <a href="undocumented#GrBackendTexture">GrBackendTexture</a> from the provided <a href="#SkImage">SkImage</a>. Returns true on success. The
<a href="undocumented#GrBackendTexture">GrBackendTexture</a> and <a href="#SkImage_BackendTextureReleaseProc">BackendTextureReleaseProc</a> are populated on success. It is the callers
responsibility to call the <a href="#SkImage_BackendTextureReleaseProc">BackendTextureReleaseProc</a> once they have deleted the texture.
Note that the <a href="#SkImage_BackendTextureReleaseProc">BackendTextureReleaseProc</a> allows Skia to clean up auxiliary data related
to the <a href="undocumented#GrBackendTexture">GrBackendTexture</a>, and is not a substitute for the client deleting the <a href="undocumented#GrBackendTexture">GrBackendTexture</a>
themselves.

If <a href="#SkImage_MakeBackendTextureFromSkImage_image">image</a> is both texture backed and singly referenced; that is, its only
reference was transferred using std::move(): <a href="#SkImage_MakeBackendTextureFromSkImage_image">image</a> is returned in <a href="#SkImage_MakeBackendTextureFromSkImage_backendTexture">backendTexture</a>
without conversion or making a copy.

If the <a href="#SkImage">SkImage</a> is not texture backed, this function will generate a texture with the <a href="#SkImage_MakeBackendTextureFromSkImage_image">image</a>'s
contents and return that.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeBackendTextureFromSkImage_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeBackendTextureFromSkImage_image"> <code><strong>image </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeBackendTextureFromSkImage_backendTexture"> <code><strong>backendTexture </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeBackendTextureFromSkImage_backendTextureReleaseProc"> <code><strong>backendTextureReleaseProc </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

## <a name="SkImage_LegacyBitmapMode"></a> Enum SkImage::LegacyBitmapMode

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum <a href="#SkImage_LegacyBitmapMode">LegacyBitmapMode</a> {
<a href="#SkImage_kRO_LegacyBitmapMode">kRO LegacyBitmapMode</a>,
<a href="#SkImage_kRW_LegacyBitmapMode">kRW LegacyBitmapMode</a>,
};</pre>

Helper functions to convert to <a href="SkBitmap_Reference#SkBitmap">SkBitmap</a>

### Constants

<table>
  <tr>
    <td><a name="SkImage_kRO_LegacyBitmapMode"> <code><strong>SkImage::kRO_LegacyBitmapMode </strong></code> </a></td><td>0</td><td></td>
  </tr>
  <tr>
    <td><a name="SkImage_kRW_LegacyBitmapMode"> <code><strong>SkImage::kRW_LegacyBitmapMode </strong></code> </a></td><td>1</td><td></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete



<a name="SkImage_asLegacyBitmap"></a>
## asLegacyBitmap

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool asLegacyBitmap(SkBitmap* bitmap, LegacyBitmapMode legacyBitmapMode) const
</pre>

Creates raster <a href="SkBitmap_Reference#Bitmap">Bitmap</a> with same pixels as <a href="#Image">Image</a>. If <a href="#SkImage_asLegacyBitmap_legacyBitmapMode">legacyBitmapMode</a> is <a href="#SkImage_kRO_LegacyBitmapMode">kRO LegacyBitmapMode</a>,
returned <a href="#SkImage_asLegacyBitmap_bitmap">bitmap</a> is read-only and immutable.
Returns true if <a href="SkBitmap_Reference#Bitmap">Bitmap</a> is stored in <a href="#SkImage_asLegacyBitmap_bitmap">bitmap</a>. Returns false and resets <a href="#SkImage_asLegacyBitmap_bitmap">bitmap</a> if <a href="SkBitmap_Reference#Bitmap">Bitmap</a>
write did not succeed.

### Parameters

<table>  <tr>    <td><a name="SkImage_asLegacyBitmap_bitmap"> <code><strong>bitmap </strong></code> </a></td> <td>
storage for legacy <a href="SkBitmap_Reference#Bitmap">Bitmap</a></td>
  </tr>  <tr>    <td><a name="SkImage_asLegacyBitmap_legacyBitmapMode"> <code><strong>legacyBitmapMode </strong></code> </a></td> <td>
one of: <a href="#SkImage_kRO_LegacyBitmapMode">kRO LegacyBitmapMode</a>, <a href="#SkImage_kRW_LegacyBitmapMode">kRW LegacyBitmapMode</a></td>
  </tr>
</table>

### Return Value

true if <a href="SkBitmap_Reference#Bitmap">Bitmap</a> was created

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImage_isLazyGenerated"></a>
## isLazyGenerated

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isLazyGenerated() const
</pre>

Returns true if <a href="#Image">Image</a> is backed by an image-generator or other service that creates
and caches its pixels or texture on-demand.

### Return Value

true if <a href="#Image">Image</a> is created as needed

### Example

<div><fiddle-embed name="a8b8bd4bfe968e2c63085f867665227f"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImage_makeColorSpace"></a>
## makeColorSpace

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkImage&gt; makeColorSpace(sk_sp&lt;SkColorSpace&gt; target, SkTransferFunctionBehavior premulBehavior) const
</pre>

If <a href="#SkImage_makeColorSpace_target">target</a> is supported, returns an <a href="#SkImage">SkImage</a> in <a href="#SkImage_makeColorSpace_target">target</a> color space.
Otherwise, returns nullptr.
This will leave the image as is if it already in <a href="#SkImage_makeColorSpace_target">target</a> color space.
Otherwise, it will convert the pixels from <a href="#Image">Image</a> color space to <a href="#SkImage_makeColorSpace_target">target</a>
color space.  If this-><a href="#SkImage_colorSpace">colorSpace</a> is nullptr, <a href="#Image">Image</a> color space will be
treated as sRGB.

If <a href="#SkImage_makeColorSpace_premulBehavior">premulBehavior</a> is <a href="#SkTransferFunctionBehavior_kRespect">SkTransferFunctionBehavior::kRespect</a>: converts <a href="#Image">Image</a>
pixels to a linear space before converting to match destination <a href="#Color_Type">Color Type</a>
and <a href="undocumented#Color_Space">Color Space</a>.
If <a href="#SkImage_makeColorSpace_premulBehavior">premulBehavior</a> is <a href="#SkTransferFunctionBehavior_kIgnore">SkTransferFunctionBehavior::kIgnore</a>: <a href="#Image">Image</a>
pixels are treated as if they are linear, regardless of how they are encoded.

### Parameters

<table>  <tr>    <td><a name="SkImage_makeColorSpace_target"> <code><strong>target </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_makeColorSpace_premulBehavior"> <code><strong>premulBehavior </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

