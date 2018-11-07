SkTextBlobBuilder Reference
===


<a name='SkTextBlobBuilder'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>SkTextBlobBuilder</a> {
<a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>public</a>:
    <a href='#SkTextBlobBuilder_empty_constructor'>SkTextBlobBuilder()</a>;
    ~<a href='#SkTextBlobBuilder_empty_constructor'>SkTextBlobBuilder()</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>> <a href='#SkTextBlobBuilder_make'>make()</a>;

    <a href='#SkTextBlobBuilder_make'>struct</a> <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a> {
        <a href='undocumented#SkGlyphID'>SkGlyphID</a>* <a href='undocumented#Glyph'>glyphs</a>;
        <a href='undocumented#SkScalar'>SkScalar</a>* <a href='undocumented#SkScalar'>pos</a>;
        <a href='undocumented#SkScalar'>char</a>* <a href='undocumented#SkScalar'>utf8text</a>;
        <a href='undocumented#SkScalar'>uint32_t</a>* <a href='undocumented#SkScalar'>clusters</a>;
    };

    <a href='undocumented#SkScalar'>const</a> <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRun'>allocRun</a>(<a href='#SkTextBlobBuilder_allocRun'>const</a> <a href='undocumented#SkFont'>SkFont</a>& <a href='undocumented#Font'>font</a>, <a href='undocumented#Font'>int</a> <a href='undocumented#Font'>count</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>,
                              <a href='undocumented#SkScalar'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a> = <a href='SkRect_Reference#SkRect'>nullptr</a>);
    <a href='SkRect_Reference#SkRect'>const</a> <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRunPosH'>allocRunPosH</a>(<a href='#SkTextBlobBuilder_allocRunPosH'>const</a> <a href='undocumented#SkFont'>SkFont</a>& <a href='undocumented#Font'>font</a>, <a href='undocumented#Font'>int</a> <a href='undocumented#Font'>count</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>,
                                  <a href='undocumented#SkScalar'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a> = <a href='SkRect_Reference#SkRect'>nullptr</a>);
    <a href='SkRect_Reference#SkRect'>const</a> <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRunPos'>allocRunPos</a>(<a href='#SkTextBlobBuilder_allocRunPos'>const</a> <a href='undocumented#SkFont'>SkFont</a>& <a href='undocumented#Font'>font</a>, <a href='undocumented#Font'>int</a> <a href='undocumented#Font'>count</a>,
                                 <a href='undocumented#Font'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a> = <a href='SkRect_Reference#SkRect'>nullptr</a>);
    <a href='SkRect_Reference#SkRect'>const</a> <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRun'>allocRun</a>(<a href='#SkTextBlobBuilder_allocRun'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='undocumented#Font'>font</a>, <a href='undocumented#Font'>int</a> <a href='undocumented#Font'>count</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>,
                              <a href='undocumented#SkScalar'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a> = <a href='SkRect_Reference#SkRect'>nullptr</a>);
    <a href='SkRect_Reference#SkRect'>const</a> <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRunPosH'>allocRunPosH</a>(<a href='#SkTextBlobBuilder_allocRunPosH'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='undocumented#Font'>font</a>, <a href='undocumented#Font'>int</a> <a href='undocumented#Font'>count</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>,
                                  <a href='undocumented#SkScalar'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a> = <a href='SkRect_Reference#SkRect'>nullptr</a>);
    <a href='SkRect_Reference#SkRect'>const</a> <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRunPos'>allocRunPos</a>(<a href='#SkTextBlobBuilder_allocRunPos'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='undocumented#Font'>font</a>, <a href='undocumented#Font'>int</a> <a href='undocumented#Font'>count</a>,
                                 <a href='undocumented#Font'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a> = <a href='SkRect_Reference#SkRect'>nullptr</a>);
};
</pre>

Helper class for constructing <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>.

<a name='SkTextBlobBuilder_RunBuffer'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    struct <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a> {
        <a href='undocumented#SkGlyphID'>SkGlyphID</a>* <a href='#SkTextBlobBuilder_RunBuffer_glyphs'>glyphs</a>;
        <a href='undocumented#SkScalar'>SkScalar</a>* <a href='#SkTextBlobBuilder_RunBuffer_pos'>pos</a>;
        <a href='#SkTextBlobBuilder_RunBuffer_pos'>char</a>* <a href='#SkTextBlobBuilder_RunBuffer_utf8text'>utf8text</a>;
        <a href='#SkTextBlobBuilder_RunBuffer_utf8text'>uint32_t</a>* <a href='#SkTextBlobBuilder_RunBuffer_clusters'>clusters</a>;
    };
