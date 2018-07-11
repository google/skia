SkPoint Reference
===

# <a name='Point'>Point</a>

# <a name='SkPoint'>Struct SkPoint</a>

## <a name='Typedef'>Typedef</a>


SkPoint  <code>typedef</code> define a data type.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
</table>

<a href='#SkPoint'>SkPoint</a> holds two 32-bit floating point coordinates.

## Overview

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Constructor'>Constructors</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>functions that construct <a href='#SkPoint'>SkPoint</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Member_Function'>Functions</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>global and class member functions</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Member'>Members</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>member values</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Operator'>Operators</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>operator overloading methods</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Related_Function'>Related Functions</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>similar member functions grouped together</td>
  </tr>
</table>


## <a name='Related_Function'>Related Function</a>


SkPoint global, <code>struct</code>, and <code>class</code> related member functions share a topic.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_Offset'>Offset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>moves sides</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Property'>Property</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>member values</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Set'>Set</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>replaces all values</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Vector'>Vector</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>alias for <a href='#Point'>Point</a></td>
  </tr>
</table>

## <a name='Member_Function'>Member Function</a>


SkPoint member functions read and modify the structure properties.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_CrossProduct'>CrossProduct</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns cross product</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_Distance'>Distance</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns straight-line distance between points</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_DotProduct'>DotProduct</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns dot product</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_Length'>Length</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns straight-line distance to origin</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_Make'>Make</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs from <a href='undocumented#SkScalar'>SkScalar</a> inputs</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_Normalize'>Normalize</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets length to one, and returns prior length</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_Offset'>Offset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>translates <a href='#Point'>Point</a> array</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_cross'>cross</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns cross product</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_distanceToOrigin'>distanceToOrigin</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns straight-line distance to origin</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_dot'>dot</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns dot product</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_equals'>equals</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if <a href='#Point'>Points</a> are equal</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_isFinite'>isFinite</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if no member is infinite or NaN</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_isZero'>isZero</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if both members equal zero</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_iset'>iset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets to integer input</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_length'>length</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns straight-line distance to origin</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_negate'>negate</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>reverses the sign of both members</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_normalize'>normalize</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets length to one, preserving direction</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_offset'>offset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>translates <a href='#Point'>Point</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_scale'>scale</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>multiplies <a href='#Point'>Point</a> by scale factor</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_set'>set</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets to <a href='undocumented#SkScalar'>SkScalar</a> input</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_setAbs'>setAbs</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets sign of both members to positive</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_setLength'>setLength</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets straight-line distance to origin</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_setNormalize'>setNormalize</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets length to one, in direction of (x, y)</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_x'>x</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#SkPoint_fX'>fX</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_y'>y</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#SkPoint_fY'>fY</a></td>
  </tr>
</table>

## <a name='Member'>Member</a>



### Members

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Name</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#fX'><code>fX</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>x-axis value</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#fY'><code>fY</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>y-axis value</td>
  </tr>
</table>


SkPoint members may be read and written directly without using a member function.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_fX'>fX</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>x-axis value</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_fY'>fY</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>y-axis value</td>
  </tr>
</table>

### Members

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Name</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPoint_fX'><code>fX</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
x-axis value used by both <a href='#Point'>Point</a> and <a href='#Vector'>Vector</a>. May contain any value, including
infinities and NaN.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPoint_fY'><code>fY</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
y-axis value used by both <a href='#Point'>Point</a> and <a href='#Vector'>Vector</a>. May contain any value, including
infinities and NaN.
</td>
  </tr>


## <a name='Constructor'>Constructor</a>


SkPoint can be constructed or initialized by these functions, including C++ class constructors.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_Make'>Make</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs from <a href='undocumented#SkScalar'>SkScalar</a> inputs</td>
  </tr>
</table>

<a name='SkPoint_Make'></a>
## Make

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='#SkPoint'>SkPoint</a> <a href='#SkPoint_Make'>Make</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y)
</pre>

Sets <a href='#SkPoint_fX'>fX</a> to x, <a href='#SkPoint_fY'>fY</a> to y. Used both to set <a href='#Point'>Point</a> and <a href='#Vector'>Vector</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_Make_x'><code><strong>x</strong></code></a></td>
    <td><a href='undocumented#SkScalar'>SkScalar</a> x-axis value of constructed <a href='#Point'>Point</a> or <a href='#Vector'>Vector</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_Make_y'><code><strong>y</strong></code></a></td>
    <td><a href='undocumented#SkScalar'>SkScalar</a> y-axis value of constructed <a href='#Point'>Point</a> or <a href='#Vector'>Vector</a></td>
  </tr>
</table>

### Return Value

<a href='#Point'>Point</a> (x, y)

### Example

<div><fiddle-embed name="d266e70977847001f7c42f8a2513bee7">

#### Example Output

~~~~
all equal
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPoint_set'>set</a> <a href='#SkPoint_iset'>iset</a><sup><a href='#SkPoint_iset_2'>[2]</a></sup> <a href='SkIPoint_Reference#SkIPoint_Make'>SkIPoint::Make</a>

---

## <a name='Property'>Property</a>


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_Distance'>Distance</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns straight-line distance between points</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_Length'>Length</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns straight-line distance to origin</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_distanceToOrigin'>distanceToOrigin</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns straight-line distance to origin</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_isFinite'>isFinite</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if no member is infinite or NaN</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_isZero'>isZero</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if both members equal zero</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_length'>length</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns straight-line distance to origin</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_x'>x</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#SkPoint_fX'>fX</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_y'>y</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#SkPoint_fY'>fY</a></td>
  </tr>
</table>

<a name='SkPoint_x'></a>
## x

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_x'>x</a>() const
</pre>

Returns x-axis value of <a href='#Point'>Point</a> or <a href='#Vector'>Vector</a>.

### Return Value

<a href='#SkPoint_fX'>fX</a>

### Example

<div><fiddle-embed name="9f3fe446b800ae1d940785d438634941">

#### Example Output

~~~~
pt1.fX == pt1.x()
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPoint_y'>y</a> <a href='SkIPoint_Reference#SkIPoint_x'>SkIPoint::x()</a>

---

<a name='SkPoint_y'></a>
## y

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_y'>y</a>() const
</pre>

Returns y-axis value of <a href='#Point'>Point</a> or <a href='#Vector'>Vector</a>.

### Return Value

<a href='#SkPoint_fY'>fY</a>

### Example

