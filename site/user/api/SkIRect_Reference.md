SkIRect Reference
===

# <a name="IRect"></a> IRect

# <a name="SkIRect"></a> Struct SkIRect
<a href="#SkIRect">SkIRect</a> holds four 32 bit integer coordinates describing the upper and
lower bounds of a rectangle. <a href="#SkIRect">SkIRect</a> may be created from outer bounds or
from position, <a href="#SkIRect_width">width</a>, and <a href="#SkIRect_height">height</a>. <a href="#SkIRect">SkIRect</a> describes an area; if its <a href="#SkIRect_right">right</a>
is less than or equal to its <a href="#SkIRect_left">left</a>, or if its <a href="#SkIRect_bottom">bottom</a> is less than or equal to
its <a href="#SkIRect_top">top</a>, it is considered empty.

# <a name="Overview"></a> Overview

## <a name="Subtopics"></a> Subtopics

| topics | description |
| --- | ---  |

## <a name="Operators"></a> Operators

| description | function |
| --- | ---  |
| bool <a href="#SkIRect_notequal_operator">operator!=(const SkIRect& a, const SkIRect& b)</a> | Returns true if members are unequal. |
| bool <a href="#SkIRect_equal_operator">operator==(const SkIRect& a, const SkIRect& b)</a> | Returns true if members are equal. |

## <a name="Member_Functions"></a> Member Functions

| description | function |
| --- | ---  |
| <a href="#SkIRect_EmptyIRect">EmptyIRect</a> | Returns immutable bounds of (0, 0, 0, 0). |
| <a href="#SkIRect_Intersects">Intersects</a> | Returns true if areas overlap. |
| <a href="#SkIRect_IntersectsNoEmptyCheck">IntersectsNoEmptyCheck</a> | Returns true if areas overlap. Skips empty check. |
| <a href="#SkIRect_MakeEmpty">MakeEmpty</a> | Returns bounds of (0, 0, 0, 0). |
| <a href="#SkIRect_MakeLTRB">MakeLTRB</a> | Constructs from int <a href="#SkIRect_left">left</a>, <a href="#SkIRect_top">top</a>, <a href="#SkIRect_right">right</a>, <a href="#SkIRect_bottom">bottom</a>. |
| <a href="#SkIRect_MakeSize">MakeSize</a> | Constructs from <a href="undocumented#ISize">ISize</a> returning (0, 0, <a href="#SkIRect_width">width</a>, <a href="#SkIRect_height">height</a>). |
| <a href="#SkIRect_MakeWH">MakeWH</a> | Constructs from int input returning (0, 0, <a href="#SkIRect_width">width</a>, <a href="#SkIRect_height">height</a>). |
| <a href="#SkIRect_MakeXYWH">MakeXYWH</a> | Constructs from int input returning (<a href="#SkIRect_x">x</a>, <a href="#SkIRect_y">y</a>, <a href="#SkIRect_width">width</a>, <a href="#SkIRect_height">height</a>). |
| <a href="#SkIRect_bottom">bottom</a> | Returns larger bounds in <a href="#SkIRect_y">y</a>, if sorted. |
| <a href="#SkIRect_centerX">centerX</a> | Returns midpoint in <a href="#SkIRect_x">x</a>. |
| <a href="#SkIRect_centerY">centerY</a> | Returns midpoint in <a href="#SkIRect_y">y</a>. |
| <a href="#SkIRect_contains">contains</a> | Returns true if points are equal or inside. |
| <a href="#SkIRect_containsNoEmptyCheck">containsNoEmptyCheck</a> | Returns true if points are equal or inside. Skips empty check. |
| <a href="#SkIRect_height">height</a> | Returns span in <a href="#SkIRect_y">y</a>. |
| <a href="#SkIRect_height64">height64</a> | Returns span in <a href="#SkIRect_y">y</a> as int64_t. |
| <a href="#SkIRect_inset">inset</a> | Moves the sides symmetrically about the center. |
| <a href="#SkIRect_intersect">intersect</a> | Sets to shared area; returns true if not empty. |
| <a href="#SkIRect_intersectNoEmptyCheck">intersectNoEmptyCheck</a> | Sets to shared area; returns true if not empty. Skips empty check. |
| <a href="#SkIRect_is16Bit">is16Bit</a> | Returns true if members fit in 16-bit word. |
| <a href="#SkIRect_isEmpty">isEmpty</a> | Returns true if <a href="#SkIRect_width">width</a> or <a href="#SkIRect_height">height</a> are zero or negative or they exceed int32_t. |
| <a href="#SkIRect_isEmpty64">isEmpty64</a> | Returns true if <a href="#SkIRect_width">width</a> or <a href="#SkIRect_height">height</a> are zero or negative. |
| <a href="#SkIRect_join">join</a> | Sets to union of bounds. |
| <a href="#SkIRect_left">left</a> | Returns smaller bounds in <a href="#SkIRect_x">x</a>, if sorted. |
| <a href="#SkIRect_makeInset">makeInset</a> | Constructs from sides moved symmetrically about the center. |
| <a href="#SkIRect_makeOffset">makeOffset</a> | Constructs from translated sides. |
| <a href="#SkIRect_makeOutset">makeOutset</a> | Constructs from sides moved symmetrically about the center. |
| <a href="#SkIRect_makeSorted">makeSorted</a> | Constructs, ordering sides from smaller to larger. |
| <a href="#SkIRect_offset">offset</a> | Translates sides without changing <a href="#SkIRect_width">width</a> and <a href="#SkIRect_height">height</a>. |
| <a href="#SkIRect_offsetTo">offsetTo</a> | Translates to (<a href="#SkIRect_x">x</a>, <a href="#SkIRect_y">y</a>) without changing <a href="#SkIRect_width">width</a> and <a href="#SkIRect_height">height</a>. |
| <a href="#SkIRect_outset">outset</a> | Moves the sides symmetrically about the center. |
| <a href="#SkIRect_quickReject">quickReject</a> | Returns true if rectangles do not <a href="#SkIRect_intersect">intersect</a>. |
| <a href="#SkIRect_right">right</a> | Returns larger bounds in <a href="#SkIRect_x">x</a>, if sorted. |
| <a href="#SkIRect_set">set</a> | Sets to (<a href="#SkIRect_left">left</a>, <a href="#SkIRect_top">top</a>, <a href="#SkIRect_right">right</a>, <a href="#SkIRect_bottom">bottom</a>). |
| <a href="#SkIRect_setEmpty">setEmpty</a> | Sets to (0, 0, 0, 0). |
| <a href="#SkIRect_setLTRB">setLTRB</a> | Sets to <a href="undocumented#SkScalar">SkScalar</a> input (<a href="#SkIRect_left">left</a>, <a href="#SkIRect_top">top</a>, <a href="#SkIRect_right">right</a>, <a href="#SkIRect_bottom">bottom</a>). |
| <a href="#SkIRect_setXYWH">setXYWH</a> | Sets to (<a href="#SkIRect_x">x</a>, <a href="#SkIRect_y">y</a>, <a href="#SkIRect_width">width</a>, <a href="#SkIRect_height">height</a>). |
| <a href="#SkIRect_size">size</a> | Returns <a href="undocumented#ISize">ISize</a> (<a href="#SkIRect_width">width</a>, <a href="#SkIRect_height">height</a>). |
| <a href="#SkIRect_sort">sort</a> | Orders sides from smaller to larger. |
| <a href="#SkIRect_top">top</a> | Returns smaller bounds in <a href="#SkIRect_y">y</a>, if sorted. |
| <a href="#SkIRect_width">width</a> | Returns span in <a href="#SkIRect_x">x</a>. |
| <a href="#SkIRect_width64">width64</a> | Returns span in <a href="#SkIRect_y">y</a> as int64_t. |
| <a href="#SkIRect_x">x</a> | Returns bounds <a href="#SkIRect_left">left</a>. |
| <a href="#SkIRect_y">y</a> | Returns bounds <a href="#SkIRect_top">top</a>. |

<a name="SkIRect_fLeft"> <code><strong>int32_t  fLeft</strong></code> </a>

May contain any value. The smaller of the horizontal values when sorted.
When equal to or greater than <a href="#SkIRect_fRight">fRight</a>, <a href="#IRect">IRect</a> is empty.

<a name="SkIRect_fTop"> <code><strong>int32_t  fTop</strong></code> </a>

May contain any value. The smaller of the horizontal values when sorted.
When equal to or greater than <a href="#SkIRect_fBottom">fBottom</a>, <a href="#IRect">IRect</a> is empty.

<a name="SkIRect_fRight"> <code><strong>int32_t  fRight</strong></code> </a>

May contain any value. The larger of the vertical values when sorted.
When equal to or less than <a href="#SkIRect_fLeft">fLeft</a>, <a href="#IRect">IRect</a> is empty.

<a name="SkIRect_fBottom"> <code><strong>int32_t  fBottom</strong></code> </a>

May contain any value. The larger of the vertical values when sorted.
When equal to or less than <a href="#SkIRect_fTop">fTop</a>, <a href="#IRect">IRect</a> is empty.

<a name="SkIRect_MakeEmpty"></a>
## MakeEmpty

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static constexpr SkIRect SK_WARN_UNUSED_RESULT MakeEmpty()
</pre>

Returns constructed <a href="#IRect">IRect</a> <a href="#SkIRect_set">set</a> to (0, 0, 0, 0).
Many other rectangles are empty; if <a href="#SkIRect_left">left</a> is equal to or greater than <a href="#SkIRect_right">right</a>,
or if <a href="#SkIRect_top">top</a> is equal to or greater than <a href="#SkIRect_bottom">bottom</a>. Setting all members to zero
is a convenience, but does not designate a special empty rectangle.

### Return Value

bounds (0, 0, 0, 0)

### Example

<div><fiddle-embed name="0ade3971c1d2616564992e286966ec8a">

#### Example Output

~~~~
MakeEmpty isEmpty: true
offset rect isEmpty: true
inset rect isEmpty: true
outset rect isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_EmptyIRect">EmptyIRect</a> <a href="#SkIRect_isEmpty">isEmpty</a> <a href="#SkIRect_setEmpty">setEmpty</a> <a href="#SkRect_MakeEmpty">SkRect::MakeEmpty</a>

---

<a name="SkIRect_MakeWH"></a>
## MakeWH

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static constexpr SkIRect SK_WARN_UNUSED_RESULT MakeWH(int32_t w, int32_t h)
</pre>

Returns constructed <a href="#IRect">IRect</a> <a href="#SkIRect_set">set</a> to (0, 0, <a href="#SkIRect_MakeWH_w">w</a>, <a href="#SkIRect_MakeWH_h">h</a>). Does not validate input; <a href="#SkIRect_MakeWH_w">w</a> or <a href="#SkIRect_MakeWH_h">h</a>
may be negative.

### Parameters

<table>  <tr>    <td><a name="SkIRect_MakeWH_w"> <code><strong>w </strong></code> </a></td> <td>
<a href="#SkIRect_width">width</a> of constructed <a href="SkRect_Reference#Rect">Rect</a></td>
  </tr>  <tr>    <td><a name="SkIRect_MakeWH_h"> <code><strong>h </strong></code> </a></td> <td>
