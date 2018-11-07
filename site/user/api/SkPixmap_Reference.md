SkPixmap Reference
===


<a name='SkPixmap'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> {
<a href='SkPixmap_Reference#SkPixmap'>public</a>:
    <a href='#SkPixmap_empty_constructor'>SkPixmap()</a>;
    <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>(<a href='SkPixmap_Reference#SkPixmap'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='SkImageInfo_Reference#SkImageInfo'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>addr</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='#SkPixmap_rowBytes'>rowBytes</a>);
    <a href='#SkPixmap_rowBytes'>void</a> <a href='#SkPixmap_reset'>reset()</a>;
    <a href='#SkPixmap_reset'>void</a> <a href='#SkPixmap_reset'>reset</a>(<a href='#SkPixmap_reset'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='SkImageInfo_Reference#SkImageInfo'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>addr</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='#SkPixmap_rowBytes'>rowBytes</a>);
    <a href='#SkPixmap_rowBytes'>void</a> <a href='#SkPixmap_setColorSpace'>setColorSpace</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='#SkPixmap_colorSpace'>colorSpace</a>);
    <a href='#SkPixmap_colorSpace'>bool</a> <a href='#SkPixmap_colorSpace'>reset</a>(<a href='#SkPixmap_colorSpace'>const</a> <a href='undocumented#SkMask'>SkMask</a>& <a href='undocumented#SkMask'>mask</a>);
    <a href='undocumented#SkMask'>bool</a> <a href='#SkPixmap_extractSubset'>extractSubset</a>(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>* <a href='SkPixmap_Reference#SkPixmap'>subset</a>, <a href='SkPixmap_Reference#SkPixmap'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>area</a>) <a href='SkIRect_Reference#SkIRect'>const</a>;
    <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='#SkPixmap_info'>info()</a> <a href='#SkPixmap_info'>const</a>;
    <a href='#SkPixmap_info'>size_t</a> <a href='#SkPixmap_rowBytes'>rowBytes</a>() <a href='#SkPixmap_rowBytes'>const</a>;
    <a href='#SkPixmap_rowBytes'>const</a> <a href='#SkPixmap_rowBytes'>void</a>* <a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_addr'>const</a>;
    <a href='#SkPixmap_addr'>int</a> <a href='#SkPixmap_width'>width()</a> <a href='#SkPixmap_width'>const</a>;
    <a href='#SkPixmap_width'>int</a> <a href='#SkPixmap_height'>height()</a> <a href='#SkPixmap_height'>const</a>;
    <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkPixmap_colorType'>colorType</a>() <a href='#SkPixmap_colorType'>const</a>;
    <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkPixmap_alphaType'>alphaType</a>() <a href='#SkPixmap_alphaType'>const</a>;
    <a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='#SkPixmap_colorSpace'>colorSpace</a>() <a href='#SkPixmap_colorSpace'>const</a>;
    <a href='#SkPixmap_colorSpace'>bool</a> <a href='#SkPixmap_isOpaque'>isOpaque</a>() <a href='#SkPixmap_isOpaque'>const</a>;
    <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkPixmap_bounds'>bounds()</a> <a href='#SkPixmap_bounds'>const</a>;
    <a href='#SkPixmap_bounds'>int</a> <a href='#SkPixmap_rowBytesAsPixels'>rowBytesAsPixels</a>() <a href='#SkPixmap_rowBytesAsPixels'>const</a>;
    <a href='#SkPixmap_rowBytesAsPixels'>int</a> <a href='#SkPixmap_shiftPerPixel'>shiftPerPixel</a>() <a href='#SkPixmap_shiftPerPixel'>const</a>;
    <a href='#SkPixmap_shiftPerPixel'>size_t</a> <a href='#SkPixmap_computeByteSize'>computeByteSize</a>() <a href='#SkPixmap_computeByteSize'>const</a>;
    <a href='#SkPixmap_computeByteSize'>bool</a> <a href='#SkPixmap_computeIsOpaque'>computeIsOpaque</a>() <a href='#SkPixmap_computeIsOpaque'>const</a>;
    <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkPixmap_getColor'>getColor</a>(<a href='#SkPixmap_getColor'>int</a> <a href='#SkPixmap_getColor'>x</a>, <a href='#SkPixmap_getColor'>int</a> <a href='#SkPixmap_getColor'>y</a>) <a href='#SkPixmap_getColor'>const</a>;
    <a href='#SkPixmap_getColor'>float</a> <a href='#SkPixmap_getAlphaf'>getAlphaf</a>(<a href='#SkPixmap_getAlphaf'>int</a> <a href='#SkPixmap_getAlphaf'>x</a>, <a href='#SkPixmap_getAlphaf'>int</a> <a href='#SkPixmap_getAlphaf'>y</a>) <a href='#SkPixmap_getAlphaf'>const</a>;
    <a href='#SkPixmap_getAlphaf'>const</a> <a href='#SkPixmap_getAlphaf'>void</a>* <a href='#SkPixmap_addr'>addr</a>(<a href='#SkPixmap_addr'>int</a> <a href='#SkPixmap_addr'>x</a>, <a href='#SkPixmap_addr'>int</a> <a href='#SkPixmap_addr'>y</a>) <a href='#SkPixmap_addr'>const</a>;
    <a href='#SkPixmap_addr'>const</a> <a href='#SkPixmap_addr'>uint8_t</a>* <a href='#SkPixmap_addr8'>addr8</a>() <a href='#SkPixmap_addr8'>const</a>;
    <a href='#SkPixmap_addr8'>const</a> <a href='#SkPixmap_addr8'>uint16_t</a>* <a href='#SkPixmap_addr16'>addr16</a>() <a href='#SkPixmap_addr16'>const</a>;
    <a href='#SkPixmap_addr16'>const</a> <a href='#SkPixmap_addr16'>uint32_t</a>* <a href='#SkPixmap_addr32'>addr32</a>() <a href='#SkPixmap_addr32'>const</a>;
    <a href='#SkPixmap_addr32'>const</a> <a href='#SkPixmap_addr32'>uint64_t</a>* <a href='#SkPixmap_addr64'>addr64</a>() <a href='#SkPixmap_addr64'>const</a>;
    <a href='#SkPixmap_addr64'>const</a> <a href='#SkPixmap_addr64'>uint16_t</a>* <a href='#SkPixmap_addrF16'>addrF16</a>() <a href='#SkPixmap_addrF16'>const</a>;
    <a href='#SkPixmap_addrF16'>const</a> <a href='#SkPixmap_addrF16'>uint8_t</a>* <a href='#SkPixmap_addr8'>addr8</a>(<a href='#SkPixmap_addr8'>int</a> <a href='#SkPixmap_addr8'>x</a>, <a href='#SkPixmap_addr8'>int</a> <a href='#SkPixmap_addr8'>y</a>) <a href='#SkPixmap_addr8'>const</a>;
    <a href='#SkPixmap_addr8'>const</a> <a href='#SkPixmap_addr8'>uint16_t</a>* <a href='#SkPixmap_addr16'>addr16</a>(<a href='#SkPixmap_addr16'>int</a> <a href='#SkPixmap_addr16'>x</a>, <a href='#SkPixmap_addr16'>int</a> <a href='#SkPixmap_addr16'>y</a>) <a href='#SkPixmap_addr16'>const</a>;
    <a href='#SkPixmap_addr16'>const</a> <a href='#SkPixmap_addr16'>uint32_t</a>* <a href='#SkPixmap_addr32'>addr32</a>(<a href='#SkPixmap_addr32'>int</a> <a href='#SkPixmap_addr32'>x</a>, <a href='#SkPixmap_addr32'>int</a> <a href='#SkPixmap_addr32'>y</a>) <a href='#SkPixmap_addr32'>const</a>;
    <a href='#SkPixmap_addr32'>const</a> <a href='#SkPixmap_addr32'>uint64_t</a>* <a href='#SkPixmap_addr64'>addr64</a>(<a href='#SkPixmap_addr64'>int</a> <a href='#SkPixmap_addr64'>x</a>, <a href='#SkPixmap_addr64'>int</a> <a href='#SkPixmap_addr64'>y</a>) <a href='#SkPixmap_addr64'>const</a>;
    <a href='#SkPixmap_addr64'>const</a> <a href='#SkPixmap_addr64'>uint16_t</a>* <a href='#SkPixmap_addrF16'>addrF16</a>(<a href='#SkPixmap_addrF16'>int</a> <a href='#SkPixmap_addrF16'>x</a>, <a href='#SkPixmap_addrF16'>int</a> <a href='#SkPixmap_addrF16'>y</a>) <a href='#SkPixmap_addrF16'>const</a>;
    <a href='#SkPixmap_addrF16'>void</a>* <a href='#SkPixmap_writable_addr'>writable_addr</a>() <a href='#SkPixmap_writable_addr'>const</a>;
    <a href='#SkPixmap_writable_addr'>void</a>* <a href='#SkPixmap_writable_addr'>writable_addr</a>(<a href='#SkPixmap_writable_addr'>int</a> <a href='#SkPixmap_writable_addr'>x</a>, <a href='#SkPixmap_writable_addr'>int</a> <a href='#SkPixmap_writable_addr'>y</a>) <a href='#SkPixmap_writable_addr'>const</a>;
    <a href='#SkPixmap_writable_addr'>uint8_t</a>* <a href='#SkPixmap_writable_addr8'>writable_addr8</a>(<a href='#SkPixmap_writable_addr8'>int</a> <a href='#SkPixmap_writable_addr8'>x</a>, <a href='#SkPixmap_writable_addr8'>int</a> <a href='#SkPixmap_writable_addr8'>y</a>) <a href='#SkPixmap_writable_addr8'>const</a>;
    <a href='#SkPixmap_writable_addr8'>uint16_t</a>* <a href='#SkPixmap_writable_addr16'>writable_addr16</a>(<a href='#SkPixmap_writable_addr16'>int</a> <a href='#SkPixmap_writable_addr16'>x</a>, <a href='#SkPixmap_writable_addr16'>int</a> <a href='#SkPixmap_writable_addr16'>y</a>) <a href='#SkPixmap_writable_addr16'>const</a>;
    <a href='#SkPixmap_writable_addr16'>uint32_t</a>* <a href='#SkPixmap_writable_addr32'>writable_addr32</a>(<a href='#SkPixmap_writable_addr32'>int</a> <a href='#SkPixmap_writable_addr32'>x</a>, <a href='#SkPixmap_writable_addr32'>int</a> <a href='#SkPixmap_writable_addr32'>y</a>) <a href='#SkPixmap_writable_addr32'>const</a>;
    <a href='#SkPixmap_writable_addr32'>uint64_t</a>* <a href='#SkPixmap_writable_addr64'>writable_addr64</a>(<a href='#SkPixmap_writable_addr64'>int</a> <a href='#SkPixmap_writable_addr64'>x</a>, <a href='#SkPixmap_writable_addr64'>int</a> <a href='#SkPixmap_writable_addr64'>y</a>) <a href='#SkPixmap_writable_addr64'>const</a>;
    <a href='#SkPixmap_writable_addr64'>uint16_t</a>* <a href='#SkPixmap_writable_addrF16'>writable_addrF16</a>(<a href='#SkPixmap_writable_addrF16'>int</a> <a href='#SkPixmap_writable_addrF16'>x</a>, <a href='#SkPixmap_writable_addrF16'>int</a> <a href='#SkPixmap_writable_addrF16'>y</a>) <a href='#SkPixmap_writable_addrF16'>const</a>;
    <a href='#SkPixmap_writable_addrF16'>bool</a> <a href='#SkPixmap_readPixels'>readPixels</a>(<a href='#SkPixmap_readPixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>dstInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>dstPixels</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>dstRowBytes</a>) <a href='SkImageInfo_Reference#SkImageInfo'>const</a>;
    <a href='SkImageInfo_Reference#SkImageInfo'>bool</a> <a href='#SkPixmap_readPixels'>readPixels</a>(<a href='#SkPixmap_readPixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>dstInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>dstPixels</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>dstRowBytes</a>, <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>srcX</a>,
                    <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>srcY</a>) <a href='SkImageInfo_Reference#SkImageInfo'>const</a>;
    <a href='SkImageInfo_Reference#SkImageInfo'>bool</a> <a href='#SkPixmap_readPixels'>readPixels</a>(<a href='#SkPixmap_readPixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#SkPixmap'>dst</a>, <a href='SkPixmap_Reference#SkPixmap'>int</a> <a href='SkPixmap_Reference#SkPixmap'>srcX</a>, <a href='SkPixmap_Reference#SkPixmap'>int</a> <a href='SkPixmap_Reference#SkPixmap'>srcY</a>) <a href='SkPixmap_Reference#SkPixmap'>const</a>;
    <a href='SkPixmap_Reference#SkPixmap'>bool</a> <a href='#SkPixmap_readPixels'>readPixels</a>(<a href='#SkPixmap_readPixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#SkPixmap'>dst</a>) <a href='SkPixmap_Reference#SkPixmap'>const</a>;
    <a href='SkPixmap_Reference#SkPixmap'>bool</a> <a href='#SkPixmap_scalePixels'>scalePixels</a>(<a href='#SkPixmap_scalePixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#SkPixmap'>dst</a>, <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> <a href='undocumented#SkFilterQuality'>filterQuality</a>) <a href='undocumented#SkFilterQuality'>const</a>;
    <a href='undocumented#SkFilterQuality'>bool</a> <a href='#SkPixmap_erase'>erase</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>subset</a>) <a href='SkIRect_Reference#SkIRect'>const</a>;
    <a href='SkIRect_Reference#SkIRect'>bool</a> <a href='#SkPixmap_erase'>erase</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#Color'>color</a>) <a href='SkColor_Reference#Color'>const</a>;
    <a href='SkColor_Reference#Color'>bool</a> <a href='#SkPixmap_erase'>erase</a>(<a href='#SkPixmap_erase'>const</a> <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a>& <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* <a href='SkIRect_Reference#SkIRect'>subset</a> = <a href='SkIRect_Reference#SkIRect'>nullptr</a>) <a href='SkIRect_Reference#SkIRect'>const</a>;
};
</pre>

<a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='SkPixmap_Reference#Pixmap'>provides</a> <a href='SkPixmap_Reference#Pixmap'>a</a> <a href='SkPixmap_Reference#Pixmap'>utility</a> <a href='SkPixmap_Reference#Pixmap'>to</a> <a href='SkPixmap_Reference#Pixmap'>pair</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>with</a> <a href='SkImageInfo_Reference#SkImageInfo'>pixels</a> <a href='SkImageInfo_Reference#SkImageInfo'>and</a> <a href='SkImageInfo_Reference#SkImageInfo'>row</a> <a href='SkImageInfo_Reference#SkImageInfo'>bytes</a>.
<a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='SkPixmap_Reference#Pixmap'>is</a> <a href='SkPixmap_Reference#Pixmap'>a</a> <a href='SkPixmap_Reference#Pixmap'>low</a> <a href='SkPixmap_Reference#Pixmap'>level</a> <a href='SkPixmap_Reference#Pixmap'>class</a> <a href='SkPixmap_Reference#Pixmap'>which</a> <a href='SkPixmap_Reference#Pixmap'>provides</a> <a href='SkPixmap_Reference#Pixmap'>convenience</a> <a href='SkPixmap_Reference#Pixmap'>functions</a> <a href='SkPixmap_Reference#Pixmap'>to</a> <a href='SkPixmap_Reference#Pixmap'>access</a>
<a href='SkPixmap_Reference#Pixmap'>raster</a> <a href='SkPixmap_Reference#Pixmap'>destinations</a>. <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>can</a> <a href='SkCanvas_Reference#Canvas'>not</a> <a href='SkCanvas_Reference#Canvas'>draw</a> <a href='SkPixmap_Reference#Pixmap'>Pixmap</a>, <a href='SkPixmap_Reference#Pixmap'>nor</a> <a href='SkPixmap_Reference#Pixmap'>does</a> <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='SkPixmap_Reference#Pixmap'>provide</a>
<a href='SkPixmap_Reference#Pixmap'>a</a> <a href='SkPixmap_Reference#Pixmap'>direct</a> <a href='SkPixmap_Reference#Pixmap'>drawing</a> <a href='SkPixmap_Reference#Pixmap'>destination</a>.

