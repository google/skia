SkIPoint Reference
===

# <a name='IPoint'>IPoint</a>

# <a name='SkIPoint'>Struct SkIPoint</a>

## <a name='Typedef'>Typedef</a>


SkIPoint  <code>typedef</code> define a data type.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
</table>

<a href='#SkIPoint'>SkIPoint</a> holds two 32-bit integer coordinates.

## Overview

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Constructor'>Constructors</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>functions that construct <a href='#SkIPoint'>SkIPoint</a></td>
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


SkIPoint global, <code>struct</code>, and <code>class</code> related member functions share a topic.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#IVector'>IVector</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>alias for <a href='#IPoint'>IPoint</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Property'>Property</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>member values</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Set'>Set</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>replaces all values</td>
  </tr>
</table>

## <a name='Member_Function'>Member Function</a>


SkIPoint member functions read and modify the structure properties.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIPoint_Make'>Make</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs from integer inputs</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIPoint_equals'>equals</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if members are equal</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIPoint_isZero'>isZero</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if both members equal zero</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIPoint_set'>set</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets to integer input</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIPoint_x'>x</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#SkIPoint_fX'>fX</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIPoint_y'>y</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#SkIPoint_fY'>fY</a></td>
  </tr>
</table>

## <a name='Member'>Member</a>



### Members

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Name</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#fX'><code>fX</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>x-axis value</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#fY'><code>fY</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>y-axis value</td>
  </tr>
</table>


SkIPoint members may be read and written directly without using a member function.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIPoint_fX'>fX</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>x-axis value</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIPoint_fY'>fY</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>y-axis value</td>
  </tr>
</table>

### Members

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Name</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkIPoint_fX'><code>fX</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
x-axis value used by <a href='#IPoint'>IPoint</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkIPoint_fY'><code>fY</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
y-axis value used by <a href='#IPoint'>IPoint</a>.
</td>
  </tr>


## <a name='Constructor'>Constructor</a>


SkIPoint can be constructed or initialized by these functions, including C++ class constructors.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIPoint_Make'>Make</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs from integer inputs</td>
  </tr>
</table>

<a name='SkIPoint_Make'></a>
## Make

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='#SkIPoint'>SkIPoint</a> <a href='#SkIPoint_Make'>Make</a>(int32_t x, int32_t y)
</pre>

Sets <a href='#SkIPoint_fX'>fX</a> to x, <a href='#SkIPoint_fY'>fY</a> to y.

### Parameters

<table>  <tr>    <td><a name='SkIPoint_Make_x'><code><strong>x</strong></code></a></td>
    <td>integer x-axis value of constructed <a href='#IPoint'>IPoint</a></td>
  </tr>
  <tr>    <td><a name='SkIPoint_Make_y'><code><strong>y</strong></code></a></td>
    <td>integer y-axis value of constructed <a href='#IPoint'>IPoint</a></td>
  </tr>
</table>

### Return Value

<a href='#IPoint'>IPoint</a> (x, y)

### Example

<div><fiddle-embed name="e5cf5159525bd3140f288a95fe641fae">

#### Example Output

~~~~
pt1 == pt2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIPoint_set'>set</a> <a href='SkPoint_Reference#SkPoint_iset'>SkPoint::iset()</a><sup><a href='SkPoint_Reference#SkPoint_iset_2'>[2]</a></sup> <a href='SkPoint_Reference#SkPoint_Make'>SkPoint::Make</a>

---

## <a name='Property'>Property</a>


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIPoint_isZero'>isZero</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if both members equal zero</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIPoint_x'>x</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#SkIPoint_fX'>fX</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIPoint_y'>y</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#SkIPoint_fY'>fY</a></td>
  </tr>
</table>

<a name='SkIPoint_x'></a>
## x

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIPoint_x'>x</a>() const
</pre>

Returns x-axis value of <a href='#IPoint'>IPoint</a>.

### Return Value

<a href='#SkIPoint_fX'>fX</a>

### Example

<div><fiddle-embed name="eed4185294f8a8216fc354e6ee6b2e3a">

#### Example Output

