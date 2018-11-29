SkBitmap Reference
===


<a name='SkBitmap'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> {

    <a href='#SkBitmap_empty_constructor'>SkBitmap()</a>;
    <a href='#SkBitmap_copy_const_SkBitmap'>SkBitmap</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& src);
    <a href='#SkBitmap_move_SkBitmap'>SkBitmap</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>&& src);
    <a href='#SkBitmap_destructor'>~SkBitmap()</a>;
    <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='#SkBitmap_copy_operator'>operator=</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& src);
    <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='#SkBitmap_move_operator'>operator=</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>&& src);
    void <a href='#SkBitmap_swap'>swap</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& other);
    const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='#SkBitmap_pixmap'>pixmap()</a> const;
    const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='#SkBitmap_info'>info()</a> const;
    int <a href='#SkBitmap_width'>width()</a> const;
    int <a href='#SkBitmap_height'>height()</a> const;
    <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkBitmap_colorType'>colorType</a>() const;
    <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkBitmap_alphaType'>alphaType</a>() const;
    <a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='#SkBitmap_colorSpace'>colorSpace</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkColorSpace'>SkColorSpace</a>> <a href='#SkBitmap_refColorSpace'>refColorSpace</a>() const;
    int <a href='#SkBitmap_bytesPerPixel'>bytesPerPixel</a>() const;
    int <a href='#SkBitmap_rowBytesAsPixels'>rowBytesAsPixels</a>() const;
    int <a href='#SkBitmap_shiftPerPixel'>shiftPerPixel</a>() const;
    bool <a href='#SkBitmap_empty'>empty()</a> const;
    bool <a href='#SkBitmap_isNull'>isNull</a>() const;
    bool <a href='#SkBitmap_drawsNothing'>drawsNothing</a>() const;
    size_t <a href='#SkBitmap_rowBytes'>rowBytes</a>() const;
    bool <a href='#SkBitmap_setAlphaType'>setAlphaType</a>(<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkBitmap_alphaType'>alphaType</a>);
    void* <a href='#SkBitmap_getPixels'>getPixels</a>() const;
    size_t <a href='#SkBitmap_computeByteSize'>computeByteSize</a>() const;
    bool <a href='#SkBitmap_isImmutable'>isImmutable</a>() const;
    void <a href='#SkBitmap_setImmutable'>setImmutable</a>();
    bool <a href='#SkBitmap_isOpaque'>isOpaque</a>() const;
    bool <a href='#SkBitmap_isVolatile'>isVolatile</a>() const;
    void <a href='#SkBitmap_setIsVolatile'>setIsVolatile</a>(bool <a href='#SkBitmap_isVolatile'>isVolatile</a>);
    void <a href='#SkBitmap_reset'>reset()</a>;
    static bool <a href='#SkBitmap_ComputeIsOpaque'>ComputeIsOpaque</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& bm);
    void <a href='#SkBitmap_getBounds'>getBounds</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* bounds) const;
    void <a href='#SkBitmap_getBounds'>getBounds</a>(<a href='SkIRect_Reference#SkIRect'>SkIRect</a>* bounds) const;
    <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkBitmap_bounds'>bounds()</a> const;
    <a href='undocumented#SkISize'>SkISize</a> <a href='#SkBitmap_dimensions'>dimensions()</a> const;
    <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkBitmap_getSubset'>getSubset</a>() const;
    bool <a href='#SkBitmap_setInfo'>setInfo</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo, size_t <a href='#SkBitmap_rowBytes'>rowBytes</a> = 0);

    enum <a href='#SkBitmap_AllocFlags'>AllocFlags</a> {
        <a href='#SkBitmap_kZeroPixels_AllocFlag'>kZeroPixels_AllocFlag</a> = 1 << 0,
    };

    bool <a href='#SkBitmap_tryAllocPixelsFlags'>tryAllocPixelsFlags</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, uint32_t flags);
    void <a href='#SkBitmap_allocPixelsFlags'>allocPixelsFlags</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, uint32_t flags);
    bool <a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, size_t <a href='#SkBitmap_rowBytes'>rowBytes</a>);
    void <a href='#SkBitmap_allocPixels'>allocPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, size_t <a href='#SkBitmap_rowBytes'>rowBytes</a>);
    bool <a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info);
    void <a href='#SkBitmap_allocPixels'>allocPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info);
    bool <a href='#SkBitmap_tryAllocN32Pixels'>tryAllocN32Pixels</a>(int width, int height, bool <a href='#SkBitmap_isOpaque'>isOpaque</a> = false);
    void <a href='#SkBitmap_allocN32Pixels'>allocN32Pixels</a>(int width, int height, bool <a href='#SkBitmap_isOpaque'>isOpaque</a> = false);
    bool <a href='#SkBitmap_installPixels'>installPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, void* pixels, size_t <a href='#SkBitmap_rowBytes'>rowBytes</a>,
                       void (*releaseProc)(void* addr, void* context), void* context);
    bool <a href='#SkBitmap_installPixels'>installPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, void* pixels, size_t <a href='#SkBitmap_rowBytes'>rowBytes</a>);
    bool <a href='#SkBitmap_installPixels'>installPixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#Pixmap'>pixmap</a>);
    void <a href='#SkBitmap_setPixels'>setPixels</a>(void* pixels);
    bool <a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a>();
    void <a href='#SkBitmap_allocPixels'>allocPixels</a>();
    bool <a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a>(<a href='#SkBitmap_Allocator'>Allocator</a>* allocator);
    void <a href='#SkBitmap_allocPixels'>allocPixels</a>(<a href='#SkBitmap_Allocator'>Allocator</a>* allocator);
    <a href='undocumented#SkPixelRef'>SkPixelRef</a>* <a href='#SkBitmap_pixelRef'>pixelRef</a>() const;
    <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='#SkBitmap_pixelRefOrigin'>pixelRefOrigin</a>() const;
    void <a href='#SkBitmap_setPixelRef'>setPixelRef</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkPixelRef'>SkPixelRef</a>> <a href='#SkBitmap_pixelRef'>pixelRef</a>, int dx, int dy);
    bool <a href='#SkBitmap_readyToDraw'>readyToDraw</a>() const;
    uint32_t <a href='#SkBitmap_getGenerationID'>getGenerationID</a>() const;
    void <a href='#SkBitmap_notifyPixelsChanged'>notifyPixelsChanged</a>() const;
    void <a href='#SkBitmap_eraseColor'>eraseColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> c) const;
    void <a href='#SkBitmap_eraseARGB'>eraseARGB</a>(<a href='undocumented#U8CPU'>U8CPU</a> a, <a href='undocumented#U8CPU'>U8CPU</a> r, <a href='undocumented#U8CPU'>U8CPU</a> g, <a href='undocumented#U8CPU'>U8CPU</a> b) const;
    void <a href='#SkBitmap_erase'>erase</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> c, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& area) const;
    <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkBitmap_getColor'>getColor</a>(int x, int y) const;
    float <a href='#SkBitmap_getAlphaf'>getAlphaf</a>(int x, int y) const;
    void* <a href='#SkBitmap_getAddr'>getAddr</a>(int x, int y) const;
    uint32_t* <a href='#SkBitmap_getAddr32'>getAddr32</a>(int x, int y) const;
    uint16_t* <a href='#SkBitmap_getAddr16'>getAddr16</a>(int x, int y) const;
    uint8_t* <a href='#SkBitmap_getAddr8'>getAddr8</a>(int x, int y) const;
    bool <a href='#SkBitmap_extractSubset'>extractSubset</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* dst, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& subset) const;
    bool <a href='#SkBitmap_readPixels'>readPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& dstInfo, void* dstPixels, size_t dstRowBytes,
                    int srcX, int srcY) const;
    bool <a href='#SkBitmap_readPixels'>readPixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& dst, int srcX, int srcY) const;
    bool <a href='#SkBitmap_readPixels'>readPixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& dst) const;
    bool <a href='#SkBitmap_writePixels'>writePixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& src, int dstX, int dstY);
    bool <a href='#SkBitmap_writePixels'>writePixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& src);
    bool <a href='#SkBitmap_extractAlpha'>extractAlpha</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* dst) const;
    bool <a href='#SkBitmap_extractAlpha'>extractAlpha</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* dst, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>,
                      <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>* offset) const;
    bool <a href='#SkBitmap_extractAlpha'>extractAlpha</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* dst, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='#SkBitmap_Allocator'>Allocator</a>* allocator,
                      <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>* offset) const;
    bool <a href='#SkBitmap_peekPixels'>peekPixels</a>(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>* <a href='SkPixmap_Reference#Pixmap'>pixmap</a>) const;
    void <a href='#SkBitmap_validate'>validate()</a> const;
};

</pre>

<a href='SkBitmap_Reference#Bitmap'>Bitmap</a> describes a two-dimensional raster <a href='undocumented#Pixel'>pixel</a> array. <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> is built on
<a href='#Image_Info'>Image_Info</a>, containing integer width and height, <a href='#Image_Info_Color_Type'>Color_Type</a> and <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>
describing the <a href='undocumented#Pixel'>pixel</a> format, and <a href='#Color_Space'>Color_Space</a> describing the range of colors.
<a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='SkPoint_Reference#Point'>points</a> to <a href='#Pixel_Ref'>Pixel_Ref</a>, which describes the physical array of pixels.
<a href='#Image_Info'>Image_Info</a> bounds may be located anywhere fully inside <a href='#Pixel_Ref'>Pixel_Ref</a> bounds.

<a href='SkBitmap_Reference#Bitmap'>Bitmap</a> can be drawn using <a href='SkCanvas_Reference#Canvas'>Canvas</a>. <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> can be a drawing destination for <a href='SkCanvas_Reference#Canvas'>Canvas</a>
draw member functions. <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> flexibility as a <a href='undocumented#Pixel'>pixel</a> container limits some
optimizations available to the target platform.

If <a href='undocumented#Pixel'>pixel</a> array is primarily read-only, use <a href='SkImage_Reference#Image'>Image</a> for better performance.
If <a href='undocumented#Pixel'>pixel</a> array is primarily written to, use <a href='SkSurface_Reference#Surface'>Surface</a> for better performance.

Declaring <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> const prevents altering <a href='#Image_Info'>Image_Info</a>: the <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> height, width,
and so on cannot change. It does not affect <a href='#Pixel_Ref'>Pixel_Ref</a>: a caller may write its
pixels. Declaring <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> const affects <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> configuration, not its contents.

<a href='SkBitmap_Reference#Bitmap'>Bitmap</a> is not thread safe. Each thread must have its own copy of <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> fields,
although threads may share the underlying <a href='undocumented#Pixel'>pixel</a> array.

<a name='Row_Bytes'></a>

<a href='SkBitmap_Reference#Bitmap'>Bitmap</a> pixels may be contiguous, or may have a gap at the end of each row.
<a href='#Bitmap_Row_Bytes'>Row_Bytes</a> is the interval from one row to the next. <a href='#Bitmap_Row_Bytes'>Row_Bytes</a> may be specified;
sometimes passing zero will compute the <a href='#Bitmap_Row_Bytes'>Row_Bytes</a> from the row width and the
number of bytes in a <a href='undocumented#Pixel'>pixel</a>. <a href='#Bitmap_Row_Bytes'>Row_Bytes</a> may be larger than the row requires. This
is useful to position one or more <a href='SkBitmap_Reference#Bitmap'>Bitmaps</a> within a shared <a href='undocumented#Pixel'>pixel</a> array.

<a name='SkBitmap_Allocator'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    class <a href='#SkBitmap_Allocator'>Allocator</a> : public <a href='undocumented#SkRefCnt'>SkRefCnt</a> {

        virtual bool <a href='#SkBitmap_Allocator_allocPixelRef'>allocPixelRef</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* <a href='SkBitmap_Reference#Bitmap'>bitmap</a>) = 0;
    };

</pre>

Abstract subclass of <a href='#SkBitmap_HeapAllocator'>HeapAllocator</a>.

<a name='SkBitmap_Allocator_allocPixelRef'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual bool <a href='#SkBitmap_Allocator_allocPixelRef'>allocPixelRef</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* <a href='SkBitmap_Reference#Bitmap'>bitmap</a>) = 0
</pre>

Allocates the <a href='undocumented#Pixel'>pixel</a> memory for the <a href='#SkBitmap_Allocator_allocPixelRef_bitmap'>bitmap</a>, given its dimensions and
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>. Returns true on success, where success means either <a href='#SkBitmap_setPixels'>setPixels</a>()
or <a href='#SkBitmap_setPixelRef'>setPixelRef</a>() was called.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_Allocator_allocPixelRef_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> containing <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> as input, and <a href='undocumented#SkPixelRef'>SkPixelRef</a> as output</td>
  </tr>
</table>

### Return Value

true if <a href='undocumented#SkPixelRef'>SkPixelRef</a> was allocated

### See Also

<a href='#SkBitmap_HeapAllocator'>HeapAllocator</a>

<a name='SkBitmap_HeapAllocator'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    class <a href='#SkBitmap_HeapAllocator'>HeapAllocator</a> : public <a href='#SkBitmap_Allocator'>Allocator</a> {

        bool <a href='#SkBitmap_HeapAllocator_allocPixelRef'>allocPixelRef</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* <a href='SkBitmap_Reference#Bitmap'>bitmap</a>) override;
    };

</pre>

