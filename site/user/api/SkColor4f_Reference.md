SkColor4f Reference
===


<a name='SkPM4f'></a>

---

<a name='SkRGBA4f'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
struct <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> {
    <a href='SkColor4f_Reference#SkRGBA4f'>float</a> <a href='#SkRGBA4f_fR'>fR</a>;
    <a href='#SkRGBA4f_fR'>float</a> <a href='#SkRGBA4f_fG'>fG</a>;
    <a href='#SkRGBA4f_fG'>float</a> <a href='#SkRGBA4f_fB'>fB</a>;
    <a href='#SkRGBA4f_fB'>float</a> <a href='#SkRGBA4f_fA'>fA</a>;

    <a href='#SkRGBA4f_fA'>bool</a> <a href='#SkRGBA4f_fA'>operator</a>==(<a href='#SkRGBA4f_fA'>const</a> <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a>& <a href='SkColor4f_Reference#SkRGBA4f'>other</a>) <a href='SkColor4f_Reference#SkRGBA4f'>const</a>;
    <a href='SkColor4f_Reference#SkRGBA4f'>bool</a> <a href='SkColor4f_Reference#SkRGBA4f'>operator</a>!=(<a href='SkColor4f_Reference#SkRGBA4f'>const</a> <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a>& <a href='SkColor4f_Reference#SkRGBA4f'>other</a>) <a href='SkColor4f_Reference#SkRGBA4f'>const</a>;
    <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='SkColor4f_Reference#SkRGBA4f'>operator</a>*(<a href='SkColor4f_Reference#SkRGBA4f'>float</a> <a href='SkColor4f_Reference#SkRGBA4f'>scale</a>) <a href='SkColor4f_Reference#SkRGBA4f'>const</a>;
    <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='SkColor4f_Reference#SkRGBA4f'>operator</a>*(<a href='SkColor4f_Reference#SkRGBA4f'>const</a> <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a>& <a href='SkColor4f_Reference#SkRGBA4f'>scale</a>) <a href='SkColor4f_Reference#SkRGBA4f'>const</a>;
    <a href='SkColor4f_Reference#SkRGBA4f'>const</a> <a href='SkColor4f_Reference#SkRGBA4f'>float</a>* <a href='#SkRGBA4f_vec'>vec()</a> <a href='#SkRGBA4f_vec'>const</a>;
    <a href='#SkRGBA4f_vec'>float</a>* <a href='#SkRGBA4f_vec'>vec()</a>;
    <a href='#SkRGBA4f_vec'>float</a> <a href='#SkRGBA4f_vec'>operator</a>[](<a href='#SkRGBA4f_vec'>int</a> <a href='#SkRGBA4f_vec'>index</a>) <a href='#SkRGBA4f_vec'>const</a>;
    <a href='#SkRGBA4f_vec'>float</a>& <a href='#SkRGBA4f_vec'>operator</a>[](<a href='#SkRGBA4f_vec'>int</a> <a href='#SkRGBA4f_vec'>index</a>);
    <a href='#SkRGBA4f_vec'>bool</a> <a href='#SkRGBA4f_isOpaque'>isOpaque</a>() <a href='#SkRGBA4f_isOpaque'>const</a>;
    <a href='#SkRGBA4f_isOpaque'>static</a> <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_FromColor'>FromColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a>);
    <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkRGBA4f_toSkColor'>toSkColor</a>() <a href='#SkRGBA4f_toSkColor'>const</a>;
    <a href='#SkRGBA4f_toSkColor'>static</a> <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_FromPMColor'>FromPMColor</a>(<a href='SkColor_Reference#SkPMColor'>SkPMColor</a>);
    <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a><<a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>> <a href='#SkRGBA4f_premul'>premul()</a> <a href='#SkRGBA4f_premul'>const</a>;
    <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a><<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>> <a href='#SkRGBA4f_unpremul'>unpremul()</a> <a href='#SkRGBA4f_unpremul'>const</a>;
    <a href='#SkRGBA4f_unpremul'>uint32_t</a> <a href='#SkRGBA4f_toBytes_RGBA'>toBytes_RGBA</a>() <a href='#SkRGBA4f_toBytes_RGBA'>const</a>;
    <a href='#SkRGBA4f_toBytes_RGBA'>static</a> <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_FromBytes_RGBA'>FromBytes_RGBA</a>(<a href='#SkRGBA4f_FromBytes_RGBA'>uint32_t</a> <a href='SkColor_Reference#Color'>color</a>);
    <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_makeOpaque'>makeOpaque</a>() <a href='#SkRGBA4f_makeOpaque'>const</a>;
};
</pre>

