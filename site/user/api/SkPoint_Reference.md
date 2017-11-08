SkPoint Reference
===

# <a name="Vector"></a> Vector

# <a name="Point"></a> Point

# <a name="SkPoint"></a> Struct SkPoint

# <a name="Overview"></a> Overview

## <a name="Subtopics"></a> Subtopics

| topics | description |
| --- | ---  |

## <a name="Operators"></a> Operators

| description | function |
| --- | ---  |
| <a href="#SkPoint">SkPoint</a> <a href="#SkPoint_multiply_operator">operator*(SkScalar scale)</a> const | Returns members by scaled. |
| <a href="#SkPoint">SkPoint</a> <a href="#SkPoint_minus_operator">operator-</a> const | Reverses the sign of both members. |
| <a href="#SkPoint">SkPoint</a>& <a href="#SkPoint_multiplyby_operator">operator*=(SkScalar scale)</a> | Multiplies members by <a href="#SkPoint_scale">scale</a> factor. |
| <a href="#SkPoint">SkPoint</a> <a href="#SkPoint_add_operator">operator+(const SkPoint& a, const SkPoint& b)</a> | Returns <a href="#Point">Point</a> <a href="#SkPoint_offset">offset</a> by second <a href="#Point">Point</a>. |
| <a href="#SkPoint">SkPoint</a> <a href="#SkPoint_subtract_operator">operator-(const SkPoint& a, const SkPoint& b)</a> | Returns difference between <a href="#Point">Points</a>. |
| bool <a href="#SkPoint_notequal_operator">operator!=(const SkPoint& a, const SkPoint& b)</a> | Returns true if members are unequal. |
| bool <a href="#SkPoint_equal_operator">operator==(const SkPoint& a, const SkPoint& b)</a> | Returns true if members are equal. |
| void <a href="#SkPoint_addto_operator">operator+=(const SkPoint& v)</a> | Offsets members by <a href="SkPoint_Reference#Vector">Vector</a>. |
| void <a href="#SkPoint_subtractfrom_operator">operator-=(const SkPoint& v)</a> | Offsets members opposite of <a href="SkPoint_Reference#Vector">Vector</a>. |

## <a name="Member_Functions"></a> Member Functions

