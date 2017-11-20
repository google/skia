SkSurface Reference
===

# <a name="Surface"></a> Surface

# <a name="SkSurface"></a> Class SkSurface
<a href="#SkSurface">SkSurface</a> is responsible for managing the pixels that a canvas draws into. The pixels can be
allocated either in <a href="undocumented#CPU">CPU</a> memory (a raster surface) or on the <a href="undocumented#GPU">GPU</a> (a <a href="undocumented#GrRenderTarget">GrRenderTarget</a> surface).
<a href="#SkSurface">SkSurface</a> takes care of allocating a <a href="SkCanvas_Reference#SkCanvas">SkCanvas</a> that will <a href="#SkSurface_draw">draw</a> into the surface. Call
surface-><a href="#SkSurface_getCanvas">getCanvas</a> to use that canvas (but don't delete it, it is owned by the surface).
<a href="#SkSurface">SkSurface</a> always has non-zero dimensions. If there is a request for a new surface, and either
of the requested dimensions are zero, then nullptr will be returned.

# <a name="Overview"></a> Overview

## <a name="Subtopics"></a> Subtopics

| topics | description |
| --- | ---  |

## <a name="Member_Functions"></a> Member Functions

| description | function |
| --- | ---  |
| <a href="#SkSurface_MakeFromBackendRenderTarget">MakeFromBackendRenderTarget</a> | Creates <a href="#Surface">Surface</a> matching |
| <a href="#SkSurface_MakeFromBackendTexture">MakeFromBackendTexture</a> |  |
| <a href="#SkSurface_MakeFromBackendTextureAsRenderTarget">MakeFromBackendTextureAsRenderTarget</a> |  |
| <a href="#SkSurface_MakeNull">MakeNull</a> |  |
| <a href="#SkSurface_MakeRaster">MakeRaster</a> |  |
| <a href="#SkSurface_MakeRasterDirect">MakeRasterDirect</a> | Creates <a href="#Surface">Surface</a> from <a href="undocumented#SkImageInfo">SkImageInfo</a> and <a href="#Storage">Pixel Storage</a>. |
| <a href="#SkSurface_MakeRasterDirectReleaseProc">MakeRasterDirectReleaseProc</a> |  |
| <a href="#SkSurface_MakeRasterN32Premul">MakeRasterN32Premul</a> | Creates <a href="#Surface">Surface</a> from <a href="#SkSurface_width">width</a>, <a href="#SkSurface_height">height</a> matching output. |
| <a href="#SkSurface_MakeRenderTarget">MakeRenderTarget</a> |  |
| <a href="#SkSurface_characterize">characterize</a> |  |
| <a href="#SkSurface_draw">draw</a> |  |
| <a href="#SkSurface_flush">flush</a> |  |
| <a href="#SkSurface_flushAndSignalSemaphores">flushAndSignalSemaphores</a> |  |
| <a href="#SkSurface_generationID">generationID</a> |  |
| <a href="#SkSurface_getCanvas">getCanvas</a> |  |
| <a href="#SkSurface_getRenderTargetHandle">getRenderTargetHandle</a> |  |
| <a href="#SkSurface_getTextureHandle">getTextureHandle</a> |  |
| <a href="#SkSurface_height">height</a> |  |
| <a href="#SkSurface_makeImageSnapshot">makeImageSnapshot</a> |  |
| <a href="#SkSurface_makeSurface">makeSurface</a> |  |
| <a href="#SkSurface_notifyContentWillChange">notifyContentWillChange</a> |  |
| <a href="#SkSurface_peekPixels">peekPixels</a> |  |
| <a href="#SkSurface_prepareForExternalIO">prepareForExternalIO</a> |  |
| <a href="#SkSurface_props">props</a> |  |
| <a href="#SkSurface_readPixels">readPixels</a> |  |
| <a href="#SkSurface_wait">wait</a> |  |
| <a href="#SkSurface_width">width</a> |  |

<a name="SkSurface_MakeRasterDirect"></a>
## MakeRasterDirect

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static sk_sp&lt;SkSurface&gt; MakeRasterDirect(const SkImageInfo& imageInfo,
                                         void* pixels, size_t rowBytes,
                                         const SkSurfaceProps* surfaceProps = nullptr)
</pre>

Allocates raster <a href="#Surface">Surface</a>. <a href="SkCanvas_Reference#Canvas">Canvas</a> returned by <a href="#Surface">Surface</a> draws directly into <a href="#SkSurface_MakeRasterDirect_pixels">pixels</a>.

<a href="#Surface">Surface</a> is returned if all parameters are valid.
Valid parameters include:
info dimensions are zero or positive;
info contains <a href="undocumented#Image_Color_Type">Image Color Type</a> and <a href="undocumented#Image_Alpha_Type">Image Alpha Type</a> supported by <a href="undocumented#Raster_Surface">Raster Surface</a>;
<a href="#SkSurface_MakeRasterDirect_pixels">pixels</a> is not nullptr;
<a href="#SkSurface_MakeRasterDirect_rowBytes">rowBytes</a> is large enough to contain info <a href="#SkSurface_width">width</a> <a href="#SkSurface_MakeRasterDirect_pixels">pixels</a> of <a href="undocumented#Image_Color_Type">Image Color Type</a>.

<a href="undocumented#Pixel">Pixel</a> buffer size should be info <a href="#SkSurface_height">height</a> times computed <a href="#SkSurface_MakeRasterDirect_rowBytes">rowBytes</a>.
Pixels are not initialized.
To access <a href="#SkSurface_MakeRasterDirect_pixels">pixels</a> after drawing, call <a href="#SkSurface_flush">flush</a> or <a href="#SkSurface_peekPixels">peekPixels</a>.

### Parameters

<table>  <tr>    <td><a name="SkSurface_MakeRasterDirect_imageInfo"> <code><strong>imageInfo </strong></code> </a></td> <td>
<a href="#SkSurface_width">width</a>, <a href="#SkSurface_height">height</a>, <a href="undocumented#Image_Color_Type">Image Color Type</a>, <a href="undocumented#Image_Alpha_Type">Image Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a>,
of <a href="undocumented#Raster_Surface">Raster Surface</a>; <a href="#SkSurface_width">width</a>, or <a href="#SkSurface_height">height</a>, or both, may be zero</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRasterDirect_pixels"> <code><strong>pixels </strong></code> </a></td> <td>
pointer to destination <a href="#SkSurface_MakeRasterDirect_pixels">pixels</a> buffer</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRasterDirect_rowBytes"> <code><strong>rowBytes </strong></code> </a></td> <td>
interval from one <a href="#Surface">Surface</a> row to the next</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRasterDirect_surfaceProps"> <code><strong>surfaceProps </strong></code> </a></td> <td>
<a href="undocumented#LCD">LCD</a> striping orientation and setting for device independent fonts;
may be nullptr</td>
  </tr>
</table>

### Return Value

<a href="#Surface">Surface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="3f5aeb870104187643197354a7f1d27a">

#### Example Output

~~~~
---
-x-
---
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkSurface_MakeRasterDirectReleaseProc">MakeRasterDirectReleaseProc</a> <a href="#SkSurface_MakeRaster">MakeRaster</a> <a href="#SkSurface_MakeRasterN32Premul">MakeRasterN32Premul</a> <a href="#SkCanvas_MakeRasterDirect">SkCanvas::MakeRasterDirect</a>

---

<a name="SkSurface_MakeRasterDirectReleaseProc"></a>
## MakeRasterDirectReleaseProc

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static sk_sp&lt;SkSurface&gt; 
                                                    MakeRasterDirectReleaseProc(const SkImageInfo& imageInfo,
                                                    void* pixels,
                                                    size_t rowBytes,
                                                    void (*releaseProc)
                                                    (void* pixels,
                                                    void* context) ,
                                                    void* context,
                                                    const SkSurfaceProps* surfaceProps = nullptr)
</pre>

Allocates raster <a href="#Surface">Surface</a>. <a href="SkCanvas_Reference#Canvas">Canvas</a> returned by <a href="#Surface">Surface</a> draws directly into <a href="#SkSurface_MakeRasterDirectReleaseProc_pixels">pixels</a>.
<a href="#SkSurface_MakeRasterDirectReleaseProc_releaseProc">releaseProc</a> is called with <a href="#SkSurface_MakeRasterDirectReleaseProc_pixels">pixels</a> and <a href="#SkSurface_MakeRasterDirectReleaseProc_context">context</a> when <a href="#Surface">Surface</a> is deleted.

<a href="#Surface">Surface</a> is returned if all parameters are valid.
Valid parameters include:
info dimensions are zero or positive;
info contains <a href="undocumented#Image_Color_Type">Image Color Type</a> and <a href="undocumented#Image_Alpha_Type">Image Alpha Type</a> supported by <a href="undocumented#Raster_Surface">Raster Surface</a>;
<a href="#SkSurface_MakeRasterDirectReleaseProc_pixels">pixels</a> is not nullptr;
<a href="#SkSurface_MakeRasterDirectReleaseProc_rowBytes">rowBytes</a> is large enough to contain info <a href="#SkSurface_width">width</a> <a href="#SkSurface_MakeRasterDirectReleaseProc_pixels">pixels</a> of <a href="undocumented#Image_Color_Type">Image Color Type</a>.

<a href="undocumented#Pixel">Pixel</a> buffer size should be info <a href="#SkSurface_height">height</a> times computed <a href="#SkSurface_MakeRasterDirectReleaseProc_rowBytes">rowBytes</a>.
Pixels are not initialized.
To access <a href="#SkSurface_MakeRasterDirectReleaseProc_pixels">pixels</a> after drawing, call <a href="#SkSurface_flush">flush</a> or <a href="#SkSurface_peekPixels">peekPixels</a>.

### Parameters

<table>  <tr>    <td><a name="SkSurface_MakeRasterDirectReleaseProc_imageInfo"> <code><strong>imageInfo </strong></code> </a></td> <td>
<a href="#SkSurface_width">width</a>, <a href="#SkSurface_height">height</a>, <a href="undocumented#Image_Color_Type">Image Color Type</a>, <a href="undocumented#Image_Alpha_Type">Image Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a>,
of <a href="undocumented#Raster_Surface">Raster Surface</a>; <a href="#SkSurface_width">width</a>, or <a href="#SkSurface_height">height</a>, or both, may be zero</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRasterDirectReleaseProc_pixels"> <code><strong>pixels </strong></code> </a></td> <td>
pointer to destination <a href="#SkSurface_MakeRasterDirectReleaseProc_pixels">pixels</a> buffer</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRasterDirectReleaseProc_rowBytes"> <code><strong>rowBytes </strong></code> </a></td> <td>
interval from one <a href="#Surface">Surface</a> row to the next</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRasterDirectReleaseProc_releaseProc"> <code><strong>releaseProc </strong></code> </a></td> <td>
called when <a href="#Surface">Surface</a> is deleted; may be nullptr</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRasterDirectReleaseProc_context"> <code><strong>context </strong></code> </a></td> <td>
passed to <a href="#SkSurface_MakeRasterDirectReleaseProc_releaseProc">releaseProc</a>; may be nullptr</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRasterDirectReleaseProc_surfaceProps"> <code><strong>surfaceProps </strong></code> </a></td> <td>
<a href="undocumented#LCD">LCD</a> striping orientation and setting for device independent fonts;
may be nullptr</td>
  </tr>
</table>

### Return Value

<a href="#Surface">Surface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="8e6530b26ab4096a9a91cfaadda1c568">

#### Example Output

~~~~
---
-x-
---
expected release context
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkSurface_MakeRasterDirect">MakeRasterDirect</a> <a href="#SkSurface_MakeRasterN32Premul">MakeRasterN32Premul</a> <a href="#SkSurface_MakeRaster">MakeRaster</a>

---

<a name="SkSurface_MakeRaster"></a>
## MakeRaster

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static sk_sp&lt;SkSurface&gt; MakeRaster(const SkImageInfo& imageInfo,
                                   size_t rowBytes,
                                   const SkSurfaceProps* surfaceProps)
</pre>

Allocates raster <a href="#Surface">Surface</a>. <a href="SkCanvas_Reference#Canvas">Canvas</a> returned by <a href="#Surface">Surface</a> draws directly into pixels.
Allocates and zeroes pixel memory. <a href="undocumented#Pixel">Pixel</a> memory size is <a href="#SkSurface_MakeRaster_imageInfo">imageInfo</a>.<a href="#SkSurface_height">height</a> times
<a href="#SkSurface_MakeRaster_rowBytes">rowBytes</a>, or times <a href="#SkSurface_MakeRaster_imageInfo">imageInfo</a>.minRowBytes() if <a href="#SkSurface_MakeRaster_rowBytes">rowBytes</a> is zero.
<a href="undocumented#Pixel">Pixel</a> memory is deleted when <a href="#Surface">Surface</a> is deleted.

<a href="#Surface">Surface</a> is returned if all parameters are valid.
Valid parameters include:
info dimensions are zero or positive;
info contains <a href="undocumented#Image_Color_Type">Image Color Type</a> and <a href="undocumented#Image_Alpha_Type">Image Alpha Type</a> supported by <a href="undocumented#Raster_Surface">Raster Surface</a>;
<a href="#SkSurface_MakeRaster_rowBytes">rowBytes</a> is large enough to contain info <a href="#SkSurface_width">width</a> pixels of <a href="undocumented#Image_Color_Type">Image Color Type</a>, or is zero.

If <a href="#SkSurface_MakeRaster_rowBytes">rowBytes</a> is greater than zero, subsequent images returned by <a href="#SkSurface_makeImageSnapshot">makeImageSnapshot</a>
have the same <a href="#SkSurface_MakeRaster_rowBytes">rowBytes</a>.

### Parameters

<table>  <tr>    <td><a name="SkSurface_MakeRaster_imageInfo"> <code><strong>imageInfo </strong></code> </a></td> <td>
<a href="#SkSurface_width">width</a>, <a href="#SkSurface_height">height</a>, <a href="undocumented#Image_Color_Type">Image Color Type</a>, <a href="undocumented#Image_Alpha_Type">Image Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a>,
of <a href="undocumented#Raster_Surface">Raster Surface</a>; <a href="#SkSurface_width">width</a>, or <a href="#SkSurface_height">height</a>, or both, may be zero</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRaster_rowBytes"> <code><strong>rowBytes </strong></code> </a></td> <td>
interval from one <a href="#Surface">Surface</a> row to the next; may be zero</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRaster_surfaceProps"> <code><strong>surfaceProps </strong></code> </a></td> <td>
<a href="undocumented#LCD">LCD</a> striping orientation and setting for device independent fonts;
may be nullptr</td>
  </tr>
</table>

### Return Value

<a href="#Surface">Surface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="a803910ada4f8733f0b62456afead55f">

#### Example Output

~~~~
---
-x-
---
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkSurface_MakeRasterDirect">MakeRasterDirect</a> <a href="#SkSurface_MakeRasterN32Premul">MakeRasterN32Premul</a> <a href="#SkSurface_MakeRasterDirectReleaseProc">MakeRasterDirectReleaseProc</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static sk_sp&lt;SkSurface&gt; MakeRaster(const SkImageInfo& imageInfo,
                                   const SkSurfaceProps* props = nullptr)
