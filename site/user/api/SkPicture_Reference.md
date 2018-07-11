SkPicture Reference
===

# <a name='Picture'>Picture</a>

# <a name='SkPicture'>Class SkPicture</a>

## <a name='Constructor'>Constructor</a>


SkPicture can be constructed or initialized by these functions, including C++ class constructors.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPicture_MakeFromData'>MakeFromData</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs <a href='#Picture'>Picture</a> from data</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPicture_MakeFromStream'>MakeFromStream</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs <a href='#Picture'>Picture</a> from stream</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPicture_MakePlaceholder'>MakePlaceholder</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs placeholder with unique identifier</td>
  </tr>
</table>

An <a href='#SkPicture'>SkPicture</a> records drawing commands made to a canvas to be played back at a later time.
This base class handles serialization and a few other miscellany.

## Overview

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Class'>Class Declarations</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>embedded class members</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Constructor'>Constructors</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>functions that construct <a href='#SkPicture'>SkPicture</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Member_Function'>Functions</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>global and class member functions</td>
  </tr>
</table>


## <a name='Class'>Class</a>


SkPicture uses C++ classes to declare the public data structures and interfaces.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPicture_AbortCallback'>AbortCallback</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>utility to stop picture playback</td>
  </tr>
</table>

## <a name='Member_Function'>Member Function</a>


SkPicture member functions read and modify the structure properties.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPicture_MakeFromData'>MakeFromData</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs <a href='#Picture'>Picture</a> from data</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPicture_MakeFromStream'>MakeFromStream</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs <a href='#Picture'>Picture</a> from stream</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPicture_MakePlaceholder'>MakePlaceholder</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs placeholder with unique identifier</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPicture_approximateBytesUsed'>approximateBytesUsed</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns approximate size</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPicture_approximateOpCount'>approximateOpCount</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns approximate operation count</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPicture_cullRect'>cullRect</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns bounds used to record <a href='#Picture'>Picture</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPicture_playback'>playback</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>replays drawing commands on canvas</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPicture_serialize'>serialize</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>writes <a href='#Picture'>Picture</a> to data</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPicture_uniqueID'>uniqueID</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns identifier for <a href='#Picture'>Picture</a></td>
  </tr>
</table>

# <a name='SkPicture_AbortCallback'>Class SkPicture::AbortCallback</a>

## <a name='Constructor'>Constructor</a>


SkPicture can be constructed or initialized by these functions, including C++ class constructors.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
</table>

## <a name='Member_Function'>Member_Function</a>


