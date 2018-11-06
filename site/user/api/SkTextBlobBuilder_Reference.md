SkTextBlobBuilder Reference
===


<a name='SkTextBlobBuilder'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>SkTextBlobBuilder</a> {
public:
    <a href='#SkTextBlobBuilder_empty_constructor'>SkTextBlobBuilder()</a>;
    ~<a href='#SkTextBlobBuilder_empty_constructor'>SkTextBlobBuilder()</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>> <a href='#SkTextBlobBuilder_make'>make()</a>;

    struct <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a> {
        <a href='undocumented#SkGlyphID'>SkGlyphID</a>* <a href='undocumented#Glyph'>glyphs</a>;
        <a href='undocumented#SkScalar'>SkScalar</a>* pos;
        char* utf8text;
        uint32_t* clusters;
    };

    const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRun'>allocRun</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='undocumented#Font'>font</a>, int count, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y,
                              const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds = nullptr);
    const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRunPosH'>allocRunPosH</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='undocumented#Font'>font</a>, int count, <a href='undocumented#SkScalar'>SkScalar</a> y,
                                  const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds = nullptr);
    const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRunPos'>allocRunPos</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='undocumented#Font'>font</a>, int count,
                                 const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds = nullptr);
    const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRun'>allocRun</a>(const <a href='undocumented#SkFont'>SkFont</a>& <a href='undocumented#Font'>font</a>, int count, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y,
                              const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds = nullptr);
    const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRunPosH'>allocRunPosH</a>(const <a href='undocumented#SkFont'>SkFont</a>& <a href='undocumented#Font'>font</a>, int count, <a href='undocumented#SkScalar'>SkScalar</a> y,
                                  const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds = nullptr);
    const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRunPos'>allocRunPos</a>(const <a href='undocumented#SkFont'>SkFont</a>& <a href='undocumented#Font'>font</a>, int count,
                                 const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds = nullptr);
};
</pre>

Helper class for constructing <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>.

<a name='SkTextBlobBuilder_RunBuffer'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    struct <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a> {
        <a href='undocumented#SkGlyphID'>SkGlyphID</a>* <a href='#SkTextBlobBuilder_RunBuffer_glyphs'>glyphs</a>;
        <a href='undocumented#SkScalar'>SkScalar</a>* <a href='#SkTextBlobBuilder_RunBuffer_pos'>pos</a>;
        char* <a href='#SkTextBlobBuilder_RunBuffer_utf8text'>utf8text</a>;
        uint32_t* <a href='#SkTextBlobBuilder_RunBuffer_clusters'>clusters</a>;
    };
</pre>

<a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a> supplies storage for <a href='undocumented#Glyph'>Glyphs</a> and positions within a run.

A run is a sequence of <a href='undocumented#Glyph'>Glyphs</a> sharing <a href='#Paint_Font_Metrics'>Paint_Font_Metrics</a> and positioning.
Each run may position its <a href='undocumented#Glyph'>Glyphs</a> in one of three ways:
by specifying where the first <a href='undocumented#Glyph'>Glyph</a> is drawn, and allowing <a href='#Paint_Font_Metrics'>Paint_Font_Metrics</a> to
determine the advance to subsequent <a href='undocumented#Glyph'>Glyphs</a>; by specifying a baseline, and
the position on that baseline for each <a href='undocumented#Glyph'>Glyph</a> in run; or by providing <a href='SkPoint_Reference#Point'>Point</a>
array, one per <a href='undocumented#Glyph'>Glyph</a>.<table style='border-collapse: collapse; width: 62.5em'>

  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Member</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkGlyphID*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkTextBlobBuilder_RunBuffer_glyphs'><code>glyphs</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#SkTextBlobBuilder_RunBuffer_glyphs'>glyphs</a> <a href='SkPoint_Reference#Point'>points</a> to memory for one or more <a href='undocumented#Glyph'>Glyphs</a>. <a href='#SkTextBlobBuilder_RunBuffer_glyphs'>glyphs</a> memory must be
