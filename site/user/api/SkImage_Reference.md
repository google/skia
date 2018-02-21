SkImage Reference
===

# <a name="Image"></a> Image

## <a name="Overview"></a> Overview

## <a name="Overview_Subtopic"></a> Overview Subtopic

| name | description |
| --- | --- |
| Constant | enum and enum class, const values |
| <a href="#Constructor">Constructor</a> | functions that construct <a href="#SkImage">SkImage</a> |
| <a href="#Member_Function">Member Function</a> | static functions and member methods |
| <a href="#Related_Function">Related Function</a> | similar methods grouped together |

# <a name="SkImage"></a> Class SkImage
<a href="#Image">Image</a> describes a two dimensional array of pixels to draw. The pixels may be
decoded in a <a href="undocumented#Raster_Bitmap">Raster Bitmap</a>, encoded in a <a href="undocumented#Picture">Picture</a> or compressed data stream,
or located in GPU memory as a <a href="undocumented#GPU_Texture">GPU Texture</a>.

<a href="#Image">Image</a> cannot be modified after it is created. <a href="#Image">Image</a> may allocate additional
storage as needed; for instance, an encoded <a href="#Image">Image</a> may decode when drawn.

<a href="#Image">Image</a> width and height are greater than zero. Creating an <a href="#Image">Image</a> with zero width
or height returns <a href="#Image">Image</a> equal to nullptr.

<a href="#Image">Image</a> may be created from <a href="SkBitmap_Reference#Bitmap">Bitmap</a>, <a href="SkPixmap_Reference#Pixmap">Pixmap</a>, <a href="SkSurface_Reference#Surface">Surface</a>, <a href="undocumented#Picture">Picture</a>, encoded streams,
<a href="undocumented#GPU_Texture">GPU Texture</a>, <a href="SkImageInfo_Reference#YUV_ColorSpace">YUV ColorSpace</a> data, or hardware buffer. Encoded streams supported
include BMP, GIF, HEIF, ICO, JPEG, PNG, WBMP, WebP. Supported encoding details
vary with platform.

## <a name="Raster_Image"></a> Raster Image

<a href="#Raster_Image">Raster Image</a> pixels are decoded in a <a href="undocumented#Raster_Bitmap">Raster Bitmap</a>. These pixels may be read
directly and in most cases written to, although edited pixels may not be drawn
if <a href="#Image">Image</a> has been copied internally.

## <a name="Texture_Image"></a> Texture Image

<a href="#Texture_Image">Texture Image</a> are located on GPU and pixels are not accessible. <a href="#Texture_Image">Texture Image</a>
are allocated optimally for best performance. <a href="#Raster_Image">Raster Image</a> may
be drawn to <a href="undocumented#GPU_Surface">GPU Surface</a>, but pixels are uploaded from CPU to GPU downgrading
performance.

## <a name="Lazy_Image"></a> Lazy Image

<a href="#Lazy_Image">Lazy Image</a> defer allocating buffer for <a href="#Image">Image</a> pixels and decoding stream until
<a href="#Image">Image</a> is drawn. <a href="#Lazy_Image">Lazy Image</a> caches result if possible to speed up repeated
drawing.

## <a name="Related_Function"></a> Related Function

| name | description |
| --- | --- |
| <a href="#Lazy_Image">Lazy Image</a> | deferred pixel buffer |
| <a href="#Pixels">Pixels</a> | read and write pixel values |
| <a href="#Property">Property</a> | values and attributes |
| <a href="#Raster_Image">Raster Image</a> | pixels decoded in <a href="undocumented#Raster_Bitmap">Raster Bitmap</a> |
| <a href="#Texture_Image">Texture Image</a> | pixels located on GPU |
| <a href="#Utility">Utility</a> | rarely called management functions |

## <a name="Class_or_Struct"></a> Class or Struct

| name | description |
| --- | --- |

## <a name="Constructor"></a> Constructor

| name | description |
| --- | --- |
| <a href="#SkImage_MakeBackendTextureFromSkImage">MakeBackendTextureFromSkImage</a> | creates <a href="undocumented#GPU_Texture">GPU Texture</a> from <a href="#Image">Image</a> |
| <a href="#SkImage_MakeCrossContextFromEncoded">MakeCrossContextFromEncoded</a> | creates <a href="#Image">Image</a> from encoded data, and uploads to GPU |
| <a href="#SkImage_MakeCrossContextFromPixmap">MakeCrossContextFromPixmap</a> | creates <a href="#Image">Image</a> from <a href="SkPixmap_Reference#Pixmap">Pixmap</a>, and uploads to GPU |
| <a href="#SkImage_MakeFromAHardwareBuffer">MakeFromAHardwareBuffer</a> | creates <a href="#Image">Image</a> from Android hardware buffer |
| <a href="#SkImage_MakeFromAdoptedTexture_2">MakeFromAdoptedTexture 2</a> | creates <a href="#Image">Image</a> from <a href="undocumented#GPU_Texture">GPU Texture</a>, managed internally |
| <a href="#SkImage_MakeFromBitmap">MakeFromBitmap</a> | creates <a href="#Image">Image</a> from <a href="SkBitmap_Reference#Bitmap">Bitmap</a>, sharing or copying pixels |
| <a href="#SkImage_MakeFromEncoded">MakeFromEncoded</a> | creates <a href="#Image">Image</a> from encoded data |
| <a href="#SkImage_MakeFromGenerator">MakeFromGenerator</a> | creates <a href="#Image">Image</a> from a stream of data |
| <a href="#SkImage_MakeFromNV12TexturesCopy">MakeFromNV12TexturesCopy</a> | creates <a href="#Image">Image</a> from <a href="SkImageInfo_Reference#YUV_ColorSpace">YUV ColorSpace</a> data in two planes |
|  | <a href="#SkImage_MakeFromNV12TexturesCopy">MakeFromNV12TexturesCopy(GrContext* context, SkYUVColorSpace yuvColorSpace, const GrBackendObject nv12TextureHandles[2], const SkISize nv12Sizes[2], GrSurfaceOrigin surfaceOrigin, sk sp&lt;SkColorSpace&gt; colorSpace = nullptr)</a> |
|  | <a href="#SkImage_MakeFromNV12TexturesCopy_2">MakeFromNV12TexturesCopy(GrContext* context, SkYUVColorSpace yuvColorSpace, const GrBackendTexture nv12TextureHandles[2], const SkISize nv12Sizes[2], GrSurfaceOrigin surfaceOrigin, sk sp&lt;SkColorSpace&gt; colorSpace = nullptr)</a> |
| <a href="#SkImage_MakeFromPicture">MakeFromPicture</a> | creates <a href="#Image">Image</a> from <a href="undocumented#Picture">Picture</a> |
| <a href="#SkImage_MakeFromRaster">MakeFromRaster</a> | creates <a href="#Image">Image</a> from <a href="SkPixmap_Reference#Pixmap">Pixmap</a>, with release |
| <a href="#SkImage_MakeFromTexture_3">MakeFromTexture 3</a> | creates <a href="#Image">Image</a> from <a href="undocumented#GPU_Texture">GPU Texture</a> |
|  | static <a href="undocumented#sk_sp">sk sp</a>&lt;<a href="#SkImage">SkImage</a>&gt; <a href="#SkImage_MakeFromTexture_3">MakeFromTexture(GrContext* context, const GrBackendTexture& backendTexture, GrSurfaceOrigin origin, SkColorType colorType, SkAlphaType alphaType, sk sp&lt;SkColorSpace&gt; colorSpace)</a> |
| <a href="#SkImage_MakeFromYUVTexturesCopy">MakeFromYUVTexturesCopy</a> | creates <a href="#Image">Image</a> from <a href="SkImageInfo_Reference#YUV_ColorSpace">YUV ColorSpace</a> data in three planes |
|  | <a href="#SkImage_MakeFromYUVTexturesCopy">MakeFromYUVTexturesCopy(GrContext* context, SkYUVColorSpace yuvColorSpace, const GrBackendObject yuvTextureHandles[3], const SkISize yuvSizes[3], GrSurfaceOrigin surfaceOrigin, sk sp&lt;SkColorSpace&gt; colorSpace = nullptr)</a> |
|  | <a href="#SkImage_MakeFromYUVTexturesCopy_2">MakeFromYUVTexturesCopy(GrContext* context, SkYUVColorSpace yuvColorSpace, const GrBackendTexture yuvTextureHandles[3], const SkISize yuvSizes[3], GrSurfaceOrigin surfaceOrigin, sk sp&lt;SkColorSpace&gt; colorSpace = nullptr)</a> |
| <a href="#SkImage_MakeRasterCopy">MakeRasterCopy</a> | creates <a href="#Image">Image</a> from <a href="SkPixmap_Reference#Pixmap">Pixmap</a> and copied pixels |
| <a href="#SkImage_MakeRasterData">MakeRasterData</a> | creates <a href="#Image">Image</a> from <a href="SkImageInfo_Reference#Image_Info">Image Info</a> and shared pixels |
| <a href="#SkImage_asLegacyBitmap">asLegacyBitmap</a> | returns as <a href="undocumented#Raster_Bitmap">Raster Bitmap</a> |
| <a href="#SkImage_makeColorSpace">makeColorSpace</a> | creates <a href="#Image">Image</a> matching <a href="undocumented#Color_Space">Color Space</a> if possible |
| <a href="#SkImage_makeNonTextureImage">makeNonTextureImage</a> | creates <a href="#Image">Image</a> without dependency on <a href="undocumented#GPU_Texture">GPU Texture</a> |
| <a href="#SkImage_makeRasterImage">makeRasterImage</a> | creates <a href="#Image">Image</a> compatible with <a href="undocumented#Raster_Surface">Raster Surface</a> if possible |
| <a href="#SkImage_makeShader">makeShader</a> | creates <a href="undocumented#Shader">Shader</a>, <a href="SkPaint_Reference#Paint">Paint</a> element that can tile <a href="#Image">Image</a> |
|  | <a href="#SkImage_makeShader">makeShader(SkShader::TileMode tileMode1, SkShader::TileMode tileMode2, const SkMatrix* localMatrix = nullptr)</a> const |
|  | <a href="#SkImage_makeShader_2">makeShader(const SkMatrix* localMatrix = nullptr)</a> const |
| <a href="#SkImage_makeSubset">makeSubset</a> | creates <a href="#Image">Image</a> containing part of original |
| <a href="#SkImage_makeTextureImage">makeTextureImage</a> | creates <a href="#Image">Image</a> matching <a href="undocumented#Color_Space">Color Space</a> if possible |
| <a href="#SkImage_makeWithFilter">makeWithFilter</a> | creates filtered, clipped <a href="#Image">Image</a> |

## <a name="Member_Function"></a> Member Function

