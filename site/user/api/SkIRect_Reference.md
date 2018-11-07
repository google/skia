SkIRect Reference
===


<a name='SkIRect'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
struct <a href='SkIRect_Reference#SkIRect'>SkIRect</a> {
    <a href='SkIRect_Reference#SkIRect'>int32_t</a> <a href='#SkIRect_fLeft'>fLeft</a>;
    <a href='#SkIRect_fLeft'>int32_t</a> <a href='#SkIRect_fTop'>fTop</a>;
    <a href='#SkIRect_fTop'>int32_t</a> <a href='#SkIRect_fRight'>fRight</a>;
    <a href='#SkIRect_fRight'>int32_t</a> <a href='#SkIRect_fBottom'>fBottom</a>;

    <a href='#SkIRect_fBottom'>static</a> <a href='#SkIRect_fBottom'>constexpr</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkIRect_MakeEmpty'>MakeEmpty</a>();
    <a href='#SkIRect_MakeEmpty'>static</a> <a href='#SkIRect_MakeEmpty'>constexpr</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkIRect_MakeWH'>MakeWH</a>(<a href='#SkIRect_MakeWH'>int32_t</a> <a href='#SkIRect_MakeWH'>w</a>, <a href='#SkIRect_MakeWH'>int32_t</a> <a href='#SkIRect_MakeWH'>h</a>);
    <a href='#SkIRect_MakeWH'>static</a> <a href='#SkIRect_MakeWH'>constexpr</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkIRect_MakeSize'>MakeSize</a>(<a href='#SkIRect_MakeSize'>const</a> <a href='undocumented#SkISize'>SkISize</a>& <a href='undocumented#Size'>size</a>);
    <a href='undocumented#Size'>static</a> <a href='undocumented#Size'>constexpr</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkIRect_MakeLTRB'>MakeLTRB</a>(<a href='#SkIRect_MakeLTRB'>int32_t</a> <a href='#SkIRect_MakeLTRB'>l</a>, <a href='#SkIRect_MakeLTRB'>int32_t</a> <a href='#SkIRect_MakeLTRB'>t</a>,
                                      <a href='#SkIRect_MakeLTRB'>int32_t</a> <a href='#SkIRect_MakeLTRB'>r</a>, <a href='#SkIRect_MakeLTRB'>int32_t</a> <a href='#SkIRect_MakeLTRB'>b</a>);
    <a href='#SkIRect_MakeLTRB'>static</a> <a href='#SkIRect_MakeLTRB'>constexpr</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkIRect_MakeXYWH'>MakeXYWH</a>(<a href='#SkIRect_MakeXYWH'>int32_t</a> <a href='#SkIRect_MakeXYWH'>x</a>, <a href='#SkIRect_MakeXYWH'>int32_t</a> <a href='#SkIRect_MakeXYWH'>y</a>,
                                      <a href='#SkIRect_MakeXYWH'>int32_t</a> <a href='#SkIRect_MakeXYWH'>w</a>, <a href='#SkIRect_MakeXYWH'>int32_t</a> <a href='#SkIRect_MakeXYWH'>h</a>);
    <a href='#SkIRect_MakeXYWH'>int32_t</a> <a href='#SkIRect_left'>left()</a> <a href='#SkIRect_left'>const</a>;
    <a href='#SkIRect_left'>int32_t</a> <a href='#SkIRect_top'>top()</a> <a href='#SkIRect_top'>const</a>;
    <a href='#SkIRect_top'>int32_t</a> <a href='#SkIRect_right'>right()</a> <a href='#SkIRect_right'>const</a>;
    <a href='#SkIRect_right'>int32_t</a> <a href='#SkIRect_bottom'>bottom()</a> <a href='#SkIRect_bottom'>const</a>;
    <a href='#SkIRect_bottom'>int32_t</a> <a href='#SkIRect_x'>x()</a> <a href='#SkIRect_x'>const</a>;
    <a href='#SkIRect_x'>int32_t</a> <a href='#SkIRect_y'>y()</a> <a href='#SkIRect_y'>const</a>;
    <a href='#SkIRect_y'>int32_t</a> <a href='#SkIRect_width'>width()</a> <a href='#SkIRect_width'>const</a>;
    <a href='#SkIRect_width'>int32_t</a> <a href='#SkIRect_height'>height()</a> <a href='#SkIRect_height'>const</a>;
    <a href='undocumented#SkISize'>SkISize</a> <a href='#SkIRect_size'>size()</a> <a href='#SkIRect_size'>const</a>;
    <a href='#SkIRect_size'>int64_t</a> <a href='#SkIRect_width64'>width64</a>() <a href='#SkIRect_width64'>const</a>;
    <a href='#SkIRect_width64'>int64_t</a> <a href='#SkIRect_height64'>height64</a>() <a href='#SkIRect_height64'>const</a>;
    <a href='#SkIRect_height64'>bool</a> <a href='#SkIRect_isEmpty64'>isEmpty64</a>() <a href='#SkIRect_isEmpty64'>const</a>;
    <a href='#SkIRect_isEmpty64'>bool</a> <a href='#SkIRect_isEmpty'>isEmpty</a>() <a href='#SkIRect_isEmpty'>const</a>;
    <a href='#SkIRect_isEmpty'>friend</a> <a href='#SkIRect_isEmpty'>bool</a> <a href='#SkIRect_isEmpty'>operator</a>==(<a href='#SkIRect_isEmpty'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>a</a>, <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>b</a>);
    <a href='SkIRect_Reference#SkIRect'>friend</a> <a href='SkIRect_Reference#SkIRect'>bool</a> <a href='SkIRect_Reference#SkIRect'>operator</a>!=(<a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>a</a>, <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>b</a>);
    <a href='SkIRect_Reference#SkIRect'>void</a> <a href='#SkIRect_setEmpty'>setEmpty</a>();
    <a href='#SkIRect_setEmpty'>void</a> <a href='#SkIRect_setEmpty'>set</a>(<a href='#SkIRect_setEmpty'>int32_t</a> <a href='#SkIRect_setEmpty'>left</a>, <a href='#SkIRect_setEmpty'>int32_t</a> <a href='#SkIRect_setEmpty'>top</a>, <a href='#SkIRect_setEmpty'>int32_t</a> <a href='#SkIRect_setEmpty'>right</a>, <a href='#SkIRect_setEmpty'>int32_t</a> <a href='#SkIRect_setEmpty'>bottom</a>);
    <a href='#SkIRect_setEmpty'>void</a> <a href='#SkIRect_setLTRB'>setLTRB</a>(<a href='#SkIRect_setLTRB'>int32_t</a> <a href='#SkIRect_setLTRB'>left</a>, <a href='#SkIRect_setLTRB'>int32_t</a> <a href='#SkIRect_setLTRB'>top</a>, <a href='#SkIRect_setLTRB'>int32_t</a> <a href='#SkIRect_setLTRB'>right</a>, <a href='#SkIRect_setLTRB'>int32_t</a> <a href='#SkIRect_setLTRB'>bottom</a>);
    <a href='#SkIRect_setLTRB'>void</a> <a href='#SkIRect_setXYWH'>setXYWH</a>(<a href='#SkIRect_setXYWH'>int32_t</a> <a href='#SkIRect_setXYWH'>x</a>, <a href='#SkIRect_setXYWH'>int32_t</a> <a href='#SkIRect_setXYWH'>y</a>, <a href='#SkIRect_setXYWH'>int32_t</a> <a href='#SkIRect_setXYWH'>width</a>, <a href='#SkIRect_setXYWH'>int32_t</a> <a href='#SkIRect_setXYWH'>height</a>);
    <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkIRect_makeOffset'>makeOffset</a>(<a href='#SkIRect_makeOffset'>int32_t</a> <a href='#SkIRect_makeOffset'>dx</a>, <a href='#SkIRect_makeOffset'>int32_t</a> <a href='#SkIRect_makeOffset'>dy</a>) <a href='#SkIRect_makeOffset'>const</a>;
    <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkIRect_makeInset'>makeInset</a>(<a href='#SkIRect_makeInset'>int32_t</a> <a href='#SkIRect_makeInset'>dx</a>, <a href='#SkIRect_makeInset'>int32_t</a> <a href='#SkIRect_makeInset'>dy</a>) <a href='#SkIRect_makeInset'>const</a>;
    <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkIRect_makeOutset'>makeOutset</a>(<a href='#SkIRect_makeOutset'>int32_t</a> <a href='#SkIRect_makeOutset'>dx</a>, <a href='#SkIRect_makeOutset'>int32_t</a> <a href='#SkIRect_makeOutset'>dy</a>) <a href='#SkIRect_makeOutset'>const</a>;
    <a href='#SkIRect_makeOutset'>void</a> <a href='#SkIRect_makeOutset'>offset</a>(<a href='#SkIRect_makeOutset'>int32_t</a> <a href='#SkIRect_makeOutset'>dx</a>, <a href='#SkIRect_makeOutset'>int32_t</a> <a href='#SkIRect_makeOutset'>dy</a>);
    <a href='#SkIRect_makeOutset'>void</a> <a href='#SkIRect_makeOutset'>offset</a>(<a href='#SkIRect_makeOutset'>const</a> <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& <a href='SkIPoint_Reference#SkIPoint'>delta</a>);
    <a href='SkIPoint_Reference#SkIPoint'>void</a> <a href='#SkIRect_offsetTo'>offsetTo</a>(<a href='#SkIRect_offsetTo'>int32_t</a> <a href='#SkIRect_offsetTo'>newX</a>, <a href='#SkIRect_offsetTo'>int32_t</a> <a href='#SkIRect_offsetTo'>newY</a>);
    <a href='#SkIRect_offsetTo'>void</a> <a href='#SkIRect_offsetTo'>inset</a>(<a href='#SkIRect_offsetTo'>int32_t</a> <a href='#SkIRect_offsetTo'>dx</a>, <a href='#SkIRect_offsetTo'>int32_t</a> <a href='#SkIRect_offsetTo'>dy</a>);
    <a href='#SkIRect_offsetTo'>void</a> <a href='#SkIRect_offsetTo'>outset</a>(<a href='#SkIRect_offsetTo'>int32_t</a> <a href='#SkIRect_offsetTo'>dx</a>, <a href='#SkIRect_offsetTo'>int32_t</a> <a href='#SkIRect_offsetTo'>dy</a>);
    <a href='#SkIRect_offsetTo'>void</a> <a href='#SkIRect_adjust'>adjust</a>(<a href='#SkIRect_adjust'>int32_t</a> <a href='#SkIRect_adjust'>dL</a>, <a href='#SkIRect_adjust'>int32_t</a> <a href='#SkIRect_adjust'>dT</a>, <a href='#SkIRect_adjust'>int32_t</a> <a href='#SkIRect_adjust'>dR</a>, <a href='#SkIRect_adjust'>int32_t</a> <a href='#SkIRect_adjust'>dB</a>);
    <a href='#SkIRect_adjust'>bool</a> <a href='#SkIRect_adjust'>contains</a>(<a href='#SkIRect_adjust'>int32_t</a> <a href='#SkIRect_adjust'>x</a>, <a href='#SkIRect_adjust'>int32_t</a> <a href='#SkIRect_adjust'>y</a>) <a href='#SkIRect_adjust'>const</a>;
    <a href='#SkIRect_adjust'>bool</a> <a href='#SkIRect_adjust'>contains</a>(<a href='#SkIRect_adjust'>int32_t</a> <a href='#SkIRect_adjust'>left</a>, <a href='#SkIRect_adjust'>int32_t</a> <a href='#SkIRect_adjust'>top</a>, <a href='#SkIRect_adjust'>int32_t</a> <a href='#SkIRect_adjust'>right</a>, <a href='#SkIRect_adjust'>int32_t</a> <a href='#SkIRect_adjust'>bottom</a>) <a href='#SkIRect_adjust'>const</a>;
    <a href='#SkIRect_adjust'>bool</a> <a href='#SkIRect_adjust'>contains</a>(<a href='#SkIRect_adjust'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>r</a>) <a href='SkIRect_Reference#SkIRect'>const</a>;
    <a href='SkIRect_Reference#SkIRect'>bool</a> <a href='SkIRect_Reference#SkIRect'>contains</a>(<a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>r</a>) <a href='SkRect_Reference#SkRect'>const</a>;
    <a href='SkRect_Reference#SkRect'>bool</a> <a href='#SkIRect_containsNoEmptyCheck'>containsNoEmptyCheck</a>(<a href='#SkIRect_containsNoEmptyCheck'>int32_t</a> <a href='#SkIRect_containsNoEmptyCheck'>left</a>, <a href='#SkIRect_containsNoEmptyCheck'>int32_t</a> <a href='#SkIRect_containsNoEmptyCheck'>top</a>,
                              <a href='#SkIRect_containsNoEmptyCheck'>int32_t</a> <a href='#SkIRect_containsNoEmptyCheck'>right</a>, <a href='#SkIRect_containsNoEmptyCheck'>int32_t</a> <a href='#SkIRect_containsNoEmptyCheck'>bottom</a>) <a href='#SkIRect_containsNoEmptyCheck'>const</a>;
    <a href='#SkIRect_containsNoEmptyCheck'>bool</a> <a href='#SkIRect_containsNoEmptyCheck'>containsNoEmptyCheck</a>(<a href='#SkIRect_containsNoEmptyCheck'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>r</a>) <a href='SkIRect_Reference#SkIRect'>const</a>;
    <a href='SkIRect_Reference#SkIRect'>bool</a> <a href='SkIRect_Reference#SkIRect'>intersect</a>(<a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>r</a>);
    <a href='SkIRect_Reference#SkIRect'>bool</a> <a href='#SkIRect_intersectNoEmptyCheck'>intersectNoEmptyCheck</a>(<a href='#SkIRect_intersectNoEmptyCheck'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>a</a>, <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>b</a>);
    <a href='SkIRect_Reference#SkIRect'>bool</a> <a href='SkIRect_Reference#SkIRect'>intersect</a>(<a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>a</a>, <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>b</a>);
    <a href='SkIRect_Reference#SkIRect'>bool</a> <a href='SkIRect_Reference#SkIRect'>intersect</a>(<a href='SkIRect_Reference#SkIRect'>int32_t</a> <a href='SkIRect_Reference#SkIRect'>left</a>, <a href='SkIRect_Reference#SkIRect'>int32_t</a> <a href='SkIRect_Reference#SkIRect'>top</a>, <a href='SkIRect_Reference#SkIRect'>int32_t</a> <a href='SkIRect_Reference#SkIRect'>right</a>, <a href='SkIRect_Reference#SkIRect'>int32_t</a> <a href='SkIRect_Reference#SkIRect'>bottom</a>);
    <a href='SkIRect_Reference#SkIRect'>static</a> <a href='SkIRect_Reference#SkIRect'>bool</a> <a href='#SkIRect_Intersects'>Intersects</a>(<a href='#SkIRect_Intersects'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>a</a>, <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>b</a>);
    <a href='SkIRect_Reference#SkIRect'>static</a> <a href='SkIRect_Reference#SkIRect'>bool</a> <a href='#SkIRect_IntersectsNoEmptyCheck'>IntersectsNoEmptyCheck</a>(<a href='#SkIRect_IntersectsNoEmptyCheck'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>a</a>, <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>b</a>);
    <a href='SkIRect_Reference#SkIRect'>void</a> <a href='SkIRect_Reference#SkIRect'>join</a>(<a href='SkIRect_Reference#SkIRect'>int32_t</a> <a href='SkIRect_Reference#SkIRect'>left</a>, <a href='SkIRect_Reference#SkIRect'>int32_t</a> <a href='SkIRect_Reference#SkIRect'>top</a>, <a href='SkIRect_Reference#SkIRect'>int32_t</a> <a href='SkIRect_Reference#SkIRect'>right</a>, <a href='SkIRect_Reference#SkIRect'>int32_t</a> <a href='SkIRect_Reference#SkIRect'>bottom</a>);
    <a href='SkIRect_Reference#SkIRect'>void</a> <a href='SkIRect_Reference#SkIRect'>join</a>(<a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>r</a>);
    <a href='SkIRect_Reference#SkIRect'>void</a> <a href='#SkIRect_sort'>sort()</a>;
    <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkIRect_makeSorted'>makeSorted</a>() <a href='#SkIRect_makeSorted'>const</a>;
    <a href='#SkIRect_makeSorted'>static</a> <a href='#SkIRect_makeSorted'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='#SkIRect_EmptyIRect'>EmptyIRect</a>();
};
</pre>

