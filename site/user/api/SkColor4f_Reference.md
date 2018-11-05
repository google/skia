SkColor4f Reference
===


<a name='SkPM4f'></a>

---

<a name='SkRGBA4f'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
struct <a href='#SkRGBA4f'>SkRGBA4f</a> {
    float <a href='#SkRGBA4f_fR'>fR</a>;
    float <a href='#SkRGBA4f_fG'>fG</a>;
    float <a href='#SkRGBA4f_fB'>fB</a>;
    float <a href='#SkRGBA4f_fA'>fA</a>;

    bool <a href='#SkRGBA4f_equal1_operator'>operator==(const SkRGBA4f& other)_const</a>;
    bool <a href='#SkRGBA4f_notequal1_operator'>operator!=(const SkRGBA4f& other)_const</a>;
    <a href='#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_multiply_operator'>operator*(float scale)_const</a>;
    <a href='#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_multiply1_operator'>operator*(const SkRGBA4f& scale)_const</a>;
    const float* <a href='#SkRGBA4f_vec'>vec</a>() const;
    float* <a href='#SkRGBA4f_vec'>vec</a>();
    float <a href='#SkRGBA4f_array_operator'>operator[](int index)_const</a>;
    float& <a href='#SkRGBA4f_array1_operator'>operator[](int index)</a>;
    bool <a href='#SkRGBA4f_isOpaque'>isOpaque</a>() const;
    static <a href='#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_FromColor'>FromColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a>);
    <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkRGBA4f_toSkColor'>toSkColor</a>() const;
    static <a href='#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_FromPMColor'>FromPMColor</a>(<a href='SkColor_Reference#SkPMColor'>SkPMColor</a>);
    <a href='#SkRGBA4f'>SkRGBA4f</a><<a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>> <a href='#SkRGBA4f_premul'>premul</a>() const;
    <a href='#SkRGBA4f'>SkRGBA4f</a><<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>> <a href='#SkRGBA4f_unpremul'>unpremul</a>() const;
    uint32_t <a href='#SkRGBA4f_toBytes_RGBA'>toBytes_RGBA</a>() const;
    static <a href='#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_FromBytes_RGBA'>FromBytes_RGBA</a>(uint32_t color);
    <a href='#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_makeOpaque'>makeOpaque</a>() const;
};
</pre>

Each component is stored as a 32-bit single precision floating <a href='SkPoint_Reference#Point'>point</a> float value.
All values are allowed, but only the range from zero to one is meaningful.

Components are independent of the others if defined with <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>;
<a href='#SkRGBA4f_fA'>fA</a> <a href='SkColor_Reference#Alpha'>Alpha</a> is may be greater or smaller than <a href='#SkRGBA4f_fG'>fG</a> green, <a href='#SkRGBA4f_fB'>fB</a> blue, or <a href='#SkRGBA4f_fR'>fR</a> red.
<a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a> is shorthand for <a href='undocumented#Unpremultiply'>Unpremultiplied</a> <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a>.

Components are connected if defined with <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>;
<a href='#SkRGBA4f_fA'>fA</a> <a href='SkColor_Reference#Alpha'>Alpha</a> is equal to or larger than <a href='#SkRGBA4f_fG'>fG</a> green, <a href='#SkRGBA4f_fB'>fB</a> blue, and <a href='#SkRGBA4f_fR'>fR</a> red. The values
stored in <a href='#SkRGBA4f_fG'>fG</a>, <a href='#SkRGBA4f_fB'>fB</a>, and <a href='#SkRGBA4f_fR'>fR</a> combine the <a href='SkColor_Reference#Color'>color</a> component with the <a href='SkColor_Reference#Alpha'>Alpha</a> component.

Values smaller than zero or larger than one are allowed. Values out of range
may be used with <a href='#Blend_Mode'>Blend_Mode</a> so that the final component is in range.<table style='border-collapse: collapse; width: 62.5em'>

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
Single precision float for <a href='SkColor_Reference#Alpha'>Alpha</a> ranges from no <a href='SkColor_Reference#Alpha'>Alpha</a> (0.0) to full <a href='SkColor_Reference#Alpha'>Alpha</a> (1.0).
</td>
  </tr>
</table>

<a name='SkRGBA4f_equal1_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator==(const <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a>& other) const
</pre>

Compares <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> with <a href='#SkRGBA4f_equal1_operator_other'>other</a>, and returns true if all components are equivalent.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_equal1_operator_other'><code><strong>other</strong></code></a></td>
    <td>SkRGBA4f to compare</td>
  </tr>
</table>

### Return Value

true if SkRGBA4f equals other

### Example

<div><fiddle-embed name="e5b34bcb7f80f2ed890cdacaa059db0d">

#### Example Output

~~~~
colorRed == colorNamedRed
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRGBA4f_notequal1_operator'>operator!=(const SkRGBA4f& other) const</a>

<a name='SkRGBA4f_notequal1_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator!=(const <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a>& other) const
</pre>

Compares <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> with <a href='#SkRGBA4f_notequal1_operator_other'>other</a>, and returns true if all components are not
equivalent.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_notequal1_operator_other'><code><strong>other</strong></code></a></td>
    <td>SkRGBA4f to compare</td>
  </tr>
</table>

### Return Value

true if SkRGBA4f is not equal to other

### Example

<div><fiddle-embed name="82f1a9b4c2b27aa547061786d1f33dab">

#### Example Output

