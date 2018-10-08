SkTextBlobBuilder Reference
===

<a name='SkTextBlobBuilder'></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='#SkTextBlobBuilder'>SkTextBlobBuilder</a> {
public:
    <a href='#SkTextBlobBuilder_empty_constructor'>SkTextBlobBuilder()</a>;
    <a href='#SkTextBlobBuilder_destructor'>~SkTextBlobBuilder()</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>> <a href='#SkTextBlobBuilder_make'>make</a>();

    struct <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a> {
        <a href='undocumented#SkGlyphID'>SkGlyphID</a>* glyphs;
        <a href='undocumented#SkScalar'>SkScalar</a>* pos;
        char* <a href='#SkTextBlobBuilder_RunBuffer_utf8text'>utf8text</a>;
        uint32_t* clusters;
    };

    const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRun'>allocRun</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& font, int count, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y,
                              const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds = nullptr);
    const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRunPosH'>allocRunPosH</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& font, int count, <a href='undocumented#SkScalar'>SkScalar</a> y,
                                  const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds = nullptr);
    const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRunPos'>allocRunPos</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& font, int count,
                                 const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds = nullptr);
};
</pre>

Helper class for constructing <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>
<a name='SkTextBlobBuilder_RunBuffer'></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    struct <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a> {
        <a href='undocumented#SkGlyphID'>SkGlyphID</a>* glyphs;
        <a href='undocumented#SkScalar'>SkScalar</a>* pos;
        char* <a href='#SkTextBlobBuilder_RunBuffer_utf8text'>utf8text</a>;
        uint32_t* clusters;
    };
</pre>

<a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a> supplies storage for <a href='undocumented#Glyph'>Glyphs</a> and positions within a run<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Member</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkGlyphID*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkTextBlobBuilder_RunBuffer_glyphs'><code>glyphs</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
glyphs points to memory for one or more <a href='undocumented#Glyph'>Glyphs</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkTextBlobBuilder_RunBuffer_pos'><code>pos</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
pos points to memory for <a href='undocumented#Glyph'>Glyph</a> positions</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>char*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkTextBlobBuilder_RunBuffer_utf8text'><code>utf8text</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Reserved for future use</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>uint32_t*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkTextBlobBuilder_RunBuffer_clusters'><code>clusters</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Reserved for future use</td>
  </tr>
</table>

### See Also

<a href='#SkTextBlobBuilder_allocRun'>allocRun</a> <a href='#SkTextBlobBuilder_allocRunPos'>allocRunPos</a> <a href='#SkTextBlobBuilder_allocRunPosH'>allocRunPosH</a>

<a name='SkTextBlobBuilder_empty_constructor'></a>
## SkTextBlobBuilder

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkTextBlobBuilder'>SkTextBlobBuilder</a>(
</pre>

Constructs empty <a href='#Text_Blob_Builder'>Text Blob Builder</a>

### Return Value

empty <a href='#Text_Blob_Builder'>Text Blob Builder</a>

### Example

<div><fiddle-embed name="d9dbabfe24aad92ee3c8144513e90d81">

#### Example Output

~~~~
blob equals nullptr
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkTextBlobBuilder_make'>make</a> <a href='SkTextBlob_Reference#SkTextBlob_MakeFromText'>SkTextBlob::MakeFromText</a>

---

<a name='SkTextBlobBuilder_destructor'></a>
## ~SkTextBlobBuilder

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkTextBlobBuilder_destructor'>~SkTextBlobBuilder</a>(
</pre>

Deletes data allocated internally by <a href='#Text_Blob_Builder'>Text Blob Builder</a>

### See Also

<a href='#SkTextBlobBuilder_empty_constructor'>SkTextBlobBuilder()</a>

---

<a name='SkTextBlobBuilder_make'></a>
## make

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Returns <a href='SkTextBlob_Reference#Text_Blob'>Text Blob</a> built from runs of <a href='undocumented#Glyph'>Glyphs</a> added by builder

### Return Value

<a href='SkTextBlob_Reference#Text_Blob'>Text Blob</a> or nullptr

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

<a href='SkTextBlob_Reference#SkTextBlob_MakeFromText'>SkTextBlob::MakeFromText</a>

---

<a name='SkTextBlobBuilder_allocRun'></a>
## allocRun

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>
</pre>

Returns run with storage for <a href='undocumented#Glyph'>Glyphs</a> <a href='undocumented#Typeface'>Typeface</a>

### Parameters

<table>  <tr>    <td><a name='SkTextBlobBuilder_allocRun_font'><code><strong>font</strong></code></a></td>
    <td><a href='SkPaint_Reference#Paint'>Paint</a> used for this run</td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRun_count'><code><strong>count</strong></code></a></td>
    <td>number of glyphs</td>
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

writable glyph buffer

### Example

<div><fiddle-embed name="aedc51083fadd99451fe5180b8ff4c7d"></fiddle-embed></div>

### See Also

<a href='#SkTextBlobBuilder_allocRunPosH'>allocRunPosH</a> <a href='#SkTextBlobBuilder_allocRunPos'>allocRunPos</a>

---

<a name='SkTextBlobBuilder_allocRunPosH'></a>
## allocRunPosH

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>
</pre>

Returns run with storage for <a href='undocumented#Glyph'>Glyphs</a> and positions along baseline <a href='undocumented#Typeface'>Typeface</a>

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

writable glyph buffer and x

### Example

<div><fiddle-embed name="735c352787b24e490740dedd035987d2"></fiddle-embed></div>

### See Also

<a href='#SkTextBlobBuilder_allocRunPos'>allocRunPos</a> <a href='#SkTextBlobBuilder_allocRun'>allocRun</a>

---

<a name='SkTextBlobBuilder_allocRunPos'></a>
## allocRunPos

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>
</pre>

Returns run with storage for <a href='undocumented#Glyph'>Glyphs</a> and <a href='SkPoint_Reference#Point'>Point</a> positions <a href='undocumented#Typeface'>Typeface</a>

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

writable glyph buffer and <a href='SkPoint_Reference#Point'>Point</a> buffer

### Example

<div><fiddle-embed name="f65c92ff5bfcc95ba58a2ba4d67f944f"></fiddle-embed></div>

### See Also

<a href='#SkTextBlobBuilder_allocRunPosH'>allocRunPosH</a> <a href='#SkTextBlobBuilder_allocRun'>allocRun</a>

---

