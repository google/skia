SkPoint Reference
===


<a name='SkPoint'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
struct <a href='SkPoint_Reference#SkPoint'>SkPoint</a> {
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_fX'>fX</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_fY'>fY</a>;

    <a href='#SkPoint_fY'>static</a> <a href='#SkPoint_fY'>constexpr</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkPoint_Make'>Make</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>);
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_x'>x()</a> <a href='#SkPoint_x'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_y'>y()</a> <a href='#SkPoint_y'>const</a>;
    <a href='#SkPoint_y'>bool</a> <a href='#SkPoint_isZero'>isZero</a>() <a href='#SkPoint_isZero'>const</a>;
    <a href='#SkPoint_isZero'>void</a> <a href='#SkPoint_isZero'>set</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkPoint_iset'>iset</a>(<a href='#SkPoint_iset'>int32_t</a> <a href='#SkPoint_iset'>x</a>, <a href='#SkPoint_iset'>int32_t</a> <a href='#SkPoint_iset'>y</a>);
    <a href='#SkPoint_iset'>void</a> <a href='#SkPoint_iset'>iset</a>(<a href='#SkPoint_iset'>const</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& <a href='SkIPoint_Reference#SkIPoint'>p</a>);
    <a href='SkIPoint_Reference#SkIPoint'>void</a> <a href='#SkPoint_setAbs'>setAbs</a>(<a href='#SkPoint_setAbs'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>pt</a>);
    <a href='SkPoint_Reference#SkPoint'>static</a> <a href='SkPoint_Reference#SkPoint'>void</a> <a href='#SkPoint_Offset'>Offset</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#Point'>points</a>[], <a href='SkPoint_Reference#Point'>int</a> <a href='SkPoint_Reference#Point'>count</a>, <a href='SkPoint_Reference#Point'>const</a> <a href='SkPoint_Reference#SkVector'>SkVector</a>& <a href='SkPoint_Reference#SkVector'>offset</a>);
    <a href='SkPoint_Reference#SkVector'>static</a> <a href='SkPoint_Reference#SkVector'>void</a> <a href='#SkPoint_Offset'>Offset</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#Point'>points</a>[], <a href='SkPoint_Reference#Point'>int</a> <a href='SkPoint_Reference#Point'>count</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='undocumented#SkScalar'>offset</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>);
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_length'>length()</a> <a href='#SkPoint_length'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_distanceToOrigin'>distanceToOrigin</a>() <a href='#SkPoint_distanceToOrigin'>const</a>;
    <a href='#SkPoint_distanceToOrigin'>bool</a> <a href='#SkPoint_normalize'>normalize()</a>;
    <a href='#SkPoint_normalize'>bool</a> <a href='#SkPoint_setNormalize'>setNormalize</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>);
    <a href='undocumented#SkScalar'>bool</a> <a href='#SkPoint_setLength'>setLength</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>length</a>);
    <a href='undocumented#SkScalar'>bool</a> <a href='#SkPoint_setLength'>setLength</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>length</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='undocumented#SkScalar'>scale</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>scale</a>, <a href='SkPoint_Reference#SkPoint'>SkPoint</a>* <a href='SkPoint_Reference#SkPoint'>dst</a>) <a href='SkPoint_Reference#SkPoint'>const</a>;
    <a href='SkPoint_Reference#SkPoint'>void</a> <a href='SkPoint_Reference#SkPoint'>scale</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>value</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkPoint_negate'>negate()</a>;
    <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>operator</a>-() <a href='SkPoint_Reference#SkPoint'>const</a>;
    <a href='SkPoint_Reference#SkPoint'>void</a> <a href='SkPoint_Reference#SkPoint'>operator</a>+=(<a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkVector'>SkVector</a>& <a href='SkPoint_Reference#SkVector'>v</a>);
    <a href='SkPoint_Reference#SkVector'>void</a> <a href='SkPoint_Reference#SkVector'>operator</a>-=(<a href='SkPoint_Reference#SkVector'>const</a> <a href='SkPoint_Reference#SkVector'>SkVector</a>& <a href='SkPoint_Reference#SkVector'>v</a>);
    <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>operator</a>*(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>scale</a>) <a href='undocumented#SkScalar'>const</a>;
    <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>operator</a>*=(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>scale</a>);
    <a href='undocumented#SkScalar'>bool</a> <a href='#SkPoint_isFinite'>isFinite</a>() <a href='#SkPoint_isFinite'>const</a>;
    <a href='#SkPoint_isFinite'>bool</a> <a href='#SkPoint_isFinite'>equals</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>) <a href='undocumented#SkScalar'>const</a>;
    <a href='undocumented#SkScalar'>friend</a> <a href='undocumented#SkScalar'>bool</a> <a href='undocumented#SkScalar'>operator</a>==(<a href='undocumented#SkScalar'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>a</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>b</a>);
    <a href='SkPoint_Reference#SkPoint'>friend</a> <a href='SkPoint_Reference#SkPoint'>bool</a> <a href='SkPoint_Reference#SkPoint'>operator</a>!=(<a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>a</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>b</a>);
    <a href='SkPoint_Reference#SkPoint'>friend</a> <a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='SkPoint_Reference#SkVector'>operator</a>-(<a href='SkPoint_Reference#SkVector'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>a</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>b</a>);
    <a href='SkPoint_Reference#SkPoint'>friend</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>operator</a>+(<a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>a</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkVector'>SkVector</a>& <a href='SkPoint_Reference#SkVector'>b</a>);
    <a href='SkPoint_Reference#SkVector'>static</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_Length'>Length</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>);
    <a href='undocumented#SkScalar'>static</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_Normalize'>Normalize</a>(<a href='SkPoint_Reference#SkVector'>SkVector</a>* <a href='SkPoint_Reference#SkVector'>vec</a>);
    <a href='SkPoint_Reference#SkVector'>static</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_Distance'>Distance</a>(<a href='#SkPoint_Distance'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>a</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>b</a>);
    <a href='SkPoint_Reference#SkPoint'>static</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_DotProduct'>DotProduct</a>(<a href='#SkPoint_DotProduct'>const</a> <a href='SkPoint_Reference#SkVector'>SkVector</a>& <a href='SkPoint_Reference#SkVector'>a</a>, <a href='SkPoint_Reference#SkVector'>const</a> <a href='SkPoint_Reference#SkVector'>SkVector</a>& <a href='SkPoint_Reference#SkVector'>b</a>);
    <a href='SkPoint_Reference#SkVector'>static</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_CrossProduct'>CrossProduct</a>(<a href='#SkPoint_CrossProduct'>const</a> <a href='SkPoint_Reference#SkVector'>SkVector</a>& <a href='SkPoint_Reference#SkVector'>a</a>, <a href='SkPoint_Reference#SkVector'>const</a> <a href='SkPoint_Reference#SkVector'>SkVector</a>& <a href='SkPoint_Reference#SkVector'>b</a>);
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>cross</a>(<a href='undocumented#SkScalar'>const</a> <a href='SkPoint_Reference#SkVector'>SkVector</a>& <a href='SkPoint_Reference#SkVector'>vec</a>) <a href='SkPoint_Reference#SkVector'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dot</a>(<a href='undocumented#SkScalar'>const</a> <a href='SkPoint_Reference#SkVector'>SkVector</a>& <a href='SkPoint_Reference#SkVector'>vec</a>) <a href='SkPoint_Reference#SkVector'>const</a>;
};
</pre>

<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>holds</a> <a href='SkPoint_Reference#SkPoint'>two</a> 32-<a href='SkPoint_Reference#SkPoint'>bit</a> <a href='SkPoint_Reference#SkPoint'>floating</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>coordinates</a>.<table style='border-collapse: collapse; width: 62.5em'>

  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Member</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPoint_fX'><code>fX</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
x-axis value used by both <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Vector'>Vector</a>. <a href='SkPoint_Reference#Vector'>May</a> <a href='SkPoint_Reference#Vector'>contain</a> <a href='SkPoint_Reference#Vector'>any</a> <a href='SkPoint_Reference#Vector'>value</a>, <a href='SkPoint_Reference#Vector'>including</a>
<a href='SkPoint_Reference#Vector'>infinities</a> <a href='SkPoint_Reference#Vector'>and</a> <a href='SkPoint_Reference#Vector'>NaN</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPoint_fY'><code>fY</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
y-axis value used by both <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Vector'>Vector</a>. <a href='SkPoint_Reference#Vector'>May</a> <a href='SkPoint_Reference#Vector'>contain</a> <a href='SkPoint_Reference#Vector'>any</a> <a href='SkPoint_Reference#Vector'>value</a>, <a href='SkPoint_Reference#Vector'>including</a>
<a href='SkPoint_Reference#Vector'>infinities</a> <a href='SkPoint_Reference#Vector'>and</a> <a href='SkPoint_Reference#Vector'>NaN</a>.
</td>
  </tr>
</table>

<a name='SkPoint_Make'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkPoint_Make'>Make</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>)
</pre>