Subclass of <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_Allocator'>Allocator</a> that returns a <a href='#Pixel_Ref'>Pixel_Ref</a> that allocates its <a href='undocumented#Pixel'>pixel</a>
memory from the heap. This is the default <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_Allocator'>Allocator</a> invoked by
<a href='#SkBitmap_allocPixels'>allocPixels</a>.

<a name='SkBitmap_HeapAllocator_allocPixelRef'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_HeapAllocator_allocPixelRef'>allocPixelRef</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* <a href='SkBitmap_Reference#Bitmap'>bitmap</a>) override
</pre>

Allocates the <a href='undocumented#Pixel'>pixel</a> memory for the <a href='#SkBitmap_HeapAllocator_allocPixelRef_bitmap'>bitmap</a>, given its dimensions and
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>. Returns true on success, where success means either <a href='#SkBitmap_setPixels'>setPixels</a>()
or <a href='#SkBitmap_setPixelRef'>setPixelRef</a>() was called.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_HeapAllocator_allocPixelRef_bitmap'><code><strong>bitmap</strong></code></a></td>
    <td><a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> containing <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> as input, and <a href='undocumented#SkPixelRef'>SkPixelRef</a> as output</td>
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

Creates an empty <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> without pixels, with <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>,
<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, and with a width and height of zero. <a href='undocumented#SkPixelRef'>SkPixelRef</a> origin is
set to (0, 0). <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> is not volatile.

Use <a href='#SkBitmap_setInfo'>setInfo</a>() to associate <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, width, and height
after <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> has been created.

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
<a href='#SkBitmap_copy_const_SkBitmap'>SkBitmap</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& src)
</pre>

Copies settings from <a href='#SkBitmap_copy_const_SkBitmap_src'>src</a> to returned <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>. Shares pixels if <a href='#SkBitmap_copy_const_SkBitmap_src'>src</a> has pixels
allocated, so both <a href='SkBitmap_Reference#Bitmap'>bitmaps</a> reference the same pixels.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_copy_const_SkBitmap_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> to copy <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, and share <a href='undocumented#SkPixelRef'>SkPixelRef</a></td>
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
<a href='#SkBitmap_move_SkBitmap'>SkBitmap</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>&& src)
</pre>

Copies settings from <a href='#SkBitmap_move_SkBitmap_src'>src</a> to returned <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>. Moves ownership of <a href='#SkBitmap_move_SkBitmap_src'>src</a> pixels to
<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_move_SkBitmap_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> to copy <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, and reassign <a href='undocumented#SkPixelRef'>SkPixelRef</a></td>
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
<a href='#SkBitmap_destructor'>~SkBitmap()</a>
</pre>

Decrements <a href='undocumented#SkPixelRef'>SkPixelRef</a> reference count, if <a href='undocumented#SkPixelRef'>SkPixelRef</a> is not nullptr.

### See Also

<a href='#Pixel_Ref'>Pixel_Ref</a>

<a name='SkBitmap_copy_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='#SkBitmap_copy_operator'>operator=</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& src)
</pre>

Copies settings from <a href='#SkBitmap_copy_operator_src'>src</a> to returned <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>. Shares pixels if <a href='#SkBitmap_copy_operator_src'>src</a> has pixels
allocated, so both <a href='SkBitmap_Reference#Bitmap'>bitmaps</a> reference the same pixels.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_copy_operator_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> to copy <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, and share <a href='undocumented#SkPixelRef'>SkPixelRef</a></td>
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
<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& <a href='#SkBitmap_move_operator'>operator=</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>&& src)
</pre>

Copies settings from <a href='#SkBitmap_move_operator_src'>src</a> to returned <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>. Moves ownership of <a href='#SkBitmap_move_operator_src'>src</a> pixels to
<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_move_operator_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> to copy <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, and reassign <a href='undocumented#SkPixelRef'>SkPixelRef</a></td>
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
void <a href='#SkBitmap_swap'>swap</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& other)
</pre>

Swaps the fields of the two <a href='SkBitmap_Reference#Bitmap'>bitmaps</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_swap_other'><code><strong>other</strong></code></a></td>
    <td><a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> exchanged with original</td>
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

<a href='#SkBitmap_move_SkBitmap'>SkBitmap</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>&& src) <a href='#SkBitmap_move_operator'>operator=</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>&& src)

<a name='Property'></a>

<a name='SkBitmap_pixmap'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='#SkBitmap_pixmap'>pixmap()</a>const
</pre>

Returns a constant reference to the <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> holding the <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='undocumented#Pixel'>pixel</a>
address, row bytes, and <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>.

### Return Value

reference to <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a> describing this <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>

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
const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& <a href='#SkBitmap_info'>info()</a>const
</pre>

Returns width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, and <a href='undocumented#SkColorSpace'>SkColorSpace</a>.

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
int <a href='#SkBitmap_width'>width()</a>const
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> count in each row. Should be equal or less than
<code><a href='#SkBitmap_rowBytes'>rowBytes</a>() / <a href='#SkBitmap_info'>info()</a>.<a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>()</code>.

May be less than <a href='#SkBitmap_pixelRef'>pixelRef</a>().<a href='#SkPixelRef_width'>width()</a>. Will not exceed <a href='#SkBitmap_pixelRef'>pixelRef</a>().<a href='#SkPixelRef_width'>width()</a> less
<a href='#SkBitmap_pixelRefOrigin'>pixelRefOrigin</a>().<a href='#SkIPoint_fX'>fX</a>.

### Return Value

<a href='undocumented#Pixel'>pixel</a> width in <a href='#Image_Info'>Image_Info</a>

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
int <a href='#SkBitmap_height'>height()</a>const
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> row count.

Maybe be less than <a href='#SkBitmap_pixelRef'>pixelRef</a>().<a href='#SkPixelRef_height'>height()</a>. Will not exceed <a href='#SkBitmap_pixelRef'>pixelRef</a>().<a href='#SkPixelRef_height'>height()</a> less
<a href='#SkBitmap_pixelRefOrigin'>pixelRefOrigin</a>().<a href='#SkIPoint_fY'>fY</a>.

### Return Value

<a href='undocumented#Pixel'>pixel</a> height in <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>

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
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> <a href='#SkBitmap_colorType'>colorType</a>()const
</pre>

Returns <a href='#Image_Info_Color_Type'>Color_Type</a>, one of: <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>,
<a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_888x_SkColorType'>kRGB_888x_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_1010102_SkColorType'>kRGBA_1010102_SkColorType</a>, <a href='SkImageInfo_Reference#kRGB_101010x_SkColorType'>kRGB_101010x_SkColorType</a>,
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>
.

### Return Value

<a href='#Image_Info_Color_Type'>Color_Type</a> in <a href='#Image_Info'>Image_Info</a>

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
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> <a href='#SkBitmap_alphaType'>alphaType</a>()const
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

<a href='#SkBitmap_colorType'>colorType</a>() <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_alphaType'>alphaType</a>

<a name='SkBitmap_colorSpace'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkColorSpace'>SkColorSpace</a>* <a href='#SkBitmap_colorSpace'>colorSpace</a>()const
</pre>

Returns <a href='undocumented#SkColorSpace'>SkColorSpace</a>, the range of colors, associated with <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>. The
reference count of <a href='undocumented#SkColorSpace'>SkColorSpace</a> is unchanged. The returned <a href='undocumented#SkColorSpace'>SkColorSpace</a> is
immutable.

### Return Value

<a href='undocumented#SkColorSpace'>SkColorSpace</a> in <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, or nullptr

### Example

<div><fiddle-embed name="817f95879fadba44baf87ea60e9b595a"><div><a href='undocumented#SkColorSpace'>SkColorSpace</a>::<a href='#SkColorSpace_MakeSRGBLinear'>MakeSRGBLinear</a> creates <a href='#Color_Space'>Color_Space</a> with linear gamma
and an sRGB gamut. This <a href='#Color_Space'>Color_Space</a> gamma is not close to sRGB gamma.
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
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkColorSpace'>SkColorSpace</a>&gt; <a href='#SkBitmap_refColorSpace'>refColorSpace</a>()const
</pre>

Returns smart pointer to <a href='undocumented#SkColorSpace'>SkColorSpace</a>, the range of colors, associated with
<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>. The smart pointer tracks the number of objects sharing this
<a href='undocumented#SkColorSpace'>SkColorSpace</a> reference so the memory is released when the owners destruct.

The returned <a href='undocumented#SkColorSpace'>SkColorSpace</a> is immutable.

### Return Value

<a href='undocumented#SkColorSpace'>SkColorSpace</a> in <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> wrapped in a smart pointer

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
int <a href='#SkBitmap_bytesPerPixel'>bytesPerPixel</a>()const
</pre>

Returns number of bytes per <a href='undocumented#Pixel'>pixel</a> required by <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.
Returns zero if <a href='#SkBitmap_colorType'>colorType</a>( is <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>.

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
int <a href='#SkBitmap_rowBytesAsPixels'>rowBytesAsPixels</a>()const
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
int <a href='#SkBitmap_shiftPerPixel'>shiftPerPixel</a>()const
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
bool <a href='#SkBitmap_empty'>empty()</a>const
</pre>

Returns true if either <a href='#SkBitmap_width'>width()</a> or <a href='#SkBitmap_height'>height()</a> are zero.

Does not check if <a href='undocumented#SkPixelRef'>SkPixelRef</a> is nullptr; call <a href='#SkBitmap_drawsNothing'>drawsNothing</a>() to check <a href='#SkBitmap_width'>width()</a>,
<a href='#SkBitmap_height'>height()</a>, and <a href='undocumented#SkPixelRef'>SkPixelRef</a>.

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
bool <a href='#SkBitmap_isNull'>isNull</a>()const
</pre>

Returns true if <a href='undocumented#SkPixelRef'>SkPixelRef</a> is nullptr.

Does not check if <a href='#SkBitmap_width'>width()</a> or <a href='#SkBitmap_height'>height()</a> are zero; call <a href='#SkBitmap_drawsNothing'>drawsNothing</a>() to check
<a href='#SkBitmap_width'>width()</a>, <a href='#SkBitmap_height'>height()</a>, and <a href='undocumented#SkPixelRef'>SkPixelRef</a>.

### Return Value

true if no <a href='undocumented#SkPixelRef'>SkPixelRef</a> is associated

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
bool <a href='#SkBitmap_drawsNothing'>drawsNothing</a>()const
</pre>

Returns true if <a href='#SkBitmap_width'>width()</a> or <a href='#SkBitmap_height'>height()</a> are zero, or if <a href='undocumented#SkPixelRef'>SkPixelRef</a> is nullptr.
If true, <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> has no effect when drawn or drawn into.

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
size_t <a href='#SkBitmap_rowBytes'>rowBytes</a>()const
</pre>

Returns row bytes, the interval from one <a href='undocumented#Pixel'>pixel</a> row to the next. Row bytes
is at least as large as: <code><a href='#SkBitmap_width'>width()</a> * <a href='#SkBitmap_info'>info()</a>.<a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>()</code>.

Returns zero if <a href='#SkBitmap_colorType'>colorType</a> is <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, or if row bytes supplied to
<a href='#SkBitmap_setInfo'>setInfo</a> is not large enough to hold a row of pixels.

### Return Value

byte length of <a href='undocumented#Pixel'>pixel</a> row

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

Sets <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, if <a href='#SkBitmap_setAlphaType_alphaType'>alphaType</a> is compatible with <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.
Returns true unless <a href='#SkBitmap_setAlphaType_alphaType'>alphaType</a> is <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a> and current <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>
is not <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>.

Returns true if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>. <a href='#SkBitmap_setAlphaType_alphaType'>alphaType</a> is ignored, and
<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> remains <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>.

Returns true if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a> or <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>.
<a href='#SkBitmap_setAlphaType_alphaType'>alphaType</a> is ignored, and <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> remains <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>.

If <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, or <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>: returns true unless
<a href='#SkBitmap_setAlphaType_alphaType'>alphaType</a> is <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a> and <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> is not <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>.
If <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> is <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='#SkBitmap_setAlphaType_alphaType'>alphaType</a> is ignored.

If <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>, returns true unless
<a href='#SkBitmap_setAlphaType_alphaType'>alphaType</a> is <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a> and <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> is not <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>.
If <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> is <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='#SkBitmap_setAlphaType_alphaType'>alphaType</a> is ignored. If <a href='#SkBitmap_setAlphaType_alphaType'>alphaType</a> is
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>, it is treated as <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>.

This changes <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> in <a href='undocumented#SkPixelRef'>SkPixelRef</a>; all <a href='SkBitmap_Reference#Bitmap'>bitmaps</a> sharing <a href='undocumented#SkPixelRef'>SkPixelRef</a>
are affected.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_setAlphaType_alphaType'><code><strong>alphaType</strong></code></a></td>
    <td>one of:</td>
  </tr>
</table>

<a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>, <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>,
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>

### Return Value

true if <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> is set

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
void* <a href='#SkBitmap_getPixels'>getPixels</a>()const
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> address, the base address corresponding to the <a href='undocumented#Pixel'>pixel</a> origin.

### Return Value

<a href='undocumented#Pixel'>pixel</a> address

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
size_t <a href='#SkBitmap_computeByteSize'>computeByteSize</a>()const
</pre>

Returns minimum memory required for <a href='undocumented#Pixel'>pixel</a> storage.
Does not include unused memory on last row when <a href='#SkBitmap_rowBytesAsPixels'>rowBytesAsPixels</a>() exceeds <a href='#SkBitmap_width'>width()</a>.
Returns zero if result does not fit in size_t.
Returns zero if <a href='#SkBitmap_height'>height()</a> or <a href='#SkBitmap_width'>width()</a> is 0.
Returns <a href='#SkBitmap_height'>height()</a> times <a href='#SkBitmap_rowBytes'>rowBytes</a>() if <a href='#SkBitmap_colorType'>colorType</a>() is <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>.

### Return Value

<a href='undocumented#Size'>size</a> in bytes of <a href='SkImage_Reference#Image'>image</a> buffer

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
bool <a href='#SkBitmap_isImmutable'>isImmutable</a>()const
</pre>

Returns true if pixels can not change.

Most immutable <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> checks trigger an assert only on debug builds.

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

Sets internal flag to mark <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> as immutable. Once set, pixels can not change.
Any other <a href='SkBitmap_Reference#Bitmap'>bitmap</a> sharing the same <a href='undocumented#SkPixelRef'>SkPixelRef</a> are also marked as immutable.
Once <a href='undocumented#SkPixelRef'>SkPixelRef</a> is marked immutable, the setting cannot be cleared.

Writing to immutable <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> pixels triggers an assert on debug builds.

### Example

<div><fiddle-embed name="9210060d1f4ca46e1375496237902ef3"><div>Triggers assert if SK_DEBUG is true, runs fine otherwise.
</div></fiddle-embed></div>

### See Also

<a href='#SkBitmap_isImmutable'>isImmutable</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>::<a href='#SkPixelRef_setImmutable'>setImmutable</a> <a href='SkImage_Reference#SkImage'>SkImage</a>

<a name='SkBitmap_isOpaque'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_isOpaque'>isOpaque</a>()const
</pre>

Returns true if <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> is set to hint that all pixels are opaque; their
<a href='SkColor_Reference#Alpha'>alpha</a> value is implicitly or explicitly 1.0. If true, and all pixels are
not opaque, Skia may draw incorrectly.

Does not check if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> allows <a href='SkColor_Reference#Alpha'>alpha</a>, or if any <a href='undocumented#Pixel'>pixel</a> value has
transparency.

### Return Value

true if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> is <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>

### Example

<div><fiddle-embed name="5e76b68bb46d54315eb0c12d83bd6949"><div><a href='#SkBitmap_isOpaque'>isOpaque</a> ignores whether all pixels are opaque or not.
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
bool <a href='#SkBitmap_isVolatile'>isVolatile</a>()const
</pre>

Provides a hint to caller that pixels should not be cached. Only true if
<a href='#SkBitmap_setIsVolatile'>setIsVolatile</a>() has been called to mark as volatile.

Volatile state is not shared by other <a href='SkBitmap_Reference#Bitmap'>bitmaps</a> sharing the same <a href='undocumented#SkPixelRef'>SkPixelRef</a>.

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
void <a href='#SkBitmap_setIsVolatile'>setIsVolatile</a>(bool <a href='#SkBitmap_isVolatile'>isVolatile</a>)
</pre>

Sets if pixels should be read from <a href='undocumented#SkPixelRef'>SkPixelRef</a> on every access. <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> are not
volatile by default; a GPU back end may upload <a href='undocumented#Pixel'>pixel</a> values expecting them to be
accessed repeatedly. Marking temporary <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> as volatile provides a hint to
<a href='undocumented#SkBaseDevice'>SkBaseDevice</a> that the <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> pixels should not be cached. This can
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

Resets to its initial state; all fields are set to zero, as if <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> had
been initialized by <a href='#SkBitmap_empty_constructor'>SkBitmap()</a>.

Sets width, height, row bytes to zero; <a href='undocumented#Pixel'>pixel</a> address to nullptr; <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> to
<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>; and <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> to <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>.

If <a href='undocumented#SkPixelRef'>SkPixelRef</a> is allocated, its reference count is decreased by one, releasing
its memory if <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> is the sole owner.

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
static bool <a href='#SkBitmap_ComputeIsOpaque'>ComputeIsOpaque</a>(const <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>& bm)
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

### Parameters

<table>  <tr>    <td><a name='SkBitmap_ComputeIsOpaque_bm'><code><strong>bm</strong></code></a></td>
    <td><a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> to check</td>
  </tr>
</table>

### Return Value

true if all pixels have opaque values or <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is opaque

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
void <a href='#SkBitmap_getBounds'>getBounds</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* bounds)const
</pre>

