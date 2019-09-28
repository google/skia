SkImage Reference
===


<a name='SkImage'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkImage_Reference#SkImage'>SkImage</a> : public <a href='undocumented#SkRefCnt'>SkRefCnt</a> {

    typedef void* <a href='#SkImage_ReleaseContext'>ReleaseContext</a>;

    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeRasterCopy'>MakeRasterCopy</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#Pixmap'>pixmap</a>);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeRasterData'>MakeRasterData</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkData'>SkData</a>> pixels,
                                         size_t rowBytes);

    typedef void (*<a href='#SkImage_RasterReleaseProc'>RasterReleaseProc</a>)(const void* pixels, <a href='#SkImage_ReleaseContext'>ReleaseContext</a>);

    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromRaster'>MakeFromRaster</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#Pixmap'>pixmap</a>,
                                         <a href='#SkImage_RasterReleaseProc'>RasterReleaseProc</a> rasterReleaseProc,
                                         <a href='#SkImage_ReleaseContext'>ReleaseContext</a> releaseContext);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromBitmap'>MakeFromBitmap</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromGenerator'>MakeFromGenerator</a>(std::unique_ptr<<a href='undocumented#SkImageGenerator'>SkImageGenerator</a>> imageGenerator,
                                            const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* subset = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromEncoded'>MakeFromEncoded</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkData'>SkData</a>> encoded, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* subset = nullptr);

    typedef void (*<a href='#SkImage_TextureReleaseProc'>TextureReleaseProc</a>)(<a href='#SkImage_ReleaseContext'>ReleaseContext</a> releaseContext);

    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromTexture'>MakeFromTexture</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                          const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture,
                                          <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> origin,
                                          <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkImage_colorType'>colorType</a>,
                                          <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImage_alphaType'>alphaType</a>,
                                          <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='#SkImage_colorSpace'>colorSpace</a>);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromTexture'>MakeFromTexture</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                          const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture,
                                          <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> origin,
                                          <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkImage_colorType'>colorType</a>,
                                          <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImage_alphaType'>alphaType</a>,
                                          <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='#SkImage_colorSpace'>colorSpace</a>,
                                          <a href='#SkImage_TextureReleaseProc'>TextureReleaseProc</a> textureReleaseProc,
                                          <a href='#SkImage_ReleaseContext'>ReleaseContext</a> releaseContext);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeCrossContextFromEncoded'>MakeCrossContextFromEncoded</a>(<a href='undocumented#GrContext'>GrContext</a>* context, <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkData'>SkData</a>> <a href='undocumented#Data'>data</a>,
                                                      bool buildMips, <a href='undocumented#SkColorSpace'>SkColorSpace</a>* dstColorSpace,
                                                      bool limitToMaxTextureSize = false);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeCrossContextFromPixmap'>MakeCrossContextFromPixmap</a>(<a href='undocumented#GrContext'>GrContext</a>* context, const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#Pixmap'>pixmap</a>,
                                                     bool buildMips, <a href='undocumented#SkColorSpace'>SkColorSpace</a>* dstColorSpace,
                                                     bool limitToMaxTextureSize = false);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromAdoptedTexture'>MakeFromAdoptedTexture</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                                 const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture,
                                                 <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> surfaceOrigin,
                                                 <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkImage_colorType'>colorType</a>,
                                                 <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImage_alphaType'>alphaType</a> = <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
                                                 <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='#SkImage_colorSpace'>colorSpace</a> = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromYUVATexturesCopy'>MakeFromYUVATexturesCopy</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                                   <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> yuvColorSpace,
                                                   const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> yuvaTextures[],
                                                   const <a href='undocumented#SkYUVAIndex'>SkYUVAIndex</a> yuvaIndices[4],
                                                   <a href='undocumented#SkISize'>SkISize</a> imageSize,
                                                   <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> imageOrigin,
                                                   <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> imageColorSpace = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend'>MakeFromYUVATexturesCopyWithExternalBackend</a>(
            <a href='undocumented#GrContext'>GrContext</a>* context,
            <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> yuvColorSpace,
            const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> yuvaTextures[],
            const <a href='undocumented#SkYUVAIndex'>SkYUVAIndex</a> yuvaIndices[4],
            <a href='undocumented#SkISize'>SkISize</a> imageSize,
            <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> imageOrigin,
            const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture,
            <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> imageColorSpace = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromYUVATextures'>MakeFromYUVATextures</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                               <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> yuvColorSpace,
                                               const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> yuvaTextures[],
                                               const <a href='undocumented#SkYUVAIndex'>SkYUVAIndex</a> yuvaIndices[4],
                                               <a href='undocumented#SkISize'>SkISize</a> imageSize,
                                               <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> imageOrigin,
                                               <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> imageColorSpace = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromYUVAPixmaps'>MakeFromYUVAPixmaps</a>(
            <a href='undocumented#GrContext'>GrContext</a>* context, <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> yuvColorSpace, const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> yuvaPixmaps[],
            const <a href='undocumented#SkYUVAIndex'>SkYUVAIndex</a> yuvaIndices[4], <a href='undocumented#SkISize'>SkISize</a> imageSize, <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> imageOrigin,
            bool buildMips, bool limitToMaxTextureSize = false,
            <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> imageColorSpace = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromYUVTexturesCopy'>MakeFromYUVTexturesCopy</a>(<a href='undocumented#GrContext'>GrContext</a>* context, <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> yuvColorSpace,
                                                  const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> yuvTextures[3],
                                                  <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> imageOrigin,
                                                  <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> imageColorSpace = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend'>MakeFromYUVTexturesCopyWithExternalBackend</a>(
            <a href='undocumented#GrContext'>GrContext</a>* context, <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> yuvColorSpace,
            const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> yuvTextures[3], <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> imageOrigin,
            const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture, <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> imageColorSpace = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromNV12TexturesCopy'>MakeFromNV12TexturesCopy</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                                   <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> yuvColorSpace,
                                                   const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> nv12Textures[2],
                                                   <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> imageOrigin,
                                                   <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> imageColorSpace = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend'>MakeFromNV12TexturesCopyWithExternalBackend</a>(
            <a href='undocumented#GrContext'>GrContext</a>* context,
            <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> yuvColorSpace,
            const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> nv12Textures[2],
            <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> imageOrigin,
            const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture,
            <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> imageColorSpace = nullptr);

    enum class <a href='#SkImage_BitDepth'>BitDepth</a> {
        kU8,
        kF16,
    };

    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromPicture'>MakeFromPicture</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='SkPicture_Reference#SkPicture'>SkPicture</a>> <a href='SkPicture_Reference#Picture'>picture</a>, const <a href='undocumented#SkISize'>SkISize</a>& dimensions,
                                          const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#Matrix'>matrix</a>, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                                          <a href='#SkImage_BitDepth'>BitDepth</a> bitDepth,
                                          <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='#SkImage_colorSpace'>colorSpace</a>);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromAHardwareBuffer'>MakeFromAHardwareBuffer</a>(
            AHardwareBuffer* hardwareBuffer,
            <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImage_alphaType'>alphaType</a> = <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
            <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='#SkImage_colorSpace'>colorSpace</a> = nullptr,
            <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> surfaceOrigin = <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a>);
    int <a href='#SkImage_width'>width()</a> const;
    int <a href='#SkImage_height'>height()</a> const;
    <a href='undocumented#SkISize'>SkISize</a> <a href='#SkImage_dimensions'>dimensions()</a> const;
    <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkImage_bounds'>bounds()</a> const;
    uint32_t <a href='#SkImage_uniqueID'>uniqueID</a>() const;
    <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImage_alphaType'>alphaType</a>() const;
    <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkImage_colorType'>colorType</a>() const;
    <a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='#SkImage_colorSpace'>colorSpace</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='#SkImage_refColorSpace'>refColorSpace</a>() const;
    bool <a href='#SkImage_isAlphaOnly'>isAlphaOnly</a>() const;
    bool <a href='#SkImage_isOpaque'>isOpaque</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkShader'>SkShader</a>> <a href='#SkImage_makeShader'>makeShader</a>(<a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_TileMode'>TileMode</a> tileMode1, <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_TileMode'>TileMode</a> tileMode2,
                               const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* localMatrix = nullptr) const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkShader'>SkShader</a>> <a href='#SkImage_makeShader'>makeShader</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* localMatrix = nullptr) const;
    bool <a href='#SkImage_peekPixels'>peekPixels</a>(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>* <a href='SkPixmap_Reference#Pixmap'>pixmap</a>) const;
    bool <a href='#SkImage_isTextureBacked'>isTextureBacked</a>() const;
    bool <a href='#SkImage_isValid'>isValid</a>(<a href='undocumented#GrContext'>GrContext</a>* context) const;
    <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='#SkImage_getBackendTexture'>getBackendTexture</a>(bool flushPendingGrContextIO,
                                       <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a>* origin = nullptr) const;

    enum <a href='#SkImage_CachingHint'>CachingHint</a> {
        <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>,
        <a href='#SkImage_kDisallow_CachingHint'>kDisallow_CachingHint</a>,
    };

    bool <a href='#SkImage_readPixels'>readPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& dstInfo, void* dstPixels, size_t dstRowBytes,
                    int srcX, int srcY, <a href='#SkImage_CachingHint'>CachingHint</a> cachingHint = <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>) const;
    bool <a href='#SkImage_readPixels'>readPixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& dst, int srcX, int srcY,
                    <a href='#SkImage_CachingHint'>CachingHint</a> cachingHint = <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>) const;
    bool <a href='#SkImage_scalePixels'>scalePixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& dst, <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> filterQuality,
                     <a href='#SkImage_CachingHint'>CachingHint</a> cachingHint = <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>) const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkData'>SkData</a>> <a href='#SkImage_encodeToData'>encodeToData</a>(<a href='undocumented#SkEncodedImageFormat'>SkEncodedImageFormat</a> encodedImageFormat, int quality) const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkData'>SkData</a>> <a href='#SkImage_encodeToData'>encodeToData</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkData'>SkData</a>> <a href='#SkImage_refEncodedData'>refEncodedData</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_makeSubset'>makeSubset</a>(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& subset) const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_makeTextureImage'>makeTextureImage</a>(<a href='undocumented#GrContext'>GrContext</a>* context, <a href='undocumented#SkColorSpace'>SkColorSpace</a>* dstColorSpace,
                                    <a href='undocumented#GrMipMapped'>GrMipMapped</a> mipMapped = <a href='undocumented#GrMipMapped'>GrMipMapped</a>::<a href='#GrMipMapped_kNo'>kNo</a>) const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_makeNonTextureImage'>makeNonTextureImage</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_makeRasterImage'>makeRasterImage</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_makeWithFilter'>makeWithFilter</a>(const <a href='undocumented#SkImageFilter'>SkImageFilter</a>* filter, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& subset,
                                  const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& clipBounds, <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* outSubset,
                                  <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>* offset) const;

    typedef std::function<void(<a href='undocumented#GrBackendTexture'>GrBackendTexture</a>)> <a href='#SkImage_BackendTextureReleaseProc'>BackendTextureReleaseProc</a>;

    static bool <a href='#SkImage_MakeBackendTextureFromSkImage'>MakeBackendTextureFromSkImage</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                              <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='SkImage_Reference#Image'>image</a>,
                                              <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>* backendTexture,
                                              <a href='#SkImage_BackendTextureReleaseProc'>BackendTextureReleaseProc</a>* backendTextureReleaseProc);
    bool <a href='#SkImage_isLazyGenerated'>isLazyGenerated</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_makeColorSpace'>makeColorSpace</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> target) const;
};

</pre>

<a href='SkImage_Reference#Image'>Image</a> describes a two dimensional array of pixels to draw. The pixels may be
decoded in a <a href='#Raster_Bitmap'>Raster_Bitmap</a>, encoded in a <a href='SkPicture_Reference#Picture'>Picture</a> or compressed <a href='undocumented#Data'>data</a> <a href='SkStream_Reference#Stream'>stream</a>,
or located in GPU memory as a <a href='#GPU_Texture'>GPU_Texture</a>.

<a href='SkImage_Reference#Image'>Image</a> cannot be modified after it is created. <a href='SkImage_Reference#Image'>Image</a> may allocate additional
storage as needed; for instance, an encoded <a href='SkImage_Reference#Image'>Image</a> may decode when drawn.

<a href='SkImage_Reference#Image'>Image</a> width and height are greater than zero. Creating an <a href='SkImage_Reference#Image'>Image</a> with zero width
or height returns <a href='SkImage_Reference#Image'>Image</a> equal to nullptr.

<a href='SkImage_Reference#Image'>Image</a> may be created from <a href='SkBitmap_Reference#Bitmap'>Bitmap</a>, <a href='SkPixmap_Reference#Pixmap'>Pixmap</a>, <a href='SkSurface_Reference#Surface'>Surface</a>, <a href='SkPicture_Reference#Picture'>Picture</a>, encoded streams,
<a href='#GPU_Texture'>GPU_Texture</a>, <a href='#Image_Info_YUV_ColorSpace'>YUV_ColorSpace</a> <a href='undocumented#Data'>data</a>, or hardware buffer. Encoded streams supported
include BMP, GIF, HEIF, ICO, JPEG, PNG, WBMP, WebP. Supported encoding details
vary with platform.

<a name='Raster_Image'></a>

<a href='#Image_Raster_Image'>Raster_Image</a> pixels are decoded in a <a href='#Raster_Bitmap'>Raster_Bitmap</a>. These pixels may be read
directly and in most cases written to, although edited pixels may not be drawn
if <a href='SkImage_Reference#Image'>Image</a> has been copied internally.

<a name='Texture_Image'></a>

<a href='#Image_Texture_Image'>Texture_Image</a> are located on GPU and pixels are not accessible. <a href='#Image_Texture_Image'>Texture_Image</a>
are allocated optimally for best performance. <a href='#Image_Raster_Image'>Raster_Image</a> may
be drawn to <a href='#GPU_Surface'>GPU_Surface</a>, but pixels are uploaded from CPU to GPU downgrading
performance.

<a name='Lazy_Image'></a>

<a href='#Image_Lazy_Image'>Lazy_Image</a> defer allocating buffer for <a href='SkImage_Reference#Image'>Image</a> pixels and decoding <a href='SkStream_Reference#Stream'>stream</a> until
<a href='SkImage_Reference#Image'>Image</a> is drawn. <a href='#Image_Lazy_Image'>Lazy_Image</a> caches result if possible to speed up repeated
drawing.

<a name='SkImage_MakeRasterCopy'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_MakeRasterCopy'>MakeRasterCopy</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#Pixmap'>pixmap</a>)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> from <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> and copy of pixels. Since pixels are copied, <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>
pixels may be modified or deleted without affecting <a href='SkImage_Reference#SkImage'>SkImage</a>.

