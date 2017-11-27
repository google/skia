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

## <a name="Structs"></a> Structs

| description | struct |
| --- | ---  |
| <a href="#SkImage_DeferredTextureImageUsageParams">DeferredTextureImageUsageParams</a> |  |

## <a name="Member_Functions"></a> Member Functions

| description | function |
| --- | ---  |
| <a href="#SkImage_MakeFromAHardwareBuffer">MakeFromAHardwareBuffer</a> |  |
| <a href="#SkImage_MakeBackendTextureFromSkImage">MakeBackendTextureFromSkImage</a> |  |
| <a href="#SkImage_MakeCrossContextFromEncoded">MakeCrossContextFromEncoded</a> |  |
| <a href="#SkImage_MakeFromAHardwareBuffer">MakeFromAHardwareBuffer</a> |  |
| <a href="#SkImage_MakeFromAdoptedTexture">MakeFromAdoptedTexture</a> |  |
| <a href="#SkImage_MakeFromBitmap">MakeFromBitmap</a> |  |
| <a href="#SkImage_MakeFromDeferredTextureImageData">MakeFromDeferredTextureImageData</a> |  |
| <a href="#SkImage_MakeFromEncoded">MakeFromEncoded</a> |  |
| <a href="#SkImage_MakeFromGenerator">MakeFromGenerator</a> |  |
| <a href="#SkImage_MakeFromNV12TexturesCopy">MakeFromNV12TexturesCopy</a> |  |
| <a href="#SkImage_MakeFromPicture">MakeFromPicture</a> |  |
| <a href="#SkImage_MakeFromRaster">MakeFromRaster</a> |  |
| <a href="#SkImage_MakeFromTexture">MakeFromTexture</a> |  |
| <a href="#SkImage_MakeFromYUVTexturesCopy">MakeFromYUVTexturesCopy</a> |  |
| <a href="#SkImage_MakeRasterCopy">MakeRasterCopy</a> |  |
| <a href="#SkImage_MakeRasterData">MakeRasterData</a> |  |
| <a href="#SkImage_alphaType">alphaType</a> |  |
| <a href="#SkImage_asLegacyBitmap">asLegacyBitmap</a> |  |
| <a href="#SkImage_bounds">bounds</a> |  |
| <a href="#SkImage_colorSpace">colorSpace</a> |  |
| <a href="#SkImage_dimensions">dimensions</a> |  |
| <a href="#SkImage_encodeToData">encodeToData</a> |  |
| <a href="#SkImage_getDeferredTextureImageData">getDeferredTextureImageData</a> |  |
| <a href="#SkImage_getTexture">getTexture</a> |  |
| <a href="#SkImage_getTextureHandle">getTextureHandle</a> |  |
| <a href="#SkImage_height">height</a> |  |
| <a href="#SkImage_isAlphaOnly">isAlphaOnly</a> |  |
| <a href="#SkImage_isLazyGenerated">isLazyGenerated</a> |  |
| <a href="#SkImage_isOpaque">isOpaque</a> |  |
| <a href="#SkImage_isTextureBacked">isTextureBacked</a> |  |
| <a href="#SkImage_isValid">isValid</a> |  |
| <a href="#SkImage_makeColorSpace">makeColorSpace</a> |  |
| <a href="#SkImage_makeNonTextureImage">makeNonTextureImage</a> |  |
| <a href="#SkImage_makeShader">makeShader</a> |  |
| <a href="#SkImage_makeSubset">makeSubset</a> |  |
| <a href="#SkImage_makeTextureImage">makeTextureImage</a> |  |
| <a href="#SkImage_makeWithFilter">makeWithFilter</a> |  |
| <a href="#SkImage_peekPixels">peekPixels</a> |  |
| <a href="#SkImage_readPixels">readPixels</a> |  |
| <a href="#SkImage_refColorSpace">refColorSpace</a> |  |
| <a href="#SkImage_refEncodedData">refEncodedData</a> |  |
| <a href="#SkImage_scalePixels">scalePixels</a> |  |
| <a href="#SkImage_toString">toString</a> |  |
| <a href="#SkImage_uniqueID">uniqueID</a> |  |
| <a href="#SkImage_width">width</a> |  |

