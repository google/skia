SkImage Reference
===

<a name='SkImage'></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='#SkImage'>SkImage</a> : public <a href='undocumented#SkRefCnt'>SkRefCnt</a> {
public:
    typedef void* <a href='#SkImage_ReleaseContext'>ReleaseContext</a>;

    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_MakeRasterCopy'>MakeRasterCopy</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& pixmap);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_MakeRasterData'>MakeRasterData</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, sk_sp<<a href='undocumented#SkData'>SkData</a>> pixels,
                                         size_t rowBytes);

    typedef void (*<a href='#SkImage_RasterReleaseProc'>RasterReleaseProc</a>)(const void* pixels, <a href='#SkImage_ReleaseContext'>ReleaseContext</a>);

    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromRaster'>MakeFromRaster</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& pixmap,
                                         <a href='#SkImage_RasterReleaseProc'>RasterReleaseProc</a> rasterReleaseProc,
                                         <a href='#SkImage_ReleaseContext'>ReleaseContext</a> releaseContext);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromBitmap'>MakeFromBitmap</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& bitmap);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromGenerator'>MakeFromGenerator</a>(std::unique_ptr<<a href='undocumented#SkImageGenerator'>SkImageGenerator</a>> imageGenerator,
                                            const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* subset = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromEncoded'>MakeFromEncoded</a>(sk_sp<<a href='undocumented#SkData'>SkData</a>> encoded, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* subset = nullptr);

    typedef void (*<a href='#SkImage_TextureReleaseProc'>TextureReleaseProc</a>)(<a href='#SkImage_ReleaseContext'>ReleaseContext</a> releaseContext);

    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromTexture'>MakeFromTexture</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                          const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture,
                                          <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> origin,
                                          <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> colorType,
                                          <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> alphaType,
                                          sk_sp<<a href='undocumented#SkColorSpace'>SkColorSpace</a>> colorSpace);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromTexture_2'>MakeFromTexture</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                          const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture,
                                          <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> origin,
                                          <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> colorType,
                                          <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> alphaType,
                                          sk_sp<<a href='undocumented#SkColorSpace'>SkColorSpace</a>> colorSpace,
                                          <a href='#SkImage_TextureReleaseProc'>TextureReleaseProc</a> textureReleaseProc,
                                          <a href='#SkImage_ReleaseContext'>ReleaseContext</a> releaseContext);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_MakeCrossContextFromEncoded'>MakeCrossContextFromEncoded</a>(<a href='undocumented#GrContext'>GrContext</a>* context, sk_sp<<a href='undocumented#SkData'>SkData</a>> data,
                                                      bool buildMips, <a href='undocumented#SkColorSpace'>SkColorSpace</a>* dstColorSpace,
                                                      bool limitToMaxTextureSize = false);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_MakeCrossContextFromPixmap'>MakeCrossContextFromPixmap</a>(<a href='undocumented#GrContext'>GrContext</a>* context, const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& pixmap,
                                                     bool buildMips, <a href='undocumented#SkColorSpace'>SkColorSpace</a>* dstColorSpace,
                                                     bool limitToMaxTextureSize = false);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromAdoptedTexture'>MakeFromAdoptedTexture</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                                 const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture,
                                                 <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> surfaceOrigin,
                                                 <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> colorType,
                                                 <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> alphaType = <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
                                                 sk_sp<<a href='undocumented#SkColorSpace'>SkColorSpace</a>> colorSpace = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromYUVATexturesCopy'>MakeFromYUVATexturesCopy</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                                   <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> yuvColorSpace,
                                                   const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> yuvaTextures[],
                                                   const <a href='undocumented#SkYUVAIndex'>SkYUVAIndex</a> yuvaIndices[4],
                                                   <a href='undocumented#SkISize'>SkISize</a> imageSize,
                                                   <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> imageOrigin,
                                                   sk_sp<<a href='undocumented#SkColorSpace'>SkColorSpace</a>> imageColorSpace = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend'>MakeFromYUVATexturesCopyWithExternalBackend</a>(
            <a href='undocumented#GrContext'>GrContext</a>* context,
            <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> yuvColorSpace,
            const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> yuvaTextures[],
            const <a href='undocumented#SkYUVAIndex'>SkYUVAIndex</a> yuvaIndices[4],
            <a href='undocumented#SkISize'>SkISize</a> imageSize,
            <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> imageOrigin,
            const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture,
            sk_sp<<a href='undocumented#SkColorSpace'>SkColorSpace</a>> imageColorSpace = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromYUVTexturesCopy'>MakeFromYUVTexturesCopy</a>(<a href='undocumented#GrContext'>GrContext</a>* context, <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> yuvColorSpace,
                                                  const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> yuvTextures[3],
                                                  <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> imageOrigin,
                                                  sk_sp<<a href='undocumented#SkColorSpace'>SkColorSpace</a>> imageColorSpace = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend'>MakeFromYUVTexturesCopyWithExternalBackend</a>(
            <a href='undocumented#GrContext'>GrContext</a>* context, <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> yuvColorSpace,
            const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> yuvTextures[3], <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> imageOrigin,
            const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture, sk_sp<<a href='undocumented#SkColorSpace'>SkColorSpace</a>> imageColorSpace = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromNV12TexturesCopy'>MakeFromNV12TexturesCopy</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                                   <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> yuvColorSpace,
                                                   const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> nv12Textures[2],
                                                   <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> imageOrigin,
                                                   sk_sp<<a href='undocumented#SkColorSpace'>SkColorSpace</a>> imageColorSpace = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend'>MakeFromNV12TexturesCopyWithExternalBackend</a>(
            <a href='undocumented#GrContext'>GrContext</a>* context,
            <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> yuvColorSpace,
            const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> nv12Textures[2],
            <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> imageOrigin,
            const <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& backendTexture,
            sk_sp<<a href='undocumented#SkColorSpace'>SkColorSpace</a>> imageColorSpace = nullptr);

    enum class <a href='#SkImage_BitDepth'>BitDepth</a> {
        <a href='#SkImage_BitDepth_kU8'>kU8</a>,
        <a href='#SkImage_BitDepth_kF16'>kF16</a>,
    };

    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromPicture'>MakeFromPicture</a>(sk_sp<<a href='SkPicture_Reference#SkPicture'>SkPicture</a>> picture, const <a href='undocumented#SkISize'>SkISize</a>& dimensions,
                                          const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* matrix, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* paint,
                                          <a href='#SkImage_BitDepth'>BitDepth</a> bitDepth,
                                          sk_sp<<a href='undocumented#SkColorSpace'>SkColorSpace</a>> colorSpace);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromAHardwareBuffer'>MakeFromAHardwareBuffer</a>(
            AHardwareBuffer* hardwareBuffer,
            <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> alphaType = <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
            sk_sp<<a href='undocumented#SkColorSpace'>SkColorSpace</a>> colorSpace = nullptr,
            <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> surfaceOrigin = <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a>);
    int <a href='#SkImage_width'>width</a>() const;
    int <a href='#SkImage_height'>height</a>() const;
    <a href='undocumented#SkISize'>SkISize</a> <a href='#SkImage_dimensions'>dimensions</a>() const;
    <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkImage_bounds'>bounds</a>() const;
    uint32_t <a href='#SkImage_uniqueID'>uniqueID</a>() const;
    <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImage_alphaType'>alphaType</a>() const;
    <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkImage_colorType'>colorType</a>() const;
    <a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='#SkImage_colorSpace'>colorSpace</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='#SkImage_refColorSpace'>refColorSpace</a>() const;
    bool <a href='#SkImage_isAlphaOnly'>isAlphaOnly</a>() const;
    bool <a href='#SkImage_isOpaque'>isOpaque</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkShader'>SkShader</a>> <a href='#SkImage_makeShader'>makeShader</a>(<a href='undocumented#SkShader_TileMode'>SkShader::TileMode</a> tileMode1, <a href='undocumented#SkShader_TileMode'>SkShader::TileMode</a> tileMode2,
                               const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* localMatrix = nullptr) const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkShader'>SkShader</a>> <a href='#SkImage_makeShader_2'>makeShader</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* localMatrix = nullptr) const;
    bool <a href='#SkImage_peekPixels'>peekPixels</a>(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>* pixmap) const;
    GrTexture* <a href='#SkImage_getTexture'>getTexture</a>() const;
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
    bool <a href='#SkImage_readPixels_2'>readPixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& dst, int srcX, int srcY,
                    <a href='#SkImage_CachingHint'>CachingHint</a> cachingHint = <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>) const;
    bool <a href='#SkImage_scalePixels'>scalePixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& dst, <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> filterQuality,
                     <a href='#SkImage_CachingHint'>CachingHint</a> cachingHint = <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>) const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkData'>SkData</a>> <a href='#SkImage_encodeToData'>encodeToData</a>(<a href='undocumented#SkEncodedImageFormat'>SkEncodedImageFormat</a> encodedImageFormat, int quality) const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkData'>SkData</a>> <a href='#SkImage_encodeToData_2'>encodeToData</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkData'>SkData</a>> <a href='#SkImage_refEncodedData'>refEncodedData</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_makeSubset'>makeSubset</a>(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& subset) const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_makeTextureImage'>makeTextureImage</a>(<a href='undocumented#GrContext'>GrContext</a>* context, <a href='undocumented#SkColorSpace'>SkColorSpace</a>* dstColorSpace,
                                    <a href='undocumented#GrMipMapped'>GrMipMapped</a> mipMapped = <a href='undocumented#GrMipMapped_kNo'>GrMipMapped::kNo</a>) const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_makeNonTextureImage'>makeNonTextureImage</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_makeRasterImage'>makeRasterImage</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_makeWithFilter'>makeWithFilter</a>(const <a href='undocumented#SkImageFilter'>SkImageFilter</a>* filter, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& subset,
                                  const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& clipBounds, <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* outSubset,
                                  <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>* offset) const;

    typedef std::function<void(GrBackendTexture)> <a href='#SkImage_BackendTextureReleaseProc'>BackendTextureReleaseProc</a>;

    static bool <a href='#SkImage_MakeBackendTextureFromSkImage'>MakeBackendTextureFromSkImage</a>(<a href='undocumented#GrContext'>GrContext</a>* context,
                                              sk_sp<<a href='#SkImage'>SkImage</a>> image,
                                              <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>* backendTexture,
                                              <a href='#SkImage_BackendTextureReleaseProc'>BackendTextureReleaseProc</a>* backendTextureReleaseProc);

    enum <a href='#SkImage_LegacyBitmapMode'>LegacyBitmapMode</a> {
        <a href='#SkImage_kRO_LegacyBitmapMode'>kRO_LegacyBitmapMode</a>,
    };

    bool <a href='#SkImage_asLegacyBitmap'>asLegacyBitmap</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* bitmap, <a href='#SkImage_LegacyBitmapMode'>LegacyBitmapMode</a> legacyBitmapMode = <a href='#SkImage_kRO_LegacyBitmapMode'>kRO_LegacyBitmapMode</a>) const;
    bool <a href='#SkImage_isLazyGenerated'>isLazyGenerated</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkImage'>SkImage</a>> <a href='#SkImage_makeColorSpace'>makeColorSpace</a>(sk_sp<<a href='undocumented#SkColorSpace'>SkColorSpace</a>> target) const;
};
</pre>