Each component is stored as a 32-bit single precision floating <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>float</a> <a href='SkPoint_Reference#Point'>value</a>.
<a href='SkPoint_Reference#Point'>All</a> <a href='SkPoint_Reference#Point'>values</a> <a href='SkPoint_Reference#Point'>are</a> <a href='SkPoint_Reference#Point'>allowed</a>, <a href='SkPoint_Reference#Point'>but</a> <a href='SkPoint_Reference#Point'>only</a> <a href='SkPoint_Reference#Point'>the</a> <a href='SkPoint_Reference#Point'>range</a> <a href='SkPoint_Reference#Point'>from</a> <a href='SkPoint_Reference#Point'>zero</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>one</a> <a href='SkPoint_Reference#Point'>is</a> <a href='SkPoint_Reference#Point'>meaningful</a>.

<a href='SkPoint_Reference#Point'>Components</a> <a href='SkPoint_Reference#Point'>are</a> <a href='SkPoint_Reference#Point'>independent</a> <a href='SkPoint_Reference#Point'>of</a> <a href='SkPoint_Reference#Point'>the</a> <a href='SkPoint_Reference#Point'>others</a> <a href='SkPoint_Reference#Point'>if</a> <a href='SkPoint_Reference#Point'>defined</a> <a href='SkPoint_Reference#Point'>with</a> <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>;
<a href='#SkRGBA4f_fA'>fA</a> <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>may</a> <a href='SkColor_Reference#Alpha'>be</a> <a href='SkColor_Reference#Alpha'>greater</a> <a href='SkColor_Reference#Alpha'>or</a> <a href='SkColor_Reference#Alpha'>smaller</a> <a href='SkColor_Reference#Alpha'>than</a> <a href='#SkRGBA4f_fG'>fG</a> <a href='#SkRGBA4f_fG'>green</a>, <a href='#SkRGBA4f_fB'>fB</a> <a href='#SkRGBA4f_fB'>blue</a>, <a href='#SkRGBA4f_fB'>or</a> <a href='#SkRGBA4f_fR'>fR</a> <a href='#SkRGBA4f_fR'>red</a>.
<a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a> <a href='SkColor4f_Reference#SkColor4f'>is</a> <a href='SkColor4f_Reference#SkColor4f'>shorthand</a> <a href='SkColor4f_Reference#SkColor4f'>for</a> <a href='undocumented#Unpremultiply'>Unpremultiplied</a> <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a>.

<a href='SkColor4f_Reference#SkRGBA4f'>Components</a> <a href='SkColor4f_Reference#SkRGBA4f'>are</a> <a href='SkColor4f_Reference#SkRGBA4f'>connected</a> <a href='SkColor4f_Reference#SkRGBA4f'>if</a> <a href='SkColor4f_Reference#SkRGBA4f'>defined</a> <a href='SkColor4f_Reference#SkRGBA4f'>with</a> <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>;
<a href='#SkRGBA4f_fA'>fA</a> <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>equal</a> <a href='SkColor_Reference#Alpha'>to</a> <a href='SkColor_Reference#Alpha'>or</a> <a href='SkColor_Reference#Alpha'>larger</a> <a href='SkColor_Reference#Alpha'>than</a> <a href='#SkRGBA4f_fG'>fG</a> <a href='#SkRGBA4f_fG'>green</a>, <a href='#SkRGBA4f_fB'>fB</a> <a href='#SkRGBA4f_fB'>blue</a>, <a href='#SkRGBA4f_fB'>and</a> <a href='#SkRGBA4f_fR'>fR</a> <a href='#SkRGBA4f_fR'>red</a>. <a href='#SkRGBA4f_fR'>The</a> <a href='#SkRGBA4f_fR'>values</a>
<a href='#SkRGBA4f_fR'>stored</a> <a href='#SkRGBA4f_fR'>in</a> <a href='#SkRGBA4f_fG'>fG</a>, <a href='#SkRGBA4f_fB'>fB</a>, <a href='#SkRGBA4f_fB'>and</a> <a href='#SkRGBA4f_fR'>fR</a> <a href='#SkRGBA4f_fR'>combine</a> <a href='#SkRGBA4f_fR'>the</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>component</a> <a href='SkColor_Reference#Color'>with</a> <a href='SkColor_Reference#Color'>the</a> <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>component</a>.

