SkPixmap Reference
===

# <a name="Pixmap"></a> Pixmap

# <a name="SkPixmap"></a> Class SkPixmap
<a href="#Pixmap">Pixmap</a> provides a utility to pair <a href="undocumented#SkImageInfo">SkImageInfo</a> with pixels and row bytes.
<a href="#Pixmap">Pixmap</a> is a low level class which provides convenience functions to access
raster destinations. <a href="SkCanvas_Reference#Canvas">Canvas</a> can not draw <a href="#Pixmap">Pixmap</a>, nor does <a href="#Pixmap">Pixmap</a> provide
a direct drawing destination.

Use <a href="SkBitmap_Reference#Bitmap">Bitmap</a> to draw pixels referenced by <a href="#Pixmap">Pixmap</a>; use <a href="SkSurface_Reference#Surface">Surface</a> to draw into
pixels referenced by <a href="#Pixmap">Pixmap</a>.

<a href="#Pixmap">Pixmap</a> does not try to manage the lifetime of the pixel memory. Use <a href="undocumented#Pixel_Ref">Pixel Ref</a>
to manage pixel memory; <a href="undocumented#Pixel_Ref">Pixel Ref</a> is safe across threads.

# <a name="Overview"></a> Overview

## <a name="Subtopics"></a> Subtopics

| topics | description |
| --- | ---  |
| <a href="#Image_Info_Access">Image Info Access</a> | Returns all or part of <a href="undocumented#Image_Info">Image Info</a>. |
| <a href="#Initialization">Initialization</a> | Sets fields for use. |
| <a href="#Reader">Reader</a> | Examine pixel value. |
| <a href="#Writer">Writer</a> | Copy to pixel values. |
| <a href="#Readable_Address">Readable Address</a> | Returns read only pixels. |
| <a href="#Writable_Address">Writable Address</a> | Returns writable pixels. |

## <a name="Constructors"></a> Constructors

|  | description |
| --- | ---  |
| <a href="#SkPixmap_empty_constructor">SkPixmap()</a> | Constructs with default values. |
| <a href="#SkPixmap_const_SkImageInfo_const_star">SkPixmap(const SkImageInfo& info, const void* addr, size t rowBytes)</a> | Constructs from <a href="undocumented#Image_Info">Image Info</a>, pixels. |

## <a name="Member_Functions"></a> Member Functions

| function | description |
| --- | ---  |
| <a href="#SkPixmap_addr">addr</a> | Returns readable pixel address as void pointer. |
| <a href="#SkPixmap_addr16">addr16</a> | Returns readable pixel address as 16-bit pointer. |
| <a href="#SkPixmap_addr32">addr32</a> | Returns readable pixel address as 32-bit pointer. |
| <a href="#SkPixmap_addr64">addr64</a> | Returns readable pixel address as 64-bit pointer. |
| <a href="#SkPixmap_addr8">addr8</a> | Returns readable pixel address as 8-bit pointer. |
| <a href="#SkPixmap_addrF16">addrF16</a> | Returns readable pixel component address as 16-bit pointer. |
| <a href="#SkPixmap_alphaType">alphaType</a> | Returns <a href="undocumented#Image_Info">Image Info</a> <a href="undocumented#Image_Alpha_Type">Alpha Type</a>. |
| <a href="#SkPixmap_bounds">bounds</a> | Returns <a href="#SkPixmap_width">width</a> and <a href="#SkPixmap_height">height</a> as Rectangle. |
| <a href="#SkPixmap_colorSpace">colorSpace</a> | Returns <a href="undocumented#Image_Info">Image Info</a> <a href="undocumented#Color_Space">Color Space</a>. |
| <a href="#SkPixmap_colorType">colorType</a> | Returns <a href="undocumented#Image_Info">Image Info</a> <a href="undocumented#Image_Color_Type">Color Type</a>. |
| <a href="#SkPixmap_computeByteSize">computeByteSize</a> | Returns size required for pixels. |
| <a href="#SkPixmap_computeIsOpaque">computeIsOpaque</a> | Returns true if all pixels are opaque. |
| <a href="#SkPixmap_erase">erase</a> | Writes <a href="undocumented#Color">Color</a> to pixels. |
| <a href="#SkPixmap_extractSubset">extractSubset</a> | Sets pointer to portion of original. |
| <a href="#SkPixmap_getColor">getColor</a> | Returns one pixel as <a href="#Unpremultiply">Unpremultiplied</a> <a href="undocumented#Color">Color</a>. |
| <a href="#SkPixmap_height">height</a> | Returns pixel row count. |
| <a href="#SkPixmap_info">info</a> | Returns <a href="undocumented#Image_Info">Image Info</a>. |
| <a href="#SkPixmap_isOpaque">isOpaque</a> | Returns true if <a href="undocumented#Image_Info">Image Info</a> describes opaque pixels. |
| <a href="#SkPixmap_readPixels">readPixels</a> | Copies and converts pixels. |
| <a href="#SkPixmap_reset">reset</a> | Reuses existing <a href="#Pixmap">Pixmap</a> with replacement values. |
| <a href="#SkPixmap_rowBytes">rowBytes</a> | Returns interval between rows in bytes. |
| <a href="#SkPixmap_rowBytesAsPixels">rowBytesAsPixels</a> | Returns interval between rows in pixels. |
| <a href="#SkPixmap_scalePixels">scalePixels</a> | Scales and converts pixels. |
| <a href="#SkPixmap_setColorSpace">setColorSpace</a> | Sets <a href="undocumented#Image_Info">Image Info</a> <a href="undocumented#Color_Space">Color Space</a>. |
| <a href="#SkPixmap_shiftPerPixel">shiftPerPixel</a> | Returns bit shift from pixels to bytes. |
| <a href="#SkPixmap_width">width</a> | Returns pixel column count. |
| <a href="#SkPixmap_writable_addr">writable addr</a> | Returns writable pixel address as void pointer. |
| <a href="#SkPixmap_writable_addr16">writable addr16</a> | Returns writable pixel address as 16-bit pointer. |
| <a href="#SkPixmap_writable_addr32">writable addr32</a> | Returns writable pixel address as 32-bit pointer. |
| <a href="#SkPixmap_writable_addr64">writable addr64</a> | Returns writable pixel address as 64-bit pointer. |
| <a href="#SkPixmap_writable_addr8">writable addr8</a> | Returns writable pixel address as 8-bit pointer. |
| <a href="#SkPixmap_writable_addrF16">writable addrF16</a> | Returns writable pixel component address as 16-bit pointer. |

## <a name="Initialization"></a> Initialization

<a name="SkPixmap_empty_constructor"></a>
## SkPixmap

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkPixmap()
</pre>

Creates an empty <a href="#Pixmap">Pixmap</a> without pixels, with <a href="undocumented#SkColorType">kUnknown SkColorType</a>, with
<a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>, and with a <a href="#SkPixmap_width">width</a> and <a href="#SkPixmap_height">height</a> of zero. Use
<a href="#SkPixmap_reset">reset</a> to associate pixels, <a href="undocumented#SkColorType">SkColorType</a>, <a href="undocumented#SkAlphaType">SkAlphaType</a>, <a href="#SkPixmap_width">width</a>, and <a href="#SkPixmap_height">height</a>
after <a href="#Pixmap">Pixmap</a> has been created.

### Return Value

empty <a href="#Pixmap">Pixmap</a>

### Example

<div><fiddle-embed name="7befb4876e17f4bfc89ae8c54aef8660">

#### Example Output

~~~~
width:  0  height:  0  color: kUnknown_SkColorType  alpha: kUnknown_SkAlphaType
width: 25  height: 35  color: kRGBA_8888_SkColorType  alpha: kOpaque_SkAlphaType
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkPixmap_const_SkImageInfo_const_star">SkPixmap(const SkImageInfo& info, const void* addr, size t rowBytes)</a> <a href="#SkPixmap_reset">reset</a> <a href="undocumented#SkAlphaType">SkAlphaType</a> <a href="undocumented#SkColorType">SkColorType</a>

---

<a name="SkPixmap_const_SkImageInfo_const_star"></a>
## SkPixmap

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkPixmap(const SkImageInfo& info, const void* addr, size_t rowBytes)
</pre>

Creates <a href="#Pixmap">Pixmap</a> from <a href="#SkPixmap_info">info</a> <a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>, <a href="undocumented#SkAlphaType">SkAlphaType</a>, and <a href="undocumented#SkColorType">SkColorType</a>.
<a href="#SkPixmap_addr">addr</a> points to pixels, or nullptr. <a href="#SkPixmap_rowBytes">rowBytes</a> should be <a href="#SkPixmap_info">info</a>.<a href="#SkPixmap_width">width</a> times
<a href="#SkPixmap_info">info</a>.bytesPerPixel(), or larger.

No parameter checking is performed; it is up to the caller to ensure that
<a href="#SkPixmap_addr">addr</a> and <a href="#SkPixmap_rowBytes">rowBytes</a> agree with <a href="#SkPixmap_info">info</a>.

The memory lifetime of pixels is managed by the caller. When <a href="#Pixmap">Pixmap</a> goes
out of scope, <a href="#SkPixmap_addr">addr</a> is unaffected.

