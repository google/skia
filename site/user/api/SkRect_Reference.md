SkRect Reference
===


<a name='SkRect'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
struct <a href='SkRect_Reference#SkRect'>SkRect</a> {

    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRect_fLeft'>fLeft</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRect_fTop'>fTop</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRect_fRight'>fRight</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRect_fBottom'>fBottom</a>;

    static constexpr <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_MakeEmpty'>MakeEmpty</a>();
    static constexpr <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_MakeWH'>MakeWH</a>(<a href='undocumented#SkScalar'>SkScalar</a> w, <a href='undocumented#SkScalar'>SkScalar</a> h);
    static <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_MakeIWH'>MakeIWH</a>(int w, int h);
    static constexpr <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_MakeSize'>MakeSize</a>(const <a href='undocumented#SkSize'>SkSize</a>& <a href='undocumented#Size'>size</a>);
    static constexpr <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_MakeLTRB'>MakeLTRB</a>(<a href='undocumented#SkScalar'>SkScalar</a> l, <a href='undocumented#SkScalar'>SkScalar</a> t, <a href='undocumented#SkScalar'>SkScalar</a> r,
                                     <a href='undocumented#SkScalar'>SkScalar</a> b);
    static constexpr <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_MakeXYWH'>MakeXYWH</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, <a href='undocumented#SkScalar'>SkScalar</a> w,
                                     <a href='undocumented#SkScalar'>SkScalar</a> h);
    static <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_Make'>Make</a>(const <a href='undocumented#SkISize'>SkISize</a>& <a href='undocumented#Size'>size</a>);
    static <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_Make'>Make</a>(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& irect);
    bool <a href='#SkRect_isEmpty'>isEmpty</a>() const;
    bool <a href='#SkRect_isSorted'>isSorted</a>() const;
    bool <a href='#SkRect_isFinite'>isFinite</a>() const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRect_x'>x()</a> const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRect_y'>y()</a> const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRect_left'>left()</a> const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRect_top'>top()</a> const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRect_right'>right()</a> const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRect_bottom'>bottom()</a> const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRect_width'>width()</a> const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRect_height'>height()</a> const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRect_centerX'>centerX</a>() const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRect_centerY'>centerY</a>() const;
    friend bool <a href='#SkRect_equal_operator'>operator==</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& a, const <a href='SkRect_Reference#SkRect'>SkRect</a>& b);
    friend bool <a href='#SkRect_notequal_operator'>operator!=</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& a, const <a href='SkRect_Reference#SkRect'>SkRect</a>& b);
    void <a href='#SkRect_toQuad'>toQuad</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPath_Reference#Quad'>quad</a>[4]) const;
    void <a href='#SkRect_setEmpty'>setEmpty</a>();
    void <a href='#SkRect_set'>set</a>(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& src);
    void <a href='#SkRect_set'>set</a>(<a href='undocumented#SkScalar'>SkScalar</a> left, <a href='undocumented#SkScalar'>SkScalar</a> top, <a href='undocumented#SkScalar'>SkScalar</a> right, <a href='undocumented#SkScalar'>SkScalar</a> bottom);
    void <a href='#SkRect_setLTRB'>setLTRB</a>(<a href='undocumented#SkScalar'>SkScalar</a> left, <a href='undocumented#SkScalar'>SkScalar</a> top, <a href='undocumented#SkScalar'>SkScalar</a> right, <a href='undocumented#SkScalar'>SkScalar</a> bottom);
    void <a href='#SkRect_iset'>iset</a>(int left, int top, int right, int bottom);
    void <a href='#SkRect_isetWH'>isetWH</a>(int width, int height);
    void <a href='#SkRect_set'>set</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> pts[], int count);
    void <a href='#SkRect_setBounds'>setBounds</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> pts[], int count);
    bool <a href='#SkRect_setBoundsCheck'>setBoundsCheck</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> pts[], int count);
    void <a href='#SkRect_setBoundsNoCheck'>setBoundsNoCheck</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> pts[], int count);
    void <a href='#SkRect_set'>set</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p0, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p1);
    void <a href='#SkRect_setXYWH'>setXYWH</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, <a href='undocumented#SkScalar'>SkScalar</a> width, <a href='undocumented#SkScalar'>SkScalar</a> height);
    void <a href='#SkRect_setWH'>setWH</a>(<a href='undocumented#SkScalar'>SkScalar</a> width, <a href='undocumented#SkScalar'>SkScalar</a> height);
    <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_makeOffset'>makeOffset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy) const;
    <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_makeInset'>makeInset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy) const;
    <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_makeOutset'>makeOutset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy) const;
    void <a href='#SkRect_offset'>offset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy);
    void <a href='#SkRect_offset'>offset</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& delta);
    void <a href='#SkRect_offsetTo'>offsetTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> newX, <a href='undocumented#SkScalar'>SkScalar</a> newY);
    void <a href='#SkRect_inset'>inset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy);
    void <a href='#SkRect_outset'>outset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy);
    bool <a href='#SkRect_intersect'>intersect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& r);
    bool <a href='#SkRect_intersect'>intersect</a>(<a href='undocumented#SkScalar'>SkScalar</a> left, <a href='undocumented#SkScalar'>SkScalar</a> top, <a href='undocumented#SkScalar'>SkScalar</a> right, <a href='undocumented#SkScalar'>SkScalar</a> bottom);
    bool <a href='#SkRect_intersect'>intersect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& a, const <a href='SkRect_Reference#SkRect'>SkRect</a>& b);
    bool <a href='#SkRect_intersects'>intersects</a>(<a href='undocumented#SkScalar'>SkScalar</a> left, <a href='undocumented#SkScalar'>SkScalar</a> top, <a href='undocumented#SkScalar'>SkScalar</a> right, <a href='undocumented#SkScalar'>SkScalar</a> bottom) const;
    bool <a href='#SkRect_intersects'>intersects</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& r) const;
    static bool <a href='#SkRect_Intersects'>Intersects</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& a, const <a href='SkRect_Reference#SkRect'>SkRect</a>& b);
    void <a href='#SkRect_join'>join</a>(<a href='undocumented#SkScalar'>SkScalar</a> left, <a href='undocumented#SkScalar'>SkScalar</a> top, <a href='undocumented#SkScalar'>SkScalar</a> right, <a href='undocumented#SkScalar'>SkScalar</a> bottom);
    void <a href='#SkRect_join'>join</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& r);
    void <a href='#SkRect_joinNonEmptyArg'>joinNonEmptyArg</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& r);
    void <a href='#SkRect_joinPossiblyEmptyRect'>joinPossiblyEmptyRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& r);
    bool <a href='#SkRect_contains'>contains</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y) const;
    bool <a href='#SkRect_contains'>contains</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& r) const;
    bool <a href='#SkRect_contains'>contains</a>(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& r) const;
    void <a href='#SkRect_round'>round</a>(<a href='SkIRect_Reference#SkIRect'>SkIRect</a>* dst) const;
    void <a href='#SkRect_roundOut'>roundOut</a>(<a href='SkIRect_Reference#SkIRect'>SkIRect</a>* dst) const;
    void <a href='#SkRect_roundOut'>roundOut</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* dst) const;
    void <a href='#SkRect_roundIn'>roundIn</a>(<a href='SkIRect_Reference#SkIRect'>SkIRect</a>* dst) const;
    <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkRect_round'>round()</a> const;
    <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkRect_roundOut'>roundOut</a>() const;
    void <a href='#SkRect_sort'>sort()</a>;
    <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_makeSorted'>makeSorted</a>() const;
    const <a href='undocumented#SkScalar'>SkScalar</a>* <a href='#SkRect_asScalars'>asScalars</a>() const;
    void <a href='#SkRect_dump'>dump</a>(bool asHex) const;
    void <a href='#SkRect_dump'>dump()</a> const;
    void <a href='#SkRect_dumpHex'>dumpHex</a>() const;
};

</pre>

<a href='SkRect_Reference#SkRect'>SkRect</a> holds four <a href='undocumented#SkScalar'>SkScalar</a> coordinates describing the upper and
lower bounds of a rectangle. <a href='SkRect_Reference#SkRect'>SkRect</a> may be created from outer bounds or
from position, width, and height. <a href='SkRect_Reference#SkRect'>SkRect</a> describes an area; if its right
is less than or equal to its left, or if its bottom is less than or equal to
its top, it is considered empty.

<a href='SkRect_Reference#SkRect'>SkRect</a> can be constructed from int values to avoid compiler warnings that
integer input cannot convert to <a href='undocumented#SkScalar'>SkScalar</a> without loss of precision.<table style='border-collapse: collapse; width: 62.5em'>

  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Member</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRect_fLeft'><code>fLeft</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May contain any value, including infinities and NaN. The smaller of the
horizontal values when sorted. When equal to or greater than <a href='#SkRect_fRight'>fRight</a>, <a href='SkRect_Reference#Rect'>Rect</a> is empty.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRect_fTop'><code>fTop</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May contain any value, including infinities and NaN. The smaller of the
vertical values when sorted. When equal to or greater than <a href='#SkRect_fBottom'>fBottom</a>, <a href='SkRect_Reference#Rect'>Rect</a> is empty.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRect_fRight'><code>fRight</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May contain any value, including infinities and NaN. The larger of the
horizontal values when sorted. When equal to or less than <a href='#SkRect_fLeft'>fLeft</a>, <a href='SkRect_Reference#Rect'>Rect</a> is empty.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRect_fBottom'><code>fBottom</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May contain any value, including infinities and NaN. The larger of the
vertical values when sorted. When equal to or less than <a href='#SkRect_fTop'>fTop</a>, <a href='SkRect_Reference#Rect'>Rect</a> is empty.
</td>
  </tr>
</table>

<a name='SkRect_MakeEmpty'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_MakeEmpty'>MakeEmpty</a>()
</pre>

Returns constructed <a href='SkRect_Reference#SkRect'>SkRect</a> set to (0, 0, 0, 0).
Many other rectangles are empty; if left is equal to or greater than right,
or if top is equal to or greater than bottom. Setting all members to zero
is a convenience, but does not designate a special empty rectangle.

### Return Value

bounds (0, 0, 0, 0)

### Example

<div><fiddle-embed name="2e262d0ac4b8ef51695e0525fc3ecdf6">

#### Example Output

~~~~
MakeEmpty isEmpty: true
offset rect isEmpty: true
inset rect isEmpty: true
outset rect isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_isEmpty'>isEmpty</a> <a href='#SkRect_setEmpty'>setEmpty</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_MakeEmpty'>MakeEmpty</a>

<a name='SkRect_MakeWH'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_MakeWH'>MakeWH</a>(<a href='undocumented#SkScalar'>SkScalar</a> w, <a href='undocumented#SkScalar'>SkScalar</a> h)
</pre>

Returns constructed <a href='SkRect_Reference#SkRect'>SkRect</a> set to <a href='undocumented#SkScalar'>SkScalar</a> values (0, 0, <a href='#SkRect_MakeWH_w'>w</a>, <a href='#SkRect_MakeWH_h'>h</a>). Does not
validate input; <a href='#SkRect_MakeWH_w'>w</a> or <a href='#SkRect_MakeWH_h'>h</a> may be negative.

Passing integer values may generate a compiler warning since <a href='SkRect_Reference#SkRect'>SkRect</a> cannot
represent 32-bit integers exactly. Use <a href='SkIRect_Reference#SkIRect'>SkIRect</a> for an exact integer rectangle.

### Parameters

<table>  <tr>    <td><a name='SkRect_MakeWH_w'><code><strong>w</strong></code></a></td>
    <td><a href='undocumented#SkScalar'>SkScalar</a> width of constructed <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
  <tr>    <td><a name='SkRect_MakeWH_h'><code><strong>h</strong></code></a></td>
    <td><a href='undocumented#SkScalar'>SkScalar</a> height of constructed <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
</table>

### Return Value

bounds (0, 0, <a href='#SkRect_MakeWH_w'>w</a>, <a href='#SkRect_MakeWH_h'>h</a>)

### Example

<div><fiddle-embed name="8009d30f431e01f8aea4808e9017d9bf">

#### Example Output

~~~~
all equal
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_MakeSize'>MakeSize</a> <a href='#SkRect_MakeXYWH'>MakeXYWH</a> <a href='#SkRect_MakeIWH'>MakeIWH</a> <a href='#SkRect_setWH'>setWH</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_MakeWH'>MakeWH</a>

<a name='SkRect_MakeIWH'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_MakeIWH'>MakeIWH</a>(int w, int h)
</pre>

Returns constructed <a href='SkRect_Reference#SkRect'>SkRect</a> set to integer values (0, 0, <a href='#SkRect_MakeIWH_w'>w</a>, <a href='#SkRect_MakeIWH_h'>h</a>). Does not validate
input; <a href='#SkRect_MakeIWH_w'>w</a> or <a href='#SkRect_MakeIWH_h'>h</a> may be negative.

Use to avoid a compiler warning that input may lose precision when stored.
Use <a href='SkIRect_Reference#SkIRect'>SkIRect</a> for an exact integer rectangle.

### Parameters

<table>  <tr>    <td><a name='SkRect_MakeIWH_w'><code><strong>w</strong></code></a></td>
    <td>integer width of constructed <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
  <tr>    <td><a name='SkRect_MakeIWH_h'><code><strong>h</strong></code></a></td>
    <td>integer height of constructed <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
