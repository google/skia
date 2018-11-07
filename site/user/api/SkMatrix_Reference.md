SkMatrix Reference
===


<a name='SkMatrix'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> {
<a href='SkMatrix_Reference#SkMatrix'>public</a>:
    <a href='SkMatrix_Reference#SkMatrix'>static</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_MakeScale'>MakeScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sy</a>);
    <a href='undocumented#SkScalar'>static</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_MakeScale'>MakeScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>scale</a>);
    <a href='undocumented#SkScalar'>static</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_MakeTrans'>MakeTrans</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>);
    <a href='undocumented#SkScalar'>static</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_MakeAll'>MakeAll</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>scaleX</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>skewX</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>transX</a>,
                     <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>skewY</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>scaleY</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>transY</a>,
                     <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>pers0</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>pers1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>pers2</a>);

    <a href='undocumented#SkScalar'>enum</a> <a href='#SkMatrix_TypeMask'>TypeMask</a> {
        <a href='#SkMatrix_kIdentity_Mask'>kIdentity_Mask</a> = 0,
        <a href='#SkMatrix_kTranslate_Mask'>kTranslate_Mask</a> = 0<a href='#SkMatrix_kTranslate_Mask'>x01</a>,
        <a href='#SkMatrix_kScale_Mask'>kScale_Mask</a> = 0<a href='#SkMatrix_kScale_Mask'>x02</a>,
        <a href='#SkMatrix_kAffine_Mask'>kAffine_Mask</a> = 0<a href='#SkMatrix_kAffine_Mask'>x04</a>,
        <a href='#SkMatrix_kPerspective_Mask'>kPerspective_Mask</a> = 0<a href='#SkMatrix_kPerspective_Mask'>x08</a>,
    };

    <a href='#SkMatrix_TypeMask'>TypeMask</a> <a href='#SkMatrix_getType'>getType</a>() <a href='#SkMatrix_getType'>const</a>;
    <a href='#SkMatrix_getType'>bool</a> <a href='#SkMatrix_isIdentity'>isIdentity</a>() <a href='#SkMatrix_isIdentity'>const</a>;
    <a href='#SkMatrix_isIdentity'>bool</a> <a href='#SkMatrix_isScaleTranslate'>isScaleTranslate</a>() <a href='#SkMatrix_isScaleTranslate'>const</a>;
    <a href='#SkMatrix_isScaleTranslate'>bool</a> <a href='#SkMatrix_isTranslate'>isTranslate</a>() <a href='#SkMatrix_isTranslate'>const</a>;
    <a href='#SkMatrix_isTranslate'>bool</a> <a href='#SkMatrix_rectStaysRect'>rectStaysRect</a>() <a href='#SkMatrix_rectStaysRect'>const</a>;
    <a href='#SkMatrix_rectStaysRect'>bool</a> <a href='#SkMatrix_preservesAxisAlignment'>preservesAxisAlignment</a>() <a href='#SkMatrix_preservesAxisAlignment'>const</a>;
    <a href='#SkMatrix_preservesAxisAlignment'>bool</a> <a href='#SkMatrix_hasPerspective'>hasPerspective</a>() <a href='#SkMatrix_hasPerspective'>const</a>;
    <a href='#SkMatrix_hasPerspective'>bool</a> <a href='#SkMatrix_isSimilarity'>isSimilarity</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>tol</a> = <a href='undocumented#SK_ScalarNearlyZero'>SK_ScalarNearlyZero</a>) <a href='undocumented#SK_ScalarNearlyZero'>const</a>;
    <a href='undocumented#SK_ScalarNearlyZero'>bool</a> <a href='#SkMatrix_preservesRightAngles'>preservesRightAngles</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>tol</a> = <a href='undocumented#SK_ScalarNearlyZero'>SK_ScalarNearlyZero</a>) <a href='undocumented#SK_ScalarNearlyZero'>const</a>;

    <a href='undocumented#SK_ScalarNearlyZero'>static</a> <a href='undocumented#SK_ScalarNearlyZero'>constexpr</a> <a href='undocumented#SK_ScalarNearlyZero'>int</a> <a href='#SkMatrix_kMScaleX'>kMScaleX</a> = 0;
    <a href='#SkMatrix_kMScaleX'>static</a> <a href='#SkMatrix_kMScaleX'>constexpr</a> <a href='#SkMatrix_kMScaleX'>int</a> <a href='#SkMatrix_kMSkewX'>kMSkewX</a> = 1;
    <a href='#SkMatrix_kMSkewX'>static</a> <a href='#SkMatrix_kMSkewX'>constexpr</a> <a href='#SkMatrix_kMSkewX'>int</a> <a href='#SkMatrix_kMTransX'>kMTransX</a> = 2;
    <a href='#SkMatrix_kMTransX'>static</a> <a href='#SkMatrix_kMTransX'>constexpr</a> <a href='#SkMatrix_kMTransX'>int</a> <a href='#SkMatrix_kMSkewY'>kMSkewY</a> = 3;
    <a href='#SkMatrix_kMSkewY'>static</a> <a href='#SkMatrix_kMSkewY'>constexpr</a> <a href='#SkMatrix_kMSkewY'>int</a> <a href='#SkMatrix_kMScaleY'>kMScaleY</a> = 4;
    <a href='#SkMatrix_kMScaleY'>static</a> <a href='#SkMatrix_kMScaleY'>constexpr</a> <a href='#SkMatrix_kMScaleY'>int</a> <a href='#SkMatrix_kMTransY'>kMTransY</a> = 5;
    <a href='#SkMatrix_kMTransY'>static</a> <a href='#SkMatrix_kMTransY'>constexpr</a> <a href='#SkMatrix_kMTransY'>int</a> <a href='#SkMatrix_kMPersp0'>kMPersp0</a> = 6;
    <a href='#SkMatrix_kMPersp0'>static</a> <a href='#SkMatrix_kMPersp0'>constexpr</a> <a href='#SkMatrix_kMPersp0'>int</a> <a href='#SkMatrix_kMPersp1'>kMPersp1</a> = 7;
    <a href='#SkMatrix_kMPersp1'>static</a> <a href='#SkMatrix_kMPersp1'>constexpr</a> <a href='#SkMatrix_kMPersp1'>int</a> <a href='#SkMatrix_kMPersp2'>kMPersp2</a> = 8;
    <a href='#SkMatrix_kMPersp2'>static</a> <a href='#SkMatrix_kMPersp2'>constexpr</a> <a href='#SkMatrix_kMPersp2'>int</a> <a href='#SkMatrix_kAScaleX'>kAScaleX</a> = 0;
    <a href='#SkMatrix_kAScaleX'>static</a> <a href='#SkMatrix_kAScaleX'>constexpr</a> <a href='#SkMatrix_kAScaleX'>int</a> <a href='#SkMatrix_kASkewY'>kASkewY</a> = 1;
    <a href='#SkMatrix_kASkewY'>static</a> <a href='#SkMatrix_kASkewY'>constexpr</a> <a href='#SkMatrix_kASkewY'>int</a> <a href='#SkMatrix_kASkewX'>kASkewX</a> = 2;
    <a href='#SkMatrix_kASkewX'>static</a> <a href='#SkMatrix_kASkewX'>constexpr</a> <a href='#SkMatrix_kASkewX'>int</a> <a href='#SkMatrix_kAScaleY'>kAScaleY</a> = 3;
    <a href='#SkMatrix_kAScaleY'>static</a> <a href='#SkMatrix_kAScaleY'>constexpr</a> <a href='#SkMatrix_kAScaleY'>int</a> <a href='#SkMatrix_kATransX'>kATransX</a> = 4;
    <a href='#SkMatrix_kATransX'>static</a> <a href='#SkMatrix_kATransX'>constexpr</a> <a href='#SkMatrix_kATransX'>int</a> <a href='#SkMatrix_kATransY'>kATransY</a> = 5;

    <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>operator</a>[](<a href='undocumented#SkScalar'>int</a> <a href='undocumented#SkScalar'>index</a>) <a href='undocumented#SkScalar'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>get</a>(<a href='undocumented#SkScalar'>int</a> <a href='undocumented#SkScalar'>index</a>) <a href='undocumented#SkScalar'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getScaleX'>getScaleX</a>() <a href='#SkMatrix_getScaleX'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getScaleY'>getScaleY</a>() <a href='#SkMatrix_getScaleY'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getSkewY'>getSkewY</a>() <a href='#SkMatrix_getSkewY'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getSkewX'>getSkewX</a>() <a href='#SkMatrix_getSkewX'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getTranslateX'>getTranslateX</a>() <a href='#SkMatrix_getTranslateX'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getTranslateY'>getTranslateY</a>() <a href='#SkMatrix_getTranslateY'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getPerspX'>getPerspX</a>() <a href='#SkMatrix_getPerspX'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getPerspY'>getPerspY</a>() <a href='#SkMatrix_getPerspY'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a>& <a href='undocumented#SkScalar'>operator</a>[](<a href='undocumented#SkScalar'>int</a> <a href='undocumented#SkScalar'>index</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='undocumented#SkScalar'>set</a>(<a href='undocumented#SkScalar'>int</a> <a href='undocumented#SkScalar'>index</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>value</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_setScaleX'>setScaleX</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>v</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_setScaleY'>setScaleY</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>v</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_setSkewY'>setSkewY</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>v</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_setSkewX'>setSkewX</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>v</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_setTranslateX'>setTranslateX</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>v</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_setTranslateY'>setTranslateY</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>v</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_setPerspX'>setPerspX</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>v</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_setPerspY'>setPerspY</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>v</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_setAll'>setAll</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>scaleX</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>skewX</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>transX</a>,
                <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>skewY</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>scaleY</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>transY</a>,
                <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>persp0</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>persp1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>persp2</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_get9'>get9</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>buffer</a>[9]) <a href='undocumented#SkScalar'>const</a>;
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_set9'>set9</a>(<a href='#SkMatrix_set9'>const</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>buffer</a>[9]);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_reset'>reset()</a>;
    <a href='#SkMatrix_reset'>void</a> <a href='#SkMatrix_setIdentity'>setIdentity</a>();
    <a href='#SkMatrix_setIdentity'>void</a> <a href='#SkMatrix_setTranslate'>setTranslate</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_setTranslate'>setTranslate</a>(<a href='#SkMatrix_setTranslate'>const</a> <a href='SkPoint_Reference#SkVector'>SkVector</a>& <a href='SkPoint_Reference#SkVector'>v</a>);
    <a href='SkPoint_Reference#SkVector'>void</a> <a href='#SkMatrix_setScale'>setScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sy</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>px</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>py</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_setScale'>setScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sy</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_setRotate'>setRotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>degrees</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>px</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>py</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_setRotate'>setRotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>degrees</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_setSinCos'>setSinCos</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sinValue</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>cosValue</a>,
                   <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>px</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>py</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_setSinCos'>setSinCos</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sinValue</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>cosValue</a>);
    <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='#SkMatrix_setRSXform'>setRSXform</a>(<a href='#SkMatrix_setRSXform'>const</a> <a href='undocumented#SkRSXform'>SkRSXform</a>& <a href='undocumented#SkRSXform'>rsxForm</a>);
    <a href='undocumented#SkRSXform'>void</a> <a href='#SkMatrix_setSkew'>setSkew</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>kx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>ky</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>px</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>py</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_setSkew'>setSkew</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>kx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>ky</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_setConcat'>setConcat</a>(<a href='#SkMatrix_setConcat'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#SkMatrix'>a</a>, <a href='SkMatrix_Reference#SkMatrix'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#SkMatrix'>b</a>);
    <a href='SkMatrix_Reference#SkMatrix'>void</a> <a href='#SkMatrix_preTranslate'>preTranslate</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_preScale'>preScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sy</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>px</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>py</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_preScale'>preScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sy</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_preRotate'>preRotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>degrees</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>px</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>py</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_preRotate'>preRotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>degrees</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_preSkew'>preSkew</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>kx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>ky</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>px</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>py</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_preSkew'>preSkew</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>kx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>ky</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_preConcat'>preConcat</a>(<a href='#SkMatrix_preConcat'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#SkMatrix'>other</a>);
    <a href='SkMatrix_Reference#SkMatrix'>void</a> <a href='#SkMatrix_postTranslate'>postTranslate</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_postScale'>postScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sy</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>px</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>py</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_postScale'>postScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sy</a>);
    <a href='undocumented#SkScalar'>bool</a> <a href='#SkMatrix_postIDiv'>postIDiv</a>(<a href='#SkMatrix_postIDiv'>int</a> <a href='#SkMatrix_postIDiv'>divx</a>, <a href='#SkMatrix_postIDiv'>int</a> <a href='#SkMatrix_postIDiv'>divy</a>);
    <a href='#SkMatrix_postIDiv'>void</a> <a href='#SkMatrix_postRotate'>postRotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>degrees</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>px</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>py</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_postRotate'>postRotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>degrees</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_postSkew'>postSkew</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>kx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>ky</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>px</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>py</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_postSkew'>postSkew</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>kx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>ky</a>);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_postConcat'>postConcat</a>(<a href='#SkMatrix_postConcat'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#SkMatrix'>other</a>);

    <a href='SkMatrix_Reference#SkMatrix'>enum</a> <a href='#SkMatrix_ScaleToFit'>ScaleToFit</a> {
        <a href='#SkMatrix_kFill_ScaleToFit'>kFill_ScaleToFit</a>,
        <a href='#SkMatrix_kStart_ScaleToFit'>kStart_ScaleToFit</a>,
        <a href='#SkMatrix_kCenter_ScaleToFit'>kCenter_ScaleToFit</a>,
        <a href='#SkMatrix_kEnd_ScaleToFit'>kEnd_ScaleToFit</a>,
    };

    <a href='#SkMatrix_kEnd_ScaleToFit'>bool</a> <a href='#SkMatrix_setRectToRect'>setRectToRect</a>(<a href='#SkMatrix_setRectToRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>src</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>, <a href='#SkMatrix_ScaleToFit'>ScaleToFit</a> <a href='#SkMatrix_ScaleToFit'>stf</a>);
    <a href='#SkMatrix_ScaleToFit'>static</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_MakeRectToRect'>MakeRectToRect</a>(<a href='#SkMatrix_MakeRectToRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>src</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>, <a href='#SkMatrix_ScaleToFit'>ScaleToFit</a> <a href='#SkMatrix_ScaleToFit'>stf</a>);
    <a href='#SkMatrix_ScaleToFit'>bool</a> <a href='#SkMatrix_setPolyToPoly'>setPolyToPoly</a>(<a href='#SkMatrix_setPolyToPoly'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>src</a>[], <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>dst</a>[], <a href='SkPoint_Reference#SkPoint'>int</a> <a href='SkPoint_Reference#SkPoint'>count</a>);
    <a href='SkPoint_Reference#SkPoint'>bool</a> <a href='#SkMatrix_invert'>invert</a>(<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#SkMatrix'>inverse</a>) <a href='SkMatrix_Reference#SkMatrix'>const</a>;
    <a href='SkMatrix_Reference#SkMatrix'>static</a> <a href='SkMatrix_Reference#SkMatrix'>void</a> <a href='#SkMatrix_SetAffineIdentity'>SetAffineIdentity</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>affine</a>[6]);
    <a href='undocumented#SkScalar'>bool</a> <a href='#SkMatrix_asAffine'>asAffine</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>affine</a>[6]) <a href='undocumented#SkScalar'>const</a>;
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_setAffine'>setAffine</a>(<a href='#SkMatrix_setAffine'>const</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>affine</a>[6]);
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_mapPoints'>mapPoints</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>dst</a>[], <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>src</a>[], <a href='SkPoint_Reference#SkPoint'>int</a> <a href='SkPoint_Reference#SkPoint'>count</a>) <a href='SkPoint_Reference#SkPoint'>const</a>;
    <a href='SkPoint_Reference#SkPoint'>void</a> <a href='#SkMatrix_mapPoints'>mapPoints</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pts</a>[], <a href='SkPoint_Reference#SkPoint'>int</a> <a href='SkPoint_Reference#SkPoint'>count</a>) <a href='SkPoint_Reference#SkPoint'>const</a>;
    <a href='SkPoint_Reference#SkPoint'>void</a> <a href='#SkMatrix_mapHomogeneousPoints'>mapHomogeneousPoints</a>(<a href='undocumented#SkPoint3'>SkPoint3</a> <a href='undocumented#SkPoint3'>dst</a>[], <a href='undocumented#SkPoint3'>const</a> <a href='undocumented#SkPoint3'>SkPoint3</a> <a href='undocumented#SkPoint3'>src</a>[], <a href='undocumented#SkPoint3'>int</a> <a href='undocumented#SkPoint3'>count</a>) <a href='undocumented#SkPoint3'>const</a>;
    <a href='undocumented#SkPoint3'>void</a> <a href='#SkMatrix_mapXY'>mapXY</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>, <a href='SkPoint_Reference#SkPoint'>SkPoint</a>* <a href='SkPoint_Reference#SkPoint'>result</a>) <a href='SkPoint_Reference#SkPoint'>const</a>;
    <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkMatrix_mapXY'>mapXY</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>) <a href='undocumented#SkScalar'>const</a>;
    <a href='undocumented#SkScalar'>void</a> <a href='#SkMatrix_mapVectors'>mapVectors</a>(<a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='SkPoint_Reference#SkVector'>dst</a>[], <a href='SkPoint_Reference#SkVector'>const</a> <a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='SkPoint_Reference#SkVector'>src</a>[], <a href='SkPoint_Reference#SkVector'>int</a> <a href='SkPoint_Reference#SkVector'>count</a>) <a href='SkPoint_Reference#SkVector'>const</a>;
    <a href='SkPoint_Reference#SkVector'>void</a> <a href='#SkMatrix_mapVectors'>mapVectors</a>(<a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='SkPoint_Reference#SkVector'>vecs</a>[], <a href='SkPoint_Reference#SkVector'>int</a> <a href='SkPoint_Reference#SkVector'>count</a>) <a href='SkPoint_Reference#SkVector'>const</a>;
    <a href='SkPoint_Reference#SkVector'>void</a> <a href='#SkMatrix_mapVector'>mapVector</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>, <a href='SkPoint_Reference#SkVector'>SkVector</a>* <a href='SkPoint_Reference#SkVector'>result</a>) <a href='SkPoint_Reference#SkVector'>const</a>;
    <a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='#SkMatrix_mapVector'>mapVector</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>) <a href='undocumented#SkScalar'>const</a>;
    <a href='undocumented#SkScalar'>bool</a> <a href='#SkMatrix_mapRect'>mapRect</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>dst</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>src</a>) <a href='SkRect_Reference#SkRect'>const</a>;
    <a href='SkRect_Reference#SkRect'>bool</a> <a href='#SkMatrix_mapRect'>mapRect</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#Rect'>rect</a>) <a href='SkRect_Reference#Rect'>const</a>;
    <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkMatrix_mapRect'>mapRect</a>(<a href='#SkMatrix_mapRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>src</a>) <a href='SkRect_Reference#SkRect'>const</a>;
    <a href='SkRect_Reference#SkRect'>void</a> <a href='#SkMatrix_mapRectToQuad'>mapRectToQuad</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>dst</a>[4], <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>) <a href='SkRect_Reference#Rect'>const</a>;
    <a href='SkRect_Reference#Rect'>void</a> <a href='#SkMatrix_mapRectScaleTranslate'>mapRectScaleTranslate</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>dst</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>src</a>) <a href='SkRect_Reference#SkRect'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_mapRadius'>mapRadius</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>radius</a>) <a href='undocumented#SkScalar'>const</a>;
    <a href='undocumented#SkScalar'>bool</a> <a href='#SkMatrix_isFixedStepInX'>isFixedStepInX</a>() <a href='#SkMatrix_isFixedStepInX'>const</a>;
    <a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='#SkMatrix_fixedStepInX'>fixedStepInX</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>) <a href='undocumented#SkScalar'>const</a>;
    <a href='undocumented#SkScalar'>bool</a> <a href='#SkMatrix_cheapEqualTo'>cheapEqualTo</a>(<a href='#SkMatrix_cheapEqualTo'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#SkMatrix'>m</a>) <a href='SkMatrix_Reference#SkMatrix'>const</a>;
    <a href='SkMatrix_Reference#SkMatrix'>friend</a> <a href='SkMatrix_Reference#SkMatrix'>bool</a> <a href='SkMatrix_Reference#SkMatrix'>operator</a>==(<a href='SkMatrix_Reference#SkMatrix'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#SkMatrix'>a</a>, <a href='SkMatrix_Reference#SkMatrix'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#SkMatrix'>b</a>);
    <a href='SkMatrix_Reference#SkMatrix'>friend</a> <a href='SkMatrix_Reference#SkMatrix'>bool</a> <a href='SkMatrix_Reference#SkMatrix'>operator</a>!=(<a href='SkMatrix_Reference#SkMatrix'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#SkMatrix'>a</a>, <a href='SkMatrix_Reference#SkMatrix'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#SkMatrix'>b</a>);
    <a href='SkMatrix_Reference#SkMatrix'>void</a> <a href='#SkMatrix_dump'>dump()</a> <a href='#SkMatrix_dump'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getMinScale'>getMinScale</a>() <a href='#SkMatrix_getMinScale'>const</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getMaxScale'>getMaxScale</a>() <a href='#SkMatrix_getMaxScale'>const</a>;
    <a href='#SkMatrix_getMaxScale'>bool</a> <a href='#SkMatrix_getMinMaxScales'>getMinMaxScales</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>scaleFactors</a>[2]) <a href='undocumented#SkScalar'>const</a>;
    <a href='undocumented#SkScalar'>bool</a> <a href='#SkMatrix_decomposeScale'>decomposeScale</a>(<a href='undocumented#SkSize'>SkSize</a>* <a href='undocumented#SkSize'>scale</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#SkMatrix'>remaining</a> = <a href='SkMatrix_Reference#SkMatrix'>nullptr</a>) <a href='SkMatrix_Reference#SkMatrix'>const</a>;
    <a href='SkMatrix_Reference#SkMatrix'>static</a> <a href='SkMatrix_Reference#SkMatrix'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='#SkMatrix_I'>I</a>();
    <a href='#SkMatrix_I'>static</a> <a href='#SkMatrix_I'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='#SkMatrix_InvalidMatrix'>InvalidMatrix</a>();
    <a href='#SkMatrix_InvalidMatrix'>static</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_Concat'>Concat</a>(<a href='#SkMatrix_Concat'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#SkMatrix'>a</a>, <a href='SkMatrix_Reference#SkMatrix'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#SkMatrix'>b</a>);
    <a href='SkMatrix_Reference#SkMatrix'>void</a> <a href='#SkMatrix_dirtyMatrixTypeCache'>dirtyMatrixTypeCache</a>();
    <a href='#SkMatrix_dirtyMatrixTypeCache'>void</a> <a href='#SkMatrix_setScaleTranslate'>setScaleTranslate</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sy</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>tx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>ty</a>);
    <a href='undocumented#SkScalar'>bool</a> <a href='#SkMatrix_isFinite'>isFinite</a>() <a href='#SkMatrix_isFinite'>const</a>;
};
</pre>

<a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>holds</a> <a href='SkMatrix_Reference#Matrix'>a</a> 3 <a href='SkMatrix_Reference#Matrix'>by</a> 3 <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>for</a> <a href='SkMatrix_Reference#Matrix'>transforming</a> <a href='SkMatrix_Reference#Matrix'>coordinates</a>. <a href='SkMatrix_Reference#Matrix'>This</a> <a href='SkMatrix_Reference#Matrix'>allows</a> <a href='SkMatrix_Reference#Matrix'>mapping</a>
<a href='SkPoint_Reference#Point'>Points</a> <a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Vector'>Vectors</a> <a href='SkPoint_Reference#Vector'>with</a> <a href='SkPoint_Reference#Vector'>translation</a>, <a href='SkPoint_Reference#Vector'>scaling</a>, <a href='SkPoint_Reference#Vector'>skewing</a>, <a href='SkPoint_Reference#Vector'>rotation</a>, <a href='SkPoint_Reference#Vector'>and</a>
<a href='SkPoint_Reference#Vector'>perspective</a>.

<a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>elements</a> <a href='SkMatrix_Reference#Matrix'>are</a> <a href='SkMatrix_Reference#Matrix'>in</a> <a href='SkMatrix_Reference#Matrix'>row</a> <a href='SkMatrix_Reference#Matrix'>major</a> <a href='SkMatrix_Reference#Matrix'>order</a>. <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>does</a> <a href='SkMatrix_Reference#Matrix'>not</a> <a href='SkMatrix_Reference#Matrix'>have</a> <a href='SkMatrix_Reference#Matrix'>a</a> <a href='SkMatrix_Reference#Matrix'>constructor</a>,
<a href='SkMatrix_Reference#Matrix'>so</a> <a href='SkMatrix_Reference#Matrix'>it</a> <a href='SkMatrix_Reference#Matrix'>must</a> <a href='SkMatrix_Reference#Matrix'>be</a> <a href='SkMatrix_Reference#Matrix'>explicitly</a> <a href='SkMatrix_Reference#Matrix'>initialized</a>. <a href='#SkMatrix_setIdentity'>setIdentity</a> <a href='#SkMatrix_setIdentity'>initializes</a> <a href='SkMatrix_Reference#Matrix'>Matrix</a>
<a href='SkMatrix_Reference#Matrix'>so</a> <a href='SkMatrix_Reference#Matrix'>it</a> <a href='SkMatrix_Reference#Matrix'>has</a> <a href='SkMatrix_Reference#Matrix'>no</a> <a href='SkMatrix_Reference#Matrix'>effect</a>. <a href='#SkMatrix_setTranslate'>setTranslate</a>, <a href='#SkMatrix_setScale'>setScale</a>, <a href='#SkMatrix_setSkew'>setSkew</a>, <a href='#SkMatrix_setRotate'>setRotate</a>, <a href='#SkMatrix_set9'>set9</a> <a href='#SkMatrix_set9'>and</a> <a href='#SkMatrix_setAll'>setAll</a>
<a href='#SkMatrix_setAll'>initializes</a> <a href='#SkMatrix_setAll'>all</a> <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>elements</a> <a href='SkMatrix_Reference#Matrix'>with</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>corresponding</a> <a href='SkMatrix_Reference#Matrix'>mapping</a>.

<a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>includes</a> <a href='SkMatrix_Reference#Matrix'>a</a> <a href='SkMatrix_Reference#Matrix'>hidden</a> <a href='SkMatrix_Reference#Matrix'>variable</a> <a href='SkMatrix_Reference#Matrix'>that</a> <a href='SkMatrix_Reference#Matrix'>classifies</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>type</a> <a href='SkMatrix_Reference#Matrix'>of</a> <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>to</a>
<a href='SkMatrix_Reference#Matrix'>improve</a> <a href='SkMatrix_Reference#Matrix'>performance</a>. <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>is</a> <a href='SkMatrix_Reference#Matrix'>not</a> <a href='SkMatrix_Reference#Matrix'>thread</a> <a href='SkMatrix_Reference#Matrix'>safe</a> <a href='SkMatrix_Reference#Matrix'>unless</a> <a href='#SkMatrix_getType'>getType</a> <a href='#SkMatrix_getType'>is</a> <a href='#SkMatrix_getType'>called</a> <a href='#SkMatrix_getType'>first</a>.

<a name='SkMatrix_MakeScale'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_MakeScale'>MakeScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sy</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>scale</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> (<a href='#SkMatrix_MakeScale_sx'>sx</a>, <a href='#SkMatrix_MakeScale_sy'>sy</a>). <a href='#SkMatrix_MakeScale_sy'>Returned</a> <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>is</a>:

| <a href='#SkMatrix_MakeScale_sx'>sx</a>  0  0 |
|  0 <a href='#SkMatrix_MakeScale_sy'>sy</a>  0 |
|  0  0  1 |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_MakeScale_sx'><code><strong>sx</strong></code></a></td>
    <td>horizontal scale factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_MakeScale_sy'><code><strong>sy</strong></code></a></td>
    <td>vertical scale factor</td>
  </tr>
</table>

### Return Value

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>with</a> <a href='SkMatrix_Reference#SkMatrix'>scale</a>

### Example

<div><fiddle-embed name="7ff17718111df6d6f95381d8a8f1b389"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setScale'>setScale</a> <a href='#SkMatrix_postScale'>postScale</a> <a href='#SkMatrix_preScale'>preScale</a>

<a name='SkMatrix_MakeScale_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_MakeScale'>MakeScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>scale</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='#SkMatrix_MakeScale_2_scale'>scale</a> <a href='#SkMatrix_MakeScale_2_scale'>by</a> (<a href='#SkMatrix_MakeScale_2_scale'>scale</a>, <a href='#SkMatrix_MakeScale_2_scale'>scale</a>). <a href='#SkMatrix_MakeScale_2_scale'>Returned</a> <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>is</a>:

| <a href='#SkMatrix_MakeScale_2_scale'>scale</a>   0   0 |
|   0   <a href='#SkMatrix_MakeScale_2_scale'>scale</a> 0 |
|   0     0   1 |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_MakeScale_2_scale'><code><strong>scale</strong></code></a></td>
    <td>horizontal and vertical <a href='#SkMatrix_MakeScale_2_scale'>scale</a> <a href='#SkMatrix_MakeScale_2_scale'>factor</a></td>
  </tr>
</table>

### Return Value

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>with</a> <a href='#SkMatrix_MakeScale_2_scale'>scale</a>

### Example

<div><fiddle-embed name="2956aeb50fa862cdb13995e1e56a4bc8"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setScale'>setScale</a> <a href='#SkMatrix_postScale'>postScale</a> <a href='#SkMatrix_preScale'>preScale</a>

<a name='SkMatrix_MakeTrans'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_MakeTrans'>MakeTrans</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>translate</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> (<a href='#SkMatrix_MakeTrans_dx'>dx</a>, <a href='#SkMatrix_MakeTrans_dy'>dy</a>). <a href='#SkMatrix_MakeTrans_dy'>Returned</a> <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>is</a>:

| 1 0 <a href='#SkMatrix_MakeTrans_dx'>dx</a> |
| 0 1 <a href='#SkMatrix_MakeTrans_dy'>dy</a> |
| 0 0  1 |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_MakeTrans_dx'><code><strong>dx</strong></code></a></td>
    <td>horizontal translation</td>
  </tr>
  <tr>    <td><a name='SkMatrix_MakeTrans_dy'><code><strong>dy</strong></code></a></td>
    <td>vertical translation</td>
  </tr>
</table>

### Return Value

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>with</a> <a href='SkMatrix_Reference#SkMatrix'>translation</a>

### Example

<div><fiddle-embed name="b2479df0d9cf296ff64ac31e36684557"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setTranslate'>setTranslate</a> <a href='#SkMatrix_postTranslate'>postTranslate</a> <a href='#SkMatrix_preTranslate'>preTranslate</a>

<a name='SkMatrix_MakeAll'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_MakeAll'>MakeAll</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>scaleX</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>skewX</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>transX</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>skewY</a>,
                        <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>scaleY</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>transY</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>pers0</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>pers1</a>,
                        <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>pers2</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a>:

| <a href='#SkMatrix_MakeAll_scaleX'>scaleX</a>  <a href='#SkMatrix_MakeAll_skewX'>skewX</a> <a href='#SkMatrix_MakeAll_transX'>transX</a> |
|  <a href='#SkMatrix_MakeAll_skewY'>skewY</a> <a href='#SkMatrix_MakeAll_scaleY'>scaleY</a> <a href='#SkMatrix_MakeAll_transY'>transY</a> |
|  <a href='#SkMatrix_MakeAll_pers0'>pers0</a>  <a href='#SkMatrix_MakeAll_pers1'>pers1</a>  <a href='#SkMatrix_MakeAll_pers2'>pers2</a> |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_MakeAll_scaleX'><code><strong>scaleX</strong></code></a></td>
    <td>horizontal scale factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_MakeAll_skewX'><code><strong>skewX</strong></code></a></td>
    <td>horizontal skew factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_MakeAll_transX'><code><strong>transX</strong></code></a></td>
    <td>horizontal translation</td>
  </tr>
  <tr>    <td><a name='SkMatrix_MakeAll_skewY'><code><strong>skewY</strong></code></a></td>
    <td>vertical skew factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_MakeAll_scaleY'><code><strong>scaleY</strong></code></a></td>
    <td>vertical scale factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_MakeAll_transY'><code><strong>transY</strong></code></a></td>
    <td>vertical translation</td>
  </tr>
  <tr>    <td><a name='SkMatrix_MakeAll_pers0'><code><strong>pers0</strong></code></a></td>
    <td>input x-axis perspective factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_MakeAll_pers1'><code><strong>pers1</strong></code></a></td>
    <td>input y-axis perspective factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_MakeAll_pers2'><code><strong>pers2</strong></code></a></td>
    <td>perspective scale factor</td>
  </tr>
</table>

### Return Value

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>constructed</a> <a href='SkMatrix_Reference#SkMatrix'>from</a> <a href='SkMatrix_Reference#SkMatrix'>parameters</a>

### Example

<div><fiddle-embed name="6bad83b64de9266e323c29d550e04188"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setAll'>setAll</a> <a href='#SkMatrix_set9'>set9</a> <a href='#SkMatrix_postConcat'>postConcat</a> <a href='#SkMatrix_preConcat'>preConcat</a>

<a name='SkMatrix_TypeMask'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkMatrix_TypeMask'>TypeMask</a> {
        <a href='#SkMatrix_kIdentity_Mask'>kIdentity_Mask</a> = 0,
        <a href='#SkMatrix_kTranslate_Mask'>kTranslate_Mask</a> = 0<a href='#SkMatrix_kTranslate_Mask'>x01</a>,
        <a href='#SkMatrix_kScale_Mask'>kScale_Mask</a> = 0<a href='#SkMatrix_kScale_Mask'>x02</a>,
        <a href='#SkMatrix_kAffine_Mask'>kAffine_Mask</a> = 0<a href='#SkMatrix_kAffine_Mask'>x04</a>,
        <a href='#SkMatrix_kPerspective_Mask'>kPerspective_Mask</a> = 0<a href='#SkMatrix_kPerspective_Mask'>x08</a>,
    };
</pre>

Enumeration of bit fields for mask returned by <a href='#SkMatrix_getType'>getType</a>.
<a href='#SkMatrix_getType'>Used</a> <a href='#SkMatrix_getType'>to</a> <a href='#SkMatrix_getType'>identify</a> <a href='#SkMatrix_getType'>the</a> <a href='#SkMatrix_getType'>complexity</a> <a href='#SkMatrix_getType'>of</a> <a href='SkMatrix_Reference#Matrix'>Matrix</a>, <a href='SkMatrix_Reference#Matrix'>to</a> <a href='SkMatrix_Reference#Matrix'>optimize</a> <a href='SkMatrix_Reference#Matrix'>performance</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kIdentity_Mask'><code>SkMatrix::kIdentity_Mask</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
all bits clear if <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>is</a> <a href='SkMatrix_Reference#Matrix'>identity</a>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kTranslate_Mask'><code>SkMatrix::kTranslate_Mask</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
set if <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>has</a> <a href='SkMatrix_Reference#Matrix'>translation</a>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kScale_Mask'><code>SkMatrix::kScale_Mask</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
set if <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>scales</a> <a href='SkMatrix_Reference#Matrix'>x-axis</a> <a href='SkMatrix_Reference#Matrix'>or</a> <a href='SkMatrix_Reference#Matrix'>y-axis</a>
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kAffine_Mask'><code>SkMatrix::kAffine_Mask</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
set if <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>skews</a> <a href='SkMatrix_Reference#Matrix'>or</a> <a href='SkMatrix_Reference#Matrix'>rotates</a>
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kPerspective_Mask'><code>SkMatrix::kPerspective_Mask</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>8</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
set if <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>has</a> <a href='SkMatrix_Reference#Matrix'>perspective</a>
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ba19b36df8cd78586f3dff54e2d4c093">

#### Example Output

~~~~
after reset: kIdentity_Mask
after postTranslate: kTranslate_Mask
after postScale: kTranslate_Mask kScale_Mask
after postScale: kTranslate_Mask kScale_Mask kAffine_Mask
after setPolyToPoly: kTranslate_Mask kScale_Mask kAffine_Mask kPerspective_Mask
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_getType'>getType</a>

<a name='Property'></a>

<a name='SkMatrix_getType'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkMatrix_TypeMask'>TypeMask</a> <a href='#SkMatrix_getType'>getType</a>() <a href='#SkMatrix_getType'>const</a>
</pre>

