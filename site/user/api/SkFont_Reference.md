SkFont Reference
===


<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkFont_Reference#SkFont'>SkFont</a> {
    // <i><a href='SkFont_Reference#SkFont'>SkFont</a> interface</i>
};
</pre>

<a name='Advance'></a>

<a name='Engine'></a>

<a name='Size'></a>

<a href='#Font_Size'>Font_Size</a> adjusts the overall <a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a> in <a href='SkPoint_Reference#Point'>points</a>.
<a href='#Font_Size'>Font_Size</a> can be set to any positive value or zero.
<a href='#Font_Size'>Font_Size</a> defaults to 12.
<a href='#Font_Size'>Font_Size</a>

<a name='Scale_X'></a>

<a href='#Font_Scale_X'>Font_Scale_X</a> adjusts the <a href='undocumented#Text'>text</a> horizontal scale.
<a href='undocumented#Text'>Text</a> scaling approximates condensed and expanded type faces when the actual face
is not available.
<a href='#Font_Scale_X'>Font_Scale_X</a> can be set to any value.
<a href='#Font_Scale_X'>Font_Scale_X</a> defaults to 1.

<a name='Skew_X'></a>

<a href='#Font_Skew_X'>Font_Skew_X</a> adjusts the <a href='undocumented#Text'>text</a> horizontal slant.
<a href='undocumented#Text'>Text</a> skewing approximates italic and oblique type faces when the actual face
is not available.
<a href='#Font_Skew_X'>Font_Skew_X</a> can be set to any value.
<a href='#Font_Skew_X'>Font_Skew_X</a> defaults to 0.

<a name='Embolden'></a>

<a href='#Font_Embolden'>Font_Embolden</a> approximates the bold <a href='SkFont_Reference#Font'>font</a> style accompanying a normal <a href='SkFont_Reference#Font'>font</a> when a bold <a href='SkFont_Reference#Font'>font</a> face
is not available. Skia does not provide <a href='SkFont_Reference#Font'>font</a> substitution; it is up to the client to find the
bold <a href='SkFont_Reference#Font'>font</a> face using the platform <a href='#Font_Manager'>Font_Manager</a>.

Use <a href='#Font_Skew_X'>Font_Skew_X</a> to approximate an italic <a href='SkFont_Reference#Font'>font</a> style when the italic <a href='SkFont_Reference#Font'>font</a> face
is not available.

A FreeType based port may define SK_USE_FREETYPE_EMBOLDEN at compile time to direct
the  <a href='SkFont_Reference#Font_Engine'>font engine</a> to create the bold <a href='undocumented#Glyph'>Glyphs</a>. Otherwise, the extra bold is computed
by increasing the stroke width and setting the <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> to
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kStrokeAndFill_Style'>kStrokeAndFill_Style</a> as needed.

<a href='#Font_Embolden'>Font_Embolden</a> is disabled by default.

<a name='Hinting_Spacing'></a>

If Hinting is set to <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kFull'>kFull</a>, <a href='#Font_Hinting_Spacing'>Hinting_Spacing</a> adjusts the character
spacing by the difference of the hinted and unhinted <a href='#Left_Side_Bearing'>Left_Side_Bearing</a> and
<a href='#Right_Side_Bearing'>Right_Side_Bearing</a>. <a href='#Font_Hinting_Spacing'>Hinting_Spacing</a> only applies to platforms that use
FreeType as their <a href='#Font_Engine'>Font_Engine</a>.

<a href='#Font_Hinting_Spacing'>Hinting_Spacing</a> is not related to <a href='undocumented#Text'>text</a> kerning, where the space between
a specific pair of characters is adjusted using <a href='undocumented#Data'>data</a> in the <a href='SkFont_Reference#Font'>font</a> kerning tables.

<a name='Linear'></a>

<a href='#Font_Linear'>Font_Linear</a> selects whether <a href='undocumented#Text'>text</a> is rendered as a <a href='undocumented#Glyph'>Glyph</a> or as a <a href='SkPath_Reference#Path'>Path</a>.
If <a href='#Font_Linear'>Font_Linear</a> is set, it has the same effect as setting Hinting to <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kNormal'>kNormal</a>.
If <a href='#Font_Linear'>Font_Linear</a> is clear, it is the same as setting Hinting to <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kNone'>kNone</a>.

<a name='Subpixel'></a>

<a href='#Font_Subpixel'>Font_Subpixel</a> uses the <a href='undocumented#Pixel'>pixel</a> transparency to represent a fractional offset.
As the opaqueness of the <a href='SkColor_Reference#Color'>color</a> increases, the edge of the <a href='undocumented#Glyph'>glyph</a> appears to move
towards the outside of the <a href='undocumented#Pixel'>pixel</a>.

<a name='Anti_Alias'></a>

When set, <a href='#Paint_Anti_Alias'>Anti_Alias</a> positions <a href='undocumented#Glyph'>glyphs</a> within a <a href='undocumented#Pixel'>pixel</a>, using <a href='SkColor_Reference#Alpha'>alpha</a> and
possibly RGB striping. It can take advantage of the organization of RGB stripes
that create a <a href='SkColor_Reference#Color'>color</a>, and relies on the small <a href='undocumented#Size'>size</a> of the stripe and visual perception
to make the <a href='SkColor_Reference#Color'>color</a> fringing imperceptible.

<a href='#Paint_Anti_Alias'>Anti_Alias</a> can be enabled on devices that orient stripes horizontally
or vertically, and that order the <a href='SkColor_Reference#Color'>color</a> components as RGB or BGR. Internally, the
<a href='undocumented#Glyph'>glyph</a> cache may store multiple copies of the same <a href='undocumented#Glyph'>glyph</a> with different <a href='SkFont_Reference#Subpixel'>sub-pixel</a>
positions, requiring more memory.

<a name='Force_Hinting'></a>

