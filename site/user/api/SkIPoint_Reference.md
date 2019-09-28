SkIPoint Reference
===


<a name='SkIPoint'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
struct <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> {

    int32_t <a href='#SkIPoint_fX'>fX</a>;
    int32_t <a href='#SkIPoint_fY'>fY</a>;

    static constexpr <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='#SkIPoint_Make'>Make</a>(int32_t x, int32_t y);
    int32_t <a href='#SkIPoint_x'>x()</a> const;
    int32_t <a href='#SkIPoint_y'>y()</a> const;
    bool <a href='#SkIPoint_isZero'>isZero</a>() const;
    void <a href='#SkIPoint_set'>set</a>(int32_t x, int32_t y);
    <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> operator-() const;
    void <a href='#SkIPoint_addto_operator'>operator+=</a>(const <a href='SkIPoint_Reference#SkIVector'>SkIVector</a>& v);
    void <a href='#SkIPoint_subtractfrom_operator'>operator-=</a>(const <a href='SkIPoint_Reference#SkIVector'>SkIVector</a>& v);
    bool <a href='#SkIPoint_equals'>equals</a>(int32_t x, int32_t y) const;
    friend bool <a href='#SkIPoint_equal_operator'>operator==</a>(const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& a, const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& b);
    friend bool <a href='#SkIPoint_notequal_operator'>operator!=</a>(const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& a, const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& b);
    friend <a href='SkIPoint_Reference#SkIVector'>SkIVector</a> <a href='#SkIPoint_subtract_operator'>operator-</a>(const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& a, const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& b);
    friend <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='#SkIPoint_add_operator'>operator+</a>(const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& a, const <a href='SkIPoint_Reference#SkIVector'>SkIVector</a>& b);
};

</pre>

<a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> holds two 32-bit integer coordinates.<table style='border-collapse: collapse; width: 62.5em'>

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
static constexpr <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='#SkIPoint_Make'>Make</a>(int32_t x, int32_t y)
</pre>

Sets <a href='#SkIPoint_fX'>fX</a> to <a href='#SkIPoint_Make_x'>x</a>, <a href='#SkIPoint_fY'>fY</a> to <a href='#SkIPoint_Make_y'>y</a>.

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
int32_t <a href='#SkIPoint_x'>x()</a>const
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
int32_t <a href='#SkIPoint_y'>y()</a>const
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
bool <a href='#SkIPoint_isZero'>isZero</a>()const
</pre>

Returns true if <a href='#SkIPoint_fX'>fX</a> and <a href='#SkIPoint_fY'>fY</a> are both zero.

### Return Value

true if <a href='#SkIPoint_fX'>fX</a> is zero and <a href='#SkIPoint_fY'>fY</a> is zero

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

Sets <a href='#SkIPoint_fX'>fX</a> to <a href='#SkIPoint_set_x'>x</a> and <a href='#SkIPoint_fY'>fY</a> to <a href='#SkIPoint_set_y'>y</a>.

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
<a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> operator-()const
</pre>

Returns <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> changing the signs of <a href='#SkIPoint_fX'>fX</a> and <a href='#SkIPoint_fY'>fY</a>.

### Return Value

<a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> as (-<a href='#SkIPoint_fX'>fX</a>, -<a href='#SkIPoint_fY'>fY</a>)

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

<a href='#SkIPoint_subtract_operator'>operator-</a>(const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& a, const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& b) <a href='#SkIPoint_subtractfrom_operator'>operator-=</a>(const <a href='SkIPoint_Reference#SkIVector'>SkIVector</a>& v) <a href='SkPoint_Reference#SkPoint'>SkPoint</a>::operator-() const

<a name='SkIPoint_addto_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIPoint_addto_operator'>operator+=</a>(const <a href='SkIPoint_Reference#SkIVector'>SkIVector</a>& v)
</pre>

Offsets <a href='SkIPoint_Reference#IPoint'>IPoint</a> by <a href='#IPoint_IVector'>IVector</a> <a href='#SkIPoint_addto_operator_v'>v</a>. Sets <a href='SkIPoint_Reference#IPoint'>IPoint</a> to <code>(<a href='#SkIPoint_fX'>fX</a> + <a href='#SkIPoint_addto_operator_v'>v</a>.<a href='#SkIPoint_fX'>fX</a>, <a href='#SkIPoint_fY'>fY</a> + <a href='#SkIPoint_addto_operator_v'>v</a>.<a href='#SkIPoint_fY'>fY</a>)</code>.

### Parameters

<table>  <tr>    <td><a name='SkIPoint_addto_operator_v'><code><strong>v</strong></code></a></td>
    <td><a href='#IPoint_IVector'>IVector</a> to add</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4eb2d95c9e9a66f05296e345bb68bd51"></fiddle-embed></div>

### See Also

<a href='#SkIPoint_add_operator'>operator+</a>(const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& a, const <a href='SkIPoint_Reference#SkIVector'>SkIVector</a>& b) <a href='SkPoint_Reference#SkPoint'>SkPoint</a>::operator+=(const <a href='SkPoint_Reference#SkVector'>SkVector</a>& <a href='#SkIPoint_addto_operator_v'>v</a>)

