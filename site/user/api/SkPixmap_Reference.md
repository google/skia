SkPixmap Reference
===

# <a name="Pixmap"></a> Pixmap

## <a name="Overview"></a> Overview

## <a name="Overview_Subtopic"></a> Overview Subtopic

| name | description |
| --- | --- |
| <a href="#Constructor">Constructor</a> | functions that construct <a href="#SkPixmap">SkPixmap</a> |
| <a href="#Member_Function">Member Function</a> | static functions and member methods |
| <a href="#Related_Function">Related Function</a> | similar methods grouped together |

# <a name="SkPixmap"></a> Class SkPixmap
<a href="#Pixmap">Pixmap</a> provides a utility to pair <a href="SkImageInfo_Reference#SkImageInfo">SkImageInfo</a> with pixels and row bytes.
<a href="#Pixmap">Pixmap</a> is a low level class which provides convenience functions to access
raster destinations. <a href="SkCanvas_Reference#Canvas">Canvas</a> can not draw <a href="#Pixmap">Pixmap</a>, nor does <a href="#Pixmap">Pixmap</a> provide
a direct drawing destination.

Use <a href="SkBitmap_Reference#Bitmap">Bitmap</a> to draw pixels referenced by <a href="#Pixmap">Pixmap</a>; use <a href="SkSurface_Reference#Surface">Surface</a> to draw into
pixels referenced by <a href="#Pixmap">Pixmap</a>.

<a href="#Pixmap">Pixmap</a> does not try to manage the lifetime of the pixel memory. Use <a href="undocumented#Pixel_Ref">Pixel Ref</a>
to manage pixel memory; <a href="undocumented#Pixel_Ref">Pixel Ref</a> is safe across threads.

## <a name="Related_Function"></a> Related Function

| name | description |
| --- | --- |
| <a href="#Image_Info_Access">Image Info Access</a> | returns all or part of <a href="SkImageInfo_Reference#Image_Info">Image Info</a> |
| <a href="#Initialization">Initialization</a> | sets fields for use |
| <a href="#Pixels">Pixels</a> | read and write pixel values |
| <a href="#Readable_Address">Readable Address</a> | returns read only pixels |
| <a href="#Reader">Reader</a> | examine pixel value |
| <a href="#Writable_Address">Writable Address</a> | returns writable pixels |

## <a name="Constructor"></a> Constructor

| name | description |
| --- | --- |
| <a href="#SkPixmap_empty_constructor">SkPixmap()</a> | constructs with default values |
| <a href="#SkPixmap_const_SkImageInfo_const_star">SkPixmap(const SkImageInfo& info, const void* addr, size t rowBytes)</a> | constructs from <a href="SkImageInfo_Reference#Image_Info">Image Info</a>, pixels |

## <a name="Member_Function"></a> Member Function

| name | description |
| --- | --- |
| <a href="#SkPixmap_addr">addr</a> | returns readable pixel address as void pointer |
| <a href="#SkPixmap_addr16">addr16</a> | returns readable pixel address as 16-bit pointer |
| <a href="#SkPixmap_addr32">addr32</a> | returns readable pixel address as 32-bit pointer |
| <a href="#SkPixmap_addr64">addr64</a> | returns readable pixel address as 64-bit pointer |
| <a href="#SkPixmap_addr8">addr8</a> | returns readable pixel address as 8-bit pointer |
| <a href="#SkPixmap_addrF16">addrF16</a> | returns readable pixel component address as 16-bit pointer |
| <a href="#SkPixmap_alphaType">alphaType</a> | returns <a href="SkImageInfo_Reference#Image_Info">Image Info</a> <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a> |
| <a href="#SkPixmap_bounds">bounds</a> | returns width and height as Rectangle |
| <a href="#SkPixmap_colorSpace">colorSpace</a> | returns <a href="SkImageInfo_Reference#Image_Info">Image Info</a> <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkPixmap_colorType">colorType</a> | returns <a href="SkImageInfo_Reference#Image_Info">Image Info</a> <a href="SkImageInfo_Reference#Color_Type">Color Type</a> |
| <a href="#SkPixmap_computeByteSize">computeByteSize</a> | returns size required for pixels |
| <a href="#SkPixmap_computeIsOpaque">computeIsOpaque</a> | returns true if all pixels are opaque |
| <a href="#SkPixmap_erase">erase</a> | writes <a href="undocumented#Color">Color</a> to pixels |
| <a href="#SkPixmap_extractSubset">extractSubset</a> | sets pointer to portion of original |
| <a href="#SkPixmap_getColor">getColor</a> | returns one pixel as <a href="undocumented#Unpremultiply">Unpremultiplied</a> <a href="undocumented#Color">Color</a> |
| <a href="#SkPixmap_height">height</a> | returns pixel row count |
| <a href="#SkPixmap_info">info</a> | returns <a href="SkImageInfo_Reference#Image_Info">Image Info</a> |
| <a href="#SkPixmap_isOpaque">isOpaque</a> | returns true if <a href="SkImageInfo_Reference#Image_Info">Image Info</a> describes opaque pixels |
| <a href="#SkPixmap_readPixels">readPixels</a> | copies and converts pixels |
| <a href="#SkPixmap_reset">reset</a> | reuses existing <a href="#Pixmap">Pixmap</a> with replacement values |
| <a href="#SkPixmap_rowBytes">rowBytes</a> | returns interval between rows in bytes |
| <a href="#SkPixmap_rowBytesAsPixels">rowBytesAsPixels</a> | returns interval between rows in pixels |
| <a href="#SkPixmap_scalePixels">scalePixels</a> | scales and converts pixels |
| <a href="#SkPixmap_setColorSpace">setColorSpace</a> | sets <a href="SkImageInfo_Reference#Image_Info">Image Info</a> <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkPixmap_shiftPerPixel">shiftPerPixel</a> | returns bit shift from pixels to bytes |
| <a href="#SkPixmap_width">width</a> | returns pixel column count |
| <a href="#SkPixmap_writable_addr">writable addr</a> | returns writable pixel address as void pointer |
| <a href="#SkPixmap_writable_addr16">writable addr16</a> | returns writable pixel address as 16-bit pointer |
| <a href="#SkPixmap_writable_addr32">writable addr32</a> | returns writable pixel address as 32-bit pointer |
| <a href="#SkPixmap_writable_addr64">writable addr64</a> | returns writable pixel address as 64-bit pointer |
| <a href="#SkPixmap_writable_addr8">writable addr8</a> | returns writable pixel address as 8-bit pointer |
| <a href="#SkPixmap_writable_addrF16">writable addrF16</a> | returns writable pixel component address as 16-bit pointer |

## <a name="Initialization"></a> Initialization

<a name="SkPixmap_empty_constructor"></a>
## SkPixmap

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkPixmap()
</pre>

Creates an empty <a href="#Pixmap">Pixmap</a> without pixels, with <a href="SkImageInfo_Reference#kUnknown_SkColorType">kUnknown_SkColorType</a>, with
<a href="SkImageInfo_Reference#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a>, and with a width and height of zero. Use
<a href="#SkPixmap_reset">reset</a> to associate pixels, <a href="SkImageInfo_Reference#SkColorType">SkColorType</a>, <a href="SkImageInfo_Reference#SkAlphaType">SkAlphaType</a>, width, and height
after <a href="#Pixmap">Pixmap</a> has been created.

### Return Value

empty <a href="#Pixmap">Pixmap</a>

### Example

<div><fiddle-embed name="9547e74a9d37553a667b913ffd1312dd">

#### Example Output

~~~~
width:  0  height:  0  color: kUnknown_SkColorType  alpha: kUnknown_SkAlphaType
width: 25  height: 35  color: kRGBA_8888_SkColorType  alpha: kOpaque_SkAlphaType
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkPixmap_const_SkImageInfo_const_star">SkPixmap(const SkImageInfo& info, const void* addr, size t rowBytes)</a> <a href="#SkPixmap_reset">reset</a><sup><a href="#SkPixmap_reset_2">[2]</a></sup><sup><a href="#SkPixmap_reset_3">[3]</a></sup> <a href="SkImageInfo_Reference#SkAlphaType">SkAlphaType</a> <a href="SkImageInfo_Reference#SkColorType">SkColorType</a>

---

<a name="SkPixmap_const_SkImageInfo_const_star"></a>
## SkPixmap

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkPixmap(const SkImageInfo& info, const void* addr, size_t rowBytes)
</pre>

Creates <a href="#Pixmap">Pixmap</a> from info width, height, <a href="SkImageInfo_Reference#SkAlphaType">SkAlphaType</a>, and <a href="SkImageInfo_Reference#SkColorType">SkColorType</a>.
addr points to pixels, or nullptr. <a href="#SkPixmap_rowBytes">rowBytes</a> should be info.<a href="#SkPixmap_width">width</a> times
info.bytesPerPixel(), or larger.

No parameter checking is performed; it is up to the caller to ensure that
addr and <a href="#SkPixmap_rowBytes">rowBytes</a> agree with info.

The memory lifetime of pixels is managed by the caller. When <a href="#Pixmap">Pixmap</a> goes
out of scope, addr is unaffected.

<a href="#Pixmap">Pixmap</a> may be later modified by <a href="#SkPixmap_reset">reset</a> to change its size, pixel type, or
storage.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_const_SkImageInfo_const_star_info"> <code><strong>info </strong></code> </a></td> <td>
width, height, <a href="SkImageInfo_Reference#SkAlphaType">SkAlphaType</a>, <a href="SkImageInfo_Reference#SkColorType">SkColorType</a> of <a href="SkImageInfo_Reference#Image_Info">Image Info</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_const_SkImageInfo_const_star_addr"> <code><strong>addr </strong></code> </a></td> <td>
pointer to pixels allocated by caller; may be nullptr</td>
  </tr>  <tr>    <td><a name="SkPixmap_const_SkImageInfo_const_star_rowBytes"> <code><strong>rowBytes </strong></code> </a></td> <td>
size of one row of addr; width times pixel size, or larger</td>
  </tr>
</table>

### Return Value

initialized <a href="#Pixmap">Pixmap</a>

### Example

<div><fiddle-embed name="9a00774be57d7308313b3a9073e6e696"><div><a href="SkImage_Reference#SkImage_MakeRasterCopy">SkImage::MakeRasterCopy</a> takes const <a href="#SkPixmap">SkPixmap</a>& as an argument. The example
constructs a <a href="#SkPixmap">SkPixmap</a> from the brace-delimited parameters.</div>

#### Example Output

~~~~
image alpha only = false
copy alpha only = true
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkPixmap_empty_constructor">SkPixmap()</a> <a href="#SkPixmap_reset">reset</a><sup><a href="#SkPixmap_reset_2">[2]</a></sup><sup><a href="#SkPixmap_reset_3">[3]</a></sup> <a href="SkImageInfo_Reference#SkAlphaType">SkAlphaType</a> <a href="SkImageInfo_Reference#SkColorType">SkColorType</a>

---

<a name="SkPixmap_reset"></a>
## reset

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void reset()
</pre>

Sets width, height, row bytes to zero; pixel address to nullptr; <a href="SkImageInfo_Reference#SkColorType">SkColorType</a> to
<a href="SkImageInfo_Reference#kUnknown_SkColorType">kUnknown_SkColorType</a>; and <a href="SkImageInfo_Reference#SkAlphaType">SkAlphaType</a> to <a href="SkImageInfo_Reference#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a>.

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

<a href="#SkPixmap_empty_constructor">SkPixmap()</a> <a href="SkImageInfo_Reference#SkAlphaType">SkAlphaType</a> <a href="SkImageInfo_Reference#SkColorType">SkColorType</a>

---

<a name="SkPixmap_reset_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void reset(const SkImageInfo& info, const void* addr, size_t rowBytes)
</pre>

Sets width, height, <a href="SkImageInfo_Reference#SkAlphaType">SkAlphaType</a>, and <a href="SkImageInfo_Reference#SkColorType">SkColorType</a> from info.
Sets pixel address from addr, which may be nullptr.
Sets row bytes from <a href="#SkPixmap_rowBytes">rowBytes</a>, which should be info.<a href="#SkPixmap_width">width</a> times
info.bytesPerPixel(), or larger.

Does not check addr. Asserts if built with SK_DEBUG defined and if <a href="#SkPixmap_rowBytes">rowBytes</a> is
too small to hold one row of pixels.