<a href='#Image'>Image</a> describes a two dimensional array of pixels to draw

## <a name='Raster_Image'>Raster Image</a>

<a href='#Raster_Image'>Raster Image</a> pixels are decoded in a <a href='undocumented#Raster_Bitmap'>Raster Bitmap</a>

## <a name='Texture_Image'>Texture Image</a>

<a href='#Texture_Image'>Texture Image</a> are located on GPU and pixels are not accessible

## <a name='Lazy_Image'>Lazy Image</a>

<a href='#Lazy_Image'>Lazy Image</a> defer allocating buffer for <a href='#Image'>Image</a> pixels and decoding stream until
<a href='#Image'>Image</a> is drawn

<a name='SkImage_MakeRasterCopy'></a>
## MakeRasterCopy

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates <a href='#Image'>Image</a> from <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> and copy of pixels

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeRasterCopy_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#Image_Info'>Image Info</a></td>
  </tr>
</table>

### Return Value

copy of <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> pixels

### Example

<div><fiddle-embed name="513afec5795a9504ebf6af5373d16b6b"><div>Draw a five by five bitmap</div></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeRasterData'>MakeRasterData</a> <a href='#SkImage_MakeFromGenerator'>MakeFromGenerator</a>

---

<a name='SkImage_MakeRasterData'></a>
## MakeRasterData

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates <a href='#Image'>Image</a> from <a href='SkImageInfo_Reference#Image_Info'>Image Info</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeRasterData_info'><code><strong>info</strong></code></a></td>
    <td>contains width</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeRasterData_pixels'><code><strong>pixels</strong></code></a></td>
    <td>address or pixel storage</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeRasterData_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>size of pixel row or larger</td>
  </tr>
</table>

### Return Value

<a href='#Image'>Image</a> sharing <a href='#SkImage_MakeRasterData_pixels'>pixels</a>

### Example

<div><fiddle-embed name="22e7ce79ab2fe94252d23319f2258127"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeRasterCopy'>MakeRasterCopy</a> <a href='#SkImage_MakeFromGenerator'>MakeFromGenerator</a>

---

## <a name='SkImage_ReleaseContext'>Typedef SkImage::ReleaseContext</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
typedef void* <a href='#SkImage_ReleaseContext'>ReleaseContext</a>;
</pre>

Caller data passed to <a href='#SkImage_RasterReleaseProc'>RasterReleaseProc</a>

### See Also

<a href='#SkImage_MakeFromRaster'>MakeFromRaster</a> <a href='#SkImage_RasterReleaseProc'>RasterReleaseProc</a>

## <a name='SkImage_RasterReleaseProc'>Typedef SkImage::RasterReleaseProc</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
typedef void (*<a href='#SkImage_RasterReleaseProc'>RasterReleaseProc</a>)(const void* pixels, <a href='#SkImage_ReleaseContext'>ReleaseContext</a>);
</pre>

Function called when <a href='#Image'>Image</a> no longer shares pixels

### See Also

<a href='#SkImage_ReleaseContext'>ReleaseContext</a> <a href='#SkImage_MakeFromRaster'>MakeFromRaster</a>

<a name='SkImage_MakeFromRaster'></a>
## MakeFromRaster

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates <a href='#Image'>Image</a> from <a href='#SkImage_MakeFromRaster_pixmap'>pixmap</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromRaster_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#Image_Info'>Image Info</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromRaster_rasterReleaseProc'><code><strong>rasterReleaseProc</strong></code></a></td>
    <td>function called when pixels can be released</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromRaster_releaseContext'><code><strong>releaseContext</strong></code></a></td>
    <td>state passed to <a href='#SkImage_MakeFromRaster_rasterReleaseProc'>rasterReleaseProc</a></td>
  </tr>
</table>

### Return Value

<a href='#Image'>Image</a> sharing <a href='#SkImage_MakeFromRaster_pixmap'>pixmap</a>

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

---

<a name='SkImage_MakeFromBitmap'></a>
## MakeFromBitmap

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates <a href='#Image'>Image</a> from <a href='#SkImage_MakeFromBitmap_bitmap'>bitmap</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromBitmap_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#Image_Info'>Image Info</a></td>
  </tr>
</table>

### Return Value

created <a href='#Image'>Image</a>

### Example

<div><fiddle-embed name="cf2cf53321e4e6a77c2841bfbc0ef707"><div>The first <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> is shared</div></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromRaster'>MakeFromRaster</a> <a href='#SkImage_MakeRasterCopy'>MakeRasterCopy</a> <a href='#SkImage_MakeFromGenerator'>MakeFromGenerator</a> <a href='#SkImage_MakeRasterData'>MakeRasterData</a>

