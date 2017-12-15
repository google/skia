SkImage Reference
===

# <a name="Image"></a> Image

# <a name="SkImage"></a> Class SkImage
<a href="#SkImage">SkImage</a> is an abstraction for drawing a rectangle of pixels, though the
particular type of image could be actually storing its data on the <a href="undocumented#GPU">GPU</a>, or
as drawing commands (picture or <a href="undocumented#PDF">PDF</a> or otherwise), ready to be played back
into another canvas.
The content of <a href="#SkImage">SkImage</a> is always immutable, though the actual storage may
change, if for example that image can be re-created via encoded data or
other means.
<a href="#SkImage">SkImage</a> always has a non-zero <a href="#SkImage_dimensions">dimensions</a>. If there is a request to create a new
image, either directly or via <a href="SkSurface_Reference#SkSurface">SkSurface</a>, and either of the requested <a href="#SkImage_dimensions">dimensions</a>
are zero, then nullptr will be returned.

# <a name="Overview"></a> Overview

## <a name="Subtopics"></a> Subtopics

| topics | description |
| --- | ---  |

## <a name="Member_Functions"></a> Member Functions

| description | function |
| --- | ---  |
| <a href="#SkImage_MakeBackendTextureFromSkImage">MakeBackendTextureFromSkImage</a> | Creates <a href="undocumented#GPU">GPU</a> texture from <a href="#Image">Image</a>. |
| <a href="#SkImage_MakeCrossContextFromEncoded">MakeCrossContextFromEncoded</a> | Creates <a href="#Image">Image</a> from encoded data, and uploads to <a href="undocumented#GPU">GPU</a>. |
| <a href="#SkImage_MakeFromAHardwareBuffer">MakeFromAHardwareBuffer</a> | Creates <a href="#Image">Image</a> from <a href="undocumented#Android">Android</a> hardware buffer. |
| <a href="#SkImage_MakeFromAdoptedTexture">MakeFromAdoptedTexture</a> | Creates <a href="#Image">Image</a> from <a href="undocumented#GPU">GPU</a> texture, managed internally. |
| <a href="#SkImage_MakeFromBitmap">MakeFromBitmap</a> | Creates <a href="#Image">Image</a> from <a href="SkBitmap_Reference#Bitmap">Bitmap</a>, sharing or copying pixels. |
| <a href="#SkImage_MakeFromEncoded">MakeFromEncoded</a> | Creates <a href="#Image">Image</a> from encoded data. |
| <a href="#SkImage_MakeFromGenerator">MakeFromGenerator</a> | Creates <a href="#Image">Image</a> from a stream of data. |
| <a href="#SkImage_MakeFromNV12TexturesCopy">MakeFromNV12TexturesCopy</a> | Creates <a href="#Image">Image</a> from <a href="undocumented#YUV_ColorSpace">YUV ColorSpace</a> data in two planes. |
| <a href="#SkImage_MakeFromPicture">MakeFromPicture</a> | Creates <a href="#Image">Image</a> from <a href="undocumented#Picture">Picture</a>. |
| <a href="#SkImage_MakeFromRaster">MakeFromRaster</a> | Creates <a href="#Image">Image</a> from <a href="SkPixmap_Reference#Pixmap">Pixmap</a>, with release. |
| <a href="#SkImage_MakeFromTexture">MakeFromTexture</a> | Creates <a href="#Image">Image</a> from <a href="undocumented#GPU">GPU</a> texture, managed externally. |
| <a href="#SkImage_MakeFromYUVTexturesCopy">MakeFromYUVTexturesCopy</a> | Creates <a href="#Image">Image</a> from <a href="undocumented#YUV_ColorSpace">YUV ColorSpace</a> data in three planes. |
| <a href="#SkImage_MakeRasterCopy">MakeRasterCopy</a> | Creates <a href="#Image">Image</a> from <a href="SkPixmap_Reference#Pixmap">Pixmap</a> and copied pixels. |
| <a href="#SkImage_MakeRasterData">MakeRasterData</a> | Creates <a href="#Image">Image</a> from <a href="#Info">Image Info</a> and shared pixels. |
| <a href="#SkImage_alphaType">alphaType</a> | Returns <a href="#Alpha_Type">Alpha Type</a> |
| <a href="#SkImage_asLegacyBitmap">asLegacyBitmap</a> | Returns as <a href="undocumented#Raster_Bitmap">Raster Bitmap</a> |
| <a href="#SkImage_bounds">bounds</a> | Returns <a href="#SkImage_width">width</a> and <a href="#SkImage_height">height</a> as Rectangle. |
| <a href="#SkImage_colorSpace">colorSpace</a> | Returns <a href="undocumented#Color_Space">Color Space</a>. |
| <a href="#SkImage_dimensions">dimensions</a> | Returns <a href="#SkImage_width">width</a> and <a href="#SkImage_height">height</a>. |
| <a href="#SkImage_encodeToData">encodeToData</a> | Returns encoded <a href="#Image">Image</a> as <a href="undocumented#SkData">SkData</a>. |
| <a href="#SkImage_getTexture">getTexture</a> | Deprecated. |
| <a href="#SkImage_getTextureHandle">getTextureHandle</a> | Returns <a href="undocumented#GPU">GPU</a> reference to <a href="#Image">Image</a> as texture. |
| <a href="#SkImage_height">height</a> | Returns pixel row count. |
| <a href="#SkImage_isAlphaOnly">isAlphaOnly</a> | Returns if pixels represent a transparency mask. |
| <a href="#SkImage_isLazyGenerated">isLazyGenerated</a> | Returns if <a href="#Image">Image</a> is created as needed. |
| <a href="#SkImage_isOpaque">isOpaque</a> | Returns if <a href="#Alpha_Type">Alpha Type</a> is <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>. |
| <a href="#SkImage_isTextureBacked">isTextureBacked</a> | Returns if <a href="#Image">Image</a> was created from <a href="undocumented#GPU">GPU</a> texture. |
| <a href="#SkImage_isValid">isValid</a> | Returns if <a href="#Image">Image</a> can draw to <a href="undocumented#Raster_Surface">Raster Surface</a> or <a href="undocumented#GPU_Context">GPU Context</a>. |
| <a href="#SkImage_makeColorSpace">makeColorSpace</a> | Creates <a href="#Image">Image</a> matching <a href="undocumented#Color_Space">Color Space</a> if possible. |
| <a href="#SkImage_makeNonTextureImage">makeNonTextureImage</a> | Creates <a href="undocumented#Raster_Image">Raster Image</a> if possible. |
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

