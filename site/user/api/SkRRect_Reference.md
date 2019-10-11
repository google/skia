SkRRect Reference
===


<a name='SkRRect'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkRRect_Reference#SkRRect'>SkRRect</a> {
public:
    <a href='#SkRRect_empty_constructor'>SkRRect()</a> = default;
    <a href='#SkRRect_copy_const_SkRRect'>SkRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& rrect) = default;
    <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='#SkRRect_copy_operator'>operator=</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& rrect) = default;

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
    <a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_type'>type()</a> const;
    bool <a href='#SkRRect_isEmpty'>isEmpty</a>() const;
    bool <a href='#SkRRect_isRect'>isRect</a>() const;
    bool <a href='#SkRRect_isOval'>isOval</a>() const;
    bool <a href='#SkRRect_isSimple'>isSimple</a>() const;
    bool <a href='#SkRRect_isNinePatch'>isNinePatch</a>() const;
    bool <a href='#SkRRect_isComplex'>isComplex</a>() const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRRect_width'>width()</a> const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRRect_height'>height()</a> const;
    <a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='#SkRRect_getSimpleRadii'>getSimpleRadii</a>() const;
    void <a href='#SkRRect_setEmpty'>setEmpty</a>();
    void <a href='#SkRRect_setRect'>setRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>);
    static <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkRRect_MakeEmpty'>MakeEmpty</a>();
    static <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkRRect_MakeRect'>MakeRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& r);
    static <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkRRect_MakeOval'>MakeOval</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='undocumented#Oval'>oval</a>);
    static <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkRRect_MakeRectXY'>MakeRectXY</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkScalar'>SkScalar</a> xRad, <a href='undocumented#SkScalar'>SkScalar</a> yRad);
    void <a href='#SkRRect_setOval'>setOval</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='undocumented#Oval'>oval</a>);
    void <a href='#SkRRect_setRectXY'>setRectXY</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkScalar'>SkScalar</a> xRad, <a href='undocumented#SkScalar'>SkScalar</a> yRad);
    void <a href='#SkRRect_setNinePatch'>setNinePatch</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkScalar'>SkScalar</a> leftRad, <a href='undocumented#SkScalar'>SkScalar</a> topRad,
                      <a href='undocumented#SkScalar'>SkScalar</a> rightRad, <a href='undocumented#SkScalar'>SkScalar</a> bottomRad);
    void <a href='#SkRRect_setRectRadii'>setRectRadii</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, const <a href='SkPoint_Reference#SkVector'>SkVector</a> radii[4]);

    enum <a href='#SkRRect_Corner'>Corner</a> {
        <a href='#SkRRect_kUpperLeft_Corner'>kUpperLeft_Corner</a>,
        <a href='#SkRRect_kUpperRight_Corner'>kUpperRight_Corner</a>,
        <a href='#SkRRect_kLowerRight_Corner'>kLowerRight_Corner</a>,
        <a href='#SkRRect_kLowerLeft_Corner'>kLowerLeft_Corner</a>,
    };

    const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkRRect_rect'>rect()</a> const;
    <a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='#SkRRect_radii'>radii</a>(<a href='#SkRRect_Corner'>Corner</a> corner) const;
    const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkRRect_getBounds'>getBounds</a>() const;
    friend bool <a href='#SkRRect_equal_operator'>operator==</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& a, const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& b);
    friend bool <a href='#SkRRect_notequal_operator'>operator!=</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& a, const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& b);
    void <a href='#SkRRect_inset'>inset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy, <a href='SkRRect_Reference#SkRRect'>SkRRect</a>* dst) const;
    void <a href='#SkRRect_inset'>inset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy);
    void <a href='#SkRRect_outset'>outset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy, <a href='SkRRect_Reference#SkRRect'>SkRRect</a>* dst) const;
    void <a href='#SkRRect_outset'>outset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy);
    void <a href='#SkRRect_offset'>offset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy);
    <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkRRect_makeOffset'>makeOffset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy) const;
    bool <a href='#SkRRect_contains'>contains</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>) const;
    bool <a href='#SkRRect_isValid'>isValid</a>() const;

    static constexpr size_t <a href='#SkRRect_kSizeInMemory'>kSizeInMemory</a> = 12 * <a href='undocumented#sizeof()'>sizeof</a>(<a href='undocumented#SkScalar'>SkScalar</a>);

    size_t <a href='#SkRRect_writeToMemory'>writeToMemory</a>(void* buffer) const;
    size_t <a href='#SkRRect_readFromMemory'>readFromMemory</a>(const void* buffer, size_t length);
    bool <a href='#SkRRect_transform'>transform</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#Matrix'>matrix</a>, <a href='SkRRect_Reference#SkRRect'>SkRRect</a>* dst) const;
    void <a href='#SkRRect_dump'>dump</a>(bool asHex) const;
    void <a href='#SkRRect_dump'>dump()</a> const;
    void <a href='#SkRRect_dumpHex'>dumpHex</a>() const;
};
</pre>

<a href='SkRRect_Reference#SkRRect'>SkRRect</a> describes a rounded rectangle with a bounds and a pair of radii for each corner.
The bounds and radii can be set so that <a href='SkRRect_Reference#SkRRect'>SkRRect</a> describes: a rectangle with sharp corners;
a <a href='undocumented#Circle'>Circle</a>; an <a href='undocumented#Oval'>Oval</a>; or a rectangle with one or more rounded corners.

<a href='SkRRect_Reference#SkRRect'>SkRRect</a> allows implementing CSS properties that describe rounded corners.
<a href='SkRRect_Reference#SkRRect'>SkRRect</a> may have up to eight different radii, one for each axis on each of its four
corners.

<a href='SkRRect_Reference#SkRRect'>SkRRect</a> may modify the provided parameters when initializing bounds and radii.
If either axis radii is zero or less: radii are stored as zero; corner is square.
If corner <a href='undocumented#Curve'>curves</a> overlap, radii are proportionally reduced to fit within bounds.

<a name='SkRRect_empty_constructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkRRect_empty_constructor'>SkRRect()</a>
</pre>

Initializes bounds at (0, 0), the origin, with zero width and height.
Initializes corner radii to (0, 0), and sets type of <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>.

### Return Value

empty <a href='SkRRect_Reference#SkRRect'>SkRRect</a>

### Example

<div><fiddle-embed name="@RRect_empty_constructor"></fiddle-embed></div>

### See Also

<a href='#SkRRect_setEmpty'>setEmpty</a> <a href='#SkRRect_isEmpty'>isEmpty</a>

<a name='SkRRect_copy_const_SkRRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkRRect_copy_const_SkRRect'>SkRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& rrect)
</pre>

Initializes to copy of <a href='#SkRRect_copy_const_SkRRect_rrect'>rrect</a> bounds and corner radii.

### Parameters

<table>  <tr>    <td><a name='SkRRect_copy_const_SkRRect_rrect'><code><strong>rrect</strong></code></a></td>
    <td>bounds and corner to copy</td>
  </tr>
</table>

### Return Value

copy of <a href='#SkRRect_copy_const_SkRRect_rrect'>rrect</a>

### Example

<div><fiddle-embed name="@RRect_copy_const_SkRRect"></fiddle-embed></div>

