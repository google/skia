SkIPoint16 Reference
===

# <a name="IPoint16"></a> IPoint16

# <a name="SkIPoint16"></a> Struct SkIPoint16
<a href="SkIPoint_Reference#SkIPoint">SkIPoint</a> holds two 16 bit integer coordinates

# <a name="Overview"></a> Overview

## <a name="Subtopics"></a> Subtopics

| topics | description |
| --- | ---  |

## <a name="Member_Functions"></a> Member Functions

| description | function |
| --- | ---  |
| <a href="#SkIPoint16_Make">Make</a> | Constructs from integer inputs. |
| <a href="#SkIPoint16_set">set</a> | Sets to integer input. |
| <a href="#SkIPoint16_x">x</a> | Returns <a href="#SkIPoint16_fX">fX</a>. |
| <a href="#SkIPoint16_y">y</a> | Returns <a href="#SkIPoint16_fY">fY</a>. |

<a name="SkIPoint16_fX"> <code><strong>int16_t  fX</strong></code> </a>

<a href="#SkIPoint16_x">x</a>-axis value used by <a href="#IPoint16">IPoint16</a>.

<a name="SkIPoint16_fY"> <code><strong>int16_t  fY</strong></code> </a>

<a href="#SkIPoint16_y">y</a>-axis value used by <a href="#IPoint16">IPoint16</a>.

<a name="SkIPoint16_Make"></a>
## Make

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static constexpr SkIPoint16 Make(int x, int y)
</pre>

Sets <a href="#SkIPoint16_fX">fX</a> to <a href="#SkIPoint16_x">x</a>, <a href="#SkIPoint16_fY">fY</a> to <a href="#SkIPoint16_y">y</a>. If SK_DEBUG is defined, asserts
if <a href="#SkIPoint16_x">x</a> or <a href="#SkIPoint16_y">y</a> does not fit in 16 bits.

### Parameters

<table>  <tr>    <td><a name="SkIPoint16_Make_x"> <code><strong>x </strong></code> </a></td> <td>
integer <a href="#SkIPoint16_x">x</a>-axis value of constructed <a href="SkIPoint_Reference#IPoint">IPoint</a></td>
  </tr>  <tr>    <td><a name="SkIPoint16_Make_y"> <code><strong>y </strong></code> </a></td> <td>
integer <a href="#SkIPoint16_y">y</a>-axis value of constructed <a href="SkIPoint_Reference#IPoint">IPoint</a></td>
  </tr>
</table>

### Return Value

<a href="#IPoint16">IPoint16</a> (<a href="#SkIPoint16_x">x</a>, <a href="#SkIPoint16_y">y</a>)

### Example

<div><fiddle-embed name="d815ca04fbf22b5acec6f85b6351f362">

#### Example Output

~~~~
pt1.fX == pt2.fX
pt1.fY == pt2.fY
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIPoint16_set">set</a> <a href="#SkPoint_iset">SkPoint::iset()</a> <a href="#SkIPoint_Make">SkIPoint::Make</a>

---

<a name="SkIPoint16_x"></a>
## x

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int16_t x() const
</pre>

Returns <a href="#SkIPoint16_x">x</a>-axis value of <a href="#IPoint16">IPoint16</a>.

### Return Value

<a href="#SkIPoint16_fX">fX</a>

### Example

<div><fiddle-embed name="f7fd3b3674f042869de3582ab793dbf7">

#### Example Output

~~~~
pt1.fX == pt1.x()
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIPoint16_y">y</a> <a href="#SkIPoint_x">SkIPoint::x()</a>

---

<a name="SkIPoint16_y"></a>
## y

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
int16_t y() const
</pre>

Returns <a href="#SkIPoint16_y">y</a>-axis value of <a href="SkIPoint_Reference#IPoint">IPoint</a>.

### Return Value

<a href="#SkIPoint16_fY">fY</a>

### Example

<div><fiddle-embed name="3662cedaf1e9924a401f794902da3b1f">

#### Example Output

~~~~
pt1.fY == pt1.y()
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIPoint16_x">x</a> <a href="#SkPoint_y">SkPoint::y()</a> <a href="#SkIPoint_y">SkIPoint::y()</a>

---

<a name="SkIPoint16_set"></a>
## set

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void set(int x, int y)
</pre>

Sets <a href="#SkIPoint16_fX">fX</a> to <a href="#SkIPoint16_x">x</a> and <a href="#SkIPoint16_fY">fY</a> to <a href="#SkIPoint16_y">y</a>.

### Parameters

<table>  <tr>    <td><a name="SkIPoint16_set_x"> <code><strong>x </strong></code> </a></td> <td>
new value for <a href="#SkIPoint16_fX">fX</a></td>
  </tr>  <tr>    <td><a name="SkIPoint16_set_y"> <code><strong>y </strong></code> </a></td> <td>
new value for <a href="#SkIPoint16_fY">fY</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="abff78d3f2d97b1284ccb13d0c56b6c8">

#### Example Output

~~~~
pt1.fX == pt2.fX
pt1.fY == pt2.fY
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkIPoint16_Make">Make</a> <a href="#SkPoint_set">SkPoint::set</a>

---

