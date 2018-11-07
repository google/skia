SkIPoint Reference
===


<a name='SkIPoint'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
struct <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> {
    <a href='SkIPoint_Reference#SkIPoint'>int32_t</a> <a href='#SkIPoint_fX'>fX</a>;
    <a href='#SkIPoint_fX'>int32_t</a> <a href='#SkIPoint_fY'>fY</a>;

    <a href='#SkIPoint_fY'>static</a> <a href='#SkIPoint_fY'>constexpr</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='#SkIPoint_Make'>Make</a>(<a href='#SkIPoint_Make'>int32_t</a> <a href='#SkIPoint_Make'>x</a>, <a href='#SkIPoint_Make'>int32_t</a> <a href='#SkIPoint_Make'>y</a>);
    <a href='#SkIPoint_Make'>int32_t</a> <a href='#SkIPoint_x'>x()</a> <a href='#SkIPoint_x'>const</a>;
    <a href='#SkIPoint_x'>int32_t</a> <a href='#SkIPoint_y'>y()</a> <a href='#SkIPoint_y'>const</a>;
    <a href='#SkIPoint_y'>bool</a> <a href='#SkIPoint_isZero'>isZero</a>() <a href='#SkIPoint_isZero'>const</a>;
    <a href='#SkIPoint_isZero'>void</a> <a href='#SkIPoint_isZero'>set</a>(<a href='#SkIPoint_isZero'>int32_t</a> <a href='#SkIPoint_isZero'>x</a>, <a href='#SkIPoint_isZero'>int32_t</a> <a href='#SkIPoint_isZero'>y</a>);
    <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='SkIPoint_Reference#SkIPoint'>operator</a>-() <a href='SkIPoint_Reference#SkIPoint'>const</a>;
    <a href='SkIPoint_Reference#SkIPoint'>void</a> <a href='SkIPoint_Reference#SkIPoint'>operator</a>+=(<a href='SkIPoint_Reference#SkIPoint'>const</a> <a href='SkIPoint_Reference#SkIVector'>SkIVector</a>& <a href='SkIPoint_Reference#SkIVector'>v</a>);
    <a href='SkIPoint_Reference#SkIVector'>void</a> <a href='SkIPoint_Reference#SkIVector'>operator</a>-=(<a href='SkIPoint_Reference#SkIVector'>const</a> <a href='SkIPoint_Reference#SkIVector'>SkIVector</a>& <a href='SkIPoint_Reference#SkIVector'>v</a>);
    <a href='SkIPoint_Reference#SkIVector'>bool</a> <a href='SkIPoint_Reference#SkIVector'>equals</a>(<a href='SkIPoint_Reference#SkIVector'>int32_t</a> <a href='SkIPoint_Reference#SkIVector'>x</a>, <a href='SkIPoint_Reference#SkIVector'>int32_t</a> <a href='SkIPoint_Reference#SkIVector'>y</a>) <a href='SkIPoint_Reference#SkIVector'>const</a>;
    <a href='SkIPoint_Reference#SkIVector'>friend</a> <a href='SkIPoint_Reference#SkIVector'>bool</a> <a href='SkIPoint_Reference#SkIVector'>operator</a>==(<a href='SkIPoint_Reference#SkIVector'>const</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& <a href='SkIPoint_Reference#SkIPoint'>a</a>, <a href='SkIPoint_Reference#SkIPoint'>const</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& <a href='SkIPoint_Reference#SkIPoint'>b</a>);
    <a href='SkIPoint_Reference#SkIPoint'>friend</a> <a href='SkIPoint_Reference#SkIPoint'>bool</a> <a href='SkIPoint_Reference#SkIPoint'>operator</a>!=(<a href='SkIPoint_Reference#SkIPoint'>const</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& <a href='SkIPoint_Reference#SkIPoint'>a</a>, <a href='SkIPoint_Reference#SkIPoint'>const</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& <a href='SkIPoint_Reference#SkIPoint'>b</a>);
    <a href='SkIPoint_Reference#SkIPoint'>friend</a> <a href='SkIPoint_Reference#SkIVector'>SkIVector</a> <a href='SkIPoint_Reference#SkIVector'>operator</a>-(<a href='SkIPoint_Reference#SkIVector'>const</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& <a href='SkIPoint_Reference#SkIPoint'>a</a>, <a href='SkIPoint_Reference#SkIPoint'>const</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& <a href='SkIPoint_Reference#SkIPoint'>b</a>);
    <a href='SkIPoint_Reference#SkIPoint'>friend</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='SkIPoint_Reference#SkIPoint'>operator</a>+(<a href='SkIPoint_Reference#SkIPoint'>const</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& <a href='SkIPoint_Reference#SkIPoint'>a</a>, <a href='SkIPoint_Reference#SkIPoint'>const</a> <a href='SkIPoint_Reference#SkIVector'>SkIVector</a>& <a href='SkIPoint_Reference#SkIVector'>b</a>);
};
</pre>

