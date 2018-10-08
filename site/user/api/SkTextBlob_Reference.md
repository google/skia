SkTextBlob Reference
===

<a name='SkTextBlob'></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='#SkTextBlob'>SkTextBlob</a> final : public SkNVRefCnt<<a href='#SkTextBlob'>SkTextBlob</a>> {
public:
    const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkTextBlob_bounds'>bounds</a>() const;
    uint32_t <a href='#SkTextBlob_uniqueID'>uniqueID</a>() const;
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkTextBlob'>SkTextBlob</a>> <a href='#SkTextBlob_MakeFromText'>MakeFromText</a>( const void* text, size_t byteLength, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkTextBlob'>SkTextBlob</a>> <a href='#SkTextBlob_MakeFromString'>MakeFromString</a>(const char* string, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint);
    size_t <a href='#SkTextBlob_serialize'>serialize</a>(const <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>& procs, void* memory, size_t memory_size) const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkData'>SkData</a>> <a href='#SkTextBlob_serialize_2'>serialize</a>(const <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>& procs) const;
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkTextBlob'>SkTextBlob</a>> <a href='#SkTextBlob_Deserialize'>Deserialize</a>(const void* data, size_t size,
                                         const <a href='undocumented#SkDeserialProcs'>SkDeserialProcs</a>& procs);
};
</pre>

<a href='#SkTextBlob'>SkTextBlob</a> combines multiple text runs into an immutable container. Each text
run consists of <a href='undocumented#Glyph'>Glyphs</a>, <a href='SkPaint_Reference#Paint'>Paint</a>, and position. Only parts of <a href='SkPaint_Reference#Paint'>Paint</a> related to
fonts and text rendering are used by run.

<a name='SkTextBlob_bounds'></a>
## bounds

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkTextBlob_bounds'>bounds</a>() const
</pre>

Returns conservative bounding box. Uses <a href='SkPaint_Reference#Paint'>Paint</a> associated with each glyph to
determine glyph bounds, and unions all bounds. Returned bounds may be
larger than the bounds of all <a href='undocumented#Glyph'>Glyphs</a> in runs.

### Return Value

conservative bounding box

### Example

<div><fiddle-embed name="52ba6c8a9483df8c373fdb49b32a5e19"></fiddle-embed></div>

### See Also

<a href='SkPath_Reference#SkPath_getBounds'>SkPath::getBounds</a>

---

<a name='SkTextBlob_uniqueID'></a>
## uniqueID

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkTextBlob_uniqueID'>uniqueID</a>() const
</pre>

Returns a non-zero value unique among all text blobs.

### Return Value

identifier for <a href='#Text_Blob'>Text Blob</a>

### Example

<div><fiddle-embed name="5a29bcc0076950339c955149c34e1d46"></fiddle-embed></div>

### See Also

<a href='undocumented#SkRefCnt'>SkRefCnt</a>

---

<a name='SkTextBlob_MakeFromText'></a>
## MakeFromText

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='#SkTextBlob'>SkTextBlob</a>&gt; <a href='#SkTextBlob_MakeFromText'>MakeFromText</a>(
            const void* text, size_t byteLength,
                                      const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint)
</pre>

Creates <a href='#Text_Blob'>Text Blob</a> with a single run. <a href='#SkTextBlob_MakeFromText_text'>text</a> meaning depends on <a href='SkPaint_Reference#Text_Encoding'>Paint Text Encoding</a>;
by default, <a href='#SkTextBlob_MakeFromText_text'>text</a> is encoded as UTF-8.

<a href='#SkTextBlob_MakeFromText_paint'>paint</a> contains attributes used to define the run <a href='#SkTextBlob_MakeFromText_text'>text</a>: <a href='undocumented#Typeface'>Typeface</a>, <a href='SkPaint_Reference#Text_Size'>Paint Text Size</a>, <a href='SkPaint_Reference#Text_Scale_X'>Paint Text Scale X</a>,
<a href='SkPaint_Reference#Text_Skew_X'>Paint Text Skew X</a>, <a href='SkPaint_Reference#Text_Align'>Paint Text Align</a>, <a href='SkPaint_Reference#Hinting'>Paint Hinting</a>, <a href='SkPaint_Reference#Anti_Alias'>Anti Alias</a>, <a href='SkPaint_Reference#Fake_Bold'>Paint Fake Bold</a>,
<a href='SkPaint_Reference#Font_Embedded_Bitmaps'>Font Embedded Bitmaps</a>, <a href='SkPaint_Reference#Full_Hinting_Spacing'>Full Hinting Spacing</a>, <a href='SkPaint_Reference#LCD_Text'>LCD Text</a>, <a href='SkPaint_Reference#Linear_Text'>Linear Text</a>,
and <a href='SkPaint_Reference#Subpixel_Text'>Subpixel Text</a>.

### Parameters