Returns <a href='SkRect_Reference#SkRect'>SkRect</a> { 0, 0, <a href='#SkBitmap_width'>width()</a>, <a href='#SkBitmap_height'>height()</a> }.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_getBounds_bounds'><code><strong>bounds</strong></code></a></td>
    <td>container for floating <a href='SkPoint_Reference#Point'>point</a> rectangle</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="2431ebc7e7d1e91e6d9daafd0f7a478f"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_bounds'>bounds()</a>

<a name='SkBitmap_getBounds_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_getBounds'>getBounds</a>(<a href='SkIRect_Reference#SkIRect'>SkIRect</a>* bounds)const
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
<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkBitmap_bounds'>bounds()</a>const
</pre>

Returns <a href='SkIRect_Reference#SkIRect'>SkIRect</a> { 0, 0, <a href='#SkBitmap_width'>width()</a>, <a href='#SkBitmap_height'>height()</a> }.

### Return Value

integral rectangle from origin to <a href='#SkBitmap_width'>width()</a> and <a href='#SkBitmap_height'>height()</a>

### Example

<div><fiddle-embed name="3e9126152ff1cc592aef6facbcb5fc96"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_getBounds'>getBounds</a>

<a name='SkBitmap_dimensions'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkISize'>SkISize</a> <a href='#SkBitmap_dimensions'>dimensions()</a>const
</pre>

Returns <a href='undocumented#SkISize'>SkISize</a> { <a href='#SkBitmap_width'>width()</a>, <a href='#SkBitmap_height'>height()</a> }.

### Return Value

integral <a href='undocumented#Size'>size</a> of <a href='#SkBitmap_width'>width()</a> and <a href='#SkBitmap_height'>height()</a>

### Example

<div><fiddle-embed name="647056bcc12c27fb4413f212f33a2898"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_height'>height</a> <a href='#SkBitmap_width'>width</a>

<a name='SkBitmap_getSubset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkBitmap_getSubset'>getSubset</a>()const
</pre>

Returns the bounds of this <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, offset by its <a href='undocumented#SkPixelRef'>SkPixelRef</a> origin.

### Return Value

bounds within <a href='undocumented#SkPixelRef'>SkPixelRef</a> bounds

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
bool <a href='#SkBitmap_setInfo'>setInfo</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& imageInfo, size_t <a href='#SkBitmap_rowBytes'>rowBytes</a> = 0)
</pre>

Sets width, height, <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Color_Space'>Color_Space</a>, and optional
<a href='#SkBitmap_setInfo_rowBytes'>rowBytes</a>. Frees pixels, and returns true if successful.

<a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() may be altered to a value permitted by <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_colorSpace'>colorSpace</a>().
If <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_colorType'>colorType</a>() is <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() is
set to <a href='SkImageInfo_Reference#kUnknown_SkAlphaType'>kUnknown_SkAlphaType</a>.
If <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_colorType'>colorType</a>() is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a> and <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() is
<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>, <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() is replaced by <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>.
If <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_colorType'>colorType</a>() is <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a> or <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>,
<a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() is set to <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>.
If <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_colorType'>colorType</a>() is <a href='SkImageInfo_Reference#kARGB_4444_SkColorType'>kARGB_4444_SkColorType</a>, <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a>,
<a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a>, or <a href='SkImageInfo_Reference#kRGBA_F16_SkColorType'>kRGBA_F16_SkColorType</a>: <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() remains
unchanged.

<a href='#SkBitmap_setInfo_rowBytes'>rowBytes</a> must equal or exceed <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>(). If <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_colorSpace'>colorSpace</a>() is
<a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, <a href='#SkBitmap_setInfo_rowBytes'>rowBytes</a> is ignored and treated as zero; for all other
<a href='#Color_Space'>Color_Space</a> values, <a href='#SkBitmap_setInfo_rowBytes'>rowBytes</a> of zero is treated as <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>().

Calls <a href='#SkBitmap_reset'>reset()</a> and returns false if:

<table>  <tr>
    <td><a href='#SkBitmap_setInfo_rowBytes'>rowBytes</a> exceeds 31 bits</td>
  </tr>  <tr>
    <td><a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_width'>width()</a> is negative</td>
  </tr>  <tr>
    <td><a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_height'>height()</a> is negative</td>
  </tr>  <tr>
    <td><a href='#SkBitmap_setInfo_rowBytes'>rowBytes</a> is positive and less than <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_width'>width()</a> times <a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>()</td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkBitmap_setInfo_imageInfo'><code><strong>imageInfo</strong></code></a></td>
    <td>contains width, height, <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Color_Space'>Color_Space</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_setInfo_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td><a href='#SkBitmap_setInfo_imageInfo'>imageInfo</a>.<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>() or larger; or zero</td>
  </tr>
</table>

### Return Value

true if <a href='#Image_Info'>Image_Info</a> set successfully

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

<a href='#SkBitmap_AllocFlags'>AllocFlags</a> provides the option to zero <a href='undocumented#Pixel'>pixel</a> memory when allocated.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkBitmap_kZeroPixels_AllocFlag'><code>SkBitmap::kZeroPixels_AllocFlag</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Instructs <a href='#SkBitmap_tryAllocPixelsFlags'>tryAllocPixelsFlags</a> and <a href='#SkBitmap_allocPixelsFlags'>allocPixelsFlags</a> to zero <a href='undocumented#Pixel'>pixel</a> memory.
</td>
  </tr>
</table>

### See Also

<a href='#SkBitmap_tryAllocPixelsFlags'>tryAllocPixelsFlags</a> <a href='#SkBitmap_allocPixelsFlags'>allocPixelsFlags</a> <a href='#SkBitmap_erase'>erase</a> <a href='#SkBitmap_eraseColor'>eraseColor</a>

<a name='Allocate'></a>

<a name='SkBitmap_tryAllocPixelsFlags'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_tryAllocPixelsFlags'>tryAllocPixelsFlags</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, uint32_t flags)
</pre>

Sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> to <a href='#SkBitmap_tryAllocPixelsFlags_info'>info</a> following the rules in <a href='#SkBitmap_setInfo'>setInfo</a>() and allocates <a href='undocumented#Pixel'>pixel</a>
memory. If <a href='#SkBitmap_tryAllocPixelsFlags_flags'>flags</a> is <a href='#SkBitmap_kZeroPixels_AllocFlag'>kZeroPixels_AllocFlag</a>, memory is zeroed.

Returns false and calls <a href='#SkBitmap_reset'>reset()</a> if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> could not be set, or memory could
not be allocated, or memory could not optionally be zeroed.

On most platforms, allocating <a href='undocumented#Pixel'>pixel</a> memory may succeed even though there is
not sufficient memory to hold pixels; allocation does not take place
until the pixels are written to. The actual behavior depends on the platform
implementation of malloc(), if <a href='#SkBitmap_tryAllocPixelsFlags_flags'>flags</a> is zero, and calloc(), if <a href='#SkBitmap_tryAllocPixelsFlags_flags'>flags</a> is
<a href='#SkBitmap_kZeroPixels_AllocFlag'>kZeroPixels_AllocFlag</a>.

<a href='#SkBitmap_tryAllocPixelsFlags_flags'>flags</a> set to <a href='#SkBitmap_kZeroPixels_AllocFlag'>kZeroPixels_AllocFlag</a> offers equal or better performance than
subsequently calling <a href='#SkBitmap_eraseColor'>eraseColor</a>() with <a href='SkColor_Reference#SK_ColorTRANSPARENT'>SK_ColorTRANSPARENT</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_tryAllocPixelsFlags_info'><code><strong>info</strong></code></a></td>
    <td>contains width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_tryAllocPixelsFlags_flags'><code><strong>flags</strong></code></a></td>
    <td><a href='#SkBitmap_kZeroPixels_AllocFlag'>kZeroPixels_AllocFlag</a>, or zero</td>
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
void <a href='#SkBitmap_allocPixelsFlags'>allocPixelsFlags</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, uint32_t flags)
</pre>

Sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> to <a href='#SkBitmap_allocPixelsFlags_info'>info</a> following the rules in <a href='#SkBitmap_setInfo'>setInfo</a>() and allocates <a href='undocumented#Pixel'>pixel</a>
memory. If <a href='#SkBitmap_allocPixelsFlags_flags'>flags</a> is <a href='#SkBitmap_kZeroPixels_AllocFlag'>kZeroPixels_AllocFlag</a>, memory is zeroed.

Aborts execution if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> could not be set, or memory could
not be allocated, or memory could not optionally
be zeroed. Abort steps may be provided by the user at compile time by defining
SK_ABORT.

On most platforms, allocating <a href='undocumented#Pixel'>pixel</a> memory may succeed even though there is
not sufficient memory to hold pixels; allocation does not take place
until the pixels are written to. The actual behavior depends on the platform
implementation of malloc(), if <a href='#SkBitmap_allocPixelsFlags_flags'>flags</a> is zero, and calloc(), if <a href='#SkBitmap_allocPixelsFlags_flags'>flags</a> is
<a href='#SkBitmap_kZeroPixels_AllocFlag'>kZeroPixels_AllocFlag</a>.

