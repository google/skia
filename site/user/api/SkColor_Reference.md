SkColor Reference
===

# <a name="Color"></a> Color

## <a name="Overview"></a> Overview

## <a name="Overview_Subtopic"></a> Overview Subtopic

| name | description |
| --- | --- |

## <a name="Define"></a> Define

| name | description |
| --- | --- |

## <a name="Function"></a> Function

| name | description |
| --- | --- |

## <a name="Typedef"></a> Typedef

| name | description |
| --- | --- |

## <a name="Alpha"></a> Alpha

## <a name="RGB"></a> RGB

## <a name="RGB_Red"></a> RGB Red

## <a name="RGB_Blue"></a> RGB Blue

## <a name="RGB_Green"></a> RGB Green

## <a name="ARGB"></a> ARGB

## <a name="RBG"></a> RBG

## <a name="RGB-565"></a> RGB-565

## <a name="Gray"></a> Gray

<a href="undocumented#Types">Types</a> and macros for colors
8-bit type for an alpha value. 0xFF is 100% opaque, 0x00 is 100% transparent.
32 bit <a href="#ARGB">ARGB</a> color value, not premultiplied. The color components are always in
a known order. This is different from <a href="#SkPMColor">SkPMColor</a>, which has its bytes in a configuration
dependent order, to match the format of <a href="SkImageInfo_Reference#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a> bitmaps. <a href="#SkColor">SkColor</a>
is the type used to specify colors in <a href="SkPaint_Reference#SkPaint">SkPaint</a> and in gradients.

<a name="SkColorSetARGB"></a>
## SkColorSetARGB

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static constexpr inline SkColor SkColorSetARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b)
</pre>

Return <a href="#SkColorSetARGB_a">a</a> <a href="#SkColor">SkColor</a> value from 8 bit component values

### Parameters

<table>  <tr>    <td><a name="SkColorSetARGB_a"> <code><strong>a </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkColorSetARGB_r"> <code><strong>r </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkColorSetARGB_g"> <code><strong>g </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkColorSetARGB_b"> <code><strong>b </strong></code> </a></td> <td>
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

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define <a href="#SkColorSetARGBInline">SkColorSetARGBInline</a> <a href="#SkColorSetARGB">SkColorSetARGB</a>
</pre>

Legacy aliases.

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define <a href="#SkColorSetARGBMacro">SkColorSetARGBMacro</a>  <a href="#SkColorSetARGB">SkColorSetARGB</a>
</pre>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define SkColorSetRGB(r, g, b)  <a href="#SkColorSetARGB">SkColorSetARGB(0xFF, r, g, b)</a>
</pre>

Return a <a href="#SkColor">SkColor</a> value from 8 bit component values, with an implied value
of 0xFF for alpha (fully opaque)

### Parameters

<table>  <tr>    <td><a name="SkColorSetRGB_r"> <code><strong>r </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkColorSetRGB_g"> <code><strong>g </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkColorSetRGB_b"> <code><strong>b </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define SkColorGetA(color)      (((color) >> 24) & 0xFF)
</pre>

return the alpha byte from a <a href="#SkColor">SkColor</a> value

### Parameters

<table>  <tr>    <td><a name="SkColorGetA_color"> <code><strong>color </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define SkColorGetR(color)      (((color) >> 16) & 0xFF)
</pre>

return the red byte from a <a href="#SkColor">SkColor</a> value

### Parameters

<table>  <tr>    <td><a name="SkColorGetR_color"> <code><strong>color </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define SkColorGetG(color)      (((color) >>  8) & 0xFF)
</pre>

return the green byte from a <a href="#SkColor">SkColor</a> value

### Parameters

<table>  <tr>    <td><a name="SkColorGetG_color"> <code><strong>color </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define SkColorGetB(color)      (((color) >>  0) & 0xFF)
</pre>

return the blue byte from a <a href="#SkColor">SkColor</a> value

### Parameters

<table>  <tr>    <td><a name="SkColorGetB_color"> <code><strong>color </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name="SkColorSetA"></a>
## SkColorSetA

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static constexpr inline SkColor SkColorSetA(SkColor c, U8CPU a)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkColorSetA_c"> <code><strong>c </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkColorSetA_a"> <code><strong>a </strong></code> </a></td> <td>
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

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define <a href="#SK_AlphaTRANSPARENT">SK AlphaTRANSPARENT</a> static_cast<<a href="#SkAlpha">SkAlpha</a>>(0x00)
</pre>

common colors
transparent <a href="#SkAlpha">SkAlpha</a> value

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define <a href="#SK_AlphaOPAQUE">SK AlphaOPAQUE</a>      static_cast<<a href="#SkAlpha">SkAlpha</a>>(0xFF)
</pre>

opaque <a href="#SkAlpha">SkAlpha</a> value

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define <a href="#SK_ColorTRANSPARENT">SK ColorTRANSPARENT</a> static_cast<<a href="#SkColor">SkColor</a>>(0x00000000)
</pre>

