SkImage Reference
===


<a name='SkImage'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkImage_Reference#SkImage'>SkImage</a> : <a href='SkImage_Reference#SkImage'>public</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> {
<a href='undocumented#SkRefCnt'>public</a>:
    <a href='undocumented#SkRefCnt'>typedef</a> <a href='undocumented#SkRefCnt'>void</a>* <a href='#SkImage_ReleaseContext'>ReleaseContext</a>;

    <a href='#SkImage_ReleaseContext'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeRasterCopy'>MakeRasterCopy</a>(<a href='#SkImage_MakeRasterCopy'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#Pixmap'>pixmap</a>);
    <a href='SkPixmap_Reference#Pixmap'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeRasterData'>MakeRasterData</a>(<a href='#SkImage_MakeRasterData'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkData'>SkData</a>> <a href='undocumented#SkData'>pixels</a>,
                                         <a href='undocumented#SkData'>size_t</a> <a href='undocumented#SkData'>rowBytes</a>);

    <a href='undocumented#SkData'>typedef</a> <a href='undocumented#SkData'>void</a> (*<a href='#SkImage_RasterReleaseProc'>RasterReleaseProc</a>)(<a href='#SkImage_RasterReleaseProc'>const</a> <a href='#SkImage_RasterReleaseProc'>void</a>* <a href='#SkImage_RasterReleaseProc'>pixels</a>, <a href='#SkImage_ReleaseContext'>ReleaseContext</a>);

    <a href='#SkImage_ReleaseContext'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromRaster'>MakeFromRaster</a>(<a href='#SkImage_MakeFromRaster'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#Pixmap'>pixmap</a>,
                                         <a href='#SkImage_RasterReleaseProc'>RasterReleaseProc</a> <a href='#SkImage_RasterReleaseProc'>rasterReleaseProc</a>,
                                         <a href='#SkImage_ReleaseContext'>ReleaseContext</a> <a href='#SkImage_ReleaseContext'>releaseContext</a>);
    <a href='#SkImage_ReleaseContext'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromBitmap'>MakeFromBitmap</a>(<a href='#SkImage_MakeFromBitmap'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>);
    <a href='SkBitmap_Reference#Bitmap'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromGenerator'>MakeFromGenerator</a>(<a href='#SkImage_MakeFromGenerator'>std</a>::<a href='#SkImage_MakeFromGenerator'>unique_ptr</a><<a href='undocumented#SkImageGenerator'>SkImageGenerator</a>> <a href='undocumented#SkImageGenerator'>imageGenerator</a>,
                                            <a href='undocumented#SkImageGenerator'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* <a href='SkIRect_Reference#SkIRect'>subset</a> = <a href='SkIRect_Reference#SkIRect'>nullptr</a>);
    <a href='SkIRect_Reference#SkIRect'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromEncoded'>MakeFromEncoded</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkData'>SkData</a>> <a href='undocumented#SkData'>encoded</a>, <a href='undocumented#SkData'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* <a href='SkIRect_Reference#SkIRect'>subset</a> = <a href='SkIRect_Reference#SkIRect'>nullptr</a>);

    <a href='SkIRect_Reference#SkIRect'>typedef</a> <a href='SkIRect_Reference#SkIRect'>void</a> (*<a href='#SkImage_TextureReleaseProc'>TextureReleaseProc</a>)(<a href='#SkImage_ReleaseContext'>ReleaseContext</a> <a href='#SkImage_ReleaseContext'>releaseContext</a>);

    <a href='#SkImage_ReleaseContext'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromTexture'>MakeFromTexture</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>,
                                          <a href='undocumented#GrContext'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& <a href='undocumented#GrBackendTexture'>backendTexture</a>,
                                          <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>origin</a>,
                                          <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkImage_colorType'>colorType</a>,
                                          <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImage_alphaType'>alphaType</a>,
                                          <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='#SkImage_colorSpace'>colorSpace</a>);
    <a href='#SkImage_colorSpace'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromTexture'>MakeFromTexture</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>,
                                          <a href='undocumented#GrContext'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& <a href='undocumented#GrBackendTexture'>backendTexture</a>,
                                          <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>origin</a>,
                                          <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkImage_colorType'>colorType</a>,
                                          <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImage_alphaType'>alphaType</a>,
                                          <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='#SkImage_colorSpace'>colorSpace</a>,
                                          <a href='#SkImage_TextureReleaseProc'>TextureReleaseProc</a> <a href='#SkImage_TextureReleaseProc'>textureReleaseProc</a>,
                                          <a href='#SkImage_ReleaseContext'>ReleaseContext</a> <a href='#SkImage_ReleaseContext'>releaseContext</a>);
    <a href='#SkImage_ReleaseContext'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeCrossContextFromEncoded'>MakeCrossContextFromEncoded</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>, <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkData'>SkData</a>> <a href='undocumented#Data'>data</a>,
                                                      <a href='undocumented#Data'>bool</a> <a href='undocumented#Data'>buildMips</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='undocumented#SkColorSpace'>dstColorSpace</a>,
                                                      <a href='undocumented#SkColorSpace'>bool</a> <a href='undocumented#SkColorSpace'>limitToMaxTextureSize</a> = <a href='undocumented#SkColorSpace'>false</a>);
    <a href='undocumented#SkColorSpace'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeCrossContextFromPixmap'>MakeCrossContextFromPixmap</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>, <a href='undocumented#GrContext'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#Pixmap'>pixmap</a>,
                                                     <a href='SkPixmap_Reference#Pixmap'>bool</a> <a href='SkPixmap_Reference#Pixmap'>buildMips</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='undocumented#SkColorSpace'>dstColorSpace</a>,
                                                     <a href='undocumented#SkColorSpace'>bool</a> <a href='undocumented#SkColorSpace'>limitToMaxTextureSize</a> = <a href='undocumented#SkColorSpace'>false</a>);
    <a href='undocumented#SkColorSpace'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromAdoptedTexture'>MakeFromAdoptedTexture</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>,
                                                 <a href='undocumented#GrContext'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& <a href='undocumented#GrBackendTexture'>backendTexture</a>,
                                                 <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>surfaceOrigin</a>,
                                                 <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkImage_colorType'>colorType</a>,
                                                 <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImage_alphaType'>alphaType</a> = <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
                                                 <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='#SkImage_colorSpace'>colorSpace</a> = <a href='#SkImage_colorSpace'>nullptr</a>);
    <a href='#SkImage_colorSpace'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromYUVATexturesCopy'>MakeFromYUVATexturesCopy</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>,
                                                   <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> <a href='SkImageInfo_Reference#SkYUVColorSpace'>yuvColorSpace</a>,
                                                   <a href='SkImageInfo_Reference#SkYUVColorSpace'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='undocumented#GrBackendTexture'>yuvaTextures</a>[],
                                                   <a href='undocumented#GrBackendTexture'>const</a> <a href='undocumented#SkYUVAIndex'>SkYUVAIndex</a> <a href='undocumented#SkYUVAIndex'>yuvaIndices</a>[4],
                                                   <a href='undocumented#SkISize'>SkISize</a> <a href='undocumented#SkISize'>imageSize</a>,
                                                   <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>imageOrigin</a>,
                                                   <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='undocumented#SkColorSpace'>imageColorSpace</a> = <a href='undocumented#SkColorSpace'>nullptr</a>);
    <a href='undocumented#SkColorSpace'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromYUVATextures'>MakeFromYUVATextures</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>,
                                               <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> <a href='SkImageInfo_Reference#SkYUVColorSpace'>yuvColorSpace</a>,
                                               <a href='SkImageInfo_Reference#SkYUVColorSpace'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='undocumented#GrBackendTexture'>yuvaTextures</a>[],
                                               <a href='undocumented#GrBackendTexture'>const</a> <a href='undocumented#SkYUVAIndex'>SkYUVAIndex</a> <a href='undocumented#SkYUVAIndex'>yuvaIndices</a>[4],
                                               <a href='undocumented#SkISize'>SkISize</a> <a href='undocumented#SkISize'>imageSize</a>,
                                               <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>imageOrigin</a>,
                                               <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='undocumented#SkColorSpace'>imageColorSpace</a> = <a href='undocumented#SkColorSpace'>nullptr</a>);
    <a href='undocumented#SkColorSpace'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend'>MakeFromYUVATexturesCopyWithExternalBackend</a>(
            <a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>,
            <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> <a href='SkImageInfo_Reference#SkYUVColorSpace'>yuvColorSpace</a>,
            <a href='SkImageInfo_Reference#SkYUVColorSpace'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='undocumented#GrBackendTexture'>yuvaTextures</a>[],
            <a href='undocumented#GrBackendTexture'>const</a> <a href='undocumented#SkYUVAIndex'>SkYUVAIndex</a> <a href='undocumented#SkYUVAIndex'>yuvaIndices</a>[4],
            <a href='undocumented#SkISize'>SkISize</a> <a href='undocumented#SkISize'>imageSize</a>,
            <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>imageOrigin</a>,
            <a href='undocumented#GrSurfaceOrigin'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& <a href='undocumented#GrBackendTexture'>backendTexture</a>,
            <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='undocumented#SkColorSpace'>imageColorSpace</a> = <a href='undocumented#SkColorSpace'>nullptr</a>);
    <a href='undocumented#SkColorSpace'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromYUVTexturesCopy'>MakeFromYUVTexturesCopy</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>, <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> <a href='SkImageInfo_Reference#SkYUVColorSpace'>yuvColorSpace</a>,
                                                  <a href='SkImageInfo_Reference#SkYUVColorSpace'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='undocumented#GrBackendTexture'>yuvTextures</a>[3],
                                                  <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>imageOrigin</a>,
                                                  <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='undocumented#SkColorSpace'>imageColorSpace</a> = <a href='undocumented#SkColorSpace'>nullptr</a>);
    <a href='undocumented#SkColorSpace'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend'>MakeFromYUVTexturesCopyWithExternalBackend</a>(
            <a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>, <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> <a href='SkImageInfo_Reference#SkYUVColorSpace'>yuvColorSpace</a>,
            <a href='SkImageInfo_Reference#SkYUVColorSpace'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='undocumented#GrBackendTexture'>yuvTextures</a>[3], <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>imageOrigin</a>,
            <a href='undocumented#GrSurfaceOrigin'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& <a href='undocumented#GrBackendTexture'>backendTexture</a>, <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='undocumented#SkColorSpace'>imageColorSpace</a> = <a href='undocumented#SkColorSpace'>nullptr</a>);
    <a href='undocumented#SkColorSpace'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromNV12TexturesCopy'>MakeFromNV12TexturesCopy</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>,
                                                   <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> <a href='SkImageInfo_Reference#SkYUVColorSpace'>yuvColorSpace</a>,
                                                   <a href='SkImageInfo_Reference#SkYUVColorSpace'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='undocumented#GrBackendTexture'>nv12Textures</a>[2],
                                                   <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>imageOrigin</a>,
                                                   <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='undocumented#SkColorSpace'>imageColorSpace</a> = <a href='undocumented#SkColorSpace'>nullptr</a>);
    <a href='undocumented#SkColorSpace'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend'>MakeFromNV12TexturesCopyWithExternalBackend</a>(
            <a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>,
            <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> <a href='SkImageInfo_Reference#SkYUVColorSpace'>yuvColorSpace</a>,
            <a href='SkImageInfo_Reference#SkYUVColorSpace'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='undocumented#GrBackendTexture'>nv12Textures</a>[2],
            <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>imageOrigin</a>,
            <a href='undocumented#GrSurfaceOrigin'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& <a href='undocumented#GrBackendTexture'>backendTexture</a>,
            <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='undocumented#SkColorSpace'>imageColorSpace</a> = <a href='undocumented#SkColorSpace'>nullptr</a>);

    <a href='undocumented#SkColorSpace'>enum</a> <a href='undocumented#SkColorSpace'>class</a> <a href='#SkImage_BitDepth'>BitDepth</a> {
        <a href='#SkImage_BitDepth'>kU8</a>,
        <a href='#SkImage_BitDepth'>kF16</a>,
    };

    <a href='#SkImage_BitDepth'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromPicture'>MakeFromPicture</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='SkPicture_Reference#SkPicture'>SkPicture</a>> <a href='SkPicture_Reference#Picture'>picture</a>, <a href='SkPicture_Reference#Picture'>const</a> <a href='undocumented#SkISize'>SkISize</a>& <a href='undocumented#SkISize'>dimensions</a>,
                                          <a href='undocumented#SkISize'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#Matrix'>matrix</a>, <a href='SkMatrix_Reference#Matrix'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                                          <a href='#SkImage_BitDepth'>BitDepth</a> <a href='#SkImage_BitDepth'>bitDepth</a>,
                                          <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='#SkImage_colorSpace'>colorSpace</a>);
    <a href='#SkImage_colorSpace'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_MakeFromAHardwareBuffer'>MakeFromAHardwareBuffer</a>(
            <a href='#SkImage_MakeFromAHardwareBuffer'>AHardwareBuffer</a>* <a href='#SkImage_MakeFromAHardwareBuffer'>hardwareBuffer</a>,
            <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImage_alphaType'>alphaType</a> = <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
            <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='#SkImage_colorSpace'>colorSpace</a> = <a href='#SkImage_colorSpace'>nullptr</a>,
            <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>surfaceOrigin</a> = <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a>);
    <a href='undocumented#kTopLeft_GrSurfaceOrigin'>int</a> <a href='#SkImage_width'>width()</a> <a href='#SkImage_width'>const</a>;
    <a href='#SkImage_width'>int</a> <a href='#SkImage_height'>height()</a> <a href='#SkImage_height'>const</a>;
    <a href='undocumented#SkISize'>SkISize</a> <a href='#SkImage_dimensions'>dimensions()</a> <a href='#SkImage_dimensions'>const</a>;
    <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkImage_bounds'>bounds()</a> <a href='#SkImage_bounds'>const</a>;
    <a href='#SkImage_bounds'>uint32_t</a> <a href='#SkImage_uniqueID'>uniqueID</a>() <a href='#SkImage_uniqueID'>const</a>;
    <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImage_alphaType'>alphaType</a>() <a href='#SkImage_alphaType'>const</a>;
    <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkImage_colorType'>colorType</a>() <a href='#SkImage_colorType'>const</a>;
    <a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='#SkImage_colorSpace'>colorSpace</a>() <a href='#SkImage_colorSpace'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='#SkImage_refColorSpace'>refColorSpace</a>() <a href='#SkImage_refColorSpace'>const</a>;
    <a href='#SkImage_refColorSpace'>bool</a> <a href='#SkImage_isAlphaOnly'>isAlphaOnly</a>() <a href='#SkImage_isAlphaOnly'>const</a>;
    <a href='#SkImage_isAlphaOnly'>bool</a> <a href='#SkImage_isOpaque'>isOpaque</a>() <a href='#SkImage_isOpaque'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkShader'>SkShader</a>> <a href='#SkImage_makeShader'>makeShader</a>(<a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_TileMode'>TileMode</a> <a href='#SkShader_TileMode'>tileMode1</a>, <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_TileMode'>TileMode</a> <a href='#SkShader_TileMode'>tileMode2</a>,
                               <a href='#SkShader_TileMode'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#SkMatrix'>localMatrix</a> = <a href='SkMatrix_Reference#SkMatrix'>nullptr</a>) <a href='SkMatrix_Reference#SkMatrix'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkShader'>SkShader</a>> <a href='#SkImage_makeShader'>makeShader</a>(<a href='#SkImage_makeShader'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#SkMatrix'>localMatrix</a> = <a href='SkMatrix_Reference#SkMatrix'>nullptr</a>) <a href='SkMatrix_Reference#SkMatrix'>const</a>;
    <a href='SkMatrix_Reference#SkMatrix'>bool</a> <a href='#SkImage_peekPixels'>peekPixels</a>(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>* <a href='SkPixmap_Reference#Pixmap'>pixmap</a>) <a href='SkPixmap_Reference#Pixmap'>const</a>;
    <a href='SkPixmap_Reference#Pixmap'>GrTexture</a>* <a href='#SkImage_getTexture'>getTexture</a>() <a href='#SkImage_getTexture'>const</a>;
    <a href='#SkImage_getTexture'>bool</a> <a href='#SkImage_isTextureBacked'>isTextureBacked</a>() <a href='#SkImage_isTextureBacked'>const</a>;
    <a href='#SkImage_isTextureBacked'>bool</a> <a href='#SkImage_isValid'>isValid</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>) <a href='undocumented#GrContext'>const</a>;
    <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='#SkImage_getBackendTexture'>getBackendTexture</a>(<a href='#SkImage_getBackendTexture'>bool</a> <a href='#SkImage_getBackendTexture'>flushPendingGrContextIO</a>,
                                       <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a>* <a href='undocumented#GrSurfaceOrigin'>origin</a> = <a href='undocumented#GrSurfaceOrigin'>nullptr</a>) <a href='undocumented#GrSurfaceOrigin'>const</a>;

    <a href='undocumented#GrSurfaceOrigin'>enum</a> <a href='#SkImage_CachingHint'>CachingHint</a> {
        <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>,
        <a href='#SkImage_kDisallow_CachingHint'>kDisallow_CachingHint</a>,
    };

    <a href='#SkImage_kDisallow_CachingHint'>bool</a> <a href='#SkImage_readPixels'>readPixels</a>(<a href='#SkImage_readPixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>dstInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>dstPixels</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>dstRowBytes</a>,
                    <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>srcX</a>, <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>srcY</a>, <a href='#SkImage_CachingHint'>CachingHint</a> <a href='#SkImage_CachingHint'>cachingHint</a> = <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>) <a href='#SkImage_kAllow_CachingHint'>const</a>;
    <a href='#SkImage_kAllow_CachingHint'>bool</a> <a href='#SkImage_readPixels'>readPixels</a>(<a href='#SkImage_readPixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#SkPixmap'>dst</a>, <a href='SkPixmap_Reference#SkPixmap'>int</a> <a href='SkPixmap_Reference#SkPixmap'>srcX</a>, <a href='SkPixmap_Reference#SkPixmap'>int</a> <a href='SkPixmap_Reference#SkPixmap'>srcY</a>,
                    <a href='#SkImage_CachingHint'>CachingHint</a> <a href='#SkImage_CachingHint'>cachingHint</a> = <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>) <a href='#SkImage_kAllow_CachingHint'>const</a>;
    <a href='#SkImage_kAllow_CachingHint'>bool</a> <a href='#SkImage_scalePixels'>scalePixels</a>(<a href='#SkImage_scalePixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#SkPixmap'>dst</a>, <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> <a href='undocumented#SkFilterQuality'>filterQuality</a>,
                     <a href='#SkImage_CachingHint'>CachingHint</a> <a href='#SkImage_CachingHint'>cachingHint</a> = <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>) <a href='#SkImage_kAllow_CachingHint'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkData'>SkData</a>> <a href='#SkImage_encodeToData'>encodeToData</a>(<a href='undocumented#SkEncodedImageFormat'>SkEncodedImageFormat</a> <a href='undocumented#SkEncodedImageFormat'>encodedImageFormat</a>, <a href='undocumented#SkEncodedImageFormat'>int</a> <a href='undocumented#SkEncodedImageFormat'>quality</a>) <a href='undocumented#SkEncodedImageFormat'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkData'>SkData</a>> <a href='#SkImage_encodeToData'>encodeToData</a>() <a href='#SkImage_encodeToData'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkData'>SkData</a>> <a href='#SkImage_refEncodedData'>refEncodedData</a>() <a href='#SkImage_refEncodedData'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_makeSubset'>makeSubset</a>(<a href='#SkImage_makeSubset'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>subset</a>) <a href='SkIRect_Reference#SkIRect'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_makeTextureImage'>makeTextureImage</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='undocumented#SkColorSpace'>dstColorSpace</a>,
                                    <a href='undocumented#GrMipMapped'>GrMipMapped</a> <a href='undocumented#GrMipMapped'>mipMapped</a> = <a href='undocumented#GrMipMapped'>GrMipMapped</a>::<a href='#GrMipMapped_kNo'>kNo</a>) <a href='#GrMipMapped_kNo'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_makeNonTextureImage'>makeNonTextureImage</a>() <a href='#SkImage_makeNonTextureImage'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_makeRasterImage'>makeRasterImage</a>() <a href='#SkImage_makeRasterImage'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_makeWithFilter'>makeWithFilter</a>(<a href='#SkImage_makeWithFilter'>const</a> <a href='undocumented#SkImageFilter'>SkImageFilter</a>* <a href='undocumented#SkImageFilter'>filter</a>, <a href='undocumented#SkImageFilter'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>subset</a>,
                                  <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>clipBounds</a>, <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* <a href='SkIRect_Reference#SkIRect'>outSubset</a>,
                                  <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>* <a href='SkIPoint_Reference#SkIPoint'>offset</a>) <a href='SkIPoint_Reference#SkIPoint'>const</a>;

    <a href='SkIPoint_Reference#SkIPoint'>typedef</a> <a href='SkIPoint_Reference#SkIPoint'>std</a>::<a href='SkIPoint_Reference#SkIPoint'>function</a><<a href='SkIPoint_Reference#SkIPoint'>void</a>(<a href='undocumented#GrBackendTexture'>GrBackendTexture</a>)> <a href='#SkImage_BackendTextureReleaseProc'>BackendTextureReleaseProc</a>;

    <a href='#SkImage_BackendTextureReleaseProc'>static</a> <a href='#SkImage_BackendTextureReleaseProc'>bool</a> <a href='#SkImage_MakeBackendTextureFromSkImage'>MakeBackendTextureFromSkImage</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>,
                                              <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='SkImage_Reference#Image'>image</a>,
                                              <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>* <a href='undocumented#GrBackendTexture'>backendTexture</a>,
                                              <a href='#SkImage_BackendTextureReleaseProc'>BackendTextureReleaseProc</a>* <a href='#SkImage_BackendTextureReleaseProc'>backendTextureReleaseProc</a>);

    <a href='#SkImage_BackendTextureReleaseProc'>enum</a> <a href='#SkImage_LegacyBitmapMode'>LegacyBitmapMode</a> {
        <a href='#SkImage_kRO_LegacyBitmapMode'>kRO_LegacyBitmapMode</a>,
    };

    <a href='#SkImage_kRO_LegacyBitmapMode'>bool</a> <a href='#SkImage_asLegacyBitmap'>asLegacyBitmap</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* <a href='SkBitmap_Reference#Bitmap'>bitmap</a>,
                        <a href='#SkImage_LegacyBitmapMode'>LegacyBitmapMode</a> <a href='#SkImage_LegacyBitmapMode'>legacyBitmapMode</a> = <a href='#SkImage_kRO_LegacyBitmapMode'>kRO_LegacyBitmapMode</a>) <a href='#SkImage_kRO_LegacyBitmapMode'>const</a>;
    <a href='#SkImage_kRO_LegacyBitmapMode'>bool</a> <a href='#SkImage_isLazyGenerated'>isLazyGenerated</a>() <a href='#SkImage_isLazyGenerated'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkImage_Reference#SkImage'>SkImage</a>> <a href='#SkImage_makeColorSpace'>makeColorSpace</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='undocumented#SkColorSpace'>target</a>) <a href='undocumented#SkColorSpace'>const</a>;
};
</pre>

<a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>describes</a> <a href='SkImage_Reference#Image'>a</a> <a href='SkImage_Reference#Image'>two</a> <a href='SkImage_Reference#Image'>dimensional</a> <a href='SkImage_Reference#Image'>array</a> <a href='SkImage_Reference#Image'>of</a> <a href='SkImage_Reference#Image'>pixels</a> <a href='SkImage_Reference#Image'>to</a> <a href='SkImage_Reference#Image'>draw</a>. <a href='SkImage_Reference#Image'>The</a> <a href='SkImage_Reference#Image'>pixels</a> <a href='SkImage_Reference#Image'>may</a> <a href='SkImage_Reference#Image'>be</a>
<a href='SkImage_Reference#Image'>decoded</a> <a href='SkImage_Reference#Image'>in</a> <a href='SkImage_Reference#Image'>a</a> <a href='#Raster_Bitmap'>Raster_Bitmap</a>, <a href='#Raster_Bitmap'>encoded</a> <a href='#Raster_Bitmap'>in</a> <a href='#Raster_Bitmap'>a</a> <a href='SkPicture_Reference#Picture'>Picture</a> <a href='SkPicture_Reference#Picture'>or</a> <a href='SkPicture_Reference#Picture'>compressed</a> <a href='undocumented#Data'>data</a> <a href='SkStream_Reference#Stream'>stream</a>,
<a href='SkStream_Reference#Stream'>or</a> <a href='SkStream_Reference#Stream'>located</a> <a href='SkStream_Reference#Stream'>in</a> <a href='SkStream_Reference#Stream'>GPU</a> <a href='SkStream_Reference#Stream'>memory</a> <a href='SkStream_Reference#Stream'>as</a> <a href='SkStream_Reference#Stream'>a</a> <a href='#GPU_Texture'>GPU_Texture</a>.