<a href='#SkBitmap_allocPixelsFlags_flags'>flags</a> set to <a href='#SkBitmap_kZeroPixels_AllocFlag'>kZeroPixels_AllocFlag</a> offers equal or better performance than
subsequently calling <a href='#SkBitmap_eraseColor'>eraseColor</a>() with <a href='SkColor_Reference#SK_ColorTRANSPARENT'>SK_ColorTRANSPARENT</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_allocPixelsFlags_info'><code><strong>info</strong></code></a></td>
    <td>contains width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_allocPixelsFlags_flags'><code><strong>flags</strong></code></a></td>
    <td><a href='#SkBitmap_kZeroPixels_AllocFlag'>kZeroPixels_AllocFlag</a>, or zero</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="737e721c7d9e0f367d25521a1b823b9d"><div><a href='undocumented#Text'>Text</a> is drawn on a transparent background; drawing the <a href='SkBitmap_Reference#Bitmap'>bitmap</a> a second time
lets the first draw show through.
</div></fiddle-embed></div>

### See Also

<a href='#SkBitmap_tryAllocPixelsFlags'>tryAllocPixelsFlags</a> <a href='#SkBitmap_allocPixels'>allocPixels</a> <a href='undocumented#SkMallocPixelRef'>SkMallocPixelRef</a>::<a href='#SkMallocPixelRef_MakeZeroed'>MakeZeroed</a>

<a name='SkBitmap_tryAllocPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, size_t <a href='#SkBitmap_rowBytes'>rowBytes</a>)
</pre>

Sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> to <a href='#SkBitmap_tryAllocPixels_info'>info</a> following the rules in <a href='#SkBitmap_setInfo'>setInfo</a>() and allocates <a href='undocumented#Pixel'>pixel</a>
memory. <a href='#SkBitmap_tryAllocPixels_rowBytes'>rowBytes</a> must equal or exceed <a href='#SkBitmap_tryAllocPixels_info'>info</a>.<a href='#SkImageInfo_width'>width()</a> times <a href='#SkBitmap_tryAllocPixels_info'>info</a>.<a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>(),
or equal zero. Pass in zero for <a href='#SkBitmap_tryAllocPixels_rowBytes'>rowBytes</a> to compute the minimum valid value.

Returns false and calls <a href='#SkBitmap_reset'>reset()</a> if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> could not be set, or memory could
not be allocated.

On most platforms, allocating <a href='undocumented#Pixel'>pixel</a> memory may succeed even though there is
not sufficient memory to hold pixels; allocation does not take place
until the pixels are written to. The actual behavior depends on the platform
implementation of malloc().

### Parameters

<table>  <tr>    <td><a name='SkBitmap_tryAllocPixels_info'><code><strong>info</strong></code></a></td>
    <td>contains width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_tryAllocPixels_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> of <a href='undocumented#Pixel'>pixel</a> row or larger; may be zero</td>
  </tr>
</table>

### Return Value

true if  <a href='undocumented#Pixel_Storage'>pixel storage</a> is allocated

### Example

<div><fiddle-embed name="34479d5aa23ce9f5e334b0786c9edb22"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_tryAllocPixelsFlags'>tryAllocPixelsFlags</a> <a href='#SkBitmap_allocPixels'>allocPixels</a> <a href='undocumented#SkMallocPixelRef'>SkMallocPixelRef</a>::<a href='#SkMallocPixelRef_MakeAllocate'>MakeAllocate</a>

<a name='SkBitmap_allocPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_allocPixels'>allocPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, size_t <a href='#SkBitmap_rowBytes'>rowBytes</a>)
</pre>

Sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> to <a href='#SkBitmap_allocPixels_info'>info</a> following the rules in <a href='#SkBitmap_setInfo'>setInfo</a>() and allocates <a href='undocumented#Pixel'>pixel</a>
memory. <a href='#SkBitmap_allocPixels_rowBytes'>rowBytes</a> must equal or exceed <a href='#SkBitmap_allocPixels_info'>info</a>.<a href='#SkImageInfo_width'>width()</a> times <a href='#SkBitmap_allocPixels_info'>info</a>.<a href='#SkImageInfo_bytesPerPixel'>bytesPerPixel</a>(),
or equal zero. Pass in zero for <a href='#SkBitmap_allocPixels_rowBytes'>rowBytes</a> to compute the minimum valid value.

Aborts execution if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> could not be set, or memory could
not be allocated. Abort steps may be provided by
the user at compile time by defining SK_ABORT.

On most platforms, allocating <a href='undocumented#Pixel'>pixel</a> memory may succeed even though there is
not sufficient memory to hold pixels; allocation does not take place
until the pixels are written to. The actual behavior depends on the platform
implementation of malloc().

### Parameters

<table>  <tr>    <td><a name='SkBitmap_allocPixels_info'><code><strong>info</strong></code></a></td>
    <td>contains width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_allocPixels_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> of <a href='undocumented#Pixel'>pixel</a> row or larger; may be zero</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="555c0f62f96602a9dcd459badcd005e0"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a> <a href='#SkBitmap_allocPixelsFlags'>allocPixelsFlags</a> <a href='undocumented#SkMallocPixelRef'>SkMallocPixelRef</a>::<a href='#SkMallocPixelRef_MakeAllocate'>MakeAllocate</a>

<a name='SkBitmap_tryAllocPixels_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info)
</pre>

Sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> to <a href='#SkBitmap_tryAllocPixels_2_info'>info</a> following the rules in <a href='#SkBitmap_setInfo'>setInfo</a>() and allocates <a href='undocumented#Pixel'>pixel</a>
memory.

Returns false and calls <a href='#SkBitmap_reset'>reset()</a> if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> could not be set, or memory could
not be allocated.

On most platforms, allocating <a href='undocumented#Pixel'>pixel</a> memory may succeed even though there is
not sufficient memory to hold pixels; allocation does not take place
until the pixels are written to. The actual behavior depends on the platform
implementation of malloc().

### Parameters

<table>  <tr>    <td><a name='SkBitmap_tryAllocPixels_2_info'><code><strong>info</strong></code></a></td>
    <td>contains width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a></td>
  </tr>
</table>

### Return Value

true if  <a href='undocumented#Pixel_Storage'>pixel storage</a> is allocated

### Example

<div><fiddle-embed name="7ef3d043c4c5885649e591dd7dca92ff"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_tryAllocPixelsFlags'>tryAllocPixelsFlags</a> <a href='#SkBitmap_allocPixels'>allocPixels</a> <a href='undocumented#SkMallocPixelRef'>SkMallocPixelRef</a>::<a href='#SkMallocPixelRef_MakeAllocate'>MakeAllocate</a>

<a name='SkBitmap_allocPixels_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_allocPixels'>allocPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info)
</pre>

Sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> to <a href='#SkBitmap_allocPixels_2_info'>info</a> following the rules in <a href='#SkBitmap_setInfo'>setInfo</a>() and allocates <a href='undocumented#Pixel'>pixel</a>
memory.

Aborts execution if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> could not be set, or memory could
not be allocated. Abort steps may be provided by
the user at compile time by defining SK_ABORT.

On most platforms, allocating <a href='undocumented#Pixel'>pixel</a> memory may succeed even though there is
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
bool <a href='#SkBitmap_tryAllocN32Pixels'>tryAllocN32Pixels</a>(int width, int height, bool <a href='#SkBitmap_isOpaque'>isOpaque</a> = false)
</pre>

Sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> to <a href='#SkBitmap_tryAllocN32Pixels_width'>width</a>, <a href='#SkBitmap_tryAllocN32Pixels_height'>height</a>, and native  <a href='SkImageInfo_Reference#Color_Type'>color type</a>; and allocates
<a href='undocumented#Pixel'>pixel</a> memory. If <a href='#SkBitmap_tryAllocN32Pixels_isOpaque'>isOpaque</a> is true, sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> to <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>;
otherwise, sets to <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>.

Calls <a href='#SkBitmap_reset'>reset()</a> and returns false if <a href='#SkBitmap_tryAllocN32Pixels_width'>width</a> exceeds 29 bits or is negative,
or <a href='#SkBitmap_tryAllocN32Pixels_height'>height</a> is negative.

Returns false if allocation fails.

Use to create <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> that matches <a href='SkColor_Reference#SkPMColor'>SkPMColor</a>, the native <a href='undocumented#Pixel'>pixel</a> arrangement on
the platform. <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> drawn to output <a href='undocumented#Device'>device</a> skips converting its <a href='undocumented#Pixel'>pixel</a> format.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_tryAllocN32Pixels_width'><code><strong>width</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> column count; must be zero or greater</td>
  </tr>
  <tr>    <td><a name='SkBitmap_tryAllocN32Pixels_height'><code><strong>height</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> row count; must be zero or greater</td>
  </tr>
  <tr>    <td><a name='SkBitmap_tryAllocN32Pixels_isOpaque'><code><strong>isOpaque</strong></code></a></td>
    <td>true if pixels do not have transparency</td>
  </tr>
</table>

### Return Value

true if  <a href='undocumented#Pixel_Storage'>pixel storage</a> is allocated

### Example

<div><fiddle-embed name="a2b1e0910f37066f15ae56368775a6d8"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a> <a href='#SkBitmap_allocN32Pixels'>allocN32Pixels</a> <a href='undocumented#SkMallocPixelRef'>SkMallocPixelRef</a>::<a href='#SkMallocPixelRef_MakeAllocate'>MakeAllocate</a>

<a name='SkBitmap_allocN32Pixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_allocN32Pixels'>allocN32Pixels</a>(int width, int height, bool <a href='#SkBitmap_isOpaque'>isOpaque</a> = false)
</pre>

Sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> to <a href='#SkBitmap_allocN32Pixels_width'>width</a>, <a href='#SkBitmap_allocN32Pixels_height'>height</a>, and the native  <a href='SkImageInfo_Reference#Color_Type'>color type</a>; and allocates
<a href='undocumented#Pixel'>pixel</a> memory. If <a href='#SkBitmap_allocN32Pixels_isOpaque'>isOpaque</a> is true, sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> to <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>;
otherwise, sets to <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>.

Aborts if <a href='#SkBitmap_allocN32Pixels_width'>width</a> exceeds 29 bits or is negative, or <a href='#SkBitmap_allocN32Pixels_height'>height</a> is negative, or
allocation fails. Abort steps may be provided by the user at compile time by
defining SK_ABORT.

Use to create <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> that matches <a href='SkColor_Reference#SkPMColor'>SkPMColor</a>, the native <a href='undocumented#Pixel'>pixel</a> arrangement on
the platform. <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> drawn to output <a href='undocumented#Device'>device</a> skips converting its <a href='undocumented#Pixel'>pixel</a> format.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_allocN32Pixels_width'><code><strong>width</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> column count; must be zero or greater</td>
  </tr>
  <tr>    <td><a name='SkBitmap_allocN32Pixels_height'><code><strong>height</strong></code></a></td>
    <td><a href='undocumented#Pixel'>pixel</a> row count; must be zero or greater</td>
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
bool <a href='#SkBitmap_installPixels'>installPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, void* pixels, size_t <a href='#SkBitmap_rowBytes'>rowBytes</a>, void (*releaseProc)
                   (void* addr, void* context) , void* context)
</pre>

Sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> to <a href='#SkBitmap_installPixels_info'>info</a> following the rules in <a href='#SkBitmap_setInfo'>setInfo</a>(), and creates <a href='undocumented#SkPixelRef'>SkPixelRef</a>
containing <a href='#SkBitmap_installPixels_pixels'>pixels</a> and <a href='#SkBitmap_installPixels_rowBytes'>rowBytes</a>. <a href='#SkBitmap_installPixels_releaseProc'>releaseProc</a>, if not nullptr, is called
immediately on failure or when <a href='#SkBitmap_installPixels_pixels'>pixels</a> are no longer referenced. <a href='#SkBitmap_installPixels_context'>context</a> may be
nullptr.

If <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> could not be set, or <a href='#SkBitmap_installPixels_rowBytes'>rowBytes</a> is less than <a href='#SkBitmap_installPixels_info'>info</a>.<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>():
calls <a href='#SkBitmap_installPixels_releaseProc'>releaseProc</a> if present, calls <a href='#SkBitmap_reset'>reset()</a>, and returns false.

Otherwise, if <a href='#SkBitmap_installPixels_pixels'>pixels</a> equals nullptr: sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, calls <a href='#SkBitmap_installPixels_releaseProc'>releaseProc</a> if
present, returns true.

If <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> is set, <a href='#SkBitmap_installPixels_pixels'>pixels</a> is not nullptr, and <a href='#SkBitmap_installPixels_releaseProc'>releaseProc</a> is not nullptr:
when <a href='#SkBitmap_installPixels_pixels'>pixels</a> are no longer referenced, calls <a href='#SkBitmap_installPixels_releaseProc'>releaseProc</a> with <a href='#SkBitmap_installPixels_pixels'>pixels</a> and <a href='#SkBitmap_installPixels_context'>context</a>
as parameters.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_installPixels_info'><code><strong>info</strong></code></a></td>
    <td>contains width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_installPixels_pixels'><code><strong>pixels</strong></code></a></td>
    <td>address or  <a href='undocumented#Pixel_Storage'>pixel storage</a>; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkBitmap_installPixels_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> of <a href='undocumented#Pixel'>pixel</a> row or larger</td>
  </tr>
  <tr>    <td><a name='SkBitmap_installPixels_releaseProc'><code><strong>releaseProc</strong></code></a></td>
    <td>function called when <a href='#SkBitmap_installPixels_pixels'>pixels</a> can be deleted; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkBitmap_installPixels_context'><code><strong>context</strong></code></a></td>
    <td>caller state passed to <a href='#SkBitmap_installPixels_releaseProc'>releaseProc</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

