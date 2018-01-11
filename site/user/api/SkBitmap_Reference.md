SkBitmap Reference
===

# <a name="Bitmap"></a> Bitmap

# <a name="SkBitmap"></a> Class SkBitmap
<a href="#Bitmap">Bitmap</a> describes a two-dimensional raster pixel array. <a href="#Bitmap">Bitmap</a> is built on
<a href="undocumented#Image_Info">Image Info</a>, containing integer <a href="#SkBitmap_width">width</a> and <a href="#SkBitmap_height">height</a>, <a href="undocumented#Image_Color_Type">Color Type</a> and <a href="undocumented#Image_Alpha_Type">Alpha Type</a>
describing the pixel format, and <a href="undocumented#Color_Space">Color Space</a> describing the range of colors.
<a href="#Bitmap">Bitmap</a> points to <a href="undocumented#Pixel_Ref">Pixel Ref</a>, which describes the physical array of pixels.
<a href="undocumented#Image_Info">Image Info</a> <a href="#SkBitmap_bounds">bounds</a> may be located anywhere fully inside <a href="undocumented#Pixel_Ref">Pixel Ref</a> <a href="#SkBitmap_bounds">bounds</a>.

<a href="#Bitmap">Bitmap</a> can be drawn using <a href="SkCanvas_Reference#Canvas">Canvas</a>. <a href="#Bitmap">Bitmap</a> can be a drawing destination for <a href="SkCanvas_Reference#Canvas">Canvas</a>
draw methods. <a href="#Bitmap">Bitmap</a> flexibility as a pixel container limits some optimizations
available to the target platform.

If pixel array is primarily read-only, use <a href="SkImage_Reference#Image">Image</a> for better performance.
If pixel array is primarily written to, use <a href="SkSurface_Reference#Surface">Surface</a> for better performance.

Declaring <a href="#SkBitmap">SkBitmap</a> const prevents altering <a href="undocumented#Image_Info">Image Info</a>: the <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_height">height</a>, <a href="#SkBitmap_width">width</a>,
and so on cannot change. It does not affect <a href="undocumented#Pixel_Ref">Pixel Ref</a>: a caller may write its
pixels. Declaring <a href="#SkBitmap">SkBitmap</a> const affects <a href="#Bitmap">Bitmap</a> configuration, not its contents.

<a href="#Bitmap">Bitmap</a> is not thread safe. Each thread must have its own copy of <a href="#Bitmap">Bitmap</a> fields,
although threads may share the underlying pixel array.

# <a name="Row_Bytes"></a> Row Bytes
<a href="#Bitmap">Bitmap</a> pixels may be contiguous, or may have a gap at the end of each row.
<a href="#Row_Bytes">Row Bytes</a> is the interval from one row to the next. <a href="#Row_Bytes">Row Bytes</a> may be specified;
sometimes passing zero will compute the <a href="#Row_Bytes">Row Bytes</a> from the row <a href="#SkBitmap_width">width</a> and the
number of bytes in a pixel. <a href="#Row_Bytes">Row Bytes</a> may be larger than the row requires. This
is useful to position one or more <a href="#Bitmap">Bitmaps</a> within a shared pixel array.

# <a name="Overview"></a> Overview

## <a name="Subtopics"></a> Subtopics

| topics | description |
| --- | ---  |

## <a name="Structs"></a> Structs

| description | struct |
| --- | ---  |
| <a href="#SkBitmap_Allocator">Allocator</a> |  |
| <a href="#SkBitmap_HeapAllocator">HeapAllocator</a> |  |

## <a name="Constructors"></a> Constructors

| description | function |
| --- | ---  |
| <a href="#SkBitmap_empty_constructor">SkBitmap()</a> | Constructs with default values. |
| <a href="#SkBitmap_move_SkBitmap">SkBitmap(SkBitmap&& src)</a> | Takes ownership of pixels. |
| <a href="#SkBitmap_copy_const_SkBitmap">SkBitmap(const SkBitmap& src)</a> | Shares ownership of pixels. |
|  | Releases ownership of pixels. |

## <a name="Operators"></a> Operators

| description | function |
| --- | ---  |
| <a href="#SkBitmap">SkBitmap</a>& <a href="#SkBitmap_move_operator">operator=(SkBitmap&& src)</a> | Takes ownership of pixels. |
| <a href="#SkBitmap">SkBitmap</a>& <a href="#SkBitmap_copy_operator">operator=(const SkBitmap& src)</a> | Shares ownership of pixels. |

## <a name="Member_Functions"></a> Member Functions

| description | function |
| --- | ---  |
| <a href="#SkBitmap_ComputeIsOpaque">ComputeIsOpaque</a> | Returns true if all pixels are opaque. |
| <a href="#SkBitmap_allocN32Pixels">allocN32Pixels</a> | Allocates compatible <a href="#ARGB">Color ARGB</a> pixels, or aborts. |
| <a href="#SkBitmap_allocPixels">allocPixels</a> | Allocates pixels from <a href="undocumented#Image_Info">Image Info</a>, or aborts. |
| <a href="#SkBitmap_allocPixelsFlags">allocPixelsFlags</a> | Allocates pixels from <a href="undocumented#Image_Info">Image Info</a> with options, or aborts. |
| <a href="#SkBitmap_alphaType">alphaType</a> | Returns <a href="undocumented#Image_Info">Image Info</a> <a href="undocumented#Image_Alpha_Type">Alpha Type</a>. |
| <a href="#SkBitmap_bounds">bounds</a> | Returns <a href="#SkBitmap_width">width</a> and <a href="#SkBitmap_height">height</a> as Rectangle. |
| <a href="#SkBitmap_bytesPerPixel">bytesPerPixel</a> | Returns number of bytes in pixel based on <a href="undocumented#Image_Color_Type">Color Type</a>. |
| <a href="#SkBitmap_colorSpace">colorSpace</a> | Returns <a href="undocumented#Image_Info">Image Info</a> <a href="undocumented#Color_Space">Color Space</a>. |
| <a href="#SkBitmap_colorType">colorType</a> | Returns <a href="undocumented#Image_Info">Image Info</a> <a href="undocumented#Image_Color_Type">Color Type</a>. |
| <a href="#SkBitmap_computeByteSize">computeByteSize</a> | Returns size required for pixels. |
| <a href="#SkBitmap_dimensions">dimensions</a> | Returns <a href="#SkBitmap_width">width</a> and <a href="#SkBitmap_height">height</a>. |
| <a href="#SkBitmap_drawsNothing">drawsNothing</a> | Returns true if no <a href="#SkBitmap_width">width</a>, no <a href="#SkBitmap_height">height</a>, or no <a href="undocumented#Pixel_Ref">Pixel Ref</a>. |
| <a href="#SkBitmap_empty">empty</a> | Returns true if <a href="undocumented#Image_Info">Image Info</a> has zero <a href="#SkBitmap_width">width</a> or <a href="#SkBitmap_height">height</a>. |
| <a href="#SkBitmap_erase">erase</a> | Writes <a href="undocumented#Color">Color</a> to rectangle of pixels. |
| <a href="#SkBitmap_eraseARGB">eraseARGB</a> | Writes <a href="undocumented#Color">Color</a> to pixels. |
| <a href="#SkBitmap_eraseArea">eraseArea</a> | Deprecated |
| <a href="#SkBitmap_eraseColor">eraseColor</a> | Writes <a href="undocumented#Color">Color</a> to pixels. |
| <a href="#SkBitmap_eraseRGB">eraseRGB</a> | Deprecated |
| <a href="#SkBitmap_extractAlpha">extractAlpha</a> | Creates <a href="#Bitmap">Bitmap</a> containing <a href="#Alpha">Alpha</a> of pixels. |
| <a href="#SkBitmap_extractSubset">extractSubset</a> | Creates <a href="#Bitmap">Bitmap</a>, sharing pixels if possible. |
| <a href="#SkBitmap_getAddr">getAddr</a> | Returns readable pixel address as void pointer. |
| <a href="#SkBitmap_getAddr16">getAddr16</a> | Returns readable pixel address as 16-bit pointer. |
| <a href="#SkBitmap_getAddr32">getAddr32</a> | Returns readable pixel address as 32-bit pointer. |
| <a href="#SkBitmap_getAddr8">getAddr8</a> | Returns readable pixel address as 8-bit pointer. |
| <a href="#SkBitmap_getBounds">getBounds</a> | Returns <a href="#SkBitmap_width">width</a> and <a href="#SkBitmap_height">height</a> as Rectangle. |
| <a href="#SkBitmap_getColor">getColor</a> | Returns one pixel as <a href="#Unpremultiply">Unpremultiplied</a> <a href="undocumented#Color">Color</a>. |
| <a href="#SkBitmap_getGenerationID">getGenerationID</a> | Returns unique ID. |
| <a href="#SkBitmap_getPixels">getPixels</a> | Returns address of pixels. |
| <a href="#SkBitmap_getSubset">getSubset</a> | Returns <a href="#SkBitmap_bounds">bounds</a> offset by origin. |
| <a href="#SkBitmap_hasHardwareMipMap">hasHardwareMipMap</a> | Returns <a href="undocumented#Mip_Map">Mip Map</a> support present; Android only. |
| <a href="#SkBitmap_height">height</a> | Returns pixel row count. |
| <a href="#SkBitmap_info">info</a> | Returns <a href="undocumented#Image_Info">Image Info</a>. |
| <a href="#SkBitmap_installMaskPixels">installMaskPixels</a> | Creates <a href="undocumented#Pixel_Ref">Pixel Ref</a> from <a href="undocumented#Mask">Mask</a>. |
| <a href="#SkBitmap_installPixels">installPixels</a> | Creates <a href="undocumented#Pixel_Ref">Pixel Ref</a>, with optional release function. |
| <a href="#SkBitmap_isImmutable">isImmutable</a> | Returns true if pixels will not change. |
| <a href="#SkBitmap_isNull">isNull</a> | Returns true if <a href="undocumented#Pixel_Ref">Pixel Ref</a> is nullptr. |
| <a href="#SkBitmap_isOpaque">isOpaque</a> | Returns true if <a href="undocumented#Image_Info">Image Info</a> describes opaque pixels. |
| <a href="#SkBitmap_isVolatile">isVolatile</a> | Returns true if pixels should not be cached. |
| <a href="#SkBitmap_notifyPixelsChanged">notifyPixelsChanged</a> | Marks pixels as changed, altering the unique ID. |
| <a href="#SkBitmap_peekPixels">peekPixels</a> | Returns <a href="SkPixmap_Reference#Pixmap">Pixmap</a> if possible. |
| <a href="#SkBitmap_pixelRef">pixelRef</a> | Returns <a href="undocumented#Pixel_Ref">Pixel Ref</a>, or nullptr. |
| <a href="#SkBitmap_pixelRefOrigin">pixelRefOrigin</a> | Returns offset within <a href="undocumented#Pixel_Ref">Pixel Ref</a>. |
| <a href="#SkBitmap_pixmap">pixmap</a> | Returns <a href="SkPixmap_Reference#Pixmap">Pixmap</a>. |
| <a href="#SkBitmap_readPixels">readPixels</a> | Copies and converts pixels. |
| <a href="#SkBitmap_readyToDraw">readyToDraw</a> | Returns true if address of pixels is not nullptr. |
| <a href="#SkBitmap_refColorSpace">refColorSpace</a> | Returns <a href="undocumented#Image_Info">Image Info</a> <a href="undocumented#Color_Space">Color Space</a>. |
| <a href="#SkBitmap_reset">reset</a> | Sets to default values, releases pixel ownership. |
| <a href="#SkBitmap_rowBytes">rowBytes</a> | Returns interval between rows in bytes. |
| <a href="#SkBitmap_rowBytesAsPixels">rowBytesAsPixels</a> | Returns interval between rows in pixels. |
| <a href="#SkBitmap_setAlphaType">setAlphaType</a> | Sets <a href="undocumented#Image_Alpha_Type">Alpha Type</a> of shared pixels. |
| <a href="#SkBitmap_setHasHardwareMipMap">setHasHardwareMipMap</a> | Sets <a href="undocumented#Mip_Map">Mip Map</a> support present; Android only. |
| <a href="#SkBitmap_setImmutable">setImmutable</a> | Marks that pixels will not change. |
| <a href="#SkBitmap_setInfo">setInfo</a> | Sets <a href="#SkBitmap_height">height</a>, <a href="#SkBitmap_width">width</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, and so on, releasing pixels. |
| <a href="#SkBitmap_setIsVolatile">setIsVolatile</a> | Marks if pixels should not be cached. |
| <a href="#SkBitmap_setPixelRef">setPixelRef</a> | Sets <a href="undocumented#Pixel_Ref">Pixel Ref</a> and offset. |
| <a href="#SkBitmap_setPixels">setPixels</a> | Sets <a href="undocumented#Pixel_Ref">Pixel Ref</a> without an offset. |
| <a href="#SkBitmap_shiftPerPixel">shiftPerPixel</a> | Returns bit shift from pixels to bytes. |
| <a href="#SkBitmap_swap">swap</a> | Exchanges <a href="#Bitmap">Bitmap</a> pair. |
| <a href="#SkBitmap_toString">toString</a> | Converts <a href="#Bitmap">Bitmap</a> to machine readable form. |
| <a href="#SkBitmap_tryAllocN32Pixels">tryAllocN32Pixels</a> | Allocates compatible <a href="#ARGB">Color ARGB</a> pixels if possible. |
| <a href="#SkBitmap_tryAllocPixels">tryAllocPixels</a> | Allocates pixels from <a href="undocumented#Image_Info">Image Info</a> if possible. |
| <a href="#SkBitmap_tryAllocPixelsFlags">tryAllocPixelsFlags</a> | Allocates pixels from <a href="undocumented#Image_Info">Image Info</a> with options if possible. |
| <a href="#SkBitmap_validate">validate</a> | Asserts if <a href="#Bitmap">Bitmap</a> is invalid (debug only). |
| <a href="#SkBitmap_width">width</a> | Returns pixel column count. |
| <a href="#SkBitmap_writePixels">writePixels</a> | Copies and converts pixels. |

# <a name="SkBitmap::Allocator"></a> Class SkBitmap::Allocator

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href="#SkBitmap_Allocator">Allocator</a> : public <a href="undocumented#SkRefCnt">SkRefCnt</a> {
public:
virtual bool <a href="#SkBitmap_Allocator_allocPixelRef">allocPixelRef(SkBitmap* bitmap)</a> = 0;
};</pre>

Abstract subclass of <a href="#SkBitmap_HeapAllocator">HeapAllocator</a>.

<a name="SkBitmap_Allocator_allocPixelRef"></a>
## allocPixelRef

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
virtual bool allocPixelRef(SkBitmap* bitmap) = 0
</pre>

Allocates the pixel memory for the <a href="#SkBitmap_Allocator_allocPixelRef_bitmap">bitmap</a>, given its <a href="#SkBitmap_dimensions">dimensions</a> and
<a href="undocumented#Image_Color_Type">Color Type</a>. Returns true on success, where success means either <a href="#SkBitmap_setPixels">setPixels</a>
or <a href="#SkBitmap_setPixelRef">setPixelRef</a> was called.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_Allocator_allocPixelRef_bitmap"> <code><strong>bitmap </strong></code> </a></td> <td>
<a href="#Bitmap">Bitmap</a> containing <a href="undocumented#Image_Info">Image Info</a> as input, and <a href="undocumented#Pixel_Ref">Pixel Ref</a> as output</td>
  </tr>
</table>

### Return Value

true if <a href="undocumented#Pixel_Ref">Pixel Ref</a> was allocated

### See Also

<a href="#SkBitmap_HeapAllocator">HeapAllocator</a>

---

# <a name="SkBitmap::HeapAllocator"></a> Class SkBitmap::HeapAllocator

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href="#SkBitmap_HeapAllocator">HeapAllocator</a> : public <a href="#SkBitmap_Allocator">Allocator</a> {
public:
bool <a href="#SkBitmap_HeapAllocator_allocPixelRef">allocPixelRef(SkBitmap* bitmap)</a> override;
};</pre>

Subclass of <a href="#SkBitmap_Allocator">Allocator</a> that returns a <a href="undocumented#Pixel_Ref">Pixel Ref</a> that allocates its pixel
memory from the heap. This is the default <a href="#SkBitmap_Allocator">Allocator</a> invoked by
<a href="#SkBitmap_allocPixels">allocPixels</a>.

<a name="SkBitmap_HeapAllocator_allocPixelRef"></a>
## allocPixelRef

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool allocPixelRef(SkBitmap* bitmap) override
</pre>

Allocates the pixel memory for the <a href="#SkBitmap_HeapAllocator_allocPixelRef_bitmap">bitmap</a>, given its <a href="#SkBitmap_dimensions">dimensions</a> and
<a href="undocumented#Image_Color_Type">Color Type</a>. Returns true on success, where success means either <a href="#SkBitmap_setPixels">setPixels</a>
or <a href="#SkBitmap_setPixelRef">setPixelRef</a> was called.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_HeapAllocator_allocPixelRef_bitmap"> <code><strong>bitmap </strong></code> </a></td> <td>
<a href="#Bitmap">Bitmap</a> containing <a href="undocumented#Image_Info">Image Info</a> as input, and <a href="undocumented#Pixel_Ref">Pixel Ref</a> as output</td>
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

<a href="#SkBitmap_Allocator">Allocator</a> <a href="#SkBitmap_tryAllocPixels">tryAllocPixels</a>

---

<a name="SkBitmap_empty_constructor"></a>
## SkBitmap

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkBitmap()
</pre>

Creates an <a href="#SkBitmap_empty">empty</a> <a href="#Bitmap">Bitmap</a> without pixels, with <a href="undocumented#SkColorType">kUnknown SkColorType</a>,
<a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>, and with a <a href="#SkBitmap_width">width</a> and <a href="#SkBitmap_height">height</a> of zero. <a href="undocumented#Pixel_Ref">Pixel Ref</a> origin is
set to (0, 0). <a href="#Bitmap">Bitmap</a> is not volatile.

Use <a href="#SkBitmap_setInfo">setInfo</a> to associate <a href="undocumented#SkColorType">SkColorType</a>, <a href="undocumented#SkAlphaType">SkAlphaType</a>, <a href="#SkBitmap_width">width</a>, and <a href="#SkBitmap_height">height</a>
after <a href="#Bitmap">Bitmap</a> has been created.

### Return Value

<a href="#SkBitmap_empty">empty</a> <a href="#Bitmap">Bitmap</a>

### Example

<div><fiddle-embed name="8bf225da00df80970271722cd3d1ced4">

#### Example Output

~~~~
width:  0  height:  0  color: kUnknown_SkColorType  alpha: kUnknown_SkAlphaType
width: 25  height: 35  color: kRGBA_8888_SkColorType  alpha: kOpaque_SkAlphaType
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkBitmap_setInfo">setInfo</a>

---

<a name="SkBitmap_copy_const_SkBitmap"></a>
## SkBitmap

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkBitmap(const SkBitmap& src)
</pre>