The memory lifetime pixels are managed by the caller. When <a href="#Pixmap">Pixmap</a> goes
out of scope, addr is unaffected.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_reset_2_info"> <code><strong>info </strong></code> </a></td> <td>
width, height, <a href="SkImageInfo_Reference#SkAlphaType">SkAlphaType</a>, <a href="SkImageInfo_Reference#SkColorType">SkColorType</a> of <a href="SkImageInfo_Reference#Image_Info">Image Info</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_reset_2_addr"> <code><strong>addr </strong></code> </a></td> <td>
pointer to pixels allocated by caller; may be nullptr</td>
  </tr>  <tr>    <td><a name="SkPixmap_reset_2_rowBytes"> <code><strong>rowBytes </strong></code> </a></td> <td>
size of one row of addr; width times pixel size, or larger</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a7c9bfe44f5d888ab5b9996f2b126788"></fiddle-embed></div>

### See Also

<a href="#SkPixmap_const_SkImageInfo_const_star">SkPixmap(const SkImageInfo& info, const void* addr, size t rowBytes)</a> <a href="#SkPixmap_reset">reset</a><sup><a href="#SkPixmap_reset_2">[2]</a></sup><sup><a href="#SkPixmap_reset_3">[3]</a></sup> <a href="SkImageInfo_Reference#SkAlphaType">SkAlphaType</a> <a href="SkImageInfo_Reference#SkColorType">SkColorType</a>

---

<a name="SkPixmap_setColorSpace"></a>
## setColorSpace

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setColorSpace(sk_sp&lt;SkColorSpace&gt; colorSpace)
</pre>

Changes <a href="undocumented#Color_Space">Color Space</a> in <a href="SkImageInfo_Reference#Image_Info">Image Info</a>; preserves width, height, <a href="SkImageInfo_Reference#SkAlphaType">SkAlphaType</a>, and
<a href="SkImageInfo_Reference#SkColorType">SkColorType</a> in <a href="SkImage_Reference#Image">Image</a>, and leaves pixel address and row bytes unchanged.
<a href="undocumented#Color_Space">Color Space</a> reference count is incremented.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_setColorSpace_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
<a href="undocumented#Color_Space">Color Space</a> moved to <a href="SkImageInfo_Reference#Image_Info">Image Info</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="bc42aea1e30b7234544bc25b4fc09dd0">

#### Example Output

~~~~
is unique
is not unique
~~~~

</fiddle-embed></div>

### See Also

<a href="undocumented#Color_Space">Color Space</a> <a href="SkImageInfo_Reference#SkImageInfo_makeColorSpace">SkImageInfo::makeColorSpace</a>

---

<a name="SkPixmap_reset_3"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool SK_WARN_UNUSED_RESULT reset(const SkMask& mask)
</pre>

soon

---

<a name="SkPixmap_extractSubset"></a>
## extractSubset

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool SK_WARN_UNUSED_RESULT extractSubset(SkPixmap* subset, const SkIRect& area) const
</pre>

Sets <a href="#SkPixmap_extractSubset_subset">subset</a> width, height, pixel address to intersection of <a href="#Pixmap">Pixmap</a> with <a href="#SkPixmap_extractSubset_area">area</a>,
if intersection is not empty; and return true. Otherwise, leave <a href="#SkPixmap_extractSubset_subset">subset</a> unchanged
and return false.

Failing to read the return value generates a compile time warning.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_extractSubset_subset"> <code><strong>subset </strong></code> </a></td> <td>
storage for width, height, pixel address of intersection</td>
  </tr>  <tr>    <td><a name="SkPixmap_extractSubset_area"> <code><strong>area </strong></code> </a></td> <td>
bounds to intersect with <a href="#Pixmap">Pixmap</a></td>
  </tr>
</table>

### Return Value

true if intersection of <a href="#Pixmap">Pixmap</a> and <a href="#SkPixmap_extractSubset_area">area</a> is not empty

### Example

<div><fiddle-embed name="febdbfac6cf4cde69837643be2e1f6dd"></fiddle-embed></div>

### See Also

<a href="#SkPixmap_reset">reset</a><sup><a href="#SkPixmap_reset_2">[2]</a></sup><sup><a href="#SkPixmap_reset_3">[3]</a></sup> <a href="SkIRect_Reference#SkIRect_intersect">SkIRect::intersect</a><sup><a href="SkIRect_Reference#SkIRect_intersect_2">[2]</a></sup><sup><a href="SkIRect_Reference#SkIRect_intersect_3">[3]</a></sup>

---

## <a name="Image_Info_Access"></a> Image Info Access

<a name="SkPixmap_info"></a>
## info

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const SkImageInfo& info() const
</pre>

Returns width, height, <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a>, <a href="SkImageInfo_Reference#Color_Type">Color Type</a>, and <a href="undocumented#Color_Space">Color Space</a>.

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

<a href="SkImageInfo_Reference#Image_Info">Image Info</a>

---

<a name="SkPixmap_rowBytes"></a>
## rowBytes

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
size_t rowBytes() const
</pre>

Returns row bytes, the interval from one pixel row to the next. Row bytes
is at least as large as:
<a href="#SkPixmap_width">width</a> * <a href="#SkPixmap_info">info</a>.bytesPerPixel().

Returns zero if <a href="#SkPixmap_colorType">colorType</a> is <a href="SkImageInfo_Reference#kUnknown_SkColorType">kUnknown_SkColorType</a>.
It is up to the <a href="SkBitmap_Reference#Bitmap">Bitmap</a> creator to ensure that row bytes is a useful value.

### Return Value

byte length of pixel row

### Example

<div><fiddle-embed name="da5e1f7f49891d3805a5a6103a000eff">

#### Example Output

~~~~
rowBytes: 2 minRowBytes: 4
rowBytes: 8 minRowBytes: 4
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkPixmap_addr">addr</a><sup><a href="#SkPixmap_addr_2">[2]</a></sup> <a href="#SkPixmap_info">info</a> <a href="SkImageInfo_Reference#SkImageInfo_minRowBytes">SkImageInfo::minRowBytes</a>

---

<a name="SkPixmap_addr"></a>
## addr

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const void* addr() const
</pre>

Returns pixel address, the base address corresponding to the pixel origin.

It is up to the <a href="#Pixmap">Pixmap</a> creator to ensure that pixel address is a useful value.

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

<a href="#SkPixmap_addr_2">addr(int x, int y)</a> <a href="#SkPixmap_addr8">addr8</a><sup><a href="#SkPixmap_addr8_2">[2]</a></sup> <a href="#SkPixmap_addr16">addr16</a><sup><a href="#SkPixmap_addr16_2">[2]</a></sup> <a href="#SkPixmap_addr32">addr32</a><sup><a href="#SkPixmap_addr32_2">[2]</a></sup> <a href="#SkPixmap_addr64">addr64</a><sup><a href="#SkPixmap_addr64_2">[2]</a></sup> <a href="#SkPixmap_info">info</a> <a href="#SkPixmap_rowBytes">rowBytes</a>

---

<a name="SkPixmap_width"></a>
## width

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int width() const
</pre>

Returns pixel count in each pixel row. Should be equal or less than:

<a href="#SkPixmap_rowBytes">rowBytes</a> / <a href="#SkPixmap_info">info</a>.bytesPerPixel().

### Return Value

pixel width in <a href="SkImageInfo_Reference#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="f68617b7153a20b2ed3d7f9ed5c6e5e4">

#### Example Output

~~~~
pixmap width: 16  info width: 16
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkPixmap_height">height</a> <a href="SkImageInfo_Reference#SkImageInfo_width">SkImageInfo::width()</a>

---

<a name="SkPixmap_height"></a>
## height

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int height() const
</pre>

Returns pixel row count.

### Return Value

pixel height in <a href="SkImageInfo_Reference#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="4a996d32122f469d51ddd0186efb48cc">

#### Example Output

~~~~
pixmap height: 32  info height: 32
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkPixmap_width">width</a> ImageInfo::height()

---

<a name="SkPixmap_colorType"></a>
## colorType

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkColorType colorType() const
</pre>

Returns <a href="SkImageInfo_Reference#Color_Type">Color Type</a>, one of: <a href="SkImageInfo_Reference#kUnknown_SkColorType">kUnknown_SkColorType</a>, <a href="SkImageInfo_Reference#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>,
<a href="SkImageInfo_Reference#kRGB_565_SkColorType">kRGB_565_SkColorType</a>, <a href="SkImageInfo_Reference#kARGB_4444_SkColorType">kARGB_4444_SkColorType</a>, <a href="SkImageInfo_Reference#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a>,
<a href="SkImageInfo_Reference#kRGB_888x_SkColorType">kRGB_888x_SkColorType</a>, <a href="SkImageInfo_Reference#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>, <a href="SkImageInfo_Reference#kRGBA_1010102_SkColorType">kRGBA_1010102_SkColorType</a>,
<a href="SkImageInfo_Reference#kRGB_101010x_SkColorType">kRGB_101010x_SkColorType</a>, <a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, <a href="SkImageInfo_Reference#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a>.

### Return Value

<a href="SkImageInfo_Reference#Color_Type">Color Type</a> in <a href="SkImageInfo_Reference#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="0ab5c7af272685f2ce177cc79e6b9457">

#### Example Output

~~~~
color type: kAlpha_8_SkColorType
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkPixmap_alphaType">alphaType</a> <a href="SkImageInfo_Reference#SkImageInfo_colorType">SkImageInfo::colorType</a>

---

<a name="SkPixmap_alphaType"></a>
## alphaType

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkAlphaType alphaType() const
</pre>

Returns <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a>, one of: <a href="SkImageInfo_Reference#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a>, <a href="SkImageInfo_Reference#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>,
<a href="SkImageInfo_Reference#kPremul_SkAlphaType">kPremul_SkAlphaType</a>, <a href="SkImageInfo_Reference#kUnpremul_SkAlphaType">kUnpremul_SkAlphaType</a>.

### Return Value

<a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a> in <a href="SkImageInfo_Reference#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="070b1a60232be499eb10c6ea62371804">

#### Example Output

~~~~
alpha type: kPremul_SkAlphaType
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkPixmap_colorType">colorType</a> <a href="SkImageInfo_Reference#SkImageInfo_alphaType">SkImageInfo::alphaType</a>

---

<a name="SkPixmap_colorSpace"></a>
## colorSpace

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkColorSpace* colorSpace() const
</pre>

Returns <a href="undocumented#Color_Space">Color Space</a> associated with <a href="SkImageInfo_Reference#Image_Info">Image Info</a>. The
reference count of <a href="undocumented#Color_Space">Color Space</a> is unchanged. The returned <a href="undocumented#Color_Space">Color Space</a> is
immutable.

### Return Value

<a href="undocumented#Color_Space">Color Space</a>, the range of colors, in <a href="SkImageInfo_Reference#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="34c71f803b8edb48eaf1cd0c55bb212e"><div><a href="undocumented#SkColorSpace_MakeSRGBLinear">SkColorSpace::MakeSRGBLinear</a> creates <a href="undocumented#Color_Space">Color Space</a> with linear gamma
and an sRGB gamut. This <a href="undocumented#Color_Space">Color Space</a> gamma is not close to sRGB gamma.</div>

#### Example Output

~~~~
gammaCloseToSRGB: false  gammaIsLinear: true  isSRGB: false
~~~~

</fiddle-embed></div>

### See Also

<a href="undocumented#Color_Space">Color Space</a> <a href="SkImageInfo_Reference#SkImageInfo_colorSpace">SkImageInfo::colorSpace</a>

---

<a name="SkPixmap_isOpaque"></a>
## isOpaque

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isOpaque() const
</pre>

Returns true if <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a> is <a href="SkImageInfo_Reference#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>.
Does not check if <a href="SkImageInfo_Reference#Color_Type">Color Type</a> allows <a href="undocumented#Alpha">Alpha</a>, or if any pixel value has
transparency.

### Return Value

true if <a href="SkImageInfo_Reference#Image_Info">Image Info</a> has opaque <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a>

### Example

<div><fiddle-embed name="efd083f121e888a523455ea8a49e50d1"><div><a href="#SkPixmap_isOpaque">isOpaque</a> ignores whether all pixels are opaque or not.</div>

#### Example Output

~~~~
isOpaque: false
isOpaque: false
isOpaque: true
isOpaque: true
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkPixmap_computeIsOpaque">computeIsOpaque</a> <a href="SkImageInfo_Reference#SkImageInfo_isOpaque">SkImageInfo::isOpaque</a>