<div><fiddle-embed name="4c962850c2dbea4d2325df469400680e">

#### Example Output

~~~~
pt1.fY == pt1.y()
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPoint_x'>x</a> <a href='SkIPoint_Reference#SkIPoint_y'>SkIPoint::y()</a>

---

<a name='SkPoint_isZero'></a>
## isZero

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPoint_isZero'>isZero</a>() const
</pre>

Returns true if <a href='#SkPoint_fX'>fX</a> and <a href='#SkPoint_fY'>fY</a> are both zero.

### Return Value

true if <a href='#SkPoint_fX'>fX</a> is zero and <a href='#SkPoint_fY'>fY</a> is zero

### Example

<div><fiddle-embed name="81b9665110b88ef6bcbc20464aed7da1">

#### Example Output

~~~~
pt.fX=+0 pt.fY=-0
pt.isZero() == true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPoint_isFinite'>isFinite</a> <a href='SkIPoint_Reference#SkIPoint_isZero'>SkIPoint::isZero</a>

---

## <a name='Set'>Set</a>


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_iset'>iset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets to integer input</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_iset'>iset(int32 t x, int32 t y)</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_iset_2'>iset(const SkIPoint& p)</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_normalize'>normalize</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets length to one, preserving direction</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_set'>set</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets to <a href='undocumented#SkScalar'>SkScalar</a> input</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_setAbs'>setAbs</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets sign of both members to positive</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_setLength'>setLength</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets straight-line distance to origin</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_setLength'>setLength(SkScalar length)</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_setLength_2'>setLength(SkScalar x, SkScalar y, SkScalar length)</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_setNormalize'>setNormalize</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets length to one, in direction of (x, y)</td>
  </tr>
</table>

<a name='SkPoint_set'></a>
## set

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPoint_set'>set</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y)
</pre>

Sets <a href='#SkPoint_fX'>fX</a> to x and <a href='#SkPoint_fY'>fY</a> to y.

### Parameters

<table>  <tr>    <td><a name='SkPoint_set_x'><code><strong>x</strong></code></a></td>
    <td>new value for <a href='#SkPoint_fX'>fX</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_set_y'><code><strong>y</strong></code></a></td>
    <td>new value for <a href='#SkPoint_fY'>fY</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d08d1e7dafcad4342d1619fdbb2f5781">

#### Example Output

~~~~
pt1 == pt2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPoint_iset'>iset</a><sup><a href='#SkPoint_iset_2'>[2]</a></sup> <a href='#SkPoint_Make'>Make</a>

---

<a name='SkPoint_iset'></a>
## iset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPoint_iset'>iset</a>(int32_t x, int32_t y)
</pre>

Sets <a href='#SkPoint_fX'>fX</a> to x and <a href='#SkPoint_fY'>fY</a> to y, promoting integers to <a href='undocumented#SkScalar'>SkScalar</a> values.

Assigning a large integer value directly to <a href='#SkPoint_fX'>fX</a> or <a href='#SkPoint_fY'>fY</a> may cause a compiler
error, triggered by narrowing conversion of int to <a href='undocumented#SkScalar'>SkScalar</a>. This safely
casts x and y to avoid the error.

### Parameters

<table>  <tr>    <td><a name='SkPoint_iset_x'><code><strong>x</strong></code></a></td>
    <td>new value for <a href='#SkPoint_fX'>fX</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_iset_y'><code><strong>y</strong></code></a></td>
    <td>new value for <a href='#SkPoint_fY'>fY</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0d9e8ed734981b5b113f22c7bfde5357"></fiddle-embed></div>

### See Also

<a href='#SkPoint_set'>set</a> <a href='#SkPoint_Make'>Make</a> <a href='SkIPoint_Reference#SkIPoint_set'>SkIPoint::set</a>

---

<a name='SkPoint_iset_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPoint_iset'>iset</a>(const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& p)
</pre>

Sets <a href='#SkPoint_fX'>fX</a> to <a href='#SkPoint_iset_2_p'>p</a>.<a href='#SkPoint_fX'>fX</a> and <a href='#SkPoint_fY'>fY</a> to <a href='#SkPoint_iset_2_p'>p</a>.<a href='#SkPoint_fY'>fY</a>, promoting integers to <a href='undocumented#SkScalar'>SkScalar</a> values.

Assigning an <a href='SkIPoint_Reference#IPoint'>IPoint</a> containing a large integer value directly to <a href='#SkPoint_fX'>fX</a> or <a href='#SkPoint_fY'>fY</a> may
cause a compiler error, triggered by narrowing conversion of int to <a href='undocumented#SkScalar'>SkScalar</a>.
This safely casts <a href='#SkPoint_iset_2_p'>p</a>.<a href='#SkPoint_fX'>fX</a> and <a href='#SkPoint_iset_2_p'>p</a>.<a href='#SkPoint_fY'>fY</a> to avoid the error.

### Parameters

<table>  <tr>    <td><a name='SkPoint_iset_2_p'><code><strong>p</strong></code></a></td>
    <td><a href='SkIPoint_Reference#IPoint'>IPoint</a> members promoted to <a href='undocumented#SkScalar'>SkScalar</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="12b7164a769e232bb772f19c59600ee7">

#### Example Output

~~~~
iPt: -2147483647, 2147483647
fPt: -2.14748e+09, 2.14748e+09
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPoint_set'>set</a> <a href='#SkPoint_Make'>Make</a> <a href='SkIPoint_Reference#SkIPoint_set'>SkIPoint::set</a>

---

<a name='SkPoint_setAbs'></a>
## setAbs

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPoint_setAbs'>setAbs</a>(const <a href='#SkPoint'>SkPoint</a>& pt)
</pre>

Sets <a href='#SkPoint_fX'>fX</a> to absolute value of <a href='#SkPoint_setAbs_pt'>pt</a>.<a href='#SkPoint_fX'>fX</a>; and <a href='#SkPoint_fY'>fY</a> to absolute value of <a href='#SkPoint_setAbs_pt'>pt</a>.<a href='#SkPoint_fY'>fY</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_setAbs_pt'><code><strong>pt</strong></code></a></td>
    <td>members providing magnitude for <a href='#SkPoint_fX'>fX</a> and <a href='#SkPoint_fY'>fY</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="7f70860e820b67a347cff03c00488426">

#### Example Output

~~~~
pt: 0, -0  abs: 0, 0
pt: -1, -2  abs: 1, 2
pt: inf, -inf  abs: inf, inf
pt: nan, -nan  abs: nan, nan
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPoint_set'>set</a> <a href='#SkPoint_Make'>Make</a> <a href='#SkPoint_negate'>negate</a>