<a href="#Pixmap">Pixmap</a> may be later modified by <a href="#SkPixmap_reset">reset</a> to change its size, pixel type, or
storage.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_const_SkImageInfo_const_star_info"> <code><strong>info </strong></code> </a></td> <td>
<a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>, <a href="undocumented#SkAlphaType">SkAlphaType</a>, <a href="undocumented#SkColorType">SkColorType</a> of <a href="undocumented#Image_Info">Image Info</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_const_SkImageInfo_const_star_addr"> <code><strong>addr </strong></code> </a></td> <td>
pointer to pixels allocated by caller; may be nullptr</td>
  </tr>  <tr>    <td><a name="SkPixmap_const_SkImageInfo_const_star_rowBytes"> <code><strong>rowBytes </strong></code> </a></td> <td>
size of one row of <a href="#SkPixmap_addr">addr</a>; <a href="#SkPixmap_width">width</a> times pixel size, or larger</td>
  </tr>
</table>

### Return Value

initialized <a href="#Pixmap">Pixmap</a>

### Example

<div><fiddle-embed name="9a00774be57d7308313b3a9073e6e696"><div><a href="#SkImage_MakeRasterCopy">SkImage::MakeRasterCopy</a> takes const <a href="#SkPixmap">SkPixmap</a>& as an argument. The example
constructs a <a href="#SkPixmap">SkPixmap</a> from the brace-delimited parameters.</div>

#### Example Output

~~~~
image alpha only = false
copy alpha only = true
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkPixmap_empty_constructor">SkPixmap()</a> <a href="#SkPixmap_reset">reset</a> <a href="undocumented#SkAlphaType">SkAlphaType</a> <a href="undocumented#SkColorType">SkColorType</a>

---

<a name="SkPixmap_reset"></a>
## reset

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void reset()
</pre>

Sets <a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>, row bytes to zero; pixel address to nullptr; <a href="undocumented#SkColorType">SkColorType</a> to
<a href="undocumented#SkColorType">kUnknown SkColorType</a>; and <a href="undocumented#SkAlphaType">SkAlphaType</a> to <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>.

The prior pixels are unaffected; it is up to the caller to release pixels
memory if desired.

### Example

<div><fiddle-embed name="759feda9c22260f96567b9e2ccc1a118">

#### Example Output

~~~~
width: 25  height: 35  color: kRGBA_8888_SkColorType  alpha: kOpaque_SkAlphaType
width:  0  height:  0  color: kUnknown_SkColorType  alpha: kUnknown_SkAlphaType
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkPixmap_empty_constructor">SkPixmap()</a> <a href="undocumented#SkAlphaType">SkAlphaType</a> <a href="undocumented#SkColorType">SkColorType</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void reset(const SkImageInfo& info, const void* addr, size_t rowBytes)
</pre>

Sets <a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>, <a href="undocumented#SkAlphaType">SkAlphaType</a>, and <a href="undocumented#SkColorType">SkColorType</a> from <a href="#SkPixmap_info">info</a>.
Sets pixel address from <a href="#SkPixmap_addr">addr</a>, which may be nullptr.
Sets row bytes from <a href="#SkPixmap_rowBytes">rowBytes</a>, which should be <a href="#SkPixmap_info">info</a>.<a href="#SkPixmap_width">width</a> times
<a href="#SkPixmap_info">info</a>.bytesPerPixel(), or larger.

Does not check <a href="#SkPixmap_addr">addr</a>. Asserts if built with SK_DEBUG defined and if <a href="#SkPixmap_rowBytes">rowBytes</a> is
too small to hold one row of pixels.

The memory lifetime pixels are managed by the caller. When <a href="#Pixmap">Pixmap</a> goes
out of scope, <a href="#SkPixmap_addr">addr</a> is unaffected.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_reset_2_info"> <code><strong>info </strong></code> </a></td> <td>
<a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>, <a href="undocumented#SkAlphaType">SkAlphaType</a>, <a href="undocumented#SkColorType">SkColorType</a> of <a href="undocumented#Image_Info">Image Info</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_reset_2_addr"> <code><strong>addr </strong></code> </a></td> <td>
pointer to pixels allocated by caller; may be nullptr</td>
  </tr>  <tr>    <td><a name="SkPixmap_reset_2_rowBytes"> <code><strong>rowBytes </strong></code> </a></td> <td>
size of one row of <a href="#SkPixmap_addr">addr</a>; <a href="#SkPixmap_width">width</a> times pixel size, or larger</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a7c9bfe44f5d888ab5b9996f2b126788"></fiddle-embed></div>

### See Also

<a href="#SkPixmap_const_SkImageInfo_const_star">SkPixmap(const SkImageInfo& info, const void* addr, size t rowBytes)</a> <a href="#SkPixmap_reset">reset</a> <a href="undocumented#SkAlphaType">SkAlphaType</a> <a href="undocumented#SkColorType">SkColorType</a>

---

<a name="SkPixmap_setColorSpace"></a>
## setColorSpace

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setColorSpace(sk_sp&lt;SkColorSpace&gt; colorSpace)
</pre>

Changes <a href="undocumented#Color_Space">Color Space</a> in <a href="undocumented#Image_Info">Image Info</a>; preserves <a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>, <a href="undocumented#SkAlphaType">SkAlphaType</a>, and
<a href="undocumented#SkColorType">SkColorType</a> in <a href="SkImage_Reference#Image">Image</a>, and leaves pixel address and row bytes unchanged.
<a href="undocumented#Color_Space">Color Space</a> reference count is incremented.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_setColorSpace_colorSpace"> <code><strong>colorSpace </strong></code> </a></td> <td>
<a href="undocumented#Color_Space">Color Space</a> moved to <a href="undocumented#Image_Info">Image Info</a></td>
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

<a href="undocumented#Color_Space">Color Space</a> <a href="#SkImageInfo_makeColorSpace">SkImageInfo::makeColorSpace</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool SK_WARN_UNUSED_RESULT reset(const SkMask& mask)
</pre>

Sets <a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>, pixel address, and row bytes to <a href="undocumented#Mask">Mask</a> properties, if <a href="undocumented#Mask">Mask</a>
format is <a href="#SkMask_kA8_Format">SkMask::kA8 Format</a>; and returns true. Otherwise sets <a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>,
row bytes to zero; pixel address to nullptr; <a href="undocumented#SkColorType">SkColorType</a> to <a href="undocumented#SkColorType">kUnknown SkColorType</a>;
and <a href="undocumented#SkAlphaType">SkAlphaType</a> to <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>; and returns false.

Failing to read the return value generates a compile time warning.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_reset_3_mask"> <code><strong>mask </strong></code> </a></td> <td>
<a href="undocumented#Mask">Mask</a> containing pixels and dimensions</td>
  </tr>
</table>

### Return Value

true if set to <a href="undocumented#Mask">Mask</a> properties

### Example

<div><fiddle-embed name="379761a97bd7a116638a34eb3e80bf0d">

#### Example Output

~~~~
success: true  width: 2 height: 2
success: false width: 0 height: 0
~~~~

</fiddle-embed></div>

### See Also

<a href="undocumented#Mask">Mask</a> <a href="#SkPixmap_reset">reset</a>

---

<a name="SkPixmap_extractSubset"></a>
## extractSubset

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool SK_WARN_UNUSED_RESULT extractSubset(SkPixmap* subset, const SkIRect& area) const
</pre>

Sets <a href="#SkPixmap_extractSubset_subset">subset</a> <a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>, pixel address to intersection of <a href="#Pixmap">Pixmap</a> with <a href="#SkPixmap_extractSubset_area">area</a>,
if intersection is not empty; and return true. Otherwise, leave <a href="#SkPixmap_extractSubset_subset">subset</a> unchanged
and return false.

Failing to read the return value generates a compile time warning.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_extractSubset_subset"> <code><strong>subset </strong></code> </a></td> <td>
storage for <a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>, pixel address of intersection</td>
  </tr>  <tr>    <td><a name="SkPixmap_extractSubset_area"> <code><strong>area </strong></code> </a></td> <td>
<a href="#SkPixmap_bounds">bounds</a> to intersect with <a href="#Pixmap">Pixmap</a></td>
  </tr>
</table>

### Return Value

true if intersection of <a href="#Pixmap">Pixmap</a> and <a href="#SkPixmap_extractSubset_area">area</a> is not empty

### Example

<div><fiddle-embed name="febdbfac6cf4cde69837643be2e1f6dd"></fiddle-embed></div>

### See Also

<a href="#SkPixmap_reset">reset</a> <a href="#SkIRect_intersect">SkIRect::intersect</a>

---

## <a name="Image_Info_Access"></a> Image Info Access

<a name="SkPixmap_info"></a>
## info

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const SkImageInfo& info() const
</pre>

Returns <a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, and <a href="undocumented#Color_Space">Color Space</a>.

### Return Value

reference to ImageInfo

### Example

<div><fiddle-embed name="7294fbffeb15bf062b6ce989719b783f">

#### Example Output

~~~~
width: 384 height: 384 color: BGRA_8888 alpha: Opaque
~~~~

</fiddle-embed></div>

### See Also

<a href="undocumented#Image_Info">Image Info</a>

---

<a name="SkPixmap_rowBytes"></a>
## rowBytes

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
size_t rowBytes() const
</pre>