written to by the caller.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkTextBlobBuilder_RunBuffer_pos'><code>pos</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#SkTextBlobBuilder_RunBuffer_pos'>pos</a> <a href='SkPoint_Reference#Point'>points</a> to memory for <a href='undocumented#Glyph'>Glyph</a> positions. Depending on how <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>
is allocated, <a href='#SkTextBlobBuilder_RunBuffer_pos'>pos</a> may <a href='SkPoint_Reference#Point'>point</a> to zero bytes per <a href='undocumented#Glyph'>Glyph</a>, one <a href='undocumented#Scalar'>Scalar</a> per <a href='undocumented#Glyph'>Glyph</a>,
or one <a href='SkPoint_Reference#Point'>Point</a> per <a href='undocumented#Glyph'>Glyph</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>char*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkTextBlobBuilder_RunBuffer_utf8text'><code>utf8text</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Reserved for future use. <a href='#SkTextBlobBuilder_RunBuffer_utf8text'>utf8text</a> should not be read or written.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>uint32_t*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkTextBlobBuilder_RunBuffer_clusters'><code>clusters</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Reserved for future use. <a href='#SkTextBlobBuilder_RunBuffer_clusters'>clusters</a> should not be read or written.
</td>
  </tr>
</table>

### See Also

<a href='#SkTextBlobBuilder_allocRun'>allocRun</a> <a href='#SkTextBlobBuilder_allocRunPos'>allocRunPos</a> <a href='#SkTextBlobBuilder_allocRunPosH'>allocRunPosH</a>

<a name='SkTextBlobBuilder_empty_constructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkTextBlobBuilder_empty_constructor'>SkTextBlobBuilder()</a>
</pre>

Constructs empty <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>SkTextBlobBuilder</a>. By default, <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>SkTextBlobBuilder</a> has no runs.

### Return Value

empty <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>SkTextBlobBuilder</a>

### Example

<div><fiddle-embed name="d9dbabfe24aad92ee3c8144513e90d81">

#### Example Output

~~~~
blob equals nullptr
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkTextBlobBuilder_make'>make</a> <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>::<a href='#SkTextBlob_MakeFromText'>MakeFromText</a>

<a name='SkTextBlobBuilder_destructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
~<a href='#SkTextBlobBuilder_empty_constructor'>SkTextBlobBuilder()</a>
</pre>

Deletes <a href='undocumented#Data'>data</a> allocated internally by <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>SkTextBlobBuilder</a>.

### See Also

<a href='#SkTextBlobBuilder_empty_constructor'>SkTextBlobBuilder()</a>

<a name='SkTextBlobBuilder_make'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>&gt; <a href='#SkTextBlobBuilder_make'>make()</a>
</pre>

Returns <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a> built from runs of <a href='undocumented#Glyph'>glyphs</a> added by builder. Returned
<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a> is immutable; it may be copied, but its contents may not be altered.
Returns nullptr if no runs of <a href='undocumented#Glyph'>glyphs</a> were added by builder.

Resets <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>SkTextBlobBuilder</a> to its initial empty state, allowing it to be
reused to build a new set of runs.

### Return Value

<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a> or nullptr

### Example

<div><fiddle-embed name="595f6ae785a623ac26059f0573fda995">

#### Example Output

~~~~
blob equals nullptr
blob does not equal nullptr
blob equals nullptr
~~~~

</fiddle-embed></div>

### See Also

<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>::<a href='#SkTextBlob_MakeFromText'>MakeFromText</a>

<a name='SkTextBlobBuilder_allocRun'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRun'>allocRun</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='undocumented#Font'>font</a>, int count, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y,
                          const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds = nullptr)
</pre>

Returns run with storage for <a href='undocumented#Glyph'>Glyphs</a>. Caller must write <a href='#SkTextBlobBuilder_allocRun_count'>count</a> <a href='undocumented#Glyph'>Glyphs</a> to
<a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_glyphs'>glyphs</a> before next call to <a href='#Text_Blob_Builder'>Text_Blob_Builder</a>.

<a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_utf8text'>utf8text</a>, and <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_clusters'>clusters</a> should be ignored.

