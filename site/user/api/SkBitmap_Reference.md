SkBitmap Reference
===


<a name='SkBitmap'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> {
<a href='SkBitmap_Reference#SkBitmap'>public</a>:
    <a href='#SkBitmap_empty_constructor'>SkBitmap()</a>;
    <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>(<a href='SkBitmap_Reference#SkBitmap'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#SkBitmap'>src</a>);
    <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>&& <a href='SkBitmap_Reference#SkBitmap'>src</a>);
    ~<a href='#SkBitmap_empty_constructor'>SkBitmap()</a>;
    <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#SkBitmap'>operator</a>=(<a href='SkBitmap_Reference#SkBitmap'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#SkBitmap'>src</a>);
    <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#SkBitmap'>operator</a>=(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>&& <a href='SkBitmap_Reference#SkBitmap'>src</a>);
    <a href='SkBitmap_Reference#SkBitmap'>void</a> <a href='#SkBitmap_swap'>swap</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#SkBitmap'>other</a>);
    <a href='SkBitmap_Reference#SkBitmap'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='#SkBitmap_pixmap'>pixmap()</a> <a href='#SkBitmap_pixmap'>const</a>;
    <a href='#SkBitmap_pixmap'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='#SkBitmap_info'>info()</a> <a href='#SkBitmap_info'>const</a>;
    <a href='#SkBitmap_info'>int</a> <a href='#SkBitmap_width'>width()</a> <a href='#SkBitmap_width'>const</a>;
    <a href='#SkBitmap_width'>int</a> <a href='#SkBitmap_height'>height()</a> <a href='#SkBitmap_height'>const</a>;
    <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkBitmap_colorType'>colorType</a>() <a href='#SkBitmap_colorType'>const</a>;
    <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkBitmap_alphaType'>alphaType</a>() <a href='#SkBitmap_alphaType'>const</a>;
    <a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='#SkBitmap_colorSpace'>colorSpace</a>() <a href='#SkBitmap_colorSpace'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='#SkBitmap_refColorSpace'>refColorSpace</a>() <a href='#SkBitmap_refColorSpace'>const</a>;
    <a href='#SkBitmap_refColorSpace'>int</a> <a href='#SkBitmap_bytesPerPixel'>bytesPerPixel</a>() <a href='#SkBitmap_bytesPerPixel'>const</a>;
    <a href='#SkBitmap_bytesPerPixel'>int</a> <a href='#SkBitmap_rowBytesAsPixels'>rowBytesAsPixels</a>() <a href='#SkBitmap_rowBytesAsPixels'>const</a>;
    <a href='#SkBitmap_rowBytesAsPixels'>int</a> <a href='#SkBitmap_shiftPerPixel'>shiftPerPixel</a>() <a href='#SkBitmap_shiftPerPixel'>const</a>;
    <a href='#SkBitmap_shiftPerPixel'>bool</a> <a href='#SkBitmap_empty'>empty()</a> <a href='#SkBitmap_empty'>const</a>;
    <a href='#SkBitmap_empty'>bool</a> <a href='#SkBitmap_isNull'>isNull</a>() <a href='#SkBitmap_isNull'>const</a>;
    <a href='#SkBitmap_isNull'>bool</a> <a href='#SkBitmap_drawsNothing'>drawsNothing</a>() <a href='#SkBitmap_drawsNothing'>const</a>;
    <a href='#SkBitmap_drawsNothing'>size_t</a> <a href='#SkBitmap_rowBytes'>rowBytes</a>() <a href='#SkBitmap_rowBytes'>const</a>;
    <a href='#SkBitmap_rowBytes'>bool</a> <a href='#SkBitmap_setAlphaType'>setAlphaType</a>(<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkBitmap_alphaType'>alphaType</a>);
    <a href='#SkBitmap_alphaType'>void</a>* <a href='#SkBitmap_getPixels'>getPixels</a>() <a href='#SkBitmap_getPixels'>const</a>;
    <a href='#SkBitmap_getPixels'>size_t</a> <a href='#SkBitmap_computeByteSize'>computeByteSize</a>() <a href='#SkBitmap_computeByteSize'>const</a>;
    <a href='#SkBitmap_computeByteSize'>bool</a> <a href='#SkBitmap_isImmutable'>isImmutable</a>() <a href='#SkBitmap_isImmutable'>const</a>;
    <a href='#SkBitmap_isImmutable'>void</a> <a href='#SkBitmap_setImmutable'>setImmutable</a>();
    <a href='#SkBitmap_setImmutable'>bool</a> <a href='#SkBitmap_isOpaque'>isOpaque</a>() <a href='#SkBitmap_isOpaque'>const</a>;
    <a href='#SkBitmap_isOpaque'>bool</a> <a href='#SkBitmap_isVolatile'>isVolatile</a>() <a href='#SkBitmap_isVolatile'>const</a>;
    <a href='#SkBitmap_isVolatile'>void</a> <a href='#SkBitmap_setIsVolatile'>setIsVolatile</a>(<a href='#SkBitmap_setIsVolatile'>bool</a> <a href='#SkBitmap_isVolatile'>isVolatile</a>);
    <a href='#SkBitmap_isVolatile'>void</a> <a href='#SkBitmap_reset'>reset()</a>;
    <a href='#SkBitmap_reset'>static</a> <a href='#SkBitmap_reset'>bool</a> <a href='#SkBitmap_ComputeIsOpaque'>ComputeIsOpaque</a>(<a href='#SkBitmap_ComputeIsOpaque'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#SkBitmap'>bm</a>);
    <a href='SkBitmap_Reference#SkBitmap'>void</a> <a href='#SkBitmap_getBounds'>getBounds</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a>) <a href='SkRect_Reference#SkRect'>const</a>;
    <a href='SkRect_Reference#SkRect'>void</a> <a href='#SkBitmap_getBounds'>getBounds</a>(<a href='SkIRect_Reference#SkIRect'>SkIRect</a>* <a href='SkIRect_Reference#SkIRect'>bounds</a>) <a href='SkIRect_Reference#SkIRect'>const</a>;
    <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkBitmap_bounds'>bounds()</a> <a href='#SkBitmap_bounds'>const</a>;
    <a href='undocumented#SkISize'>SkISize</a> <a href='#SkBitmap_dimensions'>dimensions()</a> <a href='#SkBitmap_dimensions'>const</a>;
    <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkBitmap_getSubset'>getSubset</a>() <a href='#SkBitmap_getSubset'>const</a>;
    <a href='#SkBitmap_getSubset'>bool</a> <a href='#SkBitmap_setInfo'>setInfo</a>(<a href='#SkBitmap_setInfo'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>imageInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='#SkBitmap_rowBytes'>rowBytes</a> = 0);

    <a href='#SkBitmap_rowBytes'>enum</a> <a href='#SkBitmap_AllocFlags'>AllocFlags</a> {
        <a href='#SkBitmap_kZeroPixels_AllocFlag'>kZeroPixels_AllocFlag</a> = 1 << 0,
    };

    <a href='#SkBitmap_kZeroPixels_AllocFlag'>bool</a> <a href='#SkBitmap_tryAllocPixelsFlags'>tryAllocPixelsFlags</a>(<a href='#SkBitmap_tryAllocPixelsFlags'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='SkImageInfo_Reference#SkImageInfo'>uint32_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>flags</a>);
    <a href='SkImageInfo_Reference#SkImageInfo'>void</a> <a href='#SkBitmap_allocPixelsFlags'>allocPixelsFlags</a>(<a href='#SkBitmap_allocPixelsFlags'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='SkImageInfo_Reference#SkImageInfo'>uint32_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>flags</a>);
    <a href='SkImageInfo_Reference#SkImageInfo'>bool</a> <a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a>(<a href='#SkBitmap_tryAllocPixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='#SkBitmap_rowBytes'>rowBytes</a>);
    <a href='#SkBitmap_rowBytes'>void</a> <a href='#SkBitmap_allocPixels'>allocPixels</a>(<a href='#SkBitmap_allocPixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='#SkBitmap_rowBytes'>rowBytes</a>);
    <a href='#SkBitmap_rowBytes'>bool</a> <a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a>(<a href='#SkBitmap_tryAllocPixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>);
    <a href='SkImageInfo_Reference#SkImageInfo'>void</a> <a href='#SkBitmap_allocPixels'>allocPixels</a>(<a href='#SkBitmap_allocPixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>);
    <a href='SkImageInfo_Reference#SkImageInfo'>bool</a> <a href='#SkBitmap_tryAllocN32Pixels'>tryAllocN32Pixels</a>(<a href='#SkBitmap_tryAllocN32Pixels'>int</a> <a href='#SkBitmap_tryAllocN32Pixels'>width</a>, <a href='#SkBitmap_tryAllocN32Pixels'>int</a> <a href='#SkBitmap_tryAllocN32Pixels'>height</a>, <a href='#SkBitmap_tryAllocN32Pixels'>bool</a> <a href='#SkBitmap_isOpaque'>isOpaque</a> = <a href='#SkBitmap_isOpaque'>false</a>);
    <a href='#SkBitmap_isOpaque'>void</a> <a href='#SkBitmap_allocN32Pixels'>allocN32Pixels</a>(<a href='#SkBitmap_allocN32Pixels'>int</a> <a href='#SkBitmap_allocN32Pixels'>width</a>, <a href='#SkBitmap_allocN32Pixels'>int</a> <a href='#SkBitmap_allocN32Pixels'>height</a>, <a href='#SkBitmap_allocN32Pixels'>bool</a> <a href='#SkBitmap_isOpaque'>isOpaque</a> = <a href='#SkBitmap_isOpaque'>false</a>);
    <a href='#SkBitmap_isOpaque'>bool</a> <a href='#SkBitmap_installPixels'>installPixels</a>(<a href='#SkBitmap_installPixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>pixels</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='#SkBitmap_rowBytes'>rowBytes</a>,
                       <a href='#SkBitmap_rowBytes'>void</a> (*<a href='#SkBitmap_rowBytes'>releaseProc</a>)(<a href='#SkBitmap_rowBytes'>void</a>* <a href='#SkBitmap_rowBytes'>addr</a>, <a href='#SkBitmap_rowBytes'>void</a>* <a href='#SkBitmap_rowBytes'>context</a>), <a href='#SkBitmap_rowBytes'>void</a>* <a href='#SkBitmap_rowBytes'>context</a>);
    <a href='#SkBitmap_rowBytes'>bool</a> <a href='#SkBitmap_installPixels'>installPixels</a>(<a href='#SkBitmap_installPixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>pixels</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='#SkBitmap_rowBytes'>rowBytes</a>);
    <a href='#SkBitmap_rowBytes'>bool</a> <a href='#SkBitmap_installPixels'>installPixels</a>(<a href='#SkBitmap_installPixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#Pixmap'>pixmap</a>);
    <a href='SkPixmap_Reference#Pixmap'>bool</a> <a href='#SkBitmap_installMaskPixels'>installMaskPixels</a>(<a href='#SkBitmap_installMaskPixels'>const</a> <a href='undocumented#SkMask'>SkMask</a>& <a href='undocumented#SkMask'>mask</a>);
    <a href='undocumented#SkMask'>void</a> <a href='#SkBitmap_setPixels'>setPixels</a>(<a href='#SkBitmap_setPixels'>void</a>* <a href='#SkBitmap_setPixels'>pixels</a>);
    <a href='#SkBitmap_setPixels'>bool</a> <a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a>();
    <a href='#SkBitmap_tryAllocPixels'>void</a> <a href='#SkBitmap_allocPixels'>allocPixels</a>();
    <a href='#SkBitmap_allocPixels'>bool</a> <a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a>(<a href='#SkBitmap_Allocator'>Allocator</a>* <a href='#SkBitmap_Allocator'>allocator</a>);
    <a href='#SkBitmap_Allocator'>void</a> <a href='#SkBitmap_allocPixels'>allocPixels</a>(<a href='#SkBitmap_Allocator'>Allocator</a>* <a href='#SkBitmap_Allocator'>allocator</a>);
    <a href='undocumented#SkPixelRef'>SkPixelRef</a>* <a href='#SkBitmap_pixelRef'>pixelRef</a>() <a href='#SkBitmap_pixelRef'>const</a>;
    <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='#SkBitmap_pixelRefOrigin'>pixelRefOrigin</a>() <a href='#SkBitmap_pixelRefOrigin'>const</a>;
    <a href='#SkBitmap_pixelRefOrigin'>void</a> <a href='#SkBitmap_setPixelRef'>setPixelRef</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkPixelRef'>SkPixelRef</a>> <a href='#SkBitmap_pixelRef'>pixelRef</a>, <a href='#SkBitmap_pixelRef'>int</a> <a href='#SkBitmap_pixelRef'>dx</a>, <a href='#SkBitmap_pixelRef'>int</a> <a href='#SkBitmap_pixelRef'>dy</a>);
    <a href='#SkBitmap_pixelRef'>bool</a> <a href='#SkBitmap_readyToDraw'>readyToDraw</a>() <a href='#SkBitmap_readyToDraw'>const</a>;
    <a href='#SkBitmap_readyToDraw'>uint32_t</a> <a href='#SkBitmap_getGenerationID'>getGenerationID</a>() <a href='#SkBitmap_getGenerationID'>const</a>;
    <a href='#SkBitmap_getGenerationID'>void</a> <a href='#SkBitmap_notifyPixelsChanged'>notifyPixelsChanged</a>() <a href='#SkBitmap_notifyPixelsChanged'>const</a>;
    <a href='#SkBitmap_notifyPixelsChanged'>void</a> <a href='#SkBitmap_eraseColor'>eraseColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColor'>c</a>) <a href='SkColor_Reference#SkColor'>const</a>;
    <a href='SkColor_Reference#SkColor'>void</a> <a href='#SkBitmap_eraseARGB'>eraseARGB</a>(<a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>a</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>r</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>g</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>b</a>) <a href='undocumented#U8CPU'>const</a>;
    <a href='undocumented#U8CPU'>void</a> <a href='#SkBitmap_erase'>erase</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColor'>c</a>, <a href='SkColor_Reference#SkColor'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>area</a>) <a href='SkIRect_Reference#SkIRect'>const</a>;
    <a href='SkIRect_Reference#SkIRect'>void</a> <a href='#SkBitmap_eraseArea'>eraseArea</a>(<a href='#SkBitmap_eraseArea'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>area</a>, <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColor'>c</a>) <a href='SkColor_Reference#SkColor'>const</a>;
    <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkBitmap_getColor'>getColor</a>(<a href='#SkBitmap_getColor'>int</a> <a href='#SkBitmap_getColor'>x</a>, <a href='#SkBitmap_getColor'>int</a> <a href='#SkBitmap_getColor'>y</a>) <a href='#SkBitmap_getColor'>const</a>;
    <a href='#SkBitmap_getColor'>float</a> <a href='#SkBitmap_getAlphaf'>getAlphaf</a>(<a href='#SkBitmap_getAlphaf'>int</a> <a href='#SkBitmap_getAlphaf'>x</a>, <a href='#SkBitmap_getAlphaf'>int</a> <a href='#SkBitmap_getAlphaf'>y</a>) <a href='#SkBitmap_getAlphaf'>const</a>;
    <a href='#SkBitmap_getAlphaf'>void</a>* <a href='#SkBitmap_getAddr'>getAddr</a>(<a href='#SkBitmap_getAddr'>int</a> <a href='#SkBitmap_getAddr'>x</a>, <a href='#SkBitmap_getAddr'>int</a> <a href='#SkBitmap_getAddr'>y</a>) <a href='#SkBitmap_getAddr'>const</a>;
    <a href='#SkBitmap_getAddr'>uint32_t</a>* <a href='#SkBitmap_getAddr32'>getAddr32</a>(<a href='#SkBitmap_getAddr32'>int</a> <a href='#SkBitmap_getAddr32'>x</a>, <a href='#SkBitmap_getAddr32'>int</a> <a href='#SkBitmap_getAddr32'>y</a>) <a href='#SkBitmap_getAddr32'>const</a>;
    <a href='#SkBitmap_getAddr32'>uint16_t</a>* <a href='#SkBitmap_getAddr16'>getAddr16</a>(<a href='#SkBitmap_getAddr16'>int</a> <a href='#SkBitmap_getAddr16'>x</a>, <a href='#SkBitmap_getAddr16'>int</a> <a href='#SkBitmap_getAddr16'>y</a>) <a href='#SkBitmap_getAddr16'>const</a>;
    <a href='#SkBitmap_getAddr16'>uint8_t</a>* <a href='#SkBitmap_getAddr8'>getAddr8</a>(<a href='#SkBitmap_getAddr8'>int</a> <a href='#SkBitmap_getAddr8'>x</a>, <a href='#SkBitmap_getAddr8'>int</a> <a href='#SkBitmap_getAddr8'>y</a>) <a href='#SkBitmap_getAddr8'>const</a>;
    <a href='#SkBitmap_getAddr8'>bool</a> <a href='#SkBitmap_extractSubset'>extractSubset</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* <a href='SkBitmap_Reference#SkBitmap'>dst</a>, <a href='SkBitmap_Reference#SkBitmap'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>subset</a>) <a href='SkIRect_Reference#SkIRect'>const</a>;
    <a href='SkIRect_Reference#SkIRect'>bool</a> <a href='#SkBitmap_readPixels'>readPixels</a>(<a href='#SkBitmap_readPixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>dstInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>dstPixels</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>dstRowBytes</a>,
                    <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>srcX</a>, <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>srcY</a>) <a href='SkImageInfo_Reference#SkImageInfo'>const</a>;
    <a href='SkImageInfo_Reference#SkImageInfo'>bool</a> <a href='#SkBitmap_readPixels'>readPixels</a>(<a href='#SkBitmap_readPixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#SkPixmap'>dst</a>, <a href='SkPixmap_Reference#SkPixmap'>int</a> <a href='SkPixmap_Reference#SkPixmap'>srcX</a>, <a href='SkPixmap_Reference#SkPixmap'>int</a> <a href='SkPixmap_Reference#SkPixmap'>srcY</a>) <a href='SkPixmap_Reference#SkPixmap'>const</a>;
    <a href='SkPixmap_Reference#SkPixmap'>bool</a> <a href='#SkBitmap_readPixels'>readPixels</a>(<a href='#SkBitmap_readPixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#SkPixmap'>dst</a>) <a href='SkPixmap_Reference#SkPixmap'>const</a>;
    <a href='SkPixmap_Reference#SkPixmap'>bool</a> <a href='#SkBitmap_writePixels'>writePixels</a>(<a href='#SkBitmap_writePixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#SkPixmap'>src</a>, <a href='SkPixmap_Reference#SkPixmap'>int</a> <a href='SkPixmap_Reference#SkPixmap'>dstX</a>, <a href='SkPixmap_Reference#SkPixmap'>int</a> <a href='SkPixmap_Reference#SkPixmap'>dstY</a>);
    <a href='SkPixmap_Reference#SkPixmap'>bool</a> <a href='#SkBitmap_writePixels'>writePixels</a>(<a href='#SkBitmap_writePixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#SkPixmap'>src</a>);
    <a href='SkPixmap_Reference#SkPixmap'>bool</a> <a href='#SkBitmap_extractAlpha'>extractAlpha</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* <a href='SkBitmap_Reference#SkBitmap'>dst</a>) <a href='SkBitmap_Reference#SkBitmap'>const</a>;
    <a href='SkBitmap_Reference#SkBitmap'>bool</a> <a href='#SkBitmap_extractAlpha'>extractAlpha</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* <a href='SkBitmap_Reference#SkBitmap'>dst</a>, <a href='SkBitmap_Reference#SkBitmap'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                      <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>* <a href='SkIPoint_Reference#SkIPoint'>offset</a>) <a href='SkIPoint_Reference#SkIPoint'>const</a>;
    <a href='SkIPoint_Reference#SkIPoint'>bool</a> <a href='#SkBitmap_extractAlpha'>extractAlpha</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* <a href='SkBitmap_Reference#SkBitmap'>dst</a>, <a href='SkBitmap_Reference#SkBitmap'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='#SkBitmap_Allocator'>Allocator</a>* <a href='#SkBitmap_Allocator'>allocator</a>,
                      <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>* <a href='SkIPoint_Reference#SkIPoint'>offset</a>) <a href='SkIPoint_Reference#SkIPoint'>const</a>;
    <a href='SkIPoint_Reference#SkIPoint'>bool</a> <a href='#SkBitmap_peekPixels'>peekPixels</a>(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>* <a href='SkPixmap_Reference#Pixmap'>pixmap</a>) <a href='SkPixmap_Reference#Pixmap'>const</a>;
    <a href='SkPixmap_Reference#Pixmap'>void</a> <a href='#SkBitmap_validate'>validate()</a> <a href='#SkBitmap_validate'>const</a>;

    <a href='#SkBitmap_validate'>class</a> <a href='#SkBitmap_Allocator'>Allocator</a> : <a href='#SkBitmap_Allocator'>public</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> {
    <a href='undocumented#SkRefCnt'>public</a>:
        <a href='undocumented#SkRefCnt'>virtual</a> <a href='undocumented#SkRefCnt'>bool</a> <a href='undocumented#SkRefCnt'>allocPixelRef</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* <a href='SkBitmap_Reference#Bitmap'>bitmap</a>) = 0;
    };

    <a href='SkBitmap_Reference#Bitmap'>class</a> <a href='#SkBitmap_HeapAllocator'>HeapAllocator</a> : <a href='#SkBitmap_HeapAllocator'>public</a> <a href='#SkBitmap_Allocator'>Allocator</a> {
    <a href='#SkBitmap_Allocator'>public</a>:
        <a href='#SkBitmap_Allocator'>bool</a> <a href='#SkBitmap_Allocator'>allocPixelRef</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* <a href='SkBitmap_Reference#Bitmap'>bitmap</a>) <a href='SkBitmap_Reference#Bitmap'>override</a>;
    };
};
</pre>

<a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>describes</a> <a href='SkBitmap_Reference#Bitmap'>a</a> <a href='SkBitmap_Reference#Bitmap'>two-dimensional</a> <a href='SkBitmap_Reference#Bitmap'>raster</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>array</a>. <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>is</a> <a href='SkBitmap_Reference#Bitmap'>built</a> <a href='SkBitmap_Reference#Bitmap'>on</a>
<a href='#Image_Info'>Image_Info</a>, <a href='#Image_Info'>containing</a> <a href='#Image_Info'>integer</a> <a href='#Image_Info'>width</a> <a href='#Image_Info'>and</a> <a href='#Image_Info'>height</a>, <a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Image_Info_Color_Type'>and</a> <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>
<a href='#Image_Info_Alpha_Type'>describing</a> <a href='#Image_Info_Alpha_Type'>the</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>format</a>, <a href='undocumented#Pixel'>and</a> <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>describing</a> <a href='#Color_Space'>the</a> <a href='#Color_Space'>range</a> <a href='#Color_Space'>of</a> <a href='#Color_Space'>colors</a>.
<a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>to</a> <a href='#Pixel_Ref'>Pixel_Ref</a>, <a href='#Pixel_Ref'>which</a> <a href='#Pixel_Ref'>describes</a> <a href='#Pixel_Ref'>the</a> <a href='#Pixel_Ref'>physical</a> <a href='#Pixel_Ref'>array</a> <a href='#Pixel_Ref'>of</a> <a href='#Pixel_Ref'>pixels</a>.
<a href='#Image_Info'>Image_Info</a> <a href='#Image_Info'>bounds</a> <a href='#Image_Info'>may</a> <a href='#Image_Info'>be</a> <a href='#Image_Info'>located</a> <a href='#Image_Info'>anywhere</a> <a href='#Image_Info'>fully</a> <a href='#Image_Info'>inside</a> <a href='#Pixel_Ref'>Pixel_Ref</a> <a href='#Pixel_Ref'>bounds</a>.

<a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>can</a> <a href='SkBitmap_Reference#Bitmap'>be</a> <a href='SkBitmap_Reference#Bitmap'>drawn</a> <a href='SkBitmap_Reference#Bitmap'>using</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a>. <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>can</a> <a href='SkBitmap_Reference#Bitmap'>be</a> <a href='SkBitmap_Reference#Bitmap'>a</a> <a href='SkBitmap_Reference#Bitmap'>drawing</a> <a href='SkBitmap_Reference#Bitmap'>destination</a> <a href='SkBitmap_Reference#Bitmap'>for</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a>
<a href='SkCanvas_Reference#Canvas'>draw</a> <a href='SkCanvas_Reference#Canvas'>member</a> <a href='SkCanvas_Reference#Canvas'>functions</a>. <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>flexibility</a> <a href='SkBitmap_Reference#Bitmap'>as</a> <a href='SkBitmap_Reference#Bitmap'>a</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>container</a> <a href='undocumented#Pixel'>limits</a> <a href='undocumented#Pixel'>some</a>
<a href='undocumented#Pixel'>optimizations</a> <a href='undocumented#Pixel'>available</a> <a href='undocumented#Pixel'>to</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>target</a> <a href='undocumented#Pixel'>platform</a>.

<a href='undocumented#Pixel'>If</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>array</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>primarily</a> <a href='undocumented#Pixel'>read-only</a>, <a href='undocumented#Pixel'>use</a> <a href='SkImage_Reference#Image'>Image</a> <a href='SkImage_Reference#Image'>for</a> <a href='SkImage_Reference#Image'>better</a> <a href='SkImage_Reference#Image'>performance</a>.
<a href='SkImage_Reference#Image'>If</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>array</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>primarily</a> <a href='undocumented#Pixel'>written</a> <a href='undocumented#Pixel'>to</a>, <a href='undocumented#Pixel'>use</a> <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>for</a> <a href='SkSurface_Reference#Surface'>better</a> <a href='SkSurface_Reference#Surface'>performance</a>.

<a href='SkSurface_Reference#Surface'>Declaring</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>const</a> <a href='SkBitmap_Reference#SkBitmap'>prevents</a> <a href='SkBitmap_Reference#SkBitmap'>altering</a> <a href='#Image_Info'>Image_Info</a>: <a href='#Image_Info'>the</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>height</a>, <a href='SkBitmap_Reference#Bitmap'>width</a>,
<a href='SkBitmap_Reference#Bitmap'>and</a> <a href='SkBitmap_Reference#Bitmap'>so</a> <a href='SkBitmap_Reference#Bitmap'>on</a> <a href='SkBitmap_Reference#Bitmap'>cannot</a> <a href='SkBitmap_Reference#Bitmap'>change</a>. <a href='SkBitmap_Reference#Bitmap'>It</a> <a href='SkBitmap_Reference#Bitmap'>does</a> <a href='SkBitmap_Reference#Bitmap'>not</a> <a href='SkBitmap_Reference#Bitmap'>affect</a> <a href='#Pixel_Ref'>Pixel_Ref</a>: <a href='#Pixel_Ref'>a</a> <a href='#Pixel_Ref'>caller</a> <a href='#Pixel_Ref'>may</a> <a href='#Pixel_Ref'>write</a> <a href='#Pixel_Ref'>its</a>
<a href='#Pixel_Ref'>pixels</a>. <a href='#Pixel_Ref'>Declaring</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>const</a> <a href='SkBitmap_Reference#SkBitmap'>affects</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>configuration</a>, <a href='SkBitmap_Reference#Bitmap'>not</a> <a href='SkBitmap_Reference#Bitmap'>its</a> <a href='SkBitmap_Reference#Bitmap'>contents</a>.

<a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>is</a> <a href='SkBitmap_Reference#Bitmap'>not</a> <a href='SkBitmap_Reference#Bitmap'>thread</a> <a href='SkBitmap_Reference#Bitmap'>safe</a>. <a href='SkBitmap_Reference#Bitmap'>Each</a> <a href='SkBitmap_Reference#Bitmap'>thread</a> <a href='SkBitmap_Reference#Bitmap'>must</a> <a href='SkBitmap_Reference#Bitmap'>have</a> <a href='SkBitmap_Reference#Bitmap'>its</a> <a href='SkBitmap_Reference#Bitmap'>own</a> <a href='SkBitmap_Reference#Bitmap'>copy</a> <a href='SkBitmap_Reference#Bitmap'>of</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>fields</a>,
<a href='SkBitmap_Reference#Bitmap'>although</a> <a href='SkBitmap_Reference#Bitmap'>threads</a> <a href='SkBitmap_Reference#Bitmap'>may</a> <a href='SkBitmap_Reference#Bitmap'>share</a> <a href='SkBitmap_Reference#Bitmap'>the</a> <a href='SkBitmap_Reference#Bitmap'>underlying</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>array</a>.

<a name='Row_Bytes'></a>

<a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>pixels</a> <a href='SkBitmap_Reference#Bitmap'>may</a> <a href='SkBitmap_Reference#Bitmap'>be</a> <a href='SkBitmap_Reference#Bitmap'>contiguous</a>, <a href='SkBitmap_Reference#Bitmap'>or</a> <a href='SkBitmap_Reference#Bitmap'>may</a> <a href='SkBitmap_Reference#Bitmap'>have</a> <a href='SkBitmap_Reference#Bitmap'>a</a> <a href='SkBitmap_Reference#Bitmap'>gap</a> <a href='SkBitmap_Reference#Bitmap'>at</a> <a href='SkBitmap_Reference#Bitmap'>the</a> <a href='SkBitmap_Reference#Bitmap'>end</a> <a href='SkBitmap_Reference#Bitmap'>of</a> <a href='SkBitmap_Reference#Bitmap'>each</a> <a href='SkBitmap_Reference#Bitmap'>row</a>.
<a href='#Bitmap_Row_Bytes'>Row_Bytes</a> <a href='#Bitmap_Row_Bytes'>is</a> <a href='#Bitmap_Row_Bytes'>the</a> <a href='#Bitmap_Row_Bytes'>interval</a> <a href='#Bitmap_Row_Bytes'>from</a> <a href='#Bitmap_Row_Bytes'>one</a> <a href='#Bitmap_Row_Bytes'>row</a> <a href='#Bitmap_Row_Bytes'>to</a> <a href='#Bitmap_Row_Bytes'>the</a> <a href='#Bitmap_Row_Bytes'>next</a>. <a href='#Bitmap_Row_Bytes'>Row_Bytes</a> <a href='#Bitmap_Row_Bytes'>may</a> <a href='#Bitmap_Row_Bytes'>be</a> <a href='#Bitmap_Row_Bytes'>specified</a>;
<a href='#Bitmap_Row_Bytes'>sometimes</a> <a href='#Bitmap_Row_Bytes'>passing</a> <a href='#Bitmap_Row_Bytes'>zero</a> <a href='#Bitmap_Row_Bytes'>will</a> <a href='#Bitmap_Row_Bytes'>compute</a> <a href='#Bitmap_Row_Bytes'>the</a> <a href='#Bitmap_Row_Bytes'>Row_Bytes</a> <a href='#Bitmap_Row_Bytes'>from</a> <a href='#Bitmap_Row_Bytes'>the</a> <a href='#Bitmap_Row_Bytes'>row</a> <a href='#Bitmap_Row_Bytes'>width</a> <a href='#Bitmap_Row_Bytes'>and</a> <a href='#Bitmap_Row_Bytes'>the</a>
<a href='#Bitmap_Row_Bytes'>number</a> <a href='#Bitmap_Row_Bytes'>of</a> <a href='#Bitmap_Row_Bytes'>bytes</a> <a href='#Bitmap_Row_Bytes'>in</a> <a href='#Bitmap_Row_Bytes'>a</a> <a href='undocumented#Pixel'>pixel</a>. <a href='#Bitmap_Row_Bytes'>Row_Bytes</a> <a href='#Bitmap_Row_Bytes'>may</a> <a href='#Bitmap_Row_Bytes'>be</a> <a href='#Bitmap_Row_Bytes'>larger</a> <a href='#Bitmap_Row_Bytes'>than</a> <a href='#Bitmap_Row_Bytes'>the</a> <a href='#Bitmap_Row_Bytes'>row</a> <a href='#Bitmap_Row_Bytes'>requires</a>. <a href='#Bitmap_Row_Bytes'>This</a>
<a href='#Bitmap_Row_Bytes'>is</a> <a href='#Bitmap_Row_Bytes'>useful</a> <a href='#Bitmap_Row_Bytes'>to</a> <a href='#Bitmap_Row_Bytes'>position</a> <a href='#Bitmap_Row_Bytes'>one</a> <a href='#Bitmap_Row_Bytes'>or</a> <a href='#Bitmap_Row_Bytes'>more</a> <a href='SkBitmap_Reference#Bitmap'>Bitmaps</a> <a href='SkBitmap_Reference#Bitmap'>within</a> <a href='SkBitmap_Reference#Bitmap'>a</a> <a href='SkBitmap_Reference#Bitmap'>shared</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>array</a>.

<a name='SkBitmap_Allocator'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    class <a href='#SkBitmap_Allocator'>Allocator</a> : <a href='#SkBitmap_Allocator'>public</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> {
    <a href='undocumented#SkRefCnt'>public</a>:
        <a href='undocumented#SkRefCnt'>virtual</a> <a href='undocumented#SkRefCnt'>bool</a> <a href='#SkBitmap_Allocator_allocPixelRef'>allocPixelRef</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* <a href='SkBitmap_Reference#Bitmap'>bitmap</a>) = 0;
    };