true if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> is set to <a href='#SkBitmap_installPixels_info'>info</a>

### Example

<div><fiddle-embed name="8c4f7bf73fffa1a812ee8e88e44e639c"><div><a href='#SkBitmap_installPixels_releaseProc'>releaseProc</a> is called immediately because <a href='#SkBitmap_installPixels_rowBytes'>rowBytes</a> is too small for <a href='#Pixel_Ref'>Pixel_Ref</a>.
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
bool <a href='#SkBitmap_installPixels'>installPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& info, void* pixels, size_t <a href='#SkBitmap_rowBytes'>rowBytes</a>)
</pre>

Sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> to <a href='#SkBitmap_installPixels_2_info'>info</a> following the rules in <a href='#SkBitmap_setInfo'>setInfo</a>(), and creates <a href='undocumented#SkPixelRef'>SkPixelRef</a>
containing <a href='#SkBitmap_installPixels_2_pixels'>pixels</a> and <a href='#SkBitmap_installPixels_2_rowBytes'>rowBytes</a>.

If <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> could not be set, or <a href='#SkBitmap_installPixels_2_rowBytes'>rowBytes</a> is less than <a href='#SkBitmap_installPixels_2_info'>info</a>.<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>():
calls <a href='#SkBitmap_reset'>reset()</a>, and returns false.

Otherwise, if <a href='#SkBitmap_installPixels_2_pixels'>pixels</a> equals nullptr: sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, returns true.

Caller must ensure that <a href='#SkBitmap_installPixels_2_pixels'>pixels</a> are valid for the lifetime of <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> and <a href='undocumented#SkPixelRef'>SkPixelRef</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_installPixels_2_info'><code><strong>info</strong></code></a></td>
    <td>contains width, height, <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a>, <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>, <a href='undocumented#SkColorSpace'>SkColorSpace</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_installPixels_2_pixels'><code><strong>pixels</strong></code></a></td>
    <td>address or  <a href='undocumented#Pixel_Storage'>pixel storage</a>; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkBitmap_installPixels_2_rowBytes'><code><strong>rowBytes</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> of <a href='undocumented#Pixel'>pixel</a> row or larger</td>
  </tr>
</table>

### Return Value

true if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> is set to <a href='#SkBitmap_installPixels_2_info'>info</a>

### Example

<div><fiddle-embed name="a7e04447b2081010c50d7920e80a6bb2"><div>GPU does not support <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>, does not assert that it does not.
</div></fiddle-embed></div>

### See Also

<a href='#SkBitmap_allocPixels'>allocPixels</a>

<a name='SkBitmap_installPixels_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_installPixels'>installPixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& <a href='SkPixmap_Reference#Pixmap'>pixmap</a>)
</pre>

Sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> to <a href='#SkBitmap_installPixels_3_pixmap'>pixmap</a>.<a href='#SkPixmap_info'>info()</a> following the rules in <a href='#SkBitmap_setInfo'>setInfo</a>(), and creates
<a href='undocumented#SkPixelRef'>SkPixelRef</a> containing <a href='#SkBitmap_installPixels_3_pixmap'>pixmap</a>.<a href='#SkPixmap_addr'>addr()</a> and <a href='#SkBitmap_installPixels_3_pixmap'>pixmap</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>().

If <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> could not be set, or <a href='#SkBitmap_installPixels_3_pixmap'>pixmap</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() is less than
<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>(): calls <a href='#SkBitmap_reset'>reset()</a>, and returns false.

Otherwise, if <a href='#SkBitmap_installPixels_3_pixmap'>pixmap</a>.<a href='#SkPixmap_addr'>addr()</a> equals nullptr: sets <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, returns true.

Caller must ensure that <a href='#SkBitmap_installPixels_3_pixmap'>pixmap</a> is valid for the lifetime of <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> and <a href='undocumented#SkPixelRef'>SkPixelRef</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_installPixels_3_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td><a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>, <a href='undocumented#Pixel'>pixel</a> address, and <a href='#SkBitmap_rowBytes'>rowBytes</a>()</td>
  </tr>
</table>

### Return Value

true if <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> was set to <a href='#SkBitmap_installPixels_3_pixmap'>pixmap</a>.<a href='#SkPixmap_info'>info()</a>

### Example

<div><fiddle-embed name="6e2a8c9358b34aebd2ec586815fe9d3a"><div>Draw a five by five <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, and draw it again with a center white <a href='undocumented#Pixel'>pixel</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkBitmap_allocPixels'>allocPixels</a>

<a name='Pixels'></a>

<a name='SkBitmap_setPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_setPixels'>setPixels</a>(void* pixels)
</pre>

Replaces <a href='undocumented#SkPixelRef'>SkPixelRef</a> with <a href='#SkBitmap_setPixels_pixels'>pixels</a>, preserving <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> and <a href='#SkBitmap_rowBytes'>rowBytes</a>().
Sets <a href='undocumented#SkPixelRef'>SkPixelRef</a> origin to (0, 0).

If <a href='#SkBitmap_setPixels_pixels'>pixels</a> is nullptr, or if <a href='#SkBitmap_info'>info()</a>.<a href='#SkImageInfo_colorType'>colorType</a>() equals <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>;
release reference to <a href='undocumented#SkPixelRef'>SkPixelRef</a>, and set <a href='undocumented#SkPixelRef'>SkPixelRef</a> to nullptr.

Caller is responsible for handling ownership <a href='undocumented#Pixel'>pixel</a> memory for the lifetime
of <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> and <a href='undocumented#SkPixelRef'>SkPixelRef</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_setPixels_pixels'><code><strong>pixels</strong></code></a></td>
    <td>address of  <a href='undocumented#Pixel_Storage'>pixel storage</a>, managed by caller</td>
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

Allocates <a href='undocumented#Pixel'>pixel</a> memory with <a href='#SkBitmap_HeapAllocator'>HeapAllocator</a>, and replaces existing <a href='undocumented#SkPixelRef'>SkPixelRef</a>.
The allocation <a href='undocumented#Size'>size</a> is determined by <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> width, height, and <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.

Returns false if <a href='#SkBitmap_info'>info()</a>.<a href='#SkImageInfo_colorType'>colorType</a>() is <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, or allocation fails.

### Return Value

true if the allocation succeeds

### Example

<div><fiddle-embed name="720e4c053fae9e929ab6518b47e49370"><div><a href='SkBitmap_Reference#Bitmap'>Bitmap</a> hosts and draws gray values in set1. <a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a> replaces <a href='#Pixel_Ref'>Pixel_Ref</a>
and erases it to black, but does not alter set1. <a href='#SkBitmap_setPixels'>setPixels</a> replaces black
<a href='#Pixel_Ref'>Pixel_Ref</a> with set1.
</div></fiddle-embed></div>

### See Also

<a href='#SkBitmap_allocPixels'>allocPixels</a> <a href='#SkBitmap_installPixels'>installPixels</a> <a href='#SkBitmap_setPixels'>setPixels</a>

<a name='SkBitmap_allocPixels_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_allocPixels'>allocPixels</a>()
</pre>

Allocates <a href='undocumented#Pixel'>pixel</a> memory with <a href='#SkBitmap_HeapAllocator'>HeapAllocator</a>, and replaces existing <a href='undocumented#SkPixelRef'>SkPixelRef</a>.
The allocation <a href='undocumented#Size'>size</a> is determined by <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> width, height, and <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.

Aborts if <a href='#SkBitmap_info'>info()</a>.<a href='#SkImageInfo_colorType'>colorType</a>() is <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, or allocation fails.
Abort steps may be provided by the user at compile
time by defining SK_ABORT.

### Example

<div><fiddle-embed name="1219b38c788bf270fb20f8cd2d78cff8"><div><a href='SkBitmap_Reference#Bitmap'>Bitmap</a> hosts and draws gray values in set1. <a href='#SkBitmap_allocPixels'>allocPixels</a> replaces <a href='#Pixel_Ref'>Pixel_Ref</a>
and erases it to black, but does not alter set1. <a href='#SkBitmap_setPixels'>setPixels</a> replaces black
<a href='#Pixel_Ref'>Pixel_Ref</a> with set2.
</div></fiddle-embed></div>

### See Also

<a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a> <a href='#SkBitmap_installPixels'>installPixels</a> <a href='#SkBitmap_setPixels'>setPixels</a>

<a name='SkBitmap_tryAllocPixels_4'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_tryAllocPixels'>tryAllocPixels</a>(<a href='#SkBitmap_Allocator'>Allocator</a>* allocator)
</pre>

Allocates <a href='undocumented#Pixel'>pixel</a> memory with <a href='#SkBitmap_tryAllocPixels_4_allocator'>allocator</a>, and replaces existing <a href='undocumented#SkPixelRef'>SkPixelRef</a>.
The allocation <a href='undocumented#Size'>size</a> is determined by <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> width, height, and <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.
If <a href='#SkBitmap_tryAllocPixels_4_allocator'>allocator</a> is nullptr, use <a href='#SkBitmap_HeapAllocator'>HeapAllocator</a> instead.

Returns false if <a href='#SkBitmap_Allocator'>Allocator</a>::<a href='#SkBitmap_Allocator_allocPixelRef'>allocPixelRef</a> return false.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_tryAllocPixels_4_allocator'><code><strong>allocator</strong></code></a></td>
    <td>instance of <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_Allocator'>Allocator</a> instantiation</td>
  </tr>
</table>

### Return Value

true if custom <a href='#SkBitmap_tryAllocPixels_4_allocator'>allocator</a> reports success

### Example

<div><fiddle-embed name="eb6f861ca1839146d26e40d56c2a001c"><div><a href='#SkBitmap_HeapAllocator'>HeapAllocator</a> limits the maximum <a href='undocumented#Size'>size</a> of <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> to two gigabytes. Using
a custom <a href='#SkBitmap_tryAllocPixels_4_allocator'>allocator</a>, this limitation may be relaxed. This example can be
modified to allocate an eight gigabyte <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> on a 64-bit platform with
sufficient memory.
</div></fiddle-embed></div>

### See Also

<a href='#SkBitmap_allocPixels'>allocPixels</a> <a href='#SkBitmap_Allocator'>Allocator</a> <a href='#Pixel_Ref'>Pixel_Ref</a>

<a name='SkBitmap_allocPixels_4'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_allocPixels'>allocPixels</a>(<a href='#SkBitmap_Allocator'>Allocator</a>* allocator)
</pre>

Allocates <a href='undocumented#Pixel'>pixel</a> memory with <a href='#SkBitmap_allocPixels_4_allocator'>allocator</a>, and replaces existing <a href='undocumented#SkPixelRef'>SkPixelRef</a>.
The allocation <a href='undocumented#Size'>size</a> is determined by <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> width, height, and <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a>.
If <a href='#SkBitmap_allocPixels_4_allocator'>allocator</a> is nullptr, use <a href='#SkBitmap_HeapAllocator'>HeapAllocator</a> instead.

Aborts if <a href='#SkBitmap_Allocator'>Allocator</a>::<a href='#SkBitmap_Allocator_allocPixelRef'>allocPixelRef</a> return false. Abort steps may be provided by
the user at compile time by defining SK_ABORT.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_allocPixels_4_allocator'><code><strong>allocator</strong></code></a></td>
    <td>instance of <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>::<a href='#SkBitmap_Allocator'>Allocator</a> instantiation</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1b2800d23c9ea249b45c2c21a34b6d14"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_allocPixels'>allocPixels</a> <a href='#SkBitmap_Allocator'>Allocator</a> <a href='#Pixel_Ref'>Pixel_Ref</a>

<a name='SkBitmap_pixelRef'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkPixelRef'>SkPixelRef</a>* <a href='#SkBitmap_pixelRef'>pixelRef</a>()const
</pre>

Returns <a href='undocumented#SkPixelRef'>SkPixelRef</a>, which contains: <a href='undocumented#Pixel'>pixel</a> base address; its dimensions; and
<a href='#SkBitmap_rowBytes'>rowBytes</a>(), the interval from one row to the next. Does not change <a href='undocumented#SkPixelRef'>SkPixelRef</a>
reference count. <a href='undocumented#SkPixelRef'>SkPixelRef</a> may be shared by multiple <a href='SkBitmap_Reference#Bitmap'>bitmaps</a>.
If <a href='undocumented#SkPixelRef'>SkPixelRef</a> has not been set, returns nullptr.

### Return Value

<a href='undocumented#SkPixelRef'>SkPixelRef</a>, or nullptr

### Example

<div><fiddle-embed name="5db2d30870a7cc45f28e22578d1880c3"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_getPixels'>getPixels</a> <a href='#SkBitmap_getAddr'>getAddr</a>

<a name='SkBitmap_pixelRefOrigin'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='#SkBitmap_pixelRefOrigin'>pixelRefOrigin</a>()const
</pre>

