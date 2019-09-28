SkPixmap Reference
===


<a name='SkPixmap'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> {

    <a href='#SkPixmap_empty_constructor'>SkPixmap()</a>;
    <a href='#SkPixmap_const_SkImageInfo_const_star'>SkPixmap</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, const void* addr, size_t <a href='#SkPixmap_rowBytes'>rowBytes</a>);
    void <a href='#SkPixmap_reset'>reset()</a>;
    void <a href='#SkPixmap_reset'>reset</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, const void* addr, size_t <a href='#SkPixmap_rowBytes'>rowBytes</a>);
    void <a href='#SkPixmap_setColorSpace'>setColorSpace</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='#SkPixmap_colorSpace'>colorSpace</a>);
    bool <a href='#SkPixmap_extractSubset'>extractSubset</a>(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>* subset, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& area) const;
    const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='#SkPixmap_info'>info()</a> const;
    size_t <a href='#SkPixmap_rowBytes'>rowBytes</a>() const;
    const void* <a href='#SkPixmap_addr'>addr()</a> const;
    int <a href='#SkPixmap_width'>width()</a> const;
    int <a href='#SkPixmap_height'>height()</a> const;
    <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkPixmap_colorType'>colorType</a>() const;
    <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkPixmap_alphaType'>alphaType</a>() const;
    <a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='#SkPixmap_colorSpace'>colorSpace</a>() const;
    bool <a href='#SkPixmap_isOpaque'>isOpaque</a>() const;
    <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkPixmap_bounds'>bounds()</a> const;
    int <a href='#SkPixmap_rowBytesAsPixels'>rowBytesAsPixels</a>() const;
    int <a href='#SkPixmap_shiftPerPixel'>shiftPerPixel</a>() const;
    size_t <a href='#SkPixmap_computeByteSize'>computeByteSize</a>() const;
    bool <a href='#SkPixmap_computeIsOpaque'>computeIsOpaque</a>() const;
    <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkPixmap_getColor'>getColor</a>(int x, int y) const;
    float <a href='#SkPixmap_getAlphaf'>getAlphaf</a>(int x, int y) const;
    const void* <a href='#SkPixmap_addr'>addr</a>(int x, int y) const;
    const uint8_t* <a href='#SkPixmap_addr8'>addr8</a>() const;
    const uint16_t* <a href='#SkPixmap_addr16'>addr16</a>() const;
    const uint32_t* <a href='#SkPixmap_addr32'>addr32</a>() const;
    const uint64_t* <a href='#SkPixmap_addr64'>addr64</a>() const;
    const uint16_t* <a href='#SkPixmap_addrF16'>addrF16</a>() const;
    const uint8_t* <a href='#SkPixmap_addr8'>addr8</a>(int x, int y) const;
    const uint16_t* <a href='#SkPixmap_addr16'>addr16</a>(int x, int y) const;
    const uint32_t* <a href='#SkPixmap_addr32'>addr32</a>(int x, int y) const;
    const uint64_t* <a href='#SkPixmap_addr64'>addr64</a>(int x, int y) const;
    const uint16_t* <a href='#SkPixmap_addrF16'>addrF16</a>(int x, int y) const;
    void* <a href='#SkPixmap_writable_addr'>writable_addr</a>() const;
    void* <a href='#SkPixmap_writable_addr'>writable_addr</a>(int x, int y) const;
    uint8_t* <a href='#SkPixmap_writable_addr8'>writable_addr8</a>(int x, int y) const;
    uint16_t* <a href='#SkPixmap_writable_addr16'>writable_addr16</a>(int x, int y) const;
    uint32_t* <a href='#SkPixmap_writable_addr32'>writable_addr32</a>(int x, int y) const;
    uint64_t* <a href='#SkPixmap_writable_addr64'>writable_addr64</a>(int x, int y) const;
    uint16_t* <a href='#SkPixmap_writable_addrF16'>writable_addrF16</a>(int x, int y) const;
    bool <a href='#SkPixmap_readPixels'>readPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& dstInfo, void* dstPixels, size_t dstRowBytes) const;
    bool <a href='#SkPixmap_readPixels'>readPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& dstInfo, void* dstPixels, size_t dstRowBytes, int srcX,
                    int srcY) const;
    bool <a href='#SkPixmap_readPixels'>readPixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& dst, int srcX, int srcY) const;
    bool <a href='#SkPixmap_readPixels'>readPixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& dst) const;
    bool <a href='#SkPixmap_scalePixels'>scalePixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& dst, <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> filterQuality) const;
    bool <a href='#SkPixmap_erase'>erase</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#Color'>color</a>, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& subset) const;
    bool <a href='#SkPixmap_erase'>erase</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#Color'>color</a>) const;
    bool <a href='#SkPixmap_erase'>erase</a>(const <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a>& <a href='SkColor_Reference#Color'>color</a>, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* subset = nullptr) const;
};

</pre>

<a href='SkPixmap_Reference#Pixmap'>Pixmap</a> provides a utility to pair <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> with pixels and row bytes.
<a href='SkPixmap_Reference#Pixmap'>Pixmap</a> is a low level class which provides convenience functions to access
raster destinations. <a href='SkCanvas_Reference#Canvas'>Canvas</a> can not draw <a href='SkPixmap_Reference#Pixmap'>Pixmap</a>, nor does <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> provide
a direct drawing destination.

Use <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> to draw pixels referenced by <a href='SkPixmap_Reference#Pixmap'>Pixmap</a>; use <a href='SkSurface_Reference#Surface'>Surface</a> to draw into
pixels referenced by <a href='SkPixmap_Reference#Pixmap'>Pixmap</a>.

<a href='SkPixmap_Reference#Pixmap'>Pixmap</a> does not try to manage the lifetime of the <a href='undocumented#Pixel'>pixel</a> memory. Use <a href='#Pixel_Ref'>Pixel_Ref</a>
to manage <a href='undocumented#Pixel'>pixel</a> memory; <a href='#Pixel_Ref'>Pixel_Ref</a> is safe across threads.

<a name='Initialization'></a>

<a name='SkPixmap_empty_constructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPixmap_empty_constructor'>SkPixmap()</a>
</pre>