</table>

### Return Value

bounds (0, 0, <a href='#SkRect_MakeIWH_w'>w</a>, <a href='#SkRect_MakeIWH_h'>h</a>)

### Example

<div><fiddle-embed name="faa660ac19eaddc3f3eab57a0bddfdcb">

#### Example Output

~~~~
i_rect width: 25 f_rect width:25
i_rect width: 125000111 f_rect width:125000112
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_MakeXYWH'>MakeXYWH</a> <a href='#SkRect_MakeWH'>MakeWH</a> <a href='#SkRect_isetWH'>isetWH</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_MakeWH'>MakeWH</a>

<a name='SkRect_MakeSize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_MakeSize'>MakeSize</a>(const <a href='undocumented#SkSize'>SkSize</a>& <a href='undocumented#Size'>size</a>)
</pre>

Returns constructed <a href='SkRect_Reference#SkRect'>SkRect</a> set to (0, 0, <a href='#SkRect_MakeSize_size'>size</a>.<a href='#SkSize_width'>width()</a>, <a href='#SkRect_MakeSize_size'>size</a>.<a href='#SkSize_height'>height()</a>). Does not
validate input; <a href='#SkRect_MakeSize_size'>size</a>.<a href='#SkSize_width'>width()</a> or <a href='#SkRect_MakeSize_size'>size</a>.<a href='#SkSize_height'>height()</a> may be negative.

### Parameters

<table>  <tr>    <td><a name='SkRect_MakeSize_size'><code><strong>size</strong></code></a></td>
    <td><a href='undocumented#SkScalar'>SkScalar</a> values for <a href='SkRect_Reference#SkRect'>SkRect</a> width and height</td>
  </tr>
</table>

### Return Value

bounds (0, 0, <a href='#SkRect_MakeSize_size'>size</a>.<a href='#SkSize_width'>width()</a>, <a href='#SkRect_MakeSize_size'>size</a>.<a href='#SkSize_height'>height()</a>)

### Example

<div><fiddle-embed name="ab2c1a55016c8de9172b77fdf69e00a2">

#### Example Output

~~~~
rect width: 25.5  height: 35.5
floor width: 25  height: 35
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_MakeWH'>MakeWH</a> <a href='#SkRect_MakeXYWH'>MakeXYWH</a> <a href='#SkRect_MakeIWH'>MakeIWH</a> <a href='#SkRect_setWH'>setWH</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_MakeWH'>MakeWH</a>

<a name='SkRect_MakeLTRB'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_MakeLTRB'>MakeLTRB</a>(<a href='undocumented#SkScalar'>SkScalar</a> l, <a href='undocumented#SkScalar'>SkScalar</a> t, <a href='undocumented#SkScalar'>SkScalar</a> r, <a href='undocumented#SkScalar'>SkScalar</a> b)
</pre>

Returns constructed <a href='SkRect_Reference#SkRect'>SkRect</a> set to (<a href='#SkRect_MakeLTRB_l'>l</a>, <a href='#SkRect_MakeLTRB_t'>t</a>, <a href='#SkRect_MakeLTRB_r'>r</a>, <a href='#SkRect_MakeLTRB_b'>b</a>). Does not sort input; <a href='SkRect_Reference#SkRect'>SkRect</a> may
result in <a href='#SkRect_fLeft'>fLeft</a> greater than <a href='#SkRect_fRight'>fRight</a>, or <a href='#SkRect_fTop'>fTop</a> greater than <a href='#SkRect_fBottom'>fBottom</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_MakeLTRB_l'><code><strong>l</strong></code></a></td>
    <td><a href='undocumented#SkScalar'>SkScalar</a> stored in <a href='#SkRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkRect_MakeLTRB_t'><code><strong>t</strong></code></a></td>
    <td><a href='undocumented#SkScalar'>SkScalar</a> stored in <a href='#SkRect_fTop'>fTop</a></td>
  </tr>
  <tr>    <td><a name='SkRect_MakeLTRB_r'><code><strong>r</strong></code></a></td>
    <td><a href='undocumented#SkScalar'>SkScalar</a> stored in <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_MakeLTRB_b'><code><strong>b</strong></code></a></td>
    <td><a href='undocumented#SkScalar'>SkScalar</a> stored in <a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

bounds (<a href='#SkRect_MakeLTRB_l'>l</a>, <a href='#SkRect_MakeLTRB_t'>t</a>, <a href='#SkRect_MakeLTRB_r'>r</a>, <a href='#SkRect_MakeLTRB_b'>b</a>)

### Example

<div><fiddle-embed name="158b8dd9d02d65a5ae5ab7d1595a5b4c">

#### Example Output

~~~~
rect: 5, 35, 15, 25  isEmpty: true
rect: 5, 25, 15, 35  isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_MakeXYWH'>MakeXYWH</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_MakeLTRB'>MakeLTRB</a>

<a name='SkRect_MakeXYWH'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_MakeXYWH'>MakeXYWH</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, <a href='undocumented#SkScalar'>SkScalar</a> w, <a href='undocumented#SkScalar'>SkScalar</a> h)
</pre>

Returns constructed <a href='SkRect_Reference#Rect'>Rect</a> set to <code>(<a href='#SkRect_MakeXYWH_x'>x</a>, <a href='#SkRect_MakeXYWH_y'>y</a>, <a href='#SkRect_MakeXYWH_x'>x</a> + <a href='#SkRect_MakeXYWH_w'>w</a>, <a href='#SkRect_MakeXYWH_y'>y</a> + <a href='#SkRect_MakeXYWH_h'>h</a>)</code>.
Does not validate input; <a href='#SkRect_MakeXYWH_w'>w</a> or <a href='#SkRect_MakeXYWH_h'>h</a> may be negative.

### Parameters

<table>  <tr>    <td><a name='SkRect_MakeXYWH_x'><code><strong>x</strong></code></a></td>
    <td>stored in <a href='#SkRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkRect_MakeXYWH_y'><code><strong>y</strong></code></a></td>
    <td>stored in <a href='#SkRect_fTop'>fTop</a></td>
  </tr>
  <tr>    <td><a name='SkRect_MakeXYWH_w'><code><strong>w</strong></code></a></td>
    <td>added to <a href='#SkRect_MakeXYWH_x'>x</a> and stored in <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_MakeXYWH_h'><code><strong>h</strong></code></a></td>
    <td>added to <a href='#SkRect_MakeXYWH_y'>y</a> and stored in <a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

bounds at (<a href='#SkRect_MakeXYWH_x'>x</a>, <a href='#SkRect_MakeXYWH_y'>y</a>) with width <a href='#SkRect_MakeXYWH_w'>w</a> and height <a href='#SkRect_MakeXYWH_h'>h</a>

### Example

<div><fiddle-embed name="38e464dba13be11ac21e210fbf3b5afc">

#### Example Output

~~~~
rect: 5, 35, -10, 60  isEmpty: true
rect: -10, 35, 5, 60  isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_MakeLTRB'>MakeLTRB</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_MakeXYWH'>MakeXYWH</a>

<a name='SkRect_Make'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_Make'>Make</a>(const <a href='undocumented#SkISize'>SkISize</a>& <a href='undocumented#Size'>size</a>)
</pre>

Returns constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a> set to (0, 0, <a href='#SkRect_Make_size'>size</a>.<a href='#SkISize_width'>width()</a>, <a href='#SkRect_Make_size'>size</a>.<a href='#SkISize_height'>height()</a>).
Does not validate input; <a href='#SkRect_Make_size'>size</a>.<a href='#SkISize_width'>width()</a> or <a href='#SkRect_Make_size'>size</a>.<a href='#SkISize_height'>height()</a> may be negative.

### Parameters

<table>  <tr>    <td><a name='SkRect_Make_size'><code><strong>size</strong></code></a></td>
    <td>integer values for <a href='SkRect_Reference#SkRect'>SkRect</a> width and height</td>
  </tr>
</table>

### Return Value

bounds (0, 0, <a href='#SkRect_Make_size'>size</a>.<a href='#SkISize_width'>width()</a>, <a href='#SkRect_Make_size'>size</a>.<a href='#SkISize_height'>height()</a>)

### Example

<div><fiddle-embed name="e866f5e4f6ac52e89acadf48e54ac8e0">

#### Example Output

~~~~
rect1 == rect2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_MakeWH'>MakeWH</a> <a href='#SkRect_MakeXYWH'>MakeXYWH</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_MakeIWH'>MakeIWH</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_MakeSize'>MakeSize</a>

<a name='SkRect_Make_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_Make'>Make</a>(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& irect)
</pre>

Returns constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a> set to <a href='#SkRect_Make_2_irect'>irect</a>, promoting integers to <a href='undocumented#Scalar'>scalar</a>.
Does not validate input; <a href='#SkRect_fLeft'>fLeft</a> may be greater than <a href='#SkRect_fRight'>fRight</a>, <a href='#SkRect_fTop'>fTop</a> may be greater
than <a href='#SkRect_fBottom'>fBottom</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_Make_2_irect'><code><strong>irect</strong></code></a></td>
    <td>integer unsorted bounds</td>
  </tr>
</table>

### Return Value

<a href='#SkRect_Make_2_irect'>irect</a> members converted to <a href='undocumented#SkScalar'>SkScalar</a>

### Example

<div><fiddle-embed name="dd801faa1e60a0fe9e0657674461e063"></fiddle-embed></div>

### See Also

<a href='#SkRect_MakeLTRB'>MakeLTRB</a>

<a name='Property'></a>

<a name='SkRect_isEmpty'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRect_isEmpty'>isEmpty</a>()const
</pre>

Returns true if <a href='#SkRect_fLeft'>fLeft</a> is equal to or greater than <a href='#SkRect_fRight'>fRight</a>, or if <a href='#SkRect_fTop'>fTop</a> is equal
to or greater than <a href='#SkRect_fBottom'>fBottom</a>. Call <a href='#SkRect_sort'>sort()</a> to reverse rectangles with negative
<a href='#SkRect_width'>width()</a> or <a href='#SkRect_height'>height()</a>.

### Return Value

true if <a href='#SkRect_width'>width()</a> or <a href='#SkRect_height'>height()</a> are zero or negative

### Example

<div><fiddle-embed name="1d7b924d6ca2a6aef09684a8a632439c">

#### Example Output

~~~~
rect: {20, 40, 10, 50} is empty
sorted: {10, 40, 20, 50} is not empty
rect: {20, 40, 20, 50} is empty
sorted: {20, 40, 20, 50} is empty
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_MakeEmpty'>MakeEmpty</a> <a href='#SkRect_sort'>sort</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_isEmpty'>isEmpty</a>

<a name='SkRect_isSorted'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRect_isSorted'>isSorted</a>()const
</pre>

Returns true if <a href='#SkRect_fLeft'>fLeft</a> is equal to or less than <a href='#SkRect_fRight'>fRight</a>, or if <a href='#SkRect_fTop'>fTop</a> is equal
to or less than <a href='#SkRect_fBottom'>fBottom</a>. Call <a href='#SkRect_sort'>sort()</a> to reverse rectangles with negative
<a href='#SkRect_width'>width()</a> or <a href='#SkRect_height'>height()</a>.

### Return Value

true if <a href='#SkRect_width'>width()</a> or <a href='#SkRect_height'>height()</a> are zero or positive

### Example

<div><fiddle-embed name="c7065a83b220a96f903dbbb65906fe7b">

#### Example Output

~~~~
rect: {20, 40, 10, 50} is not sorted
sorted: {10, 40, 20, 50} is sorted
rect: {20, 40, 20, 50} is sorted
sorted: {20, 40, 20, 50} is sorted
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_sort'>sort</a> <a href='#SkRect_makeSorted'>makeSorted</a> <a href='#SkRect_isEmpty'>isEmpty</a>

<a name='SkRect_isFinite'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRect_isFinite'>isFinite</a>()const
</pre>

Returns true if all values in the rectangle are finite: <a href='undocumented#SK_ScalarMin'>SK_ScalarMin</a> or larger,
and <a href='undocumented#SK_ScalarMax'>SK_ScalarMax</a> or smaller.

### Return Value

true if no member is infinite or NaN

### Example

<div><fiddle-embed name="443fe5f8296d4cdb19cc9862a9cf77a4">

#### Example Output

~~~~
largest is finite: true
large width inf
widest is finite: false
~~~~

</fiddle-embed></div>

### See Also

<a href='undocumented#SkScalarIsFinite'>SkScalarIsFinite</a> <a href='undocumented#SkScalarIsNaN'>SkScalarIsNaN</a>

<a name='SkRect_x'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkRect_x'>x()</a>const
</pre>

Returns left edge of <a href='SkRect_Reference#SkRect'>SkRect</a>, if sorted. Call <a href='#SkRect_isSorted'>isSorted</a>() to see if <a href='SkRect_Reference#SkRect'>SkRect</a> is valid.
Call <a href='#SkRect_sort'>sort()</a> to reverse <a href='#SkRect_fLeft'>fLeft</a> and <a href='#SkRect_fRight'>fRight</a> if needed.

### Return Value

<a href='#SkRect_fLeft'>fLeft</a>

### Example