<a href='SkImage_Reference#SkImage'>SkImage</a> is returned if <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> is valid. Valid <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> parameters include:
dimensions are greater than zero;
each dimension fits in 29 bits;
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> and <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> are valid, and <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is not <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>;
row bytes are large enough to hold one row of pixels;
<a href='undocumented#Pixel'>pixel</a> address is not nullptr.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeRasterCopy_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, <a href='undocumented#Pixel'>pixel</a> address, and row bytes</td>
  </tr>
</table>

### Return Value

copy of <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> pixels, or nullptr

### Example

<div><fiddle-embed name="513afec5795a9504ebf6af5373d16b6b"><div>Draw a five by five <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, and draw a copy in an <a href='SkImage_Reference#Image'>Image</a>. Editing the <a href='#SkImage_MakeRasterCopy_pixmap'>pixmap</a>
alters the  <a href='SkBitmap_Reference#Bitmap_Draw'>bitmap draw</a>, but does not alter the <a href='SkImage_Reference#Image'>Image</a> draw since the <a href='SkImage_Reference#Image'>Image</a>
contains a copy of the pixels.
</div></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeRasterData'>MakeRasterData</a> <a href='#SkImage_MakeFromGenerator'>MakeFromGenerator</a>

<a name='SkImage_MakeRasterData'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_MakeRasterData'>MakeRasterData</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkData'>SkData</a>&gt; pixels, size_t rowBytes)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> from <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, sharing <a href='#SkImage_MakeRasterData_pixels'>pixels</a>.

<a href='SkImage_Reference#SkImage'>SkImage</a> is returned if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> is valid. Valid <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> parameters include:
dimensions are greater than zero;
each dimension fits in 29 bits;
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> and <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> are valid, and <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is not <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>;
<a href='#SkImage_MakeRasterData_rowBytes'>rowBytes</a> are large enough to hold one row of <a href='#SkImage_MakeRasterData_pixels'>pixels</a>;
<a href='#SkImage_MakeRasterData_pixels'>pixels</a> is not nullptr, and contains enough <a href='undocumented#Data'>data</a> for <a href='SkImage_Reference#SkImage'>SkImage</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeRasterData_info'><code><strong>info</strong></code></a></td>
    <td>contains width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeRasterData_pixels'><code><strong>pixels</strong></code></a></td>
    <td>address or  <a href='undocumented#Pixel_Storage'>pixel storage</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeRasterData_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> of <a href='undocumented#Pixel'>pixel</a> row or larger</td>
  </tr>
</table>

### Return Value

<a href='SkImage_Reference#SkImage'>SkImage</a> sharing <a href='#SkImage_MakeRasterData_pixels'>pixels</a>, or nullptr

### Example

<div><fiddle-embed name="22e7ce79ab2fe94252d23319f2258127"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeRasterCopy'>MakeRasterCopy</a> <a href='#SkImage_MakeFromGenerator'>MakeFromGenerator</a>

<a name='SkImage_ReleaseContext'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    typedef void* <a href='#SkImage_ReleaseContext'>ReleaseContext</a>;
</pre>

Caller <a href='undocumented#Data'>data</a> passed to <a href='#SkImage_RasterReleaseProc'>RasterReleaseProc</a>; may be nullptr.

### See Also

<a href='#SkImage_MakeFromRaster'>MakeFromRaster</a> <a href='#SkImage_RasterReleaseProc'>RasterReleaseProc</a>

<a name='SkImage_RasterReleaseProc'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    typedef void (*<a href='#SkImage_RasterReleaseProc'>RasterReleaseProc</a>)(const void* pixels, <a href='#SkImage_ReleaseContext'>ReleaseContext</a>);
</pre>

Function called when <a href='SkImage_Reference#Image'>Image</a> no longer shares pixels. <a href='#SkImage_ReleaseContext'>ReleaseContext</a> is
provided by caller when <a href='SkImage_Reference#Image'>Image</a> is created, and may be nullptr.

### See Also

<a href='#SkImage_ReleaseContext'>ReleaseContext</a> <a href='#SkImage_MakeFromRaster'>MakeFromRaster</a>

<a name='SkImage_MakeFromRaster'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_MakeFromRaster'>MakeFromRaster</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#Pixmap'>pixmap</a>, <a href='#SkImage_RasterReleaseProc'>RasterReleaseProc</a> rasterReleaseProc,
                                     <a href='#SkImage_ReleaseContext'>ReleaseContext</a> releaseContext)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> from <a href='#SkImage_MakeFromRaster_pixmap'>pixmap</a>, sharing <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> pixels. Pixels must remain valid and
unchanged until <a href='#SkImage_MakeFromRaster_rasterReleaseProc'>rasterReleaseProc</a> is called. <a href='#SkImage_MakeFromRaster_rasterReleaseProc'>rasterReleaseProc</a> is passed
<a href='#SkImage_MakeFromRaster_releaseContext'>releaseContext</a> when <a href='SkImage_Reference#SkImage'>SkImage</a> is deleted or no longer refers to  <a href='SkPixmap_Reference#Pixmap_Pixels'>pixmap pixels</a>.

Pass nullptr for <a href='#SkImage_MakeFromRaster_rasterReleaseProc'>rasterReleaseProc</a> to share <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> without requiring a callback
when <a href='SkImage_Reference#SkImage'>SkImage</a> is released. Pass nullptr for <a href='#SkImage_MakeFromRaster_releaseContext'>releaseContext</a> if <a href='#SkImage_MakeFromRaster_rasterReleaseProc'>rasterReleaseProc</a>
does not require state.

<a href='SkImage_Reference#SkImage'>SkImage</a> is returned if <a href='#SkImage_MakeFromRaster_pixmap'>pixmap</a> is valid. Valid <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> parameters include:
dimensions are greater than zero;
each dimension fits in 29 bits;
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> and <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> are valid, and <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is not <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>;
row bytes are large enough to hold one row of pixels;
<a href='undocumented#Pixel'>pixel</a> address is not nullptr.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromRaster_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, <a href='undocumented#Pixel'>pixel</a> address, and row bytes</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromRaster_rasterReleaseProc'><code><strong>rasterReleaseProc</strong></code></a></td>
    <td>function called when pixels can be released; or nullptr</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromRaster_releaseContext'><code><strong>releaseContext</strong></code></a></td>
    <td>state passed to <a href='#SkImage_MakeFromRaster_rasterReleaseProc'>rasterReleaseProc</a>; or nullptr</td>
  </tr>
</table>

### Return Value

<a href='SkImage_Reference#SkImage'>SkImage</a> sharing <a href='#SkImage_MakeFromRaster_pixmap'>pixmap</a>

### Example

<div><fiddle-embed name="275356b65d18c8868f4434137350cddc">

#### Example Output

~~~~
before reset: 0
after reset: 1
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImage_MakeRasterCopy'>MakeRasterCopy</a> <a href='#SkImage_MakeRasterData'>MakeRasterData</a> <a href='#SkImage_MakeFromGenerator'>MakeFromGenerator</a> <a href='#SkImage_RasterReleaseProc'>RasterReleaseProc</a> <a href='#SkImage_ReleaseContext'>ReleaseContext</a>

<a name='SkImage_MakeFromBitmap'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_MakeFromBitmap'>MakeFromBitmap</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> from <a href='#SkImage_MakeFromBitmap_bitmap'>bitmap</a>, sharing or copying  <a href='SkBitmap_Reference#Bitmap_Pixels'>bitmap pixels</a>. If the <a href='#SkImage_MakeFromBitmap_bitmap'>bitmap</a>
is marked immutable, and its <a href='undocumented#Pixel'>pixel</a> memory is shareable, it may be shared
instead of copied.

<a href='SkImage_Reference#SkImage'>SkImage</a> is returned if <a href='#SkImage_MakeFromBitmap_bitmap'>bitmap</a> is valid. Valid <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> parameters include:
dimensions are greater than zero;
each dimension fits in 29 bits;
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> and <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> are valid, and <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is not <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>;
row bytes are large enough to hold one row of pixels;
<a href='undocumented#Pixel'>pixel</a> address is not nullptr.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromBitmap_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, row bytes, and pixels</td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, or nullptr

### Example

<div><fiddle-embed name="cf2cf53321e4e6a77c2841bfbc0ef707"><div>The first <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> is shared; writing to the <a href='undocumented#Pixel'>pixel</a> memory changes the first
<a href='SkImage_Reference#Image'>Image</a>.
The second <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> is marked immutable, and is copied; writing to the <a href='undocumented#Pixel'>pixel</a>
memory does not alter the second <a href='SkImage_Reference#Image'>Image</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromRaster'>MakeFromRaster</a> <a href='#SkImage_MakeRasterCopy'>MakeRasterCopy</a> <a href='#SkImage_MakeFromGenerator'>MakeFromGenerator</a> <a href='#SkImage_MakeRasterData'>MakeRasterData</a>

<a name='SkImage_MakeFromGenerator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_MakeFromGenerator'>MakeFromGenerator</a>(std::unique_ptr&lt;<a href='undocumented#SkImageGenerator'>SkImageGenerator</a>&gt; imageGenerator,
                                 const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* subset = nullptr)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> from <a href='undocumented#Data'>data</a> returned by <a href='#SkImage_MakeFromGenerator_imageGenerator'>imageGenerator</a>. Generated <a href='undocumented#Data'>data</a> is owned by <a href='SkImage_Reference#SkImage'>SkImage</a> and
may not be shared or accessed.

<a href='#SkImage_MakeFromGenerator_subset'>subset</a> allows selecting a portion of the full <a href='SkImage_Reference#Image'>image</a>. Pass nullptr to select the entire
<a href='SkImage_Reference#Image'>image</a>; otherwise, <a href='#SkImage_MakeFromGenerator_subset'>subset</a> must be contained by <a href='SkImage_Reference#Image'>image</a> bounds.

<a href='SkImage_Reference#SkImage'>SkImage</a> is returned if generator <a href='undocumented#Data'>data</a> is valid. Valid <a href='undocumented#Data'>data</a> parameters vary by type of <a href='undocumented#Data'>data</a>
and platform.

<a href='#SkImage_MakeFromGenerator_imageGenerator'>imageGenerator</a> may wrap <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='undocumented#Data'>data</a>, codec <a href='undocumented#Data'>data</a>, or custom <a href='undocumented#Data'>data</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromGenerator_imageGenerator'><code><strong>imageGenerator</strong></code></a></td>
    <td>stock or custom routines to retrieve <a href='SkImage_Reference#SkImage'>SkImage</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromGenerator_subset'><code><strong>subset</strong></code></a></td>
    <td>bounds of returned <a href='SkImage_Reference#SkImage'>SkImage</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, or nullptr

### Example

<div><fiddle-embed name="c2fec0746f88ca34d7dce59dd9bdef9e"><div>The generator returning <a href='SkPicture_Reference#Picture'>Picture</a> cannot be shared; std::move transfers ownership to generated <a href='SkImage_Reference#Image'>Image</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromEncoded'>MakeFromEncoded</a>

<a name='SkImage_MakeFromEncoded'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_MakeFromEncoded'>MakeFromEncoded</a>(<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkData'>SkData</a>&gt; encoded, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* subset = nullptr)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> from <a href='#SkImage_MakeFromEncoded_encoded'>encoded</a> <a href='undocumented#Data'>data</a>.
<a href='#SkImage_MakeFromEncoded_subset'>subset</a> allows selecting a portion of the full <a href='SkImage_Reference#Image'>image</a>. Pass nullptr to select the entire
<a href='SkImage_Reference#Image'>image</a>; otherwise, <a href='#SkImage_MakeFromEncoded_subset'>subset</a> must be contained by <a href='SkImage_Reference#Image'>image</a> bounds.

<a href='SkImage_Reference#SkImage'>SkImage</a> is returned if format of the <a href='#SkImage_MakeFromEncoded_encoded'>encoded</a> <a href='undocumented#Data'>data</a> is recognized and supported.
Recognized formats vary by platform.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromEncoded_encoded'><code><strong>encoded</strong></code></a></td>
    <td><a href='undocumented#Data'>data</a> of <a href='SkImage_Reference#SkImage'>SkImage</a> to decode</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromEncoded_subset'><code><strong>subset</strong></code></a></td>
    <td>bounds of returned <a href='SkImage_Reference#SkImage'>SkImage</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, or nullptr

### Example

<div><fiddle-embed name="894f732ed6409b1f392bc5481421d0e9"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromGenerator'>MakeFromGenerator</a>

<a name='SkImage_TextureReleaseProc'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    typedef void (*<a href='#SkImage_TextureReleaseProc'>TextureReleaseProc</a>)(<a href='#SkImage_ReleaseContext'>ReleaseContext</a> releaseContext);
</pre>

User function called when supplied <a href='undocumented#Texture'>texture</a> may be deleted.

### See Also

<a href='#SkImage_MakeFromTexture'>MakeFromTexture</a>

<a name='SkImage_MakeFromTexture'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_MakeFromTexture'>MakeFromTexture</a>(<a href='undocumented#GrContext'>GrContext</a>* context, const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture,
                                      <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> origin, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkImage_colorType'>colorType</a>,
                                      <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImage_alphaType'>alphaType</a>, <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; <a href='#SkImage_colorSpace'>colorSpace</a>)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> from  <a href='undocumented#GPU_Texture'>GPU texture</a> associated with <a href='#SkImage_MakeFromTexture_context'>context</a>. Caller is responsible for
managing the lifetime of  <a href='undocumented#GPU_Texture'>GPU texture</a>.

<a href='SkImage_Reference#SkImage'>SkImage</a> is returned if format of <a href='#SkImage_MakeFromTexture_backendTexture'>backendTexture</a> is recognized and supported.
Recognized formats vary by GPU back-end.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromTexture_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromTexture_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td><a href='undocumented#Texture'>texture</a> residing on GPU</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromTexture_origin'><code><strong>origin</strong></code></a></td>
    <td>one of: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft_GrSurfaceOrigin</a>, <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromTexture_colorType'><code><strong>colorType</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>,
<a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>, <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>,
<a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromTexture_alphaType'><code><strong>alphaType</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromTexture_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td>range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, or nullptr

### Example

<div><fiddle-embed name="94e9296c53bad074bf2a48ff885dac13" gpu="true"><div>A back-end <a href='undocumented#Texture'>texture</a> has been created and uploaded to the GPU outside of this example.
</div></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromAdoptedTexture'>MakeFromAdoptedTexture</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_MakeFromBackendTexture'>MakeFromBackendTexture</a>

<a name='SkImage_MakeFromTexture_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_MakeFromTexture'>MakeFromTexture</a>(<a href='undocumented#GrContext'>GrContext</a>* context, const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture,
                                      <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> origin, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkImage_colorType'>colorType</a>,
                                      <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImage_alphaType'>alphaType</a>, <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; <a href='#SkImage_colorSpace'>colorSpace</a>,
                                      <a href='#SkImage_TextureReleaseProc'>TextureReleaseProc</a> textureReleaseProc,
                                      <a href='#SkImage_ReleaseContext'>ReleaseContext</a> releaseContext)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> from  <a href='undocumented#GPU_Texture'>GPU texture</a> associated with <a href='#SkImage_MakeFromTexture_2_context'>context</a>.  <a href='undocumented#GPU_Texture'>GPU texture</a> must stay