Creates <a href="#Image">Image</a> from <a href="undocumented#GPU">GPU</a> texture associated with <a href="#SkImage_MakeFromTexture_context">context</a>. Caller is responsible for
managing the lifetime of <a href="undocumented#GPU">GPU</a> texture.

<a href="#Image">Image</a> is returned if format of <a href="#SkImage_MakeFromTexture_backendTexture">backendTexture</a> is recognized and supported.
Recognized formats vary by <a href="undocumented#GPU">GPU</a> back-end.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromTexture_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_backendTexture"> <code><strong>backendTexture </strong></code> </a></td> <td>
texture residing on <a href="undocumented#GPU">GPU</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_origin"> <code><strong>origin </strong></code> </a></td> <td>
one of: <a href="undocumented#GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>, <a href="undocumented#GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_alphaType"> <code><strong>alphaType </strong></code> </a></td> <td>
one of: <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>, <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>,
<a href="undocumented#SkAlphaType">kPremul SkAlphaType</a>, <a href="undocumented#SkAlphaType">kUnpremul SkAlphaType</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="2faa98d6a1d578010326af17ee7e4d2a" gpu="true"><div>A back-end texture has been created and uploaded to the <a href="undocumented#GPU">GPU</a> outside of this example.</div></fiddle-embed></div>

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

