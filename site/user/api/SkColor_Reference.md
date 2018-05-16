SkColor Reference
===

# <a name='Color'>Color</a>

## Overview

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Define'>Defines</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>preprocessor definitions of functions, values</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Member_Function'>Functions</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>global and class member functions</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Related_Function'>Related Functions</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>similar member functions grouped together</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Typedef'>Typedef Declarations</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>types defined by other types</td>
  </tr>
</table>


## <a name='Define'>Define</a>


SkColor uses preprocessor definitions to inline code and constants, and to abstract platform-specific functionality.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkColorGetA'>SkColorGetA</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#Alpha'>Alpha</a> component</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkColorGetB'>SkColorGetB</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>incomplete</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkColorGetG'>SkColorGetG</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>incomplete</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkColorGetR'>SkColorGetR</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>incomplete</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkColorSetRGB'>SkColorSetRGB</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns opaque <a href='#Color'>Color</a></td>
  </tr>
</table>
<a href='#Color'>Color</a> constants can be helpful to write code, documenting the meaning of values
the represent transparency and color values. The use of <a href='#Color'>Color</a> constants is not
required.

## <a name='Constant'>Constant</a>


SkColor related constants are defined by <code>enum</code>, <code>enum class</code>,  <code>#define</code>, <code>const</code>, and <code>constexpr</code>.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SK_AlphaOPAQUE'>SK AlphaOPAQUE</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>fully opaque <a href='#SkAlpha'>SkAlpha</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SK_AlphaTRANSPARENT'>SK AlphaTRANSPARENT</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>fully transparent <a href='#SkAlpha'>SkAlpha</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SK_ColorBLACK'>SK ColorBLACK</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>black <a href='#Color'>Color</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SK_ColorBLUE'>SK ColorBLUE</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>blue <a href='#Color'>Color</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SK_ColorCYAN'>SK ColorCYAN</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>cyan <a href='#Color'>Color</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SK_ColorDKGRAY'>SK ColorDKGRAY</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>dark gray <a href='#Color'>Color</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SK_ColorGRAY'>SK ColorGRAY</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>gray <a href='#Color'>Color</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SK_ColorGREEN'>SK ColorGREEN</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>green <a href='#Color'>Color</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SK_ColorLTGRAY'>SK ColorLTGRAY</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>light gray <a href='#Color'>Color</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SK_ColorMAGENTA'>SK ColorMAGENTA</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>magenta <a href='#Color'>Color</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SK_ColorRED'>SK ColorRED</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>red <a href='#Color'>Color</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SK_ColorTRANSPARENT'>SK ColorTRANSPARENT</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>transparent <a href='#Color'>Color</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SK_ColorWHITE'>SK ColorWHITE</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>white <a href='#Color'>Color</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SK_ColorYELLOW'>SK ColorYELLOW</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>yellow <a href='#Color'>Color</a></td>
  </tr>
</table>


## <a name='Function'>Function</a>


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkColorSetA'>SkColorSetA</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>incomplete</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkColorSetARGB'>SkColorSetARGB</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#Alpha'>Color Alpha</a> and <a href='#RGB'>Color RGB</a> combined</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkColorToHSV'>SkColorToHSV</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>converts RGB to <a href='#HSV'>HSV</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkHSVToColor'>SkHSVToColor</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>converts <a href='#HSV'>HSV</a> with <a href='#Alpha'>Alpha</a> to RGB</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkHSVToColor'>SkHSVToColor(U8CPU alpha, const SkScalar hsv[3])</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPreMultiplyARGB'>SkPreMultiplyARGB</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>converts <a href='undocumented#Unpremultiply'>Unpremultiplied</a> <a href='#ARGB'>ARGB</a> to <a href='undocumented#Premultiply'>Premultiplied</a> <a href='#PMColor'>PMColor</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPreMultiplyColor'>SkPreMultiplyColor</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>converts <a href='undocumented#Unpremultiply'>Unpremultiplied</a> <a href='#Color'>Color</a> to <a href='undocumented#Premultiply'>Premultiplied</a> <a href='#PMColor'>PMColor</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkRGBToHSV'>SkRGBToHSV</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>incomplete</td>
  </tr>
</table>


## <a name='Typedef'>Typedef</a>