Returns row bytes, the interval from one pixel row to the next. Row bytes
is at least as large as:
<a href="#SkPixmap_width">width</a> * <a href="#SkPixmap_info">info</a>.bytesPerPixel().

Returns zero if <a href="#SkPixmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kUnknown SkColorType</a>.
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

<a href="#SkPixmap_addr">addr</a> <a href="#SkPixmap_info">info</a> <a href="#SkImageInfo_minRowBytes">SkImageInfo::minRowBytes</a>

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

<a href="#SkPixmap_addr_2">addr(int x, int y)</a> <a href="#SkPixmap_addr8">addr8</a> <a href="#SkPixmap_addr16">addr16</a> <a href="#SkPixmap_addr32">addr32</a> <a href="#SkPixmap_addr64">addr64</a> <a href="#SkPixmap_info">info</a> <a href="#SkPixmap_rowBytes">rowBytes</a>

---

<a name="SkPixmap_width"></a>
## width

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int width() const
</pre>

Returns pixel count in each pixel row. Should be equal or less than:

<a href="#SkPixmap_rowBytes">rowBytes</a> / <a href="#SkPixmap_info">info</a>.bytesPerPixel().

### Return Value

pixel <a href="#SkPixmap_width">width</a> in <a href="undocumented#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="f68617b7153a20b2ed3d7f9ed5c6e5e4">

#### Example Output

~~~~
pixmap width: 16  info width: 16
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkPixmap_height">height</a> <a href="#SkImageInfo_width">SkImageInfo::width()</a>

---

<a name="SkPixmap_height"></a>
## height

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int height() const
</pre>

Returns pixel row count.

### Return Value

pixel <a href="#SkPixmap_height">height</a> in <a href="undocumented#Image_Info">Image Info</a>

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

Returns <a href="undocumented#Image_Color_Type">Color Type</a>, one of: <a href="undocumented#SkColorType">kUnknown SkColorType</a>, <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>,
<a href="undocumented#SkColorType">kRGB 565 SkColorType</a>, <a href="undocumented#SkColorType">kARGB 4444 SkColorType</a>, <a href="undocumented#SkColorType">kRGBA 8888 SkColorType</a>,
<a href="undocumented#SkColorType">kBGRA 8888 SkColorType</a>, <a href="undocumented#SkColorType">kGray 8 SkColorType</a>, <a href="undocumented#SkColorType">kRGBA F16 SkColorType</a>.

### Return Value

<a href="undocumented#Image_Color_Type">Color Type</a> in <a href="undocumented#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="57cebd9fc88c4dfe04b20d0ff34f77fd">

#### Example Output

~~~~
color type: kAlpha_SkColorType
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkPixmap_alphaType">alphaType</a> <a href="#SkImageInfo_colorType">SkImageInfo::colorType</a>

---

<a name="SkPixmap_alphaType"></a>
## alphaType

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkAlphaType alphaType() const
</pre>

Returns <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, one of: <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>, <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>,
<a href="undocumented#SkAlphaType">kPremul SkAlphaType</a>, <a href="undocumented#SkAlphaType">kUnpremul SkAlphaType</a>.

### Return Value

<a href="undocumented#Image_Alpha_Type">Alpha Type</a> in <a href="undocumented#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="070b1a60232be499eb10c6ea62371804">

#### Example Output

~~~~
alpha type: kPremul_SkAlphaType
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkPixmap_colorType">colorType</a> <a href="#SkImageInfo_alphaType">SkImageInfo::alphaType</a>

---

<a name="SkPixmap_colorSpace"></a>
## colorSpace

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkColorSpace* colorSpace() const
</pre>

Returns <a href="undocumented#Color_Space">Color Space</a> associated with <a href="undocumented#Image_Info">Image Info</a>. The
reference count of <a href="undocumented#Color_Space">Color Space</a> is unchanged. The returned <a href="undocumented#Color_Space">Color Space</a> is
immutable.

### Return Value

<a href="undocumented#Color_Space">Color Space</a>, the range of colors, in <a href="undocumented#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="34c71f803b8edb48eaf1cd0c55bb212e"><div><a href="#SkColorSpace_MakeSRGBLinear">SkColorSpace::MakeSRGBLinear</a> creates <a href="undocumented#Color_Space">Color Space</a> with linear gamma
and an sRGB gamut. This <a href="undocumented#Color_Space">Color Space</a> gamma is not close to sRGB gamma.</div>

#### Example Output

~~~~
gammaCloseToSRGB: false  gammaIsLinear: true  isSRGB: false
~~~~

</fiddle-embed></div>

### See Also

<a href="undocumented#Color_Space">Color Space</a> <a href="#SkImageInfo_colorSpace">SkImageInfo::colorSpace</a>

---

<a name="SkPixmap_isOpaque"></a>
## isOpaque

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isOpaque() const
</pre>

Returns true if <a href="undocumented#Image_Alpha_Type">Alpha Type</a> is <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>.
Does not check if <a href="undocumented#Image_Color_Type">Color Type</a> allows <a href="#Alpha">Alpha</a>, or if any pixel value has
transparency.

### Return Value

true if <a href="undocumented#Image_Info">Image Info</a> has opaque <a href="undocumented#Image_Alpha_Type">Alpha Type</a>

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

<a href="#SkPixmap_computeIsOpaque">computeIsOpaque</a> <a href="#SkImageInfo_isOpaque">SkImageInfo::isOpaque</a>

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

<a href="#SkPixmap_rowBytes">rowBytes</a> <a href="#SkPixmap_shiftPerPixel">shiftPerPixel</a> <a href="#SkPixmap_width">width</a> <a href="#SkImageInfo_bytesPerPixel">SkImageInfo::bytesPerPixel</a>

---

<a name="SkPixmap_shiftPerPixel"></a>
## shiftPerPixel

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int shiftPerPixel() const
</pre>

Returns bit shift converting row bytes to row pixels.
Returns zero for <a href="undocumented#SkColorType">kUnknown SkColorType</a>.

### Return Value

one of: 0, 1, 2, 3; left shift to convert pixels to bytes

### Example

<div><fiddle-embed name="d91065811bb118dd686a2b94abe3360a">

#### Example Output

~~~~
color: kUnknown_SkColorType   bytesPerPixel: 0 shiftPerPixel: 0
color: kAlpha_SkColorType     bytesPerPixel: 1 shiftPerPixel: 0
color: kRGB_565_SkColorType   bytesPerPixel: 2 shiftPerPixel: 1
color: kARGB_4444_SkColorType bytesPerPixel: 2 shiftPerPixel: 1
color: kRGBA_8888_SkColorType bytesPerPixel: 4 shiftPerPixel: 2
color: kBGRA_8888_SkColorType bytesPerPixel: 4 shiftPerPixel: 2
color: kGray_8_SkColorType    bytesPerPixel: 1 shiftPerPixel: 0
color: kRGBA_F16_SkColorType  bytesPerPixel: 8 shiftPerPixel: 3
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkPixmap_rowBytes">rowBytes</a> <a href="#SkPixmap_rowBytesAsPixels">rowBytesAsPixels</a> <a href="#SkPixmap_width">width</a> <a href="#SkImageInfo_bytesPerPixel">SkImageInfo::bytesPerPixel</a>

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
Returns <a href="#SkPixmap_height">height</a> times <a href="#SkPixmap_rowBytes">rowBytes</a> if <a href="#SkPixmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kUnknown SkColorType</a>.

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

<a href="#SkImageInfo_computeByteSize">SkImageInfo::computeByteSize</a>

---

## <a name="Reader"></a> Reader

<a name="SkPixmap_computeIsOpaque"></a>
## computeIsOpaque

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool computeIsOpaque() const
</pre>

Returns true if all pixels are opaque. <a href="undocumented#Image_Color_Type">Color Type</a> determines how pixels
are encoded, and whether pixel describes <a href="#Alpha">Alpha</a>. Returns true for <a href="undocumented#Image_Color_Type">Color Types</a>
without alpha in each pixel; for other <a href="undocumented#Image_Color_Type">Color Types</a>, returns true if all
pixels have alpha values equivalent to 1.0 or greater.

For <a href="undocumented#Image_Color_Type">Color Types</a> <a href="undocumented#SkColorType">kRGB 565 SkColorType</a> or <a href="undocumented#SkColorType">kGray 8 SkColorType</a>: always
returns true. For <a href="undocumented#Image_Color_Type">Color Types</a> <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>, <a href="undocumented#SkColorType">kBGRA 8888 SkColorType</a>,
<a href="undocumented#SkColorType">kRGBA 8888 SkColorType</a>: returns true if all pixel <a href="#Alpha">Alpha</a> values are 255.
For <a href="undocumented#Image_Color_Type">Color Type</a> <a href="undocumented#SkColorType">kARGB 4444 SkColorType</a>: returns true if all pixel <a href="#Alpha">Alpha</a> values are 15.
For <a href="undocumented#SkColorType">kRGBA F16 SkColorType</a>: returns true if all pixel <a href="#Alpha">Alpha</a> values are 1.0 or
greater.

Returns false for <a href="undocumented#SkColorType">kUnknown SkColorType</a>.

### Return Value

true if all pixels have opaque values or <a href="undocumented#Image_Color_Type">Color Type</a> is opaque

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