<a href='SkPixmap_Reference#Pixmap'>Use</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>to</a> <a href='SkBitmap_Reference#Bitmap'>draw</a> <a href='SkBitmap_Reference#Bitmap'>pixels</a> <a href='SkBitmap_Reference#Bitmap'>referenced</a> <a href='SkBitmap_Reference#Bitmap'>by</a> <a href='SkPixmap_Reference#Pixmap'>Pixmap</a>; <a href='SkPixmap_Reference#Pixmap'>use</a> <a href='SkSurface_Reference#Surface'>Surface</a> <a href='SkSurface_Reference#Surface'>to</a> <a href='SkSurface_Reference#Surface'>draw</a> <a href='SkSurface_Reference#Surface'>into</a>
<a href='SkSurface_Reference#Surface'>pixels</a> <a href='SkSurface_Reference#Surface'>referenced</a> <a href='SkSurface_Reference#Surface'>by</a> <a href='SkPixmap_Reference#Pixmap'>Pixmap</a>.

<a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='SkPixmap_Reference#Pixmap'>does</a> <a href='SkPixmap_Reference#Pixmap'>not</a> <a href='SkPixmap_Reference#Pixmap'>try</a> <a href='SkPixmap_Reference#Pixmap'>to</a> <a href='SkPixmap_Reference#Pixmap'>manage</a> <a href='SkPixmap_Reference#Pixmap'>the</a> <a href='SkPixmap_Reference#Pixmap'>lifetime</a> <a href='SkPixmap_Reference#Pixmap'>of</a> <a href='SkPixmap_Reference#Pixmap'>the</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a>. <a href='undocumented#Pixel'>Use</a> <a href='#Pixel_Ref'>Pixel_Ref</a>
<a href='#Pixel_Ref'>to</a> <a href='#Pixel_Ref'>manage</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a>; <a href='#Pixel_Ref'>Pixel_Ref</a> <a href='#Pixel_Ref'>is</a> <a href='#Pixel_Ref'>safe</a> <a href='#Pixel_Ref'>across</a> <a href='#Pixel_Ref'>threads</a>.

<a name='Initialization'></a>

<a name='SkPixmap_empty_constructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPixmap_empty_constructor'>SkPixmap()</a>
</pre>

Creates an empty <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='SkPixmap_Reference#SkPixmap'>without</a> <a href='SkPixmap_Reference#SkPixmap'>pixels</a>, <a href='SkPixmap_Reference#SkPixmap'>with</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kUnknown_SkColorType'>with</a>
<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>and</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>with</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>a</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>width</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>and</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>height</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>of</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>zero</a>. <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>Use</a>
<a href='#SkPixmap_reset'>reset()</a> <a href='#SkPixmap_reset'>to</a> <a href='#SkPixmap_reset'>associate</a> <a href='#SkPixmap_reset'>pixels</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>width</a>, <a href='SkImageInfo_Reference#SkAlphaType'>and</a> <a href='SkImageInfo_Reference#SkAlphaType'>height</a>
after <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='SkPixmap_Reference#SkPixmap'>has</a> <a href='SkPixmap_Reference#SkPixmap'>been</a> <a href='SkPixmap_Reference#SkPixmap'>created</a>.

### Return Value

empty <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>

### Example

<div><fiddle-embed name="9547e74a9d37553a667b913ffd1312dd">

#### Example Output

~~~~
width:  0  height:  0  color: kUnknown_SkColorType  alpha: kUnknown_SkAlphaType
width: 25  height: 35  color: kRGBA_8888_SkColorType  alpha: kOpaque_SkAlphaType
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_const_SkImageInfo_const_star'>SkPixmap(const SkImageInfo& info, const void* addr, size_t rowBytes)</a> <a href='#SkPixmap_reset'>reset()</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>

<a name='SkPixmap_const_SkImageInfo_const_star'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>(<a href='SkPixmap_Reference#SkPixmap'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='SkImageInfo_Reference#SkImageInfo'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>addr</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='#SkPixmap_rowBytes'>rowBytes</a>)
</pre>

Creates <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='SkPixmap_Reference#SkPixmap'>from</a> <a href='#SkPixmap_const_SkImageInfo_const_star_info'>info</a> <a href='#SkPixmap_const_SkImageInfo_const_star_info'>width</a>, <a href='#SkPixmap_const_SkImageInfo_const_star_info'>height</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>and</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.
<a href='#SkPixmap_const_SkImageInfo_const_star_addr'>addr</a> <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>pixels</a>, <a href='SkPoint_Reference#Point'>or</a> <a href='SkPoint_Reference#Point'>nullptr</a>. <a href='#SkPixmap_const_SkImageInfo_const_star_rowBytes'>rowBytes</a> <a href='#SkPixmap_const_SkImageInfo_const_star_rowBytes'>should</a> <a href='#SkPixmap_const_SkImageInfo_const_star_rowBytes'>be</a> <a href='#SkPixmap_const_SkImageInfo_const_star_info'>info</a>.<a href='#SkImageInfo_width'>width()</a> <a href='#SkImageInfo_width'>times</a>
<a href='#SkPixmap_const_SkImageInfo_const_star_info'>info</a>.<a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>(), <a href='#SkImageInfo_bytesPerPixel'>or</a> <a href='#SkImageInfo_bytesPerPixel'>larger</a>.

No parameter checking is performed; it is up to the caller to ensure that
<a href='#SkPixmap_const_SkImageInfo_const_star_addr'>addr</a> <a href='#SkPixmap_const_SkImageInfo_const_star_addr'>and</a> <a href='#SkPixmap_const_SkImageInfo_const_star_rowBytes'>rowBytes</a> <a href='#SkPixmap_const_SkImageInfo_const_star_rowBytes'>agree</a> <a href='#SkPixmap_const_SkImageInfo_const_star_rowBytes'>with</a> <a href='#SkPixmap_const_SkImageInfo_const_star_info'>info</a>.

The memory lifetime of pixels is managed by the caller. When <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='SkPixmap_Reference#SkPixmap'>goes</a>
out of scope, <a href='#SkPixmap_const_SkImageInfo_const_star_addr'>addr</a> <a href='#SkPixmap_const_SkImageInfo_const_star_addr'>is</a> <a href='#SkPixmap_const_SkImageInfo_const_star_addr'>unaffected</a>.

<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='SkPixmap_Reference#SkPixmap'>may</a> <a href='SkPixmap_Reference#SkPixmap'>be</a> <a href='SkPixmap_Reference#SkPixmap'>later</a> <a href='SkPixmap_Reference#SkPixmap'>modified</a> <a href='SkPixmap_Reference#SkPixmap'>by</a> <a href='#SkPixmap_reset'>reset()</a> <a href='#SkPixmap_reset'>to</a> <a href='#SkPixmap_reset'>change</a> <a href='#SkPixmap_reset'>its</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>type</a>, <a href='undocumented#Pixel'>or</a>
storage.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_const_SkImageInfo_const_star_info'><code><strong>info</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>of</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_const_SkImageInfo_const_star_addr'><code><strong>addr</strong></code></a></td>
    <td>pointer to pixels allocated by caller; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkPixmap_const_SkImageInfo_const_star_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Size'>one</a> <a href='undocumented#Size'>row</a> <a href='undocumented#Size'>of</a> <a href='#SkPixmap_const_SkImageInfo_const_star_addr'>addr</a>; <a href='#SkPixmap_const_SkImageInfo_const_star_addr'>width</a> <a href='#SkPixmap_const_SkImageInfo_const_star_addr'>times</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Size'>or</a> <a href='undocumented#Size'>larger</a></td>
  </tr>
</table>

### Return Value

initialized <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>

### Example

<div><fiddle-embed name="9a00774be57d7308313b3a9073e6e696"><div><a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_MakeRasterCopy'>MakeRasterCopy</a> <a href='#SkImage_MakeRasterCopy'>takes</a> <a href='#SkImage_MakeRasterCopy'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#SkPixmap'>as</a> <a href='SkPixmap_Reference#SkPixmap'>an</a> <a href='SkPixmap_Reference#SkPixmap'>argument</a>. <a href='SkPixmap_Reference#SkPixmap'>The</a> <a href='SkPixmap_Reference#SkPixmap'>example</a>
<a href='SkPixmap_Reference#SkPixmap'>constructs</a> <a href='SkPixmap_Reference#SkPixmap'>a</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='SkPixmap_Reference#SkPixmap'>from</a> <a href='SkPixmap_Reference#SkPixmap'>the</a> <a href='SkPixmap_Reference#SkPixmap'>brace-delimited</a> <a href='SkPixmap_Reference#SkPixmap'>parameters</a>.
</div>

#### Example Output

~~~~
image alpha only = false
copy alpha only = true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_empty_constructor'>SkPixmap()</a> <a href='#SkPixmap_reset'>reset()</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>

<a name='SkPixmap_reset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPixmap_reset'>reset()</a>
</pre>

Sets width, height, row bytes to zero; <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>to</a> <a href='undocumented#Pixel'>nullptr</a>; <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>to</a>
<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>; <a href='SkImageInfo_Reference#kUnknown_SkColorType'>and</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>to</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>.

The prior pixels are unaffected; it is up to the caller to release pixels
memory if desired.

### Example

<div><fiddle-embed name="d9eb583c39f4f0baea79896b89245c98">

#### Example Output

~~~~
width: 25  height: 35  color: kRGBA_8888_SkColorType  alpha: kOpaque_SkAlphaType
width:  0  height:  0  color: kUnknown_SkColorType  alpha: kUnknown_SkAlphaType
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_empty_constructor'>SkPixmap()</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>

<a name='SkPixmap_reset_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void reset(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>info</a>, <a href='SkImageInfo_Reference#SkImageInfo'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>addr</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='#SkPixmap_rowBytes'>rowBytes</a>)
</pre>

Sets width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>and</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>from</a> <a href='#SkPixmap_reset_2_info'>info</a>.
Sets <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>from</a> <a href='#SkPixmap_reset_2_addr'>addr</a>, <a href='#SkPixmap_reset_2_addr'>which</a> <a href='#SkPixmap_reset_2_addr'>may</a> <a href='#SkPixmap_reset_2_addr'>be</a> <a href='#SkPixmap_reset_2_addr'>nullptr</a>.
Sets row bytes from <a href='#SkPixmap_reset_2_rowBytes'>rowBytes</a>, <a href='#SkPixmap_reset_2_rowBytes'>which</a> <a href='#SkPixmap_reset_2_rowBytes'>should</a> <a href='#SkPixmap_reset_2_rowBytes'>be</a> <a href='#SkPixmap_reset_2_info'>info</a>.<a href='#SkImageInfo_width'>width()</a> <a href='#SkImageInfo_width'>times</a>
<a href='#SkPixmap_reset_2_info'>info</a>.<a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>(), <a href='#SkImageInfo_bytesPerPixel'>or</a> <a href='#SkImageInfo_bytesPerPixel'>larger</a>.

Does not check <a href='#SkPixmap_reset_2_addr'>addr</a>. <a href='#SkPixmap_reset_2_addr'>Asserts</a> <a href='#SkPixmap_reset_2_addr'>if</a> <a href='#SkPixmap_reset_2_addr'>built</a> <a href='#SkPixmap_reset_2_addr'>with</a> <a href='#SkPixmap_reset_2_addr'>SK_DEBUG</a> <a href='#SkPixmap_reset_2_addr'>defined</a> <a href='#SkPixmap_reset_2_addr'>and</a> <a href='#SkPixmap_reset_2_addr'>if</a> <a href='#SkPixmap_reset_2_rowBytes'>rowBytes</a> <a href='#SkPixmap_reset_2_rowBytes'>is</a>
too small to hold one row of pixels.

The memory lifetime pixels are managed by the caller. When <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='SkPixmap_Reference#SkPixmap'>goes</a>
out of scope, <a href='#SkPixmap_reset_2_addr'>addr</a> <a href='#SkPixmap_reset_2_addr'>is</a> <a href='#SkPixmap_reset_2_addr'>unaffected</a>.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_reset_2_info'><code><strong>info</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>of</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_reset_2_addr'><code><strong>addr</strong></code></a></td>
    <td>pointer to pixels allocated by caller; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkPixmap_reset_2_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Size'>one</a> <a href='undocumented#Size'>row</a> <a href='undocumented#Size'>of</a> <a href='#SkPixmap_reset_2_addr'>addr</a>; <a href='#SkPixmap_reset_2_addr'>width</a> <a href='#SkPixmap_reset_2_addr'>times</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Size'>or</a> <a href='undocumented#Size'>larger</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="9a392b753167cfa849cebeefd5a6e07d"></fiddle-embed></div>

### See Also

<a href='#SkPixmap_const_SkImageInfo_const_star'>SkPixmap(const SkImageInfo& info, const void* addr, size_t rowBytes)</a> <a href='#SkPixmap_reset'>reset()</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>

<a name='SkPixmap_setColorSpace'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPixmap_setColorSpace'>setColorSpace</a>(<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&<a href='undocumented#SkColorSpace'>gt</a>; <a href='#SkPixmap_colorSpace'>colorSpace</a>)
</pre>

Changes <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>in</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>; <a href='SkImageInfo_Reference#SkImageInfo'>preserves</a> <a href='SkImageInfo_Reference#SkImageInfo'>width</a>, <a href='SkImageInfo_Reference#SkImageInfo'>height</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>and</a>
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>in</a> <a href='SkImage_Reference#SkImage'>SkImage</a>, <a href='SkImage_Reference#SkImage'>and</a> <a href='SkImage_Reference#SkImage'>leaves</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>and</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>bytes</a> <a href='undocumented#Pixel'>unchanged</a>.
<a href='undocumented#SkColorSpace'>SkColorSpace</a>  <a href='undocumented#Reference_Count'>reference count</a> <a href='undocumented#SkColorSpace'>is</a> <a href='undocumented#SkColorSpace'>incremented</a>.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_setColorSpace_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td><a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>moved</a> <a href='undocumented#SkColorSpace'>to</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="30d70aec4de17c831dba71e03dc9664a">

#### Example Output

~~~~
is unique
is not unique
~~~~

</fiddle-embed></div>

### See Also

<a href='#Color_Space'>Color_Space</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_makeColorSpace'>makeColorSpace</a>

<a name='SkPixmap_reset_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool reset(const <a href='undocumented#SkMask'>SkMask</a>& <a href='undocumented#SkMask'>mask</a>)
</pre>

To be deprecated soon.

<a name='SkPixmap_extractSubset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool extractSubset(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>* <a href='SkPixmap_Reference#SkPixmap'>subset</a>, <a href='SkPixmap_Reference#SkPixmap'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>area</a>) <a href='SkIRect_Reference#SkIRect'>const</a>
</pre>

Sets <a href='#SkPixmap_extractSubset_subset'>subset</a> <a href='#SkPixmap_extractSubset_subset'>width</a>, <a href='#SkPixmap_extractSubset_subset'>height</a>, <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>to</a> <a href='undocumented#Pixel'>intersection</a> <a href='undocumented#Pixel'>of</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='SkPixmap_Reference#SkPixmap'>with</a> <a href='#SkPixmap_extractSubset_area'>area</a>,
if intersection is not empty; and return true. Otherwise, leave <a href='#SkPixmap_extractSubset_subset'>subset</a> <a href='#SkPixmap_extractSubset_subset'>unchanged</a>
and return false.

Failing to read the return value generates a compile time warning.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_extractSubset_subset'><code><strong>subset</strong></code></a></td>
    <td>storage for width, height, <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>intersection</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_extractSubset_area'><code><strong>area</strong></code></a></td>
    <td>bounds to intersect with <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a></td>
  </tr>
</table>

### Return Value