valid and unchanged until <a href='#SkImage_MakeFromTexture_2_textureReleaseProc'>textureReleaseProc</a> is called. <a href='#SkImage_MakeFromTexture_2_textureReleaseProc'>textureReleaseProc</a> is
passed <a href='#SkImage_MakeFromTexture_2_releaseContext'>releaseContext</a> when <a href='SkImage_Reference#SkImage'>SkImage</a> is deleted or no longer refers to <a href='undocumented#Texture'>texture</a>.

<a href='SkImage_Reference#SkImage'>SkImage</a> is returned if format of <a href='#SkImage_MakeFromTexture_2_backendTexture'>backendTexture</a> is recognized and supported.
Recognized formats vary by GPU back-end.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromTexture_2_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromTexture_2_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td><a href='undocumented#Texture'>texture</a> residing on GPU</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromTexture_2_origin'><code><strong>origin</strong></code></a></td>
    <td>one of: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft_GrSurfaceOrigin</a>, <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromTexture_2_colorType'><code><strong>colorType</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>,
<a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>, <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>,
<a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>,
<a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>, <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>,
<a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromTexture_2_alphaType'><code><strong>alphaType</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromTexture_2_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td>range of colors; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromTexture_2_textureReleaseProc'><code><strong>textureReleaseProc</strong></code></a></td>
    <td>function called when <a href='undocumented#Texture'>texture</a> can be released</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromTexture_2_releaseContext'><code><strong>releaseContext</strong></code></a></td>
    <td>state passed to <a href='#SkImage_MakeFromTexture_2_textureReleaseProc'>textureReleaseProc</a></td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, or nullptr

### Example

<div><fiddle-embed name="2b1e46354d823dbb53fa6af570135329" gpu="true"><div><a href='#SkImage_MakeFromTexture_2_textureReleaseProc'>textureReleaseProc</a> may be called at some later <a href='SkPoint_Reference#Point'>point</a> in time. In this example,
<a href='#SkImage_MakeFromTexture_2_textureReleaseProc'>textureReleaseProc</a> has no effect on the drawing.
</div></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromAdoptedTexture'>MakeFromAdoptedTexture</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_MakeFromBackendTexture'>MakeFromBackendTexture</a>

<a name='SkImage_MakeCrossContextFromEncoded'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_MakeCrossContextFromEncoded'>MakeCrossContextFromEncoded</a>(<a href='undocumented#GrContext'>GrContext</a>* context, <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkData'>SkData</a>&gt; <a href='undocumented#Data'>data</a>,
                                                  bool buildMips, <a href='undocumented#SkColorSpace'>SkColorSpace</a>* dstColorSpace,
                                                  bool limitToMaxTextureSize = false)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> from encoded <a href='#SkImage_MakeCrossContextFromEncoded_data'>data</a>. <a href='SkImage_Reference#SkImage'>SkImage</a> is uploaded to GPU back-end using <a href='#SkImage_MakeCrossContextFromEncoded_context'>context</a>.

Created <a href='SkImage_Reference#SkImage'>SkImage</a> is available to other GPU contexts, and is available across thread
boundaries. All contexts must be in the same   <a href='undocumented#GPU_Share_Group'>GPU share group</a>, or otherwise
share resources.

When <a href='SkImage_Reference#SkImage'>SkImage</a> is no longer referenced, <a href='#SkImage_MakeCrossContextFromEncoded_context'>context</a> releases <a href='undocumented#Texture'>texture</a> memory
asynchronously.

<a href='undocumented#GrBackendTexture'>GrBackendTexture</a> decoded from <a href='#SkImage_MakeCrossContextFromEncoded_data'>data</a> is uploaded to match <a href='SkSurface_Reference#SkSurface'>SkSurface</a> created with
<a href='#SkImage_MakeCrossContextFromEncoded_dstColorSpace'>dstColorSpace</a>. <a href='undocumented#SkColorSpace'>SkColorSpace</a> of <a href='SkImage_Reference#SkImage'>SkImage</a> is determined by encoded <a href='#SkImage_MakeCrossContextFromEncoded_data'>data</a>.

<a href='SkImage_Reference#SkImage'>SkImage</a> is returned if format of <a href='#SkImage_MakeCrossContextFromEncoded_data'>data</a> is recognized and supported, and if <a href='#SkImage_MakeCrossContextFromEncoded_context'>context</a>
supports moving resources. Recognized formats vary by platform and GPU back-end.

<a href='SkImage_Reference#SkImage'>SkImage</a> is returned using <a href='#SkImage_MakeFromEncoded'>MakeFromEncoded</a>() if <a href='#SkImage_MakeCrossContextFromEncoded_context'>context</a> is nullptr or does not support
moving resources between contexts.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeCrossContextFromEncoded_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeCrossContextFromEncoded_data'><code><strong>data</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> to decode</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeCrossContextFromEncoded_buildMips'><code><strong>buildMips</strong></code></a></td>
    <td>create <a href='SkImage_Reference#SkImage'>SkImage</a> as  <a href='undocumented#Mip_Map'>mip map</a> if true</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeCrossContextFromEncoded_dstColorSpace'><code><strong>dstColorSpace</strong></code></a></td>
    <td>range of colors of matching <a href='SkSurface_Reference#SkSurface'>SkSurface</a> on GPU</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeCrossContextFromEncoded_limitToMaxTextureSize'><code><strong>limitToMaxTextureSize</strong></code></a></td>
    <td>downscale <a href='SkImage_Reference#Image'>image</a> to GPU maximum <a href='undocumented#Texture'>texture</a> <a href='undocumented#Size'>size</a>, if necessary</td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, or nullptr

### Example

<div><fiddle-embed name="069c7b116479e3ca46f953f07dcbdd36"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeCrossContextFromPixmap'>MakeCrossContextFromPixmap</a>

<a name='SkImage_MakeCrossContextFromPixmap'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_MakeCrossContextFromPixmap'>MakeCrossContextFromPixmap</a>(<a href='undocumented#GrContext'>GrContext</a>* context, const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#Pixmap'>pixmap</a>,
                                                 bool buildMips, <a href='undocumented#SkColorSpace'>SkColorSpace</a>* dstColorSpace,
                                                 bool limitToMaxTextureSize = false)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> from <a href='#SkImage_MakeCrossContextFromPixmap_pixmap'>pixmap</a>. <a href='SkImage_Reference#SkImage'>SkImage</a> is uploaded to GPU back-end using <a href='#SkImage_MakeCrossContextFromPixmap_context'>context</a>.

Created <a href='SkImage_Reference#SkImage'>SkImage</a> is available to other GPU contexts, and is available across thread
boundaries. All contexts must be in the same   <a href='undocumented#GPU_Share_Group'>GPU share group</a>, or otherwise
share resources.

When <a href='SkImage_Reference#SkImage'>SkImage</a> is no longer referenced, <a href='#SkImage_MakeCrossContextFromPixmap_context'>context</a> releases <a href='undocumented#Texture'>texture</a> memory
asynchronously.

<a href='undocumented#GrBackendTexture'>GrBackendTexture</a> created from <a href='#SkImage_MakeCrossContextFromPixmap_pixmap'>pixmap</a> is uploaded to match <a href='SkSurface_Reference#SkSurface'>SkSurface</a> created with
<a href='#SkImage_MakeCrossContextFromPixmap_dstColorSpace'>dstColorSpace</a>. <a href='undocumented#SkColorSpace'>SkColorSpace</a> of <a href='SkImage_Reference#SkImage'>SkImage</a> is determined by <a href='#SkImage_MakeCrossContextFromPixmap_pixmap'>pixmap</a>.<a href='#SkPixmap_colorSpace'>colorSpace</a>().

<a href='SkImage_Reference#SkImage'>SkImage</a> is returned referring to GPU back-end if <a href='#SkImage_MakeCrossContextFromPixmap_context'>context</a> is not nullptr,
format of <a href='undocumented#Data'>data</a> is recognized and supported, and if <a href='#SkImage_MakeCrossContextFromPixmap_context'>context</a> supports moving
resources between contexts. Otherwise, <a href='#SkImage_MakeCrossContextFromPixmap_pixmap'>pixmap</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Data'>data</a> is copied and <a href='SkImage_Reference#SkImage'>SkImage</a>
as returned in raster format if possible; nullptr may be returned.
Recognized GPU formats vary by platform and GPU back-end.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeCrossContextFromPixmap_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeCrossContextFromPixmap_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, <a href='undocumented#Pixel'>pixel</a> address, and row bytes</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeCrossContextFromPixmap_buildMips'><code><strong>buildMips</strong></code></a></td>
    <td>create <a href='SkImage_Reference#SkImage'>SkImage</a> as  <a href='undocumented#Mip_Map'>mip map</a> if true</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeCrossContextFromPixmap_dstColorSpace'><code><strong>dstColorSpace</strong></code></a></td>
    <td>range of colors of matching <a href='SkSurface_Reference#SkSurface'>SkSurface</a> on GPU</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeCrossContextFromPixmap_limitToMaxTextureSize'><code><strong>limitToMaxTextureSize</strong></code></a></td>
    <td>downscale <a href='SkImage_Reference#Image'>image</a> to GPU maximum <a href='undocumented#Texture'>texture</a> <a href='undocumented#Size'>size</a>, if necessary</td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, or nullptr

### Example

<div><fiddle-embed name="45bca8747b8f49b5be34b520897ef048"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeCrossContextFromEncoded'>MakeCrossContextFromEncoded</a>

<a name='SkImage_MakeFromAdoptedTexture'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_MakeFromAdoptedTexture'>MakeFromAdoptedTexture</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                             const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture,
                                             <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> surfaceOrigin, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkImage_colorType'>colorType</a>,
                                             <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImage_alphaType'>alphaType</a> = <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
                                             <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; <a href='#SkImage_colorSpace'>colorSpace</a> = nullptr)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> from <a href='#SkImage_MakeFromAdoptedTexture_backendTexture'>backendTexture</a> associated with <a href='#SkImage_MakeFromAdoptedTexture_context'>context</a>. <a href='#SkImage_MakeFromAdoptedTexture_backendTexture'>backendTexture</a> and
returned <a href='SkImage_Reference#SkImage'>SkImage</a> are managed internally, and are released when no longer needed.

<a href='SkImage_Reference#SkImage'>SkImage</a> is returned if format of <a href='#SkImage_MakeFromAdoptedTexture_backendTexture'>backendTexture</a> is recognized and supported.
Recognized formats vary by GPU back-end.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromAdoptedTexture_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromAdoptedTexture_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td><a href='undocumented#Texture'>texture</a> residing on GPU</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromAdoptedTexture_surfaceOrigin'><code><strong>surfaceOrigin</strong></code></a></td>
    <td>one of: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft_GrSurfaceOrigin</a>, <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromAdoptedTexture_colorType'><code><strong>colorType</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>,
<a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>, <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>,
<a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>,
<a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>, <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>,
<a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromAdoptedTexture_alphaType'><code><strong>alphaType</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromAdoptedTexture_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td>range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, or nullptr

### Example

<div><fiddle-embed name="b034517e39394b7543f06ec885e36d7d" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromTexture'>MakeFromTexture</a> <a href='#SkImage_MakeFromYUVTexturesCopy'>MakeFromYUVTexturesCopy</a>

<a name='SkImage_MakeFromYUVATexturesCopy'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_MakeFromYUVATexturesCopy'>MakeFromYUVATexturesCopy</a>(<a href='undocumented#GrContext'>GrContext</a>* context, <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> yuvColorSpace,
                                               const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> yuvaTextures[],
                                               const <a href='undocumented#SkYUVAIndex'>SkYUVAIndex</a> yuvaIndices[4], <a href='undocumented#SkISize'>SkISize</a> imageSize,
                                               <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> imageOrigin,
                                               <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; imageColorSpace = nullptr)
</pre>

Creates an <a href='SkImage_Reference#SkImage'>SkImage</a> by flattening the specified YUVA planes into a single, interleaved RGBA
<a href='SkImage_Reference#Image'>image</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopy_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopy_yuvColorSpace'><code><strong>yuvColorSpace</strong></code></a></td>
    <td>How the YUV values are converted to RGB. One of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kJPEG_SkYUVColorSpace'>kJPEG_SkYUVColorSpace</a>, <a href='SkImageInfo_Reference#kRec601_SkYUVColorSpace'>kRec601_SkYUVColorSpace</a>,
<a href='SkImageInfo_Reference#kRec709_SkYUVColorSpace'>kRec709_SkYUVColorSpace</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopy_yuvaTextures'><code><strong>yuvaTextures</strong></code></a></td>
    <td>array of (up to four) YUVA textures on GPU which contain the,</td>
  </tr>
</table>

possibly interleaved, YUVA planes

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopy_yuvaIndices'><code><strong>yuvaIndices</strong></code></a></td>
    <td>array indicating which <a href='undocumented#Texture'>texture</a> in <a href='#SkImage_MakeFromYUVATexturesCopy_yuvaTextures'>yuvaTextures</a>, and channel</td>
  </tr>
</table>

in that <a href='undocumented#Texture'>texture</a>, maps to each component of YUVA.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopy_imageSize'><code><strong>imageSize</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> of the resulting <a href='SkImage_Reference#Image'>image</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopy_imageOrigin'><code><strong>imageOrigin</strong></code></a></td>
    <td>origin of the resulting <a href='SkImage_Reference#Image'>image</a>. One of: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft_GrSurfaceOrigin</a>,</td>
  </tr>
</table>

<a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopy_imageColorSpace'><code><strong>imageColorSpace</strong></code></a></td>
    <td>range of colors of the resulting <a href='SkImage_Reference#Image'>image</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, or nullptr

### See Also

<a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend'>MakeFromYUVATexturesCopyWithExternalBackend</a> <a href='#SkImage_MakeFromYUVATextures'>MakeFromYUVATextures</a>

<a name='SkImage_MakeFromYUVATextures'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_MakeFromYUVATextures'>MakeFromYUVATextures</a>(<a href='undocumented#GrContext'>GrContext</a>* context, <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> yuvColorSpace,
                                           const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> yuvaTextures[],
                                           const <a href='undocumented#SkYUVAIndex'>SkYUVAIndex</a> yuvaIndices[4], <a href='undocumented#SkISize'>SkISize</a> imageSize,
                                           <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> imageOrigin,
                                           <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; imageColorSpace = nullptr) ;
</pre>

Creates an <a href='SkImage_Reference#SkImage'>SkImage</a> by storing the specified YUVA planes into an <a href='SkImage_Reference#Image'>image</a>, to be rendered
via multitexturing.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVATextures_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVATextures_yuvColorSpace'><code><strong>yuvColorSpace</strong></code></a></td>
    <td>How the YUV values are converted to RGB. One of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kJPEG_SkYUVColorSpace'>kJPEG_SkYUVColorSpace</a>, <a href='SkImageInfo_Reference#kRec601_SkYUVColorSpace'>kRec601_SkYUVColorSpace</a>,