<a href='undocumented#Glyph'>Glyphs</a> share <a href='#Paint_Font_Metrics'>Paint_Font_Metrics</a> in <a href='#SkTextBlobBuilder_allocRun_font'>font</a>, including: <a href='undocumented#Typeface'>Typeface</a>, <a href='#Paint_Text_Size'>Paint_Text_Size</a>, <a href='#Paint_Text_Scale_X'>Paint_Text_Scale_X</a>,
<a href='#Paint_Text_Skew_X'>Paint_Text_Skew_X</a>, <a href='#Paint_Hinting'>Paint_Hinting</a>, <a href='#Paint_Anti_Alias'>Anti_Alias</a>, <a href='#Paint_Fake_Bold'>Paint_Fake_Bold</a>,
<a href='#Paint_Font_Embedded_Bitmaps'>Font_Embedded_Bitmaps</a>, <a href='#Paint_Full_Hinting_Spacing'>Full_Hinting_Spacing</a>, <a href='#Paint_LCD_Text'>LCD_Text</a>, <a href='#Paint_Linear_Text'>Linear_Text</a>,
and <a href='#Paint_Subpixel_Text'>Subpixel_Text</a>
.

<a href='undocumented#Glyph'>Glyphs</a> are positioned on a baseline at (<a href='#SkTextBlobBuilder_allocRun_x'>x</a>, <a href='#SkTextBlobBuilder_allocRun_y'>y</a>), using <a href='#SkTextBlobBuilder_allocRun_font'>font</a> <a href='#Paint_Font_Metrics'>Paint_Font_Metrics</a> to
determine their relative placement.

<a href='#SkTextBlobBuilder_allocRun_bounds'>bounds</a> defines an optional bounding box, used to suppress drawing when <a href='#Text_Blob'>Text_Blob</a>
<a href='#SkTextBlobBuilder_allocRun_bounds'>bounds</a> does not intersect <a href='SkSurface_Reference#Surface'>Surface</a> <a href='#SkTextBlobBuilder_allocRun_bounds'>bounds</a>. If <a href='#SkTextBlobBuilder_allocRun_bounds'>bounds</a> is nullptr, <a href='#Text_Blob'>Text_Blob</a> <a href='#SkTextBlobBuilder_allocRun_bounds'>bounds</a>
is computed from (<a href='#SkTextBlobBuilder_allocRun_x'>x</a>, <a href='#SkTextBlobBuilder_allocRun_y'>y</a>) and <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_glyphs'>glyphs</a> <a href='#Paint_Font_Metrics'>Paint_Font_Metrics</a>.

### Parameters

<table>  <tr>    <td><a name='SkTextBlobBuilder_allocRun_font'><code><strong>font</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> used for this run</td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRun_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='undocumented#Glyph'>glyphs</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRun_x'><code><strong>x</strong></code></a></td>
    <td>horizontal offset within the blob</td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRun_y'><code><strong>y</strong></code></a></td>
    <td>vertical offset within the blob</td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRun_bounds'><code><strong>bounds</strong></code></a></td>
    <td>optional run bounding box</td>
  </tr>
</table>

### Return Value

writable <a href='undocumented#Glyph'>glyph</a> buffer

### Example

<div><fiddle-embed name="aedc51083fadd99451fe5180b8ff4c7d"></fiddle-embed></div>

### See Also

<a href='#SkTextBlobBuilder_allocRunPosH'>allocRunPosH</a> <a href='#SkTextBlobBuilder_allocRunPos'>allocRunPos</a>

<a name='SkTextBlobBuilder_allocRun_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRun'>allocRun</a>(const <a href='undocumented#SkFont'>SkFont</a>& <a href='undocumented#Font'>font</a>, int count, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y,
                          const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds = nullptr)
</pre>

Returns run with storage for <a href='undocumented#Glyph'>Glyphs</a>. Caller must write count <a href='undocumented#Glyph'>Glyphs</a> to
<a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_glyphs'>glyphs</a> before next call to <a href='#Text_Blob_Builder'>Text_Blob_Builder</a>.

<a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_utf8text'>utf8text</a>, and <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_clusters'>clusters</a> should be ignored.

<a href='undocumented#Glyph'>Glyphs</a> share <a href='#Paint_Font_Metrics'>Paint_Font_Metrics</a> in <a href='undocumented#Font'>font</a>, including: <a href='undocumented#Typeface'>Typeface</a>, <a href='#Paint_Text_Size'>Paint_Text_Size</a>, <a href='#Paint_Text_Scale_X'>Paint_Text_Scale_X</a>,
<a href='#Paint_Text_Skew_X'>Paint_Text_Skew_X</a>, <a href='#Paint_Hinting'>Paint_Hinting</a>, <a href='#Paint_Anti_Alias'>Anti_Alias</a>, <a href='#Paint_Fake_Bold'>Paint_Fake_Bold</a>,
<a href='#Paint_Font_Embedded_Bitmaps'>Font_Embedded_Bitmaps</a>, <a href='#Paint_Full_Hinting_Spacing'>Full_Hinting_Spacing</a>, <a href='#Paint_LCD_Text'>LCD_Text</a>, <a href='#Paint_Linear_Text'>Linear_Text</a>,
and <a href='#Paint_Subpixel_Text'>Subpixel_Text</a>
.

