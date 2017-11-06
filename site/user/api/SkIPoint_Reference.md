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
| <a href="#SkIPoint">SkIPoint</a> <a href="#SkIPoint_minus_operator">operator-</a> const |  |
| friend <a href="#SkIPoint">SkIPoint</a> <a href="#SkIPoint_add_operator">operator+(const SkIPoint& a, const SkIPoint& b)</a> |  |
| friend <a href="#SkIPoint">SkIPoint</a> <a href="#SkIPoint_subtract_operator">operator-(const SkIPoint& a, const SkIPoint& b)</a> |  |
| friend bool <a href="#SkIPoint_notequal_operator">operator!=(const SkIPoint& a, const SkIPoint& b)</a> |  |
| friend bool <a href="#SkIPoint_equal_operator">operator==(const SkIPoint& a, const SkIPoint& b)</a> |  |
| void <a href="#SkIPoint_addto_operator">operator+=(const SkIPoint& v)</a> |  |
| void <a href="#SkIPoint_subtractfrom_operator">operator-=(const SkIPoint& v)</a> |  |

## <a name="Member_Functions"></a> Member Functions

| description | function |
| --- | ---  |
| <a href="#SkIPoint_CrossProduct">CrossProduct</a> |  |
| <a href="#SkIPoint_DotProduct">DotProduct</a> |  |
| <a href="#SkIPoint_Make">Make</a> |  |
| <a href="#SkIPoint_equals">equals</a> |  |
| <a href="#SkIPoint_isZero">isZero</a> |  |
| <a href="#SkIPoint_negate">negate</a> |  |
| <a href="#SkIPoint_rotateCCW">rotateCCW</a> |  |
| <a href="#SkIPoint_rotateCW">rotateCW</a> |  |
| <a href="#SkIPoint_set">set</a> |  |
| <a href="#SkIPoint_setX">setX</a> |  |
| <a href="#SkIPoint_setY">setY</a> |  |
| <a href="#SkIPoint_setZero">setZero</a> |  |
| <a href="#SkIPoint_x">x</a> |  |
| <a href="#SkIPoint_y">y</a> |  |

<a name="SkIPoint_fX"> <code><strong>int32_t  fX</strong></code> </a>

<a name="SkIPoint_fY"> <code><strong>int32_t  fY</strong></code> </a>

<a name="SkIPoint_Make"></a>
## Make

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static SkIPoint Make(int32_t x, int32_t y)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkIPoint_Make_x"> <code><strong>x </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkIPoint_Make_y"> <code><strong>y </strong></code> </a></td> <td>
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

<a name="SkIPoint_x"></a>
## x

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
int32_t x() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkIPoint_y"></a>
## y

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
int32_t y() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkIPoint_setX"></a>
## setX

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setX(int32_t x)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkIPoint_setX_x"> <code><strong>x </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkIPoint_setY"></a>
## setY

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setY(int32_t y)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkIPoint_setY_y"> <code><strong>y </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkIPoint_isZero"></a>
## isZero

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isZero() const
</pre>

Returns true iff <a href="#SkIPoint_fX">fX</a> and <a href="#SkIPoint_fY">fY</a> are both zero.

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkIPoint_setZero"></a>
## setZero

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setZero()
</pre>

Set both <a href="#SkIPoint_fX">fX</a> and <a href="#SkIPoint_fY">fY</a> to zero. Same as

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkIPoint_set"></a>
## set

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void set(int32_t x, int32_t y)
</pre>

Set the <a href="#SkIPoint_x">x</a> and <a href="#SkIPoint_y">y</a> values of the point.

### Parameters

<table>  <tr>    <td><a name="SkIPoint_set_x"> <code><strong>x </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkIPoint_set_y"> <code><strong>y </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkIPoint_rotateCW"></a>
## rotateCW

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void rotateCW(SkIPoint* dst) const
</pre>

Rotate the point clockwise, writing the new point into <a href="#SkIPoint_rotateCW_dst">dst</a>.
It is legal for <a href="#SkIPoint_rotateCW_dst">dst</a> == this

### Parameters

<table>  <tr>    <td><a name="SkIPoint_rotateCW_dst"> <code><strong>dst </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void rotateCW()
</pre>

Rotate the point clockwise, writing the new point back into the point

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkIPoint_rotateCCW"></a>
## rotateCCW

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void rotateCCW(SkIPoint* dst) const
</pre>

Rotate the point counter-clockwise, writing the new point into <a href="#SkIPoint_rotateCCW_dst">dst</a>.
It is legal for <a href="#SkIPoint_rotateCCW_dst">dst</a> == this

### Parameters

<table>  <tr>    <td><a name="SkIPoint_rotateCCW_dst"> <code><strong>dst </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void rotateCCW()
</pre>

Rotate the point counter-clockwise, writing the new point back into
the point

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkIPoint_negate"></a>
## negate

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void negate()
</pre>

Negate the <a href="#SkIPoint_x">x</a> and <a href="#SkIPoint_y">y</a> coordinates of the point.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

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

