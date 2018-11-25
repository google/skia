SkTextBlob Reference
===


<a name='SkTextBlob'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a> final : public SkNVRefCnt<<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>> {
public:
    const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkTextBlob_bounds'>bounds()</a> const;
    uint32_t <a href='#SkTextBlob_uniqueID'>uniqueID</a>() const;
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>> <a href='#SkTextBlob_MakeFromText'>MakeFromText</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength, const <a href='undocumented#SkFont'>SkFont</a>& <a href='undocumented#Font'>font</a>,
                               <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> encoding = <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kUTF8_TextEncoding'>kUTF8_TextEncoding</a>);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>> <a href='#SkTextBlob_MakeFromString'>MakeFromString</a>(const char* <a href='undocumented#String'>string</a>, const <a href='undocumented#SkFont'>SkFont</a>& <a href='undocumented#Font'>font</a>,
                                    <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> encoding = <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kUTF8_TextEncoding'>kUTF8_TextEncoding</a>);
    size_t <a href='#SkTextBlob_serialize'>serialize</a>(const <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>& procs, void* memory, size_t memory_size) const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkData'>SkData</a>> <a href='#SkTextBlob_serialize'>serialize</a>(const <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>& procs) const;
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>> <a href='#SkTextBlob_Deserialize'>Deserialize</a>(const void* <a href='undocumented#Data'>data</a>, size_t <a href='undocumented#Size'>size</a>,
                                         const <a href='undocumented#SkDeserialProcs'>SkDeserialProcs</a>& procs);
};
</pre>

<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a> combines multiple <a href='undocumented#Text'>text</a> runs into an immutable container. Each <a href='undocumented#Text'>text</a>
run consists of <a href='undocumented#Glyph'>Glyphs</a>, <a href='SkPaint_Reference#Paint'>Paint</a>, and position. Only parts of <a href='SkPaint_Reference#Paint'>Paint</a> related to
fonts and <a href='undocumented#Text'>text</a> rendering are used by run.

<a name='SkTextBlob_bounds'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkTextBlob_bounds'>bounds()</a> const
</pre>

Returns conservative bounding box. Uses <a href='SkPaint_Reference#SkPaint'>SkPaint</a> associated with each <a href='undocumented#Glyph'>glyph</a> to
determine <a href='undocumented#Glyph'>glyph</a> bounds, and unions all bounds. Returned bounds may be
larger than the bounds of all <a href='undocumented#Glyph'>glyphs</a> in runs.

### Return Value

conservative bounding box

### Example

<div><fiddle-embed name="a22a490fd43bcc1cd3e26430debfb99e"></fiddle-embed></div>

### See Also

<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_getBounds'>getBounds</a>

<a name='SkTextBlob_uniqueID'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkTextBlob_uniqueID'>uniqueID</a>() const
</pre>

Returns a non-zero value unique among all <a href='undocumented#Text'>text</a> blobs.

### Return Value

identifier for <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>

### Example

<div><fiddle-embed name="fdd9c8c4470cc4f725af779e9d6db6e4"></fiddle-embed></div>

### See Also

<a href='undocumented#SkRefCnt'>SkRefCnt</a>

<a name='SkTextBlob_MakeFromText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>&gt; <a href='#SkTextBlob_MakeFromText'>MakeFromText</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength, const <a href='undocumented#SkFont'>SkFont</a>& <a href='undocumented#Font'>font</a>,
                                      <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> encoding = <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kUTF8_TextEncoding'>kUTF8_TextEncoding</a>)
</pre>

Creates <a href='#Text_Blob'>Text_Blob</a> with a single run. <a href='#SkTextBlob_MakeFromText_text'>text</a> meaning depends on <a href='#Paint_Text_Encoding'>Paint_Text_Encoding</a>;
by default, <a href='#SkTextBlob_MakeFromText_text'>text</a> is encoded as UTF-8.