Sets <a href='#SkPoint_fX'>fX</a> <a href='#SkPoint_fX'>to</a> <a href='#SkPoint_Make_x'>x</a>, <a href='#SkPoint_fY'>fY</a> <a href='#SkPoint_fY'>to</a> <a href='#SkPoint_Make_y'>y</a>. <a href='#SkPoint_Make_y'>Used</a> <a href='#SkPoint_Make_y'>both</a> <a href='#SkPoint_Make_y'>to</a> <a href='#SkPoint_Make_y'>set</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>and</a> <a href='SkPoint_Reference#Vector'>vector</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_Make_x'><code><strong>x</strong></code></a></td>
    <td><a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x-axis</a> <a href='undocumented#SkScalar'>value</a> <a href='undocumented#SkScalar'>of</a> <a href='undocumented#SkScalar'>constructed</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>or</a> <a href='SkPoint_Reference#Vector'>vector</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_Make_y'><code><strong>y</strong></code></a></td>
    <td><a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y-axis</a> <a href='undocumented#SkScalar'>value</a> <a href='undocumented#SkScalar'>of</a> <a href='undocumented#SkScalar'>constructed</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>or</a> <a href='SkPoint_Reference#Vector'>vector</a></td>
  </tr>
</table>

### Return Value

<a href='SkPoint_Reference#SkPoint'>SkPoint</a> (<a href='#SkPoint_Make_x'>x</a>, <a href='#SkPoint_Make_y'>y</a>)

### Example

<div><fiddle-embed name="d266e70977847001f7c42f8a2513bee7">

#### Example Output

~~~~
all equal
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPoint_set'>set()</a> <a href='#SkPoint_iset'>iset()</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>::<a href='#SkIPoint_Make'>Make</a>

<a name='Property'></a>

<a name='SkPoint_x'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_x'>x()</a> <a href='#SkPoint_x'>const</a>
</pre>

Returns x-axis value of <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>or</a> <a href='SkPoint_Reference#Vector'>vector</a>.

### Return Value

<a href='#SkPoint_fX'>fX</a>

### Example

<div><fiddle-embed name="9f3fe446b800ae1d940785d438634941">

#### Example Output

~~~~
pt1.fX == pt1.x()
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPoint_y'>y()</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>::<a href='#SkIPoint_x'>x()</a>

<a name='SkPoint_y'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_y'>y()</a> <a href='#SkPoint_y'>const</a>
</pre>

Returns y-axis value of <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>or</a> <a href='SkPoint_Reference#Vector'>vector</a>.

### Return Value

<a href='#SkPoint_fY'>fY</a>

### Example

<div><fiddle-embed name="4c962850c2dbea4d2325df469400680e">

#### Example Output

~~~~
pt1.fY == pt1.y()
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPoint_x'>x()</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>::<a href='#SkIPoint_y'>y()</a>

<a name='SkPoint_isZero'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPoint_isZero'>isZero</a>() <a href='#SkPoint_isZero'>const</a>
</pre>

Returns true if <a href='#SkPoint_fX'>fX</a> <a href='#SkPoint_fX'>and</a> <a href='#SkPoint_fY'>fY</a> <a href='#SkPoint_fY'>are</a> <a href='#SkPoint_fY'>both</a> <a href='#SkPoint_fY'>zero</a>.

### Return Value

true if <a href='#SkPoint_fX'>fX</a> <a href='#SkPoint_fX'>is</a> <a href='#SkPoint_fX'>zero</a> <a href='#SkPoint_fX'>and</a> <a href='#SkPoint_fY'>fY</a> <a href='#SkPoint_fY'>is</a> <a href='#SkPoint_fY'>zero</a>

### Example

<div><fiddle-embed name="81b9665110b88ef6bcbc20464aed7da1">

#### Example Output

~~~~
pt.fX=+0 pt.fY=-0
pt.isZero() == true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPoint_isFinite'>isFinite</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>::<a href='#SkIPoint_isZero'>isZero</a>

<a name='Set'></a>

<a name='SkPoint_set'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void set(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>)
</pre>

Sets <a href='#SkPoint_fX'>fX</a> <a href='#SkPoint_fX'>to</a> <a href='#SkPoint_set_x'>x</a> <a href='#SkPoint_set_x'>and</a> <a href='#SkPoint_fY'>fY</a> <a href='#SkPoint_fY'>to</a> <a href='#SkPoint_set_y'>y</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_set_x'><code><strong>x</strong></code></a></td>
    <td>new value for <a href='#SkPoint_fX'>fX</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_set_y'><code><strong>y</strong></code></a></td>
    <td>new value for <a href='#SkPoint_fY'>fY</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d08d1e7dafcad4342d1619fdbb2f5781">

#### Example Output

~~~~
pt1 == pt2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPoint_iset'>iset()</a> <a href='#SkPoint_Make'>Make</a>

<a name='SkPoint_iset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPoint_iset'>iset</a>(<a href='#SkPoint_iset'>int32_t</a> <a href='#SkPoint_iset'>x</a>, <a href='#SkPoint_iset'>int32_t</a> <a href='#SkPoint_iset'>y</a>)
</pre>

Sets <a href='#SkPoint_fX'>fX</a> <a href='#SkPoint_fX'>to</a> <a href='#SkPoint_iset_x'>x</a> <a href='#SkPoint_iset_x'>and</a> <a href='#SkPoint_fY'>fY</a> <a href='#SkPoint_fY'>to</a> <a href='#SkPoint_iset_y'>y</a>, <a href='#SkPoint_iset_y'>promoting</a> <a href='#SkPoint_iset_y'>integers</a> <a href='#SkPoint_iset_y'>to</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>values</a>.

Assigning a large integer value directly to <a href='#SkPoint_fX'>fX</a> <a href='#SkPoint_fX'>or</a> <a href='#SkPoint_fY'>fY</a> <a href='#SkPoint_fY'>may</a> <a href='#SkPoint_fY'>cause</a> <a href='#SkPoint_fY'>a</a> <a href='#SkPoint_fY'>compiler</a>
error, triggered by narrowing conversion of int to <a href='undocumented#SkScalar'>SkScalar</a>. <a href='undocumented#SkScalar'>This</a> <a href='undocumented#SkScalar'>safely</a>
casts <a href='#SkPoint_iset_x'>x</a> <a href='#SkPoint_iset_x'>and</a> <a href='#SkPoint_iset_y'>y</a> <a href='#SkPoint_iset_y'>to</a> <a href='#SkPoint_iset_y'>avoid</a> <a href='#SkPoint_iset_y'>the</a> <a href='#SkPoint_iset_y'>error</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_iset_x'><code><strong>x</strong></code></a></td>
    <td>new value for <a href='#SkPoint_fX'>fX</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_iset_y'><code><strong>y</strong></code></a></td>
    <td>new value for <a href='#SkPoint_fY'>fY</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0d9e8ed734981b5b113f22c7bfde5357"></fiddle-embed></div>

### See Also

<a href='#SkPoint_set'>set</a> <a href='#SkPoint_Make'>Make</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>::<a href='#SkIPoint_set'>set</a>

<a name='SkPoint_iset_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPoint_iset'>iset</a>(<a href='#SkPoint_iset'>const</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& <a href='SkIPoint_Reference#SkIPoint'>p</a>)
</pre>

Sets <a href='#SkPoint_fX'>fX</a> <a href='#SkPoint_fX'>to</a> <a href='#SkPoint_iset_2_p'>p</a>.<a href='#SkIPoint_fX'>fX</a> <a href='#SkIPoint_fX'>and</a> <a href='#SkPoint_fY'>fY</a> <a href='#SkPoint_fY'>to</a> <a href='#SkPoint_iset_2_p'>p</a>.<a href='#SkIPoint_fY'>fY</a>, <a href='#SkIPoint_fY'>promoting</a> <a href='#SkIPoint_fY'>integers</a> <a href='#SkIPoint_fY'>to</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>values</a>.

Assigning an <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='SkIPoint_Reference#SkIPoint'>containing</a> <a href='SkIPoint_Reference#SkIPoint'>a</a> <a href='SkIPoint_Reference#SkIPoint'>large</a> <a href='SkIPoint_Reference#SkIPoint'>integer</a> <a href='SkIPoint_Reference#SkIPoint'>value</a> <a href='SkIPoint_Reference#SkIPoint'>directly</a> <a href='SkIPoint_Reference#SkIPoint'>to</a> <a href='#SkPoint_fX'>fX</a> <a href='#SkPoint_fX'>or</a> <a href='#SkPoint_fY'>fY</a> <a href='#SkPoint_fY'>may</a>
cause a compiler error, triggered by narrowing conversion of int to <a href='undocumented#SkScalar'>SkScalar</a>.
This safely casts <a href='#SkPoint_iset_2_p'>p</a>.<a href='#SkIPoint_fX'>fX</a> <a href='#SkIPoint_fX'>and</a> <a href='#SkPoint_iset_2_p'>p</a>.<a href='#SkIPoint_fY'>fY</a> <a href='#SkIPoint_fY'>to</a> <a href='#SkIPoint_fY'>avoid</a> <a href='#SkIPoint_fY'>the</a> <a href='#SkIPoint_fY'>error</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_iset_2_p'><code><strong>p</strong></code></a></td>
    <td><a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='SkIPoint_Reference#SkIPoint'>members</a> <a href='SkIPoint_Reference#SkIPoint'>promoted</a> <a href='SkIPoint_Reference#SkIPoint'>to</a> <a href='undocumented#SkScalar'>SkScalar</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="12b7164a769e232bb772f19c59600ee7">