Copies settings from <a href="#SkBitmap_copy_const_SkBitmap_src">src</a> to returned <a href="#Bitmap">Bitmap</a>. Shares pixels if <a href="#SkBitmap_copy_const_SkBitmap_src">src</a> has pixels
allocated, so both bitmaps reference the same pixels.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_copy_const_SkBitmap_src"> <code><strong>src </strong></code> </a></td> <td>
<a href="#Bitmap">Bitmap</a> to copy <a href="undocumented#Image_Info">Image Info</a>, and share <a href="undocumented#Pixel_Ref">Pixel Ref</a></td>
  </tr>
</table>

### Return Value

copy of <a href="#SkBitmap_copy_const_SkBitmap_src">src</a>

### Example

<div><fiddle-embed name="dbf46a0b60324ec611cc18d86772e7b9">

#### Example Output

~~~~
original has pixels before copy: true
original has pixels after copy: true
copy has pixels: true
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkBitmap_setInfo">setInfo</a> <a href="#SkBitmap_setPixelRef">setPixelRef</a> <a href="#SkBitmap_setPixels">setPixels</a> <a href="#SkBitmap_swap">swap</a>

---

<a name="SkBitmap_move_SkBitmap"></a>
## SkBitmap

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkBitmap(SkBitmap&& src)
</pre>

Copies settings from <a href="#SkBitmap_move_SkBitmap_src">src</a> to returned <a href="#Bitmap">Bitmap</a>. Moves ownership of <a href="#SkBitmap_move_SkBitmap_src">src</a> pixels to
<a href="#Bitmap">Bitmap</a>.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_move_SkBitmap_src"> <code><strong>src </strong></code> </a></td> <td>
<a href="#Bitmap">Bitmap</a> to copy <a href="undocumented#Image_Info">Image Info</a>, and reassign <a href="undocumented#Pixel_Ref">Pixel Ref</a></td>
  </tr>
</table>

### Return Value

copy of <a href="#SkBitmap_move_SkBitmap_src">src</a>

### Example

<div><fiddle-embed name="8172a14e66cb763b7133f70dc4ff601e">

#### Example Output

~~~~
original has pixels before move: true
original has pixels after move: false
copy has pixels: true
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkBitmap_setInfo">setInfo</a> <a href="#SkBitmap_setPixelRef">setPixelRef</a> <a href="#SkBitmap_setPixels">setPixels</a> <a href="#SkBitmap_swap">swap</a>

---

<a name="SkBitmap_destructor"></a>
## ~SkBitmap

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
~SkBitmap()
</pre>

Decrements <a href="undocumented#Pixel_Ref">Pixel Ref</a> reference count, if <a href="undocumented#Pixel_Ref">Pixel Ref</a> is not nullptr.

### See Also

<a href="undocumented#Pixel_Ref">Pixel Ref</a>

---

<a name="SkBitmap_copy_operator"></a>
## operator=

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkBitmap& operator=(const SkBitmap& src)
</pre>

Copies settings from <a href="#SkBitmap_copy_operator_src">src</a> to returned <a href="#Bitmap">Bitmap</a>. Shares pixels if <a href="#SkBitmap_copy_operator_src">src</a> has pixels
allocated, so both bitmaps reference the same pixels.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_copy_operator_src"> <code><strong>src </strong></code> </a></td> <td>
<a href="#Bitmap">Bitmap</a> to copy <a href="undocumented#Image_Info">Image Info</a>, and share <a href="undocumented#Pixel_Ref">Pixel Ref</a></td>
  </tr>
</table>

### Return Value

copy of <a href="#SkBitmap_copy_operator_src">src</a>

### Example

<div><fiddle-embed name="6366fa1b1b3df1bebbfa3ed1c5a43457">

#### Example Output

~~~~
original has pixels before copy: true
original has pixels after copy: true
copy has pixels: true
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkBitmap_setInfo">setInfo</a> <a href="#SkBitmap_setPixelRef">setPixelRef</a> <a href="#SkBitmap_setPixels">setPixels</a> <a href="#SkBitmap_swap">swap</a>

---

<a name="SkBitmap_move_operator"></a>
## operator=

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkBitmap& operator=(SkBitmap&& src)
</pre>

Copies settings from <a href="#SkBitmap_move_operator_src">src</a> to returned <a href="#Bitmap">Bitmap</a>. Moves ownership of <a href="#SkBitmap_move_operator_src">src</a> pixels to
<a href="#Bitmap">Bitmap</a>.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_move_operator_src"> <code><strong>src </strong></code> </a></td> <td>
<a href="#Bitmap">Bitmap</a> to copy <a href="undocumented#Image_Info">Image Info</a>, and reassign <a href="undocumented#Pixel_Ref">Pixel Ref</a></td>
  </tr>
</table>

### Return Value

copy of <a href="#SkBitmap_move_operator_src">src</a>

### Example

<div><fiddle-embed name="5066583054077df5098bb1d2aa402149">

#### Example Output

~~~~
original has pixels before move: true
original has pixels after move: false
copy has pixels: true
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkBitmap_setInfo">setInfo</a> <a href="#SkBitmap_setPixelRef">setPixelRef</a> <a href="#SkBitmap_setPixels">setPixels</a> <a href="#SkBitmap_swap">swap</a>

---

<a name="SkBitmap_swap"></a>
## swap

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void swap(SkBitmap& other)
</pre>

Swaps the fields of the two bitmaps.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_swap_other"> <code><strong>other </strong></code> </a></td> <td>
<a href="#Bitmap">Bitmap</a> exchanged with original</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="aaf9157531f4969b4ec048e9edcdb069">

#### Example Output

~~~~
one width:1 height:1 colorType:kRGBA_8888_SkColorType alphaType:kOpaque_SkAlphaType
two width:2 height:2 colorType:kBGRA_8888_SkColorType alphaType:kPremul_SkAlphaType
one width:2 height:2 colorType:kBGRA_8888_SkColorType alphaType:kPremul_SkAlphaType
two width:1 height:1 colorType:kRGBA_8888_SkColorType alphaType:kOpaque_SkAlphaType
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkBitmap_move_SkBitmap">SkBitmap(SkBitmap&& src)</a> <a href="#SkBitmap_move_operator">operator=(SkBitmap&& src)</a>

---

<a name="SkBitmap_pixmap"></a>
## pixmap

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const SkPixmap& pixmap() const
</pre>

Returns a constant reference to the <a href="SkPixmap_Reference#Pixmap">Pixmap</a> holding the <a href="#Bitmap">Bitmap</a> pixel
address, row bytes, and <a href="undocumented#Image_Info">Image Info</a>.

### Return Value

reference to <a href="SkPixmap_Reference#Pixmap">Pixmap</a> describing this <a href="#Bitmap">Bitmap</a>

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

<a href="#SkBitmap_peekPixels">peekPixels</a> <a href="#SkBitmap_installPixels">installPixels</a> <a href="#SkBitmap_readPixels">readPixels</a> <a href="#SkBitmap_writePixels">writePixels</a>

---

<a name="SkBitmap_info"></a>
## info

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
const SkImageInfo& info() const
</pre>

Returns <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, and <a href="undocumented#Color_Space">Color Space</a>.

### Return Value

reference to <a href="undocumented#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="2932fccfb074cf52b17cec238f70d888">

#### Example Output

~~~~
width: 56 height: 56 color: BGRA_8888 alpha: Opaque
~~~~

</fiddle-embed></div>

### See Also

<a href="undocumented#Image_Info">Image Info</a>

---

<a name="SkBitmap_width"></a>
## width

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int width() const
</pre>

Returns pixel count in each row. Should be equal or less than:

<a href="#SkBitmap_rowBytes">rowBytes</a> / <a href="#SkBitmap_info">info</a>.<a href="#SkBitmap_bytesPerPixel">bytesPerPixel</a>.

Maybe be less than <a href="#SkBitmap_pixelRef">pixelRef</a>.<a href="#SkBitmap_width">width</a>. Will not exceed <a href="#SkBitmap_pixelRef">pixelRef</a>.<a href="#SkBitmap_width">width</a> less
<a href="#SkBitmap_pixelRefOrigin">pixelRefOrigin</a>.fX.

### Return Value

pixel <a href="#SkBitmap_width">width</a> in <a href="undocumented#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="d06880c42f8bb3b4c3b67bd988046049">

#### Example Output

~~~~
bitmap width: 16  info width: 16
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkBitmap_height">height</a> <a href="#SkPixelRef_width">SkPixelRef::width()</a> <a href="#SkImageInfo_width">SkImageInfo::width()</a>

---

<a name="SkBitmap_height"></a>
## height

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int height() const
</pre>

Returns pixel row count.

Maybe be less than <a href="#SkBitmap_pixelRef">pixelRef</a>.<a href="#SkBitmap_height">height</a>. Will not exceed <a href="#SkBitmap_pixelRef">pixelRef</a>.<a href="#SkBitmap_height">height</a> less
<a href="#SkBitmap_pixelRefOrigin">pixelRefOrigin</a>.fY.

### Return Value

pixel <a href="#SkBitmap_height">height</a> in <a href="undocumented#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="c79a196278c58b34cd5f551b0124ecc9">

#### Example Output

~~~~
bitmap height: 32  info height: 32
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkBitmap_width">width</a> <a href="#SkPixelRef_height">SkPixelRef::height()</a> <a href="#SkImageInfo_height">SkImageInfo::height()</a>

---

<a name="SkBitmap_colorType"></a>
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

<div><fiddle-embed name="8aa2ab0565091082a874c23886ddb0c5">

#### Example Output

~~~~
color type: kAlpha_SkColorType
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkBitmap_alphaType">alphaType</a> <a href="#SkImageInfo_colorType">SkImageInfo::colorType</a>

---

<a name="SkBitmap_alphaType"></a>
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

<a href="#SkBitmap_colorType">colorType</a> <a href="#SkImageInfo_alphaType">SkImageInfo::alphaType</a>

---

<a name="SkBitmap_colorSpace"></a>
## colorSpace

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkColorSpace* colorSpace() const
</pre>

Returns <a href="undocumented#Color_Space">Color Space</a>, the range of colors, associated with <a href="undocumented#Image_Info">Image Info</a>. The
reference count of <a href="undocumented#Color_Space">Color Space</a> is unchanged. The returned <a href="undocumented#Color_Space">Color Space</a> is
immutable.

### Return Value

<a href="undocumented#Color_Space">Color Space</a> in <a href="undocumented#Image_Info">Image Info</a>, or nullptr

### Example

<div><fiddle-embed name="7ade8a1a21ea5197e565c86740b47b73"><div><a href="#SkColorSpace_MakeSRGBLinear">SkColorSpace::MakeSRGBLinear</a> creates <a href="undocumented#Color_Space">Color Space</a> with linear gamma
and an sRGB gamut. This <a href="undocumented#Color_Space">Color Space</a> gamma is not close to sRGB gamma.</div>

#### Example Output

~~~~
gammaCloseToSRGB: false  gammaIsLinear: true  isSRGB: false
~~~~

</fiddle-embed></div>

### See Also

<a href="undocumented#Color_Space">Color Space</a> <a href="#SkImageInfo_colorSpace">SkImageInfo::colorSpace</a>

---

<a name="SkBitmap_refColorSpace"></a>
## refColorSpace

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkColorSpace&gt; refColorSpace() const
</pre>

Returns a smart pointer to <a href="undocumented#Color_Space">Color Space</a>, the range of colors, associated with
<a href="undocumented#Image_Info">Image Info</a>. The smart pointer tracks the number of objects sharing this
<a href="undocumented#Color_Space">Color Space</a> reference so the memory is released when the owners destruct.

The returned <a href="undocumented#Color_Space">Color Space</a> is immutable.

### Return Value

<a href="undocumented#Color_Space">Color Space</a> in <a href="undocumented#Image_Info">Image Info</a> wrapped in a smart pointer

### Example

<div><fiddle-embed name="972e9bb22c1ce94bb97b2d106168280e">

#### Example Output

~~~~
gammaCloseToSRGB: false  gammaIsLinear: true  isSRGB: false
~~~~

</fiddle-embed></div>

### See Also

<a href="undocumented#Color_Space">Color Space</a> <a href="#SkImageInfo_colorSpace">SkImageInfo::colorSpace</a>

---

<a name="SkBitmap_bytesPerPixel"></a>
## bytesPerPixel

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int bytesPerPixel() const
</pre>