Returns a bit field describing the transformations the <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>may</a>
perform. The bit field is computed conservatively, so it may include
false positives. For example, when <a href='#SkMatrix_kPerspective_Mask'>kPerspective_Mask</a> <a href='#SkMatrix_kPerspective_Mask'>is</a> <a href='#SkMatrix_kPerspective_Mask'>set</a>, <a href='#SkMatrix_kPerspective_Mask'>all</a>
other bits are set.

### Return Value

<a href='#SkMatrix_kIdentity_Mask'>kIdentity_Mask</a>, <a href='#SkMatrix_kIdentity_Mask'>or</a> <a href='#SkMatrix_kIdentity_Mask'>combinations</a> <a href='#SkMatrix_kIdentity_Mask'>of</a>: <a href='#SkMatrix_kTranslate_Mask'>kTranslate_Mask</a>, <a href='#SkMatrix_kScale_Mask'>kScale_Mask</a>,

<a href='#SkMatrix_kAffine_Mask'>kAffine_Mask</a>, <a href='#SkMatrix_kPerspective_Mask'>kPerspective_Mask</a>

### Example

<div><fiddle-embed name="8e45fe2dd52731bb2d4318686257e1d7">

#### Example Output

~~~~
identity flags hex: 0 decimal: 0
set all  flags hex: f decimal: 15
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_TypeMask'>TypeMask</a>

<a name='SkMatrix_isIdentity'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_isIdentity'>isIdentity</a>() <a href='#SkMatrix_isIdentity'>const</a>
</pre>

Returns true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>is</a> <a href='SkMatrix_Reference#SkMatrix'>identity</a>.  <a href='SkMatrix_Reference#SkMatrix'>Identity</a> <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>is</a>:

| 1 0 0 |
| 0 1 0 |
| 0 0 1 |

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>has</a> <a href='SkMatrix_Reference#SkMatrix'>no</a> <a href='SkMatrix_Reference#SkMatrix'>effect</a>

### Example

<div><fiddle-embed name="780ab376325b3cfa889ea26c0769ec11">

#### Example Output

~~~~
is identity: true
is identity: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_reset'>reset()</a> <a href='#SkMatrix_setIdentity'>setIdentity</a> <a href='#SkMatrix_getType'>getType</a>

<a name='SkMatrix_isScaleTranslate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_isScaleTranslate'>isScaleTranslate</a>() <a href='#SkMatrix_isScaleTranslate'>const</a>
</pre>

Returns true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>at</a> <a href='SkMatrix_Reference#SkMatrix'>most</a> <a href='SkMatrix_Reference#SkMatrix'>scales</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>translates</a>. <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>may</a> <a href='SkMatrix_Reference#SkMatrix'>be</a> <a href='SkMatrix_Reference#SkMatrix'>identity</a>,
contain only scale elements, only translate elements, or both. <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>form</a> <a href='SkMatrix_Reference#SkMatrix'>is</a>:

| scale-x    0    translate-x |
|    0    scale-y translate-y |
|    0       0         1      |

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>is</a> <a href='SkMatrix_Reference#SkMatrix'>identity</a>; <a href='SkMatrix_Reference#SkMatrix'>or</a> <a href='SkMatrix_Reference#SkMatrix'>scales</a>, <a href='SkMatrix_Reference#SkMatrix'>translates</a>, <a href='SkMatrix_Reference#SkMatrix'>or</a> <a href='SkMatrix_Reference#SkMatrix'>both</a>

### Example

<div><fiddle-embed name="6287e29674a487eb94174992d45b9a34">

#### Example Output

~~~~
is scale-translate: true
is scale-translate: true
is scale-translate: true
is scale-translate: true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_setScale'>setScale</a> <a href='#SkMatrix_isTranslate'>isTranslate</a> <a href='#SkMatrix_setTranslate'>setTranslate</a> <a href='#SkMatrix_getType'>getType</a>

<a name='SkMatrix_isTranslate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_isTranslate'>isTranslate</a>() <a href='#SkMatrix_isTranslate'>const</a>
</pre>

Returns true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>is</a> <a href='SkMatrix_Reference#SkMatrix'>identity</a>, <a href='SkMatrix_Reference#SkMatrix'>or</a> <a href='SkMatrix_Reference#SkMatrix'>translates</a>. <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>form</a> <a href='SkMatrix_Reference#SkMatrix'>is</a>:

| 1 0 translate-x |
| 0 1 translate-y |
| 0 0      1      |

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>is</a> <a href='SkMatrix_Reference#SkMatrix'>identity</a>, <a href='SkMatrix_Reference#SkMatrix'>or</a> <a href='SkMatrix_Reference#SkMatrix'>translates</a>

### Example

<div><fiddle-embed name="73ac71a8a30841873577c11c6c9b38ee">

#### Example Output

~~~~
is translate: true
is translate: true
is translate: false
is translate: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_setTranslate'>setTranslate</a> <a href='#SkMatrix_getType'>getType</a>

<a name='SkMatrix_rectStaysRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_rectStaysRect'>rectStaysRect</a>() <a href='#SkMatrix_rectStaysRect'>const</a>
</pre>

Returns true <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>maps</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>another</a> <a href='SkRect_Reference#SkRect'>SkRect</a>. <a href='SkRect_Reference#SkRect'>If</a> <a href='SkRect_Reference#SkRect'>true</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>is</a> <a href='SkMatrix_Reference#SkMatrix'>identity</a>,
or scales, or rotates a multiple of 90 degrees, or mirrors on axes. In all
cases, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>may</a> <a href='SkMatrix_Reference#SkMatrix'>also</a> <a href='SkMatrix_Reference#SkMatrix'>have</a> <a href='SkMatrix_Reference#SkMatrix'>translation</a>. <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>form</a> <a href='SkMatrix_Reference#SkMatrix'>is</a> <a href='SkMatrix_Reference#SkMatrix'>either</a>:

| scale-x    0    translate-x |
|    0    scale-y translate-y |
|    0       0         1      |

or

|    0     rotate-x translate-x |
| rotate-y    0     translate-y |
|    0        0          1      |

for non-zero values of scale-x, scale-y, rotate-x, and rotate-y.

Also called <a href='#SkMatrix_preservesAxisAlignment'>preservesAxisAlignment</a>(); <a href='#SkMatrix_preservesAxisAlignment'>use</a> <a href='#SkMatrix_preservesAxisAlignment'>the</a> <a href='#SkMatrix_preservesAxisAlignment'>one</a> <a href='#SkMatrix_preservesAxisAlignment'>that</a> <a href='#SkMatrix_preservesAxisAlignment'>provides</a> <a href='#SkMatrix_preservesAxisAlignment'>better</a> <a href='#SkMatrix_preservesAxisAlignment'>inline</a>
documentation.

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>maps</a> <a href='SkMatrix_Reference#SkMatrix'>one</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>into</a> <a href='SkRect_Reference#SkRect'>another</a>

### Example

<div><fiddle-embed name="ce5319c036c9b5086da8a0009fe409f8">

#### Example Output

~~~~
rectStaysRect: true
rectStaysRect: true
rectStaysRect: true
rectStaysRect: true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_preservesAxisAlignment'>preservesAxisAlignment</a> <a href='#SkMatrix_preservesRightAngles'>preservesRightAngles</a>

<a name='SkMatrix_preservesAxisAlignment'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_preservesAxisAlignment'>preservesAxisAlignment</a>() <a href='#SkMatrix_preservesAxisAlignment'>const</a>
</pre>

Returns true <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>maps</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>another</a> <a href='SkRect_Reference#SkRect'>SkRect</a>. <a href='SkRect_Reference#SkRect'>If</a> <a href='SkRect_Reference#SkRect'>true</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>is</a> <a href='SkMatrix_Reference#SkMatrix'>identity</a>,
or scales, or rotates a multiple of 90 degrees, or mirrors on axes. In all
cases, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>may</a> <a href='SkMatrix_Reference#SkMatrix'>also</a> <a href='SkMatrix_Reference#SkMatrix'>have</a> <a href='SkMatrix_Reference#SkMatrix'>translation</a>. <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>form</a> <a href='SkMatrix_Reference#SkMatrix'>is</a> <a href='SkMatrix_Reference#SkMatrix'>either</a>:

| scale-x    0    translate-x |
|    0    scale-y translate-y |
|    0       0         1      |

or

|    0     rotate-x translate-x |
| rotate-y    0     translate-y |
|    0        0          1      |

for non-zero values of scale-x, scale-y, rotate-x, and rotate-y.

Also called <a href='#SkMatrix_rectStaysRect'>rectStaysRect</a>(); <a href='#SkMatrix_rectStaysRect'>use</a> <a href='#SkMatrix_rectStaysRect'>the</a> <a href='#SkMatrix_rectStaysRect'>one</a> <a href='#SkMatrix_rectStaysRect'>that</a> <a href='#SkMatrix_rectStaysRect'>provides</a> <a href='#SkMatrix_rectStaysRect'>better</a> <a href='#SkMatrix_rectStaysRect'>inline</a>
documentation.

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>maps</a> <a href='SkMatrix_Reference#SkMatrix'>one</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>into</a> <a href='SkRect_Reference#SkRect'>another</a>

### Example

<div><fiddle-embed name="7a234c96608fb7cb8135b9940b0b15f7">

#### Example Output

~~~~
preservesAxisAlignment: true
preservesAxisAlignment: true
preservesAxisAlignment: true
preservesAxisAlignment: true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_rectStaysRect'>rectStaysRect</a> <a href='#SkMatrix_preservesRightAngles'>preservesRightAngles</a>

<a name='SkMatrix_hasPerspective'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_hasPerspective'>hasPerspective</a>() <a href='#SkMatrix_hasPerspective'>const</a>
</pre>

Returns true if the <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>contains</a> <a href='SkMatrix_Reference#Matrix'>perspective</a> <a href='SkMatrix_Reference#Matrix'>elements</a>. <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>form</a> <a href='SkMatrix_Reference#SkMatrix'>is</a>:

|       --            --              --          |
|       --            --              --          |
| perspective-x  perspective-y  perspective-scale |

where perspective-x or perspective-y is non-zero, or perspective-scale is
not one. All other elements may have any value.

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>is</a> <a href='SkMatrix_Reference#SkMatrix'>in</a> <a href='SkMatrix_Reference#SkMatrix'>most</a> <a href='SkMatrix_Reference#SkMatrix'>general</a> <a href='SkMatrix_Reference#SkMatrix'>form</a>

### Example

<div><fiddle-embed name="688123908c733169bbbfaf11f41ecff6"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setAll'>setAll</a> <a href='#SkMatrix_set9'>set9</a> <a href='#SkMatrix_MakeAll'>MakeAll</a>

<a name='SkMatrix_isSimilarity'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_isSimilarity'>isSimilarity</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>tol</a> = <a href='undocumented#SK_ScalarNearlyZero'>SK_ScalarNearlyZero</a>) <a href='undocumented#SK_ScalarNearlyZero'>const</a>
</pre>

Returns true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>contains</a> <a href='SkMatrix_Reference#SkMatrix'>only</a> <a href='SkMatrix_Reference#SkMatrix'>translation</a>, <a href='SkMatrix_Reference#SkMatrix'>rotation</a>, <a href='SkMatrix_Reference#SkMatrix'>reflection</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a>
uniform scale.
Returns false if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>contains</a> <a href='SkMatrix_Reference#SkMatrix'>different</a> <a href='SkMatrix_Reference#SkMatrix'>scales</a>, <a href='SkMatrix_Reference#SkMatrix'>skewing</a>, <a href='SkMatrix_Reference#SkMatrix'>perspective</a>, <a href='SkMatrix_Reference#SkMatrix'>or</a>
degenerate forms that collapse to a <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>or</a> <a href='SkPoint_Reference#Point'>point</a>.

Describes that the <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>makes</a> <a href='SkMatrix_Reference#SkMatrix'>rendering</a> <a href='SkMatrix_Reference#SkMatrix'>with</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>without</a> <a href='SkMatrix_Reference#SkMatrix'>the</a> <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>are</a>
visually alike; a transformed <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>remains</a> <a href='undocumented#Circle'>a</a> <a href='undocumented#Circle'>circle</a>. <a href='undocumented#Circle'>Mathematically</a>, <a href='undocumented#Circle'>this</a> <a href='undocumented#Circle'>is</a>
referred to as similarity of a  <a href='undocumented#Euclidean_Space'>Euclidean space</a>, or a similarity transformation.

Preserves right angles, keeping the arms of the angle equal lengths.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_isSimilarity_tol'><code><strong>tol</strong></code></a></td>
    <td>to be deprecated</td>
  </tr>
</table>

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>only</a> <a href='SkMatrix_Reference#SkMatrix'>rotates</a>, <a href='SkMatrix_Reference#SkMatrix'>uniformly</a> <a href='SkMatrix_Reference#SkMatrix'>scales</a>, <a href='SkMatrix_Reference#SkMatrix'>translates</a>

### Example

<div><fiddle-embed name="8b37f4ae7fec1756433c0f984175fb14"><div><a href='undocumented#String'>String</a> <a href='undocumented#String'>is</a> <a href='undocumented#String'>drawn</a> <a href='undocumented#String'>four</a> <a href='undocumented#String'>times</a> <a href='undocumented#String'>through</a> <a href='undocumented#String'>but</a> <a href='undocumented#String'>only</a> <a href='undocumented#String'>two</a> <a href='undocumented#String'>are</a> <a href='undocumented#String'>visible</a>. <a href='undocumented#String'>Drawing</a> <a href='undocumented#String'>the</a> <a href='undocumented#String'>pair</a>
<a href='undocumented#String'>with</a> <a href='#SkMatrix_isSimilarity'>isSimilarity</a> <a href='#SkMatrix_isSimilarity'>false</a> <a href='#SkMatrix_isSimilarity'>reveals</a> <a href='#SkMatrix_isSimilarity'>the</a> <a href='#SkMatrix_isSimilarity'>pair</a> <a href='#SkMatrix_isSimilarity'>not</a> <a href='#SkMatrix_isSimilarity'>visible</a> <a href='#SkMatrix_isSimilarity'>through</a> <a href='#SkMatrix_isSimilarity'>the</a> <a href='SkMatrix_Reference#Matrix'>matrix</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkMatrix_isScaleTranslate'>isScaleTranslate</a> <a href='#SkMatrix_preservesRightAngles'>preservesRightAngles</a> <a href='#SkMatrix_rectStaysRect'>rectStaysRect</a> <a href='#SkMatrix_isFixedStepInX'>isFixedStepInX</a>

<a name='SkMatrix_preservesRightAngles'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_preservesRightAngles'>preservesRightAngles</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>tol</a> = <a href='undocumented#SK_ScalarNearlyZero'>SK_ScalarNearlyZero</a>) <a href='undocumented#SK_ScalarNearlyZero'>const</a>
</pre>

Returns true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>contains</a> <a href='SkMatrix_Reference#SkMatrix'>only</a> <a href='SkMatrix_Reference#SkMatrix'>translation</a>, <a href='SkMatrix_Reference#SkMatrix'>rotation</a>, <a href='SkMatrix_Reference#SkMatrix'>reflection</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a>
scale. Scale may differ along rotated axes.
Returns false if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>skewing</a>, <a href='SkMatrix_Reference#SkMatrix'>perspective</a>, <a href='SkMatrix_Reference#SkMatrix'>or</a> <a href='SkMatrix_Reference#SkMatrix'>degenerate</a> <a href='SkMatrix_Reference#SkMatrix'>forms</a> <a href='SkMatrix_Reference#SkMatrix'>that</a> <a href='SkMatrix_Reference#SkMatrix'>collapse</a>
to a <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>or</a> <a href='SkPoint_Reference#Point'>point</a>.

Preserves right angles, but not requiring that the arms of the angle
retain equal lengths.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_preservesRightAngles_tol'><code><strong>tol</strong></code></a></td>
    <td>to be deprecated</td>
  </tr>
</table>

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>only</a> <a href='SkMatrix_Reference#SkMatrix'>rotates</a>, <a href='SkMatrix_Reference#SkMatrix'>scales</a>, <a href='SkMatrix_Reference#SkMatrix'>translates</a>

### Example

<div><fiddle-embed name="b9becf0dc24a9f00726e24a81fb72f16"><div>Equal scale is both similar and preserves right angles.
Unequal scale is not similar but preserves right angles.
Skews are not similar and do not preserve right angles.
</div></fiddle-embed></div>

### See Also

<a href='#SkMatrix_isScaleTranslate'>isScaleTranslate</a> <a href='#SkMatrix_isSimilarity'>isSimilarity</a> <a href='#SkMatrix_rectStaysRect'>rectStaysRect</a> <a href='#SkMatrix_isFixedStepInX'>isFixedStepInX</a>

<a name='MemberIndex'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    static constexpr int <a href='#SkMatrix_kMScaleX'>kMScaleX</a> = 0;
    <a href='#SkMatrix_kMScaleX'>static</a> <a href='#SkMatrix_kMScaleX'>constexpr</a> <a href='#SkMatrix_kMScaleX'>int</a> <a href='#SkMatrix_kMSkewX'>kMSkewX</a> = 1;
    <a href='#SkMatrix_kMSkewX'>static</a> <a href='#SkMatrix_kMSkewX'>constexpr</a> <a href='#SkMatrix_kMSkewX'>int</a> <a href='#SkMatrix_kMTransX'>kMTransX</a> = 2;
    <a href='#SkMatrix_kMTransX'>static</a> <a href='#SkMatrix_kMTransX'>constexpr</a> <a href='#SkMatrix_kMTransX'>int</a> <a href='#SkMatrix_kMSkewY'>kMSkewY</a> = 3;
    <a href='#SkMatrix_kMSkewY'>static</a> <a href='#SkMatrix_kMSkewY'>constexpr</a> <a href='#SkMatrix_kMSkewY'>int</a> <a href='#SkMatrix_kMScaleY'>kMScaleY</a> = 4;
    <a href='#SkMatrix_kMScaleY'>static</a> <a href='#SkMatrix_kMScaleY'>constexpr</a> <a href='#SkMatrix_kMScaleY'>int</a> <a href='#SkMatrix_kMTransY'>kMTransY</a> = 5;
    <a href='#SkMatrix_kMTransY'>static</a> <a href='#SkMatrix_kMTransY'>constexpr</a> <a href='#SkMatrix_kMTransY'>int</a> <a href='#SkMatrix_kMPersp0'>kMPersp0</a> = 6;
    <a href='#SkMatrix_kMPersp0'>static</a> <a href='#SkMatrix_kMPersp0'>constexpr</a> <a href='#SkMatrix_kMPersp0'>int</a> <a href='#SkMatrix_kMPersp1'>kMPersp1</a> = 7;
    <a href='#SkMatrix_kMPersp1'>static</a> <a href='#SkMatrix_kMPersp1'>constexpr</a> <a href='#SkMatrix_kMPersp1'>int</a> <a href='#SkMatrix_kMPersp2'>kMPersp2</a> = 8;
</pre>

<a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>organizes</a> <a href='SkMatrix_Reference#Matrix'>its</a> <a href='SkMatrix_Reference#Matrix'>values</a> <a href='SkMatrix_Reference#Matrix'>in</a> <a href='SkMatrix_Reference#Matrix'>row</a> <a href='SkMatrix_Reference#Matrix'>order</a>. <a href='SkMatrix_Reference#Matrix'>These</a> <a href='SkMatrix_Reference#Matrix'>members</a> <a href='SkMatrix_Reference#Matrix'>correspond</a> <a href='SkMatrix_Reference#Matrix'>to</a>
<a href='SkMatrix_Reference#Matrix'>each</a> <a href='SkMatrix_Reference#Matrix'>value</a> <a href='SkMatrix_Reference#Matrix'>in</a> <a href='SkMatrix_Reference#Matrix'>Matrix</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kMScaleX'><code>SkMatrix::kMScaleX</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
horizontal scale factor</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kMSkewX'><code>SkMatrix::kMSkewX</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
horizontal skew factor</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kMTransX'><code>SkMatrix::kMTransX</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
horizontal translation</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kMSkewY'><code>SkMatrix::kMSkewY</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
vertical skew factor</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kMScaleY'><code>SkMatrix::kMScaleY</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
vertical scale factor</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kMTransY'><code>SkMatrix::kMTransY</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>5</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
vertical translation</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kMPersp0'><code>SkMatrix::kMPersp0</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>6</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
input x perspective factor</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kMPersp1'><code>SkMatrix::kMPersp1</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>7</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
input y perspective factor</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kMPersp2'><code>SkMatrix::kMPersp2</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>8</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
perspective bias</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3bbf75f4748420810aa2586e3c8548d9"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_get'>get()</a> <a href='#SkMatrix_set'>set()</a>

<a name='AffineIndex'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
</pre>

Affine arrays are in column major order to match the <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>used</a> <a href='SkMatrix_Reference#Matrix'>by</a>
<a href='SkMatrix_Reference#Matrix'>PDF</a> <a href='SkMatrix_Reference#Matrix'>and</a> <a href='SkMatrix_Reference#Matrix'>XPS</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kAScaleX'><code>SkMatrix::kAScaleX</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
horizontal scale factor</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kASkewY'><code>SkMatrix::kASkewY</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
vertical skew factor</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kASkewX'><code>SkMatrix::kASkewX</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
horizontal skew factor</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kAScaleY'><code>SkMatrix::kAScaleY</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
vertical scale factor</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kATransX'><code>SkMatrix::kATransX</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
horizontal translation</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kATransY'><code>SkMatrix::kATransY</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>5</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
vertical translation</td>
  </tr>
</table>

### See Also

<a href='#SkMatrix_SetAffineIdentity'>SetAffineIdentity</a> <a href='#SkMatrix_asAffine'>asAffine</a> <a href='#SkMatrix_setAffine'>setAffine</a>

<a name='SkMatrix_array_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>operator</a>[](<a href='undocumented#SkScalar'>int</a> <a href='undocumented#SkScalar'>index</a>) <a href='undocumented#SkScalar'>const</a>
</pre>

### Example

<div><fiddle-embed name="e8740493abdf0c6341762db9cee56b89">

#### Example Output

~~~~
matrix[SkMatrix::kMScaleX] == 42
matrix[SkMatrix::kMScaleY] == 24
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_get'>get</a> <a href='#SkMatrix_set'>set</a>

<a name='SkMatrix_get'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>get</a>(<a href='undocumented#SkScalar'>int</a> <a href='undocumented#SkScalar'>index</a>) <a href='undocumented#SkScalar'>const</a>
</pre>

Returns one <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>value</a>. <a href='SkMatrix_Reference#Matrix'>Asserts</a> <a href='SkMatrix_Reference#Matrix'>if</a> <a href='#SkMatrix_get_index'>index</a> <a href='#SkMatrix_get_index'>is</a> <a href='#SkMatrix_get_index'>out</a> <a href='#SkMatrix_get_index'>of</a> <a href='#SkMatrix_get_index'>range</a> <a href='#SkMatrix_get_index'>and</a> <a href='#SkMatrix_get_index'>SK_DEBUG</a> <a href='#SkMatrix_get_index'>is</a>
defined.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_get_index'><code><strong>index</strong></code></a></td>
    <td>one of: <a href='#SkMatrix_kMScaleX'>kMScaleX</a>, <a href='#SkMatrix_kMSkewX'>kMSkewX</a>, <a href='#SkMatrix_kMTransX'>kMTransX</a>, <a href='#SkMatrix_kMSkewY'>kMSkewY</a>, <a href='#SkMatrix_kMScaleY'>kMScaleY</a>, <a href='#SkMatrix_kMTransY'>kMTransY</a>,</td>
  </tr>
</table>

<a href='#SkMatrix_kMPersp0'>kMPersp0</a>, <a href='#SkMatrix_kMPersp1'>kMPersp1</a>, <a href='#SkMatrix_kMPersp2'>kMPersp2</a>

### Return Value

value corresponding to <a href='#SkMatrix_get_index'>index</a>

### Example

<div><fiddle-embed name="f5ed382bd04fa7d50b2398cce2fca23a">

#### Example Output

~~~~
matrix.get(SkMatrix::kMSkewX) == 42
matrix.get(SkMatrix::kMSkewY) == 24
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_array1_operator'>operator[](int index)</a> <a href='#SkMatrix_set'>set</a>

<a name='SkMatrix_getScaleX'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getScaleX'>getScaleX</a>() <a href='#SkMatrix_getScaleX'>const</a>
</pre>

Returns scale factor multiplied by x-axis input, contributing to x-axis output.
With <a href='#SkMatrix_mapPoints'>mapPoints</a>(), <a href='#SkMatrix_mapPoints'>scales</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>along</a> <a href='SkPoint_Reference#SkPoint'>the</a> <a href='SkPoint_Reference#SkPoint'>x-axis</a>.

### Return Value

horizontal scale factor

### Example

<div><fiddle-embed name="ab746d9be63975041ae8e50cba84dc3d">

#### Example Output

~~~~
matrix.getScaleX() == 42
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_get'>get</a> <a href='#SkMatrix_getScaleY'>getScaleY</a> <a href='#SkMatrix_setScaleX'>setScaleX</a> <a href='#SkMatrix_setScale'>setScale</a>

<a name='SkMatrix_getScaleY'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getScaleY'>getScaleY</a>() <a href='#SkMatrix_getScaleY'>const</a>
</pre>

Returns scale factor multiplied by y-axis input, contributing to y-axis output.
With <a href='#SkMatrix_mapPoints'>mapPoints</a>(), <a href='#SkMatrix_mapPoints'>scales</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>along</a> <a href='SkPoint_Reference#SkPoint'>the</a> <a href='SkPoint_Reference#SkPoint'>y-axis</a>.

### Return Value

vertical scale factor

### Example

<div><fiddle-embed name="708b1a548a2f8661b2ab570782fbc751">

#### Example Output

~~~~
matrix.getScaleY() == 24
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_get'>get</a> <a href='#SkMatrix_getScaleX'>getScaleX</a> <a href='#SkMatrix_setScaleY'>setScaleY</a> <a href='#SkMatrix_setScale'>setScale</a>

<a name='SkMatrix_getSkewY'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getSkewY'>getSkewY</a>() <a href='#SkMatrix_getSkewY'>const</a>
</pre>

Returns scale factor multiplied by x-axis input, contributing to y-axis output.
With <a href='#SkMatrix_mapPoints'>mapPoints</a>(), <a href='#SkMatrix_mapPoints'>skews</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>along</a> <a href='SkPoint_Reference#SkPoint'>the</a> <a href='SkPoint_Reference#SkPoint'>y-axis</a>.
Skewing both axes can rotate <a href='SkPoint_Reference#SkPoint'>SkPoint</a>.

### Return Value

vertical skew factor

### Example

<div><fiddle-embed name="6be5704506d029ffc91ba03b1d3e674b">

#### Example Output

~~~~
matrix.getSkewY() == 24
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_get'>get</a> <a href='#SkMatrix_getSkewX'>getSkewX</a> <a href='#SkMatrix_setSkewY'>setSkewY</a> <a href='#SkMatrix_setSkew'>setSkew</a>

<a name='SkMatrix_getSkewX'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getSkewX'>getSkewX</a>() <a href='#SkMatrix_getSkewX'>const</a>
</pre>

Returns scale factor multiplied by y-axis input, contributing to x-axis output.
With <a href='#SkMatrix_mapPoints'>mapPoints</a>(), <a href='#SkMatrix_mapPoints'>skews</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>along</a> <a href='SkPoint_Reference#SkPoint'>the</a> <a href='SkPoint_Reference#SkPoint'>x-axis</a>.
Skewing both axes can rotate <a href='SkPoint_Reference#SkPoint'>SkPoint</a>.

### Return Value

horizontal scale factor

### Example

<div><fiddle-embed name="df3a5d3c688e7597eae1e4e07bf91ae6">

#### Example Output

~~~~
matrix.getSkewX() == 42
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_get'>get</a> <a href='#SkMatrix_getSkewY'>getSkewY</a> <a href='#SkMatrix_setSkewX'>setSkewX</a> <a href='#SkMatrix_setSkew'>setSkew</a>

<a name='SkMatrix_getTranslateX'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getTranslateX'>getTranslateX</a>() <a href='#SkMatrix_getTranslateX'>const</a>
</pre>

Returns translation contributing to x-axis output.
With <a href='#SkMatrix_mapPoints'>mapPoints</a>(), <a href='#SkMatrix_mapPoints'>moves</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>along</a> <a href='SkPoint_Reference#SkPoint'>the</a> <a href='SkPoint_Reference#SkPoint'>x-axis</a>.

### Return Value

horizontal translation factor

### Example

<div><fiddle-embed name="6236f7f2b91aff977a66ba2ee2558ca4">

#### Example Output

~~~~
matrix.getTranslateX() == 42
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_get'>get</a> <a href='#SkMatrix_getTranslateY'>getTranslateY</a> <a href='#SkMatrix_setTranslateX'>setTranslateX</a> <a href='#SkMatrix_setTranslate'>setTranslate</a>

<a name='SkMatrix_getTranslateY'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getTranslateY'>getTranslateY</a>() <a href='#SkMatrix_getTranslateY'>const</a>
</pre>

Returns translation contributing to y-axis output.
With <a href='#SkMatrix_mapPoints'>mapPoints</a>(), <a href='#SkMatrix_mapPoints'>moves</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>along</a> <a href='SkPoint_Reference#SkPoint'>the</a> <a href='SkPoint_Reference#SkPoint'>y-axis</a>.

### Return Value

vertical translation factor

### Example

<div><fiddle-embed name="08464e32d22421d2b254c71a84545ef5">

#### Example Output

~~~~
matrix.getTranslateY() == 24
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_get'>get</a> <a href='#SkMatrix_getTranslateX'>getTranslateX</a> <a href='#SkMatrix_setTranslateY'>setTranslateY</a> <a href='#SkMatrix_setTranslate'>setTranslate</a>

<a name='SkMatrix_getPerspX'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getPerspX'>getPerspX</a>() <a href='#SkMatrix_getPerspX'>const</a>
</pre>

Returns factor scaling input x-axis relative to input y-axis.

### Return Value

input x-axis perspective factor

### Example

<div><fiddle-embed name="a0f5bf4b55e8c33bfda29bf67e34306f"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_kMPersp0'>kMPersp0</a> <a href='#SkMatrix_getPerspY'>getPerspY</a>

<a name='SkMatrix_getPerspY'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getPerspY'>getPerspY</a>() <a href='#SkMatrix_getPerspY'>const</a>
</pre>

Returns factor scaling input y-axis relative to input x-axis.

### Return Value

input y-axis perspective factor

### Example

<div><fiddle-embed name="424a00a73675dbd99ad20feb0267442b"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_kMPersp1'>kMPersp1</a> <a href='#SkMatrix_getPerspX'>getPerspX</a>

<a name='SkMatrix_array1_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a>& <a href='undocumented#SkScalar'>operator</a>[](<a href='undocumented#SkScalar'>int</a> <a href='undocumented#SkScalar'>index</a>)
</pre>

### Example

<div><fiddle-embed name="f4365ef332f51f7fd25040e0771ba9a2">

#### Example Output

~~~~
with identity matrix: x = 24
after skew x mod:     x = 24
after 2nd skew x mod: x = 24
after dirty cache:    x = 66
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_get'>get</a> <a href='#SkMatrix_dirtyMatrixTypeCache'>dirtyMatrixTypeCache</a> <a href='#SkMatrix_set'>set</a>

<a name='Set'></a>

<a name='SkMatrix_set'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void set(int index, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>value</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_set_value'>value</a>. <a href='#SkMatrix_set_value'>Asserts</a> <a href='#SkMatrix_set_value'>if</a> <a href='#SkMatrix_set_index'>index</a> <a href='#SkMatrix_set_index'>is</a> <a href='#SkMatrix_set_index'>out</a> <a href='#SkMatrix_set_index'>of</a> <a href='#SkMatrix_set_index'>range</a> <a href='#SkMatrix_set_index'>and</a> <a href='#SkMatrix_set_index'>SK_DEBUG</a> <a href='#SkMatrix_set_index'>is</a>
defined. Safer than operator[]; internal cache is always maintained.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_set_index'><code><strong>index</strong></code></a></td>
    <td>one of: <a href='#SkMatrix_kMScaleX'>kMScaleX</a>, <a href='#SkMatrix_kMSkewX'>kMSkewX</a>, <a href='#SkMatrix_kMTransX'>kMTransX</a>, <a href='#SkMatrix_kMSkewY'>kMSkewY</a>, <a href='#SkMatrix_kMScaleY'>kMScaleY</a>, <a href='#SkMatrix_kMTransY'>kMTransY</a>,</td>
  </tr>
</table>

<a href='#SkMatrix_kMPersp0'>kMPersp0</a>, <a href='#SkMatrix_kMPersp1'>kMPersp1</a>, <a href='#SkMatrix_kMPersp2'>kMPersp2</a>

### Parameters

<table>  <tr>    <td><a name='SkMatrix_set_value'><code><strong>value</strong></code></a></td>
    <td><a href='undocumented#Scalar'>scalar</a> <a href='undocumented#Scalar'>to</a> <a href='undocumented#Scalar'>store</a> <a href='undocumented#Scalar'>in</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1d400a92ca826cc89bcb88ea051f28c8">

#### Example Output

~~~~
with identity matrix: x = 24
after skew x mod:     x = 24
after 2nd skew x mod: x = 66
~~~~

</fiddle-embed></div>

### See Also

operator[] <a href='#SkMatrix_get'>get</a>

<a name='SkMatrix_setScaleX'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setScaleX'>setScaleX</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>v</a>)
</pre>

Sets horizontal scale factor.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setScaleX_v'><code><strong>v</strong></code></a></td>
    <td>horizontal scale factor to store</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a39dfed98c3c3c3a56be9ad59fe4e21e"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_set'>set</a> <a href='#SkMatrix_setScale'>setScale</a> <a href='#SkMatrix_setScaleY'>setScaleY</a>

<a name='SkMatrix_setScaleY'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setScaleY'>setScaleY</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>v</a>)
</pre>

Sets vertical scale factor.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setScaleY_v'><code><strong>v</strong></code></a></td>
    <td>vertical scale factor to store</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f040c6dd85a02e94eaca00d5c2832604"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_set'>set</a> <a href='#SkMatrix_setScale'>setScale</a> <a href='#SkMatrix_setScaleX'>setScaleX</a>

<a name='SkMatrix_setSkewY'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setSkewY'>setSkewY</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>v</a>)
</pre>

Sets vertical skew factor.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setSkewY_v'><code><strong>v</strong></code></a></td>
    <td>vertical skew factor to store</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="b418d15df9829aefcc6aca93a37428bb"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_set'>set</a> <a href='#SkMatrix_setSkew'>setSkew</a> <a href='#SkMatrix_setSkewX'>setSkewX</a>

<a name='SkMatrix_setSkewX'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setSkewX'>setSkewX</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>v</a>)
</pre>

Sets horizontal skew factor.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setSkewX_v'><code><strong>v</strong></code></a></td>
    <td>horizontal skew factor to store</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c7177a6fbc1545be95a5ebca87e0cd0d"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_set'>set</a> <a href='#SkMatrix_setSkew'>setSkew</a> <a href='#SkMatrix_setSkewX'>setSkewX</a>

<a name='SkMatrix_setTranslateX'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setTranslateX'>setTranslateX</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>v</a>)
</pre>

Sets horizontal translation.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setTranslateX_v'><code><strong>v</strong></code></a></td>
    <td>horizontal translation to store</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a18bc2e3607ac3a8e438bcb61fb13130"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_set'>set</a> <a href='#SkMatrix_setTranslate'>setTranslate</a> <a href='#SkMatrix_setTranslateY'>setTranslateY</a>

<a name='SkMatrix_setTranslateY'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setTranslateY'>setTranslateY</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>v</a>)
</pre>

Sets vertical translation.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setTranslateY_v'><code><strong>v</strong></code></a></td>
    <td>vertical translation to store</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="34e3c70a72b836abf7f4858d35eecc98"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_set'>set</a> <a href='#SkMatrix_setTranslate'>setTranslate</a> <a href='#SkMatrix_setTranslateX'>setTranslateX</a>

<a name='SkMatrix_setPerspX'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setPerspX'>setPerspX</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>v</a>)
</pre>

Sets input x-axis perspective factor, which causes <a href='#SkMatrix_mapXY'>mapXY</a>() <a href='#SkMatrix_mapXY'>to</a> <a href='#SkMatrix_mapXY'>vary</a> <a href='#SkMatrix_mapXY'>input</a> <a href='#SkMatrix_mapXY'>x-axis</a> <a href='#SkMatrix_mapXY'>values</a>
inversely proportional to input y-axis values.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setPerspX_v'><code><strong>v</strong></code></a></td>
    <td>perspective factor</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="830a9e4e4bb93d25afd83b2fea63929e"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_getPerspX'>getPerspX</a> <a href='#SkMatrix_set'>set</a> <a href='#SkMatrix_setAll'>setAll</a> <a href='#SkMatrix_set9'>set9</a> <a href='#SkMatrix_MakeAll'>MakeAll</a>

<a name='SkMatrix_setPerspY'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setPerspY'>setPerspY</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>v</a>)
</pre>

Sets input y-axis perspective factor, which causes <a href='#SkMatrix_mapXY'>mapXY</a>() <a href='#SkMatrix_mapXY'>to</a> <a href='#SkMatrix_mapXY'>vary</a> <a href='#SkMatrix_mapXY'>input</a> <a href='#SkMatrix_mapXY'>y-axis</a> <a href='#SkMatrix_mapXY'>values</a>
inversely proportional to input x-axis values.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setPerspY_v'><code><strong>v</strong></code></a></td>
    <td>perspective factor</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="aeb258b7922c1a11b698b00f562182ec"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_getPerspY'>getPerspY</a> <a href='#SkMatrix_set'>set</a> <a href='#SkMatrix_setAll'>setAll</a> <a href='#SkMatrix_set9'>set9</a> <a href='#SkMatrix_MakeAll'>MakeAll</a>

<a name='SkMatrix_setAll'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setAll'>setAll</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>scaleX</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>skewX</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>transX</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>skewY</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>scaleY</a>,
            <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>transY</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>persp0</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>persp1</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>persp2</a>)
