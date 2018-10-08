SkPixmap Reference
===

<a name='SkPixmap'></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='#SkPixmap'>SkPixmap</a> {
public:
    <a href='#SkPixmap_empty_constructor'>SkPixmap()</a>;
    <a href='#SkPixmap_const_SkImageInfo_const_star'>SkPixmap(const SkImageInfo& info, const void* addr, size_t rowBytes)</a>;
    void <a href='#SkPixmap_reset'>reset</a>();
    void <a href='#SkPixmap_reset_2'>reset</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, const void* addr, size_t rowBytes);
    void <a href='#SkPixmap_setColorSpace'>setColorSpace</a>(sk_sp<<a href='undocumented#SkColorSpace'>SkColorSpace</a>> colorSpace);
    bool <a href='#SkPixmap_reset_3'>reset</a>(const <a href='undocumented#SkMask'>SkMask</a>& mask);
    bool <a href='#SkPixmap_extractSubset'>extractSubset</a>(<a href='#SkPixmap'>SkPixmap</a>* subset, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& area) const;
    const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='#SkPixmap_info'>info</a>() const;
    size_t <a href='#SkPixmap_rowBytes'>rowBytes</a>() const;
    const void* <a href='#SkPixmap_addr'>addr</a>() const;
    int <a href='#SkPixmap_width'>width</a>() const;
    int <a href='#SkPixmap_height'>height</a>() const;
    <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkPixmap_colorType'>colorType</a>() const;
    <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkPixmap_alphaType'>alphaType</a>() const;
    <a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='#SkPixmap_colorSpace'>colorSpace</a>() const;
    bool <a href='#SkPixmap_isOpaque'>isOpaque</a>() const;
    <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkPixmap_bounds'>bounds</a>() const;
    int <a href='#SkPixmap_rowBytesAsPixels'>rowBytesAsPixels</a>() const;
    int <a href='#SkPixmap_shiftPerPixel'>shiftPerPixel</a>() const;
    size_t <a href='#SkPixmap_computeByteSize'>computeByteSize</a>() const;
    bool <a href='#SkPixmap_computeIsOpaque'>computeIsOpaque</a>() const;
    <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkPixmap_getColor'>getColor</a>(int x, int y) const;
    float <a href='#SkPixmap_getAlphaf'>getAlphaf</a>(int x, int y) const;
    const void* <a href='#SkPixmap_addr_2'>addr</a>(int x, int y) const;
    const uint8_t* <a href='#SkPixmap_addr8'>addr8</a>() const;
    const uint16_t* <a href='#SkPixmap_addr16'>addr16</a>() const;
    const uint32_t* <a href='#SkPixmap_addr32'>addr32</a>() const;
    const uint64_t* <a href='#SkPixmap_addr64'>addr64</a>() const;
    const uint16_t* <a href='#SkPixmap_addrF16'>addrF16</a>() const;
    const uint8_t* <a href='#SkPixmap_addr8_2'>addr8</a>(int x, int y) const;
    const uint16_t* <a href='#SkPixmap_addr16_2'>addr16</a>(int x, int y) const;
    const uint32_t* <a href='#SkPixmap_addr32_2'>addr32</a>(int x, int y) const;
    const uint64_t* <a href='#SkPixmap_addr64_2'>addr64</a>(int x, int y) const;
    const uint16_t* <a href='#SkPixmap_addrF16_2'>addrF16</a>(int x, int y) const;
    void* <a href='#SkPixmap_writable_addr'>writable_addr</a>() const;
    void* <a href='#SkPixmap_writable_addr_2'>writable_addr</a>(int x, int y) const;
    uint8_t* <a href='#SkPixmap_writable_addr8'>writable_addr8</a>(int x, int y) const;
    uint16_t* <a href='#SkPixmap_writable_addr16'>writable_addr16</a>(int x, int y) const;
    uint32_t* <a href='#SkPixmap_writable_addr32'>writable_addr32</a>(int x, int y) const;
    uint64_t* <a href='#SkPixmap_writable_addr64'>writable_addr64</a>(int x, int y) const;
    uint16_t* <a href='#SkPixmap_writable_addrF16'>writable_addrF16</a>(int x, int y) const;
    bool <a href='#SkPixmap_readPixels'>readPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& dstInfo, void* dstPixels, size_t dstRowBytes) const;
    bool <a href='#SkPixmap_readPixels_2'>readPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& dstInfo, void* dstPixels, size_t dstRowBytes, int srcX,
                    int srcY) const;
    bool <a href='#SkPixmap_readPixels_3'>readPixels</a>(const <a href='#SkPixmap'>SkPixmap</a>& dst, int srcX, int srcY) const;
    bool <a href='#SkPixmap_readPixels_4'>readPixels</a>(const <a href='#SkPixmap'>SkPixmap</a>& dst) const;
    bool <a href='#SkPixmap_scalePixels'>scalePixels</a>(const <a href='#SkPixmap'>SkPixmap</a>& dst, <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> filterQuality) const;
    bool <a href='#SkPixmap_erase'>erase</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> color, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& subset) const;
    bool <a href='#SkPixmap_erase_2'>erase</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> color) const;
    bool <a href='#SkPixmap_erase_3'>erase</a>(const <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a>& color, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* subset = nullptr) const;
};
</pre>

<a href='#Pixmap'>Pixmap</a> provides a utility to pair <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> with pixels and row bytes

## <a name='Initialization'>Initialization</a>

<a name='SkPixmap_empty_constructor'></a>
## SkPixmap

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPixmap'>SkPixmap</a>(
</pre>

Creates an empty <a href='#Pixmap'>Pixmap</a> without pixels

### Return Value

empty <a href='#Pixmap'>Pixmap</a>

### Example

<div><fiddle-embed name="9547e74a9d37553a667b913ffd1312dd">

#### Example Output

~~~~
width:  0  height:  0  color: kUnknown_SkColorType  alpha: kUnknown_SkAlphaType
width: 25  height: 35  color: kRGBA_8888_SkColorType  alpha: kOpaque_SkAlphaType
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_const_SkImageInfo_const_star'>SkPixmap(const SkImageInfo& info, const void* addr, size t rowBytes)</a> <a href='#SkPixmap_reset'>reset</a><sup><a href='#SkPixmap_reset_2'>[2]</a></sup><sup><a href='#SkPixmap_reset_3'>[3]</a></sup>(