<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>holds</a> <a href='SkIRect_Reference#SkIRect'>four</a> 32-<a href='SkIRect_Reference#SkIRect'>bit</a> <a href='SkIRect_Reference#SkIRect'>integer</a> <a href='SkIRect_Reference#SkIRect'>coordinates</a> <a href='SkIRect_Reference#SkIRect'>describing</a> <a href='SkIRect_Reference#SkIRect'>the</a> <a href='SkIRect_Reference#SkIRect'>upper</a> <a href='SkIRect_Reference#SkIRect'>and</a>
<a href='SkIRect_Reference#SkIRect'>lower</a> <a href='SkIRect_Reference#SkIRect'>bounds</a> <a href='SkIRect_Reference#SkIRect'>of</a> <a href='SkIRect_Reference#SkIRect'>a</a> <a href='SkIRect_Reference#SkIRect'>rectangle</a>. <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>may</a> <a href='SkIRect_Reference#SkIRect'>be</a> <a href='SkIRect_Reference#SkIRect'>created</a> <a href='SkIRect_Reference#SkIRect'>from</a> <a href='SkIRect_Reference#SkIRect'>outer</a> <a href='SkIRect_Reference#SkIRect'>bounds</a> <a href='SkIRect_Reference#SkIRect'>or</a>
<a href='SkIRect_Reference#SkIRect'>from</a> <a href='SkIRect_Reference#SkIRect'>position</a>, <a href='SkIRect_Reference#SkIRect'>width</a>, <a href='SkIRect_Reference#SkIRect'>and</a> <a href='SkIRect_Reference#SkIRect'>height</a>. <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>describes</a> <a href='SkIRect_Reference#SkIRect'>an</a> <a href='SkIRect_Reference#SkIRect'>area</a>; <a href='SkIRect_Reference#SkIRect'>if</a> <a href='SkIRect_Reference#SkIRect'>its</a> <a href='SkIRect_Reference#SkIRect'>right</a>
<a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>less</a> <a href='SkIRect_Reference#SkIRect'>than</a> <a href='SkIRect_Reference#SkIRect'>or</a> <a href='SkIRect_Reference#SkIRect'>equal</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>its</a> <a href='SkIRect_Reference#SkIRect'>left</a>, <a href='SkIRect_Reference#SkIRect'>or</a> <a href='SkIRect_Reference#SkIRect'>if</a> <a href='SkIRect_Reference#SkIRect'>its</a> <a href='SkIRect_Reference#SkIRect'>bottom</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>less</a> <a href='SkIRect_Reference#SkIRect'>than</a> <a href='SkIRect_Reference#SkIRect'>or</a> <a href='SkIRect_Reference#SkIRect'>equal</a> <a href='SkIRect_Reference#SkIRect'>to</a>
<a href='SkIRect_Reference#SkIRect'>its</a> <a href='SkIRect_Reference#SkIRect'>top</a>, <a href='SkIRect_Reference#SkIRect'>it</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>considered</a> <a href='SkIRect_Reference#SkIRect'>empty</a>.<table style='border-collapse: collapse; width: 62.5em'>

  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Member</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkIRect_fLeft'><code>fLeft</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May contain any value. The smaller of the horizontal values when sorted.
When equal to or greater than <a href='#SkIRect_fRight'>fRight</a>, <a href='SkIRect_Reference#IRect'>IRect</a> <a href='SkIRect_Reference#IRect'>is</a> <a href='SkIRect_Reference#IRect'>empty</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkIRect_fTop'><code>fTop</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May contain any value. The smaller of the horizontal values when sorted.
When equal to or greater than <a href='#SkIRect_fBottom'>fBottom</a>, <a href='SkIRect_Reference#IRect'>IRect</a> <a href='SkIRect_Reference#IRect'>is</a> <a href='SkIRect_Reference#IRect'>empty</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkIRect_fRight'><code>fRight</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May contain any value. The larger of the vertical values when sorted.
When equal to or less than <a href='#SkIRect_fLeft'>fLeft</a>, <a href='SkIRect_Reference#IRect'>IRect</a> <a href='SkIRect_Reference#IRect'>is</a> <a href='SkIRect_Reference#IRect'>empty</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>int32_t</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkIRect_fBottom'><code>fBottom</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May contain any value. The larger of the vertical values when sorted.
When equal to or less than <a href='#SkIRect_fTop'>fTop</a>, <a href='SkIRect_Reference#IRect'>IRect</a> <a href='SkIRect_Reference#IRect'>is</a> <a href='SkIRect_Reference#IRect'>empty</a>.
</td>
  </tr>
</table>

<a name='SkIRect_MakeEmpty'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkIRect_MakeEmpty'>MakeEmpty</a>()
</pre>

Returns constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>set</a> <a href='SkIRect_Reference#SkIRect'>to</a> (0, 0, 0, 0).
Many other rectangles are empty; if left is equal to or greater than right,
or if top is equal to or greater than bottom. Setting all members to zero
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

<a href='#SkIRect_EmptyIRect'>EmptyIRect</a> <a href='#SkIRect_isEmpty'>isEmpty</a> <a href='#SkIRect_setEmpty'>setEmpty</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_MakeEmpty'>MakeEmpty</a>

<a name='SkIRect_MakeWH'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkIRect_MakeWH'>MakeWH</a>(<a href='#SkIRect_MakeWH'>int32_t</a> <a href='#SkIRect_MakeWH'>w</a>, <a href='#SkIRect_MakeWH'>int32_t</a> <a href='#SkIRect_MakeWH'>h</a>)
</pre>

Returns constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>set</a> <a href='SkIRect_Reference#SkIRect'>to</a> (0, 0, <a href='#SkIRect_MakeWH_w'>w</a>, <a href='#SkIRect_MakeWH_h'>h</a>). <a href='#SkIRect_MakeWH_h'>Does</a> <a href='#SkIRect_MakeWH_h'>not</a> <a href='#SkIRect_MakeWH_h'>validate</a> <a href='#SkIRect_MakeWH_h'>input</a>; <a href='#SkIRect_MakeWH_w'>w</a> <a href='#SkIRect_MakeWH_w'>or</a> <a href='#SkIRect_MakeWH_h'>h</a>
may be negative.

### Parameters

<table>  <tr>    <td><a name='SkIRect_MakeWH_w'><code><strong>w</strong></code></a></td>
    <td>width of constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_MakeWH_h'><code><strong>h</strong></code></a></td>
    <td>height of constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a></td>
  </tr>
</table>

### Return Value

bounds (0, 0, <a href='#SkIRect_MakeWH_w'>w</a>, <a href='#SkIRect_MakeWH_h'>h</a>)

### Example

<div><fiddle-embed name="e36827a1a6ae2b1c26e7a8a08f325a07">

#### Example Output

~~~~
all equal
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_MakeSize'>MakeSize</a> <a href='#SkIRect_MakeXYWH'>MakeXYWH</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_MakeWH'>MakeWH</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_MakeIWH'>MakeIWH</a>

<a name='SkIRect_MakeSize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkIRect_MakeSize'>MakeSize</a>(<a href='#SkIRect_MakeSize'>const</a> <a href='undocumented#SkISize'>SkISize</a>& <a href='undocumented#Size'>size</a>)
</pre>

Returns constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>set</a> <a href='SkIRect_Reference#SkIRect'>to</a> (0, 0, <a href='#SkIRect_MakeSize_size'>size</a>.<a href='#SkISize_width'>width()</a>, <a href='#SkIRect_MakeSize_size'>size</a>.<a href='#SkISize_height'>height()</a>).
Does not validate input; <a href='#SkIRect_MakeSize_size'>size</a>.<a href='#SkISize_width'>width()</a> <a href='#SkISize_width'>or</a> <a href='#SkIRect_MakeSize_size'>size</a>.<a href='#SkISize_height'>height()</a> <a href='#SkISize_height'>may</a> <a href='#SkISize_height'>be</a> <a href='#SkISize_height'>negative</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_MakeSize_size'><code><strong>size</strong></code></a></td>
    <td>values for <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>width</a> <a href='SkIRect_Reference#SkIRect'>and</a> <a href='SkIRect_Reference#SkIRect'>height</a></td>
  </tr>
</table>

### Return Value

bounds (0, 0, <a href='#SkIRect_MakeSize_size'>size</a>.<a href='#SkISize_width'>width()</a>, <a href='#SkIRect_MakeSize_size'>size</a>.<a href='#SkISize_height'>height()</a>)

### Example

<div><fiddle-embed name="c6586ff8d24869c780169b0d19c75df6">

#### Example Output

~~~~
round width: 26  height: 36
floor width: 25  height: 35
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_MakeWH'>MakeWH</a> <a href='#SkIRect_MakeXYWH'>MakeXYWH</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_Make'>Make</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_MakeIWH'>MakeIWH</a>

<a name='SkIRect_MakeLTRB'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkIRect_MakeLTRB'>MakeLTRB</a>(<a href='#SkIRect_MakeLTRB'>int32_t</a> <a href='#SkIRect_MakeLTRB'>l</a>, <a href='#SkIRect_MakeLTRB'>int32_t</a> <a href='#SkIRect_MakeLTRB'>t</a>, <a href='#SkIRect_MakeLTRB'>int32_t</a> <a href='#SkIRect_MakeLTRB'>r</a>, <a href='#SkIRect_MakeLTRB'>int32_t</a> <a href='#SkIRect_MakeLTRB'>b</a>)
</pre>

Returns constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>set</a> <a href='SkIRect_Reference#SkIRect'>to</a> (<a href='#SkIRect_MakeLTRB_l'>l</a>, <a href='#SkIRect_MakeLTRB_t'>t</a>, <a href='#SkIRect_MakeLTRB_r'>r</a>, <a href='#SkIRect_MakeLTRB_b'>b</a>). <a href='#SkIRect_MakeLTRB_b'>Does</a> <a href='#SkIRect_MakeLTRB_b'>not</a> <a href='#SkIRect_MakeLTRB_b'>sort</a> <a href='#SkIRect_MakeLTRB_b'>input</a>; <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>may</a>
result in <a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_fLeft'>greater</a> <a href='#SkIRect_fLeft'>than</a> <a href='#SkIRect_fRight'>fRight</a>, <a href='#SkIRect_fRight'>or</a> <a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_fTop'>greater</a> <a href='#SkIRect_fTop'>than</a> <a href='#SkIRect_fBottom'>fBottom</a>.

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

bounds (<a href='#SkIRect_MakeLTRB_l'>l</a>, <a href='#SkIRect_MakeLTRB_t'>t</a>, <a href='#SkIRect_MakeLTRB_r'>r</a>, <a href='#SkIRect_MakeLTRB_b'>b</a>)

### Example

<div><fiddle-embed name="ec1473b700c594f2df9749a12a06b89b">

#### Example Output

~~~~
rect: 5, 35, 15, 25  isEmpty: true
rect: 5, 25, 15, 35  isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_MakeXYWH'>MakeXYWH</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_MakeLTRB'>MakeLTRB</a>

<a name='SkIRect_MakeXYWH'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkIRect_MakeXYWH'>MakeXYWH</a>(<a href='#SkIRect_MakeXYWH'>int32_t</a> <a href='#SkIRect_MakeXYWH'>x</a>, <a href='#SkIRect_MakeXYWH'>int32_t</a> <a href='#SkIRect_MakeXYWH'>y</a>, <a href='#SkIRect_MakeXYWH'>int32_t</a> <a href='#SkIRect_MakeXYWH'>w</a>, <a href='#SkIRect_MakeXYWH'>int32_t</a> <a href='#SkIRect_MakeXYWH'>h</a>)
</pre>

Returns constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>set</a> <a href='SkIRect_Reference#SkIRect'>to</a>: (<a href='#SkIRect_MakeXYWH_x'>x</a>, <a href='#SkIRect_MakeXYWH_y'>y</a>, <a href='#SkIRect_MakeXYWH_x'>x</a> + <a href='#SkIRect_MakeXYWH_w'>w</a>, <a href='#SkIRect_MakeXYWH_y'>y</a> + <a href='#SkIRect_MakeXYWH_h'>h</a>).
Does not validate input; <a href='#SkIRect_MakeXYWH_w'>w</a> <a href='#SkIRect_MakeXYWH_w'>or</a> <a href='#SkIRect_MakeXYWH_h'>h</a> <a href='#SkIRect_MakeXYWH_h'>may</a> <a href='#SkIRect_MakeXYWH_h'>be</a> <a href='#SkIRect_MakeXYWH_h'>negative</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_MakeXYWH_x'><code><strong>x</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_MakeXYWH_y'><code><strong>y</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fTop'>fTop</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_MakeXYWH_w'><code><strong>w</strong></code></a></td>
    <td>added to <a href='#SkIRect_MakeXYWH_x'>x</a> <a href='#SkIRect_MakeXYWH_x'>and</a> <a href='#SkIRect_MakeXYWH_x'>stored</a> <a href='#SkIRect_MakeXYWH_x'>in</a> <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_MakeXYWH_h'><code><strong>h</strong></code></a></td>
    <td>added to <a href='#SkIRect_MakeXYWH_y'>y</a> <a href='#SkIRect_MakeXYWH_y'>and</a> <a href='#SkIRect_MakeXYWH_y'>stored</a> <a href='#SkIRect_MakeXYWH_y'>in</a> <a href='#SkIRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

