SkColor Reference
===


<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
typedef uint8_t <a href='SkColor_Reference#SkAlpha'>SkAlpha</a>;
typedef uint32_t <a href='SkColor_Reference#SkColor'>SkColor</a>;

static constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(<a href='undocumented#U8CPU'>U8CPU</a> a, <a href='undocumented#U8CPU'>U8CPU</a> r, <a href='undocumented#U8CPU'>U8CPU</a> g, <a href='undocumented#U8CPU'>U8CPU</a> b);
#define <a href='SkColor_Reference#SkColorSetRGB'>SkColorSetRGB</a>(r, g, b) <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0xFF, r, g, b)
#define <a href='SkColor_Reference#SkColorGetA'>SkColorGetA</a>(<a href='SkColor_Reference#Color'>color</a>) (((<a href='SkColor_Reference#Color'>color</a>) >> 24) & 0xFF)
#define <a href='SkColor_Reference#SkColorGetR'>SkColorGetR</a>(<a href='SkColor_Reference#Color'>color</a>) (((<a href='SkColor_Reference#Color'>color</a>) >> 16) & 0xFF)
#define <a href='SkColor_Reference#SkColorGetG'>SkColorGetG</a>(<a href='SkColor_Reference#Color'>color</a>) (((<a href='SkColor_Reference#Color'>color</a>) >> 8) & 0xFF)
#define <a href='SkColor_Reference#SkColorGetB'>SkColorGetB</a>(<a href='SkColor_Reference#Color'>color</a>) (((<a href='SkColor_Reference#Color'>color</a>) >> 0) & 0xFF)

static constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColorSetA'>SkColorSetA</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> c, <a href='undocumented#U8CPU'>U8CPU</a> a);
constexpr <a href='SkColor_Reference#SkAlpha'>SkAlpha</a> <a href='SkColor_Reference#SK_AlphaTRANSPARENT'>SK_AlphaTRANSPARENT</a> = 0x00;
constexpr <a href='SkColor_Reference#SkAlpha'>SkAlpha</a> <a href='SkColor_Reference#SK_AlphaOPAQUE'>SK_AlphaOPAQUE</a> = 0xFF;
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorTRANSPARENT'>SK_ColorTRANSPARENT</a> = <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0x00, 0x00, 0x00, 0x00);
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorBLACK'>SK_ColorBLACK</a> = <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0xFF, 0x00, 0x00, 0x00);
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorDKGRAY'>SK_ColorDKGRAY</a> = <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0xFF, 0x44, 0x44, 0x44);
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorGRAY'>SK_ColorGRAY</a> = <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0xFF, 0x88, 0x88, 0x88);
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorLTGRAY'>SK_ColorLTGRAY</a> = <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0xFF, 0xCC, 0xCC, 0xCC);
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorWHITE'>SK_ColorWHITE</a> = <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0xFF, 0xFF, 0xFF, 0xFF);
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorRED'>SK_ColorRED</a> = <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0xFF, 0xFF, 0x00, 0x00);
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorGREEN'>SK_ColorGREEN</a> = <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0xFF, 0x00, 0xFF, 0x00);
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorBLUE'>SK_ColorBLUE</a> = <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0xFF, 0x00, 0x00, 0xFF);
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorYELLOW'>SK_ColorYELLOW</a> = <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0xFF, 0xFF, 0xFF, 0x00);
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorCYAN'>SK_ColorCYAN</a> = <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0xFF, 0x00, 0xFF, 0xFF);
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorMAGENTA'>SK_ColorMAGENTA</a> = <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0xFF, 0xFF, 0x00, 0xFF);

void <a href='SkColor_Reference#SkRGBToHSV'>SkRGBToHSV</a>(<a href='undocumented#U8CPU'>U8CPU</a> red, <a href='undocumented#U8CPU'>U8CPU</a> green, <a href='undocumented#U8CPU'>U8CPU</a> blue, <a href='undocumented#SkScalar'>SkScalar</a> hsv[3]);

static void <a href='SkColor_Reference#SkColorToHSV'>SkColorToHSV</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#Color'>color</a>, <a href='undocumented#SkScalar'>SkScalar</a> hsv[3]);

<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkHSVToColor'>SkHSVToColor</a>(<a href='undocumented#U8CPU'>U8CPU</a> <a href='SkColor_Reference#Alpha'>alpha</a>, const <a href='undocumented#SkScalar'>SkScalar</a> hsv[3]);

