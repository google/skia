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
};</pre>

Describes how to interpret the alpha component of a pixel. A pixel may
be opaque, or <a href="undocumented#Alpha">Color Alpha</a>, describing multiple levels of transparency.

In simple blending, <a href="undocumented#Alpha">Color Alpha</a> weights the draw color and the destination
color to create a new color. If alpha describes a weight from zero to one:

new color = draw color * alpha + destination color * (1 - alpha)In practice alpha is encoded in two or more bits, where 1.0 equals all bits set.

<a href="undocumented#RGB">Color RGB</a> may have <a href="undocumented#Alpha">Color Alpha</a> included in each component value; the stored
value is the original <a href="undocumented#RGB">Color RGB</a> multiplied by <a href="undocumented#Alpha">Color Alpha</a>. <a href="undocumented#Premultiply">Premultiplied</a> color
components improve performance.

### Constants

<table>
  <tr>
    <td><a name="kUnknown_SkAlphaType"> <code><strong>kUnknown_SkAlphaType </strong></code> </a></td><td>0</td><td><a href="#Alpha_Type">Alpha Type</a> is uninitialized.</td>
  </tr>
  <tr>
    <td><a name="kOpaque_SkAlphaType"> <code><strong>kOpaque_SkAlphaType </strong></code> </a></td><td>1</td><td>Pixels are opaque. The <a href="#Color_Type">Color Type</a> must have no explicit alpha
component, or all alpha components must be set to their maximum value.</td>
  </tr>
  <tr>
    <td><a name="kPremul_SkAlphaType"> <code><strong>kPremul_SkAlphaType </strong></code> </a></td><td>2</td><td>Pixels have alpha premultiplied into color components.
<a href="SkSurface_Reference#Surface">Surface</a> pixels must be premultiplied.</td>
  </tr>
  <tr>
    <td><a name="kUnpremul_SkAlphaType"> <code><strong>kUnpremul_SkAlphaType </strong></code> </a></td><td>3</td><td><a href="undocumented#Pixel">Pixel</a> color component values are independent of alpha value.
Images generated from encoded data like PNG do not premultiply pixel color
components. <a href="#kUnpremul_SkAlphaType">kUnpremul_SkAlphaType</a> is supported for <a href="SkImage_Reference#Image">Image</a> pixels, but not for
<a href="SkSurface_Reference#Surface">Surface</a> pixels.</td>
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
in the same order.</div></fiddle-embed></div>

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
by the original color.</div></fiddle-embed></div>

## <a name="Alpha_Type_Unpremul"></a> Alpha Type Unpremul

Use <a href="#Alpha_Type_Unpremul">Unpremul</a> if stored color components are not divided by the alpha component.
Some drawing destinations may not support <a href="#Alpha_Type_Unpremul">Unpremul</a>.

### Example

<div><fiddle-embed name="46e528e0c6b3f3e296d0d0930d638629"><div><a href="undocumented#SkColorSetARGB">SkColorSetARGB</a> parameter a is set to 150, less than its maximum value, and is
interpreted as <a href="undocumented#Alpha">Color Alpha</a> of about 0.6. color is not premultiplied;
color components may have values greater than color alpha.
The four displayed values are the original component values, though not necessarily
in the same order.</div></fiddle-embed></div>

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
<a href="#kN32_SkColorType">kN32_SkColorType</a> = <a href="#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>,
<a href="#kN32_SkColorType">kN32_SkColorType</a> = <a href="#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a>,
};</pre>

Describes how pixel bits encode color. A pixel may be an alpha mask, a
gray level, <a href="undocumented#RGB">Color RGB</a>, or <a href="undocumented#ARGB">Color ARGB</a>.