Returns number of bytes per pixel required by <a href="undocumented#Image_Color_Type">Color Type</a>.
Returns zero if <a href="#SkBitmap_colorType">colorType</a>( is <a href="undocumented#SkColorType">kUnknown SkColorType</a>.

### Return Value

bytes in pixel

### Example

<div><fiddle-embed name="f09e181f3210d427e4b3d2ae6667ae6a">

#### Example Output

~~~~
color: kUnknown_SkColorType   bytesPerPixel: 0
color: kAlpha_SkColorType     bytesPerPixel: 1
color: kRGB_565_SkColorType   bytesPerPixel: 2
color: kARGB_4444_SkColorType bytesPerPixel: 2
color: kRGBA_8888_SkColorType bytesPerPixel: 4
color: kBGRA_8888_SkColorType bytesPerPixel: 4
color: kGray_8_SkColorType    bytesPerPixel: 1
color: kRGBA_F16_SkColorType  bytesPerPixel: 8
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkBitmap_rowBytes">rowBytes</a> <a href="#SkBitmap_rowBytesAsPixels">rowBytesAsPixels</a> <a href="#SkBitmap_width">width</a> <a href="#SkBitmap_shiftPerPixel">shiftPerPixel</a>

---

<a name="SkBitmap_rowBytesAsPixels"></a>
## rowBytesAsPixels

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int rowBytesAsPixels() const
</pre>

Returns number of pixels that fit on row. Should be greater than or equal to
<a href="#SkBitmap_width">width</a>.

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

<a href="#SkBitmap_rowBytes">rowBytes</a> <a href="#SkBitmap_shiftPerPixel">shiftPerPixel</a> <a href="#SkBitmap_width">width</a> <a href="#SkBitmap_bytesPerPixel">bytesPerPixel</a>

---

<a name="SkBitmap_shiftPerPixel"></a>
## shiftPerPixel

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int shiftPerPixel() const
</pre>

Returns bit shift converting row bytes to row pixels.
Returns zero for <a href="undocumented#SkColorType">kUnknown SkColorType</a>.

### Return Value

one of: 0, 1, 2, 3; left shift to convert pixels to bytes

### Example

<div><fiddle-embed name="8fa214f2b9a655d7b492181d2143538c">

#### Example Output

~~~~
color: kUnknown_SkColorType   shiftPerPixel: 0
color: kAlpha_SkColorType     shiftPerPixel: 0
color: kRGB_565_SkColorType   shiftPerPixel: 1
color: kARGB_4444_SkColorType shiftPerPixel: 1
color: kRGBA_8888_SkColorType shiftPerPixel: 2
color: kBGRA_8888_SkColorType shiftPerPixel: 2
color: kGray_8_SkColorType    shiftPerPixel: 0
color: kRGBA_F16_SkColorType  shiftPerPixel: 3
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkBitmap_rowBytes">rowBytes</a> <a href="#SkBitmap_rowBytesAsPixels">rowBytesAsPixels</a> <a href="#SkBitmap_width">width</a> <a href="#SkBitmap_bytesPerPixel">bytesPerPixel</a>

---

<a name="SkBitmap_empty"></a>
## empty

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool empty() const
</pre>

Returns true if either <a href="#SkBitmap_width">width</a> or <a href="#SkBitmap_height">height</a> are zero.

Does not check if <a href="undocumented#Pixel_Ref">Pixel Ref</a> is nullptr; call <a href="#SkBitmap_drawsNothing">drawsNothing</a> to check <a href="#SkBitmap_width">width</a>,
<a href="#SkBitmap_height">height</a>, and <a href="undocumented#Pixel_Ref">Pixel Ref</a>.

### Return Value

true if <a href="#SkBitmap_dimensions">dimensions</a> do not enclose area

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

<a href="#SkBitmap_height">height</a> <a href="#SkBitmap_width">width</a> <a href="#SkBitmap_drawsNothing">drawsNothing</a>

---

<a name="SkBitmap_isNull"></a>
## isNull

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isNull() const
</pre>

Return true if <a href="undocumented#Pixel_Ref">Pixel Ref</a> is nullptr.

Does not check if <a href="#SkBitmap_width">width</a> or <a href="#SkBitmap_height">height</a> are zero; call <a href="#SkBitmap_drawsNothing">drawsNothing</a> to check
<a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, and <a href="undocumented#Pixel_Ref">Pixel Ref</a>.

### Return Value

true if no <a href="undocumented#Pixel_Ref">Pixel Ref</a> is associated

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

<a href="#SkBitmap_empty">empty</a> <a href="#SkBitmap_drawsNothing">drawsNothing</a> <a href="#SkBitmap_pixelRef">pixelRef</a>

---

<a name="SkBitmap_drawsNothing"></a>
## drawsNothing

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool drawsNothing() const
</pre>

Return true if <a href="#SkBitmap_width">width</a> or <a href="#SkBitmap_height">height</a> are zero, or if <a href="undocumented#Pixel_Ref">Pixel Ref</a> is nullptr.
If true, <a href="#Bitmap">Bitmap</a> has no effect when drawn or drawn into.

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

<a href="#SkBitmap_empty">empty</a> <a href="#SkBitmap_isNull">isNull</a> <a href="#SkBitmap_pixelRef">pixelRef</a>

---

<a name="SkBitmap_rowBytes"></a>
## rowBytes

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
size_t rowBytes() const
</pre>

Returns row bytes, the interval from one pixel row to the next. Row bytes
is at least as large as<a href="#SkBitmap_width">width</a> * <a href="#SkBitmap_info">info</a>.<a href="#SkBitmap_bytesPerPixel">bytesPerPixel</a>.

Returns zero if <a href="#SkBitmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kUnknown SkColorType</a>, or if row bytes supplied to
<a href="#SkBitmap_setInfo">setInfo</a> is not large enough to hold a row of pixels.

### Return Value

byte length of pixel row

### Example

<div><fiddle-embed name="a654fd0b73f424859ae6c95e03f55099">

#### Example Output

~~~~
setInfo returned:false rowBytes:0
setInfo returned:true  rowBytes:8
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkBitmap_info">info</a> <a href="#SkBitmap_setInfo">setInfo</a> <a href="#SkImageInfo_minRowBytes">SkImageInfo::minRowBytes</a>

---

<a name="SkBitmap_setAlphaType"></a>
## setAlphaType

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool setAlphaType(SkAlphaType alphaType)
</pre>

Sets <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, if <a href="#SkBitmap_alphaType">alphaType</a> is compatible with <a href="undocumented#Image_Color_Type">Color Type</a>.
Returns true unless <a href="#SkBitmap_alphaType">alphaType</a> is <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a> and current <a href="undocumented#Image_Alpha_Type">Alpha Type</a>
is not <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>.

Returns true if <a href="undocumented#Image_Color_Type">Color Type</a> is <a href="undocumented#SkColorType">kUnknown SkColorType</a>. <a href="#SkBitmap_alphaType">alphaType</a> is ignored, and
<a href="undocumented#Image_Alpha_Type">Alpha Type</a> remains <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>.

Returns true if <a href="undocumented#Image_Color_Type">Color Type</a> is <a href="undocumented#SkColorType">kRGB 565 SkColorType</a> or <a href="undocumented#SkColorType">kGray 8 SkColorType</a>.
<a href="#SkBitmap_alphaType">alphaType</a> is ignored, and <a href="undocumented#Image_Alpha_Type">Alpha Type</a> remains <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>.

If <a href="undocumented#Image_Color_Type">Color Type</a> is <a href="undocumented#SkColorType">kARGB 4444 SkColorType</a>, <a href="undocumented#SkColorType">kRGBA 8888 SkColorType</a>,
<a href="undocumented#SkColorType">kBGRA 8888 SkColorType</a>, or <a href="undocumented#SkColorType">kRGBA F16 SkColorType</a>: returns true unless
<a href="#SkBitmap_alphaType">alphaType</a> is <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a> and <a href="undocumented#Image_Alpha_Type">Alpha Type</a> is not <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>.
If <a href="undocumented#Image_Alpha_Type">Alpha Type</a> is <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>, <a href="#SkBitmap_alphaType">alphaType</a> is ignored.

If <a href="undocumented#Image_Color_Type">Color Type</a> is <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>, returns true unless
<a href="#SkBitmap_alphaType">alphaType</a> is <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a> and <a href="undocumented#Image_Alpha_Type">Alpha Type</a> is not <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>.
If <a href="undocumented#Image_Alpha_Type">Alpha Type</a> is <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>, <a href="#SkBitmap_alphaType">alphaType</a> is ignored. If <a href="#SkBitmap_alphaType">alphaType</a> is
<a href="undocumented#SkAlphaType">kUnpremul SkAlphaType</a>, it is treated as <a href="undocumented#SkAlphaType">kPremul SkAlphaType</a>.

This changes <a href="undocumented#Image_Alpha_Type">Alpha Type</a> in <a href="undocumented#Pixel_Ref">Pixel Ref</a>; all bitmaps sharing <a href="undocumented#Pixel_Ref">Pixel Ref</a>
are affected.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_setAlphaType_alphaType"> <code><strong>alphaType </strong></code> </a></td> <td>
one of: <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>, <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>,
<a href="undocumented#SkAlphaType">kPremul SkAlphaType</a>, <a href="undocumented#SkAlphaType">kUnpremul SkAlphaType</a></td>
  </tr>
</table>

### Return Value

true if <a href="undocumented#Image_Alpha_Type">Alpha Type</a> is set

### Example

<div><fiddle-embed name="786005ff582d0369c83e7d686d94138d"></fiddle-embed></div>

### See Also

<a href="undocumented#Image_Alpha_Type">Alpha Type</a> <a href="undocumented#Image_Color_Type">Color Type</a> <a href="undocumented#Image_Info">Image Info</a> <a href="#SkBitmap_setInfo">setInfo</a>

---

<a name="SkBitmap_getPixels"></a>
## getPixels

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void* getPixels() const
</pre>

Returns pixel address, the base address corresponding to the pixel origin.

### Return Value

pixel address

### Example

<div><fiddle-embed name="e006bb05cf74ec8d2b3d6adeb5dba11b">

#### Example Output

~~~~
bitmap.getColor(0, 1) == 0x00000000
bitmap.getColor(0, 0) == 0xFFFFFFFF
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkBitmap_isNull">isNull</a> <a href="#SkBitmap_drawsNothing">drawsNothing</a>

---

<a name="SkBitmap_computeByteSize"></a>
## computeByteSize

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
size_t computeByteSize() const
</pre>

Returns minimum memory required for pixel storage.
Does not include unused memory on last row when <a href="#SkBitmap_rowBytesAsPixels">rowBytesAsPixels</a> exceeds <a href="#SkBitmap_width">width</a>.
Returns zero if result does not fit in size_t.
Returns zero if <a href="#SkBitmap_height">height</a> or <a href="#SkBitmap_width">width</a> is 0.
Returns <a href="#SkBitmap_height">height</a> times <a href="#SkBitmap_rowBytes">rowBytes</a> if <a href="#SkBitmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kUnknown SkColorType</a>.

### Return Value

size in bytes of image buffer

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

<a href="#SkImageInfo_computeByteSize">SkImageInfo::computeByteSize</a>

---

<a name="SkBitmap_isImmutable"></a>
## isImmutable

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isImmutable() const
</pre>

Returns true if pixels can not change.

Most immutable <a href="#Bitmap">Bitmap</a> checks trigger an assert only on debug builds.

### Return Value

true if pixels are immutable

### Example

<div><fiddle-embed name="cca4f94232e5e1fcee9a841b13df9d9c">

#### Example Output

~~~~
original is immutable
copy is immutable
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkBitmap_setImmutable">setImmutable</a> <a href="#SkPixelRef_isImmutable">SkPixelRef::isImmutable</a> <a href="SkImage_Reference#SkImage">SkImage</a>

---

<a name="SkBitmap_setImmutable"></a>
## setImmutable

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setImmutable()
</pre>

Sets internal flag to mark <a href="#Bitmap">Bitmap</a> as immutable. Once set, pixels can not change.
Any other bitmap sharing the same <a href="undocumented#Pixel_Ref">Pixel Ref</a> are also marked as immutable.
Once <a href="undocumented#Pixel_Ref">Pixel Ref</a> is marked immutable, the setting cannot be cleared.

Writing to immutable <a href="#Bitmap">Bitmap</a> pixels triggers an assert on debug builds.

### Example

<div><fiddle-embed name="9210060d1f4ca46e1375496237902ef3"><div>Triggers assert if SK_DEBUG is true, runs fine otherwise.</div></fiddle-embed></div>

### See Also

<a href="#SkBitmap_isImmutable">isImmutable</a> <a href="#SkPixelRef_setImmutable">SkPixelRef::setImmutable</a> <a href="SkImage_Reference#SkImage">SkImage</a>

---

<a name="SkBitmap_isOpaque"></a>
## isOpaque

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isOpaque() const
</pre>

Returns true if <a href="undocumented#Image_Alpha_Type">Alpha Type</a> is <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>.
Does not check if <a href="undocumented#Image_Color_Type">Color Type</a> allows <a href="#Alpha">Alpha</a>, or if any pixel value has
transparency.

### Return Value

true if <a href="undocumented#Image_Info">Image Info</a> describes opaque <a href="#Alpha">Alpha</a>

### Example

<div><fiddle-embed name="5e76b68bb46d54315eb0c12d83bd6949"><div><a href="#SkBitmap_isOpaque">isOpaque</a> ignores whether all pixels are opaque or not.</div>

#### Example Output

~~~~
isOpaque: false
isOpaque: false
isOpaque: true
isOpaque: true
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkBitmap_ComputeIsOpaque">ComputeIsOpaque</a> <a href="#SkImageInfo_isOpaque">SkImageInfo::isOpaque</a>

---

<a name="SkBitmap_isVolatile"></a>
## isVolatile

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isVolatile() const
</pre>

If true, provides a hint to caller that pixels should not
be cached. Only true if <a href="#SkBitmap_setIsVolatile">setIsVolatile</a> has been called to mark as volatile.

Volatile state is not shared by other bitmaps sharing the same <a href="undocumented#Pixel_Ref">Pixel Ref</a>.

### Return Value

true if marked volatile

### Example

<div><fiddle-embed name="4f3e4d39cf34ea52fcd933dbafc46dfa">

#### Example Output

~~~~
original is volatile
copy is not volatile
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkBitmap_setIsVolatile">setIsVolatile</a>

---

<a name="SkBitmap_setIsVolatile"></a>
## setIsVolatile

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setIsVolatile(bool isVolatile)
</pre>

Sets if pixels should be read from <a href="undocumented#Pixel_Ref">Pixel Ref</a> on every access. <a href="#Bitmap">Bitmaps</a> are not
volatile by default; a GPU back end may upload pixel values expecting them to be
accessed repeatedly. Marking temporary <a href="#Bitmap">Bitmaps</a> as volatile provides a hint to
<a href="undocumented#Device">Device</a> that the <a href="#Bitmap">Bitmap</a> pixels should not be cached. This can
improve performance by avoiding overhead and reducing resource
consumption on <a href="undocumented#Device">Device</a>.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_setIsVolatile_isVolatile"> <code><strong>isVolatile </strong></code> </a></td> <td>
true if backing pixels are temporary</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="9443ada07e6123fc895be4c958244218"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_isVolatile">isVolatile</a>

---

<a name="SkBitmap_reset"></a>
## reset

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void reset()
</pre>

Resets to its initial state; all fields are set to zero, as if <a href="#Bitmap">Bitmap</a> had
been initialized by <a href="#SkBitmap_empty_constructor">SkBitmap()</a>.

Sets <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, row bytes to zero; pixel address to nullptr; <a href="undocumented#SkColorType">SkColorType</a> to
<a href="undocumented#SkColorType">kUnknown SkColorType</a>; and <a href="undocumented#SkAlphaType">SkAlphaType</a> to <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>.

If <a href="undocumented#Pixel_Ref">Pixel Ref</a> is allocated, its reference count is decreased by one, releasing
its memory if <a href="#Bitmap">Bitmap</a> is the sole owner.

### Example

<div><fiddle-embed name="458f838acf38302abec477cec3102f0f">

#### Example Output

~~~~
width:1 height:1 isNull:false
width:0 height:0 isNull:true
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkBitmap_empty_constructor">SkBitmap()</a> <a href="undocumented#SkAlphaType">SkAlphaType</a> <a href="undocumented#SkColorType">SkColorType</a>

---

<a name="SkBitmap_ComputeIsOpaque"></a>
## ComputeIsOpaque

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static bool ComputeIsOpaque(const SkBitmap& bm)
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

### Parameters

<table>  <tr>    <td><a name="SkBitmap_ComputeIsOpaque_bm"> <code><strong>bm </strong></code> </a></td> <td>
<a href="#Bitmap">Bitmap</a> to check</td>
  </tr>
</table>

### Return Value

true if all pixels have opaque values or <a href="undocumented#Image_Color_Type">Color Type</a> is opaque

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

<a href="#SkBitmap_isOpaque">isOpaque</a> <a href="undocumented#Image_Color_Type">Color Type</a> <a href="#Alpha">Alpha</a>

---

<a name="SkBitmap_getBounds"></a>
## getBounds

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void getBounds(SkRect* bounds) const
</pre>

Returns <a href="SkRect_Reference#Rect">Rect</a> { 0, 0, <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a> }.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_getBounds_bounds"> <code><strong>bounds </strong></code> </a></td> <td>
container for floating point rectangle</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="2431ebc7e7d1e91e6d9daafd0f7a478f"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_bounds">bounds</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void getBounds(SkIRect* bounds) const
</pre>

Returns <a href="SkIRect_Reference#IRect">IRect</a> { 0, 0, <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a> }.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_getBounds_2_bounds"> <code><strong>bounds </strong></code> </a></td> <td>
container for integral rectangle</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0c45da35172bc0a529b2faecddae62a2"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_bounds">bounds</a>

---

<a name="SkBitmap_bounds"></a>
## bounds

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkIRect bounds() const
</pre>

Returns <a href="SkIRect_Reference#IRect">IRect</a> { 0, 0, <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a> }.

### Return Value

integral rectangle from origin to <a href="#SkBitmap_width">width</a> and <a href="#SkBitmap_height">height</a>

### Example

<div><fiddle-embed name="41a60435d6eb76cb400fe9be635e3762"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_getBounds">getBounds</a>

---

<a name="SkBitmap_dimensions"></a>
## dimensions

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkISize dimensions() const
</pre>

Returns <a href="undocumented#ISize">ISize</a> { <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a> }.

### Return Value

integral size of <a href="#SkBitmap_width">width</a> and <a href="#SkBitmap_height">height</a>

### Example

<div><fiddle-embed name="647056bcc12c27fb4413f212f33a2898"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_height">height</a> <a href="#SkBitmap_width">width</a>

---

<a name="SkBitmap_getSubset"></a>
## getSubset

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkIRect getSubset() const
</pre>

Returns the <a href="#SkBitmap_bounds">bounds</a> of this bitmap, offset by its <a href="undocumented#Pixel_Ref">Pixel Ref</a> origin.

### Return Value

<a href="#SkBitmap_bounds">bounds</a> within <a href="undocumented#Pixel_Ref">Pixel Ref</a> <a href="#SkBitmap_bounds">bounds</a>

### Example

<div><fiddle-embed name="d6dd0b425aa550f21b938a18c2e1a981">

#### Example Output

~~~~
source: 0, 0, 512, 512
subset: 100, 100, 412, 412
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkBitmap_extractSubset">extractSubset</a> <a href="#SkBitmap_getBounds">getBounds</a>

---

<a name="SkBitmap_setInfo"></a>
## setInfo

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool setInfo(const SkImageInfo& imageInfo, size_t rowBytes = 0)
</pre>

Sets <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Color_Space">Color Space</a>, and optional
<a href="#SkBitmap_rowBytes">rowBytes</a>. Frees pixels, and returns true if successful.

<a href="#SkBitmap_setInfo_imageInfo">imageInfo</a>.<a href="#SkBitmap_alphaType">alphaType</a> may be altered to a value permitted by <a href="#SkBitmap_setInfo_imageInfo">imageInfo</a>.<a href="#SkBitmap_colorSpace">colorSpace</a>.
If <a href="#SkBitmap_setInfo_imageInfo">imageInfo</a>.<a href="#SkBitmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kUnknown SkColorType</a>, <a href="#SkBitmap_setInfo_imageInfo">imageInfo</a>.<a href="#SkBitmap_alphaType">alphaType</a> is
set to <a href="undocumented#SkAlphaType">kUnknown SkAlphaType</a>.
If <a href="#SkBitmap_setInfo_imageInfo">imageInfo</a>.<a href="#SkBitmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a> and <a href="#SkBitmap_setInfo_imageInfo">imageInfo</a>.<a href="#SkBitmap_alphaType">alphaType</a> is
<a href="undocumented#SkAlphaType">kUnpremul SkAlphaType</a>, <a href="#SkBitmap_setInfo_imageInfo">imageInfo</a>.<a href="#SkBitmap_alphaType">alphaType</a> is replaced by <a href="undocumented#SkAlphaType">kPremul SkAlphaType</a>.
If <a href="#SkBitmap_setInfo_imageInfo">imageInfo</a>.<a href="#SkBitmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kRGB 565 SkColorType</a> or <a href="undocumented#SkColorType">kGray 8 SkColorType</a>,
<a href="#SkBitmap_setInfo_imageInfo">imageInfo</a>.<a href="#SkBitmap_alphaType">alphaType</a> is set to <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>.
If <a href="#SkBitmap_setInfo_imageInfo">imageInfo</a>.<a href="#SkBitmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kARGB 4444 SkColorType</a>, <a href="undocumented#SkColorType">kRGBA 8888 SkColorType</a>,
<a href="undocumented#SkColorType">kBGRA 8888 SkColorType</a>, or <a href="undocumented#SkColorType">kRGBA F16 SkColorType</a>: <a href="#SkBitmap_setInfo_imageInfo">imageInfo</a>.<a href="#SkBitmap_alphaType">alphaType</a> remains
unchanged.

<a href="#SkBitmap_rowBytes">rowBytes</a> must equal or exceed <a href="#SkBitmap_setInfo_imageInfo">imageInfo</a>.<a href="undocumented#SkImageInfo">minRowBytes</a>. If <a href="#SkBitmap_setInfo_imageInfo">imageInfo</a>.<a href="#SkBitmap_colorSpace">colorSpace</a> is
<a href="undocumented#SkColorType">kUnknown SkColorType</a>, <a href="#SkBitmap_rowBytes">rowBytes</a> is ignored and treated as zero; for all other
<a href="undocumented#Color_Space">Color Space</a> values, <a href="#SkBitmap_rowBytes">rowBytes</a> of zero is treated as <a href="#SkBitmap_setInfo_imageInfo">imageInfo</a>.<a href="undocumented#SkImageInfo">minRowBytes</a>.

Calls <a href="#SkBitmap_reset">reset</a> and returns false if:

<table>  <tr>
    <td><a href="#SkBitmap_rowBytes">rowBytes</a> exceeds 31 bits</td>  </tr>  <tr>
    <td><a href="#SkBitmap_setInfo_imageInfo">imageInfo</a>.<a href="#SkBitmap_width">width</a> is negative</td>  </tr>  <tr>
    <td><a href="#SkBitmap_setInfo_imageInfo">imageInfo</a>.<a href="#SkBitmap_height">height</a> is negative</td>  </tr>  <tr>
    <td><a href="#SkBitmap_rowBytes">rowBytes</a> is positive and less than <a href="#SkBitmap_setInfo_imageInfo">imageInfo</a>.<a href="#SkBitmap_width">width</a> times <a href="#SkBitmap_setInfo_imageInfo">imageInfo</a>.<a href="#SkBitmap_bytesPerPixel">bytesPerPixel</a></td>  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name="SkBitmap_setInfo_imageInfo"> <code><strong>imageInfo </strong></code> </a></td> <td>
contains <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Color_Space">Color Space</a></td>
  </tr>  <tr>    <td><a name="SkBitmap_setInfo_rowBytes"> <code><strong>rowBytes </strong></code> </a></td> <td>
<a href="#SkBitmap_setInfo_imageInfo">imageInfo</a>.<a href="undocumented#SkImageInfo">minRowBytes</a> or larger; or zero</td>
  </tr>
</table>

### Return Value

true if <a href="undocumented#Image_Info">Image Info</a> set successfully

### Example

<div><fiddle-embed name="599ab64d0aea005498176249bbfb64eb"></fiddle-embed></div>

### See Also

<a href="undocumented#Image_Alpha_Type">Alpha Type</a> <a href="undocumented#Image_Color_Type">Color Type</a> <a href="undocumented#Color_Space">Color Space</a> <a href="#SkBitmap_height">height</a> <a href="#SkBitmap_rowBytes">rowBytes</a> <a href="#SkBitmap_width">width</a>

---

## <a name="SkBitmap_AllocFlags"></a> Enum SkBitmap::AllocFlags

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum <a href="#SkBitmap_AllocFlags">AllocFlags</a> {
<a href="#SkBitmap_kZeroPixels_AllocFlag">kZeroPixels AllocFlag</a> = 1 << 0,
};</pre>

<a href="#SkBitmap_AllocFlags">AllocFlags</a> provides the option to zero pixel memory when allocated.

### Constants

<table>
  <tr>
    <td><a name="SkBitmap_kZeroPixels_AllocFlag"> <code><strong>SkBitmap::kZeroPixels_AllocFlag </strong></code> </a></td><td>1</td><td>Instructs <a href="#SkBitmap_tryAllocPixelsFlags">tryAllocPixelsFlags</a> and <a href="#SkBitmap_allocPixelsFlags">allocPixelsFlags</a> to zero pixel memory.</td>
  </tr>
</table>

### See Also

<a href="#SkBitmap_tryAllocPixelsFlags">tryAllocPixelsFlags</a> <a href="#SkBitmap_allocPixelsFlags">allocPixelsFlags</a> <a href="#SkBitmap_erase">erase</a> <a href="#SkBitmap_eraseColor">eraseColor</a>



<a name="SkBitmap_tryAllocPixelsFlags"></a>
## tryAllocPixelsFlags

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool SK_WARN_UNUSED_RESULT tryAllocPixelsFlags(const SkImageInfo& info, uint32_t flags)
</pre>

Sets <a href="undocumented#Image_Info">Image Info</a> to <a href="#SkBitmap_info">info</a> following the rules in <a href="#SkBitmap_setInfo">setInfo</a> and allocates pixel
memory. If <a href="#SkBitmap_tryAllocPixelsFlags_flags">flags</a> is <a href="#SkBitmap_kZeroPixels_AllocFlag">kZeroPixels AllocFlag</a>, memory is zeroed.

Returns false and calls <a href="#SkBitmap_reset">reset</a> if <a href="undocumented#Image_Info">Image Info</a> could not be set, or memory could
not be allocated, or memory could not optionally be zeroed.

On most platforms, allocating pixel memory may succeed even though there is
not sufficient memory to hold pixels; allocation does not take place
until the pixels are written to. The actual behavior depends on the platform
implementation of malloc(), if <a href="#SkBitmap_tryAllocPixelsFlags_flags">flags</a> is zero, and calloc(), if <a href="#SkBitmap_tryAllocPixelsFlags_flags">flags</a> is
<a href="#SkBitmap_kZeroPixels_AllocFlag">kZeroPixels AllocFlag</a>.

Passing <a href="#SkBitmap_kZeroPixels_AllocFlag">kZeroPixels AllocFlag</a> is usually faster than separately calling

### Parameters

<table>  <tr>    <td><a name="SkBitmap_tryAllocPixelsFlags_info"> <code><strong>info </strong></code> </a></td> <td>
contains <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Color_Space">Color Space</a></td>
  </tr>  <tr>    <td><a name="SkBitmap_tryAllocPixelsFlags_flags"> <code><strong>flags </strong></code> </a></td> <td>
<a href="#SkBitmap_kZeroPixels_AllocFlag">kZeroPixels AllocFlag</a>, or zero</td>
  </tr>
</table>

### Return Value

true if pixels allocation is successful

### Example

<div><fiddle-embed name="4fff5f5034ced615ad5a1d50c40aeff3">

#### Example Output

~~~~
bitmap allocation succeeded!
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkBitmap_allocPixelsFlags">allocPixelsFlags</a> <a href="#SkBitmap_tryAllocPixels">tryAllocPixels</a> <a href="#SkMallocPixelRef_MakeZeroed">SkMallocPixelRef::MakeZeroed</a>

---

<a name="SkBitmap_allocPixelsFlags"></a>
## allocPixelsFlags

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void allocPixelsFlags(const SkImageInfo& info, uint32_t flags)
</pre>

Sets <a href="undocumented#Image_Info">Image Info</a> to <a href="#SkBitmap_info">info</a> following the rules in <a href="#SkBitmap_setInfo">setInfo</a> and allocates pixel
memory. If <a href="#SkBitmap_allocPixelsFlags_flags">flags</a> is <a href="#SkBitmap_kZeroPixels_AllocFlag">kZeroPixels AllocFlag</a>, memory is zeroed.

Aborts execution if <a href="undocumented#Image_Info">Image Info</a> could not be set, or memory could
not be allocated, or memory could not optionally
be zeroed. Abort steps may be provided by the user at compile time by defining
SK_ABORT.

On most platforms, allocating pixel memory may succeed even though there is
not sufficient memory to hold pixels; allocation does not take place
until the pixels are written to. The actual behavior depends on the platform
implementation of malloc(), if <a href="#SkBitmap_allocPixelsFlags_flags">flags</a> is zero, and calloc(), if <a href="#SkBitmap_allocPixelsFlags_flags">flags</a> is
<a href="#SkBitmap_kZeroPixels_AllocFlag">kZeroPixels AllocFlag</a>.

Passing <a href="#SkBitmap_kZeroPixels_AllocFlag">kZeroPixels AllocFlag</a> is usually faster than separately calling

### Parameters

<table>  <tr>    <td><a name="SkBitmap_allocPixelsFlags_info"> <code><strong>info </strong></code> </a></td> <td>
contains <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Color_Space">Color Space</a></td>
  </tr>  <tr>    <td><a name="SkBitmap_allocPixelsFlags_flags"> <code><strong>flags </strong></code> </a></td> <td>
<a href="#SkBitmap_kZeroPixels_AllocFlag">kZeroPixels AllocFlag</a>, or zero</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f21b8965a88c94a32393a8890c4672af"><div><a href="undocumented#Text">Text</a> is drawn on a transparent background; drawing the bitmap a second time
lets the first draw show through.</div></fiddle-embed></div>

### See Also

<a href="#SkBitmap_tryAllocPixelsFlags">tryAllocPixelsFlags</a> <a href="#SkBitmap_allocPixels">allocPixels</a> <a href="#SkMallocPixelRef_MakeZeroed">SkMallocPixelRef::MakeZeroed</a>

---

<a name="SkBitmap_tryAllocPixels"></a>
## tryAllocPixels

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool SK_WARN_UNUSED_RESULT tryAllocPixels(const SkImageInfo& info, size_t rowBytes)
</pre>

Sets <a href="undocumented#Image_Info">Image Info</a> to <a href="#SkBitmap_info">info</a> following the rules in <a href="#SkBitmap_setInfo">setInfo</a> and allocates pixel
memory. <a href="#SkBitmap_rowBytes">rowBytes</a> must equal or exceed <a href="#SkBitmap_info">info</a>.<a href="#SkBitmap_width">width</a> times <a href="#SkBitmap_info">info</a>.<a href="#SkBitmap_bytesPerPixel">bytesPerPixel</a>,
or equal zero. Pass in zero for <a href="#SkBitmap_rowBytes">rowBytes</a> to compute the minimum valid value.

Returns false and calls <a href="#SkBitmap_reset">reset</a> if <a href="undocumented#Image_Info">Image Info</a> could not be set, or memory could
not be allocated.

On most platforms, allocating pixel memory may succeed even though there is
not sufficient memory to hold pixels; allocation does not take place
until the pixels are written to. The actual behavior depends on the platform
implementation of malloc().

### Parameters

<table>  <tr>    <td><a name="SkBitmap_tryAllocPixels_info"> <code><strong>info </strong></code> </a></td> <td>
contains <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Color_Space">Color Space</a></td>
  </tr>  <tr>    <td><a name="SkBitmap_tryAllocPixels_rowBytes"> <code><strong>rowBytes </strong></code> </a></td> <td>
size of pixel row or larger; may be zero</td>
  </tr>
</table>

### Return Value

true if pixel storage is allocated

### Example

<div><fiddle-embed name="34479d5aa23ce9f5e334b0786c9edb22"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_tryAllocPixelsFlags">tryAllocPixelsFlags</a> <a href="#SkBitmap_allocPixels">allocPixels</a> <a href="#SkMallocPixelRef_MakeAllocate">SkMallocPixelRef::MakeAllocate</a>

---

<a name="SkBitmap_allocPixels"></a>
## allocPixels

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void allocPixels(const SkImageInfo& info, size_t rowBytes)
</pre>

Sets <a href="undocumented#Image_Info">Image Info</a> to <a href="#SkBitmap_info">info</a> following the rules in <a href="#SkBitmap_setInfo">setInfo</a> and allocates pixel
memory. <a href="#SkBitmap_rowBytes">rowBytes</a> must equal or exceed <a href="#SkBitmap_info">info</a>.<a href="#SkBitmap_width">width</a> times <a href="#SkBitmap_info">info</a>.<a href="#SkBitmap_bytesPerPixel">bytesPerPixel</a>,
or equal zero. Pass in zero for <a href="#SkBitmap_rowBytes">rowBytes</a> to compute the minimum valid value.

Aborts execution if <a href="undocumented#Image_Info">Image Info</a> could not be set, or memory could
not be allocated. Abort steps may be provided by
the user at compile time by defining SK_ABORT.

On most platforms, allocating pixel memory may succeed even though there is
not sufficient memory to hold pixels; allocation does not take place
until the pixels are written to. The actual behavior depends on the platform
implementation of malloc().

### Parameters

<table>  <tr>    <td><a name="SkBitmap_allocPixels_info"> <code><strong>info </strong></code> </a></td> <td>
contains <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Color_Space">Color Space</a></td>
  </tr>  <tr>    <td><a name="SkBitmap_allocPixels_rowBytes"> <code><strong>rowBytes </strong></code> </a></td> <td>
size of pixel row or larger; may be zero</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="555c0f62f96602a9dcd459badcd005e0"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_tryAllocPixels">tryAllocPixels</a> <a href="#SkBitmap_allocPixelsFlags">allocPixelsFlags</a> <a href="#SkMallocPixelRef_MakeAllocate">SkMallocPixelRef::MakeAllocate</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool SK_WARN_UNUSED_RESULT tryAllocPixels(const SkImageInfo& info)
</pre>

Sets <a href="undocumented#Image_Info">Image Info</a> to <a href="#SkBitmap_info">info</a> following the rules in <a href="#SkBitmap_setInfo">setInfo</a> and allocates pixel
memory.

Returns false and calls <a href="#SkBitmap_reset">reset</a> if <a href="undocumented#Image_Info">Image Info</a> could not be set, or memory could
not be allocated.

On most platforms, allocating pixel memory may succeed even though there is
not sufficient memory to hold pixels; allocation does not take place
until the pixels are written to. The actual behavior depends on the platform
implementation of malloc().

### Parameters

<table>  <tr>    <td><a name="SkBitmap_tryAllocPixels_2_info"> <code><strong>info </strong></code> </a></td> <td>
contains <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Color_Space">Color Space</a></td>
  </tr>
</table>

### Return Value

true if pixel storage is allocated

### Example

<div><fiddle-embed name="7ef3d043c4c5885649e591dd7dca92ff"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_tryAllocPixelsFlags">tryAllocPixelsFlags</a> <a href="#SkBitmap_allocPixels">allocPixels</a> <a href="#SkMallocPixelRef_MakeAllocate">SkMallocPixelRef::MakeAllocate</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void allocPixels(const SkImageInfo& info)
</pre>

Sets <a href="undocumented#Image_Info">Image Info</a> to <a href="#SkBitmap_info">info</a> following the rules in <a href="#SkBitmap_setInfo">setInfo</a> and allocates pixel
memory.

Aborts execution if <a href="undocumented#Image_Info">Image Info</a> could not be set, or memory could
not be allocated. Abort steps may be provided by
the user at compile time by defining SK_ABORT.

On most platforms, allocating pixel memory may succeed even though there is
not sufficient memory to hold pixels; allocation does not take place
until the pixels are written to. The actual behavior depends on the platform
implementation of malloc().

### Parameters

<table>  <tr>    <td><a name="SkBitmap_allocPixels_2_info"> <code><strong>info </strong></code> </a></td> <td>
contains <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Color_Space">Color Space</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="91f474a11a2112cd5c88c40a9015048d"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_tryAllocPixels">tryAllocPixels</a> <a href="#SkBitmap_allocPixelsFlags">allocPixelsFlags</a> <a href="#SkMallocPixelRef_MakeAllocate">SkMallocPixelRef::MakeAllocate</a>

---

<a name="SkBitmap_tryAllocN32Pixels"></a>
## tryAllocN32Pixels

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool SK_WARN_UNUSED_RESULT tryAllocN32Pixels(int width, int height, bool isOpaque = false)
</pre>

Sets <a href="undocumented#Image_Info">Image Info</a> to <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, and <a href="undocumented#Native_Color_Type">Native Color Type</a>; and allocates
pixel memory. If <a href="#SkBitmap_isOpaque">isOpaque</a> is true, sets <a href="undocumented#Image_Info">Image Info</a> to <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>;
otherwise, sets to <a href="undocumented#SkAlphaType">kPremul SkAlphaType</a>.

Calls <a href="#SkBitmap_reset">reset</a> and returns false if <a href="#SkBitmap_width">width</a> exceeds 29 bits or is negative,
or <a href="#SkBitmap_height">height</a> is negative.

Returns false if allocation fails.

Use to create <a href="#Bitmap">Bitmap</a> that matches <a href="undocumented#SkPMColor">SkPMColor</a>, the native pixel arrangement on
the platform. <a href="#Bitmap">Bitmap</a> drawn to output device skips converting its pixel format.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_tryAllocN32Pixels_width"> <code><strong>width </strong></code> </a></td> <td>
pixel column count; must be zero or greater</td>
  </tr>  <tr>    <td><a name="SkBitmap_tryAllocN32Pixels_height"> <code><strong>height </strong></code> </a></td> <td>
pixel row count; must be zero or greater</td>
  </tr>  <tr>    <td><a name="SkBitmap_tryAllocN32Pixels_isOpaque"> <code><strong>isOpaque </strong></code> </a></td> <td>
true if pixels do not have transparency</td>
  </tr>
</table>

### Return Value

true if pixel storage is allocated

### Example

<div><fiddle-embed name="a2b1e0910f37066f15ae56368775a6d8"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_tryAllocPixels">tryAllocPixels</a> <a href="#SkBitmap_allocN32Pixels">allocN32Pixels</a> <a href="#SkMallocPixelRef_MakeAllocate">SkMallocPixelRef::MakeAllocate</a>

---

<a name="SkBitmap_allocN32Pixels"></a>
## allocN32Pixels

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void allocN32Pixels(int width, int height, bool isOpaque = false)
</pre>

Sets <a href="undocumented#Image_Info">Image Info</a> to <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, and the <a href="undocumented#Native_Color_Type">Native Color Type</a>; and allocates
pixel memory. If <a href="#SkBitmap_isOpaque">isOpaque</a> is true, sets <a href="undocumented#Image_Info">Image Info</a> to <a href="undocumented#SkAlphaType">kPremul SkAlphaType</a>;
otherwise, sets to <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>.

Aborts if <a href="#SkBitmap_width">width</a> exceeds 29 bits or is negative, or <a href="#SkBitmap_height">height</a> is negative, or
allocation fails. Abort steps may be provided by the user at compile time by
defining SK_ABORT.

Use to create <a href="#Bitmap">Bitmap</a> that matches <a href="undocumented#SkPMColor">SkPMColor</a>, the native pixel arrangement on
the platform. <a href="#Bitmap">Bitmap</a> drawn to output device skips converting its pixel format.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_allocN32Pixels_width"> <code><strong>width </strong></code> </a></td> <td>
pixel column count; must be zero or greater</td>
  </tr>  <tr>    <td><a name="SkBitmap_allocN32Pixels_height"> <code><strong>height </strong></code> </a></td> <td>
pixel row count; must be zero or greater</td>
  </tr>  <tr>    <td><a name="SkBitmap_allocN32Pixels_isOpaque"> <code><strong>isOpaque </strong></code> </a></td> <td>
true if pixels do not have transparency</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c717491f9251604724c9cbde7088ec20"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_allocPixels">allocPixels</a> <a href="#SkBitmap_tryAllocN32Pixels">tryAllocN32Pixels</a> <a href="#SkMallocPixelRef_MakeAllocate">SkMallocPixelRef::MakeAllocate</a>

---

<a name="SkBitmap_installPixels"></a>
## installPixels

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool installPixels(const SkImageInfo& info, void* pixels, size_t rowBytes, void (*releaseProc)
                   (void* addr, void* context) , void* context)
</pre>

Sets <a href="undocumented#Image_Info">Image Info</a> to <a href="#SkBitmap_info">info</a> following the rules in <a href="#SkBitmap_setInfo">setInfo</a>, and creates <a href="undocumented#Pixel_Ref">Pixel Ref</a>
containing <a href="#SkBitmap_installPixels_pixels">pixels</a> and <a href="#SkBitmap_rowBytes">rowBytes</a>. <a href="#SkBitmap_installPixels_releaseProc">releaseProc</a>, if not nullptr, is called
immediately on failure or when <a href="#SkBitmap_installPixels_pixels">pixels</a> are no longer referenced. <a href="#SkBitmap_installPixels_context">context</a> may be
nullptr.

If <a href="undocumented#Image_Info">Image Info</a> could not be set, or <a href="#SkBitmap_rowBytes">rowBytes</a> is less than <a href="#SkBitmap_info">info</a>.<a href="undocumented#SkImageInfo">minRowBytes</a>:
calls <a href="#SkBitmap_installPixels_releaseProc">releaseProc</a> if present, calls <a href="#SkBitmap_reset">reset</a>, and returns false.

Otherwise, if <a href="#SkBitmap_installPixels_pixels">pixels</a> equals nullptr: sets <a href="undocumented#Image_Info">Image Info</a>, calls <a href="#SkBitmap_installPixels_releaseProc">releaseProc</a> if
present, returns true.

If <a href="undocumented#Image_Info">Image Info</a> is set, <a href="#SkBitmap_installPixels_pixels">pixels</a> is not nullptr, and <a href="#SkBitmap_installPixels_releaseProc">releaseProc</a> is not nullptr:
when <a href="#SkBitmap_installPixels_pixels">pixels</a> are no longer referenced, calls <a href="#SkBitmap_installPixels_releaseProc">releaseProc</a> with <a href="#SkBitmap_installPixels_pixels">pixels</a> and <a href="#SkBitmap_installPixels_context">context</a>
as parameters.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_installPixels_info"> <code><strong>info </strong></code> </a></td> <td>
contains <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Color_Space">Color Space</a></td>
  </tr>  <tr>    <td><a name="SkBitmap_installPixels_pixels"> <code><strong>pixels </strong></code> </a></td> <td>
address or pixel storage; may be nullptr</td>
  </tr>  <tr>    <td><a name="SkBitmap_installPixels_rowBytes"> <code><strong>rowBytes </strong></code> </a></td> <td>
size of pixel row or larger</td>
  </tr>  <tr>    <td><a name="SkBitmap_installPixels_releaseProc"> <code><strong>releaseProc </strong></code> </a></td> <td>
function called when <a href="#SkBitmap_installPixels_pixels">pixels</a> can be deleted; may be nullptr</td>
  </tr>  <tr>    <td><a name="SkBitmap_installPixels_context"> <code><strong>context </strong></code> </a></td> <td>
caller state passed to <a href="#SkBitmap_installPixels_releaseProc">releaseProc</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

true if <a href="undocumented#Image_Info">Image Info</a> is set to <a href="#SkBitmap_info">info</a>

### Example

<div><fiddle-embed name="ff5b5c349efb5d929ff5dff07b7727e6"><div><a href="#SkBitmap_installPixels_releaseProc">releaseProc</a> is called immediately because <a href="#SkBitmap_rowBytes">rowBytes</a> is too small for <a href="undocumented#Pixel_Ref">Pixel Ref</a>.</div>

#### Example Output

~~~~
before installPixels
releaseProc called
install not successful
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkBitmap_allocPixels">allocPixels</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool installPixels(const SkImageInfo& info, void* pixels, size_t rowBytes)
</pre>

Sets <a href="undocumented#Image_Info">Image Info</a> to <a href="#SkBitmap_info">info</a> following the rules in <a href="#SkBitmap_setInfo">setInfo</a>, and creates <a href="undocumented#Pixel_Ref">Pixel Ref</a>
containing <a href="#SkBitmap_installPixels_2_pixels">pixels</a> and <a href="#SkBitmap_rowBytes">rowBytes</a>.

If <a href="undocumented#Image_Info">Image Info</a> could not be set, or <a href="#SkBitmap_rowBytes">rowBytes</a> is less than <a href="#SkBitmap_info">info</a>.<a href="undocumented#SkImageInfo">minRowBytes</a>:
calls <a href="#SkBitmap_reset">reset</a>, and returns false.

Otherwise, if <a href="#SkBitmap_installPixels_2_pixels">pixels</a> equals nullptr: sets <a href="undocumented#Image_Info">Image Info</a>, returns true.

Caller must ensure that <a href="#SkBitmap_installPixels_2_pixels">pixels</a> are valid for the lifetime of <a href="#Bitmap">Bitmap</a> and <a href="undocumented#Pixel_Ref">Pixel Ref</a>.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_installPixels_2_info"> <code><strong>info </strong></code> </a></td> <td>
contains <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Color_Space">Color Space</a></td>
  </tr>  <tr>    <td><a name="SkBitmap_installPixels_2_pixels"> <code><strong>pixels </strong></code> </a></td> <td>
address or pixel storage; may be nullptr</td>
  </tr>  <tr>    <td><a name="SkBitmap_installPixels_2_rowBytes"> <code><strong>rowBytes </strong></code> </a></td> <td>
size of pixel row or larger</td>
  </tr>
</table>

### Return Value

true if <a href="undocumented#Image_Info">Image Info</a> is set to <a href="#SkBitmap_info">info</a>

### Example

<div><fiddle-embed name="a7e04447b2081010c50d7920e80a6bb2"><div>GPU does not support <a href="undocumented#SkAlphaType">kUnpremul SkAlphaType</a>, does not assert that it does not.</div></fiddle-embed></div>

### See Also

<a href="#SkBitmap_allocPixels">allocPixels</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool installPixels(const SkPixmap& pixmap)
</pre>

Sets <a href="undocumented#Image_Info">Image Info</a> to <a href="#SkBitmap_pixmap">pixmap</a>.<a href="#SkBitmap_info">info</a> following the rules in <a href="#SkBitmap_setInfo">setInfo</a>, and creates
<a href="undocumented#Pixel_Ref">Pixel Ref</a> containing <a href="#SkBitmap_pixmap">pixmap</a>.addr() and <a href="#SkBitmap_pixmap">pixmap</a>.<a href="#SkBitmap_rowBytes">rowBytes</a>.

If <a href="undocumented#Image_Info">Image Info</a> could not be set, or <a href="#SkBitmap_pixmap">pixmap</a>.<a href="#SkBitmap_rowBytes">rowBytes</a> is less than
<a href="#SkImageInfo_minRowBytes">SkImageInfo::minRowBytes</a>: calls <a href="#SkBitmap_reset">reset</a>, and returns false.

Otherwise, if <a href="#SkBitmap_pixmap">pixmap</a>.addr() equals nullptr: sets <a href="undocumented#Image_Info">Image Info</a>, returns true.

Caller must ensure that <a href="#SkBitmap_pixmap">pixmap</a> is valid for the lifetime of <a href="#Bitmap">Bitmap</a> and <a href="undocumented#Pixel_Ref">Pixel Ref</a>.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_installPixels_3_pixmap"> <code><strong>pixmap </strong></code> </a></td> <td>
<a href="undocumented#Image_Info">Image Info</a>, pixel address, and <a href="#SkBitmap_rowBytes">rowBytes</a></td>
  </tr>
</table>

### Return Value

true if <a href="undocumented#Image_Info">Image Info</a> was set to <a href="#SkBitmap_pixmap">pixmap</a>.<a href="#SkBitmap_info">info</a>

### Example

<div><fiddle-embed name="6e2a8c9358b34aebd2ec586815fe9d3a"><div>Draw a five by five bitmap, and draw it again with a center white pixel.</div></fiddle-embed></div>

### See Also

<a href="#SkBitmap_allocPixels">allocPixels</a>

---

<a name="SkBitmap_installMaskPixels"></a>
## installMaskPixels

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool installMaskPixels(const SkMask& mask)
</pre>

Sets <a href="undocumented#Image_Info">Image Info</a> to <a href="#SkBitmap_installMaskPixels_mask">mask</a> <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_installMaskPixels_mask">mask</a> <a href="#SkBitmap_height">height</a>, <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>, and
<a href="undocumented#SkAlphaType">kPremul SkAlphaType</a>. Sets <a href="undocumented#Pixel_Ref">Pixel Ref</a> to <a href="#SkBitmap_installMaskPixels_mask">mask</a> image and <a href="#SkBitmap_installMaskPixels_mask">mask</a> <a href="#SkBitmap_rowBytes">rowBytes</a>.

Returns false and calls <a href="#SkBitmap_reset">reset</a> if <a href="#SkBitmap_installMaskPixels_mask">mask</a> format is not <a href="#SkMask_kA8_Format">SkMask::kA8 Format</a>,
or if <a href="#SkBitmap_installMaskPixels_mask">mask</a> <a href="#SkBitmap_width">width</a> or <a href="#SkBitmap_installMaskPixels_mask">mask</a> <a href="#SkBitmap_height">height</a> is negative, or if <a href="#SkBitmap_installMaskPixels_mask">mask</a> <a href="#SkBitmap_rowBytes">rowBytes</a> is less
than <a href="#SkBitmap_installMaskPixels_mask">mask</a> <a href="#SkBitmap_width">width</a>.

Caller must ensure that <a href="#SkBitmap_installMaskPixels_mask">mask</a> is valid for the lifetime of <a href="#Bitmap">Bitmap</a> and <a href="undocumented#Pixel_Ref">Pixel Ref</a>.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_installMaskPixels_mask"> <code><strong>mask </strong></code> </a></td> <td>
<a href="#Alpha">Alpha</a> 8-bit bitmap</td>
  </tr>
</table>

### Return Value

true if <a href="undocumented#Image_Info">Image Info</a> and <a href="undocumented#Pixel_Ref">Pixel Ref</a> refer to <a href="#SkBitmap_installMaskPixels_mask">mask</a>

### Example

<div><fiddle-embed name="b0456fc063ca467ccecf2105160a3d1d"><div>Draw a five by five bitmap, and draw it again with a center white pixel.</div></fiddle-embed></div>

### See Also

<a href="#SkBitmap_installPixels">installPixels</a> <a href="#SkBitmap_allocPixels">allocPixels</a>

---

<a name="SkBitmap_setPixels"></a>
## setPixels

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setPixels(void* pixels)
</pre>

Replaces <a href="undocumented#Pixel_Ref">Pixel Ref</a> with <a href="#SkBitmap_setPixels_pixels">pixels</a>, preserving <a href="undocumented#Image_Info">Image Info</a> and <a href="#SkBitmap_rowBytes">rowBytes</a>.
Sets <a href="undocumented#Pixel_Ref">Pixel Ref</a> origin to (0, 0).

If <a href="#SkBitmap_setPixels_pixels">pixels</a> is nullptr, or if <a href="#SkBitmap_info">info</a>.<a href="#SkBitmap_colorType">colorType</a> equals <a href="undocumented#SkColorType">kUnknown SkColorType</a>;
release reference to <a href="undocumented#Pixel_Ref">Pixel Ref</a>, and set <a href="undocumented#Pixel_Ref">Pixel Ref</a> to nullptr.

Caller is responsible for handling ownership pixel memory for the lifetime
of <a href="#Bitmap">Bitmap</a> and <a href="undocumented#Pixel_Ref">Pixel Ref</a>.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_setPixels_pixels"> <code><strong>pixels </strong></code> </a></td> <td>
address of pixel storage, managed by caller</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f0db16e06c9a1436917c8179f8c1718f"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_installPixels">installPixels</a> <a href="#SkBitmap_allocPixels">allocPixels</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool SK_WARN_UNUSED_RESULT tryAllocPixels()
</pre>

Allocates pixel memory with <a href="#SkBitmap_HeapAllocator">HeapAllocator</a>, and replaces existing <a href="undocumented#Pixel_Ref">Pixel Ref</a>.
The allocation size is determined by <a href="undocumented#Image_Info">Image Info</a> <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, and <a href="undocumented#Image_Color_Type">Color Type</a>.

Returns false if <a href="#SkBitmap_info">info</a>.<a href="#SkBitmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kUnknown SkColorType</a>, or allocation fails.

### Return Value

true if the allocation succeeds

### Example

<div><fiddle-embed name="720e4c053fae9e929ab6518b47e49370"><div><a href="#Bitmap">Bitmap</a> hosts and draws gray values in set1. <a href="#SkBitmap_tryAllocPixels">tryAllocPixels</a> replaces <a href="undocumented#Pixel_Ref">Pixel Ref</a>
and erases it to black, but does not alter set1. <a href="#SkBitmap_setPixels">setPixels</a> replaces black
<a href="undocumented#Pixel_Ref">Pixel Ref</a> with set1.</div></fiddle-embed></div>

### See Also

<a href="#SkBitmap_allocPixels">allocPixels</a> <a href="#SkBitmap_installPixels">installPixels</a> <a href="#SkBitmap_setPixels">setPixels</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void allocPixels()
</pre>

Allocates pixel memory with <a href="#SkBitmap_HeapAllocator">HeapAllocator</a>, and replaces existing <a href="undocumented#Pixel_Ref">Pixel Ref</a>.
The allocation size is determined by <a href="undocumented#Image_Info">Image Info</a> <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, and <a href="undocumented#Image_Color_Type">Color Type</a>.

Aborts if <a href="#SkBitmap_info">info</a>.<a href="#SkBitmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kUnknown SkColorType</a>, or allocation fails.
Abort steps may be provided by the user at compile
time by defining SK_ABORT.

### Example

<div><fiddle-embed name="1219b38c788bf270fb20f8cd2d78cff8"><div><a href="#Bitmap">Bitmap</a> hosts and draws gray values in set1. <a href="#SkBitmap_allocPixels">allocPixels</a> replaces <a href="undocumented#Pixel_Ref">Pixel Ref</a>
and erases it to black, but does not alter set1. <a href="#SkBitmap_setPixels">setPixels</a> replaces black
<a href="undocumented#Pixel_Ref">Pixel Ref</a> with set2.</div></fiddle-embed></div>

### See Also

<a href="#SkBitmap_tryAllocPixels">tryAllocPixels</a> <a href="#SkBitmap_installPixels">installPixels</a> <a href="#SkBitmap_setPixels">setPixels</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool SK_WARN_UNUSED_RESULT tryAllocPixels(Allocator* allocator)
</pre>

Allocates pixel memory with <a href="#SkBitmap_tryAllocPixels_4_allocator">allocator</a>, and replaces existing <a href="undocumented#Pixel_Ref">Pixel Ref</a>.
The allocation size is determined by <a href="undocumented#Image_Info">Image Info</a> <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, and <a href="undocumented#Image_Color_Type">Color Type</a>.
If <a href="#SkBitmap_tryAllocPixels_4_allocator">allocator</a> is nullptr, use <a href="#SkBitmap_HeapAllocator">HeapAllocator</a> instead.

Returns false if <a href="#SkBitmap_tryAllocPixels_4_allocator">allocator</a> <a href="#SkBitmap_HeapAllocator_allocPixelRef">allocPixelRef</a> return false.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_tryAllocPixels_4_allocator"> <code><strong>allocator </strong></code> </a></td> <td>
instance of <a href="#SkBitmap_Allocator">SkBitmap::Allocator</a> instantiation</td>
  </tr>
</table>

### Return Value

true if custom <a href="#SkBitmap_tryAllocPixels_4_allocator">allocator</a> reports success

### Example

<div><fiddle-embed name="eb6f861ca1839146d26e40d56c2a001c"><div><a href="#SkBitmap_HeapAllocator">HeapAllocator</a> limits the maximum size of <a href="#Bitmap">Bitmap</a> to two gigabytes. Using
a custom <a href="#SkBitmap_tryAllocPixels_4_allocator">allocator</a>, this limitation may be relaxed. This example can be
modified to allocate an eight gigabyte <a href="#Bitmap">Bitmap</a> on a 64 bit platform with
sufficient memory.</div></fiddle-embed></div>

### See Also

<a href="#SkBitmap_allocPixels">allocPixels</a> <a href="#SkBitmap_Allocator">Allocator</a> <a href="undocumented#Pixel_Ref">Pixel Ref</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void allocPixels(Allocator* allocator)
</pre>

Allocates pixel memory with <a href="#SkBitmap_allocPixels_4_allocator">allocator</a>, and replaces existing <a href="undocumented#Pixel_Ref">Pixel Ref</a>.
The allocation size is determined by <a href="undocumented#Image_Info">Image Info</a> <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, and <a href="undocumented#Image_Color_Type">Color Type</a>.
If <a href="#SkBitmap_allocPixels_4_allocator">allocator</a> is nullptr, use <a href="#SkBitmap_HeapAllocator">HeapAllocator</a> instead.

Aborts if <a href="#SkBitmap_allocPixels_4_allocator">allocator</a> <a href="#SkBitmap_HeapAllocator_allocPixelRef">allocPixelRef</a> return false. Abort steps may be provided by
the user at compile time by defining SK_ABORT.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_allocPixels_4_allocator"> <code><strong>allocator </strong></code> </a></td> <td>
instance of <a href="#SkBitmap_Allocator">SkBitmap::Allocator</a> instantiation</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1b2800d23c9ea249b45c2c21a34b6d14"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_allocPixels">allocPixels</a> <a href="#SkBitmap_Allocator">Allocator</a> <a href="undocumented#Pixel_Ref">Pixel Ref</a>

---

<a name="SkBitmap_pixelRef"></a>
## pixelRef

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkPixelRef* pixelRef() const
</pre>

Returns <a href="undocumented#Pixel_Ref">Pixel Ref</a>, which contains: pixel base address; its <a href="#SkBitmap_dimensions">dimensions</a>; and
<a href="#SkBitmap_rowBytes">rowBytes</a>, the interval from one row to the next. Does not change <a href="undocumented#Pixel_Ref">Pixel Ref</a>
reference count. <a href="undocumented#Pixel_Ref">Pixel Ref</a> may be shared by multiple bitmaps.
If <a href="undocumented#Pixel_Ref">Pixel Ref</a> has not been set, returns nullptr.

### Return Value

<a href="undocumented#Pixel_Ref">Pixel Ref</a>, or nullptr

### Example

<div><fiddle-embed name="5db2d30870a7cc45f28e22578d1880c3"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_getPixels">getPixels</a> <a href="#SkBitmap_getAddr">getAddr</a>

---

<a name="SkBitmap_pixelRefOrigin"></a>
## pixelRefOrigin

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkIPoint pixelRefOrigin() const
</pre>

Returns origin of pixels within <a href="undocumented#Pixel_Ref">Pixel Ref</a>. <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_bounds">bounds</a> is always contained
by <a href="undocumented#Pixel_Ref">Pixel Ref</a> <a href="#SkBitmap_bounds">bounds</a>, which may be the same size or larger. Multiple <a href="#Bitmap">Bitmaps</a>
can share the same <a href="undocumented#Pixel_Ref">Pixel Ref</a>, where each <a href="#Bitmap">Bitmap</a> has different <a href="#SkBitmap_bounds">bounds</a>.

The returned origin added to <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_dimensions">dimensions</a> equals or is smaller than the
<a href="undocumented#Pixel_Ref">Pixel Ref</a> <a href="#SkBitmap_dimensions">dimensions</a>.

Returns (0, 0) if <a href="undocumented#Pixel_Ref">Pixel Ref</a> is nullptr.

### Return Value

pixel origin within <a href="undocumented#Pixel_Ref">Pixel Ref</a>

### Example

<div><fiddle-embed name="6d31686c6c0829c70f284ae716526d6a">

#### Example Output

~~~~
source origin: 0, 0
subset origin: 32, 64
~~~~

</fiddle-embed></div>

### See Also

<a href="undocumented#SkPixelRef">SkPixelRef</a> <a href="#SkBitmap_getSubset">getSubset</a> <a href="#SkBitmap_setPixelRef">setPixelRef</a>

---

<a name="SkBitmap_setPixelRef"></a>
## setPixelRef

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setPixelRef(sk_sp&lt;SkPixelRef&gt; pixelRef, int dx, int dy)
</pre>

Replaces <a href="#SkBitmap_pixelRef">pixelRef</a> and origin in <a href="#Bitmap">Bitmap</a>.  <a href="#SkBitmap_setPixelRef_dx">dx</a> and <a href="#SkBitmap_setPixelRef_dy">dy</a> specify the offset
within the <a href="undocumented#Pixel_Ref">Pixel Ref</a> pixels for the top-left corner of the bitmap.

Asserts in debug builds if <a href="#SkBitmap_setPixelRef_dx">dx</a> or <a href="#SkBitmap_setPixelRef_dy">dy</a> are out of range. Pins <a href="#SkBitmap_setPixelRef_dx">dx</a> and <a href="#SkBitmap_setPixelRef_dy">dy</a>
to legal range in release builds.

The caller is responsible for ensuring that the pixels match the
<a href="undocumented#Image_Color_Type">Color Type</a> and <a href="undocumented#Image_Alpha_Type">Alpha Type</a> in <a href="undocumented#Image_Info">Image Info</a>.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_setPixelRef_pixelRef"> <code><strong>pixelRef </strong></code> </a></td> <td>
<a href="undocumented#Pixel_Ref">Pixel Ref</a> describing pixel address and <a href="#SkBitmap_rowBytes">rowBytes</a></td>
  </tr>  <tr>    <td><a name="SkBitmap_setPixelRef_dx"> <code><strong>dx </strong></code> </a></td> <td>
column offset in <a href="undocumented#Pixel_Ref">Pixel Ref</a> for bitmap origin</td>
  </tr>  <tr>    <td><a name="SkBitmap_setPixelRef_dy"> <code><strong>dy </strong></code> </a></td> <td>
row offset in <a href="undocumented#Pixel_Ref">Pixel Ref</a> for bitmap origin</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="13df9e5b1adcec33d11e4b0f8a91ecb8"><div>Treating 32 bit data as 8 bit data is unlikely to produce useful results.</div></fiddle-embed></div>

### See Also

<a href="#SkBitmap_setInfo">setInfo</a>

---

<a name="SkBitmap_readyToDraw"></a>
## readyToDraw

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool readyToDraw() const
</pre>

Returns true if <a href="#Bitmap">Bitmap</a> is can be drawn.

### Return Value

true if <a href="#SkBitmap_getPixels">getPixels</a> is not nullptr

### Example

<div><fiddle-embed name="e89c78ca992e2e789ed50944fe68f920"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_getPixels">getPixels</a> <a href="#SkBitmap_drawsNothing">drawsNothing</a>

---

<a name="SkBitmap_getGenerationID"></a>
## getGenerationID

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
uint32_t getGenerationID() const
</pre>

Returns a unique value corresponding to the pixels in <a href="undocumented#Pixel_Ref">Pixel Ref</a>.
Returns a different value after <a href="#SkBitmap_notifyPixelsChanged">notifyPixelsChanged</a> has been called.
Returns zero if <a href="undocumented#Pixel_Ref">Pixel Ref</a> is nullptr.

Determines if pixels have changed since last examined.

### Return Value

unique value for pixels in <a href="undocumented#Pixel_Ref">Pixel Ref</a>

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

<a href="#SkBitmap_notifyPixelsChanged">notifyPixelsChanged</a> <a href="undocumented#Pixel_Ref">Pixel Ref</a>

---

<a name="SkBitmap_notifyPixelsChanged"></a>
## notifyPixelsChanged

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void notifyPixelsChanged() const
</pre>

Marks that pixels in <a href="undocumented#Pixel_Ref">Pixel Ref</a> have changed. Subsequent calls to
<a href="#SkBitmap_getGenerationID">getGenerationID</a> return a different value.

### Example

<div><fiddle-embed name="706e6409c8364e7786d6c69db7242c76"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_getGenerationID">getGenerationID</a> <a href="#SkBitmap_isVolatile">isVolatile</a> <a href="undocumented#Pixel_Ref">Pixel Ref</a>

---

<a name="SkBitmap_eraseColor"></a>
## eraseColor

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void eraseColor(SkColor c) const
</pre>

Replaces pixel values with <a href="#SkBitmap_eraseColor_c">c</a>. All pixels contained by <a href="#SkBitmap_bounds">bounds</a> are affected.
If the <a href="#SkBitmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kGray 8 SkColorType</a> or k565_SkColorType, then <a href="#Alpha">Color Alpha</a>
is ignored; <a href="#RGB">Color RGB</a> is treated as opaque. If <a href="#SkBitmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>,
then <a href="#RGB">Color RGB</a> is ignored.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_eraseColor_c"> <code><strong>c </strong></code> </a></td> <td>
<a href="#Unpremultiply">Unpremultiplied</a> <a href="undocumented#Color">Color</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="22804faa423192f96a29578b1d396ace"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_eraseARGB">eraseARGB</a> <a href="#SkBitmap_erase">erase</a>

---

<a name="SkBitmap_eraseARGB"></a>
## eraseARGB

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void eraseARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b) const
</pre>

