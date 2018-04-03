SkImageInfo Reference
===

# <a name="Image_Info"></a> Image Info
<a href="#Image_Info">Image Info</a> specifies the dimensions and encoding of the pixels in a <a href="SkBitmap_Reference#Bitmap">Bitmap</a>.
The dimensions are integral width and height. The encoding is how pixel
bits describe <a href="undocumented#Alpha">Color Alpha</a>, transparency; <a href="undocumented#Color">Color</a> components red, blue,
and green; and <a href="undocumented#Color_Space">Color Space</a>, the range and linearity of colors.

<a href="#Image_Info">Image Info</a> describes an uncompressed raster pixels. In contrast, <a href="SkImage_Reference#Image">Image</a>
additionally describes compressed pixels like PNG, and <a href="SkSurface_Reference#Surface">Surface</a> describes
destinations on the GPU. <a href="SkImage_Reference#Image">Image</a> and <a href="SkSurface_Reference#Surface">Surface</a> may be specified by <a href="#Image_Info">Image Info</a>,
but <a href="SkImage_Reference#Image">Image</a> and <a href="SkSurface_Reference#Surface">Surface</a> may not contain <a href="#Image_Info">Image Info</a>.

## <a name="Overview"></a> Overview

## <a name="Overview_Subtopic"></a> Overview Subtopic

| name | description |
| --- | --- |
| <a href="#Constructor">Constructor</a> | functions that construct <a href="#SkImageInfo">SkImageInfo</a> |
| <a href="#Member_Function">Member Function</a> | static functions and member methods |
| <a href="#Operator">Operator</a> | operator overloading methods |
| <a href="#Related_Function">Related Function</a> | similar methods grouped together |

## <a name="Constant"></a> Constant

| name | description |
| --- | --- |

## <a name="Alpha_Type"></a> Alpha Type

## <a name="SkAlphaType"></a> Enum SkAlphaType

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href="#SkAlphaType">SkAlphaType</a> {
        <a href="#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a>,
        <a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>,
        <a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a>,
        <a href="#kUnpremul_SkAlphaType">kUnpremul_SkAlphaType</a>,
        kLastEnum_SkAlphaType = <a href="#kUnpremul_SkAlphaType">kUnpremul_SkAlphaType</a>,
    };
</pre>

Describes how to interpret the alpha component of a pixel. A pixel may
be opaque, or <a href="undocumented#Alpha">Color Alpha</a>, describing multiple levels of transparency.

In simple blending, <a href="undocumented#Alpha">Color Alpha</a> weights the draw color and the destination
color to create a new color. If alpha describes a weight from zero to one:

new color = draw color * alpha + destination color * (1 - alpha)
In practice alpha is encoded in two or more bits, where 1.0 equals all bits set.

<a href="undocumented#RGB">Color RGB</a> may have <a href="undocumented#Alpha">Color Alpha</a> included in each component value; the stored
value is the original <a href="undocumented#RGB">Color RGB</a> multiplied by <a href="undocumented#Alpha">Color Alpha</a>. <a href="undocumented#Premultiply">Premultiplied</a> color
components improve performance.

### Constants

<table>
  <tr>
    <td><a name="kUnknown_SkAlphaType"> <code><strong>kUnknown_SkAlphaType </strong></code> </a></td><td>0</td><td><a href="#Alpha_Type">Alpha Type</a> is uninitialized.
</td>
  </tr>
  <tr>
    <td><a name="kOpaque_SkAlphaType"> <code><strong>kOpaque_SkAlphaType </strong></code> </a></td><td>1</td><td>Pixels are opaque. The <a href="#Color_Type">Color Type</a> must have no explicit alpha
component, or all alpha components must be set to their maximum value.
</td>
  </tr>
  <tr>
    <td><a name="kPremul_SkAlphaType"> <code><strong>kPremul_SkAlphaType </strong></code> </a></td><td>2</td><td>Pixels have alpha premultiplied into color components.
<a href="SkSurface_Reference#Surface">Surface</a> pixels must be premultiplied.
</td>
  </tr>
  <tr>
    <td><a name="kUnpremul_SkAlphaType"> <code><strong>kUnpremul_SkAlphaType </strong></code> </a></td><td>3</td><td><a href="undocumented#Pixel">Pixel</a> color component values are independent of alpha value.
Images generated from encoded data like PNG do not premultiply pixel color
components. <a href="#kUnpremul_SkAlphaType">kUnpremul_SkAlphaType</a> is supported for <a href="SkImage_Reference#Image">Image</a> pixels, but not for
<a href="SkSurface_Reference#Surface">Surface</a> pixels.
</td>
  </tr>
</table>

### See Also

<a href="#SkColorType">SkColorType</a> <a href="undocumented#SkColorSpace">SkColorSpace</a>



## <a name="Alpha_Type_Opaque"></a> Alpha Type Opaque

Use <a href="#Alpha_Type_Opaque">Opaque</a> as a hint to optimize drawing when alpha component
of all pixel is set to its maximum value of 1.0; all alpha component bits are set.
If <a href="#Image_Info">Image Info</a> is set to <a href="#Alpha_Type_Opaque">Opaque</a> but all alpha values are not 1.0, results are
undefined.

### Example

<div><fiddle-embed name="79146a1a41d58d22582fdc567c6ffe4e"><div><a href="undocumented#SkPreMultiplyARGB">SkPreMultiplyARGB</a> parameter a is set to 255, its maximum value, and is interpreted
as <a href="undocumented#Alpha">Color Alpha</a> of 1.0. <a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a> may be set to improve performance.
If <a href="undocumented#SkPreMultiplyARGB">SkPreMultiplyARGB</a> parameter a is set to a value smaller than 255,
<a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a> must be used instead to avoid undefined results.
The four displayed values are the original component values, though not necessarily
in the same order.
</div></fiddle-embed></div>

## <a name="Alpha_Type_Premul"></a> Alpha Type Premul

Use <a href="#Alpha_Type_Premul">Premul</a> when stored color components are the original color multiplied by the
alpha component. The alpha component range of 0.0 to 1.0 is achieved by dividing
the integer bit value by the maximum bit value.

stored color = original color * alpha / max alphaThe color component must be equal to or smaller than the alpha component,
or the results are undefined.

### Example

<div><fiddle-embed name="ad696b39c915803d566e96896ec3a36c"><div><a href="undocumented#SkPreMultiplyARGB">SkPreMultiplyARGB</a> parameter a is set to 150, less than its maximum value, and is
interpreted as <a href="undocumented#Alpha">Color Alpha</a> of about 0.6. <a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a> must be set, since
<a href="undocumented#SkPreMultiplyARGB">SkPreMultiplyARGB</a> parameter a is set to a value smaller than 255,
to avoid undefined results.
The four displayed values reflect that the alpha component has been multiplied
by the original color.
</div></fiddle-embed></div>

## <a name="Alpha_Type_Unpremul"></a> Alpha Type Unpremul

Use <a href="#Alpha_Type_Unpremul">Unpremul</a> if stored color components are not divided by the alpha component.
Some drawing destinations may not support <a href="#Alpha_Type_Unpremul">Unpremul</a>.

### Example

<div><fiddle-embed name="b8216a9e5ff5bc61a0e46eba7d36307b"><div><a href="undocumented#SkColorSetARGB">SkColorSetARGB</a> parameter a is set to 150, less than its maximum value, and is
interpreted as <a href="undocumented#Alpha">Color Alpha</a> of about 0.6. color is not premultiplied;
color components may have values greater than color alpha.
The four displayed values are the original component values, though not necessarily
in the same order.
</div></fiddle-embed></div>

<a name="SkAlphaTypeIsOpaque"></a>
## SkAlphaTypeIsOpaque

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static inline bool SkAlphaTypeIsOpaque(SkAlphaType at)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkAlphaTypeIsOpaque_at"> <code><strong>at </strong></code> </a></td> <td>
one of: <a href="#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a>, <a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>, <a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a>,
<a href="#kUnpremul_SkAlphaType">kUnpremul_SkAlphaType</a> </td>
  </tr>
</table>

### Return Value

true if <a href="#SkAlphaTypeIsOpaque_at">at</a> equals <a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>

---

## <a name="Color_Type"></a> Color Type

## <a name="Color_Type_Native"></a> Color Type Native

## <a name="SkColorType"></a> Enum SkColorType

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href="#SkColorType">SkColorType</a> {
        <a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>,
        <a href="#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>,
        <a href="#kRGB_565_SkColorType">kRGB_565_SkColorType</a>,
        <a href="#kARGB_4444_SkColorType">kARGB_4444_SkColorType</a>,
        <a href="#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a>,
        <a href="#kRGB_888x_SkColorType">kRGB_888x_SkColorType</a>,
        <a href="#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>,
        <a href="#kRGBA_1010102_SkColorType">kRGBA_1010102_SkColorType</a>,
        <a href="#kRGB_101010x_SkColorType">kRGB_101010x_SkColorType</a>,
        <a href="#kGray_8_SkColorType">kGray_8_SkColorType</a>,
        <a href="#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a>,

        kLastEnum_SkColorType = <a href="#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a>,
    #if SK_PMCOLOR_BYTE_ORDER(B,G,R,A)
        <a href="#kN32_SkColorType">kN32_SkColorType</a> = <a href="#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>,
    #elif SK_PMCOLOR_BYTE_ORDER(R,G,B,A)
        <a href="#kN32_SkColorType">kN32_SkColorType</a> = <a href="#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a>,
    #else
        #error "SK_*32_SHIFT values must correspond to BGRA or RGBA byte order"
    #endif
    };
</pre>

Describes how pixel bits encode color. A pixel may be an alpha mask, a
gray level, <a href="undocumented#RGB">Color RGB</a>, or <a href="undocumented#ARGB">Color ARGB</a>.

<a href="#kN32_SkColorType">kN32_SkColorType</a> selects the native 32-bit <a href="undocumented#ARGB">Color ARGB</a> format. On Little_Endian
processors, pixels containing 8-bit <a href="undocumented#ARGB">Color ARGB</a> components pack into 32-bit
<a href="#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>. On Big_Endian processors, pixels pack into 32-bit
<a href="#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a>.

### Constants

<table>
  <tr>
    <td><a name="kUnknown_SkColorType"> <code><strong>kUnknown_SkColorType </strong></code> </a></td><td>0</td><td><a href="#Color_Type">Color Type</a> is uninitialized; encoding format and size is unknown.
</td>
  </tr>
  <tr>
    <td><a name="kAlpha_8_SkColorType"> <code><strong>kAlpha_8_SkColorType </strong></code> </a></td><td>1</td><td>Encodes <a href="undocumented#Alpha">Color Alpha</a> as <a href="#Color_Type_Alpha_8">Alpha 8</a> pixel in an 8-bit byte.
</td>
  </tr>
  <tr>
    <td><a name="kRGB_565_SkColorType"> <code><strong>kRGB_565_SkColorType </strong></code> </a></td><td>2</td><td>Encodes <a href="undocumented#RGB">Color RGB</a> as <a href="#Color_Type_BGR_565">BGR 565</a> pixel in a 16-bit word.
</td>
  </tr>
  <tr>
    <td><a name="kARGB_4444_SkColorType"> <code><strong>kARGB_4444_SkColorType </strong></code> </a></td><td>3</td><td>Encodes <a href="undocumented#ARGB">Color ARGB</a> as <a href="#Color_Type_ABGR_4444">ABGR 4444</a> pixel in a 16-bit word.
</td>
  </tr>
  <tr>
    <td><a name="kRGBA_8888_SkColorType"> <code><strong>kRGBA_8888_SkColorType </strong></code> </a></td><td>4</td><td>Encodes <a href="undocumented#ARGB">Color ARGB</a> as <a href="#Color_Type_RGBA_8888">RGBA 8888</a> pixel in a 32-bit word.