<a href='SkImageInfo_Reference#kRec709_SkYUVColorSpace'>kRec709_SkYUVColorSpace</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVATextures_yuvaTextures'><code><strong>yuvaTextures</strong></code></a></td>
    <td>array of (up to four) YUVA textures on GPU which contain the,</td>
  </tr>
</table>

possibly interleaved, YUVA planes

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVATextures_yuvaIndices'><code><strong>yuvaIndices</strong></code></a></td>
    <td>array indicating which <a href='undocumented#Texture'>texture</a> in <a href='#SkImage_MakeFromYUVATextures_yuvaTextures'>yuvaTextures</a>, and channel</td>
  </tr>
</table>

in that <a href='undocumented#Texture'>texture</a>, maps to each component of YUVA.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVATextures_imageSize'><code><strong>imageSize</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> of the resulting <a href='SkImage_Reference#Image'>image</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVATextures_imageOrigin'><code><strong>imageOrigin</strong></code></a></td>
    <td>origin of the resulting <a href='SkImage_Reference#Image'>image</a>. One of: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft_GrSurfaceOrigin</a>,</td>
  </tr>
</table>

<a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVATextures_imageColorSpace'><code><strong>imageColorSpace</strong></code></a></td>
    <td>range of colors of the resulting <a href='SkImage_Reference#Image'>image</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, or nullptr

### See Also

<a href='#SkImage_MakeFromYUVATexturesCopy'>MakeFromYUVATexturesCopy</a> <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend'>MakeFromYUVATexturesCopyWithExternalBackend</a>

<a name='SkImage_MakeFromYUVAPixmaps'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; 
             <a href='#SkImage_MakeFromYUVAPixmaps'>MakeFromYUVAPixmaps</a>(
                                               <a href='undocumented#GrContext'>GrContext</a>* context,
             <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> yuvColorSpace, const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> yuvaPixmaps[],
             const <a href='undocumented#SkYUVAIndex'>SkYUVAIndex</a> yuvaIndices[4], <a href='undocumented#SkISize'>SkISize</a> imageSize, <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> imageOrigin,
             bool buildMips, bool limitToMaxTextureSize = false,
             <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; imageColorSpace = nullptr) ;
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> from <a href='SkPixmap_Reference#Pixmap'>pixmap</a> array representing YUVA <a href='undocumented#Data'>data</a>.
<a href='SkImage_Reference#SkImage'>SkImage</a> is uploaded to GPU back-end using <a href='#SkImage_MakeFromYUVAPixmaps_context'>context</a>.

Each <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> created from <a href='#SkImage_MakeFromYUVAPixmaps_yuvaPixmaps'>yuvaPixmaps</a> array is uploaded to match <a href='SkSurface_Reference#SkSurface'>SkSurface</a>
using <a href='undocumented#SkColorSpace'>SkColorSpace</a> of <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>. <a href='undocumented#SkColorSpace'>SkColorSpace</a> of <a href='SkImage_Reference#SkImage'>SkImage</a> is determined by <a href='#SkImage_MakeFromYUVAPixmaps_imageColorSpace'>imageColorSpace</a>.

<a href='SkImage_Reference#SkImage'>SkImage</a> is returned referring to GPU back-end if <a href='#SkImage_MakeFromYUVAPixmaps_context'>context</a> is not nullptr and
format of <a href='undocumented#Data'>data</a> is recognized and supported. Otherwise, nullptr is returned.
Recognized GPU formats vary by platform and GPU back-end.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVAPixmaps_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVAPixmaps_yuvColorSpace'><code><strong>yuvColorSpace</strong></code></a></td>
    <td>How the YUV values are converted to RGB. One of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kJPEG_SkYUVColorSpace'>kJPEG_SkYUVColorSpace</a>, <a href='SkImageInfo_Reference#kRec601_SkYUVColorSpace'>kRec601_SkYUVColorSpace</a>,
<a href='SkImageInfo_Reference#kRec709_SkYUVColorSpace'>kRec709_SkYUVColorSpace</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVAPixmaps_yuvaPixmaps'><code><strong>yuvaPixmaps</strong></code></a></td>
    <td>array of (up to four) <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> which contain the,</td>
  </tr>
</table>

possibly interleaved, YUVA planes

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVAPixmaps_yuvaIndices'><code><strong>yuvaIndices</strong></code></a></td>
    <td>array indicating which <a href='SkPixmap_Reference#Pixmap'>pixmap</a> in <a href='#SkImage_MakeFromYUVAPixmaps_yuvaPixmaps'>yuvaPixmaps</a>, and channel</td>
  </tr>
</table>

in that <a href='SkPixmap_Reference#Pixmap'>pixmap</a>, maps to each component of YUVA.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVAPixmaps_imageSize'><code><strong>imageSize</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> of the resulting <a href='SkImage_Reference#Image'>image</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVAPixmaps_imageOrigin'><code><strong>imageOrigin</strong></code></a></td>
    <td>origin of the resulting <a href='SkImage_Reference#Image'>image</a>. One of:</td>
  </tr>
</table>

<a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft_GrSurfaceOrigin</a>, <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVAPixmaps_buildMips'><code><strong>buildMips</strong></code></a></td>
    <td>create internal YUVA textures as  <a href='undocumented#Mip_Map'>mip map</a> if true</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVAPixmaps_limitToMaxTextureSize'><code><strong>limitToMaxTextureSize</strong></code></a></td>
    <td>downscale <a href='SkImage_Reference#Image'>image</a> to GPU maximum <a href='undocumented#Texture'>texture</a> <a href='undocumented#Size'>size</a>, if necessary</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVAPixmaps_imageColorSpace'><code><strong>imageColorSpace</strong></code></a></td>
    <td>range of colors of the resulting <a href='SkImage_Reference#Image'>image</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, or nullptr

### See Also

<a href='#SkImage_MakeFromYUVATextures'>MakeFromYUVATextures</a>

<a name='SkImage_MakeFromYUVATexturesCopyWithExternalBackend'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend'>MakeFromYUVATexturesCopyWithExternalBackend</a>(
            <a href='undocumented#GrContext'>GrContext</a>* context,
                        <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> yuvColorSpace, const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> yuvaTextures[],
                        const <a href='undocumented#SkYUVAIndex'>SkYUVAIndex</a> yuvaIndices[4], <a href='undocumented#SkISize'>SkISize</a> imageSize,
                        <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> imageOrigin, const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture,
                        <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; imageColorSpace = nullptr)
</pre>

Creates an <a href='SkImage_Reference#SkImage'>SkImage</a> by flattening the specified YUVA planes into a single, interleaved RGBA
<a href='SkImage_Reference#Image'>image</a>. '<a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend_backendTexture'>backendTexture</a>' is used to store the result of the flattening.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopyWithExternalBackend_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopyWithExternalBackend_yuvColorSpace'><code><strong>yuvColorSpace</strong></code></a></td>
    <td>How the YUV values are converted to RGB. One of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kJPEG_SkYUVColorSpace'>kJPEG_SkYUVColorSpace</a>, <a href='SkImageInfo_Reference#kRec601_SkYUVColorSpace'>kRec601_SkYUVColorSpace</a>,
<a href='SkImageInfo_Reference#kRec709_SkYUVColorSpace'>kRec709_SkYUVColorSpace</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopyWithExternalBackend_yuvaTextures'><code><strong>yuvaTextures</strong></code></a></td>
    <td>array of (up to four) YUVA textures on GPU which contain the,</td>
  </tr>
</table>

possibly interleaved, YUVA planes

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopyWithExternalBackend_yuvaIndices'><code><strong>yuvaIndices</strong></code></a></td>
    <td>array indicating which <a href='undocumented#Texture'>texture</a> in <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend_yuvaTextures'>yuvaTextures</a>, and channel</td>
  </tr>
</table>

in that <a href='undocumented#Texture'>texture</a>, maps to each component of YUVA.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopyWithExternalBackend_imageSize'><code><strong>imageSize</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> of the resulting <a href='SkImage_Reference#Image'>image</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopyWithExternalBackend_imageOrigin'><code><strong>imageOrigin</strong></code></a></td>
    <td>origin of the resulting <a href='SkImage_Reference#Image'>image</a>. One of: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft_GrSurfaceOrigin</a>,</td>
  </tr>
</table>

<a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopyWithExternalBackend_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td>the resource that stores the final pixels</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopyWithExternalBackend_imageColorSpace'><code><strong>imageColorSpace</strong></code></a></td>
    <td>range of colors of the resulting <a href='SkImage_Reference#Image'>image</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, or nullptr

### See Also

<a href='#SkImage_MakeFromYUVATexturesCopy'>MakeFromYUVATexturesCopy</a> <a href='#SkImage_MakeFromYUVATextures'>MakeFromYUVATextures</a>

<a name='SkImage_MakeFromYUVTexturesCopy'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_MakeFromYUVTexturesCopy'>MakeFromYUVTexturesCopy</a>(<a href='undocumented#GrContext'>GrContext</a>* context, <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> yuvColorSpace,
                                              const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> yuvTextures[3],
                                              <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> imageOrigin,
                                              <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; imageColorSpace = nullptr)
</pre>

To be deprecated.

### See Also

<a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend'>MakeFromYUVTexturesCopyWithExternalBackend</a> <a href='#SkImage_MakeFromNV12TexturesCopy'>MakeFromNV12TexturesCopy</a> <a href='#SkImage_MakeFromYUVATexturesCopy'>MakeFromYUVATexturesCopy</a>

<a name='SkImage_MakeFromYUVTexturesCopyWithExternalBackend'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend'>MakeFromYUVTexturesCopyWithExternalBackend</a>(
        <a href='undocumented#GrContext'>GrContext</a>* context,
                             <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> yuvColorSpace, const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> yuvTextures[3],
                             <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> imageOrigin, const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture,
                             <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; imageColorSpace = nullptr) ;
</pre>

To be deprecated.

### See Also

<a href='#SkImage_MakeFromYUVTexturesCopy'>MakeFromYUVTexturesCopy</a> <a href='#SkImage_MakeFromNV12TexturesCopy'>MakeFromNV12TexturesCopy</a> <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend'>MakeFromYUVATexturesCopyWithExternalBackend</a>

<a name='SkImage_MakeFromNV12TexturesCopy'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_MakeFromNV12TexturesCopy'>MakeFromNV12TexturesCopy</a>(<a href='undocumented#GrContext'>GrContext</a>* context, <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> yuvColorSpace,
                                               const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> nv12Textures[2],
                                               <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> imageOrigin,
                                               <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; imageColorSpace = nullptr)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> from copy of <a href='#SkImage_MakeFromNV12TexturesCopy_nv12Textures'>nv12Textures</a>, an array of textures on GPU.
<a href='#SkImage_MakeFromNV12TexturesCopy_nv12Textures'>nv12Textures</a>[0] contains pixels for   <a href='undocumented#YUV_Component_Y'>YUV component y</a> plane.
<a href='#SkImage_MakeFromNV12TexturesCopy_nv12Textures'>nv12Textures</a>[1] contains pixels for   <a href='undocumented#YUV_Component_U'>YUV component u</a> plane,
followed by pixels for   <a href='undocumented#YUV_Component_V'>YUV component v</a> plane.
Returned <a href='SkImage_Reference#SkImage'>SkImage</a> has the dimensions <a href='#SkImage_MakeFromNV12TexturesCopy_nv12Textures'>nv12Textures</a>[2].
<a href='#SkImage_MakeFromNV12TexturesCopy_yuvColorSpace'>yuvColorSpace</a> describes how YUV colors convert to RGB colors.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromNV12TexturesCopy_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromNV12TexturesCopy_yuvColorSpace'><code><strong>yuvColorSpace</strong></code></a></td>
    <td>one of: <a href='SkImageInfo_Reference#kJPEG_SkYUVColorSpace'>kJPEG_SkYUVColorSpace</a>, <a href='SkImageInfo_Reference#kRec601_SkYUVColorSpace'>kRec601_SkYUVColorSpace</a>,</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kRec709_SkYUVColorSpace'>kRec709_SkYUVColorSpace</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromNV12TexturesCopy_nv12Textures'><code><strong>nv12Textures</strong></code></a></td>
    <td>array of YUV textures on GPU</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromNV12TexturesCopy_imageOrigin'><code><strong>imageOrigin</strong></code></a></td>
    <td>one of: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft_GrSurfaceOrigin</a>, <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromNV12TexturesCopy_imageColorSpace'><code><strong>imageColorSpace</strong></code></a></td>
    <td>range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, or nullptr

### See Also

<a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend'>MakeFromNV12TexturesCopyWithExternalBackend</a> <a href='#SkImage_MakeFromYUVTexturesCopy'>MakeFromYUVTexturesCopy</a> <a href='#SkImage_MakeFromYUVATexturesCopy'>MakeFromYUVATexturesCopy</a>

<a name='SkImage_MakeFromNV12TexturesCopyWithExternalBackend'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend'>MakeFromNV12TexturesCopyWithExternalBackend</a>(
            <a href='undocumented#GrContext'>GrContext</a>* context,
                        <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> yuvColorSpace, const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> nv12Textures[2],
                        <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> imageOrigin, const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture,
                        <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; imageColorSpace = nullptr) ;
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> from copy of <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>nv12Textures</a>, an array of textures on GPU.
<a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>nv12Textures</a>[0] contains pixels for   <a href='undocumented#YUV_Component_Y'>YUV component y</a> plane.
<a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>nv12Textures</a>[1] contains pixels for   <a href='undocumented#YUV_Component_U'>YUV component u</a> plane,
followed by pixels for   <a href='undocumented#YUV_Component_V'>YUV component v</a> plane.
Returned <a href='SkImage_Reference#SkImage'>SkImage</a> has the dimensions <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>nv12Textures</a>[2] and stores pixels in <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_backendTexture'>backendTexture</a>.
<a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_yuvColorSpace'>yuvColorSpace</a> describes how YUV colors convert to RGB colors.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromNV12TexturesCopyWithExternalBackend_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromNV12TexturesCopyWithExternalBackend_yuvColorSpace'><code><strong>yuvColorSpace</strong></code></a></td>
    <td>one of: <a href='SkImageInfo_Reference#kJPEG_SkYUVColorSpace'>kJPEG_SkYUVColorSpace</a>, <a href='SkImageInfo_Reference#kRec601_SkYUVColorSpace'>kRec601_SkYUVColorSpace</a>,</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kRec709_SkYUVColorSpace'>kRec709_SkYUVColorSpace</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'><code><strong>nv12Textures</strong></code></a></td>
    <td>array of YUV textures on GPU</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromNV12TexturesCopyWithExternalBackend_imageOrigin'><code><strong>imageOrigin</strong></code></a></td>
    <td>one of: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft_GrSurfaceOrigin</a>, <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromNV12TexturesCopyWithExternalBackend_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td>the resource that stores the final pixels</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromNV12TexturesCopyWithExternalBackend_imageColorSpace'><code><strong>imageColorSpace</strong></code></a></td>
    <td>range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, or nullptr