<a href="#SkIRect_height">height</a> of constructed <a href="SkRect_Reference#Rect">Rect</a></td>
  </tr>
</table>

### Return Value

bounds (0, 0, <a href="#SkIRect_MakeWH_w">w</a>, <a href="#SkIRect_MakeWH_h">h</a>)

### Example

<div><fiddle-embed name="e36827a1a6ae2b1c26e7a8a08f325a07">

#### Example Output

~~~~
all equal
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_MakeSize">MakeSize</a> <a href="#SkIRect_MakeXYWH">MakeXYWH</a> <a href="#SkRect_MakeWH">SkRect::MakeWH</a> <a href="#SkRect_MakeIWH">SkRect::MakeIWH</a>

---

<a name="SkIRect_MakeSize"></a>
## MakeSize

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static constexpr SkIRect SK_WARN_UNUSED_RESULT MakeSize(const SkISize& size)
</pre>

Returns constructed <a href="#IRect">IRect</a> <a href="#SkIRect_set">set</a> to (0, 0, <a href="#SkIRect_size">size</a>.<a href="#SkIRect_width">width</a>, <a href="#SkIRect_size">size</a>.<a href="#SkIRect_height">height</a>).
Does not validate input; <a href="#SkIRect_size">size</a>.<a href="#SkIRect_width">width</a> or <a href="#SkIRect_size">size</a>.<a href="#SkIRect_height">height</a> may be negative.

### Parameters

<table>  <tr>    <td><a name="SkIRect_MakeSize_size"> <code><strong>size </strong></code> </a></td> <td>
values for <a href="SkRect_Reference#Rect">Rect</a> <a href="#SkIRect_width">width</a> and <a href="#SkIRect_height">height</a></td>
  </tr>
</table>

### Return Value

bounds (0, 0, <a href="#SkIRect_size">size</a>.<a href="#SkIRect_width">width</a>, <a href="#SkIRect_size">size</a>.<a href="#SkIRect_height">height</a>)

### Example

<div><fiddle-embed name="c6586ff8d24869c780169b0d19c75df6">

#### Example Output

~~~~
round width: 26  height: 36
floor width: 25  height: 35
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_MakeWH">MakeWH</a> <a href="#SkIRect_MakeXYWH">MakeXYWH</a> <a href="#SkRect_Make">SkRect::Make</a> <a href="#SkRect_MakeIWH">SkRect::MakeIWH</a>

---

<a name="SkIRect_MakeLTRB"></a>
## MakeLTRB

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static constexpr SkIRect SK_WARN_UNUSED_RESULT MakeLTRB(int32_t l, int32_t t, int32_t r, int32_t b)
</pre>

Returns constructed <a href="#IRect">IRect</a> <a href="#SkIRect_set">set</a> to (<a href="#SkIRect_MakeLTRB_l">l</a>, <a href="#SkIRect_MakeLTRB_t">t</a>, <a href="#SkIRect_MakeLTRB_r">r</a>, <a href="#SkIRect_MakeLTRB_b">b</a>). Does not <a href="#SkIRect_sort">sort</a> input; <a href="SkRect_Reference#Rect">Rect</a> may
result in <a href="#SkIRect_fLeft">fLeft</a> greater than <a href="#SkIRect_fRight">fRight</a>, or <a href="#SkIRect_fTop">fTop</a> greater than <a href="#SkIRect_fBottom">fBottom</a>.

### Parameters

<table>  <tr>    <td><a name="SkIRect_MakeLTRB_l"> <code><strong>l </strong></code> </a></td> <td>
integer stored in <a href="#SkIRect_fLeft">fLeft</a></td>
  </tr>  <tr>    <td><a name="SkIRect_MakeLTRB_t"> <code><strong>t </strong></code> </a></td> <td>
integer stored in <a href="#SkIRect_fTop">fTop</a></td>
  </tr>  <tr>    <td><a name="SkIRect_MakeLTRB_r"> <code><strong>r </strong></code> </a></td> <td>
integer stored in <a href="#SkIRect_fRight">fRight</a></td>
  </tr>  <tr>    <td><a name="SkIRect_MakeLTRB_b"> <code><strong>b </strong></code> </a></td> <td>
integer stored in <a href="#SkIRect_fBottom">fBottom</a></td>
  </tr>
</table>

### Return Value

bounds (<a href="#SkIRect_MakeLTRB_l">l</a>, <a href="#SkIRect_MakeLTRB_t">t</a>, <a href="#SkIRect_MakeLTRB_r">r</a>, <a href="#SkIRect_MakeLTRB_b">b</a>)

### Example

<div><fiddle-embed name="ec1473b700c594f2df9749a12a06b89b">

#### Example Output

~~~~
rect: 5, 35, 15, 25  isEmpty: true
rect: 5, 25, 15, 35  isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_MakeXYWH">MakeXYWH</a> <a href="#SkRect_MakeLTRB">SkRect::MakeLTRB</a>

---

<a name="SkIRect_MakeXYWH"></a>
## MakeXYWH

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static constexpr SkIRect SK_WARN_UNUSED_RESULT MakeXYWH(int32_t x, int32_t y, int32_t w, int32_t h)
</pre>

Returns constructed <a href="#IRect">IRect</a> <a href="#SkIRect_set">set</a> to:
(<a href="#SkIRect_x">x</a>, <a href="#SkIRect_y">y</a>, <a href="#SkIRect_x">x</a> + <a href="#SkIRect_MakeXYWH_w">w</a>, <a href="#SkIRect_y">y</a> + <a href="#SkIRect_MakeXYWH_h">h</a>).
Does not validate input;
<a href="#SkIRect_MakeXYWH_w">w</a> or <a href="#SkIRect_MakeXYWH_h">h</a> may be negative.

### Parameters

<table>  <tr>    <td><a name="SkIRect_MakeXYWH_x"> <code><strong>x </strong></code> </a></td> <td>
stored in <a href="#SkIRect_fLeft">fLeft</a></td>
  </tr>  <tr>    <td><a name="SkIRect_MakeXYWH_y"> <code><strong>y </strong></code> </a></td> <td>
stored in <a href="#SkIRect_fTop">fTop</a></td>
  </tr>  <tr>    <td><a name="SkIRect_MakeXYWH_w"> <code><strong>w </strong></code> </a></td> <td>
added to <a href="#SkIRect_x">x</a> and stored in <a href="#SkIRect_fRight">fRight</a></td>
  </tr>  <tr>    <td><a name="SkIRect_MakeXYWH_h"> <code><strong>h </strong></code> </a></td> <td>
added to <a href="#SkIRect_y">y</a> and stored in <a href="#SkIRect_fBottom">fBottom</a></td>
  </tr>
</table>

### Return Value

bounds at (<a href="#SkIRect_x">x</a>, <a href="#SkIRect_y">y</a>) with <a href="#SkIRect_width">width</a> <a href="#SkIRect_MakeXYWH_w">w</a> and <a href="#SkIRect_height">height</a> <a href="#SkIRect_MakeXYWH_h">h</a>

### Example

<div><fiddle-embed name="598ee14350bd1d961cae6b36fa3df17e">

#### Example Output

~~~~
rect: 5, 35, -10, 60  isEmpty: true
rect: -10, 35, 5, 60  isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_MakeLTRB">MakeLTRB</a> <a href="#SkRect_MakeXYWH">SkRect::MakeXYWH</a>

---

<a name="SkIRect_left"></a>
## left

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int32_t left() const
</pre>

Returns <a href="#SkIRect_left">left</a> edge of <a href="#IRect">IRect</a>, if sorted.
Call <a href="#SkIRect_sort">sort</a> to reverse <a href="#SkIRect_fLeft">fLeft</a> and <a href="#SkIRect_fRight">fRight</a> if needed.

### Return Value

<a href="#SkIRect_fLeft">fLeft</a>

### Example

<div><fiddle-embed name="caf38ea4431bc246ba198f6a8c2b0f01">

#### Example Output

~~~~
unsorted.fLeft: 15 unsorted.left(): 15
sorted.fLeft: 10 sorted.left(): 10
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_fLeft">fLeft</a> <a href="#SkIRect_x">x</a> <a href="#SkRect_left">SkRect::left()</a>

---

<a name="SkIRect_top"></a>
## top

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int32_t top() const
</pre>

Returns <a href="#SkIRect_top">top</a> edge of <a href="#IRect">IRect</a>, if sorted. Call <a href="#SkIRect_isEmpty">isEmpty</a> to see if <a href="#IRect">IRect</a> may be invalid,
and <a href="#SkIRect_sort">sort</a> to reverse <a href="#SkIRect_fTop">fTop</a> and <a href="#SkIRect_fBottom">fBottom</a> if needed.

### Return Value

<a href="#SkIRect_fTop">fTop</a>

### Example

<div><fiddle-embed name="cbec1ae6530e95943775450b1d11f19e">

#### Example Output

~~~~
unsorted.fTop: 25 unsorted.top(): 25
sorted.fTop: 5 sorted.top(): 5
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_fTop">fTop</a> <a href="#SkIRect_y">y</a> <a href="#SkRect_top">SkRect::top()</a>

---

<a name="SkIRect_right"></a>
## right

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int32_t right() const
</pre>

Returns <a href="#SkIRect_right">right</a> edge of <a href="#IRect">IRect</a>, if sorted.
Call <a href="#SkIRect_sort">sort</a> to reverse <a href="#SkIRect_fLeft">fLeft</a> and <a href="#SkIRect_fRight">fRight</a> if needed.

### Return Value

<a href="#SkIRect_fRight">fRight</a>

### Example

<div><fiddle-embed name="97e210976f1ee0387b30c70635cf114f">

#### Example Output

~~~~
unsorted.fRight: 10 unsorted.right(): 10
sorted.fRight: 15 sorted.right(): 15
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_fRight">fRight</a> <a href="#SkRect_right">SkRect::right()</a>

---

<a name="SkIRect_bottom"></a>
## bottom

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int32_t bottom() const
</pre>

Returns <a href="#SkIRect_bottom">bottom</a> edge of <a href="#IRect">IRect</a>, if sorted. Call <a href="#SkIRect_isEmpty">isEmpty</a> to see if <a href="#IRect">IRect</a> may be invalid,
and <a href="#SkIRect_sort">sort</a> to reverse <a href="#SkIRect_fTop">fTop</a> and <a href="#SkIRect_fBottom">fBottom</a> if needed.

### Return Value

<a href="#SkIRect_fBottom">fBottom</a>

### Example

<div><fiddle-embed name="c32afebc296054a181621648a184b8e3">

#### Example Output

~~~~
unsorted.fBottom: 5 unsorted.bottom(): 5
sorted.fBottom: 25 sorted.bottom(): 25
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_fBottom">fBottom</a> <a href="#SkRect_bottom">SkRect::bottom()</a>

---

<a name="SkIRect_x"></a>
## x

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int32_t x() const
</pre>

Returns <a href="#SkIRect_left">left</a> edge of <a href="#IRect">IRect</a>, if sorted. Call <a href="#SkIRect_isEmpty">isEmpty</a> to see if <a href="#IRect">IRect</a> may be invalid,
and <a href="#SkIRect_sort">sort</a> to reverse <a href="#SkIRect_fLeft">fLeft</a> and <a href="#SkIRect_fRight">fRight</a> if needed.