</pre>

Sets all values from parameters. Sets <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>to</a>:

| <a href='#SkMatrix_setAll_scaleX'>scaleX</a>  <a href='#SkMatrix_setAll_skewX'>skewX</a> <a href='#SkMatrix_setAll_transX'>transX</a> |
|  <a href='#SkMatrix_setAll_skewY'>skewY</a> <a href='#SkMatrix_setAll_scaleY'>scaleY</a> <a href='#SkMatrix_setAll_transY'>transY</a> |
| <a href='#SkMatrix_setAll_persp0'>persp0</a> <a href='#SkMatrix_setAll_persp1'>persp1</a> <a href='#SkMatrix_setAll_persp2'>persp2</a> |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setAll_scaleX'><code><strong>scaleX</strong></code></a></td>
    <td>horizontal scale factor to store</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setAll_skewX'><code><strong>skewX</strong></code></a></td>
    <td>horizontal skew factor to store</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setAll_transX'><code><strong>transX</strong></code></a></td>
    <td>horizontal translation to store</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setAll_skewY'><code><strong>skewY</strong></code></a></td>
    <td>vertical skew factor to store</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setAll_scaleY'><code><strong>scaleY</strong></code></a></td>
    <td>vertical scale factor to store</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setAll_transY'><code><strong>transY</strong></code></a></td>
    <td>vertical translation to store</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setAll_persp0'><code><strong>persp0</strong></code></a></td>
    <td>input x-axis values perspective factor to store</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setAll_persp1'><code><strong>persp1</strong></code></a></td>
    <td>input y-axis values perspective factor to store</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setAll_persp2'><code><strong>persp2</strong></code></a></td>
    <td>perspective scale factor to store</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="95ccfc2a89ce593e6b7a9f992a844bc0"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_set9'>set9</a> <a href='#SkMatrix_MakeAll'>MakeAll</a>

<a name='SkMatrix_get9'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_get9'>get9</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>buffer</a>[9]) <a href='undocumented#SkScalar'>const</a>
</pre>

Copies nine <a href='undocumented#Scalar'>scalar</a> <a href='undocumented#Scalar'>values</a> <a href='undocumented#Scalar'>contained</a> <a href='undocumented#Scalar'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>into</a> <a href='#SkMatrix_get9_buffer'>buffer</a>, <a href='#SkMatrix_get9_buffer'>in</a> <a href='#SkMatrix_get9_buffer'>member</a> <a href='#SkMatrix_get9_buffer'>value</a>
ascending order: <a href='#SkMatrix_kMScaleX'>kMScaleX</a>, <a href='#SkMatrix_kMSkewX'>kMSkewX</a>, <a href='#SkMatrix_kMTransX'>kMTransX</a>, <a href='#SkMatrix_kMSkewY'>kMSkewY</a>, <a href='#SkMatrix_kMScaleY'>kMScaleY</a>, <a href='#SkMatrix_kMTransY'>kMTransY</a>,
<a href='#SkMatrix_kMPersp0'>kMPersp0</a>, <a href='#SkMatrix_kMPersp1'>kMPersp1</a>, <a href='#SkMatrix_kMPersp2'>kMPersp2</a>.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_get9_buffer'><code><strong>buffer</strong></code></a></td>
    <td>storage for nine <a href='undocumented#Scalar'>scalar</a> <a href='undocumented#Scalar'>values</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="379fc375e011050b54ed9df83c0996a7">

#### Example Output

~~~~
{4, 0, 3},
{0, 5, 4},
{0, 0, 1}
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_set9'>set9</a>

<a name='SkMatrix_set9'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_set9'>set9</a>(<a href='#SkMatrix_set9'>const</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>buffer</a>[9])
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>nine</a> <a href='undocumented#Scalar'>scalar</a> <a href='undocumented#Scalar'>values</a> <a href='undocumented#Scalar'>in</a> <a href='#SkMatrix_set9_buffer'>buffer</a>, <a href='#SkMatrix_set9_buffer'>in</a> <a href='#SkMatrix_set9_buffer'>member</a> <a href='#SkMatrix_set9_buffer'>value</a> <a href='#SkMatrix_set9_buffer'>ascending</a> <a href='#SkMatrix_set9_buffer'>order</a>:
<a href='#SkMatrix_kMScaleX'>kMScaleX</a>, <a href='#SkMatrix_kMSkewX'>kMSkewX</a>, <a href='#SkMatrix_kMTransX'>kMTransX</a>, <a href='#SkMatrix_kMSkewY'>kMSkewY</a>, <a href='#SkMatrix_kMScaleY'>kMScaleY</a>, <a href='#SkMatrix_kMTransY'>kMTransY</a>, <a href='#SkMatrix_kMPersp0'>kMPersp0</a>, <a href='#SkMatrix_kMPersp1'>kMPersp1</a>,
<a href='#SkMatrix_kMPersp2'>kMPersp2</a>.

Sets <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>to</a>:

| <a href='#SkMatrix_set9_buffer'>buffer</a>[0] <a href='#SkMatrix_set9_buffer'>buffer</a>[1] <a href='#SkMatrix_set9_buffer'>buffer</a>[2] |
| <a href='#SkMatrix_set9_buffer'>buffer</a>[3] <a href='#SkMatrix_set9_buffer'>buffer</a>[4] <a href='#SkMatrix_set9_buffer'>buffer</a>[5] |
| <a href='#SkMatrix_set9_buffer'>buffer</a>[6] <a href='#SkMatrix_set9_buffer'>buffer</a>[7] <a href='#SkMatrix_set9_buffer'>buffer</a>[8] |

In the future, <a href='#SkMatrix_set9'>set9</a> <a href='#SkMatrix_set9'>followed</a> <a href='#SkMatrix_set9'>by</a> <a href='#SkMatrix_get9'>get9</a> <a href='#SkMatrix_get9'>may</a> <a href='#SkMatrix_get9'>not</a> <a href='#SkMatrix_get9'>return</a> <a href='#SkMatrix_get9'>the</a> <a href='#SkMatrix_get9'>same</a> <a href='#SkMatrix_get9'>values</a>. <a href='#SkMatrix_get9'>Since</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
maps non-homogeneous coordinates, scaling all nine values produces an equivalent
transformation, possibly improving precision.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_set9_buffer'><code><strong>buffer</strong></code></a></td>
    <td>nine <a href='undocumented#Scalar'>scalar</a> <a href='undocumented#Scalar'>values</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ec5de0d23e5fe28ba7628625d1402e85"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setAll'>setAll</a> <a href='#SkMatrix_get9'>get9</a> <a href='#SkMatrix_MakeAll'>MakeAll</a>

<a name='SkMatrix_reset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_reset'>reset()</a>
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>identity</a>; <a href='SkMatrix_Reference#SkMatrix'>which</a> <a href='SkMatrix_Reference#SkMatrix'>has</a> <a href='SkMatrix_Reference#SkMatrix'>no</a> <a href='SkMatrix_Reference#SkMatrix'>effect</a> <a href='SkMatrix_Reference#SkMatrix'>on</a> <a href='SkMatrix_Reference#SkMatrix'>mapped</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>. <a href='SkPoint_Reference#SkPoint'>Sets</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a>:

| 1 0 0 |
| 0 1 0 |
| 0 0 1 |

Also called <a href='#SkMatrix_setIdentity'>setIdentity</a>(); <a href='#SkMatrix_setIdentity'>use</a> <a href='#SkMatrix_setIdentity'>the</a> <a href='#SkMatrix_setIdentity'>one</a> <a href='#SkMatrix_setIdentity'>that</a> <a href='#SkMatrix_setIdentity'>provides</a> <a href='#SkMatrix_setIdentity'>better</a> <a href='#SkMatrix_setIdentity'>inline</a>
documentation.

### Example

<div><fiddle-embed name="ca94f7922bc37ef03bbc51ad70536fcf">

#### Example Output

~~~~
m.isIdentity(): true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_isIdentity'>isIdentity</a> <a href='#SkMatrix_setIdentity'>setIdentity</a>

<a name='SkMatrix_setIdentity'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setIdentity'>setIdentity</a>()
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>identity</a>; <a href='SkMatrix_Reference#SkMatrix'>which</a> <a href='SkMatrix_Reference#SkMatrix'>has</a> <a href='SkMatrix_Reference#SkMatrix'>no</a> <a href='SkMatrix_Reference#SkMatrix'>effect</a> <a href='SkMatrix_Reference#SkMatrix'>on</a> <a href='SkMatrix_Reference#SkMatrix'>mapped</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a>. <a href='SkPoint_Reference#SkPoint'>Sets</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a>:

| 1 0 0 |
| 0 1 0 |
| 0 0 1 |

Also called <a href='#SkMatrix_reset'>reset()</a>; <a href='#SkMatrix_reset'>use</a> <a href='#SkMatrix_reset'>the</a> <a href='#SkMatrix_reset'>one</a> <a href='#SkMatrix_reset'>that</a> <a href='#SkMatrix_reset'>provides</a> <a href='#SkMatrix_reset'>better</a> <a href='#SkMatrix_reset'>inline</a>
documentation.

### Example

<div><fiddle-embed name="3979c865bb482e6ef1fafc71e56bbb91">

#### Example Output

~~~~
m.isIdentity(): true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_isIdentity'>isIdentity</a> <a href='#SkMatrix_reset'>reset</a>

<a name='SkMatrix_setTranslate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setTranslate'>setTranslate</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>translate</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> (<a href='#SkMatrix_setTranslate_dx'>dx</a>, <a href='#SkMatrix_setTranslate_dy'>dy</a>).

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setTranslate_dx'><code><strong>dx</strong></code></a></td>
    <td>horizontal translation</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setTranslate_dy'><code><strong>dy</strong></code></a></td>
    <td>vertical translation</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="63ca62985741b1bccb5e8b9cf734874e"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setTranslateX'>setTranslateX</a> <a href='#SkMatrix_setTranslateY'>setTranslateY</a>