true if intersection of <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='SkPixmap_Reference#SkPixmap'>and</a> <a href='#SkPixmap_extractSubset_area'>area</a> <a href='#SkPixmap_extractSubset_area'>is</a> <a href='#SkPixmap_extractSubset_area'>not</a> <a href='#SkPixmap_extractSubset_area'>empty</a>

### Example

<div><fiddle-embed name="febdbfac6cf4cde69837643be2e1f6dd"></fiddle-embed></div>

### See Also

<a href='#SkPixmap_reset'>reset()</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_intersect'>intersect</a>

<a name='Image_Info_Access'></a>

<a name='SkPixmap_info'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='#SkPixmap_info'>info()</a> <a href='#SkPixmap_info'>const</a>
</pre>

Returns width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkColorType'>and</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

### Return Value

reference to <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="6e0f558bf7fabc655041116288559134">

#### Example Output

~~~~
width: 384 height: 384 color: BGRA_8888 alpha: Opaque
~~~~

</fiddle-embed></div>

### See Also

<a href='#Image_Info'>Image_Info</a>

<a name='SkPixmap_rowBytes'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkPixmap_rowBytes'>rowBytes</a>() <a href='#SkPixmap_rowBytes'>const</a>
</pre>

Returns row bytes, the interval from one <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>to</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>next</a>. <a href='undocumented#Pixel'>Row</a> <a href='undocumented#Pixel'>bytes</a>
<a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>at</a> <a href='undocumented#Pixel'>least</a> <a href='undocumented#Pixel'>as</a> <a href='undocumented#Pixel'>large</a> <a href='undocumented#Pixel'>as</a>: <code><a href='#SkPixmap_width'>width()</a> * <a href='#SkPixmap_info'>info()</a>.<a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>()</code>.

Returns zero if <a href='#SkPixmap_colorType'>colorType</a> <a href='#SkPixmap_colorType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>.
<a href='SkImageInfo_Reference#kUnknown_SkColorType'>It</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>up</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>to</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>the</a> <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>creator</a> <a href='SkBitmap_Reference#Bitmap'>to</a> <a href='SkBitmap_Reference#Bitmap'>ensure</a> <a href='SkBitmap_Reference#Bitmap'>that</a> <a href='SkBitmap_Reference#Bitmap'>row</a> <a href='SkBitmap_Reference#Bitmap'>bytes</a> <a href='SkBitmap_Reference#Bitmap'>is</a> <a href='SkBitmap_Reference#Bitmap'>a</a> <a href='SkBitmap_Reference#Bitmap'>useful</a> <a href='SkBitmap_Reference#Bitmap'>value</a>.

### Return Value

byte length of <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a>

### Example

<div><fiddle-embed name="19ac8bb81854680bd408fec8cb797d5c">

#### Example Output

~~~~
rowBytes: 2 minRowBytes: 4
rowBytes: 8 minRowBytes: 4
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_info'>info()</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>

<a name='SkPixmap_addr'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const void* <a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_addr'>const</a>
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a>, <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>base</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>corresponding</a> <a href='undocumented#Pixel'>to</a> <a href='undocumented#Pixel'>the</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>origin</a>.

It is up to the <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='SkPixmap_Reference#SkPixmap'>creator</a> <a href='SkPixmap_Reference#SkPixmap'>to</a> <a href='SkPixmap_Reference#SkPixmap'>ensure</a> <a href='SkPixmap_Reference#SkPixmap'>that</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>a</a> <a href='undocumented#Pixel'>useful</a> <a href='undocumented#Pixel'>value</a>.

### Return Value

<a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a>

### Example

<div><fiddle-embed name="17bcabaaee2dbb7beba562e9ca50b55e">

#### Example Output

~~~~
#Volatile
pixels address: 0x7f2a440bb010
inset address:  0x7f2a440fb210
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr_2'>addr(int x, int y)</a> <a href='#SkPixmap_addr8'>addr8</a> <a href='#SkPixmap_addr16'>addr16</a> <a href='#SkPixmap_addr32'>addr32</a> <a href='#SkPixmap_addr64'>addr64</a> <a href='#SkPixmap_info'>info()</a> <a href='#SkPixmap_rowBytes'>rowBytes</a>()

<a name='SkPixmap_width'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPixmap_width'>width()</a> <a href='#SkPixmap_width'>const</a>
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>count</a> <a href='undocumented#Pixel'>in</a> <a href='undocumented#Pixel'>each</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a>. <a href='undocumented#Pixel'>Should</a> <a href='undocumented#Pixel'>be</a> <a href='undocumented#Pixel'>equal</a> <a href='undocumented#Pixel'>or</a> <a href='undocumented#Pixel'>less</a> <a href='undocumented#Pixel'>than</a>:

<code><a href='#SkPixmap_rowBytes'>rowBytes</a>() / <a href='#SkPixmap_info'>info()</a>.<a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>()</code>.

### Return Value

<a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>width</a> <a href='undocumented#Pixel'>in</a> <a href='#Image_Info'>Image_Info</a>

### Example

<div><fiddle-embed name="f68617b7153a20b2ed3d7f9ed5c6e5e4">

#### Example Output

~~~~
pixmap width: 16  info width: 16
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_height'>height()</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_width'>width()</a>

<a name='SkPixmap_height'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPixmap_height'>height()</a> <a href='#SkPixmap_height'>const</a>
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>row</a> <a href='undocumented#Pixel'>count</a>.

### Return Value

<a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>height</a> <a href='undocumented#Pixel'>in</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

### Example

<div><fiddle-embed name="4a996d32122f469d51ddd0186efb48cc">

#### Example Output

~~~~
pixmap height: 32  info height: 32
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_width'>width</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_height'>height</a>

<a name='SkPixmap_colorType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkPixmap_colorType'>colorType</a>() <a href='#SkPixmap_colorType'>const</a>
</pre>

Returns <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Color_Type'>one</a> <a href='#Image_Info_Color_Type'>of</a>: <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>
.

### Return Value

<a href='#Image_Info_Color_Type'>Color_Type</a> <a href='#Image_Info_Color_Type'>in</a> <a href='#Image_Info'>Image_Info</a>

### Example

<div><fiddle-embed name="0ab5c7af272685f2ce177cc79e6b9457">

#### Example Output

~~~~
color type: kAlpha_8_SkColorType
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_alphaType'>alphaType</a>() <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_colorType'>colorType</a>

<a name='SkPixmap_alphaType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkPixmap_alphaType'>alphaType</a>() <a href='#SkPixmap_alphaType'>const</a>
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

<a href='#SkPixmap_colorType'>colorType</a>() <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_alphaType'>alphaType</a>

<a name='SkPixmap_colorSpace'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='#SkPixmap_colorSpace'>colorSpace</a>() <a href='#SkPixmap_colorSpace'>const</a>
</pre>

Returns <a href='undocumented#SkColorSpace'>SkColorSpace</a>, <a href='undocumented#SkColorSpace'>the</a> <a href='undocumented#SkColorSpace'>range</a> <a href='undocumented#SkColorSpace'>of</a> <a href='undocumented#SkColorSpace'>colors</a>, <a href='undocumented#SkColorSpace'>associated</a> <a href='undocumented#SkColorSpace'>with</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>. <a href='SkImageInfo_Reference#SkImageInfo'>The</a>
reference count of <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>is</a> <a href='undocumented#SkColorSpace'>unchanged</a>. <a href='undocumented#SkColorSpace'>The</a> <a href='undocumented#SkColorSpace'>returned</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>is</a>
immutable.

### Return Value

<a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>in</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>or</a> <a href='SkImageInfo_Reference#SkImageInfo'>nullptr</a>

### Example

<div><fiddle-embed name="3421bb20a302d563832ba7bb45e0cc58"><div><a href='undocumented#SkColorSpace'>SkColorSpace</a>::<a href='#SkColorSpace_MakeSRGBLinear'>MakeSRGBLinear</a> <a href='#SkColorSpace_MakeSRGBLinear'>creates</a> <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>with</a> <a href='#Color_Space'>linear</a> <a href='#Color_Space'>gamma</a>
<a href='#Color_Space'>and</a> <a href='#Color_Space'>an</a> <a href='#Color_Space'>sRGB</a> <a href='#Color_Space'>gamut</a>. <a href='#Color_Space'>This</a> <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>gamma</a> <a href='#Color_Space'>is</a> <a href='#Color_Space'>not</a> <a href='#Color_Space'>close</a> <a href='#Color_Space'>to</a> <a href='#Color_Space'>sRGB</a> <a href='#Color_Space'>gamma</a>.
</div>

#### Example Output

~~~~
gammaCloseToSRGB: false  gammaIsLinear: true  isSRGB: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#Color_Space'>Color_Space</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_colorSpace'>colorSpace</a>

<a name='SkPixmap_isOpaque'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixmap_isOpaque'>isOpaque</a>() <a href='#SkPixmap_isOpaque'>const</a>
</pre>

Returns true if <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>is</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>.
Does not check if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>allows</a> <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='SkColor_Reference#Alpha'>or</a> <a href='SkColor_Reference#Alpha'>if</a> <a href='SkColor_Reference#Alpha'>any</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>value</a> <a href='undocumented#Pixel'>has</a>
transparency.

### Return Value

true if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>has</a> <a href='SkImageInfo_Reference#SkImageInfo'>opaque</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>

### Example

<div><fiddle-embed name="efd083f121e888a523455ea8a49e50d1"><div><a href='#SkPixmap_isOpaque'>isOpaque</a> <a href='#SkPixmap_isOpaque'>ignores</a> <a href='#SkPixmap_isOpaque'>whether</a> <a href='#SkPixmap_isOpaque'>all</a> <a href='#SkPixmap_isOpaque'>pixels</a> <a href='#SkPixmap_isOpaque'>are</a> <a href='#SkPixmap_isOpaque'>opaque</a> <a href='#SkPixmap_isOpaque'>or</a> <a href='#SkPixmap_isOpaque'>not</a>.
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

<a href='#SkPixmap_computeIsOpaque'>computeIsOpaque</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_isOpaque'>isOpaque</a>

<a name='SkPixmap_bounds'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkPixmap_bounds'>bounds()</a> <a href='#SkPixmap_bounds'>const</a>
</pre>

Returns <a href='SkIRect_Reference#SkIRect'>SkIRect</a> { 0, 0, <a href='#SkPixmap_width'>width()</a>, <a href='#SkPixmap_height'>height()</a> }.

### Return Value

integral rectangle from origin to <a href='#SkPixmap_width'>width()</a> <a href='#SkPixmap_width'>and</a> <a href='#SkPixmap_height'>height()</a>

### Example

<div><fiddle-embed name="79750fb1d898a4e5c8c828b7bc9acec5">

#### Example Output

~~~~
width: 0 height: 0 empty: true
width: 0 height: 2 empty: true
width: 2 height: 0 empty: true
width: 2 height: 2 empty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_height'>height()</a> <a href='#SkPixmap_width'>width()</a> <a href='SkIRect_Reference#IRect'>IRect</a>

<a name='SkPixmap_rowBytesAsPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPixmap_rowBytesAsPixels'>rowBytesAsPixels</a>() <a href='#SkPixmap_rowBytesAsPixels'>const</a>
</pre>

Returns number of pixels that fit on row. Should be greater than or equal to
<a href='#SkPixmap_width'>width()</a>.

### Return Value

maximum pixels per row

### Example

<div><fiddle-embed name="6231bb212d0c231b5bc44eac626fbcb5">

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

<a href='#SkPixmap_rowBytes'>rowBytes</a> <a href='#SkPixmap_shiftPerPixel'>shiftPerPixel</a> <a href='#SkPixmap_width'>width</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>

<a name='SkPixmap_shiftPerPixel'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPixmap_shiftPerPixel'>shiftPerPixel</a>() <a href='#SkPixmap_shiftPerPixel'>const</a>
</pre>

Returns bit shift converting row bytes to row pixels.
Returns zero for <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>.

### Return Value

one of: 0, 1, 2, 3; left shift to convert pixels to bytes

### Example

<div><fiddle-embed name="bf31ee140e2c163c3957276e6d4c4f0c">

#### Example Output

~~~~
color: kUnknown_SkColorType   bytesPerPixel: 0 shiftPerPixel: 0
color: kAlpha_8_SkColorType   bytesPerPixel: 1 shiftPerPixel: 0
color: kRGB_565_SkColorType   bytesPerPixel: 2 shiftPerPixel: 1
color: kARGB_4444_SkColorType bytesPerPixel: 2 shiftPerPixel: 1
color: kRGBA_8888_SkColorType bytesPerPixel: 4 shiftPerPixel: 2
color: kBGRA_8888_SkColorType bytesPerPixel: 4 shiftPerPixel: 2
color: kGray_8_SkColorType    bytesPerPixel: 1 shiftPerPixel: 0
color: kRGBA_F16_SkColorType  bytesPerPixel: 8 shiftPerPixel: 3
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_rowBytes'>rowBytes</a> <a href='#SkPixmap_rowBytesAsPixels'>rowBytesAsPixels</a> <a href='#SkPixmap_width'>width</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>

<a name='SkPixmap_computeByteSize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkPixmap_computeByteSize'>computeByteSize</a>() <a href='#SkPixmap_computeByteSize'>const</a>
</pre>

Returns minimum memory required for <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>storage</a>.
Does not include unused memory on last row when <a href='#SkPixmap_rowBytesAsPixels'>rowBytesAsPixels</a>() <a href='#SkPixmap_rowBytesAsPixels'>exceeds</a> <a href='#SkPixmap_width'>width()</a>.
Returns zero if result does not fit in size_t.
Returns zero if <a href='#SkPixmap_height'>height()</a> <a href='#SkPixmap_height'>or</a> <a href='#SkPixmap_width'>width()</a> <a href='#SkPixmap_width'>is</a> 0.
Returns <a href='#SkPixmap_height'>height()</a> <a href='#SkPixmap_height'>times</a> <a href='#SkPixmap_rowBytes'>rowBytes</a>() <a href='#SkPixmap_rowBytes'>if</a> <a href='#SkPixmap_colorType'>colorType</a>() <a href='#SkPixmap_colorType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>.

### Return Value

<a href='undocumented#Size'>size</a> <a href='undocumented#Size'>in</a> <a href='undocumented#Size'>bytes</a> <a href='undocumented#Size'>of</a> <a href='SkImage_Reference#Image'>image</a> <a href='SkImage_Reference#Image'>buffer</a>

### Example

<div><fiddle-embed name="410d14ddc45d272598c5a4e52bb047de">

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

<a name='Reader'></a>

<a name='SkPixmap_computeIsOpaque'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixmap_computeIsOpaque'>computeIsOpaque</a>() <a href='#SkPixmap_computeIsOpaque'>const</a>
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

### Return Value

true if all pixels have opaque values or <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>opaque</a>

### Example

<div><fiddle-embed name="6ef37d5be03d0bfaec992dbb5a94c66f">

#### Example Output

~~~~
computeIsOpaque: false
computeIsOpaque: true
computeIsOpaque: false
computeIsOpaque: true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_isOpaque'>isOpaque</a> <a href='#Image_Info_Color_Type'>Color_Type</a> <a href='SkColor_Reference#Alpha'>Alpha</a>

<a name='SkPixmap_getColor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkPixmap_getColor'>getColor</a>(<a href='#SkPixmap_getColor'>int</a> <a href='#SkPixmap_getColor'>x</a>, <a href='#SkPixmap_getColor'>int</a> <a href='#SkPixmap_getColor'>y</a>) <a href='#SkPixmap_getColor'>const</a>
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkPixmap_getColor_x'>x</a>, <a href='#SkPixmap_getColor_y'>y</a>) <a href='#SkPixmap_getColor_y'>as</a> <a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='SkColor_Reference#Color'>color</a>.
Returns black with <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>if</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>.

Input is not validated: out of bounds values of <a href='#SkPixmap_getColor_x'>x</a> <a href='#SkPixmap_getColor_x'>or</a> <a href='#SkPixmap_getColor_y'>y</a> <a href='#SkPixmap_getColor_y'>trigger</a> <a href='#SkPixmap_getColor_y'>an</a> <a href='#SkPixmap_getColor_y'>assert()</a> <a href='#SkPixmap_getColor_y'>if</a>
built with SK_DEBUG defined; and returns undefined values or may crash if
SK_RELEASE is defined. Fails if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>or</a>
<a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>nullptr</a>.