| description | function |
| --- | ---  |
| <a href="#SkPoint_CanNormalize">CanNormalize</a> | Returns if normal can be computed. |
| <a href="#SkPoint_CrossProduct">CrossProduct</a> | Returns <a href="#SkPoint_cross">cross</a> product. |
| <a href="#SkPoint_Distance">Distance</a> | Returns straight-line distance between points. |
| <a href="#SkPoint_DotProduct">DotProduct</a> | Returns <a href="#SkPoint_dot">dot</a> product. |
| <a href="#SkPoint_Length">Length</a> | Returns straight-line distance to origin. |
| <a href="#SkPoint_Make">Make</a> | Constructs from <a href="undocumented#SkScalar">SkScalar</a> inputs. |
| <a href="#SkPoint_Normalize">Normalize</a> | Sets <a href="#SkPoint_length">length</a> to one, and returns prior <a href="#SkPoint_length">length</a>. |
| <a href="#SkPoint_Offset">Offset</a> | Translates <a href="#Point">Point</a> array. |
| <a href="#SkPoint_asScalars">asScalars</a> | Returns <a href="#Point">Point</a> as <a href="undocumented#SkScalar">SkScalar</a> array. |
| <a href="#SkPoint_canNormalize">canNormalize</a> | Returns if normal can be computed. |
| <a href="#SkPoint_cross">cross</a> | Returns <a href="#SkPoint_cross">cross</a> product. |
| <a href="#SkPoint_distanceToLineBetween">distanceToLineBetween</a> | Returns distance to line described by <a href="#Point">Point</a> pair. |
| <a href="#SkPoint_distanceToLineBetweenSqd">distanceToLineBetweenSqd</a> | Returns square of distance to line. |
| <a href="#SkPoint_distanceToLineSegmentBetween">distanceToLineSegmentBetween</a> | Returns distance to line segment. |
| <a href="#SkPoint_distanceToLineSegmentBetweenSqd">distanceToLineSegmentBetweenSqd</a> | Returns square of distance to line segment. |
| <a href="#SkPoint_distanceToOrigin">distanceToOrigin</a> | Returns straight-line distance to origin. |
| <a href="#SkPoint_distanceToSqd">distanceToSqd</a> | Returns square of straight-line distance to origin. |
| <a href="#SkPoint_dot">dot</a> | Returns <a href="#SkPoint_dot">dot</a> product. |
| <a href="#SkPoint_equals">equals</a> | Returns true if members are equal. |
| <a href="#SkPoint_equalsWithinTolerance">equalsWithinTolerance</a> | Returns true if members are nearly equal. |
| <a href="#SkPoint_isFinite">isFinite</a> | Returns true if no member is infinite or <a href="undocumented#NaN">NaN</a>. |
| <a href="#SkPoint_isZero">isZero</a> | Returns true if both members equal zero. |
| <a href="#SkPoint_iset">iset</a> | Sets to integer input. |
| <a href="#SkPoint_length">length</a> | Returns straight-line distance to origin. |
| <a href="#SkPoint_lengthSqd">lengthSqd</a> | Returns square of straight-line distance to origin. |
| <a href="#SkPoint_negate">negate</a> | Reverses the sign of both members. |
| <a href="#SkPoint_normalize">normalize</a> | Sets <a href="#SkPoint_length">length</a> to one, preserving direction. |
| <a href="#SkPoint_offset">offset</a> | Translates members. |
| <a href="#SkPoint_rotateCCW">rotateCCW</a> | Rotates counterclockwise by 90 degrees. |
| <a href="#SkPoint_rotateCW">rotateCW</a> | Rotates clockwise by 90 degrees. |
| <a href="#SkPoint_scale">scale</a> | Multiplies members by <a href="#SkPoint_scale">scale</a> factor. |
| <a href="#SkPoint_set">set</a> | Sets to <a href="undocumented#SkScalar">SkScalar</a> input. |
| <a href="#SkPoint_setAbs">setAbs</a> | Sets sign of both members to positive. |
| <a href="#SkPoint_setIRectFan">setIRectFan</a> | Deprecated. |
| <a href="#SkPoint_setLength">setLength</a> | Sets straight-line distance to origin. |
| <a href="#SkPoint_setLengthFast">setLengthFast</a> | Sets approximate straight-line distance to origin. |
| <a href="#SkPoint_setNormalize">setNormalize</a> | Sets <a href="#SkPoint_length">length</a> to one, in direction of (<a href="#SkPoint_x">x</a>, <a href="#SkPoint_y">y</a>). |
| <a href="#SkPoint_setOrthog">setOrthog</a> | Sets <a href="SkPoint_Reference#Vector">Vector</a> perpendicular to reference. |
| <a href="#SkPoint_setRectFan">setRectFan</a> | Deprecated. |
| <a href="#SkPoint_setRectTriStrip">setRectTriStrip</a> | Deprecated. |
| <a href="#SkPoint_x">x</a> | Returns <a href="#SkPoint_fX">fX</a>. |
| <a href="#SkPoint_y">y</a> | Returns <a href="#SkPoint_fY">fY</a>. |

<a name="SkPoint_fX"> <code><strong>SkScalar  fX</strong></code> </a>

x-axis value used by both <a href="#Point">Point</a> and <a href="SkPoint_Reference#Vector">Vector</a>. May contain any value, including
infinities and <a href="undocumented#NaN">NaN</a>.

<a name="SkPoint_fY"> <code><strong>SkScalar  fY</strong></code> </a>

y-axis value used by both <a href="#Point">Point</a> and <a href="SkPoint_Reference#Vector">Vector</a>. May contain any value, including
infinities and <a href="undocumented#NaN">NaN</a>.

<a name="SkPoint_Make"></a>
## Make

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static SkPoint Make(SkScalar x, SkScalar y)
</pre>

Sets <a href="#SkPoint_fX">fX</a> to <a href="#SkPoint_x">x</a>, <a href="#SkPoint_fY">fY</a> to <a href="#SkPoint_y">y</a>. Used both to <a href="#SkPoint_set">set</a> <a href="#Point">Point</a> and <a href="SkPoint_Reference#Vector">Vector</a>.

### Parameters

<table>  <tr>    <td><a name="SkPoint_Make_x"> <code><strong>x </strong></code> </a></td> <td>
<a href="undocumented#SkScalar">SkScalar</a> x-axis value of constructed <a href="#Point">Point</a> or <a href="SkPoint_Reference#Vector">Vector</a></td>
  </tr>  <tr>    <td><a name="SkPoint_Make_y"> <code><strong>y </strong></code> </a></td> <td>