---

<a name="SkPixmap_bounds"></a>
## bounds

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkIRect bounds() const
</pre>

Returns <a href="SkIRect_Reference#IRect">IRect</a> { 0, 0, <a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a> }.

### Return Value

integral rectangle from origin to <a href="#SkPixmap_width">width</a> and <a href="#SkPixmap_height">height</a>

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

<a href="#SkPixmap_height">height</a> <a href="#SkPixmap_width">width</a> <a href="SkIRect_Reference#IRect">IRect</a>

---

<a name="SkPixmap_rowBytesAsPixels"></a>
## rowBytesAsPixels

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int rowBytesAsPixels() const
</pre>

Returns number of pixels that fit on row. Should be greater than or equal to
<a href="#SkPixmap_width">width</a>.

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

<a href="#SkPixmap_rowBytes">rowBytes</a> <a href="#SkPixmap_shiftPerPixel">shiftPerPixel</a> <a href="#SkPixmap_width">width</a> <a href="SkImageInfo_Reference#SkImageInfo_bytesPerPixel">SkImageInfo::bytesPerPixel</a>

---

<a name="SkPixmap_shiftPerPixel"></a>
## shiftPerPixel

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int shiftPerPixel() const
</pre>

Returns bit shift converting row bytes to row pixels.
Returns zero for <a href="SkImageInfo_Reference#kUnknown_SkColorType">kUnknown_SkColorType</a>.

### Return Value

one of: 0, 1, 2, 3; left shift to convert pixels to bytes

### Example

<div><fiddle-embed name="2e778ffd6edea51af4b07f5d322ceb6a">

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

<a href="#SkPixmap_rowBytes">rowBytes</a> <a href="#SkPixmap_rowBytesAsPixels">rowBytesAsPixels</a> <a href="#SkPixmap_width">width</a> <a href="SkImageInfo_Reference#SkImageInfo_bytesPerPixel">SkImageInfo::bytesPerPixel</a>

---

<a name="SkPixmap_computeByteSize"></a>
## computeByteSize

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
size_t computeByteSize() const
</pre>

Returns minimum memory required for pixel storage.
Does not include unused memory on last row when <a href="#SkPixmap_rowBytesAsPixels">rowBytesAsPixels</a> exceeds <a href="#SkPixmap_width">width</a>.
Returns zero if result does not fit in size_t.
Returns zero if <a href="#SkPixmap_height">height</a> or <a href="#SkPixmap_width">width</a> is 0.
Returns <a href="#SkPixmap_height">height</a> times <a href="#SkPixmap_rowBytes">rowBytes</a> if <a href="#SkPixmap_colorType">colorType</a> is <a href="SkImageInfo_Reference#kUnknown_SkColorType">kUnknown_SkColorType</a>.

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

<a href="SkImageInfo_Reference#SkImageInfo_computeByteSize">SkImageInfo::computeByteSize</a>

---

## <a name="Reader"></a> Reader

<a name="SkPixmap_computeIsOpaque"></a>
## computeIsOpaque

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool computeIsOpaque() const
</pre>

Returns true if all pixels are opaque. <a href="SkImageInfo_Reference#Color_Type">Color Type</a> determines how pixels
are encoded, and whether pixel describes <a href="undocumented#Alpha">Alpha</a>. Returns true for <a href="SkImageInfo_Reference#Color_Type">Color Types</a>
without alpha in each pixel; for other <a href="SkImageInfo_Reference#Color_Type">Color Types</a>, returns true if all
pixels have alpha values equivalent to 1.0 or greater.

For <a href="SkImageInfo_Reference#Color_Type">Color Types</a> <a href="SkImageInfo_Reference#kRGB_565_SkColorType">kRGB_565_SkColorType</a> or <a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>: always
returns true. For <a href="SkImageInfo_Reference#Color_Type">Color Types</a> <a href="SkImageInfo_Reference#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>, <a href="SkImageInfo_Reference#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>,
<a href="SkImageInfo_Reference#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a>: returns true if all pixel <a href="undocumented#Alpha">Alpha</a> values are 255.
For <a href="SkImageInfo_Reference#Color_Type">Color Type</a> <a href="SkImageInfo_Reference#kARGB_4444_SkColorType">kARGB_4444_SkColorType</a>: returns true if all pixel <a href="undocumented#Alpha">Alpha</a> values are 15.
For <a href="SkImageInfo_Reference#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a>: returns true if all pixel <a href="undocumented#Alpha">Alpha</a> values are 1.0 or
greater.

Returns false for <a href="SkImageInfo_Reference#kUnknown_SkColorType">kUnknown_SkColorType</a>.

### Return Value

true if all pixels have opaque values or <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is opaque

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

<a href="#SkPixmap_isOpaque">isOpaque</a> <a href="SkImageInfo_Reference#Color_Type">Color Type</a> <a href="undocumented#Alpha">Alpha</a>

---

<a name="SkPixmap_getColor"></a>
## getColor

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkColor getColor(int x, int y) const
</pre>

Returns pixel at (<a href="#SkPixmap_getColor_x">x</a>, <a href="#SkPixmap_getColor_y">y</a>) as <a href="undocumented#Unpremultiply">Unpremultiplied</a> <a href="undocumented#Color">Color</a>.
Returns black with <a href="undocumented#Alpha">Alpha</a> if <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is <a href="SkImageInfo_Reference#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>.

Input is not validated: out of bounds values of <a href="#SkPixmap_getColor_x">x</a> or <a href="#SkPixmap_getColor_y">y</a> trigger an assert() if
built with SK_DEBUG defined; and returns undefined values or may crash if
SK_RELEASE is defined. Fails if <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is <a href="SkImageInfo_Reference#kUnknown_SkColorType">kUnknown_SkColorType</a> or
pixel address is nullptr.

<a href="undocumented#Color_Space">Color Space</a> in <a href="SkImageInfo_Reference#Image_Info">Image Info</a> is ignored. Some <a href="undocumented#Color">Color</a> precision may be lost in the
conversion to <a href="undocumented#Unpremultiply">Unpremultiplied</a> <a href="undocumented#Color">Color</a>; original pixel data may have additional
precision.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_getColor_x"> <code><strong>x </strong></code> </a></td> <td>
column index, zero or greater, and less than <a href="#SkPixmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_getColor_y"> <code><strong>y </strong></code> </a></td> <td>
row index, zero or greater, and less than <a href="#SkPixmap_height">height</a></td>
  </tr>
</table>

### Return Value

pixel converted to <a href="undocumented#Unpremultiply">Unpremultiplied</a> <a href="undocumented#Color">Color</a>

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

<a href="#SkPixmap_addr">addr</a><sup><a href="#SkPixmap_addr_2">[2]</a></sup> <a href="#SkPixmap_readPixels">readPixels</a><sup><a href="#SkPixmap_readPixels_2">[2]</a></sup><sup><a href="#SkPixmap_readPixels_3">[3]</a></sup><sup><a href="#SkPixmap_readPixels_4">[4]</a></sup><sup><a href="#SkPixmap_readPixels_5">[5]</a></sup>

---

## <a name="Readable_Address"></a> Readable Address

<a name="SkPixmap_addr_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const void* addr(int x, int y) const
</pre>

Returns readable pixel address at (<a href="#SkPixmap_addr_2_x">x</a>, <a href="#SkPixmap_addr_2_y">y</a>). Returns nullptr if <a href="undocumented#Pixel_Ref">Pixel Ref</a> is nullptr.

Input is not validated: out of bounds values of <a href="#SkPixmap_addr_2_x">x</a> or <a href="#SkPixmap_addr_2_y">y</a> trigger an assert() if
built with SK_DEBUG defined. Returns nullptr if <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is <a href="SkImageInfo_Reference#kUnknown_SkColorType">kUnknown_SkColorType</a>.

Performs a lookup of pixel size; for better performance, call
one of: <a href="#SkPixmap_addr8">addr8</a>, <a href="#SkPixmap_addr16">addr16</a>, <a href="#SkPixmap_addr32">addr32</a>, <a href="#SkPixmap_addr64">addr64</a>, or <a href="#SkPixmap_addrF16">addrF16</a>.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_addr_2_x"> <code><strong>x </strong></code> </a></td> <td>
column index, zero or greater, and less than <a href="#SkPixmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_addr_2_y"> <code><strong>y </strong></code> </a></td> <td>
row index, zero or greater, and less than <a href="#SkPixmap_height">height</a></td>
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

<a href="#SkPixmap_addr8">addr8</a><sup><a href="#SkPixmap_addr8_2">[2]</a></sup> <a href="#SkPixmap_addr16">addr16</a><sup><a href="#SkPixmap_addr16_2">[2]</a></sup> <a href="#SkPixmap_addr32">addr32</a><sup><a href="#SkPixmap_addr32_2">[2]</a></sup> <a href="#SkPixmap_addr64">addr64</a><sup><a href="#SkPixmap_addr64_2">[2]</a></sup> <a href="#SkPixmap_addrF16">addrF16</a><sup><a href="#SkPixmap_addrF16_2">[2]</a></sup> <a href="#SkPixmap_getColor">getColor</a> <a href="#SkPixmap_writable_addr">writable addr</a><sup><a href="#SkPixmap_writable_addr_2">[2]</a></sup> <a href="SkBitmap_Reference#SkBitmap_getAddr">SkBitmap::getAddr</a>

---

<a name="SkPixmap_addr8"></a>
## addr8

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const uint8_t* addr8() const
</pre>

Returns readable base pixel address. Result is addressable as unsigned 8-bit bytes.
Will trigger an assert() if <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is not <a href="SkImageInfo_Reference#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a> or
<a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, and is built with SK_DEBUG defined.

One byte corresponds to one pixel.

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

<a href="#SkPixmap_addr">addr</a><sup><a href="#SkPixmap_addr_2">[2]</a></sup> <a href="#SkPixmap_addr16">addr16</a><sup><a href="#SkPixmap_addr16_2">[2]</a></sup> <a href="#SkPixmap_addr32">addr32</a><sup><a href="#SkPixmap_addr32_2">[2]</a></sup> <a href="#SkPixmap_addr64">addr64</a><sup><a href="#SkPixmap_addr64_2">[2]</a></sup> <a href="#SkPixmap_addrF16">addrF16</a><sup><a href="#SkPixmap_addrF16_2">[2]</a></sup> <a href="#SkPixmap_getColor">getColor</a> <a href="#SkPixmap_writable_addr">writable addr</a><sup><a href="#SkPixmap_writable_addr_2">[2]</a></sup> <a href="#SkPixmap_writable_addr8">writable addr8</a>

---

<a name="SkPixmap_addr16"></a>
## addr16

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const uint16_t* addr16() const
</pre>

Returns readable base pixel address. Result is addressable as unsigned 16-bit words.
Will trigger an assert() if <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is not <a href="SkImageInfo_Reference#kRGB_565_SkColorType">kRGB_565_SkColorType</a> or
<a href="SkImageInfo_Reference#kARGB_4444_SkColorType">kARGB_4444_SkColorType</a>, and is built with SK_DEBUG defined.

One word corresponds to one pixel.

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

<a href="#SkPixmap_addr">addr</a><sup><a href="#SkPixmap_addr_2">[2]</a></sup> <a href="#SkPixmap_addr8">addr8</a><sup><a href="#SkPixmap_addr8_2">[2]</a></sup> <a href="#SkPixmap_addr32">addr32</a><sup><a href="#SkPixmap_addr32_2">[2]</a></sup> <a href="#SkPixmap_addr64">addr64</a><sup><a href="#SkPixmap_addr64_2">[2]</a></sup> <a href="#SkPixmap_addrF16">addrF16</a><sup><a href="#SkPixmap_addrF16_2">[2]</a></sup> <a href="#SkPixmap_getColor">getColor</a> <a href="#SkPixmap_writable_addr">writable addr</a><sup><a href="#SkPixmap_writable_addr_2">[2]</a></sup> <a href="#SkPixmap_writable_addr16">writable addr16</a>

---

<a name="SkPixmap_addr32"></a>
## addr32

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const uint32_t* addr32() const
</pre>

Returns readable base pixel address. Result is addressable as unsigned 32-bit words.
Will trigger an assert() if <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is not <a href="SkImageInfo_Reference#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a> or
<a href="SkImageInfo_Reference#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>, and is built with SK_DEBUG defined.

One word corresponds to one pixel.

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