<a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>cannot</a> <a href='SkImage_Reference#Image'>be</a> <a href='SkImage_Reference#Image'>modified</a> <a href='SkImage_Reference#Image'>after</a> <a href='SkImage_Reference#Image'>it</a> <a href='SkImage_Reference#Image'>is</a> <a href='SkImage_Reference#Image'>created</a>. <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>may</a> <a href='SkImage_Reference#Image'>allocate</a> <a href='SkImage_Reference#Image'>additional</a>
<a href='SkImage_Reference#Image'>storage</a> <a href='SkImage_Reference#Image'>as</a> <a href='SkImage_Reference#Image'>needed</a>; <a href='SkImage_Reference#Image'>for</a> <a href='SkImage_Reference#Image'>instance</a>, <a href='SkImage_Reference#Image'>an</a> <a href='SkImage_Reference#Image'>encoded</a> <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>may</a> <a href='SkImage_Reference#Image'>decode</a> <a href='SkImage_Reference#Image'>when</a> <a href='SkImage_Reference#Image'>drawn</a>.

<a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>width</a> <a href='SkImage_Reference#Image'>and</a> <a href='SkImage_Reference#Image'>height</a> <a href='SkImage_Reference#Image'>are</a> <a href='SkImage_Reference#Image'>greater</a> <a href='SkImage_Reference#Image'>than</a> <a href='SkImage_Reference#Image'>zero</a>. <a href='SkImage_Reference#Image'>Creating</a> <a href='SkImage_Reference#Image'>an</a> <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>with</a> <a href='SkImage_Reference#Image'>zero</a> <a href='SkImage_Reference#Image'>width</a>
<a href='SkImage_Reference#Image'>or</a> <a href='SkImage_Reference#Image'>height</a> <a href='SkImage_Reference#Image'>returns</a> <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>equal</a> <a href='SkImage_Reference#Image'>to</a> <a href='SkImage_Reference#Image'>nullptr</a>.

<a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>may</a> <a href='SkImage_Reference#Image'>be</a> <a href='SkImage_Reference#Image'>created</a> <a href='SkImage_Reference#Image'>from</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a>, <a href='SkPixmap_Reference#Pixmap'>Pixmap</a>, <a href='SkSurface_Reference#Surface'>Surface</a>, <a href='SkPicture_Reference#Picture'>Picture</a>, <a href='SkPicture_Reference#Picture'>encoded</a> <a href='SkPicture_Reference#Picture'>streams</a>,
<a href='#GPU_Texture'>GPU_Texture</a>, <a href='#Image_Info_YUV_ColorSpace'>YUV_ColorSpace</a> <a href='undocumented#Data'>data</a>, <a href='undocumented#Data'>or</a> <a href='undocumented#Data'>hardware</a> <a href='undocumented#Data'>buffer</a>. <a href='undocumented#Data'>Encoded</a> <a href='undocumented#Data'>streams</a> <a href='undocumented#Data'>supported</a>
<a href='undocumented#Data'>include</a> <a href='undocumented#Data'>BMP</a>, <a href='undocumented#Data'>GIF</a>, <a href='undocumented#Data'>HEIF</a>, <a href='undocumented#Data'>ICO</a>, <a href='undocumented#Data'>JPEG</a>, <a href='undocumented#Data'>PNG</a>, <a href='undocumented#Data'>WBMP</a>, <a href='undocumented#Data'>WebP</a>. <a href='undocumented#Data'>Supported</a> <a href='undocumented#Data'>encoding</a> <a href='undocumented#Data'>details</a>
<a href='undocumented#Data'>vary</a> <a href='undocumented#Data'>with</a> <a href='undocumented#Data'>platform</a>.

<a name='Raster_Image'></a>

<a href='#Image_Raster_Image'>Raster_Image</a> <a href='#Image_Raster_Image'>pixels</a> <a href='#Image_Raster_Image'>are</a> <a href='#Image_Raster_Image'>decoded</a> <a href='#Image_Raster_Image'>in</a> <a href='#Image_Raster_Image'>a</a> <a href='#Raster_Bitmap'>Raster_Bitmap</a>. <a href='#Raster_Bitmap'>These</a> <a href='#Raster_Bitmap'>pixels</a> <a href='#Raster_Bitmap'>may</a> <a href='#Raster_Bitmap'>be</a> <a href='#Raster_Bitmap'>read</a>
<a href='#Raster_Bitmap'>directly</a> <a href='#Raster_Bitmap'>and</a> <a href='#Raster_Bitmap'>in</a> <a href='#Raster_Bitmap'>most</a> <a href='#Raster_Bitmap'>cases</a> <a href='#Raster_Bitmap'>written</a> <a href='#Raster_Bitmap'>to</a>, <a href='#Raster_Bitmap'>although</a> <a href='#Raster_Bitmap'>edited</a> <a href='#Raster_Bitmap'>pixels</a> <a href='#Raster_Bitmap'>may</a> <a href='#Raster_Bitmap'>not</a> <a href='#Raster_Bitmap'>be</a> <a href='#Raster_Bitmap'>drawn</a>
<a href='#Raster_Bitmap'>if</a> <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>has</a> <a href='SkImage_Reference#Image'>been</a> <a href='SkImage_Reference#Image'>copied</a> <a href='SkImage_Reference#Image'>internally</a>.

<a name='Texture_Image'></a>

<a href='#Image_Texture_Image'>Texture_Image</a> <a href='#Image_Texture_Image'>are</a> <a href='#Image_Texture_Image'>located</a> <a href='#Image_Texture_Image'>on</a> <a href='#Image_Texture_Image'>GPU</a> <a href='#Image_Texture_Image'>and</a> <a href='#Image_Texture_Image'>pixels</a> <a href='#Image_Texture_Image'>are</a> <a href='#Image_Texture_Image'>not</a> <a href='#Image_Texture_Image'>accessible</a>. <a href='#Image_Texture_Image'>Texture_Image</a>
<a href='#Image_Texture_Image'>are</a> <a href='#Image_Texture_Image'>allocated</a> <a href='#Image_Texture_Image'>optimally</a> <a href='#Image_Texture_Image'>for</a> <a href='#Image_Texture_Image'>best</a> <a href='#Image_Texture_Image'>performance</a>. <a href='#Image_Raster_Image'>Raster_Image</a> <a href='#Image_Raster_Image'>may</a>
<a href='#Image_Raster_Image'>be</a> <a href='#Image_Raster_Image'>drawn</a> <a href='#Image_Raster_Image'>to</a> <a href='#GPU_Surface'>GPU_Surface</a>, <a href='#GPU_Surface'>but</a> <a href='#GPU_Surface'>pixels</a> <a href='#GPU_Surface'>are</a> <a href='#GPU_Surface'>uploaded</a> <a href='#GPU_Surface'>from</a> <a href='#GPU_Surface'>CPU</a> <a href='#GPU_Surface'>to</a> <a href='#GPU_Surface'>GPU</a> <a href='#GPU_Surface'>downgrading</a>
<a href='#GPU_Surface'>performance</a>.

<a name='Lazy_Image'></a>

<a href='#Image_Lazy_Image'>Lazy_Image</a> <a href='#Image_Lazy_Image'>defer</a> <a href='#Image_Lazy_Image'>allocating</a> <a href='#Image_Lazy_Image'>buffer</a> <a href='#Image_Lazy_Image'>for</a> <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>pixels</a> <a href='SkImage_Reference#Image'>and</a> <a href='SkImage_Reference#Image'>decoding</a> <a href='SkStream_Reference#Stream'>stream</a> <a href='SkStream_Reference#Stream'>until</a>
<a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>is</a> <a href='SkImage_Reference#Image'>drawn</a>. <a href='#Image_Lazy_Image'>Lazy_Image</a> <a href='#Image_Lazy_Image'>caches</a> <a href='#Image_Lazy_Image'>result</a> <a href='#Image_Lazy_Image'>if</a> <a href='#Image_Lazy_Image'>possible</a> <a href='#Image_Lazy_Image'>to</a> <a href='#Image_Lazy_Image'>speed</a> <a href='#Image_Lazy_Image'>up</a> <a href='#Image_Lazy_Image'>repeated</a>
<a href='#Image_Lazy_Image'>drawing</a>.

<a name='SkImage_MakeRasterCopy'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_MakeRasterCopy'>MakeRasterCopy</a>(<a href='#SkImage_MakeRasterCopy'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#Pixmap'>pixmap</a>)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>from</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='SkPixmap_Reference#SkPixmap'>and</a> <a href='SkPixmap_Reference#SkPixmap'>copy</a> <a href='SkPixmap_Reference#SkPixmap'>of</a> <a href='SkPixmap_Reference#SkPixmap'>pixels</a>. <a href='SkPixmap_Reference#SkPixmap'>Since</a> <a href='SkPixmap_Reference#SkPixmap'>pixels</a> <a href='SkPixmap_Reference#SkPixmap'>are</a> <a href='SkPixmap_Reference#SkPixmap'>copied</a>, <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>
pixels may be modified or deleted without affecting <a href='SkImage_Reference#SkImage'>SkImage</a>.

<a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>returned</a> <a href='SkImage_Reference#SkImage'>if</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='SkPixmap_Reference#SkPixmap'>is</a> <a href='SkPixmap_Reference#SkPixmap'>valid</a>. <a href='SkPixmap_Reference#SkPixmap'>Valid</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='SkPixmap_Reference#SkPixmap'>parameters</a> <a href='SkPixmap_Reference#SkPixmap'>include</a>:
dimensions are greater than zero;
each dimension fits in 29 bits;
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>and</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>are</a> <a href='SkImageInfo_Reference#SkAlphaType'>valid</a>, <a href='SkImageInfo_Reference#SkAlphaType'>and</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>not</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>;
row bytes are large enough to hold one row of pixels;
<a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>not</a> <a href='undocumented#Pixel'>nullptr</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeRasterCopy_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a>, <a href='undocumented#Pixel'>and</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>bytes</a></td>
  </tr>
</table>

### Return Value

copy of <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='SkPixmap_Reference#SkPixmap'>pixels</a>, <a href='SkPixmap_Reference#SkPixmap'>or</a> <a href='SkPixmap_Reference#SkPixmap'>nullptr</a>

### Example

<div><fiddle-embed name="513afec5795a9504ebf6af5373d16b6b"><div>Draw a five by five <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='SkBitmap_Reference#Bitmap'>and</a> <a href='SkBitmap_Reference#Bitmap'>draw</a> <a href='SkBitmap_Reference#Bitmap'>a</a> <a href='SkBitmap_Reference#Bitmap'>copy</a> <a href='SkBitmap_Reference#Bitmap'>in</a> <a href='SkBitmap_Reference#Bitmap'>an</a> <a href='SkImage_Reference#Image'>Image</a>. <a href='SkImage_Reference#Image'>Editing</a> <a href='SkImage_Reference#Image'>the</a> <a href='#SkImage_MakeRasterCopy_pixmap'>pixmap</a>
<a href='#SkImage_MakeRasterCopy_pixmap'>alters</a> <a href='#SkImage_MakeRasterCopy_pixmap'>the</a>  <a href='SkBitmap_Reference#Bitmap_Draw'>bitmap draw</a>, <a href='SkBitmap_Reference#Bitmap'>but</a> <a href='SkBitmap_Reference#Bitmap'>does</a> <a href='SkBitmap_Reference#Bitmap'>not</a> <a href='SkBitmap_Reference#Bitmap'>alter</a> <a href='SkBitmap_Reference#Bitmap'>the</a> <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>draw</a> <a href='SkImage_Reference#Image'>since</a> <a href='SkImage_Reference#Image'>the</a> <a href='SkImage_Reference#Image'>Image</a>
<a href='SkImage_Reference#Image'>contains</a> <a href='SkImage_Reference#Image'>a</a> <a href='SkImage_Reference#Image'>copy</a> <a href='SkImage_Reference#Image'>of</a> <a href='SkImage_Reference#Image'>the</a> <a href='SkImage_Reference#Image'>pixels</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeRasterData'>MakeRasterData</a> <a href='#SkImage_MakeFromGenerator'>MakeFromGenerator</a>

<a name='SkImage_MakeRasterData'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_MakeRasterData'>MakeRasterData</a>(<a href='#SkImage_MakeRasterData'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkData'>SkData</a>&<a href='undocumented#SkData'>gt</a>; <a href='undocumented#SkData'>pixels</a>, <a href='undocumented#SkData'>size_t</a> <a href='undocumented#SkData'>rowBytes</a>)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>from</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>sharing</a> <a href='#SkImage_MakeRasterData_pixels'>pixels</a>.

<a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>returned</a> <a href='SkImage_Reference#SkImage'>if</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>is</a> <a href='SkImageInfo_Reference#SkImageInfo'>valid</a>. <a href='SkImageInfo_Reference#SkImageInfo'>Valid</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>parameters</a> <a href='SkImageInfo_Reference#SkImageInfo'>include</a>:
dimensions are greater than zero;
each dimension fits in 29 bits;
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>and</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>are</a> <a href='SkImageInfo_Reference#SkAlphaType'>valid</a>, <a href='SkImageInfo_Reference#SkAlphaType'>and</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>not</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>;
<a href='#SkImage_MakeRasterData_rowBytes'>rowBytes</a> <a href='#SkImage_MakeRasterData_rowBytes'>are</a> <a href='#SkImage_MakeRasterData_rowBytes'>large</a> <a href='#SkImage_MakeRasterData_rowBytes'>enough</a> <a href='#SkImage_MakeRasterData_rowBytes'>to</a> <a href='#SkImage_MakeRasterData_rowBytes'>hold</a> <a href='#SkImage_MakeRasterData_rowBytes'>one</a> <a href='#SkImage_MakeRasterData_rowBytes'>row</a> <a href='#SkImage_MakeRasterData_rowBytes'>of</a> <a href='#SkImage_MakeRasterData_pixels'>pixels</a>;
<a href='#SkImage_MakeRasterData_pixels'>pixels</a> <a href='#SkImage_MakeRasterData_pixels'>is</a> <a href='#SkImage_MakeRasterData_pixels'>not</a> <a href='#SkImage_MakeRasterData_pixels'>nullptr</a>, <a href='#SkImage_MakeRasterData_pixels'>and</a> <a href='#SkImage_MakeRasterData_pixels'>contains</a> <a href='#SkImage_MakeRasterData_pixels'>enough</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>for</a> <a href='SkImage_Reference#SkImage'>SkImage</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeRasterData_info'><code><strong>info</strong></code></a></td>
    <td>contains width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeRasterData_pixels'><code><strong>pixels</strong></code></a></td>
    <td>address or  <a href='undocumented#Pixel_Storage'>pixel storage</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeRasterData_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>larger</a></td>
  </tr>
</table>

### Return Value

<a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>sharing</a> <a href='#SkImage_MakeRasterData_pixels'>pixels</a>, <a href='#SkImage_MakeRasterData_pixels'>or</a> <a href='#SkImage_MakeRasterData_pixels'>nullptr</a>

### Example

<div><fiddle-embed name="22e7ce79ab2fe94252d23319f2258127"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeRasterCopy'>MakeRasterCopy</a> <a href='#SkImage_MakeFromGenerator'>MakeFromGenerator</a>

<a name='SkImage_ReleaseContext'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    typedef void* <a href='#SkImage_ReleaseContext'>ReleaseContext</a>;
</pre>

Caller <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>passed</a> <a href='undocumented#Data'>to</a> <a href='#SkImage_RasterReleaseProc'>RasterReleaseProc</a>; <a href='#SkImage_RasterReleaseProc'>may</a> <a href='#SkImage_RasterReleaseProc'>be</a> <a href='#SkImage_RasterReleaseProc'>nullptr</a>.

### See Also

<a href='#SkImage_MakeFromRaster'>MakeFromRaster</a> <a href='#SkImage_RasterReleaseProc'>RasterReleaseProc</a>

<a name='SkImage_RasterReleaseProc'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    typedef void (*<a href='#SkImage_RasterReleaseProc'>RasterReleaseProc</a>)(<a href='#SkImage_RasterReleaseProc'>const</a> <a href='#SkImage_RasterReleaseProc'>void</a>* <a href='#SkImage_RasterReleaseProc'>pixels</a>, <a href='#SkImage_ReleaseContext'>ReleaseContext</a>);
</pre>

Function called when <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>no</a> <a href='SkImage_Reference#Image'>longer</a> <a href='SkImage_Reference#Image'>shares</a> <a href='SkImage_Reference#Image'>pixels</a>. <a href='#SkImage_ReleaseContext'>ReleaseContext</a> <a href='#SkImage_ReleaseContext'>is</a>
<a href='#SkImage_ReleaseContext'>provided</a> <a href='#SkImage_ReleaseContext'>by</a> <a href='#SkImage_ReleaseContext'>caller</a> <a href='#SkImage_ReleaseContext'>when</a> <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>is</a> <a href='SkImage_Reference#Image'>created</a>, <a href='SkImage_Reference#Image'>and</a> <a href='SkImage_Reference#Image'>may</a> <a href='SkImage_Reference#Image'>be</a> <a href='SkImage_Reference#Image'>nullptr</a>.

### See Also

<a href='#SkImage_ReleaseContext'>ReleaseContext</a> <a href='#SkImage_MakeFromRaster'>MakeFromRaster</a>

<a name='SkImage_MakeFromRaster'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_MakeFromRaster'>MakeFromRaster</a>(<a href='#SkImage_MakeFromRaster'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#Pixmap'>pixmap</a>, <a href='#SkImage_RasterReleaseProc'>RasterReleaseProc</a> <a href='#SkImage_RasterReleaseProc'>rasterReleaseProc</a>,
                                     <a href='#SkImage_ReleaseContext'>ReleaseContext</a> <a href='#SkImage_ReleaseContext'>releaseContext</a>)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>from</a> <a href='#SkImage_MakeFromRaster_pixmap'>pixmap</a>, <a href='#SkImage_MakeFromRaster_pixmap'>sharing</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='SkPixmap_Reference#SkPixmap'>pixels</a>. <a href='SkPixmap_Reference#SkPixmap'>Pixels</a> <a href='SkPixmap_Reference#SkPixmap'>must</a> <a href='SkPixmap_Reference#SkPixmap'>remain</a> <a href='SkPixmap_Reference#SkPixmap'>valid</a> <a href='SkPixmap_Reference#SkPixmap'>and</a>
unchanged until <a href='#SkImage_MakeFromRaster_rasterReleaseProc'>rasterReleaseProc</a> <a href='#SkImage_MakeFromRaster_rasterReleaseProc'>is</a> <a href='#SkImage_MakeFromRaster_rasterReleaseProc'>called</a>. <a href='#SkImage_MakeFromRaster_rasterReleaseProc'>rasterReleaseProc</a> <a href='#SkImage_MakeFromRaster_rasterReleaseProc'>is</a> <a href='#SkImage_MakeFromRaster_rasterReleaseProc'>passed</a>
<a href='#SkImage_MakeFromRaster_releaseContext'>releaseContext</a> <a href='#SkImage_MakeFromRaster_releaseContext'>when</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>deleted</a> <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>no</a> <a href='SkImage_Reference#SkImage'>longer</a> <a href='SkImage_Reference#SkImage'>refers</a> <a href='SkImage_Reference#SkImage'>to</a>  <a href='SkPixmap_Reference#Pixmap_Pixels'>pixmap pixels</a>.

Pass nullptr for <a href='#SkImage_MakeFromRaster_rasterReleaseProc'>rasterReleaseProc</a> <a href='#SkImage_MakeFromRaster_rasterReleaseProc'>to</a> <a href='#SkImage_MakeFromRaster_rasterReleaseProc'>share</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='SkPixmap_Reference#SkPixmap'>without</a> <a href='SkPixmap_Reference#SkPixmap'>requiring</a> <a href='SkPixmap_Reference#SkPixmap'>a</a> <a href='SkPixmap_Reference#SkPixmap'>callback</a>
when <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>released</a>. <a href='SkImage_Reference#SkImage'>Pass</a> <a href='SkImage_Reference#SkImage'>nullptr</a> <a href='SkImage_Reference#SkImage'>for</a> <a href='#SkImage_MakeFromRaster_releaseContext'>releaseContext</a> <a href='#SkImage_MakeFromRaster_releaseContext'>if</a> <a href='#SkImage_MakeFromRaster_rasterReleaseProc'>rasterReleaseProc</a>
does not require state.

<a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>returned</a> <a href='SkImage_Reference#SkImage'>if</a> <a href='#SkImage_MakeFromRaster_pixmap'>pixmap</a> <a href='#SkImage_MakeFromRaster_pixmap'>is</a> <a href='#SkImage_MakeFromRaster_pixmap'>valid</a>. <a href='#SkImage_MakeFromRaster_pixmap'>Valid</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='SkPixmap_Reference#SkPixmap'>parameters</a> <a href='SkPixmap_Reference#SkPixmap'>include</a>:
dimensions are greater than zero;
each dimension fits in 29 bits;
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>and</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>are</a> <a href='SkImageInfo_Reference#SkAlphaType'>valid</a>, <a href='SkImageInfo_Reference#SkAlphaType'>and</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>not</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>;
row bytes are large enough to hold one row of pixels;
<a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>not</a> <a href='undocumented#Pixel'>nullptr</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromRaster_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a>, <a href='undocumented#Pixel'>and</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>bytes</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromRaster_rasterReleaseProc'><code><strong>rasterReleaseProc</strong></code></a></td>
    <td>function called when pixels can be released; or nullptr</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromRaster_releaseContext'><code><strong>releaseContext</strong></code></a></td>
    <td>state passed to <a href='#SkImage_MakeFromRaster_rasterReleaseProc'>rasterReleaseProc</a>; <a href='#SkImage_MakeFromRaster_rasterReleaseProc'>or</a> <a href='#SkImage_MakeFromRaster_rasterReleaseProc'>nullptr</a></td>
  </tr>
</table>

### Return Value

<a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>sharing</a> <a href='#SkImage_MakeFromRaster_pixmap'>pixmap</a>

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
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_MakeFromBitmap'>MakeFromBitmap</a>(<a href='#SkImage_MakeFromBitmap'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#Bitmap'>bitmap</a>)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>from</a> <a href='#SkImage_MakeFromBitmap_bitmap'>bitmap</a>, <a href='#SkImage_MakeFromBitmap_bitmap'>sharing</a> <a href='#SkImage_MakeFromBitmap_bitmap'>or</a> <a href='#SkImage_MakeFromBitmap_bitmap'>copying</a>  <a href='SkBitmap_Reference#Bitmap_Pixels'>bitmap pixels</a>. <a href='#SkImage_MakeFromBitmap_bitmap'>If</a> <a href='#SkImage_MakeFromBitmap_bitmap'>the</a> <a href='#SkImage_MakeFromBitmap_bitmap'>bitmap</a>
is marked immutable, and its <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>shareable</a>, <a href='undocumented#Pixel'>it</a> <a href='undocumented#Pixel'>may</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>shared</a>
instead of copied.

<a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>returned</a> <a href='SkImage_Reference#SkImage'>if</a> <a href='#SkImage_MakeFromBitmap_bitmap'>bitmap</a> <a href='#SkImage_MakeFromBitmap_bitmap'>is</a> <a href='#SkImage_MakeFromBitmap_bitmap'>valid</a>. <a href='#SkImage_MakeFromBitmap_bitmap'>Valid</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>parameters</a> <a href='SkBitmap_Reference#SkBitmap'>include</a>:
dimensions are greater than zero;
each dimension fits in 29 bits;
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>and</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>are</a> <a href='SkImageInfo_Reference#SkAlphaType'>valid</a>, <a href='SkImageInfo_Reference#SkAlphaType'>and</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>not</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>;
row bytes are large enough to hold one row of pixels;
<a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>not</a> <a href='undocumented#Pixel'>nullptr</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromBitmap_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>row</a> <a href='SkImageInfo_Reference#SkImageInfo'>bytes</a>, <a href='SkImageInfo_Reference#SkImageInfo'>and</a> <a href='SkImageInfo_Reference#SkImageInfo'>pixels</a></td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>

### Example

<div><fiddle-embed name="cf2cf53321e4e6a77c2841bfbc0ef707"><div>The first <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>is</a> <a href='SkBitmap_Reference#Bitmap'>shared</a>; <a href='SkBitmap_Reference#Bitmap'>writing</a> <a href='SkBitmap_Reference#Bitmap'>to</a> <a href='SkBitmap_Reference#Bitmap'>the</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a> <a href='undocumented#Pixel'>changes</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>first</a>
<a href='SkImage_Reference#Image'>Image</a>.
<a href='SkImage_Reference#Image'>The</a> <a href='SkImage_Reference#Image'>second</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>is</a> <a href='SkBitmap_Reference#Bitmap'>marked</a> <a href='SkBitmap_Reference#Bitmap'>immutable</a>, <a href='SkBitmap_Reference#Bitmap'>and</a> <a href='SkBitmap_Reference#Bitmap'>is</a> <a href='SkBitmap_Reference#Bitmap'>copied</a>; <a href='SkBitmap_Reference#Bitmap'>writing</a> <a href='SkBitmap_Reference#Bitmap'>to</a> <a href='SkBitmap_Reference#Bitmap'>the</a> <a href='undocumented#Pixel'>pixel</a>
<a href='undocumented#Pixel'>memory</a> <a href='undocumented#Pixel'>does</a> <a href='undocumented#Pixel'>not</a> <a href='undocumented#Pixel'>alter</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>second</a> <a href='SkImage_Reference#Image'>Image</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromRaster'>MakeFromRaster</a> <a href='#SkImage_MakeRasterCopy'>MakeRasterCopy</a> <a href='#SkImage_MakeFromGenerator'>MakeFromGenerator</a> <a href='#SkImage_MakeRasterData'>MakeRasterData</a>

