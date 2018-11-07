SkSurface Reference
===


<a name='SkSurface'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkSurface_Reference#SkSurface'>SkSurface</a> : <a href='SkSurface_Reference#SkSurface'>public</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> {
<a href='undocumented#SkRefCnt'>public</a>:
    <a href='undocumented#SkRefCnt'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRasterDirect'>MakeRasterDirect</a>(<a href='#SkSurface_MakeRasterDirect'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>imageInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>pixels</a>,
                                      <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>rowBytes</a>,
                                      <a href='SkImageInfo_Reference#SkImageInfo'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>surfaceProps</a> = <a href='undocumented#SkSurfaceProps'>nullptr</a>);
    <a href='undocumented#SkSurfaceProps'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRasterDirectReleaseProc'>MakeRasterDirectReleaseProc</a>(<a href='#SkSurface_MakeRasterDirectReleaseProc'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>imageInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>pixels</a>,
                                    <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>rowBytes</a>,
                                    <a href='SkImageInfo_Reference#SkImageInfo'>void</a> (*<a href='SkImageInfo_Reference#SkImageInfo'>releaseProc</a>)(<a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>pixels</a>, <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>context</a>),
                                    <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>context</a>, <a href='SkImageInfo_Reference#SkImageInfo'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>surfaceProps</a> = <a href='undocumented#SkSurfaceProps'>nullptr</a>);
    <a href='undocumented#SkSurfaceProps'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRaster'>MakeRaster</a>(<a href='#SkSurface_MakeRaster'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>imageInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>rowBytes</a>,
                                       <a href='SkImageInfo_Reference#SkImageInfo'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>surfaceProps</a>);
    <a href='undocumented#SkSurfaceProps'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRaster'>MakeRaster</a>(<a href='#SkSurface_MakeRaster'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>imageInfo</a>,
                                       <a href='SkImageInfo_Reference#SkImageInfo'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>props</a> = <a href='undocumented#SkSurfaceProps'>nullptr</a>);
    <a href='undocumented#SkSurfaceProps'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRasterN32Premul'>MakeRasterN32Premul</a>(<a href='#SkSurface_MakeRasterN32Premul'>int</a> <a href='#SkSurface_MakeRasterN32Premul'>width</a>, <a href='#SkSurface_MakeRasterN32Premul'>int</a> <a href='#SkSurface_MakeRasterN32Premul'>height</a>,
                                                <a href='#SkSurface_MakeRasterN32Premul'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>surfaceProps</a> = <a href='undocumented#SkSurfaceProps'>nullptr</a>);
    <a href='undocumented#SkSurfaceProps'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeFromBackendTexture'>MakeFromBackendTexture</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>,
                                                   <a href='undocumented#GrContext'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& <a href='undocumented#GrBackendTexture'>backendTexture</a>,
                                                   <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>origin</a>, <a href='undocumented#GrSurfaceOrigin'>int</a> <a href='undocumented#GrSurfaceOrigin'>sampleCnt</a>,
                                                   <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>colorType</a>,
                                                   <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='undocumented#SkColorSpace'>colorSpace</a>,
                                                   <a href='undocumented#SkColorSpace'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>surfaceProps</a>);
    <a href='undocumented#SkSurfaceProps'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>,
                                                <a href='undocumented#GrContext'>const</a> <a href='undocumented#GrBackendRenderTarget'>GrBackendRenderTarget</a>& <a href='undocumented#GrBackendRenderTarget'>backendRenderTarget</a>,
                                                <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>origin</a>,
                                                <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>colorType</a>,
                                                <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='undocumented#SkColorSpace'>colorSpace</a>,
                                                <a href='undocumented#SkColorSpace'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>surfaceProps</a>);
    <a href='undocumented#SkSurfaceProps'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget'>MakeFromBackendTextureAsRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>,
                                                            <a href='undocumented#GrContext'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& <a href='undocumented#GrBackendTexture'>backendTexture</a>,
                                                            <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>origin</a>,
                                                            <a href='undocumented#GrSurfaceOrigin'>int</a> <a href='undocumented#GrSurfaceOrigin'>sampleCnt</a>,
                                                            <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>colorType</a>,
                                                            <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='undocumented#SkColorSpace'>colorSpace</a>,
                                                            <a href='undocumented#SkColorSpace'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>surfaceProps</a>);
    <a href='undocumented#SkSurfaceProps'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>, <a href='undocumented#SkBudgeted'>SkBudgeted</a> <a href='undocumented#SkBudgeted'>budgeted</a>,
                                             <a href='undocumented#SkBudgeted'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>imageInfo</a>,
                                             <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>sampleCount</a>, <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>surfaceOrigin</a>,
                                             <a href='undocumented#GrSurfaceOrigin'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>surfaceProps</a>,
                                             <a href='undocumented#SkSurfaceProps'>bool</a> <a href='undocumented#SkSurfaceProps'>shouldCreateWithMips</a> = <a href='undocumented#SkSurfaceProps'>false</a>);
    <a href='undocumented#SkSurfaceProps'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>, <a href='undocumented#SkBudgeted'>SkBudgeted</a> <a href='undocumented#SkBudgeted'>budgeted</a>,
                                             <a href='undocumented#SkBudgeted'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>imageInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>sampleCount</a>,
                                             <a href='SkImageInfo_Reference#SkImageInfo'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>props</a>);
    <a href='undocumented#SkSurfaceProps'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>, <a href='undocumented#SkBudgeted'>SkBudgeted</a> <a href='undocumented#SkBudgeted'>budgeted</a>,
                                             <a href='undocumented#SkBudgeted'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>imageInfo</a>);
    <a href='SkImageInfo_Reference#SkImageInfo'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>,
                                             <a href='undocumented#GrContext'>const</a> <a href='undocumented#SkSurfaceCharacterization'>SkSurfaceCharacterization</a>& <a href='undocumented#SkSurfaceCharacterization'>characterization</a>,
                                             <a href='undocumented#SkBudgeted'>SkBudgeted</a> <a href='undocumented#SkBudgeted'>budgeted</a>);
    <a href='undocumented#SkBudgeted'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_MakeNull'>MakeNull</a>(<a href='#SkSurface_MakeNull'>int</a> <a href='#SkSurface_MakeNull'>width</a>, <a href='#SkSurface_MakeNull'>int</a> <a href='#SkSurface_MakeNull'>height</a>);
    <a href='#SkSurface_MakeNull'>int</a> <a href='#SkSurface_width'>width()</a> <a href='#SkSurface_width'>const</a>;
    <a href='#SkSurface_width'>int</a> <a href='#SkSurface_height'>height()</a> <a href='#SkSurface_height'>const</a>;
    <a href='#SkSurface_height'>uint32_t</a> <a href='#SkSurface_generationID'>generationID</a>();

    <a href='#SkSurface_generationID'>enum</a> <a href='#SkSurface_ContentChangeMode'>ContentChangeMode</a> {
        <a href='#SkSurface_kDiscard_ContentChangeMode'>kDiscard_ContentChangeMode</a>,
        <a href='#SkSurface_kRetain_ContentChangeMode'>kRetain_ContentChangeMode</a>,
    };

    <a href='#SkSurface_kRetain_ContentChangeMode'>void</a> <a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a>(<a href='#SkSurface_ContentChangeMode'>ContentChangeMode</a> <a href='#SkSurface_ContentChangeMode'>mode</a>);

    <a href='#SkSurface_ContentChangeMode'>enum</a> <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> {
        <a href='#SkSurface_kFlushRead_BackendHandleAccess'>kFlushRead_BackendHandleAccess</a>,
        <a href='#SkSurface_kFlushWrite_BackendHandleAccess'>kFlushWrite_BackendHandleAccess</a>,
        <a href='#SkSurface_kDiscardWrite_BackendHandleAccess'>kDiscardWrite_BackendHandleAccess</a>,
    };

    <a href='#SkSurface_kDiscardWrite_BackendHandleAccess'>static</a> <a href='#SkSurface_kDiscardWrite_BackendHandleAccess'>const</a> <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_kFlushRead_TextureHandleAccess'>kFlushRead_TextureHandleAccess</a> =
            <a href='#SkSurface_kFlushRead_BackendHandleAccess'>kFlushRead_BackendHandleAccess</a>;
    <a href='#SkSurface_kFlushRead_BackendHandleAccess'>static</a> <a href='#SkSurface_kFlushRead_BackendHandleAccess'>const</a> <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_kFlushWrite_TextureHandleAccess'>kFlushWrite_TextureHandleAccess</a> =
            <a href='#SkSurface_kFlushWrite_BackendHandleAccess'>kFlushWrite_BackendHandleAccess</a>;
    <a href='#SkSurface_kFlushWrite_BackendHandleAccess'>static</a> <a href='#SkSurface_kFlushWrite_BackendHandleAccess'>const</a> <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_kDiscardWrite_TextureHandleAccess'>kDiscardWrite_TextureHandleAccess</a> =
            <a href='#SkSurface_kDiscardWrite_BackendHandleAccess'>kDiscardWrite_BackendHandleAccess</a>;
    <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='#SkSurface_getBackendTexture'>getBackendTexture</a>(<a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_BackendHandleAccess'>backendHandleAccess</a>);
    <a href='undocumented#GrBackendRenderTarget'>GrBackendRenderTarget</a> <a href='#SkSurface_getBackendRenderTarget'>getBackendRenderTarget</a>(<a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_BackendHandleAccess'>backendHandleAccess</a>);
    <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* <a href='#SkSurface_getCanvas'>getCanvas</a>();
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkSurface_Reference#SkSurface'>SkSurface</a>> <a href='#SkSurface_makeSurface'>makeSurface</a>(<a href='#SkSurface_makeSurface'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>imageInfo</a>);
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a>();
    <a href='#SkSurface_makeImageSnapshot'>void</a> <a href='#SkSurface_makeImageSnapshot'>draw</a>(<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* <a href='SkCanvas_Reference#Canvas'>canvas</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>, <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>);
    <a href='SkPaint_Reference#Paint'>bool</a> <a href='#SkSurface_peekPixels'>peekPixels</a>(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>* <a href='SkPixmap_Reference#Pixmap'>pixmap</a>);
    <a href='SkPixmap_Reference#Pixmap'>bool</a> <a href='#SkSurface_readPixels'>readPixels</a>(<a href='#SkSurface_readPixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#SkPixmap'>dst</a>, <a href='SkPixmap_Reference#SkPixmap'>int</a> <a href='SkPixmap_Reference#SkPixmap'>srcX</a>, <a href='SkPixmap_Reference#SkPixmap'>int</a> <a href='SkPixmap_Reference#SkPixmap'>srcY</a>);
    <a href='SkPixmap_Reference#SkPixmap'>bool</a> <a href='#SkSurface_readPixels'>readPixels</a>(<a href='#SkSurface_readPixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>dstInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>dstPixels</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>dstRowBytes</a>,
                    <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>srcX</a>, <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>srcY</a>);
    <a href='SkImageInfo_Reference#SkImageInfo'>bool</a> <a href='#SkSurface_readPixels'>readPixels</a>(<a href='#SkSurface_readPixels'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#SkBitmap'>dst</a>, <a href='SkBitmap_Reference#SkBitmap'>int</a> <a href='SkBitmap_Reference#SkBitmap'>srcX</a>, <a href='SkBitmap_Reference#SkBitmap'>int</a> <a href='SkBitmap_Reference#SkBitmap'>srcY</a>);
    <a href='SkBitmap_Reference#SkBitmap'>void</a> <a href='#SkSurface_writePixels'>writePixels</a>(<a href='#SkSurface_writePixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#SkPixmap'>src</a>, <a href='SkPixmap_Reference#SkPixmap'>int</a> <a href='SkPixmap_Reference#SkPixmap'>dstX</a>, <a href='SkPixmap_Reference#SkPixmap'>int</a> <a href='SkPixmap_Reference#SkPixmap'>dstY</a>);
    <a href='SkPixmap_Reference#SkPixmap'>void</a> <a href='#SkSurface_writePixels'>writePixels</a>(<a href='#SkSurface_writePixels'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#SkBitmap'>src</a>, <a href='SkBitmap_Reference#SkBitmap'>int</a> <a href='SkBitmap_Reference#SkBitmap'>dstX</a>, <a href='SkBitmap_Reference#SkBitmap'>int</a> <a href='SkBitmap_Reference#SkBitmap'>dstY</a>);
    <a href='SkBitmap_Reference#SkBitmap'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>& <a href='#SkSurface_props'>props()</a> <a href='#SkSurface_props'>const</a>;
    <a href='#SkSurface_props'>void</a> <a href='#SkSurface_prepareForExternalIO'>prepareForExternalIO</a>();
    <a href='#SkSurface_prepareForExternalIO'>void</a> <a href='#SkSurface_flush'>flush()</a>;
    <a href='undocumented#GrSemaphoresSubmitted'>GrSemaphoresSubmitted</a> <a href='#SkSurface_flushAndSignalSemaphores'>flushAndSignalSemaphores</a>(<a href='#SkSurface_flushAndSignalSemaphores'>int</a> <a href='#SkSurface_flushAndSignalSemaphores'>numSemaphores</a>,
                                                   <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a> <a href='undocumented#GrBackendSemaphore'>signalSemaphores</a>[]);
    <a href='undocumented#GrBackendSemaphore'>bool</a> <a href='undocumented#GrBackendSemaphore'>wait</a>(<a href='undocumented#GrBackendSemaphore'>int</a> <a href='undocumented#GrBackendSemaphore'>numSemaphores</a>, <a href='undocumented#GrBackendSemaphore'>const</a> <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a>* <a href='undocumented#GrBackendSemaphore'>waitSemaphores</a>);
    <a href='undocumented#GrBackendSemaphore'>bool</a> <a href='#SkSurface_characterize'>characterize</a>(<a href='undocumented#SkSurfaceCharacterization'>SkSurfaceCharacterization</a>* <a href='undocumented#SkSurfaceCharacterization'>characterization</a>) <a href='undocumented#SkSurfaceCharacterization'>const</a>;
    <a href='undocumented#SkSurfaceCharacterization'>bool</a> <a href='undocumented#SkSurfaceCharacterization'>draw</a>(<a href='undocumented#SkDeferredDisplayList'>SkDeferredDisplayList</a>* <a href='undocumented#SkDeferredDisplayList'>deferredDisplayList</a>);
};
</pre>

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>responsible</a> <a href='SkSurface_Reference#SkSurface'>for</a> <a href='SkSurface_Reference#SkSurface'>managing</a> <a href='SkSurface_Reference#SkSurface'>the</a> <a href='SkSurface_Reference#SkSurface'>pixels</a> <a href='SkSurface_Reference#SkSurface'>that</a> <a href='SkSurface_Reference#SkSurface'>a</a> <a href='SkCanvas_Reference#Canvas'>canvas</a> <a href='SkCanvas_Reference#Canvas'>draws</a> <a href='SkCanvas_Reference#Canvas'>into</a>. <a href='SkCanvas_Reference#Canvas'>The</a> <a href='SkCanvas_Reference#Canvas'>pixels</a> <a href='SkCanvas_Reference#Canvas'>can</a> <a href='SkCanvas_Reference#Canvas'>be</a>
<a href='SkCanvas_Reference#Canvas'>allocated</a> <a href='SkCanvas_Reference#Canvas'>either</a> <a href='SkCanvas_Reference#Canvas'>in</a> <a href='SkCanvas_Reference#Canvas'>CPU</a> <a href='SkCanvas_Reference#Canvas'>memory</a>, <a href='SkCanvas_Reference#Canvas'>if</a> <a href='SkCanvas_Reference#Canvas'>a</a>  <a href='undocumented#Raster_Surface'>raster surface</a>; <a href='SkCanvas_Reference#Canvas'>or</a> <a href='SkCanvas_Reference#Canvas'>on</a> <a href='SkCanvas_Reference#Canvas'>the</a> <a href='SkCanvas_Reference#Canvas'>GPU</a>, <a href='SkCanvas_Reference#Canvas'>for</a> <a href='SkCanvas_Reference#Canvas'>a</a> <a href='undocumented#GrRenderTarget'>GrRenderTarget</a> <a href='SkSurface_Reference#Surface'>surface</a>.
<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>takes</a> <a href='SkSurface_Reference#SkSurface'>care</a> <a href='SkSurface_Reference#SkSurface'>of</a> <a href='SkSurface_Reference#SkSurface'>allocating</a> <a href='SkSurface_Reference#SkSurface'>a</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>that</a> <a href='SkCanvas_Reference#SkCanvas'>will</a> <a href='SkCanvas_Reference#SkCanvas'>draw</a> <a href='SkCanvas_Reference#SkCanvas'>into</a> <a href='SkCanvas_Reference#SkCanvas'>the</a> <a href='SkSurface_Reference#Surface'>surface</a>. <a href='SkSurface_Reference#Surface'>Call</a>
<a href='SkSurface_Reference#Surface'>surface</a>-><a href='#SkSurface_getCanvas'>getCanvas</a>() <a href='#SkSurface_getCanvas'>to</a> <a href='#SkSurface_getCanvas'>use</a> <a href='#SkSurface_getCanvas'>that</a> <a href='SkCanvas_Reference#Canvas'>canvas</a>. <a href='SkCanvas_Reference#Canvas'>The</a> <a href='SkCanvas_Reference#Canvas'>caller</a> <a href='SkCanvas_Reference#Canvas'>should</a> <a href='SkCanvas_Reference#Canvas'>not</a> <a href='SkCanvas_Reference#Canvas'>delete</a> <a href='SkCanvas_Reference#Canvas'>the</a> <a href='SkCanvas_Reference#Canvas'>returned</a> <a href='SkCanvas_Reference#Canvas'>canvas</a>;
<a href='SkCanvas_Reference#Canvas'>it</a> <a href='SkCanvas_Reference#Canvas'>is</a> <a href='SkCanvas_Reference#Canvas'>owned</a> <a href='SkCanvas_Reference#Canvas'>by</a> <a href='SkSurface_Reference#Surface'>surface</a>.

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>always</a> <a href='SkSurface_Reference#SkSurface'>has</a> <a href='SkSurface_Reference#SkSurface'>non-zero</a> <a href='SkSurface_Reference#SkSurface'>dimensions</a>. <a href='SkSurface_Reference#SkSurface'>If</a> <a href='SkSurface_Reference#SkSurface'>there</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>a</a> <a href='SkSurface_Reference#SkSurface'>request</a> <a href='SkSurface_Reference#SkSurface'>for</a> <a href='SkSurface_Reference#SkSurface'>a</a> <a href='SkSurface_Reference#SkSurface'>new</a> <a href='SkSurface_Reference#Surface'>surface</a>, <a href='SkSurface_Reference#Surface'>and</a> <a href='SkSurface_Reference#Surface'>either</a>
<a href='SkSurface_Reference#Surface'>of</a> <a href='SkSurface_Reference#Surface'>the</a> <a href='SkSurface_Reference#Surface'>requested</a> <a href='SkSurface_Reference#Surface'>dimensions</a> <a href='SkSurface_Reference#Surface'>are</a> <a href='SkSurface_Reference#Surface'>zero</a>, <a href='SkSurface_Reference#Surface'>then</a> <a href='SkSurface_Reference#Surface'>nullptr</a> <a href='SkSurface_Reference#Surface'>will</a> <a href='SkSurface_Reference#Surface'>be</a> <a href='SkSurface_Reference#Surface'>returned</a>.

<a name='SkSurface_MakeRasterDirect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&<a href='SkSurface_Reference#SkSurface'>gt</a>; <a href='#SkSurface_MakeRasterDirect'>MakeRasterDirect</a>(<a href='#SkSurface_MakeRasterDirect'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>imageInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>pixels</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>rowBytes</a>,
                                         <a href='SkImageInfo_Reference#SkImageInfo'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>surfaceProps</a> = <a href='undocumented#SkSurfaceProps'>nullptr</a>)
</pre>

Allocates raster <a href='SkSurface_Reference#SkSurface'>SkSurface</a>. <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>returned</a> <a href='SkCanvas_Reference#SkCanvas'>by</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>draws</a> <a href='SkSurface_Reference#SkSurface'>directly</a> <a href='SkSurface_Reference#SkSurface'>into</a> <a href='#SkSurface_MakeRasterDirect_pixels'>pixels</a>.

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>returned</a> <a href='SkSurface_Reference#SkSurface'>if</a> <a href='SkSurface_Reference#SkSurface'>all</a> <a href='SkSurface_Reference#SkSurface'>parameters</a> <a href='SkSurface_Reference#SkSurface'>are</a> <a href='SkSurface_Reference#SkSurface'>valid</a>.
Valid parameters include:
info dimensions are greater than zero;
info contains <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>and</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>supported</a> <a href='SkImageInfo_Reference#SkAlphaType'>by</a>  <a href='undocumented#Raster_Surface'>raster surface</a>;
<a href='#SkSurface_MakeRasterDirect_pixels'>pixels</a> <a href='#SkSurface_MakeRasterDirect_pixels'>is</a> <a href='#SkSurface_MakeRasterDirect_pixels'>not</a> <a href='#SkSurface_MakeRasterDirect_pixels'>nullptr</a>;
<a href='#SkSurface_MakeRasterDirect_rowBytes'>rowBytes</a> <a href='#SkSurface_MakeRasterDirect_rowBytes'>is</a> <a href='#SkSurface_MakeRasterDirect_rowBytes'>large</a> <a href='#SkSurface_MakeRasterDirect_rowBytes'>enough</a> <a href='#SkSurface_MakeRasterDirect_rowBytes'>to</a> <a href='#SkSurface_MakeRasterDirect_rowBytes'>contain</a> <a href='#SkSurface_MakeRasterDirect_rowBytes'>info</a> <a href='#SkSurface_MakeRasterDirect_rowBytes'>width</a> <a href='#SkSurface_MakeRasterDirect_pixels'>pixels</a> <a href='#SkSurface_MakeRasterDirect_pixels'>of</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.

<a href='undocumented#Pixel'>Pixel</a> <a href='undocumented#Pixel'>buffer</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>should</a> <a href='undocumented#Size'>be</a> <a href='undocumented#Size'>info</a> <a href='undocumented#Size'>height</a> <a href='undocumented#Size'>times</a> <a href='undocumented#Size'>computed</a> <a href='#SkSurface_MakeRasterDirect_rowBytes'>rowBytes</a>.
Pixels are not initialized.
To access <a href='#SkSurface_MakeRasterDirect_pixels'>pixels</a> <a href='#SkSurface_MakeRasterDirect_pixels'>after</a> <a href='#SkSurface_MakeRasterDirect_pixels'>drawing</a>, <a href='#SkSurface_MakeRasterDirect_pixels'>call</a> <a href='#SkSurface_flush'>flush()</a> <a href='#SkSurface_flush'>or</a> <a href='#SkSurface_peekPixels'>peekPixels</a>().

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRasterDirect_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>,</td>
  </tr>
</table>

of  <a href='undocumented#Raster_Surface'>raster surface</a>; width and height must be greater than zero

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRasterDirect_pixels'><code><strong>pixels</strong></code></a></td>
    <td>pointer to destination <a href='#SkSurface_MakeRasterDirect_pixels'>pixels</a> <a href='#SkSurface_MakeRasterDirect_pixels'>buffer</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirect_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>interval from one <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>row</a> <a href='SkSurface_Reference#SkSurface'>to</a> <a href='SkSurface_Reference#SkSurface'>the</a> <a href='SkSurface_Reference#SkSurface'>next</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirect_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>independent</a> <a href='undocumented#Device'>fonts</a>;</td>
  </tr>
</table>

may be nullptr

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>if</a> <a href='SkSurface_Reference#SkSurface'>all</a> <a href='SkSurface_Reference#SkSurface'>parameters</a> <a href='SkSurface_Reference#SkSurface'>are</a> <a href='SkSurface_Reference#SkSurface'>valid</a>; <a href='SkSurface_Reference#SkSurface'>otherwise</a>, <a href='SkSurface_Reference#SkSurface'>nullptr</a>

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
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&<a href='SkSurface_Reference#SkSurface'>gt</a>; <a href='#SkSurface_MakeRasterDirectReleaseProc'>MakeRasterDirectReleaseProc</a>(<a href='#SkSurface_MakeRasterDirectReleaseProc'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>imageInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>pixels</a>,
                                           <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>rowBytes</a>, <a href='SkImageInfo_Reference#SkImageInfo'>void</a> (*<a href='SkImageInfo_Reference#SkImageInfo'>releaseProc</a>) (<a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>pixels</a>,
                                           <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>context</a>) , <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>context</a>,
                                           <a href='SkImageInfo_Reference#SkImageInfo'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>surfaceProps</a> = <a href='undocumented#SkSurfaceProps'>nullptr</a>)
</pre>

Allocates raster <a href='SkSurface_Reference#SkSurface'>SkSurface</a>. <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>returned</a> <a href='SkCanvas_Reference#SkCanvas'>by</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>draws</a> <a href='SkSurface_Reference#SkSurface'>directly</a> <a href='SkSurface_Reference#SkSurface'>into</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>pixels</a>.
<a href='#SkSurface_MakeRasterDirectReleaseProc_releaseProc'>releaseProc</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_releaseProc'>is</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_releaseProc'>called</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_releaseProc'>with</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>pixels</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>and</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_context'>context</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_context'>when</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>deleted</a>.

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>returned</a> <a href='SkSurface_Reference#SkSurface'>if</a> <a href='SkSurface_Reference#SkSurface'>all</a> <a href='SkSurface_Reference#SkSurface'>parameters</a> <a href='SkSurface_Reference#SkSurface'>are</a> <a href='SkSurface_Reference#SkSurface'>valid</a>.
Valid parameters include:
info dimensions are greater than zero;
info contains <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>and</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>supported</a> <a href='SkImageInfo_Reference#SkAlphaType'>by</a>  <a href='undocumented#Raster_Surface'>raster surface</a>;
<a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>pixels</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>is</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>not</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>nullptr</a>;
<a href='#SkSurface_MakeRasterDirectReleaseProc_rowBytes'>rowBytes</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_rowBytes'>is</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_rowBytes'>large</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_rowBytes'>enough</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_rowBytes'>to</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_rowBytes'>contain</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_rowBytes'>info</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_rowBytes'>width</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>pixels</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>of</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.

<a href='undocumented#Pixel'>Pixel</a> <a href='undocumented#Pixel'>buffer</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>should</a> <a href='undocumented#Size'>be</a> <a href='undocumented#Size'>info</a> <a href='undocumented#Size'>height</a> <a href='undocumented#Size'>times</a> <a href='undocumented#Size'>computed</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_rowBytes'>rowBytes</a>.
Pixels are not initialized.
To access <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>pixels</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>after</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>drawing</a>, <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>call</a> <a href='#SkSurface_flush'>flush()</a> <a href='#SkSurface_flush'>or</a> <a href='#SkSurface_peekPixels'>peekPixels</a>().

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRasterDirectReleaseProc_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>,</td>
  </tr>
</table>

of  <a href='undocumented#Raster_Surface'>raster surface</a>; width and height must be greater than zero

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRasterDirectReleaseProc_pixels'><code><strong>pixels</strong></code></a></td>
    <td>pointer to destination <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>pixels</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_pixels'>buffer</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirectReleaseProc_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>interval from one <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>row</a> <a href='SkSurface_Reference#SkSurface'>to</a> <a href='SkSurface_Reference#SkSurface'>the</a> <a href='SkSurface_Reference#SkSurface'>next</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirectReleaseProc_releaseProc'><code><strong>releaseProc</strong></code></a></td>
    <td>called when <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>deleted</a>; <a href='SkSurface_Reference#SkSurface'>may</a> <a href='SkSurface_Reference#SkSurface'>be</a> <a href='SkSurface_Reference#SkSurface'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirectReleaseProc_context'><code><strong>context</strong></code></a></td>
    <td>passed to <a href='#SkSurface_MakeRasterDirectReleaseProc_releaseProc'>releaseProc</a>; <a href='#SkSurface_MakeRasterDirectReleaseProc_releaseProc'>may</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_releaseProc'>be</a> <a href='#SkSurface_MakeRasterDirectReleaseProc_releaseProc'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterDirectReleaseProc_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>independent</a> <a href='undocumented#Device'>fonts</a>;</td>
  </tr>
</table>

may be nullptr

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>if</a> <a href='SkSurface_Reference#SkSurface'>all</a> <a href='SkSurface_Reference#SkSurface'>parameters</a> <a href='SkSurface_Reference#SkSurface'>are</a> <a href='SkSurface_Reference#SkSurface'>valid</a>; <a href='SkSurface_Reference#SkSurface'>otherwise</a>, <a href='SkSurface_Reference#SkSurface'>nullptr</a>

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
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&<a href='SkSurface_Reference#SkSurface'>gt</a>; <a href='#SkSurface_MakeRaster'>MakeRaster</a>(<a href='#SkSurface_MakeRaster'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>imageInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>rowBytes</a>,
                                   <a href='SkImageInfo_Reference#SkImageInfo'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>surfaceProps</a>)
</pre>

Allocates raster <a href='SkSurface_Reference#SkSurface'>SkSurface</a>. <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>returned</a> <a href='SkCanvas_Reference#SkCanvas'>by</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>draws</a> <a href='SkSurface_Reference#SkSurface'>directly</a> <a href='SkSurface_Reference#SkSurface'>into</a> <a href='SkSurface_Reference#SkSurface'>pixels</a>.
Allocates and zeroes <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a>. <a href='undocumented#Pixel'>Pixel</a> <a href='undocumented#Pixel'>memory</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>is</a> <a href='#SkSurface_MakeRaster_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_height'>height()</a> <a href='#SkImageInfo_height'>times</a>
<a href='#SkSurface_MakeRaster_rowBytes'>rowBytes</a>, <a href='#SkSurface_MakeRaster_rowBytes'>or</a> <a href='#SkSurface_MakeRaster_rowBytes'>times</a> <a href='#SkSurface_MakeRaster_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>() <a href='#SkImageInfo_minRowBytes'>if</a> <a href='#SkSurface_MakeRaster_rowBytes'>rowBytes</a> <a href='#SkSurface_MakeRaster_rowBytes'>is</a> <a href='#SkSurface_MakeRaster_rowBytes'>zero</a>.
<a href='undocumented#Pixel'>Pixel</a> <a href='undocumented#Pixel'>memory</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>deleted</a> <a href='undocumented#Pixel'>when</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>deleted</a>.

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>returned</a> <a href='SkSurface_Reference#SkSurface'>if</a> <a href='SkSurface_Reference#SkSurface'>all</a> <a href='SkSurface_Reference#SkSurface'>parameters</a> <a href='SkSurface_Reference#SkSurface'>are</a> <a href='SkSurface_Reference#SkSurface'>valid</a>.
Valid parameters include:
info dimensions are greater than zero;
info contains <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>and</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>supported</a> <a href='SkImageInfo_Reference#SkAlphaType'>by</a>  <a href='undocumented#Raster_Surface'>raster surface</a>;
<a href='#SkSurface_MakeRaster_rowBytes'>rowBytes</a> <a href='#SkSurface_MakeRaster_rowBytes'>is</a> <a href='#SkSurface_MakeRaster_rowBytes'>large</a> <a href='#SkSurface_MakeRaster_rowBytes'>enough</a> <a href='#SkSurface_MakeRaster_rowBytes'>to</a> <a href='#SkSurface_MakeRaster_rowBytes'>contain</a> <a href='#SkSurface_MakeRaster_rowBytes'>info</a> <a href='#SkSurface_MakeRaster_rowBytes'>width</a> <a href='#SkSurface_MakeRaster_rowBytes'>pixels</a> <a href='#SkSurface_MakeRaster_rowBytes'>of</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkColorType'>or</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>zero</a>.

If <a href='#SkSurface_MakeRaster_rowBytes'>rowBytes</a> <a href='#SkSurface_MakeRaster_rowBytes'>is</a> <a href='#SkSurface_MakeRaster_rowBytes'>not</a> <a href='#SkSurface_MakeRaster_rowBytes'>zero</a>, <a href='#SkSurface_MakeRaster_rowBytes'>subsequent</a> <a href='#SkSurface_MakeRaster_rowBytes'>images</a> <a href='#SkSurface_MakeRaster_rowBytes'>returned</a> <a href='#SkSurface_MakeRaster_rowBytes'>by</a> <a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a>()
have the same <a href='#SkSurface_MakeRaster_rowBytes'>rowBytes</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRaster_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>,</td>
  </tr>
</table>

of  <a href='undocumented#Raster_Surface'>raster surface</a>; width and height must be greater than zero

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRaster_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>interval from one <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>row</a> <a href='SkSurface_Reference#SkSurface'>to</a> <a href='SkSurface_Reference#SkSurface'>the</a> <a href='SkSurface_Reference#SkSurface'>next</a>; <a href='SkSurface_Reference#SkSurface'>may</a> <a href='SkSurface_Reference#SkSurface'>be</a> <a href='SkSurface_Reference#SkSurface'>zero</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRaster_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>independent</a> <a href='undocumented#Device'>fonts</a>;</td>
  </tr>
</table>

may be nullptr

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>if</a> <a href='SkSurface_Reference#SkSurface'>all</a> <a href='SkSurface_Reference#SkSurface'>parameters</a> <a href='SkSurface_Reference#SkSurface'>are</a> <a href='SkSurface_Reference#SkSurface'>valid</a>; <a href='SkSurface_Reference#SkSurface'>otherwise</a>, <a href='SkSurface_Reference#SkSurface'>nullptr</a>

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
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&<a href='SkSurface_Reference#SkSurface'>gt</a>; <a href='#SkSurface_MakeRaster'>MakeRaster</a>(<a href='#SkSurface_MakeRaster'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>imageInfo</a>,
                                   <a href='SkImageInfo_Reference#SkImageInfo'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>props</a> = <a href='undocumented#SkSurfaceProps'>nullptr</a>)
</pre>

Allocates raster <a href='SkSurface_Reference#SkSurface'>SkSurface</a>. <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>returned</a> <a href='SkCanvas_Reference#SkCanvas'>by</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>draws</a> <a href='SkSurface_Reference#SkSurface'>directly</a> <a href='SkSurface_Reference#SkSurface'>into</a> <a href='SkSurface_Reference#SkSurface'>pixels</a>.
Allocates and zeroes <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a>. <a href='undocumented#Pixel'>Pixel</a> <a href='undocumented#Pixel'>memory</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>is</a> <a href='#SkSurface_MakeRaster_2_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_height'>height()</a> <a href='#SkImageInfo_height'>times</a>
<a href='#SkSurface_MakeRaster_2_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>().
<a href='undocumented#Pixel'>Pixel</a> <a href='undocumented#Pixel'>memory</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>deleted</a> <a href='undocumented#Pixel'>when</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>deleted</a>.

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>returned</a> <a href='SkSurface_Reference#SkSurface'>if</a> <a href='SkSurface_Reference#SkSurface'>all</a> <a href='SkSurface_Reference#SkSurface'>parameters</a> <a href='SkSurface_Reference#SkSurface'>are</a> <a href='SkSurface_Reference#SkSurface'>valid</a>.
Valid parameters include:
info dimensions are greater than zero;
info contains <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>and</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>supported</a> <a href='SkImageInfo_Reference#SkAlphaType'>by</a>  <a href='undocumented#Raster_Surface'>raster surface</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRaster_2_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>,</td>
  </tr>
</table>

of  <a href='undocumented#Raster_Surface'>raster surface</a>; width and height must be greater than zero

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRaster_2_props'><code><strong>props</strong></code></a></td>
    <td>LCD striping orientation and setting for <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>independent</a> <a href='undocumented#Device'>fonts</a>;</td>
  </tr>
</table>

may be nullptr

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>if</a> <a href='SkSurface_Reference#SkSurface'>all</a> <a href='SkSurface_Reference#SkSurface'>parameters</a> <a href='SkSurface_Reference#SkSurface'>are</a> <a href='SkSurface_Reference#SkSurface'>valid</a>; <a href='SkSurface_Reference#SkSurface'>otherwise</a>, <a href='SkSurface_Reference#SkSurface'>nullptr</a>

### Example

<div><fiddle-embed name="c6197d204ef9e4ccfb583242651fb2a7"></fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeRasterDirect'>MakeRasterDirect</a> <a href='#SkSurface_MakeRasterN32Premul'>MakeRasterN32Premul</a> <a href='#SkSurface_MakeRasterDirectReleaseProc'>MakeRasterDirectReleaseProc</a>

<a name='SkSurface_MakeRasterN32Premul'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&<a href='SkSurface_Reference#SkSurface'>gt</a>; <a href='#SkSurface_MakeRasterN32Premul'>MakeRasterN32Premul</a>(<a href='#SkSurface_MakeRasterN32Premul'>int</a> <a href='#SkSurface_MakeRasterN32Premul'>width</a>, <a href='#SkSurface_MakeRasterN32Premul'>int</a> <a href='#SkSurface_MakeRasterN32Premul'>height</a>,
                                            <a href='#SkSurface_MakeRasterN32Premul'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>surfaceProps</a> = <a href='undocumented#SkSurfaceProps'>nullptr</a>)
</pre>

Allocates raster <a href='SkSurface_Reference#SkSurface'>SkSurface</a>. <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>returned</a> <a href='SkCanvas_Reference#SkCanvas'>by</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>draws</a> <a href='SkSurface_Reference#SkSurface'>directly</a> <a href='SkSurface_Reference#SkSurface'>into</a> <a href='SkSurface_Reference#SkSurface'>pixels</a>.
Allocates and zeroes <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a>. <a href='undocumented#Pixel'>Pixel</a> <a href='undocumented#Pixel'>memory</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>is</a> <a href='#SkSurface_MakeRasterN32Premul_height'>height</a> <a href='#SkSurface_MakeRasterN32Premul_height'>times</a> <a href='#SkSurface_MakeRasterN32Premul_width'>width</a> <a href='#SkSurface_MakeRasterN32Premul_width'>times</a>
four. <a href='undocumented#Pixel'>Pixel</a> <a href='undocumented#Pixel'>memory</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>deleted</a> <a href='undocumented#Pixel'>when</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>deleted</a>.

Internally, sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>to</a> <a href='#SkSurface_MakeRasterN32Premul_width'>width</a>, <a href='#SkSurface_MakeRasterN32Premul_height'>height</a>, <a href='#SkSurface_MakeRasterN32Premul_height'>native</a>  <a href='SkImageInfo_Reference#Color_Type'>color type</a>, <a href='SkColor_Reference#Color'>and</a>
<a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>.

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>returned</a> <a href='SkSurface_Reference#SkSurface'>if</a> <a href='#SkSurface_MakeRasterN32Premul_width'>width</a> <a href='#SkSurface_MakeRasterN32Premul_width'>and</a> <a href='#SkSurface_MakeRasterN32Premul_height'>height</a> <a href='#SkSurface_MakeRasterN32Premul_height'>are</a> <a href='#SkSurface_MakeRasterN32Premul_height'>greater</a> <a href='#SkSurface_MakeRasterN32Premul_height'>than</a> <a href='#SkSurface_MakeRasterN32Premul_height'>zero</a>.

Use to create <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>that</a> <a href='SkSurface_Reference#SkSurface'>matches</a> <a href='SkColor_Reference#SkPMColor'>SkPMColor</a>, <a href='SkColor_Reference#SkPMColor'>the</a> <a href='SkColor_Reference#SkPMColor'>native</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>arrangement</a> <a href='undocumented#Pixel'>on</a>
the platform. <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>drawn</a> <a href='SkSurface_Reference#SkSurface'>to</a> <a href='SkSurface_Reference#SkSurface'>output</a> <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>skips</a> <a href='undocumented#Device'>converting</a> <a href='undocumented#Device'>its</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>format</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRasterN32Premul_width'><code><strong>width</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>column</a> <a href='undocumented#Pixel'>count</a>; <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>greater</a> <a href='undocumented#Pixel'>than</a> <a href='undocumented#Pixel'>zero</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterN32Premul_height'><code><strong>height</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>count</a>; <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>greater</a> <a href='undocumented#Pixel'>than</a> <a href='undocumented#Pixel'>zero</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRasterN32Premul_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>independent</a></td>
  </tr>
</table>

fonts; may be nullptr

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>if</a> <a href='SkSurface_Reference#SkSurface'>all</a> <a href='SkSurface_Reference#SkSurface'>parameters</a> <a href='SkSurface_Reference#SkSurface'>are</a> <a href='SkSurface_Reference#SkSurface'>valid</a>; <a href='SkSurface_Reference#SkSurface'>otherwise</a>, <a href='SkSurface_Reference#SkSurface'>nullptr</a>

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
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&<a href='SkSurface_Reference#SkSurface'>gt</a>; <a href='#SkSurface_MakeFromBackendTexture'>MakeFromBackendTexture</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>,
                                               <a href='undocumented#GrContext'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& <a href='undocumented#GrBackendTexture'>backendTexture</a>,
                                               <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>origin</a>, <a href='undocumented#GrSurfaceOrigin'>int</a> <a href='undocumented#GrSurfaceOrigin'>sampleCnt</a>,
                                               <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>colorType</a>,
                                               <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&<a href='undocumented#SkColorSpace'>gt</a>; <a href='undocumented#SkColorSpace'>colorSpace</a>,
                                               <a href='undocumented#SkColorSpace'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>surfaceProps</a>)
</pre>

Wraps a GPU-backed <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>into</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>. <a href='SkSurface_Reference#SkSurface'>Caller</a> <a href='SkSurface_Reference#SkSurface'>must</a> <a href='SkSurface_Reference#SkSurface'>ensure</a> <a href='SkSurface_Reference#SkSurface'>the</a> <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>is</a>
valid for the lifetime of returned <a href='SkSurface_Reference#SkSurface'>SkSurface</a>. <a href='SkSurface_Reference#SkSurface'>If</a> <a href='#SkSurface_MakeFromBackendTexture_sampleCnt'>sampleCnt</a> <a href='#SkSurface_MakeFromBackendTexture_sampleCnt'>greater</a> <a href='#SkSurface_MakeFromBackendTexture_sampleCnt'>than</a> <a href='#SkSurface_MakeFromBackendTexture_sampleCnt'>zero</a>,
creates an intermediate MSAA <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>which</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>used</a> <a href='SkSurface_Reference#SkSurface'>for</a> <a href='SkSurface_Reference#SkSurface'>drawing</a> <a href='#SkSurface_MakeFromBackendTexture_backendTexture'>backendTexture</a>.

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>returned</a> <a href='SkSurface_Reference#SkSurface'>if</a> <a href='SkSurface_Reference#SkSurface'>all</a> <a href='SkSurface_Reference#SkSurface'>parameters</a> <a href='SkSurface_Reference#SkSurface'>are</a> <a href='SkSurface_Reference#SkSurface'>valid</a>. <a href='#SkSurface_MakeFromBackendTexture_backendTexture'>backendTexture</a> <a href='#SkSurface_MakeFromBackendTexture_backendTexture'>is</a> <a href='#SkSurface_MakeFromBackendTexture_backendTexture'>valid</a> <a href='#SkSurface_MakeFromBackendTexture_backendTexture'>if</a>
its <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>configuration</a> <a href='undocumented#Pixel'>agrees</a> <a href='undocumented#Pixel'>with</a> <a href='#SkSurface_MakeFromBackendTexture_colorSpace'>colorSpace</a> <a href='#SkSurface_MakeFromBackendTexture_colorSpace'>and</a> <a href='#SkSurface_MakeFromBackendTexture_context'>context</a>; <a href='#SkSurface_MakeFromBackendTexture_context'>for</a> <a href='#SkSurface_MakeFromBackendTexture_context'>instance</a>, <a href='#SkSurface_MakeFromBackendTexture_context'>if</a>
<a href='#SkSurface_MakeFromBackendTexture_backendTexture'>backendTexture</a> <a href='#SkSurface_MakeFromBackendTexture_backendTexture'>has</a> <a href='#SkSurface_MakeFromBackendTexture_backendTexture'>an</a> <a href='#SkSurface_MakeFromBackendTexture_backendTexture'>sRGB</a> <a href='#SkSurface_MakeFromBackendTexture_backendTexture'>configuration</a>, <a href='#SkSurface_MakeFromBackendTexture_backendTexture'>then</a> <a href='#SkSurface_MakeFromBackendTexture_context'>context</a> <a href='#SkSurface_MakeFromBackendTexture_context'>must</a> <a href='#SkSurface_MakeFromBackendTexture_context'>support</a> <a href='#SkSurface_MakeFromBackendTexture_context'>sRGB</a>,
and <a href='#SkSurface_MakeFromBackendTexture_colorSpace'>colorSpace</a> <a href='#SkSurface_MakeFromBackendTexture_colorSpace'>must</a> <a href='#SkSurface_MakeFromBackendTexture_colorSpace'>be</a> <a href='#SkSurface_MakeFromBackendTexture_colorSpace'>present</a>. <a href='#SkSurface_MakeFromBackendTexture_colorSpace'>Further</a>, <a href='#SkSurface_MakeFromBackendTexture_backendTexture'>backendTexture</a> <a href='#SkSurface_MakeFromBackendTexture_backendTexture'>width</a> <a href='#SkSurface_MakeFromBackendTexture_backendTexture'>and</a> <a href='#SkSurface_MakeFromBackendTexture_backendTexture'>height</a> <a href='#SkSurface_MakeFromBackendTexture_backendTexture'>must</a>
not exceed <a href='#SkSurface_MakeFromBackendTexture_context'>context</a> <a href='#SkSurface_MakeFromBackendTexture_context'>capabilities</a>, <a href='#SkSurface_MakeFromBackendTexture_context'>and</a> <a href='#SkSurface_MakeFromBackendTexture_context'>the</a> <a href='#SkSurface_MakeFromBackendTexture_context'>context</a> <a href='#SkSurface_MakeFromBackendTexture_context'>must</a> <a href='#SkSurface_MakeFromBackendTexture_context'>be</a> <a href='#SkSurface_MakeFromBackendTexture_context'>able</a> <a href='#SkSurface_MakeFromBackendTexture_context'>to</a> <a href='#SkSurface_MakeFromBackendTexture_context'>support</a>
back-end textures.

If SK_SUPPORT_GPU is defined as zero, has no effect and returns nullptr.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td><a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>residing</a> <a href='undocumented#Texture'>on</a> <a href='undocumented#Texture'>GPU</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_origin'><code><strong>origin</strong></code></a></td>
    <td>one of: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft_GrSurfaceOrigin</a>, <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTexture_sampleCnt'><code><strong>sampleCnt</strong></code></a></td>
    <td>samples per <a href='undocumented#Pixel'>pixel</a>, <a href='undocumented#Pixel'>or</a> 0 <a href='undocumented#Pixel'>to</a> <a href='undocumented#Pixel'>disable</a> <a href='undocumented#Pixel'>full</a> <a href='undocumented#Pixel'>scene</a> <a href='SkPaint_Reference#Anti_Alias'>anti-aliasing</a></td>
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
    <td>LCD striping orientation and setting for <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>independent</a></td>
  </tr>
</table>

fonts; may be nullptr

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>if</a> <a href='SkSurface_Reference#SkSurface'>all</a> <a href='SkSurface_Reference#SkSurface'>parameters</a> <a href='SkSurface_Reference#SkSurface'>are</a> <a href='SkSurface_Reference#SkSurface'>valid</a>; <a href='SkSurface_Reference#SkSurface'>otherwise</a>, <a href='SkSurface_Reference#SkSurface'>nullptr</a>

### Example

<div><fiddle-embed name="d3aec071998f871809f515e58abb1b0e" gpu="true" cpu="true"></fiddle-embed></div>

### See Also

<a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a> <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>

<a name='SkSurface_MakeFromBackendRenderTarget'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&<a href='SkSurface_Reference#SkSurface'>gt</a>; <a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>,
                                                   <a href='undocumented#GrContext'>const</a> <a href='undocumented#GrBackendRenderTarget'>GrBackendRenderTarget</a>& <a href='undocumented#GrBackendRenderTarget'>backendRenderTarget</a>,
                                                   <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>origin</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>colorType</a>,
                                                   <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&<a href='undocumented#SkColorSpace'>gt</a>; <a href='undocumented#SkColorSpace'>colorSpace</a>,
                                                   <a href='undocumented#SkColorSpace'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>surfaceProps</a>)