~~~~
pt1.fX == pt1.x()
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIPoint_y'>y</a> <a href='SkPoint_Reference#SkPoint_x'>SkPoint::x()</a>

---

<a name='SkIPoint_y'></a>
## y

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIPoint_y'>y</a>() const
</pre>

Returns y-axis value of <a href='#IPoint'>IPoint</a>.

### Return Value

<a href='#SkIPoint_fY'>fY</a>

### Example

<div><fiddle-embed name="35c41b8ba7cebf8c9a7a8494e610e14d">

#### Example Output

~~~~
pt1.fY == pt1.y()
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIPoint_x'>x</a> <a href='SkPoint_Reference#SkPoint_y'>SkPoint::y()</a>

---

<a name='SkIPoint_isZero'></a>
## isZero

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIPoint_isZero'>isZero</a>() const
</pre>

Returns true if <a href='#SkIPoint_fX'>fX</a> and <a href='#SkIPoint_fY'>fY</a> are both zero.

### Return Value

true if <a href='#SkIPoint_fX'>fX</a> is zero and <a href='#SkIPoint_fY'>fY</a> is zero

### Example

<div><fiddle-embed name="658c1df611b4577cc7e0bb384e95737e">

#### Example Output

~~~~
pt.isZero() == true
~~~~

</fiddle-embed></div>

### See Also

<a href='SkPoint_Reference#SkPoint_isZero'>SkPoint::isZero</a>

---

## <a name='Set'>Set</a>


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIPoint_set'>set</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets to integer input</td>
  </tr>
</table>

<a name='SkIPoint_set'></a>
## set

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIPoint_set'>set</a>(int32_t x, int32_t y)
</pre>

Sets <a href='#SkIPoint_fX'>fX</a> to x and <a href='#SkIPoint_fY'>fY</a> to y.

### Parameters

<table>  <tr>    <td><a name='SkIPoint_set_x'><code><strong>x</strong></code></a></td>
    <td>new value for <a href='#SkIPoint_fX'>fX</a></td>
  </tr>
  <tr>    <td><a name='SkIPoint_set_y'><code><strong>y</strong></code></a></td>
    <td>new value for <a href='#SkIPoint_fY'>fY</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="165418b5718d79d8f1682a8a0ee32ba0">

#### Example Output

~~~~
pt1 == pt2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIPoint_Make'>Make</a>

---

## <a name='Operator'>Operator</a>


SkIPoint operators inline class member functions with arithmetic equivalents.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIPoint_equals'>equals</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if members are equal</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIPoint_notequal_operator'>operator!=(const SkIPoint& a, const SkIPoint& b)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if <a href='#IPoint'>IPoints</a> are unequal</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIPoint_add_operator'>operator+(const SkIPoint& a, const SkIVector& b)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#IPoint'>IPoint</a> offset by <a href='#IVector'>IVector</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIPoint_addto_operator'>operator+=(const SkIVector& v)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds <a href='#IVector'>IVector</a> to <a href='#IPoint'>IPoint</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIPoint_minus_operator'>operator-() const</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>reverses sign of <a href='#IPoint'>IPoint</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIPoint_subtract_operator'>operator-(const SkIPoint& a, const SkIPoint& b)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#IVector'>IVector</a> between <a href='#IPoint'>IPoints</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIPoint_subtractfrom_operator'>operator-=(const SkIVector& v)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>subtracts <a href='#IVector'>IVector</a> from <a href='#IPoint'>IPoint</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkIPoint_equal_operator'>operator==(const SkIPoint& a, const SkIPoint& b)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if <a href='#IPoint'>IPoints</a> are equal</td>
  </tr>
</table>

<a name='SkIPoint_minus_operator'></a>
## operator-

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkIPoint'>SkIPoint</a> operator-() _const
</pre>

Returns <a href='#IPoint'>IPoint</a> changing the signs of <a href='#SkIPoint_fX'>fX</a> and <a href='#SkIPoint_fY'>fY</a>.

### Return Value

<a href='#IPoint'>IPoint</a> as (-<a href='#SkIPoint_fX'>fX</a>, -<a href='#SkIPoint_fY'>fY</a>)