### See Also

<a href='#SkRRect_copy_operator'>operator=</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='#SkRRect_copy_const_SkRRect_rrect'>rrect</a>) <a href='#SkRRect_MakeRect'>MakeRect</a>

<a name='SkRRect_copy_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='#SkRRect_copy_operator'>operator=</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& rrect)
</pre>

Copies <a href='#SkRRect_copy_operator_rrect'>rrect</a> bounds and corner radii.

### Parameters

<table>  <tr>    <td><a name='SkRRect_copy_operator_rrect'><code><strong>rrect</strong></code></a></td>
    <td>bounds and corner to copy</td>
  </tr>
</table>

### Return Value

copy of <a href='#SkRRect_copy_operator_rrect'>rrect</a>

### Example

<div><fiddle-embed name="@RRect_copy_operator"></fiddle-embed></div>

### See Also

<a href='#SkRRect_copy_const_SkRRect'>SkRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='#SkRRect_copy_operator_rrect'>rrect</a>) <a href='#SkRRect_MakeRect'>MakeRect</a>

<a name='Type'></a>

<a name='SkRRect_Type'></a>

---

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

<a href='#SkRRect_Type'>Type</a> describes possible specializations of <a href='#RRect'>Round_Rect</a>. Each <a href='#SkRRect_Type'>Type</a> is
exclusive; a <a href='#RRect'>Round_Rect</a> may only have one type.

<a href='#SkRRect_Type'>Type</a> members become progressively less restrictive; larger values of
<a href='#SkRRect_Type'>Type</a> have more degrees of freedom than smaller values.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kEmpty_Type'><code>SkRRect::kEmpty_Type</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#RRect'>Round_Rect</a> has zero width or height. All radii are zero.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kRect_Type'><code>SkRRect::kRect_Type</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#RRect'>Round_Rect</a> has width and height. All radii are zero.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kOval_Type'><code>SkRRect::kOval_Type</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#RRect'>Round_Rect</a> has width and height. All four x-radii are equal,
and at least half the width. All four y-radii are equal,
and at least half the height.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kSimple_Type'><code>SkRRect::kSimple_Type</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#RRect'>Round_Rect</a> has width and height. All four x-radii are equal and
greater than zero, and all four y-radii are equal and greater than
zero. Either x-radii are less than half the width, or y-radii is
less than half the height, or both.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kNinePatch_Type'><code>SkRRect::kNinePatch_Type</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#RRect'>Round_Rect</a> has width and height. Left x-radii are equal, top
y-radii are equal, right x-radii are equal, and bottom y-radii
are equal. The radii do not describe <a href='SkRect_Reference#Rect'>Rect</a>, <a href='undocumented#Oval'>Oval</a>, or simple type.

The centers of the corner ellipses form an axis-aligned rectangle
that divides the <a href='#RRect'>Round_Rect</a> into nine rectangular <a href='undocumented#Patch'>patches</a>; an
interior rectangle, four edges, and four corners.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kComplex_Type'><code>SkRRect::kComplex_Type</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>5</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
both radii are non-zero.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kLastType'><code>SkRRect::kLastType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>5</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
largest Type value</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a4233634c75b72fc7a2815ddb69bd669"></fiddle-embed></div>

### See Also

<a href='SkRect_Reference#Rect'>Rect</a> <a href='SkPath_Reference#Path'>Path</a>

<a name='SkRRect_getType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_getType'>getType</a>()const
</pre>

Returns <a href='#SkRRect_Type'>Type</a>, one of: <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>, <a href='#SkRRect_kRect_Type'>kRect_Type</a>, <a href='#SkRRect_kOval_Type'>kOval_Type</a>, <a href='#SkRRect_kSimple_Type'>kSimple_Type</a>, <a href='#SkRRect_kNinePatch_Type'>kNinePatch_Type</a>,
<a href='#SkRRect_kComplex_Type'>kComplex_Type</a>
.

### Return Value

<a href='#SkRRect_Type'>Type</a>

### Example

<div><fiddle-embed name="ace8f4aebf90527d43e4b7291375c9ad"><div>rrect2 is not a <a href='SkRect_Reference#Rect'>Rect</a>; <a href='#SkRRect_inset'>inset()</a> has made it empty.
</div></fiddle-embed></div>

### See Also

<a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_type'>type</a>

<a name='SkRRect_type'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_type'>type()</a>const
</pre>

Returns <a href='#SkRRect_Type'>Type</a>, one of: <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>, <a href='#SkRRect_kRect_Type'>kRect_Type</a>, <a href='#SkRRect_kOval_Type'>kOval_Type</a>, <a href='#SkRRect_kSimple_Type'>kSimple_Type</a>, <a href='#SkRRect_kNinePatch_Type'>kNinePatch_Type</a>,
<a href='#SkRRect_kComplex_Type'>kComplex_Type</a>
.

### Return Value

<a href='#SkRRect_Type'>Type</a>

### Example

<div><fiddle-embed name="1080805c8449406a4e26d694bc56d2dc"><div><a href='#SkRRect_inset'>inset()</a> has made rrect2 empty.
</div></fiddle-embed></div>

### See Also

<a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_getType'>getType</a>

<a name='SkRRect_isEmpty'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_isEmpty'>isEmpty</a>()const
</pre>

### Example

<div><fiddle-embed name="099d79ecfbdfb0a19c10deb7201859c3"></fiddle-embed></div>

### See Also

<a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_isEmpty'>isEmpty</a> <a href='#SkRRect_height'>height</a> <a href='#SkRRect_width'>width</a>

<a name='SkRRect_isRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_isRect'>isRect</a>()const
</pre>

### Example

<div><fiddle-embed name="bc931c9a6eb8ffe7ea8d3fb47e07a475"></fiddle-embed></div>

### See Also

<a href='#SkRRect_isEmpty'>isEmpty</a> <a href='#SkRRect_radii'>radii</a>

<a name='SkRRect_isOval'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_isOval'>isOval</a>()const
</pre>

### Example

<div><fiddle-embed name="4dfdb28d8343958425f2c1323fe8170d"><div>The first radii are scaled down proportionately until both x-axis and y-axis fit
within the bounds. After scaling, x-axis radius is smaller than half the width;
left <a href='#RRect'>Round_Rect</a> is not an <a href='undocumented#Oval'>oval</a>. The second radii are equal to half the
dimensions; right <a href='#RRect'>Round_Rect</a> is an <a href='undocumented#Oval'>oval</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkRRect_isEmpty'>isEmpty</a> <a href='#SkRRect_isSimple'>isSimple</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawOval'>drawOval</a>

<a name='SkRRect_isSimple'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_isSimple'>isSimple</a>()const
</pre>

### Example

<div><fiddle-embed name="f6959ea422a7c6e98ddfad216a52c707"></fiddle-embed></div>

### See Also

<a href='#SkRRect_isEmpty'>isEmpty</a> <a href='#SkRRect_isRect'>isRect</a> <a href='#SkRRect_isOval'>isOval</a> <a href='#SkRRect_isNinePatch'>isNinePatch</a>

<a name='SkRRect_isNinePatch'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_isNinePatch'>isNinePatch</a>()const
</pre>