</pre>

<a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a> <a href='#SkTextBlobBuilder_RunBuffer'>supplies</a> <a href='#SkTextBlobBuilder_RunBuffer'>storage</a> <a href='#SkTextBlobBuilder_RunBuffer'>for</a> <a href='undocumented#Glyph'>Glyphs</a> <a href='undocumented#Glyph'>and</a> <a href='undocumented#Glyph'>positions</a> <a href='undocumented#Glyph'>within</a> <a href='undocumented#Glyph'>a</a> <a href='undocumented#Glyph'>run</a>.

<a href='undocumented#Glyph'>A</a> <a href='undocumented#Glyph'>run</a> <a href='undocumented#Glyph'>is</a> <a href='undocumented#Glyph'>a</a> <a href='undocumented#Glyph'>sequence</a> <a href='undocumented#Glyph'>of</a> <a href='undocumented#Glyph'>Glyphs</a> <a href='undocumented#Glyph'>sharing</a> <a href='#Paint_Font_Metrics'>Paint_Font_Metrics</a> <a href='#Paint_Font_Metrics'>and</a> <a href='#Paint_Font_Metrics'>positioning</a>.
<a href='#Paint_Font_Metrics'>Each</a> <a href='#Paint_Font_Metrics'>run</a> <a href='#Paint_Font_Metrics'>may</a> <a href='#Paint_Font_Metrics'>position</a> <a href='#Paint_Font_Metrics'>its</a> <a href='undocumented#Glyph'>Glyphs</a> <a href='undocumented#Glyph'>in</a> <a href='undocumented#Glyph'>one</a> <a href='undocumented#Glyph'>of</a> <a href='undocumented#Glyph'>three</a> <a href='undocumented#Glyph'>ways</a>:
<a href='undocumented#Glyph'>by</a> <a href='undocumented#Glyph'>specifying</a> <a href='undocumented#Glyph'>where</a> <a href='undocumented#Glyph'>the</a> <a href='undocumented#Glyph'>first</a> <a href='undocumented#Glyph'>Glyph</a> <a href='undocumented#Glyph'>is</a> <a href='undocumented#Glyph'>drawn</a>, <a href='undocumented#Glyph'>and</a> <a href='undocumented#Glyph'>allowing</a> <a href='#Paint_Font_Metrics'>Paint_Font_Metrics</a> <a href='#Paint_Font_Metrics'>to</a>
<a href='#Paint_Font_Metrics'>determine</a> <a href='#Paint_Font_Metrics'>the</a> <a href='#Paint_Font_Metrics'>advance</a> <a href='#Paint_Font_Metrics'>to</a> <a href='#Paint_Font_Metrics'>subsequent</a> <a href='undocumented#Glyph'>Glyphs</a>; <a href='undocumented#Glyph'>by</a> <a href='undocumented#Glyph'>specifying</a> <a href='undocumented#Glyph'>a</a> <a href='undocumented#Glyph'>baseline</a>, <a href='undocumented#Glyph'>and</a>
<a href='undocumented#Glyph'>the</a> <a href='undocumented#Glyph'>position</a> <a href='undocumented#Glyph'>on</a> <a href='undocumented#Glyph'>that</a> <a href='undocumented#Glyph'>baseline</a> <a href='undocumented#Glyph'>for</a> <a href='undocumented#Glyph'>each</a> <a href='undocumented#Glyph'>Glyph</a> <a href='undocumented#Glyph'>in</a> <a href='undocumented#Glyph'>run</a>; <a href='undocumented#Glyph'>or</a> <a href='undocumented#Glyph'>by</a> <a href='undocumented#Glyph'>providing</a> <a href='SkPoint_Reference#Point'>Point</a>
<a href='SkPoint_Reference#Point'>array</a>, <a href='SkPoint_Reference#Point'>one</a> <a href='SkPoint_Reference#Point'>per</a> <a href='undocumented#Glyph'>Glyph</a>.<table style='border-collapse: collapse; width: 62.5em'>

  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Member</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkGlyphID*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkTextBlobBuilder_RunBuffer_glyphs'><code>glyphs</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#SkTextBlobBuilder_RunBuffer_glyphs'>glyphs</a> <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>memory</a> <a href='SkPoint_Reference#Point'>for</a> <a href='SkPoint_Reference#Point'>one</a> <a href='SkPoint_Reference#Point'>or</a> <a href='SkPoint_Reference#Point'>more</a> <a href='undocumented#Glyph'>Glyphs</a>. <a href='#SkTextBlobBuilder_RunBuffer_glyphs'>glyphs</a> <a href='#SkTextBlobBuilder_RunBuffer_glyphs'>memory</a> <a href='#SkTextBlobBuilder_RunBuffer_glyphs'>must</a> <a href='#SkTextBlobBuilder_RunBuffer_glyphs'>be</a>