<a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>in</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>is</a> <a href='SkImageInfo_Reference#SkImageInfo'>ignored</a>. <a href='SkImageInfo_Reference#SkImageInfo'>Some</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>precision</a> <a href='SkColor_Reference#Color'>may</a> <a href='SkColor_Reference#Color'>be</a> <a href='SkColor_Reference#Color'>lost</a> <a href='SkColor_Reference#Color'>in</a> <a href='SkColor_Reference#Color'>the</a>
conversion to <a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='SkColor_Reference#Color'>color</a>; <a href='SkColor_Reference#Color'>original</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>may</a> <a href='undocumented#Data'>have</a> <a href='undocumented#Data'>additional</a>
precision.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_getColor_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkPixmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_getColor_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkPixmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

<a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>converted</a> <a href='undocumented#Pixel'>to</a> <a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='SkColor_Reference#Color'>color</a>

### Example

<div><fiddle-embed name="94ad244056dc80ecd87daae004266334">

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

<a href='#SkPixmap_getAlphaf'>getAlphaf</a> <a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_readPixels'>readPixels</a>

<a name='SkPixmap_getAlphaf'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
float <a href='#SkPixmap_getAlphaf'>getAlphaf</a>(<a href='#SkPixmap_getAlphaf'>int</a> <a href='#SkPixmap_getAlphaf'>x</a>, <a href='#SkPixmap_getAlphaf'>int</a> <a href='#SkPixmap_getAlphaf'>y</a>) <a href='#SkPixmap_getAlphaf'>const</a>
</pre>

Looks up the <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkPixmap_getAlphaf_x'>x</a>,<a href='#SkPixmap_getAlphaf_y'>y</a>) <a href='#SkPixmap_getAlphaf_y'>and</a> <a href='#SkPixmap_getAlphaf_y'>return</a> <a href='#SkPixmap_getAlphaf_y'>its</a> <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>component</a>, <a href='SkColor_Reference#Alpha'>normalized</a> <a href='SkColor_Reference#Alpha'>to</a> [0..1].
<a href='SkColor_Reference#Alpha'>This</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>roughly</a> <a href='SkColor_Reference#Alpha'>equivalent</a> <a href='SkColor_Reference#Alpha'>to</a> <code>SkGetColorA(<a href='#SkPixmap_getColor'>getColor</a>())</code>, but can be more efficient
(and more precise if the pixels store more than 8 bits per component).

### Parameters

<table>  <tr>    <td><a name='SkPixmap_getAlphaf_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkPixmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_getAlphaf_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkPixmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

<a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkColor_Reference#Alpha'>converted</a> <a href='SkColor_Reference#Alpha'>to</a> <a href='SkColor_Reference#Alpha'>normalized</a> <a href='SkColor_Reference#Alpha'>float</a>

### See Also

<a href='#SkPixmap_getColor'>getColor</a>

<a name='Readable_Address'></a>

<a name='SkPixmap_addr_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const void* <a href='#SkPixmap_addr'>addr</a>(<a href='#SkPixmap_addr'>int</a> <a href='#SkPixmap_addr'>x</a>, <a href='#SkPixmap_addr'>int</a> <a href='#SkPixmap_addr'>y</a>) <a href='#SkPixmap_addr'>const</a>
</pre>

Returns readable <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkPixmap_addr_2_x'>x</a>, <a href='#SkPixmap_addr_2_y'>y</a>). <a href='#SkPixmap_addr_2_y'>Returns</a> <a href='#SkPixmap_addr_2_y'>nullptr</a> <a href='#SkPixmap_addr_2_y'>if</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a> <a href='undocumented#SkPixelRef'>is</a> <a href='undocumented#SkPixelRef'>nullptr</a>.

Input is not validated: out of bounds values of <a href='#SkPixmap_addr_2_x'>x</a> <a href='#SkPixmap_addr_2_x'>or</a> <a href='#SkPixmap_addr_2_y'>y</a> <a href='#SkPixmap_addr_2_y'>trigger</a> <a href='#SkPixmap_addr_2_y'>an</a> <a href='#SkPixmap_addr_2_y'>assert()</a> <a href='#SkPixmap_addr_2_y'>if</a>
built with SK_DEBUG defined. Returns nullptr if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>.

Performs a lookup of <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Size'>size</a>; <a href='undocumented#Size'>for</a> <a href='undocumented#Size'>better</a> <a href='undocumented#Size'>performance</a>, <a href='undocumented#Size'>call</a>
one of: <a href='#SkPixmap_addr8'>addr8</a>, <a href='#SkPixmap_addr16'>addr16</a>, <a href='#SkPixmap_addr32'>addr32</a>, <a href='#SkPixmap_addr64'>addr64</a>, <a href='#SkPixmap_addr64'>or</a> <a href='#SkPixmap_addrF16'>addrF16</a>().

### Parameters

<table>  <tr>    <td><a name='SkPixmap_addr_2_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkPixmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_addr_2_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkPixmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

readable generic pointer to <a href='undocumented#Pixel'>pixel</a>

### Example

<div><fiddle-embed name="6e6e29e860eafed77308c973400cc84d">

#### Example Output

~~~~
pixmap.addr(1, 2) == &storage[1 + 2 * w]
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr8'>addr8</a> <a href='#SkPixmap_addr16'>addr16</a> <a href='#SkPixmap_addr32'>addr32</a> <a href='#SkPixmap_addr64'>addr64</a> <a href='#SkPixmap_addrF16'>addrF16</a> <a href='#SkPixmap_getColor'>getColor</a> <a href='#SkPixmap_writable_addr'>writable_addr</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_getAddr'>getAddr</a>

<a name='SkPixmap_addr8'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const uint8_t* <a href='#SkPixmap_addr8'>addr8</a>() <a href='#SkPixmap_addr8'>const</a>
</pre>

Returns readable base <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a>. <a href='undocumented#Pixel'>Result</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>addressable</a> <a href='undocumented#Pixel'>as</a> <a href='undocumented#Pixel'>unsigned</a> 8-<a href='undocumented#Pixel'>bit</a> <a href='undocumented#Pixel'>bytes</a>.
Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>not</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>or</a>
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kGray_8_SkColorType'>and</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>is</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>built</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>with</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>SK_DEBUG</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>defined</a>.

One byte corresponds to one <a href='undocumented#Pixel'>pixel</a>.

### Return Value

readable unsigned 8-bit pointer to pixels

### Example

<div><fiddle-embed name="9adda80b2dd1b08ec5ccf66da7c8bd91">

#### Example Output

~~~~
pixmap.addr8() == storage
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_addr16'>addr16</a> <a href='#SkPixmap_addr32'>addr32</a> <a href='#SkPixmap_addr64'>addr64</a> <a href='#SkPixmap_addrF16'>addrF16</a> <a href='#SkPixmap_getColor'>getColor</a> <a href='#SkPixmap_writable_addr'>writable_addr</a> <a href='#SkPixmap_writable_addr8'>writable_addr8</a>

<a name='SkPixmap_addr16'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const uint16_t* <a href='#SkPixmap_addr16'>addr16</a>() <a href='#SkPixmap_addr16'>const</a>
</pre>

Returns readable base <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a>. <a href='undocumented#Pixel'>Result</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>addressable</a> <a href='undocumented#Pixel'>as</a> <a href='undocumented#Pixel'>unsigned</a> 16-<a href='undocumented#Pixel'>bit</a> <a href='undocumented#Pixel'>words</a>.
Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>not</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>or</a>
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>and</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>is</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>built</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>with</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>SK_DEBUG</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>defined</a>.

One word corresponds to one <a href='undocumented#Pixel'>pixel</a>.

### Return Value

readable unsigned 16-bit pointer to pixels

### Example

<div><fiddle-embed name="9b16012d265c954c6de13f3fc960da52">

#### Example Output

~~~~
pixmap.addr16() == storage
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_addr8'>addr8</a> <a href='#SkPixmap_addr32'>addr32</a> <a href='#SkPixmap_addr64'>addr64</a> <a href='#SkPixmap_addrF16'>addrF16</a> <a href='#SkPixmap_getColor'>getColor</a> <a href='#SkPixmap_writable_addr'>writable_addr</a> <a href='#SkPixmap_writable_addr16'>writable_addr16</a>

<a name='SkPixmap_addr32'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const uint32_t* <a href='#SkPixmap_addr32'>addr32</a>() <a href='#SkPixmap_addr32'>const</a>
</pre>

Returns readable base <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a>. <a href='undocumented#Pixel'>Result</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>addressable</a> <a href='undocumented#Pixel'>as</a> <a href='undocumented#Pixel'>unsigned</a> 32-<a href='undocumented#Pixel'>bit</a> <a href='undocumented#Pixel'>words</a>.
Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>not</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>or</a>
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>and</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>is</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>built</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>with</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>SK_DEBUG</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>defined</a>.

One word corresponds to one <a href='undocumented#Pixel'>pixel</a>.

### Return Value

readable unsigned 32-bit pointer to pixels

### Example

<div><fiddle-embed name="6b90c7ae9f254fe4ea9ef638f893a3e6">

#### Example Output

~~~~
pixmap.addr32() == storage
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_addr8'>addr8</a> <a href='#SkPixmap_addr16'>addr16</a> <a href='#SkPixmap_addr64'>addr64</a> <a href='#SkPixmap_addrF16'>addrF16</a> <a href='#SkPixmap_getColor'>getColor</a> <a href='#SkPixmap_writable_addr'>writable_addr</a> <a href='#SkPixmap_writable_addr32'>writable_addr32</a>

<a name='SkPixmap_addr64'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const uint64_t* <a href='#SkPixmap_addr64'>addr64</a>() <a href='#SkPixmap_addr64'>const</a>
</pre>

Returns readable base <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a>. <a href='undocumented#Pixel'>Result</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>addressable</a> <a href='undocumented#Pixel'>as</a> <a href='undocumented#Pixel'>unsigned</a> 64-<a href='undocumented#Pixel'>bit</a> <a href='undocumented#Pixel'>words</a>.
Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>not</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>and</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>is</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>built</a>
with SK_DEBUG defined.

One word corresponds to one <a href='undocumented#Pixel'>pixel</a>.

### Return Value

readable unsigned 64-bit pointer to pixels

### Example

<div><fiddle-embed name="0d17085a4698a8a2e2235fad9041b4b4">

#### Example Output

~~~~
pixmap.addr64() == storage
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_addr8'>addr8</a> <a href='#SkPixmap_addr16'>addr16</a> <a href='#SkPixmap_addr32'>addr32</a> <a href='#SkPixmap_addrF16'>addrF16</a> <a href='#SkPixmap_getColor'>getColor</a> <a href='#SkPixmap_writable_addr'>writable_addr</a> <a href='#SkPixmap_writable_addr64'>writable_addr64</a>

<a name='SkPixmap_addrF16'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const uint16_t* <a href='#SkPixmap_addrF16'>addrF16</a>() <a href='#SkPixmap_addrF16'>const</a>
</pre>

Returns readable base <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a>. <a href='undocumented#Pixel'>Result</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>addressable</a> <a href='undocumented#Pixel'>as</a> <a href='undocumented#Pixel'>unsigned</a> 16-<a href='undocumented#Pixel'>bit</a> <a href='undocumented#Pixel'>words</a>.
Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>not</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>and</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>is</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>built</a>
with SK_DEBUG defined.

Each word represents one <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>component</a> <a href='SkColor_Reference#Color'>encoded</a> <a href='SkColor_Reference#Color'>as</a> <a href='SkColor_Reference#Color'>a</a> <a href='SkColor_Reference#Color'>half</a> <a href='SkColor_Reference#Color'>float</a>.
Four words correspond to one <a href='undocumented#Pixel'>pixel</a>.

### Return Value

readable unsigned 16-bit pointer to first component of pixels

### Example

<div><fiddle-embed name="54e8525a592f05623c33b375aebc90c1">

#### Example Output

~~~~
pixmap.addrF16() == storage
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_addr8'>addr8</a> <a href='#SkPixmap_addr16'>addr16</a> <a href='#SkPixmap_addr32'>addr32</a> <a href='#SkPixmap_addr64'>addr64</a> <a href='#SkPixmap_getColor'>getColor</a> <a href='#SkPixmap_writable_addr'>writable_addr</a> <a href='#SkPixmap_writable_addrF16'>writable_addrF16</a>

<a name='SkPixmap_addr8_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const uint8_t* <a href='#SkPixmap_addr8'>addr8</a>(<a href='#SkPixmap_addr8'>int</a> <a href='#SkPixmap_addr8'>x</a>, <a href='#SkPixmap_addr8'>int</a> <a href='#SkPixmap_addr8'>y</a>) <a href='#SkPixmap_addr8'>const</a>
</pre>

Returns readable <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkPixmap_addr8_2_x'>x</a>, <a href='#SkPixmap_addr8_2_y'>y</a>).

Input is not validated: out of bounds values of <a href='#SkPixmap_addr8_2_x'>x</a> <a href='#SkPixmap_addr8_2_x'>or</a> <a href='#SkPixmap_addr8_2_y'>y</a> <a href='#SkPixmap_addr8_2_y'>trigger</a> <a href='#SkPixmap_addr8_2_y'>an</a> <a href='#SkPixmap_addr8_2_y'>assert()</a> <a href='#SkPixmap_addr8_2_y'>if</a>
built with SK_DEBUG defined.

Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>not</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>or</a>
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kGray_8_SkColorType'>and</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>is</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>built</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>with</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>SK_DEBUG</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>defined</a>.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_addr8_2_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkPixmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_addr8_2_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkPixmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

readable unsigned 8-bit pointer to <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkPixmap_addr8_2_x'>x</a>, <a href='#SkPixmap_addr8_2_y'>y</a>)

### Example

<div><fiddle-embed name="5b986272268ef2c52045c1856f8b6107">

#### Example Output

~~~~
pixmap.addr8(1, 2) == &storage[1 + 2 * w]
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_addr16'>addr16</a> <a href='#SkPixmap_addr32'>addr32</a> <a href='#SkPixmap_addr64'>addr64</a> <a href='#SkPixmap_addrF16'>addrF16</a> <a href='#SkPixmap_getColor'>getColor</a> <a href='#SkPixmap_writable_addr'>writable_addr</a> <a href='#SkPixmap_writable_addr8'>writable_addr8</a>

<a name='SkPixmap_addr16_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const uint16_t* <a href='#SkPixmap_addr16'>addr16</a>(<a href='#SkPixmap_addr16'>int</a> <a href='#SkPixmap_addr16'>x</a>, <a href='#SkPixmap_addr16'>int</a> <a href='#SkPixmap_addr16'>y</a>) <a href='#SkPixmap_addr16'>const</a>
</pre>

Returns readable <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkPixmap_addr16_2_x'>x</a>, <a href='#SkPixmap_addr16_2_y'>y</a>).

Input is not validated: out of bounds values of <a href='#SkPixmap_addr16_2_x'>x</a> <a href='#SkPixmap_addr16_2_x'>or</a> <a href='#SkPixmap_addr16_2_y'>y</a> <a href='#SkPixmap_addr16_2_y'>trigger</a> <a href='#SkPixmap_addr16_2_y'>an</a> <a href='#SkPixmap_addr16_2_y'>assert()</a> <a href='#SkPixmap_addr16_2_y'>if</a>
built with SK_DEBUG defined.

Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>not</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>or</a>
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>and</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>is</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>built</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>with</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>SK_DEBUG</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>defined</a>.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_addr16_2_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkPixmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_addr16_2_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkPixmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

readable unsigned 16-bit pointer to <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkPixmap_addr16_2_x'>x</a>, <a href='#SkPixmap_addr16_2_y'>y</a>)

### Example

<div><fiddle-embed name="2c0c88a546d4ef093ab63ff72dac00b9">

#### Example Output