<table>  <tr>    <td><a name='SkTextBlob_MakeFromText_text'><code><strong>text</strong></code></a></td>
    <td>character code points or <a href='undocumented#Glyph'>Glyphs</a> drawn</td>
  </tr>
  <tr>    <td><a name='SkTextBlob_MakeFromText_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>byte length of <a href='#SkTextBlob_MakeFromText_text'>text</a> array</td>
  </tr>
  <tr>    <td><a name='SkTextBlob_MakeFromText_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='#SkTextBlob_MakeFromText_text'>text</a> size, typeface, <a href='#SkTextBlob_MakeFromText_text'>text</a> scale, and so on, used to draw</td>
  </tr>
</table>

### Return Value

<a href='#Text_Blob'>Text Blob</a> constructed from one run

### Example

<div><fiddle-embed name="74686684967a310dc06fe2915b0a4798"></fiddle-embed></div>

### See Also

<a href='#SkTextBlob_MakeFromString'>MakeFromString</a> TextBlobBuilder

---

<a name='SkTextBlob_MakeFromString'></a>
## MakeFromString

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='#SkTextBlob'>SkTextBlob</a>&gt; <a href='#SkTextBlob_MakeFromString'>MakeFromString</a>(const char* string, const <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& paint)
</pre>

Creates <a href='#Text_Blob'>Text Blob</a> with a single run. <a href='#SkTextBlob_MakeFromString_string'>string</a> meaning depends on <a href='SkPaint_Reference#Text_Encoding'>Paint Text Encoding</a>;
by default, <a href='#SkTextBlob_MakeFromString_string'>string</a> is encoded as UTF-8.

<a href='#SkTextBlob_MakeFromString_paint'>paint</a> contains <a href='SkPaint_Reference#Font_Metrics'>Paint Font Metrics</a> used to define the run text: <a href='undocumented#Typeface'>Typeface</a>, <a href='SkPaint_Reference#Text_Size'>Paint Text Size</a>, <a href='SkPaint_Reference#Text_Scale_X'>Paint Text Scale X</a>,
<a href='SkPaint_Reference#Text_Skew_X'>Paint Text Skew X</a>, <a href='SkPaint_Reference#Text_Align'>Paint Text Align</a>, <a href='SkPaint_Reference#Hinting'>Paint Hinting</a>, <a href='SkPaint_Reference#Anti_Alias'>Anti Alias</a>, <a href='SkPaint_Reference#Fake_Bold'>Paint Fake Bold</a>,
<a href='SkPaint_Reference#Font_Embedded_Bitmaps'>Font Embedded Bitmaps</a>, <a href='SkPaint_Reference#Full_Hinting_Spacing'>Full Hinting Spacing</a>, <a href='SkPaint_Reference#LCD_Text'>LCD Text</a>, <a href='SkPaint_Reference#Linear_Text'>Linear Text</a>,
and <a href='SkPaint_Reference#Subpixel_Text'>Subpixel Text</a>.

### Parameters

<table>  <tr>    <td><a name='SkTextBlob_MakeFromString_string'><code><strong>string</strong></code></a></td>
    <td>character code points or <a href='undocumented#Glyph'>Glyphs</a> drawn</td>
  </tr>
  <tr>    <td><a name='SkTextBlob_MakeFromString_paint'><code><strong>paint</strong></code></a></td>
    <td>text size, typeface, text scale, and so on, used to draw</td>
  </tr>
</table>

### Return Value

<a href='#Text_Blob'>Text Blob</a> constructed from one run

### Example

<div><fiddle-embed name="705b26bb5e361369d897eeb511b6a184"></fiddle-embed></div>

### See Also

<a href='#SkTextBlob_MakeFromText'>MakeFromText</a> TextBlobBuilder

---

<a name='SkTextBlob_serialize'></a>
## serialize

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkTextBlob_serialize'>serialize</a>(const <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>& procs, void* memory, size_t memory_size) const
</pre>

Writes data to allow later reconstruction of <a href='#Text_Blob'>Text Blob</a>. <a href='#SkTextBlob_serialize_memory'>memory</a> points to storage
to receive the encoded data, and memory_size describes the <a href='#SkTextBlob_serialize_size'>size</a> of storage.
Returns bytes used if provided storage is large enough to hold all data;
otherwise, returns zero.

<a href='#SkTextBlob_serialize_procs'>procs</a>.fTypefaceProc permits supplying a custom function to encode <a href='undocumented#Typeface'>Typeface</a>.
If <a href='#SkTextBlob_serialize_procs'>procs</a>.fTypefaceProc is nullptr, default encoding is used. <a href='#SkTextBlob_serialize_procs'>procs</a>.fTypefaceCtx
may be used to provide user context to <a href='#SkTextBlob_serialize_procs'>procs</a>.fTypefaceProc; <a href='#SkTextBlob_serialize_procs'>procs</a>.fTypefaceProc
is called with a pointer to <a href='undocumented#Typeface'>Typeface</a> and user context.