If Hinting is set to <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kNormal'>kNormal</a> or <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kFull'>kFull</a>, <a href='#Font_Force_Hinting'>Force_Hinting</a>
instructs the <a href='#Font_Manager'>Font_Manager</a> to always hint <a href='undocumented#Glyph'>Glyphs</a>.
<a href='#Font_Force_Hinting'>Force_Hinting</a> has no effect if Hinting is set to <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kNone'>kNone</a> or
<a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kSlight'>kSlight</a>.

<a href='#Font_Force_Hinting'>Force_Hinting</a> only affects platforms that use FreeType as the <a href='#Font_Manager'>Font_Manager</a>.

<a name='Embedded_Bitmaps'></a>

<a href='#Font_Embedded_Bitmaps'>Embedded_Bitmaps</a> allows selecting custom sized <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='undocumented#Glyph'>Glyphs</a>.
<a href='#Font_Embedded_Bitmaps'>Embedded_Bitmaps</a> when set chooses an embedded <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='undocumented#Glyph'>glyph</a> over an outline contained
in a <a href='SkFont_Reference#Font'>font</a> if the platform supports this option.

FreeType selects the <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='undocumented#Glyph'>glyph</a> if available when <a href='#Font_Embedded_Bitmaps'>Embedded_Bitmaps</a> is set, and selects
the outline <a href='undocumented#Glyph'>glyph</a> if <a href='#Font_Embedded_Bitmaps'>Embedded_Bitmaps</a> is clear.
Windows may select the <a href='SkBitmap_Reference#Bitmap'>bitmap</a> <a href='undocumented#Glyph'>glyph</a> but is not required to do so.
<a href='#OS_X'>OS_X</a> and iOS do not support this option.

<a name='SkFont'></a>

---

<a href='SkFont_Reference#SkFont'>SkFont</a> controls options applied when drawing and measuring <a href='undocumented#Text'>text</a>.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkFont_Reference#SkFont'>SkFont</a> {

    enum class <a href='#SkFont_Edging'>Edging</a> {
        kAlias,
        kAntiAlias,
        kSubpixelAntiAlias,
    };

    <a href='#SkFont_empty_constructor'>SkFont()</a>;
    <a href='#SkFont_SkTypeface_SkScalar'>SkFont</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkTypeface'>SkTypeface</a>> <a href='undocumented#Typeface'>typeface</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#Size'>size</a>);
    <a href='#SkFont_SkTypeface_SkScalar_SkScalar_SkScalar'>SkFont</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkTypeface'>SkTypeface</a>> <a href='undocumented#Typeface'>typeface</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#SkScalar'>SkScalar</a> scaleX, <a href='undocumented#SkScalar'>SkScalar</a> skewX);
    bool <a href='#SkFont_equal1_operator'>operator==</a>(const <a href='SkFont_Reference#SkFont'>SkFont</a>& <a href='SkFont_Reference#Font'>font</a>) const;
    bool <a href='#SkFont_isForceAutoHinting'>isForceAutoHinting</a>() const;
    bool <a href='#SkFont_isEmbeddedBitmaps'>isEmbeddedBitmaps</a>() const;
    bool <a href='#SkFont_isSubpixel'>isSubpixel</a>() const;
    bool <a href='#SkFont_isLinearMetrics'>isLinearMetrics</a>() const;
    bool <a href='#SkFont_isEmbolden'>isEmbolden</a>() const;
    void <a href='#SkFont_setForceAutoHinting'>setForceAutoHinting</a>(bool forceAutoHinting);
    void <a href='#SkFont_setEmbeddedBitmaps'>setEmbeddedBitmaps</a>(bool embeddedBitmaps);
    void <a href='#SkFont_setSubpixel'>setSubpixel</a>(bool subpixel);
    void <a href='#SkFont_setLinearMetrics'>setLinearMetrics</a>(bool linearMetrics);
    void <a href='#SkFont_setEmbolden'>setEmbolden</a>(bool embolden);
    <a href='#SkFont_Edging'>Edging</a> <a href='#SkFont_getEdging'>getEdging</a>() const;
    void <a href='#SkFont_setEdging'>setEdging</a>(<a href='#SkFont_Edging'>Edging</a> edging);
    void <a href='#SkFont_setHinting'>setHinting</a>(<a href='undocumented#SkFontHinting'>SkFontHinting</a> hintingLevel);
    <a href='undocumented#SkFontHinting'>SkFontHinting</a> <a href='#SkFont_getHinting'>getHinting</a>() const;
    <a href='SkFont_Reference#SkFont'>SkFont</a> <a href='#SkFont_makeWithSize'>makeWithSize</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#Size'>size</a>) const;
    <a href='undocumented#SkTypeface'>SkTypeface</a>* <a href='#SkFont_getTypeface'>getTypeface</a>() const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkFont_getSize'>getSize</a>() const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkFont_getScaleX'>getScaleX</a>() const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkFont_getSkewX'>getSkewX</a>() const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkTypeface'>SkTypeface</a>> <a href='#SkFont_refTypeface'>refTypeface</a>() const;
    void <a href='#SkFont_setTypeface'>setTypeface</a>(<a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkTypeface'>SkTypeface</a>> tf);
    void <a href='#SkFont_setSize'>setSize</a>(<a href='undocumented#SkScalar'>SkScalar</a> textSize);
    void <a href='#SkFont_setScaleX'>setScaleX</a>(<a href='undocumented#SkScalar'>SkScalar</a> scaleX);
    void <a href='#SkFont_setSkewX'>setSkewX</a>(<a href='undocumented#SkScalar'>SkScalar</a> skewX);
    int <a href='#SkFont_textToGlyphs'>textToGlyphs</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength, <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> encoding,
                     <a href='undocumented#SkGlyphID'>SkGlyphID</a> <a href='undocumented#Glyph'>glyphs</a>[], int maxGlyphCount) const;
    uint16_t <a href='#SkFont_unicharToGlyph'>unicharToGlyph</a>(<a href='undocumented#SkUnichar'>SkUnichar</a> uni) const;
    int <a href='#SkFont_countText'>countText</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength, <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> encoding) const;
    bool <a href='#SkFont_containsText'>containsText</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength, <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> encoding) const;
    size_t <a href='#SkFont_breakText'>breakText</a>(const void* <a href='undocumented#Text'>text</a>, size_t length, <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> encoding, <a href='undocumented#SkScalar'>SkScalar</a> maxWidth,
                     <a href='undocumented#SkScalar'>SkScalar</a>* measuredWidth = nullptr) const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkFont_measureText'>measureText</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength, <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> encoding,
                         <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds = nullptr) const;
    bool <a href='#SkFont_getPath'>getPath</a>(uint16_t glyphID, <a href='SkPath_Reference#SkPath'>SkPath</a>* <a href='SkPath_Reference#Path'>path</a>) const;
    void <a href='#SkFont_getPaths'>getPaths</a>(const uint16_t glyphIDs[], int count,
                  void (*glyphPathProc)(const <a href='SkPath_Reference#SkPath'>SkPath</a>* pathOrNull, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& mx, void* ctx),
                  void* ctx) const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkFont_getMetrics'>getMetrics</a>(<a href='undocumented#SkFontMetrics'>SkFontMetrics</a>* metrics) const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkFont_getSpacing'>getSpacing</a>() const;
};