<div><fiddle-embed name="23c77a35ac54a439a2989f840aa5cb99">

#### Example Output

~~~~
unsorted.fLeft: 15 unsorted.x(): 15
sorted.fLeft: 10 sorted.x(): 10
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_left'>left()</a> <a href='#SkRect_y'>y()</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_x'>x()</a>

<a name='SkRect_y'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkRect_y'>y()</a>const
</pre>

Returns top edge of <a href='SkRect_Reference#SkRect'>SkRect</a>, if sorted. Call <a href='#SkRect_isEmpty'>isEmpty</a>() to see if <a href='SkRect_Reference#SkRect'>SkRect</a> may be invalid,
and <a href='#SkRect_sort'>sort()</a> to reverse <a href='#SkRect_fTop'>fTop</a> and <a href='#SkRect_fBottom'>fBottom</a> if needed.

### Return Value

<a href='#SkRect_fTop'>fTop</a>

### Example

<div><fiddle-embed name="c653d9017983d2a047b1fee6a481d82b">

#### Example Output

~~~~
unsorted.fTop: 25 unsorted.y(): 25
sorted.fTop: 5 sorted.y(): 5
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_top'>top()</a> <a href='#SkRect_x'>x()</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_y'>y()</a>

<a name='SkRect_left'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkRect_left'>left()</a>const
</pre>

Returns left edge of <a href='SkRect_Reference#SkRect'>SkRect</a>, if sorted. Call <a href='#SkRect_isSorted'>isSorted</a>() to see if <a href='SkRect_Reference#SkRect'>SkRect</a> is valid.
Call <a href='#SkRect_sort'>sort()</a> to reverse <a href='#SkRect_fLeft'>fLeft</a> and <a href='#SkRect_fRight'>fRight</a> if needed.

### Return Value

<a href='#SkRect_fLeft'>fLeft</a>

### Example

<div><fiddle-embed name="900dc96c3549795a87036d6458c4fde6">

#### Example Output

~~~~
unsorted.fLeft: 15 unsorted.left(): 15
sorted.fLeft: 10 sorted.left(): 10
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_x'>x()</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_left'>left()</a>

<a name='SkRect_top'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkRect_top'>top()</a>const
</pre>

Returns top edge of <a href='SkRect_Reference#SkRect'>SkRect</a>, if sorted. Call <a href='#SkRect_isEmpty'>isEmpty</a>() to see if <a href='SkRect_Reference#SkRect'>SkRect</a> may be invalid,
and <a href='#SkRect_sort'>sort()</a> to reverse <a href='#SkRect_fTop'>fTop</a> and <a href='#SkRect_fBottom'>fBottom</a> if needed.

### Return Value

<a href='#SkRect_fTop'>fTop</a>

### Example

<div><fiddle-embed name="3cfc24b011aef1ca8ccb57c05711620c">

#### Example Output

~~~~
unsorted.fTop: 25 unsorted.top(): 25
sorted.fTop: 5 sorted.top(): 5
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_y'>y()</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_top'>top()</a>

<a name='SkRect_right'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkRect_right'>right()</a>const
</pre>

Returns right edge of <a href='SkRect_Reference#SkRect'>SkRect</a>, if sorted. Call <a href='#SkRect_isSorted'>isSorted</a>() to see if <a href='SkRect_Reference#SkRect'>SkRect</a> is valid.
Call <a href='#SkRect_sort'>sort()</a> to reverse <a href='#SkRect_fLeft'>fLeft</a> and <a href='#SkRect_fRight'>fRight</a> if needed.

### Return Value

<a href='#SkRect_fRight'>fRight</a>

### Example

<div><fiddle-embed name="ca3de7e5e292b3ad3633b1c39a31d3ab">

#### Example Output

~~~~
unsorted.fRight: 10 unsorted.right(): 10
sorted.fRight: 15 sorted.right(): 15
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_fRight'>fRight</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_right'>right()</a>

<a name='SkRect_bottom'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkRect_bottom'>bottom()</a>const
</pre>

Returns bottom edge of <a href='SkRect_Reference#SkRect'>SkRect</a>, if sorted. Call <a href='#SkRect_isEmpty'>isEmpty</a>() to see if <a href='SkRect_Reference#SkRect'>SkRect</a> may be invalid,
and <a href='#SkRect_sort'>sort()</a> to reverse <a href='#SkRect_fTop'>fTop</a> and <a href='#SkRect_fBottom'>fBottom</a> if needed.

### Return Value

<a href='#SkRect_fBottom'>fBottom</a>

### Example

<div><fiddle-embed name="a98993a66616ae406d8bdc54adfb1411">

#### Example Output

~~~~
unsorted.fBottom: 5 unsorted.bottom(): 5
sorted.fBottom: 25 sorted.bottom(): 25
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_fBottom'>fBottom</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_bottom'>bottom()</a>

<a name='SkRect_width'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkRect_width'>width()</a>const
</pre>

Returns span on the x-axis. This does not check if <a href='SkRect_Reference#SkRect'>SkRect</a> is sorted, or if
result fits in 32-bit float; result may be negative or infinity.

### Return Value

<a href='#SkRect_fRight'>fRight</a> minus <a href='#SkRect_fLeft'>fLeft</a>

### Example

<div><fiddle-embed name="11f8f0efe6291019fee0ac17844f6c1a"><div>Compare with <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_width'>width()</a> example.
</div>

#### Example Output

~~~~
unsorted width: -5
large width: 4294967296
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_height'>height()</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_width'>width()</a>

<a name='SkRect_height'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkRect_height'>height()</a>const
</pre>

Returns span on the y-axis. This does not check if <a href='SkRect_Reference#SkRect'>SkRect</a> is sorted, or if
result fits in 32-bit float; result may be negative or infinity.

### Return Value

<a href='#SkRect_fBottom'>fBottom</a> minus <a href='#SkRect_fTop'>fTop</a>

### Example

<div><fiddle-embed name="39429e45f05240218ecd511443ab3e44"><div>Compare with <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_height'>height()</a> example.
</div>

#### Example Output

~~~~
unsorted height: -5
large height: 4294967296
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_width'>width()</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_height'>height()</a>

<a name='SkRect_centerX'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkRect_centerX'>centerX</a>()const
</pre>

Returns average of left edge and right edge. Result does not change if <a href='SkRect_Reference#SkRect'>SkRect</a>
is sorted. Result may overflow to infinity if <a href='SkRect_Reference#SkRect'>SkRect</a> is far from the origin.

### Return Value

midpoint on x-axis

### Example

<div><fiddle-embed name="d8439ba8d23a424fa032fb97147fd2d2">

#### Example Output

~~~~
left:  20 right:  41 centerX: 30.5
left:  20 right:  41 centerX: 30.5
left: -20 right: -41 centerX: -30.5
left: -41 right: -20 centerX: -30.5
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_centerY'>centerY</a>

<a name='SkRect_centerY'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkRect_centerY'>centerY</a>()const
</pre>

Returns average of top edge and bottom edge. Result does not change if <a href='SkRect_Reference#SkRect'>SkRect</a>
is sorted.

### Return Value

midpoint on y-axis

### Example

<div><fiddle-embed name="ebeeafafeb8fe39d5ffc9115b02c2340">

#### Example Output

~~~~
left: 2e+38 right: 3e+38 centerX: 2.5e+38 safe mid x: 2.5e+38
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_centerX'>centerX</a>

<a name='Operators'></a>

<a name='SkRect_equal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRect_equal_operator'>operator==</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& a, const <a href='SkRect_Reference#SkRect'>SkRect</a>& b)
</pre>

Returns true if all members in <a href='#SkRect_equal_operator_a'>a</a>: <a href='#SkRect_fLeft'>fLeft</a>, <a href='#SkRect_fTop'>fTop</a>, <a href='#SkRect_fRight'>fRight</a>, and <a href='#SkRect_fBottom'>fBottom</a>; are
equal to the corresponding members in <a href='#SkRect_equal_operator_b'>b</a>.

<a href='#SkRect_equal_operator_a'>a</a> and <a href='#SkRect_equal_operator_b'>b</a> are not equal if either contain NaN. <a href='#SkRect_equal_operator_a'>a</a> and <a href='#SkRect_equal_operator_b'>b</a> are equal if members
contain zeroes with different signs.

### Parameters

<table>  <tr>    <td><a name='SkRect_equal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> to compare</td>
  </tr>
  <tr>    <td><a name='SkRect_equal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> to compare</td>
  </tr>
</table>

### Return Value

true if members are equal

### Example

<div><fiddle-embed name="c6c5b40cad7c3a839fdf576b380391a6">

#### Example Output

~~~~
tests are equal
{0, 0, 2, 2} == {-0, -0, 2, 2} and are numerically equal
{0, 0, 2, 2} == {-0, -0, 2, 2} and are numerically equal
{0, 0, 2, 2} == {-0, -0, 2, 2} and are numerically equal
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_notequal_operator'>operator!=</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkRect_equal_operator_a'>a</a>, const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkRect_equal_operator_b'>b</a>)

<a name='SkRect_notequal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRect_notequal_operator'>operator!=</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& a, const <a href='SkRect_Reference#SkRect'>SkRect</a>& b)
</pre>

Returns true if any in <a href='#SkRect_notequal_operator_a'>a</a>: <a href='#SkRect_fLeft'>fLeft</a>, <a href='#SkRect_fTop'>fTop</a>, <a href='#SkRect_fRight'>fRight</a>, and <a href='#SkRect_fBottom'>fBottom</a>; does not
equal the corresponding members in <a href='#SkRect_notequal_operator_b'>b</a>.

<a href='#SkRect_notequal_operator_a'>a</a> and <a href='#SkRect_notequal_operator_b'>b</a> are not equal if either contain NaN. <a href='#SkRect_notequal_operator_a'>a</a> and <a href='#SkRect_notequal_operator_b'>b</a> are equal if members
contain zeroes with different signs.

### Parameters

<table>  <tr>    <td><a name='SkRect_notequal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> to compare</td>
  </tr>
  <tr>    <td><a name='SkRect_notequal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> to compare</td>
  </tr>
</table>

### Return Value

true if members are not equal

### Example

<div><fiddle-embed name="286072f8c27ff15be9eb945fa38dc9f7">

#### Example Output

~~~~
test with NaN is not equal to itself
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_equal_operator'>operator==</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkRect_notequal_operator_a'>a</a>, const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkRect_notequal_operator_b'>b</a>)

<a name='As_Points'></a>

<a name='SkRect_toQuad'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRect_toQuad'>toQuad</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPath_Reference#Quad'>quad</a>[4])const
</pre>

Returns four <a href='SkPoint_Reference#Point'>points</a> in <a href='#SkRect_toQuad_quad'>quad</a> that enclose <a href='SkRect_Reference#SkRect'>SkRect</a> ordered as: top-left, top-right,
bottom-right, bottom-left.

### Parameters

<table>  <tr>    <td><a name='SkRect_toQuad_quad'><code><strong>quad</strong></code></a></td>
    <td>storage for corners of <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="59a6e7d202ac17ab80ec21b233e51f59">

#### Example Output

~~~~
rect: {1, 2, 3, 4}
corners: {1, 2} {3, 2} {3, 4} {1, 4}
~~~~

</fiddle-embed></div>

### See Also

<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_addRect'>addRect</a>

<a name='SkRect_setBounds'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRect_setBounds'>setBounds</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> pts[], int count)
</pre>

Sets to bounds of  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> with <a href='#SkRect_setBounds_count'>count</a> entries. If <a href='#SkRect_setBounds_count'>count</a> is zero or smaller,
or if  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> contains an infinity or NaN, sets to (0, 0, 0, 0).

Result is either empty or sorted: <a href='#SkRect_fLeft'>fLeft</a> is less than or equal to <a href='#SkRect_fRight'>fRight</a>, and
<a href='#SkRect_fTop'>fTop</a> is less than or equal to <a href='#SkRect_fBottom'>fBottom</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_setBounds_pts'><code><strong>pts</strong></code></a></td>
    <td><a href='SkPath_Reference#Point_Array'>SkPoint array</a></td>
  </tr>
  <tr>    <td><a name='SkRect_setBounds_count'><code><strong>count</strong></code></a></td>
    <td>entries in array</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="cf0da15f48aa54fd1889e7f913601710">

#### Example Output

~~~~
count: 0 rect: 0, 0, 0, 0
added:   3, 4 count: 1 rect: 3, 4, 3, 4
added:   1, 2 count: 2 rect: 1, 2, 3, 4
added:   5, 6 count: 3 rect: 1, 2, 5, 6
added: nan, 8 count: 4 rect: 0, 0, 0, 0
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_set'>set</a> <a href='#SkRect_setBoundsCheck'>setBoundsCheck</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_addPoly'>addPoly</a>

<a name='SkRect_setBoundsCheck'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRect_setBoundsCheck'>setBoundsCheck</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> pts[], int count)
</pre>

Sets to bounds of  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> with <a href='#SkRect_setBoundsCheck_count'>count</a> entries. Returns false if <a href='#SkRect_setBoundsCheck_count'>count</a> is
zero or smaller, or if  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> contains an infinity or NaN; in these cases
sets <a href='SkRect_Reference#SkRect'>SkRect</a> to (0, 0, 0, 0).

