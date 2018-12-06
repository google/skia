SkSurface Reference
===


<a name='SkSurface'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkSurface_Reference#SkSurface'>SkSurface</a> : public <a href='undocumented#SkRefCnt'>SkRefCnt</a> {

    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRasterDirect'>MakeRasterDirect</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo, void* pixels,
                                             size_t rowBytes,
                                             const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRasterDirectReleaseProc'>MakeRasterDirectReleaseProc</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo, void* pixels,
                                    size_t rowBytes,
                                    void (*releaseProc)(void* pixels, void* context),
                                    void* context, const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRaster'>MakeRaster</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo, size_t rowBytes,
                                       const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRaster'>MakeRaster</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo,
                                       const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* props = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRasterN32Premul'>MakeRasterN32Premul</a>(int width, int height,
                                                const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeFromBackendTexture'>MakeFromBackendTexture</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                                   const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture,
                                                   <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> origin, int sampleCnt,
                                                   <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> colorType,
                                                   <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> colorSpace,
                                                   const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                                const <a href='undocumented#GrBackendRenderTarget'>GrBackendRenderTarget</a>& backendRenderTarget,
                                                <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> origin,
                                                <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> colorType,
                                                <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> colorSpace,
                                                const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget'>MakeFromBackendTextureAsRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                                            const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture,
                                                            <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> origin,
                                                            int sampleCnt,
                                                            <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> colorType,
                                                            <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> colorSpace,
                                                            const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* context, <a href='undocumented#SkBudgeted'>SkBudgeted</a> budgeted,
                                             const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo,
                                             int sampleCount, <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> surfaceOrigin,
                                             const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps,
                                             bool shouldCreateWithMips = false);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* context, <a href='undocumented#SkBudgeted'>SkBudgeted</a> budgeted,
                                             const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo, int sampleCount,
                                             const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* props);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* context, <a href='undocumented#SkBudgeted'>SkBudgeted</a> budgeted,
                                             const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                             const <a href='undocumented#SkSurfaceCharacterization'>SkSurfaceCharacterization</a>& characterization,
                                             <a href='undocumented#SkBudgeted'>SkBudgeted</a> budgeted);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeNull'>MakeNull</a>(int width, int height);
    int <a href='#SkSurface_width'>width()</a> const;
    int <a href='#SkSurface_height'>height()</a> const;
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

    <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='#SkSurface_getBackendTexture'>getBackendTexture</a>(<a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> backendHandleAccess);
    <a href='undocumented#GrBackendRenderTarget'>GrBackendRenderTarget</a> <a href='#SkSurface_getBackendRenderTarget'>getBackendRenderTarget</a>(<a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> backendHandleAccess);
    <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* <a href='#SkSurface_getCanvas'>getCanvas</a>();
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_makeSurface'>makeSurface</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo);
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a>();
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a>(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& bounds);
    void <a href='#SkSurface_draw'>draw</a>(<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* <a href='SkCanvas_Reference#Canvas'>canvas</a>, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>);
    bool <a href='#SkSurface_peekPixels'>peekPixels</a>(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>* <a href='SkPixmap_Reference#Pixmap'>pixmap</a>);
    bool <a href='#SkSurface_readPixels'>readPixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& dst, int srcX, int srcY);
    bool <a href='#SkSurface_readPixels'>readPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& dstInfo, void* dstPixels, size_t dstRowBytes,
                    int srcX, int srcY);
    bool <a href='#SkSurface_readPixels'>readPixels</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& dst, int srcX, int srcY);
    void <a href='#SkSurface_writePixels'>writePixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& src, int dstX, int dstY);
    void <a href='#SkSurface_writePixels'>writePixels</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& src, int dstX, int dstY);
    const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>& <a href='#SkSurface_props'>props()</a> const;
    void <a href='#SkSurface_flush'>flush()</a>;
    <a href='undocumented#GrSemaphoresSubmitted'>GrSemaphoresSubmitted</a> <a href='#SkSurface_flushAndSignalSemaphores'>flushAndSignalSemaphores</a>(int numSemaphores,
                                                   <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a> signalSemaphores[]);
    bool <a href='#SkSurface_wait'>wait</a>(int numSemaphores, const <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a>* waitSemaphores);
    bool <a href='#SkSurface_characterize'>characterize</a>(<a href='undocumented#SkSurfaceCharacterization'>SkSurfaceCharacterization</a>* characterization) const;
    bool <a href='#SkSurface_draw'>draw</a>(<a href='undocumented#SkDeferredDisplayList'>SkDeferredDisplayList</a>* deferredDisplayList);
};

</pre>

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> is responsible for managing the pixels that a <a href='SkCanvas_Reference#Canvas'>canvas</a> draws into. The pixels can be
allocated either in CPU memory, if a  <a href='undocumented#Raster_Surface'>raster surface</a>; or on the GPU, for a <a href='undocumented#GrRenderTarget'>GrRenderTarget</a> <a href='SkSurface_Reference#Surface'>surface</a>.
<a href='SkSurface_Reference#SkSurface'>SkSurface</a> takes care of allocating a <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> that will draw into the <a href='SkSurface_Reference#Surface'>surface</a>. Call
<a href='SkSurface_Reference#Surface'>surface</a>-><a href='#SkSurface_getCanvas'>getCanvas</a>() to use that <a href='SkCanvas_Reference#Canvas'>canvas</a>. The caller should not delete the returned <a href='SkCanvas_Reference#Canvas'>canvas</a>;
it is owned by <a href='SkSurface_Reference#Surface'>surface</a>.

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> always has non-zero dimensions. If there is a request for a new <a href='SkSurface_Reference#Surface'>surface</a>, and either
of the requested dimensions are zero, then nullptr will be returned.

<a name='SkSurface_MakeRasterDirect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeRasterDirect'>MakeRasterDirect</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo, void* pixels, size_t rowBytes,
                                         const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps = nullptr)
</pre>

Allocates raster <a href='SkSurface_Reference#SkSurface'>SkSurface</a>. <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> returned by <a href='SkSurface_Reference#SkSurface'>SkSurface</a> draws directly into <a href='#SkSurface_MakeRasterDirect_pixels'>pixels</a>.

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> is returned if all parameters are valid.
Valid parameters include:
info dimensions are greater than zero;
info contains <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> and <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> supported by  <a href='undocumented#Raster_Surface'>raster surface</a>;
<a href='#SkSurface_MakeRasterDirect_pixels'>pixels</a> is not nullptr;
<a href='#SkSurface_MakeRasterDirect_rowBytes'>rowBytes</a> is large enough to contain info width <a href='#SkSurface_MakeRasterDirect_pixels'>pixels</a> of <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.

<a href='undocumented#Pixel'>Pixel</a> buffer <a href='undocumented#Size'>size</a> should be info height times computed <a href='#SkSurface_MakeRasterDirect_rowBytes'>rowBytes</a>.
Pixels are not initialized.
To access <a href='#SkSurface_MakeRasterDirect_pixels'>pixels</a> after drawing, call <a href='#SkSurface_flush'>flush()</a> or <a href='#SkSurface_peekPixels'>peekPixels</a>().

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRasterDirect_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>,</td>
  </tr>
</table>

of  <a href='undocumented#Raster_Surface'>raster surface</a>; width and height must be greater than zero

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRasterDirect_pixels'><code><strong>pixels</strong></code></a></td>
    <td>pointer to destination <a href='#SkSurface_MakeRasterDirect_pixels'>pixels</a> buffer</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirect_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>interval from one <a href='SkSurface_Reference#SkSurface'>SkSurface</a> row to the next</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirect_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for <a href='undocumented#Device'>device</a> independent fonts;</td>
  </tr>
</table>

may be nullptr

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> if all parameters are valid; otherwise, nullptr

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

<a href='#SkSurface_MakeRasterDirectReleaseProc'>MakeRasterDirectReleaseProc</a> <a href='#SkSurface_MakeRaster'>MakeRaster</a> <a href='#SkSurface_MakeRasterN32Premul'>MakeRasterN32Premul</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_MakeRasterDirect'>MakeRasterDirect</a>

<a name='SkSurface_MakeRasterDirectReleaseProc'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeRasterDirectReleaseProc'>MakeRasterDirectReleaseProc</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo, void* pixels,
                                           size_t rowBytes, void (*releaseProc) (void* pixels,
                                           void* context) , void* context,
                                           const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps = nullptr)
</pre>

Allocates raster <a href='SkSurface_Reference#SkSurface'>SkSurface</a>. <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> returned by <a href='SkSurface_Reference#SkSurface'>SkSurface</a> draws directly into <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>pixels</a>.
<a href='#SkSurface_MakeRasterDirectReleaseProc_releaseProc'>releaseProc</a> is called with <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>pixels</a> and <a href='#SkSurface_MakeRasterDirectReleaseProc_context'>context</a> when <a href='SkSurface_Reference#SkSurface'>SkSurface</a> is deleted.

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> is returned if all parameters are valid.
Valid parameters include:
info dimensions are greater than zero;
info contains <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> and <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> supported by  <a href='undocumented#Raster_Surface'>raster surface</a>;
<a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>pixels</a> is not nullptr;
<a href='#SkSurface_MakeRasterDirectReleaseProc_rowBytes'>rowBytes</a> is large enough to contain info width <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>pixels</a> of <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.