---

## <a name='Offset'>Offset</a>


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_Normalize'>Normalize</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets length to one, and returns prior length</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_Offset'>Offset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>translates <a href='#Point'>Point</a> array</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_Offset'>Offset(SkPoint points[], int count, const SkVector& offset)</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_Offset_2'>Offset(SkPoint points[], int count, SkScalar dx, SkScalar dy)</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_offset'>offset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>translates <a href='#Point'>Point</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_scale'>scale</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>multiplies <a href='#Point'>Point</a> by scale factor</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_scale'>scale(SkScalar scale, SkPoint* dst)</a> const</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_scale_2'>scale(SkScalar value)</a></td>
  </tr>
</table>

<a name='SkPoint_Offset'></a>
## Offset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static void <a href='#SkPoint_Offset'>Offset</a>(<a href='#SkPoint'>SkPoint</a> points[], int count, const <a href='#SkVector'>SkVector</a>& offset)
</pre>

Adds offset to each <a href='#Point'>Point</a> in <a href='#SkPoint_Offset_points'>points</a> array with <a href='#SkPoint_Offset_count'>count</a> entries.

### Parameters

<table>  <tr>    <td><a name='SkPoint_Offset_points'><code><strong>points</strong></code></a></td>
    <td><a href='#Point'>Point</a> array</td>
  </tr>
  <tr>    <td><a name='SkPoint_Offset_count'><code><strong>count</strong></code></a></td>
    <td>entries in array</td>
  </tr>
  <tr>    <td><a name='SkPoint_Offset_offset'><code><strong>offset</strong></code></a></td>
    <td><a href='#Vector'>Vector</a> added to <a href='#SkPoint_Offset_points'>points</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f0f24726df78a5d797bcf311e694a0a3"></fiddle-embed></div>

### See Also

<a href='#SkPoint_offset'>offset</a> <a href='#SkPoint_addto_operator'>operator+=(const SkVector& v)</a>

---

<a name='SkPoint_Offset_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static void <a href='#SkPoint_Offset'>Offset</a>(<a href='#SkPoint'>SkPoint</a> points[], int count, <a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy)
</pre>

Adds offset (<a href='#SkPoint_Offset_2_dx'>dx</a>, <a href='#SkPoint_Offset_2_dy'>dy</a>) to each <a href='#Point'>Point</a> in <a href='#SkPoint_Offset_2_points'>points</a> array of length <a href='#SkPoint_Offset_2_count'>count</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_Offset_2_points'><code><strong>points</strong></code></a></td>
    <td><a href='#Point'>Point</a> array</td>
  </tr>
  <tr>    <td><a name='SkPoint_Offset_2_count'><code><strong>count</strong></code></a></td>
    <td>entries in array</td>
  </tr>
  <tr>    <td><a name='SkPoint_Offset_2_dx'><code><strong>dx</strong></code></a></td>
    <td>added to <a href='#SkPoint_fX'>fX</a> in <a href='#SkPoint_Offset_2_points'>points</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_Offset_2_dy'><code><strong>dy</strong></code></a></td>
    <td>added to <a href='#SkPoint_fY'>fY</a> in <a href='#SkPoint_Offset_2_points'>points</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="532849faa838de885b86d3ebffae3712"></fiddle-embed></div>

### See Also

<a href='#SkPoint_offset'>offset</a> <a href='#SkPoint_addto_operator'>operator+=(const SkVector& v)</a>

---

<a name='SkPoint_offset'></a>
## offset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPoint_offset'>offset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy)
</pre>

Adds offset (<a href='#SkPoint_offset_dx'>dx</a>, <a href='#SkPoint_offset_dy'>dy</a>) to <a href='#Point'>Point</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_offset_dx'><code><strong>dx</strong></code></a></td>
    <td>added to <a href='#SkPoint_fX'>fX</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_offset_dy'><code><strong>dy</strong></code></a></td>
    <td>added to <a href='#SkPoint_fY'>fY</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="02750ceaa874f956e6e6544ef6b858ee"></fiddle-embed></div>

### See Also

<a href='#SkPoint_Offset'>Offset</a><sup><a href='#SkPoint_Offset_2'>[2]</a></sup> <a href='#SkPoint_addto_operator'>operator+=(const SkVector& v)</a>

---

<a name='SkPoint_length'></a>
## length

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_length'>length</a>() const
</pre>

Returns the <a href='undocumented#Euclidean_Distance'>Euclidean Distance</a> from origin, computed as:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sqrt(fX * fX + fY * fY)
</pre>

.

### Return Value

straight-line distance to origin

### Example

<div><fiddle-embed name="8363ab179447ee4b827679e20d3d81eb"></fiddle-embed></div>

### See Also

<a href='#SkPoint_distanceToOrigin'>distanceToOrigin</a> <a href='#SkPoint_Length'>Length</a> <a href='#SkPoint_setLength'>setLength</a><sup><a href='#SkPoint_setLength_2'>[2]</a></sup> <a href='#SkPoint_Distance'>Distance</a>

---

<a name='SkPoint_distanceToOrigin'></a>
## distanceToOrigin

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_distanceToOrigin'>distanceToOrigin</a>() const
</pre>

Returns the <a href='undocumented#Euclidean_Distance'>Euclidean Distance</a> from origin, computed as:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sqrt(fX * fX + fY * fY)
</pre>

.

### Return Value

straight-line distance to origin

### Example

<div><fiddle-embed name="812cf26d91b1cdcd2c6b9438a8172518"></fiddle-embed></div>

### See Also

<a href='#SkPoint_length'>length</a> <a href='#SkPoint_Length'>Length</a> <a href='#SkPoint_setLength'>setLength</a><sup><a href='#SkPoint_setLength_2'>[2]</a></sup> <a href='#SkPoint_Distance'>Distance</a>

---

<a name='SkPoint_normalize'></a>
## normalize

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPoint_normalize'>normalize</a>()
</pre>

Scales (<a href='#SkPoint_fX'>fX</a>, <a href='#SkPoint_fY'>fY</a>) so that <a href='#SkPoint_length'>length</a> returns one, while preserving ratio of <a href='#SkPoint_fX'>fX</a> to <a href='#SkPoint_fY'>fY</a>,
if possible. If prior length is nearly zero, sets <a href='#Vector'>Vector</a> to (0, 0) and returns
false; otherwise returns true.