<a href="undocumented#SkScalar">SkScalar</a> y-axis value of constructed <a href="#Point">Point</a> or <a href="SkPoint_Reference#Vector">Vector</a></td>
  </tr>
</table>

### Return Value

<a href="#Point">Point</a> (<a href="#SkPoint_x">x</a>, <a href="#SkPoint_y">y</a>)

### Example

<div><fiddle-embed name="d266e70977847001f7c42f8a2513bee7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_x"></a>
## x

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar x() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_y"></a>
## y

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar y() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_isZero"></a>
## isZero

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isZero() const
</pre>

Returns true iff <a href="#SkPoint_fX">fX</a> and <a href="#SkPoint_fY">fY</a> are both zero.

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_set"></a>
## set

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void set(SkScalar x, SkScalar y)
</pre>

Sets <a href="#SkPoint_fX">fX</a> to <a href="#SkPoint_x">x</a> and <a href="#SkPoint_fY">fY</a> to <a href="#SkPoint_y">y</a>.

### Parameters

<table>  <tr>    <td><a name="SkPoint_set_x"> <code><strong>x </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_set_y"> <code><strong>y </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_iset"></a>
## iset

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void iset(int32_t x, int32_t y)
</pre>

Sets <a href="#SkPoint_fX">fX</a> to <a href="#SkPoint_x">x</a> and <a href="#SkPoint_fY">fY</a> to <a href="#SkPoint_y">y</a>, promoting integers to <a href="undocumented#SkScalar">SkScalar</a> values.

### Parameters

<table>  <tr>    <td><a name="SkPoint_iset_x"> <code><strong>x </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_iset_y"> <code><strong>y </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void iset(const SkIPoint& p)
</pre>

Sets <a href="#SkPoint_fX">fX</a> to <a href="#SkPoint_iset_2_p">p</a>.<a href="#SkPoint_fX">fX</a> and <a href="#SkPoint_fY">fY</a> to <a href="#SkPoint_iset_2_p">p</a>.<a href="#SkPoint_fY">fY</a>, promoting integers to <a href="undocumented#SkScalar">SkScalar</a> values.

### Parameters

<table>  <tr>    <td><a name="SkPoint_iset_2_p"> <code><strong>p </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_setAbs"></a>
## setAbs

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setAbs(const SkPoint& pt)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkPoint_setAbs_pt"> <code><strong>pt </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_setIRectFan"></a>
## setIRectFan

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setIRectFan(int l, int t, int r, int b)
</pre>

counter-clockwise fan

### Parameters

<table>  <tr>    <td><a name="SkPoint_setIRectFan_l"> <code><strong>l </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_setIRectFan_t"> <code><strong>t </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_setIRectFan_r"> <code><strong>r </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_setIRectFan_b"> <code><strong>b </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setIRectFan(int l, int t, int r, int b, size_t stride)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkPoint_setIRectFan_2_l"> <code><strong>l </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_setIRectFan_2_t"> <code><strong>t </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_setIRectFan_2_r"> <code><strong>r </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_setIRectFan_2_b"> <code><strong>b </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_setIRectFan_2_stride"> <code><strong>stride </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_setRectFan"></a>
## setRectFan

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setRectFan(SkScalar l, SkScalar t, SkScalar r, SkScalar b, size_t stride)
</pre>

counter-clockwise fan

### Parameters

<table>  <tr>    <td><a name="SkPoint_setRectFan_l"> <code><strong>l </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_setRectFan_t"> <code><strong>t </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_setRectFan_r"> <code><strong>r </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_setRectFan_b"> <code><strong>b </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_setRectFan_stride"> <code><strong>stride </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_setRectTriStrip"></a>
## setRectTriStrip

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setRectTriStrip(SkScalar l, SkScalar t, SkScalar r, SkScalar b, size_t stride)
</pre>

tri strip with two counter-clockwise triangles

### Parameters