</pre>

Allocates raster <a href="#Surface">Surface</a>. <a href="SkCanvas_Reference#Canvas">Canvas</a> returned by <a href="#Surface">Surface</a> draws directly into pixels.
Allocates and zeroes pixel memory. <a href="undocumented#Pixel">Pixel</a> memory size is <a href="#SkSurface_MakeRaster_2_imageInfo">imageInfo</a>.<a href="#SkSurface_height">height</a> times
<a href="#SkSurface_MakeRaster_2_imageInfo">imageInfo</a>.minRowBytes().
<a href="undocumented#Pixel">Pixel</a> memory is deleted when <a href="#Surface">Surface</a> is deleted.

<a href="#Surface">Surface</a> is returned if all parameters are valid.
Valid parameters include:
info dimensions are zero or positive;
info contains <a href="undocumented#Image_Color_Type">Image Color Type</a> and <a href="undocumented#Image_Alpha_Type">Image Alpha Type</a> supported by <a href="undocumented#Raster_Surface">Raster Surface</a>.

### Parameters

<table>  <tr>    <td><a name="SkSurface_MakeRaster_2_imageInfo"> <code><strong>imageInfo </strong></code> </a></td> <td>
<a href="#SkSurface_width">width</a>, <a href="#SkSurface_height">height</a>, <a href="undocumented#Image_Color_Type">Image Color Type</a>, <a href="undocumented#Image_Alpha_Type">Image Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a>,
of <a href="undocumented#Raster_Surface">Raster Surface</a>; <a href="#SkSurface_width">width</a>, or <a href="#SkSurface_height">height</a>, or both, may be zero</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRaster_2_props"> <code><strong>props </strong></code> </a></td> <td>
<a href="undocumented#LCD">LCD</a> striping orientation and setting for device independent fonts;
may be nullptr</td>
  </tr>