<a href='#SkTextBlob_MakeFromText_font'>font</a> contains attributes used to define the run <a href='#SkTextBlob_MakeFromText_text'>text</a>: <a href='undocumented#Typeface'>Typeface</a>, <a href='#Paint_Text_Size'>Paint_Text_Size</a>, <a href='#Paint_Text_Scale_X'>Paint_Text_Scale_X</a>,
<a href='#Paint_Text_Skew_X'>Paint_Text_Skew_X</a>, <a href='#Paint_Hinting'>Paint_Hinting</a>, <a href='#Paint_Anti_Alias'>Anti_Alias</a>, <a href='#Paint_Fake_Bold'>Paint_Fake_Bold</a>,
<a href='#Paint_Font_Embedded_Bitmaps'>Font_Embedded_Bitmaps</a>, <a href='#Paint_Full_Hinting_Spacing'>Full_Hinting_Spacing</a>, <a href='#Paint_LCD_Text'>LCD_Text</a>, <a href='#Paint_Linear_Text'>Linear_Text</a>,
and <a href='#Paint_Subpixel_Text'>Subpixel_Text</a>
.

### Parameters

<table>  <tr>    <td><a name='SkTextBlob_MakeFromText_text'><code><strong>text</strong></code></a></td>
    <td>character code <a href='SkPoint_Reference#Point'>points</a> or <a href='undocumented#Glyph'>Glyphs</a> drawn</td>
  </tr>
  <tr>    <td><a name='SkTextBlob_MakeFromText_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>byte length of <a href='#SkTextBlob_MakeFromText_text'>text</a> array</td>
  </tr>
  <tr>    <td><a name='SkTextBlob_MakeFromText_font'><code><strong>font</strong></code></a></td>
    <td><a href='#SkTextBlob_MakeFromText_text'>text</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Typeface'>typeface</a>,  <a href='#SkTextBlob_MakeFromText_text'>text scale</a>, and so on, used to draw</td>
  </tr>
  <tr>    <td><a name='SkTextBlob_MakeFromText_encoding'><code><strong>encoding</strong></code></a></td>
    <td>one of: <a href='undocumented#kUTF8_SkTextEncoding'>kUTF8_SkTextEncoding</a>, <a href='undocumented#kUTF16_SkTextEncoding'>kUTF16_SkTextEncoding</a>,
<a href='undocumented#kUTF32_SkTextEncoding'>kUTF32_SkTextEncoding</a>, <a href='undocumented#kGlyphID_SkTextEncoding'>kGlyphID_SkTextEncoding</a>
</td>
  </tr>
</table>

### Return Value

<a href='#Text_Blob'>Text_Blob</a> constructed from one run

### Example

<div><fiddle-embed name="bec2252bc36dc8fd023015629d60c405"></fiddle-embed></div>

### See Also

<a href='#SkTextBlob_MakeFromString'>MakeFromString</a> <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>SkTextBlobBuilder</a>

<a name='SkTextBlob_MakeFromString'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>&gt; <a href='#SkTextBlob_MakeFromString'>MakeFromString</a>(const char* <a href='undocumented#String'>string</a>, const <a href='undocumented#SkFont'>SkFont</a>& <a href='undocumented#Font'>font</a>,
                                       <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_TextEncoding'>TextEncoding</a> encoding = <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kUTF8_TextEncoding'>kUTF8_TextEncoding</a>)
</pre>

Creates <a href='#Text_Blob'>Text_Blob</a> with a single run. <a href='#SkTextBlob_MakeFromString_string'>string</a> meaning depends on <a href='#Paint_Text_Encoding'>Paint_Text_Encoding</a>;
by default, <a href='#SkTextBlob_MakeFromString_string'>string</a> is encoded as UTF-8.

<a href='#SkTextBlob_MakeFromString_font'>font</a> contains <a href='#Paint_Font_Metrics'>Paint_Font_Metrics</a> used to define the run <a href='undocumented#Text'>text</a>: <a href='undocumented#Typeface'>Typeface</a>, <a href='#Paint_Text_Size'>Paint_Text_Size</a>, <a href='#Paint_Text_Scale_X'>Paint_Text_Scale_X</a>,
<a href='#Paint_Text_Skew_X'>Paint_Text_Skew_X</a>, <a href='#Paint_Hinting'>Paint_Hinting</a>, <a href='#Paint_Anti_Alias'>Anti_Alias</a>, <a href='#Paint_Fake_Bold'>Paint_Fake_Bold</a>,
<a href='#Paint_Font_Embedded_Bitmaps'>Font_Embedded_Bitmaps</a>, <a href='#Paint_Full_Hinting_Spacing'>Full_Hinting_Spacing</a>, <a href='#Paint_LCD_Text'>LCD_Text</a>, <a href='#Paint_Linear_Text'>Linear_Text</a>,
and <a href='#Paint_Subpixel_Text'>Subpixel_Text</a>
.