<a href="#SkPixmap_isOpaque">isOpaque</a> <a href="undocumented#Image_Color_Type">Color Type</a> <a href="#Alpha">Alpha</a>

---

<a name="SkPixmap_getColor"></a>
## getColor

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkColor getColor(int x, int y) const
</pre>

Returns pixel at (<a href="#SkPixmap_getColor_x">x</a>, <a href="#SkPixmap_getColor_y">y</a>) as <a href="#Unpremultiply">Unpremultiplied</a> <a href="undocumented#Color">Color</a>.
Returns black with <a href="#Alpha">Alpha</a> if <a href="undocumented#Image_Color_Type">Color Type</a> is <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>.

Input is not validated: out of <a href="#SkPixmap_bounds">bounds</a> values of <a href="#SkPixmap_getColor_x">x</a> or <a href="#SkPixmap_getColor_y">y</a> trigger an assert() if
built with SK_DEBUG defined; and returns undefined values or may crash if
SK_RELEASE is defined. Fails if <a href="undocumented#Image_Color_Type">Color Type</a> is <a href="undocumented#SkColorType">kUnknown SkColorType</a> or
pixel address is nullptr.

<a href="undocumented#Color_Space">Color Space</a> in <a href="undocumented#Image_Info">Image Info</a> is ignored. Some <a href="undocumented#Color">Color</a> precision may be lost in the
conversion to <a href="#Unpremultiply">Unpremultiplied</a> <a href="undocumented#Color">Color</a>; original pixel data may have additional
precision.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_getColor_x"> <code><strong>x </strong></code> </a></td> <td>
column index, zero or greater, and less than <a href="#SkPixmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_getColor_y"> <code><strong>y </strong></code> </a></td> <td>
row index, zero or greater, and less than <a href="#SkPixmap_height">height</a></td>
  </tr>
</table>

### Return Value

pixel converted to <a href="#Unpremultiply">Unpremultiplied</a> <a href="undocumented#Color">Color</a>

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

<a href="#SkPixmap_addr">addr</a> <a href="#SkPixmap_readPixels">readPixels</a>

---

## <a name="Readable_Address"></a> Readable Address

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const void* addr(int x, int y) const
</pre>

Returns readable pixel address at (<a href="#SkPixmap_addr_2_x">x</a>, <a href="#SkPixmap_addr_2_y">y</a>). Returns nullptr if <a href="undocumented#Pixel_Ref">Pixel Ref</a> is nullptr.

Input is not validated: out of <a href="#SkPixmap_bounds">bounds</a> values of <a href="#SkPixmap_addr_2_x">x</a> or <a href="#SkPixmap_addr_2_y">y</a> trigger an assert() if
built with SK_DEBUG defined. Returns nullptr if <a href="undocumented#Image_Color_Type">Color Type</a> is <a href="undocumented#SkColorType">kUnknown SkColorType</a>.

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

<a href="#SkPixmap_addr8">addr8</a> <a href="#SkPixmap_addr16">addr16</a> <a href="#SkPixmap_addr32">addr32</a> <a href="#SkPixmap_addr64">addr64</a> <a href="#SkPixmap_addrF16">addrF16</a> <a href="#SkPixmap_getColor">getColor</a> <a href="#SkPixmap_writable_addr">writable addr</a> <a href="#SkBitmap_getAddr">SkBitmap::getAddr</a>

---

<a name="SkPixmap_addr8"></a>
## addr8

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const uint8_t* addr8() const
</pre>

Returns readable base pixel address. Result is addressable as unsigned 8-bit bytes.
Will trigger an assert() if <a href="undocumented#Image_Color_Type">Color Type</a> is not <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a> or
<a href="undocumented#SkColorType">kGray 8 SkColorType</a>, and is built with SK_DEBUG defined.

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

<a href="#SkPixmap_addr">addr</a> <a href="#SkPixmap_addr16">addr16</a> <a href="#SkPixmap_addr32">addr32</a> <a href="#SkPixmap_addr64">addr64</a> <a href="#SkPixmap_addrF16">addrF16</a> <a href="#SkPixmap_getColor">getColor</a> <a href="#SkPixmap_writable_addr">writable addr</a> <a href="#SkPixmap_writable_addr8">writable addr8</a>

---

<a name="SkPixmap_addr16"></a>
## addr16

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const uint16_t* addr16() const
</pre>

Returns readable base pixel address. Result is addressable as unsigned 16-bit words.
Will trigger an assert() if <a href="undocumented#Image_Color_Type">Color Type</a> is not <a href="undocumented#SkColorType">kRGB 565 SkColorType</a> or
<a href="undocumented#SkColorType">kARGB 4444 SkColorType</a>, and is built with SK_DEBUG defined.

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

<a href="#SkPixmap_addr">addr</a> <a href="#SkPixmap_addr8">addr8</a> <a href="#SkPixmap_addr32">addr32</a> <a href="#SkPixmap_addr64">addr64</a> <a href="#SkPixmap_addrF16">addrF16</a> <a href="#SkPixmap_getColor">getColor</a> <a href="#SkPixmap_writable_addr">writable addr</a> <a href="#SkPixmap_writable_addr16">writable addr16</a>

---

<a name="SkPixmap_addr32"></a>
## addr32

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const uint32_t* addr32() const
</pre>

Returns readable base pixel address. Result is addressable as unsigned 32-bit words.
Will trigger an assert() if <a href="undocumented#Image_Color_Type">Color Type</a> is not <a href="undocumented#SkColorType">kRGBA 8888 SkColorType</a> or
<a href="undocumented#SkColorType">kBGRA 8888 SkColorType</a>, and is built with SK_DEBUG defined.

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

<a href="#SkPixmap_addr">addr</a> <a href="#SkPixmap_addr8">addr8</a> <a href="#SkPixmap_addr16">addr16</a> <a href="#SkPixmap_addr64">addr64</a> <a href="#SkPixmap_addrF16">addrF16</a> <a href="#SkPixmap_getColor">getColor</a> <a href="#SkPixmap_writable_addr">writable addr</a> <a href="#SkPixmap_writable_addr32">writable addr32</a>

---

<a name="SkPixmap_addr64"></a>
## addr64

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const uint64_t* addr64() const
</pre>

Returns readable base pixel address. Result is addressable as unsigned 64-bit words.
Will trigger an assert() if <a href="undocumented#Image_Color_Type">Color Type</a> is not <a href="undocumented#SkColorType">kRGBA F16 SkColorType</a> and is built
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

<a href="#SkPixmap_addr">addr</a> <a href="#SkPixmap_addr8">addr8</a> <a href="#SkPixmap_addr16">addr16</a> <a href="#SkPixmap_addr32">addr32</a> <a href="#SkPixmap_addrF16">addrF16</a> <a href="#SkPixmap_getColor">getColor</a> <a href="#SkPixmap_writable_addr">writable addr</a> <a href="#SkPixmap_writable_addr64">writable addr64</a>

---

<a name="SkPixmap_addrF16"></a>
## addrF16

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const uint16_t* addrF16() const
</pre>

Returns readable base pixel address. Result is addressable as unsigned 16-bit words.
Will trigger an assert() if <a href="undocumented#Image_Color_Type">Color Type</a> is not <a href="undocumented#SkColorType">kRGBA F16 SkColorType</a> and is built
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

<a href="#SkPixmap_addr">addr</a> <a href="#SkPixmap_addr8">addr8</a> <a href="#SkPixmap_addr16">addr16</a> <a href="#SkPixmap_addr32">addr32</a> <a href="#SkPixmap_addr64">addr64</a> <a href="#SkPixmap_getColor">getColor</a> <a href="#SkPixmap_writable_addr">writable addr</a> <a href="#SkPixmap_writable_addrF16">writable addrF16</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const uint8_t* addr8(int x, int y) const
</pre>

Returns readable pixel address at (<a href="#SkPixmap_addr8_2_x">x</a>, <a href="#SkPixmap_addr8_2_y">y</a>).

Input is not validated: out of <a href="#SkPixmap_bounds">bounds</a> values of <a href="#SkPixmap_addr8_2_x">x</a> or <a href="#SkPixmap_addr8_2_y">y</a> trigger an assert() if
built with SK_DEBUG defined.

Will trigger an assert() if <a href="undocumented#Image_Color_Type">Color Type</a> is not <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a> or
<a href="undocumented#SkColorType">kGray 8 SkColorType</a>, and is built with SK_DEBUG defined.

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

<a href="#SkPixmap_addr">addr</a> <a href="#SkPixmap_addr16">addr16</a> <a href="#SkPixmap_addr32">addr32</a> <a href="#SkPixmap_addr64">addr64</a> <a href="#SkPixmap_addrF16">addrF16</a> <a href="#SkPixmap_getColor">getColor</a> <a href="#SkPixmap_writable_addr">writable addr</a> <a href="#SkPixmap_writable_addr8">writable addr8</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const uint16_t* addr16(int x, int y) const
</pre>

Returns readable pixel address at (<a href="#SkPixmap_addr16_2_x">x</a>, <a href="#SkPixmap_addr16_2_y">y</a>).

Input is not validated: out of <a href="#SkPixmap_bounds">bounds</a> values of <a href="#SkPixmap_addr16_2_x">x</a> or <a href="#SkPixmap_addr16_2_y">y</a> trigger an assert() if
built with SK_DEBUG defined.