</td>
  </tr>
  <tr>
    <td><a name="kRGB_888x_SkColorType"> <code><strong>kRGB_888x_SkColorType </strong></code> </a></td><td>5</td><td>Encodes <a href="undocumented#RGB">Color RGB</a> as <a href="#Color_Type_RGB_888x">RGB 888x</a> pixel in a 32-bit word.
</td>
  </tr>
  <tr>
    <td><a name="kBGRA_8888_SkColorType"> <code><strong>kBGRA_8888_SkColorType </strong></code> </a></td><td>6</td><td>Encodes <a href="undocumented#ARGB">Color ARGB</a> as <a href="#Color_Type_BGRA_8888">BGRA 8888</a> pixel in a 32-bit word.
</td>
  </tr>
  <tr>
    <td><a name="kRGBA_1010102_SkColorType"> <code><strong>kRGBA_1010102_SkColorType </strong></code> </a></td><td>7</td><td>Encodes <a href="undocumented#ARGB">Color ARGB</a> as <a href="#Color_Type_RGBA_1010102">RGBA 1010102</a> pixel in a 32-bit word.
</td>
  </tr>
  <tr>
    <td><a name="kRGB_101010x_SkColorType"> <code><strong>kRGB_101010x_SkColorType </strong></code> </a></td><td>8</td><td>Encodes <a href="undocumented#RGB">Color RGB</a> as <a href="#Color_Type_RGB_101010x">RGB 101010x</a> pixel in a 32-bit word.
</td>
  </tr>
  <tr>
    <td><a name="kGray_8_SkColorType"> <code><strong>kGray_8_SkColorType </strong></code> </a></td><td>9</td><td>Encodes <a href="undocumented#Gray">Color Gray</a> as <a href="#Color_Type_Gray_8">Gray 8</a> in an 8-bit byte.
</td>
  </tr>
  <tr>
    <td><a name="kRGBA_F16_SkColorType"> <code><strong>kRGBA_F16_SkColorType </strong></code> </a></td><td>10</td><td>Encodes <a href="undocumented#ARGB">Color ARGB</a> as <a href="#Color_Type_RGBA_F16">RGBA F16</a> in a 64-bit word.
</td>
  </tr>
</table>

### Constants

<table>
  <tr>
    <td><a name="kN32_SkColorType"> <code><strong>kN32_SkColorType </strong></code> </a></td><td>4</td><td>Encodes <a href="undocumented#ARGB">Color ARGB</a> as either <a href="#Color_Type_RGBA_8888">RGBA 8888</a> or <a href="#Color_Type_BGRA_8888">BGRA 8888</a>, whichever
is native to the platform.
</td>
  </tr>
</table>

### See Also

<a href="#SkAlphaType">SkAlphaType</a> <a href="undocumented#SkColorSpace">SkColorSpace</a>



## <a name="Color_Type_Alpha_8"></a> Color Type Alpha 8

<a href="#Color_Type_Alpha_8">Alpha 8</a> is an 8-bit byte pixel encoding that represents transparency. A value of zero is
completely transparent; a value of 255 is completely opaque. <a href="SkBitmap_Reference#Bitmap">Bitmap</a> with <a href="#Color_Type_Alpha_8">Alpha 8</a>
pixels does not visibly draw, because its pixels have no color information.
The paired <a href="#SkAlphaType">SkAlphaType</a> is ignored.

### Example

<div><fiddle-embed name="21ae21e4ce53d2018e042dd457997300"><div><a href="#Color_Type_Alpha_8">Alpha 8</a> pixels can modify another draw. orangePaint fills the bounds of bitmap,
with its transparency set to alpha8 pixel value.
</div></fiddle-embed></div>

## <a name="Color_Type_BGR_565"></a> Color Type BGR 565

<a href="#Color_Type_BGR_565">BGR 565</a> is a 16-bit word pixel encoding that contains five bits of blue,
six bits of green, and five bits of red. <a href="#Color_Type_BGR_565">BGR 565</a> is fully opaque as if its
<a href="undocumented#Alpha">Color Alpha</a> was set to one, and should always be paired with <a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>.