<a href='undocumented#Pixel'>Pixel</a> buffer <a href='undocumented#Size'>size</a> should be info height times computed <a href='#SkSurface_MakeRasterDirectReleaseProc_rowBytes'>rowBytes</a>.
Pixels are not initialized.
To access <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>pixels</a> after drawing, call <a href='#SkSurface_flush'>flush()</a> or <a href='#SkSurface_peekPixels'>peekPixels</a>().

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRasterDirectReleaseProc_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>,</td>
  </tr>
</table>

of  <a href='undocumented#Raster_Surface'>raster surface</a>; width and height must be greater than zero

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRasterDirectReleaseProc_pixels'><code><strong>pixels</strong></code></a></td>
    <td>pointer to destination <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>pixels</a> buffer</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirectReleaseProc_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>interval from one <a href='SkSurface_Reference#SkSurface'>SkSurface</a> row to the next</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirectReleaseProc_releaseProc'><code><strong>releaseProc</strong></code></a></td>
    <td>called when <a href='SkSurface_Reference#SkSurface'>SkSurface</a> is deleted; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirectReleaseProc_context'><code><strong>context</strong></code></a></td>
    <td>passed to <a href='#SkSurface_MakeRasterDirectReleaseProc_releaseProc'>releaseProc</a>; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirectReleaseProc_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for <a href='undocumented#Device'>device</a> independent fonts;</td>
  </tr>
</table>

may be nullptr

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> if all parameters are valid; otherwise, nullptr

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

<a href='#SkSurface_MakeRasterDirect'>MakeRasterDirect</a> <a href='#SkSurface_MakeRasterN32Premul'>MakeRasterN32Premul</a> <a href='#SkSurface_MakeRaster'>MakeRaster</a>

<a name='SkSurface_MakeRaster'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeRaster'>MakeRaster</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo, size_t rowBytes,
                                   const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps)
</pre>

Allocates raster <a href='SkSurface_Reference#SkSurface'>SkSurface</a>. <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> returned by <a href='SkSurface_Reference#SkSurface'>SkSurface</a> draws directly into pixels.
Allocates and zeroes <a href='undocumented#Pixel'>pixel</a> memory. <a href='undocumented#Pixel'>Pixel</a> memory <a href='undocumented#Size'>size</a> is <a href='#SkSurface_MakeRaster_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_height'>height()</a> times
<a href='#SkSurface_MakeRaster_rowBytes'>rowBytes</a>, or times <a href='#SkSurface_MakeRaster_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>() if <a href='#SkSurface_MakeRaster_rowBytes'>rowBytes</a> is zero.
<a href='undocumented#Pixel'>Pixel</a> memory is deleted when <a href='SkSurface_Reference#SkSurface'>SkSurface</a> is deleted.

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> is returned if all parameters are valid.
Valid parameters include:
info dimensions are greater than zero;
info contains <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> and <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> supported by  <a href='undocumented#Raster_Surface'>raster surface</a>;
<a href='#SkSurface_MakeRaster_rowBytes'>rowBytes</a> is large enough to contain info width pixels of <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, or is zero.

If <a href='#SkSurface_MakeRaster_rowBytes'>rowBytes</a> is not zero, subsequent images returned by <a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a>()
have the same <a href='#SkSurface_MakeRaster_rowBytes'>rowBytes</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRaster_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>,</td>
  </tr>
</table>

of  <a href='undocumented#Raster_Surface'>raster surface</a>; width and height must be greater than zero

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRaster_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>interval from one <a href='SkSurface_Reference#SkSurface'>SkSurface</a> row to the next; may be zero</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRaster_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for <a href='undocumented#Device'>device</a> independent fonts;</td>
  </tr>
</table>

may be nullptr

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> if all parameters are valid; otherwise, nullptr

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

<a name='SkSurface_MakeRaster_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeRaster'>MakeRaster</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo,
                                   const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* props = nullptr)
</pre>

Allocates raster <a href='SkSurface_Reference#SkSurface'>SkSurface</a>. <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> returned by <a href='SkSurface_Reference#SkSurface'>SkSurface</a> draws directly into pixels.
Allocates and zeroes <a href='undocumented#Pixel'>pixel</a> memory. <a href='undocumented#Pixel'>Pixel</a> memory <a href='undocumented#Size'>size</a> is <a href='#SkSurface_MakeRaster_2_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_height'>height()</a> times
<a href='#SkSurface_MakeRaster_2_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>().
<a href='undocumented#Pixel'>Pixel</a> memory is deleted when <a href='SkSurface_Reference#SkSurface'>SkSurface</a> is deleted.

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> is returned if all parameters are valid.
Valid parameters include:
info dimensions are greater than zero;
info contains <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> and <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> supported by  <a href='undocumented#Raster_Surface'>raster surface</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRaster_2_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>,</td>
  </tr>
</table>

of  <a href='undocumented#Raster_Surface'>raster surface</a>; width and height must be greater than zero

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRaster_2_props'><code><strong>props</strong></code></a></td>
    <td>LCD striping orientation and setting for <a href='undocumented#Device'>device</a> independent fonts;</td>
  </tr>
</table>

may be nullptr

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="c6197d204ef9e4ccfb583242651fb2a7"></fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeRasterDirect'>MakeRasterDirect</a> <a href='#SkSurface_MakeRasterN32Premul'>MakeRasterN32Premul</a> <a href='#SkSurface_MakeRasterDirectReleaseProc'>MakeRasterDirectReleaseProc</a>

<a name='SkSurface_MakeRasterN32Premul'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeRasterN32Premul'>MakeRasterN32Premul</a>(int width, int height,
                                            const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps = nullptr)
</pre>

Allocates raster <a href='SkSurface_Reference#SkSurface'>SkSurface</a>. <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> returned by <a href='SkSurface_Reference#SkSurface'>SkSurface</a> draws directly into pixels.
Allocates and zeroes <a href='undocumented#Pixel'>pixel</a> memory. <a href='undocumented#Pixel'>Pixel</a> memory <a href='undocumented#Size'>size</a> is <a href='#SkSurface_MakeRasterN32Premul_height'>height</a> times <a href='#SkSurface_MakeRasterN32Premul_width'>width</a> times
four. <a href='undocumented#Pixel'>Pixel</a> memory is deleted when <a href='SkSurface_Reference#SkSurface'>SkSurface</a> is deleted.

Internally, sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> to <a href='#SkSurface_MakeRasterN32Premul_width'>width</a>, <a href='#SkSurface_MakeRasterN32Premul_height'>height</a>, native  <a href='SkImageInfo_Reference#Color_Type'>color type</a>, and
<a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>.

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> is returned if <a href='#SkSurface_MakeRasterN32Premul_width'>width</a> and <a href='#SkSurface_MakeRasterN32Premul_height'>height</a> are greater than zero.

Use to create <a href='SkSurface_Reference#SkSurface'>SkSurface</a> that matches <a href='SkColor_Reference#SkPMColor'>SkPMColor</a>, the native <a href='undocumented#Pixel'>pixel</a> arrangement on
the platform. <a href='SkSurface_Reference#SkSurface'>SkSurface</a> drawn to output <a href='undocumented#Device'>device</a> skips converting its <a href='undocumented#Pixel'>pixel</a> format.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRasterN32Premul_width'><code><strong>width</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> column count; must be greater than zero</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterN32Premul_height'><code><strong>height</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> row count; must be greater than zero</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterN32Premul_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for <a href='undocumented#Device'>device</a> independent</td>
  </tr>
</table>

fonts; may be nullptr

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> if all parameters are valid; otherwise, nullptr

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

<a name='SkSurface_MakeFromBackendTexture'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeFromBackendTexture'>MakeFromBackendTexture</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                               const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture,
                                               <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> origin, int sampleCnt,
                                               <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> colorType,
                                               <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; colorSpace,
                                               const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps)
</pre>