</pre>

<a name='SkFont_Edging'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum class <a href='#SkFont_Edging'>Edging</a> {
        <a href='#SkFont_Edging_kAlias'>kAlias</a>,
        <a href='#SkFont_Edging_kAntiAlias'>kAntiAlias</a>,
        <a href='#SkFont_Edging_kSubpixelAntiAlias'>kSubpixelAntiAlias</a>,
    };

</pre>

Whether edge pixels draw opaque or with partial transparency.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkFont_Edging_kAlias'><code>SkFont::Edging::kAlias</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
no transparent pixels on glyph edges</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkFont_Edging_kAntiAlias'><code>SkFont::Edging::kAntiAlias</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
may have transparent pixels on glyph edges</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkFont_Edging_kSubpixelAntiAlias'><code>SkFont::Edging::kSubpixelAntiAlias</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
glyph positioned in pixel using transparency</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_empty_constructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkFont_empty_constructor'>SkFont()</a>
</pre>

Constructs <a href='SkFont_Reference#SkFont'>SkFont</a> with default values.

### Return Value

default initialized <a href='SkFont_Reference#SkFont'>SkFont</a>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_SkTypeface_SkScalar'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkFont_Reference#SkFont'>SkFont</a>(<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkTypeface'>SkTypeface</a>&gt; <a href='undocumented#Typeface'>typeface</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#Size'>size</a>)
</pre>

Constructs <a href='SkFont_Reference#SkFont'>SkFont</a> with default values with <a href='undocumented#SkTypeface'>SkTypeface</a> and <a href='#SkFont_SkTypeface_SkScalar_size'>size</a> in <a href='SkPoint_Reference#Point'>points</a>.

### Parameters

<table>  <tr>    <td><a name='SkFont_SkTypeface_SkScalar_typeface'><code><strong>typeface</strong></code></a></td>
    <td><a href='SkFont_Reference#Font'>font</a> and style used to draw and measure <a href='undocumented#Text'>text</a></td>
  </tr>
  <tr>    <td><a name='SkFont_SkTypeface_SkScalar_size'><code><strong>size</strong></code></a></td>
    <td>typographic height of <a href='undocumented#Text'>text</a></td>
  </tr>
</table>

### Return Value

initialized <a href='SkFont_Reference#SkFont'>SkFont</a>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_SkTypeface_SkScalar_SkScalar_SkScalar'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkFont_Reference#SkFont'>SkFont</a>(<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkTypeface'>SkTypeface</a>&gt; <a href='undocumented#Typeface'>typeface</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#SkScalar'>SkScalar</a> scaleX, <a href='undocumented#SkScalar'>SkScalar</a> skewX)
</pre>

Constructs <a href='SkFont_Reference#SkFont'>SkFont</a> with default values with <a href='undocumented#SkTypeface'>SkTypeface</a> and <a href='#SkFont_SkTypeface_SkScalar_SkScalar_SkScalar_size'>size</a> in <a href='SkPoint_Reference#Point'>points</a>,
horizontal scale, and horizontal skew. Horizontal scale emulates condensed
and expanded fonts. Horizontal skew emulates oblique fonts.

### Parameters

<table>  <tr>    <td><a name='SkFont_SkTypeface_SkScalar_SkScalar_SkScalar_typeface'><code><strong>typeface</strong></code></a></td>
    <td><a href='SkFont_Reference#Font'>font</a> and style used to draw and measure <a href='undocumented#Text'>text</a></td>
  </tr>
  <tr>    <td><a name='SkFont_SkTypeface_SkScalar_SkScalar_SkScalar_size'><code><strong>size</strong></code></a></td>
    <td>typographic height of <a href='undocumented#Text'>text</a></td>
  </tr>
  <tr>    <td><a name='SkFont_SkTypeface_SkScalar_SkScalar_SkScalar_scaleX'><code><strong>scaleX</strong></code></a></td>
    <td><a href='undocumented#Text'>text</a> horizontal scale</td>
  </tr>
  <tr>    <td><a name='SkFont_SkTypeface_SkScalar_SkScalar_SkScalar_skewX'><code><strong>skewX</strong></code></a></td>
    <td>additional shear on x-axis relative to y-axis</td>
  </tr>
</table>

### Return Value

initialized <a href='SkFont_Reference#SkFont'>SkFont</a>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_equal1_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator==(const <a href='SkFont_Reference#SkFont'>SkFont</a>& <a href='SkFont_Reference#Font'>font</a>)const
</pre>

Compares <a href='SkFont_Reference#SkFont'>SkFont</a> and <a href='#SkFont_equal1_operator_font'>font</a>, and returns true if they are equivalent.
May return false if <a href='undocumented#SkTypeface'>SkTypeface</a> has identical contents but different pointers.

### Parameters

<table>  <tr>    <td><a name='SkFont_equal1_operator_font'><code><strong>font</strong></code></a></td>
    <td><a href='#SkFont_equal1_operator_font'>font</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href='SkFont_Reference#SkFont'>SkFont</a> pair are equivalent

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_isForceAutoHinting'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkFont_isForceAutoHinting'>isForceAutoHinting</a>()const
</pre>

If true, instructs the <a href='SkFont_Reference#Font'>font</a> manager to always hint <a href='undocumented#Glyph'>glyphs</a>.
Returned value is only meaningful if platform uses FreeType as the <a href='SkFont_Reference#Font'>font</a> manager.

### Return Value

true if all <a href='undocumented#Glyph'>glyphs</a> are hinted

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_isEmbeddedBitmaps'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkFont_isEmbeddedBitmaps'>isEmbeddedBitmaps</a>()const
</pre>

Returns true if <a href='SkFont_Reference#Font'>font</a> engine may return <a href='undocumented#Glyph'>glyphs</a> from <a href='SkFont_Reference#Font'>font</a> <a href='SkBitmap_Reference#Bitmap'>bitmaps</a> instead of from outlines.

### Return Value

true if <a href='undocumented#Glyph'>glyphs</a> may be <a href='SkFont_Reference#Font'>font</a> <a href='SkBitmap_Reference#Bitmap'>bitmaps</a>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_isSubpixel'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkFont_isSubpixel'>isSubpixel</a>()const
</pre>

Returns true if <a href='undocumented#Glyph'>glyphs</a> at different <a href='SkFont_Reference#Subpixel'>sub-pixel</a> positions may differ on <a href='undocumented#Pixel'>pixel</a> edge coverage.

### Return Value

true if <a href='undocumented#Glyph'>glyph</a> positioned in <a href='undocumented#Pixel'>pixel</a> using transparency

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_isLinearMetrics'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkFont_isLinearMetrics'>isLinearMetrics</a>()const
</pre>

Returns true if <a href='undocumented#Text'>text</a> is converted to <a href='SkPath_Reference#SkPath'>SkPath</a> before drawing and measuring.

### Return Value

true <a href='undocumented#Glyph'>glyph</a> hints are never applied

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_isEmbolden'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkFont_isEmbolden'>isEmbolden</a>()const
</pre>

Returns true if bold is approximated by increasing the stroke width when creating <a href='undocumented#Glyph'>glyph</a>
<a href='SkBitmap_Reference#Bitmap'>bitmaps</a> from outlines.

### Return Value

bold is approximated through stroke width

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_setForceAutoHinting'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkFont_setForceAutoHinting'>setForceAutoHinting</a>(bool forceAutoHinting)
</pre>

Sets whether to always hint <a href='undocumented#Glyph'>glyphs</a>.
If <a href='#SkFont_setForceAutoHinting_forceAutoHinting'>forceAutoHinting</a> is set, instructs the  <a href='undocumented#Font_Manager'>font manager</a> to always hint <a href='undocumented#Glyph'>glyphs</a>.

Only affects platforms that use FreeType as the  <a href='undocumented#Font_Manager'>font manager</a>.

### Parameters

<table>  <tr>    <td><a name='SkFont_setForceAutoHinting_forceAutoHinting'><code><strong>forceAutoHinting</strong></code></a></td>
    <td>setting to always hint <a href='undocumented#Glyph'>glyphs</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_setEmbeddedBitmaps'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkFont_setEmbeddedBitmaps'>setEmbeddedBitmaps</a>(bool embeddedBitmaps)
</pre>

Requests, but does not require, to use <a href='SkBitmap_Reference#Bitmap'>bitmaps</a> in fonts instead of outlines.

### Parameters

<table>  <tr>    <td><a name='SkFont_setEmbeddedBitmaps_embeddedBitmaps'><code><strong>embeddedBitmaps</strong></code></a></td>
    <td>setting to use <a href='SkBitmap_Reference#Bitmap'>bitmaps</a> in fonts</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_setSubpixel'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkFont_setSubpixel'>setSubpixel</a>(bool subpixel)
</pre>

Requests, but does not require, that <a href='undocumented#Glyph'>glyphs</a> respect <a href='SkFont_Reference#Subpixel'>sub-pixel</a> positioning.

### Parameters

<table>  <tr>    <td><a name='SkFont_setSubpixel_subpixel'><code><strong>subpixel</strong></code></a></td>
    <td>setting for <a href='SkFont_Reference#Subpixel'>sub-pixel</a> positioning</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_setLinearMetrics'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkFont_setLinearMetrics'>setLinearMetrics</a>(bool linearMetrics)
</pre>

Requests, but does not require, that <a href='undocumented#Glyph'>glyphs</a> are converted to <a href='SkPath_Reference#SkPath'>SkPath</a>
before drawing and measuring.

### Parameters

<table>  <tr>    <td><a name='SkFont_setLinearMetrics_linearMetrics'><code><strong>linearMetrics</strong></code></a></td>
    <td>setting for converting <a href='undocumented#Glyph'>glyphs</a> to <a href='SkPath_Reference#Path'>paths</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_setEmbolden'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkFont_setEmbolden'>setEmbolden</a>(bool embolden)
</pre>

Increases stroke width when creating <a href='undocumented#Glyph'>glyph</a> <a href='SkBitmap_Reference#Bitmap'>bitmaps</a> to approximate a bold <a href='undocumented#Typeface'>typeface</a>.

### Parameters

<table>  <tr>    <td><a name='SkFont_setEmbolden_embolden'><code><strong>embolden</strong></code></a></td>
    <td>setting for bold approximation</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_getEdging'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkFont_Edging'>Edging</a> <a href='#SkFont_getEdging'>getEdging</a>()const
</pre>

Whether edge pixels draw opaque or with partial transparency.

### Return Value

one of: <a href='#SkFont_Edging'>Edging</a>::<a href='#SkFont_Edging_kAlias'>kAlias</a>, <a href='#SkFont_Edging'>Edging</a>::<a href='#SkFont_Edging_kAntiAlias'>kAntiAlias</a>, <a href='#SkFont_Edging'>Edging</a>::<a href='#SkFont_Edging_kSubpixelAntiAlias'>kSubpixelAntiAlias</a>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_setEdging'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkFont_setEdging'>setEdging</a>(<a href='#SkFont_Edging'>Edging</a> edging)
</pre>

Requests, but does not require, that edge pixels draw opaque or with
partial transparency.

### Parameters

<table>  <tr>    <td><a name='SkFont_setEdging_edging'><code><strong>edging</strong></code></a></td>
    <td>one of: <a href='#SkFont_Edging'>Edging</a>::<a href='#SkFont_Edging_kAlias'>kAlias</a>, <a href='#SkFont_Edging'>Edging</a>::<a href='#SkFont_Edging_kAntiAlias'>kAntiAlias</a>, <a href='#SkFont_Edging'>Edging</a>::<a href='#SkFont_Edging_kSubpixelAntiAlias'>kSubpixelAntiAlias</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_setHinting'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkFont_setHinting'>setHinting</a>(<a href='undocumented#SkFontHinting'>SkFontHinting</a> hintingLevel)
</pre>

Sets level of <a href='undocumented#Glyph'>glyph</a> outline adjustment.
Does not check for valid values of <a href='#SkFont_setHinting_hintingLevel'>hintingLevel</a>.

### Parameters

<table>  <tr>    <td><a name='SkFont_setHinting_hintingLevel'><code><strong>hintingLevel</strong></code></a></td>
    <td>one of: <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kNone'>kNone</a>, <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kSlight'>kSlight</a>,</td>
  </tr>
</table>

<a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kNormal'>kNormal</a>, <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kFull'>kFull</a>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_getHinting'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkFontHinting'>SkFontHinting</a> <a href='#SkFont_getHinting'>getHinting</a>()const
</pre>

Returns level of <a href='undocumented#Glyph'>glyph</a> outline adjustment.

### Return Value

one of: <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kNone'>kNone</a>, <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kSlight'>kSlight</a>, <a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kNormal'>kNormal</a>,

<a href='undocumented#SkFontHinting'>SkFontHinting</a>::<a href='#SkFontHinting_kFull'>kFull</a>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_makeWithSize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkFont_Reference#SkFont'>SkFont</a> <a href='#SkFont_makeWithSize'>makeWithSize</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#Size'>size</a>)const
</pre>

Returns a <a href='SkFont_Reference#Font'>font</a> with the same attributes of this <a href='SkFont_Reference#Font'>font</a>, but with the specified <a href='#SkFont_makeWithSize_size'>size</a>.
Returns nullptr if <a href='#SkFont_makeWithSize_size'>size</a> is less than zero, infinite, or NaN.

### Parameters

<table>  <tr>    <td><a name='SkFont_makeWithSize_size'><code><strong>size</strong></code></a></td>
    <td>typographic height of <a href='undocumented#Text'>text</a></td>
  </tr>
</table>

### Return Value

initialized <a href='SkFont_Reference#SkFont'>SkFont</a>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_getTypeface'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkTypeface'>SkTypeface</a>* <a href='#SkFont_getTypeface'>getTypeface</a>()const
</pre>

Returns <a href='undocumented#SkTypeface'>SkTypeface</a> if set, or nullptr.
Does not alter <a href='undocumented#SkTypeface'>SkTypeface</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a>.

### Return Value

<a href='undocumented#SkTypeface'>SkTypeface</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_getSize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkFont_getSize'>getSize</a>()const
</pre>

Returns <a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a> in <a href='SkPoint_Reference#Point'>points</a>.

### Return Value

typographic height of <a href='undocumented#Text'>text</a>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_getScaleX'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkFont_getScaleX'>getScaleX</a>()const
</pre>

Returns <a href='undocumented#Text'>text</a> scale on x-axis.
Default value is 1.

### Return Value

<a href='undocumented#Text'>text</a> horizontal scale

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_getSkewX'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkFont_getSkewX'>getSkewX</a>()const
</pre>

Returns <a href='undocumented#Text'>text</a> skew on x-axis.
Default value is zero.

### Return Value

additional shear on x-axis relative to y-axis

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_refTypeface'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkTypeface'>SkTypeface</a>&gt; <a href='#SkFont_refTypeface'>refTypeface</a>()const
</pre>

Increases <a href='undocumented#SkTypeface'>SkTypeface</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> by one.

### Return Value

<a href='undocumented#SkTypeface'>SkTypeface</a> if previously set, nullptr otherwise

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_setTypeface'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkFont_setTypeface'>setTypeface</a>(<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkTypeface'>SkTypeface</a>&gt; tf)
</pre>

Sets <a href='undocumented#SkTypeface'>SkTypeface</a> to <a href='undocumented#Typeface'>typeface</a>, decreasing <a href='undocumented#SkRefCnt'>SkRefCnt</a> of the previous <a href='undocumented#SkTypeface'>SkTypeface</a>.
Pass nullptr to clear <a href='undocumented#SkTypeface'>SkTypeface</a> and use the default <a href='undocumented#Typeface'>typeface</a>. Increments
<a href='#SkFont_setTypeface_tf'>tf</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> by one.

### Parameters

<table>  <tr>    <td><a name='SkFont_setTypeface_tf'><code><strong>tf</strong></code></a></td>
    <td><a href='SkFont_Reference#Font'>font</a> and style used to draw <a href='undocumented#Text'>text</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_setSize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkFont_setSize'>setSize</a>(<a href='undocumented#SkScalar'>SkScalar</a> textSize)
</pre>

Sets <a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a> in <a href='SkPoint_Reference#Point'>points</a>.
Has no effect if <a href='#SkFont_setSize_textSize'>textSize</a> is not greater than or equal to zero.

### Parameters

<table>  <tr>    <td><a name='SkFont_setSize_textSize'><code><strong>textSize</strong></code></a></td>
    <td>typographic height of <a href='undocumented#Text'>text</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_setScaleX'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkFont_setScaleX'>setScaleX</a>(<a href='undocumented#SkScalar'>SkScalar</a> scaleX)
</pre>

Sets  <a href='undocumented#Text'>text scale</a> on x-axis.
Default value is 1.

### Parameters

<table>  <tr>    <td><a name='SkFont_setScaleX_scaleX'><code><strong>scaleX</strong></code></a></td>
    <td><a href='undocumented#Text'>text</a> horizontal scale</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_setSkewX'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkFont_setSkewX'>setSkewX</a>(<a href='undocumented#SkScalar'>SkScalar</a> skewX)
</pre>

Sets  <a href='undocumented#Text'>text skew</a> on x-axis.
Default value is zero.

### Parameters

<table>  <tr>    <td><a name='SkFont_setSkewX_skewX'><code><strong>skewX</strong></code></a></td>
    <td>additional shear on x-axis relative to y-axis</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_textToGlyphs'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkFont_textToGlyphs'>textToGlyphs</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength, <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> encoding, <a href='undocumented#SkGlyphID'>SkGlyphID</a> <a href='undocumented#Glyph'>glyphs</a>[],
                 int maxGlyphCount)const
</pre>

Converts <a href='#SkFont_textToGlyphs_text'>text</a> into <a href='undocumented#Glyph'>glyph</a> indices.
Returns the number of <a href='undocumented#Glyph'>glyph</a> indices represented by <a href='#SkFont_textToGlyphs_text'>text</a>.
<a href='undocumented#SkTextEncoding'>SkTextEncoding</a> specifies how <a href='#SkFont_textToGlyphs_text'>text</a> represents characters or <a href='#SkFont_textToGlyphs_glyphs'>glyphs</a>.
<a href='#SkFont_textToGlyphs_glyphs'>glyphs</a> may be nullptr, to compute the <a href='undocumented#Glyph'>glyph</a> count.

Does not check <a href='#SkFont_textToGlyphs_text'>text</a> for valid character codes or valid <a href='undocumented#Glyph'>glyph</a> indices.

If <a href='#SkFont_textToGlyphs_byteLength'>byteLength</a> equals zero, returns zero.
If <a href='#SkFont_textToGlyphs_byteLength'>byteLength</a> includes a partial character, the partial character is ignored.

If <a href='#SkFont_textToGlyphs_encoding'>encoding</a> is <a href='undocumented#SkTextEncoding::kUTF8'>SkTextEncoding::kUTF8</a> and <a href='#SkFont_textToGlyphs_text'>text</a> contains an invalid UTF-8 sequence,
zero is returned.

If <a href='#SkFont_textToGlyphs_maxGlyphCount'>maxGlyphCount</a> is not sufficient to store all the <a href='#SkFont_textToGlyphs_glyphs'>glyphs</a>, no <a href='#SkFont_textToGlyphs_glyphs'>glyphs</a> are copied.
The total <a href='undocumented#Glyph'>glyph</a> count is returned for subsequent buffer reallocation.

### Parameters

<table>  <tr>    <td><a name='SkFont_textToGlyphs_text'><code><strong>text</strong></code></a></td>
    <td>character storage encoded with <a href='undocumented#SkTextEncoding'>SkTextEncoding</a></td>
  </tr>
  <tr>    <td><a name='SkFont_textToGlyphs_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>length of character storage in bytes</td>
  </tr>
  <tr>    <td><a name='SkFont_textToGlyphs_encoding'><code><strong>encoding</strong></code></a></td>
    <td>one of: <a href='undocumented#SkTextEncoding::kUTF8'>SkTextEncoding::kUTF8</a>, <a href='undocumented#SkTextEncoding::kUTF16'>SkTextEncoding::kUTF16</a>,</td>
  </tr>
</table>

<a href='undocumented#SkTextEncoding::kUTF32'>SkTextEncoding::kUTF32</a>, <a href='undocumented#SkTextEncoding::kGlyphID'>SkTextEncoding::kGlyphID</a>

### Parameters

<table>  <tr>    <td><a name='SkFont_textToGlyphs_glyphs'><code><strong>glyphs</strong></code></a></td>
    <td>storage for <a href='undocumented#Glyph'>glyph</a> indices; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkFont_textToGlyphs_maxGlyphCount'><code><strong>maxGlyphCount</strong></code></a></td>
    <td>storage capacity</td>
  </tr>
</table>

### Return Value

number of <a href='#SkFont_textToGlyphs_glyphs'>glyphs</a> represented by <a href='#SkFont_textToGlyphs_text'>text</a> of length <a href='#SkFont_textToGlyphs_byteLength'>byteLength</a>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_unicharToGlyph'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint16_t <a href='#SkFont_unicharToGlyph'>unicharToGlyph</a>(<a href='undocumented#SkUnichar'>SkUnichar</a> uni)const
</pre>

Returns <a href='undocumented#Glyph'>glyph</a> index for Unicode character.

### Parameters

<table>  <tr>    <td><a name='SkFont_unicharToGlyph_uni'><code><strong>uni</strong></code></a></td>
    <td>Unicode character</td>
  </tr>
</table>

### Return Value

<a href='undocumented#Glyph'>glyph</a> index

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_countText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkFont_countText'>countText</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength, <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> encoding)const
</pre>

Returns number of <a href='undocumented#Glyph'>glyphs</a> represented by <a href='#SkFont_countText_text'>text</a>.

### Parameters

<table>  <tr>    <td><a name='SkFont_countText_text'><code><strong>text</strong></code></a></td>
    <td>character storage encoded with <a href='undocumented#SkTextEncoding'>SkTextEncoding</a></td>
  </tr>
  <tr>    <td><a name='SkFont_countText_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>length of character storage in bytes</td>
  </tr>
  <tr>    <td><a name='SkFont_countText_encoding'><code><strong>encoding</strong></code></a></td>
    <td>one of: <a href='undocumented#SkTextEncoding::kUTF8'>SkTextEncoding::kUTF8</a>, <a href='undocumented#SkTextEncoding::kUTF16'>SkTextEncoding::kUTF16</a>,</td>
  </tr>
</table>

<a href='undocumented#SkTextEncoding::kUTF32'>SkTextEncoding::kUTF32</a>, <a href='undocumented#SkTextEncoding::kGlyphID'>SkTextEncoding::kGlyphID</a>

### Return Value

number of <a href='undocumented#Glyph'>glyphs</a> represented by <a href='#SkFont_countText_text'>text</a> of length <a href='#SkFont_countText_byteLength'>byteLength</a>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_containsText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkFont_containsText'>containsText</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength, <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> encoding)const
</pre>

Returns true if all <a href='#SkFont_containsText_text'>text</a> corresponds to a non-zero <a href='undocumented#Glyph'>glyph</a> index.
Returns false if any characters in <a href='#SkFont_containsText_text'>text</a> are not supported in
<a href='undocumented#SkTypeface'>SkTypeface</a>.

If <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> is <a href='undocumented#SkTextEncoding::kGlyphID'>SkTextEncoding::kGlyphID</a>,
returns true if all <a href='undocumented#Glyph'>glyph</a> indices in <a href='#SkFont_containsText_text'>text</a> are non-zero;
does not check to see if <a href='#SkFont_containsText_text'>text</a> contains valid <a href='undocumented#Glyph'>glyph</a> indices for <a href='undocumented#SkTypeface'>SkTypeface</a>.

Returns true if <a href='#SkFont_containsText_byteLength'>byteLength</a> is zero.

### Parameters

<table>  <tr>    <td><a name='SkFont_containsText_text'><code><strong>text</strong></code></a></td>
    <td>array of characters or <a href='undocumented#Glyph'>glyphs</a></td>
  </tr>
  <tr>    <td><a name='SkFont_containsText_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>number of bytes in <a href='#SkFont_containsText_text'>text</a> array</td>
  </tr>
  <tr>    <td><a name='SkFont_containsText_encoding'><code><strong>encoding</strong></code></a></td>
    <td><a href='undocumented#Text_Encoding'>text encoding</a></td>
  </tr>
</table>

### Return Value

true if all <a href='#SkFont_containsText_text'>text</a> corresponds to a non-zero <a href='undocumented#Glyph'>glyph</a> index

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_breakText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkFont_breakText'>breakText</a>(const void* <a href='undocumented#Text'>text</a>, size_t length, <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> encoding, <a href='undocumented#SkScalar'>SkScalar</a> maxWidth,
                 <a href='undocumented#SkScalar'>SkScalar</a>* measuredWidth = nullptr)const
</pre>

Returns the bytes of <a href='#SkFont_breakText_text'>text</a> that fit within <a href='#SkFont_breakText_maxWidth'>maxWidth</a>.
The <a href='#SkFont_breakText_text'>text</a> fragment fits if its advance width is less than or equal to <a href='#SkFont_breakText_maxWidth'>maxWidth</a>.
Measures only while the advance is less than or equal to <a href='#SkFont_breakText_maxWidth'>maxWidth</a>.
Returns the advance or the <a href='#SkFont_breakText_text'>text</a> fragment in <a href='#SkFont_breakText_measuredWidth'>measuredWidth</a> if it not nullptr.
Uses <a href='#SkFont_breakText_encoding'>encoding</a> to decode <a href='#SkFont_breakText_text'>text</a>, <a href='undocumented#SkTypeface'>SkTypeface</a> to get the  <a href='undocumented#Font_Metrics'>font metrics</a>,
and <a href='#SkFont_breakText_text'>text</a> <a href='undocumented#Size'>size</a> to scale the metrics.
Does not scale the advance or bounds by fake bold.

### Parameters

<table>  <tr>    <td><a name='SkFont_breakText_text'><code><strong>text</strong></code></a></td>
    <td>character codes or <a href='undocumented#Glyph'>glyph</a> indices to be measured</td>
  </tr>
  <tr>    <td><a name='SkFont_breakText_length'><code><strong>length</strong></code></a></td>
    <td>number of bytes of <a href='#SkFont_breakText_text'>text</a> to measure</td>
  </tr>
  <tr>    <td><a name='SkFont_breakText_encoding'><code><strong>encoding</strong></code></a></td>
    <td><a href='undocumented#Text_Encoding'>text encoding</a></td>
  </tr>
  <tr>    <td><a name='SkFont_breakText_maxWidth'><code><strong>maxWidth</strong></code></a></td>
    <td>advance limit; <a href='#SkFont_breakText_text'>text</a> is measured while advance is less than <a href='#SkFont_breakText_maxWidth'>maxWidth</a></td>
  </tr>
  <tr>    <td><a name='SkFont_breakText_measuredWidth'><code><strong>measuredWidth</strong></code></a></td>
    <td>returns the width of the <a href='#SkFont_breakText_text'>text</a> less than or equal to <a href='#SkFont_breakText_maxWidth'>maxWidth</a></td>
  </tr>
</table>

### Return Value

bytes of <a href='#SkFont_breakText_text'>text</a> that fit, always less than or equal to <a href='#SkFont_breakText_length'>length</a>

### Example

<div><fiddle-embed name="3cad18678254526be66ef162eecd1d23"><div><a href='undocumented#Line'>Line</a> under "Breakfast" shows desired width, shorter than available characters.
<a href='undocumented#Line'>Line</a> under "Bre" shows measured width after breaking <a href='#SkFont_breakText_text'>text</a>.
</div></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_measureText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkFont_measureText'>measureText</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength, <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> encoding,
                     <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds = nullptr)const