<a href='undocumented#Glyph'>Glyphs</a> are positioned on a baseline at (x, y), using <a href='undocumented#Font'>font</a> <a href='#Paint_Font_Metrics'>Paint_Font_Metrics</a> to
determine their relative placement.

bounds defines an optional bounding box, used to suppress drawing when <a href='#Text_Blob'>Text_Blob</a>
bounds does not intersect <a href='SkSurface_Reference#Surface'>Surface</a> bounds. If bounds is nullptr, <a href='#Text_Blob'>Text_Blob</a> bounds
is computed from (x, y) and <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_glyphs'>glyphs</a> <a href='#Paint_Font_Metrics'>Paint_Font_Metrics</a>.

### Parameters

<table>  <tr>    <td><a name='SkTextBlobBuilder_allocRun_2_font'><code><strong>font</strong></code></a></td>
    <td><a href='undocumented#Font'>Font</a> used for this run</td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRun_2_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='undocumented#Glyph'>glyphs</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRun_2_x'><code><strong>x</strong></code></a></td>
    <td>horizontal offset within the blob</td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRun_2_y'><code><strong>y</strong></code></a></td>
    <td>vertical offset within the blob</td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRun_2_bounds'><code><strong>bounds</strong></code></a></td>
    <td>optional run bounding box</td>
  </tr>
</table>

### Return Value

writable <a href='undocumented#Glyph'>glyph</a> buffer

### Example

<div><fiddle-embed name="f0e584aec20eaee7a5bfed62aa885eee"></fiddle-embed></div>

### See Also

<a href='#SkTextBlobBuilder_allocRunPosH'>allocRunPosH</a> <a href='#SkTextBlobBuilder_allocRunPos'>allocRunPos</a>

<a name='SkTextBlobBuilder_allocRunPosH'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRunPosH'>allocRunPosH</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='undocumented#Font'>font</a>, int count, <a href='undocumented#SkScalar'>SkScalar</a> y,
                              const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds = nullptr)
</pre>

Returns run with storage for <a href='undocumented#Glyph'>Glyphs</a> and positions along baseline. Caller must
write <a href='#SkTextBlobBuilder_allocRunPosH_count'>count</a> <a href='undocumented#Glyph'>Glyphs</a> to <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_glyphs'>glyphs</a>, and <a href='#SkTextBlobBuilder_allocRunPosH_count'>count</a> <a href='undocumented#Scalar'>Scalars</a> to <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_pos'>pos</a>;
before next call to <a href='#Text_Blob_Builder'>Text_Blob_Builder</a>.

<a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_utf8text'>utf8text</a>, and <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_clusters'>clusters</a> should be ignored.

<a href='undocumented#Glyph'>Glyphs</a> share <a href='#Paint_Font_Metrics'>Paint_Font_Metrics</a> in <a href='#SkTextBlobBuilder_allocRunPosH_font'>font</a>, including: <a href='undocumented#Typeface'>Typeface</a>, <a href='#Paint_Text_Size'>Paint_Text_Size</a>, <a href='#Paint_Text_Scale_X'>Paint_Text_Scale_X</a>,
<a href='#Paint_Text_Skew_X'>Paint_Text_Skew_X</a>, <a href='#Paint_Hinting'>Paint_Hinting</a>, <a href='#Paint_Anti_Alias'>Anti_Alias</a>, <a href='#Paint_Fake_Bold'>Paint_Fake_Bold</a>,
<a href='#Paint_Font_Embedded_Bitmaps'>Font_Embedded_Bitmaps</a>, <a href='#Paint_Full_Hinting_Spacing'>Full_Hinting_Spacing</a>, <a href='#Paint_LCD_Text'>LCD_Text</a>, <a href='#Paint_Linear_Text'>Linear_Text</a>,
and <a href='#Paint_Subpixel_Text'>Subpixel_Text</a>
.