Wraps a GPU-backed <a href='undocumented#Texture'>texture</a> into <a href='SkSurface_Reference#SkSurface'>SkSurface</a>. Caller must ensure the <a href='undocumented#Texture'>texture</a> is
valid for the lifetime of returned <a href='SkSurface_Reference#SkSurface'>SkSurface</a>. If <a href='#SkSurface_MakeFromBackendTexture_sampleCnt'>sampleCnt</a> greater than zero,
creates an intermediate MSAA <a href='SkSurface_Reference#SkSurface'>SkSurface</a> which is used for drawing <a href='#SkSurface_MakeFromBackendTexture_backendTexture'>backendTexture</a>.

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> is returned if all parameters are valid. <a href='#SkSurface_MakeFromBackendTexture_backendTexture'>backendTexture</a> is valid if
its <a href='undocumented#Pixel'>pixel</a> configuration agrees with <a href='#SkSurface_MakeFromBackendTexture_colorSpace'>colorSpace</a> and <a href='#SkSurface_MakeFromBackendTexture_context'>context</a>; for instance, if
<a href='#SkSurface_MakeFromBackendTexture_backendTexture'>backendTexture</a> has an sRGB configuration, then <a href='#SkSurface_MakeFromBackendTexture_context'>context</a> must support sRGB,
and <a href='#SkSurface_MakeFromBackendTexture_colorSpace'>colorSpace</a> must be present. Further, <a href='#SkSurface_MakeFromBackendTexture_backendTexture'>backendTexture</a> width and height must
not exceed <a href='#SkSurface_MakeFromBackendTexture_context'>context</a> capabilities, and the <a href='#SkSurface_MakeFromBackendTexture_context'>context</a> must be able to support
back-end textures.

If SK_SUPPORT_GPU is defined as zero, has no effect and returns nullptr.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td><a href='undocumented#Texture'>texture</a> residing on GPU</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_origin'><code><strong>origin</strong></code></a></td>
    <td>one of: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft_GrSurfaceOrigin</a>, <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_sampleCnt'><code><strong>sampleCnt</strong></code></a></td>
    <td>samples per <a href='undocumented#Pixel'>pixel</a>, or 0 to disable full scene <a href='SkPaint_Reference#Anti_Alias'>anti-aliasing</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_colorType'><code><strong>colorType</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>,
<a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>, <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>,
<a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td>range of colors; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for <a href='undocumented#Device'>device</a> independent</td>
  </tr>
</table>

fonts; may be nullptr

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="d3aec071998f871809f515e58abb1b0e" gpu="true" cpu="true"></fiddle-embed></div>

### See Also

<a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a> <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>

<a name='SkSurface_MakeFromBackendRenderTarget'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                                   const <a href='undocumented#GrBackendRenderTarget'>GrBackendRenderTarget</a>& backendRenderTarget,
                                                   <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> origin, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> colorType,
                                                   <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; colorSpace,
                                                   const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps)
</pre>

Wraps a GPU-backed buffer into <a href='SkSurface_Reference#SkSurface'>SkSurface</a>. Caller must ensure <a href='#SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'>backendRenderTarget</a>
is valid for the lifetime of returned <a href='SkSurface_Reference#SkSurface'>SkSurface</a>.

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> is returned if all parameters are valid. <a href='#SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'>backendRenderTarget</a> is valid if
its <a href='undocumented#Pixel'>pixel</a> configuration agrees with <a href='#SkSurface_MakeFromBackendRenderTarget_colorSpace'>colorSpace</a> and <a href='#SkSurface_MakeFromBackendRenderTarget_context'>context</a>; for instance, if
<a href='#SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'>backendRenderTarget</a> has an sRGB configuration, then <a href='#SkSurface_MakeFromBackendRenderTarget_context'>context</a> must support sRGB,
and <a href='#SkSurface_MakeFromBackendRenderTarget_colorSpace'>colorSpace</a> must be present. Further, <a href='#SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'>backendRenderTarget</a> width and height must
not exceed <a href='#SkSurface_MakeFromBackendRenderTarget_context'>context</a> capabilities, and the <a href='#SkSurface_MakeFromBackendRenderTarget_context'>context</a> must be able to support
back-end render targets.

If SK_SUPPORT_GPU is defined as zero, has no effect and returns nullptr.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeFromBackendRenderTarget_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'><code><strong>backendRenderTarget</strong></code></a></td>
    <td>GPU intermediate memory buffer</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendRenderTarget_origin'><code><strong>origin</strong></code></a></td>
    <td>one of: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft_GrSurfaceOrigin</a>, <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendRenderTarget_colorType'><code><strong>colorType</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>,
<a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>,
<a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>, <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>,
<a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeFromBackendRenderTarget_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td>range of colors</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendRenderTarget_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for <a href='undocumented#Device'>device</a> independent</td>
  </tr>
</table>

fonts; may be nullptr

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> if all parameters are valid; otherwise, nullptr

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

<a href='#SkSurface_MakeFromBackendTexture'>MakeFromBackendTexture</a> <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>

<a name='SkSurface_MakeFromBackendTextureAsRenderTarget'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget'>MakeFromBackendTextureAsRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                            const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture,
                                            <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> origin, int sampleCnt,
                                            <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> colorType, <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; colorSpace,
                                            const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps)
</pre>

Wraps a GPU-backed <a href='undocumented#Texture'>texture</a> into <a href='SkSurface_Reference#SkSurface'>SkSurface</a>. Caller must ensure <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>backendTexture</a> is
valid for the lifetime of returned <a href='SkSurface_Reference#SkSurface'>SkSurface</a>. If <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_sampleCnt'>sampleCnt</a> greater than zero,
creates an intermediate MSAA <a href='SkSurface_Reference#SkSurface'>SkSurface</a> which is used for drawing <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>backendTexture</a>.

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> is returned if all parameters are valid. <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>backendTexture</a> is valid if
its <a href='undocumented#Pixel'>pixel</a> configuration agrees with <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_colorSpace'>colorSpace</a> and <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_context'>context</a>; for instance, if
<a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>backendTexture</a> has an sRGB configuration, then <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_context'>context</a> must support sRGB,
and <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_colorSpace'>colorSpace</a> must be present. Further, <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>backendTexture</a> width and height must
not exceed <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_context'>context</a> capabilities.

Returned <a href='SkSurface_Reference#SkSurface'>SkSurface</a> is available only for drawing into, and cannot generate an
<a href='SkImage_Reference#SkImage'>SkImage</a>.

If SK_SUPPORT_GPU is defined as zero, has no effect and returns nullptr.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td><a href='undocumented#Texture'>texture</a> residing on GPU</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_origin'><code><strong>origin</strong></code></a></td>
    <td>one of: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft_GrSurfaceOrigin</a>, <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_sampleCnt'><code><strong>sampleCnt</strong></code></a></td>
    <td>samples per <a href='undocumented#Pixel'>pixel</a>, or 0 to disable full scene <a href='SkPaint_Reference#Anti_Alias'>anti-aliasing</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_colorType'><code><strong>colorType</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>,
<a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>, <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>,
<a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td>range of colors; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for <a href='undocumented#Device'>device</a> independent</td>
  </tr>
</table>

fonts; may be nullptr

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="5e87093b9cbe95124ae14cbe77091eb7" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a> <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>

<a name='SkSurface_MakeRenderTarget'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* context, <a href='undocumented#SkBudgeted'>SkBudgeted</a> budgeted,
                                         const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo, int sampleCount,
                                         <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> surfaceOrigin,
                                         const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* surfaceProps,
                                         bool shouldCreateWithMips = false)
</pre>

Returns <a href='SkSurface_Reference#SkSurface'>SkSurface</a> on GPU indicated by <a href='#SkSurface_MakeRenderTarget_context'>context</a>. Allocates memory for
pixels, based on the width, height, and <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> in <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>.  <a href='#SkSurface_MakeRenderTarget_budgeted'>budgeted</a>
selects whether allocation for pixels is tracked by <a href='#SkSurface_MakeRenderTarget_context'>context</a>. <a href='#SkSurface_MakeRenderTarget_imageInfo'>imageInfo</a>
describes the <a href='undocumented#Pixel'>pixel</a> format in <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, and transparency in
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, and <a href='SkColor_Reference#Color'>color</a> matching in <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

<a href='#SkSurface_MakeRenderTarget_sampleCount'>sampleCount</a> requests the number of samples per <a href='undocumented#Pixel'>pixel</a>.
Pass zero to disable  <a href='undocumented#Multi_Sample_Anti_Aliasing'>multi-sample anti-aliasing</a>.  The request is rounded
up to the next supported count, or rounded down if it is larger than the
maximum supported count.

<a href='#SkSurface_MakeRenderTarget_surfaceOrigin'>surfaceOrigin</a> pins either the top-left or the bottom-left corner to the origin.

<a href='#SkSurface_MakeRenderTarget_shouldCreateWithMips'>shouldCreateWithMips</a> hints that <a href='SkImage_Reference#SkImage'>SkImage</a> returned by <a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a>() is  <a href='undocumented#Mip_Map'>mip map</a>.

If SK_SUPPORT_GPU is defined as zero, has no effect and returns nullptr.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRenderTarget_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_budgeted'><code><strong>budgeted</strong></code></a></td>
    <td>one of: <a href='undocumented#SkBudgeted'>SkBudgeted</a>::<a href='#SkBudgeted_kNo'>kNo</a>, <a href='undocumented#SkBudgeted'>SkBudgeted</a>::<a href='#SkBudgeted_kYes'>kYes</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>;</td>
  </tr>
</table>

