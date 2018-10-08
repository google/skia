SkSurface Reference
===

<a name='SkSurface'></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='#SkSurface'>SkSurface</a> : public <a href='undocumented#SkRefCnt'>SkRefCnt</a> {
public:
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRasterDirect'>MakeRasterDirect</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo, void* pixels,
                                      size_t rowBytes,
                                      const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRasterDirectReleaseProc'>MakeRasterDirectReleaseProc</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo, void* pixels,
                                    size_t rowBytes,
                                    void (*releaseProc)(void* pixels, void* context),
                                    void* context, const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRaster'>MakeRaster</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo, size_t rowBytes,
                                       const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRaster_2'>MakeRaster</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo,
                                       const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* props = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRasterN32Premul'>MakeRasterN32Premul</a>(int width, int height,
                                                const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeFromBackendTexture'>MakeFromBackendTexture</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                                   const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture,
                                                   <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> origin, int sampleCnt,
                                                   <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> colorType,
                                                   sk_sp<<a href='undocumented#SkColorSpace'>SkColorSpace</a>> colorSpace,
                                                   const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                                const <a href='undocumented#GrBackendRenderTarget'>GrBackendRenderTarget</a>& backendRenderTarget,
                                                <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> origin,
                                                <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> colorType,
                                                sk_sp<<a href='undocumented#SkColorSpace'>SkColorSpace</a>> colorSpace,
                                                const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget'>MakeFromBackendTextureAsRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                                            const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture,
                                                            <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> origin,
                                                            int sampleCnt,
                                                            <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> colorType,
                                                            sk_sp<<a href='undocumented#SkColorSpace'>SkColorSpace</a>> colorSpace,
                                                            const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* context, <a href='undocumented#SkBudgeted'>SkBudgeted</a> budgeted,
                                             const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo,
                                             int sampleCount, <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> surfaceOrigin,
                                             const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps,
                                             bool shouldCreateWithMips = false);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRenderTarget_2'>MakeRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* context, <a href='undocumented#SkBudgeted'>SkBudgeted</a> budgeted,
                                             const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo, int sampleCount,
                                             const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* props);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRenderTarget_3'>MakeRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* context, <a href='undocumented#SkBudgeted'>SkBudgeted</a> budgeted,
                                             const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRenderTarget_4'>MakeRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                             const <a href='undocumented#SkSurfaceCharacterization'>SkSurfaceCharacterization</a>& characterization,
                                             <a href='undocumented#SkBudgeted'>SkBudgeted</a> budgeted);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeNull'>MakeNull</a>(int width, int height);
    int <a href='#SkSurface_width'>width</a>() const;
    int <a href='#SkSurface_height'>height</a>() const;
    uint32_t <a href='#SkSurface_generationID'>generationID</a>();

    enum <a href='#SkSurface_ContentChangeMode'>ContentChangeMode</a> {
        <a href='#SkSurface_kDiscard_ContentChangeMode'>kDiscard_ContentChangeMode</a>,
        <a href='#SkSurface_kRetain_ContentChangeMode'>kRetain_ContentChangeMode</a>,
    };

    void <a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a>(<a href='#SkSurface_ContentChangeMode'>ContentChangeMode</a> mode);

    enum <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> {
        <a href='#SkSurface_kFlushRead_BackendHandleAccess'>kFlushRead_BackendHandleAccess</a>,
        <a href='#SkSurface_kFlushWrite_BackendHandleAccess'>kFlushWrite_BackendHandleAccess</a>,
        <a href='#SkSurface_kDiscardWrite_BackendHandleAccess'>kDiscardWrite_BackendHandleAccess</a>,
    };

    static const <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_kFlushRead_TextureHandleAccess'>kFlushRead_TextureHandleAccess</a> =
            <a href='#SkSurface_kFlushRead_BackendHandleAccess'>kFlushRead_BackendHandleAccess</a>;
    static const <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_kFlushWrite_TextureHandleAccess'>kFlushWrite_TextureHandleAccess</a> =
            <a href='#SkSurface_kFlushWrite_BackendHandleAccess'>kFlushWrite_BackendHandleAccess</a>;
    static const <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_kDiscardWrite_TextureHandleAccess'>kDiscardWrite_TextureHandleAccess</a> =
            <a href='#SkSurface_kDiscardWrite_BackendHandleAccess'>kDiscardWrite_BackendHandleAccess</a>;
    <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='#SkSurface_getBackendTexture'>getBackendTexture</a>(<a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> backendHandleAccess);
    <a href='undocumented#GrBackendRenderTarget'>GrBackendRenderTarget</a> <a href='#SkSurface_getBackendRenderTarget'>getBackendRenderTarget</a>(<a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> backendHandleAccess);
    <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* <a href='#SkSurface_getCanvas'>getCanvas</a>();
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkSurface'>SkSurface</a>> <a href='#SkSurface_makeSurface'>makeSurface</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo);
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a>();
    void <a href='#SkSurface_draw'>draw</a>(<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* canvas, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint);
    bool <a href='#SkSurface_peekPixels'>peekPixels</a>(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>* pixmap);
    bool <a href='#SkSurface_readPixels'>readPixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& dst, int srcX, int srcY);
    bool <a href='#SkSurface_readPixels_2'>readPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& dstInfo, void* dstPixels, size_t dstRowBytes,
                    int srcX, int srcY);
    bool <a href='#SkSurface_readPixels_3'>readPixels</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& dst, int srcX, int srcY);
    void <a href='#SkSurface_writePixels'>writePixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& src, int dstX, int dstY);
    void <a href='#SkSurface_writePixels_2'>writePixels</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& src, int dstX, int dstY);
    const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>& <a href='#SkSurface_props'>props</a>() const;
    void <a href='#SkSurface_prepareForExternalIO'>prepareForExternalIO</a>();
    void <a href='#SkSurface_flush'>flush</a>();
    <a href='undocumented#GrSemaphoresSubmitted'>GrSemaphoresSubmitted</a> <a href='#SkSurface_flushAndSignalSemaphores'>flushAndSignalSemaphores</a>(int numSemaphores,
                                                   <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a> signalSemaphores[]);
    bool <a href='#SkSurface_wait'>wait</a>(int numSemaphores, const <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a>* waitSemaphores);
    bool <a href='#SkSurface_characterize'>characterize</a>(<a href='undocumented#SkSurfaceCharacterization'>SkSurfaceCharacterization</a>* characterization) const;
    bool <a href='#SkSurface_draw_2'>draw</a>(<a href='undocumented#SkDeferredDisplayList'>SkDeferredDisplayList</a>* deferredDisplayList);
};
</pre>