</pre>

Wraps a GPU-backed buffer into <a href='SkSurface_Reference#SkSurface'>SkSurface</a>. <a href='SkSurface_Reference#SkSurface'>Caller</a> <a href='SkSurface_Reference#SkSurface'>must</a> <a href='SkSurface_Reference#SkSurface'>ensure</a> <a href='#SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'>backendRenderTarget</a>
is valid for the lifetime of returned <a href='SkSurface_Reference#SkSurface'>SkSurface</a>.

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>returned</a> <a href='SkSurface_Reference#SkSurface'>if</a> <a href='SkSurface_Reference#SkSurface'>all</a> <a href='SkSurface_Reference#SkSurface'>parameters</a> <a href='SkSurface_Reference#SkSurface'>are</a> <a href='SkSurface_Reference#SkSurface'>valid</a>. <a href='#SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'>backendRenderTarget</a> <a href='#SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'>is</a> <a href='#SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'>valid</a> <a href='#SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'>if</a>
its <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>configuration</a> <a href='undocumented#Pixel'>agrees</a> <a href='undocumented#Pixel'>with</a> <a href='#SkSurface_MakeFromBackendRenderTarget_colorSpace'>colorSpace</a> <a href='#SkSurface_MakeFromBackendRenderTarget_colorSpace'>and</a> <a href='#SkSurface_MakeFromBackendRenderTarget_context'>context</a>; <a href='#SkSurface_MakeFromBackendRenderTarget_context'>for</a> <a href='#SkSurface_MakeFromBackendRenderTarget_context'>instance</a>, <a href='#SkSurface_MakeFromBackendRenderTarget_context'>if</a>
<a href='#SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'>backendRenderTarget</a> <a href='#SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'>has</a> <a href='#SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'>an</a> <a href='#SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'>sRGB</a> <a href='#SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'>configuration</a>, <a href='#SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'>then</a> <a href='#SkSurface_MakeFromBackendRenderTarget_context'>context</a> <a href='#SkSurface_MakeFromBackendRenderTarget_context'>must</a> <a href='#SkSurface_MakeFromBackendRenderTarget_context'>support</a> <a href='#SkSurface_MakeFromBackendRenderTarget_context'>sRGB</a>,
and <a href='#SkSurface_MakeFromBackendRenderTarget_colorSpace'>colorSpace</a> <a href='#SkSurface_MakeFromBackendRenderTarget_colorSpace'>must</a> <a href='#SkSurface_MakeFromBackendRenderTarget_colorSpace'>be</a> <a href='#SkSurface_MakeFromBackendRenderTarget_colorSpace'>present</a>. <a href='#SkSurface_MakeFromBackendRenderTarget_colorSpace'>Further</a>, <a href='#SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'>backendRenderTarget</a> <a href='#SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'>width</a> <a href='#SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'>and</a> <a href='#SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'>height</a> <a href='#SkSurface_MakeFromBackendRenderTarget_backendRenderTarget'>must</a>
not exceed <a href='#SkSurface_MakeFromBackendRenderTarget_context'>context</a> <a href='#SkSurface_MakeFromBackendRenderTarget_context'>capabilities</a>, <a href='#SkSurface_MakeFromBackendRenderTarget_context'>and</a> <a href='#SkSurface_MakeFromBackendRenderTarget_context'>the</a> <a href='#SkSurface_MakeFromBackendRenderTarget_context'>context</a> <a href='#SkSurface_MakeFromBackendRenderTarget_context'>must</a> <a href='#SkSurface_MakeFromBackendRenderTarget_context'>be</a> <a href='#SkSurface_MakeFromBackendRenderTarget_context'>able</a> <a href='#SkSurface_MakeFromBackendRenderTarget_context'>to</a> <a href='#SkSurface_MakeFromBackendRenderTarget_context'>support</a>
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
    <td>LCD striping orientation and setting for <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>independent</a></td>
  </tr>