<a name='SkIPoint_subtractfrom_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIPoint_subtractfrom_operator'>operator-=</a>(const <a href='SkIPoint_Reference#SkIVector'>SkIVector</a>& v)
</pre>

Subtracts <a href='#IPoint_IVector'>IVector</a> <a href='#SkIPoint_subtractfrom_operator_v'>v</a> from <a href='SkIPoint_Reference#IPoint'>IPoint</a>. Sets <a href='SkIPoint_Reference#IPoint'>IPoint</a> to: <code>(<a href='#SkIPoint_fX'>fX</a> - <a href='#SkIPoint_subtractfrom_operator_v'>v</a>.<a href='#SkIPoint_fX'>fX</a>, <a href='#SkIPoint_fY'>fY</a> - <a href='#SkIPoint_subtractfrom_operator_v'>v</a>.<a href='#SkIPoint_fY'>fY</a>)</code>.

### Parameters

<table>  <tr>    <td><a name='SkIPoint_subtractfrom_operator_v'><code><strong>v</strong></code></a></td>
    <td><a href='#IPoint_IVector'>IVector</a> to subtract</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a01e533dc7ab34ed728dc4e7a5f1f0ee"></fiddle-embed></div>

### See Also

<a href='#SkIPoint_subtract_operator'>operator-</a>(const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& a, const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& b) <a href='SkPoint_Reference#SkPoint'>SkPoint</a>::operator-=(const <a href='SkPoint_Reference#SkVector'>SkVector</a>& <a href='#SkIPoint_subtractfrom_operator_v'>v</a>)

<a name='SkIPoint_equals'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool equals(int32_t x, int32_t y)const
</pre>

Returns true if <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> is equivalent to <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> constructed from (<a href='#SkIPoint_equals_x'>x</a>, <a href='#SkIPoint_equals_y'>y</a>).

### Parameters

<table>  <tr>    <td><a name='SkIPoint_equals_x'><code><strong>x</strong></code></a></td>
    <td>value compared with <a href='#SkIPoint_fX'>fX</a></td>
  </tr>
  <tr>    <td><a name='SkIPoint_equals_y'><code><strong>y</strong></code></a></td>
    <td>value compared with <a href='#SkIPoint_fY'>fY</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> equals (<a href='#SkIPoint_equals_x'>x</a>, <a href='#SkIPoint_equals_y'>y</a>)

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

<a href='#SkIPoint_equal_operator'>operator==</a>(const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& a, const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& b)

<a name='SkIPoint_equal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIPoint_equal_operator'>operator==</a>(const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& a, const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& b)
</pre>

Returns true if <a href='#SkIPoint_equal_operator_a'>a</a> is equivalent to <a href='#SkIPoint_equal_operator_b'>b</a>.

### Parameters

<table>  <tr>    <td><a name='SkIPoint_equal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> to compare</td>
  </tr>
  <tr>    <td><a name='SkIPoint_equal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href='#SkIPoint_equal_operator_a'>a</a>.<a href='#SkIPoint_fX'>fX</a> == <a href='#SkIPoint_equal_operator_b'>b</a>.<a href='#SkIPoint_fX'>fX</a> and <a href='#SkIPoint_equal_operator_a'>a</a>.<a href='#SkIPoint_fY'>fY</a> == <a href='#SkIPoint_equal_operator_b'>b</a>.<a href='#SkIPoint_fY'>fY</a>

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

<a href='#SkIPoint_equals'>equals()</a> <a href='#SkIPoint_notequal_operator'>operator!=</a>(const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& <a href='#SkIPoint_equal_operator_a'>a</a>, const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& <a href='#SkIPoint_equal_operator_b'>b</a>)

<a name='SkIPoint_notequal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIPoint_notequal_operator'>operator!=</a>(const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& a, const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& b)
</pre>

Returns true if <a href='#SkIPoint_notequal_operator_a'>a</a> is not equivalent to <a href='#SkIPoint_notequal_operator_b'>b</a>.

### Parameters

<table>  <tr>    <td><a name='SkIPoint_notequal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> to compare</td>
  </tr>
  <tr>    <td><a name='SkIPoint_notequal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href='#SkIPoint_notequal_operator_a'>a</a>.<a href='#SkIPoint_fX'>fX</a> != <a href='#SkIPoint_notequal_operator_b'>b</a>.<a href='#SkIPoint_fX'>fX</a> or <a href='#SkIPoint_notequal_operator_a'>a</a>.<a href='#SkIPoint_fY'>fY</a> != <a href='#SkIPoint_notequal_operator_b'>b</a>.<a href='#SkIPoint_fY'>fY</a>

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

<a href='#SkIPoint_equal_operator'>operator==</a>(const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& <a href='#SkIPoint_notequal_operator_a'>a</a>, const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& <a href='#SkIPoint_notequal_operator_b'>b</a>) <a href='#SkIPoint_equals'>equals()</a>