bounds at (<a href='#SkIRect_MakeXYWH_x'>x</a>, <a href='#SkIRect_MakeXYWH_y'>y</a>) <a href='#SkIRect_MakeXYWH_y'>with</a> <a href='#SkIRect_MakeXYWH_y'>width</a> <a href='#SkIRect_MakeXYWH_w'>w</a> <a href='#SkIRect_MakeXYWH_w'>and</a> <a href='#SkIRect_MakeXYWH_w'>height</a> <a href='#SkIRect_MakeXYWH_h'>h</a>

### Example

<div><fiddle-embed name="598ee14350bd1d961cae6b36fa3df17e">

#### Example Output

~~~~
rect: 5, 35, -10, 60  isEmpty: true
rect: -10, 35, 5, 60  isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_MakeLTRB'>MakeLTRB</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_MakeXYWH'>MakeXYWH</a>

<a name='Property'></a>

<a name='SkIRect_left'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIRect_left'>left()</a> <a href='#SkIRect_left'>const</a>
</pre>

Returns left edge of <a href='SkIRect_Reference#SkIRect'>SkIRect</a>, <a href='SkIRect_Reference#SkIRect'>if</a> <a href='SkIRect_Reference#SkIRect'>sorted</a>.
Call <a href='#SkIRect_sort'>sort()</a> <a href='#SkIRect_sort'>to</a> <a href='#SkIRect_sort'>reverse</a> <a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_fLeft'>and</a> <a href='#SkIRect_fRight'>fRight</a> <a href='#SkIRect_fRight'>if</a> <a href='#SkIRect_fRight'>needed</a>.

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

<a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_x'>x()</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_left'>left()</a>

<a name='SkIRect_top'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIRect_top'>top()</a> <a href='#SkIRect_top'>const</a>
</pre>

Returns top edge of <a href='SkIRect_Reference#SkIRect'>SkIRect</a>, <a href='SkIRect_Reference#SkIRect'>if</a> <a href='SkIRect_Reference#SkIRect'>sorted</a>. <a href='SkIRect_Reference#SkIRect'>Call</a> <a href='#SkIRect_isEmpty'>isEmpty</a>() <a href='#SkIRect_isEmpty'>to</a> <a href='#SkIRect_isEmpty'>see</a> <a href='#SkIRect_isEmpty'>if</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>may</a> <a href='SkIRect_Reference#SkIRect'>be</a> <a href='SkIRect_Reference#SkIRect'>invalid</a>,
and <a href='#SkIRect_sort'>sort()</a> <a href='#SkIRect_sort'>to</a> <a href='#SkIRect_sort'>reverse</a> <a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_fTop'>and</a> <a href='#SkIRect_fBottom'>fBottom</a> <a href='#SkIRect_fBottom'>if</a> <a href='#SkIRect_fBottom'>needed</a>.

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

<a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_y'>y()</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_top'>top()</a>

<a name='SkIRect_right'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIRect_right'>right()</a> <a href='#SkIRect_right'>const</a>
</pre>

Returns right edge of <a href='SkIRect_Reference#SkIRect'>SkIRect</a>, <a href='SkIRect_Reference#SkIRect'>if</a> <a href='SkIRect_Reference#SkIRect'>sorted</a>.
Call <a href='#SkIRect_sort'>sort()</a> <a href='#SkIRect_sort'>to</a> <a href='#SkIRect_sort'>reverse</a> <a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_fLeft'>and</a> <a href='#SkIRect_fRight'>fRight</a> <a href='#SkIRect_fRight'>if</a> <a href='#SkIRect_fRight'>needed</a>.

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

<a href='#SkIRect_fRight'>fRight</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_right'>right()</a>

<a name='SkIRect_bottom'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIRect_bottom'>bottom()</a> <a href='#SkIRect_bottom'>const</a>
</pre>

Returns bottom edge of <a href='SkIRect_Reference#SkIRect'>SkIRect</a>, <a href='SkIRect_Reference#SkIRect'>if</a> <a href='SkIRect_Reference#SkIRect'>sorted</a>. <a href='SkIRect_Reference#SkIRect'>Call</a> <a href='#SkIRect_isEmpty'>isEmpty</a>() <a href='#SkIRect_isEmpty'>to</a> <a href='#SkIRect_isEmpty'>see</a> <a href='#SkIRect_isEmpty'>if</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>may</a> <a href='SkIRect_Reference#SkIRect'>be</a> <a href='SkIRect_Reference#SkIRect'>invalid</a>,
and <a href='#SkIRect_sort'>sort()</a> <a href='#SkIRect_sort'>to</a> <a href='#SkIRect_sort'>reverse</a> <a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_fTop'>and</a> <a href='#SkIRect_fBottom'>fBottom</a> <a href='#SkIRect_fBottom'>if</a> <a href='#SkIRect_fBottom'>needed</a>.

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

<a href='#SkIRect_fBottom'>fBottom</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_bottom'>bottom()</a>

<a name='SkIRect_x'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIRect_x'>x()</a> <a href='#SkIRect_x'>const</a>
</pre>

Returns left edge of <a href='SkIRect_Reference#SkIRect'>SkIRect</a>, <a href='SkIRect_Reference#SkIRect'>if</a> <a href='SkIRect_Reference#SkIRect'>sorted</a>. <a href='SkIRect_Reference#SkIRect'>Call</a> <a href='#SkIRect_isEmpty'>isEmpty</a>() <a href='#SkIRect_isEmpty'>to</a> <a href='#SkIRect_isEmpty'>see</a> <a href='#SkIRect_isEmpty'>if</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>may</a> <a href='SkIRect_Reference#SkIRect'>be</a> <a href='SkIRect_Reference#SkIRect'>invalid</a>,
and <a href='#SkIRect_sort'>sort()</a> <a href='#SkIRect_sort'>to</a> <a href='#SkIRect_sort'>reverse</a> <a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_fLeft'>and</a> <a href='#SkIRect_fRight'>fRight</a> <a href='#SkIRect_fRight'>if</a> <a href='#SkIRect_fRight'>needed</a>.

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

<a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_left'>left()</a> <a href='#SkIRect_y'>y()</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_x'>x()</a>

<a name='SkIRect_y'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIRect_y'>y()</a> <a href='#SkIRect_y'>const</a>
</pre>

Returns top edge of <a href='SkIRect_Reference#SkIRect'>SkIRect</a>, <a href='SkIRect_Reference#SkIRect'>if</a> <a href='SkIRect_Reference#SkIRect'>sorted</a>. <a href='SkIRect_Reference#SkIRect'>Call</a> <a href='#SkIRect_isEmpty'>isEmpty</a>() <a href='#SkIRect_isEmpty'>to</a> <a href='#SkIRect_isEmpty'>see</a> <a href='#SkIRect_isEmpty'>if</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>may</a> <a href='SkIRect_Reference#SkIRect'>be</a> <a href='SkIRect_Reference#SkIRect'>invalid</a>,
and <a href='#SkIRect_sort'>sort()</a> <a href='#SkIRect_sort'>to</a> <a href='#SkIRect_sort'>reverse</a> <a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_fTop'>and</a> <a href='#SkIRect_fBottom'>fBottom</a> <a href='#SkIRect_fBottom'>if</a> <a href='#SkIRect_fBottom'>needed</a>.

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

<a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_top'>top()</a> <a href='#SkIRect_x'>x()</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_y'>y()</a>

<a name='SkIRect_width'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIRect_width'>width()</a> <a href='#SkIRect_width'>const</a>
</pre>

Returns span on the x-axis. This does not check if <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>sorted</a>, <a href='SkIRect_Reference#SkIRect'>or</a> <a href='SkIRect_Reference#SkIRect'>if</a>
result fits in 32-bit signed integer; result may be negative.

### Return Value

<a href='#SkIRect_fRight'>fRight</a> <a href='#SkIRect_fRight'>minus</a> <a href='#SkIRect_fLeft'>fLeft</a>

### Example

<div><fiddle-embed name="4acfbe051805940210c8916a94794142">

#### Example Output

~~~~
unsorted width: -5
large width: -5
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_height'>height()</a> <a href='#SkIRect_width64'>width64</a>() <a href='#SkIRect_height64'>height64</a>() <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_width'>width()</a>

<a name='SkIRect_width64'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int64_t <a href='#SkIRect_width64'>width64</a>() <a href='#SkIRect_width64'>const</a>
</pre>

Returns span on the x-axis. This does not check if <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>sorted</a>, <a href='SkIRect_Reference#SkIRect'>so</a> <a href='SkIRect_Reference#SkIRect'>the</a>
result may be negative. This is safer than calling <a href='#SkIRect_width'>width()</a> <a href='#SkIRect_width'>since</a> <a href='#SkIRect_width'>width()</a> <a href='#SkIRect_width'>might</a>
overflow in its calculation.

### Return Value

<a href='#SkIRect_fRight'>fRight</a> <a href='#SkIRect_fRight'>minus</a> <a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_fLeft'>cast</a> <a href='#SkIRect_fLeft'>to</a> <a href='#SkIRect_fLeft'>int64_t</a>

### Example

<div><fiddle-embed name="63977f97999bbd6eecfdcc7575d75492">

#### Example Output

~~~~
width: -5 width64: 4294967291
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_width'>width()</a> <a href='#SkIRect_height'>height()</a> <a href='#SkIRect_height64'>height64</a>() <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_width'>width()</a>

<a name='SkIRect_height'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int32_t <a href='#SkIRect_height'>height()</a> <a href='#SkIRect_height'>const</a>
</pre>

Returns span on the y-axis. This does not check if <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>sorted</a>, <a href='SkIRect_Reference#SkIRect'>or</a> <a href='SkIRect_Reference#SkIRect'>if</a>
result fits in 32-bit signed integer; result may be negative.

### Return Value

<a href='#SkIRect_fBottom'>fBottom</a> <a href='#SkIRect_fBottom'>minus</a> <a href='#SkIRect_fTop'>fTop</a>

### Example

<div><fiddle-embed name="0175bae87fafcd9433ae661574695586">

#### Example Output

~~~~
unsorted height: -5
large height: -5
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_width'>width()</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_height'>height()</a>

<a name='SkIRect_height64'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int64_t <a href='#SkIRect_height64'>height64</a>() <a href='#SkIRect_height64'>const</a>
</pre>

Returns span on the y-axis. This does not check if <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>sorted</a>, <a href='SkIRect_Reference#SkIRect'>so</a> <a href='SkIRect_Reference#SkIRect'>the</a>
result may be negative. This is safer than calling <a href='#SkIRect_height'>height()</a> <a href='#SkIRect_height'>since</a> <a href='#SkIRect_height'>height()</a> <a href='#SkIRect_height'>might</a>
overflow in its calculation.

### Return Value

<a href='#SkIRect_fBottom'>fBottom</a> <a href='#SkIRect_fBottom'>minus</a> <a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_fTop'>cast</a> <a href='#SkIRect_fTop'>to</a> <a href='#SkIRect_fTop'>int64_t</a>

### Example

<div><fiddle-embed name="02dd98716e54bbd8c2f0ff23b7ef98cf">

#### Example Output

~~~~
height: -5 height64: 4294967291
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_width'>width()</a> <a href='#SkIRect_height'>height()</a> <a href='#SkIRect_width64'>width64</a>() <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_height'>height()</a>

<a name='SkIRect_size'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkISize'>SkISize</a> <a href='#SkIRect_size'>size()</a> <a href='#SkIRect_size'>const</a>
</pre>

Returns spans on the x-axis and y-axis. This does not check if <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>sorted</a>,
or if result fits in 32-bit signed integer; result may be negative.

### Return Value

<a href='undocumented#SkISize'>SkISize</a> (<a href='undocumented#SkISize'>width</a>, <a href='undocumented#SkISize'>height</a>)

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

<a href='#SkIRect_height'>height()</a> <a href='#SkIRect_width'>width()</a> <a href='#SkIRect_MakeSize'>MakeSize</a>

<a name='SkIRect_isEmpty'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_isEmpty'>isEmpty</a>() <a href='#SkIRect_isEmpty'>const</a>
</pre>

Returns true if <a href='#SkIRect_width'>width()</a> <a href='#SkIRect_width'>or</a> <a href='#SkIRect_height'>height()</a> <a href='#SkIRect_height'>are</a> <a href='#SkIRect_height'>zero</a> <a href='#SkIRect_height'>or</a> <a href='#SkIRect_height'>negative</a>.

### Return Value

true if <a href='#SkIRect_width'>width()</a> <a href='#SkIRect_width'>or</a> <a href='#SkIRect_height'>height()</a> <a href='#SkIRect_height'>are</a> <a href='#SkIRect_height'>zero</a> <a href='#SkIRect_height'>or</a> <a href='#SkIRect_height'>negative</a>

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

<a href='#SkIRect_EmptyIRect'>EmptyIRect</a> <a href='#SkIRect_MakeEmpty'>MakeEmpty</a> <a href='#SkIRect_sort'>sort</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_isEmpty'>isEmpty</a>

<a name='SkIRect_isEmpty64'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_isEmpty64'>isEmpty64</a>() <a href='#SkIRect_isEmpty64'>const</a>
</pre>

Returns true if <a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_fLeft'>is</a> <a href='#SkIRect_fLeft'>equal</a> <a href='#SkIRect_fLeft'>to</a> <a href='#SkIRect_fLeft'>or</a> <a href='#SkIRect_fLeft'>greater</a> <a href='#SkIRect_fLeft'>than</a> <a href='#SkIRect_fRight'>fRight</a>, <a href='#SkIRect_fRight'>or</a> <a href='#SkIRect_fRight'>if</a> <a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_fTop'>is</a> <a href='#SkIRect_fTop'>equal</a>
to or greater than <a href='#SkIRect_fBottom'>fBottom</a>. <a href='#SkIRect_fBottom'>Call</a> <a href='#SkIRect_sort'>sort()</a> <a href='#SkIRect_sort'>to</a> <a href='#SkIRect_sort'>reverse</a> <a href='#SkIRect_sort'>rectangles</a> <a href='#SkIRect_sort'>with</a> <a href='#SkIRect_sort'>negative</a>
<a href='#SkIRect_width64'>width64</a>() <a href='#SkIRect_width64'>or</a> <a href='#SkIRect_height64'>height64</a>().