</table>

### Return Value

<a href="#Surface">Surface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="c6197d204ef9e4ccfb583242651fb2a7"></fiddle-embed></div>

### See Also

<a href="#SkSurface_MakeRasterDirect">MakeRasterDirect</a> <a href="#SkSurface_MakeRasterN32Premul">MakeRasterN32Premul</a> <a href="#SkSurface_MakeRasterDirectReleaseProc">MakeRasterDirectReleaseProc</a>

---

<a name="SkSurface_MakeRasterN32Premul"></a>
## MakeRasterN32Premul

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static sk_sp&lt;SkSurface&gt; MakeRasterN32Premul(int width, int height,
                                            const SkSurfaceProps* surfaceProps = nullptr)
</pre>

Allocates raster <a href="#Surface">Surface</a>. <a href="SkCanvas_Reference#Canvas">Canvas</a> returned by <a href="#Surface">Surface</a> draws directly into pixels.
Allocates and zeroes pixel memory. <a href="undocumented#Pixel">Pixel</a> memory size is <a href="#SkSurface_height">height</a> times <a href="#SkSurface_width">width</a> times
four. <a href="undocumented#Pixel">Pixel</a> memory is deleted when <a href="#Surface">Surface</a> is deleted.

Internally, sets <a href="undocumented#Image_Info">Image Info</a> to <a href="#SkSurface_width">width</a>, <a href="#SkSurface_height">height</a>, <a href="undocumented#Native_Color_Type">Native Color Type</a>, and
<a href="undocumented#SkAlphaType">kPremul SkAlphaType</a>.

<a href="#Surface">Surface</a> is returned if <a href="#SkSurface_width">width</a> and <a href="#SkSurface_height">height</a> are zero or greater.

Use to create <a href="#Surface">Surface</a> that matches <a href="undocumented#SkPMColor">SkPMColor</a>, the native pixel arrangement on
the platform. <a href="#Surface">Surface</a> drawn to output device skips converting its pixel format.

### Parameters

<table>  <tr>    <td><a name="SkSurface_MakeRasterN32Premul_width"> <code><strong>width </strong></code> </a></td> <td>
pixel column count; must be zero or greater</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRasterN32Premul_height"> <code><strong>height </strong></code> </a></td> <td>
pixel row count; must be zero or greater</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRasterN32Premul_surfaceProps"> <code><strong>surfaceProps </strong></code> </a></td> <td>
<a href="undocumented#LCD">LCD</a> striping orientation and setting for device independent
fonts; may be nullptr</td>
  </tr>
</table>

### Return Value

<a href="#Surface">Surface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="b932a2bd68455fb0af2e7a1ed19e36b3"></fiddle-embed></div>

### See Also

<a href="#SkSurface_MakeRasterDirect">MakeRasterDirect</a> <a href="#SkSurface_MakeRasterN32Premul">MakeRasterN32Premul</a> <a href="#SkSurface_MakeRasterDirectReleaseProc">MakeRasterDirectReleaseProc</a>

---

<a name="SkSurface_MakeFromBackendTexture"></a>
## MakeFromBackendTexture

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static sk_sp&lt;SkSurface&gt; MakeFromBackendTexture(GrContext* context,
                                               const GrBackendTexture& backendTexture,
                                               GrSurfaceOrigin origin,
                                               int sampleCnt,
                                               sk_sp&lt;SkColorSpace&gt; colorSpace,
                                               const SkSurfaceProps* surfaceProps)
</pre>

Wraps a <a href="undocumented#GPU">GPU</a>-backed texture into <a href="#Surface">Surface</a>. Caller must ensure the texture is
valid for the lifetime of returned <a href="#Surface">Surface</a>. If <a href="#SkSurface_MakeFromBackendTexture_sampleCnt">sampleCnt</a> greater than zero,
creates an intermediate <a href="undocumented#MSAA">MSAA</a> <a href="#Surface">Surface</a> which is used for drawing <a href="#SkSurface_MakeFromBackendTexture_backendTexture">backendTexture</a>.