# <a name="SkImage_DeferredTextureImageUsageParams"></a> Struct SkImage::DeferredTextureImageUsageParams

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
</pre>

Drawing parameters for which a deferred texture image data should be optimized. */

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

<a name="SkImage_DeferredTextureImageUsageParams_fMatrix"> <code><strong>SkMatrix  fMatrix</strong></code> </a>

<a name="SkImage_DeferredTextureImageUsageParams_fQuality"> <code><strong>SkFilterQuality  fQuality</strong></code> </a>

<a name="SkImage_DeferredTextureImageUsageParams_fPreScaleMipLevel"> <code><strong>int  fPreScaleMipLevel</strong></code> </a>

<a name="SkImage_MakeRasterCopy"></a>
## MakeRasterCopy

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeRasterCopy(const SkPixmap& pixmap)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeRasterCopy_pixmap"> <code><strong>pixmap </strong></code> </a></td> <td>
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

<a name="SkImage_MakeRasterData"></a>
## MakeRasterData

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeRasterData(const Info& info, sk_sp&lt;SkData&gt; pixels, size_t rowBytes)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeRasterData_info"> <code><strong>info </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeRasterData_pixels"> <code><strong>pixels </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeRasterData_rowBytes"> <code><strong>rowBytes </strong></code> </a></td> <td>
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

<a name="SkImage_MakeFromRaster"></a>
## MakeFromRaster

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromRaster(const SkPixmap& pixmap, RasterReleaseProc rasterReleaseProc,
                                     ReleaseContext releaseContext)
</pre>

Return a new <a href="#Image">Image</a> referencing the specified pixels. These must remain valid and unchanged
until the specified release-proc is called, indicating that <a href="undocumented#Skia">Skia</a> no longer has a reference
to the pixels.
Returns nullptr if the requested <a href="#SkImage_MakeFromRaster_pixmap">pixmap</a> info is unsupported.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromRaster_pixmap"> <code><strong>pixmap </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromRaster_rasterReleaseProc"> <code><strong>rasterReleaseProc </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromRaster_releaseContext"> <code><strong>releaseContext </strong></code> </a></td> <td>
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

<a name="SkImage_MakeFromBitmap"></a>
## MakeFromBitmap

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromBitmap(const SkBitmap& bitmap)
</pre>

Construct a new image from the specified <a href="#SkImage_MakeFromBitmap_bitmap">bitmap</a>. If the <a href="#SkImage_MakeFromBitmap_bitmap">bitmap</a> is marked immutable, and
its pixel memory is shareable, it may be shared instead of copied.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromBitmap_bitmap"> <code><strong>bitmap </strong></code> </a></td> <td>
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

<a name="SkImage_MakeFromGenerator"></a>
## MakeFromGenerator

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromGenerator(std::unique_ptr&lt;SkImageGenerator&gt; imageGenerator,
                                 const SkIRect* subset = nullptr)
</pre>

Construct a new <a href="#SkImage">SkImage</a> based on the given ImageGenerator. Returns nullptr on error.
This function will always take ownership of the passed generator.
If a <a href="#SkImage_MakeFromGenerator_subset">subset</a> is specified, it must be contained within the generator's <a href="#SkImage_bounds">bounds</a>.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromGenerator_imageGenerator"> <code><strong>imageGenerator </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromGenerator_subset"> <code><strong>subset </strong></code> </a></td> <td>
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

<a name="SkImage_MakeFromEncoded"></a>
## MakeFromEncoded

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromEncoded(sk_sp&lt;SkData&gt; encoded, const SkIRect* subset = nullptr)
</pre>