#### Example Output

~~~~
iPt: -2147483647, 2147483647
fPt: -2.14748e+09, 2.14748e+09
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPoint_set'>set</a> <a href='#SkPoint_Make'>Make</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>::<a href='#SkIPoint_set'>set</a>

<a name='SkPoint_setAbs'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPoint_setAbs'>setAbs</a>(<a href='#SkPoint_setAbs'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>pt</a>)
</pre>

Sets <a href='#SkPoint_fX'>fX</a> <a href='#SkPoint_fX'>to</a> <a href='#SkPoint_fX'>absolute</a> <a href='#SkPoint_fX'>value</a> <a href='#SkPoint_fX'>of</a> <a href='#SkPoint_setAbs_pt'>pt</a>.<a href='#SkPoint_fX'>fX</a>; <a href='#SkPoint_fX'>and</a> <a href='#SkPoint_fY'>fY</a> <a href='#SkPoint_fY'>to</a> <a href='#SkPoint_fY'>absolute</a> <a href='#SkPoint_fY'>value</a> <a href='#SkPoint_fY'>of</a> <a href='#SkPoint_setAbs_pt'>pt</a>.<a href='#SkPoint_fY'>fY</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_setAbs_pt'><code><strong>pt</strong></code></a></td>
    <td>members providing magnitude for <a href='#SkPoint_fX'>fX</a> <a href='#SkPoint_fX'>and</a> <a href='#SkPoint_fY'>fY</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="7f70860e820b67a347cff03c00488426">

#### Example Output

~~~~
pt: 0, -0  abs: 0, 0
pt: -1, -2  abs: 1, 2
pt: inf, -inf  abs: inf, inf
pt: nan, -nan  abs: nan, nan
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPoint_set'>set</a> <a href='#SkPoint_Make'>Make</a> <a href='#SkPoint_negate'>negate</a>

<a name='Offset'></a>

<a name='SkPoint_Offset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static void <a href='#SkPoint_Offset'>Offset</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#Point'>points</a>[], <a href='SkPoint_Reference#Point'>int</a> <a href='SkPoint_Reference#Point'>count</a>, <a href='SkPoint_Reference#Point'>const</a> <a href='SkPoint_Reference#SkVector'>SkVector</a>& <a href='SkPoint_Reference#SkVector'>offset</a>)
</pre>

Adds <a href='#SkPoint_Offset_offset'>offset</a> <a href='#SkPoint_Offset_offset'>to</a> <a href='#SkPoint_Offset_offset'>each</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>in</a> <a href='#SkPoint_Offset_points'>points</a> <a href='#SkPoint_Offset_points'>array</a> <a href='#SkPoint_Offset_points'>with</a> <a href='#SkPoint_Offset_count'>count</a> <a href='#SkPoint_Offset_count'>entries</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_Offset_points'><code><strong>points</strong></code></a></td>
    <td><a href='SkPath_Reference#Point_Array'>SkPoint array</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_Offset_count'><code><strong>count</strong></code></a></td>
    <td>entries in array</td>
  </tr>
  <tr>    <td><a name='SkPoint_Offset_offset'><code><strong>offset</strong></code></a></td>
    <td><a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>added</a> <a href='SkPoint_Reference#Vector'>to</a> <a href='#SkPoint_Offset_points'>points</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f0f24726df78a5d797bcf311e694a0a3"></fiddle-embed></div>

### See Also

<a href='#SkPoint_Offset_offset'>offset</a> offset<a href='#SkPoint_addto_operator'>operator+=(const SkVector& v)</a>

<a name='SkPoint_Offset_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static void <a href='#SkPoint_Offset'>Offset</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#Point'>points</a>[], <a href='SkPoint_Reference#Point'>int</a> <a href='SkPoint_Reference#Point'>count</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>)
</pre>

Adds offset (<a href='#SkPoint_Offset_2_dx'>dx</a>, <a href='#SkPoint_Offset_2_dy'>dy</a>) <a href='#SkPoint_Offset_2_dy'>to</a> <a href='#SkPoint_Offset_2_dy'>each</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>in</a> <a href='#SkPoint_Offset_2_points'>points</a> <a href='#SkPoint_Offset_2_points'>array</a> <a href='#SkPoint_Offset_2_points'>of</a> <a href='#SkPoint_Offset_2_points'>length</a> <a href='#SkPoint_Offset_2_count'>count</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_Offset_2_points'><code><strong>points</strong></code></a></td>
    <td><a href='SkPath_Reference#Point_Array'>SkPoint array</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_Offset_2_count'><code><strong>count</strong></code></a></td>
    <td>entries in array</td>
  </tr>
  <tr>    <td><a name='SkPoint_Offset_2_dx'><code><strong>dx</strong></code></a></td>
    <td>added to <a href='#SkPoint_fX'>fX</a> <a href='#SkPoint_fX'>in</a> <a href='#SkPoint_Offset_2_points'>points</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_Offset_2_dy'><code><strong>dy</strong></code></a></td>
    <td>added to <a href='#SkPoint_fY'>fY</a> <a href='#SkPoint_fY'>in</a> <a href='#SkPoint_Offset_2_points'>points</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="532849faa838de885b86d3ebffae3712"></fiddle-embed></div>

### See Also

<a href='#SkPoint_offset'>offset</a> offset<a href='#SkPoint_addto_operator'>operator+=(const SkVector& v)</a>

<a name='SkPoint_offset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void offset(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>)
</pre>

Adds offset (<a href='#SkPoint_offset_dx'>dx</a>, <a href='#SkPoint_offset_dy'>dy</a>) <a href='#SkPoint_offset_dy'>to</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_offset_dx'><code><strong>dx</strong></code></a></td>
    <td>added to <a href='#SkPoint_fX'>fX</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_offset_dy'><code><strong>dy</strong></code></a></td>
    <td>added to <a href='#SkPoint_fY'>fY</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="02750ceaa874f956e6e6544ef6b858ee"></fiddle-embed></div>

### See Also

<a href='#SkPoint_Offset'>Offset</a> Offset<a href='#SkPoint_addto_operator'>operator+=(const SkVector& v)</a>

<a name='SkPoint_length'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_length'>length()</a> <a href='#SkPoint_length'>const</a>
</pre>

Returns the Euclidean distance from origin, computed as:

<a href='undocumented#sqrt()'>sqrt</a>(<a href='#SkPoint_fX'>fX</a> * <a href='#SkPoint_fX'>fX</a> + <a href='#SkPoint_fY'>fY</a> * <a href='#SkPoint_fY'>fY</a>)

.

### Return Value

straight-line distance to origin

### Example

<div><fiddle-embed name="8363ab179447ee4b827679e20d3d81eb"></fiddle-embed></div>

### See Also

<a href='#SkPoint_distanceToOrigin'>distanceToOrigin</a> <a href='#SkPoint_Length'>Length</a> <a href='#SkPoint_setLength'>setLength</a> <a href='#SkPoint_Distance'>Distance</a>

<a name='SkPoint_distanceToOrigin'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_distanceToOrigin'>distanceToOrigin</a>() <a href='#SkPoint_distanceToOrigin'>const</a>
</pre>

Returns the Euclidean distance from origin, computed as:

<a href='undocumented#sqrt()'>sqrt</a>(<a href='#SkPoint_fX'>fX</a> * <a href='#SkPoint_fX'>fX</a> + <a href='#SkPoint_fY'>fY</a> * <a href='#SkPoint_fY'>fY</a>)

.

### Return Value

straight-line distance to origin

### Example

<div><fiddle-embed name="812cf26d91b1cdcd2c6b9438a8172518"></fiddle-embed></div>

### See Also

<a href='#SkPoint_length'>length</a> <a href='#SkPoint_Length'>Length</a> <a href='#SkPoint_setLength'>setLength</a> <a href='#SkPoint_Distance'>Distance</a>

<a name='SkPoint_normalize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPoint_normalize'>normalize()</a>
</pre>