<a href='#SkTextBlobBuilder_RunBuffer_glyphs'>written</a> <a href='#SkTextBlobBuilder_RunBuffer_glyphs'>to</a> <a href='#SkTextBlobBuilder_RunBuffer_glyphs'>by</a> <a href='#SkTextBlobBuilder_RunBuffer_glyphs'>the</a> <a href='#SkTextBlobBuilder_RunBuffer_glyphs'>caller</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkTextBlobBuilder_RunBuffer_pos'><code>pos</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#SkTextBlobBuilder_RunBuffer_pos'>pos</a> <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>memory</a> <a href='SkPoint_Reference#Point'>for</a> <a href='undocumented#Glyph'>Glyph</a> <a href='undocumented#Glyph'>positions</a>. <a href='undocumented#Glyph'>Depending</a> <a href='undocumented#Glyph'>on</a> <a href='undocumented#Glyph'>how</a> <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>
<a href='#SkTextBlobBuilder_RunBuffer'>is</a> <a href='#SkTextBlobBuilder_RunBuffer'>allocated</a>, <a href='#SkTextBlobBuilder_RunBuffer_pos'>pos</a> <a href='#SkTextBlobBuilder_RunBuffer_pos'>may</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>zero</a> <a href='SkPoint_Reference#Point'>bytes</a> <a href='SkPoint_Reference#Point'>per</a> <a href='undocumented#Glyph'>Glyph</a>, <a href='undocumented#Glyph'>one</a> <a href='undocumented#Scalar'>Scalar</a> <a href='undocumented#Scalar'>per</a> <a href='undocumented#Glyph'>Glyph</a>,
<a href='undocumented#Glyph'>or</a> <a href='undocumented#Glyph'>one</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>per</a> <a href='undocumented#Glyph'>Glyph</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>char*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkTextBlobBuilder_RunBuffer_utf8text'><code>utf8text</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Reserved for future use. <a href='#SkTextBlobBuilder_RunBuffer_utf8text'>utf8text</a> <a href='#SkTextBlobBuilder_RunBuffer_utf8text'>should</a> <a href='#SkTextBlobBuilder_RunBuffer_utf8text'>not</a> <a href='#SkTextBlobBuilder_RunBuffer_utf8text'>be</a> <a href='#SkTextBlobBuilder_RunBuffer_utf8text'>read</a> <a href='#SkTextBlobBuilder_RunBuffer_utf8text'>or</a> <a href='#SkTextBlobBuilder_RunBuffer_utf8text'>written</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>uint32_t*</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkTextBlobBuilder_RunBuffer_clusters'><code>clusters</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Reserved for future use. <a href='#SkTextBlobBuilder_RunBuffer_clusters'>clusters</a> <a href='#SkTextBlobBuilder_RunBuffer_clusters'>should</a> <a href='#SkTextBlobBuilder_RunBuffer_clusters'>not</a> <a href='#SkTextBlobBuilder_RunBuffer_clusters'>be</a> <a href='#SkTextBlobBuilder_RunBuffer_clusters'>read</a> <a href='#SkTextBlobBuilder_RunBuffer_clusters'>or</a> <a href='#SkTextBlobBuilder_RunBuffer_clusters'>written</a>.
</td>
  </tr>
</table>

### See Also

<a href='#SkTextBlobBuilder_allocRun'>allocRun</a> <a href='#SkTextBlobBuilder_allocRunPos'>allocRunPos</a> <a href='#SkTextBlobBuilder_allocRunPosH'>allocRunPosH</a>

<a name='SkTextBlobBuilder_empty_constructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkTextBlobBuilder_empty_constructor'>SkTextBlobBuilder()</a>
</pre>

