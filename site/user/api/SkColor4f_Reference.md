SkColor4f Reference
===

# <a name='Color4f'>Color4f</a>

# <a name='SkColor4f'>Struct SkColor4f</a>

## <a name='Member'>Member</a>


SkColor4f members may be read and written directly without using a member function.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkColor4f_fA'>fA</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>alpha component</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkColor4f_fB'>fB</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>blue component</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkColor4f_fG'>fG</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>green component</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkColor4f_fR'>fR</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>red component</td>
  </tr>
</table>

Each component is stored as a 32-bit single precision floating point float value.
All values are allowed, but only the range from zero to one is meaningful.

Each component is independent of the others; <a href='#SkColor4f_fA'>fA</a> <a href='SkColor_Reference#Alpha'>Alpha</a> is not <a href='undocumented#Premultiply'>Premultiplied</a>
with <a href='#SkColor4f_fG'>fG</a> green, <a href='#SkColor4f_fB'>fB</a> blue, or <a href='#SkColor4f_fR'>fR</a> red.

Values smaller than zero or larger than one are allowed. Values out of range
may be used with <a href='SkBlendMode_Reference#Blend_Mode'>Blend Mode</a> so that the final component is in range.

## Overview

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Member_Function'>Functions</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>global and class member functions</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Member'>Members</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>member values</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Operator'>Operators</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>operator overloading methods</td>
  </tr>
</table>


## <a name='Operator'>Operator</a>


SkColor4f operators inline class member functions with arithmetic equivalents.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkColor4f_notequal1_operator'>operator!=(const SkColor4f& other) const</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>compares colors for inequality</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkColor4f_equal1_operator'>operator==(const SkColor4f& other) const</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>compares <a href='#Color4f'>Color4f</a> for equality</td>
  </tr>
</table>

## <a name='Member_Function'>Member Function</a>


SkColor4f member functions read and modify the structure properties.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkColor4f_FromColor'>FromColor</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets components from <a href='SkColor_Reference#Color'>Color</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkColor4f_Pin'>Pin</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets components to valid range</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkColor4f_pin'>pin</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets components to valid range</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkColor4f_premul'>premul</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='undocumented#Premultiply'>Premultiplied</a> color; internal use only</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkColor4f_toSkColor'>toSkColor</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns closest <a href='SkColor_Reference#Color'>Color</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkColor4f_vec'>vec</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns array of components</td>
  </tr>
</table>

### Members

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Name</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>float</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkColor4f_fR'><code>fR</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Single precision float for red ranges from no red (0.0) to full red (1.0).
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>float</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkColor4f_fG'><code>fG</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Single precision float for green ranges from no green (0.0) to full green (1.0).
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>float</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkColor4f_fB'><code>fB</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Single precision float for blue ranges from no blue (0.0) to full blue (1.0).
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>float</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkColor4f_fA'><code>fA</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Single precision float for <a href='SkColor_Reference#Alpha'>Alpha</a> ranges from no <a href='SkColor_Reference#Alpha'>Alpha</a> (0.0) to full <a href='SkColor_Reference#Alpha'>Alpha</a> (1.0).
</td>
  </tr>
</table>

<a name='SkColor4f_equal1_operator'></a>
## operator==

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator==(const SkColor4f& other) _const
</pre>

Compares <a href='#Color4f'>Color4f</a> with <a href='#SkColor4f_equal1_operator_other'>other</a>, and returns true if all components are equivalent.

### Parameters

<table>  <tr>    <td><a name='SkColor4f_equal1_operator_other'><code><strong>other</strong></code></a></td>
    <td><a href='#Color4f'>Color4f</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href='#Color4f'>Color4f</a> equals <a href='#SkColor4f_equal1_operator_other'>other</a>

### Example

<div><fiddle-embed name="e5b34bcb7f80f2ed890cdacaa059db0d">

#### Example Output

~~~~
colorRed == colorNamedRed
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkColor4f_notequal1_operator'>operator!=(const SkColor4f& other) const</a>

---

<a name='SkColor4f_notequal1_operator'></a>
## operator!=

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator!=(const SkColor4f& other) _const
</pre>

Compares <a href='#Color4f'>Color4f</a> with <a href='#SkColor4f_notequal1_operator_other'>other</a>, and returns true if all components are not
equivalent.

### Parameters

<table>  <tr>    <td><a name='SkColor4f_notequal1_operator_other'><code><strong>other</strong></code></a></td>
    <td><a href='#Color4f'>Color4f</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href='#Color4f'>Color4f</a> is not equal to <a href='#SkColor4f_notequal1_operator_other'>other</a>

### Example

<div><fiddle-embed name="82f1a9b4c2b27aa547061786d1f33dab">

#### Example Output

~~~~
colorGray != colorNamedGray
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkColor4f_equal1_operator'>operator==(const SkColor4f& other) const</a>

---

<a name='SkColor4f_vec'></a>
## vec

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const float* <a href='#SkColor4f_vec'>vec</a>() const
</pre>