~~~~
colorGray != colorNamedGray
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRGBA4f_equal1_operator'>operator==(const SkRGBA4f& other) const</a>

<a name='SkRGBA4f_multiply_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> operator*(float scale) const
</pre>

Multiplies each component by <a href='#SkRGBA4f_multiply_operator_scale'>scale</a>. Does not pin the result.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_multiply_operator_scale'><code><strong>scale</strong></code></a></td>
    <td>component multiplier</td>
  </tr>
</table>

### Return Value

scaled color

### See Also

<a href='SkBlendMode_Reference#SkBlendMode_kMultiply'>SkBlendMode::kMultiply</a>

<a name='SkRGBA4f_multiply1_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> operator*(const <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a>& scale) const
</pre>

Multiplies each component by <a href='#SkRGBA4f_multiply1_operator_scale'>scale</a> component. Does not pin the result.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_multiply1_operator_scale'><code><strong>scale</strong></code></a></td>
    <td>SkRGBA4f component multipliers</td>
  </tr>
</table>

### Return Value

scaled color

### See Also

<a href='SkBlendMode_Reference#SkBlendMode_kMultiply'>SkBlendMode::kMultiply</a>

<a name='Property_Functions'></a>

<a name='SkRGBA4f_vec'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const float* <a href='#SkRGBA4f_vec'>vec()</a> const
</pre>

Returns SkRGBA4f components as a read-only array.

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

<a href='#SkColor4f'>SkColor4f</a>

<a name='SkRGBA4f_vec_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
float* vec()
</pre>

Returns SkRGBA4f components as a writable array.

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

<a href='#SkColor4f'>SkColor4f</a>

<a name='SkRGBA4f_array_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
float operator[](int index) const
</pre>

Returns SkRGBA4f component by index, zero through three. index out of range
triggers an assert in debug builds.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_array_operator_index'><code><strong>index</strong></code></a></td>
    <td>component, zero through three</td>
  </tr>
</table>

### Return Value

component by index

### See Also

<a href='#SkRGBA4f_vec'>vec</a><sup><a href='#SkRGBA4f_vec_2'>[2]</a></sup>

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

<a href='#SkRGBA4f_vec'>vec</a><sup><a href='#SkRGBA4f_vec_2'>[2]</a></sup>

<a name='Utility_Functions'></a>

<a name='SkRGBA4f_isOpaque'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRGBA4f_isOpaque'>isOpaque</a>() const
</pre>

Returns true if Alpha component is one. Color has no transparency regardless of
whether color is Premultiplied or Unpremultiplied. Triggers a debugging assert
if Alpha not valid.

### Return Value

true if Alpha is one

### See Also

<a href='#SkRGBA4f_vec'>vec</a><sup><a href='#SkRGBA4f_vec_2'>[2]</a></sup> <a href='SkColor_Reference#SkColorGetA'>SkColorGetA</a>

<a name='SkRGBA4f_FromColor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_FromColor'>FromColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a>)
</pre>

Converts to closest SkRGBA4f.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_FromColor_SkColor'><code><strong>SkColor</strong></code></a></td>
    <td>Color with Alpha, red, blue, and green components</td>
  </tr>
</table>

### Return Value

SkRGBA4f equivalent

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
<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkRGBA4f_toSkColor'>toSkColor</a>() const
</pre>

Converts to closest SkColor.

### Return Value

closest Color

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

Converts from Premultiplied integer components to Unpremultiplied float
components.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_FromPMColor_SkPMColor'><code><strong>SkPMColor</strong></code></a></td>
    <td>Premultiplied color</td>
  </tr>
</table>

### Return Value

Unpremultiplied color

### See Also

<a href='#SkRGBA4f_FromColor'>FromColor</a>

<a name='SkRGBA4f_premul'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a>&lt;<a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>&gt; <a href='#SkRGBA4f_premul'>premul()</a> const
</pre>

Returns SkColor4f with all components premultiplied by Alpha.

### Return Value

Premultiplied color

### See Also

<a href='#SkRGBA4f_unpremul'>unpremul</a>

<a name='SkRGBA4f_unpremul'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a>&lt;<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>&gt; <a href='#SkRGBA4f_unpremul'>unpremul()</a> const
</pre>

Returns SkRGBA4f with all components independent of Alpha.

### Return Value

Unpremultiplied color

### See Also

<a href='#SkRGBA4f_premul'>premul</a>

<a name='SkRGBA4f_toBytes_RGBA'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkRGBA4f_toBytes_RGBA'>toBytes_RGBA</a>() const
</pre>

Produces bytes in RGBA order. Component values are not affected by color Alpha.

### Return Value

color

<a name='SkRGBA4f_FromBytes_RGBA'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_FromBytes_RGBA'>FromBytes_RGBA</a>(uint32_t <a href='SkColor_Reference#Color'>color</a>)
</pre>

Returns from color kRGBA_8888_SkColorType order. Component values are
not affected by color Alpha.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_FromBytes_RGBA_color'><code><strong>color</strong></code></a></td>
    <td>Premultiplied or Unpremultiplied</td>
  </tr>
</table>

### Return Value

color

<a name='SkRGBA4f_makeOpaque'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor4f_Reference#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_makeOpaque'>makeOpaque</a>() const
</pre>

Returns color with Alpha set to one.

### Return Value

color

<a name='SkColor4f'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
using <a href='#SkColor4f'>SkColor4f</a> = <a href='#SkRGBA4f'>SkRGBA4f</a><<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>>;
</pre>