Returns origin of pixels within <a href='undocumented#SkPixelRef'>SkPixelRef</a>. <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> bounds is always contained
by <a href='undocumented#SkPixelRef'>SkPixelRef</a> bounds, which may be the same <a href='undocumented#Size'>size</a> or larger. Multiple <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>
can share the same <a href='undocumented#SkPixelRef'>SkPixelRef</a>, where each <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> has different bounds.

The returned origin added to <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> dimensions equals or is smaller than the
<a href='undocumented#SkPixelRef'>SkPixelRef</a> dimensions.

Returns (0, 0) if <a href='undocumented#SkPixelRef'>SkPixelRef</a> is nullptr.

### Return Value

<a href='undocumented#Pixel'>pixel</a> origin within <a href='undocumented#SkPixelRef'>SkPixelRef</a>

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
void <a href='#SkBitmap_setPixelRef'>setPixelRef</a>(<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkPixelRef'>SkPixelRef</a>&gt; <a href='#SkBitmap_pixelRef'>pixelRef</a>, int dx, int dy)
</pre>

Replaces <a href='#SkBitmap_setPixelRef_pixelRef'>pixelRef</a> and origin in <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>.  <a href='#SkBitmap_setPixelRef_dx'>dx</a> and <a href='#SkBitmap_setPixelRef_dy'>dy</a> specify the offset
within the <a href='undocumented#SkPixelRef'>SkPixelRef</a> pixels for the top-left corner of the <a href='SkBitmap_Reference#Bitmap'>bitmap</a>.

Asserts in debug builds if <a href='#SkBitmap_setPixelRef_dx'>dx</a> or <a href='#SkBitmap_setPixelRef_dy'>dy</a> are out of range. Pins <a href='#SkBitmap_setPixelRef_dx'>dx</a> and <a href='#SkBitmap_setPixelRef_dy'>dy</a>
to legal range in release builds.

The caller is responsible for ensuring that the pixels match the
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> and <a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> in <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_setPixelRef_pixelRef'><code><strong>pixelRef</strong></code></a></td>
    <td><a href='undocumented#SkPixelRef'>SkPixelRef</a> describing <a href='undocumented#Pixel'>pixel</a> address and <a href='#SkBitmap_rowBytes'>rowBytes</a>()</td>
  </tr>
  <tr>    <td><a name='SkBitmap_setPixelRef_dx'><code><strong>dx</strong></code></a></td>
    <td>column offset in <a href='undocumented#SkPixelRef'>SkPixelRef</a> for <a href='SkBitmap_Reference#Bitmap'>bitmap</a> origin</td>
  </tr>
  <tr>    <td><a name='SkBitmap_setPixelRef_dy'><code><strong>dy</strong></code></a></td>
    <td>row offset in <a href='undocumented#SkPixelRef'>SkPixelRef</a> for <a href='SkBitmap_Reference#Bitmap'>bitmap</a> origin</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f98cc0451c6e77a8833d261c9a484c5f"><div>Treating 32-bit <a href='undocumented#Data'>data</a> as 8-bit <a href='undocumented#Data'>data</a> is unlikely to produce useful results.
</div></fiddle-embed></div>

### See Also

<a href='#SkBitmap_setInfo'>setInfo</a>

<a name='SkBitmap_readyToDraw'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_readyToDraw'>readyToDraw</a>()const
</pre>

Returns true if <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> is can be drawn.

### Return Value

true if <a href='#SkBitmap_getPixels'>getPixels</a>() is not nullptr

### Example

<div><fiddle-embed name="e89c78ca992e2e789ed50944fe68f920"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_getPixels'>getPixels</a> <a href='#SkBitmap_drawsNothing'>drawsNothing</a>

<a name='SkBitmap_getGenerationID'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkBitmap_getGenerationID'>getGenerationID</a>()const
</pre>

Returns a unique value corresponding to the pixels in <a href='undocumented#SkPixelRef'>SkPixelRef</a>.
Returns a different value after <a href='#SkBitmap_notifyPixelsChanged'>notifyPixelsChanged</a>() has been called.
Returns zero if <a href='undocumented#SkPixelRef'>SkPixelRef</a> is nullptr.

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
void <a href='#SkBitmap_notifyPixelsChanged'>notifyPixelsChanged</a>()const
</pre>

Marks that pixels in <a href='undocumented#SkPixelRef'>SkPixelRef</a> have changed. Subsequent calls to
<a href='#SkBitmap_getGenerationID'>getGenerationID</a>() return a different value.

### Example

<div><fiddle-embed name="8f463ed17b0ed4fb9c503a0ec71706f9"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_getGenerationID'>getGenerationID</a> <a href='#SkBitmap_isVolatile'>isVolatile</a> <a href='#Pixel_Ref'>Pixel_Ref</a>

<a name='Draw'></a>

<a name='SkBitmap_eraseColor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkBitmap_eraseColor'>eraseColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> c)const
</pre>

Replaces <a href='undocumented#Pixel'>pixel</a> values with <a href='#SkBitmap_eraseColor_c'>c</a>. All pixels contained by <a href='#SkBitmap_bounds'>bounds()</a> are affected.
If the <a href='#SkBitmap_colorType'>colorType</a>() is <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a> or <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>, then <a href='SkColor_Reference#Alpha'>alpha</a>
is ignored; RGB is treated as opaque. If <a href='#SkBitmap_colorType'>colorType</a>() is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>,
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
void <a href='#SkBitmap_eraseARGB'>eraseARGB</a>(<a href='undocumented#U8CPU'>U8CPU</a> a, <a href='undocumented#U8CPU'>U8CPU</a> r, <a href='undocumented#U8CPU'>U8CPU</a> g, <a href='undocumented#U8CPU'>U8CPU</a> b)const
</pre>

Replaces <a href='undocumented#Pixel'>pixel</a> values with <a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='SkColor_Reference#Color'>color</a> built from <a href='#SkBitmap_eraseARGB_a'>a</a>, <a href='#SkBitmap_eraseARGB_r'>r</a>, <a href='#SkBitmap_eraseARGB_g'>g</a>, and <a href='#SkBitmap_eraseARGB_b'>b</a>.
All pixels contained by <a href='#SkBitmap_bounds'>bounds()</a> are affected.
If the <a href='#SkBitmap_colorType'>colorType</a>() is <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a> or <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>, then <a href='#SkBitmap_eraseARGB_a'>a</a>
is ignored; <a href='#SkBitmap_eraseARGB_r'>r</a>, <a href='#SkBitmap_eraseARGB_g'>g</a>, and <a href='#SkBitmap_eraseARGB_b'>b</a> are treated as opaque. If <a href='#SkBitmap_colorType'>colorType</a>() is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>,
then <a href='#SkBitmap_eraseARGB_r'>r</a>, <a href='#SkBitmap_eraseARGB_g'>g</a>, and <a href='#SkBitmap_eraseARGB_b'>b</a> are ignored.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_eraseARGB_a'><code><strong>a</strong></code></a></td>
    <td>amount of <a href='SkColor_Reference#Alpha'>alpha</a>, from fully transparent (0) to fully opaque (255)</td>
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
void <a href='#SkBitmap_erase'>erase</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> c, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& area)const
</pre>

Replaces <a href='undocumented#Pixel'>pixel</a> values inside <a href='#SkBitmap_erase_area'>area</a> with <a href='#SkBitmap_erase_c'>c</a>. If <a href='#SkBitmap_erase_area'>area</a> does not intersect <a href='#SkBitmap_bounds'>bounds()</a>,
call has no effect.

If the <a href='#SkBitmap_colorType'>colorType</a>() is <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a> or <a href='SkImageInfo_Reference#kRGB_565_SkColorType'>kRGB_565_SkColorType</a>, then <a href='SkColor_Reference#Alpha'>alpha</a>
is ignored; RGB is treated as opaque. If <a href='#SkBitmap_colorType'>colorType</a>() is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>,
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

<a name='SkBitmap_getColor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkBitmap_getColor'>getColor</a>(int x, int y)const
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> at (<a href='#SkBitmap_getColor_x'>x</a>, <a href='#SkBitmap_getColor_y'>y</a>) as <a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='SkColor_Reference#Color'>color</a>.
Returns black with <a href='SkColor_Reference#Alpha'>alpha</a> if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>.

Input is not validated: out of bounds values of <a href='#SkBitmap_getColor_x'>x</a> or <a href='#SkBitmap_getColor_y'>y</a> trigger an assert() if
built with SK_DEBUG defined; and returns undefined values or may crash if
SK_RELEASE is defined. Fails if <a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a> or
<a href='undocumented#Pixel'>pixel</a> address is nullptr.

<a href='undocumented#SkColorSpace'>SkColorSpace</a> in <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> is ignored. Some <a href='SkColor_Reference#Color'>color</a> precision may be lost in the
conversion to <a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='SkColor_Reference#Color'>color</a>; original <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Data'>data</a> may have additional
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

<a href='undocumented#Pixel'>pixel</a> converted to <a href='undocumented#Unpremultiply'>unpremultiplied</a> <a href='SkColor_Reference#Color'>color</a>

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
float <a href='#SkBitmap_getAlphaf'>getAlphaf</a>(int x, int y)const
</pre>

Looks up the <a href='undocumented#Pixel'>pixel</a> at (<a href='#SkBitmap_getAlphaf_x'>x</a>,<a href='#SkBitmap_getAlphaf_y'>y</a>) and return its <a href='SkColor_Reference#Alpha'>alpha</a> component, normalized to [0..1].
This is roughly equivalent to <code>SkGetColorA(<a href='#SkBitmap_getColor'>getColor</a>())</code>, but can be more efficient
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

<a href='SkColor_Reference#Alpha'>alpha</a> converted to normalized float

### See Also

<a href='#SkBitmap_getColor'>getColor</a>

<a name='SkBitmap_getAddr'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void* <a href='#SkBitmap_getAddr'>getAddr</a>(int x, int y)const
</pre>

Returns <a href='undocumented#Pixel'>pixel</a> address at (<a href='#SkBitmap_getAddr_x'>x</a>, <a href='#SkBitmap_getAddr_y'>y</a>).

Input is not validated: out of bounds values of <a href='#SkBitmap_getAddr_x'>x</a> or <a href='#SkBitmap_getAddr_y'>y</a>, or <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>,
trigger an assert() if built with SK_DEBUG defined. Returns nullptr if
<a href='SkImageInfo_Reference#SkColorType'>SkColorType</a> is <a href='SkImageInfo_Reference#kUnknown_SkColorType'>kUnknown_SkColorType</a>, or <a href='undocumented#SkPixelRef'>SkPixelRef</a> is nullptr.

Performs a lookup of <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Size'>size</a>; for better performance, call
one of: <a href='#SkBitmap_getAddr8'>getAddr8</a>(), <a href='#SkBitmap_getAddr16'>getAddr16</a>(), or <a href='#SkBitmap_getAddr32'>getAddr32</a>().

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
uint32_t* <a href='#SkBitmap_getAddr32'>getAddr32</a>(int x, int y)const
</pre>

Returns address at (<a href='#SkBitmap_getAddr32_x'>x</a>, <a href='#SkBitmap_getAddr32_y'>y</a>).

Input is not validated. Triggers an assert() if built with SK_DEBUG defined and:

<table>  <tr>
    <td><a href='#Pixel_Ref'>Pixel_Ref</a> is nullptr</td>
  </tr>  <tr>
    <td><a href='#SkBitmap_bytesPerPixel'>bytesPerPixel</a>() is not four</td>
  </tr>  <tr>
    <td><a href='#SkBitmap_getAddr32_x'>x</a> is negative, or not less than <a href='#SkBitmap_width'>width()</a></td>
  </tr>  <tr>
    <td><a href='#SkBitmap_getAddr32_y'>y</a> is negative, or not less than <a href='#SkBitmap_height'>height()</a></td>
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

unsigned 32-bit pointer to <a href='undocumented#Pixel'>pixel</a> at (<a href='#SkBitmap_getAddr32_x'>x</a>, <a href='#SkBitmap_getAddr32_y'>y</a>)

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
uint16_t* <a href='#SkBitmap_getAddr16'>getAddr16</a>(int x, int y)const
</pre>

Returns address at (<a href='#SkBitmap_getAddr16_x'>x</a>, <a href='#SkBitmap_getAddr16_y'>y</a>).

Input is not validated. Triggers an assert() if built with SK_DEBUG defined and:

<table>  <tr>
    <td><a href='#Pixel_Ref'>Pixel_Ref</a> is nullptr</td>
  </tr>  <tr>
    <td><a href='#SkBitmap_bytesPerPixel'>bytesPerPixel</a>() is not two</td>
  </tr>  <tr>
    <td><a href='#SkBitmap_getAddr16_x'>x</a> is negative, or not less than <a href='#SkBitmap_width'>width()</a></td>
  </tr>  <tr>
    <td><a href='#SkBitmap_getAddr16_y'>y</a> is negative, or not less than <a href='#SkBitmap_height'>height()</a></td>
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

unsigned 16-bit pointer to <a href='undocumented#Pixel'>pixel</a> at (<a href='#SkBitmap_getAddr16_x'>x</a>, <a href='#SkBitmap_getAddr16_y'>y</a>)

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
uint8_t* <a href='#SkBitmap_getAddr8'>getAddr8</a>(int x, int y)const
</pre>