Result is either empty or sorted: <a href='#SkRect_fLeft'>fLeft</a> is less than or equal to <a href='#SkRect_fRight'>fRight</a>, and
<a href='#SkRect_fTop'>fTop</a> is less than or equal to <a href='#SkRect_fBottom'>fBottom</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_setBoundsCheck_pts'><code><strong>pts</strong></code></a></td>
    <td><a href='SkPath_Reference#Point_Array'>SkPoint array</a></td>
  </tr>
  <tr>    <td><a name='SkRect_setBoundsCheck_count'><code><strong>count</strong></code></a></td>
    <td>entries in array</td>
  </tr>
</table>

### Return Value

true if all <a href='SkPoint_Reference#SkPoint'>SkPoint</a> values are finite

### Example

<div><fiddle-embed name="83d879b92683b15f9daaf0c9e71c5b35">

#### Example Output

~~~~
count: 0 rect: 0, 0, 0, 0 success: true
added:   3, 4 count: 1 rect: 3, 4, 3, 4 success: true
added:   1, 2 count: 2 rect: 1, 2, 3, 4 success: true
added:   5, 6 count: 3 rect: 1, 2, 5, 6 success: true
added: nan, 8 count: 4 rect: 0, 0, 0, 0 success: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_set'>set</a> <a href='#SkRect_setBounds'>setBounds</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_addPoly'>addPoly</a>

<a name='Set'></a>

<a name='SkRect_setBoundsNoCheck'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRect_setBoundsNoCheck'>setBoundsNoCheck</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> pts[], int count)
</pre>

Sets to bounds of <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkRect_setBoundsNoCheck_pts'>pts</a> array with <a href='#SkRect_setBoundsNoCheck_count'>count</a> entries. If any <a href='SkPoint_Reference#SkPoint'>SkPoint</a> in <a href='#SkRect_setBoundsNoCheck_pts'>pts</a>
contains infinity or NaN, all <a href='SkRect_Reference#SkRect'>SkRect</a> dimensions are set to NaN.

### Parameters

<table>  <tr>    <td><a name='SkRect_setBoundsNoCheck_pts'><code><strong>pts</strong></code></a></td>
    <td><a href='SkPath_Reference#Point_Array'>SkPoint array</a></td>
  </tr>
  <tr>    <td><a name='SkRect_setBoundsNoCheck_count'><code><strong>count</strong></code></a></td>
    <td>entries in array</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="be10cb1411dbcf7e38e0198e8a9b8b0e"></fiddle-embed></div>

### See Also

<a href='#SkRect_setBoundsCheck'>setBoundsCheck</a>

<a name='SkRect_setEmpty'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRect_setEmpty'>setEmpty</a>()
</pre>

Sets <a href='SkRect_Reference#SkRect'>SkRect</a> to (0, 0, 0, 0).

Many other rectangles are empty; if left is equal to or greater than right,
or if top is equal to or greater than bottom. Setting all members to zero
is a convenience, but does not designate a special empty rectangle.

### Example

<div><fiddle-embed name="2cf67542d45ef5d7a7efb673b651ff54">

#### Example Output

~~~~
rect: {3, 4, 1, 2} is empty
rect: {0, 0, 0, 0} is empty
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_MakeEmpty'>MakeEmpty</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_setEmpty'>setEmpty</a>

<a name='SkRect_set'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void set(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& src)
</pre>

Sets <a href='SkRect_Reference#SkRect'>SkRect</a> to <a href='#SkRect_set_src'>src</a>, promoting <a href='#SkRect_set_src'>src</a> members from integer to <a href='undocumented#Scalar'>scalar</a>.
Very large values in <a href='#SkRect_set_src'>src</a> may lose precision.

### Parameters

<table>  <tr>    <td><a name='SkRect_set_src'><code><strong>src</strong></code></a></td>
    <td>integer <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a10ad8d97062bc3f40942f47e5108917">

#### Example Output

~~~~
i_rect: {3, 4, 1, 2}
f_rect: {3, 4, 1, 2}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_setLTRB'>setLTRB</a> <a href='undocumented#SkIntToScalar'>SkIntToScalar</a>

<a name='SkRect_set_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void set(<a href='undocumented#SkScalar'>SkScalar</a> left, <a href='undocumented#SkScalar'>SkScalar</a> top, <a href='undocumented#SkScalar'>SkScalar</a> right, <a href='undocumented#SkScalar'>SkScalar</a> bottom)
</pre>

Sets <a href='SkRect_Reference#SkRect'>SkRect</a> to (<a href='#SkRect_set_2_left'>left</a>, <a href='#SkRect_set_2_top'>top</a>, <a href='#SkRect_set_2_right'>right</a>, <a href='#SkRect_set_2_bottom'>bottom</a>).
<a href='#SkRect_set_2_left'>left</a> and <a href='#SkRect_set_2_right'>right</a> are not sorted; <a href='#SkRect_set_2_left'>left</a> is not necessarily less than <a href='#SkRect_set_2_right'>right</a>.
<a href='#SkRect_set_2_top'>top</a> and <a href='#SkRect_set_2_bottom'>bottom</a> are not sorted; <a href='#SkRect_set_2_top'>top</a> is not necessarily less than <a href='#SkRect_set_2_bottom'>bottom</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_set_2_left'><code><strong>left</strong></code></a></td>
    <td>stored in <a href='#SkRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkRect_set_2_top'><code><strong>top</strong></code></a></td>
    <td>stored in <a href='#SkRect_fTop'>fTop</a></td>
  </tr>
  <tr>    <td><a name='SkRect_set_2_right'><code><strong>right</strong></code></a></td>
    <td>stored in <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_set_2_bottom'><code><strong>bottom</strong></code></a></td>
    <td>stored in <a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="9b29ea460d69b4d47323fd9e3e17721e">

#### Example Output

~~~~
rect1: {3, 4, 1, 2}
rect2: {3, 4, 1, 2}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_setLTRB'>setLTRB</a> <a href='#SkRect_setXYWH'>setXYWH</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_set'>set</a>

<a name='SkRect_setLTRB'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRect_setLTRB'>setLTRB</a>(<a href='undocumented#SkScalar'>SkScalar</a> left, <a href='undocumented#SkScalar'>SkScalar</a> top, <a href='undocumented#SkScalar'>SkScalar</a> right, <a href='undocumented#SkScalar'>SkScalar</a> bottom)
</pre>

Sets <a href='SkRect_Reference#SkRect'>SkRect</a> to (<a href='#SkRect_setLTRB_left'>left</a>, <a href='#SkRect_setLTRB_top'>top</a>, <a href='#SkRect_setLTRB_right'>right</a>, <a href='#SkRect_setLTRB_bottom'>bottom</a>).
<a href='#SkRect_setLTRB_left'>left</a> and <a href='#SkRect_setLTRB_right'>right</a> are not sorted; <a href='#SkRect_setLTRB_left'>left</a> is not necessarily less than <a href='#SkRect_setLTRB_right'>right</a>.
<a href='#SkRect_setLTRB_top'>top</a> and <a href='#SkRect_setLTRB_bottom'>bottom</a> are not sorted; <a href='#SkRect_setLTRB_top'>top</a> is not necessarily less than <a href='#SkRect_setLTRB_bottom'>bottom</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_setLTRB_left'><code><strong>left</strong></code></a></td>
    <td>stored in <a href='#SkRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkRect_setLTRB_top'><code><strong>top</strong></code></a></td>
    <td>stored in <a href='#SkRect_fTop'>fTop</a></td>
  </tr>
  <tr>    <td><a name='SkRect_setLTRB_right'><code><strong>right</strong></code></a></td>
    <td>stored in <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_setLTRB_bottom'><code><strong>bottom</strong></code></a></td>
    <td>stored in <a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="70692838793454c8e045d6eaf7edcbff">

#### Example Output

~~~~
rect1: {3, 4, 1, 2}
rect2: {3, 4, 1, 2}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_set'>set</a> <a href='#SkRect_setXYWH'>setXYWH</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_set'>set</a>

<a name='SkRect_set_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void set(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> pts[], int count)
</pre>

Sets to bounds of  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> with <a href='#SkRect_set_3_count'>count</a> entries. If <a href='#SkRect_set_3_count'>count</a> is zero or smaller,
or if  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> contains an infinity or NaN, sets <a href='SkRect_Reference#SkRect'>SkRect</a> to (0, 0, 0, 0).

Result is either empty or sorted: <a href='#SkRect_fLeft'>fLeft</a> is less than or equal to <a href='#SkRect_fRight'>fRight</a>, and
<a href='#SkRect_fTop'>fTop</a> is less than or equal to <a href='#SkRect_fBottom'>fBottom</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_set_3_pts'><code><strong>pts</strong></code></a></td>
    <td><a href='SkPath_Reference#Point_Array'>SkPoint array</a></td>
  </tr>
  <tr>    <td><a name='SkRect_set_3_count'><code><strong>count</strong></code></a></td>
    <td>entries in array</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="94295fa5197e21256171b99b4023dd48">

#### Example Output

~~~~
count: 0 rect: 0, 0, 0, 0
added:   3, 4 count: 1 rect: 3, 4, 3, 4
added:   1, 2 count: 2 rect: 1, 2, 3, 4
added:   5, 6 count: 3 rect: 1, 2, 5, 6
added: nan, 8 count: 4 rect: 0, 0, 0, 0
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_setBounds'>setBounds</a> <a href='#SkRect_setBoundsCheck'>setBoundsCheck</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_addPoly'>addPoly</a>

<a name='SkRect_set_4'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void set(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p0, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p1)
</pre>

Sets bounds to the smallest <a href='SkRect_Reference#SkRect'>SkRect</a> enclosing <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkRect_set_4_p0'>p0</a> and <a href='#SkRect_set_4_p1'>p1</a>. The result is
sorted and may be empty. Does not check to see if values are finite.

### Parameters

<table>  <tr>    <td><a name='SkRect_set_4_p0'><code><strong>p0</strong></code></a></td>
    <td>corner to include</td>
  </tr>
  <tr>    <td><a name='SkRect_set_4_p1'><code><strong>p1</strong></code></a></td>
    <td>corner to include</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ee72450381f768f3869153cdbeccdc3e"><div><a href='#SkRect_set_4_p0'>p0</a> and <a href='#SkRect_set_4_p1'>p1</a> may be swapped and have the same effect unless one contains NaN.
</div></fiddle-embed></div>

### See Also

<a href='#SkRect_setBounds'>setBounds</a> <a href='#SkRect_setBoundsCheck'>setBoundsCheck</a>

<a name='SkRect_setXYWH'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRect_setXYWH'>setXYWH</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, <a href='undocumented#SkScalar'>SkScalar</a> width, <a href='undocumented#SkScalar'>SkScalar</a> height)
</pre>

Sets <a href='SkRect_Reference#Rect'>Rect</a> to <code>(<a href='#SkRect_setXYWH_x'>x</a>, <a href='#SkRect_setXYWH_y'>y</a>, <a href='#SkRect_setXYWH_x'>x</a> + <a href='#SkRect_setXYWH_width'>width</a>, <a href='#SkRect_setXYWH_y'>y</a> + <a href='#SkRect_setXYWH_height'>height</a>)</code>.
Does not validate input; <a href='#SkRect_setXYWH_width'>width</a> or <a href='#SkRect_setXYWH_height'>height</a> may be negative.

### Parameters

<table>  <tr>    <td><a name='SkRect_setXYWH_x'><code><strong>x</strong></code></a></td>
    <td>stored in <a href='#SkRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkRect_setXYWH_y'><code><strong>y</strong></code></a></td>
    <td>stored in <a href='#SkRect_fTop'>fTop</a></td>
  </tr>
  <tr>    <td><a name='SkRect_setXYWH_width'><code><strong>width</strong></code></a></td>
    <td>added to <a href='#SkRect_setXYWH_x'>x</a> and stored in <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_setXYWH_height'><code><strong>height</strong></code></a></td>
    <td>added to <a href='#SkRect_setXYWH_y'>y</a> and stored in <a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="373cce4c61b9da0384b735b838765163">

#### Example Output

~~~~
rect: 5, 35, -10, 60  isEmpty: true
rect: -10, 35, 5, 60  isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_MakeXYWH'>MakeXYWH</a> <a href='#SkRect_setLTRB'>setLTRB</a> <a href='#SkRect_set'>set</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_setXYWH'>setXYWH</a>

<a name='SkRect_setWH'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRect_setWH'>setWH</a>(<a href='undocumented#SkScalar'>SkScalar</a> width, <a href='undocumented#SkScalar'>SkScalar</a> height)
</pre>

Sets <a href='SkRect_Reference#SkRect'>SkRect</a> to (0, 0, <a href='#SkRect_setWH_width'>width</a>, <a href='#SkRect_setWH_height'>height</a>). Does not validate input;
<a href='#SkRect_setWH_width'>width</a> or <a href='#SkRect_setWH_height'>height</a> may be negative.

### Parameters

<table>  <tr>    <td><a name='SkRect_setWH_width'><code><strong>width</strong></code></a></td>
    <td>stored in <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_setWH_height'><code><strong>height</strong></code></a></td>
    <td>stored in <a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="9cb5fee17802fa49341f3707bdf5d235">

#### Example Output