Constructs empty <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>SkTextBlobBuilder</a>. <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>By</a> <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>default</a>, <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>SkTextBlobBuilder</a> <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>has</a> <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>no</a> <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>runs</a>.

### Return Value

empty <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>SkTextBlobBuilder</a>

### Example

<div><fiddle-embed name="d9dbabfe24aad92ee3c8144513e90d81">

#### Example Output

~~~~
blob equals nullptr
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkTextBlobBuilder_make'>make</a> <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>::<a href='#SkTextBlob_MakeFromText'>MakeFromText</a>

<a name='SkTextBlobBuilder_destructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
~<a href='#SkTextBlobBuilder_empty_constructor'>SkTextBlobBuilder()</a>
</pre>

Deletes <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>allocated</a> <a href='undocumented#Data'>internally</a> <a href='undocumented#Data'>by</a> <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>SkTextBlobBuilder</a>.

### See Also

<a href='#SkTextBlobBuilder_empty_constructor'>SkTextBlobBuilder()</a>

<a name='SkTextBlobBuilder_make'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>&<a href='SkTextBlob_Reference#SkTextBlob'>gt</a>; <a href='#SkTextBlobBuilder_make'>make()</a>
</pre>

Returns <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a> <a href='SkTextBlob_Reference#SkTextBlob'>built</a> <a href='SkTextBlob_Reference#SkTextBlob'>from</a> <a href='SkTextBlob_Reference#SkTextBlob'>runs</a> <a href='SkTextBlob_Reference#SkTextBlob'>of</a> <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>added</a> <a href='undocumented#Glyph'>by</a> <a href='undocumented#Glyph'>builder</a>. <a href='undocumented#Glyph'>Returned</a>
<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a> <a href='SkTextBlob_Reference#SkTextBlob'>is</a> <a href='SkTextBlob_Reference#SkTextBlob'>immutable</a>; <a href='SkTextBlob_Reference#SkTextBlob'>it</a> <a href='SkTextBlob_Reference#SkTextBlob'>may</a> <a href='SkTextBlob_Reference#SkTextBlob'>be</a> <a href='SkTextBlob_Reference#SkTextBlob'>copied</a>, <a href='SkTextBlob_Reference#SkTextBlob'>but</a> <a href='SkTextBlob_Reference#SkTextBlob'>its</a> <a href='SkTextBlob_Reference#SkTextBlob'>contents</a> <a href='SkTextBlob_Reference#SkTextBlob'>may</a> <a href='SkTextBlob_Reference#SkTextBlob'>not</a> <a href='SkTextBlob_Reference#SkTextBlob'>be</a> <a href='SkTextBlob_Reference#SkTextBlob'>altered</a>.
Returns nullptr if no runs of <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>were</a> <a href='undocumented#Glyph'>added</a> <a href='undocumented#Glyph'>by</a> <a href='undocumented#Glyph'>builder</a>.

Resets <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>SkTextBlobBuilder</a> <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>to</a> <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>its</a> <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>initial</a> <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>empty</a> <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>state</a>, <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>allowing</a> <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>it</a> <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>to</a> <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>be</a>
reused to build a new set of runs.

### Return Value

<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a> <a href='SkTextBlob_Reference#SkTextBlob'>or</a> <a href='SkTextBlob_Reference#SkTextBlob'>nullptr</a>

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

<a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>::<a href='#SkTextBlob_MakeFromText'>MakeFromText</a>

<a name='SkTextBlobBuilder_allocRun'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRun'>allocRun</a>(<a href='#SkTextBlobBuilder_allocRun'>const</a> <a href='undocumented#SkFont'>SkFont</a>& <a href='undocumented#Font'>font</a>, <a href='undocumented#Font'>int</a> <a href='undocumented#Font'>count</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>,
                          <a href='undocumented#SkScalar'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a> = <a href='SkRect_Reference#SkRect'>nullptr</a>)
</pre>

Returns run with storage for <a href='undocumented#Glyph'>glyphs</a>. <a href='undocumented#Glyph'>Caller</a> <a href='undocumented#Glyph'>must</a> <a href='undocumented#Glyph'>write</a> <a href='#SkTextBlobBuilder_allocRun_count'>count</a> <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>to</a>
<a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_glyphs'>glyphs</a> <a href='#SkTextBlobBuilder_RunBuffer_glyphs'>before</a> <a href='#SkTextBlobBuilder_RunBuffer_glyphs'>next</a> <a href='#SkTextBlobBuilder_RunBuffer_glyphs'>call</a> <a href='#SkTextBlobBuilder_RunBuffer_glyphs'>to</a> <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>SkTextBlobBuilder</a>.