### Return Value

true if former length is not zero or nearly zero

### Example

<div><fiddle-embed name="d84fce292d86c7d9ef37ae2d179c03c7"></fiddle-embed></div>

### See Also

<a href='#SkPoint_Normalize'>Normalize</a> <a href='#SkPoint_setLength'>setLength</a><sup><a href='#SkPoint_setLength_2'>[2]</a></sup> <a href='#SkPoint_length'>length</a> <a href='#SkPoint_Length'>Length</a>

---

<a name='SkPoint_setNormalize'></a>
## setNormalize

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPoint_setNormalize'>setNormalize</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y)
</pre>

Sets <a href='#Vector'>Vector</a> to (x, y) scaled so <a href='#SkPoint_length'>length</a> returns one, and so that
(<a href='#SkPoint_fX'>fX</a>, <a href='#SkPoint_fY'>fY</a>) is proportional to (x, y).  If (x, y) length is nearly zero,
sets <a href='#Vector'>Vector</a> to (0, 0) and returns false; otherwise returns true.

### Parameters

<table>  <tr>    <td><a name='SkPoint_setNormalize_x'><code><strong>x</strong></code></a></td>
    <td>proportional value for <a href='#SkPoint_fX'>fX</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_setNormalize_y'><code><strong>y</strong></code></a></td>
    <td>proportional value for <a href='#SkPoint_fY'>fY</a></td>
  </tr>
</table>

### Return Value

true if (x, y) length is not zero or nearly zero

### Example

<div><fiddle-embed name="3e4f147d143a388802484bf0d26534c2"></fiddle-embed></div>

### See Also

<a href='#SkPoint_normalize'>normalize</a> <a href='#SkPoint_setLength'>setLength</a><sup><a href='#SkPoint_setLength_2'>[2]</a></sup>

---

<a name='SkPoint_setLength'></a>
## setLength

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPoint_setLength'>setLength</a>(<a href='undocumented#SkScalar'>SkScalar</a> length)
</pre>

Scales <a href='#Vector'>Vector</a> so that <a href='#SkPoint_distanceToOrigin'>distanceToOrigin</a> returns length, if possible. If former
length is nearly zero, sets <a href='#Vector'>Vector</a> to (0, 0) and return false; otherwise returns
true.

### Parameters

<table>  <tr>    <td><a name='SkPoint_setLength_length'><code><strong>length</strong></code></a></td>
    <td>straight-line distance to origin</td>
  </tr>
</table>

### Return Value

true if former length is not zero or nearly zero

### Example

<div><fiddle-embed name="cbe7db206ece825aa3b9b7c3256aeaf0"></fiddle-embed></div>

### See Also

<a href='#SkPoint_length'>length</a> <a href='#SkPoint_Length'>Length</a> <a href='#SkPoint_setNormalize'>setNormalize</a> <a href='#SkPoint_setAbs'>setAbs</a>

---

<a name='SkPoint_setLength_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPoint_setLength'>setLength</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, <a href='undocumented#SkScalar'>SkScalar</a> length)
</pre>

Sets <a href='#Vector'>Vector</a> to (x, y) scaled to length, if possible. If former
length is nearly zero, sets <a href='#Vector'>Vector</a> to (0, 0) and return false; otherwise returns
true.

### Parameters

<table>  <tr>    <td><a name='SkPoint_setLength_2_x'><code><strong>x</strong></code></a></td>
    <td>proportional value for <a href='#SkPoint_fX'>fX</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_setLength_2_y'><code><strong>y</strong></code></a></td>
    <td>proportional value for <a href='#SkPoint_fY'>fY</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_setLength_2_length'><code><strong>length</strong></code></a></td>
    <td>straight-line distance to origin</td>
  </tr>
</table>

### Return Value

true if (x, y) length is not zero or nearly zero

### Example

<div><fiddle-embed name="3cc0662b6fbbee1fe3442a0acfece22c"></fiddle-embed></div>

### See Also

<a href='#SkPoint_length'>length</a> <a href='#SkPoint_Length'>Length</a> <a href='#SkPoint_setNormalize'>setNormalize</a> <a href='#SkPoint_setAbs'>setAbs</a>

---

## <a name='Operator'>Operator</a>


SkPoint operators inline class member functions with arithmetic equivalents.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_CrossProduct'>CrossProduct</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns cross product</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_DotProduct'>DotProduct</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns dot product</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_cross'>cross</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns cross product</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_dot'>dot</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns dot product</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_equals'>equals</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if <a href='#Point'>Points</a> are equal</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_negate'>negate</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>reverses the sign of both members</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_notequal_operator'>operator!=(const SkPoint& a, const SkPoint& b)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if <a href='#Point'>Point</a> are unequal</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_multiply_operator'>operator*(SkScalar scale) const</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#Point'>Point</a> multiplied by scale</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_multiplyby_operator'>operator*=(SkScalar scale)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>multiplies <a href='#Point'>Point</a> by scale factor</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_add_operator'>operator+(const SkPoint& a, const SkVector& b)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#Point'>Point</a> offset by <a href='#Vector'>Vector</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_addto_operator'>operator+=(const SkVector& v)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds <a href='#Vector'>Vector</a> to <a href='#Point'>Point</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_minus_operator'>operator-() const</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>reverses sign of <a href='#Point'>Point</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_subtract_operator'>operator-(const SkPoint& a, const SkPoint& b)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#Vector'>Vector</a> between <a href='#Point'>Points</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_subtractfrom_operator'>operator-=(const SkVector& v)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>subtracts <a href='#Vector'>Vector</a> from <a href='#Point'>Point</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPoint_equal_operator'>operator==(const SkPoint& a, const SkPoint& b)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if <a href='#Point'>Point</a> are equal</td>
  </tr>
</table>

<a name='SkPoint_scale'></a>
## scale

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPoint_scale'>scale</a>(<a href='undocumented#SkScalar'>SkScalar</a> scale, <a href='#SkPoint'>SkPoint</a>* dst) const
</pre>

Sets <a href='#SkPoint_scale_dst'>dst</a> to <a href='#Point'>Point</a> times scale. <a href='#SkPoint_scale_dst'>dst</a> may be <a href='#Point'>Point</a> to modify <a href='#Point'>Point</a> in place.

### Parameters