<a href="#SkPixmap_addr">addr</a><sup><a href="#SkPixmap_addr_2">[2]</a></sup> <a href="#SkPixmap_addr8">addr8</a><sup><a href="#SkPixmap_addr8_2">[2]</a></sup> <a href="#SkPixmap_addr16">addr16</a><sup><a href="#SkPixmap_addr16_2">[2]</a></sup> <a href="#SkPixmap_addr64">addr64</a><sup><a href="#SkPixmap_addr64_2">[2]</a></sup> <a href="#SkPixmap_addrF16">addrF16</a><sup><a href="#SkPixmap_addrF16_2">[2]</a></sup> <a href="#SkPixmap_getColor">getColor</a> <a href="#SkPixmap_writable_addr">writable addr</a><sup><a href="#SkPixmap_writable_addr_2">[2]</a></sup> <a href="#SkPixmap_writable_addr32">writable addr32</a>

---

<a name="SkPixmap_addr64"></a>
## addr64

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const uint64_t* addr64() const
</pre>

Returns readable base pixel address. Result is addressable as unsigned 64-bit words.
Will trigger an assert() if <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is not <a href="SkImageInfo_Reference#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a> and is built
with SK_DEBUG defined.

One word corresponds to one pixel.

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

<a href="#SkPixmap_addr">addr</a><sup><a href="#SkPixmap_addr_2">[2]</a></sup> <a href="#SkPixmap_addr8">addr8</a><sup><a href="#SkPixmap_addr8_2">[2]</a></sup> <a href="#SkPixmap_addr16">addr16</a><sup><a href="#SkPixmap_addr16_2">[2]</a></sup> <a href="#SkPixmap_addr32">addr32</a><sup><a href="#SkPixmap_addr32_2">[2]</a></sup> <a href="#SkPixmap_addrF16">addrF16</a><sup><a href="#SkPixmap_addrF16_2">[2]</a></sup> <a href="#SkPixmap_getColor">getColor</a> <a href="#SkPixmap_writable_addr">writable addr</a><sup><a href="#SkPixmap_writable_addr_2">[2]</a></sup> <a href="#SkPixmap_writable_addr64">writable addr64</a>

---

<a name="SkPixmap_addrF16"></a>
## addrF16

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const uint16_t* addrF16() const
</pre>

Returns readable base pixel address. Result is addressable as unsigned 16-bit words.
Will trigger an assert() if <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is not <a href="SkImageInfo_Reference#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a> and is built
with SK_DEBUG defined.

Each word represents one color component encoded as a half float.
Four words correspond to one pixel.

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

<a href="#SkPixmap_addr">addr</a><sup><a href="#SkPixmap_addr_2">[2]</a></sup> <a href="#SkPixmap_addr8">addr8</a><sup><a href="#SkPixmap_addr8_2">[2]</a></sup> <a href="#SkPixmap_addr16">addr16</a><sup><a href="#SkPixmap_addr16_2">[2]</a></sup> <a href="#SkPixmap_addr32">addr32</a><sup><a href="#SkPixmap_addr32_2">[2]</a></sup> <a href="#SkPixmap_addr64">addr64</a><sup><a href="#SkPixmap_addr64_2">[2]</a></sup> <a href="#SkPixmap_getColor">getColor</a> <a href="#SkPixmap_writable_addr">writable addr</a><sup><a href="#SkPixmap_writable_addr_2">[2]</a></sup> <a href="#SkPixmap_writable_addrF16">writable addrF16</a>

---

<a name="SkPixmap_addr8_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const uint8_t* addr8(int x, int y) const
</pre>

Returns readable pixel address at (<a href="#SkPixmap_addr8_2_x">x</a>, <a href="#SkPixmap_addr8_2_y">y</a>).

Input is not validated: out of bounds values of <a href="#SkPixmap_addr8_2_x">x</a> or <a href="#SkPixmap_addr8_2_y">y</a> trigger an assert() if
built with SK_DEBUG defined.

Will trigger an assert() if <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is not <a href="SkImageInfo_Reference#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a> or
<a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, and is built with SK_DEBUG defined.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_addr8_2_x"> <code><strong>x </strong></code> </a></td> <td>
column index, zero or greater, and less than <a href="#SkPixmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_addr8_2_y"> <code><strong>y </strong></code> </a></td> <td>
row index, zero or greater, and less than <a href="#SkPixmap_height">height</a></td>
  </tr>
</table>

### Return Value

readable unsigned 8-bit pointer to pixel at (<a href="#SkPixmap_addr8_2_x">x</a>, <a href="#SkPixmap_addr8_2_y">y</a>)

### Example

<div><fiddle-embed name="5b986272268ef2c52045c1856f8b6107">

#### Example Output

~~~~
pixmap.addr8(1, 2) == &storage[1 + 2 * w]
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkPixmap_addr">addr</a><sup><a href="#SkPixmap_addr_2">[2]</a></sup> <a href="#SkPixmap_addr16">addr16</a><sup><a href="#SkPixmap_addr16_2">[2]</a></sup> <a href="#SkPixmap_addr32">addr32</a><sup><a href="#SkPixmap_addr32_2">[2]</a></sup> <a href="#SkPixmap_addr64">addr64</a><sup><a href="#SkPixmap_addr64_2">[2]</a></sup> <a href="#SkPixmap_addrF16">addrF16</a><sup><a href="#SkPixmap_addrF16_2">[2]</a></sup> <a href="#SkPixmap_getColor">getColor</a> <a href="#SkPixmap_writable_addr">writable addr</a><sup><a href="#SkPixmap_writable_addr_2">[2]</a></sup> <a href="#SkPixmap_writable_addr8">writable addr8</a>

---

<a name="SkPixmap_addr16_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const uint16_t* addr16(int x, int y) const
</pre>

Returns readable pixel address at (<a href="#SkPixmap_addr16_2_x">x</a>, <a href="#SkPixmap_addr16_2_y">y</a>).

Input is not validated: out of bounds values of <a href="#SkPixmap_addr16_2_x">x</a> or <a href="#SkPixmap_addr16_2_y">y</a> trigger an assert() if
built with SK_DEBUG defined.

Will trigger an assert() if <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is not <a href="SkImageInfo_Reference#kRGB_565_SkColorType">kRGB_565_SkColorType</a> or
<a href="SkImageInfo_Reference#kARGB_4444_SkColorType">kARGB_4444_SkColorType</a>, and is built with SK_DEBUG defined.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_addr16_2_x"> <code><strong>x </strong></code> </a></td> <td>
column index, zero or greater, and less than <a href="#SkPixmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_addr16_2_y"> <code><strong>y </strong></code> </a></td> <td>
row index, zero or greater, and less than <a href="#SkPixmap_height">height</a></td>
  </tr>
</table>

### Return Value

readable unsigned 16-bit pointer to pixel at (<a href="#SkPixmap_addr16_2_x">x</a>, <a href="#SkPixmap_addr16_2_y">y</a>)

### Example

<div><fiddle-embed name="2c0c88a546d4ef093ab63ff72dac00b9">

#### Example Output

~~~~
pixmap.addr16(1, 2) == &storage[1 + 2 * w]
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkPixmap_addr">addr</a><sup><a href="#SkPixmap_addr_2">[2]</a></sup> <a href="#SkPixmap_addr8">addr8</a><sup><a href="#SkPixmap_addr8_2">[2]</a></sup> <a href="#SkPixmap_addr32">addr32</a><sup><a href="#SkPixmap_addr32_2">[2]</a></sup> <a href="#SkPixmap_addr64">addr64</a><sup><a href="#SkPixmap_addr64_2">[2]</a></sup> <a href="#SkPixmap_addrF16">addrF16</a><sup><a href="#SkPixmap_addrF16_2">[2]</a></sup> <a href="#SkPixmap_getColor">getColor</a> <a href="#SkPixmap_writable_addr">writable addr</a><sup><a href="#SkPixmap_writable_addr_2">[2]</a></sup> <a href="#SkPixmap_writable_addr16">writable addr16</a>

---

<a name="SkPixmap_addr32_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const uint32_t* addr32(int x, int y) const
</pre>

Returns readable pixel address at (<a href="#SkPixmap_addr32_2_x">x</a>, <a href="#SkPixmap_addr32_2_y">y</a>).

Input is not validated: out of bounds values of <a href="#SkPixmap_addr32_2_x">x</a> or <a href="#SkPixmap_addr32_2_y">y</a> trigger an assert() if
built with SK_DEBUG defined.

Will trigger an assert() if <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is not <a href="SkImageInfo_Reference#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a> or
<a href="SkImageInfo_Reference#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>, and is built with SK_DEBUG defined.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_addr32_2_x"> <code><strong>x </strong></code> </a></td> <td>
column index, zero or greater, and less than <a href="#SkPixmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_addr32_2_y"> <code><strong>y </strong></code> </a></td> <td>
row index, zero or greater, and less than <a href="#SkPixmap_height">height</a></td>
  </tr>
</table>

### Return Value

readable unsigned 32-bit pointer to pixel at (<a href="#SkPixmap_addr32_2_x">x</a>, <a href="#SkPixmap_addr32_2_y">y</a>)

### Example

<div><fiddle-embed name="12f8b5ce9fb25604f33df336677f5d62">

#### Example Output

~~~~
pixmap.addr32(1, 2) == &storage[1 + 2 * w]
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkPixmap_addr">addr</a><sup><a href="#SkPixmap_addr_2">[2]</a></sup> <a href="#SkPixmap_addr8">addr8</a><sup><a href="#SkPixmap_addr8_2">[2]</a></sup> <a href="#SkPixmap_addr16">addr16</a><sup><a href="#SkPixmap_addr16_2">[2]</a></sup> <a href="#SkPixmap_addr64">addr64</a><sup><a href="#SkPixmap_addr64_2">[2]</a></sup> <a href="#SkPixmap_addrF16">addrF16</a><sup><a href="#SkPixmap_addrF16_2">[2]</a></sup> <a href="#SkPixmap_getColor">getColor</a> <a href="#SkPixmap_writable_addr">writable addr</a><sup><a href="#SkPixmap_writable_addr_2">[2]</a></sup> <a href="#SkPixmap_writable_addr64">writable addr64</a>

---

<a name="SkPixmap_addr64_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const uint64_t* addr64(int x, int y) const
</pre>

Returns readable pixel address at (<a href="#SkPixmap_addr64_2_x">x</a>, <a href="#SkPixmap_addr64_2_y">y</a>).

Input is not validated: out of bounds values of <a href="#SkPixmap_addr64_2_x">x</a> or <a href="#SkPixmap_addr64_2_y">y</a> trigger an assert() if
built with SK_DEBUG defined.

Will trigger an assert() if <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is not <a href="SkImageInfo_Reference#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a> and is built
with SK_DEBUG defined.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_addr64_2_x"> <code><strong>x </strong></code> </a></td> <td>
column index, zero or greater, and less than <a href="#SkPixmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_addr64_2_y"> <code><strong>y </strong></code> </a></td> <td>
row index, zero or greater, and less than <a href="#SkPixmap_height">height</a></td>
  </tr>
</table>

### Return Value

readable unsigned 64-bit pointer to pixel at (<a href="#SkPixmap_addr64_2_x">x</a>, <a href="#SkPixmap_addr64_2_y">y</a>)

### Example

<div><fiddle-embed name="5449f65fd7673273b0b57807fd3117ff">

#### Example Output

~~~~
pixmap.addr64(1, 2) == &storage[1 + 2 * w]
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkPixmap_addr">addr</a><sup><a href="#SkPixmap_addr_2">[2]</a></sup> <a href="#SkPixmap_addr8">addr8</a><sup><a href="#SkPixmap_addr8_2">[2]</a></sup> <a href="#SkPixmap_addr16">addr16</a><sup><a href="#SkPixmap_addr16_2">[2]</a></sup> <a href="#SkPixmap_addr32">addr32</a><sup><a href="#SkPixmap_addr32_2">[2]</a></sup> <a href="#SkPixmap_addrF16">addrF16</a><sup><a href="#SkPixmap_addrF16_2">[2]</a></sup> <a href="#SkPixmap_getColor">getColor</a> <a href="#SkPixmap_writable_addr">writable addr</a><sup><a href="#SkPixmap_writable_addr_2">[2]</a></sup> <a href="#SkPixmap_writable_addr64">writable addr64</a>

---

<a name="SkPixmap_addrF16_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const uint16_t* addrF16(int x, int y) const
</pre>