</pre>

Abstract subclass of <a href='#SkBitmap_HeapAllocator'>HeapAllocator</a>.

<a name='SkBitmap_Allocator_allocPixelRef'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual bool <a href='#SkBitmap_Allocator_allocPixelRef'>allocPixelRef</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* <a href='SkBitmap_Reference#Bitmap'>bitmap</a>) = 0
</pre>

Allocates the <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a> <a href='undocumented#Pixel'>for</a> <a href='undocumented#Pixel'>the</a> <a href='#SkBitmap_Allocator_allocPixelRef_bitmap'>bitmap</a>, <a href='#SkBitmap_Allocator_allocPixelRef_bitmap'>given</a> <a href='#SkBitmap_Allocator_allocPixelRef_bitmap'>its</a> <a href='#SkBitmap_Allocator_allocPixelRef_bitmap'>dimensions</a> <a href='#SkBitmap_Allocator_allocPixelRef_bitmap'>and</a>
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>. <a href='SkImageInfo_Reference#SkColorType'>Returns</a> <a href='SkImageInfo_Reference#SkColorType'>true</a> <a href='SkImageInfo_Reference#SkColorType'>on</a> <a href='SkImageInfo_Reference#SkColorType'>success</a>, <a href='SkImageInfo_Reference#SkColorType'>where</a> <a href='SkImageInfo_Reference#SkColorType'>success</a> <a href='SkImageInfo_Reference#SkColorType'>means</a> <a href='SkImageInfo_Reference#SkColorType'>either</a> <a href='#SkBitmap_setPixels'>setPixels</a>()
or <a href='#SkBitmap_setPixelRef'>setPixelRef</a>() <a href='#SkBitmap_setPixelRef'>was</a> <a href='#SkBitmap_setPixelRef'>called</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_Allocator_allocPixelRef_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>containing</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>as</a> <a href='SkImageInfo_Reference#SkImageInfo'>input</a>, <a href='SkImageInfo_Reference#SkImageInfo'>and</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>as</a> <a href='undocumented#SkPixelRef'>output</a></td>
  </tr>
</table>

### Return Value

true if <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>was</a> <a href='undocumented#SkPixelRef'>allocated</a>

### See Also

<a href='#SkBitmap_HeapAllocator'>HeapAllocator</a>

<a name='SkBitmap_HeapAllocator'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    class <a href='#SkBitmap_HeapAllocator'>HeapAllocator</a> : <a href='#SkBitmap_HeapAllocator'>public</a> <a href='#SkBitmap_Allocator'>Allocator</a> {
    <a href='#SkBitmap_Allocator'>public</a>:
        <a href='#SkBitmap_Allocator'>bool</a> <a href='#SkBitmap_HeapAllocator_allocPixelRef'>allocPixelRef</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* <a href='SkBitmap_Reference#Bitmap'>bitmap</a>) <a href='SkBitmap_Reference#Bitmap'>override</a>;
    };
</pre>

Subclass of <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_Allocator'>Allocator</a> <a href='#SkBitmap_Allocator'>that</a> <a href='#SkBitmap_Allocator'>returns</a> <a href='#SkBitmap_Allocator'>a</a> <a href='#Pixel_Ref'>Pixel_Ref</a> <a href='#Pixel_Ref'>that</a> <a href='#Pixel_Ref'>allocates</a> <a href='#Pixel_Ref'>its</a> <a href='undocumented#Pixel'>pixel</a>
<a href='undocumented#Pixel'>memory</a> <a href='undocumented#Pixel'>from</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>heap</a>. <a href='undocumented#Pixel'>This</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>default</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_Allocator'>Allocator</a> <a href='#SkBitmap_Allocator'>invoked</a> <a href='#SkBitmap_Allocator'>by</a>
<a href='#SkBitmap_allocPixels'>allocPixels</a>.

<a name='SkBitmap_HeapAllocator_allocPixelRef'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_HeapAllocator_allocPixelRef'>allocPixelRef</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* <a href='SkBitmap_Reference#Bitmap'>bitmap</a>) <a href='SkBitmap_Reference#Bitmap'>override</a>
</pre>

Allocates the <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a> <a href='undocumented#Pixel'>for</a> <a href='undocumented#Pixel'>the</a> <a href='#SkBitmap_HeapAllocator_allocPixelRef_bitmap'>bitmap</a>, <a href='#SkBitmap_HeapAllocator_allocPixelRef_bitmap'>given</a> <a href='#SkBitmap_HeapAllocator_allocPixelRef_bitmap'>its</a> <a href='#SkBitmap_HeapAllocator_allocPixelRef_bitmap'>dimensions</a> <a href='#SkBitmap_HeapAllocator_allocPixelRef_bitmap'>and</a>
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>. <a href='SkImageInfo_Reference#SkColorType'>Returns</a> <a href='SkImageInfo_Reference#SkColorType'>true</a> <a href='SkImageInfo_Reference#SkColorType'>on</a> <a href='SkImageInfo_Reference#SkColorType'>success</a>, <a href='SkImageInfo_Reference#SkColorType'>where</a> <a href='SkImageInfo_Reference#SkColorType'>success</a> <a href='SkImageInfo_Reference#SkColorType'>means</a> <a href='SkImageInfo_Reference#SkColorType'>either</a> <a href='#SkBitmap_setPixels'>setPixels</a>()
or <a href='#SkBitmap_setPixelRef'>setPixelRef</a>() <a href='#SkBitmap_setPixelRef'>was</a> <a href='#SkBitmap_setPixelRef'>called</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_HeapAllocator_allocPixelRef_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>containing</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>as</a> <a href='SkImageInfo_Reference#SkImageInfo'>input</a>, <a href='SkImageInfo_Reference#SkImageInfo'>and</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>as</a> <a href='undocumented#SkPixelRef'>output</a></td>
  </tr>
</table>

### Return Value

true if pixels are allocated

### Example

<div><fiddle-embed name="fe79a9c1ec350264eb9c7b2509dd3638">

#### Example Output

~~~~
#Volatile
pixel address = (nil)
pixel address = 0x560ddd0ac670
~~~~

</fiddle-embed></div>

### See Also

<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_Allocator'>Allocator</a> <a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a>

<a name='SkBitmap_empty_constructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkBitmap_empty_constructor'>SkBitmap()</a>
</pre>

Creates an empty <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>without</a> <a href='SkBitmap_Reference#SkBitmap'>pixels</a>, <a href='SkBitmap_Reference#SkBitmap'>with</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>,
<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>and</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>with</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>a</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>width</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>and</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>height</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>of</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>zero</a>. <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>origin</a> <a href='undocumented#SkPixelRef'>is</a>
set to (0, 0). <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>is</a> <a href='SkBitmap_Reference#SkBitmap'>not</a> <a href='SkBitmap_Reference#SkBitmap'>volatile</a>.

Use <a href='#SkBitmap_setInfo'>setInfo</a>() <a href='#SkBitmap_setInfo'>to</a> <a href='#SkBitmap_setInfo'>associate</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>width</a>, <a href='SkImageInfo_Reference#SkAlphaType'>and</a> <a href='SkImageInfo_Reference#SkAlphaType'>height</a>
after <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>has</a> <a href='SkBitmap_Reference#SkBitmap'>been</a> <a href='SkBitmap_Reference#SkBitmap'>created</a>.

### Return Value

empty <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>

### Example

<div><fiddle-embed name="6739d14ec0d6a373f2fcadc6b3077fd4">

#### Example Output

~~~~
width:  0  height:  0  color: kUnknown_SkColorType  alpha: kUnknown_SkAlphaType
width: 25  height: 35  color: kRGBA_8888_SkColorType  alpha: kOpaque_SkAlphaType
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_setInfo'>setInfo</a>

<a name='SkBitmap_copy_const_SkBitmap'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>(<a href='SkBitmap_Reference#SkBitmap'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#SkBitmap'>src</a>)
</pre>

Copies settings from <a href='#SkBitmap_copy_const_SkBitmap_src'>src</a> <a href='#SkBitmap_copy_const_SkBitmap_src'>to</a> <a href='#SkBitmap_copy_const_SkBitmap_src'>returned</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>. <a href='SkBitmap_Reference#SkBitmap'>Shares</a> <a href='SkBitmap_Reference#SkBitmap'>pixels</a> <a href='SkBitmap_Reference#SkBitmap'>if</a> <a href='#SkBitmap_copy_const_SkBitmap_src'>src</a> <a href='#SkBitmap_copy_const_SkBitmap_src'>has</a> <a href='#SkBitmap_copy_const_SkBitmap_src'>pixels</a>
allocated, so both <a href='SkBitmap_Reference#Bitmap'>bitmaps</a> <a href='SkBitmap_Reference#Bitmap'>reference</a> <a href='SkBitmap_Reference#Bitmap'>the</a> <a href='SkBitmap_Reference#Bitmap'>same</a> <a href='SkBitmap_Reference#Bitmap'>pixels</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_copy_const_SkBitmap_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>to</a> <a href='SkBitmap_Reference#SkBitmap'>copy</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>and</a> <a href='SkImageInfo_Reference#SkImageInfo'>share</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a></td>
  </tr>
</table>

### Return Value

copy of <a href='#SkBitmap_copy_const_SkBitmap_src'>src</a>

### Example

<div><fiddle-embed name="bbbae7a181bfd128a4484e8e9f454db1">

#### Example Output

~~~~
original has pixels before copy: true
original has pixels after copy: true
copy has pixels: true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_setInfo'>setInfo</a> <a href='#SkBitmap_setPixelRef'>setPixelRef</a> <a href='#SkBitmap_setPixels'>setPixels</a> <a href='#SkBitmap_swap'>swap</a>

<a name='SkBitmap_move_SkBitmap'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>&& <a href='SkBitmap_Reference#SkBitmap'>src</a>)
</pre>

Copies settings from <a href='#SkBitmap_move_SkBitmap_src'>src</a> <a href='#SkBitmap_move_SkBitmap_src'>to</a> <a href='#SkBitmap_move_SkBitmap_src'>returned</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>. <a href='SkBitmap_Reference#SkBitmap'>Moves</a> <a href='SkBitmap_Reference#SkBitmap'>ownership</a> <a href='SkBitmap_Reference#SkBitmap'>of</a> <a href='#SkBitmap_move_SkBitmap_src'>src</a> <a href='#SkBitmap_move_SkBitmap_src'>pixels</a> <a href='#SkBitmap_move_SkBitmap_src'>to</a>
<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_move_SkBitmap_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>to</a> <a href='SkBitmap_Reference#SkBitmap'>copy</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>and</a> <a href='SkImageInfo_Reference#SkImageInfo'>reassign</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a></td>
  </tr>
</table>

### Return Value

copy of <a href='#SkBitmap_move_SkBitmap_src'>src</a>

### Example

<div><fiddle-embed name="40afd4f1fa69e02d69d92b38252088ef">

#### Example Output

~~~~
original has pixels before move: true
original has pixels after move: false
copy has pixels: true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_setInfo'>setInfo</a> <a href='#SkBitmap_setPixelRef'>setPixelRef</a> <a href='#SkBitmap_setPixels'>setPixels</a> <a href='#SkBitmap_swap'>swap</a>

<a name='SkBitmap_destructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
~<a href='#SkBitmap_empty_constructor'>SkBitmap()</a>
</pre>

Decrements <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>reference</a> <a href='undocumented#SkPixelRef'>count</a>, <a href='undocumented#SkPixelRef'>if</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>is</a> <a href='undocumented#SkPixelRef'>not</a> <a href='undocumented#SkPixelRef'>nullptr</a>.

### See Also

<a href='#Pixel_Ref'>Pixel_Ref</a>

<a name='SkBitmap_copy_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#SkBitmap'>operator</a>=(<a href='SkBitmap_Reference#SkBitmap'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#SkBitmap'>src</a>)
</pre>

Copies settings from <a href='#SkBitmap_copy_operator_src'>src</a> <a href='#SkBitmap_copy_operator_src'>to</a> <a href='#SkBitmap_copy_operator_src'>returned</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>. <a href='SkBitmap_Reference#SkBitmap'>Shares</a> <a href='SkBitmap_Reference#SkBitmap'>pixels</a> <a href='SkBitmap_Reference#SkBitmap'>if</a> <a href='#SkBitmap_copy_operator_src'>src</a> <a href='#SkBitmap_copy_operator_src'>has</a> <a href='#SkBitmap_copy_operator_src'>pixels</a>
allocated, so both <a href='SkBitmap_Reference#Bitmap'>bitmaps</a> <a href='SkBitmap_Reference#Bitmap'>reference</a> <a href='SkBitmap_Reference#Bitmap'>the</a> <a href='SkBitmap_Reference#Bitmap'>same</a> <a href='SkBitmap_Reference#Bitmap'>pixels</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_copy_operator_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>to</a> <a href='SkBitmap_Reference#SkBitmap'>copy</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>and</a> <a href='SkImageInfo_Reference#SkImageInfo'>share</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a></td>
  </tr>
</table>

### Return Value

copy of <a href='#SkBitmap_copy_operator_src'>src</a>

### Example

<div><fiddle-embed name="45279c519ae808f78bd30e9d84bdfdde">

#### Example Output

~~~~
original has pixels before copy: true
original has pixels after copy: true
copy has pixels: true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_setInfo'>setInfo</a> <a href='#SkBitmap_setPixelRef'>setPixelRef</a> <a href='#SkBitmap_setPixels'>setPixels</a> <a href='#SkBitmap_swap'>swap</a>

<a name='SkBitmap_move_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#SkBitmap'>operator</a>=(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>&& <a href='SkBitmap_Reference#SkBitmap'>src</a>)
</pre>

Copies settings from <a href='#SkBitmap_move_operator_src'>src</a> <a href='#SkBitmap_move_operator_src'>to</a> <a href='#SkBitmap_move_operator_src'>returned</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>. <a href='SkBitmap_Reference#SkBitmap'>Moves</a> <a href='SkBitmap_Reference#SkBitmap'>ownership</a> <a href='SkBitmap_Reference#SkBitmap'>of</a> <a href='#SkBitmap_move_operator_src'>src</a> <a href='#SkBitmap_move_operator_src'>pixels</a> <a href='#SkBitmap_move_operator_src'>to</a>
<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_move_operator_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>to</a> <a href='SkBitmap_Reference#SkBitmap'>copy</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>and</a> <a href='SkImageInfo_Reference#SkImageInfo'>reassign</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a></td>
  </tr>
</table>

### Return Value

copy of <a href='#SkBitmap_move_operator_src'>src</a>

### Example

<div><fiddle-embed name="35ea3fed27d8db22dc00f48670de64de">

#### Example Output

~~~~
original has pixels before move: true
original has pixels after move: false
copy has pixels: true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_setInfo'>setInfo</a> <a href='#SkBitmap_setPixelRef'>setPixelRef</a> <a href='#SkBitmap_setPixels'>setPixels</a> <a href='#SkBitmap_swap'>swap</a>

<a name='SkBitmap_swap'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_swap'>swap</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#SkBitmap'>other</a>)
</pre>

Swaps the fields of the two <a href='SkBitmap_Reference#Bitmap'>bitmaps</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_swap_other'><code><strong>other</strong></code></a></td>
    <td><a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>exchanged</a> <a href='SkBitmap_Reference#SkBitmap'>with</a> <a href='SkBitmap_Reference#SkBitmap'>original</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="de9be45255e48fae445c916a41063abc">

#### Example Output

~~~~
one width:1 height:1 colorType:kRGBA_8888_SkColorType alphaType:kOpaque_SkAlphaType
two width:2 height:2 colorType:kBGRA_8888_SkColorType alphaType:kPremul_SkAlphaType
one width:2 height:2 colorType:kBGRA_8888_SkColorType alphaType:kPremul_SkAlphaType
two width:1 height:1 colorType:kRGBA_8888_SkColorType alphaType:kOpaque_SkAlphaType
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_move_SkBitmap'>SkBitmap(SkBitmap&& src)</a> SkBitmap(SkBitmap&& src)<a href='#SkBitmap_move_operator'>operator=(SkBitmap&& src)</a>

<a name='Property'></a>

<a name='SkBitmap_pixmap'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='#SkBitmap_pixmap'>pixmap()</a> <a href='#SkBitmap_pixmap'>const</a>
</pre>

Returns a constant reference to the <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='SkPixmap_Reference#SkPixmap'>holding</a> <a href='SkPixmap_Reference#SkPixmap'>the</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='undocumented#Pixel'>pixel</a>
address, row bytes, and <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>.

### Return Value

reference to <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='SkPixmap_Reference#SkPixmap'>describing</a> <a href='SkPixmap_Reference#SkPixmap'>this</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>

### Example

<div><fiddle-embed name="7f972d742dd78d2500034d8867e9ef2f">

#### Example Output

~~~~
----------
---xx-----
--x--x----
--x-------
--xx------
--x-x---x-
-x---x--x-
-x----xx--
-xx---x---
--xxxx-xx-
----------
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_peekPixels'>peekPixels</a> <a href='#SkBitmap_installPixels'>installPixels</a> <a href='#SkBitmap_readPixels'>readPixels</a> <a href='#SkBitmap_writePixels'>writePixels</a>

<a name='SkBitmap_info'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='#SkBitmap_info'>info()</a> <a href='#SkBitmap_info'>const</a>
</pre>

Returns width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkColorType'>and</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

### Return Value

reference to <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="ec47c4dc23e2925ad565eaba55a91553">

#### Example Output

~~~~
width: 56 height: 56 color: BGRA_8888 alpha: Opaque
~~~~

</fiddle-embed></div>

### See Also

<a href='#Image_Info'>Image_Info</a>

<a name='SkBitmap_width'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkBitmap_width'>width()</a> <a href='#SkBitmap_width'>const</a>
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>count</a> <a href='undocumented#Pixel'>in</a> <a href='undocumented#Pixel'>each</a> <a href='undocumented#Pixel'>row</a>. <a href='undocumented#Pixel'>Should</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>equal</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>less</a> <a href='undocumented#Pixel'>than</a>
<code><a href='#SkBitmap_rowBytes'>rowBytes</a>() / <a href='#SkBitmap_info'>info()</a>.<a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>()</code>.

May be less than <a href='#SkBitmap_pixelRef'>pixelRef</a>().<a href='#SkPixelRef_width'>width()</a>. <a href='#SkPixelRef_width'>Will</a> <a href='#SkPixelRef_width'>not</a> <a href='#SkPixelRef_width'>exceed</a> <a href='#SkBitmap_pixelRef'>pixelRef</a>().<a href='#SkPixelRef_width'>width()</a> <a href='#SkPixelRef_width'>less</a>
<a href='#SkBitmap_pixelRefOrigin'>pixelRefOrigin</a>().<a href='#SkIPoint_fX'>fX</a>.

### Return Value

<a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>width</a> <a href='undocumented#Pixel'>in</a> <a href='#Image_Info'>Image_Info</a>

### Example

<div><fiddle-embed name="d06880c42f8bb3b4c3b67bd988046049">

#### Example Output

~~~~
bitmap width: 16  info width: 16
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_height'>height()</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>::<a href='#SkPixelRef_width'>width()</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_width'>width()</a>

<a name='SkBitmap_height'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkBitmap_height'>height()</a> <a href='#SkBitmap_height'>const</a>
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>count</a>.

Maybe be less than <a href='#SkBitmap_pixelRef'>pixelRef</a>().<a href='#SkPixelRef_height'>height()</a>. <a href='#SkPixelRef_height'>Will</a> <a href='#SkPixelRef_height'>not</a> <a href='#SkPixelRef_height'>exceed</a> <a href='#SkBitmap_pixelRef'>pixelRef</a>().<a href='#SkPixelRef_height'>height()</a> <a href='#SkPixelRef_height'>less</a>
<a href='#SkBitmap_pixelRefOrigin'>pixelRefOrigin</a>().<a href='#SkIPoint_fY'>fY</a>.

### Return Value

<a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>height</a> <a href='undocumented#Pixel'>in</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="c79a196278c58b34cd5f551b0124ecc9">

#### Example Output

~~~~
bitmap height: 32  info height: 32
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_width'>width()</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>::<a href='#SkPixelRef_height'>height()</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_height'>height()</a>

<a name='SkBitmap_colorType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkBitmap_colorType'>colorType</a>() <a href='#SkBitmap_colorType'>const</a>
</pre>

Returns <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Color_Type'>one</a> <a href='#Image_Info_Color_Type'>of</a>: <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>
.

### Return Value

<a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Image_Info_Color_Type'>in</a> <a href='#Image_Info'>Image_Info</a>

### Example

<div><fiddle-embed name="ceb77fab7326b57822a147b04aa0960e">

#### Example Output

~~~~
color type: kAlpha_8_SkColorType
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_alphaType'>alphaType</a>() <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_colorType'>colorType</a>

<a name='SkBitmap_alphaType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkBitmap_alphaType'>alphaType</a>() <a href='#SkBitmap_alphaType'>const</a>
</pre>

Returns <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, <a href='#Image_Info_Alpha_Type'>one</a> <a href='#Image_Info_Alpha_Type'>of</a>: <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>
.

### Return Value

<a href='#Image_Info_Alpha_Type'>Alpha_Type</a> <a href='#Image_Info_Alpha_Type'>in</a> <a href='#Image_Info'>Image_Info</a>

### Example

<div><fiddle-embed name="070b1a60232be499eb10c6ea62371804">

#### Example Output

~~~~
alpha type: kPremul_SkAlphaType
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_colorType'>colorType</a>() <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_alphaType'>alphaType</a>

<a name='SkBitmap_colorSpace'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='#SkBitmap_colorSpace'>colorSpace</a>() <a href='#SkBitmap_colorSpace'>const</a>
</pre>

Returns <a href='undocumented#SkColorSpace'>SkColorSpace</a>, <a href='undocumented#SkColorSpace'>the</a> <a href='undocumented#SkColorSpace'>range</a> <a href='undocumented#SkColorSpace'>of</a> <a href='undocumented#SkColorSpace'>colors</a>, <a href='undocumented#SkColorSpace'>associated</a> <a href='undocumented#SkColorSpace'>with</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>. <a href='SkImageInfo_Reference#SkImageInfo'>The</a>
reference count of <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>is</a> <a href='undocumented#SkColorSpace'>unchanged</a>. <a href='undocumented#SkColorSpace'>The</a> <a href='undocumented#SkColorSpace'>returned</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>is</a>
immutable.

### Return Value

<a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>in</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>or</a> <a href='SkImageInfo_Reference#SkImageInfo'>nullptr</a>

### Example

<div><fiddle-embed name="817f95879fadba44baf87ea60e9b595a"><div><a href='undocumented#SkColorSpace'>SkColorSpace</a>::<a href='#SkColorSpace_MakeSRGBLinear'>MakeSRGBLinear</a> <a href='#SkColorSpace_MakeSRGBLinear'>creates</a> <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>with</a> <a href='#Color_Space'>linear</a> <a href='#Color_Space'>gamma</a>
<a href='#Color_Space'>and</a> <a href='#Color_Space'>an</a> <a href='#Color_Space'>sRGB</a> <a href='#Color_Space'>gamut</a>. <a href='#Color_Space'>This</a> <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>gamma</a> <a href='#Color_Space'>is</a> <a href='#Color_Space'>not</a> <a href='#Color_Space'>close</a> <a href='#Color_Space'>to</a> <a href='#Color_Space'>sRGB</a> <a href='#Color_Space'>gamma</a>.
</div>

#### Example Output

~~~~
gammaCloseToSRGB: false  gammaIsLinear: true  isSRGB: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#Color_Space'>Color_Space</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_colorSpace'>colorSpace</a>

<a name='SkBitmap_refColorSpace'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&<a href='undocumented#SkColorSpace'>gt</a>; <a href='#SkBitmap_refColorSpace'>refColorSpace</a>() <a href='#SkBitmap_refColorSpace'>const</a>
</pre>

Returns smart pointer to <a href='undocumented#SkColorSpace'>SkColorSpace</a>, <a href='undocumented#SkColorSpace'>the</a> <a href='undocumented#SkColorSpace'>range</a> <a href='undocumented#SkColorSpace'>of</a> <a href='undocumented#SkColorSpace'>colors</a>, <a href='undocumented#SkColorSpace'>associated</a> <a href='undocumented#SkColorSpace'>with</a>
<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>. <a href='SkImageInfo_Reference#SkImageInfo'>The</a> <a href='SkImageInfo_Reference#SkImageInfo'>smart</a> <a href='SkImageInfo_Reference#SkImageInfo'>pointer</a> <a href='SkImageInfo_Reference#SkImageInfo'>tracks</a> <a href='SkImageInfo_Reference#SkImageInfo'>the</a> <a href='SkImageInfo_Reference#SkImageInfo'>number</a> <a href='SkImageInfo_Reference#SkImageInfo'>of</a> <a href='SkImageInfo_Reference#SkImageInfo'>objects</a> <a href='SkImageInfo_Reference#SkImageInfo'>sharing</a> <a href='SkImageInfo_Reference#SkImageInfo'>this</a>
<a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>reference</a> <a href='undocumented#SkColorSpace'>so</a> <a href='undocumented#SkColorSpace'>the</a> <a href='undocumented#SkColorSpace'>memory</a> <a href='undocumented#SkColorSpace'>is</a> <a href='undocumented#SkColorSpace'>released</a> <a href='undocumented#SkColorSpace'>when</a> <a href='undocumented#SkColorSpace'>the</a> <a href='undocumented#SkColorSpace'>owners</a> <a href='undocumented#SkColorSpace'>destruct</a>.

The returned <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>is</a> <a href='undocumented#SkColorSpace'>immutable</a>.

### Return Value

<a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>in</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>wrapped</a> <a href='SkImageInfo_Reference#SkImageInfo'>in</a> <a href='SkImageInfo_Reference#SkImageInfo'>a</a> <a href='SkImageInfo_Reference#SkImageInfo'>smart</a> <a href='SkImageInfo_Reference#SkImageInfo'>pointer</a>

### Example

<div><fiddle-embed name="cb028b7931da85b949ad0953b9711f9f">

#### Example Output

~~~~
gammaCloseToSRGB: false  gammaIsLinear: true  isSRGB: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#Color_Space'>Color_Space</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_colorSpace'>colorSpace</a>

<a name='SkBitmap_bytesPerPixel'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkBitmap_bytesPerPixel'>bytesPerPixel</a>() <a href='#SkBitmap_bytesPerPixel'>const</a>
</pre>