### See Also

<a href='#SkImage_MakeFromNV12TexturesCopy'>MakeFromNV12TexturesCopy</a> <a href='#SkImage_MakeFromYUVTexturesCopy'>MakeFromYUVTexturesCopy</a> <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend'>MakeFromYUVATexturesCopyWithExternalBackend</a>

<a name='SkImage_BitDepth'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum class <a href='#SkImage_BitDepth'>BitDepth</a> {
        <a href='#SkImage_BitDepth_kU8'>kU8</a>,
        <a href='#SkImage_BitDepth_kF16'>kF16</a>,
    };

</pre>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkImage_BitDepth_kU8'><code>SkImage::BitDepth::kU8</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Use 8 bits per ARGB component using unsigned integer format.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkImage_BitDepth_kF16'><code>SkImage::BitDepth::kF16</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Use 16 bits per ARGB component using half-precision floating <a href='SkPoint_Reference#Point'>point</a> format.
</td>
  </tr>
</table>

### See Also

<a href='#SkImage_MakeFromPicture'>MakeFromPicture</a>

<a name='SkImage_MakeFromPicture'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_MakeFromPicture'>MakeFromPicture</a>(<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkPicture_Reference#SkPicture'>SkPicture</a>&gt; <a href='SkPicture_Reference#Picture'>picture</a>, const <a href='undocumented#SkISize'>SkISize</a>& dimensions,
                                      const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#Matrix'>matrix</a>, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                                      <a href='#SkImage_BitDepth'>BitDepth</a> bitDepth, <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; <a href='#SkImage_colorSpace'>colorSpace</a>)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> from <a href='#SkImage_MakeFromPicture_picture'>picture</a>. Returned <a href='SkImage_Reference#SkImage'>SkImage</a> width and height are set by <a href='#SkImage_MakeFromPicture_dimensions'>dimensions</a>.
<a href='SkImage_Reference#SkImage'>SkImage</a> draws <a href='#SkImage_MakeFromPicture_picture'>picture</a> with <a href='#SkImage_MakeFromPicture_matrix'>matrix</a> and <a href='#SkImage_MakeFromPicture_paint'>paint</a>, set to <a href='#SkImage_MakeFromPicture_bitDepth'>bitDepth</a> and <a href='#SkImage_MakeFromPicture_colorSpace'>colorSpace</a>.

If <a href='#SkImage_MakeFromPicture_matrix'>matrix</a> is nullptr, draws with identity <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. If <a href='#SkImage_MakeFromPicture_paint'>paint</a> is nullptr, draws
with default <a href='SkPaint_Reference#SkPaint'>SkPaint</a>. <a href='#SkImage_MakeFromPicture_colorSpace'>colorSpace</a> may be nullptr.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromPicture_picture'><code><strong>picture</strong></code></a></td>
    <td><a href='SkStream_Reference#Stream'>stream</a> of drawing commands</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromPicture_dimensions'><code><strong>dimensions</strong></code></a></td>
    <td>width and height</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromPicture_matrix'><code><strong>matrix</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to rotate, scale, translate, and so on; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromPicture_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> to apply transparency, filtering, and so on; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromPicture_bitDepth'><code><strong>bitDepth</strong></code></a></td>
    <td>8-bit integer or 16-bit float: per component</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromPicture_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td>range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, or nullptr

### Example

<div><fiddle-embed name="4aa2879b9e44dfd6648995326d2c4dcf"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawPicture'>drawPicture</a>

<a name='SkImage_MakeFromAHardwareBuffer'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_MakeFromAHardwareBuffer'>MakeFromAHardwareBuffer</a>(
        AHardwareBuffer* hardwareBuffer,
                                   <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImage_alphaType'>alphaType</a> = <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
                                   <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; <a href='#SkImage_colorSpace'>colorSpace</a> = nullptr,
                                   <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> surfaceOrigin = <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a>)
</pre>

(See Skia bug 7447)
Creates <a href='SkImage_Reference#SkImage'>SkImage</a> from Android hardware buffer.
Returned <a href='SkImage_Reference#SkImage'>SkImage</a> takes a reference on the buffer.

Only available on Android, when __ANDROID_API__ is defined to be 26 or greater.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromAHardwareBuffer_hardwareBuffer'><code><strong>hardwareBuffer</strong></code></a></td>
    <td>AHardwareBuffer Android hardware buffer</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromAHardwareBuffer_alphaType'><code><strong>alphaType</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromAHardwareBuffer_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td>range of colors; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromAHardwareBuffer_surfaceOrigin'><code><strong>surfaceOrigin</strong></code></a></td>
    <td>one of: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft_GrSurfaceOrigin</a>, <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a></td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, or nullptr

### See Also

<a href='#SkImage_MakeFromRaster'>MakeFromRaster</a>

<a name='Property'></a>

<a name='SkImage_width'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkImage_width'>width()</a>const
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> count in each row.

### Return Value

<a href='undocumented#Pixel'>pixel</a> width in <a href='SkImage_Reference#SkImage'>SkImage</a>

### Example

<div><fiddle-embed name="9aec65fc252ffc9982fa8867433eca18"></fiddle-embed></div>

### See Also

<a href='#SkImage_dimensions'>dimensions()</a> <a href='#SkImage_height'>height()</a>

<a name='SkImage_height'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkImage_height'>height()</a>const
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> row count.

### Return Value

<a href='undocumented#Pixel'>pixel</a> height in <a href='SkImage_Reference#SkImage'>SkImage</a>

### Example

<div><fiddle-embed name="a4f53a0b6ac85e7bc3887245b728530d"></fiddle-embed></div>

### See Also

<a href='#SkImage_dimensions'>dimensions()</a> <a href='#SkImage_width'>width()</a>

<a name='SkImage_dimensions'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkISize'>SkISize</a> <a href='#SkImage_dimensions'>dimensions()</a>const
</pre>

Returns <a href='undocumented#SkISize'>SkISize</a> { <a href='#SkImage_width'>width()</a>, <a href='#SkImage_height'>height()</a> }.

### Return Value

integral <a href='undocumented#Size'>size</a> of <a href='#SkImage_width'>width()</a> and <a href='#SkImage_height'>height()</a>

### Example

<div><fiddle-embed name="96b4bc43b3667df9ba9e2dafb770d33c">

#### Example Output

~~~~
dimensionsAsBounds == bounds
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImage_height'>height()</a> <a href='#SkImage_width'>width()</a> <a href='#SkImage_bounds'>bounds()</a>

<a name='SkImage_bounds'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkImage_bounds'>bounds()</a>const
</pre>

Returns <a href='SkIRect_Reference#SkIRect'>SkIRect</a> { 0, 0, <a href='#SkImage_width'>width()</a>, <a href='#SkImage_height'>height()</a> }.

### Return Value

integral rectangle from origin to <a href='#SkImage_width'>width()</a> and <a href='#SkImage_height'>height()</a>

### Example

<div><fiddle-embed name="c204b38b3fc08914b0a634aa4eaec894"></fiddle-embed></div>

### See Also

<a href='#SkImage_dimensions'>dimensions()</a>

<a name='SkImage_uniqueID'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkImage_uniqueID'>uniqueID</a>()const
</pre>

Returns value unique to <a href='SkImage_Reference#Image'>image</a>. <a href='SkImage_Reference#SkImage'>SkImage</a> contents cannot change after <a href='SkImage_Reference#SkImage'>SkImage</a> is
created. Any operation to create a new <a href='SkImage_Reference#SkImage'>SkImage</a> will receive generate a new
unique number.

### Return Value

unique identifier

### Example

<div><fiddle-embed name="d70194c9c51e700335f95de91846d023"></fiddle-embed></div>

### See Also

<a href='#SkImage_isLazyGenerated'>isLazyGenerated</a>

<a name='SkImage_alphaType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImage_alphaType'>alphaType</a>()const
</pre>

Returns <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, one of: <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>
.

<a href='#Image_Info_Alpha_Type'>Alpha_Type</a> returned was a parameter to an <a href='SkImage_Reference#Image'>Image</a> constructor,
or was parsed from encoded <a href='undocumented#Data'>data</a>.

### Return Value

<a href='#Image_Info_Alpha_Type'>Alpha_Type</a> in <a href='SkImage_Reference#Image'>Image</a>

### Example

<div><fiddle-embed name="1b9f1f05026ceb14ccb6926a13cdaa83"></fiddle-embed></div>

### See Also

<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_alphaType'>alphaType</a>

<a name='SkImage_colorType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkImage_colorType'>colorType</a>()const
</pre>

Returns <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> if known; otherwise, returns <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>.

### Return Value

<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> of <a href='SkImage_Reference#SkImage'>SkImage</a>

### Example

<div><fiddle-embed name="50396fad4a128f58e400ca00fe09711f"></fiddle-embed></div>

### See Also

<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_colorType'>colorType</a>

<a name='SkImage_colorSpace'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='#SkImage_colorSpace'>colorSpace</a>()const
</pre>

Returns <a href='undocumented#SkColorSpace'>SkColorSpace</a>, the range of colors, associated with <a href='SkImage_Reference#SkImage'>SkImage</a>.  The
reference count of <a href='undocumented#SkColorSpace'>SkColorSpace</a> is unchanged. The returned <a href='undocumented#SkColorSpace'>SkColorSpace</a> is
immutable.

<a href='undocumented#SkColorSpace'>SkColorSpace</a> returned was passed to an <a href='SkImage_Reference#SkImage'>SkImage</a> constructor,
or was parsed from encoded <a href='undocumented#Data'>data</a>. <a href='undocumented#SkColorSpace'>SkColorSpace</a> returned may be ignored when <a href='SkImage_Reference#SkImage'>SkImage</a>
is drawn, depending on the capabilities of the <a href='SkSurface_Reference#SkSurface'>SkSurface</a> receiving the drawing.

### Return Value

<a href='undocumented#SkColorSpace'>SkColorSpace</a> in <a href='SkImage_Reference#SkImage'>SkImage</a>, or nullptr

### Example

<div><fiddle-embed name="4468d573f42af6f5e234be10a5453bb2"></fiddle-embed></div>

### See Also

<a href='#SkImage_refColorSpace'>refColorSpace</a> <a href='#SkImage_makeColorSpace'>makeColorSpace</a>

<a name='SkImage_refColorSpace'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; <a href='#SkImage_refColorSpace'>refColorSpace</a>()const
</pre>

Returns a smart pointer to <a href='undocumented#SkColorSpace'>SkColorSpace</a>, the range of colors, associated with
<a href='SkImage_Reference#SkImage'>SkImage</a>.  The smart pointer tracks the number of objects sharing this
<a href='undocumented#SkColorSpace'>SkColorSpace</a> reference so the memory is released when the owners destruct.

The returned <a href='undocumented#SkColorSpace'>SkColorSpace</a> is immutable.

<a href='undocumented#SkColorSpace'>SkColorSpace</a> returned was passed to an <a href='SkImage_Reference#SkImage'>SkImage</a> constructor,
or was parsed from encoded <a href='undocumented#Data'>data</a>. <a href='undocumented#SkColorSpace'>SkColorSpace</a> returned may be ignored when <a href='SkImage_Reference#SkImage'>SkImage</a>
is drawn, depending on the capabilities of the <a href='SkSurface_Reference#SkSurface'>SkSurface</a> receiving the drawing.

### Return Value

<a href='undocumented#SkColorSpace'>SkColorSpace</a> in <a href='SkImage_Reference#SkImage'>SkImage</a>, or nullptr, wrapped in a smart pointer

### Example

<div><fiddle-embed name="59b2078ebfbda8736a57c0486ae33332"></fiddle-embed></div>

### See Also

<a href='#SkImage_colorSpace'>colorSpace</a> <a href='#SkImage_makeColorSpace'>makeColorSpace</a>

<a name='SkImage_isAlphaOnly'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_isAlphaOnly'>isAlphaOnly</a>()const
</pre>

Returns true if <a href='SkImage_Reference#SkImage'>SkImage</a> pixels represent transparency only. If true, each <a href='undocumented#Pixel'>pixel</a>
is packed in 8 bits as defined by <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>.

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

<a href='#SkImage_alphaType'>alphaType</a> <a href='#SkImage_isOpaque'>isOpaque</a>

<a name='SkImage_isOpaque'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_isOpaque'>isOpaque</a>()const
</pre>

Returns true if pixels ignore their <a href='SkColor_Reference#Alpha'>alpha</a> value and are treated as fully opaque.

### Return Value

true if <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> is <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>

### Example

<div><fiddle-embed name="e3340460003b74ee286d625e68589d65">

#### Example Output

~~~~
isOpaque = false
isOpaque = true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImage_alphaType'>alphaType</a> <a href='#SkImage_isAlphaOnly'>isAlphaOnly</a>

<a name='SkImage_makeShader'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkShader'>SkShader</a>&gt; <a href='#SkImage_makeShader'>makeShader</a>(<a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_TileMode'>TileMode</a> tileMode1, <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_TileMode'>TileMode</a> tileMode2,
                           const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* localMatrix = nullptr)const
</pre>

Creates <a href='undocumented#SkShader'>SkShader</a> from <a href='SkImage_Reference#SkImage'>SkImage</a>. <a href='undocumented#SkShader'>SkShader</a> dimensions are taken from <a href='SkImage_Reference#SkImage'>SkImage</a>. <a href='undocumented#SkShader'>SkShader</a> uses
<a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_TileMode'>TileMode</a> rules to fill drawn area outside <a href='SkImage_Reference#SkImage'>SkImage</a>. <a href='#SkImage_makeShader_localMatrix'>localMatrix</a> permits
transforming <a href='SkImage_Reference#SkImage'>SkImage</a> before <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkMatrix_Reference#Matrix'>matrix</a> is applied.

### Parameters

<table>  <tr>    <td><a name='SkImage_makeShader_tileMode1'><code><strong>tileMode1</strong></code></a></td>
    <td>tiling on x-axis, one of: <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a>,</td>
  </tr>
</table>

<a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kRepeat_TileMode'>kRepeat_TileMode</a>, <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kMirror_TileMode'>kMirror_TileMode</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_makeShader_tileMode2'><code><strong>tileMode2</strong></code></a></td>
    <td>tiling on y-axis, one of: <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a>,</td>
  </tr>
</table>

<a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kRepeat_TileMode'>kRepeat_TileMode</a>, <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kMirror_TileMode'>kMirror_TileMode</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_makeShader_localMatrix'><code><strong>localMatrix</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> transformation, or nullptr</td>
  </tr>
</table>

### Return Value

<a href='undocumented#SkShader'>SkShader</a> containing <a href='SkImage_Reference#SkImage'>SkImage</a>

### Example