Creates <a href="#Image">Image</a> from <a href="undocumented#GPU">GPU</a> texture associated with <a href="#SkImage_MakeFromTexture_2_context">context</a>. <a href="undocumented#GPU">GPU</a> texture must stay
valid and unchanged until <a href="#SkImage_MakeFromTexture_2_textureReleaseProc">textureReleaseProc</a> is called. <a href="#SkImage_MakeFromTexture_2_textureReleaseProc">textureReleaseProc</a> is
passed <a href="#SkImage_MakeFromTexture_2_releaseContext">releaseContext</a> when <a href="#Image">Image</a> is deleted or no longer refers to texture.

<a href="#Image">Image</a> is returned if format of <a href="#SkImage_MakeFromTexture_2_backendTexture">backendTexture</a> is recognized and supported.
Recognized formats vary by <a href="undocumented#GPU">GPU</a> back-end.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromTexture_2_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_2_backendTexture"> <code><strong>backendTexture </strong></code> </a></td> <td>
texture residing on <a href="undocumented#GPU">GPU</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_2_origin"> <code><strong>origin </strong></code> </a></td> <td>
one of: <a href="undocumented#GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>, <a href="undocumented#GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_2_alphaType"> <code><strong>alphaType </strong></code> </a></td> <td>
one of: <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>, <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>,
<a href="undocumented#SkAlphaType">kPremul SkAlphaType</a>, <a href="undocumented#SkAlphaType">kUnpremul SkAlphaType</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_2_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_2_textureReleaseProc"> <code><strong>textureReleaseProc </strong></code> </a></td> <td>
function called when texture can be released</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_2_releaseContext"> <code><strong>releaseContext </strong></code> </a></td> <td>
state passed to <a href="#SkImage_MakeFromTexture_2_textureReleaseProc">textureReleaseProc</a></td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="dea2835da4be78d1e9ab4be58010d1ad"></fiddle-embed></div>

### See Also

<a href="#SkImage_MakeFromAdoptedTexture">MakeFromAdoptedTexture</a> <a href="#SkSurface_MakeFromBackendTexture">SkSurface::MakeFromBackendTexture</a>

---

<a name="SkImage_MakeCrossContextFromEncoded"></a>
## MakeCrossContextFromEncoded

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeCrossContextFromEncoded(GrContext* context, sk_sp&lt;SkData&gt; data,
                                                  bool buildMips, SkColorSpace* dstColorSpace)
</pre>

Creates <a href="#Image">Image</a> from encoded <a href="#SkImage_MakeCrossContextFromEncoded_data">data</a>. <a href="#Image">Image</a> is uploaded to <a href="undocumented#GPU">GPU</a> back-end using <a href="#SkImage_MakeCrossContextFromEncoded_context">context</a>. 

Created <a href="#Image">Image</a> is available to other <a href="undocumented#GPU">GPU</a> contexts, and is available across thread
boundaries. All contexts must be in the same <a href="undocumented#GPU_Share_Group">GPU Share Group</a>, or otherwise 
share resources.

When <a href="#Image">Image</a> is no longer referenced, <a href="#SkImage_MakeCrossContextFromEncoded_context">context</a> releases texture memory
asynchronously.

<a href="undocumented#Texture">Texture</a> decoded from <a href="#SkImage_MakeCrossContextFromEncoded_data">data</a> is uploaded to match <a href="SkSurface_Reference#Surface">Surface</a> created with
<a href="#SkImage_MakeCrossContextFromEncoded_dstColorSpace">dstColorSpace</a>. <a href="undocumented#Color_Space">Color Space</a> of <a href="#Image">Image</a> is determined by encoded <a href="#SkImage_MakeCrossContextFromEncoded_data">data</a>.