---

<a name='SkPixmap_const_SkImageInfo_const_star'></a>
## SkPixmap

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPixmap'>SkPixmap</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>
</pre>

Creates <a href='#Pixmap'>Pixmap</a> from info width

### Parameters

<table>  <tr>    <td><a name='SkPixmap_const_SkImageInfo_const_star_info'><code><strong>info</strong></code></a></td>
    <td>width</td>
  </tr>
  <tr>    <td><a name='SkPixmap_const_SkImageInfo_const_star_addr'><code><strong>addr</strong></code></a></td>
    <td>pointer to pixels allocated by caller</td>
  </tr>
  <tr>    <td><a name='SkPixmap_const_SkImageInfo_const_star_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>size of one row of addr</td>
  </tr>
</table>

### Return Value

initialized <a href='#Pixmap'>Pixmap</a>

### Example

<div><fiddle-embed name="9a00774be57d7308313b3a9073e6e696"><div><a href='SkImage_Reference#SkImage_MakeRasterCopy'>SkImage::MakeRasterCopy</a> takes const <a href='#SkPixmap'>SkPixmap</a></div>

#### Example Output

~~~~
image alpha only = false
copy alpha only = true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_empty_constructor'>SkPixmap()</a> <a href='#SkPixmap_reset'>reset</a><sup><a href='#SkPixmap_reset_2'>[2]</a></sup><sup><a href='#SkPixmap_reset_3'>[3]</a></sup>(

---

<a name='SkPixmap_reset'></a>
## reset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPixmap_reset'>reset</a>(
</pre>

Sets width

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

---

<a name='SkPixmap_reset_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPixmap_reset'>reset</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>
</pre>

Sets width

### Parameters

<table>  <tr>    <td><a name='SkPixmap_reset_2_info'><code><strong>info</strong></code></a></td>
    <td>width</td>
  </tr>
  <tr>    <td><a name='SkPixmap_reset_2_addr'><code><strong>addr</strong></code></a></td>
    <td>pointer to pixels allocated by caller</td>
  </tr>
  <tr>    <td><a name='SkPixmap_reset_2_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td>size of one row of addr</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="9a392b753167cfa849cebeefd5a6e07d"></fiddle-embed></div>

### See Also