<a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='SkIPoint_Reference#SkIPoint'>holds</a> <a href='SkIPoint_Reference#SkIPoint'>two</a> 32-<a href='SkIPoint_Reference#SkIPoint'>bit</a> <a href='SkIPoint_Reference#SkIPoint'>integer</a> <a href='SkIPoint_Reference#SkIPoint'>coordinates</a>.<table style='border-collapse: collapse; width: 62.5em'>

  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Member</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkIPoint_fX'><code>fX</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
x-axis value used by <a href='SkIPoint_Reference#IPoint'>IPoint</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkIPoint_fY'><code>fY</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
y-axis value used by <a href='SkIPoint_Reference#IPoint'>IPoint</a>.
</td>
  </tr>
</table>

<a name='SkIPoint_Make'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='#SkIPoint_Make'>Make</a>(<a href='#SkIPoint_Make'>int32_t</a> <a href='#SkIPoint_Make'>x</a>, <a href='#SkIPoint_Make'>int32_t</a> <a href='#SkIPoint_Make'>y</a>)
</pre>

Sets <a href='#SkIPoint_fX'>fX</a> <a href='#SkIPoint_fX'>to</a> <a href='#SkIPoint_Make_x'>x</a>, <a href='#SkIPoint_fY'>fY</a> <a href='#SkIPoint_fY'>to</a> <a href='#SkIPoint_Make_y'>y</a>.

### Parameters

<table>  <tr>    <td><a name='SkIPoint_Make_x'><code><strong>x</strong></code></a></td>
    <td>integer x-axis value of constructed <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a></td>
  </tr>
  <tr>    <td><a name='SkIPoint_Make_y'><code><strong>y</strong></code></a></td>
    <td>integer y-axis value of constructed <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a></td>
  </tr>
</table>

### Return Value

<a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> (<a href='#SkIPoint_Make_x'>x</a>, <a href='#SkIPoint_Make_y'>y</a>)

### Example

<div><fiddle-embed name="e5cf5159525bd3140f288a95fe641fae">

#### Example Output

~~~~
pt1 == pt2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIPoint_set'>set()</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>::<a href='#SkPoint_iset'>iset()</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>::<a href='#SkPoint_Make'>Make</a>

<a name='Property'></a>

<a name='SkIPoint_x'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIPoint_x'>x()</a> <a href='#SkIPoint_x'>const</a>
</pre>

Returns x-axis value of <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>.

### Return Value

<a href='#SkIPoint_fX'>fX</a>

### Example

<div><fiddle-embed name="eed4185294f8a8216fc354e6ee6b2e3a">

#### Example Output

~~~~
pt1.fX == pt1.x()
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIPoint_y'>y()</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>::<a href='#SkPoint_x'>x()</a>

<a name='SkIPoint_y'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIPoint_y'>y()</a> <a href='#SkIPoint_y'>const</a>
</pre>

Returns y-axis value of <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>.

### Return Value

<a href='#SkIPoint_fY'>fY</a>

### Example

<div><fiddle-embed name="35c41b8ba7cebf8c9a7a8494e610e14d">

#### Example Output

~~~~
pt1.fY == pt1.y()
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIPoint_x'>x()</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>::<a href='#SkPoint_y'>y()</a>

<a name='SkIPoint_isZero'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIPoint_isZero'>isZero</a>() <a href='#SkIPoint_isZero'>const</a>
</pre>

