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

    <a href='#SkRect_fBottom'>static</a> <a href='#SkRect_fBottom'>constexpr</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_MakeEmpty'>MakeEmpty</a>();
    <a href='#SkRect_MakeEmpty'>static</a> <a href='#SkRect_MakeEmpty'>constexpr</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_MakeWH'>MakeWH</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>w</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>h</a>);
    <a href='undocumented#SkScalar'>static</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_MakeIWH'>MakeIWH</a>(<a href='#SkRect_MakeIWH'>int</a> <a href='#SkRect_MakeIWH'>w</a>, <a href='#SkRect_MakeIWH'>int</a> <a href='#SkRect_MakeIWH'>h</a>);
    <a href='#SkRect_MakeIWH'>static</a> <a href='#SkRect_MakeIWH'>constexpr</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_MakeSize'>MakeSize</a>(<a href='#SkRect_MakeSize'>const</a> <a href='undocumented#SkSize'>SkSize</a>& <a href='undocumented#Size'>size</a>);
    <a href='undocumented#Size'>static</a> <a href='undocumented#Size'>constexpr</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_MakeLTRB'>MakeLTRB</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>l</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>t</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>r</a>,
                                     <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>b</a>);
    <a href='undocumented#SkScalar'>static</a> <a href='undocumented#SkScalar'>constexpr</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_MakeXYWH'>MakeXYWH</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>w</a>,
                                     <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>h</a>);
    <a href='undocumented#SkScalar'>static</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_Make'>Make</a>(<a href='#SkRect_Make'>const</a> <a href='undocumented#SkISize'>SkISize</a>& <a href='undocumented#Size'>size</a>);
    <a href='undocumented#Size'>static</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_Make'>Make</a>(<a href='#SkRect_Make'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>irect</a>);
    <a href='SkIRect_Reference#SkIRect'>bool</a> <a href='#SkRect_isEmpty'>isEmpty</a>() <a href='#SkRect_isEmpty'>const</a>;
    <a href='#SkRect_isEmpty'>bool</a> <a href='#SkRect_isSorted'>isSorted</a>() <a href='#SkRect_isSorted'>const</a>;
    <a href='#SkRect_isSorted'>bool</a> <a href='#SkRect_isFinite'>isFinite</a>() <a href='#SkRect_isFinite'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRect_x'>x()</a> <a href='#SkRect_x'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRect_y'>y()</a> <a href='#SkRect_y'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRect_left'>left()</a> <a href='#SkRect_left'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRect_top'>top()</a> <a href='#SkRect_top'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRect_right'>right()</a> <a href='#SkRect_right'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRect_bottom'>bottom()</a> <a href='#SkRect_bottom'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRect_width'>width()</a> <a href='#SkRect_width'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRect_height'>height()</a> <a href='#SkRect_height'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRect_centerX'>centerX</a>() <a href='#SkRect_centerX'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkRect_centerY'>centerY</a>() <a href='#SkRect_centerY'>const</a>;
    <a href='#SkRect_centerY'>friend</a> <a href='#SkRect_centerY'>bool</a> <a href='#SkRect_centerY'>operator</a>==(<a href='#SkRect_centerY'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>a</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>b</a>);
    <a href='SkRect_Reference#SkRect'>friend</a> <a href='SkRect_Reference#SkRect'>bool</a> <a href='SkRect_Reference#SkRect'>operator</a>!=(<a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>a</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>b</a>);
    <a href='SkRect_Reference#SkRect'>void</a> <a href='#SkRect_toQuad'>toQuad</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPath_Reference#Quad'>quad</a>[4]) <a href='SkPath_Reference#Quad'>const</a>;
    <a href='SkPath_Reference#Quad'>void</a> <a href='#SkRect_setEmpty'>setEmpty</a>();
    <a href='#SkRect_setEmpty'>void</a> <a href='#SkRect_setEmpty'>set</a>(<a href='#SkRect_setEmpty'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>src</a>);
    <a href='SkIRect_Reference#SkIRect'>void</a> <a href='SkIRect_Reference#SkIRect'>set</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>left</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>top</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>right</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>bottom</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkRect_setLTRB'>setLTRB</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>left</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>top</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>right</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>bottom</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkRect_iset'>iset</a>(<a href='#SkRect_iset'>int</a> <a href='#SkRect_iset'>left</a>, <a href='#SkRect_iset'>int</a> <a href='#SkRect_iset'>top</a>, <a href='#SkRect_iset'>int</a> <a href='#SkRect_iset'>right</a>, <a href='#SkRect_iset'>int</a> <a href='#SkRect_iset'>bottom</a>);
    <a href='#SkRect_iset'>void</a> <a href='#SkRect_isetWH'>isetWH</a>(<a href='#SkRect_isetWH'>int</a> <a href='#SkRect_isetWH'>width</a>, <a href='#SkRect_isetWH'>int</a> <a href='#SkRect_isetWH'>height</a>);
    <a href='#SkRect_isetWH'>void</a> <a href='#SkRect_isetWH'>set</a>(<a href='#SkRect_isetWH'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pts</a>[], <a href='SkPoint_Reference#SkPoint'>int</a> <a href='SkPoint_Reference#SkPoint'>count</a>);
    <a href='SkPoint_Reference#SkPoint'>void</a> <a href='#SkRect_setBounds'>setBounds</a>(<a href='#SkRect_setBounds'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pts</a>[], <a href='SkPoint_Reference#SkPoint'>int</a> <a href='SkPoint_Reference#SkPoint'>count</a>);
    <a href='SkPoint_Reference#SkPoint'>bool</a> <a href='#SkRect_setBoundsCheck'>setBoundsCheck</a>(<a href='#SkRect_setBoundsCheck'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pts</a>[], <a href='SkPoint_Reference#SkPoint'>int</a> <a href='SkPoint_Reference#SkPoint'>count</a>);
    <a href='SkPoint_Reference#SkPoint'>void</a> <a href='#SkRect_setBoundsNoCheck'>setBoundsNoCheck</a>(<a href='#SkRect_setBoundsNoCheck'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pts</a>[], <a href='SkPoint_Reference#SkPoint'>int</a> <a href='SkPoint_Reference#SkPoint'>count</a>);
    <a href='SkPoint_Reference#SkPoint'>void</a> <a href='SkPoint_Reference#SkPoint'>set</a>(<a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p0</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p1</a>);
    <a href='SkPoint_Reference#SkPoint'>void</a> <a href='#SkRect_setXYWH'>setXYWH</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>width</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>height</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkRect_setWH'>setWH</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>width</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>height</a>);
    <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_makeOffset'>makeOffset</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>) <a href='undocumented#SkScalar'>const</a>;
    <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_makeInset'>makeInset</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>) <a href='undocumented#SkScalar'>const</a>;
    <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_makeOutset'>makeOutset</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>) <a href='undocumented#SkScalar'>const</a>;
    <a href='undocumented#SkScalar'>void</a> <a href='undocumented#SkScalar'>offset</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='undocumented#SkScalar'>offset</a>(<a href='undocumented#SkScalar'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>delta</a>);
    <a href='SkPoint_Reference#SkPoint'>void</a> <a href='#SkRect_offsetTo'>offsetTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>newX</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>newY</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='undocumented#SkScalar'>inset</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='undocumented#SkScalar'>outset</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>);
    <a href='undocumented#SkScalar'>bool</a> <a href='undocumented#SkScalar'>intersect</a>(<a href='undocumented#SkScalar'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>r</a>);
    <a href='SkRect_Reference#SkRect'>bool</a> <a href='SkRect_Reference#SkRect'>intersect</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>left</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>top</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>right</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>bottom</a>);
    <a href='undocumented#SkScalar'>bool</a> <a href='undocumented#SkScalar'>intersect</a>(<a href='undocumented#SkScalar'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>a</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>b</a>);
    <a href='SkRect_Reference#SkRect'>bool</a> <a href='SkRect_Reference#SkRect'>intersects</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>left</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>top</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>right</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>bottom</a>) <a href='undocumented#SkScalar'>const</a>;
    <a href='undocumented#SkScalar'>bool</a> <a href='undocumented#SkScalar'>intersects</a>(<a href='undocumented#SkScalar'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>r</a>) <a href='SkRect_Reference#SkRect'>const</a>;
    <a href='SkRect_Reference#SkRect'>static</a> <a href='SkRect_Reference#SkRect'>bool</a> <a href='#SkRect_Intersects'>Intersects</a>(<a href='#SkRect_Intersects'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>a</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>b</a>);
    <a href='SkRect_Reference#SkRect'>void</a> <a href='SkRect_Reference#SkRect'>join</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>left</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>top</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>right</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>bottom</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='undocumented#SkScalar'>join</a>(<a href='undocumented#SkScalar'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>r</a>);
    <a href='SkRect_Reference#SkRect'>void</a> <a href='#SkRect_joinNonEmptyArg'>joinNonEmptyArg</a>(<a href='#SkRect_joinNonEmptyArg'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>r</a>);
    <a href='SkRect_Reference#SkRect'>void</a> <a href='#SkRect_joinPossiblyEmptyRect'>joinPossiblyEmptyRect</a>(<a href='#SkRect_joinPossiblyEmptyRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>r</a>);
    <a href='SkRect_Reference#SkRect'>bool</a> <a href='SkRect_Reference#SkRect'>contains</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>) <a href='undocumented#SkScalar'>const</a>;
    <a href='undocumented#SkScalar'>bool</a> <a href='undocumented#SkScalar'>contains</a>(<a href='undocumented#SkScalar'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>r</a>) <a href='SkRect_Reference#SkRect'>const</a>;
    <a href='SkRect_Reference#SkRect'>bool</a> <a href='SkRect_Reference#SkRect'>contains</a>(<a href='SkRect_Reference#SkRect'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>r</a>) <a href='SkIRect_Reference#SkIRect'>const</a>;
    <a href='SkIRect_Reference#SkIRect'>void</a> <a href='SkIRect_Reference#SkIRect'>round</a>(<a href='SkIRect_Reference#SkIRect'>SkIRect</a>* <a href='SkIRect_Reference#SkIRect'>dst</a>) <a href='SkIRect_Reference#SkIRect'>const</a>;
    <a href='SkIRect_Reference#SkIRect'>void</a> <a href='#SkRect_roundOut'>roundOut</a>(<a href='SkIRect_Reference#SkIRect'>SkIRect</a>* <a href='SkIRect_Reference#SkIRect'>dst</a>) <a href='SkIRect_Reference#SkIRect'>const</a>;
    <a href='SkIRect_Reference#SkIRect'>void</a> <a href='#SkRect_roundOut'>roundOut</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>dst</a>) <a href='SkRect_Reference#SkRect'>const</a>;
    <a href='SkRect_Reference#SkRect'>void</a> <a href='#SkRect_roundIn'>roundIn</a>(<a href='SkIRect_Reference#SkIRect'>SkIRect</a>* <a href='SkIRect_Reference#SkIRect'>dst</a>) <a href='SkIRect_Reference#SkIRect'>const</a>;
    <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkRect_round'>round()</a> <a href='#SkRect_round'>const</a>;
    <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkRect_roundOut'>roundOut</a>() <a href='#SkRect_roundOut'>const</a>;
    <a href='#SkRect_roundOut'>void</a> <a href='#SkRect_sort'>sort()</a>;
    <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_makeSorted'>makeSorted</a>() <a href='#SkRect_makeSorted'>const</a>;
    <a href='#SkRect_makeSorted'>const</a> <a href='undocumented#SkScalar'>SkScalar</a>* <a href='#SkRect_asScalars'>asScalars</a>() <a href='#SkRect_asScalars'>const</a>;
    <a href='#SkRect_asScalars'>void</a> <a href='#SkRect_dump'>dump</a>(<a href='#SkRect_dump'>bool</a> <a href='#SkRect_dump'>asHex</a>) <a href='#SkRect_dump'>const</a>;
    <a href='#SkRect_dump'>void</a> <a href='#SkRect_dump'>dump()</a> <a href='#SkRect_dump'>const</a>;
    <a href='#SkRect_dump'>void</a> <a href='#SkRect_dumpHex'>dumpHex</a>() <a href='#SkRect_dumpHex'>const</a>;
};
</pre>

<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>holds</a> <a href='SkRect_Reference#SkRect'>four</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>coordinates</a> <a href='undocumented#SkScalar'>describing</a> <a href='undocumented#SkScalar'>the</a> <a href='undocumented#SkScalar'>upper</a> <a href='undocumented#SkScalar'>and</a>
<a href='undocumented#SkScalar'>lower</a> <a href='undocumented#SkScalar'>bounds</a> <a href='undocumented#SkScalar'>of</a> <a href='undocumented#SkScalar'>a</a> <a href='undocumented#SkScalar'>rectangle</a>. <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>may</a> <a href='SkRect_Reference#SkRect'>be</a> <a href='SkRect_Reference#SkRect'>created</a> <a href='SkRect_Reference#SkRect'>from</a> <a href='SkRect_Reference#SkRect'>outer</a> <a href='SkRect_Reference#SkRect'>bounds</a> <a href='SkRect_Reference#SkRect'>or</a>
<a href='SkRect_Reference#SkRect'>from</a> <a href='SkRect_Reference#SkRect'>position</a>, <a href='SkRect_Reference#SkRect'>width</a>, <a href='SkRect_Reference#SkRect'>and</a> <a href='SkRect_Reference#SkRect'>height</a>. <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>describes</a> <a href='SkRect_Reference#SkRect'>an</a> <a href='SkRect_Reference#SkRect'>area</a>; <a href='SkRect_Reference#SkRect'>if</a> <a href='SkRect_Reference#SkRect'>its</a> <a href='SkRect_Reference#SkRect'>right</a>
<a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>less</a> <a href='SkRect_Reference#SkRect'>than</a> <a href='SkRect_Reference#SkRect'>or</a> <a href='SkRect_Reference#SkRect'>equal</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>its</a> <a href='SkRect_Reference#SkRect'>left</a>, <a href='SkRect_Reference#SkRect'>or</a> <a href='SkRect_Reference#SkRect'>if</a> <a href='SkRect_Reference#SkRect'>its</a> <a href='SkRect_Reference#SkRect'>bottom</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>less</a> <a href='SkRect_Reference#SkRect'>than</a> <a href='SkRect_Reference#SkRect'>or</a> <a href='SkRect_Reference#SkRect'>equal</a> <a href='SkRect_Reference#SkRect'>to</a>
<a href='SkRect_Reference#SkRect'>its</a> <a href='SkRect_Reference#SkRect'>top</a>, <a href='SkRect_Reference#SkRect'>it</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>considered</a> <a href='SkRect_Reference#SkRect'>empty</a>.

