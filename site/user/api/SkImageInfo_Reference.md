SkImageInfo Reference
===

# <a name="Image_Info"></a> Image Info

## <a name="Overview"></a> Overview

## <a name="Subtopic"></a> Subtopic

| name | description |
| --- | --- |
| <a href="SkImageInfo_Reference#Image_Info_Constructor">Constructor</a> | functions that construct <a href="SkImageInfo_Reference#SkImageInfo">SkImageInfo</a> |
| <a href="SkImageInfo_Reference#Image_Info_Member_Function">Member Function</a> | static functions and member methods |
| <a href="SkImageInfo_Reference#Image_Info_Operator">Operator</a> | operator overloading methods |
| <a href="SkImageInfo_Reference#Image_Info_Related_Function">Related Function</a> | similar methods grouped together |

## <a name="Constant"></a> Constant

| name | description |
| --- | --- |

## <a name="Alpha_Type"></a> Alpha Type

## <a name="SkAlphaType"></a> Enum SkAlphaType

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum <a href="#SkAlphaType">SkAlphaType</a> {
<a href="SkImageInfo_Reference#kUnknown_SkAlphaType">kUnknown SkAlphaType</a>,
<a href="SkImageInfo_Reference#kOpaque_SkAlphaType">kOpaque SkAlphaType</a>,
<a href="SkImageInfo_Reference#kPremul_SkAlphaType">kPremul SkAlphaType</a>,
<a href="SkImageInfo_Reference#kUnpremul_SkAlphaType">kUnpremul SkAlphaType</a>,
kLastEnum_SkAlphaType = <a href="SkImageInfo_Reference#kUnpremul_SkAlphaType">kUnpremul SkAlphaType</a>,
};</pre>

Describes how to interpret the alpha component of a pixel.

### Constants

<table>
  <tr>
    <td><a name="kUnknown_SkAlphaType"> <code><strong>kUnknown_SkAlphaType </strong></code> </a></td><td>0</td><td></td>
  </tr>
  <tr>
    <td><a name="kOpaque_SkAlphaType"> <code><strong>kOpaque_SkAlphaType </strong></code> </a></td><td>1</td><td>All pixels are stored as opaque.</td>
  </tr>
  <tr>
    <td><a name="kPremul_SkAlphaType"> <code><strong>kPremul_SkAlphaType </strong></code> </a></td><td>2</td><td>All pixels have their alpha premultiplied in their color components.
This is the natural format for the rendering target pixels.</td>
  </tr>
  <tr>
    <td><a name="kUnpremul_SkAlphaType"> <code><strong>kUnpremul_SkAlphaType </strong></code> </a></td><td>3</td><td>All pixels have their color components stored without any regard to the
alpha. e.g. this is the default configuration for PNG images.
<a href="SkImageInfo_Reference#kUnpremul_SkAlphaType">kUnpremul SkAlphaType</a> is supported only for input images. Rendering cannot
generate this on output.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete



## <a name="Color_Type"></a> Color Type