Creates an empty <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> without pixels, with <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, with
<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, and with a width and height of zero. Use
<a href='#SkPixmap_reset'>reset()</a> to associate pixels, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, width, and height
after <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> has been created.

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

<a href='#SkPixmap_const_SkImageInfo_const_star'>SkPixmap</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='#SkPixmap_info'>info</a>, const void* <a href='#SkPixmap_addr'>addr</a>, size_t <a href='#SkPixmap_rowBytes'>rowBytes</a>) <a href='#SkPixmap_reset'>reset()</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>

<a name='SkPixmap_const_SkImageInfo_const_star'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPixmap_const_SkImageInfo_const_star'>SkPixmap</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, const void* addr, size_t <a href='#SkPixmap_rowBytes'>rowBytes</a>)
</pre>

Creates <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> from <a href='#SkPixmap_const_SkImageInfo_const_star_info'>info</a> width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, and <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.
<a href='#SkPixmap_const_SkImageInfo_const_star_addr'>addr</a> <a href='SkPoint_Reference#Point'>points</a> to pixels, or nullptr. <a href='#SkPixmap_const_SkImageInfo_const_star_rowBytes'>rowBytes</a> should be <a href='#SkPixmap_const_SkImageInfo_const_star_info'>info</a>.<a href='#SkImageInfo_width'>width()</a> times
<a href='#SkPixmap_const_SkImageInfo_const_star_info'>info</a>.<a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>(), or larger.

No parameter checking is performed; it is up to the caller to ensure that
<a href='#SkPixmap_const_SkImageInfo_const_star_addr'>addr</a> and <a href='#SkPixmap_const_SkImageInfo_const_star_rowBytes'>rowBytes</a> agree with <a href='#SkPixmap_const_SkImageInfo_const_star_info'>info</a>.

The memory lifetime of pixels is managed by the caller. When <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> goes
out of scope, <a href='#SkPixmap_const_SkImageInfo_const_star_addr'>addr</a> is unaffected.

<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> may be later modified by <a href='#SkPixmap_reset'>reset()</a> to change its <a href='undocumented#Size'>size</a>, <a href='undocumented#Pixel'>pixel</a> type, or
storage.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_const_SkImageInfo_const_star_info'><code><strong>info</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> of <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_const_SkImageInfo_const_star_addr'><code><strong>addr</strong></code></a></td>
    <td>pointer to pixels allocated by caller; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkPixmap_const_SkImageInfo_const_star_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> of one row of <a href='#SkPixmap_const_SkImageInfo_const_star_addr'>addr</a>; width times <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Size'>size</a>, or larger</td>
  </tr>
</table>

### Return Value

initialized <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>

### Example

<div><fiddle-embed name="9a00774be57d7308313b3a9073e6e696"><div><a href='SkImage_Reference#SkImage'>SkImage</a>::<a href='#SkImage_MakeRasterCopy'>MakeRasterCopy</a> takes const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& as an argument. The example
constructs a <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> from the brace-delimited parameters.
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

Sets width, height, row bytes to zero; <a href='undocumented#Pixel'>pixel</a> address to nullptr; <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> to
<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>; and <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> to <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>.

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
void reset(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, const void* addr, size_t <a href='#SkPixmap_rowBytes'>rowBytes</a>)
</pre>

Sets width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, and <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> from <a href='#SkPixmap_reset_2_info'>info</a>.
Sets <a href='undocumented#Pixel'>pixel</a> address from <a href='#SkPixmap_reset_2_addr'>addr</a>, which may be nullptr.
Sets row bytes from <a href='#SkPixmap_reset_2_rowBytes'>rowBytes</a>, which should be <a href='#SkPixmap_reset_2_info'>info</a>.<a href='#SkImageInfo_width'>width()</a> times
<a href='#SkPixmap_reset_2_info'>info</a>.<a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>(), or larger.

Does not check <a href='#SkPixmap_reset_2_addr'>addr</a>. Asserts if built with SK_DEBUG defined and if <a href='#SkPixmap_reset_2_rowBytes'>rowBytes</a> is
too small to hold one row of pixels.

The memory lifetime pixels are managed by the caller. When <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> goes
out of scope, <a href='#SkPixmap_reset_2_addr'>addr</a> is unaffected.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_reset_2_info'><code><strong>info</strong></code></a></td>
    <td>width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> of <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_reset_2_addr'><code><strong>addr</strong></code></a></td>
    <td>pointer to pixels allocated by caller; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkPixmap_reset_2_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> of one row of <a href='#SkPixmap_reset_2_addr'>addr</a>; width times <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Size'>size</a>, or larger</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="9a392b753167cfa849cebeefd5a6e07d"></fiddle-embed></div>

### See Also

<a href='#SkPixmap_const_SkImageInfo_const_star'>SkPixmap</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='#SkPixmap_reset_2_info'>info</a>, const void* <a href='#SkPixmap_reset_2_addr'>addr</a>, size_t <a href='#SkPixmap_reset_2_rowBytes'>rowBytes</a>) <a href='#SkPixmap_reset'>reset()</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>

<a name='SkPixmap_setColorSpace'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPixmap_setColorSpace'>setColorSpace</a>(<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; <a href='#SkPixmap_colorSpace'>colorSpace</a>)
</pre>

Changes <a href='undocumented#SkColorSpace'>SkColorSpace</a> in <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>; preserves width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, and
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> in <a href='SkImage_Reference#SkImage'>SkImage</a>, and leaves <a href='undocumented#Pixel'>pixel</a> address and row bytes unchanged.
<a href='undocumented#SkColorSpace'>SkColorSpace</a>  <a href='undocumented#Reference_Count'>reference count</a> is incremented.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_setColorSpace_colorSpace'><code><strong>colorSpace</strong></code></a></td>
    <td><a href='undocumented#SkColorSpace'>SkColorSpace</a> moved to <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a></td>
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

<a name='SkPixmap_extractSubset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixmap_extractSubset'>extractSubset</a>(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>* subset, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& area)const
</pre>

Sets <a href='#SkPixmap_extractSubset_subset'>subset</a> width, height, <a href='undocumented#Pixel'>pixel</a> address to intersection of <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> with <a href='#SkPixmap_extractSubset_area'>area</a>,
if intersection is not empty; and return true. Otherwise, leave <a href='#SkPixmap_extractSubset_subset'>subset</a> unchanged
and return false.