<a name='SkIPoint_subtract_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkIPoint_Reference#SkIVector'>SkIVector</a> <a href='#SkIPoint_subtract_operator'>operator-</a>(const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& a, const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& b)
</pre>

Returns <a href='#IPoint_IVector'>IVector</a> from <a href='#SkIPoint_subtract_operator_b'>b</a> to <a href='#SkIPoint_subtract_operator_a'>a</a>; computed as <code>(<a href='#SkIPoint_subtract_operator_a'>a</a>.<a href='#SkIPoint_fX'>fX</a> - <a href='#SkIPoint_subtract_operator_b'>b</a>.<a href='#SkIPoint_fX'>fX</a>, <a href='#SkIPoint_subtract_operator_a'>a</a>.<a href='#SkIPoint_fY'>fY</a> - <a href='#SkIPoint_subtract_operator_b'>b</a>.<a href='#SkIPoint_fY'>fY</a>)</code>.

Can also be used to subtract <a href='#IPoint_IVector'>IVector</a> from <a href='#IPoint_IVector'>IVector</a>, returning <a href='#IPoint_IVector'>IVector</a>.

### Parameters

<table>  <tr>    <td><a name='SkIPoint_subtract_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkIPoint_Reference#IPoint'>IPoint</a> or <a href='#IPoint_IVector'>IVector</a> to subtract from</td>
  </tr>
  <tr>    <td><a name='SkIPoint_subtract_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='#IPoint_IVector'>IVector</a> to subtract</td>
  </tr>
</table>

### Return Value

<a href='#IPoint_IVector'>IVector</a> from <a href='#SkIPoint_subtract_operator_b'>b</a> to <a href='#SkIPoint_subtract_operator_a'>a</a>

### Example

<div><fiddle-embed name="e626e26bf557857b824aa7d03f723e0f"></fiddle-embed></div>

### See Also

<a href='#SkIPoint_subtractfrom_operator'>operator-=</a>(const <a href='SkIPoint_Reference#SkIVector'>SkIVector</a>& v)

<a name='SkIPoint_add_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='#SkIPoint_add_operator'>operator+</a>(const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& a, const <a href='SkIPoint_Reference#SkIVector'>SkIVector</a>& b)
</pre>

Returns <a href='SkIPoint_Reference#IPoint'>IPoint</a> resulting from <a href='SkIPoint_Reference#IPoint'>IPoint</a> <a href='#SkIPoint_add_operator_a'>a</a> offset by <a href='#IPoint_IVector'>IVector</a> <a href='#SkIPoint_add_operator_b'>b</a>, computed as:
<code>(<a href='#SkIPoint_add_operator_a'>a</a>.<a href='#SkIPoint_fX'>fX</a> + <a href='#SkIPoint_add_operator_b'>b</a>.<a href='#SkIPoint_fX'>fX</a>, <a href='#SkIPoint_add_operator_a'>a</a>.<a href='#SkIPoint_fY'>fY</a> + <a href='#SkIPoint_add_operator_b'>b</a>.<a href='#SkIPoint_fY'>fY</a>)</code>.

Can also be used to offset <a href='SkIPoint_Reference#IPoint'>IPoint</a> <a href='#SkIPoint_add_operator_b'>b</a> by <a href='#IPoint_IVector'>IVector</a> <a href='#SkIPoint_add_operator_a'>a</a>, returning <a href='SkIPoint_Reference#IPoint'>IPoint</a>.
Can also be used to add <a href='#IPoint_IVector'>IVector</a> to <a href='#IPoint_IVector'>IVector</a>, returning <a href='#IPoint_IVector'>IVector</a>.

### Parameters

<table>  <tr>    <td><a name='SkIPoint_add_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkIPoint_Reference#IPoint'>IPoint</a> or <a href='#IPoint_IVector'>IVector</a> to add to</td>
  </tr>
  <tr>    <td><a name='SkIPoint_add_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkIPoint_Reference#IPoint'>IPoint</a> or <a href='#IPoint_IVector'>IVector</a> to add</td>
  </tr>
</table>

### Return Value

<a href='SkIPoint_Reference#IPoint'>IPoint</a> equal to <a href='#SkIPoint_add_operator_a'>a</a> offset by <a href='#SkIPoint_add_operator_b'>b</a>

### Example

<div><fiddle-embed name="63f4cba971c6d8434595906f865b5a29"></fiddle-embed></div>

### See Also

<a href='#SkIPoint_addto_operator'>operator+=</a>(const <a href='SkIPoint_Reference#SkIVector'>SkIVector</a>& v)

<a name='IVector'></a>

<a name='SkIVector'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    typedef <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='SkIPoint_Reference#SkIVector'>SkIVector</a>;
</pre>

<a href='SkIPoint_Reference#SkIVector'>SkIVector</a> provides an alternative name for <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>. <a href='SkIPoint_Reference#SkIVector'>SkIVector</a> and <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>
can be used interchangeably for all purposes.