<a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_utf8text'>utf8text</a>, <a href='#SkTextBlobBuilder_RunBuffer_utf8text'>and</a> <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_clusters'>clusters</a> <a href='#SkTextBlobBuilder_RunBuffer_clusters'>should</a> <a href='#SkTextBlobBuilder_RunBuffer_clusters'>be</a> <a href='#SkTextBlobBuilder_RunBuffer_clusters'>ignored</a>.

<a href='undocumented#Glyph'>Glyphs</a> <a href='undocumented#Glyph'>share</a> <a href='undocumented#Glyph'>metrics</a> <a href='undocumented#Glyph'>in</a> <a href='#SkTextBlobBuilder_allocRun_font'>font</a>.

<a href='undocumented#Glyph'>Glyphs</a> <a href='undocumented#Glyph'>are</a> <a href='undocumented#Glyph'>positioned</a> <a href='undocumented#Glyph'>on</a> <a href='undocumented#Glyph'>a</a> <a href='undocumented#Glyph'>baseline</a> <a href='undocumented#Glyph'>at</a> (<a href='#SkTextBlobBuilder_allocRun_x'>x</a>, <a href='#SkTextBlobBuilder_allocRun_y'>y</a>), <a href='#SkTextBlobBuilder_allocRun_y'>using</a> <a href='#SkTextBlobBuilder_allocRun_font'>font</a> <a href='#SkTextBlobBuilder_allocRun_font'>metrics</a> <a href='#SkTextBlobBuilder_allocRun_font'>to</a>
determine their relative placement.

<a href='#SkTextBlobBuilder_allocRun_bounds'>bounds</a> <a href='#SkTextBlobBuilder_allocRun_bounds'>defines</a> <a href='#SkTextBlobBuilder_allocRun_bounds'>an</a> <a href='#SkTextBlobBuilder_allocRun_bounds'>optional</a> <a href='#SkTextBlobBuilder_allocRun_bounds'>bounding</a> <a href='#SkTextBlobBuilder_allocRun_bounds'>box</a>, <a href='#SkTextBlobBuilder_allocRun_bounds'>used</a> <a href='#SkTextBlobBuilder_allocRun_bounds'>to</a> <a href='#SkTextBlobBuilder_allocRun_bounds'>suppress</a> <a href='#SkTextBlobBuilder_allocRun_bounds'>drawing</a> <a href='#SkTextBlobBuilder_allocRun_bounds'>when</a> <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>
<a href='#SkTextBlobBuilder_allocRun_bounds'>bounds</a> <a href='#SkTextBlobBuilder_allocRun_bounds'>does</a> <a href='#SkTextBlobBuilder_allocRun_bounds'>not</a> <a href='#SkTextBlobBuilder_allocRun_bounds'>intersect</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='#SkTextBlobBuilder_allocRun_bounds'>bounds</a>. <a href='#SkTextBlobBuilder_allocRun_bounds'>If</a> <a href='#SkTextBlobBuilder_allocRun_bounds'>bounds</a> <a href='#SkTextBlobBuilder_allocRun_bounds'>is</a> <a href='#SkTextBlobBuilder_allocRun_bounds'>nullptr</a>, <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a> <a href='#SkTextBlobBuilder_allocRun_bounds'>bounds</a>
is computed from (<a href='#SkTextBlobBuilder_allocRun_x'>x</a>, <a href='#SkTextBlobBuilder_allocRun_y'>y</a>) <a href='#SkTextBlobBuilder_allocRun_y'>and</a> <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_glyphs'>glyphs</a> <a href='#SkTextBlobBuilder_RunBuffer_glyphs'>metrics</a>.

### Parameters

<table>  <tr>    <td><a name='SkTextBlobBuilder_allocRun_font'><code><strong>font</strong></code></a></td>
    <td><a href='undocumented#SkFont'>SkFont</a> <a href='undocumented#SkFont'>used</a> <a href='undocumented#SkFont'>for</a> <a href='undocumented#SkFont'>this</a> <a href='undocumented#SkFont'>run</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRun_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='undocumented#Glyph'>glyphs</a></td>
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