static <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkHSVToColor'>SkHSVToColor</a>(const <a href='undocumented#SkScalar'>SkScalar</a> hsv[3]);
typedef uint32_t <a href='SkColor_Reference#SkPMColor'>SkPMColor</a>;

<a href='SkColor_Reference#SkPMColor'>SkPMColor</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>SkPreMultiplyARGB</a>(<a href='undocumented#U8CPU'>U8CPU</a> a, <a href='undocumented#U8CPU'>U8CPU</a> r, <a href='undocumented#U8CPU'>U8CPU</a> g, <a href='undocumented#U8CPU'>U8CPU</a> b);

<a href='SkColor_Reference#SkPMColor'>SkPMColor</a> <a href='SkColor_Reference#SkPreMultiplyColor'>SkPreMultiplyColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> c);
template <<a href='SkImageInfo_Reference#SkAlphaType'>SkAlphaType</a> kAT>
struct <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> {
    // <i><a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> interface</i>
};
template <> <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a> <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a>::<a href='SkColor4f_Reference#SkRGBA4f_FromColor'>FromColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a>);
template <> <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a>::<a href='SkColor4f_Reference#SkRGBA4f_toSkColor'>toSkColor</a>() const;
</pre>

<a href='SkColor_Reference#Color'>Color</a> constants can be helpful to write code, documenting the meaning of values
the represent transparency and <a href='SkColor_Reference#Color'>color</a> values. The use of <a href='SkColor_Reference#Color'>Color</a> constants is not
required.

<a name='Functions'></a>

<a name='Alpha'></a>

<a href='SkColor_Reference#Alpha'>Alpha</a> represents the transparency of <a href='SkColor_Reference#Color'>Color</a>. <a href='SkColor_Reference#Color'>Color</a> with <a href='SkColor_Reference#Alpha'>Alpha</a> of zero is fully
transparent. <a href='SkColor_Reference#Color'>Color</a> with <a href='SkColor_Reference#Alpha'>Alpha</a> of 255 is fully opaque. Some, but not all <a href='undocumented#Pixel'>pixel</a>
formats contain <a href='SkColor_Reference#Alpha'>Alpha</a>. Pixels with <a href='SkColor_Reference#Alpha'>Alpha</a> may store it as unsigned integers or
floating <a href='SkPoint_Reference#Point'>point</a> values. Unsigned integer <a href='SkColor_Reference#Alpha'>Alpha</a> ranges from zero, fully
transparent, to all bits set, fully opaque. Floating <a href='SkPoint_Reference#Point'>point</a> <a href='SkColor_Reference#Alpha'>Alpha</a> ranges from
zero, fully transparent, to one, fully opaque.

<a name='SkAlpha'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
typedef uint8_t <a href='SkColor_Reference#SkAlpha'>SkAlpha</a>;
</pre>

8-bit type for an <a href='SkColor_Reference#Alpha'>alpha</a> value. 255 is 100% opaque, zero is 100% transparent.

<a name='SkColor'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
typedef uint32_t <a href='SkColor_Reference#SkColor'>SkColor</a>;
</pre>

32-bit ARGB <a href='SkColor_Reference#Color'>Color</a> value, <a href='undocumented#Unpremultiply'>Unpremultiplied</a>. <a href='SkColor_Reference#Color'>Color</a> components are always in
a known order. This is different from <a href='SkColor_Reference#SkPMColor'>SkPMColor</a>, which has its bytes in a configuration
dependent order, to match the format of <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a> <a href='SkBitmap_Reference#Bitmap'>bitmaps</a>. <a href='SkColor_Reference#SkColor'>SkColor</a>
is the type used to specify colors in <a href='SkPaint_Reference#SkPaint'>SkPaint</a> and in gradients.

<a href='SkColor_Reference#Color'>Color</a> that is <a href='undocumented#Premultiply'>Premultiplied</a> has the same component values as <a href='SkColor_Reference#Color'>Color</a>
that is <a href='undocumented#Unpremultiply'>Unpremultiplied</a> if <a href='SkColor_Reference#Alpha'>Alpha</a> is 255, fully opaque, although may have the
component values in a different order.

### See Also

<a href='SkColor_Reference#SkPMColor'>SkPMColor</a>