SkColor  <code>typedef</code> define a data type.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkAlpha'>SkAlpha</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>defines <a href='#Alpha'>Alpha</a> as eight bits</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkColor'>SkColor</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>defines <a href='#Color'>Color</a> as 32 bits</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPMColor'>SkPMColor</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>defines <a href='undocumented#Premultiply'>Premultiplied</a> <a href='#Color'>Color</a> as 32 bits</td>
  </tr>
</table>


## <a name='RGB'>RGB</a>

## <a name='RGB_Red'>RGB Red</a>

## <a name='RGB_Blue'>RGB Blue</a>

## <a name='RGB_Green'>RGB Green</a>

## <a name='ARGB'>ARGB</a>

## <a name='RBG'>RBG</a>

## <a name='RGB-565'>RGB-565</a>

## <a name='Gray'>Gray</a>

## <a name='Alpha'>Alpha</a>

<a href='#Alpha'>Alpha</a> represents the transparency of <a href='#Color'>Color</a>. <a href='#Color'>Color</a> with <a href='#Alpha'>Alpha</a> of zero is fully
transparent. <a href='#Color'>Color</a> with <a href='#Alpha'>Alpha</a> of 255 is fully opaque. Some, but not all pixel
formats contain <a href='#Alpha'>Alpha</a>. Pixels with <a href='#Alpha'>Alpha</a> may store it as unsigned integers or
floating point values. Unsigned integer <a href='#Alpha'>Alpha</a> ranges from zero, fully
transparent, to all bits set, fully opaque. Floating point <a href='#Alpha'>Alpha</a> ranges from
zero, fully transparent, to one, fully opaque.

# <a name='SkAlpha'>Typedef SkAlpha</a>
8-bit type for an alpha value. 0xFF is 100% opaque, 0x00 is 100% transparent.

# <a name='SkColor'>Typedef SkColor</a>
32-bit <a href='#ARGB'>ARGB</a> <a href='#Color'>Color</a> value, <a href='undocumented#Unpremultiply'>Unpremultiplied</a>. <a href='#Color'>Color</a> components are always in
a known order. This is different from <a href='#SkPMColor'>SkPMColor</a>, which has its bytes in a configuration
dependent order, to match the format of <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a> bitmaps. <a href='#SkColor'>SkColor</a>
is the type used to specify colors in <a href='SkPaint_Reference#SkPaint'>SkPaint</a> and in gradients.

<a href='#Color'>Color</a> that is <a href='undocumented#Premultiply'>Premultiplied</a> has the same component values as <a href='#Color'>Color</a>
that is <a href='undocumented#Unpremultiply'>Unpremultiplied</a> if <a href='#Alpha'>Alpha</a> is 255, fully opaque, although may have the
component values in a different order.

### See Also

<a href='#SkPMColor'>SkPMColor</a>

<a name='SkColorSetARGB'></a>
## SkColorSetARGB

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr inline <a href='#SkColor'>SkColor</a> <a href='#SkColorSetARGB'>SkColorSetARGB</a>(<a href='undocumented#U8CPU'>U8CPU</a> a, <a href='undocumented#U8CPU'>U8CPU</a> r, <a href='undocumented#U8CPU'>U8CPU</a> g, <a href='undocumented#U8CPU'>U8CPU</a> b)
</pre>

Returns <a href='#Color'>Color</a> value from 8-bit component values. Asserts if SK_DEBUG is defined
if <a href='#SkColorSetARGB_a'>a</a>, <a href='#SkColorSetARGB_r'>r</a>, <a href='#SkColorSetARGB_g'>g</a>, or <a href='#SkColorSetARGB_b'>b</a> exceed 255. Since <a href='#Color'>Color</a> is <a href='undocumented#Unpremultiply'>Unpremultiplied</a>, <a href='#SkColorSetARGB_a'>a</a> may be smaller
than the largest of <a href='#SkColorSetARGB_r'>r</a>, <a href='#SkColorSetARGB_g'>g</a>, and <a href='#SkColorSetARGB_b'>b</a>.

### Parameters