### Return Value

true if <a href='#SkIRect_width64'>width64</a>() <a href='#SkIRect_width64'>or</a> <a href='#SkIRect_height64'>height64</a>() <a href='#SkIRect_height64'>are</a> <a href='#SkIRect_height64'>zero</a> <a href='#SkIRect_height64'>or</a> <a href='#SkIRect_height64'>negative</a>

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

<a href='#SkIRect_EmptyIRect'>EmptyIRect</a> <a href='#SkIRect_MakeEmpty'>MakeEmpty</a> <a href='#SkIRect_sort'>sort</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_isEmpty'>isEmpty</a>

<a name='Operators'></a>

<a name='SkIRect_equal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator==(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>a</a>, <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>b</a>)
</pre>

Returns true if all members in <a href='#SkIRect_equal_operator_a'>a</a>: <a href='#SkIRect_fLeft'>fLeft</a>, <a href='#SkIRect_fTop'>fTop</a>, <a href='#SkIRect_fRight'>fRight</a>, <a href='#SkIRect_fRight'>and</a> <a href='#SkIRect_fBottom'>fBottom</a>; <a href='#SkIRect_fBottom'>are</a>
identical to corresponding members in <a href='#SkIRect_equal_operator_b'>b</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_equal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>compare</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_equal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>compare</a></td>
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

<a name='SkIRect_notequal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator!=(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>a</a>, <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>b</a>)
</pre>

Returns true if any member in <a href='#SkIRect_notequal_operator_a'>a</a>: <a href='#SkIRect_fLeft'>fLeft</a>, <a href='#SkIRect_fTop'>fTop</a>, <a href='#SkIRect_fRight'>fRight</a>, <a href='#SkIRect_fRight'>and</a> <a href='#SkIRect_fBottom'>fBottom</a>; <a href='#SkIRect_fBottom'>is</a> <a href='#SkIRect_fBottom'>not</a>
identical to the corresponding member in <a href='#SkIRect_notequal_operator_b'>b</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_notequal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>compare</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_notequal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>compare</a></td>
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

<a name='Set'></a>

<a name='SkIRect_setEmpty'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_setEmpty'>setEmpty</a>()
</pre>

Sets <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> (0, 0, 0, 0).

Many other rectangles are empty; if left is equal to or greater than right,
or if top is equal to or greater than bottom. Setting all members to zero
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

<a href='#SkIRect_MakeEmpty'>MakeEmpty</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_setEmpty'>setEmpty</a>

<a name='SkIRect_set'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void set(int32_t left, int32_t top, int32_t right, int32_t bottom)
</pre>

Sets <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> (<a href='#SkIRect_set_left'>left</a>, <a href='#SkIRect_set_top'>top</a>, <a href='#SkIRect_set_right'>right</a>, <a href='#SkIRect_set_bottom'>bottom</a>).
<a href='#SkIRect_set_left'>left</a> <a href='#SkIRect_set_left'>and</a> <a href='#SkIRect_set_right'>right</a> <a href='#SkIRect_set_right'>are</a> <a href='#SkIRect_set_right'>not</a> <a href='#SkIRect_set_right'>sorted</a>; <a href='#SkIRect_set_left'>left</a> <a href='#SkIRect_set_left'>is</a> <a href='#SkIRect_set_left'>not</a> <a href='#SkIRect_set_left'>necessarily</a> <a href='#SkIRect_set_left'>less</a> <a href='#SkIRect_set_left'>than</a> <a href='#SkIRect_set_right'>right</a>.
<a href='#SkIRect_set_top'>top</a> <a href='#SkIRect_set_top'>and</a> <a href='#SkIRect_set_bottom'>bottom</a> <a href='#SkIRect_set_bottom'>are</a> <a href='#SkIRect_set_bottom'>not</a> <a href='#SkIRect_set_bottom'>sorted</a>; <a href='#SkIRect_set_top'>top</a> <a href='#SkIRect_set_top'>is</a> <a href='#SkIRect_set_top'>not</a> <a href='#SkIRect_set_top'>necessarily</a> <a href='#SkIRect_set_top'>less</a> <a href='#SkIRect_set_top'>than</a> <a href='#SkIRect_set_bottom'>bottom</a>.

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

<a href='#SkIRect_setLTRB'>setLTRB</a> <a href='#SkIRect_setXYWH'>setXYWH</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_set'>set</a>

<a name='SkIRect_setLTRB'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_setLTRB'>setLTRB</a>(<a href='#SkIRect_setLTRB'>int32_t</a> <a href='#SkIRect_setLTRB'>left</a>, <a href='#SkIRect_setLTRB'>int32_t</a> <a href='#SkIRect_setLTRB'>top</a>, <a href='#SkIRect_setLTRB'>int32_t</a> <a href='#SkIRect_setLTRB'>right</a>, <a href='#SkIRect_setLTRB'>int32_t</a> <a href='#SkIRect_setLTRB'>bottom</a>)
</pre>

Sets <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> (<a href='#SkIRect_setLTRB_left'>left</a>, <a href='#SkIRect_setLTRB_top'>top</a>, <a href='#SkIRect_setLTRB_right'>right</a>, <a href='#SkIRect_setLTRB_bottom'>bottom</a>).
<a href='#SkIRect_setLTRB_left'>left</a> <a href='#SkIRect_setLTRB_left'>and</a> <a href='#SkIRect_setLTRB_right'>right</a> <a href='#SkIRect_setLTRB_right'>are</a> <a href='#SkIRect_setLTRB_right'>not</a> <a href='#SkIRect_setLTRB_right'>sorted</a>; <a href='#SkIRect_setLTRB_left'>left</a> <a href='#SkIRect_setLTRB_left'>is</a> <a href='#SkIRect_setLTRB_left'>not</a> <a href='#SkIRect_setLTRB_left'>necessarily</a> <a href='#SkIRect_setLTRB_left'>less</a> <a href='#SkIRect_setLTRB_left'>than</a> <a href='#SkIRect_setLTRB_right'>right</a>.
<a href='#SkIRect_setLTRB_top'>top</a> <a href='#SkIRect_setLTRB_top'>and</a> <a href='#SkIRect_setLTRB_bottom'>bottom</a> <a href='#SkIRect_setLTRB_bottom'>are</a> <a href='#SkIRect_setLTRB_bottom'>not</a> <a href='#SkIRect_setLTRB_bottom'>sorted</a>; <a href='#SkIRect_setLTRB_top'>top</a> <a href='#SkIRect_setLTRB_top'>is</a> <a href='#SkIRect_setLTRB_top'>not</a> <a href='#SkIRect_setLTRB_top'>necessarily</a> <a href='#SkIRect_setLTRB_top'>less</a> <a href='#SkIRect_setLTRB_top'>than</a> <a href='#SkIRect_setLTRB_bottom'>bottom</a>.

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

<a href='#SkIRect_set'>set</a> <a href='#SkIRect_setXYWH'>setXYWH</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_setLTRB'>setLTRB</a>

<a name='SkIRect_setXYWH'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_setXYWH'>setXYWH</a>(<a href='#SkIRect_setXYWH'>int32_t</a> <a href='#SkIRect_setXYWH'>x</a>, <a href='#SkIRect_setXYWH'>int32_t</a> <a href='#SkIRect_setXYWH'>y</a>, <a href='#SkIRect_setXYWH'>int32_t</a> <a href='#SkIRect_setXYWH'>width</a>, <a href='#SkIRect_setXYWH'>int32_t</a> <a href='#SkIRect_setXYWH'>height</a>)
</pre>

Sets <a href='SkIRect_Reference#IRect'>IRect</a> <a href='SkIRect_Reference#IRect'>to</a>: <code>(<a href='#SkIRect_setXYWH_x'>x</a>, <a href='#SkIRect_setXYWH_y'>y</a>, <a href='#SkIRect_setXYWH_x'>x</a> + <a href='#SkIRect_setXYWH_width'>width</a>, <a href='#SkIRect_setXYWH_y'>y</a> + <a href='#SkIRect_setXYWH_height'>height</a>)</code>.
Does not validate input; <a href='#SkIRect_setXYWH_width'>width</a> <a href='#SkIRect_setXYWH_width'>or</a> <a href='#SkIRect_setXYWH_height'>height</a> <a href='#SkIRect_setXYWH_height'>may</a> <a href='#SkIRect_setXYWH_height'>be</a> <a href='#SkIRect_setXYWH_height'>negative</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_setXYWH_x'><code><strong>x</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_setXYWH_y'><code><strong>y</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fTop'>fTop</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_setXYWH_width'><code><strong>width</strong></code></a></td>
    <td>added to <a href='#SkIRect_setXYWH_x'>x</a> <a href='#SkIRect_setXYWH_x'>and</a> <a href='#SkIRect_setXYWH_x'>stored</a> <a href='#SkIRect_setXYWH_x'>in</a> <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_setXYWH_height'><code><strong>height</strong></code></a></td>
    <td>added to <a href='#SkIRect_setXYWH_y'>y</a> <a href='#SkIRect_setXYWH_y'>and</a> <a href='#SkIRect_setXYWH_y'>stored</a> <a href='#SkIRect_setXYWH_y'>in</a> <a href='#SkIRect_fBottom'>fBottom</a></td>
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

<a href='#SkIRect_MakeXYWH'>MakeXYWH</a> <a href='#SkIRect_setLTRB'>setLTRB</a> <a href='#SkIRect_set'>set</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_setXYWH'>setXYWH</a>

<a name='Inset_Outset_Offset'></a>

<a name='SkIRect_makeOffset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkIRect_makeOffset'>makeOffset</a>(<a href='#SkIRect_makeOffset'>int32_t</a> <a href='#SkIRect_makeOffset'>dx</a>, <a href='#SkIRect_makeOffset'>int32_t</a> <a href='#SkIRect_makeOffset'>dy</a>) <a href='#SkIRect_makeOffset'>const</a>
</pre>

Returns <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>offset</a> <a href='SkIRect_Reference#SkIRect'>by</a> (<a href='#SkIRect_makeOffset_dx'>dx</a>, <a href='#SkIRect_makeOffset_dy'>dy</a>).

If <a href='#SkIRect_makeOffset_dx'>dx</a> <a href='#SkIRect_makeOffset_dx'>is</a> <a href='#SkIRect_makeOffset_dx'>negative</a>, <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>returned</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>moved</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>the</a> <a href='SkIRect_Reference#SkIRect'>left</a>.
If <a href='#SkIRect_makeOffset_dx'>dx</a> <a href='#SkIRect_makeOffset_dx'>is</a> <a href='#SkIRect_makeOffset_dx'>positive</a>, <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>returned</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>moved</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>the</a> <a href='SkIRect_Reference#SkIRect'>right</a>.
If <a href='#SkIRect_makeOffset_dy'>dy</a> <a href='#SkIRect_makeOffset_dy'>is</a> <a href='#SkIRect_makeOffset_dy'>negative</a>, <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>returned</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>moved</a> <a href='SkIRect_Reference#SkIRect'>upward</a>.
If <a href='#SkIRect_makeOffset_dy'>dy</a> <a href='#SkIRect_makeOffset_dy'>is</a> <a href='#SkIRect_makeOffset_dy'>positive</a>, <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>returned</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>moved</a> <a href='SkIRect_Reference#SkIRect'>downward</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_makeOffset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_fLeft'>and</a> <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_makeOffset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_fTop'>and</a> <a href='#SkIRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>offset</a> <a href='SkIRect_Reference#SkIRect'>by</a> <a href='#SkIRect_makeOffset_dx'>dx</a> <a href='#SkIRect_makeOffset_dx'>and</a> <a href='#SkIRect_makeOffset_dy'>dy</a>, <a href='#SkIRect_makeOffset_dy'>with</a> <a href='#SkIRect_makeOffset_dy'>original</a> <a href='#SkIRect_makeOffset_dy'>width</a> <a href='#SkIRect_makeOffset_dy'>and</a> <a href='#SkIRect_makeOffset_dy'>height</a>

### Example

<div><fiddle-embed name="737c747df07ddf392c05970440de0927">

#### Example Output

~~~~
rect: 10, 50, 20, 60  isEmpty: false
rect: 25, 82, 35, 92  isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_offset'>offset()</a> <a href='#SkIRect_makeInset'>makeInset</a> <a href='#SkIRect_makeOutset'>makeOutset</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_makeOffset'>makeOffset</a>

<a name='SkIRect_makeInset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkIRect_makeInset'>makeInset</a>(<a href='#SkIRect_makeInset'>int32_t</a> <a href='#SkIRect_makeInset'>dx</a>, <a href='#SkIRect_makeInset'>int32_t</a> <a href='#SkIRect_makeInset'>dy</a>) <a href='#SkIRect_makeInset'>const</a>
</pre>

Returns <a href='SkIRect_Reference#SkIRect'>SkIRect</a>, <a href='SkIRect_Reference#SkIRect'>inset</a> <a href='SkIRect_Reference#SkIRect'>by</a> (<a href='#SkIRect_makeInset_dx'>dx</a>, <a href='#SkIRect_makeInset_dy'>dy</a>).

If <a href='#SkIRect_makeInset_dx'>dx</a> <a href='#SkIRect_makeInset_dx'>is</a> <a href='#SkIRect_makeInset_dx'>negative</a>, <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>returned</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>wider</a>.
If <a href='#SkIRect_makeInset_dx'>dx</a> <a href='#SkIRect_makeInset_dx'>is</a> <a href='#SkIRect_makeInset_dx'>positive</a>, <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>returned</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>narrower</a>.
If <a href='#SkIRect_makeInset_dy'>dy</a> <a href='#SkIRect_makeInset_dy'>is</a> <a href='#SkIRect_makeInset_dy'>negative</a>, <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>returned</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>taller</a>.
If <a href='#SkIRect_makeInset_dy'>dy</a> <a href='#SkIRect_makeInset_dy'>is</a> <a href='#SkIRect_makeInset_dy'>positive</a>, <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>returned</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>shorter</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_makeInset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_fLeft'>and</a> <a href='#SkIRect_fLeft'>subtracted</a> <a href='#SkIRect_fLeft'>from</a> <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_makeInset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_fTop'>and</a> <a href='#SkIRect_fTop'>subtracted</a> <a href='#SkIRect_fTop'>from</a> <a href='#SkIRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>inset</a> <a href='SkIRect_Reference#SkIRect'>symmetrically</a> <a href='SkIRect_Reference#SkIRect'>left</a> <a href='SkIRect_Reference#SkIRect'>and</a> <a href='SkIRect_Reference#SkIRect'>right</a>, <a href='SkIRect_Reference#SkIRect'>top</a> <a href='SkIRect_Reference#SkIRect'>and</a> <a href='SkIRect_Reference#SkIRect'>bottom</a>