<a name='SkColorSetARGB'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr inline <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(<a href='undocumented#U8CPU'>U8CPU</a> a, <a href='undocumented#U8CPU'>U8CPU</a> r, <a href='undocumented#U8CPU'>U8CPU</a> g, <a href='undocumented#U8CPU'>U8CPU</a> b)
</pre>

Returns <a href='SkColor_Reference#Color'>Color</a> value from 8-bit component values. Asserts if SK_DEBUG is defined
if <a href='#SkColorSetARGB_a'>a</a>, <a href='#SkColorSetARGB_r'>r</a>, <a href='#SkColorSetARGB_g'>g</a>, or <a href='#SkColorSetARGB_b'>b</a> exceed 255. Since <a href='SkColor_Reference#Color'>Color</a> is <a href='undocumented#Unpremultiply'>Unpremultiplied</a>, <a href='#SkColorSetARGB_a'>a</a> may be smaller
than the largest of <a href='#SkColorSetARGB_r'>r</a>, <a href='#SkColorSetARGB_g'>g</a>, and <a href='#SkColorSetARGB_b'>b</a>.

### Parameters

<table>  <tr>    <td><a name='SkColorSetARGB_a'><code><strong>a</strong></code></a></td>
    <td>amount of <a href='SkColor_Reference#Alpha'>Alpha</a>, from fully transparent (0) to fully opaque (255)</td>
  </tr>
  <tr>    <td><a name='SkColorSetARGB_r'><code><strong>r</strong></code></a></td>
    <td>amount of red, from no red (0) to full red (255)</td>
  </tr>
  <tr>    <td><a name='SkColorSetARGB_g'><code><strong>g</strong></code></a></td>
    <td>amount of green, from no green (0) to full green (255)</td>
  </tr>
  <tr>    <td><a name='SkColorSetARGB_b'><code><strong>b</strong></code></a></td>
    <td>amount of blue, from no blue (0) to full blue (255)</td>
  </tr>
</table>

### Return Value

<a href='SkColor_Reference#Color'>color</a> and <a href='SkColor_Reference#Alpha'>alpha</a>, <a href='undocumented#Unpremultiply'>Unpremultiplied</a>

### Example

<div><fiddle-embed name="35888f0869e01a6e03b5b93bba563734"></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkColorSetRGB'>SkColorSetRGB</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_setARGB'>setARGB</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_setColor'>setColor</a> <a href='SkColor_Reference#SkColorSetA'>SkColorSetA</a>

<a name='SkColorSetRGB'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#define <a href='SkColor_Reference#SkColorSetRGB'>SkColorSetRGB</a>(r, g, b) <a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>(0xFF, r, g, b)
r g b
</pre>

Returns <a href='SkColor_Reference#Color'>Color</a> value from 8-bit component values, with <a href='SkColor_Reference#Alpha'>Alpha</a> set
fully opaque to 255.

### Parameters

<table>  <tr>    <td><a name='SkColorSetRGB_r'><code><strong>r</strong></code></a></td>
    <td>amount of red, from no red (0) to full red (255)</td>
  </tr>
  <tr>    <td><a name='SkColorSetRGB_g'><code><strong>g</strong></code></a></td>
    <td>amount of green, from no green (0) to full green (255)</td>
  </tr>
  <tr>    <td><a name='SkColorSetRGB_b'><code><strong>b</strong></code></a></td>
    <td>amount of blue, from no blue (0) to full blue (255)</td>
  </tr>
</table>

### Return Value

<a href='SkColor_Reference#Color'>color</a> with opaque <a href='SkColor_Reference#Alpha'>alpha</a>

### Example

<div><fiddle-embed name="dad12dd912197cd5edd789ac0801bf8a"></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>

<a name='SkColorGetA'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#define <a href='SkColor_Reference#SkColorGetA'>SkColorGetA</a>(<a href='SkColor_Reference#Color'>color</a>) (((<a href='SkColor_Reference#Color'>color</a>) >> 24) & 0xFF)
<a href='SkColor_Reference#Color'>color</a>
</pre>

Returns <a href='SkColor_Reference#Alpha'>Alpha</a> byte from <a href='SkColor_Reference#Color'>Color</a> value.

### Parameters

<table>  <tr>    <td><a name='SkColorGetA_color'><code><strong>color</strong></code></a></td>
    <td><a href='SkColor_Reference#SkColor'>SkColor</a>, a 32-bit unsigned int, in 0xAARRGGBB format</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="896ce0316b489608a95af5439ca2aab1"></fiddle-embed></div>