<a href="#Surface">Surface</a> is returned if all parameters are valid. <a href="#SkSurface_MakeFromBackendTexture_backendTexture">backendTexture</a> is valid if
its pixel configuration agrees with <a href="#SkSurface_MakeFromBackendTexture_colorSpace">colorSpace</a> and <a href="#SkSurface_MakeFromBackendTexture_context">context</a>; for instance, if
<a href="#SkSurface_MakeFromBackendTexture_backendTexture">backendTexture</a> has an <a href="undocumented#sRGB">sRGB</a> configuration, then <a href="#SkSurface_MakeFromBackendTexture_context">context</a> must support <a href="undocumented#sRGB">sRGB</a>,
and <a href="#SkSurface_MakeFromBackendTexture_colorSpace">colorSpace</a> must be present. Further, <a href="#SkSurface_MakeFromBackendTexture_backendTexture">backendTexture</a> <a href="#SkSurface_width">width</a> and <a href="#SkSurface_height">height</a> must
not exceed <a href="#SkSurface_MakeFromBackendTexture_context">context</a> capabilities, and the <a href="#SkSurface_MakeFromBackendTexture_context">context</a> must be able to support
backend textures.

If <a href="undocumented#SK_SUPPORT_GPU">SK SUPPORT GPU</a> is defined as zero, has no effect and returns nullptr.

### Parameters

<table>  <tr>    <td><a name="SkSurface_MakeFromBackendTexture_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkSurface_MakeFromBackendTexture_backendTexture"> <code><strong>backendTexture </strong></code> </a></td> <td>
texture residing on <a href="undocumented#GPU">GPU</a></td>
  </tr>  <tr>    <td><a name="SkSurface_MakeFromBackendTexture_origin"> <code><strong>origin </strong></code> </a></td> <td>
one of: <a href="undocumented#GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>, <a href="undocumented#GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a></td>
  </tr>  <tr>    <td><a name="SkSurface_MakeFromBackendTexture_sampleCnt"> <code><strong>sampleCnt </strong></code> </a></td> <td>
samples per pixel, or 0 to disable full scene anti-aliasing</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeFromBackendTexture_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeFromBackendTexture_surfaceProps"> <code><strong>surfaceProps </strong></code> </a></td> <td>
<a href="undocumented#LCD">LCD</a> striping orientation and setting for device independent
fonts; may be nullptr</td>
  </tr>
</table>

### Return Value

<a href="#Surface">Surface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

<a href="undocumented#GrBackendTexture">GrBackendTexture</a> <a href="#SkSurface_MakeFromBackendRenderTarget">MakeFromBackendRenderTarget</a> <a href="#SkSurface_MakeRenderTarget">MakeRenderTarget</a>

---

<a name="SkSurface_MakeFromBackendRenderTarget"></a>
## MakeFromBackendRenderTarget

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static sk_sp&lt;SkSurface&gt; MakeFromBackendRenderTarget(GrContext* context,
                                                    const GrBackendRenderTarget& backendRenderTarget,
                                                    GrSurfaceOrigin origin,
                                                    sk_sp&lt;SkColorSpace&gt; colorSpace,
                                                    const SkSurfaceProps* surfaceProps)
</pre>

If <a href="undocumented#SK_SUPPORT_GPU">SK SUPPORT GPU</a> is defined as zero, has no effect and returns nullptr.

### Parameters

<table>  <tr>    <td><a name="SkSurface_MakeFromBackendRenderTarget_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkSurface_MakeFromBackendRenderTarget_backendRenderTarget"> <code><strong>backendRenderTarget </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeFromBackendRenderTarget_origin"> <code><strong>origin </strong></code> </a></td> <td>
one of: <a href="undocumented#GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>, <a href="undocumented#GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a></td>
  </tr>  <tr>    <td><a name="SkSurface_MakeFromBackendRenderTarget_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeFromBackendRenderTarget_surfaceProps"> <code><strong>surfaceProps </strong></code> </a></td> <td>
<a href="undocumented#LCD">LCD</a> striping orientation and setting for device independent
fonts; may be nullptr</td>
  </tr>
</table>

### Return Value

<a href="#Surface">Surface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkSurface_MakeFromBackendTextureAsRenderTarget"></a>
## MakeFromBackendTextureAsRenderTarget

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static sk_sp&lt;SkSurface&gt; 
                                                             MakeFromBackendTextureAsRenderTarget(GrContext* context,
                                                             const GrBackendTexture& backendTexture,
                                                             GrSurfaceOrigin origin,
                                                             int sampleCnt,
                                                             sk_sp&lt;SkColorSpace&gt; colorSpace,
                                                             const SkSurfaceProps* surfaceProps)
</pre>

Used to wrap a <a href="undocumented#GPU">GPU</a>-backed texture as a <a href="#SkSurface">SkSurface</a>. <a href="undocumented#Skia">Skia</a> will treat the texture as
a rendering target only, but unlike NewFromBackendRenderTarget, <a href="undocumented#Skia">Skia</a> will manage and own
the associated render target objects (but not the provided texture). <a href="undocumented#Skia">Skia</a> will not assume
ownership of the texture and the client must ensure the texture is valid for the lifetime
of the <a href="#SkSurface">SkSurface</a>.

If <a href="undocumented#SK_SUPPORT_GPU">SK SUPPORT GPU</a> is defined as zero, has no effect and returns nullptr.

### Parameters

<table>  <tr>    <td><a name="SkSurface_MakeFromBackendTextureAsRenderTarget_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture"> <code><strong>backendTexture </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeFromBackendTextureAsRenderTarget_origin"> <code><strong>origin </strong></code> </a></td> <td>
one of: <a href="undocumented#GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>, <a href="undocumented#GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a></td>
  </tr>  <tr>    <td><a name="SkSurface_MakeFromBackendTextureAsRenderTarget_sampleCnt"> <code><strong>sampleCnt </strong></code> </a></td> <td>
samples per pixel, or 0 to disable full scene anti-aliasing</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeFromBackendTextureAsRenderTarget_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
range of colors</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeFromBackendTextureAsRenderTarget_surfaceProps"> <code><strong>surfaceProps </strong></code> </a></td> <td>
<a href="undocumented#LCD">LCD</a> striping orientation and setting for device independent
fonts; may be nullptr</td>
  </tr>
</table>

### Return Value

<a href="#Surface">Surface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

<a href="#SkSurface_MakeFromBackendRenderTarget">MakeFromBackendRenderTarget</a> <a href="#SkSurface_MakeRenderTarget">MakeRenderTarget</a>

---

<a name="SkSurface_MakeRenderTarget"></a>
## MakeRenderTarget

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static sk_sp&lt;SkSurface&gt; MakeRenderTarget(GrContext* context,
                                         SkBudgeted budgeted,
                                         const SkImageInfo& imageInfo,
                                         int sampleCount,
                                         GrSurfaceOrigin surfaceOrigin,
                                         const SkSurfaceProps* surfaceProps,
                                         bool shouldCreateWithMips = false)