### Example

<div><fiddle-embed name="b30d4780475d113a7fed3637af7f0db1">

#### Example Output

~~~~
pt: 0, 0  negate: 0, 0
pt: -1, -2  negate: 1, 2
pt: 2147483647, -2147483647  negate: -2147483647, 2147483647
pt: -2147483648, -2147483648  negate: -2147483648, -2147483648
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIPoint_subtract_operator'>operator-(const SkIPoint& a, const SkIPoint& b)</a> <a href='#SkIPoint_subtractfrom_operator'>operator-=(const SkIVector& v)</a> <a href='SkPoint_Reference#SkPoint_minus_operator'>SkPoint::operator-() const</a>

---

<a name='SkIPoint_addto_operator'></a>
## operator+=

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIPoint_addto_operator'>operator+=(const SkIVector& v)</a>
</pre>

Offsets <a href='#IPoint'>IPoint</a> by <a href='#IVector'>IVector</a> <a href='#SkIPoint_addto_operator_v'>v</a>. Sets <a href='#IPoint'>IPoint</a> to(<a href='#SkIPoint_fX'>fX</a> + <a href='#SkIPoint_addto_operator_v'>v</a>.<a href='#SkIPoint_fX'>fX</a>, <a href='#SkIPoint_fY'>fY</a> + <a href='#SkIPoint_addto_operator_v'>v</a>.<a href='#SkIPoint_fY'>fY</a>)
.

### Parameters

<table>  <tr>    <td><a name='SkIPoint_addto_operator_v'><code><strong>v</strong></code></a></td>
    <td><a href='#IVector'>IVector</a> to add</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4eb2d95c9e9a66f05296e345bb68bd51"></fiddle-embed></div>

### See Also

<a href='#SkIPoint_add_operator'>operator+(const SkIPoint& a, const SkIVector& b)</a> <a href='SkPoint_Reference#SkPoint_addto_operator'>SkPoint::operator+=(const SkVector& v)</a>

---

<a name='SkIPoint_subtractfrom_operator'></a>
## operator-=

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIPoint_subtractfrom_operator'>operator-=(const SkIVector& v)</a>
</pre>

Subtracts <a href='#IVector'>IVector</a> <a href='#SkIPoint_subtractfrom_operator_v'>v</a> from <a href='#IPoint'>IPoint</a>. Sets <a href='#IPoint'>IPoint</a> to:
(<a href='#SkIPoint_fX'>fX</a> - <a href='#SkIPoint_subtractfrom_operator_v'>v</a>.<a href='#SkIPoint_fX'>fX</a>, <a href='#SkIPoint_fY'>fY</a> - <a href='#SkIPoint_subtractfrom_operator_v'>v</a>.<a href='#SkIPoint_fY'>fY</a>)
.

### Parameters

<table>  <tr>    <td><a name='SkIPoint_subtractfrom_operator_v'><code><strong>v</strong></code></a></td>
    <td><a href='#IVector'>IVector</a> to subtract</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a01e533dc7ab34ed728dc4e7a5f1f0ee"></fiddle-embed></div>

### See Also

<a href='#SkIPoint_subtract_operator'>operator-(const SkIPoint& a, const SkIPoint& b)</a> <a href='SkPoint_Reference#SkPoint_subtractfrom_operator'>SkPoint::operator-=(const SkVector& v)</a>

---

<a name='SkIPoint_equals'></a>
## equals

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIPoint_equals'>equals</a>(int32_t x, int32_t y) const
</pre>

Returns true if <a href='#IPoint'>IPoint</a> is equivalent to <a href='#IPoint'>IPoint</a> constructed from (x, y).

### Parameters

<table>  <tr>    <td><a name='SkIPoint_equals_x'><code><strong>x</strong></code></a></td>
    <td>value compared with <a href='#SkIPoint_fX'>fX</a></td>
  </tr>
  <tr>    <td><a name='SkIPoint_equals_y'><code><strong>y</strong></code></a></td>
    <td>value compared with <a href='#SkIPoint_fY'>fY</a></td>
  </tr>