<a name='SkImage_MakeFromGenerator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_MakeFromGenerator'>MakeFromGenerator</a>(<a href='#SkImage_MakeFromGenerator'>std</a>::<a href='#SkImage_MakeFromGenerator'>unique_ptr</a>&<a href='#SkImage_MakeFromGenerator'>lt</a>;<a href='undocumented#SkImageGenerator'>SkImageGenerator</a>&<a href='undocumented#SkImageGenerator'>gt</a>; <a href='undocumented#SkImageGenerator'>imageGenerator</a>,
                                 <a href='undocumented#SkImageGenerator'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* <a href='SkIRect_Reference#SkIRect'>subset</a> = <a href='SkIRect_Reference#SkIRect'>nullptr</a>)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>from</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>returned</a> <a href='undocumented#Data'>by</a> <a href='#SkImage_MakeFromGenerator_imageGenerator'>imageGenerator</a>. <a href='#SkImage_MakeFromGenerator_imageGenerator'>Generated</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>is</a> <a href='undocumented#Data'>owned</a> <a href='undocumented#Data'>by</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>and</a>
may not be shared or accessed.

<a href='#SkImage_MakeFromGenerator_subset'>subset</a> <a href='#SkImage_MakeFromGenerator_subset'>allows</a> <a href='#SkImage_MakeFromGenerator_subset'>selecting</a> <a href='#SkImage_MakeFromGenerator_subset'>a</a> <a href='#SkImage_MakeFromGenerator_subset'>portion</a> <a href='#SkImage_MakeFromGenerator_subset'>of</a> <a href='#SkImage_MakeFromGenerator_subset'>the</a> <a href='#SkImage_MakeFromGenerator_subset'>full</a> <a href='SkImage_Reference#Image'>image</a>. <a href='SkImage_Reference#Image'>Pass</a> <a href='SkImage_Reference#Image'>nullptr</a> <a href='SkImage_Reference#Image'>to</a> <a href='SkImage_Reference#Image'>select</a> <a href='SkImage_Reference#Image'>the</a> <a href='SkImage_Reference#Image'>entire</a>
<a href='SkImage_Reference#Image'>image</a>; <a href='SkImage_Reference#Image'>otherwise</a>, <a href='#SkImage_MakeFromGenerator_subset'>subset</a> <a href='#SkImage_MakeFromGenerator_subset'>must</a> <a href='#SkImage_MakeFromGenerator_subset'>be</a> <a href='#SkImage_MakeFromGenerator_subset'>contained</a> <a href='#SkImage_MakeFromGenerator_subset'>by</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>bounds</a>.

<a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>returned</a> <a href='SkImage_Reference#SkImage'>if</a> <a href='SkImage_Reference#SkImage'>generator</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>is</a> <a href='undocumented#Data'>valid</a>. <a href='undocumented#Data'>Valid</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>parameters</a> <a href='undocumented#Data'>vary</a> <a href='undocumented#Data'>by</a> <a href='undocumented#Data'>type</a> <a href='undocumented#Data'>of</a> <a href='undocumented#Data'>data</a>
and platform.

<a href='#SkImage_MakeFromGenerator_imageGenerator'>imageGenerator</a> <a href='#SkImage_MakeFromGenerator_imageGenerator'>may</a> <a href='#SkImage_MakeFromGenerator_imageGenerator'>wrap</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='undocumented#Data'>data</a>, <a href='undocumented#Data'>codec</a> <a href='undocumented#Data'>data</a>, <a href='undocumented#Data'>or</a> <a href='undocumented#Data'>custom</a> <a href='undocumented#Data'>data</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromGenerator_imageGenerator'><code><strong>imageGenerator</strong></code></a></td>
    <td>stock or custom routines to retrieve <a href='SkImage_Reference#SkImage'>SkImage</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromGenerator_subset'><code><strong>subset</strong></code></a></td>
    <td>bounds of returned <a href='SkImage_Reference#SkImage'>SkImage</a>; <a href='SkImage_Reference#SkImage'>may</a> <a href='SkImage_Reference#SkImage'>be</a> <a href='SkImage_Reference#SkImage'>nullptr</a></td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>

### Example

<div><fiddle-embed name="c2fec0746f88ca34d7dce59dd9bdef9e"><div>The generator returning <a href='SkPicture_Reference#Picture'>Picture</a> <a href='SkPicture_Reference#Picture'>cannot</a> <a href='SkPicture_Reference#Picture'>be</a> <a href='SkPicture_Reference#Picture'>shared</a>; <a href='SkPicture_Reference#Picture'>std</a>::<a href='SkPicture_Reference#Picture'>move</a> <a href='SkPicture_Reference#Picture'>transfers</a> <a href='SkPicture_Reference#Picture'>ownership</a> <a href='SkPicture_Reference#Picture'>to</a> <a href='SkPicture_Reference#Picture'>generated</a> <a href='SkImage_Reference#Image'>Image</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromEncoded'>MakeFromEncoded</a>

<a name='SkImage_MakeFromEncoded'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_MakeFromEncoded'>MakeFromEncoded</a>(<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkData'>SkData</a>&<a href='undocumented#SkData'>gt</a>; <a href='undocumented#SkData'>encoded</a>, <a href='undocumented#SkData'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* <a href='SkIRect_Reference#SkIRect'>subset</a> = <a href='SkIRect_Reference#SkIRect'>nullptr</a>)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>from</a> <a href='#SkImage_MakeFromEncoded_encoded'>encoded</a> <a href='undocumented#Data'>data</a>.
<a href='#SkImage_MakeFromEncoded_subset'>subset</a> <a href='#SkImage_MakeFromEncoded_subset'>allows</a> <a href='#SkImage_MakeFromEncoded_subset'>selecting</a> <a href='#SkImage_MakeFromEncoded_subset'>a</a> <a href='#SkImage_MakeFromEncoded_subset'>portion</a> <a href='#SkImage_MakeFromEncoded_subset'>of</a> <a href='#SkImage_MakeFromEncoded_subset'>the</a> <a href='#SkImage_MakeFromEncoded_subset'>full</a> <a href='SkImage_Reference#Image'>image</a>. <a href='SkImage_Reference#Image'>Pass</a> <a href='SkImage_Reference#Image'>nullptr</a> <a href='SkImage_Reference#Image'>to</a> <a href='SkImage_Reference#Image'>select</a> <a href='SkImage_Reference#Image'>the</a> <a href='SkImage_Reference#Image'>entire</a>
<a href='SkImage_Reference#Image'>image</a>; <a href='SkImage_Reference#Image'>otherwise</a>, <a href='#SkImage_MakeFromEncoded_subset'>subset</a> <a href='#SkImage_MakeFromEncoded_subset'>must</a> <a href='#SkImage_MakeFromEncoded_subset'>be</a> <a href='#SkImage_MakeFromEncoded_subset'>contained</a> <a href='#SkImage_MakeFromEncoded_subset'>by</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>bounds</a>.

<a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>returned</a> <a href='SkImage_Reference#SkImage'>if</a> <a href='SkImage_Reference#SkImage'>format</a> <a href='SkImage_Reference#SkImage'>of</a> <a href='SkImage_Reference#SkImage'>the</a> <a href='#SkImage_MakeFromEncoded_encoded'>encoded</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>is</a> <a href='undocumented#Data'>recognized</a> <a href='undocumented#Data'>and</a> <a href='undocumented#Data'>supported</a>.
Recognized formats vary by platform.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromEncoded_encoded'><code><strong>encoded</strong></code></a></td>
    <td><a href='undocumented#Data'>data</a> <a href='undocumented#Data'>of</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>to</a> <a href='SkImage_Reference#SkImage'>decode</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromEncoded_subset'><code><strong>subset</strong></code></a></td>
    <td>bounds of returned <a href='SkImage_Reference#SkImage'>SkImage</a>; <a href='SkImage_Reference#SkImage'>may</a> <a href='SkImage_Reference#SkImage'>be</a> <a href='SkImage_Reference#SkImage'>nullptr</a></td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>

### Example

<div><fiddle-embed name="894f732ed6409b1f392bc5481421d0e9"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromGenerator'>MakeFromGenerator</a>

<a name='SkImage_TextureReleaseProc'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    typedef void (*<a href='#SkImage_TextureReleaseProc'>TextureReleaseProc</a>)(<a href='#SkImage_ReleaseContext'>ReleaseContext</a> <a href='#SkImage_ReleaseContext'>releaseContext</a>);
</pre>

User function called when supplied <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>may</a> <a href='undocumented#Texture'>be</a> <a href='undocumented#Texture'>deleted</a>.

### See Also

<a href='#SkImage_MakeFromTexture'>MakeFromTexture</a>

<a name='SkImage_MakeFromTexture'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_MakeFromTexture'>MakeFromTexture</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>, <a href='undocumented#GrContext'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& <a href='undocumented#GrBackendTexture'>backendTexture</a>,
                                      <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>origin</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkImage_colorType'>colorType</a>,
                                      <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImage_alphaType'>alphaType</a>, <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&<a href='undocumented#SkColorSpace'>gt</a>; <a href='#SkImage_colorSpace'>colorSpace</a>)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>from</a>  <a href='undocumented#GPU_Texture'>GPU texture</a> <a href='SkImage_Reference#SkImage'>associated</a> <a href='SkImage_Reference#SkImage'>with</a> <a href='#SkImage_MakeFromTexture_context'>context</a>. <a href='#SkImage_MakeFromTexture_context'>Caller</a> <a href='#SkImage_MakeFromTexture_context'>is</a> <a href='#SkImage_MakeFromTexture_context'>responsible</a> <a href='#SkImage_MakeFromTexture_context'>for</a>
managing the lifetime of  <a href='undocumented#GPU_Texture'>GPU texture</a>.

<a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>returned</a> <a href='SkImage_Reference#SkImage'>if</a> <a href='SkImage_Reference#SkImage'>format</a> <a href='SkImage_Reference#SkImage'>of</a> <a href='#SkImage_MakeFromTexture_backendTexture'>backendTexture</a> <a href='#SkImage_MakeFromTexture_backendTexture'>is</a> <a href='#SkImage_MakeFromTexture_backendTexture'>recognized</a> <a href='#SkImage_MakeFromTexture_backendTexture'>and</a> <a href='#SkImage_MakeFromTexture_backendTexture'>supported</a>.
Recognized formats vary by GPU back-end.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromTexture_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromTexture_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td><a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>residing</a> <a href='undocumented#Texture'>on</a> <a href='undocumented#Texture'>GPU</a></td>
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

created <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>

### Example

<div><fiddle-embed name="94e9296c53bad074bf2a48ff885dac13" gpu="true"><div>A back-end <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>has</a> <a href='undocumented#Texture'>been</a> <a href='undocumented#Texture'>created</a> <a href='undocumented#Texture'>and</a> <a href='undocumented#Texture'>uploaded</a> <a href='undocumented#Texture'>to</a> <a href='undocumented#Texture'>the</a> <a href='undocumented#Texture'>GPU</a> <a href='undocumented#Texture'>outside</a> <a href='undocumented#Texture'>of</a> <a href='undocumented#Texture'>this</a> <a href='undocumented#Texture'>example</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromAdoptedTexture'>MakeFromAdoptedTexture</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_MakeFromBackendTexture'>MakeFromBackendTexture</a>

<a name='SkImage_MakeFromTexture_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_MakeFromTexture'>MakeFromTexture</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>, <a href='undocumented#GrContext'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& <a href='undocumented#GrBackendTexture'>backendTexture</a>,
                                      <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>origin</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkImage_colorType'>colorType</a>,
                                      <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImage_alphaType'>alphaType</a>, <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&<a href='undocumented#SkColorSpace'>gt</a>; <a href='#SkImage_colorSpace'>colorSpace</a>,
                                      <a href='#SkImage_TextureReleaseProc'>TextureReleaseProc</a> <a href='#SkImage_TextureReleaseProc'>textureReleaseProc</a>,
                                      <a href='#SkImage_ReleaseContext'>ReleaseContext</a> <a href='#SkImage_ReleaseContext'>releaseContext</a>)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>from</a>  <a href='undocumented#GPU_Texture'>GPU texture</a> <a href='SkImage_Reference#SkImage'>associated</a> <a href='SkImage_Reference#SkImage'>with</a> <a href='#SkImage_MakeFromTexture_2_context'>context</a>.  <a href='undocumented#GPU_Texture'>GPU texture</a> <a href='#SkImage_MakeFromTexture_2_context'>must</a> <a href='#SkImage_MakeFromTexture_2_context'>stay</a>
valid and unchanged until <a href='#SkImage_MakeFromTexture_2_textureReleaseProc'>textureReleaseProc</a> <a href='#SkImage_MakeFromTexture_2_textureReleaseProc'>is</a> <a href='#SkImage_MakeFromTexture_2_textureReleaseProc'>called</a>. <a href='#SkImage_MakeFromTexture_2_textureReleaseProc'>textureReleaseProc</a> <a href='#SkImage_MakeFromTexture_2_textureReleaseProc'>is</a>
passed <a href='#SkImage_MakeFromTexture_2_releaseContext'>releaseContext</a> <a href='#SkImage_MakeFromTexture_2_releaseContext'>when</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>deleted</a> <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>no</a> <a href='SkImage_Reference#SkImage'>longer</a> <a href='SkImage_Reference#SkImage'>refers</a> <a href='SkImage_Reference#SkImage'>to</a> <a href='undocumented#Texture'>texture</a>.

<a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>returned</a> <a href='SkImage_Reference#SkImage'>if</a> <a href='SkImage_Reference#SkImage'>format</a> <a href='SkImage_Reference#SkImage'>of</a> <a href='#SkImage_MakeFromTexture_2_backendTexture'>backendTexture</a> <a href='#SkImage_MakeFromTexture_2_backendTexture'>is</a> <a href='#SkImage_MakeFromTexture_2_backendTexture'>recognized</a> <a href='#SkImage_MakeFromTexture_2_backendTexture'>and</a> <a href='#SkImage_MakeFromTexture_2_backendTexture'>supported</a>.
Recognized formats vary by GPU back-end.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromTexture_2_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromTexture_2_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td><a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>residing</a> <a href='undocumented#Texture'>on</a> <a href='undocumented#Texture'>GPU</a></td>
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
    <td>function called when <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>can</a> <a href='undocumented#Texture'>be</a> <a href='undocumented#Texture'>released</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromTexture_2_releaseContext'><code><strong>releaseContext</strong></code></a></td>
    <td>state passed to <a href='#SkImage_MakeFromTexture_2_textureReleaseProc'>textureReleaseProc</a></td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>

### Example

<div><fiddle-embed name="f40e1ebba6b067714062b81877b22fa1" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromAdoptedTexture'>MakeFromAdoptedTexture</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_MakeFromBackendTexture'>MakeFromBackendTexture</a>

<a name='SkImage_MakeCrossContextFromEncoded'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_MakeCrossContextFromEncoded'>MakeCrossContextFromEncoded</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>, <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkData'>SkData</a>&<a href='undocumented#SkData'>gt</a>; <a href='undocumented#Data'>data</a>,
                                                  <a href='undocumented#Data'>bool</a> <a href='undocumented#Data'>buildMips</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='undocumented#SkColorSpace'>dstColorSpace</a>,
                                                  <a href='undocumented#SkColorSpace'>bool</a> <a href='undocumented#SkColorSpace'>limitToMaxTextureSize</a> = <a href='undocumented#SkColorSpace'>false</a>)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>from</a> <a href='SkImage_Reference#SkImage'>encoded</a> <a href='#SkImage_MakeCrossContextFromEncoded_data'>data</a>. <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>uploaded</a> <a href='SkImage_Reference#SkImage'>to</a> <a href='SkImage_Reference#SkImage'>GPU</a> <a href='SkImage_Reference#SkImage'>back-end</a> <a href='SkImage_Reference#SkImage'>using</a> <a href='#SkImage_MakeCrossContextFromEncoded_context'>context</a>.

Created <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>available</a> <a href='SkImage_Reference#SkImage'>to</a> <a href='SkImage_Reference#SkImage'>other</a> <a href='SkImage_Reference#SkImage'>GPU</a> <a href='SkImage_Reference#SkImage'>contexts</a>, <a href='SkImage_Reference#SkImage'>and</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>available</a> <a href='SkImage_Reference#SkImage'>across</a> <a href='SkImage_Reference#SkImage'>thread</a>
boundaries. All contexts must be in the same   <a href='undocumented#GPU_Share_Group'>GPU share group</a>, or otherwise
share resources.

When <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>no</a> <a href='SkImage_Reference#SkImage'>longer</a> <a href='SkImage_Reference#SkImage'>referenced</a>, <a href='#SkImage_MakeCrossContextFromEncoded_context'>context</a> <a href='#SkImage_MakeCrossContextFromEncoded_context'>releases</a> <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>memory</a>
asynchronously.

<a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='undocumented#GrBackendTexture'>decoded</a> <a href='undocumented#GrBackendTexture'>from</a> <a href='#SkImage_MakeCrossContextFromEncoded_data'>data</a> <a href='#SkImage_MakeCrossContextFromEncoded_data'>is</a> <a href='#SkImage_MakeCrossContextFromEncoded_data'>uploaded</a> <a href='#SkImage_MakeCrossContextFromEncoded_data'>to</a> <a href='#SkImage_MakeCrossContextFromEncoded_data'>match</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>created</a> <a href='SkSurface_Reference#SkSurface'>with</a>
<a href='#SkImage_MakeCrossContextFromEncoded_dstColorSpace'>dstColorSpace</a>. <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>of</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>determined</a> <a href='SkImage_Reference#SkImage'>by</a> <a href='SkImage_Reference#SkImage'>encoded</a> <a href='#SkImage_MakeCrossContextFromEncoded_data'>data</a>.

<a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>returned</a> <a href='SkImage_Reference#SkImage'>if</a> <a href='SkImage_Reference#SkImage'>format</a> <a href='SkImage_Reference#SkImage'>of</a> <a href='#SkImage_MakeCrossContextFromEncoded_data'>data</a> <a href='#SkImage_MakeCrossContextFromEncoded_data'>is</a> <a href='#SkImage_MakeCrossContextFromEncoded_data'>recognized</a> <a href='#SkImage_MakeCrossContextFromEncoded_data'>and</a> <a href='#SkImage_MakeCrossContextFromEncoded_data'>supported</a>, <a href='#SkImage_MakeCrossContextFromEncoded_data'>and</a> <a href='#SkImage_MakeCrossContextFromEncoded_data'>if</a> <a href='#SkImage_MakeCrossContextFromEncoded_context'>context</a>
supports moving resources. Recognized formats vary by platform and GPU back-end.

<a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>returned</a> <a href='SkImage_Reference#SkImage'>using</a> <a href='#SkImage_MakeFromEncoded'>MakeFromEncoded</a>() <a href='#SkImage_MakeFromEncoded'>if</a> <a href='#SkImage_MakeCrossContextFromEncoded_context'>context</a> <a href='#SkImage_MakeCrossContextFromEncoded_context'>is</a> <a href='#SkImage_MakeCrossContextFromEncoded_context'>nullptr</a> <a href='#SkImage_MakeCrossContextFromEncoded_context'>or</a> <a href='#SkImage_MakeCrossContextFromEncoded_context'>does</a> <a href='#SkImage_MakeCrossContextFromEncoded_context'>not</a> <a href='#SkImage_MakeCrossContextFromEncoded_context'>support</a>
moving resources between contexts.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeCrossContextFromEncoded_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeCrossContextFromEncoded_data'><code><strong>data</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>to</a> <a href='SkImage_Reference#SkImage'>decode</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeCrossContextFromEncoded_buildMips'><code><strong>buildMips</strong></code></a></td>
    <td>create <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>as</a>  <a href='undocumented#Mip_Map'>mip map</a> <a href='SkImage_Reference#SkImage'>if</a> <a href='SkImage_Reference#SkImage'>true</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeCrossContextFromEncoded_dstColorSpace'><code><strong>dstColorSpace</strong></code></a></td>
    <td>range of colors of matching <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>on</a> <a href='SkSurface_Reference#SkSurface'>GPU</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeCrossContextFromEncoded_limitToMaxTextureSize'><code><strong>limitToMaxTextureSize</strong></code></a></td>
    <td>downscale <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>to</a> <a href='SkImage_Reference#Image'>GPU</a> <a href='SkImage_Reference#Image'>maximum</a> <a href='undocumented#Texture'>texture</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Size'>if</a> <a href='undocumented#Size'>necessary</a></td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>

### Example

<div><fiddle-embed name="069c7b116479e3ca46f953f07dcbdd36"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeCrossContextFromPixmap'>MakeCrossContextFromPixmap</a>

<a name='SkImage_MakeCrossContextFromPixmap'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_MakeCrossContextFromPixmap'>MakeCrossContextFromPixmap</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>, <a href='undocumented#GrContext'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#Pixmap'>pixmap</a>,
                                                 <a href='SkPixmap_Reference#Pixmap'>bool</a> <a href='SkPixmap_Reference#Pixmap'>buildMips</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='undocumented#SkColorSpace'>dstColorSpace</a>,
                                                 <a href='undocumented#SkColorSpace'>bool</a> <a href='undocumented#SkColorSpace'>limitToMaxTextureSize</a> = <a href='undocumented#SkColorSpace'>false</a>)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>from</a> <a href='#SkImage_MakeCrossContextFromPixmap_pixmap'>pixmap</a>. <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>uploaded</a> <a href='SkImage_Reference#SkImage'>to</a> <a href='SkImage_Reference#SkImage'>GPU</a> <a href='SkImage_Reference#SkImage'>back-end</a> <a href='SkImage_Reference#SkImage'>using</a> <a href='#SkImage_MakeCrossContextFromPixmap_context'>context</a>.

Created <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>available</a> <a href='SkImage_Reference#SkImage'>to</a> <a href='SkImage_Reference#SkImage'>other</a> <a href='SkImage_Reference#SkImage'>GPU</a> <a href='SkImage_Reference#SkImage'>contexts</a>, <a href='SkImage_Reference#SkImage'>and</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>available</a> <a href='SkImage_Reference#SkImage'>across</a> <a href='SkImage_Reference#SkImage'>thread</a>
boundaries. All contexts must be in the same   <a href='undocumented#GPU_Share_Group'>GPU share group</a>, or otherwise
share resources.

When <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>no</a> <a href='SkImage_Reference#SkImage'>longer</a> <a href='SkImage_Reference#SkImage'>referenced</a>, <a href='#SkImage_MakeCrossContextFromPixmap_context'>context</a> <a href='#SkImage_MakeCrossContextFromPixmap_context'>releases</a> <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>memory</a>
asynchronously.

<a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='undocumented#GrBackendTexture'>created</a> <a href='undocumented#GrBackendTexture'>from</a> <a href='#SkImage_MakeCrossContextFromPixmap_pixmap'>pixmap</a> <a href='#SkImage_MakeCrossContextFromPixmap_pixmap'>is</a> <a href='#SkImage_MakeCrossContextFromPixmap_pixmap'>uploaded</a> <a href='#SkImage_MakeCrossContextFromPixmap_pixmap'>to</a> <a href='#SkImage_MakeCrossContextFromPixmap_pixmap'>match</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>created</a> <a href='SkSurface_Reference#SkSurface'>with</a>
<a href='#SkImage_MakeCrossContextFromPixmap_dstColorSpace'>dstColorSpace</a>. <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>of</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>determined</a> <a href='SkImage_Reference#SkImage'>by</a> <a href='#SkImage_MakeCrossContextFromPixmap_pixmap'>pixmap</a>.<a href='#SkPixmap_colorSpace'>colorSpace</a>().

<a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>returned</a> <a href='SkImage_Reference#SkImage'>referring</a> <a href='SkImage_Reference#SkImage'>to</a> <a href='SkImage_Reference#SkImage'>GPU</a> <a href='SkImage_Reference#SkImage'>back-end</a> <a href='SkImage_Reference#SkImage'>if</a> <a href='#SkImage_MakeCrossContextFromPixmap_context'>context</a> <a href='#SkImage_MakeCrossContextFromPixmap_context'>is</a> <a href='#SkImage_MakeCrossContextFromPixmap_context'>not</a> <a href='#SkImage_MakeCrossContextFromPixmap_context'>nullptr</a>,
format of <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>is</a> <a href='undocumented#Data'>recognized</a> <a href='undocumented#Data'>and</a> <a href='undocumented#Data'>supported</a>, <a href='undocumented#Data'>and</a> <a href='undocumented#Data'>if</a> <a href='#SkImage_MakeCrossContextFromPixmap_context'>context</a> <a href='#SkImage_MakeCrossContextFromPixmap_context'>supports</a> <a href='#SkImage_MakeCrossContextFromPixmap_context'>moving</a>
resources between contexts. Otherwise, <a href='#SkImage_MakeCrossContextFromPixmap_pixmap'>pixmap</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>is</a> <a href='undocumented#Data'>copied</a> <a href='undocumented#Data'>and</a> <a href='SkImage_Reference#SkImage'>SkImage</a>
as returned in raster format if possible; nullptr may be returned.
Recognized GPU formats vary by platform and GPU back-end.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeCrossContextFromPixmap_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeCrossContextFromPixmap_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a>, <a href='undocumented#Pixel'>and</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>bytes</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeCrossContextFromPixmap_buildMips'><code><strong>buildMips</strong></code></a></td>
    <td>create <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>as</a>  <a href='undocumented#Mip_Map'>mip map</a> <a href='SkImage_Reference#SkImage'>if</a> <a href='SkImage_Reference#SkImage'>true</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeCrossContextFromPixmap_dstColorSpace'><code><strong>dstColorSpace</strong></code></a></td>
    <td>range of colors of matching <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>on</a> <a href='SkSurface_Reference#SkSurface'>GPU</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeCrossContextFromPixmap_limitToMaxTextureSize'><code><strong>limitToMaxTextureSize</strong></code></a></td>
    <td>downscale <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>to</a> <a href='SkImage_Reference#Image'>GPU</a> <a href='SkImage_Reference#Image'>maximum</a> <a href='undocumented#Texture'>texture</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Size'>if</a> <a href='undocumented#Size'>necessary</a></td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>