</table>

fonts; may be nullptr

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>if</a> <a href='SkSurface_Reference#SkSurface'>all</a> <a href='SkSurface_Reference#SkSurface'>parameters</a> <a href='SkSurface_Reference#SkSurface'>are</a> <a href='SkSurface_Reference#SkSurface'>valid</a>; <a href='SkSurface_Reference#SkSurface'>otherwise</a>, <a href='SkSurface_Reference#SkSurface'>nullptr</a>

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
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&<a href='SkSurface_Reference#SkSurface'>gt</a>; <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget'>MakeFromBackendTextureAsRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>,
                                            <a href='undocumented#GrContext'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& <a href='undocumented#GrBackendTexture'>backendTexture</a>,
                                            <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>origin</a>, <a href='undocumented#GrSurfaceOrigin'>int</a> <a href='undocumented#GrSurfaceOrigin'>sampleCnt</a>,
                                            <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>colorType</a>, <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&<a href='undocumented#SkColorSpace'>gt</a>; <a href='undocumented#SkColorSpace'>colorSpace</a>,
                                            <a href='undocumented#SkColorSpace'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>surfaceProps</a>)
</pre>

Wraps a GPU-backed <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>into</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>. <a href='SkSurface_Reference#SkSurface'>Caller</a> <a href='SkSurface_Reference#SkSurface'>must</a> <a href='SkSurface_Reference#SkSurface'>ensure</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>backendTexture</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>is</a>
valid for the lifetime of returned <a href='SkSurface_Reference#SkSurface'>SkSurface</a>. <a href='SkSurface_Reference#SkSurface'>If</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_sampleCnt'>sampleCnt</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_sampleCnt'>greater</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_sampleCnt'>than</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_sampleCnt'>zero</a>,
creates an intermediate MSAA <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>which</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>used</a> <a href='SkSurface_Reference#SkSurface'>for</a> <a href='SkSurface_Reference#SkSurface'>drawing</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>backendTexture</a>.

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>returned</a> <a href='SkSurface_Reference#SkSurface'>if</a> <a href='SkSurface_Reference#SkSurface'>all</a> <a href='SkSurface_Reference#SkSurface'>parameters</a> <a href='SkSurface_Reference#SkSurface'>are</a> <a href='SkSurface_Reference#SkSurface'>valid</a>. <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>backendTexture</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>is</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>valid</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>if</a>
its <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>configuration</a> <a href='undocumented#Pixel'>agrees</a> <a href='undocumented#Pixel'>with</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_colorSpace'>colorSpace</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_colorSpace'>and</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_context'>context</a>; <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_context'>for</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_context'>instance</a>, <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_context'>if</a>
<a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>backendTexture</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>has</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>an</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>sRGB</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>configuration</a>, <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>then</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_context'>context</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_context'>must</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_context'>support</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_context'>sRGB</a>,
and <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_colorSpace'>colorSpace</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_colorSpace'>must</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_colorSpace'>be</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_colorSpace'>present</a>. <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_colorSpace'>Further</a>, <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>backendTexture</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>width</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>and</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>height</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'>must</a>
not exceed <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_context'>context</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget_context'>capabilities</a>.