Returns <a href='#Color4f'>Color4f</a> components as a read-only array.

### Return Value

components as read-only array

### Example

<div><fiddle-embed name="229057023515224358a36acf15508cf6">

#### Example Output

~~~~
red=0.0578054 green=0.246201 blue=0.603827 alpha=0.533333
[0]=0.0578054 [1]=0.246201 [2]=0.603827 [3]=0.533333
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkColor4f'>SkColor4f</a>

---

<a name='SkColor4f_vec_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
float* <a href='#SkColor4f_vec'>vec</a>()
</pre>

Returns <a href='#Color4f'>Color4f</a> components as a writable array.

### Return Value

components as writable array

### Example

<div><fiddle-embed name="7420bf0a7cae5c6577c4c4a4613e7e7e">

#### Example Output

~~~~
red=0.0578054 green=0.246201 blue=0.603827 alpha=0.533333
[0]=0.0578054 [1]=0.246201 [2]=0.603827 [3]=1
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkColor4f'>SkColor4f</a>

---

<a name='SkColor4f_Pin'></a>
## Pin

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#SkColor4f'>SkColor4f</a> <a href='#SkColor4f_Pin'>Pin</a>(float r, float g, float b, float a)
</pre>

Constructs and returns <a href='#Color4f'>Color4f</a> with each component pinned from zero to one.

### Parameters

<table>  <tr>    <td><a name='SkColor4f_Pin_r'><code><strong>r</strong></code></a></td>
    <td>red component</td>
  </tr>
  <tr>    <td><a name='SkColor4f_Pin_g'><code><strong>g</strong></code></a></td>
    <td>green component</td>
  </tr>
  <tr>    <td><a name='SkColor4f_Pin_b'><code><strong>b</strong></code></a></td>
    <td>blue component</td>
  </tr>
  <tr>    <td><a name='SkColor4f_Pin_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkColor_Reference#Alpha'>Alpha</a> component</td>
  </tr>
</table>

### Return Value

<a href='#Color4f'>Color4f</a> with valid components

### Example

<div><fiddle-embed name="c989cf16c7f8849874eb008cd701af76"></fiddle-embed></div>

### See Also

<a href='#SkColor4f_pin'>pin</a> <a href='#SkColor4f_FromColor'>FromColor</a>

---

<a name='SkColor4f_FromColor'></a>
## FromColor

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#SkColor4f'>SkColor4f</a> <a href='#SkColor4f_FromColor'>FromColor</a>(<a href='SkColor_Reference#SkColor'>SkColor</a>)
</pre>

Converts to closest <a href='#Color4f'>Color4f</a>.

### Parameters

<table>  <tr>    <td><a name='SkColor4f_FromColor_SkColor'><code><strong>SkColor</strong></code></a></td>
    <td><a href='SkColor_Reference#Color'>Color</a> with <a href='SkColor_Reference#Alpha'>Alpha</a>, red, blue, and green components</td>
  </tr>
</table>

### Return Value

<a href='#Color4f'>Color4f</a> equivalent

### Example

<div><fiddle-embed name="33b029064e8d1928e42a587c953d0e4e">

#### Example Output

~~~~
red=0.0742136 green=0.130136 blue=0.318547 alpha=0.168627
red=77 green=101 blue=153 alpha=43
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkColor4f_toSkColor'>toSkColor</a>

---

<a name='SkColor4f_toSkColor'></a>
## toSkColor

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkColor_Reference#SkColor'>SkColor</a> <a href='#SkColor4f_toSkColor'>toSkColor</a>() const
</pre>

Converts to closest <a href='SkColor_Reference#SkColor'>SkColor</a>.

### Return Value

closest <a href='SkColor_Reference#Color'>Color</a>

### Example

<div><fiddle-embed name="edc5fd18d961f7607d2bcbf7f7d427e5">

#### Example Output

~~~~
red=75 green=101 blue=153 alpha=43
red=0.0703601 green=0.130136 blue=0.318547 alpha=0.168627
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkColor4f_FromColor'>FromColor</a>

---

<a name='SkColor4f_pin'></a>
## pin

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkColor4f'>SkColor4f</a> <a href='#SkColor4f_pin'>pin</a>() const
</pre>

Returns <a href='#Color4f'>Color4f</a> with all components in the range from zero to one.

### Return Value

<a href='#Color4f'>Color4f</a> with valid components

### Example

<div><fiddle-embed name="9e349862c5189a44d2acef5da24f2e79"></fiddle-embed></div>

### See Also

<a href='#SkColor4f_Pin'>Pin</a>

---

<a name='SkColor4f_premul'></a>
## premul

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPM4f'>SkPM4f</a> <a href='#SkColor4f_premul'>premul</a>() const
</pre>

### Return Value

<a href='undocumented#Premultiply'>Premultiplied</a> color

---

# <a name='SkPM4f'>Struct SkPM4f</a>