width, or height, or both, may be zero

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRenderTarget_sampleCount'><code><strong>sampleCount</strong></code></a></td>
    <td>samples per <a href='undocumented#Pixel'>pixel</a>, or 0 to disable full scene <a href='SkPaint_Reference#Anti_Alias'>anti-aliasing</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_surfaceOrigin'><code><strong>surfaceOrigin</strong></code></a></td>
    <td>one of: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft_GrSurfaceOrigin</a>, <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for <a href='undocumented#Device'>device</a> independent</td>
  </tr>
</table>

fonts; may be nullptr

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRenderTarget_shouldCreateWithMips'><code><strong>shouldCreateWithMips</strong></code></a></td>
    <td>hint that <a href='SkSurface_Reference#SkSurface'>SkSurface</a> will host  <a href='undocumented#Mip_Map'>mip map</a> images</td>
  </tr>
</table>

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="67b6609471a3f1ed0f4b1657004cdecb" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget'>MakeFromBackendTextureAsRenderTarget</a>

<a name='SkSurface_MakeRenderTarget_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* context, <a href='undocumented#SkBudgeted'>SkBudgeted</a> budgeted,
                                         const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo, int sampleCount,
                                         const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* props)
</pre>

Returns <a href='SkSurface_Reference#SkSurface'>SkSurface</a> on GPU indicated by <a href='#SkSurface_MakeRenderTarget_2_context'>context</a>. Allocates memory for
pixels, based on the width, height, and <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> in <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>.  <a href='#SkSurface_MakeRenderTarget_2_budgeted'>budgeted</a>
selects whether allocation for pixels is tracked by <a href='#SkSurface_MakeRenderTarget_2_context'>context</a>. <a href='#SkSurface_MakeRenderTarget_2_imageInfo'>imageInfo</a>
describes the <a href='undocumented#Pixel'>pixel</a> format in <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, and transparency in
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, and <a href='SkColor_Reference#Color'>color</a> matching in <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

<a href='#SkSurface_MakeRenderTarget_2_sampleCount'>sampleCount</a> requests the number of samples per <a href='undocumented#Pixel'>pixel</a>.
Pass zero to disable  <a href='undocumented#Multi_Sample_Anti_Aliasing'>multi-sample anti-aliasing</a>.  The request is rounded
up to the next supported count, or rounded down if it is larger than the
maximum supported count.

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> bottom-left corner is pinned to the origin.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRenderTarget_2_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_2_budgeted'><code><strong>budgeted</strong></code></a></td>
    <td>one of: <a href='undocumented#SkBudgeted'>SkBudgeted</a>::<a href='#SkBudgeted_kNo'>kNo</a>, <a href='undocumented#SkBudgeted'>SkBudgeted</a>::<a href='#SkBudgeted_kYes'>kYes</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_2_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>,</td>
  </tr>
</table>

of  <a href='undocumented#Raster_Surface'>raster surface</a>; width, or height, or both, may be zero

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRenderTarget_2_sampleCount'><code><strong>sampleCount</strong></code></a></td>
    <td>samples per <a href='undocumented#Pixel'>pixel</a>, or 0 to disable  <a href='undocumented#Multi_Sample_Anti_Aliasing'>multi-sample anti-aliasing</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_2_props'><code><strong>props</strong></code></a></td>
    <td>LCD striping orientation and setting for <a href='undocumented#Device'>device</a> independent</td>
  </tr>
</table>

fonts; may be nullptr

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="640321e8ecfb3f9329f3bc6e1f02485f" gpu="true" cpu="true"><div>LCD <a href='undocumented#Text'>text</a> takes advantage of raster striping to improve resolution. Only one of
the four combinations is correct, depending on whether monitor LCD striping is
horizontal or vertical, and whether the order of the stripes is red blue green
or red green blue.
</div></fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget'>MakeFromBackendTextureAsRenderTarget</a>

<a name='SkSurface_MakeRenderTarget_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* context, <a href='undocumented#SkBudgeted'>SkBudgeted</a> budgeted,
                                         const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo)
</pre>

Returns <a href='SkSurface_Reference#SkSurface'>SkSurface</a> on GPU indicated by <a href='#SkSurface_MakeRenderTarget_3_context'>context</a>. Allocates memory for
pixels, based on the width, height, and <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> in <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>.  <a href='#SkSurface_MakeRenderTarget_3_budgeted'>budgeted</a>
selects whether allocation for pixels is tracked by <a href='#SkSurface_MakeRenderTarget_3_context'>context</a>. <a href='#SkSurface_MakeRenderTarget_3_imageInfo'>imageInfo</a>
describes the <a href='undocumented#Pixel'>pixel</a> format in <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, and transparency in
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, and <a href='SkColor_Reference#Color'>color</a> matching in <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> bottom-left corner is pinned to the origin.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRenderTarget_3_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_3_budgeted'><code><strong>budgeted</strong></code></a></td>
    <td>one of: <a href='undocumented#SkBudgeted'>SkBudgeted</a>::<a href='#SkBudgeted_kNo'>kNo</a>, <a href='undocumented#SkBudgeted'>SkBudgeted</a>::<a href='#SkBudgeted_kYes'>kYes</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_3_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>,</td>
  </tr>
</table>

of  <a href='undocumented#Raster_Surface'>raster surface</a>; width, or height, or both, may be zero

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> if all parameters are valid; otherwise, nullptr

### Example

<div><fiddle-embed name="5c7629c15e9ac93f098335e72560fa2e" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget'>MakeFromBackendTextureAsRenderTarget</a>

<a name='SkSurface_MakeRenderTarget_4'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                         const <a href='undocumented#SkSurfaceCharacterization'>SkSurfaceCharacterization</a>& characterization,
                                         <a href='undocumented#SkBudgeted'>SkBudgeted</a> budgeted)
</pre>

Returns <a href='SkSurface_Reference#SkSurface'>SkSurface</a> on GPU indicated by <a href='#SkSurface_MakeRenderTarget_4_context'>context</a> that is compatible with the provided
<a href='#SkSurface_MakeRenderTarget_4_characterization'>characterization</a>. <a href='#SkSurface_MakeRenderTarget_4_budgeted'>budgeted</a> selects whether allocation for pixels is tracked by <a href='#SkSurface_MakeRenderTarget_4_context'>context</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRenderTarget_4_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_4_characterization'><code><strong>characterization</strong></code></a></td>
    <td>description of the desired <a href='SkSurface_Reference#SkSurface'>SkSurface</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_4_budgeted'><code><strong>budgeted</strong></code></a></td>
    <td>one of: <a href='undocumented#SkBudgeted'>SkBudgeted</a>::<a href='#SkBudgeted_kNo'>kNo</a>, <a href='undocumented#SkBudgeted'>SkBudgeted</a>::<a href='#SkBudgeted_kYes'>kYes</a></td>
  </tr>
</table>

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> if all parameters are valid; otherwise, nullptr

### See Also

<a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget'>MakeFromBackendTextureAsRenderTarget</a>

<a name='SkSurface_MakeNull'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_MakeNull'>MakeNull</a>(int width, int height)
</pre>

Returns <a href='SkSurface_Reference#SkSurface'>SkSurface</a> without backing pixels. Drawing to <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> returned from <a href='SkSurface_Reference#SkSurface'>SkSurface</a>
has no effect. Calling <a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a>() on returned <a href='SkSurface_Reference#SkSurface'>SkSurface</a> returns nullptr.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeNull_width'><code><strong>width</strong></code></a></td>
    <td>one or greater</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeNull_height'><code><strong>height</strong></code></a></td>
    <td>one or greater</td>
  </tr>
</table>

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> if <a href='#SkSurface_MakeNull_width'>width</a> and <a href='#SkSurface_MakeNull_height'>height</a> are positive; otherwise, nullptr

### Example

<div><fiddle-embed name="99a54b814ccab7d2b1143c88581649ff">

#### Example Output

~~~~
SkSurface::MakeNull(0, 0) == nullptr
surf->makeImageSnapshot() == nullptr
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeRaster'>MakeRaster</a> <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>

<a name='Property'></a>

<a name='SkSurface_width'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkSurface_width'>width()</a>const
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> count in each row; may be zero or greater.

### Return Value

number of <a href='undocumented#Pixel'>pixel</a> columns

### Example

<div><fiddle-embed name="df066b56dd97c7c589fd2bb6a2539de8">

#### Example Output

~~~~
surface width=37  canvas width=37
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkSurface_height'>height()</a>

<a name='SkSurface_height'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkSurface_height'>height()</a>const
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> row count; may be zero or greater.

### Return Value

number of <a href='undocumented#Pixel'>pixel</a> rows

### Example

<div><fiddle-embed name="20571cc23e3146deaa09046b64cc0aef">

#### Example Output

~~~~
surface height=1000  canvas height=1000
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkSurface_width'>width()</a>

<a name='SkSurface_generationID'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkSurface_generationID'>generationID</a>()
</pre>

Returns unique value identifying the content of <a href='SkSurface_Reference#SkSurface'>SkSurface</a>. Returned value changes
each time the content changes. Content is changed by drawing, or by calling
<a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a>().

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

