SkTextBlob Reference
===


<a name='SkTextBlob'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a> <a href='SkTextBlob_Reference#SkTextBlob'>final</a> : <a href='SkTextBlob_Reference#SkTextBlob'>public</a> <a href='SkTextBlob_Reference#SkTextBlob'>SkNVRefCnt</a><<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>> {
<a href='SkTextBlob_Reference#SkTextBlob'>public</a>:
    <a href='SkTextBlob_Reference#SkTextBlob'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkTextBlob_bounds'>bounds()</a> <a href='#SkTextBlob_bounds'>const</a>;
    <a href='#SkTextBlob_bounds'>uint32_t</a> <a href='#SkTextBlob_uniqueID'>uniqueID</a>() <a href='#SkTextBlob_uniqueID'>const</a>;
    <a href='#SkTextBlob_uniqueID'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>> <a href='#SkTextBlob_MakeFromText'>MakeFromText</a>(<a href='#SkTextBlob_MakeFromText'>const</a> <a href='#SkTextBlob_MakeFromText'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>byteLength</a>, <a href='undocumented#Text'>const</a> <a href='undocumented#SkFont'>SkFont</a>& <a href='undocumented#Font'>font</a>,
                               <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>encoding</a> = <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kUTF8_TextEncoding'>kUTF8_TextEncoding</a>);
    <a href='#SkPaint_kUTF8_TextEncoding'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>> <a href='#SkTextBlob_MakeFromString'>MakeFromString</a>(<a href='#SkTextBlob_MakeFromString'>const</a> <a href='#SkTextBlob_MakeFromString'>char</a>* <a href='undocumented#String'>string</a>, <a href='undocumented#String'>const</a> <a href='undocumented#SkFont'>SkFont</a>& <a href='undocumented#Font'>font</a>,
                                    <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>encoding</a> = <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kUTF8_TextEncoding'>kUTF8_TextEncoding</a>);
    <a href='#SkPaint_kUTF8_TextEncoding'>size_t</a> <a href='#SkTextBlob_serialize'>serialize</a>(<a href='#SkTextBlob_serialize'>const</a> <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>& <a href='undocumented#SkSerialProcs'>procs</a>, <a href='undocumented#SkSerialProcs'>void</a>* <a href='undocumented#SkSerialProcs'>memory</a>, <a href='undocumented#SkSerialProcs'>size_t</a> <a href='undocumented#SkSerialProcs'>memory_size</a>) <a href='undocumented#SkSerialProcs'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkData'>SkData</a>> <a href='#SkTextBlob_serialize'>serialize</a>(<a href='#SkTextBlob_serialize'>const</a> <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>& <a href='undocumented#SkSerialProcs'>procs</a>) <a href='undocumented#SkSerialProcs'>const</a>;
    <a href='undocumented#SkSerialProcs'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>> <a href='#SkTextBlob_Deserialize'>Deserialize</a>(<a href='#SkTextBlob_Deserialize'>const</a> <a href='#SkTextBlob_Deserialize'>void</a>* <a href='undocumented#Data'>data</a>, <a href='undocumented#Data'>size_t</a> <a href='undocumented#Size'>size</a>,
                                         <a href='undocumented#Size'>const</a> <a href='undocumented#SkDeserialProcs'>SkDeserialProcs</a>& <a href='undocumented#SkDeserialProcs'>procs</a>);
};
</pre>

<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a> <a href='SkTextBlob_Reference#SkTextBlob'>combines</a> <a href='SkTextBlob_Reference#SkTextBlob'>multiple</a> <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>runs</a> <a href='undocumented#Text'>into</a> <a href='undocumented#Text'>an</a> <a href='undocumented#Text'>immutable</a> <a href='undocumented#Text'>container</a>. <a href='undocumented#Text'>Each</a> <a href='undocumented#Text'>text</a>
<a href='undocumented#Text'>run</a> <a href='undocumented#Text'>consists</a> <a href='undocumented#Text'>of</a> <a href='undocumented#Glyph'>Glyphs</a>, <a href='SkPaint_Reference#Paint'>Paint</a>, <a href='SkPaint_Reference#Paint'>and</a> <a href='SkPaint_Reference#Paint'>position</a>. <a href='SkPaint_Reference#Paint'>Only</a> <a href='SkPaint_Reference#Paint'>parts</a> <a href='SkPaint_Reference#Paint'>of</a> <a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>related</a> <a href='SkPaint_Reference#Paint'>to</a>
<a href='SkPaint_Reference#Paint'>fonts</a> <a href='SkPaint_Reference#Paint'>and</a> <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>rendering</a> <a href='undocumented#Text'>are</a> <a href='undocumented#Text'>used</a> <a href='undocumented#Text'>by</a> <a href='undocumented#Text'>run</a>.

<a name='SkTextBlob_bounds'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkTextBlob_bounds'>bounds()</a> <a href='#SkTextBlob_bounds'>const</a>
</pre>

Returns conservative bounding box. Uses <a href='SkPaint_Reference#SkPaint'>SkPaint</a> <a href='SkPaint_Reference#SkPaint'>associated</a> <a href='SkPaint_Reference#SkPaint'>with</a> <a href='SkPaint_Reference#SkPaint'>each</a> <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>to</a>
determine <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>bounds</a>, <a href='undocumented#Glyph'>and</a> <a href='undocumented#Glyph'>unions</a> <a href='undocumented#Glyph'>all</a> <a href='undocumented#Glyph'>bounds</a>. <a href='undocumented#Glyph'>Returned</a> <a href='undocumented#Glyph'>bounds</a> <a href='undocumented#Glyph'>may</a> <a href='undocumented#Glyph'>be</a>
larger than the bounds of all <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>in</a> <a href='undocumented#Glyph'>runs</a>.

### Return Value

conservative bounding box

### Example

<div><fiddle-embed name="52ba6c8a9483df8c373fdb49b32a5e19"></fiddle-embed></div>

### See Also

<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_getBounds'>getBounds</a>

<a name='SkTextBlob_uniqueID'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkTextBlob_uniqueID'>uniqueID</a>() <a href='#SkTextBlob_uniqueID'>const</a>
</pre>

Returns a non-zero value unique among all <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>blobs</a>.

### Return Value

identifier for <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>

### Example

<div><fiddle-embed name="5a29bcc0076950339c955149c34e1d46"></fiddle-embed></div>

### See Also

<a href='undocumented#SkRefCnt'>SkRefCnt</a>

<a name='SkTextBlob_MakeFromText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>&<a href='SkTextBlob_Reference#SkTextBlob'>gt</a>; <a href='#SkTextBlob_MakeFromText'>MakeFromText</a>(<a href='#SkTextBlob_MakeFromText'>const</a> <a href='#SkTextBlob_MakeFromText'>void</a>* <a href='undocumented#Text'>text</a>, <a href='undocumented#Text'>size_t</a> <a href='undocumented#Text'>byteLength</a>, <a href='undocumented#Text'>const</a> <a href='undocumented#SkFont'>SkFont</a>& <a href='undocumented#Font'>font</a>,
                                      <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>encoding</a> = <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kUTF8_TextEncoding'>kUTF8_TextEncoding</a>)
</pre>

Creates <a href='#Text_Blob'>Text_Blob</a> <a href='#Text_Blob'>with</a> <a href='#Text_Blob'>a</a> <a href='#Text_Blob'>single</a> <a href='#Text_Blob'>run</a>. <a href='#SkTextBlob_MakeFromText_text'>text</a> <a href='#SkTextBlob_MakeFromText_text'>meaning</a> <a href='#SkTextBlob_MakeFromText_text'>depends</a> <a href='#SkTextBlob_MakeFromText_text'>on</a> <a href='#Paint_Text_Encoding'>Paint_Text_Encoding</a>;
<a href='#Paint_Text_Encoding'>by</a> <a href='#Paint_Text_Encoding'>default</a>, <a href='#SkTextBlob_MakeFromText_text'>text</a> <a href='#SkTextBlob_MakeFromText_text'>is</a> <a href='#SkTextBlob_MakeFromText_text'>encoded</a> <a href='#SkTextBlob_MakeFromText_text'>as</a> <a href='#SkTextBlob_MakeFromText_text'>UTF-8</a>.

<a href='#SkTextBlob_MakeFromText_font'>font</a> <a href='#SkTextBlob_MakeFromText_font'>contains</a> <a href='#SkTextBlob_MakeFromText_font'>attributes</a> <a href='#SkTextBlob_MakeFromText_font'>used</a> <a href='#SkTextBlob_MakeFromText_font'>to</a> <a href='#SkTextBlob_MakeFromText_font'>define</a> <a href='#SkTextBlob_MakeFromText_font'>the</a> <a href='#SkTextBlob_MakeFromText_font'>run</a> <a href='#SkTextBlob_MakeFromText_text'>text</a>: <a href='undocumented#Typeface'>Typeface</a>, <a href='#Paint_Text_Size'>Paint_Text_Size</a>, <a href='#Paint_Text_Scale_X'>Paint_Text_Scale_X</a>,
<a href='#Paint_Text_Skew_X'>Paint_Text_Skew_X</a>, <a href='#Paint_Hinting'>Paint_Hinting</a>, <a href='#Paint_Anti_Alias'>Anti_Alias</a>, <a href='#Paint_Fake_Bold'>Paint_Fake_Bold</a>,
<a href='#Paint_Font_Embedded_Bitmaps'>Font_Embedded_Bitmaps</a>, <a href='#Paint_Full_Hinting_Spacing'>Full_Hinting_Spacing</a>, <a href='#Paint_LCD_Text'>LCD_Text</a>, <a href='#Paint_Linear_Text'>Linear_Text</a>,
<a href='#Paint_Linear_Text'>and</a> <a href='#Paint_Subpixel_Text'>Subpixel_Text</a>
.

### Parameters

<table>  <tr>    <td><a name='SkTextBlob_MakeFromText_text'><code><strong>text</strong></code></a></td>
    <td>character code <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>or</a> <a href='undocumented#Glyph'>Glyphs</a> <a href='undocumented#Glyph'>drawn</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlob_MakeFromText_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>byte length of <a href='#SkTextBlob_MakeFromText_text'>text</a> <a href='#SkTextBlob_MakeFromText_text'>array</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlob_MakeFromText_font'><code><strong>font</strong></code></a></td>
    <td><a href='#SkTextBlob_MakeFromText_text'>text</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Typeface'>typeface</a>,  <a href='#SkTextBlob_MakeFromText_text'>text scale</a>, <a href='#SkTextBlob_MakeFromText_text'>and</a> <a href='#SkTextBlob_MakeFromText_text'>so</a> <a href='#SkTextBlob_MakeFromText_text'>on</a>, <a href='#SkTextBlob_MakeFromText_text'>used</a> <a href='#SkTextBlob_MakeFromText_text'>to</a> <a href='#SkTextBlob_MakeFromText_text'>draw</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlob_MakeFromText_encoding'><code><strong>encoding</strong></code></a></td>
    <td>one of: <a href='undocumented#kUTF8_SkTextEncoding'>kUTF8_SkTextEncoding</a>, <a href='undocumented#kUTF16_SkTextEncoding'>kUTF16_SkTextEncoding</a>,
<a href='undocumented#kUTF32_SkTextEncoding'>kUTF32_SkTextEncoding</a>, <a href='undocumented#kGlyphID_SkTextEncoding'>kGlyphID_SkTextEncoding</a>
</td>
  </tr>
</table>

### Return Value

<a href='#Text_Blob'>Text_Blob</a> <a href='#Text_Blob'>constructed</a> <a href='#Text_Blob'>from</a> <a href='#Text_Blob'>one</a> <a href='#Text_Blob'>run</a>

### Example

<div><fiddle-embed name="bec2252bc36dc8fd023015629d60c405"></fiddle-embed></div>

### See Also

<a href='#SkTextBlob_MakeFromString'>MakeFromString</a> <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>SkTextBlobBuilder</a>

<a name='SkTextBlob_MakeFromString'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>&<a href='SkTextBlob_Reference#SkTextBlob'>gt</a>; <a href='#SkTextBlob_MakeFromString'>MakeFromString</a>(<a href='#SkTextBlob_MakeFromString'>const</a> <a href='#SkTextBlob_MakeFromString'>char</a>* <a href='undocumented#String'>string</a>, <a href='undocumented#String'>const</a> <a href='undocumented#SkFont'>SkFont</a>& <a href='undocumented#Font'>font</a>,
                                       <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> <a href='#SkPaint_TextEncoding'>encoding</a> = <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kUTF8_TextEncoding'>kUTF8_TextEncoding</a>)
</pre>

Creates <a href='#Text_Blob'>Text_Blob</a> <a href='#Text_Blob'>with</a> <a href='#Text_Blob'>a</a> <a href='#Text_Blob'>single</a> <a href='#Text_Blob'>run</a>. <a href='#SkTextBlob_MakeFromString_string'>string</a> <a href='#SkTextBlob_MakeFromString_string'>meaning</a> <a href='#SkTextBlob_MakeFromString_string'>depends</a> <a href='#SkTextBlob_MakeFromString_string'>on</a> <a href='#Paint_Text_Encoding'>Paint_Text_Encoding</a>;
<a href='#Paint_Text_Encoding'>by</a> <a href='#Paint_Text_Encoding'>default</a>, <a href='#SkTextBlob_MakeFromString_string'>string</a> <a href='#SkTextBlob_MakeFromString_string'>is</a> <a href='#SkTextBlob_MakeFromString_string'>encoded</a> <a href='#SkTextBlob_MakeFromString_string'>as</a> <a href='#SkTextBlob_MakeFromString_string'>UTF-8</a>.

<a href='#SkTextBlob_MakeFromString_font'>font</a> <a href='#SkTextBlob_MakeFromString_font'>contains</a> <a href='#Paint_Font_Metrics'>Paint_Font_Metrics</a> <a href='#Paint_Font_Metrics'>used</a> <a href='#Paint_Font_Metrics'>to</a> <a href='#Paint_Font_Metrics'>define</a> <a href='#Paint_Font_Metrics'>the</a> <a href='#Paint_Font_Metrics'>run</a> <a href='undocumented#Text'>text</a>: <a href='undocumented#Typeface'>Typeface</a>, <a href='#Paint_Text_Size'>Paint_Text_Size</a>, <a href='#Paint_Text_Scale_X'>Paint_Text_Scale_X</a>,
<a href='#Paint_Text_Skew_X'>Paint_Text_Skew_X</a>, <a href='#Paint_Hinting'>Paint_Hinting</a>, <a href='#Paint_Anti_Alias'>Anti_Alias</a>, <a href='#Paint_Fake_Bold'>Paint_Fake_Bold</a>,
<a href='#Paint_Font_Embedded_Bitmaps'>Font_Embedded_Bitmaps</a>, <a href='#Paint_Full_Hinting_Spacing'>Full_Hinting_Spacing</a>, <a href='#Paint_LCD_Text'>LCD_Text</a>, <a href='#Paint_Linear_Text'>Linear_Text</a>,
<a href='#Paint_Linear_Text'>and</a> <a href='#Paint_Subpixel_Text'>Subpixel_Text</a>
.

### Parameters

<table>  <tr>    <td><a name='SkTextBlob_MakeFromString_string'><code><strong>string</strong></code></a></td>
    <td>character code <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>or</a> <a href='undocumented#Glyph'>Glyphs</a> <a href='undocumented#Glyph'>drawn</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlob_MakeFromString_font'><code><strong>font</strong></code></a></td>
    <td><a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Typeface'>typeface</a>,  <a href='undocumented#Text'>text scale</a>, <a href='undocumented#Text'>and</a> <a href='undocumented#Text'>so</a> <a href='undocumented#Text'>on</a>, <a href='undocumented#Text'>used</a> <a href='undocumented#Text'>to</a> <a href='undocumented#Text'>draw</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlob_MakeFromString_encoding'><code><strong>encoding</strong></code></a></td>
    <td>one of: <a href='undocumented#kUTF8_SkTextEncoding'>kUTF8_SkTextEncoding</a>, <a href='undocumented#kUTF16_SkTextEncoding'>kUTF16_SkTextEncoding</a>,
<a href='undocumented#kUTF32_SkTextEncoding'>kUTF32_SkTextEncoding</a>, <a href='undocumented#kGlyphID_SkTextEncoding'>kGlyphID_SkTextEncoding</a>
</td>
  </tr>
</table>

### Return Value

<a href='#Text_Blob'>Text_Blob</a> <a href='#Text_Blob'>constructed</a> <a href='#Text_Blob'>from</a> <a href='#Text_Blob'>one</a> <a href='#Text_Blob'>run</a>

### Example

<div><fiddle-embed name="a5af182e793eed3f2bb3b0efc2cf4852"></fiddle-embed></div>

### See Also

<a href='#SkTextBlob_MakeFromText'>MakeFromText</a> <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>SkTextBlobBuilder</a>

<a name='SkTextBlob_serialize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkTextBlob_serialize'>serialize</a>(<a href='#SkTextBlob_serialize'>const</a> <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>& <a href='undocumented#SkSerialProcs'>procs</a>, <a href='undocumented#SkSerialProcs'>void</a>* <a href='undocumented#SkSerialProcs'>memory</a>, <a href='undocumented#SkSerialProcs'>size_t</a> <a href='undocumented#SkSerialProcs'>memory_size</a>) <a href='undocumented#SkSerialProcs'>const</a>
</pre>

Writes <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>to</a> <a href='undocumented#Data'>allow</a> <a href='undocumented#Data'>later</a> <a href='undocumented#Data'>reconstruction</a> <a href='undocumented#Data'>of</a> <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>. <a href='#SkTextBlob_serialize_memory'>memory</a> <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>storage</a>
to receive the encoded <a href='undocumented#Data'>data</a>, <a href='undocumented#Data'>and</a> <a href='#SkTextBlob_serialize_memory_size'>memory_size</a> <a href='#SkTextBlob_serialize_memory_size'>describes</a> <a href='#SkTextBlob_serialize_memory_size'>the</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Size'>storage</a>.
Returns bytes used if provided storage is large enough to hold all <a href='undocumented#Data'>data</a>;
otherwise, returns zero.

<a href='#SkTextBlob_serialize_procs'>procs</a>.<a href='#SkSerialProcs_fTypefaceProc'>fTypefaceProc</a> <a href='#SkSerialProcs_fTypefaceProc'>permits</a> <a href='#SkSerialProcs_fTypefaceProc'>supplying</a> <a href='#SkSerialProcs_fTypefaceProc'>a</a> <a href='#SkSerialProcs_fTypefaceProc'>custom</a> <a href='#SkSerialProcs_fTypefaceProc'>function</a> <a href='#SkSerialProcs_fTypefaceProc'>to</a> <a href='#SkSerialProcs_fTypefaceProc'>encode</a> <a href='undocumented#SkTypeface'>SkTypeface</a>.
If <a href='#SkTextBlob_serialize_procs'>procs</a>.<a href='#SkSerialProcs_fTypefaceProc'>fTypefaceProc</a> <a href='#SkSerialProcs_fTypefaceProc'>is</a> <a href='#SkSerialProcs_fTypefaceProc'>nullptr</a>, <a href='#SkSerialProcs_fTypefaceProc'>default</a> <a href='#SkSerialProcs_fTypefaceProc'>encoding</a> <a href='#SkSerialProcs_fTypefaceProc'>is</a> <a href='#SkSerialProcs_fTypefaceProc'>used</a>. <a href='#SkTextBlob_serialize_procs'>procs</a>.<a href='#SkSerialProcs_fTypefaceCtx'>fTypefaceCtx</a>
may be used to provide user context to <a href='#SkTextBlob_serialize_procs'>procs</a>.<a href='#SkSerialProcs_fTypefaceProc'>fTypefaceProc</a>; <a href='#SkTextBlob_serialize_procs'>procs</a>.<a href='#SkSerialProcs_fTypefaceProc'>fTypefaceProc</a>
is called with a pointer to <a href='undocumented#SkTypeface'>SkTypeface</a> <a href='undocumented#SkTypeface'>and</a> <a href='undocumented#SkTypeface'>user</a> <a href='undocumented#SkTypeface'>context</a>.

### Parameters

<table>  <tr>    <td><a name='SkTextBlob_serialize_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>encoders</a>; <a href='undocumented#Data'>may</a> <a href='undocumented#Data'>be</a> <a href='undocumented#Data'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlob_serialize_memory'><code><strong>memory</strong></code></a></td>
    <td>storage for <a href='undocumented#Data'>data</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlob_serialize_memory_size'><code><strong>memory_size</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Size'>storage</a></td>
  </tr>
</table>

### Return Value

bytes written, or zero if required storage is larger than <a href='#SkTextBlob_serialize_memory_size'>memory_size</a>

### Example

<div><fiddle-embed name="90ce8c327d407b1faac73baa2ebd0378"></fiddle-embed></div>

### See Also

<a href='#SkTextBlob_Deserialize'>Deserialize</a> <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>

<a name='SkTextBlob_serialize_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkData'>SkData</a>&<a href='undocumented#SkData'>gt</a>; <a href='#SkTextBlob_serialize'>serialize</a>(<a href='#SkTextBlob_serialize'>const</a> <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>& <a href='undocumented#SkSerialProcs'>procs</a>) <a href='undocumented#SkSerialProcs'>const</a>
</pre>

Returns storage containing <a href='undocumented#SkData'>SkData</a> <a href='undocumented#SkData'>describing</a> <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>, <a href='SkTextBlob_Reference#SkTextBlob'>using</a> <a href='SkTextBlob_Reference#SkTextBlob'>optional</a> <a href='SkTextBlob_Reference#SkTextBlob'>custom</a>
encoders.

<a href='#SkTextBlob_serialize_2_procs'>procs</a>.<a href='#SkSerialProcs_fTypefaceProc'>fTypefaceProc</a> <a href='#SkSerialProcs_fTypefaceProc'>permits</a> <a href='#SkSerialProcs_fTypefaceProc'>supplying</a> <a href='#SkSerialProcs_fTypefaceProc'>a</a> <a href='#SkSerialProcs_fTypefaceProc'>custom</a> <a href='#SkSerialProcs_fTypefaceProc'>function</a> <a href='#SkSerialProcs_fTypefaceProc'>to</a> <a href='#SkSerialProcs_fTypefaceProc'>encode</a> <a href='undocumented#SkTypeface'>SkTypeface</a>.
If <a href='#SkTextBlob_serialize_2_procs'>procs</a>.<a href='#SkSerialProcs_fTypefaceProc'>fTypefaceProc</a> <a href='#SkSerialProcs_fTypefaceProc'>is</a> <a href='#SkSerialProcs_fTypefaceProc'>nullptr</a>, <a href='#SkSerialProcs_fTypefaceProc'>default</a> <a href='#SkSerialProcs_fTypefaceProc'>encoding</a> <a href='#SkSerialProcs_fTypefaceProc'>is</a> <a href='#SkSerialProcs_fTypefaceProc'>used</a>. <a href='#SkTextBlob_serialize_2_procs'>procs</a>.<a href='#SkSerialProcs_fTypefaceCtx'>fTypefaceCtx</a>
may be used to provide user context to <a href='#SkTextBlob_serialize_2_procs'>procs</a>.<a href='#SkSerialProcs_fTypefaceProc'>fTypefaceProc</a>; <a href='#SkTextBlob_serialize_2_procs'>procs</a>.<a href='#SkSerialProcs_fTypefaceProc'>fTypefaceProc</a>
is called with a pointer to <a href='undocumented#SkTypeface'>SkTypeface</a> <a href='undocumented#SkTypeface'>and</a> <a href='undocumented#SkTypeface'>user</a> <a href='undocumented#SkTypeface'>context</a>.

### Parameters

<table>  <tr>    <td><a name='SkTextBlob_serialize_2_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>encoders</a>; <a href='undocumented#Data'>may</a> <a href='undocumented#Data'>be</a> <a href='undocumented#Data'>nullptr</a></td>
  </tr>
</table>

### Return Value

storage containing serialized <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>

### Example

<div><fiddle-embed name="464201a828f7e94fc01cd57facfcd2f4"></fiddle-embed></div>

### See Also

<a href='#SkTextBlob_Deserialize'>Deserialize</a> <a href='undocumented#SkData'>SkData</a> <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>

<a name='SkTextBlob_Deserialize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>&<a href='SkTextBlob_Reference#SkTextBlob'>gt</a>; <a href='#SkTextBlob_Deserialize'>Deserialize</a>(<a href='#SkTextBlob_Deserialize'>const</a> <a href='#SkTextBlob_Deserialize'>void</a>* <a href='undocumented#Data'>data</a>, <a href='undocumented#Data'>size_t</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Size'>const</a> <a href='undocumented#SkDeserialProcs'>SkDeserialProcs</a>& <a href='undocumented#SkDeserialProcs'>procs</a>)
</pre>

Recreates <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a> <a href='SkTextBlob_Reference#SkTextBlob'>that</a> <a href='SkTextBlob_Reference#SkTextBlob'>was</a> <a href='SkTextBlob_Reference#SkTextBlob'>serialized</a> <a href='SkTextBlob_Reference#SkTextBlob'>into</a> <a href='#SkTextBlob_Deserialize_data'>data</a>. <a href='#SkTextBlob_Deserialize_data'>Returns</a> <a href='#SkTextBlob_Deserialize_data'>constructed</a> <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>
if successful; otherwise, returns nullptr. Fails if <a href='#SkTextBlob_Deserialize_size'>size</a> <a href='#SkTextBlob_Deserialize_size'>is</a> <a href='#SkTextBlob_Deserialize_size'>smaller</a> <a href='#SkTextBlob_Deserialize_size'>than</a>
required <a href='#SkTextBlob_Deserialize_data'>data</a> <a href='#SkTextBlob_Deserialize_data'>length</a>, <a href='#SkTextBlob_Deserialize_data'>or</a> <a href='#SkTextBlob_Deserialize_data'>if</a> <a href='#SkTextBlob_Deserialize_data'>data</a> <a href='#SkTextBlob_Deserialize_data'>does</a> <a href='#SkTextBlob_Deserialize_data'>not</a> <a href='#SkTextBlob_Deserialize_data'>permit</a> <a href='#SkTextBlob_Deserialize_data'>constructing</a> <a href='#SkTextBlob_Deserialize_data'>valid</a> <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>.

<a href='#SkTextBlob_Deserialize_procs'>procs</a>.<a href='#SkDeserialProcs_fTypefaceProc'>fTypefaceProc</a> <a href='#SkDeserialProcs_fTypefaceProc'>permits</a> <a href='#SkDeserialProcs_fTypefaceProc'>supplying</a> <a href='#SkDeserialProcs_fTypefaceProc'>a</a> <a href='#SkDeserialProcs_fTypefaceProc'>custom</a> <a href='#SkDeserialProcs_fTypefaceProc'>function</a> <a href='#SkDeserialProcs_fTypefaceProc'>to</a> <a href='#SkDeserialProcs_fTypefaceProc'>decode</a> <a href='undocumented#SkTypeface'>SkTypeface</a>.
If <a href='#SkTextBlob_Deserialize_procs'>procs</a>.<a href='#SkDeserialProcs_fTypefaceProc'>fTypefaceProc</a> <a href='#SkDeserialProcs_fTypefaceProc'>is</a> <a href='#SkDeserialProcs_fTypefaceProc'>nullptr</a>, <a href='#SkDeserialProcs_fTypefaceProc'>default</a> <a href='#SkDeserialProcs_fTypefaceProc'>decoding</a> <a href='#SkDeserialProcs_fTypefaceProc'>is</a> <a href='#SkDeserialProcs_fTypefaceProc'>used</a>. <a href='#SkTextBlob_Deserialize_procs'>procs</a>.<a href='#SkDeserialProcs_fTypefaceCtx'>fTypefaceCtx</a>
may be used to provide user context to <a href='#SkTextBlob_Deserialize_procs'>procs</a>.<a href='#SkDeserialProcs_fTypefaceProc'>fTypefaceProc</a>; <a href='#SkTextBlob_Deserialize_procs'>procs</a>.<a href='#SkDeserialProcs_fTypefaceProc'>fTypefaceProc</a>
is called with a pointer to <a href='undocumented#SkTypeface'>SkTypeface</a> <a href='#SkTextBlob_Deserialize_data'>data</a>, <a href='#SkTextBlob_Deserialize_data'>data</a> <a href='#SkTextBlob_Deserialize_data'>byte</a> <a href='#SkTextBlob_Deserialize_data'>length</a>, <a href='#SkTextBlob_Deserialize_data'>and</a> <a href='#SkTextBlob_Deserialize_data'>user</a> <a href='#SkTextBlob_Deserialize_data'>context</a>.

### Parameters

<table>  <tr>    <td><a name='SkTextBlob_Deserialize_data'><code><strong>data</strong></code></a></td>
    <td>pointer for serial <a href='#SkTextBlob_Deserialize_data'>data</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlob_Deserialize_size'><code><strong>size</strong></code></a></td>
    <td><a href='#SkTextBlob_Deserialize_size'>size</a> <a href='#SkTextBlob_Deserialize_size'>of</a> <a href='#SkTextBlob_Deserialize_data'>data</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlob_Deserialize_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial <a href='#SkTextBlob_Deserialize_data'>data</a> <a href='#SkTextBlob_Deserialize_data'>decoders</a>; <a href='#SkTextBlob_Deserialize_data'>may</a> <a href='#SkTextBlob_Deserialize_data'>be</a> <a href='#SkTextBlob_Deserialize_data'>nullptr</a></td>
  </tr>
</table>

### Return Value

<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a> <a href='SkTextBlob_Reference#SkTextBlob'>constructed</a> <a href='SkTextBlob_Reference#SkTextBlob'>from</a> <a href='#SkTextBlob_Deserialize_data'>data</a> <a href='#SkTextBlob_Deserialize_data'>in</a> <a href='#SkTextBlob_Deserialize_data'>memory</a>

### Example

<div><fiddle-embed name="68b6d0208eb0b4de67fc152381af7a58"><div><a href='undocumented#Text'>Text</a> "<a href='undocumented#Text'>Hacker</a>" <a href='undocumented#Text'>replaces</a> "<a href='undocumented#Text'>World</a>!", <a href='undocumented#Text'>but</a> <a href='undocumented#Text'>does</a> <a href='undocumented#Text'>not</a> <a href='undocumented#Text'>update</a> <a href='undocumented#Text'>its</a> <a href='undocumented#Text'>metrics</a>.
<a href='undocumented#Text'>When</a> <a href='undocumented#Text'>drawn</a>, "<a href='undocumented#Text'>Hacker</a>" <a href='undocumented#Text'>uses</a> <a href='undocumented#Text'>the</a> <a href='undocumented#Text'>spacing</a> <a href='undocumented#Text'>computed</a> <a href='undocumented#Text'>for</a> "<a href='undocumented#Text'>World</a>!".
</div></fiddle-embed></div>

### See Also

<a href='#SkTextBlob_serialize'>serialize</a> <a href='undocumented#SkDeserialProcs'>SkDeserialProcs</a>