<a href='#SkSurface'>SkSurface</a> is responsible for managing the pixels that a canvas draws into

<a name='SkSurface_MakeRasterDirect'></a>
## MakeRasterDirect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Allocates raster <a href='#Surface'>Surface</a>

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRasterDirect_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirect_pixels'><code><strong>pixels</strong></code></a></td>
    <td>pointer to destination <a href='#SkSurface_MakeRasterDirect_pixels'>pixels</a> buffer</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirect_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>interval from one <a href='#Surface'>Surface</a> row to the next</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirect_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for device independent fonts</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if all parameters are valid

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

<a href='#SkSurface_MakeRasterDirectReleaseProc'>MakeRasterDirectReleaseProc</a> <a href='#SkSurface_MakeRaster'>MakeRaster</a><sup><a href='#SkSurface_MakeRaster_2'>[2]</a></sup> <a href='#SkSurface_MakeRasterN32Premul'>MakeRasterN32Premul</a> <a href='SkCanvas_Reference#SkCanvas_MakeRasterDirect'>SkCanvas::MakeRasterDirect</a>

---

<a name='SkSurface_MakeRasterDirectReleaseProc'></a>
## MakeRasterDirectReleaseProc

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Allocates raster <a href='#Surface'>Surface</a>

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRasterDirectReleaseProc_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirectReleaseProc_pixels'><code><strong>pixels</strong></code></a></td>
    <td>pointer to destination <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>pixels</a> buffer</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirectReleaseProc_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>interval from one <a href='#Surface'>Surface</a> row to the next</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirectReleaseProc_releaseProc'><code><strong>releaseProc</strong></code></a></td>
    <td>called when <a href='#Surface'>Surface</a> is deleted</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirectReleaseProc_context'><code><strong>context</strong></code></a></td>
    <td>passed to <a href='#SkSurface_MakeRasterDirectReleaseProc_releaseProc'>releaseProc</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirectReleaseProc_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for device independent fonts</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if all parameters are valid

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

<a href='#SkSurface_MakeRasterDirect'>MakeRasterDirect</a> <a href='#SkSurface_MakeRasterN32Premul'>MakeRasterN32Premul</a> <a href='#SkSurface_MakeRaster'>MakeRaster</a><sup><a href='#SkSurface_MakeRaster_2'>[2]</a></sup>

---

<a name='SkSurface_MakeRaster'></a>
## MakeRaster

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Allocates raster <a href='#Surface'>Surface</a>

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRaster_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRaster_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>interval from one <a href='#Surface'>Surface</a> row to the next</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRaster_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for device independent fonts</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if all parameters are valid

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

<a href='#SkSurface_MakeRasterDirect'>MakeRasterDirect</a> <a href='#SkSurface_MakeRasterN32Premul'>MakeRasterN32Premul</a> <a href='#SkSurface_MakeRasterDirectReleaseProc'>MakeRasterDirectReleaseProc</a>

---

<a name='SkSurface_MakeRaster_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Allocates raster <a href='#Surface'>Surface</a>

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRaster_2_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRaster_2_props'><code><strong>props</strong></code></a></td>
    <td>LCD striping orientation and setting for device independent fonts</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if all parameters are valid

### Example

<div><fiddle-embed name="c6197d204ef9e4ccfb583242651fb2a7"></fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeRasterDirect'>MakeRasterDirect</a> <a href='#SkSurface_MakeRasterN32Premul'>MakeRasterN32Premul</a> <a href='#SkSurface_MakeRasterDirectReleaseProc'>MakeRasterDirectReleaseProc</a>

---

