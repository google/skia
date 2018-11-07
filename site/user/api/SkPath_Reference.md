SkPath Reference
===


<a name='SkPath'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkPath_Reference#SkPath'>SkPath</a> {
<a href='SkPath_Reference#SkPath'>public</a>:
    <a href='SkPath_Reference#SkPath'>enum</a> <a href='#SkPath_Direction'>Direction</a> : <a href='#SkPath_Direction'>int</a> {
        <a href='#SkPath_kCW_Direction'>kCW_Direction</a>,
        <a href='#SkPath_kCCW_Direction'>kCCW_Direction</a>,
    };

    <a href='#SkPath_empty_constructor'>SkPath()</a>;
    <a href='SkPath_Reference#SkPath'>SkPath</a>(<a href='SkPath_Reference#SkPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>);
    ~<a href='#SkPath_empty_constructor'>SkPath()</a>;
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>operator</a>=(<a href='SkPath_Reference#SkPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>);
    <a href='SkPath_Reference#Path'>friend</a> <a href='SkPath_Reference#Path'>bool</a> <a href='SkPath_Reference#Path'>operator</a>==(<a href='SkPath_Reference#Path'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>a</a>, <a href='SkPath_Reference#SkPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>b</a>);
    <a href='SkPath_Reference#SkPath'>friend</a> <a href='SkPath_Reference#SkPath'>bool</a> <a href='SkPath_Reference#SkPath'>operator</a>!=(<a href='SkPath_Reference#SkPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>a</a>, <a href='SkPath_Reference#SkPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>b</a>);
    <a href='SkPath_Reference#SkPath'>bool</a> <a href='#SkPath_isInterpolatable'>isInterpolatable</a>(<a href='#SkPath_isInterpolatable'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>compare</a>) <a href='SkPath_Reference#SkPath'>const</a>;
    <a href='SkPath_Reference#SkPath'>bool</a> <a href='SkPath_Reference#SkPath'>interpolate</a>(<a href='SkPath_Reference#SkPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>ending</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>weight</a>, <a href='SkPath_Reference#SkPath'>SkPath</a>* <a href='SkPath_Reference#SkPath'>out</a>) <a href='SkPath_Reference#SkPath'>const</a>;
    <a href='SkPath_Reference#SkPath'>bool</a> <a href='#SkPath_unique'>unique()</a> <a href='#SkPath_unique'>const</a>;

    <a href='#SkPath_unique'>enum</a> <a href='#SkPath_FillType'>FillType</a> {
        <a href='#SkPath_kWinding_FillType'>kWinding_FillType</a>,
        <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd_FillType</a>,
        <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding_FillType</a>,
        <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd_FillType</a>,
    };

    <a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_getFillType'>getFillType</a>() <a href='#SkPath_getFillType'>const</a>;
    <a href='#SkPath_getFillType'>void</a> <a href='#SkPath_setFillType'>setFillType</a>(<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_FillType'>ft</a>);
    <a href='#SkPath_FillType'>bool</a> <a href='#SkPath_isInverseFillType'>isInverseFillType</a>() <a href='#SkPath_isInverseFillType'>const</a>;
    <a href='#SkPath_isInverseFillType'>void</a> <a href='#SkPath_toggleInverseFillType'>toggleInverseFillType</a>();

    <a href='#SkPath_toggleInverseFillType'>enum</a> <a href='#SkPath_Convexity'>Convexity</a> : <a href='#SkPath_Convexity'>uint8_t</a> {
        <a href='#SkPath_kUnknown_Convexity'>kUnknown_Convexity</a>,
        <a href='#SkPath_kConvex_Convexity'>kConvex_Convexity</a>,
        <a href='#SkPath_kConcave_Convexity'>kConcave_Convexity</a>,
    };

    <a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_getConvexity'>getConvexity</a>() <a href='#SkPath_getConvexity'>const</a>;
    <a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_getConvexityOrUnknown'>getConvexityOrUnknown</a>() <a href='#SkPath_getConvexityOrUnknown'>const</a>;
    <a href='#SkPath_getConvexityOrUnknown'>void</a> <a href='#SkPath_setConvexity'>setConvexity</a>(<a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_Convexity'>convexity</a>);
    <a href='#SkPath_Convexity'>bool</a> <a href='#SkPath_isConvex'>isConvex</a>() <a href='#SkPath_isConvex'>const</a>;
    <a href='#SkPath_isConvex'>bool</a> <a href='#SkPath_isOval'>isOval</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a>) <a href='SkRect_Reference#SkRect'>const</a>;
    <a href='SkRect_Reference#SkRect'>bool</a> <a href='#SkPath_isRRect'>isRRect</a>(<a href='SkRRect_Reference#SkRRect'>SkRRect</a>* <a href='SkRRect_Reference#SkRRect'>rrect</a>) <a href='SkRRect_Reference#SkRRect'>const</a>;
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_reset'>reset()</a>;
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_rewind'>rewind()</a>;
    <a href='#SkPath_rewind'>bool</a> <a href='#SkPath_isEmpty'>isEmpty</a>() <a href='#SkPath_isEmpty'>const</a>;
    <a href='#SkPath_isEmpty'>bool</a> <a href='#SkPath_isLastContourClosed'>isLastContourClosed</a>() <a href='#SkPath_isLastContourClosed'>const</a>;
    <a href='#SkPath_isLastContourClosed'>bool</a> <a href='#SkPath_isFinite'>isFinite</a>() <a href='#SkPath_isFinite'>const</a>;
    <a href='#SkPath_isFinite'>bool</a> <a href='#SkPath_isVolatile'>isVolatile</a>() <a href='#SkPath_isVolatile'>const</a>;
    <a href='#SkPath_isVolatile'>void</a> <a href='#SkPath_setIsVolatile'>setIsVolatile</a>(<a href='#SkPath_setIsVolatile'>bool</a> <a href='#SkPath_isVolatile'>isVolatile</a>);
    <a href='#SkPath_isVolatile'>static</a> <a href='#SkPath_isVolatile'>bool</a> <a href='#SkPath_IsLineDegenerate'>IsLineDegenerate</a>(<a href='#SkPath_IsLineDegenerate'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p1</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p2</a>, <a href='SkPoint_Reference#SkPoint'>bool</a> <a href='SkPoint_Reference#SkPoint'>exact</a>);
    <a href='SkPoint_Reference#SkPoint'>static</a> <a href='SkPoint_Reference#SkPoint'>bool</a> <a href='#SkPath_IsQuadDegenerate'>IsQuadDegenerate</a>(<a href='#SkPath_IsQuadDegenerate'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p1</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p2</a>,
                                 <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p3</a>, <a href='SkPoint_Reference#SkPoint'>bool</a> <a href='SkPoint_Reference#SkPoint'>exact</a>);
    <a href='SkPoint_Reference#SkPoint'>static</a> <a href='SkPoint_Reference#SkPoint'>bool</a> <a href='#SkPath_IsCubicDegenerate'>IsCubicDegenerate</a>(<a href='#SkPath_IsCubicDegenerate'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p1</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p2</a>,
                                  <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p3</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p4</a>, <a href='SkPoint_Reference#SkPoint'>bool</a> <a href='SkPoint_Reference#SkPoint'>exact</a>);
    <a href='SkPoint_Reference#SkPoint'>bool</a> <a href='#SkPath_isLine'>isLine</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='undocumented#Line'>line</a>[2]) <a href='undocumented#Line'>const</a>;
    <a href='undocumented#Line'>int</a> <a href='#SkPath_countPoints'>countPoints</a>() <a href='#SkPath_countPoints'>const</a>;
    <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkPath_getPoint'>getPoint</a>(<a href='#SkPath_getPoint'>int</a> <a href='#SkPath_getPoint'>index</a>) <a href='#SkPath_getPoint'>const</a>;
    <a href='#SkPath_getPoint'>int</a> <a href='#SkPath_getPoints'>getPoints</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#Point'>points</a>[], <a href='SkPoint_Reference#Point'>int</a> <a href='SkPoint_Reference#Point'>max</a>) <a href='SkPoint_Reference#Point'>const</a>;
    <a href='SkPoint_Reference#Point'>int</a> <a href='#SkPath_countVerbs'>countVerbs</a>() <a href='#SkPath_countVerbs'>const</a>;
    <a href='#SkPath_countVerbs'>int</a> <a href='#SkPath_getVerbs'>getVerbs</a>(<a href='#SkPath_getVerbs'>uint8_t</a> <a href='SkPath_Reference#Verb'>verbs</a>[], <a href='SkPath_Reference#Verb'>int</a> <a href='SkPath_Reference#Verb'>max</a>) <a href='SkPath_Reference#Verb'>const</a>;
    <a href='SkPath_Reference#Verb'>void</a> <a href='#SkPath_swap'>swap</a>(<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>other</a>);
    <a href='SkPath_Reference#SkPath'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkPath_getBounds'>getBounds</a>() <a href='#SkPath_getBounds'>const</a>;
    <a href='#SkPath_getBounds'>void</a> <a href='#SkPath_updateBoundsCache'>updateBoundsCache</a>() <a href='#SkPath_updateBoundsCache'>const</a>;
    <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkPath_computeTightBounds'>computeTightBounds</a>() <a href='#SkPath_computeTightBounds'>const</a>;
    <a href='#SkPath_computeTightBounds'>bool</a> <a href='#SkPath_conservativelyContainsRect'>conservativelyContainsRect</a>(<a href='#SkPath_conservativelyContainsRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>) <a href='SkRect_Reference#Rect'>const</a>;
    <a href='SkRect_Reference#Rect'>void</a> <a href='#SkPath_incReserve'>incReserve</a>(<a href='#SkPath_incReserve'>int</a> <a href='#SkPath_incReserve'>extraPtCount</a>);
    <a href='#SkPath_incReserve'>void</a> <a href='#SkPath_shrinkToFit'>shrinkToFit</a>();
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_moveTo'>moveTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_moveTo'>moveTo</a>(<a href='#SkPath_moveTo'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_rMoveTo'>rMoveTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_lineTo'>lineTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_lineTo'>lineTo</a>(<a href='#SkPath_lineTo'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_rLineTo'>rLineTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_quadTo'>quadTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y2</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_quadTo'>quadTo</a>(<a href='#SkPath_quadTo'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p1</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p2</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_rQuadTo'>rQuadTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy2</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_conicTo'>conicTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y2</a>,
                    <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>w</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_conicTo'>conicTo</a>(<a href='#SkPath_conicTo'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p1</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>w</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_rConicTo'>rConicTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy2</a>,
                     <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>w</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_cubicTo'>cubicTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y2</a>,
                    <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x3</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y3</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_cubicTo'>cubicTo</a>(<a href='#SkPath_cubicTo'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p1</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p2</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p3</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_rCubicTo'>rCubicTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy2</a>,
                     <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx3</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy3</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_arcTo'>arcTo</a>(<a href='#SkPath_arcTo'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='undocumented#Oval'>oval</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>startAngle</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sweepAngle</a>, <a href='undocumented#SkScalar'>bool</a> <a href='undocumented#SkScalar'>forceMoveTo</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_arcTo'>arcTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>radius</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_arcTo'>arcTo</a>(<a href='#SkPath_arcTo'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>p1</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>p2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>radius</a>);

    <a href='undocumented#SkScalar'>enum</a> <a href='#SkPath_ArcSize'>ArcSize</a> {
        <a href='#SkPath_kSmall_ArcSize'>kSmall_ArcSize</a>,
        <a href='#SkPath_kLarge_ArcSize'>kLarge_ArcSize</a>,
    };

    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_arcTo'>arcTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>rx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>ry</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>xAxisRotate</a>, <a href='#SkPath_ArcSize'>ArcSize</a> <a href='#SkPath_ArcSize'>largeArc</a>,
                  <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>sweep</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_arcTo'>arcTo</a>(<a href='#SkPath_arcTo'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>r</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>xAxisRotate</a>, <a href='#SkPath_ArcSize'>ArcSize</a> <a href='#SkPath_ArcSize'>largeArc</a>, <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>sweep</a>,
               <a href='#SkPath_Direction'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>xy</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_rArcTo'>rArcTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>rx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>ry</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>xAxisRotate</a>, <a href='#SkPath_ArcSize'>ArcSize</a> <a href='#SkPath_ArcSize'>largeArc</a>,
                   <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>sweep</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_close'>close()</a>;
    <a href='#SkPath_close'>static</a> <a href='#SkPath_close'>bool</a> <a href='#SkPath_IsInverseFillType'>IsInverseFillType</a>(<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_FillType'>fill</a>);
    <a href='#SkPath_FillType'>static</a> <a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_ConvertToNonInverseFillType'>ConvertToNonInverseFillType</a>(<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_FillType'>fill</a>);
    <a href='#SkPath_FillType'>static</a> <a href='#SkPath_FillType'>int</a> <a href='#SkPath_ConvertConicToQuads'>ConvertConicToQuads</a>(<a href='#SkPath_ConvertConicToQuads'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p0</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p1</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p2</a>,
                                   <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>w</a>, <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pts</a>[], <a href='SkPoint_Reference#SkPoint'>int</a> <a href='SkPoint_Reference#SkPoint'>pow2</a>);
    <a href='SkPoint_Reference#SkPoint'>bool</a> <a href='#SkPath_isRect'>isRect</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#Rect'>rect</a>, <a href='SkRect_Reference#Rect'>bool</a>* <a href='SkRect_Reference#Rect'>isClosed</a> = <a href='SkRect_Reference#Rect'>nullptr</a>, <a href='#SkPath_Direction'>Direction</a>* <a href='#SkPath_Direction'>direction</a> = <a href='#SkPath_Direction'>nullptr</a>) <a href='#SkPath_Direction'>const</a>;
    <a href='#SkPath_Direction'>bool</a> <a href='#SkPath_isNestedFillRects'>isNestedFillRects</a>(<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#Rect'>rect</a>[2], <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>dirs</a>[2] = <a href='#SkPath_Direction'>nullptr</a>) <a href='#SkPath_Direction'>const</a>;
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addRect'>addRect</a>(<a href='#SkPath_addRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>dir</a> = <a href='#SkPath_kCW_Direction'>kCW_Direction</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addRect'>addRect</a>(<a href='#SkPath_addRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>dir</a>, <a href='#SkPath_Direction'>unsigned</a> <a href='#SkPath_Direction'>start</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addRect'>addRect</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>left</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>top</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>right</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>bottom</a>,
                    <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>dir</a> = <a href='#SkPath_kCW_Direction'>kCW_Direction</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addOval'>addOval</a>(<a href='#SkPath_addOval'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='undocumented#Oval'>oval</a>, <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>dir</a> = <a href='#SkPath_kCW_Direction'>kCW_Direction</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addOval'>addOval</a>(<a href='#SkPath_addOval'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='undocumented#Oval'>oval</a>, <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>dir</a>, <a href='#SkPath_Direction'>unsigned</a> <a href='#SkPath_Direction'>start</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addCircle'>addCircle</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>radius</a>,
                      <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>dir</a> = <a href='#SkPath_kCW_Direction'>kCW_Direction</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addArc'>addArc</a>(<a href='#SkPath_addArc'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='undocumented#Oval'>oval</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>startAngle</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sweepAngle</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addRoundRect'>addRoundRect</a>(<a href='#SkPath_addRoundRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>rx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>ry</a>,
                         <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>dir</a> = <a href='#SkPath_kCW_Direction'>kCW_Direction</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addRoundRect'>addRoundRect</a>(<a href='#SkPath_addRoundRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='SkRect_Reference#Rect'>const</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>radii</a>[],
                         <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>dir</a> = <a href='#SkPath_kCW_Direction'>kCW_Direction</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addRRect'>addRRect</a>(<a href='#SkPath_addRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>rrect</a>, <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>dir</a> = <a href='#SkPath_kCW_Direction'>kCW_Direction</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addRRect'>addRRect</a>(<a href='#SkPath_addRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>rrect</a>, <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>dir</a>, <a href='#SkPath_Direction'>unsigned</a> <a href='#SkPath_Direction'>start</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addPoly'>addPoly</a>(<a href='#SkPath_addPoly'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pts</a>[], <a href='SkPoint_Reference#SkPoint'>int</a> <a href='SkPoint_Reference#SkPoint'>count</a>, <a href='SkPoint_Reference#SkPoint'>bool</a> <a href='SkPoint_Reference#SkPoint'>close</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addPoly'>addPoly</a>(<a href='#SkPath_addPoly'>const</a> <a href='#SkPath_addPoly'>std</a>::<a href='#SkPath_addPoly'>initializer_list</a><<a href='SkPoint_Reference#SkPoint'>SkPoint</a>>& <a href='SkPoint_Reference#SkPoint'>list</a>, <a href='SkPoint_Reference#SkPoint'>bool</a> <a href='SkPoint_Reference#SkPoint'>close</a>);

    <a href='SkPoint_Reference#SkPoint'>enum</a> <a href='#SkPath_AddPathMode'>AddPathMode</a> {
        <a href='#SkPath_kAppend_AddPathMode'>kAppend_AddPathMode</a>,
        <a href='#SkPath_kExtend_AddPathMode'>kExtend_AddPathMode</a>,
    };

    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addPath'>addPath</a>(<a href='#SkPath_addPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>src</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>,
                    <a href='#SkPath_AddPathMode'>AddPathMode</a> <a href='#SkPath_AddPathMode'>mode</a> = <a href='#SkPath_kAppend_AddPathMode'>kAppend_AddPathMode</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addPath'>addPath</a>(<a href='#SkPath_addPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>src</a>, <a href='#SkPath_AddPathMode'>AddPathMode</a> <a href='#SkPath_AddPathMode'>mode</a> = <a href='#SkPath_kAppend_AddPathMode'>kAppend_AddPathMode</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addPath'>addPath</a>(<a href='#SkPath_addPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>src</a>, <a href='SkPath_Reference#SkPath'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#Matrix'>matrix</a>,
                    <a href='#SkPath_AddPathMode'>AddPathMode</a> <a href='#SkPath_AddPathMode'>mode</a> = <a href='#SkPath_kAppend_AddPathMode'>kAppend_AddPathMode</a>);
    <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_reverseAddPath'>reverseAddPath</a>(<a href='#SkPath_reverseAddPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>src</a>);
    <a href='SkPath_Reference#SkPath'>void</a> <a href='SkPath_Reference#SkPath'>offset</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>, <a href='SkPath_Reference#SkPath'>SkPath</a>* <a href='SkPath_Reference#SkPath'>dst</a>) <a href='SkPath_Reference#SkPath'>const</a>;
    <a href='SkPath_Reference#SkPath'>void</a> <a href='SkPath_Reference#SkPath'>offset</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='undocumented#SkScalar'>transform</a>(<a href='undocumented#SkScalar'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#Matrix'>matrix</a>, <a href='SkPath_Reference#SkPath'>SkPath</a>* <a href='SkPath_Reference#SkPath'>dst</a>) <a href='SkPath_Reference#SkPath'>const</a>;
    <a href='SkPath_Reference#SkPath'>void</a> <a href='SkPath_Reference#SkPath'>transform</a>(<a href='SkPath_Reference#SkPath'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#Matrix'>matrix</a>);
    <a href='SkMatrix_Reference#Matrix'>bool</a> <a href='#SkPath_getLastPt'>getLastPt</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a>* <a href='SkPoint_Reference#SkPoint'>lastPt</a>) <a href='SkPoint_Reference#SkPoint'>const</a>;
    <a href='SkPoint_Reference#SkPoint'>void</a> <a href='#SkPath_setLastPt'>setLastPt</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkPath_setLastPt'>setLastPt</a>(<a href='#SkPath_setLastPt'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p</a>);

    <a href='SkPoint_Reference#SkPoint'>enum</a> <a href='#SkPath_SegmentMask'>SegmentMask</a> {
        <a href='#SkPath_kLine_SegmentMask'>kLine_SegmentMask</a> = 1 << 0,
        <a href='#SkPath_kQuad_SegmentMask'>kQuad_SegmentMask</a> = 1 << 1,
        <a href='#SkPath_kConic_SegmentMask'>kConic_SegmentMask</a> = 1 << 2,
        <a href='#SkPath_kCubic_SegmentMask'>kCubic_SegmentMask</a> = 1 << 3,
    };

    <a href='#SkPath_kCubic_SegmentMask'>uint32_t</a> <a href='#SkPath_getSegmentMasks'>getSegmentMasks</a>() <a href='#SkPath_getSegmentMasks'>const</a>;

    <a href='#SkPath_getSegmentMasks'>enum</a> <a href='#SkPath_Verb'>Verb</a> {
        <a href='#SkPath_kMove_Verb'>kMove_Verb</a>,
        <a href='#SkPath_kLine_Verb'>kLine_Verb</a>,
        <a href='#SkPath_kQuad_Verb'>kQuad_Verb</a>,
        <a href='#SkPath_kConic_Verb'>kConic_Verb</a>,
        <a href='#SkPath_kCubic_Verb'>kCubic_Verb</a>,
        <a href='#SkPath_kClose_Verb'>kClose_Verb</a>,
        <a href='#SkPath_kDone_Verb'>kDone_Verb</a>,
    };

    <a href='#SkPath_kDone_Verb'>class</a> <a href='#SkPath_Iter'>Iter</a> {
    <a href='#SkPath_Iter'>public</a>:
        <a href='#SkPath_Iter'>Iter</a>();
        <a href='#SkPath_Iter'>Iter</a>(<a href='#SkPath_Iter'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>, <a href='SkPath_Reference#Path'>bool</a> <a href='SkPath_Reference#Path'>forceClose</a>);
        <a href='SkPath_Reference#Path'>void</a> <a href='SkPath_Reference#Path'>setPath</a>(<a href='SkPath_Reference#Path'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>, <a href='SkPath_Reference#Path'>bool</a> <a href='SkPath_Reference#Path'>forceClose</a>);
        <a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>next</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pts</a>[4], <a href='SkPoint_Reference#SkPoint'>bool</a> <a href='SkPoint_Reference#SkPoint'>doConsumeDegenerates</a> = <a href='SkPoint_Reference#SkPoint'>true</a>, <a href='SkPoint_Reference#SkPoint'>bool</a> <a href='SkPoint_Reference#SkPoint'>exact</a> = <a href='SkPoint_Reference#SkPoint'>false</a>);
        <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>conicWeight</a>() <a href='undocumented#SkScalar'>const</a>;
        <a href='undocumented#SkScalar'>bool</a> <a href='undocumented#SkScalar'>isCloseLine</a>() <a href='undocumented#SkScalar'>const</a>;
        <a href='undocumented#SkScalar'>bool</a> <a href='undocumented#SkScalar'>isClosedContour</a>() <a href='undocumented#SkScalar'>const</a>;
    };

    <a href='undocumented#SkScalar'>class</a> <a href='#SkPath_RawIter'>RawIter</a> {
    <a href='#SkPath_RawIter'>public</a>:
        <a href='#SkPath_RawIter'>RawIter</a>();
        <a href='#SkPath_RawIter'>RawIter</a>(<a href='#SkPath_RawIter'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>);
        <a href='SkPath_Reference#Path'>void</a> <a href='SkPath_Reference#Path'>setPath</a>(<a href='SkPath_Reference#Path'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>);
        <a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>next</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pts</a>[4]);
        <a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>peek()</a> <a href='#SkPath_Verb'>const</a>;
        <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>conicWeight</a>() <a href='undocumented#SkScalar'>const</a>;
    };

    <a href='undocumented#SkScalar'>bool</a> <a href='undocumented#SkScalar'>contains</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>) <a href='undocumented#SkScalar'>const</a>;
    <a href='undocumented#SkScalar'>void</a> <a href='#SkPath_dump'>dump</a>(<a href='SkWStream_Reference#SkWStream'>SkWStream</a>* <a href='SkStream_Reference#Stream'>stream</a>, <a href='SkStream_Reference#Stream'>bool</a> <a href='SkStream_Reference#Stream'>forceClose</a>, <a href='SkStream_Reference#Stream'>bool</a> <a href='SkStream_Reference#Stream'>dumpAsHex</a>) <a href='SkStream_Reference#Stream'>const</a>;
    <a href='SkStream_Reference#Stream'>void</a> <a href='#SkPath_dump'>dump()</a> <a href='#SkPath_dump'>const</a>;
    <a href='#SkPath_dump'>void</a> <a href='#SkPath_dumpHex'>dumpHex</a>() <a href='#SkPath_dumpHex'>const</a>;
    <a href='#SkPath_dumpHex'>size_t</a> <a href='#SkPath_writeToMemory'>writeToMemory</a>(<a href='#SkPath_writeToMemory'>void</a>* <a href='#SkPath_writeToMemory'>buffer</a>) <a href='#SkPath_writeToMemory'>const</a>;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkData'>SkData</a>> <a href='#SkPath_serialize'>serialize()</a> <a href='#SkPath_serialize'>const</a>;
    <a href='#SkPath_serialize'>size_t</a> <a href='#SkPath_readFromMemory'>readFromMemory</a>(<a href='#SkPath_readFromMemory'>const</a> <a href='#SkPath_readFromMemory'>void</a>* <a href='#SkPath_readFromMemory'>buffer</a>, <a href='#SkPath_readFromMemory'>size_t</a> <a href='#SkPath_readFromMemory'>length</a>);
    <a href='#SkPath_readFromMemory'>uint32_t</a> <a href='#SkPath_getGenerationID'>getGenerationID</a>() <a href='#SkPath_getGenerationID'>const</a>;
    <a href='#SkPath_getGenerationID'>bool</a> <a href='#SkPath_isValid'>isValid</a>() <a href='#SkPath_isValid'>const</a>;
    <a href='#SkPath_isValid'>bool</a> <a href='#SkPath_isValid'>isValid</a>() <a href='#SkPath_isValid'>const</a>;
    <a href='#SkPath_isValid'>bool</a> <a href='#SkPath_pathRefIsValid'>pathRefIsValid</a>() <a href='#SkPath_pathRefIsValid'>const</a>;
};
</pre>

<a href='SkPath_Reference#Path'>Paths</a> <a href='SkPath_Reference#Path'>contain</a> <a href='SkPath_Reference#Path'>geometry</a>. <a href='SkPath_Reference#Path'>Paths</a> <a href='SkPath_Reference#Path'>may</a> <a href='SkPath_Reference#Path'>be</a> <a href='SkPath_Reference#Path'>empty</a>, <a href='SkPath_Reference#Path'>or</a> <a href='SkPath_Reference#Path'>contain</a> <a href='SkPath_Reference#Path'>one</a> <a href='SkPath_Reference#Path'>or</a> <a href='SkPath_Reference#Path'>more</a> <a href='SkPath_Reference#Verb'>Verbs</a> <a href='SkPath_Reference#Verb'>that</a>
<a href='SkPath_Reference#Verb'>outline</a> <a href='SkPath_Reference#Verb'>a</a> <a href='SkPath_Reference#Verb'>figure</a>. <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>always</a> <a href='SkPath_Reference#Path'>starts</a> <a href='SkPath_Reference#Path'>with</a> <a href='SkPath_Reference#Path'>a</a> <a href='SkPath_Reference#Path'>move</a> <a href='SkPath_Reference#Path'>verb</a> <a href='SkPath_Reference#Path'>to</a> <a href='SkPath_Reference#Path'>a</a> <a href='#Cartesian_Coordinate'>Cartesian_Coordinate</a>,
<a href='#Cartesian_Coordinate'>and</a> <a href='#Cartesian_Coordinate'>may</a> <a href='#Cartesian_Coordinate'>be</a> <a href='#Cartesian_Coordinate'>followed</a> <a href='#Cartesian_Coordinate'>by</a> <a href='#Cartesian_Coordinate'>additional</a> <a href='SkPath_Reference#Verb'>verbs</a> <a href='SkPath_Reference#Verb'>that</a> <a href='SkPath_Reference#Verb'>add</a> <a href='undocumented#Line'>lines</a> <a href='undocumented#Line'>or</a> <a href='undocumented#Curve'>curves</a>.
<a href='undocumented#Curve'>Adding</a> <a href='undocumented#Curve'>a</a> <a href='undocumented#Curve'>close</a> <a href='undocumented#Curve'>verb</a> <a href='undocumented#Curve'>makes</a> <a href='undocumented#Curve'>the</a> <a href='undocumented#Curve'>geometry</a> <a href='undocumented#Curve'>into</a> <a href='undocumented#Curve'>a</a> <a href='undocumented#Curve'>continuous</a> <a href='undocumented#Curve'>loop</a>, <a href='undocumented#Curve'>a</a> <a href='undocumented#Curve'>closed</a> <a href='SkPath_Overview#Contour'>contour</a>.
<a href='SkPath_Reference#Path'>Paths</a> <a href='SkPath_Reference#Path'>may</a> <a href='SkPath_Reference#Path'>contain</a> <a href='SkPath_Reference#Path'>any</a> <a href='SkPath_Reference#Path'>number</a> <a href='SkPath_Reference#Path'>of</a> <a href='SkPath_Overview#Contour'>contours</a>, <a href='SkPath_Overview#Contour'>each</a> <a href='SkPath_Overview#Contour'>beginning</a> <a href='SkPath_Overview#Contour'>with</a> <a href='SkPath_Overview#Contour'>a</a> <a href='SkPath_Overview#Contour'>move</a> <a href='SkPath_Overview#Contour'>verb</a>.

<a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Overview#Contour'>contours</a> <a href='SkPath_Overview#Contour'>may</a> <a href='SkPath_Overview#Contour'>contain</a> <a href='SkPath_Overview#Contour'>only</a> <a href='SkPath_Overview#Contour'>a</a> <a href='SkPath_Overview#Contour'>move</a> <a href='SkPath_Overview#Contour'>verb</a>, <a href='SkPath_Overview#Contour'>or</a> <a href='SkPath_Overview#Contour'>may</a> <a href='SkPath_Overview#Contour'>also</a> <a href='SkPath_Overview#Contour'>contain</a> <a href='undocumented#Line'>lines</a>,
<a href='#Path_Quad'>Quadratic_Beziers</a>, <a href='SkPath_Reference#Conic'>Conics</a>, <a href='SkPath_Reference#Conic'>and</a> <a href='#Path_Cubic'>Cubic_Beziers</a>. <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Overview#Contour'>contours</a> <a href='SkPath_Overview#Contour'>may</a> <a href='SkPath_Overview#Contour'>be</a> <a href='SkPath_Overview#Contour'>open</a> <a href='SkPath_Overview#Contour'>or</a>
<a href='SkPath_Overview#Contour'>closed</a>.

<a href='SkPath_Overview#Contour'>When</a> <a href='SkPath_Overview#Contour'>used</a> <a href='SkPath_Overview#Contour'>to</a> <a href='SkPath_Overview#Contour'>draw</a> <a href='SkPath_Overview#Contour'>a</a> <a href='SkPath_Overview#Contour'>filled</a> <a href='SkPath_Overview#Contour'>area</a>, <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>describes</a> <a href='SkPath_Reference#Path'>whether</a> <a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Path'>fill</a> <a href='SkPath_Reference#Path'>is</a> <a href='SkPath_Reference#Path'>inside</a> <a href='SkPath_Reference#Path'>or</a>
<a href='SkPath_Reference#Path'>outside</a> <a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Path'>geometry</a>. <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>also</a> <a href='SkPath_Reference#Path'>describes</a> <a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Path'>winding</a> <a href='SkPath_Reference#Path'>rule</a> <a href='SkPath_Reference#Path'>used</a> <a href='SkPath_Reference#Path'>to</a> <a href='SkPath_Reference#Path'>fill</a>
<a href='SkPath_Reference#Path'>overlapping</a> <a href='SkPath_Overview#Contour'>contours</a>.

<a href='SkPath_Overview#Contour'>Internally</a>, <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>lazily</a> <a href='SkPath_Reference#Path'>computes</a> <a href='SkPath_Reference#Path'>metrics</a> <a href='SkPath_Reference#Path'>likes</a> <a href='SkPath_Reference#Path'>bounds</a> <a href='SkPath_Reference#Path'>and</a> <a href='SkPath_Reference#Path'>convexity</a>. <a href='SkPath_Reference#Path'>Call</a>
<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_updateBoundsCache'>updateBoundsCache</a> <a href='#SkPath_updateBoundsCache'>to</a> <a href='#SkPath_updateBoundsCache'>make</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>thread</a> <a href='SkPath_Reference#Path'>safe</a>.

<a name='Verb'></a>

<a name='SkPath_Verb'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPath_Verb'>Verb</a> {
        <a href='#SkPath_kMove_Verb'>kMove_Verb</a>,
        <a href='#SkPath_kLine_Verb'>kLine_Verb</a>,
        <a href='#SkPath_kQuad_Verb'>kQuad_Verb</a>,
        <a href='#SkPath_kConic_Verb'>kConic_Verb</a>,
        <a href='#SkPath_kCubic_Verb'>kCubic_Verb</a>,
        <a href='#SkPath_kClose_Verb'>kClose_Verb</a>,
        <a href='#SkPath_kDone_Verb'>kDone_Verb</a>,
    };
</pre>

<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>instructs</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>how</a> <a href='SkPath_Reference#Path'>to</a> <a href='SkPath_Reference#Path'>interpret</a> <a href='SkPath_Reference#Path'>one</a> <a href='SkPath_Reference#Path'>or</a> <a href='SkPath_Reference#Path'>more</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Point'>optional</a> <a href='#Path_Conic_Weight'>Conic_Weight</a>;
<a href='#Path_Conic_Weight'>manage</a> <a href='SkPath_Overview#Contour'>Contour</a>, <a href='SkPath_Overview#Contour'>and</a> <a href='SkPath_Overview#Contour'>terminate</a> <a href='SkPath_Reference#Path'>Path</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kMove_Verb'><code>SkPath::kMove_Verb</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Consecutive <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>are</a> <a href='#SkPath_kMove_Verb'>preserved</a> <a href='#SkPath_kMove_Verb'>but</a> <a href='#SkPath_kMove_Verb'>all</a> <a href='#SkPath_kMove_Verb'>but</a> <a href='#SkPath_kMove_Verb'>the</a> <a href='#SkPath_kMove_Verb'>last</a> <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>is</a>
<a href='#SkPath_kMove_Verb'>ignored</a>. <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>after</a> <a href='#SkPath_kMove_Verb'>other</a> <a href='SkPath_Reference#Verb'>Verbs</a> <a href='SkPath_Reference#Verb'>implicitly</a> <a href='SkPath_Reference#Verb'>closes</a> <a href='SkPath_Reference#Verb'>the</a> <a href='SkPath_Reference#Verb'>previous</a> <a href='SkPath_Overview#Contour'>Contour</a>
<a href='SkPath_Overview#Contour'>if</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kFill_Style'>kFill_Style</a> <a href='#SkPaint_kFill_Style'>is</a> <a href='#SkPaint_kFill_Style'>set</a> <a href='#SkPaint_kFill_Style'>when</a> <a href='#SkPaint_kFill_Style'>drawn</a>; <a href='#SkPaint_kFill_Style'>otherwise</a>, <a href='#SkPaint_kFill_Style'>stroke</a> <a href='#SkPaint_kFill_Style'>is</a> <a href='#SkPaint_kFill_Style'>drawn</a> <a href='#SkPaint_kFill_Style'>open</a>.
<a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>as</a> <a href='#SkPath_kMove_Verb'>the</a> <a href='#SkPath_kMove_Verb'>last</a> <a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>is</a> <a href='#SkPath_Verb'>preserved</a> <a href='#SkPath_Verb'>but</a> <a href='#SkPath_Verb'>ignored</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kLine_Verb'><code>SkPath::kLine_Verb</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='undocumented#Line'>Line</a> <a href='undocumented#Line'>is</a> <a href='undocumented#Line'>a</a> <a href='undocumented#Line'>straight</a> <a href='undocumented#Line'>segment</a> <a href='undocumented#Line'>from</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>Point</a>. <a href='SkPoint_Reference#Point'>Consecutive</a> <a href='#SkPath_kLine_Verb'>kLine_Verb</a>
<a href='#SkPath_kLine_Verb'>extend</a> <a href='SkPath_Overview#Contour'>Contour</a>. <a href='#SkPath_kLine_Verb'>kLine_Verb</a> <a href='#SkPath_kLine_Verb'>at</a> <a href='#SkPath_kLine_Verb'>same</a> <a href='#SkPath_kLine_Verb'>position</a> <a href='#SkPath_kLine_Verb'>as</a> <a href='#SkPath_kLine_Verb'>prior</a> <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>is</a>
<a href='#SkPath_kMove_Verb'>preserved</a>, <a href='#SkPath_kMove_Verb'>and</a> <a href='#SkPath_kMove_Verb'>draws</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>if</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kStroke_Style'>kStroke_Style</a> <a href='#SkPaint_kStroke_Style'>is</a> <a href='#SkPaint_kStroke_Style'>set</a>, <a href='#SkPaint_kStroke_Style'>and</a>
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Cap'>Cap</a> <a href='#SkPaint_Cap'>is</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kSquare_Cap'>kSquare_Cap</a> <a href='#SkPaint_kSquare_Cap'>or</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kRound_Cap'>kRound_Cap</a>. <a href='#SkPath_kLine_Verb'>kLine_Verb</a>
<a href='#SkPath_kLine_Verb'>at</a> <a href='#SkPath_kLine_Verb'>same</a> <a href='#SkPath_kLine_Verb'>position</a> <a href='#SkPath_kLine_Verb'>as</a> <a href='#SkPath_kLine_Verb'>prior</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>or</a> <a href='undocumented#Curve'>curve</a> <a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>is</a> <a href='#SkPath_Verb'>preserved</a> <a href='#SkPath_Verb'>but</a> <a href='#SkPath_Verb'>is</a> <a href='#SkPath_Verb'>ignored</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kQuad_Verb'><code>SkPath::kQuad_Verb</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Adds <a href='SkPath_Reference#Quad'>Quad</a> <a href='SkPath_Reference#Quad'>from</a> <a href='#Path_Last_Point'>Last_Point</a>, <a href='#Path_Last_Point'>using</a> <a href='#Path_Last_Point'>control</a> <a href='SkPoint_Reference#Point'>Point</a>, <a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Point'>end</a> <a href='SkPoint_Reference#Point'>Point</a>.
<a href='SkPath_Reference#Quad'>Quad</a> <a href='SkPath_Reference#Quad'>is</a> <a href='SkPath_Reference#Quad'>a</a> <a href='SkPath_Reference#Quad'>parabolic</a> <a href='SkPath_Reference#Quad'>section</a> <a href='SkPath_Reference#Quad'>within</a> <a href='SkPath_Reference#Quad'>tangents</a> <a href='SkPath_Reference#Quad'>from</a> <a href='#Path_Last_Point'>Last_Point</a> <a href='#Path_Last_Point'>to</a> <a href='#Path_Last_Point'>control</a> <a href='SkPoint_Reference#Point'>Point</a>,
<a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Point'>control</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>end</a> <a href='SkPoint_Reference#Point'>Point</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kConic_Verb'><code>SkPath::kConic_Verb</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Adds <a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>from</a> <a href='#Path_Last_Point'>Last_Point</a>, <a href='#Path_Last_Point'>using</a> <a href='#Path_Last_Point'>control</a> <a href='SkPoint_Reference#Point'>Point</a>, <a href='SkPoint_Reference#Point'>end</a> <a href='SkPoint_Reference#Point'>Point</a>, <a href='SkPoint_Reference#Point'>and</a> <a href='#Path_Conic_Weight'>Conic_Weight</a>.
<a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>is</a> <a href='SkPath_Reference#Conic'>a</a> <a href='SkPath_Reference#Conic'>elliptical</a>, <a href='SkPath_Reference#Conic'>parabolic</a>, <a href='SkPath_Reference#Conic'>or</a> <a href='SkPath_Reference#Conic'>hyperbolic</a> <a href='SkPath_Reference#Conic'>section</a> <a href='SkPath_Reference#Conic'>within</a> <a href='SkPath_Reference#Conic'>tangents</a>
<a href='SkPath_Reference#Conic'>from</a> <a href='#Path_Last_Point'>Last_Point</a> <a href='#Path_Last_Point'>to</a> <a href='#Path_Last_Point'>control</a> <a href='SkPoint_Reference#Point'>Point</a>, <a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Point'>control</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>end</a> <a href='SkPoint_Reference#Point'>Point</a>, <a href='SkPoint_Reference#Point'>constrained</a>
<a href='SkPoint_Reference#Point'>by</a> <a href='#Path_Conic_Weight'>Conic_Weight</a>. <a href='#Path_Conic_Weight'>Conic_Weight</a> <a href='#Path_Conic_Weight'>less</a> <a href='#Path_Conic_Weight'>than</a> <a href='#Path_Conic_Weight'>one</a> <a href='#Path_Conic_Weight'>is</a> <a href='#Path_Conic_Weight'>elliptical</a>; <a href='#Path_Conic_Weight'>equal</a> <a href='#Path_Conic_Weight'>to</a> <a href='#Path_Conic_Weight'>one</a> <a href='#Path_Conic_Weight'>is</a>
<a href='#Path_Conic_Weight'>parabolic</a> (<a href='#Path_Conic_Weight'>and</a> <a href='#Path_Conic_Weight'>identical</a> <a href='#Path_Conic_Weight'>to</a> <a href='SkPath_Reference#Quad'>Quad</a>); <a href='SkPath_Reference#Quad'>greater</a> <a href='SkPath_Reference#Quad'>than</a> <a href='SkPath_Reference#Quad'>one</a> <a href='SkPath_Reference#Quad'>hyperbolic</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kCubic_Verb'><code>SkPath::kCubic_Verb</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Adds <a href='SkPath_Reference#Cubic'>Cubic</a> <a href='SkPath_Reference#Cubic'>from</a> <a href='#Path_Last_Point'>Last_Point</a>, <a href='#Path_Last_Point'>using</a> <a href='#Path_Last_Point'>two</a> <a href='#Path_Last_Point'>control</a> <a href='SkPoint_Reference#Point'>Points</a>, <a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Point'>end</a> <a href='SkPoint_Reference#Point'>Point</a>.
<a href='SkPath_Reference#Cubic'>Cubic</a> <a href='SkPath_Reference#Cubic'>is</a> <a href='SkPath_Reference#Cubic'>a</a> <a href='SkPath_Reference#Cubic'>third-order</a> <a href='#Bezier_Curve'>Bezier_Curve</a> <a href='#Bezier_Curve'>section</a> <a href='#Bezier_Curve'>within</a> <a href='#Bezier_Curve'>tangents</a> <a href='#Bezier_Curve'>from</a> <a href='#Path_Last_Point'>Last_Point</a>
<a href='#Path_Last_Point'>to</a> <a href='#Path_Last_Point'>first</a> <a href='#Path_Last_Point'>control</a> <a href='SkPoint_Reference#Point'>Point</a>, <a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Point'>from</a> <a href='SkPoint_Reference#Point'>second</a> <a href='SkPoint_Reference#Point'>control</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>end</a> <a href='SkPoint_Reference#Point'>Point</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kClose_Verb'><code>SkPath::kClose_Verb</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>5</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Closes <a href='SkPath_Overview#Contour'>Contour</a>, <a href='SkPath_Overview#Contour'>connecting</a> <a href='#Path_Last_Point'>Last_Point</a> <a href='#Path_Last_Point'>to</a> <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='SkPoint_Reference#Point'>Point</a>. <a href='SkPoint_Reference#Point'>Consecutive</a>
<a href='#SkPath_kClose_Verb'>kClose_Verb</a> <a href='#SkPath_kClose_Verb'>are</a> <a href='#SkPath_kClose_Verb'>preserved</a> <a href='#SkPath_kClose_Verb'>but</a> <a href='#SkPath_kClose_Verb'>only</a> <a href='#SkPath_kClose_Verb'>first</a> <a href='#SkPath_kClose_Verb'>has</a> <a href='#SkPath_kClose_Verb'>an</a> <a href='#SkPath_kClose_Verb'>effect</a>. <a href='#SkPath_kClose_Verb'>kClose_Verb</a> <a href='#SkPath_kClose_Verb'>after</a>
<a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>has</a> <a href='#SkPath_kMove_Verb'>no</a> <a href='#SkPath_kMove_Verb'>effect</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kDone_Verb'><code>SkPath::kDone_Verb</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>6</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Not in <a href='#Path_Verb_Array'>Verb_Array</a>, <a href='#Path_Verb_Array'>but</a> <a href='#Path_Verb_Array'>returned</a> <a href='#Path_Verb_Array'>by</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>iterator</a>.
</td>
  </tr>
Each <a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>has</a> <a href='#SkPath_Verb'>zero</a> <a href='#SkPath_Verb'>or</a> <a href='#SkPath_Verb'>more</a> <a href='SkPoint_Reference#Point'>Points</a> <a href='SkPoint_Reference#Point'>stored</a> <a href='SkPoint_Reference#Point'>in</a> <a href='SkPath_Reference#Path'>Path</a>.
<a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>iterator</a> <a href='SkPath_Reference#Path'>returns</a> <a href='SkPath_Reference#Path'>complete</a> <a href='undocumented#Curve'>curve</a> <a href='undocumented#Curve'>descriptions</a>, <a href='undocumented#Curve'>duplicating</a> <a href='undocumented#Curve'>shared</a> <a href='SkPoint_Reference#Point'>Points</a>
<a href='SkPoint_Reference#Point'>for</a> <a href='SkPoint_Reference#Point'>consecutive</a> <a href='SkPoint_Reference#Point'>entries</a>.

</table>

| <a href='#SkPath_Verb'>Verb</a> | Allocated <a href='SkPoint_Reference#Point'>Points</a> | Iterated <a href='SkPoint_Reference#Point'>Points</a> | <a href='SkPath_Reference#Conic_Weight'>Weights</a> |
| --- | --- | --- | ---  |
| <a href='#SkPath_kMove_Verb'>kMove_Verb</a> | 1 | 1 | 0 |
| <a href='#SkPath_kLine_Verb'>kLine_Verb</a> | 1 | 2 | 0 |
| <a href='#SkPath_kQuad_Verb'>kQuad_Verb</a> | 2 | 3 | 0 |
| <a href='#SkPath_kConic_Verb'>kConic_Verb</a> | 2 | 3 | 1 |
| <a href='#SkPath_kCubic_Verb'>kCubic_Verb</a> | 3 | 4 | 0 |
| <a href='#SkPath_kClose_Verb'>kClose_Verb</a> | 0 | 1 | 0 |
| <a href='#SkPath_kDone_Verb'>kDone_Verb</a> | -- | 0 | 0 |

### Example

<div><fiddle-embed name="799096fdc1298aa815934a74e76570ca">

#### Example Output

~~~~
verb count: 7
verbs: kMove_Verb kLine_Verb kQuad_Verb kClose_Verb kMove_Verb kCubic_Verb kConic_Verb
~~~~

</fiddle-embed></div>

<a name='Direction'></a>

<a name='SkPath_Direction'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPath_Direction'>Direction</a> : <a href='#SkPath_Direction'>int</a> {
        <a href='#SkPath_kCW_Direction'>kCW_Direction</a>,
        <a href='#SkPath_kCCW_Direction'>kCCW_Direction</a>,
    };
</pre>

<a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>describes</a> <a href='#SkPath_Direction'>whether</a> <a href='SkPath_Overview#Contour'>Contour</a> <a href='SkPath_Overview#Contour'>is</a> <a href='SkPath_Overview#Contour'>clockwise</a> <a href='SkPath_Overview#Contour'>or</a> <a href='SkPath_Overview#Contour'>counterclockwise</a>.
<a href='SkPath_Overview#Contour'>When</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>contains</a> <a href='SkPath_Reference#Path'>multiple</a> <a href='SkPath_Reference#Path'>overlapping</a> <a href='SkPath_Overview#Contour'>Contours</a>, <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>together</a> <a href='#SkPath_Direction'>with</a>
<a href='#Path_Fill_Type'>Fill_Type</a> <a href='#Path_Fill_Type'>determines</a> <a href='#Path_Fill_Type'>whether</a> <a href='#Path_Fill_Type'>overlaps</a> <a href='#Path_Fill_Type'>are</a> <a href='#Path_Fill_Type'>filled</a> <a href='#Path_Fill_Type'>or</a> <a href='#Path_Fill_Type'>form</a> <a href='#Path_Fill_Type'>holes</a>.

<a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>also</a> <a href='#SkPath_Direction'>determines</a> <a href='#SkPath_Direction'>how</a> <a href='SkPath_Overview#Contour'>Contour</a> <a href='SkPath_Overview#Contour'>is</a> <a href='SkPath_Overview#Contour'>measured</a>. <a href='SkPath_Overview#Contour'>For</a> <a href='SkPath_Overview#Contour'>instance</a>, <a href='SkPath_Overview#Contour'>dashing</a>
<a href='SkPath_Overview#Contour'>measures</a> <a href='SkPath_Overview#Contour'>along</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>to</a> <a href='SkPath_Reference#Path'>determine</a> <a href='SkPath_Reference#Path'>where</a> <a href='SkPath_Reference#Path'>to</a> <a href='SkPath_Reference#Path'>start</a> <a href='SkPath_Reference#Path'>and</a> <a href='SkPath_Reference#Path'>stop</a> <a href='SkPath_Reference#Path'>stroke</a>; <a href='#SkPath_Direction'>Direction</a>
<a href='#SkPath_Direction'>will</a> <a href='#SkPath_Direction'>change</a> <a href='#SkPath_Direction'>dashed</a> <a href='#SkPath_Direction'>results</a> <a href='#SkPath_Direction'>as</a> <a href='#SkPath_Direction'>it</a> <a href='#SkPath_Direction'>steps</a> <a href='#SkPath_Direction'>clockwise</a> <a href='#SkPath_Direction'>or</a> <a href='#SkPath_Direction'>counterclockwise</a>.

<a href='#SkPath_Direction'>Closed</a> <a href='SkPath_Overview#Contour'>Contours</a> <a href='SkPath_Overview#Contour'>like</a> <a href='SkRect_Reference#Rect'>Rect</a>, <a href='#RRect'>Round_Rect</a>, <a href='undocumented#Circle'>Circle</a>, <a href='undocumented#Circle'>and</a> <a href='undocumented#Oval'>Oval</a> <a href='undocumented#Oval'>added</a> <a href='undocumented#Oval'>with</a>
<a href='#SkPath_kCW_Direction'>kCW_Direction</a> <a href='#SkPath_kCW_Direction'>travel</a> <a href='#SkPath_kCW_Direction'>clockwise</a>; <a href='#SkPath_kCW_Direction'>the</a> <a href='#SkPath_kCW_Direction'>same</a> <a href='#SkPath_kCW_Direction'>added</a> <a href='#SkPath_kCW_Direction'>with</a> <a href='#SkPath_kCCW_Direction'>kCCW_Direction</a>
<a href='#SkPath_kCCW_Direction'>travel</a> <a href='#SkPath_kCCW_Direction'>counterclockwise</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kCW_Direction'><code>SkPath::kCW_Direction</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
contour travels clockwise</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kCCW_Direction'><code>SkPath::kCCW_Direction</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
contour travels counterclockwise</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4bbae00b40ed2cfcd0007921ad693a7b"></fiddle-embed></div>

### See Also

<a href='#SkPath_arcTo'>arcTo</a> <a href='#SkPath_rArcTo'>rArcTo</a> <a href='#SkPath_isRect'>isRect</a> <a href='#SkPath_isNestedFillRects'>isNestedFillRects</a> <a href='#SkPath_addRect'>addRect</a> <a href='#SkPath_addOval'>addOval</a>

<a name='SkPath_empty_constructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_empty_constructor'>SkPath()</a>
</pre>

Constructs an empty <a href='SkPath_Reference#SkPath'>SkPath</a>. <a href='SkPath_Reference#SkPath'>By</a> <a href='SkPath_Reference#SkPath'>default</a>, <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>has</a> <a href='SkPath_Reference#SkPath'>no</a> <a href='SkPath_Reference#Verb'>verbs</a>, <a href='SkPath_Reference#Verb'>no</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>, <a href='SkPoint_Reference#SkPoint'>and</a> <a href='SkPoint_Reference#SkPoint'>no</a> <a href='SkPath_Reference#Conic_Weight'>weights</a>.
<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_FillType'>is</a> <a href='#SkPath_FillType'>set</a> <a href='#SkPath_FillType'>to</a> <a href='#SkPath_kWinding_FillType'>kWinding_FillType</a>.

### Return Value

empty <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="0a0026fca638d1cd75c0ab884e3ee1c6">

#### Example Output

~~~~
path is empty
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_reset'>reset</a> <a href='#SkPath_rewind'>rewind</a>

<a name='SkPath_copy_const_SkPath'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>(<a href='SkPath_Reference#SkPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>)
</pre>

Constructs a copy of an existing <a href='#SkPath_copy_const_SkPath_path'>path</a>.
Copy constructor makes two <a href='SkPath_Reference#Path'>paths</a> <a href='SkPath_Reference#Path'>identical</a> <a href='SkPath_Reference#Path'>by</a> <a href='SkPath_Reference#Path'>value</a>. <a href='SkPath_Reference#Path'>Internally</a>, <a href='#SkPath_copy_const_SkPath_path'>path</a> <a href='#SkPath_copy_const_SkPath_path'>and</a>
the returned result share pointer values. The underlying  <a href='#Verb_Array'>verb array</a>,  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>
and <a href='SkPath_Reference#Conic_Weight'>weights</a> <a href='SkPath_Reference#Conic_Weight'>are</a> <a href='SkPath_Reference#Conic_Weight'>copied</a> <a href='SkPath_Reference#Conic_Weight'>when</a> <a href='SkPath_Reference#Conic_Weight'>modified</a>.

Creating a <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>copy</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>very</a> <a href='SkPath_Reference#SkPath'>efficient</a> <a href='SkPath_Reference#SkPath'>and</a> <a href='SkPath_Reference#SkPath'>never</a> <a href='SkPath_Reference#SkPath'>allocates</a> <a href='SkPath_Reference#SkPath'>memory</a>.
<a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>are</a> <a href='SkPath_Reference#SkPath'>always</a> <a href='SkPath_Reference#SkPath'>copied</a> <a href='SkPath_Reference#SkPath'>by</a> <a href='SkPath_Reference#SkPath'>value</a> <a href='SkPath_Reference#SkPath'>from</a> <a href='SkPath_Reference#SkPath'>the</a> <a href='SkPath_Reference#SkPath'>interface</a>; <a href='SkPath_Reference#SkPath'>the</a> <a href='SkPath_Reference#SkPath'>underlying</a> <a href='SkPath_Reference#SkPath'>shared</a>
pointers are not exposed.

### Parameters

<table>  <tr>    <td><a name='SkPath_copy_const_SkPath_path'><code><strong>path</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>copy</a> <a href='SkPath_Reference#SkPath'>by</a> <a href='SkPath_Reference#SkPath'>value</a></td>
  </tr>
</table>

### Return Value

copy of <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="647312aacd946c8a6eabaca797140432"><div>Modifying one <a href='#SkPath_copy_const_SkPath_path'>path</a> <a href='#SkPath_copy_const_SkPath_path'>does</a> <a href='#SkPath_copy_const_SkPath_path'>not</a> <a href='#SkPath_copy_const_SkPath_path'>effect</a> <a href='#SkPath_copy_const_SkPath_path'>another</a>, <a href='#SkPath_copy_const_SkPath_path'>even</a> <a href='#SkPath_copy_const_SkPath_path'>if</a> <a href='#SkPath_copy_const_SkPath_path'>they</a> <a href='#SkPath_copy_const_SkPath_path'>started</a> <a href='#SkPath_copy_const_SkPath_path'>as</a> <a href='#SkPath_copy_const_SkPath_path'>copies</a>
<a href='#SkPath_copy_const_SkPath_path'>of</a> <a href='#SkPath_copy_const_SkPath_path'>each</a> <a href='#SkPath_copy_const_SkPath_path'>other</a>.
</div>

#### Example Output

~~~~
path verbs: 2
path2 verbs: 3
after reset
path verbs: 0
path2 verbs: 3
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_copy_operator'>operator=(const SkPath& path)</a>

<a name='SkPath_destructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
~<a href='#SkPath_empty_constructor'>SkPath()</a>
</pre>

Releases ownership of any shared <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>and</a> <a href='undocumented#Data'>deletes</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>if</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>sole</a> <a href='SkPath_Reference#SkPath'>owner</a>.

### Example

<div><fiddle-embed name="01ad6be9b7d15a2217daea273eb3d466"><div>delete calls <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>destructor</a>, <a href='SkPath_Reference#Path'>but</a> <a href='SkPath_Reference#Path'>copy</a> <a href='SkPath_Reference#Path'>of</a> <a href='SkPath_Reference#Path'>original</a> <a href='SkPath_Reference#Path'>in</a> <a href='SkPath_Reference#Path'>path2</a> <a href='SkPath_Reference#Path'>is</a> <a href='SkPath_Reference#Path'>unaffected</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_empty_constructor'>SkPath()</a> SkPath()<a href='#SkPath_copy_const_SkPath'>SkPath(const SkPath& path)</a> SkPath(const SkPath& path)<a href='#SkPath_copy_operator'>operator=(const SkPath& path)</a>

<a name='SkPath_copy_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>operator</a>=(<a href='SkPath_Reference#SkPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>)
</pre>

Constructs a copy of an existing <a href='#SkPath_copy_operator_path'>path</a>.
<a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>assignment</a> <a href='SkPath_Reference#SkPath'>makes</a> <a href='SkPath_Reference#SkPath'>two</a> <a href='SkPath_Reference#Path'>paths</a> <a href='SkPath_Reference#Path'>identical</a> <a href='SkPath_Reference#Path'>by</a> <a href='SkPath_Reference#Path'>value</a>. <a href='SkPath_Reference#Path'>Internally</a>, <a href='SkPath_Reference#Path'>assignment</a>
shares pointer values. The underlying  <a href='#Verb_Array'>verb array</a>,  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>and</a> <a href='SkPath_Reference#Conic_Weight'>weights</a>
are copied when modified.

Copying <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>by</a> <a href='SkPath_Reference#SkPath'>assignment</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>very</a> <a href='SkPath_Reference#SkPath'>efficient</a> <a href='SkPath_Reference#SkPath'>and</a> <a href='SkPath_Reference#SkPath'>never</a> <a href='SkPath_Reference#SkPath'>allocates</a> <a href='SkPath_Reference#SkPath'>memory</a>.
<a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>are</a> <a href='SkPath_Reference#SkPath'>always</a> <a href='SkPath_Reference#SkPath'>copied</a> <a href='SkPath_Reference#SkPath'>by</a> <a href='SkPath_Reference#SkPath'>value</a> <a href='SkPath_Reference#SkPath'>from</a> <a href='SkPath_Reference#SkPath'>the</a> <a href='SkPath_Reference#SkPath'>interface</a>; <a href='SkPath_Reference#SkPath'>the</a> <a href='SkPath_Reference#SkPath'>underlying</a> <a href='SkPath_Reference#SkPath'>shared</a>
pointers are not exposed.

### Parameters

<table>  <tr>    <td><a name='SkPath_copy_operator_path'><code><strong>path</strong></code></a></td>
    <td><a href='#Verb_Array'>verb array</a>,  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>, <a href='SkPath_Reference#Conic_Weight'>weights</a>, <a href='SkPath_Reference#Conic_Weight'>and</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_FillType'>to</a> <a href='#SkPath_FillType'>copy</a></td>
  </tr>
</table>

### Return Value

<a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>copied</a> <a href='SkPath_Reference#SkPath'>by</a> <a href='SkPath_Reference#SkPath'>value</a>

### Example

<div><fiddle-embed name="bba288f5f77fc8e37e89d2ec08e0ac60">

#### Example Output

~~~~
path1 bounds = 10, 20, 30, 40
path2 bounds = 10, 20, 30, 40
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_swap'>swap</a> swap<a href='#SkPath_copy_const_SkPath'>SkPath(const SkPath& path)</a>

<a name='SkPath_equal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator==(const <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>a</a>, <a href='SkPath_Reference#SkPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>b</a>)
</pre>

Compares <a href='#SkPath_equal_operator_a'>a</a> <a href='#SkPath_equal_operator_a'>and</a> <a href='#SkPath_equal_operator_b'>b</a>; <a href='#SkPath_equal_operator_b'>returns</a> <a href='#SkPath_equal_operator_b'>true</a> <a href='#SkPath_equal_operator_b'>if</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_FillType'>FillType</a>,  <a href='#Verb_Array'>verb array</a>,  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>, <a href='SkPoint_Reference#SkPoint'>and</a> <a href='SkPath_Reference#Conic_Weight'>weights</a>
are equivalent.

### Parameters

<table>  <tr>    <td><a name='SkPath_equal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>compare</a></td>
  </tr>
  <tr>    <td><a name='SkPath_equal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>compare</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>pair</a> <a href='SkPath_Reference#SkPath'>are</a> <a href='SkPath_Reference#SkPath'>equivalent</a>

### Example

<div><fiddle-embed name="31883f51bb357f2ac5990d88f8b82e02"><div><a href='#SkPath_rewind'>rewind()</a> <a href='#SkPath_rewind'>removes</a> <a href='#Path_Verb_Array'>Verb_Array</a> <a href='#Path_Verb_Array'>but</a> <a href='#Path_Verb_Array'>leaves</a> <a href='#Path_Verb_Array'>storage</a>; <a href='#Path_Verb_Array'>since</a> <a href='#Path_Verb_Array'>storage</a> <a href='#Path_Verb_Array'>is</a> <a href='#Path_Verb_Array'>not</a> <a href='#Path_Verb_Array'>compared</a>,
<a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>pair</a> <a href='SkPath_Reference#Path'>are</a> <a href='SkPath_Reference#Path'>equivalent</a>.
</div>

#### Example Output

~~~~
empty one == two
moveTo one != two
rewind one == two
reset one == two
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_notequal_operator'>operator!=(const SkPath& a, const SkPath& b)</a> operator!=(const SkPath& a, const SkPath& b)<a href='#SkPath_copy_operator'>operator=(const SkPath& path)</a>

<a name='SkPath_notequal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator!=(const <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>a</a>, <a href='SkPath_Reference#SkPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>b</a>)
</pre>

Compares <a href='#SkPath_notequal_operator_a'>a</a> <a href='#SkPath_notequal_operator_a'>and</a> <a href='#SkPath_notequal_operator_b'>b</a>; <a href='#SkPath_notequal_operator_b'>returns</a> <a href='#SkPath_notequal_operator_b'>true</a> <a href='#SkPath_notequal_operator_b'>if</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_FillType'>FillType</a>,  <a href='#Verb_Array'>verb array</a>,  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>, <a href='SkPoint_Reference#SkPoint'>and</a> <a href='SkPath_Reference#Conic_Weight'>weights</a>
are not equivalent.

### Parameters

<table>  <tr>    <td><a name='SkPath_notequal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>compare</a></td>
  </tr>
  <tr>    <td><a name='SkPath_notequal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>compare</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>pair</a> <a href='SkPath_Reference#SkPath'>are</a> <a href='SkPath_Reference#SkPath'>not</a> <a href='SkPath_Reference#SkPath'>equivalent</a>

### Example

<div><fiddle-embed name="bbbda1cc818d96c9c0d2a06c0c48902b"><div><a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>pair</a> <a href='SkPath_Reference#Path'>are</a> <a href='SkPath_Reference#Path'>equal</a> <a href='SkPath_Reference#Path'>though</a> <a href='SkPath_Reference#Path'>their</a> <a href='SkPath_Reference#Path'>convexity</a> <a href='SkPath_Reference#Path'>is</a> <a href='SkPath_Reference#Path'>not</a> <a href='SkPath_Reference#Path'>equal</a>.
</div>

#### Example Output

~~~~
empty one == two
add rect one == two
setConvexity one == two
convexity !=
~~~~

</fiddle-embed></div>

<a name='Property'></a>

<a name='SkPath_isInterpolatable'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isInterpolatable'>isInterpolatable</a>(<a href='#SkPath_isInterpolatable'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>compare</a>) <a href='SkPath_Reference#SkPath'>const</a>
</pre>

Returns true if <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>contain</a> <a href='SkPath_Reference#SkPath'>equal</a> <a href='SkPath_Reference#Verb'>verbs</a> <a href='SkPath_Reference#Verb'>and</a> <a href='SkPath_Reference#Verb'>equal</a> <a href='SkPath_Reference#Conic_Weight'>weights</a>.
If <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>contain</a> <a href='SkPath_Reference#SkPath'>one</a> <a href='SkPath_Reference#SkPath'>or</a> <a href='SkPath_Reference#SkPath'>more</a> <a href='SkPath_Reference#Conic'>conics</a>, <a href='SkPath_Reference#Conic'>the</a> <a href='SkPath_Reference#Conic_Weight'>weights</a> <a href='SkPath_Reference#Conic_Weight'>must</a> <a href='SkPath_Reference#Conic_Weight'>match</a>.

<a href='#SkPath_conicTo'>conicTo</a>() <a href='#SkPath_conicTo'>may</a> <a href='#SkPath_conicTo'>add</a> <a href='#SkPath_conicTo'>different</a> <a href='SkPath_Reference#Verb'>verbs</a> <a href='SkPath_Reference#Verb'>depending</a> <a href='SkPath_Reference#Verb'>on</a>  <a href='#Conic_Weight'>conic weight</a>, <a href='SkPath_Reference#Conic'>so</a> <a href='SkPath_Reference#Conic'>it</a> <a href='SkPath_Reference#Conic'>is</a> <a href='SkPath_Reference#Conic'>not</a>
trivial to interpolate a pair of <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>containing</a> <a href='SkPath_Reference#Conic'>conics</a> <a href='SkPath_Reference#Conic'>with</a> <a href='SkPath_Reference#Conic'>different</a>
<a href='#Conic_Weight'>conic weight</a> <a href='SkPath_Reference#Conic'>values</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_isInterpolatable_compare'><code><strong>compare</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='#SkPath_isInterpolatable_compare'>compare</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkPath_Reference#SkPath'>SkPath</a>  <a href='#Verb_Array'>verb array</a> <a href='SkPath_Reference#SkPath'>and</a> <a href='SkPath_Reference#Conic_Weight'>weights</a> <a href='SkPath_Reference#Conic_Weight'>are</a> <a href='SkPath_Reference#Conic_Weight'>equivalent</a>

### Example

<div><fiddle-embed name="c81fc7dfaf785c3fb77209c7f2ebe5b8">

#### Example Output

~~~~
paths are interpolatable
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_isInterpolatable'>isInterpolatable</a>

<a name='Interpolate'></a>

<a name='SkPath_interpolate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool interpolate(const <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>ending</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>weight</a>, <a href='SkPath_Reference#SkPath'>SkPath</a>* <a href='SkPath_Reference#SkPath'>out</a>) <a href='SkPath_Reference#SkPath'>const</a>
</pre>

Interpolates between <a href='SkPath_Reference#Path'>Paths</a> <a href='SkPath_Reference#Path'>with</a> <a href='#Path_Point_Array'>Point_Array</a> <a href='#Path_Point_Array'>of</a> <a href='#Path_Point_Array'>equal</a> <a href='undocumented#Size'>size</a>.
<a href='undocumented#Size'>Copy</a> <a href='#Path_Verb_Array'>Verb_Array</a> <a href='#Path_Verb_Array'>and</a> <a href='SkPath_Reference#Conic_Weight'>Weights</a> <a href='SkPath_Reference#Conic_Weight'>to</a> <a href='#SkPath_interpolate_out'>out</a>, <a href='#SkPath_interpolate_out'>and</a> <a href='#SkPath_interpolate_out'>set</a> <a href='#SkPath_interpolate_out'>out</a> <a href='#Path_Point_Array'>Point_Array</a> <a href='#Path_Point_Array'>to</a> <a href='#Path_Point_Array'>a</a> <a href='#Path_Point_Array'>weighted</a>
<a href='#Path_Point_Array'>average</a> <a href='#Path_Point_Array'>of</a> <a href='#Path_Point_Array'>this</a> <a href='#Path_Point_Array'>Point_Array</a> <a href='#Path_Point_Array'>and</a> <a href='#SkPath_interpolate_ending'>ending</a> <a href='#Path_Point_Array'>Point_Array</a>, <a href='#Path_Point_Array'>using</a> <a href='#Path_Point_Array'>the</a> <a href='#Path_Point_Array'>formula</a>:
<code>(<a href='SkPath_Reference#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> * <a href='#SkPath_interpolate_weight'>weight</a>) + <a href='#SkPath_interpolate_ending'>ending</a> <a href='SkPoint_Reference#Point'>Point</a> * (1 - <a href='#SkPath_interpolate_weight'>weight</a>)</code>.

<a href='#SkPath_interpolate_weight'>weight</a> <a href='#SkPath_interpolate_weight'>is</a> <a href='#SkPath_interpolate_weight'>most</a> <a href='#SkPath_interpolate_weight'>useful</a> <a href='#SkPath_interpolate_weight'>when</a> <a href='#SkPath_interpolate_weight'>between</a> <a href='#SkPath_interpolate_weight'>zero</a> (<a href='#SkPath_interpolate_ending'>ending</a> <a href='#Path_Point_Array'>Point_Array</a>) <a href='#Path_Point_Array'>and</a>
<a href='#Path_Point_Array'>one</a> (<a href='#Path_Point_Array'>this</a> <a href='#Path_Point_Array'>Point_Array</a>); <a href='#Path_Point_Array'>will</a> <a href='#Path_Point_Array'>work</a> <a href='#Path_Point_Array'>with</a> <a href='#Path_Point_Array'>values</a> <a href='#Path_Point_Array'>outside</a> <a href='#Path_Point_Array'>of</a> <a href='#Path_Point_Array'>this</a>
<a href='#Path_Point_Array'>range</a>.

<a href='#SkPath_interpolate'>interpolate()</a> <a href='#SkPath_interpolate'>returns</a> <a href='#SkPath_interpolate'>false</a> <a href='#SkPath_interpolate'>and</a> <a href='#SkPath_interpolate'>leaves</a> <a href='#SkPath_interpolate_out'>out</a> <a href='#SkPath_interpolate_out'>unchanged</a> <a href='#SkPath_interpolate_out'>if</a> <a href='#Path_Point_Array'>Point_Array</a> <a href='#Path_Point_Array'>is</a> <a href='#Path_Point_Array'>not</a>
<a href='#Path_Point_Array'>the</a> <a href='#Path_Point_Array'>same</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>as</a> <a href='#SkPath_interpolate_ending'>ending</a> <a href='#Path_Point_Array'>Point_Array</a>. <a href='#Path_Point_Array'>Call</a> <a href='#SkPath_isInterpolatable'>isInterpolatable</a> <a href='#SkPath_isInterpolatable'>to</a> <a href='#SkPath_isInterpolatable'>check</a> <a href='SkPath_Reference#Path'>Path</a>
<a href='SkPath_Reference#Path'>compatibility</a> <a href='SkPath_Reference#Path'>prior</a> <a href='SkPath_Reference#Path'>to</a> <a href='SkPath_Reference#Path'>calling</a> <a href='#SkPath_interpolate'>interpolate()</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_interpolate_ending'><code><strong>ending</strong></code></a></td>
    <td><a href='#Path_Point_Array'>Point_Array</a> <a href='#Path_Point_Array'>averaged</a> <a href='#Path_Point_Array'>with</a> <a href='#Path_Point_Array'>this</a> <a href='#Path_Point_Array'>Point_Array</a></td>
  </tr>
  <tr>    <td><a name='SkPath_interpolate_weight'><code><strong>weight</strong></code></a></td>
    <td>contribution of this <a href='#Path_Point_Array'>Point_Array</a>, <a href='#Path_Point_Array'>and</a>
<a href='#Path_Point_Array'>one</a> <a href='#Path_Point_Array'>minus</a> <a href='#Path_Point_Array'>contribution</a> <a href='#Path_Point_Array'>of</a> <a href='#SkPath_interpolate_ending'>ending</a> <a href='#Path_Point_Array'>Point_Array</a>
</td>
  </tr>
  <tr>    <td><a name='SkPath_interpolate_out'><code><strong>out</strong></code></a></td>
    <td><a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>replaced</a> <a href='SkPath_Reference#Path'>by</a> <a href='SkPath_Reference#Path'>interpolated</a> <a href='SkPath_Reference#Path'>averages</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkPath_Reference#Path'>Paths</a> <a href='SkPath_Reference#Path'>contain</a> <a href='SkPath_Reference#Path'>same</a> <a href='SkPath_Reference#Path'>number</a> <a href='SkPath_Reference#Path'>of</a> <a href='SkPoint_Reference#Point'>Points</a>

### Example

<div><fiddle-embed name="404f11c5c9c9ca8a64822d484552a473"></fiddle-embed></div>

### See Also

<a href='#SkPath_isInterpolatable'>isInterpolatable</a>

<a name='SkPath_unique'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_unique'>unique()</a> <a href='#SkPath_unique'>const</a>
</pre>

To be deprecated soon.

Only valid for Android framework.

<a name='Fill_Type'></a>

<a name='SkPath_FillType'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPath_FillType'>FillType</a> {
        <a href='#SkPath_kWinding_FillType'>kWinding_FillType</a>,
        <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd_FillType</a>,
        <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding_FillType</a>,
        <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd_FillType</a>,
    };
</pre>

<a href='#Path_Fill_Type'>Fill_Type</a> <a href='#Path_Fill_Type'>selects</a> <a href='#Path_Fill_Type'>the</a> <a href='#Path_Fill_Type'>rule</a> <a href='#Path_Fill_Type'>used</a> <a href='#Path_Fill_Type'>to</a> <a href='#Path_Fill_Type'>fill</a> <a href='SkPath_Reference#Path'>Path</a>. <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>set</a> <a href='SkPath_Reference#Path'>to</a> <a href='#SkPath_kWinding_FillType'>kWinding_FillType</a>
<a href='#SkPath_kWinding_FillType'>fills</a> <a href='#SkPath_kWinding_FillType'>if</a> <a href='#SkPath_kWinding_FillType'>the</a> <a href='#SkPath_kWinding_FillType'>sum</a> <a href='#SkPath_kWinding_FillType'>of</a> <a href='SkPath_Overview#Contour'>Contour</a> <a href='SkPath_Overview#Contour'>edges</a> <a href='SkPath_Overview#Contour'>is</a> <a href='SkPath_Overview#Contour'>not</a> <a href='SkPath_Overview#Contour'>zero</a>, <a href='SkPath_Overview#Contour'>where</a> <a href='SkPath_Overview#Contour'>clockwise</a> <a href='SkPath_Overview#Contour'>edges</a> <a href='SkPath_Overview#Contour'>add</a> <a href='SkPath_Overview#Contour'>one</a>, <a href='SkPath_Overview#Contour'>and</a>
<a href='SkPath_Overview#Contour'>counterclockwise</a> <a href='SkPath_Overview#Contour'>edges</a> <a href='SkPath_Overview#Contour'>subtract</a> <a href='SkPath_Overview#Contour'>one</a>. <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>set</a> <a href='SkPath_Reference#Path'>to</a> <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd_FillType</a> <a href='#SkPath_kEvenOdd_FillType'>fills</a> <a href='#SkPath_kEvenOdd_FillType'>if</a> <a href='#SkPath_kEvenOdd_FillType'>the</a>
<a href='#SkPath_kEvenOdd_FillType'>number</a> <a href='#SkPath_kEvenOdd_FillType'>of</a> <a href='SkPath_Overview#Contour'>Contour</a> <a href='SkPath_Overview#Contour'>edges</a> <a href='SkPath_Overview#Contour'>is</a> <a href='SkPath_Overview#Contour'>odd</a>. <a href='SkPath_Overview#Contour'>Each</a> <a href='#Path_Fill_Type'>Fill_Type</a> <a href='#Path_Fill_Type'>has</a> <a href='#Path_Fill_Type'>an</a> <a href='#Path_Fill_Type'>inverse</a> <a href='#Path_Fill_Type'>variant</a> <a href='#Path_Fill_Type'>that</a>
<a href='#Path_Fill_Type'>reverses</a> <a href='#Path_Fill_Type'>the</a> <a href='#Path_Fill_Type'>rule</a>:
<a href='#SkPath_kInverseWinding_FillType'>kInverseWinding_FillType</a> <a href='#SkPath_kInverseWinding_FillType'>fills</a> <a href='#SkPath_kInverseWinding_FillType'>where</a> <a href='#SkPath_kInverseWinding_FillType'>the</a> <a href='#SkPath_kInverseWinding_FillType'>sum</a> <a href='#SkPath_kInverseWinding_FillType'>of</a> <a href='SkPath_Overview#Contour'>Contour</a> <a href='SkPath_Overview#Contour'>edges</a> <a href='SkPath_Overview#Contour'>is</a> <a href='SkPath_Overview#Contour'>zero</a>;
<a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd_FillType</a> <a href='#SkPath_kInverseEvenOdd_FillType'>fills</a> <a href='#SkPath_kInverseEvenOdd_FillType'>where</a> <a href='#SkPath_kInverseEvenOdd_FillType'>the</a> <a href='#SkPath_kInverseEvenOdd_FillType'>number</a> <a href='#SkPath_kInverseEvenOdd_FillType'>of</a> <a href='SkPath_Overview#Contour'>Contour</a> <a href='SkPath_Overview#Contour'>edges</a> <a href='SkPath_Overview#Contour'>is</a> <a href='SkPath_Overview#Contour'>even</a>.

### Example

<div><fiddle-embed name="71fc6c069c377d808799f2453edabaf5"><div>The top row has two clockwise rectangles. The second row has one clockwise and
one counterclockwise rectangle. The even-odd variants draw the same. The
winding variants draw the top rectangle overlap, which has a winding of 2, the
same as the outer parts of the top rectangles, which have a winding of 1.
</div></fiddle-embed></div>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kWinding_FillType'><code>SkPath::kWinding_FillType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
is enclosed by a non-zero sum of Contour Directions</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kEvenOdd_FillType'><code>SkPath::kEvenOdd_FillType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
is enclosed by an odd number of Contours</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kInverseWinding_FillType'><code>SkPath::kInverseWinding_FillType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
is enclosed by a zero sum of Contour Directions</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kInverseEvenOdd_FillType'><code>SkPath::kInverseEvenOdd_FillType</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
is enclosed by an even number of Contours</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d2c33dc791cd165dcc2423226ba5b095"></fiddle-embed></div>

### See Also

<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Style'>Style</a> <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_getFillType'>getFillType</a> <a href='#SkPath_setFillType'>setFillType</a>

<a name='SkPath_getFillType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_getFillType'>getFillType</a>() <a href='#SkPath_getFillType'>const</a>
</pre>

Returns <a href='#SkPath_FillType'>FillType</a>, <a href='#SkPath_FillType'>the</a> <a href='#SkPath_FillType'>rule</a> <a href='#SkPath_FillType'>used</a> <a href='#SkPath_FillType'>to</a> <a href='#SkPath_FillType'>fill</a> <a href='SkPath_Reference#SkPath'>SkPath</a>. <a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_FillType'>of</a> <a href='#SkPath_FillType'>a</a> <a href='#SkPath_FillType'>new</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a>
<a href='#SkPath_kWinding_FillType'>kWinding_FillType</a>.

### Return Value

one of: <a href='#SkPath_kWinding_FillType'>kWinding_FillType</a>, <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd_FillType</a>,  <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding_FillType</a>,

<a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd_FillType</a>

### Example

<div><fiddle-embed name="019af90e778914e8a109d6305ede4fc4">

#### Example Output

~~~~
default path fill type is kWinding_FillType
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_setFillType'>setFillType</a> <a href='#SkPath_isInverseFillType'>isInverseFillType</a>

<a name='SkPath_setFillType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_setFillType'>setFillType</a>(<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_FillType'>ft</a>)
</pre>

Sets <a href='#SkPath_FillType'>FillType</a>, <a href='#SkPath_FillType'>the</a> <a href='#SkPath_FillType'>rule</a> <a href='#SkPath_FillType'>used</a> <a href='#SkPath_FillType'>to</a> <a href='#SkPath_FillType'>fill</a> <a href='SkPath_Reference#SkPath'>SkPath</a>. <a href='SkPath_Reference#SkPath'>While</a> <a href='SkPath_Reference#SkPath'>there</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>no</a> <a href='SkPath_Reference#SkPath'>check</a>
that <a href='#SkPath_setFillType_ft'>ft</a> <a href='#SkPath_setFillType_ft'>is</a> <a href='#SkPath_setFillType_ft'>legal</a>, <a href='#SkPath_setFillType_ft'>values</a> <a href='#SkPath_setFillType_ft'>outside</a> <a href='#SkPath_setFillType_ft'>of</a> <a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_FillType'>are</a> <a href='#SkPath_FillType'>not</a> <a href='#SkPath_FillType'>supported</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_setFillType_ft'><code><strong>ft</strong></code></a></td>
    <td>one of: <a href='#SkPath_kWinding_FillType'>kWinding_FillType</a>, <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd_FillType</a>,  <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding_FillType</a>,</td>
  </tr>
</table>

<a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd_FillType</a>

### Example

<div><fiddle-embed name="b4a91cd7f50b2a0a0d1bec6d0ac823d2"><div>If empty <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>is</a> <a href='SkPath_Reference#Path'>set</a> <a href='SkPath_Reference#Path'>to</a> <a href='SkPath_Reference#Path'>inverse</a> <a href='#SkPath_FillType'>FillType</a>, <a href='#SkPath_FillType'>it</a> <a href='#SkPath_FillType'>fills</a> <a href='#SkPath_FillType'>all</a> <a href='#SkPath_FillType'>pixels</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_getFillType'>getFillType</a> <a href='#SkPath_toggleInverseFillType'>toggleInverseFillType</a>

<a name='SkPath_isInverseFillType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isInverseFillType'>isInverseFillType</a>() <a href='#SkPath_isInverseFillType'>const</a>
</pre>

Returns if <a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_FillType'>describes</a> <a href='#SkPath_FillType'>area</a> <a href='#SkPath_FillType'>outside</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>geometry</a>. <a href='SkPath_Reference#SkPath'>The</a> <a href='SkPath_Reference#SkPath'>inverse</a> <a href='SkPath_Reference#SkPath'>fill</a> <a href='SkPath_Reference#SkPath'>area</a>
extends indefinitely.

### Return Value

true if <a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_FillType'>is</a> <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding_FillType</a> <a href='#SkPath_kInverseWinding_FillType'>or</a> <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd_FillType</a>

### Example

<div><fiddle-embed name="2a2d39f5da611545caa18bbcea873ab2">

#### Example Output

~~~~
default path fill type is inverse: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_getFillType'>getFillType</a> <a href='#SkPath_setFillType'>setFillType</a> <a href='#SkPath_toggleInverseFillType'>toggleInverseFillType</a>

<a name='SkPath_toggleInverseFillType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_toggleInverseFillType'>toggleInverseFillType</a>()
</pre>

Replaces <a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_FillType'>with</a> <a href='#SkPath_FillType'>its</a> <a href='#SkPath_FillType'>inverse</a>. <a href='#SkPath_FillType'>The</a> <a href='#SkPath_FillType'>inverse</a> <a href='#SkPath_FillType'>of</a> <a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_FillType'>describes</a> <a href='#SkPath_FillType'>the</a> <a href='#SkPath_FillType'>area</a>
<a href='#SkPath_FillType'>unmodified</a> <a href='#SkPath_FillType'>by</a> <a href='#SkPath_FillType'>the</a> <a href='#SkPath_FillType'>original</a> <a href='#SkPath_FillType'>FillType</a>.

| <a href='#SkPath_FillType'>FillType</a> | toggled <a href='#SkPath_FillType'>FillType</a> |
| --- | ---  |
| <a href='#SkPath_kWinding_FillType'>kWinding_FillType</a> | <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding_FillType</a> |
| <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd_FillType</a> | <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd_FillType</a> |
| <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding_FillType</a> | <a href='#SkPath_kWinding_FillType'>kWinding_FillType</a> |
| <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd_FillType</a> | <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd_FillType</a> |

### Example

<div><fiddle-embed name="400facce23d417bc5043c5f58404afbd"><div><a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>drawn</a> <a href='SkPath_Reference#Path'>normally</a> <a href='SkPath_Reference#Path'>and</a> <a href='SkPath_Reference#Path'>through</a> <a href='SkPath_Reference#Path'>its</a> <a href='SkPath_Reference#Path'>inverse</a> <a href='SkPath_Reference#Path'>touches</a> <a href='SkPath_Reference#Path'>every</a> <a href='undocumented#Pixel'>pixel</a> <a href='undocumented#Pixel'>once</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_getFillType'>getFillType</a> <a href='#SkPath_setFillType'>setFillType</a> <a href='#SkPath_isInverseFillType'>isInverseFillType</a>

<a name='Convexity'></a>

<a name='SkPath_Convexity'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPath_Convexity'>Convexity</a> : <a href='#SkPath_Convexity'>uint8_t</a> {
        <a href='#SkPath_kUnknown_Convexity'>kUnknown_Convexity</a>,
        <a href='#SkPath_kConvex_Convexity'>kConvex_Convexity</a>,
        <a href='#SkPath_kConcave_Convexity'>kConcave_Convexity</a>,
    };
</pre>

<a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>is</a> <a href='SkPath_Reference#Path'>convex</a> <a href='SkPath_Reference#Path'>if</a> <a href='SkPath_Reference#Path'>it</a> <a href='SkPath_Reference#Path'>contains</a> <a href='SkPath_Reference#Path'>one</a> <a href='SkPath_Overview#Contour'>Contour</a> <a href='SkPath_Overview#Contour'>and</a> <a href='SkPath_Overview#Contour'>Contour</a> <a href='SkPath_Overview#Contour'>loops</a> <a href='SkPath_Overview#Contour'>no</a> <a href='SkPath_Overview#Contour'>more</a> <a href='SkPath_Overview#Contour'>than</a>
360 <a href='SkPath_Overview#Contour'>degrees</a>, <a href='SkPath_Overview#Contour'>and</a> <a href='SkPath_Overview#Contour'>Contour</a> <a href='SkPath_Overview#Contour'>angles</a> <a href='SkPath_Overview#Contour'>all</a> <a href='SkPath_Overview#Contour'>have</a> <a href='SkPath_Overview#Contour'>same</a> <a href='#SkPath_Direction'>Direction</a>. <a href='#SkPath_Direction'>Convex</a> <a href='SkPath_Reference#Path'>Path</a>
<a href='SkPath_Reference#Path'>may</a> <a href='SkPath_Reference#Path'>have</a> <a href='SkPath_Reference#Path'>better</a> <a href='SkPath_Reference#Path'>performance</a> <a href='SkPath_Reference#Path'>and</a> <a href='SkPath_Reference#Path'>require</a> <a href='SkPath_Reference#Path'>fewer</a> <a href='SkPath_Reference#Path'>resources</a> <a href='SkPath_Reference#Path'>on</a> <a href='#GPU_Surface'>GPU_Surface</a>.

<a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>is</a> <a href='SkPath_Reference#Path'>concave</a> <a href='SkPath_Reference#Path'>when</a> <a href='SkPath_Reference#Path'>either</a> <a href='SkPath_Reference#Path'>at</a> <a href='SkPath_Reference#Path'>least</a> <a href='SkPath_Reference#Path'>one</a> <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>change</a> <a href='#SkPath_Direction'>is</a> <a href='#SkPath_Direction'>clockwise</a> <a href='#SkPath_Direction'>and</a>
<a href='#SkPath_Direction'>another</a> <a href='#SkPath_Direction'>is</a> <a href='#SkPath_Direction'>counterclockwise</a>, <a href='#SkPath_Direction'>or</a> <a href='#SkPath_Direction'>the</a> <a href='#SkPath_Direction'>sum</a> <a href='#SkPath_Direction'>of</a> <a href='#SkPath_Direction'>the</a> <a href='#SkPath_Direction'>changes</a> <a href='#SkPath_Direction'>in</a> <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>is</a> <a href='#SkPath_Direction'>not</a> 360
<a href='#SkPath_Direction'>degrees</a>.

<a href='#SkPath_Direction'>Initially</a> <a href='SkPath_Reference#Path'>Path</a> <a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_Convexity'>is</a> <a href='#SkPath_kUnknown_Convexity'>kUnknown_Convexity</a>. <a href='SkPath_Reference#Path'>Path</a> <a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_Convexity'>is</a> <a href='#SkPath_Convexity'>computed</a>
<a href='#SkPath_Convexity'>if</a> <a href='#SkPath_Convexity'>needed</a> <a href='#SkPath_Convexity'>by</a> <a href='#SkPath_Convexity'>destination</a> <a href='SkSurface_Reference#Surface'>Surface</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kUnknown_Convexity'><code>SkPath::kUnknown_Convexity</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
indicates Convexity has not been determined</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kConvex_Convexity'><code>SkPath::kConvex_Convexity</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
one Contour made of a simple geometry without indentations</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kConcave_Convexity'><code>SkPath::kConcave_Convexity</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
more than one Contour, or a geometry with indentations</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ac49e8b810bd6ed5d84b4f5a3b40a0ec"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_getConvexity'>getConvexity</a> <a href='#SkPath_getConvexityOrUnknown'>getConvexityOrUnknown</a> <a href='#SkPath_setConvexity'>setConvexity</a> <a href='#SkPath_isConvex'>isConvex</a>

<a name='SkPath_getConvexity'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_getConvexity'>getConvexity</a>() <a href='#SkPath_getConvexity'>const</a>
</pre>

Computes <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_Convexity'>if</a> <a href='#SkPath_Convexity'>required</a>, <a href='#SkPath_Convexity'>and</a> <a href='#SkPath_Convexity'>returns</a> <a href='#SkPath_Convexity'>stored</a> <a href='#SkPath_Convexity'>value</a>.
<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_Convexity'>is</a> <a href='#SkPath_Convexity'>computed</a> <a href='#SkPath_Convexity'>if</a> <a href='#SkPath_Convexity'>stored</a> <a href='#SkPath_Convexity'>value</a> <a href='#SkPath_Convexity'>is</a> <a href='#SkPath_kUnknown_Convexity'>kUnknown_Convexity</a>,
or if <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>has</a> <a href='SkPath_Reference#SkPath'>been</a> <a href='SkPath_Reference#SkPath'>altered</a> <a href='SkPath_Reference#SkPath'>since</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_Convexity'>was</a> <a href='#SkPath_Convexity'>computed</a> <a href='#SkPath_Convexity'>or</a> <a href='#SkPath_Convexity'>set</a>.

### Return Value

computed or stored <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Convexity'>Convexity</a>

### Example

<div><fiddle-embed name="a8f36f2fa90003e3691fd0da0bb0c243"></fiddle-embed></div>

### See Also

<a href='#SkPath_Convexity'>Convexity</a> <a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_getConvexityOrUnknown'>getConvexityOrUnknown</a> <a href='#SkPath_setConvexity'>setConvexity</a> <a href='#SkPath_isConvex'>isConvex</a>

<a name='SkPath_getConvexityOrUnknown'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_getConvexityOrUnknown'>getConvexityOrUnknown</a>() <a href='#SkPath_getConvexityOrUnknown'>const</a>
</pre>

Returns last computed <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Convexity'>Convexity</a>, <a href='#SkPath_Convexity'>or</a> <a href='#SkPath_kUnknown_Convexity'>kUnknown_Convexity</a> <a href='#SkPath_kUnknown_Convexity'>if</a>
<a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>has</a> <a href='SkPath_Reference#SkPath'>been</a> <a href='SkPath_Reference#SkPath'>altered</a> <a href='SkPath_Reference#SkPath'>since</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_Convexity'>was</a> <a href='#SkPath_Convexity'>computed</a> <a href='#SkPath_Convexity'>or</a> <a href='#SkPath_Convexity'>set</a>.

### Return Value

stored <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Convexity'>Convexity</a>

### Example

<div><fiddle-embed name="111c59e9afadb940ab8f41bdc25378a4"><div><a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_Convexity'>is</a> <a href='#SkPath_Convexity'>unknown</a> <a href='#SkPath_Convexity'>unless</a> <a href='#SkPath_getConvexity'>getConvexity</a> <a href='#SkPath_getConvexity'>is</a> <a href='#SkPath_getConvexity'>called</a> <a href='#SkPath_getConvexity'>without</a> <a href='#SkPath_getConvexity'>a</a> <a href='#SkPath_getConvexity'>subsequent</a> <a href='#SkPath_getConvexity'>call</a>
<a href='#SkPath_getConvexity'>that</a> <a href='#SkPath_getConvexity'>alters</a> <a href='#SkPath_getConvexity'>the</a> <a href='SkPath_Reference#Path'>path</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_Convexity'>Convexity</a> <a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_getConvexity'>getConvexity</a> <a href='#SkPath_setConvexity'>setConvexity</a> <a href='#SkPath_isConvex'>isConvex</a>

<a name='SkPath_setConvexity'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_setConvexity'>setConvexity</a>(<a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_Convexity'>convexity</a>)
</pre>

Stores <a href='#SkPath_setConvexity_convexity'>convexity</a> <a href='#SkPath_setConvexity_convexity'>so</a> <a href='#SkPath_setConvexity_convexity'>that</a> <a href='#SkPath_setConvexity_convexity'>it</a> <a href='#SkPath_setConvexity_convexity'>is</a> <a href='#SkPath_setConvexity_convexity'>later</a> <a href='#SkPath_setConvexity_convexity'>returned</a> <a href='#SkPath_setConvexity_convexity'>by</a> <a href='#SkPath_getConvexity'>getConvexity</a>() <a href='#SkPath_getConvexity'>or</a> <a href='#SkPath_getConvexityOrUnknown'>getConvexityOrUnknown</a>().
<a href='#SkPath_setConvexity_convexity'>convexity</a> <a href='#SkPath_setConvexity_convexity'>may</a> <a href='#SkPath_setConvexity_convexity'>differ</a> <a href='#SkPath_setConvexity_convexity'>from</a> <a href='#SkPath_getConvexity'>getConvexity</a>(), <a href='#SkPath_getConvexity'>although</a> <a href='#SkPath_getConvexity'>setting</a> <a href='#SkPath_getConvexity'>an</a> <a href='#SkPath_getConvexity'>incorrect</a> <a href='#SkPath_getConvexity'>value</a> <a href='#SkPath_getConvexity'>may</a>
cause incorrect or inefficient drawing.

If <a href='#SkPath_setConvexity_convexity'>convexity</a> <a href='#SkPath_setConvexity_convexity'>is</a> <a href='#SkPath_kUnknown_Convexity'>kUnknown_Convexity</a>: <a href='#SkPath_getConvexity'>getConvexity</a>() <a href='#SkPath_getConvexity'>will</a>
compute <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Convexity'>Convexity</a>, <a href='#SkPath_Convexity'>and</a> <a href='#SkPath_getConvexityOrUnknown'>getConvexityOrUnknown</a>() <a href='#SkPath_getConvexityOrUnknown'>will</a> <a href='#SkPath_getConvexityOrUnknown'>return</a> <a href='#SkPath_kUnknown_Convexity'>kUnknown_Convexity</a>.

If <a href='#SkPath_setConvexity_convexity'>convexity</a> <a href='#SkPath_setConvexity_convexity'>is</a> <a href='#SkPath_kConvex_Convexity'>kConvex_Convexity</a> <a href='#SkPath_kConvex_Convexity'>or</a> <a href='#SkPath_kConcave_Convexity'>kConcave_Convexity</a>, <a href='#SkPath_getConvexity'>getConvexity</a>()
and <a href='#SkPath_getConvexityOrUnknown'>getConvexityOrUnknown</a>() <a href='#SkPath_getConvexityOrUnknown'>will</a> <a href='#SkPath_getConvexityOrUnknown'>return</a> <a href='#SkPath_setConvexity_convexity'>convexity</a> <a href='#SkPath_setConvexity_convexity'>until</a> <a href='#SkPath_setConvexity_convexity'>the</a> <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>is</a>
altered.

### Parameters

<table>  <tr>    <td><a name='SkPath_setConvexity_convexity'><code><strong>convexity</strong></code></a></td>
    <td>one of: <a href='#SkPath_kUnknown_Convexity'>kUnknown_Convexity</a>, <a href='#SkPath_kConvex_Convexity'>kConvex_Convexity</a>, <a href='#SkPath_kConvex_Convexity'>or</a> <a href='#SkPath_kConcave_Convexity'>kConcave_Convexity</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="875e32b4b1cb48d739325705fc0fa42c"></fiddle-embed></div>

### See Also

<a href='#SkPath_Convexity'>Convexity</a> <a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_getConvexity'>getConvexity</a> <a href='#SkPath_getConvexityOrUnknown'>getConvexityOrUnknown</a> <a href='#SkPath_isConvex'>isConvex</a>

<a name='SkPath_isConvex'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isConvex'>isConvex</a>() <a href='#SkPath_isConvex'>const</a>
</pre>

Computes <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_Convexity'>if</a> <a href='#SkPath_Convexity'>required</a>, <a href='#SkPath_Convexity'>and</a> <a href='#SkPath_Convexity'>returns</a> <a href='#SkPath_Convexity'>true</a> <a href='#SkPath_Convexity'>if</a> <a href='#SkPath_Convexity'>value</a> <a href='#SkPath_Convexity'>is</a> <a href='#SkPath_kConvex_Convexity'>kConvex_Convexity</a>.
If <a href='#SkPath_setConvexity'>setConvexity</a>() <a href='#SkPath_setConvexity'>was</a> <a href='#SkPath_setConvexity'>called</a> <a href='#SkPath_setConvexity'>with</a> <a href='#SkPath_kConvex_Convexity'>kConvex_Convexity</a> <a href='#SkPath_kConvex_Convexity'>or</a> <a href='#SkPath_kConcave_Convexity'>kConcave_Convexity</a>, <a href='#SkPath_kConcave_Convexity'>and</a>
the <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>has</a> <a href='SkPath_Reference#Path'>not</a> <a href='SkPath_Reference#Path'>been</a> <a href='SkPath_Reference#Path'>altered</a>, <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_Convexity'>is</a> <a href='#SkPath_Convexity'>not</a> <a href='#SkPath_Convexity'>recomputed</a>.

### Return Value

true if <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_Convexity'>stored</a> <a href='#SkPath_Convexity'>or</a> <a href='#SkPath_Convexity'>computed</a> <a href='#SkPath_Convexity'>is</a> <a href='#SkPath_kConvex_Convexity'>kConvex_Convexity</a>

### Example

<div><fiddle-embed name="d8be8b6e59de244e4cbf58ec9554557b"><div>Concave shape is erroneously considered convex after a forced call to
<a href='#SkPath_setConvexity'>setConvexity</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_Convexity'>Convexity</a> <a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_getConvexity'>getConvexity</a> <a href='#SkPath_getConvexityOrUnknown'>getConvexityOrUnknown</a> <a href='#SkPath_setConvexity'>setConvexity</a>

<a name='SkPath_isOval'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isOval'>isOval</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>bounds</a>) <a href='SkRect_Reference#SkRect'>const</a>
</pre>

Returns true if this <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>is</a> <a href='SkPath_Reference#Path'>recognized</a> <a href='SkPath_Reference#Path'>as</a> <a href='SkPath_Reference#Path'>an</a> <a href='undocumented#Oval'>oval</a> <a href='undocumented#Oval'>or</a> <a href='undocumented#Circle'>circle</a>.

<a href='#SkPath_isOval_bounds'>bounds</a> <a href='#SkPath_isOval_bounds'>receives</a> <a href='#SkPath_isOval_bounds'>bounds</a> <a href='#SkPath_isOval_bounds'>of</a> <a href='undocumented#Oval'>oval</a>.

<a href='#SkPath_isOval_bounds'>bounds</a> <a href='#SkPath_isOval_bounds'>is</a> <a href='#SkPath_isOval_bounds'>unmodified</a> <a href='#SkPath_isOval_bounds'>if</a> <a href='undocumented#Oval'>oval</a> <a href='undocumented#Oval'>is</a> <a href='undocumented#Oval'>not</a> <a href='undocumented#Oval'>found</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_isOval_bounds'><code><strong>bounds</strong></code></a></td>
    <td>storage for bounding <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='undocumented#Oval'>oval</a>; <a href='undocumented#Oval'>may</a> <a href='undocumented#Oval'>be</a> <a href='undocumented#Oval'>nullptr</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>recognized</a> <a href='SkPath_Reference#SkPath'>as</a> <a href='SkPath_Reference#SkPath'>an</a> <a href='undocumented#Oval'>oval</a> <a href='undocumented#Oval'>or</a> <a href='undocumented#Circle'>circle</a>

### Example

<div><fiddle-embed name="a51256952b183ee0f7004f2c87cbbf5b"></fiddle-embed></div>

### See Also

<a href='undocumented#Oval'>Oval</a> <a href='#SkPath_addCircle'>addCircle</a> <a href='#SkPath_addOval'>addOval</a>

<a name='SkPath_isRRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isRRect'>isRRect</a>(<a href='SkRRect_Reference#SkRRect'>SkRRect</a>* <a href='SkRRect_Reference#SkRRect'>rrect</a>) <a href='SkRRect_Reference#SkRRect'>const</a>
</pre>

Returns true if <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>is</a> <a href='SkPath_Reference#Path'>representable</a> <a href='SkPath_Reference#Path'>as</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>.
Returns false if <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>is</a> <a href='SkPath_Reference#Path'>representable</a> <a href='SkPath_Reference#Path'>as</a> <a href='undocumented#Oval'>oval</a>, <a href='undocumented#Circle'>circle</a>, <a href='undocumented#Circle'>or</a> <a href='SkRect_Reference#SkRect'>SkRect</a>.

<a href='#SkPath_isRRect_rrect'>rrect</a> <a href='#SkPath_isRRect_rrect'>receives</a> <a href='#SkPath_isRRect_rrect'>bounds</a> <a href='#SkPath_isRRect_rrect'>of</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>.

<a href='#SkPath_isRRect_rrect'>rrect</a> <a href='#SkPath_isRRect_rrect'>is</a> <a href='#SkPath_isRRect_rrect'>unmodified</a> <a href='#SkPath_isRRect_rrect'>if</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>is</a> <a href='SkRRect_Reference#SkRRect'>not</a> <a href='SkRRect_Reference#SkRRect'>found</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_isRRect_rrect'><code><strong>rrect</strong></code></a></td>
    <td>storage for bounding <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>of</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>; <a href='SkRRect_Reference#SkRRect'>may</a> <a href='SkRRect_Reference#SkRRect'>be</a> <a href='SkRRect_Reference#SkRRect'>nullptr</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>contains</a> <a href='SkPath_Reference#SkPath'>only</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>

### Example

<div><fiddle-embed name="2aa939b90d96aff436b145a96305132c"><div>Draw rounded rectangle and its bounds.
</div></fiddle-embed></div>

### See Also

<a href='#RRect'>Round_Rect</a> <a href='#SkPath_addRoundRect'>addRoundRect</a> <a href='#SkPath_addRRect'>addRRect</a>

<a name='SkPath_reset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_reset'>reset()</a>
</pre>

Sets <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>its</a> <a href='SkPath_Reference#SkPath'>initial</a> <a href='SkPath_Reference#SkPath'>state</a>.
Removes verb array, <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>array</a>, <a href='SkPoint_Reference#SkPoint'>and</a> <a href='SkPath_Reference#Conic_Weight'>weights</a>, <a href='SkPath_Reference#Conic_Weight'>and</a> <a href='SkPath_Reference#Conic_Weight'>sets</a> <a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_FillType'>to</a> <a href='#SkPath_kWinding_FillType'>kWinding_FillType</a>.
Internal storage associated with <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>released</a>.

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="8cdca35d2964bbbecb93d79a13f71c65"></fiddle-embed></div>

### See Also

<a href='#SkPath_rewind'>rewind()</a>

<a name='SkPath_rewind'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_rewind'>rewind()</a>
</pre>

Sets <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>its</a> <a href='SkPath_Reference#SkPath'>initial</a> <a href='SkPath_Reference#SkPath'>state</a>, <a href='SkPath_Reference#SkPath'>preserving</a> <a href='SkPath_Reference#SkPath'>internal</a> <a href='SkPath_Reference#SkPath'>storage</a>.
Removes verb array, <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>array</a>, <a href='SkPoint_Reference#SkPoint'>and</a> <a href='SkPath_Reference#Conic_Weight'>weights</a>, <a href='SkPath_Reference#Conic_Weight'>and</a> <a href='SkPath_Reference#Conic_Weight'>sets</a> <a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_FillType'>to</a> <a href='#SkPath_kWinding_FillType'>kWinding_FillType</a>.
Internal storage associated with <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>retained</a>.

Use <a href='#SkPath_rewind'>rewind()</a> <a href='#SkPath_rewind'>instead</a> <a href='#SkPath_rewind'>of</a> <a href='#SkPath_reset'>reset()</a> <a href='#SkPath_reset'>if</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>storage</a> <a href='SkPath_Reference#SkPath'>will</a> <a href='SkPath_Reference#SkPath'>be</a> <a href='SkPath_Reference#SkPath'>reused</a> <a href='SkPath_Reference#SkPath'>and</a> <a href='SkPath_Reference#SkPath'>performance</a>
is critical.

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="f1fedbb89da9c2a33a91805175663012"><div>Although path1 retains its internal storage, it is indistinguishable from
a newly initialized <a href='SkPath_Reference#Path'>path</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_reset'>reset()</a>

<a name='SkPath_isEmpty'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isEmpty'>isEmpty</a>() <a href='#SkPath_isEmpty'>const</a>
</pre>

Returns if <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>empty</a>.
Empty <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>may</a> <a href='SkPath_Reference#SkPath'>have</a> <a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_FillType'>but</a> <a href='#SkPath_FillType'>has</a> <a href='#SkPath_FillType'>no</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>, <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a>, <a href='#SkPath_Verb'>or</a> <a href='SkPath_Reference#Conic'>conic</a> <a href='SkPath_Reference#Conic'>weight</a>.
<a href='#SkPath_empty_constructor'>SkPath()</a> <a href='SkPath_Reference#SkPath'>constructs</a> <a href='SkPath_Reference#SkPath'>empty</a> <a href='SkPath_Reference#SkPath'>SkPath</a>; <a href='#SkPath_reset'>reset()</a> <a href='#SkPath_reset'>and</a> <a href='#SkPath_rewind'>rewind()</a> <a href='#SkPath_rewind'>make</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>empty</a>.

### Return Value

true if the <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>contains</a> <a href='SkPath_Reference#Path'>no</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>array</a>

### Example

<div><fiddle-embed name="0b34e6d55d11586744adeb889d2a12f4">

#### Example Output

~~~~
initial path is empty
after moveTo path is not empty
after rewind path is empty
after lineTo path is not empty
after reset path is empty
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_empty_constructor'>SkPath()</a> <a href='#SkPath_reset'>reset()</a> <a href='#SkPath_rewind'>rewind()</a>

<a name='SkPath_isLastContourClosed'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isLastContourClosed'>isLastContourClosed</a>() <a href='#SkPath_isLastContourClosed'>const</a>
</pre>

Returns if <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>is</a> <a href='SkPath_Overview#Contour'>closed</a>.
<a href='SkPath_Overview#Contour'>Contour</a> <a href='SkPath_Overview#Contour'>is</a> <a href='SkPath_Overview#Contour'>closed</a> <a href='SkPath_Overview#Contour'>if</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>array</a> <a href='#SkPath_Verb'>was</a> <a href='#SkPath_Verb'>last</a> <a href='#SkPath_Verb'>modified</a> <a href='#SkPath_Verb'>by</a> <a href='#SkPath_close'>close()</a>. <a href='#SkPath_close'>When</a> <a href='#SkPath_close'>stroked</a>,
closed <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>draws</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Join'>Join</a> <a href='#SkPaint_Join'>instead</a> <a href='#SkPaint_Join'>of</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Cap'>Cap</a> <a href='#SkPaint_Cap'>at</a> <a href='#SkPaint_Cap'>first</a> <a href='#SkPaint_Cap'>and</a> <a href='#SkPaint_Cap'>last</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>.

### Return Value

true if the last <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>ends</a> <a href='SkPath_Overview#Contour'>with</a> <a href='SkPath_Overview#Contour'>a</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a>

### Example

<div><fiddle-embed name="03b740ab94b9017800a52e30b5e7fee7"><div><a href='#SkPath_close'>close()</a> <a href='#SkPath_close'>has</a> <a href='#SkPath_close'>no</a> <a href='#SkPath_close'>effect</a> <a href='#SkPath_close'>if</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>is</a> <a href='SkPath_Reference#Path'>empty</a>; <a href='#SkPath_isLastContourClosed'>isLastContourClosed</a>() <a href='#SkPath_isLastContourClosed'>returns</a>
<a href='#SkPath_isLastContourClosed'>false</a> <a href='#SkPath_isLastContourClosed'>until</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>has</a> <a href='SkPath_Reference#Path'>geometry</a> <a href='SkPath_Reference#Path'>followed</a> <a href='SkPath_Reference#Path'>by</a> <a href='#SkPath_close'>close()</a>.
</div>

#### Example Output

~~~~
initial last contour is not closed
after close last contour is not closed
after lineTo last contour is not closed
after close last contour is closed
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_close'>close()</a>

<a name='SkPath_isFinite'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isFinite'>isFinite</a>() <a href='#SkPath_isFinite'>const</a>
</pre>

Returns true for finite <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>array</a> <a href='SkPoint_Reference#SkPoint'>values</a> <a href='SkPoint_Reference#SkPoint'>between</a> <a href='SkPoint_Reference#SkPoint'>negative</a> <a href='undocumented#SK_ScalarMax'>SK_ScalarMax</a> <a href='undocumented#SK_ScalarMax'>and</a>
positive <a href='undocumented#SK_ScalarMax'>SK_ScalarMax</a>. <a href='undocumented#SK_ScalarMax'>Returns</a> <a href='undocumented#SK_ScalarMax'>false</a> <a href='undocumented#SK_ScalarMax'>for</a> <a href='undocumented#SK_ScalarMax'>any</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>array</a> <a href='SkPoint_Reference#SkPoint'>value</a> <a href='SkPoint_Reference#SkPoint'>of</a>
<a href='undocumented#SK_ScalarInfinity'>SK_ScalarInfinity</a>, <a href='undocumented#SK_ScalarNegativeInfinity'>SK_ScalarNegativeInfinity</a>, <a href='undocumented#SK_ScalarNegativeInfinity'>or</a> <a href='undocumented#SK_ScalarNaN'>SK_ScalarNaN</a>.

### Return Value

true if all <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>values</a> <a href='SkPoint_Reference#SkPoint'>are</a> <a href='SkPoint_Reference#SkPoint'>finite</a>

### Example

<div><fiddle-embed name="dd4e4dd2aaa8039b2430729c6b3af817">

#### Example Output

~~~~
initial path is finite
after line path is finite
after scale path is not finite
~~~~

</fiddle-embed></div>

### See Also

<a href='undocumented#SkScalar'>SkScalar</a>

<a name='SkPath_isVolatile'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isVolatile'>isVolatile</a>() <a href='#SkPath_isVolatile'>const</a>
</pre>

Returns true if the <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>is</a> <a href='SkPath_Reference#Path'>volatile</a>; <a href='SkPath_Reference#Path'>it</a> <a href='SkPath_Reference#Path'>will</a> <a href='SkPath_Reference#Path'>not</a> <a href='SkPath_Reference#Path'>be</a> <a href='SkPath_Reference#Path'>altered</a> <a href='SkPath_Reference#Path'>or</a> <a href='SkPath_Reference#Path'>discarded</a>
by the caller after it is drawn. <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>by</a> <a href='SkPath_Reference#SkPath'>default</a> <a href='SkPath_Reference#SkPath'>have</a> <a href='SkPath_Reference#SkPath'>volatile</a> <a href='SkPath_Reference#SkPath'>set</a> <a href='SkPath_Reference#SkPath'>false</a>, <a href='SkPath_Reference#SkPath'>allowing</a>
<a href='SkSurface_Reference#SkSurface'>SkSurface</a> <a href='SkSurface_Reference#SkSurface'>to</a> <a href='SkSurface_Reference#SkSurface'>attach</a> <a href='SkSurface_Reference#SkSurface'>a</a> <a href='SkSurface_Reference#SkSurface'>cache</a> <a href='SkSurface_Reference#SkSurface'>of</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>which</a> <a href='undocumented#Data'>speeds</a> <a href='undocumented#Data'>repeated</a> <a href='undocumented#Data'>drawing</a>. <a href='undocumented#Data'>If</a> <a href='undocumented#Data'>true</a>, <a href='SkSurface_Reference#SkSurface'>SkSurface</a>
may not speed repeated drawing.

### Return Value

true if caller will alter <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>after</a> <a href='SkPath_Reference#SkPath'>drawing</a>

### Example

<div><fiddle-embed name="c722ebe8ac991d77757799ce29e509e1">

#### Example Output

~~~~
volatile by default is false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_setIsVolatile'>setIsVolatile</a>

<a name='Volatile'></a>

<a name='SkPath_setIsVolatile'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_setIsVolatile'>setIsVolatile</a>(<a href='#SkPath_setIsVolatile'>bool</a> <a href='#SkPath_isVolatile'>isVolatile</a>)
</pre>

Specifies whether <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>volatile</a>; <a href='SkPath_Reference#SkPath'>whether</a> <a href='SkPath_Reference#SkPath'>it</a> <a href='SkPath_Reference#SkPath'>will</a> <a href='SkPath_Reference#SkPath'>be</a> <a href='SkPath_Reference#SkPath'>altered</a> <a href='SkPath_Reference#SkPath'>or</a> <a href='SkPath_Reference#SkPath'>discarded</a>
by the caller after it is drawn. <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>by</a> <a href='SkPath_Reference#SkPath'>default</a> <a href='SkPath_Reference#SkPath'>have</a> <a href='SkPath_Reference#SkPath'>volatile</a> <a href='SkPath_Reference#SkPath'>set</a> <a href='SkPath_Reference#SkPath'>false</a>, <a href='SkPath_Reference#SkPath'>allowing</a>
<a href='undocumented#SkBaseDevice'>SkBaseDevice</a> <a href='undocumented#SkBaseDevice'>to</a> <a href='undocumented#SkBaseDevice'>attach</a> <a href='undocumented#SkBaseDevice'>a</a> <a href='undocumented#SkBaseDevice'>cache</a> <a href='undocumented#SkBaseDevice'>of</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>which</a> <a href='undocumented#Data'>speeds</a> <a href='undocumented#Data'>repeated</a> <a href='undocumented#Data'>drawing</a>.

Mark temporary <a href='SkPath_Reference#Path'>paths</a>, <a href='SkPath_Reference#Path'>discarded</a> <a href='SkPath_Reference#Path'>or</a> <a href='SkPath_Reference#Path'>modified</a> <a href='SkPath_Reference#Path'>after</a> <a href='SkPath_Reference#Path'>use</a>, <a href='SkPath_Reference#Path'>as</a> <a href='SkPath_Reference#Path'>volatile</a>
to inform <a href='undocumented#SkBaseDevice'>SkBaseDevice</a> <a href='undocumented#SkBaseDevice'>that</a> <a href='undocumented#SkBaseDevice'>the</a> <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>need</a> <a href='SkPath_Reference#Path'>not</a> <a href='SkPath_Reference#Path'>be</a> <a href='SkPath_Reference#Path'>cached</a>.

Mark animating <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>volatile</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>improve</a> <a href='SkPath_Reference#SkPath'>performance</a>.
Mark unchanging <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>non-volatile</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>improve</a> <a href='SkPath_Reference#SkPath'>repeated</a> <a href='SkPath_Reference#SkPath'>rendering</a>.

<a href='undocumented#Raster_Surface'>raster surface</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>draws</a> <a href='SkPath_Reference#SkPath'>are</a> <a href='SkPath_Reference#SkPath'>affected</a> <a href='SkPath_Reference#SkPath'>by</a> <a href='SkPath_Reference#SkPath'>volatile</a> <a href='SkPath_Reference#SkPath'>for</a> <a href='SkPath_Reference#SkPath'>some</a> <a href='SkPath_Reference#SkPath'>shadows</a>.
<a href='undocumented#GPU_Surface'>GPU surface</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>draws</a> <a href='SkPath_Reference#SkPath'>are</a> <a href='SkPath_Reference#SkPath'>affected</a> <a href='SkPath_Reference#SkPath'>by</a> <a href='SkPath_Reference#SkPath'>volatile</a> <a href='SkPath_Reference#SkPath'>for</a> <a href='SkPath_Reference#SkPath'>some</a> <a href='SkPath_Reference#SkPath'>shadows</a> <a href='SkPath_Reference#SkPath'>and</a> <a href='SkPath_Reference#SkPath'>concave</a> <a href='SkPath_Reference#SkPath'>geometries</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_setIsVolatile_isVolatile'><code><strong>isVolatile</strong></code></a></td>
    <td>true if caller will alter <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>after</a> <a href='SkPath_Reference#SkPath'>drawing</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="2049ff5141f0c80aac497618622b28af"></fiddle-embed></div>

### See Also

<a href='#SkPath_setIsVolatile_isVolatile'>isVolatile</a>

<a name='SkPath_IsLineDegenerate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkPath_IsLineDegenerate'>IsLineDegenerate</a>(<a href='#SkPath_IsLineDegenerate'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p1</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p2</a>, <a href='SkPoint_Reference#SkPoint'>bool</a> <a href='SkPoint_Reference#SkPoint'>exact</a>)
</pre>

Tests if <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>between</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pair</a> <a href='SkPoint_Reference#SkPoint'>is</a> <a href='SkPoint_Reference#SkPoint'>degenerate</a>.
<a href='undocumented#Line'>Line</a> <a href='undocumented#Line'>with</a> <a href='undocumented#Line'>no</a> <a href='undocumented#Line'>length</a> <a href='undocumented#Line'>or</a> <a href='undocumented#Line'>that</a> <a href='undocumented#Line'>moves</a> <a href='undocumented#Line'>a</a> <a href='undocumented#Line'>very</a> <a href='undocumented#Line'>short</a> <a href='undocumented#Line'>distance</a> <a href='undocumented#Line'>is</a> <a href='undocumented#Line'>degenerate</a>; <a href='undocumented#Line'>it</a> <a href='undocumented#Line'>is</a>
treated as a <a href='SkPoint_Reference#Point'>point</a>.

<a href='#SkPath_IsLineDegenerate_exact'>exact</a> <a href='#SkPath_IsLineDegenerate_exact'>changes</a> <a href='#SkPath_IsLineDegenerate_exact'>the</a> <a href='#SkPath_IsLineDegenerate_exact'>equality</a> <a href='#SkPath_IsLineDegenerate_exact'>test</a>. <a href='#SkPath_IsLineDegenerate_exact'>If</a> <a href='#SkPath_IsLineDegenerate_exact'>true</a>, <a href='#SkPath_IsLineDegenerate_exact'>returns</a> <a href='#SkPath_IsLineDegenerate_exact'>true</a> <a href='#SkPath_IsLineDegenerate_exact'>only</a> <a href='#SkPath_IsLineDegenerate_exact'>if</a> <a href='#SkPath_IsLineDegenerate_p1'>p1</a> <a href='#SkPath_IsLineDegenerate_p1'>equals</a> <a href='#SkPath_IsLineDegenerate_p2'>p2</a>.
If false, returns true if <a href='#SkPath_IsLineDegenerate_p1'>p1</a> <a href='#SkPath_IsLineDegenerate_p1'>equals</a> <a href='#SkPath_IsLineDegenerate_p1'>or</a> <a href='#SkPath_IsLineDegenerate_p1'>nearly</a> <a href='#SkPath_IsLineDegenerate_p1'>equals</a> <a href='#SkPath_IsLineDegenerate_p2'>p2</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_IsLineDegenerate_p1'><code><strong>p1</strong></code></a></td>
    <td><a href='undocumented#Line'>line</a> <a href='undocumented#Line'>start</a> <a href='SkPoint_Reference#Point'>point</a></td>
  </tr>
  <tr>    <td><a name='SkPath_IsLineDegenerate_p2'><code><strong>p2</strong></code></a></td>
    <td><a href='undocumented#Line'>line</a> <a href='undocumented#Line'>end</a> <a href='SkPoint_Reference#Point'>point</a></td>
  </tr>
  <tr>    <td><a name='SkPath_IsLineDegenerate_exact'><code><strong>exact</strong></code></a></td>
    <td>if false, allow nearly equals</td>
  </tr>
</table>

### Return Value

true if <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>is</a> <a href='undocumented#Line'>degenerate</a>; <a href='undocumented#Line'>its</a> <a href='undocumented#Line'>length</a> <a href='undocumented#Line'>is</a> <a href='undocumented#Line'>effectively</a> <a href='undocumented#Line'>zero</a>

### Example

<div><fiddle-embed name="97a031f9186ade586928563840ce9116"><div>As single precision floats, 100 and 100.000001 have the same bit representation,
and are exactly equal. 100 and 100.0001 have different bit representations, and
are not exactly equal, but are nearly equal.
</div>

#### Example Output

~~~~
line from (100,100) to (100,100) is degenerate, nearly
line from (100,100) to (100,100) is degenerate, exactly
line from (100,100) to (100.0001,100.0001) is degenerate, nearly
line from (100,100) to (100.0001,100.0001) is not degenerate, exactly
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_IsQuadDegenerate'>IsQuadDegenerate</a> <a href='#SkPath_IsCubicDegenerate'>IsCubicDegenerate</a>

<a name='SkPath_IsQuadDegenerate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkPath_IsQuadDegenerate'>IsQuadDegenerate</a>(<a href='#SkPath_IsQuadDegenerate'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p1</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p2</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p3</a>, <a href='SkPoint_Reference#SkPoint'>bool</a> <a href='SkPoint_Reference#SkPoint'>exact</a>)
</pre>

Tests if <a href='SkPath_Reference#Quad'>quad</a> <a href='SkPath_Reference#Quad'>is</a> <a href='SkPath_Reference#Quad'>degenerate</a>.
<a href='SkPath_Reference#Quad'>Quad</a> <a href='SkPath_Reference#Quad'>with</a> <a href='SkPath_Reference#Quad'>no</a> <a href='SkPath_Reference#Quad'>length</a> <a href='SkPath_Reference#Quad'>or</a> <a href='SkPath_Reference#Quad'>that</a> <a href='SkPath_Reference#Quad'>moves</a> <a href='SkPath_Reference#Quad'>a</a> <a href='SkPath_Reference#Quad'>very</a> <a href='SkPath_Reference#Quad'>short</a> <a href='SkPath_Reference#Quad'>distance</a> <a href='SkPath_Reference#Quad'>is</a> <a href='SkPath_Reference#Quad'>degenerate</a>; <a href='SkPath_Reference#Quad'>it</a> <a href='SkPath_Reference#Quad'>is</a>
treated as a <a href='SkPoint_Reference#Point'>point</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_IsQuadDegenerate_p1'><code><strong>p1</strong></code></a></td>
    <td><a href='SkPath_Reference#Quad'>quad</a> <a href='SkPath_Reference#Quad'>start</a> <a href='SkPoint_Reference#Point'>point</a></td>
  </tr>
  <tr>    <td><a name='SkPath_IsQuadDegenerate_p2'><code><strong>p2</strong></code></a></td>
    <td><a href='SkPath_Reference#Quad'>quad</a> <a href='SkPath_Reference#Quad'>control</a> <a href='SkPoint_Reference#Point'>point</a></td>
  </tr>
  <tr>    <td><a name='SkPath_IsQuadDegenerate_p3'><code><strong>p3</strong></code></a></td>
    <td><a href='SkPath_Reference#Quad'>quad</a> <a href='SkPath_Reference#Quad'>end</a> <a href='SkPoint_Reference#Point'>point</a></td>
  </tr>
  <tr>    <td><a name='SkPath_IsQuadDegenerate_exact'><code><strong>exact</strong></code></a></td>
    <td>if true, returns true only if <a href='#SkPath_IsQuadDegenerate_p1'>p1</a>, <a href='#SkPath_IsQuadDegenerate_p2'>p2</a>, <a href='#SkPath_IsQuadDegenerate_p2'>and</a> <a href='#SkPath_IsQuadDegenerate_p3'>p3</a> <a href='#SkPath_IsQuadDegenerate_p3'>are</a> <a href='#SkPath_IsQuadDegenerate_p3'>equal</a>;</td>
  </tr>
</table>

if false, returns true if <a href='#SkPath_IsQuadDegenerate_p1'>p1</a>, <a href='#SkPath_IsQuadDegenerate_p2'>p2</a>, <a href='#SkPath_IsQuadDegenerate_p2'>and</a> <a href='#SkPath_IsQuadDegenerate_p3'>p3</a> <a href='#SkPath_IsQuadDegenerate_p3'>are</a> <a href='#SkPath_IsQuadDegenerate_p3'>equal</a> <a href='#SkPath_IsQuadDegenerate_p3'>or</a> <a href='#SkPath_IsQuadDegenerate_p3'>nearly</a> <a href='#SkPath_IsQuadDegenerate_p3'>equal</a>

### Return Value

true if <a href='SkPath_Reference#Quad'>quad</a> <a href='SkPath_Reference#Quad'>is</a> <a href='SkPath_Reference#Quad'>degenerate</a>; <a href='SkPath_Reference#Quad'>its</a> <a href='SkPath_Reference#Quad'>length</a> <a href='SkPath_Reference#Quad'>is</a> <a href='SkPath_Reference#Quad'>effectively</a> <a href='SkPath_Reference#Quad'>zero</a>

### Example

<div><fiddle-embed name="a2b255a7dac1926cc3a247d318d63c62"><div>As single precision floats: 100, 100.00001, and 100.00002 have different bit representations
but nearly the same value. Translating all three by 1000 gives them the same bit representation;
the fractional portion of the number can not be represented by the float and is lost.
</div>

#### Example Output

~~~~
quad (100,100), (100.00001,100.00001), (100.00002,100.00002) is degenerate, nearly
quad (1100,1100), (1100,1100), (1100,1100) is degenerate, nearly
quad (100,100), (100.00001,100.00001), (100.00002,100.00002) is not degenerate, exactly
quad (1100,1100), (1100,1100), (1100,1100) is degenerate, exactly
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_IsLineDegenerate'>IsLineDegenerate</a> <a href='#SkPath_IsCubicDegenerate'>IsCubicDegenerate</a>

<a name='SkPath_IsCubicDegenerate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkPath_IsCubicDegenerate'>IsCubicDegenerate</a>(<a href='#SkPath_IsCubicDegenerate'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p1</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p2</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p3</a>,
                              <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p4</a>, <a href='SkPoint_Reference#SkPoint'>bool</a> <a href='SkPoint_Reference#SkPoint'>exact</a>)
</pre>

Tests if <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>is</a> <a href='SkPath_Reference#Cubic'>degenerate</a>.
<a href='SkPath_Reference#Cubic'>Cubic</a> <a href='SkPath_Reference#Cubic'>with</a> <a href='SkPath_Reference#Cubic'>no</a> <a href='SkPath_Reference#Cubic'>length</a> <a href='SkPath_Reference#Cubic'>or</a> <a href='SkPath_Reference#Cubic'>that</a> <a href='SkPath_Reference#Cubic'>moves</a> <a href='SkPath_Reference#Cubic'>a</a> <a href='SkPath_Reference#Cubic'>very</a> <a href='SkPath_Reference#Cubic'>short</a> <a href='SkPath_Reference#Cubic'>distance</a> <a href='SkPath_Reference#Cubic'>is</a> <a href='SkPath_Reference#Cubic'>degenerate</a>; <a href='SkPath_Reference#Cubic'>it</a> <a href='SkPath_Reference#Cubic'>is</a>
treated as a <a href='SkPoint_Reference#Point'>point</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_IsCubicDegenerate_p1'><code><strong>p1</strong></code></a></td>
    <td><a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>start</a> <a href='SkPoint_Reference#Point'>point</a></td>
  </tr>
  <tr>    <td><a name='SkPath_IsCubicDegenerate_p2'><code><strong>p2</strong></code></a></td>
    <td><a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>control</a> <a href='SkPoint_Reference#Point'>point</a> 1</td>
  </tr>
  <tr>    <td><a name='SkPath_IsCubicDegenerate_p3'><code><strong>p3</strong></code></a></td>
    <td><a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>control</a> <a href='SkPoint_Reference#Point'>point</a> 2</td>
  </tr>
  <tr>    <td><a name='SkPath_IsCubicDegenerate_p4'><code><strong>p4</strong></code></a></td>
    <td><a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>end</a> <a href='SkPoint_Reference#Point'>point</a></td>
  </tr>
  <tr>    <td><a name='SkPath_IsCubicDegenerate_exact'><code><strong>exact</strong></code></a></td>
    <td>if true, returns true only if <a href='#SkPath_IsCubicDegenerate_p1'>p1</a>, <a href='#SkPath_IsCubicDegenerate_p2'>p2</a>, <a href='#SkPath_IsCubicDegenerate_p3'>p3</a>, <a href='#SkPath_IsCubicDegenerate_p3'>and</a> <a href='#SkPath_IsCubicDegenerate_p4'>p4</a> <a href='#SkPath_IsCubicDegenerate_p4'>are</a> <a href='#SkPath_IsCubicDegenerate_p4'>equal</a>;</td>
  </tr>
</table>

if false, returns true if <a href='#SkPath_IsCubicDegenerate_p1'>p1</a>, <a href='#SkPath_IsCubicDegenerate_p2'>p2</a>, <a href='#SkPath_IsCubicDegenerate_p3'>p3</a>, <a href='#SkPath_IsCubicDegenerate_p3'>and</a> <a href='#SkPath_IsCubicDegenerate_p4'>p4</a> <a href='#SkPath_IsCubicDegenerate_p4'>are</a> <a href='#SkPath_IsCubicDegenerate_p4'>equal</a> <a href='#SkPath_IsCubicDegenerate_p4'>or</a> <a href='#SkPath_IsCubicDegenerate_p4'>nearly</a> <a href='#SkPath_IsCubicDegenerate_p4'>equal</a>

### Return Value

true if <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>is</a> <a href='SkPath_Reference#Cubic'>degenerate</a>; <a href='SkPath_Reference#Cubic'>its</a> <a href='SkPath_Reference#Cubic'>length</a> <a href='SkPath_Reference#Cubic'>is</a> <a href='SkPath_Reference#Cubic'>effectively</a> <a href='SkPath_Reference#Cubic'>zero</a>

### Example

<div><fiddle-embed name="6b97099acdae80b16df0c4241f593991">

#### Example Output

~~~~
0.00024414062 is degenerate
0.00024414065 is length
~~~~

</fiddle-embed></div>

<a name='SkPath_isLine'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isLine'>isLine</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='undocumented#Line'>line</a>[2]) <a href='undocumented#Line'>const</a>
</pre>

Returns true if <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>contains</a> <a href='SkPath_Reference#SkPath'>only</a> <a href='SkPath_Reference#SkPath'>one</a> <a href='#SkPath_isLine_line'>line</a>;
<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>array</a> <a href='#SkPath_Verb'>has</a> <a href='#SkPath_Verb'>two</a> <a href='#SkPath_Verb'>entries</a>: <a href='#SkPath_kMove_Verb'>kMove_Verb</a>, <a href='#SkPath_kLine_Verb'>kLine_Verb</a>.
If <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>contains</a> <a href='SkPath_Reference#SkPath'>one</a> <a href='#SkPath_isLine_line'>line</a> <a href='#SkPath_isLine_line'>and</a> <a href='#SkPath_isLine_line'>line</a> <a href='#SkPath_isLine_line'>is</a> <a href='#SkPath_isLine_line'>not</a> <a href='#SkPath_isLine_line'>nullptr</a>, <a href='#SkPath_isLine_line'>line</a> <a href='#SkPath_isLine_line'>is</a> <a href='#SkPath_isLine_line'>set</a> <a href='#SkPath_isLine_line'>to</a>
<a href='#SkPath_isLine_line'>line</a> <a href='#SkPath_isLine_line'>start</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>and</a> <a href='#SkPath_isLine_line'>line</a> <a href='#SkPath_isLine_line'>end</a> <a href='SkPoint_Reference#Point'>point</a>.
Returns false if <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>not</a> <a href='SkPath_Reference#SkPath'>one</a> <a href='#SkPath_isLine_line'>line</a>; <a href='#SkPath_isLine_line'>line</a> <a href='#SkPath_isLine_line'>is</a> <a href='#SkPath_isLine_line'>unaltered</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_isLine_line'><code><strong>line</strong></code></a></td>
    <td>storage for <a href='#SkPath_isLine_line'>line</a>. <a href='#SkPath_isLine_line'>May</a> <a href='#SkPath_isLine_line'>be</a> <a href='#SkPath_isLine_line'>nullptr</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>contains</a> <a href='SkPath_Reference#SkPath'>exactly</a> <a href='SkPath_Reference#SkPath'>one</a> <a href='#SkPath_isLine_line'>line</a>

### Example

<div><fiddle-embed name="1ad07d56e4258e041606d50cad969392">

#### Example Output

~~~~
empty is not line
zero line is line (0,0) (0,0)
line is line (10,10) (20,20)
second move is not line
~~~~

</fiddle-embed></div>

<a name='Point_Array'></a>

<a href='#Path_Point_Array'>Point_Array</a> <a href='#Path_Point_Array'>contains</a> <a href='SkPoint_Reference#Point'>Points</a> <a href='SkPoint_Reference#Point'>satisfying</a> <a href='SkPoint_Reference#Point'>the</a> <a href='SkPoint_Reference#Point'>allocated</a> <a href='SkPoint_Reference#Point'>Points</a> <a href='SkPoint_Reference#Point'>for</a>
<a href='SkPoint_Reference#Point'>each</a> <a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>in</a> <a href='#Path_Verb_Array'>Verb_Array</a>. <a href='#Path_Verb_Array'>For</a> <a href='#Path_Verb_Array'>instance</a>, <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>containing</a> <a href='SkPath_Reference#Path'>one</a> <a href='SkPath_Overview#Contour'>Contour</a> <a href='SkPath_Overview#Contour'>with</a> <a href='undocumented#Line'>Line</a>
<a href='undocumented#Line'>and</a> <a href='SkPath_Reference#Quad'>Quad</a> <a href='SkPath_Reference#Quad'>is</a> <a href='SkPath_Reference#Quad'>described</a> <a href='SkPath_Reference#Quad'>by</a> <a href='#Path_Verb_Array'>Verb_Array</a>: <a href='#SkPath_kMove_Verb'>kMove_Verb</a>, <a href='#SkPath_kLine_Verb'>kLine_Verb</a>, <a href='#SkPath_kQuad_Verb'>kQuad_Verb</a>; <a href='#SkPath_kQuad_Verb'>and</a>
<a href='#SkPath_kQuad_Verb'>one</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>for</a> <a href='SkPoint_Reference#Point'>move</a>, <a href='SkPoint_Reference#Point'>one</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>for</a> <a href='undocumented#Line'>Line</a>, <a href='undocumented#Line'>two</a> <a href='SkPoint_Reference#Point'>Points</a> <a href='SkPoint_Reference#Point'>for</a> <a href='SkPath_Reference#Quad'>Quad</a>; <a href='SkPath_Reference#Quad'>totaling</a> <a href='SkPath_Reference#Quad'>four</a> <a href='SkPoint_Reference#Point'>Points</a>.

<a href='#Path_Point_Array'>Point_Array</a> <a href='#Path_Point_Array'>may</a> <a href='#Path_Point_Array'>be</a> <a href='#Path_Point_Array'>read</a> <a href='#Path_Point_Array'>directly</a> <a href='#Path_Point_Array'>from</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>with</a> <a href='#SkPath_getPoints'>getPoints</a>, <a href='#SkPath_getPoints'>or</a> <a href='#SkPath_getPoints'>inspected</a> <a href='#SkPath_getPoints'>with</a>
<a href='#SkPath_getPoint'>getPoint</a>, <a href='#SkPath_getPoint'>with</a> <a href='#SkPath_Iter'>Iter</a>, <a href='#SkPath_Iter'>or</a> <a href='#SkPath_Iter'>with</a> <a href='#SkPath_RawIter'>RawIter</a>.

<a name='SkPath_getPoints'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPath_getPoints'>getPoints</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#Point'>points</a>[], <a href='SkPoint_Reference#Point'>int</a> <a href='SkPoint_Reference#Point'>max</a>) <a href='SkPoint_Reference#Point'>const</a>
</pre>

Returns number of <a href='#SkPath_getPoints_points'>points</a> <a href='#SkPath_getPoints_points'>in</a> <a href='SkPath_Reference#SkPath'>SkPath</a>. <a href='SkPath_Reference#SkPath'>Up</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='#SkPath_getPoints_max'>max</a> <a href='#SkPath_getPoints_points'>points</a> <a href='#SkPath_getPoints_points'>are</a> <a href='#SkPath_getPoints_points'>copied</a>.
<a href='#SkPath_getPoints_points'>points</a> <a href='#SkPath_getPoints_points'>may</a> <a href='#SkPath_getPoints_points'>be</a> <a href='#SkPath_getPoints_points'>nullptr</a>; <a href='#SkPath_getPoints_points'>then</a>, <a href='#SkPath_getPoints_max'>max</a> <a href='#SkPath_getPoints_max'>must</a> <a href='#SkPath_getPoints_max'>be</a> <a href='#SkPath_getPoints_max'>zero</a>.
If <a href='#SkPath_getPoints_max'>max</a> <a href='#SkPath_getPoints_max'>is</a> <a href='#SkPath_getPoints_max'>greater</a> <a href='#SkPath_getPoints_max'>than</a> <a href='#SkPath_getPoints_max'>number</a> <a href='#SkPath_getPoints_max'>of</a> <a href='#SkPath_getPoints_points'>points</a>, <a href='#SkPath_getPoints_points'>excess</a> <a href='#SkPath_getPoints_points'>points</a> <a href='#SkPath_getPoints_points'>storage</a> <a href='#SkPath_getPoints_points'>is</a> <a href='#SkPath_getPoints_points'>unaltered</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_getPoints_points'><code><strong>points</strong></code></a></td>
    <td>storage for <a href='SkPath_Reference#SkPath'>SkPath</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>. <a href='SkPoint_Reference#SkPoint'>May</a> <a href='SkPoint_Reference#SkPoint'>be</a> <a href='SkPoint_Reference#SkPoint'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkPath_getPoints_max'><code><strong>max</strong></code></a></td>
    <td>maximum to copy; must be greater than or equal to zero</td>
  </tr>
</table>

### Return Value

<a href='SkPath_Reference#SkPath'>SkPath</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>length</a>

### Example

<div><fiddle-embed name="9bc86efda08cbcd9c6f7c5f220294a24">

#### Example Output

~~~~
no points point count: 3
zero max point count: 3
too small point count: 3  (0,0) (20,20)
just right point count: 3  (0,0) (20,20) (-10,-10)
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_countPoints'>countPoints</a> <a href='#SkPath_getPoint'>getPoint</a>

<a name='SkPath_countPoints'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPath_countPoints'>countPoints</a>() <a href='#SkPath_countPoints'>const</a>
</pre>

Returns the number of <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>in</a> <a href='SkPath_Reference#SkPath'>SkPath</a>.
<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>count</a> <a href='SkPoint_Reference#SkPoint'>is</a> <a href='SkPoint_Reference#SkPoint'>initially</a> <a href='SkPoint_Reference#SkPoint'>zero</a>.

### Return Value

<a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>array</a> <a href='SkPoint_Reference#SkPoint'>length</a>

### Example

<div><fiddle-embed name="bca6379ccef62cb081b10db7381deb27">

#### Example Output

~~~~
empty point count: 0
zero line point count: 2
line point count: 2
second move point count: 3
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_getPoints'>getPoints</a>

<a name='SkPath_getPoint'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkPath_getPoint'>getPoint</a>(<a href='#SkPath_getPoint'>int</a> <a href='#SkPath_getPoint'>index</a>) <a href='#SkPath_getPoint'>const</a>
</pre>

Returns <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>at</a> <a href='#SkPath_getPoint_index'>index</a> <a href='#SkPath_getPoint_index'>in</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>. <a href='SkPoint_Reference#SkPoint'>Valid</a> <a href='SkPoint_Reference#SkPoint'>range</a> <a href='SkPoint_Reference#SkPoint'>for</a> <a href='#SkPath_getPoint_index'>index</a> <a href='#SkPath_getPoint_index'>is</a>
0 to <a href='#SkPath_countPoints'>countPoints</a>() - 1.
Returns (0, 0) if <a href='#SkPath_getPoint_index'>index</a> <a href='#SkPath_getPoint_index'>is</a> <a href='#SkPath_getPoint_index'>out</a> <a href='#SkPath_getPoint_index'>of</a> <a href='#SkPath_getPoint_index'>range</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_getPoint_index'><code><strong>index</strong></code></a></td>
    <td><a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>element</a> <a href='SkPoint_Reference#SkPoint'>selector</a></td>
  </tr>
</table>

### Return Value

<a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>value</a> <a href='SkPoint_Reference#SkPoint'>or</a> (0, 0)

### Example

<div><fiddle-embed name="42885f1df13de109adccc5d531f62111">

#### Example Output

~~~~
point 0: (-10,-10)
point 1: (10,10)
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_countPoints'>countPoints</a> <a href='#SkPath_getPoints'>getPoints</a>

<a name='Verb_Array'></a>

<a href='#Path_Verb_Array'>Verb_Array</a> <a href='#Path_Verb_Array'>always</a> <a href='#Path_Verb_Array'>starts</a> <a href='#Path_Verb_Array'>with</a> <a href='#SkPath_kMove_Verb'>kMove_Verb</a>.
<a href='#SkPath_kMove_Verb'>If</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a> <a href='#SkPath_kClose_Verb'>is</a> <a href='#SkPath_kClose_Verb'>not</a> <a href='#SkPath_kClose_Verb'>the</a> <a href='#SkPath_kClose_Verb'>last</a> <a href='#SkPath_kClose_Verb'>entry</a>, <a href='#SkPath_kClose_Verb'>it</a> <a href='#SkPath_kClose_Verb'>is</a> <a href='#SkPath_kClose_Verb'>always</a> <a href='#SkPath_kClose_Verb'>followed</a> <a href='#SkPath_kClose_Verb'>by</a> <a href='#SkPath_kMove_Verb'>kMove_Verb</a>;
<a href='#SkPath_kMove_Verb'>the</a> <a href='#SkPath_kMove_Verb'>quantity</a> <a href='#SkPath_kMove_Verb'>of</a> <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>equals</a> <a href='#SkPath_kMove_Verb'>the</a> <a href='SkPath_Overview#Contour'>Contour</a> <a href='SkPath_Overview#Contour'>count</a>.
<a href='#Path_Verb_Array'>Verb_Array</a> <a href='#Path_Verb_Array'>does</a> <a href='#Path_Verb_Array'>not</a> <a href='#Path_Verb_Array'>include</a> <a href='#Path_Verb_Array'>or</a> <a href='#Path_Verb_Array'>count</a> <a href='#SkPath_kDone_Verb'>kDone_Verb</a>; <a href='#SkPath_kDone_Verb'>it</a> <a href='#SkPath_kDone_Verb'>is</a> <a href='#SkPath_kDone_Verb'>a</a> <a href='#SkPath_kDone_Verb'>convenience</a>
<a href='#SkPath_kDone_Verb'>returned</a> <a href='#SkPath_kDone_Verb'>when</a> <a href='#SkPath_kDone_Verb'>iterating</a> <a href='#SkPath_kDone_Verb'>through</a> <a href='#Path_Verb_Array'>Verb_Array</a>.

<a href='#Path_Verb_Array'>Verb_Array</a> <a href='#Path_Verb_Array'>may</a> <a href='#Path_Verb_Array'>be</a> <a href='#Path_Verb_Array'>read</a> <a href='#Path_Verb_Array'>directly</a> <a href='#Path_Verb_Array'>from</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>with</a> <a href='#SkPath_getVerbs'>getVerbs</a>, <a href='#SkPath_getVerbs'>or</a> <a href='#SkPath_getVerbs'>inspected</a> <a href='#SkPath_getVerbs'>with</a> <a href='#SkPath_Iter'>Iter</a>,
<a href='#SkPath_Iter'>or</a> <a href='#SkPath_Iter'>with</a> <a href='#SkPath_RawIter'>RawIter</a>.

<a name='SkPath_countVerbs'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPath_countVerbs'>countVerbs</a>() <a href='#SkPath_countVerbs'>const</a>
</pre>

Returns the number of <a href='SkPath_Reference#Verb'>verbs</a>: <a href='#SkPath_kMove_Verb'>kMove_Verb</a>, <a href='#SkPath_kLine_Verb'>kLine_Verb</a>, <a href='#SkPath_kQuad_Verb'>kQuad_Verb</a>, <a href='#SkPath_kConic_Verb'>kConic_Verb</a>,
<a href='#SkPath_kCubic_Verb'>kCubic_Verb</a>, <a href='#SkPath_kCubic_Verb'>and</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a>; <a href='#SkPath_kClose_Verb'>added</a> <a href='#SkPath_kClose_Verb'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>.

### Return Value

length of verb array

### Example

<div><fiddle-embed name="af0c66aea3ef81b709664c7007f48aae">

#### Example Output

~~~~
empty verb count: 0
round rect verb count: 10
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_getVerbs'>getVerbs</a> <a href='#SkPath_Iter'>Iter</a> <a href='#SkPath_RawIter'>RawIter</a>

<a name='SkPath_getVerbs'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPath_getVerbs'>getVerbs</a>(<a href='#SkPath_getVerbs'>uint8_t</a> <a href='SkPath_Reference#Verb'>verbs</a>[], <a href='SkPath_Reference#Verb'>int</a> <a href='SkPath_Reference#Verb'>max</a>) <a href='SkPath_Reference#Verb'>const</a>
</pre>

Returns the number of <a href='#SkPath_getVerbs_verbs'>verbs</a> <a href='#SkPath_getVerbs_verbs'>in</a> <a href='#SkPath_getVerbs_verbs'>the</a> <a href='SkPath_Reference#Path'>path</a>. <a href='SkPath_Reference#Path'>Up</a> <a href='SkPath_Reference#Path'>to</a> <a href='#SkPath_getVerbs_max'>max</a> <a href='#SkPath_getVerbs_verbs'>verbs</a> <a href='#SkPath_getVerbs_verbs'>are</a> <a href='#SkPath_getVerbs_verbs'>copied</a>. <a href='#SkPath_getVerbs_verbs'>The</a>
<a href='#SkPath_getVerbs_verbs'>verbs</a> <a href='#SkPath_getVerbs_verbs'>are</a> <a href='#SkPath_getVerbs_verbs'>copied</a> <a href='#SkPath_getVerbs_verbs'>as</a> <a href='#SkPath_getVerbs_verbs'>one</a> <a href='#SkPath_getVerbs_verbs'>byte</a> <a href='#SkPath_getVerbs_verbs'>per</a> <a href='#SkPath_getVerbs_verbs'>verb</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_getVerbs_verbs'><code><strong>verbs</strong></code></a></td>
    <td>storage for <a href='#SkPath_getVerbs_verbs'>verbs</a>, <a href='#SkPath_getVerbs_verbs'>may</a> <a href='#SkPath_getVerbs_verbs'>be</a> <a href='#SkPath_getVerbs_verbs'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkPath_getVerbs_max'><code><strong>max</strong></code></a></td>
    <td>maximum number to copy into <a href='#SkPath_getVerbs_verbs'>verbs</a></td>
  </tr>
</table>

### Return Value

the actual number of <a href='#SkPath_getVerbs_verbs'>verbs</a> <a href='#SkPath_getVerbs_verbs'>in</a> <a href='#SkPath_getVerbs_verbs'>the</a> <a href='SkPath_Reference#Path'>path</a>

### Example

<div><fiddle-embed name="2ec66880966a6133ddd9331ce7323438">

#### Example Output

~~~~
no verbs verb count: 3
zero max verb count: 3
too small verb count: 3  move line
just right verb count: 3  move line line
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_countVerbs'>countVerbs</a> <a href='#SkPath_getPoints'>getPoints</a> <a href='#SkPath_Iter'>Iter</a> <a href='#SkPath_RawIter'>RawIter</a>

<a name='SkPath_swap'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_swap'>swap</a>(<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>other</a>)
</pre>

Exchanges the  <a href='#Verb_Array'>verb array</a>,  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>, <a href='SkPath_Reference#Conic_Weight'>weights</a>, <a href='SkPath_Reference#Conic_Weight'>and</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_FillType'>with</a> <a href='#SkPath_swap_other'>other</a>.
Cached state is also exchanged. <a href='#SkPath_swap'>swap()</a> <a href='#SkPath_swap'>internally</a> <a href='#SkPath_swap'>exchanges</a> <a href='#SkPath_swap'>pointers</a>, <a href='#SkPath_swap'>so</a>
it is lightweight and does not allocate memory.

<a href='#SkPath_swap'>swap()</a> <a href='#SkPath_swap'>usage</a> <a href='#SkPath_swap'>has</a> <a href='#SkPath_swap'>largely</a> <a href='#SkPath_swap'>been</a> <a href='#SkPath_swap'>replaced</a> <a href='#SkPath_swap'>by</a> by<a href='#SkPath_copy_operator'>operator=(const SkPath& path)</a>.
<a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>do</a> <a href='SkPath_Reference#SkPath'>not</a> <a href='SkPath_Reference#SkPath'>copy</a> <a href='SkPath_Reference#SkPath'>their</a> <a href='SkPath_Reference#SkPath'>content</a> <a href='SkPath_Reference#SkPath'>on</a> <a href='SkPath_Reference#SkPath'>assignment</a> <a href='SkPath_Reference#SkPath'>until</a> <a href='SkPath_Reference#SkPath'>they</a> <a href='SkPath_Reference#SkPath'>are</a> <a href='SkPath_Reference#SkPath'>written</a> <a href='SkPath_Reference#SkPath'>to</a>,
making assignment as efficient as <a href='#SkPath_swap'>swap()</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_swap_other'><code><strong>other</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>exchanged</a> <a href='SkPath_Reference#SkPath'>by</a> <a href='SkPath_Reference#SkPath'>value</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4c5ebee2b5039e5faefa07ae63a15467">

#### Example Output

~~~~
path1 bounds = 0, 0, 0, 0
path2 bounds = 10, 20, 30, 40
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_copy_operator'>operator=(const SkPath& path)</a>

<a name='SkPath_getBounds'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkPath_getBounds'>getBounds</a>() <a href='#SkPath_getBounds'>const</a>
</pre>

Returns minimum and maximum axes values of <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>array</a>.
Returns (0, 0, 0, 0) if <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>contains</a> <a href='SkPath_Reference#SkPath'>no</a> <a href='SkPoint_Reference#Point'>points</a>. <a href='SkPoint_Reference#Point'>Returned</a> <a href='SkPoint_Reference#Point'>bounds</a> <a href='SkPoint_Reference#Point'>width</a> <a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Point'>height</a> <a href='SkPoint_Reference#Point'>may</a>
be larger or smaller than area affected when <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>drawn</a>.

<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>returned</a> <a href='SkRect_Reference#SkRect'>includes</a> <a href='SkRect_Reference#SkRect'>all</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>added</a> <a href='SkPoint_Reference#SkPoint'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>, <a href='SkPath_Reference#SkPath'>including</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>associated</a> <a href='SkPoint_Reference#SkPoint'>with</a>
<a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>that</a> <a href='#SkPath_kMove_Verb'>define</a> <a href='#SkPath_kMove_Verb'>empty</a> <a href='SkPath_Overview#Contour'>contours</a>.

### Return Value

bounds of all <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>in</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>array</a>

### Example

<div><fiddle-embed name="45c0fc3acb74fab99d544b80eadd10ad"><div>Bounds of upright <a href='undocumented#Circle'>Circle</a> <a href='undocumented#Circle'>can</a> <a href='undocumented#Circle'>be</a> <a href='undocumented#Circle'>predicted</a> <a href='undocumented#Circle'>from</a> <a href='undocumented#Circle'>center</a> <a href='undocumented#Circle'>and</a> <a href='undocumented#Circle'>radius</a>.
<a href='undocumented#Circle'>Bounds</a> <a href='undocumented#Circle'>of</a> <a href='undocumented#Circle'>rotated</a> <a href='undocumented#Circle'>Circle</a> <a href='undocumented#Circle'>includes</a> <a href='undocumented#Circle'>control</a> <a href='SkPoint_Reference#Point'>Points</a> <a href='SkPoint_Reference#Point'>outside</a> <a href='SkPoint_Reference#Point'>of</a> <a href='SkPoint_Reference#Point'>filled</a> <a href='SkPoint_Reference#Point'>area</a>.
</div>

#### Example Output

~~~~
empty bounds = 0, 0, 0, 0
circle bounds = 25, 20, 75, 70
rotated circle bounds = 14.6447, 9.64466, 85.3553, 80.3553
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_computeTightBounds'>computeTightBounds</a> <a href='#SkPath_updateBoundsCache'>updateBoundsCache</a>

<a name='Utility'></a>

<a name='SkPath_updateBoundsCache'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_updateBoundsCache'>updateBoundsCache</a>() <a href='#SkPath_updateBoundsCache'>const</a>
</pre>

Updates internal bounds so that subsequent calls to <a href='#SkPath_getBounds'>getBounds</a>() <a href='#SkPath_getBounds'>are</a> <a href='#SkPath_getBounds'>instantaneous</a>.
Unaltered copies of <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>may</a> <a href='SkPath_Reference#SkPath'>also</a> <a href='SkPath_Reference#SkPath'>access</a> <a href='SkPath_Reference#SkPath'>cached</a> <a href='SkPath_Reference#SkPath'>bounds</a> <a href='SkPath_Reference#SkPath'>through</a> <a href='#SkPath_getBounds'>getBounds</a>().

For now, identical to calling <a href='#SkPath_getBounds'>getBounds</a>() <a href='#SkPath_getBounds'>and</a> <a href='#SkPath_getBounds'>ignoring</a> <a href='#SkPath_getBounds'>the</a> <a href='#SkPath_getBounds'>returned</a> <a href='#SkPath_getBounds'>value</a>.

Call to prepare <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>subsequently</a> <a href='SkPath_Reference#SkPath'>drawn</a> <a href='SkPath_Reference#SkPath'>from</a> <a href='SkPath_Reference#SkPath'>multiple</a> <a href='SkPath_Reference#SkPath'>threads</a>,
to avoid a race condition where each draw separately computes the bounds.

### Example

<div><fiddle-embed name="bb761cd858e6d0ca05627262cd22ff5e">

#### Example Output

~~~~
#Volatile
uncached avg: 0.18048 ms
cached avg: 0.182784 ms
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_getBounds'>getBounds</a>

<a name='SkPath_computeTightBounds'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkPath_computeTightBounds'>computeTightBounds</a>() <a href='#SkPath_computeTightBounds'>const</a>
</pre>

Returns minimum and maximum axes values of the <a href='undocumented#Line'>lines</a> <a href='undocumented#Line'>and</a> <a href='undocumented#Curve'>curves</a> <a href='undocumented#Curve'>in</a> <a href='SkPath_Reference#SkPath'>SkPath</a>.
Returns (0, 0, 0, 0) if <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>contains</a> <a href='SkPath_Reference#SkPath'>no</a> <a href='SkPoint_Reference#Point'>points</a>.
Returned bounds width and height may be larger or smaller than area affected
when <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>drawn</a>.

Includes <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>associated</a> <a href='SkPoint_Reference#SkPoint'>with</a> <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>that</a> <a href='#SkPath_kMove_Verb'>define</a> <a href='#SkPath_kMove_Verb'>empty</a>
<a href='SkPath_Overview#Contour'>contours</a>.

Behaves identically to <a href='#SkPath_getBounds'>getBounds</a>() <a href='#SkPath_getBounds'>when</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>contains</a>
only <a href='undocumented#Line'>lines</a>. <a href='undocumented#Line'>If</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>contains</a> <a href='undocumented#Curve'>curves</a>, <a href='undocumented#Curve'>computed</a> <a href='undocumented#Curve'>bounds</a> <a href='undocumented#Curve'>includes</a>
the maximum extent of the <a href='SkPath_Reference#Quad'>quad</a>, <a href='SkPath_Reference#Conic'>conic</a>, <a href='SkPath_Reference#Conic'>or</a> <a href='SkPath_Reference#Cubic'>cubic</a>; <a href='SkPath_Reference#Cubic'>is</a> <a href='SkPath_Reference#Cubic'>slower</a> <a href='SkPath_Reference#Cubic'>than</a> <a href='#SkPath_getBounds'>getBounds</a>();
and unlike <a href='#SkPath_getBounds'>getBounds</a>(), <a href='#SkPath_getBounds'>does</a> <a href='#SkPath_getBounds'>not</a> <a href='#SkPath_getBounds'>cache</a> <a href='#SkPath_getBounds'>the</a> <a href='#SkPath_getBounds'>result</a>.

### Return Value

tight bounds of <a href='undocumented#Curve'>curves</a> <a href='undocumented#Curve'>in</a> <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="9a39c56e95b19a657133b7ad1fe0cf03">

#### Example Output

~~~~
empty bounds = 0, 0, 0, 0
circle bounds = 25, 20, 75, 70
rotated circle bounds = 25, 20, 75, 70
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_getBounds'>getBounds</a>

<a name='SkPath_conservativelyContainsRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_conservativelyContainsRect'>conservativelyContainsRect</a>(<a href='#SkPath_conservativelyContainsRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>) <a href='SkRect_Reference#Rect'>const</a>
</pre>

Returns true if <a href='#SkPath_conservativelyContainsRect_rect'>rect</a> <a href='#SkPath_conservativelyContainsRect_rect'>is</a> <a href='#SkPath_conservativelyContainsRect_rect'>contained</a> <a href='#SkPath_conservativelyContainsRect_rect'>by</a> <a href='SkPath_Reference#SkPath'>SkPath</a>.
May return false when <a href='#SkPath_conservativelyContainsRect_rect'>rect</a> <a href='#SkPath_conservativelyContainsRect_rect'>is</a> <a href='#SkPath_conservativelyContainsRect_rect'>contained</a> <a href='#SkPath_conservativelyContainsRect_rect'>by</a> <a href='SkPath_Reference#SkPath'>SkPath</a>.

For now, only returns true if <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>has</a> <a href='SkPath_Reference#SkPath'>one</a> <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>and</a> <a href='SkPath_Overview#Contour'>is</a> <a href='SkPath_Overview#Contour'>convex</a>.
<a href='#SkPath_conservativelyContainsRect_rect'>rect</a> <a href='#SkPath_conservativelyContainsRect_rect'>may</a> <a href='#SkPath_conservativelyContainsRect_rect'>share</a> <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Point'>edges</a> <a href='SkPoint_Reference#Point'>with</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>and</a> <a href='SkPath_Reference#SkPath'>be</a> <a href='SkPath_Reference#SkPath'>contained</a>.
Returns true if <a href='#SkPath_conservativelyContainsRect_rect'>rect</a> <a href='#SkPath_conservativelyContainsRect_rect'>is</a> <a href='#SkPath_conservativelyContainsRect_rect'>empty</a>, <a href='#SkPath_conservativelyContainsRect_rect'>that</a> <a href='#SkPath_conservativelyContainsRect_rect'>is</a>, <a href='#SkPath_conservativelyContainsRect_rect'>it</a> <a href='#SkPath_conservativelyContainsRect_rect'>has</a> <a href='#SkPath_conservativelyContainsRect_rect'>zero</a> <a href='#SkPath_conservativelyContainsRect_rect'>width</a> <a href='#SkPath_conservativelyContainsRect_rect'>or</a> <a href='#SkPath_conservativelyContainsRect_rect'>height</a>; <a href='#SkPath_conservativelyContainsRect_rect'>and</a>
the <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>or</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>described</a> <a href='undocumented#Line'>by</a> <a href='#SkPath_conservativelyContainsRect_rect'>rect</a> <a href='#SkPath_conservativelyContainsRect_rect'>is</a> <a href='#SkPath_conservativelyContainsRect_rect'>contained</a> <a href='#SkPath_conservativelyContainsRect_rect'>by</a> <a href='SkPath_Reference#SkPath'>SkPath</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_conservativelyContainsRect_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a>, <a href='undocumented#Line'>line</a>, <a href='undocumented#Line'>or</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>checked</a> <a href='SkPoint_Reference#SkPoint'>for</a> <a href='SkPoint_Reference#SkPoint'>containment</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkPath_conservativelyContainsRect_rect'>rect</a> <a href='#SkPath_conservativelyContainsRect_rect'>is</a> <a href='#SkPath_conservativelyContainsRect_rect'>contained</a>

### Example

<div><fiddle-embed name="41638d13e40fa449ece354dde5fb1941"><div><a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>is</a> <a href='SkRect_Reference#Rect'>drawn</a> <a href='SkRect_Reference#Rect'>in</a> <a href='SkRect_Reference#Rect'>blue</a> <a href='SkRect_Reference#Rect'>if</a> <a href='SkRect_Reference#Rect'>it</a> <a href='SkRect_Reference#Rect'>is</a> <a href='SkRect_Reference#Rect'>contained</a> <a href='SkRect_Reference#Rect'>by</a> <a href='SkRect_Reference#Rect'>red</a> <a href='SkPath_Reference#Path'>Path</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_contains'>contains</a> <a href='undocumented#Op'>Op</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkPath_Convexity'>Convexity</a>

<a name='SkPath_incReserve'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_incReserve'>incReserve</a>(<a href='#SkPath_incReserve'>int</a> <a href='#SkPath_incReserve'>extraPtCount</a>)
</pre>

Grows <a href='SkPath_Reference#SkPath'>SkPath</a>  <a href='#Verb_Array'>verb array</a> <a href='SkPath_Reference#SkPath'>and</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>to</a> <a href='SkPoint_Reference#SkPoint'>contain</a> <a href='#SkPath_incReserve_extraPtCount'>extraPtCount</a> <a href='#SkPath_incReserve_extraPtCount'>additional</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>.
May improve performance and use less memory by
reducing the number and <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Size'>allocations</a> <a href='undocumented#Size'>when</a> <a href='undocumented#Size'>creating</a> <a href='SkPath_Reference#SkPath'>SkPath</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_incReserve_extraPtCount'><code><strong>extraPtCount</strong></code></a></td>
    <td>number of additional <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>to</a> <a href='SkPoint_Reference#SkPoint'>allocate</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f2260f2a170a54aef5bafe5b91c121b3"></fiddle-embed></div>

### See Also

<a href='#Path_Point_Array'>Point_Array</a>

<a name='SkPath_shrinkToFit'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_shrinkToFit'>shrinkToFit</a>()
</pre>

Shrinks <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>verb</a> <a href='SkPath_Reference#SkPath'>array</a> <a href='SkPath_Reference#SkPath'>and</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>array</a> <a href='SkPoint_Reference#SkPoint'>storage</a> <a href='SkPoint_Reference#SkPoint'>to</a> <a href='SkPoint_Reference#SkPoint'>discard</a> <a href='SkPoint_Reference#SkPoint'>unused</a> <a href='SkPoint_Reference#SkPoint'>capacity</a>.
May reduce the heap overhead for <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>known</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>be</a> <a href='SkPath_Reference#SkPath'>fully</a> <a href='SkPath_Reference#SkPath'>constructed</a>.

### See Also

<a href='#SkPath_incReserve'>incReserve</a>

<a name='Build'></a>

<a name='SkPath_moveTo'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_moveTo'>moveTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>)
</pre>

Adds beginning of <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>at</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> (<a href='#SkPath_moveTo_x'>x</a>, <a href='#SkPath_moveTo_y'>y</a>).

### Parameters

<table>  <tr>    <td><a name='SkPath_moveTo_x'><code><strong>x</strong></code></a></td>
    <td>x-axis value of <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>start</a></td>
  </tr>
  <tr>    <td><a name='SkPath_moveTo_y'><code><strong>y</strong></code></a></td>
    <td>y-axis value of <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>start</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="84101d341e934a535a41ad6cf42218ce"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_lineTo'>lineTo</a> <a href='#SkPath_rMoveTo'>rMoveTo</a> <a href='#SkPath_quadTo'>quadTo</a> <a href='#SkPath_conicTo'>conicTo</a> <a href='#SkPath_cubicTo'>cubicTo</a> <a href='#SkPath_close'>close()</a>

<a name='SkPath_moveTo_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_moveTo'>moveTo</a>(<a href='#SkPath_moveTo'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p</a>)
</pre>

Adds beginning of <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>at</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkPath_moveTo_2_p'>p</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_moveTo_2_p'><code><strong>p</strong></code></a></td>
    <td><a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>start</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="cb8d37990f6e7df3bcc85e7240c81274"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_lineTo'>lineTo</a> <a href='#SkPath_rMoveTo'>rMoveTo</a> <a href='#SkPath_quadTo'>quadTo</a> <a href='#SkPath_conicTo'>conicTo</a> <a href='#SkPath_cubicTo'>cubicTo</a> <a href='#SkPath_close'>close()</a>

<a name='SkPath_rMoveTo'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_rMoveTo'>rMoveTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>)
</pre>

Adds beginning of <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>relative</a> <a href='SkPath_Overview#Contour'>to</a>  <a href='#Last_Point'>last point</a>.
If <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>empty</a>, <a href='SkPath_Reference#SkPath'>starts</a> <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>at</a> (<a href='#SkPath_rMoveTo_dx'>dx</a>, <a href='#SkPath_rMoveTo_dy'>dy</a>).
Otherwise, start <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>at</a>  <a href='#Last_Point'>last point</a> <a href='SkPath_Overview#Contour'>offset</a> <a href='SkPath_Overview#Contour'>by</a> (<a href='#SkPath_rMoveTo_dx'>dx</a>, <a href='#SkPath_rMoveTo_dy'>dy</a>).
Function name stands for "relative move to".

### Parameters

<table>  <tr>    <td><a name='SkPath_rMoveTo_dx'><code><strong>dx</strong></code></a></td>
    <td>offset from  <a href='#Last_Point'>last point</a> to <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>start</a> <a href='SkPath_Overview#Contour'>on</a> <a href='SkPath_Overview#Contour'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_rMoveTo_dy'><code><strong>dy</strong></code></a></td>
    <td>offset from  <a href='#Last_Point'>last point</a> to <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>start</a> <a href='SkPath_Overview#Contour'>on</a> <a href='SkPath_Overview#Contour'>y-axis</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="63e32dec4b2d8440b427f368bf8313a4"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_lineTo'>lineTo</a> <a href='#SkPath_moveTo'>moveTo</a> <a href='#SkPath_quadTo'>quadTo</a> <a href='#SkPath_conicTo'>conicTo</a> <a href='#SkPath_cubicTo'>cubicTo</a> <a href='#SkPath_close'>close()</a>

<a name='SkPath_lineTo'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_lineTo'>lineTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>)
</pre>

Adds <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>from</a>  <a href='#Last_Point'>last point</a> <a href='undocumented#Line'>to</a> (<a href='#SkPath_lineTo_x'>x</a>, <a href='#SkPath_lineTo_y'>y</a>). <a href='#SkPath_lineTo_y'>If</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>empty</a>, <a href='SkPath_Reference#SkPath'>or</a> <a href='SkPath_Reference#SkPath'>last</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>is</a>
<a href='#SkPath_kClose_Verb'>kClose_Verb</a>,  <a href='#Last_Point'>last point</a> <a href='#SkPath_kClose_Verb'>is</a> <a href='#SkPath_kClose_Verb'>set</a> <a href='#SkPath_kClose_Verb'>to</a> (0, 0) <a href='#SkPath_kClose_Verb'>before</a> <a href='#SkPath_kClose_Verb'>adding</a> <a href='undocumented#Line'>line</a>.

<a href='#SkPath_lineTo'>lineTo</a>() <a href='#SkPath_lineTo'>appends</a> <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>to</a>  <a href='#Verb_Array'>verb array</a> <a href='#SkPath_kMove_Verb'>and</a> (0, 0) <a href='#SkPath_kMove_Verb'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>, <a href='SkPoint_Reference#SkPoint'>if</a> <a href='SkPoint_Reference#SkPoint'>needed</a>.
<a href='#SkPath_lineTo'>lineTo</a>() <a href='#SkPath_lineTo'>then</a> <a href='#SkPath_lineTo'>appends</a> <a href='#SkPath_kLine_Verb'>kLine_Verb</a> <a href='#SkPath_kLine_Verb'>to</a>  <a href='#Verb_Array'>verb array</a> <a href='#SkPath_kLine_Verb'>and</a> (<a href='#SkPath_lineTo_x'>x</a>, <a href='#SkPath_lineTo_y'>y</a>) <a href='#SkPath_lineTo_y'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_lineTo_x'><code><strong>x</strong></code></a></td>
    <td>end of added <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>on</a> <a href='undocumented#Line'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_lineTo_y'><code><strong>y</strong></code></a></td>
    <td>end of added <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>on</a> <a href='undocumented#Line'>y-axis</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="e311cdd451edacec33b50cc22a4dd5dc"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a> <a href='#SkPath_rLineTo'>rLineTo</a> <a href='#SkPath_addRect'>addRect</a>

<a name='SkPath_lineTo_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_lineTo'>lineTo</a>(<a href='#SkPath_lineTo'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p</a>)
</pre>

Adds <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>from</a>  <a href='#Last_Point'>last point</a> <a href='undocumented#Line'>to</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkPath_lineTo_2_p'>p</a>. <a href='#SkPath_lineTo_2_p'>If</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>empty</a>, <a href='SkPath_Reference#SkPath'>or</a> <a href='SkPath_Reference#SkPath'>last</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>is</a>
<a href='#SkPath_kClose_Verb'>kClose_Verb</a>,  <a href='#Last_Point'>last point</a> <a href='#SkPath_kClose_Verb'>is</a> <a href='#SkPath_kClose_Verb'>set</a> <a href='#SkPath_kClose_Verb'>to</a> (0, 0) <a href='#SkPath_kClose_Verb'>before</a> <a href='#SkPath_kClose_Verb'>adding</a> <a href='undocumented#Line'>line</a>.

<a href='#SkPath_lineTo'>lineTo</a>() <a href='#SkPath_lineTo'>first</a> <a href='#SkPath_lineTo'>appends</a> <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>to</a>  <a href='#Verb_Array'>verb array</a> <a href='#SkPath_kMove_Verb'>and</a> (0, 0) <a href='#SkPath_kMove_Verb'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>, <a href='SkPoint_Reference#SkPoint'>if</a> <a href='SkPoint_Reference#SkPoint'>needed</a>.
<a href='#SkPath_lineTo'>lineTo</a>() <a href='#SkPath_lineTo'>then</a> <a href='#SkPath_lineTo'>appends</a> <a href='#SkPath_kLine_Verb'>kLine_Verb</a> <a href='#SkPath_kLine_Verb'>to</a>  <a href='#Verb_Array'>verb array</a> <a href='#SkPath_kLine_Verb'>and</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkPath_lineTo_2_p'>p</a> <a href='#SkPath_lineTo_2_p'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_lineTo_2_p'><code><strong>p</strong></code></a></td>
    <td>end <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPoint_Reference#SkPoint'>added</a> <a href='undocumented#Line'>line</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="41001546a7f7927d08e5a818bcc304f5"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a> <a href='#SkPath_rLineTo'>rLineTo</a> <a href='#SkPath_addRect'>addRect</a>

<a name='SkPath_rLineTo'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_rLineTo'>rLineTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>)
</pre>

Adds <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>from</a>  <a href='#Last_Point'>last point</a> <a href='undocumented#Line'>to</a> <a href='SkPoint_Reference#Vector'>vector</a> (<a href='#SkPath_rLineTo_dx'>dx</a>, <a href='#SkPath_rLineTo_dy'>dy</a>). <a href='#SkPath_rLineTo_dy'>If</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>empty</a>, <a href='SkPath_Reference#SkPath'>or</a> <a href='SkPath_Reference#SkPath'>last</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>is</a>
<a href='#SkPath_kClose_Verb'>kClose_Verb</a>,  <a href='#Last_Point'>last point</a> <a href='#SkPath_kClose_Verb'>is</a> <a href='#SkPath_kClose_Verb'>set</a> <a href='#SkPath_kClose_Verb'>to</a> (0, 0) <a href='#SkPath_kClose_Verb'>before</a> <a href='#SkPath_kClose_Verb'>adding</a> <a href='undocumented#Line'>line</a>.

Appends <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>to</a>  <a href='#Verb_Array'>verb array</a> <a href='#SkPath_kMove_Verb'>and</a> (0, 0) <a href='#SkPath_kMove_Verb'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>, <a href='SkPoint_Reference#SkPoint'>if</a> <a href='SkPoint_Reference#SkPoint'>needed</a>;
then appends <a href='#SkPath_kLine_Verb'>kLine_Verb</a> <a href='#SkPath_kLine_Verb'>to</a>  <a href='#Verb_Array'>verb array</a> <a href='#SkPath_kLine_Verb'>and</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>end</a> <a href='undocumented#Line'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>.
<a href='undocumented#Line'>Line</a> <a href='undocumented#Line'>end</a> <a href='undocumented#Line'>is</a>  <a href='#Last_Point'>last point</a> <a href='undocumented#Line'>plus</a> <a href='SkPoint_Reference#Vector'>vector</a> (<a href='#SkPath_rLineTo_dx'>dx</a>, <a href='#SkPath_rLineTo_dy'>dy</a>).
Function name stands for "relative <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>to</a>".

### Parameters

<table>  <tr>    <td><a name='SkPath_rLineTo_dx'><code><strong>dx</strong></code></a></td>
    <td>offset from  <a href='#Last_Point'>last point</a> to <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>end</a> <a href='undocumented#Line'>on</a> <a href='undocumented#Line'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_rLineTo_dy'><code><strong>dy</strong></code></a></td>
    <td>offset from  <a href='#Last_Point'>last point</a> to <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>end</a> <a href='undocumented#Line'>on</a> <a href='undocumented#Line'>y-axis</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="6e0be0766b8ca320da51640326e608b3"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a> <a href='#SkPath_lineTo'>lineTo</a> <a href='#SkPath_addRect'>addRect</a>

<a name='Quad'></a>

---

<a href='SkPath_Reference#Quad'>Quad</a> <a href='SkPath_Reference#Quad'>describes</a> <a href='SkPath_Reference#Quad'>a</a> <a href='#Path_Quad'>Quadratic_Bezier</a>, <a href='#Path_Quad'>a</a> <a href='#Path_Quad'>second-order</a> <a href='undocumented#Curve'>curve</a> <a href='undocumented#Curve'>identical</a> <a href='undocumented#Curve'>to</a> <a href='undocumented#Curve'>a</a> <a href='undocumented#Curve'>section</a>
<a href='undocumented#Curve'>of</a> <a href='undocumented#Curve'>a</a> <a href='undocumented#Curve'>parabola</a>. <a href='SkPath_Reference#Quad'>Quad</a> <a href='SkPath_Reference#Quad'>begins</a> <a href='SkPath_Reference#Quad'>at</a> <a href='SkPath_Reference#Quad'>a</a> <a href='SkPath_Reference#Quad'>start</a> <a href='SkPoint_Reference#Point'>Point</a>, <a href='undocumented#Curve'>curves</a> <a href='undocumented#Curve'>towards</a> <a href='undocumented#Curve'>a</a> <a href='undocumented#Curve'>control</a> <a href='SkPoint_Reference#Point'>Point</a>,
<a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Point'>then</a> <a href='undocumented#Curve'>curves</a> <a href='undocumented#Curve'>to</a> <a href='undocumented#Curve'>an</a> <a href='undocumented#Curve'>end</a> <a href='SkPoint_Reference#Point'>Point</a>.

### Example

<div><fiddle-embed name="78ad51fa1cd33eb84a6f99061e56e067"></fiddle-embed></div>

<a href='SkPath_Reference#Quad'>Quad</a> <a href='SkPath_Reference#Quad'>is</a> <a href='SkPath_Reference#Quad'>a</a> <a href='SkPath_Reference#Quad'>special</a> <a href='SkPath_Reference#Quad'>case</a> <a href='SkPath_Reference#Quad'>of</a> <a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>where</a> <a href='#Path_Conic_Weight'>Conic_Weight</a> <a href='#Path_Conic_Weight'>is</a> <a href='#Path_Conic_Weight'>set</a> <a href='#Path_Conic_Weight'>to</a> <a href='#Path_Conic_Weight'>one</a>.

<a href='SkPath_Reference#Quad'>Quad</a> <a href='SkPath_Reference#Quad'>is</a> <a href='SkPath_Reference#Quad'>always</a> <a href='SkPath_Reference#Quad'>contained</a> <a href='SkPath_Reference#Quad'>by</a> <a href='SkPath_Reference#Quad'>the</a> <a href='SkPath_Reference#Quad'>triangle</a> <a href='SkPath_Reference#Quad'>connecting</a> <a href='SkPath_Reference#Quad'>its</a> <a href='SkPath_Reference#Quad'>three</a> <a href='SkPoint_Reference#Point'>Points</a>. <a href='SkPath_Reference#Quad'>Quad</a>
<a href='SkPath_Reference#Quad'>begins</a> <a href='SkPath_Reference#Quad'>tangent</a> <a href='SkPath_Reference#Quad'>to</a> <a href='SkPath_Reference#Quad'>the</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>between</a> <a href='undocumented#Line'>start</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Point'>control</a> <a href='SkPoint_Reference#Point'>Point</a>, <a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Point'>ends</a>
<a href='SkPoint_Reference#Point'>tangent</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>the</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>between</a> <a href='undocumented#Line'>control</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Point'>end</a> <a href='SkPoint_Reference#Point'>Point</a>.

### Example

<div><fiddle-embed name="4082f66a42df11bb20462b232b156bb6"></fiddle-embed></div>

<a name='SkPath_quadTo'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_quadTo'>quadTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y2</a>)
</pre>

Adds <a href='SkPath_Reference#Quad'>quad</a> <a href='SkPath_Reference#Quad'>from</a>  <a href='#Last_Point'>last point</a> <a href='SkPath_Reference#Quad'>towards</a> (<a href='#SkPath_quadTo_x1'>x1</a>, <a href='#SkPath_quadTo_y1'>y1</a>), <a href='#SkPath_quadTo_y1'>to</a> (<a href='#SkPath_quadTo_x2'>x2</a>, <a href='#SkPath_quadTo_y2'>y2</a>).
If <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>empty</a>, <a href='SkPath_Reference#SkPath'>or</a> <a href='SkPath_Reference#SkPath'>last</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>is</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a>,  <a href='#Last_Point'>last point</a> <a href='#SkPath_kClose_Verb'>is</a> <a href='#SkPath_kClose_Verb'>set</a> <a href='#SkPath_kClose_Verb'>to</a> (0, 0)
before adding <a href='SkPath_Reference#Quad'>quad</a>.

Appends <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>to</a>  <a href='#Verb_Array'>verb array</a> <a href='#SkPath_kMove_Verb'>and</a> (0, 0) <a href='#SkPath_kMove_Verb'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>, <a href='SkPoint_Reference#SkPoint'>if</a> <a href='SkPoint_Reference#SkPoint'>needed</a>;
then appends <a href='#SkPath_kQuad_Verb'>kQuad_Verb</a> <a href='#SkPath_kQuad_Verb'>to</a>  <a href='#Verb_Array'>verb array</a>; <a href='#SkPath_kQuad_Verb'>and</a> (<a href='#SkPath_quadTo_x1'>x1</a>, <a href='#SkPath_quadTo_y1'>y1</a>), (<a href='#SkPath_quadTo_x2'>x2</a>, <a href='#SkPath_quadTo_y2'>y2</a>)
to  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_quadTo_x1'><code><strong>x1</strong></code></a></td>
    <td>control <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPath_Reference#Quad'>quad</a> <a href='SkPath_Reference#Quad'>on</a> <a href='SkPath_Reference#Quad'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_quadTo_y1'><code><strong>y1</strong></code></a></td>
    <td>control <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPath_Reference#Quad'>quad</a> <a href='SkPath_Reference#Quad'>on</a> <a href='SkPath_Reference#Quad'>y-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_quadTo_x2'><code><strong>x2</strong></code></a></td>
    <td>end <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPath_Reference#Quad'>quad</a> <a href='SkPath_Reference#Quad'>on</a> <a href='SkPath_Reference#Quad'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_quadTo_y2'><code><strong>y2</strong></code></a></td>
    <td>end <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPath_Reference#Quad'>quad</a> <a href='SkPath_Reference#Quad'>on</a> <a href='SkPath_Reference#Quad'>y-axis</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="60ee3eb747474f5781b0f0dd3a17a866"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a> <a href='#SkPath_conicTo'>conicTo</a> <a href='#SkPath_rQuadTo'>rQuadTo</a>

<a name='SkPath_quadTo_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_quadTo'>quadTo</a>(<a href='#SkPath_quadTo'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p1</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p2</a>)
</pre>

Adds <a href='SkPath_Reference#Quad'>quad</a> <a href='SkPath_Reference#Quad'>from</a>  <a href='#Last_Point'>last point</a> <a href='SkPath_Reference#Quad'>towards</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkPath_quadTo_2_p1'>p1</a>, <a href='#SkPath_quadTo_2_p1'>to</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkPath_quadTo_2_p2'>p2</a>.
If <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>empty</a>, <a href='SkPath_Reference#SkPath'>or</a> <a href='SkPath_Reference#SkPath'>last</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>is</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a>,  <a href='#Last_Point'>last point</a> <a href='#SkPath_kClose_Verb'>is</a> <a href='#SkPath_kClose_Verb'>set</a> <a href='#SkPath_kClose_Verb'>to</a> (0, 0)
before adding <a href='SkPath_Reference#Quad'>quad</a>.

Appends <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>to</a>  <a href='#Verb_Array'>verb array</a> <a href='#SkPath_kMove_Verb'>and</a> (0, 0) <a href='#SkPath_kMove_Verb'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>, <a href='SkPoint_Reference#SkPoint'>if</a> <a href='SkPoint_Reference#SkPoint'>needed</a>;
then appends <a href='#SkPath_kQuad_Verb'>kQuad_Verb</a> <a href='#SkPath_kQuad_Verb'>to</a>  <a href='#Verb_Array'>verb array</a>; <a href='#SkPath_kQuad_Verb'>and</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkPath_quadTo_2_p1'>p1</a>, <a href='#SkPath_quadTo_2_p2'>p2</a>
to  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_quadTo_2_p1'><code><strong>p1</strong></code></a></td>
    <td>control <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPoint_Reference#SkPoint'>added</a> <a href='SkPath_Reference#Quad'>quad</a></td>
  </tr>
  <tr>    <td><a name='SkPath_quadTo_2_p2'><code><strong>p2</strong></code></a></td>
    <td>end <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPoint_Reference#SkPoint'>added</a> <a href='SkPath_Reference#Quad'>quad</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="82621c4df8da1e589d9e627494067826"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a> <a href='#SkPath_conicTo'>conicTo</a> <a href='#SkPath_rQuadTo'>rQuadTo</a>

<a name='SkPath_rQuadTo'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_rQuadTo'>rQuadTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy2</a>)
</pre>

Adds <a href='SkPath_Reference#Quad'>quad</a> <a href='SkPath_Reference#Quad'>from</a>  <a href='#Last_Point'>last point</a> <a href='SkPath_Reference#Quad'>towards</a> <a href='SkPoint_Reference#Vector'>vector</a> (<a href='#SkPath_rQuadTo_dx1'>dx1</a>, <a href='#SkPath_rQuadTo_dy1'>dy1</a>), <a href='#SkPath_rQuadTo_dy1'>to</a> <a href='SkPoint_Reference#Vector'>vector</a> (<a href='#SkPath_rQuadTo_dx2'>dx2</a>, <a href='#SkPath_rQuadTo_dy2'>dy2</a>).
If <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>empty</a>, <a href='SkPath_Reference#SkPath'>or</a> <a href='SkPath_Reference#SkPath'>last</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a>
is <a href='#SkPath_kClose_Verb'>kClose_Verb</a>,  <a href='#Last_Point'>last point</a> <a href='#SkPath_kClose_Verb'>is</a> <a href='#SkPath_kClose_Verb'>set</a> <a href='#SkPath_kClose_Verb'>to</a> (0, 0) <a href='#SkPath_kClose_Verb'>before</a> <a href='#SkPath_kClose_Verb'>adding</a> <a href='SkPath_Reference#Quad'>quad</a>.

Appends <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>to</a>  <a href='#Verb_Array'>verb array</a> <a href='#SkPath_kMove_Verb'>and</a> (0, 0) <a href='#SkPath_kMove_Verb'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>,
if needed; then appends <a href='#SkPath_kQuad_Verb'>kQuad_Verb</a> <a href='#SkPath_kQuad_Verb'>to</a>  <a href='#Verb_Array'>verb array</a>; <a href='#SkPath_kQuad_Verb'>and</a> <a href='#SkPath_kQuad_Verb'>appends</a> <a href='SkPath_Reference#Quad'>quad</a>
control and <a href='SkPath_Reference#Quad'>quad</a> <a href='SkPath_Reference#Quad'>end</a> <a href='SkPath_Reference#Quad'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>.
<a href='SkPath_Reference#Quad'>Quad</a> <a href='SkPath_Reference#Quad'>control</a> <a href='SkPath_Reference#Quad'>is</a>  <a href='#Last_Point'>last point</a> <a href='SkPath_Reference#Quad'>plus</a> <a href='SkPoint_Reference#Vector'>vector</a> (<a href='#SkPath_rQuadTo_dx1'>dx1</a>, <a href='#SkPath_rQuadTo_dy1'>dy1</a>).
<a href='SkPath_Reference#Quad'>Quad</a> <a href='SkPath_Reference#Quad'>end</a> <a href='SkPath_Reference#Quad'>is</a>  <a href='#Last_Point'>last point</a> <a href='SkPath_Reference#Quad'>plus</a> <a href='SkPoint_Reference#Vector'>vector</a> (<a href='#SkPath_rQuadTo_dx2'>dx2</a>, <a href='#SkPath_rQuadTo_dy2'>dy2</a>).
Function name stands for "relative <a href='SkPath_Reference#Quad'>quad</a> <a href='SkPath_Reference#Quad'>to</a>".

### Parameters

<table>  <tr>    <td><a name='SkPath_rQuadTo_dx1'><code><strong>dx1</strong></code></a></td>
    <td>offset from  <a href='#Last_Point'>last point</a> to <a href='SkPath_Reference#Quad'>quad</a> <a href='SkPath_Reference#Quad'>control</a> <a href='SkPath_Reference#Quad'>on</a> <a href='SkPath_Reference#Quad'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_rQuadTo_dy1'><code><strong>dy1</strong></code></a></td>
    <td>offset from  <a href='#Last_Point'>last point</a> to <a href='SkPath_Reference#Quad'>quad</a> <a href='SkPath_Reference#Quad'>control</a> <a href='SkPath_Reference#Quad'>on</a> <a href='SkPath_Reference#Quad'>y-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_rQuadTo_dx2'><code><strong>dx2</strong></code></a></td>
    <td>offset from  <a href='#Last_Point'>last point</a> to <a href='SkPath_Reference#Quad'>quad</a> <a href='SkPath_Reference#Quad'>end</a> <a href='SkPath_Reference#Quad'>on</a> <a href='SkPath_Reference#Quad'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_rQuadTo_dy2'><code><strong>dy2</strong></code></a></td>
    <td>offset from  <a href='#Last_Point'>last point</a> to <a href='SkPath_Reference#Quad'>quad</a> <a href='SkPath_Reference#Quad'>end</a> <a href='SkPath_Reference#Quad'>on</a> <a href='SkPath_Reference#Quad'>y-axis</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="1c1f4cdef1c572c9aa8fdf3e461191d0"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a> <a href='#SkPath_conicTo'>conicTo</a> <a href='#SkPath_quadTo'>quadTo</a>

<a name='Conic'></a>

<a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>describes</a> <a href='SkPath_Reference#Conic'>a</a> <a href='SkPath_Reference#Conic'>conical</a> <a href='SkPath_Reference#Conic'>section</a>: <a href='SkPath_Reference#Conic'>a</a> <a href='SkPath_Reference#Conic'>piece</a> <a href='SkPath_Reference#Conic'>of</a> <a href='SkPath_Reference#Conic'>an</a> <a href='SkPath_Reference#Conic'>ellipse</a>, <a href='SkPath_Reference#Conic'>or</a> <a href='SkPath_Reference#Conic'>a</a> <a href='SkPath_Reference#Conic'>piece</a> <a href='SkPath_Reference#Conic'>of</a> <a href='SkPath_Reference#Conic'>a</a>
<a href='SkPath_Reference#Conic'>parabola</a>, <a href='SkPath_Reference#Conic'>or</a> <a href='SkPath_Reference#Conic'>a</a> <a href='SkPath_Reference#Conic'>piece</a> <a href='SkPath_Reference#Conic'>of</a> <a href='SkPath_Reference#Conic'>a</a> <a href='SkPath_Reference#Conic'>hyperbola</a>. <a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>begins</a> <a href='SkPath_Reference#Conic'>at</a> <a href='SkPath_Reference#Conic'>a</a> <a href='SkPath_Reference#Conic'>start</a> <a href='SkPoint_Reference#Point'>Point</a>,
<a href='undocumented#Curve'>curves</a> <a href='undocumented#Curve'>towards</a> <a href='undocumented#Curve'>a</a> <a href='undocumented#Curve'>control</a> <a href='SkPoint_Reference#Point'>Point</a>, <a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Point'>then</a> <a href='undocumented#Curve'>curves</a> <a href='undocumented#Curve'>to</a> <a href='undocumented#Curve'>an</a> <a href='undocumented#Curve'>end</a> <a href='SkPoint_Reference#Point'>Point</a>. <a href='SkPoint_Reference#Point'>The</a> <a href='SkPoint_Reference#Point'>influence</a>
<a href='SkPoint_Reference#Point'>of</a> <a href='SkPoint_Reference#Point'>the</a> <a href='SkPoint_Reference#Point'>control</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>is</a> <a href='SkPoint_Reference#Point'>determined</a> <a href='SkPoint_Reference#Point'>by</a> <a href='#Path_Conic_Weight'>Conic_Weight</a>.

<a href='#Path_Conic_Weight'>Each</a> <a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>in</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>adds</a> <a href='SkPath_Reference#Path'>two</a> <a href='SkPoint_Reference#Point'>Points</a> <a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Point'>one</a> <a href='#Path_Conic_Weight'>Conic_Weight</a>. <a href='#Path_Conic_Weight'>Conic_Weights</a> <a href='#Path_Conic_Weight'>in</a> <a href='SkPath_Reference#Path'>Path</a>
<a href='SkPath_Reference#Path'>may</a> <a href='SkPath_Reference#Path'>be</a> <a href='SkPath_Reference#Path'>inspected</a> <a href='SkPath_Reference#Path'>with</a> <a href='#SkPath_Iter'>Iter</a>, <a href='#SkPath_Iter'>or</a> <a href='#SkPath_Iter'>with</a> <a href='#SkPath_RawIter'>RawIter</a>.

<a name='Conic_Weight'></a>

---

Weight determines both the strength of the control <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Point'>the</a> <a href='SkPoint_Reference#Point'>type</a> <a href='SkPoint_Reference#Point'>of</a> <a href='SkPath_Reference#Conic'>Conic</a>.
<a href='SkPath_Reference#Conic'>Weight</a> <a href='SkPath_Reference#Conic'>varies</a> <a href='SkPath_Reference#Conic'>from</a> <a href='SkPath_Reference#Conic'>zero</a> <a href='SkPath_Reference#Conic'>to</a> <a href='SkPath_Reference#Conic'>infinity</a>. <a href='SkPath_Reference#Conic'>At</a> <a href='SkPath_Reference#Conic'>zero</a>, <a href='SkPath_Reference#Conic'>Weight</a> <a href='SkPath_Reference#Conic'>causes</a> <a href='SkPath_Reference#Conic'>the</a> <a href='SkPath_Reference#Conic'>control</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>to</a>
<a href='SkPoint_Reference#Point'>have</a> <a href='SkPoint_Reference#Point'>no</a> <a href='SkPoint_Reference#Point'>effect</a>; <a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>is</a> <a href='SkPath_Reference#Conic'>identical</a> <a href='SkPath_Reference#Conic'>to</a> <a href='SkPath_Reference#Conic'>a</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>segment</a> <a href='undocumented#Line'>from</a> <a href='undocumented#Line'>start</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>end</a>
<a href='SkPoint_Reference#Point'>point</a>. <a href='SkPoint_Reference#Point'>If</a> <a href='SkPoint_Reference#Point'>Weight</a> <a href='SkPoint_Reference#Point'>is</a> <a href='SkPoint_Reference#Point'>less</a> <a href='SkPoint_Reference#Point'>than</a> <a href='SkPoint_Reference#Point'>one</a>, <a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>follows</a> <a href='SkPath_Reference#Conic'>an</a> <a href='SkPath_Reference#Conic'>elliptical</a> <a href='undocumented#Arc'>arc</a>.
<a href='undocumented#Arc'>If</a> <a href='undocumented#Arc'>Weight</a> <a href='undocumented#Arc'>is</a> <a href='undocumented#Arc'>exactly</a> <a href='undocumented#Arc'>one</a>, <a href='undocumented#Arc'>then</a> <a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>is</a> <a href='SkPath_Reference#Conic'>identical</a> <a href='SkPath_Reference#Conic'>to</a> <a href='SkPath_Reference#Quad'>Quad</a>; <a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>follows</a> <a href='SkPath_Reference#Conic'>a</a>
<a href='SkPath_Reference#Conic'>parabolic</a> <a href='undocumented#Arc'>arc</a>. <a href='undocumented#Arc'>If</a> <a href='undocumented#Arc'>Weight</a> <a href='undocumented#Arc'>is</a> <a href='undocumented#Arc'>greater</a> <a href='undocumented#Arc'>than</a> <a href='undocumented#Arc'>one</a>, <a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>follows</a> <a href='SkPath_Reference#Conic'>a</a> <a href='SkPath_Reference#Conic'>hyperbolic</a>
<a href='undocumented#Arc'>arc</a>. <a href='undocumented#Arc'>If</a> <a href='undocumented#Arc'>Weight</a> <a href='undocumented#Arc'>is</a> <a href='undocumented#Arc'>infinity</a>, <a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>is</a> <a href='SkPath_Reference#Conic'>identical</a> <a href='SkPath_Reference#Conic'>to</a> <a href='SkPath_Reference#Conic'>two</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>segments</a>, <a href='undocumented#Line'>connecting</a>
<a href='undocumented#Line'>start</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>control</a> <a href='SkPoint_Reference#Point'>Point</a>, <a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Point'>control</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>end</a> <a href='SkPoint_Reference#Point'>Point</a>.

### Example

<div><fiddle-embed name="2aadded3d20dfef34d1c8abe28c7bc8d"><div>When <a href='#Path_Conic_Weight'>Conic_Weight</a> <a href='#Path_Conic_Weight'>is</a> <a href='#Path_Conic_Weight'>one</a>, <a href='SkPath_Reference#Quad'>Quad</a> <a href='SkPath_Reference#Quad'>is</a> <a href='SkPath_Reference#Quad'>added</a> <a href='SkPath_Reference#Quad'>to</a> <a href='SkPath_Reference#Path'>path</a>; <a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Path'>two</a> <a href='SkPath_Reference#Path'>are</a> <a href='SkPath_Reference#Path'>identical</a>.
</div>

#### Example Output

~~~~
move {0, 0},
quad {0, 0}, {20, 30}, {50, 60},
done
~~~~

</fiddle-embed></div>

If weight is less than one, <a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>is</a> <a href='SkPath_Reference#Conic'>an</a> <a href='SkPath_Reference#Conic'>elliptical</a> <a href='SkPath_Reference#Conic'>segment</a>.

### Example

<div><fiddle-embed name="e88f554efacfa9f75f270fb1c0add5b4"><div>A 90 degree circular <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>has</a> <a href='undocumented#Arc'>the</a> <a href='undocumented#Arc'>weight</a> <code>1 / <a href='undocumented#sqrt()'>sqrt</a>(2)</code>.
</div>

#### Example Output

~~~~
move {0, 0},
conic {0, 0}, {20, 0}, {20, 20}, weight = 0.707107
done
~~~~

</fiddle-embed></div>

If weight is greater than one, <a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>is</a> <a href='SkPath_Reference#Conic'>a</a> <a href='SkPath_Reference#Conic'>hyperbolic</a> <a href='SkPath_Reference#Conic'>segment</a>. <a href='SkPath_Reference#Conic'>As</a> <a href='SkPath_Reference#Conic'>weight</a> <a href='SkPath_Reference#Conic'>gets</a> <a href='SkPath_Reference#Conic'>large</a>,
<a href='SkPath_Reference#Conic'>a</a> <a href='SkPath_Reference#Conic'>hyperbolic</a> <a href='SkPath_Reference#Conic'>segment</a> <a href='SkPath_Reference#Conic'>can</a> <a href='SkPath_Reference#Conic'>be</a> <a href='SkPath_Reference#Conic'>approximated</a> <a href='SkPath_Reference#Conic'>by</a> <a href='SkPath_Reference#Conic'>straight</a> <a href='undocumented#Line'>lines</a> <a href='undocumented#Line'>connecting</a> <a href='undocumented#Line'>the</a>
<a href='undocumented#Line'>control</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>with</a> <a href='SkPoint_Reference#Point'>the</a> <a href='SkPoint_Reference#Point'>end</a> <a href='SkPoint_Reference#Point'>Points</a>.

### Example

<div><fiddle-embed name="6fb11419e99297fe2fe666c296117fb9">

#### Example Output

~~~~
move {0, 0},
line {0, 0}, {20, 0},
line {20, 0}, {20, 20},
done
~~~~

</fiddle-embed></div>

<a name='SkPath_conicTo'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_conicTo'>conicTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>w</a>)
</pre>

Adds <a href='SkPath_Reference#Conic'>conic</a> <a href='SkPath_Reference#Conic'>from</a>  <a href='#Last_Point'>last point</a> <a href='SkPath_Reference#Conic'>towards</a> (<a href='#SkPath_conicTo_x1'>x1</a>, <a href='#SkPath_conicTo_y1'>y1</a>), <a href='#SkPath_conicTo_y1'>to</a> (<a href='#SkPath_conicTo_x2'>x2</a>, <a href='#SkPath_conicTo_y2'>y2</a>), <a href='#SkPath_conicTo_y2'>weighted</a> <a href='#SkPath_conicTo_y2'>by</a> <a href='#SkPath_conicTo_w'>w</a>.
If <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>empty</a>, <a href='SkPath_Reference#SkPath'>or</a> <a href='SkPath_Reference#SkPath'>last</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>is</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a>,  <a href='#Last_Point'>last point</a> <a href='#SkPath_kClose_Verb'>is</a> <a href='#SkPath_kClose_Verb'>set</a> <a href='#SkPath_kClose_Verb'>to</a> (0, 0)
before adding <a href='SkPath_Reference#Conic'>conic</a>.

Appends <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>to</a>  <a href='#Verb_Array'>verb array</a> <a href='#SkPath_kMove_Verb'>and</a> (0, 0) <a href='#SkPath_kMove_Verb'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>, <a href='SkPoint_Reference#SkPoint'>if</a> <a href='SkPoint_Reference#SkPoint'>needed</a>.

If <a href='#SkPath_conicTo_w'>w</a> <a href='#SkPath_conicTo_w'>is</a> <a href='#SkPath_conicTo_w'>finite</a> <a href='#SkPath_conicTo_w'>and</a> <a href='#SkPath_conicTo_w'>not</a> <a href='#SkPath_conicTo_w'>one</a>, <a href='#SkPath_conicTo_w'>appends</a> <a href='#SkPath_kConic_Verb'>kConic_Verb</a> <a href='#SkPath_kConic_Verb'>to</a>  <a href='#Verb_Array'>verb array</a>;
and (<a href='#SkPath_conicTo_x1'>x1</a>, <a href='#SkPath_conicTo_y1'>y1</a>), (<a href='#SkPath_conicTo_x2'>x2</a>, <a href='#SkPath_conicTo_y2'>y2</a>) <a href='#SkPath_conicTo_y2'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>; <a href='SkPoint_Reference#SkPoint'>and</a> <a href='#SkPath_conicTo_w'>w</a> <a href='#SkPath_conicTo_w'>to</a>  <a href='SkPath_Reference#Conic_Weight'>conic weights</a>.

If <a href='#SkPath_conicTo_w'>w</a> <a href='#SkPath_conicTo_w'>is</a> <a href='#SkPath_conicTo_w'>one</a>, <a href='#SkPath_conicTo_w'>appends</a> <a href='#SkPath_kQuad_Verb'>kQuad_Verb</a> <a href='#SkPath_kQuad_Verb'>to</a>  <a href='#Verb_Array'>verb array</a>, <a href='#SkPath_kQuad_Verb'>and</a>
(<a href='#SkPath_conicTo_x1'>x1</a>, <a href='#SkPath_conicTo_y1'>y1</a>), (<a href='#SkPath_conicTo_x2'>x2</a>, <a href='#SkPath_conicTo_y2'>y2</a>) <a href='#SkPath_conicTo_y2'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>.

If <a href='#SkPath_conicTo_w'>w</a> <a href='#SkPath_conicTo_w'>is</a> <a href='#SkPath_conicTo_w'>not</a> <a href='#SkPath_conicTo_w'>finite</a>, <a href='#SkPath_conicTo_w'>appends</a> <a href='#SkPath_kLine_Verb'>kLine_Verb</a> <a href='#SkPath_kLine_Verb'>twice</a> <a href='#SkPath_kLine_Verb'>to</a>  <a href='#Verb_Array'>verb array</a>, <a href='#SkPath_kLine_Verb'>and</a>
(<a href='#SkPath_conicTo_x1'>x1</a>, <a href='#SkPath_conicTo_y1'>y1</a>), (<a href='#SkPath_conicTo_x2'>x2</a>, <a href='#SkPath_conicTo_y2'>y2</a>) <a href='#SkPath_conicTo_y2'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_conicTo_x1'><code><strong>x1</strong></code></a></td>
    <td>control <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPath_Reference#Conic'>conic</a> <a href='SkPath_Reference#Conic'>on</a> <a href='SkPath_Reference#Conic'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_conicTo_y1'><code><strong>y1</strong></code></a></td>
    <td>control <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPath_Reference#Conic'>conic</a> <a href='SkPath_Reference#Conic'>on</a> <a href='SkPath_Reference#Conic'>y-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_conicTo_x2'><code><strong>x2</strong></code></a></td>
    <td>end <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPath_Reference#Conic'>conic</a> <a href='SkPath_Reference#Conic'>on</a> <a href='SkPath_Reference#Conic'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_conicTo_y2'><code><strong>y2</strong></code></a></td>
    <td>end <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPath_Reference#Conic'>conic</a> <a href='SkPath_Reference#Conic'>on</a> <a href='SkPath_Reference#Conic'>y-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_conicTo_w'><code><strong>w</strong></code></a></td>
    <td>weight of added <a href='SkPath_Reference#Conic'>conic</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="358d9b6060b528b0923c007420f09c13"><div>As weight increases, <a href='undocumented#Curve'>curve</a> <a href='undocumented#Curve'>is</a> <a href='undocumented#Curve'>pulled</a> <a href='undocumented#Curve'>towards</a> <a href='undocumented#Curve'>control</a> <a href='SkPoint_Reference#Point'>point</a>.
<a href='SkPoint_Reference#Point'>The</a> <a href='SkPoint_Reference#Point'>bottom</a> <a href='SkPoint_Reference#Point'>two</a> <a href='undocumented#Curve'>curves</a> <a href='undocumented#Curve'>are</a> <a href='undocumented#Curve'>elliptical</a>; <a href='undocumented#Curve'>the</a> <a href='undocumented#Curve'>next</a> <a href='undocumented#Curve'>is</a> <a href='undocumented#Curve'>parabolic</a>; <a href='undocumented#Curve'>the</a>
<a href='undocumented#Curve'>top</a> <a href='undocumented#Curve'>curve</a> <a href='undocumented#Curve'>is</a> <a href='undocumented#Curve'>hyperbolic</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_rConicTo'>rConicTo</a> <a href='#SkPath_arcTo'>arcTo</a> <a href='#SkPath_addArc'>addArc</a> <a href='#SkPath_quadTo'>quadTo</a>

<a name='SkPath_conicTo_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_conicTo'>conicTo</a>(<a href='#SkPath_conicTo'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p1</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>w</a>)
</pre>

Adds <a href='SkPath_Reference#Conic'>conic</a> <a href='SkPath_Reference#Conic'>from</a>  <a href='#Last_Point'>last point</a> <a href='SkPath_Reference#Conic'>towards</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkPath_conicTo_2_p1'>p1</a>, <a href='#SkPath_conicTo_2_p1'>to</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkPath_conicTo_2_p2'>p2</a>, <a href='#SkPath_conicTo_2_p2'>weighted</a> <a href='#SkPath_conicTo_2_p2'>by</a> <a href='#SkPath_conicTo_2_w'>w</a>.
If <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>empty</a>, <a href='SkPath_Reference#SkPath'>or</a> <a href='SkPath_Reference#SkPath'>last</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>is</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a>,  <a href='#Last_Point'>last point</a> <a href='#SkPath_kClose_Verb'>is</a> <a href='#SkPath_kClose_Verb'>set</a> <a href='#SkPath_kClose_Verb'>to</a> (0, 0)
before adding <a href='SkPath_Reference#Conic'>conic</a>.

Appends <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>to</a>  <a href='#Verb_Array'>verb array</a> <a href='#SkPath_kMove_Verb'>and</a> (0, 0) <a href='#SkPath_kMove_Verb'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>, <a href='SkPoint_Reference#SkPoint'>if</a> <a href='SkPoint_Reference#SkPoint'>needed</a>.

If <a href='#SkPath_conicTo_2_w'>w</a> <a href='#SkPath_conicTo_2_w'>is</a> <a href='#SkPath_conicTo_2_w'>finite</a> <a href='#SkPath_conicTo_2_w'>and</a> <a href='#SkPath_conicTo_2_w'>not</a> <a href='#SkPath_conicTo_2_w'>one</a>, <a href='#SkPath_conicTo_2_w'>appends</a> <a href='#SkPath_kConic_Verb'>kConic_Verb</a> <a href='#SkPath_kConic_Verb'>to</a>  <a href='#Verb_Array'>verb array</a>;
and <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkPath_conicTo_2_p1'>p1</a>, <a href='#SkPath_conicTo_2_p2'>p2</a> <a href='#SkPath_conicTo_2_p2'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>; <a href='SkPoint_Reference#SkPoint'>and</a> <a href='#SkPath_conicTo_2_w'>w</a> <a href='#SkPath_conicTo_2_w'>to</a>  <a href='SkPath_Reference#Conic_Weight'>conic weights</a>.

If <a href='#SkPath_conicTo_2_w'>w</a> <a href='#SkPath_conicTo_2_w'>is</a> <a href='#SkPath_conicTo_2_w'>one</a>, <a href='#SkPath_conicTo_2_w'>appends</a> <a href='#SkPath_kQuad_Verb'>kQuad_Verb</a> <a href='#SkPath_kQuad_Verb'>to</a>  <a href='#Verb_Array'>verb array</a>, <a href='#SkPath_kQuad_Verb'>and</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkPath_conicTo_2_p1'>p1</a>, <a href='#SkPath_conicTo_2_p2'>p2</a>
to  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>.

If <a href='#SkPath_conicTo_2_w'>w</a> <a href='#SkPath_conicTo_2_w'>is</a> <a href='#SkPath_conicTo_2_w'>not</a> <a href='#SkPath_conicTo_2_w'>finite</a>, <a href='#SkPath_conicTo_2_w'>appends</a> <a href='#SkPath_kLine_Verb'>kLine_Verb</a> <a href='#SkPath_kLine_Verb'>twice</a> <a href='#SkPath_kLine_Verb'>to</a>  <a href='#Verb_Array'>verb array</a>, <a href='#SkPath_kLine_Verb'>and</a>
<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkPath_conicTo_2_p1'>p1</a>, <a href='#SkPath_conicTo_2_p2'>p2</a> <a href='#SkPath_conicTo_2_p2'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_conicTo_2_p1'><code><strong>p1</strong></code></a></td>
    <td>control <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPoint_Reference#SkPoint'>added</a> <a href='SkPath_Reference#Conic'>conic</a></td>
  </tr>
  <tr>    <td><a name='SkPath_conicTo_2_p2'><code><strong>p2</strong></code></a></td>
    <td>end <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPoint_Reference#SkPoint'>added</a> <a href='SkPath_Reference#Conic'>conic</a></td>
  </tr>
  <tr>    <td><a name='SkPath_conicTo_2_w'><code><strong>w</strong></code></a></td>
    <td>weight of added <a href='SkPath_Reference#Conic'>conic</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="22d25e03b19d5bae92118877e462361b"><div><a href='SkPath_Reference#Conic'>Conics</a> <a href='SkPath_Reference#Conic'>and</a> <a href='undocumented#Arc'>arcs</a> <a href='undocumented#Arc'>use</a> <a href='undocumented#Arc'>identical</a> <a href='undocumented#Arc'>representations</a>. <a href='undocumented#Arc'>As</a> <a href='undocumented#Arc'>the</a> <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>sweep</a> <a href='undocumented#Arc'>increases</a>
<a href='undocumented#Arc'>the</a> <a href='#Path_Conic_Weight'>Conic_Weight</a> <a href='#Path_Conic_Weight'>also</a> <a href='#Path_Conic_Weight'>increases</a>, <a href='#Path_Conic_Weight'>but</a> <a href='#Path_Conic_Weight'>remains</a> <a href='#Path_Conic_Weight'>smaller</a> <a href='#Path_Conic_Weight'>than</a> <a href='#Path_Conic_Weight'>one</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_rConicTo'>rConicTo</a> <a href='#SkPath_arcTo'>arcTo</a> <a href='#SkPath_addArc'>addArc</a> <a href='#SkPath_quadTo'>quadTo</a>

<a name='SkPath_rConicTo'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_rConicTo'>rConicTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>w</a>)
</pre>

Adds <a href='SkPath_Reference#Conic'>conic</a> <a href='SkPath_Reference#Conic'>from</a>  <a href='#Last_Point'>last point</a> <a href='SkPath_Reference#Conic'>towards</a> <a href='SkPoint_Reference#Vector'>vector</a> (<a href='#SkPath_rConicTo_dx1'>dx1</a>, <a href='#SkPath_rConicTo_dy1'>dy1</a>), <a href='#SkPath_rConicTo_dy1'>to</a> <a href='SkPoint_Reference#Vector'>vector</a> (<a href='#SkPath_rConicTo_dx2'>dx2</a>, <a href='#SkPath_rConicTo_dy2'>dy2</a>),
weighted by <a href='#SkPath_rConicTo_w'>w</a>. <a href='#SkPath_rConicTo_w'>If</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>empty</a>, <a href='SkPath_Reference#SkPath'>or</a> <a href='SkPath_Reference#SkPath'>last</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a>
is <a href='#SkPath_kClose_Verb'>kClose_Verb</a>,  <a href='#Last_Point'>last point</a> <a href='#SkPath_kClose_Verb'>is</a> <a href='#SkPath_kClose_Verb'>set</a> <a href='#SkPath_kClose_Verb'>to</a> (0, 0) <a href='#SkPath_kClose_Verb'>before</a> <a href='#SkPath_kClose_Verb'>adding</a> <a href='SkPath_Reference#Conic'>conic</a>.

Appends <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>to</a>  <a href='#Verb_Array'>verb array</a> <a href='#SkPath_kMove_Verb'>and</a> (0, 0) <a href='#SkPath_kMove_Verb'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>,
if needed.

If <a href='#SkPath_rConicTo_w'>w</a> <a href='#SkPath_rConicTo_w'>is</a> <a href='#SkPath_rConicTo_w'>finite</a> <a href='#SkPath_rConicTo_w'>and</a> <a href='#SkPath_rConicTo_w'>not</a> <a href='#SkPath_rConicTo_w'>one</a>, <a href='#SkPath_rConicTo_w'>next</a> <a href='#SkPath_rConicTo_w'>appends</a> <a href='#SkPath_kConic_Verb'>kConic_Verb</a> <a href='#SkPath_kConic_Verb'>to</a>  <a href='#Verb_Array'>verb array</a>,
and <a href='#SkPath_rConicTo_w'>w</a> <a href='#SkPath_rConicTo_w'>is</a> <a href='#SkPath_rConicTo_w'>recorded</a> <a href='#SkPath_rConicTo_w'>as</a>  <a href='#Conic_Weight'>conic weight</a>; <a href='SkPath_Reference#Conic'>otherwise</a>, <a href='SkPath_Reference#Conic'>if</a> <a href='#SkPath_rConicTo_w'>w</a> <a href='#SkPath_rConicTo_w'>is</a> <a href='#SkPath_rConicTo_w'>one</a>, <a href='#SkPath_rConicTo_w'>appends</a>
<a href='#SkPath_kQuad_Verb'>kQuad_Verb</a> <a href='#SkPath_kQuad_Verb'>to</a>  <a href='#Verb_Array'>verb array</a>; <a href='#SkPath_kQuad_Verb'>or</a> <a href='#SkPath_kQuad_Verb'>if</a> <a href='#SkPath_rConicTo_w'>w</a> <a href='#SkPath_rConicTo_w'>is</a> <a href='#SkPath_rConicTo_w'>not</a> <a href='#SkPath_rConicTo_w'>finite</a>, <a href='#SkPath_rConicTo_w'>appends</a> <a href='#SkPath_kLine_Verb'>kLine_Verb</a>
twice to  <a href='#Verb_Array'>verb array</a>.

In all cases appends <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>control</a> <a href='SkPoint_Reference#SkPoint'>and</a> <a href='SkPoint_Reference#SkPoint'>end</a> <a href='SkPoint_Reference#SkPoint'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>.
control is  <a href='#Last_Point'>last point</a> plus <a href='SkPoint_Reference#Vector'>vector</a> (<a href='#SkPath_rConicTo_dx1'>dx1</a>, <a href='#SkPath_rConicTo_dy1'>dy1</a>).
end is  <a href='#Last_Point'>last point</a> plus <a href='SkPoint_Reference#Vector'>vector</a> (<a href='#SkPath_rConicTo_dx2'>dx2</a>, <a href='#SkPath_rConicTo_dy2'>dy2</a>).

Function name stands for "relative <a href='SkPath_Reference#Conic'>conic</a> <a href='SkPath_Reference#Conic'>to</a>".

### Parameters

<table>  <tr>    <td><a name='SkPath_rConicTo_dx1'><code><strong>dx1</strong></code></a></td>
    <td>offset from  <a href='#Last_Point'>last point</a> to <a href='SkPath_Reference#Conic'>conic</a> <a href='SkPath_Reference#Conic'>control</a> <a href='SkPath_Reference#Conic'>on</a> <a href='SkPath_Reference#Conic'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_rConicTo_dy1'><code><strong>dy1</strong></code></a></td>
    <td>offset from  <a href='#Last_Point'>last point</a> to <a href='SkPath_Reference#Conic'>conic</a> <a href='SkPath_Reference#Conic'>control</a> <a href='SkPath_Reference#Conic'>on</a> <a href='SkPath_Reference#Conic'>y-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_rConicTo_dx2'><code><strong>dx2</strong></code></a></td>
    <td>offset from  <a href='#Last_Point'>last point</a> to <a href='SkPath_Reference#Conic'>conic</a> <a href='SkPath_Reference#Conic'>end</a> <a href='SkPath_Reference#Conic'>on</a> <a href='SkPath_Reference#Conic'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_rConicTo_dy2'><code><strong>dy2</strong></code></a></td>
    <td>offset from  <a href='#Last_Point'>last point</a> to <a href='SkPath_Reference#Conic'>conic</a> <a href='SkPath_Reference#Conic'>end</a> <a href='SkPath_Reference#Conic'>on</a> <a href='SkPath_Reference#Conic'>y-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_rConicTo_w'><code><strong>w</strong></code></a></td>
    <td>weight of added <a href='SkPath_Reference#Conic'>conic</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="3d52763e7c0e20c0b1d484a0afa622d2"></fiddle-embed></div>

### See Also

<a href='#SkPath_conicTo'>conicTo</a> <a href='#SkPath_arcTo'>arcTo</a> <a href='#SkPath_addArc'>addArc</a> <a href='#SkPath_quadTo'>quadTo</a>

<a name='Cubic'></a>

---

<a href='SkPath_Reference#Cubic'>Cubic</a> <a href='SkPath_Reference#Cubic'>describes</a> <a href='SkPath_Reference#Cubic'>a</a> <a href='#Bezier_Curve'>Bezier_Curve</a> <a href='#Bezier_Curve'>segment</a> <a href='#Bezier_Curve'>described</a> <a href='#Bezier_Curve'>by</a> <a href='#Bezier_Curve'>a</a> <a href='#Bezier_Curve'>third-order</a> <a href='#Bezier_Curve'>polynomial</a>.
<a href='SkPath_Reference#Cubic'>Cubic</a> <a href='SkPath_Reference#Cubic'>begins</a> <a href='SkPath_Reference#Cubic'>at</a> <a href='SkPath_Reference#Cubic'>a</a> <a href='SkPath_Reference#Cubic'>start</a> <a href='SkPoint_Reference#Point'>Point</a>, <a href='SkPoint_Reference#Point'>curving</a> <a href='SkPoint_Reference#Point'>towards</a> <a href='SkPoint_Reference#Point'>the</a> <a href='SkPoint_Reference#Point'>first</a> <a href='SkPoint_Reference#Point'>control</a> <a href='SkPoint_Reference#Point'>Point</a>;
<a href='SkPoint_Reference#Point'>and</a> <a href='undocumented#Curve'>curves</a> <a href='undocumented#Curve'>from</a> <a href='undocumented#Curve'>the</a> <a href='undocumented#Curve'>end</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>towards</a> <a href='SkPoint_Reference#Point'>the</a> <a href='SkPoint_Reference#Point'>second</a> <a href='SkPoint_Reference#Point'>control</a> <a href='SkPoint_Reference#Point'>Point</a>.

### Example

<div><fiddle-embed name="466445ed991d86de08587066392d654a"></fiddle-embed></div>

<a name='SkPath_cubicTo'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_cubicTo'>cubicTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x3</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y3</a>)
</pre>

Adds <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>from</a>  <a href='#Last_Point'>last point</a> <a href='SkPath_Reference#Cubic'>towards</a> (<a href='#SkPath_cubicTo_x1'>x1</a>, <a href='#SkPath_cubicTo_y1'>y1</a>), <a href='#SkPath_cubicTo_y1'>then</a> <a href='#SkPath_cubicTo_y1'>towards</a> (<a href='#SkPath_cubicTo_x2'>x2</a>, <a href='#SkPath_cubicTo_y2'>y2</a>), <a href='#SkPath_cubicTo_y2'>ending</a> <a href='#SkPath_cubicTo_y2'>at</a>
(<a href='#SkPath_cubicTo_x3'>x3</a>, <a href='#SkPath_cubicTo_y3'>y3</a>). <a href='#SkPath_cubicTo_y3'>If</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>empty</a>, <a href='SkPath_Reference#SkPath'>or</a> <a href='SkPath_Reference#SkPath'>last</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>is</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a>,  <a href='#Last_Point'>last point</a> <a href='#SkPath_kClose_Verb'>is</a> <a href='#SkPath_kClose_Verb'>set</a> <a href='#SkPath_kClose_Verb'>to</a>
(0, 0) before adding <a href='SkPath_Reference#Cubic'>cubic</a>.

Appends <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>to</a>  <a href='#Verb_Array'>verb array</a> <a href='#SkPath_kMove_Verb'>and</a> (0, 0) <a href='#SkPath_kMove_Verb'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>, <a href='SkPoint_Reference#SkPoint'>if</a> <a href='SkPoint_Reference#SkPoint'>needed</a>;
then appends <a href='#SkPath_kCubic_Verb'>kCubic_Verb</a> <a href='#SkPath_kCubic_Verb'>to</a>  <a href='#Verb_Array'>verb array</a>; <a href='#SkPath_kCubic_Verb'>and</a> (<a href='#SkPath_cubicTo_x1'>x1</a>, <a href='#SkPath_cubicTo_y1'>y1</a>), (<a href='#SkPath_cubicTo_x2'>x2</a>, <a href='#SkPath_cubicTo_y2'>y2</a>), (<a href='#SkPath_cubicTo_x3'>x3</a>, <a href='#SkPath_cubicTo_y3'>y3</a>)
to  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_cubicTo_x1'><code><strong>x1</strong></code></a></td>
    <td>first control <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>on</a> <a href='SkPath_Reference#Cubic'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_cubicTo_y1'><code><strong>y1</strong></code></a></td>
    <td>first control <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>on</a> <a href='SkPath_Reference#Cubic'>y-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_cubicTo_x2'><code><strong>x2</strong></code></a></td>
    <td>second control <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>on</a> <a href='SkPath_Reference#Cubic'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_cubicTo_y2'><code><strong>y2</strong></code></a></td>
    <td>second control <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>on</a> <a href='SkPath_Reference#Cubic'>y-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_cubicTo_x3'><code><strong>x3</strong></code></a></td>
    <td>end <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>on</a> <a href='SkPath_Reference#Cubic'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_cubicTo_y3'><code><strong>y3</strong></code></a></td>
    <td>end <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>on</a> <a href='SkPath_Reference#Cubic'>y-axis</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="3e476378e3e0550ab134bbaf61112d98"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a> <a href='#SkPath_rCubicTo'>rCubicTo</a> <a href='#SkPath_quadTo'>quadTo</a>

<a name='SkPath_cubicTo_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_cubicTo'>cubicTo</a>(<a href='#SkPath_cubicTo'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p1</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p2</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p3</a>)
</pre>

Adds <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>from</a>  <a href='#Last_Point'>last point</a> <a href='SkPath_Reference#Cubic'>towards</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkPath_cubicTo_2_p1'>p1</a>, <a href='#SkPath_cubicTo_2_p1'>then</a> <a href='#SkPath_cubicTo_2_p1'>towards</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkPath_cubicTo_2_p2'>p2</a>, <a href='#SkPath_cubicTo_2_p2'>ending</a> <a href='#SkPath_cubicTo_2_p2'>at</a>
<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkPath_cubicTo_2_p3'>p3</a>. <a href='#SkPath_cubicTo_2_p3'>If</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>empty</a>, <a href='SkPath_Reference#SkPath'>or</a> <a href='SkPath_Reference#SkPath'>last</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>is</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a>,  <a href='#Last_Point'>last point</a> <a href='#SkPath_kClose_Verb'>is</a> <a href='#SkPath_kClose_Verb'>set</a> <a href='#SkPath_kClose_Verb'>to</a>
(0, 0) before adding <a href='SkPath_Reference#Cubic'>cubic</a>.

Appends <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>to</a>  <a href='#Verb_Array'>verb array</a> <a href='#SkPath_kMove_Verb'>and</a> (0, 0) <a href='#SkPath_kMove_Verb'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>, <a href='SkPoint_Reference#SkPoint'>if</a> <a href='SkPoint_Reference#SkPoint'>needed</a>;
then appends <a href='#SkPath_kCubic_Verb'>kCubic_Verb</a> <a href='#SkPath_kCubic_Verb'>to</a>  <a href='#Verb_Array'>verb array</a>; <a href='#SkPath_kCubic_Verb'>and</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkPath_cubicTo_2_p1'>p1</a>, <a href='#SkPath_cubicTo_2_p2'>p2</a>, <a href='#SkPath_cubicTo_2_p3'>p3</a>
to  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_cubicTo_2_p1'><code><strong>p1</strong></code></a></td>
    <td>first control <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPath_Reference#Cubic'>cubic</a></td>
  </tr>
  <tr>    <td><a name='SkPath_cubicTo_2_p2'><code><strong>p2</strong></code></a></td>
    <td>second control <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPath_Reference#Cubic'>cubic</a></td>
  </tr>
  <tr>    <td><a name='SkPath_cubicTo_2_p3'><code><strong>p3</strong></code></a></td>
    <td>end <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPath_Reference#Cubic'>cubic</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="d38aaf12c6ff5b8d901a2201bcee5476"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a> <a href='#SkPath_rCubicTo'>rCubicTo</a> <a href='#SkPath_quadTo'>quadTo</a>

<a name='SkPath_rCubicTo'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_rCubicTo'>rCubicTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx3</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy3</a>)
</pre>

Adds <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>from</a>  <a href='#Last_Point'>last point</a> <a href='SkPath_Reference#Cubic'>towards</a> <a href='SkPoint_Reference#Vector'>vector</a> (<a href='#SkPath_rCubicTo_dx1'>dx1</a>, <a href='#SkPath_rCubicTo_dy1'>dy1</a>), <a href='#SkPath_rCubicTo_dy1'>then</a> <a href='#SkPath_rCubicTo_dy1'>towards</a>
<a href='SkPoint_Reference#Vector'>vector</a> (<a href='#SkPath_rCubicTo_dx2'>dx2</a>, <a href='#SkPath_rCubicTo_dy2'>dy2</a>), <a href='#SkPath_rCubicTo_dy2'>to</a> <a href='SkPoint_Reference#Vector'>vector</a> (<a href='#SkPath_rCubicTo_dx3'>dx3</a>, <a href='#SkPath_rCubicTo_dy3'>dy3</a>).
If <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>empty</a>, <a href='SkPath_Reference#SkPath'>or</a> <a href='SkPath_Reference#SkPath'>last</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a>
is <a href='#SkPath_kClose_Verb'>kClose_Verb</a>,  <a href='#Last_Point'>last point</a> <a href='#SkPath_kClose_Verb'>is</a> <a href='#SkPath_kClose_Verb'>set</a> <a href='#SkPath_kClose_Verb'>to</a> (0, 0) <a href='#SkPath_kClose_Verb'>before</a> <a href='#SkPath_kClose_Verb'>adding</a> <a href='SkPath_Reference#Cubic'>cubic</a>.

Appends <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>to</a>  <a href='#Verb_Array'>verb array</a> <a href='#SkPath_kMove_Verb'>and</a> (0, 0) <a href='#SkPath_kMove_Verb'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>,
if needed; then appends <a href='#SkPath_kCubic_Verb'>kCubic_Verb</a> <a href='#SkPath_kCubic_Verb'>to</a>  <a href='#Verb_Array'>verb array</a>; <a href='#SkPath_kCubic_Verb'>and</a> <a href='#SkPath_kCubic_Verb'>appends</a> <a href='SkPath_Reference#Cubic'>cubic</a>
control and <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>end</a> <a href='SkPath_Reference#Cubic'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>.
<a href='SkPath_Reference#Cubic'>Cubic</a> <a href='SkPath_Reference#Cubic'>control</a> <a href='SkPath_Reference#Cubic'>is</a>  <a href='#Last_Point'>last point</a> <a href='SkPath_Reference#Cubic'>plus</a> <a href='SkPoint_Reference#Vector'>vector</a> (<a href='#SkPath_rCubicTo_dx1'>dx1</a>, <a href='#SkPath_rCubicTo_dy1'>dy1</a>).
<a href='SkPath_Reference#Cubic'>Cubic</a> <a href='SkPath_Reference#Cubic'>end</a> <a href='SkPath_Reference#Cubic'>is</a>  <a href='#Last_Point'>last point</a> <a href='SkPath_Reference#Cubic'>plus</a> <a href='SkPoint_Reference#Vector'>vector</a> (<a href='#SkPath_rCubicTo_dx2'>dx2</a>, <a href='#SkPath_rCubicTo_dy2'>dy2</a>).
Function name stands for "relative <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>to</a>".

### Parameters

<table>  <tr>    <td><a name='SkPath_rCubicTo_dx1'><code><strong>dx1</strong></code></a></td>
    <td>offset from  <a href='#Last_Point'>last point</a> to first <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>control</a> <a href='SkPath_Reference#Cubic'>on</a> <a href='SkPath_Reference#Cubic'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_rCubicTo_dy1'><code><strong>dy1</strong></code></a></td>
    <td>offset from  <a href='#Last_Point'>last point</a> to first <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>control</a> <a href='SkPath_Reference#Cubic'>on</a> <a href='SkPath_Reference#Cubic'>y-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_rCubicTo_dx2'><code><strong>dx2</strong></code></a></td>
    <td>offset from  <a href='#Last_Point'>last point</a> to second <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>control</a> <a href='SkPath_Reference#Cubic'>on</a> <a href='SkPath_Reference#Cubic'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_rCubicTo_dy2'><code><strong>dy2</strong></code></a></td>
    <td>offset from  <a href='#Last_Point'>last point</a> to second <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>control</a> <a href='SkPath_Reference#Cubic'>on</a> <a href='SkPath_Reference#Cubic'>y-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_rCubicTo_dx3'><code><strong>dx3</strong></code></a></td>
    <td>offset from  <a href='#Last_Point'>last point</a> to <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>end</a> <a href='SkPath_Reference#Cubic'>on</a> <a href='SkPath_Reference#Cubic'>x-axis</a></td>
  </tr>
  <tr>    <td><a name='SkPath_rCubicTo_dy3'><code><strong>dy3</strong></code></a></td>
    <td>offset from  <a href='#Last_Point'>last point</a> to <a href='SkPath_Reference#Cubic'>cubic</a> <a href='SkPath_Reference#Cubic'>end</a> <a href='SkPath_Reference#Cubic'>on</a> <a href='SkPath_Reference#Cubic'>y-axis</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="19f0cfc7eeba8937fe19446ec0b5f932"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a> <a href='#SkPath_cubicTo'>cubicTo</a> <a href='#SkPath_quadTo'>quadTo</a>

<a name='Arc'></a>

---

<a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>can</a> <a href='undocumented#Arc'>be</a> <a href='undocumented#Arc'>constructed</a> <a href='undocumented#Arc'>in</a> <a href='undocumented#Arc'>a</a> <a href='undocumented#Arc'>number</a> <a href='undocumented#Arc'>of</a> <a href='undocumented#Arc'>ways</a>. <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>may</a> <a href='undocumented#Arc'>be</a> <a href='undocumented#Arc'>described</a> <a href='undocumented#Arc'>by</a> <a href='undocumented#Arc'>part</a> <a href='undocumented#Arc'>of</a> <a href='undocumented#Oval'>Oval</a> <a href='undocumented#Oval'>and</a> <a href='undocumented#Oval'>angles</a>,
<a href='undocumented#Oval'>by</a> <a href='undocumented#Oval'>start</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Point'>end</a> <a href='SkPoint_Reference#Point'>point</a>, <a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Point'>by</a> <a href='SkPoint_Reference#Point'>radius</a> <a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Point'>tangent</a> <a href='undocumented#Line'>lines</a>. <a href='undocumented#Line'>Each</a> <a href='undocumented#Line'>construction</a> <a href='undocumented#Line'>has</a> <a href='undocumented#Line'>advantages</a>,
<a href='undocumented#Line'>and</a> <a href='undocumented#Line'>some</a> <a href='undocumented#Line'>constructions</a> <a href='undocumented#Line'>correspond</a> <a href='undocumented#Line'>to</a> <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>drawing</a> <a href='undocumented#Arc'>in</a> <a href='undocumented#Arc'>graphics</a> <a href='undocumented#Arc'>standards</a>.

<a href='undocumented#Arc'>All</a> <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>draws</a> <a href='undocumented#Arc'>are</a> <a href='undocumented#Arc'>implemented</a> <a href='undocumented#Arc'>by</a> <a href='undocumented#Arc'>one</a> <a href='undocumented#Arc'>or</a> <a href='undocumented#Arc'>more</a> <a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>draws</a>. <a href='SkPath_Reference#Conic'>When</a> <a href='#Path_Conic_Weight'>Conic_Weight</a> <a href='#Path_Conic_Weight'>is</a> <a href='#Path_Conic_Weight'>less</a> <a href='#Path_Conic_Weight'>than</a> <a href='#Path_Conic_Weight'>one</a>,
<a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>describes</a> <a href='SkPath_Reference#Conic'>an</a> <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>of</a> <a href='undocumented#Arc'>some</a> <a href='undocumented#Oval'>Oval</a> <a href='undocumented#Oval'>or</a> <a href='undocumented#Circle'>Circle</a>.

Circle<a href='#SkPath_arcTo'>arcTo(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle, bool forceMoveTo)</a>
<a href='#SkPath_arcTo'>describes</a> <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>as</a> <a href='undocumented#Arc'>a</a> <a href='undocumented#Arc'>piece</a> <a href='undocumented#Arc'>of</a> <a href='undocumented#Oval'>Oval</a>, <a href='undocumented#Oval'>beginning</a> <a href='undocumented#Oval'>at</a> <a href='undocumented#Oval'>start</a> <a href='undocumented#Oval'>angle</a>, <a href='undocumented#Oval'>sweeping</a> <a href='undocumented#Oval'>clockwise</a> <a href='undocumented#Oval'>or</a> <a href='undocumented#Oval'>counterclockwise</a>,
<a href='undocumented#Oval'>which</a> <a href='undocumented#Oval'>may</a> <a href='undocumented#Oval'>continue</a> <a href='SkPath_Overview#Contour'>Contour</a> <a href='SkPath_Overview#Contour'>or</a> <a href='SkPath_Overview#Contour'>start</a> <a href='SkPath_Overview#Contour'>a</a> <a href='SkPath_Overview#Contour'>new</a> <a href='SkPath_Overview#Contour'>one</a>. <a href='SkPath_Overview#Contour'>This</a> <a href='SkPath_Overview#Contour'>construction</a> <a href='SkPath_Overview#Contour'>is</a> <a href='SkPath_Overview#Contour'>similar</a> <a href='SkPath_Overview#Contour'>to</a> <a href='undocumented#PostScript'>PostScript</a> <a href='undocumented#PostScript'>and</a>
<a href='#HTML_Canvas'>HTML_Canvas</a> <a href='undocumented#Arc'>arcs</a>. <a href='undocumented#Arc'>Variation</a> <a href='#SkPath_addArc'>addArc</a> <a href='#SkPath_addArc'>always</a> <a href='#SkPath_addArc'>starts</a> <a href='#SkPath_addArc'>new</a> <a href='SkPath_Overview#Contour'>Contour</a>. <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawArc'>drawArc</a> <a href='#SkCanvas_drawArc'>draws</a> <a href='#SkCanvas_drawArc'>without</a>
<a href='#SkCanvas_drawArc'>requiring</a> <a href='SkPath_Reference#Path'>Path</a>.

Path<a href='#SkPath_arcTo_2'>arcTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar radius)</a>
<a href='#SkPath_arcTo_2'>describes</a> <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>as</a> <a href='undocumented#Arc'>tangent</a> <a href='undocumented#Arc'>to</a> <a href='undocumented#Arc'>the</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>segment</a> <a href='undocumented#Line'>from</a> <a href='undocumented#Line'>last</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>added</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>to</a> (<a href='SkPath_Reference#Path'>x1</a>, <a href='SkPath_Reference#Path'>y1</a>); <a href='SkPath_Reference#Path'>and</a> <a href='SkPath_Reference#Path'>tangent</a>
<a href='SkPath_Reference#Path'>to</a> <a href='SkPath_Reference#Path'>the</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>segment</a> <a href='undocumented#Line'>from</a> (<a href='undocumented#Line'>x1</a>, <a href='undocumented#Line'>y1</a>) <a href='undocumented#Line'>to</a> (<a href='undocumented#Line'>x2</a>, <a href='undocumented#Line'>y2</a>). <a href='undocumented#Line'>This</a> <a href='undocumented#Line'>construction</a> <a href='undocumented#Line'>is</a> <a href='undocumented#Line'>similar</a> <a href='undocumented#Line'>to</a> <a href='undocumented#PostScript'>PostScript</a> <a href='undocumented#PostScript'>and</a>
<a href='#HTML_Canvas'>HTML_Canvas</a> <a href='undocumented#Arc'>arcs</a>.

arcs<a href='#SkPath_arcTo_4'>arcTo(SkScalar rx, SkScalar ry, SkScalar xAxisRotate, ArcSize largeArc, Direction sweep, SkScalar x, SkScalar y)</a>
<a href='#SkPath_arcTo_4'>describes</a> <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>as</a> <a href='undocumented#Arc'>part</a> <a href='undocumented#Arc'>of</a> <a href='undocumented#Oval'>Oval</a> <a href='undocumented#Oval'>with</a> <a href='undocumented#Oval'>radii</a> (<a href='undocumented#Oval'>rx</a>, <a href='undocumented#Oval'>ry</a>), <a href='undocumented#Oval'>beginning</a> <a href='undocumented#Oval'>at</a>
<a href='undocumented#Oval'>last</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>added</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>and</a> <a href='SkPath_Reference#Path'>ending</a> <a href='SkPath_Reference#Path'>at</a> (<a href='SkPath_Reference#Path'>x</a>, <a href='SkPath_Reference#Path'>y</a>). <a href='SkPath_Reference#Path'>More</a> <a href='SkPath_Reference#Path'>than</a> <a href='SkPath_Reference#Path'>one</a> <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>satisfies</a> <a href='undocumented#Arc'>this</a> <a href='undocumented#Arc'>criteria</a>,
<a href='undocumented#Arc'>so</a> <a href='undocumented#Arc'>additional</a> <a href='undocumented#Arc'>values</a> <a href='undocumented#Arc'>choose</a> <a href='undocumented#Arc'>a</a> <a href='undocumented#Arc'>single</a> <a href='undocumented#Arc'>solution</a>. <a href='undocumented#Arc'>This</a> <a href='undocumented#Arc'>construction</a> <a href='undocumented#Arc'>is</a> <a href='undocumented#Arc'>similar</a> <a href='undocumented#Arc'>to</a> <a href='undocumented#SVG'>SVG</a> <a href='undocumented#Arc'>arcs</a>.

<a href='#SkPath_conicTo'>conicTo</a> <a href='#SkPath_conicTo'>describes</a> <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>of</a> <a href='undocumented#Arc'>less</a> <a href='undocumented#Arc'>than</a> 180 <a href='undocumented#Arc'>degrees</a> <a href='undocumented#Arc'>as</a> <a href='undocumented#Arc'>a</a> <a href='undocumented#Arc'>pair</a> <a href='undocumented#Arc'>of</a> <a href='undocumented#Arc'>tangent</a> <a href='undocumented#Line'>lines</a> <a href='undocumented#Line'>and</a> <a href='#Path_Conic_Weight'>Conic_Weight</a>.
<a href='#SkPath_conicTo'>conicTo</a> <a href='#SkPath_conicTo'>can</a> <a href='#SkPath_conicTo'>represent</a> <a href='#SkPath_conicTo'>any</a> <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>with</a> <a href='undocumented#Arc'>a</a> <a href='undocumented#Arc'>sweep</a> <a href='undocumented#Arc'>less</a> <a href='undocumented#Arc'>than</a> 180 <a href='undocumented#Arc'>degrees</a> <a href='undocumented#Arc'>at</a> <a href='undocumented#Arc'>any</a> <a href='undocumented#Arc'>rotation</a>. <a href='undocumented#Arc'>All</a> <a href='#SkPath_arcTo'>arcTo</a>
<a href='#SkPath_arcTo'>constructions</a> <a href='#SkPath_arcTo'>are</a> <a href='#SkPath_arcTo'>converted</a> <a href='#SkPath_arcTo'>to</a> <a href='SkPath_Reference#Conic'>Conic</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>when</a> <a href='undocumented#Data'>added</a> <a href='undocumented#Data'>to</a> <a href='SkPath_Reference#Path'>Path</a>.

![Arc](https://fiddle.skia.org/i/e17e48e9d2182e9afc0f5d26b72c60f0_raster.png "")

<table>  <tr>
    <td><sup>1</sup> sup<a href='#SkPath_arcTo'>arcTo(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle, bool forceMoveTo)</a></td>
  </tr>  <tr>
    <td><sup>2</sup> parameter adds move to first <a href='SkPoint_Reference#Point'>point</a></td>
  </tr>  <tr>
    <td><sup>3</sup> start angle must be multiple of 90 degrees</td>
  </tr>  <tr>
    <td><sup>4</sup> sup<a href='#SkPath_arcTo_2'>arcTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar radius)</a></td>
  </tr>  <tr>
    <td><sup>5</sup> sup<a href='#SkPath_arcTo_4'>arcTo(SkScalar rx, SkScalar ry, SkScalar xAxisRotate, ArcSize largeArc, Direction sweep, SkScalar x, SkScalar y)</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="5acc77eba0cb4d00bbf3a8f4db0c0aee"></fiddle-embed></div>

In the example above:

<table>  <tr>
    <td>1 describes an <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>from</a> <a href='undocumented#Arc'>an</a> <a href='undocumented#Oval'>oval</a>, <a href='undocumented#Oval'>a</a> <a href='undocumented#Oval'>starting</a> <a href='undocumented#Oval'>angle</a>, <a href='undocumented#Oval'>and</a> <a href='undocumented#Oval'>a</a> <a href='undocumented#Oval'>sweep</a> <a href='undocumented#Oval'>angle</a>.</td>
  </tr>  <tr>
    <td>2 is similar to 1, but does not require building a <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>to</a> <a href='SkPath_Reference#Path'>draw</a>.</td>
  </tr>  <tr>
    <td>3 is similar to 1, but always begins new <a href='SkPath_Overview#Contour'>Contour</a>.</td>
  </tr>  <tr>
    <td>4 describes an <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>from</a> <a href='undocumented#Arc'>a</a> <a href='undocumented#Arc'>pair</a> <a href='undocumented#Arc'>of</a> <a href='undocumented#Arc'>tangent</a> <a href='undocumented#Line'>lines</a> <a href='undocumented#Line'>and</a> <a href='undocumented#Line'>a</a> <a href='undocumented#Line'>radius</a>.</td>
  </tr>  <tr>
    <td>5 describes an <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>from</a> <a href='undocumented#Oval'>Oval</a> <a href='undocumented#Oval'>center</a>, <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>start</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>and</a> <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>end</a> <a href='SkPoint_Reference#Point'>Point</a>.</td>
  </tr>  <tr>
    <td>6 describes an <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>from</a> <a href='undocumented#Arc'>a</a> <a href='undocumented#Arc'>pair</a> <a href='undocumented#Arc'>of</a> <a href='undocumented#Arc'>tangent</a> <a href='undocumented#Line'>lines</a> <a href='undocumented#Line'>and</a> <a href='undocumented#Line'>a</a> <a href='#Path_Conic_Weight'>Conic_Weight</a>.</td>
  </tr>
</table>

<a name='SkPath_arcTo'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_arcTo'>arcTo</a>(<a href='#SkPath_arcTo'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='undocumented#Oval'>oval</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>startAngle</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sweepAngle</a>, <a href='undocumented#SkScalar'>bool</a> <a href='undocumented#SkScalar'>forceMoveTo</a>)
</pre>

Appends <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>. <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>added</a> <a href='undocumented#Arc'>is</a> <a href='undocumented#Arc'>part</a> <a href='undocumented#Arc'>of</a> <a href='undocumented#Arc'>ellipse</a>
bounded by <a href='#SkPath_arcTo_oval'>oval</a>, <a href='#SkPath_arcTo_oval'>from</a> <a href='#SkPath_arcTo_startAngle'>startAngle</a> <a href='#SkPath_arcTo_startAngle'>through</a> <a href='#SkPath_arcTo_sweepAngle'>sweepAngle</a>. <a href='#SkPath_arcTo_sweepAngle'>Both</a> <a href='#SkPath_arcTo_startAngle'>startAngle</a> <a href='#SkPath_arcTo_startAngle'>and</a>
<a href='#SkPath_arcTo_sweepAngle'>sweepAngle</a> <a href='#SkPath_arcTo_sweepAngle'>are</a> <a href='#SkPath_arcTo_sweepAngle'>measured</a> <a href='#SkPath_arcTo_sweepAngle'>in</a> <a href='#SkPath_arcTo_sweepAngle'>degrees</a>, <a href='#SkPath_arcTo_sweepAngle'>where</a> <a href='#SkPath_arcTo_sweepAngle'>zero</a> <a href='#SkPath_arcTo_sweepAngle'>degrees</a> <a href='#SkPath_arcTo_sweepAngle'>is</a> <a href='#SkPath_arcTo_sweepAngle'>aligned</a> <a href='#SkPath_arcTo_sweepAngle'>with</a> <a href='#SkPath_arcTo_sweepAngle'>the</a>
positive x-axis, and positive sweeps extends <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>clockwise</a>.

<a href='#SkPath_arcTo'>arcTo</a>() <a href='#SkPath_arcTo'>adds</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>connecting</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>last</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>to</a> <a href='SkPoint_Reference#SkPoint'>initial</a> <a href='undocumented#Arc'>arc</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>if</a> <a href='#SkPath_arcTo_forceMoveTo'>forceMoveTo</a>
is false and <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>not</a> <a href='SkPath_Reference#SkPath'>empty</a>. <a href='SkPath_Reference#SkPath'>Otherwise</a>, <a href='SkPath_Reference#SkPath'>added</a> <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>begins</a> <a href='SkPath_Overview#Contour'>with</a> <a href='SkPath_Overview#Contour'>first</a> <a href='SkPoint_Reference#Point'>point</a>
of <a href='undocumented#Arc'>arc</a>. <a href='undocumented#Arc'>Angles</a> <a href='undocumented#Arc'>greater</a> <a href='undocumented#Arc'>than</a> -360 <a href='undocumented#Arc'>and</a> <a href='undocumented#Arc'>less</a> <a href='undocumented#Arc'>than</a> 360 <a href='undocumented#Arc'>are</a> <a href='undocumented#Arc'>treated</a> <a href='undocumented#Arc'>modulo</a> 360.

### Parameters

<table>  <tr>    <td><a name='SkPath_arcTo_oval'><code><strong>oval</strong></code></a></td>
    <td>bounds of ellipse containing <a href='undocumented#Arc'>arc</a></td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_startAngle'><code><strong>startAngle</strong></code></a></td>
    <td>starting angle of <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>in</a> <a href='undocumented#Arc'>degrees</a></td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_sweepAngle'><code><strong>sweepAngle</strong></code></a></td>
    <td>sweep, in degrees. Positive is clockwise; treated modulo 360</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_forceMoveTo'><code><strong>forceMoveTo</strong></code></a></td>
    <td>true to start a new <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>with</a> <a href='undocumented#Arc'>arc</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="5f02890edaa10cb5e1a4243a82b6a382"><div><a href='#SkPath_arcTo'>arcTo</a> <a href='#SkPath_arcTo'>continues</a> <a href='#SkPath_arcTo'>a</a> <a href='#SkPath_arcTo'>previous</a> <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>when</a> <a href='#SkPath_arcTo_forceMoveTo'>forceMoveTo</a> <a href='#SkPath_arcTo_forceMoveTo'>is</a> <a href='#SkPath_arcTo_forceMoveTo'>false</a> <a href='#SkPath_arcTo_forceMoveTo'>and</a> <a href='#SkPath_arcTo_forceMoveTo'>when</a> <a href='SkPath_Reference#Path'>Path</a>
<a href='SkPath_Reference#Path'>is</a> <a href='SkPath_Reference#Path'>not</a> <a href='SkPath_Reference#Path'>empty</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_addArc'>addArc</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawArc'>drawArc</a> <a href='#SkPath_conicTo'>conicTo</a>

<a name='SkPath_arcTo_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_arcTo'>arcTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>radius</a>)
</pre>

Appends <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>, <a href='SkPath_Reference#SkPath'>after</a> <a href='SkPath_Reference#SkPath'>appending</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>if</a> <a href='undocumented#Line'>needed</a>. <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>is</a> <a href='undocumented#Arc'>implemented</a> <a href='undocumented#Arc'>by</a> <a href='SkPath_Reference#Conic'>conic</a>
weighted to describe part of <a href='undocumented#Circle'>circle</a>. <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>is</a> <a href='undocumented#Arc'>contained</a> <a href='undocumented#Arc'>by</a> <a href='undocumented#Arc'>tangent</a> <a href='undocumented#Arc'>from</a>
last <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>to</a> (<a href='#SkPath_arcTo_2_x1'>x1</a>, <a href='#SkPath_arcTo_2_y1'>y1</a>), <a href='#SkPath_arcTo_2_y1'>and</a> <a href='#SkPath_arcTo_2_y1'>tangent</a> <a href='#SkPath_arcTo_2_y1'>from</a> (<a href='#SkPath_arcTo_2_x1'>x1</a>, <a href='#SkPath_arcTo_2_y1'>y1</a>) <a href='#SkPath_arcTo_2_y1'>to</a> (<a href='#SkPath_arcTo_2_x2'>x2</a>, <a href='#SkPath_arcTo_2_y2'>y2</a>). <a href='undocumented#Arc'>Arc</a>
is part of <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>sized</a> <a href='undocumented#Circle'>to</a> <a href='#SkPath_arcTo_2_radius'>radius</a>, <a href='#SkPath_arcTo_2_radius'>positioned</a> <a href='#SkPath_arcTo_2_radius'>so</a> <a href='#SkPath_arcTo_2_radius'>it</a> <a href='#SkPath_arcTo_2_radius'>touches</a> <a href='#SkPath_arcTo_2_radius'>both</a> <a href='#SkPath_arcTo_2_radius'>tangent</a> <a href='undocumented#Line'>lines</a>.

If last <a href='SkPath_Reference#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>does</a> <a href='SkPoint_Reference#Point'>not</a> <a href='SkPoint_Reference#Point'>start</a> <a href='undocumented#Arc'>Arc</a>, <a href='#SkPath_arcTo'>arcTo</a> <a href='#SkPath_arcTo'>appends</a> <a href='#SkPath_arcTo'>connecting</a> <a href='undocumented#Line'>Line</a> <a href='undocumented#Line'>to</a> <a href='SkPath_Reference#Path'>Path</a>.
The length of <a href='SkPoint_Reference#Vector'>Vector</a> <a href='SkPoint_Reference#Vector'>from</a> (<a href='#SkPath_arcTo_2_x1'>x1</a>, <a href='#SkPath_arcTo_2_y1'>y1</a>) <a href='#SkPath_arcTo_2_y1'>to</a> (<a href='#SkPath_arcTo_2_x2'>x2</a>, <a href='#SkPath_arcTo_2_y2'>y2</a>) <a href='#SkPath_arcTo_2_y2'>does</a> <a href='#SkPath_arcTo_2_y2'>not</a> <a href='#SkPath_arcTo_2_y2'>affect</a> <a href='undocumented#Arc'>Arc</a>.

<a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>sweep</a> <a href='undocumented#Arc'>is</a> <a href='undocumented#Arc'>always</a> <a href='undocumented#Arc'>less</a> <a href='undocumented#Arc'>than</a> 180 <a href='undocumented#Arc'>degrees</a>. <a href='undocumented#Arc'>If</a> <a href='#SkPath_arcTo_2_radius'>radius</a> <a href='#SkPath_arcTo_2_radius'>is</a> <a href='#SkPath_arcTo_2_radius'>zero</a>, <a href='#SkPath_arcTo_2_radius'>or</a> <a href='#SkPath_arcTo_2_radius'>if</a>
tangents are nearly parallel, <a href='#SkPath_arcTo'>arcTo</a> <a href='#SkPath_arcTo'>appends</a> <a href='undocumented#Line'>Line</a> <a href='undocumented#Line'>from</a> <a href='undocumented#Line'>last</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>to</a> (<a href='#SkPath_arcTo_2_x1'>x1</a>, <a href='#SkPath_arcTo_2_y1'>y1</a>).

<a href='#SkPath_arcTo'>arcTo</a> <a href='#SkPath_arcTo'>appends</a> <a href='#SkPath_arcTo'>at</a> <a href='#SkPath_arcTo'>most</a> <a href='#SkPath_arcTo'>one</a> <a href='undocumented#Line'>Line</a> <a href='undocumented#Line'>and</a> <a href='undocumented#Line'>one</a> <a href='SkPath_Reference#Conic'>conic</a>.
<a href='#SkPath_arcTo'>arcTo</a> <a href='#SkPath_arcTo'>implements</a> <a href='#SkPath_arcTo'>the</a> <a href='#SkPath_arcTo'>functionality</a> <a href='#SkPath_arcTo'>of</a>  <a href='undocumented#Arct'>PostScript arct</a> <a href='undocumented#PostScript'>and</a>   <a href='undocumented#ArcTo'>HTML Canvas arcTo</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_arcTo_2_x1'><code><strong>x1</strong></code></a></td>
    <td>x-axis value common to pair of tangents</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_2_y1'><code><strong>y1</strong></code></a></td>
    <td>y-axis value common to pair of tangents</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_2_x2'><code><strong>x2</strong></code></a></td>
    <td>x-axis value end of second tangent</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_2_y2'><code><strong>y2</strong></code></a></td>
    <td>y-axis value end of second tangent</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_2_radius'><code><strong>radius</strong></code></a></td>
    <td>distance from <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>to</a> <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>center</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="386000684073fccabc224d7d6dc81cd9"></fiddle-embed></div>

### Example

<div><fiddle-embed name="78f3c65fa900610bb52518989b547095"></fiddle-embed></div>

### Example

<div><fiddle-embed name="498360fa0a201cc5db04b1c27256358f"><div><a href='#SkPath_arcTo'>arcTo</a> <a href='#SkPath_arcTo'>is</a> <a href='#SkPath_arcTo'>represented</a> <a href='#SkPath_arcTo'>by</a> <a href='undocumented#Line'>Line</a> <a href='undocumented#Line'>and</a> <a href='undocumented#Line'>circular</a> <a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>in</a> <a href='SkPath_Reference#Path'>Path</a>.
</div>

#### Example Output

~~~~
move to (156,20)
line (156,20),(79.2893,20)
conic (79.2893,20),(200,20),(114.645,105.355) weight 0.382683
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_conicTo'>conicTo</a>

<a name='SkPath_arcTo_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_arcTo'>arcTo</a>(<a href='#SkPath_arcTo'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>p1</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>p2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>radius</a>)
</pre>

Appends <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>, <a href='SkPath_Reference#SkPath'>after</a> <a href='SkPath_Reference#SkPath'>appending</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>if</a> <a href='undocumented#Line'>needed</a>. <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>is</a> <a href='undocumented#Arc'>implemented</a> <a href='undocumented#Arc'>by</a> <a href='SkPath_Reference#Conic'>conic</a>
weighted to describe part of <a href='undocumented#Circle'>circle</a>. <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>is</a> <a href='undocumented#Arc'>contained</a> <a href='undocumented#Arc'>by</a> <a href='undocumented#Arc'>tangent</a> <a href='undocumented#Arc'>from</a>
last <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>to</a> <a href='#SkPath_arcTo_3_p1'>p1</a>, <a href='#SkPath_arcTo_3_p1'>and</a> <a href='#SkPath_arcTo_3_p1'>tangent</a> <a href='#SkPath_arcTo_3_p1'>from</a> <a href='#SkPath_arcTo_3_p1'>p1</a> <a href='#SkPath_arcTo_3_p1'>to</a> <a href='#SkPath_arcTo_3_p2'>p2</a>. <a href='undocumented#Arc'>Arc</a>
is part of <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>sized</a> <a href='undocumented#Circle'>to</a> <a href='#SkPath_arcTo_3_radius'>radius</a>, <a href='#SkPath_arcTo_3_radius'>positioned</a> <a href='#SkPath_arcTo_3_radius'>so</a> <a href='#SkPath_arcTo_3_radius'>it</a> <a href='#SkPath_arcTo_3_radius'>touches</a> <a href='#SkPath_arcTo_3_radius'>both</a> <a href='#SkPath_arcTo_3_radius'>tangent</a> <a href='undocumented#Line'>lines</a>.

If last <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>does</a> <a href='SkPoint_Reference#SkPoint'>not</a> <a href='SkPoint_Reference#SkPoint'>start</a> <a href='undocumented#Arc'>arc</a>, <a href='#SkPath_arcTo'>arcTo</a>() <a href='#SkPath_arcTo'>appends</a> <a href='#SkPath_arcTo'>connecting</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>.
The length of <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>from</a> <a href='#SkPath_arcTo_3_p1'>p1</a> <a href='#SkPath_arcTo_3_p1'>to</a> <a href='#SkPath_arcTo_3_p2'>p2</a> <a href='#SkPath_arcTo_3_p2'>does</a> <a href='#SkPath_arcTo_3_p2'>not</a> <a href='#SkPath_arcTo_3_p2'>affect</a> <a href='undocumented#Arc'>arc</a>.

<a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>sweep</a> <a href='undocumented#Arc'>is</a> <a href='undocumented#Arc'>always</a> <a href='undocumented#Arc'>less</a> <a href='undocumented#Arc'>than</a> 180 <a href='undocumented#Arc'>degrees</a>. <a href='undocumented#Arc'>If</a> <a href='#SkPath_arcTo_3_radius'>radius</a> <a href='#SkPath_arcTo_3_radius'>is</a> <a href='#SkPath_arcTo_3_radius'>zero</a>, <a href='#SkPath_arcTo_3_radius'>or</a> <a href='#SkPath_arcTo_3_radius'>if</a>
tangents are nearly parallel, <a href='#SkPath_arcTo'>arcTo</a>() <a href='#SkPath_arcTo'>appends</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>from</a> <a href='undocumented#Line'>last</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>to</a> <a href='#SkPath_arcTo_3_p1'>p1</a>.

<a href='#SkPath_arcTo'>arcTo</a>() <a href='#SkPath_arcTo'>appends</a> <a href='#SkPath_arcTo'>at</a> <a href='#SkPath_arcTo'>most</a> <a href='#SkPath_arcTo'>one</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>and</a> <a href='undocumented#Line'>one</a> <a href='SkPath_Reference#Conic'>conic</a>.
<a href='#SkPath_arcTo'>arcTo</a>() <a href='#SkPath_arcTo'>implements</a> <a href='#SkPath_arcTo'>the</a> <a href='#SkPath_arcTo'>functionality</a> <a href='#SkPath_arcTo'>of</a>  <a href='undocumented#Arct'>PostScript arct</a> <a href='undocumented#PostScript'>and</a>   <a href='undocumented#ArcTo'>HTML Canvas arcTo</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_arcTo_3_p1'><code><strong>p1</strong></code></a></td>
    <td><a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>common</a> <a href='SkPoint_Reference#SkPoint'>to</a> <a href='SkPoint_Reference#SkPoint'>pair</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPoint_Reference#SkPoint'>tangents</a></td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_3_p2'><code><strong>p2</strong></code></a></td>
    <td>end of second tangent</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_3_radius'><code><strong>radius</strong></code></a></td>
    <td>distance from <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>to</a> <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>center</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="0c056264a361579c18e5d02d3172d4d4"><div>Because tangent <a href='undocumented#Line'>lines</a> <a href='undocumented#Line'>are</a> <a href='undocumented#Line'>parallel</a>, <a href='#SkPath_arcTo'>arcTo</a> <a href='#SkPath_arcTo'>appends</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>from</a> <a href='undocumented#Line'>last</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>to</a>
<a href='#SkPath_arcTo_3_p1'>p1</a>, <a href='#SkPath_arcTo_3_p1'>but</a> <a href='#SkPath_arcTo_3_p1'>does</a> <a href='#SkPath_arcTo_3_p1'>not</a> <a href='#SkPath_arcTo_3_p1'>append</a> <a href='#SkPath_arcTo_3_p1'>a</a> <a href='#SkPath_arcTo_3_p1'>circular</a> <a href='SkPath_Reference#Conic'>Conic</a>.
</div>

#### Example Output

~~~~
move to (156,20)
line (156,20),(200,20)
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_conicTo'>conicTo</a>

<a name='SkPath_ArcSize'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPath_ArcSize'>ArcSize</a> {
        <a href='#SkPath_kSmall_ArcSize'>kSmall_ArcSize</a>,
        <a href='#SkPath_kLarge_ArcSize'>kLarge_ArcSize</a>,
    };
</pre>

Four axis-aligned <a href='undocumented#Oval'>Ovals</a> <a href='undocumented#Oval'>with</a> <a href='undocumented#Oval'>the</a> <a href='undocumented#Oval'>same</a> <a href='undocumented#Oval'>height</a> <a href='undocumented#Oval'>and</a> <a href='undocumented#Oval'>width</a> <a href='undocumented#Oval'>intersect</a> <a href='undocumented#Oval'>a</a> <a href='undocumented#Oval'>pair</a> <a href='undocumented#Oval'>of</a> <a href='SkPoint_Reference#Point'>Points</a>.
<a href='#SkPath_ArcSize'>ArcSize</a> <a href='#SkPath_ArcSize'>and</a> <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>select</a> <a href='#SkPath_Direction'>one</a> <a href='#SkPath_Direction'>of</a> <a href='#SkPath_Direction'>the</a> <a href='#SkPath_Direction'>four</a> <a href='undocumented#Oval'>Ovals</a>, <a href='undocumented#Oval'>by</a> <a href='undocumented#Oval'>choosing</a> <a href='undocumented#Oval'>the</a> <a href='undocumented#Oval'>larger</a> <a href='undocumented#Oval'>or</a> <a href='undocumented#Oval'>smaller</a>
<a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>between</a> <a href='undocumented#Arc'>the</a> <a href='SkPoint_Reference#Point'>Points</a>; <a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Point'>by</a> <a href='SkPoint_Reference#Point'>choosing</a> <a href='SkPoint_Reference#Point'>the</a> <a href='undocumented#Arc'>arc</a> <a href='#SkPath_Direction'>Direction</a>, <a href='#SkPath_Direction'>clockwise</a>
<a href='#SkPath_Direction'>or</a> <a href='#SkPath_Direction'>counterclockwise</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kSmall_ArcSize'><code>SkPath::kSmall_ArcSize</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
smaller of Arc pair</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kLarge_ArcSize'><code>SkPath::kLarge_ArcSize</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
larger of Arc pair</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="8e40c546eecd9cc213200717240898ba"><div><a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>begins</a> <a href='undocumented#Arc'>at</a> <a href='undocumented#Arc'>top</a> <a href='undocumented#Arc'>of</a> <a href='undocumented#Oval'>Oval</a> <a href='undocumented#Oval'>pair</a> <a href='undocumented#Oval'>and</a> <a href='undocumented#Oval'>ends</a> <a href='undocumented#Oval'>at</a> <a href='undocumented#Oval'>bottom</a>. <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>can</a> <a href='undocumented#Arc'>take</a> <a href='undocumented#Arc'>four</a> <a href='undocumented#Arc'>routes</a> <a href='undocumented#Arc'>to</a> <a href='undocumented#Arc'>get</a> <a href='undocumented#Arc'>there</a>.
<a href='undocumented#Arc'>Two</a> <a href='undocumented#Arc'>routes</a> <a href='undocumented#Arc'>are</a> <a href='undocumented#Arc'>large</a>, <a href='undocumented#Arc'>and</a> <a href='undocumented#Arc'>two</a> <a href='undocumented#Arc'>routes</a> <a href='undocumented#Arc'>are</a> <a href='undocumented#Arc'>counterclockwise</a>. <a href='undocumented#Arc'>The</a> <a href='undocumented#Arc'>one</a> <a href='undocumented#Arc'>route</a> <a href='undocumented#Arc'>both</a> <a href='undocumented#Arc'>large</a>
<a href='undocumented#Arc'>and</a> <a href='undocumented#Arc'>counterclockwise</a> <a href='undocumented#Arc'>is</a> <a href='undocumented#Arc'>blue</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_arcTo'>arcTo</a> <a href='#SkPath_Direction'>Direction</a>

<a name='SkPath_arcTo_4'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_arcTo'>arcTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>rx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>ry</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>xAxisRotate</a>, <a href='#SkPath_ArcSize'>ArcSize</a> <a href='#SkPath_ArcSize'>largeArc</a>, <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>sweep</a>,
              <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>)
</pre>

Appends <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>. <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>is</a> <a href='undocumented#Arc'>implemented</a> <a href='undocumented#Arc'>by</a> <a href='undocumented#Arc'>one</a> <a href='undocumented#Arc'>or</a> <a href='undocumented#Arc'>more</a> <a href='SkPath_Reference#Conic'>conics</a> <a href='SkPath_Reference#Conic'>weighted</a> <a href='SkPath_Reference#Conic'>to</a>
describe part of <a href='undocumented#Oval'>oval</a> <a href='undocumented#Oval'>with</a> <a href='undocumented#Oval'>radii</a> (<a href='#SkPath_arcTo_4_rx'>rx</a>, <a href='#SkPath_arcTo_4_ry'>ry</a>) <a href='#SkPath_arcTo_4_ry'>rotated</a> <a href='#SkPath_arcTo_4_ry'>by</a> <a href='#SkPath_arcTo_4_xAxisRotate'>xAxisRotate</a> <a href='#SkPath_arcTo_4_xAxisRotate'>degrees</a>. <a href='undocumented#Arc'>Arc</a>
<a href='undocumented#Curve'>curves</a> <a href='undocumented#Curve'>from</a> <a href='undocumented#Curve'>last</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>to</a> (<a href='#SkPath_arcTo_4_x'>x</a>, <a href='#SkPath_arcTo_4_y'>y</a>), <a href='#SkPath_arcTo_4_y'>choosing</a> <a href='#SkPath_arcTo_4_y'>one</a> <a href='#SkPath_arcTo_4_y'>of</a> <a href='#SkPath_arcTo_4_y'>four</a> <a href='#SkPath_arcTo_4_y'>possible</a> <a href='#SkPath_arcTo_4_y'>routes</a>:
clockwise or counterclockwise, and smaller or larger.

<a href='undocumented#Arc'>Arc</a> <a href='#SkPath_arcTo_4_sweep'>sweep</a> <a href='#SkPath_arcTo_4_sweep'>is</a> <a href='#SkPath_arcTo_4_sweep'>always</a> <a href='#SkPath_arcTo_4_sweep'>less</a> <a href='#SkPath_arcTo_4_sweep'>than</a> 360 <a href='#SkPath_arcTo_4_sweep'>degrees</a>. <a href='#SkPath_arcTo'>arcTo</a>() <a href='#SkPath_arcTo'>appends</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>to</a> (<a href='#SkPath_arcTo_4_x'>x</a>, <a href='#SkPath_arcTo_4_y'>y</a>) <a href='#SkPath_arcTo_4_y'>if</a>
either radii are zero, or if last <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>equals</a> (<a href='#SkPath_arcTo_4_x'>x</a>, <a href='#SkPath_arcTo_4_y'>y</a>). <a href='#SkPath_arcTo'>arcTo</a>() <a href='#SkPath_arcTo'>scales</a> <a href='#SkPath_arcTo'>radii</a>
(<a href='#SkPath_arcTo_4_rx'>rx</a>, <a href='#SkPath_arcTo_4_ry'>ry</a>) <a href='#SkPath_arcTo_4_ry'>to</a> <a href='#SkPath_arcTo_4_ry'>fit</a> <a href='#SkPath_arcTo_4_ry'>last</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>and</a> (<a href='#SkPath_arcTo_4_x'>x</a>, <a href='#SkPath_arcTo_4_y'>y</a>) <a href='#SkPath_arcTo_4_y'>if</a> <a href='#SkPath_arcTo_4_y'>both</a> <a href='#SkPath_arcTo_4_y'>are</a> <a href='#SkPath_arcTo_4_y'>greater</a> <a href='#SkPath_arcTo_4_y'>than</a> <a href='#SkPath_arcTo_4_y'>zero</a> <a href='#SkPath_arcTo_4_y'>but</a>
too small.

<a href='#SkPath_arcTo'>arcTo</a>() <a href='#SkPath_arcTo'>appends</a> <a href='#SkPath_arcTo'>up</a> <a href='#SkPath_arcTo'>to</a> <a href='#SkPath_arcTo'>four</a> <a href='SkPath_Reference#Conic'>conic</a> <a href='undocumented#Curve'>curves</a>.
<a href='#SkPath_arcTo'>arcTo</a>() <a href='#SkPath_arcTo'>implements</a> <a href='#SkPath_arcTo'>the</a> <a href='#SkPath_arcTo'>functionality</a> <a href='#SkPath_arcTo'>of</a>  <a href='undocumented#SVG_Arc'>SVG arc</a>, <a href='undocumented#SVG'>although</a>  <a href='undocumented#Sweep_Flag'>SVG sweep-flag</a> <a href='undocumented#SVG'>value</a>
is opposite the integer value of <a href='#SkPath_arcTo_4_sweep'>sweep</a>;  <a href='undocumented#Sweep_Flag'>SVG sweep-flag</a> <a href='undocumented#SVG'>uses</a> 1 <a href='undocumented#SVG'>for</a> <a href='undocumented#SVG'>clockwise</a>,
while <a href='#SkPath_kCW_Direction'>kCW_Direction</a> <a href='#SkPath_kCW_Direction'>cast</a> <a href='#SkPath_kCW_Direction'>to</a> <a href='#SkPath_kCW_Direction'>int</a> <a href='#SkPath_kCW_Direction'>is</a> <a href='#SkPath_kCW_Direction'>zero</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_arcTo_4_rx'><code><strong>rx</strong></code></a></td>
    <td>radius on x-axis before x-axis rotation</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_4_ry'><code><strong>ry</strong></code></a></td>
    <td>radius on y-axis before x-axis rotation</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_4_xAxisRotate'><code><strong>xAxisRotate</strong></code></a></td>
    <td>x-axis rotation in degrees; positive values are clockwise</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_4_largeArc'><code><strong>largeArc</strong></code></a></td>
    <td>chooses smaller or larger <a href='undocumented#Arc'>arc</a></td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_4_sweep'><code><strong>sweep</strong></code></a></td>
    <td>chooses clockwise or counterclockwise <a href='undocumented#Arc'>arc</a></td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_4_x'><code><strong>x</strong></code></a></td>
    <td>end of <a href='undocumented#Arc'>arc</a></td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_4_y'><code><strong>y</strong></code></a></td>
    <td>end of <a href='undocumented#Arc'>arc</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="6b6ea44f659b27918f3a6fa621bf6173"></fiddle-embed></div>

### See Also

<a href='#SkPath_rArcTo'>rArcTo</a> <a href='#SkPath_ArcSize'>ArcSize</a> <a href='#SkPath_Direction'>Direction</a>

<a name='SkPath_arcTo_5'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_arcTo'>arcTo</a>(<a href='#SkPath_arcTo'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>r</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>xAxisRotate</a>, <a href='#SkPath_ArcSize'>ArcSize</a> <a href='#SkPath_ArcSize'>largeArc</a>, <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>sweep</a>,
              <a href='#SkPath_Direction'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>xy</a>)
</pre>

Appends <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>. <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>is</a> <a href='undocumented#Arc'>implemented</a> <a href='undocumented#Arc'>by</a> <a href='undocumented#Arc'>one</a> <a href='undocumented#Arc'>or</a> <a href='undocumented#Arc'>more</a> <a href='SkPath_Reference#Conic'>conic</a> <a href='SkPath_Reference#Conic'>weighted</a> <a href='SkPath_Reference#Conic'>to</a> <a href='SkPath_Reference#Conic'>describe</a>
part of <a href='undocumented#Oval'>oval</a> <a href='undocumented#Oval'>with</a> <a href='undocumented#Oval'>radii</a> (<a href='#SkPath_arcTo_5_r'>r</a>.<a href='#SkPoint_fX'>fX</a>, <a href='#SkPath_arcTo_5_r'>r</a>.<a href='#SkPoint_fY'>fY</a>) <a href='#SkPoint_fY'>rotated</a> <a href='#SkPoint_fY'>by</a> <a href='#SkPath_arcTo_5_xAxisRotate'>xAxisRotate</a> <a href='#SkPath_arcTo_5_xAxisRotate'>degrees</a>. <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Curve'>curves</a>
from last <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>to</a> (<a href='#SkPath_arcTo_5_xy'>xy</a>.<a href='#SkPoint_fX'>fX</a>, <a href='#SkPath_arcTo_5_xy'>xy</a>.<a href='#SkPoint_fY'>fY</a>), <a href='#SkPoint_fY'>choosing</a> <a href='#SkPoint_fY'>one</a> <a href='#SkPoint_fY'>of</a> <a href='#SkPoint_fY'>four</a> <a href='#SkPoint_fY'>possible</a> <a href='#SkPoint_fY'>routes</a>:
clockwise or counterclockwise,
and smaller or larger.

<a href='undocumented#Arc'>Arc</a> <a href='#SkPath_arcTo_5_sweep'>sweep</a> <a href='#SkPath_arcTo_5_sweep'>is</a> <a href='#SkPath_arcTo_5_sweep'>always</a> <a href='#SkPath_arcTo_5_sweep'>less</a> <a href='#SkPath_arcTo_5_sweep'>than</a> 360 <a href='#SkPath_arcTo_5_sweep'>degrees</a>. <a href='#SkPath_arcTo'>arcTo</a>() <a href='#SkPath_arcTo'>appends</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>to</a> <a href='#SkPath_arcTo_5_xy'>xy</a> <a href='#SkPath_arcTo_5_xy'>if</a> <a href='#SkPath_arcTo_5_xy'>either</a>
radii are zero, or if last <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>equals</a> (<a href='#SkPath_arcTo_5_xy'>xy</a>.<a href='#SkPoint_fX'>fX</a>, <a href='#SkPath_arcTo_5_xy'>xy</a>.<a href='#SkPoint_fY'>fY</a>). <a href='#SkPath_arcTo'>arcTo</a>() <a href='#SkPath_arcTo'>scales</a> <a href='#SkPath_arcTo'>radii</a> <a href='#SkPath_arcTo_5_r'>r</a> <a href='#SkPath_arcTo_5_r'>to</a>
fit last <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>and</a> <a href='#SkPath_arcTo_5_xy'>xy</a> <a href='#SkPath_arcTo_5_xy'>if</a> <a href='#SkPath_arcTo_5_xy'>both</a> <a href='#SkPath_arcTo_5_xy'>are</a> <a href='#SkPath_arcTo_5_xy'>greater</a> <a href='#SkPath_arcTo_5_xy'>than</a> <a href='#SkPath_arcTo_5_xy'>zero</a> <a href='#SkPath_arcTo_5_xy'>but</a> <a href='#SkPath_arcTo_5_xy'>too</a> <a href='#SkPath_arcTo_5_xy'>small</a> <a href='#SkPath_arcTo_5_xy'>to</a> <a href='#SkPath_arcTo_5_xy'>describe</a>
an <a href='undocumented#Arc'>arc</a>.

<a href='#SkPath_arcTo'>arcTo</a>() <a href='#SkPath_arcTo'>appends</a> <a href='#SkPath_arcTo'>up</a> <a href='#SkPath_arcTo'>to</a> <a href='#SkPath_arcTo'>four</a> <a href='SkPath_Reference#Conic'>conic</a> <a href='undocumented#Curve'>curves</a>.
<a href='#SkPath_arcTo'>arcTo</a>() <a href='#SkPath_arcTo'>implements</a> <a href='#SkPath_arcTo'>the</a> <a href='#SkPath_arcTo'>functionality</a> <a href='#SkPath_arcTo'>of</a>  <a href='undocumented#SVG_Arc'>SVG arc</a>, <a href='undocumented#SVG'>although</a>  <a href='undocumented#Sweep_Flag'>SVG sweep-flag</a> <a href='undocumented#SVG'>value</a> <a href='undocumented#SVG'>is</a>
opposite the integer value of <a href='#SkPath_arcTo_5_sweep'>sweep</a>;  <a href='undocumented#Sweep_Flag'>SVG sweep-flag</a> <a href='undocumented#SVG'>uses</a> 1 <a href='undocumented#SVG'>for</a> <a href='undocumented#SVG'>clockwise</a>, <a href='undocumented#SVG'>while</a>
<a href='#SkPath_kCW_Direction'>kCW_Direction</a> <a href='#SkPath_kCW_Direction'>cast</a> <a href='#SkPath_kCW_Direction'>to</a> <a href='#SkPath_kCW_Direction'>int</a> <a href='#SkPath_kCW_Direction'>is</a> <a href='#SkPath_kCW_Direction'>zero</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_arcTo_5_r'><code><strong>r</strong></code></a></td>
    <td>radii on axes before x-axis rotation</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_5_xAxisRotate'><code><strong>xAxisRotate</strong></code></a></td>
    <td>x-axis rotation in degrees; positive values are clockwise</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_5_largeArc'><code><strong>largeArc</strong></code></a></td>
    <td>chooses smaller or larger <a href='undocumented#Arc'>arc</a></td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_5_sweep'><code><strong>sweep</strong></code></a></td>
    <td>chooses clockwise or counterclockwise <a href='undocumented#Arc'>arc</a></td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_5_xy'><code><strong>xy</strong></code></a></td>
    <td>end of <a href='undocumented#Arc'>arc</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="3f76a1007416181a4848c1a87fc81dbd"></fiddle-embed></div>

### See Also

<a href='#SkPath_rArcTo'>rArcTo</a> <a href='#SkPath_ArcSize'>ArcSize</a> <a href='#SkPath_Direction'>Direction</a>

<a name='SkPath_rArcTo'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_rArcTo'>rArcTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>rx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>ry</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>xAxisRotate</a>, <a href='#SkPath_ArcSize'>ArcSize</a> <a href='#SkPath_ArcSize'>largeArc</a>, <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>sweep</a>,
               <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>)
</pre>

Appends <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>to</a> <a href='SkPath_Reference#Path'>Path</a>, <a href='SkPath_Reference#Path'>relative</a> <a href='SkPath_Reference#Path'>to</a> <a href='SkPath_Reference#Path'>last</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a>. <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>is</a> <a href='undocumented#Arc'>implemented</a> <a href='undocumented#Arc'>by</a> <a href='undocumented#Arc'>one</a> <a href='undocumented#Arc'>or</a>
<a href='undocumented#Arc'>more</a> <a href='SkPath_Reference#Conic'>Conic</a>, <a href='SkPath_Reference#Conic'>weighted</a> <a href='SkPath_Reference#Conic'>to</a> <a href='SkPath_Reference#Conic'>describe</a> <a href='SkPath_Reference#Conic'>part</a> <a href='SkPath_Reference#Conic'>of</a> <a href='undocumented#Oval'>Oval</a> <a href='undocumented#Oval'>with</a> <a href='undocumented#Oval'>radii</a> (<a href='#SkPath_rArcTo_rx'>rx</a>, <a href='#SkPath_rArcTo_ry'>ry</a>) <a href='#SkPath_rArcTo_ry'>rotated</a> <a href='#SkPath_rArcTo_ry'>by</a>
<a href='#SkPath_rArcTo_xAxisRotate'>xAxisRotate</a> <a href='#SkPath_rArcTo_xAxisRotate'>degrees</a>. <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Curve'>curves</a> <a href='undocumented#Curve'>from</a> <a href='undocumented#Curve'>last</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>relative</a> <a href='SkPoint_Reference#Point'>end</a> <a href='SkPoint_Reference#Point'>Point</a>
(<a href='#SkPath_rArcTo_dx'>dx</a>, <a href='#SkPath_rArcTo_dy'>dy</a>), <a href='#SkPath_rArcTo_dy'>choosing</a> <a href='#SkPath_rArcTo_dy'>one</a> <a href='#SkPath_rArcTo_dy'>of</a> <a href='#SkPath_rArcTo_dy'>four</a> <a href='#SkPath_rArcTo_dy'>possible</a> <a href='#SkPath_rArcTo_dy'>routes</a>: <a href='#SkPath_rArcTo_dy'>clockwise</a> <a href='#SkPath_rArcTo_dy'>or</a>
<a href='#SkPath_rArcTo_dy'>counterclockwise</a>, <a href='#SkPath_rArcTo_dy'>and</a> <a href='#SkPath_rArcTo_dy'>smaller</a> <a href='#SkPath_rArcTo_dy'>or</a> <a href='#SkPath_rArcTo_dy'>larger</a>. <a href='#SkPath_rArcTo_dy'>If</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>is</a> <a href='SkPath_Reference#Path'>empty</a>, <a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Path'>start</a> <a href='undocumented#Arc'>Arc</a> <a href='SkPoint_Reference#Point'>Point</a>
<a href='SkPoint_Reference#Point'>is</a> (0, 0).

<a href='undocumented#Arc'>Arc</a> <a href='#SkPath_rArcTo_sweep'>sweep</a> <a href='#SkPath_rArcTo_sweep'>is</a> <a href='#SkPath_rArcTo_sweep'>always</a> <a href='#SkPath_rArcTo_sweep'>less</a> <a href='#SkPath_rArcTo_sweep'>than</a> 360 <a href='#SkPath_rArcTo_sweep'>degrees</a>. <a href='#SkPath_arcTo'>arcTo</a> <a href='#SkPath_arcTo'>appends</a> <a href='undocumented#Line'>Line</a> <a href='undocumented#Line'>to</a> <a href='undocumented#Line'>end</a> <a href='SkPoint_Reference#Point'>Point</a>
<a href='SkPoint_Reference#Point'>if</a> <a href='SkPoint_Reference#Point'>either</a> <a href='SkPoint_Reference#Point'>radii</a> <a href='SkPoint_Reference#Point'>are</a> <a href='SkPoint_Reference#Point'>zero</a>, <a href='SkPoint_Reference#Point'>or</a> <a href='SkPoint_Reference#Point'>if</a> <a href='SkPoint_Reference#Point'>last</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>equals</a> <a href='SkPoint_Reference#Point'>end</a> <a href='SkPoint_Reference#Point'>Point</a>.
<a href='#SkPath_arcTo'>arcTo</a> <a href='#SkPath_arcTo'>scales</a> <a href='#SkPath_arcTo'>radii</a> (<a href='#SkPath_rArcTo_rx'>rx</a>, <a href='#SkPath_rArcTo_ry'>ry</a>) <a href='#SkPath_rArcTo_ry'>to</a> <a href='#SkPath_rArcTo_ry'>fit</a> <a href='#SkPath_rArcTo_ry'>last</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Point'>end</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>if</a> <a href='SkPoint_Reference#Point'>both</a> <a href='SkPoint_Reference#Point'>are</a>
<a href='SkPoint_Reference#Point'>greater</a> <a href='SkPoint_Reference#Point'>than</a> <a href='SkPoint_Reference#Point'>zero</a> <a href='SkPoint_Reference#Point'>but</a> <a href='SkPoint_Reference#Point'>too</a> <a href='SkPoint_Reference#Point'>small</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>describe</a> <a href='SkPoint_Reference#Point'>an</a> <a href='undocumented#Arc'>arc</a>.

<a href='#SkPath_arcTo'>arcTo</a> <a href='#SkPath_arcTo'>appends</a> <a href='#SkPath_arcTo'>up</a> <a href='#SkPath_arcTo'>to</a> <a href='#SkPath_arcTo'>four</a> <a href='SkPath_Reference#Conic'>Conic</a> <a href='undocumented#Curve'>curves</a>.
<a href='#SkPath_arcTo'>arcTo</a> <a href='#SkPath_arcTo'>implements</a> <a href='#SkPath_arcTo'>the</a> <a href='#SkPath_arcTo'>functionality</a> <a href='#SkPath_arcTo'>of</a> <a href='#SVG_Arc'>SVG_Arc</a>, <a href='#SVG_Arc'>although</a> <a href='undocumented#SVG'>SVG</a> "<a href='undocumented#SVG'>sweep-flag</a>" <a href='undocumented#SVG'>value</a> <a href='undocumented#SVG'>is</a>
<a href='undocumented#SVG'>opposite</a> <a href='undocumented#SVG'>the</a> <a href='undocumented#SVG'>integer</a> <a href='undocumented#SVG'>value</a> <a href='undocumented#SVG'>of</a> <a href='#SkPath_rArcTo_sweep'>sweep</a>; <a href='undocumented#SVG'>SVG</a> "<a href='undocumented#SVG'>sweep-flag</a>" <a href='undocumented#SVG'>uses</a> 1 <a href='undocumented#SVG'>for</a> <a href='undocumented#SVG'>clockwise</a>, <a href='undocumented#SVG'>while</a>
<a href='#SkPath_kCW_Direction'>kCW_Direction</a> <a href='#SkPath_kCW_Direction'>cast</a> <a href='#SkPath_kCW_Direction'>to</a> <a href='#SkPath_kCW_Direction'>int</a> <a href='#SkPath_kCW_Direction'>is</a> <a href='#SkPath_kCW_Direction'>zero</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_rArcTo_rx'><code><strong>rx</strong></code></a></td>
    <td>radius before x-axis rotation</td>
  </tr>
  <tr>    <td><a name='SkPath_rArcTo_ry'><code><strong>ry</strong></code></a></td>
    <td>radius before x-axis rotation</td>
  </tr>
  <tr>    <td><a name='SkPath_rArcTo_xAxisRotate'><code><strong>xAxisRotate</strong></code></a></td>
    <td>x-axis rotation in degrees; positive values are clockwise</td>
  </tr>
  <tr>    <td><a name='SkPath_rArcTo_largeArc'><code><strong>largeArc</strong></code></a></td>
    <td>chooses smaller or larger <a href='undocumented#Arc'>Arc</a></td>
  </tr>
  <tr>    <td><a name='SkPath_rArcTo_sweep'><code><strong>sweep</strong></code></a></td>
    <td>chooses clockwise or counterclockwise <a href='undocumented#Arc'>Arc</a></td>
  </tr>
  <tr>    <td><a name='SkPath_rArcTo_dx'><code><strong>dx</strong></code></a></td>
    <td>x-axis offset end of <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>from</a> <a href='undocumented#Arc'>last</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a></td>
  </tr>
  <tr>    <td><a name='SkPath_rArcTo_dy'><code><strong>dy</strong></code></a></td>
    <td>y-axis offset end of <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>from</a> <a href='undocumented#Arc'>last</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#Path'>Path</a>

### Example

<div><fiddle-embed name="3f76a1007416181a4848c1a87fc81dbd"></fiddle-embed></div>

### See Also

<a href='#SkPath_arcTo'>arcTo</a> <a href='#SkPath_ArcSize'>ArcSize</a> <a href='#SkPath_Direction'>Direction</a>

<a name='SkPath_close'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_close'>close()</a>
</pre>

Appends <a href='#SkPath_kClose_Verb'>kClose_Verb</a> <a href='#SkPath_kClose_Verb'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>. <a href='SkPath_Reference#SkPath'>A</a> <a href='SkPath_Reference#SkPath'>closed</a> <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>connects</a> <a href='SkPath_Overview#Contour'>the</a> <a href='SkPath_Overview#Contour'>first</a> <a href='SkPath_Overview#Contour'>and</a> <a href='SkPath_Overview#Contour'>last</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>
with <a href='undocumented#Line'>line</a>, <a href='undocumented#Line'>forming</a> <a href='undocumented#Line'>a</a> <a href='undocumented#Line'>continuous</a> <a href='undocumented#Line'>loop</a>. <a href='undocumented#Line'>Open</a> <a href='undocumented#Line'>and</a> <a href='undocumented#Line'>closed</a> <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>draw</a> <a href='SkPath_Overview#Contour'>the</a> <a href='SkPath_Overview#Contour'>same</a>
with <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kFill_Style'>kFill_Style</a>. <a href='#SkPaint_kFill_Style'>With</a> <a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_kStroke_Style'>kStroke_Style</a>, <a href='#SkPaint_kStroke_Style'>open</a> <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>draws</a>
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Cap'>Cap</a> <a href='#SkPaint_Cap'>at</a> <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>start</a> <a href='SkPath_Overview#Contour'>and</a> <a href='SkPath_Overview#Contour'>end</a>; <a href='SkPath_Overview#Contour'>closed</a> <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>draws</a>
<a href='SkPaint_Reference#SkPaint'>SkPaint</a>::<a href='#SkPaint_Join'>Join</a> <a href='#SkPaint_Join'>at</a> <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>start</a> <a href='SkPath_Overview#Contour'>and</a> <a href='SkPath_Overview#Contour'>end</a>.

<a href='#SkPath_close'>close()</a> <a href='#SkPath_close'>has</a> <a href='#SkPath_close'>no</a> <a href='#SkPath_close'>effect</a> <a href='#SkPath_close'>if</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>empty</a> <a href='SkPath_Reference#SkPath'>or</a> <a href='SkPath_Reference#SkPath'>last</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>is</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a>.

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="9235f6309271d6420fa5c45dc28664c5"></fiddle-embed></div>

### See Also

<a name='SkPath_IsInverseFillType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkPath_IsInverseFillType'>IsInverseFillType</a>(<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_FillType'>fill</a>)
</pre>

Returns true if <a href='#SkPath_IsInverseFillType_fill'>fill</a> <a href='#SkPath_IsInverseFillType_fill'>is</a> <a href='#SkPath_IsInverseFillType_fill'>inverted</a> <a href='#SkPath_IsInverseFillType_fill'>and</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>with</a> <a href='#SkPath_IsInverseFillType_fill'>fill</a> <a href='#SkPath_IsInverseFillType_fill'>represents</a> <a href='#SkPath_IsInverseFillType_fill'>area</a> <a href='#SkPath_IsInverseFillType_fill'>outside</a>
<a href='#SkPath_IsInverseFillType_fill'>of</a> <a href='#SkPath_IsInverseFillType_fill'>its</a> <a href='#SkPath_IsInverseFillType_fill'>geometric</a> <a href='#SkPath_IsInverseFillType_fill'>bounds</a>.

| <a href='#SkPath_FillType'>FillType</a> | is inverse |
| --- | ---  |
| <a href='#SkPath_kWinding_FillType'>kWinding_FillType</a> | false |
| <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd_FillType</a> | false |
| <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding_FillType</a> | true |
| <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd_FillType</a> | true |

### Parameters

<table>  <tr>    <td><a name='SkPath_IsInverseFillType_fill'><code><strong>fill</strong></code></a></td>
    <td>one of: <a href='#SkPath_kWinding_FillType'>kWinding_FillType</a>, <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd_FillType</a>,
<a href='#SkPath_kInverseWinding_FillType'>kInverseWinding_FillType</a>, <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd_FillType</a>
</td>
  </tr>
</table>

### Return Value

true if <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>fills</a> <a href='SkPath_Reference#Path'>outside</a> <a href='SkPath_Reference#Path'>its</a> <a href='SkPath_Reference#Path'>bounds</a>

### Example

<div><fiddle-embed name="1453856a9d0c73e8192bf298c4143563">

#### Example Output

~~~~
IsInverseFillType(kWinding_FillType) == false
IsInverseFillType(kEvenOdd_FillType) == false
IsInverseFillType(kInverseWinding_FillType) == true
IsInverseFillType(kInverseEvenOdd_FillType) == true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_getFillType'>getFillType</a> <a href='#SkPath_setFillType'>setFillType</a> <a href='#SkPath_ConvertToNonInverseFillType'>ConvertToNonInverseFillType</a>

<a name='SkPath_ConvertToNonInverseFillType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_ConvertToNonInverseFillType'>ConvertToNonInverseFillType</a>(<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_FillType'>fill</a>)
</pre>

Returns equivalent <a href='#Path_Fill_Type'>Fill_Type</a> <a href='#Path_Fill_Type'>representing</a> <a href='SkPath_Reference#Path'>Path</a> <a href='#SkPath_ConvertToNonInverseFillType_fill'>fill</a> <a href='#SkPath_ConvertToNonInverseFillType_fill'>inside</a> <a href='#SkPath_ConvertToNonInverseFillType_fill'>its</a> <a href='#SkPath_ConvertToNonInverseFillType_fill'>bounds</a>.

| <a href='#SkPath_FillType'>FillType</a> | inside <a href='#SkPath_FillType'>FillType</a> |
| --- | ---  |
| <a href='#SkPath_kWinding_FillType'>kWinding_FillType</a> | <a href='#SkPath_kWinding_FillType'>kWinding_FillType</a> |
| <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd_FillType</a> | <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd_FillType</a> |
| <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding_FillType</a> | <a href='#SkPath_kWinding_FillType'>kWinding_FillType</a> |
| <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd_FillType</a> | <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd_FillType</a> |

### Parameters

<table>  <tr>    <td><a name='SkPath_ConvertToNonInverseFillType_fill'><code><strong>fill</strong></code></a></td>
    <td>one of: <a href='#SkPath_kWinding_FillType'>kWinding_FillType</a>, <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd_FillType</a>,
<a href='#SkPath_kInverseWinding_FillType'>kInverseWinding_FillType</a>, <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd_FillType</a>
</td>
  </tr>
</table>

### Return Value

<a href='#SkPath_ConvertToNonInverseFillType_fill'>fill</a>, <a href='#SkPath_ConvertToNonInverseFillType_fill'>or</a> <a href='#SkPath_kWinding_FillType'>kWinding_FillType</a> <a href='#SkPath_kWinding_FillType'>or</a> <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd_FillType</a> <a href='#SkPath_kEvenOdd_FillType'>if</a> <a href='#SkPath_ConvertToNonInverseFillType_fill'>fill</a> <a href='#SkPath_ConvertToNonInverseFillType_fill'>is</a> <a href='#SkPath_ConvertToNonInverseFillType_fill'>inverted</a>

### Example

<div><fiddle-embed name="319f6b124458dcc0f9ce4d7bbde65810">

#### Example Output

~~~~
ConvertToNonInverseFillType(kWinding_FillType) == kWinding_FillType
ConvertToNonInverseFillType(kEvenOdd_FillType) == kEvenOdd_FillType
ConvertToNonInverseFillType(kInverseWinding_FillType) == kWinding_FillType
ConvertToNonInverseFillType(kInverseEvenOdd_FillType) == kEvenOdd_FillType
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_getFillType'>getFillType</a> <a href='#SkPath_setFillType'>setFillType</a> <a href='#SkPath_IsInverseFillType'>IsInverseFillType</a>

<a name='SkPath_ConvertConicToQuads'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static int <a href='#SkPath_ConvertConicToQuads'>ConvertConicToQuads</a>(<a href='#SkPath_ConvertConicToQuads'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p0</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p1</a>, <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p2</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>w</a>,
                               <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pts</a>[], <a href='SkPoint_Reference#SkPoint'>int</a> <a href='SkPoint_Reference#SkPoint'>pow2</a>)
</pre>

Approximates <a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>with</a> <a href='SkPath_Reference#Quad'>Quad</a> <a href='SkPath_Reference#Quad'>array</a>. <a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>is</a> <a href='SkPath_Reference#Conic'>constructed</a> <a href='SkPath_Reference#Conic'>from</a> <a href='SkPath_Reference#Conic'>start</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='#SkPath_ConvertConicToQuads_p0'>p0</a>,
<a href='#SkPath_ConvertConicToQuads_p0'>control</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='#SkPath_ConvertConicToQuads_p1'>p1</a>, <a href='#SkPath_ConvertConicToQuads_p1'>end</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='#SkPath_ConvertConicToQuads_p2'>p2</a>, <a href='#SkPath_ConvertConicToQuads_p2'>and</a> <a href='#SkPath_ConvertConicToQuads_p2'>weight</a> <a href='#SkPath_ConvertConicToQuads_w'>w</a>.
<a href='SkPath_Reference#Quad'>Quad</a> <a href='SkPath_Reference#Quad'>array</a> <a href='SkPath_Reference#Quad'>is</a> <a href='SkPath_Reference#Quad'>stored</a> <a href='SkPath_Reference#Quad'>in</a> <a href='#SkPath_ConvertConicToQuads_pts'>pts</a>; <a href='#SkPath_ConvertConicToQuads_pts'>this</a> <a href='#SkPath_ConvertConicToQuads_pts'>storage</a> <a href='#SkPath_ConvertConicToQuads_pts'>is</a> <a href='#SkPath_ConvertConicToQuads_pts'>supplied</a> <a href='#SkPath_ConvertConicToQuads_pts'>by</a> <a href='#SkPath_ConvertConicToQuads_pts'>caller</a>.
<a href='#SkPath_ConvertConicToQuads_pts'>Maximum</a> <a href='SkPath_Reference#Quad'>Quad</a> <a href='SkPath_Reference#Quad'>count</a> <a href='SkPath_Reference#Quad'>is</a> 2 <a href='SkPath_Reference#Quad'>to</a> <a href='SkPath_Reference#Quad'>the</a> <a href='#SkPath_ConvertConicToQuads_pow2'>pow2</a>.
<a href='#SkPath_ConvertConicToQuads_pow2'>Every</a> <a href='#SkPath_ConvertConicToQuads_pow2'>third</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>in</a> <a href='SkPoint_Reference#Point'>array</a> <a href='SkPoint_Reference#Point'>shares</a> <a href='SkPoint_Reference#Point'>last</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>of</a> <a href='SkPoint_Reference#Point'>previous</a> <a href='SkPath_Reference#Quad'>Quad</a> <a href='SkPath_Reference#Quad'>and</a> <a href='SkPath_Reference#Quad'>first</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>of</a>
<a href='SkPoint_Reference#Point'>next</a> <a href='SkPath_Reference#Quad'>Quad</a>. <a href='SkPath_Reference#Quad'>Maximum</a> <a href='#SkPath_ConvertConicToQuads_pts'>pts</a> <a href='#SkPath_ConvertConicToQuads_pts'>storage</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>is</a> <a href='undocumented#Size'>given</a> <a href='undocumented#Size'>by</a>:
<code>(1 + 2 * (1 << <a href='#SkPath_ConvertConicToQuads_pow2'>pow2</a>)) * <a href='undocumented#sizeof()'>sizeof</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a>)</code>.

Returns <a href='SkPath_Reference#Quad'>Quad</a> <a href='SkPath_Reference#Quad'>count</a> <a href='SkPath_Reference#Quad'>used</a> <a href='SkPath_Reference#Quad'>the</a> <a href='SkPath_Reference#Quad'>approximation</a>, <a href='SkPath_Reference#Quad'>which</a> <a href='SkPath_Reference#Quad'>may</a> <a href='SkPath_Reference#Quad'>be</a> <a href='SkPath_Reference#Quad'>smaller</a>
<a href='SkPath_Reference#Quad'>than</a> <a href='SkPath_Reference#Quad'>the</a> <a href='SkPath_Reference#Quad'>number</a> <a href='SkPath_Reference#Quad'>requested</a>.

<a href='#Path_Conic_Weight'>Conic_Weight</a> <a href='#Path_Conic_Weight'>determines</a> <a href='#Path_Conic_Weight'>the</a> <a href='#Path_Conic_Weight'>amount</a> <a href='#Path_Conic_Weight'>of</a> <a href='#Path_Conic_Weight'>influence</a> <a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>control</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>has</a> <a href='SkPoint_Reference#Point'>on</a> <a href='SkPoint_Reference#Point'>the</a> <a href='undocumented#Curve'>curve</a>.
<a href='#SkPath_ConvertConicToQuads_w'>w</a> <a href='#SkPath_ConvertConicToQuads_w'>less</a> <a href='#SkPath_ConvertConicToQuads_w'>than</a> <a href='#SkPath_ConvertConicToQuads_w'>one</a> <a href='#SkPath_ConvertConicToQuads_w'>represents</a> <a href='#SkPath_ConvertConicToQuads_w'>an</a> <a href='#SkPath_ConvertConicToQuads_w'>elliptical</a> <a href='#SkPath_ConvertConicToQuads_w'>section</a>. <a href='#SkPath_ConvertConicToQuads_w'>w</a> <a href='#SkPath_ConvertConicToQuads_w'>greater</a> <a href='#SkPath_ConvertConicToQuads_w'>than</a> <a href='#SkPath_ConvertConicToQuads_w'>one</a> <a href='#SkPath_ConvertConicToQuads_w'>represents</a>
<a href='#SkPath_ConvertConicToQuads_w'>a</a> <a href='#SkPath_ConvertConicToQuads_w'>hyperbolic</a> <a href='#SkPath_ConvertConicToQuads_w'>section</a>. <a href='#SkPath_ConvertConicToQuads_w'>w</a> <a href='#SkPath_ConvertConicToQuads_w'>equal</a> <a href='#SkPath_ConvertConicToQuads_w'>to</a> <a href='#SkPath_ConvertConicToQuads_w'>one</a> <a href='#SkPath_ConvertConicToQuads_w'>represents</a> <a href='#SkPath_ConvertConicToQuads_w'>a</a> <a href='#SkPath_ConvertConicToQuads_w'>parabolic</a> <a href='#SkPath_ConvertConicToQuads_w'>section</a>.

<a href='#SkPath_ConvertConicToQuads_w'>Two</a> <a href='SkPath_Reference#Quad'>Quad</a> <a href='undocumented#Curve'>curves</a> <a href='undocumented#Curve'>are</a> <a href='undocumented#Curve'>sufficient</a> <a href='undocumented#Curve'>to</a> <a href='undocumented#Curve'>approximate</a> <a href='undocumented#Curve'>an</a> <a href='undocumented#Curve'>elliptical</a> <a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>with</a> <a href='SkPath_Reference#Conic'>a</a> <a href='SkPath_Reference#Conic'>sweep</a>
<a href='SkPath_Reference#Conic'>of</a> <a href='SkPath_Reference#Conic'>up</a> <a href='SkPath_Reference#Conic'>to</a> 90 <a href='SkPath_Reference#Conic'>degrees</a>; <a href='SkPath_Reference#Conic'>in</a> <a href='SkPath_Reference#Conic'>this</a> <a href='SkPath_Reference#Conic'>case</a>, <a href='SkPath_Reference#Conic'>set</a> <a href='#SkPath_ConvertConicToQuads_pow2'>pow2</a> <a href='#SkPath_ConvertConicToQuads_pow2'>to</a> <a href='#SkPath_ConvertConicToQuads_pow2'>one</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_ConvertConicToQuads_p0'><code><strong>p0</strong></code></a></td>
    <td><a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>start</a> <a href='SkPoint_Reference#Point'>Point</a></td>
  </tr>
  <tr>    <td><a name='SkPath_ConvertConicToQuads_p1'><code><strong>p1</strong></code></a></td>
    <td><a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>control</a> <a href='SkPoint_Reference#Point'>Point</a></td>
  </tr>
  <tr>    <td><a name='SkPath_ConvertConicToQuads_p2'><code><strong>p2</strong></code></a></td>
    <td><a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>end</a> <a href='SkPoint_Reference#Point'>Point</a></td>
  </tr>
  <tr>    <td><a name='SkPath_ConvertConicToQuads_w'><code><strong>w</strong></code></a></td>
    <td><a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Conic'>weight</a></td>
  </tr>
  <tr>    <td><a name='SkPath_ConvertConicToQuads_pts'><code><strong>pts</strong></code></a></td>
    <td>storage for <a href='SkPath_Reference#Quad'>Quad</a> <a href='SkPath_Reference#Quad'>array</a></td>
  </tr>
  <tr>    <td><a name='SkPath_ConvertConicToQuads_pow2'><code><strong>pow2</strong></code></a></td>
    <td><a href='SkPath_Reference#Quad'>Quad</a> <a href='SkPath_Reference#Quad'>count</a>, <a href='SkPath_Reference#Quad'>as</a> <a href='SkPath_Reference#Quad'>power</a> <a href='SkPath_Reference#Quad'>of</a> <a href='SkPath_Reference#Quad'>two</a>, <a href='SkPath_Reference#Quad'>normally</a> 0 <a href='SkPath_Reference#Quad'>to</a> 5 (1 <a href='SkPath_Reference#Quad'>to</a> 32 <a href='SkPath_Reference#Quad'>Quad</a> <a href='undocumented#Curve'>curves</a>)</td>
  </tr>
</table>

### Return Value

number of <a href='SkPath_Reference#Quad'>Quad</a> <a href='undocumented#Curve'>curves</a> <a href='undocumented#Curve'>written</a> <a href='undocumented#Curve'>to</a> <a href='#SkPath_ConvertConicToQuads_pts'>pts</a>

### Example

<div><fiddle-embed name="3ba94448a4ba48f926e643baeb5b1016"><div>A pair of <a href='SkPath_Reference#Quad'>Quad</a> <a href='undocumented#Curve'>curves</a> <a href='undocumented#Curve'>are</a> <a href='undocumented#Curve'>drawn</a> <a href='undocumented#Curve'>in</a> <a href='undocumented#Curve'>red</a> <a href='undocumented#Curve'>on</a> <a href='undocumented#Curve'>top</a> <a href='undocumented#Curve'>of</a> <a href='undocumented#Curve'>the</a> <a href='undocumented#Curve'>elliptical</a> <a href='SkPath_Reference#Conic'>Conic</a> <a href='undocumented#Curve'>curve</a> <a href='undocumented#Curve'>in</a> <a href='undocumented#Curve'>black</a>.
<a href='undocumented#Curve'>The</a> <a href='undocumented#Curve'>middle</a> <a href='undocumented#Curve'>curve</a> <a href='undocumented#Curve'>is</a> <a href='undocumented#Curve'>nearly</a> <a href='undocumented#Curve'>circular</a>. <a href='undocumented#Curve'>The</a> <a href='undocumented#Curve'>top-right</a> <a href='undocumented#Curve'>curve</a> <a href='undocumented#Curve'>is</a> <a href='undocumented#Curve'>parabolic</a>, <a href='undocumented#Curve'>which</a> <a href='undocumented#Curve'>can</a>
<a href='undocumented#Curve'>be</a> <a href='undocumented#Curve'>drawn</a> <a href='undocumented#Curve'>exactly</a> <a href='undocumented#Curve'>with</a> <a href='undocumented#Curve'>a</a> <a href='undocumented#Curve'>single</a> <a href='SkPath_Reference#Quad'>Quad</a>.
</div></fiddle-embed></div>

### See Also

<a href='SkPath_Reference#Conic'>Conic</a> <a href='SkPath_Reference#Quad'>Quad</a>

<a name='SkPath_isRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isRect'>isRect</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#Rect'>rect</a>, <a href='SkRect_Reference#Rect'>bool</a>* <a href='SkRect_Reference#Rect'>isClosed</a> = <a href='SkRect_Reference#Rect'>nullptr</a>, <a href='#SkPath_Direction'>Direction</a>* <a href='#SkPath_Direction'>direction</a> = <a href='#SkPath_Direction'>nullptr</a>) <a href='#SkPath_Direction'>const</a>
</pre>

Returns true if <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>equivalent</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>when</a> <a href='SkRect_Reference#SkRect'>filled</a>.
If false: <a href='#SkPath_isRect_rect'>rect</a>, <a href='#SkPath_isRect_isClosed'>isClosed</a>, <a href='#SkPath_isRect_isClosed'>and</a> <a href='#SkPath_isRect_direction'>direction</a> <a href='#SkPath_isRect_direction'>are</a> <a href='#SkPath_isRect_direction'>unchanged</a>.
If true: <a href='#SkPath_isRect_rect'>rect</a>, <a href='#SkPath_isRect_isClosed'>isClosed</a>, <a href='#SkPath_isRect_isClosed'>and</a> <a href='#SkPath_isRect_direction'>direction</a> <a href='#SkPath_isRect_direction'>are</a> <a href='#SkPath_isRect_direction'>written</a> <a href='#SkPath_isRect_direction'>to</a> <a href='#SkPath_isRect_direction'>if</a> <a href='#SkPath_isRect_direction'>not</a> <a href='#SkPath_isRect_direction'>nullptr</a>.

<a href='#SkPath_isRect_rect'>rect</a> <a href='#SkPath_isRect_rect'>may</a> <a href='#SkPath_isRect_rect'>be</a> <a href='#SkPath_isRect_rect'>smaller</a> <a href='#SkPath_isRect_rect'>than</a> <a href='#SkPath_isRect_rect'>the</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>bounds</a>. <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>bounds</a> <a href='SkPath_Reference#SkPath'>may</a> <a href='SkPath_Reference#SkPath'>include</a> <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='SkPoint_Reference#Point'>points</a>
that do not alter the area drawn by the returned <a href='#SkPath_isRect_rect'>rect</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_isRect_rect'><code><strong>rect</strong></code></a></td>
    <td>storage for bounds of <a href='SkRect_Reference#SkRect'>SkRect</a>; <a href='SkRect_Reference#SkRect'>may</a> <a href='SkRect_Reference#SkRect'>be</a> <a href='SkRect_Reference#SkRect'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkPath_isRect_isClosed'><code><strong>isClosed</strong></code></a></td>
    <td>storage set to true if <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>closed</a>; <a href='SkPath_Reference#SkPath'>may</a> <a href='SkPath_Reference#SkPath'>be</a> <a href='SkPath_Reference#SkPath'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkPath_isRect_direction'><code><strong>direction</strong></code></a></td>
    <td>storage set to <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkPath_isRect_direction'>direction</a>; <a href='#SkPath_isRect_direction'>may</a> <a href='#SkPath_isRect_direction'>be</a> <a href='#SkPath_isRect_direction'>nullptr</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>contains</a> <a href='SkRect_Reference#SkRect'>SkRect</a>

### Example

<div><fiddle-embed name="81a2aac1b8f0ff3d4c8d35ccb9149b16"><div>After <a href='#SkPath_addRect'>addRect</a>, <a href='#SkPath_isRect'>isRect</a> <a href='#SkPath_isRect'>returns</a> <a href='#SkPath_isRect'>true</a>. <a href='#SkPath_isRect'>Following</a> <a href='#SkPath_moveTo'>moveTo</a> <a href='#SkPath_moveTo'>permits</a> <a href='#SkPath_isRect'>isRect</a> <a href='#SkPath_isRect'>to</a> <a href='#SkPath_isRect'>return</a> <a href='#SkPath_isRect'>true</a>, <a href='#SkPath_isRect'>but</a>
<a href='#SkPath_isRect'>following</a> <a href='#SkPath_lineTo'>lineTo</a> <a href='#SkPath_lineTo'>does</a> <a href='#SkPath_lineTo'>not</a>. <a href='#SkPath_addPoly'>addPoly</a> <a href='#SkPath_addPoly'>returns</a> <a href='#SkPath_addPoly'>true</a> <a href='#SkPath_addPoly'>even</a> <a href='#SkPath_addPoly'>though</a> <a href='#SkPath_isRect_rect'>rect</a> <a href='#SkPath_isRect_rect'>is</a> <a href='#SkPath_isRect_rect'>not</a> <a href='#SkPath_isRect_rect'>closed</a>, <a href='#SkPath_isRect_rect'>and</a> <a href='#SkPath_isRect_rect'>one</a>
<a href='#SkPath_isRect_rect'>side</a> <a href='#SkPath_isRect_rect'>of</a> <a href='#SkPath_isRect_rect'>rect</a> <a href='#SkPath_isRect_rect'>is</a> <a href='#SkPath_isRect_rect'>made</a> <a href='#SkPath_isRect_rect'>up</a> <a href='#SkPath_isRect_rect'>of</a> <a href='#SkPath_isRect_rect'>consecutive</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>segments</a>.
</div>

#### Example Output

~~~~
empty is not rect
addRect is rect (10, 20, 30, 40); is closed; direction CW
moveTo is rect (10, 20, 30, 40); is closed; direction CW
lineTo is not rect
addPoly is rect (0, 0, 80, 80); is not closed; direction CCW
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_computeTightBounds'>computeTightBounds</a> <a href='#SkPath_conservativelyContainsRect'>conservativelyContainsRect</a> <a href='#SkPath_getBounds'>getBounds</a> <a href='#SkPath_isConvex'>isConvex</a> <a href='#SkPath_isLastContourClosed'>isLastContourClosed</a> <a href='#SkPath_isNestedFillRects'>isNestedFillRects</a>

<a name='SkPath_isNestedFillRects'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isNestedFillRects'>isNestedFillRects</a>(<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#Rect'>rect</a>[2], <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>dirs</a>[2] = <a href='#SkPath_Direction'>nullptr</a>) <a href='#SkPath_Direction'>const</a>
</pre>

Returns true if <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>equivalent</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>nested</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>pair</a> <a href='SkRect_Reference#SkRect'>when</a> <a href='SkRect_Reference#SkRect'>filled</a>.
If false, <a href='#SkPath_isNestedFillRects_rect'>rect</a> <a href='#SkPath_isNestedFillRects_rect'>and</a> <a href='#SkPath_isNestedFillRects_dirs'>dirs</a> <a href='#SkPath_isNestedFillRects_dirs'>are</a> <a href='#SkPath_isNestedFillRects_dirs'>unchanged</a>.
If true, <a href='#SkPath_isNestedFillRects_rect'>rect</a> <a href='#SkPath_isNestedFillRects_rect'>and</a> <a href='#SkPath_isNestedFillRects_dirs'>dirs</a> <a href='#SkPath_isNestedFillRects_dirs'>are</a> <a href='#SkPath_isNestedFillRects_dirs'>written</a> <a href='#SkPath_isNestedFillRects_dirs'>to</a> <a href='#SkPath_isNestedFillRects_dirs'>if</a> <a href='#SkPath_isNestedFillRects_dirs'>not</a> <a href='#SkPath_isNestedFillRects_dirs'>nullptr</a>:
setting <a href='#SkPath_isNestedFillRects_rect'>rect</a>[0] <a href='#SkPath_isNestedFillRects_rect'>to</a> <a href='#SkPath_isNestedFillRects_rect'>outer</a> <a href='SkRect_Reference#SkRect'>SkRect</a>, <a href='SkRect_Reference#SkRect'>and</a> <a href='#SkPath_isNestedFillRects_rect'>rect</a>[1] <a href='#SkPath_isNestedFillRects_rect'>to</a> <a href='#SkPath_isNestedFillRects_rect'>inner</a> <a href='SkRect_Reference#SkRect'>SkRect</a>;
setting <a href='#SkPath_isNestedFillRects_dirs'>dirs</a>[0] <a href='#SkPath_isNestedFillRects_dirs'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>of</a> <a href='#SkPath_Direction'>outer</a> <a href='SkRect_Reference#SkRect'>SkRect</a>, <a href='SkRect_Reference#SkRect'>and</a> <a href='#SkPath_isNestedFillRects_dirs'>dirs</a>[1] <a href='#SkPath_isNestedFillRects_dirs'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>of</a>
inner <a href='SkRect_Reference#SkRect'>SkRect</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_isNestedFillRects_rect'><code><strong>rect</strong></code></a></td>
    <td>storage for <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>pair</a>; <a href='SkRect_Reference#SkRect'>may</a> <a href='SkRect_Reference#SkRect'>be</a> <a href='SkRect_Reference#SkRect'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkPath_isNestedFillRects_dirs'><code><strong>dirs</strong></code></a></td>
    <td>storage for <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>pair</a>; <a href='#SkPath_Direction'>may</a> <a href='#SkPath_Direction'>be</a> <a href='#SkPath_Direction'>nullptr</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>contains</a> <a href='SkPath_Reference#SkPath'>nested</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>pair</a>

### Example

<div><fiddle-embed name="77e4394caf9fa083c19c21c2462efe14">

#### Example Output

~~~~
outer (7.5, 17.5, 32.5, 42.5); direction CW
inner (12.5, 22.5, 27.5, 37.5); direction CCW
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_computeTightBounds'>computeTightBounds</a> <a href='#SkPath_conservativelyContainsRect'>conservativelyContainsRect</a> <a href='#SkPath_getBounds'>getBounds</a> <a href='#SkPath_isConvex'>isConvex</a> <a href='#SkPath_isLastContourClosed'>isLastContourClosed</a> <a href='#SkPath_isRect'>isRect</a>

<a name='SkPath_addRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addRect'>addRect</a>(<a href='#SkPath_addRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>dir</a> = <a href='#SkPath_kCW_Direction'>kCW_Direction</a>)
</pre>

Adds <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>, <a href='SkPath_Reference#SkPath'>appending</a> <a href='#SkPath_kMove_Verb'>kMove_Verb</a>, <a href='#SkPath_kMove_Verb'>three</a> <a href='#SkPath_kLine_Verb'>kLine_Verb</a>, <a href='#SkPath_kLine_Verb'>and</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a>,
starting with top-left corner of <a href='SkRect_Reference#SkRect'>SkRect</a>; <a href='SkRect_Reference#SkRect'>followed</a> <a href='SkRect_Reference#SkRect'>by</a> <a href='SkRect_Reference#SkRect'>top-right</a>, <a href='SkRect_Reference#SkRect'>bottom-right</a>,
and bottom-left if <a href='#SkPath_addRect_dir'>dir</a> <a href='#SkPath_addRect_dir'>is</a> <a href='#SkPath_kCW_Direction'>kCW_Direction</a>; <a href='#SkPath_kCW_Direction'>or</a> <a href='#SkPath_kCW_Direction'>followed</a> <a href='#SkPath_kCW_Direction'>by</a> <a href='#SkPath_kCW_Direction'>bottom-left</a>,
bottom-right, and top-right if <a href='#SkPath_addRect_dir'>dir</a> <a href='#SkPath_addRect_dir'>is</a> <a href='#SkPath_kCCW_Direction'>kCCW_Direction</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_addRect_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>add</a> <a href='SkRect_Reference#SkRect'>as</a> <a href='SkRect_Reference#SkRect'>a</a> <a href='SkRect_Reference#SkRect'>closed</a> <a href='SkPath_Overview#Contour'>contour</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addRect_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>to</a> <a href='#SkPath_Direction'>wind</a> <a href='#SkPath_Direction'>added</a> <a href='SkPath_Overview#Contour'>contour</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="0f841e4eaebb613b5069800567917c2d"><div>The left <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>dashes</a> <a href='SkRect_Reference#Rect'>starting</a> <a href='SkRect_Reference#Rect'>at</a> <a href='SkRect_Reference#Rect'>the</a> <a href='SkRect_Reference#Rect'>top-left</a> <a href='SkRect_Reference#Rect'>corner</a>, <a href='SkRect_Reference#Rect'>to</a> <a href='SkRect_Reference#Rect'>the</a> <a href='SkRect_Reference#Rect'>right</a>.
<a href='SkRect_Reference#Rect'>The</a> <a href='SkRect_Reference#Rect'>right</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>dashes</a> <a href='SkRect_Reference#Rect'>starting</a> <a href='SkRect_Reference#Rect'>at</a> <a href='SkRect_Reference#Rect'>the</a> <a href='SkRect_Reference#Rect'>top-left</a> <a href='SkRect_Reference#Rect'>corner</a>, <a href='SkRect_Reference#Rect'>towards</a> <a href='SkRect_Reference#Rect'>the</a> <a href='SkRect_Reference#Rect'>bottom</a>.
</div></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkPath_Direction'>Direction</a>

<a name='SkPath_addRect_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addRect'>addRect</a>(<a href='#SkPath_addRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>dir</a>, <a href='#SkPath_Direction'>unsigned</a> <a href='#SkPath_Direction'>start</a>)
</pre>

Adds <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>to</a> <a href='SkPath_Reference#Path'>Path</a>, <a href='SkPath_Reference#Path'>appending</a> <a href='#SkPath_kMove_Verb'>kMove_Verb</a>, <a href='#SkPath_kMove_Verb'>three</a> <a href='#SkPath_kLine_Verb'>kLine_Verb</a>, <a href='#SkPath_kLine_Verb'>and</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a>.
<a href='#SkPath_kClose_Verb'>If</a> <a href='#SkPath_addRect_2_dir'>dir</a> <a href='#SkPath_addRect_2_dir'>is</a> <a href='#SkPath_kCW_Direction'>kCW_Direction</a>, <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>corners</a> <a href='SkRect_Reference#Rect'>are</a> <a href='SkRect_Reference#Rect'>added</a> <a href='SkRect_Reference#Rect'>clockwise</a>; <a href='SkRect_Reference#Rect'>if</a> <a href='#SkPath_addRect_2_dir'>dir</a> <a href='#SkPath_addRect_2_dir'>is</a>
<a href='#SkPath_kCCW_Direction'>kCCW_Direction</a>, <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>corners</a> <a href='SkRect_Reference#Rect'>are</a> <a href='SkRect_Reference#Rect'>added</a> <a href='SkRect_Reference#Rect'>counterclockwise</a>.
<a href='#SkPath_addRect_2_start'>start</a> <a href='#SkPath_addRect_2_start'>determines</a> <a href='#SkPath_addRect_2_start'>the</a> <a href='#SkPath_addRect_2_start'>first</a> <a href='#SkPath_addRect_2_start'>corner</a> <a href='#SkPath_addRect_2_start'>added</a>.

| <a href='#SkPath_addRect_2_start'>start</a> | first corner |
| --- | ---  |
| 0 | top-left |
| 1 | top-right |
| 2 | bottom-right |
| 3 | bottom-left |

### Parameters

<table>  <tr>    <td><a name='SkPath_addRect_2_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>to</a> <a href='SkRect_Reference#Rect'>add</a> <a href='SkRect_Reference#Rect'>as</a> <a href='SkRect_Reference#Rect'>a</a> <a href='SkRect_Reference#Rect'>closed</a> <a href='SkPath_Overview#Contour'>contour</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addRect_2_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>to</a> <a href='#SkPath_Direction'>wind</a> <a href='#SkPath_Direction'>added</a> <a href='SkPath_Overview#Contour'>contour</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addRect_2_start'><code><strong>start</strong></code></a></td>
    <td>initial corner of <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>to</a> <a href='SkRect_Reference#Rect'>add</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#Path'>Path</a>

### Example

<div><fiddle-embed name="9202430b3f4f5275af8eec5cc9d7baa8"><div>The arrow is just after the initial corner and <a href='SkPoint_Reference#Point'>points</a> <a href='SkPoint_Reference#Point'>towards</a> <a href='SkPoint_Reference#Point'>the</a> <a href='SkPoint_Reference#Point'>next</a>
<a href='SkPoint_Reference#Point'>corner</a> <a href='SkPoint_Reference#Point'>appended</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPath_Reference#Path'>Path</a>.
</div></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkPath_Direction'>Direction</a>

<a name='SkPath_addRect_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addRect'>addRect</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>left</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>top</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>right</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>bottom</a>,
                <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>dir</a> = <a href='#SkPath_kCW_Direction'>kCW_Direction</a>)
</pre>

Adds <a href='SkRect_Reference#SkRect'>SkRect</a> (<a href='#SkPath_addRect_3_left'>left</a>, <a href='#SkPath_addRect_3_top'>top</a>, <a href='#SkPath_addRect_3_right'>right</a>, <a href='#SkPath_addRect_3_bottom'>bottom</a>) <a href='#SkPath_addRect_3_bottom'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>,
appending <a href='#SkPath_kMove_Verb'>kMove_Verb</a>, <a href='#SkPath_kMove_Verb'>three</a> <a href='#SkPath_kLine_Verb'>kLine_Verb</a>, <a href='#SkPath_kLine_Verb'>and</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a>,
starting with top-left corner of <a href='SkRect_Reference#SkRect'>SkRect</a>; <a href='SkRect_Reference#SkRect'>followed</a> <a href='SkRect_Reference#SkRect'>by</a> <a href='SkRect_Reference#SkRect'>top-right</a>, <a href='SkRect_Reference#SkRect'>bottom-right</a>,
and bottom-left if <a href='#SkPath_addRect_3_dir'>dir</a> <a href='#SkPath_addRect_3_dir'>is</a> <a href='#SkPath_kCW_Direction'>kCW_Direction</a>; <a href='#SkPath_kCW_Direction'>or</a> <a href='#SkPath_kCW_Direction'>followed</a> <a href='#SkPath_kCW_Direction'>by</a> <a href='#SkPath_kCW_Direction'>bottom-left</a>,
bottom-right, and top-right if <a href='#SkPath_addRect_3_dir'>dir</a> <a href='#SkPath_addRect_3_dir'>is</a> <a href='#SkPath_kCCW_Direction'>kCCW_Direction</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_addRect_3_left'><code><strong>left</strong></code></a></td>
    <td>smaller x-axis value of <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addRect_3_top'><code><strong>top</strong></code></a></td>
    <td>smaller y-axis value of <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addRect_3_right'><code><strong>right</strong></code></a></td>
    <td>larger x-axis value of <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addRect_3_bottom'><code><strong>bottom</strong></code></a></td>
    <td>larger y-axis value of <a href='SkRect_Reference#SkRect'>SkRect</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addRect_3_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>to</a> <a href='#SkPath_Direction'>wind</a> <a href='#SkPath_Direction'>added</a> <a href='SkPath_Overview#Contour'>contour</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="3837827310e8b88b8c2e128ef9fbbd65"><div>The <a href='#SkPath_addRect_3_left'>left</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>dashes</a> <a href='SkRect_Reference#Rect'>start</a> <a href='SkRect_Reference#Rect'>at</a> <a href='SkRect_Reference#Rect'>the</a> <a href='SkRect_Reference#Rect'>top-left</a> <a href='SkRect_Reference#Rect'>corner</a>, <a href='SkRect_Reference#Rect'>and</a> <a href='SkRect_Reference#Rect'>continue</a> <a href='SkRect_Reference#Rect'>to</a> <a href='SkRect_Reference#Rect'>the</a> <a href='#SkPath_addRect_3_right'>right</a>.
<a href='#SkPath_addRect_3_right'>The</a> <a href='#SkPath_addRect_3_right'>right</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>dashes</a> <a href='SkRect_Reference#Rect'>start</a> <a href='SkRect_Reference#Rect'>at</a> <a href='SkRect_Reference#Rect'>the</a> <a href='SkRect_Reference#Rect'>top-left</a> <a href='SkRect_Reference#Rect'>corner</a>, <a href='SkRect_Reference#Rect'>and</a> <a href='SkRect_Reference#Rect'>continue</a> <a href='SkRect_Reference#Rect'>down</a>.
</div></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawRect'>drawRect</a> <a href='#SkPath_Direction'>Direction</a>

<a name='SkPath_addOval'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addOval'>addOval</a>(<a href='#SkPath_addOval'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='undocumented#Oval'>oval</a>, <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>dir</a> = <a href='#SkPath_kCW_Direction'>kCW_Direction</a>)
</pre>

Adds <a href='#SkPath_addOval_oval'>oval</a> <a href='#SkPath_addOval_oval'>to</a> <a href='SkPath_Reference#Path'>path</a>, <a href='SkPath_Reference#Path'>appending</a> <a href='#SkPath_kMove_Verb'>kMove_Verb</a>, <a href='#SkPath_kMove_Verb'>four</a> <a href='#SkPath_kConic_Verb'>kConic_Verb</a>, <a href='#SkPath_kConic_Verb'>and</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a>.
<a href='undocumented#Oval'>Oval</a> <a href='undocumented#Oval'>is</a> <a href='undocumented#Oval'>upright</a> <a href='undocumented#Oval'>ellipse</a> <a href='undocumented#Oval'>bounded</a> <a href='undocumented#Oval'>by</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkPath_addOval_oval'>oval</a> <a href='#SkPath_addOval_oval'>with</a> <a href='#SkPath_addOval_oval'>radii</a> <a href='#SkPath_addOval_oval'>equal</a> <a href='#SkPath_addOval_oval'>to</a> <a href='#SkPath_addOval_oval'>half</a> <a href='#SkPath_addOval_oval'>oval</a> <a href='#SkPath_addOval_oval'>width</a>
and half <a href='#SkPath_addOval_oval'>oval</a> <a href='#SkPath_addOval_oval'>height</a>. <a href='undocumented#Oval'>Oval</a> <a href='undocumented#Oval'>begins</a> <a href='undocumented#Oval'>at</a> (<a href='#SkPath_addOval_oval'>oval</a>.<a href='#SkRect_fRight'>fRight</a>, <a href='#SkPath_addOval_oval'>oval</a>.<a href='#SkRect_centerY'>centerY</a>()) <a href='#SkRect_centerY'>and</a> <a href='#SkRect_centerY'>continues</a>
clockwise if <a href='#SkPath_addOval_dir'>dir</a> <a href='#SkPath_addOval_dir'>is</a> <a href='#SkPath_kCW_Direction'>kCW_Direction</a>, <a href='#SkPath_kCW_Direction'>counterclockwise</a> <a href='#SkPath_kCW_Direction'>if</a> <a href='#SkPath_addOval_dir'>dir</a> <a href='#SkPath_addOval_dir'>is</a> <a href='#SkPath_kCCW_Direction'>kCCW_Direction</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_addOval_oval'><code><strong>oval</strong></code></a></td>
    <td>bounds of ellipse added</td>
  </tr>
  <tr>    <td><a name='SkPath_addOval_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>to</a> <a href='#SkPath_Direction'>wind</a> <a href='#SkPath_Direction'>ellipse</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="cac84cf68e63a453c2a8b64c91537704"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawOval'>drawOval</a> <a href='#SkPath_Direction'>Direction</a> <a href='undocumented#Oval'>Oval</a>

<a name='SkPath_addOval_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addOval'>addOval</a>(<a href='#SkPath_addOval'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='undocumented#Oval'>oval</a>, <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>dir</a>, <a href='#SkPath_Direction'>unsigned</a> <a href='#SkPath_Direction'>start</a>)
</pre>

Adds <a href='undocumented#Oval'>Oval</a> <a href='undocumented#Oval'>to</a> <a href='SkPath_Reference#Path'>Path</a>, <a href='SkPath_Reference#Path'>appending</a> <a href='#SkPath_kMove_Verb'>kMove_Verb</a>, <a href='#SkPath_kMove_Verb'>four</a> <a href='#SkPath_kConic_Verb'>kConic_Verb</a>, <a href='#SkPath_kConic_Verb'>and</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a>.
<a href='undocumented#Oval'>Oval</a> <a href='undocumented#Oval'>is</a> <a href='undocumented#Oval'>upright</a> <a href='undocumented#Oval'>ellipse</a> <a href='undocumented#Oval'>bounded</a> <a href='undocumented#Oval'>by</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkPath_addOval_2_oval'>oval</a> <a href='#SkPath_addOval_2_oval'>with</a> <a href='#SkPath_addOval_2_oval'>radii</a> <a href='#SkPath_addOval_2_oval'>equal</a> <a href='#SkPath_addOval_2_oval'>to</a> <a href='#SkPath_addOval_2_oval'>half</a> <a href='#SkPath_addOval_2_oval'>oval</a> <a href='#SkPath_addOval_2_oval'>width</a>
<a href='#SkPath_addOval_2_oval'>and</a> <a href='#SkPath_addOval_2_oval'>half</a> <a href='#SkPath_addOval_2_oval'>oval</a> <a href='#SkPath_addOval_2_oval'>height</a>. <a href='undocumented#Oval'>Oval</a> <a href='undocumented#Oval'>begins</a> <a href='undocumented#Oval'>at</a> <a href='#SkPath_addOval_2_start'>start</a> <a href='#SkPath_addOval_2_start'>and</a> <a href='#SkPath_addOval_2_start'>continues</a>
<a href='#SkPath_addOval_2_start'>clockwise</a> <a href='#SkPath_addOval_2_start'>if</a> <a href='#SkPath_addOval_2_dir'>dir</a> <a href='#SkPath_addOval_2_dir'>is</a> <a href='#SkPath_kCW_Direction'>kCW_Direction</a>, <a href='#SkPath_kCW_Direction'>counterclockwise</a> <a href='#SkPath_kCW_Direction'>if</a> <a href='#SkPath_addOval_2_dir'>dir</a> <a href='#SkPath_addOval_2_dir'>is</a> <a href='#SkPath_kCCW_Direction'>kCCW_Direction</a>.

| <a href='#SkPath_addOval_2_start'>start</a> | <a href='SkPoint_Reference#Point'>Point</a> |
| --- | ---  |
| 0 | <a href='#SkPath_addOval_2_oval'>oval</a>.<a href='#SkRect_centerX'>centerX</a>(), <a href='#SkPath_addOval_2_oval'>oval</a>.<a href='#SkRect_fTop'>fTop</a> |
| 1 | <a href='#SkPath_addOval_2_oval'>oval</a>.<a href='#SkRect_fRight'>fRight</a>, <a href='#SkPath_addOval_2_oval'>oval</a>.<a href='#SkRect_centerY'>centerY</a>() |
| 2 | <a href='#SkPath_addOval_2_oval'>oval</a>.<a href='#SkRect_centerX'>centerX</a>(), <a href='#SkPath_addOval_2_oval'>oval</a>.<a href='#SkRect_fBottom'>fBottom</a> |
| 3 | <a href='#SkPath_addOval_2_oval'>oval</a>.<a href='#SkRect_fLeft'>fLeft</a>, <a href='#SkPath_addOval_2_oval'>oval</a>.<a href='#SkRect_centerY'>centerY</a>() |

### Parameters

<table>  <tr>    <td><a name='SkPath_addOval_2_oval'><code><strong>oval</strong></code></a></td>
    <td>bounds of ellipse added</td>
  </tr>
  <tr>    <td><a name='SkPath_addOval_2_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>to</a> <a href='#SkPath_Direction'>wind</a> <a href='#SkPath_Direction'>ellipse</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addOval_2_start'><code><strong>start</strong></code></a></td>
    <td>index of initial <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>of</a> <a href='SkPoint_Reference#Point'>ellipse</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#Path'>Path</a>

### Example

<div><fiddle-embed name="f1122d6fffddac0167e96fab4b9a862f"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawOval'>drawOval</a> <a href='#SkPath_Direction'>Direction</a> <a href='undocumented#Oval'>Oval</a>

<a name='SkPath_addCircle'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addCircle'>addCircle</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>radius</a>, <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>dir</a> = <a href='#SkPath_kCW_Direction'>kCW_Direction</a>)
</pre>

Adds <a href='undocumented#Circle'>Circle</a> <a href='undocumented#Circle'>centered</a> <a href='undocumented#Circle'>at</a> (<a href='#SkPath_addCircle_x'>x</a>, <a href='#SkPath_addCircle_y'>y</a>) <a href='#SkPath_addCircle_y'>of</a> <a href='undocumented#Size'>size</a> <a href='#SkPath_addCircle_radius'>radius</a> <a href='#SkPath_addCircle_radius'>to</a> <a href='SkPath_Reference#Path'>Path</a>, <a href='SkPath_Reference#Path'>appending</a> <a href='#SkPath_kMove_Verb'>kMove_Verb</a>,
<a href='#SkPath_kMove_Verb'>four</a> <a href='#SkPath_kConic_Verb'>kConic_Verb</a>, <a href='#SkPath_kConic_Verb'>and</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a>. <a href='undocumented#Circle'>Circle</a> <a href='undocumented#Circle'>begins</a> <a href='undocumented#Circle'>at</a>: <code>(<a href='#SkPath_addCircle_x'>x</a> + <a href='#SkPath_addCircle_radius'>radius</a>, <a href='#SkPath_addCircle_y'>y</a>)</code>, continuing
clockwise if <a href='#SkPath_addCircle_dir'>dir</a> <a href='#SkPath_addCircle_dir'>is</a> <a href='#SkPath_kCW_Direction'>kCW_Direction</a>, <a href='#SkPath_kCW_Direction'>and</a> <a href='#SkPath_kCW_Direction'>counterclockwise</a> <a href='#SkPath_kCW_Direction'>if</a> <a href='#SkPath_addCircle_dir'>dir</a> <a href='#SkPath_addCircle_dir'>is</a> <a href='#SkPath_kCCW_Direction'>kCCW_Direction</a>.

<a href='#SkPath_kCCW_Direction'>Has</a> <a href='#SkPath_kCCW_Direction'>no</a> <a href='#SkPath_kCCW_Direction'>effect</a> <a href='#SkPath_kCCW_Direction'>if</a> <a href='#SkPath_addCircle_radius'>radius</a> <a href='#SkPath_addCircle_radius'>is</a> <a href='#SkPath_addCircle_radius'>zero</a> <a href='#SkPath_addCircle_radius'>or</a> <a href='#SkPath_addCircle_radius'>negative</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_addCircle_x'><code><strong>x</strong></code></a></td>
    <td>center of <a href='undocumented#Circle'>Circle</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addCircle_y'><code><strong>y</strong></code></a></td>
    <td>center of <a href='undocumented#Circle'>Circle</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addCircle_radius'><code><strong>radius</strong></code></a></td>
    <td>distance from center to edge</td>
  </tr>
  <tr>    <td><a name='SkPath_addCircle_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>to</a> <a href='#SkPath_Direction'>wind</a> <a href='undocumented#Circle'>Circle</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#Path'>Path</a>

### Example

<div><fiddle-embed name="bd5286cb9a5e5c32cd980f72b8f400fb"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawCircle'>drawCircle</a> <a href='#SkPath_Direction'>Direction</a> <a href='undocumented#Circle'>Circle</a>

<a name='SkPath_addArc'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addArc'>addArc</a>(<a href='#SkPath_addArc'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='undocumented#Oval'>oval</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>startAngle</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sweepAngle</a>)
</pre>

Appends <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>, <a href='SkPath_Reference#SkPath'>as</a> <a href='SkPath_Reference#SkPath'>the</a> <a href='SkPath_Reference#SkPath'>start</a> <a href='SkPath_Reference#SkPath'>of</a> <a href='SkPath_Reference#SkPath'>new</a> <a href='SkPath_Overview#Contour'>contour</a>. <a href='undocumented#Arc'>Arc</a> <a href='undocumented#Arc'>added</a> <a href='undocumented#Arc'>is</a> <a href='undocumented#Arc'>part</a> <a href='undocumented#Arc'>of</a> <a href='undocumented#Arc'>ellipse</a>
bounded by <a href='#SkPath_addArc_oval'>oval</a>, <a href='#SkPath_addArc_oval'>from</a> <a href='#SkPath_addArc_startAngle'>startAngle</a> <a href='#SkPath_addArc_startAngle'>through</a> <a href='#SkPath_addArc_sweepAngle'>sweepAngle</a>. <a href='#SkPath_addArc_sweepAngle'>Both</a> <a href='#SkPath_addArc_startAngle'>startAngle</a> <a href='#SkPath_addArc_startAngle'>and</a>
<a href='#SkPath_addArc_sweepAngle'>sweepAngle</a> <a href='#SkPath_addArc_sweepAngle'>are</a> <a href='#SkPath_addArc_sweepAngle'>measured</a> <a href='#SkPath_addArc_sweepAngle'>in</a> <a href='#SkPath_addArc_sweepAngle'>degrees</a>, <a href='#SkPath_addArc_sweepAngle'>where</a> <a href='#SkPath_addArc_sweepAngle'>zero</a> <a href='#SkPath_addArc_sweepAngle'>degrees</a> <a href='#SkPath_addArc_sweepAngle'>is</a> <a href='#SkPath_addArc_sweepAngle'>aligned</a> <a href='#SkPath_addArc_sweepAngle'>with</a> <a href='#SkPath_addArc_sweepAngle'>the</a>
positive x-axis, and positive sweeps extends <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>clockwise</a>.

If <a href='#SkPath_addArc_sweepAngle'>sweepAngle</a> <= -360, <a href='#SkPath_addArc_sweepAngle'>or</a> <a href='#SkPath_addArc_sweepAngle'>sweepAngle</a> >= 360; <a href='#SkPath_addArc_sweepAngle'>and</a> <a href='#SkPath_addArc_startAngle'>startAngle</a> <a href='#SkPath_addArc_startAngle'>modulo</a> 90 <a href='#SkPath_addArc_startAngle'>is</a> <a href='#SkPath_addArc_startAngle'>nearly</a>
zero, append <a href='#SkPath_addArc_oval'>oval</a> <a href='#SkPath_addArc_oval'>instead</a> <a href='#SkPath_addArc_oval'>of</a> <a href='undocumented#Arc'>arc</a>. <a href='undocumented#Arc'>Otherwise</a>, <a href='#SkPath_addArc_sweepAngle'>sweepAngle</a> <a href='#SkPath_addArc_sweepAngle'>values</a> <a href='#SkPath_addArc_sweepAngle'>are</a> <a href='#SkPath_addArc_sweepAngle'>treated</a>
modulo 360, and <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>may</a> <a href='undocumented#Arc'>or</a> <a href='undocumented#Arc'>may</a> <a href='undocumented#Arc'>not</a> <a href='undocumented#Arc'>draw</a> <a href='undocumented#Arc'>depending</a> <a href='undocumented#Arc'>on</a> <a href='undocumented#Arc'>numeric</a> <a href='undocumented#Arc'>rounding</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_addArc_oval'><code><strong>oval</strong></code></a></td>
    <td>bounds of ellipse containing <a href='undocumented#Arc'>arc</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addArc_startAngle'><code><strong>startAngle</strong></code></a></td>
    <td>starting angle of <a href='undocumented#Arc'>arc</a> <a href='undocumented#Arc'>in</a> <a href='undocumented#Arc'>degrees</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addArc_sweepAngle'><code><strong>sweepAngle</strong></code></a></td>
    <td>sweep, in degrees. Positive is clockwise; treated modulo 360</td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="9cf5122475624e4cf39f06c698f80b1a"><div>The middle row of the left and right columns draw differently from the entries
above and below because <a href='#SkPath_addArc_sweepAngle'>sweepAngle</a> <a href='#SkPath_addArc_sweepAngle'>is</a> <a href='#SkPath_addArc_sweepAngle'>outside</a> <a href='#SkPath_addArc_sweepAngle'>of</a> <a href='#SkPath_addArc_sweepAngle'>the</a> <a href='#SkPath_addArc_sweepAngle'>range</a> <a href='#SkPath_addArc_sweepAngle'>of</a> +/-360,
<a href='#SkPath_addArc_sweepAngle'>and</a> <a href='#SkPath_addArc_startAngle'>startAngle</a> <a href='#SkPath_addArc_startAngle'>modulo</a> 90 <a href='#SkPath_addArc_startAngle'>is</a> <a href='#SkPath_addArc_startAngle'>not</a> <a href='#SkPath_addArc_startAngle'>zero</a>.
</div></fiddle-embed></div>

### See Also

<a href='undocumented#Arc'>Arc</a> <a href='#SkPath_arcTo'>arcTo</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawArc'>drawArc</a>

<a name='SkPath_addRoundRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addRoundRect'>addRoundRect</a>(<a href='#SkPath_addRoundRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>rx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>ry</a>, <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>dir</a> = <a href='#SkPath_kCW_Direction'>kCW_Direction</a>)
</pre>

Appends <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>, <a href='SkPath_Reference#SkPath'>creating</a> <a href='SkPath_Reference#SkPath'>a</a> <a href='SkPath_Reference#SkPath'>new</a> <a href='SkPath_Reference#SkPath'>closed</a> <a href='SkPath_Overview#Contour'>contour</a>. <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>has</a> <a href='SkRRect_Reference#SkRRect'>bounds</a>
equal to <a href='#SkPath_addRoundRect_rect'>rect</a>; <a href='#SkPath_addRoundRect_rect'>each</a> <a href='#SkPath_addRoundRect_rect'>corner</a> <a href='#SkPath_addRoundRect_rect'>is</a> 90 <a href='#SkPath_addRoundRect_rect'>degrees</a> <a href='#SkPath_addRoundRect_rect'>of</a> <a href='#SkPath_addRoundRect_rect'>an</a> <a href='#SkPath_addRoundRect_rect'>ellipse</a> <a href='#SkPath_addRoundRect_rect'>with</a> <a href='#SkPath_addRoundRect_rect'>radii</a> (<a href='#SkPath_addRoundRect_rx'>rx</a>, <a href='#SkPath_addRoundRect_ry'>ry</a>). <a href='#SkPath_addRoundRect_ry'>If</a>
<a href='#SkPath_addRoundRect_dir'>dir</a> <a href='#SkPath_addRoundRect_dir'>is</a> <a href='#SkPath_kCW_Direction'>kCW_Direction</a>, <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>starts</a> <a href='SkRRect_Reference#SkRRect'>at</a> <a href='SkRRect_Reference#SkRRect'>top-left</a> <a href='SkRRect_Reference#SkRRect'>of</a> <a href='SkRRect_Reference#SkRRect'>the</a> <a href='SkRRect_Reference#SkRRect'>lower-left</a> <a href='SkRRect_Reference#SkRRect'>corner</a> <a href='SkRRect_Reference#SkRRect'>and</a>
winds clockwise. If <a href='#SkPath_addRoundRect_dir'>dir</a> <a href='#SkPath_addRoundRect_dir'>is</a> <a href='#SkPath_kCCW_Direction'>kCCW_Direction</a>, <a href='SkRRect_Reference#SkRRect'>SkRRect</a> <a href='SkRRect_Reference#SkRRect'>starts</a> <a href='SkRRect_Reference#SkRRect'>at</a> <a href='SkRRect_Reference#SkRRect'>the</a> <a href='SkRRect_Reference#SkRRect'>bottom-left</a>
of the upper-left corner and winds counterclockwise.

If either <a href='#SkPath_addRoundRect_rx'>rx</a> <a href='#SkPath_addRoundRect_rx'>or</a> <a href='#SkPath_addRoundRect_ry'>ry</a> <a href='#SkPath_addRoundRect_ry'>is</a> <a href='#SkPath_addRoundRect_ry'>too</a> <a href='#SkPath_addRoundRect_ry'>large</a>, <a href='#SkPath_addRoundRect_rx'>rx</a> <a href='#SkPath_addRoundRect_rx'>and</a> <a href='#SkPath_addRoundRect_ry'>ry</a> <a href='#SkPath_addRoundRect_ry'>are</a> <a href='#SkPath_addRoundRect_ry'>scaled</a> <a href='#SkPath_addRoundRect_ry'>uniformly</a> <a href='#SkPath_addRoundRect_ry'>until</a> <a href='#SkPath_addRoundRect_ry'>the</a>
corners fit. If <a href='#SkPath_addRoundRect_rx'>rx</a> <a href='#SkPath_addRoundRect_rx'>or</a> <a href='#SkPath_addRoundRect_ry'>ry</a> <a href='#SkPath_addRoundRect_ry'>is</a> <a href='#SkPath_addRoundRect_ry'>less</a> <a href='#SkPath_addRoundRect_ry'>than</a> <a href='#SkPath_addRoundRect_ry'>or</a> <a href='#SkPath_addRoundRect_ry'>equal</a> <a href='#SkPath_addRoundRect_ry'>to</a> <a href='#SkPath_addRoundRect_ry'>zero</a>, <a href='#SkPath_addRoundRect'>addRoundRect</a>() <a href='#SkPath_addRoundRect'>appends</a>
<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkPath_addRoundRect_rect'>rect</a> <a href='#SkPath_addRoundRect_rect'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>.

After appending, <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>may</a> <a href='SkPath_Reference#SkPath'>be</a> <a href='SkPath_Reference#SkPath'>empty</a>, <a href='SkPath_Reference#SkPath'>or</a> <a href='SkPath_Reference#SkPath'>may</a> <a href='SkPath_Reference#SkPath'>contain</a>: <a href='SkRect_Reference#SkRect'>SkRect</a>, <a href='undocumented#Oval'>oval</a>, <a href='undocumented#Oval'>or</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_addRoundRect_rect'><code><strong>rect</strong></code></a></td>
    <td>bounds of <a href='SkRRect_Reference#SkRRect'>SkRRect</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addRoundRect_rx'><code><strong>rx</strong></code></a></td>
    <td>x-axis radius of rounded corners on the <a href='SkRRect_Reference#SkRRect'>SkRRect</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addRoundRect_ry'><code><strong>ry</strong></code></a></td>
    <td>y-axis radius of rounded corners on the <a href='SkRRect_Reference#SkRRect'>SkRRect</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addRoundRect_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>to</a> <a href='#SkPath_Direction'>wind</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="24736f685f265cf533f1700c042db353"><div>If either radius is zero, <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>contains</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>and</a> <a href='SkRect_Reference#Rect'>is</a> <a href='SkRect_Reference#Rect'>drawn</a> <a href='SkRect_Reference#Rect'>red</a>.
<a href='SkRect_Reference#Rect'>If</a> <a href='SkRect_Reference#Rect'>sides</a> <a href='SkRect_Reference#Rect'>are</a> <a href='SkRect_Reference#Rect'>only</a> <a href='SkRect_Reference#Rect'>radii</a>, <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>contains</a> <a href='undocumented#Oval'>Oval</a> <a href='undocumented#Oval'>and</a> <a href='undocumented#Oval'>is</a> <a href='undocumented#Oval'>drawn</a> <a href='undocumented#Oval'>blue</a>.
<a href='undocumented#Oval'>All</a> <a href='undocumented#Oval'>remaining</a> <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>draws</a> <a href='SkPath_Reference#Path'>are</a> <a href='SkPath_Reference#Path'>convex</a>, <a href='SkPath_Reference#Path'>and</a> <a href='SkPath_Reference#Path'>are</a> <a href='SkPath_Reference#Path'>drawn</a> <a href='SkPath_Reference#Path'>in</a> <a href='SkPath_Reference#Path'>gray</a>; <a href='SkPath_Reference#Path'>no</a>
<a href='SkPath_Reference#Path'>paths</a> <a href='SkPath_Reference#Path'>constructed</a> <a href='SkPath_Reference#Path'>from</a> <a href='#SkPath_addRoundRect'>addRoundRect</a> <a href='#SkPath_addRoundRect'>are</a> <a href='#SkPath_addRoundRect'>concave</a>, <a href='#SkPath_addRoundRect'>so</a> <a href='#SkPath_addRoundRect'>none</a> <a href='#SkPath_addRoundRect'>are</a>
<a href='#SkPath_addRoundRect'>drawn</a> <a href='#SkPath_addRoundRect'>in</a> <a href='#SkPath_addRoundRect'>green</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_addRRect'>addRRect</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawRoundRect'>drawRoundRect</a>

<a name='SkPath_addRoundRect_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addRoundRect'>addRoundRect</a>(<a href='#SkPath_addRoundRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>, <a href='SkRect_Reference#Rect'>const</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>radii</a>[], <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>dir</a> = <a href='#SkPath_kCW_Direction'>kCW_Direction</a>)
</pre>

Appends <a href='#RRect'>Round_Rect</a> <a href='#RRect'>to</a> <a href='SkPath_Reference#Path'>Path</a>, <a href='SkPath_Reference#Path'>creating</a> <a href='SkPath_Reference#Path'>a</a> <a href='SkPath_Reference#Path'>new</a> <a href='SkPath_Reference#Path'>closed</a> <a href='SkPath_Overview#Contour'>Contour</a>. <a href='#RRect'>Round_Rect</a> <a href='#RRect'>has</a> <a href='#RRect'>bounds</a>
<a href='#RRect'>equal</a> <a href='#RRect'>to</a> <a href='#SkPath_addRoundRect_2_rect'>rect</a>; <a href='#SkPath_addRoundRect_2_rect'>each</a> <a href='#SkPath_addRoundRect_2_rect'>corner</a> <a href='#SkPath_addRoundRect_2_rect'>is</a> 90 <a href='#SkPath_addRoundRect_2_rect'>degrees</a> <a href='#SkPath_addRoundRect_2_rect'>of</a> <a href='#SkPath_addRoundRect_2_rect'>an</a> <a href='#SkPath_addRoundRect_2_rect'>ellipse</a> <a href='#SkPath_addRoundRect_2_rect'>with</a> <a href='#SkPath_addRoundRect_2_radii'>radii</a> <a href='#SkPath_addRoundRect_2_radii'>from</a> <a href='#SkPath_addRoundRect_2_radii'>the</a>
<a href='#SkPath_addRoundRect_2_radii'>array</a>.

| <a href='#SkPath_addRoundRect_2_radii'>radii</a> <a href='#SkPath_addRoundRect_2_radii'>index</a> | location |
| --- | ---  |
| 0 | x-axis radius of top-left corner |
| 1 | y-axis radius of top-left corner |
| 2 | x-axis radius of top-right corner |
| 3 | y-axis radius of top-right corner |
| 4 | x-axis radius of bottom-right corner |
| 5 | y-axis radius of bottom-right corner |
| 6 | x-axis radius of bottom-left corner |
| 7 | y-axis radius of bottom-left corner |

If <a href='#SkPath_addRoundRect_2_dir'>dir</a> <a href='#SkPath_addRoundRect_2_dir'>is</a> <a href='#SkPath_kCW_Direction'>kCW_Direction</a>, <a href='#RRect'>Round_Rect</a> <a href='#RRect'>starts</a> <a href='#RRect'>at</a> <a href='#RRect'>top-left</a> <a href='#RRect'>of</a> <a href='#RRect'>the</a> <a href='#RRect'>lower-left</a> <a href='#RRect'>corner</a>
<a href='#RRect'>and</a> <a href='#RRect'>winds</a> <a href='#RRect'>clockwise</a>. <a href='#RRect'>If</a> <a href='#SkPath_addRoundRect_2_dir'>dir</a> <a href='#SkPath_addRoundRect_2_dir'>is</a> <a href='#SkPath_kCCW_Direction'>kCCW_Direction</a>, <a href='#RRect'>Round_Rect</a> <a href='#RRect'>starts</a> <a href='#RRect'>at</a> <a href='#RRect'>the</a>
<a href='#RRect'>bottom-left</a> <a href='#RRect'>of</a> <a href='#RRect'>the</a> <a href='#RRect'>upper-left</a> <a href='#RRect'>corner</a> <a href='#RRect'>and</a> <a href='#RRect'>winds</a> <a href='#RRect'>counterclockwise</a>.

<a href='#RRect'>If</a> <a href='#RRect'>both</a> <a href='#SkPath_addRoundRect_2_radii'>radii</a> <a href='#SkPath_addRoundRect_2_radii'>on</a> <a href='#SkPath_addRoundRect_2_radii'>any</a> <a href='#SkPath_addRoundRect_2_radii'>side</a> <a href='#SkPath_addRoundRect_2_radii'>of</a> <a href='#SkPath_addRoundRect_2_rect'>rect</a> <a href='#SkPath_addRoundRect_2_rect'>exceed</a> <a href='#SkPath_addRoundRect_2_rect'>its</a> <a href='#SkPath_addRoundRect_2_rect'>length</a>, <a href='#SkPath_addRoundRect_2_rect'>all</a> <a href='#SkPath_addRoundRect_2_radii'>radii</a> <a href='#SkPath_addRoundRect_2_radii'>are</a> <a href='#SkPath_addRoundRect_2_radii'>scaled</a>
<a href='#SkPath_addRoundRect_2_radii'>uniformly</a> <a href='#SkPath_addRoundRect_2_radii'>until</a> <a href='#SkPath_addRoundRect_2_radii'>the</a> <a href='#SkPath_addRoundRect_2_radii'>corners</a> <a href='#SkPath_addRoundRect_2_radii'>fit</a>. <a href='#SkPath_addRoundRect_2_radii'>If</a> <a href='#SkPath_addRoundRect_2_radii'>either</a> <a href='#SkPath_addRoundRect_2_radii'>radius</a> <a href='#SkPath_addRoundRect_2_radii'>of</a> <a href='#SkPath_addRoundRect_2_radii'>a</a> <a href='#SkPath_addRoundRect_2_radii'>corner</a> <a href='#SkPath_addRoundRect_2_radii'>is</a> <a href='#SkPath_addRoundRect_2_radii'>less</a> <a href='#SkPath_addRoundRect_2_radii'>than</a> <a href='#SkPath_addRoundRect_2_radii'>or</a>
<a href='#SkPath_addRoundRect_2_radii'>equal</a> <a href='#SkPath_addRoundRect_2_radii'>to</a> <a href='#SkPath_addRoundRect_2_radii'>zero</a>, <a href='#SkPath_addRoundRect_2_radii'>both</a> <a href='#SkPath_addRoundRect_2_radii'>are</a> <a href='#SkPath_addRoundRect_2_radii'>treated</a> <a href='#SkPath_addRoundRect_2_radii'>as</a> <a href='#SkPath_addRoundRect_2_radii'>zero</a>.

<a href='#SkPath_addRoundRect_2_radii'>After</a> <a href='#SkPath_addRoundRect_2_radii'>appending</a>, <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>may</a> <a href='SkPath_Reference#Path'>be</a> <a href='SkPath_Reference#Path'>empty</a>, <a href='SkPath_Reference#Path'>or</a> <a href='SkPath_Reference#Path'>may</a> <a href='SkPath_Reference#Path'>contain</a>: <a href='SkRect_Reference#Rect'>Rect</a>, <a href='undocumented#Oval'>Oval</a>, <a href='undocumented#Oval'>or</a> <a href='#RRect'>Round_Rect</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_addRoundRect_2_rect'><code><strong>rect</strong></code></a></td>
    <td>bounds of <a href='#RRect'>Round_Rect</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addRoundRect_2_radii'><code><strong>radii</strong></code></a></td>
    <td>array of 8 <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>values</a>, <a href='undocumented#SkScalar'>a</a> <a href='undocumented#SkScalar'>radius</a> <a href='undocumented#SkScalar'>pair</a> <a href='undocumented#SkScalar'>for</a> <a href='undocumented#SkScalar'>each</a> <a href='undocumented#SkScalar'>corner</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addRoundRect_2_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>to</a> <a href='#SkPath_Direction'>wind</a> <a href='#RRect'>Round_Rect</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#Path'>Path</a>

### Example

<div><fiddle-embed name="c43d70606b4ee464d2befbcf448c5e73"></fiddle-embed></div>

### See Also

<a href='#SkPath_addRRect'>addRRect</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawRoundRect'>drawRoundRect</a>

<a name='SkPath_addRRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addRRect'>addRRect</a>(<a href='#SkPath_addRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>rrect</a>, <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>dir</a> = <a href='#SkPath_kCW_Direction'>kCW_Direction</a>)
</pre>

Adds <a href='#SkPath_addRRect_rrect'>rrect</a> <a href='#SkPath_addRRect_rrect'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>, <a href='SkPath_Reference#SkPath'>creating</a> <a href='SkPath_Reference#SkPath'>a</a> <a href='SkPath_Reference#SkPath'>new</a> <a href='SkPath_Reference#SkPath'>closed</a> <a href='SkPath_Overview#Contour'>contour</a>. <a href='SkPath_Overview#Contour'>If</a>
<a href='#SkPath_addRRect_dir'>dir</a> <a href='#SkPath_addRRect_dir'>is</a> <a href='#SkPath_kCW_Direction'>kCW_Direction</a>, <a href='#SkPath_addRRect_rrect'>rrect</a> <a href='#SkPath_addRRect_rrect'>starts</a> <a href='#SkPath_addRRect_rrect'>at</a> <a href='#SkPath_addRRect_rrect'>top-left</a> <a href='#SkPath_addRRect_rrect'>of</a> <a href='#SkPath_addRRect_rrect'>the</a> <a href='#SkPath_addRRect_rrect'>lower-left</a> <a href='#SkPath_addRRect_rrect'>corner</a> <a href='#SkPath_addRRect_rrect'>and</a>
winds clockwise. If <a href='#SkPath_addRRect_dir'>dir</a> <a href='#SkPath_addRRect_dir'>is</a> <a href='#SkPath_kCCW_Direction'>kCCW_Direction</a>, <a href='#SkPath_addRRect_rrect'>rrect</a> <a href='#SkPath_addRRect_rrect'>starts</a> <a href='#SkPath_addRRect_rrect'>at</a> <a href='#SkPath_addRRect_rrect'>the</a> <a href='#SkPath_addRRect_rrect'>bottom-left</a>
of the upper-left corner and winds counterclockwise.

After appending, <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>may</a> <a href='SkPath_Reference#SkPath'>be</a> <a href='SkPath_Reference#SkPath'>empty</a>, <a href='SkPath_Reference#SkPath'>or</a> <a href='SkPath_Reference#SkPath'>may</a> <a href='SkPath_Reference#SkPath'>contain</a>: <a href='SkRect_Reference#SkRect'>SkRect</a>, <a href='undocumented#Oval'>oval</a>, <a href='undocumented#Oval'>or</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_addRRect_rrect'><code><strong>rrect</strong></code></a></td>
    <td>bounds and radii of rounded rectangle</td>
  </tr>
  <tr>    <td><a name='SkPath_addRRect_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>to</a> <a href='#SkPath_Direction'>wind</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="d9ecd58081b5bc77a157636fcb345dc6"></fiddle-embed></div>

### See Also

<a href='#SkPath_addRoundRect'>addRoundRect</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawRRect'>drawRRect</a>

<a name='SkPath_addRRect_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addRRect'>addRRect</a>(<a href='#SkPath_addRRect'>const</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& <a href='SkRRect_Reference#SkRRect'>rrect</a>, <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>dir</a>, <a href='#SkPath_Direction'>unsigned</a> <a href='#SkPath_Direction'>start</a>)
</pre>

Adds <a href='#SkPath_addRRect_2_rrect'>rrect</a> <a href='#SkPath_addRRect_2_rrect'>to</a> <a href='SkPath_Reference#Path'>Path</a>, <a href='SkPath_Reference#Path'>creating</a> <a href='SkPath_Reference#Path'>a</a> <a href='SkPath_Reference#Path'>new</a> <a href='SkPath_Reference#Path'>closed</a> <a href='SkPath_Overview#Contour'>Contour</a>. <a href='SkPath_Overview#Contour'>If</a> <a href='#SkPath_addRRect_2_dir'>dir</a> <a href='#SkPath_addRRect_2_dir'>is</a> <a href='#SkPath_kCW_Direction'>kCW_Direction</a>, <a href='#SkPath_addRRect_2_rrect'>rrect</a>
<a href='#SkPath_addRRect_2_rrect'>winds</a> <a href='#SkPath_addRRect_2_rrect'>clockwise</a>; <a href='#SkPath_addRRect_2_rrect'>if</a> <a href='#SkPath_addRRect_2_dir'>dir</a> <a href='#SkPath_addRRect_2_dir'>is</a> <a href='#SkPath_kCCW_Direction'>kCCW_Direction</a>, <a href='#SkPath_addRRect_2_rrect'>rrect</a> <a href='#SkPath_addRRect_2_rrect'>winds</a> <a href='#SkPath_addRRect_2_rrect'>counterclockwise</a>.
<a href='#SkPath_addRRect_2_start'>start</a> <a href='#SkPath_addRRect_2_start'>determines</a> <a href='#SkPath_addRRect_2_start'>the</a> <a href='#SkPath_addRRect_2_start'>first</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>of</a> <a href='#SkPath_addRRect_2_rrect'>rrect</a> <a href='#SkPath_addRRect_2_rrect'>to</a> <a href='#SkPath_addRRect_2_rrect'>add</a>.

| <a href='#SkPath_addRRect_2_start'>start</a> | location |
| --- | ---  |
| 0 | right of top-left corner |
| 1 | left of top-right corner |
| 2 | bottom of top-right corner |
| 3 | top of bottom-right corner |
| 4 | left of bottom-right corner |
| 5 | right of bottom-left corner |
| 6 | top of bottom-left corner |
| 7 | bottom of top-left corner |

After appending, <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>may</a> <a href='SkPath_Reference#Path'>be</a> <a href='SkPath_Reference#Path'>empty</a>, <a href='SkPath_Reference#Path'>or</a> <a href='SkPath_Reference#Path'>may</a> <a href='SkPath_Reference#Path'>contain</a>: <a href='SkRect_Reference#Rect'>Rect</a>, <a href='undocumented#Oval'>Oval</a>, <a href='undocumented#Oval'>or</a> <a href='#RRect'>Round_Rect</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_addRRect_2_rrect'><code><strong>rrect</strong></code></a></td>
    <td>bounds and radii of rounded rectangle</td>
  </tr>
  <tr>    <td><a name='SkPath_addRRect_2_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_Direction'>to</a> <a href='#SkPath_Direction'>wind</a> <a href='#RRect'>Round_Rect</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addRRect_2_start'><code><strong>start</strong></code></a></td>
    <td>index of initial <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>of</a> <a href='#RRect'>Round_Rect</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#Path'>Path</a>

### Example

<div><fiddle-embed name="888edd4c4a91ca62ceb01bce8ab675b2"></fiddle-embed></div>

### See Also

<a href='#SkPath_addRoundRect'>addRoundRect</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawRRect'>drawRRect</a>

<a name='SkPath_addPoly'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addPoly'>addPoly</a>(<a href='#SkPath_addPoly'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pts</a>[], <a href='SkPoint_Reference#SkPoint'>int</a> <a href='SkPoint_Reference#SkPoint'>count</a>, <a href='SkPoint_Reference#SkPoint'>bool</a> <a href='SkPoint_Reference#SkPoint'>close</a>)
</pre>

Adds <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>created</a> <a href='SkPath_Overview#Contour'>from</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>array</a>, <a href='undocumented#Line'>adding</a> (<a href='#SkPath_addPoly_count'>count</a> - 1) <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>segments</a>.
<a href='SkPath_Overview#Contour'>Contour</a> <a href='SkPath_Overview#Contour'>added</a> <a href='SkPath_Overview#Contour'>starts</a> <a href='SkPath_Overview#Contour'>at</a> <a href='#SkPath_addPoly_pts'>pts</a>[0], <a href='#SkPath_addPoly_pts'>then</a> <a href='#SkPath_addPoly_pts'>adds</a> <a href='#SkPath_addPoly_pts'>a</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>for</a> <a href='undocumented#Line'>every</a> <a href='undocumented#Line'>additional</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>
in <a href='#SkPath_addPoly_pts'>pts</a> <a href='#SkPath_addPoly_pts'>array</a>. <a href='#SkPath_addPoly_pts'>If</a> <a href='#SkPath_addPoly_close'>close</a> <a href='#SkPath_addPoly_close'>is</a> <a href='#SkPath_addPoly_close'>true</a>, <a href='#SkPath_addPoly_close'>appends</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a> <a href='#SkPath_kClose_Verb'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>, <a href='SkPath_Reference#SkPath'>connecting</a>
<a href='#SkPath_addPoly_pts'>pts</a>[<a href='#SkPath_addPoly_count'>count</a> - 1] <a href='#SkPath_addPoly_count'>and</a> <a href='#SkPath_addPoly_pts'>pts</a>[0].

If <a href='#SkPath_addPoly_count'>count</a> <a href='#SkPath_addPoly_count'>is</a> <a href='#SkPath_addPoly_count'>zero</a>, <a href='#SkPath_addPoly_count'>append</a> <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>to</a> <a href='SkPath_Reference#Path'>path</a>.
Has no effect if <a href='#SkPath_addPoly_count'>count</a> <a href='#SkPath_addPoly_count'>is</a> <a href='#SkPath_addPoly_count'>less</a> <a href='#SkPath_addPoly_count'>than</a> <a href='#SkPath_addPoly_count'>one</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_addPoly_pts'><code><strong>pts</strong></code></a></td>
    <td>array of <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>sharing</a> <a href='undocumented#Line'>end</a> <a href='undocumented#Line'>and</a> <a href='undocumented#Line'>start</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addPoly_count'><code><strong>count</strong></code></a></td>
    <td>length of  <a href='SkPath_Reference#Point_Array'>SkPoint array</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addPoly_close'><code><strong>close</strong></code></a></td>
    <td>true to add <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>connecting</a> <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>end</a> <a href='SkPath_Overview#Contour'>and</a> <a href='SkPath_Overview#Contour'>start</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="182b3999772f330f3b0b891b492634ae"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawPoints'>drawPoints</a>

<a name='SkPath_addPoly_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addPoly'>addPoly</a>(<a href='#SkPath_addPoly'>const</a> <a href='#SkPath_addPoly'>std</a>::<a href='#SkPath_addPoly'>initializer_list</a>&<a href='#SkPath_addPoly'>lt</a>;<a href='SkPoint_Reference#SkPoint'>SkPoint</a>&<a href='SkPoint_Reference#SkPoint'>gt</a>;& <a href='SkPoint_Reference#SkPoint'>list</a>, <a href='SkPoint_Reference#SkPoint'>bool</a> <a href='SkPoint_Reference#SkPoint'>close</a>)
</pre>

Adds <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>created</a> <a href='SkPath_Overview#Contour'>from</a> <a href='#SkPath_addPoly_2_list'>list</a>. <a href='SkPath_Overview#Contour'>Contour</a> <a href='SkPath_Overview#Contour'>added</a> <a href='SkPath_Overview#Contour'>starts</a> <a href='SkPath_Overview#Contour'>at</a> <a href='#SkPath_addPoly_2_list'>list</a>[0], <a href='#SkPath_addPoly_2_list'>then</a> <a href='#SkPath_addPoly_2_list'>adds</a> <a href='#SkPath_addPoly_2_list'>a</a> <a href='undocumented#Line'>line</a>
for every additional <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>in</a> <a href='#SkPath_addPoly_2_list'>list</a>. <a href='#SkPath_addPoly_2_list'>If</a> <a href='#SkPath_addPoly_2_close'>close</a> <a href='#SkPath_addPoly_2_close'>is</a> <a href='#SkPath_addPoly_2_close'>true</a>, <a href='#SkPath_addPoly_2_close'>appends</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a> <a href='#SkPath_kClose_Verb'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>,
connecting last and first <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>in</a> <a href='#SkPath_addPoly_2_list'>list</a>.

If <a href='#SkPath_addPoly_2_list'>list</a> <a href='#SkPath_addPoly_2_list'>is</a> <a href='#SkPath_addPoly_2_list'>empty</a>, <a href='#SkPath_addPoly_2_list'>append</a> <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>to</a> <a href='SkPath_Reference#Path'>path</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_addPoly_2_list'><code><strong>list</strong></code></a></td>
    <td>array of <a href='SkPoint_Reference#SkPoint'>SkPoint</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addPoly_2_close'><code><strong>close</strong></code></a></td>
    <td>true to add <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>connecting</a> <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>end</a> <a href='SkPath_Overview#Contour'>and</a> <a href='SkPath_Overview#Contour'>start</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="1a6b69acad5ceafede3c5984ec6634cb"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_drawPoints'>drawPoints</a>

<a name='SkPath_AddPathMode'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPath_AddPathMode'>AddPathMode</a> {
        <a href='#SkPath_kAppend_AddPathMode'>kAppend_AddPathMode</a>,
        <a href='#SkPath_kExtend_AddPathMode'>kExtend_AddPathMode</a>,
    };
</pre>

<a href='#SkPath_AddPathMode'>AddPathMode</a> <a href='#SkPath_AddPathMode'>chooses</a> <a href='#SkPath_AddPathMode'>how</a> <a href='#SkPath_addPath'>addPath</a> <a href='#SkPath_addPath'>appends</a>. <a href='#SkPath_addPath'>Adding</a> <a href='#SkPath_addPath'>one</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>to</a> <a href='SkPath_Reference#Path'>another</a> <a href='SkPath_Reference#Path'>can</a> <a href='SkPath_Reference#Path'>extend</a>
<a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Path'>last</a> <a href='SkPath_Overview#Contour'>Contour</a> <a href='SkPath_Overview#Contour'>or</a> <a href='SkPath_Overview#Contour'>start</a> <a href='SkPath_Overview#Contour'>a</a> <a href='SkPath_Overview#Contour'>new</a> <a href='SkPath_Overview#Contour'>Contour</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kAppend_AddPathMode'><code>SkPath::kAppend_AddPathMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#Line # appended to destination unaltered ##</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Verb'>Verbs</a>, <a href='SkPoint_Reference#Point'>Points</a>, <a href='SkPoint_Reference#Point'>and</a> <a href='#Path_Conic_Weight'>Conic_Weights</a> <a href='#Path_Conic_Weight'>are</a> <a href='#Path_Conic_Weight'>appended</a> <a href='#Path_Conic_Weight'>to</a> <a href='#Path_Conic_Weight'>destination</a> <a href='#Path_Conic_Weight'>unaltered</a>.
<a href='#Path_Conic_Weight'>Since</a> <a href='SkPath_Reference#Path'>Path</a> <a href='#Path_Verb_Array'>Verb_Array</a> <a href='#Path_Verb_Array'>begins</a> <a href='#Path_Verb_Array'>with</a> <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>if</a> <a href='#SkPath_kMove_Verb'>src</a> <a href='#SkPath_kMove_Verb'>is</a> <a href='#SkPath_kMove_Verb'>not</a> <a href='#SkPath_kMove_Verb'>empty</a>, <a href='#SkPath_kMove_Verb'>this</a>
<a href='#SkPath_kMove_Verb'>starts</a> <a href='#SkPath_kMove_Verb'>a</a> <a href='#SkPath_kMove_Verb'>new</a> <a href='SkPath_Overview#Contour'>Contour</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kExtend_AddPathMode'><code>SkPath::kExtend_AddPathMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#Line # add line if prior Contour is not closed ##</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
If destination is closed or empty, start a new <a href='SkPath_Overview#Contour'>Contour</a>. <a href='SkPath_Overview#Contour'>If</a> <a href='SkPath_Overview#Contour'>destination</a>
<a href='SkPath_Overview#Contour'>is</a> <a href='SkPath_Overview#Contour'>not</a> <a href='SkPath_Overview#Contour'>empty</a>, <a href='SkPath_Overview#Contour'>add</a> <a href='undocumented#Line'>Line</a> <a href='undocumented#Line'>from</a> <a href='#Path_Last_Point'>Last_Point</a> <a href='#Path_Last_Point'>to</a> <a href='#Path_Last_Point'>added</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>first</a> <a href='SkPoint_Reference#Point'>Point</a>. <a href='SkPoint_Reference#Point'>Skip</a> <a href='SkPoint_Reference#Point'>added</a>
<a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>initial</a> <a href='#SkPath_kMove_Verb'>kMove_Verb</a>, <a href='#SkPath_kMove_Verb'>then</a> <a href='#SkPath_kMove_Verb'>append</a> <a href='#SkPath_kMove_Verb'>remaining</a> <a href='SkPath_Reference#Verb'>Verbs</a>, <a href='SkPoint_Reference#Point'>Points</a>, <a href='SkPoint_Reference#Point'>and</a> <a href='#Path_Conic_Weight'>Conic_Weights</a>.
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="801b02e74c64aafdb734f2e5cf3e5ab0"><div>test is built from <a href='SkPath_Reference#Path'>path</a>, <a href='SkPath_Reference#Path'>open</a> <a href='SkPath_Reference#Path'>on</a> <a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Path'>top</a> <a href='SkPath_Reference#Path'>row</a>, <a href='SkPath_Reference#Path'>and</a> <a href='SkPath_Reference#Path'>closed</a> <a href='SkPath_Reference#Path'>on</a> <a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Path'>bottom</a> <a href='SkPath_Reference#Path'>row</a>.
<a href='SkPath_Reference#Path'>The</a> <a href='SkPath_Reference#Path'>left</a> <a href='SkPath_Reference#Path'>column</a> <a href='SkPath_Reference#Path'>uses</a> <a href='#SkPath_kAppend_AddPathMode'>kAppend_AddPathMode</a>; <a href='#SkPath_kAppend_AddPathMode'>the</a> <a href='#SkPath_kAppend_AddPathMode'>right</a> <a href='#SkPath_kAppend_AddPathMode'>uses</a> <a href='#SkPath_kExtend_AddPathMode'>kExtend_AddPathMode</a>.
<a href='#SkPath_kExtend_AddPathMode'>The</a> <a href='#SkPath_kExtend_AddPathMode'>top</a> <a href='#SkPath_kExtend_AddPathMode'>right</a> <a href='#SkPath_kExtend_AddPathMode'>composition</a> <a href='#SkPath_kExtend_AddPathMode'>is</a> <a href='#SkPath_kExtend_AddPathMode'>made</a> <a href='#SkPath_kExtend_AddPathMode'>up</a> <a href='#SkPath_kExtend_AddPathMode'>of</a> <a href='#SkPath_kExtend_AddPathMode'>one</a> <a href='SkPath_Overview#Contour'>contour</a>; <a href='SkPath_Overview#Contour'>the</a> <a href='SkPath_Overview#Contour'>other</a> <a href='SkPath_Overview#Contour'>three</a> <a href='SkPath_Overview#Contour'>have</a> <a href='SkPath_Overview#Contour'>two</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_addPath'>addPath</a> <a href='#SkPath_reverseAddPath'>reverseAddPath</a>

<a name='SkPath_addPath'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addPath'>addPath</a>(<a href='#SkPath_addPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>src</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>, <a href='#SkPath_AddPathMode'>AddPathMode</a> <a href='#SkPath_AddPathMode'>mode</a> = <a href='#SkPath_kAppend_AddPathMode'>kAppend_AddPathMode</a>)
</pre>

Appends <a href='#SkPath_addPath_src'>src</a> <a href='#SkPath_addPath_src'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>, <a href='SkPath_Reference#SkPath'>offset</a> <a href='SkPath_Reference#SkPath'>by</a> (<a href='#SkPath_addPath_dx'>dx</a>, <a href='#SkPath_addPath_dy'>dy</a>).

If <a href='#SkPath_addPath_mode'>mode</a> <a href='#SkPath_addPath_mode'>is</a> <a href='#SkPath_kAppend_AddPathMode'>kAppend_AddPathMode</a>, <a href='#SkPath_addPath_src'>src</a>  <a href='#Verb_Array'>verb array</a>,  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>, <a href='SkPoint_Reference#SkPoint'>and</a>  <a href='SkPath_Reference#Conic_Weight'>conic weights</a> <a href='SkPath_Reference#Conic'>are</a>
added unaltered. If <a href='#SkPath_addPath_mode'>mode</a> <a href='#SkPath_addPath_mode'>is</a> <a href='#SkPath_kExtend_AddPathMode'>kExtend_AddPathMode</a>, <a href='#SkPath_kExtend_AddPathMode'>add</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>before</a> <a href='undocumented#Line'>appending</a>
<a href='SkPath_Reference#Verb'>verbs</a>, <a href='SkPoint_Reference#SkPoint'>SkPoint</a>, <a href='SkPoint_Reference#SkPoint'>and</a>  <a href='SkPath_Reference#Conic_Weight'>conic weights</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_addPath_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#Verb'>verbs</a>, <a href='SkPoint_Reference#SkPoint'>SkPoint</a>, <a href='SkPoint_Reference#SkPoint'>and</a>  <a href='SkPath_Reference#Conic_Weight'>conic weights</a> <a href='SkPath_Reference#Conic'>to</a> <a href='SkPath_Reference#Conic'>add</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addPath_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#SkPath_addPath_src'>src</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>x-axis</a> <a href='SkPoint_Reference#SkPoint'>coordinates</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addPath_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#SkPath_addPath_src'>src</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>y-axis</a> <a href='SkPoint_Reference#SkPoint'>coordinates</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addPath_mode'><code><strong>mode</strong></code></a></td>
    <td><a href='#SkPath_kAppend_AddPathMode'>kAppend_AddPathMode</a> <a href='#SkPath_kAppend_AddPathMode'>or</a> <a href='#SkPath_kExtend_AddPathMode'>kExtend_AddPathMode</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="c416bddfe286628974e1c7f0fd66f3f4"></fiddle-embed></div>

### See Also

<a href='#SkPath_AddPathMode'>AddPathMode</a> <a href='#SkPath_offset'>offset</a> <a href='#SkPath_reverseAddPath'>reverseAddPath</a>

<a name='SkPath_addPath_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addPath'>addPath</a>(<a href='#SkPath_addPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>src</a>, <a href='#SkPath_AddPathMode'>AddPathMode</a> <a href='#SkPath_AddPathMode'>mode</a> = <a href='#SkPath_kAppend_AddPathMode'>kAppend_AddPathMode</a>)
</pre>

Appends <a href='#SkPath_addPath_2_src'>src</a> <a href='#SkPath_addPath_2_src'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>.

If <a href='#SkPath_addPath_2_mode'>mode</a> <a href='#SkPath_addPath_2_mode'>is</a> <a href='#SkPath_kAppend_AddPathMode'>kAppend_AddPathMode</a>, <a href='#SkPath_addPath_2_src'>src</a>  <a href='#Verb_Array'>verb array</a>,  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>, <a href='SkPoint_Reference#SkPoint'>and</a>  <a href='SkPath_Reference#Conic_Weight'>conic weights</a> <a href='SkPath_Reference#Conic'>are</a>
added unaltered. If <a href='#SkPath_addPath_2_mode'>mode</a> <a href='#SkPath_addPath_2_mode'>is</a> <a href='#SkPath_kExtend_AddPathMode'>kExtend_AddPathMode</a>, <a href='#SkPath_kExtend_AddPathMode'>add</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>before</a> <a href='undocumented#Line'>appending</a>
<a href='SkPath_Reference#Verb'>verbs</a>, <a href='SkPoint_Reference#SkPoint'>SkPoint</a>, <a href='SkPoint_Reference#SkPoint'>and</a>  <a href='SkPath_Reference#Conic_Weight'>conic weights</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_addPath_2_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#Verb'>verbs</a>, <a href='SkPoint_Reference#SkPoint'>SkPoint</a>, <a href='SkPoint_Reference#SkPoint'>and</a>  <a href='SkPath_Reference#Conic_Weight'>conic weights</a> <a href='SkPath_Reference#Conic'>to</a> <a href='SkPath_Reference#Conic'>add</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addPath_2_mode'><code><strong>mode</strong></code></a></td>
    <td><a href='#SkPath_kAppend_AddPathMode'>kAppend_AddPathMode</a> <a href='#SkPath_kAppend_AddPathMode'>or</a> <a href='#SkPath_kExtend_AddPathMode'>kExtend_AddPathMode</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="84b2d1c0fc29f1b35e855b6fc6672f9e"></fiddle-embed></div>

### See Also

<a href='#SkPath_AddPathMode'>AddPathMode</a> <a href='#SkPath_reverseAddPath'>reverseAddPath</a>

<a name='SkPath_addPath_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_addPath'>addPath</a>(<a href='#SkPath_addPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>src</a>, <a href='SkPath_Reference#SkPath'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#Matrix'>matrix</a>, <a href='#SkPath_AddPathMode'>AddPathMode</a> <a href='#SkPath_AddPathMode'>mode</a> = <a href='#SkPath_kAppend_AddPathMode'>kAppend_AddPathMode</a>)
</pre>

Appends <a href='#SkPath_addPath_3_src'>src</a> <a href='#SkPath_addPath_3_src'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>, <a href='SkPath_Reference#SkPath'>transformed</a> <a href='SkPath_Reference#SkPath'>by</a> <a href='#SkPath_addPath_3_matrix'>matrix</a>. <a href='#SkPath_addPath_3_matrix'>Transformed</a> <a href='undocumented#Curve'>curves</a> <a href='undocumented#Curve'>may</a> <a href='undocumented#Curve'>have</a> <a href='undocumented#Curve'>different</a>
<a href='SkPath_Reference#Verb'>verbs</a>, <a href='SkPoint_Reference#SkPoint'>SkPoint</a>, <a href='SkPoint_Reference#SkPoint'>and</a>  <a href='SkPath_Reference#Conic_Weight'>conic weights</a>.

If <a href='#SkPath_addPath_3_mode'>mode</a> <a href='#SkPath_addPath_3_mode'>is</a> <a href='#SkPath_kAppend_AddPathMode'>kAppend_AddPathMode</a>, <a href='#SkPath_addPath_3_src'>src</a>  <a href='#Verb_Array'>verb array</a>,  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>, <a href='SkPoint_Reference#SkPoint'>and</a>  <a href='SkPath_Reference#Conic_Weight'>conic weights</a> <a href='SkPath_Reference#Conic'>are</a>
added unaltered. If <a href='#SkPath_addPath_3_mode'>mode</a> <a href='#SkPath_addPath_3_mode'>is</a> <a href='#SkPath_kExtend_AddPathMode'>kExtend_AddPathMode</a>, <a href='#SkPath_kExtend_AddPathMode'>add</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>before</a> <a href='undocumented#Line'>appending</a>
<a href='SkPath_Reference#Verb'>verbs</a>, <a href='SkPoint_Reference#SkPoint'>SkPoint</a>, <a href='SkPoint_Reference#SkPoint'>and</a>  <a href='SkPath_Reference#Conic_Weight'>conic weights</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_addPath_3_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#Verb'>verbs</a>, <a href='SkPoint_Reference#SkPoint'>SkPoint</a>, <a href='SkPoint_Reference#SkPoint'>and</a>  <a href='SkPath_Reference#Conic_Weight'>conic weights</a> <a href='SkPath_Reference#Conic'>to</a> <a href='SkPath_Reference#Conic'>add</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addPath_3_matrix'><code><strong>matrix</strong></code></a></td>
    <td>transform applied to <a href='#SkPath_addPath_3_src'>src</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addPath_3_mode'><code><strong>mode</strong></code></a></td>
    <td><a href='#SkPath_kAppend_AddPathMode'>kAppend_AddPathMode</a> <a href='#SkPath_kAppend_AddPathMode'>or</a> <a href='#SkPath_kExtend_AddPathMode'>kExtend_AddPathMode</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="3a90a91030f7289d5df0671d342dbbad"></fiddle-embed></div>

### See Also

<a href='#SkPath_AddPathMode'>AddPathMode</a> <a href='#SkPath_transform'>transform</a> <a href='#SkPath_offset'>offset</a> <a href='#SkPath_reverseAddPath'>reverseAddPath</a>

<a name='SkPath_reverseAddPath'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='#SkPath_reverseAddPath'>reverseAddPath</a>(<a href='#SkPath_reverseAddPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#SkPath'>src</a>)
</pre>

Appends <a href='#SkPath_reverseAddPath_src'>src</a> <a href='#SkPath_reverseAddPath_src'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>, <a href='SkPath_Reference#SkPath'>from</a> <a href='SkPath_Reference#SkPath'>back</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>front</a>.
Reversed <a href='#SkPath_reverseAddPath_src'>src</a> <a href='#SkPath_reverseAddPath_src'>always</a> <a href='#SkPath_reverseAddPath_src'>appends</a> <a href='#SkPath_reverseAddPath_src'>a</a> <a href='#SkPath_reverseAddPath_src'>new</a> <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_reverseAddPath_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#Verb'>verbs</a>, <a href='SkPoint_Reference#SkPoint'>SkPoint</a>, <a href='SkPoint_Reference#SkPoint'>and</a>  <a href='SkPath_Reference#Conic_Weight'>conic weights</a> <a href='SkPath_Reference#Conic'>to</a> <a href='SkPath_Reference#Conic'>add</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="5e8513f073db09acde3ff616f6426e3d"></fiddle-embed></div>

### See Also

<a href='#SkPath_AddPathMode'>AddPathMode</a> <a href='#SkPath_transform'>transform</a> <a href='#SkPath_offset'>offset</a> <a href='#SkPath_addPath'>addPath</a>

<a name='SkPath_offset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void offset(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>, <a href='SkPath_Reference#SkPath'>SkPath</a>* <a href='SkPath_Reference#SkPath'>dst</a>) <a href='SkPath_Reference#SkPath'>const</a>
</pre>

Offsets  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>by</a> (<a href='#SkPath_offset_dx'>dx</a>, <a href='#SkPath_offset_dy'>dy</a>). <a href='#SkPath_offset_dy'>Offset</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>replaces</a> <a href='#SkPath_offset_dst'>dst</a>.
If <a href='#SkPath_offset_dst'>dst</a> <a href='#SkPath_offset_dst'>is</a> <a href='#SkPath_offset_dst'>nullptr</a>, <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>replaced</a> <a href='SkPath_Reference#SkPath'>by</a> <a href='SkPath_Reference#SkPath'>offset</a> <a href='undocumented#Data'>data</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_offset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>x-axis</a> <a href='SkPoint_Reference#SkPoint'>coordinates</a></td>
  </tr>
  <tr>    <td><a name='SkPath_offset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>y-axis</a> <a href='SkPoint_Reference#SkPoint'>coordinates</a></td>
  </tr>
  <tr>    <td><a name='SkPath_offset_dst'><code><strong>dst</strong></code></a></td>
    <td>overwritten, translated copy of <a href='SkPath_Reference#SkPath'>SkPath</a>; <a href='SkPath_Reference#SkPath'>may</a> <a href='SkPath_Reference#SkPath'>be</a> <a href='SkPath_Reference#SkPath'>nullptr</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1d1892196ba5bda257df4f3351abd084"></fiddle-embed></div>

### See Also

<a href='#SkPath_addPath'>addPath</a> <a href='#SkPath_transform'>transform</a>

<a name='Transform'></a>

<a name='SkPath_offset_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void offset(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>)
</pre>

Offsets  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>by</a> (<a href='#SkPath_offset_2_dx'>dx</a>, <a href='#SkPath_offset_2_dy'>dy</a>). <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>replaced</a> <a href='SkPath_Reference#SkPath'>by</a> <a href='SkPath_Reference#SkPath'>offset</a> <a href='undocumented#Data'>data</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_offset_2_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>x-axis</a> <a href='SkPoint_Reference#SkPoint'>coordinates</a></td>
  </tr>
  <tr>    <td><a name='SkPath_offset_2_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>y-axis</a> <a href='SkPoint_Reference#SkPoint'>coordinates</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="5188d77585715db30bef228f2dfbcccd"></fiddle-embed></div>

### See Also

<a href='#SkPath_addPath'>addPath</a> <a href='#SkPath_transform'>transform</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_translate'>translate()</a>

<a name='SkPath_transform'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void transform(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#Matrix'>matrix</a>, <a href='SkPath_Reference#SkPath'>SkPath</a>* <a href='SkPath_Reference#SkPath'>dst</a>) <a href='SkPath_Reference#SkPath'>const</a>
</pre>

Transforms  <a href='#Verb_Array'>verb array</a>,  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>, <a href='SkPoint_Reference#SkPoint'>and</a> <a href='SkPoint_Reference#SkPoint'>weight</a> <a href='SkPoint_Reference#SkPoint'>by</a> <a href='#SkPath_transform_matrix'>matrix</a>.
transform may change <a href='SkPath_Reference#Verb'>verbs</a> <a href='SkPath_Reference#Verb'>and</a> <a href='SkPath_Reference#Verb'>increase</a> <a href='SkPath_Reference#Verb'>their</a> <a href='SkPath_Reference#Verb'>number</a>.
Transformed <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>replaces</a> <a href='#SkPath_transform_dst'>dst</a>; <a href='#SkPath_transform_dst'>if</a> <a href='#SkPath_transform_dst'>dst</a> <a href='#SkPath_transform_dst'>is</a> <a href='#SkPath_transform_dst'>nullptr</a>, <a href='#SkPath_transform_dst'>original</a> <a href='undocumented#Data'>data</a>
is replaced.

### Parameters

<table>  <tr>    <td><a name='SkPath_transform_matrix'><code><strong>matrix</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>apply</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a></td>
  </tr>
  <tr>    <td><a name='SkPath_transform_dst'><code><strong>dst</strong></code></a></td>
    <td>overwritten, transformed copy of <a href='SkPath_Reference#SkPath'>SkPath</a>; <a href='SkPath_Reference#SkPath'>may</a> <a href='SkPath_Reference#SkPath'>be</a> <a href='SkPath_Reference#SkPath'>nullptr</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="99761add116ce3b0730557224c1b0105"></fiddle-embed></div>

### See Also

<a href='#SkPath_addPath'>addPath</a> <a href='#SkPath_offset'>offset</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_concat'>concat()</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>

<a name='SkPath_transform_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void transform(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#Matrix'>matrix</a>)
</pre>

Transforms  <a href='#Verb_Array'>verb array</a>,  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>, <a href='SkPoint_Reference#SkPoint'>and</a> <a href='SkPoint_Reference#SkPoint'>weight</a> <a href='SkPoint_Reference#SkPoint'>by</a> <a href='#SkPath_transform_2_matrix'>matrix</a>.
transform may change <a href='SkPath_Reference#Verb'>verbs</a> <a href='SkPath_Reference#Verb'>and</a> <a href='SkPath_Reference#Verb'>increase</a> <a href='SkPath_Reference#Verb'>their</a> <a href='SkPath_Reference#Verb'>number</a>.
<a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>replaced</a> <a href='SkPath_Reference#SkPath'>by</a> <a href='SkPath_Reference#SkPath'>transformed</a> <a href='undocumented#Data'>data</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_transform_2_matrix'><code><strong>matrix</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>apply</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkPath_Reference#SkPath'>SkPath</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c40979a3b92a30cfb7bae36abcc1d805"></fiddle-embed></div>

### See Also

<a href='#SkPath_addPath'>addPath</a> <a href='#SkPath_offset'>offset</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_concat'>concat()</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>

<a name='Last_Point'></a>

<a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>is</a> <a href='SkPath_Reference#Path'>defined</a> <a href='SkPath_Reference#Path'>cumulatively</a>, <a href='SkPath_Reference#Path'>often</a> <a href='SkPath_Reference#Path'>by</a> <a href='SkPath_Reference#Path'>adding</a> <a href='SkPath_Reference#Path'>a</a> <a href='SkPath_Reference#Path'>segment</a> <a href='SkPath_Reference#Path'>to</a> <a href='SkPath_Reference#Path'>the</a> <a href='SkPath_Reference#Path'>end</a> <a href='SkPath_Reference#Path'>of</a> <a href='SkPath_Reference#Path'>last</a>
<a href='SkPath_Overview#Contour'>Contour</a>. <a href='#Path_Last_Point'>Last_Point</a> <a href='#Path_Last_Point'>of</a> <a href='SkPath_Overview#Contour'>Contour</a> <a href='SkPath_Overview#Contour'>is</a> <a href='SkPath_Overview#Contour'>shared</a> <a href='SkPath_Overview#Contour'>as</a> <a href='SkPath_Overview#Contour'>first</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>of</a> <a href='SkPoint_Reference#Point'>added</a> <a href='undocumented#Line'>Line</a> <a href='undocumented#Line'>or</a> <a href='undocumented#Curve'>Curve</a>.
<a href='#Path_Last_Point'>Last_Point</a> <a href='#Path_Last_Point'>can</a> <a href='#Path_Last_Point'>be</a> <a href='#Path_Last_Point'>read</a> <a href='#Path_Last_Point'>and</a> <a href='#Path_Last_Point'>written</a> <a href='#Path_Last_Point'>directly</a> <a href='#Path_Last_Point'>with</a> <a href='#SkPath_getLastPt'>getLastPt</a> <a href='#SkPath_getLastPt'>and</a> <a href='#SkPath_setLastPt'>setLastPt</a>.

<a name='SkPath_getLastPt'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_getLastPt'>getLastPt</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a>* <a href='SkPoint_Reference#SkPoint'>lastPt</a>) <a href='SkPoint_Reference#SkPoint'>const</a>
</pre>

Returns  <a href='#Last_Point'>last point</a> on <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>in</a> <a href='#SkPath_getLastPt_lastPt'>lastPt</a>. <a href='#SkPath_getLastPt_lastPt'>Returns</a> <a href='#SkPath_getLastPt_lastPt'>false</a> <a href='#SkPath_getLastPt_lastPt'>if</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>is</a> <a href='SkPoint_Reference#SkPoint'>empty</a>,
storing (0, 0) if <a href='#SkPath_getLastPt_lastPt'>lastPt</a> <a href='#SkPath_getLastPt_lastPt'>is</a> <a href='#SkPath_getLastPt_lastPt'>not</a> <a href='#SkPath_getLastPt_lastPt'>nullptr</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_getLastPt_lastPt'><code><strong>lastPt</strong></code></a></td>
    <td>storage for final <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>in</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>; <a href='SkPoint_Reference#SkPoint'>may</a> <a href='SkPoint_Reference#SkPoint'>be</a> <a href='SkPoint_Reference#SkPoint'>nullptr</a></td>
  </tr>
</table>

### Return Value

true if  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>contains</a> <a href='SkPoint_Reference#SkPoint'>one</a> <a href='SkPoint_Reference#SkPoint'>or</a> <a href='SkPoint_Reference#SkPoint'>more</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>

### Example

<div><fiddle-embed name="df8160dd7ac8aa4b40fce7286fe49952">

#### Example Output

~~~~
last point: 35.2786, 52.9772
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_setLastPt'>setLastPt</a>

<a name='SkPath_setLastPt'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_setLastPt'>setLastPt</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>)
</pre>

Sets  <a href='#Last_Point'>last point</a> to (<a href='#SkPath_setLastPt_x'>x</a>, <a href='#SkPath_setLastPt_y'>y</a>). <a href='#SkPath_setLastPt_y'>If</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>is</a> <a href='SkPoint_Reference#SkPoint'>empty</a>, <a href='SkPoint_Reference#SkPoint'>append</a> <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>to</a>
<a href='#Verb_Array'>verb array</a> and append (<a href='#SkPath_setLastPt_x'>x</a>, <a href='#SkPath_setLastPt_y'>y</a>) <a href='#SkPath_setLastPt_y'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_setLastPt_x'><code><strong>x</strong></code></a></td>
    <td>set x-axis value of  <a href='#Last_Point'>last point</a></td>
  </tr>
  <tr>    <td><a name='SkPath_setLastPt_y'><code><strong>y</strong></code></a></td>
    <td>set y-axis value of  <a href='#Last_Point'>last point</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="542c5afaea5f57baa11d0561dd402e18"></fiddle-embed></div>

### See Also

<a href='#SkPath_getLastPt'>getLastPt</a>

<a name='SkPath_setLastPt_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_setLastPt'>setLastPt</a>(<a href='#SkPath_setLastPt'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& <a href='SkPoint_Reference#SkPoint'>p</a>)
</pre>

Sets the  <a href='#Last_Point'>last point</a> on the <a href='SkPath_Reference#Path'>path</a>. <a href='SkPath_Reference#Path'>If</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>is</a> <a href='SkPoint_Reference#SkPoint'>empty</a>, <a href='SkPoint_Reference#SkPoint'>append</a> <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>to</a>
<a href='#Verb_Array'>verb array</a> and append <a href='#SkPath_setLastPt_2_p'>p</a> <a href='#SkPath_setLastPt_2_p'>to</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_setLastPt_2_p'><code><strong>p</strong></code></a></td>
    <td>set value of  <a href='#Last_Point'>last point</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6fa5e8f9513b3225e106778592e27e94"></fiddle-embed></div>

### See Also

<a href='#SkPath_getLastPt'>getLastPt</a>

<a name='SkPath_SegmentMask'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPath_SegmentMask'>SegmentMask</a> {
        <a href='#SkPath_kLine_SegmentMask'>kLine_SegmentMask</a> = 1 << 0,
        <a href='#SkPath_kQuad_SegmentMask'>kQuad_SegmentMask</a> = 1 << 1,
        <a href='#SkPath_kConic_SegmentMask'>kConic_SegmentMask</a> = 1 << 2,
        <a href='#SkPath_kCubic_SegmentMask'>kCubic_SegmentMask</a> = 1 << 3,
    };
</pre>

<a href='#SkPath_SegmentMask'>SegmentMask</a> <a href='#SkPath_SegmentMask'>constants</a> <a href='#SkPath_SegmentMask'>correspond</a> <a href='#SkPath_SegmentMask'>to</a> <a href='#SkPath_SegmentMask'>each</a> <a href='#SkPath_SegmentMask'>drawing</a> <a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>type</a> <a href='#SkPath_Verb'>in</a> <a href='SkPath_Reference#Path'>Path</a>; <a href='SkPath_Reference#Path'>for</a>
<a href='SkPath_Reference#Path'>instance</a>, <a href='SkPath_Reference#Path'>if</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>only</a> <a href='SkPath_Reference#Path'>contains</a> <a href='undocumented#Line'>Lines</a>, <a href='undocumented#Line'>only</a> <a href='undocumented#Line'>the</a> <a href='#SkPath_kLine_SegmentMask'>kLine_SegmentMask</a> <a href='#SkPath_kLine_SegmentMask'>bit</a> <a href='#SkPath_kLine_SegmentMask'>is</a> <a href='#SkPath_kLine_SegmentMask'>set</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kLine_SegmentMask'><code>SkPath::kLine_SegmentMask</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Set if <a href='#Path_Verb_Array'>Verb_Array</a> <a href='#Path_Verb_Array'>contains</a> <a href='#SkPath_kLine_Verb'>kLine_Verb</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kQuad_SegmentMask'><code>SkPath::kQuad_SegmentMask</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Set if <a href='#Path_Verb_Array'>Verb_Array</a> <a href='#Path_Verb_Array'>contains</a> <a href='#SkPath_kQuad_Verb'>kQuad_Verb</a>. <a href='#SkPath_kQuad_Verb'>Note</a> <a href='#SkPath_kQuad_Verb'>that</a> <a href='#SkPath_conicTo'>conicTo</a> <a href='#SkPath_conicTo'>may</a> <a href='#SkPath_conicTo'>add</a> <a href='#SkPath_conicTo'>a</a> <a href='SkPath_Reference#Quad'>Quad</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kConic_SegmentMask'><code>SkPath::kConic_SegmentMask</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Set if <a href='#Path_Verb_Array'>Verb_Array</a> <a href='#Path_Verb_Array'>contains</a> <a href='#SkPath_kConic_Verb'>kConic_Verb</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kCubic_SegmentMask'><code>SkPath::kCubic_SegmentMask</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>8</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Set if <a href='#Path_Verb_Array'>Verb_Array</a> <a href='#Path_Verb_Array'>contains</a> <a href='#SkPath_kCubic_Verb'>kCubic_Verb</a>.
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a61e5758574e28190ec4ed8c4ae7e7fa"><div>When <a href='#SkPath_conicTo'>conicTo</a> <a href='#SkPath_conicTo'>has</a> <a href='#SkPath_conicTo'>a</a> <a href='#SkPath_conicTo'>weight</a> <a href='#SkPath_conicTo'>of</a> <a href='#SkPath_conicTo'>one</a>, <a href='SkPath_Reference#Quad'>Quad</a> <a href='SkPath_Reference#Quad'>is</a> <a href='SkPath_Reference#Quad'>added</a> <a href='SkPath_Reference#Quad'>to</a> <a href='SkPath_Reference#Path'>Path</a>.
</div>

#### Example Output

~~~~
Path kConic_SegmentMask is clear
Path kQuad_SegmentMask is set
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_getSegmentMasks'>getSegmentMasks</a> <a href='#SkPath_Verb'>Verb</a>

<a name='SkPath_getSegmentMasks'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkPath_getSegmentMasks'>getSegmentMasks</a>() <a href='#SkPath_getSegmentMasks'>const</a>
</pre>

Returns a mask, where each set bit corresponds to a <a href='#SkPath_SegmentMask'>SegmentMask</a> <a href='#SkPath_SegmentMask'>constant</a>
if <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>contains</a> <a href='SkPath_Reference#SkPath'>one</a> <a href='SkPath_Reference#SkPath'>or</a> <a href='SkPath_Reference#SkPath'>more</a> <a href='SkPath_Reference#Verb'>verbs</a> <a href='SkPath_Reference#Verb'>of</a> <a href='SkPath_Reference#Verb'>that</a> <a href='SkPath_Reference#Verb'>type</a>.
Returns zero if <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>contains</a> <a href='SkPath_Reference#SkPath'>no</a> <a href='undocumented#Line'>lines</a>, <a href='undocumented#Line'>or</a> <a href='undocumented#Curve'>curves</a>: <a href='SkPath_Reference#Quad'>quads</a>, <a href='SkPath_Reference#Conic'>conics</a>, <a href='SkPath_Reference#Conic'>or</a> <a href='SkPath_Reference#Cubic'>cubics</a>.

<a href='#SkPath_getSegmentMasks'>getSegmentMasks</a>() <a href='#SkPath_getSegmentMasks'>returns</a> <a href='#SkPath_getSegmentMasks'>a</a> <a href='#SkPath_getSegmentMasks'>cached</a> <a href='#SkPath_getSegmentMasks'>result</a>; <a href='#SkPath_getSegmentMasks'>it</a> <a href='#SkPath_getSegmentMasks'>is</a> <a href='#SkPath_getSegmentMasks'>very</a> <a href='#SkPath_getSegmentMasks'>fast</a>.

### Return Value

<a href='#SkPath_SegmentMask'>SegmentMask</a> <a href='#SkPath_SegmentMask'>bits</a> <a href='#SkPath_SegmentMask'>or</a> <a href='#SkPath_SegmentMask'>zero</a>

### Example

<div><fiddle-embed name="657a3f3e11acafea92b84d6bb0c13633">

#### Example Output

~~~~
mask quad set
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_getSegmentMasks'>getSegmentMasks</a> <a href='#SkPath_Verb'>Verb</a>

<a name='SkPath_contains'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool contains(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>) <a href='undocumented#SkScalar'>const</a>
</pre>

Returns true if the <a href='SkPoint_Reference#Point'>point</a> (<a href='#SkPath_contains_x'>x</a>, <a href='#SkPath_contains_y'>y</a>) <a href='#SkPath_contains_y'>is</a> <a href='#SkPath_contains_y'>contained</a> <a href='#SkPath_contains_y'>by</a> <a href='SkPath_Reference#Path'>Path</a>, <a href='SkPath_Reference#Path'>taking</a> <a href='SkPath_Reference#Path'>into</a>
<a href='SkPath_Reference#Path'>account</a> <a href='#SkPath_FillType'>FillType</a>.

| <a href='#SkPath_FillType'>FillType</a> | <a href='#SkPath_contains'>contains()</a> <a href='#SkPath_contains'>returns</a> <a href='#SkPath_contains'>true</a> <a href='#SkPath_contains'>if</a> <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>is</a> <a href='SkPoint_Reference#Point'>enclosed</a> <a href='SkPoint_Reference#Point'>by</a> |
| --- | ---  |
| <a href='#SkPath_kWinding_FillType'>kWinding_FillType</a> | a non-zero sum of <a href='SkPath_Overview#Contour'>Contour</a> <a href='SkPath_Reference#Direction'>Directions</a>. |
| <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd_FillType</a> | an odd number of <a href='SkPath_Overview#Contour'>Contours</a>. |
| <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding_FillType</a> | a zero sum of <a href='SkPath_Overview#Contour'>Contour</a> <a href='SkPath_Reference#Direction'>Directions</a>. |
| <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd_FillType</a> | and even number of <a href='SkPath_Overview#Contour'>Contours</a>. |

### Parameters

<table>  <tr>    <td><a name='SkPath_contains_x'><code><strong>x</strong></code></a></td>
    <td>x-axis value of containment test</td>
  </tr>
  <tr>    <td><a name='SkPath_contains_y'><code><strong>y</strong></code></a></td>
    <td>y-axis value of containment test</td>
  </tr>
</table>

### Return Value

true if <a href='SkPoint_Reference#Point'>Point</a> <a href='SkPoint_Reference#Point'>is</a> <a href='SkPoint_Reference#Point'>in</a> <a href='SkPath_Reference#Path'>Path</a>

### Example

<div><fiddle-embed name="c0216b3f7ebd80b9589ae5728f08fc80"></fiddle-embed></div>

### See Also

<a href='#SkPath_conservativelyContainsRect'>conservativelyContainsRect</a> <a href='#Path_Fill_Type'>Fill_Type</a> <a href='undocumented#Op'>Op</a>

<a name='SkPath_dump'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_dump'>dump</a>(<a href='SkWStream_Reference#SkWStream'>SkWStream</a>* <a href='SkStream_Reference#Stream'>stream</a>, <a href='SkStream_Reference#Stream'>bool</a> <a href='SkStream_Reference#Stream'>forceClose</a>, <a href='SkStream_Reference#Stream'>bool</a> <a href='SkStream_Reference#Stream'>dumpAsHex</a>) <a href='SkStream_Reference#Stream'>const</a>
</pre>

Writes <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>representation</a> <a href='undocumented#Text'>of</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='#SkPath_dump_stream'>stream</a>. <a href='#SkPath_dump_stream'>If</a> <a href='#SkPath_dump_stream'>stream</a> <a href='#SkPath_dump_stream'>is</a> <a href='#SkPath_dump_stream'>nullptr</a>, <a href='#SkPath_dump_stream'>writes</a> <a href='#SkPath_dump_stream'>to</a>
standard output. Set <a href='#SkPath_dump_forceClose'>forceClose</a> <a href='#SkPath_dump_forceClose'>to</a> <a href='#SkPath_dump_forceClose'>true</a> <a href='#SkPath_dump_forceClose'>to</a> <a href='#SkPath_dump_forceClose'>get</a> <a href='#SkPath_dump_forceClose'>edges</a> <a href='#SkPath_dump_forceClose'>used</a> <a href='#SkPath_dump_forceClose'>to</a> <a href='#SkPath_dump_forceClose'>fill</a> <a href='SkPath_Reference#SkPath'>SkPath</a>.
Set <a href='#SkPath_dump_dumpAsHex'>dumpAsHex</a> <a href='#SkPath_dump_dumpAsHex'>true</a> <a href='#SkPath_dump_dumpAsHex'>to</a> <a href='#SkPath_dump_dumpAsHex'>generate</a> <a href='#SkPath_dump_dumpAsHex'>exact</a> <a href='#SkPath_dump_dumpAsHex'>binary</a> <a href='#SkPath_dump_dumpAsHex'>representations</a>
of floating <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>numbers</a> <a href='SkPoint_Reference#Point'>used</a> <a href='SkPoint_Reference#Point'>in</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>and</a>  <a href='SkPath_Reference#Conic_Weight'>conic weights</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_dump_stream'><code><strong>stream</strong></code></a></td>
    <td>writable <a href='SkWStream_Reference#SkWStream'>SkWStream</a> <a href='SkWStream_Reference#SkWStream'>receiving</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>representation</a>; <a href='undocumented#Text'>may</a> <a href='undocumented#Text'>be</a> <a href='undocumented#Text'>nullptr</a></td>
  </tr>
  <tr>    <td><a name='SkPath_dump_forceClose'><code><strong>forceClose</strong></code></a></td>
    <td>true if missing <a href='#SkPath_kClose_Verb'>kClose_Verb</a> <a href='#SkPath_kClose_Verb'>is</a> <a href='#SkPath_kClose_Verb'>output</a></td>
  </tr>
  <tr>    <td><a name='SkPath_dump_dumpAsHex'><code><strong>dumpAsHex</strong></code></a></td>
    <td>true if <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>values</a> <a href='undocumented#SkScalar'>are</a> <a href='undocumented#SkScalar'>written</a> <a href='undocumented#SkScalar'>as</a> <a href='undocumented#SkScalar'>hexadecimal</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="8036d764452a62f9953af50846f0f3c0">

#### Example Output

~~~~
path.setFillType(SkPath::kWinding_FillType);
path.moveTo(0, 0);
path.quadTo(20, 30, 40, 50);
path.setFillType(SkPath::kWinding_FillType);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.quadTo(SkBits2Float(0x41a00000), SkBits2Float(0x41f00000), SkBits2Float(0x42200000), SkBits2Float(0x42480000));  // 20, 30, 40, 50
path.setFillType(SkPath::kWinding_FillType);
path.moveTo(0, 0);
path.quadTo(20, 30, 40, 50);
path.lineTo(0, 0);
path.close();
path.setFillType(SkPath::kWinding_FillType);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.quadTo(SkBits2Float(0x41a00000), SkBits2Float(0x41f00000), SkBits2Float(0x42200000), SkBits2Float(0x42480000));  // 20, 30, 40, 50
path.lineTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.close();
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_dumpHex'>dumpHex</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_dump'>dump()</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>::<a href='#SkRRect_dump'>dump()</a> <a href='undocumented#SkPathMeasure'>SkPathMeasure</a>::<a href='#SkPathMeasure_dump'>dump()</a>

<a name='SkPath_dump_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_dump'>dump()</a> <a href='#SkPath_dump'>const</a>
</pre>

Writes <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>representation</a> <a href='undocumented#Text'>of</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>standard</a> <a href='SkPath_Reference#SkPath'>output</a>. <a href='SkPath_Reference#SkPath'>The</a> <a href='SkPath_Reference#SkPath'>representation</a> <a href='SkPath_Reference#SkPath'>may</a> <a href='SkPath_Reference#SkPath'>be</a>
directly compiled as C++ code. Floating <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>values</a> <a href='SkPoint_Reference#Point'>are</a> <a href='SkPoint_Reference#Point'>written</a>
with limited precision; it may not be possible to reconstruct original <a href='SkPath_Reference#SkPath'>SkPath</a>
from output.

### Example

<div><fiddle-embed name="92e0032f85181795d1f8b5a2c8e4e4b7">

#### Example Output

~~~~
path.setFillType(SkPath::kWinding_FillType);
path.moveTo(0, 0);
path.lineTo(0.857143f, 0.666667f);
path is not equal to copy
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_dumpHex'>dumpHex</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_dump'>dump()</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>::<a href='#SkRRect_dump'>dump()</a> <a href='undocumented#SkPathMeasure'>SkPathMeasure</a>::<a href='#SkPathMeasure_dump'>dump()</a> <a href='#SkPath_writeToMemory'>writeToMemory</a>

<a name='SkPath_dumpHex'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_dumpHex'>dumpHex</a>() <a href='#SkPath_dumpHex'>const</a>
</pre>

Writes <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>representation</a> <a href='undocumented#Text'>of</a> <a href='SkPath_Reference#Path'>Path</a> <a href='SkPath_Reference#Path'>to</a> <a href='SkPath_Reference#Path'>standard</a> <a href='SkPath_Reference#Path'>output</a>. <a href='SkPath_Reference#Path'>The</a> <a href='SkPath_Reference#Path'>representation</a> <a href='SkPath_Reference#Path'>may</a> <a href='SkPath_Reference#Path'>be</a>
<a href='SkPath_Reference#Path'>directly</a> <a href='SkPath_Reference#Path'>compiled</a> <a href='SkPath_Reference#Path'>as</a> <a href='SkPath_Reference#Path'>C</a>++ <a href='SkPath_Reference#Path'>code</a>. <a href='SkPath_Reference#Path'>Floating</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>values</a> <a href='SkPoint_Reference#Point'>are</a> <a href='SkPoint_Reference#Point'>written</a>
<a href='SkPoint_Reference#Point'>in</a> <a href='SkPoint_Reference#Point'>hexadecimal</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>preserve</a> <a href='SkPoint_Reference#Point'>their</a> <a href='SkPoint_Reference#Point'>exact</a> <a href='SkPoint_Reference#Point'>bit</a> <a href='SkPoint_Reference#Point'>pattern</a>. <a href='SkPoint_Reference#Point'>The</a> <a href='SkPoint_Reference#Point'>output</a> <a href='SkPoint_Reference#Point'>reconstructs</a> <a href='SkPoint_Reference#Point'>the</a>
<a href='SkPoint_Reference#Point'>original</a> <a href='SkPath_Reference#Path'>Path</a>.

<a href='SkPath_Reference#Path'>Use</a> <a href='SkPath_Reference#Path'>instead</a> <a href='SkPath_Reference#Path'>of</a> <a href='#SkPath_dump'>dump()</a> <a href='#SkPath_dump'>when</a> <a href='#SkPath_dump'>submitting</a>
<a href='https://bug.skia.org'>bug reports against Skia</a></a> .

### Example

<div><fiddle-embed name="72a92fe058e8b3be6c8a30fad7fd1266">

#### Example Output

~~~~
path.setFillType(SkPath::kWinding_FillType);
path.moveTo(SkBits2Float(0x00000000), SkBits2Float(0x00000000));  // 0, 0
path.lineTo(SkBits2Float(0x3f5b6db7), SkBits2Float(0x3f2aaaab));  // 0.857143f, 0.666667f
path is equal to copy
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_dump'>dump</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_dumpHex'>dumpHex</a> <a href='SkRRect_Reference#SkRRect'>SkRRect</a>::<a href='#SkRRect_dumpHex'>dumpHex</a> <a href='#SkPath_writeToMemory'>writeToMemory</a>

<a name='SkPath_writeToMemory'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkPath_writeToMemory'>writeToMemory</a>(<a href='#SkPath_writeToMemory'>void</a>* <a href='#SkPath_writeToMemory'>buffer</a>) <a href='#SkPath_writeToMemory'>const</a>
</pre>

Writes <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='#SkPath_writeToMemory_buffer'>buffer</a>, <a href='#SkPath_writeToMemory_buffer'>returning</a> <a href='#SkPath_writeToMemory_buffer'>the</a> <a href='#SkPath_writeToMemory_buffer'>number</a> <a href='#SkPath_writeToMemory_buffer'>of</a> <a href='#SkPath_writeToMemory_buffer'>bytes</a> <a href='#SkPath_writeToMemory_buffer'>written</a>.
Pass nullptr to obtain the storage <a href='undocumented#Size'>size</a>.

Writes <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_FillType'>FillType</a>,  <a href='#Verb_Array'>verb array</a>,  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>,  <a href='#Conic_Weight'>conic weight</a>, <a href='SkPath_Reference#Conic'>and</a>
additionally writes computed information like <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_Convexity'>and</a> <a href='#SkPath_Convexity'>bounds</a>.

Use only be used in concert with <a href='#SkPath_readFromMemory'>readFromMemory</a>();
the format used for <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>in</a> <a href='SkPath_Reference#SkPath'>memory</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>not</a> <a href='SkPath_Reference#SkPath'>guaranteed</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_writeToMemory_buffer'><code><strong>buffer</strong></code></a></td>
    <td>storage for <a href='SkPath_Reference#SkPath'>SkPath</a>; <a href='SkPath_Reference#SkPath'>may</a> <a href='SkPath_Reference#SkPath'>be</a> <a href='SkPath_Reference#SkPath'>nullptr</a></td>
  </tr>
</table>

### Return Value

<a href='undocumented#Size'>size</a> <a href='undocumented#Size'>of</a> <a href='undocumented#Size'>storage</a> <a href='undocumented#Size'>required</a> <a href='undocumented#Size'>for</a> <a href='SkPath_Reference#SkPath'>SkPath</a>; <a href='SkPath_Reference#SkPath'>always</a> <a href='SkPath_Reference#SkPath'>a</a> <a href='SkPath_Reference#SkPath'>multiple</a> <a href='SkPath_Reference#SkPath'>of</a> 4

### Example

<div><fiddle-embed name="e5f16eda6a1c2d759556285f72598445">

#### Example Output

~~~~
path is equal to copy
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_serialize'>serialize</a> <a href='#SkPath_readFromMemory'>readFromMemory</a> <a href='#SkPath_dump'>dump</a> <a href='#SkPath_dumpHex'>dumpHex</a>

<a name='SkPath_serialize'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk_sp</a>&<a href='undocumented#sk_sp'>lt</a>;<a href='undocumented#SkData'>SkData</a>&<a href='undocumented#SkData'>gt</a>; <a href='#SkPath_serialize'>serialize()</a> <a href='#SkPath_serialize'>const</a>
</pre>

Writes <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>buffer</a>, <a href='SkPath_Reference#SkPath'>returning</a> <a href='SkPath_Reference#SkPath'>the</a> <a href='SkPath_Reference#SkPath'>buffer</a> <a href='SkPath_Reference#SkPath'>written</a> <a href='SkPath_Reference#SkPath'>to</a>, <a href='SkPath_Reference#SkPath'>wrapped</a> <a href='SkPath_Reference#SkPath'>in</a> <a href='undocumented#SkData'>SkData</a>.

<a href='#SkPath_serialize'>serialize()</a> <a href='#SkPath_serialize'>writes</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_FillType'>FillType</a>, <a href='#SkPath_FillType'>verb</a> <a href='#SkPath_FillType'>array</a>, <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>array</a>, <a href='SkPath_Reference#Conic'>conic</a> <a href='SkPath_Reference#Conic'>weight</a>, <a href='SkPath_Reference#Conic'>and</a>
additionally writes computed information like <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_Convexity'>and</a> <a href='#SkPath_Convexity'>bounds</a>.

<a href='#SkPath_serialize'>serialize()</a> <a href='#SkPath_serialize'>should</a> <a href='#SkPath_serialize'>only</a> <a href='#SkPath_serialize'>be</a> <a href='#SkPath_serialize'>used</a> <a href='#SkPath_serialize'>in</a> <a href='#SkPath_serialize'>concert</a> <a href='#SkPath_serialize'>with</a> <a href='#SkPath_readFromMemory'>readFromMemory</a>().
The format used for <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>in</a> <a href='SkPath_Reference#SkPath'>memory</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>not</a> <a href='SkPath_Reference#SkPath'>guaranteed</a>.

### Return Value

<a href='SkPath_Reference#SkPath'>SkPath</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>wrapped</a> <a href='undocumented#Data'>in</a> <a href='undocumented#SkData'>SkData</a> <a href='undocumented#SkData'>buffer</a>

### Example

<div><fiddle-embed name="2c6aff73608cd198659db6d1eeaaae4f">

#### Example Output

~~~~
path is equal to copy
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_writeToMemory'>writeToMemory</a> <a href='#SkPath_readFromMemory'>readFromMemory</a> <a href='#SkPath_dump'>dump</a> <a href='#SkPath_dumpHex'>dumpHex</a>

<a name='SkPath_readFromMemory'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkPath_readFromMemory'>readFromMemory</a>(<a href='#SkPath_readFromMemory'>const</a> <a href='#SkPath_readFromMemory'>void</a>* <a href='#SkPath_readFromMemory'>buffer</a>, <a href='#SkPath_readFromMemory'>size_t</a> <a href='#SkPath_readFromMemory'>length</a>)
</pre>

Initializes <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>from</a> <a href='#SkPath_readFromMemory_buffer'>buffer</a> <a href='#SkPath_readFromMemory_buffer'>of</a> <a href='undocumented#Size'>size</a> <a href='#SkPath_readFromMemory_length'>length</a>. <a href='#SkPath_readFromMemory_length'>Returns</a> <a href='#SkPath_readFromMemory_length'>zero</a> <a href='#SkPath_readFromMemory_length'>if</a> <a href='#SkPath_readFromMemory_length'>the</a> <a href='#SkPath_readFromMemory_buffer'>buffer</a> <a href='#SkPath_readFromMemory_buffer'>is</a>
<a href='undocumented#Data'>data</a> <a href='undocumented#Data'>is</a> <a href='undocumented#Data'>inconsistent</a>, <a href='undocumented#Data'>or</a> <a href='undocumented#Data'>the</a> <a href='#SkPath_readFromMemory_length'>length</a> <a href='#SkPath_readFromMemory_length'>is</a> <a href='#SkPath_readFromMemory_length'>too</a> <a href='#SkPath_readFromMemory_length'>small</a>.

Reads <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_FillType'>FillType</a>,  <a href='#Verb_Array'>verb array</a>,  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>,  <a href='#Conic_Weight'>conic weight</a>, <a href='SkPath_Reference#Conic'>and</a>
additionally reads computed information like <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_Convexity'>and</a> <a href='#SkPath_Convexity'>bounds</a>.

Used only in concert with <a href='#SkPath_writeToMemory'>writeToMemory</a>();
the format used for <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>in</a> <a href='SkPath_Reference#SkPath'>memory</a> <a href='SkPath_Reference#SkPath'>is</a> <a href='SkPath_Reference#SkPath'>not</a> <a href='SkPath_Reference#SkPath'>guaranteed</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_readFromMemory_buffer'><code><strong>buffer</strong></code></a></td>
    <td>storage for <a href='SkPath_Reference#SkPath'>SkPath</a></td>
  </tr>
  <tr>    <td><a name='SkPath_readFromMemory_length'><code><strong>length</strong></code></a></td>
    <td><a href='#SkPath_readFromMemory_buffer'>buffer</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>in</a> <a href='undocumented#Size'>bytes</a>; <a href='undocumented#Size'>must</a> <a href='undocumented#Size'>be</a> <a href='undocumented#Size'>multiple</a> <a href='undocumented#Size'>of</a> 4</td>
  </tr>
</table>

### Return Value

number of bytes read, or zero on failure

### Example

<div><fiddle-embed name="9c6edd836c573a0fd232d2b8aa11a678">

#### Example Output

~~~~
length = 32; returned by readFromMemory = 0
length = 40; returned by readFromMemory = 36
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_writeToMemory'>writeToMemory</a>

<a name='Generation_ID'></a>

<a href='#Path_Generation_ID'>Generation_ID</a> <a href='#Path_Generation_ID'>provides</a> <a href='#Path_Generation_ID'>a</a> <a href='#Path_Generation_ID'>quick</a> <a href='#Path_Generation_ID'>way</a> <a href='#Path_Generation_ID'>to</a> <a href='#Path_Generation_ID'>check</a> <a href='#Path_Generation_ID'>if</a> <a href='#Path_Verb_Array'>Verb_Array</a>, <a href='#Path_Point_Array'>Point_Array</a>, <a href='#Path_Point_Array'>or</a>
<a href='#Path_Conic_Weight'>Conic_Weight</a> <a href='#Path_Conic_Weight'>has</a> <a href='#Path_Conic_Weight'>changed</a>. <a href='#Path_Generation_ID'>Generation_ID</a> <a href='#Path_Generation_ID'>is</a> <a href='#Path_Generation_ID'>not</a> <a href='#Path_Generation_ID'>a</a> <a href='#Path_Generation_ID'>hash</a>; <a href='#Path_Generation_ID'>identical</a> <a href='SkPath_Reference#Path'>Paths</a> <a href='SkPath_Reference#Path'>will</a>
<a href='SkPath_Reference#Path'>not</a> <a href='SkPath_Reference#Path'>necessarily</a> <a href='SkPath_Reference#Path'>have</a> <a href='SkPath_Reference#Path'>matching</a> <a href='#Path_Generation_ID'>Generation_IDs</a>.

<a href='#Path_Generation_ID'>Empty</a> <a href='SkPath_Reference#Path'>Paths</a> <a href='SkPath_Reference#Path'>have</a> <a href='SkPath_Reference#Path'>a</a> <a href='#Path_Generation_ID'>Generation_ID</a> <a href='#Path_Generation_ID'>of</a> <a href='#Path_Generation_ID'>one</a>.

<a name='SkPath_getGenerationID'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkPath_getGenerationID'>getGenerationID</a>() <a href='#SkPath_getGenerationID'>const</a>
</pre>

(See Skia bug 1762.)
Returns a non-zero, globally unique value. A different value is returned
if verb array, <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>array</a>, <a href='SkPoint_Reference#SkPoint'>or</a> <a href='SkPath_Reference#Conic'>conic</a> <a href='SkPath_Reference#Conic'>weight</a> <a href='SkPath_Reference#Conic'>changes</a>.

Setting <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_FillType'>does</a> <a href='#SkPath_FillType'>not</a> <a href='#SkPath_FillType'>change</a> <a href='#SkPath_FillType'>generation</a> <a href='#SkPath_FillType'>identifier</a>.

Each time the <a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>is</a> <a href='SkPath_Reference#Path'>modified</a>, <a href='SkPath_Reference#Path'>a</a> <a href='SkPath_Reference#Path'>different</a> <a href='SkPath_Reference#Path'>generation</a> <a href='SkPath_Reference#Path'>identifier</a> <a href='SkPath_Reference#Path'>will</a> <a href='SkPath_Reference#Path'>be</a> <a href='SkPath_Reference#Path'>returned</a>.
<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_FillType'>does</a> <a href='#SkPath_FillType'>affect</a> <a href='#SkPath_FillType'>generation</a> <a href='#SkPath_FillType'>identifier</a> <a href='#SkPath_FillType'>on</a> <a href='#SkPath_FillType'>Android</a> <a href='#SkPath_FillType'>framework</a>.

### Return Value

non-zero, globally unique value

### Example

<div><fiddle-embed name="a0f166715d6479f91258d854e63e586d">

#### Example Output

~~~~
empty genID = 1
1st lineTo genID = 2
empty genID = 1
2nd lineTo genID = 3
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_equal_operator'>operator==(const SkPath& a, const SkPath& b)</a>

<a name='SkPath_isValid'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isValid'>isValid</a>() <a href='#SkPath_isValid'>const</a>
</pre>

Returns if <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>is</a> <a href='undocumented#Data'>consistent</a>. <a href='undocumented#Data'>Corrupt</a> <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>is</a> <a href='undocumented#Data'>detected</a> <a href='undocumented#Data'>if</a>
internal values are out of range or internal storage does not match
array dimensions.

### Return Value

true if <a href='SkPath_Reference#SkPath'>SkPath</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>is</a> <a href='undocumented#Data'>consistent</a>

<a name='SkPath_pathRefIsValid'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_pathRefIsValid'>pathRefIsValid</a>() <a href='#SkPath_pathRefIsValid'>const</a>
</pre>

To be deprecated soon.

<a name='SkPath_Iter'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    class <a href='#SkPath_Iter'>Iter</a> {
    <a href='#SkPath_Iter'>public</a>:
        <a href='#SkPath_Iter_Iter'>Iter()</a>;
        <a href='#SkPath_Iter'>Iter</a>(<a href='#SkPath_Iter'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>, <a href='SkPath_Reference#Path'>bool</a> <a href='SkPath_Reference#Path'>forceClose</a>);
        <a href='SkPath_Reference#Path'>void</a> <a href='#SkPath_Iter_setPath'>setPath</a>(<a href='#SkPath_Iter_setPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>, <a href='SkPath_Reference#Path'>bool</a> <a href='SkPath_Reference#Path'>forceClose</a>);
        <a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>next</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pts</a>[4], <a href='SkPoint_Reference#SkPoint'>bool</a> <a href='SkPoint_Reference#SkPoint'>doConsumeDegenerates</a> = <a href='SkPoint_Reference#SkPoint'>true</a>, <a href='SkPoint_Reference#SkPoint'>bool</a> <a href='SkPoint_Reference#SkPoint'>exact</a> = <a href='SkPoint_Reference#SkPoint'>false</a>);
        <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPath_Iter_conicWeight'>conicWeight</a>() <a href='#SkPath_Iter_conicWeight'>const</a>;
        <a href='#SkPath_Iter_conicWeight'>bool</a> <a href='#SkPath_Iter_isCloseLine'>isCloseLine</a>() <a href='#SkPath_Iter_isCloseLine'>const</a>;
        <a href='#SkPath_Iter_isCloseLine'>bool</a> <a href='#SkPath_Iter_isClosedContour'>isClosedContour</a>() <a href='#SkPath_Iter_isClosedContour'>const</a>;
    };
</pre>

Iterates through <a href='#Path_Verb_Array'>Verb_Array</a>, <a href='#Path_Verb_Array'>and</a> <a href='#Path_Verb_Array'>associated</a> <a href='#Path_Point_Array'>Point_Array</a> <a href='#Path_Point_Array'>and</a> <a href='#Path_Conic_Weight'>Conic_Weight</a>.
<a href='#Path_Conic_Weight'>Provides</a> <a href='#Path_Conic_Weight'>options</a> <a href='#Path_Conic_Weight'>to</a> <a href='#Path_Conic_Weight'>treat</a> <a href='#Path_Conic_Weight'>open</a> <a href='SkPath_Overview#Contour'>Contours</a> <a href='SkPath_Overview#Contour'>as</a> <a href='SkPath_Overview#Contour'>closed</a>, <a href='SkPath_Overview#Contour'>and</a> <a href='SkPath_Overview#Contour'>to</a> <a href='SkPath_Overview#Contour'>ignore</a>
<a href='SkPath_Overview#Contour'>degenerate</a> <a href='undocumented#Data'>data</a>.

### Example

<div><fiddle-embed name="2f53df9201769ab7e7c0e164a1334309"><div>Ignoring the actual <a href='SkPath_Reference#Verb'>Verbs</a> <a href='SkPath_Reference#Verb'>and</a> <a href='SkPath_Reference#Verb'>replacing</a> <a href='SkPath_Reference#Verb'>them</a> <a href='SkPath_Reference#Verb'>with</a> <a href='SkPath_Reference#Quad'>Quads</a> <a href='SkPath_Reference#Quad'>rounds</a> <a href='SkPath_Reference#Quad'>the</a>
<a href='SkPath_Reference#Path'>path</a> <a href='SkPath_Reference#Path'>of</a> <a href='SkPath_Reference#Path'>the</a> <a href='undocumented#Glyph'>glyph</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_RawIter'>RawIter</a>

<a name='SkPath_Iter_Iter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_Iter_Iter'>Iter()</a>
</pre>

Initializes <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Iter'>Iter</a> <a href='#SkPath_Iter'>with</a> <a href='#SkPath_Iter'>an</a> <a href='#SkPath_Iter'>empty</a> <a href='SkPath_Reference#SkPath'>SkPath</a>. <a href='#SkPath_Iter_next'>next()</a> <a href='#SkPath_Iter_next'>on</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Iter'>Iter</a> <a href='#SkPath_Iter'>returns</a>
<a href='#SkPath_kDone_Verb'>kDone_Verb</a>.
Call <a href='#SkPath_Iter_setPath'>setPath</a> <a href='#SkPath_Iter_setPath'>to</a> <a href='#SkPath_Iter_setPath'>initialize</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Iter'>Iter</a> <a href='#SkPath_Iter'>at</a> <a href='#SkPath_Iter'>a</a> <a href='#SkPath_Iter'>later</a> <a href='#SkPath_Iter'>time</a>.

### Return Value

<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Iter'>Iter</a> <a href='#SkPath_Iter'>of</a> <a href='#SkPath_Iter'>empty</a> <a href='SkPath_Reference#SkPath'>SkPath</a>

### Example

<div><fiddle-embed name="01648775cb9b354b2f1836dad82a25ab">

#### Example Output

~~~~
iter is done
iter is done
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_Iter_setPath'>setPath</a>

<a name='SkPath_Iter_const_SkPath'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_Iter'>Iter</a>(<a href='#SkPath_Iter'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>, <a href='SkPath_Reference#Path'>bool</a> <a href='SkPath_Reference#Path'>forceClose</a>)
</pre>

Sets <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Iter'>Iter</a> <a href='#SkPath_Iter'>to</a> <a href='#SkPath_Iter'>return</a> <a href='#SkPath_Iter'>elements</a> <a href='#SkPath_Iter'>of</a>  <a href='#Verb_Array'>verb array</a>,  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>, <a href='SkPoint_Reference#SkPoint'>and</a>  <a href='#Conic_Weight'>conic weight</a> <a href='SkPath_Reference#Conic'>in</a>
<a href='#SkPath_Iter_const_SkPath_path'>path</a>. <a href='#SkPath_Iter_const_SkPath_path'>If</a> <a href='#SkPath_Iter_const_SkPath_forceClose'>forceClose</a> <a href='#SkPath_Iter_const_SkPath_forceClose'>is</a> <a href='#SkPath_Iter_const_SkPath_forceClose'>true</a>, <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Iter'>Iter</a> <a href='#SkPath_Iter'>will</a> <a href='#SkPath_Iter'>add</a> <a href='#SkPath_kLine_Verb'>kLine_Verb</a> <a href='#SkPath_kLine_Verb'>and</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a> <a href='#SkPath_kClose_Verb'>after</a> <a href='#SkPath_kClose_Verb'>each</a>
open <a href='SkPath_Overview#Contour'>contour</a>. <a href='#SkPath_Iter_const_SkPath_path'>path</a> <a href='#SkPath_Iter_const_SkPath_path'>is</a> <a href='#SkPath_Iter_const_SkPath_path'>not</a> <a href='#SkPath_Iter_const_SkPath_path'>altered</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_Iter_const_SkPath_path'><code><strong>path</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>iterate</a></td>
  </tr>
  <tr>    <td><a name='SkPath_Iter_const_SkPath_forceClose'><code><strong>forceClose</strong></code></a></td>
    <td>true if open <a href='SkPath_Overview#Contour'>contours</a> <a href='SkPath_Overview#Contour'>generate</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a></td>
  </tr>
</table>

### Return Value

<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Iter'>Iter</a> <a href='#SkPath_Iter'>of</a> <a href='#SkPath_Iter_const_SkPath_path'>path</a>

### Example

<div><fiddle-embed name="13044dbf68885c0f15322c0633b633a3">

#### Example Output

~~~~
open:
kMove_Verb {0, 0},
kQuad_Verb {0, 0}, {10, 20}, {30, 40},
kDone_Verb
closed:
kMove_Verb {0, 0},
kQuad_Verb {0, 0}, {10, 20}, {30, 40},
kLine_Verb {30, 40}, {0, 0},
kClose_Verb {0, 0},
kDone_Verb
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_Iter_setPath'>setPath</a>

<a name='SkPath_Iter_setPath'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_Iter_setPath'>setPath</a>(<a href='#SkPath_Iter_setPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>, <a href='SkPath_Reference#Path'>bool</a> <a href='SkPath_Reference#Path'>forceClose</a>)
</pre>

Sets <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Iter'>Iter</a> <a href='#SkPath_Iter'>to</a> <a href='#SkPath_Iter'>return</a> <a href='#SkPath_Iter'>elements</a> <a href='#SkPath_Iter'>of</a>  <a href='#Verb_Array'>verb array</a>,  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>, <a href='SkPoint_Reference#SkPoint'>and</a>  <a href='#Conic_Weight'>conic weight</a> <a href='SkPath_Reference#Conic'>in</a>
<a href='#SkPath_Iter_setPath_path'>path</a>. <a href='#SkPath_Iter_setPath_path'>If</a> <a href='#SkPath_Iter_setPath_forceClose'>forceClose</a> <a href='#SkPath_Iter_setPath_forceClose'>is</a> <a href='#SkPath_Iter_setPath_forceClose'>true</a>, <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Iter'>Iter</a> <a href='#SkPath_Iter'>will</a> <a href='#SkPath_Iter'>add</a> <a href='#SkPath_kLine_Verb'>kLine_Verb</a> <a href='#SkPath_kLine_Verb'>and</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a> <a href='#SkPath_kClose_Verb'>after</a> <a href='#SkPath_kClose_Verb'>each</a>
open <a href='SkPath_Overview#Contour'>contour</a>. <a href='#SkPath_Iter_setPath_path'>path</a> <a href='#SkPath_Iter_setPath_path'>is</a> <a href='#SkPath_Iter_setPath_path'>not</a> <a href='#SkPath_Iter_setPath_path'>altered</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_Iter_setPath_path'><code><strong>path</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>iterate</a></td>
  </tr>
  <tr>    <td><a name='SkPath_Iter_setPath_forceClose'><code><strong>forceClose</strong></code></a></td>
    <td>true if open <a href='SkPath_Overview#Contour'>contours</a> <a href='SkPath_Overview#Contour'>generate</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6c9688008cea8937ad5cc188b38ecf16">

#### Example Output

~~~~
quad open:
kMove_Verb {0, 0},
kQuad_Verb {0, 0}, {10, 20}, {30, 40},
kDone_Verb
conic closed:
kMove_Verb {0, 0},
kConic_Verb {0, 0}, {1, 2}, {3, 4}, weight = 0.5
kLine_Verb {3, 4}, {0, 0},
kClose_Verb {0, 0},
kDone_Verb
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_Iter_const_SkPath'>Iter(const SkPath& path, bool forceClose)</a>

<a name='SkPath_Iter_next'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>next</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pts</a>[4], <a href='SkPoint_Reference#SkPoint'>bool</a> <a href='SkPoint_Reference#SkPoint'>doConsumeDegenerates</a> = <a href='SkPoint_Reference#SkPoint'>true</a>, <a href='SkPoint_Reference#SkPoint'>bool</a> <a href='SkPoint_Reference#SkPoint'>exact</a> = <a href='SkPoint_Reference#SkPoint'>false</a>)
</pre>

Returns next <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>in</a>  <a href='#Verb_Array'>verb array</a>, <a href='#SkPath_Verb'>and</a> <a href='#SkPath_Verb'>advances</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Iter'>Iter</a>.
When  <a href='#Verb_Array'>verb array</a> is exhausted, returns <a href='#SkPath_kDone_Verb'>kDone_Verb</a>.

Zero to four <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>are</a> <a href='SkPoint_Reference#SkPoint'>stored</a> <a href='SkPoint_Reference#SkPoint'>in</a> <a href='#SkPath_Iter_next_pts'>pts</a>, <a href='#SkPath_Iter_next_pts'>depending</a> <a href='#SkPath_Iter_next_pts'>on</a> <a href='#SkPath_Iter_next_pts'>the</a> <a href='#SkPath_Iter_next_pts'>returned</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a>.

If <a href='#SkPath_Iter_next_doConsumeDegenerates'>doConsumeDegenerates</a> <a href='#SkPath_Iter_next_doConsumeDegenerates'>is</a> <a href='#SkPath_Iter_next_doConsumeDegenerates'>true</a>, <a href='#SkPath_Iter_next_doConsumeDegenerates'>skip</a> <a href='#SkPath_Iter_next_doConsumeDegenerates'>consecutive</a> <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>entries</a>, <a href='#SkPath_kMove_Verb'>returning</a>
only the last in the series; and skip very small <a href='undocumented#Line'>lines</a>, <a href='SkPath_Reference#Quad'>quads</a>, <a href='SkPath_Reference#Quad'>and</a> <a href='SkPath_Reference#Conic'>conics</a>; <a href='SkPath_Reference#Conic'>and</a>
skip <a href='#SkPath_kClose_Verb'>kClose_Verb</a> <a href='#SkPath_kClose_Verb'>following</a> <a href='#SkPath_kMove_Verb'>kMove_Verb</a>.
if <a href='#SkPath_Iter_next_doConsumeDegenerates'>doConsumeDegenerates</a> <a href='#SkPath_Iter_next_doConsumeDegenerates'>is</a> <a href='#SkPath_Iter_next_doConsumeDegenerates'>true</a> <a href='#SkPath_Iter_next_doConsumeDegenerates'>and</a> <a href='#SkPath_Iter_next_exact'>exact</a> <a href='#SkPath_Iter_next_exact'>is</a> <a href='#SkPath_Iter_next_exact'>true</a>, <a href='#SkPath_Iter_next_exact'>only</a> <a href='#SkPath_Iter_next_exact'>skip</a> <a href='undocumented#Line'>lines</a>, <a href='SkPath_Reference#Quad'>quads</a>, <a href='SkPath_Reference#Quad'>and</a>
<a href='SkPath_Reference#Conic'>conics</a> <a href='SkPath_Reference#Conic'>with</a> <a href='SkPath_Reference#Conic'>zero</a> <a href='SkPath_Reference#Conic'>lengths</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_Iter_next_pts'><code><strong>pts</strong></code></a></td>
    <td>storage for <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>describing</a> <a href='undocumented#Data'>returned</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a></td>
  </tr>
  <tr>    <td><a name='SkPath_Iter_next_doConsumeDegenerates'><code><strong>doConsumeDegenerates</strong></code></a></td>
    <td>if true, skip degenerate <a href='SkPath_Reference#Verb'>verbs</a></td>
  </tr>
  <tr>    <td><a name='SkPath_Iter_next_exact'><code><strong>exact</strong></code></a></td>
    <td>skip  zero length <a href='undocumented#Curve'>curves</a></td>
  </tr>
</table>

### Return Value

next <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>from</a>  <a href='#Verb_Array'>verb array</a>

### Example

<div><fiddle-embed name="00ae8984856486bdb626d0ed6587855a"><div>skip degenerate skips the first in a <a href='#SkPath_kMove_Verb'>kMove_Verb</a> <a href='#SkPath_kMove_Verb'>pair</a>, <a href='#SkPath_kMove_Verb'>the</a> <a href='#SkPath_kMove_Verb'>kMove_Verb</a>
<a href='#SkPath_kMove_Verb'>followed</a> <a href='#SkPath_kMove_Verb'>by</a> <a href='#SkPath_kMove_Verb'>the</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a>, <a href='#SkPath_kClose_Verb'>the</a>  <a href='#SkPath_kClose_Verb'>zero length</a> <a href='undocumented#Line'>Line</a> <a href='undocumented#Line'>and</a> <a href='undocumented#Line'>the</a> <a href='undocumented#Line'>very</a> <a href='undocumented#Line'>small</a> <a href='undocumented#Line'>Line</a>.

<a href='undocumented#Line'>skip</a> <a href='undocumented#Line'>degenerate</a> <a href='undocumented#Line'>if</a> <a href='#SkPath_Iter_next_exact'>exact</a> <a href='#SkPath_Iter_next_exact'>skips</a> <a href='#SkPath_Iter_next_exact'>the</a> <a href='#SkPath_Iter_next_exact'>same</a> <a href='#SkPath_Iter_next_exact'>as</a> <a href='#SkPath_Iter_next_exact'>skip</a> <a href='#SkPath_Iter_next_exact'>degenerate</a>, <a href='#SkPath_Iter_next_exact'>but</a> <a href='#SkPath_Iter_next_exact'>shows</a>
<a href='#SkPath_Iter_next_exact'>the</a> <a href='#SkPath_Iter_next_exact'>very</a> <a href='#SkPath_Iter_next_exact'>small</a> <a href='undocumented#Line'>Line</a>.

<a href='undocumented#Line'>skip</a> <a href='undocumented#Line'>none</a> <a href='undocumented#Line'>shows</a> <a href='undocumented#Line'>all</a> <a href='undocumented#Line'>of</a> <a href='undocumented#Line'>the</a> <a href='SkPath_Reference#Verb'>Verbs</a> <a href='SkPath_Reference#Verb'>and</a> <a href='SkPoint_Reference#Point'>Points</a> <a href='SkPoint_Reference#Point'>in</a> <a href='SkPath_Reference#Path'>Path</a>.
</div>

#### Example Output

~~~~
skip degenerate:
kMove_Verb {20, 20},
kQuad_Verb {20, 20}, {10, 20}, {30, 40},
kDone_Verb
skip degenerate if exact:
kMove_Verb {20, 20},
kQuad_Verb {20, 20}, {10, 20}, {30, 40},
kMove_Verb {30, 30},
kLine_Verb {30, 30}, {30.00001, 30},
kDone_Verb
skip none:
kMove_Verb {10, 10},
kMove_Verb {20, 20},
kQuad_Verb {20, 20}, {10, 20}, {30, 40},
kMove_Verb {1, 1},
kClose_Verb {1, 1},
kMove_Verb {30, 30},
kLine_Verb {30, 30}, {30, 30},
kMove_Verb {30, 30},
kLine_Verb {30, 30}, {30.00001, 30},
kDone_Verb
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_IsLineDegenerate'>IsLineDegenerate</a> <a href='#SkPath_IsCubicDegenerate'>IsCubicDegenerate</a> <a href='#SkPath_IsQuadDegenerate'>IsQuadDegenerate</a>

<a name='SkPath_Iter_conicWeight'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPath_Iter_conicWeight'>conicWeight</a>() <a href='#SkPath_Iter_conicWeight'>const</a>
</pre>

Returns <a href='SkPath_Reference#Conic'>conic</a> <a href='SkPath_Reference#Conic'>weight</a> <a href='SkPath_Reference#Conic'>if</a> <a href='#SkPath_Iter_next'>next()</a> <a href='#SkPath_Iter_next'>returned</a> <a href='#SkPath_kConic_Verb'>kConic_Verb</a>.

If <a href='#SkPath_Iter_next'>next()</a> <a href='#SkPath_Iter_next'>has</a> <a href='#SkPath_Iter_next'>not</a> <a href='#SkPath_Iter_next'>been</a> <a href='#SkPath_Iter_next'>called</a>, <a href='#SkPath_Iter_next'>or</a> <a href='#SkPath_Iter_next'>next()</a> <a href='#SkPath_Iter_next'>did</a> <a href='#SkPath_Iter_next'>not</a> <a href='#SkPath_Iter_next'>return</a> <a href='#SkPath_kConic_Verb'>kConic_Verb</a>,
result is undefined.

### Return Value

<a href='SkPath_Reference#Conic'>conic</a> <a href='SkPath_Reference#Conic'>weight</a> <a href='SkPath_Reference#Conic'>for</a> <a href='SkPath_Reference#Conic'>conic</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>returned</a> <a href='SkPoint_Reference#SkPoint'>by</a> <a href='#SkPath_Iter_next'>next()</a>

### Example

<div><fiddle-embed name="7cdea37741d50f0594c6244eb07fd175">

#### Example Output

~~~~
first verb is move
next verb is conic
conic points: {0,0}, {1,2}, {3,4}
conic weight: 0.5
~~~~

</fiddle-embed></div>

### See Also

<a href='#Path_Conic_Weight'>Conic_Weight</a>

<a name='SkPath_Iter_isCloseLine'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_Iter_isCloseLine'>isCloseLine</a>() <a href='#SkPath_Iter_isCloseLine'>const</a>
</pre>

Returns true if last <a href='#SkPath_kLine_Verb'>kLine_Verb</a> <a href='#SkPath_kLine_Verb'>returned</a> <a href='#SkPath_kLine_Verb'>by</a> <a href='#SkPath_Iter_next'>next()</a> <a href='#SkPath_Iter_next'>was</a> <a href='#SkPath_Iter_next'>generated</a>
by <a href='#SkPath_kClose_Verb'>kClose_Verb</a>. <a href='#SkPath_kClose_Verb'>When</a> <a href='#SkPath_kClose_Verb'>true</a>, <a href='#SkPath_kClose_Verb'>the</a> <a href='#SkPath_kClose_Verb'>end</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>returned</a> <a href='SkPoint_Reference#Point'>by</a> <a href='#SkPath_Iter_next'>next()</a> <a href='#SkPath_Iter_next'>is</a>
also the start <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>of</a> <a href='SkPath_Overview#Contour'>contour</a>.

If <a href='#SkPath_Iter_next'>next()</a> <a href='#SkPath_Iter_next'>has</a> <a href='#SkPath_Iter_next'>not</a> <a href='#SkPath_Iter_next'>been</a> <a href='#SkPath_Iter_next'>called</a>, <a href='#SkPath_Iter_next'>or</a> <a href='#SkPath_Iter_next'>next()</a> <a href='#SkPath_Iter_next'>did</a> <a href='#SkPath_Iter_next'>not</a> <a href='#SkPath_Iter_next'>return</a> <a href='#SkPath_kLine_Verb'>kLine_Verb</a>,
result is undefined.

### Return Value

true if last <a href='#SkPath_kLine_Verb'>kLine_Verb</a> <a href='#SkPath_kLine_Verb'>was</a> <a href='#SkPath_kLine_Verb'>generated</a> <a href='#SkPath_kLine_Verb'>by</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a>

### Example

<div><fiddle-embed name="7000b501f49341629bfdd9f80e686103">

#### Example Output

~~~~
1st verb is move
moveTo point: {6,7}
2nd verb is conic
3rd verb is line
line points: {3,4}, {6,7}
line generated by close
4th verb is close
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_close'>close()</a>

<a name='SkPath_Iter_isClosedContour'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_Iter_isClosedContour'>isClosedContour</a>() <a href='#SkPath_Iter_isClosedContour'>const</a>
</pre>

Returns true if subsequent calls to <a href='#SkPath_Iter_next'>next()</a> <a href='#SkPath_Iter_next'>return</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a> <a href='#SkPath_kClose_Verb'>before</a> <a href='#SkPath_kClose_Verb'>returning</a>
<a href='#SkPath_kMove_Verb'>kMove_Verb</a>. <a href='#SkPath_kMove_Verb'>if</a> <a href='#SkPath_kMove_Verb'>true</a>, <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Iter'>Iter</a> <a href='#SkPath_Iter'>is</a> <a href='#SkPath_Iter'>processing</a> <a href='#SkPath_Iter'>may</a> <a href='#SkPath_Iter'>end</a> <a href='#SkPath_Iter'>with</a> <a href='#SkPath_kClose_Verb'>kClose_Verb</a>, <a href='#SkPath_kClose_Verb'>or</a>
<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Iter'>Iter</a> <a href='#SkPath_Iter'>may</a> <a href='#SkPath_Iter'>have</a> <a href='#SkPath_Iter'>been</a> <a href='#SkPath_Iter'>initialized</a> <a href='#SkPath_Iter'>with</a> <a href='#SkPath_Iter'>force</a> <a href='#SkPath_Iter'>close</a> <a href='#SkPath_Iter'>set</a> <a href='#SkPath_Iter'>to</a> <a href='#SkPath_Iter'>true</a>.

### Return Value

true if <a href='SkPath_Overview#Contour'>contour</a> <a href='SkPath_Overview#Contour'>is</a> <a href='SkPath_Overview#Contour'>closed</a>

### Example

<div><fiddle-embed name="b0d48a6e949db1cb545216ae9c3c3c70">

#### Example Output

~~~~
without close(), forceClose is false: isClosedContour returns false
with close(),    forceClose is false: isClosedContour returns true
without close(), forceClose is true : isClosedContour returns true
with close(),    forceClose is true : isClosedContour returns true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_Iter_const_SkPath'>Iter(const SkPath& path, bool forceClose)</a>

<a name='SkPath_RawIter'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    class <a href='#SkPath_RawIter'>RawIter</a> {
    <a href='#SkPath_RawIter'>public</a>:
        <a href='#SkPath_RawIter_RawIter'>RawIter()</a>;
        <a href='#SkPath_RawIter'>RawIter</a>(<a href='#SkPath_RawIter'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>);
        <a href='SkPath_Reference#Path'>void</a> <a href='#SkPath_RawIter_setPath'>setPath</a>(<a href='#SkPath_RawIter_setPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>);
        <a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>next</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pts</a>[4]);
        <a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_RawIter_peek'>peek()</a> <a href='#SkPath_RawIter_peek'>const</a>;
        <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPath_RawIter_conicWeight'>conicWeight</a>() <a href='#SkPath_RawIter_conicWeight'>const</a>;
    };
</pre>

Iterates through <a href='#Path_Verb_Array'>Verb_Array</a>, <a href='#Path_Verb_Array'>and</a> <a href='#Path_Verb_Array'>associated</a> <a href='#Path_Point_Array'>Point_Array</a> <a href='#Path_Point_Array'>and</a> <a href='#Path_Conic_Weight'>Conic_Weight</a>.
<a href='#Path_Verb_Array'>Verb_Array</a>, <a href='#Path_Point_Array'>Point_Array</a>, <a href='#Path_Point_Array'>and</a> <a href='#Path_Conic_Weight'>Conic_Weight</a> <a href='#Path_Conic_Weight'>are</a> <a href='#Path_Conic_Weight'>returned</a> <a href='#Path_Conic_Weight'>unaltered</a>.

<a name='SkPath_RawIter_RawIter'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_RawIter_RawIter'>RawIter()</a>
</pre>

Initializes <a href='#SkPath_RawIter'>RawIter</a> <a href='#SkPath_RawIter'>with</a> <a href='#SkPath_RawIter'>an</a> <a href='#SkPath_RawIter'>empty</a> <a href='SkPath_Reference#SkPath'>SkPath</a>. <a href='#SkPath_RawIter_next'>next()</a> <a href='#SkPath_RawIter_next'>on</a> <a href='#SkPath_RawIter'>RawIter</a> <a href='#SkPath_RawIter'>returns</a> <a href='#SkPath_kDone_Verb'>kDone_Verb</a>.
Call <a href='#SkPath_RawIter_setPath'>setPath</a> <a href='#SkPath_RawIter_setPath'>to</a> <a href='#SkPath_RawIter_setPath'>initialize</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Iter'>Iter</a> <a href='#SkPath_Iter'>at</a> <a href='#SkPath_Iter'>a</a> <a href='#SkPath_Iter'>later</a> <a href='#SkPath_Iter'>time</a>.

### Return Value

<a href='#SkPath_RawIter'>RawIter</a> <a href='#SkPath_RawIter'>of</a> <a href='#SkPath_RawIter'>empty</a> <a href='SkPath_Reference#SkPath'>SkPath</a>

<a name='SkPath_RawIter_copy_const_SkPath'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_RawIter'>RawIter</a>(<a href='#SkPath_RawIter'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>)
</pre>

Sets <a href='#SkPath_RawIter'>RawIter</a> <a href='#SkPath_RawIter'>to</a> <a href='#SkPath_RawIter'>return</a> <a href='#SkPath_RawIter'>elements</a> <a href='#SkPath_RawIter'>of</a>  <a href='#Verb_Array'>verb array</a>,  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>, <a href='SkPoint_Reference#SkPoint'>and</a>  <a href='#Conic_Weight'>conic weight</a> <a href='SkPath_Reference#Conic'>in</a> <a href='#SkPath_RawIter_copy_const_SkPath_path'>path</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_RawIter_copy_const_SkPath_path'><code><strong>path</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>iterate</a></td>
  </tr>
</table>

### Return Value

<a href='#SkPath_RawIter'>RawIter</a> <a href='#SkPath_RawIter'>of</a> <a href='#SkPath_RawIter_copy_const_SkPath_path'>path</a>

<a name='SkPath_RawIter_setPath'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_RawIter_setPath'>setPath</a>(<a href='#SkPath_RawIter_setPath'>const</a> <a href='SkPath_Reference#SkPath'>SkPath</a>& <a href='SkPath_Reference#Path'>path</a>)
</pre>

Sets <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Iter'>Iter</a> <a href='#SkPath_Iter'>to</a> <a href='#SkPath_Iter'>return</a> <a href='#SkPath_Iter'>elements</a> <a href='#SkPath_Iter'>of</a>  <a href='#Verb_Array'>verb array</a>,  <a href='SkPath_Reference#Point_Array'>SkPoint array</a>, <a href='SkPoint_Reference#SkPoint'>and</a>  <a href='#Conic_Weight'>conic weight</a> <a href='SkPath_Reference#Conic'>in</a>
<a href='#SkPath_RawIter_setPath_path'>path</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_RawIter_setPath_path'><code><strong>path</strong></code></a></td>
    <td><a href='SkPath_Reference#SkPath'>SkPath</a> <a href='SkPath_Reference#SkPath'>to</a> <a href='SkPath_Reference#SkPath'>iterate</a></td>
  </tr>
</table>

<a name='SkPath_RawIter_next'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>next</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pts</a>[4])
</pre>

Returns next <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>in</a>  <a href='#Verb_Array'>verb array</a>, <a href='#SkPath_Verb'>and</a> <a href='#SkPath_Verb'>advances</a> <a href='#SkPath_RawIter'>RawIter</a>.
When  <a href='#Verb_Array'>verb array</a> is exhausted, returns <a href='#SkPath_kDone_Verb'>kDone_Verb</a>.
Zero to four <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>are</a> <a href='SkPoint_Reference#SkPoint'>stored</a> <a href='SkPoint_Reference#SkPoint'>in</a> <a href='#SkPath_RawIter_next_pts'>pts</a>, <a href='#SkPath_RawIter_next_pts'>depending</a> <a href='#SkPath_RawIter_next_pts'>on</a> <a href='#SkPath_RawIter_next_pts'>the</a> <a href='#SkPath_RawIter_next_pts'>returned</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_RawIter_next_pts'><code><strong>pts</strong></code></a></td>
    <td>storage for <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>describing</a> <a href='undocumented#Data'>returned</a> <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a></td>
  </tr>
</table>

### Return Value

next <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>from</a>  <a href='#Verb_Array'>verb array</a>

### Example

<div><fiddle-embed name="944a80c7ff8c04e1fecc4aec4a47ea60">

#### Example Output

~~~~
kMove_Verb {50, 60},
kQuad_Verb {50, 60}, {10, 20}, {30, 40},
kClose_Verb {50, 60},
kMove_Verb {50, 60},
kLine_Verb {50, 60}, {30, 30},
kConic_Verb {30, 30}, {1, 2}, {3, 4}, weight = 0.5
kCubic_Verb {3, 4}, {-1, -2}, {-3, -4}, {-5, -6},
kDone_Verb
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_RawIter_peek'>peek()</a>

<a name='SkPath_RawIter_peek'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_RawIter_peek'>peek()</a> <a href='#SkPath_RawIter_peek'>const</a>
</pre>

Returns next <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a>, <a href='#SkPath_Verb'>but</a> <a href='#SkPath_Verb'>does</a> <a href='#SkPath_Verb'>not</a> <a href='#SkPath_Verb'>advance</a> <a href='#SkPath_RawIter'>RawIter</a>.

### Return Value

next <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Verb'>from</a> <a href='#SkPath_Verb'>verb</a> <a href='#SkPath_Verb'>array</a>

### Example

<div><fiddle-embed name="eb5fa5bea23059ce538e883502f828f5">

#### Example Output

~~~~
#Volatile
peek Move == verb Move
peek Quad == verb Quad
peek Conic == verb Conic
peek Cubic == verb Cubic
peek Done == verb Done
peek Done == verb Done
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_RawIter_next'>next</a>

<a name='SkPath_RawIter_conicWeight'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPath_RawIter_conicWeight'>conicWeight</a>() <a href='#SkPath_RawIter_conicWeight'>const</a>
</pre>

Returns <a href='SkPath_Reference#Conic'>conic</a> <a href='SkPath_Reference#Conic'>weight</a> <a href='SkPath_Reference#Conic'>if</a> <a href='#SkPath_RawIter_next'>next()</a> <a href='#SkPath_RawIter_next'>returned</a> <a href='#SkPath_kConic_Verb'>kConic_Verb</a>.

If <a href='#SkPath_RawIter_next'>next()</a> <a href='#SkPath_RawIter_next'>has</a> <a href='#SkPath_RawIter_next'>not</a> <a href='#SkPath_RawIter_next'>been</a> <a href='#SkPath_RawIter_next'>called</a>, <a href='#SkPath_RawIter_next'>or</a> <a href='#SkPath_RawIter_next'>next()</a> <a href='#SkPath_RawIter_next'>did</a> <a href='#SkPath_RawIter_next'>not</a> <a href='#SkPath_RawIter_next'>return</a> <a href='#SkPath_kConic_Verb'>kConic_Verb</a>,
result is undefined.

### Return Value

<a href='SkPath_Reference#Conic'>conic</a> <a href='SkPath_Reference#Conic'>weight</a> <a href='SkPath_Reference#Conic'>for</a> <a href='SkPath_Reference#Conic'>conic</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>returned</a> <a href='SkPoint_Reference#SkPoint'>by</a> <a href='#SkPath_RawIter_next'>next()</a>

### Example

<div><fiddle-embed name="69f360a0ba8f40c51ef4cd9f972c5893">

#### Example Output

~~~~
first verb is move
next verb is conic
conic points: {0,0}, {1,2}, {3,4}
conic weight: 0.5
~~~~

</fiddle-embed></div>

### See Also

<a href='#Path_Conic_Weight'>Conic_Weight</a>