Scales (<a href='#SkPoint_fX'>fX</a>, <a href='#SkPoint_fY'>fY</a>) <a href='#SkPoint_fY'>so</a> <a href='#SkPoint_fY'>that</a> <a href='#SkPoint_length'>length()</a> <a href='#SkPoint_length'>returns</a> <a href='#SkPoint_length'>one</a>, <a href='#SkPoint_length'>while</a> <a href='#SkPoint_length'>preserving</a> <a href='#SkPoint_length'>ratio</a> <a href='#SkPoint_length'>of</a> <a href='#SkPoint_fX'>fX</a> <a href='#SkPoint_fX'>to</a> <a href='#SkPoint_fY'>fY</a>,
if possible. If prior length is nearly zero, sets <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>to</a> (0, 0) <a href='SkPoint_Reference#Vector'>and</a> <a href='SkPoint_Reference#Vector'>returns</a>
false; otherwise returns true.

### Return Value

true if former length is not zero or nearly zero

### Example

<div><fiddle-embed name="d84fce292d86c7d9ef37ae2d179c03c7"></fiddle-embed></div>

### See Also

<a href='#SkPoint_Normalize'>Normalize</a> <a href='#SkPoint_setLength'>setLength</a> <a href='#SkPoint_length'>length</a> <a href='#SkPoint_Length'>Length</a>

<a name='SkPoint_setNormalize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPoint_setNormalize'>setNormalize</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>)
</pre>

Sets <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>to</a> (<a href='#SkPoint_setNormalize_x'>x</a>, <a href='#SkPoint_setNormalize_y'>y</a>) <a href='#SkPoint_setNormalize_y'>scaled</a> <a href='#SkPoint_setNormalize_y'>so</a> <a href='#SkPoint_length'>length()</a> <a href='#SkPoint_length'>returns</a> <a href='#SkPoint_length'>one</a>, <a href='#SkPoint_length'>and</a> <a href='#SkPoint_length'>so</a> <a href='#SkPoint_length'>that</a>
(<a href='#SkPoint_fX'>fX</a>, <a href='#SkPoint_fY'>fY</a>) <a href='#SkPoint_fY'>is</a> <a href='#SkPoint_fY'>proportional</a> <a href='#SkPoint_fY'>to</a> (<a href='#SkPoint_setNormalize_x'>x</a>, <a href='#SkPoint_setNormalize_y'>y</a>).  <a href='#SkPoint_setNormalize_y'>If</a> (<a href='#SkPoint_setNormalize_x'>x</a>, <a href='#SkPoint_setNormalize_y'>y</a>) <a href='#SkPoint_setNormalize_y'>length</a> <a href='#SkPoint_setNormalize_y'>is</a> <a href='#SkPoint_setNormalize_y'>nearly</a> <a href='#SkPoint_setNormalize_y'>zero</a>,
sets <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>to</a> (0, 0) <a href='SkPoint_Reference#Vector'>and</a> <a href='SkPoint_Reference#Vector'>returns</a> <a href='SkPoint_Reference#Vector'>false</a>; <a href='SkPoint_Reference#Vector'>otherwise</a> <a href='SkPoint_Reference#Vector'>returns</a> <a href='SkPoint_Reference#Vector'>true</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_setNormalize_x'><code><strong>x</strong></code></a></td>
    <td>proportional value for <a href='#SkPoint_fX'>fX</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_setNormalize_y'><code><strong>y</strong></code></a></td>
    <td>proportional value for <a href='#SkPoint_fY'>fY</a></td>
  </tr>
</table>

### Return Value

true if (<a href='#SkPoint_setNormalize_x'>x</a>, <a href='#SkPoint_setNormalize_y'>y</a>) <a href='#SkPoint_setNormalize_y'>length</a> <a href='#SkPoint_setNormalize_y'>is</a> <a href='#SkPoint_setNormalize_y'>not</a> <a href='#SkPoint_setNormalize_y'>zero</a> <a href='#SkPoint_setNormalize_y'>or</a> <a href='#SkPoint_setNormalize_y'>nearly</a> <a href='#SkPoint_setNormalize_y'>zero</a>

### Example

<div><fiddle-embed name="3e4f147d143a388802484bf0d26534c2"></fiddle-embed></div>

### See Also

<a href='#SkPoint_normalize'>normalize</a> <a href='#SkPoint_setLength'>setLength</a>

<a name='SkPoint_setLength'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPoint_setLength'>setLength</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>length</a>)
</pre>

Scales <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>so</a> <a href='SkPoint_Reference#Vector'>that</a> <a href='#SkPoint_distanceToOrigin'>distanceToOrigin</a>() <a href='#SkPoint_distanceToOrigin'>returns</a> <a href='#SkPoint_setLength_length'>length</a>, <a href='#SkPoint_setLength_length'>if</a> <a href='#SkPoint_setLength_length'>possible</a>. <a href='#SkPoint_setLength_length'>If</a> <a href='#SkPoint_setLength_length'>former</a>
<a href='#SkPoint_setLength_length'>length</a> <a href='#SkPoint_setLength_length'>is</a> <a href='#SkPoint_setLength_length'>nearly</a> <a href='#SkPoint_setLength_length'>zero</a>, <a href='#SkPoint_setLength_length'>sets</a> <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>to</a> (0, 0) <a href='SkPoint_Reference#Vector'>and</a> <a href='SkPoint_Reference#Vector'>return</a> <a href='SkPoint_Reference#Vector'>false</a>; <a href='SkPoint_Reference#Vector'>otherwise</a> <a href='SkPoint_Reference#Vector'>returns</a>
true.

### Parameters

<table>  <tr>    <td><a name='SkPoint_setLength_length'><code><strong>length</strong></code></a></td>
    <td>straight-line distance to origin</td>
  </tr>
</table>

### Return Value

true if former <a href='#SkPoint_setLength_length'>length</a> <a href='#SkPoint_setLength_length'>is</a> <a href='#SkPoint_setLength_length'>not</a> <a href='#SkPoint_setLength_length'>zero</a> <a href='#SkPoint_setLength_length'>or</a> <a href='#SkPoint_setLength_length'>nearly</a> <a href='#SkPoint_setLength_length'>zero</a>

### Example

<div><fiddle-embed name="cbe7db206ece825aa3b9b7c3256aeaf0"></fiddle-embed></div>

### See Also

<a href='#SkPoint_setLength_length'>length</a> <a href='#SkPoint_Length'>Length</a> <a href='#SkPoint_setNormalize'>setNormalize</a> <a href='#SkPoint_setAbs'>setAbs</a>

<a name='SkPoint_setLength_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPoint_setLength'>setLength</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>length</a>)
</pre>

Sets <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>to</a> (<a href='#SkPoint_setLength_2_x'>x</a>, <a href='#SkPoint_setLength_2_y'>y</a>) <a href='#SkPoint_setLength_2_y'>scaled</a> <a href='#SkPoint_setLength_2_y'>to</a> <a href='#SkPoint_setLength_2_length'>length</a>, <a href='#SkPoint_setLength_2_length'>if</a> <a href='#SkPoint_setLength_2_length'>possible</a>. <a href='#SkPoint_setLength_2_length'>If</a> <a href='#SkPoint_setLength_2_length'>former</a>
<a href='#SkPoint_setLength_2_length'>length</a> <a href='#SkPoint_setLength_2_length'>is</a> <a href='#SkPoint_setLength_2_length'>nearly</a> <a href='#SkPoint_setLength_2_length'>zero</a>, <a href='#SkPoint_setLength_2_length'>sets</a> <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>to</a> (0, 0) <a href='SkPoint_Reference#Vector'>and</a> <a href='SkPoint_Reference#Vector'>return</a> <a href='SkPoint_Reference#Vector'>false</a>; <a href='SkPoint_Reference#Vector'>otherwise</a> <a href='SkPoint_Reference#Vector'>returns</a>
true.

### Parameters

<table>  <tr>    <td><a name='SkPoint_setLength_2_x'><code><strong>x</strong></code></a></td>
    <td>proportional value for <a href='#SkPoint_fX'>fX</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_setLength_2_y'><code><strong>y</strong></code></a></td>
    <td>proportional value for <a href='#SkPoint_fY'>fY</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_setLength_2_length'><code><strong>length</strong></code></a></td>
    <td>straight-line distance to origin</td>
  </tr>
</table>

### Return Value

true if (<a href='#SkPoint_setLength_2_x'>x</a>, <a href='#SkPoint_setLength_2_y'>y</a>) <a href='#SkPoint_setLength_2_length'>length</a> <a href='#SkPoint_setLength_2_length'>is</a> <a href='#SkPoint_setLength_2_length'>not</a> <a href='#SkPoint_setLength_2_length'>zero</a> <a href='#SkPoint_setLength_2_length'>or</a> <a href='#SkPoint_setLength_2_length'>nearly</a> <a href='#SkPoint_setLength_2_length'>zero</a>

### Example

<div><fiddle-embed name="3cc0662b6fbbee1fe3442a0acfece22c"></fiddle-embed></div>

### See Also

<a href='#SkPoint_setLength_2_length'>length</a> <a href='#SkPoint_Length'>Length</a> <a href='#SkPoint_setNormalize'>setNormalize</a> <a href='#SkPoint_setAbs'>setAbs</a>

