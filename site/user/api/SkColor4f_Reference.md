SkColor4f Reference
===

<a name='SkPM4f'></a>
<a name='SkRGBA4f'></a>

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
    static <a href='#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_Pin'>Pin</a>(float r, float g, float b, float a);
    <a href='#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_pin'>pin</a>() const;
    static <a href='#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_FromColor'>FromColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a>);
    <a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkRGBA4f_toSkColor'>toSkColor</a>() const;
    static <a href='#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_FromPMColor'>FromPMColor</a>(<a href='SkColor_Reference#SkPMColor'>SkPMColor</a>);
    <a href='#SkRGBA4f'>SkRGBA4f</a><<a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>> <a href='#SkRGBA4f_premul'>premul</a>() const;
    <a href='#SkRGBA4f'>SkRGBA4f</a><<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>> <a href='#SkRGBA4f_unpremul'>unpremul</a>() const;
};
</pre>

Each component is stored as a 32-bit single precision floating point float value.
All values are allowed, but only the range from zero to one is meaningful.

Components are independent of the others if defined with <a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>;
<a href='#SkRGBA4f_fA'>fA</a> <a href='SkColor_Reference#Alpha'>Alpha</a> is may be greater or smaller than <a href='#SkRGBA4f_fG'>fG</a> green, <a href='#SkRGBA4f_fB'>fB</a> blue, or <a href='#SkRGBA4f_fR'>fR</a> red.
<a href='#SkColor4f'>SkColor4f</a> is shorthand for <a href='undocumented#Unpremultiply'>Unpremultiplied</a> <a href='#SkRGBA4f'>SkRGBA4f</a>.

Components are connnected if defined with <a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>;
<a href='#SkRGBA4f_fA'>fA</a> <a href='SkColor_Reference#Alpha'>Alpha</a> is equal to or larger than <a href='#SkRGBA4f_fG'>fG</a> green, <a href='#SkRGBA4f_fB'>fB</a> blue, and <a href='#SkRGBA4f_fR'>fR</a> red. The values
stored in <a href='#SkRGBA4f_fG'>fG</a>, <a href='#SkRGBA4f_fB'>fB</a>, and <a href='#SkRGBA4f_fR'>fR</a> combine the color component with the <a href='SkColor_Reference#Alpha'>Alpha</a> component.

Values smaller than zero or larger than one are allowed. Values out of range
may be used with <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a> so that the final component is in range.<table style='border-collapse: collapse; width: 62.5em'>

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
## operator==

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator==(const SkRGBA4f& other) const
</pre>

Compares <a href='#SkRGBA4f'>SkRGBA4f</a> with <a href='#SkRGBA4f_equal1_operator_other'>other</a>, and returns true if all components are equivalent.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_equal1_operator_other'><code><strong>other</strong></code></a></td>
    <td><a href='#SkRGBA4f'>SkRGBA4f</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href='#SkRGBA4f'>SkRGBA4f</a> equals <a href='#SkRGBA4f_equal1_operator_other'>other</a>

### Example

<div><fiddle-embed name="e5b34bcb7f80f2ed890cdacaa059db0d">

#### Example Output

~~~~
colorRed == colorNamedRed
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRGBA4f_notequal1_operator'>operator!=(const SkRGBA4f& other) const</a>

---

<a name='SkRGBA4f_notequal1_operator'></a>
## operator!=

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator!=(const SkRGBA4f& other) const
</pre>

Compares <a href='#SkRGBA4f'>SkRGBA4f</a> with <a href='#SkRGBA4f_notequal1_operator_other'>other</a>, and returns true if all components are not
equivalent.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_notequal1_operator_other'><code><strong>other</strong></code></a></td>
    <td><a href='#SkRGBA4f'>SkRGBA4f</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href='#SkRGBA4f'>SkRGBA4f</a> is not equal to <a href='#SkRGBA4f_notequal1_operator_other'>other</a>

### Example

<div><fiddle-embed name="82f1a9b4c2b27aa547061786d1f33dab">

#### Example Output

~~~~
colorGray != colorNamedGray
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRGBA4f_equal1_operator'>operator==(const SkRGBA4f& other) const</a>

---

<a name='SkRGBA4f_multiply_operator'></a>
## operator*

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkRGBA4f'>SkRGBA4f</a> operator*(float scale) const
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

---

