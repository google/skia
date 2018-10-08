SkRRect Reference
===

<a name='SkRRect'></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='#SkRRect'>SkRRect</a> {
public:
    <a href='#SkRRect_empty_constructor'>SkRRect()</a> = default;
    <a href='#SkRRect_copy_const_SkRRect'>SkRRect(const SkRRect& rrect)</a> = default;
    <a href='#SkRRect'>SkRRect</a>& <a href='#SkRRect_copy_operator'>operator=(const SkRRect& rrect)</a> = default;

    enum <a href='#SkRRect_Type'>Type</a> {
        <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>,
        <a href='#SkRRect_kRect_Type'>kRect_Type</a>,
        <a href='#SkRRect_kOval_Type'>kOval_Type</a>,
        <a href='#SkRRect_kSimple_Type'>kSimple_Type</a>,
        <a href='#SkRRect_kNinePatch_Type'>kNinePatch_Type</a>,
        <a href='#SkRRect_kComplex_Type'>kComplex_Type</a>,
        <a href='#SkRRect_kLastType'>kLastType</a>       = <a href='#SkRRect_kComplex_Type'>kComplex_Type</a>,
    };

    <a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_getType'>getType</a>() const;
    <a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_type'>type</a>() const;
    bool <a href='#SkRRect_isEmpty'>isEmpty</a>() const;
    bool <a href='#SkRRect_isRect'>isRect</a>() const;
    bool <a href='#SkRRect_isOval'>isOval</a>() const;
    bool <a href='#SkRRect_isSimple'>isSimple</a>() const;
    bool <a href='#SkRRect_isNinePatch'>isNinePatch</a>() const;
    bool <a href='#SkRRect_isComplex'>isComplex</a>() const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRRect_width'>width</a>() const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRRect_height'>height</a>() const;
    <a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='#SkRRect_getSimpleRadii'>getSimpleRadii</a>() const;
    void <a href='#SkRRect_setEmpty'>setEmpty</a>();
    void <a href='#SkRRect_setRect'>setRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& rect);
    static <a href='#SkRRect'>SkRRect</a> <a href='#SkRRect_MakeEmpty'>MakeEmpty</a>();
    static <a href='#SkRRect'>SkRRect</a> <a href='#SkRRect_MakeRect'>MakeRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& r);
    static <a href='#SkRRect'>SkRRect</a> <a href='#SkRRect_MakeOval'>MakeOval</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& oval);
    static <a href='#SkRRect'>SkRRect</a> <a href='#SkRRect_MakeRectXY'>MakeRectXY</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& rect, <a href='undocumented#SkScalar'>SkScalar</a> xRad, <a href='undocumented#SkScalar'>SkScalar</a> yRad);
    void <a href='#SkRRect_setOval'>setOval</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& oval);
    void <a href='#SkRRect_setRectXY'>setRectXY</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& rect, <a href='undocumented#SkScalar'>SkScalar</a> xRad, <a href='undocumented#SkScalar'>SkScalar</a> yRad);
    void <a href='#SkRRect_setNinePatch'>setNinePatch</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& rect, <a href='undocumented#SkScalar'>SkScalar</a> leftRad, <a href='undocumented#SkScalar'>SkScalar</a> topRad,
                      <a href='undocumented#SkScalar'>SkScalar</a> rightRad, <a href='undocumented#SkScalar'>SkScalar</a> bottomRad);
    void <a href='#SkRRect_setRectRadii'>setRectRadii</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& rect, const <a href='SkPoint_Reference#SkVector'>SkVector</a> radii[4]);

    enum <a href='#SkRRect_Corner'>Corner</a> {
        <a href='#SkRRect_kUpperLeft_Corner'>kUpperLeft_Corner</a>,
        <a href='#SkRRect_kUpperRight_Corner'>kUpperRight_Corner</a>,
        <a href='#SkRRect_kLowerRight_Corner'>kLowerRight_Corner</a>,
        <a href='#SkRRect_kLowerLeft_Corner'>kLowerLeft_Corner</a>,
    };

    const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkRRect_rect'>rect</a>() const;
    <a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='#SkRRect_radii'>radii</a>(<a href='#SkRRect_Corner'>Corner</a> corner) const;
    const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkRRect_getBounds'>getBounds</a>() const;
    friend bool <a href='#SkRRect_equal_operator'>operator==(const SkRRect& a, const SkRRect& b)</a>;
    friend bool <a href='#SkRRect_notequal_operator'>operator!=(const SkRRect& a, const SkRRect& b)</a>;
    void <a href='#SkRRect_inset'>inset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy, <a href='#SkRRect'>SkRRect</a>* dst) const;
    void <a href='#SkRRect_inset_2'>inset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy);
    void <a href='#SkRRect_outset'>outset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy, <a href='#SkRRect'>SkRRect</a>* dst) const;
    void <a href='#SkRRect_outset_2'>outset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy);
    void <a href='#SkRRect_offset'>offset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy);
    <a href='#SkRRect'>SkRRect</a> <a href='#SkRRect_makeOffset'>makeOffset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy) const;
    bool <a href='#SkRRect_contains'>contains</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& rect) const;
    bool <a href='#SkRRect_isValid'>isValid</a>() const;

    static constexpr size_t <a href='#SkRRect_kSizeInMemory'>kSizeInMemory</a> = 12 * sizeof(SkScalar);

    size_t <a href='#SkRRect_writeToMemory'>writeToMemory</a>(void* buffer) const;
    size_t <a href='#SkRRect_readFromMemory'>readFromMemory</a>(const void* buffer, size_t length);
    bool <a href='#SkRRect_transform'>transform</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& matrix, <a href='#SkRRect'>SkRRect</a>* dst) const;
    void <a href='#SkRRect_dump'>dump</a>(bool asHex) const;
    void <a href='#SkRRect_dump_2'>dump</a>() const;
    void <a href='#SkRRect_dumpHex'>dumpHex</a>() const;
};
</pre>