## <a name="SkColorType"></a> Enum SkColorType

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum <a href="#SkColorType">SkColorType</a> {
<a href="SkImageInfo_Reference#kUnknown_SkColorType">kUnknown SkColorType</a>,
<a href="SkImageInfo_Reference#kAlpha_8_SkColorType">kAlpha 8 SkColorType</a>,
<a href="SkImageInfo_Reference#kRGB_565_SkColorType">kRGB 565 SkColorType</a>,
<a href="SkImageInfo_Reference#kARGB_4444_SkColorType">kARGB 4444 SkColorType</a>,
<a href="SkImageInfo_Reference#kRGBA_8888_SkColorType">kRGBA 8888 SkColorType</a>,
<a href="SkImageInfo_Reference#kRGB_888x_SkColorType">kRGB 888x SkColorType</a>,
<a href="SkImageInfo_Reference#kBGRA_8888_SkColorType">kBGRA 8888 SkColorType</a>,
<a href="SkImageInfo_Reference#kRGBA_1010102_SkColorType">kRGBA 1010102 SkColorType</a>,
<a href="SkImageInfo_Reference#kRGB_101010x_SkColorType">kRGB 101010x SkColorType</a>,
<a href="SkImageInfo_Reference#kGray_8_SkColorType">kGray 8 SkColorType</a>,
<a href="SkImageInfo_Reference#kRGBA_F16_SkColorType">kRGBA F16 SkColorType</a>,
kLastEnum_SkColorType = <a href="SkImageInfo_Reference#kRGBA_F16_SkColorType">kRGBA F16 SkColorType</a>,
<a href="SkImageInfo_Reference#kN32_SkColorType">kN32 SkColorType</a> = <a href="SkImageInfo_Reference#kBGRA_8888_SkColorType">kBGRA 8888 SkColorType</a>,
<a href="SkImageInfo_Reference#kN32_SkColorType">kN32 SkColorType</a> = <a href="SkImageInfo_Reference#kRGBA_8888_SkColorType">kRGBA 8888 SkColorType</a>,
};</pre>

Describes how to interpret the components of a pixel.

<a href="SkImageInfo_Reference#kN32_SkColorType">kN32 SkColorType</a> selects the native 32-bit <a href="#ARGB">Color ARGB</a> format. On Little_Endian
processors, pixels containing 8-bit <a href="#ARGB">Color ARGB</a> components pack into 32-bit
<a href="SkImageInfo_Reference#kBGRA_8888_SkColorType">kBGRA 8888 SkColorType</a>. On Big_Endian processors, pixels pack into 32-bit
<a href="SkImageInfo_Reference#kRGBA_8888_SkColorType">kRGBA 8888 SkColorType</a>.

### Constants

<table>
  <tr>
    <td><a name="kUnknown_SkColorType"> <code><strong>kUnknown_SkColorType </strong></code> </a></td><td>0</td><td></td>
  </tr>
  <tr>
    <td><a name="kAlpha_8_SkColorType"> <code><strong>kAlpha_8_SkColorType </strong></code> </a></td><td>1</td><td></td>
  </tr>
  <tr>
    <td><a name="kRGB_565_SkColorType"> <code><strong>kRGB_565_SkColorType </strong></code> </a></td><td>2</td><td></td>
  </tr>
  <tr>
    <td><a name="kARGB_4444_SkColorType"> <code><strong>kARGB_4444_SkColorType </strong></code> </a></td><td>3</td><td></td>
  </tr>
  <tr>
    <td><a name="kRGBA_8888_SkColorType"> <code><strong>kRGBA_8888_SkColorType </strong></code> </a></td><td>4</td><td></td>
  </tr>
  <tr>
    <td><a name="kRGB_888x_SkColorType"> <code><strong>kRGB_888x_SkColorType </strong></code> </a></td><td>5</td><td></td>
  </tr>
  <tr>
    <td><a name="kBGRA_8888_SkColorType"> <code><strong>kBGRA_8888_SkColorType </strong></code> </a></td><td>6</td><td></td>
  </tr>
  <tr>
    <td><a name="kRGBA_1010102_SkColorType"> <code><strong>kRGBA_1010102_SkColorType </strong></code> </a></td><td>7</td><td></td>
  </tr>
  <tr>
    <td><a name="kRGB_101010x_SkColorType"> <code><strong>kRGB_101010x_SkColorType </strong></code> </a></td><td>8</td><td></td>
  </tr>
  <tr>
    <td><a name="kGray_8_SkColorType"> <code><strong>kGray_8_SkColorType </strong></code> </a></td><td>9</td><td></td>
  </tr>
  <tr>
    <td><a name="kRGBA_F16_SkColorType"> <code><strong>kRGBA_F16_SkColorType </strong></code> </a></td><td>10</td><td></td>
  </tr>
</table>

### Constants

<table>
  <tr>
    <td><a name="kN32_SkColorType"> <code><strong>kN32_SkColorType </strong></code> </a></td><td>4</td><td></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete



## <a name="YUV_ColorSpace"></a> YUV ColorSpace

## <a name="SkYUVColorSpace"></a> Enum SkYUVColorSpace

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum <a href="#SkYUVColorSpace">SkYUVColorSpace</a> {
<a href="SkImageInfo_Reference#kJPEG_SkYUVColorSpace">kJPEG SkYUVColorSpace</a>,
<a href="SkImageInfo_Reference#kRec601_SkYUVColorSpace">kRec601 SkYUVColorSpace</a>,
<a href="SkImageInfo_Reference#kRec709_SkYUVColorSpace">kRec709 SkYUVColorSpace</a>,
kLastEnum_SkYUVColorSpace = <a href="SkImageInfo_Reference#kRec709_SkYUVColorSpace">kRec709 SkYUVColorSpace</a>,
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
| <a href="#SkImageInfo_Make">Make</a> | creates <a href="SkImageInfo_Reference#Image_Info">Image Info</a> from dimensions, <a href="SkImageInfo_Reference#Color_Type">Color Type</a>, <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkImageInfo_MakeA8">MakeA8</a> | creates <a href="SkImageInfo_Reference#Image_Info">Image Info</a> with <a href="SkImageInfo_Reference#SkColorType">kAlpha 8 SkColorType</a>, <a href="SkImageInfo_Reference#SkAlphaType">kPremul SkAlphaType</a> |
| <a href="#SkImageInfo_MakeN32">MakeN32</a> | creates <a href="SkImageInfo_Reference#Image_Info">Image Info</a> with <a href="SkImageInfo_Reference#Color_Type">Native Color Type</a> |
| <a href="#SkImageInfo_MakeN32Premul">MakeN32Premul</a> | creates <a href="SkImageInfo_Reference#Image_Info">Image Info</a> with <a href="SkImageInfo_Reference#Color_Type">Native Color Type</a>, <a href="SkImageInfo_Reference#SkAlphaType">kPremul SkAlphaType</a> |
| <a href="#SkImageInfo_MakeS32">MakeS32</a> | creates <a href="SkImageInfo_Reference#Image_Info">Image Info</a> with <a href="SkImageInfo_Reference#Color_Type">Native Color Type</a>, sRGB <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkImageInfo_MakeUnknown">MakeUnknown</a> | creates <a href="SkImageInfo_Reference#Image_Info">Image Info</a> with <a href="SkImageInfo_Reference#SkColorType">kUnknown SkColorType</a>, <a href="SkImageInfo_Reference#SkAlphaType">kUnknown SkAlphaType</a> |
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
| <a href="#SkImageInfo_makeAlphaType">makeAlphaType</a> | creates <a href="SkImageInfo_Reference#Image_Info">Image Info</a> with changed <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a> |
| <a href="#SkImageInfo_makeColorSpace">makeColorSpace</a> | creates <a href="SkImageInfo_Reference#Image_Info">Image Info</a> with changed <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkImageInfo_makeColorType">makeColorType</a> | creates <a href="SkImageInfo_Reference#Image_Info">Image Info</a> with changed <a href="SkImageInfo_Reference#Color_Type">Color Type</a> |
| <a href="#SkImageInfo_makeWH">makeWH</a> | creates <a href="SkImageInfo_Reference#Image_Info">Image Info</a> with changed dimensions |
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
| <a href="SkImageInfo_Reference#Image_Info_Property">Property</a> | metrics and attributes |
| <a href="SkImageInfo_Reference#Image_Info_Utility">Utility</a> | rarely called management functions |

## <a name="Constructor"></a> Constructor

| name | description |
| --- | --- |
| <a href="#SkImageInfo_Make">Make</a> | creates <a href="SkImageInfo_Reference#Image_Info">Image Info</a> from dimensions, <a href="SkImageInfo_Reference#Color_Type">Color Type</a>, <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a>, <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkImageInfo_MakeA8">MakeA8</a> | creates <a href="SkImageInfo_Reference#Image_Info">Image Info</a> with <a href="SkImageInfo_Reference#SkColorType">kAlpha 8 SkColorType</a>, <a href="SkImageInfo_Reference#SkAlphaType">kPremul SkAlphaType</a> |
| <a href="#SkImageInfo_MakeN32">MakeN32</a> | creates <a href="SkImageInfo_Reference#Image_Info">Image Info</a> with <a href="SkImageInfo_Reference#Color_Type">Native Color Type</a> |
| <a href="#SkImageInfo_MakeN32Premul">MakeN32Premul</a> | creates <a href="SkImageInfo_Reference#Image_Info">Image Info</a> with <a href="SkImageInfo_Reference#Color_Type">Native Color Type</a>, <a href="SkImageInfo_Reference#SkAlphaType">kPremul SkAlphaType</a> |
|  | <a href="#SkImageInfo_MakeN32Premul">MakeN32Premul(int width, int height, sk sp&lt;SkColorSpace&gt; cs = nullptr)</a> |
|  | <a href="#SkImageInfo_MakeN32Premul_2">MakeN32Premul(const SkISize& size)</a> |
| <a href="#SkImageInfo_MakeS32">MakeS32</a> | creates <a href="SkImageInfo_Reference#Image_Info">Image Info</a> with <a href="SkImageInfo_Reference#Color_Type">Native Color Type</a>, sRGB <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkImageInfo_MakeUnknown">MakeUnknown</a> | creates <a href="SkImageInfo_Reference#Image_Info">Image Info</a> with <a href="SkImageInfo_Reference#SkColorType">kUnknown SkColorType</a>, <a href="SkImageInfo_Reference#SkAlphaType">kUnknown SkAlphaType</a> |
|  | <a href="#SkImageInfo_MakeUnknown">MakeUnknown(int width, int height)</a> |
|  | <a href="#SkImageInfo_MakeUnknown_2">MakeUnknown()</a> |
| <a href="#SkImageInfo_empty_constructor">SkImageInfo()</a> | creates with zero dimensions, <a href="SkImageInfo_Reference#SkColorType">kUnknown SkColorType</a>, <a href="SkImageInfo_Reference#SkAlphaType">kUnknown SkAlphaType</a> |
| <a href="#SkImageInfo_makeAlphaType">makeAlphaType</a> | creates <a href="SkImageInfo_Reference#Image_Info">Image Info</a> with changed <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a> |
| <a href="#SkImageInfo_makeColorSpace">makeColorSpace</a> | creates <a href="SkImageInfo_Reference#Image_Info">Image Info</a> with changed <a href="undocumented#Color_Space">Color Space</a> |
| <a href="#SkImageInfo_makeColorType">makeColorType</a> | creates <a href="SkImageInfo_Reference#Image_Info">Image Info</a> with changed <a href="SkImageInfo_Reference#Color_Type">Color Type</a> |
| <a href="#SkImageInfo_makeWH">makeWH</a> | creates <a href="SkImageInfo_Reference#Image_Info">Image Info</a> with changed dimensions |
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

Sets <a href="SkImageInfo_Reference#Color_Type">Color Type</a> to <a href="SkImageInfo_Reference#SkColorType">kN32 SkColorType</a>.

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

Creates <a href="SkImageInfo_Reference#Image_Info">Image Info</a> marked as sRGB with <a href="SkImageInfo_Reference#SkColorType">kN32 SkColorType</a> swizzle.

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

Sets <a href="SkImageInfo_Reference#Color_Type">Color Type</a> to <a href="SkImageInfo_Reference#SkColorType">kN32 SkColorType</a>, and the <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a> to <a href="SkImageInfo_Reference#SkAlphaType">kPremul SkAlphaType</a>.

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

Creates <a href="SkImageInfo_Reference#Image_Info">Image Info</a> with the same <a href="SkImageInfo_Reference#Color_Type">Color Type</a> and <a href="SkImageInfo_Reference#Alpha_Type">Alpha Type</a> as this info,
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

