SkTextBlobBuilder Reference
===

# <a name='Text_Blob_Builder'>Text Blob Builder</a>

# <a name='SkTextBlobBuilder'>Class SkTextBlobBuilder</a>

## <a name='Struct'>Struct</a>


SkTextBlobBuilder uses C++ structs to declare the public data structures and interfaces.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>storage for <a href='undocumented#Glyph'>Glyphs</a> and <a href='undocumented#Glyph'>Glyph</a> positions</td>
  </tr>
</table>

Helper class for constructing <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>.

## Overview

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Constructor'>Constructors</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>functions that construct <a href='#SkTextBlobBuilder'>SkTextBlobBuilder</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Member_Function'>Functions</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>global and class member functions</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Struct'>Struct Declarations</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>embedded struct members</td>
  </tr>
</table>


## <a name='Class'>Class</a>


SkTextBlobBuilder uses C++ classes to declare the public data structures and interfaces.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
</table>

## <a name='Constructor'>Constructor</a>


SkTextBlobBuilder can be constructed or initialized by these functions, including C++ class constructors.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkTextBlobBuilder_empty_constructor'>SkTextBlobBuilder()</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs with default values</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkTextBlobBuilder_make'>make</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs <a href='SkTextBlob_Reference#Text_Blob'>Text Blob</a> from bulider</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkTextBlobBuilder_destructor'>~SkTextBlobBuilder()</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>deletes storage</td>
  </tr>
</table>

## <a name='Member_Function'>Member Function</a>


SkTextBlobBuilder member functions read and modify the structure properties.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkTextBlobBuilder_allocRun'>allocRun</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns writable glyph buffer at <a href='SkPoint_Reference#Point'>Point</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkTextBlobBuilder_allocRunPos'>allocRunPos</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns writable glyph and <a href='SkPoint_Reference#Point'>Point</a> buffers</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkTextBlobBuilder_allocRunPosH'>allocRunPosH</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns writable glyph and x-axis position buffers</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkTextBlobBuilder_make'>make</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs <a href='SkTextBlob_Reference#Text_Blob'>Text Blob</a> from bulider</td>
  </tr>
</table>

# <a name='SkTextBlobBuilder_RunBuffer'>Struct SkTextBlobBuilder::RunBuffer</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    struct <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a> {
        <a href='undocumented#SkGlyphID'>SkGlyphID</a>* glyphs;
        <a href='undocumented#SkScalar'>SkScalar</a>* pos;
        char* <a href='#SkTextBlobBuilder_RunBuffer_utf8text'>utf8text</a>;
        uint32_t* clusters;
    };
</pre>

<a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a> supplies storage for <a href='undocumented#Glyph'>Glyphs</a> and positions within a run.

A run is a sequence of <a href='undocumented#Glyph'>Glyphs</a> sharing <a href='SkPaint_Reference#Font_Metrics'>Paint Font Metrics</a> and positioning.
Each run may position its <a href='undocumented#Glyph'>Glyphs</a> in one of three ways:
by specifying where the first <a href='undocumented#Glyph'>Glyph</a> is drawn, and allowing <a href='SkPaint_Reference#Font_Metrics'>Paint Font Metrics</a> to
determine the advance to subsequent <a href='undocumented#Glyph'>Glyphs</a>; by specifying a baseline, and
the position on that baseline for each <a href='undocumented#Glyph'>Glyph</a> in run; or by providing <a href='SkPoint_Reference#Point'>Point</a>
array, one per <a href='undocumented#Glyph'>Glyph</a>.

## <a name='Member'>Member</a>


SkTextBlobBuilder::RunBuffer members may be read and written directly without using a member function.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>clusters</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>reserved for future use</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>glyphs</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>storage for <a href='undocumented#Glyph'>Glyphs</a> in run</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>pos</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>storage for positions in run</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkTextBlobBuilder_RunBuffer_utf8text'>utf8text</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>reserved for future use</td>
  </tr>
</table>