Will trigger an assert() if <a href="undocumented#Image_Color_Type">Color Type</a> is not <a href="undocumented#SkColorType">kRGB 565 SkColorType</a> or
<a href="undocumented#SkColorType">kARGB 4444 SkColorType</a>, and is built with SK_DEBUG defined.

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

<a href="#SkPixmap_addr">addr</a> <a href="#SkPixmap_addr8">addr8</a> <a href="#SkPixmap_addr32">addr32</a> <a href="#SkPixmap_addr64">addr64</a> <a href="#SkPixmap_addrF16">addrF16</a> <a href="#SkPixmap_getColor">getColor</a> <a href="#SkPixmap_writable_addr">writable addr</a> <a href="#SkPixmap_writable_addr16">writable addr16</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const uint32_t* addr32(int x, int y) const
</pre>

Returns readable pixel address at (<a href="#SkPixmap_addr32_2_x">x</a>, <a href="#SkPixmap_addr32_2_y">y</a>).

Input is not validated: out of <a href="#SkPixmap_bounds">bounds</a> values of <a href="#SkPixmap_addr32_2_x">x</a> or <a href="#SkPixmap_addr32_2_y">y</a> trigger an assert() if
built with SK_DEBUG defined.

Will trigger an assert() if <a href="undocumented#Image_Color_Type">Color Type</a> is not <a href="undocumented#SkColorType">kRGBA 8888 SkColorType</a> or
<a href="undocumented#SkColorType">kBGRA 8888 SkColorType</a>, and is built with SK_DEBUG defined.

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

<a href="#SkPixmap_addr">addr</a> <a href="#SkPixmap_addr8">addr8</a> <a href="#SkPixmap_addr16">addr16</a> <a href="#SkPixmap_addr64">addr64</a> <a href="#SkPixmap_addrF16">addrF16</a> <a href="#SkPixmap_getColor">getColor</a> <a href="#SkPixmap_writable_addr">writable addr</a> <a href="#SkPixmap_writable_addr64">writable addr64</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const uint64_t* addr64(int x, int y) const
</pre>

Returns readable pixel address at (<a href="#SkPixmap_addr64_2_x">x</a>, <a href="#SkPixmap_addr64_2_y">y</a>).

Input is not validated: out of <a href="#SkPixmap_bounds">bounds</a> values of <a href="#SkPixmap_addr64_2_x">x</a> or <a href="#SkPixmap_addr64_2_y">y</a> trigger an assert() if
built with SK_DEBUG defined.

Will trigger an assert() if <a href="undocumented#Image_Color_Type">Color Type</a> is not <a href="undocumented#SkColorType">kRGBA F16 SkColorType</a> and is built
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

<a href="#SkPixmap_addr">addr</a> <a href="#SkPixmap_addr8">addr8</a> <a href="#SkPixmap_addr16">addr16</a> <a href="#SkPixmap_addr32">addr32</a> <a href="#SkPixmap_addrF16">addrF16</a> <a href="#SkPixmap_getColor">getColor</a> <a href="#SkPixmap_writable_addr">writable addr</a> <a href="#SkPixmap_writable_addr64">writable addr64</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const uint16_t* addrF16(int x, int y) const
</pre>

Returns readable pixel address at (<a href="#SkPixmap_addrF16_2_x">x</a>, <a href="#SkPixmap_addrF16_2_y">y</a>).

Input is not validated: out of <a href="#SkPixmap_bounds">bounds</a> values of <a href="#SkPixmap_addrF16_2_x">x</a> or <a href="#SkPixmap_addrF16_2_y">y</a> trigger an assert() if
built with SK_DEBUG defined.

Will trigger an assert() if <a href="undocumented#Image_Color_Type">Color Type</a> is not <a href="undocumented#SkColorType">kRGBA F16 SkColorType</a> and is built
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

<a href="#SkPixmap_addr">addr</a> <a href="#SkPixmap_addr8">addr8</a> <a href="#SkPixmap_addr16">addr16</a> <a href="#SkPixmap_addr32">addr32</a> <a href="#SkPixmap_addr64">addr64</a> <a href="#SkPixmap_getColor">getColor</a> <a href="#SkPixmap_writable_addr">writable addr</a> <a href="#SkPixmap_writable_addrF16">writable addrF16</a>

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

<a href="#SkPixmap_writable_addr8">writable addr8</a> <a href="#SkPixmap_writable_addr16">writable addr16</a> <a href="#SkPixmap_writable_addr32">writable addr32</a> <a href="#SkPixmap_writable_addr64">writable addr64</a> <a href="#SkPixmap_writable_addrF16">writable addrF16</a> <a href="#SkPixmap_addr">addr</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void* writable_addr(int x, int y) const
</pre>

Returns writable pixel address at (<a href="#SkPixmap_writable_addr_2_x">x</a>, <a href="#SkPixmap_writable_addr_2_y">y</a>).

Input is not validated: out of <a href="#SkPixmap_bounds">bounds</a> values of <a href="#SkPixmap_writable_addr_2_x">x</a> or <a href="#SkPixmap_writable_addr_2_y">y</a> trigger an assert() if
built with SK_DEBUG defined. Returns zero if <a href="undocumented#Image_Color_Type">Color Type</a> is <a href="undocumented#SkColorType">kUnknown SkColorType</a>.

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

<a href="#SkPixmap_writable_addr8">writable addr8</a> <a href="#SkPixmap_writable_addr16">writable addr16</a> <a href="#SkPixmap_writable_addr32">writable addr32</a> <a href="#SkPixmap_writable_addr64">writable addr64</a> <a href="#SkPixmap_writable_addrF16">writable addrF16</a> <a href="#SkPixmap_addr">addr</a>

---

<a name="SkPixmap_writable_addr8"></a>
## writable_addr8

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
uint8_t* writable_addr8(int x, int y) const
</pre>

Returns writable pixel address at (<a href="#SkPixmap_writable_addr8_x">x</a>, <a href="#SkPixmap_writable_addr8_y">y</a>). Result is addressable as unsigned
8-bit bytes. Will trigger an assert() if <a href="undocumented#Image_Color_Type">Color Type</a> is not <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>
or <a href="undocumented#SkColorType">kGray 8 SkColorType</a>, and is built with SK_DEBUG defined.

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
drawing on all platforms. Adding a second <a href="#SkBitmap_installPixels">SkBitmap::installPixels</a> after editing
pixel memory is safer.</div></fiddle-embed></div>

### See Also

<a href="#SkPixmap_writable_addr">writable addr</a> <a href="#SkPixmap_writable_addr16">writable addr16</a> <a href="#SkPixmap_writable_addr32">writable addr32</a> <a href="#SkPixmap_writable_addr64">writable addr64</a> <a href="#SkPixmap_writable_addrF16">writable addrF16</a> <a href="#SkPixmap_addr">addr</a> <a href="#SkPixmap_addr8">addr8</a>

---

<a name="SkPixmap_writable_addr16"></a>
## writable_addr16

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
uint16_t* writable_addr16(int x, int y) const
</pre>

Returns <a href="#SkPixmap_writable_addr">writable addr</a> pixel address at (<a href="#SkPixmap_writable_addr16_x">x</a>, <a href="#SkPixmap_writable_addr16_y">y</a>). Result is addressable as unsigned
16-bit words. Will trigger an assert() if <a href="undocumented#Image_Color_Type">Color Type</a> is not <a href="undocumented#SkColorType">kRGB 565 SkColorType</a>
or <a href="undocumented#SkColorType">kARGB 4444 SkColorType</a>, and is built with SK_DEBUG defined.

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
The low nibble of the 16-bit word is <a href="#Alpha">Alpha</a>.</div></fiddle-embed></div>

### See Also

<a href="#SkPixmap_writable_addr">writable addr</a> <a href="#SkPixmap_writable_addr8">writable addr8</a> <a href="#SkPixmap_writable_addr32">writable addr32</a> <a href="#SkPixmap_writable_addr64">writable addr64</a> <a href="#SkPixmap_writable_addrF16">writable addrF16</a> <a href="#SkPixmap_addr">addr</a> <a href="#SkPixmap_addr16">addr16</a>

---

<a name="SkPixmap_writable_addr32"></a>
## writable_addr32

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
uint32_t* writable_addr32(int x, int y) const
</pre>

Returns writable pixel address at (<a href="#SkPixmap_writable_addr32_x">x</a>, <a href="#SkPixmap_writable_addr32_y">y</a>). Result is addressable as unsigned
32-bit words. Will trigger an assert() if <a href="undocumented#Image_Color_Type">Color Type</a> is not
<a href="undocumented#SkColorType">kRGBA 8888 SkColorType</a> or <a href="undocumented#SkColorType">kBGRA 8888 SkColorType</a>, and is built with SK_DEBUG
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

<a href="#SkPixmap_writable_addr">writable addr</a> <a href="#SkPixmap_writable_addr8">writable addr8</a> <a href="#SkPixmap_writable_addr16">writable addr16</a> <a href="#SkPixmap_writable_addr64">writable addr64</a> <a href="#SkPixmap_writable_addrF16">writable addrF16</a> <a href="#SkPixmap_addr">addr</a> <a href="#SkPixmap_addr32">addr32</a>

---

<a name="SkPixmap_writable_addr64"></a>
## writable_addr64

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
uint64_t* writable_addr64(int x, int y) const
</pre>

