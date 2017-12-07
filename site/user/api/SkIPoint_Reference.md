SkIPoint Reference
===

# <a name="IPoint"></a> IPoint

# <a name="SkIPoint"></a> Struct SkIPoint
<a href="#SkIPoint">SkIPoint</a> holds two 32 bit integer coordinates

# <a name="Overview"></a> Overview

## <a name="Subtopics"></a> Subtopics

| topics | description |
| --- | ---  |

## <a name="Operators"></a> Operators

| description | function |
| --- | ---  |
| <a href="#SkIPoint">SkIPoint</a> <a href="#SkIPoint_minus_operator">operator-() const</a> | Reverses sign of <a href="#IPoint">IPoint</a>. |
| <a href="#SkIPoint">SkIPoint</a> <a href="#SkIPoint_add_operator">operator+(const SkIPoint& a, const SkIVector& b)</a> | Returns <a href="#IPoint">IPoint</a> offset by <a href="SkIPoint_Reference#IVector">IVector</a>. |
| <a href="SkIPoint_Reference#SkIVector">SkIVector</a> <a href="#SkIPoint_subtract_operator">operator-(const SkIPoint& a, const SkIPoint& b)</a> | Returns <a href="SkIPoint_Reference#IVector">IVector</a> between <a href="#IPoint">IPoints</a>. |
| bool <a href="#SkIPoint_notequal_operator">operator!=(const SkIPoint& a, const SkIPoint& b)</a> | Returns true if <a href="#IPoint">IPoints</a> are unequal. |
| bool <a href="#SkIPoint_equal_operator">operator==(const SkIPoint& a, const SkIPoint& b)</a> | Returns true if <a href="#IPoint">IPoints</a> are equal. |
| void <a href="#SkIPoint_addto_operator">operator+=(const SkIVector& v)</a> | Adds <a href="SkIPoint_Reference#IVector">IVector</a> to <a href="#IPoint">IPoint</a>. |
| void <a href="#SkIPoint_subtractfrom_operator">operator-=(const SkIVector& v)</a> | Subtracts <a href="SkIPoint_Reference#IVector">IVector</a> from <a href="#IPoint">IPoint</a>. |

## <a name="Member_Functions"></a> Member Functions

| description | function |
| --- | ---  |
| <a href="#SkIPoint_Make">Make</a> | Constructs from integer inputs. |
| <a href="#SkIPoint_equals">equals</a> | Returns true if members are equal. |
| <a href="#SkIPoint_isZero">isZero</a> | Returns true if both members equal zero. |
| <a href="#SkIPoint_set">set</a> | Sets to integer input. |
| <a href="#SkIPoint_x">x</a> | Returns <a href="#SkIPoint_fX">fX</a>. |
| <a href="#SkIPoint_y">y</a> | Returns <a href="#SkIPoint_fY">fY</a>. |

<a name="SkIPoint_fX"> <code><strong>int32_t  fX</strong></code> </a>

<a href="#SkIPoint_x">x</a>-axis value used by <a href="#IPoint">IPoint</a>.

<a name="SkIPoint_fY"> <code><strong>int32_t  fY</strong></code> </a>

<a href="#SkIPoint_y">y</a>-axis value used by <a href="#IPoint">IPoint</a>.

<a name="SkIPoint_Make"></a>
## Make

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static constexpr SkIPoint Make(int32_t x, int32_t y)
</pre>

Sets <a href="#SkIPoint_fX">fX</a> to <a href="#SkIPoint_x">x</a>, <a href="#SkIPoint_fY">fY</a> to <a href="#SkIPoint_y">y</a>.

### Parameters

<table>  <tr>    <td><a name="SkIPoint_Make_x"> <code><strong>x </strong></code> </a></td> <td>
integer <a href="#SkIPoint_x">x</a>-axis value of constructed <a href="#IPoint">IPoint</a></td>
  </tr>  <tr>    <td><a name="SkIPoint_Make_y"> <code><strong>y </strong></code> </a></td> <td>