### Members

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Name</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkGlyphID*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkTextBlobBuilder_RunBuffer_glyphs'><code>glyphs</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
glyphs points to memory for one or more <a href='undocumented#Glyph'>Glyphs</a>. glyphs memory must be
written to by the caller.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkTextBlobBuilder_RunBuffer_pos'><code>pos</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
pos points to memory for <a href='undocumented#Glyph'>Glyph</a> positions. Depending on how <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>
is allocated, pos may point to zero bytes per <a href='undocumented#Glyph'>Glyph</a>, one <a href='undocumented#Scalar'>Scalar</a> per <a href='undocumented#Glyph'>Glyph</a>,
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
Reserved for future use. clusters should not be read or written.
</td>
  </tr>
</table>

### See Also

<a href='#SkTextBlobBuilder_allocRun'>allocRun</a> <a href='#SkTextBlobBuilder_allocRunPos'>allocRunPos</a> <a href='#SkTextBlobBuilder_allocRunPosH'>allocRunPosH</a>

<a name='SkTextBlobBuilder_empty_constructor'></a>
## SkTextBlobBuilder

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkTextBlobBuilder'>SkTextBlobBuilder</a>()
</pre>

Constructs empty <a href='#Text_Blob_Builder'>Text Blob Builder</a>. By default, <a href='#Text_Blob_Builder'>Text Blob Builder</a> has no runs.

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
<a href='#SkTextBlobBuilder_destructor'>~SkTextBlobBuilder</a>()
</pre>

Deletes data allocated internally by <a href='#Text_Blob_Builder'>Text Blob Builder</a>.

### See Also

<a href='#SkTextBlobBuilder_empty_constructor'>SkTextBlobBuilder()</a>

---

<a name='SkTextBlobBuilder_make'></a>
## make

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>&gt; <a href='#SkTextBlobBuilder_make'>make</a>()
</pre>

Returns <a href='SkTextBlob_Reference#Text_Blob'>Text Blob</a> built from runs of <a href='undocumented#Glyph'>Glyphs</a> added by builder. Returned
<a href='SkTextBlob_Reference#Text_Blob'>Text Blob</a> is immutable; it may be copied, but its contents may not be altered.
Returns nullptr if no runs of <a href='undocumented#Glyph'>Glyphs</a> were added by builder.

Resets <a href='#Text_Blob_Builder'>Text Blob Builder</a> to its initial empty state, allowing it to be
reused to build a new set of runs.

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
const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRun'>allocRun</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& font, int count, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y,
                          const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds = nullptr)
</pre>

Returns run with storage for <a href='undocumented#Glyph'>Glyphs</a>. Caller must write <a href='#SkTextBlobBuilder_allocRun_count'>count</a> <a href='undocumented#Glyph'>Glyphs</a> to
<a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>.glyphs before next call to FontBlobBuilder.

<a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>.utf8text, and <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>.clusters should be ignored.

<a href='undocumented#Glyph'>Glyphs</a> share <a href='SkPaint_Reference#Font_Metrics'>Paint Font Metrics</a> in <a href='#SkTextBlobBuilder_allocRun_font'>font</a>, including: <a href='undocumented#Typeface'>Typeface</a>, <a href='SkPaint_Reference#Text_Size'>Paint Text Size</a>, <a href='SkPaint_Reference#Text_Scale_X'>Paint Text Scale X</a>,
<a href='SkPaint_Reference#Text_Skew_X'>Paint Text Skew X</a>, <a href='SkPaint_Reference#Text_Align'>Paint Text Align</a>, <a href='SkPaint_Reference#Hinting'>Paint Hinting</a>, <a href='SkPaint_Reference#Anti_Alias'>Anti Alias</a>, <a href='SkPaint_Reference#Fake_Bold'>Paint Fake Bold</a>,
<a href='SkPaint_Reference#Font_Embedded_Bitmaps'>Font Embedded Bitmaps</a>, <a href='SkPaint_Reference#Full_Hinting_Spacing'>Full Hinting Spacing</a>, <a href='SkPaint_Reference#LCD_Text'>LCD Text</a>, <a href='SkPaint_Reference#Linear_Text'>Linear Text</a>,
and <a href='SkPaint_Reference#Subpixel_Text'>Subpixel Text</a>.