### Return Value

<a href="#SkIRect_fLeft">fLeft</a>

### Example

<div><fiddle-embed name="2a59cbfd1330a0db520d6ebb2b7c68c7">

#### Example Output

~~~~
unsorted.fLeft: 15 unsorted.x(): 15
sorted.fLeft: 10 sorted.x(): 10
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_fLeft">fLeft</a> <a href="#SkIRect_left">left</a> <a href="#SkIRect_y">y</a> <a href="#SkRect_x">SkRect::x()</a>

---

<a name="SkIRect_y"></a>
## y

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int32_t y() const
</pre>

Returns <a href="#SkIRect_top">top</a> edge of <a href="#IRect">IRect</a>, if sorted. Call <a href="#SkIRect_isEmpty">isEmpty</a> to see if <a href="#IRect">IRect</a> may be invalid,
and <a href="#SkIRect_sort">sort</a> to reverse <a href="#SkIRect_fTop">fTop</a> and <a href="#SkIRect_fBottom">fBottom</a> if needed.

### Return Value

<a href="#SkIRect_fTop">fTop</a>

### Example

<div><fiddle-embed name="6ea461e71f7fc80605818fbf493caa63">

#### Example Output

~~~~
unsorted.fTop: 25 unsorted.y(): 25
sorted.fTop: 5 sorted.y(): 5
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_fTop">fTop</a> <a href="#SkIRect_top">top</a> <a href="#SkIRect_x">x</a> <a href="#SkRect_y">SkRect::y()</a>

---

<a name="SkIRect_width"></a>
## width

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int32_t width() const
</pre>

Returns span on the <a href="#SkIRect_x">x</a>-axis. This does not check if <a href="#IRect">IRect</a> is sorted, or if
result fits in 32-bit signed integer; result may be negative.

### Return Value

<a href="#SkIRect_fRight">fRight</a> minus <a href="#SkIRect_fLeft">fLeft</a>

### Example

<div><fiddle-embed name="4acfbe051805940210c8916a94794142">

#### Example Output

~~~~
unsorted width: -5
large width: -5
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_height">height</a> <a href="#SkIRect_width64">width64</a> <a href="#SkIRect_height64">height64</a> <a href="#SkRect_width">SkRect::width()</a>

---

<a name="SkIRect_width64"></a>
## width64

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int64_t width64() const
</pre>

Returns span on the <a href="#SkIRect_x">x</a>-axis. This does not check if <a href="#IRect">IRect</a> is sorted, so the
result may be negative. This is safer than calling <a href="#SkIRect_width">width</a> since <a href="#SkIRect_width">width</a> might
overflow in its calculation.

### Return Value

<a href="#SkIRect_fRight">fRight</a> minus <a href="#SkIRect_fLeft">fLeft</a> cast to int64_t

<a href="#SkIRect">SkIRect</a> large = { -2147483647, 1, 2147483644, 2 };

#### Example Output

~~~~
width: -5 width64: 4294967291
~~~~

### See Also

<a href="#SkIRect_width">width</a> <a href="#SkIRect_height">height</a> <a href="#SkIRect_height64">height64</a> <a href="#SkRect_width">SkRect::width()</a>

---

<a name="SkIRect_height"></a>
## height

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int32_t height() const
</pre>

Returns span on the <a href="#SkIRect_y">y</a>-axis. This does not check if <a href="#IRect">IRect</a> is sorted, or if
result fits in 32-bit signed integer; result may be negative.

### Return Value

<a href="#SkIRect_fBottom">fBottom</a> minus <a href="#SkIRect_fTop">fTop</a>

### Example

<div><fiddle-embed name="0175bae87fafcd9433ae661574695586">

#### Example Output

~~~~
unsorted height: -5
large height: -5
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_width">width</a> <a href="#SkRect_height">SkRect::height()</a>

---

<a name="SkIRect_height64"></a>
## height64

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int64_t height64() const
</pre>

Returns span on the <a href="#SkIRect_y">y</a>-axis. This does not check if <a href="#IRect">IRect</a> is sorted, so the
result may be negative. This is safer than calling <a href="#SkIRect_height">height</a> since <a href="#SkIRect_height">height</a> might
overflow in its calculation.

### Return Value

<a href="#SkIRect_fBottom">fBottom</a> minus <a href="#SkIRect_fTop">fTop</a> cast to int64_t

<a href="#SkIRect">SkIRect</a> large = { 1, -2147483647, 2, 2147483644 };

#### Example Output

~~~~
height: -5 height64: 4294967291
~~~~

### See Also

<a href="#SkIRect_width">width</a> <a href="#SkIRect_height">height</a> <a href="#SkIRect_width64">width64</a> <a href="#SkRect_height">SkRect::height()</a>

---

<a name="SkIRect_size"></a>
## size

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkISize size() const
</pre>

Returns spans on the <a href="#SkIRect_x">x</a>-axis and <a href="#SkIRect_y">y</a>-axis. This does not check if <a href="#IRect">IRect</a> is sorted,
or if result fits in 32-bit signed integer; result may be negative.

### Return Value

<a href="undocumented#ISize">ISize</a> (<a href="#SkIRect_width">width</a>, <a href="#SkIRect_height">height</a>)

### Example

<div><fiddle-embed name="8b3224641cb3053a7b8a5798b6cd1cf6">

#### Example Output

~~~~
original rect: 20, 30, 40, 50  size: 20, 20
offset rect: 40, 50, 60, 70  size: 20, 20
outset rect: 20, 30, 80, 90  size: 60, 60
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_height">height</a> <a href="#SkIRect_width">width</a> <a href="#SkIRect_MakeSize">MakeSize</a>

---

<a name="SkIRect_centerX"></a>
## centerX

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int32_t centerX() const
</pre>

Returns average of <a href="#SkIRect_left">left</a> edge and <a href="#SkIRect_right">right</a> edge. Result does not change if <a href="SkRect_Reference#Rect">Rect</a>
is sorted. Result may be incorrect if <a href="SkRect_Reference#Rect">Rect</a> is far from the origin.

Result is rounded down.

### Return Value

midpoint in <a href="#SkIRect_x">x</a>

### Example

<div><fiddle-embed name="549b840a9ceaaf7cb4e604f9f3d7108d"><div>Dividing by two rounds towards zero. <a href="#SkIRect_centerX">centerX</a> uses a bit shift and rounds down.</div>

#### Example Output

~~~~
left:  20 right:  41 centerX:  30 div2:  30
left: -20 right: -41 centerX: -31 div2: -30
left: -10 right:  11 centerX:   0 div2:   0
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_centerY">centerY</a> <a href="#SkRect_centerX">SkRect::centerX</a>

---

<a name="SkIRect_centerY"></a>
## centerY

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int32_t centerY() const
</pre>

Returns average of <a href="#SkIRect_top">top</a> edge and <a href="#SkIRect_bottom">bottom</a> edge. Result does not change if <a href="SkRect_Reference#Rect">Rect</a>
is sorted. Result may be incorrect if <a href="SkRect_Reference#Rect">Rect</a> is far from the origin.

Result is rounded down.

### Return Value

midpoint in <a href="#SkIRect_y">y</a>

### Example

<div><fiddle-embed name="687d833b042fb018f8948764e73a37b1">

#### Example Output

~~~~
left: 1073741824 right: 1073741826 centerX: -1073741823 safe mid x: 1073741825
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_centerX">centerX</a> <a href="#SkRect_centerY">SkRect::centerY</a>

---

<a name="SkIRect_isEmpty"></a>
## isEmpty

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isEmpty() const
</pre>

Returns true if <a href="#SkIRect_width">width</a> or <a href="#SkIRect_height">height</a> .

### Return Value

true if <a href="#SkIRect_width">width</a> or <a href="#SkIRect_height">height</a> are zero or negative

### Example

<div><fiddle-embed name="edaad064b6de249b7a7c768dfa000adc">

#### Example Output

~~~~
rect: {20, 40, 10, 50} is empty
sorted: {10, 40, 20, 50} is not empty
rect: {20, 40, 20, 50} is empty
sorted: {20, 40, 20, 50} is empty
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_EmptyIRect">EmptyIRect</a> <a href="#SkIRect_MakeEmpty">MakeEmpty</a> <a href="#SkIRect_sort">sort</a> <a href="#SkRect_isEmpty">SkRect::isEmpty</a>

---

<a name="SkIRect_isEmpty64"></a>
## isEmpty64

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isEmpty64() const
</pre>

Returns true if <a href="#SkIRect_fLeft">fLeft</a> is equal to or greater than <a href="#SkIRect_fRight">fRight</a>, or if <a href="#SkIRect_fTop">fTop</a> is equal
to or greater than <a href="#SkIRect_fBottom">fBottom</a>. Call <a href="#SkIRect_sort">sort</a> to reverse rectangles with negative
<a href="#SkIRect_width64">width64</a> or <a href="#SkIRect_height64">height64</a>.

### Return Value

true if <a href="#SkIRect_width64">width64</a> or <a href="#SkIRect_height64">height64</a> are zero or negative