Failing to read the return value generates a compile time warning.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_extractSubset_subset'><code><strong>subset</strong></code></a></td>
    <td>storage for width, height, <a href='undocumented#Pixel'>pixel</a> address of intersection</td>
  </tr>
  <tr>    <td><a name='SkPixmap_extractSubset_area'><code><strong>area</strong></code></a></td>
    <td>bounds to intersect with <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a></td>
  </tr>
</table>

### Return Value

true if intersection of <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> and <a href='#SkPixmap_extractSubset_area'>area</a> is not empty

### Example

<div><fiddle-embed name="febdbfac6cf4cde69837643be2e1f6dd"></fiddle-embed></div>

### See Also

<a href='#SkPixmap_reset'>reset()</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_intersect'>intersect</a>

<a name='Image_Info_Access'></a>

<a name='SkPixmap_info'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='#SkPixmap_info'>info()</a>const
</pre>

Returns width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, and <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

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
size_t <a href='#SkPixmap_rowBytes'>rowBytes</a>()const
</pre>

Returns row bytes, the interval from one <a href='undocumented#Pixel'>pixel</a> row to the next. Row bytes
is at least as large as: <code><a href='#SkPixmap_width'>width()</a> * <a href='#SkPixmap_info'>info()</a>.<a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>()</code>.

Returns zero if <a href='#SkPixmap_colorType'>colorType</a> is <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>.
It is up to the <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> creator to ensure that row bytes is a useful value.

### Return Value

byte length of <a href='undocumented#Pixel'>pixel</a> row

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
const void* <a href='#SkPixmap_addr'>addr()</a>const
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> address, the base address corresponding to the <a href='undocumented#Pixel'>pixel</a> origin.

It is up to the <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> creator to ensure that <a href='undocumented#Pixel'>pixel</a> address is a useful value.

### Return Value

<a href='undocumented#Pixel'>pixel</a> address

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
int <a href='#SkPixmap_width'>width()</a>const
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> count in each <a href='undocumented#Pixel'>pixel</a> row. Should be equal or less than:

<code><a href='#SkPixmap_rowBytes'>rowBytes</a>() / <a href='#SkPixmap_info'>info()</a>.<a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>()</code>.

### Return Value

<a href='undocumented#Pixel'>pixel</a> width in <a href='#Image_Info'>Image_Info</a>

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
int <a href='#SkPixmap_height'>height()</a>const
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> row count.

### Return Value

<a href='undocumented#Pixel'>pixel</a> height in <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

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
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkPixmap_colorType'>colorType</a>()const
</pre>

Returns <a href='#Image_Info_Color_Type'>Color_Type</a>, one of: <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>
.

### Return Value

<a href='#Image_Info_Color_Type'>Color_Type</a> in <a href='#Image_Info'>Image_Info</a>

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
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkPixmap_alphaType'>alphaType</a>()const
</pre>

Returns <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, one of: <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>
.

### Return Value

<a href='#Image_Info_Alpha_Type'>Alpha_Type</a> in <a href='#Image_Info'>Image_Info</a>

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
<a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='#SkPixmap_colorSpace'>colorSpace</a>()const
</pre>

Returns <a href='undocumented#SkColorSpace'>SkColorSpace</a>, the range of colors, associated with <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>. The
reference count of <a href='undocumented#SkColorSpace'>SkColorSpace</a> is unchanged. The returned <a href='undocumented#SkColorSpace'>SkColorSpace</a> is
immutable.

### Return Value

<a href='undocumented#SkColorSpace'>SkColorSpace</a> in <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, or nullptr

### Example

<div><fiddle-embed name="3421bb20a302d563832ba7bb45e0cc58"><div><a href='undocumented#SkColorSpace'>SkColorSpace</a>::<a href='#SkColorSpace_MakeSRGBLinear'>MakeSRGBLinear</a> creates <a href='#Color_Space'>Color_Space</a> with linear gamma
and an sRGB gamut. This <a href='#Color_Space'>Color_Space</a> gamma is not close to sRGB gamma.
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
bool <a href='#SkPixmap_isOpaque'>isOpaque</a>()const
</pre>

Returns true if <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> is <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>.
Does not check if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> allows <a href='SkColor_Reference#Alpha'>alpha</a>, or if any <a href='undocumented#Pixel'>pixel</a> value has
transparency.

### Return Value

true if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> has opaque <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>

### Example

<div><fiddle-embed name="efd083f121e888a523455ea8a49e50d1"><div><a href='#SkPixmap_isOpaque'>isOpaque</a> ignores whether all pixels are opaque or not.
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
<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkPixmap_bounds'>bounds()</a>const
</pre>

Returns <a href='SkIRect_Reference#SkIRect'>SkIRect</a> { 0, 0, <a href='#SkPixmap_width'>width()</a>, <a href='#SkPixmap_height'>height()</a> }.

### Return Value

integral rectangle from origin to <a href='#SkPixmap_width'>width()</a> and <a href='#SkPixmap_height'>height()</a>

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
int <a href='#SkPixmap_rowBytesAsPixels'>rowBytesAsPixels</a>()const
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
int <a href='#SkPixmap_shiftPerPixel'>shiftPerPixel</a>()const
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
size_t <a href='#SkPixmap_computeByteSize'>computeByteSize</a>()const
</pre>

Returns minimum memory required for <a href='undocumented#Pixel'>pixel</a> storage.
Does not include unused memory on last row when <a href='#SkPixmap_rowBytesAsPixels'>rowBytesAsPixels</a>() exceeds <a href='#SkPixmap_width'>width()</a>.
Returns zero if result does not fit in size_t.
Returns zero if <a href='#SkPixmap_height'>height()</a> or <a href='#SkPixmap_width'>width()</a> is 0.
Returns <a href='#SkPixmap_height'>height()</a> times <a href='#SkPixmap_rowBytes'>rowBytes</a>() if <a href='#SkPixmap_colorType'>colorType</a>() is <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>.

### Return Value

<a href='undocumented#Size'>size</a> in bytes of <a href='SkImage_Reference#Image'>image</a> buffer

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
bool <a href='#SkPixmap_computeIsOpaque'>computeIsOpaque</a>()const
</pre>