<a name='SkSurface_MakeRasterN32Premul'></a>
## MakeRasterN32Premul

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Allocates raster <a href='#Surface'>Surface</a>

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRasterN32Premul_width'><code><strong>width</strong></code></a></td>
    <td>pixel column count</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterN32Premul_height'><code><strong>height</strong></code></a></td>
    <td>pixel row count</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterN32Premul_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for device independent
fonts</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if all parameters are valid

### Example

<div><fiddle-embed name="b932a2bd68455fb0af2e7a1ed19e36b3">

#### Example Output

~~~~
---
-x-
---
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeRasterDirect'>MakeRasterDirect</a> <a href='#SkSurface_MakeRasterN32Premul'>MakeRasterN32Premul</a> <a href='#SkSurface_MakeRasterDirectReleaseProc'>MakeRasterDirectReleaseProc</a>

---

<a name='SkSurface_MakeFromBackendTexture'></a>
## MakeFromBackendTexture

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Wraps a GPU

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td>texture residing on GPU</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_origin'><code><strong>origin</strong></code></a></td>
    <td>one of</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_sampleCnt'><code><strong>sampleCnt</strong></code></a></td>
    <td>samples per pixel</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_colorType'><code><strong>colorType</strong></code></a></td>
    <td>one of <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a> </td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td>range of colors</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for device independent
fonts</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if all parameters are valid

### Example

<div><fiddle-embed name="d3aec071998f871809f515e58abb1b0e" gpu="true" cpu="true"></fiddle-embed></div>

### See Also

<a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a> <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a><sup><a href='#SkSurface_MakeRenderTarget_2'>[2]</a></sup><sup><a href='#SkSurface_MakeRenderTarget_3'>[3]</a></sup><sup><a href='#SkSurface_MakeRenderTarget_4'>[4]</a></sup>

---

<a name='SkSurface_MakeFromBackendRenderTarget'></a>
## MakeFromBackendRenderTarget

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Wraps a GPU

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeFromBackendRenderTarget_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'><code><strong>backendRenderTarget</strong></code></a></td>
    <td>GPU intermediate memory buffer</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendRenderTarget_origin'><code><strong>origin</strong></code></a></td>
    <td>one of</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendRenderTarget_colorType'><code><strong>colorType</strong></code></a></td>
    <td>one of <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a> </td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendRenderTarget_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td>range of colors</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendRenderTarget_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for device independent
fonts</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if all parameters are valid

### Example

<pre style="padding: 1em 1em 1em 1em; font-size: 13px width: 62.5em; background-color: #f0f0f0">

    SkPaint paint;
    paint.setTextSize(32);
    GrContext* context = canvas->getGrContext();
    if (!context) {
         canvas->drawString("GPU only!", 20, 40, paint);
         return;
    }
    sk_sp<SkSurface> gpuSurface = SkSurface::MakeFromBackendRenderTarget(context,
            backEndRenderTarget, kTopLeft_GrSurfaceOrigin, kRGBA_8888_SkColorType,
            nullptr, nullptr);
    auto surfaceCanvas = gpuSurface->getCanvas();
    surfaceCanvas->drawString("GPU rocks!", 20, 40, paint);
    sk_sp<SkImage> image(gpuSurface->makeImageSnapshot());
    canvas->drawImage(image, 0, 0);

</pre>

### See Also

<a href='#SkSurface_MakeFromBackendTexture'>MakeFromBackendTexture</a> <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a><sup><a href='#SkSurface_MakeRenderTarget_2'>[2]</a></sup><sup><a href='#SkSurface_MakeRenderTarget_3'>[3]</a></sup><sup><a href='#SkSurface_MakeRenderTarget_4'>[4]</a></sup>

---

<a name='SkSurface_MakeFromBackendTextureAsRenderTarget'></a>
## MakeFromBackendTextureAsRenderTarget

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Wraps a GPU

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td>texture residing on GPU</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_origin'><code><strong>origin</strong></code></a></td>
    <td>one of</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_sampleCnt'><code><strong>sampleCnt</strong></code></a></td>
    <td>samples per pixel</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_colorType'><code><strong>colorType</strong></code></a></td>
    <td>one of <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a> </td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td>range of colors</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for device independent
fonts</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if all parameters are valid

### Example

<div><fiddle-embed name="5e87093b9cbe95124ae14cbe77091eb7" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a> <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a><sup><a href='#SkSurface_MakeRenderTarget_2'>[2]</a></sup><sup><a href='#SkSurface_MakeRenderTarget_3'>[3]</a></sup><sup><a href='#SkSurface_MakeRenderTarget_4'>[4]</a></sup>

---

<a name='SkSurface_MakeRenderTarget'></a>
## MakeRenderTarget

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Returns <a href='#Surface'>Surface</a> on GPU indicated by <a href='#SkSurface_MakeRenderTarget_context'>context</a>

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRenderTarget_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_budgeted'><code><strong>budgeted</strong></code></a></td>
    <td>one of</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_sampleCount'><code><strong>sampleCount</strong></code></a></td>
    <td>samples per pixel</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_surfaceOrigin'><code><strong>surfaceOrigin</strong></code></a></td>
    <td>one of</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for device independent