<a href="#Image">Image</a> is returned if format of <a href="#SkImage_MakeCrossContextFromEncoded_data">data</a> is recognized and supported, and if <a href="#SkImage_MakeCrossContextFromEncoded_context">context</a>
supports moving resources. Recognized formats vary by platform and <a href="undocumented#GPU">GPU</a> back-end.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeCrossContextFromEncoded_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeCrossContextFromEncoded_data"> <code><strong>data </strong></code> </a></td> <td>
<a href="#Image">Image</a> to decode</td>
  </tr>  <tr>    <td><a name="SkImage_MakeCrossContextFromEncoded_buildMips"> <code><strong>buildMips </strong></code> </a></td> <td>
create <a href="#Image">Image</a> as <a href="undocumented#Mip_Map">Mip Map</a> if true</td>
  </tr>  <tr>    <td><a name="SkImage_MakeCrossContextFromEncoded_dstColorSpace"> <code><strong>dstColorSpace </strong></code> </a></td> <td>
range of colors of matching <a href="SkSurface_Reference#Surface">Surface</a> on <a href="undocumented#GPU">GPU</a></td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

<a href="#SkImage_MakeCrossContextFromPixmap">MakeCrossContextFromPixmap</a>

---

<a name="SkImage_MakeCrossContextFromPixmap"></a>
## MakeCrossContextFromPixmap

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeCrossContextFromPixmap(GrContext* context, const SkPixmap& pixmap,
                                                 bool buildMips, SkColorSpace* dstColorSpace)
</pre>

Creates <a href="#Image">Image</a> from <a href="#SkImage_MakeCrossContextFromPixmap_pixmap">pixmap</a>. <a href="#Image">Image</a> is uploaded to <a href="undocumented#GPU">GPU</a> back-end using <a href="#SkImage_MakeCrossContextFromPixmap_context">context</a>. 

Created <a href="#Image">Image</a> is available to other <a href="undocumented#GPU">GPU</a> contexts, and is available across thread
boundaries. All contexts must be in the same <a href="undocumented#GPU_Share_Group">GPU Share Group</a>, or otherwise 
share resources.

When <a href="#Image">Image</a> is no longer referenced, <a href="#SkImage_MakeCrossContextFromPixmap_context">context</a> releases texture memory
asynchronously.

<a href="undocumented#Texture">Texture</a> created from <a href="#SkImage_MakeCrossContextFromPixmap_pixmap">pixmap</a> is uploaded to match <a href="SkSurface_Reference#Surface">Surface</a> created with
<a href="#SkImage_MakeCrossContextFromPixmap_dstColorSpace">dstColorSpace</a>. <a href="undocumented#Color_Space">Color Space</a> of <a href="#Image">Image</a> is determined by <a href="#SkImage_MakeCrossContextFromPixmap_pixmap">pixmap</a>.<a href="#SkImage_colorSpace">colorSpace</a>.

<a href="#Image">Image</a> is returned referring to <a href="undocumented#GPU">GPU</a> back-end if format of data is recognized and 
supported, and if <a href="#SkImage_MakeCrossContextFromPixmap_context">context</a> supports moving resources. Otherwise, <a href="#SkImage_MakeCrossContextFromPixmap_pixmap">pixmap</a> pixel
data is copied and <a href="#Image">Image</a> as returned in raster format if possible; nullptr may
be returned. Recognized <a href="undocumented#GPU">GPU</a> formats vary by platform and <a href="undocumented#GPU">GPU</a> back-end.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeCrossContextFromPixmap_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeCrossContextFromPixmap_pixmap"> <code><strong>pixmap </strong></code> </a></td> <td>
<a href="#Info">Image Info</a>, pixel address, and row bytes</td>
  </tr>  <tr>    <td><a name="SkImage_MakeCrossContextFromPixmap_buildMips"> <code><strong>buildMips </strong></code> </a></td> <td>
create <a href="#Image">Image</a> as <a href="undocumented#Mip_Map">Mip Map</a> if true</td>
  </tr>  <tr>    <td><a name="SkImage_MakeCrossContextFromPixmap_dstColorSpace"> <code><strong>dstColorSpace </strong></code> </a></td> <td>
range of colors of matching <a href="SkSurface_Reference#Surface">Surface</a> on <a href="undocumented#GPU">GPU</a></td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

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