</pre>

Returns offscreen <a href="#Surface">Surface</a> on <a href="undocumented#GPU">GPU</a> indicated by <a href="#SkSurface_MakeRenderTarget_context">context</a>. Allocates memory for
pixels, based on the <a href="#SkSurface_width">width</a>, <a href="#SkSurface_height">height</a>, and <a href="undocumented#Image_Color_Type">Image Color Type</a> in ImageInfo.  <a href="#SkSurface_MakeRenderTarget_budgeted">budgeted</a>
selects whether allocation for offscreen pixels is tracked by <a href="#SkSurface_MakeRenderTarget_context">context</a>. <a href="#SkSurface_MakeRenderTarget_imageInfo">imageInfo</a>
describes the pixel format in <a href="undocumented#Image_Color_Type">Image Color Type</a>, and transparency in
<a href="undocumented#Image_Alpha_Type">Image Alpha Type</a>, and color matching in <a href="undocumented#Color_Space">Color Space</a>.

<a href="#SkSurface_MakeRenderTarget_sampleCount">sampleCount</a> requests the number of samples per pixel. 
Pass zero to disable multisample anti-aliasing.  The request is rounded
up to the next supported count, or rounded down if it is larger than the
maximum supported count.

<a href="#SkSurface_MakeRenderTarget_surfaceOrigin">surfaceOrigin</a> pins either the top-left or the bottom-left corner to the origin.

<a href="#SkSurface_MakeRenderTarget_shouldCreateWithMips">shouldCreateWithMips</a> hints that <a href="SkImage_Reference#Image">Image</a> returned by <a href="#SkSurface_makeImageSnapshot">makeImageSnapshot</a> is mipmap.

If <a href="undocumented#SK_SUPPORT_GPU">SK SUPPORT GPU</a> is defined as zero, has no effect and returns nullptr.

### Parameters

<table>  <tr>    <td><a name="SkSurface_MakeRenderTarget_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRenderTarget_budgeted"> <code><strong>budgeted </strong></code> </a></td> <td>
if allocation counts against <a href="#SkSurface_MakeRenderTarget_context">context</a> <a href="undocumented#Resource_Cache_Limits">Resource Cache Limits</a></td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRenderTarget_imageInfo"> <code><strong>imageInfo </strong></code> </a></td> <td>
<a href="#SkSurface_width">width</a>, <a href="#SkSurface_height">height</a>, <a href="undocumented#Image_Color_Type">Image Color Type</a>, <a href="undocumented#Image_Alpha_Type">Image Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a>;
<a href="#SkSurface_width">width</a>, or <a href="#SkSurface_height">height</a>, or both, may be zero</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRenderTarget_sampleCount"> <code><strong>sampleCount </strong></code> </a></td> <td>
samples per pixel, or 0 to disable full scene anti-aliasing</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRenderTarget_surfaceOrigin"> <code><strong>surfaceOrigin </strong></code> </a></td> <td>
one of: <a href="undocumented#GrSurfaceOrigin">kBottomLeft GrSurfaceOrigin</a>, <a href="undocumented#GrSurfaceOrigin">kTopLeft GrSurfaceOrigin</a></td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRenderTarget_surfaceProps"> <code><strong>surfaceProps </strong></code> </a></td> <td>
<a href="undocumented#LCD">LCD</a> striping orientation and setting for device independent
fonts; may be nullptr</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRenderTarget_shouldCreateWithMips"> <code><strong>shouldCreateWithMips </strong></code> </a></td> <td>
hint that <a href="#Surface">Surface</a> will host mipmap images</td>
  </tr>
</table>

### Return Value

<a href="#Surface">Surface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="4e2364b3c077769b9453edc62c7ed37a"></fiddle-embed></div>

### See Also

<a href="#SkSurface_MakeFromBackendRenderTarget">MakeFromBackendRenderTarget</a> <a href="#SkSurface_MakeFromBackendTextureAsRenderTarget">MakeFromBackendTextureAsRenderTarget</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static sk_sp&lt;SkSurface&gt; MakeRenderTarget(GrContext* context,
                                         SkBudgeted budgeted,
                                         const SkImageInfo& imageInfo,
                                         int sampleCount,
                                         const SkSurfaceProps* props)
</pre>

Returns offscreen <a href="#Surface">Surface</a> on <a href="undocumented#GPU">GPU</a> indicated by <a href="#SkSurface_MakeRenderTarget_2_context">context</a>. Allocates memory for
pixels, based on the <a href="#SkSurface_width">width</a>, <a href="#SkSurface_height">height</a>, and <a href="undocumented#Image_Color_Type">Image Color Type</a> in ImageInfo.  <a href="#SkSurface_MakeRenderTarget_2_budgeted">budgeted</a>
selects whether allocation for offscreen pixels is tracked by <a href="#SkSurface_MakeRenderTarget_2_context">context</a>. <a href="#SkSurface_MakeRenderTarget_2_imageInfo">imageInfo</a>
describes the pixel format in <a href="undocumented#Image_Color_Type">Image Color Type</a>, and transparency in
<a href="undocumented#Image_Alpha_Type">Image Alpha Type</a>, and color matching in <a href="undocumented#Color_Space">Color Space</a>.

<a href="#SkSurface_MakeRenderTarget_2_sampleCount">sampleCount</a> requests the number of samples per pixel. 
Pass zero to disable multisample anti-aliasing.  The request is rounded
up to the next supported count, or rounded down if it is larger than the
maximum supported count.

<a href="#Surface">Surface</a> bottom-left corner is pinned to the origin.

### Parameters

<table>  <tr>    <td><a name="SkSurface_MakeRenderTarget_2_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRenderTarget_2_budgeted"> <code><strong>budgeted </strong></code> </a></td> <td>
if allocation counts against <a href="#SkSurface_MakeRenderTarget_2_context">context</a> <a href="undocumented#Resource_Cache_Limits">Resource Cache Limits</a></td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRenderTarget_2_imageInfo"> <code><strong>imageInfo </strong></code> </a></td> <td>
<a href="#SkSurface_width">width</a>, <a href="#SkSurface_height">height</a>, <a href="undocumented#Image_Color_Type">Image Color Type</a>, <a href="undocumented#Image_Alpha_Type">Image Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a>,
of <a href="undocumented#Raster_Surface">Raster Surface</a>; <a href="#SkSurface_width">width</a>, or <a href="#SkSurface_height">height</a>, or both, may be zero</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRenderTarget_2_sampleCount"> <code><strong>sampleCount </strong></code> </a></td> <td>
samples per pixel, or 0 to disable multisample anti-aliasing</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRenderTarget_2_props"> <code><strong>props </strong></code> </a></td> <td>
<a href="undocumented#LCD">LCD</a> striping orientation and setting for device independent
fonts; may be nullptr</td>
  </tr>
</table>

### Return Value