<a name='SkPoint_scale'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void scale(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>scale</a>, <a href='SkPoint_Reference#SkPoint'>SkPoint</a>* <a href='SkPoint_Reference#SkPoint'>dst</a>) <a href='SkPoint_Reference#SkPoint'>const</a>
</pre>

Sets <a href='#SkPoint_scale_dst'>dst</a> <a href='#SkPoint_scale_dst'>to</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>times</a> <a href='#SkPoint_scale_scale'>scale</a>. <a href='#SkPoint_scale_dst'>dst</a> <a href='#SkPoint_scale_dst'>may</a> <a href='#SkPoint_scale_dst'>be</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>to</a> <a href='SkPoint_Reference#SkPoint'>modify</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>in</a> <a href='SkPoint_Reference#SkPoint'>place</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_scale_scale'><code><strong>scale</strong></code></a></td>
    <td>factor to multiply <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>by</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_scale_dst'><code><strong>dst</strong></code></a></td>
    <td>storage for scaled <a href='SkPoint_Reference#SkPoint'>SkPoint</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="972e4e230806281adb928e068bcd8551"></fiddle-embed></div>

### See Also

<a href='#SkPoint_multiply_operator'>operator*(SkScalar scale)_const</a> operator*(SkScalar scale)_const<a href='#SkPoint_multiplyby_operator'>operator*=(SkScalar scale)</a> <a href='#SkPoint_setLength'>setLength</a>

<a name='SkPoint_scale_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void scale(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>value</a>)
</pre>

Scales <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>in</a> <a href='SkPoint_Reference#SkPoint'>place</a> <a href='SkPoint_Reference#SkPoint'>by</a> <a href='SkPoint_Reference#SkPoint'>scale</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_scale_2_value'><code><strong>value</strong></code></a></td>
    <td>factor to multiply <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>by</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1060a4f27d8ef29519e6ac006ce90f2b"></fiddle-embed></div>

### See Also

<a href='#SkPoint_multiply_operator'>operator*(SkScalar scale)_const</a> operator*(SkScalar scale)_const<a href='#SkPoint_multiplyby_operator'>operator*=(SkScalar scale)</a> <a href='#SkPoint_setLength'>setLength</a>

<a name='SkPoint_negate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPoint_negate'>negate()</a>
</pre>

Changes the sign of <a href='#SkPoint_fX'>fX</a> <a href='#SkPoint_fX'>and</a> <a href='#SkPoint_fY'>fY</a>.

### Example

<div><fiddle-embed name="312c0c8065ab5d0adfda80cccf2d11e6">

#### Example Output

~~~~
pt: 0, -0  negate: -0, 0
pt: -1, -2  negate: 1, 2
pt: inf, -inf  negate: -inf, inf
pt: nan, -nan  negate: -nan, nan
~~~~

</fiddle-embed></div>

### See Also

operator-()_const <a href='#SkPoint_setAbs'>setAbs</a>

<a name='SkPoint_minus_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>operator</a>-() <a href='SkPoint_Reference#SkPoint'>const</a>
</pre>

Returns <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>changing</a> <a href='SkPoint_Reference#SkPoint'>the</a> <a href='SkPoint_Reference#SkPoint'>signs</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='#SkPoint_fX'>fX</a> <a href='#SkPoint_fX'>and</a> <a href='#SkPoint_fY'>fY</a>.

### Return Value

<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>as</a> (-<a href='#SkPoint_fX'>fX</a>, -<a href='#SkPoint_fY'>fY</a>)

### Example

<div><fiddle-embed name="9baf247cfcd8272c0ddf6ce93f676b37">

#### Example Output

~~~~
pt: 0, -0  negate: -0, 0
pt: -1, -2  negate: 1, 2
pt: inf, -inf  negate: -inf, inf
pt: nan, -nan  negate: -nan, nan
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPoint_negate'>negate</a> negate<a href='#SkPoint_subtract_operator'>operator-(const SkPoint& a, const SkPoint& b)</a> operator-(const SkPoint& a, const SkPoint& b)<a href='#SkPoint_subtractfrom_operator'>operator-=(const SkVector& v)</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>::<a href='SkIPoint_Reference#SkIPoint'>operator</a>-()_<a href='SkIPoint_Reference#SkIPoint'>const</a>

<a name='SkPoint_addto_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void operator+=(const <a href='SkPoint_Reference#SkVector'>SkVector</a>& <a href='SkPoint_Reference#SkVector'>v</a>)
</pre>

Adds <a href='SkPoint_Reference#Vector'>Vector</a> <a href='#SkPoint_addto_operator_v'>v</a> <a href='#SkPoint_addto_operator_v'>to</a> <a href='SkPoint_Reference#Point'>Point</a>. <a href='SkPoint_Reference#Point'>Sets</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>to</a>: <code>(<a href='#SkPoint_fX'>fX</a> + <a href='#SkPoint_addto_operator_v'>v</a>.<a href='#SkPoint_fX'>fX</a>, <a href='#SkPoint_fY'>fY</a> + <a href='#SkPoint_addto_operator_v'>v</a>.<a href='#SkPoint_fY'>fY</a>)</code>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_addto_operator_v'><code><strong>v</strong></code></a></td>
    <td><a href='SkPoint_Reference#Vector'>Vector</a> <a href='SkPoint_Reference#Vector'>to</a> <a href='SkPoint_Reference#Vector'>add</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="8b4e79109e2381345258cb744881b20c"></fiddle-embed></div>

### See Also

<a href='#SkPoint_offset'>offset()</a> offset()<a href='#SkPoint_add_operator'>operator+(const SkPoint& a, const SkVector& b)</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>::SkIPoint<a href='#SkIPoint_addto_operator'>operator+=(const SkIVector& v)</a>

<a name='SkPoint_subtractfrom_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void operator-=(const <a href='SkPoint_Reference#SkVector'>SkVector</a>& <a href='SkPoint_Reference#SkVector'>v</a>)
</pre>

Subtracts <a href='SkPoint_Reference#Vector'>Vector</a> <a href='#SkPoint_subtractfrom_operator_v'>v</a> <a href='#SkPoint_subtractfrom_operator_v'>from</a> <a href='SkPoint_Reference#Point'>Point</a>. <a href='SkPoint_Reference#Point'>Sets</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>to</a>: <code>(<a href='#SkPoint_fX'>fX</a> - <a href='#SkPoint_subtractfrom_operator_v'>v</a>.<a href='#SkPoint_fX'>fX</a>, <a href='#SkPoint_fY'>fY</a> - <a href='#SkPoint_subtractfrom_operator_v'>v</a>.<a href='#SkPoint_fY'>fY</a>)</code>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_subtractfrom_operator_v'><code><strong>v</strong></code></a></td>
    <td><a href='SkPoint_Reference#Vector'>Vector</a> <a href='SkPoint_Reference#Vector'>to</a> <a href='SkPoint_Reference#Vector'>subtract</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="86c0399704d8dff4091bf87b8d87d40b"></fiddle-embed></div>

### See Also

<a href='#SkPoint_offset'>offset()</a> offset()<a href='#SkPoint_subtract_operator'>operator-(const SkPoint& a, const SkPoint& b)</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>::SkIPoint<a href='#SkIPoint_subtractfrom_operator'>operator-=(const SkIVector& v)</a>

<a name='SkPoint_multiply_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>operator</a>*(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>scale</a>) <a href='undocumented#SkScalar'>const</a>
</pre>

Returns <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>multiplied</a> <a href='SkPoint_Reference#SkPoint'>by</a> <a href='#SkPoint_multiply_operator_scale'>scale</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_multiply_operator_scale'><code><strong>scale</strong></code></a></td>
    <td><a href='undocumented#Scalar'>scalar</a> <a href='undocumented#Scalar'>to</a> <a href='undocumented#Scalar'>multiply</a> <a href='undocumented#Scalar'>by</a></td>
  </tr>
</table>

### Return Value

<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>as</a> (<a href='#SkPoint_fX'>fX</a> * <a href='#SkPoint_multiply_operator_scale'>scale</a>, <a href='#SkPoint_fY'>fY</a> * <a href='#SkPoint_multiply_operator_scale'>scale</a>)

### Example

<div><fiddle-embed name="35b3bc675779de043706ae4817ee950c"></fiddle-embed></div>

### See Also

<a href='#SkPoint_multiplyby_operator'>operator*=(SkScalar scale)</a> <a href='#SkPoint_scale'>scale()</a> <a href='#SkPoint_setLength'>setLength</a> <a href='#SkPoint_setNormalize'>setNormalize</a>

<a name='SkPoint_multiplyby_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>operator</a>*=(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>scale</a>)
</pre>