Creates <a href="#Image">Image</a> from <a href="#SkImage_MakeFromAdoptedTexture_backendTexture">backendTexture</a> associated with <a href="#SkImage_MakeFromAdoptedTexture_context">context</a>. <a href="#SkImage_MakeFromAdoptedTexture_backendTexture">backendTexture</a> and
returned <a href="#Image">Image</a> are managed internally, and are released when no longer needed.

<a href="#Image">Image</a> is returned if format of <a href="#SkImage_MakeFromAdoptedTexture_backendTexture">backendTexture</a> is recognized and supported.
Recognized formats vary by <a href="undocumented#GPU">GPU</a> back-end.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromAdoptedTexture_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAdoptedTexture_backendTexture"> <code><strong>backendTexture </strong></code> </a></td> <td>
texture residing on <a href="undocumented#GPU">GPU</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAdoptedTexture_surfaceOrigin"> <code><strong>surfaceOrigin </strong></code> </a></td> <td>
one of: <a href="undocumented#GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>, <a href="undocumented#GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAdoptedTexture_alphaType"> <code><strong>alphaType </strong></code> </a></td> <td>
one of: <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>, <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>,
<a href="undocumented#SkAlphaType">kPremul SkAlphaType</a>, <a href="undocumented#SkAlphaType">kUnpremul SkAlphaType</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAdoptedTexture_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

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

Create a new image by copying the pixels from the specified y, u, v textures. The data
from the textures is immediately ingested into the image and the textures can be modified or
deleted after the function returns. The image will have the <a href="#SkImage_dimensions">dimensions</a> of the y texture.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_context"> <code><strong>context </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_yuvColorSpace"> <code><strong>yuvColorSpace </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_yuvTextureHandles"> <code><strong>yuvTextureHandles </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_yuvSizes"> <code><strong>yuvSizes </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_surfaceOrigin"> <code><strong>surfaceOrigin </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
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

<a name="SkImage_MakeFromNV12TexturesCopy"></a>
## MakeFromNV12TexturesCopy

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromNV12TexturesCopy(GrContext* context, SkYUVColorSpace yuvColorSpace,
                                               const GrBackendObject nv12TextureHandles[2],
                                               const SkISize nv12Sizes[2],
                                               GrSurfaceOrigin surfaceOrigin,
                                               sk_sp&lt;SkColorSpace&gt; colorSpace = nullptr)
</pre>

Create a new image by copying the pixels from the specified y and <a href="undocumented#UV_Mapping">UV Mapping</a>. The data
from the textures is immediately ingested into the image and the textures can be modified or
deleted after the function returns. The image will have the <a href="#SkImage_dimensions">dimensions</a> of the y texture.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_context"> <code><strong>context </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_yuvColorSpace"> <code><strong>yuvColorSpace </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_nv12TextureHandles"> <code><strong>nv12TextureHandles </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_nv12Sizes"> <code><strong>nv12Sizes </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_surfaceOrigin"> <code><strong>surfaceOrigin </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
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

## <a name="SkImage_BitDepth"></a> Enum SkImage::BitDepth

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum <a href="#SkImage_BitDepth">BitDepth</a> {
<a href="#SkImage_kU8">kU8</a>,
<a href="#SkImage_kF16">kF16</a>,
};</pre>

### Constants

<table>
  <tr>
    <td><a name="SkImage_kU8"> <code><strong>SkImage::kU8 </strong></code> </a></td><td>0</td><td></td>
  </tr>
  <tr>
    <td><a name="SkImage_kF16"> <code><strong>SkImage::kF16 </strong></code> </a></td><td>1</td><td></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete



<a name="SkImage_MakeFromPicture"></a>
## MakeFromPicture

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromPicture(sk_sp&lt;SkPicture&gt; picture, const SkISize& dimensions,
                                      const SkMatrix* matrix, const SkPaint* paint,
                                      BitDepth bitDepth, sk_sp&lt;SkColorSpace&gt; colorSpace)