<table>  <tr>    <td><a name='SkPoint_scale_scale'><code><strong>scale</strong></code></a></td>
    <td>factor to multiply <a href='#Point'>Point</a> by</td>
  </tr>
  <tr>    <td><a name='SkPoint_scale_dst'><code><strong>dst</strong></code></a></td>
    <td>storage for scaled <a href='#Point'>Point</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="972e4e230806281adb928e068bcd8551"></fiddle-embed></div>

### See Also

<a href='#SkPoint_multiply_operator'>operator*(SkScalar scale) const</a> <a href='#SkPoint_multiplyby_operator'>operator*=(SkScalar scale)</a> <a href='#SkPoint_setLength'>setLength</a><sup><a href='#SkPoint_setLength_2'>[2]</a></sup>

---

<a name='SkPoint_scale_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPoint_scale'>scale</a>(<a href='undocumented#SkScalar'>SkScalar</a> value)
</pre>

Scales <a href='#Point'>Point</a> in place by scale.

### Parameters

<table>  <tr>    <td><a name='SkPoint_scale_2_value'><code><strong>value</strong></code></a></td>
    <td>factor to multiply <a href='#Point'>Point</a> by</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1060a4f27d8ef29519e6ac006ce90f2b"></fiddle-embed></div>

### See Also

<a href='#SkPoint_multiply_operator'>operator*(SkScalar scale) const</a> <a href='#SkPoint_multiplyby_operator'>operator*=(SkScalar scale)</a> <a href='#SkPoint_setLength'>setLength</a><sup><a href='#SkPoint_setLength_2'>[2]</a></sup>

---

<a name='SkPoint_negate'></a>
## negate

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPoint_negate'>negate</a>()
</pre>

Changes the sign of <a href='#SkPoint_fX'>fX</a> and <a href='#SkPoint_fY'>fY</a>.

### Example

<div><fiddle-embed name="312c0c8065ab5d0adfda80cccf2d11e6">

#### Example Output

~~~~
pt: 0, -0  negate: -0, 0
pt: -1, -2  negate: 1, 2
pt: inf, -inf  negate: -inf, inf
pt: nan, -nan  negate: -nan, nan
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPoint_minus_operator'>operator-() const</a> <a href='#SkPoint_setAbs'>setAbs</a>

---

<a name='SkPoint_minus_operator'></a>
## operator-

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPoint'>SkPoint</a> operator-() _const
</pre>

Returns <a href='#Point'>Point</a> changing the signs of <a href='#SkPoint_fX'>fX</a> and <a href='#SkPoint_fY'>fY</a>.

### Return Value

<a href='#Point'>Point</a> as (-<a href='#SkPoint_fX'>fX</a>, -<a href='#SkPoint_fY'>fY</a>)

### Example

<div><fiddle-embed name="9baf247cfcd8272c0ddf6ce93f676b37">

#### Example Output

~~~~
pt: 0, -0  negate: -0, 0
pt: -1, -2  negate: 1, 2
pt: inf, -inf  negate: -inf, inf
pt: nan, -nan  negate: -nan, nan
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPoint_negate'>negate</a> <a href='#SkPoint_subtract_operator'>operator-(const SkPoint& a, const SkPoint& b)</a> <a href='#SkPoint_subtractfrom_operator'>operator-=(const SkVector& v)</a> <a href='SkIPoint_Reference#SkIPoint_minus_operator'>SkIPoint::operator-() const</a>

---

<a name='SkPoint_addto_operator'></a>
## operator+=

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPoint_addto_operator'>operator+=(const SkVector& v)</a>
</pre>

Adds <a href='#Vector'>Vector</a> <a href='#SkPoint_addto_operator_v'>v</a> to <a href='#Point'>Point</a>. Sets <a href='#Point'>Point</a> to:
(<a href='#SkPoint_fX'>fX</a> + <a href='#SkPoint_addto_operator_v'>v</a>.<a href='#SkPoint_fX'>fX</a>, <a href='#SkPoint_fY'>fY</a> + <a href='#SkPoint_addto_operator_v'>v</a>.<a href='#SkPoint_fY'>fY</a>)
.

### Parameters

<table>  <tr>    <td><a name='SkPoint_addto_operator_v'><code><strong>v</strong></code></a></td>
    <td><a href='#Vector'>Vector</a> to add</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="8b4e79109e2381345258cb744881b20c"></fiddle-embed></div>

### See Also

<a href='#SkPoint_offset'>offset</a> <a href='#SkPoint_add_operator'>operator+(const SkPoint& a, const SkVector& b)</a> <a href='SkIPoint_Reference#SkIPoint_addto_operator'>SkIPoint::operator+=(const SkIVector& v)</a>

---

<a name='SkPoint_subtractfrom_operator'></a>
## operator-=

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPoint_subtractfrom_operator'>operator-=(const SkVector& v)</a>
</pre>

Subtracts <a href='#Vector'>Vector</a> <a href='#SkPoint_subtractfrom_operator_v'>v</a> from <a href='#Point'>Point</a>. Sets <a href='#Point'>Point</a> to:
(<a href='#SkPoint_fX'>fX</a> - <a href='#SkPoint_subtractfrom_operator_v'>v</a>.<a href='#SkPoint_fX'>fX</a>, <a href='#SkPoint_fY'>fY</a> - <a href='#SkPoint_subtractfrom_operator_v'>v</a>.<a href='#SkPoint_fY'>fY</a>)
.

### Parameters

<table>  <tr>    <td><a name='SkPoint_subtractfrom_operator_v'><code><strong>v</strong></code></a></td>
    <td><a href='#Vector'>Vector</a> to subtract</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="86c0399704d8dff4091bf87b8d87d40b"></fiddle-embed></div>

### See Also

<a href='#SkPoint_offset'>offset</a> <a href='#SkPoint_subtract_operator'>operator-(const SkPoint& a, const SkPoint& b)</a> <a href='SkIPoint_Reference#SkIPoint_subtractfrom_operator'>SkIPoint::operator-=(const SkIVector& v)</a>

---

<a name='SkPoint_multiply_operator'></a>
## operator*

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPoint'>SkPoint</a> operator*(SkScalar scale) _const
</pre>

Returns <a href='#Point'>Point</a> multiplied by scale.

### Parameters

<table>  <tr>    <td><a name='SkPoint_multiply_operator_scale'><code><strong>scale</strong></code></a></td>
    <td><a href='undocumented#Scalar'>Scalar</a> to multiply by</td>
  </tr>
</table>

### Return Value

<a href='#Point'>Point</a> as (<a href='#SkPoint_fX'>fX</a> * scale, <a href='#SkPoint_fY'>fY</a> * scale)