Construct a new <a href="#SkImage">SkImage</a> based on the specified <a href="#SkImage_MakeFromEncoded_encoded">encoded</a> data. Returns nullptr on failure,
which can mean that the format of the <a href="#SkImage_MakeFromEncoded_encoded">encoded</a> data was not recognized/supported.
If a <a href="#SkImage_MakeFromEncoded_subset">subset</a> is specified, it must be contained within the <a href="#SkImage_MakeFromEncoded_encoded">encoded</a> data's <a href="#SkImage_bounds">bounds</a>.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromEncoded_encoded"> <code><strong>encoded </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromEncoded_subset"> <code><strong>subset </strong></code> </a></td> <td>
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

<a name="SkImage_MakeFromTexture"></a>
## MakeFromTexture

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromTexture(GrContext* context, const GrBackendTexture& backendTexture,
                                      GrSurfaceOrigin origin, SkAlphaType alphaType,
                                      sk_sp&lt;SkColorSpace&gt; colorSpace)
</pre>

Create a new image from the specified descriptor. Note - the caller is responsible for
managing the lifetime of the underlying platform texture.
Will return nullptr if the specified back-end texture is unsupported.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromTexture_context"> <code><strong>context </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_backendTexture"> <code><strong>backendTexture </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_origin"> <code><strong>origin </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_alphaType"> <code><strong>alphaType </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
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
static sk_sp&lt;SkImage&gt; MakeFromTexture(GrContext* context, const GrBackendTexture& backendTexture,
                                      GrSurfaceOrigin origin, SkAlphaType alphaType,
                                      sk_sp&lt;SkColorSpace&gt; colorSpace,
                                      TextureReleaseProc textureReleaseProc,
                                      ReleaseContext releaseContext)
</pre>

Create a new image from the <a href="undocumented#GrBackendTexture">GrBackendTexture</a>. The underlying platform texture must stay
valid and unaltered until the specified release-proc is invoked, indicating that <a href="undocumented#Skia">Skia</a>
no longer is holding a reference to it.
Will return nullptr if the specified back-end texture is unsupported.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromTexture_2_context"> <code><strong>context </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_2_backendTexture"> <code><strong>backendTexture </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_2_origin"> <code><strong>origin </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_2_alphaType"> <code><strong>alphaType </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_2_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_2_textureReleaseProc"> <code><strong>textureReleaseProc </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromTexture_2_releaseContext"> <code><strong>releaseContext </strong></code> </a></td> <td>
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

<a name="SkImage_MakeCrossContextFromEncoded"></a>
## MakeCrossContextFromEncoded

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeCrossContextFromEncoded(GrContext* context, sk_sp&lt;SkData&gt; data,
                                                  bool buildMips, SkColorSpace* dstColorSpace)
</pre>

Decodes and uploads the encoded <a href="#SkImage_MakeCrossContextFromEncoded_data">data</a> to a <a href="undocumented#GPU">GPU</a> backed image using the supplied <a href="undocumented#GrContext">GrContext</a>.
That image can be safely used by other GrContexts, across thread boundaries. The <a href="undocumented#GrContext">GrContext</a>
used here, and the ones used to draw this image later must be in the same <a href="undocumented#OpenGL">OpenGL</a> share group,
or otherwise be able to share resources.
When the image's ref count reaches zero, the original <a href="undocumented#GrContext">GrContext</a> will destroy the texture,
asynchronously.
The texture will be decoded and uploaded to be suitable for use with surfaces that have the
supplied destination color space. The color space of the image itself will be determined
from the encoded <a href="#SkImage_MakeCrossContextFromEncoded_data">data</a>.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeCrossContextFromEncoded_context"> <code><strong>context </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeCrossContextFromEncoded_data"> <code><strong>data </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeCrossContextFromEncoded_buildMips"> <code><strong>buildMips </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeCrossContextFromEncoded_dstColorSpace"> <code><strong>dstColorSpace </strong></code> </a></td> <td>
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