<a href='undocumented#Glyph'>Glyphs</a> are positioned on a baseline at <a href='#SkTextBlobBuilder_allocRunPosH_y'>y</a>, using x-axis positions written by
caller to <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_pos'>pos</a>.

<a href='#SkTextBlobBuilder_allocRunPosH_bounds'>bounds</a> defines an optional bounding box, used to suppress drawing when <a href='#Text_Blob'>Text_Blob</a>
<a href='#SkTextBlobBuilder_allocRunPosH_bounds'>bounds</a> does not intersect <a href='SkSurface_Reference#Surface'>Surface</a> <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>bounds</a>. If <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>bounds</a> is nullptr, <a href='#Text_Blob'>Text_Blob</a> <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>bounds</a>
is computed from <a href='#SkTextBlobBuilder_allocRunPosH_y'>y</a>, <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_pos'>pos</a>, and <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_glyphs'>glyphs</a> <a href='#Paint_Font_Metrics'>Paint_Font_Metrics</a>.

### Parameters

<table>  <tr>    <td><a name='SkTextBlobBuilder_allocRunPosH_font'><code><strong>font</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> used for this run</td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRunPosH_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='undocumented#Glyph'>Glyphs</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRunPosH_y'><code><strong>y</strong></code></a></td>
    <td>vertical offset within the blob</td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRunPosH_bounds'><code><strong>bounds</strong></code></a></td>
    <td>optional run bounding box</td>
  </tr>
</table>

### Return Value

writable <a href='undocumented#Glyph'>glyph</a> buffer and x-axis position buffer

### Example

<div><fiddle-embed name="735c352787b24e490740dedd035987d2"></fiddle-embed></div>

### See Also

<a href='#SkTextBlobBuilder_allocRunPos'>allocRunPos</a> <a href='#SkTextBlobBuilder_allocRun'>allocRun</a>

<a name='SkTextBlobBuilder_allocRunPosH_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRunPosH'>allocRunPosH</a>(const <a href='undocumented#SkFont'>SkFont</a>& <a href='undocumented#Font'>font</a>, int count, <a href='undocumented#SkScalar'>SkScalar</a> y,
                              const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds = nullptr)
</pre>

Returns run with storage for <a href='undocumented#Glyph'>Glyphs</a> and positions along baseline. Caller must
write count <a href='undocumented#Glyph'>Glyphs</a> to <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_glyphs'>glyphs</a>, and count <a href='undocumented#Scalar'>Scalars</a> to <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_pos'>pos</a>;
before next call to <a href='#Text_Blob_Builder'>Text_Blob_Builder</a>.

<a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_utf8text'>utf8text</a>, and <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_clusters'>clusters</a> should be ignored.

<a href='undocumented#Glyph'>Glyphs</a> share <a href='#Paint_Font_Metrics'>Paint_Font_Metrics</a> in <a href='undocumented#Font'>font</a>, including: <a href='undocumented#Typeface'>Typeface</a>, <a href='#Paint_Text_Size'>Paint_Text_Size</a>, <a href='#Paint_Text_Scale_X'>Paint_Text_Scale_X</a>,
<a href='#Paint_Text_Skew_X'>Paint_Text_Skew_X</a>, <a href='#Paint_Hinting'>Paint_Hinting</a>, <a href='#Paint_Anti_Alias'>Anti_Alias</a>, <a href='#Paint_Fake_Bold'>Paint_Fake_Bold</a>,
<a href='#Paint_Font_Embedded_Bitmaps'>Font_Embedded_Bitmaps</a>, <a href='#Paint_Full_Hinting_Spacing'>Full_Hinting_Spacing</a>, <a href='#Paint_LCD_Text'>LCD_Text</a>, <a href='#Paint_Linear_Text'>Linear_Text</a>,
and <a href='#Paint_Subpixel_Text'>Subpixel_Text</a>
.

<a href='undocumented#Glyph'>Glyphs</a> are positioned on a baseline at y, using x-axis positions written by
caller to <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_pos'>pos</a>.

bounds defines an optional bounding box, used to suppress drawing when <a href='#Text_Blob'>Text_Blob</a>
bounds does not intersect <a href='SkSurface_Reference#Surface'>Surface</a> bounds. If bounds is nullptr, <a href='#Text_Blob'>Text_Blob</a> bounds
is computed from y, <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_pos'>pos</a>, and <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_glyphs'>glyphs</a> <a href='#Paint_Font_Metrics'>Paint_Font_Metrics</a>.