### Example

<div><fiddle-embed name="429f6dfd4cf6287df3c3c77fa7681c99"></fiddle-embed></div>

### See Also

<a href='#SkRRect_isEmpty'>isEmpty</a> <a href='#SkRRect_isRect'>isRect</a> <a href='#SkRRect_isOval'>isOval</a> <a href='#SkRRect_isSimple'>isSimple</a> <a href='#SkRRect_isComplex'>isComplex</a>

<a name='SkRRect_isComplex'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_isComplex'>isComplex</a>()const
</pre>

### Example

<div><fiddle-embed name="b62c183dc435d1fc091111fb2f3c3f8e"></fiddle-embed></div>

### See Also

<a href='#SkRRect_isEmpty'>isEmpty</a> <a href='#SkRRect_isRect'>isRect</a> <a href='#SkRRect_isOval'>isOval</a> <a href='#SkRRect_isSimple'>isSimple</a> <a href='#SkRRect_isNinePatch'>isNinePatch</a>

<a name='SkRRect_width'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRRect_width'>width()</a>const
</pre>

Returns span on the x-axis. This does not check if result fits in 32-bit float;
result may be infinity.

### Return Value

<a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fRight'>fRight</a> minus <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fLeft'>fLeft</a>

### Example

<div><fiddle-embed name="@RRect_width"><div><a href='SkRRect_Reference#SkRRect'>SkRRect</a>::<a href='#SkRRect_MakeRect'>MakeRect</a> sorts its input, so <a href='#SkRRect_width'>width()</a> is always zero or larger.
</div>

#### Example Output

~~~~
unsorted width: 5
large width: inf
~~~~

</fiddle-embed></div>

### See Also

<a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_width'>width</a> <a href='#SkRRect_height'>height</a> <a href='#SkRRect_getBounds'>getBounds</a>

<a name='SkRRect_height'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRRect_height'>height()</a>const
</pre>

Returns span on the y-axis. This does not check if result fits in 32-bit float;
result may be infinity.

### Return Value

<a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fBottom'>fBottom</a> minus <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fTop'>fTop</a>

### Example

<div><fiddle-embed name="@RRect_height"><div><a href='SkRRect_Reference#SkRRect'>SkRRect</a>::<a href='#SkRRect_MakeRect'>MakeRect</a> sorts its input, so <a href='#SkRRect_height'>height()</a> is always zero or larger.
</div>

#### Example Output

~~~~
unsorted height: 5
large height: inf
~~~~

</fiddle-embed></div>

### See Also

<a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_height'>height</a> <a href='#SkRRect_width'>width</a> <a href='#SkRRect_getBounds'>getBounds</a>

<a name='SkRRect_getSimpleRadii'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='#SkRRect_getSimpleRadii'>getSimpleRadii</a>()const
</pre>

Returns top-left corner radii. If <a href='#SkRRect_type'>type()</a> returns <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>, <a href='#SkRRect_kRect_Type'>kRect_Type</a>,
<a href='#SkRRect_kOval_Type'>kOval_Type</a>, or <a href='#SkRRect_kSimple_Type'>kSimple_Type</a>, returns a value representative of all corner radii.
If <a href='#SkRRect_type'>type()</a> returns <a href='#SkRRect_kNinePatch_Type'>kNinePatch_Type</a> or <a href='#SkRRect_kComplex_Type'>kComplex_Type</a>, at least one of the
remaining three corners has a different value.

### Return Value

corner radii for simple types

### Example

<div><fiddle-embed name="81345f7619a072bb2b0cf59810fe86d0"></fiddle-embed></div>

### See Also

<a href='#SkRRect_radii'>radii</a> <a href='#SkRRect_getBounds'>getBounds</a> <a href='#SkRRect_getType'>getType</a> <a href='#SkRRect_isSimple'>isSimple</a>

<a name='SkRRect_setEmpty'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_setEmpty'>setEmpty</a>()
</pre>

Sets bounds to zero width and height at (0, 0), the origin. Sets
corner radii to zero and sets type to <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>.

### Example

<div><fiddle-embed name="@RRect_setEmpty"><div>Nothing blue is drawn because <a href='#RRect'>Round_Rect</a> is set to empty.
</div></fiddle-embed></div>

### See Also

<a href='#SkRRect_MakeEmpty'>MakeEmpty</a> <a href='#SkRRect_setRect'>setRect</a>

<a name='SkRRect_setRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_setRect'>setRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>)
</pre>

Sets bounds to sorted <a href='#SkRRect_setRect_rect'>rect</a>, and sets corner radii to zero.
If set bounds has width and height, and sets type to <a href='#SkRRect_kRect_Type'>kRect_Type</a>;
otherwise, sets type to <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>.

### Parameters

<table>  <tr>    <td><a name='SkRRect_setRect_rect'><code><strong>rect</strong></code></a></td>
    <td>bounds to set</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@RRect_setRect"></fiddle-embed></div>

### See Also

<a href='#SkRRect_MakeRect'>MakeRect</a> <a href='#SkRRect_setRectXY'>setRectXY</a>

<a name='SkRRect_MakeEmpty'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkRRect_MakeEmpty'>MakeEmpty</a>()
</pre>

Initializes bounds at (0, 0), the origin, with zero width and height.
Initializes corner radii to (0, 0), and sets type of <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>.

### Return Value

empty <a href='SkRRect_Reference#SkRRect'>SkRRect</a>

### Example

<div><fiddle-embed name="c6c6be3b3c137226adbb5b5af9203d27"></fiddle-embed></div>

### See Also

<a href='#SkRRect_empty_constructor'>SkRRect()</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_MakeEmpty'>MakeEmpty</a>

<a name='SkRRect_MakeRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkRRect_MakeRect'>MakeRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& r)
</pre>

Initializes to copy of <a href='#SkRRect_MakeRect_r'>r</a> bounds and zeroes corner radii.

### Parameters

<table>  <tr>    <td><a name='SkRRect_MakeRect_r'><code><strong>r</strong></code></a></td>
    <td>bounds to copy</td>
  </tr>
</table>

### Return Value

copy of <a href='#SkRRect_MakeRect_r'>r</a>

### Example

<div><fiddle-embed name="@RRect_MakeRect"></fiddle-embed></div>

### See Also

<a href='#SkRRect_setRect'>setRect</a> <a href='#SkRRect_MakeOval'>MakeOval</a> <a href='#SkRRect_MakeRectXY'>MakeRectXY</a>

<a name='SkRRect_MakeOval'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkRRect_MakeOval'>MakeOval</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='undocumented#Oval'>oval</a>)
</pre>

Sets bounds to <a href='#SkRRect_MakeOval_oval'>oval</a>, x-axis radii to half <a href='#SkRRect_MakeOval_oval'>oval</a>.<a href='#SkRect_width'>width()</a>, and all y-axis radii
to half <a href='#SkRRect_MakeOval_oval'>oval</a>.<a href='#SkRect_height'>height()</a>. If <a href='#SkRRect_MakeOval_oval'>oval</a> bounds is empty, sets to <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>.
Otherwise, sets to <a href='#SkRRect_kOval_Type'>kOval_Type</a>.

### Parameters