| name | description |
| --- | --- |
| <a href="#SkImage_MakeBackendTextureFromSkImage">MakeBackendTextureFromSkImage</a> | creates <a href="undocumented#GPU_Texture">GPU Texture</a> from <a href="#Image">Image</a> |
| <a href="#SkImage_MakeCrossContextFromEncoded">MakeCrossContextFromEncoded</a> | creates <a href="#Image">Image</a> from encoded data, and uploads to GPU |
| <a href="#SkImage_MakeCrossContextFromPixmap">MakeCrossContextFromPixmap</a> | creates <a href="#Image">Image</a> from <a href="SkPixmap_Reference#Pixmap">Pixmap</a>, and uploads to GPU |
| <a href="#SkImage_MakeFromAHardwareBuffer">MakeFromAHardwareBuffer</a> | creates <a href="#Image">Image</a> from Android hardware buffer |
| <a href="#SkImage_MakeFromAdoptedTexture_2">MakeFromAdoptedTexture 2</a> | creates <a href="#Image">Image</a> from <a href="undocumented#GPU_Texture">GPU Texture</a>, managed internally |
| <a href="#SkImage_MakeFromBitmap">MakeFromBitmap</a> | creates <a href="#Image">Image</a> from <a href="SkBitmap_Reference#Bitmap">Bitmap</a>, sharing or copying pixels |
| <a href="#SkImage_MakeFromEncoded">MakeFromEncoded</a> | creates <a href="#Image">Image</a> from encoded data |
| <a href="#SkImage_MakeFromGenerator">MakeFromGenerator</a> | creates <a href="#Image">Image</a> from a stream of data |
| <a href="#SkImage_MakeFromNV12TexturesCopy">MakeFromNV12TexturesCopy</a> | creates <a href="#Image">Image</a> from <a href="SkImageInfo_Reference#YUV_ColorSpace">YUV ColorSpace</a> data in two planes |
| <a href="#SkImage_MakeFromPicture">MakeFromPicture</a> | creates <a href="#Image">Image</a> from <a href="undocumented#Picture">Picture</a> |
| <a href="#SkImage_MakeFromRaster">MakeFromRaster</a> | creates <a href="#Image">Image</a> from <a href="SkPixmap_Reference#Pixmap">Pixmap</a>, with release |
| <a href="#SkImage_MakeFromTexture_3">MakeFromTexture 3</a> | creates <a href="#Image">Image</a> from <a href="undocumented#GPU_Texture">GPU Texture</a> |
| <a href="#SkImage_MakeFromYUVTexturesCopy">MakeFromYUVTexturesCopy</a> | creates <a href="#Image">Image</a> from <a href="SkImageInfo_Reference#YUV_ColorSpace">YUV ColorSpace</a> data in three planes |
| <a href="#SkImage_MakeRasterCopy">MakeRasterCopy</a> | creates <a href="#Image">Image</a> from <a href="SkPixmap_Reference#Pixmap">Pixmap</a> and copied pixels |
| <a href="#SkImage_MakeRasterData">MakeRasterData</a> | creates <a href="#Image">Image</a> from <a href="SkImageInfo_Reference#Image_Info">Image Info</a> and shared pixels |
| <a href="#SkImage_alphaType">alphaType</a> | returns <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a> |
| <a href="#SkImage_asLegacyBitmap">asLegacyBitmap</a> | returns as <a href="undocumented#Raster_Bitmap">Raster Bitmap</a> |
| <a href="#SkImage_bounds">bounds</a> | returns <a href="#SkImage_width">width</a> and <a href="#SkImage_height">height</a> as Rectangle |
| <a href="#SkImage_colorSpace">colorSpace</a> | returns <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkImage_dimensions">dimensions</a> | returns <a href="#SkImage_width">width</a> and <a href="#SkImage_height">height</a> |
| <a href="#SkImage_encodeToData">encodeToData</a> | returns encoded <a href="#Image">Image</a> as <a href="undocumented#SkData">SkData</a> |
| <a href="#SkImage_getTextureHandle">getTextureHandle</a> | returns GPU reference to <a href="#Image">Image</a> as texture |
| <a href="#SkImage_height">height</a> | returns pixel row count |
| <a href="#SkImage_isAlphaOnly">isAlphaOnly</a> | returns if pixels represent a transparency mask |
| <a href="#SkImage_isLazyGenerated">isLazyGenerated</a> | returns if <a href="#Image">Image</a> is created as needed |
| <a href="#SkImage_isOpaque">isOpaque</a> | returns if <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a> is <a href="SkImageInfo_Reference#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a> |
| <a href="#SkImage_isTextureBacked">isTextureBacked</a> | returns if <a href="#Image">Image</a> was created from <a href="undocumented#GPU_Texture">GPU Texture</a> |
| <a href="#SkImage_isValid">isValid</a> | returns if <a href="#Image">Image</a> can draw to <a href="undocumented#Raster_Surface">Raster Surface</a> or <a href="undocumented#GPU_Context">GPU Context</a> |
| <a href="#SkImage_makeColorSpace">makeColorSpace</a> | creates <a href="#Image">Image</a> matching <a href="undocumented#Color_Space">Color Space</a> if possible |
| <a href="#SkImage_makeNonTextureImage">makeNonTextureImage</a> | creates <a href="#Image">Image</a> without dependency on <a href="undocumented#GPU_Texture">GPU Texture</a> |
| <a href="#SkImage_makeRasterImage">makeRasterImage</a> | creates <a href="#Image">Image</a> compatible with <a href="undocumented#Raster_Surface">Raster Surface</a> if possible |
| <a href="#SkImage_makeShader">makeShader</a> | creates <a href="undocumented#Shader">Shader</a>, <a href="SkPaint_Reference#Paint">Paint</a> element that can tile <a href="#Image">Image</a> |
| <a href="#SkImage_makeSubset">makeSubset</a> | creates <a href="#Image">Image</a> containing part of original |
| <a href="#SkImage_makeTextureImage">makeTextureImage</a> | creates <a href="#Image">Image</a> matching <a href="undocumented#Color_Space">Color Space</a> if possible |
| <a href="#SkImage_makeWithFilter">makeWithFilter</a> | creates filtered, clipped <a href="#Image">Image</a> |
| <a href="#SkImage_peekPixels">peekPixels</a> | returns <a href="SkPixmap_Reference#Pixmap">Pixmap</a> if possible |
| <a href="#SkImage_readPixels">readPixels</a> | copies and converts pixels |
| <a href="#SkImage_refColorSpace">refColorSpace</a> | returns <a href="SkImageInfo_Reference#Image_Info">Image Info</a> <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkImage_refEncodedData">refEncodedData</a> | returns <a href="#Image">Image</a> encoded in <a href="undocumented#SkData">SkData</a> if present |
| <a href="#SkImage_scalePixels">scalePixels</a> | scales and converts one <a href="#Image">Image</a> to another |
| <a href="#SkImage_toString">toString</a> | converts <a href="#Image">Image</a> to machine readable form |
| <a href="#SkImage_uniqueID">uniqueID</a> | identifier for <a href="#Image">Image</a> |
| <a href="#SkImage_width">width</a> | returns pixel column count |

<a name="SkImage_MakeRasterCopy"></a>
## MakeRasterCopy

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeRasterCopy(const SkPixmap& pixmap)
</pre>

Creates <a href="#Image">Image</a> from <a href="SkPixmap_Reference#Pixmap">Pixmap</a> and copy of pixels. Since pixels are copied, <a href="SkPixmap_Reference#Pixmap">Pixmap</a>
pixels may be modified or deleted without affecting <a href="#Image">Image</a>.

<a href="#Image">Image</a> is returned if <a href="SkPixmap_Reference#Pixmap">Pixmap</a> is valid. Valid <a href="SkPixmap_Reference#Pixmap">Pixmap</a> parameters include:
dimensions are greater than zero;
each dimension fits in 29 bits;
<a href="SkImageInfo_Reference#Color_Type">Color Type</a> and <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a> are valid, and <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is not <a href="SkImageInfo_Reference#kUnknown_SkColorType">kUnknown_SkColorType</a>;
row bytes are large enough to hold one row of pixels;
pixel address is not nullptr.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeRasterCopy_pixmap"> <code><strong>pixmap </strong></code> </a></td> <td>
<a href="SkImageInfo_Reference#Image_Info">Image Info</a>, pixel address, and row bytes</td>
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

Creates <a href="#Image">Image</a> from <a href="SkImageInfo_Reference#Image_Info">Image Info</a>, sharing <a href="#SkImage_MakeRasterData_pixels">pixels</a>.

<a href="#Image">Image</a> is returned if <a href="SkImageInfo_Reference#Image_Info">Image Info</a> is valid. Valid <a href="SkImageInfo_Reference#Image_Info">Image Info</a> parameters include:
dimensions are greater than zero;
each dimension fits in 29 bits;
<a href="SkImageInfo_Reference#Color_Type">Color Type</a> and <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a> are valid, and <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is not <a href="SkImageInfo_Reference#kUnknown_SkColorType">kUnknown_SkColorType</a>;
<a href="#SkImage_MakeRasterData_rowBytes">rowBytes</a> are large enough to hold one row of <a href="#SkImage_MakeRasterData_pixels">pixels</a>;
<a href="#SkImage_MakeRasterData_pixels">pixels</a> is not nullptr, and contains enough data for <a href="#Image">Image</a>.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeRasterData_info"> <code><strong>info </strong></code> </a></td> <td>
contains width, height, <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a>, <a href="SkImageInfo_Reference#Color_Type">Color Type</a>, <a href="undocumented#Color_Space">Color Space</a></td>
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

Creates <a href="#Image">Image</a> from <a href="#SkImage_MakeFromRaster_pixmap">pixmap</a>, sharing <a href="SkPixmap_Reference#Pixmap">Pixmap</a> pixels. <a href="#Pixels">Pixels</a> must remain valid and
unchanged until <a href="#SkImage_MakeFromRaster_rasterReleaseProc">rasterReleaseProc</a> is called. <a href="#SkImage_MakeFromRaster_rasterReleaseProc">rasterReleaseProc</a> is passed
<a href="#SkImage_MakeFromRaster_releaseContext">releaseContext</a> when <a href="#Image">Image</a> is deleted or no longer refers to <a href="#SkImage_MakeFromRaster_pixmap">pixmap</a> pixels.

Pass nullptr for <a href="#SkImage_MakeFromRaster_rasterReleaseProc">rasterReleaseProc</a> to share <a href="SkPixmap_Reference#Pixmap">Pixmap</a> without requiring a callback
when <a href="#Image">Image</a> is released. Pass nullptr for <a href="#SkImage_MakeFromRaster_releaseContext">releaseContext</a> if <a href="#SkImage_MakeFromRaster_rasterReleaseProc">rasterReleaseProc</a>
does not require state.

<a href="#Image">Image</a> is returned if <a href="#SkImage_MakeFromRaster_pixmap">pixmap</a> is valid. Valid <a href="SkPixmap_Reference#Pixmap">Pixmap</a> parameters include:
dimensions are greater than zero;
each dimension fits in 29 bits;
<a href="SkImageInfo_Reference#Color_Type">Color Type</a> and <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a> are valid, and <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is not <a href="SkImageInfo_Reference#kUnknown_SkColorType">kUnknown_SkColorType</a>;
row bytes are large enough to hold one row of pixels;
pixel address is not nullptr.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromRaster_pixmap"> <code><strong>pixmap </strong></code> </a></td> <td>
<a href="SkImageInfo_Reference#Image_Info">Image Info</a>, pixel address, and row bytes</td>
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
dimensions are greater than zero;
each dimension fits in 29 bits;
<a href="SkImageInfo_Reference#Color_Type">Color Type</a> and <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a> are valid, and <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is not <a href="SkImageInfo_Reference#kUnknown_SkColorType">kUnknown_SkColorType</a>;
row bytes are large enough to hold one row of pixels;
pixel address is not nullptr.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromBitmap_bitmap"> <code><strong>bitmap </strong></code> </a></td> <td>
<a href="SkImageInfo_Reference#Image_Info">Image Info</a>, row bytes, and pixels</td>
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
otherwise, <a href="#SkImage_MakeFromGenerator_subset">subset</a> must be contained by image bounds.

<a href="#Image">Image</a> is returned if generator data is valid. Valid data parameters vary by type of data
and platform.

<a href="#SkImage_MakeFromGenerator_imageGenerator">imageGenerator</a> may wrap <a href="undocumented#Picture">Picture</a> data, codec data, or custom data.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromGenerator_imageGenerator"> <code><strong>imageGenerator </strong></code> </a></td> <td>
stock or custom routines to retrieve <a href="#Image">Image</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromGenerator_subset"> <code><strong>subset </strong></code> </a></td> <td>
bounds of returned <a href="#Image">Image</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="c2fec0746f88ca34d7dce59dd9bdef9e"><div>The generator returning <a href="undocumented#Picture">Picture</a> cannot be shared; std::move transfers ownership to generated <a href="#Image">Image</a>.</div></fiddle-embed></div>

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
otherwise, <a href="#SkImage_MakeFromEncoded_subset">subset</a> must be contained by image bounds.

<a href="#Image">Image</a> is returned if format of the <a href="#SkImage_MakeFromEncoded_encoded">encoded</a> data is recognized and supported.
Recognized formats vary by platform.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromEncoded_encoded"> <code><strong>encoded </strong></code> </a></td> <td>
data of <a href="#Image">Image</a> to decode</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromEncoded_subset"> <code><strong>subset </strong></code> </a></td> <td>
bounds of returned <a href="#Image">Image</a>; may be nullptr</td>
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

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromTexture(GrContext* context, const GrBackendTexture& backendTexture,
                                      GrSurfaceOrigin origin, SkAlphaType alphaType,
                                      sk_sp&lt;SkColorSpace&gt; colorSpace)
