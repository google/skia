SkTextBlob Reference
===


<a name='SkTextBlob'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a> final : public <a href='undocumented#SkNVRefCnt'>SkNVRefCnt</a><<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>> {

    const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkTextBlob_bounds'>bounds()</a> const;
    uint32_t <a href='#SkTextBlob_uniqueID'>uniqueID</a>() const;
    int <a href='#SkTextBlob_getIntercepts'>getIntercepts</a>(const <a href='undocumented#SkScalar'>SkScalar</a> bounds[2], <a href='undocumented#SkScalar'>SkScalar</a> intervals[],
                      const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = nullptr) const;
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>> <a href='#SkTextBlob_MakeFromText'>MakeFromText</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength, const <a href='SkFont_Reference#SkFont'>SkFont</a>& <a href='SkFont_Reference#Font'>font</a>,
                                      <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> encoding = <a href='undocumented#SkTextEncoding::kUTF8'>SkTextEncoding::kUTF8</a>);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>> <a href='#SkTextBlob_MakeFromString'>MakeFromString</a>(const char* <a href='undocumented#String'>string</a>, const <a href='SkFont_Reference#SkFont'>SkFont</a>& <a href='SkFont_Reference#Font'>font</a>,
                                    <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> encoding = <a href='undocumented#SkTextEncoding::kUTF8'>SkTextEncoding::kUTF8</a>);
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
const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkTextBlob_bounds'>bounds()</a>const
</pre>

Returns conservative bounding box. Uses <a href='SkPaint_Reference#SkPaint'>SkPaint</a> associated with each <a href='undocumented#Glyph'>glyph</a> to
determine <a href='undocumented#Glyph'>glyph</a> bounds, and unions all bounds. Returned bounds may be
larger than the bounds of all <a href='undocumented#Glyph'>glyphs</a> in runs.

### Return Value

conservative bounding box

### Example

<div><fiddle-embed name="fb8b2502bbe52d2029aecdf569dd9fdb"></fiddle-embed></div>

### See Also

<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_getBounds'>getBounds</a>

<a name='SkTextBlob_uniqueID'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkTextBlob_uniqueID'>uniqueID</a>()const
</pre>

Returns a non-zero value unique among all <a href='undocumented#Text'>text</a> blobs.

### Return Value

identifier for <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>

### Example

<div><fiddle-embed name="6e12cceca981ddabc0fc18c380543f34"></fiddle-embed></div>

### See Also

<a href='undocumented#SkRefCnt'>SkRefCnt</a>

<a name='Text_Intercepts'></a>

<a href='#Text_Blob_Text_Intercepts'>Text_Intercepts</a> describe the intersection of drawn <a href='undocumented#Text'>text</a> <a href='undocumented#Glyph'>Glyphs</a> with a pair
of <a href='undocumented#Line'>lines</a> parallel to the <a href='undocumented#Text'>text</a> advance. <a href='#Text_Blob_Text_Intercepts'>Text_Intercepts</a> permits creating a
underline that skips Descenders.

<a name='SkTextBlob_getIntercepts'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkTextBlob_getIntercepts'>getIntercepts</a>(const <a href='undocumented#SkScalar'>SkScalar</a> bounds[2], <a href='undocumented#SkScalar'>SkScalar</a> intervals[], const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>* <a href='SkPaint_Reference#Paint'>paint</a> = nullptr) const;
</pre>

Returns the number of <a href='#SkTextBlob_getIntercepts_intervals'>intervals</a> that intersect <a href='#SkTextBlob_getIntercepts_bounds'>bounds</a>.
<a href='#SkTextBlob_getIntercepts_bounds'>bounds</a> describes a pair of <a href='undocumented#Line'>lines</a> parallel to the <a href='undocumented#Text'>text</a> advance.
The return count is zero or a multiple of two, and is at most twice the number of <a href='undocumented#Glyph'>glyphs</a> in
the the blob.

Pass nullptr for <a href='#SkTextBlob_getIntercepts_intervals'>intervals</a> to determine the <a href='undocumented#Size'>size</a> of the interval array.

Runs within the blob that contain <a href='undocumented#SkRSXform'>SkRSXform</a> are ignored when computing intercepts.

### Parameters

<table>  <tr>    <td><a name='SkTextBlob_getIntercepts_bounds'><code><strong>bounds</strong></code></a></td>
    <td>lower and upper <a href='undocumented#Line'>line</a> parallel to the advance</td>
  </tr>
  <tr>    <td><a name='SkTextBlob_getIntercepts_intervals'><code><strong>intervals</strong></code></a></td>
    <td>returned intersections; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkTextBlob_getIntercepts_paint'><code><strong>paint</strong></code></a></td>
    <td>specifies stroking, <a href='undocumented#SkPathEffect'>SkPathEffect</a> that affects the result; may be nullptr</td>
  </tr>
</table>

### Return Value

number of intersections; may be zero

### Example

<div><fiddle-embed name="e9d4eb8ece521b1329e7433d4b243fdf"></fiddle-embed></div>

<a name='SkTextBlob_MakeFromText'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>&gt; <a href='#SkTextBlob_MakeFromText'>MakeFromText</a>(const void* <a href='undocumented#Text'>text</a>, size_t byteLength, const <a href='SkFont_Reference#SkFont'>SkFont</a>& <a href='SkFont_Reference#Font'>font</a>,
                                      <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> encoding = <a href='undocumented#SkTextEncoding::kUTF8'>SkTextEncoding::kUTF8</a>)