~~~~
rect: 0, 0, -15, 25  isEmpty: true
rect: -15, 0, 0, 25  isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_MakeWH'>MakeWH</a> <a href='#SkRect_setXYWH'>setXYWH</a> <a href='#SkRect_isetWH'>isetWH</a>

<a name='From_Integers'></a>

<a name='SkRect_iset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRect_iset'>iset</a>(int left, int top, int right, int bottom)
</pre>

Sets <a href='SkRect_Reference#SkRect'>SkRect</a> to (<a href='#SkRect_iset_left'>left</a>, <a href='#SkRect_iset_top'>top</a>, <a href='#SkRect_iset_right'>right</a>, <a href='#SkRect_iset_bottom'>bottom</a>).
All parameters are promoted from integer to <a href='undocumented#Scalar'>scalar</a>.
<a href='#SkRect_iset_left'>left</a> and <a href='#SkRect_iset_right'>right</a> are not sorted; <a href='#SkRect_iset_left'>left</a> is not necessarily less than <a href='#SkRect_iset_right'>right</a>.
<a href='#SkRect_iset_top'>top</a> and <a href='#SkRect_iset_bottom'>bottom</a> are not sorted; <a href='#SkRect_iset_top'>top</a> is not necessarily less than <a href='#SkRect_iset_bottom'>bottom</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_iset_left'><code><strong>left</strong></code></a></td>
    <td>promoted to <a href='undocumented#SkScalar'>SkScalar</a> and stored in <a href='#SkRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkRect_iset_top'><code><strong>top</strong></code></a></td>
    <td>promoted to <a href='undocumented#SkScalar'>SkScalar</a> and stored in <a href='#SkRect_fTop'>fTop</a></td>
  </tr>
  <tr>    <td><a name='SkRect_iset_right'><code><strong>right</strong></code></a></td>
    <td>promoted to <a href='undocumented#SkScalar'>SkScalar</a> and stored in <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_iset_bottom'><code><strong>bottom</strong></code></a></td>
    <td>promoted to <a href='undocumented#SkScalar'>SkScalar</a> and stored in <a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="18532f1aa90b76364fb8d7ea072f1892">

#### Example Output

~~~~
rect1: {3, 4, 1, 2}
rect2: {3, 4, 1, 2}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_set'>set</a> <a href='#SkRect_setLTRB'>setLTRB</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_set'>set</a> <a href='undocumented#SkIntToScalar'>SkIntToScalar</a>

<a name='SkRect_isetWH'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRect_isetWH'>isetWH</a>(int width, int height)
</pre>

Sets <a href='SkRect_Reference#SkRect'>SkRect</a> to (0, 0, <a href='#SkRect_isetWH_width'>width</a>, <a href='#SkRect_isetWH_height'>height</a>).
<a href='#SkRect_isetWH_width'>width</a> and <a href='#SkRect_isetWH_height'>height</a> may be zero or negative. <a href='#SkRect_isetWH_width'>width</a> and <a href='#SkRect_isetWH_height'>height</a> are promoted from
integer to <a href='undocumented#SkScalar'>SkScalar</a>, large values may lose precision.

### Parameters

<table>  <tr>    <td><a name='SkRect_isetWH_width'><code><strong>width</strong></code></a></td>
    <td>promoted to <a href='undocumented#SkScalar'>SkScalar</a> and stored in <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_isetWH_height'><code><strong>height</strong></code></a></td>
    <td>promoted to <a href='undocumented#SkScalar'>SkScalar</a> and stored in <a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ee6000080fc7123214ea404018cf9176">

#### Example Output

~~~~
rect1: {0, 0, 1, 2}
rect2: {0, 0, 1, 2}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_MakeWH'>MakeWH</a> <a href='#SkRect_MakeXYWH'>MakeXYWH</a> <a href='#SkRect_iset'>iset()</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>:<a href='#SkRect_MakeWH'>MakeWH</a>

<a name='Inset_Outset_Offset'></a>

<a name='SkRect_makeOffset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_makeOffset'>makeOffset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy)const
</pre>

Returns <a href='SkRect_Reference#SkRect'>SkRect</a> offset by (<a href='#SkRect_makeOffset_dx'>dx</a>, <a href='#SkRect_makeOffset_dy'>dy</a>).

If <a href='#SkRect_makeOffset_dx'>dx</a> is negative, <a href='SkRect_Reference#SkRect'>SkRect</a> returned is moved to the left.
If <a href='#SkRect_makeOffset_dx'>dx</a> is positive, <a href='SkRect_Reference#SkRect'>SkRect</a> returned is moved to the right.
If <a href='#SkRect_makeOffset_dy'>dy</a> is negative, <a href='SkRect_Reference#SkRect'>SkRect</a> returned is moved upward.
If <a href='#SkRect_makeOffset_dy'>dy</a> is positive, <a href='SkRect_Reference#SkRect'>SkRect</a> returned is moved downward.

### Parameters

<table>  <tr>    <td><a name='SkRect_makeOffset_dx'><code><strong>dx</strong></code></a></td>
    <td>added to <a href='#SkRect_fLeft'>fLeft</a> and <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_makeOffset_dy'><code><strong>dy</strong></code></a></td>
    <td>added to <a href='#SkRect_fTop'>fTop</a> and <a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

<a href='SkRect_Reference#SkRect'>SkRect</a> offset on axes, with original width and height

### Example

<div><fiddle-embed name="98841ab0a932f99cccd8e6a34d94ba05">

#### Example Output

~~~~
rect: 10, 50, 20, 60  isEmpty: false
rect: 25, 82, 35, 92  isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_offset'>offset()</a> <a href='#SkRect_makeInset'>makeInset</a> <a href='#SkRect_makeOutset'>makeOutset</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_makeOffset'>makeOffset</a>

<a name='SkRect_makeInset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_makeInset'>makeInset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy)const
</pre>

Returns <a href='SkRect_Reference#SkRect'>SkRect</a>, inset by (<a href='#SkRect_makeInset_dx'>dx</a>, <a href='#SkRect_makeInset_dy'>dy</a>).

If <a href='#SkRect_makeInset_dx'>dx</a> is negative, <a href='SkRect_Reference#SkRect'>SkRect</a> returned is wider.
If <a href='#SkRect_makeInset_dx'>dx</a> is positive, <a href='SkRect_Reference#SkRect'>SkRect</a> returned is narrower.
If <a href='#SkRect_makeInset_dy'>dy</a> is negative, <a href='SkRect_Reference#SkRect'>SkRect</a> returned is taller.
If <a href='#SkRect_makeInset_dy'>dy</a> is positive, <a href='SkRect_Reference#SkRect'>SkRect</a> returned is shorter.

### Parameters

<table>  <tr>    <td><a name='SkRect_makeInset_dx'><code><strong>dx</strong></code></a></td>
    <td>added to <a href='#SkRect_fLeft'>fLeft</a> and subtracted from <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_makeInset_dy'><code><strong>dy</strong></code></a></td>
    <td>added to <a href='#SkRect_fTop'>fTop</a> and subtracted from <a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

<a href='SkRect_Reference#SkRect'>SkRect</a> inset symmetrically left and right, top and bottom

### Example

<div><fiddle-embed name="b8d32ab2f7ea3d4d5fb5a4ea2156f1c5">

#### Example Output

~~~~
rect: 10, 50, 20, 60  isEmpty: false
rect: 25, 82, 5, 28  isEmpty: true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_inset'>inset()</a> <a href='#SkRect_makeOffset'>makeOffset</a> <a href='#SkRect_makeOutset'>makeOutset</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_makeInset'>makeInset</a>

<a name='SkRect_makeOutset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_makeOutset'>makeOutset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy)const
</pre>

Returns <a href='SkRect_Reference#SkRect'>SkRect</a>, outset by (<a href='#SkRect_makeOutset_dx'>dx</a>, <a href='#SkRect_makeOutset_dy'>dy</a>).

If <a href='#SkRect_makeOutset_dx'>dx</a> is negative, <a href='SkRect_Reference#SkRect'>SkRect</a> returned is narrower.
If <a href='#SkRect_makeOutset_dx'>dx</a> is positive, <a href='SkRect_Reference#SkRect'>SkRect</a> returned is wider.
If <a href='#SkRect_makeOutset_dy'>dy</a> is negative, <a href='SkRect_Reference#SkRect'>SkRect</a> returned is shorter.
If <a href='#SkRect_makeOutset_dy'>dy</a> is positive, <a href='SkRect_Reference#SkRect'>SkRect</a> returned is taller.

### Parameters

<table>  <tr>    <td><a name='SkRect_makeOutset_dx'><code><strong>dx</strong></code></a></td>
    <td>subtracted to <a href='#SkRect_fLeft'>fLeft</a> and added from <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_makeOutset_dy'><code><strong>dy</strong></code></a></td>
    <td>subtracted to <a href='#SkRect_fTop'>fTop</a> and added from <a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

<a href='SkRect_Reference#SkRect'>SkRect</a> outset symmetrically left and right, top and bottom

### Example

<div><fiddle-embed name="87176fc60914cbca9c6a20998a033c24">

#### Example Output

~~~~
rect: 10, 50, 20, 60  isEmpty: false
rect: -5, 18, 35, 92  isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_outset'>outset()</a> <a href='#SkRect_makeOffset'>makeOffset</a> <a href='#SkRect_makeInset'>makeInset</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_makeOutset'>makeOutset</a>

<a name='SkRect_offset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void offset(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy)
</pre>

Offsets <a href='SkRect_Reference#SkRect'>SkRect</a> by adding <a href='#SkRect_offset_dx'>dx</a> to <a href='#SkRect_fLeft'>fLeft</a>, <a href='#SkRect_fRight'>fRight</a>; and by adding <a href='#SkRect_offset_dy'>dy</a> to <a href='#SkRect_fTop'>fTop</a>, <a href='#SkRect_fBottom'>fBottom</a>.

If <a href='#SkRect_offset_dx'>dx</a> is negative, moves <a href='SkRect_Reference#SkRect'>SkRect</a> to the left.
If <a href='#SkRect_offset_dx'>dx</a> is positive, moves <a href='SkRect_Reference#SkRect'>SkRect</a> to the right.
If <a href='#SkRect_offset_dy'>dy</a> is negative, moves <a href='SkRect_Reference#SkRect'>SkRect</a> upward.
If <a href='#SkRect_offset_dy'>dy</a> is positive, moves <a href='SkRect_Reference#SkRect'>SkRect</a> downward.

### Parameters

<table>  <tr>    <td><a name='SkRect_offset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#SkRect_fLeft'>fLeft</a> and <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_offset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#SkRect_fTop'>fTop</a> and <a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="04eb33f0fd376f2942ca5f1c7f6cbcfc">

#### Example Output

~~~~
rect: 15, 27, 55, 86
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_offsetTo'>offsetTo</a> <a href='#SkRect_makeOffset'>makeOffset</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_offset'>offset</a>

<a name='SkRect_offset_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void offset(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& delta)
</pre>

Offsets <a href='SkRect_Reference#SkRect'>SkRect</a> by adding <a href='#SkRect_offset_2_delta'>delta</a>.<a href='#SkPoint_fX'>fX</a> to <a href='#SkRect_fLeft'>fLeft</a>, <a href='#SkRect_fRight'>fRight</a>; and by adding <a href='#SkRect_offset_2_delta'>delta</a>.<a href='#SkPoint_fY'>fY</a> to
<a href='#SkRect_fTop'>fTop</a>, <a href='#SkRect_fBottom'>fBottom</a>.

If <a href='#SkRect_offset_2_delta'>delta</a>.<a href='#SkPoint_fX'>fX</a> is negative, moves <a href='SkRect_Reference#SkRect'>SkRect</a> to the left.
If <a href='#SkRect_offset_2_delta'>delta</a>.<a href='#SkPoint_fX'>fX</a> is positive, moves <a href='SkRect_Reference#SkRect'>SkRect</a> to the right.
If <a href='#SkRect_offset_2_delta'>delta</a>.<a href='#SkPoint_fY'>fY</a> is negative, moves <a href='SkRect_Reference#SkRect'>SkRect</a> upward.
If <a href='#SkRect_offset_2_delta'>delta</a>.<a href='#SkPoint_fY'>fY</a> is positive, moves <a href='SkRect_Reference#SkRect'>SkRect</a> downward.

### Parameters

<table>  <tr>    <td><a name='SkRect_offset_2_delta'><code><strong>delta</strong></code></a></td>
    <td>added to <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="b24cf65561c98c1858a06c39f10fb797">

#### Example Output

~~~~
rect: 15, 27, 55, 86
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_offsetTo'>offsetTo</a> <a href='#SkRect_makeOffset'>makeOffset</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_offset'>offset</a>

<a name='SkRect_offsetTo'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRect_offsetTo'>offsetTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> newX, <a href='undocumented#SkScalar'>SkScalar</a> newY)
</pre>

Offsets <a href='SkRect_Reference#SkRect'>SkRect</a> so that <a href='#SkRect_fLeft'>fLeft</a> equals <a href='#SkRect_offsetTo_newX'>newX</a>, and <a href='#SkRect_fTop'>fTop</a> equals <a href='#SkRect_offsetTo_newY'>newY</a>. width and height
are unchanged.

### Parameters