</pre>

---

<a name="SkImage_MakeFromTexture_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromTexture(GrContext* context, const GrBackendTexture& backendTexture,
                                      GrSurfaceOrigin origin, SkAlphaType alphaType,
                                      sk_sp&lt;SkColorSpace&gt; colorSpace,
                                      TextureReleaseProc textureReleaseProc,
                                      ReleaseContext releaseContext)
</pre>

---

<a name="SkImage_MakeFromTexture_3"></a>
## MakeFromTexture_3

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
one of: <a href="undocumented#kBottomLeft_GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>, <a href="undocumented#kTopLeft_GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_3_colorType"> <code><strong>colorType </strong></code> </a></td> <td>
one of: <a href="SkImageInfo_Reference#kUnknown_SkColorType">kUnknown_SkColorType</a>, <a href="SkImageInfo_Reference#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>,
<a href="SkImageInfo_Reference#kRGB_565_SkColorType">kRGB_565_SkColorType</a>, <a href="SkImageInfo_Reference#kARGB_4444_SkColorType">kARGB_4444_SkColorType</a>,
<a href="SkImageInfo_Reference#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a>, <a href="SkImageInfo_Reference#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>,
<a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, <a href="SkImageInfo_Reference#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_3_alphaType"> <code><strong>alphaType </strong></code> </a></td> <td>
one of: <a href="SkImageInfo_Reference#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a>, <a href="SkImageInfo_Reference#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>,
<a href="SkImageInfo_Reference#kPremul_SkAlphaType">kPremul_SkAlphaType</a>, <a href="SkImageInfo_Reference#kUnpremul_SkAlphaType">kUnpremul_SkAlphaType</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_3_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="fdc498de45b53569743ec13012bf476c" gpu="true"><div>A back-end texture has been created and uploaded to the GPU outside of this example.</div></fiddle-embed></div>

### See Also

<a href="#SkImage_MakeFromAdoptedTexture">MakeFromAdoptedTexture</a> <a href="SkSurface_Reference#SkSurface_MakeFromBackendTexture">SkSurface::MakeFromBackendTexture</a><sup><a href="SkSurface_Reference#SkSurface_MakeFromBackendTexture_2">[2]</a></sup>

---

<a name="SkImage_MakeFromTexture_4"></a>

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
one of: <a href="undocumented#kBottomLeft_GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>, <a href="undocumented#kTopLeft_GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_4_colorType"> <code><strong>colorType </strong></code> </a></td> <td>
one of: <a href="SkImageInfo_Reference#kUnknown_SkColorType">kUnknown_SkColorType</a>, <a href="SkImageInfo_Reference#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>,
<a href="SkImageInfo_Reference#kRGB_565_SkColorType">kRGB_565_SkColorType</a>, <a href="SkImageInfo_Reference#kARGB_4444_SkColorType">kARGB_4444_SkColorType</a>,
<a href="SkImageInfo_Reference#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a>, <a href="SkImageInfo_Reference#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>,
<a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, <a href="SkImageInfo_Reference#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_4_alphaType"> <code><strong>alphaType </strong></code> </a></td> <td>
one of: <a href="SkImageInfo_Reference#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a>, <a href="SkImageInfo_Reference#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>,
<a href="SkImageInfo_Reference#kPremul_SkAlphaType">kPremul_SkAlphaType</a>, <a href="SkImageInfo_Reference#kUnpremul_SkAlphaType">kUnpremul_SkAlphaType</a></td>
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

<div><fiddle-embed name="c7be9423f7c2ef819523ba4d607d17b8" gpu="true"></fiddle-embed></div>

### See Also

<a href="#SkImage_MakeFromAdoptedTexture">MakeFromAdoptedTexture</a> <a href="SkSurface_Reference#SkSurface_MakeFromBackendTexture">SkSurface::MakeFromBackendTexture</a><sup><a href="SkSurface_Reference#SkSurface_MakeFromBackendTexture_2">[2]</a></sup>

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
create <a href="#Image">Image</a> as Mip_Map if true</td>
  </tr>  <tr>    <td><a name="SkImage_MakeCrossContextFromEncoded_dstColorSpace"> <code><strong>dstColorSpace </strong></code> </a></td> <td>
range of colors of matching <a href="SkSurface_Reference#Surface">Surface</a> on GPU</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="069c7b116479e3ca46f953f07dcbdd36"></fiddle-embed></div>

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
<a href="SkImageInfo_Reference#Image_Info">Image Info</a>, pixel address, and row bytes</td>
  </tr>  <tr>    <td><a name="SkImage_MakeCrossContextFromPixmap_buildMips"> <code><strong>buildMips </strong></code> </a></td> <td>
create <a href="#Image">Image</a> as Mip_Map if true</td>
  </tr>  <tr>    <td><a name="SkImage_MakeCrossContextFromPixmap_dstColorSpace"> <code><strong>dstColorSpace </strong></code> </a></td> <td>
range of colors of matching <a href="SkSurface_Reference#Surface">Surface</a> on GPU</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="45bca8747b8f49b5be34b520897ef048"></fiddle-embed></div>

### See Also

<a href="#SkImage_MakeCrossContextFromEncoded">MakeCrossContextFromEncoded</a>

---

<a name="SkImage_MakeFromAdoptedTexture"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromAdoptedTexture(GrContext* context,
                                             const GrBackendTexture& backendTexture,
                                             GrSurfaceOrigin surfaceOrigin,
                                             SkAlphaType alphaType = kPremul_SkAlphaType,
                                             sk_sp&lt;SkColorSpace&gt; colorSpace = nullptr)
</pre>

---

<a name="SkImage_MakeFromAdoptedTexture_2"></a>
## MakeFromAdoptedTexture_2

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
one of: <a href="undocumented#kBottomLeft_GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>, <a href="undocumented#kTopLeft_GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAdoptedTexture_2_colorType"> <code><strong>colorType </strong></code> </a></td> <td>
one of: <a href="SkImageInfo_Reference#kUnknown_SkColorType">kUnknown_SkColorType</a>, <a href="SkImageInfo_Reference#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>,
<a href="SkImageInfo_Reference#kRGB_565_SkColorType">kRGB_565_SkColorType</a>, <a href="SkImageInfo_Reference#kARGB_4444_SkColorType">kARGB_4444_SkColorType</a>,
<a href="SkImageInfo_Reference#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a>, <a href="SkImageInfo_Reference#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>,
<a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, <a href="SkImageInfo_Reference#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAdoptedTexture_2_alphaType"> <code><strong>alphaType </strong></code> </a></td> <td>
one of: <a href="SkImageInfo_Reference#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a>, <a href="SkImageInfo_Reference#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>,
<a href="SkImageInfo_Reference#kPremul_SkAlphaType">kPremul_SkAlphaType</a>, <a href="SkImageInfo_Reference#kUnpremul_SkAlphaType">kUnpremul_SkAlphaType</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAdoptedTexture_2_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="b07964ec9c5c8a6febba805f1cf4d071" gpu="true"></fiddle-embed></div>

### See Also

<a href="#SkImage_MakeFromTexture">MakeFromTexture</a> <a href="#SkImage_MakeFromYUVTexturesCopy">MakeFromYUVTexturesCopy</a><sup><a href="#SkImage_MakeFromYUVTexturesCopy_2">[2]</a></sup>

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
<a href="#SkImage_MakeFromYUVTexturesCopy_yuvSizes">yuvSizes</a> contain dimensions for each pixel plane. Dimensions must be greater than
zero but may differ from plane to plane. Returned <a href="#Image">Image</a> has the dimensions
<a href="#SkImage_MakeFromYUVTexturesCopy_yuvSizes">yuvSizes</a>[0]. <a href="#SkImage_MakeFromYUVTexturesCopy_yuvColorSpace">yuvColorSpace</a> describes how YUV colors convert to RGB colors.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_yuvColorSpace"> <code><strong>yuvColorSpace </strong></code> </a></td> <td>
one of: <a href="SkImageInfo_Reference#kJPEG_SkYUVColorSpace">kJPEG_SkYUVColorSpace</a>, <a href="SkImageInfo_Reference#kRec601_SkYUVColorSpace">kRec601_SkYUVColorSpace</a>,
<a href="SkImageInfo_Reference#kRec709_SkYUVColorSpace">kRec709_SkYUVColorSpace</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_yuvTextureHandles"> <code><strong>yuvTextureHandles </strong></code> </a></td> <td>
array of YUV textures on GPU</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_yuvSizes"> <code><strong>yuvSizes </strong></code> </a></td> <td>
dimensions of YUV textures</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_surfaceOrigin"> <code><strong>surfaceOrigin </strong></code> </a></td> <td>
one of: <a href="undocumented#kBottomLeft_GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>, <a href="undocumented#kTopLeft_GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### See Also

<a href="#SkImage_MakeFromNV12TexturesCopy">MakeFromNV12TexturesCopy</a><sup><a href="#SkImage_MakeFromNV12TexturesCopy_2">[2]</a></sup>

---

<a name="SkImage_MakeFromYUVTexturesCopy_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromYUVTexturesCopy(GrContext* context, SkYUVColorSpace yuvColorSpace,
                                              const GrBackendTexture yuvTextureHandles[3],
                                              const SkISize yuvSizes[3],
                                              GrSurfaceOrigin surfaceOrigin,
                                              sk_sp&lt;SkColorSpace&gt; colorSpace = nullptr)
</pre>

Creates <a href="#Image">Image</a> from copy of <a href="#SkImage_MakeFromYUVTexturesCopy_2_yuvTextureHandles">yuvTextureHandles</a>, an array of textures on GPU.
<a href="#SkImage_MakeFromYUVTexturesCopy_2_yuvTextureHandles">yuvTextureHandles</a> contain pixels for YUV planes of <a href="#Image">Image</a>.
<a href="#SkImage_MakeFromYUVTexturesCopy_2_yuvSizes">yuvSizes</a> contain dimensions for each pixel plane. Dimensions must be greater than
zero but may differ from plane to plane. Returned <a href="#Image">Image</a> has the dimensions
<a href="#SkImage_MakeFromYUVTexturesCopy_2_yuvSizes">yuvSizes</a>[0]. <a href="#SkImage_MakeFromYUVTexturesCopy_2_yuvColorSpace">yuvColorSpace</a> describes how YUV colors convert to RGB colors.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_2_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_2_yuvColorSpace"> <code><strong>yuvColorSpace </strong></code> </a></td> <td>
one of: <a href="SkImageInfo_Reference#kJPEG_SkYUVColorSpace">kJPEG_SkYUVColorSpace</a>, <a href="SkImageInfo_Reference#kRec601_SkYUVColorSpace">kRec601_SkYUVColorSpace</a>,
<a href="SkImageInfo_Reference#kRec709_SkYUVColorSpace">kRec709_SkYUVColorSpace</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_2_yuvTextureHandles"> <code><strong>yuvTextureHandles </strong></code> </a></td> <td>
array of YUV textures on GPU</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_2_yuvSizes"> <code><strong>yuvSizes </strong></code> </a></td> <td>
dimensions of YUV textures</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_2_surfaceOrigin"> <code><strong>surfaceOrigin </strong></code> </a></td> <td>
one of: <a href="undocumented#kBottomLeft_GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>, <a href="undocumented#kTopLeft_GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromYUVTexturesCopy_2_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### See Also

<a href="#SkImage_MakeFromNV12TexturesCopy">MakeFromNV12TexturesCopy</a><sup><a href="#SkImage_MakeFromNV12TexturesCopy_2">[2]</a></sup>

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
<a href="#SkImage_MakeFromNV12TexturesCopy_nv12Sizes">nv12Sizes</a> contain dimensions for each pixel plane. Dimensions must be greater than
zero but may differ from plane to plane. Returned <a href="#Image">Image</a> has the dimensions
<a href="#SkImage_MakeFromNV12TexturesCopy_nv12Sizes">nv12Sizes</a>[0]. <a href="#SkImage_MakeFromNV12TexturesCopy_yuvColorSpace">yuvColorSpace</a> describes how YUV colors convert to RGB colors.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_yuvColorSpace"> <code><strong>yuvColorSpace </strong></code> </a></td> <td>
one of: <a href="SkImageInfo_Reference#kJPEG_SkYUVColorSpace">kJPEG_SkYUVColorSpace</a>, <a href="SkImageInfo_Reference#kRec601_SkYUVColorSpace">kRec601_SkYUVColorSpace</a>,
<a href="SkImageInfo_Reference#kRec709_SkYUVColorSpace">kRec709_SkYUVColorSpace</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_nv12TextureHandles"> <code><strong>nv12TextureHandles </strong></code> </a></td> <td>
array of YUV textures on GPU</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_nv12Sizes"> <code><strong>nv12Sizes </strong></code> </a></td> <td>
dimensions of YUV textures</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_surfaceOrigin"> <code><strong>surfaceOrigin </strong></code> </a></td> <td>
one of: <a href="undocumented#kBottomLeft_GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>, <a href="undocumented#kTopLeft_GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### See Also