<a name='SkSurface_ContentChangeMode'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkSurface_ContentChangeMode'>ContentChangeMode</a> {
        <a href='#SkSurface_kDiscard_ContentChangeMode'>kDiscard_ContentChangeMode</a>,
        <a href='#SkSurface_kRetain_ContentChangeMode'>kRetain_ContentChangeMode</a>,
    };
</pre>

<a href='#SkSurface_ContentChangeMode'>ContentChangeMode</a> members are parameters to <a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSurface_kDiscard_ContentChangeMode'><code>SkSurface::kDiscard_ContentChangeMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#Line # discards surface on change ##</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Pass to <a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a> to discard <a href='SkSurface_Reference#Surface'>surface</a> contents when
the <a href='SkSurface_Reference#Surface'>surface</a> is cleared or overwritten.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSurface_kRetain_ContentChangeMode'><code>SkSurface::kRetain_ContentChangeMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#Line # preserves surface on change ##</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Pass to <a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a> when to preserve <a href='SkSurface_Reference#Surface'>surface</a> contents.
If a snapshot has been generated, this copies the <a href='SkSurface_Reference#Surface'>Surface</a> contents.
</td>
  </tr>
</table>

### See Also

<a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a> <a href='#SkSurface_generationID'>generationID</a>

<a name='Miscellaneous'></a>

<a name='SkSurface_notifyContentWillChange'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a>(<a href='#SkSurface_ContentChangeMode'>ContentChangeMode</a> mode)
</pre>

Notifies that <a href='SkSurface_Reference#SkSurface'>SkSurface</a> contents will be changed by code outside of Skia.
Subsequent calls to <a href='#SkSurface_generationID'>generationID</a>() return a different value.

### Parameters

<table>  <tr>    <td><a name='SkSurface_notifyContentWillChange_mode'><code><strong>mode</strong></code></a></td>
    <td>one of: <a href='#SkSurface_kDiscard_ContentChangeMode'>kDiscard_ContentChangeMode</a>, <a href='#SkSurface_kRetain_ContentChangeMode'>kRetain_ContentChangeMode</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="be9574c4a14f891e1abb4ec2b1e51d6c"></fiddle-embed></div>

### See Also

<a href='#SkSurface_ContentChangeMode'>ContentChangeMode</a> <a href='#SkSurface_generationID'>generationID</a>

<a name='SkSurface_BackendHandleAccess'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> {
        <a href='#SkSurface_kFlushRead_BackendHandleAccess'>kFlushRead_BackendHandleAccess</a>,
        <a href='#SkSurface_kFlushWrite_BackendHandleAccess'>kFlushWrite_BackendHandleAccess</a>,
        <a href='#SkSurface_kDiscardWrite_BackendHandleAccess'>kDiscardWrite_BackendHandleAccess</a>,
    };

    static const <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> kFlushRead_TextureHandleAccess =
            <a href='#SkSurface_kFlushRead_BackendHandleAccess'>kFlushRead_BackendHandleAccess</a>;
    static const <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> kFlushWrite_TextureHandleAccess =
            <a href='#SkSurface_kFlushWrite_BackendHandleAccess'>kFlushWrite_BackendHandleAccess</a>;
    static const <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> kDiscardWrite_TextureHandleAccess =
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
Caller may read from the back-end object.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSurface_kFlushWrite_BackendHandleAccess'><code>SkSurface::kFlushWrite_BackendHandleAccess</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Caller may write to the back-end object.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSurface_kDiscardWrite_BackendHandleAccess'><code>SkSurface::kDiscardWrite_BackendHandleAccess</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Caller must overwrite the entire back-end object.
</td>
  </tr>
</table>

### See Also

<a href='#SkSurface_getBackendTexture'>getBackendTexture</a> <a href='#SkSurface_getBackendRenderTarget'>getBackendRenderTarget</a>

<a name='SkSurface_getBackendTexture'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='#SkSurface_getBackendTexture'>getBackendTexture</a>(<a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> backendHandleAccess)
</pre>

Retrieves the back-end <a href='undocumented#Texture'>texture</a>. If <a href='SkSurface_Reference#SkSurface'>SkSurface</a> has no back-end <a href='undocumented#Texture'>texture</a>, an invalid
object is returned. Call <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>::<a href='#GrBackendTexture_isValid'>isValid</a> to determine if the result
is valid.

The returned <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> should be discarded if the <a href='SkSurface_Reference#SkSurface'>SkSurface</a> is drawn to or deleted.

### Parameters

<table>  <tr>    <td><a name='SkSurface_getBackendTexture_backendHandleAccess'><code><strong>backendHandleAccess</strong></code></a></td>
    <td>one of:  <a href='#SkSurface_kFlushRead_BackendHandleAccess'>kFlushRead_BackendHandleAccess</a>,</td>
  </tr>
</table>

<a href='#SkSurface_kFlushWrite_BackendHandleAccess'>kFlushWrite_BackendHandleAccess</a>,
<a href='#SkSurface_kDiscardWrite_BackendHandleAccess'>kDiscardWrite_BackendHandleAccess</a>

### Return Value

<a href='undocumented#GPU_Texture'>GPU texture</a> reference; invalid on failure

### See Also

<a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_getBackendRenderTarget'>getBackendRenderTarget</a>

<a name='SkSurface_getBackendRenderTarget'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#GrBackendRenderTarget'>GrBackendRenderTarget</a> <a href='#SkSurface_getBackendRenderTarget'>getBackendRenderTarget</a>(<a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> backendHandleAccess)
</pre>

Retrieves the back-end  <a href='undocumented#Render_Target'>render target</a>. If <a href='SkSurface_Reference#SkSurface'>SkSurface</a> has no back-end  <a href='undocumented#Render_Target'>render target</a>, an invalid
object is returned. Call <a href='undocumented#GrBackendRenderTarget'>GrBackendRenderTarget</a>::<a href='#GrBackendRenderTarget_isValid'>isValid</a> to determine if the result
is valid.

The returned <a href='undocumented#GrBackendRenderTarget'>GrBackendRenderTarget</a> should be discarded if the <a href='SkSurface_Reference#SkSurface'>SkSurface</a> is drawn to
or deleted.

### Parameters

<table>  <tr>    <td><a name='SkSurface_getBackendRenderTarget_backendHandleAccess'><code><strong>backendHandleAccess</strong></code></a></td>
    <td>one of:  <a href='#SkSurface_kFlushRead_BackendHandleAccess'>kFlushRead_BackendHandleAccess</a>,</td>
  </tr>
</table>

<a href='#SkSurface_kFlushWrite_BackendHandleAccess'>kFlushWrite_BackendHandleAccess</a>,
<a href='#SkSurface_kDiscardWrite_BackendHandleAccess'>kDiscardWrite_BackendHandleAccess</a>

### Return Value

GPU  <a href='undocumented#Render_Target'>render target</a> reference; invalid on failure

### See Also

<a href='undocumented#GrBackendRenderTarget'>GrBackendRenderTarget</a> <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_getBackendTexture'>getBackendTexture</a>

<a name='SkSurface_getCanvas'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* <a href='#SkSurface_getCanvas'>getCanvas</a>()
</pre>

Returns <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> that draws into <a href='SkSurface_Reference#SkSurface'>SkSurface</a>. Subsequent calls return the same <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>.
<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> returned is managed and owned by <a href='SkSurface_Reference#SkSurface'>SkSurface</a>, and is deleted when <a href='SkSurface_Reference#SkSurface'>SkSurface</a>
is deleted.

### Return Value

drawing <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> for <a href='SkSurface_Reference#SkSurface'>SkSurface</a>

### Example

<div><fiddle-embed name="33d0c5ad5a4810e533ae1010e29f8b75"></fiddle-embed></div>

### See Also

<a href='#SkSurface_makeSurface'>makeSurface</a> <a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a> <a href='#SkSurface_draw'>draw</a>

<a name='SkSurface_makeSurface'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&gt; <a href='#SkSurface_makeSurface'>makeSurface</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo)
</pre>

Returns a compatible <a href='SkSurface_Reference#SkSurface'>SkSurface</a>, or nullptr. Returned <a href='SkSurface_Reference#SkSurface'>SkSurface</a> contains
the same raster, GPU, or null properties as the original. Returned <a href='SkSurface_Reference#SkSurface'>SkSurface</a>
does not share the same pixels.

Returns nullptr if <a href='#SkSurface_makeSurface_imageInfo'>imageInfo</a> width or height are zero, or if <a href='#SkSurface_makeSurface_imageInfo'>imageInfo</a>
is incompatible with <a href='SkSurface_Reference#SkSurface'>SkSurface</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_makeSurface_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>,</td>
  </tr>
</table>

of <a href='SkSurface_Reference#SkSurface'>SkSurface</a>; width and height must be greater than zero

### Return Value

compatible <a href='SkSurface_Reference#SkSurface'>SkSurface</a> or nullptr

### Example

<div><fiddle-embed name="a9889b519a26896b900da0444e423c61"></fiddle-embed></div>

### See Also

<a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a> <a href='#SkSurface_getCanvas'>getCanvas</a> <a href='#SkSurface_draw'>draw</a>