Returned <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>available</a> <a href='SkSurface_Reference#SkSurface'>only</a> <a href='SkSurface_Reference#SkSurface'>for</a> <a href='SkSurface_Reference#SkSurface'>drawing</a> <a href='SkSurface_Reference#SkSurface'>into</a>, <a href='SkSurface_Reference#SkSurface'>and</a> <a href='SkSurface_Reference#SkSurface'>cannot</a> <a href='SkSurface_Reference#SkSurface'>generate</a> <a href='SkSurface_Reference#SkSurface'>an</a>
<a href='SkImage_Reference#SkImage'>SkImage</a>.

If SK_SUPPORT_GPU is defined as zero, has no effect and returns nullptr.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td><a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>residing</a> <a href='undocumented#Texture'>on</a> <a href='undocumented#Texture'>GPU</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_origin'><code><strong>origin</strong></code></a></td>
    <td>one of: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft_GrSurfaceOrigin</a>, <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeFromBackendTextureAsRenderTarget_sampleCnt'><code><strong>sampleCnt</strong></code></a></td>
    <td>samples per <a href='undocumented#Pixel'>pixel</a>, <a href='undocumented#Pixel'>or</a> 0 <a href='undocumented#Pixel'>to</a> <a href='undocumented#Pixel'>disable</a> <a href='undocumented#Pixel'>full</a> <a href='undocumented#Pixel'>scene</a> <a href='SkPaint_Reference#Anti_Alias'>anti-aliasing</a></td>
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
    <td>LCD striping orientation and setting for <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>independent</a></td>
  </tr>
</table>

fonts; may be nullptr

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>if</a> <a href='SkSurface_Reference#SkSurface'>all</a> <a href='SkSurface_Reference#SkSurface'>parameters</a> <a href='SkSurface_Reference#SkSurface'>are</a> <a href='SkSurface_Reference#SkSurface'>valid</a>; <a href='SkSurface_Reference#SkSurface'>otherwise</a>, <a href='SkSurface_Reference#SkSurface'>nullptr</a>

### Example

<div><fiddle-embed name="5e87093b9cbe95124ae14cbe77091eb7" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a> <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>

<a name='SkSurface_MakeRenderTarget'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&<a href='SkSurface_Reference#SkSurface'>gt</a>; <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>, <a href='undocumented#SkBudgeted'>SkBudgeted</a> <a href='undocumented#SkBudgeted'>budgeted</a>,
                                         <a href='undocumented#SkBudgeted'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>imageInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>sampleCount</a>,
                                         <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>surfaceOrigin</a>,
                                         <a href='undocumented#GrSurfaceOrigin'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>surfaceProps</a>,
                                         <a href='undocumented#SkSurfaceProps'>bool</a> <a href='undocumented#SkSurfaceProps'>shouldCreateWithMips</a> = <a href='undocumented#SkSurfaceProps'>false</a>)
</pre>

Returns <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>on</a> <a href='SkSurface_Reference#SkSurface'>GPU</a> <a href='SkSurface_Reference#SkSurface'>indicated</a> <a href='SkSurface_Reference#SkSurface'>by</a> <a href='#SkSurface_MakeRenderTarget_context'>context</a>. <a href='#SkSurface_MakeRenderTarget_context'>Allocates</a> <a href='#SkSurface_MakeRenderTarget_context'>memory</a> <a href='#SkSurface_MakeRenderTarget_context'>for</a>
pixels, based on the width, height, and <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>in</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>.  <a href='#SkSurface_MakeRenderTarget_budgeted'>budgeted</a>
selects whether allocation for pixels is tracked by <a href='#SkSurface_MakeRenderTarget_context'>context</a>. <a href='#SkSurface_MakeRenderTarget_imageInfo'>imageInfo</a>
describes the <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>format</a> <a href='undocumented#Pixel'>in</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkColorType'>and</a> <a href='SkImageInfo_Reference#SkColorType'>transparency</a> <a href='SkImageInfo_Reference#SkColorType'>in</a>
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>and</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>matching</a> <a href='SkColor_Reference#Color'>in</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

<a href='#SkSurface_MakeRenderTarget_sampleCount'>sampleCount</a> <a href='#SkSurface_MakeRenderTarget_sampleCount'>requests</a> <a href='#SkSurface_MakeRenderTarget_sampleCount'>the</a> <a href='#SkSurface_MakeRenderTarget_sampleCount'>number</a> <a href='#SkSurface_MakeRenderTarget_sampleCount'>of</a> <a href='#SkSurface_MakeRenderTarget_sampleCount'>samples</a> <a href='#SkSurface_MakeRenderTarget_sampleCount'>per</a> <a href='undocumented#Pixel'>pixel</a>.
Pass zero to disable  <a href='undocumented#Multi_Sample_Anti_Aliasing'>multi-sample anti-aliasing</a>.  The request is rounded
up to the next supported count, or rounded down if it is larger than the
maximum supported count.

<a href='#SkSurface_MakeRenderTarget_surfaceOrigin'>surfaceOrigin</a> <a href='#SkSurface_MakeRenderTarget_surfaceOrigin'>pins</a> <a href='#SkSurface_MakeRenderTarget_surfaceOrigin'>either</a> <a href='#SkSurface_MakeRenderTarget_surfaceOrigin'>the</a> <a href='#SkSurface_MakeRenderTarget_surfaceOrigin'>top-left</a> <a href='#SkSurface_MakeRenderTarget_surfaceOrigin'>or</a> <a href='#SkSurface_MakeRenderTarget_surfaceOrigin'>the</a> <a href='#SkSurface_MakeRenderTarget_surfaceOrigin'>bottom-left</a> <a href='#SkSurface_MakeRenderTarget_surfaceOrigin'>corner</a> <a href='#SkSurface_MakeRenderTarget_surfaceOrigin'>to</a> <a href='#SkSurface_MakeRenderTarget_surfaceOrigin'>the</a> <a href='#SkSurface_MakeRenderTarget_surfaceOrigin'>origin</a>.

<a href='#SkSurface_MakeRenderTarget_shouldCreateWithMips'>shouldCreateWithMips</a> <a href='#SkSurface_MakeRenderTarget_shouldCreateWithMips'>hints</a> <a href='#SkSurface_MakeRenderTarget_shouldCreateWithMips'>that</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>returned</a> <a href='SkImage_Reference#SkImage'>by</a> <a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a>() <a href='#SkSurface_makeImageSnapshot'>is</a>  <a href='undocumented#Mip_Map'>mip map</a>.

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
    <td>samples per <a href='undocumented#Pixel'>pixel</a>, <a href='undocumented#Pixel'>or</a> 0 <a href='undocumented#Pixel'>to</a> <a href='undocumented#Pixel'>disable</a> <a href='undocumented#Pixel'>full</a> <a href='undocumented#Pixel'>scene</a> <a href='SkPaint_Reference#Anti_Alias'>anti-aliasing</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_surfaceOrigin'><code><strong>surfaceOrigin</strong></code></a></td>
    <td>one of: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft_GrSurfaceOrigin</a>, <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_surfaceProps'><code><strong>surfaceProps</strong></code></a></td>
    <td>LCD striping orientation and setting for <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>independent</a></td>
  </tr>
</table>

fonts; may be nullptr

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeRenderTarget_shouldCreateWithMips'><code><strong>shouldCreateWithMips</strong></code></a></td>
    <td>hint that <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>will</a> <a href='SkSurface_Reference#SkSurface'>host</a>  <a href='undocumented#Mip_Map'>mip map</a> <a href='SkSurface_Reference#SkSurface'>images</a></td>
  </tr>
</table>

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>if</a> <a href='SkSurface_Reference#SkSurface'>all</a> <a href='SkSurface_Reference#SkSurface'>parameters</a> <a href='SkSurface_Reference#SkSurface'>are</a> <a href='SkSurface_Reference#SkSurface'>valid</a>; <a href='SkSurface_Reference#SkSurface'>otherwise</a>, <a href='SkSurface_Reference#SkSurface'>nullptr</a>

### Example

<div><fiddle-embed name="67b6609471a3f1ed0f4b1657004cdecb" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget'>MakeFromBackendTextureAsRenderTarget</a>

<a name='SkSurface_MakeRenderTarget_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&<a href='SkSurface_Reference#SkSurface'>gt</a>; <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>, <a href='undocumented#SkBudgeted'>SkBudgeted</a> <a href='undocumented#SkBudgeted'>budgeted</a>,
                                         <a href='undocumented#SkBudgeted'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>imageInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>sampleCount</a>,
                                         <a href='SkImageInfo_Reference#SkImageInfo'>const</a> <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>* <a href='undocumented#SkSurfaceProps'>props</a>)
</pre>

Returns <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>on</a> <a href='SkSurface_Reference#SkSurface'>GPU</a> <a href='SkSurface_Reference#SkSurface'>indicated</a> <a href='SkSurface_Reference#SkSurface'>by</a> <a href='#SkSurface_MakeRenderTarget_2_context'>context</a>. <a href='#SkSurface_MakeRenderTarget_2_context'>Allocates</a> <a href='#SkSurface_MakeRenderTarget_2_context'>memory</a> <a href='#SkSurface_MakeRenderTarget_2_context'>for</a>
pixels, based on the width, height, and <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>in</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>.  <a href='#SkSurface_MakeRenderTarget_2_budgeted'>budgeted</a>
selects whether allocation for pixels is tracked by <a href='#SkSurface_MakeRenderTarget_2_context'>context</a>. <a href='#SkSurface_MakeRenderTarget_2_imageInfo'>imageInfo</a>
describes the <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>format</a> <a href='undocumented#Pixel'>in</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkColorType'>and</a> <a href='SkImageInfo_Reference#SkColorType'>transparency</a> <a href='SkImageInfo_Reference#SkColorType'>in</a>
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>and</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>matching</a> <a href='SkColor_Reference#Color'>in</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

<a href='#SkSurface_MakeRenderTarget_2_sampleCount'>sampleCount</a> <a href='#SkSurface_MakeRenderTarget_2_sampleCount'>requests</a> <a href='#SkSurface_MakeRenderTarget_2_sampleCount'>the</a> <a href='#SkSurface_MakeRenderTarget_2_sampleCount'>number</a> <a href='#SkSurface_MakeRenderTarget_2_sampleCount'>of</a> <a href='#SkSurface_MakeRenderTarget_2_sampleCount'>samples</a> <a href='#SkSurface_MakeRenderTarget_2_sampleCount'>per</a> <a href='undocumented#Pixel'>pixel</a>.
Pass zero to disable  <a href='undocumented#Multi_Sample_Anti_Aliasing'>multi-sample anti-aliasing</a>.  The request is rounded
up to the next supported count, or rounded down if it is larger than the
maximum supported count.

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>bottom-left</a> <a href='SkSurface_Reference#SkSurface'>corner</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>pinned</a> <a href='SkSurface_Reference#SkSurface'>to</a> <a href='SkSurface_Reference#SkSurface'>the</a> <a href='SkSurface_Reference#SkSurface'>origin</a>.

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
    <td>samples per <a href='undocumented#Pixel'>pixel</a>, <a href='undocumented#Pixel'>or</a> 0 <a href='undocumented#Pixel'>to</a> <a href='undocumented#Pixel'>disable</a>  <a href='undocumented#Multi_Sample_Anti_Aliasing'>multi-sample anti-aliasing</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeRenderTarget_2_props'><code><strong>props</strong></code></a></td>
    <td>LCD striping orientation and setting for <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>independent</a></td>
  </tr>
</table>

fonts; may be nullptr

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>if</a> <a href='SkSurface_Reference#SkSurface'>all</a> <a href='SkSurface_Reference#SkSurface'>parameters</a> <a href='SkSurface_Reference#SkSurface'>are</a> <a href='SkSurface_Reference#SkSurface'>valid</a>; <a href='SkSurface_Reference#SkSurface'>otherwise</a>, <a href='SkSurface_Reference#SkSurface'>nullptr</a>

### Example

<div><fiddle-embed name="640321e8ecfb3f9329f3bc6e1f02485f" gpu="true" cpu="true"><div><a href='SkPaint_Reference#LCD_Text'>LCD text</a> takes advantage of raster striping to improve resolution. Only one of
the four combinations is correct, depending on whether monitor LCD striping is
horizontal or vertical, and whether the order of the stripes is red blue green
or red green blue.
</div></fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget'>MakeFromBackendTextureAsRenderTarget</a>

<a name='SkSurface_MakeRenderTarget_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&<a href='SkSurface_Reference#SkSurface'>gt</a>; <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>, <a href='undocumented#SkBudgeted'>SkBudgeted</a> <a href='undocumented#SkBudgeted'>budgeted</a>,
                                         <a href='undocumented#SkBudgeted'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>imageInfo</a>)
</pre>