<a href="#SkImage_MakeFromYUVTexturesCopy">MakeFromYUVTexturesCopy</a><sup><a href="#SkImage_MakeFromYUVTexturesCopy_2">[2]</a></sup>

---

<a name="SkImage_MakeFromNV12TexturesCopy_2"></a>

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
<a href="#SkImage_MakeFromNV12TexturesCopy_2_nv12Sizes">nv12Sizes</a> contain dimensions for each pixel plane. Dimensions must be greater than
zero but may differ from plane to plane. Returned <a href="#Image">Image</a> has the dimensions
<a href="#SkImage_MakeFromNV12TexturesCopy_2_nv12Sizes">nv12Sizes</a>[0]. <a href="#SkImage_MakeFromNV12TexturesCopy_2_yuvColorSpace">yuvColorSpace</a> describes how YUV colors convert to RGB colors.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_2_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_2_yuvColorSpace"> <code><strong>yuvColorSpace </strong></code> </a></td> <td>
one of: <a href="SkImageInfo_Reference#kJPEG_SkYUVColorSpace">kJPEG_SkYUVColorSpace</a>, <a href="SkImageInfo_Reference#kRec601_SkYUVColorSpace">kRec601_SkYUVColorSpace</a>,
<a href="SkImageInfo_Reference#kRec709_SkYUVColorSpace">kRec709_SkYUVColorSpace</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_2_nv12TextureHandles"> <code><strong>nv12TextureHandles </strong></code> </a></td> <td>
array of YUV textures on GPU</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_2_nv12Sizes"> <code><strong>nv12Sizes </strong></code> </a></td> <td>
dimensions of YUV textures</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_2_surfaceOrigin"> <code><strong>surfaceOrigin </strong></code> </a></td> <td>
one of: <a href="undocumented#kBottomLeft_GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>, <a href="undocumented#kTopLeft_GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromNV12TexturesCopy_2_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### See Also

<a href="#SkImage_MakeFromYUVTexturesCopy">MakeFromYUVTexturesCopy</a><sup><a href="#SkImage_MakeFromYUVTexturesCopy_2">[2]</a></sup>

---

## <a name="SkImage_BitDepth"></a> Enum SkImage::BitDepth

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum class <a href="#SkImage_BitDepth">BitDepth</a> {
<a href="#SkImage_BitDepth_kU8">kU8</a>,
<a href="#SkImage_BitDepth_kF16">kF16</a>,
};</pre>

### Constants

<table>
  <tr>
    <td><a name="SkImage_BitDepth_kU8"> <code><strong>SkImage::BitDepth::kU8 </strong></code> </a></td><td>0</td><td>Use 8 bits per <a href="undocumented#ARGB">Color ARGB</a> component using unsigned integer format.</td>
  </tr>
  <tr>
    <td><a name="SkImage_BitDepth_kF16"> <code><strong>SkImage::BitDepth::kF16 </strong></code> </a></td><td>1</td><td>Use 16 bits per <a href="undocumented#ARGB">Color ARGB</a> component using half-precision floating point format.</td>
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

Creates <a href="#Image">Image</a> from <a href="#SkImage_MakeFromPicture_picture">picture</a>. Returned <a href="#Image">Image</a> width and height are set by dimensions.
<a href="#Image">Image</a> draws <a href="#SkImage_MakeFromPicture_picture">picture</a> with <a href="#SkImage_MakeFromPicture_matrix">matrix</a> and <a href="#SkImage_MakeFromPicture_paint">paint</a>, set to <a href="#SkImage_MakeFromPicture_bitDepth">bitDepth</a> and <a href="#SkImage_colorSpace">colorSpace</a>.

If <a href="#SkImage_MakeFromPicture_matrix">matrix</a> is nullptr, draws with identity <a href="SkMatrix_Reference#Matrix">Matrix</a>. If <a href="#SkImage_MakeFromPicture_paint">paint</a> is nullptr, draws
with default <a href="SkPaint_Reference#Paint">Paint</a>. <a href="#SkImage_colorSpace">colorSpace</a> may be nullptr.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromPicture_picture"> <code><strong>picture </strong></code> </a></td> <td>
stream of drawing commands</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromPicture_dimensions"> <code><strong>dimensions </strong></code> </a></td> <td>
width and height</td>
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

<a href="SkCanvas_Reference#SkCanvas_drawPicture">SkCanvas::drawPicture</a><sup><a href="SkCanvas_Reference#SkCanvas_drawPicture_2">[2]</a></sup><sup><a href="SkCanvas_Reference#SkCanvas_drawPicture_3">[3]</a></sup><sup><a href="SkCanvas_Reference#SkCanvas_drawPicture_4">[4]</a></sup>

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
one of: <a href="SkImageInfo_Reference#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a>, <a href="SkImageInfo_Reference#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>,
<a href="SkImageInfo_Reference#kPremul_SkAlphaType">kPremul_SkAlphaType</a>, <a href="SkImageInfo_Reference#kUnpremul_SkAlphaType">kUnpremul_SkAlphaType</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAHardwareBuffer_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### See Also

<a href="#SkImage_MakeFromRaster">MakeFromRaster</a>

---

## <a name="Property"></a> Property

| name | description |
| --- | --- |
| <a href="#SkImage_alphaType">alphaType</a> | returns <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a> |
| <a href="#SkImage_bounds">bounds</a> | returns <a href="#SkImage_width">width</a> and <a href="#SkImage_height">height</a> as Rectangle |
| <a href="#SkImage_colorSpace">colorSpace</a> | returns <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkImage_dimensions">dimensions</a> | returns <a href="#SkImage_width">width</a> and <a href="#SkImage_height">height</a> |
| <a href="#SkImage_getTextureHandle">getTextureHandle</a> | returns GPU reference to <a href="#Image">Image</a> as texture |
| <a href="#SkImage_height">height</a> | returns pixel row count |
| <a href="#SkImage_isAlphaOnly">isAlphaOnly</a> | returns if pixels represent a transparency mask |
| <a href="#SkImage_isLazyGenerated">isLazyGenerated</a> | returns if <a href="#Image">Image</a> is created as needed |
| <a href="#SkImage_isOpaque">isOpaque</a> | returns if <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a> is <a href="SkImageInfo_Reference#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a> |
| <a href="#SkImage_isTextureBacked">isTextureBacked</a> | returns if <a href="#Image">Image</a> was created from <a href="undocumented#GPU_Texture">GPU Texture</a> |
| <a href="#SkImage_isValid">isValid</a> | returns if <a href="#Image">Image</a> can draw to <a href="undocumented#Raster_Surface">Raster Surface</a> or <a href="undocumented#GPU_Context">GPU Context</a> |
| <a href="#SkImage_refColorSpace">refColorSpace</a> | returns <a href="SkImageInfo_Reference#Image_Info">Image Info</a> <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkImage_uniqueID">uniqueID</a> | identifier for <a href="#Image">Image</a> |
| <a href="#SkImage_width">width</a> | returns pixel column count |

<a name="SkImage_width"></a>
## width

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int width() const
</pre>

Returns pixel count in each row.

### Return Value

pixel width in <a href="#Image">Image</a>

### Example

<div><fiddle-embed name="39a6d0bbeac6d957c2338e0bff865cf8"></fiddle-embed></div>

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

pixel height in <a href="#Image">Image</a>

### Example

<div><fiddle-embed name="6e563cb8351d34bd8af555a51bcd7a96"></fiddle-embed></div>

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

<div><fiddle-embed name="c204b38b3fc08914b0a634aa4eaec894"></fiddle-embed></div>

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

Returns <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a>, one of: <a href="SkImageInfo_Reference#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a>, <a href="SkImageInfo_Reference#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>,
<a href="SkImageInfo_Reference#kPremul_SkAlphaType">kPremul_SkAlphaType</a>, <a href="SkImageInfo_Reference#kUnpremul_SkAlphaType">kUnpremul_SkAlphaType</a>.

<a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a> returned was a parameter to an <a href="#Image">Image</a> constructor,
or was parsed from encoded data.

### Return Value

<a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a> in <a href="#Image">Image</a>

### Example

<div><fiddle-embed name="dac1403132a42459d6881585efbfe74b"></fiddle-embed></div>

### See Also

<a href="SkImageInfo_Reference#SkImageInfo_alphaType">SkImageInfo::alphaType</a>

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
is packed in 8 bits as defined by <a href="SkImageInfo_Reference#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>.

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

Returns true if pixels ignore their <a href="undocumented#Alpha">Alpha</a> value and are treated as fully opaque.

### Return Value

true if <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a> is <a href="SkImageInfo_Reference#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>

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

Creates <a href="undocumented#Shader">Shader</a> from <a href="#Image">Image</a>. <a href="undocumented#Shader">Shader</a> dimensions are taken from <a href="#Image">Image</a>. <a href="undocumented#Shader">Shader</a> uses
<a href="undocumented#SkShader_TileMode">SkShader::TileMode</a> rules to fill drawn area outside <a href="#Image">Image</a>. <a href="#SkImage_makeShader_localMatrix">localMatrix</a> permits
transforming <a href="#Image">Image</a> before <a href="SkCanvas_Reference#Matrix">Canvas Matrix</a> is applied.

### Parameters

<table>  <tr>    <td><a name="SkImage_makeShader_tileMode1"> <code><strong>tileMode1 </strong></code> </a></td> <td>
tiling in x, one of: <a href="undocumented#SkShader_kClamp_TileMode">SkShader::kClamp TileMode</a>, <a href="undocumented#SkShader_kRepeat_TileMode">SkShader::kRepeat TileMode</a>,
<a href="undocumented#SkShader_kMirror_TileMode">SkShader::kMirror TileMode</a></td>
  </tr>  <tr>    <td><a name="SkImage_makeShader_tileMode2"> <code><strong>tileMode2 </strong></code> </a></td> <td>
tiling in y, one of: <a href="undocumented#SkShader_kClamp_TileMode">SkShader::kClamp TileMode</a>, <a href="undocumented#SkShader_kRepeat_TileMode">SkShader::kRepeat TileMode</a>,
<a href="undocumented#SkShader_kMirror_TileMode">SkShader::kMirror TileMode</a></td>
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

<a name="SkImage_makeShader_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkShader&gt; makeShader(const SkMatrix* localMatrix = nullptr) const
</pre>

Creates <a href="undocumented#Shader">Shader</a> from <a href="#Image">Image</a>. <a href="undocumented#Shader">Shader</a> dimensions are taken from <a href="#Image">Image</a>. <a href="undocumented#Shader">Shader</a> uses
<a href="undocumented#SkShader_kClamp_TileMode">SkShader::kClamp TileMode</a> to fill drawn area outside <a href="#Image">Image</a>. <a href="#SkImage_makeShader_2_localMatrix">localMatrix</a> permits
transforming <a href="#Image">Image</a> before <a href="SkCanvas_Reference#Matrix">Canvas Matrix</a> is applied.

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

<a href="#SkImage_scalePixels">scalePixels</a>

---

## <a name="Pixels"></a> Pixels

| name | description |
| --- | --- |
| <a href="#SkImage_peekPixels">peekPixels</a> | returns <a href="SkPixmap_Reference#Pixmap">Pixmap</a> if possible |
| <a href="#SkImage_readPixels">readPixels</a> | copies and converts pixels |
|  | <a href="#SkImage_readPixels">readPixels(const SkImageInfo& dstInfo, void* dstPixels, size t dstRowBytes, int srcX, int srcY, CachingHint cachingHint = kAllow CachingHint)</a> const |
|  | <a href="#SkImage_readPixels_2">readPixels(const SkPixmap& dst, int srcX, int srcY, CachingHint cachingHint = kAllow CachingHint)</a> const |
| <a href="#SkImage_scalePixels">scalePixels</a> | scales and converts one <a href="#Image">Image</a> to another |