<a name='SkMatrix_setTranslate_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setTranslate'>setTranslate</a>(<a href='#SkMatrix_setTranslate'>const</a> <a href='SkPoint_Reference#SkVector'>SkVector</a>& <a href='SkPoint_Reference#SkVector'>v</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>translate</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> (<a href='#SkMatrix_setTranslate_2_v'>v</a>.<a href='#SkPoint_fX'>fX</a>, <a href='#SkMatrix_setTranslate_2_v'>v</a>.<a href='#SkPoint_fY'>fY</a>).

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setTranslate_2_v'><code><strong>v</strong></code></a></td>
    <td><a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>containing</a> <a href='SkPoint_Reference#Vector'>horizontal</a> <a href='SkPoint_Reference#Vector'>and</a> <a href='SkPoint_Reference#Vector'>vertical</a> <a href='SkPoint_Reference#Vector'>translation</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ccfc734aff2ddea0b097c83f5621de5e"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setTranslateX'>setTranslateX</a> <a href='#SkMatrix_setTranslateY'>setTranslateY</a> <a href='#SkMatrix_MakeTrans'>MakeTrans</a>

<a name='SkMatrix_setScale'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setScale'>setScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sy</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>px</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>py</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>scale</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkMatrix_setScale_sx'>sx</a> <a href='#SkMatrix_setScale_sx'>and</a> <a href='#SkMatrix_setScale_sy'>sy</a>, <a href='#SkMatrix_setScale_sy'>about</a> <a href='#SkMatrix_setScale_sy'>a</a> <a href='#SkMatrix_setScale_sy'>pivot</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>at</a> (<a href='#SkMatrix_setScale_px'>px</a>, <a href='#SkMatrix_setScale_py'>py</a>).
The pivot <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>is</a> <a href='SkPoint_Reference#Point'>unchanged</a> <a href='SkPoint_Reference#Point'>when</a> <a href='SkPoint_Reference#Point'>mapped</a> <a href='SkPoint_Reference#Point'>with</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setScale_sx'><code><strong>sx</strong></code></a></td>
    <td>horizontal scale factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setScale_sy'><code><strong>sy</strong></code></a></td>
    <td>vertical scale factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setScale_px'><code><strong>px</strong></code></a></td>
    <td>pivot on x-axis</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setScale_py'><code><strong>py</strong></code></a></td>
    <td>pivot on y-axis</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4565a0792058178c88e0a129a87272d6"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setScaleX'>setScaleX</a> <a href='#SkMatrix_setScaleY'>setScaleY</a> <a href='#SkMatrix_MakeScale'>MakeScale</a> <a href='#SkMatrix_preScale'>preScale</a> <a href='#SkMatrix_postScale'>postScale</a>

<a name='SkMatrix_setScale_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setScale'>setScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sy</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>scale</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkMatrix_setScale_2_sx'>sx</a> <a href='#SkMatrix_setScale_2_sx'>and</a> <a href='#SkMatrix_setScale_2_sy'>sy</a> <a href='#SkMatrix_setScale_2_sy'>about</a> <a href='#SkMatrix_setScale_2_sy'>at</a> <a href='#SkMatrix_setScale_2_sy'>pivot</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>at</a> (0, 0).

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setScale_2_sx'><code><strong>sx</strong></code></a></td>
    <td>horizontal scale factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setScale_2_sy'><code><strong>sy</strong></code></a></td>
    <td>vertical scale factor</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1579d0cc109c26e69f66f73abd35fb0e"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setScaleX'>setScaleX</a> <a href='#SkMatrix_setScaleY'>setScaleY</a> <a href='#SkMatrix_MakeScale'>MakeScale</a> <a href='#SkMatrix_preScale'>preScale</a> <a href='#SkMatrix_postScale'>postScale</a>

<a name='SkMatrix_setRotate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setRotate'>setRotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>degrees</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>px</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>py</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>rotate</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkMatrix_setRotate_degrees'>degrees</a> <a href='#SkMatrix_setRotate_degrees'>about</a> <a href='#SkMatrix_setRotate_degrees'>a</a> <a href='#SkMatrix_setRotate_degrees'>pivot</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>at</a> (<a href='#SkMatrix_setRotate_px'>px</a>, <a href='#SkMatrix_setRotate_py'>py</a>).
The pivot <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>is</a> <a href='SkPoint_Reference#Point'>unchanged</a> <a href='SkPoint_Reference#Point'>when</a> <a href='SkPoint_Reference#Point'>mapped</a> <a href='SkPoint_Reference#Point'>with</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Positive <a href='#SkMatrix_setRotate_degrees'>degrees</a> <a href='#SkMatrix_setRotate_degrees'>rotates</a> <a href='#SkMatrix_setRotate_degrees'>clockwise</a>.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setRotate_degrees'><code><strong>degrees</strong></code></a></td>
    <td>angle of axes relative to upright axes</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setRotate_px'><code><strong>px</strong></code></a></td>
    <td>pivot on x-axis</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setRotate_py'><code><strong>py</strong></code></a></td>
    <td>pivot on y-axis</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="8c28db3add9cd0177225088f6df6bbb5"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setSinCos'>setSinCos</a> <a href='#SkMatrix_preRotate'>preRotate</a> <a href='#SkMatrix_postRotate'>postRotate</a>

<a name='SkMatrix_setRotate_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setRotate'>setRotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>degrees</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>rotate</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkMatrix_setRotate_2_degrees'>degrees</a> <a href='#SkMatrix_setRotate_2_degrees'>about</a> <a href='#SkMatrix_setRotate_2_degrees'>a</a> <a href='#SkMatrix_setRotate_2_degrees'>pivot</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>at</a> (0, 0).
Positive <a href='#SkMatrix_setRotate_2_degrees'>degrees</a> <a href='#SkMatrix_setRotate_2_degrees'>rotates</a> <a href='#SkMatrix_setRotate_2_degrees'>clockwise</a>.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setRotate_2_degrees'><code><strong>degrees</strong></code></a></td>
    <td>angle of axes relative to upright axes</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="93efb9d191bf1b9710c173513e014d6c"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setSinCos'>setSinCos</a> <a href='#SkMatrix_preRotate'>preRotate</a> <a href='#SkMatrix_postRotate'>postRotate</a>

<a name='SkMatrix_setSinCos'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setSinCos'>setSinCos</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sinValue</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>cosValue</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>px</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>py</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>rotate</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkMatrix_setSinCos_sinValue'>sinValue</a> <a href='#SkMatrix_setSinCos_sinValue'>and</a> <a href='#SkMatrix_setSinCos_cosValue'>cosValue</a>, <a href='#SkMatrix_setSinCos_cosValue'>about</a> <a href='#SkMatrix_setSinCos_cosValue'>a</a> <a href='#SkMatrix_setSinCos_cosValue'>pivot</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>at</a> (<a href='#SkMatrix_setSinCos_px'>px</a>, <a href='#SkMatrix_setSinCos_py'>py</a>).
The pivot <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>is</a> <a href='SkPoint_Reference#Point'>unchanged</a> <a href='SkPoint_Reference#Point'>when</a> <a href='SkPoint_Reference#Point'>mapped</a> <a href='SkPoint_Reference#Point'>with</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

<a href='SkPoint_Reference#Vector'>Vector</a> (<a href='#SkMatrix_setSinCos_sinValue'>sinValue</a>, <a href='#SkMatrix_setSinCos_cosValue'>cosValue</a>) <a href='#SkMatrix_setSinCos_cosValue'>describes</a> <a href='#SkMatrix_setSinCos_cosValue'>the</a> <a href='#SkMatrix_setSinCos_cosValue'>angle</a> <a href='#SkMatrix_setSinCos_cosValue'>of</a> <a href='#SkMatrix_setSinCos_cosValue'>rotation</a> <a href='#SkMatrix_setSinCos_cosValue'>relative</a> <a href='#SkMatrix_setSinCos_cosValue'>to</a> (0, 1).
<a href='SkPoint_Reference#Vector'>Vector</a> <a href='SkPoint_Reference#Vector'>length</a> <a href='SkPoint_Reference#Vector'>specifies</a> <a href='SkPoint_Reference#Vector'>scale</a>.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setSinCos_sinValue'><code><strong>sinValue</strong></code></a></td>
    <td>rotation <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>x-axis</a> <a href='SkPoint_Reference#Vector'>component</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_setSinCos_cosValue'><code><strong>cosValue</strong></code></a></td>
    <td>rotation <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>y-axis</a> <a href='SkPoint_Reference#Vector'>component</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_setSinCos_px'><code><strong>px</strong></code></a></td>
    <td>pivot on x-axis</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setSinCos_py'><code><strong>py</strong></code></a></td>
    <td>pivot on y-axis</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="187e1d9228e2e4341ef820bd77b6fda9"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setRotate'>setRotate</a> <a href='#SkMatrix_setScale'>setScale</a> <a href='#SkMatrix_setRSXform'>setRSXform</a>

<a name='SkMatrix_setSinCos_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setSinCos'>setSinCos</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sinValue</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>cosValue</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>rotate</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkMatrix_setSinCos_2_sinValue'>sinValue</a> <a href='#SkMatrix_setSinCos_2_sinValue'>and</a> <a href='#SkMatrix_setSinCos_2_cosValue'>cosValue</a>, <a href='#SkMatrix_setSinCos_2_cosValue'>about</a> <a href='#SkMatrix_setSinCos_2_cosValue'>a</a> <a href='#SkMatrix_setSinCos_2_cosValue'>pivot</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>at</a> (0, 0).

<a href='SkPoint_Reference#Vector'>Vector</a> (<a href='#SkMatrix_setSinCos_2_sinValue'>sinValue</a>, <a href='#SkMatrix_setSinCos_2_cosValue'>cosValue</a>) <a href='#SkMatrix_setSinCos_2_cosValue'>describes</a> <a href='#SkMatrix_setSinCos_2_cosValue'>the</a> <a href='#SkMatrix_setSinCos_2_cosValue'>angle</a> <a href='#SkMatrix_setSinCos_2_cosValue'>of</a> <a href='#SkMatrix_setSinCos_2_cosValue'>rotation</a> <a href='#SkMatrix_setSinCos_2_cosValue'>relative</a> <a href='#SkMatrix_setSinCos_2_cosValue'>to</a> (0, 1).
<a href='SkPoint_Reference#Vector'>Vector</a> <a href='SkPoint_Reference#Vector'>length</a> <a href='SkPoint_Reference#Vector'>specifies</a> <a href='SkPoint_Reference#Vector'>scale</a>.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setSinCos_2_sinValue'><code><strong>sinValue</strong></code></a></td>
    <td>rotation <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>x-axis</a> <a href='SkPoint_Reference#Vector'>component</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_setSinCos_2_cosValue'><code><strong>cosValue</strong></code></a></td>
    <td>rotation <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>y-axis</a> <a href='SkPoint_Reference#Vector'>component</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="e37a94a53c959951b059fcd624639ef6"><div><a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>needs</a> <a href='SkCanvas_Reference#Canvas'>offset</a> <a href='SkCanvas_Reference#Canvas'>after</a> <a href='SkCanvas_Reference#Canvas'>applying</a> <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>to</a> <a href='SkMatrix_Reference#Matrix'>pivot</a> <a href='SkMatrix_Reference#Matrix'>about</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>center</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setRotate'>setRotate</a> <a href='#SkMatrix_setScale'>setScale</a> <a href='#SkMatrix_setRSXform'>setRSXform</a>

<a name='SkMatrix_setRSXform'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='#SkMatrix_setRSXform'>setRSXform</a>(<a href='#SkMatrix_setRSXform'>const</a> <a href='undocumented#SkRSXform'>SkRSXform</a>& <a href='undocumented#SkRSXform'>rsxForm</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>rotate</a>, <a href='SkMatrix_Reference#SkMatrix'>scale</a>, <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>translate</a> <a href='SkMatrix_Reference#SkMatrix'>using</a> <a href='SkMatrix_Reference#SkMatrix'>a</a> <a href='SkMatrix_Reference#SkMatrix'>compressed</a> <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>form</a>.

<a href='SkPoint_Reference#Vector'>Vector</a> (<a href='#SkMatrix_setRSXform_rsxForm'>rsxForm</a>.<a href='#SkRSXform_fSSin'>fSSin</a>, <a href='#SkMatrix_setRSXform_rsxForm'>rsxForm</a>.<a href='#SkRSXform_fSCos'>fSCos</a>) <a href='#SkRSXform_fSCos'>describes</a> <a href='#SkRSXform_fSCos'>the</a> <a href='#SkRSXform_fSCos'>angle</a> <a href='#SkRSXform_fSCos'>of</a> <a href='#SkRSXform_fSCos'>rotation</a> <a href='#SkRSXform_fSCos'>relative</a>
to (0, 1). <a href='SkPoint_Reference#Vector'>Vector</a> <a href='SkPoint_Reference#Vector'>length</a> <a href='SkPoint_Reference#Vector'>specifies</a> <a href='SkPoint_Reference#Vector'>scale</a>. <a href='SkPoint_Reference#Vector'>Mapped</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>is</a> <a href='SkPoint_Reference#Point'>rotated</a> <a href='SkPoint_Reference#Point'>and</a> <a href='SkPoint_Reference#Point'>scaled</a>
by <a href='SkPoint_Reference#Vector'>vector</a>, <a href='SkPoint_Reference#Vector'>then</a> <a href='SkPoint_Reference#Vector'>translated</a> <a href='SkPoint_Reference#Vector'>by</a> (<a href='#SkMatrix_setRSXform_rsxForm'>rsxForm</a>.<a href='#SkRSXform_fTx'>fTx</a>, <a href='#SkMatrix_setRSXform_rsxForm'>rsxForm</a>.<a href='#SkRSXform_fTy'>fTy</a>).

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setRSXform_rsxForm'><code><strong>rsxForm</strong></code></a></td>
    <td>compressed <a href='undocumented#SkRSXform'>SkRSXform</a> <a href='SkMatrix_Reference#Matrix'>matrix</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>

### Example

<div><fiddle-embed name="c3f5faddca466f78278b32b88fd5f5eb"><div><a href='SkCanvas_Reference#Canvas'>Canvas</a> <a href='SkCanvas_Reference#Canvas'>needs</a> <a href='SkCanvas_Reference#Canvas'>offset</a> <a href='SkCanvas_Reference#Canvas'>after</a> <a href='SkCanvas_Reference#Canvas'>applying</a> <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>to</a> <a href='SkMatrix_Reference#Matrix'>pivot</a> <a href='SkMatrix_Reference#Matrix'>about</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>center</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setSinCos'>setSinCos</a> <a href='#SkMatrix_setScale'>setScale</a> <a href='#SkMatrix_setTranslate'>setTranslate</a>

<a name='SkMatrix_setSkew'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setSkew'>setSkew</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>kx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>ky</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>px</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>py</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>skew</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkMatrix_setSkew_kx'>kx</a> <a href='#SkMatrix_setSkew_kx'>and</a> <a href='#SkMatrix_setSkew_ky'>ky</a>, <a href='#SkMatrix_setSkew_ky'>about</a> <a href='#SkMatrix_setSkew_ky'>a</a> <a href='#SkMatrix_setSkew_ky'>pivot</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>at</a> (<a href='#SkMatrix_setSkew_px'>px</a>, <a href='#SkMatrix_setSkew_py'>py</a>).
The pivot <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>is</a> <a href='SkPoint_Reference#Point'>unchanged</a> <a href='SkPoint_Reference#Point'>when</a> <a href='SkPoint_Reference#Point'>mapped</a> <a href='SkPoint_Reference#Point'>with</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setSkew_kx'><code><strong>kx</strong></code></a></td>
    <td>horizontal skew factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setSkew_ky'><code><strong>ky</strong></code></a></td>
    <td>vertical skew factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setSkew_px'><code><strong>px</strong></code></a></td>
    <td>pivot on x-axis</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setSkew_py'><code><strong>py</strong></code></a></td>
    <td>pivot on y-axis</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="55e0431adc6c5b1987ebb8123cc10342"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setSkewX'>setSkewX</a> <a href='#SkMatrix_setSkewY'>setSkewY</a> <a href='#SkMatrix_preSkew'>preSkew</a> <a href='#SkMatrix_postSkew'>postSkew</a>

<a name='SkMatrix_setSkew_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setSkew'>setSkew</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>kx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>ky</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>skew</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkMatrix_setSkew_2_kx'>kx</a> <a href='#SkMatrix_setSkew_2_kx'>and</a> <a href='#SkMatrix_setSkew_2_ky'>ky</a>, <a href='#SkMatrix_setSkew_2_ky'>about</a> <a href='#SkMatrix_setSkew_2_ky'>a</a> <a href='#SkMatrix_setSkew_2_ky'>pivot</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>at</a> (0, 0).

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setSkew_2_kx'><code><strong>kx</strong></code></a></td>
    <td>horizontal skew factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setSkew_2_ky'><code><strong>ky</strong></code></a></td>
    <td>vertical skew factor</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="05be7844e9afdd7b9bfc31c5423a70a2"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setSkewX'>setSkewX</a> <a href='#SkMatrix_setSkewY'>setSkewY</a> <a href='#SkMatrix_preSkew'>preSkew</a> <a href='#SkMatrix_postSkew'>postSkew</a>

<a name='SkMatrix_setConcat'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setConcat'>setConcat</a>(<a href='#SkMatrix_setConcat'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#SkMatrix'>a</a>, <a href='SkMatrix_Reference#SkMatrix'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#SkMatrix'>b</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_setConcat_a'>a</a> <a href='#SkMatrix_setConcat_a'>multiplied</a> <a href='#SkMatrix_setConcat_a'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_setConcat_b'>b</a>. <a href='#SkMatrix_setConcat_b'>Either</a> <a href='#SkMatrix_setConcat_a'>a</a> <a href='#SkMatrix_setConcat_a'>or</a> <a href='#SkMatrix_setConcat_b'>b</a> <a href='#SkMatrix_setConcat_b'>may</a> <a href='#SkMatrix_setConcat_b'>be</a> <a href='#SkMatrix_setConcat_b'>this</a>.

Given:

| A B C |      | J K L |
<a href='#SkMatrix_setConcat_a'>a</a> = | <a href='#SkMatrix_setConcat_a'>D</a> <a href='#SkMatrix_setConcat_a'>E</a> <a href='#SkMatrix_setConcat_a'>F</a> |, <a href='#SkMatrix_setConcat_b'>b</a> = | <a href='#SkMatrix_setConcat_b'>M</a> <a href='#SkMatrix_setConcat_b'>N</a> <a href='#SkMatrix_setConcat_b'>O</a> |
| G H <a href='#SkMatrix_I'>I</a> |      | <a href='#SkMatrix_I'>P</a> <a href='#SkMatrix_I'>Q</a> <a href='#SkMatrix_I'>R</a> |

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a>:

| A B C |   | J K L |   | AJ+BM+CP AK+BN+CQ AL+BO+CR |
<a href='#SkMatrix_setConcat_a'>a</a> * <a href='#SkMatrix_setConcat_b'>b</a> = | <a href='#SkMatrix_setConcat_b'>D</a> <a href='#SkMatrix_setConcat_b'>E</a> <a href='#SkMatrix_setConcat_b'>F</a> | * | <a href='#SkMatrix_setConcat_b'>M</a> <a href='#SkMatrix_setConcat_b'>N</a> <a href='#SkMatrix_setConcat_b'>O</a> | = | <a href='#SkMatrix_setConcat_b'>DJ</a>+<a href='#SkMatrix_setConcat_b'>EM</a>+<a href='#SkMatrix_setConcat_b'>FP</a> <a href='#SkMatrix_setConcat_b'>DK</a>+<a href='#SkMatrix_setConcat_b'>EN</a>+<a href='#SkMatrix_setConcat_b'>FQ</a> <a href='#SkMatrix_setConcat_b'>DL</a>+<a href='#SkMatrix_setConcat_b'>EO</a>+<a href='#SkMatrix_setConcat_b'>FR</a> |
| G H <a href='#SkMatrix_I'>I</a> |   | <a href='#SkMatrix_I'>P</a> <a href='#SkMatrix_I'>Q</a> <a href='#SkMatrix_I'>R</a> |   | <a href='#SkMatrix_I'>GJ</a>+<a href='#SkMatrix_I'>HM</a>+<a href='#SkMatrix_I'>IP</a> <a href='#SkMatrix_I'>GK</a>+<a href='#SkMatrix_I'>HN</a>+<a href='#SkMatrix_I'>IQ</a> <a href='#SkMatrix_I'>GL</a>+<a href='#SkMatrix_I'>HO</a>+<a href='#SkMatrix_I'>IR</a> |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setConcat_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>on</a>  <a href='SkMatrix_Reference#SkMatrix'>left side</a> <a href='SkMatrix_Reference#SkMatrix'>of</a> <a href='SkMatrix_Reference#SkMatrix'>multiply</a> <a href='SkMatrix_Reference#SkMatrix'>expression</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_setConcat_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>on</a>  <a href='SkMatrix_Reference#SkMatrix'>right side</a> <a href='SkMatrix_Reference#SkMatrix'>of</a> <a href='SkMatrix_Reference#SkMatrix'>multiply</a> <a href='SkMatrix_Reference#SkMatrix'>expression</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0381a10ac69bdefdf9d15b47cbb9fefe"><div><a href='#SkMatrix_setPolyToPoly'>setPolyToPoly</a> <a href='#SkMatrix_setPolyToPoly'>creates</a> <a href='#SkMatrix_setPolyToPoly'>perspective</a> <a href='SkMatrix_Reference#Matrix'>matrices</a>, <a href='SkMatrix_Reference#Matrix'>one</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>inverse</a> <a href='SkMatrix_Reference#Matrix'>of</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>other</a>.
<a href='SkMatrix_Reference#Matrix'>Multiplying</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>by</a> <a href='SkMatrix_Reference#Matrix'>its</a> <a href='SkMatrix_Reference#Matrix'>inverse</a> <a href='SkMatrix_Reference#Matrix'>turns</a> <a href='SkMatrix_Reference#Matrix'>into</a> <a href='SkMatrix_Reference#Matrix'>an</a> <a href='SkMatrix_Reference#Matrix'>identity</a> <a href='SkMatrix_Reference#Matrix'>matrix</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkMatrix_Concat'>Concat</a> <a href='#SkMatrix_preConcat'>preConcat</a> <a href='#SkMatrix_postConcat'>postConcat</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_concat'>concat</a>

<a name='SkMatrix_preTranslate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_preTranslate'>preTranslate</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>multiplied</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>constructed</a> <a href='SkMatrix_Reference#SkMatrix'>from</a> <a href='SkMatrix_Reference#SkMatrix'>translation</a> (<a href='#SkMatrix_preTranslate_dx'>dx</a>, <a href='#SkMatrix_preTranslate_dy'>dy</a>).
This can be thought of as moving the <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>be</a> <a href='SkPoint_Reference#Point'>mapped</a> <a href='SkPoint_Reference#Point'>before</a> <a href='SkPoint_Reference#Point'>applying</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Given:

| A B C |               | 1 0 <a href='#SkMatrix_preTranslate_dx'>dx</a> |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> <a href='SkMatrix_Reference#Matrix'>F</a> |,  <a href='SkMatrix_Reference#Matrix'>T</a>(<a href='#SkMatrix_preTranslate_dx'>dx</a>, <a href='#SkMatrix_preTranslate_dy'>dy</a>) = | 0 1 <a href='#SkMatrix_preTranslate_dy'>dy</a> |
| G H <a href='#SkMatrix_I'>I</a> |               | 0 0  1 |

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a>:

| A B C | | 1 0 <a href='#SkMatrix_preTranslate_dx'>dx</a> |   | <a href='#SkMatrix_preTranslate_dx'>A</a> <a href='#SkMatrix_preTranslate_dx'>B</a> <a href='#SkMatrix_preTranslate_dx'>A</a>*<a href='#SkMatrix_preTranslate_dx'>dx</a>+<a href='#SkMatrix_preTranslate_dx'>B</a>*<a href='#SkMatrix_preTranslate_dy'>dy</a>+<a href='#SkMatrix_preTranslate_dy'>C</a> |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * <a href='SkMatrix_Reference#Matrix'>T</a>(<a href='#SkMatrix_preTranslate_dx'>dx</a>, <a href='#SkMatrix_preTranslate_dy'>dy</a>) = | <a href='#SkMatrix_preTranslate_dy'>D</a> <a href='#SkMatrix_preTranslate_dy'>E</a> <a href='#SkMatrix_preTranslate_dy'>F</a> | | 0 1 <a href='#SkMatrix_preTranslate_dy'>dy</a> | = | <a href='#SkMatrix_preTranslate_dy'>D</a> <a href='#SkMatrix_preTranslate_dy'>E</a> <a href='#SkMatrix_preTranslate_dy'>D</a>*<a href='#SkMatrix_preTranslate_dx'>dx</a>+<a href='#SkMatrix_preTranslate_dx'>E</a>*<a href='#SkMatrix_preTranslate_dy'>dy</a>+<a href='#SkMatrix_preTranslate_dy'>F</a> |
| G H <a href='#SkMatrix_I'>I</a> | | 0 0  1 |   | <a href='#SkMatrix_I'>G</a> <a href='#SkMatrix_I'>H</a> <a href='#SkMatrix_I'>G</a>*<a href='#SkMatrix_preTranslate_dx'>dx</a>+<a href='#SkMatrix_preTranslate_dx'>H</a>*<a href='#SkMatrix_preTranslate_dy'>dy</a>+<a href='#SkMatrix_I'>I</a> |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_preTranslate_dx'><code><strong>dx</strong></code></a></td>
    <td>x-axis translation before applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_preTranslate_dy'><code><strong>dy</strong></code></a></td>
    <td>y-axis translation before applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f75a9b629aa6c51ed888f8799b5ba5f7"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_postTranslate'>postTranslate</a> <a href='#SkMatrix_setTranslate'>setTranslate</a> <a href='#SkMatrix_MakeTrans'>MakeTrans</a>

<a name='SkMatrix_preScale'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_preScale'>preScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sy</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>px</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>py</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>multiplied</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>constructed</a> <a href='SkMatrix_Reference#SkMatrix'>from</a> <a href='SkMatrix_Reference#SkMatrix'>scaling</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> (<a href='#SkMatrix_preScale_sx'>sx</a>, <a href='#SkMatrix_preScale_sy'>sy</a>)
about pivot <a href='SkPoint_Reference#Point'>point</a> (<a href='#SkMatrix_preScale_px'>px</a>, <a href='#SkMatrix_preScale_py'>py</a>).
This can be thought of as scaling about a pivot <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>before</a> <a href='SkPoint_Reference#Point'>applying</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Given:

| A B C |                       | <a href='#SkMatrix_preScale_sx'>sx</a>  0 <a href='#SkMatrix_preScale_sx'>dx</a> |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> <a href='SkMatrix_Reference#Matrix'>F</a> |,  <a href='SkMatrix_Reference#Matrix'>S</a>(<a href='#SkMatrix_preScale_sx'>sx</a>, <a href='#SkMatrix_preScale_sy'>sy</a>, <a href='#SkMatrix_preScale_px'>px</a>, <a href='#SkMatrix_preScale_py'>py</a>) = |  0 <a href='#SkMatrix_preScale_sy'>sy</a> <a href='#SkMatrix_preScale_sy'>dy</a> |
| G H <a href='#SkMatrix_I'>I</a> |                       |  0  0  1 |

where

dx = <a href='#SkMatrix_preScale_px'>px</a> - <a href='#SkMatrix_preScale_sx'>sx</a> * <a href='#SkMatrix_preScale_px'>px</a>
dy = <a href='#SkMatrix_preScale_py'>py</a> - <a href='#SkMatrix_preScale_sy'>sy</a> * <a href='#SkMatrix_preScale_py'>py</a>

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a>:

| A B C | | <a href='#SkMatrix_preScale_sx'>sx</a>  0 <a href='#SkMatrix_preScale_sx'>dx</a> |   | <a href='#SkMatrix_preScale_sx'>A</a>*<a href='#SkMatrix_preScale_sx'>sx</a> <a href='#SkMatrix_preScale_sx'>B</a>*<a href='#SkMatrix_preScale_sy'>sy</a> <a href='#SkMatrix_preScale_sy'>A</a>*<a href='#SkMatrix_preScale_sy'>dx</a>+<a href='#SkMatrix_preScale_sy'>B</a>*<a href='#SkMatrix_preScale_sy'>dy</a>+<a href='#SkMatrix_preScale_sy'>C</a> |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * <a href='SkMatrix_Reference#Matrix'>S</a>(<a href='#SkMatrix_preScale_sx'>sx</a>, <a href='#SkMatrix_preScale_sy'>sy</a>, <a href='#SkMatrix_preScale_px'>px</a>, <a href='#SkMatrix_preScale_py'>py</a>) = | <a href='#SkMatrix_preScale_py'>D</a> <a href='#SkMatrix_preScale_py'>E</a> <a href='#SkMatrix_preScale_py'>F</a> | |  0 <a href='#SkMatrix_preScale_sy'>sy</a> <a href='#SkMatrix_preScale_sy'>dy</a> | = | <a href='#SkMatrix_preScale_sy'>D</a>*<a href='#SkMatrix_preScale_sx'>sx</a> <a href='#SkMatrix_preScale_sx'>E</a>*<a href='#SkMatrix_preScale_sy'>sy</a> <a href='#SkMatrix_preScale_sy'>D</a>*<a href='#SkMatrix_preScale_sy'>dx</a>+<a href='#SkMatrix_preScale_sy'>E</a>*<a href='#SkMatrix_preScale_sy'>dy</a>+<a href='#SkMatrix_preScale_sy'>F</a> |
| G H <a href='#SkMatrix_I'>I</a> | |  0  0  1 |   | <a href='#SkMatrix_I'>G</a>*<a href='#SkMatrix_preScale_sx'>sx</a> <a href='#SkMatrix_preScale_sx'>H</a>*<a href='#SkMatrix_preScale_sy'>sy</a> <a href='#SkMatrix_preScale_sy'>G</a>*<a href='#SkMatrix_preScale_sy'>dx</a>+<a href='#SkMatrix_preScale_sy'>H</a>*<a href='#SkMatrix_preScale_sy'>dy</a>+<a href='#SkMatrix_I'>I</a> |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_preScale_sx'><code><strong>sx</strong></code></a></td>
    <td>horizontal scale factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_preScale_sy'><code><strong>sy</strong></code></a></td>
    <td>vertical scale factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_preScale_px'><code><strong>px</strong></code></a></td>
    <td>pivot on x-axis</td>
  </tr>
  <tr>    <td><a name='SkMatrix_preScale_py'><code><strong>py</strong></code></a></td>
    <td>pivot on y-axis</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="2531f8d1e05d7b6dc22f3efcd2fb84e4"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_postScale'>postScale</a> <a href='#SkMatrix_setScale'>setScale</a> <a href='#SkMatrix_MakeScale'>MakeScale</a>

<a name='SkMatrix_preScale_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_preScale'>preScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sy</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>multiplied</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>constructed</a> <a href='SkMatrix_Reference#SkMatrix'>from</a> <a href='SkMatrix_Reference#SkMatrix'>scaling</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> (<a href='#SkMatrix_preScale_2_sx'>sx</a>, <a href='#SkMatrix_preScale_2_sy'>sy</a>)
about pivot <a href='SkPoint_Reference#Point'>point</a> (0, 0).
This can be thought of as scaling about the origin before applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Given:

| A B C |               | <a href='#SkMatrix_preScale_2_sx'>sx</a>  0  0 |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> <a href='SkMatrix_Reference#Matrix'>F</a> |,  <a href='SkMatrix_Reference#Matrix'>S</a>(<a href='#SkMatrix_preScale_2_sx'>sx</a>, <a href='#SkMatrix_preScale_2_sy'>sy</a>) = |  0 <a href='#SkMatrix_preScale_2_sy'>sy</a>  0 |
| G H <a href='#SkMatrix_I'>I</a> |               |  0  0  1 |

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a>:

| A B C | | <a href='#SkMatrix_preScale_2_sx'>sx</a>  0  0 |   | <a href='#SkMatrix_preScale_2_sx'>A</a>*<a href='#SkMatrix_preScale_2_sx'>sx</a> <a href='#SkMatrix_preScale_2_sx'>B</a>*<a href='#SkMatrix_preScale_2_sy'>sy</a> <a href='#SkMatrix_preScale_2_sy'>C</a> |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * <a href='SkMatrix_Reference#Matrix'>S</a>(<a href='#SkMatrix_preScale_2_sx'>sx</a>, <a href='#SkMatrix_preScale_2_sy'>sy</a>) = | <a href='#SkMatrix_preScale_2_sy'>D</a> <a href='#SkMatrix_preScale_2_sy'>E</a> <a href='#SkMatrix_preScale_2_sy'>F</a> | |  0 <a href='#SkMatrix_preScale_2_sy'>sy</a>  0 | = | <a href='#SkMatrix_preScale_2_sy'>D</a>*<a href='#SkMatrix_preScale_2_sx'>sx</a> <a href='#SkMatrix_preScale_2_sx'>E</a>*<a href='#SkMatrix_preScale_2_sy'>sy</a> <a href='#SkMatrix_preScale_2_sy'>F</a> |
| G H <a href='#SkMatrix_I'>I</a> | |  0  0  1 |   | <a href='#SkMatrix_I'>G</a>*<a href='#SkMatrix_preScale_2_sx'>sx</a> <a href='#SkMatrix_preScale_2_sx'>H</a>*<a href='#SkMatrix_preScale_2_sy'>sy</a> <a href='#SkMatrix_I'>I</a> |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_preScale_2_sx'><code><strong>sx</strong></code></a></td>
    <td>horizontal scale factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_preScale_2_sy'><code><strong>sy</strong></code></a></td>
    <td>vertical scale factor</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3edbdea8e43d06086abf33ec4a9b415b"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_postScale'>postScale</a> <a href='#SkMatrix_setScale'>setScale</a> <a href='#SkMatrix_MakeScale'>MakeScale</a>

<a name='SkMatrix_preRotate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_preRotate'>preRotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>degrees</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>px</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>py</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>multiplied</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>constructed</a> <a href='SkMatrix_Reference#SkMatrix'>from</a> <a href='SkMatrix_Reference#SkMatrix'>rotating</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkMatrix_preRotate_degrees'>degrees</a>
about pivot <a href='SkPoint_Reference#Point'>point</a> (<a href='#SkMatrix_preRotate_px'>px</a>, <a href='#SkMatrix_preRotate_py'>py</a>).
This can be thought of as rotating about a pivot <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>before</a> <a href='SkPoint_Reference#Point'>applying</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Positive <a href='#SkMatrix_preRotate_degrees'>degrees</a> <a href='#SkMatrix_preRotate_degrees'>rotates</a> <a href='#SkMatrix_preRotate_degrees'>clockwise</a>.

Given:

| A B C |                        | c -s dx |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> <a href='SkMatrix_Reference#Matrix'>F</a> |,  <a href='SkMatrix_Reference#Matrix'>R</a>(<a href='#SkMatrix_preRotate_degrees'>degrees</a>, <a href='#SkMatrix_preRotate_px'>px</a>, <a href='#SkMatrix_preRotate_py'>py</a>) = | <a href='#SkMatrix_preRotate_py'>s</a>  <a href='#SkMatrix_preRotate_py'>c</a> <a href='#SkMatrix_preRotate_py'>dy</a> |
| G H <a href='#SkMatrix_I'>I</a> |                        | 0  0  1 |

where

c  = cos(<a href='#SkMatrix_preRotate_degrees'>degrees</a>)
s  = sin(<a href='#SkMatrix_preRotate_degrees'>degrees</a>)
dx =  s * <a href='#SkMatrix_preRotate_py'>py</a> + (1 - <a href='#SkMatrix_preRotate_py'>c</a>) * <a href='#SkMatrix_preRotate_px'>px</a>
dy = -s * <a href='#SkMatrix_preRotate_px'>px</a> + (1 - <a href='#SkMatrix_preRotate_px'>c</a>) * <a href='#SkMatrix_preRotate_py'>py</a>

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a>:

| A B C | | c -s dx |   | Ac+Bs -As+Bc A*dx+B*dy+C |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * <a href='SkMatrix_Reference#Matrix'>R</a>(<a href='#SkMatrix_preRotate_degrees'>degrees</a>, <a href='#SkMatrix_preRotate_px'>px</a>, <a href='#SkMatrix_preRotate_py'>py</a>) = | <a href='#SkMatrix_preRotate_py'>D</a> <a href='#SkMatrix_preRotate_py'>E</a> <a href='#SkMatrix_preRotate_py'>F</a> | | <a href='#SkMatrix_preRotate_py'>s</a>  <a href='#SkMatrix_preRotate_py'>c</a> <a href='#SkMatrix_preRotate_py'>dy</a> | = | <a href='#SkMatrix_preRotate_py'>Dc</a>+<a href='#SkMatrix_preRotate_py'>Es</a> -<a href='#SkMatrix_preRotate_py'>Ds</a>+<a href='#SkMatrix_preRotate_py'>Ec</a> <a href='#SkMatrix_preRotate_py'>D</a>*<a href='#SkMatrix_preRotate_py'>dx</a>+<a href='#SkMatrix_preRotate_py'>E</a>*<a href='#SkMatrix_preRotate_py'>dy</a>+<a href='#SkMatrix_preRotate_py'>F</a> |
| G H <a href='#SkMatrix_I'>I</a> | | 0  0  1 |   | <a href='#SkMatrix_I'>Gc</a>+<a href='#SkMatrix_I'>Hs</a> -<a href='#SkMatrix_I'>Gs</a>+<a href='#SkMatrix_I'>Hc</a> <a href='#SkMatrix_I'>G</a>*<a href='#SkMatrix_I'>dx</a>+<a href='#SkMatrix_I'>H</a>*<a href='#SkMatrix_I'>dy</a>+<a href='#SkMatrix_I'>I</a> |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_preRotate_degrees'><code><strong>degrees</strong></code></a></td>
    <td>angle of axes relative to upright axes</td>
  </tr>
  <tr>    <td><a name='SkMatrix_preRotate_px'><code><strong>px</strong></code></a></td>
    <td>pivot on x-axis</td>
  </tr>
  <tr>    <td><a name='SkMatrix_preRotate_py'><code><strong>py</strong></code></a></td>
    <td>pivot on y-axis</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a70bb18d67c06a20ab514e7a47924e5a"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_postRotate'>postRotate</a> <a href='#SkMatrix_setRotate'>setRotate</a>

<a name='SkMatrix_preRotate_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_preRotate'>preRotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>degrees</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>multiplied</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>constructed</a> <a href='SkMatrix_Reference#SkMatrix'>from</a> <a href='SkMatrix_Reference#SkMatrix'>rotating</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkMatrix_preRotate_2_degrees'>degrees</a>
about pivot <a href='SkPoint_Reference#Point'>point</a> (0, 0).
This can be thought of as rotating about the origin before applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Positive <a href='#SkMatrix_preRotate_2_degrees'>degrees</a> <a href='#SkMatrix_preRotate_2_degrees'>rotates</a> <a href='#SkMatrix_preRotate_2_degrees'>clockwise</a>.

Given:

| A B C |                        | c -s 0 |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> <a href='SkMatrix_Reference#Matrix'>F</a> |,  <a href='SkMatrix_Reference#Matrix'>R</a>(<a href='#SkMatrix_preRotate_2_degrees'>degrees</a>, <a href='#SkMatrix_preRotate_2_degrees'>px</a>, <a href='#SkMatrix_preRotate_2_degrees'>py</a>) = | <a href='#SkMatrix_preRotate_2_degrees'>s</a>  <a href='#SkMatrix_preRotate_2_degrees'>c</a> 0 |
| G H <a href='#SkMatrix_I'>I</a> |                        | 0  0 1 |

where

c  = cos(<a href='#SkMatrix_preRotate_2_degrees'>degrees</a>)
s  = sin(<a href='#SkMatrix_preRotate_2_degrees'>degrees</a>)

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a>:

| A B C | | c -s 0 |   | Ac+Bs -As+Bc C |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * <a href='SkMatrix_Reference#Matrix'>R</a>(<a href='#SkMatrix_preRotate_2_degrees'>degrees</a>, <a href='#SkMatrix_preRotate_2_degrees'>px</a>, <a href='#SkMatrix_preRotate_2_degrees'>py</a>) = | <a href='#SkMatrix_preRotate_2_degrees'>D</a> <a href='#SkMatrix_preRotate_2_degrees'>E</a> <a href='#SkMatrix_preRotate_2_degrees'>F</a> | | <a href='#SkMatrix_preRotate_2_degrees'>s</a>  <a href='#SkMatrix_preRotate_2_degrees'>c</a> 0 | = | <a href='#SkMatrix_preRotate_2_degrees'>Dc</a>+<a href='#SkMatrix_preRotate_2_degrees'>Es</a> -<a href='#SkMatrix_preRotate_2_degrees'>Ds</a>+<a href='#SkMatrix_preRotate_2_degrees'>Ec</a> <a href='#SkMatrix_preRotate_2_degrees'>F</a> |
| G H <a href='#SkMatrix_I'>I</a> | | 0  0 1 |   | <a href='#SkMatrix_I'>Gc</a>+<a href='#SkMatrix_I'>Hs</a> -<a href='#SkMatrix_I'>Gs</a>+<a href='#SkMatrix_I'>Hc</a> <a href='#SkMatrix_I'>I</a> |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_preRotate_2_degrees'><code><strong>degrees</strong></code></a></td>
    <td>angle of axes relative to upright axes</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="5acd49bd931c79a808dd6c7cc0e92f72"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_postRotate'>postRotate</a> <a href='#SkMatrix_setRotate'>setRotate</a>

<a name='SkMatrix_preSkew'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_preSkew'>preSkew</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>kx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>ky</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>px</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>py</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>multiplied</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>constructed</a> <a href='SkMatrix_Reference#SkMatrix'>from</a> <a href='SkMatrix_Reference#SkMatrix'>skewing</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> (<a href='#SkMatrix_preSkew_kx'>kx</a>, <a href='#SkMatrix_preSkew_ky'>ky</a>)
about pivot <a href='SkPoint_Reference#Point'>point</a> (<a href='#SkMatrix_preSkew_px'>px</a>, <a href='#SkMatrix_preSkew_py'>py</a>).
This can be thought of as skewing about a pivot <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>before</a> <a href='SkPoint_Reference#Point'>applying</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Given:

| A B C |                       |  1 <a href='#SkMatrix_preSkew_kx'>kx</a> <a href='#SkMatrix_preSkew_kx'>dx</a> |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> <a href='SkMatrix_Reference#Matrix'>F</a> |,  <a href='SkMatrix_Reference#Matrix'>K</a>(<a href='#SkMatrix_preSkew_kx'>kx</a>, <a href='#SkMatrix_preSkew_ky'>ky</a>, <a href='#SkMatrix_preSkew_px'>px</a>, <a href='#SkMatrix_preSkew_py'>py</a>) = | <a href='#SkMatrix_preSkew_ky'>ky</a>  1 <a href='#SkMatrix_preSkew_ky'>dy</a> |
| G H <a href='#SkMatrix_I'>I</a> |                       |  0  0  1 |

where

dx = -<a href='#SkMatrix_preSkew_kx'>kx</a> * <a href='#SkMatrix_preSkew_py'>py</a>
dy = -<a href='#SkMatrix_preSkew_ky'>ky</a> * <a href='#SkMatrix_preSkew_px'>px</a>

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a>:

| A B C | |  1 <a href='#SkMatrix_preSkew_kx'>kx</a> <a href='#SkMatrix_preSkew_kx'>dx</a> |   | <a href='#SkMatrix_preSkew_kx'>A</a>+<a href='#SkMatrix_preSkew_kx'>B</a>*<a href='#SkMatrix_preSkew_ky'>ky</a> <a href='#SkMatrix_preSkew_ky'>A</a>*<a href='#SkMatrix_preSkew_kx'>kx</a>+<a href='#SkMatrix_preSkew_kx'>B</a> <a href='#SkMatrix_preSkew_kx'>A</a>*<a href='#SkMatrix_preSkew_kx'>dx</a>+<a href='#SkMatrix_preSkew_kx'>B</a>*<a href='#SkMatrix_preSkew_kx'>dy</a>+<a href='#SkMatrix_preSkew_kx'>C</a> |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * <a href='SkMatrix_Reference#Matrix'>K</a>(<a href='#SkMatrix_preSkew_kx'>kx</a>, <a href='#SkMatrix_preSkew_ky'>ky</a>, <a href='#SkMatrix_preSkew_px'>px</a>, <a href='#SkMatrix_preSkew_py'>py</a>) = | <a href='#SkMatrix_preSkew_py'>D</a> <a href='#SkMatrix_preSkew_py'>E</a> <a href='#SkMatrix_preSkew_py'>F</a> | | <a href='#SkMatrix_preSkew_ky'>ky</a>  1 <a href='#SkMatrix_preSkew_ky'>dy</a> | = | <a href='#SkMatrix_preSkew_ky'>D</a>+<a href='#SkMatrix_preSkew_ky'>E</a>*<a href='#SkMatrix_preSkew_ky'>ky</a> <a href='#SkMatrix_preSkew_ky'>D</a>*<a href='#SkMatrix_preSkew_kx'>kx</a>+<a href='#SkMatrix_preSkew_kx'>E</a> <a href='#SkMatrix_preSkew_kx'>D</a>*<a href='#SkMatrix_preSkew_kx'>dx</a>+<a href='#SkMatrix_preSkew_kx'>E</a>*<a href='#SkMatrix_preSkew_kx'>dy</a>+<a href='#SkMatrix_preSkew_kx'>F</a> |
| G H <a href='#SkMatrix_I'>I</a> | |  0  0  1 |   | <a href='#SkMatrix_I'>G</a>+<a href='#SkMatrix_I'>H</a>*<a href='#SkMatrix_preSkew_ky'>ky</a> <a href='#SkMatrix_preSkew_ky'>G</a>*<a href='#SkMatrix_preSkew_kx'>kx</a>+<a href='#SkMatrix_preSkew_kx'>H</a> <a href='#SkMatrix_preSkew_kx'>G</a>*<a href='#SkMatrix_preSkew_kx'>dx</a>+<a href='#SkMatrix_preSkew_kx'>H</a>*<a href='#SkMatrix_preSkew_kx'>dy</a>+<a href='#SkMatrix_I'>I</a> |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_preSkew_kx'><code><strong>kx</strong></code></a></td>
    <td>horizontal skew factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_preSkew_ky'><code><strong>ky</strong></code></a></td>
    <td>vertical skew factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_preSkew_px'><code><strong>px</strong></code></a></td>
    <td>pivot on x-axis</td>
  </tr>
  <tr>    <td><a name='SkMatrix_preSkew_py'><code><strong>py</strong></code></a></td>
    <td>pivot on y-axis</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="199a18ad61d702664ce6df1d7037aa48"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_postSkew'>postSkew</a> <a href='#SkMatrix_setSkew'>setSkew</a>

<a name='SkMatrix_preSkew_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_preSkew'>preSkew</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>kx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>ky</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>multiplied</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>constructed</a> <a href='SkMatrix_Reference#SkMatrix'>from</a> <a href='SkMatrix_Reference#SkMatrix'>skewing</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> (<a href='#SkMatrix_preSkew_2_kx'>kx</a>, <a href='#SkMatrix_preSkew_2_ky'>ky</a>)
about pivot <a href='SkPoint_Reference#Point'>point</a> (0, 0).
This can be thought of as skewing about the origin before applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Given:

| A B C |               |  1 <a href='#SkMatrix_preSkew_2_kx'>kx</a> 0 |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> <a href='SkMatrix_Reference#Matrix'>F</a> |,  <a href='SkMatrix_Reference#Matrix'>K</a>(<a href='#SkMatrix_preSkew_2_kx'>kx</a>, <a href='#SkMatrix_preSkew_2_ky'>ky</a>) = | <a href='#SkMatrix_preSkew_2_ky'>ky</a>  1 0 |
| G H <a href='#SkMatrix_I'>I</a> |               |  0  0 1 |

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a>:

| A B C | |  1 <a href='#SkMatrix_preSkew_2_kx'>kx</a> 0 |   | <a href='#SkMatrix_preSkew_2_kx'>A</a>+<a href='#SkMatrix_preSkew_2_kx'>B</a>*<a href='#SkMatrix_preSkew_2_ky'>ky</a> <a href='#SkMatrix_preSkew_2_ky'>A</a>*<a href='#SkMatrix_preSkew_2_kx'>kx</a>+<a href='#SkMatrix_preSkew_2_kx'>B</a> <a href='#SkMatrix_preSkew_2_kx'>C</a> |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * <a href='SkMatrix_Reference#Matrix'>K</a>(<a href='#SkMatrix_preSkew_2_kx'>kx</a>, <a href='#SkMatrix_preSkew_2_ky'>ky</a>) = | <a href='#SkMatrix_preSkew_2_ky'>D</a> <a href='#SkMatrix_preSkew_2_ky'>E</a> <a href='#SkMatrix_preSkew_2_ky'>F</a> | | <a href='#SkMatrix_preSkew_2_ky'>ky</a>  1 0 | = | <a href='#SkMatrix_preSkew_2_ky'>D</a>+<a href='#SkMatrix_preSkew_2_ky'>E</a>*<a href='#SkMatrix_preSkew_2_ky'>ky</a> <a href='#SkMatrix_preSkew_2_ky'>D</a>*<a href='#SkMatrix_preSkew_2_kx'>kx</a>+<a href='#SkMatrix_preSkew_2_kx'>E</a> <a href='#SkMatrix_preSkew_2_kx'>F</a> |
| G H <a href='#SkMatrix_I'>I</a> | |  0  0 1 |   | <a href='#SkMatrix_I'>G</a>+<a href='#SkMatrix_I'>H</a>*<a href='#SkMatrix_preSkew_2_ky'>ky</a> <a href='#SkMatrix_preSkew_2_ky'>G</a>*<a href='#SkMatrix_preSkew_2_kx'>kx</a>+<a href='#SkMatrix_preSkew_2_kx'>H</a> <a href='#SkMatrix_I'>I</a> |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_preSkew_2_kx'><code><strong>kx</strong></code></a></td>
    <td>horizontal skew factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_preSkew_2_ky'><code><strong>ky</strong></code></a></td>
    <td>vertical skew factor</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="e100c543869fe8fd516ba69de79444ba"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_postSkew'>postSkew</a> <a href='#SkMatrix_setSkew'>setSkew</a>

<a name='SkMatrix_preConcat'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_preConcat'>preConcat</a>(<a href='#SkMatrix_preConcat'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#SkMatrix'>other</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>multiplied</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_preConcat_other'>other</a>.
This can be thought of mapping by <a href='#SkMatrix_preConcat_other'>other</a> <a href='#SkMatrix_preConcat_other'>before</a> <a href='#SkMatrix_preConcat_other'>applying</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Given:

| A B C |          | J K L |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> <a href='SkMatrix_Reference#Matrix'>F</a> |, <a href='#SkMatrix_preConcat_other'>other</a> = | <a href='#SkMatrix_preConcat_other'>M</a> <a href='#SkMatrix_preConcat_other'>N</a> <a href='#SkMatrix_preConcat_other'>O</a> |
| G H <a href='#SkMatrix_I'>I</a> |          | <a href='#SkMatrix_I'>P</a> <a href='#SkMatrix_I'>Q</a> <a href='#SkMatrix_I'>R</a> |

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a>:

| A B C |   | J K L |   | AJ+BM+CP AK+BN+CQ AL+BO+CR |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * <a href='#SkMatrix_preConcat_other'>other</a> = | <a href='#SkMatrix_preConcat_other'>D</a> <a href='#SkMatrix_preConcat_other'>E</a> <a href='#SkMatrix_preConcat_other'>F</a> | * | <a href='#SkMatrix_preConcat_other'>M</a> <a href='#SkMatrix_preConcat_other'>N</a> <a href='#SkMatrix_preConcat_other'>O</a> | = | <a href='#SkMatrix_preConcat_other'>DJ</a>+<a href='#SkMatrix_preConcat_other'>EM</a>+<a href='#SkMatrix_preConcat_other'>FP</a> <a href='#SkMatrix_preConcat_other'>DK</a>+<a href='#SkMatrix_preConcat_other'>EN</a>+<a href='#SkMatrix_preConcat_other'>FQ</a> <a href='#SkMatrix_preConcat_other'>DL</a>+<a href='#SkMatrix_preConcat_other'>EO</a>+<a href='#SkMatrix_preConcat_other'>FR</a> |
| G H <a href='#SkMatrix_I'>I</a> |   | <a href='#SkMatrix_I'>P</a> <a href='#SkMatrix_I'>Q</a> <a href='#SkMatrix_I'>R</a> |   | <a href='#SkMatrix_I'>GJ</a>+<a href='#SkMatrix_I'>HM</a>+<a href='#SkMatrix_I'>IP</a> <a href='#SkMatrix_I'>GK</a>+<a href='#SkMatrix_I'>HN</a>+<a href='#SkMatrix_I'>IQ</a> <a href='#SkMatrix_I'>GL</a>+<a href='#SkMatrix_I'>HO</a>+<a href='#SkMatrix_I'>IR</a> |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_preConcat_other'><code><strong>other</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>on</a>  <a href='SkMatrix_Reference#SkMatrix'>right side</a> <a href='SkMatrix_Reference#SkMatrix'>of</a> <a href='SkMatrix_Reference#SkMatrix'>multiply</a> <a href='SkMatrix_Reference#SkMatrix'>expression</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="b07e62298e7b0ab5683db199faffceb2"><div><a href='#SkMatrix_setPolyToPoly'>setPolyToPoly</a> <a href='#SkMatrix_setPolyToPoly'>creates</a> <a href='#SkMatrix_setPolyToPoly'>perspective</a> <a href='SkMatrix_Reference#Matrix'>matrices</a>, <a href='SkMatrix_Reference#Matrix'>one</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>inverse</a> <a href='SkMatrix_Reference#Matrix'>of</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='#SkMatrix_preConcat_other'>other</a>.
<a href='#SkMatrix_preConcat_other'>Multiplying</a> <a href='#SkMatrix_preConcat_other'>the</a> <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>by</a> <a href='SkMatrix_Reference#Matrix'>its</a> <a href='SkMatrix_Reference#Matrix'>inverse</a> <a href='SkMatrix_Reference#Matrix'>turns</a> <a href='SkMatrix_Reference#Matrix'>into</a> <a href='SkMatrix_Reference#Matrix'>an</a> <a href='SkMatrix_Reference#Matrix'>identity</a> <a href='SkMatrix_Reference#Matrix'>matrix</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkMatrix_postConcat'>postConcat</a> <a href='#SkMatrix_setConcat'>setConcat</a> <a href='#SkMatrix_Concat'>Concat</a>

<a name='SkMatrix_postTranslate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_postTranslate'>postTranslate</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>constructed</a> <a href='SkMatrix_Reference#SkMatrix'>from</a> <a href='SkMatrix_Reference#SkMatrix'>translation</a> (<a href='#SkMatrix_postTranslate_dx'>dx</a>, <a href='#SkMatrix_postTranslate_dy'>dy</a>) <a href='#SkMatrix_postTranslate_dy'>multiplied</a> <a href='#SkMatrix_postTranslate_dy'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
This can be thought of as moving the <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>be</a> <a href='SkPoint_Reference#Point'>mapped</a> <a href='SkPoint_Reference#Point'>after</a> <a href='SkPoint_Reference#Point'>applying</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Given:

| J K L |               | 1 0 <a href='#SkMatrix_postTranslate_dx'>dx</a> |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>M</a> <a href='SkMatrix_Reference#Matrix'>N</a> <a href='SkMatrix_Reference#Matrix'>O</a> |,  <a href='SkMatrix_Reference#Matrix'>T</a>(<a href='#SkMatrix_postTranslate_dx'>dx</a>, <a href='#SkMatrix_postTranslate_dy'>dy</a>) = | 0 1 <a href='#SkMatrix_postTranslate_dy'>dy</a> |
| P Q R |               | 0 0  1 |

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a>:

| 1 0 <a href='#SkMatrix_postTranslate_dx'>dx</a> | | <a href='#SkMatrix_postTranslate_dx'>J</a> <a href='#SkMatrix_postTranslate_dx'>K</a> <a href='#SkMatrix_postTranslate_dx'>L</a> |   | <a href='#SkMatrix_postTranslate_dx'>J</a>+<a href='#SkMatrix_postTranslate_dx'>dx</a>*<a href='#SkMatrix_postTranslate_dx'>P</a> <a href='#SkMatrix_postTranslate_dx'>K</a>+<a href='#SkMatrix_postTranslate_dx'>dx</a>*<a href='#SkMatrix_postTranslate_dx'>Q</a> <a href='#SkMatrix_postTranslate_dx'>L</a>+<a href='#SkMatrix_postTranslate_dx'>dx</a>*<a href='#SkMatrix_postTranslate_dx'>R</a> |
T(<a href='#SkMatrix_postTranslate_dx'>dx</a>, <a href='#SkMatrix_postTranslate_dy'>dy</a>) * <a href='SkMatrix_Reference#Matrix'>Matrix</a> = | 0 1 <a href='#SkMatrix_postTranslate_dy'>dy</a> | | <a href='#SkMatrix_postTranslate_dy'>M</a> <a href='#SkMatrix_postTranslate_dy'>N</a> <a href='#SkMatrix_postTranslate_dy'>O</a> | = | <a href='#SkMatrix_postTranslate_dy'>M</a>+<a href='#SkMatrix_postTranslate_dy'>dy</a>*<a href='#SkMatrix_postTranslate_dy'>P</a> <a href='#SkMatrix_postTranslate_dy'>N</a>+<a href='#SkMatrix_postTranslate_dy'>dy</a>*<a href='#SkMatrix_postTranslate_dy'>Q</a> <a href='#SkMatrix_postTranslate_dy'>O</a>+<a href='#SkMatrix_postTranslate_dy'>dy</a>*<a href='#SkMatrix_postTranslate_dy'>R</a> |
| 0 0  1 | | P Q R |   |      P      Q      R |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_postTranslate_dx'><code><strong>dx</strong></code></a></td>
    <td>x-axis translation after applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_postTranslate_dy'><code><strong>dy</strong></code></a></td>
    <td>y-axis translation after applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f5144ef4bd7cea294fad2f756ed335af"><div>Compare with <a href='#SkMatrix_preTranslate'>preTranslate</a> <a href='#SkMatrix_preTranslate'>example</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkMatrix_preTranslate'>preTranslate</a> <a href='#SkMatrix_setTranslate'>setTranslate</a> <a href='#SkMatrix_MakeTrans'>MakeTrans</a>

<a name='SkMatrix_postScale'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_postScale'>postScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sy</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>px</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>py</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>constructed</a> <a href='SkMatrix_Reference#SkMatrix'>from</a> <a href='SkMatrix_Reference#SkMatrix'>scaling</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> (<a href='#SkMatrix_postScale_sx'>sx</a>, <a href='#SkMatrix_postScale_sy'>sy</a>) <a href='#SkMatrix_postScale_sy'>about</a> <a href='#SkMatrix_postScale_sy'>pivot</a> <a href='SkPoint_Reference#Point'>point</a>
(<a href='#SkMatrix_postScale_px'>px</a>, <a href='#SkMatrix_postScale_py'>py</a>), <a href='#SkMatrix_postScale_py'>multiplied</a> <a href='#SkMatrix_postScale_py'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
This can be thought of as scaling about a pivot <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>after</a> <a href='SkPoint_Reference#Point'>applying</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Given:

| J K L |                       | <a href='#SkMatrix_postScale_sx'>sx</a>  0 <a href='#SkMatrix_postScale_sx'>dx</a> |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>M</a> <a href='SkMatrix_Reference#Matrix'>N</a> <a href='SkMatrix_Reference#Matrix'>O</a> |,  <a href='SkMatrix_Reference#Matrix'>S</a>(<a href='#SkMatrix_postScale_sx'>sx</a>, <a href='#SkMatrix_postScale_sy'>sy</a>, <a href='#SkMatrix_postScale_px'>px</a>, <a href='#SkMatrix_postScale_py'>py</a>) = |  0 <a href='#SkMatrix_postScale_sy'>sy</a> <a href='#SkMatrix_postScale_sy'>dy</a> |
| P Q R |                       |  0  0  1 |

where

dx = <a href='#SkMatrix_postScale_px'>px</a> - <a href='#SkMatrix_postScale_sx'>sx</a> * <a href='#SkMatrix_postScale_px'>px</a>
dy = <a href='#SkMatrix_postScale_py'>py</a> - <a href='#SkMatrix_postScale_sy'>sy</a> * <a href='#SkMatrix_postScale_py'>py</a>

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a>:

| <a href='#SkMatrix_postScale_sx'>sx</a>  0 <a href='#SkMatrix_postScale_sx'>dx</a> | | <a href='#SkMatrix_postScale_sx'>J</a> <a href='#SkMatrix_postScale_sx'>K</a> <a href='#SkMatrix_postScale_sx'>L</a> |   | <a href='#SkMatrix_postScale_sx'>sx</a>*<a href='#SkMatrix_postScale_sx'>J</a>+<a href='#SkMatrix_postScale_sx'>dx</a>*<a href='#SkMatrix_postScale_sx'>P</a> <a href='#SkMatrix_postScale_sx'>sx</a>*<a href='#SkMatrix_postScale_sx'>K</a>+<a href='#SkMatrix_postScale_sx'>dx</a>*<a href='#SkMatrix_postScale_sx'>Q</a> <a href='#SkMatrix_postScale_sx'>sx</a>*<a href='#SkMatrix_postScale_sx'>L</a>+<a href='#SkMatrix_postScale_sx'>dx</a>+<a href='#SkMatrix_postScale_sx'>R</a> |
S(<a href='#SkMatrix_postScale_sx'>sx</a>, <a href='#SkMatrix_postScale_sy'>sy</a>, <a href='#SkMatrix_postScale_px'>px</a>, <a href='#SkMatrix_postScale_py'>py</a>) * <a href='SkMatrix_Reference#Matrix'>Matrix</a> = |  0 <a href='#SkMatrix_postScale_sy'>sy</a> <a href='#SkMatrix_postScale_sy'>dy</a> | | <a href='#SkMatrix_postScale_sy'>M</a> <a href='#SkMatrix_postScale_sy'>N</a> <a href='#SkMatrix_postScale_sy'>O</a> | = | <a href='#SkMatrix_postScale_sy'>sy</a>*<a href='#SkMatrix_postScale_sy'>M</a>+<a href='#SkMatrix_postScale_sy'>dy</a>*<a href='#SkMatrix_postScale_sy'>P</a> <a href='#SkMatrix_postScale_sy'>sy</a>*<a href='#SkMatrix_postScale_sy'>N</a>+<a href='#SkMatrix_postScale_sy'>dy</a>*<a href='#SkMatrix_postScale_sy'>Q</a> <a href='#SkMatrix_postScale_sy'>sy</a>*<a href='#SkMatrix_postScale_sy'>O</a>+<a href='#SkMatrix_postScale_sy'>dy</a>*<a href='#SkMatrix_postScale_sy'>R</a> |
|  0  0  1 | | P Q R |   |         P         Q         R |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_postScale_sx'><code><strong>sx</strong></code></a></td>
    <td>horizontal scale factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_postScale_sy'><code><strong>sy</strong></code></a></td>
    <td>vertical scale factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_postScale_px'><code><strong>px</strong></code></a></td>
    <td>pivot on x-axis</td>
  </tr>
  <tr>    <td><a name='SkMatrix_postScale_py'><code><strong>py</strong></code></a></td>
    <td>pivot on y-axis</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ed3aa18ba0ea95c85cc49aa3829fe384"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_preScale'>preScale</a> <a href='#SkMatrix_setScale'>setScale</a> <a href='#SkMatrix_MakeScale'>MakeScale</a>

<a name='SkMatrix_postScale_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_postScale'>postScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sy</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>constructed</a> <a href='SkMatrix_Reference#SkMatrix'>from</a> <a href='SkMatrix_Reference#SkMatrix'>scaling</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> (<a href='#SkMatrix_postScale_2_sx'>sx</a>, <a href='#SkMatrix_postScale_2_sy'>sy</a>) <a href='#SkMatrix_postScale_2_sy'>about</a> <a href='#SkMatrix_postScale_2_sy'>pivot</a> <a href='SkPoint_Reference#Point'>point</a>
(0, 0), multiplied by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
This can be thought of as scaling about the origin after applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Given:

| J K L |               | <a href='#SkMatrix_postScale_2_sx'>sx</a>  0  0 |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>M</a> <a href='SkMatrix_Reference#Matrix'>N</a> <a href='SkMatrix_Reference#Matrix'>O</a> |,  <a href='SkMatrix_Reference#Matrix'>S</a>(<a href='#SkMatrix_postScale_2_sx'>sx</a>, <a href='#SkMatrix_postScale_2_sy'>sy</a>) = |  0 <a href='#SkMatrix_postScale_2_sy'>sy</a>  0 |
| P Q R |               |  0  0  1 |

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a>:

| <a href='#SkMatrix_postScale_2_sx'>sx</a>  0  0 | | <a href='#SkMatrix_postScale_2_sx'>J</a> <a href='#SkMatrix_postScale_2_sx'>K</a> <a href='#SkMatrix_postScale_2_sx'>L</a> |   | <a href='#SkMatrix_postScale_2_sx'>sx</a>*<a href='#SkMatrix_postScale_2_sx'>J</a> <a href='#SkMatrix_postScale_2_sx'>sx</a>*<a href='#SkMatrix_postScale_2_sx'>K</a> <a href='#SkMatrix_postScale_2_sx'>sx</a>*<a href='#SkMatrix_postScale_2_sx'>L</a> |
S(<a href='#SkMatrix_postScale_2_sx'>sx</a>, <a href='#SkMatrix_postScale_2_sy'>sy</a>) * <a href='SkMatrix_Reference#Matrix'>Matrix</a> = |  0 <a href='#SkMatrix_postScale_2_sy'>sy</a>  0 | | <a href='#SkMatrix_postScale_2_sy'>M</a> <a href='#SkMatrix_postScale_2_sy'>N</a> <a href='#SkMatrix_postScale_2_sy'>O</a> | = | <a href='#SkMatrix_postScale_2_sy'>sy</a>*<a href='#SkMatrix_postScale_2_sy'>M</a> <a href='#SkMatrix_postScale_2_sy'>sy</a>*<a href='#SkMatrix_postScale_2_sy'>N</a> <a href='#SkMatrix_postScale_2_sy'>sy</a>*<a href='#SkMatrix_postScale_2_sy'>O</a> |
|  0  0  1 | | P Q R |   |    P    Q    R |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_postScale_2_sx'><code><strong>sx</strong></code></a></td>
    <td>horizontal scale factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_postScale_2_sy'><code><strong>sy</strong></code></a></td>
    <td>vertical scale factor</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1931017698766a67d3a26423453b8095"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_preScale'>preScale</a> <a href='#SkMatrix_setScale'>setScale</a> <a href='#SkMatrix_MakeScale'>MakeScale</a>

<a name='SkMatrix_postIDiv'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_postIDiv'>postIDiv</a>(<a href='#SkMatrix_postIDiv'>int</a> <a href='#SkMatrix_postIDiv'>divx</a>, <a href='#SkMatrix_postIDiv'>int</a> <a href='#SkMatrix_postIDiv'>divy</a>)
</pre>

Sets <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>to</a> <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>constructed</a> <a href='SkMatrix_Reference#Matrix'>from</a> <a href='SkMatrix_Reference#Matrix'>scaling</a> <a href='SkMatrix_Reference#Matrix'>by</a> (1/<a href='#SkMatrix_postIDiv_divx'>divx</a>, 1/<a href='#SkMatrix_postIDiv_divy'>divy</a>),
<a href='#SkMatrix_postIDiv_divy'>multiplied</a> <a href='#SkMatrix_postIDiv_divy'>by</a> <a href='SkMatrix_Reference#Matrix'>Matrix</a>.

<a href='SkMatrix_Reference#Matrix'>Returns</a> <a href='SkMatrix_Reference#Matrix'>false</a> <a href='SkMatrix_Reference#Matrix'>if</a> <a href='SkMatrix_Reference#Matrix'>either</a> <a href='#SkMatrix_postIDiv_divx'>divx</a> <a href='#SkMatrix_postIDiv_divx'>or</a> <a href='#SkMatrix_postIDiv_divy'>divy</a> <a href='#SkMatrix_postIDiv_divy'>is</a> <a href='#SkMatrix_postIDiv_divy'>zero</a>.

<a href='#SkMatrix_postIDiv_divy'>Given</a>:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | J K L |                   | sx  0  0 |
Matrix = | M N O |,  I(divx, divy) = |  0 sy  0 |
         | P Q R |                   |  0  0  1 |
</pre>

where

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sx = 1 / divx
sy = 1 / divy
</pre>

sets <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>to</a>:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
                         | sx  0  0 | | J K L |   | sx*J sx*K sx*L |
I(divx, divy) * Matrix = |  0 sy  0 | | M N O | = | sy*M sy*N sy*O |
                         |  0  0  1 | | P Q R |   |    P    Q    R |
</pre>

### Parameters

<table>  <tr>    <td><a name='SkMatrix_postIDiv_divx'><code><strong>divx</strong></code></a></td>
    <td>integer divisor for inverse scale on x-axis</td>
  </tr>
  <tr>    <td><a name='SkMatrix_postIDiv_divy'><code><strong>divy</strong></code></a></td>
    <td>integer divisor for inverse scale on y-axis</td>
  </tr>
</table>

### Return Value

true on successful scale

### Example

<div><fiddle-embed name="e6ad0bd2999613d9e4758b661d45070c"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_postScale'>postScale</a> <a href='#SkMatrix_MakeScale'>MakeScale</a>

<a name='SkMatrix_postRotate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_postRotate'>postRotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>degrees</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>px</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>py</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>constructed</a> <a href='SkMatrix_Reference#SkMatrix'>from</a> <a href='SkMatrix_Reference#SkMatrix'>rotating</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkMatrix_postRotate_degrees'>degrees</a> <a href='#SkMatrix_postRotate_degrees'>about</a> <a href='#SkMatrix_postRotate_degrees'>pivot</a> <a href='SkPoint_Reference#Point'>point</a>
(<a href='#SkMatrix_postRotate_px'>px</a>, <a href='#SkMatrix_postRotate_py'>py</a>), <a href='#SkMatrix_postRotate_py'>multiplied</a> <a href='#SkMatrix_postRotate_py'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
This can be thought of as rotating about a pivot <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>after</a> <a href='SkPoint_Reference#Point'>applying</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Positive <a href='#SkMatrix_postRotate_degrees'>degrees</a> <a href='#SkMatrix_postRotate_degrees'>rotates</a> <a href='#SkMatrix_postRotate_degrees'>clockwise</a>.

Given:

| J K L |                        | c -s dx |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>M</a> <a href='SkMatrix_Reference#Matrix'>N</a> <a href='SkMatrix_Reference#Matrix'>O</a> |,  <a href='SkMatrix_Reference#Matrix'>R</a>(<a href='#SkMatrix_postRotate_degrees'>degrees</a>, <a href='#SkMatrix_postRotate_px'>px</a>, <a href='#SkMatrix_postRotate_py'>py</a>) = | <a href='#SkMatrix_postRotate_py'>s</a>  <a href='#SkMatrix_postRotate_py'>c</a> <a href='#SkMatrix_postRotate_py'>dy</a> |
| P Q R |                        | 0  0  1 |

where

c  = cos(<a href='#SkMatrix_postRotate_degrees'>degrees</a>)
s  = sin(<a href='#SkMatrix_postRotate_degrees'>degrees</a>)
dx =  s * <a href='#SkMatrix_postRotate_py'>py</a> + (1 - <a href='#SkMatrix_postRotate_py'>c</a>) * <a href='#SkMatrix_postRotate_px'>px</a>
dy = -s * <a href='#SkMatrix_postRotate_px'>px</a> + (1 - <a href='#SkMatrix_postRotate_px'>c</a>) * <a href='#SkMatrix_postRotate_py'>py</a>

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a>:

|c -s dx| |J K L|   |cJ-sM+dx*P cK-sN+dx*Q cL-sO+dx+R|
R(<a href='#SkMatrix_postRotate_degrees'>degrees</a>, <a href='#SkMatrix_postRotate_px'>px</a>, <a href='#SkMatrix_postRotate_py'>py</a>) * <a href='SkMatrix_Reference#Matrix'>Matrix</a> = |<a href='SkMatrix_Reference#Matrix'>s</a>  <a href='SkMatrix_Reference#Matrix'>c</a> <a href='SkMatrix_Reference#Matrix'>dy</a>| |<a href='SkMatrix_Reference#Matrix'>M</a> <a href='SkMatrix_Reference#Matrix'>N</a> <a href='SkMatrix_Reference#Matrix'>O</a>| = |<a href='SkMatrix_Reference#Matrix'>sJ</a>+<a href='SkMatrix_Reference#Matrix'>cM</a>+<a href='SkMatrix_Reference#Matrix'>dy</a>*<a href='SkMatrix_Reference#Matrix'>P</a> <a href='SkMatrix_Reference#Matrix'>sK</a>+<a href='SkMatrix_Reference#Matrix'>cN</a>+<a href='SkMatrix_Reference#Matrix'>dy</a>*<a href='SkMatrix_Reference#Matrix'>Q</a> <a href='SkMatrix_Reference#Matrix'>sL</a>+<a href='SkMatrix_Reference#Matrix'>cO</a>+<a href='SkMatrix_Reference#Matrix'>dy</a>*<a href='SkMatrix_Reference#Matrix'>R</a>|
|0  0  1| |P Q R|   |         P          Q          R|

### Parameters

<table>  <tr>    <td><a name='SkMatrix_postRotate_degrees'><code><strong>degrees</strong></code></a></td>
    <td>angle of axes relative to upright axes</td>
  </tr>
  <tr>    <td><a name='SkMatrix_postRotate_px'><code><strong>px</strong></code></a></td>
    <td>pivot on x-axis</td>
  </tr>
  <tr>    <td><a name='SkMatrix_postRotate_py'><code><strong>py</strong></code></a></td>
    <td>pivot on y-axis</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="e09194ee48a81e7b375ade473d340f0d"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_preRotate'>preRotate</a> <a href='#SkMatrix_setRotate'>setRotate</a>

<a name='SkMatrix_postRotate_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_postRotate'>postRotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>degrees</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>constructed</a> <a href='SkMatrix_Reference#SkMatrix'>from</a> <a href='SkMatrix_Reference#SkMatrix'>rotating</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='#SkMatrix_postRotate_2_degrees'>degrees</a> <a href='#SkMatrix_postRotate_2_degrees'>about</a> <a href='#SkMatrix_postRotate_2_degrees'>pivot</a> <a href='SkPoint_Reference#Point'>point</a>
(0, 0), multiplied by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
This can be thought of as rotating about the origin after applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Positive <a href='#SkMatrix_postRotate_2_degrees'>degrees</a> <a href='#SkMatrix_postRotate_2_degrees'>rotates</a> <a href='#SkMatrix_postRotate_2_degrees'>clockwise</a>.

Given:

| J K L |                        | c -s 0 |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>M</a> <a href='SkMatrix_Reference#Matrix'>N</a> <a href='SkMatrix_Reference#Matrix'>O</a> |,  <a href='SkMatrix_Reference#Matrix'>R</a>(<a href='#SkMatrix_postRotate_2_degrees'>degrees</a>, <a href='#SkMatrix_postRotate_2_degrees'>px</a>, <a href='#SkMatrix_postRotate_2_degrees'>py</a>) = | <a href='#SkMatrix_postRotate_2_degrees'>s</a>  <a href='#SkMatrix_postRotate_2_degrees'>c</a> 0 |
| P Q R |                        | 0  0 1 |

where

c  = cos(<a href='#SkMatrix_postRotate_2_degrees'>degrees</a>)
s  = sin(<a href='#SkMatrix_postRotate_2_degrees'>degrees</a>)

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a>:

| c -s dx | | J K L |   | cJ-sM cK-sN cL-sO |
R(<a href='#SkMatrix_postRotate_2_degrees'>degrees</a>, <a href='#SkMatrix_postRotate_2_degrees'>px</a>, <a href='#SkMatrix_postRotate_2_degrees'>py</a>) * <a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>s</a>  <a href='SkMatrix_Reference#Matrix'>c</a> <a href='SkMatrix_Reference#Matrix'>dy</a> | | <a href='SkMatrix_Reference#Matrix'>M</a> <a href='SkMatrix_Reference#Matrix'>N</a> <a href='SkMatrix_Reference#Matrix'>O</a> | = | <a href='SkMatrix_Reference#Matrix'>sJ</a>+<a href='SkMatrix_Reference#Matrix'>cM</a> <a href='SkMatrix_Reference#Matrix'>sK</a>+<a href='SkMatrix_Reference#Matrix'>cN</a> <a href='SkMatrix_Reference#Matrix'>sL</a>+<a href='SkMatrix_Reference#Matrix'>cO</a> |
| 0  0  1 | | P Q R |   |     P     Q     R |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_postRotate_2_degrees'><code><strong>degrees</strong></code></a></td>
    <td>angle of axes relative to upright axes</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="52e4c53e26971af5576b30de60fa70c2"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_preRotate'>preRotate</a> <a href='#SkMatrix_setRotate'>setRotate</a>

<a name='SkMatrix_postSkew'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_postSkew'>postSkew</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>kx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>ky</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>px</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>py</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>constructed</a> <a href='SkMatrix_Reference#SkMatrix'>from</a> <a href='SkMatrix_Reference#SkMatrix'>skewing</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> (<a href='#SkMatrix_postSkew_kx'>kx</a>, <a href='#SkMatrix_postSkew_ky'>ky</a>) <a href='#SkMatrix_postSkew_ky'>about</a> <a href='#SkMatrix_postSkew_ky'>pivot</a> <a href='SkPoint_Reference#Point'>point</a>
(<a href='#SkMatrix_postSkew_px'>px</a>, <a href='#SkMatrix_postSkew_py'>py</a>), <a href='#SkMatrix_postSkew_py'>multiplied</a> <a href='#SkMatrix_postSkew_py'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
This can be thought of as skewing about a pivot <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>after</a> <a href='SkPoint_Reference#Point'>applying</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Given:

| J K L |                       |  1 <a href='#SkMatrix_postSkew_kx'>kx</a> <a href='#SkMatrix_postSkew_kx'>dx</a> |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>M</a> <a href='SkMatrix_Reference#Matrix'>N</a> <a href='SkMatrix_Reference#Matrix'>O</a> |,  <a href='SkMatrix_Reference#Matrix'>K</a>(<a href='#SkMatrix_postSkew_kx'>kx</a>, <a href='#SkMatrix_postSkew_ky'>ky</a>, <a href='#SkMatrix_postSkew_px'>px</a>, <a href='#SkMatrix_postSkew_py'>py</a>) = | <a href='#SkMatrix_postSkew_ky'>ky</a>  1 <a href='#SkMatrix_postSkew_ky'>dy</a> |
| P Q R |                       |  0  0  1 |

where

dx = -<a href='#SkMatrix_postSkew_kx'>kx</a> * <a href='#SkMatrix_postSkew_py'>py</a>
dy = -<a href='#SkMatrix_postSkew_ky'>ky</a> * <a href='#SkMatrix_postSkew_px'>px</a>

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a>:

| 1 <a href='#SkMatrix_postSkew_kx'>kx</a> <a href='#SkMatrix_postSkew_kx'>dx</a>| |<a href='#SkMatrix_postSkew_kx'>J</a> <a href='#SkMatrix_postSkew_kx'>K</a> <a href='#SkMatrix_postSkew_kx'>L</a>|   |<a href='#SkMatrix_postSkew_kx'>J</a>+<a href='#SkMatrix_postSkew_kx'>kx</a>*<a href='#SkMatrix_postSkew_kx'>M</a>+<a href='#SkMatrix_postSkew_kx'>dx</a>*<a href='#SkMatrix_postSkew_kx'>P</a> <a href='#SkMatrix_postSkew_kx'>K</a>+<a href='#SkMatrix_postSkew_kx'>kx</a>*<a href='#SkMatrix_postSkew_kx'>N</a>+<a href='#SkMatrix_postSkew_kx'>dx</a>*<a href='#SkMatrix_postSkew_kx'>Q</a> <a href='#SkMatrix_postSkew_kx'>L</a>+<a href='#SkMatrix_postSkew_kx'>kx</a>*<a href='#SkMatrix_postSkew_kx'>O</a>+<a href='#SkMatrix_postSkew_kx'>dx</a>+<a href='#SkMatrix_postSkew_kx'>R</a>|
K(<a href='#SkMatrix_postSkew_kx'>kx</a>, <a href='#SkMatrix_postSkew_ky'>ky</a>, <a href='#SkMatrix_postSkew_px'>px</a>, <a href='#SkMatrix_postSkew_py'>py</a>) * <a href='SkMatrix_Reference#Matrix'>Matrix</a> = |<a href='#SkMatrix_postSkew_ky'>ky</a>  1 <a href='#SkMatrix_postSkew_ky'>dy</a>| |<a href='#SkMatrix_postSkew_ky'>M</a> <a href='#SkMatrix_postSkew_ky'>N</a> <a href='#SkMatrix_postSkew_ky'>O</a>| = |<a href='#SkMatrix_postSkew_ky'>ky</a>*<a href='#SkMatrix_postSkew_ky'>J</a>+<a href='#SkMatrix_postSkew_ky'>M</a>+<a href='#SkMatrix_postSkew_ky'>dy</a>*<a href='#SkMatrix_postSkew_ky'>P</a> <a href='#SkMatrix_postSkew_ky'>ky</a>*<a href='#SkMatrix_postSkew_ky'>K</a>+<a href='#SkMatrix_postSkew_ky'>N</a>+<a href='#SkMatrix_postSkew_ky'>dy</a>*<a href='#SkMatrix_postSkew_ky'>Q</a> <a href='#SkMatrix_postSkew_ky'>ky</a>*<a href='#SkMatrix_postSkew_ky'>L</a>+<a href='#SkMatrix_postSkew_ky'>O</a>+<a href='#SkMatrix_postSkew_ky'>dy</a>*<a href='#SkMatrix_postSkew_ky'>R</a>|
| 0  0  1| |P Q R|   |          P           Q           R|

### Parameters

<table>  <tr>    <td><a name='SkMatrix_postSkew_kx'><code><strong>kx</strong></code></a></td>
    <td>horizontal skew factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_postSkew_ky'><code><strong>ky</strong></code></a></td>
    <td>vertical skew factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_postSkew_px'><code><strong>px</strong></code></a></td>
    <td>pivot on x-axis</td>
  </tr>
  <tr>    <td><a name='SkMatrix_postSkew_py'><code><strong>py</strong></code></a></td>
    <td>pivot on y-axis</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="8c34ae3a2b7e2742bb969819737365ec"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_preSkew'>preSkew</a> <a href='#SkMatrix_setSkew'>setSkew</a>

<a name='SkMatrix_postSkew_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_postSkew'>postSkew</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>kx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>ky</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>constructed</a> <a href='SkMatrix_Reference#SkMatrix'>from</a> <a href='SkMatrix_Reference#SkMatrix'>skewing</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> (<a href='#SkMatrix_postSkew_2_kx'>kx</a>, <a href='#SkMatrix_postSkew_2_ky'>ky</a>) <a href='#SkMatrix_postSkew_2_ky'>about</a> <a href='#SkMatrix_postSkew_2_ky'>pivot</a> <a href='SkPoint_Reference#Point'>point</a>
(0, 0), multiplied by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
This can be thought of as skewing about the origin after applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Given:

| J K L |               |  1 <a href='#SkMatrix_postSkew_2_kx'>kx</a> 0 |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>M</a> <a href='SkMatrix_Reference#Matrix'>N</a> <a href='SkMatrix_Reference#Matrix'>O</a> |,  <a href='SkMatrix_Reference#Matrix'>K</a>(<a href='#SkMatrix_postSkew_2_kx'>kx</a>, <a href='#SkMatrix_postSkew_2_ky'>ky</a>) = | <a href='#SkMatrix_postSkew_2_ky'>ky</a>  1 0 |
| P Q R |               |  0  0 1 |

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a>:

|  1 <a href='#SkMatrix_postSkew_2_kx'>kx</a> 0 | | <a href='#SkMatrix_postSkew_2_kx'>J</a> <a href='#SkMatrix_postSkew_2_kx'>K</a> <a href='#SkMatrix_postSkew_2_kx'>L</a> |   | <a href='#SkMatrix_postSkew_2_kx'>J</a>+<a href='#SkMatrix_postSkew_2_kx'>kx</a>*<a href='#SkMatrix_postSkew_2_kx'>M</a> <a href='#SkMatrix_postSkew_2_kx'>K</a>+<a href='#SkMatrix_postSkew_2_kx'>kx</a>*<a href='#SkMatrix_postSkew_2_kx'>N</a> <a href='#SkMatrix_postSkew_2_kx'>L</a>+<a href='#SkMatrix_postSkew_2_kx'>kx</a>*<a href='#SkMatrix_postSkew_2_kx'>O</a> |
K(<a href='#SkMatrix_postSkew_2_kx'>kx</a>, <a href='#SkMatrix_postSkew_2_ky'>ky</a>) * <a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='#SkMatrix_postSkew_2_ky'>ky</a>  1 0 | | <a href='#SkMatrix_postSkew_2_ky'>M</a> <a href='#SkMatrix_postSkew_2_ky'>N</a> <a href='#SkMatrix_postSkew_2_ky'>O</a> | = | <a href='#SkMatrix_postSkew_2_ky'>ky</a>*<a href='#SkMatrix_postSkew_2_ky'>J</a>+<a href='#SkMatrix_postSkew_2_ky'>M</a> <a href='#SkMatrix_postSkew_2_ky'>ky</a>*<a href='#SkMatrix_postSkew_2_ky'>K</a>+<a href='#SkMatrix_postSkew_2_ky'>N</a> <a href='#SkMatrix_postSkew_2_ky'>ky</a>*<a href='#SkMatrix_postSkew_2_ky'>L</a>+<a href='#SkMatrix_postSkew_2_ky'>O</a> |
|  0  0 1 | | P Q R |   |      P      Q      R |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_postSkew_2_kx'><code><strong>kx</strong></code></a></td>
    <td>horizontal skew factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_postSkew_2_ky'><code><strong>ky</strong></code></a></td>
    <td>vertical skew factor</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3aa2603225dff72ac53dd359f897f494"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_preSkew'>preSkew</a> <a href='#SkMatrix_setSkew'>setSkew</a>

<a name='SkMatrix_postConcat'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_postConcat'>postConcat</a>(<a href='#SkMatrix_postConcat'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#SkMatrix'>other</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_postConcat_other'>other</a> <a href='#SkMatrix_postConcat_other'>multiplied</a> <a href='#SkMatrix_postConcat_other'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
This can be thought of mapping by <a href='#SkMatrix_postConcat_other'>other</a> <a href='#SkMatrix_postConcat_other'>after</a> <a href='#SkMatrix_postConcat_other'>applying</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Given:

| J K L |           | A B C |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>M</a> <a href='SkMatrix_Reference#Matrix'>N</a> <a href='SkMatrix_Reference#Matrix'>O</a> |,  <a href='#SkMatrix_postConcat_other'>other</a> = | <a href='#SkMatrix_postConcat_other'>D</a> <a href='#SkMatrix_postConcat_other'>E</a> <a href='#SkMatrix_postConcat_other'>F</a> |
| P Q R |           | G H <a href='#SkMatrix_I'>I</a> |

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a>:

| A B C |   | J K L |   | AJ+BM+CP AK+BN+CQ AL+BO+CR |
<a href='#SkMatrix_postConcat_other'>other</a> * <a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> <a href='SkMatrix_Reference#Matrix'>F</a> | * | <a href='SkMatrix_Reference#Matrix'>M</a> <a href='SkMatrix_Reference#Matrix'>N</a> <a href='SkMatrix_Reference#Matrix'>O</a> | = | <a href='SkMatrix_Reference#Matrix'>DJ</a>+<a href='SkMatrix_Reference#Matrix'>EM</a>+<a href='SkMatrix_Reference#Matrix'>FP</a> <a href='SkMatrix_Reference#Matrix'>DK</a>+<a href='SkMatrix_Reference#Matrix'>EN</a>+<a href='SkMatrix_Reference#Matrix'>FQ</a> <a href='SkMatrix_Reference#Matrix'>DL</a>+<a href='SkMatrix_Reference#Matrix'>EO</a>+<a href='SkMatrix_Reference#Matrix'>FR</a> |
| G H <a href='#SkMatrix_I'>I</a> |   | <a href='#SkMatrix_I'>P</a> <a href='#SkMatrix_I'>Q</a> <a href='#SkMatrix_I'>R</a> |   | <a href='#SkMatrix_I'>GJ</a>+<a href='#SkMatrix_I'>HM</a>+<a href='#SkMatrix_I'>IP</a> <a href='#SkMatrix_I'>GK</a>+<a href='#SkMatrix_I'>HN</a>+<a href='#SkMatrix_I'>IQ</a> <a href='#SkMatrix_I'>GL</a>+<a href='#SkMatrix_I'>HO</a>+<a href='#SkMatrix_I'>IR</a> |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_postConcat_other'><code><strong>other</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>on</a>  <a href='SkMatrix_Reference#SkMatrix'>left side</a> <a href='SkMatrix_Reference#SkMatrix'>of</a> <a href='SkMatrix_Reference#SkMatrix'>multiply</a> <a href='SkMatrix_Reference#SkMatrix'>expression</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="e4226c55d9bdbc119264bd372b2b9835"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_preConcat'>preConcat</a> <a href='#SkMatrix_setConcat'>setConcat</a> <a href='#SkMatrix_Concat'>Concat</a>

<a name='SkMatrix_ScaleToFit'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkMatrix_ScaleToFit'>ScaleToFit</a> {
        <a href='#SkMatrix_kFill_ScaleToFit'>kFill_ScaleToFit</a>,
        <a href='#SkMatrix_kStart_ScaleToFit'>kStart_ScaleToFit</a>,
        <a href='#SkMatrix_kCenter_ScaleToFit'>kCenter_ScaleToFit</a>,
        <a href='#SkMatrix_kEnd_ScaleToFit'>kEnd_ScaleToFit</a>,
    };
</pre>

<a href='#SkMatrix_ScaleToFit'>ScaleToFit</a> <a href='#SkMatrix_ScaleToFit'>describes</a> <a href='#SkMatrix_ScaleToFit'>how</a> <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>is</a> <a href='SkMatrix_Reference#Matrix'>constructed</a> <a href='SkMatrix_Reference#Matrix'>to</a> <a href='SkMatrix_Reference#Matrix'>map</a> <a href='SkMatrix_Reference#Matrix'>one</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>to</a> <a href='SkRect_Reference#Rect'>another</a>.
<a href='#SkMatrix_ScaleToFit'>ScaleToFit</a> <a href='#SkMatrix_ScaleToFit'>may</a> <a href='#SkMatrix_ScaleToFit'>allow</a> <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>to</a> <a href='SkMatrix_Reference#Matrix'>have</a> <a href='SkMatrix_Reference#Matrix'>unequal</a> <a href='SkMatrix_Reference#Matrix'>horizontal</a> <a href='SkMatrix_Reference#Matrix'>and</a> <a href='SkMatrix_Reference#Matrix'>vertical</a> <a href='SkMatrix_Reference#Matrix'>scaling</a>,
<a href='SkMatrix_Reference#Matrix'>or</a> <a href='SkMatrix_Reference#Matrix'>may</a> <a href='SkMatrix_Reference#Matrix'>restrict</a> <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>to</a> <a href='SkMatrix_Reference#Matrix'>square</a> <a href='SkMatrix_Reference#Matrix'>scaling</a>. <a href='SkMatrix_Reference#Matrix'>If</a> <a href='SkMatrix_Reference#Matrix'>restricted</a>, <a href='#SkMatrix_ScaleToFit'>ScaleToFit</a> <a href='#SkMatrix_ScaleToFit'>specifies</a>
<a href='#SkMatrix_ScaleToFit'>how</a> <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>maps</a> <a href='SkMatrix_Reference#Matrix'>to</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>side</a> <a href='SkMatrix_Reference#Matrix'>or</a> <a href='SkMatrix_Reference#Matrix'>center</a> <a href='SkMatrix_Reference#Matrix'>of</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>destination</a> <a href='SkRect_Reference#Rect'>Rect</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kFill_ScaleToFit'><code>SkMatrix::kFill_ScaleToFit</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Computes <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>that</a> <a href='SkMatrix_Reference#Matrix'>scales</a> <a href='SkMatrix_Reference#Matrix'>about</a> <a href='SkMatrix_Reference#Matrix'>x-axis</a> <a href='SkMatrix_Reference#Matrix'>and</a> <a href='SkMatrix_Reference#Matrix'>y-axis</a> <a href='SkMatrix_Reference#Matrix'>independently</a>, <a href='SkMatrix_Reference#Matrix'>so</a> <a href='SkMatrix_Reference#Matrix'>that</a>
<a href='SkMatrix_Reference#Matrix'>source</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>is</a> <a href='SkRect_Reference#Rect'>mapped</a> <a href='SkRect_Reference#Rect'>to</a> <a href='SkRect_Reference#Rect'>completely</a> <a href='SkRect_Reference#Rect'>fill</a> <a href='SkRect_Reference#Rect'>destination</a> <a href='SkRect_Reference#Rect'>Rect</a>. <a href='SkRect_Reference#Rect'>The</a> <a href='SkRect_Reference#Rect'>aspect</a> <a href='SkRect_Reference#Rect'>ratio</a>
<a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>source</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>may</a> <a href='SkRect_Reference#Rect'>change</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kStart_ScaleToFit'><code>SkMatrix::kStart_ScaleToFit</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Computes <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>that</a> <a href='SkMatrix_Reference#Matrix'>maintains</a> <a href='SkMatrix_Reference#Matrix'>source</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>aspect</a> <a href='SkRect_Reference#Rect'>ratio</a>, <a href='SkRect_Reference#Rect'>mapping</a> <a href='SkRect_Reference#Rect'>source</a> <a href='SkRect_Reference#Rect'>Rect</a>
<a href='SkRect_Reference#Rect'>width</a> <a href='SkRect_Reference#Rect'>or</a> <a href='SkRect_Reference#Rect'>height</a> <a href='SkRect_Reference#Rect'>to</a> <a href='SkRect_Reference#Rect'>destination</a> <a href='SkRect_Reference#Rect'>Rect</a>. <a href='SkRect_Reference#Rect'>Aligns</a> <a href='SkRect_Reference#Rect'>mapping</a> <a href='SkRect_Reference#Rect'>to</a> <a href='SkRect_Reference#Rect'>left</a> <a href='SkRect_Reference#Rect'>and</a> <a href='SkRect_Reference#Rect'>top</a> <a href='SkRect_Reference#Rect'>edges</a>
<a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>destination</a> <a href='SkRect_Reference#Rect'>Rect</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kCenter_ScaleToFit'><code>SkMatrix::kCenter_ScaleToFit</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Computes <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>that</a> <a href='SkMatrix_Reference#Matrix'>maintains</a> <a href='SkMatrix_Reference#Matrix'>source</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>aspect</a> <a href='SkRect_Reference#Rect'>ratio</a>, <a href='SkRect_Reference#Rect'>mapping</a> <a href='SkRect_Reference#Rect'>source</a> <a href='SkRect_Reference#Rect'>Rect</a>
<a href='SkRect_Reference#Rect'>width</a> <a href='SkRect_Reference#Rect'>or</a> <a href='SkRect_Reference#Rect'>height</a> <a href='SkRect_Reference#Rect'>to</a> <a href='SkRect_Reference#Rect'>destination</a> <a href='SkRect_Reference#Rect'>Rect</a>. <a href='SkRect_Reference#Rect'>Aligns</a> <a href='SkRect_Reference#Rect'>mapping</a> <a href='SkRect_Reference#Rect'>to</a> <a href='SkRect_Reference#Rect'>center</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>destination</a>
<a href='SkRect_Reference#Rect'>Rect</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kEnd_ScaleToFit'><code>SkMatrix::kEnd_ScaleToFit</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Computes <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>that</a> <a href='SkMatrix_Reference#Matrix'>maintains</a> <a href='SkMatrix_Reference#Matrix'>source</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='SkRect_Reference#Rect'>aspect</a> <a href='SkRect_Reference#Rect'>ratio</a>, <a href='SkRect_Reference#Rect'>mapping</a> <a href='SkRect_Reference#Rect'>source</a> <a href='SkRect_Reference#Rect'>Rect</a>
<a href='SkRect_Reference#Rect'>width</a> <a href='SkRect_Reference#Rect'>or</a> <a href='SkRect_Reference#Rect'>height</a> <a href='SkRect_Reference#Rect'>to</a> <a href='SkRect_Reference#Rect'>destination</a> <a href='SkRect_Reference#Rect'>Rect</a>. <a href='SkRect_Reference#Rect'>Aligns</a> <a href='SkRect_Reference#Rect'>mapping</a> <a href='SkRect_Reference#Rect'>to</a> <a href='SkRect_Reference#Rect'>right</a> <a href='SkRect_Reference#Rect'>and</a> <a href='SkRect_Reference#Rect'>bottom</a>
<a href='SkRect_Reference#Rect'>edges</a> <a href='SkRect_Reference#Rect'>of</a> <a href='SkRect_Reference#Rect'>destination</a> <a href='SkRect_Reference#Rect'>Rect</a>.
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="17c3070b31b700ea8f52e48af9a66b6e"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setRectToRect'>setRectToRect</a> <a href='#SkMatrix_MakeRectToRect'>MakeRectToRect</a> <a href='#SkMatrix_setPolyToPoly'>setPolyToPoly</a>

<a name='SkMatrix_setRectToRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_setRectToRect'>setRectToRect</a>(<a href='#SkMatrix_setRectToRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>src</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>, <a href='#SkMatrix_ScaleToFit'>ScaleToFit</a> <a href='#SkMatrix_ScaleToFit'>stf</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>scale</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>translate</a> <a href='#SkMatrix_setRectToRect_src'>src</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='#SkMatrix_setRectToRect_dst'>dst</a> <a href='SkRect_Reference#SkRect'>SkRect</a>. <a href='#SkMatrix_setRectToRect_stf'>stf</a> <a href='#SkMatrix_setRectToRect_stf'>selects</a> <a href='#SkMatrix_setRectToRect_stf'>whether</a>
mapping completely fills <a href='#SkMatrix_setRectToRect_dst'>dst</a> <a href='#SkMatrix_setRectToRect_dst'>or</a> <a href='#SkMatrix_setRectToRect_dst'>preserves</a> <a href='#SkMatrix_setRectToRect_dst'>the</a> <a href='#SkMatrix_setRectToRect_dst'>aspect</a> <a href='#SkMatrix_setRectToRect_dst'>ratio</a>, <a href='#SkMatrix_setRectToRect_dst'>and</a> <a href='#SkMatrix_setRectToRect_dst'>how</a> <a href='#SkMatrix_setRectToRect_dst'>to</a> <a href='#SkMatrix_setRectToRect_dst'>align</a>
<a href='#SkMatrix_setRectToRect_src'>src</a> <a href='#SkMatrix_setRectToRect_src'>within</a> <a href='#SkMatrix_setRectToRect_dst'>dst</a>. <a href='#SkMatrix_setRectToRect_dst'>Returns</a> <a href='#SkMatrix_setRectToRect_dst'>false</a> <a href='#SkMatrix_setRectToRect_dst'>if</a> <a href='#SkMatrix_setRectToRect_src'>src</a> <a href='#SkMatrix_setRectToRect_src'>is</a> <a href='#SkMatrix_setRectToRect_src'>empty</a>, <a href='#SkMatrix_setRectToRect_src'>and</a> <a href='#SkMatrix_setRectToRect_src'>sets</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>identity</a>.
Returns true if <a href='#SkMatrix_setRectToRect_dst'>dst</a> <a href='#SkMatrix_setRectToRect_dst'>is</a> <a href='#SkMatrix_setRectToRect_dst'>empty</a>, <a href='#SkMatrix_setRectToRect_dst'>and</a> <a href='#SkMatrix_setRectToRect_dst'>sets</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a>:

| 0 0 0 |
| 0 0 0 |
| 0 0 1 |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setRectToRect_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>map</a> <a href='SkRect_Reference#SkRect'>from</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_setRectToRect_dst'><code><strong>dst</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>map</a> <a href='SkRect_Reference#SkRect'>to</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_setRectToRect_stf'><code><strong>stf</strong></code></a></td>
    <td>one of: <a href='#SkMatrix_kFill_ScaleToFit'>kFill_ScaleToFit</a>, <a href='#SkMatrix_kStart_ScaleToFit'>kStart_ScaleToFit</a>,</td>
  </tr>
</table>

<a href='#SkMatrix_kCenter_ScaleToFit'>kCenter_ScaleToFit</a>, <a href='#SkMatrix_kEnd_ScaleToFit'>kEnd_ScaleToFit</a>

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>can</a> <a href='SkMatrix_Reference#SkMatrix'>represent</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>mapping</a>

### Example

<div><fiddle-embed name="69cdea599dcaaec35efcb24403f4287b">

#### Example Output

~~~~
src: 0, 0, 0, 0  dst: 0, 0, 0, 0  success: false
[  1.0000   0.0000   0.0000][  0.0000   1.0000   0.0000][  0.0000   0.0000   1.0000]
src: 0, 0, 0, 0  dst: 5, 6, 8, 9  success: false
[  1.0000   0.0000   0.0000][  0.0000   1.0000   0.0000][  0.0000   0.0000   1.0000]
src: 1, 2, 3, 4  dst: 0, 0, 0, 0  success: true
[  0.0000   0.0000   0.0000][  0.0000   0.0000   0.0000][  0.0000   0.0000   1.0000]
src: 1, 2, 3, 4  dst: 5, 6, 8, 9  success: true
[  1.5000   0.0000   3.5000][  0.0000   1.5000   3.0000][  0.0000   0.0000   1.0000]
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_MakeRectToRect'>MakeRectToRect</a> <a href='#SkMatrix_ScaleToFit'>ScaleToFit</a> <a href='#SkMatrix_setPolyToPoly'>setPolyToPoly</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_isEmpty'>isEmpty</a>

<a name='SkMatrix_MakeRectToRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_MakeRectToRect'>MakeRectToRect</a>(<a href='#SkMatrix_MakeRectToRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>src</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>dst</a>, <a href='#SkMatrix_ScaleToFit'>ScaleToFit</a> <a href='#SkMatrix_ScaleToFit'>stf</a>)
</pre>

Returns <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>set</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>scale</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>translate</a> <a href='#SkMatrix_MakeRectToRect_src'>src</a> <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='#SkMatrix_MakeRectToRect_dst'>dst</a> <a href='SkRect_Reference#SkRect'>SkRect</a>. <a href='#SkMatrix_MakeRectToRect_stf'>stf</a> <a href='#SkMatrix_MakeRectToRect_stf'>selects</a>
whether mapping completely fills <a href='#SkMatrix_MakeRectToRect_dst'>dst</a> <a href='#SkMatrix_MakeRectToRect_dst'>or</a> <a href='#SkMatrix_MakeRectToRect_dst'>preserves</a> <a href='#SkMatrix_MakeRectToRect_dst'>the</a> <a href='#SkMatrix_MakeRectToRect_dst'>aspect</a> <a href='#SkMatrix_MakeRectToRect_dst'>ratio</a>, <a href='#SkMatrix_MakeRectToRect_dst'>and</a> <a href='#SkMatrix_MakeRectToRect_dst'>how</a> <a href='#SkMatrix_MakeRectToRect_dst'>to</a>
align <a href='#SkMatrix_MakeRectToRect_src'>src</a> <a href='#SkMatrix_MakeRectToRect_src'>within</a> <a href='#SkMatrix_MakeRectToRect_dst'>dst</a>. <a href='#SkMatrix_MakeRectToRect_dst'>Returns</a> <a href='#SkMatrix_MakeRectToRect_dst'>the</a> <a href='#SkMatrix_MakeRectToRect_dst'>identity</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>if</a> <a href='#SkMatrix_MakeRectToRect_src'>src</a> <a href='#SkMatrix_MakeRectToRect_src'>is</a> <a href='#SkMatrix_MakeRectToRect_src'>empty</a>. <a href='#SkMatrix_MakeRectToRect_src'>If</a> <a href='#SkMatrix_MakeRectToRect_dst'>dst</a> <a href='#SkMatrix_MakeRectToRect_dst'>is</a>
empty, returns <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>set</a> <a href='SkMatrix_Reference#SkMatrix'>to</a>:

| 0 0 0 |
| 0 0 0 |
| 0 0 1 |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_MakeRectToRect_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>map</a> <a href='SkRect_Reference#SkRect'>from</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_MakeRectToRect_dst'><code><strong>dst</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>map</a> <a href='SkRect_Reference#SkRect'>to</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_MakeRectToRect_stf'><code><strong>stf</strong></code></a></td>
    <td>one of: <a href='#SkMatrix_kFill_ScaleToFit'>kFill_ScaleToFit</a>, <a href='#SkMatrix_kStart_ScaleToFit'>kStart_ScaleToFit</a>,</td>
  </tr>
</table>

<a href='#SkMatrix_kCenter_ScaleToFit'>kCenter_ScaleToFit</a>, <a href='#SkMatrix_kEnd_ScaleToFit'>kEnd_ScaleToFit</a>

### Return Value

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>mapping</a> <a href='#SkMatrix_MakeRectToRect_src'>src</a> <a href='#SkMatrix_MakeRectToRect_src'>to</a> <a href='#SkMatrix_MakeRectToRect_dst'>dst</a>

### Example

<div><fiddle-embed name="a1d6a6721b39350f81021f71a1b93208">

#### Example Output

~~~~
src: 0, 0, 0, 0  dst: 0, 0, 0, 0
[  1.0000   0.0000   0.0000][  0.0000   1.0000   0.0000][  0.0000   0.0000   1.0000]
src: 0, 0, 0, 0  dst: 5, 6, 8, 9
[  1.0000   0.0000   0.0000][  0.0000   1.0000   0.0000][  0.0000   0.0000   1.0000]
src: 1, 2, 3, 4  dst: 0, 0, 0, 0
[  0.0000   0.0000   0.0000][  0.0000   0.0000   0.0000][  0.0000   0.0000   1.0000]
src: 1, 2, 3, 4  dst: 5, 6, 8, 9
[  1.5000   0.0000   3.5000][  0.0000   1.5000   3.0000][  0.0000   0.0000   1.0000]
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_setRectToRect'>setRectToRect</a> <a href='#SkMatrix_ScaleToFit'>ScaleToFit</a> <a href='#SkMatrix_setPolyToPoly'>setPolyToPoly</a> <a href='SkRect_Reference#SkRect'>SkRect</a>::<a href='#SkRect_isEmpty'>isEmpty</a>

<a name='SkMatrix_setPolyToPoly'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_setPolyToPoly'>setPolyToPoly</a>(<a href='#SkMatrix_setPolyToPoly'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>src</a>[], <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>dst</a>[], <a href='SkPoint_Reference#SkPoint'>int</a> <a href='SkPoint_Reference#SkPoint'>count</a>)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>map</a> <a href='#SkMatrix_setPolyToPoly_src'>src</a> <a href='#SkMatrix_setPolyToPoly_src'>to</a> <a href='#SkMatrix_setPolyToPoly_dst'>dst</a>. <a href='#SkMatrix_setPolyToPoly_count'>count</a> <a href='#SkMatrix_setPolyToPoly_count'>must</a> <a href='#SkMatrix_setPolyToPoly_count'>be</a> <a href='#SkMatrix_setPolyToPoly_count'>zero</a> <a href='#SkMatrix_setPolyToPoly_count'>or</a> <a href='#SkMatrix_setPolyToPoly_count'>greater</a>, <a href='#SkMatrix_setPolyToPoly_count'>and</a> <a href='#SkMatrix_setPolyToPoly_count'>four</a> <a href='#SkMatrix_setPolyToPoly_count'>or</a> <a href='#SkMatrix_setPolyToPoly_count'>less</a>.

If <a href='#SkMatrix_setPolyToPoly_count'>count</a> <a href='#SkMatrix_setPolyToPoly_count'>is</a> <a href='#SkMatrix_setPolyToPoly_count'>zero</a>, <a href='#SkMatrix_setPolyToPoly_count'>sets</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>identity</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>returns</a> <a href='SkMatrix_Reference#SkMatrix'>true</a>.
If <a href='#SkMatrix_setPolyToPoly_count'>count</a> <a href='#SkMatrix_setPolyToPoly_count'>is</a> <a href='#SkMatrix_setPolyToPoly_count'>one</a>, <a href='#SkMatrix_setPolyToPoly_count'>sets</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>translate</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>returns</a> <a href='SkMatrix_Reference#SkMatrix'>true</a>.
If <a href='#SkMatrix_setPolyToPoly_count'>count</a> <a href='#SkMatrix_setPolyToPoly_count'>is</a> <a href='#SkMatrix_setPolyToPoly_count'>two</a> <a href='#SkMatrix_setPolyToPoly_count'>or</a> <a href='#SkMatrix_setPolyToPoly_count'>more</a>, <a href='#SkMatrix_setPolyToPoly_count'>sets</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>map</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>if</a> <a href='SkPoint_Reference#SkPoint'>possible</a>; <a href='SkPoint_Reference#SkPoint'>returns</a> <a href='SkPoint_Reference#SkPoint'>false</a>
if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>cannot</a> <a href='SkMatrix_Reference#SkMatrix'>be</a> <a href='SkMatrix_Reference#SkMatrix'>constructed</a>. <a href='SkMatrix_Reference#SkMatrix'>If</a> <a href='#SkMatrix_setPolyToPoly_count'>count</a> <a href='#SkMatrix_setPolyToPoly_count'>is</a> <a href='#SkMatrix_setPolyToPoly_count'>four</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>may</a> <a href='SkMatrix_Reference#SkMatrix'>include</a>
perspective.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setPolyToPoly_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>to</a> <a href='SkPoint_Reference#SkPoint'>map</a> <a href='SkPoint_Reference#SkPoint'>from</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_setPolyToPoly_dst'><code><strong>dst</strong></code></a></td>
    <td><a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>to</a> <a href='SkPoint_Reference#SkPoint'>map</a> <a href='SkPoint_Reference#SkPoint'>to</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_setPolyToPoly_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>in</a> <a href='#SkMatrix_setPolyToPoly_src'>src</a> <a href='#SkMatrix_setPolyToPoly_src'>and</a> <a href='#SkMatrix_setPolyToPoly_dst'>dst</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>was</a> <a href='SkMatrix_Reference#SkMatrix'>constructed</a> <a href='SkMatrix_Reference#SkMatrix'>successfully</a>

### Example

<div><fiddle-embed name="c851d1313e8909aaea4f0591699fdb7b"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setRectToRect'>setRectToRect</a> <a href='#SkMatrix_MakeRectToRect'>MakeRectToRect</a>

<a name='SkMatrix_invert'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_invert'>invert</a>(<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#SkMatrix'>inverse</a>) <a href='SkMatrix_Reference#SkMatrix'>const</a>
</pre>

Sets <a href='#SkMatrix_invert_inverse'>inverse</a> <a href='#SkMatrix_invert_inverse'>to</a> <a href='#SkMatrix_invert_inverse'>reciprocal</a> <a href='SkMatrix_Reference#Matrix'>matrix</a>, <a href='SkMatrix_Reference#Matrix'>returning</a> <a href='SkMatrix_Reference#Matrix'>true</a> <a href='SkMatrix_Reference#Matrix'>if</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>can</a> <a href='SkMatrix_Reference#SkMatrix'>be</a> <a href='SkMatrix_Reference#SkMatrix'>inverted</a>.
Geometrically, if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>maps</a> <a href='SkMatrix_Reference#SkMatrix'>from</a> <a href='SkMatrix_Reference#SkMatrix'>source</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>destination</a>, <a href='#SkMatrix_invert_inverse'>inverse</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
maps from destination to source. If <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>can</a> <a href='SkMatrix_Reference#SkMatrix'>not</a> <a href='SkMatrix_Reference#SkMatrix'>be</a> <a href='SkMatrix_Reference#SkMatrix'>inverted</a>, <a href='#SkMatrix_invert_inverse'>inverse</a> <a href='#SkMatrix_invert_inverse'>is</a>
unchanged.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_invert_inverse'><code><strong>inverse</strong></code></a></td>
    <td>storage for inverted <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>; <a href='SkMatrix_Reference#SkMatrix'>may</a> <a href='SkMatrix_Reference#SkMatrix'>be</a> <a href='SkMatrix_Reference#SkMatrix'>nullptr</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>can</a> <a href='SkMatrix_Reference#SkMatrix'>be</a> <a href='SkMatrix_Reference#SkMatrix'>inverted</a>

### Example

<div><fiddle-embed name="0e03cd9f154603ed4b21ca56d69dae44"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_Concat'>Concat</a>

<a name='SkMatrix_SetAffineIdentity'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static void <a href='#SkMatrix_SetAffineIdentity'>SetAffineIdentity</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>affine</a>[6])
</pre>

Fills <a href='#SkMatrix_SetAffineIdentity_affine'>affine</a> <a href='#SkMatrix_SetAffineIdentity_affine'>with</a> <a href='#SkMatrix_SetAffineIdentity_affine'>identity</a> <a href='#SkMatrix_SetAffineIdentity_affine'>values</a> <a href='#SkMatrix_SetAffineIdentity_affine'>in</a> <a href='#SkMatrix_SetAffineIdentity_affine'>column</a> <a href='#SkMatrix_SetAffineIdentity_affine'>major</a> <a href='#SkMatrix_SetAffineIdentity_affine'>order</a>.
Sets <a href='#SkMatrix_SetAffineIdentity_affine'>affine</a> <a href='#SkMatrix_SetAffineIdentity_affine'>to</a>:

| 1 0 0 |
| 0 1 0 |

Affine 3 by 2 <a href='SkMatrix_Reference#Matrix'>matrices</a> <a href='SkMatrix_Reference#Matrix'>in</a> <a href='SkMatrix_Reference#Matrix'>column</a> <a href='SkMatrix_Reference#Matrix'>major</a> <a href='SkMatrix_Reference#Matrix'>order</a> <a href='SkMatrix_Reference#Matrix'>are</a> <a href='SkMatrix_Reference#Matrix'>used</a> <a href='SkMatrix_Reference#Matrix'>by</a> <a href='SkMatrix_Reference#Matrix'>OpenGL</a> <a href='SkMatrix_Reference#Matrix'>and</a> <a href='SkMatrix_Reference#Matrix'>XPS</a>.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_SetAffineIdentity_affine'><code><strong>affine</strong></code></a></td>
    <td>storage for 3 by 2 <a href='#SkMatrix_SetAffineIdentity_affine'>affine</a> <a href='SkMatrix_Reference#Matrix'>matrix</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="e10adbd0bcc940c5d4d872db0e78e892">

#### Example Output

~~~~
ScaleX: 1 SkewY: 0 SkewX: 0 ScaleY: 1 TransX: 0 TransY: 0
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_setAffine'>setAffine</a> <a href='#SkMatrix_asAffine'>asAffine</a>

<a name='SkMatrix_asAffine'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_asAffine'>asAffine</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>affine</a>[6]) <a href='undocumented#SkScalar'>const</a>
</pre>

Fills <a href='#SkMatrix_asAffine_affine'>affine</a> <a href='#SkMatrix_asAffine_affine'>in</a> <a href='#SkMatrix_asAffine_affine'>column</a> <a href='#SkMatrix_asAffine_affine'>major</a> <a href='#SkMatrix_asAffine_affine'>order</a>. <a href='#SkMatrix_asAffine_affine'>Sets</a> <a href='#SkMatrix_asAffine_affine'>affine</a> <a href='#SkMatrix_asAffine_affine'>to</a>:

| scale-x  skew-x translate-x |
| skew-y  scale-y translate-y |

If <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>contains</a> <a href='SkMatrix_Reference#SkMatrix'>perspective</a>, <a href='SkMatrix_Reference#SkMatrix'>returns</a> <a href='SkMatrix_Reference#SkMatrix'>false</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>leaves</a> <a href='#SkMatrix_asAffine_affine'>affine</a> <a href='#SkMatrix_asAffine_affine'>unchanged</a>.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_asAffine_affine'><code><strong>affine</strong></code></a></td>
    <td>storage for 3 by 2 <a href='#SkMatrix_asAffine_affine'>affine</a> <a href='SkMatrix_Reference#Matrix'>matrix</a>; <a href='SkMatrix_Reference#Matrix'>may</a> <a href='SkMatrix_Reference#Matrix'>be</a> <a href='SkMatrix_Reference#Matrix'>nullptr</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>does</a> <a href='SkMatrix_Reference#SkMatrix'>not</a> <a href='SkMatrix_Reference#SkMatrix'>contain</a> <a href='SkMatrix_Reference#SkMatrix'>perspective</a>

### Example

<div><fiddle-embed name="3325bdf82bd86d9fbc4199f248afa82c">

#### Example Output

~~~~
ScaleX: 2 SkewY: 5 SkewX: 3 ScaleY: 6 TransX: 4 TransY: 7
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_setAffine'>setAffine</a> <a href='#SkMatrix_SetAffineIdentity'>SetAffineIdentity</a>

<a name='SkMatrix_setAffine'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setAffine'>setAffine</a>(<a href='#SkMatrix_setAffine'>const</a> <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>affine</a>[6])
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='#SkMatrix_setAffine_affine'>affine</a> <a href='#SkMatrix_setAffine_affine'>values</a>, <a href='#SkMatrix_setAffine_affine'>passed</a> <a href='#SkMatrix_setAffine_affine'>in</a> <a href='#SkMatrix_setAffine_affine'>column</a> <a href='#SkMatrix_setAffine_affine'>major</a> <a href='#SkMatrix_setAffine_affine'>order</a>. <a href='#SkMatrix_setAffine_affine'>Given</a> <a href='#SkMatrix_setAffine_affine'>affine</a>,
column, then row, as:

| scale-x  skew-x translate-x |
|  skew-y scale-y translate-y |

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>is</a> <a href='SkMatrix_Reference#SkMatrix'>set</a>, <a href='SkMatrix_Reference#SkMatrix'>row</a>, <a href='SkMatrix_Reference#SkMatrix'>then</a> <a href='SkMatrix_Reference#SkMatrix'>column</a>, <a href='SkMatrix_Reference#SkMatrix'>to</a>:

| scale-x  skew-x translate-x |
|  skew-y scale-y translate-y |
|       0       0           1 |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setAffine_affine'><code><strong>affine</strong></code></a></td>
    <td>3 by 2 <a href='#SkMatrix_setAffine_affine'>affine</a> <a href='SkMatrix_Reference#Matrix'>matrix</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f5b6d371c4d65e5b5ac6eebdd4b237d8">

#### Example Output

~~~~
ScaleX: 2 SkewY: 5 SkewX: 3 ScaleY: 6 TransX: 4 TransY: 7
[  2.0000   3.0000   4.0000][  5.0000   6.0000   7.0000][  0.0000   0.0000   1.0000]
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_asAffine'>asAffine</a> <a href='#SkMatrix_SetAffineIdentity'>SetAffineIdentity</a>

<a name='Transform'></a>

<a name='SkMatrix_mapPoints'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_mapPoints'>mapPoints</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>dst</a>[], <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>src</a>[], <a href='SkPoint_Reference#SkPoint'>int</a> <a href='SkPoint_Reference#SkPoint'>count</a>) <a href='SkPoint_Reference#SkPoint'>const</a>
</pre>

Maps <a href='#SkMatrix_mapPoints_src'>src</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPoint_Reference#SkPoint'>length</a> <a href='#SkMatrix_mapPoints_count'>count</a> <a href='#SkMatrix_mapPoints_count'>to</a> <a href='#SkMatrix_mapPoints_dst'>dst</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPoint_Reference#SkPoint'>equal</a> <a href='SkPoint_Reference#SkPoint'>or</a> <a href='SkPoint_Reference#SkPoint'>greater</a>
length. <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>are</a> <a href='SkPoint_Reference#SkPoint'>mapped</a> <a href='SkPoint_Reference#SkPoint'>by</a> <a href='SkPoint_Reference#SkPoint'>multiplying</a> <a href='SkPoint_Reference#SkPoint'>each</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. <a href='SkMatrix_Reference#SkMatrix'>Given</a>:

| A B C |        | x |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> <a href='SkMatrix_Reference#Matrix'>F</a> |,  <a href='SkMatrix_Reference#Matrix'>pt</a> = | <a href='SkMatrix_Reference#Matrix'>y</a> |
| G H <a href='#SkMatrix_I'>I</a> |        | 1 |

where

for (i = 0; i < <a href='#SkMatrix_mapPoints_count'>count</a>; ++<a href='#SkMatrix_mapPoints_count'>i</a>) {
x = <a href='#SkMatrix_mapPoints_src'>src</a>[<a href='#SkMatrix_mapPoints_src'>i</a>].<a href='#SkMatrix_mapPoints_src'>fX</a>
y = <a href='#SkMatrix_mapPoints_src'>src</a>[<a href='#SkMatrix_mapPoints_src'>i</a>].<a href='#SkMatrix_mapPoints_src'>fY</a>
}

each <a href='#SkMatrix_mapPoints_dst'>dst</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>is</a> <a href='SkPoint_Reference#SkPoint'>computed</a> <a href='SkPoint_Reference#SkPoint'>as</a>:

|A B C| |x|                               Ax+By+C   Dx+Ey+F
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * <a href='SkMatrix_Reference#Matrix'>pt</a> = |<a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> <a href='SkMatrix_Reference#Matrix'>F</a>| |<a href='SkMatrix_Reference#Matrix'>y</a>| = |<a href='SkMatrix_Reference#Matrix'>Ax</a>+<a href='SkMatrix_Reference#Matrix'>By</a>+<a href='SkMatrix_Reference#Matrix'>C</a> <a href='SkMatrix_Reference#Matrix'>Dx</a>+<a href='SkMatrix_Reference#Matrix'>Ey</a>+<a href='SkMatrix_Reference#Matrix'>F</a> <a href='SkMatrix_Reference#Matrix'>Gx</a>+<a href='SkMatrix_Reference#Matrix'>Hy</a>+<a href='#SkMatrix_I'>I</a>| = ------- , -------
|G H <a href='#SkMatrix_I'>I</a>| |1|                               <a href='#SkMatrix_I'>Gx</a>+<a href='#SkMatrix_I'>Hy</a>+<a href='#SkMatrix_I'>I</a>   <a href='#SkMatrix_I'>Gx</a>+<a href='#SkMatrix_I'>Hy</a>+<a href='#SkMatrix_I'>I</a>

<a href='#SkMatrix_mapPoints_src'>src</a> <a href='#SkMatrix_mapPoints_src'>and</a> <a href='#SkMatrix_mapPoints_dst'>dst</a> <a href='#SkMatrix_mapPoints_dst'>may</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>the</a> <a href='SkPoint_Reference#Point'>same</a> <a href='SkPoint_Reference#Point'>storage</a>.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapPoints_dst'><code><strong>dst</strong></code></a></td>
    <td>storage for mapped <a href='SkPoint_Reference#SkPoint'>SkPoint</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapPoints_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>to</a> <a href='SkPoint_Reference#SkPoint'>transform</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapPoints_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>to</a> <a href='SkPoint_Reference#SkPoint'>transform</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f99dcb00296d0c56b6c0e178e94b3534"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapXY'>mapXY</a> <a href='#SkMatrix_mapHomogeneousPoints'>mapHomogeneousPoints</a> <a href='#SkMatrix_mapVectors'>mapVectors</a>

<a name='SkMatrix_mapPoints_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_mapPoints'>mapPoints</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>pts</a>[], <a href='SkPoint_Reference#SkPoint'>int</a> <a href='SkPoint_Reference#SkPoint'>count</a>) <a href='SkPoint_Reference#SkPoint'>const</a>
</pre>