<a href='#SkRRect'>SkRRect</a> describes a rounded rectangle with a bounds and a pair of radii for each corner

<a name='SkRRect_empty_constructor'></a>
## SkRRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkRRect'>SkRRect</a>(
</pre>

Initializes bounds at

### Return Value

empty <a href='#RRect'>Round Rect</a>

### Example

<div><fiddle-embed name="471e7aad0feaf9ec3a21757a317a64f5"></fiddle-embed></div>

### See Also

<a href='#SkRRect_setEmpty'>setEmpty</a> <a href='#SkRRect_isEmpty'>isEmpty</a>

---

<a name='SkRRect_copy_const_SkRRect'></a>
## SkRRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkRRect'>SkRRect</a>(const <a href='#SkRRect'>SkRRect</a>
</pre>

Initializes to copy of <a href='#SkRRect_copy_const_SkRRect_rrect'>rrect</a> bounds and corner radii

### Parameters

<table>  <tr>    <td><a name='SkRRect_copy_const_SkRRect_rrect'><code><strong>rrect</strong></code></a></td>
    <td>bounds and corner to copy</td>
  </tr>
</table>

### Return Value

copy of <a href='#SkRRect_copy_const_SkRRect_rrect'>rrect</a>

### Example

<div><fiddle-embed name="ad8f5d49edfcee60eddfe2a955b6c5f5"></fiddle-embed></div>

### See Also

<a href='#SkRRect_copy_operator'>operator=(const SkRRect& rrect)</a> <a href='#SkRRect_MakeRect'>MakeRect</a>

---

<a name='SkRRect_copy_operator'></a>
## operator=

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkRRect'>SkRRect</a>
</pre>

Copies <a href='#SkRRect_copy_operator_rrect'>rrect</a> bounds and corner radii

### Parameters

<table>  <tr>    <td><a name='SkRRect_copy_operator_rrect'><code><strong>rrect</strong></code></a></td>
    <td>bounds and corner to copy</td>
  </tr>
</table>

### Return Value

copy of <a href='#SkRRect_copy_operator_rrect'>rrect</a>

### Example

<div><fiddle-embed name="52926c98c1cca00606d3ea99f23fea3d"></fiddle-embed></div>

### See Also

<a href='#SkRRect_copy_const_SkRRect'>SkRRect(const SkRRect& rrect)</a> <a href='#SkRRect_MakeRect'>MakeRect</a>

---

## <a name='Type'>Type</a>

## <a name='SkRRect_Type'>Enum SkRRect::Type</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkRRect_Type'>Type</a> {
        <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>,
        <a href='#SkRRect_kRect_Type'>kRect_Type</a>,
        <a href='#SkRRect_kOval_Type'>kOval_Type</a>,
        <a href='#SkRRect_kSimple_Type'>kSimple_Type</a>,
        <a href='#SkRRect_kNinePatch_Type'>kNinePatch_Type</a>,
        <a href='#SkRRect_kComplex_Type'>kComplex_Type</a>,
        <a href='#SkRRect_kLastType'>kLastType</a> = <a href='#SkRRect_kComplex_Type'>kComplex_Type</a>,
    };
</pre>

<a href='#SkRRect_Type'>Type</a> describes possible specializations of <a href='#RRect'>Round Rect</a>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kEmpty_Type'><code>SkRRect::kEmpty_Type</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#RRect'>Round Rect</a> has zero width or height</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kRect_Type'><code>SkRRect::kRect_Type</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#RRect'>Round Rect</a> has width and height</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kOval_Type'><code>SkRRect::kOval_Type</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#RRect'>Round Rect</a> has width and height</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kSimple_Type'><code>SkRRect::kSimple_Type</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#RRect'>Round Rect</a> has width and height</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kNinePatch_Type'><code>SkRRect::kNinePatch_Type</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#RRect'>Round Rect</a> has width and height</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kComplex_Type'><code>SkRRect::kComplex_Type</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>5</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
both radii are non</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kLastType'><code>SkRRect::kLastType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>5</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
largest Type value</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="38a559936ea7c8d482098c189ee5c9b8"></fiddle-embed></div>

### See Also

<a href='SkRect_Reference#Rect'>Rect</a> <a href='SkPath_Reference#Path'>Path</a>

<a name='SkRRect_getType'></a>
## getType

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_getType'>getType</a>(
</pre>

Returns <a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_kEmpty_Type'>kEmpty Type</a>

### Return Value

<a href='#SkRRect_Type'>Type</a>

### Example

<div><fiddle-embed name="ace8f4aebf90527d43e4b7291375c9ad"><div>rrect2 is not a <a href='SkRect_Reference#Rect'>Rect</a></div></fiddle-embed></div>

### See Also

<a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_type'>type</a>

---

<a name='SkRRect_type'></a>
## type

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_type'>type</a>(
</pre>

Returns <a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_kEmpty_Type'>kEmpty Type</a>

### Return Value

<a href='#SkRRect_Type'>Type</a>

### Example