### Parameters

<table>  <tr>    <td><a name='SkTextBlob_MakeFromString_string'><code><strong>string</strong></code></a></td>
    <td>character code <a href='SkPoint_Reference#Point'>points</a> or <a href='undocumented#Glyph'>Glyphs</a> drawn</td>
  </tr>
  <tr>    <td><a name='SkTextBlob_MakeFromString_font'><code><strong>font</strong></code></a></td>
    <td><a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Typeface'>typeface</a>,  <a href='undocumented#Text'>text scale</a>, and so on, used to draw</td>
  </tr>
  <tr>    <td><a name='SkTextBlob_MakeFromString_encoding'><code><strong>encoding</strong></code></a></td>
    <td>one of: <a href='undocumented#kUTF8_SkTextEncoding'>kUTF8_SkTextEncoding</a>, <a href='undocumented#kUTF16_SkTextEncoding'>kUTF16_SkTextEncoding</a>,
<a href='undocumented#kUTF32_SkTextEncoding'>kUTF32_SkTextEncoding</a>, <a href='undocumented#kGlyphID_SkTextEncoding'>kGlyphID_SkTextEncoding</a>
</td>
  </tr>
</table>

### Return Value

<a href='#Text_Blob'>Text_Blob</a> constructed from one run

### Example

<div><fiddle-embed name="a5af182e793eed3f2bb3b0efc2cf4852"></fiddle-embed></div>

### See Also

<a href='#SkTextBlob_MakeFromText'>MakeFromText</a> <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>SkTextBlobBuilder</a>

<a name='SkTextBlob_serialize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkTextBlob_serialize'>serialize</a>(const <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>& procs, void* memory, size_t memory_size) const
</pre>

Writes <a href='undocumented#Data'>data</a> to allow later reconstruction of <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>. <a href='#SkTextBlob_serialize_memory'>memory</a> <a href='SkPoint_Reference#Point'>points</a> to storage
to receive the encoded <a href='undocumented#Data'>data</a>, and <a href='#SkTextBlob_serialize_memory_size'>memory_size</a> describes the <a href='undocumented#Size'>size</a> of storage.
Returns bytes used if provided storage is large enough to hold all <a href='undocumented#Data'>data</a>;
otherwise, returns zero.

<a href='#SkTextBlob_serialize_procs'>procs</a>.<a href='#SkSerialProcs_fTypefaceProc'>fTypefaceProc</a> permits supplying a custom function to encode <a href='undocumented#SkTypeface'>SkTypeface</a>.
If <a href='#SkTextBlob_serialize_procs'>procs</a>.<a href='#SkSerialProcs_fTypefaceProc'>fTypefaceProc</a> is nullptr, default encoding is used. <a href='#SkTextBlob_serialize_procs'>procs</a>.<a href='#SkSerialProcs_fTypefaceCtx'>fTypefaceCtx</a>
may be used to provide user context to <a href='#SkTextBlob_serialize_procs'>procs</a>.<a href='#SkSerialProcs_fTypefaceProc'>fTypefaceProc</a>; <a href='#SkTextBlob_serialize_procs'>procs</a>.<a href='#SkSerialProcs_fTypefaceProc'>fTypefaceProc</a>
is called with a pointer to <a href='undocumented#SkTypeface'>SkTypeface</a> and user context.

### Parameters

<table>  <tr>    <td><a name='SkTextBlob_serialize_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial <a href='undocumented#Data'>data</a> encoders; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkTextBlob_serialize_memory'><code><strong>memory</strong></code></a></td>
    <td>storage for <a href='undocumented#Data'>data</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlob_serialize_memory_size'><code><strong>memory_size</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> of storage</td>
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
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkData'>SkData</a>&gt; <a href='#SkTextBlob_serialize'>serialize</a>(const <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>& procs) const
</pre>

Returns storage containing <a href='undocumented#SkData'>SkData</a> describing <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>, using optional custom
encoders.

