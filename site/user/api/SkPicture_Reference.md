SkPicture Reference
===


<a name='SkPicture'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkPicture_Reference#SkPicture'>SkPicture</a> : <a href='SkPicture_Reference#SkPicture'>public</a> <a href='undocumented#SkRefCnt'>SkRefCnt</a> {
<a href='undocumented#SkRefCnt'>public</a>:
    <a href='undocumented#SkRefCnt'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkPicture_Reference#SkPicture'>SkPicture</a>> <a href='#SkPicture_MakeFromStream'>MakeFromStream</a>(<a href='SkStream_Reference#SkStream'>SkStream</a>* <a href='SkStream_Reference#Stream'>stream</a>,
                                    <a href='SkStream_Reference#Stream'>const</a> <a href='undocumented#SkDeserialProcs'>SkDeserialProcs</a>* <a href='undocumented#SkDeserialProcs'>procs</a> = <a href='undocumented#SkDeserialProcs'>nullptr</a>);
    <a href='undocumented#SkDeserialProcs'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkPicture_Reference#SkPicture'>SkPicture</a>> <a href='#SkPicture_MakeFromData'>MakeFromData</a>(<a href='#SkPicture_MakeFromData'>const</a> <a href='undocumented#SkData'>SkData</a>* <a href='undocumented#Data'>data</a>,
                                         <a href='undocumented#Data'>const</a> <a href='undocumented#SkDeserialProcs'>SkDeserialProcs</a>* <a href='undocumented#SkDeserialProcs'>procs</a> = <a href='undocumented#SkDeserialProcs'>nullptr</a>);
    <a href='undocumented#SkDeserialProcs'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkPicture_Reference#SkPicture'>SkPicture</a>> <a href='#SkPicture_MakeFromData'>MakeFromData</a>(<a href='#SkPicture_MakeFromData'>const</a> <a href='#SkPicture_MakeFromData'>void</a>* <a href='undocumented#Data'>data</a>, <a href='undocumented#Data'>size_t</a> <a href='undocumented#Size'>size</a>,
                                         <a href='undocumented#Size'>const</a> <a href='undocumented#SkDeserialProcs'>SkDeserialProcs</a>* <a href='undocumented#SkDeserialProcs'>procs</a> = <a href='undocumented#SkDeserialProcs'>nullptr</a>);

    <a href='undocumented#SkDeserialProcs'>class</a> <a href='#SkPicture_AbortCallback'>AbortCallback</a> {
    <a href='#SkPicture_AbortCallback'>public</a>:
        <a href='#SkPicture_AbortCallback'>AbortCallback</a>();
        <a href='#SkPicture_AbortCallback'>virtual</a> ~<a href='#SkPicture_AbortCallback'>AbortCallback</a>();
        <a href='#SkPicture_AbortCallback'>virtual</a> <a href='#SkPicture_AbortCallback'>bool</a> <a href='#SkPicture_AbortCallback'>abort()</a> = 0;
    };

    <a href='#SkPicture_AbortCallback'>virtual</a> <a href='#SkPicture_AbortCallback'>void</a> <a href='#SkPicture_AbortCallback'>playback</a>(<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* <a href='SkCanvas_Reference#Canvas'>canvas</a>, <a href='#SkPicture_AbortCallback'>AbortCallback</a>* <a href='#SkPicture_AbortCallback'>callback</a> = <a href='#SkPicture_AbortCallback'>nullptr</a>) <a href='#SkPicture_AbortCallback'>const</a> = 0;
    <a href='#SkPicture_AbortCallback'>virtual</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkPicture_cullRect'>cullRect</a>() <a href='#SkPicture_cullRect'>const</a> = 0;
    <a href='#SkPicture_cullRect'>uint32_t</a> <a href='#SkPicture_uniqueID'>uniqueID</a>() <a href='#SkPicture_uniqueID'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkData'>SkData</a>> <a href='#SkPicture_serialize'>serialize</a>(<a href='#SkPicture_serialize'>const</a> <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>* <a href='undocumented#SkSerialProcs'>procs</a> = <a href='undocumented#SkSerialProcs'>nullptr</a>) <a href='undocumented#SkSerialProcs'>const</a>;
    <a href='undocumented#SkSerialProcs'>void</a> <a href='#SkPicture_serialize'>serialize</a>(<a href='SkWStream_Reference#SkWStream'>SkWStream</a>* <a href='SkStream_Reference#Stream'>stream</a>, <a href='SkStream_Reference#Stream'>const</a> <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>* <a href='undocumented#SkSerialProcs'>procs</a> = <a href='undocumented#SkSerialProcs'>nullptr</a>) <a href='undocumented#SkSerialProcs'>const</a>;
    <a href='undocumented#SkSerialProcs'>static</a> <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkPicture_Reference#SkPicture'>SkPicture</a>> <a href='#SkPicture_MakePlaceholder'>MakePlaceholder</a>(<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>cull</a>);
    <a href='SkRect_Reference#SkRect'>virtual</a> <a href='SkRect_Reference#SkRect'>int</a> <a href='#SkPicture_approximateOpCount'>approximateOpCount</a>() <a href='#SkPicture_approximateOpCount'>const</a> = 0;
    <a href='#SkPicture_approximateOpCount'>virtual</a> <a href='#SkPicture_approximateOpCount'>size_t</a> <a href='#SkPicture_approximateBytesUsed'>approximateBytesUsed</a>() <a href='#SkPicture_approximateBytesUsed'>const</a> = 0;
};
</pre>