Returns <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>on</a> <a href='SkSurface_Reference#SkSurface'>GPU</a> <a href='SkSurface_Reference#SkSurface'>indicated</a> <a href='SkSurface_Reference#SkSurface'>by</a> <a href='#SkSurface_MakeRenderTarget_3_context'>context</a>. <a href='#SkSurface_MakeRenderTarget_3_context'>Allocates</a> <a href='#SkSurface_MakeRenderTarget_3_context'>memory</a> <a href='#SkSurface_MakeRenderTarget_3_context'>for</a>
pixels, based on the width, height, and <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>in</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>.  <a href='#SkSurface_MakeRenderTarget_3_budgeted'>budgeted</a>
selects whether allocation for pixels is tracked by <a href='#SkSurface_MakeRenderTarget_3_context'>context</a>. <a href='#SkSurface_MakeRenderTarget_3_imageInfo'>imageInfo</a>
describes the <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>format</a> <a href='undocumented#Pixel'>in</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkColorType'>and</a> <a href='SkImageInfo_Reference#SkColorType'>transparency</a> <a href='SkImageInfo_Reference#SkColorType'>in</a>
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>and</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>matching</a> <a href='SkColor_Reference#Color'>in</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>bottom-left</a> <a href='SkSurface_Reference#SkSurface'>corner</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>pinned</a> <a href='SkSurface_Reference#SkSurface'>to</a> <a href='SkSurface_Reference#SkSurface'>the</a> <a href='SkSurface_Reference#SkSurface'>origin</a>.

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

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>if</a> <a href='SkSurface_Reference#SkSurface'>all</a> <a href='SkSurface_Reference#SkSurface'>parameters</a> <a href='SkSurface_Reference#SkSurface'>are</a> <a href='SkSurface_Reference#SkSurface'>valid</a>; <a href='SkSurface_Reference#SkSurface'>otherwise</a>, <a href='SkSurface_Reference#SkSurface'>nullptr</a>

### Example

<div><fiddle-embed name="5c7629c15e9ac93f098335e72560fa2e" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget'>MakeFromBackendTextureAsRenderTarget</a>

<a name='SkSurface_MakeRenderTarget_4'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&<a href='SkSurface_Reference#SkSurface'>gt</a>; <a href='#SkSurface_MakeRenderTarget'>MakeRenderTarget</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>,
                                         <a href='undocumented#GrContext'>const</a> <a href='undocumented#SkSurfaceCharacterization'>SkSurfaceCharacterization</a>& <a href='undocumented#SkSurfaceCharacterization'>characterization</a>,
                                         <a href='undocumented#SkBudgeted'>SkBudgeted</a> <a href='undocumented#SkBudgeted'>budgeted</a>)
</pre>

Returns <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>on</a> <a href='SkSurface_Reference#SkSurface'>GPU</a> <a href='SkSurface_Reference#SkSurface'>indicated</a> <a href='SkSurface_Reference#SkSurface'>by</a> <a href='#SkSurface_MakeRenderTarget_4_context'>context</a> <a href='#SkSurface_MakeRenderTarget_4_context'>that</a> <a href='#SkSurface_MakeRenderTarget_4_context'>is</a> <a href='#SkSurface_MakeRenderTarget_4_context'>compatible</a> <a href='#SkSurface_MakeRenderTarget_4_context'>with</a> <a href='#SkSurface_MakeRenderTarget_4_context'>the</a> <a href='#SkSurface_MakeRenderTarget_4_context'>provided</a>
<a href='#SkSurface_MakeRenderTarget_4_characterization'>characterization</a>. <a href='#SkSurface_MakeRenderTarget_4_budgeted'>budgeted</a> <a href='#SkSurface_MakeRenderTarget_4_budgeted'>selects</a> <a href='#SkSurface_MakeRenderTarget_4_budgeted'>whether</a> <a href='#SkSurface_MakeRenderTarget_4_budgeted'>allocation</a> <a href='#SkSurface_MakeRenderTarget_4_budgeted'>for</a> <a href='#SkSurface_MakeRenderTarget_4_budgeted'>pixels</a> <a href='#SkSurface_MakeRenderTarget_4_budgeted'>is</a> <a href='#SkSurface_MakeRenderTarget_4_budgeted'>tracked</a> <a href='#SkSurface_MakeRenderTarget_4_budgeted'>by</a> <a href='#SkSurface_MakeRenderTarget_4_context'>context</a>.

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

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>if</a> <a href='SkSurface_Reference#SkSurface'>all</a> <a href='SkSurface_Reference#SkSurface'>parameters</a> <a href='SkSurface_Reference#SkSurface'>are</a> <a href='SkSurface_Reference#SkSurface'>valid</a>; <a href='SkSurface_Reference#SkSurface'>otherwise</a>, <a href='SkSurface_Reference#SkSurface'>nullptr</a>

### See Also

<a href='#SkSurface_MakeFromBackendRenderTarget'>MakeFromBackendRenderTarget</a> <a href='#SkSurface_MakeFromBackendTextureAsRenderTarget'>MakeFromBackendTextureAsRenderTarget</a>

<a name='SkSurface_MakeNull'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&<a href='SkSurface_Reference#SkSurface'>gt</a>; <a href='#SkSurface_MakeNull'>MakeNull</a>(<a href='#SkSurface_MakeNull'>int</a> <a href='#SkSurface_MakeNull'>width</a>, <a href='#SkSurface_MakeNull'>int</a> <a href='#SkSurface_MakeNull'>height</a>)
</pre>

Returns <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>without</a> <a href='SkSurface_Reference#SkSurface'>backing</a> <a href='SkSurface_Reference#SkSurface'>pixels</a>. <a href='SkSurface_Reference#SkSurface'>Drawing</a> <a href='SkSurface_Reference#SkSurface'>to</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>returned</a> <a href='SkCanvas_Reference#SkCanvas'>from</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>
has no effect. Calling <a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a>() <a href='#SkSurface_makeImageSnapshot'>on</a> <a href='#SkSurface_makeImageSnapshot'>returned</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>returns</a> <a href='SkSurface_Reference#SkSurface'>nullptr</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_MakeNull_width'><code><strong>width</strong></code></a></td>
    <td>one or greater</td>
  </tr>
  <tr>    <td><a name='SkSurface_MakeNull_height'><code><strong>height</strong></code></a></td>
    <td>one or greater</td>
  </tr>
</table>

### Return Value

<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>if</a> <a href='#SkSurface_MakeNull_width'>width</a> <a href='#SkSurface_MakeNull_width'>and</a> <a href='#SkSurface_MakeNull_height'>height</a> <a href='#SkSurface_MakeNull_height'>are</a> <a href='#SkSurface_MakeNull_height'>positive</a>; <a href='#SkSurface_MakeNull_height'>otherwise</a>, <a href='#SkSurface_MakeNull_height'>nullptr</a>

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
int <a href='#SkSurface_width'>width()</a> <a href='#SkSurface_width'>const</a>
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>count</a> <a href='undocumented#Pixel'>in</a> <a href='undocumented#Pixel'>each</a> <a href='undocumented#Pixel'>row</a>; <a href='undocumented#Pixel'>may</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>zero</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>greater</a>.

### Return Value

number of <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>columns</a>

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
int <a href='#SkSurface_height'>height()</a> <a href='#SkSurface_height'>const</a>
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>count</a>; <a href='undocumented#Pixel'>may</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>zero</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>greater</a>.

### Return Value

number of <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>rows</a>

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

Returns unique value identifying the content of <a href='SkSurface_Reference#SkSurface'>SkSurface</a>. <a href='SkSurface_Reference#SkSurface'>Returned</a> <a href='SkSurface_Reference#SkSurface'>value</a> <a href='SkSurface_Reference#SkSurface'>changes</a>
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

<a href='#SkSurface_ContentChangeMode'>ContentChangeMode</a> <a href='#SkSurface_ContentChangeMode'>members</a> <a href='#SkSurface_ContentChangeMode'>are</a> <a href='#SkSurface_ContentChangeMode'>parameters</a> <a href='#SkSurface_ContentChangeMode'>to</a> <a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSurface_kDiscard_ContentChangeMode'><code>SkSurface::kDiscard_ContentChangeMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#Line # discards surface on change ##</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Pass to <a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a> <a href='#SkSurface_notifyContentWillChange'>to</a> <a href='#SkSurface_notifyContentWillChange'>discard</a> <a href='SkSurface_Reference#Surface'>surface</a> <a href='SkSurface_Reference#Surface'>contents</a> <a href='SkSurface_Reference#Surface'>when</a>
<a href='SkSurface_Reference#Surface'>the</a> <a href='SkSurface_Reference#Surface'>surface</a> <a href='SkSurface_Reference#Surface'>is</a> <a href='SkSurface_Reference#Surface'>cleared</a> <a href='SkSurface_Reference#Surface'>or</a> <a href='SkSurface_Reference#Surface'>overwritten</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkSurface_kRetain_ContentChangeMode'><code>SkSurface::kRetain_ContentChangeMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#Line # preserves surface on change ##</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Pass to <a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a> <a href='#SkSurface_notifyContentWillChange'>when</a> <a href='#SkSurface_notifyContentWillChange'>to</a> <a href='#SkSurface_notifyContentWillChange'>preserve</a> <a href='SkSurface_Reference#Surface'>surface</a> <a href='SkSurface_Reference#Surface'>contents</a>.
<a href='SkSurface_Reference#Surface'>If</a> <a href='SkSurface_Reference#Surface'>a</a> <a href='SkSurface_Reference#Surface'>snapshot</a> <a href='SkSurface_Reference#Surface'>has</a> <a href='SkSurface_Reference#Surface'>been</a> <a href='SkSurface_Reference#Surface'>generated</a>, <a href='SkSurface_Reference#Surface'>this</a> <a href='SkSurface_Reference#Surface'>copies</a> <a href='SkSurface_Reference#Surface'>the</a> <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>contents</a>.
</td>
  </tr>
</table>

### See Also

<a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a> <a href='#SkSurface_generationID'>generationID</a>

<a name='Miscellaneous'></a>

<a name='SkSurface_notifyContentWillChange'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkSurface_notifyContentWillChange'>notifyContentWillChange</a>(<a href='#SkSurface_ContentChangeMode'>ContentChangeMode</a> <a href='#SkSurface_ContentChangeMode'>mode</a>)
</pre>

Notifies that <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>contents</a> <a href='SkSurface_Reference#Surface'>will</a> <a href='SkSurface_Reference#Surface'>be</a> <a href='SkSurface_Reference#Surface'>changed</a> <a href='SkSurface_Reference#Surface'>by</a> <a href='SkSurface_Reference#Surface'>code</a> <a href='SkSurface_Reference#Surface'>outside</a> <a href='SkSurface_Reference#Surface'>of</a> <a href='SkSurface_Reference#Surface'>Skia</a>.
<a href='SkSurface_Reference#Surface'>Subsequent</a> <a href='SkSurface_Reference#Surface'>calls</a> <a href='SkSurface_Reference#Surface'>to</a> <a href='#SkSurface_generationID'>generationID</a> <a href='#SkSurface_generationID'>return</a> <a href='#SkSurface_generationID'>a</a> <a href='#SkSurface_generationID'>different</a> <a href='#SkSurface_generationID'>value</a>.

<a href='#SkSurface_notifyContentWillChange_mode'>mode</a> <a href='#SkSurface_notifyContentWillChange_mode'>is</a> <a href='#SkSurface_notifyContentWillChange_mode'>normally</a> <a href='#SkSurface_notifyContentWillChange_mode'>passed</a> <a href='#SkSurface_notifyContentWillChange_mode'>as</a> <a href='#SkSurface_kRetain_ContentChangeMode'>kRetain_ContentChangeMode</a>.