Maps <a href='#SkMatrix_mapPoints_2_pts'>pts</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPoint_Reference#SkPoint'>length</a> <a href='#SkMatrix_mapPoints_2_count'>count</a> <a href='#SkMatrix_mapPoints_2_count'>in</a> <a href='#SkMatrix_mapPoints_2_count'>place</a>. <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>are</a> <a href='SkPoint_Reference#SkPoint'>mapped</a> <a href='SkPoint_Reference#SkPoint'>by</a> <a href='SkPoint_Reference#SkPoint'>multiplying</a>
each <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. <a href='SkMatrix_Reference#SkMatrix'>Given</a>:

| A B C |        | x |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> <a href='SkMatrix_Reference#Matrix'>F</a> |,  <a href='SkMatrix_Reference#Matrix'>pt</a> = | <a href='SkMatrix_Reference#Matrix'>y</a> |
| G H <a href='#SkMatrix_I'>I</a> |        | 1 |

where

for (i = 0; i < <a href='#SkMatrix_mapPoints_2_count'>count</a>; ++<a href='#SkMatrix_mapPoints_2_count'>i</a>) {
x = <a href='#SkMatrix_mapPoints_2_pts'>pts</a>[<a href='#SkMatrix_mapPoints_2_pts'>i</a>].<a href='#SkMatrix_mapPoints_2_pts'>fX</a>
y = <a href='#SkMatrix_mapPoints_2_pts'>pts</a>[<a href='#SkMatrix_mapPoints_2_pts'>i</a>].<a href='#SkMatrix_mapPoints_2_pts'>fY</a>
}

