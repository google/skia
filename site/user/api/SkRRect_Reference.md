SkRRect Reference
===


<a name='SkRRect'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkRRect_Reference#SkRRect'>SkRRect</a> {
<a href='SkRRect_Reference#SkRRect'>public</a>:
    <a href='#SkRRect_empty_constructor'>SkRRect()</a> = <a href='SkRRect_Reference#SkRRect'>default</a>;
    <a href='SkRRect_Reference#SkRRect'>SkRRect</a>(<a href='SkRRect_Reference#SkRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>rrect</a>) = <a href='SkRRect_Reference#SkRRect'>default</a>;
    <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>operator</a>=(<a href='SkRRect_Reference#SkRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>rrect</a>) = <a href='SkRRect_Reference#SkRRect'>default</a>;

    <a href='SkRRect_Reference#SkRRect'>enum</a> <a href='#SkRRect_Type'>Type</a> {
        <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>,
        <a href='#SkRRect_kRect_Type'>kRect_Type</a>,
        <a href='#SkRRect_kOval_Type'>kOval_Type</a>,
        <a href='#SkRRect_kSimple_Type'>kSimple_Type</a>,
        <a href='#SkRRect_kNinePatch_Type'>kNinePatch_Type</a>,
        <a href='#SkRRect_kComplex_Type'>kComplex_Type</a>,
        <a href='#SkRRect_kLastType'>kLastType</a>       = <a href='#SkRRect_kComplex_Type'>kComplex_Type</a>,
    };

    <a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_getType'>getType</a>() <a href='#SkRRect_getType'>const</a>;
    <a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_type'>type()</a> <a href='#SkRRect_type'>const</a>;
    <a href='#SkRRect_type'>bool</a> <a href='#SkRRect_isEmpty'>isEmpty</a>() <a href='#SkRRect_isEmpty'>const</a>;
    <a href='#SkRRect_isEmpty'>bool</a> <a href='#SkRRect_isRect'>isRect</a>() <a href='#SkRRect_isRect'>const</a>;
    <a href='#SkRRect_isRect'>bool</a> <a href='#SkRRect_isOval'>isOval</a>() <a href='#SkRRect_isOval'>const</a>;
    <a href='#SkRRect_isOval'>bool</a> <a href='#SkRRect_isSimple'>isSimple</a>() <a href='#SkRRect_isSimple'>const</a>;
    <a href='#SkRRect_isSimple'>bool</a> <a href='#SkRRect_isNinePatch'>isNinePatch</a>() <a href='#SkRRect_isNinePatch'>const</a>;
    <a href='#SkRRect_isNinePatch'>bool</a> <a href='#SkRRect_isComplex'>isComplex</a>() <a href='#SkRRect_isComplex'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRRect_width'>width()</a> <a href='#SkRRect_width'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRRect_height'>height()</a> <a href='#SkRRect_height'>const</a>;
    <a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='#SkRRect_getSimpleRadii'>getSimpleRadii</a>() <a href='#SkRRect_getSimpleRadii'>const</a>;
    <a href='#SkRRect_getSimpleRadii'>void</a> <a href='#SkRRect_setEmpty'>setEmpty</a>();
    <a href='#SkRRect_setEmpty'>void</a> <a href='#SkRRect_setRect'>setRect</a>(<a href='#SkRRect_setRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>);
    <a href='SkRect_Reference#Rect'>static</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkRRect_MakeEmpty'>MakeEmpty</a>();
    <a href='#SkRRect_MakeEmpty'>static</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkRRect_MakeRect'>MakeRect</a>(<a href='#SkRRect_MakeRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>r</a>);
    <a href='SkRect_Reference#SkRect'>static</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkRRect_MakeOval'>MakeOval</a>(<a href='#SkRRect_MakeOval'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='undocumented#Oval'>oval</a>);
    <a href='undocumented#Oval'>static</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkRRect_MakeRectXY'>MakeRectXY</a>(<a href='#SkRRect_MakeRectXY'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>xRad</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>yRad</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkRRect_setOval'>setOval</a>(<a href='#SkRRect_setOval'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='undocumented#Oval'>oval</a>);
    <a href='undocumented#Oval'>void</a> <a href='#SkRRect_setRectXY'>setRectXY</a>(<a href='#SkRRect_setRectXY'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>xRad</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>yRad</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkRRect_setNinePatch'>setNinePatch</a>(<a href='#SkRRect_setNinePatch'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>leftRad</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>topRad</a>,
                      <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>rightRad</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>bottomRad</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkRRect_setRectRadii'>setRectRadii</a>(<a href='#SkRRect_setRectRadii'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='SkRect_Reference#Rect'>const</a> <a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='SkPoint_Reference#SkVector'>radii</a>[4]);

    <a href='SkPoint_Reference#SkVector'>enum</a> <a href='#SkRRect_Corner'>Corner</a> {
        <a href='#SkRRect_kUpperLeft_Corner'>kUpperLeft_Corner</a>,
        <a href='#SkRRect_kUpperRight_Corner'>kUpperRight_Corner</a>,
        <a href='#SkRRect_kLowerRight_Corner'>kLowerRight_Corner</a>,
        <a href='#SkRRect_kLowerLeft_Corner'>kLowerLeft_Corner</a>,
    };

    <a href='#SkRRect_kLowerLeft_Corner'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkRRect_rect'>rect()</a> <a href='#SkRRect_rect'>const</a>;
    <a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='SkPoint_Reference#SkVector'>radii</a>(<a href='#SkRRect_Corner'>Corner</a> <a href='#SkRRect_Corner'>corner</a>) <a href='#SkRRect_Corner'>const</a>;
    <a href='#SkRRect_Corner'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkRRect_getBounds'>getBounds</a>() <a href='#SkRRect_getBounds'>const</a>;
    <a href='#SkRRect_getBounds'>friend</a> <a href='#SkRRect_getBounds'>bool</a> <a href='#SkRRect_getBounds'>operator</a>==(<a href='#SkRRect_getBounds'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>a</a>, <a href='SkRRect_Reference#SkRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>b</a>);
    <a href='SkRRect_Reference#SkRRect'>friend</a> <a href='SkRRect_Reference#SkRRect'>bool</a> <a href='SkRRect_Reference#SkRRect'>operator</a>!=(<a href='SkRRect_Reference#SkRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>a</a>, <a href='SkRRect_Reference#SkRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>b</a>);
    <a href='SkRRect_Reference#SkRRect'>void</a> <a href='SkRRect_Reference#SkRRect'>inset</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>, <a href='SkRRect_Reference#SkRRect'>SkRRect</a>* <a href='SkRRect_Reference#SkRRect'>dst</a>) <a href='SkRRect_Reference#SkRRect'>const</a>;
    <a href='SkRRect_Reference#SkRRect'>void</a> <a href='SkRRect_Reference#SkRRect'>inset</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='undocumented#SkScalar'>outset</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>, <a href='SkRRect_Reference#SkRRect'>SkRRect</a>* <a href='SkRRect_Reference#SkRRect'>dst</a>) <a href='SkRRect_Reference#SkRRect'>const</a>;
    <a href='SkRRect_Reference#SkRRect'>void</a> <a href='SkRRect_Reference#SkRRect'>outset</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='undocumented#SkScalar'>offset</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>);
    <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkRRect_makeOffset'>makeOffset</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>) <a href='undocumented#SkScalar'>const</a>;
    <a href='undocumented#SkScalar'>bool</a> <a href='undocumented#SkScalar'>contains</a>(<a href='undocumented#SkScalar'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>) <a href='SkRect_Reference#Rect'>const</a>;
    <a href='SkRect_Reference#Rect'>bool</a> <a href='#SkRRect_isValid'>isValid</a>() <a href='#SkRRect_isValid'>const</a>;

    <a href='#SkRRect_isValid'>static</a> <a href='#SkRRect_isValid'>constexpr</a> <a href='#SkRRect_isValid'>size_t</a> <a href='#SkRRect_kSizeInMemory'>kSizeInMemory</a> = 12 * <a href='undocumented#sizeof()'>sizeof</a>(<a href='undocumented#SkScalar'>SkScalar</a>);

    <a href='undocumented#SkScalar'>size_t</a> <a href='#SkRRect_writeToMemory'>writeToMemory</a>(<a href='#SkRRect_writeToMemory'>void</a>* <a href='#SkRRect_writeToMemory'>buffer</a>) <a href='#SkRRect_writeToMemory'>const</a>;
    <a href='#SkRRect_writeToMemory'>size_t</a> <a href='#SkRRect_readFromMemory'>readFromMemory</a>(<a href='#SkRRect_readFromMemory'>const</a> <a href='#SkRRect_readFromMemory'>void</a>* <a href='#SkRRect_readFromMemory'>buffer</a>, <a href='#SkRRect_readFromMemory'>size_t</a> <a href='#SkRRect_readFromMemory'>length</a>);
    <a href='#SkRRect_readFromMemory'>bool</a> <a href='#SkRRect_readFromMemory'>transform</a>(<a href='#SkRRect_readFromMemory'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#Matrix'>matrix</a>, <a href='SkRRect_Reference#SkRRect'>SkRRect</a>* <a href='SkRRect_Reference#SkRRect'>dst</a>) <a href='SkRRect_Reference#SkRRect'>const</a>;
    <a href='SkRRect_Reference#SkRRect'>void</a> <a href='#SkRRect_dump'>dump</a>(<a href='#SkRRect_dump'>bool</a> <a href='#SkRRect_dump'>asHex</a>) <a href='#SkRRect_dump'>const</a>;
    <a href='#SkRRect_dump'>void</a> <a href='#SkRRect_dump'>dump()</a> <a href='#SkRRect_dump'>const</a>;
    <a href='#SkRRect_dump'>void</a> <a href='#SkRRect_dumpHex'>dumpHex</a>() <a href='#SkRRect_dumpHex'>const</a>;
};
</pre>

<a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>describes</a> <a href='SkRRect_Reference#SkRRect'>a</a> <a href='SkRRect_Reference#SkRRect'>rounded</a> <a href='SkRRect_Reference#SkRRect'>rectangle</a> <a href='SkRRect_Reference#SkRRect'>with</a> <a href='SkRRect_Reference#SkRRect'>a</a> <a href='SkRRect_Reference#SkRRect'>bounds</a> <a href='SkRRect_Reference#SkRRect'>and</a> <a href='SkRRect_Reference#SkRRect'>a</a> <a href='SkRRect_Reference#SkRRect'>pair</a> <a href='SkRRect_Reference#SkRRect'>of</a> <a href='SkRRect_Reference#SkRRect'>radii</a> <a href='SkRRect_Reference#SkRRect'>for</a> <a href='SkRRect_Reference#SkRRect'>each</a> <a href='SkRRect_Reference#SkRRect'>corner</a>.
<a href='SkRRect_Reference#SkRRect'>The</a> <a href='SkRRect_Reference#SkRRect'>bounds</a> <a href='SkRRect_Reference#SkRRect'>and</a> <a href='SkRRect_Reference#SkRRect'>radii</a> <a href='SkRRect_Reference#SkRRect'>can</a> <a href='SkRRect_Reference#SkRRect'>be</a> <a href='SkRRect_Reference#SkRRect'>set</a> <a href='SkRRect_Reference#SkRRect'>so</a> <a href='SkRRect_Reference#SkRRect'>that</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>describes</a>: <a href='SkRRect_Reference#SkRRect'>a</a> <a href='SkRRect_Reference#SkRRect'>rectangle</a> <a href='SkRRect_Reference#SkRRect'>with</a> <a href='SkRRect_Reference#SkRRect'>sharp</a> <a href='SkRRect_Reference#SkRRect'>corners</a>;
<a href='SkRRect_Reference#SkRRect'>a</a> <a href='undocumented#Circle'>Circle</a>; <a href='undocumented#Circle'>an</a> <a href='undocumented#Oval'>Oval</a>; <a href='undocumented#Oval'>or</a> <a href='undocumented#Oval'>a</a> <a href='undocumented#Oval'>rectangle</a> <a href='undocumented#Oval'>with</a> <a href='undocumented#Oval'>one</a> <a href='undocumented#Oval'>or</a> <a href='undocumented#Oval'>more</a> <a href='undocumented#Oval'>rounded</a> <a href='undocumented#Oval'>corners</a>.

<a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>allows</a> <a href='SkRRect_Reference#SkRRect'>implementing</a> <a href='SkRRect_Reference#SkRRect'>CSS</a> <a href='SkRRect_Reference#SkRRect'>properties</a> <a href='SkRRect_Reference#SkRRect'>that</a> <a href='SkRRect_Reference#SkRRect'>describe</a> <a href='SkRRect_Reference#SkRRect'>rounded</a> <a href='SkRRect_Reference#SkRRect'>corners</a>.
<a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>may</a> <a href='SkRRect_Reference#SkRRect'>have</a> <a href='SkRRect_Reference#SkRRect'>up</a> <a href='SkRRect_Reference#SkRRect'>to</a> <a href='SkRRect_Reference#SkRRect'>eight</a> <a href='SkRRect_Reference#SkRRect'>different</a> <a href='SkRRect_Reference#SkRRect'>radii</a>, <a href='SkRRect_Reference#SkRRect'>one</a> <a href='SkRRect_Reference#SkRRect'>for</a> <a href='SkRRect_Reference#SkRRect'>each</a> <a href='SkRRect_Reference#SkRRect'>axis</a> <a href='SkRRect_Reference#SkRRect'>on</a> <a href='SkRRect_Reference#SkRRect'>each</a> <a href='SkRRect_Reference#SkRRect'>of</a> <a href='SkRRect_Reference#SkRRect'>its</a> <a href='SkRRect_Reference#SkRRect'>four</a>
<a href='SkRRect_Reference#SkRRect'>corners</a>.

<a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>may</a> <a href='SkRRect_Reference#SkRRect'>modify</a> <a href='SkRRect_Reference#SkRRect'>the</a> <a href='SkRRect_Reference#SkRRect'>provided</a> <a href='SkRRect_Reference#SkRRect'>parameters</a> <a href='SkRRect_Reference#SkRRect'>when</a> <a href='SkRRect_Reference#SkRRect'>initializing</a> <a href='SkRRect_Reference#SkRRect'>bounds</a> <a href='SkRRect_Reference#SkRRect'>and</a> <a href='SkRRect_Reference#SkRRect'>radii</a>.
<a href='SkRRect_Reference#SkRRect'>If</a> <a href='SkRRect_Reference#SkRRect'>either</a> <a href='SkRRect_Reference#SkRRect'>axis</a> <a href='SkRRect_Reference#SkRRect'>radii</a> <a href='SkRRect_Reference#SkRRect'>is</a> <a href='SkRRect_Reference#SkRRect'>zero</a> <a href='SkRRect_Reference#SkRRect'>or</a> <a href='SkRRect_Reference#SkRRect'>less</a>: <a href='SkRRect_Reference#SkRRect'>radii</a> <a href='SkRRect_Reference#SkRRect'>are</a> <a href='SkRRect_Reference#SkRRect'>stored</a> <a href='SkRRect_Reference#SkRRect'>as</a> <a href='SkRRect_Reference#SkRRect'>zero</a>; <a href='SkRRect_Reference#SkRRect'>corner</a> <a href='SkRRect_Reference#SkRRect'>is</a> <a href='SkRRect_Reference#SkRRect'>square</a>.
<a href='SkRRect_Reference#SkRRect'>If</a> <a href='SkRRect_Reference#SkRRect'>corner</a> <a href='undocumented#Curve'>curves</a> <a href='undocumented#Curve'>overlap</a>, <a href='undocumented#Curve'>radii</a> <a href='undocumented#Curve'>are</a> <a href='undocumented#Curve'>proportionally</a> <a href='undocumented#Curve'>reduced</a> <a href='undocumented#Curve'>to</a> <a href='undocumented#Curve'>fit</a> <a href='undocumented#Curve'>within</a> <a href='undocumented#Curve'>bounds</a>.

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

<div><fiddle-embed name="471e7aad0feaf9ec3a21757a317a64f5"></fiddle-embed></div>

### See Also

<a href='#SkRRect_setEmpty'>setEmpty</a> <a href='#SkRRect_isEmpty'>isEmpty</a>

<a name='SkRRect_copy_const_SkRRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkRRect_Reference#SkRRect'>SkRRect</a>(<a href='SkRRect_Reference#SkRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>rrect</a>)
</pre>

Initializes to copy of <a href='#SkRRect_copy_const_SkRRect_rrect'>rrect</a> <a href='#SkRRect_copy_const_SkRRect_rrect'>bounds</a> <a href='#SkRRect_copy_const_SkRRect_rrect'>and</a> <a href='#SkRRect_copy_const_SkRRect_rrect'>corner</a> <a href='#SkRRect_copy_const_SkRRect_rrect'>radii</a>.

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

<a name='SkRRect_copy_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>operator</a>=(<a href='SkRRect_Reference#SkRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>rrect</a>)
</pre>

Copies <a href='#SkRRect_copy_operator_rrect'>rrect</a> <a href='#SkRRect_copy_operator_rrect'>bounds</a> <a href='#SkRRect_copy_operator_rrect'>and</a> <a href='#SkRRect_copy_operator_rrect'>corner</a> <a href='#SkRRect_copy_operator_rrect'>radii</a>.

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

<a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_Type'>describes</a> <a href='#SkRRect_Type'>possible</a> <a href='#SkRRect_Type'>specializations</a> <a href='#SkRRect_Type'>of</a> <a href='#RRect'>Round_Rect</a>. <a href='#RRect'>Each</a> <a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_Type'>is</a>
<a href='#SkRRect_Type'>exclusive</a>; <a href='#SkRRect_Type'>a</a> <a href='#RRect'>Round_Rect</a> <a href='#RRect'>may</a> <a href='#RRect'>only</a> <a href='#RRect'>have</a> <a href='#RRect'>one</a> <a href='#RRect'>type</a>.

<a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_Type'>members</a> <a href='#SkRRect_Type'>become</a> <a href='#SkRRect_Type'>progressively</a> <a href='#SkRRect_Type'>less</a> <a href='#SkRRect_Type'>restrictive</a>; <a href='#SkRRect_Type'>larger</a> <a href='#SkRRect_Type'>values</a> <a href='#SkRRect_Type'>of</a>
<a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_Type'>have</a> <a href='#SkRRect_Type'>more</a> <a href='#SkRRect_Type'>degrees</a> <a href='#SkRRect_Type'>of</a> <a href='#SkRRect_Type'>freedom</a> <a href='#SkRRect_Type'>than</a> <a href='#SkRRect_Type'>smaller</a> <a href='#SkRRect_Type'>values</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kEmpty_Type'><code>SkRRect::kEmpty_Type</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#RRect'>Round_Rect</a> <a href='#RRect'>has</a> <a href='#RRect'>zero</a> <a href='#RRect'>width</a> <a href='#RRect'>or</a> <a href='#RRect'>height</a>. <a href='#RRect'>All</a> <a href='#RRect'>radii</a> <a href='#RRect'>are</a> <a href='#RRect'>zero</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kRect_Type'><code>SkRRect::kRect_Type</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#RRect'>Round_Rect</a> <a href='#RRect'>has</a> <a href='#RRect'>width</a> <a href='#RRect'>and</a> <a href='#RRect'>height</a>. <a href='#RRect'>All</a> <a href='#RRect'>radii</a> <a href='#RRect'>are</a> <a href='#RRect'>zero</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kOval_Type'><code>SkRRect::kOval_Type</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#RRect'>Round_Rect</a> <a href='#RRect'>has</a> <a href='#RRect'>width</a> <a href='#RRect'>and</a> <a href='#RRect'>height</a>. <a href='#RRect'>All</a> <a href='#RRect'>four</a> <a href='#RRect'>x-radii</a> <a href='#RRect'>are</a> <a href='#RRect'>equal</a>,
<a href='#RRect'>and</a> <a href='#RRect'>at</a> <a href='#RRect'>least</a> <a href='#RRect'>half</a> <a href='#RRect'>the</a> <a href='#RRect'>width</a>. <a href='#RRect'>All</a> <a href='#RRect'>four</a> <a href='#RRect'>y-radii</a> <a href='#RRect'>are</a> <a href='#RRect'>equal</a>,
<a href='#RRect'>and</a> <a href='#RRect'>at</a> <a href='#RRect'>least</a> <a href='#RRect'>half</a> <a href='#RRect'>the</a> <a href='#RRect'>height</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kSimple_Type'><code>SkRRect::kSimple_Type</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#RRect'>Round_Rect</a> <a href='#RRect'>has</a> <a href='#RRect'>width</a> <a href='#RRect'>and</a> <a href='#RRect'>height</a>. <a href='#RRect'>All</a> <a href='#RRect'>four</a> <a href='#RRect'>x-radii</a> <a href='#RRect'>are</a> <a href='#RRect'>equal</a> <a href='#RRect'>and</a>
<a href='#RRect'>greater</a> <a href='#RRect'>than</a> <a href='#RRect'>zero</a>, <a href='#RRect'>and</a> <a href='#RRect'>all</a> <a href='#RRect'>four</a> <a href='#RRect'>y-radii</a> <a href='#RRect'>are</a> <a href='#RRect'>equal</a> <a href='#RRect'>and</a> <a href='#RRect'>greater</a> <a href='#RRect'>than</a>
<a href='#RRect'>zero</a>. <a href='#RRect'>Either</a> <a href='#RRect'>x-radii</a> <a href='#RRect'>are</a> <a href='#RRect'>less</a> <a href='#RRect'>than</a> <a href='#RRect'>half</a> <a href='#RRect'>the</a> <a href='#RRect'>width</a>, <a href='#RRect'>or</a> <a href='#RRect'>y-radii</a> <a href='#RRect'>is</a>
<a href='#RRect'>less</a> <a href='#RRect'>than</a> <a href='#RRect'>half</a> <a href='#RRect'>the</a> <a href='#RRect'>height</a>, <a href='#RRect'>or</a> <a href='#RRect'>both</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRRect_kNinePatch_Type'><code>SkRRect::kNinePatch_Type</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#RRect'>Round_Rect</a> <a href='#RRect'>has</a> <a href='#RRect'>width</a> <a href='#RRect'>and</a> <a href='#RRect'>height</a>. <a href='#RRect'>Left</a> <a href='#RRect'>x-radii</a> <a href='#RRect'>are</a> <a href='#RRect'>equal</a>, <a href='#RRect'>top</a>
<a href='#RRect'>y-radii</a> <a href='#RRect'>are</a> <a href='#RRect'>equal</a>, <a href='#RRect'>right</a> <a href='#RRect'>x-radii</a> <a href='#RRect'>are</a> <a href='#RRect'>equal</a>, <a href='#RRect'>and</a> <a href='#RRect'>bottom</a> <a href='#RRect'>y-radii</a>
<a href='#RRect'>are</a> <a href='#RRect'>equal</a>. <a href='#RRect'>The</a> <a href='#RRect'>radii</a> <a href='#RRect'>do</a> <a href='#RRect'>not</a> <a href='#RRect'>describe</a> <a href='SkRect_Reference#Rect'>Rect</a>, <a href='undocumented#Oval'>Oval</a>, <a href='undocumented#Oval'>or</a> <a href='undocumented#Oval'>simple</a> <a href='undocumented#Oval'>type</a>.

<a href='undocumented#Oval'>The</a> <a href='undocumented#Oval'>centers</a> <a href='undocumented#Oval'>of</a> <a href='undocumented#Oval'>the</a> <a href='undocumented#Oval'>corner</a> <a href='undocumented#Oval'>ellipses</a> <a href='undocumented#Oval'>form</a> <a href='undocumented#Oval'>an</a> <a href='undocumented#Oval'>axis-aligned</a> <a href='undocumented#Oval'>rectangle</a>
<a href='undocumented#Oval'>that</a> <a href='undocumented#Oval'>divides</a> <a href='undocumented#Oval'>the</a> <a href='#RRect'>Round_Rect</a> <a href='#RRect'>into</a> <a href='#RRect'>nine</a> <a href='#RRect'>rectangular</a> <a href='undocumented#Patch'>patches</a>; <a href='undocumented#Patch'>an</a>
<a href='undocumented#Patch'>interior</a> <a href='undocumented#Patch'>rectangle</a>, <a href='undocumented#Patch'>four</a> <a href='undocumented#Patch'>edges</a>, <a href='undocumented#Patch'>and</a> <a href='undocumented#Patch'>four</a> <a href='undocumented#Patch'>corners</a>.
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
<a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_getType'>getType</a>() <a href='#SkRRect_getType'>const</a>
</pre>

Returns <a href='#SkRRect_Type'>Type</a>, <a href='#SkRRect_Type'>one</a> <a href='#SkRRect_Type'>of</a>: <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>, <a href='#SkRRect_kRect_Type'>kRect_Type</a>, <a href='#SkRRect_kOval_Type'>kOval_Type</a>, <a href='#SkRRect_kSimple_Type'>kSimple_Type</a>, <a href='#SkRRect_kNinePatch_Type'>kNinePatch_Type</a>,
<a href='#SkRRect_kComplex_Type'>kComplex_Type</a>
.

### Return Value

<a href='#SkRRect_Type'>Type</a>

### Example

<div><fiddle-embed name="ace8f4aebf90527d43e4b7291375c9ad"><div>rrect2 is not a <a href='SkRect_Reference#Rect'>Rect</a>; <a href='#SkRRect_inset'>inset()</a> <a href='#SkRRect_inset'>has</a> <a href='#SkRRect_inset'>made</a> <a href='#SkRRect_inset'>it</a> <a href='#SkRRect_inset'>empty</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_type'>type</a>

<a name='SkRRect_type'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_type'>type()</a> <a href='#SkRRect_type'>const</a>
</pre>

Returns <a href='#SkRRect_Type'>Type</a>, <a href='#SkRRect_Type'>one</a> <a href='#SkRRect_Type'>of</a>: <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>, <a href='#SkRRect_kRect_Type'>kRect_Type</a>, <a href='#SkRRect_kOval_Type'>kOval_Type</a>, <a href='#SkRRect_kSimple_Type'>kSimple_Type</a>, <a href='#SkRRect_kNinePatch_Type'>kNinePatch_Type</a>,
<a href='#SkRRect_kComplex_Type'>kComplex_Type</a>
.

### Return Value

<a href='#SkRRect_Type'>Type</a>

### Example

<div><fiddle-embed name="1080805c8449406a4e26d694bc56d2dc"><div><a href='#SkRRect_inset'>inset()</a> <a href='#SkRRect_inset'>has</a> <a href='#SkRRect_inset'>made</a> <a href='#SkRRect_inset'>rrect2</a> <a href='#SkRRect_inset'>empty</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_getType'>getType</a>

<a name='SkRRect_isEmpty'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_isEmpty'>isEmpty</a>() <a href='#SkRRect_isEmpty'>const</a>
</pre>

### Example

<div><fiddle-embed name="099d79ecfbdfb0a19c10deb7201859c3"></fiddle-embed></div>

### See Also

<a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_isEmpty'>isEmpty</a> <a href='#SkRRect_height'>height</a> <a href='#SkRRect_width'>width</a>

<a name='SkRRect_isRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_isRect'>isRect</a>() <a href='#SkRRect_isRect'>const</a>
</pre>

### Example

<div><fiddle-embed name="bc931c9a6eb8ffe7ea8d3fb47e07a475"></fiddle-embed></div>

### See Also

<a href='#SkRRect_isEmpty'>isEmpty</a> <a href='#SkRRect_radii'>radii</a>

<a name='SkRRect_isOval'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_isOval'>isOval</a>() <a href='#SkRRect_isOval'>const</a>
</pre>

### Example

<div><fiddle-embed name="4dfdb28d8343958425f2c1323fe8170d"><div>The first radii are scaled down proportionately until both x-axis and y-axis fit
within the bounds. After scaling, x-axis radius is smaller than half the width;
left <a href='#RRect'>Round_Rect</a> <a href='#RRect'>is</a> <a href='#RRect'>not</a> <a href='#RRect'>an</a> <a href='undocumented#Oval'>oval</a>. <a href='undocumented#Oval'>The</a> <a href='undocumented#Oval'>second</a> <a href='undocumented#Oval'>radii</a> <a href='undocumented#Oval'>are</a> <a href='undocumented#Oval'>equal</a> <a href='undocumented#Oval'>to</a> <a href='undocumented#Oval'>half</a> <a href='undocumented#Oval'>the</a>
<a href='undocumented#Oval'>dimensions</a>; <a href='undocumented#Oval'>right</a> <a href='#RRect'>Round_Rect</a> <a href='#RRect'>is</a> <a href='#RRect'>an</a> <a href='undocumented#Oval'>oval</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkRRect_isEmpty'>isEmpty</a> <a href='#SkRRect_isSimple'>isSimple</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawOval'>drawOval</a>

<a name='SkRRect_isSimple'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_isSimple'>isSimple</a>() <a href='#SkRRect_isSimple'>const</a>
</pre>

### Example

<div><fiddle-embed name="f6959ea422a7c6e98ddfad216a52c707"></fiddle-embed></div>

### See Also

<a href='#SkRRect_isEmpty'>isEmpty</a> <a href='#SkRRect_isRect'>isRect</a> <a href='#SkRRect_isOval'>isOval</a> <a href='#SkRRect_isNinePatch'>isNinePatch</a>

<a name='SkRRect_isNinePatch'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_isNinePatch'>isNinePatch</a>() <a href='#SkRRect_isNinePatch'>const</a>
</pre>

### Example

<div><fiddle-embed name="429f6dfd4cf6287df3c3c77fa7681c99"></fiddle-embed></div>

### See Also

<a href='#SkRRect_isEmpty'>isEmpty</a> <a href='#SkRRect_isRect'>isRect</a> <a href='#SkRRect_isOval'>isOval</a> <a href='#SkRRect_isSimple'>isSimple</a> <a href='#SkRRect_isComplex'>isComplex</a>

<a name='SkRRect_isComplex'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_isComplex'>isComplex</a>() <a href='#SkRRect_isComplex'>const</a>
</pre>

### Example

<div><fiddle-embed name="b62c183dc435d1fc091111fb2f3c3f8e"></fiddle-embed></div>

### See Also

<a href='#SkRRect_isEmpty'>isEmpty</a> <a href='#SkRRect_isRect'>isRect</a> <a href='#SkRRect_isOval'>isOval</a> <a href='#SkRRect_isSimple'>isSimple</a> <a href='#SkRRect_isNinePatch'>isNinePatch</a>

<a name='SkRRect_width'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRRect_width'>width()</a> <a href='#SkRRect_width'>const</a>
</pre>

Returns span on the x-axis. This does not check if result fits in 32-bit float;
result may be infinity.

### Return Value

<a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fRight'>fRight</a> <a href='#SkRect_fRight'>minus</a> <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fLeft'>fLeft</a>

### Example

<div><fiddle-embed name="c675a480b41dee157f84fa2550a2a53c"><div><a href='SkRRect_Reference#SkRRect'>SkRRect</a>::<a href='#SkRRect_MakeRect'>MakeRect</a> <a href='#SkRRect_MakeRect'>sorts</a> <a href='#SkRRect_MakeRect'>its</a> <a href='#SkRRect_MakeRect'>input</a>, <a href='#SkRRect_MakeRect'>so</a> <a href='#SkRRect_width'>width()</a> <a href='#SkRRect_width'>is</a> <a href='#SkRRect_width'>always</a> <a href='#SkRRect_width'>zero</a> <a href='#SkRRect_width'>or</a> <a href='#SkRRect_width'>larger</a>.
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
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRRect_height'>height()</a> <a href='#SkRRect_height'>const</a>
</pre>

Returns span on the y-axis. This does not check if result fits in 32-bit float;
result may be infinity.

### Return Value

<a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fBottom'>fBottom</a> <a href='#SkRect_fBottom'>minus</a> <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fTop'>fTop</a>

### Example

<div><fiddle-embed name="5a3eb1755164a7becec33cec6e6eca31"><div><a href='SkRRect_Reference#SkRRect'>SkRRect</a>::<a href='#SkRRect_MakeRect'>MakeRect</a> <a href='#SkRRect_MakeRect'>sorts</a> <a href='#SkRRect_MakeRect'>its</a> <a href='#SkRRect_MakeRect'>input</a>, <a href='#SkRRect_MakeRect'>so</a> <a href='#SkRRect_height'>height()</a> <a href='#SkRRect_height'>is</a> <a href='#SkRRect_height'>always</a> <a href='#SkRRect_height'>zero</a> <a href='#SkRRect_height'>or</a> <a href='#SkRRect_height'>larger</a>.
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
<a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='#SkRRect_getSimpleRadii'>getSimpleRadii</a>() <a href='#SkRRect_getSimpleRadii'>const</a>
</pre>

Returns top-left corner radii. If <a href='#SkRRect_type'>type()</a> <a href='#SkRRect_type'>returns</a> <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>, <a href='#SkRRect_kRect_Type'>kRect_Type</a>,
<a href='#SkRRect_kOval_Type'>kOval_Type</a>, <a href='#SkRRect_kOval_Type'>or</a> <a href='#SkRRect_kSimple_Type'>kSimple_Type</a>, <a href='#SkRRect_kSimple_Type'>returns</a> <a href='#SkRRect_kSimple_Type'>a</a> <a href='#SkRRect_kSimple_Type'>value</a> <a href='#SkRRect_kSimple_Type'>representative</a> <a href='#SkRRect_kSimple_Type'>of</a> <a href='#SkRRect_kSimple_Type'>all</a> <a href='#SkRRect_kSimple_Type'>corner</a> <a href='#SkRRect_kSimple_Type'>radii</a>.
If <a href='#SkRRect_type'>type()</a> <a href='#SkRRect_type'>returns</a> <a href='#SkRRect_kNinePatch_Type'>kNinePatch_Type</a> <a href='#SkRRect_kNinePatch_Type'>or</a> <a href='#SkRRect_kComplex_Type'>kComplex_Type</a>, <a href='#SkRRect_kComplex_Type'>at</a> <a href='#SkRRect_kComplex_Type'>least</a> <a href='#SkRRect_kComplex_Type'>one</a> <a href='#SkRRect_kComplex_Type'>of</a> <a href='#SkRRect_kComplex_Type'>the</a>
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

<div><fiddle-embed name="44e9a9c2c5ef1af2a616086ff46a9037"><div>Nothing blue is drawn because <a href='#RRect'>Round_Rect</a> <a href='#RRect'>is</a> <a href='#RRect'>set</a> <a href='#RRect'>to</a> <a href='#RRect'>empty</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkRRect_MakeEmpty'>MakeEmpty</a> <a href='#SkRRect_setRect'>setRect</a>

<a name='SkRRect_setRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_setRect'>setRect</a>(<a href='#SkRRect_setRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>)
</pre>

Sets bounds to sorted <a href='#SkRRect_setRect_rect'>rect</a>, <a href='#SkRRect_setRect_rect'>and</a> <a href='#SkRRect_setRect_rect'>sets</a> <a href='#SkRRect_setRect_rect'>corner</a> <a href='#SkRRect_setRect_rect'>radii</a> <a href='#SkRRect_setRect_rect'>to</a> <a href='#SkRRect_setRect_rect'>zero</a>.
If set bounds has width and height, and sets type to <a href='#SkRRect_kRect_Type'>kRect_Type</a>;
otherwise, sets type to <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>.

### Parameters

<table>  <tr>    <td><a name='SkRRect_setRect_rect'><code><strong>rect</strong></code></a></td>
    <td>bounds to set</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3afc3ac9bebd1d7387822cc608571e82"></fiddle-embed></div>

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
static <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkRRect_MakeRect'>MakeRect</a>(<a href='#SkRRect_MakeRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>r</a>)
</pre>

Initializes to copy of <a href='#SkRRect_MakeRect_r'>r</a> <a href='#SkRRect_MakeRect_r'>bounds</a> <a href='#SkRRect_MakeRect_r'>and</a> <a href='#SkRRect_MakeRect_r'>zeroes</a> <a href='#SkRRect_MakeRect_r'>corner</a> <a href='#SkRRect_MakeRect_r'>radii</a>.

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

<a name='SkRRect_MakeOval'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkRRect_MakeOval'>MakeOval</a>(<a href='#SkRRect_MakeOval'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='undocumented#Oval'>oval</a>)
</pre>

Sets bounds to <a href='#SkRRect_MakeOval_oval'>oval</a>, <a href='#SkRRect_MakeOval_oval'>x-axis</a> <a href='#SkRRect_MakeOval_oval'>radii</a> <a href='#SkRRect_MakeOval_oval'>to</a> <a href='#SkRRect_MakeOval_oval'>half</a> <a href='#SkRRect_MakeOval_oval'>oval</a>.<a href='#SkRect_width'>width()</a>, <a href='#SkRect_width'>and</a> <a href='#SkRect_width'>all</a> <a href='#SkRect_width'>y-axis</a> <a href='#SkRect_width'>radii</a>
to half <a href='#SkRRect_MakeOval_oval'>oval</a>.<a href='#SkRect_height'>height()</a>. <a href='#SkRect_height'>If</a> <a href='#SkRRect_MakeOval_oval'>oval</a> <a href='#SkRRect_MakeOval_oval'>bounds</a> <a href='#SkRRect_MakeOval_oval'>is</a> <a href='#SkRRect_MakeOval_oval'>empty</a>, <a href='#SkRRect_MakeOval_oval'>sets</a> <a href='#SkRRect_MakeOval_oval'>to</a> <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>.
Otherwise, sets to <a href='#SkRRect_kOval_Type'>kOval_Type</a>.

### Parameters

<table>  <tr>    <td><a name='SkRRect_MakeOval_oval'><code><strong>oval</strong></code></a></td>
    <td>bounds of <a href='#SkRRect_MakeOval_oval'>oval</a></td>
  </tr>
</table>

### Return Value

<a href='#SkRRect_MakeOval_oval'>oval</a>

### Example

<div><fiddle-embed name="0b99ee38fd154f769f6031242e02fa7a"></fiddle-embed></div>

### See Also

<a href='#SkRRect_setOval'>setOval</a> <a href='#SkRRect_MakeRect'>MakeRect</a> <a href='#SkRRect_MakeRectXY'>MakeRectXY</a>

<a name='SkRRect_MakeRectXY'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkRRect_MakeRectXY'>MakeRectXY</a>(<a href='#SkRRect_MakeRectXY'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>xRad</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>yRad</a>)
</pre>

Sets to rounded rectangle with the same radii for all four corners.
If <a href='#SkRRect_MakeRectXY_rect'>rect</a> <a href='#SkRRect_MakeRectXY_rect'>is</a> <a href='#SkRRect_MakeRectXY_rect'>empty</a>, <a href='#SkRRect_MakeRectXY_rect'>sets</a> <a href='#SkRRect_MakeRectXY_rect'>to</a> <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>.
Otherwise, if <a href='#SkRRect_MakeRectXY_xRad'>xRad</a> <a href='#SkRRect_MakeRectXY_xRad'>and</a> <a href='#SkRRect_MakeRectXY_yRad'>yRad</a> <a href='#SkRRect_MakeRectXY_yRad'>are</a> <a href='#SkRRect_MakeRectXY_yRad'>zero</a>, <a href='#SkRRect_MakeRectXY_yRad'>sets</a> <a href='#SkRRect_MakeRectXY_yRad'>to</a> <a href='#SkRRect_kRect_Type'>kRect_Type</a>.
Otherwise, if <a href='#SkRRect_MakeRectXY_xRad'>xRad</a> <a href='#SkRRect_MakeRectXY_xRad'>is</a> <a href='#SkRRect_MakeRectXY_xRad'>at</a> <a href='#SkRRect_MakeRectXY_xRad'>least</a> <a href='#SkRRect_MakeRectXY_xRad'>half</a> <a href='#SkRRect_MakeRectXY_rect'>rect</a>.<a href='#SkRect_width'>width()</a> <a href='#SkRect_width'>and</a> <a href='#SkRRect_MakeRectXY_yRad'>yRad</a> <a href='#SkRRect_MakeRectXY_yRad'>is</a> <a href='#SkRRect_MakeRectXY_yRad'>at</a> <a href='#SkRRect_MakeRectXY_yRad'>least</a> <a href='#SkRRect_MakeRectXY_yRad'>half</a>
<a href='#SkRRect_MakeRectXY_rect'>rect</a>.<a href='#SkRect_height'>height()</a>, <a href='#SkRect_height'>sets</a> <a href='#SkRect_height'>to</a> <a href='#SkRRect_kOval_Type'>kOval_Type</a>.
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

<div><fiddle-embed name="2b24a1247637cbc94f8b3c77d37ed3e2"></fiddle-embed></div>

### See Also

<a href='#SkRRect_setRectXY'>setRectXY</a>

<a name='SkRRect_setOval'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_setOval'>setOval</a>(<a href='#SkRRect_setOval'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='undocumented#Oval'>oval</a>)
</pre>

Sets bounds to <a href='#SkRRect_setOval_oval'>oval</a>, <a href='#SkRRect_setOval_oval'>x-axis</a> <a href='#SkRRect_setOval_oval'>radii</a> <a href='#SkRRect_setOval_oval'>to</a> <a href='#SkRRect_setOval_oval'>half</a> <a href='#SkRRect_setOval_oval'>oval</a>.<a href='#SkRect_width'>width()</a>, <a href='#SkRect_width'>and</a> <a href='#SkRect_width'>all</a> <a href='#SkRect_width'>y-axis</a> <a href='#SkRect_width'>radii</a>
to half <a href='#SkRRect_setOval_oval'>oval</a>.<a href='#SkRect_height'>height()</a>. <a href='#SkRect_height'>If</a> <a href='#SkRRect_setOval_oval'>oval</a> <a href='#SkRRect_setOval_oval'>bounds</a> <a href='#SkRRect_setOval_oval'>is</a> <a href='#SkRRect_setOval_oval'>empty</a>, <a href='#SkRRect_setOval_oval'>sets</a> <a href='#SkRRect_setOval_oval'>to</a> <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>.
Otherwise, sets to <a href='#SkRRect_kOval_Type'>kOval_Type</a>.

### Parameters

<table>  <tr>    <td><a name='SkRRect_setOval_oval'><code><strong>oval</strong></code></a></td>
    <td>bounds of <a href='#SkRRect_setOval_oval'>oval</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="cf418af29cbab6243ac16aacd1217ffe"></fiddle-embed></div>

### See Also

<a href='#SkRRect_MakeOval'>MakeOval</a>

<a name='SkRRect_setRectXY'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_setRectXY'>setRectXY</a>(<a href='#SkRRect_setRectXY'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>xRad</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>yRad</a>)
</pre>

Sets to rounded rectangle with the same radii for all four corners.
If <a href='#SkRRect_setRectXY_rect'>rect</a> <a href='#SkRRect_setRectXY_rect'>is</a> <a href='#SkRRect_setRectXY_rect'>empty</a>, <a href='#SkRRect_setRectXY_rect'>sets</a> <a href='#SkRRect_setRectXY_rect'>to</a> <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>.
Otherwise, if <a href='#SkRRect_setRectXY_xRad'>xRad</a> <a href='#SkRRect_setRectXY_xRad'>or</a> <a href='#SkRRect_setRectXY_yRad'>yRad</a> <a href='#SkRRect_setRectXY_yRad'>is</a> <a href='#SkRRect_setRectXY_yRad'>zero</a>, <a href='#SkRRect_setRectXY_yRad'>sets</a> <a href='#SkRRect_setRectXY_yRad'>to</a> <a href='#SkRRect_kRect_Type'>kRect_Type</a>.
Otherwise, if <a href='#SkRRect_setRectXY_xRad'>xRad</a> <a href='#SkRRect_setRectXY_xRad'>is</a> <a href='#SkRRect_setRectXY_xRad'>at</a> <a href='#SkRRect_setRectXY_xRad'>least</a> <a href='#SkRRect_setRectXY_xRad'>half</a> <a href='#SkRRect_setRectXY_rect'>rect</a>.<a href='#SkRect_width'>width()</a> <a href='#SkRect_width'>and</a> <a href='#SkRRect_setRectXY_yRad'>yRad</a> <a href='#SkRRect_setRectXY_yRad'>is</a> <a href='#SkRRect_setRectXY_yRad'>at</a> <a href='#SkRRect_setRectXY_yRad'>least</a> <a href='#SkRRect_setRectXY_yRad'>half</a>
<a href='#SkRRect_setRectXY_rect'>rect</a>.<a href='#SkRect_height'>height()</a>, <a href='#SkRect_height'>sets</a> <a href='#SkRect_height'>to</a> <a href='#SkRRect_kOval_Type'>kOval_Type</a>.
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

<div><fiddle-embed name="6ac569e40fb68c758319e85428b9ae95"></fiddle-embed></div>

### See Also

<a href='#SkRRect_MakeRectXY'>MakeRectXY</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_addRoundRect'>addRoundRect</a>

<a name='SkRRect_setNinePatch'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_setNinePatch'>setNinePatch</a>(<a href='#SkRRect_setNinePatch'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>leftRad</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>topRad</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>rightRad</a>,
                  <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>bottomRad</a>)
</pre>

Sets bounds to <a href='#SkRRect_setNinePatch_rect'>rect</a>. <a href='#SkRRect_setNinePatch_rect'>Sets</a> <a href='#SkRRect_setNinePatch_rect'>radii</a> <a href='#SkRRect_setNinePatch_rect'>to</a> (<a href='#SkRRect_setNinePatch_leftRad'>leftRad</a>, <a href='#SkRRect_setNinePatch_topRad'>topRad</a>), (<a href='#SkRRect_setNinePatch_rightRad'>rightRad</a>, <a href='#SkRRect_setNinePatch_topRad'>topRad</a>),
(<a href='#SkRRect_setNinePatch_rightRad'>rightRad</a>, <a href='#SkRRect_setNinePatch_bottomRad'>bottomRad</a>), (<a href='#SkRRect_setNinePatch_leftRad'>leftRad</a>, <a href='#SkRRect_setNinePatch_bottomRad'>bottomRad</a>).

If <a href='#SkRRect_setNinePatch_rect'>rect</a> <a href='#SkRRect_setNinePatch_rect'>is</a> <a href='#SkRRect_setNinePatch_rect'>empty</a>, <a href='#SkRRect_setNinePatch_rect'>sets</a> <a href='#SkRRect_setNinePatch_rect'>to</a> <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>.
Otherwise, if <a href='#SkRRect_setNinePatch_leftRad'>leftRad</a> <a href='#SkRRect_setNinePatch_leftRad'>and</a> <a href='#SkRRect_setNinePatch_rightRad'>rightRad</a> <a href='#SkRRect_setNinePatch_rightRad'>are</a> <a href='#SkRRect_setNinePatch_rightRad'>zero</a>, <a href='#SkRRect_setNinePatch_rightRad'>sets</a> <a href='#SkRRect_setNinePatch_rightRad'>to</a> <a href='#SkRRect_kRect_Type'>kRect_Type</a>.
Otherwise, if <a href='#SkRRect_setNinePatch_topRad'>topRad</a> <a href='#SkRRect_setNinePatch_topRad'>and</a> <a href='#SkRRect_setNinePatch_bottomRad'>bottomRad</a> <a href='#SkRRect_setNinePatch_bottomRad'>are</a> <a href='#SkRRect_setNinePatch_bottomRad'>zero</a>, <a href='#SkRRect_setNinePatch_bottomRad'>sets</a> <a href='#SkRRect_setNinePatch_bottomRad'>to</a> <a href='#SkRRect_kRect_Type'>kRect_Type</a>.
Otherwise, if <a href='#SkRRect_setNinePatch_leftRad'>leftRad</a> <a href='#SkRRect_setNinePatch_leftRad'>and</a> <a href='#SkRRect_setNinePatch_rightRad'>rightRad</a> <a href='#SkRRect_setNinePatch_rightRad'>are</a> <a href='#SkRRect_setNinePatch_rightRad'>equal</a> <a href='#SkRRect_setNinePatch_rightRad'>and</a> <a href='#SkRRect_setNinePatch_rightRad'>at</a> <a href='#SkRRect_setNinePatch_rightRad'>least</a> <a href='#SkRRect_setNinePatch_rightRad'>half</a> <a href='#SkRRect_setNinePatch_rect'>rect</a>.<a href='#SkRect_width'>width()</a>, <a href='#SkRect_width'>and</a>
<a href='#SkRRect_setNinePatch_topRad'>topRad</a> <a href='#SkRRect_setNinePatch_topRad'>and</a> <a href='#SkRRect_setNinePatch_bottomRad'>bottomRad</a> <a href='#SkRRect_setNinePatch_bottomRad'>are</a> <a href='#SkRRect_setNinePatch_bottomRad'>equal</a> <a href='#SkRRect_setNinePatch_bottomRad'>at</a> <a href='#SkRRect_setNinePatch_bottomRad'>least</a> <a href='#SkRRect_setNinePatch_bottomRad'>half</a> <a href='#SkRRect_setNinePatch_rect'>rect</a>.<a href='#SkRect_height'>height()</a>, <a href='#SkRect_height'>sets</a> <a href='#SkRect_height'>to</a> <a href='#SkRRect_kOval_Type'>kOval_Type</a>.
Otherwise, if <a href='#SkRRect_setNinePatch_leftRad'>leftRad</a> <a href='#SkRRect_setNinePatch_leftRad'>and</a> <a href='#SkRRect_setNinePatch_rightRad'>rightRad</a> <a href='#SkRRect_setNinePatch_rightRad'>are</a> <a href='#SkRRect_setNinePatch_rightRad'>equal</a>, <a href='#SkRRect_setNinePatch_rightRad'>and</a> <a href='#SkRRect_setNinePatch_topRad'>topRad</a> <a href='#SkRRect_setNinePatch_topRad'>and</a> <a href='#SkRRect_setNinePatch_bottomRad'>bottomRad</a> <a href='#SkRRect_setNinePatch_bottomRad'>are</a> <a href='#SkRRect_setNinePatch_bottomRad'>equal</a>,
sets to <a href='#SkRRect_kSimple_Type'>kSimple_Type</a>. <a href='#SkRRect_kSimple_Type'>Otherwise</a>, <a href='#SkRRect_kSimple_Type'>sets</a> <a href='#SkRRect_kSimple_Type'>to</a> <a href='#SkRRect_kNinePatch_Type'>kNinePatch_Type</a>.

Nine <a href='undocumented#Patch'>patch</a> <a href='undocumented#Patch'>refers</a> <a href='undocumented#Patch'>to</a> <a href='undocumented#Patch'>the</a> <a href='undocumented#Patch'>nine</a> <a href='undocumented#Patch'>parts</a> <a href='undocumented#Patch'>defined</a> <a href='undocumented#Patch'>by</a> <a href='undocumented#Patch'>the</a> <a href='undocumented#Patch'>radii</a>: <a href='undocumented#Patch'>one</a> <a href='undocumented#Patch'>center</a> <a href='undocumented#Patch'>rectangle</a>,
four edge <a href='undocumented#Patch'>patches</a>, <a href='undocumented#Patch'>and</a> <a href='undocumented#Patch'>four</a> <a href='undocumented#Patch'>corner</a> <a href='undocumented#Patch'>patches</a>.

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

<div><fiddle-embed name="c4620df2eaba447b581688d3100053b1"></fiddle-embed></div>

### See Also

<a href='#SkRRect_setRectRadii'>setRectRadii</a>

<a name='SkRRect_setRectRadii'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_setRectRadii'>setRectRadii</a>(<a href='#SkRRect_setRectRadii'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='SkRect_Reference#Rect'>const</a> <a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='SkPoint_Reference#SkVector'>radii</a>[4])
</pre>

Sets bounds to <a href='#SkRRect_setRectRadii_rect'>rect</a>. <a href='#SkRRect_setRectRadii_rect'>Sets</a> <a href='#SkRRect_setRectRadii_radii'>radii</a> <a href='#SkRRect_setRectRadii_radii'>array</a> <a href='#SkRRect_setRectRadii_radii'>for</a> <a href='#SkRRect_setRectRadii_radii'>individual</a> <a href='#SkRRect_setRectRadii_radii'>control</a> <a href='#SkRRect_setRectRadii_radii'>of</a> <a href='#SkRRect_setRectRadii_radii'>all</a> <a href='#SkRRect_setRectRadii_radii'>for</a> <a href='#SkRRect_setRectRadii_radii'>corners</a>.

If <a href='#SkRRect_setRectRadii_rect'>rect</a> <a href='#SkRRect_setRectRadii_rect'>is</a> <a href='#SkRRect_setRectRadii_rect'>empty</a>, <a href='#SkRRect_setRectRadii_rect'>sets</a> <a href='#SkRRect_setRectRadii_rect'>to</a> <a href='#SkRRect_kEmpty_Type'>kEmpty_Type</a>.
Otherwise, if one of each corner <a href='#SkRRect_setRectRadii_radii'>radii</a> <a href='#SkRRect_setRectRadii_radii'>are</a> <a href='#SkRRect_setRectRadii_radii'>zero</a>, <a href='#SkRRect_setRectRadii_radii'>sets</a> <a href='#SkRRect_setRectRadii_radii'>to</a> <a href='#SkRRect_kRect_Type'>kRect_Type</a>.
Otherwise, if all x-axis <a href='#SkRRect_setRectRadii_radii'>radii</a> <a href='#SkRRect_setRectRadii_radii'>are</a> <a href='#SkRRect_setRectRadii_radii'>equal</a> <a href='#SkRRect_setRectRadii_radii'>and</a> <a href='#SkRRect_setRectRadii_radii'>at</a> <a href='#SkRRect_setRectRadii_radii'>least</a> <a href='#SkRRect_setRectRadii_radii'>half</a> <a href='#SkRRect_setRectRadii_rect'>rect</a>.<a href='#SkRect_width'>width()</a>, <a href='#SkRect_width'>and</a>
all y-axis <a href='#SkRRect_setRectRadii_radii'>radii</a> <a href='#SkRRect_setRectRadii_radii'>are</a> <a href='#SkRRect_setRectRadii_radii'>equal</a> <a href='#SkRRect_setRectRadii_radii'>at</a> <a href='#SkRRect_setRectRadii_radii'>least</a> <a href='#SkRRect_setRectRadii_radii'>half</a> <a href='#SkRRect_setRectRadii_rect'>rect</a>.<a href='#SkRect_height'>height()</a>, <a href='#SkRect_height'>sets</a> <a href='#SkRect_height'>to</a> <a href='#SkRRect_kOval_Type'>kOval_Type</a>.
Otherwise, if all x-axis <a href='#SkRRect_setRectRadii_radii'>radii</a> <a href='#SkRRect_setRectRadii_radii'>are</a> <a href='#SkRRect_setRectRadii_radii'>equal</a>, <a href='#SkRRect_setRectRadii_radii'>and</a> <a href='#SkRRect_setRectRadii_radii'>all</a> <a href='#SkRRect_setRectRadii_radii'>y-axis</a> <a href='#SkRRect_setRectRadii_radii'>radii</a> <a href='#SkRRect_setRectRadii_radii'>are</a> <a href='#SkRRect_setRectRadii_radii'>equal</a>,
sets to <a href='#SkRRect_kSimple_Type'>kSimple_Type</a>. <a href='#SkRRect_kSimple_Type'>Otherwise</a>, <a href='#SkRRect_kSimple_Type'>sets</a> <a href='#SkRRect_kSimple_Type'>to</a> <a href='#SkRRect_kNinePatch_Type'>kNinePatch_Type</a>.

### Parameters

<table>  <tr>    <td><a name='SkRRect_setRectRadii_rect'><code><strong>rect</strong></code></a></td>
    <td>bounds of rounded rectangle</td>
  </tr>
  <tr>    <td><a name='SkRRect_setRectRadii_radii'><code><strong>radii</strong></code></a></td>
    <td>corner x-axis and y-axis <a href='#SkRRect_setRectRadii_radii'>radii</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="340d6c51efaa1f7f3d0dcaf8b0e90696"></fiddle-embed></div>

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

<div><fiddle-embed name="9205393f30b156e1507e88aa27f1dd91"></fiddle-embed></div>

### See Also

<a href='#SkRRect_radii'>radii</a>

<a name='SkRRect_rect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkRRect_rect'>rect()</a> <a href='#SkRRect_rect'>const</a>
</pre>

Returns bounds. Bounds may have zero width or zero height. Bounds right is
greater than or equal to left; bounds bottom is greater than or equal to top.
Result is identical to <a href='#SkRRect_getBounds'>getBounds</a>().

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

<a name='SkRRect_radii'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='SkPoint_Reference#SkVector'>radii</a>(<a href='#SkRRect_Corner'>Corner</a> <a href='#SkRRect_Corner'>corner</a>) <a href='#SkRRect_Corner'>const</a>
</pre>

Returns <a href='undocumented#Scalar'>scalar</a> <a href='undocumented#Scalar'>pair</a> <a href='undocumented#Scalar'>for</a> <a href='undocumented#Scalar'>radius</a> <a href='undocumented#Scalar'>of</a> <a href='undocumented#Curve'>curve</a> <a href='undocumented#Curve'>on</a> <a href='undocumented#Curve'>x-axis</a> <a href='undocumented#Curve'>and</a> <a href='undocumented#Curve'>y-axis</a> <a href='undocumented#Curve'>for</a> <a href='undocumented#Curve'>one</a> <a href='#SkRRect_radii_corner'>corner</a>.
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

<div><fiddle-embed name="8d5c88478528584913867ada423e0d59"><div>Finite values are scaled proportionately to fit; other values are set to zero.
Scaled values cannot be larger than 25, half the bounding <a href='#RRect'>Round_Rect</a> <a href='#RRect'>width</a>.
<a href='#RRect'>Small</a> <a href='#RRect'>scaled</a> <a href='#RRect'>values</a> <a href='#RRect'>are</a> <a href='#RRect'>halved</a> <a href='#RRect'>to</a> <a href='#RRect'>scale</a> <a href='#RRect'>in</a> <a href='#RRect'>proportion</a> <a href='#RRect'>to</a> <a href='#RRect'>the</a> <a href='#RRect'>y-axis</a> <a href='#SkRRect_radii_corner'>corner</a>
<a href='#SkRRect_radii_corner'>radius</a>, <a href='#SkRRect_radii_corner'>which</a> <a href='#SkRRect_radii_corner'>is</a> <a href='#SkRRect_radii_corner'>twice</a> <a href='#SkRRect_radii_corner'>the</a> <a href='#SkRRect_radii_corner'>bounds</a> <a href='#SkRRect_radii_corner'>height</a>.
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
const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkRRect_getBounds'>getBounds</a>() <a href='#SkRRect_getBounds'>const</a>
</pre>

Returns bounds. Bounds may have zero width or zero height. Bounds right is
greater than or equal to left; bounds bottom is greater than or equal to top.
Result is identical to <a href='#SkRRect_rect'>rect()</a>.

### Return Value

bounding box

### Example

<div><fiddle-embed name="4577e2dcb086b241bb43d8b89ee0b0dd"></fiddle-embed></div>

### See Also

<a href='#SkRRect_rect'>rect</a>

<a name='SkRRect_equal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator==(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>a</a>, <a href='SkRRect_Reference#SkRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>b</a>)
</pre>

Returns true if bounds and radii in <a href='#SkRRect_equal_operator_a'>a</a> <a href='#SkRRect_equal_operator_a'>are</a> <a href='#SkRRect_equal_operator_a'>equal</a> <a href='#SkRRect_equal_operator_a'>to</a> <a href='#SkRRect_equal_operator_a'>bounds</a> <a href='#SkRRect_equal_operator_a'>and</a> <a href='#SkRRect_equal_operator_a'>radii</a> <a href='#SkRRect_equal_operator_a'>in</a> <a href='#SkRRect_equal_operator_b'>b</a>.

<a href='#SkRRect_equal_operator_a'>a</a> <a href='#SkRRect_equal_operator_a'>and</a> <a href='#SkRRect_equal_operator_b'>b</a> <a href='#SkRRect_equal_operator_b'>are</a> <a href='#SkRRect_equal_operator_b'>not</a> <a href='#SkRRect_equal_operator_b'>equal</a> <a href='#SkRRect_equal_operator_b'>if</a> <a href='#SkRRect_equal_operator_b'>either</a> <a href='#SkRRect_equal_operator_b'>contain</a> <a href='#SkRRect_equal_operator_b'>NaN</a>. <a href='#SkRRect_equal_operator_a'>a</a> <a href='#SkRRect_equal_operator_a'>and</a> <a href='#SkRRect_equal_operator_b'>b</a> <a href='#SkRRect_equal_operator_b'>are</a> <a href='#SkRRect_equal_operator_b'>equal</a> <a href='#SkRRect_equal_operator_b'>if</a> <a href='#SkRRect_equal_operator_b'>members</a>
contain zeroes with different signs.

### Parameters

<table>  <tr>    <td><a name='SkRRect_equal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>bounds</a> <a href='SkRect_Reference#SkRect'>and</a> <a href='SkRect_Reference#SkRect'>radii</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>compare</a></td>
  </tr>
  <tr>    <td><a name='SkRRect_equal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>bounds</a> <a href='SkRect_Reference#SkRect'>and</a> <a href='SkRect_Reference#SkRect'>radii</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>compare</a></td>
  </tr>
</table>

### Return Value

true if members are equal

### Example

<div><fiddle-embed name="df181af37f1d2b06f0f45af73df7b47d"></fiddle-embed></div>

### See Also

<a href='#SkRRect_notequal_operator'>operator!=(const SkRRect& a, const SkRRect& b)</a>

<a name='SkRRect_notequal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator!=(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>a</a>, <a href='SkRRect_Reference#SkRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>b</a>)
</pre>

Returns true if bounds and radii in <a href='#SkRRect_notequal_operator_a'>a</a> <a href='#SkRRect_notequal_operator_a'>are</a> <a href='#SkRRect_notequal_operator_a'>not</a> <a href='#SkRRect_notequal_operator_a'>equal</a> <a href='#SkRRect_notequal_operator_a'>to</a> <a href='#SkRRect_notequal_operator_a'>bounds</a> <a href='#SkRRect_notequal_operator_a'>and</a> <a href='#SkRRect_notequal_operator_a'>radii</a> <a href='#SkRRect_notequal_operator_a'>in</a> <a href='#SkRRect_notequal_operator_b'>b</a>.

<a href='#SkRRect_notequal_operator_a'>a</a> <a href='#SkRRect_notequal_operator_a'>and</a> <a href='#SkRRect_notequal_operator_b'>b</a> <a href='#SkRRect_notequal_operator_b'>are</a> <a href='#SkRRect_notequal_operator_b'>not</a> <a href='#SkRRect_notequal_operator_b'>equal</a> <a href='#SkRRect_notequal_operator_b'>if</a> <a href='#SkRRect_notequal_operator_b'>either</a> <a href='#SkRRect_notequal_operator_b'>contain</a> <a href='#SkRRect_notequal_operator_b'>NaN</a>. <a href='#SkRRect_notequal_operator_a'>a</a> <a href='#SkRRect_notequal_operator_a'>and</a> <a href='#SkRRect_notequal_operator_b'>b</a> <a href='#SkRRect_notequal_operator_b'>are</a> <a href='#SkRRect_notequal_operator_b'>equal</a> <a href='#SkRRect_notequal_operator_b'>if</a> <a href='#SkRRect_notequal_operator_b'>members</a>
contain zeroes with different signs.

### Parameters

<table>  <tr>    <td><a name='SkRRect_notequal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>bounds</a> <a href='SkRect_Reference#SkRect'>and</a> <a href='SkRect_Reference#SkRect'>radii</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>compare</a></td>
  </tr>
  <tr>    <td><a name='SkRRect_notequal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>bounds</a> <a href='SkRect_Reference#SkRect'>and</a> <a href='SkRect_Reference#SkRect'>radii</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>compare</a></td>
  </tr>
</table>

### Return Value

true if members are not equal

### Example

<div><fiddle-embed name="505e47b3e6474ebdecdc04c3c2af2c34"></fiddle-embed></div>

### See Also

<a href='#SkRRect_equal_operator'>operator==(const SkRRect& a, const SkRRect& b)</a>

<a name='SkRRect_inset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void inset(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>, <a href='SkRRect_Reference#SkRRect'>SkRRect</a>* <a href='SkRRect_Reference#SkRRect'>dst</a>) <a href='SkRRect_Reference#SkRRect'>const</a>
</pre>

Copies <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>to</a> <a href='#SkRRect_inset_dst'>dst</a>, <a href='#SkRRect_inset_dst'>then</a> <a href='#SkRRect_inset_dst'>insets</a> <a href='#SkRRect_inset_dst'>dst</a> <a href='#SkRRect_inset_dst'>bounds</a> <a href='#SkRRect_inset_dst'>by</a> <a href='#SkRRect_inset_dx'>dx</a> <a href='#SkRRect_inset_dx'>and</a> <a href='#SkRRect_inset_dy'>dy</a>, <a href='#SkRRect_inset_dy'>and</a> <a href='#SkRRect_inset_dy'>adjusts</a> <a href='#SkRRect_inset_dst'>dst</a>
radii by <a href='#SkRRect_inset_dx'>dx</a> <a href='#SkRRect_inset_dx'>and</a> <a href='#SkRRect_inset_dy'>dy</a>. <a href='#SkRRect_inset_dx'>dx</a> <a href='#SkRRect_inset_dx'>and</a> <a href='#SkRRect_inset_dy'>dy</a> <a href='#SkRRect_inset_dy'>may</a> <a href='#SkRRect_inset_dy'>be</a> <a href='#SkRRect_inset_dy'>positive</a>, <a href='#SkRRect_inset_dy'>negative</a>, <a href='#SkRRect_inset_dy'>or</a> <a href='#SkRRect_inset_dy'>zero</a>. <a href='#SkRRect_inset_dst'>dst</a> <a href='#SkRRect_inset_dst'>may</a> <a href='#SkRRect_inset_dst'>be</a>
<a href='SkRRect_Reference#SkRRect'>SkRRect</a>.

If either corner radius is zero, the corner has no curvature and is unchanged.
Otherwise, if adjusted radius becomes negative, pins radius to zero.
If <a href='#SkRRect_inset_dx'>dx</a> <a href='#SkRRect_inset_dx'>exceeds</a> <a href='#SkRRect_inset_dx'>half</a> <a href='#SkRRect_inset_dst'>dst</a> <a href='#SkRRect_inset_dst'>bounds</a> <a href='#SkRRect_inset_dst'>width</a>, <a href='#SkRRect_inset_dst'>dst</a> <a href='#SkRRect_inset_dst'>bounds</a> <a href='#SkRRect_inset_dst'>left</a> <a href='#SkRRect_inset_dst'>and</a> <a href='#SkRRect_inset_dst'>right</a> <a href='#SkRRect_inset_dst'>are</a> <a href='#SkRRect_inset_dst'>set</a> <a href='#SkRRect_inset_dst'>to</a>
bounds x-axis center. If <a href='#SkRRect_inset_dy'>dy</a> <a href='#SkRRect_inset_dy'>exceeds</a> <a href='#SkRRect_inset_dy'>half</a> <a href='#SkRRect_inset_dst'>dst</a> <a href='#SkRRect_inset_dst'>bounds</a> <a href='#SkRRect_inset_dst'>height</a>, <a href='#SkRRect_inset_dst'>dst</a> <a href='#SkRRect_inset_dst'>bounds</a> <a href='#SkRRect_inset_dst'>top</a> <a href='#SkRRect_inset_dst'>and</a>
bottom are set to bounds y-axis center.

If <a href='#SkRRect_inset_dx'>dx</a> <a href='#SkRRect_inset_dx'>or</a> <a href='#SkRRect_inset_dy'>dy</a> <a href='#SkRRect_inset_dy'>cause</a> <a href='#SkRRect_inset_dy'>the</a> <a href='#SkRRect_inset_dy'>bounds</a> <a href='#SkRRect_inset_dy'>to</a> <a href='#SkRRect_inset_dy'>become</a> <a href='#SkRRect_inset_dy'>infinite</a>, <a href='#SkRRect_inset_dst'>dst</a> <a href='#SkRRect_inset_dst'>bounds</a> <a href='#SkRRect_inset_dst'>is</a> <a href='#SkRRect_inset_dst'>zeroed</a>.

### Parameters

<table>  <tr>    <td><a name='SkRRect_inset_dx'><code><strong>dx</strong></code></a></td>
    <td>added to <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fLeft'>fLeft</a>, <a href='#SkRect_fLeft'>and</a> <a href='#SkRect_fLeft'>subtracted</a> <a href='#SkRect_fLeft'>from</a> <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRRect_inset_dy'><code><strong>dy</strong></code></a></td>
    <td>added to <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fTop'>fTop</a>, <a href='#SkRect_fTop'>and</a> <a href='#SkRect_fTop'>subtracted</a> <a href='#SkRect_fTop'>from</a> <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
  <tr>    <td><a name='SkRRect_inset_dst'><code><strong>dst</strong></code></a></td>
    <td>insets bounds and radii</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f02f0110d5605dac6d14dcb8d1d8cb6e"></fiddle-embed></div>

### See Also

<a href='#SkRRect_outset'>outset</a> <a href='#SkRRect_offset'>offset</a> <a href='#SkRRect_makeOffset'>makeOffset</a>

<a name='SkRRect_inset_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void inset(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>)
</pre>

Insets bounds by <a href='#SkRRect_inset_2_dx'>dx</a> <a href='#SkRRect_inset_2_dx'>and</a> <a href='#SkRRect_inset_2_dy'>dy</a>, <a href='#SkRRect_inset_2_dy'>and</a> <a href='#SkRRect_inset_2_dy'>adjusts</a> <a href='#SkRRect_inset_2_dy'>radii</a> <a href='#SkRRect_inset_2_dy'>by</a> <a href='#SkRRect_inset_2_dx'>dx</a> <a href='#SkRRect_inset_2_dx'>and</a> <a href='#SkRRect_inset_2_dy'>dy</a>. <a href='#SkRRect_inset_2_dx'>dx</a> <a href='#SkRRect_inset_2_dx'>and</a> <a href='#SkRRect_inset_2_dy'>dy</a> <a href='#SkRRect_inset_2_dy'>may</a> <a href='#SkRRect_inset_2_dy'>be</a>
positive, negative, or zero.

If either corner radius is zero, the corner has no curvature and is unchanged.
Otherwise, if adjusted radius becomes negative, pins radius to zero.
If <a href='#SkRRect_inset_2_dx'>dx</a> <a href='#SkRRect_inset_2_dx'>exceeds</a> <a href='#SkRRect_inset_2_dx'>half</a> <a href='#SkRRect_inset_2_dx'>bounds</a> <a href='#SkRRect_inset_2_dx'>width</a>, <a href='#SkRRect_inset_2_dx'>bounds</a> <a href='#SkRRect_inset_2_dx'>left</a> <a href='#SkRRect_inset_2_dx'>and</a> <a href='#SkRRect_inset_2_dx'>right</a> <a href='#SkRRect_inset_2_dx'>are</a> <a href='#SkRRect_inset_2_dx'>set</a> <a href='#SkRRect_inset_2_dx'>to</a>
bounds x-axis center. If <a href='#SkRRect_inset_2_dy'>dy</a> <a href='#SkRRect_inset_2_dy'>exceeds</a> <a href='#SkRRect_inset_2_dy'>half</a> <a href='#SkRRect_inset_2_dy'>bounds</a> <a href='#SkRRect_inset_2_dy'>height</a>, <a href='#SkRRect_inset_2_dy'>bounds</a> <a href='#SkRRect_inset_2_dy'>top</a> <a href='#SkRRect_inset_2_dy'>and</a>
bottom are set to bounds y-axis center.

If <a href='#SkRRect_inset_2_dx'>dx</a> <a href='#SkRRect_inset_2_dx'>or</a> <a href='#SkRRect_inset_2_dy'>dy</a> <a href='#SkRRect_inset_2_dy'>cause</a> <a href='#SkRRect_inset_2_dy'>the</a> <a href='#SkRRect_inset_2_dy'>bounds</a> <a href='#SkRRect_inset_2_dy'>to</a> <a href='#SkRRect_inset_2_dy'>become</a> <a href='#SkRRect_inset_2_dy'>infinite</a>, <a href='#SkRRect_inset_2_dy'>bounds</a> <a href='#SkRRect_inset_2_dy'>is</a> <a href='#SkRRect_inset_2_dy'>zeroed</a>.

### Parameters

<table>  <tr>    <td><a name='SkRRect_inset_2_dx'><code><strong>dx</strong></code></a></td>
    <td>added to <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fLeft'>fLeft</a>, <a href='#SkRect_fLeft'>and</a> <a href='#SkRect_fLeft'>subtracted</a> <a href='#SkRect_fLeft'>from</a> <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRRect_inset_2_dy'><code><strong>dy</strong></code></a></td>
    <td>added to <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fTop'>fTop</a>, <a href='#SkRect_fTop'>and</a> <a href='#SkRect_fTop'>subtracted</a> <a href='#SkRect_fTop'>from</a> <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="da61054322550a2d5ac15114da23bd23"></fiddle-embed></div>

### See Also

<a href='#SkRRect_outset'>outset</a> <a href='#SkRRect_offset'>offset</a> <a href='#SkRRect_makeOffset'>makeOffset</a>

<a name='SkRRect_outset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void outset(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>, <a href='SkRRect_Reference#SkRRect'>SkRRect</a>* <a href='SkRRect_Reference#SkRRect'>dst</a>) <a href='SkRRect_Reference#SkRRect'>const</a>
</pre>

Outsets <a href='#SkRRect_outset_dst'>dst</a> <a href='#SkRRect_outset_dst'>bounds</a> <a href='#SkRRect_outset_dst'>by</a> <a href='#SkRRect_outset_dx'>dx</a> <a href='#SkRRect_outset_dx'>and</a> <a href='#SkRRect_outset_dy'>dy</a>, <a href='#SkRRect_outset_dy'>and</a> <a href='#SkRRect_outset_dy'>adjusts</a> <a href='#SkRRect_outset_dy'>radii</a> <a href='#SkRRect_outset_dy'>by</a> <a href='#SkRRect_outset_dx'>dx</a> <a href='#SkRRect_outset_dx'>and</a> <a href='#SkRRect_outset_dy'>dy</a>. <a href='#SkRRect_outset_dx'>dx</a> <a href='#SkRRect_outset_dx'>and</a> <a href='#SkRRect_outset_dy'>dy</a> <a href='#SkRRect_outset_dy'>may</a> <a href='#SkRRect_outset_dy'>be</a>
positive, negative, or zero.

If either corner radius is zero, the corner has no curvature and is unchanged.
Otherwise, if adjusted radius becomes negative, pins radius to zero.
If <a href='#SkRRect_outset_dx'>dx</a> <a href='#SkRRect_outset_dx'>exceeds</a> <a href='#SkRRect_outset_dx'>half</a> <a href='#SkRRect_outset_dst'>dst</a> <a href='#SkRRect_outset_dst'>bounds</a> <a href='#SkRRect_outset_dst'>width</a>, <a href='#SkRRect_outset_dst'>dst</a> <a href='#SkRRect_outset_dst'>bounds</a> <a href='#SkRRect_outset_dst'>left</a> <a href='#SkRRect_outset_dst'>and</a> <a href='#SkRRect_outset_dst'>right</a> <a href='#SkRRect_outset_dst'>are</a> <a href='#SkRRect_outset_dst'>set</a> <a href='#SkRRect_outset_dst'>to</a>
bounds x-axis center. If <a href='#SkRRect_outset_dy'>dy</a> <a href='#SkRRect_outset_dy'>exceeds</a> <a href='#SkRRect_outset_dy'>half</a> <a href='#SkRRect_outset_dst'>dst</a> <a href='#SkRRect_outset_dst'>bounds</a> <a href='#SkRRect_outset_dst'>height</a>, <a href='#SkRRect_outset_dst'>dst</a> <a href='#SkRRect_outset_dst'>bounds</a> <a href='#SkRRect_outset_dst'>top</a> <a href='#SkRRect_outset_dst'>and</a>
bottom are set to bounds y-axis center.

If <a href='#SkRRect_outset_dx'>dx</a> <a href='#SkRRect_outset_dx'>or</a> <a href='#SkRRect_outset_dy'>dy</a> <a href='#SkRRect_outset_dy'>cause</a> <a href='#SkRRect_outset_dy'>the</a> <a href='#SkRRect_outset_dy'>bounds</a> <a href='#SkRRect_outset_dy'>to</a> <a href='#SkRRect_outset_dy'>become</a> <a href='#SkRRect_outset_dy'>infinite</a>, <a href='#SkRRect_outset_dst'>dst</a> <a href='#SkRRect_outset_dst'>bounds</a> <a href='#SkRRect_outset_dst'>is</a> <a href='#SkRRect_outset_dst'>zeroed</a>.

### Parameters

<table>  <tr>    <td><a name='SkRRect_outset_dx'><code><strong>dx</strong></code></a></td>
    <td>subtracted from <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fLeft'>fLeft</a>, <a href='#SkRect_fLeft'>and</a> <a href='#SkRect_fLeft'>added</a> <a href='#SkRect_fLeft'>to</a> <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRRect_outset_dy'><code><strong>dy</strong></code></a></td>
    <td>subtracted from <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fTop'>fTop</a>, <a href='#SkRect_fTop'>and</a> <a href='#SkRect_fTop'>added</a> <a href='#SkRect_fTop'>to</a> <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
  <tr>    <td><a name='SkRRect_outset_dst'><code><strong>dst</strong></code></a></td>
    <td>outset bounds and radii</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4d69b6d9c7726c47c42827d79fc7899c"></fiddle-embed></div>

### See Also

<a href='#SkRRect_inset'>inset</a> <a href='#SkRRect_offset'>offset</a> <a href='#SkRRect_makeOffset'>makeOffset</a>

<a name='SkRRect_outset_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void outset(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>)
</pre>

Outsets bounds by <a href='#SkRRect_outset_2_dx'>dx</a> <a href='#SkRRect_outset_2_dx'>and</a> <a href='#SkRRect_outset_2_dy'>dy</a>, <a href='#SkRRect_outset_2_dy'>and</a> <a href='#SkRRect_outset_2_dy'>adjusts</a> <a href='#SkRRect_outset_2_dy'>radii</a> <a href='#SkRRect_outset_2_dy'>by</a> <a href='#SkRRect_outset_2_dx'>dx</a> <a href='#SkRRect_outset_2_dx'>and</a> <a href='#SkRRect_outset_2_dy'>dy</a>. <a href='#SkRRect_outset_2_dx'>dx</a> <a href='#SkRRect_outset_2_dx'>and</a> <a href='#SkRRect_outset_2_dy'>dy</a> <a href='#SkRRect_outset_2_dy'>may</a> <a href='#SkRRect_outset_2_dy'>be</a>
positive, negative, or zero.

If either corner radius is zero, the corner has no curvature and is unchanged.
Otherwise, if adjusted radius becomes negative, pins radius to zero.
If <a href='#SkRRect_outset_2_dx'>dx</a> <a href='#SkRRect_outset_2_dx'>exceeds</a> <a href='#SkRRect_outset_2_dx'>half</a> <a href='#SkRRect_outset_2_dx'>bounds</a> <a href='#SkRRect_outset_2_dx'>width</a>, <a href='#SkRRect_outset_2_dx'>bounds</a> <a href='#SkRRect_outset_2_dx'>left</a> <a href='#SkRRect_outset_2_dx'>and</a> <a href='#SkRRect_outset_2_dx'>right</a> <a href='#SkRRect_outset_2_dx'>are</a> <a href='#SkRRect_outset_2_dx'>set</a> <a href='#SkRRect_outset_2_dx'>to</a>
bounds x-axis center. If <a href='#SkRRect_outset_2_dy'>dy</a> <a href='#SkRRect_outset_2_dy'>exceeds</a> <a href='#SkRRect_outset_2_dy'>half</a> <a href='#SkRRect_outset_2_dy'>bounds</a> <a href='#SkRRect_outset_2_dy'>height</a>, <a href='#SkRRect_outset_2_dy'>bounds</a> <a href='#SkRRect_outset_2_dy'>top</a> <a href='#SkRRect_outset_2_dy'>and</a>
bottom are set to bounds y-axis center.

If <a href='#SkRRect_outset_2_dx'>dx</a> <a href='#SkRRect_outset_2_dx'>or</a> <a href='#SkRRect_outset_2_dy'>dy</a> <a href='#SkRRect_outset_2_dy'>cause</a> <a href='#SkRRect_outset_2_dy'>the</a> <a href='#SkRRect_outset_2_dy'>bounds</a> <a href='#SkRRect_outset_2_dy'>to</a> <a href='#SkRRect_outset_2_dy'>become</a> <a href='#SkRRect_outset_2_dy'>infinite</a>, <a href='#SkRRect_outset_2_dy'>bounds</a> <a href='#SkRRect_outset_2_dy'>is</a> <a href='#SkRRect_outset_2_dy'>zeroed</a>.

### Parameters

<table>  <tr>    <td><a name='SkRRect_outset_2_dx'><code><strong>dx</strong></code></a></td>
    <td>subtracted from <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fLeft'>fLeft</a>, <a href='#SkRect_fLeft'>and</a> <a href='#SkRect_fLeft'>added</a> <a href='#SkRect_fLeft'>to</a> <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRRect_outset_2_dy'><code><strong>dy</strong></code></a></td>
    <td>subtracted from <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fTop'>fTop</a>, <a href='#SkRect_fTop'>and</a> <a href='#SkRect_fTop'>added</a> <a href='#SkRect_fTop'>to</a> <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4391cced86653dcd0f84439a5c0bb3f2"></fiddle-embed></div>

### See Also

<a href='#SkRRect_inset'>inset</a> <a href='#SkRRect_offset'>offset</a> <a href='#SkRRect_makeOffset'>makeOffset</a>

<a name='SkRRect_offset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void offset(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>)
</pre>

Translates <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>by</a> (<a href='#SkRRect_offset_dx'>dx</a>, <a href='#SkRRect_offset_dy'>dy</a>).

### Parameters

<table>  <tr>    <td><a name='SkRRect_offset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>and</a> <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRRect_offset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>and</a> <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a45cdd46ef2fe0df62d84d41713e82e2"></fiddle-embed></div>

### See Also

<a href='#SkRRect_makeOffset'>makeOffset</a>  <a href='#SkRRect_inset'>inset outset</a>

<a name='SkRRect_makeOffset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='#SkRRect_makeOffset'>makeOffset</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>) <a href='undocumented#SkScalar'>const</a>
</pre>

Returns <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>translated</a> <a href='SkRRect_Reference#SkRRect'>by</a> (<a href='#SkRRect_makeOffset_dx'>dx</a>, <a href='#SkRRect_makeOffset_dy'>dy</a>).

### Parameters

<table>  <tr>    <td><a name='SkRRect_makeOffset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>and</a> <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRRect_makeOffset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>and</a> <a href='#SkRRect_rect'>rect()</a>.<a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

<a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>bounds</a> <a href='SkRRect_Reference#SkRRect'>offset</a> <a href='SkRRect_Reference#SkRRect'>by</a> (<a href='#SkRRect_makeOffset_dx'>dx</a>, <a href='#SkRRect_makeOffset_dy'>dy</a>), <a href='#SkRRect_makeOffset_dy'>with</a> <a href='#SkRRect_makeOffset_dy'>unchanged</a> <a href='#SkRRect_makeOffset_dy'>corner</a> <a href='#SkRRect_makeOffset_dy'>radii</a>

### Example

<div><fiddle-embed name="c433aa41eaf5e419e3349fb970a08151"></fiddle-embed></div>

### See Also

<a href='#SkRRect_offset'>offset</a>  <a href='#SkRRect_inset'>inset outset</a>

<a name='SkRRect_contains'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool contains(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>) <a href='SkRect_Reference#Rect'>const</a>
</pre>

Returns true if <a href='#SkRRect_contains_rect'>rect</a> <a href='#SkRRect_contains_rect'>is</a> <a href='#SkRRect_contains_rect'>inside</a> <a href='#SkRRect_contains_rect'>the</a> <a href='#SkRRect_contains_rect'>bounds</a> <a href='#SkRRect_contains_rect'>and</a> <a href='#SkRRect_contains_rect'>corner</a> <a href='#SkRRect_contains_rect'>radii</a>, <a href='#SkRRect_contains_rect'>and</a> <a href='#SkRRect_contains_rect'>if</a>
<a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>and</a> <a href='#SkRRect_contains_rect'>rect</a> <a href='#SkRRect_contains_rect'>are</a> <a href='#SkRRect_contains_rect'>not</a> <a href='#SkRRect_contains_rect'>empty</a>.

### Parameters

<table>  <tr>    <td><a name='SkRRect_contains_rect'><code><strong>rect</strong></code></a></td>
    <td>area tested for containment</td>
  </tr>
</table>

### Return Value

true if <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>contains</a> <a href='#SkRRect_contains_rect'>rect</a>

### Example

<div><fiddle-embed name="0bb057140e4119234bdd2e8dd2f0fa19"></fiddle-embed></div>

### See Also

<a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_contains'>contains</a>

<a name='SkRRect_isValid'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRRect_isValid'>isValid</a>() <a href='#SkRRect_isValid'>const</a>
</pre>

Returns true if bounds and radii values are finite and describe a <a href='SkRRect_Reference#SkRRect'>SkRRect</a>
<a href='SkRRect_Reference#SkRRect'>SkRRect</a>::<a href='#SkRRect_Type'>Type</a> <a href='#SkRRect_Type'>that</a> <a href='#SkRRect_Type'>matches</a> <a href='#SkRRect_getType'>getType</a>(). <a href='#SkRRect_getType'>All</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>methods</a> <a href='SkRRect_Reference#SkRRect'>construct</a> <a href='SkRRect_Reference#SkRRect'>valid</a> <a href='SkRRect_Reference#SkRRect'>types</a>,
even if the input values are not valid. Invalid <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>can</a> <a href='undocumented#Data'>only</a>
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
Space required to write the contents of <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>into</a> <a href='SkRRect_Reference#SkRRect'>a</a> <a href='SkRRect_Reference#SkRRect'>buffer</a>; <a href='SkRRect_Reference#SkRRect'>always</a> <a href='SkRRect_Reference#SkRRect'>a</a> <a href='SkRRect_Reference#SkRRect'>multiple</a> <a href='SkRRect_Reference#SkRRect'>of</a> <a href='SkRRect_Reference#SkRRect'>four</a>.
</td>
  </tr>
</table>

<a name='SkRRect_writeToMemory'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkRRect_writeToMemory'>writeToMemory</a>(<a href='#SkRRect_writeToMemory'>void</a>* <a href='#SkRRect_writeToMemory'>buffer</a>) <a href='#SkRRect_writeToMemory'>const</a>
</pre>

Writes <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>to</a> <a href='#SkRRect_writeToMemory_buffer'>buffer</a>. <a href='#SkRRect_writeToMemory_buffer'>Writes</a> <a href='#SkRRect_kSizeInMemory'>kSizeInMemory</a> <a href='#SkRRect_kSizeInMemory'>bytes</a>, <a href='#SkRRect_kSizeInMemory'>and</a> <a href='#SkRRect_kSizeInMemory'>returns</a>
<a href='#SkRRect_kSizeInMemory'>kSizeInMemory</a>, <a href='#SkRRect_kSizeInMemory'>the</a> <a href='#SkRRect_kSizeInMemory'>number</a> <a href='#SkRRect_kSizeInMemory'>of</a> <a href='#SkRRect_kSizeInMemory'>bytes</a> <a href='#SkRRect_kSizeInMemory'>written</a>.

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
size_t <a href='#SkRRect_readFromMemory'>readFromMemory</a>(<a href='#SkRRect_readFromMemory'>const</a> <a href='#SkRRect_readFromMemory'>void</a>* <a href='#SkRRect_readFromMemory'>buffer</a>, <a href='#SkRRect_readFromMemory'>size_t</a> <a href='#SkRRect_readFromMemory'>length</a>)
</pre>

Reads <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>from</a> <a href='#SkRRect_readFromMemory_buffer'>buffer</a>, <a href='#SkRRect_readFromMemory_buffer'>reading</a> <a href='#SkRRect_kSizeInMemory'>kSizeInMemory</a> <a href='#SkRRect_kSizeInMemory'>bytes</a>.
Returns <a href='#SkRRect_kSizeInMemory'>kSizeInMemory</a>, <a href='#SkRRect_kSizeInMemory'>bytes</a> <a href='#SkRRect_kSizeInMemory'>read</a> <a href='#SkRRect_kSizeInMemory'>if</a> <a href='#SkRRect_readFromMemory_length'>length</a> <a href='#SkRRect_readFromMemory_length'>is</a> <a href='#SkRRect_readFromMemory_length'>at</a> <a href='#SkRRect_readFromMemory_length'>least</a> <a href='#SkRRect_kSizeInMemory'>kSizeInMemory</a>.
Otherwise, returns zero.

### Parameters

<table>  <tr>    <td><a name='SkRRect_readFromMemory_buffer'><code><strong>buffer</strong></code></a></td>
    <td>memory to read from</td>
  </tr>
  <tr>    <td><a name='SkRRect_readFromMemory_length'><code><strong>length</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='#SkRRect_readFromMemory_buffer'>buffer</a></td>
  </tr>
</table>

### Return Value

bytes read, or 0 if <a href='#SkRRect_readFromMemory_length'>length</a> <a href='#SkRRect_readFromMemory_length'>is</a> <a href='#SkRRect_readFromMemory_length'>less</a> <a href='#SkRRect_readFromMemory_length'>than</a> <a href='#SkRRect_kSizeInMemory'>kSizeInMemory</a>

### Example

<div><fiddle-embed name="50969745cf2b23544362f4cff5592b75"></fiddle-embed></div>

### See Also

<a href='#SkRRect_writeToMemory'>writeToMemory</a>

<a name='SkRRect_transform'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool transform(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#Matrix'>matrix</a>, <a href='SkRRect_Reference#SkRRect'>SkRRect</a>* <a href='SkRRect_Reference#SkRRect'>dst</a>) <a href='SkRRect_Reference#SkRRect'>const</a>
</pre>

Transforms by <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>by</a> <a href='#SkRRect_transform_matrix'>matrix</a>, <a href='#SkRRect_transform_matrix'>storing</a> <a href='#SkRRect_transform_matrix'>result</a> <a href='#SkRRect_transform_matrix'>in</a> <a href='#SkRRect_transform_dst'>dst</a>.
Returns true if <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>transformed</a> <a href='SkRRect_Reference#SkRRect'>can</a> <a href='SkRRect_Reference#SkRRect'>be</a> <a href='SkRRect_Reference#SkRRect'>represented</a> <a href='SkRRect_Reference#SkRRect'>by</a> <a href='SkRRect_Reference#SkRRect'>another</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>.
Returns false if <a href='#SkRRect_transform_matrix'>matrix</a> <a href='#SkRRect_transform_matrix'>contains</a> <a href='#SkRRect_transform_matrix'>transformations</a> <a href='#SkRRect_transform_matrix'>other</a> <a href='#SkRRect_transform_matrix'>than</a> <a href='#SkRRect_transform_matrix'>scale</a> <a href='#SkRRect_transform_matrix'>and</a> <a href='#SkRRect_transform_matrix'>translate</a>.

Asserts in debug builds if <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>equals</a> <a href='#SkRRect_transform_dst'>dst</a>.

### Parameters

<table>  <tr>    <td><a name='SkRRect_transform_matrix'><code><strong>matrix</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>specifying</a> <a href='SkMatrix_Reference#SkMatrix'>the</a> <a href='SkMatrix_Reference#SkMatrix'>transform</a></td>
  </tr>
  <tr>    <td><a name='SkRRect_transform_dst'><code><strong>dst</strong></code></a></td>
    <td><a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>to</a> <a href='SkRRect_Reference#SkRRect'>store</a> <a href='SkRRect_Reference#SkRRect'>the</a> <a href='SkRRect_Reference#SkRRect'>result</a></td>
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
void <a href='#SkRRect_dump'>dump</a>(<a href='#SkRRect_dump'>bool</a> <a href='#SkRRect_dump'>asHex</a>) <a href='#SkRRect_dump'>const</a>
</pre>

Writes <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>representation</a> <a href='undocumented#Text'>of</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>to</a> <a href='SkRRect_Reference#SkRRect'>standard</a> <a href='SkRRect_Reference#SkRRect'>output</a>.
Set <a href='#SkRRect_dump_asHex'>asHex</a> <a href='#SkRRect_dump_asHex'>true</a> <a href='#SkRRect_dump_asHex'>to</a> <a href='#SkRRect_dump_asHex'>generate</a> <a href='#SkRRect_dump_asHex'>exact</a> <a href='#SkRRect_dump_asHex'>binary</a> <a href='#SkRRect_dump_asHex'>representations</a>
of floating <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>numbers</a>.

### Parameters

<table>  <tr>    <td><a name='SkRRect_dump_asHex'><code><strong>asHex</strong></code></a></td>
    <td>true if <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>values</a> <a href='undocumented#SkScalar'>are</a> <a href='undocumented#SkScalar'>written</a> <a href='undocumented#SkScalar'>as</a> <a href='undocumented#SkScalar'>hexadecimal</a></td>
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

<a href='#SkRRect_dumpHex'>dumpHex</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_dump'>dump</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_dump'>dump</a> <a href='undocumented#SkPathMeasure'>SkPathMeasure</a>::<a href='#SkPathMeasure_dump'>dump</a>

<a name='SkRRect_dump_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_dump'>dump()</a> <a href='#SkRRect_dump'>const</a>
</pre>

Writes <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>representation</a> <a href='undocumented#Text'>of</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>to</a> <a href='SkRRect_Reference#SkRRect'>standard</a> <a href='SkRRect_Reference#SkRRect'>output</a>. <a href='SkRRect_Reference#SkRRect'>The</a> <a href='SkRRect_Reference#SkRRect'>representation</a>
may be directly compiled as C++ code. Floating <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>values</a> <a href='SkPoint_Reference#Point'>are</a> <a href='SkPoint_Reference#Point'>written</a>
with limited precision; it may not be possible to reconstruct original
<a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>from</a> <a href='SkRRect_Reference#SkRRect'>output</a>.

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

<a href='#SkRRect_dumpHex'>dumpHex</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_dump'>dump</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_dump'>dump</a> <a href='undocumented#SkPathMeasure'>SkPathMeasure</a>::<a href='#SkPathMeasure_dump'>dump</a>

<a name='SkRRect_dumpHex'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRRect_dumpHex'>dumpHex</a>() <a href='#SkRRect_dumpHex'>const</a>
</pre>

Writes <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>representation</a> <a href='undocumented#Text'>of</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>to</a> <a href='SkRRect_Reference#SkRRect'>standard</a> <a href='SkRRect_Reference#SkRRect'>output</a>. <a href='SkRRect_Reference#SkRRect'>The</a> <a href='SkRRect_Reference#SkRRect'>representation</a>
may be directly compiled as C++ code. Floating <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>values</a> <a href='SkPoint_Reference#Point'>are</a> <a href='SkPoint_Reference#Point'>written</a>
in hexadecimal to preserve their exact bit pattern. The output reconstructs the
original <a href='SkRRect_Reference#SkRRect'>SkRRect</a>.

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

<a href='#SkRRect_dump'>dump</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_dumpHex'>dumpHex</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_dumpHex'>dumpHex</a>