<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>can</a> <a href='SkRect_Reference#SkRect'>be</a> <a href='SkRect_Reference#SkRect'>constructed</a> <a href='SkRect_Reference#SkRect'>from</a> <a href='SkRect_Reference#SkRect'>int</a> <a href='SkRect_Reference#SkRect'>values</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>avoid</a> <a href='SkRect_Reference#SkRect'>compiler</a> <a href='SkRect_Reference#SkRect'>warnings</a> <a href='SkRect_Reference#SkRect'>that</a>
<a href='SkRect_Reference#SkRect'>integer</a> <a href='SkRect_Reference#SkRect'>input</a> <a href='SkRect_Reference#SkRect'>cannot</a> <a href='SkRect_Reference#SkRect'>convert</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>without</a> <a href='undocumented#SkScalar'>loss</a> <a href='undocumented#SkScalar'>of</a> <a href='undocumented#SkScalar'>precision</a>.<table style='border-collapse: collapse; width: 62.5em'>

  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Type</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Member</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRect_fLeft'><code>fLeft</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May contain any value, including infinities and NaN. The smaller of the
horizontal values when sorted. When equal to or greater than <a href='#SkRect_fRight'>fRight</a>, <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>is</a> <a href='SkRect_Reference#Rect'>empty</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRect_fTop'><code>fTop</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May contain any value, including infinities and NaN. The smaller of the
vertical values when sorted. When equal to or greater than <a href='#SkRect_fBottom'>fBottom</a>, <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>is</a> <a href='SkRect_Reference#Rect'>empty</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRect_fRight'><code>fRight</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May contain any value, including infinities and NaN. The larger of the
horizontal values when sorted. When equal to or less than <a href='#SkRect_fLeft'>fLeft</a>, <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>is</a> <a href='SkRect_Reference#Rect'>empty</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>SkScalar</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRect_fBottom'><code>fBottom</code></a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May contain any value, including infinities and NaN. The larger of the
vertical values when sorted. When equal to or less than <a href='#SkRect_fTop'>fTop</a>, <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>is</a> <a href='SkRect_Reference#Rect'>empty</a>.
</td>
  </tr>
</table>

<a name='SkRect_MakeEmpty'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static constexpr <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_MakeEmpty'>MakeEmpty</a>()
</pre>

Returns constructed <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>set</a> <a href='SkRect_Reference#SkRect'>to</a> (0, 0, 0, 0).
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
static constexpr <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_MakeWH'>MakeWH</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>w</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>h</a>)
</pre>

Returns constructed <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>set</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>values</a> (0, 0, <a href='#SkRect_MakeWH_w'>w</a>, <a href='#SkRect_MakeWH_h'>h</a>). <a href='#SkRect_MakeWH_h'>Does</a> <a href='#SkRect_MakeWH_h'>not</a>
validate input; <a href='#SkRect_MakeWH_w'>w</a> <a href='#SkRect_MakeWH_w'>or</a> <a href='#SkRect_MakeWH_h'>h</a> <a href='#SkRect_MakeWH_h'>may</a> <a href='#SkRect_MakeWH_h'>be</a> <a href='#SkRect_MakeWH_h'>negative</a>.

Passing integer values may generate a compiler warning since <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>cannot</a>
represent 32-bit integers exactly. Use <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>for</a> <a href='SkIRect_Reference#SkIRect'>an</a> <a href='SkIRect_Reference#SkIRect'>exact</a> <a href='SkIRect_Reference#SkIRect'>integer</a> <a href='SkIRect_Reference#SkIRect'>rectangle</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_MakeWH_w'><code><strong>w</strong></code></a></td>
    <td><a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>width</a> <a href='undocumented#SkScalar'>of</a> <a href='undocumented#SkScalar'>constructed</a> <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
  <tr>    <td><a name='SkRect_MakeWH_h'><code><strong>h</strong></code></a></td>
    <td><a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>height</a> <a href='undocumented#SkScalar'>of</a> <a href='undocumented#SkScalar'>constructed</a> <a href='SkRect_Reference#SkRect'>SkRect</a></td>
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
static <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_MakeIWH'>MakeIWH</a>(<a href='#SkRect_MakeIWH'>int</a> <a href='#SkRect_MakeIWH'>w</a>, <a href='#SkRect_MakeIWH'>int</a> <a href='#SkRect_MakeIWH'>h</a>)
</pre>

Returns constructed <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>set</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>integer</a> <a href='SkRect_Reference#SkRect'>values</a> (0, 0, <a href='#SkRect_MakeIWH_w'>w</a>, <a href='#SkRect_MakeIWH_h'>h</a>). <a href='#SkRect_MakeIWH_h'>Does</a> <a href='#SkRect_MakeIWH_h'>not</a> <a href='#SkRect_MakeIWH_h'>validate</a>
input; <a href='#SkRect_MakeIWH_w'>w</a> <a href='#SkRect_MakeIWH_w'>or</a> <a href='#SkRect_MakeIWH_h'>h</a> <a href='#SkRect_MakeIWH_h'>may</a> <a href='#SkRect_MakeIWH_h'>be</a> <a href='#SkRect_MakeIWH_h'>negative</a>.

Use to avoid a compiler warning that input may lose precision when stored.
Use <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>for</a> <a href='SkIRect_Reference#SkIRect'>an</a> <a href='SkIRect_Reference#SkIRect'>exact</a> <a href='SkIRect_Reference#SkIRect'>integer</a> <a href='SkIRect_Reference#SkIRect'>rectangle</a>.

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
static constexpr <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_MakeSize'>MakeSize</a>(<a href='#SkRect_MakeSize'>const</a> <a href='undocumented#SkSize'>SkSize</a>& <a href='undocumented#Size'>size</a>)
</pre>

Returns constructed <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>set</a> <a href='SkRect_Reference#SkRect'>to</a> (0, 0, <a href='#SkRect_MakeSize_size'>size</a>.<a href='#SkSize_width'>width()</a>, <a href='#SkRect_MakeSize_size'>size</a>.<a href='#SkSize_height'>height()</a>). <a href='#SkSize_height'>Does</a> <a href='#SkSize_height'>not</a>
validate input; <a href='#SkRect_MakeSize_size'>size</a>.<a href='#SkSize_width'>width()</a> <a href='#SkSize_width'>or</a> <a href='#SkRect_MakeSize_size'>size</a>.<a href='#SkSize_height'>height()</a> <a href='#SkSize_height'>may</a> <a href='#SkSize_height'>be</a> <a href='#SkSize_height'>negative</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_MakeSize_size'><code><strong>size</strong></code></a></td>
    <td><a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>values</a> <a href='undocumented#SkScalar'>for</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>width</a> <a href='SkRect_Reference#SkRect'>and</a> <a href='SkRect_Reference#SkRect'>height</a></td>
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
static constexpr <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_MakeLTRB'>MakeLTRB</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>l</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>t</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>r</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>b</a>)
</pre>

Returns constructed <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>set</a> <a href='SkRect_Reference#SkRect'>to</a> (<a href='#SkRect_MakeLTRB_l'>l</a>, <a href='#SkRect_MakeLTRB_t'>t</a>, <a href='#SkRect_MakeLTRB_r'>r</a>, <a href='#SkRect_MakeLTRB_b'>b</a>). <a href='#SkRect_MakeLTRB_b'>Does</a> <a href='#SkRect_MakeLTRB_b'>not</a> <a href='#SkRect_MakeLTRB_b'>sort</a> <a href='#SkRect_MakeLTRB_b'>input</a>; <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>may</a>
result in <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>greater</a> <a href='#SkRect_fLeft'>than</a> <a href='#SkRect_fRight'>fRight</a>, <a href='#SkRect_fRight'>or</a> <a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>greater</a> <a href='#SkRect_fTop'>than</a> <a href='#SkRect_fBottom'>fBottom</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_MakeLTRB_l'><code><strong>l</strong></code></a></td>
    <td><a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>stored</a> <a href='undocumented#SkScalar'>in</a> <a href='#SkRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkRect_MakeLTRB_t'><code><strong>t</strong></code></a></td>
    <td><a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>stored</a> <a href='undocumented#SkScalar'>in</a> <a href='#SkRect_fTop'>fTop</a></td>
  </tr>
  <tr>    <td><a name='SkRect_MakeLTRB_r'><code><strong>r</strong></code></a></td>
    <td><a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>stored</a> <a href='undocumented#SkScalar'>in</a> <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_MakeLTRB_b'><code><strong>b</strong></code></a></td>
    <td><a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>stored</a> <a href='undocumented#SkScalar'>in</a> <a href='#SkRect_fBottom'>fBottom</a></td>
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
static constexpr <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_MakeXYWH'>MakeXYWH</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>w</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>h</a>)
</pre>

Returns constructed <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>set</a> <a href='SkRect_Reference#Rect'>to</a> <code>(<a href='#SkRect_MakeXYWH_x'>x</a>, <a href='#SkRect_MakeXYWH_y'>y</a>, <a href='#SkRect_MakeXYWH_x'>x</a> + <a href='#SkRect_MakeXYWH_w'>w</a>, <a href='#SkRect_MakeXYWH_y'>y</a> + <a href='#SkRect_MakeXYWH_h'>h</a>)</code>.
Does not validate input; <a href='#SkRect_MakeXYWH_w'>w</a> <a href='#SkRect_MakeXYWH_w'>or</a> <a href='#SkRect_MakeXYWH_h'>h</a> <a href='#SkRect_MakeXYWH_h'>may</a> <a href='#SkRect_MakeXYWH_h'>be</a> <a href='#SkRect_MakeXYWH_h'>negative</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_MakeXYWH_x'><code><strong>x</strong></code></a></td>
    <td>stored in <a href='#SkRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkRect_MakeXYWH_y'><code><strong>y</strong></code></a></td>
    <td>stored in <a href='#SkRect_fTop'>fTop</a></td>
  </tr>
  <tr>    <td><a name='SkRect_MakeXYWH_w'><code><strong>w</strong></code></a></td>
    <td>added to <a href='#SkRect_MakeXYWH_x'>x</a> <a href='#SkRect_MakeXYWH_x'>and</a> <a href='#SkRect_MakeXYWH_x'>stored</a> <a href='#SkRect_MakeXYWH_x'>in</a> <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_MakeXYWH_h'><code><strong>h</strong></code></a></td>
    <td>added to <a href='#SkRect_MakeXYWH_y'>y</a> <a href='#SkRect_MakeXYWH_y'>and</a> <a href='#SkRect_MakeXYWH_y'>stored</a> <a href='#SkRect_MakeXYWH_y'>in</a> <a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

bounds at (<a href='#SkRect_MakeXYWH_x'>x</a>, <a href='#SkRect_MakeXYWH_y'>y</a>) <a href='#SkRect_MakeXYWH_y'>with</a> <a href='#SkRect_MakeXYWH_y'>width</a> <a href='#SkRect_MakeXYWH_w'>w</a> <a href='#SkRect_MakeXYWH_w'>and</a> <a href='#SkRect_MakeXYWH_w'>height</a> <a href='#SkRect_MakeXYWH_h'>h</a>

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
static <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_Make'>Make</a>(<a href='#SkRect_Make'>const</a> <a href='undocumented#SkISize'>SkISize</a>& <a href='undocumented#Size'>size</a>)
</pre>

Returns constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>set</a> <a href='SkIRect_Reference#SkIRect'>to</a> (0, 0, <a href='#SkRect_Make_size'>size</a>.<a href='#SkISize_width'>width()</a>, <a href='#SkRect_Make_size'>size</a>.<a href='#SkISize_height'>height()</a>).
Does not validate input; <a href='#SkRect_Make_size'>size</a>.<a href='#SkISize_width'>width()</a> <a href='#SkISize_width'>or</a> <a href='#SkRect_Make_size'>size</a>.<a href='#SkISize_height'>height()</a> <a href='#SkISize_height'>may</a> <a href='#SkISize_height'>be</a> <a href='#SkISize_height'>negative</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_Make_size'><code><strong>size</strong></code></a></td>
    <td>integer values for <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>width</a> <a href='SkRect_Reference#SkRect'>and</a> <a href='SkRect_Reference#SkRect'>height</a></td>
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
static <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_Make'>Make</a>(<a href='#SkRect_Make'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>irect</a>)
</pre>

Returns constructed <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>set</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='#SkRect_Make_2_irect'>irect</a>, <a href='#SkRect_Make_2_irect'>promoting</a> <a href='#SkRect_Make_2_irect'>integers</a> <a href='#SkRect_Make_2_irect'>to</a> <a href='undocumented#Scalar'>scalar</a>.
Does not validate input; <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>may</a> <a href='#SkRect_fLeft'>be</a> <a href='#SkRect_fLeft'>greater</a> <a href='#SkRect_fLeft'>than</a> <a href='#SkRect_fRight'>fRight</a>, <a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>may</a> <a href='#SkRect_fTop'>be</a> <a href='#SkRect_fTop'>greater</a>
than <a href='#SkRect_fBottom'>fBottom</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_Make_2_irect'><code><strong>irect</strong></code></a></td>
    <td>integer unsorted bounds</td>
  </tr>
</table>

### Return Value

<a href='#SkRect_Make_2_irect'>irect</a> <a href='#SkRect_Make_2_irect'>members</a> <a href='#SkRect_Make_2_irect'>converted</a> <a href='#SkRect_Make_2_irect'>to</a> <a href='undocumented#SkScalar'>SkScalar</a>

### Example

<div><fiddle-embed name="dd801faa1e60a0fe9e0657674461e063"></fiddle-embed></div>

### See Also

<a href='#SkRect_MakeLTRB'>MakeLTRB</a>

<a name='Property'></a>

<a name='SkRect_isEmpty'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRect_isEmpty'>isEmpty</a>() <a href='#SkRect_isEmpty'>const</a>
</pre>

Returns true if <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>is</a> <a href='#SkRect_fLeft'>equal</a> <a href='#SkRect_fLeft'>to</a> <a href='#SkRect_fLeft'>or</a> <a href='#SkRect_fLeft'>greater</a> <a href='#SkRect_fLeft'>than</a> <a href='#SkRect_fRight'>fRight</a>, <a href='#SkRect_fRight'>or</a> <a href='#SkRect_fRight'>if</a> <a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>is</a> <a href='#SkRect_fTop'>equal</a>
to or greater than <a href='#SkRect_fBottom'>fBottom</a>. <a href='#SkRect_fBottom'>Call</a> <a href='#SkRect_sort'>sort()</a> <a href='#SkRect_sort'>to</a> <a href='#SkRect_sort'>reverse</a> <a href='#SkRect_sort'>rectangles</a> <a href='#SkRect_sort'>with</a> <a href='#SkRect_sort'>negative</a>
<a href='#SkRect_width'>width()</a> <a href='#SkRect_width'>or</a> <a href='#SkRect_height'>height()</a>.