Replaces pixel values with <a href="#Unpremultiply">Unpremultiplied</a> <a href="undocumented#Color">Color</a> built from <a href="#SkBitmap_eraseARGB_a">a</a>, <a href="#SkBitmap_eraseARGB_r">r</a>, <a href="#SkBitmap_eraseARGB_g">g</a>, and <a href="#SkBitmap_eraseARGB_b">b</a>.
All pixels contained by <a href="#SkBitmap_bounds">bounds</a> are affected.
If the <a href="#SkBitmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kGray 8 SkColorType</a> or k565_SkColorType, then <a href="#SkBitmap_eraseARGB_a">a</a>
is ignored; <a href="#SkBitmap_eraseARGB_r">r</a>, <a href="#SkBitmap_eraseARGB_g">g</a>, and <a href="#SkBitmap_eraseARGB_b">b</a> are treated as opaque. If <a href="#SkBitmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>,
then <a href="#SkBitmap_eraseARGB_r">r</a>, <a href="#SkBitmap_eraseARGB_g">g</a>, and <a href="#SkBitmap_eraseARGB_b">b</a> are ignored.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_eraseARGB_a"> <code><strong>a </strong></code> </a></td> <td>
amount of <a href="#Alpha">Color Alpha</a>, from fully transparent (0) to fully opaque (255)</td>
  </tr>  <tr>    <td><a name="SkBitmap_eraseARGB_r"> <code><strong>r </strong></code> </a></td> <td>