<a href="#Surface">Surface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="4473daad1b11784a9869431af72c93e0"><div><a href="undocumented#LCD">LCD</a> text takes advantage of raster striping to improve resolution. Only one of
the four combinations is correct, depending on whether the monitor's <a href="undocumented#LCD">LCD</a> is
horizontal or vertical, and whether the order of the stripes is red blue green
or red green blue.</div></fiddle-embed></div>

### See Also

<a href="#SkSurface_MakeFromBackendRenderTarget">MakeFromBackendRenderTarget</a> <a href="#SkSurface_MakeFromBackendTextureAsRenderTarget">MakeFromBackendTextureAsRenderTarget</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static sk_sp&lt;SkSurface&gt; MakeRenderTarget(GrContext* context,
                                         SkBudgeted budgeted,
                                         const SkImageInfo& imageInfo)
</pre>

Returns offscreen <a href="#Surface">Surface</a> on <a href="undocumented#GPU">GPU</a> indicated by <a href="#SkSurface_MakeRenderTarget_3_context">context</a>. Allocates memory for
pixels, based on the <a href="#SkSurface_width">width</a>, <a href="#SkSurface_height">height</a>, and <a href="undocumented#Image_Color_Type">Image Color Type</a> in ImageInfo.  <a href="#SkSurface_MakeRenderTarget_3_budgeted">budgeted</a>
selects whether allocation for offscreen pixels is tracked by <a href="#SkSurface_MakeRenderTarget_3_context">context</a>. <a href="#SkSurface_MakeRenderTarget_3_imageInfo">imageInfo</a>
describes the pixel format in <a href="undocumented#Image_Color_Type">Image Color Type</a>, and transparency in
<a href="undocumented#Image_Alpha_Type">Image Alpha Type</a>, and color matching in <a href="undocumented#Color_Space">Color Space</a>.

<a href="#Surface">Surface</a> bottom-left corner is pinned to the origin.

### Parameters

<table>  <tr>    <td><a name="SkSurface_MakeRenderTarget_3_context"> <code><strong>context </strong></code> </a></td> <td>
<a href="undocumented#GPU_Context">GPU Context</a></td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRenderTarget_3_budgeted"> <code><strong>budgeted </strong></code> </a></td> <td>
if allocation counts against <a href="#SkSurface_MakeRenderTarget_3_context">context</a> <a href="undocumented#Resource_Cache_Limits">Resource Cache Limits</a></td>
  </tr>  <tr>    <td><a name="SkSurface_MakeRenderTarget_3_imageInfo"> <code><strong>imageInfo </strong></code> </a></td> <td>
<a href="#SkSurface_width">width</a>, <a href="#SkSurface_height">height</a>, <a href="undocumented#Image_Color_Type">Image Color Type</a>, <a href="undocumented#Image_Alpha_Type">Image Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a>,
of <a href="undocumented#Raster_Surface">Raster Surface</a>; <a href="#SkSurface_width">width</a>, or <a href="#SkSurface_height">height</a>, or both, may be zero</td>
  </tr>
</table>

### Return Value

<a href="#Surface">Surface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="5c7629c15e9ac93f098335e72560fa2e"></fiddle-embed></div>

### See Also

<a href="#SkSurface_MakeFromBackendRenderTarget">MakeFromBackendRenderTarget</a> <a href="#SkSurface_MakeFromBackendTextureAsRenderTarget">MakeFromBackendTextureAsRenderTarget</a>

---

<a name="SkSurface_MakeNull"></a>
## MakeNull

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static sk_sp&lt;SkSurface&gt; MakeNull(int width, int height)
</pre>

Returns <a href="#Surface">Surface</a> without backing pixels. Drawing to <a href="SkCanvas_Reference#Canvas">Canvas</a> returned from <a href="#Surface">Surface</a>
has no effect. Calling <a href="#SkSurface_makeImageSnapshot">makeImageSnapshot</a> on returned <a href="#Surface">Surface</a> returns nullptr.

### Parameters

<table>  <tr>    <td><a name="SkSurface_MakeNull_width"> <code><strong>width </strong></code> </a></td> <td>
one or greater</td>
  </tr>  <tr>    <td><a name="SkSurface_MakeNull_height"> <code><strong>height </strong></code> </a></td> <td>
one or greater</td>
  </tr>
</table>

### Return Value

<a href="#Surface">Surface</a> if <a href="#SkSurface_width">width</a> and <a href="#SkSurface_height">height</a> are positive; otherwise, nullptr

### Example

<div><fiddle-embed name="99a54b814ccab7d2b1143c88581649ff">

#### Example Output

~~~~
SkSurface::MakeNull(0, 0) == nullptr
surf->makeImageSnapshot() == nullptr
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkSurface_MakeRaster">MakeRaster</a> <a href="#SkSurface_MakeRenderTarget">MakeRenderTarget</a>

---

<a name="SkSurface_width"></a>
## width

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
int width() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkSurface_height"></a>
## height

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
int height() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkSurface_generationID"></a>
## generationID

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
uint32_t generationID()
</pre>

Returns a unique non-zero, unique value identifying the content of this
surface. Each time the content is changed changed, either by drawing
into this surface, or explicitly calling notifyContentChanged()) this
method will return a new value.
If this surface is empty (i.e. has a zero-dimensions), this will return
0.

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

## <a name="SkSurface_ContentChangeMode"></a> Enum SkSurface::ContentChangeMode

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
enum <a href="#SkSurface_ContentChangeMode">ContentChangeMode</a> {
<a href="#SkSurface_kDiscard_ContentChangeMode">kDiscard ContentChangeMode</a>,
<a href="#SkSurface_kRetain_ContentChangeMode">kRetain ContentChangeMode</a>,
};</pre>

Modes that can be passed to <a href="#SkSurface_notifyContentWillChange">notifyContentWillChange</a>

### Constants

<table>
  <tr>
    <td><a name="SkSurface_kDiscard_ContentChangeMode"> <code><strong>SkSurface::kDiscard_ContentChangeMode </strong></code> </a></td><td>Use this mode if it is known that the upcoming content changes will</td><td>clear or overwrite prior contents, thus making them discardable.</td>
  </tr>
  <tr>
    <td><a name="SkSurface_kRetain_ContentChangeMode"> <code><strong>SkSurface::kRetain_ContentChangeMode </strong></code> </a></td><td>Use this mode if prior surface contents need to be preserved or</td><td>if in doubt.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete



<a name="SkSurface_notifyContentWillChange"></a>
## notifyContentWillChange

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void notifyContentWillChange(ContentChangeMode mode)
</pre>

Call this if the contents are about to change. This will (lazily) force a new
value to be returned from <a href="#SkSurface_generationID">generationID</a> when it is called next.

### Parameters

<table>  <tr>    <td><a name="SkSurface_notifyContentWillChange_mode"> <code><strong>mode </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

## <a name="SkSurface_BackendHandleAccess"></a> Enum SkSurface::BackendHandleAccess

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
enum <a href="#SkSurface_BackendHandleAccess">BackendHandleAccess</a> {
<a href="#SkSurface_kFlushRead_BackendHandleAccess">kFlushRead BackendHandleAccess</a>,
<a href="#SkSurface_kFlushWrite_BackendHandleAccess">kFlushWrite BackendHandleAccess</a>,
<a href="#SkSurface_kDiscardWrite_BackendHandleAccess">kDiscardWrite BackendHandleAccess</a>,
};</pre>