<a href='SkPicture_Reference#Picture'>Picture</a> <a href='SkPicture_Reference#Picture'>records</a> <a href='SkPicture_Reference#Picture'>drawing</a> <a href='SkPicture_Reference#Picture'>commands</a> <a href='SkPicture_Reference#Picture'>made</a> <a href='SkPicture_Reference#Picture'>to</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a>. <a href='SkCanvas_Reference#Canvas'>The</a> <a href='SkCanvas_Reference#Canvas'>command</a> <a href='SkStream_Reference#Stream'>stream</a> <a href='SkStream_Reference#Stream'>may</a> <a href='SkStream_Reference#Stream'>be</a>
<a href='SkStream_Reference#Stream'>played</a> <a href='SkStream_Reference#Stream'>in</a> <a href='SkStream_Reference#Stream'>whole</a> <a href='SkStream_Reference#Stream'>or</a> <a href='SkStream_Reference#Stream'>in</a> <a href='SkStream_Reference#Stream'>part</a> <a href='SkStream_Reference#Stream'>at</a> <a href='SkStream_Reference#Stream'>a</a> <a href='SkStream_Reference#Stream'>later</a> <a href='SkStream_Reference#Stream'>time</a>.

<a href='SkPicture_Reference#Picture'>Picture</a> <a href='SkPicture_Reference#Picture'>is</a> <a href='SkPicture_Reference#Picture'>an</a> <a href='SkPicture_Reference#Picture'>abstract</a> <a href='SkPicture_Reference#Picture'>class</a>. <a href='SkPicture_Reference#Picture'>Picture</a> <a href='SkPicture_Reference#Picture'>may</a> <a href='SkPicture_Reference#Picture'>be</a> <a href='SkPicture_Reference#Picture'>generated</a> <a href='SkPicture_Reference#Picture'>by</a> <a href='#Picture_Recorder'>Picture_Recorder</a>
<a href='#Picture_Recorder'>or</a> <a href='undocumented#Drawable'>Drawable</a>, <a href='undocumented#Drawable'>or</a> <a href='undocumented#Drawable'>from</a> <a href='SkPicture_Reference#Picture'>Picture</a> <a href='SkPicture_Reference#Picture'>previously</a> <a href='SkPicture_Reference#Picture'>saved</a> <a href='SkPicture_Reference#Picture'>to</a> <a href='undocumented#Data'>Data</a> <a href='undocumented#Data'>or</a> <a href='SkStream_Reference#Stream'>Stream</a>.

<a href='SkPicture_Reference#Picture'>Picture</a> <a href='SkPicture_Reference#Picture'>may</a> <a href='SkPicture_Reference#Picture'>contain</a> <a href='SkPicture_Reference#Picture'>any</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>drawing</a> <a href='SkCanvas_Reference#Canvas'>command</a>, <a href='SkCanvas_Reference#Canvas'>as</a> <a href='SkCanvas_Reference#Canvas'>well</a> <a href='SkCanvas_Reference#Canvas'>as</a> <a href='SkCanvas_Reference#Canvas'>one</a> <a href='SkCanvas_Reference#Canvas'>or</a> <a href='SkCanvas_Reference#Canvas'>more</a>
<a href='#Canvas_Matrix'>Canvas_Matrix</a> <a href='#Canvas_Matrix'>or</a> <a href='#Canvas_Clip'>Canvas_Clip</a>. <a href='SkPicture_Reference#Picture'>Picture</a> <a href='SkPicture_Reference#Picture'>has</a> <a href='SkPicture_Reference#Picture'>a</a> <a href='SkPicture_Reference#Picture'>cull</a> <a href='SkRect_Reference#Rect'>Rect</a>, <a href='SkRect_Reference#Rect'>which</a> <a href='SkRect_Reference#Rect'>is</a> <a href='SkRect_Reference#Rect'>used</a> <a href='SkRect_Reference#Rect'>as</a>
<a href='SkRect_Reference#Rect'>a</a> <a href='SkRect_Reference#Rect'>bounding</a> <a href='SkRect_Reference#Rect'>box</a> <a href='SkRect_Reference#Rect'>hint</a>. <a href='SkRect_Reference#Rect'>To</a> <a href='SkRect_Reference#Rect'>limit</a> <a href='SkPicture_Reference#Picture'>Picture</a> <a href='SkPicture_Reference#Picture'>bounds</a>, <a href='SkPicture_Reference#Picture'>use</a> <a href='#Canvas_Clip'>Canvas_Clip</a> <a href='#Canvas_Clip'>when</a>
<a href='#Canvas_Clip'>recording</a> <a href='#Canvas_Clip'>or</a> <a href='#Canvas_Clip'>drawing</a> <a href='SkPicture_Reference#Picture'>Picture</a>.

<a name='SkPicture_AbortCallback'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    class <a href='#SkPicture_AbortCallback'>AbortCallback</a> {
    <a href='#SkPicture_AbortCallback'>public</a>:
        <a href='#SkPicture_AbortCallback_AbortCallback'>AbortCallback()</a> {}
        <a href='#SkPicture_AbortCallback'>virtual</a> ~<a href='#SkPicture_AbortCallback_AbortCallback'>AbortCallback()</a> {}
        <a href='#SkPicture_AbortCallback'>virtual</a> <a href='#SkPicture_AbortCallback'>bool</a> <a href='#SkPicture_AbortCallback_abort'>abort()</a> = 0;
    };
</pre>