<a name='SkRGBA4f_multiply1_operator'></a>
## operator*

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkRGBA4f'>SkRGBA4f</a> operator*(const SkRGBA4f& scale) const
</pre>

Multiplies each component by <a href='#SkRGBA4f_multiply1_operator_scale'>scale</a> component. Does not pin the result.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_multiply1_operator_scale'><code><strong>scale</strong></code></a></td>
    <td><a href='#SkRGBA4f'>SkRGBA4f</a> component multipliers</td>
  </tr>
</table>

### Return Value

scaled color

### See Also

<a href='SkBlendMode_Reference#SkBlendMode_kMultiply'>SkBlendMode::kMultiply</a>

---

## <a name='Property_Functions'>Property Functions</a>

<a name='SkRGBA4f_vec'></a>
## vec

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const float* <a href='#SkRGBA4f_vec'>vec</a>() const
</pre>

Returns <a href='#SkRGBA4f'>SkRGBA4f</a> components as a read-only array.

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

---

<a name='SkRGBA4f_vec_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
float* <a href='#SkRGBA4f_vec'>vec</a>()
</pre>

Returns <a href='#SkRGBA4f'>SkRGBA4f</a> components as a writable array.

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

---

<a name='SkRGBA4f_array_operator'></a>
## operator[]

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
float <a href='#SkRGBA4f_array1_operator'>operator[](int index)</a> const
</pre>

Returns <a href='#SkRGBA4f'>SkRGBA4f</a> component by <a href='#SkRGBA4f_array_operator_index'>index</a>, zero through three. <a href='#SkRGBA4f_array_operator_index'>index</a> out of range
triggers an assert in debug builds.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_array_operator_index'><code><strong>index</strong></code></a></td>
    <td>component, zero through three</td>
  </tr>
</table>

### Return Value

component by <a href='#SkRGBA4f_array_operator_index'>index</a>

### See Also

<a href='#SkRGBA4f_vec'>vec</a><sup><a href='#SkRGBA4f_vec_2'>[2]</a></sup>

---

<a name='SkRGBA4f_array1_operator'></a>
## operator[]

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
float& <a href='#SkRGBA4f_array1_operator'>operator[](int index)</a>
</pre>

Returns writable component reference by <a href='#SkRGBA4f_array1_operator_index'>index</a>, zero through three. <a href='#SkRGBA4f_array1_operator_index'>index</a> out of range
triggers an assert in debug builds.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_array1_operator_index'><code><strong>index</strong></code></a></td>
    <td>component, zero through three</td>
  </tr>
</table>

### Return Value

writable component reference by <a href='#SkRGBA4f_array1_operator_index'>index</a>

### See Also

<a href='#SkRGBA4f_vec'>vec</a><sup><a href='#SkRGBA4f_vec_2'>[2]</a></sup>

---

## <a name='Utility_Functions'>Utility Functions</a>

<a name='SkRGBA4f_isOpaque'></a>
## isOpaque

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRGBA4f_isOpaque'>isOpaque</a>() const
</pre>

Returns true if <a href='SkColor_Reference#Alpha'>Alpha</a> component is one. <a href='SkColor_Reference#Color'>Color</a> has no transparency regardless of
whether color is <a href='undocumented#Premultiply'>Premultiplied</a> or <a href='undocumented#Unpremultiply'>Unpremultiplied</a>. Triggers a debugging assert
if <a href='SkColor_Reference#Alpha'>Alpha</a> not valid.

### Return Value

true if <a href='SkColor_Reference#Alpha'>Alpha</a> is one

### See Also

<a href='#SkRGBA4f_vec'>vec</a><sup><a href='#SkRGBA4f_vec_2'>[2]</a></sup> <a href='SkColor_Reference#SkColorGetA'>SkColorGetA</a>

---

<a name='SkRGBA4f_Pin'></a>
## Pin

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_Pin'>Pin</a>(float r, float g, float b, float a)
</pre>

Constructs and returns <a href='#SkRGBA4f'>SkRGBA4f</a> with each component pinned from zero to one.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_Pin_r'><code><strong>r</strong></code></a></td>
    <td>red component</td>
  </tr>
  <tr>    <td><a name='SkRGBA4f_Pin_g'><code><strong>g</strong></code></a></td>
    <td>green component</td>
  </tr>
  <tr>    <td><a name='SkRGBA4f_Pin_b'><code><strong>b</strong></code></a></td>
    <td>blue component</td>
  </tr>
  <tr>    <td><a name='SkRGBA4f_Pin_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkColor_Reference#Alpha'>Alpha</a> component</td>
  </tr>