<a name="SkImage_MakeFromAdoptedTexture"></a>
## MakeFromAdoptedTexture

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static sk_sp&lt;SkImage&gt; MakeFromAdoptedTexture(GrContext* context,
                                             const GrBackendTexture& backendTexture,
                                             GrSurfaceOrigin surfaceOrigin,
                                             SkAlphaType alphaType = kPremul_SkAlphaType,
                                             sk_sp&lt;SkColorSpace&gt; colorSpace = nullptr)
</pre>

Create a new image from the specified descriptor. Note - <a href="undocumented#Skia">Skia</a> will delete or recycle the
texture when the image is released.
Will return nullptr if the specified back-end texture is unsupported.

### Parameters

<table>  <tr>    <td><a name="SkImage_MakeFromAdoptedTexture_context"> <code><strong>context </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAdoptedTexture_backendTexture"> <code><strong>backendTexture </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAdoptedTexture_surfaceOrigin"> <code><strong>surfaceOrigin </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAdoptedTexture_alphaType"> <code><strong>alphaType </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_MakeFromAdoptedTexture_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
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

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImage_bounds"></a>
## bounds

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkIRect bounds() const
</pre>

### Return Value

incomplete

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

Returns <a href="undocumented#Color_Space">Color Space</a> of <a href="#Image">Image</a>. <a href="undocumented#Color_Space">Color Space</a> may have been a parameter when
<a href="#Image">Image</a> was created, or may have been parsed from encoded data. <a href="undocumented#Skia">Skia</a> may not be
able to draw image respecting returned <a href="undocumented#Color_Space">Color Space</a> or draw into <a href="SkSurface_Reference#Surface">Surface</a> with
returned <a href="undocumented#Color_Space">Color Space</a>.

### Return Value

<a href="undocumented#Color_Space">Color Space</a> <a href="#Image">Image</a> was created with, or nullptr

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

Returns true fi the image will be drawn as a mask, with no intrinsic color of its own.

### Return Value

incomplete

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

### Return Value

incomplete

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

DEPRECATED - 

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

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

Retrieves the back-end <a href="undocumented#API">API</a> handle of the texture. If <a href="#SkImage_getTextureHandle_flushPendingGrContextIO">flushPendingGrContextIO</a> then the
<a href="undocumented#GrContext">GrContext</a> will issue to the back-end <a href="undocumented#API">API</a> any deferred <a href="undocumented#I">I</a>/<a href="undocumented#O">O</a> operations on the texture before
returning.
If '<a href="#SkImage_getTextureHandle_origin">origin</a>' is supplied it will be filled in with the <a href="#SkImage_getTextureHandle_origin">origin</a> of the content drawn
into the image.

### Parameters

<table>  <tr>    <td><a name="SkImage_getTextureHandle_flushPendingGrContextIO"> <code><strong>flushPendingGrContextIO </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_getTextureHandle_origin"> <code><strong>origin </strong></code> </a></td> <td>
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

Encode the image's pixels and return the result as <a href="undocumented#SkData">SkData</a>.
If the image type cannot be encoded, or the requested encoder format is
not supported, this will return nullptr.

### Parameters

<table>  <tr>    <td><a name="SkImage_encodeToData_encodedImageFormat"> <code><strong>encodedImageFormat </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_encodeToData_quality"> <code><strong>quality </strong></code> </a></td> <td>
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

The same method is used both for getting the size necessary for uploading
and retrieving texture data. The <a href="#SkImage_getDeferredTextureImageData_deferredTextureImageUsageParams">deferredTextureImageUsageParams</a> array represents the set of
draws over which to optimize the texture data prior to uploading.