---

<a name='SkImage_MakeFromGenerator'></a>
## MakeFromGenerator

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates <a href='#Image'>Image</a> from data returned by <a href='#SkImage_MakeFromGenerator_imageGenerator'>imageGenerator</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromGenerator_imageGenerator'><code><strong>imageGenerator</strong></code></a></td>
    <td>stock or custom routines to retrieve <a href='#Image'>Image</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromGenerator_subset'><code><strong>subset</strong></code></a></td>
    <td>bounds of returned <a href='#Image'>Image</a></td>
  </tr>
</table>

### Return Value

created <a href='#Image'>Image</a>

### Example

<div><fiddle-embed name="c2fec0746f88ca34d7dce59dd9bdef9e"><div>The generator returning <a href='SkPicture_Reference#Picture'>Picture</a> cannot be shared</div></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromEncoded'>MakeFromEncoded</a>

---

<a name='SkImage_MakeFromEncoded'></a>
## MakeFromEncoded

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates <a href='#Image'>Image</a> from <a href='#SkImage_MakeFromEncoded_encoded'>encoded</a> data

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromEncoded_encoded'><code><strong>encoded</strong></code></a></td>
    <td>data of <a href='#Image'>Image</a> to decode</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromEncoded_subset'><code><strong>subset</strong></code></a></td>
    <td>bounds of returned <a href='#Image'>Image</a></td>
  </tr>
</table>

### Return Value

created <a href='#Image'>Image</a>

### Example

<div><fiddle-embed name="894f732ed6409b1f392bc5481421d0e9"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromGenerator'>MakeFromGenerator</a>

---

## <a name='SkImage_TextureReleaseProc'>Typedef SkImage::TextureReleaseProc</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
typedef void (*<a href='#SkImage_TextureReleaseProc'>TextureReleaseProc</a>)(<a href='#SkImage_ReleaseContext'>ReleaseContext</a> releaseContext);
</pre>

User function called when supplied texture may be deleted

### See Also

<a href='#SkImage_MakeFromTexture'>MakeFromTexture</a><sup><a href='#SkImage_MakeFromTexture_2'>[2]</a></sup>

<a name='SkImage_MakeFromTexture'></a>
## MakeFromTexture

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates <a href='#Image'>Image</a> from <a href='undocumented#GPU_Texture'>GPU Texture</a> associated with <a href='#SkImage_MakeFromTexture_context'>context</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromTexture_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromTexture_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td>texture residing on GPU</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromTexture_origin'><code><strong>origin</strong></code></a></td>
    <td>one of</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromTexture_colorType'><code><strong>colorType</strong></code></a></td>
    <td>one of <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a> </td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromTexture_alphaType'><code><strong>alphaType</strong></code></a></td>
    <td>one of <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a> </td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromTexture_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td>range of colors</td>
  </tr>
</table>

### Return Value

created <a href='#Image'>Image</a>

### Example

<div><fiddle-embed name="94e9296c53bad074bf2a48ff885dac13" gpu="true"><div>A back</div></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromAdoptedTexture'>MakeFromAdoptedTexture</a> <a href='SkSurface_Reference#SkSurface_MakeFromBackendTexture'>SkSurface::MakeFromBackendTexture</a>

---

<a name='SkImage_MakeFromTexture_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates <a href='#Image'>Image</a> from <a href='undocumented#GPU_Texture'>GPU Texture</a> associated with <a href='#SkImage_MakeFromTexture_2_context'>context</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromTexture_2_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromTexture_2_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td>texture residing on GPU</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromTexture_2_origin'><code><strong>origin</strong></code></a></td>
    <td>one of</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromTexture_2_colorType'><code><strong>colorType</strong></code></a></td>
    <td>one of <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a> </td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromTexture_2_alphaType'><code><strong>alphaType</strong></code></a></td>
    <td>one of <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a> </td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromTexture_2_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td>range of colors</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromTexture_2_textureReleaseProc'><code><strong>textureReleaseProc</strong></code></a></td>
    <td>function called when texture can be released</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromTexture_2_releaseContext'><code><strong>releaseContext</strong></code></a></td>
    <td>state passed to <a href='#SkImage_MakeFromTexture_2_textureReleaseProc'>textureReleaseProc</a></td>
  </tr>
</table>

### Return Value

created <a href='#Image'>Image</a>

### Example

<div><fiddle-embed name="f40e1ebba6b067714062b81877b22fa1" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromAdoptedTexture'>MakeFromAdoptedTexture</a> <a href='SkSurface_Reference#SkSurface_MakeFromBackendTexture'>SkSurface::MakeFromBackendTexture</a>

---

<a name='SkImage_MakeCrossContextFromEncoded'></a>
## MakeCrossContextFromEncoded

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates <a href='#Image'>Image</a> from encoded <a href='#SkImage_MakeCrossContextFromEncoded_data'>data</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeCrossContextFromEncoded_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeCrossContextFromEncoded_data'><code><strong>data</strong></code></a></td>
    <td><a href='#Image'>Image</a> to decode</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeCrossContextFromEncoded_buildMips'><code><strong>buildMips</strong></code></a></td>
    <td>create <a href='#Image'>Image</a> as <a href='undocumented#Mip_Map'>Mip Map</a> if true</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeCrossContextFromEncoded_dstColorSpace'><code><strong>dstColorSpace</strong></code></a></td>
    <td>range of colors of matching <a href='SkSurface_Reference#Surface'>Surface</a> on GPU</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeCrossContextFromEncoded_limitToMaxTextureSize'><code><strong>limitToMaxTextureSize</strong></code></a></td>
    <td>downscale image to GPU maximum texture size</td>
  </tr>
</table>

### Return Value

created <a href='#Image'>Image</a>

### Example

<div><fiddle-embed name="069c7b116479e3ca46f953f07dcbdd36"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeCrossContextFromPixmap'>MakeCrossContextFromPixmap</a>

---

<a name='SkImage_MakeCrossContextFromPixmap'></a>
## MakeCrossContextFromPixmap

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates <a href='#Image'>Image</a> from <a href='#SkImage_MakeCrossContextFromPixmap_pixmap'>pixmap</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeCrossContextFromPixmap_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeCrossContextFromPixmap_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#Image_Info'>Image Info</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeCrossContextFromPixmap_buildMips'><code><strong>buildMips</strong></code></a></td>
    <td>create <a href='#Image'>Image</a> as <a href='undocumented#Mip_Map'>Mip Map</a> if true</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeCrossContextFromPixmap_dstColorSpace'><code><strong>dstColorSpace</strong></code></a></td>
    <td>range of colors of matching <a href='SkSurface_Reference#Surface'>Surface</a> on GPU</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeCrossContextFromPixmap_limitToMaxTextureSize'><code><strong>limitToMaxTextureSize</strong></code></a></td>
    <td>downscale image to GPU maximum texture size</td>
  </tr>
</table>

### Return Value

created <a href='#Image'>Image</a>

### Example

<div><fiddle-embed name="45bca8747b8f49b5be34b520897ef048"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeCrossContextFromEncoded'>MakeCrossContextFromEncoded</a>

---

<a name='SkImage_MakeFromAdoptedTexture'></a>
## MakeFromAdoptedTexture

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates <a href='#Image'>Image</a> from <a href='#SkImage_MakeFromAdoptedTexture_backendTexture'>backendTexture</a> associated with <a href='#SkImage_MakeFromAdoptedTexture_context'>context</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromAdoptedTexture_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromAdoptedTexture_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td>texture residing on GPU</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromAdoptedTexture_surfaceOrigin'><code><strong>surfaceOrigin</strong></code></a></td>
    <td>one of</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromAdoptedTexture_colorType'><code><strong>colorType</strong></code></a></td>
    <td>one of <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a> </td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromAdoptedTexture_alphaType'><code><strong>alphaType</strong></code></a></td>
    <td>one of <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a> </td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromAdoptedTexture_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td>range of colors</td>
  </tr>
</table>

### Return Value