~~~~
pixmap.addr16(1, 2) == &storage[1 + 2 * w]
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_addr8'>addr8</a> <a href='#SkPixmap_addr32'>addr32</a> <a href='#SkPixmap_addr64'>addr64</a> <a href='#SkPixmap_addrF16'>addrF16</a> <a href='#SkPixmap_getColor'>getColor</a> <a href='#SkPixmap_writable_addr'>writable_addr</a> <a href='#SkPixmap_writable_addr16'>writable_addr16</a>

<a name='SkPixmap_addr32_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const uint32_t* <a href='#SkPixmap_addr32'>addr32</a>(<a href='#SkPixmap_addr32'>int</a> <a href='#SkPixmap_addr32'>x</a>, <a href='#SkPixmap_addr32'>int</a> <a href='#SkPixmap_addr32'>y</a>) <a href='#SkPixmap_addr32'>const</a>
</pre>

Returns readable <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkPixmap_addr32_2_x'>x</a>, <a href='#SkPixmap_addr32_2_y'>y</a>).

Input is not validated: out of bounds values of <a href='#SkPixmap_addr32_2_x'>x</a> <a href='#SkPixmap_addr32_2_x'>or</a> <a href='#SkPixmap_addr32_2_y'>y</a> <a href='#SkPixmap_addr32_2_y'>trigger</a> <a href='#SkPixmap_addr32_2_y'>an</a> <a href='#SkPixmap_addr32_2_y'>assert()</a> <a href='#SkPixmap_addr32_2_y'>if</a>
built with SK_DEBUG defined.

Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>not</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>or</a>
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>and</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>is</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>built</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>with</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>SK_DEBUG</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>defined</a>.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_addr32_2_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkPixmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_addr32_2_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkPixmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

readable unsigned 32-bit pointer to <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkPixmap_addr32_2_x'>x</a>, <a href='#SkPixmap_addr32_2_y'>y</a>)

### Example

<div><fiddle-embed name="12f8b5ce9fb25604f33df336677f5d62">

#### Example Output

~~~~
pixmap.addr32(1, 2) == &storage[1 + 2 * w]
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_addr8'>addr8</a> <a href='#SkPixmap_addr16'>addr16</a> <a href='#SkPixmap_addr64'>addr64</a> <a href='#SkPixmap_addrF16'>addrF16</a> <a href='#SkPixmap_getColor'>getColor</a> <a href='#SkPixmap_writable_addr'>writable_addr</a> <a href='#SkPixmap_writable_addr64'>writable_addr64</a>

<a name='SkPixmap_addr64_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const uint64_t* <a href='#SkPixmap_addr64'>addr64</a>(<a href='#SkPixmap_addr64'>int</a> <a href='#SkPixmap_addr64'>x</a>, <a href='#SkPixmap_addr64'>int</a> <a href='#SkPixmap_addr64'>y</a>) <a href='#SkPixmap_addr64'>const</a>
</pre>

Returns readable <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkPixmap_addr64_2_x'>x</a>, <a href='#SkPixmap_addr64_2_y'>y</a>).

Input is not validated: out of bounds values of <a href='#SkPixmap_addr64_2_x'>x</a> <a href='#SkPixmap_addr64_2_x'>or</a> <a href='#SkPixmap_addr64_2_y'>y</a> <a href='#SkPixmap_addr64_2_y'>trigger</a> <a href='#SkPixmap_addr64_2_y'>an</a> <a href='#SkPixmap_addr64_2_y'>assert()</a> <a href='#SkPixmap_addr64_2_y'>if</a>
built with SK_DEBUG defined.

Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>not</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>and</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>is</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>built</a>
with SK_DEBUG defined.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_addr64_2_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkPixmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_addr64_2_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkPixmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

readable unsigned 64-bit pointer to <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkPixmap_addr64_2_x'>x</a>, <a href='#SkPixmap_addr64_2_y'>y</a>)

### Example

<div><fiddle-embed name="5449f65fd7673273b0b57807fd3117ff">

#### Example Output

~~~~
pixmap.addr64(1, 2) == &storage[1 + 2 * w]
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_addr8'>addr8</a> <a href='#SkPixmap_addr16'>addr16</a> <a href='#SkPixmap_addr32'>addr32</a> <a href='#SkPixmap_addrF16'>addrF16</a> <a href='#SkPixmap_getColor'>getColor</a> <a href='#SkPixmap_writable_addr'>writable_addr</a> <a href='#SkPixmap_writable_addr64'>writable_addr64</a>

<a name='SkPixmap_addrF16_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const uint16_t* <a href='#SkPixmap_addrF16'>addrF16</a>(<a href='#SkPixmap_addrF16'>int</a> <a href='#SkPixmap_addrF16'>x</a>, <a href='#SkPixmap_addrF16'>int</a> <a href='#SkPixmap_addrF16'>y</a>) <a href='#SkPixmap_addrF16'>const</a>
</pre>

Returns readable <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkPixmap_addrF16_2_x'>x</a>, <a href='#SkPixmap_addrF16_2_y'>y</a>).

Input is not validated: out of bounds values of <a href='#SkPixmap_addrF16_2_x'>x</a> <a href='#SkPixmap_addrF16_2_x'>or</a> <a href='#SkPixmap_addrF16_2_y'>y</a> <a href='#SkPixmap_addrF16_2_y'>trigger</a> <a href='#SkPixmap_addrF16_2_y'>an</a> <a href='#SkPixmap_addrF16_2_y'>assert()</a> <a href='#SkPixmap_addrF16_2_y'>if</a>
built with SK_DEBUG defined.

Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>not</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>and</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>is</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>built</a>
with SK_DEBUG defined.

Each unsigned 16-bit word represents one <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>component</a> <a href='SkColor_Reference#Color'>encoded</a> <a href='SkColor_Reference#Color'>as</a> <a href='SkColor_Reference#Color'>a</a> <a href='SkColor_Reference#Color'>half</a> <a href='SkColor_Reference#Color'>float</a>.
Four words correspond to one <a href='undocumented#Pixel'>pixel</a>.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_addrF16_2_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkPixmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_addrF16_2_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkPixmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

readable unsigned 16-bit pointer to <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>component</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkPixmap_addrF16_2_x'>x</a>, <a href='#SkPixmap_addrF16_2_y'>y</a>)

### Example

<div><fiddle-embed name="f6076cad455bc80af5d06eb121d3b6f2">

#### Example Output

~~~~
pixmap.addrF16(1, 2) == &storage[1 * wordsPerPixel + 2 * rowWords]
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_addr8'>addr8</a> <a href='#SkPixmap_addr16'>addr16</a> <a href='#SkPixmap_addr32'>addr32</a> <a href='#SkPixmap_addr64'>addr64</a> <a href='#SkPixmap_getColor'>getColor</a> <a href='#SkPixmap_writable_addr'>writable_addr</a> <a href='#SkPixmap_writable_addrF16'>writable_addrF16</a>

<a name='Writable_Address'></a>

<a name='SkPixmap_writable_addr'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void* <a href='#SkPixmap_writable_addr'>writable_addr</a>() <a href='#SkPixmap_writable_addr'>const</a>
</pre>

Returns writable base <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a>.

### Return Value

writable generic base pointer to pixels

### Example

<div><fiddle-embed name="74ef460f89ed5904334d0f8883e781c4">

#### Example Output

~~~~
pixmap.writable_addr() == (void *)storage
pixmap.getColor(0, 1) == 0x00000000
pixmap.getColor(0, 0) == 0xFFFFFFFF
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_writable_addr8'>writable_addr8</a> <a href='#SkPixmap_writable_addr16'>writable_addr16</a> <a href='#SkPixmap_writable_addr32'>writable_addr32</a> <a href='#SkPixmap_writable_addr64'>writable_addr64</a> <a href='#SkPixmap_writable_addrF16'>writable_addrF16</a> <a href='#SkPixmap_addr'>addr()</a>

<a name='SkPixmap_writable_addr_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void* <a href='#SkPixmap_writable_addr'>writable_addr</a>(<a href='#SkPixmap_writable_addr'>int</a> <a href='#SkPixmap_writable_addr'>x</a>, <a href='#SkPixmap_writable_addr'>int</a> <a href='#SkPixmap_writable_addr'>y</a>) <a href='#SkPixmap_writable_addr'>const</a>
</pre>

Returns writable <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkPixmap_writable_addr_2_x'>x</a>, <a href='#SkPixmap_writable_addr_2_y'>y</a>).

Input is not validated: out of bounds values of <a href='#SkPixmap_writable_addr_2_x'>x</a> <a href='#SkPixmap_writable_addr_2_x'>or</a> <a href='#SkPixmap_writable_addr_2_y'>y</a> <a href='#SkPixmap_writable_addr_2_y'>trigger</a> <a href='#SkPixmap_writable_addr_2_y'>an</a> <a href='#SkPixmap_writable_addr_2_y'>assert()</a> <a href='#SkPixmap_writable_addr_2_y'>if</a>
built with SK_DEBUG defined. Returns zero if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_writable_addr_2_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkPixmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_writable_addr_2_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkPixmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

writable generic pointer to <a href='undocumented#Pixel'>pixel</a>

### Example

<div><fiddle-embed name="559eaca89c765bc8466ea1ba3331d4db">

#### Example Output

~~~~
pixmap.writable_addr() == (void *)storage
pixmap.getColor(0, 0) == 0x00000000
pixmap.getColor(1, 2) == 0xFFFFFFFF
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_writable_addr8'>writable_addr8</a> <a href='#SkPixmap_writable_addr16'>writable_addr16</a> <a href='#SkPixmap_writable_addr32'>writable_addr32</a> <a href='#SkPixmap_writable_addr64'>writable_addr64</a> <a href='#SkPixmap_writable_addrF16'>writable_addrF16</a> <a href='#SkPixmap_addr'>addr()</a>

<a name='SkPixmap_writable_addr8'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint8_t* <a href='#SkPixmap_writable_addr8'>writable_addr8</a>(<a href='#SkPixmap_writable_addr8'>int</a> <a href='#SkPixmap_writable_addr8'>x</a>, <a href='#SkPixmap_writable_addr8'>int</a> <a href='#SkPixmap_writable_addr8'>y</a>) <a href='#SkPixmap_writable_addr8'>const</a>
</pre>

Returns writable <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkPixmap_writable_addr8_x'>x</a>, <a href='#SkPixmap_writable_addr8_y'>y</a>). <a href='#SkPixmap_writable_addr8_y'>Result</a> <a href='#SkPixmap_writable_addr8_y'>is</a> <a href='#SkPixmap_writable_addr8_y'>addressable</a> <a href='#SkPixmap_writable_addr8_y'>as</a> <a href='#SkPixmap_writable_addr8_y'>unsigned</a>
8-bit bytes. Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>not</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>
or <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kGray_8_SkColorType'>and</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>is</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>built</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>with</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>SK_DEBUG</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>defined</a>.

One byte corresponds to one <a href='undocumented#Pixel'>pixel</a>.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_writable_addr8_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkPixmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_writable_addr8_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkPixmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

writable unsigned 8-bit pointer to pixels

### Example

<div><fiddle-embed name="809284db136748208b3efc31cd89de29"><div>Altering pixels after drawing <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkBitmap_Reference#Bitmap'>is</a> <a href='SkBitmap_Reference#Bitmap'>not</a> <a href='SkBitmap_Reference#Bitmap'>guaranteed</a> <a href='SkBitmap_Reference#Bitmap'>to</a> <a href='SkBitmap_Reference#Bitmap'>affect</a> <a href='SkBitmap_Reference#Bitmap'>subsequent</a>
<a href='SkBitmap_Reference#Bitmap'>drawing</a> <a href='SkBitmap_Reference#Bitmap'>on</a> <a href='SkBitmap_Reference#Bitmap'>all</a> <a href='SkBitmap_Reference#Bitmap'>platforms</a>. <a href='SkBitmap_Reference#Bitmap'>Adding</a> <a href='SkBitmap_Reference#Bitmap'>a</a> <a href='SkBitmap_Reference#Bitmap'>second</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_installPixels'>installPixels</a> <a href='#SkBitmap_installPixels'>after</a> <a href='#SkBitmap_installPixels'>editing</a>
<a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>memory</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>safer</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkPixmap_writable_addr'>writable_addr</a> <a href='#SkPixmap_writable_addr16'>writable_addr16</a> <a href='#SkPixmap_writable_addr32'>writable_addr32</a> <a href='#SkPixmap_writable_addr64'>writable_addr64</a> <a href='#SkPixmap_writable_addrF16'>writable_addrF16</a> <a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_addr8'>addr8</a>

<a name='SkPixmap_writable_addr16'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint16_t* <a href='#SkPixmap_writable_addr16'>writable_addr16</a>(<a href='#SkPixmap_writable_addr16'>int</a> <a href='#SkPixmap_writable_addr16'>x</a>, <a href='#SkPixmap_writable_addr16'>int</a> <a href='#SkPixmap_writable_addr16'>y</a>) <a href='#SkPixmap_writable_addr16'>const</a>
</pre>

Returns <a href='#SkPixmap_writable_addr'>writable_addr</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkPixmap_writable_addr16_x'>x</a>, <a href='#SkPixmap_writable_addr16_y'>y</a>). <a href='#SkPixmap_writable_addr16_y'>Result</a> <a href='#SkPixmap_writable_addr16_y'>is</a> <a href='#SkPixmap_writable_addr16_y'>addressable</a> <a href='#SkPixmap_writable_addr16_y'>as</a> <a href='#SkPixmap_writable_addr16_y'>unsigned</a>
16-bit words. Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>not</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>
or <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>and</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>is</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>built</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>with</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>SK_DEBUG</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>defined</a>.

One word corresponds to one <a href='undocumented#Pixel'>pixel</a>.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_writable_addr16_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkPixmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_writable_addr16_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkPixmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

writable unsigned 16-bit pointer to <a href='undocumented#Pixel'>pixel</a>

### Example

<div><fiddle-embed name="6da54774f6432b46b47ea9013c15f280"><div>Draw a five by five <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, <a href='SkBitmap_Reference#Bitmap'>and</a> <a href='SkBitmap_Reference#Bitmap'>draw</a> <a href='SkBitmap_Reference#Bitmap'>it</a> <a href='SkBitmap_Reference#Bitmap'>again</a> <a href='SkBitmap_Reference#Bitmap'>with</a> <a href='SkBitmap_Reference#Bitmap'>a</a> <a href='SkBitmap_Reference#Bitmap'>center</a> <a href='SkBitmap_Reference#Bitmap'>black</a> <a href='undocumented#Pixel'>pixel</a>.
<a href='undocumented#Pixel'>The</a> <a href='undocumented#Pixel'>low</a> <a href='undocumented#Pixel'>nibble</a> <a href='undocumented#Pixel'>of</a> <a href='undocumented#Pixel'>the</a> 16-<a href='undocumented#Pixel'>bit</a> <a href='undocumented#Pixel'>word</a> <a href='undocumented#Pixel'>is</a> <a href='SkColor_Reference#Alpha'>Alpha</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkPixmap_writable_addr'>writable_addr</a> <a href='#SkPixmap_writable_addr8'>writable_addr8</a> <a href='#SkPixmap_writable_addr32'>writable_addr32</a> <a href='#SkPixmap_writable_addr64'>writable_addr64</a> <a href='#SkPixmap_writable_addrF16'>writable_addrF16</a> <a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_addr16'>addr16</a>

<a name='SkPixmap_writable_addr32'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t* <a href='#SkPixmap_writable_addr32'>writable_addr32</a>(<a href='#SkPixmap_writable_addr32'>int</a> <a href='#SkPixmap_writable_addr32'>x</a>, <a href='#SkPixmap_writable_addr32'>int</a> <a href='#SkPixmap_writable_addr32'>y</a>) <a href='#SkPixmap_writable_addr32'>const</a>
</pre>