### Example

<div><fiddle-embed name="45bca8747b8f49b5be34b520897ef048"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeCrossContextFromEncoded'>MakeCrossContextFromEncoded</a>

<a name='SkImage_MakeFromAdoptedTexture'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_MakeFromAdoptedTexture'>MakeFromAdoptedTexture</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>,
                                             <a href='undocumented#GrContext'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& <a href='undocumented#GrBackendTexture'>backendTexture</a>,
                                             <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>surfaceOrigin</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkImage_colorType'>colorType</a>,
                                             <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImage_alphaType'>alphaType</a> = <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
                                             <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&<a href='undocumented#SkColorSpace'>gt</a>; <a href='#SkImage_colorSpace'>colorSpace</a> = <a href='#SkImage_colorSpace'>nullptr</a>)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>from</a> <a href='#SkImage_MakeFromAdoptedTexture_backendTexture'>backendTexture</a> <a href='#SkImage_MakeFromAdoptedTexture_backendTexture'>associated</a> <a href='#SkImage_MakeFromAdoptedTexture_backendTexture'>with</a> <a href='#SkImage_MakeFromAdoptedTexture_context'>context</a>. <a href='#SkImage_MakeFromAdoptedTexture_backendTexture'>backendTexture</a> <a href='#SkImage_MakeFromAdoptedTexture_backendTexture'>and</a>
returned <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>are</a> <a href='SkImage_Reference#SkImage'>managed</a> <a href='SkImage_Reference#SkImage'>internally</a>, <a href='SkImage_Reference#SkImage'>and</a> <a href='SkImage_Reference#SkImage'>are</a> <a href='SkImage_Reference#SkImage'>released</a> <a href='SkImage_Reference#SkImage'>when</a> <a href='SkImage_Reference#SkImage'>no</a> <a href='SkImage_Reference#SkImage'>longer</a> <a href='SkImage_Reference#SkImage'>needed</a>.

<a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>returned</a> <a href='SkImage_Reference#SkImage'>if</a> <a href='SkImage_Reference#SkImage'>format</a> <a href='SkImage_Reference#SkImage'>of</a> <a href='#SkImage_MakeFromAdoptedTexture_backendTexture'>backendTexture</a> <a href='#SkImage_MakeFromAdoptedTexture_backendTexture'>is</a> <a href='#SkImage_MakeFromAdoptedTexture_backendTexture'>recognized</a> <a href='#SkImage_MakeFromAdoptedTexture_backendTexture'>and</a> <a href='#SkImage_MakeFromAdoptedTexture_backendTexture'>supported</a>.
Recognized formats vary by GPU back-end.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromAdoptedTexture_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromAdoptedTexture_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td><a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>residing</a> <a href='undocumented#Texture'>on</a> <a href='undocumented#Texture'>GPU</a></td>
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

created <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>

### Example

<div><fiddle-embed name="b034517e39394b7543f06ec885e36d7d" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromTexture'>MakeFromTexture</a> <a href='#SkImage_MakeFromYUVTexturesCopy'>MakeFromYUVTexturesCopy</a>

<a name='SkImage_MakeFromYUVATexturesCopy'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_MakeFromYUVATexturesCopy'>MakeFromYUVATexturesCopy</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>, <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> <a href='SkImageInfo_Reference#SkYUVColorSpace'>yuvColorSpace</a>,
                                               <a href='SkImageInfo_Reference#SkYUVColorSpace'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='undocumented#GrBackendTexture'>yuvaTextures</a>[],
                                               <a href='undocumented#GrBackendTexture'>const</a> <a href='undocumented#SkYUVAIndex'>SkYUVAIndex</a> <a href='undocumented#SkYUVAIndex'>yuvaIndices</a>[4], <a href='undocumented#SkISize'>SkISize</a> <a href='undocumented#SkISize'>imageSize</a>,
                                               <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>imageOrigin</a>,
                                               <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&<a href='undocumented#SkColorSpace'>gt</a>; <a href='undocumented#SkColorSpace'>imageColorSpace</a> = <a href='undocumented#SkColorSpace'>nullptr</a>)
</pre>

Creates an <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>by</a> <a href='SkImage_Reference#SkImage'>flattening</a> <a href='SkImage_Reference#SkImage'>the</a> <a href='SkImage_Reference#SkImage'>specified</a> <a href='SkImage_Reference#SkImage'>YUVA</a> <a href='SkImage_Reference#SkImage'>planes</a> <a href='SkImage_Reference#SkImage'>into</a> <a href='SkImage_Reference#SkImage'>a</a> <a href='SkImage_Reference#SkImage'>single</a>, <a href='SkImage_Reference#SkImage'>interleaved</a> <a href='SkImage_Reference#SkImage'>RGBA</a>
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
    <td>array indicating which <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>in</a> <a href='#SkImage_MakeFromYUVATexturesCopy_yuvaTextures'>yuvaTextures</a>, <a href='#SkImage_MakeFromYUVATexturesCopy_yuvaTextures'>and</a> <a href='#SkImage_MakeFromYUVATexturesCopy_yuvaTextures'>channel</a></td>
  </tr>
</table>

in that <a href='undocumented#Texture'>texture</a>, <a href='undocumented#Texture'>maps</a> <a href='undocumented#Texture'>to</a> <a href='undocumented#Texture'>each</a> <a href='undocumented#Texture'>component</a> <a href='undocumented#Texture'>of</a> <a href='undocumented#Texture'>YUVA</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopy_imageSize'><code><strong>imageSize</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Size'>the</a> <a href='undocumented#Size'>resulting</a> <a href='SkImage_Reference#Image'>image</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopy_imageOrigin'><code><strong>imageOrigin</strong></code></a></td>
    <td>origin of the resulting <a href='SkImage_Reference#Image'>image</a>. <a href='SkImage_Reference#Image'>One</a> <a href='SkImage_Reference#Image'>of</a>: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft_GrSurfaceOrigin</a>,</td>
  </tr>
</table>

<a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopy_imageColorSpace'><code><strong>imageColorSpace</strong></code></a></td>
    <td>range of colors of the resulting <a href='SkImage_Reference#Image'>image</a>; <a href='SkImage_Reference#Image'>may</a> <a href='SkImage_Reference#Image'>be</a> <a href='SkImage_Reference#Image'>nullptr</a></td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>

### See Also

<a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend'>MakeFromYUVATexturesCopyWithExternalBackend</a> <a href='#SkImage_MakeFromYUVATextures'>MakeFromYUVATextures</a>

<a name='SkImage_MakeFromYUVATextures'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_MakeFromYUVATextures'>MakeFromYUVATextures</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>, <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> <a href='SkImageInfo_Reference#SkYUVColorSpace'>yuvColorSpace</a>,
                                           <a href='SkImageInfo_Reference#SkYUVColorSpace'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='undocumented#GrBackendTexture'>yuvaTextures</a>[],
                                           <a href='undocumented#GrBackendTexture'>const</a> <a href='undocumented#SkYUVAIndex'>SkYUVAIndex</a> <a href='undocumented#SkYUVAIndex'>yuvaIndices</a>[4], <a href='undocumented#SkISize'>SkISize</a> <a href='undocumented#SkISize'>imageSize</a>,
                                           <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>imageOrigin</a>,
                                           <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&<a href='undocumented#SkColorSpace'>gt</a>; <a href='undocumented#SkColorSpace'>imageColorSpace</a> = <a href='undocumented#SkColorSpace'>nullptr</a>) ;
</pre>

Creates an <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>by</a> <a href='SkImage_Reference#SkImage'>storing</a> <a href='SkImage_Reference#SkImage'>the</a> <a href='SkImage_Reference#SkImage'>specified</a> <a href='SkImage_Reference#SkImage'>YUVA</a> <a href='SkImage_Reference#SkImage'>planes</a> <a href='SkImage_Reference#SkImage'>into</a> <a href='SkImage_Reference#SkImage'>an</a> <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>to</a> <a href='SkImage_Reference#Image'>be</a> <a href='SkImage_Reference#Image'>rendered</a>
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
    <td>array indicating which <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>in</a> <a href='#SkImage_MakeFromYUVATextures_yuvaTextures'>yuvaTextures</a>, <a href='#SkImage_MakeFromYUVATextures_yuvaTextures'>and</a> <a href='#SkImage_MakeFromYUVATextures_yuvaTextures'>channel</a></td>
  </tr>
</table>

in that <a href='undocumented#Texture'>texture</a>, <a href='undocumented#Texture'>maps</a> <a href='undocumented#Texture'>to</a> <a href='undocumented#Texture'>each</a> <a href='undocumented#Texture'>component</a> <a href='undocumented#Texture'>of</a> <a href='undocumented#Texture'>YUVA</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVATextures_imageSize'><code><strong>imageSize</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Size'>the</a> <a href='undocumented#Size'>resulting</a> <a href='SkImage_Reference#Image'>image</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVATextures_imageOrigin'><code><strong>imageOrigin</strong></code></a></td>
    <td>origin of the resulting <a href='SkImage_Reference#Image'>image</a>. <a href='SkImage_Reference#Image'>One</a> <a href='SkImage_Reference#Image'>of</a>: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft_GrSurfaceOrigin</a>,</td>
  </tr>
</table>

<a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVATextures_imageColorSpace'><code><strong>imageColorSpace</strong></code></a></td>
    <td>range of colors of the resulting <a href='SkImage_Reference#Image'>image</a>; <a href='SkImage_Reference#Image'>may</a> <a href='SkImage_Reference#Image'>be</a> <a href='SkImage_Reference#Image'>nullptr</a></td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>

### See Also

<a href='#SkImage_MakeFromYUVATexturesCopy'>MakeFromYUVATexturesCopy</a> <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend'>MakeFromYUVATexturesCopyWithExternalBackend</a>

<a name='SkImage_MakeFromYUVATexturesCopyWithExternalBackend'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend'>MakeFromYUVATexturesCopyWithExternalBackend</a>(
            <a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>,
                        <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> <a href='SkImageInfo_Reference#SkYUVColorSpace'>yuvColorSpace</a>, <a href='SkImageInfo_Reference#SkYUVColorSpace'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='undocumented#GrBackendTexture'>yuvaTextures</a>[],
                        <a href='undocumented#GrBackendTexture'>const</a> <a href='undocumented#SkYUVAIndex'>SkYUVAIndex</a> <a href='undocumented#SkYUVAIndex'>yuvaIndices</a>[4], <a href='undocumented#SkISize'>SkISize</a> <a href='undocumented#SkISize'>imageSize</a>,
                        <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>imageOrigin</a>, <a href='undocumented#GrSurfaceOrigin'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& <a href='undocumented#GrBackendTexture'>backendTexture</a>,
                        <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&<a href='undocumented#SkColorSpace'>gt</a>; <a href='undocumented#SkColorSpace'>imageColorSpace</a> = <a href='undocumented#SkColorSpace'>nullptr</a>)
</pre>

Creates an <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>by</a> <a href='SkImage_Reference#SkImage'>flattening</a> <a href='SkImage_Reference#SkImage'>the</a> <a href='SkImage_Reference#SkImage'>specified</a> <a href='SkImage_Reference#SkImage'>YUVA</a> <a href='SkImage_Reference#SkImage'>planes</a> <a href='SkImage_Reference#SkImage'>into</a> <a href='SkImage_Reference#SkImage'>a</a> <a href='SkImage_Reference#SkImage'>single</a>, <a href='SkImage_Reference#SkImage'>interleaved</a> <a href='SkImage_Reference#SkImage'>RGBA</a>
<a href='SkImage_Reference#Image'>image</a>. '<a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend_backendTexture'>backendTexture</a>' <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend_backendTexture'>is</a> <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend_backendTexture'>used</a> <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend_backendTexture'>to</a> <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend_backendTexture'>store</a> <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend_backendTexture'>the</a> <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend_backendTexture'>result</a> <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend_backendTexture'>of</a> <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend_backendTexture'>the</a> <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend_backendTexture'>flattening</a>.

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
    <td>array indicating which <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>in</a> <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend_yuvaTextures'>yuvaTextures</a>, <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend_yuvaTextures'>and</a> <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend_yuvaTextures'>channel</a></td>
  </tr>
</table>

in that <a href='undocumented#Texture'>texture</a>, <a href='undocumented#Texture'>maps</a> <a href='undocumented#Texture'>to</a> <a href='undocumented#Texture'>each</a> <a href='undocumented#Texture'>component</a> <a href='undocumented#Texture'>of</a> <a href='undocumented#Texture'>YUVA</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopyWithExternalBackend_imageSize'><code><strong>imageSize</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Size'>the</a> <a href='undocumented#Size'>resulting</a> <a href='SkImage_Reference#Image'>image</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopyWithExternalBackend_imageOrigin'><code><strong>imageOrigin</strong></code></a></td>
    <td>origin of the resulting <a href='SkImage_Reference#Image'>image</a>. <a href='SkImage_Reference#Image'>One</a> <a href='SkImage_Reference#Image'>of</a>: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft_GrSurfaceOrigin</a>,</td>
  </tr>
</table>

<a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopyWithExternalBackend_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td>the resource that stores the final pixels</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVATexturesCopyWithExternalBackend_imageColorSpace'><code><strong>imageColorSpace</strong></code></a></td>
    <td>range of colors of the resulting <a href='SkImage_Reference#Image'>image</a>; <a href='SkImage_Reference#Image'>may</a> <a href='SkImage_Reference#Image'>be</a> <a href='SkImage_Reference#Image'>nullptr</a></td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>

### See Also

<a href='#SkImage_MakeFromYUVATexturesCopy'>MakeFromYUVATexturesCopy</a> <a href='#SkImage_MakeFromYUVATextures'>MakeFromYUVATextures</a>

<a name='SkImage_MakeFromYUVTexturesCopy'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_MakeFromYUVTexturesCopy'>MakeFromYUVTexturesCopy</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>, <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> <a href='SkImageInfo_Reference#SkYUVColorSpace'>yuvColorSpace</a>,
                                              <a href='SkImageInfo_Reference#SkYUVColorSpace'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='undocumented#GrBackendTexture'>yuvTextures</a>[3],
                                              <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>imageOrigin</a>,
                                              <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&<a href='undocumented#SkColorSpace'>gt</a>; <a href='undocumented#SkColorSpace'>imageColorSpace</a> = <a href='undocumented#SkColorSpace'>nullptr</a>)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>from</a> <a href='SkImage_Reference#SkImage'>copy</a> <a href='SkImage_Reference#SkImage'>of</a> <a href='#SkImage_MakeFromYUVTexturesCopy_yuvTextures'>yuvTextures</a>, <a href='#SkImage_MakeFromYUVTexturesCopy_yuvTextures'>an</a> <a href='#SkImage_MakeFromYUVTexturesCopy_yuvTextures'>array</a> <a href='#SkImage_MakeFromYUVTexturesCopy_yuvTextures'>of</a> <a href='#SkImage_MakeFromYUVTexturesCopy_yuvTextures'>textures</a> <a href='#SkImage_MakeFromYUVTexturesCopy_yuvTextures'>on</a> <a href='#SkImage_MakeFromYUVTexturesCopy_yuvTextures'>GPU</a>.
<a href='#SkImage_MakeFromYUVTexturesCopy_yuvTextures'>yuvTextures</a> <a href='#SkImage_MakeFromYUVTexturesCopy_yuvTextures'>contain</a> <a href='#SkImage_MakeFromYUVTexturesCopy_yuvTextures'>pixels</a> <a href='#SkImage_MakeFromYUVTexturesCopy_yuvTextures'>for</a>  <a href='undocumented#YUV_Planes'>YUV planes</a> <a href='#SkImage_MakeFromYUVTexturesCopy_yuvTextures'>of</a> <a href='SkImage_Reference#SkImage'>SkImage</a>. <a href='SkImage_Reference#SkImage'>Returned</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>has</a> <a href='SkImage_Reference#SkImage'>the</a> <a href='SkImage_Reference#SkImage'>dimensions</a>
<a href='#SkImage_MakeFromYUVTexturesCopy_yuvTextures'>yuvTextures</a>[0]. <a href='#SkImage_MakeFromYUVTexturesCopy_yuvColorSpace'>yuvColorSpace</a> <a href='#SkImage_MakeFromYUVTexturesCopy_yuvColorSpace'>describes</a> <a href='#SkImage_MakeFromYUVTexturesCopy_yuvColorSpace'>how</a> <a href='#SkImage_MakeFromYUVTexturesCopy_yuvColorSpace'>YUV</a> <a href='#SkImage_MakeFromYUVTexturesCopy_yuvColorSpace'>colors</a> <a href='#SkImage_MakeFromYUVTexturesCopy_yuvColorSpace'>convert</a> <a href='#SkImage_MakeFromYUVTexturesCopy_yuvColorSpace'>to</a> <a href='#SkImage_MakeFromYUVTexturesCopy_yuvColorSpace'>RGB</a> <a href='#SkImage_MakeFromYUVTexturesCopy_yuvColorSpace'>colors</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVTexturesCopy_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVTexturesCopy_yuvColorSpace'><code><strong>yuvColorSpace</strong></code></a></td>
    <td>one of: <a href='SkImageInfo_Reference#kJPEG_SkYUVColorSpace'>kJPEG_SkYUVColorSpace</a>, <a href='SkImageInfo_Reference#kRec601_SkYUVColorSpace'>kRec601_SkYUVColorSpace</a>,</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kRec709_SkYUVColorSpace'>kRec709_SkYUVColorSpace</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVTexturesCopy_yuvTextures'><code><strong>yuvTextures</strong></code></a></td>
    <td>array of YUV textures on GPU</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVTexturesCopy_imageOrigin'><code><strong>imageOrigin</strong></code></a></td>
    <td>one of: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft_GrSurfaceOrigin</a>, <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVTexturesCopy_imageColorSpace'><code><strong>imageColorSpace</strong></code></a></td>
    <td>range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>

### See Also

<a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend'>MakeFromYUVTexturesCopyWithExternalBackend</a> <a href='#SkImage_MakeFromNV12TexturesCopy'>MakeFromNV12TexturesCopy</a> <a href='#SkImage_MakeFromYUVATexturesCopy'>MakeFromYUVATexturesCopy</a>

<a name='SkImage_MakeFromYUVTexturesCopyWithExternalBackend'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend'>MakeFromYUVTexturesCopyWithExternalBackend</a>(
        <a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>,
                             <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> <a href='SkImageInfo_Reference#SkYUVColorSpace'>yuvColorSpace</a>, <a href='SkImageInfo_Reference#SkYUVColorSpace'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='undocumented#GrBackendTexture'>yuvTextures</a>[3],
                             <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>imageOrigin</a>, <a href='undocumented#GrSurfaceOrigin'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& <a href='undocumented#GrBackendTexture'>backendTexture</a>,
                             <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&<a href='undocumented#SkColorSpace'>gt</a>; <a href='undocumented#SkColorSpace'>imageColorSpace</a> = <a href='undocumented#SkColorSpace'>nullptr</a>) ;
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>from</a> <a href='SkImage_Reference#SkImage'>copy</a> <a href='SkImage_Reference#SkImage'>of</a> <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvTextures'>yuvTextures</a>, <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvTextures'>an</a> <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvTextures'>array</a> <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvTextures'>of</a> <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvTextures'>textures</a> <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvTextures'>on</a> <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvTextures'>GPU</a>.
<a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvTextures'>yuvTextures</a> <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvTextures'>contain</a> <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvTextures'>pixels</a> <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvTextures'>for</a>  <a href='undocumented#YUV_Planes'>YUV planes</a> <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvTextures'>of</a> <a href='SkImage_Reference#SkImage'>SkImage</a>. <a href='SkImage_Reference#SkImage'>Returned</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>has</a> <a href='SkImage_Reference#SkImage'>the</a> <a href='SkImage_Reference#SkImage'>dimensions</a>
<a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvTextures'>yuvTextures</a>[0] <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvTextures'>and</a> <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvTextures'>stores</a> <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvTextures'>pixels</a> <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvTextures'>in</a> <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend_backendTexture'>backendTexture</a>. <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvColorSpace'>yuvColorSpace</a> <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvColorSpace'>describes</a> <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvColorSpace'>how</a> <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvColorSpace'>YUV</a> <a href='#SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvColorSpace'>colors</a>
convert to RGB colors.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVTexturesCopyWithExternalBackend_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvColorSpace'><code><strong>yuvColorSpace</strong></code></a></td>
    <td>one of: <a href='SkImageInfo_Reference#kJPEG_SkYUVColorSpace'>kJPEG_SkYUVColorSpace</a>, <a href='SkImageInfo_Reference#kRec601_SkYUVColorSpace'>kRec601_SkYUVColorSpace</a>,</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kRec709_SkYUVColorSpace'>kRec709_SkYUVColorSpace</a>

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromYUVTexturesCopyWithExternalBackend_yuvTextures'><code><strong>yuvTextures</strong></code></a></td>
    <td>array of YUV textures on GPU</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVTexturesCopyWithExternalBackend_imageOrigin'><code><strong>imageOrigin</strong></code></a></td>
    <td>one of: <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft_GrSurfaceOrigin</a>, <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVTexturesCopyWithExternalBackend_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td>the resource that stores the final pixels</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromYUVTexturesCopyWithExternalBackend_imageColorSpace'><code><strong>imageColorSpace</strong></code></a></td>
    <td>range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>

### See Also

<a href='#SkImage_MakeFromYUVTexturesCopy'>MakeFromYUVTexturesCopy</a> <a href='#SkImage_MakeFromNV12TexturesCopy'>MakeFromNV12TexturesCopy</a> <a href='#SkImage_MakeFromYUVATexturesCopyWithExternalBackend'>MakeFromYUVATexturesCopyWithExternalBackend</a>

<a name='SkImage_MakeFromNV12TexturesCopy'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_MakeFromNV12TexturesCopy'>MakeFromNV12TexturesCopy</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>, <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> <a href='SkImageInfo_Reference#SkYUVColorSpace'>yuvColorSpace</a>,
                                               <a href='SkImageInfo_Reference#SkYUVColorSpace'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='undocumented#GrBackendTexture'>nv12Textures</a>[2],
                                               <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>imageOrigin</a>,
                                               <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&<a href='undocumented#SkColorSpace'>gt</a>; <a href='undocumented#SkColorSpace'>imageColorSpace</a> = <a href='undocumented#SkColorSpace'>nullptr</a>)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>from</a> <a href='SkImage_Reference#SkImage'>copy</a> <a href='SkImage_Reference#SkImage'>of</a> <a href='#SkImage_MakeFromNV12TexturesCopy_nv12Textures'>nv12Textures</a>, <a href='#SkImage_MakeFromNV12TexturesCopy_nv12Textures'>an</a> <a href='#SkImage_MakeFromNV12TexturesCopy_nv12Textures'>array</a> <a href='#SkImage_MakeFromNV12TexturesCopy_nv12Textures'>of</a> <a href='#SkImage_MakeFromNV12TexturesCopy_nv12Textures'>textures</a> <a href='#SkImage_MakeFromNV12TexturesCopy_nv12Textures'>on</a> <a href='#SkImage_MakeFromNV12TexturesCopy_nv12Textures'>GPU</a>.
<a href='#SkImage_MakeFromNV12TexturesCopy_nv12Textures'>nv12Textures</a>[0] <a href='#SkImage_MakeFromNV12TexturesCopy_nv12Textures'>contains</a> <a href='#SkImage_MakeFromNV12TexturesCopy_nv12Textures'>pixels</a> <a href='#SkImage_MakeFromNV12TexturesCopy_nv12Textures'>for</a>   <a href='undocumented#YUV_Component_Y'>YUV component y</a> <a href='#SkImage_MakeFromNV12TexturesCopy_nv12Textures'>plane</a>.
<a href='#SkImage_MakeFromNV12TexturesCopy_nv12Textures'>nv12Textures</a>[1] <a href='#SkImage_MakeFromNV12TexturesCopy_nv12Textures'>contains</a> <a href='#SkImage_MakeFromNV12TexturesCopy_nv12Textures'>pixels</a> <a href='#SkImage_MakeFromNV12TexturesCopy_nv12Textures'>for</a>   <a href='undocumented#YUV_Component_U'>YUV component u</a> <a href='#SkImage_MakeFromNV12TexturesCopy_nv12Textures'>plane</a>,
followed by pixels for   <a href='undocumented#YUV_Component_V'>YUV component v</a> plane.
Returned <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>has</a> <a href='SkImage_Reference#SkImage'>the</a> <a href='SkImage_Reference#SkImage'>dimensions</a> <a href='#SkImage_MakeFromNV12TexturesCopy_nv12Textures'>nv12Textures</a>[2].
<a href='#SkImage_MakeFromNV12TexturesCopy_yuvColorSpace'>yuvColorSpace</a> <a href='#SkImage_MakeFromNV12TexturesCopy_yuvColorSpace'>describes</a> <a href='#SkImage_MakeFromNV12TexturesCopy_yuvColorSpace'>how</a> <a href='#SkImage_MakeFromNV12TexturesCopy_yuvColorSpace'>YUV</a> <a href='#SkImage_MakeFromNV12TexturesCopy_yuvColorSpace'>colors</a> <a href='#SkImage_MakeFromNV12TexturesCopy_yuvColorSpace'>convert</a> <a href='#SkImage_MakeFromNV12TexturesCopy_yuvColorSpace'>to</a> <a href='#SkImage_MakeFromNV12TexturesCopy_yuvColorSpace'>RGB</a> <a href='#SkImage_MakeFromNV12TexturesCopy_yuvColorSpace'>colors</a>.

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