fonts</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_shouldCreateWithMips'><code><strong>shouldCreateWithMips</strong></code></a></td>
    <td>hint that <a href='#Surface'>Surface</a> will host <a href='undocumented#Mip_Map'>Mip Map</a> images</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if all parameters are valid

### Example

<div><fiddle-embed name="67b6609471a3f1ed0f4b1657004cdecb" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget'>MakeFromBackendTextureAsRenderTarget</a>

---

<a name='SkSurface_MakeRenderTarget_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Returns <a href='#Surface'>Surface</a> on GPU indicated by <a href='#SkSurface_MakeRenderTarget_2_context'>context</a>

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRenderTarget_2_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_2_budgeted'><code><strong>budgeted</strong></code></a></td>
    <td>one of</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_2_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_2_sampleCount'><code><strong>sampleCount</strong></code></a></td>
    <td>samples per pixel</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_2_props'><code><strong>props</strong></code></a></td>
    <td>LCD striping orientation and setting for device independent
fonts</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if all parameters are valid

### Example

<div><fiddle-embed name="640321e8ecfb3f9329f3bc6e1f02485f" gpu="true" cpu="true"><div>LCD text takes advantage of raster striping to improve resolution</div></fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget'>MakeFromBackendTextureAsRenderTarget</a>

---

<a name='SkSurface_MakeRenderTarget_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Returns <a href='#Surface'>Surface</a> on GPU indicated by <a href='#SkSurface_MakeRenderTarget_3_context'>context</a>

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRenderTarget_3_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_3_budgeted'><code><strong>budgeted</strong></code></a></td>
    <td>one of</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_3_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if all parameters are valid

### Example

<div><fiddle-embed name="5c7629c15e9ac93f098335e72560fa2e" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget'>MakeFromBackendTextureAsRenderTarget</a>

---

<a name='SkSurface_MakeRenderTarget_4'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Returns <a href='#SkSurface'>SkSurface</a> on GPU indicated by <a href='#SkSurface_MakeRenderTarget_4_context'>context</a> that is compatible with the provided
<a href='#SkSurface_MakeRenderTarget_4_characterization'>characterization</a>

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRenderTarget_4_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_4_characterization'><code><strong>characterization</strong></code></a></td>
    <td>description of the desired <a href='#SkSurface'>SkSurface</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_4_budgeted'><code><strong>budgeted</strong></code></a></td>
    <td>one of</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if all parameters are valid

### See Also

<a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget'>MakeFromBackendTextureAsRenderTarget</a>

---

<a name='SkSurface_MakeNull'></a>
## MakeNull

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Returns <a href='#Surface'>Surface</a> without backing pixels

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeNull_width'><code><strong>width</strong></code></a></td>
    <td>one or greater</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeNull_height'><code><strong>height</strong></code></a></td>
    <td>one or greater</td>
  </tr>
</table>

### Return Value

<a href='#Surface'>Surface</a> if width and height are positive

### Example

<div><fiddle-embed name="99a54b814ccab7d2b1143c88581649ff">

#### Example Output

~~~~
SkSurface::MakeNull(0, 0) == nullptr
surf->makeImageSnapshot() == nullptr
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeRaster'>MakeRaster</a><sup><a href='#SkSurface_MakeRaster_2'>[2]</a></sup> <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a><sup><a href='#SkSurface_MakeRenderTarget_2'>[2]</a></sup><sup><a href='#SkSurface_MakeRenderTarget_3'>[3]</a></sup><sup><a href='#SkSurface_MakeRenderTarget_4'>[4]</a></sup>

---

## <a name='Property'>Property</a>

<a name='SkSurface_width'></a>
## width

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkSurface_width'>width</a>(
</pre>

Returns pixel count in each row

### Return Value

number of pixel columns

### Example

<div><fiddle-embed name="df066b56dd97c7c589fd2bb6a2539de8">

#### Example Output

~~~~
surface width=37  canvas width=37
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkSurface_height'>height</a>(

---

<a name='SkSurface_height'></a>
## height

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkSurface_height'>height</a>(
</pre>

Returns pixel row count

### Return Value

number of pixel rows

### Example

<div><fiddle-embed name="20571cc23e3146deaa09046b64cc0aef">

#### Example Output

~~~~
surface height=1000  canvas height=1000
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkSurface_width'>width</a>(

---

<a name='SkSurface_generationID'></a>
## generationID

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkSurface_generationID'>generationID</a>(
</pre>

Returns unique value identifying the content of <a href='#Surface'>Surface</a>

### Return Value

unique content identifier

### Example

<div><fiddle-embed name="be9574c4a14f891e1abb4ec2b1e51d6c">

#### Example Output

~~~~
surface generationID: 1
surface generationID: 2
surface generationID: 3
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a> <a href='#SkSurface_ContentChangeMode'>ContentChangeMode</a> <a href='#SkSurface_getCanvas'>getCanvas</a>

---