<a href='#SkPicture_AbortCallback'>AbortCallback</a> <a href='#SkPicture_AbortCallback'>is</a> <a href='#SkPicture_AbortCallback'>an</a> <a href='#SkPicture_AbortCallback'>abstract</a> <a href='#SkPicture_AbortCallback'>class</a>. <a href='#SkPicture_AbortCallback'>An</a> <a href='#SkPicture_AbortCallback'>implementation</a> <a href='#SkPicture_AbortCallback'>of</a> <a href='#SkPicture_AbortCallback'>AbortCallback</a> <a href='#SkPicture_AbortCallback'>may</a>
<a href='#SkPicture_AbortCallback'>passed</a> <a href='#SkPicture_AbortCallback'>as</a> <a href='#SkPicture_AbortCallback'>a</a> <a href='#SkPicture_AbortCallback'>parameter</a> <a href='#SkPicture_AbortCallback'>to</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a>::<a href='#SkPicture_playback'>playback</a>, <a href='#SkPicture_playback'>to</a> <a href='#SkPicture_playback'>stop</a> <a href='#SkPicture_playback'>it</a> <a href='#SkPicture_playback'>before</a> <a href='#SkPicture_playback'>all</a> <a href='#SkPicture_playback'>drawing</a>
<a href='#SkPicture_playback'>commands</a> <a href='#SkPicture_playback'>have</a> <a href='#SkPicture_playback'>been</a> <a href='#SkPicture_playback'>processed</a>.

<a href='#SkPicture_playback'>If</a> <a href='#SkPicture_AbortCallback'>AbortCallback</a>::<a href='#SkPicture_AbortCallback_abort'>abort</a> <a href='#SkPicture_AbortCallback_abort'>returns</a> <a href='#SkPicture_AbortCallback_abort'>true</a>, <a href='SkPicture_Reference#SkPicture'>SkPicture</a>::<a href='#SkPicture_playback'>playback</a> <a href='#SkPicture_playback'>is</a> <a href='#SkPicture_playback'>interrupted</a>.

<a name='SkPicture_AbortCallback_AbortCallback'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPicture_AbortCallback_AbortCallback'>AbortCallback()</a>
</pre>

Has no effect.

### Return Value

abstract class cannot be instantiated

### See Also

<a href='#SkPicture_playback'>playback</a>

<a name='SkPicture_AbortCallback_destructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual ~<a href='#SkPicture_AbortCallback_AbortCallback'>AbortCallback()</a>
</pre>

Has no effect.

### See Also

<a href='#SkPicture_playback'>playback</a>

<a name='SkPicture_AbortCallback_abort'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual bool <a href='#SkPicture_AbortCallback_abort'>abort()</a> = 0
</pre>

Stops <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>playback</a> <a href='SkPicture_Reference#SkPicture'>when</a> <a href='SkPicture_Reference#SkPicture'>some</a> <a href='SkPicture_Reference#SkPicture'>condition</a> <a href='SkPicture_Reference#SkPicture'>is</a> <a href='SkPicture_Reference#SkPicture'>met</a>. <a href='SkPicture_Reference#SkPicture'>A</a> <a href='SkPicture_Reference#SkPicture'>subclass</a> <a href='SkPicture_Reference#SkPicture'>of</a>
<a href='#SkPicture_AbortCallback'>AbortCallback</a> <a href='#SkPicture_AbortCallback'>provides</a> <a href='#SkPicture_AbortCallback'>an</a> <a href='#SkPicture_AbortCallback'>override</a> <a href='#SkPicture_AbortCallback'>for</a> <a href='#SkPicture_AbortCallback_abort'>abort()</a> <a href='#SkPicture_AbortCallback_abort'>that</a> <a href='#SkPicture_AbortCallback_abort'>can</a> <a href='#SkPicture_AbortCallback_abort'>stop</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a>::<a href='#SkPicture_playback'>playback</a>.

The part of <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>drawn</a> <a href='SkPicture_Reference#SkPicture'>when</a> <a href='SkPicture_Reference#SkPicture'>aborted</a> <a href='SkPicture_Reference#SkPicture'>is</a> <a href='SkPicture_Reference#SkPicture'>undefined</a>. <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>instantiations</a> <a href='SkPicture_Reference#SkPicture'>are</a>
free to stop drawing at different <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>during</a> <a href='SkPoint_Reference#Point'>playback</a>.

If the abort happens inside one or more calls to <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_save'>save()</a>, <a href='#SkCanvas_save'>stack</a>
of <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>and</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>clip</a> <a href='SkCanvas_Reference#SkCanvas'>values</a> <a href='SkCanvas_Reference#SkCanvas'>is</a> <a href='SkCanvas_Reference#SkCanvas'>restored</a> <a href='SkCanvas_Reference#SkCanvas'>to</a> <a href='SkCanvas_Reference#SkCanvas'>its</a> <a href='SkCanvas_Reference#SkCanvas'>state</a> <a href='SkCanvas_Reference#SkCanvas'>before</a>
<a href='SkPicture_Reference#SkPicture'>SkPicture</a>::<a href='#SkPicture_playback'>playback</a> <a href='#SkPicture_playback'>was</a> <a href='#SkPicture_playback'>called</a>.

### Return Value

true to stop playback

### See Also

<a href='#SkPicture_playback'>playback</a>

### Example

<div><fiddle-embed name="56ed920dadbf2b2967ac45fb5a9bded6"><div>JustOneDraw allows the black rectangle to draw but stops playback before the
white rectangle appears.
</div></fiddle-embed></div>