</table>

### Return Value

true if <a href='#IPoint'>IPoint</a> equals (x, y)

### Example

<div><fiddle-embed name="64f575d36439d5b69aaed14ffeff1cc4">

#### Example Output

~~~~
pt: 0, 0  == pt
pt: -1, -2  == pt
pt: 2147483647, -1  == pt
pt: -2147483648, -1  == pt
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIPoint_equal_operator'>operator==(const SkIPoint& a, const SkIPoint& b)</a>

---

<a name='SkIPoint_equal_operator'></a>
## operator==

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIPoint_equal_operator'>operator==(const SkIPoint& a, const SkIPoint& b)</a>
</pre>

Returns true if <a href='#SkIPoint_equal_operator_a'>a</a> is equivalent to <a href='#SkIPoint_equal_operator_b'>b</a>.

### Parameters

<table>  <tr>    <td><a name='SkIPoint_equal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='#IPoint'>IPoint</a> to compare</td>
  </tr>
  <tr>    <td><a name='SkIPoint_equal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='#IPoint'>IPoint</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href='#SkIPoint_equal_operator_a'>a</a>.<a href='#SkIPoint_fX'>fX</a> == <a href='#SkIPoint_equal_operator_b'>b</a>.<a href='#SkIPoint_fX'>fX</a> and <a href='#SkIPoint_equal_operator_a'>a</a>.<a href='#SkIPoint_fY'>fY</a> == <a href='#SkIPoint_equal_operator_b'>b</a>.<a href='#SkIPoint_fY'>fY</a>

### Example

<div><fiddle-embed name="37ffe2817d720f99e6c252332ce70460">

#### Example Output

~~~~
pt: 0, 0  == pt
pt: -1, -2  == pt
pt: 2147483647, -1  == pt
pt: -2147483648, -1  == pt
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIPoint_equals'>equals</a> <a href='#SkIPoint_notequal_operator'>operator!=(const SkIPoint& a, const SkIPoint& b)</a>

---

<a name='SkIPoint_notequal_operator'></a>
## operator!=

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIPoint_notequal_operator'>operator!=(const SkIPoint& a, const SkIPoint& b)</a>
</pre>

Returns true if <a href='#SkIPoint_notequal_operator_a'>a</a> is not equivalent to <a href='#SkIPoint_notequal_operator_b'>b</a>.

### Parameters

<table>  <tr>    <td><a name='SkIPoint_notequal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='#IPoint'>IPoint</a> to compare</td>
  </tr>
  <tr>    <td><a name='SkIPoint_notequal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='#IPoint'>IPoint</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href='#SkIPoint_notequal_operator_a'>a</a>.<a href='#SkIPoint_fX'>fX</a> != <a href='#SkIPoint_notequal_operator_b'>b</a>.<a href='#SkIPoint_fX'>fX</a> or <a href='#SkIPoint_notequal_operator_a'>a</a>.<a href='#SkIPoint_fY'>fY</a> != <a href='#SkIPoint_notequal_operator_b'>b</a>.<a href='#SkIPoint_fY'>fY</a>

### Example

<div><fiddle-embed name="6d6f2082fcf59d9f02bfb1758b87db69">

#### Example Output

~~~~
pt: 0, 0  == pt
pt: -1, -2  == pt
pt: 2147483647, -1  == pt
pt: -2147483648, -1  == pt
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIPoint_equal_operator'>operator==(const SkIPoint& a, const SkIPoint& b)</a> <a href='#SkIPoint_equals'>equals</a>

---

<a name='SkIPoint_subtract_operator'></a>
## operator-

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkIVector'>SkIVector</a> <a href='#SkIPoint_subtract_operator'>operator-(const SkIPoint& a, const SkIPoint& b)</a>
</pre>