<table>  <tr>    <td><a name='SkColorSetARGB_a'><code><strong>a</strong></code></a></td>
    <td>amount of <a href='#Alpha'>Alpha</a>, from fully transparent (0) to fully opaque (255)</td>
  </tr>
  <tr>    <td><a name='SkColorSetARGB_r'><code><strong>r</strong></code></a></td>
    <td>amount of <a href='#RGB_Red'>RGB Red</a>, from no red (0) to full red (255)</td>
  </tr>
  <tr>    <td><a name='SkColorSetARGB_g'><code><strong>g</strong></code></a></td>
    <td>amount of <a href='#RGB_Green'>RGB Green</a>, from no green (0) to full green (255)</td>
  </tr>
  <tr>    <td><a name='SkColorSetARGB_b'><code><strong>b</strong></code></a></td>
    <td>amount of <a href='#RGB_Blue'>RGB Blue</a>, from no blue (0) to full blue (255)</td>
  </tr>
</table>

### Return Value

color and alpha, <a href='undocumented#Unpremultiply'>Unpremultiplied</a>

### Example

<div><fiddle-embed name="35888f0869e01a6e03b5b93bba563734"></fiddle-embed></div>

### See Also

<a href='#SkColorSetRGB'>SkColorSetRGB</a> <a href='SkPaint_Reference#SkPaint_setARGB'>SkPaint::setARGB</a> <a href='SkPaint_Reference#SkPaint_setColor'>SkPaint::setColor</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define <a href='#SkColorSetARGBInline'>SkColorSetARGBInline</a> <a href='#SkColorSetARGB'>SkColorSetARGB</a>
</pre>

Deprecated.

soon

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define <a href='#SkColorSetARGBMacro'>SkColorSetARGBMacro</a>  <a href='#SkColorSetARGB'>SkColorSetARGB</a>
</pre>

Deprecated.

soon

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define <a href='#SkColorSetRGB'>SkColorSetRGB(r, g, b)</a>  <a href='#SkColorSetARGB'>SkColorSetARGB(0xFF, r, g, b)</a>
</pre>

Returns <a href='#Color'>Color</a> value from 8-bit component values, with <a href='#Alpha'>Alpha</a> set
fully opaque to 255.

### Parameters

<table>  <tr>    <td><a name='SkColorSetRGB_r'><code><strong>r</strong></code></a></td>
    <td>amount of <a href='#RGB_Red'>RGB Red</a>, from no red (0) to full red (255)</td>
  </tr>
  <tr>    <td><a name='SkColorSetRGB_g'><code><strong>g</strong></code></a></td>
    <td>amount of <a href='#RGB_Green'>RGB Green</a>, from no green (0) to full green (255)</td>
  </tr>
  <tr>    <td><a name='SkColorSetRGB_b'><code><strong>b</strong></code></a></td>
    <td>amount of <a href='#RGB_Blue'>RGB Blue</a>, from no blue (0) to full blue (255)</td>
  </tr>
</table>

### Return Value

color with opaque alpha

### Example

<div><fiddle-embed name="dad12dd912197cd5edd789ac0801bf8a"></fiddle-embed></div>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define <a href='#SkColorGetA'>SkColorGetA(color)</a>      (((color) >> 24) & 0xFF)
</pre>

Returns <a href='#Alpha'>Alpha</a> byte from <a href='#Color'>Color</a> value.

### Parameters

<table>  <tr>    <td><a name='SkColorGetA_color'><code><strong>color</strong></code></a></td>
    <td><a href='#SkColor'>SkColor</a>, a 32-bit unsigned int, in 0xAARRGGBB format</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define <a href='#SkColorGetR'>SkColorGetR(color)</a>      (((color) >> 16) & 0xFF)
</pre>

Returns red component of <a href='#Color'>Color</a>, from zero to 255.

### Parameters

<table>  <tr>    <td><a name='SkColorGetR_color'><code><strong>color</strong></code></a></td>
    <td><a href='#SkColor'>SkColor</a>, a 32-bit unsigned int, in 0xAARRGGBB format</td>
  </tr>
</table>

### Return Value

red byte

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define <a href='#SkColorGetG'>SkColorGetG(color)</a>      (((color) >>  8) & 0xFF)
</pre>

Returns green component of <a href='#Color'>Color</a>, from zero to 255.

### Parameters

<table>  <tr>    <td><a name='SkColorGetG_color'><code><strong>color</strong></code></a></td>
    <td><a href='#SkColor'>SkColor</a>, a 32-bit unsigned int, in 0xAARRGGBB format</td>
  </tr>
