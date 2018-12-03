SkFont Reference
===


<a name='Advance'></a>

<a name='Engine'></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkFont_Reference#SkFont'>SkFont</a> {
    // <i><a href='SkFont_Reference#SkFont'>SkFont</a> interface</i>
};
</pre>

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
    bool operator==(const <a href='SkFont_Reference#SkFont'>SkFont</a>& <a href='SkFont_Reference#Font'>font</a>) const;
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
    size_t breakText(const void* <a href='undocumented#Text'>text</a>, size_t length, <a href='undocumented#SkTextEncoding'>SkTextEncoding</a>, <a href='undocumented#SkScalar'>SkScalar</a> maxWidth,
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
    <td><a href='SkPaint_Reference#SkPaint'>SkPaint</a> to compare</td>
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

Returns true if <a href='undocumented#Glyph'>glyphs</a> at different sub-pixel positions may differ on <a href='undocumented#Pixel'>pixel</a> edge coverage.

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

Requests, but does not require, that <a href='undocumented#Glyph'>glyphs</a> respect sub-pixel positioning.

### Parameters

<table>  <tr>    <td><a name='SkFont_setSubpixel_subpixel'><code><strong>subpixel</strong></code></a></td>
    <td>setting for sub-pixel positioning</td>
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

If <a href='#SkFont_textToGlyphs_encoding'>encoding</a> is <a href='undocumented#kUTF8_SkTextEncoding'>kUTF8_SkTextEncoding</a> and <a href='#SkFont_textToGlyphs_text'>text</a> contains an invalid UTF-8 sequence,
zero is returned.

If <a href='#SkFont_textToGlyphs_maxGlyphCount'>maxGlyphCount</a> is not sufficient to store all the <a href='#SkFont_textToGlyphs_glyphs'>glyphs</a>, no <a href='#SkFont_textToGlyphs_glyphs'>glyphs</a> are copied.
The total <a href='undocumented#Glyph'>glyph</a> count is returned for subsequent buffer reallocation.

### Parameters

<table>  <tr>    <td><a name='SkFont_textToGlyphs_text'><code><strong>text</strong></code></a></td>
    <td>character storage encoded with <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a></td>
  </tr>
  <tr>    <td><a name='SkFont_textToGlyphs_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>length of character storage in bytes</td>
  </tr>
  <tr>    <td><a name='SkFont_textToGlyphs_encoding'><code><strong>encoding</strong></code></a></td>
    <td>one of: <a href='undocumented#kUTF8_SkTextEncoding'>kUTF8_SkTextEncoding</a>, <a href='undocumented#kUTF16_SkTextEncoding'>kUTF16_SkTextEncoding</a>,</td>
  </tr>
</table>

<a href='undocumented#kUTF32_SkTextEncoding'>kUTF32_SkTextEncoding</a>, <a href='undocumented#kGlyphID_SkTextEncoding'>kGlyphID_SkTextEncoding</a>

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
    <td>character storage encoded with <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a></td>
  </tr>
  <tr>    <td><a name='SkFont_countText_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>length of character storage in bytes</td>
  </tr>
  <tr>    <td><a name='SkFont_countText_encoding'><code><strong>encoding</strong></code></a></td>
    <td>one of: <a href='undocumented#kUTF8_SkTextEncoding'>kUTF8_SkTextEncoding</a>, <a href='undocumented#kUTF16_SkTextEncoding'>kUTF16_SkTextEncoding</a>,</td>
  </tr>
</table>

<a href='undocumented#kUTF32_SkTextEncoding'>kUTF32_SkTextEncoding</a>, <a href='undocumented#kGlyphID_SkTextEncoding'>kGlyphID_SkTextEncoding</a>

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

If <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> is <a href='undocumented#kGlyphID_SkTextEncoding'>kGlyphID_SkTextEncoding</a>,
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
    <td><a href='#SkFont_containsText_text'>text</a> <a href='#SkFont_containsText_encoding'>encoding</a></td>
  </tr>
</table>

### Return Value

true if all <a href='#SkFont_containsText_text'>text</a> corresponds to a non-zero <a href='undocumented#Glyph'>glyph</a> index

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

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
    <td>character storage encoded with <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a></td>
  </tr>
  <tr>    <td><a name='SkFont_measureText_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>length of character storage in bytes</td>
  </tr>
  <tr>    <td><a name='SkFont_measureText_encoding'><code><strong>encoding</strong></code></a></td>
    <td>one of: <a href='undocumented#kUTF8_SkTextEncoding'>kUTF8_SkTextEncoding</a>, <a href='undocumented#kUTF16_SkTextEncoding'>kUTF16_SkTextEncoding</a>,</td>
  </tr>
</table>

<a href='undocumented#kUTF32_SkTextEncoding'>kUTF32_SkTextEncoding</a>, <a href='undocumented#kGlyphID_SkTextEncoding'>kGlyphID_SkTextEncoding</a>

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

