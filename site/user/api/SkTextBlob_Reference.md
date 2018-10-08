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

<a href='#SkTextBlob'>SkTextBlob</a> combines multiple text runs into an immutable container

<a name='SkTextBlob_bounds'></a>
## bounds

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Returns conservative bounding box

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
uint32_t <a href='#SkTextBlob_uniqueID'>uniqueID</a>(
</pre>

Returns a non

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
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates <a href='#Text_Blob'>Text Blob</a> with a single run <a href='undocumented#Typeface'>Typeface</a>

### Parameters

<table>  <tr>    <td><a name='SkTextBlob_MakeFromText_text'><code><strong>text</strong></code></a></td>
    <td>character code points or <a href='undocumented#Glyph'>Glyphs</a> drawn</td>
  </tr>
  <tr>    <td><a name='SkTextBlob_MakeFromText_byteLength'><code><strong>byteLength</strong></code></a></td>
    <td>byte length of <a href='#SkTextBlob_MakeFromText_text'>text</a> array</td>
  </tr>
  <tr>    <td><a name='SkTextBlob_MakeFromText_paint'><code><strong>paint</strong></code></a></td>
    <td><a href='#SkTextBlob_MakeFromText_text'>text</a> size</td>
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
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Creates <a href='#Text_Blob'>Text Blob</a> with a single run <a href='undocumented#Typeface'>Typeface</a>

### Parameters

<table>  <tr>    <td><a name='SkTextBlob_MakeFromString_string'><code><strong>string</strong></code></a></td>
    <td>character code points or <a href='undocumented#Glyph'>Glyphs</a> drawn</td>
  </tr>
  <tr>    <td><a name='SkTextBlob_MakeFromString_paint'><code><strong>paint</strong></code></a></td>
    <td>text size</td>
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
size_t <a href='#SkTextBlob_serialize'>serialize</a>(const <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>
</pre>

Writes data to allow later reconstruction of <a href='#Text_Blob'>Text Blob</a>

### Parameters

<table>  <tr>    <td><a name='SkTextBlob_serialize_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial data encoders</td>
  </tr>
  <tr>    <td><a name='SkTextBlob_serialize_memory'><code><strong>memory</strong></code></a></td>
    <td>storage for data</td>
  </tr>
  <tr>    <td><a name='SkTextBlob_serialize_size'><code><strong>size</strong></code></a></td>
    <td><a href='#SkTextBlob_serialize_size'>size</a> of storage</td>
  </tr>
</table>

### Return Value

bytes written

### Example

<div><fiddle-embed name="b1d67f0b0f342eb3e70db01bdefdc21b"></fiddle-embed></div>

### See Also

<a href='#SkTextBlob_Deserialize'>Deserialize</a> <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>

---

<a name='SkTextBlob_serialize_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Returns storage containing <a href='undocumented#Data'>Data</a> describing <a href='#Text_Blob'>Text Blob</a>

### Parameters

<table>  <tr>    <td><a name='SkTextBlob_serialize_2_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial data encoders</td>
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
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Recreates <a href='#Text_Blob'>Text Blob</a> that was serialized into <a href='#SkTextBlob_Deserialize_data'>data</a>

### Parameters

<table>  <tr>    <td><a name='SkTextBlob_Deserialize_data'><code><strong>data</strong></code></a></td>
    <td>pointer for serial <a href='#SkTextBlob_Deserialize_data'>data</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlob_Deserialize_size'><code><strong>size</strong></code></a></td>
    <td><a href='#SkTextBlob_Deserialize_size'>size</a> of <a href='#SkTextBlob_Deserialize_data'>data</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlob_Deserialize_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial <a href='#SkTextBlob_Deserialize_data'>data</a> decoders</td>
  </tr>
</table>

### Return Value

<a href='#Text_Blob'>Text Blob</a> constructed from <a href='#SkTextBlob_Deserialize_data'>data</a> in memory

### Example

<div><fiddle-embed name="9feac5842eaac425a062754097eb7b7b"><div><a href='undocumented#Text'>Text</a></div></fiddle-embed></div>

### See Also

<a href='#SkTextBlob_serialize'>serialize</a><sup><a href='#SkTextBlob_serialize_2'>[2]</a></sup> <a href='undocumented#SkDeserialProcs'>SkDeserialProcs</a>

---

