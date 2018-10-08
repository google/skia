SkIRect Reference
===

<a name='SkIRect'></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
struct <a href='#SkIRect'>SkIRect</a> {
    int32_t <a href='#SkIRect_fLeft'>fLeft</a>;
    int32_t <a href='#SkIRect_fTop'>fTop</a>;
    int32_t <a href='#SkIRect_fRight'>fRight</a>;
    int32_t <a href='#SkIRect_fBottom'>fBottom</a>;

    static constexpr <a href='#SkIRect'>SkIRect</a> <a href='#SkIRect_MakeEmpty'>MakeEmpty</a>();
    static <a href='#SkIRect'>SkIRect</a> <a href='#SkIRect_MakeLargest'>MakeLargest</a>();
    static constexpr <a href='#SkIRect'>SkIRect</a> <a href='#SkIRect_MakeWH'>MakeWH</a>(int32_t w, int32_t h);
    static constexpr <a href='#SkIRect'>SkIRect</a> <a href='#SkIRect_MakeSize'>MakeSize</a>(const <a href='undocumented#SkISize'>SkISize</a>& size);
    static constexpr <a href='#SkIRect'>SkIRect</a> <a href='#SkIRect_MakeLTRB'>MakeLTRB</a>(int32_t l, int32_t t,
                                      int32_t r, int32_t b);
    static constexpr <a href='#SkIRect'>SkIRect</a> <a href='#SkIRect_MakeXYWH'>MakeXYWH</a>(int32_t x, int32_t y,
                                      int32_t w, int32_t h);
    int32_t <a href='#SkIRect_left'>left</a>() const;
    int32_t <a href='#SkIRect_top'>top</a>() const;
    int32_t <a href='#SkIRect_right'>right</a>() const;
    int32_t <a href='#SkIRect_bottom'>bottom</a>() const;
    int32_t <a href='#SkIRect_x'>x</a>() const;
    int32_t <a href='#SkIRect_y'>y</a>() const;
    int32_t <a href='#SkIRect_width'>width</a>() const;
    int32_t <a href='#SkIRect_height'>height</a>() const;
    <a href='undocumented#SkISize'>SkISize</a> <a href='#SkIRect_size'>size</a>() const;
    int64_t <a href='#SkIRect_width64'>width64</a>() const;
    int64_t <a href='#SkIRect_height64'>height64</a>() const;
    bool <a href='#SkIRect_isEmpty64'>isEmpty64</a>() const;
    bool <a href='#SkIRect_isEmpty'>isEmpty</a>() const;
    friend bool <a href='#SkIRect_equal_operator'>operator==(const SkIRect& a, const SkIRect& b)</a>;
    friend bool <a href='#SkIRect_notequal_operator'>operator!=(const SkIRect& a, const SkIRect& b)</a>;
    void <a href='#SkIRect_setEmpty'>setEmpty</a>();
    void <a href='#SkIRect_set'>set</a>(int32_t left, int32_t top, int32_t right, int32_t bottom);
    void <a href='#SkIRect_setLTRB'>setLTRB</a>(int32_t left, int32_t top, int32_t right, int32_t bottom);
    void <a href='#SkIRect_setXYWH'>setXYWH</a>(int32_t x, int32_t y, int32_t width, int32_t height);
    <a href='#SkIRect'>SkIRect</a> <a href='#SkIRect_makeOffset'>makeOffset</a>(int32_t dx, int32_t dy) const;
    <a href='#SkIRect'>SkIRect</a> <a href='#SkIRect_makeInset'>makeInset</a>(int32_t dx, int32_t dy) const;
    <a href='#SkIRect'>SkIRect</a> <a href='#SkIRect_makeOutset'>makeOutset</a>(int32_t dx, int32_t dy) const;
    void <a href='#SkIRect_offset'>offset</a>(int32_t dx, int32_t dy);
    void <a href='#SkIRect_offset_2'>offset</a>(const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& delta);
    void <a href='#SkIRect_offsetTo'>offsetTo</a>(int32_t newX, int32_t newY);
    void <a href='#SkIRect_inset'>inset</a>(int32_t dx, int32_t dy);
    void <a href='#SkIRect_outset'>outset</a>(int32_t dx, int32_t dy);
    void <a href='#SkIRect_adjust'>adjust</a>(int32_t dL, int32_t dT, int32_t dR, int32_t dB);
    bool <a href='#SkIRect_contains'>contains</a>(int32_t x, int32_t y) const;
    bool <a href='#SkIRect_contains_2'>contains</a>(int32_t left, int32_t top, int32_t right, int32_t bottom) const;
    bool <a href='#SkIRect_contains_3'>contains</a>(const <a href='#SkIRect'>SkIRect</a>& r) const;
    bool <a href='#SkIRect_contains_4'>contains</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& r) const;
    bool <a href='#SkIRect_containsNoEmptyCheck'>containsNoEmptyCheck</a>(int32_t left, int32_t top,
                              int32_t right, int32_t bottom) const;
    bool <a href='#SkIRect_containsNoEmptyCheck_2'>containsNoEmptyCheck</a>(const <a href='#SkIRect'>SkIRect</a>& r) const;
    bool <a href='#SkIRect_intersect'>intersect</a>(const <a href='#SkIRect'>SkIRect</a>& r);
    bool <a href='#SkIRect_intersectNoEmptyCheck'>intersectNoEmptyCheck</a>(const <a href='#SkIRect'>SkIRect</a>& a, const <a href='#SkIRect'>SkIRect</a>& b);
    bool <a href='#SkIRect_intersect_2'>intersect</a>(const <a href='#SkIRect'>SkIRect</a>& a, const <a href='#SkIRect'>SkIRect</a>& b);
    bool <a href='#SkIRect_intersect_3'>intersect</a>(int32_t left, int32_t top, int32_t right, int32_t bottom);
    static bool <a href='#SkIRect_Intersects'>Intersects</a>(const <a href='#SkIRect'>SkIRect</a>& a, const <a href='#SkIRect'>SkIRect</a>& b);
    static bool <a href='#SkIRect_IntersectsNoEmptyCheck'>IntersectsNoEmptyCheck</a>(const <a href='#SkIRect'>SkIRect</a>& a, const <a href='#SkIRect'>SkIRect</a>& b);
    void <a href='#SkIRect_join'>join</a>(int32_t left, int32_t top, int32_t right, int32_t bottom);
    void <a href='#SkIRect_join_2'>join</a>(const <a href='#SkIRect'>SkIRect</a>& r);
    void <a href='#SkIRect_sort'>sort</a>();
    <a href='#SkIRect'>SkIRect</a> <a href='#SkIRect_makeSorted'>makeSorted</a>() const;
    static const <a href='#SkIRect'>SkIRect</a>& <a href='#SkIRect_EmptyIRect'>EmptyIRect</a>();
};
</pre>