</pre>

Returns the advance width of <a href='#SkFont_measureText_text'>text</a>.
The advance is the normal distance to move before drawing additional <a href='#SkFont_measureText_text'>text</a>.
Returns the bounding box of <a href='#SkFont_measureText_text'>text</a> if <a href='#SkFont_measureText_bounds'>bounds</a> is not nullptr.

### Parameters

<table>  <tr>    <td><a name='SkFont_measureText_text'><code><strong>text</strong></code></a></td>
    <td>character storage encoded with <a href='undocumented#SkTextEncoding'>SkTextEncoding</a></td>
  </tr>
  <tr>    <td><a name='SkFont_measureText_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>length of character storage in bytes</td>
  </tr>
  <tr>    <td><a name='SkFont_measureText_encoding'><code><strong>encoding</strong></code></a></td>
    <td>one of: <a href='undocumented#SkTextEncoding::kUTF8'>SkTextEncoding::kUTF8</a>, <a href='undocumented#SkTextEncoding::kUTF16'>SkTextEncoding::kUTF16</a>,</td>
  </tr>
</table>

<a href='undocumented#SkTextEncoding::kUTF32'>SkTextEncoding::kUTF32</a>, <a href='undocumented#SkTextEncoding::kGlyphID'>SkTextEncoding::kGlyphID</a>