### See Also

<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_getAlpha'>getAlpha</a>

<a name='SkColorGetR'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#define <a href='SkColor_Reference#SkColorGetR'>SkColorGetR</a>(<a href='SkColor_Reference#Color'>color</a>) (((<a href='SkColor_Reference#Color'>color</a>) >> 16) & 0xFF)
<a href='SkColor_Reference#Color'>color</a>
</pre>

Returns red component of <a href='SkColor_Reference#Color'>Color</a>, from zero to 255.

### Parameters

<table>  <tr>    <td><a name='SkColorGetR_color'><code><strong>color</strong></code></a></td>
    <td><a href='SkColor_Reference#SkColor'>SkColor</a>, a 32-bit unsigned int, in 0xAARRGGBB format</td>
  </tr>
</table>

### Return Value

red byte

### Example

<div><fiddle-embed name="d6da38577f189eaa6d9df75f6c3ed252"></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkColorGetG'>SkColorGetG</a> <a href='SkColor_Reference#SkColorGetB'>SkColorGetB</a>

<a name='SkColorGetG'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#define <a href='SkColor_Reference#SkColorGetG'>SkColorGetG</a>(<a href='SkColor_Reference#Color'>color</a>) (((<a href='SkColor_Reference#Color'>color</a>) >> 8) & 0xFF)
<a href='SkColor_Reference#Color'>color</a>
</pre>

Returns green component of <a href='SkColor_Reference#Color'>Color</a>, from zero to 255.

### Parameters

<table>  <tr>    <td><a name='SkColorGetG_color'><code><strong>color</strong></code></a></td>
    <td><a href='SkColor_Reference#SkColor'>SkColor</a>, a 32-bit unsigned int, in 0xAARRGGBB format</td>
  </tr>
</table>

### Return Value

green byte

### Example

<div><fiddle-embed name="535d38b2c019299d915170f7b03d5fea"></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkColorGetR'>SkColorGetR</a> <a href='SkColor_Reference#SkColorGetB'>SkColorGetB</a>

<a name='SkColorGetB'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#define <a href='SkColor_Reference#SkColorGetB'>SkColorGetB</a>(<a href='SkColor_Reference#Color'>color</a>) (((<a href='SkColor_Reference#Color'>color</a>) >> 0) & 0xFF)
<a href='SkColor_Reference#Color'>color</a>
</pre>

Returns blue component of <a href='SkColor_Reference#Color'>Color</a>, from zero to 255.

### Parameters

<table>  <tr>    <td><a name='SkColorGetB_color'><code><strong>color</strong></code></a></td>
    <td><a href='SkColor_Reference#SkColor'>SkColor</a>, a 32-bit unsigned int, in 0xAARRGGBB format</td>
  </tr>
</table>

### Return Value

blue byte

### Example

<div><fiddle-embed name="9ee27675284faea375611dc88123a2c5"></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkColorGetR'>SkColorGetR</a> <a href='SkColor_Reference#SkColorGetG'>SkColorGetG</a>

<a name='SkColorSetA'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr inline <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkColorSetA'>SkColorSetA</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> c, <a href='undocumented#U8CPU'>U8CPU</a> a)
</pre>

Returns <a href='undocumented#Unpremultiply'>Unpremultiplied</a> <a href='SkColor_Reference#Color'>Color</a> with red, blue, and green set from <a href='#SkColorSetA_c'>c</a>; and <a href='SkColor_Reference#Alpha'>alpha</a> set
from <a href='#SkColorSetA_a'>a</a>. <a href='SkColor_Reference#Alpha'>Alpha</a> component of <a href='#SkColorSetA_c'>c</a> is ignored and is replaced by <a href='#SkColorSetA_a'>a</a> in result.

### Parameters

<table>  <tr>    <td><a name='SkColorSetA_c'><code><strong>c</strong></code></a></td>
    <td>packed RGB, eight bits per component</td>
  </tr>
  <tr>    <td><a name='SkColorSetA_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkColor_Reference#Alpha'>Alpha</a>: transparent at zero, fully opaque at 255</td>
  </tr>
</table>

### Return Value

<a href='SkColor_Reference#Color'>Color</a> with transparency

### Example

<div><fiddle-embed name="18f6f376f771f5ffa56d5e5b2ebd20fb"></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkColorSetARGB'>SkColorSetARGB</a>