<a name='SkSurface_makeImageSnapshot'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a>()
</pre>

Returns <a href='SkImage_Reference#SkImage'>SkImage</a> capturing <a href='SkSurface_Reference#SkSurface'>SkSurface</a> contents. Subsequent drawing to <a href='SkSurface_Reference#SkSurface'>SkSurface</a> contents
are not captured. <a href='SkImage_Reference#SkImage'>SkImage</a> allocation is accounted for if <a href='SkSurface_Reference#SkSurface'>SkSurface</a> was created with
<a href='undocumented#SkBudgeted'>SkBudgeted</a>::<a href='#SkBudgeted_kYes'>kYes</a>.

### Return Value

<a href='SkImage_Reference#SkImage'>SkImage</a> initialized with <a href='SkSurface_Reference#SkSurface'>SkSurface</a> contents

### Example

<div><fiddle-embed name="46f1fa0d95e590a64bed0140407ce5f7"></fiddle-embed></div>

### See Also

<a href='#SkSurface_draw'>draw</a> <a href='#SkSurface_getCanvas'>getCanvas</a>

<a name='SkSurface_makeImageSnapshot_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a>(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& bounds)
</pre>

Like the no-parameter version, this returns an <a href='SkImage_Reference#Image'>image</a> of the current <a href='SkSurface_Reference#Surface'>surface</a> contents.
This variant takes a rectangle specifying the subset of the <a href='SkSurface_Reference#Surface'>surface</a> that is of interest.
These bounds will be sanitized before being used.
- If bounds extends beyond the <a href='SkSurface_Reference#Surface'>surface</a>, it will be trimmed to just the intersection of
it and the <a href='SkSurface_Reference#Surface'>surface</a>.
- If bounds does not intersect the <a href='SkSurface_Reference#Surface'>surface</a>, then this returns nullptr.
- If bounds == the <a href='SkSurface_Reference#Surface'>surface</a>, then this is the same as calling the no-parameter variant.

### Example

<div><fiddle-embed name="b18b8ab693b09eb70a1d22ab63790cc7"></fiddle-embed></div>

### See Also

<a href='#SkSurface_draw'>draw</a> <a href='#SkSurface_getCanvas'>getCanvas</a>

<a name='Pixels'></a>

<a name='SkSurface_draw'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void draw(<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* <a href='SkCanvas_Reference#Canvas'>canvas</a>, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkSurface_Reference#SkSurface'>SkSurface</a> contents to <a href='#SkSurface_draw_canvas'>canvas</a>, with its top-left corner at (<a href='#SkSurface_draw_x'>x</a>, <a href='#SkSurface_draw_y'>y</a>).

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkSurface_draw_paint'>paint</a> is not nullptr, apply <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, and <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_draw_canvas'><code><strong>canvas</strong></code></a></td>
    <td><a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> drawn into</td>
  </tr>
  <tr>    <td><a name='SkSurface_draw_x'><code><strong>x</strong></code></a></td>
    <td>horizontal offset in <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_draw_y'><code><strong>y</strong></code></a></td>
    <td>vertical offset in <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_draw_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> containing <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
  </tr>
</table>

and so on; or nullptr

### Example

<div><fiddle-embed name="0de693f4d8dd898a60be8cfba23952be"></fiddle-embed></div>

### See Also

<a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a> <a href='#SkSurface_getCanvas'>getCanvas</a>

<a name='SkSurface_peekPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkSurface_peekPixels'>peekPixels</a>(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>* <a href='SkPixmap_Reference#Pixmap'>pixmap</a>)
</pre>

Copies <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='undocumented#Pixel'>pixel</a> address, row bytes, and <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> to <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>, if address
is available, and returns true. If <a href='undocumented#Pixel'>pixel</a> address is not available, return
false and leave <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> unchanged.

<a href='#SkSurface_peekPixels_pixmap'>pixmap</a> contents become invalid on any future change to <a href='SkSurface_Reference#SkSurface'>SkSurface</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_peekPixels_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td>storage for <a href='undocumented#Pixel'>pixel</a> state if pixels are readable; otherwise, ignored</td>
  </tr>
</table>

### Return Value

true if <a href='SkSurface_Reference#SkSurface'>SkSurface</a> has direct access to pixels

### Example

<div><fiddle-embed name="8c6184f22cfe068f021704cf92a147a1"></fiddle-embed></div>

### See Also

<a href='#SkSurface_readPixels'>readPixels</a> <a href='#SkSurface_writePixels'>writePixels</a>

<a name='SkSurface_readPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkSurface_readPixels'>readPixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& dst, int srcX, int srcY)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels to <a href='#SkSurface_readPixels_dst'>dst</a>.

Source <a href='SkRect_Reference#Rect'>Rect</a> corners are (<a href='#SkSurface_readPixels_srcX'>srcX</a>, <a href='#SkSurface_readPixels_srcY'>srcY</a>) and <a href='SkSurface_Reference#Surface'>Surface</a> (<a href='#SkSurface_width'>width()</a>, <a href='#SkSurface_height'>height()</a>).
Destination <a href='SkRect_Reference#Rect'>Rect</a> corners are (0, 0) and (<a href='#SkSurface_readPixels_dst'>dst</a>.<a href='#SkPixmap_width'>width()</a>, <a href='#SkSurface_readPixels_dst'>dst</a>.<a href='#SkPixmap_height'>height()</a>).
Copies each readable <a href='undocumented#Pixel'>pixel</a> intersecting both rectangles, without scaling,
converting to <a href='#SkSurface_readPixels_dst'>dst</a>.<a href='#SkPixmap_colorType'>colorType</a>() and <a href='#SkSurface_readPixels_dst'>dst</a>.<a href='#SkPixmap_alphaType'>alphaType</a>() if required.

Pixels are readable when <a href='SkSurface_Reference#Surface'>Surface</a> is raster, or backed by a GPU.

The destination  <a href='undocumented#Pixel_Storage'>pixel storage</a> must be allocated by the caller.

<a href='undocumented#Pixel'>Pixel</a> values are converted only if <a href='#Image_Info_Color_Type'>Color_Type</a> and <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>
do not match. Only pixels within both source and destination rectangles
are copied. <a href='#SkSurface_readPixels_dst'>dst</a> contents outside <a href='SkRect_Reference#Rect'>Rect</a> intersection are unchanged.

Pass negative values for <a href='#SkSurface_readPixels_srcX'>srcX</a> or <a href='#SkSurface_readPixels_srcY'>srcY</a> to offset pixels across or down destination.

Does not copy, and returns false if:

<table>  <tr>
    <td>Source and destination rectangles do not intersect.</td>
  </tr>  <tr>
    <td><a href='SkPixmap_Reference#Pixmap'>Pixmap</a> pixels could not be allocated.</td>
  </tr>  <tr>
    <td><a href='#SkSurface_readPixels_dst'>dst</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() is too small to contain one row of pixels.</td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkSurface_readPixels_dst'><code><strong>dst</strong></code></a></td>
    <td>storage for pixels copied from <a href='SkSurface_Reference#Surface'>Surface</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_srcX'><code><strong>srcX</strong></code></a></td>
    <td>offset into readable pixels on x-axis; may be negative</td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_srcY'><code><strong>srcY</strong></code></a></td>
    <td>offset into readable pixels on y-axis; may be negative</td>
  </tr>
</table>

### Return Value

true if pixels were copied

### Example

<div><fiddle-embed name="9f454fb93bca6482598d198b4121f0a6"></fiddle-embed></div>

### See Also

<a href='#SkSurface_peekPixels'>peekPixels</a> <a href='#SkSurface_writePixels'>writePixels</a>

<a name='SkSurface_readPixels_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkSurface_readPixels'>readPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& dstInfo, void* dstPixels, size_t dstRowBytes, int srcX, int srcY)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels from <a href='SkCanvas_Reference#Canvas'>Canvas</a> into <a href='#SkSurface_readPixels_2_dstPixels'>dstPixels</a>.

Source <a href='SkRect_Reference#Rect'>Rect</a> corners are (<a href='#SkSurface_readPixels_2_srcX'>srcX</a>, <a href='#SkSurface_readPixels_2_srcY'>srcY</a>) and <a href='SkSurface_Reference#Surface'>Surface</a> (<a href='#SkSurface_width'>width()</a>, <a href='#SkSurface_height'>height()</a>).
Destination <a href='SkRect_Reference#Rect'>Rect</a> corners are (0, 0) and (<a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_width'>width()</a>, <a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_height'>height()</a>).
Copies each readable <a href='undocumented#Pixel'>pixel</a> intersecting both rectangles, without scaling,
converting to <a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorType'>colorType</a>() and <a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() if required.

Pixels are readable when <a href='SkSurface_Reference#Surface'>Surface</a> is raster, or backed by a GPU.

The destination  <a href='undocumented#Pixel_Storage'>pixel storage</a> must be allocated by the caller.