Returns address at (<a href='#SkBitmap_getAddr8_x'>x</a>, <a href='#SkBitmap_getAddr8_y'>y</a>).

Input is not validated. Triggers an assert() if built with SK_DEBUG defined and:

<table>  <tr>
    <td><a href='#Pixel_Ref'>Pixel_Ref</a> is nullptr</td>
  </tr>  <tr>
    <td><a href='#SkBitmap_bytesPerPixel'>bytesPerPixel</a>() is not one</td>
  </tr>  <tr>
    <td><a href='#SkBitmap_getAddr8_x'>x</a> is negative, or not less than <a href='#SkBitmap_width'>width()</a></td>
  </tr>  <tr>
    <td><a href='#SkBitmap_getAddr8_y'>y</a> is negative, or not less than <a href='#SkBitmap_height'>height()</a></td>
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

unsigned 8-bit pointer to <a href='undocumented#Pixel'>pixel</a> at (<a href='#SkBitmap_getAddr8_x'>x</a>, <a href='#SkBitmap_getAddr8_y'>y</a>)

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
bool <a href='#SkBitmap_extractSubset'>extractSubset</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* dst, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& subset)const
</pre>

Shares <a href='#Pixel_Ref'>Pixel_Ref</a> with <a href='#SkBitmap_extractSubset_dst'>dst</a>. Pixels are not copied; <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> and <a href='#SkBitmap_extractSubset_dst'>dst</a> <a href='SkPoint_Reference#Point'>point</a>
to the same pixels; <a href='#SkBitmap_extractSubset_dst'>dst</a> <a href='#SkBitmap_bounds'>bounds()</a> are set to the intersection of <a href='#SkBitmap_extractSubset_subset'>subset</a>
and the original <a href='#SkBitmap_bounds'>bounds()</a>.

<a href='#SkBitmap_extractSubset_subset'>subset</a> may be larger than <a href='#SkBitmap_bounds'>bounds()</a>. Any area outside of <a href='#SkBitmap_bounds'>bounds()</a> is ignored.

Any contents of <a href='#SkBitmap_extractSubset_dst'>dst</a> are discarded. <a href='#SkBitmap_isVolatile'>isVolatile</a> setting is copied to <a href='#SkBitmap_extractSubset_dst'>dst</a>.
<a href='#SkBitmap_extractSubset_dst'>dst</a> is set to <a href='#SkBitmap_colorType'>colorType</a>, <a href='#SkBitmap_alphaType'>alphaType</a>, and <a href='#SkBitmap_colorSpace'>colorSpace</a>.

Return false if:

<table>  <tr>
    <td><a href='#SkBitmap_extractSubset_dst'>dst</a> is nullptr</td>
  </tr>  <tr>
    <td><a href='#Pixel_Ref'>Pixel_Ref</a> is nullptr</td>
  </tr>  <tr>
    <td><a href='#SkBitmap_extractSubset_subset'>subset</a> does not intersect <a href='#SkBitmap_bounds'>bounds()</a></td>
  </tr>
</table>

### Parameters

<table>  <tr>    <td><a name='SkBitmap_extractSubset_dst'><code><strong>dst</strong></code></a></td>
    <td><a href='SkBitmap_Reference#Bitmap'>Bitmap</a> set to <a href='#SkBitmap_extractSubset_subset'>subset</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_extractSubset_subset'><code><strong>subset</strong></code></a></td>
    <td>rectangle of pixels to reference</td>
  </tr>
</table>

### Return Value

true if <a href='#SkBitmap_extractSubset_dst'>dst</a> is replaced by <a href='#SkBitmap_extractSubset_subset'>subset</a>

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
bool <a href='#SkBitmap_readPixels'>readPixels</a>(const <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>& dstInfo, void* dstPixels, size_t dstRowBytes, int srcX, int srcY)const
</pre>

Copies a <a href='SkRect_Reference#Rect'>Rect</a> of pixels from <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> to <a href='#SkBitmap_readPixels_dstPixels'>dstPixels</a>. Copy starts at (<a href='#SkBitmap_readPixels_srcX'>srcX</a>, <a href='#SkBitmap_readPixels_srcY'>srcY</a>),
and does not exceed <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> (<a href='#SkBitmap_width'>width()</a>, <a href='#SkBitmap_height'>height()</a>).

<a href='#SkBitmap_readPixels_dstInfo'>dstInfo</a> specifies width, height, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, and <a href='#Color_Space'>Color_Space</a> of
destination. <a href='#SkBitmap_readPixels_dstRowBytes'>dstRowBytes</a> specifics the gap from one destination row to the next.
Returns true if pixels are copied. Returns false if:

<table>  <tr>
    <td><a href='#SkBitmap_readPixels_dstInfo'>dstInfo</a> has no address</td>
  </tr>  <tr>
    <td><a href='#SkBitmap_readPixels_dstRowBytes'>dstRowBytes</a> is less than <a href='#SkBitmap_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>()</td>
  </tr>  <tr>
    <td><a href='#Pixel_Ref'>Pixel_Ref</a> is nullptr</td>
  </tr>
</table>

Pixels are copied only if <a href='undocumented#Pixel'>pixel</a> conversion is possible. If <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorType'>colorType</a> is
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, or <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkBitmap_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorType'>colorType</a>() must match.
If <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorType'>colorType</a> is <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkBitmap_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorSpace'>colorSpace</a>() must match.
If <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_alphaType'>alphaType</a> is <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkBitmap_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_alphaType'>alphaType</a>() must
match. If <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorSpace'>colorSpace</a> is nullptr, <a href='#SkBitmap_readPixels_dstInfo'>dstInfo</a>.<a href='#SkImageInfo_colorSpace'>colorSpace</a>() must match. Returns
false if <a href='undocumented#Pixel'>pixel</a> conversion is not possible.

<a href='#SkBitmap_readPixels_srcX'>srcX</a> and <a href='#SkBitmap_readPixels_srcY'>srcY</a> may be negative to copy only top or left of source. Returns
false if <a href='#SkBitmap_width'>width()</a> or <a href='#SkBitmap_height'>height()</a> is zero or negative.
Returns false if <code><a href='undocumented#abs()'>abs</a>(<a href='#SkBitmap_readPixels_srcX'>srcX</a>) >= <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_width'>width()</a></code>, or if <code><a href='undocumented#abs()'>abs</a>(<a href='#SkBitmap_readPixels_srcY'>srcY</a>) >= <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_height'>height()</a></code>.

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
bool <a href='#SkBitmap_readPixels'>readPixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& dst, int srcX, int srcY)const
</pre>

Copies a <a href='SkRect_Reference#Rect'>Rect</a> of pixels from <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> to <a href='#SkBitmap_readPixels_2_dst'>dst</a>. Copy starts at (<a href='#SkBitmap_readPixels_2_srcX'>srcX</a>, <a href='#SkBitmap_readPixels_2_srcY'>srcY</a>), and
does not exceed <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> (<a href='#SkBitmap_width'>width()</a>, <a href='#SkBitmap_height'>height()</a>).

<a href='#SkBitmap_readPixels_2_dst'>dst</a> specifies width, height, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, <a href='#Color_Space'>Color_Space</a>,  <a href='undocumented#Pixel_Storage'>pixel storage</a>,
and  <a href='#Row_Bytes'>row bytes</a> of destination. <a href='#SkBitmap_readPixels_2_dst'>dst</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() specifics the gap from one destination
row to the next. Returns true if pixels are copied. Returns false if:

<table>  <tr>
    <td><a href='#SkBitmap_readPixels_2_dst'>dst</a>  <a href='undocumented#Pixel_Storage'>pixel storage</a> equals nullptr</td>
  </tr>  <tr>
    <td><a href='#SkBitmap_readPixels_2_dst'>dst</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() is less than <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>()</td>
  </tr>  <tr>
    <td><a href='#Pixel_Ref'>Pixel_Ref</a> is nullptr</td>
  </tr>
</table>

Pixels are copied only if <a href='undocumented#Pixel'>pixel</a> conversion is possible. If <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorType'>colorType</a> is
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, or <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkBitmap_readPixels_2_dst'>dst</a> <a href='#Image_Info_Color_Type'>Color_Type</a> must match.
If <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorType'>colorType</a> is <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkBitmap_readPixels_2_dst'>dst</a> <a href='#Color_Space'>Color_Space</a> must match.
If <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_alphaType'>alphaType</a> is <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkBitmap_readPixels_2_dst'>dst</a> <a href='#Image_Info_Alpha_Type'>Alpha_Type</a> must
match. If <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorSpace'>colorSpace</a> is nullptr, <a href='#SkBitmap_readPixels_2_dst'>dst</a> <a href='#Color_Space'>Color_Space</a> must match. Returns
false if <a href='undocumented#Pixel'>pixel</a> conversion is not possible.

<a href='#SkBitmap_readPixels_2_srcX'>srcX</a> and <a href='#SkBitmap_readPixels_2_srcY'>srcY</a> may be negative to copy only top or left of source. Returns
false if <a href='#SkBitmap_width'>width()</a> or <a href='#SkBitmap_height'>height()</a> is zero or negative.
Returns false if <code><a href='undocumented#abs()'>abs</a>(<a href='#SkBitmap_readPixels_2_srcX'>srcX</a>) >= <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_width'>width()</a></code>, or if <code><a href='undocumented#abs()'>abs</a>(<a href='#SkBitmap_readPixels_2_srcY'>srcY</a>) >= <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_height'>height()</a></code>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_readPixels_2_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkPixmap_Reference#Pixmap'>Pixmap</a>: <a href='#Image_Info'>Image_Info</a>, pixels,  <a href='#Row_Bytes'>row bytes</a></td>
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
bool <a href='#SkBitmap_readPixels'>readPixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& dst)const
</pre>

Copies a <a href='SkRect_Reference#Rect'>Rect</a> of pixels from <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> to <a href='#SkBitmap_readPixels_3_dst'>dst</a>. Copy starts at (0, 0), and
does not exceed <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> (<a href='#SkBitmap_width'>width()</a>, <a href='#SkBitmap_height'>height()</a>).

<a href='#SkBitmap_readPixels_3_dst'>dst</a> specifies width, height, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, <a href='#Color_Space'>Color_Space</a>,  <a href='undocumented#Pixel_Storage'>pixel storage</a>,
and  <a href='#Row_Bytes'>row bytes</a> of destination. <a href='#SkBitmap_readPixels_3_dst'>dst</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() specifics the gap from one destination
row to the next. Returns true if pixels are copied. Returns false if:

<table>  <tr>
    <td><a href='#SkBitmap_readPixels_3_dst'>dst</a>  <a href='undocumented#Pixel_Storage'>pixel storage</a> equals nullptr</td>
  </tr>  <tr>
    <td><a href='#SkBitmap_readPixels_3_dst'>dst</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() is less than <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>()</td>
  </tr>  <tr>
    <td><a href='#Pixel_Ref'>Pixel_Ref</a> is nullptr</td>
  </tr>
</table>

Pixels are copied only if <a href='undocumented#Pixel'>pixel</a> conversion is possible. If <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorType'>colorType</a> is
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, or <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkBitmap_readPixels_3_dst'>dst</a> <a href='#Image_Info_Color_Type'>Color_Type</a> must match.
If <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorType'>colorType</a> is <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkBitmap_readPixels_3_dst'>dst</a> <a href='#Color_Space'>Color_Space</a> must match.
If <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_alphaType'>alphaType</a> is <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkBitmap_readPixels_3_dst'>dst</a> <a href='#Image_Info_Alpha_Type'>Alpha_Type</a> must
match. If <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorSpace'>colorSpace</a> is nullptr, <a href='#SkBitmap_readPixels_3_dst'>dst</a> <a href='#Color_Space'>Color_Space</a> must match. Returns
false if <a href='undocumented#Pixel'>pixel</a> conversion is not possible.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_readPixels_3_dst'><code><strong>dst</strong></code></a></td>
    <td>destination <a href='SkPixmap_Reference#Pixmap'>Pixmap</a>: <a href='#Image_Info'>Image_Info</a>, pixels,  <a href='#Row_Bytes'>row bytes</a></td>
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
bool <a href='#SkBitmap_writePixels'>writePixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& src, int dstX, int dstY)
</pre>

Copies a <a href='SkRect_Reference#Rect'>Rect</a> of pixels from <a href='#SkBitmap_writePixels_src'>src</a>. Copy starts at (<a href='#SkBitmap_writePixels_dstX'>dstX</a>, <a href='#SkBitmap_writePixels_dstY'>dstY</a>), and does not exceed
(<a href='#SkBitmap_writePixels_src'>src</a>.<a href='#SkPixmap_width'>width()</a>, <a href='#SkBitmap_writePixels_src'>src</a>.<a href='#SkPixmap_height'>height()</a>).

<a href='#SkBitmap_writePixels_src'>src</a> specifies width, height, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, <a href='#Color_Space'>Color_Space</a>,  <a href='undocumented#Pixel_Storage'>pixel storage</a>,
and  <a href='#Row_Bytes'>row bytes</a> of source. <a href='#SkBitmap_writePixels_src'>src</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() specifics the gap from one source
row to the next. Returns true if pixels are copied. Returns false if:

<table>  <tr>
    <td><a href='#SkBitmap_writePixels_src'>src</a>  <a href='undocumented#Pixel_Storage'>pixel storage</a> equals nullptr</td>
  </tr>  <tr>
    <td><a href='#SkBitmap_writePixels_src'>src</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() is less than <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>()</td>
  </tr>  <tr>
    <td><a href='#Pixel_Ref'>Pixel_Ref</a> is nullptr</td>
  </tr>
</table>

Pixels are copied only if <a href='undocumented#Pixel'>pixel</a> conversion is possible. If <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorType'>colorType</a> is
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, or <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkBitmap_writePixels_src'>src</a> <a href='#Image_Info_Color_Type'>Color_Type</a> must match.
If <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorType'>colorType</a> is <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkBitmap_writePixels_src'>src</a> <a href='#Color_Space'>Color_Space</a> must match.
If <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_alphaType'>alphaType</a> is <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkBitmap_writePixels_src'>src</a> <a href='#Image_Info_Alpha_Type'>Alpha_Type</a> must
match. If <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorSpace'>colorSpace</a> is nullptr, <a href='#SkBitmap_writePixels_src'>src</a> <a href='#Color_Space'>Color_Space</a> must match. Returns
false if <a href='undocumented#Pixel'>pixel</a> conversion is not possible.

<a href='#SkBitmap_writePixels_dstX'>dstX</a> and <a href='#SkBitmap_writePixels_dstY'>dstY</a> may be negative to copy only top or left of source. Returns
false if <a href='#SkBitmap_width'>width()</a> or <a href='#SkBitmap_height'>height()</a> is zero or negative.
Returns false if <code><a href='undocumented#abs()'>abs</a>(<a href='#SkBitmap_writePixels_dstX'>dstX</a>) >= <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_width'>width()</a></code>, or if <code><a href='undocumented#abs()'>abs</a>(<a href='#SkBitmap_writePixels_dstY'>dstY</a>) >= <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_height'>height()</a></code>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_writePixels_src'><code><strong>src</strong></code></a></td>
    <td>source <a href='SkPixmap_Reference#Pixmap'>Pixmap</a>: <a href='#Image_Info'>Image_Info</a>, pixels,  <a href='#Row_Bytes'>row bytes</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_writePixels_dstX'><code><strong>dstX</strong></code></a></td>
    <td>column index whose absolute value is less than <a href='#SkBitmap_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_writePixels_dstY'><code><strong>dstY</strong></code></a></td>
    <td>row index whose absolute value is less than <a href='#SkBitmap_height'>height()</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkBitmap_writePixels_src'>src</a> pixels are copied to <a href='SkBitmap_Reference#Bitmap'>Bitmap</a>

### Example

<div><fiddle-embed name="9b3133a6673d2514d166398adbe1f9f4"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_readPixels'>readPixels</a>

<a name='SkBitmap_writePixels_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_writePixels'>writePixels</a>(const <a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>& src)
</pre>

Copies a <a href='SkRect_Reference#Rect'>Rect</a> of pixels from <a href='#SkBitmap_writePixels_2_src'>src</a>. Copy starts at (0, 0), and does not exceed
(<a href='#SkBitmap_writePixels_2_src'>src</a>.<a href='#SkPixmap_width'>width()</a>, <a href='#SkBitmap_writePixels_2_src'>src</a>.<a href='#SkPixmap_height'>height()</a>).

<a href='#SkBitmap_writePixels_2_src'>src</a> specifies width, height, <a href='#Image_Info_Color_Type'>Color_Type</a>, <a href='#Image_Info_Alpha_Type'>Alpha_Type</a>, <a href='#Color_Space'>Color_Space</a>,  <a href='undocumented#Pixel_Storage'>pixel storage</a>,
and  <a href='#Row_Bytes'>row bytes</a> of source. <a href='#SkBitmap_writePixels_2_src'>src</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() specifics the gap from one source
row to the next. Returns true if pixels are copied. Returns false if:

<table>  <tr>
    <td><a href='#SkBitmap_writePixels_2_src'>src</a>  <a href='undocumented#Pixel_Storage'>pixel storage</a> equals nullptr</td>
  </tr>  <tr>
    <td><a href='#SkBitmap_writePixels_2_src'>src</a>.<a href='#SkPixmap_rowBytes'>rowBytes</a>() is less than <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_minRowBytes'>minRowBytes</a>()</td>
  </tr>  <tr>
    <td><a href='#Pixel_Ref'>Pixel_Ref</a> is nullptr</td>
  </tr>
</table>

Pixels are copied only if <a href='undocumented#Pixel'>pixel</a> conversion is possible. If <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorType'>colorType</a> is
<a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, or <a href='SkImageInfo_Reference#kAlpha_8_SkColorType'>kAlpha_8_SkColorType</a>; <a href='#SkBitmap_writePixels_2_src'>src</a> <a href='#Image_Info_Color_Type'>Color_Type</a> must match.
If <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorType'>colorType</a> is <a href='SkImageInfo_Reference#kGray_8_SkColorType'>kGray_8_SkColorType</a>, <a href='#SkBitmap_writePixels_2_src'>src</a> <a href='#Color_Space'>Color_Space</a> must match.
If <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_alphaType'>alphaType</a> is <a href='SkImageInfo_Reference#kOpaque_SkAlphaType'>kOpaque_SkAlphaType</a>, <a href='#SkBitmap_writePixels_2_src'>src</a> <a href='#Image_Info_Alpha_Type'>Alpha_Type</a> must
match. If <a href='SkBitmap_Reference#Bitmap'>Bitmap</a> <a href='#SkBitmap_colorSpace'>colorSpace</a> is nullptr, <a href='#SkBitmap_writePixels_2_src'>src</a> <a href='#Color_Space'>Color_Space</a> must match. Returns
false if <a href='undocumented#Pixel'>pixel</a> conversion is not possible.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_writePixels_2_src'><code><strong>src</strong></code></a></td>
    <td>source <a href='SkPixmap_Reference#Pixmap'>Pixmap</a>: <a href='#Image_Info'>Image_Info</a>, pixels,  <a href='#Row_Bytes'>row bytes</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkBitmap_writePixels_2_src'>src</a> pixels are copied to <a href='SkBitmap_Reference#Bitmap'>Bitmap</a>

### Example

<div><fiddle-embed name="faa5dfa466f6e16c07c124d971f32679"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_readPixels'>readPixels</a>

<a name='SkBitmap_extractAlpha'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_extractAlpha'>extractAlpha</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* dst)const
</pre>

Sets <a href='#SkBitmap_extractAlpha_dst'>dst</a> to <a href='SkColor_Reference#Alpha'>alpha</a> described by pixels. Returns false if <a href='#SkBitmap_extractAlpha_dst'>dst</a> cannot be written to
or <a href='#SkBitmap_extractAlpha_dst'>dst</a> pixels cannot be allocated.

Uses <a href='#SkBitmap_HeapAllocator'>HeapAllocator</a> to reserve memory for <a href='#SkBitmap_extractAlpha_dst'>dst</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_extractAlpha_dst'><code><strong>dst</strong></code></a></td>
    <td>holds <a href='undocumented#SkPixelRef'>SkPixelRef</a> to fill with <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkCanvas_Reference#Layer'>layer</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkCanvas_Reference#Layer'>layer</a> was constructed in <a href='#SkBitmap_extractAlpha_dst'>dst</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>

### Example

<div><fiddle-embed name="ab6577df079e6c70511cf2bfc6447b44"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_extractSubset'>extractSubset</a>

<a name='SkBitmap_extractAlpha_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_extractAlpha'>extractAlpha</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* dst, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>* offset)const
</pre>

Sets <a href='#SkBitmap_extractAlpha_2_dst'>dst</a> to <a href='SkColor_Reference#Alpha'>alpha</a> described by pixels. Returns false if <a href='#SkBitmap_extractAlpha_2_dst'>dst</a> cannot be written to
or <a href='#SkBitmap_extractAlpha_2_dst'>dst</a> pixels cannot be allocated.

If <a href='#SkBitmap_extractAlpha_2_paint'>paint</a> is not nullptr and contains <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>
generates  <a href='undocumented#Mask_Alpha'>mask alpha</a> from <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>. Uses <a href='#SkBitmap_HeapAllocator'>HeapAllocator</a> to reserve memory for <a href='#SkBitmap_extractAlpha_2_dst'>dst</a>
<a href='undocumented#SkPixelRef'>SkPixelRef</a>. Sets <a href='#SkBitmap_extractAlpha_2_offset'>offset</a> to top-left position for <a href='#SkBitmap_extractAlpha_2_dst'>dst</a> for alignment with <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>;
(0, 0) unless <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> generates mask.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_extractAlpha_2_dst'><code><strong>dst</strong></code></a></td>
    <td>holds <a href='undocumented#SkPixelRef'>SkPixelRef</a> to fill with <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkCanvas_Reference#Layer'>layer</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_extractAlpha_2_paint'><code><strong>paint</strong></code></a></td>
    <td>holds optional <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkBitmap_extractAlpha_2_offset'><code><strong>offset</strong></code></a></td>
    <td>top-left position for <a href='#SkBitmap_extractAlpha_2_dst'>dst</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

true if <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkCanvas_Reference#Layer'>layer</a> was constructed in <a href='#SkBitmap_extractAlpha_2_dst'>dst</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>

### Example

<div><fiddle-embed name="092739b4cd5d732a27c07ced8ef45f01"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_extractSubset'>extractSubset</a>

<a name='SkBitmap_extractAlpha_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_extractAlpha'>extractAlpha</a>(<a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>* dst, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a>, <a href='#SkBitmap_Allocator'>Allocator</a>* allocator, <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>* offset)const
</pre>

Sets <a href='#SkBitmap_extractAlpha_3_dst'>dst</a> to <a href='SkColor_Reference#Alpha'>alpha</a> described by pixels. Returns false if <a href='#SkBitmap_extractAlpha_3_dst'>dst</a> cannot be written to
or <a href='#SkBitmap_extractAlpha_3_dst'>dst</a> pixels cannot be allocated.

If <a href='#SkBitmap_extractAlpha_3_paint'>paint</a> is not nullptr and contains <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>, <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>
generates  <a href='undocumented#Mask_Alpha'>mask alpha</a> from <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>. <a href='#SkBitmap_extractAlpha_3_allocator'>allocator</a> may reference a custom allocation
class or be set to nullptr to use <a href='#SkBitmap_HeapAllocator'>HeapAllocator</a>. Sets <a href='#SkBitmap_extractAlpha_3_offset'>offset</a> to top-left
position for <a href='#SkBitmap_extractAlpha_3_dst'>dst</a> for alignment with <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>; (0, 0) unless <a href='undocumented#SkMaskFilter'>SkMaskFilter</a> generates
mask.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_extractAlpha_3_dst'><code><strong>dst</strong></code></a></td>
    <td>holds <a href='undocumented#SkPixelRef'>SkPixelRef</a> to fill with <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkCanvas_Reference#Layer'>layer</a></td>
  </tr>
  <tr>    <td><a name='SkBitmap_extractAlpha_3_paint'><code><strong>paint</strong></code></a></td>
    <td>holds optional <a href='undocumented#SkMaskFilter'>SkMaskFilter</a>; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkBitmap_extractAlpha_3_allocator'><code><strong>allocator</strong></code></a></td>
    <td>function to reserve memory for <a href='undocumented#SkPixelRef'>SkPixelRef</a>; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkBitmap_extractAlpha_3_offset'><code><strong>offset</strong></code></a></td>
    <td>top-left position for <a href='#SkBitmap_extractAlpha_3_dst'>dst</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

true if <a href='SkColor_Reference#Alpha'>alpha</a> <a href='SkCanvas_Reference#Layer'>layer</a> was constructed in <a href='#SkBitmap_extractAlpha_3_dst'>dst</a> <a href='undocumented#SkPixelRef'>SkPixelRef</a>

### Example

<div><fiddle-embed name="cd7543fa8c9f3cede46dc2d72eb8c4bd"></fiddle-embed></div>

### See Also

<a href='#SkBitmap_extractSubset'>extractSubset</a>

<a name='SkBitmap_peekPixels'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkBitmap_peekPixels'>peekPixels</a>(<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>* <a href='SkPixmap_Reference#Pixmap'>pixmap</a>)const
</pre>

Copies <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> <a href='undocumented#Pixel'>pixel</a> address,  <a href='#Row_Bytes'>row bytes</a>, and <a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a> to <a href='#SkBitmap_peekPixels_pixmap'>pixmap</a>, if address
is available, and returns true. If <a href='undocumented#Pixel'>pixel</a> address is not available, return
false and leave <a href='#SkBitmap_peekPixels_pixmap'>pixmap</a> unchanged.

<a href='#SkBitmap_peekPixels_pixmap'>pixmap</a> contents become invalid on any future change to <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a>.

### Parameters

<table>  <tr>    <td><a name='SkBitmap_peekPixels_pixmap'><code><strong>pixmap</strong></code></a></td>
    <td>storage for <a href='undocumented#Pixel'>pixel</a> state if pixels are readable; otherwise, ignored</td>
  </tr>
</table>

### Return Value

true if <a href='SkBitmap_Reference#SkBitmap'>SkBitmap</a> has direct access to pixels

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
void <a href='#SkBitmap_validate'>validate()</a> const;
</pre>

### See Also

<a href='SkImageInfo_Reference#SkImageInfo'>SkImageInfo</a>::<a href='#SkImageInfo_validate'>validate</a>