amount of <a href="#RGB_Red">Color RGB Red</a>, from no red (0) to full red (255)</td>
  </tr>  <tr>    <td><a name="SkBitmap_eraseARGB_g"> <code><strong>g </strong></code> </a></td> <td>
amount of <a href="#RGB_Green">Color RGB Green</a>, from no green (0) to full green (255)</td>
  </tr>  <tr>    <td><a name="SkBitmap_eraseARGB_b"> <code><strong>b </strong></code> </a></td> <td>
amount of <a href="#RGB_Blue">Color RGB Blue</a>, from no blue (0) to full blue (255)</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ebaa436565ce9c0f883bef871d099654"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_eraseColor">eraseColor</a> <a href="#SkBitmap_erase">erase</a>

---

<a name="SkBitmap_eraseRGB"></a>
## eraseRGB

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void eraseRGB(U8CPU r, U8CPU g, U8CPU b) const
</pre>

Deprecated. Use <a href="#SkBitmap_eraseARGB">eraseARGB</a> or <a href="#SkBitmap_eraseColor">eraseColor</a>.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_eraseRGB_r"> <code><strong>r </strong></code> </a></td> <td>
amount of red</td>
  </tr>  <tr>    <td><a name="SkBitmap_eraseRGB_g"> <code><strong>g </strong></code> </a></td> <td>