### Parameters

<table>  <tr>    <td><a name='SkFont_measureText_bounds'><code><strong>bounds</strong></code></a></td>
    <td>returns bounding box relative to (0, 0) if not nullptr</td>
  </tr>
</table>

### Return Value

number of <a href='undocumented#Glyph'>glyphs</a> represented by <a href='#SkFont_measureText_text'>text</a> of length <a href='#SkFont_measureText_byteLength'>byteLength</a>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_getPath'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkFont_getPath'>getPath</a>(uint16_t glyphID, <a href='SkPath_Reference#SkPath'>SkPath</a>* <a href='SkPath_Reference#Path'>path</a>)const
</pre>

Returns <a href='#SkFont_getPath_path'>path</a> corresponding to <a href='undocumented#Glyph'>glyph</a> outline.
If <a href='undocumented#Glyph'>glyph</a> has an outline, copies outline to <a href='#SkFont_getPath_path'>path</a> and returns true.
<a href='#SkFont_getPath_path'>path</a> returned may be empty.
If <a href='undocumented#Glyph'>glyph</a> is described by a <a href='SkBitmap_Reference#Bitmap'>bitmap</a>, returns false and ignores <a href='#SkFont_getPath_path'>path</a> parameter.

### Parameters

<table>  <tr>    <td><a name='SkFont_getPath_glyphID'><code><strong>glyphID</strong></code></a></td>
    <td>index of <a href='undocumented#Glyph'>glyph</a></td>
  </tr>
  <tr>    <td><a name='SkFont_getPath_path'><code><strong>path</strong></code></a></td>
    <td>pointer to existing <a href='SkPath_Reference#SkPath'>SkPath</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkFont_getPath_glyphID'>glyphID</a> is described by <a href='#SkFont_getPath_path'>path</a>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_getPaths'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkFont_getPaths'>getPaths</a>(const uint16_t glyphIDs[], int count, void (*glyphPathProc) (const <a href='SkPath_Reference#SkPath'>SkPath</a>* pathOrNull,
              const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& mx, void* ctx) , void* ctx)const