### Parameters

<table>  <tr>    <td><a name='SkTextBlobBuilder_allocRunPosH_2_font'><code><strong>font</strong></code></a></td>
    <td><a href='undocumented#Font'>Font</a> used for this run</td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRunPosH_2_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='undocumented#Glyph'>Glyphs</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRunPosH_2_y'><code><strong>y</strong></code></a></td>
    <td>vertical offset within the blob</td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRunPosH_2_bounds'><code><strong>bounds</strong></code></a></td>
    <td>optional run bounding box</td>
  </tr>
</table>

### Return Value

writable <a href='undocumented#Glyph'>glyph</a> buffer and x-axis position buffer

### Example

<div><fiddle-embed name="c77ac50f506106fdfef94d20bc1a6934"></fiddle-embed></div>

### See Also

<a href='#SkTextBlobBuilder_allocRunPos'>allocRunPos</a> <a href='#SkTextBlobBuilder_allocRun'>allocRun</a>

<a name='SkTextBlobBuilder_allocRunPos'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRunPos'>allocRunPos</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='undocumented#Font'>font</a>, int count, const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds = nullptr)
</pre>

Returns run with storage for <a href='undocumented#Glyph'>Glyphs</a> and <a href='SkPoint_Reference#Point'>Point</a> positions. Caller must
write <a href='#SkTextBlobBuilder_allocRunPos_count'>count</a> <a href='undocumented#Glyph'>Glyphs</a> to <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_glyphs'>glyphs</a>, and <a href='#SkTextBlobBuilder_allocRunPos_count'>count</a> <a href='SkPoint_Reference#Point'>Points</a> to <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_pos'>pos</a>;
before next call to <a href='#Text_Blob_Builder'>Text_Blob_Builder</a>.

<a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_utf8text'>utf8text</a>, and <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_clusters'>clusters</a> should be ignored.

<a href='undocumented#Glyph'>Glyphs</a> share <a href='#Paint_Font_Metrics'>Paint_Font_Metrics</a> in <a href='#SkTextBlobBuilder_allocRunPos_font'>font</a>, including: <a href='undocumented#Typeface'>Typeface</a>, <a href='#Paint_Text_Size'>Paint_Text_Size</a>, <a href='#Paint_Text_Scale_X'>Paint_Text_Scale_X</a>,
<a href='#Paint_Text_Skew_X'>Paint_Text_Skew_X</a>, <a href='#Paint_Hinting'>Paint_Hinting</a>, <a href='#Paint_Anti_Alias'>Anti_Alias</a>, <a href='#Paint_Fake_Bold'>Paint_Fake_Bold</a>,
<a href='#Paint_Font_Embedded_Bitmaps'>Font_Embedded_Bitmaps</a>, <a href='#Paint_Full_Hinting_Spacing'>Full_Hinting_Spacing</a>, <a href='#Paint_LCD_Text'>LCD_Text</a>, <a href='#Paint_Linear_Text'>Linear_Text</a>,
and <a href='#Paint_Subpixel_Text'>Subpixel_Text</a>
.

<a href='undocumented#Glyph'>Glyphs</a> are positioned using <a href='SkPoint_Reference#Point'>Points</a> written by caller to <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_pos'>pos</a>, using
two <a href='undocumented#Scalar'>Scalar</a> values for each <a href='SkPoint_Reference#Point'>Point</a>.

<a href='#SkTextBlobBuilder_allocRunPos_bounds'>bounds</a> defines an optional bounding box, used to suppress drawing when <a href='#Text_Blob'>Text_Blob</a>
<a href='#SkTextBlobBuilder_allocRunPos_bounds'>bounds</a> does not intersect <a href='SkSurface_Reference#Surface'>Surface</a> <a href='#SkTextBlobBuilder_allocRunPos_bounds'>bounds</a>. If <a href='#SkTextBlobBuilder_allocRunPos_bounds'>bounds</a> is nullptr, <a href='#Text_Blob'>Text_Blob</a> <a href='#SkTextBlobBuilder_allocRunPos_bounds'>bounds</a>
is computed from <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_pos'>pos</a>, and <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_glyphs'>glyphs</a> <a href='#Paint_Font_Metrics'>Paint_Font_Metrics</a>.

### Parameters

