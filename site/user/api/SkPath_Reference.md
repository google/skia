SkPath Reference
===

<a name='SkPath'></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='#SkPath'>SkPath</a> {
public:
    enum <a href='#SkPath_Direction'>Direction</a> {
        <a href='#SkPath_kCW_Direction'>kCW_Direction</a>,
        <a href='#SkPath_kCCW_Direction'>kCCW_Direction</a>,
    };

    <a href='#SkPath_empty_constructor'>SkPath()</a>;
    <a href='#SkPath_copy_const_SkPath'>SkPath(const SkPath& path)</a>;
    <a href='#SkPath_destructor'>~SkPath()</a>;
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_copy_operator'>operator=(const SkPath& path)</a>;
    friend bool <a href='#SkPath_equal_operator'>operator==(const SkPath& a, const SkPath& b)</a>;
    friend bool <a href='#SkPath_notequal_operator'>operator!=(const SkPath& a, const SkPath& b)</a>;
    bool <a href='#SkPath_isInterpolatable'>isInterpolatable</a>(const <a href='#SkPath'>SkPath</a>& compare) const;
    bool <a href='#SkPath_interpolate'>interpolate</a>(const <a href='#SkPath'>SkPath</a>& ending, <a href='undocumented#SkScalar'>SkScalar</a> weight, <a href='#SkPath'>SkPath</a>* out) const;
    bool <a href='#SkPath_unique'>unique</a>() const;

    enum <a href='#SkPath_FillType'>FillType</a> {
        <a href='#SkPath_kWinding_FillType'>kWinding_FillType</a>,
        <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd_FillType</a>,
        <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding_FillType</a>,
        <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd_FillType</a>,
    };

    <a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_getFillType'>getFillType</a>() const;
    void <a href='#SkPath_setFillType'>setFillType</a>(<a href='#SkPath_FillType'>FillType</a> ft);
    bool <a href='#SkPath_isInverseFillType'>isInverseFillType</a>() const;
    void <a href='#SkPath_toggleInverseFillType'>toggleInverseFillType</a>();

    enum <a href='#SkPath_Convexity'>Convexity</a> : uint8_t {
        <a href='#SkPath_kUnknown_Convexity'>kUnknown_Convexity</a>,
        <a href='#SkPath_kConvex_Convexity'>kConvex_Convexity</a>,
        <a href='#SkPath_kConcave_Convexity'>kConcave_Convexity</a>,
    };

    <a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_getConvexity'>getConvexity</a>() const;
    <a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_getConvexityOrUnknown'>getConvexityOrUnknown</a>() const;
    void <a href='#SkPath_setConvexity'>setConvexity</a>(<a href='#SkPath_Convexity'>Convexity</a> convexity);
    bool <a href='#SkPath_isConvex'>isConvex</a>() const;
    bool <a href='#SkPath_isOval'>isOval</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* bounds) const;
    bool <a href='#SkPath_isRRect'>isRRect</a>(<a href='SkRRect_Reference#SkRRect'>SkRRect</a>* rrect) const;
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_reset'>reset</a>();
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_rewind'>rewind</a>();
    bool <a href='#SkPath_isEmpty'>isEmpty</a>() const;
    bool <a href='#SkPath_isLastContourClosed'>isLastContourClosed</a>() const;
    bool <a href='#SkPath_isFinite'>isFinite</a>() const;
    bool <a href='#SkPath_isVolatile'>isVolatile</a>() const;
    void <a href='#SkPath_setIsVolatile'>setIsVolatile</a>(bool isVolatile);
    static bool <a href='#SkPath_IsLineDegenerate'>IsLineDegenerate</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p1, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p2, bool exact);
    static bool <a href='#SkPath_IsQuadDegenerate'>IsQuadDegenerate</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p1, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p2,
                                 const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p3, bool exact);
    static bool <a href='#SkPath_IsCubicDegenerate'>IsCubicDegenerate</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p1, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p2,
                                  const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p3, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p4, bool exact);
    bool <a href='#SkPath_isLine'>isLine</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> line[2]) const;
    int <a href='#SkPath_countPoints'>countPoints</a>() const;
    <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkPath_getPoint'>getPoint</a>(int index) const;
    int <a href='#SkPath_getPoints'>getPoints</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> points[], int max) const;
    int <a href='#SkPath_countVerbs'>countVerbs</a>() const;
    int <a href='#SkPath_getVerbs'>getVerbs</a>(uint8_t verbs[], int max) const;
    void <a href='#SkPath_swap'>swap</a>(<a href='#SkPath'>SkPath</a>& other);
    const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkPath_getBounds'>getBounds</a>() const;
    void <a href='#SkPath_updateBoundsCache'>updateBoundsCache</a>() const;
    <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkPath_computeTightBounds'>computeTightBounds</a>() const;
    bool <a href='#SkPath_conservativelyContainsRect'>conservativelyContainsRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& rect) const;
    void <a href='#SkPath_incReserve'>incReserve</a>(unsigned extraPtCount);
    void <a href='#SkPath_shrinkToFit'>shrinkToFit</a>();
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_moveTo'>moveTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_moveTo_2'>moveTo</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_rMoveTo'>rMoveTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_lineTo'>lineTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_lineTo_2'>lineTo</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_rLineTo'>rLineTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_quadTo'>quadTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> x1, <a href='undocumented#SkScalar'>SkScalar</a> y1, <a href='undocumented#SkScalar'>SkScalar</a> x2, <a href='undocumented#SkScalar'>SkScalar</a> y2);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_quadTo_2'>quadTo</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p1, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p2);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_rQuadTo'>rQuadTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx1, <a href='undocumented#SkScalar'>SkScalar</a> dy1, <a href='undocumented#SkScalar'>SkScalar</a> dx2, <a href='undocumented#SkScalar'>SkScalar</a> dy2);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_conicTo'>conicTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> x1, <a href='undocumented#SkScalar'>SkScalar</a> y1, <a href='undocumented#SkScalar'>SkScalar</a> x2, <a href='undocumented#SkScalar'>SkScalar</a> y2,
                    <a href='undocumented#SkScalar'>SkScalar</a> w);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_conicTo_2'>conicTo</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p1, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p2, <a href='undocumented#SkScalar'>SkScalar</a> w);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_rConicTo'>rConicTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx1, <a href='undocumented#SkScalar'>SkScalar</a> dy1, <a href='undocumented#SkScalar'>SkScalar</a> dx2, <a href='undocumented#SkScalar'>SkScalar</a> dy2,
                     <a href='undocumented#SkScalar'>SkScalar</a> w);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_cubicTo'>cubicTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> x1, <a href='undocumented#SkScalar'>SkScalar</a> y1, <a href='undocumented#SkScalar'>SkScalar</a> x2, <a href='undocumented#SkScalar'>SkScalar</a> y2,
                    <a href='undocumented#SkScalar'>SkScalar</a> x3, <a href='undocumented#SkScalar'>SkScalar</a> y3);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_cubicTo_2'>cubicTo</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p1, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p2, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p3);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_rCubicTo'>rCubicTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> x1, <a href='undocumented#SkScalar'>SkScalar</a> y1, <a href='undocumented#SkScalar'>SkScalar</a> x2, <a href='undocumented#SkScalar'>SkScalar</a> y2,
                     <a href='undocumented#SkScalar'>SkScalar</a> x3, <a href='undocumented#SkScalar'>SkScalar</a> y3);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_arcTo'>arcTo</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& oval, <a href='undocumented#SkScalar'>SkScalar</a> startAngle, <a href='undocumented#SkScalar'>SkScalar</a> sweepAngle, bool forceMoveTo);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_arcTo_2'>arcTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> x1, <a href='undocumented#SkScalar'>SkScalar</a> y1, <a href='undocumented#SkScalar'>SkScalar</a> x2, <a href='undocumented#SkScalar'>SkScalar</a> y2, <a href='undocumented#SkScalar'>SkScalar</a> radius);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_arcTo_3'>arcTo</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> p1, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> p2, <a href='undocumented#SkScalar'>SkScalar</a> radius);

    enum <a href='#SkPath_ArcSize'>ArcSize</a> {
        <a href='#SkPath_kSmall_ArcSize'>kSmall_ArcSize</a>,
        <a href='#SkPath_kLarge_ArcSize'>kLarge_ArcSize</a>,
    };

    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_arcTo_4'>arcTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> rx, <a href='undocumented#SkScalar'>SkScalar</a> ry, <a href='undocumented#SkScalar'>SkScalar</a> xAxisRotate, <a href='#SkPath_ArcSize'>ArcSize</a> largeArc,
                  <a href='#SkPath_Direction'>Direction</a> sweep, <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_arcTo_5'>arcTo</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> r, <a href='undocumented#SkScalar'>SkScalar</a> xAxisRotate, <a href='#SkPath_ArcSize'>ArcSize</a> largeArc, <a href='#SkPath_Direction'>Direction</a> sweep,
               const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> xy);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_rArcTo'>rArcTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> rx, <a href='undocumented#SkScalar'>SkScalar</a> ry, <a href='undocumented#SkScalar'>SkScalar</a> xAxisRotate, <a href='#SkPath_ArcSize'>ArcSize</a> largeArc,
                   <a href='#SkPath_Direction'>Direction</a> sweep, <a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_close'>close</a>();
    static bool <a href='#SkPath_IsInverseFillType'>IsInverseFillType</a>(<a href='#SkPath_FillType'>FillType</a> fill);
    static <a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_ConvertToNonInverseFillType'>ConvertToNonInverseFillType</a>(<a href='#SkPath_FillType'>FillType</a> fill);
    static int <a href='#SkPath_ConvertConicToQuads'>ConvertConicToQuads</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p0, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p1, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p2,
                                   <a href='undocumented#SkScalar'>SkScalar</a> w, <a href='SkPoint_Reference#SkPoint'>SkPoint</a> pts[], int pow2);
    bool <a href='#SkPath_isRect'>isRect</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* rect, bool* isClosed = nullptr, <a href='#SkPath_Direction'>Direction</a>* direction = nullptr) const;
    bool <a href='#SkPath_isNestedFillRects'>isNestedFillRects</a>(<a href='SkRect_Reference#SkRect'>SkRect</a> rect[2], <a href='#SkPath_Direction'>Direction</a> dirs[2] = nullptr) const;
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_addRect'>addRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& rect, <a href='#SkPath_Direction'>Direction</a> dir = <a href='#SkPath_kCW_Direction'>kCW_Direction</a>);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_addRect_2'>addRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& rect, <a href='#SkPath_Direction'>Direction</a> dir, unsigned start);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_addRect_3'>addRect</a>(<a href='undocumented#SkScalar'>SkScalar</a> left, <a href='undocumented#SkScalar'>SkScalar</a> top, <a href='undocumented#SkScalar'>SkScalar</a> right, <a href='undocumented#SkScalar'>SkScalar</a> bottom,
                    <a href='#SkPath_Direction'>Direction</a> dir = <a href='#SkPath_kCW_Direction'>kCW_Direction</a>);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_addOval'>addOval</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& oval, <a href='#SkPath_Direction'>Direction</a> dir = <a href='#SkPath_kCW_Direction'>kCW_Direction</a>);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_addOval_2'>addOval</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& oval, <a href='#SkPath_Direction'>Direction</a> dir, unsigned start);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_addCircle'>addCircle</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, <a href='undocumented#SkScalar'>SkScalar</a> radius,
                      <a href='#SkPath_Direction'>Direction</a> dir = <a href='#SkPath_kCW_Direction'>kCW_Direction</a>);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_addArc'>addArc</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& oval, <a href='undocumented#SkScalar'>SkScalar</a> startAngle, <a href='undocumented#SkScalar'>SkScalar</a> sweepAngle);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_addRoundRect'>addRoundRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& rect, <a href='undocumented#SkScalar'>SkScalar</a> rx, <a href='undocumented#SkScalar'>SkScalar</a> ry,
                         <a href='#SkPath_Direction'>Direction</a> dir = <a href='#SkPath_kCW_Direction'>kCW_Direction</a>);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_addRoundRect_2'>addRoundRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& rect, const <a href='undocumented#SkScalar'>SkScalar</a> radii[],
                         <a href='#SkPath_Direction'>Direction</a> dir = <a href='#SkPath_kCW_Direction'>kCW_Direction</a>);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_addRRect'>addRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& rrect, <a href='#SkPath_Direction'>Direction</a> dir = <a href='#SkPath_kCW_Direction'>kCW_Direction</a>);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_addRRect_2'>addRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& rrect, <a href='#SkPath_Direction'>Direction</a> dir, unsigned start);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_addPoly'>addPoly</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> pts[], int count, bool close);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_addPoly_2'>addPoly</a>(const std::initializer_list<<a href='SkPoint_Reference#SkPoint'>SkPoint</a>>& list, bool close);

    enum <a href='#SkPath_AddPathMode'>AddPathMode</a> {
        <a href='#SkPath_kAppend_AddPathMode'>kAppend_AddPathMode</a>,
        <a href='#SkPath_kExtend_AddPathMode'>kExtend_AddPathMode</a>,
    };

    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_addPath'>addPath</a>(const <a href='#SkPath'>SkPath</a>& src, <a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy,
                    <a href='#SkPath_AddPathMode'>AddPathMode</a> mode = <a href='#SkPath_kAppend_AddPathMode'>kAppend_AddPathMode</a>);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_addPath_2'>addPath</a>(const <a href='#SkPath'>SkPath</a>& src, <a href='#SkPath_AddPathMode'>AddPathMode</a> mode = <a href='#SkPath_kAppend_AddPathMode'>kAppend_AddPathMode</a>);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_addPath_3'>addPath</a>(const <a href='#SkPath'>SkPath</a>& src, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& matrix,
                    <a href='#SkPath_AddPathMode'>AddPathMode</a> mode = <a href='#SkPath_kAppend_AddPathMode'>kAppend_AddPathMode</a>);
    <a href='#SkPath'>SkPath</a>& <a href='#SkPath_reverseAddPath'>reverseAddPath</a>(const <a href='#SkPath'>SkPath</a>& src);
    void <a href='#SkPath_offset'>offset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy, <a href='#SkPath'>SkPath</a>* dst) const;
    void <a href='#SkPath_offset_2'>offset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy);
    void <a href='#SkPath_transform'>transform</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& matrix, <a href='#SkPath'>SkPath</a>* dst) const;
    void <a href='#SkPath_transform_2'>transform</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& matrix);
    bool <a href='#SkPath_getLastPt'>getLastPt</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a>* lastPt) const;
    void <a href='#SkPath_setLastPt'>setLastPt</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y);
    void <a href='#SkPath_setLastPt_2'>setLastPt</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p);

    enum <a href='#SkPath_SegmentMask'>SegmentMask</a> {
        <a href='#SkPath_kLine_SegmentMask'>kLine_SegmentMask</a> = 1 << 0,
        <a href='#SkPath_kQuad_SegmentMask'>kQuad_SegmentMask</a> = 1 << 1,
        <a href='#SkPath_kConic_SegmentMask'>kConic_SegmentMask</a> = 1 << 2,
        <a href='#SkPath_kCubic_SegmentMask'>kCubic_SegmentMask</a> = 1 << 3,
    };

    uint32_t <a href='#SkPath_getSegmentMasks'>getSegmentMasks</a>() const;

    enum <a href='#SkPath_Verb'>Verb</a> {
        <a href='#SkPath_kMove_Verb'>kMove_Verb</a>,
        <a href='#SkPath_kLine_Verb'>kLine_Verb</a>,
        <a href='#SkPath_kQuad_Verb'>kQuad_Verb</a>,
        <a href='#SkPath_kConic_Verb'>kConic_Verb</a>,
        <a href='#SkPath_kCubic_Verb'>kCubic_Verb</a>,
        <a href='#SkPath_kClose_Verb'>kClose_Verb</a>,
        <a href='#SkPath_kDone_Verb'>kDone_Verb</a>,
    };

    class <a href='#SkPath_Iter'>Iter</a> {
    public:
        <a href='#SkPath_Iter_Iter'>Iter</a>();
        <a href='#SkPath_Iter_const_SkPath'>Iter</a>(const <a href='#SkPath'>SkPath</a>& path, bool forceClose);
        void <a href='#SkPath_Iter_setPath'>setPath</a>(const <a href='#SkPath'>SkPath</a>& path, bool forceClose);
        <a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Iter_next'>next</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> pts[4], bool doConsumeDegenerates = true, bool exact = false);
        <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPath_Iter_conicWeight'>conicWeight</a>() const;
        bool <a href='#SkPath_Iter_isCloseLine'>isCloseLine</a>() const;
        bool <a href='#SkPath_Iter_isClosedContour'>isClosedContour</a>() const;
    };

    class <a href='#SkPath_RawIter'>RawIter</a> {
    public:
        <a href='#SkPath_RawIter_RawIter'>RawIter</a>();
        <a href='#SkPath_RawIter_copy_const_SkPath'>RawIter</a>(const <a href='#SkPath'>SkPath</a>& path);
        void <a href='#SkPath_RawIter_setPath'>setPath</a>(const <a href='#SkPath'>SkPath</a>& path);
        <a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_RawIter_next'>next</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> pts[4]);
        <a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_RawIter_peek'>peek</a>() const;
        <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPath_RawIter_conicWeight'>conicWeight</a>() const;
    };

    bool <a href='#SkPath_contains'>contains</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y) const;
    void <a href='#SkPath_dump'>dump</a>(<a href='SkWStream_Reference#SkWStream'>SkWStream</a>* stream, bool forceClose, bool dumpAsHex) const;
    void <a href='#SkPath_dump_2'>dump</a>() const;
    void <a href='#SkPath_dumpHex'>dumpHex</a>() const;
    size_t <a href='#SkPath_writeToMemory'>writeToMemory</a>(void* buffer) const;
    <a href='undocumented#sk_sp'>sk_sp</a><<a href='undocumented#SkData'>SkData</a>> <a href='#SkPath_serialize'>serialize</a>() const;
    size_t <a href='#SkPath_readFromMemory'>readFromMemory</a>(const void* buffer, size_t length);
    uint32_t <a href='#SkPath_getGenerationID'>getGenerationID</a>() const;
    bool <a href='#SkPath_isValid'>isValid</a>() const;
    bool <a href='#SkPath_isValid'>isValid</a>() const;
    bool <a href='#SkPath_pathRefIsValid'>pathRefIsValid</a>() const;
};
</pre>