<a href='SkColor_Reference#Alpha'>Values</a> <a href='SkColor_Reference#Alpha'>smaller</a> <a href='SkColor_Reference#Alpha'>than</a> <a href='SkColor_Reference#Alpha'>zero</a> <a href='SkColor_Reference#Alpha'>or</a> <a href='SkColor_Reference#Alpha'>larger</a> <a href='SkColor_Reference#Alpha'>than</a> <a href='SkColor_Reference#Alpha'>one</a> <a href='SkColor_Reference#Alpha'>are</a> <a href='SkColor_Reference#Alpha'>allowed</a>. <a href='SkColor_Reference#Alpha'>Values</a> <a href='SkColor_Reference#Alpha'>out</a> <a href='SkColor_Reference#Alpha'>of</a> <a href='SkColor_Reference#Alpha'>range</a>
<a href='SkColor_Reference#Alpha'>may</a> <a href='SkColor_Reference#Alpha'>be</a> <a href='SkColor_Reference#Alpha'>used</a> <a href='SkColor_Reference#Alpha'>with</a> <a href='#Blend_Mode'>Blend_Mode</a> <a href='#Blend_Mode'>so</a> <a href='#Blend_Mode'>that</a> <a href='#Blend_Mode'>the</a> <a href='#Blend_Mode'>final</a> <a href='#Blend_Mode'>component</a> <a href='#Blend_Mode'>is</a> <a href='#Blend_Mode'>in</a> <a href='#Blend_Mode'>range</a>.<table style='border-collapse: collapse; width: 62.5em'>

  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Member</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>float</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRGBA4f_fR'><code>fR</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Single precision float for red ranges from no red (0.0) to full red (1.0).
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>float</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRGBA4f_fG'><code>fG</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Single precision float for green ranges from no green (0.0) to full green (1.0).
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>float</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRGBA4f_fB'><code>fB</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Single precision float for blue ranges from no blue (0.0) to full blue (1.0).
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>float</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRGBA4f_fA'><code>fA</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Single precision float for <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>ranges</a> <a href='SkColor_Reference#Alpha'>from</a> <a href='SkColor_Reference#Alpha'>no</a> <a href='SkColor_Reference#Alpha'>Alpha</a> (0.0) <a href='SkColor_Reference#Alpha'>to</a> <a href='SkColor_Reference#Alpha'>full</a> <a href='SkColor_Reference#Alpha'>Alpha</a> (1.0).
</td>
  </tr>
</table>

<a name='SkRGBA4f_equal1_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator==(const <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a>& <a href='SkColor4f_Reference#SkRGBA4f'>other</a>) <a href='SkColor4f_Reference#SkRGBA4f'>const</a>
</pre>

Compares <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='SkColor4f_Reference#SkRGBA4f'>with</a> <a href='#SkRGBA4f_equal1_operator_other'>other</a>, <a href='#SkRGBA4f_equal1_operator_other'>and</a> <a href='#SkRGBA4f_equal1_operator_other'>returns</a> <a href='#SkRGBA4f_equal1_operator_other'>true</a> <a href='#SkRGBA4f_equal1_operator_other'>if</a> <a href='#SkRGBA4f_equal1_operator_other'>all</a> <a href='#SkRGBA4f_equal1_operator_other'>components</a> <a href='#SkRGBA4f_equal1_operator_other'>are</a> <a href='#SkRGBA4f_equal1_operator_other'>equivalent</a>.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_equal1_operator_other'><code><strong>other</strong></code></a></td>
    <td><a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='SkColor4f_Reference#SkRGBA4f'>to</a> <a href='SkColor4f_Reference#SkRGBA4f'>compare</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='SkColor4f_Reference#SkRGBA4f'>equals</a> <a href='#SkRGBA4f_equal1_operator_other'>other</a>