Returns writable pixel address at (<a href="#SkPixmap_writable_addr64_x">x</a>, <a href="#SkPixmap_writable_addr64_y">y</a>). Result is addressable as unsigned
64-bit words. Will trigger an assert() if <a href="undocumented#Image_Color_Type">Color Type</a> is not
<a href="undocumented#SkColorType">kRGBA F16 SkColorType</a> and is built with SK_DEBUG defined.

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

<a href="#SkPixmap_writable_addr">writable addr</a> <a href="#SkPixmap_writable_addr8">writable addr8</a> <a href="#SkPixmap_writable_addr16">writable addr16</a> <a href="#SkPixmap_writable_addr32">writable addr32</a> <a href="#SkPixmap_writable_addrF16">writable addrF16</a> <a href="#SkPixmap_addr">addr</a> <a href="#SkPixmap_addr64">addr64</a>

---

<a name="SkPixmap_writable_addrF16"></a>
## writable_addrF16

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
uint16_t* writable_addrF16(int x, int y) const
</pre>

Returns writable pixel address at (<a href="#SkPixmap_writable_addrF16_x">x</a>, <a href="#SkPixmap_writable_addrF16_y">y</a>). Result is addressable as unsigned
16-bit words. Will trigger an assert() if <a href="undocumented#Image_Color_Type">Color Type</a> is not
<a href="undocumented#SkColorType">kRGBA F16 SkColorType</a> and is built with SK_DEBUG defined.

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

<a href="#SkPixmap_writable_addr">writable addr</a> <a href="#SkPixmap_writable_addr8">writable addr8</a> <a href="#SkPixmap_writable_addr16">writable addr16</a> <a href="#SkPixmap_writable_addr32">writable addr32</a> <a href="#SkPixmap_writable_addr64">writable addr64</a> <a href="#SkPixmap_addr">addr</a> <a href="#SkPixmap_addrF16">addrF16</a>

---

## <a name="Writer"></a> Writer

<a name="SkPixmap_readPixels"></a>
## readPixels

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes, int srcX, int srcY,
                SkTransferFunctionBehavior behavior) const
</pre>

Copies a <a href="SkRect_Reference#Rect">Rect</a> of pixels to <a href="#SkPixmap_readPixels_dstPixels">dstPixels</a>. Copy starts at (<a href="#SkPixmap_readPixels_srcX">srcX</a>, <a href="#SkPixmap_readPixels_srcY">srcY</a>), and does not
exceed <a href="#Pixmap">Pixmap</a> (<a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>).

<a href="#SkPixmap_readPixels_dstInfo">dstInfo</a> specifies <a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, and
<a href="undocumented#Color_Space">Color Space</a> of destination. <a href="#SkPixmap_readPixels_dstRowBytes">dstRowBytes</a> specifics the gap from one destination
row to the next. Returns true if pixels are copied. Returns false if
<a href="#SkPixmap_readPixels_dstInfo">dstInfo</a>.<a href="#SkPixmap_addr">addr</a> equals nullptr, or <a href="#SkPixmap_readPixels_dstRowBytes">dstRowBytes</a> is less than <a href="#SkPixmap_readPixels_dstInfo">dstInfo</a>.<a href="undocumented#SkImageInfo">minRowBytes</a>.

Pixels are copied only if pixel conversion is possible. If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorType">colorType</a> is
<a href="undocumented#SkColorType">kGray 8 SkColorType</a>, or <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>; <a href="#SkPixmap_readPixels_dstInfo">dstInfo</a>.<a href="#SkPixmap_colorType">colorType</a> must match.
If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kGray 8 SkColorType</a>, <a href="#SkPixmap_readPixels_dstInfo">dstInfo</a>.<a href="#SkPixmap_colorSpace">colorSpace</a> must match.
If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_alphaType">alphaType</a> is <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>, <a href="#SkPixmap_readPixels_dstInfo">dstInfo</a>.<a href="#SkPixmap_alphaType">alphaType</a> must
match. If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorSpace">colorSpace</a> is nullptr, <a href="#SkPixmap_readPixels_dstInfo">dstInfo</a>.<a href="#SkPixmap_colorSpace">colorSpace</a> must match. Returns
false if pixel conversion is not possible.
<a href="#SkPixmap_readPixels_srcX">srcX</a> and <a href="#SkPixmap_readPixels_srcY">srcY</a> may be negative to copy only top or left of source. Returns
false if <a href="#SkPixmap_width">width</a> or <a href="#SkPixmap_height">height</a> is zero or negative. Returns false if:

abs(srcX) >= <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_width">width</a>,
or ifabs(srcY) >= <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_height">height</a>.

If <a href="#SkPixmap_readPixels_behavior">behavior</a> is <a href="#SkTransferFunctionBehavior_kRespect">SkTransferFunctionBehavior::kRespect</a>: converts source
pixels to a linear space before converting to <a href="#SkPixmap_readPixels_dstInfo">dstInfo</a>.
If <a href="#SkPixmap_readPixels_behavior">behavior</a> is <a href="#SkTransferFunctionBehavior_kIgnore">SkTransferFunctionBehavior::kIgnore</a>: source
pixels are treated as if they are linear, regardless of how they are encoded.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_readPixels_dstInfo"> <code><strong>dstInfo </strong></code> </a></td> <td>
destination <a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_readPixels_dstPixels"> <code><strong>dstPixels </strong></code> </a></td> <td>
destination pixel storage</td>
  </tr>  <tr>    <td><a name="SkPixmap_readPixels_dstRowBytes"> <code><strong>dstRowBytes </strong></code> </a></td> <td>
destination row length</td>
  </tr>  <tr>    <td><a name="SkPixmap_readPixels_srcX"> <code><strong>srcX </strong></code> </a></td> <td>
column index whose absolute value is less than <a href="#SkPixmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_readPixels_srcY"> <code><strong>srcY </strong></code> </a></td> <td>
row index whose absolute value is less than <a href="#SkPixmap_height">height</a></td>
  </tr>  <tr>    <td><a name="SkPixmap_readPixels_behavior"> <code><strong>behavior </strong></code> </a></td> <td>
one of: <a href="#SkTransferFunctionBehavior_kRespect">SkTransferFunctionBehavior::kRespect</a>,
<a href="#SkTransferFunctionBehavior_kIgnore">SkTransferFunctionBehavior::kIgnore</a></td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href="#SkPixmap_readPixels_dstPixels">dstPixels</a>

### Example

<div><fiddle-embed name="2b7f6cc59ea2d5ebceddccbc2f232bcf"></fiddle-embed></div>

### See Also

<a href="#SkPixmap_erase">erase</a> <a href="#SkBitmap_readPixels">SkBitmap::readPixels</a> <a href="#SkCanvas_drawBitmap">SkCanvas::drawBitmap</a> <a href="#SkCanvas_readPixels">SkCanvas::readPixels</a> <a href="#SkImage_readPixels">SkImage::readPixels</a> <a href="#SkSurface_readPixels">SkSurface::readPixels</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes) const
</pre>

Copies a <a href="SkRect_Reference#Rect">Rect</a> of pixels to <a href="#SkPixmap_readPixels_2_dstPixels">dstPixels</a>. Copy starts at (0, 0), and does not
exceed <a href="#Pixmap">Pixmap</a> (<a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>).

<a href="#SkPixmap_readPixels_2_dstInfo">dstInfo</a> specifies <a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, and
<a href="undocumented#Color_Space">Color Space</a> of destination. <a href="#SkPixmap_readPixels_2_dstRowBytes">dstRowBytes</a> specifics the gap from one destination
row to the next. Returns true if pixels are copied. Returns false if
<a href="#SkPixmap_readPixels_2_dstInfo">dstInfo</a>.<a href="#SkPixmap_addr">addr</a> equals nullptr, or <a href="#SkPixmap_readPixels_2_dstRowBytes">dstRowBytes</a> is less than <a href="#SkPixmap_readPixels_2_dstInfo">dstInfo</a>.<a href="undocumented#SkImageInfo">minRowBytes</a>.

Pixels are copied only if pixel conversion is possible. If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorType">colorType</a> is
<a href="undocumented#SkColorType">kGray 8 SkColorType</a>, or <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>; <a href="#SkPixmap_readPixels_2_dstInfo">dstInfo</a>.<a href="#SkPixmap_colorType">colorType</a> must match.
If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kGray 8 SkColorType</a>, <a href="#SkPixmap_readPixels_2_dstInfo">dstInfo</a>.<a href="#SkPixmap_colorSpace">colorSpace</a> must match.
If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_alphaType">alphaType</a> is <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>, <a href="#SkPixmap_readPixels_2_dstInfo">dstInfo</a>.<a href="#SkPixmap_alphaType">alphaType</a> must
match. If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorSpace">colorSpace</a> is nullptr, <a href="#SkPixmap_readPixels_2_dstInfo">dstInfo</a>.<a href="#SkPixmap_colorSpace">colorSpace</a> must match. Returns
false if pixel conversion is not possible.

Returns false if <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_width">width</a> or <a href="#SkPixmap_height">height</a> is zero or negative.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_readPixels_2_dstInfo"> <code><strong>dstInfo </strong></code> </a></td> <td>
destination <a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a></td>
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