<a href='undocumented#Glyph'>Glyphs</a> are positioned on a baseline at (<a href='#SkTextBlobBuilder_allocRun_x'>x</a>, <a href='#SkTextBlobBuilder_allocRun_y'>y</a>), using <a href='#SkTextBlobBuilder_allocRun_font'>font</a> <a href='SkPaint_Reference#Font_Metrics'>Paint Font Metrics</a> to
determine their relative placement.

<a href='#SkTextBlobBuilder_allocRun_bounds'>bounds</a> defines an optional bounding box, used to suppress drawing when <a href='SkTextBlob_Reference#Text_Blob'>Text Blob</a>
<a href='#SkTextBlobBuilder_allocRun_bounds'>bounds</a> does not intersect <a href='SkSurface_Reference#Surface'>Surface</a> <a href='#SkTextBlobBuilder_allocRun_bounds'>bounds</a>. If <a href='#SkTextBlobBuilder_allocRun_bounds'>bounds</a> is nullptr, <a href='SkTextBlob_Reference#Text_Blob'>Text Blob</a> <a href='#SkTextBlobBuilder_allocRun_bounds'>bounds</a>
is computed from (<a href='#SkTextBlobBuilder_allocRun_x'>x</a>, <a href='#SkTextBlobBuilder_allocRun_y'>y</a>) and <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>.glyphs <a href='SkPaint_Reference#Font_Metrics'>Paint Font Metrics</a>.

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
const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRunPosH'>allocRunPosH</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& font, int count, <a href='undocumented#SkScalar'>SkScalar</a> y,
                              const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds = nullptr)
</pre>

Returns run with storage for <a href='undocumented#Glyph'>Glyphs</a> and positions along baseline. Caller must
write <a href='#SkTextBlobBuilder_allocRunPosH_count'>count</a> <a href='undocumented#Glyph'>Glyphs</a> to <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>.glyphs, and <a href='#SkTextBlobBuilder_allocRunPosH_count'>count</a> <a href='undocumented#Scalar'>Scalars</a> to <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>.pos;
before next call to FontBlobBuilder.

<a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>.utf8text, and <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>.clusters should be ignored.

<a href='undocumented#Glyph'>Glyphs</a> share <a href='SkPaint_Reference#Font_Metrics'>Paint Font Metrics</a> in <a href='#SkTextBlobBuilder_allocRunPosH_font'>font</a>, including: <a href='undocumented#Typeface'>Typeface</a>, <a href='SkPaint_Reference#Text_Size'>Paint Text Size</a>, <a href='SkPaint_Reference#Text_Scale_X'>Paint Text Scale X</a>,
<a href='SkPaint_Reference#Text_Skew_X'>Paint Text Skew X</a>, <a href='SkPaint_Reference#Text_Align'>Paint Text Align</a>, <a href='SkPaint_Reference#Hinting'>Paint Hinting</a>, <a href='SkPaint_Reference#Anti_Alias'>Anti Alias</a>, <a href='SkPaint_Reference#Fake_Bold'>Paint Fake Bold</a>,
<a href='SkPaint_Reference#Font_Embedded_Bitmaps'>Font Embedded Bitmaps</a>, <a href='SkPaint_Reference#Full_Hinting_Spacing'>Full Hinting Spacing</a>, <a href='SkPaint_Reference#LCD_Text'>LCD Text</a>, <a href='SkPaint_Reference#Linear_Text'>Linear Text</a>,
and <a href='SkPaint_Reference#Subpixel_Text'>Subpixel Text</a>.

<a href='undocumented#Glyph'>Glyphs</a> are positioned on a baseline at <a href='#SkTextBlobBuilder_allocRunPosH_y'>y</a>, using x-axis positions written by
caller to <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>.pos.

<a href='#SkTextBlobBuilder_allocRunPosH_bounds'>bounds</a> defines an optional bounding box, used to suppress drawing when <a href='SkTextBlob_Reference#Text_Blob'>Text Blob</a>
<a href='#SkTextBlobBuilder_allocRunPosH_bounds'>bounds</a> does not intersect <a href='SkSurface_Reference#Surface'>Surface</a> <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>bounds</a>. If <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>bounds</a> is nullptr, <a href='SkTextBlob_Reference#Text_Blob'>Text Blob</a> <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>bounds</a>
is computed from <a href='#SkTextBlobBuilder_allocRunPosH_y'>y</a>, <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>.pos, and <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>.glyphs <a href='SkPaint_Reference#Font_Metrics'>Paint Font Metrics</a>.

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