Returns true if <a href='#SkIPoint_fX'>fX</a> <a href='#SkIPoint_fX'>and</a> <a href='#SkIPoint_fY'>fY</a> <a href='#SkIPoint_fY'>are</a> <a href='#SkIPoint_fY'>both</a> <a href='#SkIPoint_fY'>zero</a>.

### Return Value

true if <a href='#SkIPoint_fX'>fX</a> <a href='#SkIPoint_fX'>is</a> <a href='#SkIPoint_fX'>zero</a> <a href='#SkIPoint_fX'>and</a> <a href='#SkIPoint_fY'>fY</a> <a href='#SkIPoint_fY'>is</a> <a href='#SkIPoint_fY'>zero</a>

### Example

<div><fiddle-embed name="658c1df611b4577cc7e0bb384e95737e">

#### Example Output

~~~~
pt.isZero() == true
~~~~

</fiddle-embed></div>

### See Also

<a href='SkPoint_Reference#SkPoint'>SkPoint</a>::<a href='#SkPoint_isZero'>isZero</a>

<a name='Set'></a>

<a name='SkIPoint_set'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void set(int32_t x, int32_t y)
</pre>

Sets <a href='#SkIPoint_fX'>fX</a> <a href='#SkIPoint_fX'>to</a> <a href='#SkIPoint_set_x'>x</a> <a href='#SkIPoint_set_x'>and</a> <a href='#SkIPoint_fY'>fY</a> <a href='#SkIPoint_fY'>to</a> <a href='#SkIPoint_set_y'>y</a>.

### Parameters

<table>  <tr>    <td><a name='SkIPoint_set_x'><code><strong>x</strong></code></a></td>
    <td>new value for <a href='#SkIPoint_fX'>fX</a></td>
  </tr>
  <tr>    <td><a name='SkIPoint_set_y'><code><strong>y</strong></code></a></td>
    <td>new value for <a href='#SkIPoint_fY'>fY</a></td>
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

<a href='#SkIPoint_Make'>Make</a>

<a name='SkIPoint_minus_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='SkIPoint_Reference#SkIPoint'>operator</a>-() <a href='SkIPoint_Reference#SkIPoint'>const</a>
</pre>

Returns <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='SkIPoint_Reference#SkIPoint'>changing</a> <a href='SkIPoint_Reference#SkIPoint'>the</a> <a href='SkIPoint_Reference#SkIPoint'>signs</a> <a href='SkIPoint_Reference#SkIPoint'>of</a> <a href='#SkIPoint_fX'>fX</a> <a href='#SkIPoint_fX'>and</a> <a href='#SkIPoint_fY'>fY</a>.

### Return Value

<a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='SkIPoint_Reference#SkIPoint'>as</a> (-<a href='#SkIPoint_fX'>fX</a>, -<a href='#SkIPoint_fY'>fY</a>)

### Example

<div><fiddle-embed name="b30d4780475d113a7fed3637af7f0db1">

#### Example Output

~~~~
pt: 0, 0  negate: 0, 0
pt: -1, -2  negate: 1, 2
pt: 2147483647, -2147483647  negate: -2147483647, 2147483647
pt: -2147483648, -2147483648  negate: -2147483648, -2147483648
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIPoint_subtract_operator'>operator-(const SkIPoint& a, const SkIPoint& b)</a> operator-(const SkIPoint& a, const SkIPoint& b)<a href='#SkIPoint_subtractfrom_operator'>operator-=(const SkIVector& v)</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>::<a href='SkPoint_Reference#SkPoint'>operator</a>-()_<a href='SkPoint_Reference#SkPoint'>const</a>

<a name='SkIPoint_addto_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void operator+=(const <a href='SkIPoint_Reference#SkIVector'>SkIVector</a>& <a href='SkIPoint_Reference#SkIVector'>v</a>)
</pre>