</pre>

Creates <a href='#Text_Blob'>Text_Blob</a> with a single run. <a href='#SkTextBlob_MakeFromText_text'>text</a> meaning depends on <a href='#Text_Encoding'>Text_Encoding</a>;
by default, <a href='#SkTextBlob_MakeFromText_text'>text</a> is encoded as UTF-8.

<a href='#SkTextBlob_MakeFromText_font'>font</a> contains attributes used to define the run <a href='#SkTextBlob_MakeFromText_text'>text</a>: <a href='undocumented#Typeface'>Typeface</a>, <a href='#Font_Size'>Font_Size</a>, <a href='#Font_Scale_X'>Font_Scale_X</a>,
<a href='#Font_Skew_X'>Font_Skew_X</a>, <a href='#Font_Hinting'>Font_Hinting</a>, <a href='#Paint_Anti_Alias'>Paint_Anti_Alias</a>, <a href='#Font_Embolden'>Font_Embolden</a>, <a href='#Font_Force_Hinting'>Font_Force_Hinting</a>,
<a href='#Font_Embedded_Bitmaps'>Font_Embedded_Bitmaps</a>, <a href='#Font_Hinting_Spacing'>Font_Hinting_Spacing</a>, <a href='#Font_Anti_Alias'>Font_Anti_Alias</a>, <a href='#Font_Linear'>Font_Linear</a>,
and <a href='#Font_Subpixel'>Font_Subpixel</a>
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
    <td>one of: <a href='undocumented#SkTextEncoding::kUTF8'>SkTextEncoding::kUTF8</a>, <a href='undocumented#SkTextEncoding::kUTF16'>SkTextEncoding::kUTF16</a>,
<a href='undocumented#SkTextEncoding::kUTF32'>SkTextEncoding::kUTF32</a>, <a href='undocumented#SkTextEncoding::kGlyphID'>SkTextEncoding::kGlyphID</a>
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
static <a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>&gt; <a href='#SkTextBlob_MakeFromString'>MakeFromString</a>(const char* <a href='undocumented#String'>string</a>, const <a href='SkFont_Reference#SkFont'>SkFont</a>& <a href='SkFont_Reference#Font'>font</a>,
                                        <a href='undocumented#SkTextEncoding'>SkTextEncoding</a> encoding = <a href='undocumented#SkTextEncoding::kUTF8'>SkTextEncoding::kUTF8</a>)
</pre>

Creates <a href='#Text_Blob'>Text_Blob</a> with a single run. <a href='#SkTextBlob_MakeFromString_string'>string</a> meaning depends on <a href='#Text_Encoding'>Text_Encoding</a>;
by default, <a href='#SkTextBlob_MakeFromString_string'>string</a> is encoded as UTF-8.

<a href='#SkTextBlob_MakeFromString_font'>font</a> contains <a href='#Font_Metrics'>Font_Metrics</a> used to define the run <a href='undocumented#Text'>text</a>: <a href='undocumented#Typeface'>Typeface</a>, <a href='#Font_Size'>Font_Size</a>, <a href='#Font_Scale_X'>Font_Scale_X</a>,
<a href='#Font_Skew_X'>Font_Skew_X</a>, <a href='#Font_Hinting'>Font_Hinting</a>, <a href='#Paint_Anti_Alias'>Paint_Anti_Alias</a>, <a href='#Font_Embolden'>Font_Embolden</a>, <a href='#Font_Force_Hinting'>Font_Force_Hinting</a>,
<a href='#Font_Embedded_Bitmaps'>Font_Embedded_Bitmaps</a>, <a href='#Font_Hinting_Spacing'>Font_Hinting_Spacing</a>, <a href='#Font_Anti_Alias'>Font_Anti_Alias</a>, <a href='#Font_Linear'>Font_Linear</a>,
and <a href='#Font_Subpixel'>Font_Subpixel</a>
.

### Parameters

<table>  <tr>    <td><a name='SkTextBlob_MakeFromString_string'><code><strong>string</strong></code></a></td>
    <td>character code <a href='SkPoint_Reference#Point'>points</a> or <a href='undocumented#Glyph'>Glyphs</a> drawn</td>
  </tr>
  <tr>    <td><a name='SkTextBlob_MakeFromString_font'><code><strong>font</strong></code></a></td>
    <td><a href='undocumented#Text'>text</a> <a href='undocumented#Size'>size</a>, <a href='undocumented#Typeface'>typeface</a>,  <a href='undocumented#Text'>text scale</a>, and so on, used to draw</td>
  </tr>
  <tr>    <td><a name='SkTextBlob_MakeFromString_encoding'><code><strong>encoding</strong></code></a></td>
    <td>one of: <a href='undocumented#SkTextEncoding::kUTF8'>SkTextEncoding::kUTF8</a>, <a href='undocumented#SkTextEncoding::kUTF16'>SkTextEncoding::kUTF16</a>,
<a href='undocumented#SkTextEncoding::kUTF32'>SkTextEncoding::kUTF32</a>, <a href='undocumented#SkTextEncoding::kGlyphID'>SkTextEncoding::kGlyphID</a>
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
size_t <a href='#SkTextBlob_serialize'>serialize</a>(const <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>& procs, void* memory, size_t memory_size)const
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
<a href='undocumented#sk_sp'>sk_sp</a>&lt;<a href='undocumented#SkData'>SkData</a>&gt; <a href='#SkTextBlob_serialize'>serialize</a>(const <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>& procs)const
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