When called with a null <a href="#SkImage_getDeferredTextureImageData_buffer">buffer</a> this returns the size that the client must allocate in order
to create deferred texture data for this image (or zero if this is an inappropriate
candidate). The <a href="#SkImage_getDeferredTextureImageData_buffer">buffer</a> allocated by the client should be 8 byte aligned.
When <a href="#SkImage_getDeferredTextureImageData_buffer">buffer</a> is not null this fills in the deferred texture data for this image in the
provided <a href="#SkImage_getDeferredTextureImageData_buffer">buffer</a> (assuming this is an appropriate candidate image and the <a href="#SkImage_getDeferredTextureImageData_buffer">buffer</a> is
appropriately aligned). Upon success the size written is returned, otherwise 0.
<a href="#SkImage_getDeferredTextureImageData_dstColorSpace">dstColorSpace</a> is the color space of the surface where this texture will ultimately be used.
If the method determines that <a href="undocumented#Mip_Map">Mip Maps</a> are needed, this helps determine the correct strategy
for building them (gamma-correct or not).

<a href="#SkImage_getDeferredTextureImageData_dstColorType">dstColorType</a> is the color type of the surface where this texture will ultimately be used.
This determines the format with which the image will be uploaded to the <a href="undocumented#GPU">GPU</a>. If <a href="#SkImage_getDeferredTextureImageData_dstColorType">dstColorType</a>
does not support color spaces (low bit depth types such as <a href="undocumented#SkColorType">kARGB 4444 SkColorType</a>), then <a href="#SkImage_getDeferredTextureImageData_dstColorSpace">dstColorSpace</a>
must be null.

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
incomplete</td>
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

<a href="SkImage_Reference#BackendTextureReleaseProc">BackendTextureReleaseProc</a>

<a name="SkImage_MakeBackendTextureFromSkImage"></a>
## MakeBackendTextureFromSkImage

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static bool MakeBackendTextureFromSkImage(GrContext* context, sk_sp&lt;SkImage&gt; image,
                                          GrBackendTexture* backendTexture,
                                          BackendTextureReleaseProc* backendTextureReleaseProc)
</pre>

Creates a <a href="undocumented#GrBackendTexture">GrBackendTexture</a> from the provided <a href="#SkImage">SkImage</a>. Returns true on success. The
<a href="undocumented#GrBackendTexture">GrBackendTexture</a> and <a href="SkImage_Reference#BackendTextureReleaseProc">BackendTextureReleaseProc</a> are populated on success. It is the callers
responsibility to call the <a href="SkImage_Reference#BackendTextureReleaseProc">BackendTextureReleaseProc</a> once they have deleted the texture.
Note that the <a href="SkImage_Reference#BackendTextureReleaseProc">BackendTextureReleaseProc</a> allows <a href="undocumented#Skia">Skia</a> to clean up auxiliary data related
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

Attempt to create a <a href="#SkImage_asLegacyBitmap_bitmap">bitmap</a> with the same pixels as the image. The result will always be
a raster-backed <a href="#SkImage_asLegacyBitmap_bitmap">bitmap</a> (texture-backed bitmaps are DEPRECATED, and not supported here).
If the mode is <a href="#SkImage_kRO_LegacyBitmapMode">kRO LegacyBitmapMode</a> (read-only), the resulting <a href="#SkImage_asLegacyBitmap_bitmap">bitmap</a> will be marked as immutable.
On success, returns true. On failure, returns false and the <a href="#SkImage_asLegacyBitmap_bitmap">bitmap</a> parameter will be reset
to empty.

### Parameters

<table>  <tr>    <td><a name="SkImage_asLegacyBitmap_bitmap"> <code><strong>bitmap </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImage_asLegacyBitmap_legacyBitmapMode"> <code><strong>legacyBitmapMode </strong></code> </a></td> <td>
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

<a name="SkImage_isLazyGenerated"></a>
## isLazyGenerated

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isLazyGenerated() const
</pre>

Returns true if <a href="#Image">Image</a> is backed by an image-generator or other service that creates
and caches its pixels or texture on-demand.

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

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

