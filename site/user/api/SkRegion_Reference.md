SkRegion Reference
===
<a href='SkRegion_Reference#Region'>Region</a> <a href='SkRegion_Reference#Region'>is</a> <a href='SkRegion_Reference#Region'>a</a> <a href='SkRegion_Reference#Region'>compressed</a> <a href='SkRegion_Reference#Region'>one</a> <a href='SkRegion_Reference#Region'>bit</a> <a href='SkRegion_Reference#Region'>mask</a>. <a href='SkRegion_Reference#Region'>Region</a> <a href='SkRegion_Reference#Region'>describes</a> <a href='SkRegion_Reference#Region'>an</a> <a href='undocumented#Alias'>aliased</a> <a href='undocumented#Alias'>clipping</a> <a href='undocumented#Alias'>area</a>
<a href='undocumented#Alias'>on</a> <a href='undocumented#Alias'>integer</a> <a href='undocumented#Alias'>boundaries</a>. <a href='SkRegion_Reference#Region'>Region</a> <a href='SkRegion_Reference#Region'>can</a> <a href='SkRegion_Reference#Region'>also</a> <a href='SkRegion_Reference#Region'>describe</a> <a href='SkRegion_Reference#Region'>an</a> <a href='SkRegion_Reference#Region'>array</a> <a href='SkRegion_Reference#Region'>of</a> <a href='SkRegion_Reference#Region'>integer</a> <a href='SkRegion_Reference#Region'>rectangles</a>.

<a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>uses</a> <a href='SkRegion_Reference#Region'>Region</a> <a href='SkRegion_Reference#Region'>to</a> <a href='SkRegion_Reference#Region'>reduce</a> <a href='SkRegion_Reference#Region'>the</a> <a href='SkRegion_Reference#Region'>current</a> <a href='SkRegion_Reference#Region'>clip</a>. <a href='SkRegion_Reference#Region'>Region</a> <a href='SkRegion_Reference#Region'>may</a> <a href='SkRegion_Reference#Region'>be</a> <a href='SkRegion_Reference#Region'>drawn</a> <a href='SkRegion_Reference#Region'>to</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a>;
<a href='SkPaint_Reference#Paint'>Paint</a> <a href='SkPaint_Reference#Paint'>determines</a> <a href='SkPaint_Reference#Paint'>if</a> <a href='SkRegion_Reference#Region'>Region</a> <a href='SkRegion_Reference#Region'>is</a> <a href='SkRegion_Reference#Region'>filled</a> <a href='SkRegion_Reference#Region'>or</a> <a href='SkRegion_Reference#Region'>stroked</a>, <a href='SkRegion_Reference#Region'>its</a> <a href='SkColor_Reference#Color'>Color</a>, <a href='SkColor_Reference#Color'>and</a> <a href='SkColor_Reference#Color'>so</a> <a href='SkColor_Reference#Color'>on</a>.

<a href='SkRegion_Reference#Region'>Region</a> <a href='SkRegion_Reference#Region'>may</a> <a href='SkRegion_Reference#Region'>be</a> <a href='SkRegion_Reference#Region'>constructed</a> <a href='SkRegion_Reference#Region'>from</a> <a href='SkIRect_Reference#IRect'>IRect</a> <a href='SkIRect_Reference#IRect'>array</a> <a href='SkIRect_Reference#IRect'>or</a> <a href='SkPath_Reference#Path'>Path</a>. <a href='SkPath_Reference#Path'>Diagonal</a> <a href='undocumented#Line'>lines</a> <a href='undocumented#Line'>and</a> <a href='undocumented#Curve'>curves</a>
<a href='undocumented#Curve'>in</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>become</a> <a href='SkPath_Reference#Path'>integer</a> <a href='SkPath_Reference#Path'>rectangle</a> <a href='SkPath_Reference#Path'>edges</a>. <a href='SkRegion_Reference#Region'>Regions</a> <a href='SkRegion_Reference#Region'>operators</a> <a href='SkRegion_Reference#Region'>compute</a> <a href='SkRegion_Reference#Region'>union</a>,
<a href='SkRegion_Reference#Region'>intersection</a>, <a href='SkRegion_Reference#Region'>difference</a>, <a href='SkRegion_Reference#Region'>and</a> <a href='SkRegion_Reference#Region'>so</a> <a href='SkRegion_Reference#Region'>on</a>. <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>allows</a> <a href='SkCanvas_Reference#Canvas'>only</a> <a href='SkCanvas_Reference#Canvas'>intersection</a> <a href='SkCanvas_Reference#Canvas'>and</a>
<a href='SkCanvas_Reference#Canvas'>difference</a>; <a href='SkCanvas_Reference#Canvas'>successive</a> <a href='SkCanvas_Reference#Canvas'>clips</a> <a href='SkCanvas_Reference#Canvas'>can</a> <a href='SkCanvas_Reference#Canvas'>only</a> <a href='SkCanvas_Reference#Canvas'>reduce</a> <a href='SkCanvas_Reference#Canvas'>available</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>area</a>.