Returns true if all pixels are opaque. <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> determines how pixels
are encoded, and whether <a href='undocumented#Pixel'>pixel</a> describes <a href='SkColor_Reference#Alpha'>alpha</a>. Returns true for <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>
without <a href='SkColor_Reference#Alpha'>alpha</a> in each <a href='undocumented#Pixel'>pixel</a>; for other <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, returns true if all
pixels have <a href='SkColor_Reference#Alpha'>alpha</a> values equivalent to 1.0 or greater.

For <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a> or <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>: always
returns true. For <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>,
<a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>: returns true if all <a href='undocumented#Pixel'>pixel</a> <a href='SkColor_Reference#Alpha'>alpha</a> values are 255.
For <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>: returns true if all <a href='undocumented#Pixel'>pixel</a> <a href='SkColor_Reference#Alpha'>alpha</a> values are 15.
For <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>: returns true if all <a href='undocumented#Pixel'>pixel</a> <a href='SkColor_Reference#Alpha'>alpha</a> values are 1.0 or
greater.

Returns false for <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>.

### Return Value

true if all pixels have opaque values or <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is opaque

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
<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkPixmap_getColor'>getColor</a>(int x, int y)const
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> at (<a href='#SkPixmap_getColor_x'>x</a>, <a href='#SkPixmap_getColor_y'>y</a>) as <a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='SkColor_Reference#Color'>color</a>.
Returns black with <a href='SkColor_Reference#Alpha'>alpha</a> if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>.

Input is not validated: out of bounds values of <a href='#SkPixmap_getColor_x'>x</a> or <a href='#SkPixmap_getColor_y'>y</a> trigger an assert() if
built with SK_DEBUG defined; and returns undefined values or may crash if
SK_RELEASE is defined. Fails if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a> or
<a href='undocumented#Pixel'>pixel</a> address is nullptr.

<a href='undocumented#SkColorSpace'>SkColorSpace</a> in <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> is ignored. Some <a href='SkColor_Reference#Color'>color</a> precision may be lost in the
conversion to <a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='SkColor_Reference#Color'>color</a>; original <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Data'>data</a> may have additional
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

<a href='undocumented#Pixel'>pixel</a> converted to <a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='SkColor_Reference#Color'>color</a>

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
float <a href='#SkPixmap_getAlphaf'>getAlphaf</a>(int x, int y)const
</pre>

Looks up the <a href='undocumented#Pixel'>pixel</a> at (<a href='#SkPixmap_getAlphaf_x'>x</a>,<a href='#SkPixmap_getAlphaf_y'>y</a>) and return its <a href='SkColor_Reference#Alpha'>alpha</a> component, normalized to [0..1].
This is roughly equivalent to <code>SkGetColorA(<a href='#SkPixmap_getColor'>getColor</a>())</code>, but can be more efficient
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

<a href='SkColor_Reference#Alpha'>alpha</a> converted to normalized float

### See Also

<a href='#SkPixmap_getColor'>getColor</a>

<a name='Readable_Address'></a>

<a name='SkPixmap_addr_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const void* <a href='#SkPixmap_addr'>addr</a>(int x, int y)const
</pre>

Returns readable <a href='undocumented#Pixel'>pixel</a> address at (<a href='#SkPixmap_addr_2_x'>x</a>, <a href='#SkPixmap_addr_2_y'>y</a>). Returns nullptr if <a href='undocumented#SkPixelRef'>SkPixelRef</a> is nullptr.

Input is not validated: out of bounds values of <a href='#SkPixmap_addr_2_x'>x</a> or <a href='#SkPixmap_addr_2_y'>y</a> trigger an assert() if
built with SK_DEBUG defined. Returns nullptr if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>.

Performs a lookup of <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Size'>size</a>; for better performance, call
one of: <a href='#SkPixmap_addr8'>addr8</a>, <a href='#SkPixmap_addr16'>addr16</a>, <a href='#SkPixmap_addr32'>addr32</a>, <a href='#SkPixmap_addr64'>addr64</a>, or <a href='#SkPixmap_addrF16'>addrF16</a>().

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
const uint8_t* <a href='#SkPixmap_addr8'>addr8</a>()const
</pre>

Returns readable base <a href='undocumented#Pixel'>pixel</a> address. Result is addressable as unsigned 8-bit bytes.
Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is not <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a> or
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, and is built with SK_DEBUG defined.

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
const uint16_t* <a href='#SkPixmap_addr16'>addr16</a>()const
</pre>

Returns readable base <a href='undocumented#Pixel'>pixel</a> address. Result is addressable as unsigned 16-bit words.
Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is not <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a> or
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, and is built with SK_DEBUG defined.

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
const uint32_t* <a href='#SkPixmap_addr32'>addr32</a>()const
</pre>

Returns readable base <a href='undocumented#Pixel'>pixel</a> address. Result is addressable as unsigned 32-bit words.
Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is not <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a> or
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, and is built with SK_DEBUG defined.

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
const uint64_t* <a href='#SkPixmap_addr64'>addr64</a>()const
</pre>

Returns readable base <a href='undocumented#Pixel'>pixel</a> address. Result is addressable as unsigned 64-bit words.
Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is not <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> and is built
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
const uint16_t* <a href='#SkPixmap_addrF16'>addrF16</a>()const
</pre>

Returns readable base <a href='undocumented#Pixel'>pixel</a> address. Result is addressable as unsigned 16-bit words.
Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is not <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> and is built
with SK_DEBUG defined.

Each word represents one <a href='SkColor_Reference#Color'>color</a> component encoded as a half float.
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
const uint8_t* <a href='#SkPixmap_addr8'>addr8</a>(int x, int y)const
</pre>

Returns readable <a href='undocumented#Pixel'>pixel</a> address at (<a href='#SkPixmap_addr8_2_x'>x</a>, <a href='#SkPixmap_addr8_2_y'>y</a>).

Input is not validated: out of bounds values of <a href='#SkPixmap_addr8_2_x'>x</a> or <a href='#SkPixmap_addr8_2_y'>y</a> trigger an assert() if
built with SK_DEBUG defined.

Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is not <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a> or
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, and is built with SK_DEBUG defined.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_addr8_2_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkPixmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_addr8_2_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkPixmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