created <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>

### See Also

<a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend'>MakeFromNV12TexturesCopyWithExternalBackend</a> <a href='#SkImage_MakeFromYUVTexturesCopy'>MakeFromYUVTexturesCopy</a> <a href='#SkImage_MakeFromYUVATexturesCopy'>MakeFromYUVATexturesCopy</a>

<a name='SkImage_MakeFromNV12TexturesCopyWithExternalBackend'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend'>MakeFromNV12TexturesCopyWithExternalBackend</a>(
            <a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>,
                        <a href='SkImageInfo_Reference#SkYUVColorSpace'>SkYUVColorSpace</a> <a href='SkImageInfo_Reference#SkYUVColorSpace'>yuvColorSpace</a>, <a href='SkImageInfo_Reference#SkYUVColorSpace'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='undocumented#GrBackendTexture'>nv12Textures</a>[2],
                        <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>imageOrigin</a>, <a href='undocumented#GrSurfaceOrigin'>const</a> <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>& <a href='undocumented#GrBackendTexture'>backendTexture</a>,
                        <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&<a href='undocumented#SkColorSpace'>gt</a>; <a href='undocumented#SkColorSpace'>imageColorSpace</a> = <a href='undocumented#SkColorSpace'>nullptr</a>) ;
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>from</a> <a href='SkImage_Reference#SkImage'>copy</a> <a href='SkImage_Reference#SkImage'>of</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>nv12Textures</a>, <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>an</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>array</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>of</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>textures</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>on</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>GPU</a>.
<a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>nv12Textures</a>[0] <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>contains</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>pixels</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>for</a>   <a href='undocumented#YUV_Component_Y'>YUV component y</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>plane</a>.
<a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>nv12Textures</a>[1] <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>contains</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>pixels</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>for</a>   <a href='undocumented#YUV_Component_U'>YUV component u</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>plane</a>,
followed by pixels for   <a href='undocumented#YUV_Component_V'>YUV component v</a> plane.
Returned <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>has</a> <a href='SkImage_Reference#SkImage'>the</a> <a href='SkImage_Reference#SkImage'>dimensions</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>nv12Textures</a>[2] <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>and</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>stores</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>pixels</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_nv12Textures'>in</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_backendTexture'>backendTexture</a>.
<a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_yuvColorSpace'>yuvColorSpace</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_yuvColorSpace'>describes</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_yuvColorSpace'>how</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_yuvColorSpace'>YUV</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_yuvColorSpace'>colors</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_yuvColorSpace'>convert</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_yuvColorSpace'>to</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_yuvColorSpace'>RGB</a> <a href='#SkImage_MakeFromNV12TexturesCopyWithExternalBackend_yuvColorSpace'>colors</a>.

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

created <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>

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
Use 16 bits per ARGB component using half-precision floating <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>format</a>.
</td>
  </tr>
</table>

### See Also

<a href='#SkImage_MakeFromPicture'>MakeFromPicture</a>

<a name='SkImage_MakeFromPicture'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_MakeFromPicture'>MakeFromPicture</a>(<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkPicture_Reference#SkPicture'>SkPicture</a>&<a href='SkPicture_Reference#SkPicture'>gt</a>; <a href='SkPicture_Reference#Picture'>picture</a>, <a href='SkPicture_Reference#Picture'>const</a> <a href='undocumented#SkISize'>SkISize</a>& <a href='undocumented#SkISize'>dimensions</a>,
                                      <a href='undocumented#SkISize'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#Matrix'>matrix</a>, <a href='SkMatrix_Reference#Matrix'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                                      <a href='#SkImage_BitDepth'>BitDepth</a> <a href='#SkImage_BitDepth'>bitDepth</a>, <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&<a href='undocumented#SkColorSpace'>gt</a>; <a href='#SkImage_colorSpace'>colorSpace</a>)
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>from</a> <a href='#SkImage_MakeFromPicture_picture'>picture</a>. <a href='#SkImage_MakeFromPicture_picture'>Returned</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>width</a> <a href='SkImage_Reference#SkImage'>and</a> <a href='SkImage_Reference#SkImage'>height</a> <a href='SkImage_Reference#SkImage'>are</a> <a href='SkImage_Reference#SkImage'>set</a> <a href='SkImage_Reference#SkImage'>by</a> <a href='#SkImage_MakeFromPicture_dimensions'>dimensions</a>.
<a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>draws</a> <a href='#SkImage_MakeFromPicture_picture'>picture</a> <a href='#SkImage_MakeFromPicture_picture'>with</a> <a href='#SkImage_MakeFromPicture_matrix'>matrix</a> <a href='#SkImage_MakeFromPicture_matrix'>and</a> <a href='#SkImage_MakeFromPicture_paint'>paint</a>, <a href='#SkImage_MakeFromPicture_paint'>set</a> <a href='#SkImage_MakeFromPicture_paint'>to</a> <a href='#SkImage_MakeFromPicture_bitDepth'>bitDepth</a> <a href='#SkImage_MakeFromPicture_bitDepth'>and</a> <a href='#SkImage_MakeFromPicture_colorSpace'>colorSpace</a>.

If <a href='#SkImage_MakeFromPicture_matrix'>matrix</a> <a href='#SkImage_MakeFromPicture_matrix'>is</a> <a href='#SkImage_MakeFromPicture_matrix'>nullptr</a>, <a href='#SkImage_MakeFromPicture_matrix'>draws</a> <a href='#SkImage_MakeFromPicture_matrix'>with</a> <a href='#SkImage_MakeFromPicture_matrix'>identity</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. <a href='SkMatrix_Reference#SkMatrix'>If</a> <a href='#SkImage_MakeFromPicture_paint'>paint</a> <a href='#SkImage_MakeFromPicture_paint'>is</a> <a href='#SkImage_MakeFromPicture_paint'>nullptr</a>, <a href='#SkImage_MakeFromPicture_paint'>draws</a>
with default <a href='SkPaint_Reference#SkPaint'>SkPaint</a>. <a href='#SkImage_MakeFromPicture_colorSpace'>colorSpace</a> <a href='#SkImage_MakeFromPicture_colorSpace'>may</a> <a href='#SkImage_MakeFromPicture_colorSpace'>be</a> <a href='#SkImage_MakeFromPicture_colorSpace'>nullptr</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeFromPicture_picture'><code><strong>picture</strong></code></a></td>
    <td><a href='SkStream_Reference#Stream'>stream</a> <a href='SkStream_Reference#Stream'>of</a> <a href='SkStream_Reference#Stream'>drawing</a> <a href='SkStream_Reference#Stream'>commands</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromPicture_dimensions'><code><strong>dimensions</strong></code></a></td>
    <td>width and height</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromPicture_matrix'><code><strong>matrix</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>rotate</a>, <a href='SkMatrix_Reference#SkMatrix'>scale</a>, <a href='SkMatrix_Reference#SkMatrix'>translate</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>so</a> <a href='SkMatrix_Reference#SkMatrix'>on</a>; <a href='SkMatrix_Reference#SkMatrix'>may</a> <a href='SkMatrix_Reference#SkMatrix'>be</a> <a href='SkMatrix_Reference#SkMatrix'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromPicture_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>to</a> <a href='SkPaint_Reference#SkPaint'>apply</a> <a href='SkPaint_Reference#SkPaint'>transparency</a>, <a href='SkPaint_Reference#SkPaint'>filtering</a>, <a href='SkPaint_Reference#SkPaint'>and</a> <a href='SkPaint_Reference#SkPaint'>so</a> <a href='SkPaint_Reference#SkPaint'>on</a>; <a href='SkPaint_Reference#SkPaint'>may</a> <a href='SkPaint_Reference#SkPaint'>be</a> <a href='SkPaint_Reference#SkPaint'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromPicture_bitDepth'><code><strong>bitDepth</strong></code></a></td>
    <td>8-bit integer or 16-bit float: per component</td>
  </tr>
  <tr>    <td><a name='SkImage_MakeFromPicture_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td>range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>

### Example

<div><fiddle-embed name="4aa2879b9e44dfd6648995326d2c4dcf"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawPicture'>drawPicture</a>

<a name='SkImage_MakeFromAHardwareBuffer'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_MakeFromAHardwareBuffer'>MakeFromAHardwareBuffer</a>(
        <a href='#SkImage_MakeFromAHardwareBuffer'>AHardwareBuffer</a>* <a href='#SkImage_MakeFromAHardwareBuffer'>hardwareBuffer</a>,
                                   <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImage_alphaType'>alphaType</a> = <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
                                   <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&<a href='undocumented#SkColorSpace'>gt</a>; <a href='#SkImage_colorSpace'>colorSpace</a> = <a href='#SkImage_colorSpace'>nullptr</a>,
                                   <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a> <a href='undocumented#GrSurfaceOrigin'>surfaceOrigin</a> = <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a>)
</pre>

(See Skia bug 7447)
Creates <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>from</a> <a href='SkImage_Reference#SkImage'>Android</a> <a href='SkImage_Reference#SkImage'>hardware</a> <a href='SkImage_Reference#SkImage'>buffer</a>.
Returned <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>takes</a> <a href='SkImage_Reference#SkImage'>a</a> <a href='SkImage_Reference#SkImage'>reference</a> <a href='SkImage_Reference#SkImage'>on</a> <a href='SkImage_Reference#SkImage'>the</a> <a href='SkImage_Reference#SkImage'>buffer</a>.

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

created <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>

### See Also

<a href='#SkImage_MakeFromRaster'>MakeFromRaster</a>

<a name='Property'></a>

<a name='SkImage_width'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkImage_width'>width()</a> <a href='#SkImage_width'>const</a>
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>count</a> <a href='undocumented#Pixel'>in</a> <a href='undocumented#Pixel'>each</a> <a href='undocumented#Pixel'>row</a>.

### Return Value

<a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>width</a> <a href='undocumented#Pixel'>in</a> <a href='SkImage_Reference#SkImage'>SkImage</a>

### Example

<div><fiddle-embed name="9aec65fc252ffc9982fa8867433eca18"></fiddle-embed></div>

### See Also

<a href='#SkImage_dimensions'>dimensions()</a> <a href='#SkImage_height'>height()</a>

<a name='SkImage_height'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkImage_height'>height()</a> <a href='#SkImage_height'>const</a>
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>count</a>.

### Return Value

<a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>height</a> <a href='undocumented#Pixel'>in</a> <a href='SkImage_Reference#SkImage'>SkImage</a>

### Example

<div><fiddle-embed name="a4f53a0b6ac85e7bc3887245b728530d"></fiddle-embed></div>

### See Also

<a href='#SkImage_dimensions'>dimensions()</a> <a href='#SkImage_width'>width()</a>

<a name='SkImage_dimensions'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkISize'>SkISize</a> <a href='#SkImage_dimensions'>dimensions()</a> <a href='#SkImage_dimensions'>const</a>
</pre>

Returns <a href='undocumented#SkISize'>SkISize</a> { <a href='#SkImage_width'>width()</a>, <a href='#SkImage_height'>height()</a> }.

### Return Value

integral <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='#SkImage_width'>width()</a> <a href='#SkImage_width'>and</a> <a href='#SkImage_height'>height()</a>

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
<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkImage_bounds'>bounds()</a> <a href='#SkImage_bounds'>const</a>
</pre>

Returns <a href='SkIRect_Reference#SkIRect'>SkIRect</a> { 0, 0, <a href='#SkImage_width'>width()</a>, <a href='#SkImage_height'>height()</a> }.

### Return Value

integral rectangle from origin to <a href='#SkImage_width'>width()</a> <a href='#SkImage_width'>and</a> <a href='#SkImage_height'>height()</a>

### Example

<div><fiddle-embed name="c204b38b3fc08914b0a634aa4eaec894"></fiddle-embed></div>

### See Also

<a href='#SkImage_dimensions'>dimensions()</a>

<a name='SkImage_uniqueID'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkImage_uniqueID'>uniqueID</a>() <a href='#SkImage_uniqueID'>const</a>
</pre>

Returns value unique to <a href='SkImage_Reference#Image'>image</a>. <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>contents</a> <a href='SkImage_Reference#SkImage'>cannot</a> <a href='SkImage_Reference#SkImage'>change</a> <a href='SkImage_Reference#SkImage'>after</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a>
created. Any operation to create a new <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>will</a> <a href='SkImage_Reference#SkImage'>receive</a> <a href='SkImage_Reference#SkImage'>generate</a> <a href='SkImage_Reference#SkImage'>a</a> <a href='SkImage_Reference#SkImage'>new</a>
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
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkImage_alphaType'>alphaType</a>() <a href='#SkImage_alphaType'>const</a>
</pre>

Returns <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, <a href='#Image_Info_Alpha_Type'>one</a> <a href='#Image_Info_Alpha_Type'>of</a>: <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>
.

<a href='#Image_Info_Alpha_Type'>Alpha_Type</a> <a href='#Image_Info_Alpha_Type'>returned</a> <a href='#Image_Info_Alpha_Type'>was</a> <a href='#Image_Info_Alpha_Type'>a</a> <a href='#Image_Info_Alpha_Type'>parameter</a> <a href='#Image_Info_Alpha_Type'>to</a> <a href='#Image_Info_Alpha_Type'>an</a> <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>constructor</a>,
<a href='SkImage_Reference#Image'>or</a> <a href='SkImage_Reference#Image'>was</a> <a href='SkImage_Reference#Image'>parsed</a> <a href='SkImage_Reference#Image'>from</a> <a href='SkImage_Reference#Image'>encoded</a> <a href='undocumented#Data'>data</a>.

### Return Value

<a href='#Image_Info_Alpha_Type'>Alpha_Type</a> <a href='#Image_Info_Alpha_Type'>in</a> <a href='SkImage_Reference#Image'>Image</a>

### Example

<div><fiddle-embed name="1b9f1f05026ceb14ccb6926a13cdaa83"></fiddle-embed></div>

### See Also

<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_alphaType'>alphaType</a>

<a name='SkImage_colorType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkImage_colorType'>colorType</a>() <a href='#SkImage_colorType'>const</a>
</pre>

Returns <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>if</a> <a href='SkImageInfo_Reference#SkColorType'>known</a>; <a href='SkImageInfo_Reference#SkColorType'>otherwise</a>, <a href='SkImageInfo_Reference#SkColorType'>returns</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>.

### Return Value

<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>of</a> <a href='SkImage_Reference#SkImage'>SkImage</a>

### Example

<div><fiddle-embed name="50396fad4a128f58e400ca00fe09711f"></fiddle-embed></div>

### See Also

<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_colorType'>colorType</a>

<a name='SkImage_colorSpace'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='#SkImage_colorSpace'>colorSpace</a>() <a href='#SkImage_colorSpace'>const</a>
</pre>

Returns <a href='undocumented#SkColorSpace'>SkColorSpace</a>, <a href='undocumented#SkColorSpace'>the</a> <a href='undocumented#SkColorSpace'>range</a> <a href='undocumented#SkColorSpace'>of</a> <a href='undocumented#SkColorSpace'>colors</a>, <a href='undocumented#SkColorSpace'>associated</a> <a href='undocumented#SkColorSpace'>with</a> <a href='SkImage_Reference#SkImage'>SkImage</a>.  <a href='SkImage_Reference#SkImage'>The</a>
reference count of <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>is</a> <a href='undocumented#SkColorSpace'>unchanged</a>. <a href='undocumented#SkColorSpace'>The</a> <a href='undocumented#SkColorSpace'>returned</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>is</a>
immutable.

<a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>returned</a> <a href='undocumented#SkColorSpace'>was</a> <a href='undocumented#SkColorSpace'>passed</a> <a href='undocumented#SkColorSpace'>to</a> <a href='undocumented#SkColorSpace'>an</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>constructor</a>,
or was parsed from encoded <a href='undocumented#Data'>data</a>. <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>returned</a> <a href='undocumented#SkColorSpace'>may</a> <a href='undocumented#SkColorSpace'>be</a> <a href='undocumented#SkColorSpace'>ignored</a> <a href='undocumented#SkColorSpace'>when</a> <a href='SkImage_Reference#SkImage'>SkImage</a>
is drawn, depending on the capabilities of the <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>receiving</a> <a href='SkSurface_Reference#SkSurface'>the</a> <a href='SkSurface_Reference#SkSurface'>drawing</a>.

### Return Value

<a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>in</a> <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>

### Example

<div><fiddle-embed name="4468d573f42af6f5e234be10a5453bb2"></fiddle-embed></div>

### See Also

<a href='#SkImage_refColorSpace'>refColorSpace</a> <a href='#SkImage_makeColorSpace'>makeColorSpace</a>

<a name='SkImage_refColorSpace'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&<a href='undocumented#SkColorSpace'>gt</a>; <a href='#SkImage_refColorSpace'>refColorSpace</a>() <a href='#SkImage_refColorSpace'>const</a>
</pre>

Returns a smart pointer to <a href='undocumented#SkColorSpace'>SkColorSpace</a>, <a href='undocumented#SkColorSpace'>the</a> <a href='undocumented#SkColorSpace'>range</a> <a href='undocumented#SkColorSpace'>of</a> <a href='undocumented#SkColorSpace'>colors</a>, <a href='undocumented#SkColorSpace'>associated</a> <a href='undocumented#SkColorSpace'>with</a>
<a href='SkImage_Reference#SkImage'>SkImage</a>.  <a href='SkImage_Reference#SkImage'>The</a> <a href='SkImage_Reference#SkImage'>smart</a> <a href='SkImage_Reference#SkImage'>pointer</a> <a href='SkImage_Reference#SkImage'>tracks</a> <a href='SkImage_Reference#SkImage'>the</a> <a href='SkImage_Reference#SkImage'>number</a> <a href='SkImage_Reference#SkImage'>of</a> <a href='SkImage_Reference#SkImage'>objects</a> <a href='SkImage_Reference#SkImage'>sharing</a> <a href='SkImage_Reference#SkImage'>this</a>
<a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>reference</a> <a href='undocumented#SkColorSpace'>so</a> <a href='undocumented#SkColorSpace'>the</a> <a href='undocumented#SkColorSpace'>memory</a> <a href='undocumented#SkColorSpace'>is</a> <a href='undocumented#SkColorSpace'>released</a> <a href='undocumented#SkColorSpace'>when</a> <a href='undocumented#SkColorSpace'>the</a> <a href='undocumented#SkColorSpace'>owners</a> <a href='undocumented#SkColorSpace'>destruct</a>.

The returned <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>is</a> <a href='undocumented#SkColorSpace'>immutable</a>.

<a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>returned</a> <a href='undocumented#SkColorSpace'>was</a> <a href='undocumented#SkColorSpace'>passed</a> <a href='undocumented#SkColorSpace'>to</a> <a href='undocumented#SkColorSpace'>an</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>constructor</a>,
or was parsed from encoded <a href='undocumented#Data'>data</a>. <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>returned</a> <a href='undocumented#SkColorSpace'>may</a> <a href='undocumented#SkColorSpace'>be</a> <a href='undocumented#SkColorSpace'>ignored</a> <a href='undocumented#SkColorSpace'>when</a> <a href='SkImage_Reference#SkImage'>SkImage</a>
is drawn, depending on the capabilities of the <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>receiving</a> <a href='SkSurface_Reference#SkSurface'>the</a> <a href='SkSurface_Reference#SkSurface'>drawing</a>.

### Return Value

<a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>in</a> <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>, <a href='SkImage_Reference#SkImage'>wrapped</a> <a href='SkImage_Reference#SkImage'>in</a> <a href='SkImage_Reference#SkImage'>a</a> <a href='SkImage_Reference#SkImage'>smart</a> <a href='SkImage_Reference#SkImage'>pointer</a>

### Example

<div><fiddle-embed name="59b2078ebfbda8736a57c0486ae33332"></fiddle-embed></div>

### See Also

<a href='#SkImage_colorSpace'>colorSpace</a> <a href='#SkImage_makeColorSpace'>makeColorSpace</a>

<a name='SkImage_isAlphaOnly'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_isAlphaOnly'>isAlphaOnly</a>() <a href='#SkImage_isAlphaOnly'>const</a>
</pre>

Returns true if <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>pixels</a> <a href='SkImage_Reference#SkImage'>represent</a> <a href='SkImage_Reference#SkImage'>transparency</a> <a href='SkImage_Reference#SkImage'>only</a>. <a href='SkImage_Reference#SkImage'>If</a> <a href='SkImage_Reference#SkImage'>true</a>, <a href='SkImage_Reference#SkImage'>each</a> <a href='undocumented#Pixel'>pixel</a>
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
bool <a href='#SkImage_isOpaque'>isOpaque</a>() <a href='#SkImage_isOpaque'>const</a>
</pre>

Returns true if pixels ignore their <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>value</a> <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>are</a> <a href='SkColor_Reference#Alpha'>treated</a> <a href='SkColor_Reference#Alpha'>as</a> <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>opaque</a>.

### Return Value

true if <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>is</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>

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
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkShader'>SkShader</a>&<a href='undocumented#SkShader'>gt</a>; <a href='#SkImage_makeShader'>makeShader</a>(<a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_TileMode'>TileMode</a> <a href='#SkShader_TileMode'>tileMode1</a>, <a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_TileMode'>TileMode</a> <a href='#SkShader_TileMode'>tileMode2</a>,
                           <a href='#SkShader_TileMode'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#SkMatrix'>localMatrix</a> = <a href='SkMatrix_Reference#SkMatrix'>nullptr</a>) <a href='SkMatrix_Reference#SkMatrix'>const</a>
</pre>

Creates <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>from</a> <a href='SkImage_Reference#SkImage'>SkImage</a>. <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>dimensions</a> <a href='undocumented#SkShader'>are</a> <a href='undocumented#SkShader'>taken</a> <a href='undocumented#SkShader'>from</a> <a href='SkImage_Reference#SkImage'>SkImage</a>. <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>uses</a>
<a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_TileMode'>TileMode</a> <a href='#SkShader_TileMode'>rules</a> <a href='#SkShader_TileMode'>to</a> <a href='#SkShader_TileMode'>fill</a> <a href='#SkShader_TileMode'>drawn</a> <a href='#SkShader_TileMode'>area</a> <a href='#SkShader_TileMode'>outside</a> <a href='SkImage_Reference#SkImage'>SkImage</a>. <a href='#SkImage_makeShader_localMatrix'>localMatrix</a> <a href='#SkImage_makeShader_localMatrix'>permits</a>
transforming <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>before</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>is</a> <a href='SkMatrix_Reference#Matrix'>applied</a>.

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
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>transformation</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a></td>
  </tr>
</table>

### Return Value

<a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>containing</a> <a href='SkImage_Reference#SkImage'>SkImage</a>

### Example

<div><fiddle-embed name="1c6de6fe72b00b5be970f5f718363449"></fiddle-embed></div>

### See Also

<a href='#SkImage_scalePixels'>scalePixels</a>

