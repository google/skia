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

### See Also

<a href="#SkPMColor">SkPMColor</a>

<a name="SkColorSetARGB"></a>
## SkColorSetARGB

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static constexpr inline SkColor SkColorSetARGB(U8CPU a, U8CPU r, U8CPU g, U8CPU b)
</pre>

Return <a href="#SkColorSetARGB_a">a</a> <a href="#SkColor">SkColor</a> value from 8 bit component values

### Parameters

<table>  <tr>    <td><a name="SkColorSetARGB_a"> <code><strong>a </strong></code> </a></td> <td>
amount of <a href="#Alpha">Alpha</a>, from fully transparent (0) to fully opaque (255)</td>
  </tr>  <tr>    <td><a name="SkColorSetARGB_r"> <code><strong>r </strong></code> </a></td> <td>
amount of <a href="#RGB_Red">RGB Red</a>, from no red (0) to full red (255)</td>
  </tr>  <tr>    <td><a name="SkColorSetARGB_g"> <code><strong>g </strong></code> </a></td> <td>
amount of <a href="#RGB_Green">RGB Green</a>, from no green (0) to full green (255)</td>
  </tr>  <tr>    <td><a name="SkColorSetARGB_b"> <code><strong>b </strong></code> </a></td> <td>
amount of <a href="#RGB_Blue">RGB Blue</a>, from no blue (0) to full blue (255)</td>
  </tr>
</table>

### Return Value

color and alpha, not premultiplied

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
<a href="#SkColor">SkColor</a>, a 32-bit unsigned int, in 0xAARRGGBB format</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define SkColorGetR(color)      (((color) >> 16) & 0xFF)
</pre>

Returns red component of <a href="#Color">Color</a>, from zero to 255.

### Parameters

<table>  <tr>    <td><a name="SkColorGetR_color"> <code><strong>color </strong></code> </a></td> <td>
<a href="#SkColor">SkColor</a>, a 32-bit unsigned int, in 0xAARRGGBB format</td>
  </tr>
</table>

### Return Value

red byte

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define SkColorGetG(color)      (((color) >>  8) & 0xFF)
</pre>

Returns green component of <a href="#Color">Color</a>, from zero to 255.

### Parameters

<table>  <tr>    <td><a name="SkColorGetG_color"> <code><strong>color </strong></code> </a></td> <td>
<a href="#SkColor">SkColor</a>, a 32-bit unsigned int, in 0xAARRGGBB format</td>
  </tr>
</table>

### Return Value

green byte

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define SkColorGetB(color)      (((color) >>  0) & 0xFF)
</pre>

Returns blue component of <a href="#Color">Color</a>, from zero to 255.

### Parameters

<table>  <tr>    <td><a name="SkColorGetB_color"> <code><strong>color </strong></code> </a></td> <td>
<a href="#SkColor">SkColor</a>, a 32-bit unsigned int, in 0xAARRGGBB format</td>
  </tr>
</table>

### Return Value

blue byte

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name="SkColorSetA"></a>
## SkColorSetA

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static constexpr inline SkColor SkColorSetA(SkColor c, U8CPU a)
</pre>

Returns <a href="#Color">Color</a> with red, blue, and green set from <a href="#SkColorSetA_c">c</a>; and alpha set from <a href="#SkColorSetA_a">a</a>.

### Parameters

<table>  <tr>    <td><a name="SkColorSetA_c"> <code><strong>c </strong></code> </a></td> <td>
<a href="undocumented#Unpremultiply">Unpremultiplied</a> <a href="#ARGB">Color ARGB</a></td>
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

## <a name="Names"></a> Names

<a href="#Color">Color</a> constants can be helpful to write code, documenting the meaning of values
the represent transparency and color values. The use of <a href="#Color">Color</a> constants is not
required.

### Constants

<table>
  <tr>
    <td><a name="SK_AlphaTRANSPARENT"> <code><strong>SK_AlphaTRANSPARENT </strong></code> </a></td><td>0x00</td><td>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    constexpr <a href="#SkAlpha">SkAlpha</a> <a href="#SK_AlphaTRANSPARENT">SK AlphaTRANSPARENT</a> = 0x00;
</pre>

Represents fully transparent <a href="#SkAlpha">SkAlpha</a> value. <a href="#SkAlpha">SkAlpha</a> ranges from zero,
fully transparent; to 255, fully opaque.

### Example

<div><fiddle-embed name="b293529d2abe153e3a1dbe0410d21026"></table>