</table>

### Return Value

green byte

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    #define <a href='#SkColorGetB'>SkColorGetB(color)</a>      (((color) >>  0) & 0xFF)
</pre>

Returns blue component of <a href='#Color'>Color</a>, from zero to 255.

### Parameters

<table>  <tr>    <td><a name='SkColorGetB_color'><code><strong>color</strong></code></a></td>
    <td><a href='#SkColor'>SkColor</a>, a 32-bit unsigned int, in 0xAARRGGBB format</td>
  </tr>
</table>

### Return Value

blue byte

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkColorSetA'></a>
## SkColorSetA

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr inline <a href='#SkColor'>SkColor</a> <a href='#SkColorSetA'>SkColorSetA</a>(<a href='#SkColor'>SkColor</a> c, <a href='undocumented#U8CPU'>U8CPU</a> a)
</pre>

Returns <a href='#Color'>Color</a> with red, blue, and green set from <a href='#SkColorSetA_c'>c</a>; and alpha set from <a href='#SkColorSetA_a'>a</a>.

### Parameters

<table>  <tr>    <td><a name='SkColorSetA_c'><code><strong>c</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>Unpremultiplied</a> <a href='#ARGB'>Color ARGB</a></td>
  </tr>
  <tr>    <td><a name='SkColorSetA_a'><code><strong>a</strong></code></a></td>
    <td>incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

## <a name='Alpha_Constants'>Alpha Constants</a>



### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Color_Alpha_Constants'><code>Alpha_Constants</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#In Constant</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>fully transparent SkAlpha</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Color_Alpha_Constants'><code>Alpha_Constants</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#In Constant</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>fully opaque SkAlpha</td>
  </tr>
</table>


<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    constexpr <a href='#SkAlpha'>SkAlpha</a> <a href='#SK_AlphaTRANSPARENT'>SK AlphaTRANSPARENT</a> = 0x00;
    constexpr <a href='#SkAlpha'>SkAlpha</a> <a href='#SK_AlphaOPAQUE'>SK AlphaOPAQUE</a>      = 0xFF;
</pre>

<a href='#Alpha'>Alpha</a> constants are conveniences to represent fully transparent and fully
opaque colors and masks. Their use is not required.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Details</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_AlphaTRANSPARENT'><code>SK_AlphaTRANSPARENT</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x00</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Alpha_Constants_Transparent'>Alpha&nbsp;Constants&nbsp;Transparent</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully transparent <a href='#SkAlpha'>SkAlpha</a> value. <a href='#SkAlpha'>SkAlpha</a> ranges from zero,
fully transparent; to 255, fully opaque.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_AlphaOPAQUE'><code>SK_AlphaOPAQUE</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFF</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Alpha_Constants_Opaque'>Alpha&nbsp;Constants&nbsp;Opaque</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque <a href='#SkAlpha'>SkAlpha</a> value. <a href='#SkAlpha'>SkAlpha</a> ranges from zero,
fully transparent; to 255, fully opaque.
</td>
  </tr>
</table>

## <a name='Alpha_Constants_Transparent'>Alpha Constants Transparent</a>

### Example

<div><fiddle-embed name="bc9c7ea424d10bbcd1e5a88770d4794e"><div><a href='#Color'>Color</a> the parts of the bitmap red if they mostly contain transparent pixels.
</div></fiddle-embed></div>

### See Also

<a href='#SkAlpha'>SkAlpha</a> <a href='#SK_ColorTRANSPARENT'>SK ColorTRANSPARENT</a> <a href='#SK_AlphaOPAQUE'>SK AlphaOPAQUE</a>

## <a name='Alpha_Constants_Opaque'>Alpha Constants Opaque</a>

### Example

<div><fiddle-embed name="0424f67ebc2858e8fd04ae3367b115ff"></fiddle-embed></div>

### See Also

<a href='#SkAlpha'>SkAlpha</a> <a href='#SK_AlphaTRANSPARENT'>SK AlphaTRANSPARENT</a>

## <a name='Color_Constants'>Color Constants</a>



### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Color_Color_Constants'><code>Color_Constants</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#In Constant</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>transparent Color</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Color_Color_Constants'><code>Color_Constants</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#In Constant</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>black Color</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Color_Color_Constants'><code>Color_Constants</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#In Constant</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>dark gray Color</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Color_Color_Constants'><code>Color_Constants</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#In Constant</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>gray Color</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Color_Color_Constants'><code>Color_Constants</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#In Constant</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>light gray Color</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Color_Color_Constants'><code>Color_Constants</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#In Constant</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>white Color</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Color_Color_Constants'><code>Color_Constants</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#In Constant</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>red Color</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Color_Color_Constants'><code>Color_Constants</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#In Constant</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>green Color</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Color_Color_Constants'><code>Color_Constants</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#In Constant</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>blue Color</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Color_Color_Constants'><code>Color_Constants</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#In Constant</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>yellow Color</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Color_Color_Constants'><code>Color_Constants</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#In Constant</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>cyan Color</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Color_Color_Constants'><code>Color_Constants</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#In Constant</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>magenta Color</td>
  </tr>
</table>


<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    constexpr <a href='#SkColor'>SkColor</a> <a href='#SK_ColorTRANSPARENT'>SK ColorTRANSPARENT</a> = <a href='#SkColorSetARGB'>SkColorSetARGB(0x00, 0x00, 0x00, 0x00)</a>;
    constexpr <a href='#SkColor'>SkColor</a> <a href='#SK_ColorBLACK'>SK ColorBLACK</a>       = <a href='#SkColorSetARGB'>SkColorSetARGB(0xFF, 0x00, 0x00, 0x00)</a>;
    constexpr <a href='#SkColor'>SkColor</a> <a href='#SK_ColorDKGRAY'>SK ColorDKGRAY</a>      = <a href='#SkColorSetARGB'>SkColorSetARGB(0xFF, 0x44, 0x44, 0x44)</a>;
    constexpr <a href='#SkColor'>SkColor</a> <a href='#SK_ColorGRAY'>SK ColorGRAY</a>        = <a href='#SkColorSetARGB'>SkColorSetARGB(0xFF, 0x88, 0x88, 0x88)</a>;
    constexpr <a href='#SkColor'>SkColor</a> <a href='#SK_ColorLTGRAY'>SK ColorLTGRAY</a>      = <a href='#SkColorSetARGB'>SkColorSetARGB(0xFF, 0xCC, 0xCC, 0xCC)</a>;
    constexpr <a href='#SkColor'>SkColor</a> <a href='#SK_ColorWHITE'>SK ColorWHITE</a>       = <a href='#SkColorSetARGB'>SkColorSetARGB(0xFF, 0xFF, 0xFF, 0xFF)</a>;
    constexpr <a href='#SkColor'>SkColor</a> <a href='#SK_ColorRED'>SK ColorRED</a>         = <a href='#SkColorSetARGB'>SkColorSetARGB(0xFF, 0xFF, 0x00, 0x00)</a>;
    constexpr <a href='#SkColor'>SkColor</a> <a href='#SK_ColorGREEN'>SK ColorGREEN</a>       = <a href='#SkColorSetARGB'>SkColorSetARGB(0xFF, 0x00, 0xFF, 0x00)</a>;
    constexpr <a href='#SkColor'>SkColor</a> <a href='#SK_ColorBLUE'>SK ColorBLUE</a>        = <a href='#SkColorSetARGB'>SkColorSetARGB(0xFF, 0x00, 0x00, 0xFF)</a>;
    constexpr <a href='#SkColor'>SkColor</a> <a href='#SK_ColorYELLOW'>SK ColorYELLOW</a>      = <a href='#SkColorSetARGB'>SkColorSetARGB(0xFF, 0xFF, 0xFF, 0x00)</a>;
    constexpr <a href='#SkColor'>SkColor</a> <a href='#SK_ColorCYAN'>SK ColorCYAN</a>        = <a href='#SkColorSetARGB'>SkColorSetARGB(0xFF, 0x00, 0xFF, 0xFF)</a>;
    constexpr <a href='#SkColor'>SkColor</a> <a href='#SK_ColorMAGENTA'>SK ColorMAGENTA</a>     = <a href='#SkColorSetARGB'>SkColorSetARGB(0xFF, 0xFF, 0x00, 0xFF)</a>;
</pre>