Multiplies <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>by</a> <a href='#SkPoint_multiplyby_operator_scale'>scale</a>. <a href='#SkPoint_multiplyby_operator_scale'>Sets</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>to</a>: <code>(<a href='#SkPoint_fX'>fX</a> * <a href='#SkPoint_multiplyby_operator_scale'>scale</a>, <a href='#SkPoint_fY'>fY</a> * <a href='#SkPoint_multiplyby_operator_scale'>scale</a>)</code>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_multiplyby_operator_scale'><code><strong>scale</strong></code></a></td>
    <td><a href='undocumented#Scalar'>Scalar</a> <a href='undocumented#Scalar'>to</a> <a href='undocumented#Scalar'>multiply</a> <a href='undocumented#Scalar'>by</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPoint_Reference#Point'>Point</a>

### Example

<div><fiddle-embed name="3ce3db36235d80dbac4d39504cf756da"></fiddle-embed></div>

### See Also

<a href='#SkPoint_multiply_operator'>operator*(SkScalar scale)_const</a> <a href='#SkPoint_scale'>scale()</a> <a href='#SkPoint_setLength'>setLength</a> <a href='#SkPoint_setNormalize'>setNormalize</a>

<a name='SkPoint_isFinite'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPoint_isFinite'>isFinite</a>() <a href='#SkPoint_isFinite'>const</a>
</pre>

Returns true if both <a href='#SkPoint_fX'>fX</a> <a href='#SkPoint_fX'>and</a> <a href='#SkPoint_fY'>fY</a> <a href='#SkPoint_fY'>are</a> <a href='#SkPoint_fY'>measurable</a> <a href='#SkPoint_fY'>values</a>.

### Return Value

true for values other than infinities and NaN

### Example

<div><fiddle-embed name="937cc166cc0e220f33fb82501141d0b3">

#### Example Output

~~~~
pt: 0, -0  finite: true
pt: -1, -2  finite: true
pt: inf, 1  finite: false
pt: nan, -1  finite: false
~~~~

</fiddle-embed></div>

### See Also

<a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_isFinite'>isFinite</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_isFinite'>isFinite</a>

<a name='SkPoint_equals'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool equals(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>) <a href='undocumented#SkScalar'>const</a>
</pre>

Returns true if <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>is</a> <a href='SkPoint_Reference#SkPoint'>equivalent</a> <a href='SkPoint_Reference#SkPoint'>to</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>constructed</a> <a href='SkPoint_Reference#SkPoint'>from</a> (<a href='#SkPoint_equals_x'>x</a>, <a href='#SkPoint_equals_y'>y</a>).

### Parameters

<table>  <tr>    <td><a name='SkPoint_equals_x'><code><strong>x</strong></code></a></td>
    <td>value compared with <a href='#SkPoint_fX'>fX</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_equals_y'><code><strong>y</strong></code></a></td>
    <td>value compared with <a href='#SkPoint_fY'>fY</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>equals</a> (<a href='#SkPoint_equals_x'>x</a>, <a href='#SkPoint_equals_y'>y</a>)

### Example

<div><fiddle-embed name="4cecb878c8b66beffda051f26c00f817">

#### Example Output

~~~~
pt: 0, -0  == pt
pt: -1, -2  == pt
pt: inf, 1  == pt
pt: nan, -1  != pt
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPoint_equal_operator'>operator==(const SkPoint& a, const SkPoint& b)</a>

<a name='SkPoint_equal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator==(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>a</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>b</a>)
</pre>

Returns true if <a href='#SkPoint_equal_operator_a'>a</a> <a href='#SkPoint_equal_operator_a'>is</a> <a href='#SkPoint_equal_operator_a'>equivalent</a> <a href='#SkPoint_equal_operator_a'>to</a> <a href='#SkPoint_equal_operator_b'>b</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_equal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>to</a> <a href='SkPoint_Reference#SkPoint'>compare</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_equal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>to</a> <a href='SkPoint_Reference#SkPoint'>compare</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkPoint_equal_operator_a'>a</a>.<a href='#SkPoint_fX'>fX</a> == <a href='#SkPoint_equal_operator_b'>b</a>.<a href='#SkPoint_fX'>fX</a> <a href='#SkPoint_fX'>and</a> <a href='#SkPoint_equal_operator_a'>a</a>.<a href='#SkPoint_fY'>fY</a> == <a href='#SkPoint_equal_operator_b'>b</a>.<a href='#SkPoint_fY'>fY</a>

### Example

<div><fiddle-embed name="741f793334a48a35dadf4310d7ea52cb">

#### Example Output

~~~~
pt: 0, -0  == pt
pt: -1, -2  == pt
pt: inf, 1  == pt
pt: nan, -1  != pt
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPoint_equals'>equals()</a> equals()<a href='#SkPoint_notequal_operator'>operator!=(const SkPoint& a, const SkPoint& b)</a>

<a name='SkPoint_notequal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator!=(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>a</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>b</a>)
</pre>

Returns true if <a href='#SkPoint_notequal_operator_a'>a</a> <a href='#SkPoint_notequal_operator_a'>is</a> <a href='#SkPoint_notequal_operator_a'>not</a> <a href='#SkPoint_notequal_operator_a'>equivalent</a> <a href='#SkPoint_notequal_operator_a'>to</a> <a href='#SkPoint_notequal_operator_b'>b</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_notequal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>to</a> <a href='SkPoint_Reference#SkPoint'>compare</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_notequal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>to</a> <a href='SkPoint_Reference#SkPoint'>compare</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkPoint_notequal_operator_a'>a</a>.<a href='#SkPoint_fX'>fX</a> != <a href='#SkPoint_notequal_operator_b'>b</a>.<a href='#SkPoint_fX'>fX</a> <a href='#SkPoint_fX'>or</a> <a href='#SkPoint_notequal_operator_a'>a</a>.<a href='#SkPoint_fY'>fY</a> != <a href='#SkPoint_notequal_operator_b'>b</a>.<a href='#SkPoint_fY'>fY</a>

### Example

<div><fiddle-embed name="8fe8572685eaa617f25a5a6767a874dc">

#### Example Output

~~~~
pt: 0, -0  == pt
pt: -1, -2  == pt
pt: inf, 1  == pt
pt: nan, -1  != pt
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPoint_equal_operator'>operator==(const SkPoint& a, const SkPoint& b)</a> <a href='#SkPoint_equals'>equals()</a>

<a name='SkPoint_subtract_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='SkPoint_Reference#SkVector'>operator</a>-(<a href='SkPoint_Reference#SkVector'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>a</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>b</a>)
</pre>

Returns <a href='SkPoint_Reference#Vector'>Vector</a> <a href='SkPoint_Reference#Vector'>from</a> <a href='#SkPoint_subtract_operator_b'>b</a> <a href='#SkPoint_subtract_operator_b'>to</a> <a href='#SkPoint_subtract_operator_a'>a</a>, <a href='#SkPoint_subtract_operator_a'>computed</a> <a href='#SkPoint_subtract_operator_a'>as </a> <code>(<a href='#SkPoint_subtract_operator_a'>a</a>.<a href='#SkPoint_fX'>fX</a> - <a href='#SkPoint_subtract_operator_b'>b</a>.<a href='#SkPoint_fX'>fX</a>, <a href='#SkPoint_subtract_operator_a'>a</a>.<a href='#SkPoint_fY'>fY</a> - <a href='#SkPoint_subtract_operator_b'>b</a>.<a href='#SkPoint_fY'>fY</a>)</code>.

Can also be used to subtract <a href='SkPoint_Reference#Vector'>Vector</a> <a href='SkPoint_Reference#Vector'>from</a> <a href='SkPoint_Reference#Point'>Point</a>, <a href='SkPoint_Reference#Point'>returning</a> <a href='SkPoint_Reference#Point'>Point</a>.
<a href='SkPoint_Reference#Point'>Can</a> <a href='SkPoint_Reference#Point'>also</a> <a href='SkPoint_Reference#Point'>be</a> <a href='SkPoint_Reference#Point'>used</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>subtract</a> <a href='SkPoint_Reference#Vector'>Vector</a> <a href='SkPoint_Reference#Vector'>from</a> <a href='SkPoint_Reference#Vector'>Vector</a>, <a href='SkPoint_Reference#Vector'>returning</a> <a href='SkPoint_Reference#Vector'>Vector</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_subtract_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>subtract</a> <a href='SkPoint_Reference#Point'>from</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_subtract_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>subtract</a></td>
  </tr>
</table>

### Return Value

<a href='SkPoint_Reference#Vector'>Vector</a> <a href='SkPoint_Reference#Vector'>from</a> <a href='#SkPoint_subtract_operator_b'>b</a> <a href='#SkPoint_subtract_operator_b'>to</a> <a href='#SkPoint_subtract_operator_a'>a</a>

### Example

<div><fiddle-embed name="b6c4943ecd0b2dccf9d220b8944009e0"></fiddle-embed></div>

### See Also

<a href='#SkPoint_subtractfrom_operator'>operator-=(const SkVector& v)</a> <a href='#SkPoint_offset'>offset()</a>

<a name='SkPoint_add_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>operator</a>+(<a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>a</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkVector'>SkVector</a>& <a href='SkPoint_Reference#SkVector'>b</a>)
</pre>