transparent <a href="#SkColor">SkColor</a> value

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define <a href="#SK_ColorBLACK">SK ColorBLACK</a>       static_cast<<a href="#SkColor">SkColor</a>>(0xFF000000)
</pre>

black <a href="#SkColor">SkColor</a> value

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define <a href="#SK_ColorDKGRAY">SK ColorDKGRAY</a>      static_cast<<a href="#SkColor">SkColor</a>>(0xFF444444)
</pre>

dark gray <a href="#SkColor">SkColor</a> value

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define <a href="#SK_ColorGRAY">SK ColorGRAY</a>        static_cast<<a href="#SkColor">SkColor</a>>(0xFF888888)
</pre>

gray <a href="#SkColor">SkColor</a> value

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define <a href="#SK_ColorLTGRAY">SK ColorLTGRAY</a>      static_cast<<a href="#SkColor">SkColor</a>>(0xFFCCCCCC)
</pre>

light gray <a href="#SkColor">SkColor</a> value

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define <a href="#SK_ColorWHITE">SK ColorWHITE</a>       static_cast<<a href="#SkColor">SkColor</a>>(0xFFFFFFFF)
</pre>

white <a href="#SkColor">SkColor</a> value

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define <a href="#SK_ColorRED">SK ColorRED</a>         static_cast<<a href="#SkColor">SkColor</a>>(0xFFFF0000)
</pre>

red <a href="#SkColor">SkColor</a> value

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define <a href="#SK_ColorGREEN">SK ColorGREEN</a>       static_cast<<a href="#SkColor">SkColor</a>>(0xFF00FF00)
</pre>

green <a href="#SkColor">SkColor</a> value

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define <a href="#SK_ColorBLUE">SK ColorBLUE</a>        static_cast<<a href="#SkColor">SkColor</a>>(0xFF0000FF)
</pre>

blue <a href="#SkColor">SkColor</a> value

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define <a href="#SK_ColorYELLOW">SK ColorYELLOW</a>      static_cast<<a href="#SkColor">SkColor</a>>(0xFFFFFF00)
</pre>

yellow <a href="#SkColor">SkColor</a> value

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define <a href="#SK_ColorCYAN">SK ColorCYAN</a>        static_cast<<a href="#SkColor">SkColor</a>>(0xFF00FFFF)
</pre>

cyan <a href="#SkColor">SkColor</a> value

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define <a href="#SK_ColorMAGENTA">SK ColorMAGENTA</a>     static_cast<<a href="#SkColor">SkColor</a>>(0xFFFF00FF)
</pre>

magenta <a href="#SkColor">SkColor</a> value

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

## <a name="HSV"></a> HSV

## <a name="HSV_Hue"></a> HSV Hue

<a href="#HSV_Hue">Hue</a> represents an angle, in degrees, on a color wheel. <a href="#HSV_Hue">Hue</a> has a positive value
modulo 360, where zero degrees is red.

## <a name="HSV_Saturation"></a> HSV Saturation

## <a name="HSV_Value"></a> HSV Value

<a name="SkRGBToHSV"></a>
## SkRGBToHSV

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SK_API void SkRGBToHSV(U8CPU red, U8CPU green, U8CPU blue, SkScalar hsv[3])
</pre>

Convert RGB components to <a href="#HSV">HSV</a>.
<a href="#SkRGBToHSV_hsv">hsv</a>[0] is <a href="#HSV_Hue">Hue</a> [0 .. 360)
<a href="#SkRGBToHSV_hsv">hsv</a>[1] is <a href="#HSV_Saturation">Saturation</a> [0...1]
<a href="#SkRGBToHSV_hsv">hsv</a>[2] is <a href="#HSV_Value">Value</a> [0...1]

### Parameters

<table>  <tr>    <td><a name="SkRGBToHSV_red"> <code><strong>red </strong></code> </a></td> <td>
<a href="#SkRGBToHSV_red">red</a> component value [0..255]
</td>
  </tr>  <tr>    <td><a name="SkRGBToHSV_green"> <code><strong>green </strong></code> </a></td> <td>
<a href="#SkRGBToHSV_green">green</a> component value [0..255]
</td>
  </tr>  <tr>    <td><a name="SkRGBToHSV_blue"> <code><strong>blue </strong></code> </a></td> <td>
<a href="#SkRGBToHSV_blue">blue</a> component value [0..255]
</td>
  </tr>  <tr>    <td><a name="SkRGBToHSV_hsv"> <code><strong>hsv </strong></code> </a></td> <td>
3 element array which holds the resulting <a href="#HSV">HSV</a> components.
</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkColorToHSV"></a>
## SkColorToHSV

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static inline void SkColorToHSV(SkColor color, SkScalar hsv[3])
</pre>