<a href='#Path'>Paths</a> contain geometry

## <a name='Verb'>Verb</a>

## <a name='SkPath_Verb'>Enum SkPath::Verb</a>

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

<a href='#SkPath_Verb'>Verb</a> instructs <a href='#Path'>Path</a> how to interpret one or more <a href='SkPoint_Reference#Point'>Point</a> and optional <a href='#Conic_Weight'>Conic Weight</a>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kMove_Verb'><code>SkPath::kMove_Verb</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Consecutive <a href='#SkPath_kMove_Verb'>kMove Verb</a> are preserved but all but the last <a href='#SkPath_kMove_Verb'>kMove Verb</a> is
ignored</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kLine_Verb'><code>SkPath::kLine_Verb</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='undocumented#Line'>Line</a> is a straight segment from <a href='SkPoint_Reference#Point'>Point</a> to <a href='SkPoint_Reference#Point'>Point</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kQuad_Verb'><code>SkPath::kQuad_Verb</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Adds <a href='#Quad'>Quad</a> from <a href='#Last_Point'>Last Point</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kConic_Verb'><code>SkPath::kConic_Verb</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Adds <a href='#Conic'>Conic</a> from <a href='#Last_Point'>Last Point</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kCubic_Verb'><code>SkPath::kCubic_Verb</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Adds <a href='#Cubic'>Cubic</a> from <a href='#Last_Point'>Last Point</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kClose_Verb'><code>SkPath::kClose_Verb</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>5</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Closes <a href='SkPath_Overview#Contour'>Contour</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kDone_Verb'><code>SkPath::kDone_Verb</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>6</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Not in <a href='#Verb_Array'>Verb Array</a></td>
  </tr>
Each <a href='#SkPath_Verb'>Verb</a> has zero or more <a href='SkPoint_Reference#Point'>Points</a> stored in <a href='#Path'>Path</a></table>

| <a href='#SkPath_Verb'>Verb</a> | Allocated <a href='SkPoint_Reference#Point'>Points</a> | Iterated <a href='SkPoint_Reference#Point'>Points</a> | <a href='#Conic_Weight'>Weights</a> |
| --- | --- | --- | ---  |
| <a href='#SkPath_kMove_Verb'>kMove Verb</a> | 1 | 1 | 0 |
| <a href='#SkPath_kLine_Verb'>kLine Verb</a> | 1 | 2 | 0 |
| <a href='#SkPath_kQuad_Verb'>kQuad Verb</a> | 2 | 3 | 0 |
| <a href='#SkPath_kConic_Verb'>kConic Verb</a> | 2 | 3 | 1 |
| <a href='#SkPath_kCubic_Verb'>kCubic Verb</a> | 3 | 4 | 0 |
| <a href='#SkPath_kClose_Verb'>kClose Verb</a> | 0 | 1 | 0 |
| <a href='#SkPath_kDone_Verb'>kDone Verb</a> |  | 0 | 0 |

### Example

<div><fiddle-embed name="799096fdc1298aa815934a74e76570ca">

#### Example Output

~~~~
verb count: 7
verbs: kMove_Verb kLine_Verb kQuad_Verb kClose_Verb kMove_Verb kCubic_Verb kConic_Verb
~~~~

</fiddle-embed></div>

## <a name='Direction'>Direction</a>

## <a name='SkPath_Direction'>Enum SkPath::Direction</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPath_Direction'>Direction</a> {
        <a href='#SkPath_kCW_Direction'>kCW_Direction</a>,
        <a href='#SkPath_kCCW_Direction'>kCCW_Direction</a>,
    };
</pre>

<a href='#SkPath_Direction'>Direction</a> describes whether <a href='SkPath_Overview#Contour'>Contour</a> is clockwise or counterclockwise

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

<div><fiddle-embed name="0de03d9c939b6238318b7366866e8722"></fiddle-embed></div>

### See Also

<a href='#SkPath_arcTo'>arcTo</a><sup><a href='#SkPath_arcTo_2'>[2]</a></sup><sup><a href='#SkPath_arcTo_3'>[3]</a></sup><sup><a href='#SkPath_arcTo_4'>[4]</a></sup><sup><a href='#SkPath_arcTo_5'>[5]</a></sup> <a href='#SkPath_rArcTo'>rArcTo</a> <a href='#SkPath_isRect'>isRect</a> <a href='#SkPath_isNestedFillRects'>isNestedFillRects</a> <a href='#SkPath_addRect'>addRect</a><sup><a href='#SkPath_addRect_2'>[2]</a></sup><sup><a href='#SkPath_addRect_3'>[3]</a></sup> <a href='#SkPath_addOval'>addOval</a><sup><a href='#SkPath_addOval_2'>[2]</a></sup>

<a name='SkPath_empty_constructor'></a>
## SkPath

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>(
</pre>

Constucts an empty <a href='#Path'>Path</a>

### Return Value

empty <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="0a0026fca638d1cd75c0ab884e3ee1c6">

#### Example Output

~~~~
path is empty
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_reset'>reset</a> <a href='#SkPath_rewind'>rewind</a>

---

<a name='SkPath_copy_const_SkPath'></a>
## SkPath

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>(const <a href='#SkPath'>SkPath</a>
</pre>

Constructs a copy of an existing <a href='#SkPath_copy_const_SkPath_path'>path</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_copy_const_SkPath_path'><code><strong>path</strong></code></a></td>
    <td><a href='#Path'>Path</a> to copy by value</td>
  </tr>
</table>

### Return Value

copy of <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="647312aacd946c8a6eabaca797140432"><div>Modifying one <a href='#SkPath_copy_const_SkPath_path'>path</a> does not effect another</div>

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

---

<a name='SkPath_destructor'></a>
## ~SkPath

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_destructor'>~SkPath</a>(
</pre>

Releases ownership of any shared data and deletes data if <a href='#Path'>Path</a> is sole owner

### Example

<div><fiddle-embed name="01ad6be9b7d15a2217daea273eb3d466"><div>delete calls <a href='#Path'>Path</a> <a href='undocumented#Destructor'>Destructor</a></div></fiddle-embed></div>

### See Also

<a href='#SkPath_empty_constructor'>SkPath()</a> <a href='#SkPath_copy_const_SkPath'>SkPath(const SkPath& path)</a> <a href='#SkPath_copy_operator'>operator=(const SkPath& path)</a>

---

<a name='SkPath_copy_operator'></a>
## operator=

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Constructs a copy of an existing <a href='#SkPath_copy_operator_path'>path</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_copy_operator_path'><code><strong>path</strong></code></a></td>
    <td><a href='#Verb_Array'>Verb Array</a></td>
  </tr>
</table>

### Return Value

<a href='#Path'>Path</a> copied by value

### Example

<div><fiddle-embed name="bba288f5f77fc8e37e89d2ec08e0ac60">

#### Example Output

~~~~
path1 bounds = 10, 20, 30, 40
path2 bounds = 10, 20, 30, 40
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_swap'>swap</a> <a href='#SkPath_copy_const_SkPath'>SkPath(const SkPath& path)</a>

---

<a name='SkPath_equal_operator'></a>
## operator==

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_equal_operator'>operator==(const SkPath& a, const SkPath& b)</a>
</pre>

Compares <a href='#SkPath_equal_operator_a'>a</a> and <a href='#SkPath_equal_operator_b'>b</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_equal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='#Path'>Path</a> to compare</td>
  </tr>
  <tr>    <td><a name='SkPath_equal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='#Path'>Path</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href='#Path'>Path</a> pair are equivalent

### Example

<div><fiddle-embed name="31883f51bb357f2ac5990d88f8b82e02"><div>Rewind removes <a href='#Verb_Array'>Verb Array</a> but leaves storage</div>

#### Example Output

~~~~
empty one == two
moveTo one != two
rewind one == two
reset one == two
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_notequal_operator'>operator!=(const SkPath& a, const SkPath& b)</a> <a href='#SkPath_copy_operator'>operator=(const SkPath& path)</a>

---

<a name='SkPath_notequal_operator'></a>
## operator!=

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_notequal_operator'>operator!=(const SkPath& a, const SkPath& b)</a>
</pre>

Compares <a href='#SkPath_notequal_operator_a'>a</a> and <a href='#SkPath_notequal_operator_b'>b</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_notequal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='#Path'>Path</a> to compare</td>
  </tr>
  <tr>    <td><a name='SkPath_notequal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='#Path'>Path</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href='#Path'>Path</a> pair are not equivalent

### Example

<div><fiddle-embed name="bbbda1cc818d96c9c0d2a06c0c48902b"><div><a href='#Path'>Path</a> pair are equal though their convexity is not equal</div>

#### Example Output

~~~~
empty one == two
add rect one == two
setConvexity one == two
convexity !=
~~~~

</fiddle-embed></div>

---

## <a name='Property'>Property</a>

<a name='SkPath_isInterpolatable'></a>
## isInterpolatable

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isInterpolatable'>isInterpolatable</a>(const <a href='#SkPath'>SkPath</a>
</pre>

Returns true if <a href='#Path'>Paths</a> contain equal <a href='#Verb'>Verbs</a> and equal <a href='#Conic_Weight'>Weights</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_isInterpolatable_compare'><code><strong>compare</strong></code></a></td>
    <td><a href='#Path'>Path</a> to <a href='#SkPath_isInterpolatable_compare'>compare</a></td>
  </tr>
</table>

### Return Value

true if <a href='#Path'>Paths</a> <a href='#Verb_Array'>Verb Array</a> and <a href='#Conic_Weight'>Weights</a> are equivalent

### Example

<div><fiddle-embed name="c81fc7dfaf785c3fb77209c7f2ebe5b8">

#### Example Output

~~~~
paths are interpolatable
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_isInterpolatable'>isInterpolatable</a>

---

## <a name='Interpolate'>Interpolate</a>

<a name='SkPath_interpolate'></a>
## interpolate

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_interpolate'>interpolate</a>(const <a href='#SkPath'>SkPath</a>
</pre>

Interpolates between <a href='#Path'>Paths</a> with <a href='#Point_Array'>Point Array</a> of equal size <code></code>

### Parameters

<table>  <tr>    <td><a name='SkPath_interpolate_ending'><code><strong>ending</strong></code></a></td>
    <td><a href='#Point_Array'>Point Array</a> averaged with this <a href='#Point_Array'>Point Array</a></td>
  </tr>
  <tr>    <td><a name='SkPath_interpolate_weight'><code><strong>weight</strong></code></a></td>
    <td>contribution of this <a href='#Point_Array'>Point Array</a></td>
  </tr>
  <tr>    <td><a name='SkPath_interpolate_out'><code><strong>out</strong></code></a></td>
    <td><a href='#Path'>Path</a> replaced by interpolated averages</td>
  </tr>
</table>

### Return Value

true if <a href='#Path'>Paths</a> contain same number of <a href='SkPoint_Reference#Point'>Points</a>

### Example

<div><fiddle-embed name="404f11c5c9c9ca8a64822d484552a473"></fiddle-embed></div>

### See Also

<a href='#SkPath_isInterpolatable'>isInterpolatable</a>

---

<a name='SkPath_unique'></a>
## unique

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_unique'>unique</a>(
</pre>

To be deprecated soon.

Only valid for Android framework

---

## <a name='Fill_Type'>Fill Type</a>

## <a name='SkPath_FillType'>Enum SkPath::FillType</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPath_FillType'>FillType</a> {
        <a href='#SkPath_kWinding_FillType'>kWinding_FillType</a>,
        <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd_FillType</a>,
        <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding_FillType</a>,
        <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd_FillType</a>,
    };
</pre>

<a href='#Fill_Type'>Fill Type</a> selects the rule used to fill <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="71fc6c069c377d808799f2453edabaf5"><div>The top row has two clockwise rectangles</div></fiddle-embed></div>

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

<div><fiddle-embed name="d84cd32b0bfd9ad2714f753120ed0ee1"></fiddle-embed></div>

### See Also

<a href='SkPaint_Reference#SkPaint_Style'>SkPaint::Style</a> <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_getFillType'>getFillType</a> <a href='#SkPath_setFillType'>setFillType</a>

<a name='SkPath_getFillType'></a>
## getFillType

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_getFillType'>getFillType</a>(
</pre>

Returns <a href='#SkPath_FillType'>FillType</a>

### Return Value

one of

### Example

<div><fiddle-embed name="019af90e778914e8a109d6305ede4fc4">

#### Example Output

~~~~
default path fill type is kWinding_FillType
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_setFillType'>setFillType</a> <a href='#SkPath_isInverseFillType'>isInverseFillType</a>

---

<a name='SkPath_setFillType'></a>
## setFillType

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_setFillType'>setFillType</a>(<a href='#SkPath_FillType'>FillType</a> ft
</pre>

Sets <a href='#SkPath_FillType'>FillType</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_setFillType_ft'><code><strong>ft</strong></code></a></td>
    <td>one of</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="b4a91cd7f50b2a0a0d1bec6d0ac823d2"><div>If empty <a href='#Path'>Path</a> is set to inverse <a href='#SkPath_FillType'>FillType</a></div></fiddle-embed></div>

### See Also

<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_getFillType'>getFillType</a> <a href='#SkPath_toggleInverseFillType'>toggleInverseFillType</a>

---

<a name='SkPath_isInverseFillType'></a>
## isInverseFillType

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isInverseFillType'>isInverseFillType</a>(
</pre>

Returns if <a href='#SkPath_FillType'>FillType</a> describes area outside <a href='#Path'>Path</a> geometry

### Return Value

true if <a href='#SkPath_FillType'>FillType</a> is <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding FillType</a> or <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd FillType</a>

### Example

<div><fiddle-embed name="2a2d39f5da611545caa18bbcea873ab2">

#### Example Output

~~~~
default path fill type is inverse: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_getFillType'>getFillType</a> <a href='#SkPath_setFillType'>setFillType</a> <a href='#SkPath_toggleInverseFillType'>toggleInverseFillType</a>

---

<a name='SkPath_toggleInverseFillType'></a>
## toggleInverseFillType

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_toggleInverseFillType'>toggleInverseFillType</a>(
</pre>

Replaces <a href='#SkPath_FillType'>FillType</a> with its inverse

| <a href='#SkPath_FillType'>FillType</a> | toggled <a href='#SkPath_FillType'>FillType</a> |
| --- | ---  |
| <a href='#SkPath_kWinding_FillType'>kWinding FillType</a> | <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding FillType</a> |
| <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd FillType</a> | <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd FillType</a> |
| <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding FillType</a> | <a href='#SkPath_kWinding_FillType'>kWinding FillType</a> |
| <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd FillType</a> | <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd FillType</a> |

### Example

<div><fiddle-embed name="400facce23d417bc5043c5f58404afbd"><div><a href='#Path'>Path</a> drawn normally and through its inverse touches every pixel once</div></fiddle-embed></div>

### See Also

<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_getFillType'>getFillType</a> <a href='#SkPath_setFillType'>setFillType</a> <a href='#SkPath_isInverseFillType'>isInverseFillType</a>

---

## <a name='Convexity'>Convexity</a>

## <a name='SkPath_Convexity'>Enum SkPath::Convexity</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPath_Convexity'>Convexity</a> : uint8_t {
        <a href='#SkPath_kUnknown_Convexity'>kUnknown_Convexity</a>,
        <a href='#SkPath_kConvex_Convexity'>kConvex_Convexity</a>,
        <a href='#SkPath_kConcave_Convexity'>kConcave_Convexity</a>,
    };