each resulting <a href='#SkMatrix_mapPoints_2_pts'>pts</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>is</a> <a href='SkPoint_Reference#SkPoint'>computed</a> <a href='SkPoint_Reference#SkPoint'>as</a>:

|A B C| |x|                               Ax+By+C   Dx+Ey+F
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * <a href='SkMatrix_Reference#Matrix'>pt</a> = |<a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> <a href='SkMatrix_Reference#Matrix'>F</a>| |<a href='SkMatrix_Reference#Matrix'>y</a>| = |<a href='SkMatrix_Reference#Matrix'>Ax</a>+<a href='SkMatrix_Reference#Matrix'>By</a>+<a href='SkMatrix_Reference#Matrix'>C</a> <a href='SkMatrix_Reference#Matrix'>Dx</a>+<a href='SkMatrix_Reference#Matrix'>Ey</a>+<a href='SkMatrix_Reference#Matrix'>F</a> <a href='SkMatrix_Reference#Matrix'>Gx</a>+<a href='SkMatrix_Reference#Matrix'>Hy</a>+<a href='#SkMatrix_I'>I</a>| = ------- , -------
|G H <a href='#SkMatrix_I'>I</a>| |1|                               <a href='#SkMatrix_I'>Gx</a>+<a href='#SkMatrix_I'>Hy</a>+<a href='#SkMatrix_I'>I</a>   <a href='#SkMatrix_I'>Gx</a>+<a href='#SkMatrix_I'>Hy</a>+<a href='#SkMatrix_I'>I</a>

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapPoints_2_pts'><code><strong>pts</strong></code></a></td>
    <td>storage for mapped <a href='SkPoint_Reference#SkPoint'>SkPoint</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapPoints_2_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>to</a> <a href='SkPoint_Reference#SkPoint'>transform</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="428ca171ae3bd0d3f992458ac598b97b"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapXY'>mapXY</a> <a href='#SkMatrix_mapHomogeneousPoints'>mapHomogeneousPoints</a> <a href='#SkMatrix_mapVectors'>mapVectors</a>

<a name='SkMatrix_mapHomogeneousPoints'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_mapHomogeneousPoints'>mapHomogeneousPoints</a>(<a href='undocumented#SkPoint3'>SkPoint3</a> <a href='undocumented#SkPoint3'>dst</a>[], <a href='undocumented#SkPoint3'>const</a> <a href='undocumented#SkPoint3'>SkPoint3</a> <a href='undocumented#SkPoint3'>src</a>[], <a href='undocumented#SkPoint3'>int</a> <a href='undocumented#SkPoint3'>count</a>) <a href='undocumented#SkPoint3'>const</a>
</pre>

Maps <a href='#SkMatrix_mapHomogeneousPoints_src'>src</a> <a href='undocumented#SkPoint3'>SkPoint3</a> <a href='undocumented#SkPoint3'>array</a> <a href='undocumented#SkPoint3'>of</a> <a href='undocumented#SkPoint3'>length</a> <a href='#SkMatrix_mapHomogeneousPoints_count'>count</a> <a href='#SkMatrix_mapHomogeneousPoints_count'>to</a> <a href='#SkMatrix_mapHomogeneousPoints_dst'>dst</a> <a href='undocumented#SkPoint3'>SkPoint3</a> <a href='undocumented#SkPoint3'>array</a>, <a href='undocumented#SkPoint3'>which</a> <a href='undocumented#SkPoint3'>must</a> <a href='undocumented#SkPoint3'>of</a> <a href='undocumented#SkPoint3'>length</a> <a href='#SkMatrix_mapHomogeneousPoints_count'>count</a> <a href='#SkMatrix_mapHomogeneousPoints_count'>or</a>
greater. <a href='undocumented#SkPoint3'>SkPoint3</a> <a href='undocumented#SkPoint3'>array</a> <a href='undocumented#SkPoint3'>is</a> <a href='undocumented#SkPoint3'>mapped</a> <a href='undocumented#SkPoint3'>by</a> <a href='undocumented#SkPoint3'>multiplying</a> <a href='undocumented#SkPoint3'>each</a> <a href='undocumented#SkPoint3'>SkPoint3</a> <a href='undocumented#SkPoint3'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. <a href='SkMatrix_Reference#SkMatrix'>Given</a>:

| A B C |         | x |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> <a href='SkMatrix_Reference#Matrix'>F</a> |,  <a href='#SkMatrix_mapHomogeneousPoints_src'>src</a> = | <a href='#SkMatrix_mapHomogeneousPoints_src'>y</a> |
| G H <a href='#SkMatrix_I'>I</a> |         | <a href='#SkMatrix_I'>z</a> |

each resulting <a href='#SkMatrix_mapHomogeneousPoints_dst'>dst</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>is</a> <a href='SkPoint_Reference#SkPoint'>computed</a> <a href='SkPoint_Reference#SkPoint'>as</a>:

|A B C| |x|
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * <a href='#SkMatrix_mapHomogeneousPoints_src'>src</a> = |<a href='#SkMatrix_mapHomogeneousPoints_src'>D</a> <a href='#SkMatrix_mapHomogeneousPoints_src'>E</a> <a href='#SkMatrix_mapHomogeneousPoints_src'>F</a>| |<a href='#SkMatrix_mapHomogeneousPoints_src'>y</a>| = |<a href='#SkMatrix_mapHomogeneousPoints_src'>Ax</a>+<a href='#SkMatrix_mapHomogeneousPoints_src'>By</a>+<a href='#SkMatrix_mapHomogeneousPoints_src'>Cz</a> <a href='#SkMatrix_mapHomogeneousPoints_src'>Dx</a>+<a href='#SkMatrix_mapHomogeneousPoints_src'>Ey</a>+<a href='#SkMatrix_mapHomogeneousPoints_src'>Fz</a> <a href='#SkMatrix_mapHomogeneousPoints_src'>Gx</a>+<a href='#SkMatrix_mapHomogeneousPoints_src'>Hy</a>+<a href='#SkMatrix_mapHomogeneousPoints_src'>Iz</a>|
|G H <a href='#SkMatrix_I'>I</a>| |<a href='#SkMatrix_I'>z</a>|

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapHomogeneousPoints_dst'><code><strong>dst</strong></code></a></td>
    <td>storage for mapped <a href='undocumented#SkPoint3'>SkPoint3</a> <a href='undocumented#SkPoint3'>array</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapHomogeneousPoints_src'><code><strong>src</strong></code></a></td>
    <td><a href='undocumented#SkPoint3'>SkPoint3</a> <a href='undocumented#SkPoint3'>array</a> <a href='undocumented#SkPoint3'>to</a> <a href='undocumented#SkPoint3'>transform</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapHomogeneousPoints_count'><code><strong>count</strong></code></a></td>
    <td>items in <a href='undocumented#SkPoint3'>SkPoint3</a> <a href='undocumented#SkPoint3'>array</a> <a href='undocumented#SkPoint3'>to</a> <a href='undocumented#SkPoint3'>transform</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d56f93e4bc763c7ba4914321ed07a8b5"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapPoints'>mapPoints</a> <a href='#SkMatrix_mapXY'>mapXY</a> <a href='#SkMatrix_mapVectors'>mapVectors</a>

<a name='SkMatrix_mapXY'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_mapXY'>mapXY</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>, <a href='SkPoint_Reference#SkPoint'>SkPoint</a>* <a href='SkPoint_Reference#SkPoint'>result</a>) <a href='SkPoint_Reference#SkPoint'>const</a>
</pre>

Maps <a href='SkPoint_Reference#SkPoint'>SkPoint</a> (<a href='#SkMatrix_mapXY_x'>x</a>, <a href='#SkMatrix_mapXY_y'>y</a>) <a href='#SkMatrix_mapXY_y'>to</a> <a href='#SkMatrix_mapXY_result'>result</a>. <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>is</a> <a href='SkPoint_Reference#SkPoint'>mapped</a> <a href='SkPoint_Reference#SkPoint'>by</a> <a href='SkPoint_Reference#SkPoint'>multiplying</a> <a href='SkPoint_Reference#SkPoint'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. <a href='SkMatrix_Reference#SkMatrix'>Given</a>:

| A B C |        | <a href='#SkMatrix_mapXY_x'>x</a> |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> <a href='SkMatrix_Reference#Matrix'>F</a> |,  <a href='SkMatrix_Reference#Matrix'>pt</a> = | <a href='#SkMatrix_mapXY_y'>y</a> |
| G H <a href='#SkMatrix_I'>I</a> |        | 1 |