readable unsigned 8-bit pointer to <a href='undocumented#Pixel'>pixel</a> at (<a href='#SkPixmap_addr8_2_x'>x</a>, <a href='#SkPixmap_addr8_2_y'>y</a>)

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
const uint16_t* <a href='#SkPixmap_addr16'>addr16</a>(int x, int y)const
</pre>

Returns readable <a href='undocumented#Pixel'>pixel</a> address at (<a href='#SkPixmap_addr16_2_x'>x</a>, <a href='#SkPixmap_addr16_2_y'>y</a>).

Input is not validated: out of bounds values of <a href='#SkPixmap_addr16_2_x'>x</a> or <a href='#SkPixmap_addr16_2_y'>y</a> trigger an assert() if
built with SK_DEBUG defined.

Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is not <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a> or
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, and is built with SK_DEBUG defined.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_addr16_2_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkPixmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_addr16_2_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkPixmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

readable unsigned 16-bit pointer to <a href='undocumented#Pixel'>pixel</a> at (<a href='#SkPixmap_addr16_2_x'>x</a>, <a href='#SkPixmap_addr16_2_y'>y</a>)

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
const uint32_t* <a href='#SkPixmap_addr32'>addr32</a>(int x, int y)const
</pre>

Returns readable <a href='undocumented#Pixel'>pixel</a> address at (<a href='#SkPixmap_addr32_2_x'>x</a>, <a href='#SkPixmap_addr32_2_y'>y</a>).

Input is not validated: out of bounds values of <a href='#SkPixmap_addr32_2_x'>x</a> or <a href='#SkPixmap_addr32_2_y'>y</a> trigger an assert() if
built with SK_DEBUG defined.

Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is not <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a> or
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, and is built with SK_DEBUG defined.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_addr32_2_x'><code><strong>x</strong></code></a></td>
    <td>column index, zero or greater, and less than <a href='#SkPixmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkPixmap_addr32_2_y'><code><strong>y</strong></code></a></td>
    <td>row index, zero or greater, and less than <a href='#SkPixmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

readable unsigned 32-bit pointer to <a href='undocumented#Pixel'>pixel</a> at (<a href='#SkPixmap_addr32_2_x'>x</a>, <a href='#SkPixmap_addr32_2_y'>y</a>)

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
const uint64_t* <a href='#SkPixmap_addr64'>addr64</a>(int x, int y)const
</pre>

Returns readable <a href='undocumented#Pixel'>pixel</a> address at (<a href='#SkPixmap_addr64_2_x'>x</a>, <a href='#SkPixmap_addr64_2_y'>y</a>).

Input is not validated: out of bounds values of <a href='#SkPixmap_addr64_2_x'>x</a> or <a href='#SkPixmap_addr64_2_y'>y</a> trigger an assert() if
built with SK_DEBUG defined.

Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is not <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> and is built
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

readable unsigned 64-bit pointer to <a href='undocumented#Pixel'>pixel</a> at (<a href='#SkPixmap_addr64_2_x'>x</a>, <a href='#SkPixmap_addr64_2_y'>y</a>)

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
const uint16_t* <a href='#SkPixmap_addrF16'>addrF16</a>(int x, int y)const
</pre>

Returns readable <a href='undocumented#Pixel'>pixel</a> address at (<a href='#SkPixmap_addrF16_2_x'>x</a>, <a href='#SkPixmap_addrF16_2_y'>y</a>).

Input is not validated: out of bounds values of <a href='#SkPixmap_addrF16_2_x'>x</a> or <a href='#SkPixmap_addrF16_2_y'>y</a> trigger an assert() if
built with SK_DEBUG defined.

Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is not <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> and is built
with SK_DEBUG defined.

Each unsigned 16-bit word represents one <a href='SkColor_Reference#Color'>color</a> component encoded as a half float.
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

readable unsigned 16-bit pointer to <a href='undocumented#Pixel'>pixel</a> component at (<a href='#SkPixmap_addrF16_2_x'>x</a>, <a href='#SkPixmap_addrF16_2_y'>y</a>)

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
void* <a href='#SkPixmap_writable_addr'>writable_addr</a>()const
</pre>

Returns writable base <a href='undocumented#Pixel'>pixel</a> address.

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
void* <a href='#SkPixmap_writable_addr'>writable_addr</a>(int x, int y)const
</pre>

Returns writable <a href='undocumented#Pixel'>pixel</a> address at (<a href='#SkPixmap_writable_addr_2_x'>x</a>, <a href='#SkPixmap_writable_addr_2_y'>y</a>).

Input is not validated: out of bounds values of <a href='#SkPixmap_writable_addr_2_x'>x</a> or <a href='#SkPixmap_writable_addr_2_y'>y</a> trigger an assert() if
built with SK_DEBUG defined. Returns zero if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>.

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
uint8_t* <a href='#SkPixmap_writable_addr8'>writable_addr8</a>(int x, int y)const
</pre>

Returns writable <a href='undocumented#Pixel'>pixel</a> address at (<a href='#SkPixmap_writable_addr8_x'>x</a>, <a href='#SkPixmap_writable_addr8_y'>y</a>). Result is addressable as unsigned
8-bit bytes. Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is not <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>
or <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, and is built with SK_DEBUG defined.

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

<div><fiddle-embed name="809284db136748208b3efc31cd89de29"><div>Altering pixels after drawing <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> is not guaranteed to affect subsequent
drawing on all platforms. Adding a second <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_installPixels'>installPixels</a> after editing
<a href='undocumented#Pixel'>pixel</a> memory is safer.
</div></fiddle-embed></div>

### See Also

<a href='#SkPixmap_writable_addr'>writable_addr</a> <a href='#SkPixmap_writable_addr16'>writable_addr16</a> <a href='#SkPixmap_writable_addr32'>writable_addr32</a> <a href='#SkPixmap_writable_addr64'>writable_addr64</a> <a href='#SkPixmap_writable_addrF16'>writable_addrF16</a> <a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_addr8'>addr8</a>

<a name='SkPixmap_writable_addr16'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint16_t* <a href='#SkPixmap_writable_addr16'>writable_addr16</a>(int x, int y)const
</pre>