Returns number of bytes per <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>required</a> <a href='undocumented#Pixel'>by</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.
Returns zero if <a href='#SkBitmap_colorType'>colorType</a>( <a href='#SkBitmap_colorType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>.

### Return Value

bytes in <a href='undocumented#Pixel'>pixel</a>

### Example

<div><fiddle-embed name="2a688e6f0a516c0d44a826381e9d637f"><a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>

#### Example Output

~~~~
color: kUnknown_SkColorType      bytesPerPixel: 0
color: kAlpha_8_SkColorType      bytesPerPixel: 1
color: kRGB_565_SkColorType      bytesPerPixel: 2
color: kARGB_4444_SkColorType    bytesPerPixel: 2
color: kRGBA_8888_SkColorType    bytesPerPixel: 4
color: kRGB_888x_SkColorType     bytesPerPixel: 4
color: kBGRA_8888_SkColorType    bytesPerPixel: 4
color: kRGBA_1010102_SkColorType bytesPerPixel: 4
color: kRGB_101010x_SkColorType  bytesPerPixel: 4
color: kGray_8_SkColorType       bytesPerPixel: 1
color: kRGBA_F16_SkColorType     bytesPerPixel: 8
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_rowBytes'>rowBytes</a> <a href='#SkBitmap_rowBytesAsPixels'>rowBytesAsPixels</a> <a href='#SkBitmap_width'>width</a> <a href='#SkBitmap_shiftPerPixel'>shiftPerPixel</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>

<a name='SkBitmap_rowBytesAsPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkBitmap_rowBytesAsPixels'>rowBytesAsPixels</a>() <a href='#SkBitmap_rowBytesAsPixels'>const</a>
</pre>

Returns number of pixels that fit on row. Should be greater than or equal to
<a href='#SkBitmap_width'>width()</a>.

### Return Value

maximum pixels per row

### Example

<div><fiddle-embed name="03a9e08082a23a98de17c3e24871d61a">

#### Example Output

~~~~
rowBytes: 4 rowBytesAsPixels: 1
rowBytes: 5 rowBytesAsPixels: 1
rowBytes: 6 rowBytesAsPixels: 1
rowBytes: 7 rowBytesAsPixels: 1
rowBytes: 8 rowBytesAsPixels: 2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_rowBytes'>rowBytes</a> <a href='#SkBitmap_shiftPerPixel'>shiftPerPixel</a> <a href='#SkBitmap_width'>width</a> <a href='#SkBitmap_bytesPerPixel'>bytesPerPixel</a>

<a name='SkBitmap_shiftPerPixel'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkBitmap_shiftPerPixel'>shiftPerPixel</a>() <a href='#SkBitmap_shiftPerPixel'>const</a>
</pre>

Returns bit shift converting row bytes to row pixels.
Returns zero for <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>.

### Return Value

one of: 0, 1, 2, 3; left shift to convert pixels to bytes

### Example

<div><fiddle-embed name="56ede4b7d45c15d5936f81ac3d74f070"><a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>

#### Example Output

~~~~
color: kUnknown_SkColorType       shiftPerPixel: 0
color: kAlpha_8_SkColorType       shiftPerPixel: 0
color: kRGB_565_SkColorType       shiftPerPixel: 1
color: kARGB_4444_SkColorType     shiftPerPixel: 1
color: kRGBA_8888_SkColorType     shiftPerPixel: 2
color: kRGB_888x_SkColorType      shiftPerPixel: 2
color: kBGRA_8888_SkColorType     shiftPerPixel: 2
color: kRGBA_1010102_SkColorType  shiftPerPixel: 2
color: kRGB_101010x_SkColorType   shiftPerPixel: 2
color: kGray_8_SkColorType        shiftPerPixel: 0
color: kRGBA_F16_SkColorType      shiftPerPixel: 3
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_rowBytes'>rowBytes</a> <a href='#SkBitmap_rowBytesAsPixels'>rowBytesAsPixels</a> <a href='#SkBitmap_width'>width</a> <a href='#SkBitmap_bytesPerPixel'>bytesPerPixel</a>

<a name='SkBitmap_empty'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_empty'>empty()</a> <a href='#SkBitmap_empty'>const</a>
</pre>

Returns true if either <a href='#SkBitmap_width'>width()</a> <a href='#SkBitmap_width'>or</a> <a href='#SkBitmap_height'>height()</a> <a href='#SkBitmap_height'>are</a> <a href='#SkBitmap_height'>zero</a>.

Does not check if <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>is</a> <a href='undocumented#SkPixelRef'>nullptr</a>; <a href='undocumented#SkPixelRef'>call</a> <a href='#SkBitmap_drawsNothing'>drawsNothing</a>() <a href='#SkBitmap_drawsNothing'>to</a> <a href='#SkBitmap_drawsNothing'>check</a> <a href='#SkBitmap_width'>width()</a>,
<a href='#SkBitmap_height'>height()</a>, <a href='#SkBitmap_height'>and</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>.

### Return Value

true if dimensions do not enclose area

### Example

<div><fiddle-embed name="a3762c2722b56ba55e42689c527f146c">

#### Example Output

~~~~
width: 0 height: 0 empty: true
width: 0 height: 2 empty: true
width: 2 height: 0 empty: true
width: 2 height: 2 empty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_height'>height()</a> <a href='#SkBitmap_width'>width()</a> <a href='#SkBitmap_drawsNothing'>drawsNothing</a>

<a name='SkBitmap_isNull'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_isNull'>isNull</a>() <a href='#SkBitmap_isNull'>const</a>
</pre>

Returns true if <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>is</a> <a href='undocumented#SkPixelRef'>nullptr</a>.

Does not check if <a href='#SkBitmap_width'>width()</a> <a href='#SkBitmap_width'>or</a> <a href='#SkBitmap_height'>height()</a> <a href='#SkBitmap_height'>are</a> <a href='#SkBitmap_height'>zero</a>; <a href='#SkBitmap_height'>call</a> <a href='#SkBitmap_drawsNothing'>drawsNothing</a>() <a href='#SkBitmap_drawsNothing'>to</a> <a href='#SkBitmap_drawsNothing'>check</a>
<a href='#SkBitmap_width'>width()</a>, <a href='#SkBitmap_height'>height()</a>, <a href='#SkBitmap_height'>and</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>.

### Return Value

true if no <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>is</a> <a href='undocumented#SkPixelRef'>associated</a>

### Example

<div><fiddle-embed name="211ec89418011aa6e54aa2cc9567e003">

#### Example Output

~~~~
empty bitmap does not have pixels
bitmap with dimensions does not have pixels
allocated bitmap does have pixels
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_empty'>empty()</a> <a href='#SkBitmap_drawsNothing'>drawsNothing</a> <a href='#SkBitmap_pixelRef'>pixelRef</a>

<a name='SkBitmap_drawsNothing'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_drawsNothing'>drawsNothing</a>() <a href='#SkBitmap_drawsNothing'>const</a>
</pre>

Returns true if <a href='#SkBitmap_width'>width()</a> <a href='#SkBitmap_width'>or</a> <a href='#SkBitmap_height'>height()</a> <a href='#SkBitmap_height'>are</a> <a href='#SkBitmap_height'>zero</a>, <a href='#SkBitmap_height'>or</a> <a href='#SkBitmap_height'>if</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>is</a> <a href='undocumented#SkPixelRef'>nullptr</a>.
If true, <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>has</a> <a href='SkBitmap_Reference#SkBitmap'>no</a> <a href='SkBitmap_Reference#SkBitmap'>effect</a> <a href='SkBitmap_Reference#SkBitmap'>when</a> <a href='SkBitmap_Reference#SkBitmap'>drawn</a> <a href='SkBitmap_Reference#SkBitmap'>or</a> <a href='SkBitmap_Reference#SkBitmap'>drawn</a> <a href='SkBitmap_Reference#SkBitmap'>into</a>.

### Return Value

true if drawing has no effect

### Example

<div><fiddle-embed name="daacf43394ce4045a362a48b5774deed">

#### Example Output

~~~~
empty:true  isNull:true  drawsNothing:true
empty:true  isNull:false drawsNothing:true
empty:false isNull:true  drawsNothing:true
empty:false isNull:false drawsNothing:false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_empty'>empty()</a> <a href='#SkBitmap_isNull'>isNull</a> <a href='#SkBitmap_pixelRef'>pixelRef</a>

<a name='SkBitmap_rowBytes'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkBitmap_rowBytes'>rowBytes</a>() <a href='#SkBitmap_rowBytes'>const</a>
</pre>

Returns row bytes, the interval from one <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>to</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>next</a>. <a href='undocumented#Pixel'>Row</a> <a href='undocumented#Pixel'>bytes</a>
<a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>at</a> <a href='undocumented#Pixel'>least</a> <a href='undocumented#Pixel'>as</a> <a href='undocumented#Pixel'>large</a> <a href='undocumented#Pixel'>as</a>: <code><a href='#SkBitmap_width'>width()</a> * <a href='#SkBitmap_info'>info()</a>.<a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>()</code>.

Returns zero if <a href='#SkBitmap_colorType'>colorType</a> <a href='#SkBitmap_colorType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kUnknown_SkColorType'>or</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>if</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>row</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>bytes</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>supplied</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>to</a>
<a href='#SkBitmap_setInfo'>setInfo</a> <a href='#SkBitmap_setInfo'>is</a> <a href='#SkBitmap_setInfo'>not</a> <a href='#SkBitmap_setInfo'>large</a> <a href='#SkBitmap_setInfo'>enough</a> <a href='#SkBitmap_setInfo'>to</a> <a href='#SkBitmap_setInfo'>hold</a> <a href='#SkBitmap_setInfo'>a</a> <a href='#SkBitmap_setInfo'>row</a> <a href='#SkBitmap_setInfo'>of</a> <a href='#SkBitmap_setInfo'>pixels</a>.

### Return Value

byte length of <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a>

### Example

<div><fiddle-embed name="a654fd0b73f424859ae6c95e03f55099">

#### Example Output

~~~~
setInfo returned:false rowBytes:0
setInfo returned:true  rowBytes:8
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_info'>info()</a> <a href='#SkBitmap_setInfo'>setInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>

<a name='SkBitmap_setAlphaType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_setAlphaType'>setAlphaType</a>(<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkBitmap_alphaType'>alphaType</a>)
</pre>

Sets <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>if</a> <a href='#SkBitmap_setAlphaType_alphaType'>alphaType</a> <a href='#SkBitmap_setAlphaType_alphaType'>is</a> <a href='#SkBitmap_setAlphaType_alphaType'>compatible</a> <a href='#SkBitmap_setAlphaType_alphaType'>with</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.
Returns true unless <a href='#SkBitmap_setAlphaType_alphaType'>alphaType</a> <a href='#SkBitmap_setAlphaType_alphaType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>and</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>current</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>
is not <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>.

Returns true if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>. <a href='#SkBitmap_setAlphaType_alphaType'>alphaType</a> <a href='#SkBitmap_setAlphaType_alphaType'>is</a> <a href='#SkBitmap_setAlphaType_alphaType'>ignored</a>, <a href='#SkBitmap_setAlphaType_alphaType'>and</a>
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>remains</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>.

Returns true if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>or</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>.
<a href='#SkBitmap_setAlphaType_alphaType'>alphaType</a> <a href='#SkBitmap_setAlphaType_alphaType'>is</a> <a href='#SkBitmap_setAlphaType_alphaType'>ignored</a>, <a href='#SkBitmap_setAlphaType_alphaType'>and</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>remains</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>.

If <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>or</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>: <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>returns</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>true</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>unless</a>
<a href='#SkBitmap_setAlphaType_alphaType'>alphaType</a> <a href='#SkBitmap_setAlphaType_alphaType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>and</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>is</a> <a href='SkImageInfo_Reference#SkAlphaType'>not</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>.
If <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='#SkBitmap_setAlphaType_alphaType'>alphaType</a> <a href='#SkBitmap_setAlphaType_alphaType'>is</a> <a href='#SkBitmap_setAlphaType_alphaType'>ignored</a>.

If <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>returns</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>true</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>unless</a>
<a href='#SkBitmap_setAlphaType_alphaType'>alphaType</a> <a href='#SkBitmap_setAlphaType_alphaType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>and</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>is</a> <a href='SkImageInfo_Reference#SkAlphaType'>not</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>.
If <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='#SkBitmap_setAlphaType_alphaType'>alphaType</a> <a href='#SkBitmap_setAlphaType_alphaType'>is</a> <a href='#SkBitmap_setAlphaType_alphaType'>ignored</a>. <a href='#SkBitmap_setAlphaType_alphaType'>If</a> <a href='#SkBitmap_setAlphaType_alphaType'>alphaType</a> <a href='#SkBitmap_setAlphaType_alphaType'>is</a>
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>, <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>it</a> <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>is</a> <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>treated</a> <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>as</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>.

This changes <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>in</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>; <a href='undocumented#SkPixelRef'>all</a> <a href='SkBitmap_Reference#Bitmap'>bitmaps</a> <a href='SkBitmap_Reference#Bitmap'>sharing</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>
are affected.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_setAlphaType_alphaType'><code><strong>alphaType</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>

### Return Value

true if <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>is</a> <a href='SkImageInfo_Reference#SkAlphaType'>set</a>

### Example

<div><fiddle-embed name="af3adcbea7b58bf90298ca5e0ea93030"><a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>
<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>
</fiddle-embed></div>

### See Also

<a href='#Image_Info_Alpha_Type'>Alpha_Type</a> <a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Image_Info'>Image_Info</a> <a href='#SkBitmap_setInfo'>setInfo</a>

<a name='SkBitmap_getPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void* <a href='#SkBitmap_getPixels'>getPixels</a>() <a href='#SkBitmap_getPixels'>const</a>
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a>, <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>base</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>corresponding</a> <a href='undocumented#Pixel'>to</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>origin</a>.

### Return Value

<a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a>

### Example

<div><fiddle-embed name="e006bb05cf74ec8d2b3d6adeb5dba11b">

#### Example Output

~~~~
bitmap.getColor(0, 1) == 0x00000000
bitmap.getColor(0, 0) == 0xFFFFFFFF
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_isNull'>isNull</a> <a href='#SkBitmap_drawsNothing'>drawsNothing</a>

<a name='SkBitmap_computeByteSize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkBitmap_computeByteSize'>computeByteSize</a>() <a href='#SkBitmap_computeByteSize'>const</a>
</pre>

Returns minimum memory required for <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>storage</a>.
Does not include unused memory on last row when <a href='#SkBitmap_rowBytesAsPixels'>rowBytesAsPixels</a>() <a href='#SkBitmap_rowBytesAsPixels'>exceeds</a> <a href='#SkBitmap_width'>width()</a>.
Returns zero if result does not fit in size_t.
Returns zero if <a href='#SkBitmap_height'>height()</a> <a href='#SkBitmap_height'>or</a> <a href='#SkBitmap_width'>width()</a> <a href='#SkBitmap_width'>is</a> 0.
Returns <a href='#SkBitmap_height'>height()</a> <a href='#SkBitmap_height'>times</a> <a href='#SkBitmap_rowBytes'>rowBytes</a>() <a href='#SkBitmap_rowBytes'>if</a> <a href='#SkBitmap_colorType'>colorType</a>() <a href='#SkBitmap_colorType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>.

### Return Value

<a href='undocumented#Size'>size</a> <a href='undocumented#Size'>in</a> <a href='undocumented#Size'>bytes</a> <a href='undocumented#Size'>of</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>buffer</a>

### Example

<div><fiddle-embed name="165c8f208829fc0908e8a50da60c0076">

#### Example Output

~~~~
width:       1 height:       1 computeByteSize:             4
width:       1 height:    1000 computeByteSize:          4999
width:       1 height: 1000000 computeByteSize:       4999999
width:    1000 height:       1 computeByteSize:          4000
width:    1000 height:    1000 computeByteSize:       4999000
width:    1000 height: 1000000 computeByteSize:    4999999000
width: 1000000 height:       1 computeByteSize:       4000000
width: 1000000 height:    1000 computeByteSize:    4999000000
width: 1000000 height: 1000000 computeByteSize: 4999999000000
~~~~

</fiddle-embed></div>

### See Also

<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_computeByteSize'>computeByteSize</a>

<a name='SkBitmap_isImmutable'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_isImmutable'>isImmutable</a>() <a href='#SkBitmap_isImmutable'>const</a>
</pre>

Returns true if pixels can not change.

Most immutable <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>checks</a> <a href='SkBitmap_Reference#SkBitmap'>trigger</a> <a href='SkBitmap_Reference#SkBitmap'>an</a> <a href='SkBitmap_Reference#SkBitmap'>assert</a> <a href='SkBitmap_Reference#SkBitmap'>only</a> <a href='SkBitmap_Reference#SkBitmap'>on</a> <a href='SkBitmap_Reference#SkBitmap'>debug</a> <a href='SkBitmap_Reference#SkBitmap'>builds</a>.

### Return Value

true if pixels are immutable

### Example

<div><fiddle-embed name="db61fdcd382342ee88ea1b4f27c27b95">

#### Example Output

~~~~
original is immutable
copy is immutable
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_setImmutable'>setImmutable</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>::<a href='#SkPixelRef_isImmutable'>isImmutable</a> <a href='SkImage_Reference#SkImage'>SkImage</a>

<a name='SkBitmap_setImmutable'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_setImmutable'>setImmutable</a>()
</pre>

Sets internal flag to mark <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>as</a> <a href='SkBitmap_Reference#SkBitmap'>immutable</a>. <a href='SkBitmap_Reference#SkBitmap'>Once</a> <a href='SkBitmap_Reference#SkBitmap'>set</a>, <a href='SkBitmap_Reference#SkBitmap'>pixels</a> <a href='SkBitmap_Reference#SkBitmap'>can</a> <a href='SkBitmap_Reference#SkBitmap'>not</a> <a href='SkBitmap_Reference#SkBitmap'>change</a>.
Any other <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='SkBitmap_Reference#Bitmap'>sharing</a> <a href='SkBitmap_Reference#Bitmap'>the</a> <a href='SkBitmap_Reference#Bitmap'>same</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>are</a> <a href='undocumented#SkPixelRef'>also</a> <a href='undocumented#SkPixelRef'>marked</a> <a href='undocumented#SkPixelRef'>as</a> <a href='undocumented#SkPixelRef'>immutable</a>.
Once <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>is</a> <a href='undocumented#SkPixelRef'>marked</a> <a href='undocumented#SkPixelRef'>immutable</a>, <a href='undocumented#SkPixelRef'>the</a> <a href='undocumented#SkPixelRef'>setting</a> <a href='undocumented#SkPixelRef'>cannot</a> <a href='undocumented#SkPixelRef'>be</a> <a href='undocumented#SkPixelRef'>cleared</a>.

Writing to immutable <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>pixels</a> <a href='SkBitmap_Reference#SkBitmap'>triggers</a> <a href='SkBitmap_Reference#SkBitmap'>an</a> <a href='SkBitmap_Reference#SkBitmap'>assert</a> <a href='SkBitmap_Reference#SkBitmap'>on</a> <a href='SkBitmap_Reference#SkBitmap'>debug</a> <a href='SkBitmap_Reference#SkBitmap'>builds</a>.

### Example

<div><fiddle-embed name="9210060d1f4ca46e1375496237902ef3"><div>Triggers assert if SK_DEBUG is true, runs fine otherwise.
</div></fiddle-embed></div>

### See Also

<a href='#SkBitmap_isImmutable'>isImmutable</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>::<a href='#SkPixelRef_setImmutable'>setImmutable</a> <a href='SkImage_Reference#SkImage'>SkImage</a>

<a name='SkBitmap_isOpaque'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_isOpaque'>isOpaque</a>() <a href='#SkBitmap_isOpaque'>const</a>
</pre>

Returns true if <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>is</a> <a href='SkImageInfo_Reference#SkAlphaType'>set</a> <a href='SkImageInfo_Reference#SkAlphaType'>to</a> <a href='SkImageInfo_Reference#SkAlphaType'>hint</a> <a href='SkImageInfo_Reference#SkAlphaType'>that</a> <a href='SkImageInfo_Reference#SkAlphaType'>all</a> <a href='SkImageInfo_Reference#SkAlphaType'>pixels</a> <a href='SkImageInfo_Reference#SkAlphaType'>are</a> <a href='SkImageInfo_Reference#SkAlphaType'>opaque</a>; <a href='SkImageInfo_Reference#SkAlphaType'>their</a>
<a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>value</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>implicitly</a> <a href='SkColor_Reference#Alpha'>or</a> <a href='SkColor_Reference#Alpha'>explicitly</a> 1.0. <a href='SkColor_Reference#Alpha'>If</a> <a href='SkColor_Reference#Alpha'>true</a>, <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>all</a> <a href='SkColor_Reference#Alpha'>pixels</a> <a href='SkColor_Reference#Alpha'>are</a>
not opaque, Skia may draw incorrectly.

Does not check if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>allows</a> <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='SkColor_Reference#Alpha'>or</a> <a href='SkColor_Reference#Alpha'>if</a> <a href='SkColor_Reference#Alpha'>any</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>value</a> <a href='undocumented#Pixel'>has</a>
transparency.

### Return Value

true if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>is</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>

### Example

<div><fiddle-embed name="5e76b68bb46d54315eb0c12d83bd6949"><div><a href='#SkBitmap_isOpaque'>isOpaque</a> <a href='#SkBitmap_isOpaque'>ignores</a> <a href='#SkBitmap_isOpaque'>whether</a> <a href='#SkBitmap_isOpaque'>all</a> <a href='#SkBitmap_isOpaque'>pixels</a> <a href='#SkBitmap_isOpaque'>are</a> <a href='#SkBitmap_isOpaque'>opaque</a> <a href='#SkBitmap_isOpaque'>or</a> <a href='#SkBitmap_isOpaque'>not</a>.
</div>

#### Example Output

~~~~
isOpaque: false
isOpaque: false
isOpaque: true
isOpaque: true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_ComputeIsOpaque'>ComputeIsOpaque</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_isOpaque'>isOpaque</a>

<a name='SkBitmap_isVolatile'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_isVolatile'>isVolatile</a>() <a href='#SkBitmap_isVolatile'>const</a>
</pre>

Provides a hint to caller that pixels should not be cached. Only true if
<a href='#SkBitmap_setIsVolatile'>setIsVolatile</a>() <a href='#SkBitmap_setIsVolatile'>has</a> <a href='#SkBitmap_setIsVolatile'>been</a> <a href='#SkBitmap_setIsVolatile'>called</a> <a href='#SkBitmap_setIsVolatile'>to</a> <a href='#SkBitmap_setIsVolatile'>mark</a> <a href='#SkBitmap_setIsVolatile'>as</a> <a href='#SkBitmap_setIsVolatile'>volatile</a>.

Volatile state is not shared by other <a href='SkBitmap_Reference#Bitmap'>bitmaps</a> <a href='SkBitmap_Reference#Bitmap'>sharing</a> <a href='SkBitmap_Reference#Bitmap'>the</a> <a href='SkBitmap_Reference#Bitmap'>same</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>.

### Return Value

true if marked volatile

### Example

<div><fiddle-embed name="23c4543ac6cdd0e8fe762816a0dc2e03">

#### Example Output

~~~~
original is volatile
copy is not volatile
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_setIsVolatile'>setIsVolatile</a>

<a name='SkBitmap_setIsVolatile'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_setIsVolatile'>setIsVolatile</a>(<a href='#SkBitmap_setIsVolatile'>bool</a> <a href='#SkBitmap_isVolatile'>isVolatile</a>)
</pre>

Sets if pixels should be read from <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>on</a> <a href='undocumented#SkPixelRef'>every</a> <a href='undocumented#SkPixelRef'>access</a>. <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>are</a> <a href='SkBitmap_Reference#SkBitmap'>not</a>
volatile by default; a GPU back end may upload <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>values</a> <a href='undocumented#Pixel'>expecting</a> <a href='undocumented#Pixel'>them</a> <a href='undocumented#Pixel'>to</a> <a href='undocumented#Pixel'>be</a>
accessed repeatedly. Marking temporary <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>as</a> <a href='SkBitmap_Reference#SkBitmap'>volatile</a> <a href='SkBitmap_Reference#SkBitmap'>provides</a> <a href='SkBitmap_Reference#SkBitmap'>a</a> <a href='SkBitmap_Reference#SkBitmap'>hint</a> <a href='SkBitmap_Reference#SkBitmap'>to</a>
<a href='undocumented#SkBaseDevice'>SkBaseDevice</a> <a href='undocumented#SkBaseDevice'>that</a> <a href='undocumented#SkBaseDevice'>the</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>pixels</a> <a href='SkBitmap_Reference#SkBitmap'>should</a> <a href='SkBitmap_Reference#SkBitmap'>not</a> <a href='SkBitmap_Reference#SkBitmap'>be</a> <a href='SkBitmap_Reference#SkBitmap'>cached</a>. <a href='SkBitmap_Reference#SkBitmap'>This</a> <a href='SkBitmap_Reference#SkBitmap'>can</a>
improve performance by avoiding overhead and reducing resource
consumption on <a href='undocumented#SkBaseDevice'>SkBaseDevice</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_setIsVolatile_isVolatile'><code><strong>isVolatile</strong></code></a></td>
    <td>true if backing pixels are temporary</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="e8627a4df659b896402f89a91326618f"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_setIsVolatile_isVolatile'>isVolatile</a>

<a name='SkBitmap_reset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_reset'>reset()</a>
</pre>

Resets to its initial state; all fields are set to zero, as if <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>had</a>
been initialized by <a href='#SkBitmap_empty_constructor'>SkBitmap()</a>.

Sets width, height, row bytes to zero; <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>to</a> <a href='undocumented#Pixel'>nullptr</a>; <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>to</a>
<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>; <a href='SkImageInfo_Reference#kUnknown_SkColorType'>and</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>to</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>.

If <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>is</a> <a href='undocumented#SkPixelRef'>allocated</a>, <a href='undocumented#SkPixelRef'>its</a> <a href='undocumented#SkPixelRef'>reference</a> <a href='undocumented#SkPixelRef'>count</a> <a href='undocumented#SkPixelRef'>is</a> <a href='undocumented#SkPixelRef'>decreased</a> <a href='undocumented#SkPixelRef'>by</a> <a href='undocumented#SkPixelRef'>one</a>, <a href='undocumented#SkPixelRef'>releasing</a>
its memory if <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>is</a> <a href='SkBitmap_Reference#SkBitmap'>the</a> <a href='SkBitmap_Reference#SkBitmap'>sole</a> <a href='SkBitmap_Reference#SkBitmap'>owner</a>.

### Example

<div><fiddle-embed name="52ccaeda67924373c5b55a2b89fe314d">

#### Example Output

~~~~
width:1 height:1 isNull:false
width:0 height:0 isNull:true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_empty_constructor'>SkBitmap()</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>

<a name='SkBitmap_ComputeIsOpaque'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkBitmap_ComputeIsOpaque'>ComputeIsOpaque</a>(<a href='#SkBitmap_ComputeIsOpaque'>const</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='SkBitmap_Reference#SkBitmap'>bm</a>)
</pre>

Returns true if all pixels are opaque. <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>determines</a> <a href='SkImageInfo_Reference#SkColorType'>how</a> <a href='SkImageInfo_Reference#SkColorType'>pixels</a>
are encoded, and whether <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>describes</a> <a href='SkColor_Reference#Alpha'>alpha</a>. <a href='SkColor_Reference#Alpha'>Returns</a> <a href='SkColor_Reference#Alpha'>true</a> <a href='SkColor_Reference#Alpha'>for</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>
without <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>in</a> <a href='SkColor_Reference#Alpha'>each</a> <a href='undocumented#Pixel'>pixel</a>; <a href='undocumented#Pixel'>for</a> <a href='undocumented#Pixel'>other</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkColorType'>returns</a> <a href='SkImageInfo_Reference#SkColorType'>true</a> <a href='SkImageInfo_Reference#SkColorType'>if</a> <a href='SkImageInfo_Reference#SkColorType'>all</a>
pixels have <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>values</a> <a href='SkColor_Reference#Alpha'>equivalent</a> <a href='SkColor_Reference#Alpha'>to</a> 1.0 <a href='SkColor_Reference#Alpha'>or</a> <a href='SkColor_Reference#Alpha'>greater</a>.

For <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>or</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>: <a href='SkImageInfo_Reference#kGray_8_SkColorType'>always</a>
returns true. For <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>,
<a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>: <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>returns</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>true</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>if</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>all</a> <a href='undocumented#Pixel'>pixel</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>values</a> <a href='SkColor_Reference#Alpha'>are</a> 255.
For <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>: <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>returns</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>true</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>if</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>all</a> <a href='undocumented#Pixel'>pixel</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>values</a> <a href='SkColor_Reference#Alpha'>are</a> 15.
For <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>: <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>returns</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>true</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>if</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>all</a> <a href='undocumented#Pixel'>pixel</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>values</a> <a href='SkColor_Reference#Alpha'>are</a> 1.0 <a href='SkColor_Reference#Alpha'>or</a>
greater.

Returns false for <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_ComputeIsOpaque_bm'><code><strong>bm</strong></code></a></td>
    <td><a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>to</a> <a href='SkBitmap_Reference#SkBitmap'>check</a></td>
  </tr>
</table>

### Return Value

true if all pixels have opaque values or <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>opaque</a>

### Example

<div><fiddle-embed name="9df1baa17658fbd0c419780f26fd854f">

#### Example Output

~~~~
computeIsOpaque: false
computeIsOpaque: true
computeIsOpaque: false
computeIsOpaque: true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_isOpaque'>isOpaque</a> <a href='#Image_Info_Color_Type'>Color_Type</a> <a href='SkColor_Reference#Alpha'>Alpha</a>

<a name='SkBitmap_getBounds'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_getBounds'>getBounds</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a>) <a href='SkRect_Reference#SkRect'>const</a>
</pre>

Returns <a href='SkRect_Reference#SkRect'>SkRect</a> { 0, 0, <a href='#SkBitmap_width'>width()</a>, <a href='#SkBitmap_height'>height()</a> }.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_getBounds_bounds'><code><strong>bounds</strong></code></a></td>
    <td>container for floating <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>rectangle</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="2431ebc7e7d1e91e6d9daafd0f7a478f"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_bounds'>bounds()</a>

<a name='SkBitmap_getBounds_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_getBounds'>getBounds</a>(<a href='SkIRect_Reference#SkIRect'>SkIRect</a>* <a href='SkIRect_Reference#SkIRect'>bounds</a>) <a href='SkIRect_Reference#SkIRect'>const</a>
</pre>

Returns <a href='SkIRect_Reference#SkIRect'>SkIRect</a> { 0, 0, <a href='#SkBitmap_width'>width()</a>, <a href='#SkBitmap_height'>height()</a> }.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_getBounds_2_bounds'><code><strong>bounds</strong></code></a></td>
    <td>container for integral rectangle</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0c45da35172bc0a529b2faecddae62a2"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_bounds'>bounds()</a>

<a name='SkBitmap_bounds'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkBitmap_bounds'>bounds()</a> <a href='#SkBitmap_bounds'>const</a>
</pre>

Returns <a href='SkIRect_Reference#SkIRect'>SkIRect</a> { 0, 0, <a href='#SkBitmap_width'>width()</a>, <a href='#SkBitmap_height'>height()</a> }.

### Return Value

integral rectangle from origin to <a href='#SkBitmap_width'>width()</a> <a href='#SkBitmap_width'>and</a> <a href='#SkBitmap_height'>height()</a>

### Example

<div><fiddle-embed name="3e9126152ff1cc592aef6facbcb5fc96"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_getBounds'>getBounds</a>

<a name='SkBitmap_dimensions'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkISize'>SkISize</a> <a href='#SkBitmap_dimensions'>dimensions()</a> <a href='#SkBitmap_dimensions'>const</a>
</pre>

Returns <a href='undocumented#SkISize'>SkISize</a> { <a href='#SkBitmap_width'>width()</a>, <a href='#SkBitmap_height'>height()</a> }.

### Return Value

integral <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='#SkBitmap_width'>width()</a> <a href='#SkBitmap_width'>and</a> <a href='#SkBitmap_height'>height()</a>

### Example

<div><fiddle-embed name="647056bcc12c27fb4413f212f33a2898"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_height'>height</a> <a href='#SkBitmap_width'>width</a>

<a name='SkBitmap_getSubset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkBitmap_getSubset'>getSubset</a>() <a href='#SkBitmap_getSubset'>const</a>
</pre>

Returns the bounds of this <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='SkBitmap_Reference#Bitmap'>offset</a> <a href='SkBitmap_Reference#Bitmap'>by</a> <a href='SkBitmap_Reference#Bitmap'>its</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>origin</a>.

### Return Value

bounds within <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>bounds</a>

### Example

<div><fiddle-embed name="d6dd0b425aa550f21b938a18c2e1a981">

#### Example Output

~~~~
source: 0, 0, 512, 512
subset: 100, 100, 412, 412
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_extractSubset'>extractSubset</a> <a href='#SkBitmap_getBounds'>getBounds</a>

<a name='SkBitmap_setInfo'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_setInfo'>setInfo</a>(<a href='#SkBitmap_setInfo'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>imageInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='#SkBitmap_rowBytes'>rowBytes</a> = 0)
</pre>

Sets width, height, <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Color_Space'>Color_Space</a>, <a href='#Color_Space'>and</a> <a href='#Color_Space'>optional</a>
<a href='#SkBitmap_setInfo_rowBytes'>rowBytes</a>. <a href='#SkBitmap_setInfo_rowBytes'>Frees</a> <a href='#SkBitmap_setInfo_rowBytes'>pixels</a>, <a href='#SkBitmap_setInfo_rowBytes'>and</a> <a href='#SkBitmap_setInfo_rowBytes'>returns</a> <a href='#SkBitmap_setInfo_rowBytes'>true</a> <a href='#SkBitmap_setInfo_rowBytes'>if</a> <a href='#SkBitmap_setInfo_rowBytes'>successful</a>.

<a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() <a href='#SkImageInfo_alphaType'>may</a> <a href='#SkImageInfo_alphaType'>be</a> <a href='#SkImageInfo_alphaType'>altered</a> <a href='#SkImageInfo_alphaType'>to</a> <a href='#SkImageInfo_alphaType'>a</a> <a href='#SkImageInfo_alphaType'>value</a> <a href='#SkImageInfo_alphaType'>permitted</a> <a href='#SkImageInfo_alphaType'>by</a> <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_colorSpace'>colorSpace</a>().
<a href='#SkImageInfo_colorSpace'>If</a> <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_colorType'>colorType</a>() <a href='#SkImageInfo_colorType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() <a href='#SkImageInfo_alphaType'>is</a>
<a href='#SkImageInfo_alphaType'>set</a> <a href='#SkImageInfo_alphaType'>to</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>.
<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>If</a> <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_colorType'>colorType</a>() <a href='#SkImageInfo_colorType'>is</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>and</a> <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() <a href='#SkImageInfo_alphaType'>is</a>
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>, <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() <a href='#SkImageInfo_alphaType'>is</a> <a href='#SkImageInfo_alphaType'>replaced</a> <a href='#SkImageInfo_alphaType'>by</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>.
<a href='SkImageInfo_Reference#kPremul_SkAlphaType'>If</a> <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_colorType'>colorType</a>() <a href='#SkImageInfo_colorType'>is</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>or</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>,
<a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() <a href='#SkImageInfo_alphaType'>is</a> <a href='#SkImageInfo_alphaType'>set</a> <a href='#SkImageInfo_alphaType'>to</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>.
<a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>If</a> <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_colorType'>colorType</a>() <a href='#SkImageInfo_colorType'>is</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>or</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>: <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() <a href='#SkImageInfo_alphaType'>remains</a>
<a href='#SkImageInfo_alphaType'>unchanged</a>.

<a href='#SkBitmap_setInfo_rowBytes'>rowBytes</a> <a href='#SkBitmap_setInfo_rowBytes'>must</a> <a href='#SkBitmap_setInfo_rowBytes'>equal</a> <a href='#SkBitmap_setInfo_rowBytes'>or</a> <a href='#SkBitmap_setInfo_rowBytes'>exceed</a> <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>(). <a href='#SkImageInfo_minRowBytes'>If</a> <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_colorSpace'>colorSpace</a>() <a href='#SkImageInfo_colorSpace'>is</a>
<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='#SkBitmap_setInfo_rowBytes'>rowBytes</a> <a href='#SkBitmap_setInfo_rowBytes'>is</a> <a href='#SkBitmap_setInfo_rowBytes'>ignored</a> <a href='#SkBitmap_setInfo_rowBytes'>and</a> <a href='#SkBitmap_setInfo_rowBytes'>treated</a> <a href='#SkBitmap_setInfo_rowBytes'>as</a> <a href='#SkBitmap_setInfo_rowBytes'>zero</a>; <a href='#SkBitmap_setInfo_rowBytes'>for</a> <a href='#SkBitmap_setInfo_rowBytes'>all</a> <a href='#SkBitmap_setInfo_rowBytes'>other</a>
<a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>values</a>, <a href='#SkBitmap_setInfo_rowBytes'>rowBytes</a> <a href='#SkBitmap_setInfo_rowBytes'>of</a> <a href='#SkBitmap_setInfo_rowBytes'>zero</a> <a href='#SkBitmap_setInfo_rowBytes'>is</a> <a href='#SkBitmap_setInfo_rowBytes'>treated</a> <a href='#SkBitmap_setInfo_rowBytes'>as</a> <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>().

<a href='#SkImageInfo_minRowBytes'>Calls</a> <a href='#SkBitmap_reset'>reset()</a> <a href='#SkBitmap_reset'>and</a> <a href='#SkBitmap_reset'>returns</a> <a href='#SkBitmap_reset'>false</a> <a href='#SkBitmap_reset'>if</a>:

<table>  <tr>
    <td><a href='#SkBitmap_setInfo_rowBytes'>rowBytes</a> <a href='#SkBitmap_setInfo_rowBytes'>exceeds</a> 31 <a href='#SkBitmap_setInfo_rowBytes'>bits</a></td>
  </tr>  <tr>
    <td><a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_width'>width()</a> <a href='#SkImageInfo_width'>is</a> <a href='#SkImageInfo_width'>negative</a></td>
  </tr>  <tr>
    <td><a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_height'>height()</a> <a href='#SkImageInfo_height'>is</a> <a href='#SkImageInfo_height'>negative</a></td>
  </tr>  <tr>
    <td><a href='#SkBitmap_setInfo_rowBytes'>rowBytes</a> <a href='#SkBitmap_setInfo_rowBytes'>is</a> <a href='#SkBitmap_setInfo_rowBytes'>positive</a> <a href='#SkBitmap_setInfo_rowBytes'>and</a> <a href='#SkBitmap_setInfo_rowBytes'>less</a> <a href='#SkBitmap_setInfo_rowBytes'>than</a> <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_width'>width()</a> <a href='#SkImageInfo_width'>times</a> <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>()</td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkBitmap_setInfo_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>contains width, height, <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Color_Space'>Color_Space</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_setInfo_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td><a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>() <a href='#SkImageInfo_minRowBytes'>or</a> <a href='#SkImageInfo_minRowBytes'>larger</a>; <a href='#SkImageInfo_minRowBytes'>or</a> <a href='#SkImageInfo_minRowBytes'>zero</a></td>
  </tr>
</table>

### Return Value

true if <a href='#Image_Info'>Image_Info</a> <a href='#Image_Info'>set</a> <a href='#Image_Info'>successfully</a>

### Example

<div><fiddle-embed name="599ab64d0aea005498176249bbfb64eb"></fiddle-embed></div>

### See Also

<a href='#Image_Info_Alpha_Type'>Alpha_Type</a> <a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Color_Space'>Color_Space</a> <a href='#SkBitmap_height'>height</a> <a href='#SkBitmap_setInfo_rowBytes'>rowBytes</a> <a href='#SkBitmap_width'>width</a>

<a name='SkBitmap_AllocFlags'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkBitmap_AllocFlags'>AllocFlags</a> {
        <a href='#SkBitmap_kZeroPixels_AllocFlag'>kZeroPixels_AllocFlag</a> = 1 << 0,
    };
</pre>

<a href='#SkBitmap_AllocFlags'>AllocFlags</a> <a href='#SkBitmap_AllocFlags'>provides</a> <a href='#SkBitmap_AllocFlags'>the</a> <a href='#SkBitmap_AllocFlags'>option</a> <a href='#SkBitmap_AllocFlags'>to</a> <a href='#SkBitmap_AllocFlags'>zero</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a> <a href='undocumented#Pixel'>when</a> <a href='undocumented#Pixel'>allocated</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBitmap_kZeroPixels_AllocFlag'><code>SkBitmap::kZeroPixels_AllocFlag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Instructs <a href='#SkBitmap_tryAllocPixelsFlags'>tryAllocPixelsFlags</a> <a href='#SkBitmap_tryAllocPixelsFlags'>and</a> <a href='#SkBitmap_allocPixelsFlags'>allocPixelsFlags</a> <a href='#SkBitmap_allocPixelsFlags'>to</a> <a href='#SkBitmap_allocPixelsFlags'>zero</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a>.
</td>
  </tr>
</table>

### See Also

<a href='#SkBitmap_tryAllocPixelsFlags'>tryAllocPixelsFlags</a> <a href='#SkBitmap_allocPixelsFlags'>allocPixelsFlags</a> <a href='#SkBitmap_erase'>erase</a> <a href='#SkBitmap_eraseColor'>eraseColor</a>

<a name='Allocate'></a>

<a name='SkBitmap_tryAllocPixelsFlags'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_tryAllocPixelsFlags'>tryAllocPixelsFlags</a>(<a href='#SkBitmap_tryAllocPixelsFlags'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='SkImageInfo_Reference#SkImageInfo'>uint32_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>flags</a>)
</pre>

Sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>to</a> <a href='#SkBitmap_tryAllocPixelsFlags_info'>info</a> <a href='#SkBitmap_tryAllocPixelsFlags_info'>following</a> <a href='#SkBitmap_tryAllocPixelsFlags_info'>the</a> <a href='#SkBitmap_tryAllocPixelsFlags_info'>rules</a> <a href='#SkBitmap_tryAllocPixelsFlags_info'>in</a> <a href='#SkBitmap_setInfo'>setInfo</a>() <a href='#SkBitmap_setInfo'>and</a> <a href='#SkBitmap_setInfo'>allocates</a> <a href='undocumented#Pixel'>pixel</a>
memory. If <a href='#SkBitmap_tryAllocPixelsFlags_flags'>flags</a> <a href='#SkBitmap_tryAllocPixelsFlags_flags'>is</a> <a href='#SkBitmap_kZeroPixels_AllocFlag'>kZeroPixels_AllocFlag</a>, <a href='#SkBitmap_kZeroPixels_AllocFlag'>memory</a> <a href='#SkBitmap_kZeroPixels_AllocFlag'>is</a> <a href='#SkBitmap_kZeroPixels_AllocFlag'>zeroed</a>.

Returns false and calls <a href='#SkBitmap_reset'>reset()</a> <a href='#SkBitmap_reset'>if</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>could</a> <a href='SkImageInfo_Reference#SkImageInfo'>not</a> <a href='SkImageInfo_Reference#SkImageInfo'>be</a> <a href='SkImageInfo_Reference#SkImageInfo'>set</a>, <a href='SkImageInfo_Reference#SkImageInfo'>or</a> <a href='SkImageInfo_Reference#SkImageInfo'>memory</a> <a href='SkImageInfo_Reference#SkImageInfo'>could</a>
not be allocated, or memory could not optionally be zeroed.

On most platforms, allocating <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a> <a href='undocumented#Pixel'>may</a> <a href='undocumented#Pixel'>succeed</a> <a href='undocumented#Pixel'>even</a> <a href='undocumented#Pixel'>though</a> <a href='undocumented#Pixel'>there</a> <a href='undocumented#Pixel'>is</a>
not sufficient memory to hold pixels; allocation does not take place
until the pixels are written to. The actual behavior depends on the platform
implementation of malloc(), if <a href='#SkBitmap_tryAllocPixelsFlags_flags'>flags</a> <a href='#SkBitmap_tryAllocPixelsFlags_flags'>is</a> <a href='#SkBitmap_tryAllocPixelsFlags_flags'>zero</a>, <a href='#SkBitmap_tryAllocPixelsFlags_flags'>and</a> <a href='#SkBitmap_tryAllocPixelsFlags_flags'>calloc()</a>, <a href='#SkBitmap_tryAllocPixelsFlags_flags'>if</a> <a href='#SkBitmap_tryAllocPixelsFlags_flags'>flags</a> <a href='#SkBitmap_tryAllocPixelsFlags_flags'>is</a>
<a href='#SkBitmap_kZeroPixels_AllocFlag'>kZeroPixels_AllocFlag</a>.

<a href='#SkBitmap_tryAllocPixelsFlags_flags'>flags</a> <a href='#SkBitmap_tryAllocPixelsFlags_flags'>set</a> <a href='#SkBitmap_tryAllocPixelsFlags_flags'>to</a> <a href='#SkBitmap_kZeroPixels_AllocFlag'>kZeroPixels_AllocFlag</a> <a href='#SkBitmap_kZeroPixels_AllocFlag'>offers</a> <a href='#SkBitmap_kZeroPixels_AllocFlag'>equal</a> <a href='#SkBitmap_kZeroPixels_AllocFlag'>or</a> <a href='#SkBitmap_kZeroPixels_AllocFlag'>better</a> <a href='#SkBitmap_kZeroPixels_AllocFlag'>performance</a> <a href='#SkBitmap_kZeroPixels_AllocFlag'>than</a>
subsequently calling <a href='#SkBitmap_eraseColor'>eraseColor</a>() <a href='#SkBitmap_eraseColor'>with</a> <a href='SkColor_Reference#SK_ColorTRANSPARENT'>SK_ColorTRANSPARENT</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_tryAllocPixelsFlags_info'><code><strong>info</strong></code></a></td>
    <td>contains width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_tryAllocPixelsFlags_flags'><code><strong>flags</strong></code></a></td>
    <td><a href='#SkBitmap_kZeroPixels_AllocFlag'>kZeroPixels_AllocFlag</a>, <a href='#SkBitmap_kZeroPixels_AllocFlag'>or</a> <a href='#SkBitmap_kZeroPixels_AllocFlag'>zero</a></td>
  </tr>
</table>

### Return Value

true if pixels allocation is successful

### Example

<div><fiddle-embed name="f1d1880d38e0aea4cefd3e11745e8a09">

#### Example Output

~~~~
bitmap allocation succeeded!
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_allocPixelsFlags'>allocPixelsFlags</a> <a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a> <a href='undocumented#SkMallocPixelRef'>SkMallocPixelRef</a>::<a href='#SkMallocPixelRef_MakeZeroed'>MakeZeroed</a>

<a name='SkBitmap_allocPixelsFlags'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_allocPixelsFlags'>allocPixelsFlags</a>(<a href='#SkBitmap_allocPixelsFlags'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='SkImageInfo_Reference#SkImageInfo'>uint32_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>flags</a>)
</pre>

Sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>to</a> <a href='#SkBitmap_allocPixelsFlags_info'>info</a> <a href='#SkBitmap_allocPixelsFlags_info'>following</a> <a href='#SkBitmap_allocPixelsFlags_info'>the</a> <a href='#SkBitmap_allocPixelsFlags_info'>rules</a> <a href='#SkBitmap_allocPixelsFlags_info'>in</a> <a href='#SkBitmap_setInfo'>setInfo</a>() <a href='#SkBitmap_setInfo'>and</a> <a href='#SkBitmap_setInfo'>allocates</a> <a href='undocumented#Pixel'>pixel</a>
memory. If <a href='#SkBitmap_allocPixelsFlags_flags'>flags</a> <a href='#SkBitmap_allocPixelsFlags_flags'>is</a> <a href='#SkBitmap_kZeroPixels_AllocFlag'>kZeroPixels_AllocFlag</a>, <a href='#SkBitmap_kZeroPixels_AllocFlag'>memory</a> <a href='#SkBitmap_kZeroPixels_AllocFlag'>is</a> <a href='#SkBitmap_kZeroPixels_AllocFlag'>zeroed</a>.

Aborts execution if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>could</a> <a href='SkImageInfo_Reference#SkImageInfo'>not</a> <a href='SkImageInfo_Reference#SkImageInfo'>be</a> <a href='SkImageInfo_Reference#SkImageInfo'>set</a>, <a href='SkImageInfo_Reference#SkImageInfo'>or</a> <a href='SkImageInfo_Reference#SkImageInfo'>memory</a> <a href='SkImageInfo_Reference#SkImageInfo'>could</a>
not be allocated, or memory could not optionally
be zeroed. Abort steps may be provided by the user at compile time by defining
SK_ABORT.

On most platforms, allocating <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a> <a href='undocumented#Pixel'>may</a> <a href='undocumented#Pixel'>succeed</a> <a href='undocumented#Pixel'>even</a> <a href='undocumented#Pixel'>though</a> <a href='undocumented#Pixel'>there</a> <a href='undocumented#Pixel'>is</a>
not sufficient memory to hold pixels; allocation does not take place
until the pixels are written to. The actual behavior depends on the platform
implementation of malloc(), if <a href='#SkBitmap_allocPixelsFlags_flags'>flags</a> <a href='#SkBitmap_allocPixelsFlags_flags'>is</a> <a href='#SkBitmap_allocPixelsFlags_flags'>zero</a>, <a href='#SkBitmap_allocPixelsFlags_flags'>and</a> <a href='#SkBitmap_allocPixelsFlags_flags'>calloc()</a>, <a href='#SkBitmap_allocPixelsFlags_flags'>if</a> <a href='#SkBitmap_allocPixelsFlags_flags'>flags</a> <a href='#SkBitmap_allocPixelsFlags_flags'>is</a>
<a href='#SkBitmap_kZeroPixels_AllocFlag'>kZeroPixels_AllocFlag</a>.

<a href='#SkBitmap_allocPixelsFlags_flags'>flags</a> <a href='#SkBitmap_allocPixelsFlags_flags'>set</a> <a href='#SkBitmap_allocPixelsFlags_flags'>to</a> <a href='#SkBitmap_kZeroPixels_AllocFlag'>kZeroPixels_AllocFlag</a> <a href='#SkBitmap_kZeroPixels_AllocFlag'>offers</a> <a href='#SkBitmap_kZeroPixels_AllocFlag'>equal</a> <a href='#SkBitmap_kZeroPixels_AllocFlag'>or</a> <a href='#SkBitmap_kZeroPixels_AllocFlag'>better</a> <a href='#SkBitmap_kZeroPixels_AllocFlag'>performance</a> <a href='#SkBitmap_kZeroPixels_AllocFlag'>than</a>
subsequently calling <a href='#SkBitmap_eraseColor'>eraseColor</a>() <a href='#SkBitmap_eraseColor'>with</a> <a href='SkColor_Reference#SK_ColorTRANSPARENT'>SK_ColorTRANSPARENT</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_allocPixelsFlags_info'><code><strong>info</strong></code></a></td>
    <td>contains width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_allocPixelsFlags_flags'><code><strong>flags</strong></code></a></td>
    <td><a href='#SkBitmap_kZeroPixels_AllocFlag'>kZeroPixels_AllocFlag</a>, <a href='#SkBitmap_kZeroPixels_AllocFlag'>or</a> <a href='#SkBitmap_kZeroPixels_AllocFlag'>zero</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="737e721c7d9e0f367d25521a1b823b9d"><div><a href='undocumented#Text'>Text</a> <a href='undocumented#Text'>is</a> <a href='undocumented#Text'>drawn</a> <a href='undocumented#Text'>on</a> <a href='undocumented#Text'>a</a> <a href='undocumented#Text'>transparent</a> <a href='undocumented#Text'>background</a>; <a href='undocumented#Text'>drawing</a> <a href='undocumented#Text'>the</a> <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='SkBitmap_Reference#Bitmap'>a</a> <a href='SkBitmap_Reference#Bitmap'>second</a> <a href='SkBitmap_Reference#Bitmap'>time</a>
<a href='SkBitmap_Reference#Bitmap'>lets</a> <a href='SkBitmap_Reference#Bitmap'>the</a> <a href='SkBitmap_Reference#Bitmap'>first</a> <a href='SkBitmap_Reference#Bitmap'>draw</a> <a href='SkBitmap_Reference#Bitmap'>show</a> <a href='SkBitmap_Reference#Bitmap'>through</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkBitmap_tryAllocPixelsFlags'>tryAllocPixelsFlags</a> <a href='#SkBitmap_allocPixels'>allocPixels</a> <a href='undocumented#SkMallocPixelRef'>SkMallocPixelRef</a>::<a href='#SkMallocPixelRef_MakeZeroed'>MakeZeroed</a>

<a name='SkBitmap_tryAllocPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a>(<a href='#SkBitmap_tryAllocPixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='#SkBitmap_rowBytes'>rowBytes</a>)
</pre>

Sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>to</a> <a href='#SkBitmap_tryAllocPixels_info'>info</a> <a href='#SkBitmap_tryAllocPixels_info'>following</a> <a href='#SkBitmap_tryAllocPixels_info'>the</a> <a href='#SkBitmap_tryAllocPixels_info'>rules</a> <a href='#SkBitmap_tryAllocPixels_info'>in</a> <a href='#SkBitmap_setInfo'>setInfo</a>() <a href='#SkBitmap_setInfo'>and</a> <a href='#SkBitmap_setInfo'>allocates</a> <a href='undocumented#Pixel'>pixel</a>
memory. <a href='#SkBitmap_tryAllocPixels_rowBytes'>rowBytes</a> <a href='#SkBitmap_tryAllocPixels_rowBytes'>must</a> <a href='#SkBitmap_tryAllocPixels_rowBytes'>equal</a> <a href='#SkBitmap_tryAllocPixels_rowBytes'>or</a> <a href='#SkBitmap_tryAllocPixels_rowBytes'>exceed</a> <a href='#SkBitmap_tryAllocPixels_info'>info</a>.<a href='#SkImageInfo_width'>width()</a> <a href='#SkImageInfo_width'>times</a> <a href='#SkBitmap_tryAllocPixels_info'>info</a>.<a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>(),
or equal zero. Pass in zero for <a href='#SkBitmap_tryAllocPixels_rowBytes'>rowBytes</a> <a href='#SkBitmap_tryAllocPixels_rowBytes'>to</a> <a href='#SkBitmap_tryAllocPixels_rowBytes'>compute</a> <a href='#SkBitmap_tryAllocPixels_rowBytes'>the</a> <a href='#SkBitmap_tryAllocPixels_rowBytes'>minimum</a> <a href='#SkBitmap_tryAllocPixels_rowBytes'>valid</a> <a href='#SkBitmap_tryAllocPixels_rowBytes'>value</a>.

Returns false and calls <a href='#SkBitmap_reset'>reset()</a> <a href='#SkBitmap_reset'>if</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>could</a> <a href='SkImageInfo_Reference#SkImageInfo'>not</a> <a href='SkImageInfo_Reference#SkImageInfo'>be</a> <a href='SkImageInfo_Reference#SkImageInfo'>set</a>, <a href='SkImageInfo_Reference#SkImageInfo'>or</a> <a href='SkImageInfo_Reference#SkImageInfo'>memory</a> <a href='SkImageInfo_Reference#SkImageInfo'>could</a>
not be allocated.

On most platforms, allocating <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a> <a href='undocumented#Pixel'>may</a> <a href='undocumented#Pixel'>succeed</a> <a href='undocumented#Pixel'>even</a> <a href='undocumented#Pixel'>though</a> <a href='undocumented#Pixel'>there</a> <a href='undocumented#Pixel'>is</a>
not sufficient memory to hold pixels; allocation does not take place
until the pixels are written to. The actual behavior depends on the platform
implementation of malloc().

### Parameters

<table>  <tr>    <td><a name='SkBitmap_tryAllocPixels_info'><code><strong>info</strong></code></a></td>
    <td>contains width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_tryAllocPixels_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>larger</a>; <a href='undocumented#Pixel'>may</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>zero</a></td>
  </tr>
</table>

### Return Value

true if  <a href='undocumented#Pixel_Storage'>pixel storage</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>allocated</a>

### Example

<div><fiddle-embed name="34479d5aa23ce9f5e334b0786c9edb22"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_tryAllocPixelsFlags'>tryAllocPixelsFlags</a> <a href='#SkBitmap_allocPixels'>allocPixels</a> <a href='undocumented#SkMallocPixelRef'>SkMallocPixelRef</a>::<a href='#SkMallocPixelRef_MakeAllocate'>MakeAllocate</a>

<a name='SkBitmap_allocPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_allocPixels'>allocPixels</a>(<a href='#SkBitmap_allocPixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='#SkBitmap_rowBytes'>rowBytes</a>)
</pre>

Sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>to</a> <a href='#SkBitmap_allocPixels_info'>info</a> <a href='#SkBitmap_allocPixels_info'>following</a> <a href='#SkBitmap_allocPixels_info'>the</a> <a href='#SkBitmap_allocPixels_info'>rules</a> <a href='#SkBitmap_allocPixels_info'>in</a> <a href='#SkBitmap_setInfo'>setInfo</a>() <a href='#SkBitmap_setInfo'>and</a> <a href='#SkBitmap_setInfo'>allocates</a> <a href='undocumented#Pixel'>pixel</a>
memory. <a href='#SkBitmap_allocPixels_rowBytes'>rowBytes</a> <a href='#SkBitmap_allocPixels_rowBytes'>must</a> <a href='#SkBitmap_allocPixels_rowBytes'>equal</a> <a href='#SkBitmap_allocPixels_rowBytes'>or</a> <a href='#SkBitmap_allocPixels_rowBytes'>exceed</a> <a href='#SkBitmap_allocPixels_info'>info</a>.<a href='#SkImageInfo_width'>width()</a> <a href='#SkImageInfo_width'>times</a> <a href='#SkBitmap_allocPixels_info'>info</a>.<a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>(),
or equal zero. Pass in zero for <a href='#SkBitmap_allocPixels_rowBytes'>rowBytes</a> <a href='#SkBitmap_allocPixels_rowBytes'>to</a> <a href='#SkBitmap_allocPixels_rowBytes'>compute</a> <a href='#SkBitmap_allocPixels_rowBytes'>the</a> <a href='#SkBitmap_allocPixels_rowBytes'>minimum</a> <a href='#SkBitmap_allocPixels_rowBytes'>valid</a> <a href='#SkBitmap_allocPixels_rowBytes'>value</a>.

Aborts execution if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>could</a> <a href='SkImageInfo_Reference#SkImageInfo'>not</a> <a href='SkImageInfo_Reference#SkImageInfo'>be</a> <a href='SkImageInfo_Reference#SkImageInfo'>set</a>, <a href='SkImageInfo_Reference#SkImageInfo'>or</a> <a href='SkImageInfo_Reference#SkImageInfo'>memory</a> <a href='SkImageInfo_Reference#SkImageInfo'>could</a>
not be allocated. Abort steps may be provided by
the user at compile time by defining SK_ABORT.

On most platforms, allocating <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a> <a href='undocumented#Pixel'>may</a> <a href='undocumented#Pixel'>succeed</a> <a href='undocumented#Pixel'>even</a> <a href='undocumented#Pixel'>though</a> <a href='undocumented#Pixel'>there</a> <a href='undocumented#Pixel'>is</a>
not sufficient memory to hold pixels; allocation does not take place
until the pixels are written to. The actual behavior depends on the platform
implementation of malloc().

### Parameters

<table>  <tr>    <td><a name='SkBitmap_allocPixels_info'><code><strong>info</strong></code></a></td>
    <td>contains width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_allocPixels_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>larger</a>; <a href='undocumented#Pixel'>may</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>zero</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="555c0f62f96602a9dcd459badcd005e0"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a> <a href='#SkBitmap_allocPixelsFlags'>allocPixelsFlags</a> <a href='undocumented#SkMallocPixelRef'>SkMallocPixelRef</a>::<a href='#SkMallocPixelRef_MakeAllocate'>MakeAllocate</a>

<a name='SkBitmap_tryAllocPixels_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a>(<a href='#SkBitmap_tryAllocPixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>)
</pre>

Sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>to</a> <a href='#SkBitmap_tryAllocPixels_2_info'>info</a> <a href='#SkBitmap_tryAllocPixels_2_info'>following</a> <a href='#SkBitmap_tryAllocPixels_2_info'>the</a> <a href='#SkBitmap_tryAllocPixels_2_info'>rules</a> <a href='#SkBitmap_tryAllocPixels_2_info'>in</a> <a href='#SkBitmap_setInfo'>setInfo</a>() <a href='#SkBitmap_setInfo'>and</a> <a href='#SkBitmap_setInfo'>allocates</a> <a href='undocumented#Pixel'>pixel</a>
memory.

Returns false and calls <a href='#SkBitmap_reset'>reset()</a> <a href='#SkBitmap_reset'>if</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>could</a> <a href='SkImageInfo_Reference#SkImageInfo'>not</a> <a href='SkImageInfo_Reference#SkImageInfo'>be</a> <a href='SkImageInfo_Reference#SkImageInfo'>set</a>, <a href='SkImageInfo_Reference#SkImageInfo'>or</a> <a href='SkImageInfo_Reference#SkImageInfo'>memory</a> <a href='SkImageInfo_Reference#SkImageInfo'>could</a>
not be allocated.

On most platforms, allocating <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a> <a href='undocumented#Pixel'>may</a> <a href='undocumented#Pixel'>succeed</a> <a href='undocumented#Pixel'>even</a> <a href='undocumented#Pixel'>though</a> <a href='undocumented#Pixel'>there</a> <a href='undocumented#Pixel'>is</a>
not sufficient memory to hold pixels; allocation does not take place
until the pixels are written to. The actual behavior depends on the platform
implementation of malloc().

### Parameters

<table>  <tr>    <td><a name='SkBitmap_tryAllocPixels_2_info'><code><strong>info</strong></code></a></td>
    <td>contains width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a></td>
  </tr>
</table>

### Return Value

true if  <a href='undocumented#Pixel_Storage'>pixel storage</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>allocated</a>

### Example

<div><fiddle-embed name="7ef3d043c4c5885649e591dd7dca92ff"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_tryAllocPixelsFlags'>tryAllocPixelsFlags</a> <a href='#SkBitmap_allocPixels'>allocPixels</a> <a href='undocumented#SkMallocPixelRef'>SkMallocPixelRef</a>::<a href='#SkMallocPixelRef_MakeAllocate'>MakeAllocate</a>

<a name='SkBitmap_allocPixels_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_allocPixels'>allocPixels</a>(<a href='#SkBitmap_allocPixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>)
</pre>

Sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>to</a> <a href='#SkBitmap_allocPixels_2_info'>info</a> <a href='#SkBitmap_allocPixels_2_info'>following</a> <a href='#SkBitmap_allocPixels_2_info'>the</a> <a href='#SkBitmap_allocPixels_2_info'>rules</a> <a href='#SkBitmap_allocPixels_2_info'>in</a> <a href='#SkBitmap_setInfo'>setInfo</a>() <a href='#SkBitmap_setInfo'>and</a> <a href='#SkBitmap_setInfo'>allocates</a> <a href='undocumented#Pixel'>pixel</a>
memory.

Aborts execution if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>could</a> <a href='SkImageInfo_Reference#SkImageInfo'>not</a> <a href='SkImageInfo_Reference#SkImageInfo'>be</a> <a href='SkImageInfo_Reference#SkImageInfo'>set</a>, <a href='SkImageInfo_Reference#SkImageInfo'>or</a> <a href='SkImageInfo_Reference#SkImageInfo'>memory</a> <a href='SkImageInfo_Reference#SkImageInfo'>could</a>
not be allocated. Abort steps may be provided by
the user at compile time by defining SK_ABORT.

On most platforms, allocating <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a> <a href='undocumented#Pixel'>may</a> <a href='undocumented#Pixel'>succeed</a> <a href='undocumented#Pixel'>even</a> <a href='undocumented#Pixel'>though</a> <a href='undocumented#Pixel'>there</a> <a href='undocumented#Pixel'>is</a>
not sufficient memory to hold pixels; allocation does not take place
until the pixels are written to. The actual behavior depends on the platform
implementation of malloc().

### Parameters

<table>  <tr>    <td><a name='SkBitmap_allocPixels_2_info'><code><strong>info</strong></code></a></td>
    <td>contains width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="91f474a11a2112cd5c88c40a9015048d"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a> <a href='#SkBitmap_allocPixelsFlags'>allocPixelsFlags</a> <a href='undocumented#SkMallocPixelRef'>SkMallocPixelRef</a>::<a href='#SkMallocPixelRef_MakeAllocate'>MakeAllocate</a>

<a name='SkBitmap_tryAllocN32Pixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_tryAllocN32Pixels'>tryAllocN32Pixels</a>(<a href='#SkBitmap_tryAllocN32Pixels'>int</a> <a href='#SkBitmap_tryAllocN32Pixels'>width</a>, <a href='#SkBitmap_tryAllocN32Pixels'>int</a> <a href='#SkBitmap_tryAllocN32Pixels'>height</a>, <a href='#SkBitmap_tryAllocN32Pixels'>bool</a> <a href='#SkBitmap_isOpaque'>isOpaque</a> = <a href='#SkBitmap_isOpaque'>false</a>)
</pre>

Sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>to</a> <a href='#SkBitmap_tryAllocN32Pixels_width'>width</a>, <a href='#SkBitmap_tryAllocN32Pixels_height'>height</a>, <a href='#SkBitmap_tryAllocN32Pixels_height'>and</a> <a href='#SkBitmap_tryAllocN32Pixels_height'>native</a>  <a href='SkImageInfo_Reference#Color_Type'>color type</a>; <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>allocates</a>
<a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a>. <a href='undocumented#Pixel'>If</a> <a href='#SkBitmap_tryAllocN32Pixels_isOpaque'>isOpaque</a> <a href='#SkBitmap_tryAllocN32Pixels_isOpaque'>is</a> <a href='#SkBitmap_tryAllocN32Pixels_isOpaque'>true</a>, <a href='#SkBitmap_tryAllocN32Pixels_isOpaque'>sets</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>to</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>;
otherwise, sets to <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>.

Calls <a href='#SkBitmap_reset'>reset()</a> <a href='#SkBitmap_reset'>and</a> <a href='#SkBitmap_reset'>returns</a> <a href='#SkBitmap_reset'>false</a> <a href='#SkBitmap_reset'>if</a> <a href='#SkBitmap_tryAllocN32Pixels_width'>width</a> <a href='#SkBitmap_tryAllocN32Pixels_width'>exceeds</a> 29 <a href='#SkBitmap_tryAllocN32Pixels_width'>bits</a> <a href='#SkBitmap_tryAllocN32Pixels_width'>or</a> <a href='#SkBitmap_tryAllocN32Pixels_width'>is</a> <a href='#SkBitmap_tryAllocN32Pixels_width'>negative</a>,
or <a href='#SkBitmap_tryAllocN32Pixels_height'>height</a> <a href='#SkBitmap_tryAllocN32Pixels_height'>is</a> <a href='#SkBitmap_tryAllocN32Pixels_height'>negative</a>.

Returns false if allocation fails.

Use to create <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>that</a> <a href='SkBitmap_Reference#SkBitmap'>matches</a> <a href='SkColor_Reference#SkPMColor'>SkPMColor</a>, <a href='SkColor_Reference#SkPMColor'>the</a> <a href='SkColor_Reference#SkPMColor'>native</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>arrangement</a> <a href='undocumented#Pixel'>on</a>
the platform. <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>drawn</a> <a href='SkBitmap_Reference#SkBitmap'>to</a> <a href='SkBitmap_Reference#SkBitmap'>output</a> <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>skips</a> <a href='undocumented#Device'>converting</a> <a href='undocumented#Device'>its</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>format</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_tryAllocN32Pixels_width'><code><strong>width</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>column</a> <a href='undocumented#Pixel'>count</a>; <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>zero</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>greater</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_tryAllocN32Pixels_height'><code><strong>height</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>count</a>; <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>zero</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>greater</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_tryAllocN32Pixels_isOpaque'><code><strong>isOpaque</strong></code></a></td>
    <td>true if pixels do not have transparency</td>
  </tr>
</table>

### Return Value

true if  <a href='undocumented#Pixel_Storage'>pixel storage</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>allocated</a>

### Example

<div><fiddle-embed name="a2b1e0910f37066f15ae56368775a6d8"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a> <a href='#SkBitmap_allocN32Pixels'>allocN32Pixels</a> <a href='undocumented#SkMallocPixelRef'>SkMallocPixelRef</a>::<a href='#SkMallocPixelRef_MakeAllocate'>MakeAllocate</a>

<a name='SkBitmap_allocN32Pixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_allocN32Pixels'>allocN32Pixels</a>(<a href='#SkBitmap_allocN32Pixels'>int</a> <a href='#SkBitmap_allocN32Pixels'>width</a>, <a href='#SkBitmap_allocN32Pixels'>int</a> <a href='#SkBitmap_allocN32Pixels'>height</a>, <a href='#SkBitmap_allocN32Pixels'>bool</a> <a href='#SkBitmap_isOpaque'>isOpaque</a> = <a href='#SkBitmap_isOpaque'>false</a>)
</pre>

Sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>to</a> <a href='#SkBitmap_allocN32Pixels_width'>width</a>, <a href='#SkBitmap_allocN32Pixels_height'>height</a>, <a href='#SkBitmap_allocN32Pixels_height'>and</a> <a href='#SkBitmap_allocN32Pixels_height'>the</a> <a href='#SkBitmap_allocN32Pixels_height'>native</a>  <a href='SkImageInfo_Reference#Color_Type'>color type</a>; <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>allocates</a>
<a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a>. <a href='undocumented#Pixel'>If</a> <a href='#SkBitmap_allocN32Pixels_isOpaque'>isOpaque</a> <a href='#SkBitmap_allocN32Pixels_isOpaque'>is</a> <a href='#SkBitmap_allocN32Pixels_isOpaque'>true</a>, <a href='#SkBitmap_allocN32Pixels_isOpaque'>sets</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>to</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>;
otherwise, sets to <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>.

Aborts if <a href='#SkBitmap_allocN32Pixels_width'>width</a> <a href='#SkBitmap_allocN32Pixels_width'>exceeds</a> 29 <a href='#SkBitmap_allocN32Pixels_width'>bits</a> <a href='#SkBitmap_allocN32Pixels_width'>or</a> <a href='#SkBitmap_allocN32Pixels_width'>is</a> <a href='#SkBitmap_allocN32Pixels_width'>negative</a>, <a href='#SkBitmap_allocN32Pixels_width'>or</a> <a href='#SkBitmap_allocN32Pixels_height'>height</a> <a href='#SkBitmap_allocN32Pixels_height'>is</a> <a href='#SkBitmap_allocN32Pixels_height'>negative</a>, <a href='#SkBitmap_allocN32Pixels_height'>or</a>
allocation fails. Abort steps may be provided by the user at compile time by
defining SK_ABORT.

Use to create <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>that</a> <a href='SkBitmap_Reference#SkBitmap'>matches</a> <a href='SkColor_Reference#SkPMColor'>SkPMColor</a>, <a href='SkColor_Reference#SkPMColor'>the</a> <a href='SkColor_Reference#SkPMColor'>native</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>arrangement</a> <a href='undocumented#Pixel'>on</a>
the platform. <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>drawn</a> <a href='SkBitmap_Reference#SkBitmap'>to</a> <a href='SkBitmap_Reference#SkBitmap'>output</a> <a href='undocumented#Device'>device</a> <a href='undocumented#Device'>skips</a> <a href='undocumented#Device'>converting</a> <a href='undocumented#Device'>its</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>format</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_allocN32Pixels_width'><code><strong>width</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>column</a> <a href='undocumented#Pixel'>count</a>; <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>zero</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>greater</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_allocN32Pixels_height'><code><strong>height</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>count</a>; <a href='undocumented#Pixel'>must</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>zero</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>greater</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_allocN32Pixels_isOpaque'><code><strong>isOpaque</strong></code></a></td>
    <td>true if pixels do not have transparency</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c717491f9251604724c9cbde7088ec20"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_allocPixels'>allocPixels</a> <a href='#SkBitmap_tryAllocN32Pixels'>tryAllocN32Pixels</a> <a href='undocumented#SkMallocPixelRef'>SkMallocPixelRef</a>::<a href='#SkMallocPixelRef_MakeAllocate'>MakeAllocate</a>

<a name='SkBitmap_installPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_installPixels'>installPixels</a>(<a href='#SkBitmap_installPixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>pixels</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='#SkBitmap_rowBytes'>rowBytes</a>, <a href='#SkBitmap_rowBytes'>void</a> (*<a href='#SkBitmap_rowBytes'>releaseProc</a>)
                   (<a href='#SkBitmap_rowBytes'>void</a>* <a href='#SkBitmap_rowBytes'>addr</a>, <a href='#SkBitmap_rowBytes'>void</a>* <a href='#SkBitmap_rowBytes'>context</a>) , <a href='#SkBitmap_rowBytes'>void</a>* <a href='#SkBitmap_rowBytes'>context</a>)
</pre>

Sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>to</a> <a href='#SkBitmap_installPixels_info'>info</a> <a href='#SkBitmap_installPixels_info'>following</a> <a href='#SkBitmap_installPixels_info'>the</a> <a href='#SkBitmap_installPixels_info'>rules</a> <a href='#SkBitmap_installPixels_info'>in</a> <a href='#SkBitmap_setInfo'>setInfo</a>(), <a href='#SkBitmap_setInfo'>and</a> <a href='#SkBitmap_setInfo'>creates</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>
containing <a href='#SkBitmap_installPixels_pixels'>pixels</a> <a href='#SkBitmap_installPixels_pixels'>and</a> <a href='#SkBitmap_installPixels_rowBytes'>rowBytes</a>. <a href='#SkBitmap_installPixels_releaseProc'>releaseProc</a>, <a href='#SkBitmap_installPixels_releaseProc'>if</a> <a href='#SkBitmap_installPixels_releaseProc'>not</a> <a href='#SkBitmap_installPixels_releaseProc'>nullptr</a>, <a href='#SkBitmap_installPixels_releaseProc'>is</a> <a href='#SkBitmap_installPixels_releaseProc'>called</a>
immediately on failure or when <a href='#SkBitmap_installPixels_pixels'>pixels</a> <a href='#SkBitmap_installPixels_pixels'>are</a> <a href='#SkBitmap_installPixels_pixels'>no</a> <a href='#SkBitmap_installPixels_pixels'>longer</a> <a href='#SkBitmap_installPixels_pixels'>referenced</a>. <a href='#SkBitmap_installPixels_context'>context</a> <a href='#SkBitmap_installPixels_context'>may</a> <a href='#SkBitmap_installPixels_context'>be</a>
nullptr.

If <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>could</a> <a href='SkImageInfo_Reference#SkImageInfo'>not</a> <a href='SkImageInfo_Reference#SkImageInfo'>be</a> <a href='SkImageInfo_Reference#SkImageInfo'>set</a>, <a href='SkImageInfo_Reference#SkImageInfo'>or</a> <a href='#SkBitmap_installPixels_rowBytes'>rowBytes</a> <a href='#SkBitmap_installPixels_rowBytes'>is</a> <a href='#SkBitmap_installPixels_rowBytes'>less</a> <a href='#SkBitmap_installPixels_rowBytes'>than</a> <a href='#SkBitmap_installPixels_info'>info</a>.<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>():
calls <a href='#SkBitmap_installPixels_releaseProc'>releaseProc</a> <a href='#SkBitmap_installPixels_releaseProc'>if</a> <a href='#SkBitmap_installPixels_releaseProc'>present</a>, <a href='#SkBitmap_installPixels_releaseProc'>calls</a> <a href='#SkBitmap_reset'>reset()</a>, <a href='#SkBitmap_reset'>and</a> <a href='#SkBitmap_reset'>returns</a> <a href='#SkBitmap_reset'>false</a>.

Otherwise, if <a href='#SkBitmap_installPixels_pixels'>pixels</a> <a href='#SkBitmap_installPixels_pixels'>equals</a> <a href='#SkBitmap_installPixels_pixels'>nullptr</a>: <a href='#SkBitmap_installPixels_pixels'>sets</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>calls</a> <a href='#SkBitmap_installPixels_releaseProc'>releaseProc</a> <a href='#SkBitmap_installPixels_releaseProc'>if</a>
present, returns true.

If <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>is</a> <a href='SkImageInfo_Reference#SkImageInfo'>set</a>, <a href='#SkBitmap_installPixels_pixels'>pixels</a> <a href='#SkBitmap_installPixels_pixels'>is</a> <a href='#SkBitmap_installPixels_pixels'>not</a> <a href='#SkBitmap_installPixels_pixels'>nullptr</a>, <a href='#SkBitmap_installPixels_pixels'>and</a> <a href='#SkBitmap_installPixels_releaseProc'>releaseProc</a> <a href='#SkBitmap_installPixels_releaseProc'>is</a> <a href='#SkBitmap_installPixels_releaseProc'>not</a> <a href='#SkBitmap_installPixels_releaseProc'>nullptr</a>:
when <a href='#SkBitmap_installPixels_pixels'>pixels</a> <a href='#SkBitmap_installPixels_pixels'>are</a> <a href='#SkBitmap_installPixels_pixels'>no</a> <a href='#SkBitmap_installPixels_pixels'>longer</a> <a href='#SkBitmap_installPixels_pixels'>referenced</a>, <a href='#SkBitmap_installPixels_pixels'>calls</a> <a href='#SkBitmap_installPixels_releaseProc'>releaseProc</a> <a href='#SkBitmap_installPixels_releaseProc'>with</a> <a href='#SkBitmap_installPixels_pixels'>pixels</a> <a href='#SkBitmap_installPixels_pixels'>and</a> <a href='#SkBitmap_installPixels_context'>context</a>
as parameters.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_installPixels_info'><code><strong>info</strong></code></a></td>
    <td>contains width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_installPixels_pixels'><code><strong>pixels</strong></code></a></td>
    <td>address or  <a href='undocumented#Pixel_Storage'>pixel storage</a>; <a href='undocumented#Pixel'>may</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_installPixels_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>larger</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_installPixels_releaseProc'><code><strong>releaseProc</strong></code></a></td>
    <td>function called when <a href='#SkBitmap_installPixels_pixels'>pixels</a> <a href='#SkBitmap_installPixels_pixels'>can</a> <a href='#SkBitmap_installPixels_pixels'>be</a> <a href='#SkBitmap_installPixels_pixels'>deleted</a>; <a href='#SkBitmap_installPixels_pixels'>may</a> <a href='#SkBitmap_installPixels_pixels'>be</a> <a href='#SkBitmap_installPixels_pixels'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_installPixels_context'><code><strong>context</strong></code></a></td>
    <td>caller state passed to <a href='#SkBitmap_installPixels_releaseProc'>releaseProc</a>; <a href='#SkBitmap_installPixels_releaseProc'>may</a> <a href='#SkBitmap_installPixels_releaseProc'>be</a> <a href='#SkBitmap_installPixels_releaseProc'>nullptr</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>is</a> <a href='SkImageInfo_Reference#SkImageInfo'>set</a> <a href='SkImageInfo_Reference#SkImageInfo'>to</a> <a href='#SkBitmap_installPixels_info'>info</a>

### Example

<div><fiddle-embed name="8c4f7bf73fffa1a812ee8e88e44e639c"><div><a href='#SkBitmap_installPixels_releaseProc'>releaseProc</a> <a href='#SkBitmap_installPixels_releaseProc'>is</a> <a href='#SkBitmap_installPixels_releaseProc'>called</a> <a href='#SkBitmap_installPixels_releaseProc'>immediately</a> <a href='#SkBitmap_installPixels_releaseProc'>because</a> <a href='#SkBitmap_installPixels_rowBytes'>rowBytes</a> <a href='#SkBitmap_installPixels_rowBytes'>is</a> <a href='#SkBitmap_installPixels_rowBytes'>too</a> <a href='#SkBitmap_installPixels_rowBytes'>small</a> <a href='#SkBitmap_installPixels_rowBytes'>for</a> <a href='#Pixel_Ref'>Pixel_Ref</a>.
</div>

#### Example Output

~~~~
before installPixels
releaseProc called
install not successful
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_allocPixels'>allocPixels</a>

<a name='SkBitmap_installPixels_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_installPixels'>installPixels</a>(<a href='#SkBitmap_installPixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>pixels</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='#SkBitmap_rowBytes'>rowBytes</a>)
</pre>

Sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>to</a> <a href='#SkBitmap_installPixels_2_info'>info</a> <a href='#SkBitmap_installPixels_2_info'>following</a> <a href='#SkBitmap_installPixels_2_info'>the</a> <a href='#SkBitmap_installPixels_2_info'>rules</a> <a href='#SkBitmap_installPixels_2_info'>in</a> <a href='#SkBitmap_setInfo'>setInfo</a>(), <a href='#SkBitmap_setInfo'>and</a> <a href='#SkBitmap_setInfo'>creates</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>
containing <a href='#SkBitmap_installPixels_2_pixels'>pixels</a> <a href='#SkBitmap_installPixels_2_pixels'>and</a> <a href='#SkBitmap_installPixels_2_rowBytes'>rowBytes</a>.

If <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>could</a> <a href='SkImageInfo_Reference#SkImageInfo'>not</a> <a href='SkImageInfo_Reference#SkImageInfo'>be</a> <a href='SkImageInfo_Reference#SkImageInfo'>set</a>, <a href='SkImageInfo_Reference#SkImageInfo'>or</a> <a href='#SkBitmap_installPixels_2_rowBytes'>rowBytes</a> <a href='#SkBitmap_installPixels_2_rowBytes'>is</a> <a href='#SkBitmap_installPixels_2_rowBytes'>less</a> <a href='#SkBitmap_installPixels_2_rowBytes'>than</a> <a href='#SkBitmap_installPixels_2_info'>info</a>.<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>():
calls <a href='#SkBitmap_reset'>reset()</a>, <a href='#SkBitmap_reset'>and</a> <a href='#SkBitmap_reset'>returns</a> <a href='#SkBitmap_reset'>false</a>.

Otherwise, if <a href='#SkBitmap_installPixels_2_pixels'>pixels</a> <a href='#SkBitmap_installPixels_2_pixels'>equals</a> <a href='#SkBitmap_installPixels_2_pixels'>nullptr</a>: <a href='#SkBitmap_installPixels_2_pixels'>sets</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>returns</a> <a href='SkImageInfo_Reference#SkImageInfo'>true</a>.

Caller must ensure that <a href='#SkBitmap_installPixels_2_pixels'>pixels</a> <a href='#SkBitmap_installPixels_2_pixels'>are</a> <a href='#SkBitmap_installPixels_2_pixels'>valid</a> <a href='#SkBitmap_installPixels_2_pixels'>for</a> <a href='#SkBitmap_installPixels_2_pixels'>the</a> <a href='#SkBitmap_installPixels_2_pixels'>lifetime</a> <a href='#SkBitmap_installPixels_2_pixels'>of</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>and</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_installPixels_2_info'><code><strong>info</strong></code></a></td>
    <td>contains width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_installPixels_2_pixels'><code><strong>pixels</strong></code></a></td>
    <td>address or  <a href='undocumented#Pixel_Storage'>pixel storage</a>; <a href='undocumented#Pixel'>may</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_installPixels_2_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>larger</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>is</a> <a href='SkImageInfo_Reference#SkImageInfo'>set</a> <a href='SkImageInfo_Reference#SkImageInfo'>to</a> <a href='#SkBitmap_installPixels_2_info'>info</a>

### Example

<div><fiddle-embed name="a7e04447b2081010c50d7920e80a6bb2"><div>GPU does not support <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>, <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>does</a> <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>not</a> <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>assert</a> <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>that</a> <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>it</a> <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>does</a> <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>not</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkBitmap_allocPixels'>allocPixels</a>

<a name='SkBitmap_installPixels_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_installPixels'>installPixels</a>(<a href='#SkBitmap_installPixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#Pixmap'>pixmap</a>)
</pre>

Sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>to</a> <a href='#SkBitmap_installPixels_3_pixmap'>pixmap</a>.<a href='#SkPixmap_info'>info()</a> <a href='#SkPixmap_info'>following</a> <a href='#SkPixmap_info'>the</a> <a href='#SkPixmap_info'>rules</a> <a href='#SkPixmap_info'>in</a> <a href='#SkBitmap_setInfo'>setInfo</a>(), <a href='#SkBitmap_setInfo'>and</a> <a href='#SkBitmap_setInfo'>creates</a>
<a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>containing</a> <a href='#SkBitmap_installPixels_3_pixmap'>pixmap</a>.<a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_addr'>and</a> <a href='#SkBitmap_installPixels_3_pixmap'>pixmap</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>().

If <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>could</a> <a href='SkImageInfo_Reference#SkImageInfo'>not</a> <a href='SkImageInfo_Reference#SkImageInfo'>be</a> <a href='SkImageInfo_Reference#SkImageInfo'>set</a>, <a href='SkImageInfo_Reference#SkImageInfo'>or</a> <a href='#SkBitmap_installPixels_3_pixmap'>pixmap</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() <a href='#SkPixmap_rowBytes'>is</a> <a href='#SkPixmap_rowBytes'>less</a> <a href='#SkPixmap_rowBytes'>than</a>
<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>(): <a href='#SkImageInfo_minRowBytes'>calls</a> <a href='#SkBitmap_reset'>reset()</a>, <a href='#SkBitmap_reset'>and</a> <a href='#SkBitmap_reset'>returns</a> <a href='#SkBitmap_reset'>false</a>.

Otherwise, if <a href='#SkBitmap_installPixels_3_pixmap'>pixmap</a>.<a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_addr'>equals</a> <a href='#SkPixmap_addr'>nullptr</a>: <a href='#SkPixmap_addr'>sets</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>returns</a> <a href='SkImageInfo_Reference#SkImageInfo'>true</a>.

Caller must ensure that <a href='#SkBitmap_installPixels_3_pixmap'>pixmap</a> <a href='#SkBitmap_installPixels_3_pixmap'>is</a> <a href='#SkBitmap_installPixels_3_pixmap'>valid</a> <a href='#SkBitmap_installPixels_3_pixmap'>for</a> <a href='#SkBitmap_installPixels_3_pixmap'>the</a> <a href='#SkBitmap_installPixels_3_pixmap'>lifetime</a> <a href='#SkBitmap_installPixels_3_pixmap'>of</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>and</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_installPixels_3_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a>, <a href='undocumented#Pixel'>and</a> <a href='#SkBitmap_rowBytes'>rowBytes</a>()</td>
  </tr>
</table>

### Return Value

true if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>was</a> <a href='SkImageInfo_Reference#SkImageInfo'>set</a> <a href='SkImageInfo_Reference#SkImageInfo'>to</a> <a href='#SkBitmap_installPixels_3_pixmap'>pixmap</a>.<a href='#SkPixmap_info'>info()</a>

### Example

<div><fiddle-embed name="6e2a8c9358b34aebd2ec586815fe9d3a"><div>Draw a five by five <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='SkBitmap_Reference#Bitmap'>and</a> <a href='SkBitmap_Reference#Bitmap'>draw</a> <a href='SkBitmap_Reference#Bitmap'>it</a> <a href='SkBitmap_Reference#Bitmap'>again</a> <a href='SkBitmap_Reference#Bitmap'>with</a> <a href='SkBitmap_Reference#Bitmap'>a</a> <a href='SkBitmap_Reference#Bitmap'>center</a> <a href='SkBitmap_Reference#Bitmap'>white</a> <a href='undocumented#Pixel'>pixel</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkBitmap_allocPixels'>allocPixels</a>

<a name='SkBitmap_installMaskPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_installMaskPixels'>installMaskPixels</a>(<a href='#SkBitmap_installMaskPixels'>const</a> <a href='undocumented#SkMask'>SkMask</a>& <a href='undocumented#SkMask'>mask</a>)
</pre>

To be deprecated soon.

<a name='Pixels'></a>

<a name='SkBitmap_setPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_setPixels'>setPixels</a>(<a href='#SkBitmap_setPixels'>void</a>* <a href='#SkBitmap_setPixels'>pixels</a>)
</pre>

Replaces <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>with</a> <a href='#SkBitmap_setPixels_pixels'>pixels</a>, <a href='#SkBitmap_setPixels_pixels'>preserving</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>and</a> <a href='#SkBitmap_rowBytes'>rowBytes</a>().
Sets <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>origin</a> <a href='undocumented#SkPixelRef'>to</a> (0, 0).

If <a href='#SkBitmap_setPixels_pixels'>pixels</a> <a href='#SkBitmap_setPixels_pixels'>is</a> <a href='#SkBitmap_setPixels_pixels'>nullptr</a>, <a href='#SkBitmap_setPixels_pixels'>or</a> <a href='#SkBitmap_setPixels_pixels'>if</a> <a href='#SkBitmap_info'>info()</a>.<a href='#SkImageInfo_colorType'>colorType</a>() <a href='#SkImageInfo_colorType'>equals</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>;
release reference to <a href='undocumented#SkPixelRef'>SkPixelRef</a>, <a href='undocumented#SkPixelRef'>and</a> <a href='undocumented#SkPixelRef'>set</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>to</a> <a href='undocumented#SkPixelRef'>nullptr</a>.

Caller is responsible for handling ownership <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a> <a href='undocumented#Pixel'>for</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>lifetime</a>
of <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>and</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_setPixels_pixels'><code><strong>pixels</strong></code></a></td>
    <td>address of  <a href='undocumented#Pixel_Storage'>pixel storage</a>, <a href='undocumented#Pixel'>managed</a> <a href='undocumented#Pixel'>by</a> <a href='undocumented#Pixel'>caller</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f0db16e06c9a1436917c8179f8c1718f"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_installPixels'>installPixels</a> <a href='#SkBitmap_allocPixels'>allocPixels</a>

<a name='SkBitmap_tryAllocPixels_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a>()
</pre>

Allocates <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a> <a href='undocumented#Pixel'>with</a> <a href='#SkBitmap_HeapAllocator'>HeapAllocator</a>, <a href='#SkBitmap_HeapAllocator'>and</a> <a href='#SkBitmap_HeapAllocator'>replaces</a> <a href='#SkBitmap_HeapAllocator'>existing</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>.
The allocation <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>is</a> <a href='undocumented#Size'>determined</a> <a href='undocumented#Size'>by</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>width</a>, <a href='SkImageInfo_Reference#SkImageInfo'>height</a>, <a href='SkImageInfo_Reference#SkImageInfo'>and</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.

Returns false if <a href='#SkBitmap_info'>info()</a>.<a href='#SkImageInfo_colorType'>colorType</a>() <a href='#SkImageInfo_colorType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kUnknown_SkColorType'>or</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>allocation</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>fails</a>.

### Return Value

true if the allocation succeeds

### Example

<div><fiddle-embed name="720e4c053fae9e929ab6518b47e49370"><div><a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>hosts</a> <a href='SkBitmap_Reference#Bitmap'>and</a> <a href='SkBitmap_Reference#Bitmap'>draws</a> <a href='SkBitmap_Reference#Bitmap'>gray</a> <a href='SkBitmap_Reference#Bitmap'>values</a> <a href='SkBitmap_Reference#Bitmap'>in</a> <a href='SkBitmap_Reference#Bitmap'>set1</a>. <a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a> <a href='#SkBitmap_tryAllocPixels'>replaces</a> <a href='#Pixel_Ref'>Pixel_Ref</a>
<a href='#Pixel_Ref'>and</a> <a href='#Pixel_Ref'>erases</a> <a href='#Pixel_Ref'>it</a> <a href='#Pixel_Ref'>to</a> <a href='#Pixel_Ref'>black</a>, <a href='#Pixel_Ref'>but</a> <a href='#Pixel_Ref'>does</a> <a href='#Pixel_Ref'>not</a> <a href='#Pixel_Ref'>alter</a> <a href='#Pixel_Ref'>set1</a>. <a href='#SkBitmap_setPixels'>setPixels</a> <a href='#SkBitmap_setPixels'>replaces</a> <a href='#SkBitmap_setPixels'>black</a>
<a href='#Pixel_Ref'>Pixel_Ref</a> <a href='#Pixel_Ref'>with</a> <a href='#Pixel_Ref'>set1</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkBitmap_allocPixels'>allocPixels</a> <a href='#SkBitmap_installPixels'>installPixels</a> <a href='#SkBitmap_setPixels'>setPixels</a>

<a name='SkBitmap_allocPixels_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_allocPixels'>allocPixels</a>()
</pre>

Allocates <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a> <a href='undocumented#Pixel'>with</a> <a href='#SkBitmap_HeapAllocator'>HeapAllocator</a>, <a href='#SkBitmap_HeapAllocator'>and</a> <a href='#SkBitmap_HeapAllocator'>replaces</a> <a href='#SkBitmap_HeapAllocator'>existing</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>.
The allocation <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>is</a> <a href='undocumented#Size'>determined</a> <a href='undocumented#Size'>by</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>width</a>, <a href='SkImageInfo_Reference#SkImageInfo'>height</a>, <a href='SkImageInfo_Reference#SkImageInfo'>and</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.

Aborts if <a href='#SkBitmap_info'>info()</a>.<a href='#SkImageInfo_colorType'>colorType</a>() <a href='#SkImageInfo_colorType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kUnknown_SkColorType'>or</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>allocation</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>fails</a>.
Abort steps may be provided by the user at compile
time by defining SK_ABORT.

### Example

<div><fiddle-embed name="1219b38c788bf270fb20f8cd2d78cff8"><div><a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>hosts</a> <a href='SkBitmap_Reference#Bitmap'>and</a> <a href='SkBitmap_Reference#Bitmap'>draws</a> <a href='SkBitmap_Reference#Bitmap'>gray</a> <a href='SkBitmap_Reference#Bitmap'>values</a> <a href='SkBitmap_Reference#Bitmap'>in</a> <a href='SkBitmap_Reference#Bitmap'>set1</a>. <a href='#SkBitmap_allocPixels'>allocPixels</a> <a href='#SkBitmap_allocPixels'>replaces</a> <a href='#Pixel_Ref'>Pixel_Ref</a>
<a href='#Pixel_Ref'>and</a> <a href='#Pixel_Ref'>erases</a> <a href='#Pixel_Ref'>it</a> <a href='#Pixel_Ref'>to</a> <a href='#Pixel_Ref'>black</a>, <a href='#Pixel_Ref'>but</a> <a href='#Pixel_Ref'>does</a> <a href='#Pixel_Ref'>not</a> <a href='#Pixel_Ref'>alter</a> <a href='#Pixel_Ref'>set1</a>. <a href='#SkBitmap_setPixels'>setPixels</a> <a href='#SkBitmap_setPixels'>replaces</a> <a href='#SkBitmap_setPixels'>black</a>
<a href='#Pixel_Ref'>Pixel_Ref</a> <a href='#Pixel_Ref'>with</a> <a href='#Pixel_Ref'>set2</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a> <a href='#SkBitmap_installPixels'>installPixels</a> <a href='#SkBitmap_setPixels'>setPixels</a>

<a name='SkBitmap_tryAllocPixels_4'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a>(<a href='#SkBitmap_Allocator'>Allocator</a>* <a href='#SkBitmap_Allocator'>allocator</a>)
</pre>

Allocates <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a> <a href='undocumented#Pixel'>with</a> <a href='#SkBitmap_tryAllocPixels_4_allocator'>allocator</a>, <a href='#SkBitmap_tryAllocPixels_4_allocator'>and</a> <a href='#SkBitmap_tryAllocPixels_4_allocator'>replaces</a> <a href='#SkBitmap_tryAllocPixels_4_allocator'>existing</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>.
The allocation <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>is</a> <a href='undocumented#Size'>determined</a> <a href='undocumented#Size'>by</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>width</a>, <a href='SkImageInfo_Reference#SkImageInfo'>height</a>, <a href='SkImageInfo_Reference#SkImageInfo'>and</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.
If <a href='#SkBitmap_tryAllocPixels_4_allocator'>allocator</a> <a href='#SkBitmap_tryAllocPixels_4_allocator'>is</a> <a href='#SkBitmap_tryAllocPixels_4_allocator'>nullptr</a>, <a href='#SkBitmap_tryAllocPixels_4_allocator'>use</a> <a href='#SkBitmap_HeapAllocator'>HeapAllocator</a> <a href='#SkBitmap_HeapAllocator'>instead</a>.

Returns false if <a href='#SkBitmap_Allocator'>Allocator</a>::<a href='#SkBitmap_Allocator_allocPixelRef'>allocPixelRef</a> <a href='#SkBitmap_Allocator_allocPixelRef'>return</a> <a href='#SkBitmap_Allocator_allocPixelRef'>false</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_tryAllocPixels_4_allocator'><code><strong>allocator</strong></code></a></td>
    <td>instance of <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_Allocator'>Allocator</a> <a href='#SkBitmap_Allocator'>instantiation</a></td>
  </tr>
</table>

### Return Value

true if custom <a href='#SkBitmap_tryAllocPixels_4_allocator'>allocator</a> <a href='#SkBitmap_tryAllocPixels_4_allocator'>reports</a> <a href='#SkBitmap_tryAllocPixels_4_allocator'>success</a>

### Example

<div><fiddle-embed name="eb6f861ca1839146d26e40d56c2a001c"><div><a href='#SkBitmap_HeapAllocator'>HeapAllocator</a> <a href='#SkBitmap_HeapAllocator'>limits</a> <a href='#SkBitmap_HeapAllocator'>the</a> <a href='#SkBitmap_HeapAllocator'>maximum</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>to</a> <a href='SkBitmap_Reference#Bitmap'>two</a> <a href='SkBitmap_Reference#Bitmap'>gigabytes</a>. <a href='SkBitmap_Reference#Bitmap'>Using</a>
<a href='SkBitmap_Reference#Bitmap'>a</a> <a href='SkBitmap_Reference#Bitmap'>custom</a> <a href='#SkBitmap_tryAllocPixels_4_allocator'>allocator</a>, <a href='#SkBitmap_tryAllocPixels_4_allocator'>this</a> <a href='#SkBitmap_tryAllocPixels_4_allocator'>limitation</a> <a href='#SkBitmap_tryAllocPixels_4_allocator'>may</a> <a href='#SkBitmap_tryAllocPixels_4_allocator'>be</a> <a href='#SkBitmap_tryAllocPixels_4_allocator'>relaxed</a>. <a href='#SkBitmap_tryAllocPixels_4_allocator'>This</a> <a href='#SkBitmap_tryAllocPixels_4_allocator'>example</a> <a href='#SkBitmap_tryAllocPixels_4_allocator'>can</a> <a href='#SkBitmap_tryAllocPixels_4_allocator'>be</a>
<a href='#SkBitmap_tryAllocPixels_4_allocator'>modified</a> <a href='#SkBitmap_tryAllocPixels_4_allocator'>to</a> <a href='#SkBitmap_tryAllocPixels_4_allocator'>allocate</a> <a href='#SkBitmap_tryAllocPixels_4_allocator'>an</a> <a href='#SkBitmap_tryAllocPixels_4_allocator'>eight</a> <a href='#SkBitmap_tryAllocPixels_4_allocator'>gigabyte</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>on</a> <a href='SkBitmap_Reference#Bitmap'>a</a> 64-<a href='SkBitmap_Reference#Bitmap'>bit</a> <a href='SkBitmap_Reference#Bitmap'>platform</a> <a href='SkBitmap_Reference#Bitmap'>with</a>
<a href='SkBitmap_Reference#Bitmap'>sufficient</a> <a href='SkBitmap_Reference#Bitmap'>memory</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkBitmap_allocPixels'>allocPixels</a> <a href='#SkBitmap_Allocator'>Allocator</a> <a href='#Pixel_Ref'>Pixel_Ref</a>

<a name='SkBitmap_allocPixels_4'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_allocPixels'>allocPixels</a>(<a href='#SkBitmap_Allocator'>Allocator</a>* <a href='#SkBitmap_Allocator'>allocator</a>)
</pre>

Allocates <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a> <a href='undocumented#Pixel'>with</a> <a href='#SkBitmap_allocPixels_4_allocator'>allocator</a>, <a href='#SkBitmap_allocPixels_4_allocator'>and</a> <a href='#SkBitmap_allocPixels_4_allocator'>replaces</a> <a href='#SkBitmap_allocPixels_4_allocator'>existing</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>.
The allocation <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>is</a> <a href='undocumented#Size'>determined</a> <a href='undocumented#Size'>by</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>width</a>, <a href='SkImageInfo_Reference#SkImageInfo'>height</a>, <a href='SkImageInfo_Reference#SkImageInfo'>and</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.
If <a href='#SkBitmap_allocPixels_4_allocator'>allocator</a> <a href='#SkBitmap_allocPixels_4_allocator'>is</a> <a href='#SkBitmap_allocPixels_4_allocator'>nullptr</a>, <a href='#SkBitmap_allocPixels_4_allocator'>use</a> <a href='#SkBitmap_HeapAllocator'>HeapAllocator</a> <a href='#SkBitmap_HeapAllocator'>instead</a>.

Aborts if <a href='#SkBitmap_Allocator'>Allocator</a>::<a href='#SkBitmap_Allocator_allocPixelRef'>allocPixelRef</a> <a href='#SkBitmap_Allocator_allocPixelRef'>return</a> <a href='#SkBitmap_Allocator_allocPixelRef'>false</a>. <a href='#SkBitmap_Allocator_allocPixelRef'>Abort</a> <a href='#SkBitmap_Allocator_allocPixelRef'>steps</a> <a href='#SkBitmap_Allocator_allocPixelRef'>may</a> <a href='#SkBitmap_Allocator_allocPixelRef'>be</a> <a href='#SkBitmap_Allocator_allocPixelRef'>provided</a> <a href='#SkBitmap_Allocator_allocPixelRef'>by</a>
the user at compile time by defining SK_ABORT.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_allocPixels_4_allocator'><code><strong>allocator</strong></code></a></td>
    <td>instance of <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_Allocator'>Allocator</a> <a href='#SkBitmap_Allocator'>instantiation</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1b2800d23c9ea249b45c2c21a34b6d14"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_allocPixels'>allocPixels</a> <a href='#SkBitmap_Allocator'>Allocator</a> <a href='#Pixel_Ref'>Pixel_Ref</a>

<a name='SkBitmap_pixelRef'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkPixelRef'>SkPixelRef</a>* <a href='#SkBitmap_pixelRef'>pixelRef</a>() <a href='#SkBitmap_pixelRef'>const</a>
</pre>

Returns <a href='undocumented#SkPixelRef'>SkPixelRef</a>, <a href='undocumented#SkPixelRef'>which</a> <a href='undocumented#SkPixelRef'>contains</a>: <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>base</a> <a href='undocumented#Pixel'>address</a>; <a href='undocumented#Pixel'>its</a> <a href='undocumented#Pixel'>dimensions</a>; <a href='undocumented#Pixel'>and</a>
<a href='#SkBitmap_rowBytes'>rowBytes</a>(), <a href='#SkBitmap_rowBytes'>the</a> <a href='#SkBitmap_rowBytes'>interval</a> <a href='#SkBitmap_rowBytes'>from</a> <a href='#SkBitmap_rowBytes'>one</a> <a href='#SkBitmap_rowBytes'>row</a> <a href='#SkBitmap_rowBytes'>to</a> <a href='#SkBitmap_rowBytes'>the</a> <a href='#SkBitmap_rowBytes'>next</a>. <a href='#SkBitmap_rowBytes'>Does</a> <a href='#SkBitmap_rowBytes'>not</a> <a href='#SkBitmap_rowBytes'>change</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>
reference count. <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>may</a> <a href='undocumented#SkPixelRef'>be</a> <a href='undocumented#SkPixelRef'>shared</a> <a href='undocumented#SkPixelRef'>by</a> <a href='undocumented#SkPixelRef'>multiple</a> <a href='SkBitmap_Reference#Bitmap'>bitmaps</a>.
If <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>has</a> <a href='undocumented#SkPixelRef'>not</a> <a href='undocumented#SkPixelRef'>been</a> <a href='undocumented#SkPixelRef'>set</a>, <a href='undocumented#SkPixelRef'>returns</a> <a href='undocumented#SkPixelRef'>nullptr</a>.

### Return Value

<a href='undocumented#SkPixelRef'>SkPixelRef</a>, <a href='undocumented#SkPixelRef'>or</a> <a href='undocumented#SkPixelRef'>nullptr</a>

### Example

<div><fiddle-embed name="5db2d30870a7cc45f28e22578d1880c3"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_getPixels'>getPixels</a> <a href='#SkBitmap_getAddr'>getAddr</a>

<a name='SkBitmap_pixelRefOrigin'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='#SkBitmap_pixelRefOrigin'>pixelRefOrigin</a>() <a href='#SkBitmap_pixelRefOrigin'>const</a>
</pre>

Returns origin of pixels within <a href='undocumented#SkPixelRef'>SkPixelRef</a>. <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>bounds</a> <a href='SkBitmap_Reference#SkBitmap'>is</a> <a href='SkBitmap_Reference#SkBitmap'>always</a> <a href='SkBitmap_Reference#SkBitmap'>contained</a>
by <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>bounds</a>, <a href='undocumented#SkPixelRef'>which</a> <a href='undocumented#SkPixelRef'>may</a> <a href='undocumented#SkPixelRef'>be</a> <a href='undocumented#SkPixelRef'>the</a> <a href='undocumented#SkPixelRef'>same</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>or</a> <a href='undocumented#Size'>larger</a>. <a href='undocumented#Size'>Multiple</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>
can share the same <a href='undocumented#SkPixelRef'>SkPixelRef</a>, <a href='undocumented#SkPixelRef'>where</a> <a href='undocumented#SkPixelRef'>each</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>has</a> <a href='SkBitmap_Reference#SkBitmap'>different</a> <a href='SkBitmap_Reference#SkBitmap'>bounds</a>.

The returned origin added to <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>dimensions</a> <a href='SkBitmap_Reference#SkBitmap'>equals</a> <a href='SkBitmap_Reference#SkBitmap'>or</a> <a href='SkBitmap_Reference#SkBitmap'>is</a> <a href='SkBitmap_Reference#SkBitmap'>smaller</a> <a href='SkBitmap_Reference#SkBitmap'>than</a> <a href='SkBitmap_Reference#SkBitmap'>the</a>
<a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>dimensions</a>.

Returns (0, 0) if <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>is</a> <a href='undocumented#SkPixelRef'>nullptr</a>.

### Return Value

<a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>origin</a> <a href='undocumented#Pixel'>within</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>

### Example

<div><fiddle-embed name="6d31686c6c0829c70f284ae716526d6a">

#### Example Output

~~~~
source origin: 0, 0
subset origin: 32, 64
~~~~

</fiddle-embed></div>

### See Also

<a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='#SkBitmap_getSubset'>getSubset</a> <a href='#SkBitmap_setPixelRef'>setPixelRef</a>

<a name='Set'></a>

<a name='SkBitmap_setPixelRef'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_setPixelRef'>setPixelRef</a>(<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkPixelRef'>SkPixelRef</a>&<a href='undocumented#SkPixelRef'>gt</a>; <a href='#SkBitmap_pixelRef'>pixelRef</a>, <a href='#SkBitmap_pixelRef'>int</a> <a href='#SkBitmap_pixelRef'>dx</a>, <a href='#SkBitmap_pixelRef'>int</a> <a href='#SkBitmap_pixelRef'>dy</a>)
</pre>

Replaces <a href='#SkBitmap_setPixelRef_pixelRef'>pixelRef</a> <a href='#SkBitmap_setPixelRef_pixelRef'>and</a> <a href='#SkBitmap_setPixelRef_pixelRef'>origin</a> <a href='#SkBitmap_setPixelRef_pixelRef'>in</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>.  <a href='#SkBitmap_setPixelRef_dx'>dx</a> <a href='#SkBitmap_setPixelRef_dx'>and</a> <a href='#SkBitmap_setPixelRef_dy'>dy</a> <a href='#SkBitmap_setPixelRef_dy'>specify</a> <a href='#SkBitmap_setPixelRef_dy'>the</a> <a href='#SkBitmap_setPixelRef_dy'>offset</a>
within the <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>pixels</a> <a href='undocumented#SkPixelRef'>for</a> <a href='undocumented#SkPixelRef'>the</a> <a href='undocumented#SkPixelRef'>top-left</a> <a href='undocumented#SkPixelRef'>corner</a> <a href='undocumented#SkPixelRef'>of</a> <a href='undocumented#SkPixelRef'>the</a> <a href='SkBitmap_Reference#Bitmap'>bitmap</a>.

Asserts in debug builds if <a href='#SkBitmap_setPixelRef_dx'>dx</a> <a href='#SkBitmap_setPixelRef_dx'>or</a> <a href='#SkBitmap_setPixelRef_dy'>dy</a> <a href='#SkBitmap_setPixelRef_dy'>are</a> <a href='#SkBitmap_setPixelRef_dy'>out</a> <a href='#SkBitmap_setPixelRef_dy'>of</a> <a href='#SkBitmap_setPixelRef_dy'>range</a>. <a href='#SkBitmap_setPixelRef_dy'>Pins</a> <a href='#SkBitmap_setPixelRef_dx'>dx</a> <a href='#SkBitmap_setPixelRef_dx'>and</a> <a href='#SkBitmap_setPixelRef_dy'>dy</a>
to legal range in release builds.

The caller is responsible for ensuring that the pixels match the
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>and</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>in</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_setPixelRef_pixelRef'><code><strong>pixelRef</strong></code></a></td>
    <td><a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>describing</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>and</a> <a href='#SkBitmap_rowBytes'>rowBytes</a>()</td>
  </tr>
  <tr>    <td><a name='SkBitmap_setPixelRef_dx'><code><strong>dx</strong></code></a></td>
    <td>column offset in <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>for</a> <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='SkBitmap_Reference#Bitmap'>origin</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_setPixelRef_dy'><code><strong>dy</strong></code></a></td>
    <td>row offset in <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>for</a> <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='SkBitmap_Reference#Bitmap'>origin</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f98cc0451c6e77a8833d261c9a484c5f"><div>Treating 32-bit <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>as</a> 8-<a href='undocumented#Data'>bit</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>is</a> <a href='undocumented#Data'>unlikely</a> <a href='undocumented#Data'>to</a> <a href='undocumented#Data'>produce</a> <a href='undocumented#Data'>useful</a> <a href='undocumented#Data'>results</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkBitmap_setInfo'>setInfo</a>

<a name='SkBitmap_readyToDraw'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_readyToDraw'>readyToDraw</a>() <a href='#SkBitmap_readyToDraw'>const</a>
</pre>

Returns true if <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>is</a> <a href='SkBitmap_Reference#SkBitmap'>can</a> <a href='SkBitmap_Reference#SkBitmap'>be</a> <a href='SkBitmap_Reference#SkBitmap'>drawn</a>.

### Return Value

true if <a href='#SkBitmap_getPixels'>getPixels</a>() <a href='#SkBitmap_getPixels'>is</a> <a href='#SkBitmap_getPixels'>not</a> <a href='#SkBitmap_getPixels'>nullptr</a>

### Example

<div><fiddle-embed name="e89c78ca992e2e789ed50944fe68f920"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_getPixels'>getPixels</a> <a href='#SkBitmap_drawsNothing'>drawsNothing</a>

<a name='SkBitmap_getGenerationID'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkBitmap_getGenerationID'>getGenerationID</a>() <a href='#SkBitmap_getGenerationID'>const</a>
</pre>

Returns a unique value corresponding to the pixels in <a href='undocumented#SkPixelRef'>SkPixelRef</a>.
Returns a different value after <a href='#SkBitmap_notifyPixelsChanged'>notifyPixelsChanged</a>() <a href='#SkBitmap_notifyPixelsChanged'>has</a> <a href='#SkBitmap_notifyPixelsChanged'>been</a> <a href='#SkBitmap_notifyPixelsChanged'>called</a>.
Returns zero if <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>is</a> <a href='undocumented#SkPixelRef'>nullptr</a>.

Determines if pixels have changed since last examined.

### Return Value

unique value for pixels in <a href='undocumented#SkPixelRef'>SkPixelRef</a>

### Example

<div><fiddle-embed name="db9dd91e0207c3941c09538555817b4b">

#### Example Output

~~~~
#Volatile
empty id 0
alloc id 4
erase id 6
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_notifyPixelsChanged'>notifyPixelsChanged</a> <a href='#Pixel_Ref'>Pixel_Ref</a>

<a name='SkBitmap_notifyPixelsChanged'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_notifyPixelsChanged'>notifyPixelsChanged</a>() <a href='#SkBitmap_notifyPixelsChanged'>const</a>
</pre>

Marks that pixels in <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>have</a> <a href='undocumented#SkPixelRef'>changed</a>. <a href='undocumented#SkPixelRef'>Subsequent</a> <a href='undocumented#SkPixelRef'>calls</a> <a href='undocumented#SkPixelRef'>to</a>
<a href='#SkBitmap_getGenerationID'>getGenerationID</a>() <a href='#SkBitmap_getGenerationID'>return</a> <a href='#SkBitmap_getGenerationID'>a</a> <a href='#SkBitmap_getGenerationID'>different</a> <a href='#SkBitmap_getGenerationID'>value</a>.

### Example

<div><fiddle-embed name="8f463ed17b0ed4fb9c503a0ec71706f9"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_getGenerationID'>getGenerationID</a> <a href='#SkBitmap_isVolatile'>isVolatile</a> <a href='#Pixel_Ref'>Pixel_Ref</a>

<a name='Draw'></a>

<a name='SkBitmap_eraseColor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_eraseColor'>eraseColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColor'>c</a>) <a href='SkColor_Reference#SkColor'>const</a>
</pre>

Replaces <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>values</a> <a href='undocumented#Pixel'>with</a> <a href='#SkBitmap_eraseColor_c'>c</a>. <a href='#SkBitmap_eraseColor_c'>All</a> <a href='#SkBitmap_eraseColor_c'>pixels</a> <a href='#SkBitmap_eraseColor_c'>contained</a> <a href='#SkBitmap_eraseColor_c'>by</a> <a href='#SkBitmap_bounds'>bounds()</a> <a href='#SkBitmap_bounds'>are</a> <a href='#SkBitmap_bounds'>affected</a>.
If the <a href='#SkBitmap_colorType'>colorType</a>() <a href='#SkBitmap_colorType'>is</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>or</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>then</a> <a href='SkColor_Reference#Alpha'>alpha</a>
is ignored; RGB is treated as opaque. If <a href='#SkBitmap_colorType'>colorType</a>() <a href='#SkBitmap_colorType'>is</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>,
then RGB is ignored.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_eraseColor_c'><code><strong>c</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='SkColor_Reference#Color'>color</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="418928dbfffa9eb00c8225530f44baf5"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_eraseARGB'>eraseARGB</a> <a href='#SkBitmap_erase'>erase</a>

<a name='SkBitmap_eraseARGB'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_eraseARGB'>eraseARGB</a>(<a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>a</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>r</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>g</a>, <a href='undocumented#U8CPU'>U8CPU</a> <a href='undocumented#U8CPU'>b</a>) <a href='undocumented#U8CPU'>const</a>
</pre>

Replaces <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>values</a> <a href='undocumented#Pixel'>with</a> <a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>built</a> <a href='SkColor_Reference#Color'>from</a> <a href='#SkBitmap_eraseARGB_a'>a</a>, <a href='#SkBitmap_eraseARGB_r'>r</a>, <a href='#SkBitmap_eraseARGB_g'>g</a>, <a href='#SkBitmap_eraseARGB_g'>and</a> <a href='#SkBitmap_eraseARGB_b'>b</a>.
All pixels contained by <a href='#SkBitmap_bounds'>bounds()</a> <a href='#SkBitmap_bounds'>are</a> <a href='#SkBitmap_bounds'>affected</a>.
If the <a href='#SkBitmap_colorType'>colorType</a>() <a href='#SkBitmap_colorType'>is</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>or</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>then</a> <a href='#SkBitmap_eraseARGB_a'>a</a>
is ignored; <a href='#SkBitmap_eraseARGB_r'>r</a>, <a href='#SkBitmap_eraseARGB_g'>g</a>, <a href='#SkBitmap_eraseARGB_g'>and</a> <a href='#SkBitmap_eraseARGB_b'>b</a> <a href='#SkBitmap_eraseARGB_b'>are</a> <a href='#SkBitmap_eraseARGB_b'>treated</a> <a href='#SkBitmap_eraseARGB_b'>as</a> <a href='#SkBitmap_eraseARGB_b'>opaque</a>. <a href='#SkBitmap_eraseARGB_b'>If</a> <a href='#SkBitmap_colorType'>colorType</a>() <a href='#SkBitmap_colorType'>is</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>,
then <a href='#SkBitmap_eraseARGB_r'>r</a>, <a href='#SkBitmap_eraseARGB_g'>g</a>, <a href='#SkBitmap_eraseARGB_g'>and</a> <a href='#SkBitmap_eraseARGB_b'>b</a> <a href='#SkBitmap_eraseARGB_b'>are</a> <a href='#SkBitmap_eraseARGB_b'>ignored</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_eraseARGB_a'><code><strong>a</strong></code></a></td>
    <td>amount of <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='SkColor_Reference#Alpha'>from</a> <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>transparent</a> (0) <a href='SkColor_Reference#Alpha'>to</a> <a href='SkColor_Reference#Alpha'>fully</a> <a href='SkColor_Reference#Alpha'>opaque</a> (255)</td>
  </tr>
  <tr>    <td><a name='SkBitmap_eraseARGB_r'><code><strong>r</strong></code></a></td>
    <td>amount of red, from no red (0) to full red (255)</td>
  </tr>
  <tr>    <td><a name='SkBitmap_eraseARGB_g'><code><strong>g</strong></code></a></td>
    <td>amount of green, from no green (0) to full green (255)</td>
  </tr>
  <tr>    <td><a name='SkBitmap_eraseARGB_b'><code><strong>b</strong></code></a></td>
    <td>amount of blue, from no blue (0) to full blue (255)</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="67277b0a1003f340473a35982533561c"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_eraseColor'>eraseColor</a> <a href='#SkBitmap_erase'>erase</a>

<a name='SkBitmap_erase'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_erase'>erase</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColor'>c</a>, <a href='SkColor_Reference#SkColor'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>area</a>) <a href='SkIRect_Reference#SkIRect'>const</a>
</pre>

Replaces <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>values</a> <a href='undocumented#Pixel'>inside</a> <a href='#SkBitmap_erase_area'>area</a> <a href='#SkBitmap_erase_area'>with</a> <a href='#SkBitmap_erase_c'>c</a>. <a href='#SkBitmap_erase_c'>If</a> <a href='#SkBitmap_erase_area'>area</a> <a href='#SkBitmap_erase_area'>does</a> <a href='#SkBitmap_erase_area'>not</a> <a href='#SkBitmap_erase_area'>intersect</a> <a href='#SkBitmap_bounds'>bounds()</a>,
call has no effect.

If the <a href='#SkBitmap_colorType'>colorType</a>() <a href='#SkBitmap_colorType'>is</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>or</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>then</a> <a href='SkColor_Reference#Alpha'>alpha</a>
is ignored; RGB is treated as opaque. If <a href='#SkBitmap_colorType'>colorType</a>() <a href='#SkBitmap_colorType'>is</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>,
then RGB is ignored.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_erase_c'><code><strong>c</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='SkColor_Reference#Color'>color</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_erase_area'><code><strong>area</strong></code></a></td>
    <td>rectangle to fill</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="2c5c4230ccd2861a5d15b7cd2764ab6e"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_eraseColor'>eraseColor</a> <a href='#SkBitmap_eraseARGB'>eraseARGB</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawRect'>drawRect</a>

<a name='SkBitmap_eraseArea'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_eraseArea'>eraseArea</a>(<a href='#SkBitmap_eraseArea'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>area</a>, <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColor'>c</a>) <a href='SkColor_Reference#SkColor'>const</a>
</pre>

Deprecated.

<a name='SkBitmap_getColor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkBitmap_getColor'>getColor</a>(<a href='#SkBitmap_getColor'>int</a> <a href='#SkBitmap_getColor'>x</a>, <a href='#SkBitmap_getColor'>int</a> <a href='#SkBitmap_getColor'>y</a>) <a href='#SkBitmap_getColor'>const</a>
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkBitmap_getColor_x'>x</a>, <a href='#SkBitmap_getColor_y'>y</a>) <a href='#SkBitmap_getColor_y'>as</a> <a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='SkColor_Reference#Color'>color</a>.
Returns black with <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>if</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>.

Input is not validated: out of bounds values of <a href='#SkBitmap_getColor_x'>x</a> <a href='#SkBitmap_getColor_x'>or</a> <a href='#SkBitmap_getColor_y'>y</a> <a href='#SkBitmap_getColor_y'>trigger</a> <a href='#SkBitmap_getColor_y'>an</a> <a href='#SkBitmap_getColor_y'>assert()</a> <a href='#SkBitmap_getColor_y'>if</a>
built with SK_DEBUG defined; and returns undefined values or may crash if
SK_RELEASE is defined. Fails if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>or</a>
<a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>nullptr</a>.

<a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>in</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>is</a> <a href='SkImageInfo_Reference#SkImageInfo'>ignored</a>. <a href='SkImageInfo_Reference#SkImageInfo'>Some</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>precision</a> <a href='SkColor_Reference#Color'>may</a> <a href='SkColor_Reference#Color'>be</a> <a href='SkColor_Reference#Color'>lost</a> <a href='SkColor_Reference#Color'>in</a> <a href='SkColor_Reference#Color'>the</a>
conversion to <a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='SkColor_Reference#Color'>color</a>; <a href='SkColor_Reference#Color'>original</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>may</a> <a href='undocumented#Data'>have</a> <a href='undocumented#Data'>additional</a>
precision.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_getColor_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkBitmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_getColor_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkBitmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

<a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>converted</a> <a href='undocumented#Pixel'>to</a> <a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='SkColor_Reference#Color'>color</a>

### Example

<div><fiddle-embed name="193d1f6d8a43b7a8e9f27ba21de38617">

#### Example Output

~~~~
Premultiplied:
(0, 0) 0x00000000 0x2a0e002a 0x55380055 0x7f7f007f
(0, 1) 0x2a000e2a 0x551c1c55 0x7f542a7f 0xaaaa38aa
(0, 2) 0x55003855 0x7f2a547f 0xaa7171aa 0xd4d48dd4
(0, 3) 0x7f007f7f 0xaa38aaaa 0xd48dd4d4 0xffffffff
Unpremultiplied:
(0, 0) 0x00000000 0x2a5500ff 0x55a800ff 0x7fff00ff
(0, 1) 0x2a0055ff 0x555454ff 0x7fa954ff 0xaaff54ff
(0, 2) 0x5500a8ff 0x7f54a9ff 0xaaaaaaff 0xd4ffaaff
(0, 3) 0x7f00ffff 0xaa54ffff 0xd4aaffff 0xffffffff
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_getAlphaf'>getAlphaf</a> <a href='#SkBitmap_getAddr'>getAddr</a> <a href='#SkBitmap_readPixels'>readPixels</a>

<a name='SkBitmap_getAlphaf'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
float <a href='#SkBitmap_getAlphaf'>getAlphaf</a>(<a href='#SkBitmap_getAlphaf'>int</a> <a href='#SkBitmap_getAlphaf'>x</a>, <a href='#SkBitmap_getAlphaf'>int</a> <a href='#SkBitmap_getAlphaf'>y</a>) <a href='#SkBitmap_getAlphaf'>const</a>
</pre>

Looks up the <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkBitmap_getAlphaf_x'>x</a>,<a href='#SkBitmap_getAlphaf_y'>y</a>) <a href='#SkBitmap_getAlphaf_y'>and</a> <a href='#SkBitmap_getAlphaf_y'>return</a> <a href='#SkBitmap_getAlphaf_y'>its</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>component</a>, <a href='SkColor_Reference#Alpha'>normalized</a> <a href='SkColor_Reference#Alpha'>to</a> [0..1].
<a href='SkColor_Reference#Alpha'>This</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>roughly</a> <a href='SkColor_Reference#Alpha'>equivalent</a> <a href='SkColor_Reference#Alpha'>to</a> <code>SkGetColorA(<a href='#SkBitmap_getColor'>getColor</a>())</code>, but can be more efficient
(and more precise if the pixels store more than 8 bits per component).

### Parameters

<table>  <tr>    <td><a name='SkBitmap_getAlphaf_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkBitmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_getAlphaf_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkBitmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

<a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>converted</a> <a href='SkColor_Reference#Alpha'>to</a> <a href='SkColor_Reference#Alpha'>normalized</a> <a href='SkColor_Reference#Alpha'>float</a>

### See Also

<a href='#SkBitmap_getColor'>getColor</a>

<a name='SkBitmap_getAddr'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void* <a href='#SkBitmap_getAddr'>getAddr</a>(<a href='#SkBitmap_getAddr'>int</a> <a href='#SkBitmap_getAddr'>x</a>, <a href='#SkBitmap_getAddr'>int</a> <a href='#SkBitmap_getAddr'>y</a>) <a href='#SkBitmap_getAddr'>const</a>
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkBitmap_getAddr_x'>x</a>, <a href='#SkBitmap_getAddr_y'>y</a>).

Input is not validated: out of bounds values of <a href='#SkBitmap_getAddr_x'>x</a> <a href='#SkBitmap_getAddr_x'>or</a> <a href='#SkBitmap_getAddr_y'>y</a>, <a href='#SkBitmap_getAddr_y'>or</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>,
trigger an assert() if built with SK_DEBUG defined. Returns nullptr if
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kUnknown_SkColorType'>or</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>is</a> <a href='undocumented#SkPixelRef'>nullptr</a>.

Performs a lookup of <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Size'>size</a>; <a href='undocumented#Size'>for</a> <a href='undocumented#Size'>better</a> <a href='undocumented#Size'>performance</a>, <a href='undocumented#Size'>call</a>
one of: <a href='#SkBitmap_getAddr8'>getAddr8</a>(), <a href='#SkBitmap_getAddr16'>getAddr16</a>(), <a href='#SkBitmap_getAddr16'>or</a> <a href='#SkBitmap_getAddr32'>getAddr32</a>().

### Parameters

<table>  <tr>    <td><a name='SkBitmap_getAddr_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkBitmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_getAddr_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkBitmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

generic pointer to <a href='undocumented#Pixel'>pixel</a>

### Example

<div><fiddle-embed name="ffcefb2344cd38c3b99f69cfe6d64a17">

#### Example Output

~~~~
addr interval == rowBytes
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_getAddr8'>getAddr8</a> <a href='#SkBitmap_getAddr16'>getAddr16</a> <a href='#SkBitmap_getAddr32'>getAddr32</a> <a href='#SkBitmap_readPixels'>readPixels</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>::<a href='#SkPixmap_addr'>addr</a>

<a name='SkBitmap_getAddr32'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t* <a href='#SkBitmap_getAddr32'>getAddr32</a>(<a href='#SkBitmap_getAddr32'>int</a> <a href='#SkBitmap_getAddr32'>x</a>, <a href='#SkBitmap_getAddr32'>int</a> <a href='#SkBitmap_getAddr32'>y</a>) <a href='#SkBitmap_getAddr32'>const</a>
</pre>

Returns address at (<a href='#SkBitmap_getAddr32_x'>x</a>, <a href='#SkBitmap_getAddr32_y'>y</a>).

<a href='#SkBitmap_getAddr32_y'>Input</a> <a href='#SkBitmap_getAddr32_y'>is</a> <a href='#SkBitmap_getAddr32_y'>not</a> <a href='#SkBitmap_getAddr32_y'>validated</a>. <a href='#SkBitmap_getAddr32_y'>Triggers</a> <a href='#SkBitmap_getAddr32_y'>an</a> <a href='#SkBitmap_getAddr32_y'>assert()</a> <a href='#SkBitmap_getAddr32_y'>if</a> <a href='#SkBitmap_getAddr32_y'>built</a> <a href='#SkBitmap_getAddr32_y'>with</a> <a href='#SkBitmap_getAddr32_y'>SK_DEBUG</a> <a href='#SkBitmap_getAddr32_y'>defined</a> <a href='#SkBitmap_getAddr32_y'>and</a>:

<table>  <tr>
    <td><a href='#Pixel_Ref'>Pixel_Ref</a> <a href='#Pixel_Ref'>is</a> <a href='#Pixel_Ref'>nullptr</a></td>
  </tr>  <tr>
    <td><a href='#SkBitmap_bytesPerPixel'>bytesPerPixel</a>() <a href='#SkBitmap_bytesPerPixel'>is</a> <a href='#SkBitmap_bytesPerPixel'>not</a> <a href='#SkBitmap_bytesPerPixel'>four</a></td>
  </tr>  <tr>
    <td><a href='#SkBitmap_getAddr32_x'>x</a> <a href='#SkBitmap_getAddr32_x'>is</a> <a href='#SkBitmap_getAddr32_x'>negative</a>, <a href='#SkBitmap_getAddr32_x'>or</a> <a href='#SkBitmap_getAddr32_x'>not</a> <a href='#SkBitmap_getAddr32_x'>less</a> <a href='#SkBitmap_getAddr32_x'>than</a> <a href='#SkBitmap_width'>width()</a></td>
  </tr>  <tr>
    <td><a href='#SkBitmap_getAddr32_y'>y</a> <a href='#SkBitmap_getAddr32_y'>is</a> <a href='#SkBitmap_getAddr32_y'>negative</a>, <a href='#SkBitmap_getAddr32_y'>or</a> <a href='#SkBitmap_getAddr32_y'>not</a> <a href='#SkBitmap_getAddr32_y'>less</a> <a href='#SkBitmap_getAddr32_y'>than</a> <a href='#SkBitmap_height'>height()</a></td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkBitmap_getAddr32_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkBitmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_getAddr32_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkBitmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

unsigned 32-bit pointer to <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkBitmap_getAddr32_x'>x</a>, <a href='#SkBitmap_getAddr32_y'>y</a>)

### Example

<div><fiddle-embed name="837a2bcc9fb9ce617a3420956cefc64a">

#### Example Output

~~~~
addr interval == rowBytes
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_getAddr8'>getAddr8</a> <a href='#SkBitmap_getAddr16'>getAddr16</a> <a href='#SkBitmap_getAddr'>getAddr</a> <a href='#SkBitmap_readPixels'>readPixels</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>::<a href='#SkPixmap_addr32'>addr32</a>

<a name='SkBitmap_getAddr16'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint16_t* <a href='#SkBitmap_getAddr16'>getAddr16</a>(<a href='#SkBitmap_getAddr16'>int</a> <a href='#SkBitmap_getAddr16'>x</a>, <a href='#SkBitmap_getAddr16'>int</a> <a href='#SkBitmap_getAddr16'>y</a>) <a href='#SkBitmap_getAddr16'>const</a>
</pre>

Returns address at (<a href='#SkBitmap_getAddr16_x'>x</a>, <a href='#SkBitmap_getAddr16_y'>y</a>).

<a href='#SkBitmap_getAddr16_y'>Input</a> <a href='#SkBitmap_getAddr16_y'>is</a> <a href='#SkBitmap_getAddr16_y'>not</a> <a href='#SkBitmap_getAddr16_y'>validated</a>. <a href='#SkBitmap_getAddr16_y'>Triggers</a> <a href='#SkBitmap_getAddr16_y'>an</a> <a href='#SkBitmap_getAddr16_y'>assert()</a> <a href='#SkBitmap_getAddr16_y'>if</a> <a href='#SkBitmap_getAddr16_y'>built</a> <a href='#SkBitmap_getAddr16_y'>with</a> <a href='#SkBitmap_getAddr16_y'>SK_DEBUG</a> <a href='#SkBitmap_getAddr16_y'>defined</a> <a href='#SkBitmap_getAddr16_y'>and</a>:

<table>  <tr>
    <td><a href='#Pixel_Ref'>Pixel_Ref</a> <a href='#Pixel_Ref'>is</a> <a href='#Pixel_Ref'>nullptr</a></td>
  </tr>  <tr>
    <td><a href='#SkBitmap_bytesPerPixel'>bytesPerPixel</a>() <a href='#SkBitmap_bytesPerPixel'>is</a> <a href='#SkBitmap_bytesPerPixel'>not</a> <a href='#SkBitmap_bytesPerPixel'>two</a></td>
  </tr>  <tr>
    <td><a href='#SkBitmap_getAddr16_x'>x</a> <a href='#SkBitmap_getAddr16_x'>is</a> <a href='#SkBitmap_getAddr16_x'>negative</a>, <a href='#SkBitmap_getAddr16_x'>or</a> <a href='#SkBitmap_getAddr16_x'>not</a> <a href='#SkBitmap_getAddr16_x'>less</a> <a href='#SkBitmap_getAddr16_x'>than</a> <a href='#SkBitmap_width'>width()</a></td>
  </tr>  <tr>
    <td><a href='#SkBitmap_getAddr16_y'>y</a> <a href='#SkBitmap_getAddr16_y'>is</a> <a href='#SkBitmap_getAddr16_y'>negative</a>, <a href='#SkBitmap_getAddr16_y'>or</a> <a href='#SkBitmap_getAddr16_y'>not</a> <a href='#SkBitmap_getAddr16_y'>less</a> <a href='#SkBitmap_getAddr16_y'>than</a> <a href='#SkBitmap_height'>height()</a></td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkBitmap_getAddr16_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkBitmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_getAddr16_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkBitmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

unsigned 16-bit pointer to <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkBitmap_getAddr16_x'>x</a>, <a href='#SkBitmap_getAddr16_y'>y</a>)

### Example

<div><fiddle-embed name="53e00899ef2e00e2096daf7a07d9b059">

#### Example Output

~~~~
addr interval == rowBytes
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_getAddr8'>getAddr8</a> <a href='#SkBitmap_getAddr'>getAddr</a> <a href='#SkBitmap_getAddr32'>getAddr32</a> <a href='#SkBitmap_readPixels'>readPixels</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>::<a href='#SkPixmap_addr16'>addr16</a>

<a name='SkBitmap_getAddr8'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint8_t* <a href='#SkBitmap_getAddr8'>getAddr8</a>(<a href='#SkBitmap_getAddr8'>int</a> <a href='#SkBitmap_getAddr8'>x</a>, <a href='#SkBitmap_getAddr8'>int</a> <a href='#SkBitmap_getAddr8'>y</a>) <a href='#SkBitmap_getAddr8'>const</a>
</pre>

Returns address at (<a href='#SkBitmap_getAddr8_x'>x</a>, <a href='#SkBitmap_getAddr8_y'>y</a>).

<a href='#SkBitmap_getAddr8_y'>Input</a> <a href='#SkBitmap_getAddr8_y'>is</a> <a href='#SkBitmap_getAddr8_y'>not</a> <a href='#SkBitmap_getAddr8_y'>validated</a>. <a href='#SkBitmap_getAddr8_y'>Triggers</a> <a href='#SkBitmap_getAddr8_y'>an</a> <a href='#SkBitmap_getAddr8_y'>assert()</a> <a href='#SkBitmap_getAddr8_y'>if</a> <a href='#SkBitmap_getAddr8_y'>built</a> <a href='#SkBitmap_getAddr8_y'>with</a> <a href='#SkBitmap_getAddr8_y'>SK_DEBUG</a> <a href='#SkBitmap_getAddr8_y'>defined</a> <a href='#SkBitmap_getAddr8_y'>and</a>:

<table>  <tr>
    <td><a href='#Pixel_Ref'>Pixel_Ref</a> <a href='#Pixel_Ref'>is</a> <a href='#Pixel_Ref'>nullptr</a></td>
  </tr>  <tr>
    <td><a href='#SkBitmap_bytesPerPixel'>bytesPerPixel</a>() <a href='#SkBitmap_bytesPerPixel'>is</a> <a href='#SkBitmap_bytesPerPixel'>not</a> <a href='#SkBitmap_bytesPerPixel'>one</a></td>
  </tr>  <tr>
    <td><a href='#SkBitmap_getAddr8_x'>x</a> <a href='#SkBitmap_getAddr8_x'>is</a> <a href='#SkBitmap_getAddr8_x'>negative</a>, <a href='#SkBitmap_getAddr8_x'>or</a> <a href='#SkBitmap_getAddr8_x'>not</a> <a href='#SkBitmap_getAddr8_x'>less</a> <a href='#SkBitmap_getAddr8_x'>than</a> <a href='#SkBitmap_width'>width()</a></td>
  </tr>  <tr>
    <td><a href='#SkBitmap_getAddr8_y'>y</a> <a href='#SkBitmap_getAddr8_y'>is</a> <a href='#SkBitmap_getAddr8_y'>negative</a>, <a href='#SkBitmap_getAddr8_y'>or</a> <a href='#SkBitmap_getAddr8_y'>not</a> <a href='#SkBitmap_getAddr8_y'>less</a> <a href='#SkBitmap_getAddr8_y'>than</a> <a href='#SkBitmap_height'>height()</a></td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkBitmap_getAddr8_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkBitmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_getAddr8_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkBitmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

unsigned 8-bit pointer to <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkBitmap_getAddr8_x'>x</a>, <a href='#SkBitmap_getAddr8_y'>y</a>)

### Example

<div><fiddle-embed name="cb9a08e8ff779b6a1cf8bb54f3883aaf">

#### Example Output

~~~~
&pixels[4][2] == bitmap.getAddr8(2, 4)
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_getAddr'>getAddr</a> <a href='#SkBitmap_getAddr16'>getAddr16</a> <a href='#SkBitmap_getAddr32'>getAddr32</a> <a href='#SkBitmap_readPixels'>readPixels</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>::<a href='#SkPixmap_addr8'>addr8</a>

<a name='SkBitmap_extractSubset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_extractSubset'>extractSubset</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* <a href='SkBitmap_Reference#SkBitmap'>dst</a>, <a href='SkBitmap_Reference#SkBitmap'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>subset</a>) <a href='SkIRect_Reference#SkIRect'>const</a>
</pre>

Shares <a href='#Pixel_Ref'>Pixel_Ref</a> <a href='#Pixel_Ref'>with</a> <a href='#SkBitmap_extractSubset_dst'>dst</a>. <a href='#SkBitmap_extractSubset_dst'>Pixels</a> <a href='#SkBitmap_extractSubset_dst'>are</a> <a href='#SkBitmap_extractSubset_dst'>not</a> <a href='#SkBitmap_extractSubset_dst'>copied</a>; <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>and</a> <a href='#SkBitmap_extractSubset_dst'>dst</a> <a href='SkPoint_Reference#Point'>point</a>
<a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>the</a> <a href='SkPoint_Reference#Point'>same</a> <a href='SkPoint_Reference#Point'>pixels</a>; <a href='#SkBitmap_extractSubset_dst'>dst</a> <a href='#SkBitmap_bounds'>bounds()</a> <a href='#SkBitmap_bounds'>are</a> <a href='#SkBitmap_bounds'>set</a> <a href='#SkBitmap_bounds'>to</a> <a href='#SkBitmap_bounds'>the</a> <a href='#SkBitmap_bounds'>intersection</a> <a href='#SkBitmap_bounds'>of</a> <a href='#SkBitmap_extractSubset_subset'>subset</a>
<a href='#SkBitmap_extractSubset_subset'>and</a> <a href='#SkBitmap_extractSubset_subset'>the</a> <a href='#SkBitmap_extractSubset_subset'>original</a> <a href='#SkBitmap_bounds'>bounds()</a>.

<a href='#SkBitmap_extractSubset_subset'>subset</a> <a href='#SkBitmap_extractSubset_subset'>may</a> <a href='#SkBitmap_extractSubset_subset'>be</a> <a href='#SkBitmap_extractSubset_subset'>larger</a> <a href='#SkBitmap_extractSubset_subset'>than</a> <a href='#SkBitmap_bounds'>bounds()</a>. <a href='#SkBitmap_bounds'>Any</a> <a href='#SkBitmap_bounds'>area</a> <a href='#SkBitmap_bounds'>outside</a> <a href='#SkBitmap_bounds'>of</a> <a href='#SkBitmap_bounds'>bounds()</a> <a href='#SkBitmap_bounds'>is</a> <a href='#SkBitmap_bounds'>ignored</a>.

<a href='#SkBitmap_bounds'>Any</a> <a href='#SkBitmap_bounds'>contents</a> <a href='#SkBitmap_bounds'>of</a> <a href='#SkBitmap_extractSubset_dst'>dst</a> <a href='#SkBitmap_extractSubset_dst'>are</a> <a href='#SkBitmap_extractSubset_dst'>discarded</a>. <a href='#SkBitmap_isVolatile'>isVolatile</a> <a href='#SkBitmap_isVolatile'>setting</a> <a href='#SkBitmap_isVolatile'>is</a> <a href='#SkBitmap_isVolatile'>copied</a> <a href='#SkBitmap_isVolatile'>to</a> <a href='#SkBitmap_extractSubset_dst'>dst</a>.
<a href='#SkBitmap_extractSubset_dst'>dst</a> <a href='#SkBitmap_extractSubset_dst'>is</a> <a href='#SkBitmap_extractSubset_dst'>set</a> <a href='#SkBitmap_extractSubset_dst'>to</a> <a href='#SkBitmap_colorType'>colorType</a>, <a href='#SkBitmap_alphaType'>alphaType</a>, <a href='#SkBitmap_alphaType'>and</a> <a href='#SkBitmap_colorSpace'>colorSpace</a>.

<a href='#SkBitmap_colorSpace'>Return</a> <a href='#SkBitmap_colorSpace'>false</a> <a href='#SkBitmap_colorSpace'>if</a>:

<table>  <tr>
    <td><a href='#SkBitmap_extractSubset_dst'>dst</a> <a href='#SkBitmap_extractSubset_dst'>is</a> <a href='#SkBitmap_extractSubset_dst'>nullptr</a></td>
  </tr>  <tr>
    <td><a href='#Pixel_Ref'>Pixel_Ref</a> <a href='#Pixel_Ref'>is</a> <a href='#Pixel_Ref'>nullptr</a></td>
  </tr>  <tr>
    <td><a href='#SkBitmap_extractSubset_subset'>subset</a> <a href='#SkBitmap_extractSubset_subset'>does</a> <a href='#SkBitmap_extractSubset_subset'>not</a> <a href='#SkBitmap_extractSubset_subset'>intersect</a> <a href='#SkBitmap_bounds'>bounds()</a></td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkBitmap_extractSubset_dst'><code><strong>dst</strong></code></a></td>
    <td><a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>set</a> <a href='SkBitmap_Reference#Bitmap'>to</a> <a href='#SkBitmap_extractSubset_subset'>subset</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_extractSubset_subset'><code><strong>subset</strong></code></a></td>
    <td>rectangle of pixels to reference</td>
  </tr>
</table>

### Return Value

true if <a href='#SkBitmap_extractSubset_dst'>dst</a> <a href='#SkBitmap_extractSubset_dst'>is</a> <a href='#SkBitmap_extractSubset_dst'>replaced</a> <a href='#SkBitmap_extractSubset_dst'>by</a> <a href='#SkBitmap_extractSubset_subset'>subset</a>

### Example

<div><fiddle-embed name="304148c50c91490bfd58e9222342419c">

#### Example Output

~~~~
bounds: 0, 0, 512, 512
subset: -100,  100,    0,  200  success; false
subset: -100,  100,  100,  200  success; true  subset: 0, 0, 100, 100
subset: -100,  100, 1000,  200  success; true  subset: 0, 0, 512, 100
subset:    0,  100,    0,  200  success; false
subset:    0,  100,  100,  200  success; true  subset: 0, 0, 100, 100
subset:    0,  100, 1000,  200  success; true  subset: 0, 0, 512, 100
subset:  100,  100,    0,  200  success; false
subset:  100,  100,  100,  200  success; false
subset:  100,  100, 1000,  200  success; true  subset: 0, 0, 412, 100
subset: 1000,  100,    0,  200  success; false
subset: 1000,  100,  100,  200  success; false
subset: 1000,  100, 1000,  200  success; false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_readPixels'>readPixels</a> <a href='#SkBitmap_writePixels'>writePixels</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawBitmap'>drawBitmap</a>

<a name='SkBitmap_readPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_readPixels'>readPixels</a>(<a href='#SkBitmap_readPixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>dstInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>dstPixels</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>dstRowBytes</a>, <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>srcX</a>, <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>srcY</a>) <a href='SkImageInfo_Reference#SkImageInfo'>const</a>
</pre>

Copies a <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>pixels</a> <a href='SkRect_Reference#Rect'>from</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>to</a> <a href='#SkBitmap_readPixels_dstPixels'>dstPixels</a>. <a href='#SkBitmap_readPixels_dstPixels'>Copy</a> <a href='#SkBitmap_readPixels_dstPixels'>starts</a> <a href='#SkBitmap_readPixels_dstPixels'>at</a> (<a href='#SkBitmap_readPixels_srcX'>srcX</a>, <a href='#SkBitmap_readPixels_srcY'>srcY</a>),
<a href='#SkBitmap_readPixels_srcY'>and</a> <a href='#SkBitmap_readPixels_srcY'>does</a> <a href='#SkBitmap_readPixels_srcY'>not</a> <a href='#SkBitmap_readPixels_srcY'>exceed</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> (<a href='#SkBitmap_width'>width()</a>, <a href='#SkBitmap_height'>height()</a>).

<a href='#SkBitmap_readPixels_dstInfo'>dstInfo</a> <a href='#SkBitmap_readPixels_dstInfo'>specifies</a> <a href='#SkBitmap_readPixels_dstInfo'>width</a>, <a href='#SkBitmap_readPixels_dstInfo'>height</a>, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, <a href='#Image_Info_Alpha_Type'>and</a> <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>of</a>
<a href='#Color_Space'>destination</a>. <a href='#SkBitmap_readPixels_dstRowBytes'>dstRowBytes</a> <a href='#SkBitmap_readPixels_dstRowBytes'>specifics</a> <a href='#SkBitmap_readPixels_dstRowBytes'>the</a> <a href='#SkBitmap_readPixels_dstRowBytes'>gap</a> <a href='#SkBitmap_readPixels_dstRowBytes'>from</a> <a href='#SkBitmap_readPixels_dstRowBytes'>one</a> <a href='#SkBitmap_readPixels_dstRowBytes'>destination</a> <a href='#SkBitmap_readPixels_dstRowBytes'>row</a> <a href='#SkBitmap_readPixels_dstRowBytes'>to</a> <a href='#SkBitmap_readPixels_dstRowBytes'>the</a> <a href='#SkBitmap_readPixels_dstRowBytes'>next</a>.
<a href='#SkBitmap_readPixels_dstRowBytes'>Returns</a> <a href='#SkBitmap_readPixels_dstRowBytes'>true</a> <a href='#SkBitmap_readPixels_dstRowBytes'>if</a> <a href='#SkBitmap_readPixels_dstRowBytes'>pixels</a> <a href='#SkBitmap_readPixels_dstRowBytes'>are</a> <a href='#SkBitmap_readPixels_dstRowBytes'>copied</a>. <a href='#SkBitmap_readPixels_dstRowBytes'>Returns</a> <a href='#SkBitmap_readPixels_dstRowBytes'>false</a> <a href='#SkBitmap_readPixels_dstRowBytes'>if</a>:

<table>  <tr>
    <td><a href='#SkBitmap_readPixels_dstInfo'>dstInfo</a> <a href='#SkBitmap_readPixels_dstInfo'>has</a> <a href='#SkBitmap_readPixels_dstInfo'>no</a> <a href='#SkBitmap_readPixels_dstInfo'>address</a></td>
  </tr>  <tr>
    <td><a href='#SkBitmap_readPixels_dstRowBytes'>dstRowBytes</a> <a href='#SkBitmap_readPixels_dstRowBytes'>is</a> <a href='#SkBitmap_readPixels_dstRowBytes'>less</a> <a href='#SkBitmap_readPixels_dstRowBytes'>than</a> <a href='#SkBitmap_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>()</td>
  </tr>  <tr>
    <td><a href='#Pixel_Ref'>Pixel_Ref</a> <a href='#Pixel_Ref'>is</a> <a href='#Pixel_Ref'>nullptr</a></td>
  </tr>
</table>

Pixels are copied only if <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>possible</a>. <a href='undocumented#Pixel'>If</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorType'>colorType</a> <a href='#SkBitmap_colorType'>is</a>
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kGray_8_SkColorType'>or</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkBitmap_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorType'>colorType</a>() <a href='#SkImageInfo_colorType'>must</a> <a href='#SkImageInfo_colorType'>match</a>.
<a href='#SkImageInfo_colorType'>If</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorType'>colorType</a> <a href='#SkBitmap_colorType'>is</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkBitmap_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorSpace'>colorSpace</a>() <a href='#SkImageInfo_colorSpace'>must</a> <a href='#SkImageInfo_colorSpace'>match</a>.
<a href='#SkImageInfo_colorSpace'>If</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_alphaType'>alphaType</a> <a href='#SkBitmap_alphaType'>is</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkBitmap_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() <a href='#SkImageInfo_alphaType'>must</a>
<a href='#SkImageInfo_alphaType'>match</a>. <a href='#SkImageInfo_alphaType'>If</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorSpace'>colorSpace</a> <a href='#SkBitmap_colorSpace'>is</a> <a href='#SkBitmap_colorSpace'>nullptr</a>, <a href='#SkBitmap_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorSpace'>colorSpace</a>() <a href='#SkImageInfo_colorSpace'>must</a> <a href='#SkImageInfo_colorSpace'>match</a>. <a href='#SkImageInfo_colorSpace'>Returns</a>
<a href='#SkImageInfo_colorSpace'>false</a> <a href='#SkImageInfo_colorSpace'>if</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>not</a> <a href='undocumented#Pixel'>possible</a>.

<a href='#SkBitmap_readPixels_srcX'>srcX</a> <a href='#SkBitmap_readPixels_srcX'>and</a> <a href='#SkBitmap_readPixels_srcY'>srcY</a> <a href='#SkBitmap_readPixels_srcY'>may</a> <a href='#SkBitmap_readPixels_srcY'>be</a> <a href='#SkBitmap_readPixels_srcY'>negative</a> <a href='#SkBitmap_readPixels_srcY'>to</a> <a href='#SkBitmap_readPixels_srcY'>copy</a> <a href='#SkBitmap_readPixels_srcY'>only</a> <a href='#SkBitmap_readPixels_srcY'>top</a> <a href='#SkBitmap_readPixels_srcY'>or</a> <a href='#SkBitmap_readPixels_srcY'>left</a> <a href='#SkBitmap_readPixels_srcY'>of</a> <a href='#SkBitmap_readPixels_srcY'>source</a>. <a href='#SkBitmap_readPixels_srcY'>Returns</a>
<a href='#SkBitmap_readPixels_srcY'>false</a> <a href='#SkBitmap_readPixels_srcY'>if</a> <a href='#SkBitmap_width'>width()</a> <a href='#SkBitmap_width'>or</a> <a href='#SkBitmap_height'>height()</a> <a href='#SkBitmap_height'>is</a> <a href='#SkBitmap_height'>zero</a> <a href='#SkBitmap_height'>or</a> <a href='#SkBitmap_height'>negative</a>.
<a href='#SkBitmap_height'>Returns</a> <a href='#SkBitmap_height'>false</a> <a href='#SkBitmap_height'>if</a> <code><a href='undocumented#abs()'>abs</a>(<a href='#SkBitmap_readPixels_srcX'>srcX</a>) >= <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_width'>width()</a></code>, or if <code><a href='undocumented#abs()'>abs</a>(<a href='#SkBitmap_readPixels_srcY'>srcY</a>) >= <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_height'>height()</a></code>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_readPixels_dstInfo'><code><strong>dstInfo</strong></code></a></td>
    <td>destination width, height, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, <a href='#Color_Space'>Color_Space</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_readPixels_dstPixels'><code><strong>dstPixels</strong></code></a></td>
    <td>destination  <a href='undocumented#Pixel_Storage'>pixel storage</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_readPixels_dstRowBytes'><code><strong>dstRowBytes</strong></code></a></td>
    <td>destination row length</td>
  </tr>
  <tr>    <td><a name='SkBitmap_readPixels_srcX'><code><strong>srcX</strong></code></a></td>
    <td>column index whose absolute value is less than <a href='#SkBitmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_readPixels_srcY'><code><strong>srcY</strong></code></a></td>
    <td>row index whose absolute value is less than <a href='#SkBitmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href='#SkBitmap_readPixels_dstPixels'>dstPixels</a>

### Example

<div><fiddle-embed name="b2cbbbbcffb618865d8aae3bc04b2a62"><div>Transferring the gradient from 8 bits per component to 4 bits per component
creates visible banding.
</div></fiddle-embed></div>

### See Also

<a href='#SkBitmap_writePixels'>writePixels</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>::<a href='#SkPixmap_readPixels'>readPixels</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_readPixels'>readPixels</a> <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_readPixels'>readPixels</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_readPixels'>readPixels</a>

<a name='SkBitmap_readPixels_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_readPixels'>readPixels</a>(<a href='#SkBitmap_readPixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#SkPixmap'>dst</a>, <a href='SkPixmap_Reference#SkPixmap'>int</a> <a href='SkPixmap_Reference#SkPixmap'>srcX</a>, <a href='SkPixmap_Reference#SkPixmap'>int</a> <a href='SkPixmap_Reference#SkPixmap'>srcY</a>) <a href='SkPixmap_Reference#SkPixmap'>const</a>
</pre>

Copies a <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>pixels</a> <a href='SkRect_Reference#Rect'>from</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>to</a> <a href='#SkBitmap_readPixels_2_dst'>dst</a>. <a href='#SkBitmap_readPixels_2_dst'>Copy</a> <a href='#SkBitmap_readPixels_2_dst'>starts</a> <a href='#SkBitmap_readPixels_2_dst'>at</a> (<a href='#SkBitmap_readPixels_2_srcX'>srcX</a>, <a href='#SkBitmap_readPixels_2_srcY'>srcY</a>), <a href='#SkBitmap_readPixels_2_srcY'>and</a>
<a href='#SkBitmap_readPixels_2_srcY'>does</a> <a href='#SkBitmap_readPixels_2_srcY'>not</a> <a href='#SkBitmap_readPixels_2_srcY'>exceed</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> (<a href='#SkBitmap_width'>width()</a>, <a href='#SkBitmap_height'>height()</a>).

<a href='#SkBitmap_readPixels_2_dst'>dst</a> <a href='#SkBitmap_readPixels_2_dst'>specifies</a> <a href='#SkBitmap_readPixels_2_dst'>width</a>, <a href='#SkBitmap_readPixels_2_dst'>height</a>, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, <a href='#Color_Space'>Color_Space</a>,  <a href='undocumented#Pixel_Storage'>pixel storage</a>,
<a href='undocumented#Pixel'>and</a>  <a href='#Row_Bytes'>row bytes</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>destination</a>. <a href='#SkBitmap_readPixels_2_dst'>dst</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() <a href='#SkPixmap_rowBytes'>specifics</a> <a href='#SkPixmap_rowBytes'>the</a> <a href='#SkPixmap_rowBytes'>gap</a> <a href='#SkPixmap_rowBytes'>from</a> <a href='#SkPixmap_rowBytes'>one</a> <a href='#SkPixmap_rowBytes'>destination</a>
<a href='#SkPixmap_rowBytes'>row</a> <a href='#SkPixmap_rowBytes'>to</a> <a href='#SkPixmap_rowBytes'>the</a> <a href='#SkPixmap_rowBytes'>next</a>. <a href='#SkPixmap_rowBytes'>Returns</a> <a href='#SkPixmap_rowBytes'>true</a> <a href='#SkPixmap_rowBytes'>if</a> <a href='#SkPixmap_rowBytes'>pixels</a> <a href='#SkPixmap_rowBytes'>are</a> <a href='#SkPixmap_rowBytes'>copied</a>. <a href='#SkPixmap_rowBytes'>Returns</a> <a href='#SkPixmap_rowBytes'>false</a> <a href='#SkPixmap_rowBytes'>if</a>:

<table>  <tr>
    <td><a href='#SkBitmap_readPixels_2_dst'>dst</a>  <a href='undocumented#Pixel_Storage'>pixel storage</a> <a href='undocumented#Pixel'>equals</a> <a href='undocumented#Pixel'>nullptr</a></td>
  </tr>  <tr>
    <td><a href='#SkBitmap_readPixels_2_dst'>dst</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() <a href='#SkPixmap_rowBytes'>is</a> <a href='#SkPixmap_rowBytes'>less</a> <a href='#SkPixmap_rowBytes'>than</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>()</td>
  </tr>  <tr>
    <td><a href='#Pixel_Ref'>Pixel_Ref</a> <a href='#Pixel_Ref'>is</a> <a href='#Pixel_Ref'>nullptr</a></td>
  </tr>
</table>

Pixels are copied only if <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>possible</a>. <a href='undocumented#Pixel'>If</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorType'>colorType</a> <a href='#SkBitmap_colorType'>is</a>
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kGray_8_SkColorType'>or</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkBitmap_readPixels_2_dst'>dst</a> <a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Image_Info_Color_Type'>must</a> <a href='#Image_Info_Color_Type'>match</a>.
<a href='#Image_Info_Color_Type'>If</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorType'>colorType</a> <a href='#SkBitmap_colorType'>is</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkBitmap_readPixels_2_dst'>dst</a> <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>must</a> <a href='#Color_Space'>match</a>.
<a href='#Color_Space'>If</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_alphaType'>alphaType</a> <a href='#SkBitmap_alphaType'>is</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkBitmap_readPixels_2_dst'>dst</a> <a href='#Image_Info_Alpha_Type'>Alpha_Type</a> <a href='#Image_Info_Alpha_Type'>must</a>
<a href='#Image_Info_Alpha_Type'>match</a>. <a href='#Image_Info_Alpha_Type'>If</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorSpace'>colorSpace</a> <a href='#SkBitmap_colorSpace'>is</a> <a href='#SkBitmap_colorSpace'>nullptr</a>, <a href='#SkBitmap_readPixels_2_dst'>dst</a> <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>must</a> <a href='#Color_Space'>match</a>. <a href='#Color_Space'>Returns</a>
<a href='#Color_Space'>false</a> <a href='#Color_Space'>if</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>not</a> <a href='undocumented#Pixel'>possible</a>.

<a href='#SkBitmap_readPixels_2_srcX'>srcX</a> <a href='#SkBitmap_readPixels_2_srcX'>and</a> <a href='#SkBitmap_readPixels_2_srcY'>srcY</a> <a href='#SkBitmap_readPixels_2_srcY'>may</a> <a href='#SkBitmap_readPixels_2_srcY'>be</a> <a href='#SkBitmap_readPixels_2_srcY'>negative</a> <a href='#SkBitmap_readPixels_2_srcY'>to</a> <a href='#SkBitmap_readPixels_2_srcY'>copy</a> <a href='#SkBitmap_readPixels_2_srcY'>only</a> <a href='#SkBitmap_readPixels_2_srcY'>top</a> <a href='#SkBitmap_readPixels_2_srcY'>or</a> <a href='#SkBitmap_readPixels_2_srcY'>left</a> <a href='#SkBitmap_readPixels_2_srcY'>of</a> <a href='#SkBitmap_readPixels_2_srcY'>source</a>. <a href='#SkBitmap_readPixels_2_srcY'>Returns</a>
<a href='#SkBitmap_readPixels_2_srcY'>false</a> <a href='#SkBitmap_readPixels_2_srcY'>if</a> <a href='#SkBitmap_width'>width()</a> <a href='#SkBitmap_width'>or</a> <a href='#SkBitmap_height'>height()</a> <a href='#SkBitmap_height'>is</a> <a href='#SkBitmap_height'>zero</a> <a href='#SkBitmap_height'>or</a> <a href='#SkBitmap_height'>negative</a>.
<a href='#SkBitmap_height'>Returns</a> <a href='#SkBitmap_height'>false</a> <a href='#SkBitmap_height'>if</a> <code><a href='undocumented#abs()'>abs</a>(<a href='#SkBitmap_readPixels_2_srcX'>srcX</a>) >= <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_width'>width()</a></code>, or if <code><a href='undocumented#abs()'>abs</a>(<a href='#SkBitmap_readPixels_2_srcY'>srcY</a>) >= <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_height'>height()</a></code>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_readPixels_2_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkPixmap_Reference#Pixmap'>Pixmap</a>: <a href='#Image_Info'>Image_Info</a>, <a href='#Image_Info'>pixels</a>,  <a href='#Row_Bytes'>row bytes</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_readPixels_2_srcX'><code><strong>srcX</strong></code></a></td>
    <td>column index whose absolute value is less than <a href='#SkBitmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_readPixels_2_srcY'><code><strong>srcY</strong></code></a></td>
    <td>row index whose absolute value is less than <a href='#SkBitmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href='#SkBitmap_readPixels_2_dst'>dst</a>

### Example

<div><fiddle-embed name="e9f70cbc9827097449a386ec7a8a8188"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_writePixels'>writePixels</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>::<a href='#SkPixmap_readPixels'>readPixels</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_readPixels'>readPixels</a> <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_readPixels'>readPixels</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_readPixels'>readPixels</a>

<a name='SkBitmap_readPixels_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_readPixels'>readPixels</a>(<a href='#SkBitmap_readPixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#SkPixmap'>dst</a>) <a href='SkPixmap_Reference#SkPixmap'>const</a>
</pre>

Copies a <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>pixels</a> <a href='SkRect_Reference#Rect'>from</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>to</a> <a href='#SkBitmap_readPixels_3_dst'>dst</a>. <a href='#SkBitmap_readPixels_3_dst'>Copy</a> <a href='#SkBitmap_readPixels_3_dst'>starts</a> <a href='#SkBitmap_readPixels_3_dst'>at</a> (0, 0), <a href='#SkBitmap_readPixels_3_dst'>and</a>
<a href='#SkBitmap_readPixels_3_dst'>does</a> <a href='#SkBitmap_readPixels_3_dst'>not</a> <a href='#SkBitmap_readPixels_3_dst'>exceed</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> (<a href='#SkBitmap_width'>width()</a>, <a href='#SkBitmap_height'>height()</a>).

<a href='#SkBitmap_readPixels_3_dst'>dst</a> <a href='#SkBitmap_readPixels_3_dst'>specifies</a> <a href='#SkBitmap_readPixels_3_dst'>width</a>, <a href='#SkBitmap_readPixels_3_dst'>height</a>, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, <a href='#Color_Space'>Color_Space</a>,  <a href='undocumented#Pixel_Storage'>pixel storage</a>,
<a href='undocumented#Pixel'>and</a>  <a href='#Row_Bytes'>row bytes</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>destination</a>. <a href='#SkBitmap_readPixels_3_dst'>dst</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() <a href='#SkPixmap_rowBytes'>specifics</a> <a href='#SkPixmap_rowBytes'>the</a> <a href='#SkPixmap_rowBytes'>gap</a> <a href='#SkPixmap_rowBytes'>from</a> <a href='#SkPixmap_rowBytes'>one</a> <a href='#SkPixmap_rowBytes'>destination</a>
<a href='#SkPixmap_rowBytes'>row</a> <a href='#SkPixmap_rowBytes'>to</a> <a href='#SkPixmap_rowBytes'>the</a> <a href='#SkPixmap_rowBytes'>next</a>. <a href='#SkPixmap_rowBytes'>Returns</a> <a href='#SkPixmap_rowBytes'>true</a> <a href='#SkPixmap_rowBytes'>if</a> <a href='#SkPixmap_rowBytes'>pixels</a> <a href='#SkPixmap_rowBytes'>are</a> <a href='#SkPixmap_rowBytes'>copied</a>. <a href='#SkPixmap_rowBytes'>Returns</a> <a href='#SkPixmap_rowBytes'>false</a> <a href='#SkPixmap_rowBytes'>if</a>:

<table>  <tr>
    <td><a href='#SkBitmap_readPixels_3_dst'>dst</a>  <a href='undocumented#Pixel_Storage'>pixel storage</a> <a href='undocumented#Pixel'>equals</a> <a href='undocumented#Pixel'>nullptr</a></td>
  </tr>  <tr>
    <td><a href='#SkBitmap_readPixels_3_dst'>dst</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() <a href='#SkPixmap_rowBytes'>is</a> <a href='#SkPixmap_rowBytes'>less</a> <a href='#SkPixmap_rowBytes'>than</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>()</td>
  </tr>  <tr>
    <td><a href='#Pixel_Ref'>Pixel_Ref</a> <a href='#Pixel_Ref'>is</a> <a href='#Pixel_Ref'>nullptr</a></td>
  </tr>
</table>

Pixels are copied only if <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>possible</a>. <a href='undocumented#Pixel'>If</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorType'>colorType</a> <a href='#SkBitmap_colorType'>is</a>
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kGray_8_SkColorType'>or</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkBitmap_readPixels_3_dst'>dst</a> <a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Image_Info_Color_Type'>must</a> <a href='#Image_Info_Color_Type'>match</a>.
<a href='#Image_Info_Color_Type'>If</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorType'>colorType</a> <a href='#SkBitmap_colorType'>is</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkBitmap_readPixels_3_dst'>dst</a> <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>must</a> <a href='#Color_Space'>match</a>.
<a href='#Color_Space'>If</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_alphaType'>alphaType</a> <a href='#SkBitmap_alphaType'>is</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkBitmap_readPixels_3_dst'>dst</a> <a href='#Image_Info_Alpha_Type'>Alpha_Type</a> <a href='#Image_Info_Alpha_Type'>must</a>
<a href='#Image_Info_Alpha_Type'>match</a>. <a href='#Image_Info_Alpha_Type'>If</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorSpace'>colorSpace</a> <a href='#SkBitmap_colorSpace'>is</a> <a href='#SkBitmap_colorSpace'>nullptr</a>, <a href='#SkBitmap_readPixels_3_dst'>dst</a> <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>must</a> <a href='#Color_Space'>match</a>. <a href='#Color_Space'>Returns</a>
<a href='#Color_Space'>false</a> <a href='#Color_Space'>if</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>not</a> <a href='undocumented#Pixel'>possible</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_readPixels_3_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkPixmap_Reference#Pixmap'>Pixmap</a>: <a href='#Image_Info'>Image_Info</a>, <a href='#Image_Info'>pixels</a>,  <a href='#Row_Bytes'>row bytes</a></td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href='#SkBitmap_readPixels_3_dst'>dst</a>

### Example

<div><fiddle-embed name="4590fbf052659d6e629fbfd827081ae5"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_writePixels'>writePixels</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>::<a href='#SkPixmap_readPixels'>readPixels</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_readPixels'>readPixels</a> <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_readPixels'>readPixels</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_readPixels'>readPixels</a>

<a name='SkBitmap_writePixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_writePixels'>writePixels</a>(<a href='#SkBitmap_writePixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#SkPixmap'>src</a>, <a href='SkPixmap_Reference#SkPixmap'>int</a> <a href='SkPixmap_Reference#SkPixmap'>dstX</a>, <a href='SkPixmap_Reference#SkPixmap'>int</a> <a href='SkPixmap_Reference#SkPixmap'>dstY</a>)
</pre>

Copies a <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>pixels</a> <a href='SkRect_Reference#Rect'>from</a> <a href='#SkBitmap_writePixels_src'>src</a>. <a href='#SkBitmap_writePixels_src'>Copy</a> <a href='#SkBitmap_writePixels_src'>starts</a> <a href='#SkBitmap_writePixels_src'>at</a> (<a href='#SkBitmap_writePixels_dstX'>dstX</a>, <a href='#SkBitmap_writePixels_dstY'>dstY</a>), <a href='#SkBitmap_writePixels_dstY'>and</a> <a href='#SkBitmap_writePixels_dstY'>does</a> <a href='#SkBitmap_writePixels_dstY'>not</a> <a href='#SkBitmap_writePixels_dstY'>exceed</a>
(<a href='#SkBitmap_writePixels_src'>src</a>.<a href='#SkPixmap_width'>width()</a>, <a href='#SkBitmap_writePixels_src'>src</a>.<a href='#SkPixmap_height'>height()</a>).

<a href='#SkBitmap_writePixels_src'>src</a> <a href='#SkBitmap_writePixels_src'>specifies</a> <a href='#SkBitmap_writePixels_src'>width</a>, <a href='#SkBitmap_writePixels_src'>height</a>, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, <a href='#Color_Space'>Color_Space</a>,  <a href='undocumented#Pixel_Storage'>pixel storage</a>,
<a href='undocumented#Pixel'>and</a>  <a href='#Row_Bytes'>row bytes</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>source</a>. <a href='#SkBitmap_writePixels_src'>src</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() <a href='#SkPixmap_rowBytes'>specifics</a> <a href='#SkPixmap_rowBytes'>the</a> <a href='#SkPixmap_rowBytes'>gap</a> <a href='#SkPixmap_rowBytes'>from</a> <a href='#SkPixmap_rowBytes'>one</a> <a href='#SkPixmap_rowBytes'>source</a>
<a href='#SkPixmap_rowBytes'>row</a> <a href='#SkPixmap_rowBytes'>to</a> <a href='#SkPixmap_rowBytes'>the</a> <a href='#SkPixmap_rowBytes'>next</a>. <a href='#SkPixmap_rowBytes'>Returns</a> <a href='#SkPixmap_rowBytes'>true</a> <a href='#SkPixmap_rowBytes'>if</a> <a href='#SkPixmap_rowBytes'>pixels</a> <a href='#SkPixmap_rowBytes'>are</a> <a href='#SkPixmap_rowBytes'>copied</a>. <a href='#SkPixmap_rowBytes'>Returns</a> <a href='#SkPixmap_rowBytes'>false</a> <a href='#SkPixmap_rowBytes'>if</a>:

<table>  <tr>
    <td><a href='#SkBitmap_writePixels_src'>src</a>  <a href='undocumented#Pixel_Storage'>pixel storage</a> <a href='undocumented#Pixel'>equals</a> <a href='undocumented#Pixel'>nullptr</a></td>
  </tr>  <tr>
    <td><a href='#SkBitmap_writePixels_src'>src</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() <a href='#SkPixmap_rowBytes'>is</a> <a href='#SkPixmap_rowBytes'>less</a> <a href='#SkPixmap_rowBytes'>than</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>()</td>
  </tr>  <tr>
    <td><a href='#Pixel_Ref'>Pixel_Ref</a> <a href='#Pixel_Ref'>is</a> <a href='#Pixel_Ref'>nullptr</a></td>
  </tr>
</table>

Pixels are copied only if <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>possible</a>. <a href='undocumented#Pixel'>If</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorType'>colorType</a> <a href='#SkBitmap_colorType'>is</a>
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kGray_8_SkColorType'>or</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkBitmap_writePixels_src'>src</a> <a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Image_Info_Color_Type'>must</a> <a href='#Image_Info_Color_Type'>match</a>.
<a href='#Image_Info_Color_Type'>If</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorType'>colorType</a> <a href='#SkBitmap_colorType'>is</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkBitmap_writePixels_src'>src</a> <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>must</a> <a href='#Color_Space'>match</a>.
<a href='#Color_Space'>If</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_alphaType'>alphaType</a> <a href='#SkBitmap_alphaType'>is</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkBitmap_writePixels_src'>src</a> <a href='#Image_Info_Alpha_Type'>Alpha_Type</a> <a href='#Image_Info_Alpha_Type'>must</a>
<a href='#Image_Info_Alpha_Type'>match</a>. <a href='#Image_Info_Alpha_Type'>If</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorSpace'>colorSpace</a> <a href='#SkBitmap_colorSpace'>is</a> <a href='#SkBitmap_colorSpace'>nullptr</a>, <a href='#SkBitmap_writePixels_src'>src</a> <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>must</a> <a href='#Color_Space'>match</a>. <a href='#Color_Space'>Returns</a>
<a href='#Color_Space'>false</a> <a href='#Color_Space'>if</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>not</a> <a href='undocumented#Pixel'>possible</a>.

<a href='#SkBitmap_writePixels_dstX'>dstX</a> <a href='#SkBitmap_writePixels_dstX'>and</a> <a href='#SkBitmap_writePixels_dstY'>dstY</a> <a href='#SkBitmap_writePixels_dstY'>may</a> <a href='#SkBitmap_writePixels_dstY'>be</a> <a href='#SkBitmap_writePixels_dstY'>negative</a> <a href='#SkBitmap_writePixels_dstY'>to</a> <a href='#SkBitmap_writePixels_dstY'>copy</a> <a href='#SkBitmap_writePixels_dstY'>only</a> <a href='#SkBitmap_writePixels_dstY'>top</a> <a href='#SkBitmap_writePixels_dstY'>or</a> <a href='#SkBitmap_writePixels_dstY'>left</a> <a href='#SkBitmap_writePixels_dstY'>of</a> <a href='#SkBitmap_writePixels_dstY'>source</a>. <a href='#SkBitmap_writePixels_dstY'>Returns</a>
<a href='#SkBitmap_writePixels_dstY'>false</a> <a href='#SkBitmap_writePixels_dstY'>if</a> <a href='#SkBitmap_width'>width()</a> <a href='#SkBitmap_width'>or</a> <a href='#SkBitmap_height'>height()</a> <a href='#SkBitmap_height'>is</a> <a href='#SkBitmap_height'>zero</a> <a href='#SkBitmap_height'>or</a> <a href='#SkBitmap_height'>negative</a>.
<a href='#SkBitmap_height'>Returns</a> <a href='#SkBitmap_height'>false</a> <a href='#SkBitmap_height'>if</a> <code><a href='undocumented#abs()'>abs</a>(<a href='#SkBitmap_writePixels_dstX'>dstX</a>) >= <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_width'>width()</a></code>, or if <code><a href='undocumented#abs()'>abs</a>(<a href='#SkBitmap_writePixels_dstY'>dstY</a>) >= <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_height'>height()</a></code>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_writePixels_src'><code><strong>src</strong></code></a></td>
    <td>source <a href='SkPixmap_Reference#Pixmap'>Pixmap</a>: <a href='#Image_Info'>Image_Info</a>, <a href='#Image_Info'>pixels</a>,  <a href='#Row_Bytes'>row bytes</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_writePixels_dstX'><code><strong>dstX</strong></code></a></td>
    <td>column index whose absolute value is less than <a href='#SkBitmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_writePixels_dstY'><code><strong>dstY</strong></code></a></td>
    <td>row index whose absolute value is less than <a href='#SkBitmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkBitmap_writePixels_src'>src</a> <a href='#SkBitmap_writePixels_src'>pixels</a> <a href='#SkBitmap_writePixels_src'>are</a> <a href='#SkBitmap_writePixels_src'>copied</a> <a href='#SkBitmap_writePixels_src'>to</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a>

### Example

<div><fiddle-embed name="9b3133a6673d2514d166398adbe1f9f4"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_readPixels'>readPixels</a>

<a name='SkBitmap_writePixels_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_writePixels'>writePixels</a>(<a href='#SkBitmap_writePixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#SkPixmap'>src</a>)
</pre>

Copies a <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>pixels</a> <a href='SkRect_Reference#Rect'>from</a> <a href='#SkBitmap_writePixels_2_src'>src</a>. <a href='#SkBitmap_writePixels_2_src'>Copy</a> <a href='#SkBitmap_writePixels_2_src'>starts</a> <a href='#SkBitmap_writePixels_2_src'>at</a> (0, 0), <a href='#SkBitmap_writePixels_2_src'>and</a> <a href='#SkBitmap_writePixels_2_src'>does</a> <a href='#SkBitmap_writePixels_2_src'>not</a> <a href='#SkBitmap_writePixels_2_src'>exceed</a>
(<a href='#SkBitmap_writePixels_2_src'>src</a>.<a href='#SkPixmap_width'>width()</a>, <a href='#SkBitmap_writePixels_2_src'>src</a>.<a href='#SkPixmap_height'>height()</a>).

<a href='#SkBitmap_writePixels_2_src'>src</a> <a href='#SkBitmap_writePixels_2_src'>specifies</a> <a href='#SkBitmap_writePixels_2_src'>width</a>, <a href='#SkBitmap_writePixels_2_src'>height</a>, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, <a href='#Color_Space'>Color_Space</a>,  <a href='undocumented#Pixel_Storage'>pixel storage</a>,
<a href='undocumented#Pixel'>and</a>  <a href='#Row_Bytes'>row bytes</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>source</a>. <a href='#SkBitmap_writePixels_2_src'>src</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() <a href='#SkPixmap_rowBytes'>specifics</a> <a href='#SkPixmap_rowBytes'>the</a> <a href='#SkPixmap_rowBytes'>gap</a> <a href='#SkPixmap_rowBytes'>from</a> <a href='#SkPixmap_rowBytes'>one</a> <a href='#SkPixmap_rowBytes'>source</a>
<a href='#SkPixmap_rowBytes'>row</a> <a href='#SkPixmap_rowBytes'>to</a> <a href='#SkPixmap_rowBytes'>the</a> <a href='#SkPixmap_rowBytes'>next</a>. <a href='#SkPixmap_rowBytes'>Returns</a> <a href='#SkPixmap_rowBytes'>true</a> <a href='#SkPixmap_rowBytes'>if</a> <a href='#SkPixmap_rowBytes'>pixels</a> <a href='#SkPixmap_rowBytes'>are</a> <a href='#SkPixmap_rowBytes'>copied</a>. <a href='#SkPixmap_rowBytes'>Returns</a> <a href='#SkPixmap_rowBytes'>false</a> <a href='#SkPixmap_rowBytes'>if</a>:

<table>  <tr>
    <td><a href='#SkBitmap_writePixels_2_src'>src</a>  <a href='undocumented#Pixel_Storage'>pixel storage</a> <a href='undocumented#Pixel'>equals</a> <a href='undocumented#Pixel'>nullptr</a></td>
  </tr>  <tr>
    <td><a href='#SkBitmap_writePixels_2_src'>src</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() <a href='#SkPixmap_rowBytes'>is</a> <a href='#SkPixmap_rowBytes'>less</a> <a href='#SkPixmap_rowBytes'>than</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>()</td>
  </tr>  <tr>
    <td><a href='#Pixel_Ref'>Pixel_Ref</a> <a href='#Pixel_Ref'>is</a> <a href='#Pixel_Ref'>nullptr</a></td>
  </tr>
</table>

Pixels are copied only if <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>possible</a>. <a href='undocumented#Pixel'>If</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorType'>colorType</a> <a href='#SkBitmap_colorType'>is</a>
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kGray_8_SkColorType'>or</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkBitmap_writePixels_2_src'>src</a> <a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Image_Info_Color_Type'>must</a> <a href='#Image_Info_Color_Type'>match</a>.
<a href='#Image_Info_Color_Type'>If</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorType'>colorType</a> <a href='#SkBitmap_colorType'>is</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkBitmap_writePixels_2_src'>src</a> <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>must</a> <a href='#Color_Space'>match</a>.
<a href='#Color_Space'>If</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_alphaType'>alphaType</a> <a href='#SkBitmap_alphaType'>is</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkBitmap_writePixels_2_src'>src</a> <a href='#Image_Info_Alpha_Type'>Alpha_Type</a> <a href='#Image_Info_Alpha_Type'>must</a>
<a href='#Image_Info_Alpha_Type'>match</a>. <a href='#Image_Info_Alpha_Type'>If</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorSpace'>colorSpace</a> <a href='#SkBitmap_colorSpace'>is</a> <a href='#SkBitmap_colorSpace'>nullptr</a>, <a href='#SkBitmap_writePixels_2_src'>src</a> <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>must</a> <a href='#Color_Space'>match</a>. <a href='#Color_Space'>Returns</a>
<a href='#Color_Space'>false</a> <a href='#Color_Space'>if</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>not</a> <a href='undocumented#Pixel'>possible</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_writePixels_2_src'><code><strong>src</strong></code></a></td>
    <td>source <a href='SkPixmap_Reference#Pixmap'>Pixmap</a>: <a href='#Image_Info'>Image_Info</a>, <a href='#Image_Info'>pixels</a>,  <a href='#Row_Bytes'>row bytes</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkBitmap_writePixels_2_src'>src</a> <a href='#SkBitmap_writePixels_2_src'>pixels</a> <a href='#SkBitmap_writePixels_2_src'>are</a> <a href='#SkBitmap_writePixels_2_src'>copied</a> <a href='#SkBitmap_writePixels_2_src'>to</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a>

### Example

<div><fiddle-embed name="faa5dfa466f6e16c07c124d971f32679"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_readPixels'>readPixels</a>

<a name='SkBitmap_extractAlpha'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_extractAlpha'>extractAlpha</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* <a href='SkBitmap_Reference#SkBitmap'>dst</a>) <a href='SkBitmap_Reference#SkBitmap'>const</a>
</pre>

Sets <a href='#SkBitmap_extractAlpha_dst'>dst</a> <a href='#SkBitmap_extractAlpha_dst'>to</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>described</a> <a href='SkColor_Reference#Alpha'>by</a> <a href='SkColor_Reference#Alpha'>pixels</a>. <a href='SkColor_Reference#Alpha'>Returns</a> <a href='SkColor_Reference#Alpha'>false</a> <a href='SkColor_Reference#Alpha'>if</a> <a href='#SkBitmap_extractAlpha_dst'>dst</a> <a href='#SkBitmap_extractAlpha_dst'>cannot</a> <a href='#SkBitmap_extractAlpha_dst'>be</a> <a href='#SkBitmap_extractAlpha_dst'>written</a> <a href='#SkBitmap_extractAlpha_dst'>to</a>
or <a href='#SkBitmap_extractAlpha_dst'>dst</a> <a href='#SkBitmap_extractAlpha_dst'>pixels</a> <a href='#SkBitmap_extractAlpha_dst'>cannot</a> <a href='#SkBitmap_extractAlpha_dst'>be</a> <a href='#SkBitmap_extractAlpha_dst'>allocated</a>.

Uses <a href='#SkBitmap_HeapAllocator'>HeapAllocator</a> <a href='#SkBitmap_HeapAllocator'>to</a> <a href='#SkBitmap_HeapAllocator'>reserve</a> <a href='#SkBitmap_HeapAllocator'>memory</a> <a href='#SkBitmap_HeapAllocator'>for</a> <a href='#SkBitmap_extractAlpha_dst'>dst</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_extractAlpha_dst'><code><strong>dst</strong></code></a></td>
    <td>holds <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>to</a> <a href='undocumented#SkPixelRef'>fill</a> <a href='undocumented#SkPixelRef'>with</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkCanvas_Reference#Layer'>layer</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkCanvas_Reference#Layer'>layer</a> <a href='SkCanvas_Reference#Layer'>was</a> <a href='SkCanvas_Reference#Layer'>constructed</a> <a href='SkCanvas_Reference#Layer'>in</a> <a href='#SkBitmap_extractAlpha_dst'>dst</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>

### Example

<div><fiddle-embed name="ab6577df079e6c70511cf2bfc6447b44"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_extractSubset'>extractSubset</a>

<a name='SkBitmap_extractAlpha_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_extractAlpha'>extractAlpha</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* <a href='SkBitmap_Reference#SkBitmap'>dst</a>, <a href='SkBitmap_Reference#SkBitmap'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>* <a href='SkIPoint_Reference#SkIPoint'>offset</a>) <a href='SkIPoint_Reference#SkIPoint'>const</a>
</pre>

Sets <a href='#SkBitmap_extractAlpha_2_dst'>dst</a> <a href='#SkBitmap_extractAlpha_2_dst'>to</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>described</a> <a href='SkColor_Reference#Alpha'>by</a> <a href='SkColor_Reference#Alpha'>pixels</a>. <a href='SkColor_Reference#Alpha'>Returns</a> <a href='SkColor_Reference#Alpha'>false</a> <a href='SkColor_Reference#Alpha'>if</a> <a href='#SkBitmap_extractAlpha_2_dst'>dst</a> <a href='#SkBitmap_extractAlpha_2_dst'>cannot</a> <a href='#SkBitmap_extractAlpha_2_dst'>be</a> <a href='#SkBitmap_extractAlpha_2_dst'>written</a> <a href='#SkBitmap_extractAlpha_2_dst'>to</a>
or <a href='#SkBitmap_extractAlpha_2_dst'>dst</a> <a href='#SkBitmap_extractAlpha_2_dst'>pixels</a> <a href='#SkBitmap_extractAlpha_2_dst'>cannot</a> <a href='#SkBitmap_extractAlpha_2_dst'>be</a> <a href='#SkBitmap_extractAlpha_2_dst'>allocated</a>.

If <a href='#SkBitmap_extractAlpha_2_paint'>paint</a> <a href='#SkBitmap_extractAlpha_2_paint'>is</a> <a href='#SkBitmap_extractAlpha_2_paint'>not</a> <a href='#SkBitmap_extractAlpha_2_paint'>nullptr</a> <a href='#SkBitmap_extractAlpha_2_paint'>and</a> <a href='#SkBitmap_extractAlpha_2_paint'>contains</a> <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>
generates  <a href='undocumented#Mask_Alpha'>mask alpha</a> from <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>. <a href='SkBitmap_Reference#SkBitmap'>Uses</a> <a href='#SkBitmap_HeapAllocator'>HeapAllocator</a> <a href='#SkBitmap_HeapAllocator'>to</a> <a href='#SkBitmap_HeapAllocator'>reserve</a> <a href='#SkBitmap_HeapAllocator'>memory</a> <a href='#SkBitmap_HeapAllocator'>for</a> <a href='#SkBitmap_extractAlpha_2_dst'>dst</a>
<a href='undocumented#SkPixelRef'>SkPixelRef</a>. <a href='undocumented#SkPixelRef'>Sets</a> <a href='#SkBitmap_extractAlpha_2_offset'>offset</a> <a href='#SkBitmap_extractAlpha_2_offset'>to</a> <a href='#SkBitmap_extractAlpha_2_offset'>top-left</a> <a href='#SkBitmap_extractAlpha_2_offset'>position</a> <a href='#SkBitmap_extractAlpha_2_offset'>for</a> <a href='#SkBitmap_extractAlpha_2_dst'>dst</a> <a href='#SkBitmap_extractAlpha_2_dst'>for</a> <a href='#SkBitmap_extractAlpha_2_dst'>alignment</a> <a href='#SkBitmap_extractAlpha_2_dst'>with</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>;
(0, 0) unless <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> <a href='undocumented#SkMaskFilter'>generates</a> <a href='undocumented#SkMaskFilter'>mask</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_extractAlpha_2_dst'><code><strong>dst</strong></code></a></td>
    <td>holds <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>to</a> <a href='undocumented#SkPixelRef'>fill</a> <a href='undocumented#SkPixelRef'>with</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkCanvas_Reference#Layer'>layer</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_extractAlpha_2_paint'><code><strong>paint</strong></code></a></td>
    <td>holds optional <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>; <a href='undocumented#SkMaskFilter'>may</a> <a href='undocumented#SkMaskFilter'>be</a> <a href='undocumented#SkMaskFilter'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_extractAlpha_2_offset'><code><strong>offset</strong></code></a></td>
    <td>top-left position for <a href='#SkBitmap_extractAlpha_2_dst'>dst</a>; <a href='#SkBitmap_extractAlpha_2_dst'>may</a> <a href='#SkBitmap_extractAlpha_2_dst'>be</a> <a href='#SkBitmap_extractAlpha_2_dst'>nullptr</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkCanvas_Reference#Layer'>layer</a> <a href='SkCanvas_Reference#Layer'>was</a> <a href='SkCanvas_Reference#Layer'>constructed</a> <a href='SkCanvas_Reference#Layer'>in</a> <a href='#SkBitmap_extractAlpha_2_dst'>dst</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>

### Example

<div><fiddle-embed name="092739b4cd5d732a27c07ced8ef45f01"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_extractSubset'>extractSubset</a>

<a name='SkBitmap_extractAlpha_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_extractAlpha'>extractAlpha</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* <a href='SkBitmap_Reference#SkBitmap'>dst</a>, <a href='SkBitmap_Reference#SkBitmap'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='#SkBitmap_Allocator'>Allocator</a>* <a href='#SkBitmap_Allocator'>allocator</a>, <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>* <a href='SkIPoint_Reference#SkIPoint'>offset</a>) <a href='SkIPoint_Reference#SkIPoint'>const</a>
</pre>