<a name="SkImage_peekPixels"></a>
## peekPixels

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool peekPixels(SkPixmap* pixmap) const
</pre>

Copies <a href="#Image">Image</a> pixel address, row bytes, and <a href="SkImageInfo_Reference#Image_Info">Image Info</a> to <a href="#SkImage_peekPixels_pixmap">pixmap</a>, if address
is available, and returns true. If pixel address is not available, return
false and leave <a href="#SkImage_peekPixels_pixmap">pixmap</a> unchanged.

### Parameters

<table>  <tr>    <td><a name="SkImage_peekPixels_pixmap"> <code><strong>pixmap </strong></code> </a></td> <td>
storage for pixel state if pixels are readable; otherwise, ignored</td>
  </tr>
</table>

### Return Value

true if <a href="#Image">Image</a> has direct access to pixels

### Example

<div><fiddle-embed name="900c0eab8dfdecd8301ed5be95887f8e">

#### Example Output

~~~~
------------
--xx----x---
-x--x--x----
-x--x--x----
-x--x-x-----
--xx-xx-xx--
-----x-x--x-
----x--x--x-
----x--x--x-
---x----xx--
------------
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkImage_readPixels">readPixels</a><sup><a href="#SkImage_readPixels_2">[2]</a></sup>

---

<a name="SkImage_getTexture"></a>
## getTexture

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
GrTexture* getTexture() const
</pre>

---

<a name="SkImage_isTextureBacked"></a>
## isTextureBacked

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isTextureBacked() const
</pre>

Returns true the contents of <a href="#Image">Image</a> was created on or uploaded to GPU memory,
and is available as a <a href="undocumented#GPU_Texture">GPU Texture</a>.

### Return Value

true if <a href="#Image">Image</a> is a <a href="undocumented#GPU_Texture">GPU Texture</a>

### Example

<div><fiddle-embed name="96fd92d399778486a51c5d828ef99322" gpu="true"></fiddle-embed></div>

### See Also

<a href="#SkImage_MakeFromTexture">MakeFromTexture</a> <a href="#SkImage_isValid">isValid</a>

---

<a name="SkImage_isValid"></a>
## isValid

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isValid(GrContext* context) const
</pre>

Returns true if <a href="#Image">Image</a> can be drawn on either <a href="undocumented#Raster_Surface">Raster Surface</a> or <a href="undocumented#GPU_Surface">GPU Surface</a>.
If <a href="#SkImage_isValid_context">context</a> is nullptr, tests if <a href="#Image">Image</a> draws on <a href="undocumented#Raster_Surface">Raster Surface</a>;
otherwise, tests if <a href="#Image">Image</a> draws on <a href="undocumented#GPU_Surface">GPU Surface</a> associated with <a href="#SkImage_isValid_context">context</a>.

<a href="#Image">Image</a> backed by <a href="undocumented#GPU_Texture">GPU Texture</a> may become invalid if associated <a href="undocumented#GrContext">GrContext</a> is
invalid. <a href="#Lazy_Image">Lazy Image</a> may be invalid and may not draw to <a href="undocumented#Raster_Surface">Raster Surface</a> or
<a href="undocumented#GPU_Surface">GPU Surface</a> or both.

### Parameters

<table>  <tr>    <td><a name="SkImage_isValid_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>
</table>

### Return Value

true if <a href="#Image">Image</a> can be drawn

### Example

<div><fiddle-embed name="daf1507ab3a5f7cb0f90058cbc028402" gpu="true"></fiddle-embed></div>

### See Also

<a href="#SkImage_isTextureBacked">isTextureBacked</a> <a href="#SkImage_isLazyGenerated">isLazyGenerated</a>

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
storage for one of: <a href="undocumented#kTopLeft_GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a>,
<a href="undocumented#kBottomLeft_GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>; or nullptr</td>
  </tr>
</table>

### Return Value

back-end API texture handle, or nullptr

### Example

<div><fiddle-embed name="f8943191063bfcc69f29f2b149df5c6d" gpu="true"></fiddle-embed></div>

### Example

<div><fiddle-embed name="a86c580638fcf83f782047b95c60f43f" gpu="true"></fiddle-embed></div>

### See Also

<a href="#SkImage_MakeFromTexture">MakeFromTexture</a> <a href="#SkImage_isTextureBacked">isTextureBacked</a>

---

## <a name="SkImage_CachingHint"></a> Enum SkImage::CachingHint

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum <a href="#SkImage_CachingHint">CachingHint</a> {
<a href="#SkImage_kAllow_CachingHint">kAllow CachingHint</a>,
<a href="#SkImage_kDisallow_CachingHint">kDisallow CachingHint</a>,
};</pre>

<a href="#SkImage_CachingHint">CachingHint</a> selects whether Skia may internally cache <a href="SkBitmap_Reference#Bitmap">Bitmaps</a> generated by
decoding <a href="#Image">Image</a>, or by copying <a href="#Image">Image</a> from GPU to CPU. The default behavior
allows caching <a href="SkBitmap_Reference#Bitmap">Bitmaps</a>.

Choose <a href="#SkImage_kDisallow_CachingHint">kDisallow CachingHint</a> if <a href="#Image">Image</a> pixels are to be used only once, or
if <a href="#Image">Image</a> pixels reside in a cache outside of Skia, or to reduce memory pressure.

Choosing <a href="#SkImage_kAllow_CachingHint">kAllow CachingHint</a> does not ensure that pixels will be cached.
<a href="#Image">Image</a> pixels may not be cached if memory requirements are too large or
pixels are not accessible.

### Constants

<table>
  <tr>
    <td><a name="SkImage_kAllow_CachingHint"> <code><strong>SkImage::kAllow_CachingHint </strong></code> </a></td><td>0</td><td>Allows Skia to internally cache decoded and copied pixels.</td>
  </tr>
  <tr>
    <td><a name="SkImage_kDisallow_CachingHint"> <code><strong>SkImage::kDisallow_CachingHint </strong></code> </a></td><td>1</td><td>Disallows Skia from internally caching decoded and copied pixels.</td>
  </tr>
</table>

### See Also

<a href="#SkImage_readPixels">readPixels</a><sup><a href="#SkImage_readPixels_2">[2]</a></sup> <a href="#SkImage_scalePixels">scalePixels</a>



<a name="SkImage_readPixels"></a>
## readPixels

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes, int srcX, int srcY,
                CachingHint cachingHint = kAllow_CachingHint) const
</pre>

Copies <a href="SkRect_Reference#Rect">Rect</a> of pixels from <a href="#Image">Image</a> to <a href="#SkImage_readPixels_dstPixels">dstPixels</a>. Copy starts at offset (<a href="#SkImage_readPixels_srcX">srcX</a>, <a href="#SkImage_readPixels_srcY">srcY</a>),
and does not exceed <a href="#Image">Image</a> (<a href="#SkImage_width">width</a>, <a href="#SkImage_height">height</a>).

<a href="#SkImage_readPixels_dstInfo">dstInfo</a> specifies width, height, <a href="SkImageInfo_Reference#Color_Type">Color Type</a>, <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a>, and <a href="undocumented#Color_Space">Color Space</a> of
destination. <a href="#SkImage_readPixels_dstRowBytes">dstRowBytes</a> specifics the gap from one destination row to the next.
Returns true if pixels are copied. Returns false if:

<table>  <tr>
    <td><a href="#SkImage_readPixels_dstInfo">dstInfo</a>.addr() equals nullptr</td>  </tr>  <tr>
    <td><a href="#SkImage_readPixels_dstRowBytes">dstRowBytes</a> is less than <a href="#SkImage_readPixels_dstInfo">dstInfo</a>.<a href="SkImageInfo_Reference#SkImageInfo">minRowBytes</a></td>  </tr>  <tr>
    <td><a href="undocumented#Pixel_Ref">Pixel Ref</a> is nullptr</td>  </tr>
</table>

<a href="#Pixels">Pixels</a> are copied only if pixel conversion is possible. If <a href="#Image">Image</a> <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is
<a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, or <a href="SkImageInfo_Reference#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>; <a href="#SkImage_readPixels_dstInfo">dstInfo</a>.<a href="SkImageInfo_Reference#SkImageInfo">colorType</a> must match.
If <a href="#Image">Image</a> <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is <a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, <a href="#SkImage_readPixels_dstInfo">dstInfo</a>.<a href="#SkImage_colorSpace">colorSpace</a> must match.
If <a href="#Image">Image</a> <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a> is <a href="SkImageInfo_Reference#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>, <a href="#SkImage_readPixels_dstInfo">dstInfo</a>.<a href="#SkImage_alphaType">alphaType</a> must
match. If <a href="#Image">Image</a> <a href="undocumented#Color_Space">Color Space</a> is nullptr, <a href="#SkImage_readPixels_dstInfo">dstInfo</a>.<a href="#SkImage_colorSpace">colorSpace</a> must match. Returns
false if pixel conversion is not possible.

<a href="#SkImage_readPixels_srcX">srcX</a> and <a href="#SkImage_readPixels_srcY">srcY</a> may be negative to copy only top or left of source. Returns
false if <a href="#SkImage_width">width</a> or <a href="#SkImage_height">height</a> is zero or negative.
Returns false ifabs(srcX) >= <a href="#Image">Image</a> <a href="#SkImage_width">width</a>,
or ifabs(srcY) >= <a href="#Image">Image</a> <a href="#SkImage_height">height</a>.

If <a href="#SkImage_readPixels_cachingHint">cachingHint</a> is <a href="#SkImage_kAllow_CachingHint">kAllow CachingHint</a>, pixels may be retained locally.
If <a href="#SkImage_readPixels_cachingHint">cachingHint</a> is <a href="#SkImage_kDisallow_CachingHint">kDisallow CachingHint</a>, pixels are not added to the local cache.

### Parameters

<table>  <tr>    <td><a name="SkImage_readPixels_dstInfo"> <code><strong>dstInfo </strong></code> </a></td> <td>
destination width, height, <a href="SkImageInfo_Reference#Color_Type">Color Type</a>, <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a></td>
  </tr>  <tr>    <td><a name="SkImage_readPixels_dstPixels"> <code><strong>dstPixels </strong></code> </a></td> <td>
destination pixel storage</td>
  </tr>  <tr>    <td><a name="SkImage_readPixels_dstRowBytes"> <code><strong>dstRowBytes </strong></code> </a></td> <td>
destination row length</td>
  </tr>  <tr>    <td><a name="SkImage_readPixels_srcX"> <code><strong>srcX </strong></code> </a></td> <td>
column index whose absolute value is less than <a href="#SkImage_width">width</a></td>
  </tr>  <tr>    <td><a name="SkImage_readPixels_srcY"> <code><strong>srcY </strong></code> </a></td> <td>
row index whose absolute value is less than <a href="#SkImage_height">height</a></td>
  </tr>  <tr>    <td><a name="SkImage_readPixels_cachingHint"> <code><strong>cachingHint </strong></code> </a></td> <td>
one of: <a href="#SkImage_kAllow_CachingHint">kAllow CachingHint</a>, <a href="#SkImage_kDisallow_CachingHint">kDisallow CachingHint</a></td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href="#SkImage_readPixels_dstPixels">dstPixels</a>

### Example

<div><fiddle-embed name="8aa8ca63dff4641dfc6ea8a3c555d59c"></fiddle-embed></div>

### See Also