<a name='SkImage_makeShader_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkShader'>SkShader</a>&<a href='undocumented#SkShader'>gt</a>; <a href='#SkImage_makeShader'>makeShader</a>(<a href='#SkImage_makeShader'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#SkMatrix'>localMatrix</a> = <a href='SkMatrix_Reference#SkMatrix'>nullptr</a>) <a href='SkMatrix_Reference#SkMatrix'>const</a>
</pre>

Creates <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>from</a> <a href='SkImage_Reference#SkImage'>SkImage</a>. <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>dimensions</a> <a href='undocumented#SkShader'>are</a> <a href='undocumented#SkShader'>taken</a> <a href='undocumented#SkShader'>from</a> <a href='SkImage_Reference#SkImage'>SkImage</a>. <a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>uses</a>
<a href='undocumented#SkShader'>SkShader</a>::<a href='#SkShader_kClamp_TileMode'>kClamp_TileMode</a> <a href='#SkShader_kClamp_TileMode'>to</a> <a href='#SkShader_kClamp_TileMode'>fill</a> <a href='#SkShader_kClamp_TileMode'>drawn</a> <a href='#SkShader_kClamp_TileMode'>area</a> <a href='#SkShader_kClamp_TileMode'>outside</a> <a href='SkImage_Reference#SkImage'>SkImage</a>. <a href='#SkImage_makeShader_2_localMatrix'>localMatrix</a> <a href='#SkImage_makeShader_2_localMatrix'>permits</a>
transforming <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>before</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>is</a> <a href='SkMatrix_Reference#Matrix'>applied</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_makeShader_2_localMatrix'><code><strong>localMatrix</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>transformation</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a></td>
  </tr>
</table>

### Return Value

<a href='undocumented#SkShader'>SkShader</a> <a href='undocumented#SkShader'>containing</a> <a href='SkImage_Reference#SkImage'>SkImage</a>

### Example

<div><fiddle-embed name="10172fca71b9dbdcade772513ffeb27e"></fiddle-embed></div>

### See Also

<a href='#SkImage_scalePixels'>scalePixels</a>

<a name='Pixels'></a>

<a name='SkImage_peekPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_peekPixels'>peekPixels</a>(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>* <a href='SkPixmap_Reference#Pixmap'>pixmap</a>) <a href='SkPixmap_Reference#Pixmap'>const</a>
</pre>

Copies <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a>, <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>bytes</a>, <a href='undocumented#Pixel'>and</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>to</a> <a href='#SkImage_peekPixels_pixmap'>pixmap</a>, <a href='#SkImage_peekPixels_pixmap'>if</a> <a href='#SkImage_peekPixels_pixmap'>address</a>
is available, and returns true. If <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>not</a> <a href='undocumented#Pixel'>available</a>, <a href='undocumented#Pixel'>return</a>
false and leave <a href='#SkImage_peekPixels_pixmap'>pixmap</a> <a href='#SkImage_peekPixels_pixmap'>unchanged</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_peekPixels_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td>storage for <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>state</a> <a href='undocumented#Pixel'>if</a> <a href='undocumented#Pixel'>pixels</a> <a href='undocumented#Pixel'>are</a> <a href='undocumented#Pixel'>readable</a>; <a href='undocumented#Pixel'>otherwise</a>, <a href='undocumented#Pixel'>ignored</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>has</a> <a href='SkImage_Reference#SkImage'>direct</a> <a href='SkImage_Reference#SkImage'>access</a> <a href='SkImage_Reference#SkImage'>to</a> <a href='SkImage_Reference#SkImage'>pixels</a>

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

<a name='SkImage_getTexture'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
GrTexture* <a href='#SkImage_getTexture'>getTexture</a>() <a href='#SkImage_getTexture'>const</a>
</pre>

Deprecated.

<a name='SkImage_isTextureBacked'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_isTextureBacked'>isTextureBacked</a>() <a href='#SkImage_isTextureBacked'>const</a>
</pre>

Returns true the contents of <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>was</a> <a href='SkImage_Reference#SkImage'>created</a> <a href='SkImage_Reference#SkImage'>on</a> <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>uploaded</a> <a href='SkImage_Reference#SkImage'>to</a> <a href='SkImage_Reference#SkImage'>GPU</a> <a href='SkImage_Reference#SkImage'>memory</a>,
and is available as a GPU <a href='undocumented#Texture'>texture</a>.

### Return Value

true if <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>a</a> <a href='SkImage_Reference#SkImage'>GPU</a> <a href='undocumented#Texture'>texture</a>

### Example

<div><fiddle-embed name="9cf5c62a3d2243e6577ae563f360ea9d" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromTexture'>MakeFromTexture</a> <a href='#SkImage_isValid'>isValid</a>

<a name='SkImage_isValid'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_isValid'>isValid</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>) <a href='undocumented#GrContext'>const</a>
</pre>

Returns true if <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>can</a> <a href='SkImage_Reference#SkImage'>be</a> <a href='SkImage_Reference#SkImage'>drawn</a> <a href='SkImage_Reference#SkImage'>on</a> <a href='SkImage_Reference#SkImage'>either</a>  <a href='undocumented#Raster_Surface'>raster surface</a> <a href='SkImage_Reference#SkImage'>or</a>  <a href='undocumented#GPU_Surface'>GPU surface</a>.
If <a href='#SkImage_isValid_context'>context</a> <a href='#SkImage_isValid_context'>is</a> <a href='#SkImage_isValid_context'>nullptr</a>, <a href='#SkImage_isValid_context'>tests</a> <a href='#SkImage_isValid_context'>if</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>draws</a> <a href='SkImage_Reference#SkImage'>on</a>  <a href='undocumented#Raster_Surface'>raster surface</a>;
otherwise, tests if <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>draws</a> <a href='SkImage_Reference#SkImage'>on</a>  <a href='undocumented#GPU_Surface'>GPU surface</a> <a href='SkImage_Reference#SkImage'>associated</a> <a href='SkImage_Reference#SkImage'>with</a> <a href='#SkImage_isValid_context'>context</a>.

<a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>backed</a> <a href='SkImage_Reference#SkImage'>by</a>  <a href='undocumented#GPU_Texture'>GPU texture</a> <a href='SkImage_Reference#SkImage'>may</a> <a href='SkImage_Reference#SkImage'>become</a> <a href='SkImage_Reference#SkImage'>invalid</a> <a href='SkImage_Reference#SkImage'>if</a> <a href='SkImage_Reference#SkImage'>associated</a> <a href='undocumented#GrContext'>GrContext</a> <a href='undocumented#GrContext'>is</a>
invalid.  <a href='#Lazy_Image'>lazy image</a> may be invalid and may not draw to  <a href='undocumented#Raster_Surface'>raster surface</a> or
<a href='undocumented#GPU_Surface'>GPU surface</a> or both.

### Parameters

<table>  <tr>    <td><a name='SkImage_isValid_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>can</a> <a href='SkImage_Reference#SkImage'>be</a> <a href='SkImage_Reference#SkImage'>drawn</a>

### Example

<div><fiddle-embed name="afc62f38aebc56af8e425297ec67dd37" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_isTextureBacked'>isTextureBacked</a> <a href='#SkImage_isLazyGenerated'>isLazyGenerated</a>

<a name='SkImage_getBackendTexture'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='#SkImage_getBackendTexture'>getBackendTexture</a>(<a href='#SkImage_getBackendTexture'>bool</a> <a href='#SkImage_getBackendTexture'>flushPendingGrContextIO</a>, <a href='undocumented#GrSurfaceOrigin'>GrSurfaceOrigin</a>* <a href='undocumented#GrSurfaceOrigin'>origin</a> = <a href='undocumented#GrSurfaceOrigin'>nullptr</a>) <a href='undocumented#GrSurfaceOrigin'>const</a>
</pre>

Retrieves the back-end <a href='undocumented#Texture'>texture</a>. <a href='undocumented#Texture'>If</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>has</a> <a href='SkImage_Reference#SkImage'>no</a> <a href='SkImage_Reference#SkImage'>back-end</a> <a href='undocumented#Texture'>texture</a>, <a href='undocumented#Texture'>an</a> <a href='undocumented#Texture'>invalid</a>
object is returned. Call <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>::<a href='#GrBackendTexture_isValid'>isValid</a> <a href='#GrBackendTexture_isValid'>to</a> <a href='#GrBackendTexture_isValid'>determine</a> <a href='#GrBackendTexture_isValid'>if</a> <a href='#GrBackendTexture_isValid'>the</a> <a href='#GrBackendTexture_isValid'>result</a>
is valid.

If <a href='#SkImage_getBackendTexture_flushPendingGrContextIO'>flushPendingGrContextIO</a> <a href='#SkImage_getBackendTexture_flushPendingGrContextIO'>is</a> <a href='#SkImage_getBackendTexture_flushPendingGrContextIO'>true</a>, <a href='#SkImage_getBackendTexture_flushPendingGrContextIO'>completes</a> <a href='#SkImage_getBackendTexture_flushPendingGrContextIO'>deferred</a> <a href='#SkImage_getBackendTexture_flushPendingGrContextIO'>I/O</a> <a href='#SkImage_getBackendTexture_flushPendingGrContextIO'>operations</a>.

If <a href='#SkImage_getBackendTexture_origin'>origin</a> <a href='#SkImage_getBackendTexture_origin'>in</a> <a href='#SkImage_getBackendTexture_origin'>not</a> <a href='#SkImage_getBackendTexture_origin'>nullptr</a>, <a href='#SkImage_getBackendTexture_origin'>copies</a> <a href='#SkImage_getBackendTexture_origin'>location</a> <a href='#SkImage_getBackendTexture_origin'>of</a> <a href='#SkImage_getBackendTexture_origin'>content</a> <a href='#SkImage_getBackendTexture_origin'>drawn</a> <a href='#SkImage_getBackendTexture_origin'>into</a> <a href='SkImage_Reference#SkImage'>SkImage</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_getBackendTexture_flushPendingGrContextIO'><code><strong>flushPendingGrContextIO</strong></code></a></td>
    <td>flag to flush outstanding requests</td>
  </tr>
  <tr>    <td><a name='SkImage_getBackendTexture_origin'><code><strong>origin</strong></code></a></td>
    <td>storage for one of: <a href='undocumented#kTopLeft_GrSurfaceOrigin'>kTopLeft_GrSurfaceOrigin</a>,</td>
  </tr>
</table>

<a href='undocumented#kBottomLeft_GrSurfaceOrigin'>kBottomLeft_GrSurfaceOrigin</a>; <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>or</a> <a href='undocumented#kBottomLeft_GrSurfaceOrigin'>nullptr</a>

### Return Value

back-end API <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>handle</a>; <a href='undocumented#Texture'>invalid</a> <a href='undocumented#Texture'>on</a> <a href='undocumented#Texture'>failure</a>

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

<a href='#SkImage_CachingHint'>CachingHint</a> <a href='#SkImage_CachingHint'>selects</a> <a href='#SkImage_CachingHint'>whether</a> <a href='#SkImage_CachingHint'>Skia</a> <a href='#SkImage_CachingHint'>may</a> <a href='#SkImage_CachingHint'>internally</a> <a href='#SkImage_CachingHint'>cache</a> <a href='SkBitmap_Reference#Bitmap'>Bitmaps</a> <a href='SkBitmap_Reference#Bitmap'>generated</a> <a href='SkBitmap_Reference#Bitmap'>by</a>
<a href='SkBitmap_Reference#Bitmap'>decoding</a> <a href='SkImage_Reference#Image'>Image</a>, <a href='SkImage_Reference#Image'>or</a> <a href='SkImage_Reference#Image'>by</a> <a href='SkImage_Reference#Image'>copying</a> <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>from</a> <a href='SkImage_Reference#Image'>GPU</a> <a href='SkImage_Reference#Image'>to</a> <a href='SkImage_Reference#Image'>CPU</a>. <a href='SkImage_Reference#Image'>The</a> <a href='SkImage_Reference#Image'>default</a> <a href='SkImage_Reference#Image'>behavior</a>
<a href='SkImage_Reference#Image'>allows</a> <a href='SkImage_Reference#Image'>caching</a> <a href='SkBitmap_Reference#Bitmap'>Bitmaps</a>.

<a href='SkBitmap_Reference#Bitmap'>Choose</a> <a href='#SkImage_kDisallow_CachingHint'>kDisallow_CachingHint</a> <a href='#SkImage_kDisallow_CachingHint'>if</a> <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>pixels</a> <a href='SkImage_Reference#Image'>are</a> <a href='SkImage_Reference#Image'>to</a> <a href='SkImage_Reference#Image'>be</a> <a href='SkImage_Reference#Image'>used</a> <a href='SkImage_Reference#Image'>only</a> <a href='SkImage_Reference#Image'>once</a>, <a href='SkImage_Reference#Image'>or</a>
<a href='SkImage_Reference#Image'>if</a> <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>pixels</a> <a href='SkImage_Reference#Image'>reside</a> <a href='SkImage_Reference#Image'>in</a> <a href='SkImage_Reference#Image'>a</a> <a href='SkImage_Reference#Image'>cache</a> <a href='SkImage_Reference#Image'>outside</a> <a href='SkImage_Reference#Image'>of</a> <a href='SkImage_Reference#Image'>Skia</a>, <a href='SkImage_Reference#Image'>or</a> <a href='SkImage_Reference#Image'>to</a> <a href='SkImage_Reference#Image'>reduce</a> <a href='SkImage_Reference#Image'>memory</a> <a href='SkImage_Reference#Image'>pressure</a>.

<a href='SkImage_Reference#Image'>Choosing</a> <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a> <a href='#SkImage_kAllow_CachingHint'>does</a> <a href='#SkImage_kAllow_CachingHint'>not</a> <a href='#SkImage_kAllow_CachingHint'>ensure</a> <a href='#SkImage_kAllow_CachingHint'>that</a> <a href='#SkImage_kAllow_CachingHint'>pixels</a> <a href='#SkImage_kAllow_CachingHint'>will</a> <a href='#SkImage_kAllow_CachingHint'>be</a> <a href='#SkImage_kAllow_CachingHint'>cached</a>.
<a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>pixels</a> <a href='SkImage_Reference#Image'>may</a> <a href='SkImage_Reference#Image'>not</a> <a href='SkImage_Reference#Image'>be</a> <a href='SkImage_Reference#Image'>cached</a> <a href='SkImage_Reference#Image'>if</a> <a href='SkImage_Reference#Image'>memory</a> <a href='SkImage_Reference#Image'>requirements</a> <a href='SkImage_Reference#Image'>are</a> <a href='SkImage_Reference#Image'>too</a> <a href='SkImage_Reference#Image'>large</a> <a href='SkImage_Reference#Image'>or</a>
<a href='SkImage_Reference#Image'>pixels</a> <a href='SkImage_Reference#Image'>are</a> <a href='SkImage_Reference#Image'>not</a> <a href='SkImage_Reference#Image'>accessible</a>.

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
bool <a href='#SkImage_readPixels'>readPixels</a>(<a href='#SkImage_readPixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>dstInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>dstPixels</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>dstRowBytes</a>, <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>srcX</a>, <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>srcY</a>,
                <a href='#SkImage_CachingHint'>CachingHint</a> <a href='#SkImage_CachingHint'>cachingHint</a> = <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>) <a href='#SkImage_kAllow_CachingHint'>const</a>
</pre>

Copies <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>pixels</a> <a href='SkRect_Reference#Rect'>from</a> <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>to</a> <a href='#SkImage_readPixels_dstPixels'>dstPixels</a>. <a href='#SkImage_readPixels_dstPixels'>Copy</a> <a href='#SkImage_readPixels_dstPixels'>starts</a> <a href='#SkImage_readPixels_dstPixels'>at</a> <a href='#SkImage_readPixels_dstPixels'>offset</a> (<a href='#SkImage_readPixels_srcX'>srcX</a>, <a href='#SkImage_readPixels_srcY'>srcY</a>),
<a href='#SkImage_readPixels_srcY'>and</a> <a href='#SkImage_readPixels_srcY'>does</a> <a href='#SkImage_readPixels_srcY'>not</a> <a href='#SkImage_readPixels_srcY'>exceed</a> <a href='SkImage_Reference#Image'>Image</a> (<a href='#SkImage_width'>width()</a>, <a href='#SkImage_height'>height()</a>).

<a href='#SkImage_readPixels_dstInfo'>dstInfo</a> <a href='#SkImage_readPixels_dstInfo'>specifies</a> <a href='#SkImage_readPixels_dstInfo'>width</a>, <a href='#SkImage_readPixels_dstInfo'>height</a>, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, <a href='#Image_Info_Alpha_Type'>and</a> <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>of</a>
<a href='#Color_Space'>destination</a>. <a href='#SkImage_readPixels_dstRowBytes'>dstRowBytes</a> <a href='#SkImage_readPixels_dstRowBytes'>specifics</a> <a href='#SkImage_readPixels_dstRowBytes'>the</a> <a href='#SkImage_readPixels_dstRowBytes'>gap</a> <a href='#SkImage_readPixels_dstRowBytes'>from</a> <a href='#SkImage_readPixels_dstRowBytes'>one</a> <a href='#SkImage_readPixels_dstRowBytes'>destination</a> <a href='#SkImage_readPixels_dstRowBytes'>row</a> <a href='#SkImage_readPixels_dstRowBytes'>to</a> <a href='#SkImage_readPixels_dstRowBytes'>the</a> <a href='#SkImage_readPixels_dstRowBytes'>next</a>.
<a href='#SkImage_readPixels_dstRowBytes'>Returns</a> <a href='#SkImage_readPixels_dstRowBytes'>true</a> <a href='#SkImage_readPixels_dstRowBytes'>if</a> <a href='#SkImage_readPixels_dstRowBytes'>pixels</a> <a href='#SkImage_readPixels_dstRowBytes'>are</a> <a href='#SkImage_readPixels_dstRowBytes'>copied</a>. <a href='#SkImage_readPixels_dstRowBytes'>Returns</a> <a href='#SkImage_readPixels_dstRowBytes'>false</a> <a href='#SkImage_readPixels_dstRowBytes'>if</a>:

<table>  <tr>
    <td><a href='#SkImage_readPixels_dstInfo'>dstInfo</a> <a href='#SkImage_readPixels_dstInfo'>has</a> <a href='#SkImage_readPixels_dstInfo'>no</a> <a href='#SkImage_readPixels_dstInfo'>address</a></td>
  </tr>  <tr>
    <td><a href='#SkImage_readPixels_dstRowBytes'>dstRowBytes</a> <a href='#SkImage_readPixels_dstRowBytes'>is</a> <a href='#SkImage_readPixels_dstRowBytes'>less</a> <a href='#SkImage_readPixels_dstRowBytes'>than</a> <a href='#SkImage_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>()</td>
  </tr>  <tr>
    <td><a href='#Pixel_Ref'>Pixel_Ref</a> <a href='#Pixel_Ref'>is</a> <a href='#Pixel_Ref'>nullptr</a></td>
  </tr>
</table>

Pixels are copied only if <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>possible</a>. <a href='undocumented#Pixel'>If</a> <a href='SkImage_Reference#Image'>Image</a> <a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Image_Info_Color_Type'>is</a>
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kGray_8_SkColorType'>or</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkImage_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorType'>colorType</a>() <a href='#SkImageInfo_colorType'>must</a> <a href='#SkImageInfo_colorType'>match</a>.
<a href='#SkImageInfo_colorType'>If</a> <a href='SkImage_Reference#Image'>Image</a> <a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Image_Info_Color_Type'>is</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkImage_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorSpace'>colorSpace</a>() <a href='#SkImageInfo_colorSpace'>must</a> <a href='#SkImageInfo_colorSpace'>match</a>.
<a href='#SkImageInfo_colorSpace'>If</a> <a href='SkImage_Reference#Image'>Image</a> <a href='#Image_Info_Alpha_Type'>Alpha_Type</a> <a href='#Image_Info_Alpha_Type'>is</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkImage_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() <a href='#SkImageInfo_alphaType'>must</a>
<a href='#SkImageInfo_alphaType'>match</a>. <a href='#SkImageInfo_alphaType'>If</a> <a href='SkImage_Reference#Image'>Image</a> <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>is</a> <a href='#Color_Space'>nullptr</a>, <a href='#SkImage_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorSpace'>colorSpace</a>() <a href='#SkImageInfo_colorSpace'>must</a> <a href='#SkImageInfo_colorSpace'>match</a>. <a href='#SkImageInfo_colorSpace'>Returns</a>
<a href='#SkImageInfo_colorSpace'>false</a> <a href='#SkImageInfo_colorSpace'>if</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>not</a> <a href='undocumented#Pixel'>possible</a>.

<a href='#SkImage_readPixels_srcX'>srcX</a> <a href='#SkImage_readPixels_srcX'>and</a> <a href='#SkImage_readPixels_srcY'>srcY</a> <a href='#SkImage_readPixels_srcY'>may</a> <a href='#SkImage_readPixels_srcY'>be</a> <a href='#SkImage_readPixels_srcY'>negative</a> <a href='#SkImage_readPixels_srcY'>to</a> <a href='#SkImage_readPixels_srcY'>copy</a> <a href='#SkImage_readPixels_srcY'>only</a> <a href='#SkImage_readPixels_srcY'>top</a> <a href='#SkImage_readPixels_srcY'>or</a> <a href='#SkImage_readPixels_srcY'>left</a> <a href='#SkImage_readPixels_srcY'>of</a> <a href='#SkImage_readPixels_srcY'>source</a>. <a href='#SkImage_readPixels_srcY'>Returns</a>
<a href='#SkImage_readPixels_srcY'>false</a> <a href='#SkImage_readPixels_srcY'>if</a> <a href='#SkImage_width'>width()</a> <a href='#SkImage_width'>or</a> <a href='#SkImage_height'>height()</a> <a href='#SkImage_height'>is</a> <a href='#SkImage_height'>zero</a> <a href='#SkImage_height'>or</a> <a href='#SkImage_height'>negative</a>.
<a href='#SkImage_height'>Returns</a> <a href='#SkImage_height'>false</a> <a href='#SkImage_height'>if</a> <code><a href='undocumented#abs()'>abs</a>(<a href='#SkImage_readPixels_srcX'>srcX</a>) >= <a href='SkImage_Reference#Image'>Image</a> <a href='#SkImage_width'>width()</a></code>, or if <code><a href='undocumented#abs()'>abs</a>(<a href='#SkImage_readPixels_srcY'>srcY</a>) >= <a href='SkImage_Reference#Image'>Image</a> <a href='#SkImage_height'>height()</a></code>.

If <a href='#SkImage_readPixels_cachingHint'>cachingHint</a> <a href='#SkImage_readPixels_cachingHint'>is</a> <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>, <a href='#SkImage_kAllow_CachingHint'>pixels</a> <a href='#SkImage_kAllow_CachingHint'>may</a> <a href='#SkImage_kAllow_CachingHint'>be</a> <a href='#SkImage_kAllow_CachingHint'>retained</a> <a href='#SkImage_kAllow_CachingHint'>locally</a>.
<a href='#SkImage_kAllow_CachingHint'>If</a> <a href='#SkImage_readPixels_cachingHint'>cachingHint</a> <a href='#SkImage_readPixels_cachingHint'>is</a> <a href='#SkImage_kDisallow_CachingHint'>kDisallow_CachingHint</a>, <a href='#SkImage_kDisallow_CachingHint'>pixels</a> <a href='#SkImage_kDisallow_CachingHint'>are</a> <a href='#SkImage_kDisallow_CachingHint'>not</a> <a href='#SkImage_kDisallow_CachingHint'>added</a> <a href='#SkImage_kDisallow_CachingHint'>to</a> <a href='#SkImage_kDisallow_CachingHint'>the</a> <a href='#SkImage_kDisallow_CachingHint'>local</a> <a href='#SkImage_kDisallow_CachingHint'>cache</a>.

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
bool <a href='#SkImage_readPixels'>readPixels</a>(<a href='#SkImage_readPixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#SkPixmap'>dst</a>, <a href='SkPixmap_Reference#SkPixmap'>int</a> <a href='SkPixmap_Reference#SkPixmap'>srcX</a>, <a href='SkPixmap_Reference#SkPixmap'>int</a> <a href='SkPixmap_Reference#SkPixmap'>srcY</a>, <a href='#SkImage_CachingHint'>CachingHint</a> <a href='#SkImage_CachingHint'>cachingHint</a> = <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>) <a href='#SkImage_kAllow_CachingHint'>const</a>
</pre>

Copies a <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>pixels</a> <a href='SkRect_Reference#Rect'>from</a> <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>to</a> <a href='#SkImage_readPixels_2_dst'>dst</a>. <a href='#SkImage_readPixels_2_dst'>Copy</a> <a href='#SkImage_readPixels_2_dst'>starts</a> <a href='#SkImage_readPixels_2_dst'>at</a> (<a href='#SkImage_readPixels_2_srcX'>srcX</a>, <a href='#SkImage_readPixels_2_srcY'>srcY</a>), <a href='#SkImage_readPixels_2_srcY'>and</a>
<a href='#SkImage_readPixels_2_srcY'>does</a> <a href='#SkImage_readPixels_2_srcY'>not</a> <a href='#SkImage_readPixels_2_srcY'>exceed</a> <a href='SkImage_Reference#Image'>Image</a> (<a href='#SkImage_width'>width()</a>, <a href='#SkImage_height'>height()</a>).

<a href='#SkImage_readPixels_2_dst'>dst</a> <a href='#SkImage_readPixels_2_dst'>specifies</a> <a href='#SkImage_readPixels_2_dst'>width</a>, <a href='#SkImage_readPixels_2_dst'>height</a>, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, <a href='#Color_Space'>Color_Space</a>,  <a href='undocumented#Pixel_Storage'>pixel storage</a>,
<a href='undocumented#Pixel'>and</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>bytes</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>destination</a>. <a href='#SkImage_readPixels_2_dst'>dst</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() <a href='#SkPixmap_rowBytes'>specifics</a> <a href='#SkPixmap_rowBytes'>the</a> <a href='#SkPixmap_rowBytes'>gap</a> <a href='#SkPixmap_rowBytes'>from</a> <a href='#SkPixmap_rowBytes'>one</a> <a href='#SkPixmap_rowBytes'>destination</a>
<a href='#SkPixmap_rowBytes'>row</a> <a href='#SkPixmap_rowBytes'>to</a> <a href='#SkPixmap_rowBytes'>the</a> <a href='#SkPixmap_rowBytes'>next</a>. <a href='#SkPixmap_rowBytes'>Returns</a> <a href='#SkPixmap_rowBytes'>true</a> <a href='#SkPixmap_rowBytes'>if</a> <a href='#SkPixmap_rowBytes'>pixels</a> <a href='#SkPixmap_rowBytes'>are</a> <a href='#SkPixmap_rowBytes'>copied</a>. <a href='#SkPixmap_rowBytes'>Returns</a> <a href='#SkPixmap_rowBytes'>false</a> <a href='#SkPixmap_rowBytes'>if</a>:

<table>  <tr>
    <td><a href='#SkImage_readPixels_2_dst'>dst</a>  <a href='undocumented#Pixel_Storage'>pixel storage</a> <a href='undocumented#Pixel'>equals</a> <a href='undocumented#Pixel'>nullptr</a></td>
  </tr>  <tr>
    <td><a href='#SkImage_readPixels_2_dst'>dst</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() <a href='#SkPixmap_rowBytes'>is</a> <a href='#SkPixmap_rowBytes'>less</a> <a href='#SkPixmap_rowBytes'>than</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_minRowBytes'>minRowBytes</a></td>
  </tr>  <tr>
    <td><a href='#Pixel_Ref'>Pixel_Ref</a> <a href='#Pixel_Ref'>is</a> <a href='#Pixel_Ref'>nullptr</a></td>
  </tr>
</table>

Pixels are copied only if <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>possible</a>. <a href='undocumented#Pixel'>If</a> <a href='SkImage_Reference#Image'>Image</a> <a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Image_Info_Color_Type'>is</a>
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kGray_8_SkColorType'>or</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkImage_readPixels_2_dst'>dst</a>.<a href='#SkPixmap_colorType'>colorType</a>() <a href='#SkPixmap_colorType'>must</a> <a href='#SkPixmap_colorType'>match</a>.
<a href='#SkPixmap_colorType'>If</a> <a href='SkImage_Reference#Image'>Image</a> <a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Image_Info_Color_Type'>is</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkImage_readPixels_2_dst'>dst</a>.<a href='#SkPixmap_colorSpace'>colorSpace</a>() <a href='#SkPixmap_colorSpace'>must</a> <a href='#SkPixmap_colorSpace'>match</a>.
<a href='#SkPixmap_colorSpace'>If</a> <a href='SkImage_Reference#Image'>Image</a> <a href='#Image_Info_Alpha_Type'>Alpha_Type</a> <a href='#Image_Info_Alpha_Type'>is</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkImage_readPixels_2_dst'>dst</a>.<a href='#SkPixmap_alphaType'>alphaType</a>() <a href='#SkPixmap_alphaType'>must</a>
<a href='#SkPixmap_alphaType'>match</a>. <a href='#SkPixmap_alphaType'>If</a> <a href='SkImage_Reference#Image'>Image</a> <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>is</a> <a href='#Color_Space'>nullptr</a>, <a href='#SkImage_readPixels_2_dst'>dst</a>.<a href='#SkPixmap_colorSpace'>colorSpace</a>() <a href='#SkPixmap_colorSpace'>must</a> <a href='#SkPixmap_colorSpace'>match</a>. <a href='#SkPixmap_colorSpace'>Returns</a>
<a href='#SkPixmap_colorSpace'>false</a> <a href='#SkPixmap_colorSpace'>if</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>not</a> <a href='undocumented#Pixel'>possible</a>.

<a href='#SkImage_readPixels_2_srcX'>srcX</a> <a href='#SkImage_readPixels_2_srcX'>and</a> <a href='#SkImage_readPixels_2_srcY'>srcY</a> <a href='#SkImage_readPixels_2_srcY'>may</a> <a href='#SkImage_readPixels_2_srcY'>be</a> <a href='#SkImage_readPixels_2_srcY'>negative</a> <a href='#SkImage_readPixels_2_srcY'>to</a> <a href='#SkImage_readPixels_2_srcY'>copy</a> <a href='#SkImage_readPixels_2_srcY'>only</a> <a href='#SkImage_readPixels_2_srcY'>top</a> <a href='#SkImage_readPixels_2_srcY'>or</a> <a href='#SkImage_readPixels_2_srcY'>left</a> <a href='#SkImage_readPixels_2_srcY'>of</a> <a href='#SkImage_readPixels_2_srcY'>source</a>. <a href='#SkImage_readPixels_2_srcY'>Returns</a>
<a href='#SkImage_readPixels_2_srcY'>false</a> <a href='#SkImage_readPixels_2_srcY'>if</a> <a href='#SkImage_width'>width()</a> <a href='#SkImage_width'>or</a> <a href='#SkImage_height'>height()</a> <a href='#SkImage_height'>is</a> <a href='#SkImage_height'>zero</a> <a href='#SkImage_height'>or</a> <a href='#SkImage_height'>negative</a>.
<a href='#SkImage_height'>Returns</a> <a href='#SkImage_height'>false</a> <a href='#SkImage_height'>if</a> <code><a href='undocumented#abs()'>abs</a>(<a href='#SkImage_readPixels_2_srcX'>srcX</a>) >= <a href='SkImage_Reference#Image'>Image</a> <a href='#SkImage_width'>width()</a></code>, or if <code><a href='undocumented#abs()'>abs</a>(<a href='#SkImage_readPixels_2_srcY'>srcY</a>) >= <a href='SkImage_Reference#Image'>Image</a> <a href='#SkImage_height'>height()</a></code>.

If <a href='#SkImage_readPixels_2_cachingHint'>cachingHint</a> <a href='#SkImage_readPixels_2_cachingHint'>is</a> <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>, <a href='#SkImage_kAllow_CachingHint'>pixels</a> <a href='#SkImage_kAllow_CachingHint'>may</a> <a href='#SkImage_kAllow_CachingHint'>be</a> <a href='#SkImage_kAllow_CachingHint'>retained</a> <a href='#SkImage_kAllow_CachingHint'>locally</a>.
<a href='#SkImage_kAllow_CachingHint'>If</a> <a href='#SkImage_readPixels_2_cachingHint'>cachingHint</a> <a href='#SkImage_readPixels_2_cachingHint'>is</a> <a href='#SkImage_kDisallow_CachingHint'>kDisallow_CachingHint</a>, <a href='#SkImage_kDisallow_CachingHint'>pixels</a> <a href='#SkImage_kDisallow_CachingHint'>are</a> <a href='#SkImage_kDisallow_CachingHint'>not</a> <a href='#SkImage_kDisallow_CachingHint'>added</a> <a href='#SkImage_kDisallow_CachingHint'>to</a> <a href='#SkImage_kDisallow_CachingHint'>the</a> <a href='#SkImage_kDisallow_CachingHint'>local</a> <a href='#SkImage_kDisallow_CachingHint'>cache</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_readPixels_2_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkPixmap_Reference#Pixmap'>Pixmap</a>: <a href='#Image_Info'>Image_Info</a>, <a href='#Image_Info'>pixels</a>, <a href='#Image_Info'>row</a> <a href='#Image_Info'>bytes</a></td>
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
bool <a href='#SkImage_scalePixels'>scalePixels</a>(<a href='#SkImage_scalePixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#SkPixmap'>dst</a>, <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> <a href='undocumented#SkFilterQuality'>filterQuality</a>,
                 <a href='#SkImage_CachingHint'>CachingHint</a> <a href='#SkImage_CachingHint'>cachingHint</a> = <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>) <a href='#SkImage_kAllow_CachingHint'>const</a>
</pre>

Copies <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>to</a> <a href='#SkImage_scalePixels_dst'>dst</a>, <a href='#SkImage_scalePixels_dst'>scaling</a> <a href='#SkImage_scalePixels_dst'>pixels</a> <a href='#SkImage_scalePixels_dst'>to</a> <a href='#SkImage_scalePixels_dst'>fit</a> <a href='#SkImage_scalePixels_dst'>dst</a>.<a href='#SkPixmap_width'>width()</a> <a href='#SkPixmap_width'>and</a> <a href='#SkImage_scalePixels_dst'>dst</a>.<a href='#SkPixmap_height'>height()</a>, <a href='#SkPixmap_height'>and</a>
converting pixels to match <a href='#SkImage_scalePixels_dst'>dst</a>.<a href='#SkPixmap_colorType'>colorType</a>() <a href='#SkPixmap_colorType'>and</a> <a href='#SkImage_scalePixels_dst'>dst</a>.<a href='#SkPixmap_alphaType'>alphaType</a>(). <a href='#SkPixmap_alphaType'>Returns</a> <a href='#SkPixmap_alphaType'>true</a> <a href='#SkPixmap_alphaType'>if</a>
pixels are copied. Returns false if <a href='#SkImage_scalePixels_dst'>dst</a>.<a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_addr'>is</a> <a href='#SkPixmap_addr'>nullptr</a>, <a href='#SkPixmap_addr'>or</a> <a href='#SkImage_scalePixels_dst'>dst</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() <a href='#SkPixmap_rowBytes'>is</a>
less than <a href='#SkImage_scalePixels_dst'>dst</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>.

Pixels are copied only if <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>possible</a>. <a href='undocumented#Pixel'>If</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a>
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kGray_8_SkColorType'>or</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkImage_scalePixels_dst'>dst</a>.<a href='#SkPixmap_colorType'>colorType</a>() <a href='#SkPixmap_colorType'>must</a> <a href='#SkPixmap_colorType'>match</a>.
If <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkImage_scalePixels_dst'>dst</a>.<a href='#SkPixmap_colorSpace'>colorSpace</a>() <a href='#SkPixmap_colorSpace'>must</a> <a href='#SkPixmap_colorSpace'>match</a>.
If <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>is</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkImage_scalePixels_dst'>dst</a>.<a href='#SkPixmap_alphaType'>alphaType</a>() <a href='#SkPixmap_alphaType'>must</a>
match. If <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>is</a> <a href='undocumented#SkColorSpace'>nullptr</a>, <a href='#SkImage_scalePixels_dst'>dst</a>.<a href='#SkPixmap_colorSpace'>colorSpace</a>() <a href='#SkPixmap_colorSpace'>must</a> <a href='#SkPixmap_colorSpace'>match</a>. <a href='#SkPixmap_colorSpace'>Returns</a>
false if <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>not</a> <a href='undocumented#Pixel'>possible</a>.

Scales the <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>with</a> <a href='#SkImage_scalePixels_filterQuality'>filterQuality</a>, <a href='#SkImage_scalePixels_filterQuality'>to</a> <a href='#SkImage_scalePixels_filterQuality'>match</a> <a href='#SkImage_scalePixels_dst'>dst</a>.<a href='#SkPixmap_width'>width()</a> <a href='#SkPixmap_width'>and</a> <a href='#SkImage_scalePixels_dst'>dst</a>.<a href='#SkPixmap_height'>height()</a>.
<a href='#SkImage_scalePixels_filterQuality'>filterQuality</a> <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a> <a href='undocumented#kNone_SkFilterQuality'>is</a> <a href='undocumented#kNone_SkFilterQuality'>fastest</a>, <a href='undocumented#kNone_SkFilterQuality'>typically</a> <a href='undocumented#kNone_SkFilterQuality'>implemented</a> <a href='undocumented#kNone_SkFilterQuality'>with</a>
<a href='undocumented#Nearest_Neighbor'>nearest neighbor filter</a>. <a href='undocumented#kLow_SkFilterQuality'>kLow_SkFilterQuality</a> <a href='undocumented#kLow_SkFilterQuality'>is</a> <a href='undocumented#kLow_SkFilterQuality'>typically</a> <a href='undocumented#kLow_SkFilterQuality'>implemented</a> <a href='undocumented#kLow_SkFilterQuality'>with</a>
<a href='undocumented#Bilerp'>bilerp filter</a>. <a href='undocumented#kMedium_SkFilterQuality'>kMedium_SkFilterQuality</a> <a href='undocumented#kMedium_SkFilterQuality'>is</a> <a href='undocumented#kMedium_SkFilterQuality'>typically</a> <a href='undocumented#kMedium_SkFilterQuality'>implemented</a> <a href='undocumented#kMedium_SkFilterQuality'>with</a>
<a href='undocumented#Bilerp'>bilerp filter</a>, and  <a href='undocumented#MipMap'>mip-map filter</a> when <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>is</a> <a href='undocumented#Size'>reduced</a>.
<a href='undocumented#kHigh_SkFilterQuality'>kHigh_SkFilterQuality</a> <a href='undocumented#kHigh_SkFilterQuality'>is</a> <a href='undocumented#kHigh_SkFilterQuality'>slowest</a>, <a href='undocumented#kHigh_SkFilterQuality'>typically</a> <a href='undocumented#kHigh_SkFilterQuality'>implemented</a> <a href='undocumented#kHigh_SkFilterQuality'>with</a>  <a href='undocumented#BiCubic'>bicubic filter</a>.

If <a href='#SkImage_scalePixels_cachingHint'>cachingHint</a> <a href='#SkImage_scalePixels_cachingHint'>is</a> <a href='#SkImage_kAllow_CachingHint'>kAllow_CachingHint</a>, <a href='#SkImage_kAllow_CachingHint'>pixels</a> <a href='#SkImage_kAllow_CachingHint'>may</a> <a href='#SkImage_kAllow_CachingHint'>be</a> <a href='#SkImage_kAllow_CachingHint'>retained</a> <a href='#SkImage_kAllow_CachingHint'>locally</a>.
If <a href='#SkImage_scalePixels_cachingHint'>cachingHint</a> <a href='#SkImage_scalePixels_cachingHint'>is</a> <a href='#SkImage_kDisallow_CachingHint'>kDisallow_CachingHint</a>, <a href='#SkImage_kDisallow_CachingHint'>pixels</a> <a href='#SkImage_kDisallow_CachingHint'>are</a> <a href='#SkImage_kDisallow_CachingHint'>not</a> <a href='#SkImage_kDisallow_CachingHint'>added</a> <a href='#SkImage_kDisallow_CachingHint'>to</a> <a href='#SkImage_kDisallow_CachingHint'>the</a> <a href='#SkImage_kDisallow_CachingHint'>local</a> <a href='#SkImage_kDisallow_CachingHint'>cache</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_scalePixels_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>: <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>pixels</a>, <a href='SkImageInfo_Reference#SkImageInfo'>row</a> <a href='SkImageInfo_Reference#SkImageInfo'>bytes</a></td>
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
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkData'>SkData</a>&<a href='undocumented#SkData'>gt</a>; <a href='#SkImage_encodeToData'>encodeToData</a>(<a href='undocumented#SkEncodedImageFormat'>SkEncodedImageFormat</a> <a href='undocumented#SkEncodedImageFormat'>encodedImageFormat</a>, <a href='undocumented#SkEncodedImageFormat'>int</a> <a href='undocumented#SkEncodedImageFormat'>quality</a>) <a href='undocumented#SkEncodedImageFormat'>const</a>
</pre>

Encodes <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>pixels</a>, <a href='SkImage_Reference#SkImage'>returning</a> <a href='SkImage_Reference#SkImage'>result</a> <a href='SkImage_Reference#SkImage'>as</a> <a href='undocumented#SkData'>SkData</a>.

Returns nullptr if encoding fails, or if <a href='#SkImage_encodeToData_encodedImageFormat'>encodedImageFormat</a> <a href='#SkImage_encodeToData_encodedImageFormat'>is</a> <a href='#SkImage_encodeToData_encodedImageFormat'>not</a> <a href='#SkImage_encodeToData_encodedImageFormat'>supported</a>.

<a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>encoding</a> <a href='SkImage_Reference#SkImage'>in</a> <a href='SkImage_Reference#SkImage'>a</a> <a href='SkImage_Reference#SkImage'>format</a> <a href='SkImage_Reference#SkImage'>requires</a> <a href='SkImage_Reference#SkImage'>both</a> <a href='SkImage_Reference#SkImage'>building</a> <a href='SkImage_Reference#SkImage'>with</a> <a href='SkImage_Reference#SkImage'>one</a> <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>more</a> <a href='SkImage_Reference#SkImage'>of</a>:
SK_HAS_JPEG_LIBRARY, SK_HAS_PNG_LIBRARY, SK_HAS_WEBP_LIBRARY; and platform support
for the encoded format.

If SK_BUILD_FOR_MAC or SK_BUILD_FOR_IOS is defined, <a href='#SkImage_encodeToData_encodedImageFormat'>encodedImageFormat</a> <a href='#SkImage_encodeToData_encodedImageFormat'>can</a>
additionally be one of: <a href='undocumented#SkEncodedImageFormat'>SkEncodedImageFormat</a>::<a href='#SkEncodedImageFormat_kICO'>kICO</a>, <a href='undocumented#SkEncodedImageFormat'>SkEncodedImageFormat</a>::<a href='#SkEncodedImageFormat_kBMP'>kBMP</a>,
<a href='undocumented#SkEncodedImageFormat'>SkEncodedImageFormat</a>::<a href='#SkEncodedImageFormat_kGIF'>kGIF</a>.

<a href='#SkImage_encodeToData_quality'>quality</a> <a href='#SkImage_encodeToData_quality'>is</a> <a href='#SkImage_encodeToData_quality'>a</a> <a href='#SkImage_encodeToData_quality'>platform</a> <a href='#SkImage_encodeToData_quality'>and</a> <a href='#SkImage_encodeToData_quality'>format</a> <a href='#SkImage_encodeToData_quality'>specific</a> <a href='#SkImage_encodeToData_quality'>metric</a> <a href='#SkImage_encodeToData_quality'>trading</a> <a href='#SkImage_encodeToData_quality'>off</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>and</a> <a href='undocumented#Size'>encoding</a>
error. When used, <a href='#SkImage_encodeToData_quality'>quality</a> <a href='#SkImage_encodeToData_quality'>equaling</a> 100 <a href='#SkImage_encodeToData_quality'>encodes</a> <a href='#SkImage_encodeToData_quality'>with</a> <a href='#SkImage_encodeToData_quality'>the</a> <a href='#SkImage_encodeToData_quality'>least</a> <a href='#SkImage_encodeToData_quality'>error</a>. <a href='#SkImage_encodeToData_quality'>quality</a> <a href='#SkImage_encodeToData_quality'>may</a>
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

encoded <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>

### Example

<div><fiddle-embed name="7a3bf8851bb7160e4e49c48f8c09639d"></fiddle-embed></div>

### See Also

<a href='#SkImage_refEncodedData'>refEncodedData</a> <a href='#SkImage_MakeFromEncoded'>MakeFromEncoded</a>

<a name='SkImage_encodeToData_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkData'>SkData</a>&<a href='undocumented#SkData'>gt</a>; <a href='#SkImage_encodeToData'>encodeToData</a>() <a href='#SkImage_encodeToData'>const</a>
</pre>

Encodes <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>pixels</a>, <a href='SkImage_Reference#SkImage'>returning</a> <a href='SkImage_Reference#SkImage'>result</a> <a href='SkImage_Reference#SkImage'>as</a> <a href='undocumented#SkData'>SkData</a>. <a href='undocumented#SkData'>Returns</a> <a href='undocumented#SkData'>existing</a> <a href='undocumented#SkData'>encoded</a> <a href='undocumented#Data'>data</a>
if present; otherwise, <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>encoded</a> <a href='SkImage_Reference#SkImage'>with</a> <a href='undocumented#SkEncodedImageFormat'>SkEncodedImageFormat</a>::<a href='#SkEncodedImageFormat_kPNG'>kPNG</a>. <a href='#SkEncodedImageFormat_kPNG'>Skia</a>
must be built with SK_HAS_PNG_LIBRARY to encode <a href='SkImage_Reference#SkImage'>SkImage</a>.

Returns nullptr if existing encoded <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>is</a> <a href='undocumented#Data'>missing</a> <a href='undocumented#Data'>or</a> <a href='undocumented#Data'>invalid</a>, <a href='undocumented#Data'>and</a>
encoding fails.

### Return Value

encoded <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>

### Example

<div><fiddle-embed name="30cee813f6aa476b0a9c8a24283e53a3"></fiddle-embed></div>

### See Also

<a href='#SkImage_refEncodedData'>refEncodedData</a> <a href='#SkImage_MakeFromEncoded'>MakeFromEncoded</a>

<a name='SkImage_refEncodedData'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkData'>SkData</a>&<a href='undocumented#SkData'>gt</a>; <a href='#SkImage_refEncodedData'>refEncodedData</a>() <a href='#SkImage_refEncodedData'>const</a>
</pre>

Returns encoded <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>pixels</a> <a href='SkImage_Reference#SkImage'>as</a> <a href='undocumented#SkData'>SkData</a>, <a href='undocumented#SkData'>if</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>was</a> <a href='SkImage_Reference#SkImage'>created</a> <a href='SkImage_Reference#SkImage'>from</a> <a href='SkImage_Reference#SkImage'>supported</a>
encoded <a href='SkStream_Reference#Stream'>stream</a> <a href='SkStream_Reference#Stream'>format</a>. <a href='SkStream_Reference#Stream'>Platform</a> <a href='SkStream_Reference#Stream'>support</a> <a href='SkStream_Reference#Stream'>for</a> <a href='SkStream_Reference#Stream'>formats</a> <a href='SkStream_Reference#Stream'>vary</a> <a href='SkStream_Reference#Stream'>and</a> <a href='SkStream_Reference#Stream'>may</a> <a href='SkStream_Reference#Stream'>require</a> <a href='SkStream_Reference#Stream'>building</a>
with one or more of: SK_HAS_JPEG_LIBRARY, SK_HAS_PNG_LIBRARY, SK_HAS_WEBP_LIBRARY.

Returns nullptr if <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>contents</a> <a href='SkImage_Reference#SkImage'>are</a> <a href='SkImage_Reference#SkImage'>not</a> <a href='SkImage_Reference#SkImage'>encoded</a>.

### Return Value

encoded <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>

### Example

<div><fiddle-embed name="80856fe921ce36f8d5a32d8672bccbfc" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_encodeToData'>encodeToData</a> <a href='#SkImage_MakeFromEncoded'>MakeFromEncoded</a>

<a name='Utility'></a>

<a name='SkImage_makeSubset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_makeSubset'>makeSubset</a>(<a href='#SkImage_makeSubset'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>subset</a>) <a href='SkIRect_Reference#SkIRect'>const</a>
</pre>

Returns <a href='#SkImage_makeSubset_subset'>subset</a> <a href='#SkImage_makeSubset_subset'>of</a> <a href='SkImage_Reference#SkImage'>SkImage</a>. <a href='#SkImage_makeSubset_subset'>subset</a> <a href='#SkImage_makeSubset_subset'>must</a> <a href='#SkImage_makeSubset_subset'>be</a> <a href='#SkImage_makeSubset_subset'>fully</a> <a href='#SkImage_makeSubset_subset'>contained</a> <a href='#SkImage_makeSubset_subset'>by</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='#SkImage_dimensions'>dimensions()</a>.
The implementation may share pixels, or may copy them.

Returns nullptr if <a href='#SkImage_makeSubset_subset'>subset</a> <a href='#SkImage_makeSubset_subset'>is</a> <a href='#SkImage_makeSubset_subset'>empty</a>, <a href='#SkImage_makeSubset_subset'>or</a> <a href='#SkImage_makeSubset_subset'>subset</a> <a href='#SkImage_makeSubset_subset'>is</a> <a href='#SkImage_makeSubset_subset'>not</a> <a href='#SkImage_makeSubset_subset'>contained</a> <a href='#SkImage_makeSubset_subset'>by</a> <a href='#SkImage_makeSubset_subset'>bounds</a>, <a href='#SkImage_makeSubset_subset'>or</a>
pixels in <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>could</a> <a href='SkImage_Reference#SkImage'>not</a> <a href='SkImage_Reference#SkImage'>be</a> <a href='SkImage_Reference#SkImage'>read</a> <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>copied</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_makeSubset_subset'><code><strong>subset</strong></code></a></td>
    <td>bounds of returned <a href='SkImage_Reference#SkImage'>SkImage</a></td>
  </tr>
</table>

### Return Value

partial or full <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>

### Example

<div><fiddle-embed name="889e495ce3e3b3bacc96e8230932331c"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromEncoded'>MakeFromEncoded</a>

<a name='SkImage_makeTextureImage'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_makeTextureImage'>makeTextureImage</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='undocumented#SkColorSpace'>dstColorSpace</a>,
                                <a href='undocumented#GrMipMapped'>GrMipMapped</a> <a href='undocumented#GrMipMapped'>mipMapped</a> = <a href='undocumented#GrMipMapped'>GrMipMapped</a>::<a href='#GrMipMapped_kNo'>kNo</a>) <a href='#GrMipMapped_kNo'>const</a>