<a href='undocumented#Pixel'>Pixel</a> values are converted only if <a href='#Image_Info_Color_Type'>Color_Type</a> and <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>
do not match. Only pixels within both source and destination rectangles
are copied. <a href='#SkSurface_readPixels_2_dstPixels'>dstPixels</a> contents outside <a href='SkRect_Reference#Rect'>Rect</a> intersection are unchanged.

Pass negative values for <a href='#SkSurface_readPixels_2_srcX'>srcX</a> or <a href='#SkSurface_readPixels_2_srcY'>srcY</a> to offset pixels across or down destination.

Does not copy, and returns false if:

<table>  <tr>
    <td>Source and destination rectangles do not intersect.</td>
  </tr>  <tr>
    <td><a href='SkSurface_Reference#Surface'>Surface</a> pixels could not be converted to <a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorType'>colorType</a>() or <a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>().</td>
  </tr>  <tr>
    <td><a href='#SkSurface_readPixels_2_dstRowBytes'>dstRowBytes</a> is too small to contain one row of pixels.</td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkSurface_readPixels_2_dstInfo'><code><strong>dstInfo</strong></code></a></td>
    <td>width, height, <a href='#Image_Info_Color_Type'>Color_Type</a>, and <a href='#Image_Info_Alpha_Type'>Alpha_Type</a> of <a href='#SkSurface_readPixels_2_dstPixels'>dstPixels</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_2_dstPixels'><code><strong>dstPixels</strong></code></a></td>
    <td>storage for pixels; <a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_height'>height()</a> times <a href='#SkSurface_readPixels_2_dstRowBytes'>dstRowBytes</a>, or larger</td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_2_dstRowBytes'><code><strong>dstRowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> of one destination row; <a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_width'>width()</a> times <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Size'>size</a>, or larger</td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_2_srcX'><code><strong>srcX</strong></code></a></td>
    <td>offset into readable pixels on x-axis; may be negative</td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_2_srcY'><code><strong>srcY</strong></code></a></td>
    <td>offset into readable pixels on y-axis; may be negative</td>
  </tr>
</table>

### Return Value

true if pixels were copied

### Example

<div><fiddle-embed name="484d60dab5d846bf28c7a4d48892324a"><div>A black <a href='undocumented#Oval'>oval</a> drawn on a red background provides an <a href='SkImage_Reference#Image'>image</a> to copy.
<a href='#SkSurface_readPixels'>readPixels</a> copies one quarter of the <a href='SkSurface_Reference#Surface'>Surface</a> into each of the four corners.
The copied quarter <a href='undocumented#Oval'>ovals</a> overdraw the original <a href='undocumented#Oval'>oval</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkSurface_peekPixels'>peekPixels</a> <a href='#SkSurface_writePixels'>writePixels</a>

<a name='SkSurface_readPixels_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkSurface_readPixels'>readPixels</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& dst, int srcX, int srcY)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels from <a href='SkSurface_Reference#Surface'>Surface</a> into <a href='SkBitmap_Reference#Bitmap'>bitmap</a>.

Source <a href='SkRect_Reference#Rect'>Rect</a> corners are (<a href='#SkSurface_readPixels_3_srcX'>srcX</a>, <a href='#SkSurface_readPixels_3_srcY'>srcY</a>) and <a href='SkSurface_Reference#Surface'>Surface</a> (<a href='#SkSurface_width'>width()</a>, <a href='#SkSurface_height'>height()</a>).
Destination <a href='SkRect_Reference#Rect'>Rect</a> corners are (0, 0) and (<a href='SkBitmap_Reference#Bitmap'>bitmap</a>.<a href='#SkSurface_width'>width()</a>, <a href='SkBitmap_Reference#Bitmap'>bitmap</a>.<a href='#SkSurface_height'>height()</a>).
Copies each readable <a href='undocumented#Pixel'>pixel</a> intersecting both rectangles, without scaling,
converting to <a href='#SkSurface_readPixels_3_dst'>dst</a>.<a href='#SkBitmap_colorType'>colorType</a>() and <a href='#SkSurface_readPixels_3_dst'>dst</a>.<a href='#SkBitmap_alphaType'>alphaType</a>() if required.

Pixels are readable when <a href='SkSurface_Reference#Surface'>Surface</a> is raster, or backed by a GPU.

The destination  <a href='undocumented#Pixel_Storage'>pixel storage</a> must be allocated by the caller.

<a href='undocumented#Pixel'>Pixel</a> values are converted only if <a href='#Image_Info_Color_Type'>Color_Type</a> and <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>
do not match. Only pixels within both source and destination rectangles
are copied. <a href='#SkSurface_readPixels_3_dst'>dst</a> contents outside <a href='SkRect_Reference#Rect'>Rect</a> intersection are unchanged.

Pass negative values for <a href='#SkSurface_readPixels_3_srcX'>srcX</a> or <a href='#SkSurface_readPixels_3_srcY'>srcY</a> to offset pixels across or down destination.

Does not copy, and returns false if:

<table>  <tr>
    <td>Source and destination rectangles do not intersect.</td>
  </tr>  <tr>
    <td><a href='SkSurface_Reference#Surface'>Surface</a> pixels could not be converted to <a href='#SkSurface_readPixels_3_dst'>dst</a>.<a href='#SkBitmap_colorType'>colorType</a>() or <a href='#SkSurface_readPixels_3_dst'>dst</a>.<a href='#SkBitmap_alphaType'>alphaType</a>().</td>
  </tr>  <tr>
    <td><a href='#SkSurface_readPixels_3_dst'>dst</a> pixels could not be allocated.</td>
  </tr>  <tr>
    <td><a href='#SkSurface_readPixels_3_dst'>dst</a>.<a href='#SkBitmap_rowBytes'>rowBytes</a>() is too small to contain one row of pixels.</td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkSurface_readPixels_3_dst'><code><strong>dst</strong></code></a></td>
    <td>storage for pixels copied from <a href='SkSurface_Reference#Surface'>Surface</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_3_srcX'><code><strong>srcX</strong></code></a></td>
    <td>offset into readable pixels on x-axis; may be negative</td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_3_srcY'><code><strong>srcY</strong></code></a></td>
    <td>offset into readable pixels on y-axis; may be negative</td>
  </tr>
</table>

### Return Value

true if pixels were copied

### Example

<div><fiddle-embed name="2d991a231e49d1de13eeb2ba9b440e01"></fiddle-embed></div>

### See Also

<a href='#SkSurface_peekPixels'>peekPixels</a> <a href='#SkSurface_writePixels'>writePixels</a>

<a name='SkSurface_writePixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkSurface_writePixels'>writePixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& src, int dstX, int dstY)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels from the <a href='#SkSurface_writePixels_src'>src</a> <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> to the <a href='SkSurface_Reference#Surface'>Surface</a>.

Source <a href='SkRect_Reference#Rect'>Rect</a> corners are (0, 0) and (<a href='#SkSurface_writePixels_src'>src</a>.<a href='#SkPixmap_width'>width()</a>, <a href='#SkSurface_writePixels_src'>src</a>.<a href='#SkPixmap_height'>height()</a>).
Destination <a href='SkRect_Reference#Rect'>Rect</a> corners are (<a href='#SkSurface_writePixels_dstX'>dstX</a>, <a href='#SkSurface_writePixels_dstY'>dstY</a>) and
<code>(<a href='#SkSurface_writePixels_dstX'>dstX</a> + <a href='SkSurface_Reference#Surface'>Surface</a> <a href='#SkSurface_width'>width()</a>, <a href='#SkSurface_writePixels_dstY'>dstY</a> + <a href='SkSurface_Reference#Surface'>Surface</a> <a href='#SkSurface_height'>height()</a>)</code>.

Copies each readable <a href='undocumented#Pixel'>pixel</a> intersecting both rectangles, without scaling,
converting to <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> and <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> if required.

### Parameters

<table>  <tr>    <td><a name='SkSurface_writePixels_src'><code><strong>src</strong></code></a></td>
    <td>storage for pixels to copy to <a href='SkSurface_Reference#Surface'>Surface</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_writePixels_dstX'><code><strong>dstX</strong></code></a></td>
    <td>x-axis position relative to <a href='SkSurface_Reference#Surface'>Surface</a> to begin copy; may be negative</td>
  </tr>
  <tr>    <td><a name='SkSurface_writePixels_dstY'><code><strong>dstY</strong></code></a></td>
    <td>y-axis position relative to <a href='SkSurface_Reference#Surface'>Surface</a> to begin copy; may be negative</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="760793bcf0ef193fa61ea03e6e8fc825"></fiddle-embed></div>

### See Also

<a href='#SkSurface_readPixels'>readPixels</a> <a href='#SkSurface_peekPixels'>peekPixels</a>

<a name='SkSurface_writePixels_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkSurface_writePixels'>writePixels</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& src, int dstX, int dstY)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels from the <a href='#SkSurface_writePixels_2_src'>src</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> to the <a href='SkSurface_Reference#Surface'>Surface</a>.