Offsets <a href='SkIPoint_Reference#IPoint'>IPoint</a> <a href='SkIPoint_Reference#IPoint'>by</a> <a href='#IPoint_IVector'>IVector</a> <a href='#SkIPoint_addto_operator_v'>v</a>. <a href='#SkIPoint_addto_operator_v'>Sets</a> <a href='SkIPoint_Reference#IPoint'>IPoint</a> <a href='SkIPoint_Reference#IPoint'>to</a> <code>(<a href='#SkIPoint_fX'>fX</a> + <a href='#SkIPoint_addto_operator_v'>v</a>.<a href='#SkIPoint_fX'>fX</a>, <a href='#SkIPoint_fY'>fY</a> + <a href='#SkIPoint_addto_operator_v'>v</a>.<a href='#SkIPoint_fY'>fY</a>)</code>.

### Parameters

<table>  <tr>    <td><a name='SkIPoint_addto_operator_v'><code><strong>v</strong></code></a></td>
    <td><a href='#IPoint_IVector'>IVector</a> <a href='#IPoint_IVector'>to</a> <a href='#IPoint_IVector'>add</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4eb2d95c9e9a66f05296e345bb68bd51"></fiddle-embed></div>

### See Also

<a href='#SkIPoint_add_operator'>operator+(const SkIPoint& a, const SkIVector& b)</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>::SkPoint<a href='#SkPoint_addto_operator'>operator+=(const SkVector& v)</a>

<a name='SkIPoint_subtractfrom_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void operator-=(const <a href='SkIPoint_Reference#SkIVector'>SkIVector</a>& <a href='SkIPoint_Reference#SkIVector'>v</a>)
</pre>

Subtracts <a href='#IPoint_IVector'>IVector</a> <a href='#SkIPoint_subtractfrom_operator_v'>v</a> <a href='#SkIPoint_subtractfrom_operator_v'>from</a> <a href='SkIPoint_Reference#IPoint'>IPoint</a>. <a href='SkIPoint_Reference#IPoint'>Sets</a> <a href='SkIPoint_Reference#IPoint'>IPoint</a> <a href='SkIPoint_Reference#IPoint'>to</a>: <code>(<a href='#SkIPoint_fX'>fX</a> - <a href='#SkIPoint_subtractfrom_operator_v'>v</a>.<a href='#SkIPoint_fX'>fX</a>, <a href='#SkIPoint_fY'>fY</a> - <a href='#SkIPoint_subtractfrom_operator_v'>v</a>.<a href='#SkIPoint_fY'>fY</a>)</code>.

### Parameters

<table>  <tr>    <td><a name='SkIPoint_subtractfrom_operator_v'><code><strong>v</strong></code></a></td>
    <td><a href='#IPoint_IVector'>IVector</a> <a href='#IPoint_IVector'>to</a> <a href='#IPoint_IVector'>subtract</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a01e533dc7ab34ed728dc4e7a5f1f0ee"></fiddle-embed></div>

### See Also

<a href='#SkIPoint_subtract_operator'>operator-(const SkIPoint& a, const SkIPoint& b)</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>::SkPoint<a href='#SkPoint_subtractfrom_operator'>operator-=(const SkVector& v)</a>

<a name='SkIPoint_equals'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool equals(int32_t x, int32_t y) const
</pre>

Returns true if <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='SkIPoint_Reference#SkIPoint'>is</a> <a href='SkIPoint_Reference#SkIPoint'>equivalent</a> <a href='SkIPoint_Reference#SkIPoint'>to</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='SkIPoint_Reference#SkIPoint'>constructed</a> <a href='SkIPoint_Reference#SkIPoint'>from</a> (<a href='#SkIPoint_equals_x'>x</a>, <a href='#SkIPoint_equals_y'>y</a>).

### Parameters

<table>  <tr>    <td><a name='SkIPoint_equals_x'><code><strong>x</strong></code></a></td>
    <td>value compared with <a href='#SkIPoint_fX'>fX</a></td>
  </tr>
  <tr>    <td><a name='SkIPoint_equals_y'><code><strong>y</strong></code></a></td>
    <td>value compared with <a href='#SkIPoint_fY'>fY</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='SkIPoint_Reference#SkIPoint'>equals</a> (<a href='#SkIPoint_equals_x'>x</a>, <a href='#SkIPoint_equals_y'>y</a>)

### Example

<div><fiddle-embed name="64f575d36439d5b69aaed14ffeff1cc4">

#### Example Output