## <a name='SkSurface_ContentChangeMode'>Enum SkSurface::ContentChangeMode</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkSurface_ContentChangeMode'>ContentChangeMode</a> {
        <a href='#SkSurface_kDiscard_ContentChangeMode'>kDiscard_ContentChangeMode</a>,
        <a href='#SkSurface_kRetain_ContentChangeMode'>kRetain_ContentChangeMode</a>,
    };
</pre>

<a href='#SkSurface_ContentChangeMode'>ContentChangeMode</a> members are parameters to <a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSurface_kDiscard_ContentChangeMode'><code>SkSurface::kDiscard_ContentChangeMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#Line # discards surface on change ##</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Pass to <a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a> to discard surface contents when
the surface is cleared or overwritten</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSurface_kRetain_ContentChangeMode'><code>SkSurface::kRetain_ContentChangeMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#Line # preserves surface on change ##</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Pass to <a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a> when to preserve surface contents</td>
  </tr>
</table>

### See Also

<a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a> <a href='#SkSurface_generationID'>generationID</a>

## <a name='Miscellaneous'>Miscellaneous</a>

<a name='SkSurface_notifyContentWillChange'></a>
## notifyContentWillChange

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a>(<a href='#SkSurface_ContentChangeMode'>ContentChangeMode</a> mode
</pre>

Notifies that <a href='#Surface'>Surface</a> contents will be changed by code outside of SkiaPrivate: Can we deprecate this?

### Parameters

<table>  <tr>    <td><a name='SkSurface_notifyContentWillChange_mode'><code><strong>mode</strong></code></a></td>
    <td>one of</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="be9574c4a14f891e1abb4ec2b1e51d6c"></fiddle-embed></div>

### See Also

<a href='#SkSurface_ContentChangeMode'>ContentChangeMode</a> <a href='#SkSurface_generationID'>generationID</a>

---

## <a name='SkSurface_BackendHandleAccess'>Enum SkSurface::BackendHandleAccess</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> {
        <a href='#SkSurface_kFlushRead_BackendHandleAccess'>kFlushRead_BackendHandleAccess</a>,
        <a href='#SkSurface_kFlushWrite_BackendHandleAccess'>kFlushWrite_BackendHandleAccess</a>,
        <a href='#SkSurface_kDiscardWrite_BackendHandleAccess'>kDiscardWrite_BackendHandleAccess</a>,
    };

    static const <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_kFlushRead_TextureHandleAccess'>kFlushRead_TextureHandleAccess</a> =
            <a href='#SkSurface_kFlushRead_BackendHandleAccess'>kFlushRead_BackendHandleAccess</a>;
    static const <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_kFlushWrite_TextureHandleAccess'>kFlushWrite_TextureHandleAccess</a> =
            <a href='#SkSurface_kFlushWrite_BackendHandleAccess'>kFlushWrite_BackendHandleAccess</a>;
    static const <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_kDiscardWrite_TextureHandleAccess'>kDiscardWrite_TextureHandleAccess</a> =
            <a href='#SkSurface_kDiscardWrite_BackendHandleAccess'>kDiscardWrite_BackendHandleAccess</a>;
</pre>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSurface_kFlushRead_BackendHandleAccess'><code>SkSurface::kFlushRead_BackendHandleAccess</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Caller may read from the back</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSurface_kFlushWrite_BackendHandleAccess'><code>SkSurface::kFlushWrite_BackendHandleAccess</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Caller may write to the back</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSurface_kDiscardWrite_BackendHandleAccess'><code>SkSurface::kDiscardWrite_BackendHandleAccess</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Caller must overwrite the entire back</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSurface_kFlushRead_TextureHandleAccess'><code>SkSurface::kFlushRead_TextureHandleAccess</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Deprecated.

</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSurface_kFlushWrite_TextureHandleAccess'><code>SkSurface::kFlushWrite_TextureHandleAccess</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Deprecated.

</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSurface_kDiscardWrite_TextureHandleAccess'><code>SkSurface::kDiscardWrite_TextureHandleAccess</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Deprecated.

</td>
  </tr>
</table>

### See Also

<a href='#SkSurface_getBackendTexture'>getBackendTexture</a> <a href='#SkSurface_getBackendRenderTarget'>getBackendRenderTarget</a>