<div><fiddle-embed name="1080805c8449406a4e26d694bc56d2dc"><div><a href='#SkRRect_inset'>inset</a>(</div></fiddle-embed></div>

### See Also

<a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_getType'>getType</a>

---

<a name='SkRRect_isEmpty'></a>
## isEmpty

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_isEmpty'>isEmpty</a>(
</pre>

Returns true if <a href='#SkRRect_rect'>rect</a>(

### Return Value

true if <a href='#SkRRect_width'>width</a>(

### Example

<div><fiddle-embed name="3afe4ea247923e06326aeb2b165c7485"></fiddle-embed></div>

### See Also

<a href='SkRect_Reference#SkRect_isEmpty'>SkRect::isEmpty</a> <a href='#SkRRect_height'>height</a> <a href='#SkRRect_width'>width</a>

---

<a name='SkRRect_isRect'></a>
## isRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_isRect'>isRect</a>(
</pre>

Returns true if not empty

### Return Value

true if not empty

### Example

<div><fiddle-embed name="e2dcdad0e9cb7ba3e78a9871e9229753"></fiddle-embed></div>

### See Also

<a href='#SkRRect_isEmpty'>isEmpty</a> <a href='#SkRRect_radii'>radii</a>

---

<a name='SkRRect_isOval'></a>
## isOval

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_isOval'>isOval</a>(
</pre>

Returns true if not empty

### Return Value

true if has identical geometry to <a href='undocumented#Oval'>Oval</a>

### Example

<div><fiddle-embed name="ab9b3aef7896aee80b780789848fbba4"><div>The first radii are scaled down proportionately until both x</div></fiddle-embed></div>

### See Also

<a href='#SkRRect_isEmpty'>isEmpty</a> <a href='#SkRRect_isSimple'>isSimple</a> <a href='SkCanvas_Reference#SkCanvas_drawOval'>SkCanvas::drawOval</a>

---

<a name='SkRRect_isSimple'></a>
## isSimple

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_isSimple'>isSimple</a>(
</pre>

Returns true if not empty

### Return Value

true if not empty

### Example

<div><fiddle-embed name="65bbb109483ed79edb32027cf71851eb"></fiddle-embed></div>

### See Also

<a href='#SkRRect_isEmpty'>isEmpty</a> <a href='#SkRRect_isRect'>isRect</a> <a href='#SkRRect_isOval'>isOval</a> <a href='#SkRRect_isNinePatch'>isNinePatch</a>

---

<a name='SkRRect_isNinePatch'></a>
## isNinePatch

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_isNinePatch'>isNinePatch</a>(
</pre>

Returns true if <a href='#SkRRect_isEmpty'>isEmpty</a>

### Return Value

true if not empty

### Example

<div><fiddle-embed name="568cb730e66d0df09a7d9bd9d6142c9e"></fiddle-embed></div>

### See Also

<a href='#SkRRect_isEmpty'>isEmpty</a> <a href='#SkRRect_isRect'>isRect</a> <a href='#SkRRect_isOval'>isOval</a> <a href='#SkRRect_isSimple'>isSimple</a> <a href='#SkRRect_isComplex'>isComplex</a>

---

<a name='SkRRect_isComplex'></a>
## isComplex

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_isComplex'>isComplex</a>(
</pre>

Returns true if <a href='#SkRRect_isEmpty'>isEmpty</a>

### Return Value

true if not empty

### Example

<div><fiddle-embed name="e4ba9346ee5c2d37d5e504f8cc678544"></fiddle-embed></div>

### See Also

<a href='#SkRRect_isEmpty'>isEmpty</a> <a href='#SkRRect_isRect'>isRect</a> <a href='#SkRRect_isOval'>isOval</a> <a href='#SkRRect_isSimple'>isSimple</a> <a href='#SkRRect_isNinePatch'>isNinePatch</a>

---

<a name='SkRRect_width'></a>
## width

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRRect_width'>width</a>(
</pre>

Returns span on the x

### Return Value

bounds()

### Example

<div><fiddle-embed name="c675a480b41dee157f84fa2550a2a53c"><div><a href='#SkRRect_MakeRect'>SkRRect::MakeRect</a> sorts its input</div>

#### Example Output

~~~~
unsorted width: 5
large width: inf
~~~~

</fiddle-embed></div>

### See Also

<a href='SkRect_Reference#SkRect_width'>SkRect::width</a> <a href='#SkRRect_height'>height</a> <a href='#SkRRect_getBounds'>getBounds</a>

---

<a name='SkRRect_height'></a>
## height

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRRect_height'>height</a>(
</pre>

Returns span on the y

### Return Value

bounds()

### Example

<div><fiddle-embed name="5a3eb1755164a7becec33cec6e6eca31"><div><a href='#SkRRect_MakeRect'>SkRRect::MakeRect</a> sorts its input</div>

#### Example Output

~~~~
unsorted height: 5
large height: inf
~~~~

</fiddle-embed></div>

### See Also

<a href='SkRect_Reference#SkRect'>SkRect</a>

---

<a name='SkRRect_getSimpleRadii'></a>
## getSimpleRadii

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='#SkRRect_getSimpleRadii'>getSimpleRadii</a>(
</pre>

Returns top

### Return Value

corner radii for simple types

### Example

<div><fiddle-embed name="ebcc50ed30240e94de8439d21dd8171c"></fiddle-embed></div>

### See Also

<a href='#SkRRect_radii'>radii</a> <a href='#SkRRect_getBounds'>getBounds</a> <a href='#SkRRect_getType'>getType</a> <a href='#SkRRect_isSimple'>isSimple</a>

---

<a name='SkRRect_setEmpty'></a>
## setEmpty

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_setEmpty'>setEmpty</a>(
</pre>

Sets bounds to zero width and height at

### Example

<div><fiddle-embed name="44e9a9c2c5ef1af2a616086ff46a9037"><div>Nothing blue is drawn because <a href='#RRect'>Round Rect</a> is set to empty</div></fiddle-embed></div>

### See Also

<a href='#SkRRect_MakeEmpty'>MakeEmpty</a> <a href='#SkRRect_setRect'>setRect</a>

---

<a name='SkRRect_setRect'></a>
## setRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_setRect'>setRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Sets bounds to sorted rect

### Parameters

<table>  <tr>    <td><a name='SkRRect_setRect_rect'><code><strong>rect</strong></code></a></td>
    <td>bounds to set</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3afc3ac9bebd1d7387822cc608571e82"></fiddle-embed></div>

### See Also

<a href='#SkRRect_MakeRect'>MakeRect</a> <a href='#SkRRect_setRectXY'>setRectXY</a>

---

<a name='SkRRect_MakeEmpty'></a>
## MakeEmpty

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#SkRRect'>SkRRect</a> <a href='#SkRRect_MakeEmpty'>MakeEmpty</a>(
</pre>

Initializes bounds at

### Return Value

empty <a href='#RRect'>Round Rect</a>

### Example

<div><fiddle-embed name="c6c6be3b3c137226adbb5b5af9203d27"></fiddle-embed></div>

### See Also

<a href='#SkRRect_empty_constructor'>SkRRect()</a> <a href='SkRect_Reference#SkRect_MakeEmpty'>SkRect::MakeEmpty</a>

---

<a name='SkRRect_MakeRect'></a>
## MakeRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#SkRRect'>SkRRect</a> <a href='#SkRRect_MakeRect'>MakeRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Initializes to copy of <a href='#SkRRect_MakeRect_r'>r</a> bounds and zeroes corner radii

### Parameters

<table>  <tr>    <td><a name='SkRRect_MakeRect_r'><code><strong>r</strong></code></a></td>
    <td>bounds to copy</td>
  </tr>
</table>

### Return Value

copy of <a href='#SkRRect_MakeRect_r'>r</a>

### Example

<div><fiddle-embed name="5295b07fe4d2cdcd077979a9e19854d9"></fiddle-embed></div>

### See Also

<a href='#SkRRect_setRect'>setRect</a> <a href='#SkRRect_MakeOval'>MakeOval</a> <a href='#SkRRect_MakeRectXY'>MakeRectXY</a>

---

<a name='SkRRect_MakeOval'></a>
## MakeOval

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#SkRRect'>SkRRect</a> <a href='#SkRRect_MakeOval'>MakeOval</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Sets bounds to <a href='#SkRRect_MakeOval_oval'>oval</a>

### Parameters

<table>  <tr>    <td><a name='SkRRect_MakeOval_oval'><code><strong>oval</strong></code></a></td>
    <td>bounds of <a href='undocumented#Oval'>Oval</a></td>
  </tr>
</table>

### Return Value

<a href='undocumented#Oval'>Oval</a>

### Example

<div><fiddle-embed name="0b99ee38fd154f769f6031242e02fa7a"></fiddle-embed></div>

### See Also

<a href='#SkRRect_setOval'>setOval</a> <a href='#SkRRect_MakeRect'>MakeRect</a> <a href='#SkRRect_MakeRectXY'>MakeRectXY</a>

---

<a name='SkRRect_MakeRectXY'></a>
## MakeRectXY

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#SkRRect'>SkRRect</a> <a href='#SkRRect_MakeRectXY'>MakeRectXY</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Sets to rounded rectangle with the same radii for all four corners

### Parameters

<table>  <tr>    <td><a name='SkRRect_MakeRectXY_rect'><code><strong>rect</strong></code></a></td>
    <td>bounds of rounded rectangle</td>
  </tr>
  <tr>    <td><a name='SkRRect_MakeRectXY_xRad'><code><strong>xRad</strong></code></a></td>
    <td>x</td>
  </tr>
  <tr>    <td><a name='SkRRect_MakeRectXY_yRad'><code><strong>yRad</strong></code></a></td>
    <td>y</td>
  </tr>
</table>

### Return Value

rounded rectangle

### Example

<div><fiddle-embed name="2b24a1247637cbc94f8b3c77d37ed3e2"></fiddle-embed></div>

### See Also

<a href='#SkRRect_setRectXY'>setRectXY</a>

---

<a name='SkRRect_setOval'></a>
## setOval

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_setOval'>setOval</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Sets bounds to <a href='#SkRRect_setOval_oval'>oval</a>

### Parameters

<table>  <tr>    <td><a name='SkRRect_setOval_oval'><code><strong>oval</strong></code></a></td>
    <td>bounds of <a href='undocumented#Oval'>Oval</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="cf418af29cbab6243ac16aacd1217ffe"></fiddle-embed></div>

### See Also

<a href='#SkRRect_MakeOval'>MakeOval</a>

---

<a name='SkRRect_setRectXY'></a>
## setRectXY

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_setRectXY'>setRectXY</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Sets to rounded rectangle with the same radii for all four corners

### Parameters

<table>  <tr>    <td><a name='SkRRect_setRectXY_rect'><code><strong>rect</strong></code></a></td>
    <td>bounds of rounded rectangle</td>
  </tr>
  <tr>    <td><a name='SkRRect_setRectXY_xRad'><code><strong>xRad</strong></code></a></td>
    <td>x</td>
  </tr>
  <tr>    <td><a name='SkRRect_setRectXY_yRad'><code><strong>yRad</strong></code></a></td>
    <td>y</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6ac569e40fb68c758319e85428b9ae95"></fiddle-embed></div>

### See Also

<a href='#SkRRect_MakeRectXY'>MakeRectXY</a> <a href='SkPath_Reference#SkPath_addRoundRect'>SkPath::addRoundRect</a><sup><a href='SkPath_Reference#SkPath_addRoundRect_2'>[2]</a></sup>

---

<a name='SkRRect_setNinePatch'></a>
## setNinePatch

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_setNinePatch'>setNinePatch</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Sets bounds to rect

### Parameters

<table>  <tr>    <td><a name='SkRRect_setNinePatch_rect'><code><strong>rect</strong></code></a></td>
    <td>bounds of rounded rectangle</td>
  </tr>
  <tr>    <td><a name='SkRRect_setNinePatch_leftRad'><code><strong>leftRad</strong></code></a></td>
    <td>left</td>
  </tr>
  <tr>    <td><a name='SkRRect_setNinePatch_topRad'><code><strong>topRad</strong></code></a></td>
    <td>left</td>
  </tr>
  <tr>    <td><a name='SkRRect_setNinePatch_rightRad'><code><strong>rightRad</strong></code></a></td>
    <td>right</td>
  </tr>
  <tr>    <td><a name='SkRRect_setNinePatch_bottomRad'><code><strong>bottomRad</strong></code></a></td>
    <td>left</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c4620df2eaba447b581688d3100053b1"></fiddle-embed></div>

### See Also

<a href='#SkRRect_setRectRadii'>setRectRadii</a>

---

<a name='SkRRect_setRectRadii'></a>
## setRectRadii

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_setRectRadii'>setRectRadii</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Sets bounds to rect

### Parameters

<table>  <tr>    <td><a name='SkRRect_setRectRadii_rect'><code><strong>rect</strong></code></a></td>
    <td>bounds of rounded rectangle</td>
  </tr>
  <tr>    <td><a name='SkRRect_setRectRadii_radii'><code><strong>radii</strong></code></a></td>
    <td>corner x</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="340d6c51efaa1f7f3d0dcaf8b0e90696"></fiddle-embed></div>

### See Also

<a href='#SkRRect_setNinePatch'>setNinePatch</a> <a href='SkPath_Reference#SkPath_addRoundRect'>SkPath::addRoundRect</a><sup><a href='SkPath_Reference#SkPath_addRoundRect_2'>[2]</a></sup>

---

## <a name='SkRRect_Corner'>Enum SkRRect::Corner</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkRRect_Corner'>Corner</a> {
        <a href='#SkRRect_kUpperLeft_Corner'>kUpperLeft_Corner</a>,
        <a href='#SkRRect_kUpperRight_Corner'>kUpperRight_Corner</a>,
        <a href='#SkRRect_kLowerRight_Corner'>kLowerRight_Corner</a>,
        <a href='#SkRRect_kLowerLeft_Corner'>kLowerLeft_Corner</a>,
    };
</pre>

The radii are stored

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kUpperLeft_Corner'><code>SkRRect::kUpperLeft_Corner</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
index of top-left corner radii</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kUpperRight_Corner'><code>SkRRect::kUpperRight_Corner</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
index of top-right corner radii</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kLowerRight_Corner'><code>SkRRect::kLowerRight_Corner</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
index of bottom-right corner radii</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kLowerLeft_Corner'><code>SkRRect::kLowerLeft_Corner</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
index of bottom-left corner radii</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="9205393f30b156e1507e88aa27f1dd91"></fiddle-embed></div>

### See Also

<a href='#SkRRect_radii'>radii</a>

<a name='SkRRect_rect'></a>
## rect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Returns bounds

### Return Value

bounding box

### Example

<div><fiddle-embed name="6831adf4c536047f4709c686feb10c48">

#### Example Output

~~~~
left bounds: (nan) 0
left bounds: (inf) 0
left bounds: (100) 60
left bounds: (50) 50
left bounds: (25) 25
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRRect_getBounds'>getBounds</a>

---

<a name='SkRRect_radii'></a>
## radii

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='#SkRRect_radii'>radii</a>(<a href='#SkRRect_Corner'>Corner</a> corner
</pre>

Returns <a href='undocumented#Scalar'>Scalar</a> pair for radius of curve on x

### Parameters

<table>  <tr>    <td><a name='SkRRect_radii_corner'><code><strong>corner</strong></code></a></td>
    <td>one of</td>
  </tr>
</table>

### Return Value

x

### Example

<div><fiddle-embed name="8d5c88478528584913867ada423e0d59"><div>Finite values are scaled proportionately to fit</div>

#### Example Output

~~~~
left corner: (nan) 0
left corner: (inf) 0
left corner: (100) 25
left corner: (50) 25
left corner: (25) 12.5
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRRect_Corner'>Corner</a>

---

<a name='SkRRect_getBounds'></a>
## getBounds

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Returns bounds

### Return Value

bounding box

### Example

<div><fiddle-embed name="4577e2dcb086b241bb43d8b89ee0b0dd"></fiddle-embed></div>

### See Also

<a href='#SkRRect_rect'>rect</a>

---

<a name='SkRRect_equal_operator'></a>
## operator==

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_equal_operator'>operator==(const SkRRect& a, const SkRRect& b)</a>
</pre>

Returns true if bounds and radii in <a href='#SkRRect_equal_operator_a'>a</a> are equal to bounds and radii in <a href='#SkRRect_equal_operator_b'>b</a>

### Parameters

<table>  <tr>    <td><a name='SkRRect_equal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> bounds and radii to compare</td>
  </tr>
  <tr>    <td><a name='SkRRect_equal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> bounds and radii to compare</td>
  </tr>
</table>

### Return Value

true if members are equal

### Example

<div><fiddle-embed name="df181af37f1d2b06f0f45af73df7b47d"></fiddle-embed></div>

### See Also

<a href='#SkRRect_notequal_operator'>operator!=(const SkRRect& a, const SkRRect& b)</a>

---

<a name='SkRRect_notequal_operator'></a>
## operator!=

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_notequal_operator'>operator!=(const SkRRect& a, const SkRRect& b)</a>
</pre>

Returns true if bounds and radii in <a href='#SkRRect_notequal_operator_a'>a</a> are not equal to bounds and radii in <a href='#SkRRect_notequal_operator_b'>b</a>

### Parameters

<table>  <tr>    <td><a name='SkRRect_notequal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> bounds and radii to compare</td>
  </tr>
  <tr>    <td><a name='SkRRect_notequal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> bounds and radii to compare</td>
  </tr>
</table>

### Return Value

true if members are not equal

### Example

<div><fiddle-embed name="505e47b3e6474ebdecdc04c3c2af2c34"></fiddle-embed></div>

### See Also

<a href='#SkRRect_equal_operator'>operator==(const SkRRect& a, const SkRRect& b)</a>

---

<a name='SkRRect_inset'></a>
## inset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_inset'>inset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx
</pre>

Copies <a href='#RRect'>Round Rect</a> to <a href='#SkRRect_inset_dst'>dst</a>

### Parameters

<table>  <tr>    <td><a name='SkRRect_inset_dx'><code><strong>dx</strong></code></a></td>
    <td>added to <a href='#SkRRect_rect'>rect</a>(</td>
  </tr>
  <tr>    <td><a name='SkRRect_inset_dy'><code><strong>dy</strong></code></a></td>
    <td>added to <a href='#SkRRect_rect'>rect</a>(</td>
  </tr>
  <tr>    <td><a name='SkRRect_inset_dst'><code><strong>dst</strong></code></a></td>
    <td>insets bounds and radii</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f02f0110d5605dac6d14dcb8d1d8cb6e"></fiddle-embed></div>

### See Also

<a href='#SkRRect_outset'>outset</a><sup><a href='#SkRRect_outset_2'>[2]</a></sup> <a href='#SkRRect_offset'>offset</a> <a href='#SkRRect_makeOffset'>makeOffset</a>

---

<a name='SkRRect_inset_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_inset'>inset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx
</pre>

Insets bounds by <a href='#SkRRect_inset_2_dx'>dx</a> and <a href='#SkRRect_inset_2_dy'>dy</a>

### Parameters

<table>  <tr>    <td><a name='SkRRect_inset_2_dx'><code><strong>dx</strong></code></a></td>
    <td>added to <a href='#SkRRect_rect'>rect</a>(</td>
  </tr>
  <tr>    <td><a name='SkRRect_inset_2_dy'><code><strong>dy</strong></code></a></td>
    <td>added to <a href='#SkRRect_rect'>rect</a>(</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="da61054322550a2d5ac15114da23bd23"></fiddle-embed></div>

### See Also

<a href='#SkRRect_outset'>outset</a><sup><a href='#SkRRect_outset_2'>[2]</a></sup> <a href='#SkRRect_offset'>offset</a> <a href='#SkRRect_makeOffset'>makeOffset</a>

---

<a name='SkRRect_outset'></a>
## outset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_outset'>outset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx
</pre>

Outsets <a href='#SkRRect_outset_dst'>dst</a> bounds by <a href='#SkRRect_outset_dx'>dx</a> and <a href='#SkRRect_outset_dy'>dy</a>

### Parameters

<table>  <tr>    <td><a name='SkRRect_outset_dx'><code><strong>dx</strong></code></a></td>
    <td>subtracted from <a href='#SkRRect_rect'>rect</a>(</td>
  </tr>
  <tr>    <td><a name='SkRRect_outset_dy'><code><strong>dy</strong></code></a></td>
    <td>subtracted from <a href='#SkRRect_rect'>rect</a>(</td>
  </tr>
  <tr>    <td><a name='SkRRect_outset_dst'><code><strong>dst</strong></code></a></td>
    <td>outset bounds and radii</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4d69b6d9c7726c47c42827d79fc7899c"></fiddle-embed></div>

### See Also

<a href='#SkRRect_inset'>inset</a><sup><a href='#SkRRect_inset_2'>[2]</a></sup> <a href='#SkRRect_offset'>offset</a> <a href='#SkRRect_makeOffset'>makeOffset</a>

---

<a name='SkRRect_outset_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_outset'>outset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx
</pre>

Outsets bounds by <a href='#SkRRect_outset_2_dx'>dx</a> and <a href='#SkRRect_outset_2_dy'>dy</a>

### Parameters

<table>  <tr>    <td><a name='SkRRect_outset_2_dx'><code><strong>dx</strong></code></a></td>
    <td>subtracted from <a href='#SkRRect_rect'>rect</a>(</td>
  </tr>
  <tr>    <td><a name='SkRRect_outset_2_dy'><code><strong>dy</strong></code></a></td>
    <td>subtracted from <a href='#SkRRect_rect'>rect</a>(</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4391cced86653dcd0f84439a5c0bb3f2"></fiddle-embed></div>

### See Also

<a href='#SkRRect_inset'>inset</a><sup><a href='#SkRRect_inset_2'>[2]</a></sup> <a href='#SkRRect_offset'>offset</a> <a href='#SkRRect_makeOffset'>makeOffset</a>

---

<a name='SkRRect_offset'></a>
## offset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_offset'>offset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx
</pre>

Translates <a href='#RRect'>Round Rect</a> by

### Parameters

<table>  <tr>    <td><a name='SkRRect_offset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#SkRRect_rect'>rect</a>(</td>
  </tr>
  <tr>    <td><a name='SkRRect_offset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#SkRRect_rect'>rect</a>(</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a45cdd46ef2fe0df62d84d41713e82e2"></fiddle-embed></div>

### See Also

<a href='#SkRRect_makeOffset'>makeOffset</a> <a href='#SkRRect_inset'>inset</a><sup><a href='#SkRRect_inset_2'>[2]</a></sup> <a href='#SkRRect_outset'>outset</a><sup><a href='#SkRRect_outset_2'>[2]</a></sup>

---

<a name='SkRRect_makeOffset'></a>
## makeOffset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkRRect'>SkRRect</a> <a href='#SkRRect_makeOffset'>makeOffset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx
</pre>

Returns <a href='#RRect'>Round Rect</a> translated by

### Parameters

<table>  <tr>    <td><a name='SkRRect_makeOffset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#SkRRect_rect'>rect</a>(</td>
  </tr>
  <tr>    <td><a name='SkRRect_makeOffset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#SkRRect_rect'>rect</a>(</td>
  </tr>
</table>

### Return Value

<a href='#RRect'>Round Rect</a> bounds offset by

### Example

<div><fiddle-embed name="c433aa41eaf5e419e3349fb970a08151"></fiddle-embed></div>

### See Also

<a href='#SkRRect_offset'>offset</a> <a href='#SkRRect_inset'>inset</a><sup><a href='#SkRRect_inset_2'>[2]</a></sup> <a href='#SkRRect_outset'>outset</a><sup><a href='#SkRRect_outset_2'>[2]</a></sup>

---

<a name='SkRRect_contains'></a>
## contains

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_contains'>contains</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Returns true if rect is inside the bounds and corner radii

### Parameters

<table>  <tr>    <td><a name='SkRRect_contains_rect'><code><strong>rect</strong></code></a></td>
    <td>area tested for containment</td>
  </tr>
</table>

### Return Value

true if <a href='#RRect'>Round Rect</a> contains rect

### Example

<div><fiddle-embed name="884447c809921cfaebc87aeb63dedd48"></fiddle-embed></div>

### See Also

<a href='SkRect_Reference#SkRect_contains'>SkRect::contains</a><sup><a href='SkRect_Reference#SkRect_contains_2'>[2]</a></sup><sup><a href='SkRect_Reference#SkRect_contains_3'>[3]</a></sup>

---

<a name='SkRRect_isValid'></a>
## isValid

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_isValid'>isValid</a>(
</pre>

Returns true if bounds and radii values are finite and describe a <a href='#RRect'>Round Rect</a>
<a href='#SkRRect_Type'>Type</a> that matches <a href='#SkRRect_getType'>getType</a>

### Return Value

true if bounds and radii match <a href='#SkRRect_type'>type</a>(

### Example

<div><fiddle-embed name="d28709aa457742391842a9ab1f21b5fa"></fiddle-embed></div>

### See Also

<a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_getType'>getType</a>

---

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kSizeInMemory'><code>SkRRect::kSizeInMemory</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>48</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Space required to serialize <a href='#SkRRect'>SkRRect</a> into a buffer</td>
  </tr>
</table>

<a name='SkRRect_writeToMemory'></a>
## writeToMemory

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkRRect_writeToMemory'>writeToMemory</a>(void
</pre>

Writes <a href='#RRect'>Round Rect</a> to <a href='#SkRRect_writeToMemory_buffer'>buffer</a>

### Parameters

<table>  <tr>    <td><a name='SkRRect_writeToMemory_buffer'><code><strong>buffer</strong></code></a></td>
    <td>storage for <a href='#RRect'>Round Rect</a></td>
  </tr>
</table>

### Return Value

bytes written

### Example

<div><fiddle-embed name="1466c844a78fd05a7362537347e360ca"></fiddle-embed></div>

### See Also

<a href='#SkRRect_readFromMemory'>readFromMemory</a>

---

<a name='SkRRect_readFromMemory'></a>
## readFromMemory

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkRRect_readFromMemory'>readFromMemory</a>(const void
</pre>

Reads <a href='#RRect'>Round Rect</a> from <a href='#SkRRect_readFromMemory_buffer'>buffer</a>

### Parameters

<table>  <tr>    <td><a name='SkRRect_readFromMemory_buffer'><code><strong>buffer</strong></code></a></td>
    <td>memory to read from</td>
  </tr>
  <tr>    <td><a name='SkRRect_readFromMemory_length'><code><strong>length</strong></code></a></td>
    <td>size of <a href='#SkRRect_readFromMemory_buffer'>buffer</a></td>
  </tr>
</table>

### Return Value

bytes read

### Example

<div><fiddle-embed name="b877c0adff35470865a57aa150bf5329"></fiddle-embed></div>

### See Also

<a href='#SkRRect_writeToMemory'>writeToMemory</a>

---

<a name='SkRRect_transform'></a>
## transform

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_transform'>transform</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
</pre>

Transforms by <a href='#RRect'>Round Rect</a> by <a href='#SkRRect_transform_matrix'>matrix</a>

### Parameters

<table>  <tr>    <td><a name='SkRRect_transform_matrix'><code><strong>matrix</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> specifying the transform</td>
  </tr>
  <tr>    <td><a name='SkRRect_transform_dst'><code><strong>dst</strong></code></a></td>
    <td><a href='#SkRRect'>SkRRect</a> to store the result</td>
  </tr>
</table>

### Return Value

true if transformation succeeded

### Example

<div><fiddle-embed name="99ccc6862bb9fe3ca35228eee9f9725d"></fiddle-embed></div>

### See Also

<a href='SkPath_Reference#SkPath_transform'>SkPath::transform</a><sup><a href='SkPath_Reference#SkPath_transform_2'>[2]</a></sup>

---

<a name='SkRRect_dump'></a>
## dump

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_dump'>dump</a>(bool asHex
</pre>

Writes text representation of <a href='#RRect'>Round Rect</a> to standard output

### Parameters

<table>  <tr>    <td><a name='SkRRect_dump_asHex'><code><strong>asHex</strong></code></a></td>
    <td>true if <a href='undocumented#SkScalar'>SkScalar</a> values are written as hexadecimal</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="265b8d23288dc8026ff788e809360af7">

#### Example Output

~~~~
SkRect::MakeLTRB(0.857143f, 0.666667f, 0.857143f, 0.666667f);
const SkPoint corners[] = {
{ 0, 0 },
{ 0, 0 },
{ 0, 0 },
{ 0, 0 },
};
SkRect::MakeLTRB(SkBits2Float(0x3f5b6db7), /* 0.857143 */
SkBits2Float(0x3f2aaaab), /* 0.666667 */
SkBits2Float(0x3f5b6db7), /* 0.857143 */
SkBits2Float(0x3f2aaaab)  /* 0.666667 */);
const SkPoint corners[] = {
{ SkBits2Float(0x00000000), SkBits2Float(0x00000000) }, /* 0.000000 0.000000 */
{ SkBits2Float(0x00000000), SkBits2Float(0x00000000) }, /* 0.000000 0.000000 */
{ SkBits2Float(0x00000000), SkBits2Float(0x00000000) }, /* 0.000000 0.000000 */
{ SkBits2Float(0x00000000), SkBits2Float(0x00000000) }, /* 0.000000 0.000000 */
};
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRRect_dumpHex'>dumpHex</a> <a href='SkRect_Reference#SkRect_dump'>SkRect::dump</a><sup><a href='SkRect_Reference#SkRect_dump_2'>[2]</a></sup> <a href='SkPath_Reference#SkPath_dump'>SkPath::dump</a><sup><a href='SkPath_Reference#SkPath_dump_2'>[2]</a></sup> <a href='undocumented#SkPathMeasure_dump'>SkPathMeasure::dump</a>

---

<a name='SkRRect_dump_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_dump'>dump</a>(
</pre>

Writes text representation of <a href='#RRect'>Round Rect</a> to standard output

### Example

<div><fiddle-embed name="f850423c7c0c4f803d479ecd92221059">

#### Example Output

~~~~
SkRect::MakeLTRB(0.857143f, 0.666667f, 0.857143f, 0.666667f);
const SkPoint corners[] = {
{ 0, 0 },
{ 0, 0 },
{ 0, 0 },
{ 0, 0 },
};
rrect is not equal to copy
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRRect_dumpHex'>dumpHex</a> <a href='SkRect_Reference#SkRect_dump'>SkRect::dump</a><sup><a href='SkRect_Reference#SkRect_dump_2'>[2]</a></sup> <a href='SkPath_Reference#SkPath_dump'>SkPath::dump</a><sup><a href='SkPath_Reference#SkPath_dump_2'>[2]</a></sup> <a href='undocumented#SkPathMeasure_dump'>SkPathMeasure::dump</a>

---

<a name='SkRRect_dumpHex'></a>
## dumpHex

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_dumpHex'>dumpHex</a>(
</pre>

Writes text representation of <a href='#RRect'>Round Rect</a> to standard output

### Example

<div><fiddle-embed name="c73f5e2644d949b859f05bd367883454">

#### Example Output

~~~~
SkRect::MakeLTRB(SkBits2Float(0x3f5b6db7), /* 0.857143 */
SkBits2Float(0x3f2aaaab), /* 0.666667 */
SkBits2Float(0x3f5b6db7), /* 0.857143 */
SkBits2Float(0x3f2aaaab)  /* 0.666667 */);
const SkPoint corners[] = {
{ SkBits2Float(0x00000000), SkBits2Float(0x00000000) }, /* 0.000000 0.000000 */
{ SkBits2Float(0x00000000), SkBits2Float(0x00000000) }, /* 0.000000 0.000000 */
{ SkBits2Float(0x00000000), SkBits2Float(0x00000000) }, /* 0.000000 0.000000 */
{ SkBits2Float(0x00000000), SkBits2Float(0x00000000) }, /* 0.000000 0.000000 */
};
rrect is equal to copy
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRRect_dump'>dump</a><sup><a href='#SkRRect_dump_2'>[2]</a></sup> <a href='SkRect_Reference#SkRect_dumpHex'>SkRect::dumpHex</a> <a href='SkPath_Reference#SkPath_dumpHex'>SkPath::dumpHex</a>

---