<table>  <tr>    <td><a name='SkTextBlobBuilder_allocRunPos_font'><code><strong>font</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> used for this run</td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRunPos_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='undocumented#Glyph'>Glyphs</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRunPos_bounds'><code><strong>bounds</strong></code></a></td>
    <td>optional run bounding box</td>
  </tr>
</table>

### Return Value

writable <a href='undocumented#Glyph'>glyph</a> buffer and <a href='SkPoint_Reference#Point'>Point</a> buffer

### Example

<div><fiddle-embed name="2a11d3c287f785f3808dcbce08ccb435"></fiddle-embed></div>

### See Also

<a href='#SkTextBlobBuilder_allocRunPosH'>allocRunPosH</a> <a href='#SkTextBlobBuilder_allocRun'>allocRun</a>

<a name='SkTextBlobBuilder_allocRunPos_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRunPos'>allocRunPos</a>(const <a href='undocumented#SkFont'>SkFont</a>& <a href='undocumented#Font'>font</a>, int count, const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds = nullptr)
</pre>

Returns run with storage for <a href='undocumented#Glyph'>Glyphs</a> and <a href='SkPoint_Reference#Point'>Point</a> positions. Caller must
write count <a href='undocumented#Glyph'>Glyphs</a> to <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_glyphs'>glyphs</a>, and count <a href='SkPoint_Reference#Point'>Points</a> to <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_pos'>pos</a>;
before next call to <a href='#Text_Blob_Builder'>Text_Blob_Builder</a>.

<a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_utf8text'>utf8text</a>, and <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_clusters'>clusters</a> should be ignored.

<a href='undocumented#Glyph'>Glyphs</a> share <a href='#Paint_Font_Metrics'>Paint_Font_Metrics</a> in <a href='undocumented#Font'>font</a>, including: <a href='undocumented#Typeface'>Typeface</a>, <a href='#Paint_Text_Size'>Paint_Text_Size</a>, <a href='#Paint_Text_Scale_X'>Paint_Text_Scale_X</a>,
<a href='#Paint_Text_Skew_X'>Paint_Text_Skew_X</a>, <a href='#Paint_Hinting'>Paint_Hinting</a>, <a href='#Paint_Anti_Alias'>Anti_Alias</a>, <a href='#Paint_Fake_Bold'>Paint_Fake_Bold</a>,
<a href='#Paint_Font_Embedded_Bitmaps'>Font_Embedded_Bitmaps</a>, <a href='#Paint_Full_Hinting_Spacing'>Full_Hinting_Spacing</a>, <a href='#Paint_LCD_Text'>LCD_Text</a>, <a href='#Paint_Linear_Text'>Linear_Text</a>,
and <a href='#Paint_Subpixel_Text'>Subpixel_Text</a>
.

<a href='undocumented#Glyph'>Glyphs</a> are positioned using <a href='SkPoint_Reference#Point'>Points</a> written by caller to <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_pos'>pos</a>, using
two <a href='undocumented#Scalar'>Scalar</a> values for each <a href='SkPoint_Reference#Point'>Point</a>.

bounds defines an optional bounding box, used to suppress drawing when <a href='#Text_Blob'>Text_Blob</a>
bounds does not intersect <a href='SkSurface_Reference#Surface'>Surface</a> bounds. If bounds is nullptr, <a href='#Text_Blob'>Text_Blob</a> bounds
is computed from <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_pos'>pos</a>, and <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_glyphs'>glyphs</a> <a href='#Paint_Font_Metrics'>Paint_Font_Metrics</a>.

### Parameters

<table>  <tr>    <td><a name='SkTextBlobBuilder_allocRunPos_2_font'><code><strong>font</strong></code></a></td>
    <td><a href='undocumented#Font'>Font</a> used for this run</td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRunPos_2_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='undocumented#Glyph'>Glyphs</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRunPos_2_bounds'><code><strong>bounds</strong></code></a></td>
    <td>optional run bounding box</td>
  </tr>
</table>

### Return Value

writable <a href='undocumented#Glyph'>glyph</a> buffer and <a href='SkPoint_Reference#Point'>Point</a> buffer

### Example

<div><fiddle-embed name="da4fcb4a972b500996be9aff6c6c40e1"></fiddle-embed></div>

### See Also

<a href='#SkTextBlobBuilder_allocRunPosH'>allocRunPosH</a> <a href='#SkTextBlobBuilder_allocRun'>allocRun</a>