<div><fiddle-embed name="1c6de6fe72b00b5be970f5f718363449"></fiddle-embed></div>

### See Also

<a href='#SkImage_scalePixels'>scalePixels</a>

<a name='SkImage_makeShader_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkShader'>SkShader</a>&gt; <a href='#SkImage_makeShader'>makeShader</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* localMatrix = nullptr)const
</pre>

Creates <a href='undocumented#SkShader'>SkShader</a> from <a href='SkImage_Reference#SkImage'>SkImage</a>. <a href='undocumented#SkShader'>SkShader</a> dimensions are taken from <a href='SkImage_Reference#SkImage'>SkImage</a>. <a href='undocumented#SkShader'>SkShader</a> uses
<a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> to fill drawn area outside <a href='SkImage_Reference#SkImage'>SkImage</a>. <a href='#SkImage_makeShader_2_localMatrix'>localMatrix</a> permits
transforming <a href='SkImage_Reference#SkImage'>SkImage</a> before <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkMatrix_Reference#Matrix'>matrix</a> is applied.

### Parameters

<table>  <tr>    <td><a name='SkImage_makeShader_2_localMatrix'><code><strong>localMatrix</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> transformation, or nullptr</td>
  </tr>
</table>

### Return Value

<a href='undocumented#SkShader'>SkShader</a> containing <a href='SkImage_Reference#SkImage'>SkImage</a>

### Example

<div><fiddle-embed name="10172fca71b9dbdcade772513ffeb27e"></fiddle-embed></div>

### See Also

<a href='#SkImage_scalePixels'>scalePixels</a>

<a name='Pixels'></a>

<a name='SkImage_peekPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_peekPixels'>peekPixels</a>(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>* <a href='SkPixmap_Reference#Pixmap'>pixmap</a>)const
</pre>

Copies <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='undocumented#Pixel'>pixel</a> address, row bytes, and <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> to <a href='#SkImage_peekPixels_pixmap'>pixmap</a>, if address
is available, and returns true. If <a href='undocumented#Pixel'>pixel</a> address is not available, return
false and leave <a href='#SkImage_peekPixels_pixmap'>pixmap</a> unchanged.

### Parameters

<table>  <tr>    <td><a name='SkImage_peekPixels_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td>storage for <a href='undocumented#Pixel'>pixel</a> state if pixels are readable; otherwise, ignored</td>
  </tr>
</table>

### Return Value

true if <a href='SkImage_Reference#SkImage'>SkImage</a> has direct access to pixels

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

<a href='#SkImage_readPixels'>readPixels</a>

<a name='SkImage_isTextureBacked'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_isTextureBacked'>isTextureBacked</a>()const
</pre>

Returns true the contents of <a href='SkImage_Reference#SkImage'>SkImage</a> was created on or uploaded to GPU memory,
and is available as a GPU <a href='undocumented#Texture'>texture</a>.

### Return Value

true if <a href='SkImage_Reference#SkImage'>SkImage</a> is a GPU <a href='undocumented#Texture'>texture</a>

### Example

<div><fiddle-embed name="9cf5c62a3d2243e6577ae563f360ea9d" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromTexture'>MakeFromTexture</a> <a href='#SkImage_isValid'>isValid</a>

<a name='SkImage_isValid'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_isValid'>isValid</a>(<a href='undocumented#GrContext'>GrContext</a>* context)const
</pre>

Returns true if <a href='SkImage_Reference#SkImage'>SkImage</a> can be drawn on either  <a href='undocumented#Raster_Surface'>raster surface</a> or  <a href='undocumented#GPU_Surface'>GPU surface</a>.
If <a href='#SkImage_isValid_context'>context</a> is nullptr, tests if <a href='SkImage_Reference#SkImage'>SkImage</a> draws on  <a href='undocumented#Raster_Surface'>raster surface</a>;
otherwise, tests if <a href='SkImage_Reference#SkImage'>SkImage</a> draws on  <a href='undocumented#GPU_Surface'>GPU surface</a> associated with <a href='#SkImage_isValid_context'>context</a>.

<a href='SkImage_Reference#SkImage'>SkImage</a> backed by  <a href='undocumented#GPU_Texture'>GPU texture</a> may become invalid if associated <a href='undocumented#GrContext'>GrContext</a> is
invalid.  <a href='#Lazy_Image'>lazy image</a> may be invalid and may not draw to  <a href='undocumented#Raster_Surface'>raster surface</a> or
<a href='undocumented#GPU_Surface'>GPU surface</a> or both.

### Parameters

<table>  <tr>    <td><a name='SkImage_isValid_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkImage_Reference#SkImage'>SkImage</a> can be drawn

### Example

<div><fiddle-embed name="afc62f38aebc56af8e425297ec67dd37" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_isTextureBacked'>isTextureBacked</a> <a href='#SkImage_isLazyGenerated'>isLazyGenerated</a>

<a name='SkImage_getBackendTexture'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='#SkImage_getBackendTexture'>getBackendTexture</a>(bool flushPendingGrContextIO, <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a>* origin = nullptr)const
</pre>

Retrieves the back-end <a href='undocumented#Texture'>texture</a>. If <a href='SkImage_Reference#SkImage'>SkImage</a> has no back-end <a href='undocumented#Texture'>texture</a>, an invalid
object is returned. Call <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>::<a href='#GrBackendTexture_isValid'>isValid</a> to determine if the result
is valid.

If <a href='#SkImage_getBackendTexture_flushPendingGrContextIO'>flushPendingGrContextIO</a> is true, completes deferred I/O operations.

If <a href='#SkImage_getBackendTexture_origin'>origin</a> in not nullptr, copies location of content drawn into <a href='SkImage_Reference#SkImage'>SkImage</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_getBackendTexture_flushPendingGrContextIO'><code><strong>flushPendingGrContextIO</strong></code></a></td>
    <td>flag to flush outstanding requests</td>
  </tr>
  <tr>    <td><a name='SkImage_getBackendTexture_origin'><code><strong>origin</strong></code></a></td>
    <td>storage for one of: <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a>,</td>
  </tr>
</table>

<a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft_GrSurfaceOrigin</a>; or nullptr

### Return Value

back-end API <a href='undocumented#Texture'>texture</a> handle; invalid on failure

### Example

<div><fiddle-embed name="d093aad721261f421c4bef4a296aab48" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromTexture'>MakeFromTexture</a> <a href='#SkImage_isTextureBacked'>isTextureBacked</a>

<a name='SkImage_CachingHint'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkImage_CachingHint'>CachingHint</a> {
        <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>,
        <a href='#SkImage_kDisallow_CachingHint'>kDisallow_CachingHint</a>,
    };

</pre>

<a href='#SkImage_CachingHint'>CachingHint</a> selects whether Skia may internally cache <a href='SkBitmap_Reference#Bitmap'>Bitmaps</a> generated by
decoding <a href='SkImage_Reference#Image'>Image</a>, or by copying <a href='SkImage_Reference#Image'>Image</a> from GPU to CPU. The default behavior
allows caching <a href='SkBitmap_Reference#Bitmap'>Bitmaps</a>.

Choose <a href='#SkImage_kDisallow_CachingHint'>kDisallow_CachingHint</a> if <a href='SkImage_Reference#Image'>Image</a> pixels are to be used only once, or
if <a href='SkImage_Reference#Image'>Image</a> pixels reside in a cache outside of Skia, or to reduce memory pressure.

Choosing <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a> does not ensure that pixels will be cached.
<a href='SkImage_Reference#Image'>Image</a> pixels may not be cached if memory requirements are too large or
pixels are not accessible.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkImage_kAllow_CachingHint'><code>SkImage::kAllow_CachingHint</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
allows internally caching decoded and copied pixels</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkImage_kDisallow_CachingHint'><code>SkImage::kDisallow_CachingHint</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
disallows internally caching decoded and copied pixels</td>
  </tr>
</table>

### See Also

<a href='#SkImage_readPixels'>readPixels</a> <a href='#SkImage_scalePixels'>scalePixels</a>

<a name='SkImage_readPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_readPixels'>readPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& dstInfo, void* dstPixels, size_t dstRowBytes, int srcX, int srcY,
                <a href='#SkImage_CachingHint'>CachingHint</a> cachingHint = <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>)const
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels from <a href='SkImage_Reference#Image'>Image</a> to <a href='#SkImage_readPixels_dstPixels'>dstPixels</a>. Copy starts at offset (<a href='#SkImage_readPixels_srcX'>srcX</a>, <a href='#SkImage_readPixels_srcY'>srcY</a>),
and does not exceed <a href='SkImage_Reference#Image'>Image</a> (<a href='#SkImage_width'>width()</a>, <a href='#SkImage_height'>height()</a>).

<a href='#SkImage_readPixels_dstInfo'>dstInfo</a> specifies width, height, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, and <a href='#Color_Space'>Color_Space</a> of
destination. <a href='#SkImage_readPixels_dstRowBytes'>dstRowBytes</a> specifics the gap from one destination row to the next.
Returns true if pixels are copied. Returns false if:

<table>  <tr>
    <td><a href='#SkImage_readPixels_dstInfo'>dstInfo</a> has no address</td>
  </tr>  <tr>
    <td><a href='#SkImage_readPixels_dstRowBytes'>dstRowBytes</a> is less than <a href='#SkImage_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>()</td>
  </tr>  <tr>
    <td><a href='#Pixel_Ref'>Pixel_Ref</a> is nullptr</td>
  </tr>
</table>

Pixels are copied only if <a href='undocumented#Pixel'>pixel</a> conversion is possible. If <a href='SkImage_Reference#Image'>Image</a> <a href='#Image_Info_Color_Type'>Color_Type</a> is
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, or <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkImage_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorType'>colorType</a>() must match.
If <a href='SkImage_Reference#Image'>Image</a> <a href='#Image_Info_Color_Type'>Color_Type</a> is <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkImage_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorSpace'>colorSpace</a>() must match.
If <a href='SkImage_Reference#Image'>Image</a> <a href='#Image_Info_Alpha_Type'>Alpha_Type</a> is <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkImage_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() must
match. If <a href='SkImage_Reference#Image'>Image</a> <a href='#Color_Space'>Color_Space</a> is nullptr, <a href='#SkImage_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorSpace'>colorSpace</a>() must match. Returns
false if <a href='undocumented#Pixel'>pixel</a> conversion is not possible.

<a href='#SkImage_readPixels_srcX'>srcX</a> and <a href='#SkImage_readPixels_srcY'>srcY</a> may be negative to copy only top or left of source. Returns
false if <a href='#SkImage_width'>width()</a> or <a href='#SkImage_height'>height()</a> is zero or negative.
Returns false if <code><a href='undocumented#abs()'>abs</a>(<a href='#SkImage_readPixels_srcX'>srcX</a>) >= <a href='SkImage_Reference#Image'>Image</a> <a href='#SkImage_width'>width()</a></code>, or if <code><a href='undocumented#abs()'>abs</a>(<a href='#SkImage_readPixels_srcY'>srcY</a>) >= <a href='SkImage_Reference#Image'>Image</a> <a href='#SkImage_height'>height()</a></code>.

If <a href='#SkImage_readPixels_cachingHint'>cachingHint</a> is <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>, pixels may be retained locally.
If <a href='#SkImage_readPixels_cachingHint'>cachingHint</a> is <a href='#SkImage_kDisallow_CachingHint'>kDisallow_CachingHint</a>, pixels are not added to the local cache.

### Parameters

<table>  <tr>    <td><a name='SkImage_readPixels_dstInfo'><code><strong>dstInfo</strong></code></a></td>
    <td>destination width, height, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, <a href='#Color_Space'>Color_Space</a></td>
  </tr>
  <tr>    <td><a name='SkImage_readPixels_dstPixels'><code><strong>dstPixels</strong></code></a></td>
    <td>destination  <a href='undocumented#Pixel_Storage'>pixel storage</a></td>
  </tr>
  <tr>    <td><a name='SkImage_readPixels_dstRowBytes'><code><strong>dstRowBytes</strong></code></a></td>
    <td>destination row length</td>
  </tr>
  <tr>    <td><a name='SkImage_readPixels_srcX'><code><strong>srcX</strong></code></a></td>
    <td>column index whose absolute value is less than <a href='#SkImage_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkImage_readPixels_srcY'><code><strong>srcY</strong></code></a></td>
    <td>row index whose absolute value is less than <a href='#SkImage_height'>height()</a></td>
  </tr>
  <tr>    <td><a name='SkImage_readPixels_cachingHint'><code><strong>cachingHint</strong></code></a></td>
    <td>one of: <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>, <a href='#SkImage_kDisallow_CachingHint'>kDisallow_CachingHint</a></td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href='#SkImage_readPixels_dstPixels'>dstPixels</a>

### Example

<div><fiddle-embed name="8aa8ca63dff4641dfc6ea8a3c555d59c"></fiddle-embed></div>

### See Also

<a href='#SkImage_scalePixels'>scalePixels</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_readPixels'>readPixels</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>::<a href='#SkPixmap_readPixels'>readPixels</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_readPixels'>readPixels</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_readPixels'>readPixels</a>

<a name='SkImage_readPixels_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_readPixels'>readPixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& dst, int srcX, int srcY, <a href='#SkImage_CachingHint'>CachingHint</a> cachingHint = <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>)const
</pre>

Copies a <a href='SkRect_Reference#Rect'>Rect</a> of pixels from <a href='SkImage_Reference#Image'>Image</a> to <a href='#SkImage_readPixels_2_dst'>dst</a>. Copy starts at (<a href='#SkImage_readPixels_2_srcX'>srcX</a>, <a href='#SkImage_readPixels_2_srcY'>srcY</a>), and
does not exceed <a href='SkImage_Reference#Image'>Image</a> (<a href='#SkImage_width'>width()</a>, <a href='#SkImage_height'>height()</a>).

<a href='#SkImage_readPixels_2_dst'>dst</a> specifies width, height, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, <a href='#Color_Space'>Color_Space</a>,  <a href='undocumented#Pixel_Storage'>pixel storage</a>,
and row bytes of destination. <a href='#SkImage_readPixels_2_dst'>dst</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() specifics the gap from one destination
row to the next. Returns true if pixels are copied. Returns false if:

<table>  <tr>
    <td><a href='#SkImage_readPixels_2_dst'>dst</a>  <a href='undocumented#Pixel_Storage'>pixel storage</a> equals nullptr</td>
  </tr>  <tr>
    <td><a href='#SkImage_readPixels_2_dst'>dst</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() is less than <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_minRowBytes'>minRowBytes</a></td>
  </tr>  <tr>
    <td><a href='#Pixel_Ref'>Pixel_Ref</a> is nullptr</td>
  </tr>
</table>