Returns writable <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkPixmap_writable_addr32_x'>x</a>, <a href='#SkPixmap_writable_addr32_y'>y</a>). <a href='#SkPixmap_writable_addr32_y'>Result</a> <a href='#SkPixmap_writable_addr32_y'>is</a> <a href='#SkPixmap_writable_addr32_y'>addressable</a> <a href='#SkPixmap_writable_addr32_y'>as</a> <a href='#SkPixmap_writable_addr32_y'>unsigned</a>
32-bit words. Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>not</a>
<a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>or</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>and</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>is</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>built</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>with</a> <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>SK_DEBUG</a>
defined.

One word corresponds to one <a href='undocumented#Pixel'>pixel</a>.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_writable_addr32_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkPixmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_writable_addr32_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkPixmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

writable unsigned 32-bit pointer to <a href='undocumented#Pixel'>pixel</a>

### Example

<div><fiddle-embed name="086866243bf9e4c14c3b215a2aa69ad9"></fiddle-embed></div>

### See Also

<a href='#SkPixmap_writable_addr'>writable_addr</a> <a href='#SkPixmap_writable_addr8'>writable_addr8</a> <a href='#SkPixmap_writable_addr16'>writable_addr16</a> <a href='#SkPixmap_writable_addr64'>writable_addr64</a> <a href='#SkPixmap_writable_addrF16'>writable_addrF16</a> <a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_addr32'>addr32</a>

<a name='SkPixmap_writable_addr64'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint64_t* <a href='#SkPixmap_writable_addr64'>writable_addr64</a>(<a href='#SkPixmap_writable_addr64'>int</a> <a href='#SkPixmap_writable_addr64'>x</a>, <a href='#SkPixmap_writable_addr64'>int</a> <a href='#SkPixmap_writable_addr64'>y</a>) <a href='#SkPixmap_writable_addr64'>const</a>
</pre>

Returns writable <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkPixmap_writable_addr64_x'>x</a>, <a href='#SkPixmap_writable_addr64_y'>y</a>). <a href='#SkPixmap_writable_addr64_y'>Result</a> <a href='#SkPixmap_writable_addr64_y'>is</a> <a href='#SkPixmap_writable_addr64_y'>addressable</a> <a href='#SkPixmap_writable_addr64_y'>as</a> <a href='#SkPixmap_writable_addr64_y'>unsigned</a>
64-bit words. Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>not</a>
<a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>and</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>is</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>built</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>with</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>SK_DEBUG</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>defined</a>.

One word corresponds to one <a href='undocumented#Pixel'>pixel</a>.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_writable_addr64_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkPixmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_writable_addr64_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkPixmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

writable unsigned 64-bit pointer to <a href='undocumented#Pixel'>pixel</a>

### Example

<div><fiddle-embed name="de14d8d30e4a7b6462103d0e0dd96b0b"></fiddle-embed></div>

### See Also

<a href='#SkPixmap_writable_addr'>writable_addr</a> <a href='#SkPixmap_writable_addr8'>writable_addr8</a> <a href='#SkPixmap_writable_addr16'>writable_addr16</a> <a href='#SkPixmap_writable_addr32'>writable_addr32</a> <a href='#SkPixmap_writable_addrF16'>writable_addrF16</a> <a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_addr64'>addr64</a>

<a name='SkPixmap_writable_addrF16'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint16_t* <a href='#SkPixmap_writable_addrF16'>writable_addrF16</a>(<a href='#SkPixmap_writable_addrF16'>int</a> <a href='#SkPixmap_writable_addrF16'>x</a>, <a href='#SkPixmap_writable_addrF16'>int</a> <a href='#SkPixmap_writable_addrF16'>y</a>) <a href='#SkPixmap_writable_addrF16'>const</a>
</pre>

Returns writable <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>at</a> (<a href='#SkPixmap_writable_addrF16_x'>x</a>, <a href='#SkPixmap_writable_addrF16_y'>y</a>). <a href='#SkPixmap_writable_addrF16_y'>Result</a> <a href='#SkPixmap_writable_addrF16_y'>is</a> <a href='#SkPixmap_writable_addrF16_y'>addressable</a> <a href='#SkPixmap_writable_addrF16_y'>as</a> <a href='#SkPixmap_writable_addrF16_y'>unsigned</a>
16-bit words. Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>is</a> <a href='SkImageInfo_Reference#SkColorType'>not</a>
<a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>and</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>is</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>built</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>with</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>SK_DEBUG</a> <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>defined</a>.

Each word represents one <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>component</a> <a href='SkColor_Reference#Color'>encoded</a> <a href='SkColor_Reference#Color'>as</a> <a href='SkColor_Reference#Color'>a</a> <a href='SkColor_Reference#Color'>half</a> <a href='SkColor_Reference#Color'>float</a>.
Four words correspond to one <a href='undocumented#Pixel'>pixel</a>.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_writable_addrF16_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkPixmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_writable_addrF16_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkPixmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

writable unsigned 16-bit pointer to first component of <a href='undocumented#Pixel'>pixel</a>

### Example

<div><fiddle-embed name="7822d78f5cacf5c04267cbbc6c6d0b80"><div>Left <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='SkBitmap_Reference#Bitmap'>is</a> <a href='SkBitmap_Reference#Bitmap'>drawn</a> <a href='SkBitmap_Reference#Bitmap'>with</a> <a href='SkBitmap_Reference#Bitmap'>two</a> <a href='SkBitmap_Reference#Bitmap'>pixels</a> <a href='SkBitmap_Reference#Bitmap'>defined</a> <a href='SkBitmap_Reference#Bitmap'>in</a> <a href='SkBitmap_Reference#Bitmap'>half</a> <a href='SkBitmap_Reference#Bitmap'>float</a> <a href='SkBitmap_Reference#Bitmap'>format</a>. <a href='SkBitmap_Reference#Bitmap'>Right</a> <a href='SkBitmap_Reference#Bitmap'>bitmap</a>
<a href='SkBitmap_Reference#Bitmap'>is</a> <a href='SkBitmap_Reference#Bitmap'>drawn</a> <a href='SkBitmap_Reference#Bitmap'>after</a> <a href='SkBitmap_Reference#Bitmap'>overwriting</a> <a href='SkBitmap_Reference#Bitmap'>bottom</a> <a href='SkBitmap_Reference#Bitmap'>half</a> <a href='SkBitmap_Reference#Bitmap'>float</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>with</a> <a href='SkColor_Reference#Color'>top</a> <a href='SkColor_Reference#Color'>half</a> <a href='SkColor_Reference#Color'>float</a> <a href='SkColor_Reference#Color'>color</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkPixmap_writable_addr'>writable_addr</a> <a href='#SkPixmap_writable_addr8'>writable_addr8</a> <a href='#SkPixmap_writable_addr16'>writable_addr16</a> <a href='#SkPixmap_writable_addr32'>writable_addr32</a> <a href='#SkPixmap_writable_addr64'>writable_addr64</a> <a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_addrF16'>addrF16</a>

<a name='Pixels'></a>