<a href="#SkImage_scalePixels">scalePixels</a> <a href="SkBitmap_Reference#SkBitmap_readPixels">SkBitmap::readPixels</a><sup><a href="SkBitmap_Reference#SkBitmap_readPixels_2">[2]</a></sup><sup><a href="SkBitmap_Reference#SkBitmap_readPixels_3">[3]</a></sup><sup><a href="SkBitmap_Reference#SkBitmap_readPixels_4">[4]</a></sup> <a href="SkPixmap_Reference#SkPixmap_readPixels">SkPixmap::readPixels</a><sup><a href="SkPixmap_Reference#SkPixmap_readPixels_2">[2]</a></sup><sup><a href="SkPixmap_Reference#SkPixmap_readPixels_3">[3]</a></sup><sup><a href="SkPixmap_Reference#SkPixmap_readPixels_4">[4]</a></sup><sup><a href="SkPixmap_Reference#SkPixmap_readPixels_5">[5]</a></sup> <a href="SkCanvas_Reference#SkCanvas_readPixels">SkCanvas::readPixels</a><sup><a href="SkCanvas_Reference#SkCanvas_readPixels_2">[2]</a></sup><sup><a href="SkCanvas_Reference#SkCanvas_readPixels_3">[3]</a></sup> <a href="SkSurface_Reference#SkSurface_readPixels">SkSurface::readPixels</a><sup><a href="SkSurface_Reference#SkSurface_readPixels_2">[2]</a></sup><sup><a href="SkSurface_Reference#SkSurface_readPixels_3">[3]</a></sup>

---

<a name="SkImage_readPixels_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool readPixels(const SkPixmap& dst, int srcX, int srcY, CachingHint cachingHint = kAllow_CachingHint) const
</pre>

Copies a <a href="SkRect_Reference#Rect">Rect</a> of pixels from <a href="#Image">Image</a> to <a href="#SkImage_readPixels_2_dst">dst</a>. Copy starts at (<a href="#SkImage_readPixels_2_srcX">srcX</a>, <a href="#SkImage_readPixels_2_srcY">srcY</a>), and
does not exceed <a href="#Image">Image</a> (<a href="#SkImage_width">width</a>, <a href="#SkImage_height">height</a>).

<a href="#SkImage_readPixels_2_dst">dst</a> specifies width, height, <a href="SkImageInfo_Reference#Color_Type">Color Type</a>, <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a>, pixel storage,
and row bytes of destination. <a href="#SkImage_readPixels_2_dst">dst</a>.<a href="SkPixmap_Reference#SkPixmap">rowBytes</a> specifics the gap from one destination
row to the next. Returns true if pixels are copied. Returns false if:

<table>  <tr>
    <td><a href="#SkImage_readPixels_2_dst">dst</a> pixel storage equals nullptr</td>  </tr>  <tr>
    <td><a href="#SkImage_readPixels_2_dst">dst</a>.<a href="SkPixmap_Reference#SkPixmap">rowBytes</a> is less than <a href="SkImageInfo_Reference#SkImageInfo_minRowBytes">SkImageInfo::minRowBytes</a></td>  </tr>  <tr>
    <td><a href="undocumented#Pixel_Ref">Pixel Ref</a> is nullptr</td>  </tr>
</table>

<a href="#Pixels">Pixels</a> are copied only if pixel conversion is possible. If <a href="#Image">Image</a> <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is
<a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, or <a href="SkImageInfo_Reference#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>; <a href="#SkImage_readPixels_2_dst">dst</a>.<a href="SkPixmap_Reference#SkPixmap">colorType</a> must match.
If <a href="#Image">Image</a> <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is <a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, <a href="#SkImage_readPixels_2_dst">dst</a>.<a href="#SkImage_colorSpace">colorSpace</a> must match.
If <a href="#Image">Image</a> <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a> is <a href="SkImageInfo_Reference#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>, <a href="#SkImage_readPixels_2_dst">dst</a>.<a href="#SkImage_alphaType">alphaType</a> must
match. If <a href="#Image">Image</a> <a href="undocumented#Color_Space">Color Space</a> is nullptr, <a href="#SkImage_readPixels_2_dst">dst</a>.<a href="#SkImage_colorSpace">colorSpace</a> must match. Returns
false if pixel conversion is not possible.
<a href="#SkImage_readPixels_2_srcX">srcX</a> and <a href="#SkImage_readPixels_2_srcY">srcY</a> may be negative to copy only top or left of source. Returns
false if <a href="#SkImage_width">width</a> or <a href="#SkImage_height">height</a> is zero or negative.
Returns false ifabs(srcX) >= <a href="#Image">Image</a> <a href="#SkImage_width">width</a>,
or ifabs(srcY) >= <a href="#Image">Image</a> <a href="#SkImage_height">height</a>.

If <a href="#SkImage_readPixels_2_cachingHint">cachingHint</a> is <a href="#SkImage_kAllow_CachingHint">kAllow CachingHint</a>, pixels may be retained locally.
If <a href="#SkImage_readPixels_2_cachingHint">cachingHint</a> is <a href="#SkImage_kDisallow_CachingHint">kDisallow CachingHint</a>, pixels are not added to the local cache.

### Parameters

<table>  <tr>    <td><a name="SkImage_readPixels_2_dst"> <code><strong>dst </strong></code> </a></td> <td>
destination <a href="SkPixmap_Reference#Pixmap">Pixmap</a>: <a href="SkImageInfo_Reference#Image_Info">Image Info</a>, pixels, row bytes</td>
  </tr>  <tr>    <td><a name="SkImage_readPixels_2_srcX"> <code><strong>srcX </strong></code> </a></td> <td>
column index whose absolute value is less than <a href="#SkImage_width">width</a></td>
  </tr>  <tr>    <td><a name="SkImage_readPixels_2_srcY"> <code><strong>srcY </strong></code> </a></td> <td>
row index whose absolute value is less than <a href="#SkImage_height">height</a></td>
  </tr>  <tr>    <td><a name="SkImage_readPixels_2_cachingHint"> <code><strong>cachingHint </strong></code> </a></td> <td>
one of: <a href="#SkImage_kAllow_CachingHint">kAllow CachingHint</a>, <a href="#SkImage_kDisallow_CachingHint">kDisallow CachingHint</a></td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href="#SkImage_readPixels_2_dst">dst</a>

### Example

<div><fiddle-embed name="b77a73c4baa63a4a8e2a4fdd96144d0b"></fiddle-embed></div>

### See Also

<a href="#SkImage_scalePixels">scalePixels</a> <a href="SkBitmap_Reference#SkBitmap_readPixels">SkBitmap::readPixels</a><sup><a href="SkBitmap_Reference#SkBitmap_readPixels_2">[2]</a></sup><sup><a href="SkBitmap_Reference#SkBitmap_readPixels_3">[3]</a></sup><sup><a href="SkBitmap_Reference#SkBitmap_readPixels_4">[4]</a></sup> <a href="SkPixmap_Reference#SkPixmap_readPixels">SkPixmap::readPixels</a><sup><a href="SkPixmap_Reference#SkPixmap_readPixels_2">[2]</a></sup><sup><a href="SkPixmap_Reference#SkPixmap_readPixels_3">[3]</a></sup><sup><a href="SkPixmap_Reference#SkPixmap_readPixels_4">[4]</a></sup><sup><a href="SkPixmap_Reference#SkPixmap_readPixels_5">[5]</a></sup> <a href="SkCanvas_Reference#SkCanvas_readPixels">SkCanvas::readPixels</a><sup><a href="SkCanvas_Reference#SkCanvas_readPixels_2">[2]</a></sup><sup><a href="SkCanvas_Reference#SkCanvas_readPixels_3">[3]</a></sup> <a href="SkSurface_Reference#SkSurface_readPixels">SkSurface::readPixels</a><sup><a href="SkSurface_Reference#SkSurface_readPixels_2">[2]</a></sup><sup><a href="SkSurface_Reference#SkSurface_readPixels_3">[3]</a></sup>

---

<a name="SkImage_scalePixels"></a>
## scalePixels

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool scalePixels(const SkPixmap& dst, SkFilterQuality filterQuality,
                 CachingHint cachingHint = kAllow_CachingHint) const
</pre>

Copies <a href="#Image">Image</a> to <a href="#SkImage_scalePixels_dst">dst</a>, scaling pixels to fit <a href="#SkImage_scalePixels_dst">dst</a>.<a href="#SkImage_width">width</a> and <a href="#SkImage_scalePixels_dst">dst</a>.<a href="#SkImage_height">height</a>, and
converting pixels to match <a href="#SkImage_scalePixels_dst">dst</a>.<a href="SkPixmap_Reference#SkPixmap">colorType</a> and <a href="#SkImage_scalePixels_dst">dst</a>.<a href="#SkImage_alphaType">alphaType</a>. Returns true if
pixels are copied. Returns false if <a href="#SkImage_scalePixels_dst">dst</a>.addr() is nullptr, or <a href="#SkImage_scalePixels_dst">dst</a>.<a href="SkPixmap_Reference#SkPixmap">rowBytes</a> is
less than <a href="#SkImage_scalePixels_dst">dst</a> <a href="SkImageInfo_Reference#SkImageInfo_minRowBytes">SkImageInfo::minRowBytes</a>.

<a href="#Pixels">Pixels</a> are copied only if pixel conversion is possible. If <a href="#Image">Image</a> <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is
<a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, or <a href="SkImageInfo_Reference#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>; <a href="#SkImage_scalePixels_dst">dst</a>.<a href="SkPixmap_Reference#SkPixmap">colorType</a> must match.
If <a href="#Image">Image</a> <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is <a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, <a href="#SkImage_scalePixels_dst">dst</a>.<a href="#SkImage_colorSpace">colorSpace</a> must match.
If <a href="#Image">Image</a> <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a> is <a href="SkImageInfo_Reference#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>, <a href="#SkImage_scalePixels_dst">dst</a>.<a href="#SkImage_alphaType">alphaType</a> must
match. If <a href="#Image">Image</a> <a href="undocumented#Color_Space">Color Space</a> is nullptr, <a href="#SkImage_scalePixels_dst">dst</a>.<a href="#SkImage_colorSpace">colorSpace</a> must match. Returns
false if pixel conversion is not possible.

Scales the image, with <a href="#SkImage_scalePixels_filterQuality">filterQuality</a>, to match <a href="#SkImage_scalePixels_dst">dst</a>.<a href="#SkImage_width">width</a> and <a href="#SkImage_scalePixels_dst">dst</a>.<a href="#SkImage_height">height</a>.
<a href="#SkImage_scalePixels_filterQuality">filterQuality</a> <a href="undocumented#kNone_SkFilterQuality">kNone_SkFilterQuality</a> is fastest, typically implemented with
<a href="undocumented#Nearest_Neighbor">Filter Quality Nearest Neighbor</a>. <a href="undocumented#kLow_SkFilterQuality">kLow_SkFilterQuality</a> is typically implemented with
<a href="undocumented#Bilerp">Filter Quality Bilerp</a>. <a href="undocumented#kMedium_SkFilterQuality">kMedium_SkFilterQuality</a> is typically implemented with
<a href="undocumented#Bilerp">Filter Quality Bilerp</a>, and <a href="undocumented#MipMap">Filter Quality MipMap</a> when size is reduced.
<a href="undocumented#kHigh_SkFilterQuality">kHigh_SkFilterQuality</a> is slowest, typically implemented with <a href="undocumented#BiCubic">Filter Quality BiCubic</a>.

If <a href="#SkImage_scalePixels_cachingHint">cachingHint</a> is <a href="#SkImage_kAllow_CachingHint">kAllow CachingHint</a>, pixels may be retained locally.
If <a href="#SkImage_scalePixels_cachingHint">cachingHint</a> is <a href="#SkImage_kDisallow_CachingHint">kDisallow CachingHint</a>, pixels are not added to the local cache.

### Parameters

<table>  <tr>    <td><a name="SkImage_scalePixels_dst"> <code><strong>dst </strong></code> </a></td> <td>
destination <a href="SkPixmap_Reference#Pixmap">Pixmap</a>: <a href="SkImageInfo_Reference#Image_Info">Image Info</a>, pixels, row bytes</td>
  </tr>  <tr>    <td><a name="SkImage_scalePixels_filterQuality"> <code><strong>filterQuality </strong></code> </a></td> <td>
one of: <a href="undocumented#kNone_SkFilterQuality">kNone_SkFilterQuality</a>, <a href="undocumented#kLow_SkFilterQuality">kLow_SkFilterQuality</a>,
<a href="undocumented#kMedium_SkFilterQuality">kMedium_SkFilterQuality</a>, <a href="undocumented#kHigh_SkFilterQuality">kHigh_SkFilterQuality</a></td>
  </tr>  <tr>    <td><a name="SkImage_scalePixels_cachingHint"> <code><strong>cachingHint </strong></code> </a></td> <td>
one of: <a href="#SkImage_kAllow_CachingHint">kAllow CachingHint</a>, <a href="#SkImage_kDisallow_CachingHint">kDisallow CachingHint</a></td>
  </tr>