Pixels are copied only if <a href='undocumented#Pixel'>pixel</a> conversion is possible. If <a href='SkImage_Reference#Image'>Image</a> <a href='#Image_Info_Color_Type'>Color_Type</a> is
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, or <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkImage_readPixels_2_dst'>dst</a>.<a href='#SkPixmap_colorType'>colorType</a>() must match.
If <a href='SkImage_Reference#Image'>Image</a> <a href='#Image_Info_Color_Type'>Color_Type</a> is <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkImage_readPixels_2_dst'>dst</a>.<a href='#SkPixmap_colorSpace'>colorSpace</a>() must match.
If <a href='SkImage_Reference#Image'>Image</a> <a href='#Image_Info_Alpha_Type'>Alpha_Type</a> is <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkImage_readPixels_2_dst'>dst</a>.<a href='#SkPixmap_alphaType'>alphaType</a>() must
match. If <a href='SkImage_Reference#Image'>Image</a> <a href='#Color_Space'>Color_Space</a> is nullptr, <a href='#SkImage_readPixels_2_dst'>dst</a>.<a href='#SkPixmap_colorSpace'>colorSpace</a>() must match. Returns
false if <a href='undocumented#Pixel'>pixel</a> conversion is not possible.

<a href='#SkImage_readPixels_2_srcX'>srcX</a> and <a href='#SkImage_readPixels_2_srcY'>srcY</a> may be negative to copy only top or left of source. Returns
false if <a href='#SkImage_width'>width()</a> or <a href='#SkImage_height'>height()</a> is zero or negative.
Returns false if <code><a href='undocumented#abs()'>abs</a>(<a href='#SkImage_readPixels_2_srcX'>srcX</a>) >= <a href='SkImage_Reference#Image'>Image</a> <a href='#SkImage_width'>width()</a></code>, or if <code><a href='undocumented#abs()'>abs</a>(<a href='#SkImage_readPixels_2_srcY'>srcY</a>) >= <a href='SkImage_Reference#Image'>Image</a> <a href='#SkImage_height'>height()</a></code>.

If <a href='#SkImage_readPixels_2_cachingHint'>cachingHint</a> is <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>, pixels may be retained locally.
If <a href='#SkImage_readPixels_2_cachingHint'>cachingHint</a> is <a href='#SkImage_kDisallow_CachingHint'>kDisallow_CachingHint</a>, pixels are not added to the local cache.

### Parameters

<table>  <tr>    <td><a name='SkImage_readPixels_2_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkPixmap_Reference#Pixmap'>Pixmap</a>: <a href='#Image_Info'>Image_Info</a>, pixels, row bytes</td>
  </tr>
  <tr>    <td><a name='SkImage_readPixels_2_srcX'><code><strong>srcX</strong></code></a></td>
    <td>column index whose absolute value is less than <a href='#SkImage_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkImage_readPixels_2_srcY'><code><strong>srcY</strong></code></a></td>
    <td>row index whose absolute value is less than <a href='#SkImage_height'>height()</a></td>
  </tr>
  <tr>    <td><a name='SkImage_readPixels_2_cachingHint'><code><strong>cachingHint</strong></code></a></td>
    <td>one of: <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>, <a href='#SkImage_kDisallow_CachingHint'>kDisallow_CachingHint</a></td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href='#SkImage_readPixels_2_dst'>dst</a>

### Example

<div><fiddle-embed name="b77a73c4baa63a4a8e2a4fdd96144d0b"></fiddle-embed></div>

### See Also

<a href='#SkImage_scalePixels'>scalePixels</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_readPixels'>readPixels</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>::<a href='#SkPixmap_readPixels'>readPixels</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_readPixels'>readPixels</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_readPixels'>readPixels</a>

<a name='SkImage_scalePixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_scalePixels'>scalePixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& dst, <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> filterQuality,
                 <a href='#SkImage_CachingHint'>CachingHint</a> cachingHint = <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>)const
</pre>

Copies <a href='SkImage_Reference#SkImage'>SkImage</a> to <a href='#SkImage_scalePixels_dst'>dst</a>, scaling pixels to fit <a href='#SkImage_scalePixels_dst'>dst</a>.<a href='#SkPixmap_width'>width()</a> and <a href='#SkImage_scalePixels_dst'>dst</a>.<a href='#SkPixmap_height'>height()</a>, and
converting pixels to match <a href='#SkImage_scalePixels_dst'>dst</a>.<a href='#SkPixmap_colorType'>colorType</a>() and <a href='#SkImage_scalePixels_dst'>dst</a>.<a href='#SkPixmap_alphaType'>alphaType</a>(). Returns true if
pixels are copied. Returns false if <a href='#SkImage_scalePixels_dst'>dst</a>.<a href='#SkPixmap_addr'>addr()</a> is nullptr, or <a href='#SkImage_scalePixels_dst'>dst</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() is
less than <a href='#SkImage_scalePixels_dst'>dst</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>.

Pixels are copied only if <a href='undocumented#Pixel'>pixel</a> conversion is possible. If <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, or <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkImage_scalePixels_dst'>dst</a>.<a href='#SkPixmap_colorType'>colorType</a>() must match.
If <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkImage_scalePixels_dst'>dst</a>.<a href='#SkPixmap_colorSpace'>colorSpace</a>() must match.
If <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> is <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkImage_scalePixels_dst'>dst</a>.<a href='#SkPixmap_alphaType'>alphaType</a>() must
match. If <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> is nullptr, <a href='#SkImage_scalePixels_dst'>dst</a>.<a href='#SkPixmap_colorSpace'>colorSpace</a>() must match. Returns
false if <a href='undocumented#Pixel'>pixel</a> conversion is not possible.

Scales the <a href='SkImage_Reference#Image'>image</a>, with <a href='#SkImage_scalePixels_filterQuality'>filterQuality</a>, to match <a href='#SkImage_scalePixels_dst'>dst</a>.<a href='#SkPixmap_width'>width()</a> and <a href='#SkImage_scalePixels_dst'>dst</a>.<a href='#SkPixmap_height'>height()</a>.
<a href='#SkImage_scalePixels_filterQuality'>filterQuality</a> <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a> is fastest, typically implemented with
<a href='undocumented#Nearest_Neighbor'>nearest neighbor filter</a>. <a href='undocumented#kLow_SkFilterQuality'>kLow_SkFilterQuality</a> is typically implemented with
<a href='undocumented#Bilerp'>bilerp filter</a>. <a href='undocumented#kMedium_SkFilterQuality'>kMedium_SkFilterQuality</a> is typically implemented with
<a href='undocumented#Bilerp'>bilerp filter</a>, and  <a href='undocumented#MipMap'>mip-map filter</a> when <a href='undocumented#Size'>size</a> is reduced.
<a href='undocumented#kHigh_SkFilterQuality'>kHigh_SkFilterQuality</a> is slowest, typically implemented with  <a href='undocumented#BiCubic'>bicubic filter</a>.

If <a href='#SkImage_scalePixels_cachingHint'>cachingHint</a> is <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>, pixels may be retained locally.
If <a href='#SkImage_scalePixels_cachingHint'>cachingHint</a> is <a href='#SkImage_kDisallow_CachingHint'>kDisallow_CachingHint</a>, pixels are not added to the local cache.

### Parameters

<table>  <tr>    <td><a name='SkImage_scalePixels_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>: <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, pixels, row bytes</td>
  </tr>
  <tr>    <td><a name='SkImage_scalePixels_filterQuality'><code><strong>filterQuality</strong></code></a></td>
    <td>one of: <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a>, <a href='undocumented#kLow_SkFilterQuality'>kLow_SkFilterQuality</a>,</td>
  </tr>
</table>

<a href='undocumented#kMedium_SkFilterQuality'>kMedium_SkFilterQuality</a>, <a href='undocumented#kHigh_SkFilterQuality'>kHigh_SkFilterQuality</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_scalePixels_cachingHint'><code><strong>cachingHint</strong></code></a></td>
    <td>one of: <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>, <a href='#SkImage_kDisallow_CachingHint'>kDisallow_CachingHint</a></td>
  </tr>
</table>

### Return Value

true if pixels are scaled to fit <a href='#SkImage_scalePixels_dst'>dst</a>

### Example

<div><fiddle-embed name="5949c9a63610cae30019e5b1899ee38f"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawImage'>drawImage</a> <a href='#SkImage_readPixels'>readPixels</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>::<a href='#SkPixmap_scalePixels'>scalePixels</a>

<a name='SkImage_encodeToData'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkData'>SkData</a>&gt; <a href='#SkImage_encodeToData'>encodeToData</a>(<a href='undocumented#SkEncodedImageFormat'>SkEncodedImageFormat</a> encodedImageFormat, int quality)const
</pre>

Encodes <a href='SkImage_Reference#SkImage'>SkImage</a> pixels, returning result as <a href='undocumented#SkData'>SkData</a>.

Returns nullptr if encoding fails, or if <a href='#SkImage_encodeToData_encodedImageFormat'>encodedImageFormat</a> is not supported.

<a href='SkImage_Reference#SkImage'>SkImage</a> encoding in a format requires both building with one or more of:
SK_HAS_JPEG_LIBRARY, SK_HAS_PNG_LIBRARY, SK_HAS_WEBP_LIBRARY; and platform support
for the encoded format.

If SK_BUILD_FOR_MAC or SK_BUILD_FOR_IOS is defined, <a href='#SkImage_encodeToData_encodedImageFormat'>encodedImageFormat</a> can
additionally be one of: <a href='undocumented#SkEncodedImageFormat'>SkEncodedImageFormat</a>::<a href='#SkEncodedImageFormat_kICO'>kICO</a>, <a href='undocumented#SkEncodedImageFormat'>SkEncodedImageFormat</a>::<a href='#SkEncodedImageFormat_kBMP'>kBMP</a>,
<a href='undocumented#SkEncodedImageFormat'>SkEncodedImageFormat</a>::<a href='#SkEncodedImageFormat_kGIF'>kGIF</a>.

<a href='#SkImage_encodeToData_quality'>quality</a> is a platform and format specific metric trading off <a href='undocumented#Size'>size</a> and encoding
error. When used, <a href='#SkImage_encodeToData_quality'>quality</a> equaling 100 encodes with the least error. <a href='#SkImage_encodeToData_quality'>quality</a> may
be ignored by the encoder.

### Parameters

<table>  <tr>    <td><a name='SkImage_encodeToData_encodedImageFormat'><code><strong>encodedImageFormat</strong></code></a></td>
    <td>one of: <a href='undocumented#SkEncodedImageFormat'>SkEncodedImageFormat</a>::<a href='#SkEncodedImageFormat_kJPEG'>kJPEG</a>, <a href='undocumented#SkEncodedImageFormat'>SkEncodedImageFormat</a>::<a href='#SkEncodedImageFormat_kPNG'>kPNG</a>,</td>
  </tr>
</table>

<a href='undocumented#SkEncodedImageFormat'>SkEncodedImageFormat</a>::<a href='#SkEncodedImageFormat_kWEBP'>kWEBP</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_encodeToData_quality'><code><strong>quality</strong></code></a></td>
    <td>encoder specific metric with 100 equaling best</td>
  </tr>
</table>

### Return Value

encoded <a href='SkImage_Reference#SkImage'>SkImage</a>, or nullptr

### Example

<div><fiddle-embed name="7a3bf8851bb7160e4e49c48f8c09639d"></fiddle-embed></div>

### See Also

<a href='#SkImage_refEncodedData'>refEncodedData</a> <a href='#SkImage_MakeFromEncoded'>MakeFromEncoded</a>

<a name='SkImage_encodeToData_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkData'>SkData</a>&gt; <a href='#SkImage_encodeToData'>encodeToData</a>()const
</pre>

Encodes <a href='SkImage_Reference#SkImage'>SkImage</a> pixels, returning result as <a href='undocumented#SkData'>SkData</a>. Returns existing encoded <a href='undocumented#Data'>data</a>
if present; otherwise, <a href='SkImage_Reference#SkImage'>SkImage</a> is encoded with <a href='undocumented#SkEncodedImageFormat'>SkEncodedImageFormat</a>::<a href='#SkEncodedImageFormat_kPNG'>kPNG</a>. Skia
must be built with SK_HAS_PNG_LIBRARY to encode <a href='SkImage_Reference#SkImage'>SkImage</a>.

Returns nullptr if existing encoded <a href='undocumented#Data'>data</a> is missing or invalid, and
encoding fails.

### Return Value

encoded <a href='SkImage_Reference#SkImage'>SkImage</a>, or nullptr

### Example

<div><fiddle-embed name="30cee813f6aa476b0a9c8a24283e53a3"></fiddle-embed></div>

### See Also

<a href='#SkImage_refEncodedData'>refEncodedData</a> <a href='#SkImage_MakeFromEncoded'>MakeFromEncoded</a>

<a name='SkImage_refEncodedData'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkData'>SkData</a>&gt; <a href='#SkImage_refEncodedData'>refEncodedData</a>()const
</pre>

Returns encoded <a href='SkImage_Reference#SkImage'>SkImage</a> pixels as <a href='undocumented#SkData'>SkData</a>, if <a href='SkImage_Reference#SkImage'>SkImage</a> was created from supported
encoded <a href='SkStream_Reference#Stream'>stream</a> format. Platform support for formats vary and may require building
with one or more of: SK_HAS_JPEG_LIBRARY, SK_HAS_PNG_LIBRARY, SK_HAS_WEBP_LIBRARY.

Returns nullptr if <a href='SkImage_Reference#SkImage'>SkImage</a> contents are not encoded.

### Return Value

encoded <a href='SkImage_Reference#SkImage'>SkImage</a>, or nullptr

### Example

<div><fiddle-embed name="80856fe921ce36f8d5a32d8672bccbfc" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_encodeToData'>encodeToData</a> <a href='#SkImage_MakeFromEncoded'>MakeFromEncoded</a>

<a name='Utility'></a>

<a name='SkImage_makeSubset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_makeSubset'>makeSubset</a>(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& subset)const
</pre>

Returns <a href='#SkImage_makeSubset_subset'>subset</a> of <a href='SkImage_Reference#SkImage'>SkImage</a>. <a href='#SkImage_makeSubset_subset'>subset</a> must be fully contained by <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='#SkImage_dimensions'>dimensions()</a>.
The implementation may share pixels, or may copy them.

Returns nullptr if <a href='#SkImage_makeSubset_subset'>subset</a> is empty, or <a href='#SkImage_makeSubset_subset'>subset</a> is not contained by bounds, or
pixels in <a href='SkImage_Reference#SkImage'>SkImage</a> could not be read or copied.

### Parameters

<table>  <tr>    <td><a name='SkImage_makeSubset_subset'><code><strong>subset</strong></code></a></td>
    <td>bounds of returned <a href='SkImage_Reference#SkImage'>SkImage</a></td>
  </tr>
</table>

### Return Value

partial or full <a href='SkImage_Reference#SkImage'>SkImage</a>, or nullptr

### Example

<div><fiddle-embed name="889e495ce3e3b3bacc96e8230932331c"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromEncoded'>MakeFromEncoded</a>