Returns <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>resulting</a> <a href='SkPoint_Reference#Point'>from</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='#SkPoint_add_operator_a'>a</a> <a href='#SkPoint_add_operator_a'>offset</a> <a href='#SkPoint_add_operator_a'>by</a> <a href='SkPoint_Reference#Vector'>Vector</a> <a href='#SkPoint_add_operator_b'>b</a>, <a href='#SkPoint_add_operator_b'>computed</a> <a href='#SkPoint_add_operator_b'>as</a>:
<code>(<a href='#SkPoint_add_operator_a'>a</a>.<a href='#SkPoint_fX'>fX</a> + <a href='#SkPoint_add_operator_b'>b</a>.<a href='#SkPoint_fX'>fX</a>, <a href='#SkPoint_add_operator_a'>a</a>.<a href='#SkPoint_fY'>fY</a> + <a href='#SkPoint_add_operator_b'>b</a>.<a href='#SkPoint_fY'>fY</a>)</code>.

Can also be used to offset <a href='SkPoint_Reference#Point'>Point</a> <a href='#SkPoint_add_operator_b'>b</a> <a href='#SkPoint_add_operator_b'>by</a> <a href='SkPoint_Reference#Vector'>Vector</a> <a href='#SkPoint_add_operator_a'>a</a>, <a href='#SkPoint_add_operator_a'>returning</a> <a href='SkPoint_Reference#Point'>Point</a>.
<a href='SkPoint_Reference#Point'>Can</a> <a href='SkPoint_Reference#Point'>also</a> <a href='SkPoint_Reference#Point'>be</a> <a href='SkPoint_Reference#Point'>used</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>add</a> <a href='SkPoint_Reference#Vector'>Vector</a> <a href='SkPoint_Reference#Vector'>to</a> <a href='SkPoint_Reference#Vector'>Vector</a>, <a href='SkPoint_Reference#Vector'>returning</a> <a href='SkPoint_Reference#Vector'>Vector</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_add_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>or</a> <a href='SkPoint_Reference#Vector'>Vector</a> <a href='SkPoint_Reference#Vector'>to</a> <a href='SkPoint_Reference#Vector'>add</a> <a href='SkPoint_Reference#Vector'>to</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_add_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>or</a> <a href='SkPoint_Reference#Vector'>Vector</a> <a href='SkPoint_Reference#Vector'>to</a> <a href='SkPoint_Reference#Vector'>add</a></td>
  </tr>
</table>

### Return Value

<a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>equal</a> <a href='SkPoint_Reference#Point'>to</a> <a href='#SkPoint_add_operator_a'>a</a> <a href='#SkPoint_add_operator_a'>offset</a> <a href='#SkPoint_add_operator_a'>by</a> <a href='#SkPoint_add_operator_b'>b</a>

### Example

<div><fiddle-embed name="911a84253dfec4dabf94dbe3c71766f0"></fiddle-embed></div>

### See Also

<a href='#SkPoint_addto_operator'>operator+=(const SkVector& v)</a> <a href='#SkPoint_offset'>offset()</a>

<a name='SkPoint_Length'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_Length'>Length</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>)
</pre>

Returns the  <a href='undocumented#Euclidean_Distance'>Euclidean distance</a> from origin, computed as:

<a href='undocumented#sqrt()'>sqrt</a>(<a href='#SkPoint_Length_x'>x</a> * <a href='#SkPoint_Length_x'>x</a> + <a href='#SkPoint_Length_y'>y</a> * <a href='#SkPoint_Length_y'>y</a>)

.

### Parameters

<table>  <tr>    <td><a name='SkPoint_Length_x'><code><strong>x</strong></code></a></td>
    <td>component of length</td>
  </tr>
  <tr>    <td><a name='SkPoint_Length_y'><code><strong>y</strong></code></a></td>
    <td>component of length</td>
  </tr>
</table>

### Return Value

straight-line distance to origin

### Example

<div><fiddle-embed name="c98773d8b4509969d78cb8121e4b77f6"></fiddle-embed></div>

### See Also

<a href='#SkPoint_length'>length()</a> <a href='#SkPoint_Distance'>Distance</a> <a href='#SkPoint_setLength'>setLength</a>

<a name='SkPoint_Normalize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_Normalize'>Normalize</a>(<a href='SkPoint_Reference#SkVector'>SkVector</a>* <a href='SkPoint_Reference#SkVector'>vec</a>)
</pre>

Scales (<a href='#SkPoint_Normalize_vec'>vec</a>-><a href='#SkPoint_fX'>fX</a>, <a href='#SkPoint_Normalize_vec'>vec</a>-><a href='#SkPoint_fY'>fY</a>) <a href='#SkPoint_fY'>so</a> <a href='#SkPoint_fY'>that</a> <a href='#SkPoint_length'>length()</a> <a href='#SkPoint_length'>returns</a> <a href='#SkPoint_length'>one</a>, <a href='#SkPoint_length'>while</a> <a href='#SkPoint_length'>preserving</a> <a href='#SkPoint_length'>ratio</a> <a href='#SkPoint_length'>of</a> <a href='#SkPoint_Normalize_vec'>vec</a>-><a href='#SkPoint_fX'>fX</a>
to <a href='#SkPoint_Normalize_vec'>vec</a>-><a href='#SkPoint_fY'>fY</a>, <a href='#SkPoint_fY'>if</a> <a href='#SkPoint_fY'>possible</a>. <a href='#SkPoint_fY'>If</a> <a href='#SkPoint_fY'>original</a> <a href='#SkPoint_fY'>length</a> <a href='#SkPoint_fY'>is</a> <a href='#SkPoint_fY'>nearly</a> <a href='#SkPoint_fY'>zero</a>, <a href='#SkPoint_fY'>sets</a> <a href='#SkPoint_Normalize_vec'>vec</a> <a href='#SkPoint_Normalize_vec'>to</a> (0, 0) <a href='#SkPoint_Normalize_vec'>and</a> <a href='#SkPoint_Normalize_vec'>returns</a>
zero; otherwise, returns length of <a href='#SkPoint_Normalize_vec'>vec</a> <a href='#SkPoint_Normalize_vec'>before</a> <a href='#SkPoint_Normalize_vec'>vec</a> <a href='#SkPoint_Normalize_vec'>is</a> <a href='#SkPoint_Normalize_vec'>scaled</a>.

Returned prior length may be <a href='undocumented#SK_ScalarInfinity'>SK_ScalarInfinity</a> <a href='undocumented#SK_ScalarInfinity'>if</a> <a href='undocumented#SK_ScalarInfinity'>it</a> <a href='undocumented#SK_ScalarInfinity'>can</a> <a href='undocumented#SK_ScalarInfinity'>not</a> <a href='undocumented#SK_ScalarInfinity'>be</a> <a href='undocumented#SK_ScalarInfinity'>represented</a> <a href='undocumented#SK_ScalarInfinity'>by</a> <a href='undocumented#SkScalar'>SkScalar</a>.

Note that <a href='#SkPoint_normalize'>normalize()</a> <a href='#SkPoint_normalize'>is</a> <a href='#SkPoint_normalize'>faster</a> <a href='#SkPoint_normalize'>if</a> <a href='#SkPoint_normalize'>prior</a> <a href='#SkPoint_normalize'>length</a> <a href='#SkPoint_normalize'>is</a> <a href='#SkPoint_normalize'>not</a> <a href='#SkPoint_normalize'>required</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_Normalize_vec'><code><strong>vec</strong></code></a></td>
    <td>normalized to unit length</td>
  </tr>
</table>

### Return Value

original <a href='#SkPoint_Normalize_vec'>vec</a> <a href='#SkPoint_Normalize_vec'>length</a>

### Example

<div><fiddle-embed name="60a08f3ce75374fc815384616d114df7"></fiddle-embed></div>

### See Also

<a href='#SkPoint_normalize'>normalize()</a> <a href='#SkPoint_setLength'>setLength</a> <a href='#SkPoint_Length'>Length</a>

<a name='SkPoint_Distance'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_Distance'>Distance</a>(<a href='#SkPoint_Distance'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>a</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>b</a>)
</pre>

Returns the  <a href='undocumented#Euclidean_Distance'>Euclidean distance</a> between <a href='#SkPoint_Distance_a'>a</a> <a href='#SkPoint_Distance_a'>and</a> <a href='#SkPoint_Distance_b'>b</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_Distance_a'><code><strong>a</strong></code></a></td>
    <td><a href='undocumented#Line'>line</a> <a href='undocumented#Line'>end</a> <a href='SkPoint_Reference#Point'>point</a></td>
  </tr>
  <tr>    <td><a name='SkPoint_Distance_b'><code><strong>b</strong></code></a></td>
    <td><a href='undocumented#Line'>line</a> <a href='undocumented#Line'>end</a> <a href='SkPoint_Reference#Point'>point</a></td>
  </tr>
</table>

### Return Value