</table>

### Return Value

<a href='#Color4f'>Color4f</a> with valid components

### Example

<div><fiddle-embed name="c989cf16c7f8849874eb008cd701af76"></fiddle-embed></div>

### See Also

<a href='#SkRGBA4f_pin'>pin</a>() <a href='#SkRGBA4f_FromColor'>FromColor</a>

---

<a name='SkRGBA4f_pin'></a>
## pin

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_pin'>pin</a>() const
</pre>

Returns <a href='#SkRGBA4f'>SkRGBA4f</a> with all components in the range from zero to one.

### Return Value

<a href='#SkRGBA4f'>SkRGBA4f</a> with valid components

### Example

<div><fiddle-embed name="9e349862c5189a44d2acef5da24f2e79"></fiddle-embed></div>

### See Also

<a href='#SkRGBA4f_Pin'>Pin</a>

---

<a name='SkRGBA4f_FromColor'></a>
## FromColor

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_FromColor'>FromColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a>)
</pre>

Converts to closest <a href='#SkRGBA4f'>SkRGBA4f</a>.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_FromColor_SkColor'><code><strong>SkColor</strong></code></a></td>
    <td><a href='SkColor_Reference#Color'>Color</a> with <a href='SkColor_Reference#Alpha'>Alpha</a>, red, blue, and green components</td>
  </tr>
</table>

### Return Value

<a href='#SkRGBA4f'>SkRGBA4f</a> equivalent

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

---

<a name='SkRGBA4f_toSkColor'></a>
## toSkColor

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkRGBA4f_toSkColor'>toSkColor</a>() const
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

---

<a name='SkRGBA4f_FromPMColor'></a>
## FromPMColor

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#SkRGBA4f'>SkRGBA4f</a> <a href='#SkRGBA4f_FromPMColor'>FromPMColor</a>(<a href='SkColor_Reference#SkPMColor'>SkPMColor</a>)
</pre>

Converts from <a href='undocumented#Premultiply'>Premultiplied</a> integer components to <a href='undocumented#Unpremultiply'>Unpremultiplied</a> float
components.

### Parameters

<table>  <tr>    <td><a name='SkRGBA4f_FromPMColor_SkPMColor'><code><strong>SkPMColor</strong></code></a></td>
    <td><a href='undocumented#Premultiply'>Premultiplied</a> color</td>
  </tr>
</table>

### Return Value

<a href='undocumented#Unpremultiply'>Unpremultiplied</a> color

### See Also

<a href='#SkRGBA4f_FromColor'>FromColor</a>

---

<a name='SkRGBA4f_premul'></a>
## premul

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkRGBA4f'>SkRGBA4f</a>&lt;<a href='SkImageInfo_Reference#kPremul_SkAlphaType'>kPremul_SkAlphaType</a>&gt; <a href='#SkRGBA4f_premul'>premul</a>() const
</pre>

Returns <a href='#SkColor4f'>SkColor4f</a> with all components premultiplied by <a href='SkColor_Reference#Alpha'>Alpha</a>.

### Return Value

<a href='undocumented#Premultiply'>Premultiplied</a> color

### See Also

<a href='#SkRGBA4f_unpremul'>unpremul</a>

---

<a name='SkRGBA4f_unpremul'></a>
## unpremul

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkRGBA4f'>SkRGBA4f</a>&lt;<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>&gt; <a href='#SkRGBA4f_unpremul'>unpremul</a>() const
</pre>

Returns <a href='#SkRGBA4f'>SkRGBA4f</a> with all components independent of <a href='SkColor_Reference#Alpha'>Alpha</a>.

### Return Value

<a href='undocumented#Unpremultiply'>Unpremultiplied</a> color

### See Also

<a href='#SkRGBA4f_premul'>premul</a>

---

## <a name='SkColor4f'>Typedef SkColor4f</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
using <a href='#SkColor4f'>SkColor4f</a> = <a href='#SkRGBA4f'>SkRGBA4f</a><<a href='SkImageInfo_Reference#kUnpremul_SkAlphaType'>kUnpremul_SkAlphaType</a>>;
</pre>