created <a href='#Image'>Image</a>

### Example

<div><fiddle-embed name="b034517e39394b7543f06ec885e36d7d" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromTexture'>MakeFromTexture</a><sup><a href='#SkImage_MakeFromTexture_2'>[2]</a></sup> <a href='#SkImage_MakeFromYUVTexturesCopy'>MakeFromYUVTexturesCopy</a>

---

<a name='SkImage_MakeFromYUVATexturesCopy'></a>
## MakeFromYUVATexturesCopy

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates an <a href='#SkImage'>SkImage</a> by flattening the specified YUVA planes into a single

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopy_context'><code><strong>context</strong></code></a></td>
    <td>GPU <a href='#SkImage_MakeFromYUVATexturesCopy_context'>context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopy_yuvColorSpace'><code><strong>yuvColorSpace</strong></code></a></td>
    <td>one of</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopy_yuvaTextures'><code><strong>yuvaTextures</strong></code></a></td>
    <td>array of YUVA textures</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopy_yuvaIndices'><code><strong>yuvaIndices</strong></code></a></td>
    <td>array indicating <a href='#SkImage_MakeFromYUVATexturesCopy_yuvaTextures'>yuvaTextures</a> element and channel
that map to Y</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopy_imageSize'><code><strong>imageSize</strong></code></a></td>
    <td>size of the resulting image</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopy_imageOrigin'><code><strong>imageOrigin</strong></code></a></td>
    <td>one of</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopy_imageColorSpace'><code><strong>imageColorSpace</strong></code></a></td>
    <td>range of colors of the resulting image</td>
  </tr>
</table>

### Return Value

created <a href='#SkImage'>SkImage</a>

### See Also

<a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend'>MakeFromYUVATexturesCopyWithExternalBackend</a>

---

<a name='SkImage_MakeFromYUVATexturesCopyWithExternalBackend'></a>
## MakeFromYUVATexturesCopyWithExternalBackend

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates an <a href='#SkImage'>SkImage</a> by flattening the specified YUVA planes into a single

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopyWithExternalBackend_context'><code><strong>context</strong></code></a></td>
    <td>GPU <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend_context'>context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopyWithExternalBackend_yuvColorSpace'><code><strong>yuvColorSpace</strong></code></a></td>
    <td>one of</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopyWithExternalBackend_yuvaTextures'><code><strong>yuvaTextures</strong></code></a></td>
    <td>array of YUVA textures</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopyWithExternalBackend_yuvaIndices'><code><strong>yuvaIndices</strong></code></a></td>
    <td>array indicating <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend_yuvaTextures'>yuvaTextures</a> element and channel
that map to Y</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopyWithExternalBackend_imageSize'><code><strong>imageSize</strong></code></a></td>
    <td>size of the resulting image</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopyWithExternalBackend_imageOrigin'><code><strong>imageOrigin</strong></code></a></td>
    <td>one of</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopyWithExternalBackend_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td>resource that stores the final pixels</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopyWithExternalBackend_imageColorSpace'><code><strong>imageColorSpace</strong></code></a></td>
    <td>range of colors of the resulting image</td>
  </tr>
</table>

### Return Value

created <a href='#SkImage'>SkImage</a>

### See Also

<a href='#SkImage_MakeFromYUVATexturesCopy'>MakeFromYUVATexturesCopy</a>

---

<a name='SkImage_MakeFromYUVTexturesCopy'></a>
## MakeFromYUVTexturesCopy

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates <a href='#Image'>Image</a> from copy of <a href='#SkImage_MakeFromYUVTexturesCopy_yuvTextures'>yuvTextures</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVTexturesCopy_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVTexturesCopy_yuvColorSpace'><code><strong>yuvColorSpace</strong></code></a></td>
    <td>one of</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVTexturesCopy_yuvTextures'><code><strong>yuvTextures</strong></code></a></td>
    <td>array of YUV textures on GPU</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVTexturesCopy_imageOrigin'><code><strong>imageOrigin</strong></code></a></td>
    <td>one of</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVTexturesCopy_imageColorSpace'><code><strong>imageColorSpace</strong></code></a></td>
    <td>range of colors</td>
  </tr>
</table>

### Return Value

created <a href='#Image'>Image</a>

### See Also

<a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend'>MakeFromYUVTexturesCopyWithExternalBackend</a> <a href='#SkImage_MakeFromNV12TexturesCopy'>MakeFromNV12TexturesCopy</a> <a href='#SkImage_MakeFromYUVATexturesCopy'>MakeFromYUVATexturesCopy</a>

---

<a name='SkImage_MakeFromYUVTexturesCopyWithExternalBackend'></a>
## MakeFromYUVTexturesCopyWithExternalBackend

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates <a href='#Image'>Image</a> from copy of <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvTextures'>yuvTextures</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVTexturesCopyWithExternalBackend_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvColorSpace'><code><strong>yuvColorSpace</strong></code></a></td>
    <td>one of</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvTextures'><code><strong>yuvTextures</strong></code></a></td>
    <td>array of YUV textures on GPU</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVTexturesCopyWithExternalBackend_imageOrigin'><code><strong>imageOrigin</strong></code></a></td>
    <td>one of</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVTexturesCopyWithExternalBackend_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td>resource that stores final pixels</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVTexturesCopyWithExternalBackend_imageColorSpace'><code><strong>imageColorSpace</strong></code></a></td>
    <td>range of colors</td>
  </tr>
</table>

### Return Value

created <a href='#SkImage'>SkImage</a>

### See Also

<a href='#SkImage_MakeFromYUVTexturesCopy'>MakeFromYUVTexturesCopy</a> <a href='#SkImage_MakeFromNV12TexturesCopy'>MakeFromNV12TexturesCopy</a> <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend'>MakeFromYUVATexturesCopyWithExternalBackend</a>

---

<a name='SkImage_MakeFromNV12TexturesCopy'></a>
## MakeFromNV12TexturesCopy

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates <a href='#Image'>Image</a> from copy of <a href='#SkImage_MakeFromNV12TexturesCopy_nv12Textures'>nv12Textures</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromNV12TexturesCopy_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromNV12TexturesCopy_yuvColorSpace'><code><strong>yuvColorSpace</strong></code></a></td>
    <td>one of</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromNV12TexturesCopy_nv12Textures'><code><strong>nv12Textures</strong></code></a></td>
    <td>array of YUV textures on GPU</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromNV12TexturesCopy_imageOrigin'><code><strong>imageOrigin</strong></code></a></td>
    <td>one of</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromNV12TexturesCopy_imageColorSpace'><code><strong>imageColorSpace</strong></code></a></td>
    <td>range of colors</td>
  </tr>
</table>

### Return Value

created <a href='#Image'>Image</a>

### See Also

<a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend'>MakeFromNV12TexturesCopyWithExternalBackend</a> <a href='#SkImage_MakeFromYUVTexturesCopy'>MakeFromYUVTexturesCopy</a> <a href='#SkImage_MakeFromYUVATexturesCopy'>MakeFromYUVATexturesCopy</a>

---

<a name='SkImage_MakeFromNV12TexturesCopyWithExternalBackend'></a>
## MakeFromNV12TexturesCopyWithExternalBackend

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates <a href='#Image'>Image</a> from copy of <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>nv12Textures</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromNV12TexturesCopyWithExternalBackend_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromNV12TexturesCopyWithExternalBackend_yuvColorSpace'><code><strong>yuvColorSpace</strong></code></a></td>
    <td>one of</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'><code><strong>nv12Textures</strong></code></a></td>
    <td>array of YUV textures on GPU</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromNV12TexturesCopyWithExternalBackend_imageOrigin'><code><strong>imageOrigin</strong></code></a></td>
    <td>one of</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromNV12TexturesCopyWithExternalBackend_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td>resource that stores final pixels</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromNV12TexturesCopyWithExternalBackend_imageColorSpace'><code><strong>imageColorSpace</strong></code></a></td>
    <td>range of colors</td>
  </tr>
</table>