<a href='#SkMatrix_mapXY_result'>result</a> <a href='#SkMatrix_mapXY_result'>is</a> <a href='#SkMatrix_mapXY_result'>computed</a> <a href='#SkMatrix_mapXY_result'>as</a>:

|A B C| |<a href='#SkMatrix_mapXY_x'>x</a>|                               <a href='#SkMatrix_mapXY_x'>Ax</a>+<a href='#SkMatrix_mapXY_x'>By</a>+<a href='#SkMatrix_mapXY_x'>C</a>   <a href='#SkMatrix_mapXY_x'>Dx</a>+<a href='#SkMatrix_mapXY_x'>Ey</a>+<a href='#SkMatrix_mapXY_x'>F</a>
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * <a href='SkMatrix_Reference#Matrix'>pt</a> = |<a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> <a href='SkMatrix_Reference#Matrix'>F</a>| |<a href='#SkMatrix_mapXY_y'>y</a>| = |<a href='#SkMatrix_mapXY_y'>Ax</a>+<a href='#SkMatrix_mapXY_y'>By</a>+<a href='#SkMatrix_mapXY_y'>C</a> <a href='#SkMatrix_mapXY_y'>Dx</a>+<a href='#SkMatrix_mapXY_y'>Ey</a>+<a href='#SkMatrix_mapXY_y'>F</a> <a href='#SkMatrix_mapXY_y'>Gx</a>+<a href='#SkMatrix_mapXY_y'>Hy</a>+<a href='#SkMatrix_I'>I</a>| = ------- , -------
|G H <a href='#SkMatrix_I'>I</a>| |1|                               <a href='#SkMatrix_I'>Gx</a>+<a href='#SkMatrix_I'>Hy</a>+<a href='#SkMatrix_I'>I</a>   <a href='#SkMatrix_I'>Gx</a>+<a href='#SkMatrix_I'>Hy</a>+<a href='#SkMatrix_I'>I</a>

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapXY_x'><code><strong>x</strong></code></a></td>
    <td>x-axis value of <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>to</a> <a href='SkPoint_Reference#SkPoint'>map</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapXY_y'><code><strong>y</strong></code></a></td>
    <td>y-axis value of <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>to</a> <a href='SkPoint_Reference#SkPoint'>map</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapXY_result'><code><strong>result</strong></code></a></td>
    <td>storage for mapped <a href='SkPoint_Reference#SkPoint'>SkPoint</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="9e50185d502dc6903783679a84106089"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapPoints'>mapPoints</a> <a href='#SkMatrix_mapVectors'>mapVectors</a>

<a name='SkMatrix_mapXY_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkMatrix_mapXY'>mapXY</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>x</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>) <a href='undocumented#SkScalar'>const</a>
</pre>

Returns <a href='SkPoint_Reference#SkPoint'>SkPoint</a> (<a href='#SkMatrix_mapXY_2_x'>x</a>, <a href='#SkMatrix_mapXY_2_y'>y</a>) <a href='#SkMatrix_mapXY_2_y'>multiplied</a> <a href='#SkMatrix_mapXY_2_y'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. <a href='SkMatrix_Reference#SkMatrix'>Given</a>:

| A B C |        | <a href='#SkMatrix_mapXY_2_x'>x</a> |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> <a href='SkMatrix_Reference#Matrix'>F</a> |,  <a href='SkMatrix_Reference#Matrix'>pt</a> = | <a href='#SkMatrix_mapXY_2_y'>y</a> |
| G H <a href='#SkMatrix_I'>I</a> |        | 1 |

result is computed as:

|A B C| |<a href='#SkMatrix_mapXY_2_x'>x</a>|                               <a href='#SkMatrix_mapXY_2_x'>Ax</a>+<a href='#SkMatrix_mapXY_2_x'>By</a>+<a href='#SkMatrix_mapXY_2_x'>C</a>   <a href='#SkMatrix_mapXY_2_x'>Dx</a>+<a href='#SkMatrix_mapXY_2_x'>Ey</a>+<a href='#SkMatrix_mapXY_2_x'>F</a>
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * <a href='SkMatrix_Reference#Matrix'>pt</a> = |<a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> <a href='SkMatrix_Reference#Matrix'>F</a>| |<a href='#SkMatrix_mapXY_2_y'>y</a>| = |<a href='#SkMatrix_mapXY_2_y'>Ax</a>+<a href='#SkMatrix_mapXY_2_y'>By</a>+<a href='#SkMatrix_mapXY_2_y'>C</a> <a href='#SkMatrix_mapXY_2_y'>Dx</a>+<a href='#SkMatrix_mapXY_2_y'>Ey</a>+<a href='#SkMatrix_mapXY_2_y'>F</a> <a href='#SkMatrix_mapXY_2_y'>Gx</a>+<a href='#SkMatrix_mapXY_2_y'>Hy</a>+<a href='#SkMatrix_I'>I</a>| = ------- , -------
|G H <a href='#SkMatrix_I'>I</a>| |1|                               <a href='#SkMatrix_I'>Gx</a>+<a href='#SkMatrix_I'>Hy</a>+<a href='#SkMatrix_I'>I</a>   <a href='#SkMatrix_I'>Gx</a>+<a href='#SkMatrix_I'>Hy</a>+<a href='#SkMatrix_I'>I</a>

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapXY_2_x'><code><strong>x</strong></code></a></td>
    <td>x-axis value of <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>to</a> <a href='SkPoint_Reference#SkPoint'>map</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapXY_2_y'><code><strong>y</strong></code></a></td>
    <td>y-axis value of <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>to</a> <a href='SkPoint_Reference#SkPoint'>map</a></td>
  </tr>
</table>

### Return Value

mapped <a href='SkPoint_Reference#SkPoint'>SkPoint</a>

### Example

<div><fiddle-embed name="b1ead09c67a177ab8eace12b061610a7"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapPoints'>mapPoints</a> <a href='#SkMatrix_mapVectors'>mapVectors</a>

<a name='SkMatrix_mapVectors'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_mapVectors'>mapVectors</a>(<a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='SkPoint_Reference#SkVector'>dst</a>[], <a href='SkPoint_Reference#SkVector'>const</a> <a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='SkPoint_Reference#SkVector'>src</a>[], <a href='SkPoint_Reference#SkVector'>int</a> <a href='SkPoint_Reference#SkVector'>count</a>) <a href='SkPoint_Reference#SkVector'>const</a>
</pre>

Maps <a href='#SkMatrix_mapVectors_src'>src</a> <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>array</a> <a href='SkPoint_Reference#Vector'>of</a> <a href='SkPoint_Reference#Vector'>length</a> <a href='#SkMatrix_mapVectors_count'>count</a> <a href='#SkMatrix_mapVectors_count'>to</a> <a href='SkPoint_Reference#Vector'>vector</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> <a href='SkPoint_Reference#SkPoint'>of</a> <a href='SkPoint_Reference#SkPoint'>equal</a> <a href='SkPoint_Reference#SkPoint'>or</a> <a href='SkPoint_Reference#SkPoint'>greater</a>
length. <a href='SkPoint_Reference#Vector'>Vectors</a> <a href='SkPoint_Reference#Vector'>are</a> <a href='SkPoint_Reference#Vector'>mapped</a> <a href='SkPoint_Reference#Vector'>by</a> <a href='SkPoint_Reference#Vector'>multiplying</a> <a href='SkPoint_Reference#Vector'>each</a> <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>treating</a>
<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>translation</a> <a href='SkMatrix_Reference#SkMatrix'>as</a> <a href='SkMatrix_Reference#SkMatrix'>zero</a>. <a href='SkMatrix_Reference#SkMatrix'>Given</a>:

| A B 0 |         | x |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> 0 |,  <a href='#SkMatrix_mapVectors_src'>src</a> = | <a href='#SkMatrix_mapVectors_src'>y</a> |
| G H <a href='#SkMatrix_I'>I</a> |         | 1 |

where

for (i = 0; i < <a href='#SkMatrix_mapVectors_count'>count</a>; ++<a href='#SkMatrix_mapVectors_count'>i</a>) {
x = <a href='#SkMatrix_mapVectors_src'>src</a>[<a href='#SkMatrix_mapVectors_src'>i</a>].<a href='#SkMatrix_mapVectors_src'>fX</a>
y = <a href='#SkMatrix_mapVectors_src'>src</a>[<a href='#SkMatrix_mapVectors_src'>i</a>].<a href='#SkMatrix_mapVectors_src'>fY</a>
}

each <a href='#SkMatrix_mapVectors_dst'>dst</a> <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>is</a> <a href='SkPoint_Reference#Vector'>computed</a> <a href='SkPoint_Reference#Vector'>as</a>:

|A B 0| |x|                            Ax+By     Dx+Ey
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * <a href='#SkMatrix_mapVectors_src'>src</a> = |<a href='#SkMatrix_mapVectors_src'>D</a> <a href='#SkMatrix_mapVectors_src'>E</a> 0| |<a href='#SkMatrix_mapVectors_src'>y</a>| = |<a href='#SkMatrix_mapVectors_src'>Ax</a>+<a href='#SkMatrix_mapVectors_src'>By</a> <a href='#SkMatrix_mapVectors_src'>Dx</a>+<a href='#SkMatrix_mapVectors_src'>Ey</a> <a href='#SkMatrix_mapVectors_src'>Gx</a>+<a href='#SkMatrix_mapVectors_src'>Hy</a>+<a href='#SkMatrix_I'>I</a>| = ------- , -------
|G H <a href='#SkMatrix_I'>I</a>| |1|                           <a href='#SkMatrix_I'>Gx</a>+<a href='#SkMatrix_I'>Hy</a>+<a href='#SkMatrix_I'>I</a>   <a href='#SkMatrix_I'>Gx</a>+<a href='#SkMatrix_I'>Hy</a>+<a href='#SkMatrix_I'>I</a>

<a href='#SkMatrix_mapVectors_src'>src</a> <a href='#SkMatrix_mapVectors_src'>and</a> <a href='#SkMatrix_mapVectors_dst'>dst</a> <a href='#SkMatrix_mapVectors_dst'>may</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>to</a> <a href='SkPoint_Reference#Point'>the</a> <a href='SkPoint_Reference#Point'>same</a> <a href='SkPoint_Reference#Point'>storage</a>.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapVectors_dst'><code><strong>dst</strong></code></a></td>
    <td>storage for mapped <a href='SkPoint_Reference#Vector'>vectors</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapVectors_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkPoint_Reference#Vector'>vectors</a> <a href='SkPoint_Reference#Vector'>to</a> <a href='SkPoint_Reference#Vector'>transform</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapVectors_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='SkPoint_Reference#Vector'>vectors</a> <a href='SkPoint_Reference#Vector'>to</a> <a href='SkPoint_Reference#Vector'>transform</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="918a9778c3d7d5cb306692784399f6dc"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapVector'>mapVector</a> <a href='#SkMatrix_mapPoints'>mapPoints</a> <a href='#SkMatrix_mapXY'>mapXY</a>

<a name='SkMatrix_mapVectors_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_mapVectors'>mapVectors</a>(<a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='SkPoint_Reference#SkVector'>vecs</a>[], <a href='SkPoint_Reference#SkVector'>int</a> <a href='SkPoint_Reference#SkVector'>count</a>) <a href='SkPoint_Reference#SkVector'>const</a>
</pre>

Maps <a href='#SkMatrix_mapVectors_2_vecs'>vecs</a> <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>array</a> <a href='SkPoint_Reference#Vector'>of</a> <a href='SkPoint_Reference#Vector'>length</a> <a href='#SkMatrix_mapVectors_2_count'>count</a> <a href='#SkMatrix_mapVectors_2_count'>in</a> <a href='#SkMatrix_mapVectors_2_count'>place</a>, <a href='#SkMatrix_mapVectors_2_count'>multiplying</a> <a href='#SkMatrix_mapVectors_2_count'>each</a> <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>by</a>
<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>treating</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>translation</a> <a href='SkMatrix_Reference#SkMatrix'>as</a> <a href='SkMatrix_Reference#SkMatrix'>zero</a>. <a href='SkMatrix_Reference#SkMatrix'>Given</a>:

| A B 0 |         | x |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> 0 |,  <a href='SkMatrix_Reference#Matrix'>vec</a> = | <a href='SkMatrix_Reference#Matrix'>y</a> |
| G H <a href='#SkMatrix_I'>I</a> |         | 1 |

where

for (i = 0; i < <a href='#SkMatrix_mapVectors_2_count'>count</a>; ++<a href='#SkMatrix_mapVectors_2_count'>i</a>) {
x = <a href='#SkMatrix_mapVectors_2_vecs'>vecs</a>[<a href='#SkMatrix_mapVectors_2_vecs'>i</a>].<a href='#SkMatrix_mapVectors_2_vecs'>fX</a>
y = <a href='#SkMatrix_mapVectors_2_vecs'>vecs</a>[<a href='#SkMatrix_mapVectors_2_vecs'>i</a>].<a href='#SkMatrix_mapVectors_2_vecs'>fY</a>
}

each result <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>is</a> <a href='SkPoint_Reference#Vector'>computed</a> <a href='SkPoint_Reference#Vector'>as</a>:

|A B 0| |x|                            Ax+By     Dx+Ey
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * <a href='SkMatrix_Reference#Matrix'>vec</a> = |<a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> 0| |<a href='SkMatrix_Reference#Matrix'>y</a>| = |<a href='SkMatrix_Reference#Matrix'>Ax</a>+<a href='SkMatrix_Reference#Matrix'>By</a> <a href='SkMatrix_Reference#Matrix'>Dx</a>+<a href='SkMatrix_Reference#Matrix'>Ey</a> <a href='SkMatrix_Reference#Matrix'>Gx</a>+<a href='SkMatrix_Reference#Matrix'>Hy</a>+<a href='#SkMatrix_I'>I</a>| = ------- , -------
|G H <a href='#SkMatrix_I'>I</a>| |1|                           <a href='#SkMatrix_I'>Gx</a>+<a href='#SkMatrix_I'>Hy</a>+<a href='#SkMatrix_I'>I</a>   <a href='#SkMatrix_I'>Gx</a>+<a href='#SkMatrix_I'>Hy</a>+<a href='#SkMatrix_I'>I</a>

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapVectors_2_vecs'><code><strong>vecs</strong></code></a></td>
    <td><a href='SkPoint_Reference#Vector'>vectors</a> <a href='SkPoint_Reference#Vector'>to</a> <a href='SkPoint_Reference#Vector'>transform</a>, <a href='SkPoint_Reference#Vector'>and</a> <a href='SkPoint_Reference#Vector'>storage</a> <a href='SkPoint_Reference#Vector'>for</a> <a href='SkPoint_Reference#Vector'>mapped</a> <a href='SkPoint_Reference#Vector'>vectors</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapVectors_2_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='SkPoint_Reference#Vector'>vectors</a> <a href='SkPoint_Reference#Vector'>to</a> <a href='SkPoint_Reference#Vector'>transform</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="5754501a00a1323e76353fb53153e939"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapVector'>mapVector</a> <a href='#SkMatrix_mapPoints'>mapPoints</a> <a href='#SkMatrix_mapXY'>mapXY</a>

<a name='SkMatrix_mapVector'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_mapVector'>mapVector</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>, <a href='SkPoint_Reference#SkVector'>SkVector</a>* <a href='SkPoint_Reference#SkVector'>result</a>) <a href='SkPoint_Reference#SkVector'>const</a>
</pre>

Maps <a href='SkPoint_Reference#Vector'>vector</a> (<a href='#SkMatrix_mapVector_dx'>dx</a>, <a href='#SkMatrix_mapVector_dy'>dy</a>) <a href='#SkMatrix_mapVector_dy'>to</a> <a href='#SkMatrix_mapVector_result'>result</a>. <a href='SkPoint_Reference#Vector'>Vector</a> <a href='SkPoint_Reference#Vector'>is</a> <a href='SkPoint_Reference#Vector'>mapped</a> <a href='SkPoint_Reference#Vector'>by</a> <a href='SkPoint_Reference#Vector'>multiplying</a> <a href='SkPoint_Reference#Vector'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>,
treating <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>translation</a> <a href='SkMatrix_Reference#SkMatrix'>as</a> <a href='SkMatrix_Reference#SkMatrix'>zero</a>. <a href='SkMatrix_Reference#SkMatrix'>Given</a>:

| A B 0 |         | <a href='#SkMatrix_mapVector_dx'>dx</a> |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> 0 |,  <a href='SkMatrix_Reference#Matrix'>vec</a> = | <a href='#SkMatrix_mapVector_dy'>dy</a> |
| G H <a href='#SkMatrix_I'>I</a> |         |  1 |

each <a href='#SkMatrix_mapVector_result'>result</a> <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>is</a> <a href='SkPoint_Reference#Vector'>computed</a> <a href='SkPoint_Reference#Vector'>as</a>:

|A B 0| |<a href='#SkMatrix_mapVector_dx'>dx</a>|                                        <a href='#SkMatrix_mapVector_dx'>A</a>*<a href='#SkMatrix_mapVector_dx'>dx</a>+<a href='#SkMatrix_mapVector_dx'>B</a>*<a href='#SkMatrix_mapVector_dy'>dy</a>     <a href='#SkMatrix_mapVector_dy'>D</a>*<a href='#SkMatrix_mapVector_dx'>dx</a>+<a href='#SkMatrix_mapVector_dx'>E</a>*<a href='#SkMatrix_mapVector_dy'>dy</a>
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * <a href='SkMatrix_Reference#Matrix'>vec</a> = |<a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> 0| |<a href='#SkMatrix_mapVector_dy'>dy</a>| = |<a href='#SkMatrix_mapVector_dy'>A</a>*<a href='#SkMatrix_mapVector_dx'>dx</a>+<a href='#SkMatrix_mapVector_dx'>B</a>*<a href='#SkMatrix_mapVector_dy'>dy</a> <a href='#SkMatrix_mapVector_dy'>D</a>*<a href='#SkMatrix_mapVector_dx'>dx</a>+<a href='#SkMatrix_mapVector_dx'>E</a>*<a href='#SkMatrix_mapVector_dy'>dy</a> <a href='#SkMatrix_mapVector_dy'>G</a>*<a href='#SkMatrix_mapVector_dx'>dx</a>+<a href='#SkMatrix_mapVector_dx'>H</a>*<a href='#SkMatrix_mapVector_dy'>dy</a>+<a href='#SkMatrix_I'>I</a>| = ----------- , -----------
|G H <a href='#SkMatrix_I'>I</a>| | 1|                                       <a href='#SkMatrix_I'>G</a>*<a href='#SkMatrix_mapVector_dx'>dx</a>+<a href='#SkMatrix_mapVector_dx'>H</a>*<a href='#SkMatrix_mapVector_dy'>dy</a>+<a href='#SkMatrix_I'>I</a>   <a href='#SkMatrix_I'>G</a>*<a href='#SkMatrix_mapVector_dx'>dx</a>+*<a href='#SkMatrix_mapVector_dx'>dHy</a>+<a href='#SkMatrix_I'>I</a>

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapVector_dx'><code><strong>dx</strong></code></a></td>
    <td>x-axis value of <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>to</a> <a href='SkPoint_Reference#Vector'>map</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapVector_dy'><code><strong>dy</strong></code></a></td>
    <td>y-axis value of <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>to</a> <a href='SkPoint_Reference#Vector'>map</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapVector_result'><code><strong>result</strong></code></a></td>
    <td>storage for mapped <a href='SkPoint_Reference#Vector'>vector</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="aed143fc6cd0bce4ed029b98d1e61f2d"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapVectors'>mapVectors</a> <a href='#SkMatrix_mapPoints'>mapPoints</a> <a href='#SkMatrix_mapXY'>mapXY</a>

<a name='SkMatrix_mapVector_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='#SkMatrix_mapVector'>mapVector</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>dy</a>) <a href='undocumented#SkScalar'>const</a>
</pre>

Returns <a href='SkPoint_Reference#Vector'>vector</a> (<a href='#SkMatrix_mapVector_2_dx'>dx</a>, <a href='#SkMatrix_mapVector_2_dy'>dy</a>) <a href='#SkMatrix_mapVector_2_dy'>multiplied</a> <a href='#SkMatrix_mapVector_2_dy'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, <a href='SkMatrix_Reference#SkMatrix'>treating</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>translation</a> <a href='SkMatrix_Reference#SkMatrix'>as</a> <a href='SkMatrix_Reference#SkMatrix'>zero</a>.
Given:

| A B 0 |         | <a href='#SkMatrix_mapVector_2_dx'>dx</a> |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> 0 |,  <a href='SkMatrix_Reference#Matrix'>vec</a> = | <a href='#SkMatrix_mapVector_2_dy'>dy</a> |
| G H <a href='#SkMatrix_I'>I</a> |         |  1 |

each result <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>is</a> <a href='SkPoint_Reference#Vector'>computed</a> <a href='SkPoint_Reference#Vector'>as</a>:

|A B 0| |<a href='#SkMatrix_mapVector_2_dx'>dx</a>|                                        <a href='#SkMatrix_mapVector_2_dx'>A</a>*<a href='#SkMatrix_mapVector_2_dx'>dx</a>+<a href='#SkMatrix_mapVector_2_dx'>B</a>*<a href='#SkMatrix_mapVector_2_dy'>dy</a>     <a href='#SkMatrix_mapVector_2_dy'>D</a>*<a href='#SkMatrix_mapVector_2_dx'>dx</a>+<a href='#SkMatrix_mapVector_2_dx'>E</a>*<a href='#SkMatrix_mapVector_2_dy'>dy</a>
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * <a href='SkMatrix_Reference#Matrix'>vec</a> = |<a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> 0| |<a href='#SkMatrix_mapVector_2_dy'>dy</a>| = |<a href='#SkMatrix_mapVector_2_dy'>A</a>*<a href='#SkMatrix_mapVector_2_dx'>dx</a>+<a href='#SkMatrix_mapVector_2_dx'>B</a>*<a href='#SkMatrix_mapVector_2_dy'>dy</a> <a href='#SkMatrix_mapVector_2_dy'>D</a>*<a href='#SkMatrix_mapVector_2_dx'>dx</a>+<a href='#SkMatrix_mapVector_2_dx'>E</a>*<a href='#SkMatrix_mapVector_2_dy'>dy</a> <a href='#SkMatrix_mapVector_2_dy'>G</a>*<a href='#SkMatrix_mapVector_2_dx'>dx</a>+<a href='#SkMatrix_mapVector_2_dx'>H</a>*<a href='#SkMatrix_mapVector_2_dy'>dy</a>+<a href='#SkMatrix_I'>I</a>| = ----------- , -----------
|G H <a href='#SkMatrix_I'>I</a>| | 1|                                       <a href='#SkMatrix_I'>G</a>*<a href='#SkMatrix_mapVector_2_dx'>dx</a>+<a href='#SkMatrix_mapVector_2_dx'>H</a>*<a href='#SkMatrix_mapVector_2_dy'>dy</a>+<a href='#SkMatrix_I'>I</a>   <a href='#SkMatrix_I'>G</a>*<a href='#SkMatrix_mapVector_2_dx'>dx</a>+*<a href='#SkMatrix_mapVector_2_dx'>dHy</a>+<a href='#SkMatrix_I'>I</a>

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapVector_2_dx'><code><strong>dx</strong></code></a></td>
    <td>x-axis value of <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>to</a> <a href='SkPoint_Reference#Vector'>map</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapVector_2_dy'><code><strong>dy</strong></code></a></td>
    <td>y-axis value of <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>to</a> <a href='SkPoint_Reference#Vector'>map</a></td>
  </tr>
</table>

### Return Value

mapped <a href='SkPoint_Reference#Vector'>vector</a>

### Example

<div><fiddle-embed name="8bf1518db3f369696cd3065b541a8bd7"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapVectors'>mapVectors</a> <a href='#SkMatrix_mapPoints'>mapPoints</a> <a href='#SkMatrix_mapXY'>mapXY</a>

<a name='SkMatrix_mapRect'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_mapRect'>mapRect</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>dst</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>src</a>) <a href='SkRect_Reference#SkRect'>const</a>
</pre>

Sets <a href='#SkMatrix_mapRect_dst'>dst</a> <a href='#SkMatrix_mapRect_dst'>to</a> <a href='#SkMatrix_mapRect_dst'>bounds</a> <a href='#SkMatrix_mapRect_dst'>of</a> <a href='#SkMatrix_mapRect_src'>src</a> <a href='#SkMatrix_mapRect_src'>corners</a> <a href='#SkMatrix_mapRect_src'>mapped</a> <a href='#SkMatrix_mapRect_src'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
Returns true if mapped corners are <a href='#SkMatrix_mapRect_dst'>dst</a> <a href='#SkMatrix_mapRect_dst'>corners</a>.

Returned value is the same as calling <a href='#SkMatrix_rectStaysRect'>rectStaysRect</a>().

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapRect_dst'><code><strong>dst</strong></code></a></td>
    <td>storage for bounds of mapped <a href='SkPoint_Reference#SkPoint'>SkPoint</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapRect_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>map</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkMatrix_mapRect_dst'>dst</a> <a href='#SkMatrix_mapRect_dst'>is</a> <a href='#SkMatrix_mapRect_dst'>equivalent</a> <a href='#SkMatrix_mapRect_dst'>to</a> <a href='#SkMatrix_mapRect_dst'>mapped</a> <a href='#SkMatrix_mapRect_src'>src</a>

### Example

<div><fiddle-embed name="dbcf928b035a31ca69c99392e2e2cca9"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapPoints'>mapPoints</a> <a href='#SkMatrix_rectStaysRect'>rectStaysRect</a>

<a name='SkMatrix_mapRect_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_mapRect'>mapRect</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#Rect'>rect</a>) <a href='SkRect_Reference#Rect'>const</a>
</pre>

Sets <a href='#SkMatrix_mapRect_2_rect'>rect</a> <a href='#SkMatrix_mapRect_2_rect'>to</a> <a href='#SkMatrix_mapRect_2_rect'>bounds</a> <a href='#SkMatrix_mapRect_2_rect'>of</a> <a href='#SkMatrix_mapRect_2_rect'>rect</a> <a href='#SkMatrix_mapRect_2_rect'>corners</a> <a href='#SkMatrix_mapRect_2_rect'>mapped</a> <a href='#SkMatrix_mapRect_2_rect'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
Returns true if mapped corners are computed <a href='#SkMatrix_mapRect_2_rect'>rect</a> <a href='#SkMatrix_mapRect_2_rect'>corners</a>.

Returned value is the same as calling <a href='#SkMatrix_rectStaysRect'>rectStaysRect</a>().

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapRect_2_rect'><code><strong>rect</strong></code></a></td>
    <td>rectangle to map, and storage for bounds of mapped corners</td>
  </tr>
</table>

### Return Value

true if result is equivalent to mapped <a href='#SkMatrix_mapRect_2_rect'>rect</a>

### Example

<div><fiddle-embed name="5fafd0bd23d1ed37425b970b4a3c6cc9"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapRectScaleTranslate'>mapRectScaleTranslate</a> <a href='#SkMatrix_mapPoints'>mapPoints</a> <a href='#SkMatrix_rectStaysRect'>rectStaysRect</a>

<a name='SkMatrix_mapRect_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkMatrix_mapRect'>mapRect</a>(<a href='#SkMatrix_mapRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>src</a>) <a href='SkRect_Reference#SkRect'>const</a>
</pre>

Returns bounds of <a href='#SkMatrix_mapRect_3_src'>src</a> <a href='#SkMatrix_mapRect_3_src'>corners</a> <a href='#SkMatrix_mapRect_3_src'>mapped</a> <a href='#SkMatrix_mapRect_3_src'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapRect_3_src'><code><strong>src</strong></code></a></td>
    <td>rectangle to map</td>
  </tr>
</table>

### Return Value

mapped bounds

### Example

<div><fiddle-embed name="3b7b1f884437ab450f986234e4aec27f"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapRectToQuad'>mapRectToQuad</a> <a href='#SkMatrix_mapRectScaleTranslate'>mapRectScaleTranslate</a>

<a name='SkMatrix_mapRectToQuad'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_mapRectToQuad'>mapRectToQuad</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>dst</a>[4], <a href='SkPoint_Reference#SkPoint'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>) <a href='SkRect_Reference#Rect'>const</a>
</pre>

Maps four corners of <a href='#SkMatrix_mapRectToQuad_rect'>rect</a> <a href='#SkMatrix_mapRectToQuad_rect'>to</a> <a href='#SkMatrix_mapRectToQuad_dst'>dst</a>. <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>are</a> <a href='SkPoint_Reference#SkPoint'>mapped</a> <a href='SkPoint_Reference#SkPoint'>by</a> <a href='SkPoint_Reference#SkPoint'>multiplying</a> <a href='SkPoint_Reference#SkPoint'>each</a>
<a href='#SkMatrix_mapRectToQuad_rect'>rect</a> <a href='#SkMatrix_mapRectToQuad_rect'>corner</a> <a href='#SkMatrix_mapRectToQuad_rect'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. <a href='#SkMatrix_mapRectToQuad_rect'>rect</a> <a href='#SkMatrix_mapRectToQuad_rect'>corner</a> <a href='#SkMatrix_mapRectToQuad_rect'>is</a> <a href='#SkMatrix_mapRectToQuad_rect'>processed</a> <a href='#SkMatrix_mapRectToQuad_rect'>in</a> <a href='#SkMatrix_mapRectToQuad_rect'>this</a> <a href='#SkMatrix_mapRectToQuad_rect'>order</a>:
(<a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fLeft'>fLeft</a>, <a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fTop'>fTop</a>), (<a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fRight'>fRight</a>, <a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fTop'>fTop</a>), (<a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fRight'>fRight</a>, <a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fBottom'>fBottom</a>),
(<a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fLeft'>fLeft</a>, <a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fBottom'>fBottom</a>).

<a href='#SkMatrix_mapRectToQuad_rect'>rect</a> <a href='#SkMatrix_mapRectToQuad_rect'>may</a> <a href='#SkMatrix_mapRectToQuad_rect'>be</a> <a href='#SkMatrix_mapRectToQuad_rect'>empty</a>: <a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fLeft'>fLeft</a> <a href='#SkRect_fLeft'>may</a> <a href='#SkRect_fLeft'>be</a> <a href='#SkRect_fLeft'>greater</a> <a href='#SkRect_fLeft'>than</a> <a href='#SkRect_fLeft'>or</a> <a href='#SkRect_fLeft'>equal</a> <a href='#SkRect_fLeft'>to</a> <a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fRight'>fRight</a>;
<a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fTop'>fTop</a> <a href='#SkRect_fTop'>may</a> <a href='#SkRect_fTop'>be</a> <a href='#SkRect_fTop'>greater</a> <a href='#SkRect_fTop'>than</a> <a href='#SkRect_fTop'>or</a> <a href='#SkRect_fTop'>equal</a> <a href='#SkRect_fTop'>to</a> <a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fBottom'>fBottom</a>.

Given:

| A B C |        | x |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> <a href='SkMatrix_Reference#Matrix'>F</a> |,  <a href='SkMatrix_Reference#Matrix'>pt</a> = | <a href='SkMatrix_Reference#Matrix'>y</a> |
| G H <a href='#SkMatrix_I'>I</a> |        | 1 |

where pt is initialized from each of (<a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fLeft'>fLeft</a>, <a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fTop'>fTop</a>),
(<a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fRight'>fRight</a>, <a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fTop'>fTop</a>), (<a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fRight'>fRight</a>, <a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fBottom'>fBottom</a>), (<a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fLeft'>fLeft</a>, <a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fBottom'>fBottom</a>),
each <a href='#SkMatrix_mapRectToQuad_dst'>dst</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='SkPoint_Reference#SkPoint'>is</a> <a href='SkPoint_Reference#SkPoint'>computed</a> <a href='SkPoint_Reference#SkPoint'>as</a>:

|A B C| |x|                               Ax+By+C   Dx+Ey+F
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * <a href='SkMatrix_Reference#Matrix'>pt</a> = |<a href='SkMatrix_Reference#Matrix'>D</a> <a href='SkMatrix_Reference#Matrix'>E</a> <a href='SkMatrix_Reference#Matrix'>F</a>| |<a href='SkMatrix_Reference#Matrix'>y</a>| = |<a href='SkMatrix_Reference#Matrix'>Ax</a>+<a href='SkMatrix_Reference#Matrix'>By</a>+<a href='SkMatrix_Reference#Matrix'>C</a> <a href='SkMatrix_Reference#Matrix'>Dx</a>+<a href='SkMatrix_Reference#Matrix'>Ey</a>+<a href='SkMatrix_Reference#Matrix'>F</a> <a href='SkMatrix_Reference#Matrix'>Gx</a>+<a href='SkMatrix_Reference#Matrix'>Hy</a>+<a href='#SkMatrix_I'>I</a>| = ------- , -------
|G H <a href='#SkMatrix_I'>I</a>| |1|                               <a href='#SkMatrix_I'>Gx</a>+<a href='#SkMatrix_I'>Hy</a>+<a href='#SkMatrix_I'>I</a>   <a href='#SkMatrix_I'>Gx</a>+<a href='#SkMatrix_I'>Hy</a>+<a href='#SkMatrix_I'>I</a>

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapRectToQuad_dst'><code><strong>dst</strong></code></a></td>
    <td>storage for mapped corner <a href='SkPoint_Reference#SkPoint'>SkPoint</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapRectToQuad_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>map</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c69cd2a590b5733c3cbc92cb9ceed3f5"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapRect'>mapRect</a> <a href='#SkMatrix_mapRectScaleTranslate'>mapRectScaleTranslate</a>

<a name='SkMatrix_mapRectScaleTranslate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_mapRectScaleTranslate'>mapRectScaleTranslate</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#SkRect'>dst</a>, <a href='SkRect_Reference#SkRect'>const</a> <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#SkRect'>src</a>) <a href='SkRect_Reference#SkRect'>const</a>
</pre>

Sets <a href='#SkMatrix_mapRectScaleTranslate_dst'>dst</a> <a href='#SkMatrix_mapRectScaleTranslate_dst'>to</a> <a href='#SkMatrix_mapRectScaleTranslate_dst'>bounds</a> <a href='#SkMatrix_mapRectScaleTranslate_dst'>of</a> <a href='#SkMatrix_mapRectScaleTranslate_src'>src</a> <a href='#SkMatrix_mapRectScaleTranslate_src'>corners</a> <a href='#SkMatrix_mapRectScaleTranslate_src'>mapped</a> <a href='#SkMatrix_mapRectScaleTranslate_src'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. <a href='SkMatrix_Reference#SkMatrix'>If</a> <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>contains</a>
elements other than scale or translate: asserts if SK_DEBUG is defined;
otherwise, results are undefined.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapRectScaleTranslate_dst'><code><strong>dst</strong></code></a></td>
    <td>storage for bounds of mapped <a href='SkPoint_Reference#SkPoint'>SkPoint</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapRectScaleTranslate_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> <a href='SkRect_Reference#SkRect'>to</a> <a href='SkRect_Reference#SkRect'>map</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="62bc26989c2b4c2a54d516596a71dd97"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapRect'>mapRect</a> <a href='#SkMatrix_mapRectToQuad'>mapRectToQuad</a> <a href='#SkMatrix_isScaleTranslate'>isScaleTranslate</a> <a href='#SkMatrix_rectStaysRect'>rectStaysRect</a>

<a name='SkMatrix_mapRadius'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_mapRadius'>mapRadius</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>radius</a>) <a href='undocumented#SkScalar'>const</a>
</pre>

Returns geometric mean <a href='#SkMatrix_mapRadius_radius'>radius</a> <a href='#SkMatrix_mapRadius_radius'>of</a> <a href='#SkMatrix_mapRadius_radius'>ellipse</a> <a href='#SkMatrix_mapRadius_radius'>formed</a> <a href='#SkMatrix_mapRadius_radius'>by</a> <a href='#SkMatrix_mapRadius_radius'>constructing</a> <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>of</a>
<a href='undocumented#Size'>size</a> <a href='#SkMatrix_mapRadius_radius'>radius</a>, <a href='#SkMatrix_mapRadius_radius'>and</a> <a href='#SkMatrix_mapRadius_radius'>mapping</a> <a href='#SkMatrix_mapRadius_radius'>constructed</a> <a href='undocumented#Circle'>circle</a> <a href='undocumented#Circle'>with</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. <a href='SkMatrix_Reference#SkMatrix'>The</a> <a href='SkMatrix_Reference#SkMatrix'>result</a> <a href='SkMatrix_Reference#SkMatrix'>squared</a> <a href='SkMatrix_Reference#SkMatrix'>is</a>
equal to the major axis length times the minor axis length.
Result is not meaningful if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>contains</a> <a href='SkMatrix_Reference#SkMatrix'>perspective</a> <a href='SkMatrix_Reference#SkMatrix'>elements</a>.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapRadius_radius'><code><strong>radius</strong></code></a></td>
    <td><a href='undocumented#Circle'>circle</a> <a href='undocumented#Size'>size</a> <a href='undocumented#Size'>to</a> <a href='undocumented#Size'>map</a></td>
  </tr>
</table>

### Return Value

average mapped <a href='#SkMatrix_mapRadius_radius'>radius</a>

### Example

<div><fiddle-embed name="6d6f2082fcf59d9f02bfb1758b87db69"><div>The area enclosed by a square with sides equal to mappedRadius is the same as
the area enclosed by the ellipse major and minor axes.
</div></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapVector'>mapVector</a>

<a name='SkMatrix_isFixedStepInX'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_isFixedStepInX'>isFixedStepInX</a>() <a href='#SkMatrix_isFixedStepInX'>const</a>
</pre>

Returns true if a unit step on x-axis at some y-axis value mapped through <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
can be represented by a constant <a href='SkPoint_Reference#Vector'>vector</a>. <a href='SkPoint_Reference#Vector'>Returns</a> <a href='SkPoint_Reference#Vector'>true</a> <a href='SkPoint_Reference#Vector'>if</a> <a href='#SkMatrix_getType'>getType</a>() <a href='#SkMatrix_getType'>returns</a>
<a href='#SkMatrix_kIdentity_Mask'>kIdentity_Mask</a>, <a href='#SkMatrix_kIdentity_Mask'>or</a> <a href='#SkMatrix_kIdentity_Mask'>combinations</a> <a href='#SkMatrix_kIdentity_Mask'>of</a>: <a href='#SkMatrix_kTranslate_Mask'>kTranslate_Mask</a>, <a href='#SkMatrix_kScale_Mask'>kScale_Mask</a>, <a href='#SkMatrix_kScale_Mask'>and</a> <a href='#SkMatrix_kAffine_Mask'>kAffine_Mask</a>.

May return true if <a href='#SkMatrix_getType'>getType</a>() <a href='#SkMatrix_getType'>returns</a> <a href='#SkMatrix_kPerspective_Mask'>kPerspective_Mask</a>, <a href='#SkMatrix_kPerspective_Mask'>but</a> <a href='#SkMatrix_kPerspective_Mask'>only</a> <a href='#SkMatrix_kPerspective_Mask'>when</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
does not include rotation or skewing along the y-axis.

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>does</a> <a href='SkMatrix_Reference#SkMatrix'>not</a> <a href='SkMatrix_Reference#SkMatrix'>have</a> <a href='SkMatrix_Reference#SkMatrix'>complex</a> <a href='SkMatrix_Reference#SkMatrix'>perspective</a>

### Example

<div><fiddle-embed name="ab57b232acef69f26de9cb23d23c8a1a">

#### Example Output

~~~~
[  1.0000   0.0000   0.0000][  0.0000   1.0000   0.0000][  0.0000   0.0000   1.0000]
isFixedStepInX: true
[  1.0000   0.0000   0.0000][  0.0000   2.0000   0.0000][  0.0000   0.0000   1.0000]
isFixedStepInX: true
[  1.0000   0.0000   0.0000][  0.0000   1.0000   0.0000][  0.0000   0.1000   1.0000]
isFixedStepInX: true
[  1.0000   0.0000   0.0000][  0.0000   2.0000   0.0000][  0.0000   0.1000   1.0000]
isFixedStepInX: true
[  1.0000   0.0000   0.0000][  0.0000   1.0000   0.0000][  0.1000   0.0000   1.0000]
isFixedStepInX: false
[  1.0000   0.0000   0.0000][  0.0000   2.0000   0.0000][  0.1000   0.0000   1.0000]
isFixedStepInX: false
[  1.0000   0.0000   0.0000][  0.0000   1.0000   0.0000][  0.1000   0.1000   1.0000]
isFixedStepInX: false
[  1.0000   0.0000   0.0000][  0.0000   2.0000   0.0000][  0.1000   0.1000   1.0000]
isFixedStepInX: false
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_fixedStepInX'>fixedStepInX</a> <a href='#SkMatrix_getType'>getType</a>

<a name='SkMatrix_fixedStepInX'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='#SkMatrix_fixedStepInX'>fixedStepInX</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>y</a>) <a href='undocumented#SkScalar'>const</a>
</pre>