Private: Can we deprecate this?

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

    <a href='#SkSurface_kDiscardWrite_BackendHandleAccess'>static</a> <a href='#SkSurface_kDiscardWrite_BackendHandleAccess'>const</a> <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_kFlushRead_TextureHandleAccess'>kFlushRead_TextureHandleAccess</a> =
            <a href='#SkSurface_kFlushRead_BackendHandleAccess'>kFlushRead_BackendHandleAccess</a>;
    <a href='#SkSurface_kFlushRead_BackendHandleAccess'>static</a> <a href='#SkSurface_kFlushRead_BackendHandleAccess'>const</a> <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_kFlushWrite_TextureHandleAccess'>kFlushWrite_TextureHandleAccess</a> =
            <a href='#SkSurface_kFlushWrite_BackendHandleAccess'>kFlushWrite_BackendHandleAccess</a>;
    <a href='#SkSurface_kFlushWrite_BackendHandleAccess'>static</a> <a href='#SkSurface_kFlushWrite_BackendHandleAccess'>const</a> <a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_kDiscardWrite_TextureHandleAccess'>kDiscardWrite_TextureHandleAccess</a> =
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

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='#SkSurface_getBackendTexture'>getBackendTexture</a>(<a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_BackendHandleAccess'>backendHandleAccess</a>)
</pre>

Retrieves the back-end <a href='undocumented#Texture'>texture</a>. <a href='undocumented#Texture'>If</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>has</a> <a href='SkSurface_Reference#SkSurface'>no</a> <a href='SkSurface_Reference#SkSurface'>back-end</a> <a href='undocumented#Texture'>texture</a>, <a href='undocumented#Texture'>an</a> <a href='undocumented#Texture'>invalid</a>
object is returned. Call <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>::<a href='#GrBackendTexture_isValid'>isValid</a> <a href='#GrBackendTexture_isValid'>to</a> <a href='#GrBackendTexture_isValid'>determine</a> <a href='#GrBackendTexture_isValid'>if</a> <a href='#GrBackendTexture_isValid'>the</a> <a href='#GrBackendTexture_isValid'>result</a>
is valid.

The returned <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='undocumented#GrBackendTexture'>should</a> <a href='undocumented#GrBackendTexture'>be</a> <a href='undocumented#GrBackendTexture'>discarded</a> <a href='undocumented#GrBackendTexture'>if</a> <a href='undocumented#GrBackendTexture'>the</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>drawn</a> <a href='SkSurface_Reference#SkSurface'>to</a> <a href='SkSurface_Reference#SkSurface'>or</a> <a href='SkSurface_Reference#SkSurface'>deleted</a>.

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
<a href='undocumented#GrBackendRenderTarget'>GrBackendRenderTarget</a> <a href='#SkSurface_getBackendRenderTarget'>getBackendRenderTarget</a>(<a href='#SkSurface_BackendHandleAccess'>BackendHandleAccess</a> <a href='#SkSurface_BackendHandleAccess'>backendHandleAccess</a>)
</pre>

Retrieves the back-end  <a href='undocumented#Render_Target'>render target</a>. If <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>has</a> <a href='SkSurface_Reference#SkSurface'>no</a> <a href='SkSurface_Reference#SkSurface'>back-end</a>  <a href='undocumented#Render_Target'>render target</a>, <a href='SkSurface_Reference#SkSurface'>an</a> <a href='SkSurface_Reference#SkSurface'>invalid</a>
object is returned. Call <a href='undocumented#GrBackendRenderTarget'>GrBackendRenderTarget</a>::<a href='#GrBackendRenderTarget_isValid'>isValid</a> <a href='#GrBackendRenderTarget_isValid'>to</a> <a href='#GrBackendRenderTarget_isValid'>determine</a> <a href='#GrBackendRenderTarget_isValid'>if</a> <a href='#GrBackendRenderTarget_isValid'>the</a> <a href='#GrBackendRenderTarget_isValid'>result</a>
is valid.

The returned <a href='undocumented#GrBackendRenderTarget'>GrBackendRenderTarget</a> <a href='undocumented#GrBackendRenderTarget'>should</a> <a href='undocumented#GrBackendRenderTarget'>be</a> <a href='undocumented#GrBackendRenderTarget'>discarded</a> <a href='undocumented#GrBackendRenderTarget'>if</a> <a href='undocumented#GrBackendRenderTarget'>the</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>drawn</a> <a href='SkSurface_Reference#SkSurface'>to</a>
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

Returns <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>that</a> <a href='SkCanvas_Reference#SkCanvas'>draws</a> <a href='SkCanvas_Reference#SkCanvas'>into</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>. <a href='SkSurface_Reference#SkSurface'>Subsequent</a> <a href='SkSurface_Reference#SkSurface'>calls</a> <a href='SkSurface_Reference#SkSurface'>return</a> <a href='SkSurface_Reference#SkSurface'>the</a> <a href='SkSurface_Reference#SkSurface'>same</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>.
<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>returned</a> <a href='SkCanvas_Reference#SkCanvas'>is</a> <a href='SkCanvas_Reference#SkCanvas'>managed</a> <a href='SkCanvas_Reference#SkCanvas'>and</a> <a href='SkCanvas_Reference#SkCanvas'>owned</a> <a href='SkCanvas_Reference#SkCanvas'>by</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>, <a href='SkSurface_Reference#SkSurface'>and</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>deleted</a> <a href='SkSurface_Reference#SkSurface'>when</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>
is deleted.

### Return Value

drawing <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>for</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>

### Example

<div><fiddle-embed name="33d0c5ad5a4810e533ae1010e29f8b75"></fiddle-embed></div>

### See Also

<a href='#SkSurface_makeSurface'>makeSurface</a> <a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a> <a href='#SkSurface_draw'>draw</a>

<a name='SkSurface_makeSurface'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkSurface_Reference#SkSurface'>SkSurface</a>&<a href='SkSurface_Reference#SkSurface'>gt</a>; <a href='#SkSurface_makeSurface'>makeSurface</a>(<a href='#SkSurface_makeSurface'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>imageInfo</a>)
</pre>

Returns a compatible <a href='SkSurface_Reference#SkSurface'>SkSurface</a>, <a href='SkSurface_Reference#SkSurface'>or</a> <a href='SkSurface_Reference#SkSurface'>nullptr</a>. <a href='SkSurface_Reference#SkSurface'>Returned</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>contains</a>
the same raster, GPU, or null properties as the original. Returned <a href='SkSurface_Reference#SkSurface'>SkSurface</a>
does not share the same pixels.

Returns nullptr if <a href='#SkSurface_makeSurface_imageInfo'>imageInfo</a> <a href='#SkSurface_makeSurface_imageInfo'>width</a> <a href='#SkSurface_makeSurface_imageInfo'>or</a> <a href='#SkSurface_makeSurface_imageInfo'>height</a> <a href='#SkSurface_makeSurface_imageInfo'>are</a> <a href='#SkSurface_makeSurface_imageInfo'>zero</a>, <a href='#SkSurface_makeSurface_imageInfo'>or</a> <a href='#SkSurface_makeSurface_imageInfo'>if</a> <a href='#SkSurface_makeSurface_imageInfo'>imageInfo</a>
is incompatible with <a href='SkSurface_Reference#SkSurface'>SkSurface</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_makeSurface_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>,</td>
  </tr>
</table>

of <a href='SkSurface_Reference#SkSurface'>SkSurface</a>; <a href='SkSurface_Reference#SkSurface'>width</a> <a href='SkSurface_Reference#SkSurface'>and</a> <a href='SkSurface_Reference#SkSurface'>height</a> <a href='SkSurface_Reference#SkSurface'>must</a> <a href='SkSurface_Reference#SkSurface'>be</a> <a href='SkSurface_Reference#SkSurface'>greater</a> <a href='SkSurface_Reference#SkSurface'>than</a> <a href='SkSurface_Reference#SkSurface'>zero</a>

### Return Value

compatible <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>or</a> <a href='SkSurface_Reference#SkSurface'>nullptr</a>

### Example

<div><fiddle-embed name="a9889b519a26896b900da0444e423c61"></fiddle-embed></div>

### See Also

<a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a> <a href='#SkSurface_getCanvas'>getCanvas</a> <a href='#SkSurface_draw'>draw</a>

<a name='SkSurface_makeImageSnapshot'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkSurface_makeImageSnapshot'>makeImageSnapshot</a>()
</pre>

Returns <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>capturing</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>contents</a>. <a href='SkSurface_Reference#SkSurface'>Subsequent</a> <a href='SkSurface_Reference#SkSurface'>drawing</a> <a href='SkSurface_Reference#SkSurface'>to</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>contents</a>
are not captured. <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>allocation</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>accounted</a> <a href='SkImage_Reference#SkImage'>for</a> <a href='SkImage_Reference#SkImage'>if</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>was</a> <a href='SkSurface_Reference#SkSurface'>created</a> <a href='SkSurface_Reference#SkSurface'>with</a>
<a href='undocumented#SkBudgeted'>SkBudgeted</a>::<a href='#SkBudgeted_kYes'>kYes</a>.

### Return Value

<a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>initialized</a> <a href='SkImage_Reference#SkImage'>with</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>contents</a>

### Example

<div><fiddle-embed name="46f1fa0d95e590a64bed0140407ce5f7"></fiddle-embed></div>

### See Also

<a href='#SkSurface_draw'>draw</a> <a href='#SkSurface_getCanvas'>getCanvas</a>

<a name='Pixels'></a>

<a name='SkSurface_draw'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void draw(<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* <a href='SkCanvas_Reference#Canvas'>canvas</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>, <a href='undocumented#SkScalar'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>)
</pre>

Draws <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>contents</a> <a href='SkSurface_Reference#SkSurface'>to</a> <a href='#SkSurface_draw_canvas'>canvas</a>, <a href='#SkSurface_draw_canvas'>with</a> <a href='#SkSurface_draw_canvas'>its</a> <a href='#SkSurface_draw_canvas'>top-left</a> <a href='#SkSurface_draw_canvas'>corner</a> <a href='#SkSurface_draw_canvas'>at</a> (<a href='#SkSurface_draw_x'>x</a>, <a href='#SkSurface_draw_y'>y</a>).

If <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='#SkSurface_draw_paint'>paint</a> <a href='#SkSurface_draw_paint'>is</a> <a href='#SkSurface_draw_paint'>not</a> <a href='#SkSurface_draw_paint'>nullptr</a>, <a href='#SkSurface_draw_paint'>apply</a> <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,
<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='SkBlendMode_Reference#SkBlendMode'>and</a> <a href='undocumented#SkDrawLooper'>SkDrawLooper</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_draw_canvas'><code><strong>canvas</strong></code></a></td>
    <td><a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>drawn</a> <a href='SkCanvas_Reference#SkCanvas'>into</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_draw_x'><code><strong>x</strong></code></a></td>
    <td>horizontal offset in <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_draw_y'><code><strong>y</strong></code></a></td>
    <td>vertical offset in <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_draw_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>containing</a> <a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>, <a href='undocumented#SkColorFilter'>SkColorFilter</a>, <a href='undocumented#SkImageFilter'>SkImageFilter</a>,</td>
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

Copies <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a>, <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>bytes</a>, <a href='undocumented#Pixel'>and</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>to</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>, <a href='SkPixmap_Reference#SkPixmap'>if</a> <a href='SkPixmap_Reference#SkPixmap'>address</a>
is available, and returns true. If <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>not</a> <a href='undocumented#Pixel'>available</a>, <a href='undocumented#Pixel'>return</a>
false and leave <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='SkPixmap_Reference#SkPixmap'>unchanged</a>.

<a href='#SkSurface_peekPixels_pixmap'>pixmap</a> <a href='#SkSurface_peekPixels_pixmap'>contents</a> <a href='#SkSurface_peekPixels_pixmap'>become</a> <a href='#SkSurface_peekPixels_pixmap'>invalid</a> <a href='#SkSurface_peekPixels_pixmap'>on</a> <a href='#SkSurface_peekPixels_pixmap'>any</a> <a href='#SkSurface_peekPixels_pixmap'>future</a> <a href='#SkSurface_peekPixels_pixmap'>change</a> <a href='#SkSurface_peekPixels_pixmap'>to</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_peekPixels_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td>storage for <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>state</a> <a href='undocumented#Pixel'>if</a> <a href='undocumented#Pixel'>pixels</a> <a href='undocumented#Pixel'>are</a> <a href='undocumented#Pixel'>readable</a>; <a href='undocumented#Pixel'>otherwise</a>, <a href='undocumented#Pixel'>ignored</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>has</a> <a href='SkSurface_Reference#SkSurface'>direct</a> <a href='SkSurface_Reference#SkSurface'>access</a> <a href='SkSurface_Reference#SkSurface'>to</a> <a href='SkSurface_Reference#SkSurface'>pixels</a>

### Example

<div><fiddle-embed name="8c6184f22cfe068f021704cf92a147a1"></fiddle-embed></div>

### See Also

<a href='#SkSurface_readPixels'>readPixels</a> <a href='#SkSurface_writePixels'>writePixels</a>

<a name='SkSurface_readPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkSurface_readPixels'>readPixels</a>(<a href='#SkSurface_readPixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#SkPixmap'>dst</a>, <a href='SkPixmap_Reference#SkPixmap'>int</a> <a href='SkPixmap_Reference#SkPixmap'>srcX</a>, <a href='SkPixmap_Reference#SkPixmap'>int</a> <a href='SkPixmap_Reference#SkPixmap'>srcY</a>)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>pixels</a> <a href='SkRect_Reference#Rect'>to</a> <a href='#SkSurface_readPixels_dst'>dst</a>.

<a href='#SkSurface_readPixels_dst'>Source</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>corners</a> <a href='SkRect_Reference#Rect'>are</a> (<a href='#SkSurface_readPixels_srcX'>srcX</a>, <a href='#SkSurface_readPixels_srcY'>srcY</a>) <a href='#SkSurface_readPixels_srcY'>and</a> <a href='SkSurface_Reference#Surface'>Surface</a> (<a href='#SkSurface_width'>width()</a>, <a href='#SkSurface_height'>height()</a>).
<a href='#SkSurface_height'>Destination</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>corners</a> <a href='SkRect_Reference#Rect'>are</a> (0, 0) <a href='SkRect_Reference#Rect'>and</a> (<a href='#SkSurface_readPixels_dst'>dst</a>.<a href='#SkPixmap_width'>width()</a>, <a href='#SkSurface_readPixels_dst'>dst</a>.<a href='#SkPixmap_height'>height()</a>).
<a href='#SkPixmap_height'>Copies</a> <a href='#SkPixmap_height'>each</a> <a href='#SkPixmap_height'>readable</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>intersecting</a> <a href='undocumented#Pixel'>both</a> <a href='undocumented#Pixel'>rectangles</a>, <a href='undocumented#Pixel'>without</a> <a href='undocumented#Pixel'>scaling</a>,
<a href='undocumented#Pixel'>converting</a> <a href='undocumented#Pixel'>to</a> <a href='#SkSurface_readPixels_dst'>dst</a>.<a href='#SkPixmap_colorType'>colorType</a>() <a href='#SkPixmap_colorType'>and</a> <a href='#SkSurface_readPixels_dst'>dst</a>.<a href='#SkPixmap_alphaType'>alphaType</a>() <a href='#SkPixmap_alphaType'>if</a> <a href='#SkPixmap_alphaType'>required</a>.

<a href='#SkPixmap_alphaType'>Pixels</a> <a href='#SkPixmap_alphaType'>are</a> <a href='#SkPixmap_alphaType'>readable</a> <a href='#SkPixmap_alphaType'>when</a> <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>is</a> <a href='SkSurface_Reference#Surface'>raster</a>, <a href='SkSurface_Reference#Surface'>or</a> <a href='SkSurface_Reference#Surface'>backed</a> <a href='SkSurface_Reference#Surface'>by</a> <a href='SkSurface_Reference#Surface'>a</a> <a href='SkSurface_Reference#Surface'>GPU</a>.

<a href='SkSurface_Reference#Surface'>The</a> <a href='SkSurface_Reference#Surface'>destination</a>  <a href='undocumented#Pixel_Storage'>pixel storage</a> <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>allocated</a> <a href='undocumented#Pixel'>by</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>caller</a>.

<a href='undocumented#Pixel'>Pixel</a> <a href='undocumented#Pixel'>values</a> <a href='undocumented#Pixel'>are</a> <a href='undocumented#Pixel'>converted</a> <a href='undocumented#Pixel'>only</a> <a href='undocumented#Pixel'>if</a> <a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Image_Info_Color_Type'>and</a> <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>
<a href='#Image_Info_Alpha_Type'>do</a> <a href='#Image_Info_Alpha_Type'>not</a> <a href='#Image_Info_Alpha_Type'>match</a>. <a href='#Image_Info_Alpha_Type'>Only</a> <a href='#Image_Info_Alpha_Type'>pixels</a> <a href='#Image_Info_Alpha_Type'>within</a> <a href='#Image_Info_Alpha_Type'>both</a> <a href='#Image_Info_Alpha_Type'>source</a> <a href='#Image_Info_Alpha_Type'>and</a> <a href='#Image_Info_Alpha_Type'>destination</a> <a href='#Image_Info_Alpha_Type'>rectangles</a>
<a href='#Image_Info_Alpha_Type'>are</a> <a href='#Image_Info_Alpha_Type'>copied</a>. <a href='#SkSurface_readPixels_dst'>dst</a> <a href='#SkSurface_readPixels_dst'>contents</a> <a href='#SkSurface_readPixels_dst'>outside</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>intersection</a> <a href='SkRect_Reference#Rect'>are</a> <a href='SkRect_Reference#Rect'>unchanged</a>.

<a href='SkRect_Reference#Rect'>Pass</a> <a href='SkRect_Reference#Rect'>negative</a> <a href='SkRect_Reference#Rect'>values</a> <a href='SkRect_Reference#Rect'>for</a> <a href='#SkSurface_readPixels_srcX'>srcX</a> <a href='#SkSurface_readPixels_srcX'>or</a> <a href='#SkSurface_readPixels_srcY'>srcY</a> <a href='#SkSurface_readPixels_srcY'>to</a> <a href='#SkSurface_readPixels_srcY'>offset</a> <a href='#SkSurface_readPixels_srcY'>pixels</a> <a href='#SkSurface_readPixels_srcY'>across</a> <a href='#SkSurface_readPixels_srcY'>or</a> <a href='#SkSurface_readPixels_srcY'>down</a> <a href='#SkSurface_readPixels_srcY'>destination</a>.

<a href='#SkSurface_readPixels_srcY'>Does</a> <a href='#SkSurface_readPixels_srcY'>not</a> <a href='#SkSurface_readPixels_srcY'>copy</a>, <a href='#SkSurface_readPixels_srcY'>and</a> <a href='#SkSurface_readPixels_srcY'>returns</a> <a href='#SkSurface_readPixels_srcY'>false</a> <a href='#SkSurface_readPixels_srcY'>if</a>:

<table>  <tr>
    <td>Source and destination rectangles do not intersect.</td>
  </tr>  <tr>
    <td><a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='SkPixmap_Reference#Pixmap'>pixels</a> <a href='SkPixmap_Reference#Pixmap'>could</a> <a href='SkPixmap_Reference#Pixmap'>not</a> <a href='SkPixmap_Reference#Pixmap'>be</a> <a href='SkPixmap_Reference#Pixmap'>allocated</a>.</td>
  </tr>  <tr>
    <td><a href='#SkSurface_readPixels_dst'>dst</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() <a href='#SkPixmap_rowBytes'>is</a> <a href='#SkPixmap_rowBytes'>too</a> <a href='#SkPixmap_rowBytes'>small</a> <a href='#SkPixmap_rowBytes'>to</a> <a href='#SkPixmap_rowBytes'>contain</a> <a href='#SkPixmap_rowBytes'>one</a> <a href='#SkPixmap_rowBytes'>row</a> <a href='#SkPixmap_rowBytes'>of</a> <a href='#SkPixmap_rowBytes'>pixels</a>.</td>
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
bool <a href='#SkSurface_readPixels'>readPixels</a>(<a href='#SkSurface_readPixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>dstInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>dstPixels</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>dstRowBytes</a>, <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>srcX</a>, <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>srcY</a>)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>pixels</a> <a href='SkRect_Reference#Rect'>from</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>into</a> <a href='#SkSurface_readPixels_2_dstPixels'>dstPixels</a>.

<a href='#SkSurface_readPixels_2_dstPixels'>Source</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>corners</a> <a href='SkRect_Reference#Rect'>are</a> (<a href='#SkSurface_readPixels_2_srcX'>srcX</a>, <a href='#SkSurface_readPixels_2_srcY'>srcY</a>) <a href='#SkSurface_readPixels_2_srcY'>and</a> <a href='SkSurface_Reference#Surface'>Surface</a> (<a href='#SkSurface_width'>width()</a>, <a href='#SkSurface_height'>height()</a>).
<a href='#SkSurface_height'>Destination</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>corners</a> <a href='SkRect_Reference#Rect'>are</a> (0, 0) <a href='SkRect_Reference#Rect'>and</a> (<a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_width'>width()</a>, <a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_height'>height()</a>).
<a href='#SkImageInfo_height'>Copies</a> <a href='#SkImageInfo_height'>each</a> <a href='#SkImageInfo_height'>readable</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>intersecting</a> <a href='undocumented#Pixel'>both</a> <a href='undocumented#Pixel'>rectangles</a>, <a href='undocumented#Pixel'>without</a> <a href='undocumented#Pixel'>scaling</a>,
<a href='undocumented#Pixel'>converting</a> <a href='undocumented#Pixel'>to</a> <a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorType'>colorType</a>() <a href='#SkImageInfo_colorType'>and</a> <a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() <a href='#SkImageInfo_alphaType'>if</a> <a href='#SkImageInfo_alphaType'>required</a>.

<a href='#SkImageInfo_alphaType'>Pixels</a> <a href='#SkImageInfo_alphaType'>are</a> <a href='#SkImageInfo_alphaType'>readable</a> <a href='#SkImageInfo_alphaType'>when</a> <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>is</a> <a href='SkSurface_Reference#Surface'>raster</a>, <a href='SkSurface_Reference#Surface'>or</a> <a href='SkSurface_Reference#Surface'>backed</a> <a href='SkSurface_Reference#Surface'>by</a> <a href='SkSurface_Reference#Surface'>a</a> <a href='SkSurface_Reference#Surface'>GPU</a>.

<a href='SkSurface_Reference#Surface'>The</a> <a href='SkSurface_Reference#Surface'>destination</a>  <a href='undocumented#Pixel_Storage'>pixel storage</a> <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>allocated</a> <a href='undocumented#Pixel'>by</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>caller</a>.

<a href='undocumented#Pixel'>Pixel</a> <a href='undocumented#Pixel'>values</a> <a href='undocumented#Pixel'>are</a> <a href='undocumented#Pixel'>converted</a> <a href='undocumented#Pixel'>only</a> <a href='undocumented#Pixel'>if</a> <a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Image_Info_Color_Type'>and</a> <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>
<a href='#Image_Info_Alpha_Type'>do</a> <a href='#Image_Info_Alpha_Type'>not</a> <a href='#Image_Info_Alpha_Type'>match</a>. <a href='#Image_Info_Alpha_Type'>Only</a> <a href='#Image_Info_Alpha_Type'>pixels</a> <a href='#Image_Info_Alpha_Type'>within</a> <a href='#Image_Info_Alpha_Type'>both</a> <a href='#Image_Info_Alpha_Type'>source</a> <a href='#Image_Info_Alpha_Type'>and</a> <a href='#Image_Info_Alpha_Type'>destination</a> <a href='#Image_Info_Alpha_Type'>rectangles</a>
<a href='#Image_Info_Alpha_Type'>are</a> <a href='#Image_Info_Alpha_Type'>copied</a>. <a href='#SkSurface_readPixels_2_dstPixels'>dstPixels</a> <a href='#SkSurface_readPixels_2_dstPixels'>contents</a> <a href='#SkSurface_readPixels_2_dstPixels'>outside</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>intersection</a> <a href='SkRect_Reference#Rect'>are</a> <a href='SkRect_Reference#Rect'>unchanged</a>.

<a href='SkRect_Reference#Rect'>Pass</a> <a href='SkRect_Reference#Rect'>negative</a> <a href='SkRect_Reference#Rect'>values</a> <a href='SkRect_Reference#Rect'>for</a> <a href='#SkSurface_readPixels_2_srcX'>srcX</a> <a href='#SkSurface_readPixels_2_srcX'>or</a> <a href='#SkSurface_readPixels_2_srcY'>srcY</a> <a href='#SkSurface_readPixels_2_srcY'>to</a> <a href='#SkSurface_readPixels_2_srcY'>offset</a> <a href='#SkSurface_readPixels_2_srcY'>pixels</a> <a href='#SkSurface_readPixels_2_srcY'>across</a> <a href='#SkSurface_readPixels_2_srcY'>or</a> <a href='#SkSurface_readPixels_2_srcY'>down</a> <a href='#SkSurface_readPixels_2_srcY'>destination</a>.

<a href='#SkSurface_readPixels_2_srcY'>Does</a> <a href='#SkSurface_readPixels_2_srcY'>not</a> <a href='#SkSurface_readPixels_2_srcY'>copy</a>, <a href='#SkSurface_readPixels_2_srcY'>and</a> <a href='#SkSurface_readPixels_2_srcY'>returns</a> <a href='#SkSurface_readPixels_2_srcY'>false</a> <a href='#SkSurface_readPixels_2_srcY'>if</a>:

<table>  <tr>
    <td>Source and destination rectangles do not intersect.</td>
  </tr>  <tr>
    <td><a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>pixels</a> <a href='SkSurface_Reference#Surface'>could</a> <a href='SkSurface_Reference#Surface'>not</a> <a href='SkSurface_Reference#Surface'>be</a> <a href='SkSurface_Reference#Surface'>converted</a> <a href='SkSurface_Reference#Surface'>to</a> <a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorType'>colorType</a>() <a href='#SkImageInfo_colorType'>or</a> <a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>().</td>
  </tr>  <tr>
    <td><a href='#SkSurface_readPixels_2_dstRowBytes'>dstRowBytes</a> <a href='#SkSurface_readPixels_2_dstRowBytes'>is</a> <a href='#SkSurface_readPixels_2_dstRowBytes'>too</a> <a href='#SkSurface_readPixels_2_dstRowBytes'>small</a> <a href='#SkSurface_readPixels_2_dstRowBytes'>to</a> <a href='#SkSurface_readPixels_2_dstRowBytes'>contain</a> <a href='#SkSurface_readPixels_2_dstRowBytes'>one</a> <a href='#SkSurface_readPixels_2_dstRowBytes'>row</a> <a href='#SkSurface_readPixels_2_dstRowBytes'>of</a> <a href='#SkSurface_readPixels_2_dstRowBytes'>pixels</a>.</td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkSurface_readPixels_2_dstInfo'><code><strong>dstInfo</strong></code></a></td>
    <td>width, height, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Color_Type'>and</a> <a href='#Image_Info_Alpha_Type'>Alpha_Type</a> <a href='#Image_Info_Alpha_Type'>of</a> <a href='#SkSurface_readPixels_2_dstPixels'>dstPixels</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_2_dstPixels'><code><strong>dstPixels</strong></code></a></td>
    <td>storage for pixels; <a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_height'>height()</a> <a href='#SkImageInfo_height'>times</a> <a href='#SkSurface_readPixels_2_dstRowBytes'>dstRowBytes</a>, <a href='#SkSurface_readPixels_2_dstRowBytes'>or</a> <a href='#SkSurface_readPixels_2_dstRowBytes'>larger</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_readPixels_2_dstRowBytes'><code><strong>dstRowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Size'>one</a> <a href='undocumented#Size'>destination</a> <a href='undocumented#Size'>row</a>; <a href='#SkSurface_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_width'>width()</a> <a href='#SkImageInfo_width'>times</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Size'>or</a> <a href='undocumented#Size'>larger</a></td>
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

<div><fiddle-embed name="484d60dab5d846bf28c7a4d48892324a"><div>A black <a href='undocumented#Oval'>oval</a> <a href='undocumented#Oval'>drawn</a> <a href='undocumented#Oval'>on</a> <a href='undocumented#Oval'>a</a> <a href='undocumented#Oval'>red</a> <a href='undocumented#Oval'>background</a> <a href='undocumented#Oval'>provides</a> <a href='undocumented#Oval'>an</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>to</a> <a href='SkImage_Reference#Image'>copy</a>.
<a href='#SkSurface_readPixels'>readPixels</a> <a href='#SkSurface_readPixels'>copies</a> <a href='#SkSurface_readPixels'>one</a> <a href='#SkSurface_readPixels'>quarter</a> <a href='#SkSurface_readPixels'>of</a> <a href='#SkSurface_readPixels'>the</a> <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>into</a> <a href='SkSurface_Reference#Surface'>each</a> <a href='SkSurface_Reference#Surface'>of</a> <a href='SkSurface_Reference#Surface'>the</a> <a href='SkSurface_Reference#Surface'>four</a> <a href='SkSurface_Reference#Surface'>corners</a>.
<a href='SkSurface_Reference#Surface'>The</a> <a href='SkSurface_Reference#Surface'>copied</a> <a href='SkSurface_Reference#Surface'>quarter</a> <a href='undocumented#Oval'>ovals</a> <a href='undocumented#Oval'>overdraw</a> <a href='undocumented#Oval'>the</a> <a href='undocumented#Oval'>original</a> <a href='undocumented#Oval'>oval</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkSurface_peekPixels'>peekPixels</a> <a href='#SkSurface_writePixels'>writePixels</a>

<a name='SkSurface_readPixels_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkSurface_readPixels'>readPixels</a>(<a href='#SkSurface_readPixels'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#SkBitmap'>dst</a>, <a href='SkBitmap_Reference#SkBitmap'>int</a> <a href='SkBitmap_Reference#SkBitmap'>srcX</a>, <a href='SkBitmap_Reference#SkBitmap'>int</a> <a href='SkBitmap_Reference#SkBitmap'>srcY</a>)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>pixels</a> <a href='SkRect_Reference#Rect'>from</a> <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>into</a> <a href='SkBitmap_Reference#Bitmap'>bitmap</a>.

<a href='SkBitmap_Reference#Bitmap'>Source</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>corners</a> <a href='SkRect_Reference#Rect'>are</a> (<a href='#SkSurface_readPixels_3_srcX'>srcX</a>, <a href='#SkSurface_readPixels_3_srcY'>srcY</a>) <a href='#SkSurface_readPixels_3_srcY'>and</a> <a href='SkSurface_Reference#Surface'>Surface</a> (<a href='#SkSurface_width'>width()</a>, <a href='#SkSurface_height'>height()</a>).
<a href='#SkSurface_height'>Destination</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>corners</a> <a href='SkRect_Reference#Rect'>are</a> (0, 0) <a href='SkRect_Reference#Rect'>and</a> (<a href='SkBitmap_Reference#Bitmap'>bitmap</a>.<a href='#SkSurface_width'>width()</a>, <a href='SkBitmap_Reference#Bitmap'>bitmap</a>.<a href='#SkSurface_height'>height()</a>).
<a href='#SkSurface_height'>Copies</a> <a href='#SkSurface_height'>each</a> <a href='#SkSurface_height'>readable</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>intersecting</a> <a href='undocumented#Pixel'>both</a> <a href='undocumented#Pixel'>rectangles</a>, <a href='undocumented#Pixel'>without</a> <a href='undocumented#Pixel'>scaling</a>,
<a href='undocumented#Pixel'>converting</a> <a href='undocumented#Pixel'>to</a> <a href='#SkSurface_readPixels_3_dst'>dst</a>.<a href='#SkBitmap_colorType'>colorType</a>() <a href='#SkBitmap_colorType'>and</a> <a href='#SkSurface_readPixels_3_dst'>dst</a>.<a href='#SkBitmap_alphaType'>alphaType</a>() <a href='#SkBitmap_alphaType'>if</a> <a href='#SkBitmap_alphaType'>required</a>.

<a href='#SkBitmap_alphaType'>Pixels</a> <a href='#SkBitmap_alphaType'>are</a> <a href='#SkBitmap_alphaType'>readable</a> <a href='#SkBitmap_alphaType'>when</a> <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>is</a> <a href='SkSurface_Reference#Surface'>raster</a>, <a href='SkSurface_Reference#Surface'>or</a> <a href='SkSurface_Reference#Surface'>backed</a> <a href='SkSurface_Reference#Surface'>by</a> <a href='SkSurface_Reference#Surface'>a</a> <a href='SkSurface_Reference#Surface'>GPU</a>.

<a href='SkSurface_Reference#Surface'>The</a> <a href='SkSurface_Reference#Surface'>destination</a>  <a href='undocumented#Pixel_Storage'>pixel storage</a> <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>allocated</a> <a href='undocumented#Pixel'>by</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>caller</a>.

<a href='undocumented#Pixel'>Pixel</a> <a href='undocumented#Pixel'>values</a> <a href='undocumented#Pixel'>are</a> <a href='undocumented#Pixel'>converted</a> <a href='undocumented#Pixel'>only</a> <a href='undocumented#Pixel'>if</a> <a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Image_Info_Color_Type'>and</a> <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>
<a href='#Image_Info_Alpha_Type'>do</a> <a href='#Image_Info_Alpha_Type'>not</a> <a href='#Image_Info_Alpha_Type'>match</a>. <a href='#Image_Info_Alpha_Type'>Only</a> <a href='#Image_Info_Alpha_Type'>pixels</a> <a href='#Image_Info_Alpha_Type'>within</a> <a href='#Image_Info_Alpha_Type'>both</a> <a href='#Image_Info_Alpha_Type'>source</a> <a href='#Image_Info_Alpha_Type'>and</a> <a href='#Image_Info_Alpha_Type'>destination</a> <a href='#Image_Info_Alpha_Type'>rectangles</a>
<a href='#Image_Info_Alpha_Type'>are</a> <a href='#Image_Info_Alpha_Type'>copied</a>. <a href='#SkSurface_readPixels_3_dst'>dst</a> <a href='#SkSurface_readPixels_3_dst'>contents</a> <a href='#SkSurface_readPixels_3_dst'>outside</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>intersection</a> <a href='SkRect_Reference#Rect'>are</a> <a href='SkRect_Reference#Rect'>unchanged</a>.

<a href='SkRect_Reference#Rect'>Pass</a> <a href='SkRect_Reference#Rect'>negative</a> <a href='SkRect_Reference#Rect'>values</a> <a href='SkRect_Reference#Rect'>for</a> <a href='#SkSurface_readPixels_3_srcX'>srcX</a> <a href='#SkSurface_readPixels_3_srcX'>or</a> <a href='#SkSurface_readPixels_3_srcY'>srcY</a> <a href='#SkSurface_readPixels_3_srcY'>to</a> <a href='#SkSurface_readPixels_3_srcY'>offset</a> <a href='#SkSurface_readPixels_3_srcY'>pixels</a> <a href='#SkSurface_readPixels_3_srcY'>across</a> <a href='#SkSurface_readPixels_3_srcY'>or</a> <a href='#SkSurface_readPixels_3_srcY'>down</a> <a href='#SkSurface_readPixels_3_srcY'>destination</a>.

<a href='#SkSurface_readPixels_3_srcY'>Does</a> <a href='#SkSurface_readPixels_3_srcY'>not</a> <a href='#SkSurface_readPixels_3_srcY'>copy</a>, <a href='#SkSurface_readPixels_3_srcY'>and</a> <a href='#SkSurface_readPixels_3_srcY'>returns</a> <a href='#SkSurface_readPixels_3_srcY'>false</a> <a href='#SkSurface_readPixels_3_srcY'>if</a>:

<table>  <tr>
    <td>Source and destination rectangles do not intersect.</td>
  </tr>  <tr>
    <td><a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>pixels</a> <a href='SkSurface_Reference#Surface'>could</a> <a href='SkSurface_Reference#Surface'>not</a> <a href='SkSurface_Reference#Surface'>be</a> <a href='SkSurface_Reference#Surface'>converted</a> <a href='SkSurface_Reference#Surface'>to</a> <a href='#SkSurface_readPixels_3_dst'>dst</a>.<a href='#SkBitmap_colorType'>colorType</a>() <a href='#SkBitmap_colorType'>or</a> <a href='#SkSurface_readPixels_3_dst'>dst</a>.<a href='#SkBitmap_alphaType'>alphaType</a>().</td>
  </tr>  <tr>
    <td><a href='#SkSurface_readPixels_3_dst'>dst</a> <a href='#SkSurface_readPixels_3_dst'>pixels</a> <a href='#SkSurface_readPixels_3_dst'>could</a> <a href='#SkSurface_readPixels_3_dst'>not</a> <a href='#SkSurface_readPixels_3_dst'>be</a> <a href='#SkSurface_readPixels_3_dst'>allocated</a>.</td>
  </tr>  <tr>
    <td><a href='#SkSurface_readPixels_3_dst'>dst</a>.<a href='#SkBitmap_rowBytes'>rowBytes</a>() <a href='#SkBitmap_rowBytes'>is</a> <a href='#SkBitmap_rowBytes'>too</a> <a href='#SkBitmap_rowBytes'>small</a> <a href='#SkBitmap_rowBytes'>to</a> <a href='#SkBitmap_rowBytes'>contain</a> <a href='#SkBitmap_rowBytes'>one</a> <a href='#SkBitmap_rowBytes'>row</a> <a href='#SkBitmap_rowBytes'>of</a> <a href='#SkBitmap_rowBytes'>pixels</a>.</td>
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
void <a href='#SkSurface_writePixels'>writePixels</a>(<a href='#SkSurface_writePixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#SkPixmap'>src</a>, <a href='SkPixmap_Reference#SkPixmap'>int</a> <a href='SkPixmap_Reference#SkPixmap'>dstX</a>, <a href='SkPixmap_Reference#SkPixmap'>int</a> <a href='SkPixmap_Reference#SkPixmap'>dstY</a>)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>pixels</a> <a href='SkRect_Reference#Rect'>from</a> <a href='SkRect_Reference#Rect'>the</a> <a href='#SkSurface_writePixels_src'>src</a> <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='SkPixmap_Reference#Pixmap'>to</a> <a href='SkPixmap_Reference#Pixmap'>the</a> <a href='SkSurface_Reference#Surface'>Surface</a>.

<a href='SkSurface_Reference#Surface'>Source</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>corners</a> <a href='SkRect_Reference#Rect'>are</a> (0, 0) <a href='SkRect_Reference#Rect'>and</a> (<a href='#SkSurface_writePixels_src'>src</a>.<a href='#SkPixmap_width'>width()</a>, <a href='#SkSurface_writePixels_src'>src</a>.<a href='#SkPixmap_height'>height()</a>).
<a href='#SkPixmap_height'>Destination</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>corners</a> <a href='SkRect_Reference#Rect'>are</a> (<a href='#SkSurface_writePixels_dstX'>dstX</a>, <a href='#SkSurface_writePixels_dstY'>dstY</a>) <a href='#SkSurface_writePixels_dstY'>and</a>
<code>(<a href='#SkSurface_writePixels_dstX'>dstX</a> + <a href='SkSurface_Reference#Surface'>Surface</a> <a href='#SkSurface_width'>width()</a>, <a href='#SkSurface_writePixels_dstY'>dstY</a> + <a href='SkSurface_Reference#Surface'>Surface</a> <a href='#SkSurface_height'>height()</a>)</code>.

Copies each readable <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>intersecting</a> <a href='undocumented#Pixel'>both</a> <a href='undocumented#Pixel'>rectangles</a>, <a href='undocumented#Pixel'>without</a> <a href='undocumented#Pixel'>scaling</a>,
<a href='undocumented#Pixel'>converting</a> <a href='undocumented#Pixel'>to</a> <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>and</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>if</a> <a href='SkImageInfo_Reference#SkAlphaType'>required</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_writePixels_src'><code><strong>src</strong></code></a></td>
    <td>storage for pixels to copy to <a href='SkSurface_Reference#Surface'>Surface</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_writePixels_dstX'><code><strong>dstX</strong></code></a></td>
    <td>x-axis position relative to <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>to</a> <a href='SkSurface_Reference#Surface'>begin</a> <a href='SkSurface_Reference#Surface'>copy</a>; <a href='SkSurface_Reference#Surface'>may</a> <a href='SkSurface_Reference#Surface'>be</a> <a href='SkSurface_Reference#Surface'>negative</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_writePixels_dstY'><code><strong>dstY</strong></code></a></td>
    <td>y-axis position relative to <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>to</a> <a href='SkSurface_Reference#Surface'>begin</a> <a href='SkSurface_Reference#Surface'>copy</a>; <a href='SkSurface_Reference#Surface'>may</a> <a href='SkSurface_Reference#Surface'>be</a> <a href='SkSurface_Reference#Surface'>negative</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="760793bcf0ef193fa61ea03e6e8fc825"></fiddle-embed></div>

### See Also

<a href='#SkSurface_readPixels'>readPixels</a> <a href='#SkSurface_peekPixels'>peekPixels</a>

<a name='SkSurface_writePixels_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkSurface_writePixels'>writePixels</a>(<a href='#SkSurface_writePixels'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#SkBitmap'>src</a>, <a href='SkBitmap_Reference#SkBitmap'>int</a> <a href='SkBitmap_Reference#SkBitmap'>dstX</a>, <a href='SkBitmap_Reference#SkBitmap'>int</a> <a href='SkBitmap_Reference#SkBitmap'>dstY</a>)
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>pixels</a> <a href='SkRect_Reference#Rect'>from</a> <a href='SkRect_Reference#Rect'>the</a> <a href='#SkSurface_writePixels_2_src'>src</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>to</a> <a href='SkBitmap_Reference#Bitmap'>the</a> <a href='SkSurface_Reference#Surface'>Surface</a>.

<a href='SkSurface_Reference#Surface'>Source</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>corners</a> <a href='SkRect_Reference#Rect'>are</a> (0, 0) <a href='SkRect_Reference#Rect'>and</a> (<a href='#SkSurface_writePixels_2_src'>src</a>.<a href='#SkBitmap_width'>width()</a>, <a href='#SkSurface_writePixels_2_src'>src</a>.<a href='#SkBitmap_height'>height()</a>).
<a href='#SkBitmap_height'>Destination</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>corners</a> <a href='SkRect_Reference#Rect'>are</a> (<a href='#SkSurface_writePixels_2_dstX'>dstX</a>, <a href='#SkSurface_writePixels_2_dstY'>dstY</a>) <a href='#SkSurface_writePixels_2_dstY'>and</a>
<code>(<a href='#SkSurface_writePixels_2_dstX'>dstX</a> + <a href='SkSurface_Reference#Surface'>Surface</a> <a href='#SkSurface_width'>width()</a>, <a href='#SkSurface_writePixels_2_dstY'>dstY</a> + <a href='SkSurface_Reference#Surface'>Surface</a> <a href='#SkSurface_height'>height()</a>)</code>.

Copies each readable <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>intersecting</a> <a href='undocumented#Pixel'>both</a> <a href='undocumented#Pixel'>rectangles</a>, <a href='undocumented#Pixel'>without</a> <a href='undocumented#Pixel'>scaling</a>,
<a href='undocumented#Pixel'>converting</a> <a href='undocumented#Pixel'>to</a> <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>and</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>if</a> <a href='SkImageInfo_Reference#SkAlphaType'>required</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_writePixels_2_src'><code><strong>src</strong></code></a></td>
    <td>storage for pixels to copy to <a href='SkSurface_Reference#Surface'>Surface</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_writePixels_2_dstX'><code><strong>dstX</strong></code></a></td>
    <td>x-axis position relative to <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>to</a> <a href='SkSurface_Reference#Surface'>begin</a> <a href='SkSurface_Reference#Surface'>copy</a>; <a href='SkSurface_Reference#Surface'>may</a> <a href='SkSurface_Reference#Surface'>be</a> <a href='SkSurface_Reference#Surface'>negative</a></td>
  </tr>
  <tr>    <td><a name='SkSurface_writePixels_2_dstY'><code><strong>dstY</strong></code></a></td>
    <td>y-axis position relative to <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>to</a> <a href='SkSurface_Reference#Surface'>begin</a> <a href='SkSurface_Reference#Surface'>copy</a>; <a href='SkSurface_Reference#Surface'>may</a> <a href='SkSurface_Reference#Surface'>be</a> <a href='SkSurface_Reference#Surface'>negative</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d77790dd3bc9f678fa4f582347fb8fba"></fiddle-embed></div>

### See Also

<a href='#SkSurface_readPixels'>readPixels</a> <a href='#SkSurface_peekPixels'>peekPixels</a>

<a name='SkSurface_props'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>& <a href='#SkSurface_props'>props()</a> <a href='#SkSurface_props'>const</a>
</pre>

Returns <a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a> <a href='undocumented#SkSurfaceProps'>for</a> <a href='SkSurface_Reference#Surface'>surface</a>.

### Return Value

LCD striping orientation and setting for <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>independent</a> <a href='undocumented#Device'>fonts</a>

### Example

<div><fiddle-embed name="13cf9e7b2894ae6e98c1fd719040bf01">

#### Example Output

~~~~
surf.props(): kRGB_H_SkPixelGeometry
~~~~

</fiddle-embed></div>

### See Also

<a href='undocumented#SkSurfaceProps'>SkSurfaceProps</a>

<a name='SkSurface_prepareForExternalIO'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkSurface_prepareForExternalIO'>prepareForExternalIO</a>()
</pre>

To be deprecated soon.

<a name='Utility'></a>

<a name='SkSurface_flush'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkSurface_flush'>flush()</a>
</pre>

Issues pending <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>commands</a> <a href='SkSurface_Reference#SkSurface'>to</a> <a href='SkSurface_Reference#SkSurface'>the</a> <a href='SkSurface_Reference#SkSurface'>GPU-backed</a> <a href='SkSurface_Reference#SkSurface'>API</a> <a href='SkSurface_Reference#SkSurface'>and</a> <a href='SkSurface_Reference#SkSurface'>resolves</a> <a href='SkSurface_Reference#SkSurface'>any</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>MSAA</a>.

Skia flushes as needed, so it is not necessary to call this if Skia manages
drawing and object lifetime. Call when interleaving Skia calls with native
GPU calls.

### See Also

<a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a>

<a name='SkSurface_flushAndSignalSemaphores'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#GrSemaphoresSubmitted'>GrSemaphoresSubmitted</a> <a href='#SkSurface_flushAndSignalSemaphores'>flushAndSignalSemaphores</a>(<a href='#SkSurface_flushAndSignalSemaphores'>int</a> <a href='#SkSurface_flushAndSignalSemaphores'>numSemaphores</a>,
                                               <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a> <a href='undocumented#GrBackendSemaphore'>signalSemaphores</a>[])
</pre>

Issues pending <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>commands</a> <a href='SkSurface_Reference#SkSurface'>to</a> <a href='SkSurface_Reference#SkSurface'>the</a> <a href='SkSurface_Reference#SkSurface'>GPU-backed</a> <a href='SkSurface_Reference#SkSurface'>API</a> <a href='SkSurface_Reference#SkSurface'>and</a> <a href='SkSurface_Reference#SkSurface'>resolves</a> <a href='SkSurface_Reference#SkSurface'>any</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>MSAA</a>.
After issuing all commands, <a href='#SkSurface_flushAndSignalSemaphores_signalSemaphores'>signalSemaphores</a> <a href='#SkSurface_flushAndSignalSemaphores_signalSemaphores'>of</a> <a href='#SkSurface_flushAndSignalSemaphores_signalSemaphores'>count</a> <a href='#SkSurface_flushAndSignalSemaphores_numSemaphores'>numSemaphores</a> <a href='#SkSurface_flushAndSignalSemaphores_numSemaphores'>semaphores</a>
are signaled by the GPU.

For each <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a> <a href='undocumented#GrBackendSemaphore'>in</a> <a href='#SkSurface_flushAndSignalSemaphores_signalSemaphores'>signalSemaphores</a>:
if <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a> <a href='undocumented#GrBackendSemaphore'>is</a> <a href='undocumented#GrBackendSemaphore'>initialized</a>, <a href='undocumented#GrBackendSemaphore'>the</a> <a href='undocumented#GrBackendSemaphore'>GPU</a> <a href='undocumented#GrBackendSemaphore'>back-end</a> <a href='undocumented#GrBackendSemaphore'>uses</a> <a href='undocumented#GrBackendSemaphore'>the</a> <a href='undocumented#GrBackendSemaphore'>semaphore</a> <a href='undocumented#GrBackendSemaphore'>as</a> <a href='undocumented#GrBackendSemaphore'>is</a>;
otherwise, a new semaphore is created and initializes <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a>.

The caller must delete the semaphores created and returned in <a href='#SkSurface_flushAndSignalSemaphores_signalSemaphores'>signalSemaphores</a>.
<a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a> <a href='undocumented#GrBackendSemaphore'>can</a> <a href='undocumented#GrBackendSemaphore'>be</a> <a href='undocumented#GrBackendSemaphore'>deleted</a> <a href='undocumented#GrBackendSemaphore'>as</a> <a href='undocumented#GrBackendSemaphore'>soon</a> <a href='undocumented#GrBackendSemaphore'>as</a> <a href='undocumented#GrBackendSemaphore'>this</a> <a href='undocumented#GrBackendSemaphore'>function</a> <a href='undocumented#GrBackendSemaphore'>returns</a>.

If the back-end API is OpenGL only uninitialized  <a href='undocumented#Backend_Semaphore'>backend semaphores</a> are supported.

If the back-end API is Vulkan semaphores may be initialized or uninitialized.
If uninitialized, created semaphores are valid only with the VkDevice
with which they were created.

If <a href='undocumented#GrSemaphoresSubmitted'>GrSemaphoresSubmitted</a>::<a href='#GrSemaphoresSubmitted_kNo'>kNo</a> <a href='#GrSemaphoresSubmitted_kNo'>is</a> <a href='#GrSemaphoresSubmitted_kNo'>returned</a>, <a href='#GrSemaphoresSubmitted_kNo'>the</a> <a href='#GrSemaphoresSubmitted_kNo'>GPU</a> <a href='#GrSemaphoresSubmitted_kNo'>back-end</a> <a href='#GrSemaphoresSubmitted_kNo'>did</a> <a href='#GrSemaphoresSubmitted_kNo'>not</a> <a href='#GrSemaphoresSubmitted_kNo'>create</a> <a href='#GrSemaphoresSubmitted_kNo'>or</a>
add any semaphores to signal on the GPU; the caller should not instruct the GPU
to wait on any of the semaphores.

Pending <a href='SkSurface_Reference#Surface'>surface</a> <a href='SkSurface_Reference#Surface'>commands</a> <a href='SkSurface_Reference#Surface'>are</a> <a href='SkSurface_Reference#Surface'>flushed</a> <a href='SkSurface_Reference#Surface'>regardless</a> <a href='SkSurface_Reference#Surface'>of</a> <a href='SkSurface_Reference#Surface'>the</a> <a href='SkSurface_Reference#Surface'>return</a> <a href='SkSurface_Reference#Surface'>result</a>.

### Parameters

<table>  <tr>    <td><a name='SkSurface_flushAndSignalSemaphores_numSemaphores'><code><strong>numSemaphores</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='#SkSurface_flushAndSignalSemaphores_signalSemaphores'>signalSemaphores</a> <a href='#SkSurface_flushAndSignalSemaphores_signalSemaphores'>array</a></td>
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
bool wait(int numSemaphores, const <a href='undocumented#GrBackendSemaphore'>GrBackendSemaphore</a>* <a href='undocumented#GrBackendSemaphore'>waitSemaphores</a>)
</pre>

Inserts a list of GPU semaphores that the current GPU-backed API must wait on before
executing any more commands on the GPU for this <a href='SkSurface_Reference#Surface'>surface</a>. <a href='SkSurface_Reference#Surface'>Skia</a> <a href='SkSurface_Reference#Surface'>will</a> <a href='SkSurface_Reference#Surface'>take</a> <a href='SkSurface_Reference#Surface'>ownership</a> <a href='SkSurface_Reference#Surface'>of</a> <a href='SkSurface_Reference#Surface'>the</a>
underlying semaphores and delete them once they have been signaled and waited on.
If this call returns false, then the GPU back-end will not wait on any passed in semaphores,
and the client will still own the semaphores.

### Parameters

<table>  <tr>    <td><a name='SkSurface_wait_numSemaphores'><code><strong>numSemaphores</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='#SkSurface_wait_waitSemaphores'>waitSemaphores</a> <a href='#SkSurface_wait_waitSemaphores'>array</a></td>
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
bool <a href='#SkSurface_characterize'>characterize</a>(<a href='undocumented#SkSurfaceCharacterization'>SkSurfaceCharacterization</a>* <a href='undocumented#SkSurfaceCharacterization'>characterization</a>) <a href='undocumented#SkSurfaceCharacterization'>const</a>
</pre>

Initializes <a href='undocumented#SkSurfaceCharacterization'>SkSurfaceCharacterization</a> <a href='undocumented#SkSurfaceCharacterization'>that</a> <a href='undocumented#SkSurfaceCharacterization'>can</a> <a href='undocumented#SkSurfaceCharacterization'>be</a> <a href='undocumented#SkSurfaceCharacterization'>used</a> <a href='undocumented#SkSurfaceCharacterization'>to</a> <a href='undocumented#SkSurfaceCharacterization'>perform</a> <a href='undocumented#SkSurfaceCharacterization'>GPU</a> <a href='undocumented#SkSurfaceCharacterization'>back-end</a>
processing in a separate thread. Typically this is used to divide drawing
into multiple tiles. <a href='undocumented#SkDeferredDisplayListRecorder'>SkDeferredDisplayListRecorder</a> <a href='undocumented#SkDeferredDisplayListRecorder'>records</a> <a href='undocumented#SkDeferredDisplayListRecorder'>the</a> <a href='undocumented#SkDeferredDisplayListRecorder'>drawing</a> <a href='undocumented#SkDeferredDisplayListRecorder'>commands</a>
for each tile.

Return true if <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>supports</a> <a href='#SkSurface_characterize_characterization'>characterization</a>.  <a href='undocumented#Raster_Surface'>raster surface</a> <a href='#SkSurface_characterize_characterization'>returns</a> <a href='#SkSurface_characterize_characterization'>false</a>.

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
bool draw(<a href='undocumented#SkDeferredDisplayList'>SkDeferredDisplayList</a>* <a href='undocumented#SkDeferredDisplayList'>deferredDisplayList</a>)
</pre>

Draws   deferred display list created using <a href='undocumented#SkDeferredDisplayListRecorder'>SkDeferredDisplayListRecorder</a>.
Has no effect and returns false if <a href='undocumented#SkSurfaceCharacterization'>SkSurfaceCharacterization</a> <a href='undocumented#SkSurfaceCharacterization'>stored</a> <a href='undocumented#SkSurfaceCharacterization'>in</a>
<a href='#SkSurface_draw_2_deferredDisplayList'>deferredDisplayList</a> <a href='#SkSurface_draw_2_deferredDisplayList'>is</a> <a href='#SkSurface_draw_2_deferredDisplayList'>not</a> <a href='#SkSurface_draw_2_deferredDisplayList'>compatible</a> <a href='#SkSurface_draw_2_deferredDisplayList'>with</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>.

<a href='undocumented#Raster_Surface'>raster surface</a> returns false.

### Parameters

<table>  <tr>    <td><a name='SkSurface_draw_2_deferredDisplayList'><code><strong>deferredDisplayList</strong></code></a></td>
    <td>drawing commands</td>
  </tr>
</table>

### Return Value

false if <a href='#SkSurface_draw_2_deferredDisplayList'>deferredDisplayList</a> <a href='#SkSurface_draw_2_deferredDisplayList'>is</a> <a href='#SkSurface_draw_2_deferredDisplayList'>not</a> <a href='#SkSurface_draw_2_deferredDisplayList'>compatible</a>

### Example

<div><fiddle-embed name="46d9bacf593deaaeabd74ff42f2571a0" gpu="true" cpu="true"></fiddle-embed></div>

### See Also

<a href='#SkSurface_characterize'>characterize()</a> <a href='undocumented#SkSurfaceCharacterization'>SkSurfaceCharacterization</a> <a href='undocumented#SkDeferredDisplayList'>SkDeferredDisplayList</a>