<a name='SkPicture_MakeFromStream'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkPicture_Reference#SkPicture'>SkPicture</a>&<a href='SkPicture_Reference#SkPicture'>gt</a>; <a href='#SkPicture_MakeFromStream'>MakeFromStream</a>(<a href='SkStream_Reference#SkStream'>SkStream</a>* <a href='SkStream_Reference#Stream'>stream</a>, <a href='SkStream_Reference#Stream'>const</a> <a href='undocumented#SkDeserialProcs'>SkDeserialProcs</a>* <a href='undocumented#SkDeserialProcs'>procs</a> = <a href='undocumented#SkDeserialProcs'>nullptr</a>)
</pre>

Recreates <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>that</a> <a href='SkPicture_Reference#SkPicture'>was</a> <a href='SkPicture_Reference#SkPicture'>serialized</a> <a href='SkPicture_Reference#SkPicture'>into</a> <a href='SkPicture_Reference#SkPicture'>a</a> <a href='#SkPicture_MakeFromStream_stream'>stream</a>. <a href='#SkPicture_MakeFromStream_stream'>Returns</a> <a href='#SkPicture_MakeFromStream_stream'>constructed</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a>
if successful; otherwise, returns nullptr. Fails if <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>does</a> <a href='undocumented#Data'>not</a> <a href='undocumented#Data'>permit</a>
constructing valid <a href='SkPicture_Reference#SkPicture'>SkPicture</a>.

<a href='#SkPicture_MakeFromStream_procs'>procs</a>-><a href='#SkDeserialProcs_fPictureProc'>fPictureProc</a> <a href='#SkDeserialProcs_fPictureProc'>permits</a> <a href='#SkDeserialProcs_fPictureProc'>supplying</a> <a href='#SkDeserialProcs_fPictureProc'>a</a> <a href='#SkDeserialProcs_fPictureProc'>custom</a> <a href='#SkDeserialProcs_fPictureProc'>function</a> <a href='#SkDeserialProcs_fPictureProc'>to</a> <a href='#SkDeserialProcs_fPictureProc'>decode</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a>.
If <a href='#SkPicture_MakeFromStream_procs'>procs</a>-><a href='#SkDeserialProcs_fPictureProc'>fPictureProc</a> <a href='#SkDeserialProcs_fPictureProc'>is</a> <a href='#SkDeserialProcs_fPictureProc'>nullptr</a>, <a href='#SkDeserialProcs_fPictureProc'>default</a> <a href='#SkDeserialProcs_fPictureProc'>decoding</a> <a href='#SkDeserialProcs_fPictureProc'>is</a> <a href='#SkDeserialProcs_fPictureProc'>used</a>. <a href='#SkPicture_MakeFromStream_procs'>procs</a>-><a href='#SkDeserialProcs_fPictureCtx'>fPictureCtx</a>
may be used to provide user context to <a href='#SkPicture_MakeFromStream_procs'>procs</a>-><a href='#SkDeserialProcs_fPictureProc'>fPictureProc</a>; <a href='#SkPicture_MakeFromStream_procs'>procs</a>-><a href='#SkDeserialProcs_fPictureProc'>fPictureProc</a>
is called with a pointer to <a href='undocumented#Data'>data</a>, <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>byte</a> <a href='undocumented#Data'>length</a>, <a href='undocumented#Data'>and</a> <a href='undocumented#Data'>user</a> <a href='undocumented#Data'>context</a>.

### Parameters

<table>  <tr>    <td><a name='SkPicture_MakeFromStream_stream'><code><strong>stream</strong></code></a></td>
    <td>container for serial <a href='undocumented#Data'>data</a></td>
  </tr>
  <tr>    <td><a name='SkPicture_MakeFromStream_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>decoders</a>; <a href='undocumented#Data'>may</a> <a href='undocumented#Data'>be</a> <a href='undocumented#Data'>nullptr</a></td>
  </tr>
</table>

### Return Value

<a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>constructed</a> <a href='SkPicture_Reference#SkPicture'>from</a> <a href='#SkPicture_MakeFromStream_stream'>stream</a> <a href='undocumented#Data'>data</a>

### Example

<div><fiddle-embed name="404fb42560a289c2004cad1caf3b96de"></fiddle-embed></div>

### See Also

<a href='#SkPicture_MakeFromData'>MakeFromData</a> <a href='undocumented#SkPictureRecorder'>SkPictureRecorder</a>

<a name='SkPicture_MakeFromData'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkPicture_Reference#SkPicture'>SkPicture</a>&<a href='SkPicture_Reference#SkPicture'>gt</a>; <a href='#SkPicture_MakeFromData'>MakeFromData</a>(<a href='#SkPicture_MakeFromData'>const</a> <a href='undocumented#SkData'>SkData</a>* <a href='undocumented#Data'>data</a>, <a href='undocumented#Data'>const</a> <a href='undocumented#SkDeserialProcs'>SkDeserialProcs</a>* <a href='undocumented#SkDeserialProcs'>procs</a> = <a href='undocumented#SkDeserialProcs'>nullptr</a>)
</pre>