<div><a href="#Color">Color</a> the parts of the bitmap red if they mostly contain transparent pixels.
</div></fiddle-embed></div>

### See Also

<a href="#SkAlpha">SkAlpha</a> <a href="#SK_ColorTRANSPARENT">SK ColorTRANSPARENT</a> <a href="#SK_AlphaOPAQUE">SK AlphaOPAQUE</a>

</td>
  </tr>
</table>

### Constants

<table>
  <tr>
    <td><a name="SK_AlphaOPAQUE"> <code><strong>SK_AlphaOPAQUE </strong></code> </a></td><td>0xFF</td><td>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    constexpr <a href="#SkAlpha">SkAlpha</a> <a href="#SK_AlphaOPAQUE">SK AlphaOPAQUE</a>      = 0xFF;
</pre>

Represents fully opaque <a href="#SkAlpha">SkAlpha</a> value. <a href="#SkAlpha">SkAlpha</a> ranges from zero,
fully transparent; to 255, fully opaque.

### Example

<div><fiddle-embed name="0424f67ebc2858e8fd04ae3367b115ff"></table>

</fiddle-embed></div>

### See Also

<a href="#SkAlpha">SkAlpha</a> <a href="#SK_AlphaTRANSPARENT">SK AlphaTRANSPARENT</a>

</td>
  </tr>
</table>

<a href="#Color">Color</a> names are provided as conveniences, but are not otherwise special.
The values chosen for names may not be the same as values used by
<a href="undocumented#SVG">SVG</a>, HTML, CSS, or colors named by a platform.

### Example

<div><fiddle-embed name="1c2e38321464818847f953ddd45cb5a1"></fiddle-embed></div>

### Constants

<table>
  <tr>
    <td><a name="SK_ColorTRANSPARENT"> <code><strong>SK_ColorTRANSPARENT </strong></code> </a></td><td>0x00000000</td><td>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    constexpr <a href="#SkColor">SkColor</a> <a href="#SK_ColorTRANSPARENT">SK ColorTRANSPARENT</a> = <a href="#SkColorSetARGB">SkColorSetARGB(0x00, 0x00, 0x00, 0x00)</a>;
</pre>

Represents fully transparent <a href="#SkColor">SkColor</a>. May be used to initialize a destination
containing a mask or a non-rectangular image.

### Example

<div><fiddle-embed name="cfda8cd3b435bb28e2f4b9c7f15603a6"></fiddle-embed></div>

### See Also

<a href="#SK_AlphaTRANSPARENT">SK AlphaTRANSPARENT</a> <a href="SkCanvas_Reference#SkCanvas_clear">SkCanvas::clear</a>

</td>
  </tr>
</table>

### Constants

<table>
  <tr>
    <td><a name="SK_ColorBLACK"> <code><strong>SK_ColorBLACK </strong></code> </a></td><td>0xFF000000</td><td>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    constexpr <a href="#SkColor">SkColor</a> <a href="#SK_ColorBLACK">SK ColorBLACK</a>       = <a href="#SkColorSetARGB">SkColorSetARGB(0xFF, 0x00, 0x00, 0x00)</a>;
</pre>

Represents fully opaque black.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

</td>
  </tr>
</table>

### Constants

<table>
  <tr>
    <td><a name="SK_ColorDKGRAY"> <code><strong>SK_ColorDKGRAY </strong></code> </a></td><td>0xFF444444</td><td>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    constexpr <a href="#SkColor">SkColor</a> <a href="#SK_ColorDKGRAY">SK ColorDKGRAY</a>      = <a href="#SkColorSetARGB">SkColorSetARGB(0xFF, 0x44, 0x44, 0x44)</a>;
</pre>

Represents fully opaque dark gray.
Note that SVG_darkgray is equivalent to 0xFFA9A9A9.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

</td>
  </tr>
</table>

### Constants

<table>
  <tr>
    <td><a name="SK_ColorGRAY"> <code><strong>SK_ColorGRAY </strong></code> </a></td><td>0xFF888888</td><td>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    constexpr <a href="#SkColor">SkColor</a> <a href="#SK_ColorGRAY">SK ColorGRAY</a>        = <a href="#SkColorSetARGB">SkColorSetARGB(0xFF, 0x88, 0x88, 0x88)</a>;
</pre>

Represents fully opaque gray.
Note that HTML_Gray is equivalent to 0xFF808080.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

</td>
  </tr>
</table>

### Constants