Convert the argb <a href="#SkColorToHSV_color">color</a> to its <a href="#HSV">HSV</a> components.
<a href="#SkColorToHSV_hsv">hsv</a>[0] represents <a href="#HSV_Hue">Hue</a>, and is assigned a value from zero to less than 360.
<a href="#SkColorToHSV_hsv">hsv</a>[1] is <a href="#HSV_Saturation">Saturation</a> [0...1]
<a href="#SkColorToHSV_hsv">hsv</a>[2] is <a href="#HSV_Value">Value</a> [0...1]

### Parameters

<table>  <tr>    <td><a name="SkColorToHSV_color"> <code><strong>color </strong></code> </a></td> <td>
the argb <a href="#SkColorToHSV_color">color</a> to convert. Note: the alpha component is ignored.
</td>
  </tr>  <tr>    <td><a name="SkColorToHSV_hsv"> <code><strong>hsv </strong></code> </a></td> <td>
3 element array which holds the resulting <a href="#HSV">HSV</a> components.
</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkHSVToColor"></a>
## SkHSVToColor

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SK_API SkColor SkHSVToColor(U8CPU alpha, const SkScalar hsv[3])
</pre>

Convert <a href="#HSV">HSV</a> components to an <a href="#ARGB">ARGB</a> color. The <a href="#SkHSVToColor_alpha">alpha</a> component is passed through unchanged.
<a href="#SkHSVToColor_hsv">hsv</a>[0] represents <a href="#HSV_Hue">Hue</a>, an angle from zero to less than 360.
<a href="#SkHSVToColor_hsv">hsv</a>[1] represents <a href="#HSV_Saturation">Saturation</a>, and varies from zero to one.
<a href="#SkHSVToColor_hsv">hsv</a>[2] represents <a href="#HSV_Value">Value</a>, and varies from zero to one.

If <a href="#SkHSVToColor_hsv">hsv</a> values are out of range, they are pinned.

### Parameters

<table>  <tr>    <td><a name="SkHSVToColor_alpha"> <code><strong>alpha </strong></code> </a></td> <td>
the <a href="#SkHSVToColor_alpha">alpha</a> component of the returned argb color.
</td>
  </tr>  <tr>    <td><a name="SkHSVToColor_hsv"> <code><strong>hsv </strong></code> </a></td> <td>
3 element array which holds the input <a href="#HSV">HSV</a> components.
</td>
  </tr>
</table>

### Return Value

the resulting argb color

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkHSVToColor_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static inline SkColor SkHSVToColor(const SkScalar hsv[3])
</pre>

Convert <a href="#HSV">HSV</a> components to an <a href="#ARGB">ARGB</a> color. The alpha component set to 0xFF.
<a href="#SkHSVToColor_2_hsv">hsv</a>[0] represents <a href="#HSV_Hue">Hue</a>, an angle from zero to less than 360.
<a href="#SkHSVToColor_2_hsv">hsv</a>[1] represents <a href="#HSV_Saturation">Saturation</a>, and varies from zero to one.
<a href="#SkHSVToColor_2_hsv">hsv</a>[2] represents <a href="#HSV_Value">Value</a>, and varies from zero to one.

If <a href="#SkHSVToColor_2_hsv">hsv</a> values are out of range, they are pinned.

### Parameters

<table>  <tr>    <td><a name="SkHSVToColor_2_hsv"> <code><strong>hsv </strong></code> </a></td> <td>
3 element array which holds the input <a href="#HSV">HSV</a> components.
</td>
  </tr>
</table>

### Return Value

the resulting argb color

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

32 bit <a href="#ARGB">ARGB</a> color value, premultiplied. The byte order for this value is
configuration dependent, matching the format of <a href="SkImageInfo_Reference#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a> bitmaps.
This is different from <a href="#SkColor">SkColor</a>, which is nonpremultiplied, and is always in the
same byte order.

<a name="SkPreMultiplyARGB"></a>
## SkPreMultiplyARGB

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SK_API SkPMColor SkPreMultiplyARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b)
</pre>

Return <a href="#SkPreMultiplyARGB_a">a</a> <a href="#SkPMColor">SkPMColor</a> value from unpremultiplied 8 bit component values

### Parameters

<table>  <tr>    <td><a name="SkPreMultiplyARGB_a"> <code><strong>a </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPreMultiplyARGB_r"> <code><strong>r </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPreMultiplyARGB_g"> <code><strong>g </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPreMultiplyARGB_b"> <code><strong>b </strong></code> </a></td> <td>
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

<a name="SkPreMultiplyColor"></a>
## SkPreMultiplyColor

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SK_API SkPMColor SkPreMultiplyColor(SkColor c)
</pre>

Return a <a href="#SkPMColor">SkPMColor</a> value from a <a href="#SkColor">SkColor</a> value. This is done by multiplying the color
components by the color's alpha, and by arranging the bytes in a configuration
dependent order, to match the format of <a href="SkImageInfo_Reference#kBGRA_8888_SkColorType">kBGRA_8888_SkColorType</a> bitmaps.

### Parameters

<table>  <tr>    <td><a name="SkPreMultiplyColor_c"> <code><strong>c </strong></code> </a></td> <td>
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