Recreates <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>that</a> <a href='SkPicture_Reference#SkPicture'>was</a> <a href='SkPicture_Reference#SkPicture'>serialized</a> <a href='SkPicture_Reference#SkPicture'>into</a> <a href='#SkPicture_MakeFromData_data'>data</a>. <a href='#SkPicture_MakeFromData_data'>Returns</a> <a href='#SkPicture_MakeFromData_data'>constructed</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a>
if successful; otherwise, returns nullptr. Fails if <a href='#SkPicture_MakeFromData_data'>data</a> <a href='#SkPicture_MakeFromData_data'>does</a> <a href='#SkPicture_MakeFromData_data'>not</a> <a href='#SkPicture_MakeFromData_data'>permit</a>
constructing valid <a href='SkPicture_Reference#SkPicture'>SkPicture</a>.

<a href='#SkPicture_MakeFromData_procs'>procs</a>-><a href='#SkDeserialProcs_fPictureProc'>fPictureProc</a> <a href='#SkDeserialProcs_fPictureProc'>permits</a> <a href='#SkDeserialProcs_fPictureProc'>supplying</a> <a href='#SkDeserialProcs_fPictureProc'>a</a> <a href='#SkDeserialProcs_fPictureProc'>custom</a> <a href='#SkDeserialProcs_fPictureProc'>function</a> <a href='#SkDeserialProcs_fPictureProc'>to</a> <a href='#SkDeserialProcs_fPictureProc'>decode</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a>.
If <a href='#SkPicture_MakeFromData_procs'>procs</a>-><a href='#SkDeserialProcs_fPictureProc'>fPictureProc</a> <a href='#SkDeserialProcs_fPictureProc'>is</a> <a href='#SkDeserialProcs_fPictureProc'>nullptr</a>, <a href='#SkDeserialProcs_fPictureProc'>default</a> <a href='#SkDeserialProcs_fPictureProc'>decoding</a> <a href='#SkDeserialProcs_fPictureProc'>is</a> <a href='#SkDeserialProcs_fPictureProc'>used</a>. <a href='#SkPicture_MakeFromData_procs'>procs</a>-><a href='#SkDeserialProcs_fPictureCtx'>fPictureCtx</a>
may be used to provide user context to <a href='#SkPicture_MakeFromData_procs'>procs</a>-><a href='#SkDeserialProcs_fPictureProc'>fPictureProc</a>; <a href='#SkPicture_MakeFromData_procs'>procs</a>-><a href='#SkDeserialProcs_fPictureProc'>fPictureProc</a>
is called with a pointer to <a href='#SkPicture_MakeFromData_data'>data</a>, <a href='#SkPicture_MakeFromData_data'>data</a> <a href='#SkPicture_MakeFromData_data'>byte</a> <a href='#SkPicture_MakeFromData_data'>length</a>, <a href='#SkPicture_MakeFromData_data'>and</a> <a href='#SkPicture_MakeFromData_data'>user</a> <a href='#SkPicture_MakeFromData_data'>context</a>.

### Parameters

<table>  <tr>    <td><a name='SkPicture_MakeFromData_data'><code><strong>data</strong></code></a></td>
    <td>container for serial <a href='#SkPicture_MakeFromData_data'>data</a></td>
  </tr>
  <tr>    <td><a name='SkPicture_MakeFromData_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial <a href='#SkPicture_MakeFromData_data'>data</a> <a href='#SkPicture_MakeFromData_data'>decoders</a>; <a href='#SkPicture_MakeFromData_data'>may</a> <a href='#SkPicture_MakeFromData_data'>be</a> <a href='#SkPicture_MakeFromData_data'>nullptr</a></td>
  </tr>
</table>

### Return Value

<a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>constructed</a> <a href='SkPicture_Reference#SkPicture'>from</a> <a href='#SkPicture_MakeFromData_data'>data</a>

### Example

<div><fiddle-embed name="58b44bf47d8816782066618700afdecb"></fiddle-embed></div>

### See Also

<a href='#SkPicture_MakeFromStream'>MakeFromStream</a> <a href='undocumented#SkPictureRecorder'>SkPictureRecorder</a>

<a name='SkPicture_MakeFromData_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkPicture_Reference#SkPicture'>SkPicture</a>&<a href='SkPicture_Reference#SkPicture'>gt</a>; <a href='#SkPicture_MakeFromData'>MakeFromData</a>(<a href='#SkPicture_MakeFromData'>const</a> <a href='#SkPicture_MakeFromData'>void</a>* <a href='undocumented#Data'>data</a>, <a href='undocumented#Data'>size_t</a> <a href='undocumented#Size'>size</a>,
                                     <a href='undocumented#Size'>const</a> <a href='undocumented#SkDeserialProcs'>SkDeserialProcs</a>* <a href='undocumented#SkDeserialProcs'>procs</a> = <a href='undocumented#SkDeserialProcs'>nullptr</a>)
</pre>

### Parameters

<table>  <tr>    <td><a name='SkPicture_MakeFromData_2_data'><code><strong>data</strong></code></a></td>
    <td>pointer to serial <a href='#SkPicture_MakeFromData_2_data'>data</a></td>
  </tr>
  <tr>    <td><a name='SkPicture_MakeFromData_2_size'><code><strong>size</strong></code></a></td>
    <td><a href='#SkPicture_MakeFromData_2_size'>size</a> <a href='#SkPicture_MakeFromData_2_size'>of</a> <a href='#SkPicture_MakeFromData_2_data'>data</a></td>
  </tr>
  <tr>    <td><a name='SkPicture_MakeFromData_2_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial <a href='#SkPicture_MakeFromData_2_data'>data</a> <a href='#SkPicture_MakeFromData_2_data'>decoders</a>; <a href='#SkPicture_MakeFromData_2_data'>may</a> <a href='#SkPicture_MakeFromData_2_data'>be</a> <a href='#SkPicture_MakeFromData_2_data'>nullptr</a></td>
  </tr>