### Constants

<table>
  <tr>
    <td><a name="SkSurface_kFlushRead_BackendHandleAccess"> <code><strong>SkSurface::kFlushRead_BackendHandleAccess </strong></code> </a></td><td>caller may read from the backend object</td><td></td>
  </tr>
  <tr>
    <td><a name="SkSurface_kFlushWrite_BackendHandleAccess"> <code><strong>SkSurface::kFlushWrite_BackendHandleAccess </strong></code> </a></td><td>caller may write to the backend object</td><td></td>
  </tr>
  <tr>
    <td><a name="SkSurface_kDiscardWrite_BackendHandleAccess"> <code><strong>SkSurface::kDiscardWrite_BackendHandleAccess </strong></code> </a></td><td>caller must over-write the entire backend object</td><td></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete



<a name="SkSurface_getTextureHandle"></a>
## getTextureHandle

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
GrBackendObject getTextureHandle(BackendHandleAccess backendHandleAccess)
</pre>

Retrieves the backend <a href="undocumented#API">API</a> handle of the texture used by this surface, or 0 if the surface
is not backed by a <a href="undocumented#GPU">GPU</a> texture.
The returned texture-handle is only valid until the next <a href="#SkSurface_draw">draw</a>-call into the surface,
or the surface is deleted.

### Parameters

<table>  <tr>    <td><a name="SkSurface_getTextureHandle_backendHandleAccess"> <code><strong>backendHandleAccess </strong></code> </a></td> <td>
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

<a name="SkSurface_getRenderTargetHandle"></a>
## getRenderTargetHandle

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool getRenderTargetHandle(GrBackendObject* backendObject,
                           BackendHandleAccess backendHandleAccess)
</pre>

Retrieves the <a href="undocumented#GPU">GPU</a>-backed <a href="undocumented#GrRenderTarget">GrRenderTarget</a> for this surface.  Callers must
ensure this function returns 'true' or else <a href="#SkSurface_getRenderTargetHandle_backendObject">backendObject</a> will be invalid.
In <a href="undocumented#OpenGL">OpenGL</a> this will return the framebuffer object ID.

### Parameters

<table>  <tr>    <td><a name="SkSurface_getRenderTargetHandle_backendObject"> <code><strong>backendObject </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkSurface_getRenderTargetHandle_backendHandleAccess"> <code><strong>backendHandleAccess </strong></code> </a></td> <td>
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

<a name="SkSurface_getCanvas"></a>
## getCanvas

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkCanvas* getCanvas()
</pre>

Return a canvas that will <a href="#SkSurface_draw">draw</a> into this surface. This will always
return the same canvas for a given surface, and is manged/owned by the
surface. It should not be used when its parent surface has gone out of
scope.

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkSurface_makeSurface"></a>
## makeSurface

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
sk_sp&lt;SkSurface&gt; makeSurface(const SkImageInfo& imageInfo)
</pre>

Return a new surface that is "" with this one, in that it will
efficiently be able to be drawn into this surface. Typical calling
pattern:

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
<a href="#SkSurface">SkSurface</a>*  =</pre>

### Parameters

<table>  <tr>    <td><a name="SkSurface_makeSurface_imageInfo"> <code><strong>imageInfo </strong></code> </a></td> <td>
<a href="#SkSurface_width">width</a>, <a href="#SkSurface_height">height</a>, <a href="undocumented#Image_Color_Type">Image Color Type</a>, <a href="undocumented#Image_Alpha_Type">Image Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a>,
of <a href="undocumented#Raster_Surface">Raster Surface</a>; <a href="#SkSurface_width">width</a>, or <a href="#SkSurface_height">height</a>, or both, may be zero</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkSurface_makeImageSnapshot"></a>
## makeImageSnapshot

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
sk_sp&lt;SkImage&gt; makeImageSnapshot()
</pre>

Returns an image of the current state of the surface pixels up to this
point. Subsequent changes to the surface (by drawing into its canvas)
will not be reflected in this image. For the <a href="undocumented#GPU">GPU</a>-backend, the budgeting
decision for the snapped image will match that of the surface.

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkSurface_draw"></a>
## draw

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void draw(SkCanvas* canvas, SkScalar x, SkScalar y, const SkPaint* paint)
</pre>

Though the caller could get a snapshot image explicitly, and <a href="#SkSurface_draw">draw</a> that,
it seems that directly drawing a surface into another <a href="#SkSurface_draw_canvas">canvas</a> might be
a common pattern, and that we could possibly be more efficient, since
we'd know that the "" need only live until we've handed it off
to the <a href="#SkSurface_draw_canvas">canvas</a>.

### Parameters

<table>  <tr>    <td><a name="SkSurface_draw_canvas"> <code><strong>canvas </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkSurface_draw_x"> <code><strong>x </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkSurface_draw_y"> <code><strong>y </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkSurface_draw_paint"> <code><strong>paint </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkSurface_peekPixels"></a>
## peekPixels

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool peekPixels(SkPixmap* pixmap)
</pre>

If the surface has direct access to its pixels (i.e. they are in local
RAM) return true, and if not null, set the <a href="#SkSurface_peekPixels_pixmap">pixmap</a> parameter to point to the information
about the surface's pixels. The pixel address in the <a href="#SkSurface_peekPixels_pixmap">pixmap</a> is only valid while
the surface object is in scope, and no <a href="undocumented#API">API</a> call is made on the surface
or its canvas.
On failure, returns false and the <a href="#SkSurface_peekPixels_pixmap">pixmap</a> parameter is ignored.

### Parameters

<table>  <tr>    <td><a name="SkSurface_peekPixels_pixmap"> <code><strong>pixmap </strong></code> </a></td> <td>
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

<a name="SkSurface_readPixels"></a>
## readPixels

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool readPixels(const SkPixmap& dst, int srcX, int srcY)
</pre>

Copy the pixels from the surface into the specified pixmap,
converting them into the pixmap's format. The surface pixels are read
starting at the specified (<a href="#SkSurface_readPixels_srcX">srcX</a>,<a href="#SkSurface_readPixels_srcY">srcY</a>) location.
The pixmap and (<a href="#SkSurface_readPixels_srcX">srcX</a>,<a href="#SkSurface_readPixels_srcY">srcY</a>) offset specifies a source rectangle

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
<a href="SkRect_Reference#SkRect">SkRect</a> srcR;
srcR.setXYWH(srcX, srcY, pixmap.width(), pixmap.<a href="#SkSurface_height">height</a>);</pre>

The source rectangle is intersected with the bounds of the base-layer. If this intersection is not empty,
then we have two sets of pixels (of equal size). Replace the <a href="#SkSurface_readPixels_dst">dst</a> pixels with the
corresponding src pixels, performing any colortype/alphatype transformations needed
(in the case where the src and <a href="#SkSurface_readPixels_dst">dst</a> have different colortypes or alphatypes).
This call can fail, returning false, for several reasons:
if source rectangle does not intersect the surface bounds;
if the requested colortype/alphatype cannot be converted from the surface's types.