<table>  <tr>    <td><a name='SkRect_offsetTo_newX'><code><strong>newX</strong></code></a></td>
    <td>stored in <a href='#SkRect_fLeft'>fLeft</a>, preserving <a href='#SkRect_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkRect_offsetTo_newY'><code><strong>newY</strong></code></a></td>
    <td>stored in <a href='#SkRect_fTop'>fTop</a>, preserving <a href='#SkRect_height'>height()</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="bedb04b7b3e1af3e8039f9cffe66989e">

#### Example Output

~~~~
rect: 15, 27, 55, 86
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_offset'>offset</a> <a href='#SkRect_makeOffset'>makeOffset</a> <a href='#SkRect_setXYWH'>setXYWH</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_offsetTo'>offsetTo</a>

<a name='SkRect_inset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void inset(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy)
</pre>

Insets <a href='SkRect_Reference#SkRect'>SkRect</a> by (<a href='#SkRect_inset_dx'>dx</a>, <a href='#SkRect_inset_dy'>dy</a>).

If <a href='#SkRect_inset_dx'>dx</a> is positive, makes <a href='SkRect_Reference#SkRect'>SkRect</a> narrower.
If <a href='#SkRect_inset_dx'>dx</a> is negative, makes <a href='SkRect_Reference#SkRect'>SkRect</a> wider.
If <a href='#SkRect_inset_dy'>dy</a> is positive, makes <a href='SkRect_Reference#SkRect'>SkRect</a> shorter.
If <a href='#SkRect_inset_dy'>dy</a> is negative, makes <a href='SkRect_Reference#SkRect'>SkRect</a> taller.

### Parameters

<table>  <tr>    <td><a name='SkRect_inset_dx'><code><strong>dx</strong></code></a></td>
    <td>added to <a href='#SkRect_fLeft'>fLeft</a> and subtracted from <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_inset_dy'><code><strong>dy</strong></code></a></td>
    <td>added to <a href='#SkRect_fTop'>fTop</a> and subtracted from <a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="dae21340941dc6e4d048816dfd9f204c">

#### Example Output

~~~~
rect: 15, 27, 45, 60
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_outset'>outset</a> <a href='#SkRect_makeInset'>makeInset</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_inset'>inset</a>

<a name='SkRect_outset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void outset(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy)
</pre>

Outsets <a href='SkRect_Reference#SkRect'>SkRect</a> by (<a href='#SkRect_outset_dx'>dx</a>, <a href='#SkRect_outset_dy'>dy</a>).

If <a href='#SkRect_outset_dx'>dx</a> is positive, makes <a href='SkRect_Reference#SkRect'>SkRect</a> wider.
If <a href='#SkRect_outset_dx'>dx</a> is negative, makes <a href='SkRect_Reference#SkRect'>SkRect</a> narrower.
If <a href='#SkRect_outset_dy'>dy</a> is positive, makes <a href='SkRect_Reference#SkRect'>SkRect</a> taller.
If <a href='#SkRect_outset_dy'>dy</a> is negative, makes <a href='SkRect_Reference#SkRect'>SkRect</a> shorter.

### Parameters

<table>  <tr>    <td><a name='SkRect_outset_dx'><code><strong>dx</strong></code></a></td>
    <td>subtracted to <a href='#SkRect_fLeft'>fLeft</a> and added from <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_outset_dy'><code><strong>dy</strong></code></a></td>
    <td>subtracted to <a href='#SkRect_fTop'>fTop</a> and added from <a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="861f873ba660af8c8bf8b0b83d829cf4">

#### Example Output

~~~~
rect: 5, 1, 55, 86
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_inset'>inset</a> <a href='#SkRect_makeOutset'>makeOutset</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_outset'>outset</a>

<a name='Intersection'></a>

<a href='SkRect_Reference#Rect'>Rects</a> intersect when they enclose a common area. To intersect, each of the pair
must describe area; <a href='#SkRect_fLeft'>fLeft</a> is less than <a href='#SkRect_fRight'>fRight</a>, and <a href='#SkRect_fTop'>fTop</a> is less than <a href='#SkRect_fBottom'>fBottom</a>;
<a href='#SkRect_isEmpty'>isEmpty</a>() returns false. The intersection of <a href='SkRect_Reference#Rect'>Rect</a> pair can be described by:
<code>(<a href='undocumented#max()'>max</a>(a.<a href='#SkRect_fLeft'>fLeft</a>, b.<a href='#SkRect_fLeft'>fLeft</a>), <a href='undocumented#max()'>max</a>(a.<a href='#SkRect_fTop'>fTop</a>, b.<a href='#SkRect_fTop'>fTop</a>),
<a href='undocumented#min()'>min</a>(a.<a href='#SkRect_fRight'>fRight</a>, b.<a href='#SkRect_fRight'>fRight</a>), <a href='undocumented#min()'>min</a>(a.<a href='#SkRect_fBottom'>fBottom</a>, b.<a href='#SkRect_fBottom'>fBottom</a>))</code>.

The intersection is only meaningful if the resulting <a href='SkRect_Reference#Rect'>Rect</a> is not empty and
describes an area: <a href='#SkRect_fLeft'>fLeft</a> is less than <a href='#SkRect_fRight'>fRight</a>, and <a href='#SkRect_fTop'>fTop</a> is less than <a href='#SkRect_fBottom'>fBottom</a>.

<a name='SkRect_contains'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool contains(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y)const
</pre>

Returns true if: <a href='#SkRect_fLeft'>fLeft</a> <= <a href='#SkRect_contains_x'>x</a> < <a href='#SkRect_fRight'>fRight</a> && <a href='#SkRect_fTop'>fTop</a> <= <a href='#SkRect_contains_y'>y</a> < <a href='#SkRect_fBottom'>fBottom</a>.
Returns false if <a href='SkRect_Reference#SkRect'>SkRect</a> is empty.

### Parameters

<table>  <tr>    <td><a name='SkRect_contains_x'><code><strong>x</strong></code></a></td>
    <td>test <a href='SkPoint_Reference#SkPoint'>SkPoint</a> x-coordinate</td>
  </tr>
  <tr>    <td><a name='SkRect_contains_y'><code><strong>y</strong></code></a></td>
    <td>test <a href='SkPoint_Reference#SkPoint'>SkPoint</a> y-coordinate</td>
  </tr>
</table>

### Return Value

true if (<a href='#SkRect_contains_x'>x</a>, <a href='#SkRect_contains_y'>y</a>) is inside <a href='SkRect_Reference#SkRect'>SkRect</a>

### Example

<div><fiddle-embed name="85be528a78945a6dc4f7dccb80a80746">

#### Example Output

~~~~
rect: (30, 50, 40, 60) contains (30, 50)
rect: (30, 50, 40, 60) does not contain (39, 49)
rect: (30, 50, 40, 60) does not contain (29, 59)
~~~~

</fiddle-embed></div>

### See Also

<a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_contains'>contains</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>::<a href='#SkRRect_contains'>contains</a>

<a name='SkRect_contains_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool contains(const <a href='SkRect_Reference#SkRect'>SkRect</a>& r)const
</pre>

Returns true if <a href='SkRect_Reference#SkRect'>SkRect</a> contains <a href='#SkRect_contains_2_r'>r</a>.
Returns false if <a href='SkRect_Reference#SkRect'>SkRect</a> is empty or <a href='#SkRect_contains_2_r'>r</a> is empty.

<a href='SkRect_Reference#SkRect'>SkRect</a> contains <a href='#SkRect_contains_2_r'>r</a> when <a href='SkRect_Reference#SkRect'>SkRect</a> area completely includes <a href='#SkRect_contains_2_r'>r</a> area.

### Parameters

<table>  <tr>    <td><a name='SkRect_contains_2_r'><code><strong>r</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> contained</td>
  </tr>
</table>

### Return Value

true if all sides of <a href='SkRect_Reference#SkRect'>SkRect</a> are outside <a href='#SkRect_contains_2_r'>r</a>

### Example

<div><fiddle-embed name="92f9e6aa5bb76791139a24cf7d8df99e">

#### Example Output

~~~~
rect: (30, 50, 40, 60) contains (30, 50, 31, 51)
rect: (30, 50, 40, 60) does not contain (39, 49, 40, 50)
rect: (30, 50, 40, 60) does not contain (29, 59, 30, 60)
~~~~

</fiddle-embed></div>

### See Also

<a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_contains'>contains</a>

<a name='SkRect_contains_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool contains(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& r)const
</pre>

Returns true if <a href='SkRect_Reference#SkRect'>SkRect</a> contains <a href='#SkRect_contains_3_r'>r</a>.
Returns false if <a href='SkRect_Reference#SkRect'>SkRect</a> is empty or <a href='#SkRect_contains_3_r'>r</a> is empty.

<a href='SkRect_Reference#SkRect'>SkRect</a> contains <a href='#SkRect_contains_3_r'>r</a> when <a href='SkRect_Reference#SkRect'>SkRect</a> area completely includes <a href='#SkRect_contains_3_r'>r</a> area.

### Parameters

<table>  <tr>    <td><a name='SkRect_contains_3_r'><code><strong>r</strong></code></a></td>
    <td><a href='SkIRect_Reference#SkIRect'>SkIRect</a> contained</td>
  </tr>
</table>

### Return Value

true if all sides of <a href='SkRect_Reference#SkRect'>SkRect</a> are outside <a href='#SkRect_contains_3_r'>r</a>

### Example

<div><fiddle-embed name="dd58b699551dd44026a2c6386be27d88">

#### Example Output

~~~~
rect: (30, 50, 40, 60) contains (30, 50, 31, 51)
rect: (30, 50, 40, 60) does not contain (39, 49, 40, 50)
rect: (30, 50, 40, 60) does not contain (29, 59, 30, 60)
~~~~

</fiddle-embed></div>

### See Also

<a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_contains'>contains</a>

<a name='SkRect_intersect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool intersect(const <a href='SkRect_Reference#SkRect'>SkRect</a>& r)
</pre>

Returns true if <a href='SkRect_Reference#SkRect'>SkRect</a> intersects <a href='#SkRect_intersect_r'>r</a>, and sets <a href='SkRect_Reference#SkRect'>SkRect</a> to intersection.
Returns false if <a href='SkRect_Reference#SkRect'>SkRect</a> does not intersect <a href='#SkRect_intersect_r'>r</a>, and leaves <a href='SkRect_Reference#SkRect'>SkRect</a> unchanged.

Returns false if either <a href='#SkRect_intersect_r'>r</a> or <a href='SkRect_Reference#SkRect'>SkRect</a> is empty, leaving <a href='SkRect_Reference#SkRect'>SkRect</a> unchanged.

### Parameters

<table>  <tr>    <td><a name='SkRect_intersect_r'><code><strong>r</strong></code></a></td>
    <td>limit of result</td>
  </tr>
</table>

### Return Value

true if <a href='#SkRect_intersect_r'>r</a> and <a href='SkRect_Reference#SkRect'>SkRect</a> have area in common

### Example

<div><fiddle-embed name="5d0b12e0ef6f1c181dddded4274230ca"><div>Two <a href='undocumented#SkDebugf'>SkDebugf</a> calls are required. If the calls are combined, their arguments
may not be evaluated in left to right order: the printed intersection may
be before or after the call to intersect.
</div>

#### Example Output

~~~~
intersection: 30, 60, 50, 80
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_intersects'>intersects</a> <a href='#SkRect_Intersects'>Intersects</a> <a href='#SkRect_join'>join</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_intersect'>intersect</a>

<a name='SkRect_intersect_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool intersect(<a href='undocumented#SkScalar'>SkScalar</a> left, <a href='undocumented#SkScalar'>SkScalar</a> top, <a href='undocumented#SkScalar'>SkScalar</a> right, <a href='undocumented#SkScalar'>SkScalar</a> bottom)
</pre>

Constructs <a href='SkRect_Reference#SkRect'>SkRect</a> to intersect from (<a href='#SkRect_intersect_2_left'>left</a>, <a href='#SkRect_intersect_2_top'>top</a>, <a href='#SkRect_intersect_2_right'>right</a>, <a href='#SkRect_intersect_2_bottom'>bottom</a>). Does not sort
construction.

Returns true if <a href='SkRect_Reference#SkRect'>SkRect</a> intersects construction, and sets <a href='SkRect_Reference#SkRect'>SkRect</a> to intersection.
Returns false if <a href='SkRect_Reference#SkRect'>SkRect</a> does not intersect construction, and leaves <a href='SkRect_Reference#SkRect'>SkRect</a> unchanged.

Returns false if either construction or <a href='SkRect_Reference#SkRect'>SkRect</a> is empty, leaving <a href='SkRect_Reference#SkRect'>SkRect</a> unchanged.

### Parameters

<table>  <tr>    <td><a name='SkRect_intersect_2_left'><code><strong>left</strong></code></a></td>
    <td>x-axis minimum of constructed <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
  <tr>    <td><a name='SkRect_intersect_2_top'><code><strong>top</strong></code></a></td>
    <td>y-axis minimum of constructed <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
  <tr>    <td><a name='SkRect_intersect_2_right'><code><strong>right</strong></code></a></td>
    <td>x-axis maximum of constructed <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
  <tr>    <td><a name='SkRect_intersect_2_bottom'><code><strong>bottom</strong></code></a></td>
    <td>y-axis maximum of constructed <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