</table>

### Return Value

<a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>constructed</a> <a href='SkPicture_Reference#SkPicture'>from</a> <a href='#SkPicture_MakeFromData_2_data'>data</a>

### Example

<div><fiddle-embed name="30b9f1b310187db6aff720a5d67591e2"></fiddle-embed></div>

### See Also

<a href='#SkPicture_MakeFromStream'>MakeFromStream</a> <a href='undocumented#SkPictureRecorder'>SkPictureRecorder</a>

<a name='SkPicture_playback'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual void playback(<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* <a href='SkCanvas_Reference#Canvas'>canvas</a>, <a href='#SkPicture_AbortCallback'>AbortCallback</a>* <a href='#SkPicture_AbortCallback'>callback</a> = <a href='#SkPicture_AbortCallback'>nullptr</a>) <a href='#SkPicture_AbortCallback'>const</a> = 0
</pre>

Replays the drawing commands on the specified <a href='#SkPicture_playback_canvas'>canvas</a>. <a href='#SkPicture_playback_canvas'>In</a> <a href='#SkPicture_playback_canvas'>the</a> <a href='#SkPicture_playback_canvas'>case</a> <a href='#SkPicture_playback_canvas'>that</a> <a href='#SkPicture_playback_canvas'>the</a>
commands are recorded, each command in the <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>is</a> <a href='SkPicture_Reference#SkPicture'>sent</a> <a href='SkPicture_Reference#SkPicture'>separately</a> <a href='SkPicture_Reference#SkPicture'>to</a> <a href='#SkPicture_playback_canvas'>canvas</a>.

To add a single command to draw <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>to</a> <a href='SkPicture_Reference#SkPicture'>recording</a> <a href='#SkPicture_playback_canvas'>canvas</a>, <a href='#SkPicture_playback_canvas'>call</a>
<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawPicture'>drawPicture</a> <a href='#SkCanvas_drawPicture'>instead</a>.

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

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawPicture'>drawPicture</a>

<a name='SkPicture_cullRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkPicture_cullRect'>cullRect</a>() <a href='#SkPicture_cullRect'>const</a> = 0
</pre>

Returns cull <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>for</a> <a href='SkRect_Reference#SkRect'>this</a> <a href='SkPicture_Reference#Picture'>picture</a>, <a href='SkPicture_Reference#Picture'>passed</a> <a href='SkPicture_Reference#Picture'>in</a> <a href='SkPicture_Reference#Picture'>when</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>was</a> <a href='SkPicture_Reference#SkPicture'>created</a>.
Returned <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>does</a> <a href='SkRect_Reference#SkRect'>not</a> <a href='SkRect_Reference#SkRect'>specify</a> <a href='SkRect_Reference#SkRect'>clipping</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>for</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a>; <a href='SkPicture_Reference#SkPicture'>cull</a> <a href='SkPicture_Reference#SkPicture'>is</a> <a href='SkPicture_Reference#SkPicture'>hint</a>
of <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>bounds</a>.

<a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>is</a> <a href='SkPicture_Reference#SkPicture'>free</a> <a href='SkPicture_Reference#SkPicture'>to</a> <a href='SkPicture_Reference#SkPicture'>discard</a> <a href='SkPicture_Reference#SkPicture'>recorded</a> <a href='SkPicture_Reference#SkPicture'>drawing</a> <a href='SkPicture_Reference#SkPicture'>commands</a> <a href='SkPicture_Reference#SkPicture'>that</a> <a href='SkPicture_Reference#SkPicture'>fall</a> <a href='SkPicture_Reference#SkPicture'>outside</a>
cull.

### Return Value

bounds passed when <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>was</a> <a href='SkPicture_Reference#SkPicture'>created</a>

### Example

<div><fiddle-embed name="15bb9a9596b40c5e2045f76e8c1dcf8e"><div><a href='SkPicture_Reference#Picture'>Picture</a> <a href='SkPicture_Reference#Picture'>recorded</a> <a href='SkPicture_Reference#Picture'>bounds</a> <a href='SkPicture_Reference#Picture'>are</a> <a href='SkPicture_Reference#Picture'>smaller</a> <a href='SkPicture_Reference#Picture'>than</a> <a href='SkPicture_Reference#Picture'>contents</a>; <a href='SkPicture_Reference#Picture'>contents</a> <a href='SkPicture_Reference#Picture'>outside</a> <a href='SkPicture_Reference#Picture'>recorded</a>
<a href='SkPicture_Reference#Picture'>bounds</a> <a href='SkPicture_Reference#Picture'>may</a> <a href='SkPicture_Reference#Picture'>be</a> <a href='SkPicture_Reference#Picture'>drawn</a>, <a href='SkPicture_Reference#Picture'>and</a> <a href='SkPicture_Reference#Picture'>are</a> <a href='SkPicture_Reference#Picture'>drawn</a> <a href='SkPicture_Reference#Picture'>in</a> <a href='SkPicture_Reference#Picture'>this</a> <a href='SkPicture_Reference#Picture'>example</a>.
</div></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_clipRect'>clipRect</a>

<a name='SkPicture_uniqueID'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkPicture_uniqueID'>uniqueID</a>() <a href='#SkPicture_uniqueID'>const</a>
</pre>

Returns a non-zero value unique among <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>in</a> <a href='SkPicture_Reference#SkPicture'>Skia</a> <a href='SkPicture_Reference#SkPicture'>process</a>.