<a href='#Color'>Color</a> names are provided as conveniences, but are not otherwise special.
The values chosen for names may not be the same as values used by
<a href='undocumented#SVG'>SVG</a>, HTML, CSS, or colors named by a platform.

### Example

<div><fiddle-embed name="1c2e38321464818847f953ddd45cb5a1"></fiddle-embed></div>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Details</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorTRANSPARENT'><code>SK_ColorTRANSPARENT</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x00000000</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Color_Constants_Transparent'>Color&nbsp;Constants&nbsp;Transparent</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully transparent <a href='#SkColor'>SkColor</a>. May be used to initialize a destination
containing a mask or a non-rectangular image.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorBLACK'><code>SK_ColorBLACK</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFF000000</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Color_Constants_Black'>Color&nbsp;Constants&nbsp;Black</a>&nbsp;</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque black.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorDKGRAY'><code>SK_ColorDKGRAY</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFF444444</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque dark gray.
Note that SVG_darkgray is equivalent to 0xFFA9A9A9.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorGRAY'><code>SK_ColorGRAY</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFF888888</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque gray.
Note that HTML_Gray is equivalent to 0xFF808080.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorLTGRAY'><code>SK_ColorLTGRAY</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFFCCCCCC</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque light gray. HTML_Silver is equivalent to 0xFFC0C0C0.
Note that SVG_lightgray is equivalent to 0xFFD3D3D3.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorWHITE'><code>SK_ColorWHITE</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFFFFFFFF</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque white.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorRED'><code>SK_ColorRED</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFFFF0000</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque red.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorGREEN'><code>SK_ColorGREEN</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFF00FF00</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque green. HTML_Lime is equivalent.
Note that HTML_Green is equivalent to 0xFF008000.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorBLUE'><code>SK_ColorBLUE</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFF0000FF</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque blue.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorYELLOW'><code>SK_ColorYELLOW</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFFFFFF00</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque yellow.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorCYAN'><code>SK_ColorCYAN</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFF00FFFF</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque cyan. HTML_Aqua is equivalent.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorMAGENTA'><code>SK_ColorMAGENTA</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFFFF00FF</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque magenta. HTML_Fuchsia is equivalent.
</td>
  </tr>
</table>

## <a name='Color_Constants_Transparent'>Color Constants Transparent</a>

### Example

<div><fiddle-embed name="cfda8cd3b435bb28e2f4b9c7f15603a6"></fiddle-embed></div>

### See Also

<a href='#SK_AlphaTRANSPARENT'>SK AlphaTRANSPARENT</a> <a href='SkCanvas_Reference#SkCanvas_clear'>SkCanvas::clear</a>

## <a name='Color_Constants_Black'>Color Constants Black</a>

### Example

<div><fiddle-embed name="6971489f28291f08e429cc6ccc73b09b"></fiddle-embed></div>

### See Also

<a href='#SK_ColorTRANSPARENT'>SK ColorTRANSPARENT</a>

## <a name='Color_Constants_White'>Color Constants White</a>

### Example

<div><fiddle-embed name="fce650f997e802d4e55edf62b8437a2d"></fiddle-embed></div>

### See Also

<a href='#SK_ColorTRANSPARENT'>SK ColorTRANSPARENT</a>

## <a name='HSV'>HSV</a>

## <a name='HSV_Hue'>HSV Hue</a>

<a href='#HSV_Hue'>Hue</a> represents an angle, in degrees, on a color wheel. <a href='#HSV_Hue'>Hue</a> has a positive value
modulo 360, where zero degrees is red.

## <a name='HSV_Saturation'>HSV Saturation</a>

## <a name='HSV_Value'>HSV Value</a>

<a name='SkRGBToHSV'></a>
## SkRGBToHSV

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
SK_API void <a href='#SkRGBToHSV'>SkRGBToHSV</a>(<a href='undocumented#U8CPU'>U8CPU</a> red, <a href='undocumented#U8CPU'>U8CPU</a> green, <a href='undocumented#U8CPU'>U8CPU</a> blue, <a href='undocumented#SkScalar'>SkScalar</a> hsv[3])
</pre>