writable <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>buffer</a>

### Example

<div><fiddle-embed name="f0e584aec20eaee7a5bfed62aa885eee"></fiddle-embed></div>

### See Also

<a href='#SkTextBlobBuilder_allocRunPosH'>allocRunPosH</a> <a href='#SkTextBlobBuilder_allocRunPos'>allocRunPos</a>

<a name='SkTextBlobBuilder_allocRunPosH'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRunPosH'>allocRunPosH</a>(<a href='#SkTextBlobBuilder_allocRunPosH'>const</a> <a href='undocumented#SkFont'>SkFont</a>& <a href='undocumented#Font'>font</a>, <a href='undocumented#Font'>int</a> <a href='undocumented#Font'>count</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>,
                              <a href='undocumented#SkScalar'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a> = <a href='SkRect_Reference#SkRect'>nullptr</a>)
</pre>

Returns run with storage for <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>and</a> <a href='undocumented#Glyph'>positions</a> <a href='undocumented#Glyph'>along</a> <a href='undocumented#Glyph'>baseline</a>. <a href='undocumented#Glyph'>Caller</a> <a href='undocumented#Glyph'>must</a>
write <a href='#SkTextBlobBuilder_allocRunPosH_count'>count</a> <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>to</a> <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_glyphs'>glyphs</a>, <a href='#SkTextBlobBuilder_RunBuffer_glyphs'>and</a> <a href='#SkTextBlobBuilder_allocRunPosH_count'>count</a> <a href='undocumented#Scalar'>scalars</a> <a href='undocumented#Scalar'>to</a> <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_pos'>pos</a>;
before next call to <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>SkTextBlobBuilder</a>.

<a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_utf8text'>utf8text</a>, <a href='#SkTextBlobBuilder_RunBuffer_utf8text'>and</a> <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_clusters'>clusters</a> <a href='#SkTextBlobBuilder_RunBuffer_clusters'>should</a> <a href='#SkTextBlobBuilder_RunBuffer_clusters'>be</a> <a href='#SkTextBlobBuilder_RunBuffer_clusters'>ignored</a>.

<a href='undocumented#Glyph'>Glyphs</a> <a href='undocumented#Glyph'>share</a> <a href='undocumented#Glyph'>metrics</a> <a href='undocumented#Glyph'>in</a> <a href='#SkTextBlobBuilder_allocRunPosH_font'>font</a>.

<a href='undocumented#Glyph'>Glyphs</a> <a href='undocumented#Glyph'>are</a> <a href='undocumented#Glyph'>positioned</a> <a href='undocumented#Glyph'>on</a> <a href='undocumented#Glyph'>a</a> <a href='undocumented#Glyph'>baseline</a> <a href='undocumented#Glyph'>at</a> <a href='#SkTextBlobBuilder_allocRunPosH_y'>y</a>, <a href='#SkTextBlobBuilder_allocRunPosH_y'>using</a> <a href='#SkTextBlobBuilder_allocRunPosH_y'>x-axis</a> <a href='#SkTextBlobBuilder_allocRunPosH_y'>positions</a> <a href='#SkTextBlobBuilder_allocRunPosH_y'>written</a> <a href='#SkTextBlobBuilder_allocRunPosH_y'>by</a>
caller to <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_pos'>pos</a>.

<a href='#SkTextBlobBuilder_allocRunPosH_bounds'>bounds</a> <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>defines</a> <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>an</a> <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>optional</a> <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>bounding</a> <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>box</a>, <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>used</a> <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>to</a> <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>suppress</a> <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>drawing</a> <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>when</a> <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>
<a href='#SkTextBlobBuilder_allocRunPosH_bounds'>bounds</a> <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>does</a> <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>not</a> <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>intersect</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>bounds</a>. <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>If</a> <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>bounds</a> <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>is</a> <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>nullptr</a>, <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a> <a href='#SkTextBlobBuilder_allocRunPosH_bounds'>bounds</a>
is computed from <a href='#SkTextBlobBuilder_allocRunPosH_y'>y</a>, <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_pos'>pos</a>, <a href='#SkTextBlobBuilder_RunBuffer_pos'>and</a> <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_glyphs'>glyphs</a> <a href='#SkTextBlobBuilder_RunBuffer_glyphs'>metrics</a>.

### Parameters