</pre>

Returns <a href='SkPath_Reference#Path'>path</a> corresponding to <a href='undocumented#Glyph'>glyph</a> array.

### Parameters

<table>  <tr>    <td><a name='SkFont_getPaths_glyphIDs'><code><strong>glyphIDs</strong></code></a></td>
    <td>array of <a href='undocumented#Glyph'>glyph</a> indices</td>
  </tr>
  <tr>    <td><a name='SkFont_getPaths_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='undocumented#Glyph'>glyphs</a></td>
  </tr>
  <tr>    <td><a name='SkFont_getPaths_glyphPathProc'><code><strong>glyphPathProc</strong></code></a></td>
    <td>function returning one <a href='undocumented#Glyph'>glyph</a> description as <a href='SkPath_Reference#Path'>path</a></td>
  </tr>
  <tr>    <td><a name='SkFont_getPaths_ctx'><code><strong>ctx</strong></code></a></td>
    <td>function context</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_getMetrics'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkFont_getMetrics'>getMetrics</a>(<a href='undocumented#SkFontMetrics'>SkFontMetrics</a>* metrics)const
</pre>

Returns <a href='undocumented#SkFontMetrics'>SkFontMetrics</a> associated with <a href='undocumented#SkTypeface'>SkTypeface</a>.
The return value is the recommended spacing between <a href='undocumented#Line'>lines</a>: the sum of <a href='#SkFont_getMetrics_metrics'>metrics</a>
descent, ascent, and leading.
If <a href='#SkFont_getMetrics_metrics'>metrics</a> is not nullptr, <a href='undocumented#SkFontMetrics'>SkFontMetrics</a> is copied to <a href='#SkFont_getMetrics_metrics'>metrics</a>.
Results are scaled by <a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a> but does not take into account
dimensions required by  <a href='undocumented#Text'>text scale</a>,  <a href='undocumented#Text'>text skew</a>, fake bold,
style stroke, and <a href='undocumented#SkPathEffect'>SkPathEffect</a>.

### Parameters

<table>  <tr>    <td><a name='SkFont_getMetrics_metrics'><code><strong>metrics</strong></code></a></td>
    <td>storage for <a href='undocumented#SkFontMetrics'>SkFontMetrics</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

recommended spacing between <a href='undocumented#Line'>lines</a>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

<a name='SkFont_getSpacing'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkFont_getSpacing'>getSpacing</a>()const
</pre>

Returns the recommended spacing between <a href='undocumented#Line'>lines</a>: the sum of metrics
descent, ascent, and leading.
Result is scaled by <a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a> but does not take into account
dimensions required by stroking and <a href='undocumented#SkPathEffect'>SkPathEffect</a>.
Returns the same result as <a href='#SkFont_getMetrics'>getMetrics</a>().

### Return Value

recommended spacing between <a href='undocumented#Line'>lines</a>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