Converts RGB components to <a href='#HSV'>HSV</a>.
<a href='#SkRGBToHSV_hsv'>hsv</a>[0] contains <a href='#HSV_Hue'>Hue</a>, a value from zero to less than 360.
<a href='#SkRGBToHSV_hsv'>hsv</a>[1] contains <a href='#HSV_Saturation'>Saturation</a>, a value from zero to one.
<a href='#SkRGBToHSV_hsv'>hsv</a>[2] contains <a href='#HSV_Value'>Value</a>, a value from zero to one.

### Parameters

<table>  <tr>    <td><a name='SkRGBToHSV_red'><code><strong>red</strong></code></a></td>
    <td><a href='#SkRGBToHSV_red'>red</a> component value from zero to 255</td>
  </tr>
  <tr>    <td><a name='SkRGBToHSV_green'><code><strong>green</strong></code></a></td>
    <td><a href='#SkRGBToHSV_green'>green</a> component value from zero to 255</td>
  </tr>
  <tr>    <td><a name='SkRGBToHSV_blue'><code><strong>blue</strong></code></a></td>
    <td><a href='#SkRGBToHSV_blue'>blue</a> component value from zero to 255</td>
  </tr>
  <tr>    <td><a name='SkRGBToHSV_hsv'><code><strong>hsv</strong></code></a></td>
    <td>three element array which holds the resulting <a href='#HSV'>HSV</a> components</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name='SkColorToHSV'></a>
## SkColorToHSV

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static inline void <a href='#SkColorToHSV'>SkColorToHSV</a>(<a href='#SkColor'>SkColor</a> color, <a href='undocumented#SkScalar'>SkScalar</a> hsv[3])
</pre>

Converts <a href='#ARGB'>ARGB</a> to its <a href='#HSV'>HSV</a> components. <a href='#Alpha'>Alpha</a> in <a href='#ARGB'>ARGB</a> is ignored.
<a href='#SkColorToHSV_hsv'>hsv</a>[0] contains <a href='#HSV_Hue'>Hue</a>, and is assigned a value from zero to less than 360.
<a href='#SkColorToHSV_hsv'>hsv</a>[1] contains <a href='#HSV_Saturation'>Saturation</a>, a value from zero to one.
<a href='#SkColorToHSV_hsv'>hsv</a>[2] contains <a href='#HSV_Value'>Value</a>, a value from zero to one.

### Parameters

<table>  <tr>    <td><a name='SkColorToHSV_color'><code><strong>color</strong></code></a></td>
    <td><a href='#ARGB'>ARGB</a> <a href='#SkColorToHSV_color'>color</a> to convert</td>
  </tr>
  <tr>    <td><a name='SkColorToHSV_hsv'><code><strong>hsv</strong></code></a></td>
    <td>three element array which holds the resulting <a href='#HSV'>HSV</a> components</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name='SkHSVToColor'></a>
## SkHSVToColor

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
SK_API <a href='#SkColor'>SkColor</a> <a href='#SkHSVToColor'>SkHSVToColor</a>(<a href='undocumented#U8CPU'>U8CPU</a> alpha, const <a href='undocumented#SkScalar'>SkScalar</a> hsv[3])
</pre>

Converts <a href='#HSV'>HSV</a> components to an <a href='#ARGB'>ARGB</a> color. The <a href='#SkHSVToColor_alpha'>alpha</a> component is passed through unchanged.
<a href='#SkHSVToColor_hsv'>hsv</a>[0] represents <a href='#HSV_Hue'>Hue</a>, an angle from zero to less than 360.
<a href='#SkHSVToColor_hsv'>hsv</a>[1] represents <a href='#HSV_Saturation'>Saturation</a>, and varies from zero to one.
<a href='#SkHSVToColor_hsv'>hsv</a>[2] represents <a href='#HSV_Value'>Value</a>, and varies from zero to one.

If <a href='#SkHSVToColor_hsv'>hsv</a> values are out of range, they are pinned.

### Parameters

<table>  <tr>    <td><a name='SkHSVToColor_alpha'><code><strong>alpha</strong></code></a></td>
    <td><a href='#Alpha'>Alpha</a> component of the returned <a href='#ARGB'>ARGB</a> color</td>
  </tr>
  <tr>    <td><a name='SkHSVToColor_hsv'><code><strong>hsv</strong></code></a></td>
    <td>three element array which holds the input <a href='#HSV'>HSV</a> components</td>
  </tr>