integer <a href="#SkIPoint_y">y</a>-axis value of constructed <a href="#IPoint">IPoint</a></td>
  </tr>
</table>

### Return Value

<a href="#IPoint">IPoint</a> (<a href="#SkIPoint_x">x</a>, <a href="#SkIPoint_y">y</a>)

### Example

<div><fiddle-embed name="e5cf5159525bd3140f288a95fe641fae">

#### Example Output

~~~~
pt1 == pt2
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIPoint_set">set</a> <a href="#SkPoint_iset">SkPoint::iset()</a> <a href="#SkPoint_Make">SkPoint::Make</a> <a href="#SkIPoint16_Make">SkIPoint16::Make</a>

---

<a name="SkIPoint_x"></a>
## x

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int32_t x() const
</pre>

Returns <a href="#SkIPoint_x">x</a>-axis value of <a href="#IPoint">IPoint</a>.

### Return Value

<a href="#SkIPoint_fX">fX</a>

### Example

<div><fiddle-embed name="eed4185294f8a8216fc354e6ee6b2e3a">

#### Example Output

~~~~
pt1.fX == pt1.x()
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIPoint_y">y</a> <a href="#SkPoint_x">SkPoint::x()</a> <a href="#SkIPoint16_x">SkIPoint16::x()</a>

---

<a name="SkIPoint_y"></a>
## y

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int32_t y() const
</pre>

Returns <a href="#SkIPoint_y">y</a>-axis value of <a href="#IPoint">IPoint</a>.

### Return Value

<a href="#SkIPoint_fY">fY</a>

### Example

<div><fiddle-embed name="35c41b8ba7cebf8c9a7a8494e610e14d">

#### Example Output

~~~~
pt1.fY == pt1.y()
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIPoint_x">x</a> <a href="#SkPoint_y">SkPoint::y()</a> <a href="#SkIPoint16_y">SkIPoint16::y()</a>

---

<a name="SkIPoint_isZero"></a>
## isZero

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isZero() const
</pre>

Returns true if <a href="#SkIPoint_fX">fX</a> and <a href="#SkIPoint_fY">fY</a> are both zero.

### Return Value

true if <a href="#SkIPoint_fX">fX</a> is zero and <a href="#SkIPoint_fY">fY</a> is zero

### Example

<div><fiddle-embed name="658c1df611b4577cc7e0bb384e95737e">

#### Example Output

~~~~
pt.isZero() == true
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkPoint_isZero">SkPoint::isZero</a>

---

<a name="SkIPoint_set"></a>
## set

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void set(int32_t x, int32_t y)
</pre>

Sets <a href="#SkIPoint_fX">fX</a> to <a href="#SkIPoint_x">x</a> and <a href="#SkIPoint_fY">fY</a> to <a href="#SkIPoint_y">y</a>.

### Parameters

<table>  <tr>    <td><a name="SkIPoint_set_x"> <code><strong>x </strong></code> </a></td> <td>
new value for <a href="#SkIPoint_fX">fX</a></td>
  </tr>  <tr>    <td><a name="SkIPoint_set_y"> <code><strong>y </strong></code> </a></td> <td>
new value for <a href="#SkIPoint_fY">fY</a></td>
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

<a href="#SkIPoint_Make">Make</a> <a href="#SkIPoint16_set">SkIPoint16::set</a>

---

<a name="SkIPoint_minus_operator"></a>
## operator-

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkIPoint operator-() _const
</pre>

Returns <a href="#IPoint">IPoint</a> changing the signs of <a href="#SkIPoint_fX">fX</a> and <a href="#SkIPoint_fY">fY</a>.

### Return Value

<a href="#IPoint">IPoint</a> as (-<a href="#SkIPoint_fX">fX</a>, -<a href="#SkIPoint_fY">fY</a>)

### Example