Returns <a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>representing</a> <a href='SkPoint_Reference#Vector'>a</a> <a href='SkPoint_Reference#Vector'>unit</a> <a href='SkPoint_Reference#Vector'>step</a> <a href='SkPoint_Reference#Vector'>on</a> <a href='SkPoint_Reference#Vector'>x-axis</a> <a href='SkPoint_Reference#Vector'>at</a> <a href='#SkMatrix_fixedStepInX_y'>y</a> <a href='#SkMatrix_fixedStepInX_y'>mapped</a> <a href='#SkMatrix_fixedStepInX_y'>through</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
If <a href='#SkMatrix_isFixedStepInX'>isFixedStepInX</a>() <a href='#SkMatrix_isFixedStepInX'>is</a> <a href='#SkMatrix_isFixedStepInX'>false</a>, <a href='#SkMatrix_isFixedStepInX'>returned</a> <a href='#SkMatrix_isFixedStepInX'>value</a> <a href='#SkMatrix_isFixedStepInX'>is</a> <a href='#SkMatrix_isFixedStepInX'>undefined</a>.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_fixedStepInX_y'><code><strong>y</strong></code></a></td>
    <td>position of <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>parallel</a> <a href='undocumented#Line'>to</a> <a href='undocumented#Line'>x-axis</a></td>
  </tr>
</table>

### Return Value

<a href='SkPoint_Reference#Vector'>vector</a> <a href='SkPoint_Reference#Vector'>advance</a> <a href='SkPoint_Reference#Vector'>of</a> <a href='SkPoint_Reference#Vector'>mapped</a> <a href='SkPoint_Reference#Vector'>unit</a> <a href='SkPoint_Reference#Vector'>step</a> <a href='SkPoint_Reference#Vector'>on</a> <a href='SkPoint_Reference#Vector'>x-axis</a>

### Example

<div><fiddle-embed name="fad6b92b21b1e1deeae61978cec2d232"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_isFixedStepInX'>isFixedStepInX</a> <a href='#SkMatrix_getType'>getType</a>

<a name='SkMatrix_cheapEqualTo'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_cheapEqualTo'>cheapEqualTo</a>(<a href='#SkMatrix_cheapEqualTo'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#SkMatrix'>m</a>) <a href='SkMatrix_Reference#SkMatrix'>const</a>
</pre>

Returns true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>equals</a> <a href='#SkMatrix_cheapEqualTo_m'>m</a>, <a href='#SkMatrix_cheapEqualTo_m'>using</a> <a href='#SkMatrix_cheapEqualTo_m'>an</a> <a href='#SkMatrix_cheapEqualTo_m'>efficient</a> <a href='#SkMatrix_cheapEqualTo_m'>comparison</a>.

Returns false when the sign of zero values is the different; when one
<a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>has</a> <a href='SkMatrix_Reference#Matrix'>positive</a> <a href='SkMatrix_Reference#Matrix'>zero</a> <a href='SkMatrix_Reference#Matrix'>value</a> <a href='SkMatrix_Reference#Matrix'>and</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>other</a> <a href='SkMatrix_Reference#Matrix'>has</a> <a href='SkMatrix_Reference#Matrix'>negative</a> <a href='SkMatrix_Reference#Matrix'>zero</a> <a href='SkMatrix_Reference#Matrix'>value</a>.

Returns true even when both <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>contain</a> <a href='SkMatrix_Reference#SkMatrix'>NaN</a>.

NaN never equals any value, including itself. To improve performance, NaN values
are treated as bit patterns that are equal if their bit patterns are equal.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_cheapEqualTo_m'><code><strong>m</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>compare</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkMatrix_cheapEqualTo_m'>m</a> <a href='#SkMatrix_cheapEqualTo_m'>and</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>are</a> <a href='SkMatrix_Reference#SkMatrix'>represented</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='SkMatrix_Reference#SkMatrix'>identical</a> <a href='SkMatrix_Reference#SkMatrix'>bit</a> <a href='SkMatrix_Reference#SkMatrix'>patterns</a>

### Example

<div><fiddle-embed name="39016b3cfc6bbabb09348a53822ce508">

#### Example Output

~~~~
identity: a == b a.cheapEqualTo(b): true
neg zero: a == b a.cheapEqualTo(b): false
one NaN: a != b a.cheapEqualTo(b): false
both NaN: a != b a.cheapEqualTo(b): true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_equal_operator'>operator==(const SkMatrix& a, const SkMatrix& b)</a>

<a name='SkMatrix_equal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator==(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#SkMatrix'>a</a>, <a href='SkMatrix_Reference#SkMatrix'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#SkMatrix'>b</a>)
</pre>

Compares <a href='#SkMatrix_equal_operator_a'>a</a> <a href='#SkMatrix_equal_operator_a'>and</a> <a href='#SkMatrix_equal_operator_b'>b</a>; <a href='#SkMatrix_equal_operator_b'>returns</a> <a href='#SkMatrix_equal_operator_b'>true</a> <a href='#SkMatrix_equal_operator_b'>if</a> <a href='#SkMatrix_equal_operator_a'>a</a> <a href='#SkMatrix_equal_operator_a'>and</a> <a href='#SkMatrix_equal_operator_b'>b</a> <a href='#SkMatrix_equal_operator_b'>are</a> <a href='#SkMatrix_equal_operator_b'>numerically</a> <a href='#SkMatrix_equal_operator_b'>equal</a>. <a href='#SkMatrix_equal_operator_b'>Returns</a> <a href='#SkMatrix_equal_operator_b'>true</a>
even if sign of zero values are different. Returns false if either <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
contains NaN, even if the other <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>also</a> <a href='SkMatrix_Reference#SkMatrix'>contains</a> <a href='SkMatrix_Reference#SkMatrix'>NaN</a>.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_equal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>compare</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_equal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>compare</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_equal_operator_a'>a</a> <a href='#SkMatrix_equal_operator_a'>and</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_equal_operator_b'>b</a> <a href='#SkMatrix_equal_operator_b'>are</a> <a href='#SkMatrix_equal_operator_b'>numerically</a> <a href='#SkMatrix_equal_operator_b'>equal</a>

### Example

<div><fiddle-embed name="3902859150b0f0c4aeb1f25d00434baa">

#### Example Output

~~~~
identity: a == b a.cheapEqualTo(b): true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_cheapEqualTo'>cheapEqualTo</a> cheapEqualTo<a href='#SkMatrix_notequal_operator'>operator!=(const SkMatrix& a, const SkMatrix& b)</a>

<a name='SkMatrix_notequal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool operator!=(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#SkMatrix'>a</a>, <a href='SkMatrix_Reference#SkMatrix'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#SkMatrix'>b</a>)
</pre>

Compares <a href='#SkMatrix_notequal_operator_a'>a</a> <a href='#SkMatrix_notequal_operator_a'>and</a> <a href='#SkMatrix_notequal_operator_b'>b</a>; <a href='#SkMatrix_notequal_operator_b'>returns</a> <a href='#SkMatrix_notequal_operator_b'>true</a> <a href='#SkMatrix_notequal_operator_b'>if</a> <a href='#SkMatrix_notequal_operator_a'>a</a> <a href='#SkMatrix_notequal_operator_a'>and</a> <a href='#SkMatrix_notequal_operator_b'>b</a> <a href='#SkMatrix_notequal_operator_b'>are</a> <a href='#SkMatrix_notequal_operator_b'>not</a> <a href='#SkMatrix_notequal_operator_b'>numerically</a> <a href='#SkMatrix_notequal_operator_b'>equal</a>. <a href='#SkMatrix_notequal_operator_b'>Returns</a> <a href='#SkMatrix_notequal_operator_b'>false</a>
even if sign of zero values are different. Returns true if either <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
contains NaN, even if the other <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>also</a> <a href='SkMatrix_Reference#SkMatrix'>contains</a> <a href='SkMatrix_Reference#SkMatrix'>NaN</a>.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_notequal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>compare</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_notequal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>compare</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_notequal_operator_a'>a</a> <a href='#SkMatrix_notequal_operator_a'>and</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_notequal_operator_b'>b</a> <a href='#SkMatrix_notequal_operator_b'>are</a> <a href='#SkMatrix_notequal_operator_b'>numerically</a> <a href='#SkMatrix_notequal_operator_b'>not</a> <a href='#SkMatrix_notequal_operator_b'>equal</a>

### Example

<div><fiddle-embed name="088ab41f877599f980a99523749b0afd"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_cheapEqualTo'>cheapEqualTo</a> cheapEqualTo<a href='#SkMatrix_equal_operator'>operator==(const SkMatrix& a, const SkMatrix& b)</a>

<a name='Utility'></a>

<a name='SkMatrix_dump'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_dump'>dump()</a> <a href='#SkMatrix_dump'>const</a>
</pre>

Writes <a href='undocumented#Text'>text</a> <a href='undocumented#Text'>representation</a> <a href='undocumented#Text'>of</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a> <a href='SkMatrix_Reference#SkMatrix'>standard</a> <a href='SkMatrix_Reference#SkMatrix'>output</a>. <a href='SkMatrix_Reference#SkMatrix'>Floating</a> <a href='SkPoint_Reference#Point'>point</a> <a href='SkPoint_Reference#Point'>values</a>
are written with limited precision; it may not be possible to reconstruct
original <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>from</a> <a href='SkMatrix_Reference#SkMatrix'>output</a>.

### Example

<div><fiddle-embed name="8d72a4818e5a9188348f6c08ab5d8a40">

#### Example Output

~~~~
[  0.7071  -0.7071   0.0000][  0.7071   0.7071   0.0000][  0.0000   0.0000   1.0000]
[  0.7071  -0.7071   0.0000][  0.7071   0.7071   0.0000][  0.0000   0.0000   1.0000]
matrix != nearlyEqual
~~~~

</fiddle-embed></div>

### See Also

<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_dump'>dump</a>

<a name='SkMatrix_getMinScale'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getMinScale'>getMinScale</a>() <a href='#SkMatrix_getMinScale'>const</a>
</pre>

Returns the minimum scaling factor of <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='SkMatrix_Reference#SkMatrix'>decomposing</a> <a href='SkMatrix_Reference#SkMatrix'>the</a> <a href='SkMatrix_Reference#SkMatrix'>scaling</a> <a href='SkMatrix_Reference#SkMatrix'>and</a>
skewing elements.
Returns -1 if scale factor overflows or <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>contains</a> <a href='SkMatrix_Reference#SkMatrix'>perspective</a>.

### Return Value

minimum scale factor

### Example

<div><fiddle-embed name="1d6f67904c88a806c3731879e9af4ae5">

#### Example Output

~~~~
matrix.getMinScale() 24
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_getMaxScale'>getMaxScale</a> <a href='#SkMatrix_getMinMaxScales'>getMinMaxScales</a>

<a name='SkMatrix_getMaxScale'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getMaxScale'>getMaxScale</a>() <a href='#SkMatrix_getMaxScale'>const</a>
</pre>

Returns the maximum scaling factor of <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> <a href='SkMatrix_Reference#SkMatrix'>decomposing</a> <a href='SkMatrix_Reference#SkMatrix'>the</a> <a href='SkMatrix_Reference#SkMatrix'>scaling</a> <a href='SkMatrix_Reference#SkMatrix'>and</a>
skewing elements.
Returns -1 if scale factor overflows or <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>contains</a> <a href='SkMatrix_Reference#SkMatrix'>perspective</a>.

### Return Value

maximum scale factor

### Example

<div><fiddle-embed name="3fee4364929899649cf9efc37897e964">

#### Example Output

~~~~
matrix.getMaxScale() 42
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_getMinScale'>getMinScale</a> <a href='#SkMatrix_getMinMaxScales'>getMinMaxScales</a>

<a name='SkMatrix_getMinMaxScales'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_getMinMaxScales'>getMinMaxScales</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>scaleFactors</a>[2]) <a href='undocumented#SkScalar'>const</a>
</pre>

Sets <a href='#SkMatrix_getMinMaxScales_scaleFactors'>scaleFactors</a>[0] <a href='#SkMatrix_getMinMaxScales_scaleFactors'>to</a> <a href='#SkMatrix_getMinMaxScales_scaleFactors'>the</a> <a href='#SkMatrix_getMinMaxScales_scaleFactors'>minimum</a> <a href='#SkMatrix_getMinMaxScales_scaleFactors'>scaling</a> <a href='#SkMatrix_getMinMaxScales_scaleFactors'>factor</a>, <a href='#SkMatrix_getMinMaxScales_scaleFactors'>and</a> <a href='#SkMatrix_getMinMaxScales_scaleFactors'>scaleFactors</a>[1] <a href='#SkMatrix_getMinMaxScales_scaleFactors'>to</a> <a href='#SkMatrix_getMinMaxScales_scaleFactors'>the</a>
maximum scaling factor. Scaling factors are computed by decomposing
the <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>scaling</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>skewing</a> <a href='SkMatrix_Reference#SkMatrix'>elements</a>.

Returns true if <a href='#SkMatrix_getMinMaxScales_scaleFactors'>scaleFactors</a> <a href='#SkMatrix_getMinMaxScales_scaleFactors'>are</a> <a href='#SkMatrix_getMinMaxScales_scaleFactors'>found</a>; <a href='#SkMatrix_getMinMaxScales_scaleFactors'>otherwise</a>, <a href='#SkMatrix_getMinMaxScales_scaleFactors'>returns</a> <a href='#SkMatrix_getMinMaxScales_scaleFactors'>false</a> <a href='#SkMatrix_getMinMaxScales_scaleFactors'>and</a> <a href='#SkMatrix_getMinMaxScales_scaleFactors'>sets</a>
<a href='#SkMatrix_getMinMaxScales_scaleFactors'>scaleFactors</a> <a href='#SkMatrix_getMinMaxScales_scaleFactors'>to</a> <a href='#SkMatrix_getMinMaxScales_scaleFactors'>undefined</a> <a href='#SkMatrix_getMinMaxScales_scaleFactors'>values</a>.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_getMinMaxScales_scaleFactors'><code><strong>scaleFactors</strong></code></a></td>
    <td>storage for minimum and maximum scale factors</td>
  </tr>
</table>

### Return Value

true if scale factors were computed correctly

### Example

<div><fiddle-embed name="13adba0ecf5f82247cf051b4fa4d8a9c">

#### Example Output

~~~~
matrix.getMinMaxScales() false 2 2
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_getMinScale'>getMinScale</a> <a href='#SkMatrix_getMaxScale'>getMaxScale</a>

<a name='SkMatrix_decomposeScale'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_decomposeScale'>decomposeScale</a>(<a href='undocumented#SkSize'>SkSize</a>* <a href='undocumented#SkSize'>scale</a>, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* <a href='SkMatrix_Reference#SkMatrix'>remaining</a> = <a href='SkMatrix_Reference#SkMatrix'>nullptr</a>) <a href='SkMatrix_Reference#SkMatrix'>const</a>
</pre>

Decomposes <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>into</a> <a href='#SkMatrix_decomposeScale_scale'>scale</a> <a href='#SkMatrix_decomposeScale_scale'>components</a> <a href='#SkMatrix_decomposeScale_scale'>and</a> <a href='#SkMatrix_decomposeScale_scale'>whatever</a> <a href='#SkMatrix_decomposeScale_scale'>remains</a>. <a href='#SkMatrix_decomposeScale_scale'>Returns</a> <a href='#SkMatrix_decomposeScale_scale'>false</a> <a href='#SkMatrix_decomposeScale_scale'>if</a>
<a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>could</a> <a href='SkMatrix_Reference#Matrix'>not</a> <a href='SkMatrix_Reference#Matrix'>be</a> <a href='SkMatrix_Reference#Matrix'>decomposed</a>.

<a href='SkMatrix_Reference#Matrix'>Sets</a> <a href='#SkMatrix_decomposeScale_scale'>scale</a> <a href='#SkMatrix_decomposeScale_scale'>to</a> <a href='#SkMatrix_decomposeScale_scale'>portion</a> <a href='#SkMatrix_decomposeScale_scale'>of</a> <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>that</a> <a href='#SkMatrix_decomposeScale_scale'>scale</a> <a href='#SkMatrix_decomposeScale_scale'>axes</a>. <a href='#SkMatrix_decomposeScale_scale'>Sets</a> <a href='#SkMatrix_decomposeScale_remaining'>remaining</a> <a href='#SkMatrix_decomposeScale_remaining'>to</a> <a href='SkMatrix_Reference#Matrix'>Matrix</a>
<a href='SkMatrix_Reference#Matrix'>with</a> <a href='SkMatrix_Reference#Matrix'>scaling</a> <a href='SkMatrix_Reference#Matrix'>factored</a> <a href='SkMatrix_Reference#Matrix'>out</a>. <a href='#SkMatrix_decomposeScale_remaining'>remaining</a> <a href='#SkMatrix_decomposeScale_remaining'>may</a> <a href='#SkMatrix_decomposeScale_remaining'>be</a> <a href='#SkMatrix_decomposeScale_remaining'>passed</a> <a href='#SkMatrix_decomposeScale_remaining'>as</a> <a href='#SkMatrix_decomposeScale_remaining'>nullptr</a>
<a href='#SkMatrix_decomposeScale_remaining'>to</a> <a href='#SkMatrix_decomposeScale_remaining'>determine</a> <a href='#SkMatrix_decomposeScale_remaining'>if</a> <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>can</a> <a href='SkMatrix_Reference#Matrix'>be</a> <a href='SkMatrix_Reference#Matrix'>decomposed</a> <a href='SkMatrix_Reference#Matrix'>without</a> <a href='SkMatrix_Reference#Matrix'>computing</a> <a href='SkMatrix_Reference#Matrix'>remainder</a>.

<a href='SkMatrix_Reference#Matrix'>Returns</a> <a href='SkMatrix_Reference#Matrix'>true</a> <a href='SkMatrix_Reference#Matrix'>if</a> <a href='#SkMatrix_decomposeScale_scale'>scale</a> <a href='#SkMatrix_decomposeScale_scale'>components</a> <a href='#SkMatrix_decomposeScale_scale'>are</a> <a href='#SkMatrix_decomposeScale_scale'>found</a>. <a href='#SkMatrix_decomposeScale_scale'>scale</a> <a href='#SkMatrix_decomposeScale_scale'>and</a> <a href='#SkMatrix_decomposeScale_remaining'>remaining</a> <a href='#SkMatrix_decomposeScale_remaining'>are</a>
<a href='#SkMatrix_decomposeScale_remaining'>unchanged</a> <a href='#SkMatrix_decomposeScale_remaining'>if</a> <a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>contains</a> <a href='SkMatrix_Reference#Matrix'>perspective</a>; <a href='#SkMatrix_decomposeScale_scale'>scale</a> <a href='#SkMatrix_decomposeScale_scale'>factors</a> <a href='#SkMatrix_decomposeScale_scale'>are</a> <a href='#SkMatrix_decomposeScale_scale'>not</a> <a href='#SkMatrix_decomposeScale_scale'>finite</a>, <a href='#SkMatrix_decomposeScale_scale'>or</a>
<a href='#SkMatrix_decomposeScale_scale'>are</a> <a href='#SkMatrix_decomposeScale_scale'>nearly</a> <a href='#SkMatrix_decomposeScale_scale'>zero</a>.

<a href='#SkMatrix_decomposeScale_scale'>On</a> <a href='#SkMatrix_decomposeScale_scale'>success</a>: <code><a href='SkMatrix_Reference#Matrix'>Matrix</a> = <a href='#SkMatrix_decomposeScale_scale'>scale</a> * <a href='#SkMatrix_decomposeScale_scale'>Remaining</a></code>.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_decomposeScale_scale'><code><strong>scale</strong></code></a></td>
    <td>axes scaling factors; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkMatrix_decomposeScale_remaining'><code><strong>remaining</strong></code></a></td>
    <td><a href='SkMatrix_Reference#Matrix'>Matrix</a> <a href='SkMatrix_Reference#Matrix'>without</a> <a href='SkMatrix_Reference#Matrix'>scaling</a>; <a href='SkMatrix_Reference#Matrix'>may</a> <a href='SkMatrix_Reference#Matrix'>be</a> <a href='SkMatrix_Reference#Matrix'>nullptr</a></td>
  </tr>
</table>

### Return Value

true if <a href='#SkMatrix_decomposeScale_scale'>scale</a> <a href='#SkMatrix_decomposeScale_scale'>can</a> <a href='#SkMatrix_decomposeScale_scale'>be</a> <a href='#SkMatrix_decomposeScale_scale'>computed</a>

### Example

<div><fiddle-embed name="139b874da0a3ede1f3df88119085c0aa">

#### Example Output

~~~~
[  0.0000  -0.2500   0.0000][  0.5000   0.0000   0.0000][  0.0000   0.0000   1.0000]
success: true  scale: 0.5, 0.25
[  0.0000  -0.5000   0.0000][  2.0000   0.0000   0.0000][  0.0000   0.0000   1.0000]
[  0.0000  -0.2500   0.0000][  0.5000   0.0000   0.0000][  0.0000   0.0000   1.0000]
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_setScale'>setScale</a> <a href='#SkMatrix_MakeScale'>MakeScale</a>

<a name='SkMatrix_I'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='#SkMatrix_I'>I</a>()
</pre>

Returns reference to const identity <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. <a href='SkMatrix_Reference#SkMatrix'>Returned</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>is</a> <a href='SkMatrix_Reference#SkMatrix'>set</a> <a href='SkMatrix_Reference#SkMatrix'>to</a>:

| 1 0 0 |
| 0 1 0 |
| 0 0 1 |

### Return Value

const identity <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>

### Example

<div><fiddle-embed name="d961d91020f19037204a8c3fd8cb1060">

#### Example Output

~~~~
m1 == m2
m2 == m3
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_reset'>reset()</a> <a href='#SkMatrix_setIdentity'>setIdentity</a>

<a name='SkMatrix_InvalidMatrix'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='#SkMatrix_InvalidMatrix'>InvalidMatrix</a>()
</pre>

Returns reference to a const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>with</a> <a href='SkMatrix_Reference#SkMatrix'>invalid</a> <a href='SkMatrix_Reference#SkMatrix'>values</a>. <a href='SkMatrix_Reference#SkMatrix'>Returned</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>is</a> <a href='SkMatrix_Reference#SkMatrix'>set</a>
to:

| <a href='undocumented#SK_ScalarMax'>SK_ScalarMax</a> <a href='undocumented#SK_ScalarMax'>SK_ScalarMax</a> <a href='undocumented#SK_ScalarMax'>SK_ScalarMax</a> |
| <a href='undocumented#SK_ScalarMax'>SK_ScalarMax</a> <a href='undocumented#SK_ScalarMax'>SK_ScalarMax</a> <a href='undocumented#SK_ScalarMax'>SK_ScalarMax</a> |
| <a href='undocumented#SK_ScalarMax'>SK_ScalarMax</a> <a href='undocumented#SK_ScalarMax'>SK_ScalarMax</a> <a href='undocumented#SK_ScalarMax'>SK_ScalarMax</a> |

### Return Value

const invalid <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>

### Example

<div><fiddle-embed name="af0b72360c1c7a25b4754bfa47011dd5">

#### Example Output

~~~~
scaleX 3.40282e+38
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_getType'>getType</a>

<a name='SkMatrix_Concat'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_Concat'>Concat</a>(<a href='#SkMatrix_Concat'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#SkMatrix'>a</a>, <a href='SkMatrix_Reference#SkMatrix'>const</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='SkMatrix_Reference#SkMatrix'>b</a>)
</pre>

Returns <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_Concat_a'>a</a> <a href='#SkMatrix_Concat_a'>multiplied</a> <a href='#SkMatrix_Concat_a'>by</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_Concat_b'>b</a>.

Given:

| A B C |      | J K L |
<a href='#SkMatrix_Concat_a'>a</a> = | <a href='#SkMatrix_Concat_a'>D</a> <a href='#SkMatrix_Concat_a'>E</a> <a href='#SkMatrix_Concat_a'>F</a> |, <a href='#SkMatrix_Concat_b'>b</a> = | <a href='#SkMatrix_Concat_b'>M</a> <a href='#SkMatrix_Concat_b'>N</a> <a href='#SkMatrix_Concat_b'>O</a> |
| G H <a href='#SkMatrix_I'>I</a> |      | <a href='#SkMatrix_I'>P</a> <a href='#SkMatrix_I'>Q</a> <a href='#SkMatrix_I'>R</a> |

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>to</a>:

| A B C |   | J K L |   | AJ+BM+CP AK+BN+CQ AL+BO+CR |
<a href='#SkMatrix_Concat_a'>a</a> * <a href='#SkMatrix_Concat_b'>b</a> = | <a href='#SkMatrix_Concat_b'>D</a> <a href='#SkMatrix_Concat_b'>E</a> <a href='#SkMatrix_Concat_b'>F</a> | * | <a href='#SkMatrix_Concat_b'>M</a> <a href='#SkMatrix_Concat_b'>N</a> <a href='#SkMatrix_Concat_b'>O</a> | = | <a href='#SkMatrix_Concat_b'>DJ</a>+<a href='#SkMatrix_Concat_b'>EM</a>+<a href='#SkMatrix_Concat_b'>FP</a> <a href='#SkMatrix_Concat_b'>DK</a>+<a href='#SkMatrix_Concat_b'>EN</a>+<a href='#SkMatrix_Concat_b'>FQ</a> <a href='#SkMatrix_Concat_b'>DL</a>+<a href='#SkMatrix_Concat_b'>EO</a>+<a href='#SkMatrix_Concat_b'>FR</a> |
| G H <a href='#SkMatrix_I'>I</a> |   | <a href='#SkMatrix_I'>P</a> <a href='#SkMatrix_I'>Q</a> <a href='#SkMatrix_I'>R</a> |   | <a href='#SkMatrix_I'>GJ</a>+<a href='#SkMatrix_I'>HM</a>+<a href='#SkMatrix_I'>IP</a> <a href='#SkMatrix_I'>GK</a>+<a href='#SkMatrix_I'>HN</a>+<a href='#SkMatrix_I'>IQ</a> <a href='#SkMatrix_I'>GL</a>+<a href='#SkMatrix_I'>HO</a>+<a href='#SkMatrix_I'>IR</a> |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_Concat_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>on</a>  <a href='SkMatrix_Reference#SkMatrix'>left side</a> <a href='SkMatrix_Reference#SkMatrix'>of</a> <a href='SkMatrix_Reference#SkMatrix'>multiply</a> <a href='SkMatrix_Reference#SkMatrix'>expression</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_Concat_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>on</a>  <a href='SkMatrix_Reference#SkMatrix'>right side</a> <a href='SkMatrix_Reference#SkMatrix'>of</a> <a href='SkMatrix_Reference#SkMatrix'>multiply</a> <a href='SkMatrix_Reference#SkMatrix'>expression</a></td>
  </tr>
</table>

### Return Value

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>computed</a> <a href='SkMatrix_Reference#SkMatrix'>from</a> <a href='#SkMatrix_Concat_a'>a</a> <a href='#SkMatrix_Concat_a'>times</a> <a href='#SkMatrix_Concat_b'>b</a>

### Example

<div><fiddle-embed name="6b4562c7052da94f3d5b2412dca41946"><div><a href='#SkMatrix_setPolyToPoly'>setPolyToPoly</a> <a href='#SkMatrix_setPolyToPoly'>creates</a> <a href='#SkMatrix_setPolyToPoly'>perspective</a> <a href='SkMatrix_Reference#Matrix'>matrices</a>, <a href='SkMatrix_Reference#Matrix'>one</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>inverse</a> <a href='SkMatrix_Reference#Matrix'>of</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>other</a>.
<a href='SkMatrix_Reference#Matrix'>Multiplying</a> <a href='SkMatrix_Reference#Matrix'>the</a> <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>by</a> <a href='SkMatrix_Reference#Matrix'>its</a> <a href='SkMatrix_Reference#Matrix'>inverse</a> <a href='SkMatrix_Reference#Matrix'>turns</a> <a href='SkMatrix_Reference#Matrix'>into</a> <a href='SkMatrix_Reference#Matrix'>an</a> <a href='SkMatrix_Reference#Matrix'>identity</a> <a href='SkMatrix_Reference#Matrix'>matrix</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkMatrix_preConcat'>preConcat</a> <a href='#SkMatrix_postConcat'>postConcat</a>

<a name='SkMatrix_dirtyMatrixTypeCache'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_dirtyMatrixTypeCache'>dirtyMatrixTypeCache</a>()
</pre>

Sets internal cache to unknown state. Use to force update after repeated
modifications to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>element</a> <a href='SkMatrix_Reference#SkMatrix'>reference</a> <a href='SkMatrix_Reference#SkMatrix'>returned</a> <a href='SkMatrix_Reference#SkMatrix'>by</a> by<a href='#SkMatrix_array1_operator'>operator[](int index)</a>.

### Example

<div><fiddle-embed name="f4365ef332f51f7fd25040e0771ba9a2">

#### Example Output

~~~~
with identity matrix: x = 24
after skew x mod:     x = 24
after 2nd skew x mod: x = 24
after dirty cache:    x = 66
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_array1_operator'>operator[](int index)</a> <a href='#SkMatrix_getType'>getType</a>

<a name='SkMatrix_setScaleTranslate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setScaleTranslate'>setScaleTranslate</a>(<a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>sy</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>tx</a>, <a href='undocumented#SkScalar'>SkScalar</a> <a href='undocumented#SkScalar'>ty</a>)
</pre>

Initializes <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='SkMatrix_Reference#SkMatrix'>with</a> <a href='SkMatrix_Reference#SkMatrix'>scale</a> <a href='SkMatrix_Reference#SkMatrix'>and</a> <a href='SkMatrix_Reference#SkMatrix'>translate</a> <a href='SkMatrix_Reference#SkMatrix'>elements</a>.

| <a href='#SkMatrix_setScaleTranslate_sx'>sx</a>  0 <a href='#SkMatrix_setScaleTranslate_tx'>tx</a> |
|  0 <a href='#SkMatrix_setScaleTranslate_sy'>sy</a> <a href='#SkMatrix_setScaleTranslate_ty'>ty</a> |
|  0  0  1 |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setScaleTranslate_sx'><code><strong>sx</strong></code></a></td>
    <td>horizontal scale factor to store</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setScaleTranslate_sy'><code><strong>sy</strong></code></a></td>
    <td>vertical scale factor to store</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setScaleTranslate_tx'><code><strong>tx</strong></code></a></td>
    <td>horizontal translation to store</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setScaleTranslate_ty'><code><strong>ty</strong></code></a></td>
    <td>vertical translation to store</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="fed43797f13796529cb6731385d6f8f3">

#### Example Output

~~~~
[  1.0000   0.0000   3.0000][  0.0000   2.0000   4.0000][  0.0000   0.0000   1.0000]
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_setScale'>setScale</a> <a href='#SkMatrix_preTranslate'>preTranslate</a> <a href='#SkMatrix_postTranslate'>postTranslate</a>

<a name='SkMatrix_isFinite'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_isFinite'>isFinite</a>() <a href='#SkMatrix_isFinite'>const</a>
</pre>

Returns true if all elements of the <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>are</a> <a href='SkMatrix_Reference#Matrix'>finite</a>. <a href='SkMatrix_Reference#Matrix'>Returns</a> <a href='SkMatrix_Reference#Matrix'>false</a> <a href='SkMatrix_Reference#Matrix'>if</a> <a href='SkMatrix_Reference#Matrix'>any</a>
element is infinity, or NaN.

### Return Value

true if <a href='SkMatrix_Reference#Matrix'>matrix</a> <a href='SkMatrix_Reference#Matrix'>has</a> <a href='SkMatrix_Reference#Matrix'>only</a> <a href='SkMatrix_Reference#Matrix'>finite</a> <a href='SkMatrix_Reference#Matrix'>elements</a>

### Example

<div><fiddle-embed name="bc6c6f6a5df770287120d87f81b922eb">

#### Example Output

~~~~
[  1.0000   0.0000      nan][  0.0000   1.0000   0.0000][  0.0000   0.0000   1.0000]
matrix is finite: false
matrix != matrix
~~~~

</fiddle-embed></div>

### See Also

operator==