<a name='SkPixmap_readPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixmap_readPixels'>readPixels</a>(<a href='#SkPixmap_readPixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>dstInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>dstPixels</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>dstRowBytes</a>) <a href='SkImageInfo_Reference#SkImageInfo'>const</a>
</pre>

Copies a <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='SkRect_Reference#SkRect'>pixels</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='#SkPixmap_readPixels_dstPixels'>dstPixels</a>. <a href='#SkPixmap_readPixels_dstPixels'>Copy</a> <a href='#SkPixmap_readPixels_dstPixels'>starts</a> <a href='#SkPixmap_readPixels_dstPixels'>at</a> (0, 0), <a href='#SkPixmap_readPixels_dstPixels'>and</a> <a href='#SkPixmap_readPixels_dstPixels'>does</a> <a href='#SkPixmap_readPixels_dstPixels'>not</a>
exceed <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> (<a href='#SkPixmap_width'>width()</a>, <a href='#SkPixmap_height'>height()</a>).

<a href='#SkPixmap_readPixels_dstInfo'>dstInfo</a> <a href='#SkPixmap_readPixels_dstInfo'>specifies</a> <a href='#SkPixmap_readPixels_dstInfo'>width</a>, <a href='#SkPixmap_readPixels_dstInfo'>height</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>and</a>
<a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>of</a> <a href='undocumented#SkColorSpace'>destination</a>. <a href='#SkPixmap_readPixels_dstRowBytes'>dstRowBytes</a> <a href='#SkPixmap_readPixels_dstRowBytes'>specifics</a> <a href='#SkPixmap_readPixels_dstRowBytes'>the</a> <a href='#SkPixmap_readPixels_dstRowBytes'>gap</a> <a href='#SkPixmap_readPixels_dstRowBytes'>from</a> <a href='#SkPixmap_readPixels_dstRowBytes'>one</a> <a href='#SkPixmap_readPixels_dstRowBytes'>destination</a>
row to the next. Returns true if pixels are copied. Returns false if
<a href='#SkPixmap_readPixels_dstInfo'>dstInfo</a> <a href='#SkPixmap_readPixels_dstInfo'>address</a> <a href='#SkPixmap_readPixels_dstInfo'>equals</a> <a href='#SkPixmap_readPixels_dstInfo'>nullptr</a>, <a href='#SkPixmap_readPixels_dstInfo'>or</a> <a href='#SkPixmap_readPixels_dstRowBytes'>dstRowBytes</a> <a href='#SkPixmap_readPixels_dstRowBytes'>is</a> <a href='#SkPixmap_readPixels_dstRowBytes'>less</a> <a href='#SkPixmap_readPixels_dstRowBytes'>than</a> <a href='#SkPixmap_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>().

Pixels are copied only if <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>possible</a>. <a href='undocumented#Pixel'>If</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_colorType'>colorType</a>() <a href='#SkPixmap_colorType'>is</a>
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kGray_8_SkColorType'>or</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkPixmap_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorType'>colorType</a>() <a href='#SkImageInfo_colorType'>must</a> <a href='#SkImageInfo_colorType'>match</a>.
If <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_colorType'>colorType</a>() <a href='#SkPixmap_colorType'>is</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkPixmap_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorSpace'>colorSpace</a>() <a href='#SkImageInfo_colorSpace'>must</a> <a href='#SkImageInfo_colorSpace'>match</a>.
If <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_alphaType'>alphaType</a>() <a href='#SkPixmap_alphaType'>is</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkPixmap_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() <a href='#SkImageInfo_alphaType'>must</a>
match. If <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_colorSpace'>colorSpace</a>() <a href='#SkPixmap_colorSpace'>is</a> <a href='#SkPixmap_colorSpace'>nullptr</a>, <a href='#SkPixmap_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorSpace'>colorSpace</a>() <a href='#SkImageInfo_colorSpace'>must</a> <a href='#SkImageInfo_colorSpace'>match</a>. <a href='#SkImageInfo_colorSpace'>Returns</a>
false if <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>not</a> <a href='undocumented#Pixel'>possible</a>.

Returns false if <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_width'>width()</a> <a href='#SkPixmap_width'>or</a> <a href='#SkPixmap_height'>height()</a> <a href='#SkPixmap_height'>is</a> <a href='#SkPixmap_height'>zero</a> <a href='#SkPixmap_height'>or</a> <a href='#SkPixmap_height'>negative</a>.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_readPixels_dstInfo'><code><strong>dstInfo</strong></code></a></td>
    <td>destination width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_readPixels_dstPixels'><code><strong>dstPixels</strong></code></a></td>
    <td>destination  <a href='undocumented#Pixel_Storage'>pixel storage</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_readPixels_dstRowBytes'><code><strong>dstRowBytes</strong></code></a></td>
    <td>destination row length</td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href='#SkPixmap_readPixels_dstPixels'>dstPixels</a>

### Example

<div><fiddle-embed name="df4e355c4845350daede833b4fd21ec1"><div>Transferring the gradient from 8 bits per component to 4 bits per component
creates visible banding.
</div></fiddle-embed></div>

### See Also

<a href='#SkPixmap_erase'>erase</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_readPixels'>readPixels</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_readPixels'>readPixels</a> <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_readPixels'>readPixels</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_readPixels'>readPixels</a>

<a name='SkPixmap_readPixels_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixmap_readPixels'>readPixels</a>(<a href='#SkPixmap_readPixels'>const</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='SkImageInfo_Reference#SkImageInfo'>dstInfo</a>, <a href='SkImageInfo_Reference#SkImageInfo'>void</a>* <a href='SkImageInfo_Reference#SkImageInfo'>dstPixels</a>, <a href='SkImageInfo_Reference#SkImageInfo'>size_t</a> <a href='SkImageInfo_Reference#SkImageInfo'>dstRowBytes</a>, <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>srcX</a>, <a href='SkImageInfo_Reference#SkImageInfo'>int</a> <a href='SkImageInfo_Reference#SkImageInfo'>srcY</a>) <a href='SkImageInfo_Reference#SkImageInfo'>const</a>
</pre>

Copies a <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>pixels</a> <a href='SkRect_Reference#Rect'>to</a> <a href='#SkPixmap_readPixels_2_dstPixels'>dstPixels</a>. <a href='#SkPixmap_readPixels_2_dstPixels'>Copy</a> <a href='#SkPixmap_readPixels_2_dstPixels'>starts</a> <a href='#SkPixmap_readPixels_2_dstPixels'>at</a> (<a href='#SkPixmap_readPixels_2_srcX'>srcX</a>, <a href='#SkPixmap_readPixels_2_srcY'>srcY</a>), <a href='#SkPixmap_readPixels_2_srcY'>and</a> <a href='#SkPixmap_readPixels_2_srcY'>does</a> <a href='#SkPixmap_readPixels_2_srcY'>not</a>
<a href='#SkPixmap_readPixels_2_srcY'>exceed</a> <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> (<a href='#SkPixmap_width'>width()</a>, <a href='#SkPixmap_height'>height()</a>).

<a href='#SkPixmap_readPixels_2_dstInfo'>dstInfo</a> <a href='#SkPixmap_readPixels_2_dstInfo'>specifies</a> <a href='#SkPixmap_readPixels_2_dstInfo'>width</a>, <a href='#SkPixmap_readPixels_2_dstInfo'>height</a>, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, <a href='#Image_Info_Alpha_Type'>and</a>
<a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>of</a> <a href='#Color_Space'>destination</a>. <a href='#SkPixmap_readPixels_2_dstRowBytes'>dstRowBytes</a> <a href='#SkPixmap_readPixels_2_dstRowBytes'>specifics</a> <a href='#SkPixmap_readPixels_2_dstRowBytes'>the</a> <a href='#SkPixmap_readPixels_2_dstRowBytes'>gap</a> <a href='#SkPixmap_readPixels_2_dstRowBytes'>from</a> <a href='#SkPixmap_readPixels_2_dstRowBytes'>one</a> <a href='#SkPixmap_readPixels_2_dstRowBytes'>destination</a>
<a href='#SkPixmap_readPixels_2_dstRowBytes'>row</a> <a href='#SkPixmap_readPixels_2_dstRowBytes'>to</a> <a href='#SkPixmap_readPixels_2_dstRowBytes'>the</a> <a href='#SkPixmap_readPixels_2_dstRowBytes'>next</a>. <a href='#SkPixmap_readPixels_2_dstRowBytes'>Returns</a> <a href='#SkPixmap_readPixels_2_dstRowBytes'>true</a> <a href='#SkPixmap_readPixels_2_dstRowBytes'>if</a> <a href='#SkPixmap_readPixels_2_dstRowBytes'>pixels</a> <a href='#SkPixmap_readPixels_2_dstRowBytes'>are</a> <a href='#SkPixmap_readPixels_2_dstRowBytes'>copied</a>. <a href='#SkPixmap_readPixels_2_dstRowBytes'>Returns</a> <a href='#SkPixmap_readPixels_2_dstRowBytes'>false</a> <a href='#SkPixmap_readPixels_2_dstRowBytes'>if</a>
<a href='#SkPixmap_readPixels_2_dstInfo'>dstInfo</a> <a href='#SkPixmap_readPixels_2_dstInfo'>has</a> <a href='#SkPixmap_readPixels_2_dstInfo'>no</a> <a href='#SkPixmap_readPixels_2_dstInfo'>address</a>, <a href='#SkPixmap_readPixels_2_dstInfo'>or</a> <a href='#SkPixmap_readPixels_2_dstRowBytes'>dstRowBytes</a> <a href='#SkPixmap_readPixels_2_dstRowBytes'>is</a> <a href='#SkPixmap_readPixels_2_dstRowBytes'>less</a> <a href='#SkPixmap_readPixels_2_dstRowBytes'>than</a> <a href='#SkPixmap_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>().

<a href='#SkImageInfo_minRowBytes'>Pixels</a> <a href='#SkImageInfo_minRowBytes'>are</a> <a href='#SkImageInfo_minRowBytes'>copied</a> <a href='#SkImageInfo_minRowBytes'>only</a> <a href='#SkImageInfo_minRowBytes'>if</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>possible</a>. <a href='undocumented#Pixel'>If</a> <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_colorType'>colorType</a> <a href='#SkPixmap_colorType'>is</a>
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kGray_8_SkColorType'>or</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkPixmap_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorType'>colorType</a>() <a href='#SkImageInfo_colorType'>must</a> <a href='#SkImageInfo_colorType'>match</a>.
<a href='#SkImageInfo_colorType'>If</a> <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_colorType'>colorType</a> <a href='#SkPixmap_colorType'>is</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkPixmap_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorSpace'>colorSpace</a>() <a href='#SkImageInfo_colorSpace'>must</a> <a href='#SkImageInfo_colorSpace'>match</a>.
<a href='#SkImageInfo_colorSpace'>If</a> <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_alphaType'>alphaType</a> <a href='#SkPixmap_alphaType'>is</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkPixmap_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() <a href='#SkImageInfo_alphaType'>must</a>
<a href='#SkImageInfo_alphaType'>match</a>. <a href='#SkImageInfo_alphaType'>If</a> <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_colorSpace'>colorSpace</a> <a href='#SkPixmap_colorSpace'>is</a> <a href='#SkPixmap_colorSpace'>nullptr</a>, <a href='#SkPixmap_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorSpace'>colorSpace</a>() <a href='#SkImageInfo_colorSpace'>must</a> <a href='#SkImageInfo_colorSpace'>match</a>. <a href='#SkImageInfo_colorSpace'>Returns</a>
<a href='#SkImageInfo_colorSpace'>false</a> <a href='#SkImageInfo_colorSpace'>if</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>not</a> <a href='undocumented#Pixel'>possible</a>.

<a href='#SkPixmap_readPixels_2_srcX'>srcX</a> <a href='#SkPixmap_readPixels_2_srcX'>and</a> <a href='#SkPixmap_readPixels_2_srcY'>srcY</a> <a href='#SkPixmap_readPixels_2_srcY'>may</a> <a href='#SkPixmap_readPixels_2_srcY'>be</a> <a href='#SkPixmap_readPixels_2_srcY'>negative</a> <a href='#SkPixmap_readPixels_2_srcY'>to</a> <a href='#SkPixmap_readPixels_2_srcY'>copy</a> <a href='#SkPixmap_readPixels_2_srcY'>only</a> <a href='#SkPixmap_readPixels_2_srcY'>top</a> <a href='#SkPixmap_readPixels_2_srcY'>or</a> <a href='#SkPixmap_readPixels_2_srcY'>left</a> <a href='#SkPixmap_readPixels_2_srcY'>of</a> <a href='#SkPixmap_readPixels_2_srcY'>source</a>. <a href='#SkPixmap_readPixels_2_srcY'>Returns</a>
<a href='#SkPixmap_readPixels_2_srcY'>false</a> <a href='#SkPixmap_readPixels_2_srcY'>if</a> <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_width'>width()</a> <a href='#SkPixmap_width'>or</a> <a href='#SkPixmap_height'>height()</a> <a href='#SkPixmap_height'>is</a> <a href='#SkPixmap_height'>zero</a> <a href='#SkPixmap_height'>or</a> <a href='#SkPixmap_height'>negative</a>. <a href='#SkPixmap_height'>Returns</a> <a href='#SkPixmap_height'>false</a> <a href='#SkPixmap_height'>if</a>:

<code><a href='undocumented#abs()'>abs</a>(<a href='#SkPixmap_readPixels_2_srcX'>srcX</a>) >= <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_width'>width()</a></code>, or if <code><a href='undocumented#abs()'>abs</a>(<a href='#SkPixmap_readPixels_2_srcY'>srcY</a>) >= <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_height'>height()</a></code>.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_readPixels_2_dstInfo'><code><strong>dstInfo</strong></code></a></td>
    <td>destination width, height, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, <a href='#Color_Space'>Color_Space</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_readPixels_2_dstPixels'><code><strong>dstPixels</strong></code></a></td>
    <td>destination  <a href='undocumented#Pixel_Storage'>pixel storage</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_readPixels_2_dstRowBytes'><code><strong>dstRowBytes</strong></code></a></td>
    <td>destination row length</td>
  </tr>
  <tr>    <td><a name='SkPixmap_readPixels_2_srcX'><code><strong>srcX</strong></code></a></td>
    <td>column index whose absolute value is less than <a href='#SkPixmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_readPixels_2_srcY'><code><strong>srcY</strong></code></a></td>
    <td>row index whose absolute value is less than <a href='#SkPixmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href='#SkPixmap_readPixels_2_dstPixels'>dstPixels</a>

### Example

<div><fiddle-embed name="094ca0bd37588cc7be241bb387a3e17b"></fiddle-embed></div>

### See Also

<a href='#SkPixmap_erase'>erase</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_readPixels'>readPixels</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_readPixels'>readPixels</a> <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_readPixels'>readPixels</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_readPixels'>readPixels</a>

<a name='SkPixmap_readPixels_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixmap_readPixels'>readPixels</a>(<a href='#SkPixmap_readPixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#SkPixmap'>dst</a>, <a href='SkPixmap_Reference#SkPixmap'>int</a> <a href='SkPixmap_Reference#SkPixmap'>srcX</a>, <a href='SkPixmap_Reference#SkPixmap'>int</a> <a href='SkPixmap_Reference#SkPixmap'>srcY</a>) <a href='SkPixmap_Reference#SkPixmap'>const</a>
</pre>

Copies a <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>pixels</a> <a href='SkRect_Reference#Rect'>to</a> <a href='#SkPixmap_readPixels_3_dst'>dst</a>. <a href='#SkPixmap_readPixels_3_dst'>Copy</a> <a href='#SkPixmap_readPixels_3_dst'>starts</a> <a href='#SkPixmap_readPixels_3_dst'>at</a> (<a href='#SkPixmap_readPixels_3_srcX'>srcX</a>, <a href='#SkPixmap_readPixels_3_srcY'>srcY</a>), <a href='#SkPixmap_readPixels_3_srcY'>and</a> <a href='#SkPixmap_readPixels_3_srcY'>does</a> <a href='#SkPixmap_readPixels_3_srcY'>not</a>
<a href='#SkPixmap_readPixels_3_srcY'>exceed</a> <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> (<a href='#SkPixmap_width'>width()</a>, <a href='#SkPixmap_height'>height()</a>). <a href='#SkPixmap_readPixels_3_dst'>dst</a> <a href='#SkPixmap_readPixels_3_dst'>specifies</a> <a href='#SkPixmap_readPixels_3_dst'>width</a>, <a href='#SkPixmap_readPixels_3_dst'>height</a>, <a href='#Image_Info_Color_Type'>Color_Type</a>,
<a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, <a href='#Image_Info_Alpha_Type'>and</a> <a href='#Color_Space'>Color_Space</a> <a href='#Color_Space'>of</a> <a href='#Color_Space'>destination</a>.  <a href='#Color_Space'>Returns</a> <a href='#Color_Space'>true</a> <a href='#Color_Space'>if</a> <a href='#Color_Space'>pixels</a> <a href='#Color_Space'>are</a> <a href='#Color_Space'>copied</a>.
<a href='#Color_Space'>Returns</a> <a href='#Color_Space'>false</a> <a href='#Color_Space'>if</a> <a href='#SkPixmap_readPixels_3_dst'>dst</a>.<a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_addr'>equals</a> <a href='#SkPixmap_addr'>nullptr</a>, <a href='#SkPixmap_addr'>or</a> <a href='#SkPixmap_readPixels_3_dst'>dst</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() <a href='#SkPixmap_rowBytes'>is</a> <a href='#SkPixmap_rowBytes'>less</a> <a href='#SkPixmap_rowBytes'>than</a>
<a href='#SkPixmap_readPixels_3_dst'>dst</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>.

<a href='#SkImageInfo_minRowBytes'>Pixels</a> <a href='#SkImageInfo_minRowBytes'>are</a> <a href='#SkImageInfo_minRowBytes'>copied</a> <a href='#SkImageInfo_minRowBytes'>only</a> <a href='#SkImageInfo_minRowBytes'>if</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>possible</a>. <a href='undocumented#Pixel'>If</a> <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_colorType'>colorType</a> <a href='#SkPixmap_colorType'>is</a>
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kGray_8_SkColorType'>or</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkPixmap_readPixels_3_dst'>dst</a>.<a href='#SkPixmap_info'>info()</a>.<a href='#SkImageInfo_colorType'>colorType</a>() <a href='#SkImageInfo_colorType'>must</a> <a href='#SkImageInfo_colorType'>match</a>.
<a href='#SkImageInfo_colorType'>If</a> <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_colorType'>colorType</a> <a href='#SkPixmap_colorType'>is</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkPixmap_readPixels_3_dst'>dst</a>.<a href='#SkPixmap_info'>info()</a>.<a href='#SkImageInfo_colorSpace'>colorSpace</a>() <a href='#SkImageInfo_colorSpace'>must</a> <a href='#SkImageInfo_colorSpace'>match</a>.
<a href='#SkImageInfo_colorSpace'>If</a> <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_alphaType'>alphaType</a> <a href='#SkPixmap_alphaType'>is</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkPixmap_readPixels_3_dst'>dst</a>.<a href='#SkPixmap_info'>info()</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() <a href='#SkImageInfo_alphaType'>must</a>
<a href='#SkImageInfo_alphaType'>match</a>. <a href='#SkImageInfo_alphaType'>If</a> <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_colorSpace'>colorSpace</a> <a href='#SkPixmap_colorSpace'>is</a> <a href='#SkPixmap_colorSpace'>nullptr</a>, <a href='#SkPixmap_readPixels_3_dst'>dst</a>.<a href='#SkPixmap_info'>info()</a>.<a href='#SkImageInfo_colorSpace'>colorSpace</a>() <a href='#SkImageInfo_colorSpace'>must</a> <a href='#SkImageInfo_colorSpace'>match</a>. <a href='#SkImageInfo_colorSpace'>Returns</a>
<a href='#SkImageInfo_colorSpace'>false</a> <a href='#SkImageInfo_colorSpace'>if</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>not</a> <a href='undocumented#Pixel'>possible</a>.

<a href='#SkPixmap_readPixels_3_srcX'>srcX</a> <a href='#SkPixmap_readPixels_3_srcX'>and</a> <a href='#SkPixmap_readPixels_3_srcY'>srcY</a> <a href='#SkPixmap_readPixels_3_srcY'>may</a> <a href='#SkPixmap_readPixels_3_srcY'>be</a> <a href='#SkPixmap_readPixels_3_srcY'>negative</a> <a href='#SkPixmap_readPixels_3_srcY'>to</a> <a href='#SkPixmap_readPixels_3_srcY'>copy</a> <a href='#SkPixmap_readPixels_3_srcY'>only</a> <a href='#SkPixmap_readPixels_3_srcY'>top</a> <a href='#SkPixmap_readPixels_3_srcY'>or</a> <a href='#SkPixmap_readPixels_3_srcY'>left</a> <a href='#SkPixmap_readPixels_3_srcY'>of</a> <a href='#SkPixmap_readPixels_3_srcY'>source</a>. <a href='#SkPixmap_readPixels_3_srcY'>Returns</a>
<a href='#SkPixmap_readPixels_3_srcY'>false</a> <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_width'>width()</a> <a href='#SkPixmap_width'>or</a> <a href='#SkPixmap_height'>height()</a> <a href='#SkPixmap_height'>is</a> <a href='#SkPixmap_height'>zero</a> <a href='#SkPixmap_height'>or</a> <a href='#SkPixmap_height'>negative</a>. <a href='#SkPixmap_height'>Returns</a> <a href='#SkPixmap_height'>false</a> <a href='#SkPixmap_height'>if</a>:

<code><a href='undocumented#abs()'>abs</a>(<a href='#SkPixmap_readPixels_3_srcX'>srcX</a>) >= <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_width'>width()</a></code>, or if <code><a href='undocumented#abs()'>abs</a>(<a href='#SkPixmap_readPixels_3_srcY'>srcY</a>) >= <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_height'>height()</a></code>.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_readPixels_3_dst'><code><strong>dst</strong></code></a></td>
    <td><a href='#Image_Info'>Image_Info</a> <a href='#Image_Info'>and</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>to</a> <a href='undocumented#Pixel'>write</a> <a href='undocumented#Pixel'>to</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_readPixels_3_srcX'><code><strong>srcX</strong></code></a></td>
    <td>column index whose absolute value is less than <a href='#SkPixmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_readPixels_3_srcY'><code><strong>srcY</strong></code></a></td>
    <td>row index whose absolute value is less than <a href='#SkPixmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href='#SkPixmap_readPixels_3_dst'>dst</a>

### Example

<div><fiddle-embed name="6ec7f7b2cc163cd29f627eef6d4b061c"></fiddle-embed></div>

### See Also

<a href='#SkPixmap_erase'>erase</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_readPixels'>readPixels</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_readPixels'>readPixels</a> <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_readPixels'>readPixels</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_readPixels'>readPixels</a>

<a name='SkPixmap_readPixels_4'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixmap_readPixels'>readPixels</a>(<a href='#SkPixmap_readPixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#SkPixmap'>dst</a>) <a href='SkPixmap_Reference#SkPixmap'>const</a>
</pre>

Copies pixels inside <a href='#SkPixmap_bounds'>bounds()</a> <a href='#SkPixmap_bounds'>to</a> <a href='#SkPixmap_readPixels_4_dst'>dst</a>. <a href='#SkPixmap_readPixels_4_dst'>dst</a> <a href='#SkPixmap_readPixels_4_dst'>specifies</a> <a href='#SkPixmap_readPixels_4_dst'>width</a>, <a href='#SkPixmap_readPixels_4_dst'>height</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>,
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>and</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>of</a> <a href='undocumented#SkColorSpace'>destination</a>.  <a href='undocumented#SkColorSpace'>Returns</a> <a href='undocumented#SkColorSpace'>true</a> <a href='undocumented#SkColorSpace'>if</a> <a href='undocumented#SkColorSpace'>pixels</a> <a href='undocumented#SkColorSpace'>are</a> <a href='undocumented#SkColorSpace'>copied</a>.
Returns false if <a href='#SkPixmap_readPixels_4_dst'>dst</a> <a href='#SkPixmap_readPixels_4_dst'>address</a> <a href='#SkPixmap_readPixels_4_dst'>equals</a> <a href='#SkPixmap_readPixels_4_dst'>nullptr</a>, <a href='#SkPixmap_readPixels_4_dst'>or</a> <a href='#SkPixmap_readPixels_4_dst'>dst</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() <a href='#SkPixmap_rowBytes'>is</a> <a href='#SkPixmap_rowBytes'>less</a> <a href='#SkPixmap_rowBytes'>than</a>
<a href='#SkPixmap_readPixels_4_dst'>dst</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>.

Pixels are copied only if <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>possible</a>. <a href='undocumented#Pixel'>If</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_colorType'>colorType</a>() <a href='#SkPixmap_colorType'>is</a>
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kGray_8_SkColorType'>or</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkPixmap_readPixels_4_dst'>dst</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>must</a> <a href='SkImageInfo_Reference#SkColorType'>match</a>.
If <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_colorType'>colorType</a>() <a href='#SkPixmap_colorType'>is</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkPixmap_readPixels_4_dst'>dst</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>must</a> <a href='undocumented#SkColorSpace'>match</a>.
If <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_alphaType'>alphaType</a>() <a href='#SkPixmap_alphaType'>is</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkPixmap_readPixels_4_dst'>dst</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>must</a>
match. If <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_colorSpace'>colorSpace</a>() <a href='#SkPixmap_colorSpace'>is</a> <a href='#SkPixmap_colorSpace'>nullptr</a>, <a href='#SkPixmap_readPixels_4_dst'>dst</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>must</a> <a href='undocumented#SkColorSpace'>match</a>. <a href='undocumented#SkColorSpace'>Returns</a>
false if <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>not</a> <a href='undocumented#Pixel'>possible</a>.

Returns false if <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_width'>width()</a> <a href='#SkPixmap_width'>or</a> <a href='#SkPixmap_height'>height()</a> <a href='#SkPixmap_height'>is</a> <a href='#SkPixmap_height'>zero</a> <a href='#SkPixmap_height'>or</a> <a href='#SkPixmap_height'>negative</a>.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_readPixels_4_dst'><code><strong>dst</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>and</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>to</a> <a href='undocumented#Pixel'>write</a> <a href='undocumented#Pixel'>to</a></td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href='#SkPixmap_readPixels_4_dst'>dst</a>

### Example

<div><fiddle-embed name="e18549b5ee1039cb61b0bb38c2104fc9"></fiddle-embed></div>

### See Also

<a href='#SkPixmap_erase'>erase</a> <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_readPixels'>readPixels</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_readPixels'>readPixels</a> <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_readPixels'>readPixels</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>::<a href='#SkSurface_readPixels'>readPixels</a>

<a name='SkPixmap_scalePixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixmap_scalePixels'>scalePixels</a>(<a href='#SkPixmap_scalePixels'>const</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#SkPixmap'>dst</a>, <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> <a href='undocumented#SkFilterQuality'>filterQuality</a>) <a href='undocumented#SkFilterQuality'>const</a>
</pre>

Copies <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='SkBitmap_Reference#SkBitmap'>to</a> <a href='#SkPixmap_scalePixels_dst'>dst</a>, <a href='#SkPixmap_scalePixels_dst'>scaling</a> <a href='#SkPixmap_scalePixels_dst'>pixels</a> <a href='#SkPixmap_scalePixels_dst'>to</a> <a href='#SkPixmap_scalePixels_dst'>fit</a> <a href='#SkPixmap_scalePixels_dst'>dst</a>.<a href='#SkPixmap_width'>width()</a> <a href='#SkPixmap_width'>and</a> <a href='#SkPixmap_scalePixels_dst'>dst</a>.<a href='#SkPixmap_height'>height()</a>, <a href='#SkPixmap_height'>and</a>
converting pixels to match <a href='#SkPixmap_scalePixels_dst'>dst</a>.<a href='#SkPixmap_colorType'>colorType</a>() <a href='#SkPixmap_colorType'>and</a> <a href='#SkPixmap_scalePixels_dst'>dst</a>.<a href='#SkPixmap_alphaType'>alphaType</a>(). <a href='#SkPixmap_alphaType'>Returns</a> <a href='#SkPixmap_alphaType'>true</a> <a href='#SkPixmap_alphaType'>if</a>
pixels are copied. Returns false if <a href='#SkPixmap_scalePixels_dst'>dst</a> <a href='#SkPixmap_scalePixels_dst'>address</a> <a href='#SkPixmap_scalePixels_dst'>is</a> <a href='#SkPixmap_scalePixels_dst'>nullptr</a>, <a href='#SkPixmap_scalePixels_dst'>or</a> <a href='#SkPixmap_scalePixels_dst'>dst</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() <a href='#SkPixmap_rowBytes'>is</a>
less than <a href='#SkPixmap_scalePixels_dst'>dst</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>.

Pixels are copied only if <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>possible</a>. <a href='undocumented#Pixel'>If</a> <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_colorType'>colorType</a>() <a href='#SkPixmap_colorType'>is</a>
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kGray_8_SkColorType'>or</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkPixmap_scalePixels_dst'>dst</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#SkColorType'>must</a> <a href='SkImageInfo_Reference#SkColorType'>match</a>.
If <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_colorType'>colorType</a>() <a href='#SkPixmap_colorType'>is</a> <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkPixmap_scalePixels_dst'>dst</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>must</a> <a href='undocumented#SkColorSpace'>match</a>.
If <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_alphaType'>alphaType</a>() <a href='#SkPixmap_alphaType'>is</a> <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkPixmap_scalePixels_dst'>dst</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkAlphaType'>must</a>
match. If <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_colorSpace'>colorSpace</a>() <a href='#SkPixmap_colorSpace'>is</a> <a href='#SkPixmap_colorSpace'>nullptr</a>, <a href='#SkPixmap_scalePixels_dst'>dst</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> <a href='undocumented#SkColorSpace'>must</a> <a href='undocumented#SkColorSpace'>match</a>. <a href='undocumented#SkColorSpace'>Returns</a>
false if <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>conversion</a> <a href='undocumented#Pixel'>is</a> <a href='undocumented#Pixel'>not</a> <a href='undocumented#Pixel'>possible</a>.

Returns false if <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='#SkPixmap_width'>width()</a> <a href='#SkPixmap_width'>or</a> <a href='#SkPixmap_height'>height()</a> <a href='#SkPixmap_height'>is</a> <a href='#SkPixmap_height'>zero</a> <a href='#SkPixmap_height'>or</a> <a href='#SkPixmap_height'>negative</a>.

Scales the <a href='SkImage_Reference#Image'>image</a>, <a href='SkImage_Reference#Image'>with</a> <a href='#SkPixmap_scalePixels_filterQuality'>filterQuality</a>, <a href='#SkPixmap_scalePixels_filterQuality'>to</a> <a href='#SkPixmap_scalePixels_filterQuality'>match</a> <a href='#SkPixmap_scalePixels_dst'>dst</a>.<a href='#SkPixmap_width'>width()</a> <a href='#SkPixmap_width'>and</a> <a href='#SkPixmap_scalePixels_dst'>dst</a>.<a href='#SkPixmap_height'>height()</a>.
<a href='#SkPixmap_scalePixels_filterQuality'>filterQuality</a> <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a> <a href='undocumented#kNone_SkFilterQuality'>is</a> <a href='undocumented#kNone_SkFilterQuality'>fastest</a>, <a href='undocumented#kNone_SkFilterQuality'>typically</a> <a href='undocumented#kNone_SkFilterQuality'>implemented</a> <a href='undocumented#kNone_SkFilterQuality'>with</a>
<a href='undocumented#Nearest_Neighbor'>nearest neighbor filter</a>. <a href='undocumented#kLow_SkFilterQuality'>kLow_SkFilterQuality</a> <a href='undocumented#kLow_SkFilterQuality'>is</a> <a href='undocumented#kLow_SkFilterQuality'>typically</a> <a href='undocumented#kLow_SkFilterQuality'>implemented</a> <a href='undocumented#kLow_SkFilterQuality'>with</a>
<a href='undocumented#Bilerp'>bilerp filter</a>. <a href='undocumented#kMedium_SkFilterQuality'>kMedium_SkFilterQuality</a> <a href='undocumented#kMedium_SkFilterQuality'>is</a> <a href='undocumented#kMedium_SkFilterQuality'>typically</a> <a href='undocumented#kMedium_SkFilterQuality'>implemented</a> <a href='undocumented#kMedium_SkFilterQuality'>with</a>
<a href='undocumented#Bilerp'>bilerp filter</a>, and  <a href='undocumented#MipMap'>mip-map filter</a> when <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>is</a> <a href='undocumented#Size'>reduced</a>.
<a href='undocumented#kHigh_SkFilterQuality'>kHigh_SkFilterQuality</a> <a href='undocumented#kHigh_SkFilterQuality'>is</a> <a href='undocumented#kHigh_SkFilterQuality'>slowest</a>, <a href='undocumented#kHigh_SkFilterQuality'>typically</a> <a href='undocumented#kHigh_SkFilterQuality'>implemented</a> <a href='undocumented#kHigh_SkFilterQuality'>with</a>  <a href='undocumented#BiCubic'>bicubic filter</a>.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_scalePixels_dst'><code><strong>dst</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkImageInfo'>and</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>address</a> <a href='undocumented#Pixel'>to</a> <a href='undocumented#Pixel'>write</a> <a href='undocumented#Pixel'>to</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_scalePixels_filterQuality'><code><strong>filterQuality</strong></code></a></td>
    <td>one of: <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a>, <a href='undocumented#kLow_SkFilterQuality'>kLow_SkFilterQuality</a>,</td>
  </tr>
</table>

<a href='undocumented#kMedium_SkFilterQuality'>kMedium_SkFilterQuality</a>, <a href='undocumented#kHigh_SkFilterQuality'>kHigh_SkFilterQuality</a>

### Return Value

true if pixels are scaled to fit <a href='#SkPixmap_scalePixels_dst'>dst</a>

### Example

<div><fiddle-embed name="8e3c8a9c1d0d2e9b8bf66e24d274f792"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawBitmap'>drawBitmap</a> <a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_scalePixels'>scalePixels</a>

<a name='SkPixmap_erase'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixmap_erase'>erase</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>subset</a>) <a href='SkIRect_Reference#SkIRect'>const</a>
</pre>

Writes <a href='#SkPixmap_erase_color'>color</a> <a href='#SkPixmap_erase_color'>to</a> <a href='#SkPixmap_erase_color'>pixels</a> <a href='#SkPixmap_erase_color'>bounded</a> <a href='#SkPixmap_erase_color'>by</a> <a href='#SkPixmap_erase_subset'>subset</a>; <a href='#SkPixmap_erase_subset'>returns</a> <a href='#SkPixmap_erase_subset'>true</a> <a href='#SkPixmap_erase_subset'>on</a> <a href='#SkPixmap_erase_subset'>success</a>.
Returns false if <a href='#SkPixmap_colorType'>colorType</a>() <a href='#SkPixmap_colorType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kUnknown_SkColorType'>or</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>if</a> <a href='#SkPixmap_erase_subset'>subset</a> <a href='#SkPixmap_erase_subset'>does</a>
not intersect <a href='#SkPixmap_bounds'>bounds()</a>.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_erase_color'><code><strong>color</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='#SkPixmap_erase_color'>color</a> <a href='#SkPixmap_erase_color'>to</a> <a href='#SkPixmap_erase_color'>write</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_erase_subset'><code><strong>subset</strong></code></a></td>
    <td>bounding integer <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='SkRect_Reference#SkRect'>written</a> <a href='SkRect_Reference#SkRect'>pixels</a></td>
  </tr>
</table>

### Return Value

true if pixels are changed

### Example

<div><fiddle-embed name="a0cdbafed4786788cc90681e7b294234"></fiddle-embed></div>

### See Also

<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_erase'>erase</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_clear'>clear</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawColor'>drawColor</a>

<a name='SkPixmap_erase_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixmap_erase'>erase</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#Color'>color</a>) <a href='SkColor_Reference#Color'>const</a>
</pre>

Writes <a href='#SkPixmap_erase_2_color'>color</a> <a href='#SkPixmap_erase_2_color'>to</a> <a href='#SkPixmap_erase_2_color'>pixels</a> <a href='#SkPixmap_erase_2_color'>inside</a> <a href='#SkPixmap_bounds'>bounds()</a>; <a href='#SkPixmap_bounds'>returns</a> <a href='#SkPixmap_bounds'>true</a> <a href='#SkPixmap_bounds'>on</a> <a href='#SkPixmap_bounds'>success</a>.
Returns false if <a href='#SkPixmap_colorType'>colorType</a>() <a href='#SkPixmap_colorType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kUnknown_SkColorType'>or</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>if</a> <a href='#SkPixmap_bounds'>bounds()</a>
is empty.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_erase_2_color'><code><strong>color</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='#SkPixmap_erase_2_color'>color</a> <a href='#SkPixmap_erase_2_color'>to</a> <a href='#SkPixmap_erase_2_color'>write</a></td>
  </tr>
</table>

### Return Value

true if pixels are changed

### Example

<div><fiddle-embed name="838202e0d49cad2eb3eeb495834f6d63"></fiddle-embed></div>

### See Also

<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_erase'>erase</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_clear'>clear</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawColor'>drawColor</a>

<a name='SkPixmap_erase_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixmap_erase'>erase</a>(<a href='#SkPixmap_erase'>const</a> <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a>& <a href='SkColor_Reference#Color'>color</a>, <a href='SkColor_Reference#Color'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* <a href='SkIRect_Reference#SkIRect'>subset</a> = <a href='SkIRect_Reference#SkIRect'>nullptr</a>) <a href='SkIRect_Reference#SkIRect'>const</a>
</pre>

Writes <a href='#SkPixmap_erase_3_color'>color</a> <a href='#SkPixmap_erase_3_color'>to</a> <a href='#SkPixmap_erase_3_color'>pixels</a> <a href='#SkPixmap_erase_3_color'>bounded</a> <a href='#SkPixmap_erase_3_color'>by</a> <a href='#SkPixmap_erase_3_subset'>subset</a>; <a href='#SkPixmap_erase_3_subset'>returns</a> <a href='#SkPixmap_erase_3_subset'>true</a> <a href='#SkPixmap_erase_3_subset'>on</a> <a href='#SkPixmap_erase_3_subset'>success</a>.
if <a href='#SkPixmap_erase_3_subset'>subset</a> <a href='#SkPixmap_erase_3_subset'>is</a> <a href='#SkPixmap_erase_3_subset'>nullptr</a>, <a href='#SkPixmap_erase_3_subset'>writes</a> <a href='#SkPixmap_erase_3_subset'>colors</a> <a href='#SkPixmap_erase_3_subset'>pixels</a> <a href='#SkPixmap_erase_3_subset'>inside</a> <a href='#SkPixmap_bounds'>bounds()</a>. <a href='#SkPixmap_bounds'>Returns</a> <a href='#SkPixmap_bounds'>false</a> <a href='#SkPixmap_bounds'>if</a>
<a href='#SkPixmap_colorType'>colorType</a>() <a href='#SkPixmap_colorType'>is</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kUnknown_SkColorType'>if</a> <a href='#SkPixmap_erase_3_subset'>subset</a> <a href='#SkPixmap_erase_3_subset'>is</a> <a href='#SkPixmap_erase_3_subset'>not</a> <a href='#SkPixmap_erase_3_subset'>nullptr</a> <a href='#SkPixmap_erase_3_subset'>and</a> <a href='#SkPixmap_erase_3_subset'>does</a>
not intersect <a href='#SkPixmap_bounds'>bounds()</a>, <a href='#SkPixmap_bounds'>or</a> <a href='#SkPixmap_bounds'>if</a> <a href='#SkPixmap_erase_3_subset'>subset</a> <a href='#SkPixmap_erase_3_subset'>is</a> <a href='#SkPixmap_erase_3_subset'>nullptr</a> <a href='#SkPixmap_erase_3_subset'>and</a> <a href='#SkPixmap_bounds'>bounds()</a> <a href='#SkPixmap_bounds'>is</a> <a href='#SkPixmap_bounds'>empty</a>.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_erase_3_color'><code><strong>color</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='#SkPixmap_erase_3_color'>color</a> <a href='#SkPixmap_erase_3_color'>to</a> <a href='#SkPixmap_erase_3_color'>write</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_erase_3_subset'><code><strong>subset</strong></code></a></td>
    <td>bounding integer <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='SkRect_Reference#SkRect'>pixels</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>write</a>; <a href='SkRect_Reference#SkRect'>may</a> <a href='SkRect_Reference#SkRect'>be</a> <a href='SkRect_Reference#SkRect'>nullptr</a></td>
  </tr>
</table>

### Return Value

true if pixels are changed

### Example

<div><fiddle-embed name="f884f3f46a565f052a5e252ae2f36e9b"></fiddle-embed></div>

### See Also

<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_erase'>erase</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_clear'>clear</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawColor'>drawColor</a>