<table>  <tr>    <td><a name='SkRRect_MakeOval_oval'><code><strong>oval</strong></code></a></td>
    <td>bounds of <a href='#SkRRect_MakeOval_oval'>oval</a></td>
  </tr>
</table>

### Return Value

<a href='#SkRRect_MakeOval_oval'>oval</a>

### Example

<div><fiddle-embed name="@RRect_MakeOval"></fiddle-embed></div>

### See Also

<a href='#SkRRect_setOval'>setOval</a> <a href='#SkRRect_MakeRect'>MakeRect</a> <a href='#SkRRect_MakeRectXY'>MakeRectXY</a>

<a name='SkRRect_MakeRectXY'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkRRect_MakeRectXY'>MakeRectXY</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkScalar'>SkScalar</a> xRad, <a href='undocumented#SkScalar'>SkScalar</a> yRad)
</pre>

Sets to rounded rectangle with the same radii for all four corners.
If <a href='#SkRRect_MakeRectXY_rect'>rect</a> is empty, sets to <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>.
Otherwise, if <a href='#SkRRect_MakeRectXY_xRad'>xRad</a> and <a href='#SkRRect_MakeRectXY_yRad'>yRad</a> are zero, sets to <a href='#SkRRect_kRect_Type'>kRect_Type</a>.
Otherwise, if <a href='#SkRRect_MakeRectXY_xRad'>xRad</a> is at least half <a href='#SkRRect_MakeRectXY_rect'>rect</a>.<a href='#SkRect_width'>width()</a> and <a href='#SkRRect_MakeRectXY_yRad'>yRad</a> is at least half
<a href='#SkRRect_MakeRectXY_rect'>rect</a>.<a href='#SkRect_height'>height()</a>, sets to <a href='#SkRRect_kOval_Type'>kOval_Type</a>.
Otherwise, sets to <a href='#SkRRect_kSimple_Type'>kSimple_Type</a>.

### Parameters

<table>  <tr>    <td><a name='SkRRect_MakeRectXY_rect'><code><strong>rect</strong></code></a></td>
    <td>bounds of rounded rectangle</td>
  </tr>
  <tr>    <td><a name='SkRRect_MakeRectXY_xRad'><code><strong>xRad</strong></code></a></td>
    <td>x-axis radius of corners</td>
  </tr>
  <tr>    <td><a name='SkRRect_MakeRectXY_yRad'><code><strong>yRad</strong></code></a></td>
    <td>y-axis radius of corners</td>
  </tr>
</table>

### Return Value

rounded rectangle

### Example

<div><fiddle-embed name="@RRect_MakeRectXY"></fiddle-embed></div>

### See Also

<a href='#SkRRect_setRectXY'>setRectXY</a>

<a name='SkRRect_setOval'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_setOval'>setOval</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='undocumented#Oval'>oval</a>)
</pre>

Sets bounds to <a href='#SkRRect_setOval_oval'>oval</a>, x-axis radii to half <a href='#SkRRect_setOval_oval'>oval</a>.<a href='#SkRect_width'>width()</a>, and all y-axis radii
to half <a href='#SkRRect_setOval_oval'>oval</a>.<a href='#SkRect_height'>height()</a>. If <a href='#SkRRect_setOval_oval'>oval</a> bounds is empty, sets to <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>.
Otherwise, sets to <a href='#SkRRect_kOval_Type'>kOval_Type</a>.

### Parameters

<table>  <tr>    <td><a name='SkRRect_setOval_oval'><code><strong>oval</strong></code></a></td>
    <td>bounds of <a href='#SkRRect_setOval_oval'>oval</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@RRect_setOval"></fiddle-embed></div>

### See Also

<a href='#SkRRect_MakeOval'>MakeOval</a>

<a name='SkRRect_setRectXY'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_setRectXY'>setRectXY</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkScalar'>SkScalar</a> xRad, <a href='undocumented#SkScalar'>SkScalar</a> yRad)
</pre>

Sets to rounded rectangle with the same radii for all four corners.
If <a href='#SkRRect_setRectXY_rect'>rect</a> is empty, sets to <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>.
Otherwise, if <a href='#SkRRect_setRectXY_xRad'>xRad</a> or <a href='#SkRRect_setRectXY_yRad'>yRad</a> is zero, sets to <a href='#SkRRect_kRect_Type'>kRect_Type</a>.
Otherwise, if <a href='#SkRRect_setRectXY_xRad'>xRad</a> is at least half <a href='#SkRRect_setRectXY_rect'>rect</a>.<a href='#SkRect_width'>width()</a> and <a href='#SkRRect_setRectXY_yRad'>yRad</a> is at least half
<a href='#SkRRect_setRectXY_rect'>rect</a>.<a href='#SkRect_height'>height()</a>, sets to <a href='#SkRRect_kOval_Type'>kOval_Type</a>.
Otherwise, sets to <a href='#SkRRect_kSimple_Type'>kSimple_Type</a>.

### Parameters

<table>  <tr>    <td><a name='SkRRect_setRectXY_rect'><code><strong>rect</strong></code></a></td>
    <td>bounds of rounded rectangle</td>
  </tr>
  <tr>    <td><a name='SkRRect_setRectXY_xRad'><code><strong>xRad</strong></code></a></td>
    <td>x-axis radius of corners</td>
  </tr>
  <tr>    <td><a name='SkRRect_setRectXY_yRad'><code><strong>yRad</strong></code></a></td>
    <td>y-axis radius of corners</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@RRect_setRectXY"></fiddle-embed></div>

### See Also

<a href='#SkRRect_MakeRectXY'>MakeRectXY</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_addRoundRect'>addRoundRect</a>

<a name='SkRRect_setNinePatch'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_setNinePatch'>setNinePatch</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkScalar'>SkScalar</a> leftRad, <a href='undocumented#SkScalar'>SkScalar</a> topRad, <a href='undocumented#SkScalar'>SkScalar</a> rightRad,
                  <a href='undocumented#SkScalar'>SkScalar</a> bottomRad)
</pre>

Sets bounds to <a href='#SkRRect_setNinePatch_rect'>rect</a>. Sets radii to (<a href='#SkRRect_setNinePatch_leftRad'>leftRad</a>, <a href='#SkRRect_setNinePatch_topRad'>topRad</a>), (<a href='#SkRRect_setNinePatch_rightRad'>rightRad</a>, <a href='#SkRRect_setNinePatch_topRad'>topRad</a>),
(<a href='#SkRRect_setNinePatch_rightRad'>rightRad</a>, <a href='#SkRRect_setNinePatch_bottomRad'>bottomRad</a>), (<a href='#SkRRect_setNinePatch_leftRad'>leftRad</a>, <a href='#SkRRect_setNinePatch_bottomRad'>bottomRad</a>).