</pre>

Create a new image from the specified <a href="#SkImage_MakeFromPicture_picture">picture</a>.
On creation of the <a href="#SkImage">SkImage</a>, snap the <a href="undocumented#SkPicture">SkPicture</a> to a particular <a href="#SkImage_BitDepth">BitDepth</a> and <a href="undocumented#SkColorSpace">SkColorSpace</a>.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromPicture_picture"> <code><strong>picture </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromPicture_dimensions"> <code><strong>dimensions </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromPicture_matrix"> <code><strong>matrix </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromPicture_paint"> <code><strong>paint </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromPicture_bitDepth"> <code><strong>bitDepth </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromPicture_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
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

<a name="SkImage_MakeFromAHardwareBuffer"></a>
## MakeFromAHardwareBuffer

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromAHardwareBuffer(AHardwareBuffer* hardwareBuffer,
                                            SkAlphaType alphaType = kPremul_SkAlphaType,
                                            sk_sp&lt;SkColorSpace&gt; colorSpace = nullptr)
</pre>

Create a new image from the an <a href="undocumented#Android">Android</a> hardware buffer.
The new image takes a reference on the buffer.

Only available on <a href="undocumented#Android">Android</a>, when __<a href="undocumented#ANDROID_API__">ANDROID API  </a> is defined to be 26 or greater.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromAHardwareBuffer_hardwareBuffer"> <code><strong>hardwareBuffer </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAHardwareBuffer_alphaType"> <code><strong>alphaType </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAHardwareBuffer_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
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
static sk_sp&lt;SkImage&gt; MakeFromAHardwareBuffer(AHardwareBuffer* hardwareBuffer,
                                            SkAlphaType alphaType = kPremul_SkAlphaType,
                                            sk_sp&lt;SkColorSpace&gt; colorSpace = nullptr)
</pre>

Create a new image from the an <a href="undocumented#Android">Android</a> hardware buffer.
The new image takes a reference on the buffer.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromAHardwareBuffer_2_hardwareBuffer"> <code><strong>hardwareBuffer </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAHardwareBuffer_2_alphaType"> <code><strong>alphaType </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAHardwareBuffer_2_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
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

<a name="SkImage_width"></a>
## width

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int width() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImage_height"></a>
## height

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int height() const
</pre>

Returns pixel row count.

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

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

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

<a href="#SkImage_height">height</a> <a href="#SkImage_width">width</a>

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

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImage_uniqueID"></a>
## uniqueID

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
uint32_t uniqueID() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImage_alphaType"></a>
## alphaType

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkAlphaType alphaType() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImage_colorSpace"></a>
## colorSpace

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkColorSpace* colorSpace() const
</pre>

Returns <a href="undocumented#Color_Space">Color Space</a>, the range of colors, associated with <a href="#Image">Image</a>.  The
reference count of <a href="undocumented#Color_Space">Color Space</a> is unchanged. The returned <a href="undocumented#Color_Space">Color Space</a> is
immutable.

<a href="undocumented#Color_Space">Color Space</a> returned was a parameter to an <a href="#Image">Image</a> constructor,
or was parsed from encoded data. <a href="undocumented#Color_Space">Color Space</a> may be ignored when
drawing <a href="#Image">Image</a>, and when drawing into <a href="SkSurface_Reference#Surface">Surface</a> constructed with <a href="undocumented#Color_Space">Color Space</a>.

### Return Value

<a href="undocumented#Color_Space">Color Space</a> in <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImage_refColorSpace"></a>
## refColorSpace

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkColorSpace&gt; refColorSpace() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

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

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImage_isOpaque"></a>
## isOpaque

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isOpaque() const
</pre>

Returns if all pixels ignore any <a href="#Alpha">Alpha</a> value and are treated as fully opaque.

### Return Value

true if <a href="#Alpha_Type">Alpha Type</a> is <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImage_makeShader"></a>
## makeShader

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkShader&gt; makeShader(SkShader::TileMode tileMode1, SkShader::TileMode tileMode2,
                           const SkMatrix* localMatrix = nullptr) const