<a href="#SkIRect">SkIRect</a> tests[] = {{20, 40, 10, 50}, {20, 40, 20, 50}};
for (auto rect : tests) {

#### Example Output

~~~~
rect: {20, 40, 10, 50} is empty
sorted: {10, 40, 20, 50} is not empty
rect: {20, 40, 20, 50} is empty
sorted: {20, 40, 20, 50} is empty
~~~~

### See Also

<a href="#SkIRect_EmptyIRect">EmptyIRect</a> <a href="#SkIRect_MakeEmpty">MakeEmpty</a> <a href="#SkIRect_sort">sort</a> <a href="#SkRect_isEmpty">SkRect::isEmpty</a>

---

<a name="SkIRect_equal_operator"></a>
## operator==

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool operator==(const SkIRect& a, const SkIRect& b)
</pre>

Returns true if all members in <a href="#SkIRect_equal_operator_a">a</a>: <a href="#SkIRect_fLeft">fLeft</a>, <a href="#SkIRect_fTop">fTop</a>, <a href="#SkIRect_fRight">fRight</a>, and <a href="#SkIRect_fBottom">fBottom</a>; are
identical to corresponding members in <a href="#SkIRect_equal_operator_b">b</a>.

### Parameters

<table>  <tr>    <td><a name="SkIRect_equal_operator_a"> <code><strong>a </strong></code> </a></td> <td>
<a href="#IRect">IRect</a> to compare</td>
  </tr>  <tr>    <td><a name="SkIRect_equal_operator_b"> <code><strong>b </strong></code> </a></td> <td>
<a href="#IRect">IRect</a> to compare</td>
  </tr>
</table>

### Return Value

true if members are equal

### Example

<div><fiddle-embed name="bd8f028d9051062816c9116fea4237b2">

#### Example Output

~~~~
test == sorted
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_notequal_operator">operator!=(const SkIRect& a, const SkIRect& b)</a>

---

<a name="SkIRect_notequal_operator"></a>
## operator!=

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool operator!=(const SkIRect& a, const SkIRect& b)
</pre>

Returns true if any member in <a href="#SkIRect_notequal_operator_a">a</a>: <a href="#SkIRect_fLeft">fLeft</a>, <a href="#SkIRect_fTop">fTop</a>, <a href="#SkIRect_fRight">fRight</a>, and <a href="#SkIRect_fBottom">fBottom</a>; is not
identical to the corresponding member in <a href="#SkIRect_notequal_operator_b">b</a>.

### Parameters

<table>  <tr>    <td><a name="SkIRect_notequal_operator_a"> <code><strong>a </strong></code> </a></td> <td>
<a href="#IRect">IRect</a> to compare</td>
  </tr>  <tr>    <td><a name="SkIRect_notequal_operator_b"> <code><strong>b </strong></code> </a></td> <td>
<a href="#IRect">IRect</a> to compare</td>
  </tr>
</table>

### Return Value

true if members are not equal

### Example

<div><fiddle-embed name="6c4acd8aa203f632b7d85cae672abf4d">

#### Example Output

~~~~
test != sorted
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_equal_operator">operator==(const SkIRect& a, const SkIRect& b)</a>

---

<a name="SkIRect_is16Bit"></a>
## is16Bit

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool is16Bit() const
</pre>

Returns true if all members: <a href="#SkIRect_fLeft">fLeft</a>, <a href="#SkIRect_fTop">fTop</a>, <a href="#SkIRect_fRight">fRight</a>, and <a href="#SkIRect_fBottom">fBottom</a>; values are
equal to or larger than -32768 and equal to or smaller than 32767.

### Return Value

true if members fit in 16-bit word

### Example

<div><fiddle-embed name="103e8d463e68e87e0f8f9454a7d3441c">

#### Example Output

~~~~
{-32768, -32768, 32767, 32767} fits in 16 bits
{-32768, -32768, 32768, 32768} does not fit in 16 bits
~~~~

</fiddle-embed></div>

### See Also

<a href="undocumented#SkTFitsIn">SkTFitsIn</a>

---

<a name="SkIRect_setEmpty"></a>
## setEmpty

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setEmpty()
</pre>

Sets <a href="#IRect">IRect</a> to (0, 0, 0, 0).

Many other rectangles are empty; if <a href="#SkIRect_left">left</a> is equal to or greater than <a href="#SkIRect_right">right</a>,
or if <a href="#SkIRect_top">top</a> is equal to or greater than <a href="#SkIRect_bottom">bottom</a>. Setting all members to zero
is a convenience, but does not designate a special empty rectangle.

### Example

<div><fiddle-embed name="94039c3cc9e911c8ab2993d56fd06210">

#### Example Output

~~~~
rect: {3, 4, 1, 2} is empty
rect: {0, 0, 0, 0} is empty
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_MakeEmpty">MakeEmpty</a> <a href="#SkRect_setEmpty">SkRect::setEmpty</a>

---

<a name="SkIRect_set"></a>
## set

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void set(int32_t left, int32_t top, int32_t right, int32_t bottom)
</pre>

Sets <a href="#IRect">IRect</a> to (<a href="#SkIRect_left">left</a>, <a href="#SkIRect_top">top</a>, <a href="#SkIRect_right">right</a>, <a href="#SkIRect_bottom">bottom</a>).
<a href="#SkIRect_left">left</a> and <a href="#SkIRect_right">right</a> are not sorted; <a href="#SkIRect_left">left</a> is not necessarily less than <a href="#SkIRect_right">right</a>.
<a href="#SkIRect_top">top</a> and <a href="#SkIRect_bottom">bottom</a> are not sorted; <a href="#SkIRect_top">top</a> is not necessarily less than <a href="#SkIRect_bottom">bottom</a>.

### Parameters

<table>  <tr>    <td><a name="SkIRect_set_left"> <code><strong>left </strong></code> </a></td> <td>
assigned to <a href="#SkIRect_fLeft">fLeft</a></td>
  </tr>  <tr>    <td><a name="SkIRect_set_top"> <code><strong>top </strong></code> </a></td> <td>
assigned to <a href="#SkIRect_fTop">fTop</a></td>
  </tr>  <tr>    <td><a name="SkIRect_set_right"> <code><strong>right </strong></code> </a></td> <td>
assigned to <a href="#SkIRect_fRight">fRight</a></td>
  </tr>  <tr>    <td><a name="SkIRect_set_bottom"> <code><strong>bottom </strong></code> </a></td> <td>
assigned to <a href="#SkIRect_fBottom">fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1912c37076b7f3bf6aebfa167e971bec">

#### Example Output

~~~~
rect1: {3, 4, 1, 2}
rect2: {3, 4, 1, 2}
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_setLTRB">setLTRB</a> <a href="#SkIRect_setXYWH">setXYWH</a> <a href="#SkRect_set">SkRect::set</a>

---

<a name="SkIRect_setLTRB"></a>
## setLTRB

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setLTRB(int32_t left, int32_t top, int32_t right, int32_t bottom)
</pre>

Sets <a href="#IRect">IRect</a> to (<a href="#SkIRect_left">left</a>, <a href="#SkIRect_top">top</a>, <a href="#SkIRect_right">right</a>, <a href="#SkIRect_bottom">bottom</a>).
<a href="#SkIRect_left">left</a> and <a href="#SkIRect_right">right</a> are not sorted; <a href="#SkIRect_left">left</a> is not necessarily less than <a href="#SkIRect_right">right</a>.
<a href="#SkIRect_top">top</a> and <a href="#SkIRect_bottom">bottom</a> are not sorted; <a href="#SkIRect_top">top</a> is not necessarily less than <a href="#SkIRect_bottom">bottom</a>.

### Parameters

<table>  <tr>    <td><a name="SkIRect_setLTRB_left"> <code><strong>left </strong></code> </a></td> <td>
stored in <a href="#SkIRect_fLeft">fLeft</a></td>
  </tr>  <tr>    <td><a name="SkIRect_setLTRB_top"> <code><strong>top </strong></code> </a></td> <td>
stored in <a href="#SkIRect_fTop">fTop</a></td>
  </tr>  <tr>    <td><a name="SkIRect_setLTRB_right"> <code><strong>right </strong></code> </a></td> <td>
stored in <a href="#SkIRect_fRight">fRight</a></td>
  </tr>  <tr>    <td><a name="SkIRect_setLTRB_bottom"> <code><strong>bottom </strong></code> </a></td> <td>
stored in <a href="#SkIRect_fBottom">fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ead6bdcf2ae77ec19a1c5a96f5b31af8">

#### Example Output

~~~~
rect1: {3, 4, 1, 2}
rect2: {3, 4, 1, 2}
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_set">set</a> <a href="#SkIRect_setXYWH">setXYWH</a> <a href="#SkRect_setLTRB">SkRect::setLTRB</a>

---

<a name="SkIRect_setXYWH"></a>
## setXYWH

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setXYWH(int32_t x, int32_t y, int32_t width, int32_t height)
</pre>

Sets <a href="#IRect">IRect</a> to:
(<a href="#SkIRect_x">x</a>, <a href="#SkIRect_y">y</a>, <a href="#SkIRect_x">x</a> + <a href="#SkIRect_width">width</a>, <a href="#SkIRect_y">y</a> + <a href="#SkIRect_height">height</a>).
Does not validate input;
<a href="#SkIRect_width">width</a> or <a href="#SkIRect_height">height</a> may be negative.

### Parameters

<table>  <tr>    <td><a name="SkIRect_setXYWH_x"> <code><strong>x </strong></code> </a></td> <td>
stored in <a href="#SkIRect_fLeft">fLeft</a></td>
  </tr>  <tr>    <td><a name="SkIRect_setXYWH_y"> <code><strong>y </strong></code> </a></td> <td>
stored in <a href="#SkIRect_fTop">fTop</a></td>
  </tr>  <tr>    <td><a name="SkIRect_setXYWH_width"> <code><strong>width </strong></code> </a></td> <td>
added to <a href="#SkIRect_x">x</a> and stored in <a href="#SkIRect_fRight">fRight</a></td>
  </tr>  <tr>    <td><a name="SkIRect_setXYWH_height"> <code><strong>height </strong></code> </a></td> <td>
added to <a href="#SkIRect_y">y</a> and stored in <a href="#SkIRect_fBottom">fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0e1db8c86678c004e504f47641b44b17">

#### Example Output

~~~~
rect: 5, 35, -10, 60  isEmpty: true
rect: -10, 35, 5, 60  isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_MakeXYWH">MakeXYWH</a> <a href="#SkIRect_setLTRB">setLTRB</a> <a href="#SkIRect_set">set</a> <a href="#SkRect_setXYWH">SkRect::setXYWH</a>

---

<a name="SkIRect_makeOffset"></a>
## makeOffset

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkIRect makeOffset(int32_t dx, int32_t dy) const
</pre>

Returns <a href="#IRect">IRect</a> <a href="#SkIRect_offset">offset</a> by (<a href="#SkIRect_makeOffset_dx">dx</a>, <a href="#SkIRect_makeOffset_dy">dy</a>).

If <a href="#SkIRect_makeOffset_dx">dx</a> is negative, <a href="#IRect">IRect</a> returned is moved to the <a href="#SkIRect_left">left</a>.
If <a href="#SkIRect_makeOffset_dx">dx</a> is positive, <a href="#IRect">IRect</a> returned is moved to the <a href="#SkIRect_right">right</a>.
If <a href="#SkIRect_makeOffset_dy">dy</a> is negative, <a href="#IRect">IRect</a> returned is moved upward.
If <a href="#SkIRect_makeOffset_dy">dy</a> is positive, <a href="#IRect">IRect</a> returned is moved downward.

### Parameters

<table>  <tr>    <td><a name="SkIRect_makeOffset_dx"> <code><strong>dx </strong></code> </a></td> <td>
<a href="#SkIRect_offset">offset</a> added to <a href="#SkIRect_fLeft">fLeft</a> and <a href="#SkIRect_fRight">fRight</a></td>
  </tr>  <tr>    <td><a name="SkIRect_makeOffset_dy"> <code><strong>dy </strong></code> </a></td> <td>
<a href="#SkIRect_offset">offset</a> added to <a href="#SkIRect_fTop">fTop</a> and <a href="#SkIRect_fBottom">fBottom</a></td>
  </tr>
</table>

### Return Value

<a href="SkRect_Reference#Rect">Rect</a> <a href="#SkIRect_offset">offset</a> in <a href="#SkIRect_x">x</a> or <a href="#SkIRect_y">y</a>, with original <a href="#SkIRect_width">width</a> and <a href="#SkIRect_height">height</a>

### Example

<div><fiddle-embed name="737c747df07ddf392c05970440de0927">

#### Example Output

~~~~
rect: 10, 50, 20, 60  isEmpty: false
rect: 25, 82, 35, 92  isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_offset">offset</a> <a href="#SkIRect_makeInset">makeInset</a> <a href="#SkIRect_makeOutset">makeOutset</a> <a href="#SkRect_makeOffset">SkRect::makeOffset</a>

---

<a name="SkIRect_makeInset"></a>
## makeInset

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkIRect makeInset(int32_t dx, int32_t dy) const
</pre>

Returns <a href="#IRect">IRect</a>, <a href="#SkIRect_inset">inset</a> by (<a href="#SkIRect_makeInset_dx">dx</a>, <a href="#SkIRect_makeInset_dy">dy</a>).

If <a href="#SkIRect_makeInset_dx">dx</a> is negative, <a href="#IRect">IRect</a> returned is wider.
If <a href="#SkIRect_makeInset_dx">dx</a> is positive, <a href="#IRect">IRect</a> returned is narrower.
If <a href="#SkIRect_makeInset_dy">dy</a> is negative, <a href="#IRect">IRect</a> returned is taller.
If <a href="#SkIRect_makeInset_dy">dy</a> is positive, <a href="#IRect">IRect</a> returned is shorter.

### Parameters

<table>  <tr>    <td><a name="SkIRect_makeInset_dx"> <code><strong>dx </strong></code> </a></td> <td>
<a href="#SkIRect_offset">offset</a> added to <a href="#SkIRect_fLeft">fLeft</a> and subtracted from <a href="#SkIRect_fRight">fRight</a></td>
  </tr>  <tr>    <td><a name="SkIRect_makeInset_dy"> <code><strong>dy </strong></code> </a></td> <td>
<a href="#SkIRect_offset">offset</a> added to <a href="#SkIRect_fTop">fTop</a> and subtracted from <a href="#SkIRect_fBottom">fBottom</a></td>
  </tr>
</table>

### Return Value

<a href="SkRect_Reference#Rect">Rect</a> <a href="#SkIRect_inset">inset</a> symmetrically <a href="#SkIRect_left">left</a> and <a href="#SkIRect_right">right</a>, <a href="#SkIRect_top">top</a> and <a href="#SkIRect_bottom">bottom</a>

### Example

<div><fiddle-embed name="1db94b2c76e0a7a71856532335fa56b6">

#### Example Output

~~~~
rect: 10, 50, 20, 60  isEmpty: false
rect: 25, 82, 5, 28  isEmpty: true
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_inset">inset</a> <a href="#SkIRect_makeOffset">makeOffset</a> <a href="#SkIRect_makeOutset">makeOutset</a> <a href="#SkRect_makeInset">SkRect::makeInset</a>

---

<a name="SkIRect_makeOutset"></a>
## makeOutset

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkIRect makeOutset(int32_t dx, int32_t dy) const
</pre>

Returns <a href="#IRect">IRect</a>, <a href="#SkIRect_outset">outset</a> by (<a href="#SkIRect_makeOutset_dx">dx</a>, <a href="#SkIRect_makeOutset_dy">dy</a>).

If <a href="#SkIRect_makeOutset_dx">dx</a> is negative, <a href="#IRect">IRect</a> returned is narrower.
If <a href="#SkIRect_makeOutset_dx">dx</a> is positive, <a href="#IRect">IRect</a> returned is wider.
If <a href="#SkIRect_makeOutset_dy">dy</a> is negative, <a href="#IRect">IRect</a> returned is shorter.
If <a href="#SkIRect_makeOutset_dy">dy</a> is positive, <a href="#IRect">IRect</a> returned is taller.

### Parameters

<table>  <tr>    <td><a name="SkIRect_makeOutset_dx"> <code><strong>dx </strong></code> </a></td> <td>
<a href="#SkIRect_offset">offset</a> subtracted to <a href="#SkIRect_fLeft">fLeft</a> and added from <a href="#SkIRect_fRight">fRight</a></td>
  </tr>  <tr>    <td><a name="SkIRect_makeOutset_dy"> <code><strong>dy </strong></code> </a></td> <td>
<a href="#SkIRect_offset">offset</a> subtracted to <a href="#SkIRect_fTop">fTop</a> and added from <a href="#SkIRect_fBottom">fBottom</a></td>
  </tr>
</table>

### Return Value

<a href="SkRect_Reference#Rect">Rect</a> <a href="#SkIRect_outset">outset</a> symmetrically <a href="#SkIRect_left">left</a> and <a href="#SkIRect_right">right</a>, <a href="#SkIRect_top">top</a> and <a href="#SkIRect_bottom">bottom</a>

### Example

<div><fiddle-embed name="240e2953e3455c08f6d89255feff8416">

#### Example Output

~~~~
rect: 10, 50, 20, 60  isEmpty: false
rect: -5, 18, 35, 92  isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_outset">outset</a> <a href="#SkIRect_makeOffset">makeOffset</a> <a href="#SkIRect_makeInset">makeInset</a> <a href="#SkRect_makeOutset">SkRect::makeOutset</a>

---

<a name="SkIRect_offset"></a>
## offset

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void offset(int32_t dx, int32_t dy)
</pre>

Offsets <a href="#IRect">IRect</a> by adding <a href="#SkIRect_offset_dx">dx</a> to <a href="#SkIRect_fLeft">fLeft</a>, <a href="#SkIRect_fRight">fRight</a>; and by adding <a href="#SkIRect_offset_dy">dy</a> to <a href="#SkIRect_fTop">fTop</a>, <a href="#SkIRect_fBottom">fBottom</a>.

If <a href="#SkIRect_offset_dx">dx</a> is negative, moves <a href="#IRect">IRect</a> returned to the <a href="#SkIRect_left">left</a>.
If <a href="#SkIRect_offset_dx">dx</a> is positive, moves <a href="#IRect">IRect</a> returned to the <a href="#SkIRect_right">right</a>.
If <a href="#SkIRect_offset_dy">dy</a> is negative, moves <a href="#IRect">IRect</a> returned upward.
If <a href="#SkIRect_offset_dy">dy</a> is positive, moves <a href="#IRect">IRect</a> returned downward.

### Parameters

<table>  <tr>    <td><a name="SkIRect_offset_dx"> <code><strong>dx </strong></code> </a></td> <td>
<a href="#SkIRect_offset">offset</a> added to <a href="#SkIRect_fLeft">fLeft</a> and <a href="#SkIRect_fRight">fRight</a></td>
  </tr>  <tr>    <td><a name="SkIRect_offset_dy"> <code><strong>dy </strong></code> </a></td> <td>
<a href="#SkIRect_offset">offset</a> added to <a href="#SkIRect_fTop">fTop</a> and <a href="#SkIRect_fBottom">fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="77e633b2174ffae923c038b303418b50">

#### Example Output

~~~~
rect: 15, 27, 55, 86
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_offsetTo">offsetTo</a> <a href="#SkIRect_makeOffset">makeOffset</a> <a href="#SkRect_offset">SkRect::offset</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void offset(const SkIPoint& delta)
</pre>

Offsets <a href="#IRect">IRect</a> by adding <a href="#SkIRect_offset_2_delta">delta</a>.fX to <a href="#SkIRect_fLeft">fLeft</a>, <a href="#SkIRect_fRight">fRight</a>; and by adding <a href="#SkIRect_offset_2_delta">delta</a>.fY to
<a href="#SkIRect_fTop">fTop</a>, <a href="#SkIRect_fBottom">fBottom</a>.

If <a href="#SkIRect_offset_2_delta">delta</a>.fX is negative, moves <a href="#IRect">IRect</a> returned to the <a href="#SkIRect_left">left</a>.
If <a href="#SkIRect_offset_2_delta">delta</a>.fX is positive, moves <a href="#IRect">IRect</a> returned to the <a href="#SkIRect_right">right</a>.
If <a href="#SkIRect_offset_2_delta">delta</a>.fY is negative, moves <a href="#IRect">IRect</a> returned upward.
If <a href="#SkIRect_offset_2_delta">delta</a>.fY is positive, moves <a href="#IRect">IRect</a> returned downward.

### Parameters

<table>  <tr>    <td><a name="SkIRect_offset_2_delta"> <code><strong>delta </strong></code> </a></td> <td>
<a href="#SkIRect_offset">offset</a> added to <a href="#IRect">IRect</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="31a4c575499e76def651eb65994876f0">

#### Example Output

~~~~
rect: 15, 27, 55, 86
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_offsetTo">offsetTo</a> <a href="#SkIRect_makeOffset">makeOffset</a> <a href="#SkRect_offset">SkRect::offset</a>

---

<a name="SkIRect_offsetTo"></a>
## offsetTo

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void offsetTo(int32_t newX, int32_t newY)
</pre>

Offsets <a href="#IRect">IRect</a> so that <a href="#SkIRect_fLeft">fLeft</a> equals <a href="#SkIRect_offsetTo_newX">newX</a>, and <a href="#SkIRect_fTop">fTop</a> equals <a href="#SkIRect_offsetTo_newY">newY</a>. <a href="#SkIRect_width">width</a> and <a href="#SkIRect_height">height</a>
are unchanged.

### Parameters

<table>  <tr>    <td><a name="SkIRect_offsetTo_newX"> <code><strong>newX </strong></code> </a></td> <td>
stored in <a href="#SkIRect_fLeft">fLeft</a>, preserving <a href="#SkIRect_width">width</a></td>
  </tr>  <tr>    <td><a name="SkIRect_offsetTo_newY"> <code><strong>newY </strong></code> </a></td> <td>
stored in <a href="#SkIRect_fTop">fTop</a>, preserving <a href="#SkIRect_height">height</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a2734ff23b35653956a3002e5c29ff91">

#### Example Output

~~~~
rect: 15, 27, 55, 86
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_offset">offset</a> <a href="#SkIRect_makeOffset">makeOffset</a> <a href="#SkIRect_setXYWH">setXYWH</a> <a href="#SkRect_offsetTo">SkRect::offsetTo</a>

---

<a name="SkIRect_inset"></a>
## inset

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void inset(int32_t dx, int32_t dy)
</pre>

Insets <a href="#IRect">IRect</a> by (<a href="#SkIRect_inset_dx">dx</a>,<a href="#SkIRect_inset_dy">dy</a>).

If <a href="#SkIRect_inset_dx">dx</a> is positive, makes <a href="#IRect">IRect</a> narrower.
If <a href="#SkIRect_inset_dx">dx</a> is negative, makes <a href="#IRect">IRect</a> wider.
If <a href="#SkIRect_inset_dy">dy</a> is positive, makes <a href="#IRect">IRect</a> shorter.
If <a href="#SkIRect_inset_dy">dy</a> is negative, makes <a href="#IRect">IRect</a> taller.

### Parameters

<table>  <tr>    <td><a name="SkIRect_inset_dx"> <code><strong>dx </strong></code> </a></td> <td>
<a href="#SkIRect_offset">offset</a> added to <a href="#SkIRect_fLeft">fLeft</a> and subtracted from <a href="#SkIRect_fRight">fRight</a></td>
  </tr>  <tr>    <td><a name="SkIRect_inset_dy"> <code><strong>dy </strong></code> </a></td> <td>
<a href="#SkIRect_offset">offset</a> added to <a href="#SkIRect_fTop">fTop</a> and subtracted from <a href="#SkIRect_fBottom">fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="9debaded1aa8bdf5077a4de0b3015b8f">

#### Example Output

~~~~
rect: 15, 27, 45, 60
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_outset">outset</a> <a href="#SkIRect_makeInset">makeInset</a> <a href="#SkRect_inset">SkRect::inset</a>

---

<a name="SkIRect_outset"></a>
## outset

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void outset(int32_t dx, int32_t dy)
</pre>

Outsets <a href="#IRect">IRect</a> by (<a href="#SkIRect_outset_dx">dx</a>, <a href="#SkIRect_outset_dy">dy</a>).

If <a href="#SkIRect_outset_dx">dx</a> is positive, makes <a href="SkRect_Reference#Rect">Rect</a> wider.
If <a href="#SkIRect_outset_dx">dx</a> is negative, makes <a href="SkRect_Reference#Rect">Rect</a> narrower.
If <a href="#SkIRect_outset_dy">dy</a> is positive, makes <a href="SkRect_Reference#Rect">Rect</a> taller.
If <a href="#SkIRect_outset_dy">dy</a> is negative, makes <a href="SkRect_Reference#Rect">Rect</a> shorter.

### Parameters

<table>  <tr>    <td><a name="SkIRect_outset_dx"> <code><strong>dx </strong></code> </a></td> <td>
subtracted to <a href="#SkIRect_fLeft">fLeft</a> and added from <a href="#SkIRect_fRight">fRight</a></td>
  </tr>  <tr>    <td><a name="SkIRect_outset_dy"> <code><strong>dy </strong></code> </a></td> <td>
subtracted to <a href="#SkIRect_fTop">fTop</a> and added from <a href="#SkIRect_fBottom">fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3fc62ca29428195f33a3a02b3eb74e4f">

#### Example Output

~~~~
rect: 5, 1, 55, 86
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_inset">inset</a> <a href="#SkIRect_makeOutset">makeOutset</a> <a href="#SkRect_outset">SkRect::outset</a>

---

<a name="SkIRect_quickReject"></a>
## quickReject

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool quickReject(int l, int t, int r, int b) const
</pre>

Constructs <a href="#IRect">IRect</a> (<a href="#SkIRect_quickReject_l">l</a>, <a href="#SkIRect_quickReject_t">t</a>, <a href="#SkIRect_quickReject_r">r</a>, <a href="#SkIRect_quickReject_b">b</a>) and returns true if constructed <a href="#IRect">IRect</a> does not
<a href="#SkIRect_intersect">intersect</a> <a href="#IRect">IRect</a>. Does not check to see if construction or <a href="#IRect">IRect</a> is empty.

Is implemented with short circuit logic so that true can be returned after
a single compare.

### Parameters

<table>  <tr>    <td><a name="SkIRect_quickReject_l"> <code><strong>l </strong></code> </a></td> <td>
<a href="#SkIRect_x">x</a> minimum of constructed <a href="SkRect_Reference#Rect">Rect</a></td>
  </tr>  <tr>    <td><a name="SkIRect_quickReject_t"> <code><strong>t </strong></code> </a></td> <td>
<a href="#SkIRect_y">y</a> minimum of constructed <a href="SkRect_Reference#Rect">Rect</a></td>
  </tr>  <tr>    <td><a name="SkIRect_quickReject_r"> <code><strong>r </strong></code> </a></td> <td>
<a href="#SkIRect_x">x</a> maximum of constructed <a href="SkRect_Reference#Rect">Rect</a></td>
  </tr>  <tr>    <td><a name="SkIRect_quickReject_b"> <code><strong>b </strong></code> </a></td> <td>
<a href="#SkIRect_y">y</a> maximum of constructed <a href="SkRect_Reference#Rect">Rect</a></td>
  </tr>
</table>

### Return Value

true if construction and <a href="#IRect">IRect</a> have no area in common

### Example

<div><fiddle-embed name="f07146508efc516559d73853e6dadc78"><div><a href="#SkIRect_quickReject">quickReject</a> is the complement of <a href="#SkIRect_Intersects">Intersects</a>.</div>

#### Example Output

~~~~
rect (7, 11, 13, 17) test(13, 11, 15, 17) quickReject true; intersects false
rect (7, 11, 13, 17) test(7, 7, 13, 11) quickReject true; intersects false
rect (7, 11, 13, 17) test(12, 16, 14, 18) quickReject false; intersects true
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_Intersects">Intersects</a>

---

<a name="SkIRect_contains"></a>
## contains

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool contains(int32_t x, int32_t y) const
</pre>

Returns true if:
<a href="#SkIRect_fLeft">fLeft</a> <= <a href="#SkIRect_x">x</a> < <a href="#SkIRect_fRight">fRight</a> && <a href="#SkIRect_fTop">fTop</a> <= <a href="#SkIRect_y">y</a> < <a href="#SkIRect_fBottom">fBottom</a>.

Returns false if <a href="SkRect_Reference#Rect">Rect</a> is empty.

Considers input to describe constructed <a href="#IRect">IRect</a>:
(<a href="#SkIRect_x">x</a>, <a href="#SkIRect_y">y</a>, <a href="#SkIRect_x">x</a> + 1, <a href="#SkIRect_y">y</a> + 1)and
returns true if constructed area is completely enclosed by <a href="#IRect">IRect</a> area.

### Parameters

<table>  <tr>    <td><a name="SkIRect_contains_x"> <code><strong>x </strong></code> </a></td> <td>
test <a href="SkPoint_Reference#Point">Point</a> <a href="#SkIRect_x">x</a>-coordinate</td>
  </tr>  <tr>    <td><a name="SkIRect_contains_y"> <code><strong>y </strong></code> </a></td> <td>
test <a href="SkPoint_Reference#Point">Point</a> <a href="#SkIRect_y">y</a>-coordinate</td>
  </tr>
</table>

### Return Value

true if (<a href="#SkIRect_x">x</a>, <a href="#SkIRect_y">y</a>) is inside <a href="#IRect">IRect</a>

### Example

<div><fiddle-embed name="a7958a4e0668f5cf805a8e78eb57f51d">

#### Example Output

~~~~
rect: (30, 50, 40, 60) contains (30, 50)
rect: (30, 50, 40, 60) does not contain (40, 50)
rect: (30, 50, 40, 60) does not contain (30, 60)
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_containsNoEmptyCheck">containsNoEmptyCheck</a> <a href="#SkRect_contains">SkRect::contains</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool contains(int32_t left, int32_t top, int32_t right, int32_t bottom) const
</pre>

Constructs <a href="SkRect_Reference#Rect">Rect</a> to <a href="#SkIRect_intersect">intersect</a> from (<a href="#SkIRect_left">left</a>, <a href="#SkIRect_top">top</a>, <a href="#SkIRect_right">right</a>, <a href="#SkIRect_bottom">bottom</a>). Does not <a href="#SkIRect_sort">sort</a>
construction.

Returns true if <a href="SkRect_Reference#Rect">Rect</a> <a href="#SkIRect_contains">contains</a> construction.
Returns false if <a href="SkRect_Reference#Rect">Rect</a> is empty or construction is empty.

### Parameters

<table>  <tr>    <td><a name="SkIRect_contains_2_left"> <code><strong>left </strong></code> </a></td> <td>
<a href="#SkIRect_x">x</a> minimum of constructed <a href="SkRect_Reference#Rect">Rect</a></td>
  </tr>  <tr>    <td><a name="SkIRect_contains_2_top"> <code><strong>top </strong></code> </a></td> <td>
<a href="#SkIRect_y">y</a> minimum of constructed <a href="SkRect_Reference#Rect">Rect</a></td>
  </tr>  <tr>    <td><a name="SkIRect_contains_2_right"> <code><strong>right </strong></code> </a></td> <td>
<a href="#SkIRect_x">x</a> maximum of constructed <a href="SkRect_Reference#Rect">Rect</a></td>
  </tr>  <tr>    <td><a name="SkIRect_contains_2_bottom"> <code><strong>bottom </strong></code> </a></td> <td>
<a href="#SkIRect_y">y</a> maximum of constructed <a href="SkRect_Reference#Rect">Rect</a></td>
  </tr>
</table>

### Return Value

true if all sides of <a href="#IRect">IRect</a> are outside construction

### Example

<div><fiddle-embed name="eae55f284818d9965ec5834747d14a48">

#### Example Output

~~~~
rect: (30, 50, 40, 60) contains (30, 50, 31, 51)
rect: (30, 50, 40, 60) does not contain (39, 49, 40, 50)
rect: (30, 50, 40, 60) does not contain (29, 59, 30, 60)
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_containsNoEmptyCheck">containsNoEmptyCheck</a> <a href="#SkRect_contains">SkRect::contains</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool contains(const SkIRect& r) const
</pre>

Returns true if <a href="SkRect_Reference#Rect">Rect</a> <a href="#SkIRect_contains">contains</a> <a href="#SkIRect_contains_3_r">r</a>.
Returns false if <a href="SkRect_Reference#Rect">Rect</a> is empty or <a href="#SkIRect_contains_3_r">r</a> is empty.

<a href="SkRect_Reference#Rect">Rect</a> <a href="#SkIRect_contains">contains</a> <a href="#SkIRect_contains_3_r">r</a> when <a href="SkRect_Reference#Rect">Rect</a> area completely includes <a href="#SkIRect_contains_3_r">r</a> area.

### Parameters

<table>  <tr>    <td><a name="SkIRect_contains_3_r"> <code><strong>r </strong></code> </a></td> <td>
<a href="#IRect">IRect</a> contained</td>
  </tr>
</table>

### Return Value

true if all sides of <a href="#IRect">IRect</a> are outside <a href="#SkIRect_contains_3_r">r</a>

### Example

<div><fiddle-embed name="ee0185db622602b4eb19583c2f42c734">

#### Example Output

~~~~
rect: (30, 50, 40, 60) contains (30, 50, 31, 51)
rect: (30, 50, 40, 60) does not contain (39, 49, 40, 50)
rect: (30, 50, 40, 60) does not contain (29, 59, 30, 60)
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_containsNoEmptyCheck">containsNoEmptyCheck</a> <a href="#SkRect_contains">SkRect::contains</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool contains(const SkRect& r) const
</pre>

Returns true if <a href="SkRect_Reference#Rect">Rect</a> <a href="#SkIRect_contains">contains</a> <a href="#SkIRect_contains_4_r">r</a>.
Returns false if <a href="SkRect_Reference#Rect">Rect</a> is empty or <a href="#SkIRect_contains_4_r">r</a> is empty.

<a href="SkRect_Reference#Rect">Rect</a> <a href="#SkIRect_contains">contains</a> <a href="#SkIRect_contains_4_r">r</a> when <a href="SkRect_Reference#Rect">Rect</a> area completely includes <a href="#SkIRect_contains_4_r">r</a> area.

### Parameters

<table>  <tr>    <td><a name="SkIRect_contains_4_r"> <code><strong>r </strong></code> </a></td> <td>
<a href="SkRect_Reference#Rect">Rect</a> contained</td>
  </tr>
</table>

### Return Value

true if all sides of <a href="#IRect">IRect</a> are outside <a href="#SkIRect_contains_4_r">r</a>

### Example

<div><fiddle-embed name="acbd79ffb304f332e4b38ef18e19663e">

#### Example Output

~~~~
rect: (30, 50, 40, 60) contains (30, 50, 31, 51)
rect: (30, 50, 40, 60) does not contain (39, 49, 40, 50)
rect: (30, 50, 40, 60) does not contain (29, 59, 30, 60)
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_containsNoEmptyCheck">containsNoEmptyCheck</a> <a href="#SkRect_contains">SkRect::contains</a>

---

<a name="SkIRect_containsNoEmptyCheck"></a>
## containsNoEmptyCheck

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool containsNoEmptyCheck(int32_t left, int32_t top, int32_t right, int32_t bottom) const
</pre>

Constructs <a href="#IRect">IRect</a> from (<a href="#SkIRect_left">left</a>, <a href="#SkIRect_top">top</a>, <a href="#SkIRect_right">right</a>, <a href="#SkIRect_bottom">bottom</a>). Does not <a href="#SkIRect_sort">sort</a>
construction.

Returns true if <a href="SkRect_Reference#Rect">Rect</a> <a href="#SkIRect_contains">contains</a> construction.
Asserts if <a href="#IRect">IRect</a> is empty or construction is empty, and if SK_DEBUG is defined.

Return is undefined if <a href="SkRect_Reference#Rect">Rect</a> is empty or construction is empty.

### Parameters

<table>  <tr>    <td><a name="SkIRect_containsNoEmptyCheck_left"> <code><strong>left </strong></code> </a></td> <td>
<a href="#SkIRect_x">x</a> minimum of constructed <a href="SkRect_Reference#Rect">Rect</a></td>
  </tr>  <tr>    <td><a name="SkIRect_containsNoEmptyCheck_top"> <code><strong>top </strong></code> </a></td> <td>
<a href="#SkIRect_y">y</a> minimum of constructed <a href="SkRect_Reference#Rect">Rect</a></td>
  </tr>  <tr>    <td><a name="SkIRect_containsNoEmptyCheck_right"> <code><strong>right </strong></code> </a></td> <td>
<a href="#SkIRect_x">x</a> maximum of constructed <a href="SkRect_Reference#Rect">Rect</a></td>
  </tr>  <tr>    <td><a name="SkIRect_containsNoEmptyCheck_bottom"> <code><strong>bottom </strong></code> </a></td> <td>
<a href="#SkIRect_y">y</a> maximum of constructed <a href="SkRect_Reference#Rect">Rect</a></td>
  </tr>
</table>

### Return Value

true if all sides of <a href="#IRect">IRect</a> are outside construction

### Example

<div><fiddle-embed name="fef2a36bee224e92500199fa9d3cbb8b">

#### Example Output

~~~~
rect: (30, 50, 40, 60) contains (30, 50, 31, 51)
rect: (30, 50, 40, 60) does not contain (39, 49, 40, 50)
rect: (30, 50, 40, 60) does not contain (29, 59, 30, 60)
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_contains">contains</a> <a href="#SkRect_contains">SkRect::contains</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool containsNoEmptyCheck(const SkIRect& r) const
</pre>

Returns true if <a href="SkRect_Reference#Rect">Rect</a> <a href="#SkIRect_contains">contains</a> construction.
Asserts if <a href="#IRect">IRect</a> is empty or construction is empty, and if SK_DEBUG is defined.

Return is undefined if <a href="SkRect_Reference#Rect">Rect</a> is empty or construction is empty.

### Parameters

<table>  <tr>    <td><a name="SkIRect_containsNoEmptyCheck_2_r"> <code><strong>r </strong></code> </a></td> <td>
<a href="SkRect_Reference#Rect">Rect</a> contained</td>
  </tr>
</table>

### Return Value

true if all sides of <a href="#IRect">IRect</a> are outside <a href="#SkIRect_containsNoEmptyCheck_2_r">r</a>

### Example

<div><fiddle-embed name="8f91f58001d9c10420eb146fbc169af4">

#### Example Output

~~~~
rect: (30, 50, 40, 60) contains (30, 50, 31, 51)
rect: (30, 50, 40, 60) does not contain (39, 49, 40, 50)
rect: (30, 50, 40, 60) does not contain (29, 59, 30, 60)
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_contains">contains</a> <a href="#SkRect_contains">SkRect::contains</a>

---

# <a name="Intersection"></a> Intersection
IRects <a href="#SkIRect_intersect">intersect</a> when they enclose a common area. To <a href="#SkIRect_intersect">intersect</a>, each of the pair
must describe area; <a href="#SkIRect_fLeft">fLeft</a> is less than <a href="#SkIRect_fRight">fRight</a>, and <a href="#SkIRect_fTop">fTop</a> is less than <a href="#SkIRect_fBottom">fBottom</a>;
empty() returns false. The intersection of <a href="#IRect">IRect</a> pair can be described by:
(max(a.fLeft, b.fLeft), max(a.fTop, b.fTop),
min(a.fRight, b.fRight), min(a.fBottom, b.fBottom))The intersection is only meaningful if the resulting <a href="#IRect">IRect</a> is not empty and
describes an area: <a href="#SkIRect_fLeft">fLeft</a> is less than <a href="#SkIRect_fRight">fRight</a>, and <a href="#SkIRect_fTop">fTop</a> is less than <a href="#SkIRect_fBottom">fBottom</a>.

<a name="SkIRect_intersect"></a>
## intersect

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool intersect(const SkIRect& r)
</pre>

Returns true if <a href="#IRect">IRect</a> intersects <a href="#SkIRect_intersect_r">r</a>, and sets <a href="#IRect">IRect</a> to intersection.
Returns false if <a href="#IRect">IRect</a> does not <a href="#SkIRect_intersect">intersect</a> <a href="#SkIRect_intersect_r">r</a>, and leaves <a href="#IRect">IRect</a> unchanged.

Returns false if either <a href="#SkIRect_intersect_r">r</a> or <a href="#IRect">IRect</a> is empty, leaving <a href="#IRect">IRect</a> unchanged.

### Parameters

<table>  <tr>    <td><a name="SkIRect_intersect_r"> <code><strong>r </strong></code> </a></td> <td>
limit of result</td>
  </tr>
</table>

### Return Value

true if <a href="#SkIRect_intersect_r">r</a> and <a href="SkRect_Reference#Rect">Rect</a> have area in common

### Example

<div><fiddle-embed name="2be1302480e54a767e25cbeed5d41b41"><div>Two <a href="undocumented#SkDebugf">SkDebugf</a> calls are required. If the calls are combined, their arguments
may not be evaluated in <a href="#SkIRect_left">left</a> to <a href="#SkIRect_right">right</a> order: the printed intersection may
be before or after the call to <a href="#SkIRect_intersect">intersect</a>.</div>

#### Example Output

~~~~
intersection: 30, 60, 50, 80
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_Intersects">Intersects</a> <a href="#SkIRect_intersectNoEmptyCheck">intersectNoEmptyCheck</a> <a href="#SkIRect_join">join</a> <a href="#SkRect_intersect">SkRect::intersect</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool SK_WARN_UNUSED_RESULT intersect(const SkIRect& a, const SkIRect& b)
</pre>

Returns true if <a href="#SkIRect_intersect_2_a">a</a> intersects <a href="#SkIRect_intersect_2_b">b</a>, and sets <a href="#IRect">IRect</a> to intersection.
Returns false if <a href="#SkIRect_intersect_2_a">a</a> does not <a href="#SkIRect_intersect">intersect</a> <a href="#SkIRect_intersect_2_b">b</a>, and leaves <a href="#IRect">IRect</a> unchanged.

Returns false if either <a href="#SkIRect_intersect_2_a">a</a> or <a href="#SkIRect_intersect_2_b">b</a> is empty, leaving <a href="#IRect">IRect</a> unchanged.

### Parameters

<table>  <tr>    <td><a name="SkIRect_intersect_2_a"> <code><strong>a </strong></code> </a></td> <td>
<a href="#IRect">IRect</a> to <a href="#SkIRect_intersect">intersect</a></td>
  </tr>  <tr>    <td><a name="SkIRect_intersect_2_b"> <code><strong>b </strong></code> </a></td> <td>
<a href="#IRect">IRect</a> to <a href="#SkIRect_intersect">intersect</a></td>
  </tr>
</table>

### Return Value

true if <a href="#SkIRect_intersect_2_a">a</a> and <a href="#SkIRect_intersect_2_b">b</a> have area in common

### Example

<div><fiddle-embed name="b2db0573aacf99ca52776c5522459d02">

#### Example Output

~~~~
intersection: 30, 60, 50, 80
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_Intersects">Intersects</a> <a href="#SkIRect_intersectNoEmptyCheck">intersectNoEmptyCheck</a> <a href="#SkIRect_join">join</a> <a href="#SkRect_intersect">SkRect::intersect</a>

---

<a name="SkIRect_intersectNoEmptyCheck"></a>
## intersectNoEmptyCheck

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool SK_WARN_UNUSED_RESULT intersectNoEmptyCheck(const SkIRect& a, const SkIRect& b)
</pre>

Returns true if <a href="#SkIRect_intersectNoEmptyCheck_a">a</a> intersects <a href="#SkIRect_intersectNoEmptyCheck_b">b</a>, and sets <a href="#IRect">IRect</a> to intersection.
Returns false if <a href="#SkIRect_intersectNoEmptyCheck_a">a</a> does not <a href="#SkIRect_intersect">intersect</a> <a href="#SkIRect_intersectNoEmptyCheck_b">b</a>, and leaves <a href="#IRect">IRect</a> unchanged.

Asserts if either <a href="#SkIRect_intersectNoEmptyCheck_a">a</a> or <a href="#SkIRect_intersectNoEmptyCheck_b">b</a> is empty, and if SK_DEBUG is defined.

### Parameters

<table>  <tr>    <td><a name="SkIRect_intersectNoEmptyCheck_a"> <code><strong>a </strong></code> </a></td> <td>
<a href="#IRect">IRect</a> to <a href="#SkIRect_intersect">intersect</a></td>
  </tr>  <tr>    <td><a name="SkIRect_intersectNoEmptyCheck_b"> <code><strong>b </strong></code> </a></td> <td>
<a href="#IRect">IRect</a> to <a href="#SkIRect_intersect">intersect</a></td>
  </tr>
</table>

### Return Value

true if <a href="#SkIRect_intersectNoEmptyCheck_a">a</a> and <a href="#SkIRect_intersectNoEmptyCheck_b">b</a> have area in common

### Example

<div><fiddle-embed name="2b3e26ccba1cba3d961645f0824621ac">

#### Example Output

~~~~
intersection: 30, 60, 50, 80
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_Intersects">Intersects</a> <a href="#SkIRect_intersect">intersect</a> <a href="#SkIRect_join">join</a> <a href="#SkRect_intersect">SkRect::intersect</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool intersect(int32_t left, int32_t top, int32_t right, int32_t bottom)
</pre>

Constructs <a href="#IRect">IRect</a> to <a href="#SkIRect_intersect">intersect</a> from (<a href="#SkIRect_left">left</a>, <a href="#SkIRect_top">top</a>, <a href="#SkIRect_right">right</a>, <a href="#SkIRect_bottom">bottom</a>). Does not <a href="#SkIRect_sort">sort</a>
construction.

Returns true if <a href="#IRect">IRect</a> intersects construction, and sets <a href="#IRect">IRect</a> to intersection.
Returns false if <a href="#IRect">IRect</a> does not <a href="#SkIRect_intersect">intersect</a> construction, and leaves <a href="#IRect">IRect</a> unchanged.

Returns false if either construction or <a href="#IRect">IRect</a> is empty, leaving <a href="#IRect">IRect</a> unchanged.

### Parameters

<table>  <tr>    <td><a name="SkIRect_intersect_3_left"> <code><strong>left </strong></code> </a></td> <td>
<a href="#SkIRect_x">x</a> minimum of constructed <a href="#IRect">IRect</a></td>
  </tr>  <tr>    <td><a name="SkIRect_intersect_3_top"> <code><strong>top </strong></code> </a></td> <td>
<a href="#SkIRect_y">y</a> minimum of constructed <a href="#IRect">IRect</a></td>
  </tr>  <tr>    <td><a name="SkIRect_intersect_3_right"> <code><strong>right </strong></code> </a></td> <td>
<a href="#SkIRect_x">x</a> maximum of constructed <a href="#IRect">IRect</a></td>
  </tr>  <tr>    <td><a name="SkIRect_intersect_3_bottom"> <code><strong>bottom </strong></code> </a></td> <td>
<a href="#SkIRect_y">y</a> maximum of constructed <a href="#IRect">IRect</a></td>
  </tr>
</table>

### Return Value

true if construction and <a href="#IRect">IRect</a> have area in common

### Example

<div><fiddle-embed name="4e6f580a3906c08a5faee524f7e72334"><div>Two <a href="undocumented#SkDebugf">SkDebugf</a> calls are required. If the calls are combined, their arguments
may not be evaluated in <a href="#SkIRect_left">left</a> to <a href="#SkIRect_right">right</a> order: the printed intersection may
be before or after the call to <a href="#SkIRect_intersect">intersect</a>.</div>

#### Example Output

~~~~
intersection: 30, 60, 50, 80
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_intersectNoEmptyCheck">intersectNoEmptyCheck</a> <a href="#SkIRect_Intersects">Intersects</a> <a href="#SkIRect_join">join</a> <a href="#SkRect_intersect">SkRect::intersect</a>

---

<a name="SkIRect_Intersects"></a>
## Intersects

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static bool Intersects(const SkIRect& a, const SkIRect& b)
</pre>

Returns true if <a href="#SkIRect_Intersects_a">a</a> intersects <a href="#SkIRect_Intersects_b">b</a>.
Returns false if either <a href="#SkIRect_Intersects_a">a</a> or <a href="#SkIRect_Intersects_b">b</a> is empty, or do not <a href="#SkIRect_intersect">intersect</a>.

### Parameters

<table>  <tr>    <td><a name="SkIRect_Intersects_a"> <code><strong>a </strong></code> </a></td> <td>
<a href="#IRect">IRect</a> to <a href="#SkIRect_intersect">intersect</a></td>
  </tr>  <tr>    <td><a name="SkIRect_Intersects_b"> <code><strong>b </strong></code> </a></td> <td>
<a href="#IRect">IRect</a> to <a href="#SkIRect_intersect">intersect</a></td>
  </tr>
</table>

### Return Value

true if <a href="#SkIRect_Intersects_a">a</a> and <a href="#SkIRect_Intersects_b">b</a> have area in common

### Example

<div><fiddle-embed name="0c67cf8981389efc7108369fb9b7976b">

#### Example Output

~~~~
intersection
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_IntersectsNoEmptyCheck">IntersectsNoEmptyCheck</a> <a href="#SkIRect_intersect">intersect</a> <a href="#SkRect_intersect">SkRect::intersect</a>

---

<a name="SkIRect_IntersectsNoEmptyCheck"></a>
## IntersectsNoEmptyCheck

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static bool IntersectsNoEmptyCheck(const SkIRect& a, const SkIRect& b)
</pre>

Returns true if <a href="#SkIRect_IntersectsNoEmptyCheck_a">a</a> intersects <a href="#SkIRect_IntersectsNoEmptyCheck_b">b</a>.
Asserts if either <a href="#SkIRect_IntersectsNoEmptyCheck_a">a</a> or <a href="#SkIRect_IntersectsNoEmptyCheck_b">b</a> is empty, and if SK_DEBUG is defined.

### Parameters

<table>  <tr>    <td><a name="SkIRect_IntersectsNoEmptyCheck_a"> <code><strong>a </strong></code> </a></td> <td>
<a href="#IRect">IRect</a> to <a href="#SkIRect_intersect">intersect</a></td>
  </tr>  <tr>    <td><a name="SkIRect_IntersectsNoEmptyCheck_b"> <code><strong>b </strong></code> </a></td> <td>
<a href="#IRect">IRect</a> to <a href="#SkIRect_intersect">intersect</a></td>
  </tr>
</table>

### Return Value

true if <a href="#SkIRect_IntersectsNoEmptyCheck_a">a</a> and <a href="#SkIRect_IntersectsNoEmptyCheck_b">b</a> have area in common

### Example

<div><fiddle-embed name="dba234d15162fb5b26e1a96529ca6a2a">

#### Example Output

~~~~
intersection
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_Intersects">Intersects</a> <a href="#SkIRect_intersect">intersect</a> <a href="#SkRect_intersect">SkRect::intersect</a>

---

<a name="SkIRect_join"></a>
## join

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void join(int32_t left, int32_t top, int32_t right, int32_t bottom)
</pre>

Constructs <a href="SkRect_Reference#Rect">Rect</a> to <a href="#SkIRect_intersect">intersect</a> from (<a href="#SkIRect_left">left</a>, <a href="#SkIRect_top">top</a>, <a href="#SkIRect_right">right</a>, <a href="#SkIRect_bottom">bottom</a>). Does not <a href="#SkIRect_sort">sort</a>
construction.

Sets <a href="SkRect_Reference#Rect">Rect</a> to the union of itself and the construction.

Has no effect if construction is empty. Otherwise, if <a href="SkRect_Reference#Rect">Rect</a> is empty, sets
<a href="SkRect_Reference#Rect">Rect</a> to construction.

### Parameters

<table>  <tr>    <td><a name="SkIRect_join_left"> <code><strong>left </strong></code> </a></td> <td>
<a href="#SkIRect_x">x</a> minimum of constructed <a href="SkRect_Reference#Rect">Rect</a></td>
  </tr>  <tr>    <td><a name="SkIRect_join_top"> <code><strong>top </strong></code> </a></td> <td>
<a href="#SkIRect_y">y</a> minimum of constructed <a href="SkRect_Reference#Rect">Rect</a></td>
  </tr>  <tr>    <td><a name="SkIRect_join_right"> <code><strong>right </strong></code> </a></td> <td>
<a href="#SkIRect_x">x</a> maximum of constructed <a href="SkRect_Reference#Rect">Rect</a></td>
  </tr>  <tr>    <td><a name="SkIRect_join_bottom"> <code><strong>bottom </strong></code> </a></td> <td>
<a href="#SkIRect_y">y</a> maximum of constructed <a href="SkRect_Reference#Rect">Rect</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c00ef06289d21db70340e465690e0e08">

#### Example Output

~~~~
join: 10, 20, 55, 65
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_set">set</a> <a href="#SkRect_join">SkRect::join</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void join(const SkIRect& r)
</pre>

Sets <a href="SkRect_Reference#Rect">Rect</a> to the union of itself and <a href="#SkIRect_join_2_r">r</a>.

Has no effect if <a href="#SkIRect_join_2_r">r</a> is empty. Otherwise, if <a href="SkRect_Reference#Rect">Rect</a> is empty, sets <a href="SkRect_Reference#Rect">Rect</a> to <a href="#SkIRect_join_2_r">r</a>.

### Parameters

<table>  <tr>    <td><a name="SkIRect_join_2_r"> <code><strong>r </strong></code> </a></td> <td>
expansion <a href="SkRect_Reference#Rect">Rect</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="75fd81c1d3512e63890d085593018876">

#### Example Output

~~~~
join: 10, 20, 55, 65
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_set">set</a> <a href="#SkRect_join">SkRect::join</a>

---

<a name="SkIRect_sort"></a>
## sort

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void sort()
</pre>

Swaps <a href="#SkIRect_fLeft">fLeft</a> and <a href="#SkIRect_fRight">fRight</a> if <a href="#SkIRect_fLeft">fLeft</a> is greater than <a href="#SkIRect_fRight">fRight</a>; and swaps
<a href="#SkIRect_fTop">fTop</a> and <a href="#SkIRect_fBottom">fBottom</a> if <a href="#SkIRect_fTop">fTop</a> is greater than <a href="#SkIRect_fBottom">fBottom</a>. Result may be empty,
and <a href="#SkIRect_width">width</a> and <a href="#SkIRect_height">height</a> will be zero or positive.

### Example

<div><fiddle-embed name="fa12547fcfd4c1aef3db1a1f6aae0fe4">

#### Example Output

~~~~
rect: 30, 50, 20, 10
sorted: 20, 10, 30, 50
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_makeSorted">makeSorted</a> <a href="#SkRect_sort">SkRect::sort</a>

---

<a name="SkIRect_makeSorted"></a>
## makeSorted

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkIRect makeSorted() const
</pre>

Returns <a href="SkRect_Reference#Rect">Rect</a> with <a href="#SkIRect_fLeft">fLeft</a> and <a href="#SkIRect_fRight">fRight</a> swapped if <a href="#SkIRect_fLeft">fLeft</a> is greater than <a href="#SkIRect_fRight">fRight</a>; and
with <a href="#SkIRect_fTop">fTop</a> and <a href="#SkIRect_fBottom">fBottom</a> swapped if <a href="#SkIRect_fTop">fTop</a> is greater than <a href="#SkIRect_fBottom">fBottom</a>. Result may be empty;
and <a href="#SkIRect_width">width</a> and <a href="#SkIRect_height">height</a> will be zero or positive.

### Return Value

sorted <a href="#IRect">IRect</a>

### Example

<div><fiddle-embed name="de89926c374aa16427916900b89a3441">

#### Example Output

~~~~
rect: 30, 50, 20, 10
sorted: 20, 10, 30, 50
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_sort">sort</a> <a href="#SkRect_makeSorted">SkRect::makeSorted</a>

---

<a name="SkIRect_EmptyIRect"></a>
## EmptyIRect

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static const SkIRect& SK_WARN_UNUSED_RESULT EmptyIRect()
</pre>

Returns a reference to immutable empty <a href="#IRect">IRect</a>, <a href="#SkIRect_set">set</a> to (0, 0, 0, 0).

### Return Value

global <a href="#IRect">IRect</a> <a href="#SkIRect_set">set</a> to all zeroes

### Example

<div><fiddle-embed name="65e0b9b52e907902630577941fb3ed6d">

#### Example Output

~~~~
rect: 0, 0, 0, 0
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIRect_MakeEmpty">MakeEmpty</a>

---

<a name="SkIRect_MakeLargest"></a>
## MakeLargest

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static SkIRect SK_WARN_UNUSED_RESULT MakeLargest()
</pre>

Returns constructed <a href="#SkIRect">SkIRect</a> setting <a href="#SkIRect_left">left</a> and <a href="#SkIRect_top">top</a> to most negative value, and
setting <a href="#SkIRect_right">right</a> and <a href="#SkIRect_bottom">bottom</a> to most positive value.

### Return Value

bounds (<a href="undocumented#SK_MinS32">SK MinS32</a>, <a href="undocumented#SK_MinS32">SK MinS32</a>, <a href="undocumented#SK_MaxS32">SK MaxS32</a>, <a href="undocumented#SK_MaxS32">SK MaxS32</a>)

---