If <a href='#SkRRect_setNinePatch_rect'>rect</a> is empty, sets to <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>.
Otherwise, if <a href='#SkRRect_setNinePatch_leftRad'>leftRad</a> and <a href='#SkRRect_setNinePatch_rightRad'>rightRad</a> are zero, sets to <a href='#SkRRect_kRect_Type'>kRect_Type</a>.
Otherwise, if <a href='#SkRRect_setNinePatch_topRad'>topRad</a> and <a href='#SkRRect_setNinePatch_bottomRad'>bottomRad</a> are zero, sets to <a href='#SkRRect_kRect_Type'>kRect_Type</a>.
Otherwise, if <a href='#SkRRect_setNinePatch_leftRad'>leftRad</a> and <a href='#SkRRect_setNinePatch_rightRad'>rightRad</a> are equal and at least half <a href='#SkRRect_setNinePatch_rect'>rect</a>.<a href='#SkRect_width'>width()</a>, and
<a href='#SkRRect_setNinePatch_topRad'>topRad</a> and <a href='#SkRRect_setNinePatch_bottomRad'>bottomRad</a> are equal at least half <a href='#SkRRect_setNinePatch_rect'>rect</a>.<a href='#SkRect_height'>height()</a>, sets to <a href='#SkRRect_kOval_Type'>kOval_Type</a>.
Otherwise, if <a href='#SkRRect_setNinePatch_leftRad'>leftRad</a> and <a href='#SkRRect_setNinePatch_rightRad'>rightRad</a> are equal, and <a href='#SkRRect_setNinePatch_topRad'>topRad</a> and <a href='#SkRRect_setNinePatch_bottomRad'>bottomRad</a> are equal,
sets to <a href='#SkRRect_kSimple_Type'>kSimple_Type</a>. Otherwise, sets to <a href='#SkRRect_kNinePatch_Type'>kNinePatch_Type</a>.

Nine <a href='undocumented#Patch'>patch</a> refers to the nine parts defined by the radii: one center rectangle,
four edge <a href='undocumented#Patch'>patches</a>, and four corner <a href='undocumented#Patch'>patches</a>.

### Parameters

<table>  <tr>    <td><a name='SkRRect_setNinePatch_rect'><code><strong>rect</strong></code></a></td>
    <td>bounds of rounded rectangle</td>
  </tr>
  <tr>    <td><a name='SkRRect_setNinePatch_leftRad'><code><strong>leftRad</strong></code></a></td>
    <td>left-top and left-bottom x-axis radius</td>
  </tr>
  <tr>    <td><a name='SkRRect_setNinePatch_topRad'><code><strong>topRad</strong></code></a></td>
    <td>left-top and right-top y-axis radius</td>
  </tr>
  <tr>    <td><a name='SkRRect_setNinePatch_rightRad'><code><strong>rightRad</strong></code></a></td>
    <td>right-top and right-bottom x-axis radius</td>
  </tr>
  <tr>    <td><a name='SkRRect_setNinePatch_bottomRad'><code><strong>bottomRad</strong></code></a></td>
    <td>left-bottom and right-bottom y-axis radius</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@RRect_setNinePatch"></fiddle-embed></div>

### See Also

<a href='#SkRRect_setRectRadii'>setRectRadii</a>

<a name='SkRRect_setRectRadii'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_setRectRadii'>setRectRadii</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, const <a href='SkPoint_Reference#SkVector'>SkVector</a> radii[4])
</pre>

Sets bounds to <a href='#SkRRect_setRectRadii_rect'>rect</a>. Sets <a href='#SkRRect_setRectRadii_radii'>radii</a> array for individual control of all for corners.

If <a href='#SkRRect_setRectRadii_rect'>rect</a> is empty, sets to <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>.
Otherwise, if one of each corner <a href='#SkRRect_setRectRadii_radii'>radii</a> are zero, sets to <a href='#SkRRect_kRect_Type'>kRect_Type</a>.
Otherwise, if all x-axis <a href='#SkRRect_setRectRadii_radii'>radii</a> are equal and at least half <a href='#SkRRect_setRectRadii_rect'>rect</a>.<a href='#SkRect_width'>width()</a>, and
all y-axis <a href='#SkRRect_setRectRadii_radii'>radii</a> are equal at least half <a href='#SkRRect_setRectRadii_rect'>rect</a>.<a href='#SkRect_height'>height()</a>, sets to <a href='#SkRRect_kOval_Type'>kOval_Type</a>.
Otherwise, if all x-axis <a href='#SkRRect_setRectRadii_radii'>radii</a> are equal, and all y-axis <a href='#SkRRect_setRectRadii_radii'>radii</a> are equal,
sets to <a href='#SkRRect_kSimple_Type'>kSimple_Type</a>. Otherwise, sets to <a href='#SkRRect_kNinePatch_Type'>kNinePatch_Type</a>.

### Parameters

<table>  <tr>    <td><a name='SkRRect_setRectRadii_rect'><code><strong>rect</strong></code></a></td>
    <td>bounds of rounded rectangle</td>
  </tr>
  <tr>    <td><a name='SkRRect_setRectRadii_radii'><code><strong>radii</strong></code></a></td>
    <td>corner x-axis and y-axis <a href='#SkRRect_setRectRadii_radii'>radii</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@RRect_setRectRadii"></fiddle-embed></div>

### See Also

<a href='#SkRRect_setNinePatch'>setNinePatch</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_addRoundRect'>addRoundRect</a>

<a name='SkRRect_Corner'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkRRect_Corner'>Corner</a> {
        <a href='#SkRRect_kUpperLeft_Corner'>kUpperLeft_Corner</a>,
        <a href='#SkRRect_kUpperRight_Corner'>kUpperRight_Corner</a>,
        <a href='#SkRRect_kLowerRight_Corner'>kLowerRight_Corner</a>,
        <a href='#SkRRect_kLowerLeft_Corner'>kLowerLeft_Corner</a>,
    };
</pre>

The radii are stored: top-left, top-right, bottom-right, bottom-left.

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

<div><fiddle-embed name="@RRect_Corner"></fiddle-embed></div>

### See Also

<a href='#SkRRect_radii'>radii</a>

<a name='SkRRect_rect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkRRect_rect'>rect()</a>const
</pre>

Returns bounds. Bounds may have zero width or zero height. Bounds right is
greater than or equal to left; bounds bottom is greater than or equal to top.
Result is identical to <a href='#SkRRect_getBounds'>getBounds</a>().

### Return Value

bounding box

### Example

<div><fiddle-embed name="@RRect_rect">

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

<a name='SkRRect_radii'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPoint_Reference#SkVector'>SkVector</a> radii(<a href='#SkRRect_Corner'>Corner</a> corner)const
</pre>

Returns <a href='undocumented#Scalar'>scalar</a> pair for radius of <a href='undocumented#Curve'>curve</a> on x-axis and y-axis for one <a href='#SkRRect_radii_corner'>corner</a>.
Both radii may be zero. If not zero, both are positive and finite.

### Parameters

<table>  <tr>    <td><a name='SkRRect_radii_corner'><code><strong>corner</strong></code></a></td>
    <td>one of: <a href='#SkRRect_kUpperLeft_Corner'>kUpperLeft_Corner</a>, <a href='#SkRRect_kUpperRight_Corner'>kUpperRight_Corner</a>,</td>
  </tr>
</table>

<a href='#SkRRect_kLowerRight_Corner'>kLowerRight_Corner</a>, <a href='#SkRRect_kLowerLeft_Corner'>kLowerLeft_Corner</a>

### Return Value

x-axis and y-axis radii for one <a href='#SkRRect_radii_corner'>corner</a>