### Example

<div><fiddle-embed name="35b3bc675779de043706ae4817ee950c"></fiddle-embed></div>

### See Also

<a href='#SkPoint_multiplyby_operator'>operator*=(SkScalar scale)</a> <a href='#SkPoint_scale'>scale</a><sup><a href='#SkPoint_scale_2'>[2]</a></sup> <a href='#SkPoint_setLength'>setLength</a><sup><a href='#SkPoint_setLength_2'>[2]</a></sup> <a href='#SkPoint_setNormalize'>setNormalize</a>

---

<a name='SkPoint_multiplyby_operator'></a>
## operator*=

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPoint'>SkPoint</a>& <a href='#SkPoint_multiplyby_operator'>operator*=(SkScalar scale)</a>
</pre>

Multiplies <a href='#Point'>Point</a> by scale. Sets <a href='#Point'>Point</a> to:
(<a href='#SkPoint_fX'>fX</a> * scale, <a href='#SkPoint_fY'>fY</a> * scale)
.

### Parameters

<table>  <tr>    <td><a name='SkPoint_multiplyby_operator_scale'><code><strong>scale</strong></code></a></td>
    <td><a href='undocumented#Scalar'>Scalar</a> to multiply by</td>
  </tr>
</table>

### Return Value

reference to <a href='#Point'>Point</a>

### Example

<div><fiddle-embed name="3ce3db36235d80dbac4d39504cf756da"></fiddle-embed></div>

### See Also

<a href='#SkPoint_multiply_operator'>operator*(SkScalar scale) const</a> <a href='#SkPoint_scale'>scale</a><sup><a href='#SkPoint_scale_2'>[2]</a></sup> <a href='#SkPoint_setLength'>setLength</a><sup><a href='#SkPoint_setLength_2'>[2]</a></sup> <a href='#SkPoint_setNormalize'>setNormalize</a>

---

<a name='SkPoint_isFinite'></a>
## isFinite

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPoint_isFinite'>isFinite</a>() const
</pre>

Returns true if both <a href='#SkPoint_fX'>fX</a> and <a href='#SkPoint_fY'>fY</a> are measurable values.

### Return Value

true for values other than infinities and NaN

### Example

<div><fiddle-embed name="937cc166cc0e220f33fb82501141d0b3">

#### Example Output

~~~~
pt: 0, -0  finite: true
pt: -1, -2  finite: true
pt: inf, 1  finite: false
pt: nan, -1  finite: false
~~~~

</fiddle-embed></div>

### See Also

<a href='SkRect_Reference#SkRect_isFinite'>SkRect::isFinite</a> <a href='SkPath_Reference#SkPath_isFinite'>SkPath::isFinite</a>

---

<a name='SkPoint_equals'></a>
## equals

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPoint_equals'>equals</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y) const
</pre>

Returns true if <a href='#Point'>Point</a> is equivalent to <a href='#Point'>Point</a> constructed from (x, y).

### Parameters

<table>  <tr>    <td><a name='SkPoint_equals_x'><code><strong>x</strong></code></a></td>
    <td>value compared with <a href='#SkPoint_fX'>fX</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_equals_y'><code><strong>y</strong></code></a></td>
    <td>value compared with <a href='#SkPoint_fY'>fY</a></td>
  </tr>
</table>

### Return Value

true if <a href='#Point'>Point</a> equals (x, y)

### Example

<div><fiddle-embed name="4cecb878c8b66beffda051f26c00f817">

#### Example Output

~~~~
pt: 0, -0  == pt
pt: -1, -2  == pt
pt: inf, 1  == pt
pt: nan, -1  != pt
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPoint_equal_operator'>operator==(const SkPoint& a, const SkPoint& b)</a>

---

<a name='SkPoint_equal_operator'></a>
## operator==

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPoint_equal_operator'>operator==(const SkPoint& a, const SkPoint& b)</a>
</pre>

Returns true if <a href='#SkPoint_equal_operator_a'>a</a> is equivalent to <a href='#SkPoint_equal_operator_b'>b</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_equal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='#Point'>Point</a> to compare</td>
  </tr>
  <tr>    <td><a name='SkPoint_equal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='#Point'>Point</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href='#SkPoint_equal_operator_a'>a</a>.<a href='#SkPoint_fX'>fX</a> == <a href='#SkPoint_equal_operator_b'>b</a>.<a href='#SkPoint_fX'>fX</a> and <a href='#SkPoint_equal_operator_a'>a</a>.<a href='#SkPoint_fY'>fY</a> == <a href='#SkPoint_equal_operator_b'>b</a>.<a href='#SkPoint_fY'>fY</a>

### Example

<div><fiddle-embed name="741f793334a48a35dadf4310d7ea52cb">

#### Example Output

~~~~
pt: 0, -0  == pt
pt: -1, -2  == pt
pt: inf, 1  == pt
pt: nan, -1  != pt
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPoint_equals'>equals</a> <a href='#SkPoint_notequal_operator'>operator!=(const SkPoint& a, const SkPoint& b)</a>

---

<a name='SkPoint_notequal_operator'></a>
## operator!=

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPoint_notequal_operator'>operator!=(const SkPoint& a, const SkPoint& b)</a>
</pre>

Returns true if <a href='#SkPoint_notequal_operator_a'>a</a> is not equivalent to <a href='#SkPoint_notequal_operator_b'>b</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_notequal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='#Point'>Point</a> to compare</td>
  </tr>
  <tr>    <td><a name='SkPoint_notequal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='#Point'>Point</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href='#SkPoint_notequal_operator_a'>a</a>.<a href='#SkPoint_fX'>fX</a> != <a href='#SkPoint_notequal_operator_b'>b</a>.<a href='#SkPoint_fX'>fX</a> or <a href='#SkPoint_notequal_operator_a'>a</a>.<a href='#SkPoint_fY'>fY</a> != <a href='#SkPoint_notequal_operator_b'>b</a>.<a href='#SkPoint_fY'>fY</a>

### Example

<div><fiddle-embed name="8fe8572685eaa617f25a5a6767a874dc">

#### Example Output

~~~~
pt: 0, -0  == pt
pt: -1, -2  == pt
pt: inf, 1  == pt
pt: nan, -1  != pt
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPoint_equal_operator'>operator==(const SkPoint& a, const SkPoint& b)</a> <a href='#SkPoint_equals'>equals</a>

---