### Return Value

identifier for <a href='SkPicture_Reference#SkPicture'>SkPicture</a>

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

<a name='SkPicture_serialize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkData'>SkData</a>&<a href='undocumented#SkData'>gt</a>; <a href='#SkPicture_serialize'>serialize</a>(<a href='#SkPicture_serialize'>const</a> <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>* <a href='undocumented#SkSerialProcs'>procs</a> = <a href='undocumented#SkSerialProcs'>nullptr</a>) <a href='undocumented#SkSerialProcs'>const</a>
</pre>

Returns storage containing <a href='undocumented#SkData'>SkData</a> <a href='undocumented#SkData'>describing</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a>, <a href='SkPicture_Reference#SkPicture'>using</a> <a href='SkPicture_Reference#SkPicture'>optional</a> <a href='SkPicture_Reference#SkPicture'>custom</a>
encoders.

<a href='#SkPicture_serialize_procs'>procs</a>-><a href='#SkSerialProcs_fPictureProc'>fPictureProc</a> <a href='#SkSerialProcs_fPictureProc'>permits</a> <a href='#SkSerialProcs_fPictureProc'>supplying</a> <a href='#SkSerialProcs_fPictureProc'>a</a> <a href='#SkSerialProcs_fPictureProc'>custom</a> <a href='#SkSerialProcs_fPictureProc'>function</a> <a href='#SkSerialProcs_fPictureProc'>to</a> <a href='#SkSerialProcs_fPictureProc'>encode</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a>.
If <a href='#SkPicture_serialize_procs'>procs</a>-><a href='#SkSerialProcs_fPictureProc'>fPictureProc</a> <a href='#SkSerialProcs_fPictureProc'>is</a> <a href='#SkSerialProcs_fPictureProc'>nullptr</a>, <a href='#SkSerialProcs_fPictureProc'>default</a> <a href='#SkSerialProcs_fPictureProc'>encoding</a> <a href='#SkSerialProcs_fPictureProc'>is</a> <a href='#SkSerialProcs_fPictureProc'>used</a>. <a href='#SkPicture_serialize_procs'>procs</a>-><a href='#SkSerialProcs_fPictureCtx'>fPictureCtx</a>
may be used to provide user context to <a href='#SkPicture_serialize_procs'>procs</a>-><a href='#SkSerialProcs_fPictureProc'>fPictureProc</a>; <a href='#SkPicture_serialize_procs'>procs</a>-><a href='#SkSerialProcs_fPictureProc'>fPictureProc</a>
is called with a pointer to <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>and</a> <a href='SkPicture_Reference#SkPicture'>user</a> <a href='SkPicture_Reference#SkPicture'>context</a>.

### Parameters

<table>  <tr>    <td><a name='SkPicture_serialize_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>encoders</a>; <a href='undocumented#Data'>may</a> <a href='undocumented#Data'>be</a> <a href='undocumented#Data'>nullptr</a></td>
  </tr>
</table>

### Return Value

storage containing serialized <a href='SkPicture_Reference#SkPicture'>SkPicture</a>

### Example

<div><fiddle-embed name="dacdebe1355c884ebd3c2ea038cc7a20"></fiddle-embed></div>

### See Also

<a href='#SkPicture_MakeFromData'>MakeFromData</a> <a href='undocumented#SkData'>SkData</a> <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>

<a name='SkPicture_serialize_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPicture_serialize'>serialize</a>(<a href='SkWStream_Reference#SkWStream'>SkWStream</a>* <a href='SkStream_Reference#Stream'>stream</a>, <a href='SkStream_Reference#Stream'>const</a> <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>* <a href='undocumented#SkSerialProcs'>procs</a> = <a href='undocumented#SkSerialProcs'>nullptr</a>) <a href='undocumented#SkSerialProcs'>const</a>
</pre>

Writes <a href='SkPicture_Reference#Picture'>picture</a> <a href='SkPicture_Reference#Picture'>to</a> <a href='#SkPicture_serialize_2_stream'>stream</a>, <a href='#SkPicture_serialize_2_stream'>using</a> <a href='#SkPicture_serialize_2_stream'>optional</a> <a href='#SkPicture_serialize_2_stream'>custom</a> <a href='#SkPicture_serialize_2_stream'>encoders</a>.

<a href='#SkPicture_serialize_2_procs'>procs</a>-><a href='#SkSerialProcs_fPictureProc'>fPictureProc</a> <a href='#SkSerialProcs_fPictureProc'>permits</a> <a href='#SkSerialProcs_fPictureProc'>supplying</a> <a href='#SkSerialProcs_fPictureProc'>a</a> <a href='#SkSerialProcs_fPictureProc'>custom</a> <a href='#SkSerialProcs_fPictureProc'>function</a> <a href='#SkSerialProcs_fPictureProc'>to</a> <a href='#SkSerialProcs_fPictureProc'>encode</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a>.
If <a href='#SkPicture_serialize_2_procs'>procs</a>-><a href='#SkSerialProcs_fPictureProc'>fPictureProc</a> <a href='#SkSerialProcs_fPictureProc'>is</a> <a href='#SkSerialProcs_fPictureProc'>nullptr</a>, <a href='#SkSerialProcs_fPictureProc'>default</a> <a href='#SkSerialProcs_fPictureProc'>encoding</a> <a href='#SkSerialProcs_fPictureProc'>is</a> <a href='#SkSerialProcs_fPictureProc'>used</a>. <a href='#SkPicture_serialize_2_procs'>procs</a>-><a href='#SkSerialProcs_fPictureCtx'>fPictureCtx</a>
may be used to provide user context to <a href='#SkPicture_serialize_2_procs'>procs</a>-><a href='#SkSerialProcs_fPictureProc'>fPictureProc</a>; <a href='#SkPicture_serialize_2_procs'>procs</a>-><a href='#SkSerialProcs_fPictureProc'>fPictureProc</a>
is called with a pointer to <a href='SkPicture_Reference#SkPicture'>SkPicture</a> <a href='SkPicture_Reference#SkPicture'>and</a> <a href='SkPicture_Reference#SkPicture'>user</a> <a href='SkPicture_Reference#SkPicture'>context</a>.