![Color_Type_BGR_565](https://fiddle.skia.org/i/b674a54eb4188d5ce66c04cebdb61089_raster.png "")

### Example

<div><fiddle-embed name="54e4c690d64d73ba028c5b250c0d09f0"></fiddle-embed></div>

## <a name="Color_Type_ABGR_4444"></a> Color Type ABGR 4444

<a href="#Color_Type_ABGR_4444">ABGR 4444</a> is a 16-bit word pixel encoding that contains four bits of alpha,
four bits of blue, four bits of green, and four bits of red.

![Color_Type_ABGR_4444](https://fiddle.skia.org/i/0441bdba65a19aa72b75b7fa62d22121_raster.png "")

If paired with <a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a>: blue, green, and red components are
premultiplied by the alpha value. If blue, green, or red is greater than alpha,
the drawn result is undefined.

If paired with <a href="#kUnpremul_SkAlphaType">kUnpremul_SkAlphaType</a>: alpha, blue, green, and red components
may have any value. There may be a performance penalty with unpremultipled
pixels.

If paired with <a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>: all alpha component values are at the maximum;
blue, green, and red components are fully opaque. If any alpha component is
less than 15, the drawn result is undefined.

### Example

<div><fiddle-embed name="f89e8200d225ccb839e50a1481db48e9"></fiddle-embed></div>

## <a name="Color_Type_RGBA_8888"></a> Color Type RGBA 8888

<a href="#Color_Type_RGBA_8888">RGBA 8888</a> is a 32-bit word pixel encoding that contains eight bits of red,
eight bits of green, eight bits of blue, and eight bits of alpha.

![Color_Type_RGBA_8888](https://fiddle.skia.org/i/4ccd35f27fe73dce8cce8c75e18df23c_raster.png "")

If paired with <a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a>: red, green, and blue components are
premultiplied by the alpha value. If red, green, or blue is greater than alpha,
the drawn result is undefined.

If paired with <a href="#kUnpremul_SkAlphaType">kUnpremul_SkAlphaType</a>: alpha, red, green, and blue components
may have any value. There may be a performance penalty with unpremultipled
pixels.

If paired with <a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>: all alpha component values are at the maximum;
red, green, and blue components are fully opaque. If any alpha component is
less than 255, the drawn result is undefined.

On Big_Endian platforms, <a href="#Color_Type_RGBA_8888">RGBA 8888</a> is the native <a href="#Color_Type">Color Type</a>, and will have
the best performance. Use <a href="#kN32_SkColorType">kN32_SkColorType</a> to choose the best <a href="#Color_Type">Color Type</a> for
the platform at compile time.

### Example

<div><fiddle-embed name="f3b9effa700b1b95ce10caf422b020cd"></fiddle-embed></div>

## <a name="Color_Type_RGB_888x"></a> Color Type RGB 888x

<a href="#Color_Type_RGB_888x">RGB 888x</a> is a 32-bit word pixel encoding that contains eight bits of red,
eight bits of green, eight bits of blue, and eight unused bits. <a href="#Color_Type_RGB_888x">RGB 888x</a> is fully
opaque as if its <a href="undocumented#Alpha">Color Alpha</a> was set to one, and should always be paired with
<a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>.

![Color_Type_RGB_888x](https://fiddle.skia.org/i/fecfe58c25cfc1b1e411e5eb50f7d8d1_raster.png "")

### Example

<div><fiddle-embed name="1e236c4c9ce0597c22186589fee9dc1e"></fiddle-embed></div>

## <a name="Color_Type_BGRA_8888"></a> Color Type BGRA 8888

<a href="#Color_Type_BGRA_8888">BGRA 8888</a> is a 32-bit word pixel encoding that contains eight bits of blue,
eight bits of green, eight bits of red, and eight bits of alpha.

![Color_Type_BGRA_8888](https://fiddle.skia.org/i/babd0e12db21a88c74d4e88aa40268ab_raster.png "")

If paired with <a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a>: blue, green, and red components are
premultiplied by the alpha value. If blue, green, or red is greater than alpha,
the drawn result is undefined.

If paired with <a href="#kUnpremul_SkAlphaType">kUnpremul_SkAlphaType</a>: blue, green, red, and alpha components
may have any value. There may be a performance penalty with unpremultipled
pixels.

If paired with <a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>: all alpha component values are at the maximum;
blue, green, and red components are fully opaque. If any alpha component is
less than 255, the drawn result is undefined.

On Little_Endian platforms, <a href="#Color_Type_BGRA_8888">BGRA 8888</a> is the native <a href="#Color_Type">Color Type</a>, and will have
the best performance. Use <a href="#kN32_SkColorType">kN32_SkColorType</a> to choose the best <a href="#Color_Type">Color Type</a> for
the platform at compile time.

### Example

<div><fiddle-embed name="fcadba68a4fe7253b1726487e12b1eeb"></fiddle-embed></div>

## <a name="Color_Type_RGBA_1010102"></a> Color Type RGBA 1010102

<a href="#Color_Type_RGBA_1010102">RGBA 1010102</a> is a 32-bit word pixel encoding that contains ten bits of red,
ten bits of green, ten bits of blue, and two bits of alpha. Possible alpha
values are zero: fully transparent; one: 33% opaque; two: 67% opaque;
three: fully opaque.

![Color_Type_RGBA_1010102](https://fiddle.skia.org/i/6c470410001ad8f1ee9f58204c66f1bb_raster.png "")

If paired with <a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a>: red, green, and blue components are
premultiplied by the alpha value. If red, green, or blue is greater than the
alpha replicated to ten bits, the drawn result is undefined.

If paired with <a href="#kUnpremul_SkAlphaType">kUnpremul_SkAlphaType</a>: alpha, red, green, and blue components
may have any value. There may be a performance penalty with unpremultipled
pixels.

If paired with <a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>: all alpha component values are at the maximum;
red, green, and blue components are fully opaque. If any alpha component is
less than 3, the drawn result is undefined.

### Example

<div><fiddle-embed name="1bf1a6f087b4f2f93962960902061a7a"></fiddle-embed></div>

## <a name="Color_Type_RGB_101010x"></a> Color Type RGB 101010x

<a href="#Color_Type_RGB_101010x">RGB 101010x</a> is a 32-bit word pixel encoding that contains ten bits of red,
ten bits of green, ten bits of blue, and two unused bits.  <a href="#Color_Type_RGB_101010x">RGB 101010x</a> is fully
opaque as if its <a href="undocumented#Alpha">Color Alpha</a> was set to one, and should always be paired with
<a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>.

![Color_Type_RGB_101010x](https://fiddle.skia.org/i/c22477b11dabaa3e3a0b5bb33a7733cd_raster.png "")

### Example

<div><fiddle-embed name="d975ec17354b1297841e4a31d3f6a5d5"></fiddle-embed></div>

## <a name="Color_Type_Gray_8"></a> Color Type Gray 8

<a href="#Color_Type_Gray_8">Gray 8</a> is an 8-bit byte pixel encoding that represents equal values for red,
blue, and green, reprsenting colors from black to white.  <a href="#Color_Type_Gray_8">Gray 8</a> is fully
opaque as if its <a href="undocumented#Alpha">Color Alpha</a> was set to one, and should always be paired with
<a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>.

### Example

<div><fiddle-embed name="93da0eb0b6722a4f33dc7dae094abf0b"></fiddle-embed></div>

## <a name="Color_Type_RGBA_F16"></a> Color Type RGBA F16

<a href="#Color_Type_RGBA_F16">RGBA F16</a> is a 64-bit word pixel encoding that contains sixteen bits of blue,
sixteen bits of green, sixteen bits of red, and sixteen bits of alpha.

Each component encodes a floating point value using <a href="https://www.khronos.org/opengl/wiki/Small_Float_Formats">Half floats</a> .
Meaningful colors are represented by the range 0.0 to 1.0, although smaller
and larger values may be useful when used in combination with <a href="undocumented#Transfer_Mode">Transfer Mode</a>.

![Color_Type_RGBA_F16](https://fiddle.skia.org/i/9344796c059ff5e4f057595e781905b3_raster.png "")

If paired with <a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a>: blue, green, and red components are
premultiplied by the alpha value. If blue, green, or red is greater than alpha,
the drawn result is undefined.

If paired with <a href="#kUnpremul_SkAlphaType">kUnpremul_SkAlphaType</a>: blue, green, red, and alpha components
may have any value. There may be a performance penalty with unpremultipled
pixels.

If paired with <a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>: all alpha component values are at the maximum;
blue, green, and red components are fully opaque. If any alpha component is
less than 255, the drawn result is undefined.

### Example

<div><fiddle-embed name="1795410dffea303b08ba98ee78dc1556"></fiddle-embed></div>

<a name="SkColorTypeBytesPerPixel"></a>
## SkColorTypeBytesPerPixel

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int SkColorTypeBytesPerPixel(SkColorType ct)
</pre>

Returns the number of bytes required to store a pixel, including unused padding.
Returns zero if <a href="#SkColorTypeBytesPerPixel_ct">ct</a> is <a href="#kUnknown_SkColorType">kUnknown_SkColorType</a> or invalid.

### Parameters

<table>  <tr>    <td><a name="SkColorTypeBytesPerPixel_ct"> <code><strong>ct </strong></code> </a></td> <td>
one of: <a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>, <a href="#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>, <a href="#kRGB_565_SkColorType">kRGB_565_SkColorType</a>,
<a href="#kARGB_4444_SkColorType">kARGB_4444_SkColorType</a>, <a href="#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a>, <a href="#kRGB_888x_SkColorType">kRGB_888x_SkColorType</a>,
<a href="#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>, <a href="#kRGBA_1010102_SkColorType">kRGBA_1010102_SkColorType</a>, <a href="#kRGB_101010x_SkColorType">kRGB_101010x_SkColorType</a>,
<a href="#kGray_8_SkColorType">kGray_8_SkColorType</a>, <a href="#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a> </td>
  </tr>
</table>

### Return Value

bytes per pixel

### Example

<div><fiddle-embed name="09ef49d07cb7005ba3e34d5ea53896f5"><a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>, <a href="#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>, <a href="#kRGB_565_SkColorType">kRGB_565_SkColorType</a>,
<a href="#kARGB_4444_SkColorType">kARGB_4444_SkColorType</a>, <a href="#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a>, <a href="#kRGB_888x_SkColorType">kRGB_888x_SkColorType</a>,
<a href="#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>, <a href="#kRGBA_1010102_SkColorType">kRGBA_1010102_SkColorType</a>, <a href="#kRGB_101010x_SkColorType">kRGB_101010x_SkColorType</a>,
<a href="#kGray_8_SkColorType">kGray_8_SkColorType</a>, <a href="#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a> </fiddle-embed></div>

### See Also

<a href="#SkImageInfo_bytesPerPixel">SkImageInfo::bytesPerPixel</a>

---

<a name="SkColorTypeIsAlwaysOpaque"></a>
## SkColorTypeIsAlwaysOpaque

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool SkColorTypeIsAlwaysOpaque(SkColorType ct)
</pre>

Returns true if <a href="#Color_Type">Color Type</a> always decodes <a href="undocumented#Alpha">Color Alpha</a> to 1.0, making the pixel
fully opaque. If true, <a href="#Color_Type">Color Type</a> does not reserve bits to encode <a href="undocumented#Alpha">Color Alpha</a>.

### Parameters

<table>  <tr>    <td><a name="SkColorTypeIsAlwaysOpaque_ct"> <code><strong>ct </strong></code> </a></td> <td>
one of: <a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>, <a href="#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>, <a href="#kRGB_565_SkColorType">kRGB_565_SkColorType</a>,
<a href="#kARGB_4444_SkColorType">kARGB_4444_SkColorType</a>, <a href="#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a>, <a href="#kRGB_888x_SkColorType">kRGB_888x_SkColorType</a>,
<a href="#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>, <a href="#kRGBA_1010102_SkColorType">kRGBA_1010102_SkColorType</a>, <a href="#kRGB_101010x_SkColorType">kRGB_101010x_SkColorType</a>,
<a href="#kGray_8_SkColorType">kGray_8_SkColorType</a>, <a href="#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a> </td>
  </tr>
</table>

### Return Value

true if <a href="undocumented#Alpha">Color Alpha</a> is always set to 1.0

### Example

<div><fiddle-embed name="9b3eb5aaa0dfea9feee54e7650fa5446"><a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>, <a href="#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>, <a href="#kRGB_565_SkColorType">kRGB_565_SkColorType</a>,
<a href="#kARGB_4444_SkColorType">kARGB_4444_SkColorType</a>, <a href="#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a>, <a href="#kRGB_888x_SkColorType">kRGB_888x_SkColorType</a>,
<a href="#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>, <a href="#kRGBA_1010102_SkColorType">kRGBA_1010102_SkColorType</a>, <a href="#kRGB_101010x_SkColorType">kRGB_101010x_SkColorType</a>,
<a href="#kGray_8_SkColorType">kGray_8_SkColorType</a>, <a href="#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a> </fiddle-embed></div>

### See Also

<a href="#SkColorTypeValidateAlphaType">SkColorTypeValidateAlphaType</a>

---

<a name="SkColorTypeValidateAlphaType"></a>
## SkColorTypeValidateAlphaType

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool SkColorTypeValidateAlphaType(SkColorType colorType, SkAlphaType alphaType,
                                  SkAlphaType* canonical = nullptr)
</pre>

Returns true if <a href="#SkColorTypeValidateAlphaType_canonical">canonical</a> can be set to a valid <a href="#Alpha_Type">Alpha Type</a> for <a href="#SkColorTypeValidateAlphaType_colorType">colorType</a>. If
there is more than one valid <a href="#SkColorTypeValidateAlphaType_canonical">canonical</a> <a href="#Alpha_Type">Alpha Type</a>, set to <a href="#SkColorTypeValidateAlphaType_alphaType">alphaType</a>, if valid.
If true is returned and <a href="#SkColorTypeValidateAlphaType_canonical">canonical</a> is not nullptr, store valid <a href="#Alpha_Type">Alpha Type</a>.

Returns false only if <a href="#SkColorTypeValidateAlphaType_alphaType">alphaType</a> is <a href="#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a>, color type is not
<a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>, and <a href="#Color_Type">Color Type</a> is not always opaque. If false is returned,
<a href="#SkColorTypeValidateAlphaType_canonical">canonical</a> is ignored.

For <a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>: set <a href="#SkColorTypeValidateAlphaType_canonical">canonical</a> to <a href="#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a> and return true.
For <a href="#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>: set <a href="#SkColorTypeValidateAlphaType_canonical">canonical</a> to <a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a> or
<a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a> and return true if <a href="#SkColorTypeValidateAlphaType_alphaType">alphaType</a> is not <a href="#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a>.
For <a href="#kRGB_565_SkColorType">kRGB_565_SkColorType</a>, <a href="#kRGB_888x_SkColorType">kRGB_888x_SkColorType</a>, <a href="#kRGB_101010x_SkColorType">kRGB_101010x_SkColorType</a>, and
<a href="#kGray_8_SkColorType">kGray_8_SkColorType</a>: set <a href="#SkColorTypeValidateAlphaType_canonical">canonical</a> to <a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a> and return true.
For <a href="#kARGB_4444_SkColorType">kARGB_4444_SkColorType</a>, <a href="#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a>, <a href="#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>,
<a href="#kRGBA_1010102_SkColorType">kRGBA_1010102_SkColorType</a>, and <a href="#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a>: set <a href="#SkColorTypeValidateAlphaType_canonical">canonical</a> to <a href="#SkColorTypeValidateAlphaType_alphaType">alphaType</a>
and return true if <a href="#SkColorTypeValidateAlphaType_alphaType">alphaType</a> is not <a href="#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a>.

### Parameters

<table>  <tr>    <td><a name="SkColorTypeValidateAlphaType_colorType"> <code><strong>colorType </strong></code> </a></td> <td>
one of: <a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>, <a href="#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>, <a href="#kRGB_565_SkColorType">kRGB_565_SkColorType</a>,
<a href="#kARGB_4444_SkColorType">kARGB_4444_SkColorType</a>, <a href="#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a>, <a href="#kRGB_888x_SkColorType">kRGB_888x_SkColorType</a>,
<a href="#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>, <a href="#kRGBA_1010102_SkColorType">kRGBA_1010102_SkColorType</a>, <a href="#kRGB_101010x_SkColorType">kRGB_101010x_SkColorType</a>,
<a href="#kGray_8_SkColorType">kGray_8_SkColorType</a>, <a href="#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a> </td>
  </tr>  <tr>    <td><a name="SkColorTypeValidateAlphaType_alphaType"> <code><strong>alphaType </strong></code> </a></td> <td>
one of: <a href="#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a>, <a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>, <a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a>,
<a href="#kUnpremul_SkAlphaType">kUnpremul_SkAlphaType</a> </td>
  </tr>  <tr>    <td><a name="SkColorTypeValidateAlphaType_canonical"> <code><strong>canonical </strong></code> </a></td> <td>
storage for <a href="#Alpha_Type">Alpha Type</a></td>
  </tr>
</table>

### Return Value

true if valid <a href="#Alpha_Type">Alpha Type</a> can be associated with <a href="#SkColorTypeValidateAlphaType_colorType">colorType</a>

### Example

<div><fiddle-embed name="befac1c29ed21507d367e4d824383a04"><a href="#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a>, <a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>, <a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a>,
<a href="#kUnpremul_SkAlphaType">kUnpremul_SkAlphaType</a> <a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>, <a href="#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>, <a href="#kRGB_565_SkColorType">kRGB_565_SkColorType</a>,
<a href="#kARGB_4444_SkColorType">kARGB_4444_SkColorType</a>, <a href="#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a>, <a href="#kRGB_888x_SkColorType">kRGB_888x_SkColorType</a>,
<a href="#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>, <a href="#kRGBA_1010102_SkColorType">kRGBA_1010102_SkColorType</a>, <a href="#kRGB_101010x_SkColorType">kRGB_101010x_SkColorType</a>,
<a href="#kGray_8_SkColorType">kGray_8_SkColorType</a>, <a href="#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a> </fiddle-embed></div>

### See Also

<a href="#SkColorTypeIsAlwaysOpaque">SkColorTypeIsAlwaysOpaque</a>

---

## <a name="YUV_ColorSpace"></a> YUV ColorSpace

## <a name="SkYUVColorSpace"></a> Enum SkYUVColorSpace

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href="#SkYUVColorSpace">SkYUVColorSpace</a> {
        <a href="#kJPEG_SkYUVColorSpace">kJPEG_SkYUVColorSpace</a>,
        <a href="#kRec601_SkYUVColorSpace">kRec601_SkYUVColorSpace</a>,
        <a href="#kRec709_SkYUVColorSpace">kRec709_SkYUVColorSpace</a>,
        kLastEnum_SkYUVColorSpace = <a href="#kRec709_SkYUVColorSpace">kRec709_SkYUVColorSpace</a>,
    };
</pre>

Describes color range of YUV pixels. The color mapping from YUV to RGB varies
depending on the source. YUV pixels may be generated by JPEG images, standard
video streams, or high definition video streams. Each has its own mapping from
YUV and RGB.

JPEG YUV values encode the full range of 0 to 255 for all three components.
Video YUV values range from 16 to 235 for all three components. Details of
encoding and conversion to RGB are described in <a href="https://en.wikipedia.org/wiki/YCbCr">YCbCr color space</a> .

### Constants

<table>
  <tr>
    <td><a name="kJPEG_SkYUVColorSpace"> <code><strong>kJPEG_SkYUVColorSpace </strong></code> </a></td><td>0</td><td>Describes standard JPEG color space;
<a href="https://en.wikipedia.org/wiki/Rec._601">CCIR 601</table>

</a> with full range of 0 to 255 for components.
</td>
  </tr>
  <tr>
    <td><a name="kRec601_SkYUVColorSpace"> <code><strong>kRec601_SkYUVColorSpace </strong></code> </a></td><td>1</td><td>Describes standard used by SDTV;
<a href="https://en.wikipedia.org/wiki/Rec._601">CCIR 601</table>

</a> with studio range of 16 to 235 range for components.
</td>
  </tr>
  <tr>
    <td><a name="kRec709_SkYUVColorSpace"> <code><strong>kRec709_SkYUVColorSpace </strong></code> </a></td><td>2</td><td>Describes standard used by HDTV;
<a href="http://en.wikipedia.org/wiki/Rec._709">Rec. 709</table>

</a> with studio range of 16 to 235 range for components.
</td>
  </tr>
</table>

### See Also

<a href="SkImage_Reference#SkImage_MakeFromYUVTexturesCopy">SkImage::MakeFromYUVTexturesCopy</a> <a href="SkImage_Reference#SkImage_MakeFromNV12TexturesCopy">SkImage::MakeFromNV12TexturesCopy</a>



# <a name="SkImageInfo"></a> Struct SkImageInfo
Describes pixel dimensions and encoding. <a href="SkBitmap_Reference#Bitmap">Bitmap</a>, <a href="SkImage_Reference#Image">Image</a>, PixMap, and <a href="SkSurface_Reference#Surface">Surface</a>
can be created from <a href="#Image_Info">Image Info</a>. <a href="#Image_Info">Image Info</a> can be retrieved from <a href="SkBitmap_Reference#Bitmap">Bitmap</a> and
<a href="SkPixmap_Reference#Pixmap">Pixmap</a>, but not from <a href="SkImage_Reference#Image">Image</a> and <a href="SkSurface_Reference#Surface">Surface</a>. For example, <a href="SkImage_Reference#Image">Image</a> and <a href="SkSurface_Reference#Surface">Surface</a>
implementations may defer pixel depth, so may not completely specify <a href="#Image_Info">Image Info</a>.

<a href="#Image_Info">Image Info</a> contains dimensions, the pixel integral width and height. It encodes
how pixel bits describe <a href="undocumented#Alpha">Color Alpha</a>, transparency; <a href="undocumented#Color">Color</a> components red, blue,
and green; and <a href="undocumented#Color_Space">Color Space</a>, the range and linearity of colors.

## <a name="Member_Function"></a> Member Function

| name | description |
| --- | --- |
| <a href="#SkImageInfo_ByteSizeOverflowed">ByteSizeOverflowed</a> | checks result of <a href="#SkImageInfo_computeByteSize">computeByteSize</a> and <a href="#SkImageInfo_computeMinByteSize">computeMinByteSize</a> |
| <a href="#SkImageInfo_Make">Make</a> | creates <a href="#Image_Info">Image Info</a> from dimensions, <a href="#Color_Type">Color Type</a>, <a href="#Alpha_Type">Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkImageInfo_MakeA8">MakeA8</a> | creates <a href="#Image_Info">Image Info</a> with <a href="#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>, <a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a> |
| <a href="#SkImageInfo_MakeN32">MakeN32</a> | creates <a href="#Image_Info">Image Info</a> with <a href="#Color_Type_Native">Native Color Type</a> |
| <a href="#SkImageInfo_MakeN32Premul">MakeN32Premul</a> | creates <a href="#Image_Info">Image Info</a> with <a href="#Color_Type_Native">Native Color Type</a>, <a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a> |
| <a href="#SkImageInfo_MakeS32">MakeS32</a> | creates <a href="#Image_Info">Image Info</a> with <a href="#Color_Type_Native">Native Color Type</a>, sRGB <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkImageInfo_MakeUnknown">MakeUnknown</a> | creates <a href="#Image_Info">Image Info</a> with <a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>, <a href="#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a> |
| <a href="#SkImageInfo_alphaType">alphaType</a> | Returns <a href="#Alpha_Type">Alpha Type</a> |
| <a href="#SkImageInfo_bounds">bounds</a> | returns <a href="#SkImageInfo_width">width</a> and <a href="#SkImageInfo_height">height</a> as Rectangle |
| <a href="#SkImageInfo_bytesPerPixel">bytesPerPixel</a> | returns number of bytes in pixel based on <a href="#Color_Type">Color Type</a> |
| <a href="#SkImageInfo_colorSpace">colorSpace</a> | returns <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkImageInfo_colorType">colorType</a> | returns <a href="#Color_Type">Color Type</a> |
| <a href="#SkImageInfo_computeByteSize">computeByteSize</a> | memory required by pixel buffer with given row bytes |
| <a href="#SkImageInfo_computeMinByteSize">computeMinByteSize</a> | least memory required by pixel buffer |
| <a href="#SkImageInfo_computeOffset">computeOffset</a> | returns byte offset within pixel array |
| <a href="#SkImageInfo_dimensions">dimensions</a> | returns <a href="#SkImageInfo_width">width</a> and <a href="#SkImageInfo_height">height</a> |
| <a href="#SkImageInfo_gammaCloseToSRGB">gammaCloseToSRGB</a> | Returns if <a href="undocumented#Color_Space">Color Space</a> gamma is approximately the same as sRGB |
| <a href="#SkImageInfo_height">height</a> | returns pixel row count |
| <a href="#SkImageInfo_isEmpty">isEmpty</a> | returns if dimensions contain pixels |
| <a href="#SkImageInfo_isOpaque">isOpaque</a> | returns if <a href="#Alpha_Type">Alpha Type</a> is <a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a> |
| <a href="#SkImageInfo_makeAlphaType">makeAlphaType</a> | creates <a href="#Image_Info">Image Info</a> with changed <a href="#Alpha_Type">Alpha Type</a> |
| <a href="#SkImageInfo_makeColorSpace">makeColorSpace</a> | creates <a href="#Image_Info">Image Info</a> with changed <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkImageInfo_makeColorType">makeColorType</a> | creates <a href="#Image_Info">Image Info</a> with changed <a href="#Color_Type">Color Type</a> |
| <a href="#SkImageInfo_makeWH">makeWH</a> | creates <a href="#Image_Info">Image Info</a> with changed dimensions |
| <a href="#SkImageInfo_minRowBytes">minRowBytes</a> | returns <a href="#SkImageInfo_width">width</a> times <a href="#SkImageInfo_bytesPerPixel">bytesPerPixel</a> in 32 bits |
| <a href="#SkImageInfo_minRowBytes64">minRowBytes64</a> | returns <a href="#SkImageInfo_width">width</a> times <a href="#SkImageInfo_bytesPerPixel">bytesPerPixel</a> in 64 bits |
| <a href="#SkImageInfo_refColorSpace">refColorSpace</a> | returns <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkImageInfo_reset">reset</a> | sets zero dimensions, <a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>, <a href="#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a> |
| <a href="#SkImageInfo_shiftPerPixel">shiftPerPixel</a> | returns bit shift from pixels to bytes |
| <a href="#SkImageInfo_validRowBytes">validRowBytes</a> | checks if row bytes is large enough to contain pixel row |
| <a href="#SkImageInfo_validate">validate</a> | asserts if <a href="#Image_Info">Image Info</a> is invalid (debug only) |
| <a href="#SkImageInfo_width">width</a> | returns pixel column count |

## <a name="Related_Function"></a> Related Function

| name | description |
| --- | --- |
| <a href="#Property">Property</a> | metrics and attributes |
| <a href="#Utility">Utility</a> | rarely called management functions |

## <a name="Constructor"></a> Constructor

| name | description |
| --- | --- |
| <a href="#SkImageInfo_Make">Make</a> | creates <a href="#Image_Info">Image Info</a> from dimensions, <a href="#Color_Type">Color Type</a>, <a href="#Alpha_Type">Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkImageInfo_MakeA8">MakeA8</a> | creates <a href="#Image_Info">Image Info</a> with <a href="#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>, <a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a> |
| <a href="#SkImageInfo_MakeN32">MakeN32</a> | creates <a href="#Image_Info">Image Info</a> with <a href="#Color_Type_Native">Native Color Type</a> |
| <a href="#SkImageInfo_MakeN32Premul">MakeN32Premul</a> | creates <a href="#Image_Info">Image Info</a> with <a href="#Color_Type_Native">Native Color Type</a>, <a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a> |
|  | <a href="#SkImageInfo_MakeN32Premul">MakeN32Premul(int width, int height, sk sp&lt;SkColorSpace&gt; cs = nullptr)</a> |
|  | <a href="#SkImageInfo_MakeN32Premul_2">MakeN32Premul(const SkISize& size)</a> |
| <a href="#SkImageInfo_MakeS32">MakeS32</a> | creates <a href="#Image_Info">Image Info</a> with <a href="#Color_Type_Native">Native Color Type</a>, sRGB <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkImageInfo_MakeUnknown">MakeUnknown</a> | creates <a href="#Image_Info">Image Info</a> with <a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>, <a href="#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a> |
|  | <a href="#SkImageInfo_MakeUnknown">MakeUnknown(int width, int height)</a> |
|  | <a href="#SkImageInfo_MakeUnknown_2">MakeUnknown()</a> |
| <a href="#SkImageInfo_empty_constructor">SkImageInfo()</a> | creates with zero dimensions, <a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>, <a href="#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a> |
| <a href="#SkImageInfo_makeAlphaType">makeAlphaType</a> | creates <a href="#Image_Info">Image Info</a> with changed <a href="#Alpha_Type">Alpha Type</a> |
| <a href="#SkImageInfo_makeColorSpace">makeColorSpace</a> | creates <a href="#Image_Info">Image Info</a> with changed <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkImageInfo_makeColorType">makeColorType</a> | creates <a href="#Image_Info">Image Info</a> with changed <a href="#Color_Type">Color Type</a> |
| <a href="#SkImageInfo_makeWH">makeWH</a> | creates <a href="#Image_Info">Image Info</a> with changed dimensions |
| <a href="#SkImageInfo_reset">reset</a> | sets zero dimensions, <a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>, <a href="#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a> |

<a name="SkImageInfo_empty_constructor"></a>
## SkImageInfo

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkImageInfo()
</pre>

Creates an empty <a href="#Image_Info">Image Info</a> with <a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>, <a href="#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a>,
a width and height of zero, and no <a href="undocumented#Color_Space">Color Space</a>.

### Return Value

empty <a href="#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="f206f698e7a8db3d84334c26b1a702dc"><div>An empty <a href="#Image_Info">Image Info</a> may be passed to <a href="SkCanvas_Reference#SkCanvas_accessTopLayerPixels">SkCanvas::accessTopLayerPixels</a> as storage
for the <a href="SkCanvas_Reference#Canvas">Canvas</a> actual <a href="#Image_Info">Image Info</a>.
</div></fiddle-embed></div>

### See Also

<a href="#SkImageInfo_Make">Make</a> <a href="#SkImageInfo_MakeN32">MakeN32</a> <a href="#SkImageInfo_MakeS32">MakeS32</a> <a href="#SkImageInfo_MakeA8">MakeA8</a>

---

<a name="SkImageInfo_Make"></a>
## Make

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static SkImageInfo Make(int width, int height, SkColorType ct, SkAlphaType at,
                        sk_sp&lt;SkColorSpace&gt; cs = nullptr)
</pre>

Creates <a href="#Image_Info">Image Info</a> from integral dimensions width and height, <a href="#Color_Type">Color Type</a> <a href="#SkImageInfo_Make_ct">ct</a>,
<a href="#Alpha_Type">Alpha Type</a> <a href="#SkImageInfo_Make_at">at</a>, and optionally <a href="undocumented#Color_Space">Color Space</a> <a href="#SkImageInfo_Make_cs">cs</a>.

If <a href="undocumented#Color_Space">Color Space</a> <a href="#SkImageInfo_Make_cs">cs</a> is nullptr and <a href="#Image_Info">Image Info</a> is part of drawing source: <a href="undocumented#Color_Space">Color Space</a>
defaults to sRGB, mapping into <a href="SkSurface_Reference#Surface">Surface</a> <a href="undocumented#Color_Space">Color Space</a>.

Parameters are not validated to see if their values are legal, or that the
combination is supported.

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_Make_width"> <code><strong>width </strong></code> </a></td> <td>
pixel column count; must be zero or greater</td>
  </tr>  <tr>    <td><a name="SkImageInfo_Make_height"> <code><strong>height </strong></code> </a></td> <td>
pixel row count; must be zero or greater</td>
  </tr>  <tr>    <td><a name="SkImageInfo_Make_ct"> <code><strong>ct </strong></code> </a></td> <td>
one of: <a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>, <a href="#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>, <a href="#kRGB_565_SkColorType">kRGB_565_SkColorType</a>,
<a href="#kARGB_4444_SkColorType">kARGB_4444_SkColorType</a>, <a href="#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a>, <a href="#kRGB_888x_SkColorType">kRGB_888x_SkColorType</a>,
<a href="#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>, <a href="#kRGBA_1010102_SkColorType">kRGBA_1010102_SkColorType</a>, <a href="#kRGB_101010x_SkColorType">kRGB_101010x_SkColorType</a>,
<a href="#kGray_8_SkColorType">kGray_8_SkColorType</a>, <a href="#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a> </td>
  </tr>  <tr>    <td><a name="SkImageInfo_Make_at"> <code><strong>at </strong></code> </a></td> <td>
one of: <a href="#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a>, <a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>, <a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a>,
<a href="#kUnpremul_SkAlphaType">kUnpremul_SkAlphaType</a> </td>
  </tr>  <tr>    <td><a name="SkImageInfo_Make_cs"> <code><strong>cs </strong></code> </a></td> <td>
range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href="#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="9f47f9c2a99473f5b1113db48096d586"></fiddle-embed></div>

### See Also

<a href="#SkImageInfo_MakeN32">MakeN32</a> <a href="#SkImageInfo_MakeN32Premul">MakeN32Premul</a><sup><a href="#SkImageInfo_MakeN32Premul_2">[2]</a></sup> <a href="#SkImageInfo_MakeS32">MakeS32</a> <a href="#SkImageInfo_MakeA8">MakeA8</a>

---

<a name="SkImageInfo_MakeN32"></a>
## MakeN32

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static SkImageInfo MakeN32(int width, int height, SkAlphaType at, sk_sp&lt;SkColorSpace&gt; cs = nullptr)
</pre>

Creates <a href="#Image_Info">Image Info</a> from integral dimensions width and height, <a href="#kN32_SkColorType">kN32_SkColorType</a>,
<a href="#Alpha_Type">Alpha Type</a> <a href="#SkImageInfo_MakeN32_at">at</a>, and optionally <a href="undocumented#Color_Space">Color Space</a> <a href="#SkImageInfo_MakeN32_cs">cs</a>. <a href="#kN32_SkColorType">kN32_SkColorType</a> will equal either
<a href="#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a> or <a href="#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a>, whichever is optimal.

If <a href="undocumented#Color_Space">Color Space</a> <a href="#SkImageInfo_MakeN32_cs">cs</a> is nullptr and <a href="#Image_Info">Image Info</a> is part of drawing source: <a href="undocumented#Color_Space">Color Space</a>
defaults to sRGB, mapping into <a href="SkSurface_Reference#Surface">Surface</a> <a href="undocumented#Color_Space">Color Space</a>.

Parameters are not validated to see if their values are legal, or that the
combination is supported.

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_MakeN32_width"> <code><strong>width </strong></code> </a></td> <td>
pixel column count; must be zero or greater</td>
  </tr>  <tr>    <td><a name="SkImageInfo_MakeN32_height"> <code><strong>height </strong></code> </a></td> <td>
pixel row count; must be zero or greater</td>
  </tr>  <tr>    <td><a name="SkImageInfo_MakeN32_at"> <code><strong>at </strong></code> </a></td> <td>
one of: <a href="#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a>, <a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>, <a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a>,
<a href="#kUnpremul_SkAlphaType">kUnpremul_SkAlphaType</a> </td>
  </tr>  <tr>    <td><a name="SkImageInfo_MakeN32_cs"> <code><strong>cs </strong></code> </a></td> <td>
range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href="#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="78cea0c4cac205b61ad6f6c982cbd888"></fiddle-embed></div>

### See Also

<a href="#SkImageInfo_Make">Make</a> <a href="#SkImageInfo_MakeN32Premul">MakeN32Premul</a><sup><a href="#SkImageInfo_MakeN32Premul_2">[2]</a></sup> <a href="#SkImageInfo_MakeS32">MakeS32</a> <a href="#SkImageInfo_MakeA8">MakeA8</a>

---

<a name="SkImageInfo_MakeS32"></a>
## MakeS32

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static SkImageInfo MakeS32(int width, int height, SkAlphaType at)
</pre>

Creates <a href="#Image_Info">Image Info</a> from integral dimensions width and height, <a href="#kN32_SkColorType">kN32_SkColorType</a>,
<a href="#Alpha_Type">Alpha Type</a> <a href="#SkImageInfo_MakeS32_at">at</a>, with sRGB <a href="undocumented#Color_Space">Color Space</a>.

Parameters are not validated to see if their values are legal, or that the
combination is supported.

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_MakeS32_width"> <code><strong>width </strong></code> </a></td> <td>
pixel column count; must be zero or greater</td>
  </tr>  <tr>    <td><a name="SkImageInfo_MakeS32_height"> <code><strong>height </strong></code> </a></td> <td>
pixel row count; must be zero or greater</td>
  </tr>  <tr>    <td><a name="SkImageInfo_MakeS32_at"> <code><strong>at </strong></code> </a></td> <td>
one of: <a href="#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a>, <a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>, <a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a>,
<a href="#kUnpremul_SkAlphaType">kUnpremul_SkAlphaType</a> </td>
  </tr>
</table>

### Return Value

created <a href="#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="bb85fc8a82fe772e1d611f3ab8770a1d"><div>Top gradient is drawn to offscreen without <a href="undocumented#Color_Space">Color Space</a>. It is darker than middle
gradient, drawn to offscreen with sRGB <a href="undocumented#Color_Space">Color Space</a>. Bottom gradient shares bits
with middle, but does not specify the <a href="undocumented#Color_Space">Color Space</a> in noColorSpaceBitmap. A source
without <a href="undocumented#Color_Space">Color Space</a> is treated as sRGB; the bottom gradient is identical to the
middle gradient.
</div></fiddle-embed></div>

### See Also

<a href="#SkImageInfo_Make">Make</a> <a href="#SkImageInfo_MakeN32">MakeN32</a> <a href="#SkImageInfo_MakeN32Premul">MakeN32Premul</a><sup><a href="#SkImageInfo_MakeN32Premul_2">[2]</a></sup> <a href="#SkImageInfo_MakeA8">MakeA8</a>

---

<a name="SkImageInfo_MakeN32Premul"></a>
## MakeN32Premul

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static SkImageInfo MakeN32Premul(int width, int height, sk_sp&lt;SkColorSpace&gt; cs = nullptr)
</pre>

Creates <a href="#Image_Info">Image Info</a> from integral dimensions width and height, <a href="#kN32_SkColorType">kN32_SkColorType</a>,
<a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a>, with optional <a href="undocumented#Color_Space">Color Space</a>.

If <a href="undocumented#Color_Space">Color Space</a> <a href="#SkImageInfo_MakeN32Premul_cs">cs</a> is nullptr and <a href="#Image_Info">Image Info</a> is part of drawing source: <a href="undocumented#Color_Space">Color Space</a>
defaults to sRGB, mapping into <a href="SkSurface_Reference#Surface">Surface</a> <a href="undocumented#Color_Space">Color Space</a>.

Parameters are not validated to see if their values are legal, or that the
combination is supported.

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_MakeN32Premul_width"> <code><strong>width </strong></code> </a></td> <td>
pixel column count; must be zero or greater</td>
  </tr>  <tr>    <td><a name="SkImageInfo_MakeN32Premul_height"> <code><strong>height </strong></code> </a></td> <td>
pixel row count; must be zero or greater</td>
  </tr>  <tr>    <td><a name="SkImageInfo_MakeN32Premul_cs"> <code><strong>cs </strong></code> </a></td> <td>
range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href="#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="525650a67e19fdd8ca9f72b7eda65174"></fiddle-embed></div>

### See Also

<a href="#SkImageInfo_MakeN32">MakeN32</a> <a href="#SkImageInfo_MakeS32">MakeS32</a> <a href="#SkImageInfo_MakeA8">MakeA8</a> <a href="#SkImageInfo_Make">Make</a>

---

<a name="SkImageInfo_MakeN32Premul_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static SkImageInfo MakeN32Premul(const SkISize& size)
</pre>

Creates <a href="#Image_Info">Image Info</a> from integral dimensions width and height, <a href="#kN32_SkColorType">kN32_SkColorType</a>,
<a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a>, with <a href="undocumented#Color_Space">Color Space</a> set to nullptr.

If <a href="#Image_Info">Image Info</a> is part of drawing source: <a href="undocumented#Color_Space">Color Space</a> defaults to sRGB, mapping
into <a href="SkSurface_Reference#Surface">Surface</a> <a href="undocumented#Color_Space">Color Space</a>.

Parameters are not validated to see if their values are legal, or that the
combination is supported.

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_MakeN32Premul_2_size"> <code><strong>size </strong></code> </a></td> <td>
width and height, each must be zero or greater</td>
  </tr>
</table>

### Return Value

created <a href="#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="b9026d7f39029756bd7cab9542c64f4e"></fiddle-embed></div>

### See Also

<a href="#SkImageInfo_MakeN32">MakeN32</a> <a href="#SkImageInfo_MakeS32">MakeS32</a> <a href="#SkImageInfo_MakeA8">MakeA8</a> <a href="#SkImageInfo_Make">Make</a>

---

<a name="SkImageInfo_MakeA8"></a>
## MakeA8

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static SkImageInfo MakeA8(int width, int height)
</pre>

Creates <a href="#Image_Info">Image Info</a> from integral dimensions width and height, <a href="#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>,
<a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a>, with <a href="undocumented#Color_Space">Color Space</a> set to nullptr.

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_MakeA8_width"> <code><strong>width </strong></code> </a></td> <td>
pixel column count; must be zero or greater</td>
  </tr>  <tr>    <td><a name="SkImageInfo_MakeA8_height"> <code><strong>height </strong></code> </a></td> <td>
pixel row count; must be zero or greater</td>
  </tr>
</table>

### Return Value

created <a href="#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="547388991687b8e10d482d8b1c82777d"></fiddle-embed></div>

### See Also

<a href="#SkImageInfo_MakeN32">MakeN32</a> <a href="#SkImageInfo_MakeS32">MakeS32</a> <a href="#SkImageInfo_Make">Make</a>

---

<a name="SkImageInfo_MakeUnknown"></a>
## MakeUnknown

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static SkImageInfo MakeUnknown(int width, int height)
</pre>

Creates <a href="#Image_Info">Image Info</a> from integral dimensions width and height, <a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>,
<a href="#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a>, with <a href="undocumented#Color_Space">Color Space</a> set to nullptr.

Returned <a href="#Image_Info">Image Info</a> as part of source does not draw, and as part of destination
can not be drawn to.

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_MakeUnknown_width"> <code><strong>width </strong></code> </a></td> <td>
pixel column count; must be zero or greater</td>
  </tr>  <tr>    <td><a name="SkImageInfo_MakeUnknown_height"> <code><strong>height </strong></code> </a></td> <td>
pixel row count; must be zero or greater</td>
  </tr>
</table>

### Return Value

created <a href="#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="75f13a78b28b08c72baf32b7d868de1c"></fiddle-embed></div>

### See Also

<a href="#SkImageInfo_empty_constructor">SkImageInfo()</a> <a href="#SkImageInfo_MakeN32">MakeN32</a> <a href="#SkImageInfo_MakeS32">MakeS32</a> <a href="#SkImageInfo_Make">Make</a>

---

<a name="SkImageInfo_MakeUnknown_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static SkImageInfo MakeUnknown()
</pre>

Creates <a href="#Image_Info">Image Info</a> from integral dimensions width and height set to zero,
<a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>, <a href="#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a>, with <a href="undocumented#Color_Space">Color Space</a> set to nullptr.

Returned <a href="#Image_Info">Image Info</a> as part of source does not draw, and as part of destination
can not be drawn to.

### Return Value

created <a href="#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="a1af7696ae0cdd6f379546dd1f211b7a"></fiddle-embed></div>

### See Also

<a href="#SkImageInfo_empty_constructor">SkImageInfo()</a> <a href="#SkImageInfo_MakeN32">MakeN32</a> <a href="#SkImageInfo_MakeS32">MakeS32</a> <a href="#SkImageInfo_Make">Make</a>

---

## <a name="Property"></a> Property

| name | description |
| --- | --- |
| <a href="#SkImageInfo_alphaType">alphaType</a> | Returns <a href="#Alpha_Type">Alpha Type</a> |
| <a href="#SkImageInfo_bounds">bounds</a> | returns <a href="#SkImageInfo_width">width</a> and <a href="#SkImageInfo_height">height</a> as Rectangle |
| <a href="#SkImageInfo_bytesPerPixel">bytesPerPixel</a> | returns number of bytes in pixel based on <a href="#Color_Type">Color Type</a> |
| <a href="#SkImageInfo_colorSpace">colorSpace</a> | returns <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkImageInfo_colorType">colorType</a> | returns <a href="#Color_Type">Color Type</a> |
| <a href="#SkImageInfo_dimensions">dimensions</a> | returns <a href="#SkImageInfo_width">width</a> and <a href="#SkImageInfo_height">height</a> |
| <a href="#SkImageInfo_gammaCloseToSRGB">gammaCloseToSRGB</a> | Returns if <a href="undocumented#Color_Space">Color Space</a> gamma is approximately the same as sRGB |
| <a href="#SkImageInfo_height">height</a> | returns pixel row count |
| <a href="#SkImageInfo_isEmpty">isEmpty</a> | returns if dimensions contain pixels |
| <a href="#SkImageInfo_isOpaque">isOpaque</a> | returns if <a href="#Alpha_Type">Alpha Type</a> is <a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a> |
| <a href="#SkImageInfo_minRowBytes">minRowBytes</a> | returns <a href="#SkImageInfo_width">width</a> times <a href="#SkImageInfo_bytesPerPixel">bytesPerPixel</a> in 32 bits |
| <a href="#SkImageInfo_minRowBytes64">minRowBytes64</a> | returns <a href="#SkImageInfo_width">width</a> times <a href="#SkImageInfo_bytesPerPixel">bytesPerPixel</a> in 64 bits |
| <a href="#SkImageInfo_refColorSpace">refColorSpace</a> | returns <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkImageInfo_shiftPerPixel">shiftPerPixel</a> | returns bit shift from pixels to bytes |
| <a href="#SkImageInfo_width">width</a> | returns pixel column count |

<a name="SkImageInfo_width"></a>
## width

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int width() const
</pre>

Returns pixel count in each row.

### Return Value

pixel width

### Example

<div><fiddle-embed name="588a0a763d78c1b3b3ea0b2a6e39fda6"></fiddle-embed></div>

### See Also

<a href="#SkImageInfo_height">height</a> <a href="SkBitmap_Reference#SkBitmap_width">SkBitmap::width</a> <a href="undocumented#SkPixelRef_width">SkPixelRef::width</a> <a href="SkImage_Reference#SkImage_width">SkImage::width</a> <a href="SkSurface_Reference#SkSurface_width">SkSurface::width</a>

---

<a name="SkImageInfo_height"></a>
## height

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int height() const
</pre>

Returns pixel row count.

### Return Value

pixel height

### Example

<div><fiddle-embed name="1719751fef7fd6040447619d4e66d416"></fiddle-embed></div>

### See Also

<a href="#SkImageInfo_width">width</a> <a href="SkBitmap_Reference#SkBitmap_height">SkBitmap::height</a> <a href="undocumented#SkPixelRef_height">SkPixelRef::height</a> <a href="SkImage_Reference#SkImage_height">SkImage::height</a> <a href="SkSurface_Reference#SkSurface_height">SkSurface::height</a>

---

<a name="SkImageInfo_colorType"></a>
## colorType

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkColorType colorType() const
</pre>

Returns <a href="#Color_Type">Color Type</a>, one of: <a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>, <a href="#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>, <a href="#kRGB_565_SkColorType">kRGB_565_SkColorType</a>,
<a href="#kARGB_4444_SkColorType">kARGB_4444_SkColorType</a>, <a href="#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a>, <a href="#kRGB_888x_SkColorType">kRGB_888x_SkColorType</a>,
<a href="#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>, <a href="#kRGBA_1010102_SkColorType">kRGBA_1010102_SkColorType</a>, <a href="#kRGB_101010x_SkColorType">kRGB_101010x_SkColorType</a>,
<a href="#kGray_8_SkColorType">kGray_8_SkColorType</a>, <a href="#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a>.

### Return Value

<a href="#Color_Type">Color Type</a>

### Example

<div><fiddle-embed name="06ecc3ce7f35cc7f930cbc2a662e3105">

#### Example Output

~~~~
color type: kAlpha_8_SkColorType
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkImageInfo_alphaType">alphaType</a> <a href="SkPixmap_Reference#SkPixmap_colorType">SkPixmap::colorType</a> <a href="SkBitmap_Reference#SkBitmap_colorType">SkBitmap::colorType</a>

---

<a name="SkImageInfo_alphaType"></a>
## alphaType

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkAlphaType alphaType() const
</pre>

Returns <a href="#Alpha_Type">Alpha Type</a>, one of: <a href="#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a>, <a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>, <a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a>,
<a href="#kUnpremul_SkAlphaType">kUnpremul_SkAlphaType</a>.

### Return Value

<a href="#Alpha_Type">Alpha Type</a>

### Example

<div><fiddle-embed name="5c1d2499a4056b6cff38c1cf924158a1">

#### Example Output

~~~~
alpha type: kPremul_SkAlphaType
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkImageInfo_colorType">colorType</a> <a href="SkPixmap_Reference#SkPixmap_alphaType">SkPixmap::alphaType</a> <a href="SkBitmap_Reference#SkBitmap_alphaType">SkBitmap::alphaType</a>

---

<a name="SkImageInfo_colorSpace"></a>
## colorSpace

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkColorSpace* colorSpace() const
</pre>

Returns <a href="undocumented#Color_Space">Color Space</a>, the range of colors. The reference count of
<a href="undocumented#Color_Space">Color Space</a> is unchanged. The returned <a href="undocumented#Color_Space">Color Space</a> is immutable.

### Return Value

<a href="undocumented#Color_Space">Color Space</a>, or nullptr

### Example

<div><fiddle-embed name="2952a6c863bd504484c9f66cc727f968"><div><a href="undocumented#SkColorSpace_MakeSRGBLinear">SkColorSpace::MakeSRGBLinear</a> creates <a href="undocumented#Color_Space">Color Space</a> with linear gamma
and an sRGB gamut. This <a href="undocumented#Color_Space">Color Space</a> gamma is not close to sRGB gamma.
</div>

#### Example Output

~~~~
gammaCloseToSRGB: false  gammaIsLinear: true  isSRGB: false
~~~~

</fiddle-embed></div>

### See Also

<a href="undocumented#Color_Space">Color Space</a> <a href="SkPixmap_Reference#SkPixmap_colorSpace">SkPixmap::colorSpace</a> <a href="SkBitmap_Reference#SkBitmap_colorSpace">SkBitmap::colorSpace</a>

---

<a name="SkImageInfo_refColorSpace"></a>
## refColorSpace

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkColorSpace&gt; refColorSpace() const
</pre>

Returns smart pointer to <a href="undocumented#Color_Space">Color Space</a>, the range of colors. The smart pointer
tracks the number of objects sharing this <a href="undocumented#Color_Space">Color Space</a> reference so the memory
is released when the owners destruct.

The returned <a href="undocumented#Color_Space">Color Space</a> is immutable.

### Return Value

<a href="undocumented#Color_Space">Color Space</a> wrapped in a smart pointer

### Example

<div><fiddle-embed name="f36afcc295602f5c8c4c2c2025c25884"></fiddle-embed></div>

### See Also

<a href="undocumented#Color_Space">Color Space</a> <a href="SkBitmap_Reference#SkBitmap_refColorSpace">SkBitmap::refColorSpace</a>

---

<a name="SkImageInfo_isEmpty"></a>
## isEmpty

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isEmpty() const
</pre>

Returns if <a href="#Image_Info">Image Info</a> describes an empty area of pixels by checking if either
width or height is zero or smaller.

### Return Value

true if either dimension is zero or smaller

### Example

<div><fiddle-embed name="b8757200da5be0b43763cf79feb681a7">

#### Example Output

~~~~
width: 0 height: 0 empty: true
width: 0 height: 2 empty: true
width: 2 height: 0 empty: true
width: 2 height: 2 empty: false
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkImageInfo_dimensions">dimensions</a> <a href="#SkImageInfo_bounds">bounds</a> <a href="SkBitmap_Reference#SkBitmap_empty">SkBitmap::empty</a> <a href="SkPixmap_Reference#SkPixmap_bounds">SkPixmap::bounds</a>

---

<a name="SkImageInfo_isOpaque"></a>
## isOpaque

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isOpaque() const
</pre>

Returns true if <a href="#Alpha_Type">Alpha Type</a> is set to hint that all pixels are opaque; their
<a href="undocumented#Alpha">Color Alpha</a> value is implicitly or explicitly 1.0. If true, and all pixels are
not opaque, Skia may draw incorrectly.

Does not check if <a href="#Color_Type">Color Type</a> allows <a href="undocumented#Alpha">Alpha</a>, or if any pixel value has
transparency.

### Return Value

true if <a href="#Alpha_Type">Alpha Type</a> is <a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>

### Example

<div><fiddle-embed name="e9bd4f02b6cfb3ac864cb7fee7d7299c">

#### Example Output

~~~~
isOpaque: false
isOpaque: false
isOpaque: true
isOpaque: true
~~~~

</fiddle-embed></div>

### See Also

<a href="undocumented#Alpha">Color Alpha</a> <a href="#SkColorTypeValidateAlphaType">SkColorTypeValidateAlphaType</a> <a href="SkBitmap_Reference#SkBitmap_isOpaque">SkBitmap::isOpaque</a> <a href="SkImage_Reference#SkImage_isOpaque">SkImage::isOpaque</a> <a href="SkPixmap_Reference#SkPixmap_isOpaque">SkPixmap::isOpaque</a>

---

<a name="SkImageInfo_dimensions"></a>
## dimensions

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkISize dimensions() const
</pre>

Returns <a href="undocumented#ISize">ISize</a> { <a href="#SkImageInfo_width">width</a>, <a href="#SkImageInfo_height">height</a> }.

### Return Value

integral size of <a href="#SkImageInfo_width">width</a> and <a href="#SkImageInfo_height">height</a>

### Example

<div><fiddle-embed name="d5547cd2b302822aa85b7b0ae3f48458">

#### Example Output

~~~~
dimensionsAsBounds == bounds
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkImageInfo_width">width</a> <a href="#SkImageInfo_height">height</a> <a href="#SkImageInfo_bounds">bounds</a> <a href="SkBitmap_Reference#SkBitmap_dimensions">SkBitmap::dimensions</a>

---

<a name="SkImageInfo_bounds"></a>
## bounds

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkIRect bounds() const
</pre>

Returns <a href="SkIRect_Reference#IRect">IRect</a> { 0, 0, <a href="#SkImageInfo_width">width</a>, <a href="#SkImageInfo_height">height</a> }.

### Return Value

integral rectangle from origin to <a href="#SkImageInfo_width">width</a> and <a href="#SkImageInfo_height">height</a>

### Example

<div><fiddle-embed name="a818be8945cd0c18f99ffe53e90afa48"></fiddle-embed></div>

### See Also

<a href="#SkImageInfo_width">width</a> <a href="#SkImageInfo_height">height</a> <a href="#SkImageInfo_dimensions">dimensions</a>

---

<a name="SkImageInfo_gammaCloseToSRGB"></a>
## gammaCloseToSRGB

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool gammaCloseToSRGB() const
</pre>

Returns true if associated <a href="undocumented#Color_Space">Color Space</a> is not nullptr, and <a href="undocumented#Color_Space">Color Space</a> gamma
is approximately the same as sRGB.
This includes the <a href="">sRGB transfer function $ https://en.wikipedia.org/wiki/SRGB#The_sRGB_transfer_function_(%22gamma%22)</a> as well as a gamma curve described by a 2.2 exponent.

### Return Value

true if <a href="undocumented#Color_Space">Color Space</a> gamma is approximately the same as sRGB

### Example

<div><fiddle-embed name="dcdc308a1a2089db47b8375178491832"></fiddle-embed></div>

### See Also

<a href="undocumented#SkColorSpace_gammaCloseToSRGB">SkColorSpace::gammaCloseToSRGB</a>

---

<a name="SkImageInfo_makeWH"></a>
## makeWH

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkImageInfo makeWH(int newWidth, int newHeight) const
</pre>

Creates <a href="#Image_Info">Image Info</a> with the same <a href="#Color_Type">Color Type</a>, <a href="undocumented#Color_Space">Color Space</a>, and <a href="#Alpha_Type">Alpha Type</a>,
with dimensions set to width and height.

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_makeWH_newWidth"> <code><strong>newWidth </strong></code> </a></td> <td>
pixel column count; must be zero or greater</td>
  </tr>  <tr>    <td><a name="SkImageInfo_makeWH_newHeight"> <code><strong>newHeight </strong></code> </a></td> <td>
pixel row count; must be zero or greater</td>
  </tr>
</table>

### Return Value

created <a href="#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="26827898b6b199d6c4b5e4d2c6e6bac8"></fiddle-embed></div>

### See Also

<a href="#SkImageInfo_Make">Make</a> <a href="#SkImageInfo_makeAlphaType">makeAlphaType</a> <a href="#SkImageInfo_makeColorSpace">makeColorSpace</a> <a href="#SkImageInfo_makeColorType">makeColorType</a>

---

<a name="SkImageInfo_makeAlphaType"></a>
## makeAlphaType

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkImageInfo makeAlphaType(SkAlphaType newAlphaType) const
</pre>

Creates <a href="#Image_Info">Image Info</a> with same <a href="#Color_Type">Color Type</a>, <a href="undocumented#Color_Space">Color Space</a>, width, and height,
with <a href="#Alpha_Type">Alpha Type</a> set to <a href="#SkImageInfo_makeAlphaType_newAlphaType">newAlphaType</a>.

Created <a href="#Image_Info">Image Info</a> contains <a href="#SkImageInfo_makeAlphaType_newAlphaType">newAlphaType</a> even if it is incompatible with
<a href="#Color_Type">Color Type</a>, in which case <a href="#Alpha_Type">Alpha Type</a> in <a href="#Image_Info">Image Info</a> is ignored.

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_makeAlphaType_newAlphaType"> <code><strong>newAlphaType </strong></code> </a></td> <td>
one of: <a href="#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a>, <a href="#kOpaque_SkAlphaType">kOpaque_SkAlphaType</a>, <a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a>,
<a href="#kUnpremul_SkAlphaType">kUnpremul_SkAlphaType</a> </td>
  </tr>
</table>

### Return Value

created <a href="#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="5166f1a04d53443ed8aed519d0faa3db"></fiddle-embed></div>

### See Also

<a href="#SkImageInfo_Make">Make</a> <a href="#SkImageInfo_MakeA8">MakeA8</a> <a href="#SkImageInfo_makeColorType">makeColorType</a> <a href="#SkImageInfo_makeColorSpace">makeColorSpace</a>

---

<a name="SkImageInfo_makeColorType"></a>
## makeColorType

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkImageInfo makeColorType(SkColorType newColorType) const
</pre>

Creates <a href="#Image_Info">Image Info</a> with same <a href="#Alpha_Type">Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a>, width, and height,
with <a href="#Color_Type">Color Type</a> set to <a href="#SkImageInfo_makeColorType_newColorType">newColorType</a>.

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_makeColorType_newColorType"> <code><strong>newColorType </strong></code> </a></td> <td>
one of: <a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>, <a href="#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>, <a href="#kRGB_565_SkColorType">kRGB_565_SkColorType</a>,
<a href="#kARGB_4444_SkColorType">kARGB_4444_SkColorType</a>, <a href="#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a>, <a href="#kRGB_888x_SkColorType">kRGB_888x_SkColorType</a>,
<a href="#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>, <a href="#kRGBA_1010102_SkColorType">kRGBA_1010102_SkColorType</a>, <a href="#kRGB_101010x_SkColorType">kRGB_101010x_SkColorType</a>,
<a href="#kGray_8_SkColorType">kGray_8_SkColorType</a>, <a href="#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a> </td>
  </tr>
</table>

### Return Value

created <a href="#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="0d67609fbf0988bfaf9ca5e2460af3d3"></fiddle-embed></div>

### See Also

<a href="#SkImageInfo_Make">Make</a> <a href="#SkImageInfo_makeAlphaType">makeAlphaType</a> <a href="#SkImageInfo_makeColorSpace">makeColorSpace</a>

---

<a name="SkImageInfo_makeColorSpace"></a>
## makeColorSpace

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkImageInfo makeColorSpace(sk_sp&lt;SkColorSpace&gt; cs) const
</pre>

Creates <a href="#Image_Info">Image Info</a> with same <a href="#Alpha_Type">Alpha Type</a>, <a href="#Color_Type">Color Type</a>, width, and height,
with <a href="undocumented#Color_Space">Color Space</a> set to <a href="#SkImageInfo_makeColorSpace_cs">cs</a>.

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_makeColorSpace_cs"> <code><strong>cs </strong></code> </a></td> <td>
range of colors; may be nullptr</td>
  </tr>
</table>

### Return Value

created <a href="#Image_Info">Image Info</a>

### Example

<div><fiddle-embed name="454add968099811053e2b372238472e3"></fiddle-embed></div>

### See Also

<a href="#SkImageInfo_Make">Make</a> <a href="#SkImageInfo_MakeS32">MakeS32</a> <a href="#SkImageInfo_makeAlphaType">makeAlphaType</a> <a href="#SkImageInfo_makeColorType">makeColorType</a>

---

<a name="SkImageInfo_bytesPerPixel"></a>
## bytesPerPixel

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int bytesPerPixel() const
</pre>

Returns number of bytes per pixel required by <a href="#Color_Type">Color Type</a>.
Returns zero if <a href="#SkImageInfo_colorType">colorType</a>( is <a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>.

### Return Value

bytes in pixel

### Example

<div><fiddle-embed name="9b6de4a07b2316228e9340e5a3b82134"><a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>, <a href="#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>, <a href="#kRGB_565_SkColorType">kRGB_565_SkColorType</a>,
<a href="#kARGB_4444_SkColorType">kARGB_4444_SkColorType</a>, <a href="#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a>, <a href="#kRGB_888x_SkColorType">kRGB_888x_SkColorType</a>,
<a href="#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>, <a href="#kRGBA_1010102_SkColorType">kRGBA_1010102_SkColorType</a>, <a href="#kRGB_101010x_SkColorType">kRGB_101010x_SkColorType</a>,
<a href="#kGray_8_SkColorType">kGray_8_SkColorType</a>, <a href="#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a>

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

<a href="#SkImageInfo_width">width</a> <a href="#SkImageInfo_shiftPerPixel">shiftPerPixel</a> <a href="SkBitmap_Reference#SkBitmap_bytesPerPixel">SkBitmap::bytesPerPixel</a>

---

<a name="SkImageInfo_shiftPerPixel"></a>
## shiftPerPixel

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int shiftPerPixel() const
</pre>

Returns bit shift converting row bytes to row pixels.
Returns zero for <a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>.

### Return Value

one of: 0, 1, 2, 3; left shift to convert pixels to bytes

### Example

<div><fiddle-embed name="e47b911f94fc629f756a829e523a2a89"><a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>, <a href="#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>, <a href="#kRGB_565_SkColorType">kRGB_565_SkColorType</a>,
<a href="#kARGB_4444_SkColorType">kARGB_4444_SkColorType</a>, <a href="#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a>, <a href="#kRGB_888x_SkColorType">kRGB_888x_SkColorType</a>,
<a href="#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>, <a href="#kRGBA_1010102_SkColorType">kRGBA_1010102_SkColorType</a>, <a href="#kRGB_101010x_SkColorType">kRGB_101010x_SkColorType</a>,
<a href="#kGray_8_SkColorType">kGray_8_SkColorType</a>, <a href="#kRGBA_F16_SkColorType">kRGBA_F16_SkColorType</a>

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

<a href="#SkImageInfo_bytesPerPixel">bytesPerPixel</a> <a href="#SkImageInfo_minRowBytes">minRowBytes</a> <a href="SkBitmap_Reference#SkBitmap_shiftPerPixel">SkBitmap::shiftPerPixel</a> <a href="SkPixmap_Reference#SkPixmap_shiftPerPixel">SkPixmap::shiftPerPixel</a>

---

<a name="SkImageInfo_minRowBytes64"></a>
## minRowBytes64

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
uint64_t minRowBytes64() const
</pre>

Returns minimum bytes per row, computed from pixel <a href="#SkImageInfo_width">width</a> and <a href="#Color_Type">Color Type</a>, which
specifies <a href="#SkImageInfo_bytesPerPixel">bytesPerPixel</a>. <a href="SkBitmap_Reference#Bitmap">Bitmap</a> maximum value for row bytes must be representable
as a positive value in a 32-bit signed integer.

### Return Value

<a href="#SkImageInfo_width">width</a> times <a href="#SkImageInfo_bytesPerPixel">bytesPerPixel</a> as unsigned 64-bit integer

### Example

<div><fiddle-embed name="3004125e67431bd7a5c0ff3863aad8a0">

#### Example Output

~~~~
RGBA_F16 width 16777216 (0x01000000) OK
RGBA_F16 width 33554432 (0x02000000) OK
RGBA_F16 width 67108864 (0x04000000) OK
RGBA_F16 width 134217728 (0x08000000) OK
RGBA_F16 width 268435456 (0x10000000) too large
RGBA_F16 width 536870912 (0x20000000) too large
RGBA_F16 width 1073741824 (0x40000000) too large
RGBA_F16 width -2147483648 (0x80000000) too large
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkImageInfo_minRowBytes">minRowBytes</a> <a href="#SkImageInfo_computeByteSize">computeByteSize</a> <a href="#SkImageInfo_computeMinByteSize">computeMinByteSize</a> <a href="#SkImageInfo_validRowBytes">validRowBytes</a>

---

<a name="SkImageInfo_minRowBytes"></a>
## minRowBytes

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
size_t minRowBytes() const
</pre>

Returns minimum bytes per row, computed from pixel <a href="#SkImageInfo_width">width</a> and <a href="#Color_Type">Color Type</a>, which
specifies <a href="#SkImageInfo_bytesPerPixel">bytesPerPixel</a>. <a href="SkBitmap_Reference#Bitmap">Bitmap</a> maximum value for row bytes must be representable
as a positive value in a 32-bit signed integer.

### Return Value

<a href="#SkImageInfo_width">width</a> times <a href="#SkImageInfo_bytesPerPixel">bytesPerPixel</a> as signed 32-bit integer

### Example

<div><fiddle-embed name="4cb6975732a7ffab2c9ebac31af1432e">

#### Example Output

~~~~
RGBA_F16 width 16777216 (0x01000000) OK
RGBA_F16 width 33554432 (0x02000000) OK
RGBA_F16 width 67108864 (0x04000000) OK
RGBA_F16 width 134217728 (0x08000000) OK
RGBA_F16 width 268435456 (0x10000000) too large
RGBA_F16 width 536870912 (0x20000000) too large
RGBA_F16 width 1073741824 (0x40000000) too large
RGBA_F16 width -2147483648 (0x80000000) too large
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkImageInfo_minRowBytes64">minRowBytes64</a> <a href="#SkImageInfo_computeByteSize">computeByteSize</a> <a href="#SkImageInfo_computeMinByteSize">computeMinByteSize</a> <a href="#SkImageInfo_validRowBytes">validRowBytes</a>

---

<a name="SkImageInfo_computeOffset"></a>
## computeOffset

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
size_t computeOffset(int x, int y, size_t rowBytes) const
</pre>

Returns byte offset of pixel from pixel base address.

Asserts in debug build if <a href="#SkImageInfo_computeOffset_x">x</a> or <a href="#SkImageInfo_computeOffset_y">y</a> is outside of bounds. Does not assert if
<a href="#SkImageInfo_computeOffset_rowBytes">rowBytes</a> is smaller than <a href="#SkImageInfo_minRowBytes">minRowBytes</a>, even though result may be incorrect.

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_computeOffset_x"> <code><strong>x </strong></code> </a></td> <td>
column index, zero or greater, and less than <a href="#SkImageInfo_width">width</a></td>
  </tr>  <tr>    <td><a name="SkImageInfo_computeOffset_y"> <code><strong>y </strong></code> </a></td> <td>
row index, zero or greater, and less than <a href="#SkImageInfo_height">height</a></td>
  </tr>  <tr>    <td><a name="SkImageInfo_computeOffset_rowBytes"> <code><strong>rowBytes </strong></code> </a></td> <td>
size of pixel row or larger</td>
  </tr>
</table>

### Return Value

offset within pixel array

### Example

<div><fiddle-embed name="818e4e1191e39d2a642902cbf253b399"></fiddle-embed></div>

### See Also

<a href="#SkImageInfo_height">height</a> <a href="#SkImageInfo_width">width</a> <a href="#SkImageInfo_minRowBytes">minRowBytes</a> <a href="#SkImageInfo_computeByteSize">computeByteSize</a>

---

## <a name="Operator"></a> Operator

| name | description |
| --- | --- |
| <a href="#SkImageInfo_notequal1_operator">operator!=(const SkImageInfo& other) const</a> | compares <a href="#Image_Info">Image Info</a> for inequality |
| <a href="#SkImageInfo_equal1_operator">operator==(const SkImageInfo& other) const</a> | compares <a href="#Image_Info">Image Info</a> for equality |

<a name="SkImageInfo_equal1_operator"></a>
## operator==

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool operator==(const SkImageInfo& other) _const
</pre>

Compares <a href="#Image_Info">Image Info</a> with <a href="#SkImageInfo_equal1_operator_other">other</a>, and returns true if width, height, <a href="#Color_Type">Color Type</a>,
<a href="#Alpha_Type">Alpha Type</a>, and <a href="undocumented#Color_Space">Color Space</a> are equivalent.

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_equal1_operator_other"> <code><strong>other </strong></code> </a></td> <td>
<a href="#Image_Info">Image Info</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href="#Image_Info">Image Info</a> equals <a href="#SkImageInfo_equal1_operator_other">other</a>

### Example

<div><fiddle-embed name="53c212c4f2449df0b0eedbc6227b6ab7">

#### Example Output

~~~~
info1 != info2
info1 != info2
info1 != info2
info1 == info2
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkImageInfo_notequal1_operator">operator!=(const SkImageInfo& other) const</a> <a href="undocumented#SkColorSpace_Equals">SkColorSpace::Equals</a>

---

<a name="SkImageInfo_notequal1_operator"></a>
## operator!=

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool operator!=(const SkImageInfo& other) _const
</pre>

Compares <a href="#Image_Info">Image Info</a> with <a href="#SkImageInfo_notequal1_operator_other">other</a>, and returns true if width, height, <a href="#Color_Type">Color Type</a>,
<a href="#Alpha_Type">Alpha Type</a>, and <a href="undocumented#Color_Space">Color Space</a> are equivalent.

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_notequal1_operator_other"> <code><strong>other </strong></code> </a></td> <td>
<a href="#Image_Info">Image Info</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href="#Image_Info">Image Info</a> is not equal to <a href="#SkImageInfo_notequal1_operator_other">other</a>

### Example

<div><fiddle-embed name="8c039fde0a476ac1aa62bf9de5d61c77">

#### Example Output

~~~~
info1 != info2
info1 != info2
info1 != info2
info1 == info2
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkImageInfo_equal1_operator">operator==(const SkImageInfo& other) const</a> <a href="undocumented#SkColorSpace_Equals">SkColorSpace::Equals</a>

---

<a name="SkImageInfo_computeByteSize"></a>
## computeByteSize

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
size_t computeByteSize(size_t rowBytes) const
</pre>

Returns storage required by pixel array, given <a href="#Image_Info">Image Info</a> dimensions, <a href="#Color_Type">Color Type</a>,
and <a href="#SkImageInfo_computeByteSize_rowBytes">rowBytes</a>. <a href="#SkImageInfo_computeByteSize_rowBytes">rowBytes</a> is assumed to be at least as large as <a href="#SkImageInfo_minRowBytes">minRowBytes</a>.

Returns zero if height is zero.
Returns <a href="undocumented#SK_MaxSizeT">SK MaxSizeT</a> if answer exceeds the range of size_t.

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_computeByteSize_rowBytes"> <code><strong>rowBytes </strong></code> </a></td> <td>
size of pixel row or larger</td>
  </tr>
</table>

### Return Value

memory required by pixel buffer

### Example

<div><fiddle-embed name="9def507d2295f7051effd0c83bb04436"></fiddle-embed></div>

### See Also

<a href="#SkImageInfo_computeMinByteSize">computeMinByteSize</a> <a href="#SkImageInfo_validRowBytes">validRowBytes</a>

---

<a name="SkImageInfo_computeMinByteSize"></a>
## computeMinByteSize

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
size_t computeMinByteSize() const
</pre>

Returns storage required by pixel array, given <a href="#Image_Info">Image Info</a> dimensions, and
<a href="#Color_Type">Color Type</a>. Uses <a href="#SkImageInfo_minRowBytes">minRowBytes</a> to compute bytes for pixel row.

Returns zero if height is zero.
Returns <a href="undocumented#SK_MaxSizeT">SK MaxSizeT</a> if answer exceeds the range of size_t.

### Return Value

least memory required by pixel buffer

### Example

<div><fiddle-embed name="fc18640fdde437cb35338aed7c68d399"></fiddle-embed></div>

### See Also

<a href="#SkImageInfo_computeByteSize">computeByteSize</a> <a href="#SkImageInfo_validRowBytes">validRowBytes</a>

---

<a name="SkImageInfo_ByteSizeOverflowed"></a>
## ByteSizeOverflowed

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static bool ByteSizeOverflowed(size_t byteSize)
</pre>

Returns true if <a href="#SkImageInfo_ByteSizeOverflowed_byteSize">byteSize</a> equals <a href="undocumented#SK_MaxSizeT">SK MaxSizeT</a>. <a href="#SkImageInfo_computeByteSize">computeByteSize</a> and
<a href="#SkImageInfo_computeMinByteSize">computeMinByteSize</a> return <a href="undocumented#SK_MaxSizeT">SK MaxSizeT</a> if size_t can not hold buffer size.

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_ByteSizeOverflowed_byteSize"> <code><strong>byteSize </strong></code> </a></td> <td>
result of <a href="#SkImageInfo_computeByteSize">computeByteSize</a> or <a href="#SkImageInfo_computeMinByteSize">computeMinByteSize</a></td>
  </tr>
</table>

### Return Value

true if <a href="#SkImageInfo_computeByteSize">computeByteSize</a> or <a href="#SkImageInfo_computeMinByteSize">computeMinByteSize</a> result exceeds size_t

### Example

<div><fiddle-embed name="6a63dfdd62ab77ff57783af8c33d7b78">

#### Example Output

~~~~
rowBytes:100000000 size:99999999900000008 overflowed:false
rowBytes:1000000000 size:999999999000000008 overflowed:false
rowBytes:10000000000 size:9999999990000000008 overflowed:false
rowBytes:100000000000 size:18446744073709551615 overflowed:true
rowBytes:1000000000000 size:18446744073709551615 overflowed:true
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkImageInfo_computeByteSize">computeByteSize</a> <a href="#SkImageInfo_computeMinByteSize">computeMinByteSize</a> <a href="#SkImageInfo_validRowBytes">validRowBytes</a>

---

<a name="SkImageInfo_validRowBytes"></a>
## validRowBytes

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool validRowBytes(size_t rowBytes) const
</pre>

Returns true if <a href="#SkImageInfo_validRowBytes_rowBytes">rowBytes</a> is smaller than width times pixel size.

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_validRowBytes_rowBytes"> <code><strong>rowBytes </strong></code> </a></td> <td>
size of pixel row or larger</td>
  </tr>
</table>

### Return Value

true if <a href="#SkImageInfo_validRowBytes_rowBytes">rowBytes</a> is large enough to contain pixel row

### Example

<div><fiddle-embed name="c6b0f6a3f493cb08d9abcdefe12de245">

#### Example Output

~~~~
validRowBytes(60): false
validRowBytes(64): true
validRowBytes(68): true
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkImageInfo_ByteSizeOverflowed">ByteSizeOverflowed</a> <a href="#SkImageInfo_computeByteSize">computeByteSize</a> <a href="#SkImageInfo_computeMinByteSize">computeMinByteSize</a>

---

<a name="SkImageInfo_reset"></a>
## reset

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void reset()
</pre>

Creates an empty <a href="#Image_Info">Image Info</a> with <a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>, <a href="#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a>,
a width and height of zero, and no <a href="undocumented#Color_Space">Color Space</a>.

### Example

<div><fiddle-embed name="ab7e73786805c936de386b6c1ebe1f13">

#### Example Output

~~~~
info == copy
info != reset copy
SkImageInfo() == reset copy
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkImageInfo_empty_constructor">SkImageInfo()</a>

---

## <a name="Utility"></a> Utility

| name | description |
| --- | --- |
| <a href="#SkImageInfo_ByteSizeOverflowed">ByteSizeOverflowed</a> | checks result of <a href="#SkImageInfo_computeByteSize">computeByteSize</a> and <a href="#SkImageInfo_computeMinByteSize">computeMinByteSize</a> |
| <a href="#SkImageInfo_computeByteSize">computeByteSize</a> | memory required by pixel buffer with given row bytes |
| <a href="#SkImageInfo_computeMinByteSize">computeMinByteSize</a> | least memory required by pixel buffer |
| <a href="#SkImageInfo_computeOffset">computeOffset</a> | returns byte offset within pixel array |
| <a href="#SkImageInfo_validRowBytes">validRowBytes</a> | checks if row bytes is large enough to contain pixel row |
| <a href="#SkImageInfo_validate">validate</a> | asserts if <a href="#Image_Info">Image Info</a> is invalid (debug only) |

<a name="SkImageInfo_validate"></a>
## validate

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void validate() const
</pre>

Asserts if internal values are illegal or inconsistent. Only available if
SK_DEBUG is defined at compile time.

### See Also

<a href="#SkImageInfo_validRowBytes">validRowBytes</a> <a href="SkBitmap_Reference#SkBitmap_validate">SkBitmap::validate</a>

---