</table>

### Return Value

<a href='#ARGB'>ARGB</a> equivalent to <a href='#HSV'>HSV</a>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name='SkHSVToColor_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static inline <a href='#SkColor'>SkColor</a> <a href='#SkHSVToColor'>SkHSVToColor</a>(const <a href='undocumented#SkScalar'>SkScalar</a> hsv[3])
</pre>

Convert <a href='#HSV'>HSV</a> components to an <a href='#ARGB'>ARGB</a> color. The alpha component set to 0xFF.
<a href='#SkHSVToColor_2_hsv'>hsv</a>[0] represents <a href='#HSV_Hue'>Hue</a>, an angle from zero to less than 360.
<a href='#SkHSVToColor_2_hsv'>hsv</a>[1] represents <a href='#HSV_Saturation'>Saturation</a>, and varies from zero to one.
<a href='#SkHSVToColor_2_hsv'>hsv</a>[2] represents <a href='#HSV_Value'>Value</a>, and varies from zero to one.

If <a href='#SkHSVToColor_2_hsv'>hsv</a> values are out of range, they are pinned.

### Parameters

<table>  <tr>    <td><a name='SkHSVToColor_2_hsv'><code><strong>hsv</strong></code></a></td>
    <td>3 element array which holds the input <a href='#HSV'>HSV</a> components.
</td>
  </tr>
</table>

### Return Value

the resulting <a href='#ARGB'>ARGB</a> color

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

## <a name='PMColor'>PMColor</a>

# <a name='SkPMColor'>Typedef SkPMColor</a>
32-bit <a href='#ARGB'>ARGB</a> color value, <a href='undocumented#Premultiply'>Premultiplied</a>. The byte order for this value is
configuration dependent, matching the format of <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a> bitmaps.
This is different from <a href='#SkColor'>SkColor</a>, which is <a href='undocumented#Unpremultiply'>Unpremultiplied</a>, and is always in the
same byte order.

<a name='SkPreMultiplyARGB'></a>
## SkPreMultiplyARGB

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
SK_API <a href='#SkPMColor'>SkPMColor</a> <a href='#SkPreMultiplyARGB'>SkPreMultiplyARGB</a>(<a href='undocumented#U8CPU'>U8CPU</a> a, <a href='undocumented#U8CPU'>U8CPU</a> r, <a href='undocumented#U8CPU'>U8CPU</a> g, <a href='undocumented#U8CPU'>U8CPU</a> b)
</pre>

Return <a href='#SkPreMultiplyARGB_a'>a</a> <a href='#SkPMColor'>SkPMColor</a> value from <a href='undocumented#Unpremultiply'>Unpremultiplied</a> 8-bit component values

### Parameters

<table>  <tr>    <td><a name='SkPreMultiplyARGB_a'><code><strong>a</strong></code></a></td>
    <td>incomplete</td>
  </tr>
  <tr>    <td><a name='SkPreMultiplyARGB_r'><code><strong>r</strong></code></a></td>
    <td>incomplete</td>
  </tr>
  <tr>    <td><a name='SkPreMultiplyARGB_g'><code><strong>g</strong></code></a></td>
    <td>incomplete</td>
  </tr>
  <tr>    <td><a name='SkPreMultiplyARGB_b'><code><strong>b</strong></code></a></td>
    <td>incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name='SkPreMultiplyColor'></a>
## SkPreMultiplyColor

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
SK_API <a href='#SkPMColor'>SkPMColor</a> <a href='#SkPreMultiplyColor'>SkPreMultiplyColor</a>(<a href='#SkColor'>SkColor</a> c)
</pre>

Returns <a href='#PMColor'>PMColor</a> closest to <a href='#Color'>Color</a> <a href='#SkPreMultiplyColor_c'>c</a>. Multiplies <a href='#SkPreMultiplyColor_c'>c</a> RGB components by the <a href='#SkPreMultiplyColor_c'>c</a> <a href='#Alpha'>Alpha</a>,
and arranges the bytes to match the format of <a href='SkImageInfo_Reference#kN32_SkColorType'>kN32_SkColorType</a>.

### Parameters

<table>  <tr>    <td><a name='SkPreMultiplyColor_c'><code><strong>c</strong></code></a></td>
    <td>incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