</table>

### Return Value

true if construction and <a href='SkRect_Reference#SkRect'>SkRect</a> have area in common

### Example

<div><fiddle-embed name="5002f65a72def2787086a33131933e70"><div>Two <a href='undocumented#SkDebugf'>SkDebugf</a> calls are required. If the calls are combined, their arguments
may not be evaluated in <a href='#SkRect_intersect_2_left'>left</a> to <a href='#SkRect_intersect_2_right'>right</a> order: the printed intersection may
be before or after the call to intersect.
</div>

#### Example Output

~~~~
intersection: 30, 60, 50, 80
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_intersects'>intersects</a> <a href='#SkRect_Intersects'>Intersects</a> <a href='#SkRect_join'>join</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_intersect'>intersect</a>

<a name='SkRect_intersect_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool intersect(const <a href='SkRect_Reference#SkRect'>SkRect</a>& a, const <a href='SkRect_Reference#SkRect'>SkRect</a>& b)
</pre>

Returns true if <a href='#SkRect_intersect_3_a'>a</a> intersects <a href='#SkRect_intersect_3_b'>b</a>, and sets <a href='SkRect_Reference#SkRect'>SkRect</a> to intersection.
Returns false if <a href='#SkRect_intersect_3_a'>a</a> does not intersect <a href='#SkRect_intersect_3_b'>b</a>, and leaves <a href='SkRect_Reference#SkRect'>SkRect</a> unchanged.

Returns false if either <a href='#SkRect_intersect_3_a'>a</a> or <a href='#SkRect_intersect_3_b'>b</a> is empty, leaving <a href='SkRect_Reference#SkRect'>SkRect</a> unchanged.

### Parameters

<table>  <tr>    <td><a name='SkRect_intersect_3_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> to intersect</td>
  </tr>
  <tr>    <td><a name='SkRect_intersect_3_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> to intersect</td>
  </tr>
</table>

### Return Value

true if <a href='#SkRect_intersect_3_a'>a</a> and <a href='#SkRect_intersect_3_b'>b</a> have area in common

### Example

<div><fiddle-embed name="d610437a65dd3e952719efe605cbd0c7">

#### Example Output

~~~~
intersection: 30, 60, 50, 80
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_intersects'>intersects</a> <a href='#SkRect_Intersects'>Intersects</a> <a href='#SkRect_join'>join</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_intersect'>intersect</a>

<a name='SkRect_intersects'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool intersects(<a href='undocumented#SkScalar'>SkScalar</a> left, <a href='undocumented#SkScalar'>SkScalar</a> top, <a href='undocumented#SkScalar'>SkScalar</a> right, <a href='undocumented#SkScalar'>SkScalar</a> bottom)const
</pre>

Constructs <a href='SkRect_Reference#SkRect'>SkRect</a> to intersect from (<a href='#SkRect_intersects_left'>left</a>, <a href='#SkRect_intersects_top'>top</a>, <a href='#SkRect_intersects_right'>right</a>, <a href='#SkRect_intersects_bottom'>bottom</a>). Does not sort
construction.

Returns true if <a href='SkRect_Reference#SkRect'>SkRect</a> intersects construction.
Returns false if either construction or <a href='SkRect_Reference#SkRect'>SkRect</a> is empty, or do not intersect.

### Parameters

<table>  <tr>    <td><a name='SkRect_intersects_left'><code><strong>left</strong></code></a></td>
    <td>x-axis minimum of constructed <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
  <tr>    <td><a name='SkRect_intersects_top'><code><strong>top</strong></code></a></td>
    <td>y-axis minimum of constructed <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
  <tr>    <td><a name='SkRect_intersects_right'><code><strong>right</strong></code></a></td>
    <td>x-axis maximum of constructed <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
  <tr>    <td><a name='SkRect_intersects_bottom'><code><strong>bottom</strong></code></a></td>
    <td>y-axis maximum of constructed <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
</table>

### Return Value

true if construction and <a href='SkRect_Reference#SkRect'>SkRect</a> have area in common

### Example

<div><fiddle-embed name="7145dc17ebce4f54e892102f6c98e811">

#### Example Output

~~~~
intersection
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_intersect'>intersect</a> <a href='#SkRect_Intersects'>Intersects</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_Intersects'>Intersects</a>

<a name='SkRect_intersects_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool intersects(const <a href='SkRect_Reference#SkRect'>SkRect</a>& r)const
</pre>

Returns true if <a href='SkRect_Reference#SkRect'>SkRect</a> intersects <a href='#SkRect_intersects_2_r'>r</a>.
Returns false if either <a href='#SkRect_intersects_2_r'>r</a> or <a href='SkRect_Reference#SkRect'>SkRect</a> is empty, or do not intersect.

### Parameters

<table>  <tr>    <td><a name='SkRect_intersects_2_r'><code><strong>r</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> to intersect</td>
  </tr>
</table>

### Return Value

true if <a href='#SkRect_intersects_2_r'>r</a> and <a href='SkRect_Reference#SkRect'>SkRect</a> have area in common

### Example

<div><fiddle-embed name="ca37b4231b21eb8296cb19ba9e0c781b">

#### Example Output

~~~~
intersection
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_intersect'>intersect</a> <a href='#SkRect_Intersects'>Intersects</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_Intersects'>Intersects</a>

<a name='SkRect_Intersects'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkRect_Intersects'>Intersects</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& a, const <a href='SkRect_Reference#SkRect'>SkRect</a>& b)
</pre>

Returns true if <a href='#SkRect_Intersects_a'>a</a> intersects <a href='#SkRect_Intersects_b'>b</a>.
Returns false if either <a href='#SkRect_Intersects_a'>a</a> or <a href='#SkRect_Intersects_b'>b</a> is empty, or do not intersect.

### Parameters

<table>  <tr>    <td><a name='SkRect_Intersects_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> to intersect</td>
  </tr>
  <tr>    <td><a name='SkRect_Intersects_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> to intersect</td>
  </tr>
</table>

### Return Value

true if <a href='#SkRect_Intersects_a'>a</a> and <a href='#SkRect_Intersects_b'>b</a> have area in common

### Example

<div><fiddle-embed name="795061764b10c9e05efb466c9cb60644">

#### Example Output

~~~~
intersection
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_intersect'>intersect</a> <a href='#SkRect_intersects'>intersects</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_Intersects'>Intersects</a>

<a name='Join'></a>

<a name='SkRect_join'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void join(<a href='undocumented#SkScalar'>SkScalar</a> left, <a href='undocumented#SkScalar'>SkScalar</a> top, <a href='undocumented#SkScalar'>SkScalar</a> right, <a href='undocumented#SkScalar'>SkScalar</a> bottom)
</pre>

Constructs <a href='SkRect_Reference#SkRect'>SkRect</a> to intersect from (<a href='#SkRect_join_left'>left</a>, <a href='#SkRect_join_top'>top</a>, <a href='#SkRect_join_right'>right</a>, <a href='#SkRect_join_bottom'>bottom</a>). Does not sort
construction.

Sets <a href='SkRect_Reference#SkRect'>SkRect</a> to the union of itself and the construction.

Has no effect if construction is empty. Otherwise, if <a href='SkRect_Reference#SkRect'>SkRect</a> is empty, sets
<a href='SkRect_Reference#SkRect'>SkRect</a> to construction.

### Parameters

<table>  <tr>    <td><a name='SkRect_join_left'><code><strong>left</strong></code></a></td>
    <td>x-axis minimum of constructed <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
  <tr>    <td><a name='SkRect_join_top'><code><strong>top</strong></code></a></td>
    <td>y-axis minimum of constructed <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
  <tr>    <td><a name='SkRect_join_right'><code><strong>right</strong></code></a></td>
    <td>x-axis maximum of constructed <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
  <tr>    <td><a name='SkRect_join_bottom'><code><strong>bottom</strong></code></a></td>
    <td>y-axis maximum of constructed <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="afa9c6b4d05bb669db07fe0b7b97e6aa">

#### Example Output

~~~~
join: 10, 20, 55, 65
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_joinNonEmptyArg'>joinNonEmptyArg</a> <a href='#SkRect_joinPossiblyEmptyRect'>joinPossiblyEmptyRect</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_join'>join</a>

<a name='SkRect_join_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void join(const <a href='SkRect_Reference#SkRect'>SkRect</a>& r)
</pre>

Sets <a href='SkRect_Reference#SkRect'>SkRect</a> to the union of itself and <a href='#SkRect_join_2_r'>r</a>.

Has no effect if <a href='#SkRect_join_2_r'>r</a> is empty. Otherwise, if <a href='SkRect_Reference#SkRect'>SkRect</a> is empty, sets
<a href='SkRect_Reference#SkRect'>SkRect</a> to <a href='#SkRect_join_2_r'>r</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_join_2_r'><code><strong>r</strong></code></a></td>
    <td>expansion <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="26500032494cf93c5fa3423110fe82af">

#### Example Output

~~~~
join: 10, 20, 55, 65
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_joinNonEmptyArg'>joinNonEmptyArg</a> <a href='#SkRect_joinPossiblyEmptyRect'>joinPossiblyEmptyRect</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_join'>join</a>

<a name='SkRect_joinNonEmptyArg'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRect_joinNonEmptyArg'>joinNonEmptyArg</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& r)
</pre>

Sets <a href='SkRect_Reference#SkRect'>SkRect</a> to the union of itself and <a href='#SkRect_joinNonEmptyArg_r'>r</a>.

Asserts if <a href='#SkRect_joinNonEmptyArg_r'>r</a> is empty and SK_DEBUG is defined.
If <a href='SkRect_Reference#SkRect'>SkRect</a> is empty, sets <a href='SkRect_Reference#SkRect'>SkRect</a> to <a href='#SkRect_joinNonEmptyArg_r'>r</a>.

May produce incorrect results if <a href='#SkRect_joinNonEmptyArg_r'>r</a> is empty.

### Parameters

<table>  <tr>    <td><a name='SkRect_joinNonEmptyArg_r'><code><strong>r</strong></code></a></td>
    <td>expansion <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="88439de2aa0911262c60c0eb506396cb"><div>Since <a href='SkRect_Reference#Rect'>Rect</a> is not sorted, first result is copy of toJoin.
</div>

#### Example Output

~~~~
rect: 50, 60, 55, 65
sorted: 10, 0, 55, 100
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_join'>join</a> <a href='#SkRect_joinPossiblyEmptyRect'>joinPossiblyEmptyRect</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_join'>join</a>

<a name='SkRect_joinPossiblyEmptyRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRect_joinPossiblyEmptyRect'>joinPossiblyEmptyRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& r)
</pre>

Sets <a href='SkRect_Reference#SkRect'>SkRect</a> to the union of itself and the construction.

May produce incorrect results if <a href='SkRect_Reference#SkRect'>SkRect</a> or <a href='#SkRect_joinPossiblyEmptyRect_r'>r</a> is empty.

### Parameters

<table>  <tr>    <td><a name='SkRect_joinPossiblyEmptyRect_r'><code><strong>r</strong></code></a></td>
    <td>expansion <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a476548d0001296afd8e58c1eba1b70b"><div>Since <a href='SkRect_Reference#Rect'>Rect</a> is not sorted, first result is not useful.
</div>

#### Example Output

~~~~
rect: 10, 60, 55, 65
sorted: 10, 0, 55, 100
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_joinNonEmptyArg'>joinNonEmptyArg</a> <a href='#SkRect_join'>join</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_join'>join</a>

<a name='Rounding'></a>

<a name='SkRect_round'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void round(<a href='SkIRect_Reference#SkIRect'>SkIRect</a>* dst)const
</pre>

Sets <a href='SkIRect_Reference#IRect'>IRect</a> by adding 0.5 and discarding the fractional portion of <a href='SkRect_Reference#Rect'>Rect</a>
members, using <code>(<a href='undocumented#SkScalarRoundToInt'>SkScalarRoundToInt</a>(<a href='#SkRect_fLeft'>fLeft</a>), <a href='undocumented#SkScalarRoundToInt'>SkScalarRoundToInt</a>(<a href='#SkRect_fTop'>fTop</a>),
<a href='undocumented#SkScalarRoundToInt'>SkScalarRoundToInt</a>(<a href='#SkRect_fRight'>fRight</a>), <a href='undocumented#SkScalarRoundToInt'>SkScalarRoundToInt</a>(<a href='#SkRect_fBottom'>fBottom</a>))</code>.

### Parameters

<table>  <tr>    <td><a name='SkRect_round_dst'><code><strong>dst</strong></code></a></td>
    <td>storage for <a href='SkIRect_Reference#IRect'>IRect</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="8b9e5a9af0a9b878f76919534d88f41e">

#### Example Output

~~~~
round: 31, 51, 41, 61
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_roundIn'>roundIn</a> <a href='#SkRect_roundOut'>roundOut</a> <a href='undocumented#SkScalarRoundToInt'>SkScalarRoundToInt</a>

<a name='SkRect_roundOut'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRect_roundOut'>roundOut</a>(<a href='SkIRect_Reference#SkIRect'>SkIRect</a>* dst)const
</pre>