### Return Value

true if <a href='#SkRect_width'>width()</a> <a href='#SkRect_width'>or</a> <a href='#SkRect_height'>height()</a> <a href='#SkRect_height'>are</a> <a href='#SkRect_height'>zero</a> <a href='#SkRect_height'>or</a> <a href='#SkRect_height'>negative</a>

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
bool <a href='#SkRect_isSorted'>isSorted</a>() <a href='#SkRect_isSorted'>const</a>
</pre>

Returns true if <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>is</a> <a href='#SkRect_fLeft'>equal</a> <a href='#SkRect_fLeft'>to</a> <a href='#SkRect_fLeft'>or</a> <a href='#SkRect_fLeft'>less</a> <a href='#SkRect_fLeft'>than</a> <a href='#SkRect_fRight'>fRight</a>, <a href='#SkRect_fRight'>or</a> <a href='#SkRect_fRight'>if</a> <a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>is</a> <a href='#SkRect_fTop'>equal</a>
to or less than <a href='#SkRect_fBottom'>fBottom</a>. <a href='#SkRect_fBottom'>Call</a> <a href='#SkRect_sort'>sort()</a> <a href='#SkRect_sort'>to</a> <a href='#SkRect_sort'>reverse</a> <a href='#SkRect_sort'>rectangles</a> <a href='#SkRect_sort'>with</a> <a href='#SkRect_sort'>negative</a>
<a href='#SkRect_width'>width()</a> <a href='#SkRect_width'>or</a> <a href='#SkRect_height'>height()</a>.

### Return Value

true if <a href='#SkRect_width'>width()</a> <a href='#SkRect_width'>or</a> <a href='#SkRect_height'>height()</a> <a href='#SkRect_height'>are</a> <a href='#SkRect_height'>zero</a> <a href='#SkRect_height'>or</a> <a href='#SkRect_height'>positive</a>

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
bool <a href='#SkRect_isFinite'>isFinite</a>() <a href='#SkRect_isFinite'>const</a>
</pre>

Returns true if all values in the rectangle are finite: <a href='undocumented#SK_ScalarMin'>SK_ScalarMin</a> <a href='undocumented#SK_ScalarMin'>or</a> <a href='undocumented#SK_ScalarMin'>larger</a>,
and <a href='undocumented#SK_ScalarMax'>SK_ScalarMax</a> <a href='undocumented#SK_ScalarMax'>or</a> <a href='undocumented#SK_ScalarMax'>smaller</a>.

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
<a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkRect_x'>x()</a> <a href='#SkRect_x'>const</a>
</pre>

Returns left edge of <a href='SkRect_Reference#SkRect'>SkRect</a>, <a href='SkRect_Reference#SkRect'>if</a> <a href='SkRect_Reference#SkRect'>sorted</a>. <a href='SkRect_Reference#SkRect'>Call</a> <a href='#SkRect_isSorted'>isSorted</a>() <a href='#SkRect_isSorted'>to</a> <a href='#SkRect_isSorted'>see</a> <a href='#SkRect_isSorted'>if</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>valid</a>.
Call <a href='#SkRect_sort'>sort()</a> <a href='#SkRect_sort'>to</a> <a href='#SkRect_sort'>reverse</a> <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>and</a> <a href='#SkRect_fRight'>fRight</a> <a href='#SkRect_fRight'>if</a> <a href='#SkRect_fRight'>needed</a>.

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
<a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkRect_y'>y()</a> <a href='#SkRect_y'>const</a>
</pre>

Returns top edge of <a href='SkRect_Reference#SkRect'>SkRect</a>, <a href='SkRect_Reference#SkRect'>if</a> <a href='SkRect_Reference#SkRect'>sorted</a>. <a href='SkRect_Reference#SkRect'>Call</a> <a href='#SkRect_isEmpty'>isEmpty</a>() <a href='#SkRect_isEmpty'>to</a> <a href='#SkRect_isEmpty'>see</a> <a href='#SkRect_isEmpty'>if</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>may</a> <a href='SkRect_Reference#SkRect'>be</a> <a href='SkRect_Reference#SkRect'>invalid</a>,
and <a href='#SkRect_sort'>sort()</a> <a href='#SkRect_sort'>to</a> <a href='#SkRect_sort'>reverse</a> <a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>and</a> <a href='#SkRect_fBottom'>fBottom</a> <a href='#SkRect_fBottom'>if</a> <a href='#SkRect_fBottom'>needed</a>.

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
<a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkRect_left'>left()</a> <a href='#SkRect_left'>const</a>
</pre>

Returns left edge of <a href='SkRect_Reference#SkRect'>SkRect</a>, <a href='SkRect_Reference#SkRect'>if</a> <a href='SkRect_Reference#SkRect'>sorted</a>. <a href='SkRect_Reference#SkRect'>Call</a> <a href='#SkRect_isSorted'>isSorted</a>() <a href='#SkRect_isSorted'>to</a> <a href='#SkRect_isSorted'>see</a> <a href='#SkRect_isSorted'>if</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>valid</a>.
Call <a href='#SkRect_sort'>sort()</a> <a href='#SkRect_sort'>to</a> <a href='#SkRect_sort'>reverse</a> <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>and</a> <a href='#SkRect_fRight'>fRight</a> <a href='#SkRect_fRight'>if</a> <a href='#SkRect_fRight'>needed</a>.

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
<a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkRect_top'>top()</a> <a href='#SkRect_top'>const</a>
</pre>

Returns top edge of <a href='SkRect_Reference#SkRect'>SkRect</a>, <a href='SkRect_Reference#SkRect'>if</a> <a href='SkRect_Reference#SkRect'>sorted</a>. <a href='SkRect_Reference#SkRect'>Call</a> <a href='#SkRect_isEmpty'>isEmpty</a>() <a href='#SkRect_isEmpty'>to</a> <a href='#SkRect_isEmpty'>see</a> <a href='#SkRect_isEmpty'>if</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>may</a> <a href='SkRect_Reference#SkRect'>be</a> <a href='SkRect_Reference#SkRect'>invalid</a>,
and <a href='#SkRect_sort'>sort()</a> <a href='#SkRect_sort'>to</a> <a href='#SkRect_sort'>reverse</a> <a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>and</a> <a href='#SkRect_fBottom'>fBottom</a> <a href='#SkRect_fBottom'>if</a> <a href='#SkRect_fBottom'>needed</a>.

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
<a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkRect_right'>right()</a> <a href='#SkRect_right'>const</a>
</pre>

Returns right edge of <a href='SkRect_Reference#SkRect'>SkRect</a>, <a href='SkRect_Reference#SkRect'>if</a> <a href='SkRect_Reference#SkRect'>sorted</a>. <a href='SkRect_Reference#SkRect'>Call</a> <a href='#SkRect_isSorted'>isSorted</a>() <a href='#SkRect_isSorted'>to</a> <a href='#SkRect_isSorted'>see</a> <a href='#SkRect_isSorted'>if</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>valid</a>.
Call <a href='#SkRect_sort'>sort()</a> <a href='#SkRect_sort'>to</a> <a href='#SkRect_sort'>reverse</a> <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>and</a> <a href='#SkRect_fRight'>fRight</a> <a href='#SkRect_fRight'>if</a> <a href='#SkRect_fRight'>needed</a>.

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
<a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkRect_bottom'>bottom()</a> <a href='#SkRect_bottom'>const</a>
</pre>

Returns bottom edge of <a href='SkRect_Reference#SkRect'>SkRect</a>, <a href='SkRect_Reference#SkRect'>if</a> <a href='SkRect_Reference#SkRect'>sorted</a>. <a href='SkRect_Reference#SkRect'>Call</a> <a href='#SkRect_isEmpty'>isEmpty</a>() <a href='#SkRect_isEmpty'>to</a> <a href='#SkRect_isEmpty'>see</a> <a href='#SkRect_isEmpty'>if</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>may</a> <a href='SkRect_Reference#SkRect'>be</a> <a href='SkRect_Reference#SkRect'>invalid</a>,
and <a href='#SkRect_sort'>sort()</a> <a href='#SkRect_sort'>to</a> <a href='#SkRect_sort'>reverse</a> <a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>and</a> <a href='#SkRect_fBottom'>fBottom</a> <a href='#SkRect_fBottom'>if</a> <a href='#SkRect_fBottom'>needed</a>.

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
<a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkRect_width'>width()</a> <a href='#SkRect_width'>const</a>
</pre>

Returns span on the x-axis. This does not check if <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>sorted</a>, <a href='SkRect_Reference#SkRect'>or</a> <a href='SkRect_Reference#SkRect'>if</a>
result fits in 32-bit float; result may be negative or infinity.

### Return Value

<a href='#SkRect_fRight'>fRight</a> <a href='#SkRect_fRight'>minus</a> <a href='#SkRect_fLeft'>fLeft</a>

### Example

<div><fiddle-embed name="11f8f0efe6291019fee0ac17844f6c1a"><div>Compare with <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_width'>width()</a> <a href='#SkIRect_width'>example</a>.
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
<a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkRect_height'>height()</a> <a href='#SkRect_height'>const</a>
</pre>

Returns span on the y-axis. This does not check if <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>sorted</a>, <a href='SkRect_Reference#SkRect'>or</a> <a href='SkRect_Reference#SkRect'>if</a>
result fits in 32-bit float; result may be negative or infinity.

### Return Value

<a href='#SkRect_fBottom'>fBottom</a> <a href='#SkRect_fBottom'>minus</a> <a href='#SkRect_fTop'>fTop</a>

### Example

<div><fiddle-embed name="39429e45f05240218ecd511443ab3e44"><div>Compare with <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_height'>height()</a> <a href='#SkIRect_height'>example</a>.
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
<a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkRect_centerX'>centerX</a>() <a href='#SkRect_centerX'>const</a>
</pre>

Returns average of left edge and right edge. Result does not change if <a href='SkRect_Reference#SkRect'>SkRect</a>
is sorted. Result may overflow to infinity if <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>far</a> <a href='SkRect_Reference#SkRect'>from</a> <a href='SkRect_Reference#SkRect'>the</a> <a href='SkRect_Reference#SkRect'>origin</a>.

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
<a href='undocumented#SkScalar'>SkScalar</a>    <a href='#SkRect_centerY'>centerY</a>() <a href='#SkRect_centerY'>const</a>
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
bool operator==(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>a</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>b</a>)
</pre>

Returns true if all members in <a href='#SkRect_equal_operator_a'>a</a>: <a href='#SkRect_fLeft'>fLeft</a>, <a href='#SkRect_fTop'>fTop</a>, <a href='#SkRect_fRight'>fRight</a>, <a href='#SkRect_fRight'>and</a> <a href='#SkRect_fBottom'>fBottom</a>; <a href='#SkRect_fBottom'>are</a>
equal to the corresponding members in <a href='#SkRect_equal_operator_b'>b</a>.

<a href='#SkRect_equal_operator_a'>a</a> <a href='#SkRect_equal_operator_a'>and</a> <a href='#SkRect_equal_operator_b'>b</a> <a href='#SkRect_equal_operator_b'>are</a> <a href='#SkRect_equal_operator_b'>not</a> <a href='#SkRect_equal_operator_b'>equal</a> <a href='#SkRect_equal_operator_b'>if</a> <a href='#SkRect_equal_operator_b'>either</a> <a href='#SkRect_equal_operator_b'>contain</a> <a href='#SkRect_equal_operator_b'>NaN</a>. <a href='#SkRect_equal_operator_a'>a</a> <a href='#SkRect_equal_operator_a'>and</a> <a href='#SkRect_equal_operator_b'>b</a> <a href='#SkRect_equal_operator_b'>are</a> <a href='#SkRect_equal_operator_b'>equal</a> <a href='#SkRect_equal_operator_b'>if</a> <a href='#SkRect_equal_operator_b'>members</a>
contain zeroes with different signs.

### Parameters

<table>  <tr>    <td><a name='SkRect_equal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>compare</a></td>
  </tr>
  <tr>    <td><a name='SkRect_equal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>compare</a></td>
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

<a href='#SkRect_notequal_operator'>operator!=(const SkRect& a, const SkRect& b)</a>

<a name='SkRect_notequal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator!=(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>a</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>b</a>)
</pre>

Returns true if any in <a href='#SkRect_notequal_operator_a'>a</a>: <a href='#SkRect_fLeft'>fLeft</a>, <a href='#SkRect_fTop'>fTop</a>, <a href='#SkRect_fRight'>fRight</a>, <a href='#SkRect_fRight'>and</a> <a href='#SkRect_fBottom'>fBottom</a>; <a href='#SkRect_fBottom'>does</a> <a href='#SkRect_fBottom'>not</a>
equal the corresponding members in <a href='#SkRect_notequal_operator_b'>b</a>.

<a href='#SkRect_notequal_operator_a'>a</a> <a href='#SkRect_notequal_operator_a'>and</a> <a href='#SkRect_notequal_operator_b'>b</a> <a href='#SkRect_notequal_operator_b'>are</a> <a href='#SkRect_notequal_operator_b'>not</a> <a href='#SkRect_notequal_operator_b'>equal</a> <a href='#SkRect_notequal_operator_b'>if</a> <a href='#SkRect_notequal_operator_b'>either</a> <a href='#SkRect_notequal_operator_b'>contain</a> <a href='#SkRect_notequal_operator_b'>NaN</a>. <a href='#SkRect_notequal_operator_a'>a</a> <a href='#SkRect_notequal_operator_a'>and</a> <a href='#SkRect_notequal_operator_b'>b</a> <a href='#SkRect_notequal_operator_b'>are</a> <a href='#SkRect_notequal_operator_b'>equal</a> <a href='#SkRect_notequal_operator_b'>if</a> <a href='#SkRect_notequal_operator_b'>members</a>
contain zeroes with different signs.

### Parameters

<table>  <tr>    <td><a name='SkRect_notequal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>compare</a></td>
  </tr>
  <tr>    <td><a name='SkRect_notequal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>compare</a></td>
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

<a href='#SkRect_equal_operator'>operator==(const SkRect& a, const SkRect& b)</a>

<a name='As_Points'></a>

<a name='SkRect_toQuad'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRect_toQuad'>toQuad</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPath_Reference#Quad'>quad</a>[4]) <a href='SkPath_Reference#Quad'>const</a>
</pre>

Returns four <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>in</a> <a href='#SkRect_toQuad_quad'>quad</a> <a href='#SkRect_toQuad_quad'>that</a> <a href='#SkRect_toQuad_quad'>enclose</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>ordered</a> <a href='SkRect_Reference#Rect'>as</a>: <a href='SkRect_Reference#Rect'>top-left</a>, <a href='SkRect_Reference#Rect'>top-right</a>,
<a href='SkRect_Reference#Rect'>bottom-right</a>, <a href='SkRect_Reference#Rect'>bottom-left</a>.