Returns readable pixel address at (<a href="#SkPixmap_addrF16_2_x">x</a>, <a href="#SkPixmap_addrF16_2_y">y</a>).

Input is not validated: out of bounds values of <a href="#SkPixmap_addrF16_2_x">x</a> or <a href="#SkPixmap_addrF16_2_y">y</a> trigger an assert() if
built with SK_DEBUG defined.

Will trigger an assert() if <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is not <a href="SkImageInfo_Reference#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a> and is built
with SK_DEBUG defined.

Each unsigned 16-bit word represents one color component encoded as a half float.
Four words correspond to one pixel.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_addrF16_2_x"> <code><strong>x </strong></code> </a></td> <td>
column index, zero or greater, and less than <a href="#SkPixmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_addrF16_2_y"> <code><strong>y </strong></code> </a></td> <td>
row index, zero or greater, and less than <a href="#SkPixmap_height">height</a></td>
  </tr>
</table>

### Return Value

readable unsigned 16-bit pointer to pixel component at (<a href="#SkPixmap_addrF16_2_x">x</a>, <a href="#SkPixmap_addrF16_2_y">y</a>)

### Example

<div><fiddle-embed name="f6076cad455bc80af5d06eb121d3b6f2">

#### Example Output

~~~~
pixmap.addrF16(1, 2) == &storage[1 * wordsPerPixel + 2 * rowWords]
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkPixmap_addr">addr</a><sup><a href="#SkPixmap_addr_2">[2]</a></sup> <a href="#SkPixmap_addr8">addr8</a><sup><a href="#SkPixmap_addr8_2">[2]</a></sup> <a href="#SkPixmap_addr16">addr16</a><sup><a href="#SkPixmap_addr16_2">[2]</a></sup> <a href="#SkPixmap_addr32">addr32</a><sup><a href="#SkPixmap_addr32_2">[2]</a></sup> <a href="#SkPixmap_addr64">addr64</a><sup><a href="#SkPixmap_addr64_2">[2]</a></sup> <a href="#SkPixmap_getColor">getColor</a> <a href="#SkPixmap_writable_addr">writable addr</a><sup><a href="#SkPixmap_writable_addr_2">[2]</a></sup> <a href="#SkPixmap_writable_addrF16">writable addrF16</a>

---

## <a name="Writable_Address"></a> Writable Address

<a name="SkPixmap_writable_addr"></a>
## writable_addr

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void* writable_addr() const
</pre>

Returns writable base pixel address.

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

<a href="#SkPixmap_writable_addr8">writable addr8</a> <a href="#SkPixmap_writable_addr16">writable addr16</a> <a href="#SkPixmap_writable_addr32">writable addr32</a> <a href="#SkPixmap_writable_addr64">writable addr64</a> <a href="#SkPixmap_writable_addrF16">writable addrF16</a> <a href="#SkPixmap_addr">addr</a><sup><a href="#SkPixmap_addr_2">[2]</a></sup>

---

<a name="SkPixmap_writable_addr_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void* writable_addr(int x, int y) const
</pre>

Returns writable pixel address at (<a href="#SkPixmap_writable_addr_2_x">x</a>, <a href="#SkPixmap_writable_addr_2_y">y</a>).

Input is not validated: out of bounds values of <a href="#SkPixmap_writable_addr_2_x">x</a> or <a href="#SkPixmap_writable_addr_2_y">y</a> trigger an assert() if
built with SK_DEBUG defined. Returns zero if <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is <a href="SkImageInfo_Reference#kUnknown_SkColorType">kUnknown_SkColorType</a>.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_writable_addr_2_x"> <code><strong>x </strong></code> </a></td> <td>
column index, zero or greater, and less than <a href="#SkPixmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_writable_addr_2_y"> <code><strong>y </strong></code> </a></td> <td>
row index, zero or greater, and less than <a href="#SkPixmap_height">height</a></td>
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

<a href="#SkPixmap_writable_addr8">writable addr8</a> <a href="#SkPixmap_writable_addr16">writable addr16</a> <a href="#SkPixmap_writable_addr32">writable addr32</a> <a href="#SkPixmap_writable_addr64">writable addr64</a> <a href="#SkPixmap_writable_addrF16">writable addrF16</a> <a href="#SkPixmap_addr">addr</a><sup><a href="#SkPixmap_addr_2">[2]</a></sup>

---

<a name="SkPixmap_writable_addr8"></a>
## writable_addr8

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
uint8_t* writable_addr8(int x, int y) const
</pre>

Returns writable pixel address at (<a href="#SkPixmap_writable_addr8_x">x</a>, <a href="#SkPixmap_writable_addr8_y">y</a>). Result is addressable as unsigned
8-bit bytes. Will trigger an assert() if <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is not <a href="SkImageInfo_Reference#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>
or <a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, and is built with SK_DEBUG defined.

One byte corresponds to one pixel.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_writable_addr8_x"> <code><strong>x </strong></code> </a></td> <td>
column index, zero or greater, and less than <a href="#SkPixmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_writable_addr8_y"> <code><strong>y </strong></code> </a></td> <td>
row index, zero or greater, and less than <a href="#SkPixmap_height">height</a></td>
  </tr>
</table>

### Return Value

writable unsigned 8-bit pointer to pixels

### Example

<div><fiddle-embed name="809284db136748208b3efc31cd89de29"><div>Altering pixels after drawing <a href="SkBitmap_Reference#Bitmap">Bitmap</a> is not guaranteed to affect subsequent
drawing on all platforms. Adding a second <a href="SkBitmap_Reference#SkBitmap_installPixels">SkBitmap::installPixels</a> after editing
pixel memory is safer.</div></fiddle-embed></div>

### See Also

<a href="#SkPixmap_writable_addr">writable addr</a><sup><a href="#SkPixmap_writable_addr_2">[2]</a></sup> <a href="#SkPixmap_writable_addr16">writable addr16</a> <a href="#SkPixmap_writable_addr32">writable addr32</a> <a href="#SkPixmap_writable_addr64">writable addr64</a> <a href="#SkPixmap_writable_addrF16">writable addrF16</a> <a href="#SkPixmap_addr">addr</a><sup><a href="#SkPixmap_addr_2">[2]</a></sup> <a href="#SkPixmap_addr8">addr8</a><sup><a href="#SkPixmap_addr8_2">[2]</a></sup>

---

<a name="SkPixmap_writable_addr16"></a>
## writable_addr16

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
uint16_t* writable_addr16(int x, int y) const
</pre>

Returns <a href="#SkPixmap_writable_addr">writable addr</a> pixel address at (<a href="#SkPixmap_writable_addr16_x">x</a>, <a href="#SkPixmap_writable_addr16_y">y</a>). Result is addressable as unsigned
16-bit words. Will trigger an assert() if <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is not <a href="SkImageInfo_Reference#kRGB_565_SkColorType">kRGB_565_SkColorType</a>
or <a href="SkImageInfo_Reference#kARGB_4444_SkColorType">kARGB_4444_SkColorType</a>, and is built with SK_DEBUG defined.

One word corresponds to one pixel.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_writable_addr16_x"> <code><strong>x </strong></code> </a></td> <td>
column index, zero or greater, and less than <a href="#SkPixmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_writable_addr16_y"> <code><strong>y </strong></code> </a></td> <td>
row index, zero or greater, and less than <a href="#SkPixmap_height">height</a></td>
  </tr>
</table>

### Return Value

writable unsigned 16-bit pointer to pixel

### Example

<div><fiddle-embed name="6da54774f6432b46b47ea9013c15f280"><div>Draw a five by five bitmap, and draw it again with a center black pixel.
The low nibble of the 16-bit word is <a href="undocumented#Alpha">Alpha</a>.</div></fiddle-embed></div>

### See Also

<a href="#SkPixmap_writable_addr">writable addr</a><sup><a href="#SkPixmap_writable_addr_2">[2]</a></sup> <a href="#SkPixmap_writable_addr8">writable addr8</a> <a href="#SkPixmap_writable_addr32">writable addr32</a> <a href="#SkPixmap_writable_addr64">writable addr64</a> <a href="#SkPixmap_writable_addrF16">writable addrF16</a> <a href="#SkPixmap_addr">addr</a><sup><a href="#SkPixmap_addr_2">[2]</a></sup> <a href="#SkPixmap_addr16">addr16</a><sup><a href="#SkPixmap_addr16_2">[2]</a></sup>

---

<a name="SkPixmap_writable_addr32"></a>
## writable_addr32

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
uint32_t* writable_addr32(int x, int y) const
</pre>

Returns writable pixel address at (<a href="#SkPixmap_writable_addr32_x">x</a>, <a href="#SkPixmap_writable_addr32_y">y</a>). Result is addressable as unsigned
32-bit words. Will trigger an assert() if <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is not
<a href="SkImageInfo_Reference#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a> or <a href="SkImageInfo_Reference#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>, and is built with SK_DEBUG
defined.

One word corresponds to one pixel.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_writable_addr32_x"> <code><strong>x </strong></code> </a></td> <td>
column index, zero or greater, and less than <a href="#SkPixmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_writable_addr32_y"> <code><strong>y </strong></code> </a></td> <td>
row index, zero or greater, and less than <a href="#SkPixmap_height">height</a></td>
  </tr>
</table>

### Return Value

writable unsigned 32-bit pointer to pixel

### Example

<div><fiddle-embed name="f4fdce206b8c0a4e79f0a9f52b7f47a6"></fiddle-embed></div>

### See Also

<a href="#SkPixmap_writable_addr">writable addr</a><sup><a href="#SkPixmap_writable_addr_2">[2]</a></sup> <a href="#SkPixmap_writable_addr8">writable addr8</a> <a href="#SkPixmap_writable_addr16">writable addr16</a> <a href="#SkPixmap_writable_addr64">writable addr64</a> <a href="#SkPixmap_writable_addrF16">writable addrF16</a> <a href="#SkPixmap_addr">addr</a><sup><a href="#SkPixmap_addr_2">[2]</a></sup> <a href="#SkPixmap_addr32">addr32</a><sup><a href="#SkPixmap_addr32_2">[2]</a></sup>

---

<a name="SkPixmap_writable_addr64"></a>
## writable_addr64

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
uint64_t* writable_addr64(int x, int y) const
</pre>

Returns writable pixel address at (<a href="#SkPixmap_writable_addr64_x">x</a>, <a href="#SkPixmap_writable_addr64_y">y</a>). Result is addressable as unsigned
64-bit words. Will trigger an assert() if <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is not
<a href="SkImageInfo_Reference#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a> and is built with SK_DEBUG defined.

One word corresponds to one pixel.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_writable_addr64_x"> <code><strong>x </strong></code> </a></td> <td>
column index, zero or greater, and less than <a href="#SkPixmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_writable_addr64_y"> <code><strong>y </strong></code> </a></td> <td>
row index, zero or greater, and less than <a href="#SkPixmap_height">height</a></td>
  </tr>
</table>

### Return Value

writable unsigned 64-bit pointer to pixel

### Example

<div><fiddle-embed name="de14d8d30e4a7b6462103d0e0dd96b0b"></fiddle-embed></div>

### See Also

<a href="#SkPixmap_writable_addr">writable addr</a><sup><a href="#SkPixmap_writable_addr_2">[2]</a></sup> <a href="#SkPixmap_writable_addr8">writable addr8</a> <a href="#SkPixmap_writable_addr16">writable addr16</a> <a href="#SkPixmap_writable_addr32">writable addr32</a> <a href="#SkPixmap_writable_addrF16">writable addrF16</a> <a href="#SkPixmap_addr">addr</a><sup><a href="#SkPixmap_addr_2">[2]</a></sup> <a href="#SkPixmap_addr64">addr64</a><sup><a href="#SkPixmap_addr64_2">[2]</a></sup>

---

<a name="SkPixmap_writable_addrF16"></a>
## writable_addrF16

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
uint16_t* writable_addrF16(int x, int y) const
</pre>

Returns writable pixel address at (<a href="#SkPixmap_writable_addrF16_x">x</a>, <a href="#SkPixmap_writable_addrF16_y">y</a>). Result is addressable as unsigned
16-bit words. Will trigger an assert() if <a href="SkImageInfo_Reference#Color_Type">Color Type</a> is not
<a href="SkImageInfo_Reference#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a> and is built with SK_DEBUG defined.