### Example

<div><fiddle-embed name="e5b34bcb7f80f2ed890cdacaa059db0d">

#### Example Output

~~~~
colorRed == colorNamedRed
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRGBA4f_notequal1_operator'>operator!=(const SkRGBA4f& other)_const</a>

<a name='SkRGBA4f_notequal1_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator!=(const <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a>& <a href='SkColor4f_Reference#SkRGBA4f'>other</a>) <a href='SkColor4f_Reference#SkRGBA4f'>const</a>
</pre>

Compares <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='SkColor4f_Reference#SkRGBA4f'>with</a> <a href='#SkRGBA4f_notequal1_operator_other'>other</a>, <a href='#SkRGBA4f_notequal1_operator_other'>and</a> <a href='#SkRGBA4f_notequal1_operator_other'>returns</a> <a href='#SkRGBA4f_notequal1_operator_other'>true</a> <a href='#SkRGBA4f_notequal1_operator_other'>if</a> <a href='#SkRGBA4f_notequal1_operator_other'>all</a> <a href='#SkRGBA4f_notequal1_operator_other'>components</a> <a href='#SkRGBA4f_notequal1_operator_other'>are</a> <a href='#SkRGBA4f_notequal1_operator_other'>not</a>
<a href='#SkRGBA4f_notequal1_operator_other'>equivalent</a>.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_notequal1_operator_other'><code><strong>other</strong></code></a></td>
    <td><a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='SkColor4f_Reference#SkRGBA4f'>to</a> <a href='SkColor4f_Reference#SkRGBA4f'>compare</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='SkColor4f_Reference#SkRGBA4f'>is</a> <a href='SkColor4f_Reference#SkRGBA4f'>not</a> <a href='SkColor4f_Reference#SkRGBA4f'>equal</a> <a href='SkColor4f_Reference#SkRGBA4f'>to</a> <a href='#SkRGBA4f_notequal1_operator_other'>other</a>

### Example

<div><fiddle-embed name="82f1a9b4c2b27aa547061786d1f33dab">

#### Example Output

~~~~
colorGray != colorNamedGray
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRGBA4f_equal1_operator'>operator==(const SkRGBA4f& other)_const</a>

<a name='SkRGBA4f_multiply_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='SkColor4f_Reference#SkRGBA4f'>operator</a>*(<a href='SkColor4f_Reference#SkRGBA4f'>float</a> <a href='SkColor4f_Reference#SkRGBA4f'>scale</a>) <a href='SkColor4f_Reference#SkRGBA4f'>const</a>
</pre>

Multiplies each component by <a href='#SkRGBA4f_multiply_operator_scale'>scale</a>. <a href='#SkRGBA4f_multiply_operator_scale'>Does</a> <a href='#SkRGBA4f_multiply_operator_scale'>not</a> <a href='#SkRGBA4f_multiply_operator_scale'>pin</a> <a href='#SkRGBA4f_multiply_operator_scale'>the</a> <a href='#SkRGBA4f_multiply_operator_scale'>result</a>.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_multiply_operator_scale'><code><strong>scale</strong></code></a></td>
    <td>component multiplier</td>
  </tr>
</table>

### Return Value

scaled <a href='SkColor_Reference#Color'>color</a>

### See Also

<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kMultiply'>kMultiply</a>

<a name='SkRGBA4f_multiply1_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='SkColor4f_Reference#SkRGBA4f'>operator</a>*(<a href='SkColor4f_Reference#SkRGBA4f'>const</a> <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a>& <a href='SkColor4f_Reference#SkRGBA4f'>scale</a>) <a href='SkColor4f_Reference#SkRGBA4f'>const</a>
</pre>

Multiplies each component by <a href='#SkRGBA4f_multiply1_operator_scale'>scale</a> <a href='#SkRGBA4f_multiply1_operator_scale'>component</a>. <a href='#SkRGBA4f_multiply1_operator_scale'>Does</a> <a href='#SkRGBA4f_multiply1_operator_scale'>not</a> <a href='#SkRGBA4f_multiply1_operator_scale'>pin</a> <a href='#SkRGBA4f_multiply1_operator_scale'>the</a> <a href='#SkRGBA4f_multiply1_operator_scale'>result</a>.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_multiply1_operator_scale'><code><strong>scale</strong></code></a></td>
    <td><a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='SkColor4f_Reference#SkRGBA4f'>component</a> <a href='SkColor4f_Reference#SkRGBA4f'>multipliers</a></td>
  </tr>