Sets <a href='#SkBitmap_extractAlpha_3_dst'>dst</a> <a href='#SkBitmap_extractAlpha_3_dst'>to</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>described</a> <a href='SkColor_Reference#Alpha'>by</a> <a href='SkColor_Reference#Alpha'>pixels</a>. <a href='SkColor_Reference#Alpha'>Returns</a> <a href='SkColor_Reference#Alpha'>false</a> <a href='SkColor_Reference#Alpha'>if</a> <a href='#SkBitmap_extractAlpha_3_dst'>dst</a> <a href='#SkBitmap_extractAlpha_3_dst'>cannot</a> <a href='#SkBitmap_extractAlpha_3_dst'>be</a> <a href='#SkBitmap_extractAlpha_3_dst'>written</a> <a href='#SkBitmap_extractAlpha_3_dst'>to</a>
or <a href='#SkBitmap_extractAlpha_3_dst'>dst</a> <a href='#SkBitmap_extractAlpha_3_dst'>pixels</a> <a href='#SkBitmap_extractAlpha_3_dst'>cannot</a> <a href='#SkBitmap_extractAlpha_3_dst'>be</a> <a href='#SkBitmap_extractAlpha_3_dst'>allocated</a>.

If <a href='#SkBitmap_extractAlpha_3_paint'>paint</a> <a href='#SkBitmap_extractAlpha_3_paint'>is</a> <a href='#SkBitmap_extractAlpha_3_paint'>not</a> <a href='#SkBitmap_extractAlpha_3_paint'>nullptr</a> <a href='#SkBitmap_extractAlpha_3_paint'>and</a> <a href='#SkBitmap_extractAlpha_3_paint'>contains</a> <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>
generates  <a href='undocumented#Mask_Alpha'>mask alpha</a> from <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>. <a href='#SkBitmap_extractAlpha_3_allocator'>allocator</a> <a href='#SkBitmap_extractAlpha_3_allocator'>may</a> <a href='#SkBitmap_extractAlpha_3_allocator'>reference</a> <a href='#SkBitmap_extractAlpha_3_allocator'>a</a> <a href='#SkBitmap_extractAlpha_3_allocator'>custom</a> <a href='#SkBitmap_extractAlpha_3_allocator'>allocation</a>
class or be set to nullptr to use <a href='#SkBitmap_HeapAllocator'>HeapAllocator</a>. <a href='#SkBitmap_HeapAllocator'>Sets</a> <a href='#SkBitmap_extractAlpha_3_offset'>offset</a> <a href='#SkBitmap_extractAlpha_3_offset'>to</a> <a href='#SkBitmap_extractAlpha_3_offset'>top-left</a>
position for <a href='#SkBitmap_extractAlpha_3_dst'>dst</a> <a href='#SkBitmap_extractAlpha_3_dst'>for</a> <a href='#SkBitmap_extractAlpha_3_dst'>alignment</a> <a href='#SkBitmap_extractAlpha_3_dst'>with</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>; (0, 0) <a href='SkBitmap_Reference#SkBitmap'>unless</a> <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> <a href='undocumented#SkMaskFilter'>generates</a>
mask.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_extractAlpha_3_dst'><code><strong>dst</strong></code></a></td>
    <td>holds <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>to</a> <a href='undocumented#SkPixelRef'>fill</a> <a href='undocumented#SkPixelRef'>with</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkCanvas_Reference#Layer'>layer</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_extractAlpha_3_paint'><code><strong>paint</strong></code></a></td>
    <td>holds optional <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>; <a href='undocumented#SkMaskFilter'>may</a> <a href='undocumented#SkMaskFilter'>be</a> <a href='undocumented#SkMaskFilter'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_extractAlpha_3_allocator'><code><strong>allocator</strong></code></a></td>
    <td>function to reserve memory for <a href='undocumented#SkPixelRef'>SkPixelRef</a>; <a href='undocumented#SkPixelRef'>may</a> <a href='undocumented#SkPixelRef'>be</a> <a href='undocumented#SkPixelRef'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_extractAlpha_3_offset'><code><strong>offset</strong></code></a></td>
    <td>top-left position for <a href='#SkBitmap_extractAlpha_3_dst'>dst</a>; <a href='#SkBitmap_extractAlpha_3_dst'>may</a> <a href='#SkBitmap_extractAlpha_3_dst'>be</a> <a href='#SkBitmap_extractAlpha_3_dst'>nullptr</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkCanvas_Reference#Layer'>layer</a> <a href='SkCanvas_Reference#Layer'>was</a> <a href='SkCanvas_Reference#Layer'>constructed</a> <a href='SkCanvas_Reference#Layer'>in</a> <a href='#SkBitmap_extractAlpha_3_dst'>dst</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>