~~~~
pt: 0, 0  == pt
pt: -1, -2  == pt
pt: 2147483647, -1  == pt
pt: -2147483648, -1  == pt
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIPoint_equal_operator'>operator==(const SkIPoint& a, const SkIPoint& b)</a>

<a name='SkIPoint_equal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator==(const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& <a href='SkIPoint_Reference#SkIPoint'>a</a>, <a href='SkIPoint_Reference#SkIPoint'>const</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& <a href='SkIPoint_Reference#SkIPoint'>b</a>)
</pre>

Returns true if <a href='#SkIPoint_equal_operator_a'>a</a> <a href='#SkIPoint_equal_operator_a'>is</a> <a href='#SkIPoint_equal_operator_a'>equivalent</a> <a href='#SkIPoint_equal_operator_a'>to</a> <a href='#SkIPoint_equal_operator_b'>b</a>.

### Parameters

<table>  <tr>    <td><a name='SkIPoint_equal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='SkIPoint_Reference#SkIPoint'>to</a> <a href='SkIPoint_Reference#SkIPoint'>compare</a></td>
  </tr>
  <tr>    <td><a name='SkIPoint_equal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='SkIPoint_Reference#SkIPoint'>to</a> <a href='SkIPoint_Reference#SkIPoint'>compare</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkIPoint_equal_operator_a'>a</a>.<a href='#SkIPoint_fX'>fX</a> == <a href='#SkIPoint_equal_operator_b'>b</a>.<a href='#SkIPoint_fX'>fX</a> <a href='#SkIPoint_fX'>and</a> <a href='#SkIPoint_equal_operator_a'>a</a>.<a href='#SkIPoint_fY'>fY</a> == <a href='#SkIPoint_equal_operator_b'>b</a>.<a href='#SkIPoint_fY'>fY</a>

### Example

<div><fiddle-embed name="37ffe2817d720f99e6c252332ce70460">

#### Example Output

~~~~
pt: 0, 0  == pt
pt: -1, -2  == pt
pt: 2147483647, -1  == pt
pt: -2147483648, -1  == pt
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIPoint_equals'>equals()</a> equals()<a href='#SkIPoint_notequal_operator'>operator!=(const SkIPoint& a, const SkIPoint& b)</a>

<a name='SkIPoint_notequal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator!=(const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& <a href='SkIPoint_Reference#SkIPoint'>a</a>, <a href='SkIPoint_Reference#SkIPoint'>const</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& <a href='SkIPoint_Reference#SkIPoint'>b</a>)
</pre>

Returns true if <a href='#SkIPoint_notequal_operator_a'>a</a> <a href='#SkIPoint_notequal_operator_a'>is</a> <a href='#SkIPoint_notequal_operator_a'>not</a> <a href='#SkIPoint_notequal_operator_a'>equivalent</a> <a href='#SkIPoint_notequal_operator_a'>to</a> <a href='#SkIPoint_notequal_operator_b'>b</a>.

### Parameters

<table>  <tr>    <td><a name='SkIPoint_notequal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='SkIPoint_Reference#SkIPoint'>to</a> <a href='SkIPoint_Reference#SkIPoint'>compare</a></td>
  </tr>
  <tr>    <td><a name='SkIPoint_notequal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='SkIPoint_Reference#SkIPoint'>to</a> <a href='SkIPoint_Reference#SkIPoint'>compare</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkIPoint_notequal_operator_a'>a</a>.<a href='#SkIPoint_fX'>fX</a> != <a href='#SkIPoint_notequal_operator_b'>b</a>.<a href='#SkIPoint_fX'>fX</a> <a href='#SkIPoint_fX'>or</a> <a href='#SkIPoint_notequal_operator_a'>a</a>.<a href='#SkIPoint_fY'>fY</a> != <a href='#SkIPoint_notequal_operator_b'>b</a>.<a href='#SkIPoint_fY'>fY</a>

### Example

<div><fiddle-embed name="dd89dc48dff69b53d99530b120f204bc">

#### Example Output

~~~~
pt: 0, 0  == pt
pt: -1, -2  == pt
pt: 2147483647, -1  == pt
pt: -2147483648, -1  == pt
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIPoint_equal_operator'>operator==(const SkIPoint& a, const SkIPoint& b)</a> <a href='#SkIPoint_equals'>equals()</a>