Returns <a href='#SkPixmap_writable_addr'>writable_addr</a> <a href='undocumented#Pixel'>pixel</a> address at (<a href='#SkPixmap_writable_addr16_x'>x</a>, <a href='#SkPixmap_writable_addr16_y'>y</a>). Result is addressable as unsigned
16-bit words. Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is not <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>
or <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, and is built with SK_DEBUG defined.

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

<div><fiddle-embed name="6da54774f6432b46b47ea9013c15f280"><div>Draw a five by five <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, and draw it again with a center black <a href='undocumented#Pixel'>pixel</a>.
The low nibble of the 16-bit word is <a href='SkColor_Reference#Alpha'>Alpha</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkPixmap_writable_addr'>writable_addr</a> <a href='#SkPixmap_writable_addr8'>writable_addr8</a> <a href='#SkPixmap_writable_addr32'>writable_addr32</a> <a href='#SkPixmap_writable_addr64'>writable_addr64</a> <a href='#SkPixmap_writable_addrF16'>writable_addrF16</a> <a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_addr16'>addr16</a>

<a name='SkPixmap_writable_addr32'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t* <a href='#SkPixmap_writable_addr32'>writable_addr32</a>(int x, int y)const
</pre>

Returns writable <a href='undocumented#Pixel'>pixel</a> address at (<a href='#SkPixmap_writable_addr32_x'>x</a>, <a href='#SkPixmap_writable_addr32_y'>y</a>). Result is addressable as unsigned
32-bit words. Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is not
<a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a> or <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, and is built with SK_DEBUG
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
uint64_t* <a href='#SkPixmap_writable_addr64'>writable_addr64</a>(int x, int y)const
</pre>

Returns writable <a href='undocumented#Pixel'>pixel</a> address at (<a href='#SkPixmap_writable_addr64_x'>x</a>, <a href='#SkPixmap_writable_addr64_y'>y</a>). Result is addressable as unsigned
64-bit words. Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is not
<a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> and is built with SK_DEBUG defined.

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
uint16_t* <a href='#SkPixmap_writable_addrF16'>writable_addrF16</a>(int x, int y)const
</pre>

Returns writable <a href='undocumented#Pixel'>pixel</a> address at (<a href='#SkPixmap_writable_addrF16_x'>x</a>, <a href='#SkPixmap_writable_addrF16_y'>y</a>). Result is addressable as unsigned
16-bit words. Will trigger an assert() if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is not
<a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a> and is built with SK_DEBUG defined.

Each word represents one <a href='SkColor_Reference#Color'>color</a> component encoded as a half float.
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

<div><fiddle-embed name="7822d78f5cacf5c04267cbbc6c6d0b80"><div>Left <a href='SkBitmap_Reference#Bitmap'>bitmap</a> is drawn with two pixels defined in half float format. Right <a href='SkBitmap_Reference#Bitmap'>bitmap</a>
is drawn after overwriting bottom half float <a href='SkColor_Reference#Color'>color</a> with top half float <a href='SkColor_Reference#Color'>color</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkPixmap_writable_addr'>writable_addr</a> <a href='#SkPixmap_writable_addr8'>writable_addr8</a> <a href='#SkPixmap_writable_addr16'>writable_addr16</a> <a href='#SkPixmap_writable_addr32'>writable_addr32</a> <a href='#SkPixmap_writable_addr64'>writable_addr64</a> <a href='#SkPixmap_addr'>addr()</a> <a href='#SkPixmap_addrF16'>addrF16</a>

<a name='Pixels'></a>

<a name='SkPixmap_readPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPixmap_readPixels'>readPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& dstInfo, void* dstPixels, size_t dstRowBytes)const
</pre>

Copies a <a href='SkRect_Reference#SkRect'>SkRect</a> of pixels to <a href='#SkPixmap_readPixels_dstPixels'>dstPixels</a>. Copy starts at (0, 0), and does not
exceed <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> (<a href='#SkPixmap_width'>width()</a>, <a href='#SkPixmap_height'>height()</a>).

<a href='#SkPixmap_readPixels_dstInfo'>dstInfo</a> specifies width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, and
<a href='undocumented#SkColorSpace'>SkColorSpace</a> of destination. <a href='#SkPixmap_readPixels_dstRowBytes'>dstRowBytes</a> specifics the gap from one destination
row to the next. Returns true if pixels are copied. Returns false if
<a href='#SkPixmap_readPixels_dstInfo'>dstInfo</a> address equals nullptr, or <a href='#SkPixmap_readPixels_dstRowBytes'>dstRowBytes</a> is less than <a href='#SkPixmap_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>().

Pixels are copied only if <a href='undocumented#Pixel'>pixel</a> conversion is possible. If <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_colorType'>colorType</a>() is
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, or <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkPixmap_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorType'>colorType</a>() must match.
If <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_colorType'>colorType</a>() is <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkPixmap_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorSpace'>colorSpace</a>() must match.
If <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_alphaType'>alphaType</a>() is <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkPixmap_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() must
match. If <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_colorSpace'>colorSpace</a>() is nullptr, <a href='#SkPixmap_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorSpace'>colorSpace</a>() must match. Returns
false if <a href='undocumented#Pixel'>pixel</a> conversion is not possible.

Returns false if <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_width'>width()</a> or <a href='#SkPixmap_height'>height()</a> is zero or negative.

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
bool <a href='#SkPixmap_readPixels'>readPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& dstInfo, void* dstPixels, size_t dstRowBytes, int srcX, int srcY)const
</pre>

Copies a <a href='SkRect_Reference#Rect'>Rect</a> of pixels to <a href='#SkPixmap_readPixels_2_dstPixels'>dstPixels</a>. Copy starts at (<a href='#SkPixmap_readPixels_2_srcX'>srcX</a>, <a href='#SkPixmap_readPixels_2_srcY'>srcY</a>), and does not
exceed <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> (<a href='#SkPixmap_width'>width()</a>, <a href='#SkPixmap_height'>height()</a>).

<a href='#SkPixmap_readPixels_2_dstInfo'>dstInfo</a> specifies width, height, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, and
<a href='#Color_Space'>Color_Space</a> of destination. <a href='#SkPixmap_readPixels_2_dstRowBytes'>dstRowBytes</a> specifics the gap from one destination
row to the next. Returns true if pixels are copied. Returns false if
<a href='#SkPixmap_readPixels_2_dstInfo'>dstInfo</a> has no address, or <a href='#SkPixmap_readPixels_2_dstRowBytes'>dstRowBytes</a> is less than <a href='#SkPixmap_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>().