<a name='Alpha_Constants'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
constexpr <a href='SkColor_Reference#SkAlpha'>SkAlpha</a> <a href='SkColor_Reference#SK_AlphaTRANSPARENT'>SK_AlphaTRANSPARENT</a> = 0x00;
constexpr <a href='SkColor_Reference#SkAlpha'>SkAlpha</a> <a href='SkColor_Reference#SK_AlphaOPAQUE'>SK_AlphaOPAQUE</a> = 0xFF;
</pre>

<a href='SkColor_Reference#Alpha'>Alpha</a> constants are conveniences to represent fully transparent and fully
opaque colors and masks. Their use is not required.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_AlphaTRANSPARENT'><code>SK_AlphaTRANSPARENT</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x00</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully transparent <a href='SkColor_Reference#SkAlpha'>SkAlpha</a> value. <a href='SkColor_Reference#SkAlpha'>SkAlpha</a> ranges from zero,
fully transparent; to 255, fully opaque.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_AlphaOPAQUE'><code>SK_AlphaOPAQUE</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFF</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque <a href='SkColor_Reference#SkAlpha'>SkAlpha</a> value. <a href='SkColor_Reference#SkAlpha'>SkAlpha</a> ranges from zero,
fully transparent; to 255, fully opaque.
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="bc9c7ea424d10bbcd1e5a88770d4794e"><div><a href='SkColor_Reference#Color'>Color</a> the parts of the <a href='SkBitmap_Reference#Bitmap'>bitmap</a> red if they mostly contain transparent pixels.
</div></fiddle-embed></div>

### Example

<div><fiddle-embed name="0424f67ebc2858e8fd04ae3367b115ff"><div><a href='SkColor_Reference#Color'>Color</a> the parts of the <a href='SkBitmap_Reference#Bitmap'>bitmap</a> green if they contain fully opaque pixels.
</div></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkAlpha'>SkAlpha</a> <a href='SkColor_Reference#SK_ColorTRANSPARENT'>SK_ColorTRANSPARENT</a> <a href='SkColor_Reference#SK_ColorBLACK'>SK_ColorBLACK</a>

<a name='Color_Constants'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorTRANSPARENT'>SK_ColorTRANSPARENT</a>;
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorBLACK'>SK_ColorBLACK</a>;
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorDKGRAY'>SK_ColorDKGRAY</a>;
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorGRAY'>SK_ColorGRAY</a>;
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorLTGRAY'>SK_ColorLTGRAY</a>;
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorWHITE'>SK_ColorWHITE</a>;
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorRED'>SK_ColorRED</a>;
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorGREEN'>SK_ColorGREEN</a>;
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorBLUE'>SK_ColorBLUE</a>;
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorYELLOW'>SK_ColorYELLOW</a>;
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorCYAN'>SK_ColorCYAN</a>;
constexpr <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SK_ColorMAGENTA'>SK_ColorMAGENTA</a>;
</pre>

<a href='SkColor_Reference#Color'>Color</a> names are provided as conveniences, but are not otherwise special.
The values chosen for names may not be the same as values used by
<a href='undocumented#SVG'>SVG</a>, HTML, CSS, or colors named by a platform.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorTRANSPARENT'><code>SK_ColorTRANSPARENT</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0x00000000</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully transparent <a href='SkColor_Reference#SkColor'>SkColor</a>. May be used to initialize a destination
containing a mask or a non-rectangular <a href='SkImage_Reference#Image'>image</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorBLACK'><code>SK_ColorBLACK</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFF000000</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque black.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorDKGRAY'><code>SK_ColorDKGRAY</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFF444444</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque dark gray.
Note that <a href='#SVG_darkgray'>SVG_darkgray</a> is equivalent to 0xFFA9A9A9.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorGRAY'><code>SK_ColorGRAY</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFF888888</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque gray.
Note that <a href='#HTML_Gray'>HTML_Gray</a> is equivalent to 0xFF808080.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorLTGRAY'><code>SK_ColorLTGRAY</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFFCCCCCC</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque light gray. <a href='#HTML_Silver'>HTML_Silver</a> is equivalent to 0xFFC0C0C0.
Note that <a href='#SVG_lightgray'>SVG_lightgray</a> is equivalent to 0xFFD3D3D3.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorWHITE'><code>SK_ColorWHITE</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFFFFFFFF</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque white.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorRED'><code>SK_ColorRED</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFFFF0000</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque red.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorGREEN'><code>SK_ColorGREEN</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFF00FF00</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque green. <a href='#HTML_Lime'>HTML_Lime</a> is equivalent.
Note that <a href='#HTML_Green'>HTML_Green</a> is equivalent to 0xFF008000.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorBLUE'><code>SK_ColorBLUE</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFF0000FF</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque blue.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorYELLOW'><code>SK_ColorYELLOW</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFFFFFF00</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque yellow.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorCYAN'><code>SK_ColorCYAN</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFF00FFFF</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque cyan. <a href='#HTML_Aqua'>HTML_Aqua</a> is equivalent.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SK_ColorMAGENTA'><code>SK_ColorMAGENTA</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0xFFFF00FF</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Represents fully opaque magenta. <a href='#HTML_Fuchsia'>HTML_Fuchsia</a> is equivalent.
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1c2e38321464818847f953ddd45cb5a1"></fiddle-embed></div>