Each word represents one color component encoded as a half float.
Four words correspond to one pixel.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_writable_addrF16_x"> <code><strong>x </strong></code> </a></td> <td>
column index, zero or greater, and less than <a href="#SkPixmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_writable_addrF16_y"> <code><strong>y </strong></code> </a></td> <td>
row index, zero or greater, and less than <a href="#SkPixmap_height">height</a></td>
  </tr>
</table>

### Return Value

writable unsigned 16-bit pointer to first component of pixel

### Example

<div><fiddle-embed name="7822d78f5cacf5c04267cbbc6c6d0b80"><div>Left bitmap is drawn with two pixels defined in half float format. Right bitmap
is drawn after overwriting bottom half float color with top half float color.</div></fiddle-embed></div>

### See Also

<a href="#SkPixmap_writable_addr">writable addr</a><sup><a href="#SkPixmap_writable_addr_2">[2]</a></sup> <a href="#SkPixmap_writable_addr8">writable addr8</a> <a href="#SkPixmap_writable_addr16">writable addr16</a> <a href="#SkPixmap_writable_addr32">writable addr32</a> <a href="#SkPixmap_writable_addr64">writable addr64</a> <a href="#SkPixmap_addr">addr</a><sup><a href="#SkPixmap_addr_2">[2]</a></sup> <a href="#SkPixmap_addrF16">addrF16</a><sup><a href="#SkPixmap_addrF16_2">[2]</a></sup>

---

## <a name="Pixels"></a> Pixels

| name | description |
| --- | --- |
| <a href="#SkPixmap_erase">erase</a> | writes <a href="undocumented#Color">Color</a> to pixels |
|  | <a href="#SkPixmap_erase">erase(SkColor color, const SkIRect& subset)</a> const |
|  | <a href="#SkPixmap_erase_2">erase(SkColor color)</a> const |
|  | <a href="#SkPixmap_erase_3">erase(const SkColor4f& color, const SkIRect* subset = nullptr)</a> const |
| <a href="#SkPixmap_readPixels">readPixels</a> | copies and converts pixels |
|  | <a href="#SkPixmap_readPixels">readPixels(const SkImageInfo& dstInfo, void* dstPixels, size t dstRowBytes, int srcX, int srcY, SkTransferFunctionBehavior behavior)</a> const |
|  | <a href="#SkPixmap_readPixels_2">readPixels(const SkImageInfo& dstInfo, void* dstPixels, size t dstRowBytes)</a> const |
|  | <a href="#SkPixmap_readPixels_3">readPixels(const SkImageInfo& dstInfo, void* dstPixels, size t dstRowBytes, int srcX, int srcY)</a> const |
|  | <a href="#SkPixmap_readPixels_4">readPixels(const SkPixmap& dst, int srcX, int srcY)</a> const |
|  | <a href="#SkPixmap_readPixels_5">readPixels(const SkPixmap& dst)</a> const |
| <a href="#SkPixmap_scalePixels">scalePixels</a> | scales and converts pixels |

<a name="SkPixmap_readPixels"></a>
## readPixels

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes, int srcX, int srcY,
                SkTransferFunctionBehavior behavior) const
</pre>

Copies a <a href="SkRect_Reference#Rect">Rect</a> of pixels to <a href="#SkPixmap_readPixels_dstPixels">dstPixels</a>. Copy starts at (<a href="#SkPixmap_readPixels_srcX">srcX</a>, <a href="#SkPixmap_readPixels_srcY">srcY</a>), and does not
exceed <a href="#Pixmap">Pixmap</a> (<a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>).

<a href="#SkPixmap_readPixels_dstInfo">dstInfo</a> specifies width, height, <a href="SkImageInfo_Reference#Color_Type">Color Type</a>, <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a>, and
<a href="undocumented#Color_Space">Color Space</a> of destination. <a href="#SkPixmap_readPixels_dstRowBytes">dstRowBytes</a> specifics the gap from one destination
row to the next. Returns true if pixels are copied. Returns false if
<a href="#SkPixmap_readPixels_dstInfo">dstInfo</a>.<a href="#SkPixmap_addr">addr</a> equals nullptr, or <a href="#SkPixmap_readPixels_dstRowBytes">dstRowBytes</a> is less than <a href="#SkPixmap_readPixels_dstInfo">dstInfo</a>.<a href="SkImageInfo_Reference#SkImageInfo">minRowBytes</a>.

<a href="#Pixels">Pixels</a> are copied only if pixel conversion is possible. If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorType">colorType</a> is
<a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, or <a href="SkImageInfo_Reference#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>; <a href="#SkPixmap_readPixels_dstInfo">dstInfo</a>.<a href="#SkPixmap_colorType">colorType</a> must match.
If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorType">colorType</a> is <a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, <a href="#SkPixmap_readPixels_dstInfo">dstInfo</a>.<a href="#SkPixmap_colorSpace">colorSpace</a> must match.
If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_alphaType">alphaType</a> is <a href="SkImageInfo_Reference#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>, <a href="#SkPixmap_readPixels_dstInfo">dstInfo</a>.<a href="#SkPixmap_alphaType">alphaType</a> must
match. If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorSpace">colorSpace</a> is nullptr, <a href="#SkPixmap_readPixels_dstInfo">dstInfo</a>.<a href="#SkPixmap_colorSpace">colorSpace</a> must match. Returns
false if pixel conversion is not possible.
<a href="#SkPixmap_readPixels_srcX">srcX</a> and <a href="#SkPixmap_readPixels_srcY">srcY</a> may be negative to copy only top or left of source. Returns
false if <a href="#SkPixmap_width">width</a> or <a href="#SkPixmap_height">height</a> is zero or negative. Returns false if:

abs(srcX) >= <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_width">width</a>,
or ifabs(srcY) >= <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_height">height</a>.

If <a href="#SkPixmap_readPixels_behavior">behavior</a> is <a href="undocumented#SkTransferFunctionBehavior_kRespect">SkTransferFunctionBehavior::kRespect</a>: converts source
pixels to a linear space before converting to <a href="#SkPixmap_readPixels_dstInfo">dstInfo</a>.
If <a href="#SkPixmap_readPixels_behavior">behavior</a> is <a href="undocumented#SkTransferFunctionBehavior_kIgnore">SkTransferFunctionBehavior::kIgnore</a>: source
pixels are treated as if they are linear, regardless of how they are encoded.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_readPixels_dstInfo"> <code><strong>dstInfo </strong></code> </a></td> <td>
destination width, height, <a href="SkImageInfo_Reference#Color_Type">Color Type</a>, <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_readPixels_dstPixels"> <code><strong>dstPixels </strong></code> </a></td> <td>
destination pixel storage</td>
  </tr>  <tr>    <td><a name="SkPixmap_readPixels_dstRowBytes"> <code><strong>dstRowBytes </strong></code> </a></td> <td>
destination row length</td>
  </tr>  <tr>    <td><a name="SkPixmap_readPixels_srcX"> <code><strong>srcX </strong></code> </a></td> <td>
column index whose absolute value is less than <a href="#SkPixmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_readPixels_srcY"> <code><strong>srcY </strong></code> </a></td> <td>
row index whose absolute value is less than <a href="#SkPixmap_height">height</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_readPixels_behavior"> <code><strong>behavior </strong></code> </a></td> <td>
one of: <a href="undocumented#SkTransferFunctionBehavior_kRespect">SkTransferFunctionBehavior::kRespect</a>,
<a href="undocumented#SkTransferFunctionBehavior_kIgnore">SkTransferFunctionBehavior::kIgnore</a></td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href="#SkPixmap_readPixels_dstPixels">dstPixels</a>

### Example

<div><fiddle-embed name="2b7f6cc59ea2d5ebceddccbc2f232bcf"></fiddle-embed></div>

### See Also

<a href="#SkPixmap_erase">erase</a><sup><a href="#SkPixmap_erase_2">[2]</a></sup><sup><a href="#SkPixmap_erase_3">[3]</a></sup> <a href="SkBitmap_Reference#SkBitmap_readPixels">SkBitmap::readPixels</a><sup><a href="SkBitmap_Reference#SkBitmap_readPixels_2">[2]</a></sup><sup><a href="SkBitmap_Reference#SkBitmap_readPixels_3">[3]</a></sup><sup><a href="SkBitmap_Reference#SkBitmap_readPixels_4">[4]</a></sup> <a href="SkCanvas_Reference#SkCanvas_drawBitmap">SkCanvas::drawBitmap</a> <a href="SkCanvas_Reference#SkCanvas_readPixels">SkCanvas::readPixels</a><sup><a href="SkCanvas_Reference#SkCanvas_readPixels_2">[2]</a></sup><sup><a href="SkCanvas_Reference#SkCanvas_readPixels_3">[3]</a></sup> <a href="SkImage_Reference#SkImage_readPixels">SkImage::readPixels</a><sup><a href="SkImage_Reference#SkImage_readPixels_2">[2]</a></sup> <a href="SkSurface_Reference#SkSurface_readPixels">SkSurface::readPixels</a><sup><a href="SkSurface_Reference#SkSurface_readPixels_2">[2]</a></sup><sup><a href="SkSurface_Reference#SkSurface_readPixels_3">[3]</a></sup>

---

<a name="SkPixmap_readPixels_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes) const
</pre>

Copies a <a href="SkRect_Reference#Rect">Rect</a> of pixels to <a href="#SkPixmap_readPixels_2_dstPixels">dstPixels</a>. Copy starts at (0, 0), and does not
exceed <a href="#Pixmap">Pixmap</a> (<a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>).

<a href="#SkPixmap_readPixels_2_dstInfo">dstInfo</a> specifies width, height, <a href="SkImageInfo_Reference#Color_Type">Color Type</a>, <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a>, and
<a href="undocumented#Color_Space">Color Space</a> of destination. <a href="#SkPixmap_readPixels_2_dstRowBytes">dstRowBytes</a> specifics the gap from one destination
row to the next. Returns true if pixels are copied. Returns false if
<a href="#SkPixmap_readPixels_2_dstInfo">dstInfo</a>.<a href="#SkPixmap_addr">addr</a> equals nullptr, or <a href="#SkPixmap_readPixels_2_dstRowBytes">dstRowBytes</a> is less than <a href="#SkPixmap_readPixels_2_dstInfo">dstInfo</a>.<a href="SkImageInfo_Reference#SkImageInfo">minRowBytes</a>.

<a href="#Pixels">Pixels</a> are copied only if pixel conversion is possible. If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorType">colorType</a> is
<a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, or <a href="SkImageInfo_Reference#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>; <a href="#SkPixmap_readPixels_2_dstInfo">dstInfo</a>.<a href="#SkPixmap_colorType">colorType</a> must match.
If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorType">colorType</a> is <a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, <a href="#SkPixmap_readPixels_2_dstInfo">dstInfo</a>.<a href="#SkPixmap_colorSpace">colorSpace</a> must match.
If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_alphaType">alphaType</a> is <a href="SkImageInfo_Reference#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>, <a href="#SkPixmap_readPixels_2_dstInfo">dstInfo</a>.<a href="#SkPixmap_alphaType">alphaType</a> must
match. If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorSpace">colorSpace</a> is nullptr, <a href="#SkPixmap_readPixels_2_dstInfo">dstInfo</a>.<a href="#SkPixmap_colorSpace">colorSpace</a> must match. Returns
false if pixel conversion is not possible.

Returns false if <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_width">width</a> or <a href="#SkPixmap_height">height</a> is zero or negative.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_readPixels_2_dstInfo"> <code><strong>dstInfo </strong></code> </a></td> <td>
destination width, height, <a href="SkImageInfo_Reference#Color_Type">Color Type</a>, <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_readPixels_2_dstPixels"> <code><strong>dstPixels </strong></code> </a></td> <td>
destination pixel storage</td>
  </tr>  <tr>    <td><a name="SkPixmap_readPixels_2_dstRowBytes"> <code><strong>dstRowBytes </strong></code> </a></td> <td>
destination row length</td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href="#SkPixmap_readPixels_2_dstPixels">dstPixels</a>

### Example

<div><fiddle-embed name="df4e355c4845350daede833b4fd21ec1"><div>Transferring the gradient from 8 bits per component to 4 bits per component
creates visible banding.</div></fiddle-embed></div>

### See Also