### Example

<div><fiddle-embed name="1db94b2c76e0a7a71856532335fa56b6">

#### Example Output

~~~~
rect: 10, 50, 20, 60  isEmpty: false
rect: 25, 82, 5, 28  isEmpty: true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_inset'>inset()</a> <a href='#SkIRect_makeOffset'>makeOffset</a> <a href='#SkIRect_makeOutset'>makeOutset</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_makeInset'>makeInset</a>

<a name='SkIRect_makeOutset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkIRect_makeOutset'>makeOutset</a>(<a href='#SkIRect_makeOutset'>int32_t</a> <a href='#SkIRect_makeOutset'>dx</a>, <a href='#SkIRect_makeOutset'>int32_t</a> <a href='#SkIRect_makeOutset'>dy</a>) <a href='#SkIRect_makeOutset'>const</a>
</pre>

Returns <a href='SkIRect_Reference#SkIRect'>SkIRect</a>, <a href='SkIRect_Reference#SkIRect'>outset</a> <a href='SkIRect_Reference#SkIRect'>by</a> (<a href='#SkIRect_makeOutset_dx'>dx</a>, <a href='#SkIRect_makeOutset_dy'>dy</a>).

If <a href='#SkIRect_makeOutset_dx'>dx</a> <a href='#SkIRect_makeOutset_dx'>is</a> <a href='#SkIRect_makeOutset_dx'>negative</a>, <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>returned</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>narrower</a>.
If <a href='#SkIRect_makeOutset_dx'>dx</a> <a href='#SkIRect_makeOutset_dx'>is</a> <a href='#SkIRect_makeOutset_dx'>positive</a>, <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>returned</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>wider</a>.
If <a href='#SkIRect_makeOutset_dy'>dy</a> <a href='#SkIRect_makeOutset_dy'>is</a> <a href='#SkIRect_makeOutset_dy'>negative</a>, <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>returned</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>shorter</a>.
If <a href='#SkIRect_makeOutset_dy'>dy</a> <a href='#SkIRect_makeOutset_dy'>is</a> <a href='#SkIRect_makeOutset_dy'>positive</a>, <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>returned</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>taller</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_makeOutset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset subtracted to <a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_fLeft'>and</a> <a href='#SkIRect_fLeft'>added</a> <a href='#SkIRect_fLeft'>from</a> <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_makeOutset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset subtracted to <a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_fTop'>and</a> <a href='#SkIRect_fTop'>added</a> <a href='#SkIRect_fTop'>from</a> <a href='#SkIRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>outset</a> <a href='SkIRect_Reference#SkIRect'>symmetrically</a> <a href='SkIRect_Reference#SkIRect'>left</a> <a href='SkIRect_Reference#SkIRect'>and</a> <a href='SkIRect_Reference#SkIRect'>right</a>, <a href='SkIRect_Reference#SkIRect'>top</a> <a href='SkIRect_Reference#SkIRect'>and</a> <a href='SkIRect_Reference#SkIRect'>bottom</a>

### Example

<div><fiddle-embed name="240e2953e3455c08f6d89255feff8416">

#### Example Output

~~~~
rect: 10, 50, 20, 60  isEmpty: false
rect: -5, 18, 35, 92  isEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_outset'>outset()</a> <a href='#SkIRect_makeOffset'>makeOffset</a> <a href='#SkIRect_makeInset'>makeInset</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_makeOutset'>makeOutset</a>

<a name='SkIRect_offset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void offset(int32_t dx, int32_t dy)
</pre>

Offsets <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>by</a> <a href='SkIRect_Reference#SkIRect'>adding</a> <a href='#SkIRect_offset_dx'>dx</a> <a href='#SkIRect_offset_dx'>to</a> <a href='#SkIRect_fLeft'>fLeft</a>, <a href='#SkIRect_fRight'>fRight</a>; <a href='#SkIRect_fRight'>and</a> <a href='#SkIRect_fRight'>by</a> <a href='#SkIRect_fRight'>adding</a> <a href='#SkIRect_offset_dy'>dy</a> <a href='#SkIRect_offset_dy'>to</a> <a href='#SkIRect_fTop'>fTop</a>, <a href='#SkIRect_fBottom'>fBottom</a>.

If <a href='#SkIRect_offset_dx'>dx</a> <a href='#SkIRect_offset_dx'>is</a> <a href='#SkIRect_offset_dx'>negative</a>, <a href='#SkIRect_offset_dx'>moves</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>returned</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>the</a> <a href='SkIRect_Reference#SkIRect'>left</a>.
If <a href='#SkIRect_offset_dx'>dx</a> <a href='#SkIRect_offset_dx'>is</a> <a href='#SkIRect_offset_dx'>positive</a>, <a href='#SkIRect_offset_dx'>moves</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>returned</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>the</a> <a href='SkIRect_Reference#SkIRect'>right</a>.
If <a href='#SkIRect_offset_dy'>dy</a> <a href='#SkIRect_offset_dy'>is</a> <a href='#SkIRect_offset_dy'>negative</a>, <a href='#SkIRect_offset_dy'>moves</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>returned</a> <a href='SkIRect_Reference#SkIRect'>upward</a>.
If <a href='#SkIRect_offset_dy'>dy</a> <a href='#SkIRect_offset_dy'>is</a> <a href='#SkIRect_offset_dy'>positive</a>, <a href='#SkIRect_offset_dy'>moves</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>returned</a> <a href='SkIRect_Reference#SkIRect'>downward</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_offset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_fLeft'>and</a> <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_offset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_fTop'>and</a> <a href='#SkIRect_fBottom'>fBottom</a></td>
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

<a href='#SkIRect_offsetTo'>offsetTo</a> <a href='#SkIRect_makeOffset'>makeOffset</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_offset'>offset</a>

<a name='SkIRect_offset_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void offset(const <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a>& <a href='SkIPoint_Reference#SkIPoint'>delta</a>)
</pre>

Offsets <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>by</a> <a href='SkIRect_Reference#SkIRect'>adding</a> <a href='#SkIRect_offset_2_delta'>delta</a>.<a href='#SkIPoint_fX'>fX</a> <a href='#SkIPoint_fX'>to</a> <a href='#SkIRect_fLeft'>fLeft</a>, <a href='#SkIRect_fRight'>fRight</a>; <a href='#SkIRect_fRight'>and</a> <a href='#SkIRect_fRight'>by</a> <a href='#SkIRect_fRight'>adding</a> <a href='#SkIRect_offset_2_delta'>delta</a>.<a href='#SkIPoint_fY'>fY</a> <a href='#SkIPoint_fY'>to</a>
<a href='#SkIRect_fTop'>fTop</a>, <a href='#SkIRect_fBottom'>fBottom</a>.

If <a href='#SkIRect_offset_2_delta'>delta</a>.<a href='#SkIPoint_fX'>fX</a> <a href='#SkIPoint_fX'>is</a> <a href='#SkIPoint_fX'>negative</a>, <a href='#SkIPoint_fX'>moves</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>returned</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>the</a> <a href='SkIRect_Reference#SkIRect'>left</a>.
If <a href='#SkIRect_offset_2_delta'>delta</a>.<a href='#SkIPoint_fX'>fX</a> <a href='#SkIPoint_fX'>is</a> <a href='#SkIPoint_fX'>positive</a>, <a href='#SkIPoint_fX'>moves</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>returned</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>the</a> <a href='SkIRect_Reference#SkIRect'>right</a>.
If <a href='#SkIRect_offset_2_delta'>delta</a>.<a href='#SkIPoint_fY'>fY</a> <a href='#SkIPoint_fY'>is</a> <a href='#SkIPoint_fY'>negative</a>, <a href='#SkIPoint_fY'>moves</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>returned</a> <a href='SkIRect_Reference#SkIRect'>upward</a>.
If <a href='#SkIRect_offset_2_delta'>delta</a>.<a href='#SkIPoint_fY'>fY</a> <a href='#SkIPoint_fY'>is</a> <a href='#SkIPoint_fY'>positive</a>, <a href='#SkIPoint_fY'>moves</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>returned</a> <a href='SkIRect_Reference#SkIRect'>downward</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_offset_2_delta'><code><strong>delta</strong></code></a></td>
    <td>offset added to <a href='SkIRect_Reference#SkIRect'>SkIRect</a></td>
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

<a href='#SkIRect_offsetTo'>offsetTo</a> <a href='#SkIRect_makeOffset'>makeOffset</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_offset'>offset</a>

<a name='SkIRect_offsetTo'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_offsetTo'>offsetTo</a>(<a href='#SkIRect_offsetTo'>int32_t</a> <a href='#SkIRect_offsetTo'>newX</a>, <a href='#SkIRect_offsetTo'>int32_t</a> <a href='#SkIRect_offsetTo'>newY</a>)
</pre>

Offsets <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>so</a> <a href='SkIRect_Reference#SkIRect'>that</a> <a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_fLeft'>equals</a> <a href='#SkIRect_offsetTo_newX'>newX</a>, <a href='#SkIRect_offsetTo_newX'>and</a> <a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_fTop'>equals</a> <a href='#SkIRect_offsetTo_newY'>newY</a>. <a href='#SkIRect_offsetTo_newY'>width</a> <a href='#SkIRect_offsetTo_newY'>and</a> <a href='#SkIRect_offsetTo_newY'>height</a>
are unchanged.

### Parameters

<table>  <tr>    <td><a name='SkIRect_offsetTo_newX'><code><strong>newX</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fLeft'>fLeft</a>, <a href='#SkIRect_fLeft'>preserving</a> <a href='#SkIRect_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_offsetTo_newY'><code><strong>newY</strong></code></a></td>
    <td>stored in <a href='#SkIRect_fTop'>fTop</a>, <a href='#SkIRect_fTop'>preserving</a> <a href='#SkIRect_height'>height()</a></td>
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

<a href='#SkIRect_offset'>offset</a> <a href='#SkIRect_makeOffset'>makeOffset</a> <a href='#SkIRect_setXYWH'>setXYWH</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_offsetTo'>offsetTo</a>

<a name='SkIRect_inset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void inset(int32_t dx, int32_t dy)
</pre>

Insets <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>by</a> (<a href='#SkIRect_inset_dx'>dx</a>,<a href='#SkIRect_inset_dy'>dy</a>).

If <a href='#SkIRect_inset_dx'>dx</a> <a href='#SkIRect_inset_dx'>is</a> <a href='#SkIRect_inset_dx'>positive</a>, <a href='#SkIRect_inset_dx'>makes</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>narrower</a>.
If <a href='#SkIRect_inset_dx'>dx</a> <a href='#SkIRect_inset_dx'>is</a> <a href='#SkIRect_inset_dx'>negative</a>, <a href='#SkIRect_inset_dx'>makes</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>wider</a>.
If <a href='#SkIRect_inset_dy'>dy</a> <a href='#SkIRect_inset_dy'>is</a> <a href='#SkIRect_inset_dy'>positive</a>, <a href='#SkIRect_inset_dy'>makes</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>shorter</a>.
If <a href='#SkIRect_inset_dy'>dy</a> <a href='#SkIRect_inset_dy'>is</a> <a href='#SkIRect_inset_dy'>negative</a>, <a href='#SkIRect_inset_dy'>makes</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>taller</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_inset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_fLeft'>and</a> <a href='#SkIRect_fLeft'>subtracted</a> <a href='#SkIRect_fLeft'>from</a> <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_inset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_fTop'>and</a> <a href='#SkIRect_fTop'>subtracted</a> <a href='#SkIRect_fTop'>from</a> <a href='#SkIRect_fBottom'>fBottom</a></td>
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

<a href='#SkIRect_outset'>outset</a> <a href='#SkIRect_makeInset'>makeInset</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_inset'>inset</a>

<a name='SkIRect_outset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void outset(int32_t dx, int32_t dy)
</pre>

Outsets <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>by</a> (<a href='#SkIRect_outset_dx'>dx</a>, <a href='#SkIRect_outset_dy'>dy</a>).

If <a href='#SkIRect_outset_dx'>dx</a> <a href='#SkIRect_outset_dx'>is</a> <a href='#SkIRect_outset_dx'>positive</a>, <a href='#SkIRect_outset_dx'>makes</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>wider</a>.
If <a href='#SkIRect_outset_dx'>dx</a> <a href='#SkIRect_outset_dx'>is</a> <a href='#SkIRect_outset_dx'>negative</a>, <a href='#SkIRect_outset_dx'>makes</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>narrower</a>.
If <a href='#SkIRect_outset_dy'>dy</a> <a href='#SkIRect_outset_dy'>is</a> <a href='#SkIRect_outset_dy'>positive</a>, <a href='#SkIRect_outset_dy'>makes</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>taller</a>.
If <a href='#SkIRect_outset_dy'>dy</a> <a href='#SkIRect_outset_dy'>is</a> <a href='#SkIRect_outset_dy'>negative</a>, <a href='#SkIRect_outset_dy'>makes</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>shorter</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_outset_dx'><code><strong>dx</strong></code></a></td>
    <td>subtracted to <a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_fLeft'>and</a> <a href='#SkIRect_fLeft'>added</a> <a href='#SkIRect_fLeft'>from</a> <a href='#SkIRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_outset_dy'><code><strong>dy</strong></code></a></td>
    <td>subtracted to <a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_fTop'>and</a> <a href='#SkIRect_fTop'>added</a> <a href='#SkIRect_fTop'>from</a> <a href='#SkIRect_fBottom'>fBottom</a></td>
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

<a href='#SkIRect_inset'>inset</a> <a href='#SkIRect_makeOutset'>makeOutset</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_outset'>outset</a>

<a name='Intersection'></a>