<div><fiddle-embed name="d283e8876366349aedd85b65cadb3706">

#### Example Output

~~~~
pt: 0, 0  negate: 0, 0
pt: -1, -2  negate: 1, 2
pt: 2147483647, -2147483647  negate: -2147483647, 2147483647
pt: -2147483648, -2147483648  negate: -2147483648, -2147483648
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIPoint_subtract_operator">operator-(const SkIPoint& a, const SkIPoint& b)</a> <a href="#SkIPoint_subtractfrom_operator">operator-=(const SkIVector& v)</a> <a href="#SkPoint_minus_operator">SkPoint::operator-() const</a>

---

<a name="SkIPoint_addto_operator"></a>
## operator+=

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void operator+=(const SkIVector& v)
</pre>

Offsets <a href="#IPoint">IPoint</a> by <a href="SkIPoint_Reference#IVector">IVector</a> <a href="#SkIPoint_addto_operator_v">v</a>. Sets <a href="#IPoint">IPoint</a> to(<a href="#SkIPoint_fX">fX</a> + <a href="#SkIPoint_addto_operator_v">v</a>.<a href="#SkIPoint_fX">fX</a>, <a href="#SkIPoint_fY">fY</a> + <a href="#SkIPoint_addto_operator_v">v</a>.<a href="#SkIPoint_fY">fY</a>).

### Parameters

<table>  <tr>    <td><a name="SkIPoint_addto_operator_v"> <code><strong>v </strong></code> </a></td> <td>
<a href="SkIPoint_Reference#IVector">IVector</a> to add</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4eb2d95c9e9a66f05296e345bb68bd51"></fiddle-embed></div>

### See Also

<a href="#SkIPoint_add_operator">operator+(const SkIPoint& a, const SkIVector& b)</a> <a href="#SkPoint_addto_operator">SkPoint::operator+=(const SkVector& v)</a>

---

<a name="SkIPoint_subtractfrom_operator"></a>
## operator-=

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void operator-=(const SkIVector& v)
</pre>

Subtracts <a href="SkIPoint_Reference#IVector">IVector</a> <a href="#SkIPoint_subtractfrom_operator_v">v</a> from <a href="#IPoint">IPoint</a>. Sets <a href="#IPoint">IPoint</a> to:
(<a href="#SkIPoint_fX">fX</a> - <a href="#SkIPoint_subtractfrom_operator_v">v</a>.<a href="#SkIPoint_fX">fX</a>, <a href="#SkIPoint_fY">fY</a> - <a href="#SkIPoint_subtractfrom_operator_v">v</a>.<a href="#SkIPoint_fY">fY</a>).

### Parameters

<table>  <tr>    <td><a name="SkIPoint_subtractfrom_operator_v"> <code><strong>v </strong></code> </a></td> <td>
<a href="SkIPoint_Reference#IVector">IVector</a> to subtract</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a01e533dc7ab34ed728dc4e7a5f1f0ee"></fiddle-embed></div>

### See Also

<a href="#SkIPoint_subtract_operator">operator-(const SkIPoint& a, const SkIPoint& b)</a> <a href="#SkPoint_subtractfrom_operator">SkPoint::operator-=(const SkVector& v)</a>

---

<a name="SkIPoint_equals"></a>
## equals

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool equals(int32_t x, int32_t y) const
</pre>

Returns true if <a href="#IPoint">IPoint</a> is equivalent to <a href="#IPoint">IPoint</a> constructed from (<a href="#SkIPoint_x">x</a>, <a href="#SkIPoint_y">y</a>).

### Parameters

<table>  <tr>    <td><a name="SkIPoint_equals_x"> <code><strong>x </strong></code> </a></td> <td>
value compared with <a href="#SkIPoint_fX">fX</a></td>
  </tr>  <tr>    <td><a name="SkIPoint_equals_y"> <code><strong>y </strong></code> </a></td> <td>
value compared with <a href="#SkIPoint_fY">fY</a></td>
  </tr>