<a href='#SkPixmap_const_SkImageInfo_const_star'>SkPixmap(const SkImageInfo& info, const void* addr, size t rowBytes)</a> <a href='#SkPixmap_reset'>reset</a><sup><a href='#SkPixmap_reset_2'>[2]</a></sup><sup><a href='#SkPixmap_reset_3'>[3]</a></sup>(

---

<a name='SkPixmap_setColorSpace'></a>
## setColorSpace

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPixmap_setColorSpace'>setColorSpace</a>(<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Changes <a href='undocumented#Color_Space'>Color Space</a> in <a href='SkImageInfo_Reference#Image_Info'>Image Info</a>

### Parameters

<table>  <tr>    <td><a name='SkPixmap_setColorSpace_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td><a href='undocumented#Color_Space'>Color Space</a> moved to <a href='SkImageInfo_Reference#Image_Info'>Image Info</a></td>
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

<a href='undocumented#Color_Space'>Color Space</a> <a href='SkImageInfo_Reference#SkImageInfo_makeColorSpace'>SkImageInfo::makeColorSpace</a>

---

<a name='SkPixmap_reset_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixmap_reset'>reset</a>(const <a href='undocumented#SkMask'>SkMask</a>
</pre>

To be deprecated soon.

---

<a name='SkPixmap_extractSubset'></a>
## extractSubset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixmap_extractSubset'>extractSubset</a>(<a href='#SkPixmap'>SkPixmap</a>
</pre>

Sets <a href='#SkPixmap_extractSubset_subset'>subset</a> width

### Parameters

<table>  <tr>    <td><a name='SkPixmap_extractSubset_subset'><code><strong>subset</strong></code></a></td>
    <td>storage for width</td>
  </tr>
  <tr>    <td><a name='SkPixmap_extractSubset_area'><code><strong>area</strong></code></a></td>
    <td>bounds to intersect with <a href='#Pixmap'>Pixmap</a></td>
  </tr>
</table>

### Return Value

true if intersection of <a href='#Pixmap'>Pixmap</a> and <a href='#SkPixmap_extractSubset_area'>area</a> is not empty

### Example

<div><fiddle-embed name="febdbfac6cf4cde69837643be2e1f6dd"></fiddle-embed></div>

### See Also

<a href='#SkPixmap_reset'>reset</a><sup><a href='#SkPixmap_reset_2'>[2]</a></sup><sup><a href='#SkPixmap_reset_3'>[3]</a></sup>(

---

## <a name='Image_Info_Access'>Image Info Access</a>

<a name='SkPixmap_info'></a>
## info

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>
</pre>

Returns width

### Return Value

reference to ImageInfo

### Example

<div><fiddle-embed name="6e0f558bf7fabc655041116288559134">

#### Example Output

~~~~
width: 384 height: 384 color: BGRA_8888 alpha: Opaque
~~~~

</fiddle-embed></div>

### See Also

<a href='SkImageInfo_Reference#Image_Info'>Image Info</a>

---

<a name='SkPixmap_rowBytes'></a>
## rowBytes

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkPixmap_rowBytes'>rowBytes</a>(
</pre>

Returns row bytes <code><a href='#SkPixmap_width'>width</a>(</code>

### Return Value

byte length of pixel row

### Example

<div><fiddle-embed name="19ac8bb81854680bd408fec8cb797d5c">

#### Example Output

~~~~
rowBytes: 2 minRowBytes: 4
rowBytes: 8 minRowBytes: 4
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr'>addr</a><sup><a href='#SkPixmap_addr_2'>[2]</a></sup>(

---

<a name='SkPixmap_addr'></a>
## addr

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const void
</pre>

Returns pixel address

### Return Value

pixel address

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

<a href='#SkPixmap_addr_2'>addr</a>(int x

---

<a name='SkPixmap_width'></a>
## width

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPixmap_width'>width</a>(
</pre>

Returns pixel count in each pixel row <code><a href='#SkPixmap_rowBytes'>rowBytes</a>(</code>

### Return Value

pixel width in <a href='SkImageInfo_Reference#Image_Info'>Image Info</a>

### Example

<div><fiddle-embed name="f68617b7153a20b2ed3d7f9ed5c6e5e4">

#### Example Output

~~~~
pixmap width: 16  info width: 16
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_height'>height</a>(

---

<a name='SkPixmap_height'></a>
## height

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPixmap_height'>height</a>(
</pre>

Returns pixel row count

### Return Value

pixel height in <a href='SkImageInfo_Reference#Image_Info'>Image Info</a>

### Example

<div><fiddle-embed name="4a996d32122f469d51ddd0186efb48cc">

#### Example Output

~~~~
pixmap height: 32  info height: 32
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_width'>width</a>(

---

<a name='SkPixmap_colorType'></a>
## colorType

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkPixmap_colorType'>colorType</a>(
</pre>

Returns <a href='SkImageInfo_Reference#Color_Type'>Color Type</a> <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>

### Return Value

<a href='SkImageInfo_Reference#Color_Type'>Color Type</a> in <a href='SkImageInfo_Reference#Image_Info'>Image Info</a>

### Example

<div><fiddle-embed name="0ab5c7af272685f2ce177cc79e6b9457">

#### Example Output

~~~~
color type: kAlpha_8_SkColorType
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_alphaType'>alphaType</a>(

---

<a name='SkPixmap_alphaType'></a>
## alphaType

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkPixmap_alphaType'>alphaType</a>(
</pre>

Returns <a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a> <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>

### Return Value

<a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a> in <a href='SkImageInfo_Reference#Image_Info'>Image Info</a>

### Example

<div><fiddle-embed name="070b1a60232be499eb10c6ea62371804">

#### Example Output

~~~~
alpha type: kPremul_SkAlphaType
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_colorType'>colorType</a>(

---

<a name='SkPixmap_colorSpace'></a>
## colorSpace

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkColorSpace'>SkColorSpace</a>
</pre>

Returns <a href='undocumented#Color_Space'>Color Space</a>

### Return Value

<a href='undocumented#Color_Space'>Color Space</a> in <a href='SkImageInfo_Reference#Image_Info'>Image Info</a>

### Example

<div><fiddle-embed name="3421bb20a302d563832ba7bb45e0cc58"><div><a href='undocumented#SkColorSpace_MakeSRGBLinear'>SkColorSpace::MakeSRGBLinear</a> creates <a href='undocumented#Color_Space'>Color Space</a> with linear gamma
and an sRGB gamut</div>

#### Example Output

~~~~
gammaCloseToSRGB: false  gammaIsLinear: true  isSRGB: false
~~~~

</fiddle-embed></div>

### See Also

<a href='undocumented#Color_Space'>Color Space</a> <a href='SkImageInfo_Reference#SkImageInfo_colorSpace'>SkImageInfo::colorSpace</a>

---

<a name='SkPixmap_isOpaque'></a>
## isOpaque

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixmap_isOpaque'>isOpaque</a>(
</pre>

Returns true if <a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a> is <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>

### Return Value

true if <a href='SkImageInfo_Reference#Image_Info'>Image Info</a> has opaque <a href='SkImageInfo_Reference#Alpha_Type'>Alpha Type</a>

### Example

<div><fiddle-embed name="efd083f121e888a523455ea8a49e50d1"><div><a href='#SkPixmap_isOpaque'>isOpaque</a> ignores whether all pixels are opaque or not</div>

#### Example Output

~~~~
isOpaque: false
isOpaque: false
isOpaque: true
isOpaque: true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_computeIsOpaque'>computeIsOpaque</a> <a href='SkImageInfo_Reference#SkImageInfo_isOpaque'>SkImageInfo::isOpaque</a>

---

<a name='SkPixmap_bounds'></a>
## bounds

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkPixmap_bounds'>bounds</a>(
</pre>

Returns <a href='SkIRect_Reference#IRect'>IRect</a>

### Return Value

integral rectangle from origin to <a href='#SkPixmap_width'>width</a>(

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

<a href='#SkPixmap_height'>height</a>(

---

<a name='SkPixmap_rowBytesAsPixels'></a>
## rowBytesAsPixels

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPixmap_rowBytesAsPixels'>rowBytesAsPixels</a>(
</pre>

Returns number of pixels that fit on row

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

<a href='#SkPixmap_rowBytes'>rowBytes</a> <a href='#SkPixmap_shiftPerPixel'>shiftPerPixel</a> <a href='#SkPixmap_width'>width</a> <a href='SkImageInfo_Reference#SkImageInfo_bytesPerPixel'>SkImageInfo::bytesPerPixel</a>

---

<a name='SkPixmap_shiftPerPixel'></a>
## shiftPerPixel

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPixmap_shiftPerPixel'>shiftPerPixel</a>(
</pre>

Returns bit shift converting row bytes to row pixels

### Return Value

one of

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

<a href='#SkPixmap_rowBytes'>rowBytes</a> <a href='#SkPixmap_rowBytesAsPixels'>rowBytesAsPixels</a> <a href='#SkPixmap_width'>width</a> <a href='SkImageInfo_Reference#SkImageInfo_bytesPerPixel'>SkImageInfo::bytesPerPixel</a>

---

<a name='SkPixmap_computeByteSize'></a>
## computeByteSize

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkPixmap_computeByteSize'>computeByteSize</a>(
</pre>

Returns minimum memory required for pixel storage

### Return Value

size in bytes of image buffer

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

<a href='SkImageInfo_Reference#SkImageInfo_computeByteSize'>SkImageInfo::computeByteSize</a>

---

## <a name='Reader'>Reader</a>

<a name='SkPixmap_computeIsOpaque'></a>
## computeIsOpaque

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixmap_computeIsOpaque'>computeIsOpaque</a>(
</pre>

Returns true if all pixels are opaque

### Return Value

true if all pixels have opaque values or <a href='SkImageInfo_Reference#Color_Type'>Color Type</a> is opaque

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

<a href='#SkPixmap_isOpaque'>isOpaque</a> <a href='SkImageInfo_Reference#Color_Type'>Color Type</a> <a href='SkColor_Reference#Alpha'>Alpha</a>

---

<a name='SkPixmap_getColor'></a>
## getColor

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkPixmap_getColor'>getColor</a>(int x
</pre>

Returns pixel at

### Parameters

<table>  <tr>    <td><a name='SkPixmap_getColor_x'><code><strong>x</strong></code></a></td>
    <td>column index</td>
  </tr>
  <tr>    <td><a name='SkPixmap_getColor_y'><code><strong>y</strong></code></a></td>
    <td>row index</td>
  </tr>
</table>

### Return Value

pixel converted to <a href='undocumented#Unpremultiply'>Unpremultiplied</a> <a href='SkColor_Reference#Color'>Color</a>

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

<a href='#SkPixmap_getAlphaf'>getAlphaf</a> <a href='#SkPixmap_addr'>addr</a><sup><a href='#SkPixmap_addr_2'>[2]</a></sup>(

---

<a name='SkPixmap_getAlphaf'></a>
## getAlphaf

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
float <a href='#SkPixmap_getAlphaf'>getAlphaf</a>(int x
</pre>

Looks up the pixel at <code>SkGetColorA(getColor()</code>

### Parameters

<table>  <tr>    <td><a name='SkPixmap_getAlphaf_x'><code><strong>x</strong></code></a></td>
    <td>column index</td>
  </tr>
  <tr>    <td><a name='SkPixmap_getAlphaf_y'><code><strong>y</strong></code></a></td>
    <td>row index</td>
  </tr>
</table>

### Return Value

alpha converted to normalized float

### See Also

<a href='#SkPixmap_getColor'>getColor</a>

---

## <a name='Readable_Address'>Readable Address</a>

<a name='SkPixmap_addr_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const void
</pre>

Returns readable pixel address at

### Parameters

<table>  <tr>    <td><a name='SkPixmap_addr_2_x'><code><strong>x</strong></code></a></td>
    <td>column index</td>
  </tr>
  <tr>    <td><a name='SkPixmap_addr_2_y'><code><strong>y</strong></code></a></td>
    <td>row index</td>
  </tr>
</table>

### Return Value

readable generic pointer to pixel

### Example

<div><fiddle-embed name="6e6e29e860eafed77308c973400cc84d">

#### Example Output

~~~~
pixmap.addr(1, 2) == &storage[1 + 2 * w]
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr8'>addr8</a><sup><a href='#SkPixmap_addr8_2'>[2]</a></sup> <a href='#SkPixmap_addr16'>addr16</a><sup><a href='#SkPixmap_addr16_2'>[2]</a></sup> <a href='#SkPixmap_addr32'>addr32</a><sup><a href='#SkPixmap_addr32_2'>[2]</a></sup> <a href='#SkPixmap_addr64'>addr64</a><sup><a href='#SkPixmap_addr64_2'>[2]</a></sup> <a href='#SkPixmap_addrF16'>addrF16</a><sup><a href='#SkPixmap_addrF16_2'>[2]</a></sup> <a href='#SkPixmap_getColor'>getColor</a> <a href='#SkPixmap_writable_addr'>writable addr</a><sup><a href='#SkPixmap_writable_addr_2'>[2]</a></sup> <a href='SkBitmap_Reference#SkBitmap_getAddr'>SkBitmap::getAddr</a>

---

<a name='SkPixmap_addr8'></a>
## addr8

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const uint8_t
</pre>

Returns readable base pixel address

### Return Value

readable unsigned 8

### Example

<div><fiddle-embed name="9adda80b2dd1b08ec5ccf66da7c8bd91">

#### Example Output

~~~~
pixmap.addr8() == storage
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr'>addr</a><sup><a href='#SkPixmap_addr_2'>[2]</a></sup>(

---

<a name='SkPixmap_addr16'></a>
## addr16

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const uint16_t
</pre>

Returns readable base pixel address

### Return Value

readable unsigned 16

### Example

<div><fiddle-embed name="9b16012d265c954c6de13f3fc960da52">

#### Example Output

~~~~
pixmap.addr16() == storage
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr'>addr</a><sup><a href='#SkPixmap_addr_2'>[2]</a></sup>(

---

<a name='SkPixmap_addr32'></a>
## addr32

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const uint32_t
</pre>

Returns readable base pixel address

### Return Value

readable unsigned 32

### Example

<div><fiddle-embed name="6b90c7ae9f254fe4ea9ef638f893a3e6">

#### Example Output

~~~~
pixmap.addr32() == storage
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr'>addr</a><sup><a href='#SkPixmap_addr_2'>[2]</a></sup>(

---

<a name='SkPixmap_addr64'></a>
## addr64

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const uint64_t
</pre>

Returns readable base pixel address

### Return Value

readable unsigned 64

### Example

<div><fiddle-embed name="0d17085a4698a8a2e2235fad9041b4b4">

#### Example Output

~~~~
pixmap.addr64() == storage
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr'>addr</a><sup><a href='#SkPixmap_addr_2'>[2]</a></sup>(

---

<a name='SkPixmap_addrF16'></a>
## addrF16

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const uint16_t
</pre>

Returns readable base pixel address

### Return Value

readable unsigned 16

### Example

<div><fiddle-embed name="54e8525a592f05623c33b375aebc90c1">

#### Example Output

~~~~
pixmap.addrF16() == storage
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr'>addr</a><sup><a href='#SkPixmap_addr_2'>[2]</a></sup>(

---

<a name='SkPixmap_addr8_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const uint8_t
</pre>

Returns readable pixel address at

### Parameters

<table>  <tr>    <td><a name='SkPixmap_addr8_2_x'><code><strong>x</strong></code></a></td>
    <td>column index</td>
  </tr>
  <tr>    <td><a name='SkPixmap_addr8_2_y'><code><strong>y</strong></code></a></td>
    <td>row index</td>
  </tr>
</table>

### Return Value

readable unsigned 8

### Example

<div><fiddle-embed name="5b986272268ef2c52045c1856f8b6107">

#### Example Output

~~~~
pixmap.addr8(1, 2) == &storage[1 + 2 * w]
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr'>addr</a><sup><a href='#SkPixmap_addr_2'>[2]</a></sup>(

---

<a name='SkPixmap_addr16_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const uint16_t
</pre>

Returns readable pixel address at

### Parameters

<table>  <tr>    <td><a name='SkPixmap_addr16_2_x'><code><strong>x</strong></code></a></td>
    <td>column index</td>
  </tr>
  <tr>    <td><a name='SkPixmap_addr16_2_y'><code><strong>y</strong></code></a></td>
    <td>row index</td>
  </tr>
</table>

### Return Value

readable unsigned 16

### Example

<div><fiddle-embed name="2c0c88a546d4ef093ab63ff72dac00b9">

#### Example Output

~~~~
pixmap.addr16(1, 2) == &storage[1 + 2 * w]
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr'>addr</a><sup><a href='#SkPixmap_addr_2'>[2]</a></sup>(

---

<a name='SkPixmap_addr32_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const uint32_t
</pre>

Returns readable pixel address at

### Parameters

<table>  <tr>    <td><a name='SkPixmap_addr32_2_x'><code><strong>x</strong></code></a></td>
    <td>column index</td>
  </tr>
  <tr>    <td><a name='SkPixmap_addr32_2_y'><code><strong>y</strong></code></a></td>
    <td>row index</td>
  </tr>
</table>

### Return Value

readable unsigned 32

### Example

<div><fiddle-embed name="12f8b5ce9fb25604f33df336677f5d62">

#### Example Output

~~~~
pixmap.addr32(1, 2) == &storage[1 + 2 * w]
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr'>addr</a><sup><a href='#SkPixmap_addr_2'>[2]</a></sup>(

---

<a name='SkPixmap_addr64_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const uint64_t
</pre>

Returns readable pixel address at

### Parameters

<table>  <tr>    <td><a name='SkPixmap_addr64_2_x'><code><strong>x</strong></code></a></td>
    <td>column index</td>
  </tr>
  <tr>    <td><a name='SkPixmap_addr64_2_y'><code><strong>y</strong></code></a></td>
    <td>row index</td>
  </tr>
</table>

### Return Value

readable unsigned 64

### Example

<div><fiddle-embed name="5449f65fd7673273b0b57807fd3117ff">

#### Example Output

~~~~
pixmap.addr64(1, 2) == &storage[1 + 2 * w]
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr'>addr</a><sup><a href='#SkPixmap_addr_2'>[2]</a></sup>(

---

<a name='SkPixmap_addrF16_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const uint16_t
</pre>

Returns readable pixel address at

### Parameters

<table>  <tr>    <td><a name='SkPixmap_addrF16_2_x'><code><strong>x</strong></code></a></td>
    <td>column index</td>
  </tr>
  <tr>    <td><a name='SkPixmap_addrF16_2_y'><code><strong>y</strong></code></a></td>
    <td>row index</td>
  </tr>
</table>

### Return Value

readable unsigned 16

### Example

<div><fiddle-embed name="f6076cad455bc80af5d06eb121d3b6f2">

#### Example Output

~~~~
pixmap.addrF16(1, 2) == &storage[1 * wordsPerPixel + 2 * rowWords]
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPixmap_addr'>addr</a><sup><a href='#SkPixmap_addr_2'>[2]</a></sup>(

---

## <a name='Writable_Address'>Writable Address</a>

<a name='SkPixmap_writable_addr'></a>
## writable_addr

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void
</pre>

Returns writable base pixel address

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

<a href='#SkPixmap_writable_addr8'>writable addr8</a> <a href='#SkPixmap_writable_addr16'>writable addr16</a> <a href='#SkPixmap_writable_addr32'>writable addr32</a> <a href='#SkPixmap_writable_addr64'>writable addr64</a> <a href='#SkPixmap_writable_addrF16'>writable addrF16</a> <a href='#SkPixmap_addr'>addr</a><sup><a href='#SkPixmap_addr_2'>[2]</a></sup>(

---

<a name='SkPixmap_writable_addr_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void
</pre>

Returns writable pixel address at

### Parameters

<table>  <tr>    <td><a name='SkPixmap_writable_addr_2_x'><code><strong>x</strong></code></a></td>
    <td>column index</td>
  </tr>
  <tr>    <td><a name='SkPixmap_writable_addr_2_y'><code><strong>y</strong></code></a></td>
    <td>row index</td>
  </tr>
</table>

### Return Value

writable generic pointer to pixel

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

<a href='#SkPixmap_writable_addr8'>writable addr8</a> <a href='#SkPixmap_writable_addr16'>writable addr16</a> <a href='#SkPixmap_writable_addr32'>writable addr32</a> <a href='#SkPixmap_writable_addr64'>writable addr64</a> <a href='#SkPixmap_writable_addrF16'>writable addrF16</a> <a href='#SkPixmap_addr'>addr</a><sup><a href='#SkPixmap_addr_2'>[2]</a></sup>(

---

<a name='SkPixmap_writable_addr8'></a>
## writable_addr8

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint8_t
</pre>

Returns writable pixel address at

### Parameters

<table>  <tr>    <td><a name='SkPixmap_writable_addr8_x'><code><strong>x</strong></code></a></td>
    <td>column index</td>
  </tr>
  <tr>    <td><a name='SkPixmap_writable_addr8_y'><code><strong>y</strong></code></a></td>
    <td>row index</td>
  </tr>
</table>

### Return Value

writable unsigned 8

### Example

<div><fiddle-embed name="809284db136748208b3efc31cd89de29"><div>Altering pixels after drawing <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> is not guaranteed to affect subsequent
drawing on all platforms</div></fiddle-embed></div>

### See Also

<a href='#SkPixmap_writable_addr'>writable addr</a><sup><a href='#SkPixmap_writable_addr_2'>[2]</a></sup> <a href='#SkPixmap_writable_addr16'>writable addr16</a> <a href='#SkPixmap_writable_addr32'>writable addr32</a> <a href='#SkPixmap_writable_addr64'>writable addr64</a> <a href='#SkPixmap_writable_addrF16'>writable addrF16</a> <a href='#SkPixmap_addr'>addr</a><sup><a href='#SkPixmap_addr_2'>[2]</a></sup>(

---

<a name='SkPixmap_writable_addr16'></a>
## writable_addr16

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint16_t
</pre>

Returns <a href='#SkPixmap_writable_addr'>writable addr</a> pixel address at

### Parameters

<table>  <tr>    <td><a name='SkPixmap_writable_addr16_x'><code><strong>x</strong></code></a></td>
    <td>column index</td>
  </tr>
  <tr>    <td><a name='SkPixmap_writable_addr16_y'><code><strong>y</strong></code></a></td>
    <td>row index</td>
  </tr>
</table>

### Return Value

writable unsigned 16

### Example

<div><fiddle-embed name="6da54774f6432b46b47ea9013c15f280"><div>Draw a five by five bitmap</div></fiddle-embed></div>

### See Also

<a href='#SkPixmap_writable_addr'>writable addr</a><sup><a href='#SkPixmap_writable_addr_2'>[2]</a></sup> <a href='#SkPixmap_writable_addr8'>writable addr8</a> <a href='#SkPixmap_writable_addr32'>writable addr32</a> <a href='#SkPixmap_writable_addr64'>writable addr64</a> <a href='#SkPixmap_writable_addrF16'>writable addrF16</a> <a href='#SkPixmap_addr'>addr</a><sup><a href='#SkPixmap_addr_2'>[2]</a></sup>(

---

<a name='SkPixmap_writable_addr32'></a>
## writable_addr32

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t
</pre>

Returns writable pixel address at

### Parameters

<table>  <tr>    <td><a name='SkPixmap_writable_addr32_x'><code><strong>x</strong></code></a></td>
    <td>column index</td>
  </tr>
  <tr>    <td><a name='SkPixmap_writable_addr32_y'><code><strong>y</strong></code></a></td>
    <td>row index</td>
  </tr>
</table>

### Return Value

writable unsigned 32

### Example

<div><fiddle-embed name="086866243bf9e4c14c3b215a2aa69ad9"></fiddle-embed></div>

### See Also

<a href='#SkPixmap_writable_addr'>writable addr</a><sup><a href='#SkPixmap_writable_addr_2'>[2]</a></sup> <a href='#SkPixmap_writable_addr8'>writable addr8</a> <a href='#SkPixmap_writable_addr16'>writable addr16</a> <a href='#SkPixmap_writable_addr64'>writable addr64</a> <a href='#SkPixmap_writable_addrF16'>writable addrF16</a> <a href='#SkPixmap_addr'>addr</a><sup><a href='#SkPixmap_addr_2'>[2]</a></sup>(

---

<a name='SkPixmap_writable_addr64'></a>
## writable_addr64

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint64_t
</pre>

Returns writable pixel address at

### Parameters

<table>  <tr>    <td><a name='SkPixmap_writable_addr64_x'><code><strong>x</strong></code></a></td>
    <td>column index</td>
  </tr>
  <tr>    <td><a name='SkPixmap_writable_addr64_y'><code><strong>y</strong></code></a></td>
    <td>row index</td>
  </tr>
</table>

### Return Value

writable unsigned 64

### Example

<div><fiddle-embed name="de14d8d30e4a7b6462103d0e0dd96b0b"></fiddle-embed></div>

### See Also

<a href='#SkPixmap_writable_addr'>writable addr</a><sup><a href='#SkPixmap_writable_addr_2'>[2]</a></sup> <a href='#SkPixmap_writable_addr8'>writable addr8</a> <a href='#SkPixmap_writable_addr16'>writable addr16</a> <a href='#SkPixmap_writable_addr32'>writable addr32</a> <a href='#SkPixmap_writable_addrF16'>writable addrF16</a> <a href='#SkPixmap_addr'>addr</a><sup><a href='#SkPixmap_addr_2'>[2]</a></sup>(

---

<a name='SkPixmap_writable_addrF16'></a>
## writable_addrF16

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint16_t
</pre>

Returns writable pixel address at

### Parameters

<table>  <tr>    <td><a name='SkPixmap_writable_addrF16_x'><code><strong>x</strong></code></a></td>
    <td>column index</td>
  </tr>
  <tr>    <td><a name='SkPixmap_writable_addrF16_y'><code><strong>y</strong></code></a></td>
    <td>row index</td>
  </tr>
</table>

### Return Value

writable unsigned 16

### Example

<div><fiddle-embed name="7822d78f5cacf5c04267cbbc6c6d0b80"><div>Left bitmap is drawn with two pixels defined in half float format</div></fiddle-embed></div>

### See Also

<a href='#SkPixmap_writable_addr'>writable addr</a><sup><a href='#SkPixmap_writable_addr_2'>[2]</a></sup> <a href='#SkPixmap_writable_addr8'>writable addr8</a> <a href='#SkPixmap_writable_addr16'>writable addr16</a> <a href='#SkPixmap_writable_addr32'>writable addr32</a> <a href='#SkPixmap_writable_addr64'>writable addr64</a> <a href='#SkPixmap_addr'>addr</a><sup><a href='#SkPixmap_addr_2'>[2]</a></sup>(

---

## <a name='Pixels'>Pixels</a>

<a name='SkPixmap_readPixels'></a>
## readPixels

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixmap_readPixels'>readPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>
</pre>

Copies a <a href='SkRect_Reference#Rect'>Rect</a> of pixels to <a href='#SkPixmap_readPixels_dstPixels'>dstPixels</a>

### Parameters

<table>  <tr>    <td><a name='SkPixmap_readPixels_dstInfo'><code><strong>dstInfo</strong></code></a></td>
    <td>destination width</td>
  </tr>
  <tr>    <td><a name='SkPixmap_readPixels_dstPixels'><code><strong>dstPixels</strong></code></a></td>
    <td>destination pixel storage</td>
  </tr>
  <tr>    <td><a name='SkPixmap_readPixels_dstRowBytes'><code><strong>dstRowBytes</strong></code></a></td>
    <td>destination row length</td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href='#SkPixmap_readPixels_dstPixels'>dstPixels</a>

### Example

<div><fiddle-embed name="df4e355c4845350daede833b4fd21ec1"><div>Transferring the gradient from 8 bits per component to 4 bits per component
creates visible banding</div></fiddle-embed></div>

### See Also

<a href='#SkPixmap_erase'>erase</a><sup><a href='#SkPixmap_erase_2'>[2]</a></sup><sup><a href='#SkPixmap_erase_3'>[3]</a></sup> <a href='SkBitmap_Reference#SkBitmap_readPixels'>SkBitmap::readPixels</a><sup><a href='SkBitmap_Reference#SkBitmap_readPixels_2'>[2]</a></sup><sup><a href='SkBitmap_Reference#SkBitmap_readPixels_3'>[3]</a></sup> <a href='SkCanvas_Reference#SkCanvas_drawBitmap'>SkCanvas::drawBitmap</a> <a href='SkCanvas_Reference#SkCanvas_readPixels'>SkCanvas::readPixels</a><sup><a href='SkCanvas_Reference#SkCanvas_readPixels_2'>[2]</a></sup><sup><a href='SkCanvas_Reference#SkCanvas_readPixels_3'>[3]</a></sup> <a href='SkImage_Reference#SkImage_readPixels'>SkImage::readPixels</a><sup><a href='SkImage_Reference#SkImage_readPixels_2'>[2]</a></sup> <a href='SkSurface_Reference#SkSurface_readPixels'>SkSurface::readPixels</a><sup><a href='SkSurface_Reference#SkSurface_readPixels_2'>[2]</a></sup><sup><a href='SkSurface_Reference#SkSurface_readPixels_3'>[3]</a></sup>

---

<a name='SkPixmap_readPixels_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixmap_readPixels'>readPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>
</pre>

Copies a <a href='SkRect_Reference#Rect'>Rect</a> of pixels to <a href='#SkPixmap_readPixels_2_dstPixels'>dstPixels</a> <code>abs(srcX)</code> <code>abs(srcY)</code>

### Parameters

<table>  <tr>    <td><a name='SkPixmap_readPixels_2_dstInfo'><code><strong>dstInfo</strong></code></a></td>
    <td>destination width</td>
  </tr>
  <tr>    <td><a name='SkPixmap_readPixels_2_dstPixels'><code><strong>dstPixels</strong></code></a></td>
    <td>destination pixel storage</td>
  </tr>
  <tr>    <td><a name='SkPixmap_readPixels_2_dstRowBytes'><code><strong>dstRowBytes</strong></code></a></td>
    <td>destination row length</td>
  </tr>
  <tr>    <td><a name='SkPixmap_readPixels_2_srcX'><code><strong>srcX</strong></code></a></td>
    <td>column index whose absolute value is less than <a href='#SkPixmap_width'>width</a>(</td>
  </tr>
  <tr>    <td><a name='SkPixmap_readPixels_2_srcY'><code><strong>srcY</strong></code></a></td>
    <td>row index whose absolute value is less than <a href='#SkPixmap_height'>height</a>(</td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href='#SkPixmap_readPixels_2_dstPixels'>dstPixels</a>

### Example

<div><fiddle-embed name="094ca0bd37588cc7be241bb387a3e17b"></fiddle-embed></div>

### See Also

<a href='#SkPixmap_erase'>erase</a><sup><a href='#SkPixmap_erase_2'>[2]</a></sup><sup><a href='#SkPixmap_erase_3'>[3]</a></sup> <a href='SkBitmap_Reference#SkBitmap_readPixels'>SkBitmap::readPixels</a><sup><a href='SkBitmap_Reference#SkBitmap_readPixels_2'>[2]</a></sup><sup><a href='SkBitmap_Reference#SkBitmap_readPixels_3'>[3]</a></sup> <a href='SkCanvas_Reference#SkCanvas_drawBitmap'>SkCanvas::drawBitmap</a> <a href='SkCanvas_Reference#SkCanvas_readPixels'>SkCanvas::readPixels</a><sup><a href='SkCanvas_Reference#SkCanvas_readPixels_2'>[2]</a></sup><sup><a href='SkCanvas_Reference#SkCanvas_readPixels_3'>[3]</a></sup> <a href='SkImage_Reference#SkImage_readPixels'>SkImage::readPixels</a><sup><a href='SkImage_Reference#SkImage_readPixels_2'>[2]</a></sup> <a href='SkSurface_Reference#SkSurface_readPixels'>SkSurface::readPixels</a><sup><a href='SkSurface_Reference#SkSurface_readPixels_2'>[2]</a></sup><sup><a href='SkSurface_Reference#SkSurface_readPixels_3'>[3]</a></sup>

---

<a name='SkPixmap_readPixels_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixmap_readPixels'>readPixels</a>(const <a href='#SkPixmap'>SkPixmap</a>
</pre>

Copies a <a href='SkRect_Reference#Rect'>Rect</a> of pixels to <a href='#SkPixmap_readPixels_3_dst'>dst</a> <code>abs(srcX)</code> <code>abs(srcY)</code>

### Parameters

<table>  <tr>    <td><a name='SkPixmap_readPixels_3_dst'><code><strong>dst</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#Image_Info'>Image Info</a> and pixel address to write to</td>
  </tr>
  <tr>    <td><a name='SkPixmap_readPixels_3_srcX'><code><strong>srcX</strong></code></a></td>
    <td>column index whose absolute value is less than <a href='#SkPixmap_width'>width</a>(</td>
  </tr>
  <tr>    <td><a name='SkPixmap_readPixels_3_srcY'><code><strong>srcY</strong></code></a></td>
    <td>row index whose absolute value is less than <a href='#SkPixmap_height'>height</a>(</td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href='#SkPixmap_readPixels_3_dst'>dst</a>

### Example

<div><fiddle-embed name="6ec7f7b2cc163cd29f627eef6d4b061c"></fiddle-embed></div>

### See Also

<a href='#SkPixmap_erase'>erase</a><sup><a href='#SkPixmap_erase_2'>[2]</a></sup><sup><a href='#SkPixmap_erase_3'>[3]</a></sup> <a href='SkBitmap_Reference#SkBitmap_readPixels'>SkBitmap::readPixels</a><sup><a href='SkBitmap_Reference#SkBitmap_readPixels_2'>[2]</a></sup><sup><a href='SkBitmap_Reference#SkBitmap_readPixels_3'>[3]</a></sup> <a href='SkCanvas_Reference#SkCanvas_drawBitmap'>SkCanvas::drawBitmap</a> <a href='SkCanvas_Reference#SkCanvas_readPixels'>SkCanvas::readPixels</a><sup><a href='SkCanvas_Reference#SkCanvas_readPixels_2'>[2]</a></sup><sup><a href='SkCanvas_Reference#SkCanvas_readPixels_3'>[3]</a></sup> <a href='SkImage_Reference#SkImage_readPixels'>SkImage::readPixels</a><sup><a href='SkImage_Reference#SkImage_readPixels_2'>[2]</a></sup> <a href='SkSurface_Reference#SkSurface_readPixels'>SkSurface::readPixels</a><sup><a href='SkSurface_Reference#SkSurface_readPixels_2'>[2]</a></sup><sup><a href='SkSurface_Reference#SkSurface_readPixels_3'>[3]</a></sup>

---

<a name='SkPixmap_readPixels_4'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixmap_readPixels'>readPixels</a>(const <a href='#SkPixmap'>SkPixmap</a>
</pre>

Copies pixels inside <a href='#SkPixmap_bounds'>bounds</a>(

### Parameters

<table>  <tr>    <td><a name='SkPixmap_readPixels_4_dst'><code><strong>dst</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#Image_Info'>Image Info</a> and pixel address to write to</td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href='#SkPixmap_readPixels_4_dst'>dst</a>

### Example

<div><fiddle-embed name="e18549b5ee1039cb61b0bb38c2104fc9"></fiddle-embed></div>

### See Also

<a href='#SkPixmap_erase'>erase</a><sup><a href='#SkPixmap_erase_2'>[2]</a></sup><sup><a href='#SkPixmap_erase_3'>[3]</a></sup> <a href='SkBitmap_Reference#SkBitmap_readPixels'>SkBitmap::readPixels</a><sup><a href='SkBitmap_Reference#SkBitmap_readPixels_2'>[2]</a></sup><sup><a href='SkBitmap_Reference#SkBitmap_readPixels_3'>[3]</a></sup> <a href='SkCanvas_Reference#SkCanvas_drawBitmap'>SkCanvas::drawBitmap</a> <a href='SkCanvas_Reference#SkCanvas_readPixels'>SkCanvas::readPixels</a><sup><a href='SkCanvas_Reference#SkCanvas_readPixels_2'>[2]</a></sup><sup><a href='SkCanvas_Reference#SkCanvas_readPixels_3'>[3]</a></sup> <a href='SkImage_Reference#SkImage_readPixels'>SkImage::readPixels</a><sup><a href='SkImage_Reference#SkImage_readPixels_2'>[2]</a></sup> <a href='SkSurface_Reference#SkSurface_readPixels'>SkSurface::readPixels</a><sup><a href='SkSurface_Reference#SkSurface_readPixels_2'>[2]</a></sup><sup><a href='SkSurface_Reference#SkSurface_readPixels_3'>[3]</a></sup>

---

<a name='SkPixmap_scalePixels'></a>
## scalePixels

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixmap_scalePixels'>scalePixels</a>(const <a href='#SkPixmap'>SkPixmap</a>
</pre>

Copies <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> to <a href='#SkPixmap_scalePixels_dst'>dst</a>

### Parameters

<table>  <tr>    <td><a name='SkPixmap_scalePixels_dst'><code><strong>dst</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#Image_Info'>Image Info</a> and pixel address to write to</td>
  </tr>
  <tr>    <td><a name='SkPixmap_scalePixels_filterQuality'><code><strong>filterQuality</strong></code></a></td>
    <td>one of</td>
  </tr>
</table>

### Return Value

true if pixels are scaled to fit <a href='#SkPixmap_scalePixels_dst'>dst</a>

### Example

<div><fiddle-embed name="8e3c8a9c1d0d2e9b8bf66e24d274f792"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas_drawBitmap'>SkCanvas::drawBitmap</a> <a href='SkImage_Reference#SkImage_scalePixels'>SkImage::scalePixels</a>

---

<a name='SkPixmap_erase'></a>
## erase

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixmap_erase'>erase</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> color
</pre>

Writes <a href='#SkPixmap_erase_color'>color</a> to pixels bounded by <a href='#SkPixmap_erase_subset'>subset</a>

### Parameters

<table>  <tr>    <td><a name='SkPixmap_erase_color'><code><strong>color</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>Unpremultiplied</a> <a href='SkColor_Reference#Color'>Color</a> to write</td>
  </tr>
  <tr>    <td><a name='SkPixmap_erase_subset'><code><strong>subset</strong></code></a></td>
    <td>bounding integer <a href='SkRect_Reference#Rect'>Rect</a> of written pixels</td>
  </tr>
</table>

### Return Value

true if pixels are changed

### Example

<div><fiddle-embed name="a0cdbafed4786788cc90681e7b294234"></fiddle-embed></div>

### See Also

<a href='SkBitmap_Reference#SkBitmap_erase'>SkBitmap::erase</a> <a href='SkCanvas_Reference#SkCanvas_clear'>SkCanvas::clear</a> <a href='SkCanvas_Reference#SkCanvas_drawColor'>SkCanvas::drawColor</a>

---

<a name='SkPixmap_erase_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixmap_erase'>erase</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> color
</pre>

Writes <a href='#SkPixmap_erase_2_color'>color</a> to pixels inside <a href='#SkPixmap_bounds'>bounds</a>(

### Parameters

<table>  <tr>    <td><a name='SkPixmap_erase_2_color'><code><strong>color</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>Unpremultiplied</a> <a href='SkColor_Reference#Color'>Color</a> to write</td>
  </tr>
</table>

### Return Value

true if pixels are changed

### Example

<div><fiddle-embed name="838202e0d49cad2eb3eeb495834f6d63"></fiddle-embed></div>

### See Also

<a href='SkBitmap_Reference#SkBitmap_erase'>SkBitmap::erase</a> <a href='SkCanvas_Reference#SkCanvas_clear'>SkCanvas::clear</a> <a href='SkCanvas_Reference#SkCanvas_drawColor'>SkCanvas::drawColor</a>

---

<a name='SkPixmap_erase_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixmap_erase'>erase</a>(const <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a>
</pre>

Writes <a href='#SkPixmap_erase_3_color'>color</a> to pixels bounded by <a href='#SkPixmap_erase_3_subset'>subset</a>

### Parameters

<table>  <tr>    <td><a name='SkPixmap_erase_3_color'><code><strong>color</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>Unpremultiplied</a> <a href='SkColor_Reference#Color'>Color</a> to write</td>
  </tr>
  <tr>    <td><a name='SkPixmap_erase_3_subset'><code><strong>subset</strong></code></a></td>
    <td>bounding integer <a href='SkRect_Reference#Rect'>Rect</a> of pixels to write</td>
  </tr>
</table>

### Return Value

true if pixels are changed

### Example

<div><fiddle-embed name="f884f3f46a565f052a5e252ae2f36e9b"></fiddle-embed></div>

### See Also

<a href='SkBitmap_Reference#SkBitmap_erase'>SkBitmap::erase</a> <a href='SkCanvas_Reference#SkCanvas_clear'>SkCanvas::clear</a> <a href='SkCanvas_Reference#SkCanvas_drawColor'>SkCanvas::drawColor</a>

---