</table>

### Return Value

true if pixels are scaled to fit <a href="#SkImage_scalePixels_dst">dst</a>

### Example

<div><fiddle-embed name="5949c9a63610cae30019e5b1899ee38f"></fiddle-embed></div>

### See Also

<a href="SkCanvas_Reference#SkCanvas_drawImage">SkCanvas::drawImage</a><sup><a href="SkCanvas_Reference#SkCanvas_drawImage_2">[2]</a></sup> <a href="#SkImage_readPixels">readPixels</a><sup><a href="#SkImage_readPixels_2">[2]</a></sup> <a href="SkPixmap_Reference#SkPixmap_scalePixels">SkPixmap::scalePixels</a>

---

<a name="SkImage_encodeToData"></a>
## encodeToData

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkData&gt; encodeToData(SkEncodedImageFormat encodedImageFormat, int quality) const
</pre>

Encodes <a href="#Image">Image</a> pixels, returning result as <a href="undocumented#SkData">SkData</a>.

Returns nullptr if encoding fails, or if <a href="#SkImage_encodeToData_encodedImageFormat">encodedImageFormat</a> is not supported.

<a href="#Image">Image</a> encoding in a format requires both building with one or more of:
SK_HAS_JPEG_LIBRARY, SK_HAS_PNG_LIBRARY, SK_HAS_WEBP_LIBRARY; and platform support
for the encoded format.

If SK_BUILD_FOR_MAC or SK_BUILD_FOR_IOS is defined, <a href="#SkImage_encodeToData_encodedImageFormat">encodedImageFormat</a> can
additionally be one of: <a href="undocumented#SkEncodedImageFormat_kICO">SkEncodedImageFormat::kICO</a>, <a href="undocumented#SkEncodedImageFormat_kBMP">SkEncodedImageFormat::kBMP</a>,
<a href="undocumented#SkEncodedImageFormat_kGIF">SkEncodedImageFormat::kGIF</a>.

<a href="#SkImage_encodeToData_quality">quality</a> is a platform and format specific metric trading off size and encoding
error. When used, <a href="#SkImage_encodeToData_quality">quality</a> equaling 100 encodes with the least error. <a href="#SkImage_encodeToData_quality">quality</a> may
be ignored by the encoder.

### Parameters

<table>  <tr>    <td><a name="SkImage_encodeToData_encodedImageFormat"> <code><strong>encodedImageFormat </strong></code> </a></td> <td>
one of: <a href="undocumented#SkEncodedImageFormat_kJPEG">SkEncodedImageFormat::kJPEG</a>, <a href="undocumented#SkEncodedImageFormat_kPNG">SkEncodedImageFormat::kPNG</a>,
<a href="undocumented#SkEncodedImageFormat_kWEBP">SkEncodedImageFormat::kWEBP</a></td>
  </tr>  <tr>    <td><a name="SkImage_encodeToData_quality"> <code><strong>quality </strong></code> </a></td> <td>
encoder specific metric with 100 equaling best</td>
  </tr>
</table>

### Return Value

encoded <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="7a3bf8851bb7160e4e49c48f8c09639d"></fiddle-embed></div>

### See Also

<a href="#SkImage_refEncodedData">refEncodedData</a> <a href="#SkImage_MakeFromEncoded">MakeFromEncoded</a>

---

<a name="SkImage_encodeToData_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkData&gt; encodeToData() const
</pre>

Encodes <a href="#Image">Image</a> pixels, returning result as <a href="undocumented#SkData">SkData</a>. Returns existing encoded data
if present; otherwise, <a href="#Image">Image</a> is encoded with <a href="undocumented#SkEncodedImageFormat_kPNG">SkEncodedImageFormat::kPNG</a>. Skia
must be built with SK_HAS_PNG_LIBRARY to encode <a href="#Image">Image</a>.

Returns nullptr if existing encoded data is missing or invalid, and
encoding fails.

### Return Value

encoded <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="30cee813f6aa476b0a9c8a24283e53a3"></fiddle-embed></div>

### See Also

<a href="#SkImage_refEncodedData">refEncodedData</a> <a href="#SkImage_MakeFromEncoded">MakeFromEncoded</a>

---

<a name="SkImage_refEncodedData"></a>
## refEncodedData

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkData&gt; refEncodedData() const
</pre>

Returns encoded <a href="#Image">Image</a> pixels as <a href="undocumented#SkData">SkData</a>, if <a href="#Image">Image</a> was created from supported
encoded stream format. Platform support for formats vary and may require building
with one or more of: SK_HAS_JPEG_LIBRARY, SK_HAS_PNG_LIBRARY, SK_HAS_WEBP_LIBRARY.

Returns nullptr if <a href="#Image">Image</a> contents are not encoded.

### Return Value

encoded <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="91866923b37edd673c18232fdf3eabd8" gpu="true"></fiddle-embed></div>

### See Also

<a href="#SkImage_encodeToData">encodeToData</a><sup><a href="#SkImage_encodeToData_2">[2]</a></sup> <a href="#SkImage_MakeFromEncoded">MakeFromEncoded</a>

---

## <a name="Utility"></a> Utility

| name | description |
| --- | --- |
| <a href="#SkImage_encodeToData">encodeToData</a> | returns encoded <a href="#Image">Image</a> as <a href="undocumented#SkData">SkData</a> |
|  | <a href="#SkImage_encodeToData">encodeToData(SkEncodedImageFormat encodedImageFormat, int quality)</a> const |
|  | <a href="#SkImage_encodeToData_2">encodeToData</a> const |
| <a href="#SkImage_refEncodedData">refEncodedData</a> | returns <a href="#Image">Image</a> encoded in <a href="undocumented#SkData">SkData</a> if present |
| <a href="#SkImage_toString">toString</a> | converts <a href="#Image">Image</a> to machine readable form |

<a name="SkImage_toString"></a>
## toString

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const char* toString(SkString* string) const
</pre>

Appends <a href="#Image">Image</a> description to <a href="#SkImage_toString_string">string</a>, including unique ID, width, height, and
whether the image is opaque.

### Parameters

<table>  <tr>    <td><a name="SkImage_toString_string"> <code><strong>string </strong></code> </a></td> <td>
storage for description; existing content is preserved</td>
  </tr>
</table>

### Return Value

<a href="#SkImage_toString_string">string</a> appended with <a href="#Image">Image</a> description

### Example

<div><fiddle-embed name="e5eae6d362434154730e3bacff165ebd"></fiddle-embed></div>

### See Also

<a href="SkPaint_Reference#SkPaint_toString">SkPaint::toString</a>

---

<a name="SkImage_makeSubset"></a>
## makeSubset

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkImage&gt; makeSubset(const SkIRect& subset) const
</pre>

Returns <a href="#SkImage_makeSubset_subset">subset</a> of <a href="#Image">Image</a>. <a href="#SkImage_makeSubset_subset">subset</a> must be fully contained by <a href="#Image">Image</a> <a href="#SkImage_dimensions">dimensions</a>.
The implementation may share pixels, or may copy them.

Returns nullptr if <a href="#SkImage_makeSubset_subset">subset</a> is empty, or <a href="#SkImage_makeSubset_subset">subset</a> is not contained by bounds, or
pixels in <a href="#Image">Image</a> could not be read or copied.

### Parameters

<table>  <tr>    <td><a name="SkImage_makeSubset_subset"> <code><strong>subset </strong></code> </a></td> <td>
bounds of returned <a href="#Image">Image</a></td>
  </tr>
</table>

### Return Value

partial or full <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="93669037c9eb9d142e7776b9f936fa96"></fiddle-embed></div>

### See Also

<a href="#SkImage_MakeFromEncoded">MakeFromEncoded</a>

---

<a name="SkImage_makeTextureImage"></a>
## makeTextureImage

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkImage&gt; makeTextureImage(GrContext* context, SkColorSpace* dstColorSpace) const
</pre>

Returns <a href="#Image">Image</a> backed by <a href="undocumented#GPU_Texture">GPU Texture</a> associated with <a href="#SkImage_makeTextureImage_context">context</a>. Returned <a href="#Image">Image</a> is
compatible with <a href="SkSurface_Reference#Surface">Surface</a> created with <a href="#SkImage_makeTextureImage_dstColorSpace">dstColorSpace</a>. Returns original
<a href="#Image">Image</a> if <a href="#SkImage_makeTextureImage_context">context</a> and <a href="#SkImage_makeTextureImage_dstColorSpace">dstColorSpace</a> match.

Returns nullptr if <a href="#SkImage_makeTextureImage_context">context</a> is nullptr, or if <a href="#Image">Image</a> was created with another
<a href="undocumented#GrContext">GrContext</a>.

### Parameters

<table>  <tr>    <td><a name="SkImage_makeTextureImage_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_makeTextureImage_dstColorSpace"> <code><strong>dstColorSpace </strong></code> </a></td> <td>
range of colors of matching <a href="SkSurface_Reference#Surface">Surface</a> on GPU</td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="7d060e137662b233960200b7b2597ba6" gpu="true"></fiddle-embed></div>

### See Also

<a href="#SkImage_MakeFromTexture">MakeFromTexture</a>

---

<a name="SkImage_makeNonTextureImage"></a>
## makeNonTextureImage

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkImage&gt; makeNonTextureImage() const
</pre>

Returns <a href="#Raster_Image">Raster Image</a> or <a href="#Lazy_Image">Lazy Image</a>. Copies <a href="#Image">Image</a> backed by <a href="undocumented#GPU_Texture">GPU Texture</a> into
CPU memory if needed. Returns original <a href="#Image">Image</a> if decoded in <a href="undocumented#Raster_Bitmap">Raster Bitmap</a>,
or if encoded in a stream.

Returns nullptr if backed by <a href="undocumented#GPU_Texture">GPU Texture</a> and copy fails.

### Return Value

<a href="#Raster_Image">Raster Image</a>, <a href="#Lazy_Image">Lazy Image</a>, or nullptr

### Example

<div><fiddle-embed name="220d369551a553f8ba4cd1c21b97a793" gpu="true"></fiddle-embed></div>

### See Also

<a href="#SkImage_makeTextureImage">makeTextureImage</a> <a href="#SkImage_makeRasterImage">makeRasterImage</a> <a href="#SkImage_MakeBackendTextureFromSkImage">MakeBackendTextureFromSkImage</a>

---

<a name="SkImage_makeRasterImage"></a>
## makeRasterImage

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkImage&gt; makeRasterImage() const
</pre>

Returns <a href="#Raster_Image">Raster Image</a>. Copies <a href="#Image">Image</a> backed by <a href="undocumented#GPU_Texture">GPU Texture</a> into CPU memory,
or decodes <a href="#Image">Image</a> from <a href="#Lazy_Image">Lazy Image</a>. Returns original <a href="#Image">Image</a> if decoded in
<a href="undocumented#Raster_Bitmap">Raster Bitmap</a>.

Returns nullptr if copy, decode, or pixel read fails.

### Return Value

<a href="#Raster_Image">Raster Image</a>, or nullptr

### Example

<div><fiddle-embed name="d821b8e345df9ff0c8cbb6d91c588c02" gpu="true"></fiddle-embed></div>

### See Also

<a href="#SkImage_isTextureBacked">isTextureBacked</a> <a href="#SkImage_isLazyGenerated">isLazyGenerated</a> <a href="#SkImage_MakeFromRaster">MakeFromRaster</a>

---

<a name="SkImage_makeWithFilter"></a>
## makeWithFilter

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkImage&gt; makeWithFilter(const SkImageFilter* filter, const SkIRect& subset,
                              const SkIRect& clipBounds, SkIRect* outSubset, SkIPoint* offset) const
</pre>

Creates filtered <a href="#Image">Image</a>. <a href="#SkImage_makeWithFilter_filter">filter</a> processes original <a href="#Image">Image</a>, potentially changing
color, position, and size. <a href="#SkImage_makeWithFilter_subset">subset</a> is the bounds of original <a href="#Image">Image</a> processed
by <a href="#SkImage_makeWithFilter_filter">filter</a>. <a href="#SkImage_makeWithFilter_clipBounds">clipBounds</a> is the expected bounds of the filtered <a href="#Image">Image</a>. <a href="#SkImage_makeWithFilter_outSubset">outSubset</a>
is required storage for the actual bounds of the filtered <a href="#Image">Image</a>. <a href="#SkImage_makeWithFilter_offset">offset</a> is
required storage for translation of returned <a href="#Image">Image</a>.

