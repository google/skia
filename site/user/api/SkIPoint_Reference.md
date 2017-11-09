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
| <a href="#SkIPoint">SkIPoint</a> <a href="#SkIPoint_minus_operator">operator-</a> const | Reverses the sign of both members. |
| <a href="#SkIPoint">SkIPoint</a> <a href="#SkIPoint_add_operator">operator+(const SkIPoint& a, const SkIPoint& b)</a> | Returns <a href="#IPoint">IPoint</a> offset by vector. |
| <a href="#SkIPoint">SkIPoint</a> <a href="#SkIPoint_subtract_operator">operator-(const SkIPoint& a, const SkIPoint& b)</a> | Returns vector between <a href="#IPoint">IPoints</a>. |
| bool <a href="#SkIPoint_notequal_operator">operator!=(const SkIPoint& a, const SkIPoint& b)</a> | Returns true if members are unequal. |
| bool <a href="#SkIPoint_equal_operator">operator==(const SkIPoint& a, const SkIPoint& b)</a> | Returns true if members are equal. |
| void <a href="#SkIPoint_addto_operator">operator+=(const SkIPoint& v)</a> | Offsets members by vector. |
| void <a href="#SkIPoint_subtractfrom_operator">operator-=(const SkIPoint& v)</a> | Offsets <a href="SkPoint_Reference#Point">Point</a> opposite of vector. |

## <a name="Member_Functions"></a> Member Functions

| description | function |
| --- | ---  |
| <a href="#SkIPoint_Make">Make</a> | Constructs from integer inputs. |
| <a href="#SkIPoint_equals">equals</a> | Returns true if members are equal. |
| <a href="#SkIPoint_isZero">isZero</a> | Returns true if both members equal zero. |
| <a href="#SkIPoint_set">set</a> | Sets to <a href="undocumented#SkScalar">SkScalar</a> input. |
| <a href="#SkIPoint_x">x</a> | Returns <a href="#SkIPoint_fX">fX</a>. |
| <a href="#SkIPoint_y">y</a> | Returns <a href="#SkIPoint_fY">fY</a>. |

<a name="SkIPoint_fX"> <code><strong>int32_t  fX</strong></code> </a>

x-axis value used by <a href="#IPoint">IPoint</a>. May contain any value, including
infinities and <a href="undocumented#NaN">NaN</a>.

<a name="SkIPoint_fY"> <code><strong>int32_t  fY</strong></code> </a>

y-axis value used by <a href="#IPoint">IPoint</a>. May contain any value, including
infinities and <a href="undocumented#NaN">NaN</a>.

<a name="SkIPoint_Make"></a>
## Make

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static SkIPoint Make(int32_t x, int32_t y)
</pre>

Sets <a href="#SkIPoint_fX">fX</a> to <a href="#SkIPoint_x">x</a>, <a href="#SkIPoint_fY">fY</a> to <a href="#SkIPoint_y">y</a>.

### Parameters

<table>  <tr>    <td><a name="SkIPoint_Make_x"> <code><strong>x </strong></code> </a></td> <td>
integer x-axis value of constructed <a href="#IPoint">IPoint</a></td>
  </tr>  <tr>    <td><a name="SkIPoint_Make_y"> <code><strong>y </strong></code> </a></td> <td>
integer y-axis value of constructed <a href="#IPoint">IPoint</a></td>
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

<a href="#SkIPoint_set">set</a> <a href="#SkPoint_iset">SkPoint::iset</a> <a href="#SkPoint_Make">SkPoint::Make</a>

---

<a name="SkIPoint_x"></a>
## x

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
int32_t x() const
</pre>

Returns x-axis value of <a href="#IPoint">IPoint</a>.

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

<a href="#SkIPoint_y">y</a> <a href="#SkPoint_x">SkPoint::x()</a>

---

<a name="SkIPoint_y"></a>
## y

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
int32_t y() const
</pre>

Returns y-axis value of <a href="#IPoint">IPoint</a>.

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

<a href="#SkIPoint_x">x</a> <a href="#SkPoint_y">SkPoint::y()</a>

---

<a name="SkIPoint_isZero"></a>
## isZero

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
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

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
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

<a href="#SkIPoint_Make">Make</a>

---