### Example

<div><fiddle-embed name="9ca1e2a5b9b4c92ecf4409d0813867d6"><div><a href='SkColor_Reference#SK_ColorTRANSPARENT'>SK_ColorTRANSPARENT</a> sets <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Alpha'>Alpha</a> and components to zero.
</div></fiddle-embed></div>

### Example

<div><fiddle-embed name="6971489f28291f08e429cc6ccc73b09b"><div><a href='SkColor_Reference#SK_ColorBLACK'>SK_ColorBLACK</a> sets <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Alpha'>Alpha</a> to one and components to zero.
</div></fiddle-embed></div>

### Example

<div><fiddle-embed name="fce650f997e802d4e55edf62b8437a2d"><div><a href='SkColor_Reference#SK_ColorWHITE'>SK_ColorWHITE</a> sets <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Alpha'>Alpha</a> and components to one.
</div></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SK_ColorTRANSPARENT'>SK_ColorTRANSPARENT</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_clear'>clear</a> <a href='SkColor_Reference#SK_AlphaOPAQUE'>SK_AlphaOPAQUE</a>

<a name='HSV'></a>

<a name='HSV_Hue'></a>

Hue represents an angle, in degrees, on a <a href='SkColor_Reference#Color'>color</a> wheel. Hue has a positive value
modulo 360, where zero degrees is red.

<a name='HSV_Saturation'></a>

<a href='undocumented#Saturation'>Saturation</a> represents the intensity of the <a href='SkColor_Reference#Color'>color</a>. <a href='undocumented#Saturation'>Saturation</a> varies from zero,
with no Hue contribution; to one, with full Hue contribution.

<a name='HSV_Value'></a>

Value represents the lightness of the <a href='SkColor_Reference#Color'>color</a>. Value varies from zero, black; to
one, full brightness.

<a name='SkRGBToHSV'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='SkColor_Reference#SkRGBToHSV'>SkRGBToHSV</a>(<a href='undocumented#U8CPU'>U8CPU</a> red, <a href='undocumented#U8CPU'>U8CPU</a> green, <a href='undocumented#U8CPU'>U8CPU</a> blue, <a href='undocumented#SkScalar'>SkScalar</a> hsv[3])
</pre>

Converts RGB to its HSV components.
<a href='#SkRGBToHSV_hsv'>hsv</a>[0] contains <a href='#Color_HSV_Hue'>HSV_Hue</a>, a value from zero to less than 360.
<a href='#SkRGBToHSV_hsv'>hsv</a>[1] contains <a href='#Color_HSV_Saturation'>HSV_Saturation</a>, a value from zero to one.
<a href='#SkRGBToHSV_hsv'>hsv</a>[2] contains <a href='#Color_HSV_Value'>HSV_Value</a>, a value from zero to one.

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
    <td>three element array which holds the resulting HSV components
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4fb2da4a3d9b14ca4ac24eefb0f5126a"></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkColorToHSV'>SkColorToHSV</a> <a href='SkColor_Reference#SkHSVToColor'>SkHSVToColor</a>

<a name='SkColorToHSV'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static void <a href='SkColor_Reference#SkColorToHSV'>SkColorToHSV</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#Color'>color</a>, <a href='undocumented#SkScalar'>SkScalar</a> hsv[3])
</pre>