<a href="#SkPixmap_erase">erase</a><sup><a href="#SkPixmap_erase_2">[2]</a></sup><sup><a href="#SkPixmap_erase_3">[3]</a></sup> <a href="SkBitmap_Reference#SkBitmap_readPixels">SkBitmap::readPixels</a><sup><a href="SkBitmap_Reference#SkBitmap_readPixels_2">[2]</a></sup><sup><a href="SkBitmap_Reference#SkBitmap_readPixels_3">[3]</a></sup><sup><a href="SkBitmap_Reference#SkBitmap_readPixels_4">[4]</a></sup> <a href="SkCanvas_Reference#SkCanvas_drawBitmap">SkCanvas::drawBitmap</a> <a href="SkCanvas_Reference#SkCanvas_readPixels">SkCanvas::readPixels</a><sup><a href="SkCanvas_Reference#SkCanvas_readPixels_2">[2]</a></sup><sup><a href="SkCanvas_Reference#SkCanvas_readPixels_3">[3]</a></sup> <a href="SkImage_Reference#SkImage_readPixels">SkImage::readPixels</a><sup><a href="SkImage_Reference#SkImage_readPixels_2">[2]</a></sup> <a href="SkSurface_Reference#SkSurface_readPixels">SkSurface::readPixels</a><sup><a href="SkSurface_Reference#SkSurface_readPixels_2">[2]</a></sup><sup><a href="SkSurface_Reference#SkSurface_readPixels_3">[3]</a></sup>

---

<a name="SkPixmap_readPixels_3"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes, int srcX, int srcY) const
</pre>

Copies a <a href="SkRect_Reference#Rect">Rect</a> of pixels to <a href="#SkPixmap_readPixels_3_dstPixels">dstPixels</a>. Copy starts at (<a href="#SkPixmap_readPixels_3_srcX">srcX</a>, <a href="#SkPixmap_readPixels_3_srcY">srcY</a>), and does not
exceed <a href="#Pixmap">Pixmap</a> (<a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>).

<a href="#SkPixmap_readPixels_3_dstInfo">dstInfo</a> specifies width, height, <a href="SkImageInfo_Reference#Color_Type">Color Type</a>, <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a>, and
<a href="undocumented#Color_Space">Color Space</a> of destination. <a href="#SkPixmap_readPixels_3_dstRowBytes">dstRowBytes</a> specifics the gap from one destination
row to the next. Returns true if pixels are copied. Returns false if
<a href="#SkPixmap_readPixels_3_dstInfo">dstInfo</a>.<a href="#SkPixmap_addr">addr</a> equals nullptr, or <a href="#SkPixmap_readPixels_3_dstRowBytes">dstRowBytes</a> is less than <a href="#SkPixmap_readPixels_3_dstInfo">dstInfo</a>.<a href="SkImageInfo_Reference#SkImageInfo">minRowBytes</a>.

<a href="#Pixels">Pixels</a> are copied only if pixel conversion is possible. If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorType">colorType</a> is
<a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, or <a href="SkImageInfo_Reference#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>; <a href="#SkPixmap_readPixels_3_dstInfo">dstInfo</a>.<a href="#SkPixmap_colorType">colorType</a> must match.
If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorType">colorType</a> is <a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, <a href="#SkPixmap_readPixels_3_dstInfo">dstInfo</a>.<a href="#SkPixmap_colorSpace">colorSpace</a> must match.
If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_alphaType">alphaType</a> is <a href="SkImageInfo_Reference#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>, <a href="#SkPixmap_readPixels_3_dstInfo">dstInfo</a>.<a href="#SkPixmap_alphaType">alphaType</a> must
match. If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorSpace">colorSpace</a> is nullptr, <a href="#SkPixmap_readPixels_3_dstInfo">dstInfo</a>.<a href="#SkPixmap_colorSpace">colorSpace</a> must match. Returns
false if pixel conversion is not possible.
<a href="#SkPixmap_readPixels_3_srcX">srcX</a> and <a href="#SkPixmap_readPixels_3_srcY">srcY</a> may be negative to copy only top or left of source. Returns
false if <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_width">width</a> or <a href="#SkPixmap_height">height</a> is zero or negative. Returns false if:

abs(srcX) >= <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_width">width</a>,
or ifabs(srcY) >= <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_height">height</a>.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_readPixels_3_dstInfo"> <code><strong>dstInfo </strong></code> </a></td> <td>
destination width, height, <a href="SkImageInfo_Reference#Color_Type">Color Type</a>, <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_readPixels_3_dstPixels"> <code><strong>dstPixels </strong></code> </a></td> <td>
destination pixel storage</td>
  </tr>  <tr>    <td><a name="SkPixmap_readPixels_3_dstRowBytes"> <code><strong>dstRowBytes </strong></code> </a></td> <td>
destination row length</td>
  </tr>  <tr>    <td><a name="SkPixmap_readPixels_3_srcX"> <code><strong>srcX </strong></code> </a></td> <td>
column index whose absolute value is less than <a href="#SkPixmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_readPixels_3_srcY"> <code><strong>srcY </strong></code> </a></td> <td>
row index whose absolute value is less than <a href="#SkPixmap_height">height</a></td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href="#SkPixmap_readPixels_3_dstPixels">dstPixels</a>

### Example

<div><fiddle-embed name="094ca0bd37588cc7be241bb387a3e17b"></fiddle-embed></div>

### See Also

<a href="#SkPixmap_erase">erase</a><sup><a href="#SkPixmap_erase_2">[2]</a></sup><sup><a href="#SkPixmap_erase_3">[3]</a></sup> <a href="SkBitmap_Reference#SkBitmap_readPixels">SkBitmap::readPixels</a><sup><a href="SkBitmap_Reference#SkBitmap_readPixels_2">[2]</a></sup><sup><a href="SkBitmap_Reference#SkBitmap_readPixels_3">[3]</a></sup><sup><a href="SkBitmap_Reference#SkBitmap_readPixels_4">[4]</a></sup> <a href="SkCanvas_Reference#SkCanvas_drawBitmap">SkCanvas::drawBitmap</a> <a href="SkCanvas_Reference#SkCanvas_readPixels">SkCanvas::readPixels</a><sup><a href="SkCanvas_Reference#SkCanvas_readPixels_2">[2]</a></sup><sup><a href="SkCanvas_Reference#SkCanvas_readPixels_3">[3]</a></sup> <a href="SkImage_Reference#SkImage_readPixels">SkImage::readPixels</a><sup><a href="SkImage_Reference#SkImage_readPixels_2">[2]</a></sup> <a href="SkSurface_Reference#SkSurface_readPixels">SkSurface::readPixels</a><sup><a href="SkSurface_Reference#SkSurface_readPixels_2">[2]</a></sup><sup><a href="SkSurface_Reference#SkSurface_readPixels_3">[3]</a></sup>

---

<a name="SkPixmap_readPixels_4"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool readPixels(const SkPixmap& dst, int srcX, int srcY) const
</pre>

Copies a <a href="SkRect_Reference#Rect">Rect</a> of pixels to <a href="#SkPixmap_readPixels_4_dst">dst</a>. Copy starts at (<a href="#SkPixmap_readPixels_4_srcX">srcX</a>, <a href="#SkPixmap_readPixels_4_srcY">srcY</a>), and does not
exceed <a href="#Pixmap">Pixmap</a> (<a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>). <a href="#SkPixmap_readPixels_4_dst">dst</a> specifies width, height, <a href="SkImageInfo_Reference#Color_Type">Color Type</a>,
<a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a>, and <a href="undocumented#Color_Space">Color Space</a> of destination.  Returns true if pixels are copied.
Returns false if <a href="#SkPixmap_readPixels_4_dst">dst</a>.<a href="#SkPixmap_addr">addr</a> equals nullptr, or <a href="#SkPixmap_readPixels_4_dst">dst</a>.<a href="#SkPixmap_rowBytes">rowBytes</a> is less than
<a href="#SkPixmap_readPixels_4_dst">dst</a> <a href="SkImageInfo_Reference#SkImageInfo_minRowBytes">SkImageInfo::minRowBytes</a>.

<a href="#Pixels">Pixels</a> are copied only if pixel conversion is possible. If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorType">colorType</a> is
<a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, or <a href="SkImageInfo_Reference#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>; <a href="#SkPixmap_readPixels_4_dst">dst</a>.<a href="#SkPixmap_info">info</a>.<a href="#SkPixmap_colorType">colorType</a> must match.
If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorType">colorType</a> is <a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, <a href="#SkPixmap_readPixels_4_dst">dst</a>.<a href="#SkPixmap_info">info</a>.<a href="#SkPixmap_colorSpace">colorSpace</a> must match.
If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_alphaType">alphaType</a> is <a href="SkImageInfo_Reference#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>, <a href="#SkPixmap_readPixels_4_dst">dst</a>.<a href="#SkPixmap_info">info</a>.<a href="#SkPixmap_alphaType">alphaType</a> must
match. If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorSpace">colorSpace</a> is nullptr, <a href="#SkPixmap_readPixels_4_dst">dst</a>.<a href="#SkPixmap_info">info</a>.<a href="#SkPixmap_colorSpace">colorSpace</a> must match. Returns
false if pixel conversion is not possible.
<a href="#SkPixmap_readPixels_4_srcX">srcX</a> and <a href="#SkPixmap_readPixels_4_srcY">srcY</a> may be negative to copy only top or left of source. Returns
false <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_width">width</a> or <a href="#SkPixmap_height">height</a> is zero or negative. Returns false if:

abs(srcX) >= <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_width">width</a>,
or ifabs(srcY) >= <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_height">height</a>.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_readPixels_4_dst"> <code><strong>dst </strong></code> </a></td> <td>
<a href="SkImageInfo_Reference#Image_Info">Image Info</a> and pixel address to write to</td>
  </tr>  <tr>    <td><a name="SkPixmap_readPixels_4_srcX"> <code><strong>srcX </strong></code> </a></td> <td>
column index whose absolute value is less than <a href="#SkPixmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_readPixels_4_srcY"> <code><strong>srcY </strong></code> </a></td> <td>
row index whose absolute value is less than <a href="#SkPixmap_height">height</a></td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href="#SkPixmap_readPixels_4_dst">dst</a>

### Example

<div><fiddle-embed name="6ec7f7b2cc163cd29f627eef6d4b061c"></fiddle-embed></div>

### See Also

<a href="#SkPixmap_erase">erase</a><sup><a href="#SkPixmap_erase_2">[2]</a></sup><sup><a href="#SkPixmap_erase_3">[3]</a></sup> <a href="SkBitmap_Reference#SkBitmap_readPixels">SkBitmap::readPixels</a><sup><a href="SkBitmap_Reference#SkBitmap_readPixels_2">[2]</a></sup><sup><a href="SkBitmap_Reference#SkBitmap_readPixels_3">[3]</a></sup><sup><a href="SkBitmap_Reference#SkBitmap_readPixels_4">[4]</a></sup> <a href="SkCanvas_Reference#SkCanvas_drawBitmap">SkCanvas::drawBitmap</a> <a href="SkCanvas_Reference#SkCanvas_readPixels">SkCanvas::readPixels</a><sup><a href="SkCanvas_Reference#SkCanvas_readPixels_2">[2]</a></sup><sup><a href="SkCanvas_Reference#SkCanvas_readPixels_3">[3]</a></sup> <a href="SkImage_Reference#SkImage_readPixels">SkImage::readPixels</a><sup><a href="SkImage_Reference#SkImage_readPixels_2">[2]</a></sup> <a href="SkSurface_Reference#SkSurface_readPixels">SkSurface::readPixels</a><sup><a href="SkSurface_Reference#SkSurface_readPixels_2">[2]</a></sup><sup><a href="SkSurface_Reference#SkSurface_readPixels_3">[3]</a></sup>

---

<a name="SkPixmap_readPixels_5"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool readPixels(const SkPixmap& dst) const
</pre>

Copies pixels inside <a href="#SkPixmap_bounds">bounds</a> to <a href="#SkPixmap_readPixels_5_dst">dst</a>. <a href="#SkPixmap_readPixels_5_dst">dst</a> specifies width, height, <a href="SkImageInfo_Reference#Color_Type">Color Type</a>,
<a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a>, and <a href="undocumented#Color_Space">Color Space</a> of destination.  Returns true if pixels are copied.
Returns false if <a href="#SkPixmap_readPixels_5_dst">dst</a>.<a href="#SkPixmap_addr">addr</a> equals nullptr, or <a href="#SkPixmap_readPixels_5_dst">dst</a>.<a href="#SkPixmap_rowBytes">rowBytes</a> is less than
<a href="#SkPixmap_readPixels_5_dst">dst</a> <a href="SkImageInfo_Reference#SkImageInfo_minRowBytes">SkImageInfo::minRowBytes</a>.