</pre>

Returns <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>backed</a> <a href='SkImage_Reference#SkImage'>by</a>  <a href='undocumented#GPU_Texture'>GPU texture</a> <a href='SkImage_Reference#SkImage'>associated</a> <a href='SkImage_Reference#SkImage'>with</a> <a href='#SkImage_makeTextureImage_context'>context</a>. <a href='#SkImage_makeTextureImage_context'>Returned</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a>
compatible with <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>created</a> <a href='SkSurface_Reference#SkSurface'>with</a> <a href='#SkImage_makeTextureImage_dstColorSpace'>dstColorSpace</a>. <a href='#SkImage_makeTextureImage_dstColorSpace'>The</a> <a href='#SkImage_makeTextureImage_dstColorSpace'>returned</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>respects</a>
<a href='#SkImage_makeTextureImage_mipMapped'>mipMapped</a> <a href='#SkImage_makeTextureImage_mipMapped'>setting</a>; <a href='#SkImage_makeTextureImage_mipMapped'>if</a> <a href='#SkImage_makeTextureImage_mipMapped'>mipMapped</a> <a href='#SkImage_makeTextureImage_mipMapped'>equals</a> <a href='undocumented#GrMipMapped'>GrMipMapped</a>::<a href='#GrMipMapped_kYes'>kYes</a>, <a href='#GrMipMapped_kYes'>the</a> <a href='#GrMipMapped_kYes'>backing</a> <a href='undocumented#Texture'>texture</a>
allocates  <a href='undocumented#Mip_Map'>mip map</a> levels. Returns original <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>if</a> <a href='#SkImage_makeTextureImage_context'>context</a>
and <a href='#SkImage_makeTextureImage_dstColorSpace'>dstColorSpace</a> <a href='#SkImage_makeTextureImage_dstColorSpace'>match</a> <a href='#SkImage_makeTextureImage_dstColorSpace'>and</a> <a href='#SkImage_makeTextureImage_mipMapped'>mipMapped</a> <a href='#SkImage_makeTextureImage_mipMapped'>is</a> <a href='#SkImage_makeTextureImage_mipMapped'>compatible</a> <a href='#SkImage_makeTextureImage_mipMapped'>with</a> <a href='#SkImage_makeTextureImage_mipMapped'>backing</a>  <a href='undocumented#GPU_Texture'>GPU texture</a>.