<table>  <tr>    <td><a name="SkPoint_setRectTriStrip_l"> <code><strong>l </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_setRectTriStrip_t"> <code><strong>t </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_setRectTriStrip_r"> <code><strong>r </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_setRectTriStrip_b"> <code><strong>b </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_setRectTriStrip_stride"> <code><strong>stride </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_Offset"></a>
## Offset

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static void Offset(SkPoint points[], int count, const SkPoint& offset)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkPoint_Offset_points"> <code><strong>points </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_Offset_count"> <code><strong>count </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_Offset_offset"> <code><strong>offset </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static void Offset(SkPoint points[], int count, SkScalar dx, SkScalar dy)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkPoint_Offset_2_points"> <code><strong>points </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_Offset_2_count"> <code><strong>count </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_Offset_2_dx"> <code><strong>dx </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_Offset_2_dy"> <code><strong>dy </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_offset"></a>
## offset

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void offset(SkScalar dx, SkScalar dy)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkPoint_offset_dx"> <code><strong>dx </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_offset_dy"> <code><strong>dy </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_length"></a>
## length

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar length() const
</pre>

Return the euclidian distance from (0,0) to the point

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_distanceToOrigin"></a>
## distanceToOrigin

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar distanceToOrigin() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_CanNormalize"></a>
## CanNormalize

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static bool CanNormalize(SkScalar dx, SkScalar dy)
</pre>

Return true if the computed <a href="#SkPoint_length">length</a> of the vector is >= the internal
tolerance (used to avoid dividing by tiny values).

### Parameters

<table>  <tr>    <td><a name="SkPoint_CanNormalize_dx"> <code><strong>dx </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_CanNormalize_dy"> <code><strong>dy </strong></code> </a></td> <td>
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

<a name="SkPoint_canNormalize"></a>
## canNormalize

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool canNormalize() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_normalize"></a>
## normalize

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool normalize()
</pre>

Set the point (vector) to be unit-length in the same direction as it
already points.  If the point has a degenerate <a href="#SkPoint_length">length</a> (i.e. nearly 0)
then <a href="#SkPoint_set">set</a> it to (0,0) and return false; otherwise return true.

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_setNormalize"></a>
## setNormalize

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool setNormalize(SkScalar x, SkScalar y)
</pre>

Set the point (vector) to be unit-length in the same direction as the
<a href="#SkPoint_x">x</a>,<a href="#SkPoint_y">y</a> params. If the vector (<a href="#SkPoint_x">x</a>,<a href="#SkPoint_y">y</a>) has a degenerate <a href="#SkPoint_length">length</a> (i.e. nearly 0)
then <a href="#SkPoint_set">set</a> it to (0,0) and return false, otherwise return true.

### Parameters

<table>  <tr>    <td><a name="SkPoint_setNormalize_x"> <code><strong>x </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_setNormalize_y"> <code><strong>y </strong></code> </a></td> <td>
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

<a name="SkPoint_setLength"></a>
## setLength

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool setLength(SkScalar length)
</pre>

Scale the point (vector) to have the specified <a href="#SkPoint_length">length</a>, and return that
<a href="#SkPoint_length">length</a>. If the original <a href="#SkPoint_length">length</a> is degenerately small (nearly zero),
<a href="#SkPoint_set">set</a> it to (0,0) and return false, otherwise return true.

### Parameters

<table>  <tr>    <td><a name="SkPoint_setLength_length"> <code><strong>length </strong></code> </a></td> <td>
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

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool setLength(SkScalar x, SkScalar y, SkScalar length)
</pre>

Set the point (vector) to have the specified <a href="#SkPoint_length">length</a> in the same
direction as (<a href="#SkPoint_x">x</a>,<a href="#SkPoint_y">y</a>). If the vector (<a href="#SkPoint_x">x</a>,<a href="#SkPoint_y">y</a>) has a degenerate <a href="#SkPoint_length">length</a>
(i.e. nearly 0) then <a href="#SkPoint_set">set</a> it to (0,0) and return false, otherwise return true.

### Parameters

<table>  <tr>    <td><a name="SkPoint_setLength_2_x"> <code><strong>x </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_setLength_2_y"> <code><strong>y </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_setLength_2_length"> <code><strong>length </strong></code> </a></td> <td>
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

<a name="SkPoint_setLengthFast"></a>
## setLengthFast

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool setLengthFast(SkScalar length)
</pre>

Same as <a href="#SkPoint_setLength">setLength</a>, but favoring speed over accuracy.

### Parameters

<table>  <tr>    <td><a name="SkPoint_setLengthFast_length"> <code><strong>length </strong></code> </a></td> <td>
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

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool setLengthFast(SkScalar x, SkScalar y, SkScalar length)
</pre>