### Example

<div><fiddle-embed name="@RRect_radii"><div>Finite values are scaled proportionately to fit; other values are set to zero.
Scaled values cannot be larger than 25, half the bounding <a href='#RRect'>Round_Rect</a> width.
Small scaled values are halved to scale in proportion to the y-axis <a href='#SkRRect_radii_corner'>corner</a>
radius, which is twice the bounds height.
</div>

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

<a name='SkRRect_getBounds'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkRRect_getBounds'>getBounds</a>()const
</pre>

Returns bounds. Bounds may have zero width or zero height. Bounds right is
greater than or equal to left; bounds bottom is greater than or equal to top.
Result is identical to <a href='#SkRRect_rect'>rect()</a>.

### Return Value

bounding box

### Example

<div><fiddle-embed name="@RRect_getBounds"></fiddle-embed></div>

### See Also

<a href='#SkRRect_rect'>rect</a>

<a name='SkRRect_equal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_equal_operator'>operator==</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& a, const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& b)
</pre>

Returns true if bounds and radii in <a href='#SkRRect_equal_operator_a'>a</a> are equal to bounds and radii in <a href='#SkRRect_equal_operator_b'>b</a>.

<a href='#SkRRect_equal_operator_a'>a</a> and <a href='#SkRRect_equal_operator_b'>b</a> are not equal if either contain NaN. <a href='#SkRRect_equal_operator_a'>a</a> and <a href='#SkRRect_equal_operator_b'>b</a> are equal if members
contain zeroes with different signs.

### Parameters

<table>  <tr>    <td><a name='SkRRect_equal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> bounds and radii to compare</td>
  </tr>
  <tr>    <td><a name='SkRRect_equal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> bounds and radii to compare</td>
  </tr>
</table>

### Return Value

true if members are equal

### Example

<div><fiddle-embed name="df181af37f1d2b06f0f45af73df7b47d"></fiddle-embed></div>

### See Also

<a href='#SkRRect_notequal_operator'>operator!=</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='#SkRRect_equal_operator_a'>a</a>, const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='#SkRRect_equal_operator_b'>b</a>)

<a name='SkRRect_notequal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_notequal_operator'>operator!=</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& a, const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& b)
</pre>

Returns true if bounds and radii in <a href='#SkRRect_notequal_operator_a'>a</a> are not equal to bounds and radii in <a href='#SkRRect_notequal_operator_b'>b</a>.

<a href='#SkRRect_notequal_operator_a'>a</a> and <a href='#SkRRect_notequal_operator_b'>b</a> are not equal if either contain NaN. <a href='#SkRRect_notequal_operator_a'>a</a> and <a href='#SkRRect_notequal_operator_b'>b</a> are equal if members
contain zeroes with different signs.

### Parameters

<table>  <tr>    <td><a name='SkRRect_notequal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> bounds and radii to compare</td>
  </tr>
  <tr>    <td><a name='SkRRect_notequal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> bounds and radii to compare</td>
  </tr>
</table>

### Return Value

true if members are not equal

### Example

<div><fiddle-embed name="505e47b3e6474ebdecdc04c3c2af2c34"></fiddle-embed></div>

### See Also

<a href='#SkRRect_equal_operator'>operator==</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='#SkRRect_notequal_operator_a'>a</a>, const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='#SkRRect_notequal_operator_b'>b</a>)

<a name='SkRRect_inset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void inset(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy, <a href='SkRRect_Reference#SkRRect'>SkRRect</a>* dst)const
</pre>

Copies <a href='SkRRect_Reference#SkRRect'>SkRRect</a> to <a href='#SkRRect_inset_dst'>dst</a>, then insets <a href='#SkRRect_inset_dst'>dst</a> bounds by <a href='#SkRRect_inset_dx'>dx</a> and <a href='#SkRRect_inset_dy'>dy</a>, and adjusts <a href='#SkRRect_inset_dst'>dst</a>
radii by <a href='#SkRRect_inset_dx'>dx</a> and <a href='#SkRRect_inset_dy'>dy</a>. <a href='#SkRRect_inset_dx'>dx</a> and <a href='#SkRRect_inset_dy'>dy</a> may be positive, negative, or zero. <a href='#SkRRect_inset_dst'>dst</a> may be
<a href='SkRRect_Reference#SkRRect'>SkRRect</a>.

If either corner radius is zero, the corner has no curvature and is unchanged.
Otherwise, if adjusted radius becomes negative, pins radius to zero.
If <a href='#SkRRect_inset_dx'>dx</a> exceeds half <a href='#SkRRect_inset_dst'>dst</a> bounds width, <a href='#SkRRect_inset_dst'>dst</a> bounds left and right are set to
bounds x-axis center. If <a href='#SkRRect_inset_dy'>dy</a> exceeds half <a href='#SkRRect_inset_dst'>dst</a> bounds height, <a href='#SkRRect_inset_dst'>dst</a> bounds top and
bottom are set to bounds y-axis center.

If <a href='#SkRRect_inset_dx'>dx</a> or <a href='#SkRRect_inset_dy'>dy</a> cause the bounds to become infinite, <a href='#SkRRect_inset_dst'>dst</a> bounds is zeroed.

### Parameters

<table>  <tr>    <td><a name='SkRRect_inset_dx'><code><strong>dx</strong></code></a></td>
    <td>added to <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fLeft'>fLeft</a>, and subtracted from <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRRect_inset_dy'><code><strong>dy</strong></code></a></td>
    <td>added to <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fTop'>fTop</a>, and subtracted from <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
  <tr>    <td><a name='SkRRect_inset_dst'><code><strong>dst</strong></code></a></td>
    <td>insets bounds and radii</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@RRect_inset"></fiddle-embed></div>

### See Also

<a href='#SkRRect_outset'>outset</a> <a href='#SkRRect_offset'>offset</a> <a href='#SkRRect_makeOffset'>makeOffset</a>

<a name='SkRRect_inset_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void inset(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy)
</pre>

Insets bounds by <a href='#SkRRect_inset_2_dx'>dx</a> and <a href='#SkRRect_inset_2_dy'>dy</a>, and adjusts radii by <a href='#SkRRect_inset_2_dx'>dx</a> and <a href='#SkRRect_inset_2_dy'>dy</a>. <a href='#SkRRect_inset_2_dx'>dx</a> and <a href='#SkRRect_inset_2_dy'>dy</a> may be
positive, negative, or zero.

If either corner radius is zero, the corner has no curvature and is unchanged.
Otherwise, if adjusted radius becomes negative, pins radius to zero.
If <a href='#SkRRect_inset_2_dx'>dx</a> exceeds half bounds width, bounds left and right are set to
bounds x-axis center. If <a href='#SkRRect_inset_2_dy'>dy</a> exceeds half bounds height, bounds top and
bottom are set to bounds y-axis center.

If <a href='#SkRRect_inset_2_dx'>dx</a> or <a href='#SkRRect_inset_2_dy'>dy</a> cause the bounds to become infinite, bounds is zeroed.

### Parameters