Private: Consider adding param to control whether quad is clockwise or counterclockwise.

### Parameters

<table>  <tr>    <td><a name='SkRect_toQuad_quad'><code><strong>quad</strong></code></a></td>
    <td>storage for corners of <a href='SkRect_Reference#Rect'>Rect</a></td>
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
void <a href='#SkRect_setBounds'>setBounds</a>(<a href='#SkRect_setBounds'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pts</a>[], <a href='SkPoint_Reference#SkPoint'>int</a> <a href='SkPoint_Reference#SkPoint'>count</a>)
</pre>

Sets to bounds of  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>with</a> <a href='#SkRect_setBounds_count'>count</a> <a href='#SkRect_setBounds_count'>entries</a>. <a href='#SkRect_setBounds_count'>If</a> <a href='#SkRect_setBounds_count'>count</a> <a href='#SkRect_setBounds_count'>is</a> <a href='#SkRect_setBounds_count'>zero</a> <a href='#SkRect_setBounds_count'>or</a> <a href='#SkRect_setBounds_count'>smaller</a>,
or if  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>contains</a> <a href='SkPoint_Reference#SkPoint'>an</a> <a href='SkPoint_Reference#SkPoint'>infinity</a> <a href='SkPoint_Reference#SkPoint'>or</a> <a href='SkPoint_Reference#SkPoint'>NaN</a>, <a href='SkPoint_Reference#SkPoint'>sets</a> <a href='SkPoint_Reference#SkPoint'>to</a> (0, 0, 0, 0).

Result is either empty or sorted: <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>is</a> <a href='#SkRect_fLeft'>less</a> <a href='#SkRect_fLeft'>than</a> <a href='#SkRect_fLeft'>or</a> <a href='#SkRect_fLeft'>equal</a> <a href='#SkRect_fLeft'>to</a> <a href='#SkRect_fRight'>fRight</a>, <a href='#SkRect_fRight'>and</a>
<a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>is</a> <a href='#SkRect_fTop'>less</a> <a href='#SkRect_fTop'>than</a> <a href='#SkRect_fTop'>or</a> <a href='#SkRect_fTop'>equal</a> <a href='#SkRect_fTop'>to</a> <a href='#SkRect_fBottom'>fBottom</a>.

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
bool <a href='#SkRect_setBoundsCheck'>setBoundsCheck</a>(<a href='#SkRect_setBoundsCheck'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pts</a>[], <a href='SkPoint_Reference#SkPoint'>int</a> <a href='SkPoint_Reference#SkPoint'>count</a>)
</pre>

Sets to bounds of  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>with</a> <a href='#SkRect_setBoundsCheck_count'>count</a> <a href='#SkRect_setBoundsCheck_count'>entries</a>. <a href='#SkRect_setBoundsCheck_count'>Returns</a> <a href='#SkRect_setBoundsCheck_count'>false</a> <a href='#SkRect_setBoundsCheck_count'>if</a> <a href='#SkRect_setBoundsCheck_count'>count</a> <a href='#SkRect_setBoundsCheck_count'>is</a>
zero or smaller, or if  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>contains</a> <a href='SkPoint_Reference#SkPoint'>an</a> <a href='SkPoint_Reference#SkPoint'>infinity</a> <a href='SkPoint_Reference#SkPoint'>or</a> <a href='SkPoint_Reference#SkPoint'>NaN</a>; <a href='SkPoint_Reference#SkPoint'>in</a> <a href='SkPoint_Reference#SkPoint'>these</a> <a href='SkPoint_Reference#SkPoint'>cases</a>
sets <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> (0, 0, 0, 0).

Result is either empty or sorted: <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>is</a> <a href='#SkRect_fLeft'>less</a> <a href='#SkRect_fLeft'>than</a> <a href='#SkRect_fLeft'>or</a> <a href='#SkRect_fLeft'>equal</a> <a href='#SkRect_fLeft'>to</a> <a href='#SkRect_fRight'>fRight</a>, <a href='#SkRect_fRight'>and</a>
<a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>is</a> <a href='#SkRect_fTop'>less</a> <a href='#SkRect_fTop'>than</a> <a href='#SkRect_fTop'>or</a> <a href='#SkRect_fTop'>equal</a> <a href='#SkRect_fTop'>to</a> <a href='#SkRect_fBottom'>fBottom</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_setBoundsCheck_pts'><code><strong>pts</strong></code></a></td>
    <td><a href='SkPath_Reference#Point_Array'>SkPoint array</a></td>
  </tr>
  <tr>    <td><a name='SkRect_setBoundsCheck_count'><code><strong>count</strong></code></a></td>
    <td>entries in array</td>
  </tr>
</table>

### Return Value

true if all <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>values</a> <a href='SkPoint_Reference#SkPoint'>are</a> <a href='SkPoint_Reference#SkPoint'>finite</a>

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
void <a href='#SkRect_setBoundsNoCheck'>setBoundsNoCheck</a>(<a href='#SkRect_setBoundsNoCheck'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pts</a>[], <a href='SkPoint_Reference#SkPoint'>int</a> <a href='SkPoint_Reference#SkPoint'>count</a>)
</pre>

Sets to bounds of <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkRect_setBoundsNoCheck_pts'>pts</a> <a href='#SkRect_setBoundsNoCheck_pts'>array</a> <a href='#SkRect_setBoundsNoCheck_pts'>with</a> <a href='#SkRect_setBoundsNoCheck_count'>count</a> <a href='#SkRect_setBoundsNoCheck_count'>entries</a>. <a href='#SkRect_setBoundsNoCheck_count'>If</a> <a href='#SkRect_setBoundsNoCheck_count'>any</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>in</a> <a href='#SkRect_setBoundsNoCheck_pts'>pts</a>
contains infinity or NaN, all <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>dimensions</a> <a href='SkRect_Reference#SkRect'>are</a> <a href='SkRect_Reference#SkRect'>set</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>NaN</a>.

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

Sets <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> (0, 0, 0, 0).

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
void set(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>src</a>)
</pre>

Sets <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='#SkRect_set_src'>src</a>, <a href='#SkRect_set_src'>promoting</a> <a href='#SkRect_set_src'>src</a> <a href='#SkRect_set_src'>members</a> <a href='#SkRect_set_src'>from</a> <a href='#SkRect_set_src'>integer</a> <a href='#SkRect_set_src'>to</a> <a href='undocumented#Scalar'>scalar</a>.
Very large values in <a href='#SkRect_set_src'>src</a> <a href='#SkRect_set_src'>may</a> <a href='#SkRect_set_src'>lose</a> <a href='#SkRect_set_src'>precision</a>.

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
void set(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>left</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>top</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>right</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>bottom</a>)
</pre>

Sets <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> (<a href='#SkRect_set_2_left'>left</a>, <a href='#SkRect_set_2_top'>top</a>, <a href='#SkRect_set_2_right'>right</a>, <a href='#SkRect_set_2_bottom'>bottom</a>).
<a href='#SkRect_set_2_left'>left</a> <a href='#SkRect_set_2_left'>and</a> <a href='#SkRect_set_2_right'>right</a> <a href='#SkRect_set_2_right'>are</a> <a href='#SkRect_set_2_right'>not</a> <a href='#SkRect_set_2_right'>sorted</a>; <a href='#SkRect_set_2_left'>left</a> <a href='#SkRect_set_2_left'>is</a> <a href='#SkRect_set_2_left'>not</a> <a href='#SkRect_set_2_left'>necessarily</a> <a href='#SkRect_set_2_left'>less</a> <a href='#SkRect_set_2_left'>than</a> <a href='#SkRect_set_2_right'>right</a>.
<a href='#SkRect_set_2_top'>top</a> <a href='#SkRect_set_2_top'>and</a> <a href='#SkRect_set_2_bottom'>bottom</a> <a href='#SkRect_set_2_bottom'>are</a> <a href='#SkRect_set_2_bottom'>not</a> <a href='#SkRect_set_2_bottom'>sorted</a>; <a href='#SkRect_set_2_top'>top</a> <a href='#SkRect_set_2_top'>is</a> <a href='#SkRect_set_2_top'>not</a> <a href='#SkRect_set_2_top'>necessarily</a> <a href='#SkRect_set_2_top'>less</a> <a href='#SkRect_set_2_top'>than</a> <a href='#SkRect_set_2_bottom'>bottom</a>.

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
void <a href='#SkRect_setLTRB'>setLTRB</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>left</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>top</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>right</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>bottom</a>)
</pre>

Sets <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> (<a href='#SkRect_setLTRB_left'>left</a>, <a href='#SkRect_setLTRB_top'>top</a>, <a href='#SkRect_setLTRB_right'>right</a>, <a href='#SkRect_setLTRB_bottom'>bottom</a>).
<a href='#SkRect_setLTRB_left'>left</a> <a href='#SkRect_setLTRB_left'>and</a> <a href='#SkRect_setLTRB_right'>right</a> <a href='#SkRect_setLTRB_right'>are</a> <a href='#SkRect_setLTRB_right'>not</a> <a href='#SkRect_setLTRB_right'>sorted</a>; <a href='#SkRect_setLTRB_left'>left</a> <a href='#SkRect_setLTRB_left'>is</a> <a href='#SkRect_setLTRB_left'>not</a> <a href='#SkRect_setLTRB_left'>necessarily</a> <a href='#SkRect_setLTRB_left'>less</a> <a href='#SkRect_setLTRB_left'>than</a> <a href='#SkRect_setLTRB_right'>right</a>.
<a href='#SkRect_setLTRB_top'>top</a> <a href='#SkRect_setLTRB_top'>and</a> <a href='#SkRect_setLTRB_bottom'>bottom</a> <a href='#SkRect_setLTRB_bottom'>are</a> <a href='#SkRect_setLTRB_bottom'>not</a> <a href='#SkRect_setLTRB_bottom'>sorted</a>; <a href='#SkRect_setLTRB_top'>top</a> <a href='#SkRect_setLTRB_top'>is</a> <a href='#SkRect_setLTRB_top'>not</a> <a href='#SkRect_setLTRB_top'>necessarily</a> <a href='#SkRect_setLTRB_top'>less</a> <a href='#SkRect_setLTRB_top'>than</a> <a href='#SkRect_setLTRB_bottom'>bottom</a>.

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
void set(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pts</a>[], <a href='SkPoint_Reference#SkPoint'>int</a> <a href='SkPoint_Reference#SkPoint'>count</a>)
</pre>

Sets to bounds of  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>with</a> <a href='#SkRect_set_3_count'>count</a> <a href='#SkRect_set_3_count'>entries</a>. <a href='#SkRect_set_3_count'>If</a> <a href='#SkRect_set_3_count'>count</a> <a href='#SkRect_set_3_count'>is</a> <a href='#SkRect_set_3_count'>zero</a> <a href='#SkRect_set_3_count'>or</a> <a href='#SkRect_set_3_count'>smaller</a>,
or if  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>contains</a> <a href='SkPoint_Reference#SkPoint'>an</a> <a href='SkPoint_Reference#SkPoint'>infinity</a> <a href='SkPoint_Reference#SkPoint'>or</a> <a href='SkPoint_Reference#SkPoint'>NaN</a>, <a href='SkPoint_Reference#SkPoint'>sets</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> (0, 0, 0, 0).

Result is either empty or sorted: <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>is</a> <a href='#SkRect_fLeft'>less</a> <a href='#SkRect_fLeft'>than</a> <a href='#SkRect_fLeft'>or</a> <a href='#SkRect_fLeft'>equal</a> <a href='#SkRect_fLeft'>to</a> <a href='#SkRect_fRight'>fRight</a>, <a href='#SkRect_fRight'>and</a>
<a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>is</a> <a href='#SkRect_fTop'>less</a> <a href='#SkRect_fTop'>than</a> <a href='#SkRect_fTop'>or</a> <a href='#SkRect_fTop'>equal</a> <a href='#SkRect_fTop'>to</a> <a href='#SkRect_fBottom'>fBottom</a>.

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
void set(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p0</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p1</a>)
</pre>

Sets bounds to the smallest <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>enclosing</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkRect_set_4_p0'>p0</a> <a href='#SkRect_set_4_p0'>and</a> <a href='#SkRect_set_4_p1'>p1</a>. <a href='#SkRect_set_4_p1'>The</a> <a href='#SkRect_set_4_p1'>result</a> <a href='#SkRect_set_4_p1'>is</a>
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

<div><fiddle-embed name="ee72450381f768f3869153cdbeccdc3e"><div><a href='#SkRect_set_4_p0'>p0</a> <a href='#SkRect_set_4_p0'>and</a> <a href='#SkRect_set_4_p1'>p1</a> <a href='#SkRect_set_4_p1'>may</a> <a href='#SkRect_set_4_p1'>be</a> <a href='#SkRect_set_4_p1'>swapped</a> <a href='#SkRect_set_4_p1'>and</a> <a href='#SkRect_set_4_p1'>have</a> <a href='#SkRect_set_4_p1'>the</a> <a href='#SkRect_set_4_p1'>same</a> <a href='#SkRect_set_4_p1'>effect</a> <a href='#SkRect_set_4_p1'>unless</a> <a href='#SkRect_set_4_p1'>one</a> <a href='#SkRect_set_4_p1'>contains</a> <a href='#SkRect_set_4_p1'>NaN</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkRect_setBounds'>setBounds</a> <a href='#SkRect_setBoundsCheck'>setBoundsCheck</a>

<a name='SkRect_setXYWH'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRect_setXYWH'>setXYWH</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>width</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>height</a>)
</pre>

Sets <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>to</a> <code>(<a href='#SkRect_setXYWH_x'>x</a>, <a href='#SkRect_setXYWH_y'>y</a>, <a href='#SkRect_setXYWH_x'>x</a> + <a href='#SkRect_setXYWH_width'>width</a>, <a href='#SkRect_setXYWH_y'>y</a> + <a href='#SkRect_setXYWH_height'>height</a>)</code>.
Does not validate input; <a href='#SkRect_setXYWH_width'>width</a> <a href='#SkRect_setXYWH_width'>or</a> <a href='#SkRect_setXYWH_height'>height</a> <a href='#SkRect_setXYWH_height'>may</a> <a href='#SkRect_setXYWH_height'>be</a> <a href='#SkRect_setXYWH_height'>negative</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_setXYWH_x'><code><strong>x</strong></code></a></td>
    <td>stored in <a href='#SkRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkRect_setXYWH_y'><code><strong>y</strong></code></a></td>
    <td>stored in <a href='#SkRect_fTop'>fTop</a></td>
  </tr>
  <tr>    <td><a name='SkRect_setXYWH_width'><code><strong>width</strong></code></a></td>
    <td>added to <a href='#SkRect_setXYWH_x'>x</a> <a href='#SkRect_setXYWH_x'>and</a> <a href='#SkRect_setXYWH_x'>stored</a> <a href='#SkRect_setXYWH_x'>in</a> <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_setXYWH_height'><code><strong>height</strong></code></a></td>
    <td>added to <a href='#SkRect_setXYWH_y'>y</a> <a href='#SkRect_setXYWH_y'>and</a> <a href='#SkRect_setXYWH_y'>stored</a> <a href='#SkRect_setXYWH_y'>in</a> <a href='#SkRect_fBottom'>fBottom</a></td>
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
void <a href='#SkRect_setWH'>setWH</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>width</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>height</a>)
</pre>