### Parameters

<table>  <tr>    <td><a name='SkPicture_serialize_2_stream'><code><strong>stream</strong></code></a></td>
    <td>writable serial <a href='undocumented#Data'>data</a> <a href='#SkPicture_serialize_2_stream'>stream</a></td>
  </tr>
  <tr>    <td><a name='SkPicture_serialize_2_procs'><code><strong>procs</strong></code></a></td>
    <td>custom serial <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>encoders</a>; <a href='undocumented#Data'>may</a> <a href='undocumented#Data'>be</a> <a href='undocumented#Data'>nullptr</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="30b9f1b310187db6aff720a5d67591e2"></fiddle-embed></div>

### See Also

<a href='#SkPicture_MakeFromStream'>MakeFromStream</a> <a href='SkWStream_Reference#SkWStream'>SkWStream</a> <a href='undocumented#SkSerialProcs'>SkSerialProcs</a>

<a name='SkPicture_MakePlaceholder'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkPicture_Reference#SkPicture'>SkPicture</a>&<a href='SkPicture_Reference#SkPicture'>gt</a>; <a href='#SkPicture_MakePlaceholder'>MakePlaceholder</a>(<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>cull</a>)
</pre>

Returns a placeholder <a href='SkPicture_Reference#SkPicture'>SkPicture</a>. <a href='SkPicture_Reference#SkPicture'>Result</a> <a href='SkPicture_Reference#SkPicture'>does</a> <a href='SkPicture_Reference#SkPicture'>not</a> <a href='SkPicture_Reference#SkPicture'>draw</a>, <a href='SkPicture_Reference#SkPicture'>and</a> <a href='SkPicture_Reference#SkPicture'>contains</a> <a href='SkPicture_Reference#SkPicture'>only</a>
<a href='#SkPicture_MakePlaceholder_cull'>cull</a> <a href='SkRect_Reference#SkRect'>SkRect</a>, <a href='SkRect_Reference#SkRect'>a</a> <a href='SkRect_Reference#SkRect'>hint</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='SkRect_Reference#SkRect'>its</a> <a href='SkRect_Reference#SkRect'>bounds</a>. <a href='SkRect_Reference#SkRect'>Result</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>immutable</a>; <a href='SkRect_Reference#SkRect'>it</a> <a href='SkRect_Reference#SkRect'>cannot</a> <a href='SkRect_Reference#SkRect'>be</a> <a href='SkRect_Reference#SkRect'>changed</a>
later. Result identifier is unique.

Returned placeholder can be intercepted during playback to insert other
commands into <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>draw</a> <a href='SkStream_Reference#Stream'>stream</a>.

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

<a href='#SkPicture_MakeFromStream'>MakeFromStream</a> <a href='#SkPicture_MakeFromData'>MakeFromData</a> <a href='#SkPicture_uniqueID'>uniqueID</a>

<a name='SkPicture_approximateOpCount'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual int <a href='#SkPicture_approximateOpCount'>approximateOpCount</a>() <a href='#SkPicture_approximateOpCount'>const</a> = 0
</pre>

Returns the approximate number of operations in <a href='SkPicture_Reference#SkPicture'>SkPicture</a>. <a href='SkPicture_Reference#SkPicture'>Returned</a> <a href='SkPicture_Reference#SkPicture'>value</a>
may be greater or less than the number of <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>calls</a>
recorded: some calls may be recorded as more than one operation, other
calls may be optimized away.

### Return Value

approximate operation count

### Example

<div><fiddle-embed name="4b3d879118ef770d1f11a23c6493b2c4"></fiddle-embed></div>

### See Also

<a href='#SkPicture_approximateBytesUsed'>approximateBytesUsed</a>

<a name='SkPicture_approximateBytesUsed'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
virtual size_t <a href='#SkPicture_approximateBytesUsed'>approximateBytesUsed</a>() <a href='#SkPicture_approximateBytesUsed'>const</a> = 0
</pre>

Returns the approximate byte <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='SkPicture_Reference#SkPicture'>SkPicture</a>. <a href='SkPicture_Reference#SkPicture'>Does</a> <a href='SkPicture_Reference#SkPicture'>not</a> <a href='SkPicture_Reference#SkPicture'>include</a> <a href='SkPicture_Reference#SkPicture'>large</a> <a href='SkPicture_Reference#SkPicture'>objects</a>
referenced by <a href='SkPicture_Reference#SkPicture'>SkPicture</a>.

### Return Value

approximate <a href='undocumented#Size'>size</a>

### Example

<div><fiddle-embed name="ececbda21218bd732394a305dba393a2"></fiddle-embed></div>

### See Also

<a href='#SkPicture_approximateOpCount'>approximateOpCount</a>