</pre>

<a href='#Path'>Path</a> is convex if it contains one <a href='SkPath_Overview#Contour'>Contour</a> and <a href='SkPath_Overview#Contour'>Contour</a> loops no more than
360 degrees

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
## getConvexity

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_getConvexity'>getConvexity</a>(
</pre>

Computes <a href='#SkPath_Convexity'>Convexity</a> if required

### Return Value

computed or stored <a href='#SkPath_Convexity'>Convexity</a>

### Example

<div><fiddle-embed name="a8f36f2fa90003e3691fd0da0bb0c243"></fiddle-embed></div>

### See Also

<a href='#SkPath_Convexity'>Convexity</a> <a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_getConvexityOrUnknown'>getConvexityOrUnknown</a> <a href='#SkPath_setConvexity'>setConvexity</a> <a href='#SkPath_isConvex'>isConvex</a>

---

<a name='SkPath_getConvexityOrUnknown'></a>
## getConvexityOrUnknown

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_getConvexityOrUnknown'>getConvexityOrUnknown</a>(
</pre>

Returns last computed <a href='#SkPath_Convexity'>Convexity</a>

### Return Value

stored <a href='#SkPath_Convexity'>Convexity</a>

### Example

<div><fiddle-embed name="111c59e9afadb940ab8f41bdc25378a4"><div><a href='#SkPath_Convexity'>Convexity</a> is unknown unless <a href='#SkPath_getConvexity'>getConvexity</a> is called without a subsequent call
that alters the path</div></fiddle-embed></div>

### See Also

<a href='#SkPath_Convexity'>Convexity</a> <a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_getConvexity'>getConvexity</a> <a href='#SkPath_setConvexity'>setConvexity</a> <a href='#SkPath_isConvex'>isConvex</a>

---

<a name='SkPath_setConvexity'></a>
## setConvexity

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_setConvexity'>setConvexity</a>(<a href='#SkPath_Convexity'>Convexity</a> convexity
</pre>

Stores <a href='#SkPath_setConvexity_convexity'>convexity</a> so that it is later returned by <a href='#SkPath_getConvexity'>getConvexity</a> or <a href='#SkPath_getConvexityOrUnknown'>getConvexityOrUnknown</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_setConvexity_convexity'><code><strong>convexity</strong></code></a></td>
    <td>one of</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="875e32b4b1cb48d739325705fc0fa42c"></fiddle-embed></div>

### See Also

<a href='#SkPath_Convexity'>Convexity</a> <a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_getConvexity'>getConvexity</a> <a href='#SkPath_getConvexityOrUnknown'>getConvexityOrUnknown</a> <a href='#SkPath_isConvex'>isConvex</a>

---

<a name='SkPath_isConvex'></a>
## isConvex

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isConvex'>isConvex</a>(
</pre>

Computes <a href='#SkPath_Convexity'>Convexity</a> if required

### Return Value

true if <a href='#SkPath_Convexity'>Convexity</a> stored or computed is <a href='#SkPath_kConvex_Convexity'>kConvex Convexity</a>

### Example

<div><fiddle-embed name="d8be8b6e59de244e4cbf58ec9554557b"><div>Concave shape is erroneously considered convex after a forced call to
<a href='#SkPath_setConvexity'>setConvexity</a></div></fiddle-embed></div>

### See Also

<a href='#SkPath_Convexity'>Convexity</a> <a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_getConvexity'>getConvexity</a> <a href='#SkPath_getConvexityOrUnknown'>getConvexityOrUnknown</a> <a href='#SkPath_setConvexity'>setConvexity</a>

---

<a name='SkPath_isOval'></a>
## isOval

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isOval'>isOval</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Returns true if this path is recognized as an oval or circle

### Parameters

<table>  <tr>    <td><a name='SkPath_isOval_bounds'><code><strong>bounds</strong></code></a></td>
    <td>storage for bounding <a href='SkRect_Reference#Rect'>Rect</a> of <a href='undocumented#Oval'>Oval</a></td>
  </tr>
</table>

### Return Value

true if <a href='#Path'>Path</a> is recognized as an oval or circle

### Example

<div><fiddle-embed name="a51256952b183ee0f7004f2c87cbbf5b"></fiddle-embed></div>

### See Also

<a href='undocumented#Oval'>Oval</a> <a href='#SkPath_addCircle'>addCircle</a> <a href='#SkPath_addOval'>addOval</a><sup><a href='#SkPath_addOval_2'>[2]</a></sup>

---

<a name='SkPath_isRRect'></a>
## isRRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isRRect'>isRRect</a>(<a href='SkRRect_Reference#SkRRect'>SkRRect</a>
</pre>

Returns true if this path is recognized as a <a href='SkRRect_Reference#SkRRect'>SkRRect</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_isRRect_rrect'><code><strong>rrect</strong></code></a></td>
    <td>storage for bounding <a href='SkRect_Reference#Rect'>Rect</a> of <a href='SkRRect_Reference#RRect'>Round Rect</a></td>
  </tr>
</table>

### Return Value

true if <a href='#Path'>Path</a> contains only <a href='SkRRect_Reference#RRect'>Round Rect</a>

### Example

<div><fiddle-embed name="2aa939b90d96aff436b145a96305132c"><div>Draw rounded rectangle and its bounds</div></fiddle-embed></div>

### See Also

<a href='SkRRect_Reference#RRect'>Round Rect</a> <a href='#SkPath_addRoundRect'>addRoundRect</a><sup><a href='#SkPath_addRoundRect_2'>[2]</a></sup> <a href='#SkPath_addRRect'>addRRect</a><sup><a href='#SkPath_addRRect_2'>[2]</a></sup>

---

<a name='SkPath_reset'></a>
## reset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Sets <a href='#Path'>Path</a> to its initial state

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="8cdca35d2964bbbecb93d79a13f71c65"></fiddle-embed></div>

### See Also