Returns <a href='#IVector'>IVector</a> from <a href='#SkIPoint_subtract_operator_b'>b</a> to <a href='#SkIPoint_subtract_operator_a'>a</a>; computed as(<a href='#SkIPoint_subtract_operator_a'>a</a>.<a href='#SkIPoint_fX'>fX</a> - <a href='#SkIPoint_subtract_operator_b'>b</a>.<a href='#SkIPoint_fX'>fX</a>, <a href='#SkIPoint_subtract_operator_a'>a</a>.<a href='#SkIPoint_fY'>fY</a> - <a href='#SkIPoint_subtract_operator_b'>b</a>.<a href='#SkIPoint_fY'>fY</a>)
.

Can also be used to subtract <a href='#IVector'>IVector</a> from <a href='#IVector'>IVector</a>, returning <a href='#IVector'>IVector</a>.

### Parameters

<table>  <tr>    <td><a name='SkIPoint_subtract_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='#IPoint'>IPoint</a> or <a href='#IVector'>IVector</a> to subtract from</td>
  </tr>
  <tr>    <td><a name='SkIPoint_subtract_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='#IVector'>IVector</a> to subtract</td>
  </tr>
</table>

### Return Value

<a href='#IVector'>IVector</a> from <a href='#SkIPoint_subtract_operator_b'>b</a> to <a href='#SkIPoint_subtract_operator_a'>a</a>

### Example

<div><fiddle-embed name="e626e26bf557857b824aa7d03f723e0f"></fiddle-embed></div>

### See Also

<a href='#SkIPoint_subtractfrom_operator'>operator-=(const SkIVector& v)</a>

---

<a name='SkIPoint_add_operator'></a>
## operator+

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkIPoint'>SkIPoint</a> <a href='#SkIPoint_add_operator'>operator+(const SkIPoint& a, const SkIVector& b)</a>
</pre>

Returns <a href='#IPoint'>IPoint</a> resulting from <a href='#IPoint'>IPoint</a> <a href='#SkIPoint_add_operator_a'>a</a> offset by <a href='#IVector'>IVector</a> <a href='#SkIPoint_add_operator_b'>b</a>, computed as:
(<a href='#SkIPoint_add_operator_a'>a</a>.<a href='#SkIPoint_fX'>fX</a> + <a href='#SkIPoint_add_operator_b'>b</a>.<a href='#SkIPoint_fX'>fX</a>, <a href='#SkIPoint_add_operator_a'>a</a>.<a href='#SkIPoint_fY'>fY</a> + <a href='#SkIPoint_add_operator_b'>b</a>.<a href='#SkIPoint_fY'>fY</a>)
.

Can also be used to offset <a href='#IPoint'>IPoint</a> <a href='#SkIPoint_add_operator_b'>b</a> by <a href='#IVector'>IVector</a> <a href='#SkIPoint_add_operator_a'>a</a>, returning <a href='#IPoint'>IPoint</a>.
Can also be used to add <a href='#IVector'>IVector</a> to <a href='#IVector'>IVector</a>, returning <a href='#IVector'>IVector</a>.

### Parameters

<table>  <tr>    <td><a name='SkIPoint_add_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='#IPoint'>IPoint</a> or <a href='#IVector'>IVector</a> to add to</td>
  </tr>
  <tr>    <td><a name='SkIPoint_add_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='#IPoint'>IPoint</a> or <a href='#IVector'>IVector</a> to add</td>
  </tr>
</table>

### Return Value

<a href='#IPoint'>IPoint</a> equal to <a href='#SkIPoint_add_operator_a'>a</a> offset by <a href='#SkIPoint_add_operator_b'>b</a>

### Example

<div><fiddle-embed name="63f4cba971c6d8434595906f865b5a29"></fiddle-embed></div>

### See Also

<a href='#SkIPoint_addto_operator'>operator+=(const SkIVector& v)</a>

---

## <a name='IVector'>IVector</a>

## <a name='SkIVector'>Typedef SkIVector</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    typedef <a href='#SkIPoint'>SkIPoint</a> <a href='#SkIVector'>SkIVector</a>;
</pre>

<a href='#SkIVector'>SkIVector</a> provides an alternative name for <a href='#SkIPoint'>SkIPoint</a>. <a href='#SkIVector'>SkIVector</a> and <a href='#SkIPoint'>SkIPoint</a>
can be used interchangeably for all purposes.