Returns nullptr if <a href='#SkImage_makeTextureImage_context'>context</a> <a href='#SkImage_makeTextureImage_context'>is</a> <a href='#SkImage_makeTextureImage_context'>nullptr</a>, <a href='#SkImage_makeTextureImage_context'>or</a> <a href='#SkImage_makeTextureImage_context'>if</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>was</a> <a href='SkImage_Reference#SkImage'>created</a> <a href='SkImage_Reference#SkImage'>with</a> <a href='SkImage_Reference#SkImage'>another</a>
<a href='undocumented#GrContext'>GrContext</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_makeTextureImage_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_makeTextureImage_dstColorSpace'><code><strong>dstColorSpace</strong></code></a></td>
    <td>range of colors of matching <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>on</a> <a href='SkSurface_Reference#SkSurface'>GPU</a></td>
  </tr>
  <tr>    <td><a name='SkImage_makeTextureImage_mipMapped'><code><strong>mipMapped</strong></code></a></td>
    <td>whether created <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>must</a> <a href='undocumented#Texture'>allocate</a>  <a href='undocumented#Mip_Map'>mip map</a> <a href='undocumented#Texture'>levels</a></td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>

### Example

<div><fiddle-embed name="eeec9e07e604b44d0208899a2fe5bef5" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromTexture'>MakeFromTexture</a>

<a name='SkImage_makeNonTextureImage'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_makeNonTextureImage'>makeNonTextureImage</a>() <a href='#SkImage_makeNonTextureImage'>const</a>
</pre>

Returns raster <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>or</a> <a href='SkImage_Reference#Image'>lazy</a> <a href='SkImage_Reference#Image'>image</a>. <a href='SkImage_Reference#Image'>Copies</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>backed</a> <a href='SkImage_Reference#SkImage'>by</a> <a href='SkImage_Reference#SkImage'>GPU</a> <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>into</a>
CPU memory if needed. Returns original <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>if</a> <a href='SkImage_Reference#SkImage'>decoded</a> <a href='SkImage_Reference#SkImage'>in</a> <a href='SkImage_Reference#SkImage'>raster</a> <a href='SkBitmap_Reference#Bitmap'>bitmap</a>,
or if encoded in a <a href='SkStream_Reference#Stream'>stream</a>.

Returns nullptr if backed by GPU <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>and</a> <a href='undocumented#Texture'>copy</a> <a href='undocumented#Texture'>fails</a>.

### Return Value

raster <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>lazy</a> <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>or</a> <a href='SkImage_Reference#Image'>nullptr</a>

### Example

<div><fiddle-embed name="ecdbaff44a02c310ef672b7d393c6dea" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_makeTextureImage'>makeTextureImage</a> <a href='#SkImage_makeRasterImage'>makeRasterImage</a> <a href='#SkImage_MakeBackendTextureFromSkImage'>MakeBackendTextureFromSkImage</a>

<a name='SkImage_makeRasterImage'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_makeRasterImage'>makeRasterImage</a>() <a href='#SkImage_makeRasterImage'>const</a>
</pre>

Returns raster <a href='SkImage_Reference#Image'>image</a>. <a href='SkImage_Reference#Image'>Copies</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>backed</a> <a href='SkImage_Reference#SkImage'>by</a> <a href='SkImage_Reference#SkImage'>GPU</a> <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>into</a> <a href='undocumented#Texture'>CPU</a> <a href='undocumented#Texture'>memory</a>,
or decodes <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>from</a> <a href='SkImage_Reference#SkImage'>lazy</a> <a href='SkImage_Reference#Image'>image</a>. <a href='SkImage_Reference#Image'>Returns</a> <a href='SkImage_Reference#Image'>original</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>if</a> <a href='SkImage_Reference#SkImage'>decoded</a> <a href='SkImage_Reference#SkImage'>in</a>
raster <a href='SkBitmap_Reference#Bitmap'>bitmap</a>.

Returns nullptr if copy, decode, or <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>read</a> <a href='undocumented#Pixel'>fails</a>.

### Return Value

raster <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>or</a> <a href='SkImage_Reference#Image'>nullptr</a>

### Example

<div><fiddle-embed name="aed5f399915d40bb5d133ab586e5bac3" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_isTextureBacked'>isTextureBacked</a> <a href='#SkImage_isLazyGenerated'>isLazyGenerated</a> <a href='#SkImage_MakeFromRaster'>MakeFromRaster</a>

<a name='SkImage_makeWithFilter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_makeWithFilter'>makeWithFilter</a>(<a href='#SkImage_makeWithFilter'>const</a> <a href='undocumented#SkImageFilter'>SkImageFilter</a>* <a href='undocumented#SkImageFilter'>filter</a>, <a href='undocumented#SkImageFilter'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>subset</a>,
                              <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>clipBounds</a>, <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* <a href='SkIRect_Reference#SkIRect'>outSubset</a>, <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>* <a href='SkIPoint_Reference#SkIPoint'>offset</a>) <a href='SkIPoint_Reference#SkIPoint'>const</a>
</pre>

Creates filtered <a href='SkImage_Reference#SkImage'>SkImage</a>. <a href='#SkImage_makeWithFilter_filter'>filter</a> <a href='#SkImage_makeWithFilter_filter'>processes</a> <a href='#SkImage_makeWithFilter_filter'>original</a> <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>potentially</a> <a href='SkImage_Reference#SkImage'>changing</a>
<a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>position</a>, <a href='SkColor_Reference#Color'>and</a> <a href='undocumented#Size'>size</a>. <a href='#SkImage_makeWithFilter_subset'>subset</a> <a href='#SkImage_makeWithFilter_subset'>is</a> <a href='#SkImage_makeWithFilter_subset'>the</a> <a href='#SkImage_makeWithFilter_subset'>bounds</a> <a href='#SkImage_makeWithFilter_subset'>of</a> <a href='#SkImage_makeWithFilter_subset'>original</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>processed</a>
by <a href='#SkImage_makeWithFilter_filter'>filter</a>. <a href='#SkImage_makeWithFilter_clipBounds'>clipBounds</a> <a href='#SkImage_makeWithFilter_clipBounds'>is</a> <a href='#SkImage_makeWithFilter_clipBounds'>the</a> <a href='#SkImage_makeWithFilter_clipBounds'>expected</a> <a href='#SkImage_makeWithFilter_clipBounds'>bounds</a> <a href='#SkImage_makeWithFilter_clipBounds'>of</a> <a href='#SkImage_makeWithFilter_clipBounds'>the</a> <a href='#SkImage_makeWithFilter_clipBounds'>filtered</a> <a href='SkImage_Reference#SkImage'>SkImage</a>. <a href='#SkImage_makeWithFilter_outSubset'>outSubset</a>
is required storage for the actual bounds of the filtered <a href='SkImage_Reference#SkImage'>SkImage</a>. <a href='#SkImage_makeWithFilter_offset'>offset</a> <a href='#SkImage_makeWithFilter_offset'>is</a>
required storage for translation of returned <a href='SkImage_Reference#SkImage'>SkImage</a>.

Returns nullptr if <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>could</a> <a href='SkImage_Reference#SkImage'>not</a> <a href='SkImage_Reference#SkImage'>be</a> <a href='SkImage_Reference#SkImage'>created</a>. <a href='SkImage_Reference#SkImage'>If</a> <a href='SkImage_Reference#SkImage'>nullptr</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>returned</a>, <a href='#SkImage_makeWithFilter_outSubset'>outSubset</a>
and <a href='#SkImage_makeWithFilter_offset'>offset</a> <a href='#SkImage_makeWithFilter_offset'>are</a> <a href='#SkImage_makeWithFilter_offset'>undefined</a>.

Useful for animation of <a href='undocumented#SkImageFilter'>SkImageFilter</a> <a href='undocumented#SkImageFilter'>that</a> <a href='undocumented#SkImageFilter'>varies</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>from</a> <a href='undocumented#Size'>frame</a> <a href='undocumented#Size'>to</a> <a href='undocumented#Size'>frame</a>.
Returned <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>created</a> <a href='SkImage_Reference#SkImage'>larger</a> <a href='SkImage_Reference#SkImage'>than</a> <a href='SkImage_Reference#SkImage'>required</a> <a href='SkImage_Reference#SkImage'>by</a> <a href='#SkImage_makeWithFilter_filter'>filter</a> <a href='#SkImage_makeWithFilter_filter'>so</a> <a href='#SkImage_makeWithFilter_filter'>that</a>  <a href='undocumented#GPU_Texture'>GPU texture</a>
can be reused with different sized effects. <a href='#SkImage_makeWithFilter_outSubset'>outSubset</a> <a href='#SkImage_makeWithFilter_outSubset'>describes</a> <a href='#SkImage_makeWithFilter_outSubset'>the</a> <a href='#SkImage_makeWithFilter_outSubset'>valid</a> <a href='#SkImage_makeWithFilter_outSubset'>bounds</a>
of  <a href='undocumented#GPU_Texture'>GPU texture</a> returned. <a href='#SkImage_makeWithFilter_offset'>offset</a> <a href='#SkImage_makeWithFilter_offset'>translates</a> <a href='#SkImage_makeWithFilter_offset'>the</a> <a href='#SkImage_makeWithFilter_offset'>returned</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>to</a> <a href='SkImage_Reference#SkImage'>keep</a> <a href='SkImage_Reference#SkImage'>subsequent</a>
animation frames aligned with respect to each other.

### Parameters

<table>  <tr>    <td><a name='SkImage_makeWithFilter_filter'><code><strong>filter</strong></code></a></td>
    <td>how <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>sampled</a> <a href='SkImage_Reference#SkImage'>when</a> <a href='SkImage_Reference#SkImage'>transformed</a></td>
  </tr>
  <tr>    <td><a name='SkImage_makeWithFilter_subset'><code><strong>subset</strong></code></a></td>
    <td>bounds of <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>processed</a> <a href='SkImage_Reference#SkImage'>by</a> <a href='#SkImage_makeWithFilter_filter'>filter</a></td>
  </tr>
  <tr>    <td><a name='SkImage_makeWithFilter_clipBounds'><code><strong>clipBounds</strong></code></a></td>
    <td>expected bounds of filtered <a href='SkImage_Reference#SkImage'>SkImage</a></td>
  </tr>
  <tr>    <td><a name='SkImage_makeWithFilter_outSubset'><code><strong>outSubset</strong></code></a></td>
    <td>storage for returned <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>bounds</a></td>
  </tr>
  <tr>    <td><a name='SkImage_makeWithFilter_offset'><code><strong>offset</strong></code></a></td>
    <td>storage for returned <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>translation</a></td>
  </tr>
</table>

### Return Value

filtered <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>nullptr</a>

### Example

<div><fiddle-embed name="85a76163138a2720ac003691d6363938" gpu="true"><div>In each frame of the animation, filtered <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>is</a> <a href='SkImage_Reference#Image'>drawn</a> <a href='SkImage_Reference#Image'>in</a> <a href='SkImage_Reference#Image'>a</a> <a href='SkImage_Reference#Image'>different</a> <a href='SkImage_Reference#Image'>location</a>.
<a href='SkImage_Reference#Image'>By</a> <a href='SkImage_Reference#Image'>translating</a> <a href='SkCanvas_Reference#Canvas'>canvas</a> <a href='SkCanvas_Reference#Canvas'>by</a> <a href='SkCanvas_Reference#Canvas'>returned</a> <a href='#SkImage_makeWithFilter_offset'>offset</a>, <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>appears</a> <a href='SkImage_Reference#Image'>stationary</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkImage_makeShader'>makeShader</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_setImageFilter'>setImageFilter</a>

<a name='SkImage_BackendTextureReleaseProc'></a>

---

<a href='#SkImage_BackendTextureReleaseProc'>BackendTextureReleaseProc</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    typedef std::function<void(<a href='undocumented#GrBackendTexture'>GrBackendTexture</a>)> <a href='#SkImage_BackendTextureReleaseProc'>BackendTextureReleaseProc</a>;
</pre>

Defines a callback function, taking one parameter of type <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='undocumented#GrBackendTexture'>with</a>
<a href='undocumented#GrBackendTexture'>no</a> <a href='undocumented#GrBackendTexture'>return</a> <a href='undocumented#GrBackendTexture'>value</a>. <a href='undocumented#GrBackendTexture'>Function</a> <a href='undocumented#GrBackendTexture'>is</a> <a href='undocumented#GrBackendTexture'>called</a> <a href='undocumented#GrBackendTexture'>when</a> <a href='undocumented#GrBackendTexture'>back-end</a> <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>is</a> <a href='undocumented#Texture'>to</a> <a href='undocumented#Texture'>be</a> <a href='undocumented#Texture'>released</a>.

<a name='SkImage_MakeBackendTextureFromSkImage'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkImage_MakeBackendTextureFromSkImage'>MakeBackendTextureFromSkImage</a>(<a href='undocumented#GrContext'>GrContext</a>* <a href='undocumented#GrContext'>context</a>, <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='SkImage_Reference#Image'>image</a>,
                                          <a href='undocumented#GrBackendTexture'>GrBackendTexture</a>* <a href='undocumented#GrBackendTexture'>backendTexture</a>,
                                          <a href='#SkImage_BackendTextureReleaseProc'>BackendTextureReleaseProc</a>* <a href='#SkImage_BackendTextureReleaseProc'>backendTextureReleaseProc</a>)
</pre>

Creates a <a href='undocumented#GrBackendTexture'>GrBackendTexture</a> <a href='undocumented#GrBackendTexture'>from</a> <a href='undocumented#GrBackendTexture'>the</a> <a href='undocumented#GrBackendTexture'>provided</a> <a href='SkImage_Reference#SkImage'>SkImage</a>. <a href='SkImage_Reference#SkImage'>Returns</a> <a href='SkImage_Reference#SkImage'>true</a> <a href='SkImage_Reference#SkImage'>and</a>
stores result in <a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>backendTexture</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>and</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTextureReleaseProc'>backendTextureReleaseProc</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTextureReleaseProc'>if</a>
<a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>is</a> <a href='undocumented#Texture'>created</a>; <a href='undocumented#Texture'>otherwise</a>, <a href='undocumented#Texture'>returns</a> <a href='undocumented#Texture'>false</a> <a href='undocumented#Texture'>and</a> <a href='undocumented#Texture'>leaves</a>
<a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>backendTexture</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>and</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTextureReleaseProc'>backendTextureReleaseProc</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTextureReleaseProc'>unmodified</a>.

Call <a href='#SkImage_MakeBackendTextureFromSkImage_backendTextureReleaseProc'>backendTextureReleaseProc</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTextureReleaseProc'>after</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTextureReleaseProc'>deleting</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>backendTexture</a>.
<a href='#SkImage_MakeBackendTextureFromSkImage_backendTextureReleaseProc'>backendTextureReleaseProc</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTextureReleaseProc'>cleans</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTextureReleaseProc'>up</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTextureReleaseProc'>auxiliary</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>related</a> <a href='undocumented#Data'>to</a> <a href='undocumented#Data'>returned</a>
<a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>backendTexture</a>. <a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>The</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>caller</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>must</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>delete</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>returned</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>backendTexture</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>after</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>use</a>.

If <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>both</a> <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>backed</a> <a href='undocumented#Texture'>and</a> <a href='undocumented#Texture'>singly</a> <a href='undocumented#Texture'>referenced</a>, <a href='#SkImage_MakeBackendTextureFromSkImage_image'>image</a> <a href='#SkImage_MakeBackendTextureFromSkImage_image'>is</a> <a href='#SkImage_MakeBackendTextureFromSkImage_image'>returned</a> <a href='#SkImage_MakeBackendTextureFromSkImage_image'>in</a>
<a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>backendTexture</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>without</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>conversion</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>or</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>making</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>a</a> <a href='#SkImage_MakeBackendTextureFromSkImage_backendTexture'>copy</a>. <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>singly</a> <a href='SkImage_Reference#SkImage'>referenced</a>
if its was transferred solely using std::move().

If <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>not</a> <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>backed</a>, <a href='undocumented#Texture'>returns</a> <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>with</a> <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>contents</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_MakeBackendTextureFromSkImage_context'><code><strong>context</strong></code></a></td>
    <td><a href='undocumented#GPU_Context'>GPU context</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeBackendTextureFromSkImage_image'><code><strong>image</strong></code></a></td>
    <td><a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>used</a> <a href='SkImage_Reference#SkImage'>for</a> <a href='undocumented#Texture'>texture</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeBackendTextureFromSkImage_backendTexture'><code><strong>backendTexture</strong></code></a></td>
    <td>storage for back-end <a href='undocumented#Texture'>texture</a></td>
  </tr>
  <tr>    <td><a name='SkImage_MakeBackendTextureFromSkImage_backendTextureReleaseProc'><code><strong>backendTextureReleaseProc</strong></code></a></td>
    <td>storage for clean up function</td>
  </tr>
</table>

### Return Value

true if back-end <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>was</a> <a href='undocumented#Texture'>created</a>

### Example

<div><fiddle-embed name="06aeb3cf63ffccf7b49fe556e5def351" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromTexture'>MakeFromTexture</a> <a href='#SkImage_makeTextureImage'>makeTextureImage</a>

<a name='SkImage_LegacyBitmapMode'></a>

---

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

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_asLegacyBitmap'>asLegacyBitmap</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='#SkImage_LegacyBitmapMode'>LegacyBitmapMode</a> <a href='#SkImage_LegacyBitmapMode'>legacyBitmapMode</a> = <a href='#SkImage_kRO_LegacyBitmapMode'>kRO_LegacyBitmapMode</a>) <a href='#SkImage_kRO_LegacyBitmapMode'>const</a>
</pre>

Creates raster <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>with</a> <a href='SkBitmap_Reference#SkBitmap'>same</a> <a href='SkBitmap_Reference#SkBitmap'>pixels</a> <a href='SkBitmap_Reference#SkBitmap'>as</a> <a href='SkImage_Reference#SkImage'>SkImage</a>. <a href='SkImage_Reference#SkImage'>If</a> <a href='#SkImage_asLegacyBitmap_legacyBitmapMode'>legacyBitmapMode</a> <a href='#SkImage_asLegacyBitmap_legacyBitmapMode'>is</a>
<a href='#SkImage_kRO_LegacyBitmapMode'>kRO_LegacyBitmapMode</a>, <a href='#SkImage_kRO_LegacyBitmapMode'>returned</a> <a href='#SkImage_asLegacyBitmap_bitmap'>bitmap</a> <a href='#SkImage_asLegacyBitmap_bitmap'>is</a> <a href='#SkImage_asLegacyBitmap_bitmap'>read-only</a> <a href='#SkImage_asLegacyBitmap_bitmap'>and</a> <a href='#SkImage_asLegacyBitmap_bitmap'>immutable</a>.
Returns true if <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>is</a> <a href='SkBitmap_Reference#SkBitmap'>stored</a> <a href='SkBitmap_Reference#SkBitmap'>in</a> <a href='#SkImage_asLegacyBitmap_bitmap'>bitmap</a>. <a href='#SkImage_asLegacyBitmap_bitmap'>Returns</a> <a href='#SkImage_asLegacyBitmap_bitmap'>false</a> <a href='#SkImage_asLegacyBitmap_bitmap'>and</a> <a href='#SkImage_asLegacyBitmap_bitmap'>resets</a> <a href='#SkImage_asLegacyBitmap_bitmap'>bitmap</a> <a href='#SkImage_asLegacyBitmap_bitmap'>if</a>
<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>write</a> <a href='SkBitmap_Reference#SkBitmap'>did</a> <a href='SkBitmap_Reference#SkBitmap'>not</a> <a href='SkBitmap_Reference#SkBitmap'>succeed</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_asLegacyBitmap_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td>storage for legacy <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a></td>
  </tr>
  <tr>    <td><a name='SkImage_asLegacyBitmap_legacyBitmapMode'><code><strong>legacyBitmapMode</strong></code></a></td>
    <td>to be deprecated</td>
  </tr>
</table>

### Return Value

true if <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>was</a> <a href='SkBitmap_Reference#SkBitmap'>created</a>

### Example

<div><fiddle-embed name="78374702fa113076ddc6070053ab5cd4" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeRasterData'>MakeRasterData</a> <a href='#SkImage_makeRasterImage'>makeRasterImage</a> <a href='#SkImage_makeNonTextureImage'>makeNonTextureImage</a>

<a name='SkImage_isLazyGenerated'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkImage_isLazyGenerated'>isLazyGenerated</a>() <a href='#SkImage_isLazyGenerated'>const</a>
</pre>

Returns true if <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>backed</a> <a href='SkImage_Reference#SkImage'>by</a> <a href='SkImage_Reference#SkImage'>an</a> <a href='SkImage_Reference#SkImage'>image-generator</a> <a href='SkImage_Reference#SkImage'>or</a> <a href='SkImage_Reference#SkImage'>other</a> <a href='SkImage_Reference#SkImage'>service</a> <a href='SkImage_Reference#SkImage'>that</a> <a href='SkImage_Reference#SkImage'>creates</a>
and caches its pixels or <a href='undocumented#Texture'>texture</a> <a href='undocumented#Texture'>on-demand</a>.

### Return Value

true if <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>created</a> <a href='SkImage_Reference#SkImage'>as</a> <a href='SkImage_Reference#SkImage'>needed</a>

### Example

<div><fiddle-embed name="a8b8bd4bfe968e2c63085f867665227f"></fiddle-embed></div>

### Example

<div><fiddle-embed name="f031c2a53f6a57833dc0127e674553da" gpu="true"></fiddle-embed></div>

### See Also

<a href='#SkImage_isTextureBacked'>isTextureBacked</a> <a href='#SkImage_makeNonTextureImage'>makeNonTextureImage</a>

<a name='SkImage_makeColorSpace'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkImage_Reference#SkImage'>SkImage</a>&<a href='SkImage_Reference#SkImage'>gt</a>; <a href='#SkImage_makeColorSpace'>makeColorSpace</a>(<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&<a href='undocumented#SkColorSpace'>gt</a>; <a href='undocumented#SkColorSpace'>target</a>) <a href='undocumented#SkColorSpace'>const</a>
</pre>

Creates <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>in</a> <a href='#SkImage_makeColorSpace_target'>target</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>.
Returns nullptr if <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>could</a> <a href='SkImage_Reference#SkImage'>not</a> <a href='SkImage_Reference#SkImage'>be</a> <a href='SkImage_Reference#SkImage'>created</a>.

Returns original <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>if</a> <a href='SkImage_Reference#SkImage'>it</a> <a href='SkImage_Reference#SkImage'>is</a> <a href='SkImage_Reference#SkImage'>in</a> <a href='#SkImage_makeColorSpace_target'>target</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>.
Otherwise, converts pixels from <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>to</a> <a href='#SkImage_makeColorSpace_target'>target</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>.
If <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='#SkImage_colorSpace'>colorSpace</a>() <a href='#SkImage_colorSpace'>returns</a> <a href='#SkImage_colorSpace'>nullptr</a>, <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>is</a> <a href='undocumented#SkColorSpace'>assumed</a> <a href='undocumented#SkColorSpace'>to</a> <a href='undocumented#SkColorSpace'>be</a> <a href='undocumented#SkColorSpace'>sRGB</a>.

### Parameters

<table>  <tr>    <td><a name='SkImage_makeColorSpace_target'><code><strong>target</strong></code></a></td>
    <td><a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>describing</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>range</a> <a href='SkColor_Reference#Color'>of</a> <a href='SkColor_Reference#Color'>returned</a> <a href='SkImage_Reference#SkImage'>SkImage</a></td>
  </tr>
</table>

### Return Value

created <a href='SkImage_Reference#SkImage'>SkImage</a> <a href='SkImage_Reference#SkImage'>in</a> <a href='#SkImage_makeColorSpace_target'>target</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>

### Example

<div><fiddle-embed name="dbf5f75c1275a3013672f896767140fb"></fiddle-embed></div>

### See Also

<a href='#SkImage_MakeFromPicture'>MakeFromPicture</a> <a href='#SkImage_MakeFromTexture'>MakeFromTexture</a>