<table>  <tr>    <td><a name='SkTextBlobBuilder_allocRunPosH_font'><code><strong>font</strong></code></a></td>
    <td><a href='undocumented#SkFont'>SkFont</a> <a href='undocumented#SkFont'>used</a> <a href='undocumented#SkFont'>for</a> <a href='undocumented#SkFont'>this</a> <a href='undocumented#SkFont'>run</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRunPosH_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='undocumented#Glyph'>glyphs</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRunPosH_y'><code><strong>y</strong></code></a></td>
    <td>vertical offset within the blob</td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRunPosH_bounds'><code><strong>bounds</strong></code></a></td>
    <td>optional run bounding box</td>
  </tr>
</table>

### Return Value

writable <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>buffer</a> <a href='undocumented#Glyph'>and</a> <a href='undocumented#Glyph'>x-axis</a> <a href='undocumented#Glyph'>position</a> <a href='undocumented#Glyph'>buffer</a>

### Example

<div><fiddle-embed name="c77ac50f506106fdfef94d20bc1a6934"></fiddle-embed></div>

### See Also

<a href='#SkTextBlobBuilder_allocRunPos'>allocRunPos</a> <a href='#SkTextBlobBuilder_allocRun'>allocRun</a>

<a name='SkTextBlobBuilder_allocRunPos'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRunPos'>allocRunPos</a>(<a href='#SkTextBlobBuilder_allocRunPos'>const</a> <a href='undocumented#SkFont'>SkFont</a>& <a href='undocumented#Font'>font</a>, <a href='undocumented#Font'>int</a> <a href='undocumented#Font'>count</a>, <a href='undocumented#Font'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a> = <a href='SkRect_Reference#SkRect'>nullptr</a>)
</pre>

Returns run with storage for <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>and</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>positions</a>. <a href='SkPoint_Reference#SkPoint'>Caller</a> <a href='SkPoint_Reference#SkPoint'>must</a>
write <a href='#SkTextBlobBuilder_allocRunPos_count'>count</a> <a href='undocumented#Glyph'>glyphs</a> <a href='undocumented#Glyph'>to</a> <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_glyphs'>glyphs</a>, <a href='#SkTextBlobBuilder_RunBuffer_glyphs'>and</a> <a href='#SkTextBlobBuilder_allocRunPos_count'>count</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>to</a> <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_pos'>pos</a>;
before next call to <a href='SkTextBlobBuilder_Reference#SkTextBlobBuilder'>SkTextBlobBuilder</a>.

<a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_utf8text'>utf8text</a>, <a href='#SkTextBlobBuilder_RunBuffer_utf8text'>and</a> <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_clusters'>clusters</a> <a href='#SkTextBlobBuilder_RunBuffer_clusters'>should</a> <a href='#SkTextBlobBuilder_RunBuffer_clusters'>be</a> <a href='#SkTextBlobBuilder_RunBuffer_clusters'>ignored</a>.

<a href='undocumented#Glyph'>Glyphs</a> <a href='undocumented#Glyph'>share</a> <a href='undocumented#Glyph'>metrics</a> <a href='undocumented#Glyph'>in</a> <a href='#SkTextBlobBuilder_allocRunPos_font'>font</a>.

<a href='undocumented#Glyph'>Glyphs</a> <a href='undocumented#Glyph'>are</a> <a href='undocumented#Glyph'>positioned</a> <a href='undocumented#Glyph'>using</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>written</a> <a href='SkPoint_Reference#SkPoint'>by</a> <a href='SkPoint_Reference#SkPoint'>caller</a> <a href='SkPoint_Reference#SkPoint'>to</a> <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_pos'>pos</a>, <a href='#SkTextBlobBuilder_RunBuffer_pos'>using</a>
two <a href='undocumented#Scalar'>scalar</a> <a href='undocumented#Scalar'>values</a> <a href='undocumented#Scalar'>for</a> <a href='undocumented#Scalar'>each</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>.