<a href="#Pixels">Pixels</a> are copied only if pixel conversion is possible. If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorType">colorType</a> is
<a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, or <a href="SkImageInfo_Reference#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>; <a href="#SkPixmap_readPixels_5_dst">dst</a> <a href="SkImageInfo_Reference#Color_Type">Color Type</a> must match.
If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorType">colorType</a> is <a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, <a href="#SkPixmap_readPixels_5_dst">dst</a> <a href="undocumented#Color_Space">Color Space</a> must match.
If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_alphaType">alphaType</a> is <a href="SkImageInfo_Reference#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>, <a href="#SkPixmap_readPixels_5_dst">dst</a> <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a> must
match. If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorSpace">colorSpace</a> is nullptr, <a href="#SkPixmap_readPixels_5_dst">dst</a> <a href="undocumented#Color_Space">Color Space</a> must match. Returns
false if pixel conversion is not possible.
Returns false if <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_width">width</a> or <a href="#SkPixmap_height">height</a> is zero or negative.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_readPixels_5_dst"> <code><strong>dst </strong></code> </a></td> <td>
<a href="SkImageInfo_Reference#Image_Info">Image Info</a> and pixel address to write to</td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href="#SkPixmap_readPixels_5_dst">dst</a>

### Example

<div><fiddle-embed name="e18549b5ee1039cb61b0bb38c2104fc9"></fiddle-embed></div>

### See Also

<a href="#SkPixmap_erase">erase</a><sup><a href="#SkPixmap_erase_2">[2]</a></sup><sup><a href="#SkPixmap_erase_3">[3]</a></sup> <a href="SkBitmap_Reference#SkBitmap_readPixels">SkBitmap::readPixels</a><sup><a href="SkBitmap_Reference#SkBitmap_readPixels_2">[2]</a></sup><sup><a href="SkBitmap_Reference#SkBitmap_readPixels_3">[3]</a></sup><sup><a href="SkBitmap_Reference#SkBitmap_readPixels_4">[4]</a></sup> <a href="SkCanvas_Reference#SkCanvas_drawBitmap">SkCanvas::drawBitmap</a> <a href="SkCanvas_Reference#SkCanvas_readPixels">SkCanvas::readPixels</a><sup><a href="SkCanvas_Reference#SkCanvas_readPixels_2">[2]</a></sup><sup><a href="SkCanvas_Reference#SkCanvas_readPixels_3">[3]</a></sup> <a href="SkImage_Reference#SkImage_readPixels">SkImage::readPixels</a><sup><a href="SkImage_Reference#SkImage_readPixels_2">[2]</a></sup> <a href="SkSurface_Reference#SkSurface_readPixels">SkSurface::readPixels</a><sup><a href="SkSurface_Reference#SkSurface_readPixels_2">[2]</a></sup><sup><a href="SkSurface_Reference#SkSurface_readPixels_3">[3]</a></sup>

---

<a name="SkPixmap_scalePixels"></a>
## scalePixels

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool scalePixels(const SkPixmap& dst, SkFilterQuality filterQuality) const
</pre>

Copies <a href="SkBitmap_Reference#Bitmap">Bitmap</a> to <a href="#SkPixmap_scalePixels_dst">dst</a>, scaling pixels to fit <a href="#SkPixmap_scalePixels_dst">dst</a>.<a href="#SkPixmap_width">width</a> and <a href="#SkPixmap_scalePixels_dst">dst</a>.<a href="#SkPixmap_height">height</a>, and
converting pixels to match <a href="#SkPixmap_scalePixels_dst">dst</a>.<a href="#SkPixmap_colorType">colorType</a> and <a href="#SkPixmap_scalePixels_dst">dst</a>.<a href="#SkPixmap_alphaType">alphaType</a>. Returns true if
pixels are copied. Returns false if <a href="#SkPixmap_scalePixels_dst">dst</a>.<a href="#SkPixmap_addr">addr</a> is nullptr, or <a href="#SkPixmap_scalePixels_dst">dst</a>.<a href="#SkPixmap_rowBytes">rowBytes</a> is
less than <a href="#SkPixmap_scalePixels_dst">dst</a> <a href="SkImageInfo_Reference#SkImageInfo_minRowBytes">SkImageInfo::minRowBytes</a>.

<a href="#Pixels">Pixels</a> are copied only if pixel conversion is possible. If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorType">colorType</a> is
<a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, or <a href="SkImageInfo_Reference#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>; <a href="#SkPixmap_scalePixels_dst">dst</a> <a href="SkImageInfo_Reference#Color_Type">Color Type</a> must match.
If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorType">colorType</a> is <a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray_8_SkColorType</a>, <a href="#SkPixmap_scalePixels_dst">dst</a> <a href="undocumented#Color_Space">Color Space</a> must match.
If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_alphaType">alphaType</a> is <a href="SkImageInfo_Reference#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>, <a href="#SkPixmap_scalePixels_dst">dst</a> <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a> must
match. If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorSpace">colorSpace</a> is nullptr, <a href="#SkPixmap_scalePixels_dst">dst</a> <a href="undocumented#Color_Space">Color Space</a> must match. Returns
false if pixel conversion is not possible.

Returns false if <a href="SkBitmap_Reference#Bitmap">Bitmap</a> <a href="#SkPixmap_width">width</a> or <a href="#SkPixmap_height">height</a> is zero or negative.

Scales the image, with <a href="#SkPixmap_scalePixels_filterQuality">filterQuality</a>, to match <a href="#SkPixmap_scalePixels_dst">dst</a>.<a href="#SkPixmap_width">width</a> and <a href="#SkPixmap_scalePixels_dst">dst</a>.<a href="#SkPixmap_height">height</a>.
<a href="#SkPixmap_scalePixels_filterQuality">filterQuality</a> <a href="undocumented#kNone_SkFilterQuality">kNone_SkFilterQuality</a> is fastest, typically implemented with
<a href="undocumented#Nearest_Neighbor">Filter Quality Nearest Neighbor</a>. <a href="undocumented#kLow_SkFilterQuality">kLow_SkFilterQuality</a> is typically implemented with
<a href="undocumented#Bilerp">Filter Quality Bilerp</a>. <a href="undocumented#kMedium_SkFilterQuality">kMedium_SkFilterQuality</a> is typically implemented with
<a href="undocumented#Bilerp">Filter Quality Bilerp</a>, and <a href="undocumented#MipMap">Filter Quality MipMap</a> when size is reduced.
<a href="undocumented#kHigh_SkFilterQuality">kHigh_SkFilterQuality</a> is slowest, typically implemented with <a href="undocumented#BiCubic">Filter Quality BiCubic</a>.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_scalePixels_dst"> <code><strong>dst </strong></code> </a></td> <td>
<a href="SkImageInfo_Reference#Image_Info">Image Info</a> and pixel address to write to</td>
  </tr>  <tr>    <td><a name="SkPixmap_scalePixels_filterQuality"> <code><strong>filterQuality </strong></code> </a></td> <td>
one of: <a href="undocumented#kNone_SkFilterQuality">kNone_SkFilterQuality</a>, <a href="undocumented#kLow_SkFilterQuality">kLow_SkFilterQuality</a>,
<a href="undocumented#kMedium_SkFilterQuality">kMedium_SkFilterQuality</a>, <a href="undocumented#kHigh_SkFilterQuality">kHigh_SkFilterQuality</a></td>
  </tr>
</table>

### Return Value

true if pixels are scaled to fit <a href="#SkPixmap_scalePixels_dst">dst</a>

### Example

<div><fiddle-embed name="8e3c8a9c1d0d2e9b8bf66e24d274f792"></fiddle-embed></div>

### See Also

<a href="SkCanvas_Reference#SkCanvas_drawBitmap">SkCanvas::drawBitmap</a> <a href="SkImage_Reference#SkImage_scalePixels">SkImage::scalePixels</a>

---

<a name="SkPixmap_erase"></a>
## erase

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool erase(SkColor color, const SkIRect& subset) const
</pre>

Writes <a href="#SkPixmap_erase_color">color</a> to pixels bounded by <a href="#SkPixmap_erase_subset">subset</a>; returns true on success.
Returns false if <a href="#SkPixmap_colorType">colorType</a> is <a href="SkImageInfo_Reference#kUnknown_SkColorType">kUnknown_SkColorType</a>, or if <a href="#SkPixmap_erase_subset">subset</a> does
not intersect <a href="#SkPixmap_bounds">bounds</a>.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_erase_color"> <code><strong>color </strong></code> </a></td> <td>
<a href="undocumented#Unpremultiply">Unpremultiplied</a> <a href="undocumented#Color">Color</a> to write</td>
  </tr>  <tr>    <td><a name="SkPixmap_erase_subset"> <code><strong>subset </strong></code> </a></td> <td>
bounding integer <a href="SkRect_Reference#Rect">Rect</a> of written pixels</td>
  </tr>
</table>

### Return Value

true if pixels are changed

### Example

<div><fiddle-embed name="a0cdbafed4786788cc90681e7b294234"></fiddle-embed></div>

### See Also

<a href="SkBitmap_Reference#SkBitmap_erase">SkBitmap::erase</a> <a href="SkCanvas_Reference#SkCanvas_clear">SkCanvas::clear</a> <a href="SkCanvas_Reference#SkCanvas_drawColor">SkCanvas::drawColor</a>

---

<a name="SkPixmap_erase_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool erase(SkColor color) const
</pre>

Writes <a href="#SkPixmap_erase_2_color">color</a> to pixels inside <a href="#SkPixmap_bounds">bounds</a>; returns true on success.
Returns false if <a href="#SkPixmap_colorType">colorType</a> is <a href="SkImageInfo_Reference#kUnknown_SkColorType">kUnknown_SkColorType</a>, or if <a href="#SkPixmap_bounds">bounds</a>
is empty.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_erase_2_color"> <code><strong>color </strong></code> </a></td> <td>
<a href="undocumented#Unpremultiply">Unpremultiplied</a> <a href="undocumented#Color">Color</a> to write</td>
  </tr>
</table>

### Return Value

true if pixels are changed

### Example

<div><fiddle-embed name="838202e0d49cad2eb3eeb495834f6d63"></fiddle-embed></div>

### See Also

<a href="SkBitmap_Reference#SkBitmap_erase">SkBitmap::erase</a> <a href="SkCanvas_Reference#SkCanvas_clear">SkCanvas::clear</a> <a href="SkCanvas_Reference#SkCanvas_drawColor">SkCanvas::drawColor</a>

---

<a name="SkPixmap_erase_3"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool erase(const SkColor4f& color, const SkIRect* subset = nullptr) const
</pre>

Writes <a href="#SkPixmap_erase_3_color">color</a> to pixels bounded by <a href="#SkPixmap_erase_3_subset">subset</a>; returns true on success.
if <a href="#SkPixmap_erase_3_subset">subset</a> is nullptr, writes colors pixels inside <a href="#SkPixmap_bounds">bounds</a>. Returns false if
<a href="#SkPixmap_colorType">colorType</a> is <a href="SkImageInfo_Reference#kUnknown_SkColorType">kUnknown_SkColorType</a>, if <a href="#SkPixmap_erase_3_subset">subset</a> is not nullptr and does
not intersect <a href="#SkPixmap_bounds">bounds</a>, or if <a href="#SkPixmap_erase_3_subset">subset</a> is nullptr and <a href="#SkPixmap_bounds">bounds</a> is empty.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_erase_3_color"> <code><strong>color </strong></code> </a></td> <td>
<a href="undocumented#Unpremultiply">Unpremultiplied</a> <a href="undocumented#Color">Color</a> to write</td>
  </tr>  <tr>    <td><a name="SkPixmap_erase_3_subset"> <code><strong>subset </strong></code> </a></td> <td>
bounding integer <a href="SkRect_Reference#Rect">Rect</a> of pixels to write; may be nullptr</td>
  </tr>
</table>

### Return Value

true if pixels are changed

### Example

<div><fiddle-embed name="f884f3f46a565f052a5e252ae2f36e9b"></fiddle-embed></div>

### See Also

<a href="SkBitmap_Reference#SkBitmap_erase">SkBitmap::erase</a> <a href="SkCanvas_Reference#SkCanvas_clear">SkCanvas::clear</a> <a href="SkCanvas_Reference#SkCanvas_drawColor">SkCanvas::drawColor</a>

---