Returns nullptr if <a href="#Image">Image</a> could not be created. If nullptr is returned, <a href="#SkImage_makeWithFilter_outSubset">outSubset</a>
and <a href="#SkImage_makeWithFilter_offset">offset</a> are undefined.

Useful for animation of <a href="undocumented#SkImageFilter">SkImageFilter</a> that varies size from frame to frame.
Returned <a href="#Image">Image</a> is created larger than required by <a href="#SkImage_makeWithFilter_filter">filter</a> so that <a href="undocumented#GPU_Texture">GPU Texture</a>
can be reused with different sized effects. <a href="#SkImage_makeWithFilter_outSubset">outSubset</a> describes the valid bounds
of <a href="undocumented#GPU_Texture">GPU Texture</a> returned. <a href="#SkImage_makeWithFilter_offset">offset</a> translates the returned <a href="#Image">Image</a> to keep subsequent
animation frames aligned with respect to each other.

### Parameters

<table>  <tr>    <td><a name="SkImage_makeWithFilter_filter"> <code><strong>filter </strong></code> </a></td> <td>
how <a href="#Image">Image</a> is sampled when transformed</td>
  </tr>  <tr>    <td><a name="SkImage_makeWithFilter_subset"> <code><strong>subset </strong></code> </a></td> <td>
bounds of <a href="#Image">Image</a> processed by <a href="#SkImage_makeWithFilter_filter">filter</a></td>
  </tr>  <tr>    <td><a name="SkImage_makeWithFilter_clipBounds"> <code><strong>clipBounds </strong></code> </a></td> <td>
expected bounds of filtered <a href="#Image">Image</a></td>
  </tr>  <tr>    <td><a name="SkImage_makeWithFilter_outSubset"> <code><strong>outSubset </strong></code> </a></td> <td>
storage for returned <a href="#Image">Image</a> bounds</td>
  </tr>  <tr>    <td><a name="SkImage_makeWithFilter_offset"> <code><strong>offset </strong></code> </a></td> <td>
storage for returned <a href="#Image">Image</a> translation</td>
  </tr>
</table>

### Return Value

filtered <a href="#Image">Image</a>, or nullptr

### Example

<div><fiddle-embed name="eabb12543886ace5e1212af220a19c6d" gpu="true"><div>In each frame of the animation, filtered <a href="#Image">Image</a> is drawn in a different location.
By translating canvas by returned <a href="#SkImage_makeWithFilter_offset">offset</a>, <a href="#Image">Image</a> appears stationary.</div></fiddle-embed></div>

### See Also

<a href="#SkImage_makeShader">makeShader</a><sup><a href="#SkImage_makeShader_2">[2]</a></sup> <a href="SkPaint_Reference#SkPaint_setImageFilter">SkPaint::setImageFilter</a>

---

<a href="#SkImage_BackendTextureReleaseProc">BackendTextureReleaseProc</a>

<a name="SkImage_MakeBackendTextureFromSkImage"></a>
## MakeBackendTextureFromSkImage

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static bool MakeBackendTextureFromSkImage(GrContext* context, sk_sp&lt;SkImage&gt; image,
                                          GrBackendTexture* backendTexture,
                                          BackendTextureReleaseProc* backendTextureReleaseProc)
</pre>

Creates a <a href="undocumented#GrBackendTexture">GrBackendTexture</a> from the provided <a href="#SkImage">SkImage</a>. Returns true and
stores result in <a href="#SkImage_MakeBackendTextureFromSkImage_backendTexture">backendTexture</a> and <a href="#SkImage_MakeBackendTextureFromSkImage_backendTextureReleaseProc">backendTextureReleaseProc</a> if
texture is created; otherwise, returns false and leaves
<a href="#SkImage_MakeBackendTextureFromSkImage_backendTexture">backendTexture</a> and <a href="#SkImage_MakeBackendTextureFromSkImage_backendTextureReleaseProc">backendTextureReleaseProc</a> unmodified.

Call <a href="#SkImage_MakeBackendTextureFromSkImage_backendTextureReleaseProc">backendTextureReleaseProc</a> after deleting <a href="#SkImage_MakeBackendTextureFromSkImage_backendTexture">backendTexture</a>.
<a href="#SkImage_MakeBackendTextureFromSkImage_backendTextureReleaseProc">backendTextureReleaseProc</a> cleans up auxiliary data related to returned
<a href="#SkImage_MakeBackendTextureFromSkImage_backendTexture">backendTexture</a>. The caller must delete returned <a href="#SkImage_MakeBackendTextureFromSkImage_backendTexture">backendTexture</a> after use.

If <a href="#Image">Image</a> is both texture backed and singly referenced, <a href="#SkImage_MakeBackendTextureFromSkImage_image">image</a> is returned in
<a href="#SkImage_MakeBackendTextureFromSkImage_backendTexture">backendTexture</a> without conversion or making a copy. <a href="#Image">Image</a> is singly referenced
if its was transferred solely using std::move().

If <a href="#Image">Image</a> is not texture backed, returns texture with <a href="#Image">Image</a> contents.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeBackendTextureFromSkImage_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkImage_MakeBackendTextureFromSkImage_image"> <code><strong>image </strong></code> </a></td> <td>
<a href="#Image">Image</a> used for texture</td>
  </tr>  <tr>    <td><a name="SkImage_MakeBackendTextureFromSkImage_backendTexture"> <code><strong>backendTexture </strong></code> </a></td> <td>
storage for backend texture</td>
  </tr>  <tr>    <td><a name="SkImage_MakeBackendTextureFromSkImage_backendTextureReleaseProc"> <code><strong>backendTextureReleaseProc </strong></code> </a></td> <td>
storage for clean up function</td>
  </tr>
</table>

### Return Value

true if backend texture was created

### Example

<div><fiddle-embed name="98e70337c2964abd2624239d28bbecd7" gpu="true"></fiddle-embed></div>

### See Also

<a href="#SkImage_MakeFromTexture">MakeFromTexture</a> <a href="#SkImage_makeTextureImage">makeTextureImage</a>

---

## <a name="SkImage_LegacyBitmapMode"></a> Enum SkImage::LegacyBitmapMode

soon

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum <a href="#SkImage_LegacyBitmapMode">LegacyBitmapMode</a> {
<a href="#SkImage_kRO_LegacyBitmapMode">kRO LegacyBitmapMode</a>,
};</pre>

### Constants

<table>
  <tr>
    <td><a name="SkImage_kRO_LegacyBitmapMode"> <code><strong>SkImage::kRO_LegacyBitmapMode </strong></code> </a></td><td>0</td><td>Returned bitmap is read-only and immutable.</td>
  </tr>

</table>

<a name="SkImage_asLegacyBitmap"></a>
## asLegacyBitmap

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool asLegacyBitmap(SkBitmap* bitmap, LegacyBitmapMode legacyBitmapMode = kRO_LegacyBitmapMode) const
</pre>

Creates raster <a href="SkBitmap_Reference#Bitmap">Bitmap</a> with same pixels as <a href="#Image">Image</a>. If <a href="#SkImage_asLegacyBitmap_legacyBitmapMode">legacyBitmapMode</a> is
<a href="#SkImage_kRO_LegacyBitmapMode">kRO LegacyBitmapMode</a>, returned <a href="#SkImage_asLegacyBitmap_bitmap">bitmap</a> is read-only and immutable.
Returns true if <a href="SkBitmap_Reference#Bitmap">Bitmap</a> is stored in <a href="#SkImage_asLegacyBitmap_bitmap">bitmap</a>. Returns false and resets <a href="#SkImage_asLegacyBitmap_bitmap">bitmap</a> if
<a href="SkBitmap_Reference#Bitmap">Bitmap</a> write did not succeed.

### Parameters

<table>  <tr>    <td><a name="SkImage_asLegacyBitmap_bitmap"> <code><strong>bitmap </strong></code> </a></td> <td>
storage for legacy <a href="SkBitmap_Reference#Bitmap">Bitmap</a></td>
  </tr>  <tr>    <td><a name="SkImage_asLegacyBitmap_legacyBitmapMode"> <code><strong>legacyBitmapMode </strong></code> </a></td> <td>
to be deprecated</td>
  </tr>
</table>

### Return Value

true if <a href="SkBitmap_Reference#Bitmap">Bitmap</a> was created

### Example

<div><fiddle-embed name="eddfe9735342052ce2f8869ee5eb737a" gpu="true"></fiddle-embed></div>

### See Also

<a href="#SkImage_MakeRasterData">MakeRasterData</a> <a href="#SkImage_makeRasterImage">makeRasterImage</a> <a href="#SkImage_makeNonTextureImage">makeNonTextureImage</a>

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

### Example

<div><fiddle-embed name="070dd0405890b84c07827d93fa01c331" gpu="true"></fiddle-embed></div>

### See Also

<a href="#SkImage_isTextureBacked">isTextureBacked</a> MakeNonTextureImage

---

<a name="SkImage_makeColorSpace"></a>
## makeColorSpace

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkImage&gt; makeColorSpace(sk_sp&lt;SkColorSpace&gt; target, SkTransferFunctionBehavior premulBehavior) const
</pre>

Creates <a href="#Image">Image</a> in <a href="#SkImage_makeColorSpace_target">target</a> <a href="undocumented#Color_Space">Color Space</a>.
Returns nullptr if <a href="#Image">Image</a> could not be created.

Returns original <a href="#Image">Image</a> if it is in <a href="#SkImage_makeColorSpace_target">target</a> <a href="undocumented#Color_Space">Color Space</a>.
Otherwise, converts pixels from <a href="#Image">Image</a> <a href="undocumented#Color_Space">Color Space</a> to <a href="#SkImage_makeColorSpace_target">target</a> <a href="undocumented#Color_Space">Color Space</a>.
If <a href="#Image">Image</a> <a href="#SkImage_colorSpace">colorSpace</a> returns nullptr, <a href="#Image">Image</a> <a href="undocumented#Color_Space">Color Space</a> is assumed to be sRGB.

<a href="undocumented#SkTransferFunctionBehavior">SkTransferFunctionBehavior</a> is to be deprecated.

Set <a href="#SkImage_makeColorSpace_premulBehavior">premulBehavior</a> to <a href="undocumented#SkTransferFunctionBehavior_kRespect">SkTransferFunctionBehavior::kRespect</a> to convert <a href="#Image">Image</a>
pixels to a linear space, before converting to destination <a href="SkImageInfo_Reference#Color_Type">Color Type</a>
and <a href="undocumented#Color_Space">Color Space</a>.

Set <a href="#SkImage_makeColorSpace_premulBehavior">premulBehavior</a> to <a href="undocumented#SkTransferFunctionBehavior_kIgnore">SkTransferFunctionBehavior::kIgnore</a> to treat <a href="#Image">Image</a>
pixels as linear, when converting to destination <a href="SkImageInfo_Reference#Color_Type">Color Type</a>
and <a href="undocumented#Color_Space">Color Space</a>, ignoring pixel encoding.

### Parameters

<table>  <tr>    <td><a name="SkImage_makeColorSpace_target"> <code><strong>target </strong></code> </a></td> <td>
<a href="undocumented#Color_Space">Color Space</a> describing color range of returned <a href="#Image">Image</a></td>
  </tr>  <tr>    <td><a name="SkImage_makeColorSpace_premulBehavior"> <code><strong>premulBehavior </strong></code> </a></td> <td>
one of: <a href="undocumented#SkTransferFunctionBehavior_kRespect">SkTransferFunctionBehavior::kRespect</a>,
<a href="undocumented#SkTransferFunctionBehavior_kIgnore">SkTransferFunctionBehavior::kIgnore</a></td>
  </tr>
</table>

### Return Value

created <a href="#Image">Image</a> in <a href="#SkImage_makeColorSpace_target">target</a> <a href="undocumented#Color_Space">Color Space</a>

### Example

<div><fiddle-embed name="ab8bcb9acecbee444019a724d2b0503c"></fiddle-embed></div>

### See Also

MakeFromPixture <a href="#SkImage_MakeFromTexture">MakeFromTexture</a>

---