Converts ARGB to its HSV components. <a href='SkColor_Reference#Alpha'>Alpha</a> in ARGB is ignored.
<a href='#SkColorToHSV_hsv'>hsv</a>[0] contains <a href='#Color_HSV_Hue'>HSV_Hue</a>, and is assigned a value from zero to less than 360.
<a href='#SkColorToHSV_hsv'>hsv</a>[1] contains <a href='#Color_HSV_Saturation'>HSV_Saturation</a>, a value from zero to one.
<a href='#SkColorToHSV_hsv'>hsv</a>[2] contains <a href='#Color_HSV_Value'>HSV_Value</a>, a value from zero to one.

### Parameters

<table>  <tr>    <td><a name='SkColorToHSV_color'><code><strong>color</strong></code></a></td>
    <td>ARGB <a href='#SkColorToHSV_color'>color</a> to convert
</td>
  </tr>
  <tr>    <td><a name='SkColorToHSV_hsv'><code><strong>hsv</strong></code></a></td>
    <td>three element array which holds the resulting HSV components
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1e0370f12c8aab5b84f9e824074f1e5a"></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkRGBToHSV'>SkRGBToHSV</a> <a href='SkColor_Reference#SkHSVToColor'>SkHSVToColor</a>

<a name='SkHSVToColor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkHSVToColor'>SkHSVToColor</a>(<a href='undocumented#U8CPU'>U8CPU</a> <a href='SkColor_Reference#Alpha'>alpha</a>, const <a href='undocumented#SkScalar'>SkScalar</a> hsv[3])
</pre>

Converts HSV components to an ARGB <a href='SkColor_Reference#Color'>color</a>. <a href='SkColor_Reference#Alpha'>Alpha</a> is passed through unchanged.
<a href='#SkHSVToColor_hsv'>hsv</a>[0] represents <a href='#Color_HSV_Hue'>HSV_Hue</a>, an angle from zero to less than 360.
<a href='#SkHSVToColor_hsv'>hsv</a>[1] represents <a href='#Color_HSV_Saturation'>HSV_Saturation</a>, and varies from zero to one.
<a href='#SkHSVToColor_hsv'>hsv</a>[2] represents <a href='#Color_HSV_Value'>HSV_Value</a>, and varies from zero to one.

Out of range <a href='#SkHSVToColor_hsv'>hsv</a> values are pinned.

### Parameters

<table>  <tr>    <td><a name='SkHSVToColor_alpha'><code><strong>alpha</strong></code></a></td>
    <td><a href='SkColor_Reference#Alpha'>Alpha</a> component of the returned ARGB <a href='SkColor_Reference#Color'>color </a>
</td>
  </tr>
  <tr>    <td><a name='SkHSVToColor_hsv'><code><strong>hsv</strong></code></a></td>
    <td>three element array which holds the input HSV components
</td>
  </tr>
</table>

### Return Value

ARGB equivalent to HSV

### Example

<div><fiddle-embed name="311a59931ac340b90f202cd6ac399a0a"></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkColorToHSV'>SkColorToHSV</a> <a href='SkColor_Reference#SkRGBToHSV'>SkRGBToHSV</a>

<a name='SkHSVToColor_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='SkColor_Reference#SkHSVToColor'>SkHSVToColor</a>(const <a href='undocumented#SkScalar'>SkScalar</a> hsv[3])
</pre>

Converts HSV components to an ARGB <a href='SkColor_Reference#Color'>color</a>. <a href='SkColor_Reference#Alpha'>Alpha</a> is set to 255.
<a href='#SkHSVToColor_2_hsv'>hsv</a>[0] represents <a href='#Color_HSV_Hue'>HSV_Hue</a>, an angle from zero to less than 360.
<a href='#SkHSVToColor_2_hsv'>hsv</a>[1] represents <a href='#Color_HSV_Saturation'>HSV_Saturation</a>, and varies from zero to one.
<a href='#SkHSVToColor_2_hsv'>hsv</a>[2] represents <a href='#Color_HSV_Value'>HSV_Value</a>, and varies from zero to one.

Out of range <a href='#SkHSVToColor_2_hsv'>hsv</a> values are pinned.

### Parameters

<table>  <tr>    <td><a name='SkHSVToColor_2_hsv'><code><strong>hsv</strong></code></a></td>
    <td>three element array which holds the input HSV components
</td>
  </tr>
</table>

### Return Value

RGB equivalent to HSV

### Example