<a href='#IRect'>IRects</a> <a href='#IRect'>intersect</a> <a href='#IRect'>when</a> <a href='#IRect'>they</a> <a href='#IRect'>enclose</a> <a href='#IRect'>a</a> <a href='#IRect'>common</a> <a href='#IRect'>area</a>. <a href='#IRect'>To</a> <a href='#IRect'>intersect</a>, <a href='#IRect'>each</a> <a href='#IRect'>of</a> <a href='#IRect'>the</a> <a href='#IRect'>pair</a>
<a href='#IRect'>must</a> <a href='#IRect'>describe</a> <a href='#IRect'>area</a>; <a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_fLeft'>is</a> <a href='#SkIRect_fLeft'>less</a> <a href='#SkIRect_fLeft'>than</a> <a href='#SkIRect_fRight'>fRight</a>, <a href='#SkIRect_fRight'>and</a> <a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_fTop'>is</a> <a href='#SkIRect_fTop'>less</a> <a href='#SkIRect_fTop'>than</a> <a href='#SkIRect_fBottom'>fBottom</a>;
<a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_isEmpty'>isEmpty</a>() <a href='#SkIRect_isEmpty'>returns</a> <a href='#SkIRect_isEmpty'>false</a>. <a href='#SkIRect_isEmpty'>The</a> <a href='#SkIRect_isEmpty'>intersection</a> <a href='#SkIRect_isEmpty'>of</a> <a href='SkIRect_Reference#IRect'>IRect</a> <a href='SkIRect_Reference#IRect'>pair</a> <a href='SkIRect_Reference#IRect'>can</a> <a href='SkIRect_Reference#IRect'>be</a> <a href='SkIRect_Reference#IRect'>described</a> <a href='SkIRect_Reference#IRect'>by</a>:
<code>(<a href='undocumented#max()'>max</a>(<a href='undocumented#max()'>a</a>.<a href='#SkIRect_fLeft'>fLeft</a>, <a href='#SkIRect_fLeft'>b</a>.<a href='#SkIRect_fLeft'>fLeft</a>), <a href='undocumented#max()'>max</a>(<a href='undocumented#max()'>a</a>.<a href='#SkIRect_fTop'>fTop</a>, <a href='#SkIRect_fTop'>b</a>.<a href='#SkIRect_fTop'>fTop</a>),
<a href='undocumented#min()'>min</a>(<a href='undocumented#min()'>a</a>.<a href='#SkIRect_fRight'>fRight</a>, <a href='#SkIRect_fRight'>b</a>.<a href='#SkIRect_fRight'>fRight</a>), <a href='undocumented#min()'>min</a>(<a href='undocumented#min()'>a</a>.<a href='#SkIRect_fBottom'>fBottom</a>, <a href='#SkIRect_fBottom'>b</a>.<a href='#SkIRect_fBottom'>fBottom</a>))</code>.

The intersection is only meaningful if the resulting <a href='SkIRect_Reference#IRect'>IRect</a> <a href='SkIRect_Reference#IRect'>is</a> <a href='SkIRect_Reference#IRect'>not</a> <a href='SkIRect_Reference#IRect'>empty</a> <a href='SkIRect_Reference#IRect'>and</a>
<a href='SkIRect_Reference#IRect'>describes</a> <a href='SkIRect_Reference#IRect'>an</a> <a href='SkIRect_Reference#IRect'>area</a>: <a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_fLeft'>is</a> <a href='#SkIRect_fLeft'>less</a> <a href='#SkIRect_fLeft'>than</a> <a href='#SkIRect_fRight'>fRight</a>, <a href='#SkIRect_fRight'>and</a> <a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_fTop'>is</a> <a href='#SkIRect_fTop'>less</a> <a href='#SkIRect_fTop'>than</a> <a href='#SkIRect_fBottom'>fBottom</a>.

<a name='SkIRect_adjust'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_adjust'>adjust</a>(<a href='#SkIRect_adjust'>int32_t</a> <a href='#SkIRect_adjust'>dL</a>, <a href='#SkIRect_adjust'>int32_t</a> <a href='#SkIRect_adjust'>dT</a>, <a href='#SkIRect_adjust'>int32_t</a> <a href='#SkIRect_adjust'>dR</a>, <a href='#SkIRect_adjust'>int32_t</a> <a href='#SkIRect_adjust'>dB</a>)
</pre>

Adjusts <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>by</a> <a href='SkIRect_Reference#SkIRect'>adding</a> <a href='#SkIRect_adjust_dL'>dL</a> <a href='#SkIRect_adjust_dL'>to</a> <a href='#SkIRect_fLeft'>fLeft</a>, <a href='#SkIRect_adjust_dT'>dT</a> <a href='#SkIRect_adjust_dT'>to</a> <a href='#SkIRect_fTop'>fTop</a>, <a href='#SkIRect_adjust_dR'>dR</a> <a href='#SkIRect_adjust_dR'>to</a> <a href='#SkIRect_fRight'>fRight</a>, <a href='#SkIRect_fRight'>and</a> <a href='#SkIRect_adjust_dB'>dB</a> <a href='#SkIRect_adjust_dB'>to</a> <a href='#SkIRect_fBottom'>fBottom</a>.

If <a href='#SkIRect_adjust_dL'>dL</a> <a href='#SkIRect_adjust_dL'>is</a> <a href='#SkIRect_adjust_dL'>positive</a>, <a href='#SkIRect_adjust_dL'>narrows</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>on</a> <a href='SkIRect_Reference#SkIRect'>the</a> <a href='SkIRect_Reference#SkIRect'>left</a>. <a href='SkIRect_Reference#SkIRect'>If</a> <a href='SkIRect_Reference#SkIRect'>negative</a>, <a href='SkIRect_Reference#SkIRect'>widens</a> <a href='SkIRect_Reference#SkIRect'>it</a> <a href='SkIRect_Reference#SkIRect'>on</a> <a href='SkIRect_Reference#SkIRect'>the</a> <a href='SkIRect_Reference#SkIRect'>left</a>.
If <a href='#SkIRect_adjust_dT'>dT</a> <a href='#SkIRect_adjust_dT'>is</a> <a href='#SkIRect_adjust_dT'>positive</a>, <a href='#SkIRect_adjust_dT'>shrinks</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>on</a> <a href='SkIRect_Reference#SkIRect'>the</a> <a href='SkIRect_Reference#SkIRect'>top</a>. <a href='SkIRect_Reference#SkIRect'>If</a> <a href='SkIRect_Reference#SkIRect'>negative</a>, <a href='SkIRect_Reference#SkIRect'>lengthens</a> <a href='SkIRect_Reference#SkIRect'>it</a> <a href='SkIRect_Reference#SkIRect'>on</a> <a href='SkIRect_Reference#SkIRect'>the</a> <a href='SkIRect_Reference#SkIRect'>top</a>.
If <a href='#SkIRect_adjust_dR'>dR</a> <a href='#SkIRect_adjust_dR'>is</a> <a href='#SkIRect_adjust_dR'>positive</a>, <a href='#SkIRect_adjust_dR'>narrows</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>on</a> <a href='SkIRect_Reference#SkIRect'>the</a> <a href='SkIRect_Reference#SkIRect'>right</a>. <a href='SkIRect_Reference#SkIRect'>If</a> <a href='SkIRect_Reference#SkIRect'>negative</a>, <a href='SkIRect_Reference#SkIRect'>widens</a> <a href='SkIRect_Reference#SkIRect'>it</a> <a href='SkIRect_Reference#SkIRect'>on</a> <a href='SkIRect_Reference#SkIRect'>the</a> <a href='SkIRect_Reference#SkIRect'>right</a>.
If <a href='#SkIRect_adjust_dB'>dB</a> <a href='#SkIRect_adjust_dB'>is</a> <a href='#SkIRect_adjust_dB'>positive</a>, <a href='#SkIRect_adjust_dB'>shrinks</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>on</a> <a href='SkIRect_Reference#SkIRect'>the</a> <a href='SkIRect_Reference#SkIRect'>bottom</a>. <a href='SkIRect_Reference#SkIRect'>If</a> <a href='SkIRect_Reference#SkIRect'>negative</a>, <a href='SkIRect_Reference#SkIRect'>lengthens</a> <a href='SkIRect_Reference#SkIRect'>it</a> <a href='SkIRect_Reference#SkIRect'>on</a> <a href='SkIRect_Reference#SkIRect'>the</a> <a href='SkIRect_Reference#SkIRect'>bottom</a>.

The resulting <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>not</a> <a href='SkIRect_Reference#SkIRect'>checked</a> <a href='SkIRect_Reference#SkIRect'>for</a> <a href='SkIRect_Reference#SkIRect'>validity</a>. <a href='SkIRect_Reference#SkIRect'>Thus</a>, <a href='SkIRect_Reference#SkIRect'>if</a> <a href='SkIRect_Reference#SkIRect'>the</a> <a href='SkIRect_Reference#SkIRect'>resulting</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>left</a> <a href='SkIRect_Reference#SkIRect'>is</a>
greater than right, the <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>will</a> <a href='SkIRect_Reference#SkIRect'>be</a> <a href='SkIRect_Reference#SkIRect'>considered</a> <a href='SkIRect_Reference#SkIRect'>empty</a>. <a href='SkIRect_Reference#SkIRect'>Call</a> <a href='#SkIRect_sort'>sort()</a> <a href='#SkIRect_sort'>after</a> <a href='#SkIRect_sort'>this</a> <a href='#SkIRect_sort'>call</a>
if that is not the desired behavior.

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

<a href='#SkIRect_inset'>inset outset</a>

<a name='SkIRect_contains'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool contains(int32_t x, int32_t y) const
</pre>

Returns true if: <code><a href='#SkIRect_fLeft'>fLeft</a> <= <a href='#SkIRect_contains_x'>x</a> < <a href='#SkIRect_fRight'>fRight</a> && <a href='#SkIRect_fTop'>fTop</a> <= <a href='#SkIRect_contains_y'>y</a> < <a href='#SkIRect_fBottom'>fBottom</a></code>.
Returns false if <a href='SkIRect_Reference#IRect'>IRect</a> <a href='SkIRect_Reference#IRect'>is</a> <a href='SkIRect_Reference#IRect'>empty</a>.

<a href='SkIRect_Reference#IRect'>Considers</a> <a href='SkIRect_Reference#IRect'>input</a> <a href='SkIRect_Reference#IRect'>to</a> <a href='SkIRect_Reference#IRect'>describe</a> <a href='SkIRect_Reference#IRect'>constructed</a> <a href='SkIRect_Reference#IRect'>IRect</a>: <code>(<a href='#SkIRect_contains_x'>x</a>, <a href='#SkIRect_contains_y'>y</a>, <a href='#SkIRect_contains_x'>x</a> + 1, <a href='#SkIRect_contains_y'>y</a> + 1)</code> and
returns true if constructed area is completely enclosed by <a href='SkIRect_Reference#IRect'>IRect</a> <a href='SkIRect_Reference#IRect'>area</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_contains_x'><code><strong>x</strong></code></a></td>
    <td>test <a href='SkIPoint_Reference#IPoint'>IPoint</a> <a href='SkIPoint_Reference#IPoint'>x-coordinate</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_contains_y'><code><strong>y</strong></code></a></td>
    <td>test <a href='SkIPoint_Reference#IPoint'>IPoint</a> <a href='SkIPoint_Reference#IPoint'>y-coordinate</a></td>
  </tr>
</table>

### Return Value

true if (<a href='#SkIRect_contains_x'>x</a>, <a href='#SkIRect_contains_y'>y</a>) <a href='#SkIRect_contains_y'>is</a> <a href='#SkIRect_contains_y'>inside</a> <a href='SkIRect_Reference#IRect'>IRect</a>

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

<a href='#SkIRect_containsNoEmptyCheck'>containsNoEmptyCheck</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_contains'>contains</a>

<a name='SkIRect_contains_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool contains(int32_t left, int32_t top, int32_t right, int32_t bottom) const
</pre>

Constructs <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>intersect</a> <a href='SkIRect_Reference#SkIRect'>from</a> (<a href='#SkIRect_contains_2_left'>left</a>, <a href='#SkIRect_contains_2_top'>top</a>, <a href='#SkIRect_contains_2_right'>right</a>, <a href='#SkIRect_contains_2_bottom'>bottom</a>). <a href='#SkIRect_contains_2_bottom'>Does</a> <a href='#SkIRect_contains_2_bottom'>not</a> <a href='#SkIRect_contains_2_bottom'>sort</a>
construction.

Returns true if <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>contains</a> <a href='SkIRect_Reference#SkIRect'>construction</a>.
Returns false if <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>empty</a> <a href='SkIRect_Reference#SkIRect'>or</a> <a href='SkIRect_Reference#SkIRect'>construction</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>empty</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_contains_2_left'><code><strong>left</strong></code></a></td>
    <td>x-axis minimum of constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_contains_2_top'><code><strong>top</strong></code></a></td>
    <td>y-axis minimum of constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_contains_2_right'><code><strong>right</strong></code></a></td>
    <td>x-axis maximum of constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_contains_2_bottom'><code><strong>bottom</strong></code></a></td>
    <td>y-axis maximum of constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a></td>
  </tr>
</table>

### Return Value

true if all sides of <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>are</a> <a href='SkIRect_Reference#SkIRect'>outside</a> <a href='SkIRect_Reference#SkIRect'>construction</a>

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

<a href='#SkIRect_containsNoEmptyCheck'>containsNoEmptyCheck</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_contains'>contains</a>

<a name='SkIRect_contains_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool contains(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>r</a>) <a href='SkIRect_Reference#SkIRect'>const</a>
</pre>

Returns true if <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>contains</a> <a href='#SkIRect_contains_3_r'>r</a>.
Returns false if <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>empty</a> <a href='SkIRect_Reference#SkIRect'>or</a> <a href='#SkIRect_contains_3_r'>r</a> <a href='#SkIRect_contains_3_r'>is</a> <a href='#SkIRect_contains_3_r'>empty</a>.

<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>contains</a> <a href='#SkIRect_contains_3_r'>r</a> <a href='#SkIRect_contains_3_r'>when</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>area</a> <a href='SkIRect_Reference#SkIRect'>completely</a> <a href='SkIRect_Reference#SkIRect'>includes</a> <a href='#SkIRect_contains_3_r'>r</a> <a href='#SkIRect_contains_3_r'>area</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_contains_3_r'><code><strong>r</strong></code></a></td>
    <td><a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>contained</a></td>
  </tr>
</table>

### Return Value

true if all sides of <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>are</a> <a href='SkIRect_Reference#SkIRect'>outside</a> <a href='#SkIRect_contains_3_r'>r</a>

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

<a href='#SkIRect_containsNoEmptyCheck'>containsNoEmptyCheck</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_contains'>contains</a>

<a name='SkIRect_contains_4'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool contains(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>r</a>) <a href='SkRect_Reference#SkRect'>const</a>
</pre>

Returns true if <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>contains</a> <a href='#SkIRect_contains_4_r'>r</a>.
Returns false if <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>empty</a> <a href='SkIRect_Reference#SkIRect'>or</a> <a href='#SkIRect_contains_4_r'>r</a> <a href='#SkIRect_contains_4_r'>is</a> <a href='#SkIRect_contains_4_r'>empty</a>.