straight-line distance from <a href='#SkPoint_Distance_a'>a</a> <a href='#SkPoint_Distance_a'>to</a> <a href='#SkPoint_Distance_b'>b</a>

### Example

<div><fiddle-embed name="9e0a2de2eb94dba4521d733e73f2bda5"></fiddle-embed></div>

### See Also

<a href='#SkPoint_length'>length()</a> <a href='#SkPoint_setLength'>setLength</a>

<a name='SkPoint_DotProduct'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_DotProduct'>DotProduct</a>(<a href='#SkPoint_DotProduct'>const</a> <a href='SkPoint_Reference#SkVector'>SkVector</a>& <a href='SkPoint_Reference#SkVector'>a</a>, <a href='SkPoint_Reference#SkVector'>const</a> <a href='SkPoint_Reference#SkVector'>SkVector</a>& <a href='SkPoint_Reference#SkVector'>b</a>)
</pre>

Returns the dot product of <a href='SkPoint_Reference#Vector'>vector</a> <a href='#SkPoint_DotProduct_a'>a</a> <a href='#SkPoint_DotProduct_a'>and</a> <a href='SkPoint_Reference#Vector'>vector</a> <a href='#SkPoint_DotProduct_b'>b</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_DotProduct_a'><code><strong>a</strong></code></a></td>
    <td>left side of dot product</td>
  </tr>
  <tr>    <td><a name='SkPoint_DotProduct_b'><code><strong>b</strong></code></a></td>
    <td>right side of dot product</td>
  </tr>
</table>

### Return Value

product of input magnitudes and cosine of the angle between them

### Example

<div><fiddle-embed name="496db0131a003162faba7d7f98b30340"></fiddle-embed></div>

### See Also

<a href='#SkPoint_dot'>dot</a> <a href='#SkPoint_CrossProduct'>CrossProduct</a>

<a name='SkPoint_CrossProduct'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPoint_CrossProduct'>CrossProduct</a>(<a href='#SkPoint_CrossProduct'>const</a> <a href='SkPoint_Reference#SkVector'>SkVector</a>& <a href='SkPoint_Reference#SkVector'>a</a>, <a href='SkPoint_Reference#SkVector'>const</a> <a href='SkPoint_Reference#SkVector'>SkVector</a>& <a href='SkPoint_Reference#SkVector'>b</a>)
</pre>

Returns the cross product of <a href='SkPoint_Reference#Vector'>vector</a> <a href='#SkPoint_CrossProduct_a'>a</a> <a href='#SkPoint_CrossProduct_a'>and</a> <a href='SkPoint_Reference#Vector'>vector</a> <a href='#SkPoint_CrossProduct_b'>b</a>.

<a href='#SkPoint_CrossProduct_a'>a</a> <a href='#SkPoint_CrossProduct_a'>and</a> <a href='#SkPoint_CrossProduct_b'>b</a> <a href='#SkPoint_CrossProduct_b'>form</a> <a href='#SkPoint_CrossProduct_b'>three-dimensional</a> <a href='SkPoint_Reference#Vector'>vectors</a> <a href='SkPoint_Reference#Vector'>with</a> <a href='SkPoint_Reference#Vector'>z-axis</a> <a href='SkPoint_Reference#Vector'>value</a> <a href='SkPoint_Reference#Vector'>equal</a> <a href='SkPoint_Reference#Vector'>to</a> <a href='SkPoint_Reference#Vector'>zero</a>. <a href='SkPoint_Reference#Vector'>The</a>
cross product is <a href='#SkPoint_CrossProduct_a'>a</a> <a href='#SkPoint_CrossProduct_a'>three-dimensional</a> <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>with</a> <a href='SkPoint_Reference#Vector'>x-axis</a> <a href='SkPoint_Reference#Vector'>and</a> <a href='SkPoint_Reference#Vector'>y-axis</a> <a href='SkPoint_Reference#Vector'>values</a> <a href='SkPoint_Reference#Vector'>equal</a>
to zero. The cross product z-axis component is returned.

### Parameters

<table>  <tr>    <td><a name='SkPoint_CrossProduct_a'><code><strong>a</strong></code></a></td>
    <td>left side of cross product</td>
  </tr>
  <tr>    <td><a name='SkPoint_CrossProduct_b'><code><strong>b</strong></code></a></td>
    <td>right side of cross product</td>
  </tr>
</table>

### Return Value

area spanned by <a href='SkPoint_Reference#Vector'>vectors</a> <a href='SkPoint_Reference#Vector'>signed</a> <a href='SkPoint_Reference#Vector'>by</a> <a href='SkPoint_Reference#Vector'>angle</a> <a href='SkPoint_Reference#Vector'>direction</a>

### Example

<div><fiddle-embed name="8b8a4cd8a29d22bb9c5e63b70357bd65"></fiddle-embed></div>

### See Also

<a href='#SkPoint_cross'>cross</a> <a href='#SkPoint_DotProduct'>DotProduct</a>

<a name='SkPoint_cross'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>cross</a>(<a href='undocumented#SkScalar'>const</a> <a href='SkPoint_Reference#SkVector'>SkVector</a>& <a href='SkPoint_Reference#SkVector'>vec</a>) <a href='SkPoint_Reference#SkVector'>const</a>
</pre>

Returns the cross product of <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>and</a> <a href='#SkPoint_cross_vec'>vec</a>.

<a href='SkPoint_Reference#Vector'>Vector</a> <a href='SkPoint_Reference#Vector'>and</a> <a href='#SkPoint_cross_vec'>vec</a> <a href='#SkPoint_cross_vec'>form</a> <a href='#SkPoint_cross_vec'>three-dimensional</a> <a href='SkPoint_Reference#Vector'>vectors</a> <a href='SkPoint_Reference#Vector'>with</a> <a href='SkPoint_Reference#Vector'>z-axis</a> <a href='SkPoint_Reference#Vector'>value</a> <a href='SkPoint_Reference#Vector'>equal</a> <a href='SkPoint_Reference#Vector'>to</a> <a href='SkPoint_Reference#Vector'>zero</a>.
The cross product is a three-dimensional <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>with</a> <a href='SkPoint_Reference#Vector'>x-axis</a> <a href='SkPoint_Reference#Vector'>and</a> <a href='SkPoint_Reference#Vector'>y-axis</a> <a href='SkPoint_Reference#Vector'>values</a>
equal to zero. The cross product z-axis component is returned.

### Parameters

<table>  <tr>    <td><a name='SkPoint_cross_vec'><code><strong>vec</strong></code></a></td>
    <td>right side of cross product</td>
  </tr>
</table>

### Return Value

area spanned by <a href='SkPoint_Reference#Vector'>vectors</a> <a href='SkPoint_Reference#Vector'>signed</a> <a href='SkPoint_Reference#Vector'>by</a> <a href='SkPoint_Reference#Vector'>angle</a> <a href='SkPoint_Reference#Vector'>direction</a>

### Example

<div><fiddle-embed name="0bc7b3997357e499817278b78bdfbf1d"></fiddle-embed></div>

### See Also

<a href='#SkPoint_CrossProduct'>CrossProduct</a> <a href='#SkPoint_dot'>dot</a>

<a name='SkPoint_dot'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dot</a>(<a href='undocumented#SkScalar'>const</a> <a href='SkPoint_Reference#SkVector'>SkVector</a>& <a href='SkPoint_Reference#SkVector'>vec</a>) <a href='SkPoint_Reference#SkVector'>const</a>
</pre>

Returns the dot product of <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>and</a> <a href='SkPoint_Reference#Vector'>vector</a> <a href='#SkPoint_dot_vec'>vec</a>.

### Parameters

<table>  <tr>    <td><a name='SkPoint_dot_vec'><code><strong>vec</strong></code></a></td>
    <td>right side of dot product</td>
  </tr>
</table>

### Return Value

product of input magnitudes and cosine of the angle between them

### Example

<div><fiddle-embed name="56d01ccfedd71d3c504b09afa2875d38"></fiddle-embed></div>

### See Also

<a href='#SkPoint_DotProduct'>DotProduct</a> <a href='#SkPoint_cross'>cross</a>

<a name='Vector'></a>

<a name='SkVector'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    typedef <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkVector'>SkVector</a>;
</pre>

<a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='SkPoint_Reference#SkVector'>provides</a> <a href='SkPoint_Reference#SkVector'>an</a> <a href='SkPoint_Reference#SkVector'>alternative</a> <a href='SkPoint_Reference#SkVector'>name</a> <a href='SkPoint_Reference#SkVector'>for</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>. <a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='SkPoint_Reference#SkVector'>and</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>can</a>
<a href='SkPoint_Reference#SkPoint'>be</a> <a href='SkPoint_Reference#SkPoint'>used</a> <a href='SkPoint_Reference#SkPoint'>interchangeably</a> <a href='SkPoint_Reference#SkPoint'>for</a> <a href='SkPoint_Reference#SkPoint'>all</a> <a href='SkPoint_Reference#SkPoint'>purposes</a>.