<a name='SkPoint_subtract_operator'></a>
## operator-

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkVector'>SkVector</a> <a href='#SkPoint_subtract_operator'>operator-(const SkPoint& a, const SkPoint& b)</a>
</pre>

Returns <a href='#Vector'>Vector</a> from <a href='#SkPoint_subtract_operator_b'>b</a> to <a href='#SkPoint_subtract_operator_a'>a</a>, computed as(<a href='#SkPoint_subtract_operator_a'>a</a>.<a href='#SkPoint_fX'>fX</a> - <a href='#SkPoint_subtract_operator_b'>b</a>.<a href='#SkPoint_fX'>fX</a>, <a href='#SkPoint_subtract_operator_a'>a</a>.<a href='#SkPoint_fY'>fY</a> - <a href='#SkPoint_subtract_operator_b'>b</a>.<a href='#SkPoint_fY'>fY</a>)
.

Can also be used to subtract <a href='#Vector'>Vector</a> from <a href='#Point'>Point</a>, returning <a href='#Point'>Point</a>.
Can also be used to subtract <a href='#Vector'>Vector</a> from <a href='#Vector'>Vector</a>, returning <a href='#Vector'>Vector</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_subtract_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='#Point'>Point</a> to subtract from</td>
  </tr>
  <tr>    <td><a name='SkPoint_subtract_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='#Point'>Point</a> to subtract</td>
  </tr>
</table>

### Return Value

<a href='#Vector'>Vector</a> from <a href='#SkPoint_subtract_operator_b'>b</a> to <a href='#SkPoint_subtract_operator_a'>a</a>

### Example

<div><fiddle-embed name="b6c4943ecd0b2dccf9d220b8944009e0"></fiddle-embed></div>

### See Also

<a href='#SkPoint_subtractfrom_operator'>operator-=(const SkVector& v)</a> <a href='#SkPoint_offset'>offset</a>

---

<a name='SkPoint_add_operator'></a>
## operator+

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPoint'>SkPoint</a> <a href='#SkPoint_add_operator'>operator+(const SkPoint& a, const SkVector& b)</a>
</pre>

Returns <a href='#Point'>Point</a> resulting from <a href='#Point'>Point</a> <a href='#SkPoint_add_operator_a'>a</a> offset by <a href='#Vector'>Vector</a> <a href='#SkPoint_add_operator_b'>b</a>, computed as:
(<a href='#SkPoint_add_operator_a'>a</a>.<a href='#SkPoint_fX'>fX</a> + <a href='#SkPoint_add_operator_b'>b</a>.<a href='#SkPoint_fX'>fX</a>, <a href='#SkPoint_add_operator_a'>a</a>.<a href='#SkPoint_fY'>fY</a> + <a href='#SkPoint_add_operator_b'>b</a>.<a href='#SkPoint_fY'>fY</a>)
.

Can also be used to offset <a href='#Point'>Point</a> <a href='#SkPoint_add_operator_b'>b</a> by <a href='#Vector'>Vector</a> <a href='#SkPoint_add_operator_a'>a</a>, returning <a href='#Point'>Point</a>.
Can also be used to add <a href='#Vector'>Vector</a> to <a href='#Vector'>Vector</a>, returning <a href='#Vector'>Vector</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_add_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='#Point'>Point</a> or <a href='#Vector'>Vector</a> to add to</td>
  </tr>
  <tr>    <td><a name='SkPoint_add_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='#Point'>Point</a> or <a href='#Vector'>Vector</a> to add</td>
  </tr>
</table>

### Return Value

<a href='#Point'>Point</a> equal to <a href='#SkPoint_add_operator_a'>a</a> offset by <a href='#SkPoint_add_operator_b'>b</a>

### Example

<div><fiddle-embed name="911a84253dfec4dabf94dbe3c71766f0"></fiddle-embed></div>

### See Also

<a href='#SkPoint_addto_operator'>operator+=(const SkVector& v)</a> <a href='#SkPoint_offset'>offset</a>

---

<a name='SkPoint_Length'></a>
## Length

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_Length'>Length</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y)
</pre>

Returns the <a href='undocumented#Euclidean_Distance'>Euclidean Distance</a> from origin, computed as:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sqrt(x * x + y * y)
</pre>

.

### Parameters

<table>  <tr>    <td><a name='SkPoint_Length_x'><code><strong>x</strong></code></a></td>
    <td>component of length</td>
  </tr>
  <tr>    <td><a name='SkPoint_Length_y'><code><strong>y</strong></code></a></td>
    <td>component of length</td>
  </tr>
</table>

### Return Value

straight-line distance to origin

### Example

<div><fiddle-embed name="c98773d8b4509969d78cb8121e4b77f6"></fiddle-embed></div>

### See Also

<a href='#SkPoint_length'>length</a> <a href='#SkPoint_Distance'>Distance</a> <a href='#SkPoint_setLength'>setLength</a><sup><a href='#SkPoint_setLength_2'>[2]</a></sup>

---

<a name='SkPoint_Normalize'></a>
## Normalize

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_Normalize'>Normalize</a>(<a href='#SkVector'>SkVector</a>* vec)
</pre>

Scales (<a href='#SkPoint_Normalize_vec'>vec</a>-><a href='#SkPoint_fX'>fX</a>, <a href='#SkPoint_Normalize_vec'>vec</a>-><a href='#SkPoint_fY'>fY</a>) so that <a href='#SkPoint_length'>length</a> returns one, while preserving ratio of <a href='#SkPoint_Normalize_vec'>vec</a>-><a href='#SkPoint_fX'>fX</a> to <a href='#SkPoint_Normalize_vec'>vec</a>-><a href='#SkPoint_fY'>fY</a>,
if possible. If original length is nearly zero, sets <a href='#SkPoint_Normalize_vec'>vec</a> to (0, 0) and returns zero;
otherwise, returns length of <a href='#SkPoint_Normalize_vec'>vec</a> before <a href='#SkPoint_Normalize_vec'>vec</a> is scaled.

Returned prior length may be <a href='undocumented#SK_ScalarInfinity'>SK ScalarInfinity</a> if it can not be represented by <a href='undocumented#SkScalar'>SkScalar</a>.

Note that <a href='#SkPoint_normalize'>normalize</a> is faster if prior length is not required.

### Parameters

<table>  <tr>    <td><a name='SkPoint_Normalize_vec'><code><strong>vec</strong></code></a></td>
    <td>normalized to unit length</td>
  </tr>
</table>

### Return Value

original <a href='#SkPoint_Normalize_vec'>vec</a> length

### Example