Pixels are copied only if <a href='undocumented#Pixel'>pixel</a> conversion is possible. If <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_colorType'>colorType</a> is
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, or <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkPixmap_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorType'>colorType</a>() must match.
If <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_colorType'>colorType</a> is <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkPixmap_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorSpace'>colorSpace</a>() must match.
If <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_alphaType'>alphaType</a> is <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkPixmap_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() must
match. If <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_colorSpace'>colorSpace</a> is nullptr, <a href='#SkPixmap_readPixels_2_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorSpace'>colorSpace</a>() must match. Returns
false if <a href='undocumented#Pixel'>pixel</a> conversion is not possible.

<a href='#SkPixmap_readPixels_2_srcX'>srcX</a> and <a href='#SkPixmap_readPixels_2_srcY'>srcY</a> may be negative to copy only top or left of source. Returns
false if <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_width'>width()</a> or <a href='#SkPixmap_height'>height()</a> is zero or negative. Returns false if:

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
bool <a href='#SkPixmap_readPixels'>readPixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& dst, int srcX, int srcY)const
</pre>

Copies a <a href='SkRect_Reference#Rect'>Rect</a> of pixels to <a href='#SkPixmap_readPixels_3_dst'>dst</a>. Copy starts at (<a href='#SkPixmap_readPixels_3_srcX'>srcX</a>, <a href='#SkPixmap_readPixels_3_srcY'>srcY</a>), and does not
exceed <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> (<a href='#SkPixmap_width'>width()</a>, <a href='#SkPixmap_height'>height()</a>). <a href='#SkPixmap_readPixels_3_dst'>dst</a> specifies width, height, <a href='#Image_Info_Color_Type'>Color_Type</a>,
<a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, and <a href='#Color_Space'>Color_Space</a> of destination.  Returns true if pixels are copied.
Returns false if <a href='#SkPixmap_readPixels_3_dst'>dst</a>.<a href='#SkPixmap_addr'>addr()</a> equals nullptr, or <a href='#SkPixmap_readPixels_3_dst'>dst</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() is less than
<a href='#SkPixmap_readPixels_3_dst'>dst</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>.

Pixels are copied only if <a href='undocumented#Pixel'>pixel</a> conversion is possible. If <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_colorType'>colorType</a> is
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, or <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkPixmap_readPixels_3_dst'>dst</a>.<a href='#SkPixmap_info'>info()</a>.<a href='#SkImageInfo_colorType'>colorType</a>() must match.
If <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_colorType'>colorType</a> is <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkPixmap_readPixels_3_dst'>dst</a>.<a href='#SkPixmap_info'>info()</a>.<a href='#SkImageInfo_colorSpace'>colorSpace</a>() must match.
If <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_alphaType'>alphaType</a> is <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkPixmap_readPixels_3_dst'>dst</a>.<a href='#SkPixmap_info'>info()</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() must
match. If <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_colorSpace'>colorSpace</a> is nullptr, <a href='#SkPixmap_readPixels_3_dst'>dst</a>.<a href='#SkPixmap_info'>info()</a>.<a href='#SkImageInfo_colorSpace'>colorSpace</a>() must match. Returns
false if <a href='undocumented#Pixel'>pixel</a> conversion is not possible.

<a href='#SkPixmap_readPixels_3_srcX'>srcX</a> and <a href='#SkPixmap_readPixels_3_srcY'>srcY</a> may be negative to copy only top or left of source. Returns
false <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_width'>width()</a> or <a href='#SkPixmap_height'>height()</a> is zero or negative. Returns false if:

<code><a href='undocumented#abs()'>abs</a>(<a href='#SkPixmap_readPixels_3_srcX'>srcX</a>) >= <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_width'>width()</a></code>, or if <code><a href='undocumented#abs()'>abs</a>(<a href='#SkPixmap_readPixels_3_srcY'>srcY</a>) >= <a href='SkPixmap_Reference#Pixmap'>Pixmap</a> <a href='#SkPixmap_height'>height()</a></code>.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_readPixels_3_dst'><code><strong>dst</strong></code></a></td>
    <td><a href='#Image_Info'>Image_Info</a> and <a href='undocumented#Pixel'>pixel</a> address to write to</td>
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
bool <a href='#SkPixmap_readPixels'>readPixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& dst)const
</pre>

Copies pixels inside <a href='#SkPixmap_bounds'>bounds()</a> to <a href='#SkPixmap_readPixels_4_dst'>dst</a>. <a href='#SkPixmap_readPixels_4_dst'>dst</a> specifies width, height, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>,
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, and <a href='undocumented#SkColorSpace'>SkColorSpace</a> of destination.  Returns true if pixels are copied.
Returns false if <a href='#SkPixmap_readPixels_4_dst'>dst</a> address equals nullptr, or <a href='#SkPixmap_readPixels_4_dst'>dst</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() is less than
<a href='#SkPixmap_readPixels_4_dst'>dst</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>.

Pixels are copied only if <a href='undocumented#Pixel'>pixel</a> conversion is possible. If <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_colorType'>colorType</a>() is
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, or <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkPixmap_readPixels_4_dst'>dst</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> must match.
If <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_colorType'>colorType</a>() is <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkPixmap_readPixels_4_dst'>dst</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> must match.
If <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_alphaType'>alphaType</a>() is <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkPixmap_readPixels_4_dst'>dst</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> must
match. If <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_colorSpace'>colorSpace</a>() is nullptr, <a href='#SkPixmap_readPixels_4_dst'>dst</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> must match. Returns
false if <a href='undocumented#Pixel'>pixel</a> conversion is not possible.

Returns false if <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_width'>width()</a> or <a href='#SkPixmap_height'>height()</a> is zero or negative.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_readPixels_4_dst'><code><strong>dst</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> and <a href='undocumented#Pixel'>pixel</a> address to write to</td>
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
bool <a href='#SkPixmap_scalePixels'>scalePixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& dst, <a href='undocumented#SkFilterQuality'>SkFilterQuality</a> filterQuality)const
</pre>