Same as <a href="#SkPoint_setLength">setLength</a>, but favoring speed over accuracy.

### Parameters

<table>  <tr>    <td><a name="SkPoint_setLengthFast_2_x"> <code><strong>x </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_setLengthFast_2_y"> <code><strong>y </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_setLengthFast_2_length"> <code><strong>length </strong></code> </a></td> <td>
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

<a name="SkPoint_scale"></a>
## scale

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void scale(SkScalar scale, SkPoint* dst) const
</pre>

Scale the point's coordinates by <a href="#SkPoint_scale">scale</a>, writing the answer into <a href="#SkPoint_scale_dst">dst</a>.
It is legal for <a href="#SkPoint_scale_dst">dst</a> == this.

### Parameters

<table>  <tr>    <td><a name="SkPoint_scale_scale"> <code><strong>scale </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_scale_dst"> <code><strong>dst </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void scale(SkScalar value)
</pre>

Scale the point's coordinates by <a href="#SkPoint_scale">scale</a>, writing the answer back into
the point.

### Parameters

<table>  <tr>    <td><a name="SkPoint_scale_2_value"> <code><strong>value </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_rotateCW"></a>
## rotateCW

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void rotateCW(SkPoint* dst) const
</pre>

Rotate the point clockwise by 90 degrees, writing the answer into <a href="#SkPoint_rotateCW_dst">dst</a>.
It is legal for <a href="#SkPoint_rotateCW_dst">dst</a> == this.

### Parameters

<table>  <tr>    <td><a name="SkPoint_rotateCW_dst"> <code><strong>dst </strong></code> </a></td> <td>
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

Rotate the point clockwise by 90 degrees, writing the answer back into
the point.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_rotateCCW"></a>
## rotateCCW

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void rotateCCW(SkPoint* dst) const
</pre>

Rotate the point counter-clockwise by 90 degrees, writing the answer
into <a href="#SkPoint_rotateCCW_dst">dst</a>. It is legal for <a href="#SkPoint_rotateCCW_dst">dst</a> == this.

### Parameters

<table>  <tr>    <td><a name="SkPoint_rotateCCW_dst"> <code><strong>dst </strong></code> </a></td> <td>
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

Rotate the point counter-clockwise by 90 degrees, writing the answer
back into the point.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_negate"></a>
## negate

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void negate()
</pre>

Negate the point's coordinates

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_minus_operator"></a>
## operator-

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkPoint operator-() const
</pre>

Returns a new point whose coordinates are the negative of the point's

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_addto_operator"></a>
## operator+=

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void operator+=(const SkPoint& v)
</pre>

Add <a href="#SkPoint_addto_operator_v">v</a>'s coordinates to the point's

### Parameters

<table>  <tr>    <td><a name="SkPoint_addto_operator_v"> <code><strong>v </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_subtractfrom_operator"></a>
## operator-=

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void operator-=(const SkPoint& v)
</pre>

Subtract <a href="#SkPoint_subtractfrom_operator_v">v</a>'s coordinates from the point's

### Parameters

<table>  <tr>    <td><a name="SkPoint_subtractfrom_operator_v"> <code><strong>v </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_multiply_operator"></a>
## operator*

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkPoint operator*(SkScalar scale) const
</pre>

### Parameters

<table>  <tr>    <td><a name="SkPoint_multiply_operator_scale"> <code><strong>scale </strong></code> </a></td> <td>
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

<a name="SkPoint_multiplyby_operator"></a>
## operator*=

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkPoint& operator*=(SkScalar scale)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkPoint_multiplyby_operator_scale"> <code><strong>scale </strong></code> </a></td> <td>
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

<a name="SkPoint_isFinite"></a>
## isFinite

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isFinite() const
</pre>

Returns true if both <a href="#SkPoint_fX">fX</a> and <a href="#SkPoint_fY">fY</a> are finite (not infinity or <a href="undocumented#NaN">NaN</a>)

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_equals"></a>
## equals

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool equals(SkScalar x, SkScalar y) const
</pre>

Returns true if the point's coordinates equal (<a href="#SkPoint_x">x</a>, <a href="#SkPoint_y">y</a>)

### Parameters

<table>  <tr>    <td><a name="SkPoint_equals_x"> <code><strong>x </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_equals_y"> <code><strong>y </strong></code> </a></td> <td>
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