amount of green</td>
  </tr>  <tr>    <td><a name="SkBitmap_eraseRGB_b"> <code><strong>b </strong></code> </a></td> <td>
amount of blue</td>
  </tr>
</table>

### See Also

<a href="#SkBitmap_eraseColor">eraseColor</a> <a href="#SkBitmap_eraseARGB">eraseARGB</a> <a href="#SkBitmap_erase">erase</a>

---

<a name="SkBitmap_erase"></a>
## erase

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void erase(SkColor c, const SkIRect& area) const
</pre>

Replaces pixel values inside <a href="#SkBitmap_erase_area">area</a> with <a href="#SkBitmap_erase_c">c</a>. If <a href="#SkBitmap_erase_area">area</a> does not intersect <a href="#SkBitmap_bounds">bounds</a>,
call has no effect.

If the <a href="#SkBitmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kGray 8 SkColorType</a> or k565_SkColorType, then <a href="#Alpha">Color Alpha</a>
is ignored; <a href="#RGB">Color RGB</a> is treated as opaque. If <a href="#SkBitmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>,
then <a href="#RGB">Color RGB</a> is ignored.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_erase_c"> <code><strong>c </strong></code> </a></td> <td>
<a href="#Unpremultiply">Unpremultiplied</a> <a href="undocumented#Color">Color</a></td>
  </tr>  <tr>    <td><a name="SkBitmap_erase_area"> <code><strong>area </strong></code> </a></td> <td>
rectangle to fill</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="116baaa35348b5c0a353fb6d4917d574"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_eraseColor">eraseColor</a> <a href="#SkBitmap_eraseARGB">eraseARGB</a> <a href="#SkBitmap_eraseRGB">eraseRGB</a> <a href="#SkCanvas_drawRect">SkCanvas::drawRect</a>

---

<a name="SkBitmap_eraseArea"></a>
## eraseArea

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void eraseArea(const SkIRect& area, SkColor c) const
</pre>

Legacy call to be deprecated.

---

<a name="SkBitmap_getColor"></a>
## getColor

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkColor getColor(int x, int y) const
</pre>

Returns pixel at (<a href="#SkBitmap_getColor_x">x</a>, <a href="#SkBitmap_getColor_y">y</a>) as <a href="#Unpremultiply">Unpremultiplied</a> <a href="undocumented#Color">Color</a>.
Returns black with <a href="#Alpha">Alpha</a> if <a href="undocumented#Image_Color_Type">Color Type</a> is <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>.

Input is not validated: out of <a href="#SkBitmap_bounds">bounds</a> values of <a href="#SkBitmap_getColor_x">x</a> or <a href="#SkBitmap_getColor_y">y</a> trigger an assert() if
built with SK_DEBUG defined; and returns undefined values or may crash if
SK_RELEASE is defined. Fails if <a href="undocumented#Image_Color_Type">Color Type</a> is <a href="undocumented#SkColorType">kUnknown SkColorType</a> or
pixel address is nullptr.

<a href="undocumented#Color_Space">Color Space</a> in <a href="undocumented#Image_Info">Image Info</a> is ignored. Some <a href="undocumented#Color">Color</a> precision may be lost in the
conversion to <a href="#Unpremultiply">Unpremultiplied</a> <a href="undocumented#Color">Color</a>; original pixel data may have additional
precision.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_getColor_x"> <code><strong>x </strong></code> </a></td> <td>
column index, zero or greater, and less than <a href="#SkBitmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkBitmap_getColor_y"> <code><strong>y </strong></code> </a></td> <td>
row index, zero or greater, and less than <a href="#SkBitmap_height">height</a></td>
  </tr>
</table>

### Return Value

pixel converted to <a href="#Unpremultiply">Unpremultiplied</a> <a href="undocumented#Color">Color</a>

### Example

<div><fiddle-embed name="2e49fa71dd4be95470248ea6eb35468b">

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

<a href="#SkBitmap_getAddr">getAddr</a> <a href="#SkBitmap_readPixels">readPixels</a>

---

<a name="SkBitmap_getAddr"></a>
## getAddr

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void* getAddr(int x, int y) const
</pre>

Returns pixel address at (<a href="#SkBitmap_getAddr_x">x</a>, <a href="#SkBitmap_getAddr_y">y</a>).

Input is not validated: out of <a href="#SkBitmap_bounds">bounds</a> values of <a href="#SkBitmap_getAddr_x">x</a> or <a href="#SkBitmap_getAddr_y">y</a>, or <a href="undocumented#SkColorType">kUnknown SkColorType</a>,
trigger an assert() if built with SK_DEBUG defined. Returns nullptr if
<a href="undocumented#Image_Color_Type">Color Type</a> is <a href="undocumented#SkColorType">kUnknown SkColorType</a>, or <a href="undocumented#Pixel_Ref">Pixel Ref</a> is nullptr.

Performs a lookup of pixel size; for better performance, call
one of: <a href="#SkBitmap_getAddr8">getAddr8</a>, <a href="#SkBitmap_getAddr16">getAddr16</a>, or <a href="#SkBitmap_getAddr32">getAddr32</a>.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_getAddr_x"> <code><strong>x </strong></code> </a></td> <td>
column index, zero or greater, and less than <a href="#SkBitmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkBitmap_getAddr_y"> <code><strong>y </strong></code> </a></td> <td>
row index, zero or greater, and less than <a href="#SkBitmap_height">height</a></td>
  </tr>
</table>

### Return Value

generic pointer to pixel

### Example

<div><fiddle-embed name="bd0e0adb48b16d75129092649eab1617">

#### Example Output

~~~~
addr interval == rowBytes
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkBitmap_getAddr8">getAddr8</a> <a href="#SkBitmap_getAddr16">getAddr16</a> <a href="#SkBitmap_getAddr32">getAddr32</a> <a href="#SkBitmap_readPixels">readPixels</a> <a href="#SkPixmap_addr">SkPixmap::addr</a>

---

<a name="SkBitmap_getAddr32"></a>
## getAddr32

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
inline uint32_t* getAddr32(int x, int y) const
</pre>

Returns address at (<a href="#SkBitmap_getAddr32_x">x</a>, <a href="#SkBitmap_getAddr32_y">y</a>).

Input is not validated. Triggers an assert() if built with SK_DEBUG defined and:

<table>  <tr>
    <td><a href="undocumented#Pixel_Ref">Pixel Ref</a> is nullptr</td>  </tr>  <tr>
    <td><a href="#SkBitmap_bytesPerPixel">bytesPerPixel</a> is not four</td>  </tr>  <tr>
    <td><a href="#SkBitmap_getAddr32_x">x</a> is negative, or not less than <a href="#SkBitmap_width">width</a></td>  </tr>  <tr>
    <td><a href="#SkBitmap_getAddr32_y">y</a> is negative, or not less than <a href="#SkBitmap_height">height</a></td>  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name="SkBitmap_getAddr32_x"> <code><strong>x </strong></code> </a></td> <td>
column index, zero or greater, and less than <a href="#SkBitmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkBitmap_getAddr32_y"> <code><strong>y </strong></code> </a></td> <td>
row index, zero or greater, and less than <a href="#SkBitmap_height">height</a></td>
  </tr>
</table>

### Return Value

unsigned 32-bit pointer to pixel at (<a href="#SkBitmap_getAddr32_x">x</a>, <a href="#SkBitmap_getAddr32_y">y</a>)

### Example

<div><fiddle-embed name="837a2bcc9fb9ce617a3420956cefc64a">

#### Example Output

~~~~
addr interval == rowBytes
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkBitmap_getAddr8">getAddr8</a> <a href="#SkBitmap_getAddr16">getAddr16</a> <a href="#SkBitmap_getAddr">getAddr</a> <a href="#SkBitmap_readPixels">readPixels</a> <a href="#SkPixmap_addr32">SkPixmap::addr32</a>

---

<a name="SkBitmap_getAddr16"></a>
## getAddr16

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
inline uint16_t* getAddr16(int x, int y) const
</pre>

Returns address at (<a href="#SkBitmap_getAddr16_x">x</a>, <a href="#SkBitmap_getAddr16_y">y</a>).

Input is not validated. Triggers an assert() if built with SK_DEBUG defined and:

<table>  <tr>
    <td><a href="undocumented#Pixel_Ref">Pixel Ref</a> is nullptr</td>  </tr>  <tr>
    <td><a href="#SkBitmap_bytesPerPixel">bytesPerPixel</a> is not two</td>  </tr>  <tr>
    <td><a href="#SkBitmap_getAddr16_x">x</a> is negative, or not less than <a href="#SkBitmap_width">width</a></td>  </tr>  <tr>
    <td><a href="#SkBitmap_getAddr16_y">y</a> is negative, or not less than <a href="#SkBitmap_height">height</a></td>  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name="SkBitmap_getAddr16_x"> <code><strong>x </strong></code> </a></td> <td>
column index, zero or greater, and less than <a href="#SkBitmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkBitmap_getAddr16_y"> <code><strong>y </strong></code> </a></td> <td>
row index, zero or greater, and less than <a href="#SkBitmap_height">height</a></td>
  </tr>
</table>

### Return Value

unsigned 16-bit pointer to pixel at (<a href="#SkBitmap_getAddr16_x">x</a>, <a href="#SkBitmap_getAddr16_y">y</a>)

### Example

<div><fiddle-embed name="3d7e49d428c9a4812558b39cf505baa6">

#### Example Output

~~~~
addr interval == rowBytes
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkBitmap_getAddr8">getAddr8</a> <a href="#SkBitmap_getAddr">getAddr</a> <a href="#SkBitmap_getAddr32">getAddr32</a> <a href="#SkBitmap_readPixels">readPixels</a> <a href="#SkPixmap_addr16">SkPixmap::addr16</a>

---

<a name="SkBitmap_getAddr8"></a>
## getAddr8

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
inline uint8_t* getAddr8(int x, int y) const
</pre>

Returns address at (<a href="#SkBitmap_getAddr8_x">x</a>, <a href="#SkBitmap_getAddr8_y">y</a>).

Input is not validated. Triggers an assert() if built with SK_DEBUG defined and:

<table>  <tr>
    <td><a href="undocumented#Pixel_Ref">Pixel Ref</a> is nullptr</td>  </tr>  <tr>
    <td><a href="#SkBitmap_bytesPerPixel">bytesPerPixel</a> is not one</td>  </tr>  <tr>
    <td><a href="#SkBitmap_getAddr8_x">x</a> is negative, or not less than <a href="#SkBitmap_width">width</a></td>  </tr>  <tr>
    <td><a href="#SkBitmap_getAddr8_y">y</a> is negative, or not less than <a href="#SkBitmap_height">height</a></td>  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name="SkBitmap_getAddr8_x"> <code><strong>x </strong></code> </a></td> <td>
column index, zero or greater, and less than <a href="#SkBitmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkBitmap_getAddr8_y"> <code><strong>y </strong></code> </a></td> <td>
row index, zero or greater, and less than <a href="#SkBitmap_height">height</a></td>
  </tr>
</table>

### Return Value

unsigned 8-bit pointer to pixel at (<a href="#SkBitmap_getAddr8_x">x</a>, <a href="#SkBitmap_getAddr8_y">y</a>)

### Example

<div><fiddle-embed name="cb9a08e8ff779b6a1cf8bb54f3883aaf">

#### Example Output

~~~~
&pixels[4][2] == bitmap.getAddr8(2, 4)
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkBitmap_getAddr">getAddr</a> <a href="#SkBitmap_getAddr16">getAddr16</a> <a href="#SkBitmap_getAddr32">getAddr32</a> <a href="#SkBitmap_readPixels">readPixels</a> <a href="#SkPixmap_addr8">SkPixmap::addr8</a>

---

<a name="SkBitmap_extractSubset"></a>
## extractSubset

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool extractSubset(SkBitmap* dst, const SkIRect& subset) const
</pre>

Shares <a href="undocumented#Pixel_Ref">Pixel Ref</a> with <a href="#SkBitmap_extractSubset_dst">dst</a>. Pixels are not copied; <a href="#Bitmap">Bitmap</a> and <a href="#SkBitmap_extractSubset_dst">dst</a> point
to the same pixels; <a href="#SkBitmap_extractSubset_dst">dst</a> <a href="#SkBitmap_bounds">bounds</a> are set to the intersection of <a href="#SkBitmap_extractSubset_subset">subset</a>
and the original <a href="#SkBitmap_bounds">bounds</a>.

<a href="#SkBitmap_extractSubset_subset">subset</a> may be larger than <a href="#SkBitmap_bounds">bounds</a>. Any area outside of <a href="#SkBitmap_bounds">bounds</a> is ignored.

Any contents of <a href="#SkBitmap_extractSubset_dst">dst</a> are discarded. <a href="#SkBitmap_isVolatile">isVolatile</a> setting is copied to <a href="#SkBitmap_extractSubset_dst">dst</a>.
<a href="#SkBitmap_extractSubset_dst">dst</a> is set to <a href="#SkBitmap_colorType">colorType</a>, <a href="#SkBitmap_alphaType">alphaType</a>, and <a href="#SkBitmap_colorSpace">colorSpace</a>.

Return false if:

<table>  <tr>
    <td><a href="#SkBitmap_extractSubset_dst">dst</a> is nullptr</td>  </tr>  <tr>
    <td><a href="undocumented#Pixel_Ref">Pixel Ref</a> is nullptr</td>  </tr>  <tr>
    <td><a href="#SkBitmap_extractSubset_subset">subset</a> does not intersect <a href="#SkBitmap_bounds">bounds</a></td>  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name="SkBitmap_extractSubset_dst"> <code><strong>dst </strong></code> </a></td> <td>
<a href="#Bitmap">Bitmap</a> set to <a href="#SkBitmap_extractSubset_subset">subset</a></td>
  </tr>  <tr>    <td><a name="SkBitmap_extractSubset_subset"> <code><strong>subset </strong></code> </a></td> <td>
rectangle of pixels to reference</td>
  </tr>
</table>

### Return Value

true if <a href="#SkBitmap_extractSubset_dst">dst</a> is replaced by <a href="#SkBitmap_extractSubset_subset">subset</a>

### Example

<div><fiddle-embed name="3ad80d8fcb13344612f361812658b696">

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

<a href="#SkBitmap_readPixels">readPixels</a> <a href="#SkBitmap_writePixels">writePixels</a> <a href="#SkCanvas_drawBitmap">SkCanvas::drawBitmap</a>

---

<a name="SkBitmap_readPixels"></a>
## readPixels

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes, int srcX, int srcY,
                SkTransferFunctionBehavior behavior) const
</pre>

Copies <a href="SkRect_Reference#Rect">Rect</a> of pixels from <a href="#Bitmap">Bitmap</a> pixels to <a href="#SkBitmap_readPixels_dstPixels">dstPixels</a>. Copy starts at (<a href="#SkBitmap_readPixels_srcX">srcX</a>, <a href="#SkBitmap_readPixels_srcY">srcY</a>),
and does not exceed <a href="#Bitmap">Bitmap</a> (<a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>).

<a href="#SkBitmap_readPixels_dstInfo">dstInfo</a> specifies <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, and
<a href="undocumented#Color_Space">Color Space</a> of destination. <a href="#SkBitmap_readPixels_dstRowBytes">dstRowBytes</a> specifics the gap from one destination
row to the next. Returns true if pixels are copied. Returns false if:

<table>  <tr>
    <td><a href="#SkBitmap_readPixels_dstInfo">dstInfo</a>.addr() equals nullptr</td>  </tr>  <tr>
    <td><a href="#SkBitmap_readPixels_dstRowBytes">dstRowBytes</a> is less than <a href="#SkBitmap_readPixels_dstInfo">dstInfo</a>.<a href="undocumented#SkImageInfo">minRowBytes</a></td>  </tr>  <tr>
    <td><a href="undocumented#Pixel_Ref">Pixel Ref</a> is nullptr</td>  </tr>
</table>

Pixels are copied only if pixel conversion is possible. If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_colorType">colorType</a> is
<a href="undocumented#SkColorType">kGray 8 SkColorType</a>, or <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>; <a href="#SkBitmap_readPixels_dstInfo">dstInfo</a>.<a href="#SkBitmap_colorType">colorType</a> must match.
If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kGray 8 SkColorType</a>, <a href="#SkBitmap_readPixels_dstInfo">dstInfo</a>.<a href="#SkBitmap_colorSpace">colorSpace</a> must match.
If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_alphaType">alphaType</a> is <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>, <a href="#SkBitmap_readPixels_dstInfo">dstInfo</a>.<a href="#SkBitmap_alphaType">alphaType</a> must
match. If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_colorSpace">colorSpace</a> is nullptr, <a href="#SkBitmap_readPixels_dstInfo">dstInfo</a>.<a href="#SkBitmap_colorSpace">colorSpace</a> must match. Returns
false if pixel conversion is not possible.
<a href="#SkBitmap_readPixels_srcX">srcX</a> and <a href="#SkBitmap_readPixels_srcY">srcY</a> may be negative to copy only top or left of source. Returns
false if <a href="#SkBitmap_width">width</a> or <a href="#SkBitmap_height">height</a> is zero or negative.
Returns false ifabs(srcX) >= <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_width">width</a>,
or ifabs(srcY) >= <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_height">height</a>.

If <a href="#SkBitmap_readPixels_behavior">behavior</a> is <a href="#SkTransferFunctionBehavior_kRespect">SkTransferFunctionBehavior::kRespect</a>: converts source
pixels to a linear space before converting to <a href="#SkBitmap_readPixels_dstInfo">dstInfo</a>.
If <a href="#SkBitmap_readPixels_behavior">behavior</a> is <a href="#SkTransferFunctionBehavior_kIgnore">SkTransferFunctionBehavior::kIgnore</a>: source
pixels are treated as if they are linear, regardless of how they are encoded.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_readPixels_dstInfo"> <code><strong>dstInfo </strong></code> </a></td> <td>
destination <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a></td>
  </tr>  <tr>    <td><a name="SkBitmap_readPixels_dstPixels"> <code><strong>dstPixels </strong></code> </a></td> <td>
destination pixel storage</td>
  </tr>  <tr>    <td><a name="SkBitmap_readPixels_dstRowBytes"> <code><strong>dstRowBytes </strong></code> </a></td> <td>
destination row length</td>
  </tr>  <tr>    <td><a name="SkBitmap_readPixels_srcX"> <code><strong>srcX </strong></code> </a></td> <td>
column index whose absolute value is less than <a href="#SkBitmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkBitmap_readPixels_srcY"> <code><strong>srcY </strong></code> </a></td> <td>
row index whose absolute value is less than <a href="#SkBitmap_height">height</a></td>
  </tr>  <tr>    <td><a name="SkBitmap_readPixels_behavior"> <code><strong>behavior </strong></code> </a></td> <td>
one of: <a href="#SkTransferFunctionBehavior_kRespect">SkTransferFunctionBehavior::kRespect</a>,
<a href="#SkTransferFunctionBehavior_kIgnore">SkTransferFunctionBehavior::kIgnore</a></td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href="#SkBitmap_readPixels_dstPixels">dstPixels</a>

### Example

<div><fiddle-embed name="86fb9979516d256d71aa3f3f2182fe0c"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_writePixels">writePixels</a> <a href="#SkPixmap_readPixels">SkPixmap::readPixels</a> <a href="#SkCanvas_readPixels">SkCanvas::readPixels</a> <a href="#SkImage_readPixels">SkImage::readPixels</a> <a href="#SkSurface_readPixels">SkSurface::readPixels</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool readPixels(const SkImageInfo& dstInfo, void* dstPixels, size_t dstRowBytes, int srcX, int srcY) const
</pre>

Copies a <a href="SkRect_Reference#Rect">Rect</a> of pixels from <a href="#Bitmap">Bitmap</a> to <a href="#SkBitmap_readPixels_2_dstPixels">dstPixels</a>. Copy starts at (<a href="#SkBitmap_readPixels_2_srcX">srcX</a>, <a href="#SkBitmap_readPixels_2_srcY">srcY</a>),
and does not exceed <a href="#Bitmap">Bitmap</a> (<a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>).

<a href="#SkBitmap_readPixels_2_dstInfo">dstInfo</a> specifies <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, and <a href="undocumented#Color_Space">Color Space</a> of
destination. <a href="#SkBitmap_readPixels_2_dstRowBytes">dstRowBytes</a> specifics the gap from one destination row to the next.
Returns true if pixels are copied. Returns false if:

<table>  <tr>
    <td><a href="#SkBitmap_readPixels_2_dstInfo">dstInfo</a>.addr() equals nullptr</td>  </tr>  <tr>
    <td><a href="#SkBitmap_readPixels_2_dstRowBytes">dstRowBytes</a> is less than <a href="#SkBitmap_readPixels_2_dstInfo">dstInfo</a>.<a href="undocumented#SkImageInfo">minRowBytes</a></td>  </tr>  <tr>
    <td><a href="undocumented#Pixel_Ref">Pixel Ref</a> is nullptr</td>  </tr>
</table>

Pixels are copied only if pixel conversion is possible. If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_colorType">colorType</a> is
<a href="undocumented#SkColorType">kGray 8 SkColorType</a>, or <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>; <a href="#SkBitmap_readPixels_2_dstInfo">dstInfo</a>.<a href="#SkBitmap_colorType">colorType</a> must match.
If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kGray 8 SkColorType</a>, <a href="#SkBitmap_readPixels_2_dstInfo">dstInfo</a>.<a href="#SkBitmap_colorSpace">colorSpace</a> must match.
If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_alphaType">alphaType</a> is <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>, <a href="#SkBitmap_readPixels_2_dstInfo">dstInfo</a>.<a href="#SkBitmap_alphaType">alphaType</a> must
match. If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_colorSpace">colorSpace</a> is nullptr, <a href="#SkBitmap_readPixels_2_dstInfo">dstInfo</a>.<a href="#SkBitmap_colorSpace">colorSpace</a> must match. Returns
false if pixel conversion is not possible.
<a href="#SkBitmap_readPixels_2_srcX">srcX</a> and <a href="#SkBitmap_readPixels_2_srcY">srcY</a> may be negative to copy only top or left of source. Returns
false if <a href="#SkBitmap_width">width</a> or <a href="#SkBitmap_height">height</a> is zero or negative.
Returns false ifabs(srcX) >= <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_width">width</a>,
or ifabs(srcY) >= <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_height">height</a>.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_readPixels_2_dstInfo"> <code><strong>dstInfo </strong></code> </a></td> <td>
destination <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a></td>
  </tr>  <tr>    <td><a name="SkBitmap_readPixels_2_dstPixels"> <code><strong>dstPixels </strong></code> </a></td> <td>
destination pixel storage</td>
  </tr>  <tr>    <td><a name="SkBitmap_readPixels_2_dstRowBytes"> <code><strong>dstRowBytes </strong></code> </a></td> <td>
destination row length</td>
  </tr>  <tr>    <td><a name="SkBitmap_readPixels_2_srcX"> <code><strong>srcX </strong></code> </a></td> <td>
column index whose absolute value is less than <a href="#SkBitmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkBitmap_readPixels_2_srcY"> <code><strong>srcY </strong></code> </a></td> <td>
row index whose absolute value is less than <a href="#SkBitmap_height">height</a></td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href="#SkBitmap_readPixels_2_dstPixels">dstPixels</a>

### Example

<div><fiddle-embed name="b2cbbbbcffb618865d8aae3bc04b2a62"><div>Transferring the gradient from 8 bits per component to 4 bits per component
creates visible banding.</div></fiddle-embed></div>

### See Also

<a href="#SkBitmap_writePixels">writePixels</a> <a href="#SkPixmap_readPixels">SkPixmap::readPixels</a> <a href="#SkCanvas_readPixels">SkCanvas::readPixels</a> <a href="#SkImage_readPixels">SkImage::readPixels</a> <a href="#SkSurface_readPixels">SkSurface::readPixels</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool readPixels(const SkPixmap& dst, int srcX, int srcY) const
</pre>

Copies a <a href="SkRect_Reference#Rect">Rect</a> of pixels from <a href="#Bitmap">Bitmap</a> to <a href="#SkBitmap_readPixels_3_dst">dst</a>. Copy starts at (<a href="#SkBitmap_readPixels_3_srcX">srcX</a>, <a href="#SkBitmap_readPixels_3_srcY">srcY</a>), and
does not exceed <a href="#Bitmap">Bitmap</a> (<a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>).

<a href="#SkBitmap_readPixels_3_dst">dst</a> specifies <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a>, pixel storage,
and row bytes of destination. <a href="#SkBitmap_readPixels_3_dst">dst</a>.<a href="#SkBitmap_rowBytes">rowBytes</a> specifics the gap from one destination
row to the next. Returns true if pixels are copied. Returns false if:

<table>  <tr>
    <td><a href="#SkBitmap_readPixels_3_dst">dst</a> pixel storage equals nullptr</td>  </tr>  <tr>
    <td><a href="#SkBitmap_readPixels_3_dst">dst</a>.<a href="#SkBitmap_rowBytes">rowBytes</a> is less than <a href="#SkImageInfo_minRowBytes">SkImageInfo::minRowBytes</a></td>  </tr>  <tr>
    <td><a href="undocumented#Pixel_Ref">Pixel Ref</a> is nullptr</td>  </tr>
</table>

Pixels are copied only if pixel conversion is possible. If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_colorType">colorType</a> is
<a href="undocumented#SkColorType">kGray 8 SkColorType</a>, or <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>; <a href="#SkBitmap_readPixels_3_dst">dst</a> <a href="undocumented#Image_Color_Type">Color Type</a> must match.
If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kGray 8 SkColorType</a>, <a href="#SkBitmap_readPixels_3_dst">dst</a> <a href="undocumented#Color_Space">Color Space</a> must match.
If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_alphaType">alphaType</a> is <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>, <a href="#SkBitmap_readPixels_3_dst">dst</a> <a href="undocumented#Image_Alpha_Type">Alpha Type</a> must
match. If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_colorSpace">colorSpace</a> is nullptr, <a href="#SkBitmap_readPixels_3_dst">dst</a> <a href="undocumented#Color_Space">Color Space</a> must match. Returns
false if pixel conversion is not possible.
<a href="#SkBitmap_readPixels_3_srcX">srcX</a> and <a href="#SkBitmap_readPixels_3_srcY">srcY</a> may be negative to copy only top or left of source. Returns
false if <a href="#SkBitmap_width">width</a> or <a href="#SkBitmap_height">height</a> is zero or negative.
Returns false ifabs(srcX) >= <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_width">width</a>,
or ifabs(srcY) >= <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_height">height</a>.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_readPixels_3_dst"> <code><strong>dst </strong></code> </a></td> <td>
destination <a href="SkPixmap_Reference#Pixmap">Pixmap</a>: <a href="undocumented#Image_Info">Image Info</a>, pixels, row bytes</td>
  </tr>  <tr>    <td><a name="SkBitmap_readPixels_3_srcX"> <code><strong>srcX </strong></code> </a></td> <td>
column index whose absolute value is less than <a href="#SkBitmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkBitmap_readPixels_3_srcY"> <code><strong>srcY </strong></code> </a></td> <td>
row index whose absolute value is less than <a href="#SkBitmap_height">height</a></td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href="#SkBitmap_readPixels_3_dst">dst</a>

### Example

<div><fiddle-embed name="e9f70cbc9827097449a386ec7a8a8188"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_writePixels">writePixels</a> <a href="#SkPixmap_readPixels">SkPixmap::readPixels</a> <a href="#SkCanvas_readPixels">SkCanvas::readPixels</a> <a href="#SkImage_readPixels">SkImage::readPixels</a> <a href="#SkSurface_readPixels">SkSurface::readPixels</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool readPixels(const SkPixmap& dst) const
</pre>

Copies a <a href="SkRect_Reference#Rect">Rect</a> of pixels from <a href="#Bitmap">Bitmap</a> to <a href="#SkBitmap_readPixels_4_dst">dst</a>. Copy starts at (0, 0), and
does not exceed <a href="#Bitmap">Bitmap</a> (<a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>).

<a href="#SkBitmap_readPixels_4_dst">dst</a> specifies <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a>, pixel storage,
and row bytes of destination. <a href="#SkBitmap_readPixels_4_dst">dst</a>.<a href="#SkBitmap_rowBytes">rowBytes</a> specifics the gap from one destination
row to the next. Returns true if pixels are copied. Returns false if:

<table>  <tr>
    <td><a href="#SkBitmap_readPixels_4_dst">dst</a> pixel storage equals nullptr</td>  </tr>  <tr>
    <td><a href="#SkBitmap_readPixels_4_dst">dst</a>.<a href="#SkBitmap_rowBytes">rowBytes</a> is less than <a href="#SkImageInfo_minRowBytes">SkImageInfo::minRowBytes</a></td>  </tr>  <tr>
    <td><a href="undocumented#Pixel_Ref">Pixel Ref</a> is nullptr</td>  </tr>
</table>

Pixels are copied only if pixel conversion is possible. If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_colorType">colorType</a> is
<a href="undocumented#SkColorType">kGray 8 SkColorType</a>, or <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>; <a href="#SkBitmap_readPixels_4_dst">dst</a> <a href="undocumented#Image_Color_Type">Color Type</a> must match.
If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kGray 8 SkColorType</a>, <a href="#SkBitmap_readPixels_4_dst">dst</a> <a href="undocumented#Color_Space">Color Space</a> must match.
If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_alphaType">alphaType</a> is <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>, <a href="#SkBitmap_readPixels_4_dst">dst</a> <a href="undocumented#Image_Alpha_Type">Alpha Type</a> must
match. If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_colorSpace">colorSpace</a> is nullptr, <a href="#SkBitmap_readPixels_4_dst">dst</a> <a href="undocumented#Color_Space">Color Space</a> must match. Returns
false if pixel conversion is not possible.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_readPixels_4_dst"> <code><strong>dst </strong></code> </a></td> <td>
destination <a href="SkPixmap_Reference#Pixmap">Pixmap</a>: <a href="undocumented#Image_Info">Image Info</a>, pixels, row bytes</td>
  </tr>
</table>

### Return Value

true if pixels are copied to <a href="#SkBitmap_readPixels_4_dst">dst</a>

### Example

<div><fiddle-embed name="2260ff6d04708699b6223b4f73fd268e"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_writePixels">writePixels</a> <a href="#SkPixmap_readPixels">SkPixmap::readPixels</a> <a href="#SkCanvas_readPixels">SkCanvas::readPixels</a> <a href="#SkImage_readPixels">SkImage::readPixels</a> <a href="#SkSurface_readPixels">SkSurface::readPixels</a>

---

<a name="SkBitmap_writePixels"></a>
## writePixels

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool writePixels(const SkPixmap& src, int dstX, int dstY)
</pre>