<a name='SkImage_makeTextureImage'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_makeTextureImage'>makeTextureImage</a>(<a href='undocumented#GrContext'>GrContext</a>* context, <a href='undocumented#SkColorSpace'>SkColorSpace</a>* dstColorSpace,
                                <a href='undocumented#GrMipMapped'>GrMipMapped</a> mipMapped = <a href='undocumented#GrMipMapped'>GrMipMapped</a>::<a href='#GrMipMapped_kNo'>kNo</a>)const
</pre>

Returns <a href='SkImage_Reference#SkImage'>SkImage</a> backed by  <a href='undocumented#GPU_Texture'>GPU texture</a> associated with <a href='#SkImage_makeTextureImage_context'>context</a>. Returned <a href='SkImage_Reference#SkImage'>SkImage</a> is
compatible with <a href='SkSurface_Reference#SkSurface'>SkSurface</a> created with <a href='#SkImage_makeTextureImage_dstColorSpace'>dstColorSpace</a>. The returned <a href='SkImage_Reference#SkImage'>SkImage</a> respects
<a href='#SkImage_makeTextureImage_mipMapped'>mipMapped</a> setting; if <a href='#SkImage_makeTextureImage_mipMapped'>mipMapped</a> equals <a href='undocumented#GrMipMapped'>GrMipMapped</a>::<a href='#GrMipMapped_kYes'>kYes</a>, the backing <a href='undocumented#Texture'>texture</a>
allocates  <a href='undocumented#Mip_Map'>mip map</a> levels. Returns original <a href='SkImage_Reference#SkImage'>SkImage</a> if <a href='#SkImage_makeTextureImage_context'>context</a>
and <a href='#SkImage_makeTextureImage_dstColorSpace'>dstColorSpace</a> match and <a href='#SkImage_makeTextureImage_mipMapped'>mipMapped</a> is compatible with backing  <a href='undocumented#GPU_Texture'>GPU texture</a>.

Returns nullptr if <a href='#SkImage_makeTextureImage_context'>context</a> is nullptr, or if <a href='SkImage_Reference#SkImage'>SkImage</a> was created with another
<a href='undocumented#GrContext'>GrContext</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_makeTextureImage_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_makeTextureImage_dstColorSpace'><code><strong>dstColorSpace</strong></code></a></td>
    <td>range of colors of matching <a href='SkSurface_Reference#SkSurface'>SkSurface</a> on GPU</td>
  </tr>
  <tr>    <td><a name='SkImage_makeTextureImage_mipMapped'><code><strong>mipMapped</strong></code></a></td>
    <td>whether created <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='undocumented#Texture'>texture</a> must allocate  <a href='undocumented#Mip_Map'>mip map</a> levels</td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, or nullptr

### Example

<div><fiddle-embed name="eeec9e07e604b44d0208899a2fe5bef5" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromTexture'>MakeFromTexture</a>

<a name='SkImage_makeNonTextureImage'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_makeNonTextureImage'>makeNonTextureImage</a>()const
</pre>

Returns raster <a href='SkImage_Reference#Image'>image</a> or lazy <a href='SkImage_Reference#Image'>image</a>. Copies <a href='SkImage_Reference#SkImage'>SkImage</a> backed by GPU <a href='undocumented#Texture'>texture</a> into
CPU memory if needed. Returns original <a href='SkImage_Reference#SkImage'>SkImage</a> if decoded in raster <a href='SkBitmap_Reference#Bitmap'>bitmap</a>,
or if encoded in a <a href='SkStream_Reference#Stream'>stream</a>.

Returns nullptr if backed by GPU <a href='undocumented#Texture'>texture</a> and copy fails.

### Return Value

raster <a href='SkImage_Reference#Image'>image</a>, lazy <a href='SkImage_Reference#Image'>image</a>, or nullptr

### Example

<div><fiddle-embed name="ecdbaff44a02c310ef672b7d393c6dea" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_makeTextureImage'>makeTextureImage</a> <a href='#SkImage_makeRasterImage'>makeRasterImage</a> <a href='#SkImage_MakeBackendTextureFromSkImage'>MakeBackendTextureFromSkImage</a>

<a name='SkImage_makeRasterImage'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_makeRasterImage'>makeRasterImage</a>()const
</pre>

Returns raster <a href='SkImage_Reference#Image'>image</a>. Copies <a href='SkImage_Reference#SkImage'>SkImage</a> backed by GPU <a href='undocumented#Texture'>texture</a> into CPU memory,
or decodes <a href='SkImage_Reference#SkImage'>SkImage</a> from lazy <a href='SkImage_Reference#Image'>image</a>. Returns original <a href='SkImage_Reference#SkImage'>SkImage</a> if decoded in
raster <a href='SkBitmap_Reference#Bitmap'>bitmap</a>.

Returns nullptr if copy, decode, or <a href='undocumented#Pixel'>pixel</a> read fails.

### Return Value

raster <a href='SkImage_Reference#Image'>image</a>, or nullptr

### Example

<div><fiddle-embed name="aed5f399915d40bb5d133ab586e5bac3" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_isTextureBacked'>isTextureBacked</a> <a href='#SkImage_isLazyGenerated'>isLazyGenerated</a> <a href='#SkImage_MakeFromRaster'>MakeFromRaster</a>

<a name='SkImage_makeWithFilter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_makeWithFilter'>makeWithFilter</a>(const <a href='undocumented#SkImageFilter'>SkImageFilter</a>* filter, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& subset,
                              const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& clipBounds, <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* outSubset, <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>* offset)const
</pre>

Creates filtered <a href='SkImage_Reference#SkImage'>SkImage</a>. <a href='#SkImage_makeWithFilter_filter'>filter</a> processes original <a href='SkImage_Reference#SkImage'>SkImage</a>, potentially changing
<a href='SkColor_Reference#Color'>color</a>, position, and <a href='undocumented#Size'>size</a>. <a href='#SkImage_makeWithFilter_subset'>subset</a> is the bounds of original <a href='SkImage_Reference#SkImage'>SkImage</a> processed
by <a href='#SkImage_makeWithFilter_filter'>filter</a>. <a href='#SkImage_makeWithFilter_clipBounds'>clipBounds</a> is the expected bounds of the filtered <a href='SkImage_Reference#SkImage'>SkImage</a>. <a href='#SkImage_makeWithFilter_outSubset'>outSubset</a>
is required storage for the actual bounds of the filtered <a href='SkImage_Reference#SkImage'>SkImage</a>. <a href='#SkImage_makeWithFilter_offset'>offset</a> is
required storage for translation of returned <a href='SkImage_Reference#SkImage'>SkImage</a>.

Returns nullptr if <a href='SkImage_Reference#SkImage'>SkImage</a> could not be created. If nullptr is returned, <a href='#SkImage_makeWithFilter_outSubset'>outSubset</a>
and <a href='#SkImage_makeWithFilter_offset'>offset</a> are undefined.

Useful for animation of <a href='undocumented#SkImageFilter'>SkImageFilter</a> that varies <a href='undocumented#Size'>size</a> from frame to frame.
Returned <a href='SkImage_Reference#SkImage'>SkImage</a> is created larger than required by <a href='#SkImage_makeWithFilter_filter'>filter</a> so that  <a href='undocumented#GPU_Texture'>GPU texture</a>
can be reused with different sized effects. <a href='#SkImage_makeWithFilter_outSubset'>outSubset</a> describes the valid bounds
of  <a href='undocumented#GPU_Texture'>GPU texture</a> returned. <a href='#SkImage_makeWithFilter_offset'>offset</a> translates the returned <a href='SkImage_Reference#SkImage'>SkImage</a> to keep subsequent
animation frames aligned with respect to each other.

### Parameters

<table>  <tr>    <td><a name='SkImage_makeWithFilter_filter'><code><strong>filter</strong></code></a></td>
    <td>how <a href='SkImage_Reference#SkImage'>SkImage</a> is sampled when transformed</td>
  </tr>
  <tr>    <td><a name='SkImage_makeWithFilter_subset'><code><strong>subset</strong></code></a></td>
    <td>bounds of <a href='SkImage_Reference#SkImage'>SkImage</a> processed by <a href='#SkImage_makeWithFilter_filter'>filter</a></td>
  </tr>
  <tr>    <td><a name='SkImage_makeWithFilter_clipBounds'><code><strong>clipBounds</strong></code></a></td>
    <td>expected bounds of filtered <a href='SkImage_Reference#SkImage'>SkImage</a></td>
  </tr>
  <tr>    <td><a name='SkImage_makeWithFilter_outSubset'><code><strong>outSubset</strong></code></a></td>
    <td>storage for returned <a href='SkImage_Reference#SkImage'>SkImage</a> bounds</td>
  </tr>
  <tr>    <td><a name='SkImage_makeWithFilter_offset'><code><strong>offset</strong></code></a></td>
    <td>storage for returned <a href='SkImage_Reference#SkImage'>SkImage</a> translation</td>
  </tr>
</table>

### Return Value

filtered <a href='SkImage_Reference#SkImage'>SkImage</a>, or nullptr

### Example

<div><fiddle-embed name="85a76163138a2720ac003691d6363938" gpu="true"><div>In each frame of the animation, filtered <a href='SkImage_Reference#Image'>Image</a> is drawn in a different location.
By translating <a href='SkCanvas_Reference#Canvas'>canvas</a> by returned <a href='#SkImage_makeWithFilter_offset'>offset</a>, <a href='SkImage_Reference#Image'>Image</a> appears stationary.
</div></fiddle-embed></div>

### See Also

<a href='#SkImage_makeShader'>makeShader</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_setImageFilter'>setImageFilter</a>

<a name='SkImage_BackendTextureReleaseProc'></a>

---

<a href='#SkImage_BackendTextureReleaseProc'>BackendTextureReleaseProc</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    typedef std::function<void(<a href='undocumented#GrBackendTexture'>GrBackendTexture</a>)> <a href='#SkImage_BackendTextureReleaseProc'>BackendTextureReleaseProc</a>;
</pre>

Defines a callback function, taking one parameter of type <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> with
no return value. Function is called when back-end <a href='undocumented#Texture'>texture</a> is to be released.

<a name='SkImage_MakeBackendTextureFromSkImage'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkImage_MakeBackendTextureFromSkImage'>MakeBackendTextureFromSkImage</a>(<a href='undocumented#GrContext'>GrContext</a>* context, <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='SkImage_Reference#Image'>image</a>,
                                          <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>* backendTexture,
                                          <a href='#SkImage_BackendTextureReleaseProc'>BackendTextureReleaseProc</a>* backendTextureReleaseProc)
</pre>

Creates a <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> from the provided <a href='SkImage_Reference#SkImage'>SkImage</a>. Returns true and
stores result in <a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>backendTexture</a> and <a href='#SkImage_MakeBackendTextureFromSkImage_backendTextureReleaseProc'>backendTextureReleaseProc</a> if
<a href='undocumented#Texture'>texture</a> is created; otherwise, returns false and leaves
<a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>backendTexture</a> and <a href='#SkImage_MakeBackendTextureFromSkImage_backendTextureReleaseProc'>backendTextureReleaseProc</a> unmodified.

Call <a href='#SkImage_MakeBackendTextureFromSkImage_backendTextureReleaseProc'>backendTextureReleaseProc</a> after deleting <a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>backendTexture</a>.
<a href='#SkImage_MakeBackendTextureFromSkImage_backendTextureReleaseProc'>backendTextureReleaseProc</a> cleans up auxiliary <a href='undocumented#Data'>data</a> related to returned
<a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>backendTexture</a>. The caller must delete returned <a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>backendTexture</a> after use.

If <a href='SkImage_Reference#SkImage'>SkImage</a> is both <a href='undocumented#Texture'>texture</a> backed and singly referenced, <a href='#SkImage_MakeBackendTextureFromSkImage_image'>image</a> is returned in
<a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>backendTexture</a> without conversion or making a copy. <a href='SkImage_Reference#SkImage'>SkImage</a> is singly referenced
if its was transferred solely using std::move().

If <a href='SkImage_Reference#SkImage'>SkImage</a> is not <a href='undocumented#Texture'>texture</a> backed, returns <a href='undocumented#Texture'>texture</a> with <a href='SkImage_Reference#SkImage'>SkImage</a> contents.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeBackendTextureFromSkImage_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeBackendTextureFromSkImage_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> used for <a href='undocumented#Texture'>texture</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeBackendTextureFromSkImage_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td>storage for back-end <a href='undocumented#Texture'>texture</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeBackendTextureFromSkImage_backendTextureReleaseProc'><code><strong>backendTextureReleaseProc</strong></code></a></td>
    <td>storage for clean up function</td>
  </tr>
</table>

### Return Value

true if back-end <a href='undocumented#Texture'>texture</a> was created

### Example

<div><fiddle-embed name="06aeb3cf63ffccf7b49fe556e5def351" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromTexture'>MakeFromTexture</a> <a href='#SkImage_makeTextureImage'>makeTextureImage</a>

<a name='SkImage_isLazyGenerated'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_isLazyGenerated'>isLazyGenerated</a>()const
</pre>

Returns true if <a href='SkImage_Reference#SkImage'>SkImage</a> is backed by an image-generator or other service that creates
and caches its pixels or <a href='undocumented#Texture'>texture</a> on-demand.

### Return Value

true if <a href='SkImage_Reference#SkImage'>SkImage</a> is created as needed

### Example

<div><fiddle-embed name="a8b8bd4bfe968e2c63085f867665227f"></fiddle-embed></div>

### Example

<div><fiddle-embed name="f031c2a53f6a57833dc0127e674553da" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_isTextureBacked'>isTextureBacked</a> <a href='#SkImage_makeNonTextureImage'>makeNonTextureImage</a>

<a name='SkImage_makeColorSpace'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkImage_Reference#SkImage'>SkImage</a>&gt; <a href='#SkImage_makeColorSpace'>makeColorSpace</a>(<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; target)const
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> in <a href='#SkImage_makeColorSpace_target'>target</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>.
Returns nullptr if <a href='SkImage_Reference#SkImage'>SkImage</a> could not be created.

Returns original <a href='SkImage_Reference#SkImage'>SkImage</a> if it is in <a href='#SkImage_makeColorSpace_target'>target</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>.
Otherwise, converts pixels from <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> to <a href='#SkImage_makeColorSpace_target'>target</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>.
If <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='#SkImage_colorSpace'>colorSpace</a>() returns nullptr, <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> is assumed to be sRGB.

### Parameters

<table>  <tr>    <td><a name='SkImage_makeColorSpace_target'><code><strong>target</strong></code></a></td>
    <td><a href='undocumented#SkColorSpace'>SkColorSpace</a> describing <a href='SkColor_Reference#Color'>color</a> range of returned <a href='SkImage_Reference#SkImage'>SkImage</a></td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a> in <a href='#SkImage_makeColorSpace_target'>target</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>

### Example

<div><fiddle-embed name="dbf5f75c1275a3013672f896767140fb"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromPicture'>MakeFromPicture</a> <a href='#SkImage_MakeFromTexture'>MakeFromTexture</a>