<a name='SkIPoint_subtract_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkIPoint_Reference#SkIVector'>SkIVector</a> <a href='SkIPoint_Reference#SkIVector'>operator</a>-(<a href='SkIPoint_Reference#SkIVector'>const</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& <a href='SkIPoint_Reference#SkIPoint'>a</a>, <a href='SkIPoint_Reference#SkIPoint'>const</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& <a href='SkIPoint_Reference#SkIPoint'>b</a>)
</pre>

Returns <a href='#IPoint_IVector'>IVector</a> <a href='#IPoint_IVector'>from</a> <a href='#SkIPoint_subtract_operator_b'>b</a> <a href='#SkIPoint_subtract_operator_b'>to</a> <a href='#SkIPoint_subtract_operator_a'>a</a>; <a href='#SkIPoint_subtract_operator_a'>computed</a> <a href='#SkIPoint_subtract_operator_a'>as </a> <code>(<a href='#SkIPoint_subtract_operator_a'>a</a>.<a href='#SkIPoint_fX'>fX</a> - <a href='#SkIPoint_subtract_operator_b'>b</a>.<a href='#SkIPoint_fX'>fX</a>, <a href='#SkIPoint_subtract_operator_a'>a</a>.<a href='#SkIPoint_fY'>fY</a> - <a href='#SkIPoint_subtract_operator_b'>b</a>.<a href='#SkIPoint_fY'>fY</a>)</code>.

Can also be used to subtract <a href='#IPoint_IVector'>IVector</a> <a href='#IPoint_IVector'>from</a> <a href='#IPoint_IVector'>IVector</a>, <a href='#IPoint_IVector'>returning</a> <a href='#IPoint_IVector'>IVector</a>.

### Parameters

<table>  <tr>    <td><a name='SkIPoint_subtract_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkIPoint_Reference#IPoint'>IPoint</a> <a href='SkIPoint_Reference#IPoint'>or</a> <a href='#IPoint_IVector'>IVector</a> <a href='#IPoint_IVector'>to</a> <a href='#IPoint_IVector'>subtract</a> <a href='#IPoint_IVector'>from</a></td>
  </tr>
  <tr>    <td><a name='SkIPoint_subtract_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='#IPoint_IVector'>IVector</a> <a href='#IPoint_IVector'>to</a> <a href='#IPoint_IVector'>subtract</a></td>
  </tr>
</table>

### Return Value

<a href='#IPoint_IVector'>IVector</a> <a href='#IPoint_IVector'>from</a> <a href='#SkIPoint_subtract_operator_b'>b</a> <a href='#SkIPoint_subtract_operator_b'>to</a> <a href='#SkIPoint_subtract_operator_a'>a</a>

### Example

<div><fiddle-embed name="e626e26bf557857b824aa7d03f723e0f"></fiddle-embed></div>

### See Also

<a href='#SkIPoint_subtractfrom_operator'>operator-=(const SkIVector& v)</a>

<a name='SkIPoint_add_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='SkIPoint_Reference#SkIPoint'>operator</a>+(<a href='SkIPoint_Reference#SkIPoint'>const</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& <a href='SkIPoint_Reference#SkIPoint'>a</a>, <a href='SkIPoint_Reference#SkIPoint'>const</a> <a href='SkIPoint_Reference#SkIVector'>SkIVector</a>& <a href='SkIPoint_Reference#SkIVector'>b</a>)
</pre>

Returns <a href='SkIPoint_Reference#IPoint'>IPoint</a> <a href='SkIPoint_Reference#IPoint'>resulting</a> <a href='SkIPoint_Reference#IPoint'>from</a> <a href='SkIPoint_Reference#IPoint'>IPoint</a> <a href='#SkIPoint_add_operator_a'>a</a> <a href='#SkIPoint_add_operator_a'>offset</a> <a href='#SkIPoint_add_operator_a'>by</a> <a href='#IPoint_IVector'>IVector</a> <a href='#SkIPoint_add_operator_b'>b</a>, <a href='#SkIPoint_add_operator_b'>computed</a> <a href='#SkIPoint_add_operator_b'>as</a>:
<code>(<a href='#SkIPoint_add_operator_a'>a</a>.<a href='#SkIPoint_fX'>fX</a> + <a href='#SkIPoint_add_operator_b'>b</a>.<a href='#SkIPoint_fX'>fX</a>, <a href='#SkIPoint_add_operator_a'>a</a>.<a href='#SkIPoint_fY'>fY</a> + <a href='#SkIPoint_add_operator_b'>b</a>.<a href='#SkIPoint_fY'>fY</a>)</code>.