<a href='#SkPath_rewind'>rewind</a>(

---

<a name='SkPath_rewind'></a>
## rewind

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Sets <a href='#Path'>Path</a> to its initial state

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="f1fedbb89da9c2a33a91805175663012"><div>Although path1 retains its internal storage</div></fiddle-embed></div>

### See Also

<a href='#SkPath_reset'>reset</a>(

---

<a name='SkPath_isEmpty'></a>
## isEmpty

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isEmpty'>isEmpty</a>(
</pre>

Returns if <a href='#Path'>Path</a> is empty

### Return Value

true if the path contains no <a href='#SkPath_Verb'>Verb</a> array

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

<a href='#SkPath_empty_constructor'>SkPath()</a> <a href='#SkPath_reset'>reset</a>(

---

<a name='SkPath_isLastContourClosed'></a>
## isLastContourClosed

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isLastContourClosed'>isLastContourClosed</a>(
</pre>

Returns if <a href='SkPath_Overview#Contour'>Contour</a> is closed

### Return Value

true if the last <a href='SkPath_Overview#Contour'>Contour</a> ends with a <a href='#SkPath_kClose_Verb'>kClose Verb</a>

### Example

<div><fiddle-embed name="03b740ab94b9017800a52e30b5e7fee7"><div><a href='#SkPath_close'>close</a>(</div>

#### Example Output

~~~~
initial last contour is not closed
after close last contour is not closed
after lineTo last contour is not closed
after close last contour is closed
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_close'>close</a>(

---

<a name='SkPath_isFinite'></a>
## isFinite

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isFinite'>isFinite</a>(
</pre>

Returns true for finite <a href='SkPoint_Reference#Point'>Point</a> array values between negative <a href='undocumented#SK_ScalarMax'>SK ScalarMax</a> and
positive <a href='undocumented#SK_ScalarMax'>SK ScalarMax</a>

### Return Value

true if all <a href='SkPoint_Reference#Point'>Point</a> values are finite

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

---

<a name='SkPath_isVolatile'></a>
## isVolatile

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isVolatile'>isVolatile</a>(
</pre>

Returns true if the path is volatile

### Return Value

true if caller will alter <a href='#Path'>Path</a> after drawing

### Example

<div><fiddle-embed name="c722ebe8ac991d77757799ce29e509e1">

#### Example Output

~~~~
volatile by default is false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_setIsVolatile'>setIsVolatile</a>

---

## <a name='Volatile'>Volatile</a>

<a name='SkPath_setIsVolatile'></a>
## setIsVolatile

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_setIsVolatile'>setIsVolatile</a>(bool <a href='#SkPath_isVolatile'>isVolatile</a>
</pre>

Specifies whether <a href='#Path'>Path</a> is volatile

### Parameters

<table>  <tr>    <td><a name='SkPath_setIsVolatile_isVolatile'><code><strong>isVolatile</strong></code></a></td>
    <td>true if caller will alter <a href='#Path'>Path</a> after drawing</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="2049ff5141f0c80aac497618622b28af"></fiddle-embed></div>

### See Also

<a href='#SkPath_isVolatile'>isVolatile</a>

---

<a name='SkPath_IsLineDegenerate'></a>
## IsLineDegenerate

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkPath_IsLineDegenerate'>IsLineDegenerate</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>
</pre>

Tests if <a href='undocumented#Line'>Line</a> between <a href='SkPoint_Reference#Point'>Point</a> pair is degenerate

### Parameters

<table>  <tr>    <td><a name='SkPath_IsLineDegenerate_p1'><code><strong>p1</strong></code></a></td>
    <td>line start point</td>
  </tr>
  <tr>    <td><a name='SkPath_IsLineDegenerate_p2'><code><strong>p2</strong></code></a></td>
    <td>line end point</td>
  </tr>
  <tr>    <td><a name='SkPath_IsLineDegenerate_exact'><code><strong>exact</strong></code></a></td>
    <td>if false</td>
  </tr>
</table>

### Return Value

true if <a href='undocumented#Line'>Line</a> is degenerate

### Example

<div><fiddle-embed name="97a031f9186ade586928563840ce9116"><div>As single precision floats</div>

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

---

<a name='SkPath_IsQuadDegenerate'></a>
## IsQuadDegenerate

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkPath_IsQuadDegenerate'>IsQuadDegenerate</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>
</pre>

Tests if <a href='#Quad'>Quad</a> is degenerate

### Parameters

<table>  <tr>    <td><a name='SkPath_IsQuadDegenerate_p1'><code><strong>p1</strong></code></a></td>
    <td><a href='#Quad'>Quad</a> start point</td>
  </tr>
  <tr>    <td><a name='SkPath_IsQuadDegenerate_p2'><code><strong>p2</strong></code></a></td>
    <td><a href='#Quad'>Quad</a> control point</td>
  </tr>
  <tr>    <td><a name='SkPath_IsQuadDegenerate_p3'><code><strong>p3</strong></code></a></td>
    <td><a href='#Quad'>Quad</a> end point</td>
  </tr>
  <tr>    <td><a name='SkPath_IsQuadDegenerate_exact'><code><strong>exact</strong></code></a></td>
    <td>if true</td>
  </tr>
</table>

### Return Value

true if <a href='#Quad'>Quad</a> is degenerate

### Example

<div><fiddle-embed name="a2b255a7dac1926cc3a247d318d63c62"><div>As single precision floats</div>

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

---

<a name='SkPath_IsCubicDegenerate'></a>
## IsCubicDegenerate

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkPath_IsCubicDegenerate'>IsCubicDegenerate</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>
</pre>

Tests if <a href='#Cubic'>Cubic</a> is degenerate

### Parameters

<table>  <tr>    <td><a name='SkPath_IsCubicDegenerate_p1'><code><strong>p1</strong></code></a></td>
    <td><a href='#Cubic'>Cubic</a> start point</td>
  </tr>
  <tr>    <td><a name='SkPath_IsCubicDegenerate_p2'><code><strong>p2</strong></code></a></td>
    <td><a href='#Cubic'>Cubic</a> control point 1</td>
  </tr>
  <tr>    <td><a name='SkPath_IsCubicDegenerate_p3'><code><strong>p3</strong></code></a></td>
    <td><a href='#Cubic'>Cubic</a> control point 2</td>
  </tr>
  <tr>    <td><a name='SkPath_IsCubicDegenerate_p4'><code><strong>p4</strong></code></a></td>
    <td><a href='#Cubic'>Cubic</a> end point</td>
  </tr>
  <tr>    <td><a name='SkPath_IsCubicDegenerate_exact'><code><strong>exact</strong></code></a></td>
    <td>if true</td>
  </tr>
</table>

### Return Value

true if <a href='#Cubic'>Cubic</a> is degenerate

### Example

<div><fiddle-embed name="6b97099acdae80b16df0c4241f593991">

#### Example Output

~~~~
0.00024414062 is degenerate
0.00024414065 is length
~~~~

</fiddle-embed></div>

---

<a name='SkPath_isLine'></a>
## isLine

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isLine'>isLine</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> line
</pre>

Returns true if <a href='#Path'>Path</a> contains only one <a href='undocumented#Line'>Line</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_isLine_line'><code><strong>line</strong></code></a></td>
    <td>storage for <a href='undocumented#Line'>Line</a></td>
  </tr>
</table>

### Return Value

true if <a href='#Path'>Path</a> contains exactly one <a href='undocumented#Line'>Line</a>

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

---

## <a name='Point_Array'>Point Array</a>

<a href='#Point_Array'>Point Array</a> contains <a href='SkPoint_Reference#Point'>Points</a> satisfying the allocated <a href='SkPoint_Reference#Point'>Points</a> for
each <a href='#SkPath_Verb'>Verb</a> in <a href='#Verb_Array'>Verb Array</a>

<a name='SkPath_getPoints'></a>
## getPoints

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPath_getPoints'>getPoints</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> points
</pre>

Returns number of <a href='#SkPath_getPoints_points'>points</a> in <a href='#Path'>Path</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_getPoints_points'><code><strong>points</strong></code></a></td>
    <td>storage for <a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> array</td>
  </tr>
  <tr>    <td><a name='SkPath_getPoints_max'><code><strong>max</strong></code></a></td>
    <td>maximum to copy</td>
  </tr>
</table>

### Return Value

<a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> array length

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

---

<a name='SkPath_countPoints'></a>
## countPoints

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPath_countPoints'>countPoints</a>(
</pre>

Returns the number of points in <a href='#Path'>Path</a>

### Return Value

<a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> array length

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

---

<a name='SkPath_getPoint'></a>
## getPoint

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkPath_getPoint'>getPoint</a>(int index
</pre>

Returns <a href='SkPoint_Reference#Point'>Point</a> at <a href='#SkPath_getPoint_index'>index</a> in <a href='#Point_Array'>Point Array</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_getPoint_index'><code><strong>index</strong></code></a></td>
    <td><a href='SkPoint_Reference#Point'>Point</a> array element selector</td>
  </tr>
</table>

### Return Value

<a href='SkPoint_Reference#Point'>Point</a> array value or

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

---

## <a name='Verb_Array'>Verb Array</a>

<a href='#Verb_Array'>Verb Array</a> always starts with <a href='#SkPath_kMove_Verb'>kMove Verb</a>

<a name='SkPath_countVerbs'></a>
## countVerbs

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPath_countVerbs'>countVerbs</a>(
</pre>

Returns the number of <a href='#Verb'>Verbs</a>

### Return Value

length of <a href='#Verb_Array'>Verb Array</a>

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

---

<a name='SkPath_getVerbs'></a>
## getVerbs

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPath_getVerbs'>getVerbs</a>(uint8_t verbs
</pre>

Returns the number of <a href='#SkPath_getVerbs_verbs'>verbs</a> in the path

### Parameters

<table>  <tr>    <td><a name='SkPath_getVerbs_verbs'><code><strong>verbs</strong></code></a></td>
    <td>storage for <a href='#SkPath_getVerbs_verbs'>verbs</a></td>
  </tr>
  <tr>    <td><a name='SkPath_getVerbs_max'><code><strong>max</strong></code></a></td>
    <td>maximum number to copy into <a href='#SkPath_getVerbs_verbs'>verbs</a></td>
  </tr>
</table>

### Return Value

the actual number of <a href='#SkPath_getVerbs_verbs'>verbs</a> in the path

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

---

<a name='SkPath_swap'></a>
## swap

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_swap'>swap</a>(<a href='#SkPath'>SkPath</a>
</pre>

Exchanges the <a href='#Verb_Array'>Verb Array</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_swap_other'><code><strong>other</strong></code></a></td>
    <td><a href='#Path'>Path</a> exchanged by value</td>
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

---

<a name='SkPath_getBounds'></a>
## getBounds

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Returns minimum and maximum axes values of <a href='#Point_Array'>Point Array</a>

### Return Value

bounds of all <a href='SkPoint_Reference#Point'>Points</a> in <a href='#Point_Array'>Point Array</a>

### Example

<div><fiddle-embed name="45c0fc3acb74fab99d544b80eadd10ad"><div>Bounds of upright <a href='undocumented#Circle'>Circle</a> can be predicted from center and radius</div>

#### Example Output

~~~~
empty bounds = 0, 0, 0, 0
circle bounds = 25, 20, 75, 70
rotated circle bounds = 14.6447, 9.64466, 85.3553, 80.3553
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_computeTightBounds'>computeTightBounds</a> <a href='#SkPath_updateBoundsCache'>updateBoundsCache</a>

---

## <a name='Utility'>Utility</a>

<a name='SkPath_updateBoundsCache'></a>
## updateBoundsCache

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_updateBoundsCache'>updateBoundsCache</a>(
</pre>

Updates internal bounds so that subsequent calls to <a href='#SkPath_getBounds'>getBounds</a> are instantaneous

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

---

<a name='SkPath_computeTightBounds'></a>
## computeTightBounds

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkPath_computeTightBounds'>computeTightBounds</a>(
</pre>

Returns minimum and maximum axes values of the lines and curves in <a href='#Path'>Path</a>

### Return Value

tight bounds of curves in <a href='#Path'>Path</a>

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

---

<a name='SkPath_conservativelyContainsRect'></a>
## conservativelyContainsRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_conservativelyContainsRect'>conservativelyContainsRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Returns true if <a href='#SkPath_conservativelyContainsRect_rect'>rect</a> is contained by <a href='#Path'>Path</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_conservativelyContainsRect_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkPath_conservativelyContainsRect_rect'>rect</a> is contained

### Example

<div><fiddle-embed name="41638d13e40fa449ece354dde5fb1941"><div><a href='SkRect_Reference#Rect'>Rect</a> is drawn in blue if it is contained by red <a href='#Path'>Path</a></div></fiddle-embed></div>

### See Also

<a href='#SkPath_contains'>contains</a> <a href='undocumented#Op'>Op</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkPath_Convexity'>Convexity</a>

---

<a name='SkPath_incReserve'></a>
## incReserve

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_incReserve'>incReserve</a>(unsigned extraPtCount
</pre>

Grows <a href='#Path'>Path</a> <a href='#Verb_Array'>Verb Array</a> and <a href='#Point_Array'>Point Array</a> to contain <a href='#SkPath_incReserve_extraPtCount'>extraPtCount</a> additional <a href='SkPoint_Reference#Point'>Points</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_incReserve_extraPtCount'><code><strong>extraPtCount</strong></code></a></td>
    <td>number of additional <a href='SkPoint_Reference#Point'>Points</a> to allocate</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f2260f2a170a54aef5bafe5b91c121b3"></fiddle-embed></div>

### See Also

<a href='#Point_Array'>Point Array</a>

---

<a name='SkPath_shrinkToFit'></a>
## shrinkToFit

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_shrinkToFit'>shrinkToFit</a>(
</pre>

Shrinks <a href='#Path'>Path</a> <a href='#Verb_Array'>Verb Array</a> and <a href='#Point_Array'>Point Array</a> storage to discard unused capacity

### See Also

<a href='#SkPath_incReserve'>incReserve</a>

---

## <a name='Build'>Build</a>

<a name='SkPath_moveTo'></a>
## moveTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds beginning of <a href='SkPath_Overview#Contour'>Contour</a> at <a href='SkPoint_Reference#Point'>Point</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_moveTo_x'><code><strong>x</strong></code></a></td>
    <td><a href='#SkPath_moveTo_x'>x</a></td>
  </tr>
  <tr>    <td><a name='SkPath_moveTo_y'><code><strong>y</strong></code></a></td>
    <td><a href='#SkPath_moveTo_y'>y</a></td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="84101d341e934a535a41ad6cf42218ce"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_lineTo'>lineTo</a><sup><a href='#SkPath_lineTo_2'>[2]</a></sup> <a href='#SkPath_rMoveTo'>rMoveTo</a> <a href='#SkPath_quadTo'>quadTo</a><sup><a href='#SkPath_quadTo_2'>[2]</a></sup> <a href='#SkPath_conicTo'>conicTo</a><sup><a href='#SkPath_conicTo_2'>[2]</a></sup> <a href='#SkPath_cubicTo'>cubicTo</a><sup><a href='#SkPath_cubicTo_2'>[2]</a></sup> <a href='#SkPath_close'>close</a>(

---

<a name='SkPath_moveTo_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds beginning of <a href='SkPath_Overview#Contour'>Contour</a> at <a href='SkPoint_Reference#Point'>Point</a> <a href='#SkPath_moveTo_2_p'>p</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_moveTo_2_p'><code><strong>p</strong></code></a></td>
    <td>contour start</td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="cb8d37990f6e7df3bcc85e7240c81274"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_lineTo'>lineTo</a><sup><a href='#SkPath_lineTo_2'>[2]</a></sup> <a href='#SkPath_rMoveTo'>rMoveTo</a> <a href='#SkPath_quadTo'>quadTo</a><sup><a href='#SkPath_quadTo_2'>[2]</a></sup> <a href='#SkPath_conicTo'>conicTo</a><sup><a href='#SkPath_conicTo_2'>[2]</a></sup> <a href='#SkPath_cubicTo'>cubicTo</a><sup><a href='#SkPath_cubicTo_2'>[2]</a></sup> <a href='#SkPath_close'>close</a>(

---

<a name='SkPath_rMoveTo'></a>
## rMoveTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds beginning of <a href='SkPath_Overview#Contour'>Contour</a> relative to <a href='#Last_Point'>Last Point</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_rMoveTo_dx'><code><strong>dx</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='SkPath_Overview#Contour'>Contour</a> start on x</td>
  </tr>
  <tr>    <td><a name='SkPath_rMoveTo_dy'><code><strong>dy</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='SkPath_Overview#Contour'>Contour</a> start on y</td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="63e32dec4b2d8440b427f368bf8313a4"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_lineTo'>lineTo</a><sup><a href='#SkPath_lineTo_2'>[2]</a></sup> <a href='#SkPath_moveTo'>moveTo</a><sup><a href='#SkPath_moveTo_2'>[2]</a></sup> <a href='#SkPath_quadTo'>quadTo</a><sup><a href='#SkPath_quadTo_2'>[2]</a></sup> <a href='#SkPath_conicTo'>conicTo</a><sup><a href='#SkPath_conicTo_2'>[2]</a></sup> <a href='#SkPath_cubicTo'>cubicTo</a><sup><a href='#SkPath_cubicTo_2'>[2]</a></sup> <a href='#SkPath_close'>close</a>(

---

<a name='SkPath_lineTo'></a>
## lineTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds <a href='undocumented#Line'>Line</a> from <a href='#Last_Point'>Last Point</a> to

### Parameters

<table>  <tr>    <td><a name='SkPath_lineTo_x'><code><strong>x</strong></code></a></td>
    <td>end of added <a href='undocumented#Line'>Line</a> in <a href='#SkPath_lineTo_x'>x</a></td>
  </tr>
  <tr>    <td><a name='SkPath_lineTo_y'><code><strong>y</strong></code></a></td>
    <td>end of added <a href='undocumented#Line'>Line</a> in <a href='#SkPath_lineTo_y'>y</a></td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="e311cdd451edacec33b50cc22a4dd5dc"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a><sup><a href='#SkPath_moveTo_2'>[2]</a></sup> <a href='#SkPath_rLineTo'>rLineTo</a> <a href='#SkPath_addRect'>addRect</a><sup><a href='#SkPath_addRect_2'>[2]</a></sup><sup><a href='#SkPath_addRect_3'>[3]</a></sup>

---

<a name='SkPath_lineTo_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds <a href='undocumented#Line'>Line</a> from <a href='#Last_Point'>Last Point</a> to <a href='SkPoint_Reference#Point'>Point</a> <a href='#SkPath_lineTo_2_p'>p</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_lineTo_2_p'><code><strong>p</strong></code></a></td>
    <td>end <a href='SkPoint_Reference#Point'>Point</a> of added <a href='undocumented#Line'>Line</a></td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="41001546a7f7927d08e5a818bcc304f5"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a><sup><a href='#SkPath_moveTo_2'>[2]</a></sup> <a href='#SkPath_rLineTo'>rLineTo</a> <a href='#SkPath_addRect'>addRect</a><sup><a href='#SkPath_addRect_2'>[2]</a></sup><sup><a href='#SkPath_addRect_3'>[3]</a></sup>

---

<a name='SkPath_rLineTo'></a>
## rLineTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds <a href='undocumented#Line'>Line</a> from <a href='#Last_Point'>Last Point</a> to <a href='SkPoint_Reference#Vector'>Vector</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_rLineTo_dx'><code><strong>dx</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='undocumented#Line'>Line</a> end on x</td>
  </tr>
  <tr>    <td><a name='SkPath_rLineTo_dy'><code><strong>dy</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='undocumented#Line'>Line</a> end on y</td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="6e0be0766b8ca320da51640326e608b3"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a><sup><a href='#SkPath_moveTo_2'>[2]</a></sup> <a href='#SkPath_lineTo'>lineTo</a><sup><a href='#SkPath_lineTo_2'>[2]</a></sup> <a href='#SkPath_addRect'>addRect</a><sup><a href='#SkPath_addRect_2'>[2]</a></sup><sup><a href='#SkPath_addRect_3'>[3]</a></sup>

---

## <a name='Quad'>Quad</a>

<a href='#Quad'>Quad</a> describes a quadratic Bezier

### Example

<div><fiddle-embed name="78ad51fa1cd33eb84a6f99061e56e067"></fiddle-embed></div>

<a href='#Quad'>Quad</a> is a special case of <a href='#Conic'>Conic</a> where <a href='#Conic_Weight'>Conic Weight</a> is set to one

### Example

<div><fiddle-embed name="4082f66a42df11bb20462b232b156bb6"></fiddle-embed></div>

<a name='SkPath_quadTo'></a>
## quadTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds <a href='#Quad'>Quad</a> from <a href='#Last_Point'>Last Point</a> towards

### Parameters

<table>  <tr>    <td><a name='SkPath_quadTo_x1'><code><strong>x1</strong></code></a></td>
    <td>control <a href='SkPoint_Reference#Point'>Point</a> of <a href='#Quad'>Quad</a> in x</td>
  </tr>
  <tr>    <td><a name='SkPath_quadTo_y1'><code><strong>y1</strong></code></a></td>
    <td>control <a href='SkPoint_Reference#Point'>Point</a> of <a href='#Quad'>Quad</a> in y</td>
  </tr>
  <tr>    <td><a name='SkPath_quadTo_x2'><code><strong>x2</strong></code></a></td>
    <td>end <a href='SkPoint_Reference#Point'>Point</a> of <a href='#Quad'>Quad</a> in x</td>
  </tr>
  <tr>    <td><a name='SkPath_quadTo_y2'><code><strong>y2</strong></code></a></td>
    <td>end <a href='SkPoint_Reference#Point'>Point</a> of <a href='#Quad'>Quad</a> in y</td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="60ee3eb747474f5781b0f0dd3a17a866"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a><sup><a href='#SkPath_moveTo_2'>[2]</a></sup> <a href='#SkPath_conicTo'>conicTo</a><sup><a href='#SkPath_conicTo_2'>[2]</a></sup> <a href='#SkPath_rQuadTo'>rQuadTo</a>

---

<a name='SkPath_quadTo_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds <a href='#Quad'>Quad</a> from <a href='#Last_Point'>Last Point</a> towards <a href='SkPoint_Reference#Point'>Point</a> <a href='#SkPath_quadTo_2_p1'>p1</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_quadTo_2_p1'><code><strong>p1</strong></code></a></td>
    <td>control <a href='SkPoint_Reference#Point'>Point</a> of added <a href='#Quad'>Quad</a></td>
  </tr>
  <tr>    <td><a name='SkPath_quadTo_2_p2'><code><strong>p2</strong></code></a></td>
    <td>end <a href='SkPoint_Reference#Point'>Point</a> of added <a href='#Quad'>Quad</a></td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="82621c4df8da1e589d9e627494067826"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a><sup><a href='#SkPath_moveTo_2'>[2]</a></sup> <a href='#SkPath_conicTo'>conicTo</a><sup><a href='#SkPath_conicTo_2'>[2]</a></sup> <a href='#SkPath_rQuadTo'>rQuadTo</a>

---

<a name='SkPath_rQuadTo'></a>
## rQuadTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds <a href='#Quad'>Quad</a> from <a href='#Last_Point'>Last Point</a> towards <a href='SkPoint_Reference#Vector'>Vector</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_rQuadTo_dx1'><code><strong>dx1</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='#Quad'>Quad</a> control on x</td>
  </tr>
  <tr>    <td><a name='SkPath_rQuadTo_dy1'><code><strong>dy1</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='#Quad'>Quad</a> control on y</td>
  </tr>
  <tr>    <td><a name='SkPath_rQuadTo_dx2'><code><strong>dx2</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='#Quad'>Quad</a> end on x</td>
  </tr>
  <tr>    <td><a name='SkPath_rQuadTo_dy2'><code><strong>dy2</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='#Quad'>Quad</a> end on y</td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="1c1f4cdef1c572c9aa8fdf3e461191d0"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a><sup><a href='#SkPath_moveTo_2'>[2]</a></sup> <a href='#SkPath_conicTo'>conicTo</a><sup><a href='#SkPath_conicTo_2'>[2]</a></sup> <a href='#SkPath_quadTo'>quadTo</a><sup><a href='#SkPath_quadTo_2'>[2]</a></sup>

---

## <a name='Conic'>Conic</a>

<a href='#Conic'>Conic</a> describes a conical section

## <a name='Conic_Weight'>Conic Weight</a>

<a href='#Conic_Weight'>Weight</a> determines both the strength of the control <a href='SkPoint_Reference#Point'>Point</a> and the type of <a href='#Conic'>Conic</a>

### Example

<div><fiddle-embed name="2aadded3d20dfef34d1c8abe28c7bc8d"><div>When <a href='#Conic_Weight'>Conic Weight</a> is one</div>

#### Example Output

~~~~
move {0, 0},
quad {0, 0}, {20, 30}, {50, 60},
done
~~~~

</fiddle-embed></div>

If weight is less than one

### Example

<div><fiddle-embed name="e88f554efacfa9f75f270fb1c0add5b4"><div>A 90 degree circular arc has the weight <code>1</code></div>

#### Example Output

~~~~
move {0, 0},
conic {0, 0}, {20, 0}, {20, 20}, weight = 0.707107
done
~~~~

</fiddle-embed></div>

If weight is greater than one

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
## conicTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds <a href='#Conic'>Conic</a> from <a href='#Last_Point'>Last Point</a> towards

### Parameters

<table>  <tr>    <td><a name='SkPath_conicTo_x1'><code><strong>x1</strong></code></a></td>
    <td>control <a href='SkPoint_Reference#Point'>Point</a> of <a href='#Conic'>Conic</a> in x</td>
  </tr>
  <tr>    <td><a name='SkPath_conicTo_y1'><code><strong>y1</strong></code></a></td>
    <td>control <a href='SkPoint_Reference#Point'>Point</a> of <a href='#Conic'>Conic</a> in y</td>
  </tr>
  <tr>    <td><a name='SkPath_conicTo_x2'><code><strong>x2</strong></code></a></td>
    <td>end <a href='SkPoint_Reference#Point'>Point</a> of <a href='#Conic'>Conic</a> in x</td>
  </tr>
  <tr>    <td><a name='SkPath_conicTo_y2'><code><strong>y2</strong></code></a></td>
    <td>end <a href='SkPoint_Reference#Point'>Point</a> of <a href='#Conic'>Conic</a> in y</td>
  </tr>
  <tr>    <td><a name='SkPath_conicTo_w'><code><strong>w</strong></code></a></td>
    <td>weight of added <a href='#Conic'>Conic</a></td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="358d9b6060b528b0923c007420f09c13"><div>As weight increases</div></fiddle-embed></div>

### See Also

<a href='#SkPath_rConicTo'>rConicTo</a> <a href='#SkPath_arcTo'>arcTo</a><sup><a href='#SkPath_arcTo_2'>[2]</a></sup><sup><a href='#SkPath_arcTo_3'>[3]</a></sup><sup><a href='#SkPath_arcTo_4'>[4]</a></sup><sup><a href='#SkPath_arcTo_5'>[5]</a></sup> <a href='#SkPath_addArc'>addArc</a> <a href='#SkPath_quadTo'>quadTo</a><sup><a href='#SkPath_quadTo_2'>[2]</a></sup>

---

<a name='SkPath_conicTo_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds <a href='#Conic'>Conic</a> from <a href='#Last_Point'>Last Point</a> towards <a href='SkPoint_Reference#Point'>Point</a> <a href='#SkPath_conicTo_2_p1'>p1</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_conicTo_2_p1'><code><strong>p1</strong></code></a></td>
    <td>control <a href='SkPoint_Reference#Point'>Point</a> of added <a href='#Conic'>Conic</a></td>
  </tr>
  <tr>    <td><a name='SkPath_conicTo_2_p2'><code><strong>p2</strong></code></a></td>
    <td>end <a href='SkPoint_Reference#Point'>Point</a> of added <a href='#Conic'>Conic</a></td>
  </tr>
  <tr>    <td><a name='SkPath_conicTo_2_w'><code><strong>w</strong></code></a></td>
    <td>weight of added <a href='#Conic'>Conic</a></td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="22d25e03b19d5bae92118877e462361b"><div><a href='#Conic'>Conics</a> and arcs use identical representations</div></fiddle-embed></div>

### See Also

<a href='#SkPath_rConicTo'>rConicTo</a> <a href='#SkPath_arcTo'>arcTo</a><sup><a href='#SkPath_arcTo_2'>[2]</a></sup><sup><a href='#SkPath_arcTo_3'>[3]</a></sup><sup><a href='#SkPath_arcTo_4'>[4]</a></sup><sup><a href='#SkPath_arcTo_5'>[5]</a></sup> <a href='#SkPath_addArc'>addArc</a> <a href='#SkPath_quadTo'>quadTo</a><sup><a href='#SkPath_quadTo_2'>[2]</a></sup>

---

<a name='SkPath_rConicTo'></a>
## rConicTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds <a href='#Conic'>Conic</a> from <a href='#Last_Point'>Last Point</a> towards <a href='SkPoint_Reference#Vector'>Vector</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_rConicTo_dx1'><code><strong>dx1</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='#Conic'>Conic</a> control on x</td>
  </tr>
  <tr>    <td><a name='SkPath_rConicTo_dy1'><code><strong>dy1</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='#Conic'>Conic</a> control on y</td>
  </tr>
  <tr>    <td><a name='SkPath_rConicTo_dx2'><code><strong>dx2</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='#Conic'>Conic</a> end on x</td>
  </tr>
  <tr>    <td><a name='SkPath_rConicTo_dy2'><code><strong>dy2</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='#Conic'>Conic</a> end on y</td>
  </tr>
  <tr>    <td><a name='SkPath_rConicTo_w'><code><strong>w</strong></code></a></td>
    <td>weight of added <a href='#Conic'>Conic</a></td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="3d52763e7c0e20c0b1d484a0afa622d2"></fiddle-embed></div>

### See Also

<a href='#SkPath_conicTo'>conicTo</a><sup><a href='#SkPath_conicTo_2'>[2]</a></sup> <a href='#SkPath_arcTo'>arcTo</a><sup><a href='#SkPath_arcTo_2'>[2]</a></sup><sup><a href='#SkPath_arcTo_3'>[3]</a></sup><sup><a href='#SkPath_arcTo_4'>[4]</a></sup><sup><a href='#SkPath_arcTo_5'>[5]</a></sup> <a href='#SkPath_addArc'>addArc</a> <a href='#SkPath_quadTo'>quadTo</a><sup><a href='#SkPath_quadTo_2'>[2]</a></sup>

---

## <a name='Cubic'>Cubic</a>

<a href='#Cubic'>Cubic</a> describes a <a href='undocumented#Bezier_Curve'>Bezier Curve</a> segment described by a third

### Example

<div><fiddle-embed name="466445ed991d86de08587066392d654a"></fiddle-embed></div>

<a name='SkPath_cubicTo'></a>
## cubicTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds <a href='#Cubic'>Cubic</a> from <a href='#Last_Point'>Last Point</a> towards

### Parameters

<table>  <tr>    <td><a name='SkPath_cubicTo_x1'><code><strong>x1</strong></code></a></td>
    <td>first control <a href='SkPoint_Reference#Point'>Point</a> of <a href='#Cubic'>Cubic</a> in x</td>
  </tr>
  <tr>    <td><a name='SkPath_cubicTo_y1'><code><strong>y1</strong></code></a></td>
    <td>first control <a href='SkPoint_Reference#Point'>Point</a> of <a href='#Cubic'>Cubic</a> in y</td>
  </tr>
  <tr>    <td><a name='SkPath_cubicTo_x2'><code><strong>x2</strong></code></a></td>
    <td>second control <a href='SkPoint_Reference#Point'>Point</a> of <a href='#Cubic'>Cubic</a> in x</td>
  </tr>
  <tr>    <td><a name='SkPath_cubicTo_y2'><code><strong>y2</strong></code></a></td>
    <td>second control <a href='SkPoint_Reference#Point'>Point</a> of <a href='#Cubic'>Cubic</a> in y</td>
  </tr>
  <tr>    <td><a name='SkPath_cubicTo_x3'><code><strong>x3</strong></code></a></td>
    <td>end <a href='SkPoint_Reference#Point'>Point</a> of <a href='#Cubic'>Cubic</a> in x</td>
  </tr>
  <tr>    <td><a name='SkPath_cubicTo_y3'><code><strong>y3</strong></code></a></td>
    <td>end <a href='SkPoint_Reference#Point'>Point</a> of <a href='#Cubic'>Cubic</a> in y</td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="3e476378e3e0550ab134bbaf61112d98"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a><sup><a href='#SkPath_moveTo_2'>[2]</a></sup> <a href='#SkPath_rCubicTo'>rCubicTo</a> <a href='#SkPath_quadTo'>quadTo</a><sup><a href='#SkPath_quadTo_2'>[2]</a></sup>

---

<a name='SkPath_cubicTo_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds <a href='#Cubic'>Cubic</a> from <a href='#Last_Point'>Last Point</a> towards <a href='SkPoint_Reference#Point'>Point</a> <a href='#SkPath_cubicTo_2_p1'>p1</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_cubicTo_2_p1'><code><strong>p1</strong></code></a></td>
    <td>first control <a href='SkPoint_Reference#Point'>Point</a> of <a href='#Cubic'>Cubic</a></td>
  </tr>
  <tr>    <td><a name='SkPath_cubicTo_2_p2'><code><strong>p2</strong></code></a></td>
    <td>second control <a href='SkPoint_Reference#Point'>Point</a> of <a href='#Cubic'>Cubic</a></td>
  </tr>
  <tr>    <td><a name='SkPath_cubicTo_2_p3'><code><strong>p3</strong></code></a></td>
    <td>end <a href='SkPoint_Reference#Point'>Point</a> of <a href='#Cubic'>Cubic</a></td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="d38aaf12c6ff5b8d901a2201bcee5476"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a><sup><a href='#SkPath_moveTo_2'>[2]</a></sup> <a href='#SkPath_rCubicTo'>rCubicTo</a> <a href='#SkPath_quadTo'>quadTo</a><sup><a href='#SkPath_quadTo_2'>[2]</a></sup>

---

<a name='SkPath_rCubicTo'></a>
## rCubicTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds <a href='#Cubic'>Cubic</a> from <a href='#Last_Point'>Last Point</a> towards <a href='SkPoint_Reference#Vector'>Vector</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_rCubicTo_x1'><code><strong>x1</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to first <a href='#Cubic'>Cubic</a> control on x</td>
  </tr>
  <tr>    <td><a name='SkPath_rCubicTo_y1'><code><strong>y1</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to first <a href='#Cubic'>Cubic</a> control on y</td>
  </tr>
  <tr>    <td><a name='SkPath_rCubicTo_x2'><code><strong>x2</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to second <a href='#Cubic'>Cubic</a> control on x</td>
  </tr>
  <tr>    <td><a name='SkPath_rCubicTo_y2'><code><strong>y2</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to second <a href='#Cubic'>Cubic</a> control on y</td>
  </tr>
  <tr>    <td><a name='SkPath_rCubicTo_x3'><code><strong>x3</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='#Cubic'>Cubic</a> end on x</td>
  </tr>
  <tr>    <td><a name='SkPath_rCubicTo_y3'><code><strong>y3</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='#Cubic'>Cubic</a> end on y</td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="19f0cfc7eeba8937fe19446ec0b5f932"></fiddle-embed></div>

### See Also

<a href='SkPath_Overview#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a><sup><a href='#SkPath_moveTo_2'>[2]</a></sup> <a href='#SkPath_cubicTo'>cubicTo</a><sup><a href='#SkPath_cubicTo_2'>[2]</a></sup> <a href='#SkPath_quadTo'>quadTo</a><sup><a href='#SkPath_quadTo_2'>[2]</a></sup>

---

## <a name='Arc'>Arc</a>

<a href='#Arc'>Arc</a> can be constructed in a number of ways![Arc](https://fiddle.skia.org/i/e17e48e9d2182e9afc0f5d26b72c60f0_raster.png "")

<table>  <tr>
    <td></td>
  </tr>  <tr>
    <td></td>
  </tr>  <tr>
    <td></td>
  </tr>  <tr>
    <td></td>
  </tr>  <tr>
    <td></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="5acc77eba0cb4d00bbf3a8f4db0c0aee"></fiddle-embed></div>

In the example above

<table>  <tr>
    <td>1 describes an arc from an oval</td>
  </tr>  <tr>
    <td>2 is similar to 1</td>
  </tr>  <tr>
    <td>3 is similar to 1</td>
  </tr>  <tr>
    <td>4 describes an arc from a pair of tangent lines and a radius</td>
  </tr>  <tr>
    <td>5 describes an arc from <a href='undocumented#Oval'>Oval</a> center</td>
  </tr>  <tr>
    <td>6 describes an arc from a pair of tangent lines and a <a href='#Conic_Weight'>Conic Weight</a></td>
  </tr>
</table>

<a name='SkPath_arcTo'></a>
## arcTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Appends <a href='#Arc'>Arc</a> to <a href='#Path'>Path</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_arcTo_oval'><code><strong>oval</strong></code></a></td>
    <td>bounds of ellipse containing <a href='#Arc'>Arc</a></td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_startAngle'><code><strong>startAngle</strong></code></a></td>
    <td>starting angle of <a href='#Arc'>Arc</a> in degrees</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_sweepAngle'><code><strong>sweepAngle</strong></code></a></td>
    <td>sweep</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_forceMoveTo'><code><strong>forceMoveTo</strong></code></a></td>
    <td>true to start a new contour with <a href='#Arc'>Arc</a></td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="5f02890edaa10cb5e1a4243a82b6a382"><div><a href='#SkPath_arcTo'>arcTo</a> continues a previous contour when <a href='#SkPath_arcTo_forceMoveTo'>forceMoveTo</a> is false and when <a href='#Path'>Path</a>
is not empty</div></fiddle-embed></div>

### See Also

<a href='#SkPath_addArc'>addArc</a> <a href='SkCanvas_Reference#SkCanvas_drawArc'>SkCanvas::drawArc</a> <a href='#SkPath_conicTo'>conicTo</a><sup><a href='#SkPath_conicTo_2'>[2]</a></sup>

---

<a name='SkPath_arcTo_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Appends <a href='#Arc'>Arc</a> to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="d9c6435f26f37b3d63c664a99028f77f"></fiddle-embed></div>

If last <a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> does not start <a href='#Arc'>Arc</a>

### Example

<div><fiddle-embed name="01d2ddfd539ab86a86989e210640dffc"></fiddle-embed></div>

<a href='#Arc'>Arc</a> sweep is always less than 180 degrees

### Parameters

<table>  <tr>    <td><a name='SkPath_arcTo_2_x1'><code><strong>x1</strong></code></a></td>
    <td>x</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_2_y1'><code><strong>y1</strong></code></a></td>
    <td>y</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_2_x2'><code><strong>x2</strong></code></a></td>
    <td>x</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_2_y2'><code><strong>y2</strong></code></a></td>
    <td>y</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_2_radius'><code><strong>radius</strong></code></a></td>
    <td>distance from <a href='#Arc'>Arc</a> to <a href='undocumented#Circle'>Circle</a> center</td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="498360fa0a201cc5db04b1c27256358f"><div><a href='#SkPath_arcTo'>arcTo</a> is represented by <a href='undocumented#Line'>Line</a> and circular <a href='#Conic'>Conic</a> in <a href='#Path'>Path</a></div>

#### Example Output

~~~~
move to (156,20)
line (156,20),(79.2893,20)
conic (79.2893,20),(200,20),(114.645,105.355) weight 0.382683
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_conicTo'>conicTo</a><sup><a href='#SkPath_conicTo_2'>[2]</a></sup>

---

<a name='SkPath_arcTo_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Appends <a href='#Arc'>Arc</a> to <a href='#Path'>Path</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_arcTo_3_p1'><code><strong>p1</strong></code></a></td>
    <td><a href='SkPoint_Reference#Point'>Point</a> common to pair of tangents</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_3_p2'><code><strong>p2</strong></code></a></td>
    <td>end of second tangent</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_3_radius'><code><strong>radius</strong></code></a></td>
    <td>distance from <a href='#Arc'>Arc</a> to <a href='undocumented#Circle'>Circle</a> center</td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="0c056264a361579c18e5d02d3172d4d4"><div>Because tangent lines are parallel</div>

#### Example Output

~~~~
move to (156,20)
line (156,20),(200,20)
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_conicTo'>conicTo</a><sup><a href='#SkPath_conicTo_2'>[2]</a></sup>

---

## <a name='SkPath_ArcSize'>Enum SkPath::ArcSize</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPath_ArcSize'>ArcSize</a> {
        <a href='#SkPath_kSmall_ArcSize'>kSmall_ArcSize</a>,
        <a href='#SkPath_kLarge_ArcSize'>kLarge_ArcSize</a>,
    };
</pre>

Four <a href='undocumented#Oval'>Oval</a> parts with radii

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

<div><fiddle-embed name="8e40c546eecd9cc213200717240898ba"><div><a href='#Arc'>Arc</a> begins at top of <a href='undocumented#Oval'>Oval</a> pair and ends at bottom</div></fiddle-embed></div>

### See Also

<a href='#SkPath_arcTo'>arcTo</a><sup><a href='#SkPath_arcTo_2'>[2]</a></sup><sup><a href='#SkPath_arcTo_3'>[3]</a></sup><sup><a href='#SkPath_arcTo_4'>[4]</a></sup><sup><a href='#SkPath_arcTo_5'>[5]</a></sup> <a href='#SkPath_Direction'>Direction</a>

<a name='SkPath_arcTo_4'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Appends <a href='#Arc'>Arc</a> to <a href='#Path'>Path</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_arcTo_4_rx'><code><strong>rx</strong></code></a></td>
    <td>radius in <a href='#SkPath_arcTo_4_x'>x</a> before <a href='#SkPath_arcTo_4_x'>x</a></td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_4_ry'><code><strong>ry</strong></code></a></td>
    <td>radius in <a href='#SkPath_arcTo_4_y'>y</a> before <a href='#SkPath_arcTo_4_x'>x</a></td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_4_xAxisRotate'><code><strong>xAxisRotate</strong></code></a></td>
    <td><a href='#SkPath_arcTo_4_x'>x</a></td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_4_largeArc'><code><strong>largeArc</strong></code></a></td>
    <td>chooses smaller or larger <a href='#Arc'>Arc</a></td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_4_sweep'><code><strong>sweep</strong></code></a></td>
    <td>chooses clockwise or counterclockwise <a href='#Arc'>Arc</a></td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_4_x'><code><strong>x</strong></code></a></td>
    <td>end of <a href='#Arc'>Arc</a></td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_4_y'><code><strong>y</strong></code></a></td>
    <td>end of <a href='#Arc'>Arc</a></td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="6b6ea44f659b27918f3a6fa621bf6173"></fiddle-embed></div>

### See Also

<a href='#SkPath_rArcTo'>rArcTo</a> <a href='#SkPath_ArcSize'>ArcSize</a> <a href='#SkPath_Direction'>Direction</a>

---

<a name='SkPath_arcTo_5'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Appends <a href='#Arc'>Arc</a> to <a href='#Path'>Path</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_arcTo_5_r'><code><strong>r</strong></code></a></td>
    <td>radii on axes before x</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_5_xAxisRotate'><code><strong>xAxisRotate</strong></code></a></td>
    <td>x</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_5_largeArc'><code><strong>largeArc</strong></code></a></td>
    <td>chooses smaller or larger <a href='#Arc'>Arc</a></td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_5_sweep'><code><strong>sweep</strong></code></a></td>
    <td>chooses clockwise or counterclockwise <a href='#Arc'>Arc</a></td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_5_xy'><code><strong>xy</strong></code></a></td>
    <td>end of <a href='#Arc'>Arc</a></td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="3f76a1007416181a4848c1a87fc81dbd"></fiddle-embed></div>

### See Also

<a href='#SkPath_rArcTo'>rArcTo</a> <a href='#SkPath_ArcSize'>ArcSize</a> <a href='#SkPath_Direction'>Direction</a>

---

<a name='SkPath_rArcTo'></a>
## rArcTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Appends <a href='#Arc'>Arc</a> to <a href='#Path'>Path</a> <code></code>

### Parameters

<table>  <tr>    <td><a name='SkPath_rArcTo_rx'><code><strong>rx</strong></code></a></td>
    <td>radius before x</td>
  </tr>
  <tr>    <td><a name='SkPath_rArcTo_ry'><code><strong>ry</strong></code></a></td>
    <td>radius before x</td>
  </tr>
  <tr>    <td><a name='SkPath_rArcTo_xAxisRotate'><code><strong>xAxisRotate</strong></code></a></td>
    <td>x</td>
  </tr>
  <tr>    <td><a name='SkPath_rArcTo_largeArc'><code><strong>largeArc</strong></code></a></td>
    <td>chooses smaller or larger <a href='#Arc'>Arc</a></td>
  </tr>
  <tr>    <td><a name='SkPath_rArcTo_sweep'><code><strong>sweep</strong></code></a></td>
    <td>chooses clockwise or counterclockwise <a href='#Arc'>Arc</a></td>
  </tr>
  <tr>    <td><a name='SkPath_rArcTo_dx'><code><strong>dx</strong></code></a></td>
    <td>x</td>
  </tr>
  <tr>    <td><a name='SkPath_rArcTo_dy'><code><strong>dy</strong></code></a></td>
    <td>y</td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="3f76a1007416181a4848c1a87fc81dbd"></fiddle-embed></div>

### See Also

<a href='#SkPath_arcTo'>arcTo</a><sup><a href='#SkPath_arcTo_2'>[2]</a></sup><sup><a href='#SkPath_arcTo_3'>[3]</a></sup><sup><a href='#SkPath_arcTo_4'>[4]</a></sup><sup><a href='#SkPath_arcTo_5'>[5]</a></sup> <a href='#SkPath_ArcSize'>ArcSize</a> <a href='#SkPath_Direction'>Direction</a>

---

<a name='SkPath_close'></a>
## close

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Appends <a href='#SkPath_kClose_Verb'>kClose Verb</a> to <a href='#Path'>Path</a>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="9235f6309271d6420fa5c45dc28664c5"></fiddle-embed></div>

### See Also

---

<a name='SkPath_IsInverseFillType'></a>
## IsInverseFillType

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkPath_IsInverseFillType'>IsInverseFillType</a>(<a href='#SkPath_FillType'>FillType</a> fill
</pre>

Returns true if <a href='#SkPath_IsInverseFillType_fill'>fill</a> is inverted and <a href='#Path'>Path</a> with <a href='#SkPath_IsInverseFillType_fill'>fill</a> represents area outside
of its geometric bounds

| <a href='#SkPath_FillType'>FillType</a> | is inverse |
| --- | ---  |
| <a href='#SkPath_kWinding_FillType'>kWinding FillType</a> | false |
| <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd FillType</a> | false |
| <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding FillType</a> | true |
| <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd FillType</a> | true |

### Parameters

<table>  <tr>    <td><a name='SkPath_IsInverseFillType_fill'><code><strong>fill</strong></code></a></td>
    <td>one of</td>
  </tr>
</table>

### Return Value

true if <a href='#Path'>Path</a> fills outside its bounds

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

---

<a name='SkPath_ConvertToNonInverseFillType'></a>
## ConvertToNonInverseFillType

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_ConvertToNonInverseFillType'>ConvertToNonInverseFillType</a>(<a href='#SkPath_FillType'>FillType</a> fill
</pre>

Returns equivalent <a href='#Fill_Type'>Fill Type</a> representing <a href='#Path'>Path</a> <a href='#SkPath_ConvertToNonInverseFillType_fill'>fill</a> inside its bounds

| <a href='#SkPath_FillType'>FillType</a> | inside <a href='#SkPath_FillType'>FillType</a> |
| --- | ---  |
| <a href='#SkPath_kWinding_FillType'>kWinding FillType</a> | <a href='#SkPath_kWinding_FillType'>kWinding FillType</a> |
| <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd FillType</a> | <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd FillType</a> |
| <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding FillType</a> | <a href='#SkPath_kWinding_FillType'>kWinding FillType</a> |
| <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd FillType</a> | <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd FillType</a> |

### Parameters

<table>  <tr>    <td><a name='SkPath_ConvertToNonInverseFillType_fill'><code><strong>fill</strong></code></a></td>
    <td>one of</td>
  </tr>
</table>

### Return Value

<a href='#SkPath_ConvertToNonInverseFillType_fill'>fill</a>

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

---

<a name='SkPath_ConvertConicToQuads'></a>
## ConvertConicToQuads

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static int <a href='#SkPath_ConvertConicToQuads'>ConvertConicToQuads</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>
</pre>

Approximates <a href='#Conic'>Conic</a> with <a href='#Quad'>Quad</a> array <code></code>

### Parameters

<table>  <tr>    <td><a name='SkPath_ConvertConicToQuads_p0'><code><strong>p0</strong></code></a></td>
    <td><a href='#Conic'>Conic</a> start <a href='SkPoint_Reference#Point'>Point</a></td>
  </tr>
  <tr>    <td><a name='SkPath_ConvertConicToQuads_p1'><code><strong>p1</strong></code></a></td>
    <td><a href='#Conic'>Conic</a> control <a href='SkPoint_Reference#Point'>Point</a></td>
  </tr>
  <tr>    <td><a name='SkPath_ConvertConicToQuads_p2'><code><strong>p2</strong></code></a></td>
    <td><a href='#Conic'>Conic</a> end <a href='SkPoint_Reference#Point'>Point</a></td>
  </tr>
  <tr>    <td><a name='SkPath_ConvertConicToQuads_w'><code><strong>w</strong></code></a></td>
    <td><a href='#Conic'>Conic</a> weight</td>
  </tr>
  <tr>    <td><a name='SkPath_ConvertConicToQuads_pts'><code><strong>pts</strong></code></a></td>
    <td>storage for <a href='#Quad'>Quad</a> array</td>
  </tr>
  <tr>    <td><a name='SkPath_ConvertConicToQuads_pow2'><code><strong>pow2</strong></code></a></td>
    <td><a href='#Quad'>Quad</a> count</td>
  </tr>
</table>

### Return Value

number of <a href='#Quad'>Quad</a> curves written to <a href='#SkPath_ConvertConicToQuads_pts'>pts</a>

### Example

<div><fiddle-embed name="3ba94448a4ba48f926e643baeb5b1016"><div>A pair of <a href='#Quad'>Quad</a> curves are drawn in red on top of the elliptical <a href='#Conic'>Conic</a> curve in black</div></fiddle-embed></div>

### See Also

<a href='#Conic'>Conic</a> <a href='#Quad'>Quad</a>

---

<a name='SkPath_isRect'></a>
## isRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isRect'>isRect</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>
</pre>

Returns true if <a href='#Path'>Path</a> is equivalent to <a href='SkRect_Reference#Rect'>Rect</a> when filled

### Parameters

<table>  <tr>    <td><a name='SkPath_isRect_rect'><code><strong>rect</strong></code></a></td>
    <td>storage for bounds of <a href='SkRect_Reference#Rect'>Rect</a></td>
  </tr>
  <tr>    <td><a name='SkPath_isRect_isClosed'><code><strong>isClosed</strong></code></a></td>
    <td>storage set to true if <a href='#Path'>Path</a> is closed</td>
  </tr>
  <tr>    <td><a name='SkPath_isRect_direction'><code><strong>direction</strong></code></a></td>
    <td>storage set to <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkPath_isRect_direction'>direction</a></td>
  </tr>
</table>

### Return Value

true if <a href='#Path'>Path</a> contains <a href='SkRect_Reference#Rect'>Rect</a>

### Example

<div><fiddle-embed name="81a2aac1b8f0ff3d4c8d35ccb9149b16"><div>After <a href='#SkPath_addRect'>addRect</a></div>

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

---

<a name='SkPath_isNestedFillRects'></a>
## isNestedFillRects

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isNestedFillRects'>isNestedFillRects</a>(<a href='SkRect_Reference#SkRect'>SkRect</a> rect
</pre>

Returns true if <a href='#Path'>Path</a> is equivalent to nested <a href='SkRect_Reference#Rect'>Rect</a> pair when filled

### Parameters

<table>  <tr>    <td><a name='SkPath_isNestedFillRects_rect'><code><strong>rect</strong></code></a></td>
    <td>storage for <a href='SkRect_Reference#Rect'>Rect</a> pair</td>
  </tr>
  <tr>    <td><a name='SkPath_isNestedFillRects_dirs'><code><strong>dirs</strong></code></a></td>
    <td>storage for <a href='#SkPath_Direction'>Direction</a> pair</td>
  </tr>
</table>

### Return Value

true if <a href='#Path'>Path</a> contains nested <a href='SkRect_Reference#Rect'>Rect</a> pair

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

---

<a name='SkPath_addRect'></a>
## addRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds <a href='SkRect_Reference#Rect'>Rect</a> to <a href='#Path'>Path</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_addRect_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> to add as a closed contour</td>
  </tr>
  <tr>    <td><a name='SkPath_addRect_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='#SkPath_Direction'>Direction</a> to wind added contour</td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="0f841e4eaebb613b5069800567917c2d"><div>The left <a href='SkRect_Reference#Rect'>Rect</a> dashes starting at the top</div></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas_drawRect'>SkCanvas::drawRect</a> <a href='#SkPath_Direction'>Direction</a>

---

<a name='SkPath_addRect_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds <a href='SkRect_Reference#Rect'>Rect</a> to <a href='#Path'>Path</a>

| <a href='#SkPath_addRect_2_start'>start</a> | first corner |
| --- | ---  |
| 0 | top |
| 1 | top |
| 2 | bottom |
| 3 | bottom |

### Parameters

<table>  <tr>    <td><a name='SkPath_addRect_2_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> to add as a closed contour</td>
  </tr>
  <tr>    <td><a name='SkPath_addRect_2_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='#SkPath_Direction'>Direction</a> to wind added contour</td>
  </tr>
  <tr>    <td><a name='SkPath_addRect_2_start'><code><strong>start</strong></code></a></td>
    <td>initial corner of <a href='SkRect_Reference#Rect'>Rect</a> to add</td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="9202430b3f4f5275af8eec5cc9d7baa8"><div>The arrow is just after the initial corner and points towards the next
corner appended to <a href='#Path'>Path</a></div></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas_drawRect'>SkCanvas::drawRect</a> <a href='#SkPath_Direction'>Direction</a>

---

<a name='SkPath_addRect_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds <a href='SkRect_Reference#Rect'>Rect</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_addRect_3_left'><code><strong>left</strong></code></a></td>
    <td>smaller x</td>
  </tr>
  <tr>    <td><a name='SkPath_addRect_3_top'><code><strong>top</strong></code></a></td>
    <td>smaller y</td>
  </tr>
  <tr>    <td><a name='SkPath_addRect_3_right'><code><strong>right</strong></code></a></td>
    <td>larger x</td>
  </tr>
  <tr>    <td><a name='SkPath_addRect_3_bottom'><code><strong>bottom</strong></code></a></td>
    <td>larger y</td>
  </tr>
  <tr>    <td><a name='SkPath_addRect_3_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='#SkPath_Direction'>Direction</a> to wind added contour</td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="3837827310e8b88b8c2e128ef9fbbd65"><div>The <a href='#SkPath_addRect_3_left'>left</a> <a href='SkRect_Reference#Rect'>Rect</a> dashes start at the <a href='#SkPath_addRect_3_top'>top</a></div></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas_drawRect'>SkCanvas::drawRect</a> <a href='#SkPath_Direction'>Direction</a>

---

<a name='SkPath_addOval'></a>
## addOval

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds <a href='undocumented#Oval'>Oval</a> to path

### Parameters

<table>  <tr>    <td><a name='SkPath_addOval_oval'><code><strong>oval</strong></code></a></td>
    <td>bounds of ellipse added</td>
  </tr>
  <tr>    <td><a name='SkPath_addOval_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='#SkPath_Direction'>Direction</a> to wind ellipse</td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="cac84cf68e63a453c2a8b64c91537704"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas_drawOval'>SkCanvas::drawOval</a> <a href='#SkPath_Direction'>Direction</a> <a href='undocumented#Oval'>Oval</a>

---

<a name='SkPath_addOval_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds <a href='undocumented#Oval'>Oval</a> to <a href='#Path'>Path</a>

| <a href='#SkPath_addOval_2_start'>start</a> | <a href='SkPoint_Reference#Point'>Point</a> |
| --- | ---  |
| 0 | <a href='#SkPath_addOval_2_oval'>oval</a> |
| 1 | <a href='#SkPath_addOval_2_oval'>oval</a> |
| 2 | <a href='#SkPath_addOval_2_oval'>oval</a> |
| 3 | <a href='#SkPath_addOval_2_oval'>oval</a> |

### Parameters

<table>  <tr>    <td><a name='SkPath_addOval_2_oval'><code><strong>oval</strong></code></a></td>
    <td>bounds of ellipse added</td>
  </tr>
  <tr>    <td><a name='SkPath_addOval_2_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='#SkPath_Direction'>Direction</a> to wind ellipse</td>
  </tr>
  <tr>    <td><a name='SkPath_addOval_2_start'><code><strong>start</strong></code></a></td>
    <td>index of initial point of ellipse</td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="ab9753174060e4a551727ef3af12924d"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas_drawOval'>SkCanvas::drawOval</a> <a href='#SkPath_Direction'>Direction</a> <a href='undocumented#Oval'>Oval</a>

---

<a name='SkPath_addCircle'></a>
## addCircle

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds <a href='undocumented#Circle'>Circle</a> centered at <code></code>

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
    <td><a href='#SkPath_Direction'>Direction</a> to wind <a href='undocumented#Circle'>Circle</a></td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="bd5286cb9a5e5c32cd980f72b8f400fb"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas_drawCircle'>SkCanvas::drawCircle</a><sup><a href='SkCanvas_Reference#SkCanvas_drawCircle_2'>[2]</a></sup> <a href='#SkPath_Direction'>Direction</a> <a href='undocumented#Circle'>Circle</a>

---

<a name='SkPath_addArc'></a>
## addArc

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Appends <a href='#Arc'>Arc</a> to <a href='#Path'>Path</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_addArc_oval'><code><strong>oval</strong></code></a></td>
    <td>bounds of ellipse containing <a href='#Arc'>Arc</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addArc_startAngle'><code><strong>startAngle</strong></code></a></td>
    <td>starting angle of <a href='#Arc'>Arc</a> in degrees</td>
  </tr>
  <tr>    <td><a name='SkPath_addArc_sweepAngle'><code><strong>sweepAngle</strong></code></a></td>
    <td>sweep</td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="9cf5122475624e4cf39f06c698f80b1a"><div>The middle row of the left and right columns draw differently from the entries
above and below because <a href='#SkPath_addArc_sweepAngle'>sweepAngle</a> is outside of the range of</div></fiddle-embed></div>

### See Also

<a href='#Arc'>Arc</a> <a href='#SkPath_arcTo'>arcTo</a><sup><a href='#SkPath_arcTo_2'>[2]</a></sup><sup><a href='#SkPath_arcTo_3'>[3]</a></sup><sup><a href='#SkPath_arcTo_4'>[4]</a></sup><sup><a href='#SkPath_arcTo_5'>[5]</a></sup> <a href='SkCanvas_Reference#SkCanvas_drawArc'>SkCanvas::drawArc</a>

---

<a name='SkPath_addRoundRect'></a>
## addRoundRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Appends <a href='SkRRect_Reference#RRect'>Round Rect</a> to <a href='#Path'>Path</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_addRoundRect_rect'><code><strong>rect</strong></code></a></td>
    <td>bounds of <a href='SkRRect_Reference#RRect'>Round Rect</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addRoundRect_rx'><code><strong>rx</strong></code></a></td>
    <td>x</td>
  </tr>
  <tr>    <td><a name='SkPath_addRoundRect_ry'><code><strong>ry</strong></code></a></td>
    <td>y</td>
  </tr>
  <tr>    <td><a name='SkPath_addRoundRect_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='#SkPath_Direction'>Direction</a> to wind <a href='SkRRect_Reference#RRect'>Round Rect</a></td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="24736f685f265cf533f1700c042db353"><div>If either radius is zero</div></fiddle-embed></div>

### See Also

<a href='#SkPath_addRRect'>addRRect</a><sup><a href='#SkPath_addRRect_2'>[2]</a></sup> <a href='SkCanvas_Reference#SkCanvas_drawRoundRect'>SkCanvas::drawRoundRect</a>

---

<a name='SkPath_addRoundRect_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Appends <a href='SkRRect_Reference#RRect'>Round Rect</a> to <a href='#Path'>Path</a>

| <a href='#SkPath_addRoundRect_2_radii'>radii</a> index | location |
| --- | ---  |
| 0 | x |
| 1 | y |
| 2 | x |
| 3 | y |
| 4 | x |
| 5 | y |
| 6 | x |
| 7 | y |

If <a href='#SkPath_addRoundRect_2_dir'>dir</a> is <a href='#SkPath_kCW_Direction'>kCW Direction</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_addRoundRect_2_rect'><code><strong>rect</strong></code></a></td>
    <td>bounds of <a href='SkRRect_Reference#RRect'>Round Rect</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addRoundRect_2_radii'><code><strong>radii</strong></code></a></td>
    <td>array of 8 <a href='undocumented#SkScalar'>SkScalar</a> values</td>
  </tr>
  <tr>    <td><a name='SkPath_addRoundRect_2_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='#SkPath_Direction'>Direction</a> to wind <a href='SkRRect_Reference#RRect'>Round Rect</a></td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="c43d70606b4ee464d2befbcf448c5e73"></fiddle-embed></div>

### See Also

<a href='#SkPath_addRRect'>addRRect</a><sup><a href='#SkPath_addRRect_2'>[2]</a></sup> <a href='SkCanvas_Reference#SkCanvas_drawRoundRect'>SkCanvas::drawRoundRect</a>

---

<a name='SkPath_addRRect'></a>
## addRRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds <a href='#SkPath_addRRect_rrect'>rrect</a> to <a href='#Path'>Path</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_addRRect_rrect'><code><strong>rrect</strong></code></a></td>
    <td>bounds and radii of rounded rectangle</td>
  </tr>
  <tr>    <td><a name='SkPath_addRRect_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='#SkPath_Direction'>Direction</a> to wind <a href='SkRRect_Reference#RRect'>Round Rect</a></td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="d9ecd58081b5bc77a157636fcb345dc6"></fiddle-embed></div>

### See Also

<a href='#SkPath_addRoundRect'>addRoundRect</a><sup><a href='#SkPath_addRoundRect_2'>[2]</a></sup> <a href='SkCanvas_Reference#SkCanvas_drawRRect'>SkCanvas::drawRRect</a>

---

<a name='SkPath_addRRect_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds <a href='#SkPath_addRRect_2_rrect'>rrect</a> to <a href='#Path'>Path</a>

| <a href='#SkPath_addRRect_2_start'>start</a> | location |
| --- | ---  |
| 0 | right of top |
| 1 | left of top |
| 2 | bottom of top |
| 3 | top of bottom |
| 4 | left of bottom |
| 5 | right of bottom |
| 6 | top of bottom |
| 7 | bottom of top |

After appending

### Parameters

<table>  <tr>    <td><a name='SkPath_addRRect_2_rrect'><code><strong>rrect</strong></code></a></td>
    <td>bounds and radii of rounded rectangle</td>
  </tr>
  <tr>    <td><a name='SkPath_addRRect_2_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='#SkPath_Direction'>Direction</a> to wind <a href='SkRRect_Reference#RRect'>Round Rect</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addRRect_2_start'><code><strong>start</strong></code></a></td>
    <td>index of initial point of <a href='SkRRect_Reference#RRect'>Round Rect</a></td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="888edd4c4a91ca62ceb01bce8ab675b2"></fiddle-embed></div>

### See Also

<a href='#SkPath_addRoundRect'>addRoundRect</a><sup><a href='#SkPath_addRoundRect_2'>[2]</a></sup> <a href='SkCanvas_Reference#SkCanvas_drawRRect'>SkCanvas::drawRRect</a>

---

<a name='SkPath_addPoly'></a>
## addPoly

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds <a href='SkPath_Overview#Contour'>Contour</a> created from <a href='undocumented#Line'>Line</a> array

### Parameters

<table>  <tr>    <td><a name='SkPath_addPoly_pts'><code><strong>pts</strong></code></a></td>
    <td>array of <a href='undocumented#Line'>Line</a> sharing end and start <a href='SkPoint_Reference#Point'>Point</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addPoly_count'><code><strong>count</strong></code></a></td>
    <td>length of <a href='SkPoint_Reference#Point'>Point</a> array</td>
  </tr>
  <tr>    <td><a name='SkPath_addPoly_close'><code><strong>close</strong></code></a></td>
    <td>true to add <a href='undocumented#Line'>Line</a> connecting <a href='SkPath_Overview#Contour'>Contour</a> end and start</td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="182b3999772f330f3b0b891b492634ae"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas_drawPoints'>SkCanvas::drawPoints</a>

---

<a name='SkPath_addPoly_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Adds <a href='SkPath_Overview#Contour'>Contour</a> created from <a href='#SkPath_addPoly_2_list'>list</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_addPoly_2_list'><code><strong>list</strong></code></a></td>
    <td>array of <a href='SkPoint_Reference#Point'>Points</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addPoly_2_close'><code><strong>close</strong></code></a></td>
    <td>true to add <a href='undocumented#Line'>Line</a> connecting <a href='SkPath_Overview#Contour'>Contour</a> end and start</td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="1a6b69acad5ceafede3c5984ec6634cb"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas_drawPoints'>SkCanvas::drawPoints</a>

---

## <a name='SkPath_AddPathMode'>Enum SkPath::AddPathMode</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPath_AddPathMode'>AddPathMode</a> {
        <a href='#SkPath_kAppend_AddPathMode'>kAppend_AddPathMode</a>,
        <a href='#SkPath_kExtend_AddPathMode'>kExtend_AddPathMode</a>,
    };
</pre>

<a href='#SkPath_AddPathMode'>AddPathMode</a> chooses how <a href='#SkPath_addPath'>addPath</a> appends

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kAppend_AddPathMode'><code>SkPath::kAppend_AddPathMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#Line # appended to destination unaltered ##</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#Path'>Path</a> <a href='#Verb'>Verbs</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kExtend_AddPathMode'><code>SkPath::kExtend_AddPathMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#Line # add line if prior Contour is not closed ##</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
If destination is closed or empty</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="801b02e74c64aafdb734f2e5cf3e5ab0"><div>test is built from path</div></fiddle-embed></div>

### See Also

<a href='#SkPath_addPath'>addPath</a><sup><a href='#SkPath_addPath_2'>[2]</a></sup><sup><a href='#SkPath_addPath_3'>[3]</a></sup> <a href='#SkPath_reverseAddPath'>reverseAddPath</a>

<a name='SkPath_addPath'></a>
## addPath

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Appends <a href='#SkPath_addPath_src'>src</a> to <a href='#Path'>Path</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_addPath_src'><code><strong>src</strong></code></a></td>
    <td><a href='#Path'>Path</a> <a href='#Verb'>Verbs</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addPath_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#SkPath_addPath_src'>src</a> <a href='#Point_Array'>Point Array</a> x</td>
  </tr>
  <tr>    <td><a name='SkPath_addPath_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#SkPath_addPath_src'>src</a> <a href='#Point_Array'>Point Array</a> y</td>
  </tr>
  <tr>    <td><a name='SkPath_addPath_mode'><code><strong>mode</strong></code></a></td>
    <td><a href='#SkPath_kAppend_AddPathMode'>kAppend AddPathMode</a> or <a href='#SkPath_kExtend_AddPathMode'>kExtend AddPathMode</a></td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="c416bddfe286628974e1c7f0fd66f3f4"></fiddle-embed></div>

### See Also

<a href='#SkPath_AddPathMode'>AddPathMode</a> <a href='#SkPath_offset'>offset</a><sup><a href='#SkPath_offset_2'>[2]</a></sup> <a href='#SkPath_reverseAddPath'>reverseAddPath</a>

---

<a name='SkPath_addPath_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Appends <a href='#SkPath_addPath_2_src'>src</a> to <a href='#Path'>Path</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_addPath_2_src'><code><strong>src</strong></code></a></td>
    <td><a href='#Path'>Path</a> <a href='#Verb'>Verbs</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addPath_2_mode'><code><strong>mode</strong></code></a></td>
    <td><a href='#SkPath_kAppend_AddPathMode'>kAppend AddPathMode</a> or <a href='#SkPath_kExtend_AddPathMode'>kExtend AddPathMode</a></td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="84b2d1c0fc29f1b35e855b6fc6672f9e"></fiddle-embed></div>

### See Also

<a href='#SkPath_AddPathMode'>AddPathMode</a> <a href='#SkPath_reverseAddPath'>reverseAddPath</a>

---

<a name='SkPath_addPath_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Appends <a href='#SkPath_addPath_3_src'>src</a> to <a href='#Path'>Path</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_addPath_3_src'><code><strong>src</strong></code></a></td>
    <td><a href='#Path'>Path</a> <a href='#Verb'>Verbs</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addPath_3_matrix'><code><strong>matrix</strong></code></a></td>
    <td>transform applied to <a href='#SkPath_addPath_3_src'>src</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addPath_3_mode'><code><strong>mode</strong></code></a></td>
    <td><a href='#SkPath_kAppend_AddPathMode'>kAppend AddPathMode</a> or <a href='#SkPath_kExtend_AddPathMode'>kExtend AddPathMode</a></td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="3a90a91030f7289d5df0671d342dbbad"></fiddle-embed></div>

### See Also

<a href='#SkPath_AddPathMode'>AddPathMode</a> <a href='#SkPath_transform'>transform</a><sup><a href='#SkPath_transform_2'>[2]</a></sup> <a href='#SkPath_offset'>offset</a><sup><a href='#SkPath_offset_2'>[2]</a></sup> <a href='#SkPath_reverseAddPath'>reverseAddPath</a>

---

<a name='SkPath_reverseAddPath'></a>
## reverseAddPath

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>
</pre>

Appends <a href='#SkPath_reverseAddPath_src'>src</a> to <a href='#Path'>Path</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_reverseAddPath_src'><code><strong>src</strong></code></a></td>
    <td><a href='#Path'>Path</a> <a href='#Verb'>Verbs</a></td>
  </tr>
</table>

### Return Value

reference to <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="5e8513f073db09acde3ff616f6426e3d"></fiddle-embed></div>

### See Also

<a href='#SkPath_AddPathMode'>AddPathMode</a> <a href='#SkPath_transform'>transform</a><sup><a href='#SkPath_transform_2'>[2]</a></sup> <a href='#SkPath_offset'>offset</a><sup><a href='#SkPath_offset_2'>[2]</a></sup> <a href='#SkPath_addPath'>addPath</a><sup><a href='#SkPath_addPath_2'>[2]</a></sup><sup><a href='#SkPath_addPath_3'>[3]</a></sup>

---

<a name='SkPath_offset'></a>
## offset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_offset'>offset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx
</pre>

Offsets <a href='#Point_Array'>Point Array</a> by

### Parameters

<table>  <tr>    <td><a name='SkPath_offset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#Point_Array'>Point Array</a> x</td>
  </tr>
  <tr>    <td><a name='SkPath_offset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#Point_Array'>Point Array</a> y</td>
  </tr>
  <tr>    <td><a name='SkPath_offset_dst'><code><strong>dst</strong></code></a></td>
    <td>overwritten</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1d1892196ba5bda257df4f3351abd084"></fiddle-embed></div>

### See Also

<a href='#SkPath_addPath'>addPath</a><sup><a href='#SkPath_addPath_2'>[2]</a></sup><sup><a href='#SkPath_addPath_3'>[3]</a></sup> <a href='#SkPath_transform'>transform</a><sup><a href='#SkPath_transform_2'>[2]</a></sup>

---

## <a name='Transform'>Transform</a>

<a name='SkPath_offset_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_offset'>offset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx
</pre>

Offsets <a href='#Point_Array'>Point Array</a> by

### Parameters

<table>  <tr>    <td><a name='SkPath_offset_2_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#Point_Array'>Point Array</a> x</td>
  </tr>
  <tr>    <td><a name='SkPath_offset_2_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#Point_Array'>Point Array</a> y</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="5188d77585715db30bef228f2dfbcccd"></fiddle-embed></div>

### See Also

<a href='#SkPath_addPath'>addPath</a><sup><a href='#SkPath_addPath_2'>[2]</a></sup><sup><a href='#SkPath_addPath_3'>[3]</a></sup> <a href='#SkPath_transform'>transform</a><sup><a href='#SkPath_transform_2'>[2]</a></sup> <a href='SkCanvas_Reference#SkCanvas_translate'>SkCanvas::translate</a>(

---

<a name='SkPath_transform'></a>
## transform

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_transform'>transform</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
</pre>

Transforms <a href='#Verb_Array'>Verb Array</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_transform_matrix'><code><strong>matrix</strong></code></a></td>
    <td><a href='SkMatrix_Reference#Matrix'>Matrix</a> to apply to <a href='#Path'>Path</a></td>
  </tr>
  <tr>    <td><a name='SkPath_transform_dst'><code><strong>dst</strong></code></a></td>
    <td>overwritten</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="99761add116ce3b0730557224c1b0105"></fiddle-embed></div>

### See Also

<a href='#SkPath_addPath'>addPath</a><sup><a href='#SkPath_addPath_2'>[2]</a></sup><sup><a href='#SkPath_addPath_3'>[3]</a></sup> <a href='#SkPath_offset'>offset</a><sup><a href='#SkPath_offset_2'>[2]</a></sup> <a href='SkCanvas_Reference#SkCanvas_concat'>SkCanvas::concat</a>(

---

<a name='SkPath_transform_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_transform'>transform</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
</pre>

Transforms <a href='#Verb_Array'>Verb Array</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_transform_2_matrix'><code><strong>matrix</strong></code></a></td>
    <td><a href='SkMatrix_Reference#Matrix'>Matrix</a> to apply to <a href='#Path'>Path</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c40979a3b92a30cfb7bae36abcc1d805"></fiddle-embed></div>

### See Also

<a href='#SkPath_addPath'>addPath</a><sup><a href='#SkPath_addPath_2'>[2]</a></sup><sup><a href='#SkPath_addPath_3'>[3]</a></sup> <a href='#SkPath_offset'>offset</a><sup><a href='#SkPath_offset_2'>[2]</a></sup> <a href='SkCanvas_Reference#SkCanvas_concat'>SkCanvas::concat</a>(

---

## <a name='Last_Point'>Last Point</a>

<a href='#Path'>Path</a> is defined cumulatively

<a name='SkPath_getLastPt'></a>
## getLastPt

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_getLastPt'>getLastPt</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a>
</pre>

Returns <a href='#Last_Point'>Last Point</a> on <a href='#Path'>Path</a> in <a href='#SkPath_getLastPt_lastPt'>lastPt</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_getLastPt_lastPt'><code><strong>lastPt</strong></code></a></td>
    <td>storage for final <a href='SkPoint_Reference#Point'>Point</a> in <a href='#Point_Array'>Point Array</a></td>
  </tr>
</table>

### Return Value

true if <a href='#Point_Array'>Point Array</a> contains one or more <a href='SkPoint_Reference#Point'>Points</a>

### Example

<div><fiddle-embed name="df8160dd7ac8aa4b40fce7286fe49952">

#### Example Output

~~~~
last point: 35.2786, 52.9772
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_setLastPt'>setLastPt</a><sup><a href='#SkPath_setLastPt_2'>[2]</a></sup>

---

<a name='SkPath_setLastPt'></a>
## setLastPt

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_setLastPt'>setLastPt</a>(<a href='undocumented#SkScalar'>SkScalar</a> x
</pre>

Sets <a href='#Last_Point'>Last Point</a> to

### Parameters

<table>  <tr>    <td><a name='SkPath_setLastPt_x'><code><strong>x</strong></code></a></td>
    <td>set <a href='#SkPath_setLastPt_x'>x</a></td>
  </tr>
  <tr>    <td><a name='SkPath_setLastPt_y'><code><strong>y</strong></code></a></td>
    <td>set <a href='#SkPath_setLastPt_y'>y</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="542c5afaea5f57baa11d0561dd402e18"></fiddle-embed></div>

### See Also

<a href='#SkPath_getLastPt'>getLastPt</a>

---

<a name='SkPath_setLastPt_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_setLastPt'>setLastPt</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>
</pre>

Sets the last point on the path

### Parameters

<table>  <tr>    <td><a name='SkPath_setLastPt_2_p'><code><strong>p</strong></code></a></td>
    <td>set value of <a href='#Last_Point'>Last Point</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6fa5e8f9513b3225e106778592e27e94"></fiddle-embed></div>

### See Also

<a href='#SkPath_getLastPt'>getLastPt</a>

---

## <a name='SkPath_SegmentMask'>Enum SkPath::SegmentMask</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPath_SegmentMask'>SegmentMask</a> {
        <a href='#SkPath_kLine_SegmentMask'>kLine_SegmentMask</a> = 1 << 0,
        <a href='#SkPath_kQuad_SegmentMask'>kQuad_SegmentMask</a> = 1 << 1,
        <a href='#SkPath_kConic_SegmentMask'>kConic_SegmentMask</a> = 1 << 2,
        <a href='#SkPath_kCubic_SegmentMask'>kCubic_SegmentMask</a> = 1 << 3,
    };
</pre>

<a href='#SkPath_SegmentMask'>SegmentMask</a> constants correspond to each drawing <a href='#SkPath_Verb'>Verb</a> type in <a href='#Path'>Path</a>

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kLine_SegmentMask'><code>SkPath::kLine_SegmentMask</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Set if <a href='#Verb_Array'>Verb Array</a> contains <a href='#SkPath_kLine_Verb'>kLine Verb</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kQuad_SegmentMask'><code>SkPath::kQuad_SegmentMask</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Set if <a href='#Verb_Array'>Verb Array</a> contains <a href='#SkPath_kQuad_Verb'>kQuad Verb</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kConic_SegmentMask'><code>SkPath::kConic_SegmentMask</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Set if <a href='#Verb_Array'>Verb Array</a> contains <a href='#SkPath_kConic_Verb'>kConic Verb</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kCubic_SegmentMask'><code>SkPath::kCubic_SegmentMask</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>8</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Set if <a href='#Verb_Array'>Verb Array</a> contains <a href='#SkPath_kCubic_Verb'>kCubic Verb</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a61e5758574e28190ec4ed8c4ae7e7fa"><div>When <a href='#SkPath_conicTo'>conicTo</a> has a weight of one</div>

#### Example Output

~~~~
Path kConic_SegmentMask is clear
Path kQuad_SegmentMask is set
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_getSegmentMasks'>getSegmentMasks</a> <a href='#SkPath_Verb'>Verb</a>

<a name='SkPath_getSegmentMasks'></a>
## getSegmentMasks

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkPath_getSegmentMasks'>getSegmentMasks</a>(
</pre>

Returns a mask

### Return Value

<a href='#SkPath_SegmentMask'>SegmentMask</a> bits or zero

### Example

<div><fiddle-embed name="657a3f3e11acafea92b84d6bb0c13633">

#### Example Output

~~~~
mask quad set
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_getSegmentMasks'>getSegmentMasks</a> <a href='#SkPath_Verb'>Verb</a>

---

<a name='SkPath_contains'></a>
## contains

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_contains'>contains</a>(<a href='undocumented#SkScalar'>SkScalar</a> x
</pre>

Returns true if the point

| <a href='#SkPath_FillType'>FillType</a> | <a href='#SkPath_contains'>contains</a>( |
| --- | ---  |
| <a href='#SkPath_kWinding_FillType'>kWinding FillType</a> | a non |
| <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd FillType</a> | an odd number of <a href='SkPath_Overview#Contour'>Contours</a> |
| <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding FillType</a> | a zero sum of <a href='SkPath_Overview#Contour'>Contour</a> <a href='#Direction'>Directions</a> |
| <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd FillType</a> | and even number of <a href='SkPath_Overview#Contour'>Contours</a> |

### Parameters

<table>  <tr>    <td><a name='SkPath_contains_x'><code><strong>x</strong></code></a></td>
    <td><a href='#SkPath_contains_x'>x</a></td>
  </tr>
  <tr>    <td><a name='SkPath_contains_y'><code><strong>y</strong></code></a></td>
    <td><a href='#SkPath_contains_y'>y</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkPoint_Reference#Point'>Point</a> is in <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="c0216b3f7ebd80b9589ae5728f08fc80"></fiddle-embed></div>

### See Also

<a href='#SkPath_conservativelyContainsRect'>conservativelyContainsRect</a> <a href='#Fill_Type'>Fill Type</a> <a href='undocumented#Op'>Op</a>

---

<a name='SkPath_dump'></a>
## dump

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_dump'>dump</a>(<a href='SkWStream_Reference#SkWStream'>SkWStream</a>
</pre>

Writes text representation of <a href='#Path'>Path</a> to <a href='#SkPath_dump_stream'>stream</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_dump_stream'><code><strong>stream</strong></code></a></td>
    <td>writable <a href='SkWStream_Reference#WStream'>WStream</a> receiving <a href='#Path'>Path</a> text representation</td>
  </tr>
  <tr>    <td><a name='SkPath_dump_forceClose'><code><strong>forceClose</strong></code></a></td>
    <td>true if missing <a href='#SkPath_kClose_Verb'>kClose Verb</a> is output</td>
  </tr>
  <tr>    <td><a name='SkPath_dump_dumpAsHex'><code><strong>dumpAsHex</strong></code></a></td>
    <td>true if <a href='undocumented#SkScalar'>SkScalar</a> values are written as hexadecimal</td>
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

<a href='#SkPath_dumpHex'>dumpHex</a> <a href='SkRect_Reference#SkRect_dump'>SkRect::dump</a><sup><a href='SkRect_Reference#SkRect_dump_2'>[2]</a></sup>(

---

<a name='SkPath_dump_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_dump'>dump</a>(
</pre>

Writes text representation of <a href='#Path'>Path</a> to standard output

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

<a href='#SkPath_dumpHex'>dumpHex</a> <a href='SkRect_Reference#SkRect_dump'>SkRect::dump</a><sup><a href='SkRect_Reference#SkRect_dump_2'>[2]</a></sup>(

---

<a name='SkPath_dumpHex'></a>
## dumpHex

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_dumpHex'>dumpHex</a>(
</pre>

Writes text representation of <a href='#Path'>Path</a> to standard output <a href='https://bug.skia.org'>bug reports against Skia</a></a>

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

<a href='#SkPath_dump'>dump</a><sup><a href='#SkPath_dump_2'>[2]</a></sup> <a href='SkRect_Reference#SkRect_dumpHex'>SkRect::dumpHex</a> <a href='SkRRect_Reference#SkRRect_dumpHex'>SkRRect::dumpHex</a> <a href='#SkPath_writeToMemory'>writeToMemory</a>

---

<a name='SkPath_writeToMemory'></a>
## writeToMemory

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkPath_writeToMemory'>writeToMemory</a>(void
</pre>

Writes <a href='#Path'>Path</a> to <a href='#SkPath_writeToMemory_buffer'>buffer</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_writeToMemory_buffer'><code><strong>buffer</strong></code></a></td>
    <td>storage for <a href='#Path'>Path</a></td>
  </tr>
</table>

### Return Value

size of storage required for <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="e5f16eda6a1c2d759556285f72598445">

#### Example Output

~~~~
path is equal to copy
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_serialize'>serialize</a> <a href='#SkPath_readFromMemory'>readFromMemory</a> <a href='#SkPath_dump'>dump</a><sup><a href='#SkPath_dump_2'>[2]</a></sup> <a href='#SkPath_dumpHex'>dumpHex</a>

---

<a name='SkPath_serialize'></a>
## serialize

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#sk_sp'>sk sp</a>
</pre>

Writes <a href='#Path'>Path</a> to buffer

### Return Value

<a href='#Path'>Path</a> data wrapped in <a href='undocumented#Data'>Data</a> buffer

### Example

<div><fiddle-embed name="2c6aff73608cd198659db6d1eeaaae4f">

#### Example Output

~~~~
path is equal to copy
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkPath_writeToMemory'>writeToMemory</a> <a href='#SkPath_readFromMemory'>readFromMemory</a> <a href='#SkPath_dump'>dump</a><sup><a href='#SkPath_dump_2'>[2]</a></sup> <a href='#SkPath_dumpHex'>dumpHex</a>

---

<a name='SkPath_readFromMemory'></a>
## readFromMemory

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
size_t <a href='#SkPath_readFromMemory'>readFromMemory</a>(const void
</pre>

Initializes <a href='#Path'>Path</a> from <a href='#SkPath_readFromMemory_buffer'>buffer</a> of size <a href='#SkPath_readFromMemory_length'>length</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_readFromMemory_buffer'><code><strong>buffer</strong></code></a></td>
    <td>storage for <a href='#Path'>Path</a></td>
  </tr>
  <tr>    <td><a name='SkPath_readFromMemory_length'><code><strong>length</strong></code></a></td>
    <td><a href='#SkPath_readFromMemory_buffer'>buffer</a> size in bytes</td>
  </tr>
</table>

### Return Value

number of bytes read

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

---

## <a name='Generation_ID'>Generation ID</a>

<a href='#Generation_ID'>Generation ID</a> provides a quick way to check if <a href='#Verb_Array'>Verb Array</a>

<a name='SkPath_getGenerationID'></a>
## getGenerationID

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkPath_getGenerationID'>getGenerationID</a>(
</pre>

Returns a non<a href='#Fill_Type'>Fill Type</a> does affect <a href='#Generation_ID'>Generation ID</a> on Android framework

### Return Value

non

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

---

<a name='SkPath_isValid'></a>
## isValid

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isValid'>isValid</a>(
</pre>

Returns if <a href='#Path'>Path</a> data is consistent

### Return Value

true if <a href='#Path'>Path</a> data is consistent

---

<a name='SkPath_pathRefIsValid'></a>
## pathRefIsValid

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_pathRefIsValid'>pathRefIsValid</a>(
</pre>

To be deprecated soon.

---

<a name='SkPath_Iter'></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
</pre>

Iterates through <a href='#Verb_Array'>Verb Array</a>

### Example

<div><fiddle-embed name="2f53df9201769ab7e7c0e164a1334309"><div>Ignoring the actual <a href='#Verb'>Verbs</a> and replacing them with <a href='#Quad'>Quads</a> rounds the
path of the glyph</div></fiddle-embed></div>

### See Also

<a href='#SkPath_RawIter'>RawIter</a>

<a name='SkPath_Iter_Iter'></a>
## Iter

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_Iter_Iter'>Iter</a>(
</pre>

Initializes <a href='#SkPath_Iter_Iter'>Iter</a> with an empty <a href='#Path'>Path</a>

### Return Value

<a href='#SkPath_Iter_Iter'>Iter</a> of empty <a href='#Path'>Path</a>

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

---

<a name='SkPath_Iter_const_SkPath'></a>
## Iter

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_Iter_Iter'>Iter</a>(const <a href='#SkPath'>SkPath</a>
</pre>

Sets <a href='#SkPath_Iter_Iter'>Iter</a> to return elements of <a href='#Verb_Array'>Verb Array</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_Iter_const_SkPath_path'><code><strong>path</strong></code></a></td>
    <td><a href='#Path'>Path</a> to iterate</td>
  </tr>
  <tr>    <td><a name='SkPath_Iter_const_SkPath_forceClose'><code><strong>forceClose</strong></code></a></td>
    <td>true if open <a href='SkPath_Overview#Contour'>Contours</a> generate <a href='#SkPath_kClose_Verb'>kClose Verb</a></td>
  </tr>
</table>

### Return Value

<a href='#SkPath_Iter_Iter'>Iter</a> of <a href='#SkPath_Iter_const_SkPath_path'>path</a>

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

---

<a name='SkPath_Iter_setPath'></a>
## setPath

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_Iter_setPath'>setPath</a>(const <a href='#SkPath'>SkPath</a>
</pre>

Sets <a href='#SkPath_Iter_Iter'>Iter</a> to return elements of <a href='#Verb_Array'>Verb Array</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_Iter_setPath_path'><code><strong>path</strong></code></a></td>
    <td><a href='#Path'>Path</a> to iterate</td>
  </tr>
  <tr>    <td><a name='SkPath_Iter_setPath_forceClose'><code><strong>forceClose</strong></code></a></td>
    <td>true if open <a href='SkPath_Overview#Contour'>Contours</a> generate <a href='#SkPath_kClose_Verb'>kClose Verb</a></td>
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

---

<a name='SkPath_Iter_next'></a>
## next

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Iter_next'>next</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> pts
</pre>

Returns next <a href='#SkPath_Verb'>Verb</a> in <a href='#Verb_Array'>Verb Array</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_Iter_next_pts'><code><strong>pts</strong></code></a></td>
    <td>storage for <a href='SkPoint_Reference#Point'>Point</a> data describing returned <a href='#SkPath_Verb'>Verb</a></td>
  </tr>
  <tr>    <td><a name='SkPath_Iter_next_doConsumeDegenerates'><code><strong>doConsumeDegenerates</strong></code></a></td>
    <td>if true</td>
  </tr>
  <tr>    <td><a name='SkPath_Iter_next_exact'><code><strong>exact</strong></code></a></td>
    <td>skip zero length curves</td>
  </tr>
</table>

### Return Value

next <a href='#SkPath_Verb'>Verb</a> from <a href='#Verb_Array'>Verb Array</a>

### Example

<div><fiddle-embed name="00ae8984856486bdb626d0ed6587855a"><div>skip degenerate skips the first in a <a href='#SkPath_kMove_Verb'>kMove Verb</a> pair</div>

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

---

<a name='SkPath_Iter_conicWeight'></a>
## conicWeight

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPath_Iter_conicWeight'>conicWeight</a>(
</pre>

Returns <a href='#Conic_Weight'>Conic Weight</a> if <a href='#SkPath_Iter_next'>next</a>(

### Return Value

<a href='#Conic_Weight'>Conic Weight</a> for <a href='#Conic'>Conic</a> <a href='SkPoint_Reference#Point'>Points</a> returned by <a href='#SkPath_Iter_next'>next</a>(

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

<a href='#Conic_Weight'>Conic Weight</a>

---

<a name='SkPath_Iter_isCloseLine'></a>
## isCloseLine

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_Iter_isCloseLine'>isCloseLine</a>(
</pre>

Returns true if last <a href='#SkPath_kLine_Verb'>kLine Verb</a> returned by <a href='#SkPath_Iter_next'>next</a>(

### Return Value

true if last <a href='#SkPath_kLine_Verb'>kLine Verb</a> was generated by <a href='#SkPath_kClose_Verb'>kClose Verb</a>

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

<a href='#SkPath_close'>close</a>(

---

<a name='SkPath_Iter_isClosedContour'></a>
## isClosedContour

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_Iter_isClosedContour'>isClosedContour</a>(
</pre>

Returns true if subsequent calls to <a href='#SkPath_Iter_next'>next</a>(

### Return Value

true if <a href='SkPath_Overview#Contour'>Contour</a> is closed

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

---

<a name='SkPath_RawIter'></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
</pre>

Iterates through <a href='#Verb_Array'>Verb Array</a>

<a name='SkPath_RawIter_RawIter'></a>
## RawIter

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_RawIter_RawIter'>RawIter</a>(
</pre>

Initializes <a href='#SkPath_RawIter_RawIter'>RawIter</a> with an empty <a href='#Path'>Path</a>

### Return Value

<a href='#SkPath_RawIter_RawIter'>RawIter</a> of empty <a href='#Path'>Path</a>

---

<a name='SkPath_RawIter_copy_const_SkPath'></a>
## RawIter

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_RawIter_RawIter'>RawIter</a>(const <a href='#SkPath'>SkPath</a>
</pre>

Sets <a href='#SkPath_RawIter_RawIter'>RawIter</a> to return elements of <a href='#Verb_Array'>Verb Array</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_RawIter_copy_const_SkPath_path'><code><strong>path</strong></code></a></td>
    <td><a href='#Path'>Path</a> to iterate</td>
  </tr>
</table>

### Return Value

<a href='#SkPath_RawIter_RawIter'>RawIter</a> of <a href='#SkPath_RawIter_copy_const_SkPath_path'>path</a>

---

<a name='SkPath_RawIter_setPath'></a>
## setPath

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_RawIter_setPath'>setPath</a>(const <a href='#SkPath'>SkPath</a>
</pre>

Sets <a href='#SkPath_Iter'>SkPath::Iter</a> to return elements of <a href='#Verb_Array'>Verb Array</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_RawIter_setPath_path'><code><strong>path</strong></code></a></td>
    <td><a href='#Path'>Path</a> to iterate</td>
  </tr>
</table>

---

<a name='SkPath_RawIter_next'></a>
## next

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_RawIter_next'>next</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> pts
</pre>

Returns next <a href='#SkPath_Verb'>Verb</a> in <a href='#Verb_Array'>Verb Array</a>

### Parameters

<table>  <tr>    <td><a name='SkPath_RawIter_next_pts'><code><strong>pts</strong></code></a></td>
    <td>storage for <a href='SkPoint_Reference#Point'>Point</a> data describing returned <a href='#SkPath_Verb'>Verb</a></td>
  </tr>
</table>

### Return Value

next <a href='#SkPath_Verb'>Verb</a> from <a href='#Verb_Array'>Verb Array</a>

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

<a href='#SkPath_RawIter_peek'>peek</a>(

---

<a name='SkPath_RawIter_peek'></a>
## peek

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_RawIter_peek'>peek</a>(
</pre>

Returns next <a href='#SkPath_Verb'>Verb</a>

### Return Value

next <a href='#SkPath_Verb'>Verb</a> from <a href='#Verb_Array'>Verb Array</a>

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

StdOut is not really volatile

### See Also

<a href='#SkPath_RawIter_next'>next</a>

---

<a name='SkPath_RawIter_conicWeight'></a>
## conicWeight

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPath_RawIter_conicWeight'>conicWeight</a>(
</pre>

Returns <a href='#Conic_Weight'>Conic Weight</a> if <a href='#SkPath_RawIter_next'>next</a>(

### Return Value

<a href='#Conic_Weight'>Conic Weight</a> for <a href='#Conic'>Conic</a> <a href='SkPoint_Reference#Point'>Points</a> returned by <a href='#SkPath_RawIter_next'>next</a>(

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

<a href='#Conic_Weight'>Conic Weight</a>

---