<a name="SkIPoint_minus_operator"></a>
## operator-

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkIPoint operator-() const
</pre>

Return a new point whose <a href="#SkIPoint_x">x</a> and <a href="#SkIPoint_y">y</a> coordinates are the negative of the
original point's

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkIPoint_addto_operator"></a>
## operator+=

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void operator+=(const SkIPoint& v)
</pre>

Add <a href="#SkIPoint_addto_operator_v">v</a>'s coordinates to this point's

### Parameters

<table>  <tr>    <td><a name="SkIPoint_addto_operator_v"> <code><strong>v </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkIPoint_subtractfrom_operator"></a>
## operator-=

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void operator-=(const SkIPoint& v)
</pre>

Subtract <a href="#SkIPoint_subtractfrom_operator_v">v</a>'s coordinates from this point's

### Parameters

<table>  <tr>    <td><a name="SkIPoint_subtractfrom_operator_v"> <code><strong>v </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkIPoint_equals"></a>
## equals

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool equals(int32_t x, int32_t y) const
</pre>

Returns true if the point's coordinates equal (<a href="#SkIPoint_x">x</a>,<a href="#SkIPoint_y">y</a>)

### Parameters

<table>  <tr>    <td><a name="SkIPoint_equals_x"> <code><strong>x </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkIPoint_equals_y"> <code><strong>y </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkIPoint_equal_operator"></a>
## operator==

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
friend bool operator==(const SkIPoint& a, const SkIPoint& b)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkIPoint_equal_operator_a"> <code><strong>a </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkIPoint_equal_operator_b"> <code><strong>b </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkIPoint_notequal_operator"></a>
## operator!=

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
friend bool operator!=(const SkIPoint& a, const SkIPoint& b)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkIPoint_notequal_operator_a"> <code><strong>a </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkIPoint_notequal_operator_b"> <code><strong>b </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkIPoint_subtract_operator"></a>
## operator-

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
friend SkIPoint operator-(const SkIPoint& a, const SkIPoint& b)
</pre>

Returns <a href="#SkIPoint_subtract_operator_a">a</a> new point whose coordinates are the difference between
<a href="#SkIPoint_subtract_operator_a">a</a> and <a href="#SkIPoint_subtract_operator_b">b</a> (i.e. <a href="#SkIPoint_subtract_operator_a">a</a> - <a href="#SkIPoint_subtract_operator_b">b</a>)

### Parameters

<table>  <tr>    <td><a name="SkIPoint_subtract_operator_a"> <code><strong>a </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkIPoint_subtract_operator_b"> <code><strong>b </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkIPoint_add_operator"></a>
## operator+

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
friend SkIPoint operator+(const SkIPoint& a, const SkIPoint& b)
</pre>

Returns <a href="#SkIPoint_add_operator_a">a</a> new point whose coordinates are the sum of <a href="#SkIPoint_add_operator_a">a</a> and <a href="#SkIPoint_add_operator_b">b</a> (<a href="#SkIPoint_add_operator_a">a</a> + <a href="#SkIPoint_add_operator_b">b</a>)

### Parameters

<table>  <tr>    <td><a name="SkIPoint_add_operator_a"> <code><strong>a </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkIPoint_add_operator_b"> <code><strong>b </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkIPoint_DotProduct"></a>
## DotProduct

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static int32_t DotProduct(const SkIPoint& a, const SkIPoint& b)
</pre>

Returns the dot product of <a href="#SkIPoint_DotProduct_a">a</a> and <a href="#SkIPoint_DotProduct_b">b</a>, treating them as two dimensional vectors

### Parameters

<table>  <tr>    <td><a name="SkIPoint_DotProduct_a"> <code><strong>a </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkIPoint_DotProduct_b"> <code><strong>b </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkIPoint_CrossProduct"></a>
## CrossProduct

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static int32_t CrossProduct(const SkIPoint& a, const SkIPoint& b)
</pre>

Returns the cross product of <a href="#SkIPoint_CrossProduct_a">a</a> and <a href="#SkIPoint_CrossProduct_b">b</a>, treating them as two dimensional vectors

### Parameters

<table>  <tr>    <td><a name="SkIPoint_CrossProduct_a"> <code><strong>a </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkIPoint_CrossProduct_b"> <code><strong>b </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