<a href='#SkIRect'>SkIRect</a> holds four 32

## <a name='Members'>Members</a>



### Members

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Member</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#fLeft'><code>fLeft</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>smaller x-axis bounds</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#fTop'><code>fTop</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>smaller y-axis bounds</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#fRight'><code>fRight</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>larger x-axis bounds</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#fBottom'><code>fBottom</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>larger y-axis bounds</td>
  </tr>
</table>
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Member</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkIRect_fLeft'><code>fLeft</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May contain any value</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkIRect_fTop'><code>fTop</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May contain any value</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkIRect_fRight'><code>fRight</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May contain any value</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkIRect_fBottom'><code>fBottom</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May contain any value</td>
  </tr>


## <a name='Constructors'>Constructors</a>

<a name='SkIRect_MakeEmpty'></a>
## MakeEmpty

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='#SkIRect'>SkIRect</a> <a href='#SkIRect_MakeEmpty'>MakeEmpty</a>(
</pre>

Returns constructed <a href='#IRect'>IRect</a> set to

### Return Value

bounds

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

<a href='#SkIRect_EmptyIRect'>EmptyIRect</a> <a href='#SkIRect_isEmpty'>isEmpty</a> <a href='#SkIRect_setEmpty'>setEmpty</a> <a href='SkRect_Reference#SkRect_MakeEmpty'>SkRect::MakeEmpty</a>

---

<a name='SkIRect_MakeWH'></a>
## MakeWH

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='#SkIRect'>SkIRect</a> <a href='#SkIRect_MakeWH'>MakeWH</a>(int32_t w
</pre>

Returns constructed <a href='#IRect'>IRect</a> set to

### Parameters

<table>  <tr>    <td><a name='SkIRect_MakeWH_w'><code><strong>w</strong></code></a></td>
    <td>width of constructed <a href='#IRect'>IRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_MakeWH_h'><code><strong>h</strong></code></a></td>
    <td>height of constructed <a href='#IRect'>IRect</a></td>
  </tr>
</table>

### Return Value

bounds

### Example

<div><fiddle-embed name="e36827a1a6ae2b1c26e7a8a08f325a07">

#### Example Output

~~~~
all equal
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_MakeSize'>MakeSize</a> <a href='#SkIRect_MakeXYWH'>MakeXYWH</a> <a href='SkRect_Reference#SkRect_MakeWH'>SkRect::MakeWH</a> <a href='SkRect_Reference#SkRect_MakeIWH'>SkRect::MakeIWH</a>

---

<a name='SkIRect_MakeSize'></a>
## MakeSize

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='#SkIRect'>SkIRect</a> <a href='#SkIRect_MakeSize'>MakeSize</a>(const <a href='undocumented#SkISize'>SkISize</a>
</pre>

Returns constructed <a href='#IRect'>IRect</a> set to

### Parameters

<table>  <tr>    <td><a name='SkIRect_MakeSize_size'><code><strong>size</strong></code></a></td>
    <td>values for <a href='#IRect'>IRect</a> width and height</td>
  </tr>
</table>

### Return Value

bounds

### Example

<div><fiddle-embed name="c6586ff8d24869c780169b0d19c75df6">

#### Example Output

~~~~
round width: 26  height: 36
floor width: 25  height: 35
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_MakeWH'>MakeWH</a> <a href='#SkIRect_MakeXYWH'>MakeXYWH</a> <a href='SkRect_Reference#SkRect_Make'>SkRect::Make</a><sup><a href='SkRect_Reference#SkRect_Make_2'>[2]</a></sup> <a href='SkRect_Reference#SkRect_MakeIWH'>SkRect::MakeIWH</a>

---

<a name='SkIRect_MakeLTRB'></a>
## MakeLTRB

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='#SkIRect'>SkIRect</a> <a href='#SkIRect_MakeLTRB'>MakeLTRB</a>(int32_t l
</pre>

Returns constructed <a href='#IRect'>IRect</a> set to

### Parameters

<table>  <tr>    <td><a name='SkIRect_MakeLTRB_l'><code><strong>l</strong></code></a></td>
    <td>integer stored in <a href='#SkIRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_MakeLTRB_t'><code><strong>t</strong></code></a></td>
    <td>integer stored in <a href='#SkIRect_fTop'>fTop</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_MakeLTRB_r'><code><strong>r</strong></code></a></td>
    <td>integer stored in <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_MakeLTRB_b'><code><strong>b</strong></code></a></td>
    <td>integer stored in <a href='#SkIRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

bounds

### Example

<div><fiddle-embed name="ec1473b700c594f2df9749a12a06b89b">

#### Example Output

~~~~
rect: 5, 35, 15, 25  isEmpty: true
rect: 5, 25, 15, 35  isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_MakeXYWH'>MakeXYWH</a> <a href='SkRect_Reference#SkRect_MakeLTRB'>SkRect::MakeLTRB</a>

---

<a name='SkIRect_MakeXYWH'></a>
## MakeXYWH

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='#SkIRect'>SkIRect</a> <a href='#SkIRect_MakeXYWH'>MakeXYWH</a>(int32_t x
</pre>

Returns constructed <a href='#IRect'>IRect</a> set to <code></code>

### Parameters

<table>  <tr>    <td><a name='SkIRect_MakeXYWH_x'><code><strong>x</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_MakeXYWH_y'><code><strong>y</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fTop'>fTop</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_MakeXYWH_w'><code><strong>w</strong></code></a></td>
    <td>added to x and stored in <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_MakeXYWH_h'><code><strong>h</strong></code></a></td>
    <td>added to y and stored in <a href='#SkIRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

bounds at

### Example

<div><fiddle-embed name="598ee14350bd1d961cae6b36fa3df17e">

#### Example Output

~~~~
rect: 5, 35, -10, 60  isEmpty: true
rect: -10, 35, 5, 60  isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_MakeLTRB'>MakeLTRB</a> <a href='SkRect_Reference#SkRect_MakeXYWH'>SkRect::MakeXYWH</a>

---

## <a name='Property'>Property</a>

<a name='SkIRect_left'></a>
## left

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIRect_left'>left</a>(
</pre>

Returns left edge of <a href='#IRect'>IRect</a>

### Return Value

<a href='#SkIRect_fLeft'>fLeft</a>

### Example

<div><fiddle-embed name="caf38ea4431bc246ba198f6a8c2b0f01">

#### Example Output

~~~~
unsorted.fLeft: 15 unsorted.left(): 15
sorted.fLeft: 10 sorted.left(): 10
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_x'>x</a>(

---

<a name='SkIRect_top'></a>
## top

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIRect_top'>top</a>(
</pre>

Returns top edge of <a href='#IRect'>IRect</a>

### Return Value

<a href='#SkIRect_fTop'>fTop</a>

### Example

<div><fiddle-embed name="cbec1ae6530e95943775450b1d11f19e">

#### Example Output

~~~~
unsorted.fTop: 25 unsorted.top(): 25
sorted.fTop: 5 sorted.top(): 5
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_y'>y</a>(

---

<a name='SkIRect_right'></a>
## right

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIRect_right'>right</a>(
</pre>

Returns right edge of <a href='#IRect'>IRect</a>

### Return Value

<a href='#SkIRect_fRight'>fRight</a>

### Example

<div><fiddle-embed name="97e210976f1ee0387b30c70635cf114f">

#### Example Output

~~~~
unsorted.fRight: 10 unsorted.right(): 10
sorted.fRight: 15 sorted.right(): 15
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_fRight'>fRight</a> <a href='SkRect_Reference#SkRect_right'>SkRect::right</a>(

---

<a name='SkIRect_bottom'></a>
## bottom

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIRect_bottom'>bottom</a>(
</pre>

Returns bottom edge of <a href='#IRect'>IRect</a>

### Return Value

<a href='#SkIRect_fBottom'>fBottom</a>

### Example

<div><fiddle-embed name="c32afebc296054a181621648a184b8e3">

#### Example Output

~~~~
unsorted.fBottom: 5 unsorted.bottom(): 5
sorted.fBottom: 25 sorted.bottom(): 25
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_fBottom'>fBottom</a> <a href='SkRect_Reference#SkRect_bottom'>SkRect::bottom</a>(

---

<a name='SkIRect_x'></a>
## x

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIRect_x'>x</a>(
</pre>

Returns left edge of <a href='#IRect'>IRect</a>

### Return Value

<a href='#SkIRect_fLeft'>fLeft</a>

### Example

<div><fiddle-embed name="2a59cbfd1330a0db520d6ebb2b7c68c7">

#### Example Output

~~~~
unsorted.fLeft: 15 unsorted.x(): 15
sorted.fLeft: 10 sorted.x(): 10
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_left'>left</a>(

---

<a name='SkIRect_y'></a>
## y

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIRect_y'>y</a>(
</pre>

Returns top edge of <a href='#IRect'>IRect</a>

### Return Value

<a href='#SkIRect_fTop'>fTop</a>

### Example

<div><fiddle-embed name="6ea461e71f7fc80605818fbf493caa63">

#### Example Output

~~~~
unsorted.fTop: 25 unsorted.y(): 25
sorted.fTop: 5 sorted.y(): 5
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_top'>top</a>(

---

<a name='SkIRect_width'></a>
## width

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIRect_width'>width</a>(
</pre>

Returns span on the x

### Return Value

<a href='#SkIRect_fRight'>fRight</a> minus <a href='#SkIRect_fLeft'>fLeft</a>

### Example

<div><fiddle-embed name="4acfbe051805940210c8916a94794142">

#### Example Output

~~~~
unsorted width: -5
large width: -5
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_height'>height</a>(

---

<a name='SkIRect_width64'></a>
## width64

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int64_t <a href='#SkIRect_width64'>width64</a>(
</pre>

Returns span on the x

### Return Value

<a href='#SkIRect_fRight'>fRight</a> minus <a href='#SkIRect_fLeft'>fLeft</a> cast to int64_t

### Example

<div><fiddle-embed name="63977f97999bbd6eecfdcc7575d75492">

#### Example Output

~~~~
width: -5 width64: 4294967291
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_width'>width</a>(

---

<a name='SkIRect_height'></a>
## height

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIRect_height'>height</a>(
</pre>

Returns span on the y

### Return Value

<a href='#SkIRect_fBottom'>fBottom</a> minus <a href='#SkIRect_fTop'>fTop</a>

### Example

<div><fiddle-embed name="0175bae87fafcd9433ae661574695586">

#### Example Output

~~~~
unsorted height: -5
large height: -5
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_width'>width</a>(

---

<a name='SkIRect_height64'></a>
## height64

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int64_t <a href='#SkIRect_height64'>height64</a>(
</pre>

Returns span on the y

### Return Value

<a href='#SkIRect_fBottom'>fBottom</a> minus <a href='#SkIRect_fTop'>fTop</a> cast to int64_t

### Example

<div><fiddle-embed name="02dd98716e54bbd8c2f0ff23b7ef98cf">

#### Example Output

~~~~
height: -5 height64: 4294967291
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_width'>width</a>(

---

<a name='SkIRect_size'></a>
## size

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkISize'>SkISize</a> <a href='#SkIRect_size'>size</a>(
</pre>

Returns spans on the x

### Return Value

<a href='undocumented#ISize'>ISize</a>

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

<a href='#SkIRect_height'>height</a>(

---

<a name='SkIRect_isEmpty'></a>
## isEmpty

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_isEmpty'>isEmpty</a>(
</pre>

Returns true if <a href='#SkIRect_width'>width</a>(

### Return Value

true if <a href='#SkIRect_width'>width</a>(

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

<a href='#SkIRect_EmptyIRect'>EmptyIRect</a> <a href='#SkIRect_MakeEmpty'>MakeEmpty</a> <a href='#SkIRect_sort'>sort</a> <a href='SkRect_Reference#SkRect_isEmpty'>SkRect::isEmpty</a>

---

<a name='SkIRect_isEmpty64'></a>
## isEmpty64

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_isEmpty64'>isEmpty64</a>(
</pre>

Returns true if <a href='#SkIRect_fLeft'>fLeft</a> is equal to or greater than <a href='#SkIRect_fRight'>fRight</a>

### Return Value

true if <a href='#SkIRect_width64'>width64</a>(

### Example

<div><fiddle-embed name="eb905faa1084ccab3ad0605df4c27ea4">

#### Example Output

~~~~
rect: {20, 40, 10, 50} is empty
sorted: {10, 40, 20, 50} is not empty
rect: {20, 40, 20, 50} is empty
sorted: {20, 40, 20, 50} is empty
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_EmptyIRect'>EmptyIRect</a> <a href='#SkIRect_MakeEmpty'>MakeEmpty</a> <a href='#SkIRect_sort'>sort</a> <a href='SkRect_Reference#SkRect_isEmpty'>SkRect::isEmpty</a>

---

## <a name='Operators'>Operators</a>

<a name='SkIRect_equal_operator'></a>
## operator==

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_equal_operator'>operator==(const SkIRect& a, const SkIRect& b)</a>
</pre>

Returns true if all members in <a href='#SkIRect_equal_operator_a'>a</a>

### Parameters

<table>  <tr>    <td><a name='SkIRect_equal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> to compare</td>
  </tr>
  <tr>    <td><a name='SkIRect_equal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> to compare</td>
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

<a href='#SkIRect_notequal_operator'>operator!=(const SkIRect& a, const SkIRect& b)</a>

---

<a name='SkIRect_notequal_operator'></a>
## operator!=

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_notequal_operator'>operator!=(const SkIRect& a, const SkIRect& b)</a>
</pre>

Returns true if any member in <a href='#SkIRect_notequal_operator_a'>a</a>

### Parameters

<table>  <tr>    <td><a name='SkIRect_notequal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> to compare</td>
  </tr>
  <tr>    <td><a name='SkIRect_notequal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> to compare</td>
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

<a href='#SkIRect_equal_operator'>operator==(const SkIRect& a, const SkIRect& b)</a>

---

## <a name='Set'>Set</a>

<a name='SkIRect_setEmpty'></a>
## setEmpty

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_setEmpty'>setEmpty</a>(
</pre>

Sets <a href='#IRect'>IRect</a> to

### Example

<div><fiddle-embed name="94039c3cc9e911c8ab2993d56fd06210">

#### Example Output

~~~~
rect: {3, 4, 1, 2} is empty
rect: {0, 0, 0, 0} is empty
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_MakeEmpty'>MakeEmpty</a> <a href='SkRect_Reference#SkRect_setEmpty'>SkRect::setEmpty</a>

---

<a name='SkIRect_set'></a>
## set

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_set'>set</a>(int32_t left
</pre>

Sets <a href='#IRect'>IRect</a> to

### Parameters

<table>  <tr>    <td><a name='SkIRect_set_left'><code><strong>left</strong></code></a></td>
    <td>assigned to <a href='#SkIRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_set_top'><code><strong>top</strong></code></a></td>
    <td>assigned to <a href='#SkIRect_fTop'>fTop</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_set_right'><code><strong>right</strong></code></a></td>
    <td>assigned to <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_set_bottom'><code><strong>bottom</strong></code></a></td>
    <td>assigned to <a href='#SkIRect_fBottom'>fBottom</a></td>
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

<a href='#SkIRect_setLTRB'>setLTRB</a> <a href='#SkIRect_setXYWH'>setXYWH</a> <a href='SkRect_Reference#SkRect_set'>SkRect::set</a><sup><a href='SkRect_Reference#SkRect_set_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_set_3'>[3]</a></sup><sup><a href='SkRect_Reference#SkRect_set_4'>[4]</a></sup>

---

<a name='SkIRect_setLTRB'></a>
## setLTRB

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_setLTRB'>setLTRB</a>(int32_t left
</pre>

Sets <a href='#IRect'>IRect</a> to

### Parameters

<table>  <tr>    <td><a name='SkIRect_setLTRB_left'><code><strong>left</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_setLTRB_top'><code><strong>top</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fTop'>fTop</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_setLTRB_right'><code><strong>right</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_setLTRB_bottom'><code><strong>bottom</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fBottom'>fBottom</a></td>
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

<a href='#SkIRect_set'>set</a> <a href='#SkIRect_setXYWH'>setXYWH</a> <a href='SkRect_Reference#SkRect_setLTRB'>SkRect::setLTRB</a>

---

<a name='SkIRect_setXYWH'></a>
## setXYWH

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_setXYWH'>setXYWH</a>(int32_t x
</pre>

Sets <a href='#IRect'>IRect</a> to <code></code>

### Parameters

<table>  <tr>    <td><a name='SkIRect_setXYWH_x'><code><strong>x</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_setXYWH_y'><code><strong>y</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fTop'>fTop</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_setXYWH_width'><code><strong>width</strong></code></a></td>
    <td>added to x and stored in <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_setXYWH_height'><code><strong>height</strong></code></a></td>
    <td>added to y and stored in <a href='#SkIRect_fBottom'>fBottom</a></td>
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

<a href='#SkIRect_MakeXYWH'>MakeXYWH</a> <a href='#SkIRect_setLTRB'>setLTRB</a> <a href='#SkIRect_set'>set</a> <a href='SkRect_Reference#SkRect_setXYWH'>SkRect::setXYWH</a>

---

## <a name='Inset_Outset_Offset'>Inset Outset Offset</a>

<a name='SkIRect_makeOffset'></a>
## makeOffset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkIRect'>SkIRect</a> <a href='#SkIRect_makeOffset'>makeOffset</a>(int32_t dx
</pre>

Returns <a href='#IRect'>IRect</a> offset by

### Parameters

<table>  <tr>    <td><a name='SkIRect_makeOffset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fLeft'>fLeft</a> and <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_makeOffset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fTop'>fTop</a> and <a href='#SkIRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

<a href='#IRect'>IRect</a> offset by <a href='#SkIRect_makeOffset_dx'>dx</a> and <a href='#SkIRect_makeOffset_dy'>dy</a>

### Example

<div><fiddle-embed name="737c747df07ddf392c05970440de0927">

#### Example Output

~~~~
rect: 10, 50, 20, 60  isEmpty: false
rect: 25, 82, 35, 92  isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_offset'>offset</a><sup><a href='#SkIRect_offset_2'>[2]</a></sup>(

---

<a name='SkIRect_makeInset'></a>
## makeInset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkIRect'>SkIRect</a> <a href='#SkIRect_makeInset'>makeInset</a>(int32_t dx
</pre>

Returns <a href='#IRect'>IRect</a>

### Parameters

<table>  <tr>    <td><a name='SkIRect_makeInset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fLeft'>fLeft</a> and subtracted from <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_makeInset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fTop'>fTop</a> and subtracted from <a href='#SkIRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

<a href='#IRect'>IRect</a> inset symmetrically left and right

### Example

<div><fiddle-embed name="1db94b2c76e0a7a71856532335fa56b6">

#### Example Output

~~~~
rect: 10, 50, 20, 60  isEmpty: false
rect: 25, 82, 5, 28  isEmpty: true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_inset'>inset</a>(

---

<a name='SkIRect_makeOutset'></a>
## makeOutset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkIRect'>SkIRect</a> <a href='#SkIRect_makeOutset'>makeOutset</a>(int32_t dx
</pre>

Returns <a href='#IRect'>IRect</a>

### Parameters

<table>  <tr>    <td><a name='SkIRect_makeOutset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset subtracted to <a href='#SkIRect_fLeft'>fLeft</a> and added from <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_makeOutset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset subtracted to <a href='#SkIRect_fTop'>fTop</a> and added from <a href='#SkIRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

<a href='#IRect'>IRect</a> outset symmetrically left and right

### Example

<div><fiddle-embed name="240e2953e3455c08f6d89255feff8416">

#### Example Output

~~~~
rect: 10, 50, 20, 60  isEmpty: false
rect: -5, 18, 35, 92  isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_outset'>outset</a>(

---

<a name='SkIRect_offset'></a>
## offset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_offset'>offset</a>(int32_t dx
</pre>

Offsets <a href='#IRect'>IRect</a> by adding <a href='#SkIRect_offset_dx'>dx</a> to <a href='#SkIRect_fLeft'>fLeft</a>

### Parameters

<table>  <tr>    <td><a name='SkIRect_offset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fLeft'>fLeft</a> and <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_offset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fTop'>fTop</a> and <a href='#SkIRect_fBottom'>fBottom</a></td>
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

<a href='#SkIRect_offsetTo'>offsetTo</a> <a href='#SkIRect_makeOffset'>makeOffset</a> <a href='SkRect_Reference#SkRect_offset'>SkRect::offset</a><sup><a href='SkRect_Reference#SkRect_offset_2'>[2]</a></sup>

---

<a name='SkIRect_offset_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_offset'>offset</a>(const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>
</pre>

Offsets <a href='#IRect'>IRect</a> by adding <a href='#SkIRect_offset_2_delta'>delta</a>

### Parameters

<table>  <tr>    <td><a name='SkIRect_offset_2_delta'><code><strong>delta</strong></code></a></td>
    <td>offset added to <a href='#IRect'>IRect</a></td>
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

<a href='#SkIRect_offsetTo'>offsetTo</a> <a href='#SkIRect_makeOffset'>makeOffset</a> <a href='SkRect_Reference#SkRect_offset'>SkRect::offset</a><sup><a href='SkRect_Reference#SkRect_offset_2'>[2]</a></sup>

---

<a name='SkIRect_offsetTo'></a>
## offsetTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_offsetTo'>offsetTo</a>(int32_t newX
</pre>

Offsets <a href='#IRect'>IRect</a> so that <a href='#SkIRect_fLeft'>fLeft</a> equals <a href='#SkIRect_offsetTo_newX'>newX</a>

### Parameters

<table>  <tr>    <td><a name='SkIRect_offsetTo_newX'><code><strong>newX</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_offsetTo_newY'><code><strong>newY</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fTop'>fTop</a></td>
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

<a href='#SkIRect_offset'>offset</a><sup><a href='#SkIRect_offset_2'>[2]</a></sup> <a href='#SkIRect_makeOffset'>makeOffset</a> <a href='#SkIRect_setXYWH'>setXYWH</a> <a href='SkRect_Reference#SkRect_offsetTo'>SkRect::offsetTo</a>

---

<a name='SkIRect_inset'></a>
## inset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_inset'>inset</a>(int32_t dx
</pre>

Insets <a href='#IRect'>IRect</a> by

### Parameters

<table>  <tr>    <td><a name='SkIRect_inset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fLeft'>fLeft</a> and subtracted from <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_inset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fTop'>fTop</a> and subtracted from <a href='#SkIRect_fBottom'>fBottom</a></td>
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

<a href='#SkIRect_outset'>outset</a> <a href='#SkIRect_makeInset'>makeInset</a> <a href='SkRect_Reference#SkRect_inset'>SkRect::inset</a>

---

<a name='SkIRect_outset'></a>
## outset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_outset'>outset</a>(int32_t dx
</pre>

Outsets <a href='#IRect'>IRect</a> by

### Parameters

<table>  <tr>    <td><a name='SkIRect_outset_dx'><code><strong>dx</strong></code></a></td>
    <td>subtracted to <a href='#SkIRect_fLeft'>fLeft</a> and added from <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_outset_dy'><code><strong>dy</strong></code></a></td>
    <td>subtracted to <a href='#SkIRect_fTop'>fTop</a> and added from <a href='#SkIRect_fBottom'>fBottom</a></td>
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

<a href='#SkIRect_inset'>inset</a> <a href='#SkIRect_makeOutset'>makeOutset</a> <a href='SkRect_Reference#SkRect_outset'>SkRect::outset</a>

---

## <a name='Intersection'>Intersection</a>

<a href='#IRect'>IRects</a> intersect when they enclose a common area <code></code>

<a name='SkIRect_adjust'></a>
## adjust

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_adjust'>adjust</a>(int32_t dL
</pre>

Adjusts <a href='#IRect'>IRect</a> by adding <a href='#SkIRect_adjust_dL'>dL</a> to <a href='#SkIRect_fLeft'>fLeft</a>

### Parameters

<table>  <tr>    <td><a name='SkIRect_adjust_dL'><code><strong>dL</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_adjust_dT'><code><strong>dT</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fTop'>fTop</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_adjust_dR'><code><strong>dR</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_adjust_dB'><code><strong>dB</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="8dc91284493dd012cca3d0ce4c66bda4">

#### Example Output

~~~~
rect: 10, 10, 20, 20
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_inset'>inset</a> <a href='#SkIRect_outset'>outset</a>

---

<a name='SkIRect_contains'></a>
## contains

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_contains'>contains</a>(int32_t x
</pre>

Returns true if <code><a href='#SkIRect_fLeft'>fLeft</a></code> <code></code> and
returns true if constructed area is completely enclosed by <a href='#IRect'>IRect</a> area

### Parameters

<table>  <tr>    <td><a name='SkIRect_contains_x'><code><strong>x</strong></code></a></td>
    <td>test <a href='SkIPoint_Reference#IPoint'>IPoint</a> x</td>
  </tr>
  <tr>    <td><a name='SkIRect_contains_y'><code><strong>y</strong></code></a></td>
    <td>test <a href='SkIPoint_Reference#IPoint'>IPoint</a> y</td>
  </tr>
</table>

### Return Value

true if

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

<a href='#SkIRect_containsNoEmptyCheck'>containsNoEmptyCheck</a><sup><a href='#SkIRect_containsNoEmptyCheck_2'>[2]</a></sup> <a href='SkRect_Reference#SkRect_contains'>SkRect::contains</a><sup><a href='SkRect_Reference#SkRect_contains_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_contains_3'>[3]</a></sup>

---

<a name='SkIRect_contains_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_contains'>contains</a>(int32_t left
</pre>

Constructs <a href='#IRect'>IRect</a> to intersect from

### Parameters

<table>  <tr>    <td><a name='SkIRect_contains_2_left'><code><strong>left</strong></code></a></td>
    <td>x</td>
  </tr>
  <tr>    <td><a name='SkIRect_contains_2_top'><code><strong>top</strong></code></a></td>
    <td>y</td>
  </tr>
  <tr>    <td><a name='SkIRect_contains_2_right'><code><strong>right</strong></code></a></td>
    <td>x</td>
  </tr>
  <tr>    <td><a name='SkIRect_contains_2_bottom'><code><strong>bottom</strong></code></a></td>
    <td>y</td>
  </tr>
</table>

### Return Value

true if all sides of <a href='#IRect'>IRect</a> are outside construction

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

<a href='#SkIRect_containsNoEmptyCheck'>containsNoEmptyCheck</a><sup><a href='#SkIRect_containsNoEmptyCheck_2'>[2]</a></sup> <a href='SkRect_Reference#SkRect_contains'>SkRect::contains</a><sup><a href='SkRect_Reference#SkRect_contains_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_contains_3'>[3]</a></sup>

---

<a name='SkIRect_contains_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_contains'>contains</a>(const <a href='#SkIRect'>SkIRect</a>
</pre>

Returns true if <a href='#IRect'>IRect</a> contains <a href='#SkIRect_contains_3_r'>r</a>

### Parameters

<table>  <tr>    <td><a name='SkIRect_contains_3_r'><code><strong>r</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> contained</td>
  </tr>
</table>

### Return Value

true if all sides of <a href='#IRect'>IRect</a> are outside <a href='#SkIRect_contains_3_r'>r</a>

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

<a href='#SkIRect_containsNoEmptyCheck'>containsNoEmptyCheck</a><sup><a href='#SkIRect_containsNoEmptyCheck_2'>[2]</a></sup> <a href='SkRect_Reference#SkRect_contains'>SkRect::contains</a><sup><a href='SkRect_Reference#SkRect_contains_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_contains_3'>[3]</a></sup>

---

<a name='SkIRect_contains_4'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_contains'>contains</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Returns true if <a href='#IRect'>IRect</a> contains <a href='#SkIRect_contains_4_r'>r</a>

### Parameters

<table>  <tr>    <td><a name='SkIRect_contains_4_r'><code><strong>r</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> contained</td>
  </tr>
</table>

### Return Value

true if all sides of <a href='#IRect'>IRect</a> are outside <a href='#SkIRect_contains_4_r'>r</a>

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

<a href='#SkIRect_containsNoEmptyCheck'>containsNoEmptyCheck</a><sup><a href='#SkIRect_containsNoEmptyCheck_2'>[2]</a></sup> <a href='SkRect_Reference#SkRect_contains'>SkRect::contains</a><sup><a href='SkRect_Reference#SkRect_contains_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_contains_3'>[3]</a></sup>

---

<a name='SkIRect_containsNoEmptyCheck'></a>
## containsNoEmptyCheck

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_containsNoEmptyCheck'>containsNoEmptyCheck</a>(int32_t left
</pre>

Constructs <a href='#IRect'>IRect</a> from

### Parameters

<table>  <tr>    <td><a name='SkIRect_containsNoEmptyCheck_left'><code><strong>left</strong></code></a></td>
    <td>x</td>
  </tr>
  <tr>    <td><a name='SkIRect_containsNoEmptyCheck_top'><code><strong>top</strong></code></a></td>
    <td>y</td>
  </tr>
  <tr>    <td><a name='SkIRect_containsNoEmptyCheck_right'><code><strong>right</strong></code></a></td>
    <td>x</td>
  </tr>
  <tr>    <td><a name='SkIRect_containsNoEmptyCheck_bottom'><code><strong>bottom</strong></code></a></td>
    <td>y</td>
  </tr>
</table>

### Return Value

true if all sides of <a href='#IRect'>IRect</a> are outside construction

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

<a href='#SkIRect_contains'>contains</a><sup><a href='#SkIRect_contains_2'>[2]</a></sup><sup><a href='#SkIRect_contains_3'>[3]</a></sup><sup><a href='#SkIRect_contains_4'>[4]</a></sup> <a href='SkRect_Reference#SkRect_contains'>SkRect::contains</a><sup><a href='SkRect_Reference#SkRect_contains_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_contains_3'>[3]</a></sup>

---

<a name='SkIRect_containsNoEmptyCheck_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_containsNoEmptyCheck'>containsNoEmptyCheck</a>(const <a href='#SkIRect'>SkIRect</a>
</pre>

Returns true if <a href='#IRect'>IRect</a> contains construction

### Parameters

<table>  <tr>    <td><a name='SkIRect_containsNoEmptyCheck_2_r'><code><strong>r</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> contained</td>
  </tr>
</table>

### Return Value

true if all sides of <a href='#IRect'>IRect</a> are outside <a href='#SkIRect_containsNoEmptyCheck_2_r'>r</a>

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

<a href='#SkIRect_contains'>contains</a><sup><a href='#SkIRect_contains_2'>[2]</a></sup><sup><a href='#SkIRect_contains_3'>[3]</a></sup><sup><a href='#SkIRect_contains_4'>[4]</a></sup> <a href='SkRect_Reference#SkRect_contains'>SkRect::contains</a><sup><a href='SkRect_Reference#SkRect_contains_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_contains_3'>[3]</a></sup>

---

<a name='SkIRect_intersect'></a>
## intersect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_intersect'>intersect</a>(const <a href='#SkIRect'>SkIRect</a>
</pre>

Returns true if <a href='#IRect'>IRect</a> intersects <a href='#SkIRect_intersect_r'>r</a>

### Parameters

<table>  <tr>    <td><a name='SkIRect_intersect_r'><code><strong>r</strong></code></a></td>
    <td>limit of result</td>
  </tr>
</table>

### Return Value

true if <a href='#SkIRect_intersect_r'>r</a> and <a href='#IRect'>IRect</a> have area in common

### Example

<div><fiddle-embed name="ea233f5d5d1ae0e76fc6f2eb371c927a"><div>Two <a href='undocumented#SkDebugf'>SkDebugf</a> calls are required</div>

#### Example Output

~~~~
intersection: 30, 60, 50, 80
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_Intersects'>Intersects</a> <a href='#SkIRect_intersectNoEmptyCheck'>intersectNoEmptyCheck</a> <a href='#SkIRect_join'>join</a><sup><a href='#SkIRect_join_2'>[2]</a></sup> <a href='SkRect_Reference#SkRect_intersect'>SkRect::intersect</a><sup><a href='SkRect_Reference#SkRect_intersect_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_intersect_3'>[3]</a></sup>

---

<a name='SkIRect_intersect_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_intersect'>intersect</a>(const <a href='#SkIRect'>SkIRect</a>
</pre>

Returns true if <a href='#SkIRect_intersect_2_a'>a</a> intersects <a href='#SkIRect_intersect_2_b'>b</a>

### Parameters

<table>  <tr>    <td><a name='SkIRect_intersect_2_a'><code><strong>a</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> to intersect</td>
  </tr>
  <tr>    <td><a name='SkIRect_intersect_2_b'><code><strong>b</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> to intersect</td>
  </tr>
</table>

### Return Value

true if <a href='#SkIRect_intersect_2_a'>a</a> and <a href='#SkIRect_intersect_2_b'>b</a> have area in common

### Example

<div><fiddle-embed name="b2db0573aacf99ca52776c5522459d02">

#### Example Output

~~~~
intersection: 30, 60, 50, 80
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_Intersects'>Intersects</a> <a href='#SkIRect_intersectNoEmptyCheck'>intersectNoEmptyCheck</a> <a href='#SkIRect_join'>join</a><sup><a href='#SkIRect_join_2'>[2]</a></sup> <a href='SkRect_Reference#SkRect_intersect'>SkRect::intersect</a><sup><a href='SkRect_Reference#SkRect_intersect_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_intersect_3'>[3]</a></sup>

---

<a name='SkIRect_intersectNoEmptyCheck'></a>
## intersectNoEmptyCheck

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_intersectNoEmptyCheck'>intersectNoEmptyCheck</a>(const <a href='#SkIRect'>SkIRect</a>
</pre>

Returns true if <a href='#SkIRect_intersectNoEmptyCheck_a'>a</a> intersects <a href='#SkIRect_intersectNoEmptyCheck_b'>b</a>

### Parameters

<table>  <tr>    <td><a name='SkIRect_intersectNoEmptyCheck_a'><code><strong>a</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> to intersect</td>
  </tr>
  <tr>    <td><a name='SkIRect_intersectNoEmptyCheck_b'><code><strong>b</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> to intersect</td>
  </tr>
</table>

### Return Value

true if <a href='#SkIRect_intersectNoEmptyCheck_a'>a</a> and <a href='#SkIRect_intersectNoEmptyCheck_b'>b</a> have area in common

### Example

<div><fiddle-embed name="d35fbc9fdea71df8b8a12fd3da50d11c">

#### Example Output

~~~~
intersection: 30, 60, 50, 80
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_Intersects'>Intersects</a> <a href='#SkIRect_intersect'>intersect</a><sup><a href='#SkIRect_intersect_2'>[2]</a></sup><sup><a href='#SkIRect_intersect_3'>[3]</a></sup> <a href='#SkIRect_join'>join</a><sup><a href='#SkIRect_join_2'>[2]</a></sup> <a href='SkRect_Reference#SkRect_intersect'>SkRect::intersect</a><sup><a href='SkRect_Reference#SkRect_intersect_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_intersect_3'>[3]</a></sup>

---

<a name='SkIRect_intersect_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_intersect'>intersect</a>(int32_t left
</pre>

Constructs <a href='#IRect'>IRect</a> to intersect from

### Parameters

<table>  <tr>    <td><a name='SkIRect_intersect_3_left'><code><strong>left</strong></code></a></td>
    <td>x</td>
  </tr>
  <tr>    <td><a name='SkIRect_intersect_3_top'><code><strong>top</strong></code></a></td>
    <td>y</td>
  </tr>
  <tr>    <td><a name='SkIRect_intersect_3_right'><code><strong>right</strong></code></a></td>
    <td>x</td>
  </tr>
  <tr>    <td><a name='SkIRect_intersect_3_bottom'><code><strong>bottom</strong></code></a></td>
    <td>y</td>
  </tr>
</table>

### Return Value

true if construction and <a href='#IRect'>IRect</a> have area in common

### Example

<div><fiddle-embed name="200422990eded2f754ab9893118f2645"><div>Two <a href='undocumented#SkDebugf'>SkDebugf</a> calls are required</div>

#### Example Output

~~~~
intersection: 30, 60, 50, 80
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_intersectNoEmptyCheck'>intersectNoEmptyCheck</a> <a href='#SkIRect_Intersects'>Intersects</a> <a href='#SkIRect_join'>join</a><sup><a href='#SkIRect_join_2'>[2]</a></sup> <a href='SkRect_Reference#SkRect_intersect'>SkRect::intersect</a><sup><a href='SkRect_Reference#SkRect_intersect_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_intersect_3'>[3]</a></sup>

---

<a name='SkIRect_Intersects'></a>
## Intersects

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkIRect_Intersects'>Intersects</a>(const <a href='#SkIRect'>SkIRect</a>
</pre>

Returns true if <a href='#SkIRect_Intersects_a'>a</a> intersects <a href='#SkIRect_Intersects_b'>b</a>

### Parameters

<table>  <tr>    <td><a name='SkIRect_Intersects_a'><code><strong>a</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> to intersect</td>
  </tr>
  <tr>    <td><a name='SkIRect_Intersects_b'><code><strong>b</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> to intersect</td>
  </tr>
</table>

### Return Value

true if <a href='#SkIRect_Intersects_a'>a</a> and <a href='#SkIRect_Intersects_b'>b</a> have area in common

### Example

<div><fiddle-embed name="0c67cf8981389efc7108369fb9b7976b">

#### Example Output

~~~~
intersection
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_IntersectsNoEmptyCheck'>IntersectsNoEmptyCheck</a> <a href='#SkIRect_intersect'>intersect</a><sup><a href='#SkIRect_intersect_2'>[2]</a></sup><sup><a href='#SkIRect_intersect_3'>[3]</a></sup> <a href='SkRect_Reference#SkRect_intersect'>SkRect::intersect</a><sup><a href='SkRect_Reference#SkRect_intersect_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_intersect_3'>[3]</a></sup>

---

<a name='SkIRect_IntersectsNoEmptyCheck'></a>
## IntersectsNoEmptyCheck

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkIRect_IntersectsNoEmptyCheck'>IntersectsNoEmptyCheck</a>(const <a href='#SkIRect'>SkIRect</a>
</pre>

Returns true if <a href='#SkIRect_IntersectsNoEmptyCheck_a'>a</a> intersects <a href='#SkIRect_IntersectsNoEmptyCheck_b'>b</a>

### Parameters

<table>  <tr>    <td><a name='SkIRect_IntersectsNoEmptyCheck_a'><code><strong>a</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> to intersect</td>
  </tr>
  <tr>    <td><a name='SkIRect_IntersectsNoEmptyCheck_b'><code><strong>b</strong></code></a></td>
    <td><a href='#IRect'>IRect</a> to intersect</td>
  </tr>
</table>

### Return Value

true if <a href='#SkIRect_IntersectsNoEmptyCheck_a'>a</a> and <a href='#SkIRect_IntersectsNoEmptyCheck_b'>b</a> have area in common

### Example

<div><fiddle-embed name="dba234d15162fb5b26e1a96529ca6a2a">

#### Example Output

~~~~
intersection
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_Intersects'>Intersects</a> <a href='#SkIRect_intersect'>intersect</a><sup><a href='#SkIRect_intersect_2'>[2]</a></sup><sup><a href='#SkIRect_intersect_3'>[3]</a></sup> <a href='SkRect_Reference#SkRect_intersect'>SkRect::intersect</a><sup><a href='SkRect_Reference#SkRect_intersect_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_intersect_3'>[3]</a></sup>

---

## <a name='Join'>Join</a>

<a name='SkIRect_join'></a>
## join

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_join'>join</a>(int32_t left
</pre>

Constructs <a href='#IRect'>IRect</a> to intersect from

### Parameters

<table>  <tr>    <td><a name='SkIRect_join_left'><code><strong>left</strong></code></a></td>
    <td>x</td>
  </tr>
  <tr>    <td><a name='SkIRect_join_top'><code><strong>top</strong></code></a></td>
    <td>y</td>
  </tr>
  <tr>    <td><a name='SkIRect_join_right'><code><strong>right</strong></code></a></td>
    <td>x</td>
  </tr>
  <tr>    <td><a name='SkIRect_join_bottom'><code><strong>bottom</strong></code></a></td>
    <td>y</td>
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

<a href='#SkIRect_set'>set</a> <a href='SkRect_Reference#SkRect_join'>SkRect::join</a><sup><a href='SkRect_Reference#SkRect_join_2'>[2]</a></sup>

---

<a name='SkIRect_join_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_join'>join</a>(const <a href='#SkIRect'>SkIRect</a>
</pre>

Sets <a href='#IRect'>IRect</a> to the union of itself and <a href='#SkIRect_join_2_r'>r</a>

### Parameters

<table>  <tr>    <td><a name='SkIRect_join_2_r'><code><strong>r</strong></code></a></td>
    <td>expansion <a href='#IRect'>IRect</a></td>
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

<a href='#SkIRect_set'>set</a> <a href='SkRect_Reference#SkRect_join'>SkRect::join</a><sup><a href='SkRect_Reference#SkRect_join_2'>[2]</a></sup>

---

## <a name='Sorting'>Sorting</a>

<a name='SkIRect_sort'></a>
## sort

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_sort'>sort</a>(
</pre>

Swaps <a href='#SkIRect_fLeft'>fLeft</a> and <a href='#SkIRect_fRight'>fRight</a> if <a href='#SkIRect_fLeft'>fLeft</a> is greater than <a href='#SkIRect_fRight'>fRight</a>

### Example

<div><fiddle-embed name="fa12547fcfd4c1aef3db1a1f6aae0fe4">

#### Example Output

~~~~
rect: 30, 50, 20, 10
sorted: 20, 10, 30, 50
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_makeSorted'>makeSorted</a> <a href='SkRect_Reference#SkRect_sort'>SkRect::sort</a>

---

<a name='SkIRect_makeSorted'></a>
## makeSorted

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkIRect'>SkIRect</a> <a href='#SkIRect_makeSorted'>makeSorted</a>(
</pre>

Returns <a href='#IRect'>IRect</a> with <a href='#SkIRect_fLeft'>fLeft</a> and <a href='#SkIRect_fRight'>fRight</a> swapped if <a href='#SkIRect_fLeft'>fLeft</a> is greater than <a href='#SkIRect_fRight'>fRight</a>

### Return Value

sorted <a href='#IRect'>IRect</a>

### Example

<div><fiddle-embed name="de89926c374aa16427916900b89a3441">

#### Example Output

~~~~
rect: 30, 50, 20, 10
sorted: 20, 10, 30, 50
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_sort'>sort</a> <a href='SkRect_Reference#SkRect_makeSorted'>SkRect::makeSorted</a>

---

<a name='SkIRect_EmptyIRect'></a>
## EmptyIRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static const <a href='#SkIRect'>SkIRect</a>
</pre>

Returns a reference to immutable empty <a href='#IRect'>IRect</a>

### Return Value

global <a href='#IRect'>IRect</a> set to all zeroes

### Example

<div><fiddle-embed name="65e0b9b52e907902630577941fb3ed6d">

#### Example Output

~~~~
rect: 0, 0, 0, 0
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_MakeEmpty'>MakeEmpty</a>

---

<a name='SkIRect_MakeLargest'></a>
## MakeLargest

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#SkIRect'>SkIRect</a> <a href='#SkIRect_MakeLargest'>MakeLargest</a>(
</pre>

Deprecated.

---