<table>
  <tr>
    <td><a name="SK_ColorLTGRAY"> <code><strong>SK_ColorLTGRAY </strong></code> </a></td><td>0xFFCCCCCC</td><td>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    constexpr <a href="#SkColor">SkColor</a> <a href="#SK_ColorLTGRAY">SK ColorLTGRAY</a>      = <a href="#SkColorSetARGB">SkColorSetARGB(0xFF, 0xCC, 0xCC, 0xCC)</a>;
</pre>

Represents fully opaque light gray. HTML_Silver is equivalent to 0xFFC0C0C0.
Note that SVG_lightgray is equivalent to 0xFFD3D3D3.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

</td>
  </tr>
</table>

### Constants

<table>
  <tr>
    <td><a name="SK_ColorWHITE"> <code><strong>SK_ColorWHITE </strong></code> </a></td><td>0xFFFFFFFF</td><td>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    constexpr <a href="#SkColor">SkColor</a> <a href="#SK_ColorWHITE">SK ColorWHITE</a>       = <a href="#SkColorSetARGB">SkColorSetARGB(0xFF, 0xFF, 0xFF, 0xFF)</a>;
</pre>

Represents fully opaque white.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

</td>
  </tr>
</table>

### Constants

<table>
  <tr>
    <td><a name="SK_ColorRED"> <code><strong>SK_ColorRED </strong></code> </a></td><td>0xFFFF0000</td><td>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    constexpr <a href="#SkColor">SkColor</a> <a href="#SK_ColorRED">SK ColorRED</a>         = <a href="#SkColorSetARGB">SkColorSetARGB(0xFF, 0xFF, 0x00, 0x00)</a>;
</pre>

Represents fully opaque red.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

</td>
  </tr>
</table>

### Constants

<table>
  <tr>
    <td><a name="SK_ColorGREEN"> <code><strong>SK_ColorGREEN </strong></code> </a></td><td>0xFF00FF00</td><td>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    constexpr <a href="#SkColor">SkColor</a> <a href="#SK_ColorGREEN">SK ColorGREEN</a>       = <a href="#SkColorSetARGB">SkColorSetARGB(0xFF, 0x00, 0xFF, 0x00)</a>;
</pre>

Represents fully opaque green. HTML_Lime is equivalent.
Note that HTML_Green is equivalent to 0xFF008000.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

</td>
  </tr>
</table>

### Constants

<table>
  <tr>
    <td><a name="SK_ColorBLUE"> <code><strong>SK_ColorBLUE </strong></code> </a></td><td>0xFF0000FF</td><td>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    constexpr <a href="#SkColor">SkColor</a> <a href="#SK_ColorBLUE">SK ColorBLUE</a>        = <a href="#SkColorSetARGB">SkColorSetARGB(0xFF, 0x00, 0x00, 0xFF)</a>;
</pre>

Represents fully opaque blue.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

</td>
  </tr>
</table>

### Constants

<table>
  <tr>
    <td><a name="SK_ColorYELLOW"> <code><strong>SK_ColorYELLOW </strong></code> </a></td><td>0xFFFFFF00</td><td>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    constexpr <a href="#SkColor">SkColor</a> <a href="#SK_ColorYELLOW">SK ColorYELLOW</a>      = <a href="#SkColorSetARGB">SkColorSetARGB(0xFF, 0xFF, 0xFF, 0x00)</a>;
</pre>

Represents fully opaque yellow.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

</td>
  </tr>
</table>

### Constants

<table>
  <tr>
    <td><a name="SK_ColorCYAN"> <code><strong>SK_ColorCYAN </strong></code> </a></td><td>0xFF00FFFF</td><td>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    constexpr <a href="#SkColor">SkColor</a> <a href="#SK_ColorCYAN">SK ColorCYAN</a>        = <a href="#SkColorSetARGB">SkColorSetARGB(0xFF, 0x00, 0xFF, 0xFF)</a>;
</pre>

Represents fully opaque cyan. HTML_Aqua is equivalent.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

</td>
  </tr>
</table>

### Constants

<table>
  <tr>
    <td><a name="SK_ColorMAGENTA"> <code><strong>SK_ColorMAGENTA </strong></code> </a></td><td>0xFFFF00FF</td><td>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    constexpr <a href="#SkColor">SkColor</a> <a href="#SK_ColorMAGENTA">SK ColorMAGENTA</a>     = <a href="#SkColorSetARGB">SkColorSetARGB(0xFF, 0xFF, 0x00, 0xFF)</a>;
</pre>

Represents fully opaque magenta. HTML_Fuchsia is equivalent.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

</td>
  </tr>

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