</table>

### Return Value

scaled <a href='SkColor_Reference#Color'>color</a>

### See Also

<a href='SkBlendMode_Reference#SkBlendMode'>SkBlendMode</a>::<a href='#SkBlendMode_kMultiply'>kMultiply</a>

<a name='Property_Functions'></a>

<a name='SkRGBA4f_vec'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const float* <a href='#SkRGBA4f_vec'>vec()</a> <a href='#SkRGBA4f_vec'>const</a>
</pre>

Returns <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='SkColor4f_Reference#SkRGBA4f'>components</a> <a href='SkColor4f_Reference#SkRGBA4f'>as</a> <a href='SkColor4f_Reference#SkRGBA4f'>a</a> <a href='SkColor4f_Reference#SkRGBA4f'>read-only</a> <a href='SkColor4f_Reference#SkRGBA4f'>array</a>.

### Return Value

components as read-only array

### Example

<div><fiddle-embed name="229057023515224358a36acf15508cf6">

#### Example Output

~~~~
red=0.266667 green=0.533333 blue=0.8 alpha=0.533333
[0]=0.266667 [1]=0.533333 [2]=0.8 [3]=0.533333
~~~~

</fiddle-embed></div>

### See Also

<a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a>

<a name='SkRGBA4f_vec_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
float* <a href='#SkRGBA4f_vec'>vec()</a>
</pre>

Returns <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='SkColor4f_Reference#SkRGBA4f'>components</a> <a href='SkColor4f_Reference#SkRGBA4f'>as</a> <a href='SkColor4f_Reference#SkRGBA4f'>a</a> <a href='SkColor4f_Reference#SkRGBA4f'>writable</a> <a href='SkColor4f_Reference#SkRGBA4f'>array</a>.

### Return Value

components as writable array

### Example

<div><fiddle-embed name="7420bf0a7cae5c6577c4c4a4613e7e7e">

#### Example Output

~~~~
red=0.266667 green=0.533333 blue=0.8 alpha=0.533333
[0]=0.266667 [1]=0.533333 [2]=0.8 [3]=1
~~~~

</fiddle-embed></div>

### See Also

<a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a>

<a name='SkRGBA4f_array_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
float operator[](int index) const
</pre>

Returns <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='SkColor4f_Reference#SkRGBA4f'>component</a> <a href='SkColor4f_Reference#SkRGBA4f'>by</a> <a href='SkColor4f_Reference#SkRGBA4f'>index</a>, <a href='SkColor4f_Reference#SkRGBA4f'>zero</a> <a href='SkColor4f_Reference#SkRGBA4f'>through</a> <a href='SkColor4f_Reference#SkRGBA4f'>three</a>. <a href='SkColor4f_Reference#SkRGBA4f'>index</a> <a href='SkColor4f_Reference#SkRGBA4f'>out</a> <a href='SkColor4f_Reference#SkRGBA4f'>of</a> <a href='SkColor4f_Reference#SkRGBA4f'>range</a>
<a href='SkColor4f_Reference#SkRGBA4f'>triggers</a> <a href='SkColor4f_Reference#SkRGBA4f'>an</a> <a href='SkColor4f_Reference#SkRGBA4f'>assert</a> <a href='SkColor4f_Reference#SkRGBA4f'>in</a> <a href='SkColor4f_Reference#SkRGBA4f'>debug</a> <a href='SkColor4f_Reference#SkRGBA4f'>builds</a>.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_array_operator_index'><code><strong>index</strong></code></a></td>
    <td>component, zero through three</td>
  </tr>
</table>

### Return Value

component by index

### See Also

<a href='#SkRGBA4f_vec'>vec</a>

<a name='SkRGBA4f_array1_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
float& operator[](int index)
</pre>

Returns writable component reference by index, zero through three. index out of range
triggers an assert in debug builds.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_array1_operator_index'><code><strong>index</strong></code></a></td>
    <td>component, zero through three</td>
  </tr>