<div><fiddle-embed name="d355a17547908cdbc2c38720974b5d11"></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkColorToHSV'>SkColorToHSV</a> <a href='SkColor_Reference#SkRGBToHSV'>SkRGBToHSV</a>

<a name='PM_Color'></a>

<a name='SkPMColor'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
typedef uint32_t <a href='SkColor_Reference#SkPMColor'>SkPMColor</a>;
</pre>

32-bit ARGB <a href='SkColor_Reference#Color'>color</a> value, <a href='undocumented#Premultiply'>Premultiplied</a>. The byte order for this value is
configuration dependent, matching the format of <a href='SkImageInfo_Reference#kBGRA_8888_SkColorType'>kBGRA_8888_SkColorType</a> <a href='SkBitmap_Reference#Bitmap'>bitmaps</a>.
This is different from <a href='SkColor_Reference#SkColor'>SkColor</a>, which is <a href='undocumented#Unpremultiply'>Unpremultiplied</a>, and is always in the
same byte order.

<a name='SkPreMultiplyARGB'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor_Reference#SkPMColor'>SkPMColor</a> <a href='SkColor_Reference#SkPreMultiplyARGB'>SkPreMultiplyARGB</a>(<a href='undocumented#U8CPU'>U8CPU</a> a, <a href='undocumented#U8CPU'>U8CPU</a> r, <a href='undocumented#U8CPU'>U8CPU</a> g, <a href='undocumented#U8CPU'>U8CPU</a> b)
</pre>

Returns <a href='#SkPreMultiplyARGB_a'>a</a> <a href='SkColor_Reference#SkPMColor'>SkPMColor</a> value from <a href='undocumented#Unpremultiply'>Unpremultiplied</a> 8-bit component values.

### Parameters

<table>  <tr>    <td><a name='SkPreMultiplyARGB_a'><code><strong>a</strong></code></a></td>
    <td>amount of <a href='SkColor_Reference#Alpha'>Alpha</a>, from fully transparent (0) to fully opaque (255)</td>
  </tr>
  <tr>    <td><a name='SkPreMultiplyARGB_r'><code><strong>r</strong></code></a></td>
    <td>amount of red, from no red (0) to full red (255)</td>
  </tr>
  <tr>    <td><a name='SkPreMultiplyARGB_g'><code><strong>g</strong></code></a></td>
    <td>amount of green, from no green (0) to full green (255)</td>
  </tr>
  <tr>    <td><a name='SkPreMultiplyARGB_b'><code><strong>b</strong></code></a></td>
    <td>amount of blue, from no blue (0) to full blue (255)</td>
  </tr>
</table>

### Return Value

<a href='undocumented#Premultiply'>Premultiplied</a> <a href='SkColor_Reference#Color'>Color</a>

### Example

<div><fiddle-embed name="756345484fd48ca0ea7b6cec350f73b8"></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkPreMultiplyColor'>SkPreMultiplyColor</a>

<a name='SkPreMultiplyColor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor_Reference#SkPMColor'>SkPMColor</a> <a href='SkColor_Reference#SkPreMultiplyColor'>SkPreMultiplyColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a> c)
</pre>

Returns <a href='#Color_PM_Color'>PM_Color</a> closest to <a href='SkColor_Reference#Color'>Color</a> <a href='#SkPreMultiplyColor_c'>c</a>. Multiplies <a href='#SkPreMultiplyColor_c'>c</a> RGB components by the <a href='#SkPreMultiplyColor_c'>c</a> <a href='SkColor_Reference#Alpha'>Alpha</a>,
and arranges the bytes to match the format of <a href='SkImageInfo_Reference#kN32_SkColorType'>kN32_SkColorType</a>.

### Parameters

<table>  <tr>    <td><a name='SkPreMultiplyColor_c'><code><strong>c</strong></code></a></td>
    <td><a href='undocumented#Unpremultiply'>Unpremultiplied</a> ARGB <a href='SkColor_Reference#Color'>Color</a></td>
  </tr>
</table>

### Return Value

<a href='undocumented#Premultiply'>Premultiplied</a> <a href='SkColor_Reference#Color'>Color</a>

### Example

<div><fiddle-embed name="0bcc0f86a2aefc899f3500503dce6968"></fiddle-embed></div>

### See Also

<a href='SkColor_Reference#SkPreMultiplyARGB'>SkPreMultiplyARGB</a>