<table>  <tr>    <td><a name='SkRRect_inset_2_dx'><code><strong>dx</strong></code></a></td>
    <td>added to <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fLeft'>fLeft</a>, and subtracted from <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRRect_inset_2_dy'><code><strong>dy</strong></code></a></td>
    <td>added to <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fTop'>fTop</a>, and subtracted from <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@RRect_inset_2"></fiddle-embed></div>

### See Also

<a href='#SkRRect_outset'>outset</a> <a href='#SkRRect_offset'>offset</a> <a href='#SkRRect_makeOffset'>makeOffset</a>

<a name='SkRRect_outset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void outset(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy, <a href='SkRRect_Reference#SkRRect'>SkRRect</a>* dst)const
</pre>

Outsets <a href='#SkRRect_outset_dst'>dst</a> bounds by <a href='#SkRRect_outset_dx'>dx</a> and <a href='#SkRRect_outset_dy'>dy</a>, and adjusts radii by <a href='#SkRRect_outset_dx'>dx</a> and <a href='#SkRRect_outset_dy'>dy</a>. <a href='#SkRRect_outset_dx'>dx</a> and <a href='#SkRRect_outset_dy'>dy</a> may be
positive, negative, or zero.

If either corner radius is zero, the corner has no curvature and is unchanged.
Otherwise, if adjusted radius becomes negative, pins radius to zero.
If <a href='#SkRRect_outset_dx'>dx</a> exceeds half <a href='#SkRRect_outset_dst'>dst</a> bounds width, <a href='#SkRRect_outset_dst'>dst</a> bounds left and right are set to
bounds x-axis center. If <a href='#SkRRect_outset_dy'>dy</a> exceeds half <a href='#SkRRect_outset_dst'>dst</a> bounds height, <a href='#SkRRect_outset_dst'>dst</a> bounds top and
bottom are set to bounds y-axis center.

If <a href='#SkRRect_outset_dx'>dx</a> or <a href='#SkRRect_outset_dy'>dy</a> cause the bounds to become infinite, <a href='#SkRRect_outset_dst'>dst</a> bounds is zeroed.

### Parameters

<table>  <tr>    <td><a name='SkRRect_outset_dx'><code><strong>dx</strong></code></a></td>
    <td>subtracted from <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fLeft'>fLeft</a>, and added to <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRRect_outset_dy'><code><strong>dy</strong></code></a></td>
    <td>subtracted from <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fTop'>fTop</a>, and added to <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
  <tr>    <td><a name='SkRRect_outset_dst'><code><strong>dst</strong></code></a></td>
    <td>outset bounds and radii</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@RRect_outset"></fiddle-embed></div>

### See Also

<a href='#SkRRect_inset'>inset</a> <a href='#SkRRect_offset'>offset</a> <a href='#SkRRect_makeOffset'>makeOffset</a>

<a name='SkRRect_outset_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void outset(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy)
</pre>

Outsets bounds by <a href='#SkRRect_outset_2_dx'>dx</a> and <a href='#SkRRect_outset_2_dy'>dy</a>, and adjusts radii by <a href='#SkRRect_outset_2_dx'>dx</a> and <a href='#SkRRect_outset_2_dy'>dy</a>. <a href='#SkRRect_outset_2_dx'>dx</a> and <a href='#SkRRect_outset_2_dy'>dy</a> may be
positive, negative, or zero.

If either corner radius is zero, the corner has no curvature and is unchanged.
Otherwise, if adjusted radius becomes negative, pins radius to zero.
If <a href='#SkRRect_outset_2_dx'>dx</a> exceeds half bounds width, bounds left and right are set to
bounds x-axis center. If <a href='#SkRRect_outset_2_dy'>dy</a> exceeds half bounds height, bounds top and
bottom are set to bounds y-axis center.

If <a href='#SkRRect_outset_2_dx'>dx</a> or <a href='#SkRRect_outset_2_dy'>dy</a> cause the bounds to become infinite, bounds is zeroed.

### Parameters

<table>  <tr>    <td><a name='SkRRect_outset_2_dx'><code><strong>dx</strong></code></a></td>
    <td>subtracted from <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fLeft'>fLeft</a>, and added to <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRRect_outset_2_dy'><code><strong>dy</strong></code></a></td>
    <td>subtracted from <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fTop'>fTop</a>, and added to <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@RRect_outset_2"></fiddle-embed></div>

### See Also

<a href='#SkRRect_inset'>inset</a> <a href='#SkRRect_offset'>offset</a> <a href='#SkRRect_makeOffset'>makeOffset</a>

<a name='SkRRect_offset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void offset(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy)
</pre>

Translates <a href='SkRRect_Reference#SkRRect'>SkRRect</a> by (<a href='#SkRRect_offset_dx'>dx</a>, <a href='#SkRRect_offset_dy'>dy</a>).

### Parameters

<table>  <tr>    <td><a name='SkRRect_offset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fLeft'>fLeft</a> and <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRRect_offset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fTop'>fTop</a> and <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@RRect_offset"></fiddle-embed></div>

### See Also

<a href='#SkRRect_makeOffset'>makeOffset</a>  <a href='#SkRRect_inset'>inset outset</a>

<a name='SkRRect_makeOffset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkRRect_makeOffset'>makeOffset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy)const
</pre>

Returns <a href='SkRRect_Reference#SkRRect'>SkRRect</a> translated by (<a href='#SkRRect_makeOffset_dx'>dx</a>, <a href='#SkRRect_makeOffset_dy'>dy</a>).

### Parameters

<table>  <tr>    <td><a name='SkRRect_makeOffset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fLeft'>fLeft</a> and <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRRect_makeOffset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fTop'>fTop</a> and <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

<a href='SkRRect_Reference#SkRRect'>SkRRect</a> bounds offset by (<a href='#SkRRect_makeOffset_dx'>dx</a>, <a href='#SkRRect_makeOffset_dy'>dy</a>), with unchanged corner radii

### Example

<div><fiddle-embed name="@RRect_makeOffset"></fiddle-embed></div>

### See Also

<a href='#SkRRect_offset'>offset</a>  <a href='#SkRRect_inset'>inset outset</a>

<a name='SkRRect_contains'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool contains(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>)const
</pre>

Returns true if <a href='#SkRRect_contains_rect'>rect</a> is inside the bounds and corner radii, and if
<a href='SkRRect_Reference#SkRRect'>SkRRect</a> and <a href='#SkRRect_contains_rect'>rect</a> are not empty.

### Parameters

<table>  <tr>    <td><a name='SkRRect_contains_rect'><code><strong>rect</strong></code></a></td>
    <td>area tested for containment</td>
  </tr>
</table>

### Return Value

true if <a href='SkRRect_Reference#SkRRect'>SkRRect</a> contains <a href='#SkRRect_contains_rect'>rect</a>

### Example

<div><fiddle-embed name="0bb057140e4119234bdd2e8dd2f0fa19"></fiddle-embed></div>

### See Also

<a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_contains'>contains</a>

<a name='SkRRect_isValid'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_isValid'>isValid</a>()const
</pre>

Returns true if bounds and radii values are finite and describe a <a href='SkRRect_Reference#SkRRect'>SkRRect</a>
<a href='SkRRect_Reference#SkRRect'>SkRRect</a>::<a href='#SkRRect_Type'>Type</a> that matches <a href='#SkRRect_getType'>getType</a>(). All <a href='SkRRect_Reference#SkRRect'>SkRRect</a> methods construct valid types,
even if the input values are not valid. Invalid <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='undocumented#Data'>data</a> can only
be generated by corrupting memory.