writable glyph buffer and x-axis position buffer

### Example

<div><fiddle-embed name="735c352787b24e490740dedd035987d2"></fiddle-embed></div>

### See Also

<a href='#SkTextBlobBuilder_allocRunPos'>allocRunPos</a> <a href='#SkTextBlobBuilder_allocRun'>allocRun</a>

---

<a name='SkTextBlobBuilder_allocRunPos'></a>
## allocRunPos

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRunPos'>allocRunPos</a>(const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& font, int count, const <a href='SkRect_Reference#SkRect'>SkRect</a>* bounds = nullptr)
</pre>

Returns run with storage for <a href='undocumented#Glyph'>Glyphs</a> and <a href='SkPoint_Reference#Point'>Point</a> positions. Caller must
write <a href='#SkTextBlobBuilder_allocRunPos_count'>count</a> <a href='undocumented#Glyph'>Glyphs</a> to <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>.glyphs, and <a href='#SkTextBlobBuilder_allocRunPos_count'>count</a> <a href='SkPoint_Reference#Point'>Points</a> to <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>.pos;
before next call to FontBlobBuilder.

<a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>.utf8text, and <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>.clusters should be ignored.

<a href='undocumented#Glyph'>Glyphs</a> share <a href='SkPaint_Reference#Font_Metrics'>Paint Font Metrics</a> in <a href='#SkTextBlobBuilder_allocRunPos_font'>font</a>, including: <a href='undocumented#Typeface'>Typeface</a>, <a href='SkPaint_Reference#Text_Size'>Paint Text Size</a>, <a href='SkPaint_Reference#Text_Scale_X'>Paint Text Scale X</a>,
<a href='SkPaint_Reference#Text_Skew_X'>Paint Text Skew X</a>, <a href='SkPaint_Reference#Text_Align'>Paint Text Align</a>, <a href='SkPaint_Reference#Hinting'>Paint Hinting</a>, <a href='SkPaint_Reference#Anti_Alias'>Anti Alias</a>, <a href='SkPaint_Reference#Fake_Bold'>Paint Fake Bold</a>,
<a href='SkPaint_Reference#Font_Embedded_Bitmaps'>Font Embedded Bitmaps</a>, <a href='SkPaint_Reference#Full_Hinting_Spacing'>Full Hinting Spacing</a>, <a href='SkPaint_Reference#LCD_Text'>LCD Text</a>, <a href='SkPaint_Reference#Linear_Text'>Linear Text</a>,
and <a href='SkPaint_Reference#Subpixel_Text'>Subpixel Text</a>.

<a href='undocumented#Glyph'>Glyphs</a> are positioned using <a href='SkPoint_Reference#Point'>Points</a> written by caller to <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>.pos, using
two <a href='undocumented#Scalar'>Scalar</a> values for each <a href='SkPoint_Reference#Point'>Point</a>.

<a href='#SkTextBlobBuilder_allocRunPos_bounds'>bounds</a> defines an optional bounding box, used to suppress drawing when <a href='SkTextBlob_Reference#Text_Blob'>Text Blob</a>
<a href='#SkTextBlobBuilder_allocRunPos_bounds'>bounds</a> does not intersect <a href='SkSurface_Reference#Surface'>Surface</a> <a href='#SkTextBlobBuilder_allocRunPos_bounds'>bounds</a>. If <a href='#SkTextBlobBuilder_allocRunPos_bounds'>bounds</a> is nullptr, <a href='SkTextBlob_Reference#Text_Blob'>Text Blob</a> <a href='#SkTextBlobBuilder_allocRunPos_bounds'>bounds</a>
is computed from <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>.pos, and <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>.glyphs <a href='SkPaint_Reference#Font_Metrics'>Paint Font Metrics</a>.

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