Copies a <a href="SkRect_Reference#Rect">Rect</a> of pixels from <a href="#SkBitmap_writePixels_src">src</a>. Copy starts at (<a href="#SkBitmap_writePixels_dstX">dstX</a>, <a href="#SkBitmap_writePixels_dstY">dstY</a>), and does not exceed
(<a href="#SkBitmap_writePixels_src">src</a>.<a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_writePixels_src">src</a>.<a href="#SkBitmap_height">height</a>).

<a href="#SkBitmap_writePixels_src">src</a> specifies <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a>, pixel storage,
and row bytes of source. <a href="#SkBitmap_writePixels_src">src</a>.<a href="#SkBitmap_rowBytes">rowBytes</a> specifics the gap from one source
row to the next. Returns true if pixels are copied. Returns false if:

<table>  <tr>
    <td><a href="#SkBitmap_writePixels_src">src</a> pixel storage equals nullptr</td>  </tr>  <tr>
    <td><a href="#SkBitmap_writePixels_src">src</a>.<a href="#SkBitmap_rowBytes">rowBytes</a> is less than <a href="#SkImageInfo_minRowBytes">SkImageInfo::minRowBytes</a></td>  </tr>  <tr>
    <td><a href="undocumented#Pixel_Ref">Pixel Ref</a> is nullptr</td>  </tr>
</table>

Pixels are copied only if pixel conversion is possible. If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_colorType">colorType</a> is
<a href="undocumented#SkColorType">kGray 8 SkColorType</a>, or <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>; <a href="#SkBitmap_writePixels_src">src</a> <a href="undocumented#Image_Color_Type">Color Type</a> must match.
If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kGray 8 SkColorType</a>, <a href="#SkBitmap_writePixels_src">src</a> <a href="undocumented#Color_Space">Color Space</a> must match.
If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_alphaType">alphaType</a> is <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>, <a href="#SkBitmap_writePixels_src">src</a> <a href="undocumented#Image_Alpha_Type">Alpha Type</a> must
match. If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_colorSpace">colorSpace</a> is nullptr, <a href="#SkBitmap_writePixels_src">src</a> <a href="undocumented#Color_Space">Color Space</a> must match. Returns
false if pixel conversion is not possible.
<a href="#SkBitmap_writePixels_dstX">dstX</a> and <a href="#SkBitmap_writePixels_dstY">dstY</a> may be negative to copy only top or left of source. Returns
false if <a href="#SkBitmap_width">width</a> or <a href="#SkBitmap_height">height</a> is zero or negative.
Returns false ifabs(dstX) >= <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_width">width</a>,
or ifabs(dstY) >= <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_height">height</a>.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_writePixels_src"> <code><strong>src </strong></code> </a></td> <td>
source <a href="SkPixmap_Reference#Pixmap">Pixmap</a>: <a href="undocumented#Image_Info">Image Info</a>, pixels, row bytes</td>
  </tr>  <tr>    <td><a name="SkBitmap_writePixels_dstX"> <code><strong>dstX </strong></code> </a></td> <td>
column index whose absolute value is less than <a href="#SkBitmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkBitmap_writePixels_dstY"> <code><strong>dstY </strong></code> </a></td> <td>
row index whose absolute value is less than <a href="#SkBitmap_height">height</a></td>
  </tr>
</table>

### Return Value

true if <a href="#SkBitmap_writePixels_src">src</a> pixels are copied to <a href="#Bitmap">Bitmap</a>

### Example

<div><fiddle-embed name="9b3133a6673d2514d166398adbe1f9f4"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_readPixels">readPixels</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool writePixels(const SkPixmap& src)
</pre>

Copies a <a href="SkRect_Reference#Rect">Rect</a> of pixels from <a href="#SkBitmap_writePixels_2_src">src</a>. Copy starts at (0, 0), and does not exceed
(<a href="#SkBitmap_writePixels_2_src">src</a>.<a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_writePixels_2_src">src</a>.<a href="#SkBitmap_height">height</a>).

<a href="#SkBitmap_writePixels_2_src">src</a> specifies <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a>, pixel storage,
and row bytes of source. <a href="#SkBitmap_writePixels_2_src">src</a>.<a href="#SkBitmap_rowBytes">rowBytes</a> specifics the gap from one source
row to the next. Returns true if pixels are copied. Returns false if:

<table>  <tr>
    <td><a href="#SkBitmap_writePixels_2_src">src</a> pixel storage equals nullptr</td>  </tr>  <tr>
    <td><a href="#SkBitmap_writePixels_2_src">src</a>.<a href="#SkBitmap_rowBytes">rowBytes</a> is less than <a href="#SkImageInfo_minRowBytes">SkImageInfo::minRowBytes</a></td>  </tr>  <tr>
    <td><a href="undocumented#Pixel_Ref">Pixel Ref</a> is nullptr</td>  </tr>
</table>

Pixels are copied only if pixel conversion is possible. If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_colorType">colorType</a> is
<a href="undocumented#SkColorType">kGray 8 SkColorType</a>, or <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>; <a href="#SkBitmap_writePixels_2_src">src</a> <a href="undocumented#Image_Color_Type">Color Type</a> must match.
If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kGray 8 SkColorType</a>, <a href="#SkBitmap_writePixels_2_src">src</a> <a href="undocumented#Color_Space">Color Space</a> must match.
If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_alphaType">alphaType</a> is <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>, <a href="#SkBitmap_writePixels_2_src">src</a> <a href="undocumented#Image_Alpha_Type">Alpha Type</a> must
match. If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_colorSpace">colorSpace</a> is nullptr, <a href="#SkBitmap_writePixels_2_src">src</a> <a href="undocumented#Color_Space">Color Space</a> must match. Returns
false if pixel conversion is not possible.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_writePixels_2_src"> <code><strong>src </strong></code> </a></td> <td>
source <a href="SkPixmap_Reference#Pixmap">Pixmap</a>: <a href="undocumented#Image_Info">Image Info</a>, pixels, row bytes</td>
  </tr>
</table>

### Return Value

true if <a href="#SkBitmap_writePixels_2_src">src</a> pixels are copied to <a href="#Bitmap">Bitmap</a>

### Example

<div><fiddle-embed name="faa5dfa466f6e16c07c124d971f32679"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_readPixels">readPixels</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool writePixels(const SkPixmap& src, int x, int y, SkTransferFunctionBehavior behavior)
</pre>

Copies a <a href="SkRect_Reference#Rect">Rect</a> of pixels from <a href="#SkBitmap_writePixels_3_src">src</a>. Copy starts at (0, 0), and does not exceed
(<a href="#SkBitmap_writePixels_3_src">src</a>.<a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_writePixels_3_src">src</a>.<a href="#SkBitmap_height">height</a>).

<a href="#SkBitmap_writePixels_3_src">src</a> specifies <a href="#SkBitmap_width">width</a>, <a href="#SkBitmap_height">height</a>, <a href="undocumented#Image_Color_Type">Color Type</a>, <a href="undocumented#Image_Alpha_Type">Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a>, pixel storage,
and row bytes of source. <a href="#SkBitmap_writePixels_3_src">src</a>.<a href="#SkBitmap_rowBytes">rowBytes</a> specifics the gap from one source
row to the next. Returns true if pixels are copied. Returns false if:

<table>  <tr>
    <td><a href="#SkBitmap_writePixels_3_src">src</a> pixel storage equals nullptr</td>  </tr>  <tr>
    <td><a href="#SkBitmap_writePixels_3_src">src</a>.<a href="#SkBitmap_rowBytes">rowBytes</a> is less than <a href="#SkImageInfo_minRowBytes">SkImageInfo::minRowBytes</a></td>  </tr>  <tr>
    <td><a href="undocumented#Pixel_Ref">Pixel Ref</a> is nullptr</td>  </tr>
</table>

Pixels are copied only if pixel conversion is possible. If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_colorType">colorType</a> is
<a href="undocumented#SkColorType">kGray 8 SkColorType</a>, or <a href="undocumented#SkColorType">kAlpha 8 SkColorType</a>; <a href="#SkBitmap_writePixels_3_src">src</a> <a href="undocumented#Image_Color_Type">Color Type</a> must match.
If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_colorType">colorType</a> is <a href="undocumented#SkColorType">kGray 8 SkColorType</a>, <a href="#SkBitmap_writePixels_3_src">src</a> <a href="undocumented#Color_Space">Color Space</a> must match.
If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_alphaType">alphaType</a> is <a href="undocumented#SkAlphaType">kOpaque SkAlphaType</a>, <a href="#SkBitmap_writePixels_3_src">src</a> <a href="undocumented#Image_Alpha_Type">Alpha Type</a> must
match. If <a href="#Bitmap">Bitmap</a> <a href="#SkBitmap_colorSpace">colorSpace</a> is nullptr, <a href="#SkBitmap_writePixels_3_src">src</a> <a href="undocumented#Color_Space">Color Space</a> must match. Returns
false if pixel conversion is not possible. Returns false if <a href="#SkBitmap_width">width</a> or <a href="#SkBitmap_height">height</a>
is zero or negative.

If <a href="#SkBitmap_writePixels_3_behavior">behavior</a> is <a href="#SkTransferFunctionBehavior_kRespect">SkTransferFunctionBehavior::kRespect</a>: converts <a href="#SkBitmap_writePixels_3_src">src</a>
pixels to a linear space before converting to <a href="undocumented#Image_Info">Image Info</a>.
If <a href="#SkBitmap_writePixels_3_behavior">behavior</a> is <a href="#SkTransferFunctionBehavior_kIgnore">SkTransferFunctionBehavior::kIgnore</a>: <a href="#SkBitmap_writePixels_3_src">src</a>
pixels are treated as if they are linear, regardless of how they are encoded.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_writePixels_3_src"> <code><strong>src </strong></code> </a></td> <td>
source <a href="SkPixmap_Reference#Pixmap">Pixmap</a>: <a href="undocumented#Image_Info">Image Info</a>, pixels, row bytes</td>
  </tr>  <tr>    <td><a name="SkBitmap_writePixels_3_x"> <code><strong>x </strong></code> </a></td> <td>
column index whose absolute value is less than <a href="#SkBitmap_width">width</a></td>
  </tr>  <tr>    <td><a name="SkBitmap_writePixels_3_y"> <code><strong>y </strong></code> </a></td> <td>
row index whose absolute value is less than <a href="#SkBitmap_height">height</a></td>
  </tr>  <tr>    <td><a name="SkBitmap_writePixels_3_behavior"> <code><strong>behavior </strong></code> </a></td> <td>
one of: <a href="#SkTransferFunctionBehavior_kRespect">SkTransferFunctionBehavior::kRespect</a>,
<a href="#SkTransferFunctionBehavior_kIgnore">SkTransferFunctionBehavior::kIgnore</a></td>
  </tr>
</table>

### Return Value

true if <a href="#SkBitmap_writePixels_3_src">src</a> pixels are copied to <a href="#Bitmap">Bitmap</a>

### Example

<div><fiddle-embed name="9f046f407b01b759cb737d702a994620"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_readPixels">readPixels</a>

---

<a name="SkBitmap_hasHardwareMipMap"></a>
## hasHardwareMipMap

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool hasHardwareMipMap() const
</pre>

### Return Value

true if <a href="#SkBitmap_setHasHardwareMipMap">setHasHardwareMipMap</a> has been called with true

### See Also

<a href="#SkBitmap_setHasHardwareMipMap">setHasHardwareMipMap</a>

---

<a name="SkBitmap_setHasHardwareMipMap"></a>
## setHasHardwareMipMap

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setHasHardwareMipMap(bool hasHardwareMipMap)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkBitmap_setHasHardwareMipMap_hasHardwareMipMap"> <code><strong>hasHardwareMipMap </strong></code> </a></td> <td>
sets state</td>
  </tr>
</table>

### See Also

<a href="#SkBitmap_hasHardwareMipMap">hasHardwareMipMap</a>

---

<a name="SkBitmap_extractAlpha"></a>
## extractAlpha

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool extractAlpha(SkBitmap* dst) const
</pre>

Sets <a href="#SkBitmap_extractAlpha_dst">dst</a> to <a href="#Alpha">Alpha</a> described by pixels. Returns false if <a href="#SkBitmap_extractAlpha_dst">dst</a> cannot be written to
or <a href="#SkBitmap_extractAlpha_dst">dst</a> pixels cannot be allocated.

Uses <a href="#SkBitmap_HeapAllocator">HeapAllocator</a> to reserve memory for <a href="#SkBitmap_extractAlpha_dst">dst</a> <a href="undocumented#Pixel_Ref">Pixel Ref</a>.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_extractAlpha_dst"> <code><strong>dst </strong></code> </a></td> <td>
holds <a href="undocumented#Pixel_Ref">Pixel Ref</a> to fill with alpha layer</td>
  </tr>
</table>

### Return Value

true if <a href="#Alpha">Alpha</a> layer was constructed in <a href="#SkBitmap_extractAlpha_dst">dst</a> <a href="undocumented#Pixel_Ref">Pixel Ref</a>

### Example

<div><fiddle-embed name="ab6577df079e6c70511cf2bfc6447b44"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_extractSubset">extractSubset</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool extractAlpha(SkBitmap* dst, const SkPaint* paint, SkIPoint* offset) const
</pre>

Sets <a href="#SkBitmap_extractAlpha_2_dst">dst</a> to <a href="#Alpha">Alpha</a> described by pixels. Returns false if <a href="#SkBitmap_extractAlpha_2_dst">dst</a> cannot be written to
or <a href="#SkBitmap_extractAlpha_2_dst">dst</a> pixels cannot be allocated.

If <a href="#SkBitmap_extractAlpha_2_paint">paint</a> is not nullptr and contains <a href="undocumented#Mask_Filter">Mask Filter</a>, <a href="#SkMaskFilter_filterMask">SkMaskFilter::filterMask</a>
generates <a href="undocumented#Mask_Alpha">Mask Alpha</a> from <a href="#Bitmap">Bitmap</a>. Uses <a href="#SkBitmap_HeapAllocator">HeapAllocator</a> to reserve memory for <a href="#SkBitmap_extractAlpha_2_dst">dst</a>
<a href="undocumented#Pixel_Ref">Pixel Ref</a>. Sets <a href="#SkBitmap_extractAlpha_2_offset">offset</a> to top-left position for <a href="#SkBitmap_extractAlpha_2_dst">dst</a> for alignment with <a href="#Bitmap">Bitmap</a>;
(0, 0) unless <a href="undocumented#SkMaskFilter">SkMaskFilter</a> generates mask.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_extractAlpha_2_dst"> <code><strong>dst </strong></code> </a></td> <td>
holds <a href="undocumented#Pixel_Ref">Pixel Ref</a> to fill with alpha layer</td>
  </tr>  <tr>    <td><a name="SkBitmap_extractAlpha_2_paint"> <code><strong>paint </strong></code> </a></td> <td>
holds optional <a href="undocumented#Mask_Filter">Mask Filter</a>; may be nullptr</td>
  </tr>  <tr>    <td><a name="SkBitmap_extractAlpha_2_offset"> <code><strong>offset </strong></code> </a></td> <td>
top-left position for <a href="#SkBitmap_extractAlpha_2_dst">dst</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

true if <a href="#Alpha">Alpha</a> layer was constructed in <a href="#SkBitmap_extractAlpha_2_dst">dst</a> <a href="undocumented#Pixel_Ref">Pixel Ref</a>

### Example

<div><fiddle-embed name="e122936b119f20098aace454c2105a12"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_extractSubset">extractSubset</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool extractAlpha(SkBitmap* dst, const SkPaint* paint, Allocator* allocator, SkIPoint* offset) const
</pre>

Sets <a href="#SkBitmap_extractAlpha_3_dst">dst</a> to <a href="#Alpha">Alpha</a> described by pixels. Returns false if <a href="#SkBitmap_extractAlpha_3_dst">dst</a> cannot be written to
or <a href="#SkBitmap_extractAlpha_3_dst">dst</a> pixels cannot be allocated.

If <a href="#SkBitmap_extractAlpha_3_paint">paint</a> is not nullptr and contains <a href="undocumented#Mask_Filter">Mask Filter</a>, <a href="#SkMaskFilter_filterMask">SkMaskFilter::filterMask</a>
generates <a href="undocumented#Mask_Alpha">Mask Alpha</a> from <a href="#Bitmap">Bitmap</a>. <a href="#SkBitmap_extractAlpha_3_allocator">allocator</a> may reference a custom allocation
class or be set to nullptr to use <a href="#SkBitmap_HeapAllocator">HeapAllocator</a>. Sets <a href="#SkBitmap_extractAlpha_3_offset">offset</a> to top-left
position for <a href="#SkBitmap_extractAlpha_3_dst">dst</a> for alignment with <a href="#Bitmap">Bitmap</a>; (0, 0) unless <a href="undocumented#SkMaskFilter">SkMaskFilter</a> generates
mask.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_extractAlpha_3_dst"> <code><strong>dst </strong></code> </a></td> <td>
holds <a href="undocumented#Pixel_Ref">Pixel Ref</a> to fill with alpha layer</td>
  </tr>  <tr>    <td><a name="SkBitmap_extractAlpha_3_paint"> <code><strong>paint </strong></code> </a></td> <td>
holds optional <a href="undocumented#Mask_Filter">Mask Filter</a>; may be nullptr</td>
  </tr>  <tr>    <td><a name="SkBitmap_extractAlpha_3_allocator"> <code><strong>allocator </strong></code> </a></td> <td>
method to reserve memory for <a href="undocumented#Pixel_Ref">Pixel Ref</a>; may be nullptr</td>
  </tr>  <tr>    <td><a name="SkBitmap_extractAlpha_3_offset"> <code><strong>offset </strong></code> </a></td> <td>
top-left position for <a href="#SkBitmap_extractAlpha_3_dst">dst</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

true if <a href="#Alpha">Alpha</a> layer was constructed in <a href="#SkBitmap_extractAlpha_3_dst">dst</a> <a href="undocumented#Pixel_Ref">Pixel Ref</a>

### Example

<div><fiddle-embed name="3563cb608791d55bb584f38e842f3003"></fiddle-embed></div>

### See Also

<a href="#SkBitmap_extractSubset">extractSubset</a>

---

<a name="SkBitmap_peekPixels"></a>
## peekPixels

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool peekPixels(SkPixmap* pixmap) const
</pre>

Copies <a href="#Bitmap">Bitmap</a> pixel address, row bytes, and <a href="undocumented#Image_Info">Image Info</a> to <a href="#SkBitmap_pixmap">pixmap</a>, if address
is available, and returns true. If pixel address is not available, return
false and leave <a href="#SkBitmap_pixmap">pixmap</a> unchanged.

<a href="#SkBitmap_pixmap">pixmap</a> contents become invalid on any future change to <a href="#Bitmap">Bitmap</a>.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_peekPixels_pixmap"> <code><strong>pixmap </strong></code> </a></td> <td>
storage for pixel state if pixels are readable; otherwise, ignored</td>
  </tr>
</table>

### Return Value

true if <a href="#Bitmap">Bitmap</a> has direct access to pixels

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

<a href="#SkBitmap_pixmap">pixmap</a> <a href="#SkBitmap_installPixels">installPixels</a> <a href="#SkBitmap_readPixels">readPixels</a> <a href="#SkBitmap_writePixels">writePixels</a>

---

<a name="SkBitmap_validate"></a>
## validate

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void validate() const;
</pre>

Asserts if internal values are illegal or inconsistent. Only available if
SK_DEBUG is defined at compile time.

### See Also

<a href="#SkImageInfo_validate">SkImageInfo::validate()</a>

---

<a name="SkBitmap_toString"></a>
## toString

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void toString(SkString* str) const;
</pre>

Creates string representation of <a href="#Bitmap">Bitmap</a>. The representation is read by
internal debugging tools. The interface and implementation may be
suppressed by defining SK_IGNORE_TO_STRING.

### Parameters

<table>  <tr>    <td><a name="SkBitmap_toString_str"> <code><strong>str </strong></code> </a></td> <td>
storage for string representation</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6018d98695245510839a318caabba9c9">

#### Example Output

~~~~
bitmap dimensions (6, 11)
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkPaint_toString">SkPaint::toString</a>

---