Sets <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> (0, 0, <a href='#SkRect_setWH_width'>width</a>, <a href='#SkRect_setWH_height'>height</a>). <a href='#SkRect_setWH_height'>Does</a> <a href='#SkRect_setWH_height'>not</a> <a href='#SkRect_setWH_height'>validate</a> <a href='#SkRect_setWH_height'>input</a>;
<a href='#SkRect_setWH_width'>width</a> <a href='#SkRect_setWH_width'>or</a> <a href='#SkRect_setWH_height'>height</a> <a href='#SkRect_setWH_height'>may</a> <a href='#SkRect_setWH_height'>be</a> <a href='#SkRect_setWH_height'>negative</a>.

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
void <a href='#SkRect_iset'>iset</a>(<a href='#SkRect_iset'>int</a> <a href='#SkRect_iset'>left</a>, <a href='#SkRect_iset'>int</a> <a href='#SkRect_iset'>top</a>, <a href='#SkRect_iset'>int</a> <a href='#SkRect_iset'>right</a>, <a href='#SkRect_iset'>int</a> <a href='#SkRect_iset'>bottom</a>)
</pre>

Sets <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> (<a href='#SkRect_iset_left'>left</a>, <a href='#SkRect_iset_top'>top</a>, <a href='#SkRect_iset_right'>right</a>, <a href='#SkRect_iset_bottom'>bottom</a>).
All parameters are promoted from integer to <a href='undocumented#Scalar'>scalar</a>.
<a href='#SkRect_iset_left'>left</a> <a href='#SkRect_iset_left'>and</a> <a href='#SkRect_iset_right'>right</a> <a href='#SkRect_iset_right'>are</a> <a href='#SkRect_iset_right'>not</a> <a href='#SkRect_iset_right'>sorted</a>; <a href='#SkRect_iset_left'>left</a> <a href='#SkRect_iset_left'>is</a> <a href='#SkRect_iset_left'>not</a> <a href='#SkRect_iset_left'>necessarily</a> <a href='#SkRect_iset_left'>less</a> <a href='#SkRect_iset_left'>than</a> <a href='#SkRect_iset_right'>right</a>.
<a href='#SkRect_iset_top'>top</a> <a href='#SkRect_iset_top'>and</a> <a href='#SkRect_iset_bottom'>bottom</a> <a href='#SkRect_iset_bottom'>are</a> <a href='#SkRect_iset_bottom'>not</a> <a href='#SkRect_iset_bottom'>sorted</a>; <a href='#SkRect_iset_top'>top</a> <a href='#SkRect_iset_top'>is</a> <a href='#SkRect_iset_top'>not</a> <a href='#SkRect_iset_top'>necessarily</a> <a href='#SkRect_iset_top'>less</a> <a href='#SkRect_iset_top'>than</a> <a href='#SkRect_iset_bottom'>bottom</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_iset_left'><code><strong>left</strong></code></a></td>
    <td>promoted to <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>and</a> <a href='undocumented#SkScalar'>stored</a> <a href='undocumented#SkScalar'>in</a> <a href='#SkRect_fLeft'>fLeft</a></td>
  </tr>
  <tr>    <td><a name='SkRect_iset_top'><code><strong>top</strong></code></a></td>
    <td>promoted to <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>and</a> <a href='undocumented#SkScalar'>stored</a> <a href='undocumented#SkScalar'>in</a> <a href='#SkRect_fTop'>fTop</a></td>
  </tr>
  <tr>    <td><a name='SkRect_iset_right'><code><strong>right</strong></code></a></td>
    <td>promoted to <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>and</a> <a href='undocumented#SkScalar'>stored</a> <a href='undocumented#SkScalar'>in</a> <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_iset_bottom'><code><strong>bottom</strong></code></a></td>
    <td>promoted to <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>and</a> <a href='undocumented#SkScalar'>stored</a> <a href='undocumented#SkScalar'>in</a> <a href='#SkRect_fBottom'>fBottom</a></td>
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
void <a href='#SkRect_isetWH'>isetWH</a>(<a href='#SkRect_isetWH'>int</a> <a href='#SkRect_isetWH'>width</a>, <a href='#SkRect_isetWH'>int</a> <a href='#SkRect_isetWH'>height</a>)
</pre>

Sets <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> (0, 0, <a href='#SkRect_isetWH_width'>width</a>, <a href='#SkRect_isetWH_height'>height</a>).
<a href='#SkRect_isetWH_width'>width</a> <a href='#SkRect_isetWH_width'>and</a> <a href='#SkRect_isetWH_height'>height</a> <a href='#SkRect_isetWH_height'>may</a> <a href='#SkRect_isetWH_height'>be</a> <a href='#SkRect_isetWH_height'>zero</a> <a href='#SkRect_isetWH_height'>or</a> <a href='#SkRect_isetWH_height'>negative</a>. <a href='#SkRect_isetWH_width'>width</a> <a href='#SkRect_isetWH_width'>and</a> <a href='#SkRect_isetWH_height'>height</a> <a href='#SkRect_isetWH_height'>are</a> <a href='#SkRect_isetWH_height'>promoted</a> <a href='#SkRect_isetWH_height'>from</a>
integer to <a href='undocumented#SkScalar'>SkScalar</a>, <a href='undocumented#SkScalar'>large</a> <a href='undocumented#SkScalar'>values</a> <a href='undocumented#SkScalar'>may</a> <a href='undocumented#SkScalar'>lose</a> <a href='undocumented#SkScalar'>precision</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_isetWH_width'><code><strong>width</strong></code></a></td>
    <td>promoted to <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>and</a> <a href='undocumented#SkScalar'>stored</a> <a href='undocumented#SkScalar'>in</a> <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_isetWH_height'><code><strong>height</strong></code></a></td>
    <td>promoted to <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>and</a> <a href='undocumented#SkScalar'>stored</a> <a href='undocumented#SkScalar'>in</a> <a href='#SkRect_fBottom'>fBottom</a></td>
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
<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_makeOffset'>makeOffset</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>) <a href='undocumented#SkScalar'>const</a>
</pre>

Returns <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>offset</a> <a href='SkRect_Reference#SkRect'>by</a> (<a href='#SkRect_makeOffset_dx'>dx</a>, <a href='#SkRect_makeOffset_dy'>dy</a>).

If <a href='#SkRect_makeOffset_dx'>dx</a> <a href='#SkRect_makeOffset_dx'>is</a> <a href='#SkRect_makeOffset_dx'>negative</a>, <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>returned</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>moved</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>the</a> <a href='SkRect_Reference#SkRect'>left</a>.
If <a href='#SkRect_makeOffset_dx'>dx</a> <a href='#SkRect_makeOffset_dx'>is</a> <a href='#SkRect_makeOffset_dx'>positive</a>, <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>returned</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>moved</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>the</a> <a href='SkRect_Reference#SkRect'>right</a>.
If <a href='#SkRect_makeOffset_dy'>dy</a> <a href='#SkRect_makeOffset_dy'>is</a> <a href='#SkRect_makeOffset_dy'>negative</a>, <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>returned</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>moved</a> <a href='SkRect_Reference#SkRect'>upward</a>.
If <a href='#SkRect_makeOffset_dy'>dy</a> <a href='#SkRect_makeOffset_dy'>is</a> <a href='#SkRect_makeOffset_dy'>positive</a>, <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>returned</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>moved</a> <a href='SkRect_Reference#SkRect'>downward</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_makeOffset_dx'><code><strong>dx</strong></code></a></td>
    <td>added to <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>and</a> <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_makeOffset_dy'><code><strong>dy</strong></code></a></td>
    <td>added to <a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>and</a> <a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>offset</a> <a href='SkRect_Reference#SkRect'>on</a> <a href='SkRect_Reference#SkRect'>axes</a>, <a href='SkRect_Reference#SkRect'>with</a> <a href='SkRect_Reference#SkRect'>original</a> <a href='SkRect_Reference#SkRect'>width</a> <a href='SkRect_Reference#SkRect'>and</a> <a href='SkRect_Reference#SkRect'>height</a>

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
<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_makeInset'>makeInset</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>) <a href='undocumented#SkScalar'>const</a>
</pre>

Returns <a href='SkRect_Reference#SkRect'>SkRect</a>, <a href='SkRect_Reference#SkRect'>inset</a> <a href='SkRect_Reference#SkRect'>by</a> (<a href='#SkRect_makeInset_dx'>dx</a>, <a href='#SkRect_makeInset_dy'>dy</a>).

If <a href='#SkRect_makeInset_dx'>dx</a> <a href='#SkRect_makeInset_dx'>is</a> <a href='#SkRect_makeInset_dx'>negative</a>, <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>returned</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>wider</a>.
If <a href='#SkRect_makeInset_dx'>dx</a> <a href='#SkRect_makeInset_dx'>is</a> <a href='#SkRect_makeInset_dx'>positive</a>, <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>returned</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>narrower</a>.
If <a href='#SkRect_makeInset_dy'>dy</a> <a href='#SkRect_makeInset_dy'>is</a> <a href='#SkRect_makeInset_dy'>negative</a>, <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>returned</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>taller</a>.
If <a href='#SkRect_makeInset_dy'>dy</a> <a href='#SkRect_makeInset_dy'>is</a> <a href='#SkRect_makeInset_dy'>positive</a>, <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>returned</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>shorter</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_makeInset_dx'><code><strong>dx</strong></code></a></td>
    <td>added to <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>and</a> <a href='#SkRect_fLeft'>subtracted</a> <a href='#SkRect_fLeft'>from</a> <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_makeInset_dy'><code><strong>dy</strong></code></a></td>
    <td>added to <a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>and</a> <a href='#SkRect_fTop'>subtracted</a> <a href='#SkRect_fTop'>from</a> <a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>inset</a> <a href='SkRect_Reference#SkRect'>symmetrically</a> <a href='SkRect_Reference#SkRect'>left</a> <a href='SkRect_Reference#SkRect'>and</a> <a href='SkRect_Reference#SkRect'>right</a>, <a href='SkRect_Reference#SkRect'>top</a> <a href='SkRect_Reference#SkRect'>and</a> <a href='SkRect_Reference#SkRect'>bottom</a>

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
<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_makeOutset'>makeOutset</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>) <a href='undocumented#SkScalar'>const</a>
</pre>

Returns <a href='SkRect_Reference#SkRect'>SkRect</a>, <a href='SkRect_Reference#SkRect'>outset</a> <a href='SkRect_Reference#SkRect'>by</a> (<a href='#SkRect_makeOutset_dx'>dx</a>, <a href='#SkRect_makeOutset_dy'>dy</a>).

If <a href='#SkRect_makeOutset_dx'>dx</a> <a href='#SkRect_makeOutset_dx'>is</a> <a href='#SkRect_makeOutset_dx'>negative</a>, <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>returned</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>narrower</a>.
If <a href='#SkRect_makeOutset_dx'>dx</a> <a href='#SkRect_makeOutset_dx'>is</a> <a href='#SkRect_makeOutset_dx'>positive</a>, <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>returned</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>wider</a>.
If <a href='#SkRect_makeOutset_dy'>dy</a> <a href='#SkRect_makeOutset_dy'>is</a> <a href='#SkRect_makeOutset_dy'>negative</a>, <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>returned</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>shorter</a>.
If <a href='#SkRect_makeOutset_dy'>dy</a> <a href='#SkRect_makeOutset_dy'>is</a> <a href='#SkRect_makeOutset_dy'>positive</a>, <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>returned</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>taller</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_makeOutset_dx'><code><strong>dx</strong></code></a></td>
    <td>subtracted to <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>and</a> <a href='#SkRect_fLeft'>added</a> <a href='#SkRect_fLeft'>from</a> <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_makeOutset_dy'><code><strong>dy</strong></code></a></td>
    <td>subtracted to <a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>and</a> <a href='#SkRect_fTop'>added</a> <a href='#SkRect_fTop'>from</a> <a href='#SkRect_fBottom'>fBottom</a></td>
  </tr>
</table>

### Return Value

<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>outset</a> <a href='SkRect_Reference#SkRect'>symmetrically</a> <a href='SkRect_Reference#SkRect'>left</a> <a href='SkRect_Reference#SkRect'>and</a> <a href='SkRect_Reference#SkRect'>right</a>, <a href='SkRect_Reference#SkRect'>top</a> <a href='SkRect_Reference#SkRect'>and</a> <a href='SkRect_Reference#SkRect'>bottom</a>

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
void offset(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>)
</pre>

Offsets <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>by</a> <a href='SkRect_Reference#SkRect'>adding</a> <a href='#SkRect_offset_dx'>dx</a> <a href='#SkRect_offset_dx'>to</a> <a href='#SkRect_fLeft'>fLeft</a>, <a href='#SkRect_fRight'>fRight</a>; <a href='#SkRect_fRight'>and</a> <a href='#SkRect_fRight'>by</a> <a href='#SkRect_fRight'>adding</a> <a href='#SkRect_offset_dy'>dy</a> <a href='#SkRect_offset_dy'>to</a> <a href='#SkRect_fTop'>fTop</a>, <a href='#SkRect_fBottom'>fBottom</a>.

If <a href='#SkRect_offset_dx'>dx</a> <a href='#SkRect_offset_dx'>is</a> <a href='#SkRect_offset_dx'>negative</a>, <a href='#SkRect_offset_dx'>moves</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>the</a> <a href='SkRect_Reference#SkRect'>left</a>.
If <a href='#SkRect_offset_dx'>dx</a> <a href='#SkRect_offset_dx'>is</a> <a href='#SkRect_offset_dx'>positive</a>, <a href='#SkRect_offset_dx'>moves</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>the</a> <a href='SkRect_Reference#SkRect'>right</a>.
If <a href='#SkRect_offset_dy'>dy</a> <a href='#SkRect_offset_dy'>is</a> <a href='#SkRect_offset_dy'>negative</a>, <a href='#SkRect_offset_dy'>moves</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>upward</a>.
If <a href='#SkRect_offset_dy'>dy</a> <a href='#SkRect_offset_dy'>is</a> <a href='#SkRect_offset_dy'>positive</a>, <a href='#SkRect_offset_dy'>moves</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>downward</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_offset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>and</a> <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_offset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>and</a> <a href='#SkRect_fBottom'>fBottom</a></td>
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
void offset(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>delta</a>)
</pre>