<a name='SkSurface_getBackendTexture'></a>
## getBackendTexture

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='#SkSurface_getBackendTexture'>getBackendTexture</a>(<a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> backendHandleAccess
</pre>

Retrieves the back

### Parameters

<table>  <tr>    <td><a name='SkSurface_getBackendTexture_backendHandleAccess'><code><strong>backendHandleAccess</strong></code></a></td>
    <td>one of</td>
  </tr>
</table>

### Return Value

GPU texture reference

### See Also

<a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_getBackendRenderTarget'>getBackendRenderTarget</a>

---

<a name='SkSurface_getBackendRenderTarget'></a>
## getBackendRenderTarget

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#GrBackendRenderTarget'>GrBackendRenderTarget</a> <a href='#SkSurface_getBackendRenderTarget'>getBackendRenderTarget</a>(<a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> backendHandleAccess
</pre>

Retrieves the back

### Parameters

<table>  <tr>    <td><a name='SkSurface_getBackendRenderTarget_backendHandleAccess'><code><strong>backendHandleAccess</strong></code></a></td>
    <td>one of</td>
  </tr>
</table>

### Return Value

GPU render target reference

### See Also

<a href='undocumented#GrBackendRenderTarget'>GrBackendRenderTarget</a> <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_getBackendTexture'>getBackendTexture</a>

---

<a name='SkSurface_getCanvas'></a>
## getCanvas

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>
</pre>

Returns <a href='SkCanvas_Reference#Canvas'>Canvas</a> that draws into <a href='#Surface'>Surface</a>

### Return Value

drawing <a href='SkCanvas_Reference#Canvas'>Canvas</a> for <a href='#Surface'>Surface</a>

### Example

<div><fiddle-embed name="33d0c5ad5a4810e533ae1010e29f8b75"></fiddle-embed></div>

### See Also

<a href='#SkSurface_makeSurface'>makeSurface</a> <a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a> <a href='#SkSurface_draw'>draw</a><sup><a href='#SkSurface_draw_2'>[2]</a></sup>

---

<a name='SkSurface_makeSurface'></a>
## makeSurface

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Returns a compatible <a href='#Surface'>Surface</a>

### Parameters

<table>  <tr>    <td><a name='SkSurface_makeSurface_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width</td>
  </tr>
</table>

### Return Value

compatible <a href='#Surface'>Surface</a> or nullptr

### Example

<div><fiddle-embed name="a9889b519a26896b900da0444e423c61"></fiddle-embed></div>

### See Also

<a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a> <a href='#SkSurface_getCanvas'>getCanvas</a> <a href='#SkSurface_draw'>draw</a><sup><a href='#SkSurface_draw_2'>[2]</a></sup>

---

<a name='SkSurface_makeImageSnapshot'></a>
## makeImageSnapshot

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Returns <a href='SkImage_Reference#Image'>Image</a> capturing <a href='#Surface'>Surface</a> contents

### Return Value

<a href='SkImage_Reference#Image'>Image</a> initialized with <a href='#Surface'>Surface</a> contents

### Example

<div><fiddle-embed name="46f1fa0d95e590a64bed0140407ce5f7"></fiddle-embed></div>

### See Also

<a href='#SkSurface_draw'>draw</a><sup><a href='#SkSurface_draw_2'>[2]</a></sup> <a href='#SkSurface_getCanvas'>getCanvas</a>

---

## <a name='Pixels'>Pixels</a>

<a name='SkSurface_draw'></a>
## draw

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkSurface_draw'>draw</a>(<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>
</pre>

Draws <a href='#Surface'>Surface</a> contents to <a href='#SkSurface_draw_canvas'>canvas</a>

### Parameters

<table>  <tr>    <td><a name='SkSurface_draw_canvas'><code><strong>canvas</strong></code></a></td>
    <td><a href='SkCanvas_Reference#Canvas'>Canvas</a> drawn into</td>
  </tr>
  <tr>    <td><a name='SkSurface_draw_x'><code><strong>x</strong></code></a></td>
    <td>horizontal offset in <a href='SkCanvas_Reference#Canvas'>Canvas</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_draw_y'><code><strong>y</strong></code></a></td>
    <td>vertical offset in <a href='SkCanvas_Reference#Canvas'>Canvas</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_draw_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> containing <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0de693f4d8dd898a60be8cfba23952be"></fiddle-embed></div>

### See Also

<a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a> <a href='#SkSurface_getCanvas'>getCanvas</a>

---

<a name='SkSurface_peekPixels'></a>
## peekPixels

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkSurface_peekPixels'>peekPixels</a>(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>
</pre>

Copies <a href='#Surface'>Surface</a> pixel address

### Parameters

<table>  <tr>    <td><a name='SkSurface_peekPixels_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td>storage for pixel state if pixels are readable</td>
  </tr>
</table>

### Return Value

true if <a href='#Surface'>Surface</a> has direct access to pixels

### Example

<div><fiddle-embed name="8c6184f22cfe068f021704cf92a147a1"></fiddle-embed></div>

### See Also

<a href='#SkSurface_readPixels'>readPixels</a><sup><a href='#SkSurface_readPixels_2'>[2]</a></sup><sup><a href='#SkSurface_readPixels_3'>[3]</a></sup> <a href='#SkSurface_writePixels'>writePixels</a><sup><a href='#SkSurface_writePixels_2'>[2]</a></sup>

---

<a name='SkSurface_readPixels'></a>
## readPixels

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkSurface_readPixels'>readPixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels to <a href='#SkSurface_readPixels_dst'>dst</a>

<table>  <tr>
    <td>Source and destination rectangles do not intersect</td>
  </tr>  <tr>
    <td><a href='SkPixmap_Reference#Pixmap'>Pixmap</a> pixels could not be allocated</td>
  </tr>  <tr>
    <td><a href='#SkSurface_readPixels_dst'>dst</a></td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkSurface_readPixels_dst'><code><strong>dst</strong></code></a></td>
    <td>storage for pixels copied from <a href='#Surface'>Surface</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_srcX'><code><strong>srcX</strong></code></a></td>
    <td>offset into readable pixels on x</td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_srcY'><code><strong>srcY</strong></code></a></td>
    <td>offset into readable pixels on y</td>
  </tr>
</table>

### Return Value

true if pixels were copied

### Example

<div><fiddle-embed name="9f454fb93bca6482598d198b4121f0a6"></fiddle-embed></div>

### See Also

<a href='#SkSurface_peekPixels'>peekPixels</a> <a href='#SkSurface_writePixels'>writePixels</a><sup><a href='#SkSurface_writePixels_2'>[2]</a></sup>

---

<a name='SkSurface_readPixels_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkSurface_readPixels'>readPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels from <a href='SkCanvas_Reference#Canvas'>Canvas</a> into <a href='#SkSurface_readPixels_2_dstPixels'>dstPixels</a>

<table>  <tr>
    <td>Source and destination rectangles do not intersect</td>
  </tr>  <tr>
    <td><a href='#Surface'>Surface</a> pixels could not be converted to <a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a></td>
  </tr>  <tr>
    <td><a href='#SkSurface_readPixels_2_dstRowBytes'>dstRowBytes</a> is too small to contain one row of pixels</td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkSurface_readPixels_2_dstInfo'><code><strong>dstInfo</strong></code></a></td>
    <td>width</td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_2_dstPixels'><code><strong>dstPixels</strong></code></a></td>
    <td>storage for pixels</td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_2_dstRowBytes'><code><strong>dstRowBytes</strong></code></a></td>
    <td>size of one destination row</td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_2_srcX'><code><strong>srcX</strong></code></a></td>
    <td>offset into readable pixels on x</td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_2_srcY'><code><strong>srcY</strong></code></a></td>
    <td>offset into readable pixels on y</td>
  </tr>
</table>

### Return Value

true if pixels were copied

### Example

<div><fiddle-embed name="484d60dab5d846bf28c7a4d48892324a"><div>A black oval drawn on a red background provides an image to copy</div></fiddle-embed></div>

### See Also

<a href='#SkSurface_peekPixels'>peekPixels</a> <a href='#SkSurface_writePixels'>writePixels</a><sup><a href='#SkSurface_writePixels_2'>[2]</a></sup>

---

<a name='SkSurface_readPixels_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkSurface_readPixels'>readPixels</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels from <a href='#Surface'>Surface</a> into bitmap

<table>  <tr>
    <td>Source and destination rectangles do not intersect</td>
  </tr>  <tr>
    <td><a href='#Surface'>Surface</a> pixels could not be converted to <a href='#SkSurface_readPixels_3_dst'>dst</a></td>
  </tr>  <tr>
    <td><a href='#SkSurface_readPixels_3_dst'>dst</a> pixels could not be allocated</td>
  </tr>  <tr>
    <td><a href='#SkSurface_readPixels_3_dst'>dst</a></td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkSurface_readPixels_3_dst'><code><strong>dst</strong></code></a></td>
    <td>storage for pixels copied from <a href='#Surface'>Surface</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_3_srcX'><code><strong>srcX</strong></code></a></td>
    <td>offset into readable pixels on x</td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_3_srcY'><code><strong>srcY</strong></code></a></td>
    <td>offset into readable pixels on y</td>
  </tr>
</table>

### Return Value

true if pixels were copied

### Example

<div><fiddle-embed name="2d991a231e49d1de13eeb2ba9b440e01"></fiddle-embed></div>

### See Also

<a href='#SkSurface_peekPixels'>peekPixels</a> <a href='#SkSurface_writePixels'>writePixels</a><sup><a href='#SkSurface_writePixels_2'>[2]</a></sup>

---

<a name='SkSurface_writePixels'></a>
## writePixels

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkSurface_writePixels'>writePixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels from the <a href='#SkSurface_writePixels_src'>src</a> <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> to the <a href='#Surface'>Surface</a> <code></code>

### Parameters

<table>  <tr>    <td><a name='SkSurface_writePixels_src'><code><strong>src</strong></code></a></td>
    <td>storage for pixels to copy to <a href='#Surface'>Surface</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_writePixels_dstX'><code><strong>dstX</strong></code></a></td>
    <td>x</td>
  </tr>
  <tr>    <td><a name='SkSurface_writePixels_dstY'><code><strong>dstY</strong></code></a></td>
    <td>y</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="760793bcf0ef193fa61ea03e6e8fc825"></fiddle-embed></div>

### See Also

<a href='#SkSurface_readPixels'>readPixels</a><sup><a href='#SkSurface_readPixels_2'>[2]</a></sup><sup><a href='#SkSurface_readPixels_3'>[3]</a></sup> <a href='#SkSurface_peekPixels'>peekPixels</a>

---

<a name='SkSurface_writePixels_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkSurface_writePixels'>writePixels</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels from the <a href='#SkSurface_writePixels_2_src'>src</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> to the <a href='#Surface'>Surface</a> <code></code>

### Parameters

<table>  <tr>    <td><a name='SkSurface_writePixels_2_src'><code><strong>src</strong></code></a></td>
    <td>storage for pixels to copy to <a href='#Surface'>Surface</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_writePixels_2_dstX'><code><strong>dstX</strong></code></a></td>
    <td>x</td>
  </tr>
  <tr>    <td><a name='SkSurface_writePixels_2_dstY'><code><strong>dstY</strong></code></a></td>
    <td>y</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d77790dd3bc9f678fa4f582347fb8fba"></fiddle-embed></div>

### See Also

<a href='#SkSurface_readPixels'>readPixels</a><sup><a href='#SkSurface_readPixels_2'>[2]</a></sup><sup><a href='#SkSurface_readPixels_3'>[3]</a></sup> <a href='#SkSurface_peekPixels'>peekPixels</a>

---

<a name='SkSurface_props'></a>
## props

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>
</pre>

Returns <a href='undocumented#Surface_Properties'>Surface Properties</a> for surface

### Return Value

LCD striping orientation and setting for device independent fonts

### Example

<div><fiddle-embed name="13cf9e7b2894ae6e98c1fd719040bf01">

#### Example Output

~~~~
surf.props(): kRGB_H_SkPixelGeometry
~~~~

</fiddle-embed></div>

### See Also

<a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>

---

<a name='SkSurface_prepareForExternalIO'></a>
## prepareForExternalIO

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkSurface_prepareForExternalIO'>prepareForExternalIO</a>(
</pre>

To be deprecated soon.

---

## <a name='Utility'>Utility</a>

<a name='SkSurface_flush'></a>
## flush

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkSurface_flush'>flush</a>(
</pre>

Issues pending <a href='#Surface'>Surface</a> commands to the GPU

### See Also

<a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a>

---

<a name='SkSurface_flushAndSignalSemaphores'></a>
## flushAndSignalSemaphores

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#GrSemaphoresSubmitted'>GrSemaphoresSubmitted</a> <a href='#SkSurface_flushAndSignalSemaphores'>flushAndSignalSemaphores</a>(int numSemaphores
</pre>

Issues pending <a href='#Surface'>Surface</a> commands to the GPU

### Parameters

<table>  <tr>    <td><a name='SkSurface_flushAndSignalSemaphores_numSemaphores'><code><strong>numSemaphores</strong></code></a></td>
    <td>size of <a href='#SkSurface_flushAndSignalSemaphores_signalSemaphores'>signalSemaphores</a> array</td>
  </tr>
  <tr>    <td><a name='SkSurface_flushAndSignalSemaphores_signalSemaphores'><code><strong>signalSemaphores</strong></code></a></td>
    <td>array of semaphore containers</td>
  </tr>
</table>

### Return Value

one of

### See Also

<a href='#SkSurface_wait'>wait</a> <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a>

---

<a name='SkSurface_wait'></a>
## wait

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkSurface_wait'>wait</a>(int numSemaphores
</pre>

Inserts a list of GPU semaphores that the current GPU

### Parameters

<table>  <tr>    <td><a name='SkSurface_wait_numSemaphores'><code><strong>numSemaphores</strong></code></a></td>
    <td>size of <a href='#SkSurface_wait_waitSemaphores'>waitSemaphores</a> array</td>
  </tr>
  <tr>    <td><a name='SkSurface_wait_waitSemaphores'><code><strong>waitSemaphores</strong></code></a></td>
    <td>array of semaphore containers</td>
  </tr>
</table>

### Return Value

true if GPU is waiting on semaphores

### See Also

<a href='#SkSurface_flushAndSignalSemaphores'>flushAndSignalSemaphores</a> <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a>

---

<a name='SkSurface_characterize'></a>
## characterize

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkSurface_characterize'>characterize</a>(<a href='undocumented#SkSurfaceCharacterization'>SkSurfaceCharacterization</a>
</pre>

Initializes <a href='undocumented#Surface_Characterization'>Surface Characterization</a> that can be used to perform GPU back

### Parameters

<table>  <tr>    <td><a name='SkSurface_characterize_characterization'><code><strong>characterization</strong></code></a></td>
    <td>properties for parallel drawing</td>
  </tr>
</table>

### Return Value

true if supported

### Example

<div><fiddle-embed name="6de6f3ef699a72ff26da1b26b23a3316" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkSurface_draw'>draw</a><sup><a href='#SkSurface_draw_2'>[2]</a></sup>(

---

<a name='SkSurface_draw_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkSurface_draw'>draw</a>(<a href='undocumented#SkDeferredDisplayList'>SkDeferredDisplayList</a>
</pre>

Draws deferred display list created using <a href='undocumented#SkDeferredDisplayListRecorder'>SkDeferredDisplayListRecorder</a>

### Parameters

<table>  <tr>    <td><a name='SkSurface_draw_2_deferredDisplayList'><code><strong>deferredDisplayList</strong></code></a></td>
    <td>drawing commands</td>
  </tr>
</table>

### Return Value

false if <a href='#SkSurface_draw_2_deferredDisplayList'>deferredDisplayList</a> is not compatible

### Example

<div><fiddle-embed name="46d9bacf593deaaeabd74ff42f2571a0" gpu="true" cpu="true"></fiddle-embed></div>

### See Also

<a href='#SkSurface_characterize'>characterize</a>(

---

