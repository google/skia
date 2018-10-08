SkPicture Reference
===

<a name='SkPicture'></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='#SkPicture'>SkPicture</a> : public <a href='undocumented#SkRefCnt'>SkRefCnt</a> {
public:
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkPicture'>SkPicture</a>> <a href='#SkPicture_MakeFromStream'>MakeFromStream</a>(<a href='SkStream_Reference#SkStream'>SkStream</a>* stream,
                                    const <a href='undocumented#SkDeserialProcs'>SkDeserialProcs</a>* procs = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkPicture'>SkPicture</a>> <a href='#SkPicture_MakeFromData'>MakeFromData</a>(const <a href='undocumented#SkData'>SkData</a>* data,
                                         const <a href='undocumented#SkDeserialProcs'>SkDeserialProcs</a>* procs = nullptr);
    static <a href='undocumented#sk_sp'>sk_sp</a><<a href='#SkPicture'>SkPicture</a>> <a href='#SkPicture_MakeFromData_2'>MakeFromData</a>(const void* data, size_t size,
                                         const <a href='undocumented#SkDeserialProcs'>SkDeserialProcs</a>* procs = nullptr);

    class <a href='#SkPicture_AbortCallback'>AbortCallback</a> {
    public:
        <a href='#SkPicture_AbortCallback_AbortCallback'>AbortCallback</a>();
        virtual
</pre>

<a href='#Picture'>Picture</a> records drawing commands made to <a href='SkCanvas_Reference#Canvas'>Canvas</a>
<a name='SkPicture_AbortCallback'></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    class <a href='#SkPicture_AbortCallback_AbortCallback'>AbortCallback</a> {
    public:
        <a href='#SkPicture_AbortCallback_AbortCallback'>AbortCallback()</a> {}
        virtual <a href='#SkPicture_AbortCallback_destructor'>~AbortCallback()</a> {}
        virtual bool <a href='#SkPicture_AbortCallback_abort'>abort</a>() = 0;
    };
</pre>

<a href='#SkPicture_AbortCallback_AbortCallback'>AbortCallback</a> is an abstract class

<a name='SkPicture_AbortCallback_AbortCallback'></a>
## AbortCallback

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPicture_AbortCallback_AbortCallback'>AbortCallback</a>(
</pre>

Has no effect

### Return Value

abstract class cannot be instantiated

### See Also

<a href='#SkPicture_playback'>playback</a>

---

<a name='SkPicture_AbortCallback_destructor'></a>
## ~AbortCallback

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual <a href='#SkPicture_AbortCallback_destructor'>~AbortCallback</a>(
</pre>

Has no effect

### See Also

<a href='#SkPicture_playback'>playback</a>

---

<a name='SkPicture_AbortCallback_abort'></a>
## abort

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual bool <a href='#SkPicture_AbortCallback_abort'>abort</a>(
</pre>

Stops <a href='#Picture'>Picture</a> playback when some condition is met

### Return Value

true to stop playback

### See Also

<a href='#SkPicture_playback'>playback</a>

---

### Example

<div><fiddle-embed name="56ed920dadbf2b2967ac45fb5a9bded6"><div>JustOneDraw allows the black rectangle to draw but stops playback before the
white rectangle appears</div></fiddle-embed></div>

<a name='SkPicture_MakeFromStream'></a>
## MakeFromStream

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Recreates <a href='#Picture'>Picture</a> that was serialized into a <a href='#SkPicture_MakeFromStream_stream'>stream</a>

### Parameters

<table>  <tr>    <td><a name='SkPicture_MakeFromStream_stream'><code><strong>stream</strong></code></a></td>
    <td>container for serial data</td>
  </tr>
  <tr>    <td><a name='SkPicture_MakeFromStream_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial data decoders</td>
  </tr>
</table>

### Return Value

<a href='#Picture'>Picture</a> constructed from <a href='#SkPicture_MakeFromStream_stream'>stream</a> data

### Example

<div><fiddle-embed name="404fb42560a289c2004cad1caf3b96de"></fiddle-embed></div>

### See Also

<a href='#SkPicture_MakeFromData'>MakeFromData</a><sup><a href='#SkPicture_MakeFromData_2'>[2]</a></sup> <a href='undocumented#SkPictureRecorder'>SkPictureRecorder</a>

---

<a name='SkPicture_MakeFromData'></a>
## MakeFromData

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Recreates <a href='#Picture'>Picture</a> that was serialized into <a href='#SkPicture_MakeFromData_data'>data</a>

### Parameters

<table>  <tr>    <td><a name='SkPicture_MakeFromData_data'><code><strong>data</strong></code></a></td>
    <td>container for serial <a href='#SkPicture_MakeFromData_data'>data</a></td>
  </tr>
  <tr>    <td><a name='SkPicture_MakeFromData_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial <a href='#SkPicture_MakeFromData_data'>data</a> decoders</td>
  </tr>
</table>

### Return Value

<a href='#Picture'>Picture</a> constructed from <a href='#SkPicture_MakeFromData_data'>data</a>

### Example

<div><fiddle-embed name="58b44bf47d8816782066618700afdecb"></fiddle-embed></div>

### See Also

<a href='#SkPicture_MakeFromStream'>MakeFromStream</a> <a href='undocumented#SkPictureRecorder'>SkPictureRecorder</a>

---

<a name='SkPicture_MakeFromData_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

### Parameters

<table>  <tr>    <td><a name='SkPicture_MakeFromData_2_data'><code><strong>data</strong></code></a></td>
    <td>pointer to serial <a href='#SkPicture_MakeFromData_2_data'>data</a></td>
  </tr>
  <tr>    <td><a name='SkPicture_MakeFromData_2_size'><code><strong>size</strong></code></a></td>
    <td><a href='#SkPicture_MakeFromData_2_size'>size</a> of <a href='#SkPicture_MakeFromData_2_data'>data</a></td>
  </tr>
  <tr>    <td><a name='SkPicture_MakeFromData_2_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial <a href='#SkPicture_MakeFromData_2_data'>data</a> decoders</td>
  </tr>
Recreates <a href='#Picture'>Picture</a> that was serialized into <a href='#SkPicture_MakeFromData_2_data'>data</a></table>

### Return Value

<a href='#Picture'>Picture</a> constructed from <a href='#SkPicture_MakeFromData_2_data'>data</a>

### Example

<div><fiddle-embed name="30b9f1b310187db6aff720a5d67591e2"></fiddle-embed></div>

### See Also

<a href='#SkPicture_MakeFromStream'>MakeFromStream</a> <a href='undocumented#SkPictureRecorder'>SkPictureRecorder</a>

---

<a name='SkPicture_playback'></a>
## playback

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual void <a href='#SkPicture_playback'>playback</a>(<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>
</pre>

Replays the drawing commands on the specified <a href='#SkPicture_playback_canvas'>canvas</a>

### Parameters

<table>  <tr>    <td><a name='SkPicture_playback_canvas'><code><strong>canvas</strong></code></a></td>
    <td>receiver of drawing commands</td>
  </tr>
  <tr>    <td><a name='SkPicture_playback_callback'><code><strong>callback</strong></code></a></td>
    <td>allows interruption of playback</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6b0ffb03ba05f526b345dc65a1c73fe4"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas_drawPicture'>SkCanvas::drawPicture</a><sup><a href='SkCanvas_Reference#SkCanvas_drawPicture_2'>[2]</a></sup><sup><a href='SkCanvas_Reference#SkCanvas_drawPicture_3'>[3]</a></sup><sup><a href='SkCanvas_Reference#SkCanvas_drawPicture_4'>[4]</a></sup>

---

<a name='SkPicture_cullRect'></a>
## cullRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkPicture_cullRect'>cullRect</a>(
</pre>

Returns cull <a href='SkRect_Reference#Rect'>Rect</a> for this picture

### Return Value

bounds passed when <a href='#Picture'>Picture</a> was created

### Example

<div><fiddle-embed name="15bb9a9596b40c5e2045f76e8c1dcf8e"><div><a href='#Picture'>Picture</a> recorded bounds are smaller than contents</div></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas_clipRect'>SkCanvas::clipRect</a><sup><a href='SkCanvas_Reference#SkCanvas_clipRect_2'>[2]</a></sup><sup><a href='SkCanvas_Reference#SkCanvas_clipRect_3'>[3]</a></sup>

---

<a name='SkPicture_uniqueID'></a>
## uniqueID

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkPicture_uniqueID'>uniqueID</a>(
</pre>

Returns a non

### Return Value

identifier for <a href='#Picture'>Picture</a>

### Example

<div><fiddle-embed name="8e4257245c988c600410fe4fd7293f07">

#### Example Output

~~~~
empty picture id = 1
placeholder id = 2
~~~~

</fiddle-embed></div>

### See Also

<a href='undocumented#SkRefCnt'>SkRefCnt</a>

---

<a name='SkPicture_serialize'></a>
## serialize

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Returns storage containing <a href='undocumented#Data'>Data</a> describing <a href='#Picture'>Picture</a>

### Parameters

<table>  <tr>    <td><a name='SkPicture_serialize_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial data encoders</td>
  </tr>
</table>

### Return Value

storage containing serialized <a href='#Picture'>Picture</a>

### Example

<div><fiddle-embed name="dacdebe1355c884ebd3c2ea038cc7a20"></fiddle-embed></div>

### See Also

<a href='#SkPicture_MakeFromData'>MakeFromData</a><sup><a href='#SkPicture_MakeFromData_2'>[2]</a></sup> <a href='undocumented#SkData'>SkData</a> <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>

---

<a name='SkPicture_serialize_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPicture_serialize'>serialize</a>(<a href='SkWStream_Reference#SkWStream'>SkWStream</a>
</pre>

Writes picture to <a href='#SkPicture_serialize_2_stream'>stream</a>

### Parameters

<table>  <tr>    <td><a name='SkPicture_serialize_2_stream'><code><strong>stream</strong></code></a></td>
    <td>writable serial data <a href='#SkPicture_serialize_2_stream'>stream</a></td>
  </tr>
  <tr>    <td><a name='SkPicture_serialize_2_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial data encoders</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="30b9f1b310187db6aff720a5d67591e2"></fiddle-embed></div>

### See Also

<a href='#SkPicture_MakeFromStream'>MakeFromStream</a> <a href='SkWStream_Reference#SkWStream'>SkWStream</a> <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>

---

<a name='SkPicture_MakePlaceholder'></a>
## MakePlaceholder

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>
</pre>

Returns a placeholder <a href='#SkPicture'>SkPicture</a>

### Parameters

<table>  <tr>    <td><a name='SkPicture_MakePlaceholder_cull'><code><strong>cull</strong></code></a></td>
    <td>placeholder dimensions</td>
  </tr>
</table>

### Return Value

placeholder with unique identifier

### Example

<div><fiddle-embed name="0d2cbf82f490ffb180e0b4531afa232c"></fiddle-embed></div>

### See Also

<a href='#SkPicture_MakeFromStream'>MakeFromStream</a> <a href='#SkPicture_MakeFromData'>MakeFromData</a><sup><a href='#SkPicture_MakeFromData_2'>[2]</a></sup> <a href='#SkPicture_uniqueID'>uniqueID</a>

---

<a name='SkPicture_approximateOpCount'></a>
## approximateOpCount

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual int <a href='#SkPicture_approximateOpCount'>approximateOpCount</a>(
</pre>

Returns the approximate number of operations in <a href='#Picture'>Picture</a>

### Return Value

approximate operation count

### Example

<div><fiddle-embed name="4b3d879118ef770d1f11a23c6493b2c4"></fiddle-embed></div>

### See Also

<a href='#SkPicture_approximateBytesUsed'>approximateBytesUsed</a>

---

<a name='SkPicture_approximateBytesUsed'></a>
## approximateBytesUsed

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual size_t <a href='#SkPicture_approximateBytesUsed'>approximateBytesUsed</a>(
</pre>

Returns the approximate byte size of <a href='#Picture'>Picture</a>

### Return Value

approximate size

### Example

<div><fiddle-embed name="ececbda21218bd732394a305dba393a2"></fiddle-embed></div>

### See Also

<a href='#SkPicture_approximateOpCount'>approximateOpCount</a>

---