<a name='SkRegion'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkRegion_Reference#SkRegion'>SkRegion</a> {
<a href='SkRegion_Reference#SkRegion'>public</a>:
    <a href='#SkRegion_empty_constructor'>SkRegion()</a>;
    <a href='SkRegion_Reference#SkRegion'>SkRegion</a>(<a href='SkRegion_Reference#SkRegion'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#Region'>region</a>);
    <a href='SkRegion_Reference#Region'>explicit</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>(<a href='SkRegion_Reference#SkRegion'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkRect_Reference#Rect'>rect</a>);
    ~<a href='#SkRegion_empty_constructor'>SkRegion()</a>;
    <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>operator</a>=(<a href='SkRegion_Reference#SkRegion'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#Region'>region</a>);
    <a href='SkRegion_Reference#Region'>bool</a> <a href='SkRegion_Reference#Region'>operator</a>==(<a href='SkRegion_Reference#Region'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>other</a>) <a href='SkRegion_Reference#SkRegion'>const</a>;
    <a href='SkRegion_Reference#SkRegion'>bool</a> <a href='SkRegion_Reference#SkRegion'>operator</a>!=(<a href='SkRegion_Reference#SkRegion'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>other</a>) <a href='SkRegion_Reference#SkRegion'>const</a>;
    <a href='SkRegion_Reference#SkRegion'>bool</a> <a href='SkRegion_Reference#SkRegion'>set</a>(<a href='SkRegion_Reference#SkRegion'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>src</a>);
    <a href='SkRegion_Reference#SkRegion'>void</a> <a href='#SkRegion_swap'>swap</a>(<a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>other</a>);
    <a href='SkRegion_Reference#SkRegion'>bool</a> <a href='#SkRegion_isEmpty'>isEmpty</a>() <a href='#SkRegion_isEmpty'>const</a>;
    <a href='#SkRegion_isEmpty'>bool</a> <a href='#SkRegion_isRect'>isRect</a>() <a href='#SkRegion_isRect'>const</a>;
    <a href='#SkRegion_isRect'>bool</a> <a href='#SkRegion_isComplex'>isComplex</a>() <a href='#SkRegion_isComplex'>const</a>;
    <a href='#SkRegion_isComplex'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='#SkRegion_getBounds'>getBounds</a>() <a href='#SkRegion_getBounds'>const</a>;
    <a href='#SkRegion_getBounds'>int</a> <a href='#SkRegion_computeRegionComplexity'>computeRegionComplexity</a>() <a href='#SkRegion_computeRegionComplexity'>const</a>;
    <a href='#SkRegion_computeRegionComplexity'>bool</a> <a href='#SkRegion_getBoundaryPath'>getBoundaryPath</a>(<a href='SkPath_Reference#SkPath'>SkPath</a>* <a href='SkPath_Reference#Path'>path</a>) <a href='SkPath_Reference#Path'>const</a>;
    <a href='SkPath_Reference#Path'>bool</a> <a href='#SkRegion_setEmpty'>setEmpty</a>();
    <a href='#SkRegion_setEmpty'>bool</a> <a href='#SkRegion_setRect'>setRect</a>(<a href='#SkRegion_setRect'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkRect_Reference#Rect'>rect</a>);
    <a href='SkRect_Reference#Rect'>bool</a> <a href='#SkRegion_setRect'>setRect</a>(<a href='#SkRegion_setRect'>int32_t</a> <a href='#SkRegion_setRect'>left</a>, <a href='#SkRegion_setRect'>int32_t</a> <a href='#SkRegion_setRect'>top</a>, <a href='#SkRegion_setRect'>int32_t</a> <a href='#SkRegion_setRect'>right</a>, <a href='#SkRegion_setRect'>int32_t</a> <a href='#SkRegion_setRect'>bottom</a>);
    <a href='#SkRegion_setRect'>bool</a> <a href='#SkRegion_setRects'>setRects</a>(<a href='#SkRegion_setRects'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkRect_Reference#Rect'>rects</a>[], <a href='SkRect_Reference#Rect'>int</a> <a href='SkRect_Reference#Rect'>count</a>);
    <a href='SkRect_Reference#Rect'>bool</a> <a href='#SkRegion_setRegion'>setRegion</a>(<a href='#SkRegion_setRegion'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#Region'>region</a>);
    <a href='SkRegion_Reference#Region'>bool</a> <a href='#SkRegion_setPath'>setPath</a>(<a href='#SkRegion_setPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>, <a href='SkPath_Reference#Path'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>clip</a>);
    <a href='SkRegion_Reference#SkRegion'>bool</a> <a href='SkRegion_Reference#SkRegion'>intersects</a>(<a href='SkRegion_Reference#SkRegion'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkRect_Reference#Rect'>rect</a>) <a href='SkRect_Reference#Rect'>const</a>;
    <a href='SkRect_Reference#Rect'>bool</a> <a href='SkRect_Reference#Rect'>intersects</a>(<a href='SkRect_Reference#Rect'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>other</a>) <a href='SkRegion_Reference#SkRegion'>const</a>;
    <a href='SkRegion_Reference#SkRegion'>bool</a> <a href='SkRegion_Reference#SkRegion'>contains</a>(<a href='SkRegion_Reference#SkRegion'>int32_t</a> <a href='SkRegion_Reference#SkRegion'>x</a>, <a href='SkRegion_Reference#SkRegion'>int32_t</a> <a href='SkRegion_Reference#SkRegion'>y</a>) <a href='SkRegion_Reference#SkRegion'>const</a>;
    <a href='SkRegion_Reference#SkRegion'>bool</a> <a href='SkRegion_Reference#SkRegion'>contains</a>(<a href='SkRegion_Reference#SkRegion'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>other</a>) <a href='SkIRect_Reference#SkIRect'>const</a>;
    <a href='SkIRect_Reference#SkIRect'>bool</a> <a href='SkIRect_Reference#SkIRect'>contains</a>(<a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>other</a>) <a href='SkRegion_Reference#SkRegion'>const</a>;
    <a href='SkRegion_Reference#SkRegion'>bool</a> <a href='#SkRegion_quickContains'>quickContains</a>(<a href='#SkRegion_quickContains'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>r</a>) <a href='SkIRect_Reference#SkIRect'>const</a>;
    <a href='SkIRect_Reference#SkIRect'>bool</a> <a href='#SkRegion_quickContains'>quickContains</a>(<a href='#SkRegion_quickContains'>int32_t</a> <a href='#SkRegion_quickContains'>left</a>, <a href='#SkRegion_quickContains'>int32_t</a> <a href='#SkRegion_quickContains'>top</a>, <a href='#SkRegion_quickContains'>int32_t</a> <a href='#SkRegion_quickContains'>right</a>,
                       <a href='#SkRegion_quickContains'>int32_t</a> <a href='#SkRegion_quickContains'>bottom</a>) <a href='#SkRegion_quickContains'>const</a>;
    <a href='#SkRegion_quickContains'>bool</a> <a href='#SkRegion_quickReject'>quickReject</a>(<a href='#SkRegion_quickReject'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkRect_Reference#Rect'>rect</a>) <a href='SkRect_Reference#Rect'>const</a>;
    <a href='SkRect_Reference#Rect'>bool</a> <a href='#SkRegion_quickReject'>quickReject</a>(<a href='#SkRegion_quickReject'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>rgn</a>) <a href='SkRegion_Reference#SkRegion'>const</a>;
    <a href='SkRegion_Reference#SkRegion'>void</a> <a href='SkRegion_Reference#SkRegion'>translate</a>(<a href='SkRegion_Reference#SkRegion'>int</a> <a href='SkRegion_Reference#SkRegion'>dx</a>, <a href='SkRegion_Reference#SkRegion'>int</a> <a href='SkRegion_Reference#SkRegion'>dy</a>);
    <a href='SkRegion_Reference#SkRegion'>void</a> <a href='SkRegion_Reference#SkRegion'>translate</a>(<a href='SkRegion_Reference#SkRegion'>int</a> <a href='SkRegion_Reference#SkRegion'>dx</a>, <a href='SkRegion_Reference#SkRegion'>int</a> <a href='SkRegion_Reference#SkRegion'>dy</a>, <a href='SkRegion_Reference#SkRegion'>SkRegion</a>* <a href='SkRegion_Reference#SkRegion'>dst</a>) <a href='SkRegion_Reference#SkRegion'>const</a>;

    <a href='SkRegion_Reference#SkRegion'>enum</a> <a href='#SkRegion_Op'>Op</a> {
        <a href='#SkRegion_kDifference_Op'>kDifference_Op</a>,
        <a href='#SkRegion_kIntersect_Op'>kIntersect_Op</a>,
        <a href='#SkRegion_kUnion_Op'>kUnion_Op</a>,
        <a href='#SkRegion_kXOR_Op'>kXOR_Op</a>,
        <a href='#SkRegion_kReverseDifference_Op'>kReverseDifference_Op</a>,
        <a href='#SkRegion_kReplace_Op'>kReplace_Op</a>,
        <a href='#SkRegion_kLastOp'>kLastOp</a> = <a href='#SkRegion_kReplace_Op'>kReplace_Op</a>,
    };

    <a href='#SkRegion_kReplace_Op'>static</a> <a href='#SkRegion_kReplace_Op'>const</a> <a href='#SkRegion_kReplace_Op'>int</a> <a href='#SkRegion_kOpCnt'>kOpCnt</a> = <a href='#SkRegion_kLastOp'>kLastOp</a> + 1;
    <a href='#SkRegion_kLastOp'>bool</a> <a href='#SkRegion_op'>op</a>(<a href='#SkRegion_op'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='#SkRegion_Op'>Op</a> <a href='#SkRegion_Op'>op</a>);
    <a href='#SkRegion_Op'>bool</a> <a href='#SkRegion_op'>op</a>(<a href='#SkRegion_op'>int</a> <a href='#SkRegion_op'>left</a>, <a href='#SkRegion_op'>int</a> <a href='#SkRegion_op'>top</a>, <a href='#SkRegion_op'>int</a> <a href='#SkRegion_op'>right</a>, <a href='#SkRegion_op'>int</a> <a href='#SkRegion_op'>bottom</a>, <a href='#SkRegion_Op'>Op</a> <a href='#SkRegion_Op'>op</a>);
    <a href='#SkRegion_Op'>bool</a> <a href='#SkRegion_op'>op</a>(<a href='#SkRegion_op'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>rgn</a>, <a href='#SkRegion_Op'>Op</a> <a href='#SkRegion_Op'>op</a>);
    <a href='#SkRegion_Op'>bool</a> <a href='#SkRegion_op'>op</a>(<a href='#SkRegion_op'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='SkRect_Reference#Rect'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>rgn</a>, <a href='#SkRegion_Op'>Op</a> <a href='#SkRegion_Op'>op</a>);
    <a href='#SkRegion_Op'>bool</a> <a href='#SkRegion_op'>op</a>(<a href='#SkRegion_op'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>rgn</a>, <a href='SkRegion_Reference#SkRegion'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='#SkRegion_Op'>Op</a> <a href='#SkRegion_Op'>op</a>);
    <a href='#SkRegion_Op'>bool</a> <a href='#SkRegion_op'>op</a>(<a href='#SkRegion_op'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>rgna</a>, <a href='SkRegion_Reference#SkRegion'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>rgnb</a>, <a href='#SkRegion_Op'>Op</a> <a href='#SkRegion_Op'>op</a>);
    <a href='#SkRegion_Op'>char</a>* <a href='#SkRegion_toString'>toString</a>();

    <a href='#SkRegion_toString'>class</a> <a href='#SkRegion_Iterator'>Iterator</a> {
    <a href='#SkRegion_Iterator'>public</a>:
        <a href='#SkRegion_Iterator'>Iterator</a>();
        <a href='#SkRegion_Iterator'>Iterator</a>(<a href='#SkRegion_Iterator'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#Region'>region</a>);
        <a href='SkRegion_Reference#Region'>bool</a> <a href='SkRegion_Reference#Region'>rewind()</a>;
        <a href='SkRegion_Reference#Region'>void</a> <a href='SkRegion_Reference#Region'>reset</a>(<a href='SkRegion_Reference#Region'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#Region'>region</a>);
        <a href='SkRegion_Reference#Region'>bool</a> <a href='SkRegion_Reference#Region'>done()</a> <a href='SkRegion_Reference#Region'>const</a>;
        <a href='SkRegion_Reference#Region'>void</a> <a href='SkRegion_Reference#Region'>next()</a>;
        <a href='SkRegion_Reference#Region'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>rect()</a> <a href='SkIRect_Reference#SkIRect'>const</a>;
        <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>* <a href='SkRegion_Reference#SkRegion'>rgn()</a> <a href='SkRegion_Reference#SkRegion'>const</a>;
    };

    <a href='SkRegion_Reference#SkRegion'>class</a> <a href='#SkRegion_Cliperator'>Cliperator</a> {
    <a href='#SkRegion_Cliperator'>public</a>:
        <a href='#SkRegion_Cliperator'>Cliperator</a>(<a href='#SkRegion_Cliperator'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#Region'>region</a>, <a href='SkRegion_Reference#Region'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>clip</a>);
        <a href='SkIRect_Reference#SkIRect'>bool</a> <a href='SkIRect_Reference#SkIRect'>done()</a>;
        <a href='SkIRect_Reference#SkIRect'>void</a> <a href='SkIRect_Reference#SkIRect'>next()</a>;
        <a href='SkIRect_Reference#SkIRect'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>rect()</a> <a href='SkIRect_Reference#SkIRect'>const</a>;
    };

    <a href='SkIRect_Reference#SkIRect'>class</a> <a href='#SkRegion_Spanerator'>Spanerator</a> {
    <a href='#SkRegion_Spanerator'>public</a>:
        <a href='#SkRegion_Spanerator'>Spanerator</a>(<a href='#SkRegion_Spanerator'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#Region'>region</a>, <a href='SkRegion_Reference#Region'>int</a> <a href='SkRegion_Reference#Region'>y</a>, <a href='SkRegion_Reference#Region'>int</a> <a href='SkRegion_Reference#Region'>left</a>, <a href='SkRegion_Reference#Region'>int</a> <a href='SkRegion_Reference#Region'>right</a>);
        <a href='SkRegion_Reference#Region'>bool</a> <a href='SkRegion_Reference#Region'>next</a>(<a href='SkRegion_Reference#Region'>int</a>* <a href='SkRegion_Reference#Region'>left</a>, <a href='SkRegion_Reference#Region'>int</a>* <a href='SkRegion_Reference#Region'>right</a>);
    };

    <a href='SkRegion_Reference#Region'>size_t</a> <a href='#SkRegion_writeToMemory'>writeToMemory</a>(<a href='#SkRegion_writeToMemory'>void</a>* <a href='#SkRegion_writeToMemory'>buffer</a>) <a href='#SkRegion_writeToMemory'>const</a>;
    <a href='#SkRegion_writeToMemory'>size_t</a> <a href='#SkRegion_readFromMemory'>readFromMemory</a>(<a href='#SkRegion_readFromMemory'>const</a> <a href='#SkRegion_readFromMemory'>void</a>* <a href='#SkRegion_readFromMemory'>buffer</a>, <a href='#SkRegion_readFromMemory'>size_t</a> <a href='#SkRegion_readFromMemory'>length</a>);
};
</pre>

<a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>describes</a> <a href='SkRegion_Reference#SkRegion'>the</a> <a href='SkRegion_Reference#SkRegion'>set</a> <a href='SkRegion_Reference#SkRegion'>of</a> <a href='SkRegion_Reference#SkRegion'>pixels</a> <a href='SkRegion_Reference#SkRegion'>used</a> <a href='SkRegion_Reference#SkRegion'>to</a> <a href='SkRegion_Reference#SkRegion'>clip</a> <a href='SkCanvas_Reference#Canvas'>Canvas</a>. <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>compact</a>,
<a href='SkRegion_Reference#SkRegion'>efficiently</a> <a href='SkRegion_Reference#SkRegion'>storing</a> <a href='SkRegion_Reference#SkRegion'>a</a> <a href='SkRegion_Reference#SkRegion'>single</a> <a href='SkRegion_Reference#SkRegion'>integer</a> <a href='SkRegion_Reference#SkRegion'>rectangle</a>, <a href='SkRegion_Reference#SkRegion'>or</a> <a href='SkRegion_Reference#SkRegion'>a</a> <a href='SkRegion_Reference#SkRegion'>run</a> <a href='SkRegion_Reference#SkRegion'>length</a> <a href='SkRegion_Reference#SkRegion'>encoded</a> <a href='SkRegion_Reference#SkRegion'>array</a>
<a href='SkRegion_Reference#SkRegion'>of</a> <a href='SkRegion_Reference#SkRegion'>rectangles</a>. <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>may</a> <a href='SkRegion_Reference#SkRegion'>reduce</a> <a href='SkRegion_Reference#SkRegion'>the</a> <a href='SkRegion_Reference#SkRegion'>current</a> <a href='#Canvas_Clip'>Canvas_Clip</a>, <a href='#Canvas_Clip'>or</a> <a href='#Canvas_Clip'>may</a> <a href='#Canvas_Clip'>be</a> <a href='#Canvas_Clip'>drawn</a> <a href='#Canvas_Clip'>as</a>
<a href='#Canvas_Clip'>one</a> <a href='#Canvas_Clip'>or</a> <a href='#Canvas_Clip'>more</a> <a href='#Canvas_Clip'>integer</a> <a href='#Canvas_Clip'>rectangles</a>. <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>iterator</a> <a href='SkRegion_Reference#SkRegion'>returns</a> <a href='SkRegion_Reference#SkRegion'>the</a> <a href='SkRegion_Reference#SkRegion'>scan</a> <a href='undocumented#Line'>lines</a> <a href='undocumented#Line'>or</a>
<a href='undocumented#Line'>rectangles</a> <a href='undocumented#Line'>contained</a> <a href='undocumented#Line'>by</a> <a href='undocumented#Line'>it</a>, <a href='undocumented#Line'>optionally</a> <a href='undocumented#Line'>intersecting</a> <a href='undocumented#Line'>a</a> <a href='undocumented#Line'>bounding</a> <a href='undocumented#Line'>rectangle</a>.

<a name='SkRegion_Iterator'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    class <a href='#SkRegion_Iterator'>Iterator</a> {
    <a href='#SkRegion_Iterator'>public</a>:
        <a href='#SkRegion_Iterator_Iterator'>Iterator()</a>;
        <a href='#SkRegion_Iterator'>Iterator</a>(<a href='#SkRegion_Iterator'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#Region'>region</a>);
        <a href='SkRegion_Reference#Region'>bool</a> <a href='#SkRegion_Iterator_rewind'>rewind()</a>;
        <a href='#SkRegion_Iterator_rewind'>void</a> <a href='#SkRegion_Iterator_rewind'>reset</a>(<a href='#SkRegion_Iterator_rewind'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#Region'>region</a>);
        <a href='SkRegion_Reference#Region'>bool</a> <a href='#SkRegion_Iterator_done'>done()</a> <a href='#SkRegion_Iterator_done'>const</a>;
        <a href='#SkRegion_Iterator_done'>void</a> <a href='#SkRegion_Iterator_next'>next()</a>;
        <a href='#SkRegion_Iterator_next'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='#SkRegion_Iterator_rect'>rect()</a>;
        <a href='#SkRegion_Iterator_rect'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>* <a href='#SkRegion_Iterator_rgn'>rgn()</a>;
    };
</pre>

Returns sequence of rectangles, sorted along y-axis, then x-axis, that make
up <a href='SkRegion_Reference#Region'>Region</a>.

<a name='SkRegion_Iterator_Iterator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkRegion_Iterator_Iterator'>Iterator()</a>
</pre>

Initializes <a href='SkRegion_Reference#SkRegion'>SkRegion</a>::<a href='#SkRegion_Iterator'>Iterator</a> <a href='#SkRegion_Iterator'>with</a> <a href='#SkRegion_Iterator'>an</a> <a href='#SkRegion_Iterator'>empty</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>. <a href='#SkRegion_Iterator_done'>done()</a> <a href='#SkRegion_Iterator_done'>on</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>::<a href='#SkRegion_Iterator'>Iterator</a>
returns true.
Call <a href='#SkRegion_Iterator_reset'>reset()</a> <a href='#SkRegion_Iterator_reset'>to</a> <a href='#SkRegion_Iterator_reset'>initialized</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>::<a href='#SkRegion_Iterator'>Iterator</a> <a href='#SkRegion_Iterator'>at</a> <a href='#SkRegion_Iterator'>a</a> <a href='#SkRegion_Iterator'>later</a> <a href='#SkRegion_Iterator'>time</a>.

### Return Value

empty <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>iterator</a>

### Example

<div><fiddle-embed name="a2db43ee3cbf6893e9b23927fb44298a">

#### Example Output

~~~~
rect={1,2,3,4}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_Iterator_reset'>reset</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>

<a name='SkRegion_Iterator_copy_const_SkRegion'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkRegion_Iterator'>Iterator</a>(<a href='#SkRegion_Iterator'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#Region'>region</a>)
</pre>

Sets <a href='SkRegion_Reference#SkRegion'>SkRegion</a>::<a href='#SkRegion_Iterator'>Iterator</a> <a href='#SkRegion_Iterator'>to</a> <a href='#SkRegion_Iterator'>return</a> <a href='#SkRegion_Iterator'>elements</a> <a href='#SkRegion_Iterator'>of</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>array</a> <a href='SkIRect_Reference#SkIRect'>in</a> <a href='#SkRegion_Iterator_copy_const_SkRegion_region'>region</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_Iterator_copy_const_SkRegion_region'><code><strong>region</strong></code></a></td>
    <td><a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>to</a> <a href='SkRegion_Reference#SkRegion'>iterate</a></td>
  </tr>
</table>

### Return Value

<a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>iterator</a>

### Example

<div><fiddle-embed name="e317ceca48a6a7504219af58f35d2c95">

#### Example Output

~~~~
rect={1,2,3,4}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_Iterator_reset'>reset</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='#SkRegion_Cliperator'>Cliperator</a> <a href='#SkRegion_Spanerator'>Spanerator</a>

<a name='SkRegion_Iterator_rewind'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRegion_Iterator_rewind'>rewind()</a>
</pre>

<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>::<a href='#SkRegion_Iterator'>Iterator</a> <a href='#SkRegion_Iterator'>to</a> <a href='#SkRegion_Iterator'>start</a> <a href='#SkRegion_Iterator'>of</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>.
Returns true if <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>was</a> <a href='SkRegion_Reference#SkRegion'>set</a>; <a href='SkRegion_Reference#SkRegion'>otherwise</a>, <a href='SkRegion_Reference#SkRegion'>returns</a> <a href='SkRegion_Reference#SkRegion'>false</a>.

### Return Value

true if <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>was</a> <a href='SkRegion_Reference#SkRegion'>set</a>

### Example

<div><fiddle-embed name="32d51e959d6cc720a74ec4822511e2cd">

#### Example Output

~~~~
#Volatile
empty iter rewind success=false
empty iter rect={0,0,0,0}
empty region rewind success=true
empty region rect={0,0,0,0}
after set rect rect={1,2,3,4}
after rewind rewind success=true
after rewind rect={1,2,3,4}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_Iterator_reset'>reset</a>

<a name='SkRegion_Iterator_reset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void reset(const <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#Region'>region</a>)
</pre>

Resets iterator, using the new <a href='SkRegion_Reference#SkRegion'>SkRegion</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_Iterator_reset_region'><code><strong>region</strong></code></a></td>
    <td><a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>to</a> <a href='SkRegion_Reference#SkRegion'>iterate</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d153f87bd518a4ab947b7e407ea1db79">

#### Example Output

~~~~
empty region: done=true
after set rect: done=true
after reset: done=false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_Iterator_rewind'>rewind</a>

<a name='SkRegion_Iterator_done'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRegion_Iterator_done'>done()</a> <a href='#SkRegion_Iterator_done'>const</a>
</pre>

Returns true if <a href='SkRegion_Reference#SkRegion'>SkRegion</a>::<a href='#SkRegion_Iterator'>Iterator</a> <a href='#SkRegion_Iterator'>is</a> <a href='#SkRegion_Iterator'>pointing</a> <a href='#SkRegion_Iterator'>to</a> <a href='#SkRegion_Iterator'>final</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>in</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>.

### Return Value

true if <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>parsing</a> <a href='undocumented#Data'>is</a> <a href='undocumented#Data'>complete</a>

### Example

<div><fiddle-embed name="814efa7d7f4ae52dfc861a937c1b5c25">

#### Example Output

~~~~
done=true
done=false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_Iterator_next'>next</a> <a href='#SkRegion_Iterator_rect'>rect</a>

<a name='SkRegion_Iterator_next'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRegion_Iterator_next'>next()</a>
</pre>

Advances <a href='SkRegion_Reference#SkRegion'>SkRegion</a>::<a href='#SkRegion_Iterator'>Iterator</a> <a href='#SkRegion_Iterator'>to</a> <a href='#SkRegion_Iterator'>next</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>in</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>if</a> <a href='SkRegion_Reference#SkRegion'>it</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>not</a> <a href='SkRegion_Reference#SkRegion'>done</a>.

### Example

<div><fiddle-embed name="771236c2eadfc2fcd02a3e61a0875d39">

#### Example Output

~~~~
rect={1,2,3,4}
rect={5,6,7,8}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_Iterator_done'>done</a> <a href='#SkRegion_Iterator_rect'>rect</a>

<a name='SkRegion_Iterator_rect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='#SkRegion_Iterator_rect'>rect()</a> <a href='#SkRegion_Iterator_rect'>const</a>
</pre>

Returns <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>element</a> <a href='SkIRect_Reference#SkIRect'>in</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>. <a href='SkRegion_Reference#SkRegion'>Does</a> <a href='SkRegion_Reference#SkRegion'>not</a> <a href='SkRegion_Reference#SkRegion'>return</a> <a href='SkRegion_Reference#SkRegion'>predictable</a> <a href='SkRegion_Reference#SkRegion'>results</a> <a href='SkRegion_Reference#SkRegion'>if</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>
is empty.

### Return Value

part of <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>as</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>

### Example

<div><fiddle-embed name="0e7c58ab5d3bcfb36b1f8464cf6c7d89">

#### Example Output

~~~~
#Volatile
rect={0,0,0,0}
rect={1,2,3,4}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_Iterator_next'>next</a> <a href='#SkRegion_Iterator_done'>done</a>

<a name='SkRegion_Iterator_rgn'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkRegion_Reference#SkRegion'>SkRegion</a>* <a href='#SkRegion_Iterator_rgn'>rgn()</a> <a href='#SkRegion_Iterator_rgn'>const</a>
</pre>

Returns <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>if</a> <a href='SkRegion_Reference#SkRegion'>set</a>; <a href='SkRegion_Reference#SkRegion'>otherwise</a>, <a href='SkRegion_Reference#SkRegion'>returns</a> <a href='SkRegion_Reference#SkRegion'>nullptr</a>.

### Return Value

iterated <a href='SkRegion_Reference#SkRegion'>SkRegion</a>

### Example

<div><fiddle-embed name="bbc3c454a21186e2a16e843a5b061c44"></fiddle-embed></div>

### See Also

<a href='#SkRegion_Iterator'>Iterator</a> <a href='#SkRegion_Iterator_reset'>reset</a>

<a name='SkRegion_Cliperator'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    class SK_API <a href='#SkRegion_Cliperator'>Cliperator</a> {
    <a href='#SkRegion_Cliperator'>public</a>:
        <a href='#SkRegion_Cliperator'>Cliperator</a>(<a href='#SkRegion_Cliperator'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#Region'>region</a>, <a href='SkRegion_Reference#Region'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>clip</a>);
        <a href='SkIRect_Reference#SkIRect'>bool</a> <a href='#SkRegion_Cliperator_done'>done()</a>;
        <a href='#SkRegion_Cliperator_done'>void</a> <a href='#SkRegion_Cliperator_next'>next()</a>;
        <a href='#SkRegion_Cliperator_next'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='#SkRegion_Cliperator_rect'>rect()</a> <a href='#SkRegion_Cliperator_rect'>const</a>;
    };
</pre>

Returns the sequence of rectangles, sorted along y-axis, then x-axis, that make
up <a href='SkRegion_Reference#Region'>Region</a> <a href='SkRegion_Reference#Region'>intersected</a> <a href='SkRegion_Reference#Region'>with</a> <a href='SkRegion_Reference#Region'>the</a> <a href='SkRegion_Reference#Region'>specified</a> <a href='SkRegion_Reference#Region'>clip</a> <a href='SkRegion_Reference#Region'>rectangle</a>.

<a name='SkRegion_Cliperator_const_SkRegion_const_SkIRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkRegion_Cliperator'>Cliperator</a>(<a href='#SkRegion_Cliperator'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#Region'>region</a>, <a href='SkRegion_Reference#Region'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>clip</a>)
</pre>

Sets <a href='SkRegion_Reference#SkRegion'>SkRegion</a>::<a href='#SkRegion_Cliperator'>Cliperator</a> <a href='#SkRegion_Cliperator'>to</a> <a href='#SkRegion_Cliperator'>return</a> <a href='#SkRegion_Cliperator'>elements</a> <a href='#SkRegion_Cliperator'>of</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>array</a> <a href='SkIRect_Reference#SkIRect'>in</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>within</a> <a href='#SkRegion_Cliperator_const_SkRegion_const_SkIRect_clip'>clip</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_Cliperator_const_SkRegion_const_SkIRect_region'><code><strong>region</strong></code></a></td>
    <td><a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>to</a> <a href='SkRegion_Reference#SkRegion'>iterate</a></td>
  </tr>
  <tr>    <td><a name='SkRegion_Cliperator_const_SkRegion_const_SkIRect_clip'><code><strong>clip</strong></code></a></td>
    <td>bounds of iteration</td>
  </tr>
</table>

### Return Value

<a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>iterator</a>

### Example

<div><fiddle-embed name="3831fb6006a7e0ad5d140c266c22be78">

#### Example Output

~~~~
rect={1,2,2,3}
~~~~

</fiddle-embed></div>

### See Also

<a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='#SkRegion_Iterator'>Iterator</a> <a href='#SkRegion_Spanerator'>Spanerator</a>

<a name='SkRegion_Cliperator_done'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRegion_Cliperator_done'>done()</a>
</pre>

Returns true if <a href='SkRegion_Reference#SkRegion'>SkRegion</a>::<a href='#SkRegion_Cliperator'>Cliperator</a> <a href='#SkRegion_Cliperator'>is</a> <a href='#SkRegion_Cliperator'>pointing</a> <a href='#SkRegion_Cliperator'>to</a> <a href='#SkRegion_Cliperator'>final</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>in</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>.

### Return Value

true if <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>parsing</a> <a href='undocumented#Data'>is</a> <a href='undocumented#Data'>complete</a>

### Example

<div><fiddle-embed name="6cca7b96836266800d852664a1366453">

#### Example Output

~~~~
empty region done=true
after add rect done=false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_Cliperator_next'>next</a> <a href='#SkRegion_Cliperator_rect'>rect</a>

<a name='SkRegion_Cliperator_next'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void  <a href='#SkRegion_Cliperator_next'>next()</a>
</pre>

Advances iterator to next <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>in</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>contained</a> <a href='SkRegion_Reference#SkRegion'>by</a> <a href='SkRegion_Reference#SkRegion'>clip</a>.

### Example

<div><fiddle-embed name="3bbcc7eec19c808a8167bbcc987199f8">

#### Example Output

~~~~
rect={1,3,3,4}
rect={5,6,7,7}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_Cliperator_done'>done</a>

<a name='SkRegion_Cliperator_rect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='#SkRegion_Cliperator_rect'>rect()</a> <a href='#SkRegion_Cliperator_rect'>const</a>
</pre>

Returns <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>element</a> <a href='SkIRect_Reference#SkIRect'>in</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>, <a href='SkRegion_Reference#SkRegion'>intersected</a> <a href='SkRegion_Reference#SkRegion'>with</a> <a href='SkRegion_Reference#SkRegion'>clip</a> <a href='SkRegion_Reference#SkRegion'>passed</a> <a href='SkRegion_Reference#SkRegion'>to</a>
<a href='SkRegion_Reference#SkRegion'>SkRegion</a>::<a href='#SkRegion_Cliperator'>Cliperator</a> <a href='#SkRegion_Cliperator'>constructor</a>. <a href='#SkRegion_Cliperator'>Does</a> <a href='#SkRegion_Cliperator'>not</a> <a href='#SkRegion_Cliperator'>return</a> <a href='#SkRegion_Cliperator'>predictable</a> <a href='#SkRegion_Cliperator'>results</a> <a href='#SkRegion_Cliperator'>if</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>
is empty.

### Return Value

part of <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>inside</a> <a href='SkRegion_Reference#SkRegion'>clip</a> <a href='SkRegion_Reference#SkRegion'>as</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>

### Example

<div><fiddle-embed name="05791751f00b4c2426093fa143b43bc7">

#### Example Output

~~~~
#Volatile
empty region rect={1094713344,1065353216,0,-1}
after set rect rect={1,2,3,3}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_Cliperator_next'>next</a> <a href='#SkRegion_Cliperator_done'>done</a>

<a name='SkRegion_Spanerator'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    class <a href='#SkRegion_Spanerator'>Spanerator</a> {
    <a href='#SkRegion_Spanerator'>public</a>:
        <a href='#SkRegion_Spanerator'>Spanerator</a>(<a href='#SkRegion_Spanerator'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#Region'>region</a>, <a href='SkRegion_Reference#Region'>int</a> <a href='SkRegion_Reference#Region'>y</a>, <a href='SkRegion_Reference#Region'>int</a> <a href='SkRegion_Reference#Region'>left</a>, <a href='SkRegion_Reference#Region'>int</a> <a href='SkRegion_Reference#Region'>right</a>);
        <a href='SkRegion_Reference#Region'>bool</a> <a href='SkRegion_Reference#Region'>next</a>(<a href='SkRegion_Reference#Region'>int</a>* <a href='SkRegion_Reference#Region'>left</a>, <a href='SkRegion_Reference#Region'>int</a>* <a href='SkRegion_Reference#Region'>right</a>);
    };
</pre>

Returns the <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>segment</a> <a href='undocumented#Line'>ends</a> <a href='undocumented#Line'>within</a> <a href='SkRegion_Reference#Region'>Region</a> <a href='SkRegion_Reference#Region'>that</a> <a href='SkRegion_Reference#Region'>intersect</a> <a href='SkRegion_Reference#Region'>a</a> <a href='SkRegion_Reference#Region'>horizontal</a> <a href='undocumented#Line'>line</a>.

<a name='SkRegion_Spanerator_const_SkRegion_int_int_int'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkRegion_Spanerator'>Spanerator</a>(<a href='#SkRegion_Spanerator'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#Region'>region</a>, <a href='SkRegion_Reference#Region'>int</a> <a href='SkRegion_Reference#Region'>y</a>, <a href='SkRegion_Reference#Region'>int</a> <a href='SkRegion_Reference#Region'>left</a>, <a href='SkRegion_Reference#Region'>int</a> <a href='SkRegion_Reference#Region'>right</a>)
</pre>

Sets <a href='SkRegion_Reference#SkRegion'>SkRegion</a>::<a href='#SkRegion_Spanerator'>Spanerator</a> <a href='#SkRegion_Spanerator'>to</a> <a href='#SkRegion_Spanerator'>return</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>segments</a> <a href='undocumented#Line'>in</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>on</a> <a href='SkRegion_Reference#SkRegion'>scan</a> <a href='undocumented#Line'>line</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_Spanerator_const_SkRegion_int_int_int_region'><code><strong>region</strong></code></a></td>
    <td><a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>to</a> <a href='SkRegion_Reference#SkRegion'>iterate</a></td>
  </tr>
  <tr>    <td><a name='SkRegion_Spanerator_const_SkRegion_int_int_int_y'><code><strong>y</strong></code></a></td>
    <td>horizontal <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>to</a> <a href='undocumented#Line'>intersect</a></td>
  </tr>
  <tr>    <td><a name='SkRegion_Spanerator_const_SkRegion_int_int_int_left'><code><strong>left</strong></code></a></td>
    <td>bounds of iteration</td>
  </tr>
  <tr>    <td><a name='SkRegion_Spanerator_const_SkRegion_int_int_int_right'><code><strong>right</strong></code></a></td>
    <td>bounds of iteration</td>
  </tr>
</table>

### Return Value

<a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>iterator</a>

### Example

<div><fiddle-embed name="3073b3f8ea7252871b6156ff674dc385"></fiddle-embed></div>

### See Also

<a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='#SkRegion_Iterator'>Iterator</a> <a href='#SkRegion_Cliperator'>Cliperator</a>

<a name='SkRegion_Spanerator_next'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool next(int* left, int* right)
</pre>

Advances iterator to next span intersecting <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>within</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>segment</a> <a href='undocumented#Line'>provided</a>
in constructor. Returns true if interval was found.

### Parameters

<table>  <tr>    <td><a name='SkRegion_Spanerator_next_left'><code><strong>left</strong></code></a></td>
    <td>pointer to span start; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkRegion_Spanerator_next_right'><code><strong>right</strong></code></a></td>
    <td>pointer to span end; may be nullptr</td>
  </tr>
</table>

### Return Value

true if interval was found

### Example

<div><fiddle-embed name="03d02180fee5f64ec4a3347e118fb2ec">

#### Example Output

~~~~
empty region: result=false
after set rect: result=true left=2 right=3
~~~~

</fiddle-embed></div>

### See Also

done

<a name='SkRegion_empty_constructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkRegion_empty_constructor'>SkRegion()</a>
</pre>

Constructs an empty <a href='SkRegion_Reference#SkRegion'>SkRegion</a>. <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>set</a> <a href='SkRegion_Reference#SkRegion'>to</a> <a href='SkRegion_Reference#SkRegion'>empty</a> <a href='SkRegion_Reference#SkRegion'>bounds</a>
at (0, 0) with zero width and height.

### Return Value

empty <a href='SkRegion_Reference#SkRegion'>SkRegion</a>

### Example

<div><fiddle-embed name="4549dcda3e0f9a41b3daee0ed37deca8">

#### Example Output

~~~~
region bounds: {0, 0, 0, 0}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_setEmpty'>setEmpty</a>

<a name='SkRegion_copy_const_SkRegion'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkRegion_Reference#SkRegion'>SkRegion</a>(<a href='SkRegion_Reference#SkRegion'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#Region'>region</a>)
</pre>

Constructs a copy of an existing <a href='#SkRegion_copy_const_SkRegion_region'>region</a>.
Copy constructor makes two <a href='SkRegion_Reference#Region'>regions</a> <a href='SkRegion_Reference#Region'>identical</a> <a href='SkRegion_Reference#Region'>by</a> <a href='SkRegion_Reference#Region'>value</a>. <a href='SkRegion_Reference#Region'>Internally</a>, <a href='#SkRegion_copy_const_SkRegion_region'>region</a> <a href='#SkRegion_copy_const_SkRegion_region'>and</a>
the returned result share pointer values. The underlying <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>array</a> <a href='SkRect_Reference#SkRect'>is</a>
copied when modified.

Creating a <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>copy</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>very</a> <a href='SkRegion_Reference#SkRegion'>efficient</a> <a href='SkRegion_Reference#SkRegion'>and</a> <a href='SkRegion_Reference#SkRegion'>never</a> <a href='SkRegion_Reference#SkRegion'>allocates</a> <a href='SkRegion_Reference#SkRegion'>memory</a>.
<a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>are</a> <a href='SkRegion_Reference#SkRegion'>always</a> <a href='SkRegion_Reference#SkRegion'>copied</a> <a href='SkRegion_Reference#SkRegion'>by</a> <a href='SkRegion_Reference#SkRegion'>value</a> <a href='SkRegion_Reference#SkRegion'>from</a> <a href='SkRegion_Reference#SkRegion'>the</a> <a href='SkRegion_Reference#SkRegion'>interface</a>; <a href='SkRegion_Reference#SkRegion'>the</a> <a href='SkRegion_Reference#SkRegion'>underlying</a> <a href='SkRegion_Reference#SkRegion'>shared</a>
pointers are not exposed.

### Parameters

<table>  <tr>    <td><a name='SkRegion_copy_const_SkRegion_region'><code><strong>region</strong></code></a></td>
    <td><a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>to</a> <a href='SkRegion_Reference#SkRegion'>copy</a> <a href='SkRegion_Reference#SkRegion'>by</a> <a href='SkRegion_Reference#SkRegion'>value</a></td>
  </tr>
</table>

### Return Value

copy of <a href='SkRegion_Reference#SkRegion'>SkRegion</a>

### Example

<div><fiddle-embed name="3daa83fca809b9ec6560d2ef9e2da5e6">

#### Example Output

~~~~
region bounds: {1,2,3,4}
region2 bounds: {1,2,3,4}
after region set empty:
region bounds: {0,0,0,0}
region2 bounds: {1,2,3,4}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_setRegion'>setRegion</a> setRegion<a href='#SkRegion_copy_operator'>operator=(const SkRegion& region)</a>

<a name='SkRegion_copy_const_SkIRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
explicit <a href='SkRegion_Reference#SkRegion'>SkRegion</a>(<a href='SkRegion_Reference#SkRegion'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkRect_Reference#Rect'>rect</a>)
</pre>

Constructs a rectangular <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>matching</a> <a href='SkRegion_Reference#SkRegion'>the</a> <a href='SkRegion_Reference#SkRegion'>bounds</a> <a href='SkRegion_Reference#SkRegion'>of</a> <a href='#SkRegion_copy_const_SkIRect_rect'>rect</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_copy_const_SkIRect_rect'><code><strong>rect</strong></code></a></td>
    <td>bounds of constructed <a href='SkRegion_Reference#SkRegion'>SkRegion</a></td>
  </tr>
</table>

### Return Value

rectangular <a href='SkRegion_Reference#SkRegion'>SkRegion</a>

### Example

<div><fiddle-embed name="5253910233f7961c30b4c18ab911e917"></fiddle-embed></div>

### See Also

<a href='#SkRegion_setRect'>setRect</a> <a href='#SkRegion_setRegion'>setRegion</a>

<a name='SkRegion_destructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
~<a href='#SkRegion_empty_constructor'>SkRegion()</a>
</pre>

Releases ownership of any shared <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>and</a> <a href='undocumented#Data'>deletes</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>if</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>sole</a> <a href='SkRegion_Reference#SkRegion'>owner</a>.

### Example

<div><fiddle-embed name="985ff654a6b67288d322c748132a088e"><div>delete calls <a href='SkRegion_Reference#Region'>Region</a> <a href='SkRegion_Reference#Region'>destructor</a>, <a href='SkRegion_Reference#Region'>but</a> <a href='SkRegion_Reference#Region'>copy</a> <a href='SkRegion_Reference#Region'>of</a> <a href='SkRegion_Reference#Region'>original</a> <a href='SkRegion_Reference#Region'>in</a> <a href='SkRegion_Reference#Region'>region2</a> <a href='SkRegion_Reference#Region'>is</a> <a href='SkRegion_Reference#Region'>unaffected</a>.
</div>

#### Example Output

~~~~
region2 bounds: {1,2,3,4}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_empty_constructor'>SkRegion()</a> SkRegion()<a href='#SkRegion_copy_const_SkRegion'>SkRegion(const SkRegion& region)</a> SkRegion(const SkRegion& region)<a href='#SkRegion_copy_const_SkIRect'>SkRegion(const SkIRect& rect)</a> SkRegion(const SkIRect& rect)<a href='#SkRegion_copy_operator'>operator=(const SkRegion& region)</a>

<a name='SkRegion_copy_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>operator</a>=(<a href='SkRegion_Reference#SkRegion'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#Region'>region</a>)
</pre>

Constructs a copy of an existing <a href='#SkRegion_copy_operator_region'>region</a>.
Makes two <a href='SkRegion_Reference#Region'>regions</a> <a href='SkRegion_Reference#Region'>identical</a> <a href='SkRegion_Reference#Region'>by</a> <a href='SkRegion_Reference#Region'>value</a>. <a href='SkRegion_Reference#Region'>Internally</a>, <a href='#SkRegion_copy_operator_region'>region</a> <a href='#SkRegion_copy_operator_region'>and</a>
the returned result share pointer values. The underlying <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>array</a> <a href='SkRect_Reference#SkRect'>is</a>
copied when modified.

Creating a <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>copy</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>very</a> <a href='SkRegion_Reference#SkRegion'>efficient</a> <a href='SkRegion_Reference#SkRegion'>and</a> <a href='SkRegion_Reference#SkRegion'>never</a> <a href='SkRegion_Reference#SkRegion'>allocates</a> <a href='SkRegion_Reference#SkRegion'>memory</a>.
<a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>are</a> <a href='SkRegion_Reference#SkRegion'>always</a> <a href='SkRegion_Reference#SkRegion'>copied</a> <a href='SkRegion_Reference#SkRegion'>by</a> <a href='SkRegion_Reference#SkRegion'>value</a> <a href='SkRegion_Reference#SkRegion'>from</a> <a href='SkRegion_Reference#SkRegion'>the</a> <a href='SkRegion_Reference#SkRegion'>interface</a>; <a href='SkRegion_Reference#SkRegion'>the</a> <a href='SkRegion_Reference#SkRegion'>underlying</a> <a href='SkRegion_Reference#SkRegion'>shared</a>
pointers are not exposed.

### Parameters

<table>  <tr>    <td><a name='SkRegion_copy_operator_region'><code><strong>region</strong></code></a></td>
    <td><a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>to</a> <a href='SkRegion_Reference#SkRegion'>copy</a> <a href='SkRegion_Reference#SkRegion'>by</a> <a href='SkRegion_Reference#SkRegion'>value</a></td>
  </tr>
</table>

### Return Value

<a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>to</a> <a href='SkRegion_Reference#SkRegion'>copy</a> <a href='SkRegion_Reference#SkRegion'>by</a> <a href='SkRegion_Reference#SkRegion'>value</a>

### Example

<div><fiddle-embed name="e8513f6394c24efaa301d41921c5241a">

#### Example Output

~~~~
region1 bounds: {1,2,3,4}
region2 bounds: {1,2,3,4}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_set'>set</a> <a href='#SkRegion_swap'>swap</a> swap<a href='#SkRegion_copy_const_SkRegion'>SkRegion(const SkRegion& region)</a>

<a name='SkRegion_equal1_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator==(const <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>other</a>) <a href='SkRegion_Reference#SkRegion'>const</a>
</pre>

Compares <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>and</a> <a href='#SkRegion_equal1_operator_other'>other</a>; <a href='#SkRegion_equal1_operator_other'>returns</a> <a href='#SkRegion_equal1_operator_other'>true</a> <a href='#SkRegion_equal1_operator_other'>if</a> <a href='#SkRegion_equal1_operator_other'>they</a> <a href='#SkRegion_equal1_operator_other'>enclose</a> <a href='#SkRegion_equal1_operator_other'>exactly</a>
the same area.

### Parameters

<table>  <tr>    <td><a name='SkRegion_equal1_operator_other'><code><strong>other</strong></code></a></td>
    <td><a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>to</a> <a href='SkRegion_Reference#SkRegion'>compare</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>pair</a> <a href='SkRegion_Reference#SkRegion'>are</a> <a href='SkRegion_Reference#SkRegion'>equivalent</a>

### Example

<div><fiddle-embed name="d7f4fdc8bc63ca8410ed166ecef0aef3">

#### Example Output

~~~~
empty one == two
set rect one != two
set empty one == two
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_notequal1_operator'>operator!=(const SkRegion& other)_const</a> operator!=(const SkRegion& other)_const<a href='#SkRegion_copy_operator'>operator=(const SkRegion& region)</a>

<a name='SkRegion_notequal1_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator!=(const <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>other</a>) <a href='SkRegion_Reference#SkRegion'>const</a>
</pre>

Compares <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>and</a> <a href='#SkRegion_notequal1_operator_other'>other</a>; <a href='#SkRegion_notequal1_operator_other'>returns</a> <a href='#SkRegion_notequal1_operator_other'>true</a> <a href='#SkRegion_notequal1_operator_other'>if</a> <a href='#SkRegion_notequal1_operator_other'>they</a> <a href='#SkRegion_notequal1_operator_other'>do</a> <a href='#SkRegion_notequal1_operator_other'>not</a> <a href='#SkRegion_notequal1_operator_other'>enclose</a> <a href='#SkRegion_notequal1_operator_other'>the</a> <a href='#SkRegion_notequal1_operator_other'>same</a> <a href='#SkRegion_notequal1_operator_other'>area</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_notequal1_operator_other'><code><strong>other</strong></code></a></td>
    <td><a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>to</a> <a href='SkRegion_Reference#SkRegion'>compare</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>pair</a> <a href='SkRegion_Reference#SkRegion'>are</a> <a href='SkRegion_Reference#SkRegion'>not</a> <a href='SkRegion_Reference#SkRegion'>equivalent</a>

### Example

<div><fiddle-embed name="3357caa9d8d810f200cbccb668182496">

#### Example Output

~~~~
empty one == two
set rect one != two
union rect one == two
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_equal1_operator'>operator==(const SkRegion& other)_const</a> operator==(const SkRegion& other)_const<a href='#SkRegion_copy_operator'>operator=(const SkRegion& region)</a>

<a name='SkRegion_set'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool set(const <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>src</a>)
</pre>

Sets <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>to</a> <a href='#SkRegion_set_src'>src</a>, <a href='#SkRegion_set_src'>and</a> <a href='#SkRegion_set_src'>returns</a> <a href='#SkRegion_set_src'>true</a> <a href='#SkRegion_set_src'>if</a> <a href='#SkRegion_set_src'>src</a> <a href='#SkRegion_set_src'>bounds</a> <a href='#SkRegion_set_src'>is</a> <a href='#SkRegion_set_src'>not</a> <a href='#SkRegion_set_src'>empty</a>.
This makes <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>and</a> <a href='#SkRegion_set_src'>src</a> <a href='#SkRegion_set_src'>identical</a> <a href='#SkRegion_set_src'>by</a> <a href='#SkRegion_set_src'>value</a>. <a href='#SkRegion_set_src'>Internally</a>,
<a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>and</a> <a href='#SkRegion_set_src'>src</a> <a href='#SkRegion_set_src'>share</a> <a href='#SkRegion_set_src'>pointer</a> <a href='#SkRegion_set_src'>values</a>. <a href='#SkRegion_set_src'>The</a> <a href='#SkRegion_set_src'>underlying</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>array</a> <a href='SkRect_Reference#SkRect'>is</a>
copied when modified.

Creating a <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>copy</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>very</a> <a href='SkRegion_Reference#SkRegion'>efficient</a> <a href='SkRegion_Reference#SkRegion'>and</a> <a href='SkRegion_Reference#SkRegion'>never</a> <a href='SkRegion_Reference#SkRegion'>allocates</a> <a href='SkRegion_Reference#SkRegion'>memory</a>.
<a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>are</a> <a href='SkRegion_Reference#SkRegion'>always</a> <a href='SkRegion_Reference#SkRegion'>copied</a> <a href='SkRegion_Reference#SkRegion'>by</a> <a href='SkRegion_Reference#SkRegion'>value</a> <a href='SkRegion_Reference#SkRegion'>from</a> <a href='SkRegion_Reference#SkRegion'>the</a> <a href='SkRegion_Reference#SkRegion'>interface</a>; <a href='SkRegion_Reference#SkRegion'>the</a> <a href='SkRegion_Reference#SkRegion'>underlying</a> <a href='SkRegion_Reference#SkRegion'>shared</a>
pointers are not exposed.

### Parameters

<table>  <tr>    <td><a name='SkRegion_set_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>to</a> <a href='SkRegion_Reference#SkRegion'>copy</a></td>
  </tr>
</table>

### Return Value

copy of <a href='#SkRegion_set_src'>src</a>

### Example

<div><fiddle-embed name="b3538117c7ae2cb7de3b42ca45fe1b13">

#### Example Output

~~~~
region1 bounds: {1,2,3,4}
region2 bounds: {1,2,3,4}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_copy_operator'>operator=(const SkRegion& region)</a> <a href='#SkRegion_swap'>swap</a> swap<a href='#SkRegion_copy_const_SkRegion'>SkRegion(const SkRegion& region)</a>

<a name='SkRegion_swap'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkRegion_swap'>swap</a>(<a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>other</a>)
</pre>

Exchanges <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>array</a> <a href='SkIRect_Reference#SkIRect'>of</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>and</a> <a href='#SkRegion_swap_other'>other</a>. <a href='#SkRegion_swap'>swap()</a> <a href='#SkRegion_swap'>internally</a> <a href='#SkRegion_swap'>exchanges</a> <a href='#SkRegion_swap'>pointers</a>,
so it is lightweight and does not allocate memory.

<a href='#SkRegion_swap'>swap()</a> <a href='#SkRegion_swap'>usage</a> <a href='#SkRegion_swap'>has</a> <a href='#SkRegion_swap'>largely</a> <a href='#SkRegion_swap'>been</a> <a href='#SkRegion_swap'>replaced</a> <a href='#SkRegion_swap'>by</a> by<a href='#SkRegion_copy_operator'>operator=(const SkRegion& region)</a>.
<a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>do</a> <a href='SkPath_Reference#SkPath'>not</a> <a href='SkPath_Reference#SkPath'>copy</a> <a href='SkPath_Reference#SkPath'>their</a> <a href='SkPath_Reference#SkPath'>content</a> <a href='SkPath_Reference#SkPath'>on</a> <a href='SkPath_Reference#SkPath'>assignment</a> <a href='SkPath_Reference#SkPath'>until</a> <a href='SkPath_Reference#SkPath'>they</a> <a href='SkPath_Reference#SkPath'>are</a> <a href='SkPath_Reference#SkPath'>written</a> <a href='SkPath_Reference#SkPath'>to</a>,
making assignment as efficient as <a href='#SkRegion_swap'>swap()</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_swap_other'><code><strong>other</strong></code></a></td>
    <td><a href='#SkRegion_copy_operator'>operator=(const SkRegion& region)</a> <a href='#SkRegion_copy_operator'>set</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ae67b7b4c198b46c58e48f5af061c8f1">

#### Example Output

~~~~
region1 bounds: {0,0,0,0}
region2 bounds: {1,2,3,4}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_copy_operator'>operator=(const SkRegion& region)</a> <a href='#SkRegion_set'>set</a> set<a href='#SkRegion_copy_const_SkRegion'>SkRegion(const SkRegion& region)</a>

<a name='SkRegion_isEmpty'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRegion_isEmpty'>isEmpty</a>() <a href='#SkRegion_isEmpty'>const</a>
</pre>

Returns true if <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>empty</a>.
Empty <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>has</a> <a href='SkRegion_Reference#SkRegion'>bounds</a> <a href='SkRegion_Reference#SkRegion'>width</a> <a href='SkRegion_Reference#SkRegion'>or</a> <a href='SkRegion_Reference#SkRegion'>height</a> <a href='SkRegion_Reference#SkRegion'>less</a> <a href='SkRegion_Reference#SkRegion'>than</a> <a href='SkRegion_Reference#SkRegion'>or</a> <a href='SkRegion_Reference#SkRegion'>equal</a> <a href='SkRegion_Reference#SkRegion'>to</a> <a href='SkRegion_Reference#SkRegion'>zero</a>.
<a href='#SkRegion_empty_constructor'>SkRegion()</a> <a href='SkRegion_Reference#SkRegion'>constructs</a> <a href='SkRegion_Reference#SkRegion'>empty</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>; <a href='#SkRegion_setEmpty'>setEmpty</a>()
and <a href='#SkRegion_setRect'>setRect</a>() <a href='#SkRegion_setRect'>with</a> <a href='#SkRegion_setRect'>dimensionless</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>make</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>empty</a>.

### Return Value

true if bounds has no width or height

### Example

<div><fiddle-embed name="10ef0de39e8553dd97cf8668ce185070">

#### Example Output

~~~~
initial: region is empty
set rect: region is not empty
set empty: region is empty
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_isRect'>isRect</a> <a href='#SkRegion_isComplex'>isComplex</a> isComplex<a href='#SkRegion_equal1_operator'>operator==(const SkRegion& other)_const</a>

<a name='SkRegion_isRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRegion_isRect'>isRect</a>() <a href='#SkRegion_isRect'>const</a>
</pre>

Returns true if <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>one</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>with</a> <a href='SkIRect_Reference#SkIRect'>positive</a> <a href='SkIRect_Reference#SkIRect'>dimensions</a>.

### Return Value

true if <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>contains</a> <a href='SkRegion_Reference#SkRegion'>one</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>

### Example

<div><fiddle-embed name="b6adbdddf7fe45a1098121c4e5fd57ea">

#### Example Output

~~~~
initial: region is not rect
set rect: region is rect
set empty: region is not rect
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_isEmpty'>isEmpty</a> <a href='#SkRegion_isComplex'>isComplex</a>

<a name='SkRegion_isComplex'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRegion_isComplex'>isComplex</a>() <a href='#SkRegion_isComplex'>const</a>
</pre>

Returns true if <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>described</a> <a href='SkRegion_Reference#SkRegion'>by</a> <a href='SkRegion_Reference#SkRegion'>more</a> <a href='SkRegion_Reference#SkRegion'>than</a> <a href='SkRegion_Reference#SkRegion'>one</a> <a href='SkRegion_Reference#SkRegion'>rectangle</a>.

### Return Value

true if <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>contains</a> <a href='SkRegion_Reference#SkRegion'>more</a> <a href='SkRegion_Reference#SkRegion'>than</a> <a href='SkRegion_Reference#SkRegion'>one</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>

### Example

<div><fiddle-embed name="1fbd76d75ca2d280e81856311de4e54e">

#### Example Output

~~~~
initial: region is not complex
set rect: region is not complex
op rect: region is complex
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_isEmpty'>isEmpty</a> <a href='#SkRegion_isRect'>isRect</a>

<a name='SkRegion_getBounds'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='#SkRegion_getBounds'>getBounds</a>() <a href='#SkRegion_getBounds'>const</a>
</pre>

Returns minimum and maximum axes values of <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>array</a>.
Returns (0, 0, 0, 0) if <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>empty</a>.

### Return Value

combined bounds of all <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>elements</a>

### Example

<div><fiddle-embed name="651632582d385d2531e7aa551c31e331">

#### Example Output

~~~~
bounds: {1,2,4,5}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_isEmpty'>isEmpty</a> <a href='#SkRegion_isRect'>isRect</a>

<a name='SkRegion_computeRegionComplexity'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkRegion_computeRegionComplexity'>computeRegionComplexity</a>() <a href='#SkRegion_computeRegionComplexity'>const</a>
</pre>

Returns a value that increases with the number of
elements in <a href='SkRegion_Reference#SkRegion'>SkRegion</a>. <a href='SkRegion_Reference#SkRegion'>Returns</a> <a href='SkRegion_Reference#SkRegion'>zero</a> <a href='SkRegion_Reference#SkRegion'>if</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>empty</a>.
Returns one if <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>equals</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>; <a href='SkIRect_Reference#SkIRect'>otherwise</a>, <a href='SkIRect_Reference#SkIRect'>returns</a>
value greater than one indicating that <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>complex</a>.

Call to compare <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>for</a> <a href='SkRegion_Reference#SkRegion'>relative</a> <a href='SkRegion_Reference#SkRegion'>complexity</a>.

### Return Value

relative complexity

### Example

<div><fiddle-embed name="c4984fefdcecdd1090be160f80939d87">

#### Example Output

~~~~
initial: region complexity 0
set rect: region complexity 1
op rect: region complexity 3
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_isRect'>isRect</a> <a href='#SkRegion_isComplex'>isComplex</a>

<a name='SkRegion_getBoundaryPath'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRegion_getBoundaryPath'>getBoundaryPath</a>(<a href='SkPath_Reference#SkPath'>SkPath</a>* <a href='SkPath_Reference#Path'>path</a>) <a href='SkPath_Reference#Path'>const</a>
</pre>

Appends outline of <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>to</a> <a href='#SkRegion_getBoundaryPath_path'>path</a>.
Returns true if <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>not</a> <a href='SkRegion_Reference#SkRegion'>empty</a>; <a href='SkRegion_Reference#SkRegion'>otherwise</a>, <a href='SkRegion_Reference#SkRegion'>returns</a> <a href='SkRegion_Reference#SkRegion'>false</a>, <a href='SkRegion_Reference#SkRegion'>and</a> <a href='SkRegion_Reference#SkRegion'>leaves</a> <a href='#SkRegion_getBoundaryPath_path'>path</a>
unmodified.

### Parameters

<table>  <tr>    <td><a name='SkRegion_getBoundaryPath_path'><code><strong>path</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>append</a> <a href='SkPath_Reference#SkPath'>to</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkRegion_getBoundaryPath_path'>path</a> <a href='#SkRegion_getBoundaryPath_path'>changed</a>

### Example

<div><fiddle-embed name="6631d36406efa3b3e27960c876421a7f"></fiddle-embed></div>

### See Also

<a href='#SkRegion_isEmpty'>isEmpty</a> <a href='#SkRegion_isComplex'>isComplex</a>

<a name='SkRegion_setEmpty'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRegion_setEmpty'>setEmpty</a>()
</pre>

Constructs an empty <a href='SkRegion_Reference#SkRegion'>SkRegion</a>. <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>set</a> <a href='SkRegion_Reference#SkRegion'>to</a> <a href='SkRegion_Reference#SkRegion'>empty</a> <a href='SkRegion_Reference#SkRegion'>bounds</a>
at (0, 0) with zero width and height. Always returns false.

### Return Value

false

### Example

<div><fiddle-embed name="1314f7250963775c5ee89cc5981eee24">

#### Example Output

~~~~
region bounds: {1,2,3,4}
after region set empty:
region bounds: {0,0,0,0}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_empty_constructor'>SkRegion()</a>

<a name='SkRegion_setRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRegion_setRect'>setRect</a>(<a href='#SkRegion_setRect'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkRect_Reference#Rect'>rect</a>)
</pre>

Constructs a rectangular <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>matching</a> <a href='SkRegion_Reference#SkRegion'>the</a> <a href='SkRegion_Reference#SkRegion'>bounds</a> <a href='SkRegion_Reference#SkRegion'>of</a> <a href='#SkRegion_setRect_rect'>rect</a>.
If <a href='#SkRegion_setRect_rect'>rect</a> <a href='#SkRegion_setRect_rect'>is</a> <a href='#SkRegion_setRect_rect'>empty</a>, <a href='#SkRegion_setRect_rect'>constructs</a> <a href='#SkRegion_setRect_rect'>empty</a> <a href='#SkRegion_setRect_rect'>and</a> <a href='#SkRegion_setRect_rect'>returns</a> <a href='#SkRegion_setRect_rect'>false</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_setRect_rect'><code><strong>rect</strong></code></a></td>
    <td>bounds of constructed <a href='SkRegion_Reference#SkRegion'>SkRegion</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkRegion_setRect_rect'>rect</a> <a href='#SkRegion_setRect_rect'>is</a> <a href='#SkRegion_setRect_rect'>not</a> <a href='#SkRegion_setRect_rect'>empty</a>

### Example

<div><fiddle-embed name="e12575ffcd262f2364e0e6bece98a825">

#### Example Output

~~~~
region is not empty
region is empty
setEmpty: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_copy_const_SkIRect'>SkRegion(const SkIRect& rect)</a>

<a name='SkRegion_setRect_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRegion_setRect'>setRect</a>(<a href='#SkRegion_setRect'>int32_t</a> <a href='#SkRegion_setRect'>left</a>, <a href='#SkRegion_setRect'>int32_t</a> <a href='#SkRegion_setRect'>top</a>, <a href='#SkRegion_setRect'>int32_t</a> <a href='#SkRegion_setRect'>right</a>, <a href='#SkRegion_setRect'>int32_t</a> <a href='#SkRegion_setRect'>bottom</a>)
</pre>

Constructs <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>with</a> <a href='SkRegion_Reference#SkRegion'>bounds</a> (<a href='#SkRegion_setRect_2_left'>left</a>, <a href='#SkRegion_setRect_2_top'>top</a>, <a href='#SkRegion_setRect_2_right'>right</a>, <a href='#SkRegion_setRect_2_bottom'>bottom</a>).
Returns true if <a href='#SkRegion_setRect_2_left'>left</a> <a href='#SkRegion_setRect_2_left'>is</a> <a href='#SkRegion_setRect_2_left'>less</a> <a href='#SkRegion_setRect_2_left'>than</a> <a href='#SkRegion_setRect_2_right'>right</a> <a href='#SkRegion_setRect_2_right'>and</a> <a href='#SkRegion_setRect_2_top'>top</a> <a href='#SkRegion_setRect_2_top'>is</a> <a href='#SkRegion_setRect_2_top'>less</a> <a href='#SkRegion_setRect_2_top'>than</a> <a href='#SkRegion_setRect_2_bottom'>bottom</a>; <a href='#SkRegion_setRect_2_bottom'>otherwise</a>,
constructs empty <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>and</a> <a href='SkRegion_Reference#SkRegion'>returns</a> <a href='SkRegion_Reference#SkRegion'>false</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_setRect_2_left'><code><strong>left</strong></code></a></td>
    <td>edge of bounds on x-axis</td>
  </tr>
  <tr>    <td><a name='SkRegion_setRect_2_top'><code><strong>top</strong></code></a></td>
    <td>edge of bounds on y-axis</td>
  </tr>
  <tr>    <td><a name='SkRegion_setRect_2_right'><code><strong>right</strong></code></a></td>
    <td>edge of bounds on x-axis</td>
  </tr>
  <tr>    <td><a name='SkRegion_setRect_2_bottom'><code><strong>bottom</strong></code></a></td>
    <td>edge of bounds on y-axis</td>
  </tr>
</table>

### Return Value

rectangular <a href='SkRegion_Reference#SkRegion'>SkRegion</a>

### Example

<div><fiddle-embed name="5b31a1b077818a8150ad50f3b19e7bfe">

#### Example Output

~~~~
set to: 1,2,3,4: success:true {1,2,3,4}
set to: 3,2,1,4: success:false {0,0,0,0}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_copy_const_SkIRect'>SkRegion(const SkIRect& rect)</a>

<a name='SkRegion_setRects'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRegion_setRects'>setRects</a>(<a href='#SkRegion_setRects'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkRect_Reference#Rect'>rects</a>[], <a href='SkRect_Reference#Rect'>int</a> <a href='SkRect_Reference#Rect'>count</a>)
</pre>

Constructs <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>as</a> <a href='SkRegion_Reference#SkRegion'>the</a> <a href='SkRegion_Reference#SkRegion'>union</a> <a href='SkRegion_Reference#SkRegion'>of</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>in</a> <a href='#SkRegion_setRects_rects'>rects</a> <a href='#SkRegion_setRects_rects'>array</a>. <a href='#SkRegion_setRects_rects'>If</a> <a href='#SkRegion_setRects_count'>count</a> <a href='#SkRegion_setRects_count'>is</a>
zero, constructs empty <a href='SkRegion_Reference#SkRegion'>SkRegion</a>. <a href='SkRegion_Reference#SkRegion'>Returns</a> <a href='SkRegion_Reference#SkRegion'>false</a> <a href='SkRegion_Reference#SkRegion'>if</a> <a href='SkRegion_Reference#SkRegion'>constructed</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>empty</a>.

May be faster than repeated calls to <a href='#SkRegion_op'>op()</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_setRects_rects'><code><strong>rects</strong></code></a></td>
    <td>array of <a href='SkIRect_Reference#SkIRect'>SkIRect</a></td>
  </tr>
  <tr>    <td><a name='SkRegion_setRects_count'><code><strong>count</strong></code></a></td>
    <td>array <a href='undocumented#Size'>size</a></td>
  </tr>
</table>

### Return Value

true if constructed <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>not</a> <a href='SkRegion_Reference#SkRegion'>empty</a>

### Example

<div><fiddle-embed name="fc793a14ed76c096a68a755c963c1ee0"></fiddle-embed></div>

### See Also

<a href='#SkRegion_setRect'>setRect</a> <a href='#SkRegion_op'>op</a>

<a name='SkRegion_setRegion'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRegion_setRegion'>setRegion</a>(<a href='#SkRegion_setRegion'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#Region'>region</a>)
</pre>

Constructs a copy of an existing <a href='#SkRegion_setRegion_region'>region</a>.
Makes two <a href='SkRegion_Reference#Region'>regions</a> <a href='SkRegion_Reference#Region'>identical</a> <a href='SkRegion_Reference#Region'>by</a> <a href='SkRegion_Reference#Region'>value</a>. <a href='SkRegion_Reference#Region'>Internally</a>, <a href='#SkRegion_setRegion_region'>region</a> <a href='#SkRegion_setRegion_region'>and</a>
the returned result share pointer values. The underlying <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>array</a> <a href='SkRect_Reference#SkRect'>is</a>
copied when modified.

Creating a <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>copy</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>very</a> <a href='SkRegion_Reference#SkRegion'>efficient</a> <a href='SkRegion_Reference#SkRegion'>and</a> <a href='SkRegion_Reference#SkRegion'>never</a> <a href='SkRegion_Reference#SkRegion'>allocates</a> <a href='SkRegion_Reference#SkRegion'>memory</a>.
<a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>are</a> <a href='SkRegion_Reference#SkRegion'>always</a> <a href='SkRegion_Reference#SkRegion'>copied</a> <a href='SkRegion_Reference#SkRegion'>by</a> <a href='SkRegion_Reference#SkRegion'>value</a> <a href='SkRegion_Reference#SkRegion'>from</a> <a href='SkRegion_Reference#SkRegion'>the</a> <a href='SkRegion_Reference#SkRegion'>interface</a>; <a href='SkRegion_Reference#SkRegion'>the</a> <a href='SkRegion_Reference#SkRegion'>underlying</a> <a href='SkRegion_Reference#SkRegion'>shared</a>
pointers are not exposed.

### Parameters

<table>  <tr>    <td><a name='SkRegion_setRegion_region'><code><strong>region</strong></code></a></td>
    <td><a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>to</a> <a href='SkRegion_Reference#SkRegion'>copy</a> <a href='SkRegion_Reference#SkRegion'>by</a> <a href='SkRegion_Reference#SkRegion'>value</a></td>
  </tr>
</table>

### Return Value

<a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>to</a> <a href='SkRegion_Reference#SkRegion'>copy</a> <a href='SkRegion_Reference#SkRegion'>by</a> <a href='SkRegion_Reference#SkRegion'>value</a>

### Example

<div><fiddle-embed name="5d75d22bd155576838155762ab040751">

#### Example Output

~~~~
region bounds: {1,2,3,4}
region2 bounds: {1,2,3,4}
after region set empty:
region bounds: {1,2,3,4}
region2 bounds: {0,0,0,0}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_copy_const_SkRegion'>SkRegion(const SkRegion& region)</a>

<a name='SkRegion_setPath'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRegion_setPath'>setPath</a>(<a href='#SkRegion_setPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>, <a href='SkPath_Reference#Path'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>clip</a>)
</pre>

Constructs <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>to</a> <a href='SkRegion_Reference#SkRegion'>match</a> <a href='SkRegion_Reference#SkRegion'>outline</a> <a href='SkRegion_Reference#SkRegion'>of</a> <a href='#SkRegion_setPath_path'>path</a> <a href='#SkRegion_setPath_path'>within</a> <a href='#SkRegion_setPath_clip'>clip</a>.
Returns false if constructed <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>empty</a>.

Constructed <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>draws</a> <a href='SkRegion_Reference#SkRegion'>the</a> <a href='SkRegion_Reference#SkRegion'>same</a> <a href='SkRegion_Reference#SkRegion'>pixels</a> <a href='SkRegion_Reference#SkRegion'>as</a> <a href='#SkRegion_setPath_path'>path</a> <a href='#SkRegion_setPath_path'>through</a> <a href='#SkRegion_setPath_clip'>clip</a> <a href='#SkRegion_setPath_clip'>when</a>
<a href='SkPaint_Reference#Anti_Alias'>anti-aliasing</a> <a href='SkPaint_Reference#Anti_Alias'>is</a> <a href='SkPaint_Reference#Anti_Alias'>disabled</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_setPath_path'><code><strong>path</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>providing</a> <a href='SkPath_Reference#SkPath'>outline</a></td>
  </tr>
  <tr>    <td><a name='SkRegion_setPath_clip'><code><strong>clip</strong></code></a></td>
    <td><a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>containing</a> <a href='#SkRegion_setPath_path'>path</a></td>
  </tr>
</table>

### Return Value

true if constructed <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>not</a> <a href='SkRegion_Reference#SkRegion'>empty</a>

### Example

<div><fiddle-embed name="45b9ea2247b9ca7f10aa22ea29a426f4"></fiddle-embed></div>

### See Also

<a href='#SkRegion_setRects'>setRects</a> <a href='#SkRegion_op'>op</a>

<a name='SkRegion_intersects'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool intersects(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkRect_Reference#Rect'>rect</a>) <a href='SkRect_Reference#Rect'>const</a>
</pre>

Returns true if <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>intersects</a> <a href='#SkRegion_intersects_rect'>rect</a>.
Returns false if either <a href='#SkRegion_intersects_rect'>rect</a> <a href='#SkRegion_intersects_rect'>or</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>empty</a>, <a href='SkRegion_Reference#SkRegion'>or</a> <a href='SkRegion_Reference#SkRegion'>do</a> <a href='SkRegion_Reference#SkRegion'>not</a> <a href='SkRegion_Reference#SkRegion'>intersect</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_intersects_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>intersect</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkRegion_intersects_rect'>rect</a> <a href='#SkRegion_intersects_rect'>and</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>have</a> <a href='SkRegion_Reference#SkRegion'>area</a> <a href='SkRegion_Reference#SkRegion'>in</a> <a href='SkRegion_Reference#SkRegion'>common</a>

### Example

<div><fiddle-embed name="42bde0ef8c2ee372751428cd6e21c1ca"></fiddle-embed></div>

### See Also

<a href='#SkRegion_contains'>contains</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_intersects'>intersects</a>

<a name='SkRegion_intersects_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool intersects(const <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>other</a>) <a href='SkRegion_Reference#SkRegion'>const</a>
</pre>

Returns true if <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>intersects</a> <a href='#SkRegion_intersects_2_other'>other</a>.
Returns false if either <a href='#SkRegion_intersects_2_other'>other</a> <a href='#SkRegion_intersects_2_other'>or</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>empty</a>, <a href='SkRegion_Reference#SkRegion'>or</a> <a href='SkRegion_Reference#SkRegion'>do</a> <a href='SkRegion_Reference#SkRegion'>not</a> <a href='SkRegion_Reference#SkRegion'>intersect</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_intersects_2_other'><code><strong>other</strong></code></a></td>
    <td><a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>to</a> <a href='SkRegion_Reference#SkRegion'>intersect</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkRegion_intersects_2_other'>other</a> <a href='#SkRegion_intersects_2_other'>and</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>have</a> <a href='SkRegion_Reference#SkRegion'>area</a> <a href='SkRegion_Reference#SkRegion'>in</a> <a href='SkRegion_Reference#SkRegion'>common</a>

### Example

<div><fiddle-embed name="4263d79ac0e7df02e90948fdde9fa965"></fiddle-embed></div>

### See Also

<a href='#SkRegion_contains'>contains</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_intersects'>intersects</a>

<a name='SkRegion_contains'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool contains(int32_t x, int32_t y) const
</pre>

Returns true if <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> (<a href='#SkRegion_contains_x'>x</a>, <a href='#SkRegion_contains_y'>y</a>) <a href='#SkRegion_contains_y'>is</a> <a href='#SkRegion_contains_y'>inside</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>.
Returns false if <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>empty</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_contains_x'><code><strong>x</strong></code></a></td>
    <td>test <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='SkIPoint_Reference#SkIPoint'>x-coordinate</a></td>
  </tr>
  <tr>    <td><a name='SkRegion_contains_y'><code><strong>y</strong></code></a></td>
    <td>test <a href='SkIPoint_Reference#SkIPoint'>SkIPoint</a> <a href='SkIPoint_Reference#SkIPoint'>y-coordinate</a></td>
  </tr>
</table>

### Return Value

true if (<a href='#SkRegion_contains_x'>x</a>, <a href='#SkRegion_contains_y'>y</a>) <a href='#SkRegion_contains_y'>is</a> <a href='#SkRegion_contains_y'>inside</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>

### Example

<div><fiddle-embed name="e3899c2715c332bfc7648d5f2b9eefc6"></fiddle-embed></div>

### See Also

<a href='#SkRegion_intersects'>intersects</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_contains'>contains</a>

<a name='SkRegion_contains_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool contains(const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>other</a>) <a href='SkIRect_Reference#SkIRect'>const</a>
</pre>

Returns true if <a href='#SkRegion_contains_2_other'>other</a> <a href='#SkRegion_contains_2_other'>is</a> <a href='#SkRegion_contains_2_other'>completely</a> <a href='#SkRegion_contains_2_other'>inside</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>.
Returns false if <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>or</a> <a href='#SkRegion_contains_2_other'>other</a> <a href='#SkRegion_contains_2_other'>is</a> <a href='#SkRegion_contains_2_other'>empty</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_contains_2_other'><code><strong>other</strong></code></a></td>
    <td><a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>contain</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkRegion_contains_2_other'>other</a> <a href='#SkRegion_contains_2_other'>is</a> <a href='#SkRegion_contains_2_other'>inside</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>

### Example

<div><fiddle-embed name="100b4cbd5dd7406804e40035833a433c"></fiddle-embed></div>

### See Also

<a href='#SkRegion_intersects'>intersects</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_contains'>contains</a>

<a name='SkRegion_contains_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool contains(const <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>other</a>) <a href='SkRegion_Reference#SkRegion'>const</a>
</pre>

Returns true if <a href='#SkRegion_contains_3_other'>other</a> <a href='#SkRegion_contains_3_other'>is</a> <a href='#SkRegion_contains_3_other'>completely</a> <a href='#SkRegion_contains_3_other'>inside</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>.
Returns false if <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>or</a> <a href='#SkRegion_contains_3_other'>other</a> <a href='#SkRegion_contains_3_other'>is</a> <a href='#SkRegion_contains_3_other'>empty</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_contains_3_other'><code><strong>other</strong></code></a></td>
    <td><a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>to</a> <a href='SkRegion_Reference#SkRegion'>contain</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkRegion_contains_3_other'>other</a> <a href='#SkRegion_contains_3_other'>is</a> <a href='#SkRegion_contains_3_other'>inside</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>

### Example

<div><fiddle-embed name="46de22da2f3e08a8d7f064634fc1c7b5"></fiddle-embed></div>

### See Also

<a href='#SkRegion_intersects'>intersects</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_contains'>contains</a>

<a name='SkRegion_quickContains'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRegion_quickContains'>quickContains</a>(<a href='#SkRegion_quickContains'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkIRect_Reference#SkIRect'>r</a>) <a href='SkIRect_Reference#SkIRect'>const</a>
</pre>

Returns true if <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>a</a> <a href='SkRegion_Reference#SkRegion'>single</a> <a href='SkRegion_Reference#SkRegion'>rectangle</a> <a href='SkRegion_Reference#SkRegion'>and</a> <a href='SkRegion_Reference#SkRegion'>contains</a> <a href='#SkRegion_quickContains_r'>r</a>.
May return false even though <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>contains</a> <a href='#SkRegion_quickContains_r'>r</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_quickContains_r'><code><strong>r</strong></code></a></td>
    <td><a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>contain</a></td>
  </tr>
</table>

### Return Value

true quickly if <a href='#SkRegion_quickContains_r'>r</a> <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>are</a> <a href='SkPoint_Reference#Point'>equal</a> <a href='SkPoint_Reference#Point'>or</a> <a href='SkPoint_Reference#Point'>inside</a>

### Example

<div><fiddle-embed name="d8e5eac373e2e7cfc1b8cd0229647ba6">

#### Example Output

~~~~
quickContains 1: true
quickContains 2: true
quickContains 3: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_contains'>contains</a> <a href='#SkRegion_quickReject'>quickReject</a> <a href='#SkRegion_intersects'>intersects</a>

<a name='SkRegion_quickContains_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRegion_quickContains'>quickContains</a>(<a href='#SkRegion_quickContains'>int32_t</a> <a href='#SkRegion_quickContains'>left</a>, <a href='#SkRegion_quickContains'>int32_t</a> <a href='#SkRegion_quickContains'>top</a>, <a href='#SkRegion_quickContains'>int32_t</a> <a href='#SkRegion_quickContains'>right</a>, <a href='#SkRegion_quickContains'>int32_t</a> <a href='#SkRegion_quickContains'>bottom</a>) <a href='#SkRegion_quickContains'>const</a>
</pre>

Returns true if <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>a</a> <a href='SkRegion_Reference#SkRegion'>single</a> <a href='SkRegion_Reference#SkRegion'>rectangle</a> <a href='SkRegion_Reference#SkRegion'>and</a> <a href='SkRegion_Reference#SkRegion'>contains</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>
(<a href='#SkRegion_quickContains_2_left'>left</a>, <a href='#SkRegion_quickContains_2_top'>top</a>, <a href='#SkRegion_quickContains_2_right'>right</a>, <a href='#SkRegion_quickContains_2_bottom'>bottom</a>).
Returns false if <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>empty</a> <a href='SkRegion_Reference#SkRegion'>or</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> (<a href='#SkRegion_quickContains_2_left'>left</a>, <a href='#SkRegion_quickContains_2_top'>top</a>, <a href='#SkRegion_quickContains_2_right'>right</a>, <a href='#SkRegion_quickContains_2_bottom'>bottom</a>) <a href='#SkRegion_quickContains_2_bottom'>is</a> <a href='#SkRegion_quickContains_2_bottom'>empty</a>.
May return false even though <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>contains</a> (<a href='#SkRegion_quickContains_2_left'>left</a>, <a href='#SkRegion_quickContains_2_top'>top</a>, <a href='#SkRegion_quickContains_2_right'>right</a>, <a href='#SkRegion_quickContains_2_bottom'>bottom</a>).

### Parameters

<table>  <tr>    <td><a name='SkRegion_quickContains_2_left'><code><strong>left</strong></code></a></td>
    <td>edge of bounds on x-axis</td>
  </tr>
  <tr>    <td><a name='SkRegion_quickContains_2_top'><code><strong>top</strong></code></a></td>
    <td>edge of bounds on y-axis</td>
  </tr>
  <tr>    <td><a name='SkRegion_quickContains_2_right'><code><strong>right</strong></code></a></td>
    <td>edge of bounds on x-axis</td>
  </tr>
  <tr>    <td><a name='SkRegion_quickContains_2_bottom'><code><strong>bottom</strong></code></a></td>
    <td>edge of bounds on y-axis</td>
  </tr>
</table>

### Return Value

true quickly if <a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>are</a> <a href='SkIRect_Reference#SkIRect'>equal</a> <a href='SkIRect_Reference#SkIRect'>or</a> <a href='SkIRect_Reference#SkIRect'>inside</a>

### Example

<div><fiddle-embed name="eb6d290887e1a3a0b051b4d7b012f5e1">

#### Example Output

~~~~
quickContains 1: true
quickContains 2: true
quickContains 3: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_contains'>contains</a> <a href='#SkRegion_quickReject'>quickReject</a> <a href='#SkRegion_intersects'>intersects</a>

<a name='SkRegion_quickReject'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRegion_quickReject'>quickReject</a>(<a href='#SkRegion_quickReject'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkRect_Reference#Rect'>rect</a>) <a href='SkRect_Reference#Rect'>const</a>
</pre>

Returns true if <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>does</a> <a href='SkRegion_Reference#SkRegion'>not</a> <a href='SkRegion_Reference#SkRegion'>intersect</a> <a href='#SkRegion_quickReject_rect'>rect</a>.
Returns true if <a href='#SkRegion_quickReject_rect'>rect</a> <a href='#SkRegion_quickReject_rect'>is</a> <a href='#SkRegion_quickReject_rect'>empty</a> <a href='#SkRegion_quickReject_rect'>or</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>empty</a>.
May return false even though <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>does</a> <a href='SkRegion_Reference#SkRegion'>not</a> <a href='SkRegion_Reference#SkRegion'>intersect</a> <a href='#SkRegion_quickReject_rect'>rect</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_quickReject_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>intersect</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkRegion_quickReject_rect'>rect</a> <a href='#SkRegion_quickReject_rect'>does</a> <a href='#SkRegion_quickReject_rect'>not</a> <a href='#SkRegion_quickReject_rect'>intersect</a>

### Example

<div><fiddle-embed name="71ac24b7d91ac5ca7c14b43930d5f85d">

#### Example Output

~~~~
quickReject 1: true
quickReject 2: true
quickReject 3: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_quickContains'>quickContains</a> <a href='#SkRegion_contains'>contains</a> <a href='#SkRegion_intersects'>intersects</a>

<a name='SkRegion_quickReject_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRegion_quickReject'>quickReject</a>(<a href='#SkRegion_quickReject'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>rgn</a>) <a href='SkRegion_Reference#SkRegion'>const</a>
</pre>

Returns true if <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>does</a> <a href='SkRegion_Reference#SkRegion'>not</a> <a href='SkRegion_Reference#SkRegion'>intersect</a> <a href='#SkRegion_quickReject_2_rgn'>rgn</a>.
Returns true if <a href='#SkRegion_quickReject_2_rgn'>rgn</a> <a href='#SkRegion_quickReject_2_rgn'>is</a> <a href='#SkRegion_quickReject_2_rgn'>empty</a> <a href='#SkRegion_quickReject_2_rgn'>or</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>empty</a>.
May return false even though <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>does</a> <a href='SkRegion_Reference#SkRegion'>not</a> <a href='SkRegion_Reference#SkRegion'>intersect</a> <a href='#SkRegion_quickReject_2_rgn'>rgn</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_quickReject_2_rgn'><code><strong>rgn</strong></code></a></td>
    <td><a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>to</a> <a href='SkRegion_Reference#SkRegion'>intersect</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkRegion_quickReject_2_rgn'>rgn</a> <a href='#SkRegion_quickReject_2_rgn'>does</a> <a href='#SkRegion_quickReject_2_rgn'>not</a> <a href='#SkRegion_quickReject_2_rgn'>intersect</a>

### Example

<div><fiddle-embed name="def7dba38947c33b203e4f9db6c88be3">

#### Example Output

~~~~
quickReject 1: true
quickReject 2: true
quickReject 3: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkRegion_quickContains'>quickContains</a> <a href='#SkRegion_contains'>contains</a> <a href='#SkRegion_intersects'>intersects</a>

<a name='SkRegion_translate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void translate(int dx, int dy)
</pre>

Offsets <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>by</a> <a href='SkIPoint_Reference#IVector'>ivector</a> (<a href='#SkRegion_translate_dx'>dx</a>, <a href='#SkRegion_translate_dy'>dy</a>). <a href='#SkRegion_translate_dy'>Has</a> <a href='#SkRegion_translate_dy'>no</a> <a href='#SkRegion_translate_dy'>effect</a> <a href='#SkRegion_translate_dy'>if</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>empty</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_translate_dx'><code><strong>dx</strong></code></a></td>
    <td>x-axis offset</td>
  </tr>
  <tr>    <td><a name='SkRegion_translate_dy'><code><strong>dy</strong></code></a></td>
    <td>y-axis offset</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4e5b9e53aa1b200fed3ee6596ca01f0e"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_translate'>translate</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_offset'>offset</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_offset'>offset</a>

<a name='SkRegion_translate_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void translate(int dx, int dy, <a href='SkRegion_Reference#SkRegion'>SkRegion</a>* <a href='SkRegion_Reference#SkRegion'>dst</a>) <a href='SkRegion_Reference#SkRegion'>const</a>
</pre>

Offsets <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>by</a> <a href='SkIPoint_Reference#IVector'>ivector</a> (<a href='#SkRegion_translate_2_dx'>dx</a>, <a href='#SkRegion_translate_2_dy'>dy</a>), <a href='#SkRegion_translate_2_dy'>writing</a> <a href='#SkRegion_translate_2_dy'>result</a> <a href='#SkRegion_translate_2_dy'>to</a> <a href='#SkRegion_translate_2_dst'>dst</a>. <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>may</a> <a href='SkRegion_Reference#SkRegion'>be</a> <a href='SkRegion_Reference#SkRegion'>passed</a>
as <a href='#SkRegion_translate_2_dst'>dst</a> <a href='#SkRegion_translate_2_dst'>parameter</a>, <a href='#SkRegion_translate_2_dst'>translating</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>in</a> <a href='SkRegion_Reference#SkRegion'>place</a>. <a href='SkRegion_Reference#SkRegion'>Has</a> <a href='SkRegion_Reference#SkRegion'>no</a> <a href='SkRegion_Reference#SkRegion'>effect</a> <a href='SkRegion_Reference#SkRegion'>if</a> <a href='#SkRegion_translate_2_dst'>dst</a> <a href='#SkRegion_translate_2_dst'>is</a> <a href='#SkRegion_translate_2_dst'>nullptr</a>.
If <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>empty</a>, <a href='SkRegion_Reference#SkRegion'>sets</a> <a href='#SkRegion_translate_2_dst'>dst</a> <a href='#SkRegion_translate_2_dst'>to</a> <a href='#SkRegion_translate_2_dst'>empty</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_translate_2_dx'><code><strong>dx</strong></code></a></td>
    <td>x-axis offset</td>
  </tr>
  <tr>    <td><a name='SkRegion_translate_2_dy'><code><strong>dy</strong></code></a></td>
    <td>y-axis offset</td>
  </tr>
  <tr>    <td><a name='SkRegion_translate_2_dst'><code><strong>dst</strong></code></a></td>
    <td>translated result</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="024200960eb52fee1f471514607e6001"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_translate'>translate</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>::<a href='#SkIRect_offset'>offset</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_offset'>offset</a>

<a name='SkRegion_Op'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkRegion_Op'>Op</a> {
        <a href='#SkRegion_kDifference_Op'>kDifference_Op</a>,
        <a href='#SkRegion_kIntersect_Op'>kIntersect_Op</a>,
        <a href='#SkRegion_kUnion_Op'>kUnion_Op</a>,
        <a href='#SkRegion_kXOR_Op'>kXOR_Op</a>,
        <a href='#SkRegion_kReverseDifference_Op'>kReverseDifference_Op</a>,
        <a href='#SkRegion_kReplace_Op'>kReplace_Op</a>,
        <a href='#SkRegion_kLastOp'>kLastOp</a> = <a href='#SkRegion_kReplace_Op'>kReplace_Op</a>,
    };
</pre>

The logical operations that can be performed when combining two <a href='SkRegion_Reference#Region'>Regions</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRegion_kDifference_Op'><code>SkRegion::kDifference_Op</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Subtracts operand <a href='SkRegion_Reference#Region'>Region</a> <a href='SkRegion_Reference#Region'>from</a> <a href='SkRegion_Reference#Region'>target</a> <a href='SkRegion_Reference#Region'>Region</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRegion_kIntersect_Op'><code>SkRegion::kIntersect_Op</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Intersects operand <a href='SkRegion_Reference#Region'>Region</a> <a href='SkRegion_Reference#Region'>and</a> <a href='SkRegion_Reference#Region'>target</a> <a href='SkRegion_Reference#Region'>Region</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRegion_kUnion_Op'><code>SkRegion::kUnion_Op</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Unions operand <a href='SkRegion_Reference#Region'>Region</a> <a href='SkRegion_Reference#Region'>and</a> <a href='SkRegion_Reference#Region'>target</a> <a href='SkRegion_Reference#Region'>Region</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRegion_kXOR_Op'><code>SkRegion::kXOR_Op</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Replaces target <a href='SkRegion_Reference#Region'>Region</a> <a href='SkRegion_Reference#Region'>with</a> <a href='SkRegion_Reference#Region'>area</a> <a href='SkRegion_Reference#Region'>exclusive</a> <a href='SkRegion_Reference#Region'>to</a> <a href='SkRegion_Reference#Region'>both</a> <a href='SkRegion_Reference#Region'>Regions</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRegion_kReverseDifference_Op'><code>SkRegion::kReverseDifference_Op</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Subtracts target <a href='SkRegion_Reference#Region'>Region</a> <a href='SkRegion_Reference#Region'>from</a> <a href='SkRegion_Reference#Region'>operand</a> <a href='SkRegion_Reference#Region'>Region</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRegion_kReplace_Op'><code>SkRegion::kReplace_Op</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>5</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Replaces target <a href='SkRegion_Reference#Region'>Region</a> <a href='SkRegion_Reference#Region'>with</a> <a href='SkRegion_Reference#Region'>operand</a> <a href='SkRegion_Reference#Region'>Region</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRegion_kLastOp'><code>SkRegion::kLastOp</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>5</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
last operator</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="026dd8b180fe8e43f477fce43e9217b3"></fiddle-embed></div>

### See Also

<a href='undocumented#SkPathOp'>SkPathOp</a>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkRegion_kOpCnt'><code>SkRegion::kOpCnt</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>6</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
May be used to verify that <a href='#SkRegion_Op'>Op</a> <a href='#SkRegion_Op'>is</a> <a href='#SkRegion_Op'>a</a> <a href='#SkRegion_Op'>legal</a> <a href='#SkRegion_Op'>value</a>.</td>
  </tr>
</table>

<a name='SkRegion_op'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRegion_op'>op</a>(<a href='#SkRegion_op'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='#SkRegion_Op'>Op</a> <a href='#SkRegion_Op'>op</a>)
</pre>

Replaces <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>with</a> <a href='SkRegion_Reference#SkRegion'>the</a> <a href='SkRegion_Reference#SkRegion'>result</a> <a href='SkRegion_Reference#SkRegion'>of</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='#SkRegion_op_op'>op</a> <a href='#SkRegion_op_rect'>rect</a>.
Returns true if replaced <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>not</a> <a href='SkRegion_Reference#SkRegion'>empty</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_op_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>operand</a></td>
  </tr>
  <tr>    <td><a name='SkRegion_op_op'><code><strong>op</strong></code></a></td>
    <td>operator, one of:</td>
  </tr>
</table>

<a href='#SkRegion_kDifference_Op'>kDifference_Op</a>, <a href='#SkRegion_kIntersect_Op'>kIntersect_Op</a>, <a href='#SkRegion_kUnion_Op'>kUnion_Op</a>, <a href='#SkRegion_kXOR_Op'>kXOR_Op</a>, <a href='#SkRegion_kReverseDifference_Op'>kReverseDifference_Op</a>,
<a href='#SkRegion_kReplace_Op'>kReplace_Op</a>

### Return Value

false if result is empty

### Example

<div><fiddle-embed name="1790b2e054c536a54601138365700ac3"></fiddle-embed></div>

### See Also

<a href='#SkRegion_setRects'>setRects</a> <a href='#SkRegion_Op'>Op</a>

<a name='SkRegion_op_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRegion_op'>op</a>(<a href='#SkRegion_op'>int</a> <a href='#SkRegion_op'>left</a>, <a href='#SkRegion_op'>int</a> <a href='#SkRegion_op'>top</a>, <a href='#SkRegion_op'>int</a> <a href='#SkRegion_op'>right</a>, <a href='#SkRegion_op'>int</a> <a href='#SkRegion_op'>bottom</a>, <a href='#SkRegion_Op'>Op</a> <a href='#SkRegion_Op'>op</a>)
</pre>

Replaces <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>with</a> <a href='SkRegion_Reference#SkRegion'>the</a> <a href='SkRegion_Reference#SkRegion'>result</a> <a href='SkRegion_Reference#SkRegion'>of</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='#SkRegion_op_2_op'>op</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a> (<a href='#SkRegion_op_2_left'>left</a>, <a href='#SkRegion_op_2_top'>top</a>, <a href='#SkRegion_op_2_right'>right</a>, <a href='#SkRegion_op_2_bottom'>bottom</a>).
Returns true if replaced <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>not</a> <a href='SkRegion_Reference#SkRegion'>empty</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_op_2_left'><code><strong>left</strong></code></a></td>
    <td>edge of bounds on x-axis</td>
  </tr>
  <tr>    <td><a name='SkRegion_op_2_top'><code><strong>top</strong></code></a></td>
    <td>edge of bounds on y-axis</td>
  </tr>
  <tr>    <td><a name='SkRegion_op_2_right'><code><strong>right</strong></code></a></td>
    <td>edge of bounds on x-axis</td>
  </tr>
  <tr>    <td><a name='SkRegion_op_2_bottom'><code><strong>bottom</strong></code></a></td>
    <td>edge of bounds on y-axis</td>
  </tr>
  <tr>    <td><a name='SkRegion_op_2_op'><code><strong>op</strong></code></a></td>
    <td>operator, one of:</td>
  </tr>
</table>

<a href='#SkRegion_kDifference_Op'>kDifference_Op</a>, <a href='#SkRegion_kIntersect_Op'>kIntersect_Op</a>, <a href='#SkRegion_kUnion_Op'>kUnion_Op</a>, <a href='#SkRegion_kXOR_Op'>kXOR_Op</a>, <a href='#SkRegion_kReverseDifference_Op'>kReverseDifference_Op</a>,
<a href='#SkRegion_kReplace_Op'>kReplace_Op</a>

### Return Value

false if result is empty

### Example

<div><fiddle-embed name="2e3497890d523235f96680716c321098"></fiddle-embed></div>

### See Also

<a href='#SkRegion_setRects'>setRects</a> <a href='#SkRegion_Op'>Op</a>

<a name='SkRegion_op_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRegion_op'>op</a>(<a href='#SkRegion_op'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>rgn</a>, <a href='#SkRegion_Op'>Op</a> <a href='#SkRegion_Op'>op</a>)
</pre>

Replaces <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>with</a> <a href='SkRegion_Reference#SkRegion'>the</a> <a href='SkRegion_Reference#SkRegion'>result</a> <a href='SkRegion_Reference#SkRegion'>of</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='#SkRegion_op_3_op'>op</a> <a href='#SkRegion_op_3_rgn'>rgn</a>.
Returns true if replaced <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>not</a> <a href='SkRegion_Reference#SkRegion'>empty</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_op_3_rgn'><code><strong>rgn</strong></code></a></td>
    <td><a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>operand</a></td>
  </tr>
  <tr>    <td><a name='SkRegion_op_3_op'><code><strong>op</strong></code></a></td>
    <td>operator, one of:</td>
  </tr>
</table>

<a href='#SkRegion_kDifference_Op'>kDifference_Op</a>, <a href='#SkRegion_kIntersect_Op'>kIntersect_Op</a>, <a href='#SkRegion_kUnion_Op'>kUnion_Op</a>, <a href='#SkRegion_kXOR_Op'>kXOR_Op</a>, <a href='#SkRegion_kReverseDifference_Op'>kReverseDifference_Op</a>,
<a href='#SkRegion_kReplace_Op'>kReplace_Op</a>

### Return Value

false if result is empty

### Example

<div><fiddle-embed name="65f4eccea3514ed7e37b5067e15efddb"></fiddle-embed></div>

### See Also

<a href='#SkRegion_setRects'>setRects</a> <a href='#SkRegion_Op'>Op</a>

<a name='SkRegion_op_4'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRegion_op'>op</a>(<a href='#SkRegion_op'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='SkRect_Reference#Rect'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>rgn</a>, <a href='#SkRegion_Op'>Op</a> <a href='#SkRegion_Op'>op</a>)
</pre>

Replaces <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>with</a> <a href='SkRegion_Reference#SkRegion'>the</a> <a href='SkRegion_Reference#SkRegion'>result</a> <a href='SkRegion_Reference#SkRegion'>of</a> <a href='#SkRegion_op_4_rect'>rect</a> <a href='#SkRegion_op_4_op'>op</a> <a href='#SkRegion_op_4_rgn'>rgn</a>.
Returns true if replaced <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>not</a> <a href='SkRegion_Reference#SkRegion'>empty</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_op_4_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>operand</a></td>
  </tr>
  <tr>    <td><a name='SkRegion_op_4_rgn'><code><strong>rgn</strong></code></a></td>
    <td><a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>operand</a></td>
  </tr>
  <tr>    <td><a name='SkRegion_op_4_op'><code><strong>op</strong></code></a></td>
    <td>operator, one of:</td>
  </tr>
</table>

<a href='#SkRegion_kDifference_Op'>kDifference_Op</a>, <a href='#SkRegion_kIntersect_Op'>kIntersect_Op</a>, <a href='#SkRegion_kUnion_Op'>kUnion_Op</a>, <a href='#SkRegion_kXOR_Op'>kXOR_Op</a>, <a href='#SkRegion_kReverseDifference_Op'>kReverseDifference_Op</a>,
<a href='#SkRegion_kReplace_Op'>kReplace_Op</a>

### Return Value

false if result is empty

### Example

<div><fiddle-embed name="3f964be1e1fd2fbb977b655d3a928f0a"></fiddle-embed></div>

### See Also

<a href='#SkRegion_setRects'>setRects</a> <a href='#SkRegion_Op'>Op</a>

<a name='SkRegion_op_5'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRegion_op'>op</a>(<a href='#SkRegion_op'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>rgn</a>, <a href='SkRegion_Reference#SkRegion'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='#SkRegion_Op'>Op</a> <a href='#SkRegion_Op'>op</a>)
</pre>

Replaces <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>with</a> <a href='SkRegion_Reference#SkRegion'>the</a> <a href='SkRegion_Reference#SkRegion'>result</a> <a href='SkRegion_Reference#SkRegion'>of</a> <a href='#SkRegion_op_5_rgn'>rgn</a> <a href='#SkRegion_op_5_op'>op</a> <a href='#SkRegion_op_5_rect'>rect</a>.
Returns true if replaced <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>not</a> <a href='SkRegion_Reference#SkRegion'>empty</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_op_5_rgn'><code><strong>rgn</strong></code></a></td>
    <td><a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>operand</a></td>
  </tr>
  <tr>    <td><a name='SkRegion_op_5_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkIRect_Reference#SkIRect'>SkIRect</a> <a href='SkIRect_Reference#SkIRect'>operand</a></td>
  </tr>
  <tr>    <td><a name='SkRegion_op_5_op'><code><strong>op</strong></code></a></td>
    <td>operator, one of:</td>
  </tr>
</table>

<a href='#SkRegion_kDifference_Op'>kDifference_Op</a>, <a href='#SkRegion_kIntersect_Op'>kIntersect_Op</a>, <a href='#SkRegion_kUnion_Op'>kUnion_Op</a>, <a href='#SkRegion_kXOR_Op'>kXOR_Op</a>, <a href='#SkRegion_kReverseDifference_Op'>kReverseDifference_Op</a>,
<a href='#SkRegion_kReplace_Op'>kReplace_Op</a>

### Return Value

false if result is empty

### Example

<div><fiddle-embed name="e623208dd44f0b24499ac5f1593d1b39"></fiddle-embed></div>

### See Also

<a href='#SkRegion_setRects'>setRects</a> <a href='#SkRegion_Op'>Op</a>

<a name='SkRegion_op_6'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkRegion_op'>op</a>(<a href='#SkRegion_op'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>rgna</a>, <a href='SkRegion_Reference#SkRegion'>const</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>& <a href='SkRegion_Reference#SkRegion'>rgnb</a>, <a href='#SkRegion_Op'>Op</a> <a href='#SkRegion_Op'>op</a>)
</pre>

Replaces <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>with</a> <a href='SkRegion_Reference#SkRegion'>the</a> <a href='SkRegion_Reference#SkRegion'>result</a> <a href='SkRegion_Reference#SkRegion'>of</a> <a href='#SkRegion_op_6_rgna'>rgna</a> <a href='#SkRegion_op_6_op'>op</a> <a href='#SkRegion_op_6_rgnb'>rgnb</a>.
Returns true if replaced <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>is</a> <a href='SkRegion_Reference#SkRegion'>not</a> <a href='SkRegion_Reference#SkRegion'>empty</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_op_6_rgna'><code><strong>rgna</strong></code></a></td>
    <td><a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>operand</a></td>
  </tr>
  <tr>    <td><a name='SkRegion_op_6_rgnb'><code><strong>rgnb</strong></code></a></td>
    <td><a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>operand</a></td>
  </tr>
  <tr>    <td><a name='SkRegion_op_6_op'><code><strong>op</strong></code></a></td>
    <td>operator, one of:</td>
  </tr>
</table>

<a href='#SkRegion_kDifference_Op'>kDifference_Op</a>, <a href='#SkRegion_kIntersect_Op'>kIntersect_Op</a>, <a href='#SkRegion_kUnion_Op'>kUnion_Op</a>, <a href='#SkRegion_kXOR_Op'>kXOR_Op</a>, <a href='#SkRegion_kReverseDifference_Op'>kReverseDifference_Op</a>,
<a href='#SkRegion_kReplace_Op'>kReplace_Op</a>

### Return Value

false if result is empty

### Example

<div><fiddle-embed name="13de1a6fcb2302a2a30278cb88d3e17d"></fiddle-embed></div>

### See Also

<a href='#SkRegion_setRects'>setRects</a> <a href='#SkRegion_Op'>Op</a>

<a name='SkRegion_toString'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
char* <a href='#SkRegion_toString'>toString</a>()
</pre>

Private: Android framework only.

### Return Value

<a href='undocumented#String'>string</a> <a href='undocumented#String'>representation</a> <a href='undocumented#String'>of</a> <a href='SkRegion_Reference#Region'>Region</a>

<a name='SkRegion_writeToMemory'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkRegion_writeToMemory'>writeToMemory</a>(<a href='#SkRegion_writeToMemory'>void</a>* <a href='#SkRegion_writeToMemory'>buffer</a>) <a href='#SkRegion_writeToMemory'>const</a>
</pre>

Writes <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>to</a> <a href='#SkRegion_writeToMemory_buffer'>buffer</a>, <a href='#SkRegion_writeToMemory_buffer'>and</a> <a href='#SkRegion_writeToMemory_buffer'>returns</a> <a href='#SkRegion_writeToMemory_buffer'>number</a> <a href='#SkRegion_writeToMemory_buffer'>of</a> <a href='#SkRegion_writeToMemory_buffer'>bytes</a> <a href='#SkRegion_writeToMemory_buffer'>written</a>.
If <a href='#SkRegion_writeToMemory_buffer'>buffer</a> <a href='#SkRegion_writeToMemory_buffer'>is</a> <a href='#SkRegion_writeToMemory_buffer'>nullptr</a>, <a href='#SkRegion_writeToMemory_buffer'>returns</a> <a href='#SkRegion_writeToMemory_buffer'>number</a> <a href='#SkRegion_writeToMemory_buffer'>number</a> <a href='#SkRegion_writeToMemory_buffer'>of</a> <a href='#SkRegion_writeToMemory_buffer'>bytes</a> <a href='#SkRegion_writeToMemory_buffer'>that</a> <a href='#SkRegion_writeToMemory_buffer'>would</a> <a href='#SkRegion_writeToMemory_buffer'>be</a> <a href='#SkRegion_writeToMemory_buffer'>written</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_writeToMemory_buffer'><code><strong>buffer</strong></code></a></td>
    <td>storage for binary <a href='undocumented#Data'>data</a></td>
  </tr>
</table>

### Return Value

<a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='SkRegion_Reference#SkRegion'>SkRegion</a>

### Example

<div><fiddle-embed name="1419d2a8c22c355ab46240865d056ee5"></fiddle-embed></div>

### See Also

<a href='#SkRegion_readFromMemory'>readFromMemory</a>

<a name='SkRegion_readFromMemory'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkRegion_readFromMemory'>readFromMemory</a>(<a href='#SkRegion_readFromMemory'>const</a> <a href='#SkRegion_readFromMemory'>void</a>* <a href='#SkRegion_readFromMemory'>buffer</a>, <a href='#SkRegion_readFromMemory'>size_t</a> <a href='#SkRegion_readFromMemory'>length</a>)
</pre>

Constructs <a href='SkRegion_Reference#SkRegion'>SkRegion</a> <a href='SkRegion_Reference#SkRegion'>from</a> <a href='#SkRegion_readFromMemory_buffer'>buffer</a> <a href='#SkRegion_readFromMemory_buffer'>of</a> <a href='undocumented#Size'>size</a> <a href='#SkRegion_readFromMemory_length'>length</a>. <a href='#SkRegion_readFromMemory_length'>Returns</a> <a href='#SkRegion_readFromMemory_length'>bytes</a> <a href='#SkRegion_readFromMemory_length'>read</a>.
Returned value will be multiple of four or zero if <a href='#SkRegion_readFromMemory_length'>length</a> <a href='#SkRegion_readFromMemory_length'>was</a> <a href='#SkRegion_readFromMemory_length'>too</a> <a href='#SkRegion_readFromMemory_length'>small</a>.

### Parameters

<table>  <tr>    <td><a name='SkRegion_readFromMemory_buffer'><code><strong>buffer</strong></code></a></td>
    <td>storage for binary <a href='undocumented#Data'>data</a></td>
  </tr>
  <tr>    <td><a name='SkRegion_readFromMemory_length'><code><strong>length</strong></code></a></td>
    <td><a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='#SkRegion_readFromMemory_buffer'>buffer</a></td>
  </tr>
</table>

### Return Value

bytes read

### Example

<div><fiddle-embed name="1ede346c430ef23df0eaaf0773dd6a15"></fiddle-embed></div>

### See Also

<a href='#SkRegion_writeToMemory'>writeToMemory</a>