<a name="SkPoint_equal_operator"></a>
## operator==

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
friend bool operator==(const SkPoint& a, const SkPoint& b)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkPoint_equal_operator_a"> <code><strong>a </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_equal_operator_b"> <code><strong>b </strong></code> </a></td> <td>
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

<a name="SkPoint_notequal_operator"></a>
## operator!=

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
friend bool operator!=(const SkPoint& a, const SkPoint& b)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkPoint_notequal_operator_a"> <code><strong>a </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_notequal_operator_b"> <code><strong>b </strong></code> </a></td> <td>
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

<a name="SkPoint_equalsWithinTolerance"></a>
## equalsWithinTolerance

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool equalsWithinTolerance(const SkPoint& p) const
</pre>

Return true if this point and the given point are far enough apart
such that a vector between them would be non-degenerate.

Unlike the explicit tolerance version,
this method does not use componentwise comparison.  Instead, it
uses a comparison designed to match judgments elsewhere regarding
degeneracy ("this and <a href="#SkPoint_equalsWithinTolerance_p">p</a> are so close that the vector between them
is essentially zero").

### Parameters

<table>  <tr>    <td><a name="SkPoint_equalsWithinTolerance_p"> <code><strong>p </strong></code> </a></td> <td>
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

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool equalsWithinTolerance(const SkPoint& p, SkScalar tol) const
</pre>

There is no guarantee that the result will reflect judgments
elsewhere regarding degeneracy ("this and <a href="#SkPoint_equalsWithinTolerance_2_p">p</a> are so close that the
vector between them is essentially zero").

### Parameters

<table>  <tr>    <td><a name="SkPoint_equalsWithinTolerance_2_p"> <code><strong>p </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_equalsWithinTolerance_2_tol"> <code><strong>tol </strong></code> </a></td> <td>
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

<a name="SkPoint_subtract_operator"></a>
## operator-

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
friend SkPoint operator-(const SkPoint& a, const SkPoint& b)
</pre>

Returns <a href="#SkPoint_subtract_operator_a">a</a> new point(<a href="#SkPoint_subtract_operator_a">a</a>.<a href="#SkPoint_fX">fX</a> - <a href="#SkPoint_subtract_operator_b">b</a>.<a href="#SkPoint_fX">fX</a>, <a href="#SkPoint_subtract_operator_a">a</a>.<a href="#SkPoint_fY">fY</a> - <a href="#SkPoint_subtract_operator_b">b</a>.<a href="#SkPoint_fY">fY</a>).

### Parameters

<table>  <tr>    <td><a name="SkPoint_subtract_operator_a"> <code><strong>a </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_subtract_operator_b"> <code><strong>b </strong></code> </a></td> <td>
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

<a name="SkPoint_add_operator"></a>
## operator+

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
friend SkPoint operator+(const SkPoint& a, const SkPoint& b)
</pre>

Returns <a href="#SkPoint_add_operator_a">a</a> new point whose coordinates are the sum of <a href="#SkPoint_add_operator_a">a</a>'s and <a href="#SkPoint_add_operator_b">b</a>'s (<a href="#SkPoint_add_operator_a">a</a> + <a href="#SkPoint_add_operator_b">b</a>)

### Parameters

<table>  <tr>    <td><a name="SkPoint_add_operator_a"> <code><strong>a </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_add_operator_b"> <code><strong>b </strong></code> </a></td> <td>
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

<a name="SkPoint_Length"></a>
## Length

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static SkScalar Length(SkScalar x, SkScalar y)
</pre>

Returns the euclidian distance from (0,0) to (<a href="#SkPoint_x">x</a>,<a href="#SkPoint_y">y</a>)

### Parameters

<table>  <tr>    <td><a name="SkPoint_Length_x"> <code><strong>x </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_Length_y"> <code><strong>y </strong></code> </a></td> <td>
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

<a name="SkPoint_Normalize"></a>
## Normalize

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static SkScalar Normalize(SkPoint* pt)
</pre>

<a href="#SkPoint_Normalize">Normalize</a> <a href="#SkPoint_Normalize_pt">pt</a>, returning its previous <a href="#SkPoint_length">length</a>. If the prev <a href="#SkPoint_length">length</a> is too
small (degenerate), <a href="#SkPoint_set">set</a> <a href="#SkPoint_Normalize_pt">pt</a> to (0,0) and return 0. This uses the same
tolerance as <a href="#SkPoint_CanNormalize">CanNormalize</a>.
Note that this method may be significantly more expensive than
the non-static <a href="#SkPoint_normalize">normalize</a>, because it has to return the previous <a href="#SkPoint_length">length</a>
of the point.  If you don't need the previous <a href="#SkPoint_length">length</a>, call the
non-static <a href="#SkPoint_normalize">normalize</a> method instead.

### Parameters

<table>  <tr>    <td><a name="SkPoint_Normalize_pt"> <code><strong>pt </strong></code> </a></td> <td>
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

<a name="SkPoint_Distance"></a>
## Distance

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static SkScalar Distance(const SkPoint& a, const SkPoint& b)
</pre>

Returns the euclidian distance between <a href="#SkPoint_Distance_a">a</a> and <a href="#SkPoint_Distance_b">b</a>

### Parameters

<table>  <tr>    <td><a name="SkPoint_Distance_a"> <code><strong>a </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_Distance_b"> <code><strong>b </strong></code> </a></td> <td>
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

<a name="SkPoint_DotProduct"></a>
## DotProduct

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static SkScalar DotProduct(const SkPoint& a, const SkPoint& b)
</pre>

Returns the <a href="#SkPoint_dot">dot</a> product of <a href="#SkPoint_DotProduct_a">a</a> and <a href="#SkPoint_DotProduct_b">b</a>, treating them as two dimensional vectors

### Parameters

<table>  <tr>    <td><a name="SkPoint_DotProduct_a"> <code><strong>a </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_DotProduct_b"> <code><strong>b </strong></code> </a></td> <td>
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

<a name="SkPoint_CrossProduct"></a>
## CrossProduct

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static SkScalar CrossProduct(const SkPoint& a, const SkPoint& b)
</pre>

Returns the <a href="#SkPoint_cross">cross</a> product of <a href="#SkPoint_CrossProduct_a">a</a> and <a href="#SkPoint_CrossProduct_b">b</a>, treating them as two dimensional vectors

### Parameters

<table>  <tr>    <td><a name="SkPoint_CrossProduct_a"> <code><strong>a </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_CrossProduct_b"> <code><strong>b </strong></code> </a></td> <td>
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

<a name="SkPoint_cross"></a>
## cross

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar cross(const SkPoint& vec) const
</pre>

### Parameters

<table>  <tr>    <td><a name="SkPoint_cross_vec"> <code><strong>vec </strong></code> </a></td> <td>
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

<a name="SkPoint_dot"></a>
## dot

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar dot(const SkPoint& vec) const
</pre>

### Parameters

<table>  <tr>    <td><a name="SkPoint_dot_vec"> <code><strong>vec </strong></code> </a></td> <td>
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

<a name="SkPoint_lengthSqd"></a>
## lengthSqd

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar lengthSqd() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_distanceToSqd"></a>
## distanceToSqd

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar distanceToSqd(const SkPoint& pt) const
</pre>

### Parameters

<table>  <tr>    <td><a name="SkPoint_distanceToSqd_pt"> <code><strong>pt </strong></code> </a></td> <td>
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

## <a name="SkPoint_Side"></a> Enum SkPoint::Side

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
enum <a href="#SkPoint_Side">Side</a> {
<a href="#SkPoint_kLeft_Side">kLeft Side</a> = -1,
<a href="#SkPoint_kOn_Side">kOn Side</a> =  0,
<a href="#SkPoint_kRight_Side">kRight Side</a> =  1,
};</pre>

The side of a point relative to a line. If the line is from a to b then
the values are consistent with the sign of (b-a) <a href="#SkPoint_cross">cross</a> (pt-a)

### Constants

<table>
  <tr>
    <td><a name="SkPoint_kLeft_Side"> <code><strong>SkPoint::kLeft_Side </strong></code> </a></td><td>= -1</td><td></td>
  </tr>
  <tr>
    <td><a name="SkPoint_kOn_Side"> <code><strong>SkPoint::kOn_Side </strong></code> </a></td><td>=  0</td><td></td>
  </tr>
  <tr>
    <td><a name="SkPoint_kRight_Side"> <code><strong>SkPoint::kRight_Side </strong></code> </a></td><td>=  1</td><td></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete



<a name="SkPoint_distanceToLineBetweenSqd"></a>
## distanceToLineBetweenSqd

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar distanceToLineBetweenSqd(const SkPoint& a, const SkPoint& b,
                                  Side* side = nullptr) const
</pre>

Returns the squared distance to the infinite line between two pts. Also
optionally returns the <a href="#SkPoint_distanceToLineBetweenSqd_side">side</a> of the line that the pt falls on (looking
along line from <a href="#SkPoint_distanceToLineBetweenSqd_a">a</a> to <a href="#SkPoint_distanceToLineBetweenSqd_b">b</a>)

### Parameters

<table>  <tr>    <td><a name="SkPoint_distanceToLineBetweenSqd_a"> <code><strong>a </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_distanceToLineBetweenSqd_b"> <code><strong>b </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_distanceToLineBetweenSqd_side"> <code><strong>side </strong></code> </a></td> <td>
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

<a name="SkPoint_distanceToLineBetween"></a>
## distanceToLineBetween

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar distanceToLineBetween(const SkPoint& a, const SkPoint& b,
                               Side* side = nullptr) const
</pre>

Returns the distance to the infinite line between two pts. Also
optionally returns the <a href="#SkPoint_distanceToLineBetween_side">side</a> of the line that the pt falls on (looking
along the line from <a href="#SkPoint_distanceToLineBetween_a">a</a> to <a href="#SkPoint_distanceToLineBetween_b">b</a>)

### Parameters

<table>  <tr>    <td><a name="SkPoint_distanceToLineBetween_a"> <code><strong>a </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_distanceToLineBetween_b"> <code><strong>b </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_distanceToLineBetween_side"> <code><strong>side </strong></code> </a></td> <td>
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

<a name="SkPoint_distanceToLineSegmentBetweenSqd"></a>
## distanceToLineSegmentBetweenSqd

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar distanceToLineSegmentBetweenSqd(const SkPoint& a, const SkPoint& b) const
</pre>

Returns the squared distance to the line segment between pts <a href="#SkPoint_distanceToLineSegmentBetweenSqd_a">a</a> and <a href="#SkPoint_distanceToLineSegmentBetweenSqd_b">b</a>

### Parameters

<table>  <tr>    <td><a name="SkPoint_distanceToLineSegmentBetweenSqd_a"> <code><strong>a </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_distanceToLineSegmentBetweenSqd_b"> <code><strong>b </strong></code> </a></td> <td>
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

<a name="SkPoint_distanceToLineSegmentBetween"></a>
## distanceToLineSegmentBetween

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar distanceToLineSegmentBetween(const SkPoint& a, const SkPoint& b) const
</pre>

Returns the distance to the line segment between pts <a href="#SkPoint_distanceToLineSegmentBetween_a">a</a> and <a href="#SkPoint_distanceToLineSegmentBetween_b">b</a>.

### Parameters

<table>  <tr>    <td><a name="SkPoint_distanceToLineSegmentBetween_a"> <code><strong>a </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_distanceToLineSegmentBetween_b"> <code><strong>b </strong></code> </a></td> <td>
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

<a name="SkPoint_setOrthog"></a>
## setOrthog

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setOrthog(const SkPoint& vec, Side side = kLeft_Side)
</pre>

<a href="#SkPoint_Make">Make</a> this vector be orthogonal to <a href="#SkPoint_setOrthog_vec">vec</a>. Looking down <a href="#SkPoint_setOrthog_vec">vec</a> the
new vector will point in direction indicated by <a href="#SkPoint_setOrthog_side">side</a> (which
must be <a href="#SkPoint_kLeft_Side">kLeft Side</a> or <a href="#SkPoint_kRight_Side">kRight Side</a>).

### Parameters

<table>  <tr>    <td><a name="SkPoint_setOrthog_vec"> <code><strong>vec </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPoint_setOrthog_side"> <code><strong>side </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPoint_asScalars"></a>
## asScalars

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
const SkScalar* asScalars() const
</pre>

Returns <a href="#Point">Point</a> as an <a href="undocumented#SkScalar">SkScalar</a> array with two entries.

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

### See Also

incomplete

---

<a name="SkPointsAreFinite"></a>
## SkPointsAreFinite

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static inline bool SkPointsAreFinite(const SkPoint array[], int count)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkPointsAreFinite_array"> <code><strong>array </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkPointsAreFinite_count"> <code><strong>count </strong></code> </a></td> <td>
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