Copies <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> to <a href='#SkPixmap_scalePixels_dst'>dst</a>, scaling pixels to fit <a href='#SkPixmap_scalePixels_dst'>dst</a>.<a href='#SkPixmap_width'>width()</a> and <a href='#SkPixmap_scalePixels_dst'>dst</a>.<a href='#SkPixmap_height'>height()</a>, and
converting pixels to match <a href='#SkPixmap_scalePixels_dst'>dst</a>.<a href='#SkPixmap_colorType'>colorType</a>() and <a href='#SkPixmap_scalePixels_dst'>dst</a>.<a href='#SkPixmap_alphaType'>alphaType</a>(). Returns true if
pixels are copied. Returns false if <a href='#SkPixmap_scalePixels_dst'>dst</a> address is nullptr, or <a href='#SkPixmap_scalePixels_dst'>dst</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() is
less than <a href='#SkPixmap_scalePixels_dst'>dst</a> <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>.

Pixels are copied only if <a href='undocumented#Pixel'>pixel</a> conversion is possible. If <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_colorType'>colorType</a>() is
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, or <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkPixmap_scalePixels_dst'>dst</a> <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> must match.
If <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_colorType'>colorType</a>() is <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkPixmap_scalePixels_dst'>dst</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> must match.
If <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_alphaType'>alphaType</a>() is <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkPixmap_scalePixels_dst'>dst</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> must
match. If <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> <a href='#SkPixmap_colorSpace'>colorSpace</a>() is nullptr, <a href='#SkPixmap_scalePixels_dst'>dst</a> <a href='undocumented#SkColorSpace'>SkColorSpace</a> must match. Returns
false if <a href='undocumented#Pixel'>pixel</a> conversion is not possible.

Returns false if <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='#SkPixmap_width'>width()</a> or <a href='#SkPixmap_height'>height()</a> is zero or negative.

Scales the <a href='SkImage_Reference#Image'>image</a>, with <a href='#SkPixmap_scalePixels_filterQuality'>filterQuality</a>, to match <a href='#SkPixmap_scalePixels_dst'>dst</a>.<a href='#SkPixmap_width'>width()</a> and <a href='#SkPixmap_scalePixels_dst'>dst</a>.<a href='#SkPixmap_height'>height()</a>.
<a href='#SkPixmap_scalePixels_filterQuality'>filterQuality</a> <a href='undocumented#kNone_SkFilterQuality'>kNone_SkFilterQuality</a> is fastest, typically implemented with
<a href='undocumented#Nearest_Neighbor'>nearest neighbor filter</a>. <a href='undocumented#kLow_SkFilterQuality'>kLow_SkFilterQuality</a> is typically implemented with
<a href='undocumented#Bilerp'>bilerp filter</a>. <a href='undocumented#kMedium_SkFilterQuality'>kMedium_SkFilterQuality</a> is typically implemented with
<a href='undocumented#Bilerp'>bilerp filter</a>, and  <a href='undocumented#MipMap'>mip-map filter</a> when <a href='undocumented#Size'>size</a> is reduced.
<a href='undocumented#kHigh_SkFilterQuality'>kHigh_SkFilterQuality</a> is slowest, typically implemented with  <a href='undocumented#BiCubic'>bicubic filter</a>.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_scalePixels_dst'><code><strong>dst</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> and <a href='undocumented#Pixel'>pixel</a> address to write to</td>
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
bool <a href='#SkPixmap_erase'>erase</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#Color'>color</a>, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& subset)const
</pre>

Writes <a href='#SkPixmap_erase_color'>color</a> to pixels bounded by <a href='#SkPixmap_erase_subset'>subset</a>; returns true on success.
Returns false if <a href='#SkPixmap_colorType'>colorType</a>() is <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, or if <a href='#SkPixmap_erase_subset'>subset</a> does
not intersect <a href='#SkPixmap_bounds'>bounds()</a>.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_erase_color'><code><strong>color</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='#SkPixmap_erase_color'>color</a> to write</td>
  </tr>
  <tr>    <td><a name='SkPixmap_erase_subset'><code><strong>subset</strong></code></a></td>
    <td>bounding integer <a href='SkRect_Reference#SkRect'>SkRect</a> of written pixels</td>
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
bool <a href='#SkPixmap_erase'>erase</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#Color'>color</a>)const
</pre>

Writes <a href='#SkPixmap_erase_2_color'>color</a> to pixels inside <a href='#SkPixmap_bounds'>bounds()</a>; returns true on success.
Returns false if <a href='#SkPixmap_colorType'>colorType</a>() is <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, or if <a href='#SkPixmap_bounds'>bounds()</a>
is empty.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_erase_2_color'><code><strong>color</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='#SkPixmap_erase_2_color'>color</a> to write</td>
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
bool <a href='#SkPixmap_erase'>erase</a>(const <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a>& <a href='SkColor_Reference#Color'>color</a>, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* subset = nullptr)const
</pre>

Writes <a href='#SkPixmap_erase_3_color'>color</a> to pixels bounded by <a href='#SkPixmap_erase_3_subset'>subset</a>; returns true on success.
if <a href='#SkPixmap_erase_3_subset'>subset</a> is nullptr, writes colors pixels inside <a href='#SkPixmap_bounds'>bounds()</a>. Returns false if
<a href='#SkPixmap_colorType'>colorType</a>() is <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, if <a href='#SkPixmap_erase_3_subset'>subset</a> is not nullptr and does
not intersect <a href='#SkPixmap_bounds'>bounds()</a>, or if <a href='#SkPixmap_erase_3_subset'>subset</a> is nullptr and <a href='#SkPixmap_bounds'>bounds()</a> is empty.

### Parameters

<table>  <tr>    <td><a name='SkPixmap_erase_3_color'><code><strong>color</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='#SkPixmap_erase_3_color'>color</a> to write</td>
  </tr>
  <tr>    <td><a name='SkPixmap_erase_3_subset'><code><strong>subset</strong></code></a></td>
    <td>bounding integer <a href='SkRect_Reference#SkRect'>SkRect</a> of pixels to write; may be nullptr</td>
  </tr>
</table>

### Return Value

true if pixels are changed

### Example

<div><fiddle-embed name="f884f3f46a565f052a5e252ae2f36e9b"></fiddle-embed></div>

### See Also

<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_erase'>erase</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_clear'>clear</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawColor'>drawColor</a>