<a href="#SkPixmap_erase">erase</a> <a href="#SkBitmap_readPixels">SkBitmap::readPixels</a> <a href="#SkCanvas_drawBitmap">SkCanvas::drawBitmap</a> <a href="#SkCanvas_readPixels">SkCanvas::readPixels</a> <a href="#SkImage_readPixels">SkImage::readPixels</a> <a href="#SkSurface_readPixels">SkSurface::readPixels</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes, int srcX, int srcY) const
</pre>

Copies a <a href="SkRect_Reference#Rect">Rect</a> of pixels to <a href="#SkPixmap_readPixels_3_dstPixels">dstPixels</a>. Copy starts at (<a href="#SkPixmap_readPixels_3_srcX">srcX</a>, <a href="#SkPixmap_readPixels_3_srcY">srcY</a>), and does not
exceed <a href="#Pixmap">Pixmap</a> (<a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>).

<a href="#SkPixmap_readPixels_3_dstInfo">dstInfo</a> specifies <a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, and
<a href="undocumented#Color_Space">Color Space</a> of destination. <a href="#SkPixmap_readPixels_3_dstRowBytes">dstRowBytes</a> specifics the gap from one destination
row to the next. Returns true if pixels are copied. Returns false if
<a href="#SkPixmap_readPixels_3_dstInfo">dstInfo</a>.<a href="#SkPixmap_addr">addr</a> equals nullptr, or <a href="#SkPixmap_readPixels_3_dstRowBytes">dstRowBytes</a> is less than <a href="#SkPixmap_readPixels_3_dstInfo">dstInfo</a>.<a href="undocumented#SkImageInfo">minRowBytes</a>.

Pixels are copied only if pixel conversion is possible. If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorType">colorType</a> is
<a href="undocumented#SkColorType">kGray 8 SkColorType</a>, or <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>; <a href="#SkPixmap_readPixels_3_dstInfo">dstInfo</a>.<a href="#SkPixmap_colorType">colorType</a> must match.
If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kGray 8 SkColorType</a>, <a href="#SkPixmap_readPixels_3_dstInfo">dstInfo</a>.<a href="#SkPixmap_colorSpace">colorSpace</a> must match.
If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_alphaType">alphaType</a> is <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>, <a href="#SkPixmap_readPixels_3_dstInfo">dstInfo</a>.<a href="#SkPixmap_alphaType">alphaType</a> must
match. If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorSpace">colorSpace</a> is nullptr, <a href="#SkPixmap_readPixels_3_dstInfo">dstInfo</a>.<a href="#SkPixmap_colorSpace">colorSpace</a> must match. Returns
false if pixel conversion is not possible.
<a href="#SkPixmap_readPixels_3_srcX">srcX</a> and <a href="#SkPixmap_readPixels_3_srcY">srcY</a> may be negative to copy only top or left of source. Returns
false if <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_width">width</a> or <a href="#SkPixmap_height">height</a> is zero or negative. Returns false if:

abs(srcX) >= <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_width">width</a>,
or ifabs(srcY) >= <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_height">height</a>.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_readPixels_3_dstInfo"> <code><strong>dstInfo </strong></code> </a></td> <td>
destination <a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a></td>
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

<a href="#SkPixmap_erase">erase</a> <a href="#SkBitmap_readPixels">SkBitmap::readPixels</a> <a href="#SkCanvas_drawBitmap">SkCanvas::drawBitmap</a> <a href="#SkCanvas_readPixels">SkCanvas::readPixels</a> <a href="#SkImage_readPixels">SkImage::readPixels</a> <a href="#SkSurface_readPixels">SkSurface::readPixels</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool readPixels(const SkPixmap& dst, int srcX, int srcY) const
</pre>

Copies a <a href="SkRect_Reference#Rect">Rect</a> of pixels to <a href="#SkPixmap_readPixels_4_dst">dst</a>. Copy starts at (<a href="#SkPixmap_readPixels_4_srcX">srcX</a>, <a href="#SkPixmap_readPixels_4_srcY">srcY</a>), and does not
exceed <a href="#Pixmap">Pixmap</a> (<a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>). <a href="#SkPixmap_readPixels_4_dst">dst</a> specifies <a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>, <a href="undocumented#Image_Color_Type">Color Type</a>,
<a href="undocumented#Image_Alpha_Type">Alpha Type</a>, and <a href="undocumented#Color_Space">Color Space</a> of destination.  Returns true if pixels are copied.
Returns false if <a href="#SkPixmap_readPixels_4_dst">dst</a>.<a href="#SkPixmap_addr">addr</a> equals nullptr, or <a href="#SkPixmap_readPixels_4_dst">dst</a>.<a href="#SkPixmap_rowBytes">rowBytes</a> is less than
<a href="#SkPixmap_readPixels_4_dst">dst</a> <a href="#SkImageInfo_minRowBytes">SkImageInfo::minRowBytes</a>.

Pixels are copied only if pixel conversion is possible. If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorType">colorType</a> is
<a href="undocumented#SkColorType">kGray 8 SkColorType</a>, or <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>; <a href="#SkPixmap_readPixels_4_dst">dst</a>.<a href="#SkPixmap_info">info</a>.<a href="#SkPixmap_colorType">colorType</a> must match.
If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kGray 8 SkColorType</a>, <a href="#SkPixmap_readPixels_4_dst">dst</a>.<a href="#SkPixmap_info">info</a>.<a href="#SkPixmap_colorSpace">colorSpace</a> must match.
If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_alphaType">alphaType</a> is <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>, <a href="#SkPixmap_readPixels_4_dst">dst</a>.<a href="#SkPixmap_info">info</a>.<a href="#SkPixmap_alphaType">alphaType</a> must
match. If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorSpace">colorSpace</a> is nullptr, <a href="#SkPixmap_readPixels_4_dst">dst</a>.<a href="#SkPixmap_info">info</a>.<a href="#SkPixmap_colorSpace">colorSpace</a> must match. Returns
false if pixel conversion is not possible.
<a href="#SkPixmap_readPixels_4_srcX">srcX</a> and <a href="#SkPixmap_readPixels_4_srcY">srcY</a> may be negative to copy only top or left of source. Returns
false <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_width">width</a> or <a href="#SkPixmap_height">height</a> is zero or negative. Returns false if:

abs(srcX) >= <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_width">width</a>,
or ifabs(srcY) >= <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_height">height</a>.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_readPixels_4_dst"> <code><strong>dst </strong></code> </a></td> <td>
<a href="undocumented#Image_Info">Image Info</a> and pixel address to write to</td>
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

<a href="#SkPixmap_erase">erase</a> <a href="#SkBitmap_readPixels">SkBitmap::readPixels</a> <a href="#SkCanvas_drawBitmap">SkCanvas::drawBitmap</a> <a href="#SkCanvas_readPixels">SkCanvas::readPixels</a> <a href="#SkImage_readPixels">SkImage::readPixels</a> <a href="#SkSurface_readPixels">SkSurface::readPixels</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool readPixels(const SkPixmap& dst) const
</pre>

Copies pixels inside <a href="#SkPixmap_bounds">bounds</a> to <a href="#SkPixmap_readPixels_5_dst">dst</a>. <a href="#SkPixmap_readPixels_5_dst">dst</a> specifies <a href="#SkPixmap_width">width</a>, <a href="#SkPixmap_height">height</a>, <a href="undocumented#Image_Color_Type">Color Type</a>,
<a href="undocumented#Image_Alpha_Type">Alpha Type</a>, and <a href="undocumented#Color_Space">Color Space</a> of destination.  Returns true if pixels are copied.
Returns false if <a href="#SkPixmap_readPixels_5_dst">dst</a>.<a href="#SkPixmap_addr">addr</a> equals nullptr, or <a href="#SkPixmap_readPixels_5_dst">dst</a>.<a href="#SkPixmap_rowBytes">rowBytes</a> is less than
<a href="#SkPixmap_readPixels_5_dst">dst</a> <a href="#SkImageInfo_minRowBytes">SkImageInfo::minRowBytes</a>.

Pixels are copied only if pixel conversion is possible. If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorType">colorType</a> is
<a href="undocumented#SkColorType">kGray 8 SkColorType</a>, or <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>; <a href="#SkPixmap_readPixels_5_dst">dst</a> <a href="undocumented#Image_Color_Type">Color Type</a> must match.
If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kGray 8 SkColorType</a>, <a href="#SkPixmap_readPixels_5_dst">dst</a> <a href="undocumented#Color_Space">Color Space</a> must match.
If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_alphaType">alphaType</a> is <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>, <a href="#SkPixmap_readPixels_5_dst">dst</a> <a href="undocumented#Image_Alpha_Type">Alpha Type</a> must
match. If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorSpace">colorSpace</a> is nullptr, <a href="#SkPixmap_readPixels_5_dst">dst</a> <a href="undocumented#Color_Space">Color Space</a> must match. Returns
false if pixel conversion is not possible.
Returns false if <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_width">width</a> or <a href="#SkPixmap_height">height</a> is zero or negative.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_readPixels_5_dst"> <code><strong>dst </strong></code> </a></td> <td>
<a href="undocumented#Image_Info">Image Info</a> and pixel address to write to</td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href="#SkPixmap_readPixels_5_dst">dst</a>

### Example