Sets <a href='SkIRect_Reference#IRect'>IRect</a> by discarding the fractional portion of <a href='#SkRect_fLeft'>fLeft</a> and <a href='#SkRect_fTop'>fTop</a>; and rounding
up <a href='#SkRect_fRight'>fRight</a> and <a href='#SkRect_fBottom'>fBottom</a>, using
<code>(<a href='undocumented#SkScalarFloorToInt'>SkScalarFloorToInt</a>(<a href='#SkRect_fLeft'>fLeft</a>), <a href='undocumented#SkScalarFloorToInt'>SkScalarFloorToInt</a>(<a href='#SkRect_fTop'>fTop</a>),
<a href='undocumented#SkScalarCeilToInt'>SkScalarCeilToInt</a>(<a href='#SkRect_fRight'>fRight</a>), <a href='undocumented#SkScalarCeilToInt'>SkScalarCeilToInt</a>(<a href='#SkRect_fBottom'>fBottom</a>))</code>.

### Parameters

<table>  <tr>    <td><a name='SkRect_roundOut_dst'><code><strong>dst</strong></code></a></td>
    <td>storage for <a href='SkIRect_Reference#IRect'>IRect</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0bd13d7e6426ae7a3befa2ab151ac5fc">

#### Example Output

~~~~
round: 30, 50, 41, 61
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_roundIn'>roundIn</a> <a href='#SkRect_round'>round</a> <a href='undocumented#SkScalarRoundToInt'>SkScalarRoundToInt</a>

<a name='SkRect_roundOut_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRect_roundOut'>roundOut</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* dst)const
</pre>

Sets <a href='SkRect_Reference#Rect'>Rect</a> by discarding the fractional portion of <a href='#SkRect_fLeft'>fLeft</a> and <a href='#SkRect_fTop'>fTop</a>; and rounding
up <a href='#SkRect_fRight'>fRight</a> and <a href='#SkRect_fBottom'>fBottom</a>, using
<code>(<a href='undocumented#SkScalarFloorToInt'>SkScalarFloorToInt</a>(<a href='#SkRect_fLeft'>fLeft</a>), <a href='undocumented#SkScalarFloorToInt'>SkScalarFloorToInt</a>(<a href='#SkRect_fTop'>fTop</a>),
<a href='undocumented#SkScalarCeilToInt'>SkScalarCeilToInt</a>(<a href='#SkRect_fRight'>fRight</a>), <a href='undocumented#SkScalarCeilToInt'>SkScalarCeilToInt</a>(<a href='#SkRect_fBottom'>fBottom</a>))</code>.

### Parameters

<table>  <tr>    <td><a name='SkRect_roundOut_2_dst'><code><strong>dst</strong></code></a></td>
    <td>storage for <a href='SkRect_Reference#Rect'>Rect</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="e09a6a12869a8ac21e9c2af98a5bb686">

#### Example Output

~~~~
round: 30, 50, 41, 61
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_roundIn'>roundIn</a> <a href='#SkRect_round'>round</a> <a href='undocumented#SkScalarRoundToInt'>SkScalarRoundToInt</a>

<a name='SkRect_roundIn'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRect_roundIn'>roundIn</a>(<a href='SkIRect_Reference#SkIRect'>SkIRect</a>* dst)const
</pre>

Sets <a href='SkRect_Reference#Rect'>Rect</a> by rounding up <a href='#SkRect_fLeft'>fLeft</a> and <a href='#SkRect_fTop'>fTop</a>; and discarding the fractional portion
of <a href='#SkRect_fRight'>fRight</a> and <a href='#SkRect_fBottom'>fBottom</a>, using
<code>(<a href='undocumented#SkScalarCeilToInt'>SkScalarCeilToInt</a>(<a href='#SkRect_fLeft'>fLeft</a>), <a href='undocumented#SkScalarCeilToInt'>SkScalarCeilToInt</a>(<a href='#SkRect_fTop'>fTop</a>),
<a href='undocumented#SkScalarFloorToInt'>SkScalarFloorToInt</a>(<a href='#SkRect_fRight'>fRight</a>), <a href='undocumented#SkScalarFloorToInt'>SkScalarFloorToInt</a>(<a href='#SkRect_fBottom'>fBottom</a>))</code>.

### Parameters

<table>  <tr>    <td><a name='SkRect_roundIn_dst'><code><strong>dst</strong></code></a></td>
    <td>storage for <a href='SkIRect_Reference#IRect'>IRect</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="abb337da8fc1891f016c61258681c64c">

#### Example Output

~~~~
round: 31, 51, 40, 60
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_roundOut'>roundOut</a> <a href='#SkRect_round'>round</a> <a href='undocumented#SkScalarRoundToInt'>SkScalarRoundToInt</a>

<a name='SkRect_round_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkRect_round'>round()</a>const
</pre>

Returns <a href='SkIRect_Reference#IRect'>IRect</a> by adding 0.5 and discarding the fractional portion of <a href='SkRect_Reference#Rect'>Rect</a>
members, using <code>(<a href='undocumented#SkScalarRoundToInt'>SkScalarRoundToInt</a>(<a href='#SkRect_fLeft'>fLeft</a>), <a href='undocumented#SkScalarRoundToInt'>SkScalarRoundToInt</a>(<a href='#SkRect_fTop'>fTop</a>),
<a href='undocumented#SkScalarRoundToInt'>SkScalarRoundToInt</a>(<a href='#SkRect_fRight'>fRight</a>), <a href='undocumented#SkScalarRoundToInt'>SkScalarRoundToInt</a>(<a href='#SkRect_fBottom'>fBottom</a>))</code>.

### Return Value

rounded <a href='SkIRect_Reference#IRect'>IRect</a>

### Example

<div><fiddle-embed name="ef7ae1dd522c235b0afe41b55a624f46">

#### Example Output

~~~~
round: 31, 51, 41, 61
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_roundOut'>roundOut</a> <a href='#SkRect_roundIn'>roundIn</a> <a href='undocumented#SkScalarRoundToInt'>SkScalarRoundToInt</a>

<a name='SkRect_roundOut_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkRect_roundOut'>roundOut</a>()const
</pre>

Sets <a href='SkIRect_Reference#IRect'>IRect</a> by discarding the fractional portion of <a href='#SkRect_fLeft'>fLeft</a> and <a href='#SkRect_fTop'>fTop</a>; and rounding
up <a href='#SkRect_fRight'>fRight</a> and <a href='#SkRect_fBottom'>fBottom</a>, using
<code>(<a href='undocumented#SkScalarFloorToInt'>SkScalarFloorToInt</a>(<a href='#SkRect_fLeft'>fLeft</a>), <a href='undocumented#SkScalarFloorToInt'>SkScalarFloorToInt</a>(<a href='#SkRect_fTop'>fTop</a>),
<a href='undocumented#SkScalarCeilToInt'>SkScalarCeilToInt</a>(<a href='#SkRect_fRight'>fRight</a>), <a href='undocumented#SkScalarCeilToInt'>SkScalarCeilToInt</a>(<a href='#SkRect_fBottom'>fBottom</a>))</code>.

### Return Value

rounded <a href='SkIRect_Reference#IRect'>IRect</a>

### Example

<div><fiddle-embed name="05f0f65ae148f192656cd87df90f1d57">

#### Example Output

~~~~
round: 30, 50, 41, 61
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_round'>round</a> <a href='#SkRect_roundIn'>roundIn</a> <a href='undocumented#SkScalarRoundToInt'>SkScalarRoundToInt</a>

<a name='Sorting'></a>

<a name='SkRect_sort'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRect_sort'>sort()</a>
</pre>

Swaps <a href='#SkRect_fLeft'>fLeft</a> and <a href='#SkRect_fRight'>fRight</a> if <a href='#SkRect_fLeft'>fLeft</a> is greater than <a href='#SkRect_fRight'>fRight</a>; and swaps
<a href='#SkRect_fTop'>fTop</a> and <a href='#SkRect_fBottom'>fBottom</a> if <a href='#SkRect_fTop'>fTop</a> is greater than <a href='#SkRect_fBottom'>fBottom</a>. Result may be empty;
and <a href='#SkRect_width'>width()</a> and <a href='#SkRect_height'>height()</a> will be zero or positive.

### Example

<div><fiddle-embed name="e624fe398e3d770b573c09fc74c0c400">

#### Example Output

~~~~
rect: 30.5, 50.5, 20.5, 10.5
sorted: 20.5, 10.5, 30.5, 50.5
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_makeSorted'>makeSorted</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_sort'>sort</a> <a href='#SkRect_isSorted'>isSorted</a>

<a name='SkRect_makeSorted'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_makeSorted'>makeSorted</a>()const
</pre>

Returns <a href='SkRect_Reference#SkRect'>SkRect</a> with <a href='#SkRect_fLeft'>fLeft</a> and <a href='#SkRect_fRight'>fRight</a> swapped if <a href='#SkRect_fLeft'>fLeft</a> is greater than <a href='#SkRect_fRight'>fRight</a>; and
with <a href='#SkRect_fTop'>fTop</a> and <a href='#SkRect_fBottom'>fBottom</a> swapped if <a href='#SkRect_fTop'>fTop</a> is greater than <a href='#SkRect_fBottom'>fBottom</a>. Result may be empty;
and <a href='#SkRect_width'>width()</a> and <a href='#SkRect_height'>height()</a> will be zero or positive.

### Return Value

sorted <a href='SkRect_Reference#SkRect'>SkRect</a>

### Example

<div><fiddle-embed name="f59567042b87f6b26f9bfeeb04468032">

#### Example Output

~~~~
rect: 30.5, 50.5, 20.5, 10.5
sorted: 20.5, 10.5, 30.5, 50.5
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_sort'>sort</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_makeSorted'>makeSorted</a> <a href='#SkRect_isSorted'>isSorted</a>

<a name='SkRect_asScalars'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='undocumented#SkScalar'>SkScalar</a>* <a href='#SkRect_asScalars'>asScalars</a>()const
</pre>

Returns pointer to first <a href='undocumented#Scalar'>scalar</a> in <a href='SkRect_Reference#SkRect'>SkRect</a>, to treat it as an array with four
entries.

### Return Value

pointer to <a href='#SkRect_fLeft'>fLeft</a>

### Example

<div><fiddle-embed name="e1ea5f949d80276f3637931eae93a07c">

#### Example Output

~~~~
rect.asScalars() == &rect.fLeft
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_toQuad'>toQuad</a>

<a name='SkRect_dump'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRect_dump'>dump</a>(bool asHex)const
</pre>

Writes <a href='undocumented#Text'>text</a> representation of <a href='SkRect_Reference#SkRect'>SkRect</a> to standard output. Set <a href='#SkRect_dump_asHex'>asHex</a> to true to
generate exact binary representations of floating <a href='SkPoint_Reference#Point'>point</a> numbers.

### Parameters

<table>  <tr>    <td><a name='SkRect_dump_asHex'><code><strong>asHex</strong></code></a></td>
    <td>true if <a href='undocumented#SkScalar'>SkScalar</a> values are written as hexadecimal</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="cea049ffff702a5923da41fe0ae0763b">

#### Example Output

~~~~
SkRect::MakeLTRB(20, 30, 40, 50);
SkRect::MakeLTRB(SkBits2Float(0x41a00000), /* 20.000000 */
SkBits2Float(0x41f00000), /* 30.000000 */
SkBits2Float(0x42200000), /* 40.000000 */
SkBits2Float(0x42480000)  /* 50.000000 */);
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_dumpHex'>dumpHex</a>

<a name='SkRect_dump_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRect_dump'>dump()</a>const
</pre>

Writes <a href='undocumented#Text'>text</a> representation of <a href='SkRect_Reference#SkRect'>SkRect</a> to standard output. The representation may be
directly compiled as C++ code. Floating <a href='SkPoint_Reference#Point'>point</a> values are written
with limited precision; it may not be possible to reconstruct original <a href='SkRect_Reference#SkRect'>SkRect</a>
from output.

### Example

<div><fiddle-embed name="9fb76971b1a104a2a59816e0392267a7">

#### Example Output

~~~~
SkRect::MakeLTRB(0.857143f, 0.666667f, 2.6f, 7);
rect is not equal to copy
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_dumpHex'>dumpHex</a>

<a name='SkRect_dumpHex'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRect_dumpHex'>dumpHex</a>()const
</pre>

Writes <a href='undocumented#Text'>text</a> representation of <a href='SkRect_Reference#Rect'>Rect</a> to standard output. The representation may be
directly compiled as C++ code. Floating <a href='SkPoint_Reference#Point'>point</a> values are written
in hexadecimal to preserve their exact bit pattern. The output reconstructs the
original <a href='SkRect_Reference#Rect'>Rect</a>.

Use instead of <a href='#SkRect_dump'>dump()</a> when submitting
<a href='https://bug.skia.org'>bug reports against Skia</a></a> .

### Example

<div><fiddle-embed name="824b5a3fcfd46a7e1c5f9e3c16e6bb39">

#### Example Output

~~~~
SkRect::MakeLTRB(SkBits2Float(0x3f5b6db7), /* 0.857143 */
SkBits2Float(0x3f2aaaab), /* 0.666667 */
SkBits2Float(0x40266666), /* 2.600000 */
SkBits2Float(0x40e00000)  /* 7.000000 */);
rect is equal to copy
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRect_dump'>dump</a>