Offsets <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>by</a> <a href='SkRect_Reference#SkRect'>adding</a> <a href='#SkRect_offset_2_delta'>delta</a>.<a href='#SkPoint_fX'>fX</a> <a href='#SkPoint_fX'>to</a> <a href='#SkRect_fLeft'>fLeft</a>, <a href='#SkRect_fRight'>fRight</a>; <a href='#SkRect_fRight'>and</a> <a href='#SkRect_fRight'>by</a> <a href='#SkRect_fRight'>adding</a> <a href='#SkRect_offset_2_delta'>delta</a>.<a href='#SkPoint_fY'>fY</a> <a href='#SkPoint_fY'>to</a>
<a href='#SkRect_fTop'>fTop</a>, <a href='#SkRect_fBottom'>fBottom</a>.

If <a href='#SkRect_offset_2_delta'>delta</a>.<a href='#SkPoint_fX'>fX</a> <a href='#SkPoint_fX'>is</a> <a href='#SkPoint_fX'>negative</a>, <a href='#SkPoint_fX'>moves</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>the</a> <a href='SkRect_Reference#SkRect'>left</a>.
If <a href='#SkRect_offset_2_delta'>delta</a>.<a href='#SkPoint_fX'>fX</a> <a href='#SkPoint_fX'>is</a> <a href='#SkPoint_fX'>positive</a>, <a href='#SkPoint_fX'>moves</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>the</a> <a href='SkRect_Reference#SkRect'>right</a>.
If <a href='#SkRect_offset_2_delta'>delta</a>.<a href='#SkPoint_fY'>fY</a> <a href='#SkPoint_fY'>is</a> <a href='#SkPoint_fY'>negative</a>, <a href='#SkPoint_fY'>moves</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>upward</a>.
If <a href='#SkRect_offset_2_delta'>delta</a>.<a href='#SkPoint_fY'>fY</a> <a href='#SkPoint_fY'>is</a> <a href='#SkPoint_fY'>positive</a>, <a href='#SkPoint_fY'>moves</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>downward</a>.

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
void <a href='#SkRect_offsetTo'>offsetTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>newX</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>newY</a>)
</pre>

Offsets <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>so</a> <a href='SkRect_Reference#SkRect'>that</a> <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>equals</a> <a href='#SkRect_offsetTo_newX'>newX</a>, <a href='#SkRect_offsetTo_newX'>and</a> <a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>equals</a> <a href='#SkRect_offsetTo_newY'>newY</a>. <a href='#SkRect_offsetTo_newY'>width</a> <a href='#SkRect_offsetTo_newY'>and</a> <a href='#SkRect_offsetTo_newY'>height</a>
are unchanged.

### Parameters

<table>  <tr>    <td><a name='SkRect_offsetTo_newX'><code><strong>newX</strong></code></a></td>
    <td>stored in <a href='#SkRect_fLeft'>fLeft</a>, <a href='#SkRect_fLeft'>preserving</a> <a href='#SkRect_width'>width()</a></td>
  </tr>
  <tr>    <td><a name='SkRect_offsetTo_newY'><code><strong>newY</strong></code></a></td>
    <td>stored in <a href='#SkRect_fTop'>fTop</a>, <a href='#SkRect_fTop'>preserving</a> <a href='#SkRect_height'>height()</a></td>
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
void inset(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>)
</pre>

Insets <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>by</a> (<a href='#SkRect_inset_dx'>dx</a>, <a href='#SkRect_inset_dy'>dy</a>).

If <a href='#SkRect_inset_dx'>dx</a> <a href='#SkRect_inset_dx'>is</a> <a href='#SkRect_inset_dx'>positive</a>, <a href='#SkRect_inset_dx'>makes</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>narrower</a>.
If <a href='#SkRect_inset_dx'>dx</a> <a href='#SkRect_inset_dx'>is</a> <a href='#SkRect_inset_dx'>negative</a>, <a href='#SkRect_inset_dx'>makes</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>wider</a>.
If <a href='#SkRect_inset_dy'>dy</a> <a href='#SkRect_inset_dy'>is</a> <a href='#SkRect_inset_dy'>positive</a>, <a href='#SkRect_inset_dy'>makes</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>shorter</a>.
If <a href='#SkRect_inset_dy'>dy</a> <a href='#SkRect_inset_dy'>is</a> <a href='#SkRect_inset_dy'>negative</a>, <a href='#SkRect_inset_dy'>makes</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>taller</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_inset_dx'><code><strong>dx</strong></code></a></td>
    <td>added to <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>and</a> <a href='#SkRect_fLeft'>subtracted</a> <a href='#SkRect_fLeft'>from</a> <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_inset_dy'><code><strong>dy</strong></code></a></td>
    <td>added to <a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>and</a> <a href='#SkRect_fTop'>subtracted</a> <a href='#SkRect_fTop'>from</a> <a href='#SkRect_fBottom'>fBottom</a></td>
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
void outset(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>)
</pre>

Outsets <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>by</a> (<a href='#SkRect_outset_dx'>dx</a>, <a href='#SkRect_outset_dy'>dy</a>).

If <a href='#SkRect_outset_dx'>dx</a> <a href='#SkRect_outset_dx'>is</a> <a href='#SkRect_outset_dx'>positive</a>, <a href='#SkRect_outset_dx'>makes</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>wider</a>.
If <a href='#SkRect_outset_dx'>dx</a> <a href='#SkRect_outset_dx'>is</a> <a href='#SkRect_outset_dx'>negative</a>, <a href='#SkRect_outset_dx'>makes</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>narrower</a>.
If <a href='#SkRect_outset_dy'>dy</a> <a href='#SkRect_outset_dy'>is</a> <a href='#SkRect_outset_dy'>positive</a>, <a href='#SkRect_outset_dy'>makes</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>taller</a>.
If <a href='#SkRect_outset_dy'>dy</a> <a href='#SkRect_outset_dy'>is</a> <a href='#SkRect_outset_dy'>negative</a>, <a href='#SkRect_outset_dy'>makes</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>shorter</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_outset_dx'><code><strong>dx</strong></code></a></td>
    <td>subtracted to <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>and</a> <a href='#SkRect_fLeft'>added</a> <a href='#SkRect_fLeft'>from</a> <a href='#SkRect_fRight'>fRight</a></td>
  </tr>
  <tr>    <td><a name='SkRect_outset_dy'><code><strong>dy</strong></code></a></td>
    <td>subtracted to <a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>and</a> <a href='#SkRect_fTop'>added</a> <a href='#SkRect_fTop'>from</a> <a href='#SkRect_fBottom'>fBottom</a></td>
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

<a href='SkRect_Reference#Rect'>Rects</a> <a href='SkRect_Reference#Rect'>intersect</a> <a href='SkRect_Reference#Rect'>when</a> <a href='SkRect_Reference#Rect'>they</a> <a href='SkRect_Reference#Rect'>enclose</a> <a href='SkRect_Reference#Rect'>a</a> <a href='SkRect_Reference#Rect'>common</a> <a href='SkRect_Reference#Rect'>area</a>. <a href='SkRect_Reference#Rect'>To</a> <a href='SkRect_Reference#Rect'>intersect</a>, <a href='SkRect_Reference#Rect'>each</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>the</a> <a href='SkRect_Reference#Rect'>pair</a>
<a href='SkRect_Reference#Rect'>must</a> <a href='SkRect_Reference#Rect'>describe</a> <a href='SkRect_Reference#Rect'>area</a>; <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>is</a> <a href='#SkRect_fLeft'>less</a> <a href='#SkRect_fLeft'>than</a> <a href='#SkRect_fRight'>fRight</a>, <a href='#SkRect_fRight'>and</a> <a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>is</a> <a href='#SkRect_fTop'>less</a> <a href='#SkRect_fTop'>than</a> <a href='#SkRect_fBottom'>fBottom</a>;
<a href='#SkRect_isEmpty'>isEmpty</a>() <a href='#SkRect_isEmpty'>returns</a> <a href='#SkRect_isEmpty'>false</a>. <a href='#SkRect_isEmpty'>The</a> <a href='#SkRect_isEmpty'>intersection</a> <a href='#SkRect_isEmpty'>of</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>pair</a> <a href='SkRect_Reference#Rect'>can</a> <a href='SkRect_Reference#Rect'>be</a> <a href='SkRect_Reference#Rect'>described</a> <a href='SkRect_Reference#Rect'>by</a>:
<code>(<a href='undocumented#max()'>max</a>(<a href='undocumented#max()'>a</a>.<a href='#SkRect_fLeft'>fLeft</a>, <a href='#SkRect_fLeft'>b</a>.<a href='#SkRect_fLeft'>fLeft</a>), <a href='undocumented#max()'>max</a>(<a href='undocumented#max()'>a</a>.<a href='#SkRect_fTop'>fTop</a>, <a href='#SkRect_fTop'>b</a>.<a href='#SkRect_fTop'>fTop</a>),
<a href='undocumented#min()'>min</a>(<a href='undocumented#min()'>a</a>.<a href='#SkRect_fRight'>fRight</a>, <a href='#SkRect_fRight'>b</a>.<a href='#SkRect_fRight'>fRight</a>), <a href='undocumented#min()'>min</a>(<a href='undocumented#min()'>a</a>.<a href='#SkRect_fBottom'>fBottom</a>, <a href='#SkRect_fBottom'>b</a>.<a href='#SkRect_fBottom'>fBottom</a>))</code>.

The intersection is only meaningful if the resulting <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>is</a> <a href='SkRect_Reference#Rect'>not</a> <a href='SkRect_Reference#Rect'>empty</a> <a href='SkRect_Reference#Rect'>and</a>
<a href='SkRect_Reference#Rect'>describes</a> <a href='SkRect_Reference#Rect'>an</a> <a href='SkRect_Reference#Rect'>area</a>: <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>is</a> <a href='#SkRect_fLeft'>less</a> <a href='#SkRect_fLeft'>than</a> <a href='#SkRect_fRight'>fRight</a>, <a href='#SkRect_fRight'>and</a> <a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>is</a> <a href='#SkRect_fTop'>less</a> <a href='#SkRect_fTop'>than</a> <a href='#SkRect_fBottom'>fBottom</a>.

<a name='SkRect_contains'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool contains(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>) <a href='undocumented#SkScalar'>const</a>
</pre>

Returns true if: <a href='#SkRect_fLeft'>fLeft</a> <= <a href='#SkRect_contains_x'>x</a> < <a href='#SkRect_fRight'>fRight</a> && <a href='#SkRect_fTop'>fTop</a> <= <a href='#SkRect_contains_y'>y</a> < <a href='#SkRect_fBottom'>fBottom</a>.
Returns false if <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>empty</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_contains_x'><code><strong>x</strong></code></a></td>
    <td>test <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>x-coordinate</a></td>
  </tr>
  <tr>    <td><a name='SkRect_contains_y'><code><strong>y</strong></code></a></td>
    <td>test <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>y-coordinate</a></td>
  </tr>
</table>

### Return Value

true if (<a href='#SkRect_contains_x'>x</a>, <a href='#SkRect_contains_y'>y</a>) <a href='#SkRect_contains_y'>is</a> <a href='#SkRect_contains_y'>inside</a> <a href='SkRect_Reference#SkRect'>SkRect</a>

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
bool contains(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>r</a>) <a href='SkRect_Reference#SkRect'>const</a>
</pre>

Returns true if <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>contains</a> <a href='#SkRect_contains_2_r'>r</a>.
Returns false if <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>empty</a> <a href='SkRect_Reference#SkRect'>or</a> <a href='#SkRect_contains_2_r'>r</a> <a href='#SkRect_contains_2_r'>is</a> <a href='#SkRect_contains_2_r'>empty</a>.

<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>contains</a> <a href='#SkRect_contains_2_r'>r</a> <a href='#SkRect_contains_2_r'>when</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>area</a> <a href='SkRect_Reference#SkRect'>completely</a> <a href='SkRect_Reference#SkRect'>includes</a> <a href='#SkRect_contains_2_r'>r</a> <a href='#SkRect_contains_2_r'>area</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_contains_2_r'><code><strong>r</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>contained</a></td>
  </tr>
</table>

### Return Value

true if all sides of <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>are</a> <a href='SkRect_Reference#SkRect'>outside</a> <a href='#SkRect_contains_2_r'>r</a>

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
bool contains(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>r</a>) <a href='SkIRect_Reference#SkIRect'>const</a>
</pre>

Returns true if <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>contains</a> <a href='#SkRect_contains_3_r'>r</a>.
Returns false if <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>empty</a> <a href='SkRect_Reference#SkRect'>or</a> <a href='#SkRect_contains_3_r'>r</a> <a href='#SkRect_contains_3_r'>is</a> <a href='#SkRect_contains_3_r'>empty</a>.

<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>contains</a> <a href='#SkRect_contains_3_r'>r</a> <a href='#SkRect_contains_3_r'>when</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>area</a> <a href='SkRect_Reference#SkRect'>completely</a> <a href='SkRect_Reference#SkRect'>includes</a> <a href='#SkRect_contains_3_r'>r</a> <a href='#SkRect_contains_3_r'>area</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_contains_3_r'><code><strong>r</strong></code></a></td>
    <td><a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>contained</a></td>
  </tr>
</table>

### Return Value

true if all sides of <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>are</a> <a href='SkRect_Reference#SkRect'>outside</a> <a href='#SkRect_contains_3_r'>r</a>

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
bool intersect(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>r</a>)
</pre>

Returns true if <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>intersects</a> <a href='#SkRect_intersect_r'>r</a>, <a href='#SkRect_intersect_r'>and</a> <a href='#SkRect_intersect_r'>sets</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>intersection</a>.
Returns false if <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>does</a> <a href='SkRect_Reference#SkRect'>not</a> <a href='SkRect_Reference#SkRect'>intersect</a> <a href='#SkRect_intersect_r'>r</a>, <a href='#SkRect_intersect_r'>and</a> <a href='#SkRect_intersect_r'>leaves</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>unchanged</a>.

Returns false if either <a href='#SkRect_intersect_r'>r</a> <a href='#SkRect_intersect_r'>or</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>empty</a>, <a href='SkRect_Reference#SkRect'>leaving</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>unchanged</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_intersect_r'><code><strong>r</strong></code></a></td>
    <td>limit of result</td>
  </tr>