Source <a href='SkRect_Reference#Rect'>Rect</a> corners are (0, 0) and (<a href='#SkSurface_writePixels_2_src'>src</a>.<a href='#SkBitmap_width'>width()</a>, <a href='#SkSurface_writePixels_2_src'>src</a>.<a href='#SkBitmap_height'>height()</a>).
Destination <a href='SkRect_Reference#Rect'>Rect</a> corners are (<a href='#SkSurface_writePixels_2_dstX'>dstX</a>, <a href='#SkSurface_writePixels_2_dstY'>dstY</a>) and
<code>(<a href='#SkSurface_writePixels_2_dstX'>dstX</a> + <a href='SkSurface_Reference#Surface'>Surface</a> <a href='#SkSurface_width'>width()</a>, <a href='#SkSurface_writePixels_2_dstY'>dstY</a> + <a href='SkSurface_Reference#Surface'>Surface</a> <a href='#SkSurface_height'>height()</a>)</code>.

Copies each readable <a href='undocumented#Pixel'>pixel</a> intersecting both rectangles, without scaling,
converting to <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> and <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> if required.

### Parameters

<table>  <tr>    <td><a name='SkSurface_writePixels_2_src'><code><strong>src</strong></code></a></td>
    <td>storage for pixels to copy to <a href='SkSurface_Reference#Surface'>Surface</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_writePixels_2_dstX'><code><strong>dstX</strong></code></a></td>
    <td>x-axis position relative to <a href='SkSurface_Reference#Surface'>Surface</a> to begin copy; may be negative</td>
  </tr>
  <tr>    <td><a name='SkSurface_writePixels_2_dstY'><code><strong>dstY</strong></code></a></td>
    <td>y-axis position relative to <a href='SkSurface_Reference#Surface'>Surface</a> to begin copy; may be negative</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d77790dd3bc9f678fa4f582347fb8fba"></fiddle-embed></div>

### See Also

<a href='#SkSurface_readPixels'>readPixels</a> <a href='#SkSurface_peekPixels'>peekPixels</a>

<a name='SkSurface_props'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>& <a href='#SkSurface_props'>props()</a>const
</pre>

Returns <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a> for <a href='SkSurface_Reference#Surface'>surface</a>.

### Return Value

LCD striping orientation and setting for <a href='undocumented#Device'>device</a> independent fonts

### Example

<div><fiddle-embed name="13cf9e7b2894ae6e98c1fd719040bf01">

#### Example Output

~~~~
surf.props(): kRGB_H_SkPixelGeometry
~~~~

</fiddle-embed></div>

### See Also

<a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>

<a name='Utility'></a>

<a name='SkSurface_flush'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkSurface_flush'>flush()</a>
</pre>

Issues pending <a href='SkSurface_Reference#SkSurface'>SkSurface</a> commands to the GPU-backed API and resolves any <a href='SkSurface_Reference#SkSurface'>SkSurface</a> MSAA.

Skia flushes as needed, so it is not necessary to call this if Skia manages
drawing and object lifetime. Call when interleaving Skia calls with native
GPU calls.

### See Also

<a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a>

<a name='SkSurface_flushAndSignalSemaphores'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#GrSemaphoresSubmitted'>GrSemaphoresSubmitted</a> <a href='#SkSurface_flushAndSignalSemaphores'>flushAndSignalSemaphores</a>(int numSemaphores,
                                               <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a> signalSemaphores[])
</pre>

Issues pending <a href='SkSurface_Reference#SkSurface'>SkSurface</a> commands to the GPU-backed API and resolves any <a href='SkSurface_Reference#SkSurface'>SkSurface</a> MSAA.
After issuing all commands, <a href='#SkSurface_flushAndSignalSemaphores_signalSemaphores'>signalSemaphores</a> of count <a href='#SkSurface_flushAndSignalSemaphores_numSemaphores'>numSemaphores</a> semaphores
are signaled by the GPU.

For each <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a> in <a href='#SkSurface_flushAndSignalSemaphores_signalSemaphores'>signalSemaphores</a>:
if <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a> is initialized, the GPU back-end uses the semaphore as is;
otherwise, a new semaphore is created and initializes <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a>.

The caller must delete the semaphores created and returned in <a href='#SkSurface_flushAndSignalSemaphores_signalSemaphores'>signalSemaphores</a>.
<a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a> can be deleted as soon as this function returns.

If the back-end API is OpenGL only uninitialized  <a href='undocumented#Backend_Semaphore'>backend semaphores</a> are supported.

If the back-end API is Vulkan semaphores may be initialized or uninitialized.
If uninitialized, created semaphores are valid only with the VkDevice
with which they were created.

If <a href='undocumented#GrSemaphoresSubmitted'>GrSemaphoresSubmitted</a>::<a href='#GrSemaphoresSubmitted_kNo'>kNo</a> is returned, the GPU back-end did not create or
add any semaphores to signal on the GPU; the caller should not instruct the GPU
to wait on any of the semaphores.

Pending <a href='SkSurface_Reference#Surface'>surface</a> commands are flushed regardless of the return result.

### Parameters

<table>  <tr>    <td><a name='SkSurface_flushAndSignalSemaphores_numSemaphores'><code><strong>numSemaphores</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> of <a href='#SkSurface_flushAndSignalSemaphores_signalSemaphores'>signalSemaphores</a> array</td>
  </tr>
  <tr>    <td><a name='SkSurface_flushAndSignalSemaphores_signalSemaphores'><code><strong>signalSemaphores</strong></code></a></td>
    <td>array of semaphore containers</td>
  </tr>
</table>

### Return Value

one of: <a href='undocumented#GrSemaphoresSubmitted'>GrSemaphoresSubmitted</a>::<a href='#GrSemaphoresSubmitted_kYes'>kYes</a>, <a href='undocumented#GrSemaphoresSubmitted'>GrSemaphoresSubmitted</a>::<a href='#GrSemaphoresSubmitted_kNo'>kNo</a>

### See Also

<a href='#SkSurface_wait'>wait</a> <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a>

<a name='SkSurface_wait'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool wait(int numSemaphores, const <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a>* waitSemaphores)
</pre>

Inserts a list of GPU semaphores that the current GPU-backed API must wait on before
executing any more commands on the GPU for this <a href='SkSurface_Reference#Surface'>surface</a>. Skia will take ownership of the
underlying semaphores and delete them once they have been signaled and waited on.
If this call returns false, then the GPU back-end will not wait on any passed in semaphores,
and the client will still own the semaphores.

### Parameters

<table>  <tr>    <td><a name='SkSurface_wait_numSemaphores'><code><strong>numSemaphores</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> of <a href='#SkSurface_wait_waitSemaphores'>waitSemaphores</a> array</td>
  </tr>
  <tr>    <td><a name='SkSurface_wait_waitSemaphores'><code><strong>waitSemaphores</strong></code></a></td>
    <td>array of semaphore containers</td>
  </tr>
</table>

### Return Value

true if GPU is waiting on semaphores

### See Also

<a href='#SkSurface_flushAndSignalSemaphores'>flushAndSignalSemaphores</a> <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a>

<a name='SkSurface_characterize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkSurface_characterize'>characterize</a>(<a href='undocumented#SkSurfaceCharacterization'>SkSurfaceCharacterization</a>* characterization)const
</pre>

Initializes <a href='undocumented#SkSurfaceCharacterization'>SkSurfaceCharacterization</a> that can be used to perform GPU back-end
processing in a separate thread. Typically this is used to divide drawing
into multiple tiles. <a href='undocumented#SkDeferredDisplayListRecorder'>SkDeferredDisplayListRecorder</a> records the drawing commands
for each tile.

Return true if <a href='SkSurface_Reference#SkSurface'>SkSurface</a> supports <a href='#SkSurface_characterize_characterization'>characterization</a>.  <a href='undocumented#Raster_Surface'>raster surface</a> returns false.

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

<a href='#SkSurface_draw'>draw()</a> <a href='undocumented#SkSurfaceCharacterization'>SkSurfaceCharacterization</a> <a href='undocumented#SkDeferredDisplayList'>SkDeferredDisplayList</a>

<a name='SkSurface_draw_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool draw(<a href='undocumented#SkDeferredDisplayList'>SkDeferredDisplayList</a>* deferredDisplayList)
</pre>

Draws   deferred display list created using <a href='undocumented#SkDeferredDisplayListRecorder'>SkDeferredDisplayListRecorder</a>.
Has no effect and returns false if <a href='undocumented#SkSurfaceCharacterization'>SkSurfaceCharacterization</a> stored in
<a href='#SkSurface_draw_2_deferredDisplayList'>deferredDisplayList</a> is not compatible with <a href='SkSurface_Reference#SkSurface'>SkSurface</a>.

<a href='undocumented#Raster_Surface'>raster surface</a> returns false.

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

<a href='#SkSurface_characterize'>characterize()</a> <a href='undocumented#SkSurfaceCharacterization'>SkSurfaceCharacterization</a> <a href='undocumented#SkDeferredDisplayList'>SkDeferredDisplayList</a>