### Example

<div><fiddle-embed name="cd7543fa8c9f3cede46dc2d72eb8c4bd"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_extractSubset'>extractSubset</a>

<a name='SkBitmap_peekPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_peekPixels'>peekPixels</a>(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>* <a href='SkPixmap_Reference#Pixmap'>pixmap</a>) <a href='SkPixmap_Reference#Pixmap'>const</a>
</pre>

Copies <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a>,  <a href='#Row_Bytes'>row bytes</a>, <a href='undocumented#Pixel'>and</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>to</a> <a href='#SkBitmap_peekPixels_pixmap'>pixmap</a>, <a href='#SkBitmap_peekPixels_pixmap'>if</a> <a href='#SkBitmap_peekPixels_pixmap'>address</a>
is available, and returns true. If <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>not</a> <a href='undocumented#Pixel'>available</a>, <a href='undocumented#Pixel'>return</a>
false and leave <a href='#SkBitmap_peekPixels_pixmap'>pixmap</a> <a href='#SkBitmap_peekPixels_pixmap'>unchanged</a>.

<a href='#SkBitmap_peekPixels_pixmap'>pixmap</a> <a href='#SkBitmap_peekPixels_pixmap'>contents</a> <a href='#SkBitmap_peekPixels_pixmap'>become</a> <a href='#SkBitmap_peekPixels_pixmap'>invalid</a> <a href='#SkBitmap_peekPixels_pixmap'>on</a> <a href='#SkBitmap_peekPixels_pixmap'>any</a> <a href='#SkBitmap_peekPixels_pixmap'>future</a> <a href='#SkBitmap_peekPixels_pixmap'>change</a> <a href='#SkBitmap_peekPixels_pixmap'>to</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_peekPixels_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td>storage for <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>state</a> <a href='undocumented#Pixel'>if</a> <a href='undocumented#Pixel'>pixels</a> <a href='undocumented#Pixel'>are</a> <a href='undocumented#Pixel'>readable</a>; <a href='undocumented#Pixel'>otherwise</a>, <a href='undocumented#Pixel'>ignored</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>has</a> <a href='SkBitmap_Reference#SkBitmap'>direct</a> <a href='SkBitmap_Reference#SkBitmap'>access</a> <a href='SkBitmap_Reference#SkBitmap'>to</a> <a href='SkBitmap_Reference#SkBitmap'>pixels</a>

### Example

<div><fiddle-embed name="0cc2c6a0dffa61a88711534bd3d43b40">

#### Example Output

~~~~
------
-xxx--
x---x-
----x-
---x--
--x---
--x---
------
--x---
--x---
------
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkBitmap_peekPixels_pixmap'>pixmap</a> <a href='#SkBitmap_installPixels'>installPixels</a> <a href='#SkBitmap_readPixels'>readPixels</a> <a href='#SkBitmap_writePixels'>writePixels</a>

<a name='Utility'></a>

<a name='SkBitmap_validate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_validate'>validate()</a> <a href='#SkBitmap_validate'>const</a>;
</pre>

### See Also

<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_validate'>validate</a>