</table>

### Return Value

true if <a href="#IPoint">IPoint</a> <a href="#SkIPoint_equals">equals</a> (<a href="#SkIPoint_x">x</a>, <a href="#SkIPoint_y">y</a>)

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

<a href="#SkIPoint_equal_operator">operator==(const SkIPoint& a, const SkIPoint& b)</a>

---

<a name="SkIPoint_equal_operator"></a>
## operator==

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool operator==(const SkIPoint& a, const SkIPoint& b)
</pre>

Returns true if <a href="#SkIPoint_equal_operator_a">a</a> is equivalent to <a href="#SkIPoint_equal_operator_b">b</a>.

### Parameters

<table>  <tr>    <td><a name="SkIPoint_equal_operator_a"> <code><strong>a </strong></code> </a></td> <td>
<a href="#IPoint">IPoint</a> to compare</td>
  </tr>  <tr>    <td><a name="SkIPoint_equal_operator_b"> <code><strong>b </strong></code> </a></td> <td>
<a href="#IPoint">IPoint</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href="#SkIPoint_equal_operator_a">a</a>.<a href="#SkIPoint_fX">fX</a> == <a href="#SkIPoint_equal_operator_b">b</a>.<a href="#SkIPoint_fX">fX</a> and <a href="#SkIPoint_equal_operator_a">a</a>.<a href="#SkIPoint_fY">fY</a> == <a href="#SkIPoint_equal_operator_b">b</a>.<a href="#SkIPoint_fY">fY</a>

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

<a href="#SkIPoint_equals">equals</a> <a href="#SkIPoint_notequal_operator">operator!=(const SkIPoint& a, const SkIPoint& b)</a>

---

<a name="SkIPoint_notequal_operator"></a>
## operator!=

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool operator!=(const SkIPoint& a, const SkIPoint& b)
</pre>

Returns true if <a href="#SkIPoint_notequal_operator_a">a</a> is not equivalent to <a href="#SkIPoint_notequal_operator_b">b</a>.

### Parameters

<table>  <tr>    <td><a name="SkIPoint_notequal_operator_a"> <code><strong>a </strong></code> </a></td> <td>
<a href="#IPoint">IPoint</a> to compare</td>
  </tr>  <tr>    <td><a name="SkIPoint_notequal_operator_b"> <code><strong>b </strong></code> </a></td> <td>
<a href="#IPoint">IPoint</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href="#SkIPoint_notequal_operator_a">a</a>.<a href="#SkIPoint_fX">fX</a> != <a href="#SkIPoint_notequal_operator_b">b</a>.<a href="#SkIPoint_fX">fX</a> or <a href="#SkIPoint_notequal_operator_a">a</a>.<a href="#SkIPoint_fY">fY</a> != <a href="#SkIPoint_notequal_operator_b">b</a>.<a href="#SkIPoint_fY">fY</a>

### Example

<div><fiddle-embed name="dd89dc48dff69b53d99530b120f204bc">

#### Example Output

~~~~
pt: 0, 0  == pt
pt: -1, -2  == pt
pt: 2147483647, -1  == pt
pt: -2147483648, -1  == pt
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIPoint_equal_operator">operator==(const SkIPoint& a, const SkIPoint& b)</a> <a href="#SkIPoint_equals">equals</a>

---

<a name="SkIPoint_subtract_operator"></a>
## operator-

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkIVector operator-(const SkIPoint& a, const SkIPoint& b)
</pre>

Returns <a href="SkIPoint_Reference#IVector">IVector</a> from <a href="#SkIPoint_subtract_operator_b">b</a> to <a href="#SkIPoint_subtract_operator_a">a</a>; computed as(<a href="#SkIPoint_subtract_operator_a">a</a>.<a href="#SkIPoint_fX">fX</a> - <a href="#SkIPoint_subtract_operator_b">b</a>.<a href="#SkIPoint_fX">fX</a>, <a href="#SkIPoint_subtract_operator_a">a</a>.<a href="#SkIPoint_fY">fY</a> - <a href="#SkIPoint_subtract_operator_b">b</a>.<a href="#SkIPoint_fY">fY</a>).