</table>

### Return Value

writable component reference by index

### See Also

<a href='#SkRGBA4f_vec'>vec</a>

<a name='Utility_Functions'></a>

<a name='SkRGBA4f_isOpaque'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRGBA4f_isOpaque'>isOpaque</a>() <a href='#SkRGBA4f_isOpaque'>const</a>
</pre>

Returns true if <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>component</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>one</a>. <a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>has</a> <a href='SkColor_Reference#Color'>no</a> <a href='SkColor_Reference#Color'>transparency</a> <a href='SkColor_Reference#Color'>regardless</a> <a href='SkColor_Reference#Color'>of</a>
<a href='SkColor_Reference#Color'>whether</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>is</a> <a href='undocumented#Premultiply'>Premultiplied</a> <a href='undocumented#Premultiply'>or</a> <a href='undocumented#Unpremultiply'>Unpremultiplied</a>. <a href='undocumented#Unpremultiply'>Triggers</a> <a href='undocumented#Unpremultiply'>a</a> <a href='undocumented#Debugging'>debugging</a> <a href='undocumented#Debugging'>assert</a>
<a href='undocumented#Debugging'>if</a> <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>not</a> <a href='SkColor_Reference#Alpha'>valid</a>.

### Return Value

true if <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>is</a> <a href='SkColor_Reference#Alpha'>one</a>

### See Also

<a href='#SkRGBA4f_vec'>vec</a> <a href='SkColor_Reference#SkColorGetA'>SkColorGetA</a>

<a name='SkRGBA4f_FromColor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_FromColor'>FromColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a>)
</pre>

Converts to closest <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a>.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_FromColor_SkColor'><code><strong>SkColor</strong></code></a></td>
    <td><a href='SkColor_Reference#Color'>Color</a> <a href='SkColor_Reference#Color'>with</a> <a href='SkColor_Reference#Alpha'>Alpha</a>, <a href='SkColor_Reference#Alpha'>red</a>, <a href='SkColor_Reference#Alpha'>blue</a>, <a href='SkColor_Reference#Alpha'>and</a> <a href='SkColor_Reference#Alpha'>green</a> <a href='SkColor_Reference#Alpha'>components</a></td>
  </tr>
</table>

### Return Value

<a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='SkColor4f_Reference#SkRGBA4f'>equivalent</a>

### Example

<div><fiddle-embed name="33b029064e8d1928e42a587c953d0e4e">

#### Example Output

~~~~
red=0.301961 green=0.396078 blue=0.6 alpha=0.168627
red=77 green=101 blue=153 alpha=43
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRGBA4f_toSkColor'>toSkColor</a>

<a name='SkRGBA4f_toSkColor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkRGBA4f_toSkColor'>toSkColor</a>() <a href='#SkRGBA4f_toSkColor'>const</a>
</pre>

Converts to closest <a href='SkColor_Reference#SkColor'>SkColor</a>.

### Return Value

closest <a href='SkColor_Reference#Color'>Color</a>

### Example

<div><fiddle-embed name="edc5fd18d961f7607d2bcbf7f7d427e5">

#### Example Output

~~~~
red=18 green=33 blue=82 alpha=43
red=0.0705882 green=0.129412 blue=0.321569 alpha=0.168627
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRGBA4f_FromColor'>FromColor</a>

<a name='SkRGBA4f_FromPMColor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_FromPMColor'>FromPMColor</a>(<a href='SkColor_Reference#SkPMColor'>SkPMColor</a>)
</pre>

Converts from <a href='undocumented#Premultiply'>Premultiplied</a> <a href='undocumented#Premultiply'>integer</a> <a href='undocumented#Premultiply'>components</a> <a href='undocumented#Premultiply'>to</a> <a href='undocumented#Unpremultiply'>Unpremultiplied</a> <a href='undocumented#Unpremultiply'>float</a>
<a href='undocumented#Unpremultiply'>components</a>.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_FromPMColor_SkPMColor'><code><strong>SkPMColor</strong></code></a></td>
    <td><a href='undocumented#Premultiply'>Premultiplied</a> <a href='SkColor_Reference#Color'>color</a></td>
  </tr>
</table>

### Return Value