</table>

### Return Value

true if <a href='#SkRect_intersect_r'>r</a> <a href='#SkRect_intersect_r'>and</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>have</a> <a href='SkRect_Reference#SkRect'>area</a> <a href='SkRect_Reference#SkRect'>in</a> <a href='SkRect_Reference#SkRect'>common</a>

### Example

<div><fiddle-embed name="5d0b12e0ef6f1c181dddded4274230ca"><div>Two <a href='undocumented#SkDebugf'>SkDebugf</a> <a href='undocumented#SkDebugf'>calls</a> <a href='undocumented#SkDebugf'>are</a> <a href='undocumented#SkDebugf'>required</a>. <a href='undocumented#SkDebugf'>If</a> <a href='undocumented#SkDebugf'>the</a> <a href='undocumented#SkDebugf'>calls</a> <a href='undocumented#SkDebugf'>are</a> <a href='undocumented#SkDebugf'>combined</a>, <a href='undocumented#SkDebugf'>their</a> <a href='undocumented#SkDebugf'>arguments</a>
<a href='undocumented#SkDebugf'>may</a> <a href='undocumented#SkDebugf'>not</a> <a href='undocumented#SkDebugf'>be</a> <a href='undocumented#SkDebugf'>evaluated</a> <a href='undocumented#SkDebugf'>in</a> <a href='undocumented#SkDebugf'>left</a> <a href='undocumented#SkDebugf'>to</a> <a href='undocumented#SkDebugf'>right</a> <a href='undocumented#SkDebugf'>order</a>: <a href='undocumented#SkDebugf'>the</a> <a href='undocumented#SkDebugf'>printed</a> <a href='undocumented#SkDebugf'>intersection</a> <a href='undocumented#SkDebugf'>may</a>
<a href='undocumented#SkDebugf'>be</a> <a href='undocumented#SkDebugf'>before</a> <a href='undocumented#SkDebugf'>or</a> <a href='undocumented#SkDebugf'>after</a> <a href='undocumented#SkDebugf'>the</a> <a href='undocumented#SkDebugf'>call</a> <a href='undocumented#SkDebugf'>to</a> <a href='undocumented#SkDebugf'>intersect</a>.
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
bool intersect(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>left</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>top</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>right</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>bottom</a>)
</pre>

Constructs <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>intersect</a> <a href='SkRect_Reference#SkRect'>from</a> (<a href='#SkRect_intersect_2_left'>left</a>, <a href='#SkRect_intersect_2_top'>top</a>, <a href='#SkRect_intersect_2_right'>right</a>, <a href='#SkRect_intersect_2_bottom'>bottom</a>). <a href='#SkRect_intersect_2_bottom'>Does</a> <a href='#SkRect_intersect_2_bottom'>not</a> <a href='#SkRect_intersect_2_bottom'>sort</a>
construction.

Returns true if <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>intersects</a> <a href='SkRect_Reference#SkRect'>construction</a>, <a href='SkRect_Reference#SkRect'>and</a> <a href='SkRect_Reference#SkRect'>sets</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>intersection</a>.
Returns false if <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>does</a> <a href='SkRect_Reference#SkRect'>not</a> <a href='SkRect_Reference#SkRect'>intersect</a> <a href='SkRect_Reference#SkRect'>construction</a>, <a href='SkRect_Reference#SkRect'>and</a> <a href='SkRect_Reference#SkRect'>leaves</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>unchanged</a>.

Returns false if either construction or <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>empty</a>, <a href='SkRect_Reference#SkRect'>leaving</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>unchanged</a>.

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

true if construction and <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>have</a> <a href='SkRect_Reference#SkRect'>area</a> <a href='SkRect_Reference#SkRect'>in</a> <a href='SkRect_Reference#SkRect'>common</a>

### Example

<div><fiddle-embed name="5002f65a72def2787086a33131933e70"><div>Two <a href='undocumented#SkDebugf'>SkDebugf</a> <a href='undocumented#SkDebugf'>calls</a> <a href='undocumented#SkDebugf'>are</a> <a href='undocumented#SkDebugf'>required</a>. <a href='undocumented#SkDebugf'>If</a> <a href='undocumented#SkDebugf'>the</a> <a href='undocumented#SkDebugf'>calls</a> <a href='undocumented#SkDebugf'>are</a> <a href='undocumented#SkDebugf'>combined</a>, <a href='undocumented#SkDebugf'>their</a> <a href='undocumented#SkDebugf'>arguments</a>
<a href='undocumented#SkDebugf'>may</a> <a href='undocumented#SkDebugf'>not</a> <a href='undocumented#SkDebugf'>be</a> <a href='undocumented#SkDebugf'>evaluated</a> <a href='undocumented#SkDebugf'>in</a> <a href='#SkRect_intersect_2_left'>left</a> <a href='#SkRect_intersect_2_left'>to</a> <a href='#SkRect_intersect_2_right'>right</a> <a href='#SkRect_intersect_2_right'>order</a>: <a href='#SkRect_intersect_2_right'>the</a> <a href='#SkRect_intersect_2_right'>printed</a> <a href='#SkRect_intersect_2_right'>intersection</a> <a href='#SkRect_intersect_2_right'>may</a>
<a href='#SkRect_intersect_2_right'>be</a> <a href='#SkRect_intersect_2_right'>before</a> <a href='#SkRect_intersect_2_right'>or</a> <a href='#SkRect_intersect_2_right'>after</a> <a href='#SkRect_intersect_2_right'>the</a> <a href='#SkRect_intersect_2_right'>call</a> <a href='#SkRect_intersect_2_right'>to</a> <a href='#SkRect_intersect_2_right'>intersect</a>.
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
bool intersect(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>a</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>b</a>)
</pre>

Returns true if <a href='#SkRect_intersect_3_a'>a</a> <a href='#SkRect_intersect_3_a'>intersects</a> <a href='#SkRect_intersect_3_b'>b</a>, <a href='#SkRect_intersect_3_b'>and</a> <a href='#SkRect_intersect_3_b'>sets</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>intersection</a>.
Returns false if <a href='#SkRect_intersect_3_a'>a</a> <a href='#SkRect_intersect_3_a'>does</a> <a href='#SkRect_intersect_3_a'>not</a> <a href='#SkRect_intersect_3_a'>intersect</a> <a href='#SkRect_intersect_3_b'>b</a>, <a href='#SkRect_intersect_3_b'>and</a> <a href='#SkRect_intersect_3_b'>leaves</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>unchanged</a>.

Returns false if either <a href='#SkRect_intersect_3_a'>a</a> <a href='#SkRect_intersect_3_a'>or</a> <a href='#SkRect_intersect_3_b'>b</a> <a href='#SkRect_intersect_3_b'>is</a> <a href='#SkRect_intersect_3_b'>empty</a>, <a href='#SkRect_intersect_3_b'>leaving</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>unchanged</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_intersect_3_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>intersect</a></td>
  </tr>
  <tr>    <td><a name='SkRect_intersect_3_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>intersect</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkRect_intersect_3_a'>a</a> <a href='#SkRect_intersect_3_a'>and</a> <a href='#SkRect_intersect_3_b'>b</a> <a href='#SkRect_intersect_3_b'>have</a> <a href='#SkRect_intersect_3_b'>area</a> <a href='#SkRect_intersect_3_b'>in</a> <a href='#SkRect_intersect_3_b'>common</a>

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
bool intersects(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>left</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>top</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>right</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>bottom</a>) <a href='undocumented#SkScalar'>const</a>
</pre>

Constructs <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>intersect</a> <a href='SkRect_Reference#SkRect'>from</a> (<a href='#SkRect_intersects_left'>left</a>, <a href='#SkRect_intersects_top'>top</a>, <a href='#SkRect_intersects_right'>right</a>, <a href='#SkRect_intersects_bottom'>bottom</a>). <a href='#SkRect_intersects_bottom'>Does</a> <a href='#SkRect_intersects_bottom'>not</a> <a href='#SkRect_intersects_bottom'>sort</a>
construction.

Returns true if <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>intersects</a> <a href='SkRect_Reference#SkRect'>construction</a>.
Returns false if either construction or <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>empty</a>, <a href='SkRect_Reference#SkRect'>or</a> <a href='SkRect_Reference#SkRect'>do</a> <a href='SkRect_Reference#SkRect'>not</a> <a href='SkRect_Reference#SkRect'>intersect</a>.

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

true if construction and <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>have</a> <a href='SkRect_Reference#SkRect'>area</a> <a href='SkRect_Reference#SkRect'>in</a> <a href='SkRect_Reference#SkRect'>common</a>

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
bool intersects(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>r</a>) <a href='SkRect_Reference#SkRect'>const</a>
</pre>

Returns true if <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>intersects</a> <a href='#SkRect_intersects_2_r'>r</a>.
Returns false if either <a href='#SkRect_intersects_2_r'>r</a> <a href='#SkRect_intersects_2_r'>or</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>empty</a>, <a href='SkRect_Reference#SkRect'>or</a> <a href='SkRect_Reference#SkRect'>do</a> <a href='SkRect_Reference#SkRect'>not</a> <a href='SkRect_Reference#SkRect'>intersect</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_intersects_2_r'><code><strong>r</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>intersect</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkRect_intersects_2_r'>r</a> <a href='#SkRect_intersects_2_r'>and</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>have</a> <a href='SkRect_Reference#SkRect'>area</a> <a href='SkRect_Reference#SkRect'>in</a> <a href='SkRect_Reference#SkRect'>common</a>

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
static bool <a href='#SkRect_Intersects'>Intersects</a>(<a href='#SkRect_Intersects'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>a</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>b</a>)
</pre>

Returns true if <a href='#SkRect_Intersects_a'>a</a> <a href='#SkRect_Intersects_a'>intersects</a> <a href='#SkRect_Intersects_b'>b</a>.
Returns false if either <a href='#SkRect_Intersects_a'>a</a> <a href='#SkRect_Intersects_a'>or</a> <a href='#SkRect_Intersects_b'>b</a> <a href='#SkRect_Intersects_b'>is</a> <a href='#SkRect_Intersects_b'>empty</a>, <a href='#SkRect_Intersects_b'>or</a> <a href='#SkRect_Intersects_b'>do</a> <a href='#SkRect_Intersects_b'>not</a> <a href='#SkRect_Intersects_b'>intersect</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_Intersects_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>intersect</a></td>
  </tr>
  <tr>    <td><a name='SkRect_Intersects_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>intersect</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkRect_Intersects_a'>a</a> <a href='#SkRect_Intersects_a'>and</a> <a href='#SkRect_Intersects_b'>b</a> <a href='#SkRect_Intersects_b'>have</a> <a href='#SkRect_Intersects_b'>area</a> <a href='#SkRect_Intersects_b'>in</a> <a href='#SkRect_Intersects_b'>common</a>

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
void join(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>left</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>top</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>right</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>bottom</a>)
</pre>

Constructs <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>intersect</a> <a href='SkRect_Reference#SkRect'>from</a> (<a href='#SkRect_join_left'>left</a>, <a href='#SkRect_join_top'>top</a>, <a href='#SkRect_join_right'>right</a>, <a href='#SkRect_join_bottom'>bottom</a>). <a href='#SkRect_join_bottom'>Does</a> <a href='#SkRect_join_bottom'>not</a> <a href='#SkRect_join_bottom'>sort</a>
construction.

Sets <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>the</a> <a href='SkRect_Reference#SkRect'>union</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='SkRect_Reference#SkRect'>itself</a> <a href='SkRect_Reference#SkRect'>and</a> <a href='SkRect_Reference#SkRect'>the</a> <a href='SkRect_Reference#SkRect'>construction</a>.

Has no effect if construction is empty. Otherwise, if <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>empty</a>, <a href='SkRect_Reference#SkRect'>sets</a>
<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>construction</a>.

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
void join(const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>r</a>)
</pre>

Sets <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>the</a> <a href='SkRect_Reference#SkRect'>union</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='SkRect_Reference#SkRect'>itself</a> <a href='SkRect_Reference#SkRect'>and</a> <a href='#SkRect_join_2_r'>r</a>.

Has no effect if <a href='#SkRect_join_2_r'>r</a> <a href='#SkRect_join_2_r'>is</a> <a href='#SkRect_join_2_r'>empty</a>. <a href='#SkRect_join_2_r'>Otherwise</a>, <a href='#SkRect_join_2_r'>if</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>empty</a>, <a href='SkRect_Reference#SkRect'>sets</a>
<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='#SkRect_join_2_r'>r</a>.

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
void <a href='#SkRect_joinNonEmptyArg'>joinNonEmptyArg</a>(<a href='#SkRect_joinNonEmptyArg'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>r</a>)
</pre>

Sets <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>the</a> <a href='SkRect_Reference#SkRect'>union</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='SkRect_Reference#SkRect'>itself</a> <a href='SkRect_Reference#SkRect'>and</a> <a href='#SkRect_joinNonEmptyArg_r'>r</a>.

Asserts if <a href='#SkRect_joinNonEmptyArg_r'>r</a> <a href='#SkRect_joinNonEmptyArg_r'>is</a> <a href='#SkRect_joinNonEmptyArg_r'>empty</a> <a href='#SkRect_joinNonEmptyArg_r'>and</a> <a href='#SkRect_joinNonEmptyArg_r'>SK_DEBUG</a> <a href='#SkRect_joinNonEmptyArg_r'>is</a> <a href='#SkRect_joinNonEmptyArg_r'>defined</a>.
If <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>is</a> <a href='SkRect_Reference#SkRect'>empty</a>, <a href='SkRect_Reference#SkRect'>sets</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='#SkRect_joinNonEmptyArg_r'>r</a>.