### Parameters

<table>  <tr>    <td><a name="SkSurface_readPixels_dst"> <code><strong>dst </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkSurface_readPixels_srcX"> <code><strong>srcX </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkSurface_readPixels_srcY"> <code><strong>srcY </strong></code> </a></td> <td>
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

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes,
                int srcX, int srcY)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkSurface_readPixels_2_dstInfo"> <code><strong>dstInfo </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkSurface_readPixels_2_dstPixels"> <code><strong>dstPixels </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkSurface_readPixels_2_dstRowBytes"> <code><strong>dstRowBytes </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkSurface_readPixels_2_srcX"> <code><strong>srcX </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkSurface_readPixels_2_srcY"> <code><strong>srcY </strong></code> </a></td> <td>
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

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool readPixels(const SkBitmap& dst, int srcX, int srcY)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkSurface_readPixels_3_dst"> <code><strong>dst </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkSurface_readPixels_3_srcX"> <code><strong>srcX </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkSurface_readPixels_3_srcY"> <code><strong>srcY </strong></code> </a></td> <td>
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

<a name="SkSurface_props"></a>
## props

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
const SkSurfaceProps& props() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkSurface_prepareForExternalIO"></a>
## prepareForExternalIO

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void prepareForExternalIO()
</pre>

Issue any pending surface <a href="undocumented#I">I</a>/<a href="undocumented#O">O</a> to the current <a href="undocumented#GPU">GPU</a>-backed <a href="undocumented#API">API</a> and resolve any surface <a href="undocumented#MSAA">MSAA</a>.
The <a href="#SkSurface_flush">flush</a> calls below are the new preferred way to <a href="#SkSurface_flush">flush</a> calls to a surface, and this call
will eventually be removed.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkSurface_flush"></a>
## flush

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void flush()
</pre>

Issue any pending surface <a href="undocumented#I">I</a>/<a href="undocumented#O">O</a> to the current <a href="undocumented#GPU">GPU</a>-backed <a href="undocumented#API">API</a>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkSurface_flushAndSignalSemaphores"></a>
## flushAndSignalSemaphores

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
GrSemaphoresSubmitted flushAndSignalSemaphores(int numSemaphores,
                                               GrBackendSemaphore signalSemaphores[])
</pre>

Issue any pending surface <a href="undocumented#I">I</a>/<a href="undocumented#O">O</a> to the <a href="undocumented#GPU">GPU</a>-backed <a href="undocumented#API">API</a>. After issuing all commands,
<a href="#SkSurface_flushAndSignalSemaphores_numSemaphores">numSemaphores</a> semaphores will be signaled by the gpu. The client passes in an array of
<a href="#SkSurface_flushAndSignalSemaphores_numSemaphores">numSemaphores</a> <a href="#GrBackendSemaphore">GrBackendSemaphores</a>. In general these <a href="undocumented#GrBackendSemaphore">GrBackendSemaphore</a>'s can be either
initialized or not. If they are initialized, the backend uses the passed in semaphore.
If it is not initialized, a new semaphore is created and the <a href="undocumented#GrBackendSemaphore">GrBackendSemaphore</a> object
is initialized with that semaphore.
The client will own and be responsible for deleting the underlying semaphores that are stored
and returned in initialized <a href="undocumented#GrBackendSemaphore">GrBackendSemaphore</a> objects. The <a href="undocumented#GrBackendSemaphore">GrBackendSemaphore</a> objects
themselves can be deleted as soon as this function returns.
If the backend <a href="undocumented#API">API</a> is <a href="undocumented#OpenGL">OpenGL</a> only uninitialized <a href="#GrBackendSemaphore">GrBackendSemaphores</a> are supported.
If the backend <a href="undocumented#API">API</a> is <a href="undocumented#Vulkan">Vulkan</a> either initialized or unitialized semaphores are supported.
If unitialized, the semaphores which are created will be valid for use only with the VkDevice
with which they were created.
If this call returns GrSemaphoresSubmited::kNo, the <a href="undocumented#GPU">GPU</a> backend will not have created or
added any semaphores to signal on the <a href="undocumented#GPU">GPU</a>. Thus the client should not have the <a href="undocumented#GPU">GPU</a> <a href="#SkSurface_wait">wait</a> on
any of the semaphores. However, any pending surface <a href="undocumented#I">I</a>/<a href="undocumented#O">O</a> will still be flushed.

### Parameters

<table>  <tr>    <td><a name="SkSurface_flushAndSignalSemaphores_numSemaphores"> <code><strong>numSemaphores </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkSurface_flushAndSignalSemaphores_signalSemaphores"> <code><strong>signalSemaphores </strong></code> </a></td> <td>
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

<a name="SkSurface_wait"></a>
## wait

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool wait(int numSemaphores, const GrBackendSemaphore* waitSemaphores)
</pre>

Inserts a list of <a href="undocumented#GPU">GPU</a> semaphores that the current <a href="undocumented#GPU">GPU</a>-backed <a href="undocumented#API">API</a> must <a href="#SkSurface_wait">wait</a> on before
executing any more commands on the <a href="undocumented#GPU">GPU</a> for this surface. <a href="undocumented#Skia">Skia</a> will take ownership of the
underlying semaphores and delete them once they have been signaled and waited on.
If this call returns false, then the <a href="undocumented#GPU">GPU</a> backend will not <a href="#SkSurface_wait">wait</a> on any passed in semaphores,
and the client will still own the semaphores.

### Parameters

<table>  <tr>    <td><a name="SkSurface_wait_numSemaphores"> <code><strong>numSemaphores </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkSurface_wait_waitSemaphores"> <code><strong>waitSemaphores </strong></code> </a></td> <td>
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

<a name="SkSurface_characterize"></a>
## characterize

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool characterize(SkSurfaceCharacterization* characterization) const
</pre>

This creates a <a href="#SkSurface_characterize_characterization">characterization</a> of this <a href="#SkSurface">SkSurface</a>'s properties that can
be used to perform gpu-backend preprocessing in a separate thread (via
the <a href="undocumented#SkDeferredDisplayListRecorder">SkDeferredDisplayListRecorder</a>).
It will return false on failure (e.g., if the <a href="#SkSurface">SkSurface</a> is cpu-backed).

### Parameters

<table>  <tr>    <td><a name="SkSurface_characterize_characterization"> <code><strong>characterization </strong></code> </a></td> <td>
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

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void draw(SkDeferredDisplayList* deferredDisplayList)
</pre>

Draw a deferred display list (created via <a href="undocumented#SkDeferredDisplayListRecorder">SkDeferredDisplayListRecorder</a>).
The <a href="#SkSurface_draw">draw</a> will be skipped if the characterization stored in the display list
isn't compatible with this surface.

### Parameters

<table>  <tr>    <td><a name="SkSurface_draw_2_deferredDisplayList"> <code><strong>deferredDisplayList </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