<a href='#SkTextBlobBuilder_allocRunPos_bounds'>bounds</a> <a href='#SkTextBlobBuilder_allocRunPos_bounds'>defines</a> <a href='#SkTextBlobBuilder_allocRunPos_bounds'>an</a> <a href='#SkTextBlobBuilder_allocRunPos_bounds'>optional</a> <a href='#SkTextBlobBuilder_allocRunPos_bounds'>bounding</a> <a href='#SkTextBlobBuilder_allocRunPos_bounds'>box</a>, <a href='#SkTextBlobBuilder_allocRunPos_bounds'>used</a> <a href='#SkTextBlobBuilder_allocRunPos_bounds'>to</a> <a href='#SkTextBlobBuilder_allocRunPos_bounds'>suppress</a> <a href='#SkTextBlobBuilder_allocRunPos_bounds'>drawing</a> <a href='#SkTextBlobBuilder_allocRunPos_bounds'>when</a> <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a>
<a href='#SkTextBlobBuilder_allocRunPos_bounds'>bounds</a> <a href='#SkTextBlobBuilder_allocRunPos_bounds'>does</a> <a href='#SkTextBlobBuilder_allocRunPos_bounds'>not</a> <a href='#SkTextBlobBuilder_allocRunPos_bounds'>intersect</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='#SkTextBlobBuilder_allocRunPos_bounds'>bounds</a>. <a href='#SkTextBlobBuilder_allocRunPos_bounds'>If</a> <a href='#SkTextBlobBuilder_allocRunPos_bounds'>bounds</a> <a href='#SkTextBlobBuilder_allocRunPos_bounds'>is</a> <a href='#SkTextBlobBuilder_allocRunPos_bounds'>nullptr</a>, <a href='SkTextBlob_Reference#SkTextBlob'>SkTextBlob</a> <a href='#SkTextBlobBuilder_allocRunPos_bounds'>bounds</a>
is computed from <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_pos'>pos</a>, <a href='#SkTextBlobBuilder_RunBuffer_pos'>and</a> <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>::<a href='#SkTextBlobBuilder_RunBuffer_glyphs'>glyphs</a> <a href='#SkTextBlobBuilder_RunBuffer_glyphs'>metrics</a>.

### Parameters

<table>  <tr>    <td><a name='SkTextBlobBuilder_allocRunPos_font'><code><strong>font</strong></code></a></td>
    <td><a href='undocumented#SkFont'>SkFont</a> <a href='undocumented#SkFont'>used</a> <a href='undocumented#SkFont'>for</a> <a href='undocumented#SkFont'>this</a> <a href='undocumented#SkFont'>run</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRunPos_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='undocumented#Glyph'>glyphs</a></td>
  </tr>
  <tr>    <td><a name='SkTextBlobBuilder_allocRunPos_bounds'><code><strong>bounds</strong></code></a></td>
    <td>optional run bounding box</td>
  </tr>
</table>

### Return Value

writable <a href='undocumented#Glyph'>glyph</a> <a href='undocumented#Glyph'>buffer</a> <a href='undocumented#Glyph'>and</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>buffer</a>

### Example

<div><fiddle-embed name="da4fcb4a972b500996be9aff6c6c40e1"></fiddle-embed></div>

### See Also

<a href='#SkTextBlobBuilder_allocRunPosH'>allocRunPosH</a> <a href='#SkTextBlobBuilder_allocRun'>allocRun</a>

<a name='SkTextBlobBuilder_allocRun_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRun'>allocRun</a>(<a href='#SkTextBlobBuilder_allocRun'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='undocumented#Font'>font</a>, <a href='undocumented#Font'>int</a> <a href='undocumented#Font'>count</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>,
                          <a href='undocumented#SkScalar'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a> = <a href='SkRect_Reference#SkRect'>nullptr</a>)
</pre>

Deprecated.

<a name='SkTextBlobBuilder_allocRunPosH_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRunPosH'>allocRunPosH</a>(<a href='#SkTextBlobBuilder_allocRunPosH'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='undocumented#Font'>font</a>, <a href='undocumented#Font'>int</a> <a href='undocumented#Font'>count</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>,
                              <a href='undocumented#SkScalar'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a> = <a href='SkRect_Reference#SkRect'>nullptr</a>)
</pre>

Deprecated.

<a name='SkTextBlobBuilder_allocRunPos_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='#SkTextBlobBuilder_RunBuffer'>RunBuffer</a>& <a href='#SkTextBlobBuilder_allocRunPos'>allocRunPos</a>(<a href='#SkTextBlobBuilder_allocRunPos'>const</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>& <a href='undocumented#Font'>font</a>, <a href='undocumented#Font'>int</a> <a href='undocumented#Font'>count</a>, <a href='undocumented#Font'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a> = <a href='SkRect_Reference#SkRect'>nullptr</a>)
</pre>

Deprecated.