<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>contains</a> <a href='#SkIRect_contains_4_r'>r</a> <a href='#SkIRect_contains_4_r'>when</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>area</a> <a href='SkIRect_Reference#SkIRect'>completely</a> <a href='SkIRect_Reference#SkIRect'>includes</a> <a href='#SkIRect_contains_4_r'>r</a> <a href='#SkIRect_contains_4_r'>area</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_contains_4_r'><code><strong>r</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>contained</a></td>
  </tr>
</table>

### Return Value

true if all sides of <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>are</a> <a href='SkIRect_Reference#SkIRect'>outside</a> <a href='#SkIRect_contains_4_r'>r</a>

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

<a href='#SkIRect_containsNoEmptyCheck'>containsNoEmptyCheck</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_contains'>contains</a>

<a name='SkIRect_containsNoEmptyCheck'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_containsNoEmptyCheck'>containsNoEmptyCheck</a>(<a href='#SkIRect_containsNoEmptyCheck'>int32_t</a> <a href='#SkIRect_containsNoEmptyCheck'>left</a>, <a href='#SkIRect_containsNoEmptyCheck'>int32_t</a> <a href='#SkIRect_containsNoEmptyCheck'>top</a>, <a href='#SkIRect_containsNoEmptyCheck'>int32_t</a> <a href='#SkIRect_containsNoEmptyCheck'>right</a>, <a href='#SkIRect_containsNoEmptyCheck'>int32_t</a> <a href='#SkIRect_containsNoEmptyCheck'>bottom</a>) <a href='#SkIRect_containsNoEmptyCheck'>const</a>
</pre>

Constructs <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>from</a> (<a href='#SkIRect_containsNoEmptyCheck_left'>left</a>, <a href='#SkIRect_containsNoEmptyCheck_top'>top</a>, <a href='#SkIRect_containsNoEmptyCheck_right'>right</a>, <a href='#SkIRect_containsNoEmptyCheck_bottom'>bottom</a>). <a href='#SkIRect_containsNoEmptyCheck_bottom'>Does</a> <a href='#SkIRect_containsNoEmptyCheck_bottom'>not</a> <a href='#SkIRect_containsNoEmptyCheck_bottom'>sort</a>
construction.

Returns true if <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>contains</a> <a href='SkIRect_Reference#SkIRect'>construction</a>.
Asserts if <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>empty</a> <a href='SkIRect_Reference#SkIRect'>or</a> <a href='SkIRect_Reference#SkIRect'>construction</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>empty</a>, <a href='SkIRect_Reference#SkIRect'>and</a> <a href='SkIRect_Reference#SkIRect'>if</a> <a href='SkIRect_Reference#SkIRect'>SK_DEBUG</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>defined</a>.

Return is undefined if <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>empty</a> <a href='SkIRect_Reference#SkIRect'>or</a> <a href='SkIRect_Reference#SkIRect'>construction</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>empty</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_containsNoEmptyCheck_left'><code><strong>left</strong></code></a></td>
    <td>x-axis minimum of constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_containsNoEmptyCheck_top'><code><strong>top</strong></code></a></td>
    <td>y-axis minimum of constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_containsNoEmptyCheck_right'><code><strong>right</strong></code></a></td>
    <td>x-axis maximum of constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_containsNoEmptyCheck_bottom'><code><strong>bottom</strong></code></a></td>
    <td>y-axis maximum of constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a></td>
  </tr>
</table>

### Return Value

true if all sides of <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>are</a> <a href='SkIRect_Reference#SkIRect'>outside</a> <a href='SkIRect_Reference#SkIRect'>construction</a>

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

<a href='#SkIRect_contains'>contains</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_contains'>contains</a>

<a name='SkIRect_containsNoEmptyCheck_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_containsNoEmptyCheck'>containsNoEmptyCheck</a>(<a href='#SkIRect_containsNoEmptyCheck'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>r</a>) <a href='SkIRect_Reference#SkIRect'>const</a>
</pre>

Returns true if <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>contains</a> <a href='SkIRect_Reference#SkIRect'>construction</a>.
Asserts if <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>empty</a> <a href='SkIRect_Reference#SkIRect'>or</a> <a href='SkIRect_Reference#SkIRect'>construction</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>empty</a>, <a href='SkIRect_Reference#SkIRect'>and</a> <a href='SkIRect_Reference#SkIRect'>if</a> <a href='SkIRect_Reference#SkIRect'>SK_DEBUG</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>defined</a>.

Return is undefined if <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>empty</a> <a href='SkIRect_Reference#SkIRect'>or</a> <a href='SkIRect_Reference#SkIRect'>construction</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>empty</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_containsNoEmptyCheck_2_r'><code><strong>r</strong></code></a></td>
    <td><a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>contained</a></td>
  </tr>
</table>

### Return Value

true if all sides of <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>are</a> <a href='SkIRect_Reference#SkIRect'>outside</a> <a href='#SkIRect_containsNoEmptyCheck_2_r'>r</a>

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

<a href='#SkIRect_contains'>contains</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_contains'>contains</a>

<a name='SkIRect_intersect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool intersect(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>r</a>)
</pre>

Returns true if <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>intersects</a> <a href='#SkIRect_intersect_r'>r</a>, <a href='#SkIRect_intersect_r'>and</a> <a href='#SkIRect_intersect_r'>sets</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>intersection</a>.
Returns false if <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>does</a> <a href='SkIRect_Reference#SkIRect'>not</a> <a href='SkIRect_Reference#SkIRect'>intersect</a> <a href='#SkIRect_intersect_r'>r</a>, <a href='#SkIRect_intersect_r'>and</a> <a href='#SkIRect_intersect_r'>leaves</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>unchanged</a>.

Returns false if either <a href='#SkIRect_intersect_r'>r</a> <a href='#SkIRect_intersect_r'>or</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>empty</a>, <a href='SkIRect_Reference#SkIRect'>leaving</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>unchanged</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_intersect_r'><code><strong>r</strong></code></a></td>
    <td>limit of result</td>
  </tr>
</table>

### Return Value

true if <a href='#SkIRect_intersect_r'>r</a> <a href='#SkIRect_intersect_r'>and</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>have</a> <a href='SkIRect_Reference#SkIRect'>area</a> <a href='SkIRect_Reference#SkIRect'>in</a> <a href='SkIRect_Reference#SkIRect'>common</a>

### Example

<div><fiddle-embed name="ea233f5d5d1ae0e76fc6f2eb371c927a"><div>Two <a href='undocumented#SkDebugf'>SkDebugf</a> <a href='undocumented#SkDebugf'>calls</a> <a href='undocumented#SkDebugf'>are</a> <a href='undocumented#SkDebugf'>required</a>. <a href='undocumented#SkDebugf'>If</a> <a href='undocumented#SkDebugf'>the</a> <a href='undocumented#SkDebugf'>calls</a> <a href='undocumented#SkDebugf'>are</a> <a href='undocumented#SkDebugf'>combined</a>, <a href='undocumented#SkDebugf'>their</a> <a href='undocumented#SkDebugf'>arguments</a>
<a href='undocumented#SkDebugf'>may</a> <a href='undocumented#SkDebugf'>not</a> <a href='undocumented#SkDebugf'>be</a> <a href='undocumented#SkDebugf'>evaluated</a> <a href='undocumented#SkDebugf'>in</a> <a href='undocumented#SkDebugf'>left</a> <a href='undocumented#SkDebugf'>to</a> <a href='undocumented#SkDebugf'>right</a> <a href='undocumented#SkDebugf'>order</a>: <a href='undocumented#SkDebugf'>the</a> <a href='undocumented#SkDebugf'>printed</a> <a href='undocumented#SkDebugf'>intersection</a> <a href='undocumented#SkDebugf'>may</a>
<a href='undocumented#SkDebugf'>be</a> <a href='undocumented#SkDebugf'>before</a> <a href='undocumented#SkDebugf'>or</a> <a href='undocumented#SkDebugf'>after</a> <a href='undocumented#SkDebugf'>the</a> <a href='undocumented#SkDebugf'>call</a> <a href='undocumented#SkDebugf'>to</a> <a href='undocumented#SkDebugf'>intersect</a>.
</div>

#### Example Output

~~~~
intersection: 30, 60, 50, 80
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_Intersects'>Intersects</a> <a href='#SkIRect_intersectNoEmptyCheck'>intersectNoEmptyCheck</a> <a href='#SkIRect_join'>join</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_intersect'>intersect</a>

<a name='SkIRect_intersect_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool intersect(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>a</a>, <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>b</a>)
</pre>

Returns true if <a href='#SkIRect_intersect_2_a'>a</a> <a href='#SkIRect_intersect_2_a'>intersects</a> <a href='#SkIRect_intersect_2_b'>b</a>, <a href='#SkIRect_intersect_2_b'>and</a> <a href='#SkIRect_intersect_2_b'>sets</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>intersection</a>.
Returns false if <a href='#SkIRect_intersect_2_a'>a</a> <a href='#SkIRect_intersect_2_a'>does</a> <a href='#SkIRect_intersect_2_a'>not</a> <a href='#SkIRect_intersect_2_a'>intersect</a> <a href='#SkIRect_intersect_2_b'>b</a>, <a href='#SkIRect_intersect_2_b'>and</a> <a href='#SkIRect_intersect_2_b'>leaves</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>unchanged</a>.

Returns false if either <a href='#SkIRect_intersect_2_a'>a</a> <a href='#SkIRect_intersect_2_a'>or</a> <a href='#SkIRect_intersect_2_b'>b</a> <a href='#SkIRect_intersect_2_b'>is</a> <a href='#SkIRect_intersect_2_b'>empty</a>, <a href='#SkIRect_intersect_2_b'>leaving</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>unchanged</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_intersect_2_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>intersect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_intersect_2_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>intersect</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkIRect_intersect_2_a'>a</a> <a href='#SkIRect_intersect_2_a'>and</a> <a href='#SkIRect_intersect_2_b'>b</a> <a href='#SkIRect_intersect_2_b'>have</a> <a href='#SkIRect_intersect_2_b'>area</a> <a href='#SkIRect_intersect_2_b'>in</a> <a href='#SkIRect_intersect_2_b'>common</a>

### Example

<div><fiddle-embed name="b2db0573aacf99ca52776c5522459d02">

#### Example Output

~~~~
intersection: 30, 60, 50, 80
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_Intersects'>Intersects</a> <a href='#SkIRect_intersectNoEmptyCheck'>intersectNoEmptyCheck</a> <a href='#SkIRect_join'>join</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_intersect'>intersect</a>

<a name='SkIRect_intersectNoEmptyCheck'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkIRect_intersectNoEmptyCheck'>intersectNoEmptyCheck</a>(<a href='#SkIRect_intersectNoEmptyCheck'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>a</a>, <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>b</a>)
</pre>

Returns true if <a href='#SkIRect_intersectNoEmptyCheck_a'>a</a> <a href='#SkIRect_intersectNoEmptyCheck_a'>intersects</a> <a href='#SkIRect_intersectNoEmptyCheck_b'>b</a>, <a href='#SkIRect_intersectNoEmptyCheck_b'>and</a> <a href='#SkIRect_intersectNoEmptyCheck_b'>sets</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>intersection</a>.
Returns false if <a href='#SkIRect_intersectNoEmptyCheck_a'>a</a> <a href='#SkIRect_intersectNoEmptyCheck_a'>does</a> <a href='#SkIRect_intersectNoEmptyCheck_a'>not</a> <a href='#SkIRect_intersectNoEmptyCheck_a'>intersect</a> <a href='#SkIRect_intersectNoEmptyCheck_b'>b</a>, <a href='#SkIRect_intersectNoEmptyCheck_b'>and</a> <a href='#SkIRect_intersectNoEmptyCheck_b'>leaves</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>unchanged</a>.

Asserts if either <a href='#SkIRect_intersectNoEmptyCheck_a'>a</a> <a href='#SkIRect_intersectNoEmptyCheck_a'>or</a> <a href='#SkIRect_intersectNoEmptyCheck_b'>b</a> <a href='#SkIRect_intersectNoEmptyCheck_b'>is</a> <a href='#SkIRect_intersectNoEmptyCheck_b'>empty</a>, <a href='#SkIRect_intersectNoEmptyCheck_b'>and</a> <a href='#SkIRect_intersectNoEmptyCheck_b'>if</a> <a href='#SkIRect_intersectNoEmptyCheck_b'>SK_DEBUG</a> <a href='#SkIRect_intersectNoEmptyCheck_b'>is</a> <a href='#SkIRect_intersectNoEmptyCheck_b'>defined</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_intersectNoEmptyCheck_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>intersect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_intersectNoEmptyCheck_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>intersect</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkIRect_intersectNoEmptyCheck_a'>a</a> <a href='#SkIRect_intersectNoEmptyCheck_a'>and</a> <a href='#SkIRect_intersectNoEmptyCheck_b'>b</a> <a href='#SkIRect_intersectNoEmptyCheck_b'>have</a> <a href='#SkIRect_intersectNoEmptyCheck_b'>area</a> <a href='#SkIRect_intersectNoEmptyCheck_b'>in</a> <a href='#SkIRect_intersectNoEmptyCheck_b'>common</a>

### Example

<div><fiddle-embed name="d35fbc9fdea71df8b8a12fd3da50d11c">

#### Example Output

~~~~
intersection: 30, 60, 50, 80
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_Intersects'>Intersects</a> <a href='#SkIRect_intersect'>intersect</a> <a href='#SkIRect_join'>join</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_intersect'>intersect</a>

<a name='SkIRect_intersect_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool intersect(int32_t left, int32_t top, int32_t right, int32_t bottom)
</pre>

Constructs <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>intersect</a> <a href='SkIRect_Reference#SkIRect'>from</a> (<a href='#SkIRect_intersect_3_left'>left</a>, <a href='#SkIRect_intersect_3_top'>top</a>, <a href='#SkIRect_intersect_3_right'>right</a>, <a href='#SkIRect_intersect_3_bottom'>bottom</a>). <a href='#SkIRect_intersect_3_bottom'>Does</a> <a href='#SkIRect_intersect_3_bottom'>not</a> <a href='#SkIRect_intersect_3_bottom'>sort</a>
construction.

Returns true if <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>intersects</a> <a href='SkIRect_Reference#SkIRect'>construction</a>, <a href='SkIRect_Reference#SkIRect'>and</a> <a href='SkIRect_Reference#SkIRect'>sets</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>intersection</a>.
Returns false if <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>does</a> <a href='SkIRect_Reference#SkIRect'>not</a> <a href='SkIRect_Reference#SkIRect'>intersect</a> <a href='SkIRect_Reference#SkIRect'>construction</a>, <a href='SkIRect_Reference#SkIRect'>and</a> <a href='SkIRect_Reference#SkIRect'>leaves</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>unchanged</a>.