<div><fiddle-embed name="e18549b5ee1039cb61b0bb38c2104fc9"></fiddle-embed></div>

### See Also

<a href="#SkPixmap_erase">erase</a> <a href="#SkBitmap_readPixels">SkBitmap::readPixels</a> <a href="#SkCanvas_drawBitmap">SkCanvas::drawBitmap</a> <a href="#SkCanvas_readPixels">SkCanvas::readPixels</a> <a href="#SkImage_readPixels">SkImage::readPixels</a> <a href="#SkSurface_readPixels">SkSurface::readPixels</a>

---

<a name="SkPixmap_scalePixels"></a>
## scalePixels

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool scalePixels(const SkPixmap& dst, SkFilterQuality filterQuality) const
</pre>

Copies <a href="SkBitmap_Reference#Bitmap">Bitmap</a> to <a href="#SkPixmap_scalePixels_dst">dst</a>, scaling pixels to fit <a href="#SkPixmap_scalePixels_dst">dst</a>.<a href="#SkPixmap_width">width</a> and <a href="#SkPixmap_scalePixels_dst">dst</a>.<a href="#SkPixmap_height">height</a>, and
converting pixels to match <a href="#SkPixmap_scalePixels_dst">dst</a>.<a href="#SkPixmap_colorType">colorType</a> and <a href="#SkPixmap_scalePixels_dst">dst</a>.<a href="#SkPixmap_alphaType">alphaType</a>. Returns true if
pixels are copied. Returns false if <a href="#SkPixmap_scalePixels_dst">dst</a>.<a href="#SkPixmap_addr">addr</a> is nullptr, or <a href="#SkPixmap_scalePixels_dst">dst</a>.<a href="#SkPixmap_rowBytes">rowBytes</a> is
less than <a href="#SkPixmap_scalePixels_dst">dst</a> <a href="#SkImageInfo_minRowBytes">SkImageInfo::minRowBytes</a>.

Pixels are copied only if pixel conversion is possible. If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorType">colorType</a> is
<a href="undocumented#SkColorType">kGray 8 SkColorType</a>, or <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>; <a href="#SkPixmap_scalePixels_dst">dst</a> <a href="undocumented#Image_Color_Type">Color Type</a> must match.
If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kGray 8 SkColorType</a>, <a href="#SkPixmap_scalePixels_dst">dst</a> <a href="undocumented#Color_Space">Color Space</a> must match.
If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_alphaType">alphaType</a> is <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>, <a href="#SkPixmap_scalePixels_dst">dst</a> <a href="undocumented#Image_Alpha_Type">Alpha Type</a> must
match. If <a href="#Pixmap">Pixmap</a> <a href="#SkPixmap_colorSpace">colorSpace</a> is nullptr, <a href="#SkPixmap_scalePixels_dst">dst</a> <a href="undocumented#Color_Space">Color Space</a> must match. Returns
false if pixel conversion is not possible.

Returns false if <a href="SkBitmap_Reference#Bitmap">Bitmap</a> <a href="#SkPixmap_width">width</a> or <a href="#SkPixmap_height">height</a> is zero or negative.

Scales the image, with <a href="#SkPixmap_scalePixels_filterQuality">filterQuality</a>, to match <a href="#SkPixmap_scalePixels_dst">dst</a>.<a href="#SkPixmap_width">width</a> and <a href="#SkPixmap_scalePixels_dst">dst</a>.<a href="#SkPixmap_height">height</a>.
<a href="#SkPixmap_scalePixels_filterQuality">filterQuality</a> <a href="undocumented#SkFilterQuality">kNone SkFilterQuality</a> is fastest, typically implemented with
<a href="undocumented#Filter_Quality_Nearest_Neighbor">Filter Quality Nearest Neighbor</a>. <a href="undocumented#SkFilterQuality">kLow SkFilterQuality</a> is typically implemented with
<a href="undocumented#Filter_Quality_Bilerp">Filter Quality Bilerp</a>. <a href="undocumented#SkFilterQuality">kMedium SkFilterQuality</a> is typically implemented with
<a href="undocumented#Filter_Quality_Bilerp">Filter Quality Bilerp</a>, and <a href="undocumented#Filter_Quality_MipMap">Filter Quality MipMap</a> when size is reduced.
<a href="undocumented#SkFilterQuality">kHigh SkFilterQuality</a> is slowest, typically implemented with <a href="undocumented#Filter_Quality_BiCubic">Filter Quality BiCubic</a>.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_scalePixels_dst"> <code><strong>dst </strong></code> </a></td> <td>
<a href="undocumented#Image_Info">Image Info</a> and pixel address to write to</td>
  </tr>  <tr>    <td><a name="SkPixmap_scalePixels_filterQuality"> <code><strong>filterQuality </strong></code> </a></td> <td>
one of: <a href="undocumented#SkFilterQuality">kNone SkFilterQuality</a>, <a href="undocumented#SkFilterQuality">kLow SkFilterQuality</a>,
<a href="undocumented#SkFilterQuality">kMedium SkFilterQuality</a>, <a href="undocumented#SkFilterQuality">kHigh SkFilterQuality</a></td>
  </tr>
</table>

### Return Value

true if pixels are scaled to fit <a href="#SkPixmap_scalePixels_dst">dst</a>

### Example

<div><fiddle-embed name="8e3c8a9c1d0d2e9b8bf66e24d274f792"></fiddle-embed></div>

### See Also

<a href="#SkCanvas_drawBitmap">SkCanvas::drawBitmap</a> <a href="#SkImage_scalePixels">SkImage::scalePixels</a>

---

<a name="SkPixmap_erase"></a>
## erase

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool erase(SkColor color, const SkIRect& subset) const
</pre>

Writes <a href="#SkPixmap_erase_color">color</a> to pixels bounded by <a href="#SkPixmap_erase_subset">subset</a>; returns true on success.
Returns false if <a href="#SkPixmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kUnknown SkColorType</a>, or if <a href="#SkPixmap_erase_subset">subset</a> does
not intersect <a href="#SkPixmap_bounds">bounds</a>.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_erase_color"> <code><strong>color </strong></code> </a></td> <td>
<a href="#Unpremultiply">Unpremultiplied</a> <a href="undocumented#Color">Color</a> to write</td>
  </tr>  <tr>    <td><a name="SkPixmap_erase_subset"> <code><strong>subset </strong></code> </a></td> <td>
bounding integer <a href="SkRect_Reference#Rect">Rect</a> of written pixels</td>
  </tr>
</table>

### Return Value

true if pixels are changed

### Example

<div><fiddle-embed name="a0cdbafed4786788cc90681e7b294234"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_erase">SkBitmap::erase</a> <a href="#SkCanvas_clear">SkCanvas::clear</a> <a href="#SkCanvas_drawColor">SkCanvas::drawColor</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool erase(SkColor color) const
</pre>

Writes <a href="#SkPixmap_erase_2_color">color</a> to pixels inside <a href="#SkPixmap_bounds">bounds</a>; returns true on success.
Returns false if <a href="#SkPixmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kUnknown SkColorType</a>, or if <a href="#SkPixmap_bounds">bounds</a>
is empty.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_erase_2_color"> <code><strong>color </strong></code> </a></td> <td>
<a href="#Unpremultiply">Unpremultiplied</a> <a href="undocumented#Color">Color</a> to write</td>
  </tr>
</table>

### Return Value

true if pixels are changed

### Example

<div><fiddle-embed name="838202e0d49cad2eb3eeb495834f6d63"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_erase">SkBitmap::erase</a> <a href="#SkCanvas_clear">SkCanvas::clear</a> <a href="#SkCanvas_drawColor">SkCanvas::drawColor</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool erase(const SkColor4f& color, const SkIRect* subset = nullptr) const
</pre>

Writes <a href="#SkPixmap_erase_3_color">color</a> to pixels bounded by <a href="#SkPixmap_erase_3_subset">subset</a>; returns true on success.
if <a href="#SkPixmap_erase_3_subset">subset</a> is nullptr, writes colors pixels inside <a href="#SkPixmap_bounds">bounds</a>. Returns false if
<a href="#SkPixmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kUnknown SkColorType</a>, if <a href="#SkPixmap_erase_3_subset">subset</a> is not nullptr and does
not intersect <a href="#SkPixmap_bounds">bounds</a>, or if <a href="#SkPixmap_erase_3_subset">subset</a> is nullptr and <a href="#SkPixmap_bounds">bounds</a> is empty.

### Parameters

<table>  <tr>    <td><a name="SkPixmap_erase_3_color"> <code><strong>color </strong></code> </a></td> <td>
<a href="#Unpremultiply">Unpremultiplied</a> <a href="undocumented#Color">Color</a> to write</td>
  </tr>  <tr>    <td><a name="SkPixmap_erase_3_subset"> <code><strong>subset </strong></code> </a></td> <td>
bounding integer <a href="SkRect_Reference#Rect">Rect</a> of pixels to write; may be nullptr</td>
  </tr>
</table>

### Return Value

true if pixels are changed

### Example

<div><fiddle-embed name="f884f3f46a565f052a5e252ae2f36e9b"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_erase">SkBitmap::erase</a> <a href="#SkCanvas_clear">SkCanvas::clear</a> <a href="#SkCanvas_drawColor">SkCanvas::drawColor</a>

---