### Parameters

<table>  <tr>    <td><a name='SkTextBlob_serialize_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial data encoders; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkTextBlob_serialize_memory'><code><strong>memory</strong></code></a></td>
    <td>storage for data</td>
  </tr>
  <tr>    <td><a name='SkTextBlob_serialize_size'><code><strong>size</strong></code></a></td>
    <td><a href='#SkTextBlob_serialize_size'>size</a> of storage</td>
  </tr>
</table>

### Return Value

bytes written, or zero if required storage is larger than memory_size

### Example

<div><fiddle-embed name="b1d67f0b0f342eb3e70db01bdefdc21b"></fiddle-embed></div>

### See Also

<a href='#SkTextBlob_Deserialize'>Deserialize</a> <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>

---

<a name='SkTextBlob_serialize_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkData'>SkData</a>&gt; <a href='#SkTextBlob_serialize'>serialize</a>(const <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>& procs) const
</pre>

Returns storage containing <a href='undocumented#Data'>Data</a> describing <a href='#Text_Blob'>Text Blob</a>, using optional custom
encoders.

<a href='#SkTextBlob_serialize_2_procs'>procs</a>.fTypefaceProc permits supplying a custom function to encode <a href='undocumented#Typeface'>Typeface</a>.
If <a href='#SkTextBlob_serialize_2_procs'>procs</a>.fTypefaceProc is nullptr, default encoding is used. <a href='#SkTextBlob_serialize_2_procs'>procs</a>.fTypefaceCtx
may be used to provide user context to <a href='#SkTextBlob_serialize_2_procs'>procs</a>.fTypefaceProc; <a href='#SkTextBlob_serialize_2_procs'>procs</a>.fTypefaceProc
is called with a pointer to <a href='undocumented#Typeface'>Typeface</a> and user context.

### Parameters

<table>  <tr>    <td><a name='SkTextBlob_serialize_2_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial data encoders; may be nullptr</td>
  </tr>
</table>

### Return Value

storage containing serialized <a href='#Text_Blob'>Text Blob</a>

### Example

<div><fiddle-embed name="9447d4cf5d560b40b7535e403d854c20"></fiddle-embed></div>

### See Also

<a href='#SkTextBlob_Deserialize'>Deserialize</a> <a href='undocumented#SkData'>SkData</a> <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>

---

<a name='SkTextBlob_Deserialize'></a>
## Deserialize

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='#SkTextBlob'>SkTextBlob</a>&gt; <a href='#SkTextBlob_Deserialize'>Deserialize</a>(const void* data, size_t size, const <a href='undocumented#SkDeserialProcs'>SkDeserialProcs</a>& procs)
</pre>

Recreates <a href='#Text_Blob'>Text Blob</a> that was serialized into <a href='#SkTextBlob_Deserialize_data'>data</a>. Returns constructed <a href='#Text_Blob'>Text Blob</a>
if successful; otherwise, returns nullptr. Fails if <a href='#SkTextBlob_Deserialize_size'>size</a> is smaller than
required <a href='#SkTextBlob_Deserialize_data'>data</a> length, or if <a href='#SkTextBlob_Deserialize_data'>data</a> does not permit constructing valid <a href='#Text_Blob'>Text Blob</a>.

<a href='#SkTextBlob_Deserialize_procs'>procs</a>.fTypefaceProc permits supplying a custom function to decode <a href='undocumented#Typeface'>Typeface</a>.
If <a href='#SkTextBlob_Deserialize_procs'>procs</a>.fTypefaceProc is nullptr, default decoding is used. <a href='#SkTextBlob_Deserialize_procs'>procs</a>.fTypefaceCtx
may be used to provide user context to <a href='#SkTextBlob_Deserialize_procs'>procs</a>.fTypefaceProc; <a href='#SkTextBlob_Deserialize_procs'>procs</a>.fTypefaceProc
is called with a pointer to <a href='undocumented#Typeface'>Typeface</a> <a href='#SkTextBlob_Deserialize_data'>data</a>, <a href='#SkTextBlob_Deserialize_data'>data</a> byte length, and user context.

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

<a href='#Text_Blob'>Text Blob</a> constructed from <a href='#SkTextBlob_Deserialize_data'>data</a> in memory

### Example

<div><fiddle-embed name="9feac5842eaac425a062754097eb7b7b"><div><a href='undocumented#Text'>Text</a> "" replaces "World!", but does not update its metrics.
When drawn, "" uses the spacing computed for "World!".
</div></fiddle-embed></div>

### See Also

<a href='#SkTextBlob_serialize'>serialize</a><sup><a href='#SkTextBlob_serialize_2'>[2]</a></sup> <a href='undocumented#SkDeserialProcs'>SkDeserialProcs</a>

---