Returns false if either construction or <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>empty</a>, <a href='SkIRect_Reference#SkIRect'>leaving</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>unchanged</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_intersect_3_left'><code><strong>left</strong></code></a></td>
    <td>x-axis minimum of constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_intersect_3_top'><code><strong>top</strong></code></a></td>
    <td>y-axis minimum of constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_intersect_3_right'><code><strong>right</strong></code></a></td>
    <td>x-axis maximum of constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_intersect_3_bottom'><code><strong>bottom</strong></code></a></td>
    <td>y-axis maximum of constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a></td>
  </tr>
</table>

### Return Value

true if construction and <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>have</a> <a href='SkIRect_Reference#SkIRect'>area</a> <a href='SkIRect_Reference#SkIRect'>in</a> <a href='SkIRect_Reference#SkIRect'>common</a>

### Example

<div><fiddle-embed name="200422990eded2f754ab9893118f2645"><div>Two <a href='undocumented#SkDebugf'>SkDebugf</a> <a href='undocumented#SkDebugf'>calls</a> <a href='undocumented#SkDebugf'>are</a> <a href='undocumented#SkDebugf'>required</a>. <a href='undocumented#SkDebugf'>If</a> <a href='undocumented#SkDebugf'>the</a> <a href='undocumented#SkDebugf'>calls</a> <a href='undocumented#SkDebugf'>are</a> <a href='undocumented#SkDebugf'>combined</a>, <a href='undocumented#SkDebugf'>their</a> <a href='undocumented#SkDebugf'>arguments</a>
<a href='undocumented#SkDebugf'>may</a> <a href='undocumented#SkDebugf'>not</a> <a href='undocumented#SkDebugf'>be</a> <a href='undocumented#SkDebugf'>evaluated</a> <a href='undocumented#SkDebugf'>in</a> <a href='#SkIRect_intersect_3_left'>left</a> <a href='#SkIRect_intersect_3_left'>to</a> <a href='#SkIRect_intersect_3_right'>right</a> <a href='#SkIRect_intersect_3_right'>order</a>: <a href='#SkIRect_intersect_3_right'>the</a> <a href='#SkIRect_intersect_3_right'>printed</a> <a href='#SkIRect_intersect_3_right'>intersection</a> <a href='#SkIRect_intersect_3_right'>may</a>
<a href='#SkIRect_intersect_3_right'>be</a> <a href='#SkIRect_intersect_3_right'>before</a> <a href='#SkIRect_intersect_3_right'>or</a> <a href='#SkIRect_intersect_3_right'>after</a> <a href='#SkIRect_intersect_3_right'>the</a> <a href='#SkIRect_intersect_3_right'>call</a> <a href='#SkIRect_intersect_3_right'>to</a> <a href='#SkIRect_intersect_3_right'>intersect</a>.
</div>

#### Example Output

~~~~
intersection: 30, 60, 50, 80
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_intersectNoEmptyCheck'>intersectNoEmptyCheck</a> <a href='#SkIRect_Intersects'>Intersects</a> <a href='#SkIRect_join'>join</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_intersect'>intersect</a>

<a name='SkIRect_Intersects'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkIRect_Intersects'>Intersects</a>(<a href='#SkIRect_Intersects'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>a</a>, <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>b</a>)
</pre>

Returns true if <a href='#SkIRect_Intersects_a'>a</a> <a href='#SkIRect_Intersects_a'>intersects</a> <a href='#SkIRect_Intersects_b'>b</a>.
Returns false if either <a href='#SkIRect_Intersects_a'>a</a> <a href='#SkIRect_Intersects_a'>or</a> <a href='#SkIRect_Intersects_b'>b</a> <a href='#SkIRect_Intersects_b'>is</a> <a href='#SkIRect_Intersects_b'>empty</a>, <a href='#SkIRect_Intersects_b'>or</a> <a href='#SkIRect_Intersects_b'>do</a> <a href='#SkIRect_Intersects_b'>not</a> <a href='#SkIRect_Intersects_b'>intersect</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_Intersects_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>intersect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_Intersects_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>intersect</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkIRect_Intersects_a'>a</a> <a href='#SkIRect_Intersects_a'>and</a> <a href='#SkIRect_Intersects_b'>b</a> <a href='#SkIRect_Intersects_b'>have</a> <a href='#SkIRect_Intersects_b'>area</a> <a href='#SkIRect_Intersects_b'>in</a> <a href='#SkIRect_Intersects_b'>common</a>

### Example

<div><fiddle-embed name="0c67cf8981389efc7108369fb9b7976b">

#### Example Output

~~~~
intersection
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_IntersectsNoEmptyCheck'>IntersectsNoEmptyCheck</a> <a href='#SkIRect_intersect'>intersect</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_intersect'>intersect</a>

<a name='SkIRect_IntersectsNoEmptyCheck'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkIRect_IntersectsNoEmptyCheck'>IntersectsNoEmptyCheck</a>(<a href='#SkIRect_IntersectsNoEmptyCheck'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>a</a>, <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>b</a>)
</pre>

Returns true if <a href='#SkIRect_IntersectsNoEmptyCheck_a'>a</a> <a href='#SkIRect_IntersectsNoEmptyCheck_a'>intersects</a> <a href='#SkIRect_IntersectsNoEmptyCheck_b'>b</a>.
Asserts if either <a href='#SkIRect_IntersectsNoEmptyCheck_a'>a</a> <a href='#SkIRect_IntersectsNoEmptyCheck_a'>or</a> <a href='#SkIRect_IntersectsNoEmptyCheck_b'>b</a> <a href='#SkIRect_IntersectsNoEmptyCheck_b'>is</a> <a href='#SkIRect_IntersectsNoEmptyCheck_b'>empty</a>, <a href='#SkIRect_IntersectsNoEmptyCheck_b'>and</a> <a href='#SkIRect_IntersectsNoEmptyCheck_b'>if</a> <a href='#SkIRect_IntersectsNoEmptyCheck_b'>SK_DEBUG</a> <a href='#SkIRect_IntersectsNoEmptyCheck_b'>is</a> <a href='#SkIRect_IntersectsNoEmptyCheck_b'>defined</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_IntersectsNoEmptyCheck_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>intersect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_IntersectsNoEmptyCheck_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>intersect</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkIRect_IntersectsNoEmptyCheck_a'>a</a> <a href='#SkIRect_IntersectsNoEmptyCheck_a'>and</a> <a href='#SkIRect_IntersectsNoEmptyCheck_b'>b</a> <a href='#SkIRect_IntersectsNoEmptyCheck_b'>have</a> <a href='#SkIRect_IntersectsNoEmptyCheck_b'>area</a> <a href='#SkIRect_IntersectsNoEmptyCheck_b'>in</a> <a href='#SkIRect_IntersectsNoEmptyCheck_b'>common</a>

### Example

<div><fiddle-embed name="dba234d15162fb5b26e1a96529ca6a2a">

#### Example Output

~~~~
intersection
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_Intersects'>Intersects</a> <a href='#SkIRect_intersect'>intersect</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_intersect'>intersect</a>

<a name='Join'></a>

<a name='SkIRect_join'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void join(int32_t left, int32_t top, int32_t right, int32_t bottom)
</pre>

Constructs <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>intersect</a> <a href='SkIRect_Reference#SkIRect'>from</a> (<a href='#SkIRect_join_left'>left</a>, <a href='#SkIRect_join_top'>top</a>, <a href='#SkIRect_join_right'>right</a>, <a href='#SkIRect_join_bottom'>bottom</a>). <a href='#SkIRect_join_bottom'>Does</a> <a href='#SkIRect_join_bottom'>not</a> <a href='#SkIRect_join_bottom'>sort</a>
construction.

Sets <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>the</a> <a href='SkIRect_Reference#SkIRect'>union</a> <a href='SkIRect_Reference#SkIRect'>of</a> <a href='SkIRect_Reference#SkIRect'>itself</a> <a href='SkIRect_Reference#SkIRect'>and</a> <a href='SkIRect_Reference#SkIRect'>the</a> <a href='SkIRect_Reference#SkIRect'>construction</a>.

Has no effect if construction is empty. Otherwise, if <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>empty</a>, <a href='SkIRect_Reference#SkIRect'>sets</a>
<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>construction</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_join_left'><code><strong>left</strong></code></a></td>
    <td>x-axis minimum of constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_join_top'><code><strong>top</strong></code></a></td>
    <td>y-axis minimum of constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_join_right'><code><strong>right</strong></code></a></td>
    <td>x-axis maximum of constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a></td>
  </tr>
  <tr>    <td><a name='SkIRect_join_bottom'><code><strong>bottom</strong></code></a></td>
    <td>y-axis maximum of constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a></td>
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

<a href='#SkIRect_set'>set</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_join'>join</a>

<a name='SkIRect_join_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void join(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>r</a>)
</pre>

Sets <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>the</a> <a href='SkIRect_Reference#SkIRect'>union</a> <a href='SkIRect_Reference#SkIRect'>of</a> <a href='SkIRect_Reference#SkIRect'>itself</a> <a href='SkIRect_Reference#SkIRect'>and</a> <a href='#SkIRect_join_2_r'>r</a>.

Has no effect if <a href='#SkIRect_join_2_r'>r</a> <a href='#SkIRect_join_2_r'>is</a> <a href='#SkIRect_join_2_r'>empty</a>. <a href='#SkIRect_join_2_r'>Otherwise</a>, <a href='#SkIRect_join_2_r'>if</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>empty</a>, <a href='SkIRect_Reference#SkIRect'>sets</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='#SkIRect_join_2_r'>r</a>.

### Parameters

<table>  <tr>    <td><a name='SkIRect_join_2_r'><code><strong>r</strong></code></a></td>
    <td>expansion <a href='SkIRect_Reference#SkIRect'>SkIRect</a></td>
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

<a href='#SkIRect_set'>set</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_join'>join</a>

<a name='Sorting'></a>

<a name='SkIRect_sort'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkIRect_sort'>sort()</a>
</pre>

Swaps <a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_fLeft'>and</a> <a href='#SkIRect_fRight'>fRight</a> <a href='#SkIRect_fRight'>if</a> <a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_fLeft'>is</a> <a href='#SkIRect_fLeft'>greater</a> <a href='#SkIRect_fLeft'>than</a> <a href='#SkIRect_fRight'>fRight</a>; <a href='#SkIRect_fRight'>and</a> <a href='#SkIRect_fRight'>swaps</a>
<a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_fTop'>and</a> <a href='#SkIRect_fBottom'>fBottom</a> <a href='#SkIRect_fBottom'>if</a> <a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_fTop'>is</a> <a href='#SkIRect_fTop'>greater</a> <a href='#SkIRect_fTop'>than</a> <a href='#SkIRect_fBottom'>fBottom</a>. <a href='#SkIRect_fBottom'>Result</a> <a href='#SkIRect_fBottom'>may</a> <a href='#SkIRect_fBottom'>be</a> <a href='#SkIRect_fBottom'>empty</a>,
and <a href='#SkIRect_width'>width()</a> <a href='#SkIRect_width'>and</a> <a href='#SkIRect_height'>height()</a> <a href='#SkIRect_height'>will</a> <a href='#SkIRect_height'>be</a> <a href='#SkIRect_height'>zero</a> <a href='#SkIRect_height'>or</a> <a href='#SkIRect_height'>positive</a>.

### Example

<div><fiddle-embed name="fa12547fcfd4c1aef3db1a1f6aae0fe4">

#### Example Output

~~~~
rect: 30, 50, 20, 10
sorted: 20, 10, 30, 50
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_makeSorted'>makeSorted</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_sort'>sort</a>

<a name='SkIRect_makeSorted'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkIRect_makeSorted'>makeSorted</a>() <a href='#SkIRect_makeSorted'>const</a>
</pre>

Returns <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>with</a> <a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_fLeft'>and</a> <a href='#SkIRect_fRight'>fRight</a> <a href='#SkIRect_fRight'>swapped</a> <a href='#SkIRect_fRight'>if</a> <a href='#SkIRect_fLeft'>fLeft</a> <a href='#SkIRect_fLeft'>is</a> <a href='#SkIRect_fLeft'>greater</a> <a href='#SkIRect_fLeft'>than</a> <a href='#SkIRect_fRight'>fRight</a>; <a href='#SkIRect_fRight'>and</a>
with <a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_fTop'>and</a> <a href='#SkIRect_fBottom'>fBottom</a> <a href='#SkIRect_fBottom'>swapped</a> <a href='#SkIRect_fBottom'>if</a> <a href='#SkIRect_fTop'>fTop</a> <a href='#SkIRect_fTop'>is</a> <a href='#SkIRect_fTop'>greater</a> <a href='#SkIRect_fTop'>than</a> <a href='#SkIRect_fBottom'>fBottom</a>. <a href='#SkIRect_fBottom'>Result</a> <a href='#SkIRect_fBottom'>may</a> <a href='#SkIRect_fBottom'>be</a> <a href='#SkIRect_fBottom'>empty</a>;
and <a href='#SkIRect_width'>width()</a> <a href='#SkIRect_width'>and</a> <a href='#SkIRect_height'>height()</a> <a href='#SkIRect_height'>will</a> <a href='#SkIRect_height'>be</a> <a href='#SkIRect_height'>zero</a> <a href='#SkIRect_height'>or</a> <a href='#SkIRect_height'>positive</a>.

### Return Value

sorted <a href='SkIRect_Reference#SkIRect'>SkIRect</a>

### Example

<div><fiddle-embed name="de89926c374aa16427916900b89a3441">

#### Example Output

~~~~
rect: 30, 50, 20, 10
sorted: 20, 10, 30, 50
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_sort'>sort</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_makeSorted'>makeSorted</a>

<a name='SkIRect_EmptyIRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='#SkIRect_EmptyIRect'>EmptyIRect</a>()
</pre>

Returns a reference to immutable empty <a href='SkIRect_Reference#SkIRect'>SkIRect</a>, <a href='SkIRect_Reference#SkIRect'>set</a> <a href='SkIRect_Reference#SkIRect'>to</a> (0, 0, 0, 0).

### Return Value

global <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>set</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>all</a> <a href='SkIRect_Reference#SkIRect'>zeroes</a>

### Example

<div><fiddle-embed name="65e0b9b52e907902630577941fb3ed6d">

#### Example Output

~~~~
rect: 0, 0, 0, 0
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkIRect_MakeEmpty'>MakeEmpty</a>