SkPicture member functions read and modify the structure properties.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
</table>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    class <a href='#SkPicture_AbortCallback_empty_constructor'>AbortCallback</a> {
    public:
        <a href='#SkPicture_AbortCallback_empty_constructor'>AbortCallback()</a> {}
        virtual
</pre>

<a href='undocumented#Subclasses'>Subclasses</a> of this can be passed to <a href='#SkPicture_playback'>playback</a>. During the playback
of the picture, this callback will periodically be invoked. If its
<a href='#SkPicture_AbortCallback_abort'>abort</a> returns true, then picture playback will be interrupted.
The resulting drawing is undefined, as there is no guarantee how often the
callback will be invoked. If the abort happens inside some level of nested
calls to save(), restore will automatically be called to return the state
to the same level it was before the playback call was made.

<a name='SkPicture_AbortCallback_empty_constructor'></a>
## AbortCallback

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPicture_AbortCallback_empty_constructor'>AbortCallback</a>()
</pre>

Has no effect.

### Return Value

abstract class cannot be instantiated

### See Also

<a href='#SkPicture_playback'>playback</a>

---

<a name='SkPicture_destructor'></a>
## ~AbortCallback

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual
</pre>

Has no effect.

### See Also

<a href='#SkPicture_playback'>playback</a>

---

<a name='SkPicture_AbortCallback_abort'></a>
## abort

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual bool <a href='#SkPicture_AbortCallback_abort'>abort</a>() = 0
</pre>

Stops <a href='#Picture'>Picture</a> playback when some condition is met. A subclass of
<a href='#SkPicture_AbortCallback_empty_constructor'>AbortCallback</a> provides an override for <a href='#SkPicture_AbortCallback_abort'>abort</a> that can stop <a href='#SkPicture_playback'>playback</a> from
drawing the entire picture.

### Return Value

true to stop playback

### See Also

<a href='#SkPicture_playback'>playback</a>

---

### Example

<div><fiddle-embed name="56ed920dadbf2b2967ac45fb5a9bded6"><div>JustOneDraw allows the black rectangle to draw but stops playback before the
white rectangle appears.
</div></fiddle-embed></div>

<a name='SkPicture_MakeFromStream'></a>
## MakeFromStream

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='#SkPicture'>SkPicture</a>&gt; <a href='#SkPicture_MakeFromStream'>MakeFromStream</a>(<a href='undocumented#SkStream'>SkStream</a>* stream, const <a href='undocumented#SkDeserialProcs'>SkDeserialProcs</a>* procs = nullptr)
</pre>

Recreates a picture that was serialized into a <a href='#SkPicture_MakeFromStream_stream'>stream</a>.

### Parameters

<table>  <tr>    <td><a name='SkPicture_MakeFromStream_stream'><code><strong>stream</strong></code></a></td>
    <td>container for serial data</td>
  </tr>
  <tr>    <td><a name='SkPicture_MakeFromStream_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial data decoders; may be nullptr</td>
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
static <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='#SkPicture'>SkPicture</a>&gt; <a href='#SkPicture_MakeFromData'>MakeFromData</a>(const <a href='undocumented#SkData'>SkData</a>* data, const <a href='undocumented#SkDeserialProcs'>SkDeserialProcs</a>* procs = nullptr)
</pre>

Recreates a picture that was serialized into <a href='#SkPicture_MakeFromData_data'>data</a>.

### Parameters

<table>  <tr>    <td><a name='SkPicture_MakeFromData_data'><code><strong>data</strong></code></a></td>
    <td>container for serial <a href='#SkPicture_MakeFromData_data'>data</a></td>
  </tr>
  <tr>    <td><a name='SkPicture_MakeFromData_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial <a href='#SkPicture_MakeFromData_data'>data</a> decoders; may be nullptr</td>
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
static <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='#SkPicture'>SkPicture</a>&gt; <a href='#SkPicture_MakeFromData'>MakeFromData</a>(const void* data, size_t size,
                                     const <a href='undocumented#SkDeserialProcs'>SkDeserialProcs</a>* procs = nullptr)
</pre>

### Parameters

<table>  <tr>    <td><a name='SkPicture_MakeFromData_2_data'><code><strong>data</strong></code></a></td>
    <td>pointer to serial <a href='#SkPicture_MakeFromData_2_data'>data</a></td>
  </tr>
  <tr>    <td><a name='SkPicture_MakeFromData_2_size'><code><strong>size</strong></code></a></td>
    <td><a href='#SkPicture_MakeFromData_2_size'>size</a> of <a href='#SkPicture_MakeFromData_2_data'>data</a></td>
  </tr>
  <tr>    <td><a name='SkPicture_MakeFromData_2_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial <a href='#SkPicture_MakeFromData_2_data'>data</a> decoders; may be nullptr</td>
  </tr>
Recreates a picture that was serialized into <a href='#SkPicture_MakeFromData_2_data'>data</a>.

</table>

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
virtual void <a href='#SkPicture_playback'>playback</a>(<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* canvas, <a href='#SkPicture_AbortCallback'>AbortCallback</a>* callback = nullptr) const = 0
</pre>

Replays the drawing commands on the specified <a href='#SkPicture_playback_canvas'>canvas</a>. Note that
this has the effect of unfurling this picture into the destination
<a href='#SkPicture_playback_canvas'>canvas</a>. Using the <a href='SkCanvas_Reference#SkCanvas_drawPicture'>SkCanvas::drawPicture</a> entry point gives the destination
<a href='#SkPicture_playback_canvas'>canvas</a> the option of just taking a ref.

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
virtual <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkPicture_cullRect'>cullRect</a>() const = 0
</pre>

Returns cull <a href='SkRect_Reference#Rect'>Rect</a> for this picture.
Ops recorded into this picture that attempt to draw outside the cull might not be drawn.

### Return Value

bounds passed when <a href='#Picture'>Picture</a> was created

### Example

<div><fiddle-embed name="15bb9a9596b40c5e2045f76e8c1dcf8e"><div><a href='#Picture'>Picture</a> recorded bounds are smaller than contents; contents outside recorded
bounds may be drawn, and are drawn in this example.
</div></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas_clipRect'>SkCanvas::clipRect</a><sup><a href='SkCanvas_Reference#SkCanvas_clipRect_2'>[2]</a></sup><sup><a href='SkCanvas_Reference#SkCanvas_clipRect_3'>[3]</a></sup>

---

<a name='SkPicture_uniqueID'></a>
## uniqueID

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkPicture_uniqueID'>uniqueID</a>() const
</pre>

Returns a non-zero value unique among all pictures.

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
<a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkData'>SkData</a>&gt; <a href='#SkPicture_serialize'>serialize</a>(const <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>* procs = nullptr) const
</pre>

Returns storage containing data describing <a href='#Picture'>Picture</a>, using optional custom encoders.

### Parameters

<table>  <tr>    <td><a name='SkPicture_serialize_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial data encoders; may be nullptr</td>
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
void <a href='#SkPicture_serialize'>serialize</a>(<a href='undocumented#SkWStream'>SkWStream</a>* stream, const <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>* procs = nullptr) const
</pre>

Writes picture to <a href='#SkPicture_serialize_2_stream'>stream</a>, using optional custom encoders.

### Parameters

<table>  <tr>    <td><a name='SkPicture_serialize_2_stream'><code><strong>stream</strong></code></a></td>
    <td>writable serial data <a href='#SkPicture_serialize_2_stream'>stream</a></td>
  </tr>
  <tr>    <td><a name='SkPicture_serialize_2_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial data encoders; may be nullptr</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="30b9f1b310187db6aff720a5d67591e2"></fiddle-embed></div>

### See Also

<a href='#SkPicture_MakeFromStream'>MakeFromStream</a> <a href='undocumented#SkWStream'>SkWStream</a> <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>

---

<a name='SkPicture_MakePlaceholder'></a>
## MakePlaceholder

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='#SkPicture'>SkPicture</a>&gt; <a href='#SkPicture_MakePlaceholder'>MakePlaceholder</a>(<a href='SkRect_Reference#SkRect'>SkRect</a> cull)
</pre>

Returns a placeholder <a href='#SkPicture'>SkPicture</a>.
This placeholder does not draw anything itself.  It has a distinct <a href='#SkPicture_uniqueID'>uniqueID</a>
(just like all <a href='#Picture'>Pictures</a>) and will always be visible to <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>.

### Parameters

<table>  <tr>    <td><a name='SkPicture_MakePlaceholder_cull'><code><strong>cull</strong></code></a></td>
    <td>placeholder dimensions</td>
  </tr>
</table>

### Return Value

placeholder with unique identifier

### Example

<div><fiddle-embed name="32f84819483a906ede9c5525801845ef">

#### Example Output

~~~~
id:1 bounds:{10, 40, 80, 110}
id:2 bounds:{10, 40, 80, 110}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPicture_MakeFromStream'>MakeFromStream</a> <a href='#SkPicture_MakeFromData'>MakeFromData</a><sup><a href='#SkPicture_MakeFromData_2'>[2]</a></sup>

---

<a name='SkPicture_approximateOpCount'></a>
## approximateOpCount

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual int <a href='#SkPicture_approximateOpCount'>approximateOpCount</a>() const = 0
</pre>

Returns the approximate number of operations in this picture.  This
number may be greater or less than the number of <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> calls
recorded: some calls may be recorded as more than one operation, or some
calls may be optimized away.

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
virtual size_t <a href='#SkPicture_approximateBytesUsed'>approximateBytesUsed</a>() const = 0
</pre>

Returns the approximate byte size of <a href='#Picture'>Picture</a>. Does not include large objects
referenced <a href='#Picture'>Picture</a>.

### Return Value

approximate size

### Example

<div><fiddle-embed name="ececbda21218bd732394a305dba393a2"></fiddle-embed></div>

### See Also

<a href='#SkPicture_approximateOpCount'>approximateOpCount</a>

---