<a href="#kN32_SkColorType">kN32_SkColorType</a> selects the native 32-bit <a href="undocumented#ARGB">Color ARGB</a> format. On Little_Endian
processors, pixels containing 8-bit <a href="undocumented#ARGB">Color ARGB</a> components pack into 32-bit
<a href="#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a>. On Big_Endian processors, pixels pack into 32-bit
<a href="#kRGBA_8888_SkColorType">kRGBA_8888_SkColorType</a>.

### Constants

<table>
  <tr>
    <td><a name="kUnknown_SkColorType"> <code><strong>kUnknown_SkColorType </strong></code> </a></td><td>0</td><td></td>
  </tr>
  <tr>
    <td><a name="kAlpha_8_SkColorType"> <code><strong>kAlpha_8_SkColorType </strong></code> </a></td><td>1</td><td>Encodes <a href="undocumented#Alpha">Color Alpha</a> as <a href="#Color_Type_Alpha_8">Alpha 8</a> pixel in an 8-bit byte.</td>
  </tr>
  <tr>
    <td><a name="kRGB_565_SkColorType"> <code><strong>kRGB_565_SkColorType </strong></code> </a></td><td>2</td><td>Encodes <a href="undocumented#RGB">Color RGB</a> as <a href="#Color_Type_BGR_565">BGR 565</a> pixel in a 16-bit word.</td>
  </tr>
  <tr>
    <td><a name="kARGB_4444_SkColorType"> <code><strong>kARGB_4444_SkColorType </strong></code> </a></td><td>3</td><td>Encodes <a href="undocumented#ARGB">Color ARGB</a> as <a href="#Color_Type_ABGR_4444">ABGR 4444</a> pixel in a 16-bit word.</td>
  </tr>
  <tr>
    <td><a name="kRGBA_8888_SkColorType"> <code><strong>kRGBA_8888_SkColorType </strong></code> </a></td><td>4</td><td>Encodes <a href="undocumented#ARGB">Color ARGB</a> as <a href="#Color_Type_RGBA_8888">RGBA 8888</a> pixel in a 32-bit word.</td>
  </tr>
  <tr>
    <td><a name="kRGB_888x_SkColorType"> <code><strong>kRGB_888x_SkColorType </strong></code> </a></td><td>5</td><td>Encodes <a href="undocumented#RGB">Color RGB</a> as <a href="#Color_Type_RGB_888x">RGB 888x</a> pixel in a 32-bit word.</td>
  </tr>
  <tr>
    <td><a name="kBGRA_8888_SkColorType"> <code><strong>kBGRA_8888_SkColorType </strong></code> </a></td><td>6</td><td>Encodes <a href="undocumented#ARGB">Color ARGB</a> as <a href="#Color_Type_BGRA_8888">BGRA 8888</a> pixel in a 32-bit word.</td>
  </tr>
  <tr>
    <td><a name="kRGBA_1010102_SkColorType"> <code><strong>kRGBA_1010102_SkColorType </strong></code> </a></td><td>7</td><td>Encodes <a href="undocumented#ARGB">Color ARGB</a> as <a href="#Color_Type_RGBA_1010102">RGBA 1010102</a> pixel in a 32-bit word.</td>
  </tr>
  <tr>
    <td><a name="kRGB_101010x_SkColorType"> <code><strong>kRGB_101010x_SkColorType </strong></code> </a></td><td>8</td><td>Encodes <a href="undocumented#RGB">Color RGB</a> as <a href="#Color_Type_RGB_101010x">RGB 101010x</a> pixel in a 32-bit word.</td>
  </tr>
  <tr>
    <td><a name="kGray_8_SkColorType"> <code><strong>kGray_8_SkColorType </strong></code> </a></td><td>9</td><td>Encodes <a href="undocumented#Gray">Color Gray</a> as <a href="#Color_Type_Gray_8">Gray 8</a> in an 8-bit byte.</td>
  </tr>
  <tr>
    <td><a name="kRGBA_F16_SkColorType"> <code><strong>kRGBA_F16_SkColorType </strong></code> </a></td><td>10</td><td>Encodes <a href="undocumented#ARGB">Color ARGB</a> as <a href="#Color_Type_RGBA_F16">RGBA F16</a> in a 64-bit word.</td>
  </tr>