<a href='undocumented#Unpremultiply'>Unpremultiplied</a> <a href='SkColor_Reference#Color'>color</a>

### See Also

<a href='#SkRGBA4f_FromColor'>FromColor</a>

<a name='SkRGBA4f_premul'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a>&<a href='SkColor4f_Reference#SkRGBA4f'>lt</a>;<a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>&<a href='SkImageInfo_Reference#kPremul_SkAlphaType'>gt</a>; <a href='#SkRGBA4f_premul'>premul()</a> <a href='#SkRGBA4f_premul'>const</a>
</pre>

Returns <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a> <a href='SkColor4f_Reference#SkColor4f'>with</a> <a href='SkColor4f_Reference#SkColor4f'>all</a> <a href='SkColor4f_Reference#SkColor4f'>components</a> <a href='undocumented#Premultiply'>premultiplied</a> <a href='undocumented#Premultiply'>by</a> <a href='SkColor_Reference#Alpha'>Alpha</a>.

### Return Value

<a href='undocumented#Premultiply'>Premultiplied</a> <a href='SkColor_Reference#Color'>color</a>

### See Also

<a href='#SkRGBA4f_unpremul'>unpremul</a>

<a name='SkRGBA4f_unpremul'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a>&<a href='SkColor4f_Reference#SkRGBA4f'>lt</a>;<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>&<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>gt</a>; <a href='#SkRGBA4f_unpremul'>unpremul()</a> <a href='#SkRGBA4f_unpremul'>const</a>
</pre>

Returns <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='SkColor4f_Reference#SkRGBA4f'>with</a> <a href='SkColor4f_Reference#SkRGBA4f'>all</a> <a href='SkColor4f_Reference#SkRGBA4f'>components</a> <a href='SkColor4f_Reference#SkRGBA4f'>independent</a> <a href='SkColor4f_Reference#SkRGBA4f'>of</a> <a href='SkColor_Reference#Alpha'>Alpha</a>.

### Return Value

<a href='undocumented#Unpremultiply'>Unpremultiplied</a> <a href='SkColor_Reference#Color'>color</a>

### See Also

<a href='#SkRGBA4f_premul'>premul</a>

<a name='SkRGBA4f_toBytes_RGBA'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkRGBA4f_toBytes_RGBA'>toBytes_RGBA</a>() <a href='#SkRGBA4f_toBytes_RGBA'>const</a>
</pre>

Produces bytes in RGBA order. Component values are not affected by <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Alpha'>Alpha</a>.

### Return Value

<a href='SkColor_Reference#Color'>color</a>

<a name='SkRGBA4f_FromBytes_RGBA'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_FromBytes_RGBA'>FromBytes_RGBA</a>(<a href='#SkRGBA4f_FromBytes_RGBA'>uint32_t</a> <a href='SkColor_Reference#Color'>color</a>)
</pre>

Returns from <a href='SkColor_Reference#Color'>color</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>kRGBA_8888_SkColorType</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>order</a>. <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>Component</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>values</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>are</a>
<a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>not</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>affected</a> <a href='SkImageInfo_Reference#kRGBA_8888_SkColorType'>by</a> <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Alpha'>Alpha</a>.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_FromBytes_RGBA_color'><code><strong>color</strong></code></a></td>
    <td><a href='undocumented#Premultiply'>Premultiplied</a> <a href='undocumented#Premultiply'>or</a> <a href='undocumented#Unpremultiply'>Unpremultiplied</a></td>
  </tr>
</table>

### Return Value

<a href='SkColor_Reference#Color'>color</a>

<a name='SkRGBA4f_makeOpaque'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_makeOpaque'>makeOpaque</a>() <a href='#SkRGBA4f_makeOpaque'>const</a>
</pre>

Returns <a href='SkColor_Reference#Color'>color</a> <a href='SkColor_Reference#Color'>with</a> <a href='SkColor_Reference#Alpha'>Alpha</a> <a href='SkColor_Reference#Alpha'>set</a> <a href='SkColor_Reference#Alpha'>to</a> <a href='SkColor_Reference#Alpha'>one</a>.

### Return Value

<a href='SkColor_Reference#Color'>color</a>

<a name='SkColor4f'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
using <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a> = <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a><<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>>;
</pre>