</pre>

### Parameters

<table>  <tr>    <td><a name="SkImage_makeShader_tileMode1"> <code><strong>tileMode1 </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_makeShader_tileMode2"> <code><strong>tileMode2 </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_makeShader_localMatrix"> <code><strong>localMatrix </strong></code> </a></td> <td>
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
sk_sp&lt;SkShader&gt; makeShader(const SkMatrix* localMatrix = nullptr) const
</pre>

Helper version of

### Parameters

<table>  <tr>    <td><a name="SkImage_makeShader_2_localMatrix"> <code><strong>localMatrix </strong></code> </a></td> <td>
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
generator-backed images may be invalid for <a href="undocumented#CPU">CPU</a> and/or <a href="undocumented#GPU">GPU</a>.

### Parameters

<table>  <tr>    <td><a name="SkImage_isValid_context"> <code><strong>context </strong></code> </a></td> <td>
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

<a name="SkImage_getTextureHandle"></a>
## getTextureHandle

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
GrBackendObject getTextureHandle(bool flushPendingGrContextIO, GrSurfaceOrigin* origin = nullptr) const
</pre>

Retrieves the back-end <a href="undocumented#API">API</a> handle of texture. If <a href="#SkImage_getTextureHandle_flushPendingGrContextIO">flushPendingGrContextIO</a> is true,
complete deferred <a href="undocumented#I">I</a>/<a href="undocumented#O">O</a> operations.

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

back-end <a href="undocumented#API">API</a> texture handle

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
of decoding or a read-back from the <a href="undocumented#GPU">GPU</a>. Passing <a href="#SkImage_kAllow_CachingHint">kAllow CachingHint</a> signals that the system's default
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
sk_sp&lt;SkData&gt; encodeToData(SkPixelSerializer* pixelSerializer = nullptr) const
</pre>

Encodes <a href="#Image">Image</a> and returns result as <a href="undocumented#SkData">SkData</a>. Will reuse existing encoded data
if present, as returned by <a href="#SkImage_refEncodedData">refEncodedData</a>. <a href="#SkImage_encodeToData_2_pixelSerializer">pixelSerializer</a> validates existing
encoded data, and encodes <a href="#Image">Image</a> when existing encoded data is missing or
invalid.

Passing nullptr for <a href="#SkImage_encodeToData_2_pixelSerializer">pixelSerializer</a> selects default serialization which 
accepts all data and encodes to PNG.

Returns nullptr if existing encoded data is missing or invalid and 
encoding fails.

### Parameters

<table>  <tr>    <td><a name="SkImage_encodeToData_2_pixelSerializer"> <code><strong>pixelSerializer </strong></code> </a></td> <td>
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
incomplete</td>
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
Note that the <a href="#SkImage_BackendTextureReleaseProc">BackendTextureReleaseProc</a> allows <a href="undocumented#Skia">Skia</a> to clean up auxiliary data related
to the <a href="undocumented#GrBackendTexture">GrBackendTexture</a>, and is not a substitute for the client deleting the <a href="undocumented#GrBackendTexture">GrBackendTexture</a>
themselves.

If <a href="#SkImage_MakeBackendTextureFromSkImage_image">image</a> is both texture backed and singly referenced; that is, its only
reference was transferred using std::move(): <a href="#SkImage_MakeBackendTextureFromSkImage_image">image</a> is returned in <a href="#SkImage_MakeBackendTextureFromSkImage_backendTexture">backendTexture</a>
without conversion or making a copy. 

If the <a href="#SkImage">SkImage</a> is not texture backed, this function will generate a texture with the <a href="#SkImage_MakeBackendTextureFromSkImage_image">image</a>'s
contents and return that.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeBackendTextureFromSkImage_context"> <code><strong>context </strong></code> </a></td> <td>
incomplete</td>
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
treated as <a href="undocumented#sRGB">sRGB</a>.

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