</table>

### Constants

<table>
  <tr>
    <td><a name="kN32_SkColorType"> <code><strong>kN32_SkColorType </strong></code> </a></td><td>4</td><td>Encodes <a href="undocumented#ARGB">Color ARGB</a> as either <a href="#Color_Type_RGBA_8888">RGBA 8888</a> or <a href="#Color_Type_BGRA_8888">BGRA 8888</a>, whichever
is native to the platform.</td>
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
with its transparency set to alpha8 pixel value.</div></fiddle-embed></div>

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

![Color_Type_RGB_101010x](https://fiddle.skia.org/i/c22477b11dabaa3e3a0b5bb33a7733cd_raster.png "")

### Example

<div><fiddle-embed name="d975ec17354b1297841e4a31d3f6a5d5"></fiddle-embed></div>

## <a name="Color_Type_Gray_8"></a> Color Type Gray 8

### Example

<div><fiddle-embed name="93da0eb0b6722a4f33dc7dae094abf0b"></fiddle-embed></div>

## <a name="Color_Type_RGBA_F16"></a> Color Type RGBA F16

![Color_Type_RGBA_F16](https://fiddle.skia.org/i/9344796c059ff5e4f057595e781905b3_raster.png "")

### Example

<div><fiddle-embed name="1795410dffea303b08ba98ee78dc1556"></fiddle-embed></div>

## <a name="YUV_ColorSpace"></a> YUV ColorSpace

## <a name="SkYUVColorSpace"></a> Enum SkYUVColorSpace

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum <a href="#SkYUVColorSpace">SkYUVColorSpace</a> {
<a href="#kJPEG_SkYUVColorSpace">kJPEG_SkYUVColorSpace</a>,
<a href="#kRec601_SkYUVColorSpace">kRec601_SkYUVColorSpace</a>,
<a href="#kRec709_SkYUVColorSpace">kRec709_SkYUVColorSpace</a>,
kLastEnum_SkYUVColorSpace = <a href="#kRec709_SkYUVColorSpace">kRec709_SkYUVColorSpace</a>,
};</pre>

Describes the color space a YUV pixel.

### Constants

<table>
  <tr>
    <td><a name="kJPEG_SkYUVColorSpace"> <code><strong>kJPEG_SkYUVColorSpace </strong></code> </a></td><td>0</td><td>Standard JPEG color space.</td>
  </tr>
  <tr>
    <td><a name="kRec601_SkYUVColorSpace"> <code><strong>kRec601_SkYUVColorSpace </strong></code> </a></td><td>1</td><td>SDTV standard Rec. 601 color space. Uses "studio swing" [16, 235] color
range. See http://en.wikipedia.org/wiki/Rec._601 for details.</td>
  </tr>
  <tr>
    <td><a name="kRec709_SkYUVColorSpace"> <code><strong>kRec709_SkYUVColorSpace </strong></code> </a></td><td>2</td><td>HDTV standard Rec. 709 color space. Uses "studio swing" [16, 235] color
range. See http://en.wikipedia.org/wiki/Rec._709 for details.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete



## <a name="SkDestinationSurfaceColorMode"></a> Enum SkDestinationSurfaceColorMode

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum class <a href="#SkDestinationSurfaceColorMode">SkDestinationSurfaceColorMode</a> {
<a href="#SkDestinationSurfaceColorMode_kLegacy">kLegacy</a>,
<a href="#SkDestinationSurfaceColorMode_kGammaAndColorSpaceAware">kGammaAndColorSpaceAware</a>,
};</pre>

### Constants

<table>
  <tr>
    <td><a name="SkDestinationSurfaceColorMode_kLegacy"> <code><strong>SkDestinationSurfaceColorMode::kLegacy </strong></code> </a></td><td>0</td><td></td>
  </tr>
  <tr>
    <td><a name="SkDestinationSurfaceColorMode_kGammaAndColorSpaceAware"> <code><strong>SkDestinationSurfaceColorMode::kGammaAndColorSpaceAware </strong></code> </a></td><td>1</td><td></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete



# <a name="SkImageInfo"></a> Struct SkImageInfo
Describes <a href="SkImage_Reference#Image">Image</a> dimensions and pixel type.
Used for both source images and render-targets (surfaces).

## <a name="Member_Function"></a> Member Function

| name | description |
| --- | --- |
| <a href="#SkImageInfo_ByteSizeOverflowed">ByteSizeOverflowed</a> | incomplete |
| <a href="#SkImageInfo_Make">Make</a> | creates <a href="#Image_Info">Image Info</a> from dimensions, <a href="#Color_Type">Color Type</a>, <a href="#Alpha_Type">Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkImageInfo_MakeA8">MakeA8</a> | creates <a href="#Image_Info">Image Info</a> with <a href="#kAlpha_8_SkColorType">kAlpha_8_SkColorType</a>, <a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a> |
| <a href="#SkImageInfo_MakeN32">MakeN32</a> | creates <a href="#Image_Info">Image Info</a> with <a href="#Color_Type_Native">Native Color Type</a> |
| <a href="#SkImageInfo_MakeN32Premul">MakeN32Premul</a> | creates <a href="#Image_Info">Image Info</a> with <a href="#Color_Type_Native">Native Color Type</a>, <a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a> |
| <a href="#SkImageInfo_MakeS32">MakeS32</a> | creates <a href="#Image_Info">Image Info</a> with <a href="#Color_Type_Native">Native Color Type</a>, sRGB <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkImageInfo_MakeUnknown">MakeUnknown</a> | creates <a href="#Image_Info">Image Info</a> with <a href="#kUnknown_SkColorType">kUnknown_SkColorType</a>, <a href="#kUnknown_SkAlphaType">kUnknown_SkAlphaType</a> |
| <a href="#SkImageInfo_alphaType">alphaType</a> | incomplete |
| <a href="#SkImageInfo_bounds">bounds</a> | incomplete |
| <a href="#SkImageInfo_bytesPerPixel">bytesPerPixel</a> | incomplete |
| <a href="#SkImageInfo_colorSpace">colorSpace</a> | incomplete |
| <a href="#SkImageInfo_colorType">colorType</a> | incomplete |
| <a href="#SkImageInfo_computeByteSize">computeByteSize</a> | incomplete |
| <a href="#SkImageInfo_computeMinByteSize">computeMinByteSize</a> | incomplete |
| <a href="#SkImageInfo_computeOffset">computeOffset</a> | incomplete |
| <a href="#SkImageInfo_dimensions">dimensions</a> | incomplete |
| <a href="#SkImageInfo_flatten">flatten</a> | incomplete |
| <a href="#SkImageInfo_gammaCloseToSRGB">gammaCloseToSRGB</a> | incomplete |
| <a href="#SkImageInfo_height">height</a> | incomplete |
| <a href="#SkImageInfo_isEmpty">isEmpty</a> | incomplete |
| <a href="#SkImageInfo_isOpaque">isOpaque</a> | incomplete |
| <a href="#SkImageInfo_makeAlphaType">makeAlphaType</a> | creates <a href="#Image_Info">Image Info</a> with changed <a href="#Alpha_Type">Alpha Type</a> |
| <a href="#SkImageInfo_makeColorSpace">makeColorSpace</a> | creates <a href="#Image_Info">Image Info</a> with changed <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkImageInfo_makeColorType">makeColorType</a> | creates <a href="#Image_Info">Image Info</a> with changed <a href="#Color_Type">Color Type</a> |
| <a href="#SkImageInfo_makeWH">makeWH</a> | creates <a href="#Image_Info">Image Info</a> with changed dimensions |
| <a href="#SkImageInfo_minRowBytes">minRowBytes</a> | incomplete |
| <a href="#SkImageInfo_minRowBytes64">minRowBytes64</a> | incomplete |
| <a href="#SkImageInfo_refColorSpace">refColorSpace</a> | incomplete |
| <a href="#SkImageInfo_reset">reset</a> | incomplete |
| <a href="#SkImageInfo_shiftPerPixel">shiftPerPixel</a> | incomplete |
| <a href="#SkImageInfo_unflatten">unflatten</a> | incomplete |
| <a href="#SkImageInfo_validRowBytes">validRowBytes</a> | incomplete |
| <a href="#SkImageInfo_validate">validate</a> | incomplete |
| <a href="#SkImageInfo_width">width</a> | incomplete |

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
| <a href="#SkImageInfo_reset">reset</a> | incomplete |

<a name="SkImageInfo_empty_constructor"></a>
## SkImageInfo

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkImageInfo()
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_Make"></a>
## Make

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static SkImageInfo Make(int width, int height, SkColorType ct, SkAlphaType at,
                        sk_sp&lt;SkColorSpace&gt; cs = nullptr)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_Make_width"> <code><strong>width </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImageInfo_Make_height"> <code><strong>height </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImageInfo_Make_ct"> <code><strong>ct </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImageInfo_Make_at"> <code><strong>at </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImageInfo_Make_cs"> <code><strong>cs </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_MakeN32"></a>
## MakeN32

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static SkImageInfo MakeN32(int width, int height, SkAlphaType at, sk_sp&lt;SkColorSpace&gt; cs = nullptr)
</pre>

Sets <a href="#Color_Type">Color Type</a> to <a href="#kN32_SkColorType">kN32_SkColorType</a>.

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_MakeN32_width"> <code><strong>width </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImageInfo_MakeN32_height"> <code><strong>height </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImageInfo_MakeN32_at"> <code><strong>at </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImageInfo_MakeN32_cs"> <code><strong>cs </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_MakeS32"></a>
## MakeS32

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static SkImageInfo MakeS32(int width, int height, SkAlphaType at)
</pre>

Creates <a href="#Image_Info">Image Info</a> marked as sRGB with <a href="#kN32_SkColorType">kN32_SkColorType</a> swizzle.

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_MakeS32_width"> <code><strong>width </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImageInfo_MakeS32_height"> <code><strong>height </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImageInfo_MakeS32_at"> <code><strong>at </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_MakeN32Premul"></a>
## MakeN32Premul

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static SkImageInfo MakeN32Premul(int width, int height, sk_sp&lt;SkColorSpace&gt; cs = nullptr)
</pre>

Sets <a href="#Color_Type">Color Type</a> to <a href="#kN32_SkColorType">kN32_SkColorType</a>, and the <a href="#Alpha_Type">Alpha Type</a> to <a href="#kPremul_SkAlphaType">kPremul_SkAlphaType</a>.

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_MakeN32Premul_width"> <code><strong>width </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImageInfo_MakeN32Premul_height"> <code><strong>height </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImageInfo_MakeN32Premul_cs"> <code><strong>cs </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_MakeN32Premul_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static SkImageInfo MakeN32Premul(const SkISize& size)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_MakeN32Premul_2_size"> <code><strong>size </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_MakeA8"></a>
## MakeA8

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static SkImageInfo MakeA8(int width, int height)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_MakeA8_width"> <code><strong>width </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImageInfo_MakeA8_height"> <code><strong>height </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_MakeUnknown"></a>
## MakeUnknown

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static SkImageInfo MakeUnknown(int width, int height)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_MakeUnknown_width"> <code><strong>width </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImageInfo_MakeUnknown_height"> <code><strong>height </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_MakeUnknown_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static SkImageInfo MakeUnknown()
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

## <a name="Property"></a> Property

| name | description |
| --- | --- |
| <a href="#SkImageInfo_alphaType">alphaType</a> | incomplete |
| <a href="#SkImageInfo_bounds">bounds</a> | incomplete |
| <a href="#SkImageInfo_bytesPerPixel">bytesPerPixel</a> | incomplete |
| <a href="#SkImageInfo_colorSpace">colorSpace</a> | incomplete |
| <a href="#SkImageInfo_colorType">colorType</a> | incomplete |
| <a href="#SkImageInfo_dimensions">dimensions</a> | incomplete |
| <a href="#SkImageInfo_gammaCloseToSRGB">gammaCloseToSRGB</a> | incomplete |
| <a href="#SkImageInfo_height">height</a> | incomplete |
| <a href="#SkImageInfo_isEmpty">isEmpty</a> | incomplete |
| <a href="#SkImageInfo_isOpaque">isOpaque</a> | incomplete |
| <a href="#SkImageInfo_minRowBytes">minRowBytes</a> | incomplete |
| <a href="#SkImageInfo_minRowBytes64">minRowBytes64</a> | incomplete |
| <a href="#SkImageInfo_refColorSpace">refColorSpace</a> | incomplete |
| <a href="#SkImageInfo_shiftPerPixel">shiftPerPixel</a> | incomplete |
| <a href="#SkImageInfo_width">width</a> | incomplete |

<a name="SkImageInfo_width"></a>
## width

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int width() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_height"></a>
## height

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int height() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_colorType"></a>
## colorType

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkColorType colorType() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_alphaType"></a>
## alphaType

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkAlphaType alphaType() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_colorSpace"></a>
## colorSpace

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkColorSpace* colorSpace() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_refColorSpace"></a>
## refColorSpace

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sk_sp&lt;SkColorSpace&gt; refColorSpace() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_isEmpty"></a>
## isEmpty

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isEmpty() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_isOpaque"></a>
## isOpaque

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isOpaque() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_dimensions"></a>
## dimensions

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkISize dimensions() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_bounds"></a>
## bounds

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkIRect bounds() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_gammaCloseToSRGB"></a>
## gammaCloseToSRGB

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool gammaCloseToSRGB() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_makeWH"></a>
## makeWH

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkImageInfo makeWH(int newWidth, int newHeight) const
</pre>

Creates <a href="#Image_Info">Image Info</a> with the same <a href="#Color_Type">Color Type</a> and <a href="#Alpha_Type">Alpha Type</a> as this info,
but with the specified width and height.

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_makeWH_newWidth"> <code><strong>newWidth </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImageInfo_makeWH_newHeight"> <code><strong>newHeight </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_makeAlphaType"></a>
## makeAlphaType

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkImageInfo makeAlphaType(SkAlphaType newAlphaType) const
</pre>

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_makeAlphaType_newAlphaType"> <code><strong>newAlphaType </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_makeColorType"></a>
## makeColorType

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkImageInfo makeColorType(SkColorType newColorType) const
</pre>

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_makeColorType_newColorType"> <code><strong>newColorType </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_makeColorSpace"></a>
## makeColorSpace

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkImageInfo makeColorSpace(sk_sp&lt;SkColorSpace&gt; cs) const
</pre>

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_makeColorSpace_cs"> <code><strong>cs </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_bytesPerPixel"></a>
## bytesPerPixel

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int bytesPerPixel() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_shiftPerPixel"></a>
## shiftPerPixel

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int shiftPerPixel() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_minRowBytes64"></a>
## minRowBytes64

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
uint64_t minRowBytes64() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_minRowBytes"></a>
## minRowBytes

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
size_t minRowBytes() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_computeOffset"></a>
## computeOffset

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
size_t computeOffset(int x, int y, size_t rowBytes) const
</pre>

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_computeOffset_x"> <code><strong>x </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImageInfo_computeOffset_y"> <code><strong>y </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkImageInfo_computeOffset_rowBytes"> <code><strong>rowBytes </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

## <a name="Operator"></a> Operator

| name | description |
| --- | --- |
| <a href="#SkImageInfo_notequal1_operator">operator!=(const SkImageInfo& other) const</a> | incomplete |
| <a href="#SkImageInfo_equal1_operator">operator==(const SkImageInfo& other) const</a> | incomplete |

<a name="SkImageInfo_equal1_operator"></a>
## operator==

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool operator==(const SkImageInfo& other) _const
</pre>

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_equal1_operator_other"> <code><strong>other </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_notequal1_operator"></a>
## operator!=

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool operator!=(const SkImageInfo& other) _const
</pre>

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_notequal1_operator_other"> <code><strong>other </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_unflatten"></a>
## unflatten

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void unflatten(SkReadBuffer& buffer)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_unflatten_buffer"> <code><strong>buffer </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_flatten"></a>
## flatten

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void flatten(SkWriteBuffer& buffer) const
</pre>

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_flatten_buffer"> <code><strong>buffer </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_computeByteSize"></a>
## computeByteSize

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
size_t computeByteSize(size_t rowBytes) const
</pre>

Returns the size (in bytes) of the image buffer that this info needs, given the specified
<a href="#SkImageInfo_computeByteSize_rowBytes">rowBytes</a>. The <a href="#SkImageInfo_computeByteSize_rowBytes">rowBytes</a> must be >= this-><a href="#SkImageInfo_minRowBytes">minRowBytes</a>.
if (height == 0) {
return 0;
} else {
return (height - 1) * <a href="#SkImageInfo_computeByteSize_rowBytes">rowBytes</a> + width * bytes_per_pixel.

If the calculation overflows this returns <a href="undocumented#SK_MaxSizeT">SK MaxSizeT</a>.

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_computeByteSize_rowBytes"> <code><strong>rowBytes </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_computeMinByteSize"></a>
## computeMinByteSize

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
size_t computeMinByteSize() const
</pre>

Returns the minimum size (in bytes) of the image buffer that this info needs.
If the calculation overflows, or if the height is 0, this returns 0.

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_ByteSizeOverflowed"></a>
## ByteSizeOverflowed

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static bool ByteSizeOverflowed(size_t byteSize)
</pre>

Returns true if the result of <a href="#SkImageInfo_computeByteSize">computeByteSize</a> (or <a href="#SkImageInfo_computeMinByteSize">computeMinByteSize</a>) overflowed

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_ByteSizeOverflowed_byteSize"> <code><strong>byteSize </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_validRowBytes"></a>
## validRowBytes

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool validRowBytes(size_t rowBytes) const
</pre>

### Parameters

<table>  <tr>    <td><a name="SkImageInfo_validRowBytes_rowBytes"> <code><strong>rowBytes </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkImageInfo_reset"></a>
## reset

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void reset()
</pre>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

## <a name="Utility"></a> Utility

| name | description |
| --- | --- |
| <a href="#SkImageInfo_ByteSizeOverflowed">ByteSizeOverflowed</a> | incomplete |
| <a href="#SkImageInfo_computeByteSize">computeByteSize</a> | incomplete |
| <a href="#SkImageInfo_computeMinByteSize">computeMinByteSize</a> | incomplete |
| <a href="#SkImageInfo_computeOffset">computeOffset</a> | incomplete |
| <a href="#SkImageInfo_flatten">flatten</a> | incomplete |
| <a href="#SkImageInfo_unflatten">unflatten</a> | incomplete |
| <a href="#SkImageInfo_validRowBytes">validRowBytes</a> | incomplete |
| <a href="#SkImageInfo_validate">validate</a> | incomplete |

<a name="SkImageInfo_validate"></a>
## validate

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void validate() const
</pre>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