### Return Value

true if bounds and radii match <a href='#SkRRect_type'>type()</a>

### Example

<div><fiddle-embed name="8cc1f21c98c0416f7724ad218f557a00"></fiddle-embed></div>

### See Also

<a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_getType'>getType</a>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kSizeInMemory'><code>SkRRect::kSizeInMemory</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>48</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Space required to write the contents of <a href='SkRRect_Reference#SkRRect'>SkRRect</a> into a buffer; always a multiple of four.
</td>
  </tr>
</table>

<a name='SkRRect_writeToMemory'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkRRect_writeToMemory'>writeToMemory</a>(void* buffer)const
</pre>

Writes <a href='SkRRect_Reference#SkRRect'>SkRRect</a> to <a href='#SkRRect_writeToMemory_buffer'>buffer</a>. Writes <a href='#SkRRect_kSizeInMemory'>kSizeInMemory</a> bytes, and returns
<a href='#SkRRect_kSizeInMemory'>kSizeInMemory</a>, the number of bytes written.

### Parameters

<table>  <tr>    <td><a name='SkRRect_writeToMemory_buffer'><code><strong>buffer</strong></code></a></td>
    <td>storage for <a href='SkRRect_Reference#SkRRect'>SkRRect</a></td>
  </tr>
</table>

### Return Value

bytes written, <a href='#SkRRect_kSizeInMemory'>kSizeInMemory</a>

### Example

<div><fiddle-embed name="d6f5a3d21727ddc15e10ef4d5103ff91"></fiddle-embed></div>

### See Also

<a href='#SkRRect_readFromMemory'>readFromMemory</a>

<a name='SkRRect_readFromMemory'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkRRect_readFromMemory'>readFromMemory</a>(const void* buffer, size_t length)
</pre>

Reads <a href='SkRRect_Reference#SkRRect'>SkRRect</a> from <a href='#SkRRect_readFromMemory_buffer'>buffer</a>, reading <a href='#SkRRect_kSizeInMemory'>kSizeInMemory</a> bytes.
Returns <a href='#SkRRect_kSizeInMemory'>kSizeInMemory</a>, bytes read if <a href='#SkRRect_readFromMemory_length'>length</a> is at least <a href='#SkRRect_kSizeInMemory'>kSizeInMemory</a>.
Otherwise, returns zero.

### Parameters

<table>  <tr>    <td><a name='SkRRect_readFromMemory_buffer'><code><strong>buffer</strong></code></a></td>
    <td>memory to read from</td>
  </tr>
  <tr>    <td><a name='SkRRect_readFromMemory_length'><code><strong>length</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> of <a href='#SkRRect_readFromMemory_buffer'>buffer</a></td>
  </tr>
</table>

### Return Value

bytes read, or 0 if <a href='#SkRRect_readFromMemory_length'>length</a> is less than <a href='#SkRRect_kSizeInMemory'>kSizeInMemory</a>

### Example

<div><fiddle-embed name="50969745cf2b23544362f4cff5592b75"></fiddle-embed></div>

### See Also

<a href='#SkRRect_writeToMemory'>writeToMemory</a>

<a name='SkRRect_transform'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool transform(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#Matrix'>matrix</a>, <a href='SkRRect_Reference#SkRRect'>SkRRect</a>* dst)const
</pre>

Transforms by <a href='SkRRect_Reference#SkRRect'>SkRRect</a> by <a href='#SkRRect_transform_matrix'>matrix</a>, storing result in <a href='#SkRRect_transform_dst'>dst</a>.
Returns true if <a href='SkRRect_Reference#SkRRect'>SkRRect</a> transformed can be represented by another <a href='SkRRect_Reference#SkRRect'>SkRRect</a>.
Returns false if <a href='#SkRRect_transform_matrix'>matrix</a> contains transformations other than scale and translate.

Asserts in debug builds if <a href='SkRRect_Reference#SkRRect'>SkRRect</a> equals <a href='#SkRRect_transform_dst'>dst</a>.

### Parameters

<table>  <tr>    <td><a name='SkRRect_transform_matrix'><code><strong>matrix</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> specifying the transform</td>
  </tr>
  <tr>    <td><a name='SkRRect_transform_dst'><code><strong>dst</strong></code></a></td>
    <td><a href='SkRRect_Reference#SkRRect'>SkRRect</a> to store the result</td>
  </tr>
</table>

### Return Value

true if transformation succeeded.

### Example

<div><fiddle-embed name="68a5d24f22e2d798608fce8a20e47fd0"></fiddle-embed></div>

### See Also

<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_transform'>transform</a>

<a name='SkRRect_dump'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_dump'>dump</a>(bool asHex)const
</pre>

Writes <a href='undocumented#Text'>text</a> representation of <a href='SkRRect_Reference#SkRRect'>SkRRect</a> to standard output.
Set <a href='#SkRRect_dump_asHex'>asHex</a> true to generate exact binary representations
of floating <a href='SkPoint_Reference#Point'>point</a> numbers.

### Parameters

<table>  <tr>    <td><a name='SkRRect_dump_asHex'><code><strong>asHex</strong></code></a></td>
    <td>true if <a href='undocumented#SkScalar'>SkScalar</a> values are written as hexadecimal</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@RRect_dump">

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

<a href='#SkRRect_dumpHex'>dumpHex</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_dump'>dump</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_dump'>dump</a> <a href='undocumented#SkPathMeasure'>SkPathMeasure</a>::<a href='#SkPathMeasure_dump'>dump</a>

<a name='SkRRect_dump_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_dump'>dump()</a>const
</pre>

Writes <a href='undocumented#Text'>text</a> representation of <a href='SkRRect_Reference#SkRRect'>SkRRect</a> to standard output. The representation
may be directly compiled as C++ code. Floating <a href='SkPoint_Reference#Point'>point</a> values are written
with limited precision; it may not be possible to reconstruct original
<a href='SkRRect_Reference#SkRRect'>SkRRect</a> from output.

### Example

<div><fiddle-embed name="@RRect_dump_2">

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

<a href='#SkRRect_dumpHex'>dumpHex</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_dump'>dump</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_dump'>dump</a> <a href='undocumented#SkPathMeasure'>SkPathMeasure</a>::<a href='#SkPathMeasure_dump'>dump</a>

<a name='SkRRect_dumpHex'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_dumpHex'>dumpHex</a>()const
</pre>

Writes <a href='undocumented#Text'>text</a> representation of <a href='SkRRect_Reference#SkRRect'>SkRRect</a> to standard output. The representation
may be directly compiled as C++ code. Floating <a href='SkPoint_Reference#Point'>point</a> values are written
in hexadecimal to preserve their exact bit pattern. The output reconstructs the
original <a href='SkRRect_Reference#SkRRect'>SkRRect</a>.

### Example

<div><fiddle-embed name="@RRect_dumpHex">

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

<a href='#SkRRect_dump'>dump</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_dumpHex'>dumpHex</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_dumpHex'>dumpHex</a>