<a href='#SkTextBlob_serialize_2_procs'>procs</a>.<a href='#SkSerialProcs_fTypefaceProc'>fTypefaceProc</a> permits supplying a custom function to encode <a href='undocumented#SkTypeface'>SkTypeface</a>.
If <a href='#SkTextBlob_serialize_2_procs'>procs</a>.<a href='#SkSerialProcs_fTypefaceProc'>fTypefaceProc</a> is nullptr, default encoding is used. <a href='#SkTextBlob_serialize_2_procs'>procs</a>.<a href='#SkSerialProcs_fTypefaceCtx'>fTypefaceCtx</a>
may be used to provide user context to <a href='#SkTextBlob_serialize_2_procs'>procs</a>.<a href='#SkSerialProcs_fTypefaceProc'>fTypefaceProc</a>; <a href='#SkTextBlob_serialize_2_procs'>procs</a>.<a href='#SkSerialProcs_fTypefaceProc'>fTypefaceProc</a>
is called with a pointer to <a href='undocumented#SkTypeface'>SkTypeface</a> and user context.

### Parameters

<table>  <tr>    <td><a name='SkTextBlob_serialize_2_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial <a href='undocumented#Data'>data</a> encoders; may be nullptr</td>
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
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>&gt; <a href='#SkTextBlob_Deserialize'>Deserialize</a>(const void* <a href='undocumented#Data'>data</a>, size_t <a href='undocumented#Size'>size</a>, const <a href='undocumented#SkDeserialProcs'>SkDeserialProcs</a>& procs)
</pre>

Recreates <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a> that was serialized into <a href='#SkTextBlob_Deserialize_data'>data</a>. Returns constructed <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>
if successful; otherwise, returns nullptr. Fails if <a href='#SkTextBlob_Deserialize_size'>size</a> is smaller than
required <a href='#SkTextBlob_Deserialize_data'>data</a> length, or if <a href='#SkTextBlob_Deserialize_data'>data</a> does not permit constructing valid <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>.

<a href='#SkTextBlob_Deserialize_procs'>procs</a>.<a href='#SkDeserialProcs_fTypefaceProc'>fTypefaceProc</a> permits supplying a custom function to decode <a href='undocumented#SkTypeface'>SkTypeface</a>.
If <a href='#SkTextBlob_Deserialize_procs'>procs</a>.<a href='#SkDeserialProcs_fTypefaceProc'>fTypefaceProc</a> is nullptr, default decoding is used. <a href='#SkTextBlob_Deserialize_procs'>procs</a>.<a href='#SkDeserialProcs_fTypefaceCtx'>fTypefaceCtx</a>
may be used to provide user context to <a href='#SkTextBlob_Deserialize_procs'>procs</a>.<a href='#SkDeserialProcs_fTypefaceProc'>fTypefaceProc</a>; <a href='#SkTextBlob_Deserialize_procs'>procs</a>.<a href='#SkDeserialProcs_fTypefaceProc'>fTypefaceProc</a>
is called with a pointer to <a href='undocumented#SkTypeface'>SkTypeface</a> <a href='#SkTextBlob_Deserialize_data'>data</a>, <a href='#SkTextBlob_Deserialize_data'>data</a> byte length, and user context.

### Parameters

<table>  <tr>    <td><a name='SkTextBlob_Deserialize_data'><code><strong>data</strong></code></a></td>
    <td>pointer for serial <a href='#SkTextBlob_Deserialize_data'>data</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlob_Deserialize_size'><code><strong>size</strong></code></a></td>
    <td><a href='#SkTextBlob_Deserialize_size'>size</a> of <a href='#SkTextBlob_Deserialize_data'>data</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlob_Deserialize_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial <a href='#SkTextBlob_Deserialize_data'>data</a> decoders; may be nullptr</td>
  </tr>
</table>

### Return Value

<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a> constructed from <a href='#SkTextBlob_Deserialize_data'>data</a> in memory

### Example

<div><fiddle-embed name="68b6d0208eb0b4de67fc152381af7a58"><div><a href='undocumented#Text'>Text</a> "Hacker" replaces "World!", but does not update its metrics.
When drawn, "Hacker" uses the spacing computed for "World!".
</div></fiddle-embed></div>

### See Also

<a href='#SkTextBlob_serialize'>serialize</a> <a href='undocumented#SkDeserialProcs'>SkDeserialProcs</a>