Can also be used to offset <a href='SkIPoint_Reference#IPoint'>IPoint</a> <a href='#SkIPoint_add_operator_b'>b</a> <a href='#SkIPoint_add_operator_b'>by</a> <a href='#IPoint_IVector'>IVector</a> <a href='#SkIPoint_add_operator_a'>a</a>, <a href='#SkIPoint_add_operator_a'>returning</a> <a href='SkIPoint_Reference#IPoint'>IPoint</a>.
<a href='SkIPoint_Reference#IPoint'>Can</a> <a href='SkIPoint_Reference#IPoint'>also</a> <a href='SkIPoint_Reference#IPoint'>be</a> <a href='SkIPoint_Reference#IPoint'>used</a> <a href='SkIPoint_Reference#IPoint'>to</a> <a href='SkIPoint_Reference#IPoint'>add</a> <a href='#IPoint_IVector'>IVector</a> <a href='#IPoint_IVector'>to</a> <a href='#IPoint_IVector'>IVector</a>, <a href='#IPoint_IVector'>returning</a> <a href='#IPoint_IVector'>IVector</a>.

### Parameters

<table>  <tr>    <td><a name='SkIPoint_add_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkIPoint_Reference#IPoint'>IPoint</a> <a href='SkIPoint_Reference#IPoint'>or</a> <a href='#IPoint_IVector'>IVector</a> <a href='#IPoint_IVector'>to</a> <a href='#IPoint_IVector'>add</a> <a href='#IPoint_IVector'>to</a></td>
  </tr>
  <tr>    <td><a name='SkIPoint_add_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkIPoint_Reference#IPoint'>IPoint</a> <a href='SkIPoint_Reference#IPoint'>or</a> <a href='#IPoint_IVector'>IVector</a> <a href='#IPoint_IVector'>to</a> <a href='#IPoint_IVector'>add</a></td>
  </tr>
</table>

### Return Value

<a href='SkIPoint_Reference#IPoint'>IPoint</a> <a href='SkIPoint_Reference#IPoint'>equal</a> <a href='SkIPoint_Reference#IPoint'>to</a> <a href='#SkIPoint_add_operator_a'>a</a> <a href='#SkIPoint_add_operator_a'>offset</a> <a href='#SkIPoint_add_operator_a'>by</a> <a href='#SkIPoint_add_operator_b'>b</a>

### Example

<div><fiddle-embed name="63f4cba971c6d8434595906f865b5a29"></fiddle-embed></div>

### See Also

<a href='#SkIPoint_addto_operator'>operator+=(const SkIVector& v)</a>

<a name='IVector'></a>

<a name='SkIVector'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    typedef <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='SkIPoint_Reference#SkIVector'>SkIVector</a>;
</pre>

<a href='SkIPoint_Reference#SkIVector'>SkIVector</a> <a href='SkIPoint_Reference#SkIVector'>provides</a> <a href='SkIPoint_Reference#SkIVector'>an</a> <a href='SkIPoint_Reference#SkIVector'>alternative</a> <a href='SkIPoint_Reference#SkIVector'>name</a> <a href='SkIPoint_Reference#SkIVector'>for</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>. <a href='SkIPoint_Reference#SkIVector'>SkIVector</a> <a href='SkIPoint_Reference#SkIVector'>and</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>
<a href='SkIPoint_Reference#SkIPoint'>can</a> <a href='SkIPoint_Reference#SkIPoint'>be</a> <a href='SkIPoint_Reference#SkIPoint'>used</a> <a href='SkIPoint_Reference#SkIPoint'>interchangeably</a> <a href='SkIPoint_Reference#SkIPoint'>for</a> <a href='SkIPoint_Reference#SkIPoint'>all</a> <a href='SkIPoint_Reference#SkIPoint'>purposes</a>.