<div><fiddle-embed name="60a08f3ce75374fc815384616d114df7"></fiddle-embed></div>

### See Also

<a href='#SkPoint_normalize'>normalize</a> <a href='#SkPoint_setLength'>setLength</a><sup><a href='#SkPoint_setLength_2'>[2]</a></sup> <a href='#SkPoint_Length'>Length</a>

---

<a name='SkPoint_Distance'></a>
## Distance

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_Distance'>Distance</a>(const <a href='#SkPoint'>SkPoint</a>& a, const <a href='#SkPoint'>SkPoint</a>& b)
</pre>

Returns the <a href='undocumented#Euclidean_Distance'>Euclidean Distance</a> between <a href='#SkPoint_Distance_a'>a</a> and <a href='#SkPoint_Distance_b'>b</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_Distance_a'><code><strong>a</strong></code></a></td>
    <td>line end point</td>
  </tr>
  <tr>    <td><a name='SkPoint_Distance_b'><code><strong>b</strong></code></a></td>
    <td>line end point</td>
  </tr>
</table>

### Return Value

straight-line distance from <a href='#SkPoint_Distance_a'>a</a> to <a href='#SkPoint_Distance_b'>b</a>

### Example

<div><fiddle-embed name="9e0a2de2eb94dba4521d733e73f2bda5"></fiddle-embed></div>

### See Also

<a href='#SkPoint_length'>length</a> <a href='#SkPoint_setLength'>setLength</a><sup><a href='#SkPoint_setLength_2'>[2]</a></sup>

---

<a name='SkPoint_DotProduct'></a>
## DotProduct

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_DotProduct'>DotProduct</a>(const <a href='#SkVector'>SkVector</a>& a, const <a href='#SkVector'>SkVector</a>& b)
</pre>

Returns the dot product of <a href='#Vector'>Vector</a> <a href='#SkPoint_DotProduct_a'>a</a> and <a href='#Vector'>Vector</a> <a href='#SkPoint_DotProduct_b'>b</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_DotProduct_a'><code><strong>a</strong></code></a></td>
    <td>left side of dot product</td>
  </tr>
  <tr>    <td><a name='SkPoint_DotProduct_b'><code><strong>b</strong></code></a></td>
    <td>right side of dot product</td>
  </tr>
</table>

### Return Value

product of input magnitudes and cosine of the angle between them

### Example

<div><fiddle-embed name="496db0131a003162faba7d7f98b30340"></fiddle-embed></div>

### See Also

<a href='#SkPoint_dot'>dot</a> <a href='#SkPoint_CrossProduct'>CrossProduct</a>

---

<a name='SkPoint_CrossProduct'></a>
## CrossProduct

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_CrossProduct'>CrossProduct</a>(const <a href='#SkVector'>SkVector</a>& a, const <a href='#SkVector'>SkVector</a>& b)
</pre>

Returns the cross product of <a href='#Vector'>Vector</a> <a href='#SkPoint_CrossProduct_a'>a</a> and <a href='#Vector'>Vector</a> <a href='#SkPoint_CrossProduct_b'>b</a>.

<a href='#SkPoint_CrossProduct_a'>a</a> and <a href='#SkPoint_CrossProduct_b'>b</a> form three-dimensional vectors with z-axis value equal to zero. The
cross product is <a href='#SkPoint_CrossProduct_a'>a</a> three-dimensional vector with x-axis and y-axis values equal
to zero. The cross product z-axis component is returned.

### Parameters

<table>  <tr>    <td><a name='SkPoint_CrossProduct_a'><code><strong>a</strong></code></a></td>
    <td>left side of cross product</td>
  </tr>
  <tr>    <td><a name='SkPoint_CrossProduct_b'><code><strong>b</strong></code></a></td>
    <td>right side of cross product</td>
  </tr>
</table>

### Return Value

area spanned by <a href='#Vector'>Vectors</a> signed by angle direction

### Example

<div><fiddle-embed name="8b8a4cd8a29d22bb9c5e63b70357bd65"></fiddle-embed></div>

### See Also

<a href='#SkPoint_cross'>cross</a> <a href='#SkPoint_DotProduct'>DotProduct</a>

---

<a name='SkPoint_cross'></a>
## cross

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_cross'>cross</a>(const <a href='#SkVector'>SkVector</a>& vec) const
</pre>

Returns the cross product of <a href='#Vector'>Vector</a> and <a href='#SkPoint_cross_vec'>vec</a>.

<a href='#Vector'>Vector</a> and <a href='#SkPoint_cross_vec'>vec</a> form three-dimensional vectors with z-axis value equal to zero.
The cross product is a three-dimensional vector with x-axis and y-axis values
equal to zero. The cross product z-axis component is returned.

### Parameters

<table>  <tr>    <td><a name='SkPoint_cross_vec'><code><strong>vec</strong></code></a></td>
    <td>right side of cross product</td>
  </tr>
</table>

### Return Value

area spanned by <a href='#Vector'>Vectors</a> signed by angle direction

### Example

<div><fiddle-embed name="0bc7b3997357e499817278b78bdfbf1d"></fiddle-embed></div>

### See Also

<a href='#SkPoint_CrossProduct'>CrossProduct</a> <a href='#SkPoint_dot'>dot</a>

---

<a name='SkPoint_dot'></a>
## dot

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_dot'>dot</a>(const <a href='#SkVector'>SkVector</a>& vec) const
</pre>

Returns the dot product of <a href='#Vector'>Vector</a> and <a href='#Vector'>Vector</a> <a href='#SkPoint_dot_vec'>vec</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_dot_vec'><code><strong>vec</strong></code></a></td>
    <td>right side of dot product</td>
  </tr>
</table>

### Return Value

product of input magnitudes and cosine of the angle between them

### Example

<div><fiddle-embed name="56d01ccfedd71d3c504b09afa2875d38"></fiddle-embed></div>

### See Also

<a href='#SkPoint_DotProduct'>DotProduct</a> <a href='#SkPoint_cross'>cross</a>

---

## <a name='Vector'>Vector</a>

## <a name='SkVector'>Typedef SkVector</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    typedef <a href='#SkPoint'>SkPoint</a> <a href='#SkVector'>SkVector</a>;
</pre>

<a href='#SkVector'>SkVector</a> provides an alternative name for <a href='#SkPoint'>SkPoint</a>. <a href='#SkVector'>SkVector</a> and <a href='#SkPoint'>SkPoint</a> can
be used interchangeably for all purposes.