### Return Value

created <a href='#Image'>Image</a>

### See Also

<a href='#SkImage_MakeFromNV12TexturesCopy'>MakeFromNV12TexturesCopy</a> <a href='#SkImage_MakeFromYUVTexturesCopy'>MakeFromYUVTexturesCopy</a> <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend'>MakeFromYUVATexturesCopyWithExternalBackend</a>

---

## <a name='SkImage_BitDepth'>Enum SkImage::BitDepth</a>

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
Use 8 bits per ARGB component using unsigned integer format</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkImage_BitDepth_kF16'><code>SkImage::BitDepth::kF16</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Use 16 bits per ARGB component using half</td>
  </tr>
</table>

### See Also

<a href='#SkImage_MakeFromPicture'>MakeFromPicture</a>

<a name='SkImage_MakeFromPicture'></a>
## MakeFromPicture

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates <a href='#Image'>Image</a> from <a href='#SkImage_MakeFromPicture_picture'>picture</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromPicture_picture'><code><strong>picture</strong></code></a></td>
    <td>stream of drawing commands</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromPicture_dimensions'><code><strong>dimensions</strong></code></a></td>
    <td>width and height</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromPicture_matrix'><code><strong>matrix</strong></code></a></td>
    <td><a href='SkMatrix_Reference#Matrix'>Matrix</a> to rotate</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromPicture_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> to apply transparency</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromPicture_bitDepth'><code><strong>bitDepth</strong></code></a></td>
    <td>8</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromPicture_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td>range of colors</td>
  </tr>
</table>

### Return Value

created <a href='#Image'>Image</a>

### Example

<div><fiddle-embed name="4aa2879b9e44dfd6648995326d2c4dcf"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas_drawPicture'>SkCanvas::drawPicture</a><sup><a href='SkCanvas_Reference#SkCanvas_drawPicture_2'>[2]</a></sup><sup><a href='SkCanvas_Reference#SkCanvas_drawPicture_3'>[3]</a></sup><sup><a href='SkCanvas_Reference#SkCanvas_drawPicture_4'>[4]</a></sup>

---

<a name='SkImage_MakeFromAHardwareBuffer'></a>
## MakeFromAHardwareBuffer

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates <a href='#Image'>Image</a> from Android hardware buffer

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromAHardwareBuffer_hardwareBuffer'><code><strong>hardwareBuffer</strong></code></a></td>
    <td>AHardwareBuffer Android hardware buffer</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromAHardwareBuffer_alphaType'><code><strong>alphaType</strong></code></a></td>
    <td>one of <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a> </td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromAHardwareBuffer_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td>range of colors</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromAHardwareBuffer_surfaceOrigin'><code><strong>surfaceOrigin</strong></code></a></td>
    <td>one of</td>
  </tr>
</table>

### Return Value

created <a href='#Image'>Image</a>

### See Also

<a href='#SkImage_MakeFromRaster'>MakeFromRaster</a>

---

## <a name='Property'>Property</a>

<a name='SkImage_width'></a>
## width

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkImage_width'>width</a>(
</pre>

Returns pixel count in each row

### Return Value

pixel width in <a href='#Image'>Image</a>

### Example

<div><fiddle-embed name="39a6d0bbeac6d957c2338e0bff865cf8"></fiddle-embed></div>

### See Also