May produce incorrect results if <a href='#SkRect_joinNonEmptyArg_r'>r</a> <a href='#SkRect_joinNonEmptyArg_r'>is</a> <a href='#SkRect_joinNonEmptyArg_r'>empty</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_joinNonEmptyArg_r'><code><strong>r</strong></code></a></td>
    <td>expansion <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="88439de2aa0911262c60c0eb506396cb"><div>Since <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>is</a> <a href='SkRect_Reference#Rect'>not</a> <a href='SkRect_Reference#Rect'>sorted</a>, <a href='SkRect_Reference#Rect'>first</a> <a href='SkRect_Reference#Rect'>result</a> <a href='SkRect_Reference#Rect'>is</a> <a href='SkRect_Reference#Rect'>copy</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>toJoin</a>.
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
void <a href='#SkRect_joinPossiblyEmptyRect'>joinPossiblyEmptyRect</a>(<a href='#SkRect_joinPossiblyEmptyRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>r</a>)
</pre>

Sets <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>the</a> <a href='SkRect_Reference#SkRect'>union</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='SkRect_Reference#SkRect'>itself</a> <a href='SkRect_Reference#SkRect'>and</a> <a href='SkRect_Reference#SkRect'>the</a> <a href='SkRect_Reference#SkRect'>construction</a>.

May produce incorrect results if <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>or</a> <a href='#SkRect_joinPossiblyEmptyRect_r'>r</a> <a href='#SkRect_joinPossiblyEmptyRect_r'>is</a> <a href='#SkRect_joinPossiblyEmptyRect_r'>empty</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_joinPossiblyEmptyRect_r'><code><strong>r</strong></code></a></td>
    <td>expansion <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a476548d0001296afd8e58c1eba1b70b"><div>Since <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>is</a> <a href='SkRect_Reference#Rect'>not</a> <a href='SkRect_Reference#Rect'>sorted</a>, <a href='SkRect_Reference#Rect'>first</a> <a href='SkRect_Reference#Rect'>result</a> <a href='SkRect_Reference#Rect'>is</a> <a href='SkRect_Reference#Rect'>not</a> <a href='SkRect_Reference#Rect'>useful</a>.
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
void round(<a href='SkIRect_Reference#SkIRect'>SkIRect</a>* <a href='SkIRect_Reference#SkIRect'>dst</a>) <a href='SkIRect_Reference#SkIRect'>const</a>
</pre>

Sets <a href='SkIRect_Reference#IRect'>IRect</a> <a href='SkIRect_Reference#IRect'>by</a> <a href='SkIRect_Reference#IRect'>adding</a> 0.5 <a href='SkIRect_Reference#IRect'>and</a> <a href='SkIRect_Reference#IRect'>discarding</a> <a href='SkIRect_Reference#IRect'>the</a> <a href='SkIRect_Reference#IRect'>fractional</a> <a href='SkIRect_Reference#IRect'>portion</a> <a href='SkIRect_Reference#IRect'>of</a> <a href='SkRect_Reference#Rect'>Rect</a>
<a href='SkRect_Reference#Rect'>members</a>, <a href='SkRect_Reference#Rect'>using</a> <code>(<a href='undocumented#SkScalarRoundToInt'>SkScalarRoundToInt</a>(<a href='#SkRect_fLeft'>fLeft</a>), <a href='undocumented#SkScalarRoundToInt'>SkScalarRoundToInt</a>(<a href='#SkRect_fTop'>fTop</a>),
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
void <a href='#SkRect_roundOut'>roundOut</a>(<a href='SkIRect_Reference#SkIRect'>SkIRect</a>* <a href='SkIRect_Reference#SkIRect'>dst</a>) <a href='SkIRect_Reference#SkIRect'>const</a>
</pre>

Sets <a href='SkIRect_Reference#IRect'>IRect</a> <a href='SkIRect_Reference#IRect'>by</a> <a href='SkIRect_Reference#IRect'>discarding</a> <a href='SkIRect_Reference#IRect'>the</a> <a href='SkIRect_Reference#IRect'>fractional</a> <a href='SkIRect_Reference#IRect'>portion</a> <a href='SkIRect_Reference#IRect'>of</a> <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>and</a> <a href='#SkRect_fTop'>fTop</a>; <a href='#SkRect_fTop'>and</a> <a href='#SkRect_fTop'>rounding</a>
<a href='#SkRect_fTop'>up</a> <a href='#SkRect_fRight'>fRight</a> <a href='#SkRect_fRight'>and</a> <a href='#SkRect_fBottom'>fBottom</a>, <a href='#SkRect_fBottom'>using</a>
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
void <a href='#SkRect_roundOut'>roundOut</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>dst</a>) <a href='SkRect_Reference#SkRect'>const</a>
</pre>

Sets <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>by</a> <a href='SkRect_Reference#Rect'>discarding</a> <a href='SkRect_Reference#Rect'>the</a> <a href='SkRect_Reference#Rect'>fractional</a> <a href='SkRect_Reference#Rect'>portion</a> <a href='SkRect_Reference#Rect'>of</a> <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>and</a> <a href='#SkRect_fTop'>fTop</a>; <a href='#SkRect_fTop'>and</a> <a href='#SkRect_fTop'>rounding</a>
<a href='#SkRect_fTop'>up</a> <a href='#SkRect_fRight'>fRight</a> <a href='#SkRect_fRight'>and</a> <a href='#SkRect_fBottom'>fBottom</a>, <a href='#SkRect_fBottom'>using</a>
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
void <a href='#SkRect_roundIn'>roundIn</a>(<a href='SkIRect_Reference#SkIRect'>SkIRect</a>* <a href='SkIRect_Reference#SkIRect'>dst</a>) <a href='SkIRect_Reference#SkIRect'>const</a>
</pre>

Sets <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>by</a> <a href='SkRect_Reference#Rect'>rounding</a> <a href='SkRect_Reference#Rect'>up</a> <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>and</a> <a href='#SkRect_fTop'>fTop</a>; <a href='#SkRect_fTop'>and</a> <a href='#SkRect_fTop'>discarding</a> <a href='#SkRect_fTop'>the</a> <a href='#SkRect_fTop'>fractional</a> <a href='#SkRect_fTop'>portion</a>
<a href='#SkRect_fTop'>of</a> <a href='#SkRect_fRight'>fRight</a> <a href='#SkRect_fRight'>and</a> <a href='#SkRect_fBottom'>fBottom</a>, <a href='#SkRect_fBottom'>using</a>
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
<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkRect_round'>round()</a> <a href='#SkRect_round'>const</a>
</pre>

Returns <a href='SkIRect_Reference#IRect'>IRect</a> <a href='SkIRect_Reference#IRect'>by</a> <a href='SkIRect_Reference#IRect'>adding</a> 0.5 <a href='SkIRect_Reference#IRect'>and</a> <a href='SkIRect_Reference#IRect'>discarding</a> <a href='SkIRect_Reference#IRect'>the</a> <a href='SkIRect_Reference#IRect'>fractional</a> <a href='SkIRect_Reference#IRect'>portion</a> <a href='SkIRect_Reference#IRect'>of</a> <a href='SkRect_Reference#Rect'>Rect</a>
<a href='SkRect_Reference#Rect'>members</a>, <a href='SkRect_Reference#Rect'>using</a> <code>(<a href='undocumented#SkScalarRoundToInt'>SkScalarRoundToInt</a>(<a href='#SkRect_fLeft'>fLeft</a>), <a href='undocumented#SkScalarRoundToInt'>SkScalarRoundToInt</a>(<a href='#SkRect_fTop'>fTop</a>),
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
<a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='#SkRect_roundOut'>roundOut</a>() <a href='#SkRect_roundOut'>const</a>
</pre>

Sets <a href='SkIRect_Reference#IRect'>IRect</a> <a href='SkIRect_Reference#IRect'>by</a> <a href='SkIRect_Reference#IRect'>discarding</a> <a href='SkIRect_Reference#IRect'>the</a> <a href='SkIRect_Reference#IRect'>fractional</a> <a href='SkIRect_Reference#IRect'>portion</a> <a href='SkIRect_Reference#IRect'>of</a> <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>and</a> <a href='#SkRect_fTop'>fTop</a>; <a href='#SkRect_fTop'>and</a> <a href='#SkRect_fTop'>rounding</a>
<a href='#SkRect_fTop'>up</a> <a href='#SkRect_fRight'>fRight</a> <a href='#SkRect_fRight'>and</a> <a href='#SkRect_fBottom'>fBottom</a>, <a href='#SkRect_fBottom'>using</a>
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

Swaps <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>and</a> <a href='#SkRect_fRight'>fRight</a> <a href='#SkRect_fRight'>if</a> <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>is</a> <a href='#SkRect_fLeft'>greater</a> <a href='#SkRect_fLeft'>than</a> <a href='#SkRect_fRight'>fRight</a>; <a href='#SkRect_fRight'>and</a> <a href='#SkRect_fRight'>swaps</a>
<a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>and</a> <a href='#SkRect_fBottom'>fBottom</a> <a href='#SkRect_fBottom'>if</a> <a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>is</a> <a href='#SkRect_fTop'>greater</a> <a href='#SkRect_fTop'>than</a> <a href='#SkRect_fBottom'>fBottom</a>. <a href='#SkRect_fBottom'>Result</a> <a href='#SkRect_fBottom'>may</a> <a href='#SkRect_fBottom'>be</a> <a href='#SkRect_fBottom'>empty</a>;
and <a href='#SkRect_width'>width()</a> <a href='#SkRect_width'>and</a> <a href='#SkRect_height'>height()</a> <a href='#SkRect_height'>will</a> <a href='#SkRect_height'>be</a> <a href='#SkRect_height'>zero</a> <a href='#SkRect_height'>or</a> <a href='#SkRect_height'>positive</a>.

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
<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkRect_makeSorted'>makeSorted</a>() <a href='#SkRect_makeSorted'>const</a>
</pre>

Returns <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>with</a> <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>and</a> <a href='#SkRect_fRight'>fRight</a> <a href='#SkRect_fRight'>swapped</a> <a href='#SkRect_fRight'>if</a> <a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>is</a> <a href='#SkRect_fLeft'>greater</a> <a href='#SkRect_fLeft'>than</a> <a href='#SkRect_fRight'>fRight</a>; <a href='#SkRect_fRight'>and</a>
with <a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>and</a> <a href='#SkRect_fBottom'>fBottom</a> <a href='#SkRect_fBottom'>swapped</a> <a href='#SkRect_fBottom'>if</a> <a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>is</a> <a href='#SkRect_fTop'>greater</a> <a href='#SkRect_fTop'>than</a> <a href='#SkRect_fBottom'>fBottom</a>. <a href='#SkRect_fBottom'>Result</a> <a href='#SkRect_fBottom'>may</a> <a href='#SkRect_fBottom'>be</a> <a href='#SkRect_fBottom'>empty</a>;
and <a href='#SkRect_width'>width()</a> <a href='#SkRect_width'>and</a> <a href='#SkRect_height'>height()</a> <a href='#SkRect_height'>will</a> <a href='#SkRect_height'>be</a> <a href='#SkRect_height'>zero</a> <a href='#SkRect_height'>or</a> <a href='#SkRect_height'>positive</a>.

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
const <a href='undocumented#SkScalar'>SkScalar</a>* <a href='#SkRect_asScalars'>asScalars</a>() <a href='#SkRect_asScalars'>const</a>
</pre>

Returns pointer to first <a href='undocumented#Scalar'>scalar</a> <a href='undocumented#Scalar'>in</a> <a href='SkRect_Reference#SkRect'>SkRect</a>, <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>treat</a> <a href='SkRect_Reference#SkRect'>it</a> <a href='SkRect_Reference#SkRect'>as</a> <a href='SkRect_Reference#SkRect'>an</a> <a href='SkRect_Reference#SkRect'>array</a> <a href='SkRect_Reference#SkRect'>with</a> <a href='SkRect_Reference#SkRect'>four</a>
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
void <a href='#SkRect_dump'>dump</a>(<a href='#SkRect_dump'>bool</a> <a href='#SkRect_dump'>asHex</a>) <a href='#SkRect_dump'>const</a>
</pre>

Writes <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>representation</a> <a href='undocumented#Text'>of</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>standard</a> <a href='SkRect_Reference#SkRect'>output</a>. <a href='SkRect_Reference#SkRect'>Set</a> <a href='#SkRect_dump_asHex'>asHex</a> <a href='#SkRect_dump_asHex'>to</a> <a href='#SkRect_dump_asHex'>true</a> <a href='#SkRect_dump_asHex'>to</a>
generate exact binary representations of floating <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>numbers</a>.

### Parameters

<table>  <tr>    <td><a name='SkRect_dump_asHex'><code><strong>asHex</strong></code></a></td>
    <td>true if <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>values</a> <a href='undocumented#SkScalar'>are</a> <a href='undocumented#SkScalar'>written</a> <a href='undocumented#SkScalar'>as</a> <a href='undocumented#SkScalar'>hexadecimal</a></td>
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
void <a href='#SkRect_dump'>dump()</a> <a href='#SkRect_dump'>const</a>
</pre>

Writes <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>representation</a> <a href='undocumented#Text'>of</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>standard</a> <a href='SkRect_Reference#SkRect'>output</a>. <a href='SkRect_Reference#SkRect'>The</a> <a href='SkRect_Reference#SkRect'>representation</a> <a href='SkRect_Reference#SkRect'>may</a> <a href='SkRect_Reference#SkRect'>be</a>
directly compiled as C++ code. Floating <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>values</a> <a href='SkPoint_Reference#Point'>are</a> <a href='SkPoint_Reference#Point'>written</a>
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
void <a href='#SkRect_dumpHex'>dumpHex</a>() <a href='#SkRect_dumpHex'>const</a>
</pre>

Writes <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>representation</a> <a href='undocumented#Text'>of</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>to</a> <a href='SkRect_Reference#Rect'>standard</a> <a href='SkRect_Reference#Rect'>output</a>. <a href='SkRect_Reference#Rect'>The</a> <a href='SkRect_Reference#Rect'>representation</a> <a href='SkRect_Reference#Rect'>may</a> <a href='SkRect_Reference#Rect'>be</a>
<a href='SkRect_Reference#Rect'>directly</a> <a href='SkRect_Reference#Rect'>compiled</a> <a href='SkRect_Reference#Rect'>as</a> <a href='SkRect_Reference#Rect'>C</a>++ <a href='SkRect_Reference#Rect'>code</a>. <a href='SkRect_Reference#Rect'>Floating</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>values</a> <a href='SkPoint_Reference#Point'>are</a> <a href='SkPoint_Reference#Point'>written</a>
<a href='SkPoint_Reference#Point'>in</a> <a href='SkPoint_Reference#Point'>hexadecimal</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>preserve</a> <a href='SkPoint_Reference#Point'>their</a> <a href='SkPoint_Reference#Point'>exact</a> <a href='SkPoint_Reference#Point'>bit</a> <a href='SkPoint_Reference#Point'>pattern</a>. <a href='SkPoint_Reference#Point'>The</a> <a href='SkPoint_Reference#Point'>output</a> <a href='SkPoint_Reference#Point'>reconstructs</a> <a href='SkPoint_Reference#Point'>the</a>
<a href='SkPoint_Reference#Point'>original</a> <a href='SkRect_Reference#Rect'>Rect</a>.

<a href='SkRect_Reference#Rect'>Use</a> <a href='SkRect_Reference#Rect'>instead</a> <a href='SkRect_Reference#Rect'>of</a> <a href='#SkRect_dump'>dump()</a> <a href='#SkRect_dump'>when</a> <a href='#SkRect_dump'>submitting</a>
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