Can also be used to subtract <a href="SkIPoint_Reference#IVector">IVector</a> from <a href="SkIPoint_Reference#IVector">IVector</a>, returning <a href="SkIPoint_Reference#IVector">IVector</a>.

### Parameters

<table>  <tr>    <td><a name="SkIPoint_subtract_operator_a"> <code><strong>a </strong></code> </a></td> <td>
<a href="#IPoint">IPoint</a> or <a href="SkIPoint_Reference#IVector">IVector</a> to subtract from</td>
  </tr>  <tr>    <td><a name="SkIPoint_subtract_operator_b"> <code><strong>b </strong></code> </a></td> <td>
<a href="SkIPoint_Reference#IVector">IVector</a> to subtract</td>
  </tr>
</table>

### Return Value

<a href="SkIPoint_Reference#IVector">IVector</a> from <a href="#SkIPoint_subtract_operator_b">b</a> to <a href="#SkIPoint_subtract_operator_a">a</a>

### Example

<div><fiddle-embed name="e626e26bf557857b824aa7d03f723e0f"></fiddle-embed></div>

### See Also

<a href="#SkIPoint_subtractfrom_operator">operator-=(const SkIVector& v)</a>

---

<a name="SkIPoint_add_operator"></a>
## operator+

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkIPoint operator+(const SkIPoint& a, const SkIVector& b)
</pre>

Returns <a href="#IPoint">IPoint</a> resulting from <a href="#IPoint">IPoint</a> <a href="#SkIPoint_add_operator_a">a</a> offset by <a href="SkIPoint_Reference#IVector">IVector</a> <a href="#SkIPoint_add_operator_b">b</a>, computed as:
(<a href="#SkIPoint_add_operator_a">a</a>.<a href="#SkIPoint_fX">fX</a> + <a href="#SkIPoint_add_operator_b">b</a>.<a href="#SkIPoint_fX">fX</a>, <a href="#SkIPoint_add_operator_a">a</a>.<a href="#SkIPoint_fY">fY</a> + <a href="#SkIPoint_add_operator_b">b</a>.<a href="#SkIPoint_fY">fY</a>).

Can also be used to offset <a href="#IPoint">IPoint</a> <a href="#SkIPoint_add_operator_b">b</a> by <a href="SkIPoint_Reference#IVector">IVector</a> <a href="#SkIPoint_add_operator_a">a</a>, returning <a href="#IPoint">IPoint</a>.
Can also be used to add <a href="SkIPoint_Reference#IVector">IVector</a> to <a href="SkIPoint_Reference#IVector">IVector</a>, returning <a href="SkIPoint_Reference#IVector">IVector</a>.

### Parameters

<table>  <tr>    <td><a name="SkIPoint_add_operator_a"> <code><strong>a </strong></code> </a></td> <td>
<a href="#IPoint">IPoint</a> or <a href="SkIPoint_Reference#IVector">IVector</a> to add to</td>
  </tr>  <tr>    <td><a name="SkIPoint_add_operator_b"> <code><strong>b </strong></code> </a></td> <td>
<a href="#IPoint">IPoint</a> or <a href="SkIPoint_Reference#IVector">IVector</a> to add</td>
  </tr>
</table>

### Return Value

<a href="#IPoint">IPoint</a> equal to <a href="#SkIPoint_add_operator_a">a</a> offset by <a href="#SkIPoint_add_operator_b">b</a>

### Example

<div><fiddle-embed name="63f4cba971c6d8434595906f865b5a29"></fiddle-embed></div>

### See Also

<a href="#SkIPoint_addto_operator">operator+=(const SkIVector& v)</a>

---

# <a name="IVector"></a> IVector