<a href='#SkImage_dimensions'>dimensions</a>(

---

<a name='SkImage_height'></a>
## height

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkImage_height'>height</a>(
</pre>

Returns pixel row count

### Return Value

pixel height in <a href='#Image'>Image</a>

### Example

<div><fiddle-embed name="6e563cb8351d34bd8af555a51bcd7a96"></fiddle-embed></div>

### See Also

<a href='#SkImage_dimensions'>dimensions</a>(

---

<a name='SkImage_dimensions'></a>
## dimensions

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkISize'>SkISize</a> <a href='#SkImage_dimensions'>dimensions</a>(
</pre>

Returns <a href='undocumented#ISize'>ISize</a>

### Return Value

integral size of <a href='#SkImage_width'>width</a>(

### Example

<div><fiddle-embed name="96b4bc43b3667df9ba9e2dafb770d33c">

#### Example Output

~~~~
dimensionsAsBounds == bounds
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkImage_height'>height</a>(

---

<a name='SkImage_bounds'></a>
## bounds

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkImage_bounds'>bounds</a>(
</pre>

Returns <a href='SkIRect_Reference#IRect'>IRect</a>

### Return Value

integral rectangle from origin to <a href='#SkImage_width'>width</a>(

### Example

<div><fiddle-embed name="c204b38b3fc08914b0a634aa4eaec894"></fiddle-embed></div>

### See Also

<a href='#SkImage_dimensions'>dimensions</a>(

---

<a name='SkImage_uniqueID'></a>
## uniqueID

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkImage_uniqueID'>uniqueID</a>(
</pre>

Returns value unique to image

### Return Value

unique identifier

### Example

<div><fiddle-embed name="d70194c9c51e700335f95de91846d023"></fiddle-embed></div>

### See Also

<a href='#SkImage_isLazyGenerated'>isLazyGenerated</a>

---

<a name='SkImage_alphaType'></a>
## alphaType

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImage_alphaType'>alphaType</a>(
</pre>

Returns <a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>

### Return Value

<a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a> in <a href='#Image'>Image</a>

### Example

<div><fiddle-embed name="1b9f1f05026ceb14ccb6926a13cdaa83"></fiddle-embed></div>

### See Also

<a href='SkImageInfo_Reference#SkImageInfo_alphaType'>SkImageInfo::alphaType</a>

---

<a name='SkImage_colorType'></a>
## colorType

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkImage_colorType'>colorType</a>(
</pre>

Returns <a href='SkImageInfo_Reference#Color_Type'>Color Type</a> if known

### Return Value

<a href='SkImageInfo_Reference#Color_Type'>Color Type</a> of <a href='#Image'>Image</a>

### Example

<div><fiddle-embed name="50396fad4a128f58e400ca00fe09711f"></fiddle-embed></div>

### See Also

<a href='SkImageInfo_Reference#SkImageInfo_colorType'>SkImageInfo::colorType</a>

---

<a name='SkImage_colorSpace'></a>
## colorSpace

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkColorSpace'>SkColorSpace</a>
</pre>

Returns <a href='undocumented#Color_Space'>Color Space</a>

### Return Value

<a href='undocumented#Color_Space'>Color Space</a> in <a href='#Image'>Image</a>

### Example

<div><fiddle-embed name="4468d573f42af6f5e234be10a5453bb2"></fiddle-embed></div>

### See Also

<a href='#SkImage_refColorSpace'>refColorSpace</a> <a href='#SkImage_makeColorSpace'>makeColorSpace</a>

---

<a name='SkImage_refColorSpace'></a>
## refColorSpace

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Returns a smart pointer to <a href='undocumented#Color_Space'>Color Space</a>

### Return Value

<a href='undocumented#Color_Space'>Color Space</a> in <a href='#Image'>Image</a>

### Example

<div><fiddle-embed name="59b2078ebfbda8736a57c0486ae33332"></fiddle-embed></div>

### See Also

<a href='#SkImage_colorSpace'>colorSpace</a> <a href='#SkImage_makeColorSpace'>makeColorSpace</a>

---

<a name='SkImage_isAlphaOnly'></a>
## isAlphaOnly

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_isAlphaOnly'>isAlphaOnly</a>(
</pre>

Returns true if <a href='#Image'>Image</a> pixels represent transparency only

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

---

<a name='SkImage_isOpaque'></a>
## isOpaque

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_isOpaque'>isOpaque</a>(
</pre>

Returns true if pixels ignore their <a href='SkColor_Reference#Alpha'>Alpha</a> value and are treated as fully opaque

### Return Value

true if <a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a> is <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>

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

---

<a name='SkImage_makeShader'></a>
## makeShader

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates <a href='undocumented#Shader'>Shader</a> from <a href='#Image'>Image</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_makeShader_tileMode1'><code><strong>tileMode1</strong></code></a></td>
    <td>tiling on x</td>
  </tr>
  <tr>    <td><a name='SkImage_makeShader_tileMode2'><code><strong>tileMode2</strong></code></a></td>
    <td>tiling on y</td>
  </tr>
  <tr>    <td><a name='SkImage_makeShader_localMatrix'><code><strong>localMatrix</strong></code></a></td>
    <td><a href='#Image'>Image</a> transformation</td>
  </tr>
</table>

### Return Value

<a href='undocumented#Shader'>Shader</a> containing <a href='#Image'>Image</a>

### Example

<div><fiddle-embed name="1c6de6fe72b00b5be970f5f718363449"></fiddle-embed></div>

### See Also

<a href='#SkImage_scalePixels'>scalePixels</a>

---

<a name='SkImage_makeShader_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates <a href='undocumented#Shader'>Shader</a> from <a href='#Image'>Image</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_makeShader_2_localMatrix'><code><strong>localMatrix</strong></code></a></td>
    <td><a href='#Image'>Image</a> transformation</td>
  </tr>
</table>

### Return Value

<a href='undocumented#Shader'>Shader</a> containing <a href='#Image'>Image</a>

### Example

<div><fiddle-embed name="10172fca71b9dbdcade772513ffeb27e"></fiddle-embed></div>

### See Also

<a href='#SkImage_scalePixels'>scalePixels</a>

---

## <a name='Pixels'>Pixels</a>

<a name='SkImage_peekPixels'></a>
## peekPixels

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_peekPixels'>peekPixels</a>(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>
</pre>

Copies <a href='#Image'>Image</a> pixel address

### Parameters

<table>  <tr>    <td><a name='SkImage_peekPixels_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td>storage for pixel state if pixels are readable</td>
  </tr>
</table>

### Return Value

true if <a href='#Image'>Image</a> has direct access to pixels

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

<a href='#SkImage_readPixels'>readPixels</a><sup><a href='#SkImage_readPixels_2'>[2]</a></sup>

---

<a name='SkImage_getTexture'></a>
## getTexture

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
GrTexture
</pre>

Deprecated.

---

<a name='SkImage_isTextureBacked'></a>
## isTextureBacked

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_isTextureBacked'>isTextureBacked</a>(
</pre>

Returns true the contents of <a href='#Image'>Image</a> was created on or uploaded to GPU memory

### Return Value

true if <a href='#Image'>Image</a> is a <a href='undocumented#GPU_Texture'>GPU Texture</a>

### Example

<div><fiddle-embed name="27a0ab44659201f1aa2ac7fea73368c2" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromTexture'>MakeFromTexture</a><sup><a href='#SkImage_MakeFromTexture_2'>[2]</a></sup> <a href='#SkImage_isValid'>isValid</a>

---

<a name='SkImage_isValid'></a>
## isValid

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_isValid'>isValid</a>(<a href='undocumented#GrContext'>GrContext</a>
</pre>

Returns true if <a href='#Image'>Image</a> can be drawn on either <a href='undocumented#Raster_Surface'>Raster Surface</a> or <a href='undocumented#GPU_Surface'>GPU Surface</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_isValid_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
</table>

### Return Value

true if <a href='#Image'>Image</a> can be drawn

### Example

<div><fiddle-embed name="8f7281446008cf4a9910fe73f44fa8d6" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_isTextureBacked'>isTextureBacked</a> <a href='#SkImage_isLazyGenerated'>isLazyGenerated</a>

---

<a name='SkImage_getBackendTexture'></a>
## getBackendTexture

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='#SkImage_getBackendTexture'>getBackendTexture</a>(bool flushPendingGrContextIO
</pre>

Retrieves the back

### Parameters

<table>  <tr>    <td><a name='SkImage_getBackendTexture_flushPendingGrContextIO'><code><strong>flushPendingGrContextIO</strong></code></a></td>
    <td>flag to flush outstanding requests</td>
  </tr>
  <tr>    <td><a name='SkImage_getBackendTexture_origin'><code><strong>origin</strong></code></a></td>
    <td>storage for one of</td>
  </tr>
</table>

### Return Value

back

### Example

<div><fiddle-embed name="d093aad721261f421c4bef4a296aab48" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromTexture'>MakeFromTexture</a><sup><a href='#SkImage_MakeFromTexture_2'>[2]</a></sup> <a href='#SkImage_isTextureBacked'>isTextureBacked</a>

---

## <a name='SkImage_CachingHint'>Enum SkImage::CachingHint</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkImage_CachingHint'>CachingHint</a> {
        <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>,
        <a href='#SkImage_kDisallow_CachingHint'>kDisallow_CachingHint</a>,
    };
</pre>

<a href='#SkImage_CachingHint'>CachingHint</a> selects whether Skia may internally cache <a href='SkBitmap_Reference#Bitmap'>Bitmaps</a> generated by
decoding <a href='#Image'>Image</a>

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

<a href='#SkImage_readPixels'>readPixels</a><sup><a href='#SkImage_readPixels_2'>[2]</a></sup> <a href='#SkImage_scalePixels'>scalePixels</a>

<a name='SkImage_readPixels'></a>
## readPixels

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_readPixels'>readPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> of pixels from <a href='#Image'>Image</a> to <a href='#SkImage_readPixels_dstPixels'>dstPixels</a>

<table>  <tr>
    <td><a href='#SkImage_readPixels_dstInfo'>dstInfo</a></td>
  </tr>  <tr>
    <td><a href='#SkImage_readPixels_dstRowBytes'>dstRowBytes</a> is less than <a href='#SkImage_readPixels_dstInfo'>dstInfo</a></td>
  </tr>  <tr>
    <td><a href='undocumented#Pixel_Ref'>Pixel Ref</a> is nullptr</td>
  </tr>
</table>

<a href='#Pixels'>Pixels</a> are copied only if pixel conversion is possible <code>abs(srcX)</code> <code>abs(srcY)</code>

### Parameters

<table>  <tr>    <td><a name='SkImage_readPixels_dstInfo'><code><strong>dstInfo</strong></code></a></td>
    <td>destination width</td>
  </tr>
  <tr>    <td><a name='SkImage_readPixels_dstPixels'><code><strong>dstPixels</strong></code></a></td>
    <td>destination pixel storage</td>
  </tr>
  <tr>    <td><a name='SkImage_readPixels_dstRowBytes'><code><strong>dstRowBytes</strong></code></a></td>
    <td>destination row length</td>
  </tr>
  <tr>    <td><a name='SkImage_readPixels_srcX'><code><strong>srcX</strong></code></a></td>
    <td>column index whose absolute value is less than <a href='#SkImage_width'>width</a>(</td>
  </tr>
  <tr>    <td><a name='SkImage_readPixels_srcY'><code><strong>srcY</strong></code></a></td>
    <td>row index whose absolute value is less than <a href='#SkImage_height'>height</a>(</td>
  </tr>
  <tr>    <td><a name='SkImage_readPixels_cachingHint'><code><strong>cachingHint</strong></code></a></td>
    <td>one of</td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href='#SkImage_readPixels_dstPixels'>dstPixels</a>

### Example

<div><fiddle-embed name="8aa8ca63dff4641dfc6ea8a3c555d59c"></fiddle-embed></div>

### See Also

<a href='#SkImage_scalePixels'>scalePixels</a> <a href='SkBitmap_Reference#SkBitmap_readPixels'>SkBitmap::readPixels</a><sup><a href='SkBitmap_Reference#SkBitmap_readPixels_2'>[2]</a></sup><sup><a href='SkBitmap_Reference#SkBitmap_readPixels_3'>[3]</a></sup> <a href='SkPixmap_Reference#SkPixmap_readPixels'>SkPixmap::readPixels</a><sup><a href='SkPixmap_Reference#SkPixmap_readPixels_2'>[2]</a></sup><sup><a href='SkPixmap_Reference#SkPixmap_readPixels_3'>[3]</a></sup><sup><a href='SkPixmap_Reference#SkPixmap_readPixels_4'>[4]</a></sup> <a href='SkCanvas_Reference#SkCanvas_readPixels'>SkCanvas::readPixels</a><sup><a href='SkCanvas_Reference#SkCanvas_readPixels_2'>[2]</a></sup><sup><a href='SkCanvas_Reference#SkCanvas_readPixels_3'>[3]</a></sup> <a href='SkSurface_Reference#SkSurface_readPixels'>SkSurface::readPixels</a><sup><a href='SkSurface_Reference#SkSurface_readPixels_2'>[2]</a></sup><sup><a href='SkSurface_Reference#SkSurface_readPixels_3'>[3]</a></sup>

---

<a name='SkImage_readPixels_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_readPixels'>readPixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>
</pre>

Copies a <a href='SkRect_Reference#Rect'>Rect</a> of pixels from <a href='#Image'>Image</a> to <a href='#SkImage_readPixels_2_dst'>dst</a>

<table>  <tr>
    <td><a href='#SkImage_readPixels_2_dst'>dst</a> pixel storage equals nullptr</td>
  </tr>  <tr>
    <td><a href='#SkImage_readPixels_2_dst'>dst</a></td>
  </tr>  <tr>
    <td><a href='undocumented#Pixel_Ref'>Pixel Ref</a> is nullptr</td>
  </tr>
</table>

<a href='#Pixels'>Pixels</a> are copied only if pixel conversion is possible <code>abs(srcX)</code> <code>abs(srcY)</code>

### Parameters

<table>  <tr>    <td><a name='SkImage_readPixels_2_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkPixmap_Reference#Pixmap'>Pixmap</a></td>
  </tr>
  <tr>    <td><a name='SkImage_readPixels_2_srcX'><code><strong>srcX</strong></code></a></td>
    <td>column index whose absolute value is less than <a href='#SkImage_width'>width</a>(</td>
  </tr>
  <tr>    <td><a name='SkImage_readPixels_2_srcY'><code><strong>srcY</strong></code></a></td>
    <td>row index whose absolute value is less than <a href='#SkImage_height'>height</a>(</td>
  </tr>
  <tr>    <td><a name='SkImage_readPixels_2_cachingHint'><code><strong>cachingHint</strong></code></a></td>
    <td>one of</td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href='#SkImage_readPixels_2_dst'>dst</a>

### Example

<div><fiddle-embed name="b77a73c4baa63a4a8e2a4fdd96144d0b"></fiddle-embed></div>

### See Also

<a href='#SkImage_scalePixels'>scalePixels</a> <a href='SkBitmap_Reference#SkBitmap_readPixels'>SkBitmap::readPixels</a><sup><a href='SkBitmap_Reference#SkBitmap_readPixels_2'>[2]</a></sup><sup><a href='SkBitmap_Reference#SkBitmap_readPixels_3'>[3]</a></sup> <a href='SkPixmap_Reference#SkPixmap_readPixels'>SkPixmap::readPixels</a><sup><a href='SkPixmap_Reference#SkPixmap_readPixels_2'>[2]</a></sup><sup><a href='SkPixmap_Reference#SkPixmap_readPixels_3'>[3]</a></sup><sup><a href='SkPixmap_Reference#SkPixmap_readPixels_4'>[4]</a></sup> <a href='SkCanvas_Reference#SkCanvas_readPixels'>SkCanvas::readPixels</a><sup><a href='SkCanvas_Reference#SkCanvas_readPixels_2'>[2]</a></sup><sup><a href='SkCanvas_Reference#SkCanvas_readPixels_3'>[3]</a></sup> <a href='SkSurface_Reference#SkSurface_readPixels'>SkSurface::readPixels</a><sup><a href='SkSurface_Reference#SkSurface_readPixels_2'>[2]</a></sup><sup><a href='SkSurface_Reference#SkSurface_readPixels_3'>[3]</a></sup>

---

<a name='SkImage_scalePixels'></a>
## scalePixels

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_scalePixels'>scalePixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>
</pre>

Copies <a href='#Image'>Image</a> to <a href='#SkImage_scalePixels_dst'>dst</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_scalePixels_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkPixmap_Reference#Pixmap'>Pixmap</a></td>
  </tr>
  <tr>    <td><a name='SkImage_scalePixels_filterQuality'><code><strong>filterQuality</strong></code></a></td>
    <td>one of</td>
  </tr>
  <tr>    <td><a name='SkImage_scalePixels_cachingHint'><code><strong>cachingHint</strong></code></a></td>
    <td>one of</td>
  </tr>
</table>

### Return Value

true if pixels are scaled to fit <a href='#SkImage_scalePixels_dst'>dst</a>

### Example

<div><fiddle-embed name="5949c9a63610cae30019e5b1899ee38f"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas_drawImage'>SkCanvas::drawImage</a><sup><a href='SkCanvas_Reference#SkCanvas_drawImage_2'>[2]</a></sup> <a href='#SkImage_readPixels'>readPixels</a><sup><a href='#SkImage_readPixels_2'>[2]</a></sup> <a href='SkPixmap_Reference#SkPixmap_scalePixels'>SkPixmap::scalePixels</a>

---

<a name='SkImage_encodeToData'></a>
## encodeToData

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Encodes <a href='#Image'>Image</a> pixels

### Parameters

<table>  <tr>    <td><a name='SkImage_encodeToData_encodedImageFormat'><code><strong>encodedImageFormat</strong></code></a></td>
    <td>one of</td>
  </tr>
  <tr>    <td><a name='SkImage_encodeToData_quality'><code><strong>quality</strong></code></a></td>
    <td>encoder specific metric with 100 equaling best</td>
  </tr>
</table>

### Return Value

encoded <a href='#Image'>Image</a>

### Example

<div><fiddle-embed name="7a3bf8851bb7160e4e49c48f8c09639d"></fiddle-embed></div>

### See Also

<a href='#SkImage_refEncodedData'>refEncodedData</a> <a href='#SkImage_MakeFromEncoded'>MakeFromEncoded</a>

---

<a name='SkImage_encodeToData_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Encodes <a href='#Image'>Image</a> pixels

### Return Value

encoded <a href='#Image'>Image</a>

### Example

<div><fiddle-embed name="30cee813f6aa476b0a9c8a24283e53a3"></fiddle-embed></div>

### See Also

<a href='#SkImage_refEncodedData'>refEncodedData</a> <a href='#SkImage_MakeFromEncoded'>MakeFromEncoded</a>

---

<a name='SkImage_refEncodedData'></a>
## refEncodedData

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Returns encoded <a href='#Image'>Image</a> pixels as <a href='undocumented#SkData'>SkData</a>

### Return Value

encoded <a href='#Image'>Image</a>

### Example

<div><fiddle-embed name="80856fe921ce36f8d5a32d8672bccbfc" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_encodeToData'>encodeToData</a><sup><a href='#SkImage_encodeToData_2'>[2]</a></sup> <a href='#SkImage_MakeFromEncoded'>MakeFromEncoded</a>

---

## <a name='Utility'>Utility</a>

<a name='SkImage_makeSubset'></a>
## makeSubset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Returns <a href='#SkImage_makeSubset_subset'>subset</a> of <a href='#Image'>Image</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_makeSubset_subset'><code><strong>subset</strong></code></a></td>
    <td>bounds of returned <a href='#Image'>Image</a></td>
  </tr>
</table>

### Return Value

partial or full <a href='#Image'>Image</a>

### Example

<div><fiddle-embed name="889e495ce3e3b3bacc96e8230932331c"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromEncoded'>MakeFromEncoded</a>

---

<a name='SkImage_makeTextureImage'></a>
## makeTextureImage

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Returns <a href='#Image'>Image</a> backed by <a href='undocumented#GPU_Texture'>GPU Texture</a> associated with <a href='#SkImage_makeTextureImage_context'>context</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_makeTextureImage_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_makeTextureImage_dstColorSpace'><code><strong>dstColorSpace</strong></code></a></td>
    <td>range of colors of matching <a href='SkSurface_Reference#Surface'>Surface</a> on GPU</td>
  </tr>
  <tr>    <td><a name='SkImage_makeTextureImage_mipMapped'><code><strong>mipMapped</strong></code></a></td>
    <td>whether created <a href='#Image'>Image</a> texture must allocate <a href='undocumented#Mip_Map'>Mip Map</a> levels</td>
  </tr>
</table>

### Return Value

created <a href='#Image'>Image</a>

### Example

<div><fiddle-embed name="b14d9debfe87295373b44a179992a999" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromTexture'>MakeFromTexture</a><sup><a href='#SkImage_MakeFromTexture_2'>[2]</a></sup>

---

<a name='SkImage_makeNonTextureImage'></a>
## makeNonTextureImage

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Returns <a href='#Raster_Image'>Raster Image</a> or <a href='#Lazy_Image'>Lazy Image</a>

### Return Value

<a href='#Raster_Image'>Raster Image</a>

### Example

<div><fiddle-embed name="c77bfb00fb82e378eea4b7f7c18a8b84" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_makeTextureImage'>makeTextureImage</a> <a href='#SkImage_makeRasterImage'>makeRasterImage</a> <a href='#SkImage_MakeBackendTextureFromSkImage'>MakeBackendTextureFromSkImage</a>

---

<a name='SkImage_makeRasterImage'></a>
## makeRasterImage

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Returns <a href='#Raster_Image'>Raster Image</a>

### Return Value

<a href='#Raster_Image'>Raster Image</a>

### Example

<div><fiddle-embed name="505a6d9458394b1deb5d2f6c44e1cd76" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_isTextureBacked'>isTextureBacked</a> <a href='#SkImage_isLazyGenerated'>isLazyGenerated</a> <a href='#SkImage_MakeFromRaster'>MakeFromRaster</a>

---

<a name='SkImage_makeWithFilter'></a>
## makeWithFilter

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates filtered <a href='#Image'>Image</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_makeWithFilter_filter'><code><strong>filter</strong></code></a></td>
    <td>how <a href='#Image'>Image</a> is sampled when transformed</td>
  </tr>
  <tr>    <td><a name='SkImage_makeWithFilter_subset'><code><strong>subset</strong></code></a></td>
    <td>bounds of <a href='#Image'>Image</a> processed by <a href='#SkImage_makeWithFilter_filter'>filter</a></td>
  </tr>
  <tr>    <td><a name='SkImage_makeWithFilter_clipBounds'><code><strong>clipBounds</strong></code></a></td>
    <td>expected bounds of filtered <a href='#Image'>Image</a></td>
  </tr>
  <tr>    <td><a name='SkImage_makeWithFilter_outSubset'><code><strong>outSubset</strong></code></a></td>
    <td>storage for returned <a href='#Image'>Image</a> bounds</td>
  </tr>
  <tr>    <td><a name='SkImage_makeWithFilter_offset'><code><strong>offset</strong></code></a></td>
    <td>storage for returned <a href='#Image'>Image</a> translation</td>
  </tr>
</table>

### Return Value

filtered <a href='#Image'>Image</a>

### Example

<div><fiddle-embed name="85a76163138a2720ac003691d6363938" gpu="true"><div>In each frame of the animation</div></fiddle-embed></div>

### See Also

<a href='#SkImage_makeShader'>makeShader</a><sup><a href='#SkImage_makeShader_2'>[2]</a></sup> <a href='SkPaint_Reference#SkPaint_setImageFilter'>SkPaint::setImageFilter</a>

---

## <a name='SkImage_BackendTextureReleaseProc'>Typedef SkImage::BackendTextureReleaseProc</a>
<a href='#SkImage_BackendTextureReleaseProc'>BackendTextureReleaseProc</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
typedef std::function<void(GrBackendTexture)> <a href='#SkImage_BackendTextureReleaseProc'>BackendTextureReleaseProc</a>;
</pre>

Defines a callback function

<a name='SkImage_MakeBackendTextureFromSkImage'></a>
## MakeBackendTextureFromSkImage

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkImage_MakeBackendTextureFromSkImage'>MakeBackendTextureFromSkImage</a>(<a href='undocumented#GrContext'>GrContext</a>
</pre>

Creates a <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> from the provided <a href='#SkImage'>SkImage</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeBackendTextureFromSkImage_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU Context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeBackendTextureFromSkImage_image'><code><strong>image</strong></code></a></td>
    <td><a href='#Image'>Image</a> used for texture</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeBackendTextureFromSkImage_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td>storage for back</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeBackendTextureFromSkImage_backendTextureReleaseProc'><code><strong>backendTextureReleaseProc</strong></code></a></td>
    <td>storage for clean up function</td>
  </tr>
</table>

### Return Value

true if back

### Example

<div><fiddle-embed name="06aeb3cf63ffccf7b49fe556e5def351" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromTexture'>MakeFromTexture</a><sup><a href='#SkImage_MakeFromTexture_2'>[2]</a></sup> <a href='#SkImage_makeTextureImage'>makeTextureImage</a>

---

## <a name='SkImage_LegacyBitmapMode'>Enum SkImage::LegacyBitmapMode</a>

To be deprecated soon.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkImage_LegacyBitmapMode'>LegacyBitmapMode</a> {
        <a href='#SkImage_kRO_LegacyBitmapMode'>kRO_LegacyBitmapMode</a>,
    };
</pre>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkImage_kRO_LegacyBitmapMode'><code>SkImage::kRO_LegacyBitmapMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
returned bitmap is read-only and immutable</td>
  </tr>
</table>

<a name='SkImage_asLegacyBitmap'></a>
## asLegacyBitmap

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_asLegacyBitmap'>asLegacyBitmap</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>
</pre>

Creates raster <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> with same pixels as <a href='#Image'>Image</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_asLegacyBitmap_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td>storage for legacy <a href='SkBitmap_Reference#Bitmap'>Bitmap</a></td>
  </tr>
  <tr>    <td><a name='SkImage_asLegacyBitmap_legacyBitmapMode'><code><strong>legacyBitmapMode</strong></code></a></td>
    <td>to be deprecated</td>
  </tr>
</table>

### Return Value

true if <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> was created

### Example

<div><fiddle-embed name="78374702fa113076ddc6070053ab5cd4" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeRasterData'>MakeRasterData</a> <a href='#SkImage_makeRasterImage'>makeRasterImage</a> <a href='#SkImage_makeNonTextureImage'>makeNonTextureImage</a>

---

<a name='SkImage_isLazyGenerated'></a>
## isLazyGenerated

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_isLazyGenerated'>isLazyGenerated</a>(
</pre>

Returns true if <a href='#Image'>Image</a> is backed by an image

### Return Value

true if <a href='#Image'>Image</a> is created as needed

### Example

<div><fiddle-embed name="a8b8bd4bfe968e2c63085f867665227f"></fiddle-embed></div>

### Example

<div><fiddle-embed name="25305461b916baf40d7d379e04a5589c" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_isTextureBacked'>isTextureBacked</a> MakeNonTextureImage

---

<a name='SkImage_makeColorSpace'></a>
## makeColorSpace

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates <a href='#Image'>Image</a> in <a href='#SkImage_makeColorSpace_target'>target</a> <a href='undocumented#Color_Space'>Color Space</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_makeColorSpace_target'><code><strong>target</strong></code></a></td>
    <td><a href='undocumented#Color_Space'>Color Space</a> describing color range of returned <a href='#Image'>Image</a></td>
  </tr>
</table>

### Return Value

created <a href='#Image'>Image</a> in <a href='#SkImage_makeColorSpace_target'>target</a> <a href='undocumented#Color_Space'>Color Space</a>

### Example

<div><fiddle-embed name="dbf5f75c1275a3013672f896767140fb"></fiddle-embed></div>

### See Also

MakeFromPixture <a href='#SkImage_MakeFromTexture'>MakeFromTexture</a><sup><a href='#SkImage_MakeFromTexture_2'>[2]</a></sup>

---

