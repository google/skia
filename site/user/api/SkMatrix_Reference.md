SkMatrix Reference
===


<a name='SkMatrix'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> {

    static <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_MakeScale'>MakeScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> sx, <a href='undocumented#SkScalar'>SkScalar</a> sy);
    static <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_MakeScale'>MakeScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> scale);
    static <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_MakeTrans'>MakeTrans</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy);
    static <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_MakeAll'>MakeAll</a>(<a href='undocumented#SkScalar'>SkScalar</a> scaleX, <a href='undocumented#SkScalar'>SkScalar</a> skewX, <a href='undocumented#SkScalar'>SkScalar</a> transX,
                            <a href='undocumented#SkScalar'>SkScalar</a> skewY, <a href='undocumented#SkScalar'>SkScalar</a> scaleY, <a href='undocumented#SkScalar'>SkScalar</a> transY,
                            <a href='undocumented#SkScalar'>SkScalar</a> pers0, <a href='undocumented#SkScalar'>SkScalar</a> pers1, <a href='undocumented#SkScalar'>SkScalar</a> pers2);

    enum <a href='#SkMatrix_TypeMask'>TypeMask</a> {
        <a href='#SkMatrix_kIdentity_Mask'>kIdentity_Mask</a> = 0,
        <a href='#SkMatrix_kTranslate_Mask'>kTranslate_Mask</a> = 0x01,
        <a href='#SkMatrix_kScale_Mask'>kScale_Mask</a> = 0x02,
        <a href='#SkMatrix_kAffine_Mask'>kAffine_Mask</a> = 0x04,
        <a href='#SkMatrix_kPerspective_Mask'>kPerspective_Mask</a> = 0x08,
    };

    <a href='#SkMatrix_TypeMask'>TypeMask</a> <a href='#SkMatrix_getType'>getType</a>() const;
    bool <a href='#SkMatrix_isIdentity'>isIdentity</a>() const;
    bool <a href='#SkMatrix_isScaleTranslate'>isScaleTranslate</a>() const;
    bool <a href='#SkMatrix_isTranslate'>isTranslate</a>() const;
    bool <a href='#SkMatrix_rectStaysRect'>rectStaysRect</a>() const;
    bool <a href='#SkMatrix_preservesAxisAlignment'>preservesAxisAlignment</a>() const;
    bool <a href='#SkMatrix_hasPerspective'>hasPerspective</a>() const;
    bool <a href='#SkMatrix_isSimilarity'>isSimilarity</a>(<a href='undocumented#SkScalar'>SkScalar</a> tol = <a href='undocumented#SK_ScalarNearlyZero'>SK_ScalarNearlyZero</a>) const;
    bool <a href='#SkMatrix_preservesRightAngles'>preservesRightAngles</a>(<a href='undocumented#SkScalar'>SkScalar</a> tol = <a href='undocumented#SK_ScalarNearlyZero'>SK_ScalarNearlyZero</a>) const;

    static constexpr int <a href='#SkMatrix_kMScaleX'>kMScaleX</a> = 0    static constexpr int <a href='#SkMatrix_kMSkewX'>kMSkewX</a> = 1    static constexpr int <a href='#SkMatrix_kMTransX'>kMTransX</a> = 2    static constexpr int <a href='#SkMatrix_kMSkewY'>kMSkewY</a> = 3    static constexpr int <a href='#SkMatrix_kMScaleY'>kMScaleY</a> = 4    static constexpr int <a href='#SkMatrix_kMTransY'>kMTransY</a> = 5    static constexpr int <a href='#SkMatrix_kMPersp0'>kMPersp0</a> = 6    static constexpr int <a href='#SkMatrix_kMPersp1'>kMPersp1</a> = 7    static constexpr int <a href='#SkMatrix_kMPersp2'>kMPersp2</a> = 8    static constexpr int <a href='#SkMatrix_kAScaleX'>kAScaleX</a> = 0    static constexpr int <a href='#SkMatrix_kASkewY'>kASkewY</a> = 1    static constexpr int <a href='#SkMatrix_kASkewX'>kASkewX</a> = 2    static constexpr int <a href='#SkMatrix_kAScaleY'>kAScaleY</a> = 3    static constexpr int <a href='#SkMatrix_kATransX'>kATransX</a> = 4    static constexpr int <a href='#SkMatrix_kATransY'>kATransY</a> = 5
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_array_operator'>operator[]</a>(int index) const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_get'>get</a>(int index) const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getScaleX'>getScaleX</a>() const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getScaleY'>getScaleY</a>() const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getSkewY'>getSkewY</a>() const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getSkewX'>getSkewX</a>() const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getTranslateX'>getTranslateX</a>() const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getTranslateY'>getTranslateY</a>() const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getPerspX'>getPerspX</a>() const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getPerspY'>getPerspY</a>() const;
    <a href='undocumented#SkScalar'>SkScalar</a>& <a href='#SkMatrix_array1_operator'>operator[]</a>(int index);
    void <a href='#SkMatrix_set'>set</a>(int index, <a href='undocumented#SkScalar'>SkScalar</a> value);
    void <a href='#SkMatrix_setScaleX'>setScaleX</a>(<a href='undocumented#SkScalar'>SkScalar</a> v);
    void <a href='#SkMatrix_setScaleY'>setScaleY</a>(<a href='undocumented#SkScalar'>SkScalar</a> v);
    void <a href='#SkMatrix_setSkewY'>setSkewY</a>(<a href='undocumented#SkScalar'>SkScalar</a> v);
    void <a href='#SkMatrix_setSkewX'>setSkewX</a>(<a href='undocumented#SkScalar'>SkScalar</a> v);
    void <a href='#SkMatrix_setTranslateX'>setTranslateX</a>(<a href='undocumented#SkScalar'>SkScalar</a> v);
    void <a href='#SkMatrix_setTranslateY'>setTranslateY</a>(<a href='undocumented#SkScalar'>SkScalar</a> v);
    void <a href='#SkMatrix_setPerspX'>setPerspX</a>(<a href='undocumented#SkScalar'>SkScalar</a> v);
    void <a href='#SkMatrix_setPerspY'>setPerspY</a>(<a href='undocumented#SkScalar'>SkScalar</a> v);
    void <a href='#SkMatrix_setAll'>setAll</a>(<a href='undocumented#SkScalar'>SkScalar</a> scaleX, <a href='undocumented#SkScalar'>SkScalar</a> skewX, <a href='undocumented#SkScalar'>SkScalar</a> transX,
                <a href='undocumented#SkScalar'>SkScalar</a> skewY, <a href='undocumented#SkScalar'>SkScalar</a> scaleY, <a href='undocumented#SkScalar'>SkScalar</a> transY,
                <a href='undocumented#SkScalar'>SkScalar</a> persp0, <a href='undocumented#SkScalar'>SkScalar</a> persp1, <a href='undocumented#SkScalar'>SkScalar</a> persp2);
    void <a href='#SkMatrix_get9'>get9</a>(<a href='undocumented#SkScalar'>SkScalar</a> buffer[9]) const;
    void <a href='#SkMatrix_set9'>set9</a>(const <a href='undocumented#SkScalar'>SkScalar</a> buffer[9]);
    void <a href='#SkMatrix_reset'>reset()</a>;
    void <a href='#SkMatrix_setIdentity'>setIdentity</a>();
    void <a href='#SkMatrix_setTranslate'>setTranslate</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy);
    void <a href='#SkMatrix_setTranslate'>setTranslate</a>(const <a href='SkPoint_Reference#SkVector'>SkVector</a>& v);
    void <a href='#SkMatrix_setScale'>setScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> sx, <a href='undocumented#SkScalar'>SkScalar</a> sy, <a href='undocumented#SkScalar'>SkScalar</a> px, <a href='undocumented#SkScalar'>SkScalar</a> py);
    void <a href='#SkMatrix_setScale'>setScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> sx, <a href='undocumented#SkScalar'>SkScalar</a> sy);
    void <a href='#SkMatrix_setRotate'>setRotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> degrees, <a href='undocumented#SkScalar'>SkScalar</a> px, <a href='undocumented#SkScalar'>SkScalar</a> py);
    void <a href='#SkMatrix_setRotate'>setRotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> degrees);
    void <a href='#SkMatrix_setSinCos'>setSinCos</a>(<a href='undocumented#SkScalar'>SkScalar</a> sinValue, <a href='undocumented#SkScalar'>SkScalar</a> cosValue,
                   <a href='undocumented#SkScalar'>SkScalar</a> px, <a href='undocumented#SkScalar'>SkScalar</a> py);
    void <a href='#SkMatrix_setSinCos'>setSinCos</a>(<a href='undocumented#SkScalar'>SkScalar</a> sinValue, <a href='undocumented#SkScalar'>SkScalar</a> cosValue);
    <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='#SkMatrix_setRSXform'>setRSXform</a>(const <a href='undocumented#SkRSXform'>SkRSXform</a>& rsxForm);
    void <a href='#SkMatrix_setSkew'>setSkew</a>(<a href='undocumented#SkScalar'>SkScalar</a> kx, <a href='undocumented#SkScalar'>SkScalar</a> ky, <a href='undocumented#SkScalar'>SkScalar</a> px, <a href='undocumented#SkScalar'>SkScalar</a> py);
    void <a href='#SkMatrix_setSkew'>setSkew</a>(<a href='undocumented#SkScalar'>SkScalar</a> kx, <a href='undocumented#SkScalar'>SkScalar</a> ky);
    void <a href='#SkMatrix_setConcat'>setConcat</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& a, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& b);
    void <a href='#SkMatrix_preTranslate'>preTranslate</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy);
    void <a href='#SkMatrix_preScale'>preScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> sx, <a href='undocumented#SkScalar'>SkScalar</a> sy, <a href='undocumented#SkScalar'>SkScalar</a> px, <a href='undocumented#SkScalar'>SkScalar</a> py);
    void <a href='#SkMatrix_preScale'>preScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> sx, <a href='undocumented#SkScalar'>SkScalar</a> sy);
    void <a href='#SkMatrix_preRotate'>preRotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> degrees, <a href='undocumented#SkScalar'>SkScalar</a> px, <a href='undocumented#SkScalar'>SkScalar</a> py);
    void <a href='#SkMatrix_preRotate'>preRotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> degrees);
    void <a href='#SkMatrix_preSkew'>preSkew</a>(<a href='undocumented#SkScalar'>SkScalar</a> kx, <a href='undocumented#SkScalar'>SkScalar</a> ky, <a href='undocumented#SkScalar'>SkScalar</a> px, <a href='undocumented#SkScalar'>SkScalar</a> py);
    void <a href='#SkMatrix_preSkew'>preSkew</a>(<a href='undocumented#SkScalar'>SkScalar</a> kx, <a href='undocumented#SkScalar'>SkScalar</a> ky);
    void <a href='#SkMatrix_preConcat'>preConcat</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& other);
    void <a href='#SkMatrix_postTranslate'>postTranslate</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy);
    void <a href='#SkMatrix_postScale'>postScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> sx, <a href='undocumented#SkScalar'>SkScalar</a> sy, <a href='undocumented#SkScalar'>SkScalar</a> px, <a href='undocumented#SkScalar'>SkScalar</a> py);
    void <a href='#SkMatrix_postScale'>postScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> sx, <a href='undocumented#SkScalar'>SkScalar</a> sy);
    bool <a href='#SkMatrix_postIDiv'>postIDiv</a>(int divx, int divy);
    void <a href='#SkMatrix_postRotate'>postRotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> degrees, <a href='undocumented#SkScalar'>SkScalar</a> px, <a href='undocumented#SkScalar'>SkScalar</a> py);
    void <a href='#SkMatrix_postRotate'>postRotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> degrees);
    void <a href='#SkMatrix_postSkew'>postSkew</a>(<a href='undocumented#SkScalar'>SkScalar</a> kx, <a href='undocumented#SkScalar'>SkScalar</a> ky, <a href='undocumented#SkScalar'>SkScalar</a> px, <a href='undocumented#SkScalar'>SkScalar</a> py);
    void <a href='#SkMatrix_postSkew'>postSkew</a>(<a href='undocumented#SkScalar'>SkScalar</a> kx, <a href='undocumented#SkScalar'>SkScalar</a> ky);
    void <a href='#SkMatrix_postConcat'>postConcat</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& other);

    enum <a href='#SkMatrix_ScaleToFit'>ScaleToFit</a> {
        <a href='#SkMatrix_kFill_ScaleToFit'>kFill_ScaleToFit</a>,
        <a href='#SkMatrix_kStart_ScaleToFit'>kStart_ScaleToFit</a>,
        <a href='#SkMatrix_kCenter_ScaleToFit'>kCenter_ScaleToFit</a>,
        <a href='#SkMatrix_kEnd_ScaleToFit'>kEnd_ScaleToFit</a>,
    };

    bool <a href='#SkMatrix_setRectToRect'>setRectToRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& src, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst, <a href='#SkMatrix_ScaleToFit'>ScaleToFit</a> stf);
    static <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_MakeRectToRect'>MakeRectToRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& src, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst, <a href='#SkMatrix_ScaleToFit'>ScaleToFit</a> stf);
    bool <a href='#SkMatrix_setPolyToPoly'>setPolyToPoly</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> src[], const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> dst[], int count);
    bool <a href='#SkMatrix_invert'>invert</a>(<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* inverse) const;
    static void <a href='#SkMatrix_SetAffineIdentity'>SetAffineIdentity</a>(<a href='undocumented#SkScalar'>SkScalar</a> affine[6]);
    bool <a href='#SkMatrix_asAffine'>asAffine</a>(<a href='undocumented#SkScalar'>SkScalar</a> affine[6]) const;
    void <a href='#SkMatrix_setAffine'>setAffine</a>(const <a href='undocumented#SkScalar'>SkScalar</a> affine[6]);
    void <a href='#SkMatrix_mapPoints'>mapPoints</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> dst[], const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> src[], int count) const;
    void <a href='#SkMatrix_mapPoints'>mapPoints</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> pts[], int count) const;
    void <a href='#SkMatrix_mapHomogeneousPoints'>mapHomogeneousPoints</a>(<a href='undocumented#SkPoint3'>SkPoint3</a> dst[], const <a href='undocumented#SkPoint3'>SkPoint3</a> src[], int count) const;
    void <a href='#SkMatrix_mapXY'>mapXY</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, <a href='SkPoint_Reference#SkPoint'>SkPoint</a>* result) const;
    <a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkMatrix_mapXY'>mapXY</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y) const;
    void <a href='#SkMatrix_mapVectors'>mapVectors</a>(<a href='SkPoint_Reference#SkVector'>SkVector</a> dst[], const <a href='SkPoint_Reference#SkVector'>SkVector</a> src[], int count) const;
    void <a href='#SkMatrix_mapVectors'>mapVectors</a>(<a href='SkPoint_Reference#SkVector'>SkVector</a> vecs[], int count) const;
    void <a href='#SkMatrix_mapVector'>mapVector</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy, <a href='SkPoint_Reference#SkVector'>SkVector</a>* result) const;
    <a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='#SkMatrix_mapVector'>mapVector</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy) const;
    bool <a href='#SkMatrix_mapRect'>mapRect</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* dst, const <a href='SkRect_Reference#SkRect'>SkRect</a>& src) const;
    bool <a href='#SkMatrix_mapRect'>mapRect</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#Rect'>rect</a>) const;
    <a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkMatrix_mapRect'>mapRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& src) const;
    void <a href='#SkMatrix_mapRectToQuad'>mapRectToQuad</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> dst[4], const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>) const;
    void <a href='#SkMatrix_mapRectScaleTranslate'>mapRectScaleTranslate</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* dst, const <a href='SkRect_Reference#SkRect'>SkRect</a>& src) const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_mapRadius'>mapRadius</a>(<a href='undocumented#SkScalar'>SkScalar</a> radius) const;
    bool <a href='#SkMatrix_isFixedStepInX'>isFixedStepInX</a>() const;
    <a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='#SkMatrix_fixedStepInX'>fixedStepInX</a>(<a href='undocumented#SkScalar'>SkScalar</a> y) const;
    bool <a href='#SkMatrix_cheapEqualTo'>cheapEqualTo</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& m) const;
    friend bool <a href='#SkMatrix_equal_operator'>operator==</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& a, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& b);
    friend bool <a href='#SkMatrix_notequal_operator'>operator!=</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& a, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& b);
    void <a href='#SkMatrix_dump'>dump()</a> const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getMinScale'>getMinScale</a>() const;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getMaxScale'>getMaxScale</a>() const;
    bool <a href='#SkMatrix_getMinMaxScales'>getMinMaxScales</a>(<a href='undocumented#SkScalar'>SkScalar</a> scaleFactors[2]) const;
    bool <a href='#SkMatrix_decomposeScale'>decomposeScale</a>(<a href='undocumented#SkSize'>SkSize</a>* scale, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* remaining = nullptr) const;
    static const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='#SkMatrix_I'>I</a>();
    static const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='#SkMatrix_InvalidMatrix'>InvalidMatrix</a>();
    static <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_Concat'>Concat</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& a, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& b);
    void <a href='#SkMatrix_dirtyMatrixTypeCache'>dirtyMatrixTypeCache</a>();
    void <a href='#SkMatrix_setScaleTranslate'>setScaleTranslate</a>(<a href='undocumented#SkScalar'>SkScalar</a> sx, <a href='undocumented#SkScalar'>SkScalar</a> sy, <a href='undocumented#SkScalar'>SkScalar</a> tx, <a href='undocumented#SkScalar'>SkScalar</a> ty);
    bool <a href='#SkMatrix_isFinite'>isFinite</a>() const;
};

</pre>

<a href='SkMatrix_Reference#Matrix'>Matrix</a> holds a 3 by 3 <a href='SkMatrix_Reference#Matrix'>matrix</a> for transforming coordinates. This allows mapping
<a href='SkPoint_Reference#Point'>Points</a> and <a href='SkPoint_Reference#Vector'>Vectors</a> with translation, scaling, skewing, rotation, and
perspective.

<a href='SkMatrix_Reference#Matrix'>Matrix</a> elements are in row major order. <a href='SkMatrix_Reference#Matrix'>Matrix</a> does not have a constructor,
so it must be explicitly initialized. <a href='#SkMatrix_setIdentity'>setIdentity</a> initializes <a href='SkMatrix_Reference#Matrix'>Matrix</a>
so it has no effect. <a href='#SkMatrix_setTranslate'>setTranslate</a>, <a href='#SkMatrix_setScale'>setScale</a>, <a href='#SkMatrix_setSkew'>setSkew</a>, <a href='#SkMatrix_setRotate'>setRotate</a>, <a href='#SkMatrix_set9'>set9</a> and <a href='#SkMatrix_setAll'>setAll</a>
initializes all <a href='SkMatrix_Reference#Matrix'>Matrix</a> elements with the corresponding mapping.

<a href='SkMatrix_Reference#Matrix'>Matrix</a> includes a hidden variable that classifies the type of <a href='SkMatrix_Reference#Matrix'>matrix</a> to
improve performance. <a href='SkMatrix_Reference#Matrix'>Matrix</a> is not thread safe unless <a href='#SkMatrix_getType'>getType</a> is called first.

<a name='SkMatrix_MakeScale'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_MakeScale'>MakeScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> sx, <a href='undocumented#SkScalar'>SkScalar</a> sy)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to scale by (<a href='#SkMatrix_MakeScale_sx'>sx</a>, <a href='#SkMatrix_MakeScale_sy'>sy</a>). Returned <a href='SkMatrix_Reference#Matrix'>matrix</a> is:

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

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> with scale

### Example

<div><fiddle-embed name="@Matrix_MakeScale"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setScale'>setScale</a> <a href='#SkMatrix_postScale'>postScale</a> <a href='#SkMatrix_preScale'>preScale</a>

<a name='SkMatrix_MakeScale_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_MakeScale'>MakeScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> scale)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to <a href='#SkMatrix_MakeScale_2_scale'>scale</a> by (<a href='#SkMatrix_MakeScale_2_scale'>scale</a>, <a href='#SkMatrix_MakeScale_2_scale'>scale</a>). Returned <a href='SkMatrix_Reference#Matrix'>matrix</a> is:

| <a href='#SkMatrix_MakeScale_2_scale'>scale</a>   0   0 |
|   0   <a href='#SkMatrix_MakeScale_2_scale'>scale</a> 0 |
|   0     0   1 |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_MakeScale_2_scale'><code><strong>scale</strong></code></a></td>
    <td>horizontal and vertical <a href='#SkMatrix_MakeScale_2_scale'>scale</a> factor</td>
  </tr>
</table>

### Return Value

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> with <a href='#SkMatrix_MakeScale_2_scale'>scale</a>

### Example

<div><fiddle-embed name="@Matrix_MakeScale_2"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setScale'>setScale</a> <a href='#SkMatrix_postScale'>postScale</a> <a href='#SkMatrix_preScale'>preScale</a>

<a name='SkMatrix_MakeTrans'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_MakeTrans'>MakeTrans</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to translate by (<a href='#SkMatrix_MakeTrans_dx'>dx</a>, <a href='#SkMatrix_MakeTrans_dy'>dy</a>). Returned <a href='SkMatrix_Reference#Matrix'>matrix</a> is:

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

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> with translation

### Example

<div><fiddle-embed name="@Matrix_MakeTrans"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setTranslate'>setTranslate</a> <a href='#SkMatrix_postTranslate'>postTranslate</a> <a href='#SkMatrix_preTranslate'>preTranslate</a>

<a name='SkMatrix_MakeAll'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_MakeAll'>MakeAll</a>(<a href='undocumented#SkScalar'>SkScalar</a> scaleX, <a href='undocumented#SkScalar'>SkScalar</a> skewX, <a href='undocumented#SkScalar'>SkScalar</a> transX, <a href='undocumented#SkScalar'>SkScalar</a> skewY,
                        <a href='undocumented#SkScalar'>SkScalar</a> scaleY, <a href='undocumented#SkScalar'>SkScalar</a> transY, <a href='undocumented#SkScalar'>SkScalar</a> pers0, <a href='undocumented#SkScalar'>SkScalar</a> pers1,
                        <a href='undocumented#SkScalar'>SkScalar</a> pers2)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to:

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

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> constructed from parameters

### Example

<div><fiddle-embed name="6bad83b64de9266e323c29d550e04188"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setAll'>setAll</a> <a href='#SkMatrix_set9'>set9</a> <a href='#SkMatrix_postConcat'>postConcat</a> <a href='#SkMatrix_preConcat'>preConcat</a>

<a name='SkMatrix_TypeMask'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkMatrix_TypeMask'>TypeMask</a> {
        <a href='#SkMatrix_kIdentity_Mask'>kIdentity_Mask</a> = 0,
        <a href='#SkMatrix_kTranslate_Mask'>kTranslate_Mask</a> = 0x01,
        <a href='#SkMatrix_kScale_Mask'>kScale_Mask</a> = 0x02,
        <a href='#SkMatrix_kAffine_Mask'>kAffine_Mask</a> = 0x04,
        <a href='#SkMatrix_kPerspective_Mask'>kPerspective_Mask</a> = 0x08,
    };

</pre>

Enumeration of bit fields for mask returned by <a href='#SkMatrix_getType'>getType</a>.
Used to identify the complexity of <a href='SkMatrix_Reference#Matrix'>Matrix</a>, to optimize performance.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kIdentity_Mask'><code>SkMatrix::kIdentity_Mask</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
all bits clear if <a href='SkMatrix_Reference#Matrix'>Matrix</a> is identity
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kTranslate_Mask'><code>SkMatrix::kTranslate_Mask</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
set if <a href='SkMatrix_Reference#Matrix'>Matrix</a> has translation
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kScale_Mask'><code>SkMatrix::kScale_Mask</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
set if <a href='SkMatrix_Reference#Matrix'>Matrix</a> scales x-axis or y-axis
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kAffine_Mask'><code>SkMatrix::kAffine_Mask</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
set if <a href='SkMatrix_Reference#Matrix'>Matrix</a> skews or rotates
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kPerspective_Mask'><code>SkMatrix::kPerspective_Mask</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>8</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
set if <a href='SkMatrix_Reference#Matrix'>Matrix</a> has perspective
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_TypeMask">

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
<a href='#SkMatrix_TypeMask'>TypeMask</a> <a href='#SkMatrix_getType'>getType</a>()const
</pre>

Returns a bit field describing the transformations the <a href='SkMatrix_Reference#Matrix'>matrix</a> may
perform. The bit field is computed conservatively, so it may include
false positives. For example, when <a href='#SkMatrix_kPerspective_Mask'>kPerspective_Mask</a> is set, all
other bits are set.

### Return Value

<a href='#SkMatrix_kIdentity_Mask'>kIdentity_Mask</a>, or combinations of: <a href='#SkMatrix_kTranslate_Mask'>kTranslate_Mask</a>, <a href='#SkMatrix_kScale_Mask'>kScale_Mask</a>,

<a href='#SkMatrix_kAffine_Mask'>kAffine_Mask</a>, <a href='#SkMatrix_kPerspective_Mask'>kPerspective_Mask</a>

### Example

<div><fiddle-embed name="@Matrix_getType">

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
bool <a href='#SkMatrix_isIdentity'>isIdentity</a>()const
</pre>

Returns true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> is identity.  Identity <a href='SkMatrix_Reference#Matrix'>matrix</a> is:

| 1 0 0 |
| 0 1 0 |
| 0 0 1 |

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> has no effect

### Example

<div><fiddle-embed name="@Matrix_isIdentity">

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
bool <a href='#SkMatrix_isScaleTranslate'>isScaleTranslate</a>()const
</pre>

Returns true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> at most scales and translates. <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> may be identity,
contain only scale elements, only translate elements, or both. <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> form is:

| scale-x    0    translate-x |
|    0    scale-y translate-y |
|    0       0         1      |

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> is identity; or scales, translates, or both

### Example

<div><fiddle-embed name="@Matrix_isScaleTranslate">

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
bool <a href='#SkMatrix_isTranslate'>isTranslate</a>()const
</pre>

Returns true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> is identity, or translates. <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> form is:

| 1 0 translate-x |
| 0 1 translate-y |
| 0 0      1      |

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> is identity, or translates

### Example

<div><fiddle-embed name="@Matrix_isTranslate">

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
bool <a href='#SkMatrix_rectStaysRect'>rectStaysRect</a>()const
</pre>

Returns true <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> maps <a href='SkRect_Reference#SkRect'>SkRect</a> to another <a href='SkRect_Reference#SkRect'>SkRect</a>. If true, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> is identity,
or scales, or rotates a multiple of 90 degrees, or mirrors on axes. In all
cases, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> may also have translation. <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> form is either:

| scale-x    0    translate-x |
|    0    scale-y translate-y |
|    0       0         1      |

or

|    0     rotate-x translate-x |
| rotate-y    0     translate-y |
|    0        0          1      |

for non-zero values of scale-x, scale-y, rotate-x, and rotate-y.

Also called <a href='#SkMatrix_preservesAxisAlignment'>preservesAxisAlignment</a>(); use the one that provides better inline
documentation.

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> maps one <a href='SkRect_Reference#SkRect'>SkRect</a> into another

### Example

<div><fiddle-embed name="@Matrix_rectStaysRect">

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
bool <a href='#SkMatrix_preservesAxisAlignment'>preservesAxisAlignment</a>()const
</pre>

Returns true <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> maps <a href='SkRect_Reference#SkRect'>SkRect</a> to another <a href='SkRect_Reference#SkRect'>SkRect</a>. If true, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> is identity,
or scales, or rotates a multiple of 90 degrees, or mirrors on axes. In all
cases, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> may also have translation. <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> form is either:

| scale-x    0    translate-x |
|    0    scale-y translate-y |
|    0       0         1      |

or

|    0     rotate-x translate-x |
| rotate-y    0     translate-y |
|    0        0          1      |

for non-zero values of scale-x, scale-y, rotate-x, and rotate-y.

Also called <a href='#SkMatrix_rectStaysRect'>rectStaysRect</a>(); use the one that provides better inline
documentation.

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> maps one <a href='SkRect_Reference#SkRect'>SkRect</a> into another

### Example

<div><fiddle-embed name="@Matrix_preservesAxisAlignment">

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
bool <a href='#SkMatrix_hasPerspective'>hasPerspective</a>()const
</pre>

Returns true if the <a href='SkMatrix_Reference#Matrix'>matrix</a> contains perspective elements. <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> form is:

|       --            --              --          |
|       --            --              --          |
| perspective-x  perspective-y  perspective-scale |

where perspective-x or perspective-y is non-zero, or perspective-scale is
not one. All other elements may have any value.

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> is in most general form

### Example

<div><fiddle-embed name="688123908c733169bbbfaf11f41ecff6"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setAll'>setAll</a> <a href='#SkMatrix_set9'>set9</a> <a href='#SkMatrix_MakeAll'>MakeAll</a>

<a name='SkMatrix_isSimilarity'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_isSimilarity'>isSimilarity</a>(<a href='undocumented#SkScalar'>SkScalar</a> tol = <a href='undocumented#SK_ScalarNearlyZero'>SK_ScalarNearlyZero</a>)const
</pre>

Returns true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> contains only translation, rotation, reflection, and
uniform scale.
Returns false if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> contains different scales, skewing, perspective, or
degenerate forms that collapse to a <a href='undocumented#Line'>line</a> or <a href='SkPoint_Reference#Point'>point</a>.

Describes that the <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> makes rendering with and without the <a href='SkMatrix_Reference#Matrix'>matrix</a> are
visually alike; a transformed <a href='undocumented#Circle'>circle</a> remains a <a href='undocumented#Circle'>circle</a>. Mathematically, this is
referred to as similarity of a  <a href='undocumented#Euclidean_Space'>Euclidean space</a>, or a similarity transformation.

Preserves right angles, keeping the arms of the angle equal lengths.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_isSimilarity_tol'><code><strong>tol</strong></code></a></td>
    <td>to be deprecated</td>
  </tr>
</table>

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> only rotates, uniformly scales, translates

### Example

<div><fiddle-embed name="8b37f4ae7fec1756433c0f984175fb14"><div><a href='undocumented#String'>String</a> is drawn four times through but only two are visible. Drawing the pair
with <a href='#SkMatrix_isSimilarity'>isSimilarity</a> false reveals the pair not visible through the <a href='SkMatrix_Reference#Matrix'>matrix</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkMatrix_isScaleTranslate'>isScaleTranslate</a> <a href='#SkMatrix_preservesRightAngles'>preservesRightAngles</a> <a href='#SkMatrix_rectStaysRect'>rectStaysRect</a> <a href='#SkMatrix_isFixedStepInX'>isFixedStepInX</a>

<a name='SkMatrix_preservesRightAngles'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_preservesRightAngles'>preservesRightAngles</a>(<a href='undocumented#SkScalar'>SkScalar</a> tol = <a href='undocumented#SK_ScalarNearlyZero'>SK_ScalarNearlyZero</a>)const
</pre>

Returns true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> contains only translation, rotation, reflection, and
scale. Scale may differ along rotated axes.
Returns false if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> skewing, perspective, or degenerate forms that collapse
to a <a href='undocumented#Line'>line</a> or <a href='SkPoint_Reference#Point'>point</a>.

Preserves right angles, but not requiring that the arms of the angle
retain equal lengths.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_preservesRightAngles_tol'><code><strong>tol</strong></code></a></td>
    <td>to be deprecated</td>
  </tr>
</table>

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> only rotates, scales, translates

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
    static constexpr int <a href='#SkMatrix_kMSkewX'>kMSkewX</a> = 1;
    static constexpr int <a href='#SkMatrix_kMTransX'>kMTransX</a> = 2;
    static constexpr int <a href='#SkMatrix_kMSkewY'>kMSkewY</a> = 3;
    static constexpr int <a href='#SkMatrix_kMScaleY'>kMScaleY</a> = 4;
    static constexpr int <a href='#SkMatrix_kMTransY'>kMTransY</a> = 5;
    static constexpr int <a href='#SkMatrix_kMPersp0'>kMPersp0</a> = 6;
    static constexpr int <a href='#SkMatrix_kMPersp1'>kMPersp1</a> = 7;
    static constexpr int <a href='#SkMatrix_kMPersp2'>kMPersp2</a> = 8;
</pre>

<a href='SkMatrix_Reference#Matrix'>Matrix</a> organizes its values in row order. These members correspond to
each value in <a href='SkMatrix_Reference#Matrix'>Matrix</a>.

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

Affine arrays are in column major order to match the <a href='SkMatrix_Reference#Matrix'>matrix</a> used by
PDF and XPS.

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
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_array1_operator'>operator[]</a>(int index)const
</pre>

### Example

<div><fiddle-embed name="@Matrix_array_operator">

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
<a href='undocumented#SkScalar'>SkScalar</a> get(int index)const
</pre>

Returns one <a href='SkMatrix_Reference#Matrix'>matrix</a> value. Asserts if <a href='#SkMatrix_get_index'>index</a> is out of range and SK_DEBUG is
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

<div><fiddle-embed name="@Matrix_get">

#### Example Output

~~~~
matrix.get(SkMatrix::kMSkewX) == 42
matrix.get(SkMatrix::kMSkewY) == 24
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_array1_operator'>operator[]</a>(int <a href='#SkMatrix_get_index'>index</a>) <a href='#SkMatrix_set'>set</a>

<a name='SkMatrix_getScaleX'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getScaleX'>getScaleX</a>()const
</pre>

Returns scale factor multiplied by x-axis input, contributing to x-axis output.
With <a href='#SkMatrix_mapPoints'>mapPoints</a>(), scales <a href='SkPoint_Reference#SkPoint'>SkPoint</a> along the x-axis.

### Return Value

horizontal scale factor

### Example

<div><fiddle-embed name="@Matrix_getScaleX">

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
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getScaleY'>getScaleY</a>()const
</pre>

Returns scale factor multiplied by y-axis input, contributing to y-axis output.
With <a href='#SkMatrix_mapPoints'>mapPoints</a>(), scales <a href='SkPoint_Reference#SkPoint'>SkPoint</a> along the y-axis.

### Return Value

vertical scale factor

### Example

<div><fiddle-embed name="@Matrix_getScaleY">

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
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getSkewY'>getSkewY</a>()const
</pre>

Returns scale factor multiplied by x-axis input, contributing to y-axis output.
With <a href='#SkMatrix_mapPoints'>mapPoints</a>(), skews <a href='SkPoint_Reference#SkPoint'>SkPoint</a> along the y-axis.
Skewing both axes can rotate <a href='SkPoint_Reference#SkPoint'>SkPoint</a>.

### Return Value

vertical skew factor

### Example

<div><fiddle-embed name="@Matrix_getSkewY">

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
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getSkewX'>getSkewX</a>()const
</pre>

Returns scale factor multiplied by y-axis input, contributing to x-axis output.
With <a href='#SkMatrix_mapPoints'>mapPoints</a>(), skews <a href='SkPoint_Reference#SkPoint'>SkPoint</a> along the x-axis.
Skewing both axes can rotate <a href='SkPoint_Reference#SkPoint'>SkPoint</a>.

### Return Value

horizontal scale factor

### Example

<div><fiddle-embed name="@Matrix_getSkewX">

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
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getTranslateX'>getTranslateX</a>()const
</pre>

Returns translation contributing to x-axis output.
With <a href='#SkMatrix_mapPoints'>mapPoints</a>(), moves <a href='SkPoint_Reference#SkPoint'>SkPoint</a> along the x-axis.

### Return Value

horizontal translation factor

### Example

<div><fiddle-embed name="@Matrix_getTranslateX">

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
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getTranslateY'>getTranslateY</a>()const
</pre>

Returns translation contributing to y-axis output.
With <a href='#SkMatrix_mapPoints'>mapPoints</a>(), moves <a href='SkPoint_Reference#SkPoint'>SkPoint</a> along the y-axis.

### Return Value

vertical translation factor

### Example

<div><fiddle-embed name="@Matrix_getTranslateY">

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
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getPerspX'>getPerspX</a>()const
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
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getPerspY'>getPerspY</a>()const
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
<a href='undocumented#SkScalar'>SkScalar</a>& <a href='#SkMatrix_array1_operator'>operator[]</a>(int index)
</pre>

### Example

<div><fiddle-embed name="@Matrix_dirtyMatrixTypeCache">

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
void set(int index, <a href='undocumented#SkScalar'>SkScalar</a> value)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_set_value'>value</a>. Asserts if <a href='#SkMatrix_set_index'>index</a> is out of range and SK_DEBUG is
defined. Safer than operator[]; internal cache is always maintained.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_set_index'><code><strong>index</strong></code></a></td>
    <td>one of: <a href='#SkMatrix_kMScaleX'>kMScaleX</a>, <a href='#SkMatrix_kMSkewX'>kMSkewX</a>, <a href='#SkMatrix_kMTransX'>kMTransX</a>, <a href='#SkMatrix_kMSkewY'>kMSkewY</a>, <a href='#SkMatrix_kMScaleY'>kMScaleY</a>, <a href='#SkMatrix_kMTransY'>kMTransY</a>,</td>
  </tr>
</table>

<a href='#SkMatrix_kMPersp0'>kMPersp0</a>, <a href='#SkMatrix_kMPersp1'>kMPersp1</a>, <a href='#SkMatrix_kMPersp2'>kMPersp2</a>

### Parameters

<table>  <tr>    <td><a name='SkMatrix_set_value'><code><strong>value</strong></code></a></td>
    <td><a href='undocumented#Scalar'>scalar</a> to store in <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_set">

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
void <a href='#SkMatrix_setScaleX'>setScaleX</a>(<a href='undocumented#SkScalar'>SkScalar</a> v)
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
void <a href='#SkMatrix_setScaleY'>setScaleY</a>(<a href='undocumented#SkScalar'>SkScalar</a> v)
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
void <a href='#SkMatrix_setSkewY'>setSkewY</a>(<a href='undocumented#SkScalar'>SkScalar</a> v)
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
void <a href='#SkMatrix_setSkewX'>setSkewX</a>(<a href='undocumented#SkScalar'>SkScalar</a> v)
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
void <a href='#SkMatrix_setTranslateX'>setTranslateX</a>(<a href='undocumented#SkScalar'>SkScalar</a> v)
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
void <a href='#SkMatrix_setTranslateY'>setTranslateY</a>(<a href='undocumented#SkScalar'>SkScalar</a> v)
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
void <a href='#SkMatrix_setPerspX'>setPerspX</a>(<a href='undocumented#SkScalar'>SkScalar</a> v)
</pre>

Sets input x-axis perspective factor, which causes <a href='#SkMatrix_mapXY'>mapXY</a>() to vary input x-axis values
inversely proportional to input y-axis values.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setPerspX_v'><code><strong>v</strong></code></a></td>
    <td>perspective factor</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_setPerspX"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_getPerspX'>getPerspX</a> <a href='#SkMatrix_set'>set</a> <a href='#SkMatrix_setAll'>setAll</a> <a href='#SkMatrix_set9'>set9</a> <a href='#SkMatrix_MakeAll'>MakeAll</a>

<a name='SkMatrix_setPerspY'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setPerspY'>setPerspY</a>(<a href='undocumented#SkScalar'>SkScalar</a> v)
</pre>

Sets input y-axis perspective factor, which causes <a href='#SkMatrix_mapXY'>mapXY</a>() to vary input y-axis values
inversely proportional to input x-axis values.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setPerspY_v'><code><strong>v</strong></code></a></td>
    <td>perspective factor</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_setPerspY"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_getPerspY'>getPerspY</a> <a href='#SkMatrix_set'>set</a> <a href='#SkMatrix_setAll'>setAll</a> <a href='#SkMatrix_set9'>set9</a> <a href='#SkMatrix_MakeAll'>MakeAll</a>

<a name='SkMatrix_setAll'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setAll'>setAll</a>(<a href='undocumented#SkScalar'>SkScalar</a> scaleX, <a href='undocumented#SkScalar'>SkScalar</a> skewX, <a href='undocumented#SkScalar'>SkScalar</a> transX, <a href='undocumented#SkScalar'>SkScalar</a> skewY, <a href='undocumented#SkScalar'>SkScalar</a> scaleY,
            <a href='undocumented#SkScalar'>SkScalar</a> transY, <a href='undocumented#SkScalar'>SkScalar</a> persp0, <a href='undocumented#SkScalar'>SkScalar</a> persp1, <a href='undocumented#SkScalar'>SkScalar</a> persp2)
</pre>

Sets all values from parameters. Sets <a href='SkMatrix_Reference#Matrix'>matrix</a> to:

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
void <a href='#SkMatrix_get9'>get9</a>(<a href='undocumented#SkScalar'>SkScalar</a> buffer[9])const
</pre>

Copies nine <a href='undocumented#Scalar'>scalar</a> values contained by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> into <a href='#SkMatrix_get9_buffer'>buffer</a>, in member value
ascending order: <a href='#SkMatrix_kMScaleX'>kMScaleX</a>, <a href='#SkMatrix_kMSkewX'>kMSkewX</a>, <a href='#SkMatrix_kMTransX'>kMTransX</a>, <a href='#SkMatrix_kMSkewY'>kMSkewY</a>, <a href='#SkMatrix_kMScaleY'>kMScaleY</a>, <a href='#SkMatrix_kMTransY'>kMTransY</a>,
<a href='#SkMatrix_kMPersp0'>kMPersp0</a>, <a href='#SkMatrix_kMPersp1'>kMPersp1</a>, <a href='#SkMatrix_kMPersp2'>kMPersp2</a>.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_get9_buffer'><code><strong>buffer</strong></code></a></td>
    <td>storage for nine <a href='undocumented#Scalar'>scalar</a> values</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_get9">

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
void <a href='#SkMatrix_set9'>set9</a>(const <a href='undocumented#SkScalar'>SkScalar</a> buffer[9])
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to nine <a href='undocumented#Scalar'>scalar</a> values in <a href='#SkMatrix_set9_buffer'>buffer</a>, in member value ascending order:
<a href='#SkMatrix_kMScaleX'>kMScaleX</a>, <a href='#SkMatrix_kMSkewX'>kMSkewX</a>, <a href='#SkMatrix_kMTransX'>kMTransX</a>, <a href='#SkMatrix_kMSkewY'>kMSkewY</a>, <a href='#SkMatrix_kMScaleY'>kMScaleY</a>, <a href='#SkMatrix_kMTransY'>kMTransY</a>, <a href='#SkMatrix_kMPersp0'>kMPersp0</a>, <a href='#SkMatrix_kMPersp1'>kMPersp1</a>,
<a href='#SkMatrix_kMPersp2'>kMPersp2</a>.

Sets <a href='SkMatrix_Reference#Matrix'>matrix</a> to:

| <a href='#SkMatrix_set9_buffer'>buffer</a>[0] <a href='#SkMatrix_set9_buffer'>buffer</a>[1] <a href='#SkMatrix_set9_buffer'>buffer</a>[2] |
| <a href='#SkMatrix_set9_buffer'>buffer</a>[3] <a href='#SkMatrix_set9_buffer'>buffer</a>[4] <a href='#SkMatrix_set9_buffer'>buffer</a>[5] |
| <a href='#SkMatrix_set9_buffer'>buffer</a>[6] <a href='#SkMatrix_set9_buffer'>buffer</a>[7] <a href='#SkMatrix_set9_buffer'>buffer</a>[8] |

In the future, <a href='#SkMatrix_set9'>set9</a> followed by <a href='#SkMatrix_get9'>get9</a> may not return the same values. Since <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
maps non-homogeneous coordinates, scaling all nine values produces an equivalent
transformation, possibly improving precision.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_set9_buffer'><code><strong>buffer</strong></code></a></td>
    <td>nine <a href='undocumented#Scalar'>scalar</a> values</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_set9"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setAll'>setAll</a> <a href='#SkMatrix_get9'>get9</a> <a href='#SkMatrix_MakeAll'>MakeAll</a>

<a name='SkMatrix_reset'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_reset'>reset()</a>
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to identity; which has no effect on mapped <a href='SkPoint_Reference#SkPoint'>SkPoint</a>. Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to:

| 1 0 0 |
| 0 1 0 |
| 0 0 1 |

Also called <a href='#SkMatrix_setIdentity'>setIdentity</a>(); use the one that provides better inline
documentation.

### Example

<div><fiddle-embed name="@Matrix_reset">

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

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to identity; which has no effect on mapped <a href='SkPoint_Reference#SkPoint'>SkPoint</a>. Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to:

| 1 0 0 |
| 0 1 0 |
| 0 0 1 |

Also called <a href='#SkMatrix_reset'>reset()</a>; use the one that provides better inline
documentation.

### Example

<div><fiddle-embed name="@Matrix_setIdentity">

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
void <a href='#SkMatrix_setTranslate'>setTranslate</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to translate by (<a href='#SkMatrix_setTranslate_dx'>dx</a>, <a href='#SkMatrix_setTranslate_dy'>dy</a>).

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
void <a href='#SkMatrix_setTranslate'>setTranslate</a>(const <a href='SkPoint_Reference#SkVector'>SkVector</a>& v)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to translate by (<a href='#SkMatrix_setTranslate_2_v'>v</a>.<a href='#SkPoint_fX'>fX</a>, <a href='#SkMatrix_setTranslate_2_v'>v</a>.<a href='#SkPoint_fY'>fY</a>).

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setTranslate_2_v'><code><strong>v</strong></code></a></td>
    <td><a href='SkPoint_Reference#Vector'>vector</a> containing horizontal and vertical translation</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ccfc734aff2ddea0b097c83f5621de5e"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setTranslateX'>setTranslateX</a> <a href='#SkMatrix_setTranslateY'>setTranslateY</a> <a href='#SkMatrix_MakeTrans'>MakeTrans</a>

<a name='SkMatrix_setScale'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setScale'>setScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> sx, <a href='undocumented#SkScalar'>SkScalar</a> sy, <a href='undocumented#SkScalar'>SkScalar</a> px, <a href='undocumented#SkScalar'>SkScalar</a> py)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to scale by <a href='#SkMatrix_setScale_sx'>sx</a> and <a href='#SkMatrix_setScale_sy'>sy</a>, about a pivot <a href='SkPoint_Reference#Point'>point</a> at (<a href='#SkMatrix_setScale_px'>px</a>, <a href='#SkMatrix_setScale_py'>py</a>).
The pivot <a href='SkPoint_Reference#Point'>point</a> is unchanged when mapped with <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

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
void <a href='#SkMatrix_setScale'>setScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> sx, <a href='undocumented#SkScalar'>SkScalar</a> sy)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to scale by <a href='#SkMatrix_setScale_2_sx'>sx</a> and <a href='#SkMatrix_setScale_2_sy'>sy</a> about at pivot <a href='SkPoint_Reference#Point'>point</a> at (0, 0).

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
void <a href='#SkMatrix_setRotate'>setRotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> degrees, <a href='undocumented#SkScalar'>SkScalar</a> px, <a href='undocumented#SkScalar'>SkScalar</a> py)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to rotate by <a href='#SkMatrix_setRotate_degrees'>degrees</a> about a pivot <a href='SkPoint_Reference#Point'>point</a> at (<a href='#SkMatrix_setRotate_px'>px</a>, <a href='#SkMatrix_setRotate_py'>py</a>).
The pivot <a href='SkPoint_Reference#Point'>point</a> is unchanged when mapped with <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Positive <a href='#SkMatrix_setRotate_degrees'>degrees</a> rotates clockwise.

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

<div><fiddle-embed name="@Matrix_setRotate"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setSinCos'>setSinCos</a> <a href='#SkMatrix_preRotate'>preRotate</a> <a href='#SkMatrix_postRotate'>postRotate</a>

<a name='SkMatrix_setRotate_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setRotate'>setRotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> degrees)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to rotate by <a href='#SkMatrix_setRotate_2_degrees'>degrees</a> about a pivot <a href='SkPoint_Reference#Point'>point</a> at (0, 0).
Positive <a href='#SkMatrix_setRotate_2_degrees'>degrees</a> rotates clockwise.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setRotate_2_degrees'><code><strong>degrees</strong></code></a></td>
    <td>angle of axes relative to upright axes</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_setRotate_2"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setSinCos'>setSinCos</a> <a href='#SkMatrix_preRotate'>preRotate</a> <a href='#SkMatrix_postRotate'>postRotate</a>

<a name='SkMatrix_setSinCos'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setSinCos'>setSinCos</a>(<a href='undocumented#SkScalar'>SkScalar</a> sinValue, <a href='undocumented#SkScalar'>SkScalar</a> cosValue, <a href='undocumented#SkScalar'>SkScalar</a> px, <a href='undocumented#SkScalar'>SkScalar</a> py)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to rotate by <a href='#SkMatrix_setSinCos_sinValue'>sinValue</a> and <a href='#SkMatrix_setSinCos_cosValue'>cosValue</a>, about a pivot <a href='SkPoint_Reference#Point'>point</a> at (<a href='#SkMatrix_setSinCos_px'>px</a>, <a href='#SkMatrix_setSinCos_py'>py</a>).
The pivot <a href='SkPoint_Reference#Point'>point</a> is unchanged when mapped with <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

<a href='SkPoint_Reference#Vector'>Vector</a> (<a href='#SkMatrix_setSinCos_sinValue'>sinValue</a>, <a href='#SkMatrix_setSinCos_cosValue'>cosValue</a>) describes the angle of rotation relative to (0, 1).
<a href='SkPoint_Reference#Vector'>Vector</a> length specifies scale.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setSinCos_sinValue'><code><strong>sinValue</strong></code></a></td>
    <td>rotation <a href='SkPoint_Reference#Vector'>vector</a> x-axis component</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setSinCos_cosValue'><code><strong>cosValue</strong></code></a></td>
    <td>rotation <a href='SkPoint_Reference#Vector'>vector</a> y-axis component</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setSinCos_px'><code><strong>px</strong></code></a></td>
    <td>pivot on x-axis</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setSinCos_py'><code><strong>py</strong></code></a></td>
    <td>pivot on y-axis</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_setSinCos"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setRotate'>setRotate</a> <a href='#SkMatrix_setScale'>setScale</a> <a href='#SkMatrix_setRSXform'>setRSXform</a>

<a name='SkMatrix_setSinCos_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setSinCos'>setSinCos</a>(<a href='undocumented#SkScalar'>SkScalar</a> sinValue, <a href='undocumented#SkScalar'>SkScalar</a> cosValue)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to rotate by <a href='#SkMatrix_setSinCos_2_sinValue'>sinValue</a> and <a href='#SkMatrix_setSinCos_2_cosValue'>cosValue</a>, about a pivot <a href='SkPoint_Reference#Point'>point</a> at (0, 0).

<a href='SkPoint_Reference#Vector'>Vector</a> (<a href='#SkMatrix_setSinCos_2_sinValue'>sinValue</a>, <a href='#SkMatrix_setSinCos_2_cosValue'>cosValue</a>) describes the angle of rotation relative to (0, 1).
<a href='SkPoint_Reference#Vector'>Vector</a> length specifies scale.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setSinCos_2_sinValue'><code><strong>sinValue</strong></code></a></td>
    <td>rotation <a href='SkPoint_Reference#Vector'>vector</a> x-axis component</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setSinCos_2_cosValue'><code><strong>cosValue</strong></code></a></td>
    <td>rotation <a href='SkPoint_Reference#Vector'>vector</a> y-axis component</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_setSinCos_2"><div><a href='SkCanvas_Reference#Canvas'>Canvas</a> needs offset after applying <a href='SkMatrix_Reference#Matrix'>Matrix</a> to pivot about <a href='SkRect_Reference#Rect'>Rect</a> center.
</div></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setRotate'>setRotate</a> <a href='#SkMatrix_setScale'>setScale</a> <a href='#SkMatrix_setRSXform'>setRSXform</a>

<a name='SkMatrix_setRSXform'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='#SkMatrix_setRSXform'>setRSXform</a>(const <a href='undocumented#SkRSXform'>SkRSXform</a>& rsxForm)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to rotate, scale, and translate using a compressed <a href='SkMatrix_Reference#Matrix'>matrix</a> form.

<a href='SkPoint_Reference#Vector'>Vector</a> (<a href='#SkMatrix_setRSXform_rsxForm'>rsxForm</a>.<a href='#SkRSXform_fSSin'>fSSin</a>, <a href='#SkMatrix_setRSXform_rsxForm'>rsxForm</a>.<a href='#SkRSXform_fSCos'>fSCos</a>) describes the angle of rotation relative
to (0, 1). <a href='SkPoint_Reference#Vector'>Vector</a> length specifies scale. Mapped <a href='SkPoint_Reference#Point'>point</a> is rotated and scaled
by <a href='SkPoint_Reference#Vector'>vector</a>, then translated by (<a href='#SkMatrix_setRSXform_rsxForm'>rsxForm</a>.<a href='#SkRSXform_fTx'>fTx</a>, <a href='#SkMatrix_setRSXform_rsxForm'>rsxForm</a>.<a href='#SkRSXform_fTy'>fTy</a>).

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setRSXform_rsxForm'><code><strong>rsxForm</strong></code></a></td>
    <td>compressed <a href='undocumented#SkRSXform'>SkRSXform</a> <a href='SkMatrix_Reference#Matrix'>matrix</a></td>
  </tr>
</table>

### Return Value

reference to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>

### Example

<div><fiddle-embed name="@Matrix_setRSXform"><div><a href='SkCanvas_Reference#Canvas'>Canvas</a> needs offset after applying <a href='SkMatrix_Reference#Matrix'>Matrix</a> to pivot about <a href='SkRect_Reference#Rect'>Rect</a> center.
</div></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setSinCos'>setSinCos</a> <a href='#SkMatrix_setScale'>setScale</a> <a href='#SkMatrix_setTranslate'>setTranslate</a>

<a name='SkMatrix_setSkew'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setSkew'>setSkew</a>(<a href='undocumented#SkScalar'>SkScalar</a> kx, <a href='undocumented#SkScalar'>SkScalar</a> ky, <a href='undocumented#SkScalar'>SkScalar</a> px, <a href='undocumented#SkScalar'>SkScalar</a> py)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to skew by <a href='#SkMatrix_setSkew_kx'>kx</a> and <a href='#SkMatrix_setSkew_ky'>ky</a>, about a pivot <a href='SkPoint_Reference#Point'>point</a> at (<a href='#SkMatrix_setSkew_px'>px</a>, <a href='#SkMatrix_setSkew_py'>py</a>).
The pivot <a href='SkPoint_Reference#Point'>point</a> is unchanged when mapped with <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

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
void <a href='#SkMatrix_setSkew'>setSkew</a>(<a href='undocumented#SkScalar'>SkScalar</a> kx, <a href='undocumented#SkScalar'>SkScalar</a> ky)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to skew by <a href='#SkMatrix_setSkew_2_kx'>kx</a> and <a href='#SkMatrix_setSkew_2_ky'>ky</a>, about a pivot <a href='SkPoint_Reference#Point'>point</a> at (0, 0).

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
void <a href='#SkMatrix_setConcat'>setConcat</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& a, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& b)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_setConcat_a'>a</a> multiplied by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_setConcat_b'>b</a>. Either <a href='#SkMatrix_setConcat_a'>a</a> or <a href='#SkMatrix_setConcat_b'>b</a> may be this.

Given:

| A B C |      | J K L |
<a href='#SkMatrix_setConcat_a'>a</a> = | D E F |, <a href='#SkMatrix_setConcat_b'>b</a> = | M N O |
| G H <a href='#SkMatrix_I'>I</a> |      | P Q R |

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to:

| A B C |   | J K L |   | AJ+BM+CP AK+BN+CQ AL+BO+CR |
<a href='#SkMatrix_setConcat_a'>a</a> * <a href='#SkMatrix_setConcat_b'>b</a> = | D E F | * | M N O | = | DJ+EM+FP DK+EN+FQ DL+EO+FR |
| G H <a href='#SkMatrix_I'>I</a> |   | P Q R |   | GJ+HM+IP GK+HN+IQ GL+HO+IR |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setConcat_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> on  left side of multiply expression</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setConcat_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> on  right side of multiply expression</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_setConcat"><div><a href='#SkMatrix_setPolyToPoly'>setPolyToPoly</a> creates perspective <a href='SkMatrix_Reference#Matrix'>matrices</a>, one the inverse of the other.
Multiplying the <a href='SkMatrix_Reference#Matrix'>matrix</a> by its inverse turns into an identity <a href='SkMatrix_Reference#Matrix'>matrix</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkMatrix_Concat'>Concat</a> <a href='#SkMatrix_preConcat'>preConcat</a> <a href='#SkMatrix_postConcat'>postConcat</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_concat'>concat</a>

<a name='SkMatrix_preTranslate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_preTranslate'>preTranslate</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> multiplied by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> constructed from translation (<a href='#SkMatrix_preTranslate_dx'>dx</a>, <a href='#SkMatrix_preTranslate_dy'>dy</a>).
This can be thought of as moving the <a href='SkPoint_Reference#Point'>point</a> to be mapped before applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Given:

| A B C |               | 1 0 <a href='#SkMatrix_preTranslate_dx'>dx</a> |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | D E F |,  T(<a href='#SkMatrix_preTranslate_dx'>dx</a>, <a href='#SkMatrix_preTranslate_dy'>dy</a>) = | 0 1 <a href='#SkMatrix_preTranslate_dy'>dy</a> |
| G H <a href='#SkMatrix_I'>I</a> |               | 0 0  1 |

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to:

| A B C | | 1 0 <a href='#SkMatrix_preTranslate_dx'>dx</a> |   | A B A*<a href='#SkMatrix_preTranslate_dx'>dx</a>+B*<a href='#SkMatrix_preTranslate_dy'>dy</a>+C |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * T(<a href='#SkMatrix_preTranslate_dx'>dx</a>, <a href='#SkMatrix_preTranslate_dy'>dy</a>) = | D E F | | 0 1 <a href='#SkMatrix_preTranslate_dy'>dy</a> | = | D E D*<a href='#SkMatrix_preTranslate_dx'>dx</a>+E*<a href='#SkMatrix_preTranslate_dy'>dy</a>+F |
| G H <a href='#SkMatrix_I'>I</a> | | 0 0  1 |   | G H G*<a href='#SkMatrix_preTranslate_dx'>dx</a>+H*<a href='#SkMatrix_preTranslate_dy'>dy</a>+<a href='#SkMatrix_I'>I</a> |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_preTranslate_dx'><code><strong>dx</strong></code></a></td>
    <td>x-axis translation before applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_preTranslate_dy'><code><strong>dy</strong></code></a></td>
    <td>y-axis translation before applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_preTranslate"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_postTranslate'>postTranslate</a> <a href='#SkMatrix_setTranslate'>setTranslate</a> <a href='#SkMatrix_MakeTrans'>MakeTrans</a>

<a name='SkMatrix_preScale'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_preScale'>preScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> sx, <a href='undocumented#SkScalar'>SkScalar</a> sy, <a href='undocumented#SkScalar'>SkScalar</a> px, <a href='undocumented#SkScalar'>SkScalar</a> py)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> multiplied by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> constructed from scaling by (<a href='#SkMatrix_preScale_sx'>sx</a>, <a href='#SkMatrix_preScale_sy'>sy</a>)
about pivot <a href='SkPoint_Reference#Point'>point</a> (<a href='#SkMatrix_preScale_px'>px</a>, <a href='#SkMatrix_preScale_py'>py</a>).
This can be thought of as scaling about a pivot <a href='SkPoint_Reference#Point'>point</a> before applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Given:

| A B C |                       | <a href='#SkMatrix_preScale_sx'>sx</a>  0 dx |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | D E F |,  S(<a href='#SkMatrix_preScale_sx'>sx</a>, <a href='#SkMatrix_preScale_sy'>sy</a>, <a href='#SkMatrix_preScale_px'>px</a>, <a href='#SkMatrix_preScale_py'>py</a>) = |  0 <a href='#SkMatrix_preScale_sy'>sy</a> dy |
| G H <a href='#SkMatrix_I'>I</a> |                       |  0  0  1 |

where

dx = <a href='#SkMatrix_preScale_px'>px</a> - <a href='#SkMatrix_preScale_sx'>sx</a> * <a href='#SkMatrix_preScale_px'>px</a>
dy = <a href='#SkMatrix_preScale_py'>py</a> - <a href='#SkMatrix_preScale_sy'>sy</a> * <a href='#SkMatrix_preScale_py'>py</a>

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to:

| A B C | | <a href='#SkMatrix_preScale_sx'>sx</a>  0 dx |   | A*<a href='#SkMatrix_preScale_sx'>sx</a> B*<a href='#SkMatrix_preScale_sy'>sy</a> A*dx+B*dy+C |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * S(<a href='#SkMatrix_preScale_sx'>sx</a>, <a href='#SkMatrix_preScale_sy'>sy</a>, <a href='#SkMatrix_preScale_px'>px</a>, <a href='#SkMatrix_preScale_py'>py</a>) = | D E F | |  0 <a href='#SkMatrix_preScale_sy'>sy</a> dy | = | D*<a href='#SkMatrix_preScale_sx'>sx</a> E*<a href='#SkMatrix_preScale_sy'>sy</a> D*dx+E*dy+F |
| G H <a href='#SkMatrix_I'>I</a> | |  0  0  1 |   | G*<a href='#SkMatrix_preScale_sx'>sx</a> H*<a href='#SkMatrix_preScale_sy'>sy</a> G*dx+H*dy+<a href='#SkMatrix_I'>I</a> |

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

<div><fiddle-embed name="@Matrix_preScale"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_postScale'>postScale</a> <a href='#SkMatrix_setScale'>setScale</a> <a href='#SkMatrix_MakeScale'>MakeScale</a>

<a name='SkMatrix_preScale_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_preScale'>preScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> sx, <a href='undocumented#SkScalar'>SkScalar</a> sy)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> multiplied by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> constructed from scaling by (<a href='#SkMatrix_preScale_2_sx'>sx</a>, <a href='#SkMatrix_preScale_2_sy'>sy</a>)
about pivot <a href='SkPoint_Reference#Point'>point</a> (0, 0).
This can be thought of as scaling about the origin before applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Given:

| A B C |               | <a href='#SkMatrix_preScale_2_sx'>sx</a>  0  0 |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | D E F |,  S(<a href='#SkMatrix_preScale_2_sx'>sx</a>, <a href='#SkMatrix_preScale_2_sy'>sy</a>) = |  0 <a href='#SkMatrix_preScale_2_sy'>sy</a>  0 |
| G H <a href='#SkMatrix_I'>I</a> |               |  0  0  1 |

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to:

| A B C | | <a href='#SkMatrix_preScale_2_sx'>sx</a>  0  0 |   | A*<a href='#SkMatrix_preScale_2_sx'>sx</a> B*<a href='#SkMatrix_preScale_2_sy'>sy</a> C |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * S(<a href='#SkMatrix_preScale_2_sx'>sx</a>, <a href='#SkMatrix_preScale_2_sy'>sy</a>) = | D E F | |  0 <a href='#SkMatrix_preScale_2_sy'>sy</a>  0 | = | D*<a href='#SkMatrix_preScale_2_sx'>sx</a> E*<a href='#SkMatrix_preScale_2_sy'>sy</a> F |
| G H <a href='#SkMatrix_I'>I</a> | |  0  0  1 |   | G*<a href='#SkMatrix_preScale_2_sx'>sx</a> H*<a href='#SkMatrix_preScale_2_sy'>sy</a> <a href='#SkMatrix_I'>I</a> |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_preScale_2_sx'><code><strong>sx</strong></code></a></td>
    <td>horizontal scale factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_preScale_2_sy'><code><strong>sy</strong></code></a></td>
    <td>vertical scale factor</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_preScale_2"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_postScale'>postScale</a> <a href='#SkMatrix_setScale'>setScale</a> <a href='#SkMatrix_MakeScale'>MakeScale</a>

<a name='SkMatrix_preRotate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_preRotate'>preRotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> degrees, <a href='undocumented#SkScalar'>SkScalar</a> px, <a href='undocumented#SkScalar'>SkScalar</a> py)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> multiplied by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> constructed from rotating by <a href='#SkMatrix_preRotate_degrees'>degrees</a>
about pivot <a href='SkPoint_Reference#Point'>point</a> (<a href='#SkMatrix_preRotate_px'>px</a>, <a href='#SkMatrix_preRotate_py'>py</a>).
This can be thought of as rotating about a pivot <a href='SkPoint_Reference#Point'>point</a> before applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Positive <a href='#SkMatrix_preRotate_degrees'>degrees</a> rotates clockwise.

Given:

| A B C |                        | c -s dx |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | D E F |,  R(<a href='#SkMatrix_preRotate_degrees'>degrees</a>, <a href='#SkMatrix_preRotate_px'>px</a>, <a href='#SkMatrix_preRotate_py'>py</a>) = | s  c dy |
| G H <a href='#SkMatrix_I'>I</a> |                        | 0  0  1 |

where

c  = cos(<a href='#SkMatrix_preRotate_degrees'>degrees</a>)
s  = sin(<a href='#SkMatrix_preRotate_degrees'>degrees</a>)
dx =  s * <a href='#SkMatrix_preRotate_py'>py</a> + (1 - c) * <a href='#SkMatrix_preRotate_px'>px</a>
dy = -s * <a href='#SkMatrix_preRotate_px'>px</a> + (1 - c) * <a href='#SkMatrix_preRotate_py'>py</a>

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to:

| A B C | | c -s dx |   | Ac+Bs -As+Bc A*dx+B*dy+C |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * R(<a href='#SkMatrix_preRotate_degrees'>degrees</a>, <a href='#SkMatrix_preRotate_px'>px</a>, <a href='#SkMatrix_preRotate_py'>py</a>) = | D E F | | s  c dy | = | Dc+Es -Ds+Ec D*dx+E*dy+F |
| G H <a href='#SkMatrix_I'>I</a> | | 0  0  1 |   | Gc+Hs -Gs+Hc G*dx+H*dy+<a href='#SkMatrix_I'>I</a> |

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

<div><fiddle-embed name="@Matrix_preRotate"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_postRotate'>postRotate</a> <a href='#SkMatrix_setRotate'>setRotate</a>

<a name='SkMatrix_preRotate_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_preRotate'>preRotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> degrees)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> multiplied by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> constructed from rotating by <a href='#SkMatrix_preRotate_2_degrees'>degrees</a>
about pivot <a href='SkPoint_Reference#Point'>point</a> (0, 0).
This can be thought of as rotating about the origin before applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Positive <a href='#SkMatrix_preRotate_2_degrees'>degrees</a> rotates clockwise.

Given:

| A B C |                        | c -s 0 |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | D E F |,  R(<a href='#SkMatrix_preRotate_2_degrees'>degrees</a>, px, py) = | s  c 0 |
| G H <a href='#SkMatrix_I'>I</a> |                        | 0  0 1 |

where

c  = cos(<a href='#SkMatrix_preRotate_2_degrees'>degrees</a>)
s  = sin(<a href='#SkMatrix_preRotate_2_degrees'>degrees</a>)

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to:

| A B C | | c -s 0 |   | Ac+Bs -As+Bc C |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * R(<a href='#SkMatrix_preRotate_2_degrees'>degrees</a>, px, py) = | D E F | | s  c 0 | = | Dc+Es -Ds+Ec F |
| G H <a href='#SkMatrix_I'>I</a> | | 0  0 1 |   | Gc+Hs -Gs+Hc <a href='#SkMatrix_I'>I</a> |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_preRotate_2_degrees'><code><strong>degrees</strong></code></a></td>
    <td>angle of axes relative to upright axes</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_preRotate_2"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_postRotate'>postRotate</a> <a href='#SkMatrix_setRotate'>setRotate</a>

<a name='SkMatrix_preSkew'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_preSkew'>preSkew</a>(<a href='undocumented#SkScalar'>SkScalar</a> kx, <a href='undocumented#SkScalar'>SkScalar</a> ky, <a href='undocumented#SkScalar'>SkScalar</a> px, <a href='undocumented#SkScalar'>SkScalar</a> py)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> multiplied by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> constructed from skewing by (<a href='#SkMatrix_preSkew_kx'>kx</a>, <a href='#SkMatrix_preSkew_ky'>ky</a>)
about pivot <a href='SkPoint_Reference#Point'>point</a> (<a href='#SkMatrix_preSkew_px'>px</a>, <a href='#SkMatrix_preSkew_py'>py</a>).
This can be thought of as skewing about a pivot <a href='SkPoint_Reference#Point'>point</a> before applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Given:

| A B C |                       |  1 <a href='#SkMatrix_preSkew_kx'>kx</a> dx |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | D E F |,  K(<a href='#SkMatrix_preSkew_kx'>kx</a>, <a href='#SkMatrix_preSkew_ky'>ky</a>, <a href='#SkMatrix_preSkew_px'>px</a>, <a href='#SkMatrix_preSkew_py'>py</a>) = | <a href='#SkMatrix_preSkew_ky'>ky</a>  1 dy |
| G H <a href='#SkMatrix_I'>I</a> |                       |  0  0  1 |

where

dx = -<a href='#SkMatrix_preSkew_kx'>kx</a> * <a href='#SkMatrix_preSkew_py'>py</a>
dy = -<a href='#SkMatrix_preSkew_ky'>ky</a> * <a href='#SkMatrix_preSkew_px'>px</a>

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to:

| A B C | |  1 <a href='#SkMatrix_preSkew_kx'>kx</a> dx |   | A+B*<a href='#SkMatrix_preSkew_ky'>ky</a> A*<a href='#SkMatrix_preSkew_kx'>kx</a>+B A*dx+B*dy+C |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * K(<a href='#SkMatrix_preSkew_kx'>kx</a>, <a href='#SkMatrix_preSkew_ky'>ky</a>, <a href='#SkMatrix_preSkew_px'>px</a>, <a href='#SkMatrix_preSkew_py'>py</a>) = | D E F | | <a href='#SkMatrix_preSkew_ky'>ky</a>  1 dy | = | D+E*<a href='#SkMatrix_preSkew_ky'>ky</a> D*<a href='#SkMatrix_preSkew_kx'>kx</a>+E D*dx+E*dy+F |
| G H <a href='#SkMatrix_I'>I</a> | |  0  0  1 |   | G+H*<a href='#SkMatrix_preSkew_ky'>ky</a> G*<a href='#SkMatrix_preSkew_kx'>kx</a>+H G*dx+H*dy+<a href='#SkMatrix_I'>I</a> |

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

<div><fiddle-embed name="@Matrix_preSkew"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_postSkew'>postSkew</a> <a href='#SkMatrix_setSkew'>setSkew</a>

<a name='SkMatrix_preSkew_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_preSkew'>preSkew</a>(<a href='undocumented#SkScalar'>SkScalar</a> kx, <a href='undocumented#SkScalar'>SkScalar</a> ky)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> multiplied by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> constructed from skewing by (<a href='#SkMatrix_preSkew_2_kx'>kx</a>, <a href='#SkMatrix_preSkew_2_ky'>ky</a>)
about pivot <a href='SkPoint_Reference#Point'>point</a> (0, 0).
This can be thought of as skewing about the origin before applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Given:

| A B C |               |  1 <a href='#SkMatrix_preSkew_2_kx'>kx</a> 0 |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | D E F |,  K(<a href='#SkMatrix_preSkew_2_kx'>kx</a>, <a href='#SkMatrix_preSkew_2_ky'>ky</a>) = | <a href='#SkMatrix_preSkew_2_ky'>ky</a>  1 0 |
| G H <a href='#SkMatrix_I'>I</a> |               |  0  0 1 |

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to:

| A B C | |  1 <a href='#SkMatrix_preSkew_2_kx'>kx</a> 0 |   | A+B*<a href='#SkMatrix_preSkew_2_ky'>ky</a> A*<a href='#SkMatrix_preSkew_2_kx'>kx</a>+B C |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * K(<a href='#SkMatrix_preSkew_2_kx'>kx</a>, <a href='#SkMatrix_preSkew_2_ky'>ky</a>) = | D E F | | <a href='#SkMatrix_preSkew_2_ky'>ky</a>  1 0 | = | D+E*<a href='#SkMatrix_preSkew_2_ky'>ky</a> D*<a href='#SkMatrix_preSkew_2_kx'>kx</a>+E F |
| G H <a href='#SkMatrix_I'>I</a> | |  0  0 1 |   | G+H*<a href='#SkMatrix_preSkew_2_ky'>ky</a> G*<a href='#SkMatrix_preSkew_2_kx'>kx</a>+H <a href='#SkMatrix_I'>I</a> |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_preSkew_2_kx'><code><strong>kx</strong></code></a></td>
    <td>horizontal skew factor</td>
  </tr>
  <tr>    <td><a name='SkMatrix_preSkew_2_ky'><code><strong>ky</strong></code></a></td>
    <td>vertical skew factor</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_preSkew_2"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_postSkew'>postSkew</a> <a href='#SkMatrix_setSkew'>setSkew</a>

<a name='SkMatrix_preConcat'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_preConcat'>preConcat</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& other)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> multiplied by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_preConcat_other'>other</a>.
This can be thought of mapping by <a href='#SkMatrix_preConcat_other'>other</a> before applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Given:

| A B C |          | J K L |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | D E F |, <a href='#SkMatrix_preConcat_other'>other</a> = | M N O |
| G H <a href='#SkMatrix_I'>I</a> |          | P Q R |

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to:

| A B C |   | J K L |   | AJ+BM+CP AK+BN+CQ AL+BO+CR |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * <a href='#SkMatrix_preConcat_other'>other</a> = | D E F | * | M N O | = | DJ+EM+FP DK+EN+FQ DL+EO+FR |
| G H <a href='#SkMatrix_I'>I</a> |   | P Q R |   | GJ+HM+IP GK+HN+IQ GL+HO+IR |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_preConcat_other'><code><strong>other</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> on  right side of multiply expression</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_preConcat"><div><a href='#SkMatrix_setPolyToPoly'>setPolyToPoly</a> creates perspective <a href='SkMatrix_Reference#Matrix'>matrices</a>, one the inverse of the <a href='#SkMatrix_preConcat_other'>other</a>.
Multiplying the <a href='SkMatrix_Reference#Matrix'>matrix</a> by its inverse turns into an identity <a href='SkMatrix_Reference#Matrix'>matrix</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkMatrix_postConcat'>postConcat</a> <a href='#SkMatrix_setConcat'>setConcat</a> <a href='#SkMatrix_Concat'>Concat</a>

<a name='SkMatrix_postTranslate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_postTranslate'>postTranslate</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> constructed from translation (<a href='#SkMatrix_postTranslate_dx'>dx</a>, <a href='#SkMatrix_postTranslate_dy'>dy</a>) multiplied by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
This can be thought of as moving the <a href='SkPoint_Reference#Point'>point</a> to be mapped after applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Given:

| J K L |               | 1 0 <a href='#SkMatrix_postTranslate_dx'>dx</a> |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | M N O |,  T(<a href='#SkMatrix_postTranslate_dx'>dx</a>, <a href='#SkMatrix_postTranslate_dy'>dy</a>) = | 0 1 <a href='#SkMatrix_postTranslate_dy'>dy</a> |
| P Q R |               | 0 0  1 |

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to:

| 1 0 <a href='#SkMatrix_postTranslate_dx'>dx</a> | | J K L |   | J+<a href='#SkMatrix_postTranslate_dx'>dx</a>*P K+<a href='#SkMatrix_postTranslate_dx'>dx</a>*Q L+<a href='#SkMatrix_postTranslate_dx'>dx</a>*R |
T(<a href='#SkMatrix_postTranslate_dx'>dx</a>, <a href='#SkMatrix_postTranslate_dy'>dy</a>) * <a href='SkMatrix_Reference#Matrix'>Matrix</a> = | 0 1 <a href='#SkMatrix_postTranslate_dy'>dy</a> | | M N O | = | M+<a href='#SkMatrix_postTranslate_dy'>dy</a>*P N+<a href='#SkMatrix_postTranslate_dy'>dy</a>*Q O+<a href='#SkMatrix_postTranslate_dy'>dy</a>*R |
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

<div><fiddle-embed name="@Matrix_postTranslate"><div>Compare with <a href='#SkMatrix_preTranslate'>preTranslate</a> example.
</div></fiddle-embed></div>

### See Also

<a href='#SkMatrix_preTranslate'>preTranslate</a> <a href='#SkMatrix_setTranslate'>setTranslate</a> <a href='#SkMatrix_MakeTrans'>MakeTrans</a>

<a name='SkMatrix_postScale'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_postScale'>postScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> sx, <a href='undocumented#SkScalar'>SkScalar</a> sy, <a href='undocumented#SkScalar'>SkScalar</a> px, <a href='undocumented#SkScalar'>SkScalar</a> py)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> constructed from scaling by (<a href='#SkMatrix_postScale_sx'>sx</a>, <a href='#SkMatrix_postScale_sy'>sy</a>) about pivot <a href='SkPoint_Reference#Point'>point</a>
(<a href='#SkMatrix_postScale_px'>px</a>, <a href='#SkMatrix_postScale_py'>py</a>), multiplied by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
This can be thought of as scaling about a pivot <a href='SkPoint_Reference#Point'>point</a> after applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Given:

| J K L |                       | <a href='#SkMatrix_postScale_sx'>sx</a>  0 dx |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | M N O |,  S(<a href='#SkMatrix_postScale_sx'>sx</a>, <a href='#SkMatrix_postScale_sy'>sy</a>, <a href='#SkMatrix_postScale_px'>px</a>, <a href='#SkMatrix_postScale_py'>py</a>) = |  0 <a href='#SkMatrix_postScale_sy'>sy</a> dy |
| P Q R |                       |  0  0  1 |

where

dx = <a href='#SkMatrix_postScale_px'>px</a> - <a href='#SkMatrix_postScale_sx'>sx</a> * <a href='#SkMatrix_postScale_px'>px</a>
dy = <a href='#SkMatrix_postScale_py'>py</a> - <a href='#SkMatrix_postScale_sy'>sy</a> * <a href='#SkMatrix_postScale_py'>py</a>

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to:

| <a href='#SkMatrix_postScale_sx'>sx</a>  0 dx | | J K L |   | <a href='#SkMatrix_postScale_sx'>sx</a>*J+dx*P <a href='#SkMatrix_postScale_sx'>sx</a>*K+dx*Q <a href='#SkMatrix_postScale_sx'>sx</a>*L+dx+R |
S(<a href='#SkMatrix_postScale_sx'>sx</a>, <a href='#SkMatrix_postScale_sy'>sy</a>, <a href='#SkMatrix_postScale_px'>px</a>, <a href='#SkMatrix_postScale_py'>py</a>) * <a href='SkMatrix_Reference#Matrix'>Matrix</a> = |  0 <a href='#SkMatrix_postScale_sy'>sy</a> dy | | M N O | = | <a href='#SkMatrix_postScale_sy'>sy</a>*M+dy*P <a href='#SkMatrix_postScale_sy'>sy</a>*N+dy*Q <a href='#SkMatrix_postScale_sy'>sy</a>*O+dy*R |
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

<div><fiddle-embed name="@Matrix_postScale"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_preScale'>preScale</a> <a href='#SkMatrix_setScale'>setScale</a> <a href='#SkMatrix_MakeScale'>MakeScale</a>

<a name='SkMatrix_postScale_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_postScale'>postScale</a>(<a href='undocumented#SkScalar'>SkScalar</a> sx, <a href='undocumented#SkScalar'>SkScalar</a> sy)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> constructed from scaling by (<a href='#SkMatrix_postScale_2_sx'>sx</a>, <a href='#SkMatrix_postScale_2_sy'>sy</a>) about pivot <a href='SkPoint_Reference#Point'>point</a>
(0, 0), multiplied by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
This can be thought of as scaling about the origin after applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Given:

| J K L |               | <a href='#SkMatrix_postScale_2_sx'>sx</a>  0  0 |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | M N O |,  S(<a href='#SkMatrix_postScale_2_sx'>sx</a>, <a href='#SkMatrix_postScale_2_sy'>sy</a>) = |  0 <a href='#SkMatrix_postScale_2_sy'>sy</a>  0 |
| P Q R |               |  0  0  1 |

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to:

| <a href='#SkMatrix_postScale_2_sx'>sx</a>  0  0 | | J K L |   | <a href='#SkMatrix_postScale_2_sx'>sx</a>*J <a href='#SkMatrix_postScale_2_sx'>sx</a>*K <a href='#SkMatrix_postScale_2_sx'>sx</a>*L |
S(<a href='#SkMatrix_postScale_2_sx'>sx</a>, <a href='#SkMatrix_postScale_2_sy'>sy</a>) * <a href='SkMatrix_Reference#Matrix'>Matrix</a> = |  0 <a href='#SkMatrix_postScale_2_sy'>sy</a>  0 | | M N O | = | <a href='#SkMatrix_postScale_2_sy'>sy</a>*M <a href='#SkMatrix_postScale_2_sy'>sy</a>*N <a href='#SkMatrix_postScale_2_sy'>sy</a>*O |
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

<div><fiddle-embed name="@Matrix_postScale_2"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_preScale'>preScale</a> <a href='#SkMatrix_setScale'>setScale</a> <a href='#SkMatrix_MakeScale'>MakeScale</a>

<a name='SkMatrix_postIDiv'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_postIDiv'>postIDiv</a>(int divx, int divy)
</pre>

Sets <a href='SkMatrix_Reference#Matrix'>Matrix</a> to <a href='SkMatrix_Reference#Matrix'>Matrix</a> constructed from scaling by (1/<a href='#SkMatrix_postIDiv_divx'>divx</a>, 1/<a href='#SkMatrix_postIDiv_divy'>divy</a>),
multiplied by <a href='SkMatrix_Reference#Matrix'>Matrix</a>.

Returns false if either <a href='#SkMatrix_postIDiv_divx'>divx</a> or <a href='#SkMatrix_postIDiv_divy'>divy</a> is zero.

Given:

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

sets <a href='SkMatrix_Reference#Matrix'>Matrix</a> to:

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

<div><fiddle-embed name="@Matrix_063"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_postScale'>postScale</a> <a href='#SkMatrix_MakeScale'>MakeScale</a>

<a name='SkMatrix_postRotate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_postRotate'>postRotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> degrees, <a href='undocumented#SkScalar'>SkScalar</a> px, <a href='undocumented#SkScalar'>SkScalar</a> py)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> constructed from rotating by <a href='#SkMatrix_postRotate_degrees'>degrees</a> about pivot <a href='SkPoint_Reference#Point'>point</a>
(<a href='#SkMatrix_postRotate_px'>px</a>, <a href='#SkMatrix_postRotate_py'>py</a>), multiplied by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
This can be thought of as rotating about a pivot <a href='SkPoint_Reference#Point'>point</a> after applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Positive <a href='#SkMatrix_postRotate_degrees'>degrees</a> rotates clockwise.

Given:

| J K L |                        | c -s dx |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | M N O |,  R(<a href='#SkMatrix_postRotate_degrees'>degrees</a>, <a href='#SkMatrix_postRotate_px'>px</a>, <a href='#SkMatrix_postRotate_py'>py</a>) = | s  c dy |
| P Q R |                        | 0  0  1 |

where

c  = cos(<a href='#SkMatrix_postRotate_degrees'>degrees</a>)
s  = sin(<a href='#SkMatrix_postRotate_degrees'>degrees</a>)
dx =  s * <a href='#SkMatrix_postRotate_py'>py</a> + (1 - c) * <a href='#SkMatrix_postRotate_px'>px</a>
dy = -s * <a href='#SkMatrix_postRotate_px'>px</a> + (1 - c) * <a href='#SkMatrix_postRotate_py'>py</a>

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to:

|c -s dx| |J K L|   |cJ-sM+dx*P cK-sN+dx*Q cL-sO+dx+R|
R(<a href='#SkMatrix_postRotate_degrees'>degrees</a>, <a href='#SkMatrix_postRotate_px'>px</a>, <a href='#SkMatrix_postRotate_py'>py</a>) * <a href='SkMatrix_Reference#Matrix'>Matrix</a> = |s  c dy| |M N O| = |sJ+cM+dy*P sK+cN+dy*Q sL+cO+dy*R|
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

<div><fiddle-embed name="@Matrix_postRotate"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_preRotate'>preRotate</a> <a href='#SkMatrix_setRotate'>setRotate</a>

<a name='SkMatrix_postRotate_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_postRotate'>postRotate</a>(<a href='undocumented#SkScalar'>SkScalar</a> degrees)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> constructed from rotating by <a href='#SkMatrix_postRotate_2_degrees'>degrees</a> about pivot <a href='SkPoint_Reference#Point'>point</a>
(0, 0), multiplied by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
This can be thought of as rotating about the origin after applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Positive <a href='#SkMatrix_postRotate_2_degrees'>degrees</a> rotates clockwise.

Given:

| J K L |                        | c -s 0 |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | M N O |,  R(<a href='#SkMatrix_postRotate_2_degrees'>degrees</a>, px, py) = | s  c 0 |
| P Q R |                        | 0  0 1 |

where

c  = cos(<a href='#SkMatrix_postRotate_2_degrees'>degrees</a>)
s  = sin(<a href='#SkMatrix_postRotate_2_degrees'>degrees</a>)

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to:

| c -s dx | | J K L |   | cJ-sM cK-sN cL-sO |
R(<a href='#SkMatrix_postRotate_2_degrees'>degrees</a>, px, py) * <a href='SkMatrix_Reference#Matrix'>Matrix</a> = | s  c dy | | M N O | = | sJ+cM sK+cN sL+cO |
| 0  0  1 | | P Q R |   |     P     Q     R |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_postRotate_2_degrees'><code><strong>degrees</strong></code></a></td>
    <td>angle of axes relative to upright axes</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_postRotate_2"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_preRotate'>preRotate</a> <a href='#SkMatrix_setRotate'>setRotate</a>

<a name='SkMatrix_postSkew'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_postSkew'>postSkew</a>(<a href='undocumented#SkScalar'>SkScalar</a> kx, <a href='undocumented#SkScalar'>SkScalar</a> ky, <a href='undocumented#SkScalar'>SkScalar</a> px, <a href='undocumented#SkScalar'>SkScalar</a> py)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> constructed from skewing by (<a href='#SkMatrix_postSkew_kx'>kx</a>, <a href='#SkMatrix_postSkew_ky'>ky</a>) about pivot <a href='SkPoint_Reference#Point'>point</a>
(<a href='#SkMatrix_postSkew_px'>px</a>, <a href='#SkMatrix_postSkew_py'>py</a>), multiplied by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
This can be thought of as skewing about a pivot <a href='SkPoint_Reference#Point'>point</a> after applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Given:

| J K L |                       |  1 <a href='#SkMatrix_postSkew_kx'>kx</a> dx |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | M N O |,  K(<a href='#SkMatrix_postSkew_kx'>kx</a>, <a href='#SkMatrix_postSkew_ky'>ky</a>, <a href='#SkMatrix_postSkew_px'>px</a>, <a href='#SkMatrix_postSkew_py'>py</a>) = | <a href='#SkMatrix_postSkew_ky'>ky</a>  1 dy |
| P Q R |                       |  0  0  1 |

where

dx = -<a href='#SkMatrix_postSkew_kx'>kx</a> * <a href='#SkMatrix_postSkew_py'>py</a>
dy = -<a href='#SkMatrix_postSkew_ky'>ky</a> * <a href='#SkMatrix_postSkew_px'>px</a>

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to:

| 1 <a href='#SkMatrix_postSkew_kx'>kx</a> dx| |J K L|   |J+<a href='#SkMatrix_postSkew_kx'>kx</a>*M+dx*P K+<a href='#SkMatrix_postSkew_kx'>kx</a>*N+dx*Q L+<a href='#SkMatrix_postSkew_kx'>kx</a>*O+dx+R|
K(<a href='#SkMatrix_postSkew_kx'>kx</a>, <a href='#SkMatrix_postSkew_ky'>ky</a>, <a href='#SkMatrix_postSkew_px'>px</a>, <a href='#SkMatrix_postSkew_py'>py</a>) * <a href='SkMatrix_Reference#Matrix'>Matrix</a> = |<a href='#SkMatrix_postSkew_ky'>ky</a>  1 dy| |M N O| = |<a href='#SkMatrix_postSkew_ky'>ky</a>*J+M+dy*P <a href='#SkMatrix_postSkew_ky'>ky</a>*K+N+dy*Q <a href='#SkMatrix_postSkew_ky'>ky</a>*L+O+dy*R|
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

<div><fiddle-embed name="@Matrix_postSkew"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_preSkew'>preSkew</a> <a href='#SkMatrix_setSkew'>setSkew</a>

<a name='SkMatrix_postSkew_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_postSkew'>postSkew</a>(<a href='undocumented#SkScalar'>SkScalar</a> kx, <a href='undocumented#SkScalar'>SkScalar</a> ky)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> constructed from skewing by (<a href='#SkMatrix_postSkew_2_kx'>kx</a>, <a href='#SkMatrix_postSkew_2_ky'>ky</a>) about pivot <a href='SkPoint_Reference#Point'>point</a>
(0, 0), multiplied by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
This can be thought of as skewing about the origin after applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Given:

| J K L |               |  1 <a href='#SkMatrix_postSkew_2_kx'>kx</a> 0 |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | M N O |,  K(<a href='#SkMatrix_postSkew_2_kx'>kx</a>, <a href='#SkMatrix_postSkew_2_ky'>ky</a>) = | <a href='#SkMatrix_postSkew_2_ky'>ky</a>  1 0 |
| P Q R |               |  0  0 1 |

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to:

|  1 <a href='#SkMatrix_postSkew_2_kx'>kx</a> 0 | | J K L |   | J+<a href='#SkMatrix_postSkew_2_kx'>kx</a>*M K+<a href='#SkMatrix_postSkew_2_kx'>kx</a>*N L+<a href='#SkMatrix_postSkew_2_kx'>kx</a>*O |
K(<a href='#SkMatrix_postSkew_2_kx'>kx</a>, <a href='#SkMatrix_postSkew_2_ky'>ky</a>) * <a href='SkMatrix_Reference#Matrix'>Matrix</a> = | <a href='#SkMatrix_postSkew_2_ky'>ky</a>  1 0 | | M N O | = | <a href='#SkMatrix_postSkew_2_ky'>ky</a>*J+M <a href='#SkMatrix_postSkew_2_ky'>ky</a>*K+N <a href='#SkMatrix_postSkew_2_ky'>ky</a>*L+O |
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

<div><fiddle-embed name="@Matrix_postSkew_2"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_preSkew'>preSkew</a> <a href='#SkMatrix_setSkew'>setSkew</a>

<a name='SkMatrix_postConcat'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_postConcat'>postConcat</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& other)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_postConcat_other'>other</a> multiplied by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
This can be thought of mapping by <a href='#SkMatrix_postConcat_other'>other</a> after applying <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

Given:

| J K L |           | A B C |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | M N O |,  <a href='#SkMatrix_postConcat_other'>other</a> = | D E F |
| P Q R |           | G H <a href='#SkMatrix_I'>I</a> |

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to:

| A B C |   | J K L |   | AJ+BM+CP AK+BN+CQ AL+BO+CR |
<a href='#SkMatrix_postConcat_other'>other</a> * <a href='SkMatrix_Reference#Matrix'>Matrix</a> = | D E F | * | M N O | = | DJ+EM+FP DK+EN+FQ DL+EO+FR |
| G H <a href='#SkMatrix_I'>I</a> |   | P Q R |   | GJ+HM+IP GK+HN+IQ GL+HO+IR |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_postConcat_other'><code><strong>other</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> on  left side of multiply expression</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_postConcat"></fiddle-embed></div>

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

<a href='#SkMatrix_ScaleToFit'>ScaleToFit</a> describes how <a href='SkMatrix_Reference#Matrix'>Matrix</a> is constructed to map one <a href='SkRect_Reference#Rect'>Rect</a> to another.
<a href='#SkMatrix_ScaleToFit'>ScaleToFit</a> may allow <a href='SkMatrix_Reference#Matrix'>Matrix</a> to have unequal horizontal and vertical scaling,
or may restrict <a href='SkMatrix_Reference#Matrix'>Matrix</a> to square scaling. If restricted, <a href='#SkMatrix_ScaleToFit'>ScaleToFit</a> specifies
how <a href='SkMatrix_Reference#Matrix'>Matrix</a> maps to the side or center of the destination <a href='SkRect_Reference#Rect'>Rect</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kFill_ScaleToFit'><code>SkMatrix::kFill_ScaleToFit</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>0</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Computes <a href='SkMatrix_Reference#Matrix'>Matrix</a> that scales about x-axis and y-axis independently, so that
source <a href='SkRect_Reference#Rect'>Rect</a> is mapped to completely fill destination <a href='SkRect_Reference#Rect'>Rect</a>. The aspect ratio
of source <a href='SkRect_Reference#Rect'>Rect</a> may change.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kStart_ScaleToFit'><code>SkMatrix::kStart_ScaleToFit</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Computes <a href='SkMatrix_Reference#Matrix'>Matrix</a> that maintains source <a href='SkRect_Reference#Rect'>Rect</a> aspect ratio, mapping source <a href='SkRect_Reference#Rect'>Rect</a>
width or height to destination <a href='SkRect_Reference#Rect'>Rect</a>. Aligns mapping to left and top edges
of destination <a href='SkRect_Reference#Rect'>Rect</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kCenter_ScaleToFit'><code>SkMatrix::kCenter_ScaleToFit</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Computes <a href='SkMatrix_Reference#Matrix'>Matrix</a> that maintains source <a href='SkRect_Reference#Rect'>Rect</a> aspect ratio, mapping source <a href='SkRect_Reference#Rect'>Rect</a>
width or height to destination <a href='SkRect_Reference#Rect'>Rect</a>. Aligns mapping to center of destination
<a href='SkRect_Reference#Rect'>Rect</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkMatrix_kEnd_ScaleToFit'><code>SkMatrix::kEnd_ScaleToFit</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Computes <a href='SkMatrix_Reference#Matrix'>Matrix</a> that maintains source <a href='SkRect_Reference#Rect'>Rect</a> aspect ratio, mapping source <a href='SkRect_Reference#Rect'>Rect</a>
width or height to destination <a href='SkRect_Reference#Rect'>Rect</a>. Aligns mapping to right and bottom
edges of destination <a href='SkRect_Reference#Rect'>Rect</a>.
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
bool <a href='#SkMatrix_setRectToRect'>setRectToRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& src, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst, <a href='#SkMatrix_ScaleToFit'>ScaleToFit</a> stf)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to scale and translate <a href='#SkMatrix_setRectToRect_src'>src</a> <a href='SkRect_Reference#SkRect'>SkRect</a> to <a href='#SkMatrix_setRectToRect_dst'>dst</a> <a href='SkRect_Reference#SkRect'>SkRect</a>. <a href='#SkMatrix_setRectToRect_stf'>stf</a> selects whether
mapping completely fills <a href='#SkMatrix_setRectToRect_dst'>dst</a> or preserves the aspect ratio, and how to align
<a href='#SkMatrix_setRectToRect_src'>src</a> within <a href='#SkMatrix_setRectToRect_dst'>dst</a>. Returns false if <a href='#SkMatrix_setRectToRect_src'>src</a> is empty, and sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to identity.
Returns true if <a href='#SkMatrix_setRectToRect_dst'>dst</a> is empty, and sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to:

| 0 0 0 |
| 0 0 0 |
| 0 0 1 |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setRectToRect_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> to map from</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setRectToRect_dst'><code><strong>dst</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> to map to</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setRectToRect_stf'><code><strong>stf</strong></code></a></td>
    <td>one of: <a href='#SkMatrix_kFill_ScaleToFit'>kFill_ScaleToFit</a>, <a href='#SkMatrix_kStart_ScaleToFit'>kStart_ScaleToFit</a>,</td>
  </tr>
</table>

<a href='#SkMatrix_kCenter_ScaleToFit'>kCenter_ScaleToFit</a>, <a href='#SkMatrix_kEnd_ScaleToFit'>kEnd_ScaleToFit</a>

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> can represent <a href='SkRect_Reference#SkRect'>SkRect</a> mapping

### Example

<div><fiddle-embed name="@Matrix_setRectToRect">

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
static <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_MakeRectToRect'>MakeRectToRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& src, const <a href='SkRect_Reference#SkRect'>SkRect</a>& dst, <a href='#SkMatrix_ScaleToFit'>ScaleToFit</a> stf)
</pre>

Returns <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> set to scale and translate <a href='#SkMatrix_MakeRectToRect_src'>src</a> <a href='SkRect_Reference#SkRect'>SkRect</a> to <a href='#SkMatrix_MakeRectToRect_dst'>dst</a> <a href='SkRect_Reference#SkRect'>SkRect</a>. <a href='#SkMatrix_MakeRectToRect_stf'>stf</a> selects
whether mapping completely fills <a href='#SkMatrix_MakeRectToRect_dst'>dst</a> or preserves the aspect ratio, and how to
align <a href='#SkMatrix_MakeRectToRect_src'>src</a> within <a href='#SkMatrix_MakeRectToRect_dst'>dst</a>. Returns the identity <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> if <a href='#SkMatrix_MakeRectToRect_src'>src</a> is empty. If <a href='#SkMatrix_MakeRectToRect_dst'>dst</a> is
empty, returns <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> set to:

| 0 0 0 |
| 0 0 0 |
| 0 0 1 |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_MakeRectToRect_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> to map from</td>
  </tr>
  <tr>    <td><a name='SkMatrix_MakeRectToRect_dst'><code><strong>dst</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> to map to</td>
  </tr>
  <tr>    <td><a name='SkMatrix_MakeRectToRect_stf'><code><strong>stf</strong></code></a></td>
    <td>one of: <a href='#SkMatrix_kFill_ScaleToFit'>kFill_ScaleToFit</a>, <a href='#SkMatrix_kStart_ScaleToFit'>kStart_ScaleToFit</a>,</td>
  </tr>
</table>

<a href='#SkMatrix_kCenter_ScaleToFit'>kCenter_ScaleToFit</a>, <a href='#SkMatrix_kEnd_ScaleToFit'>kEnd_ScaleToFit</a>

### Return Value

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> mapping <a href='#SkMatrix_MakeRectToRect_src'>src</a> to <a href='#SkMatrix_MakeRectToRect_dst'>dst</a>

### Example

<div><fiddle-embed name="@Matrix_MakeRectToRect">

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
bool <a href='#SkMatrix_setPolyToPoly'>setPolyToPoly</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> src[], const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> dst[], int count)
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to map <a href='#SkMatrix_setPolyToPoly_src'>src</a> to <a href='#SkMatrix_setPolyToPoly_dst'>dst</a>. <a href='#SkMatrix_setPolyToPoly_count'>count</a> must be zero or greater, and four or less.

If <a href='#SkMatrix_setPolyToPoly_count'>count</a> is zero, sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to identity and returns true.
If <a href='#SkMatrix_setPolyToPoly_count'>count</a> is one, sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to translate and returns true.
If <a href='#SkMatrix_setPolyToPoly_count'>count</a> is two or more, sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to map <a href='SkPoint_Reference#SkPoint'>SkPoint</a> if possible; returns false
if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> cannot be constructed. If <a href='#SkMatrix_setPolyToPoly_count'>count</a> is four, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> may include
perspective.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setPolyToPoly_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkPoint_Reference#SkPoint'>SkPoint</a> to map from</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setPolyToPoly_dst'><code><strong>dst</strong></code></a></td>
    <td><a href='SkPoint_Reference#SkPoint'>SkPoint</a> to map to</td>
  </tr>
  <tr>    <td><a name='SkMatrix_setPolyToPoly_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='SkPoint_Reference#SkPoint'>SkPoint</a> in <a href='#SkMatrix_setPolyToPoly_src'>src</a> and <a href='#SkMatrix_setPolyToPoly_dst'>dst</a></td>
  </tr>
</table>

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> was constructed successfully

### Example

<div><fiddle-embed name="c851d1313e8909aaea4f0591699fdb7b"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_setRectToRect'>setRectToRect</a> <a href='#SkMatrix_MakeRectToRect'>MakeRectToRect</a>

<a name='SkMatrix_invert'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_invert'>invert</a>(<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* inverse)const
</pre>

Sets <a href='#SkMatrix_invert_inverse'>inverse</a> to reciprocal <a href='SkMatrix_Reference#Matrix'>matrix</a>, returning true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> can be inverted.
Geometrically, if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> maps from source to destination, <a href='#SkMatrix_invert_inverse'>inverse</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
maps from destination to source. If <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> can not be inverted, <a href='#SkMatrix_invert_inverse'>inverse</a> is
unchanged.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_invert_inverse'><code><strong>inverse</strong></code></a></td>
    <td>storage for inverted <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> can be inverted

### Example

<div><fiddle-embed name="@Matrix_invert"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_Concat'>Concat</a>

<a name='SkMatrix_SetAffineIdentity'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static void <a href='#SkMatrix_SetAffineIdentity'>SetAffineIdentity</a>(<a href='undocumented#SkScalar'>SkScalar</a> affine[6])
</pre>

Fills <a href='#SkMatrix_SetAffineIdentity_affine'>affine</a> with identity values in column major order.
Sets <a href='#SkMatrix_SetAffineIdentity_affine'>affine</a> to:

| 1 0 0 |
| 0 1 0 |

Affine 3 by 2 <a href='SkMatrix_Reference#Matrix'>matrices</a> in column major order are used by OpenGL and XPS.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_SetAffineIdentity_affine'><code><strong>affine</strong></code></a></td>
    <td>storage for 3 by 2 <a href='#SkMatrix_SetAffineIdentity_affine'>affine</a> <a href='SkMatrix_Reference#Matrix'>matrix</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_SetAffineIdentity">

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
bool <a href='#SkMatrix_asAffine'>asAffine</a>(<a href='undocumented#SkScalar'>SkScalar</a> affine[6])const
</pre>

Fills <a href='#SkMatrix_asAffine_affine'>affine</a> in column major order. Sets <a href='#SkMatrix_asAffine_affine'>affine</a> to:

| scale-x  skew-x translate-x |
| skew-y  scale-y translate-y |

If <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> contains perspective, returns false and leaves <a href='#SkMatrix_asAffine_affine'>affine</a> unchanged.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_asAffine_affine'><code><strong>affine</strong></code></a></td>
    <td>storage for 3 by 2 <a href='#SkMatrix_asAffine_affine'>affine</a> <a href='SkMatrix_Reference#Matrix'>matrix</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> does not contain perspective

### Example

<div><fiddle-embed name="@Matrix_asAffine">

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
void <a href='#SkMatrix_setAffine'>setAffine</a>(const <a href='undocumented#SkScalar'>SkScalar</a> affine[6])
</pre>

Sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to <a href='#SkMatrix_setAffine_affine'>affine</a> values, passed in column major order. Given <a href='#SkMatrix_setAffine_affine'>affine</a>,
column, then row, as:

| scale-x  skew-x translate-x |
|  skew-y scale-y translate-y |

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> is set, row, then column, to:

| scale-x  skew-x translate-x |
|  skew-y scale-y translate-y |
|       0       0           1 |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_setAffine_affine'><code><strong>affine</strong></code></a></td>
    <td>3 by 2 <a href='#SkMatrix_setAffine_affine'>affine</a> <a href='SkMatrix_Reference#Matrix'>matrix</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_setAffine">

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
void <a href='#SkMatrix_mapPoints'>mapPoints</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> dst[], const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> src[], int count)const
</pre>

Maps <a href='#SkMatrix_mapPoints_src'>src</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> of length <a href='#SkMatrix_mapPoints_count'>count</a> to <a href='#SkMatrix_mapPoints_dst'>dst</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> of equal or greater
length. <a href='SkPoint_Reference#SkPoint'>SkPoint</a> are mapped by multiplying each <a href='SkPoint_Reference#SkPoint'>SkPoint</a> by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. Given:

| A B C |        | x |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | D E F |,  pt = | y |
| G H <a href='#SkMatrix_I'>I</a> |        | 1 |

where

for (i = 0; i < <a href='#SkMatrix_mapPoints_count'>count</a>; ++i) {
x = <a href='#SkMatrix_mapPoints_src'>src</a>[i].fX
y = <a href='#SkMatrix_mapPoints_src'>src</a>[i].fY
}

each <a href='#SkMatrix_mapPoints_dst'>dst</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> is computed as:

|A B C| |x|                               Ax+By+C   Dx+Ey+F
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * pt = |D E F| |y| = |Ax+By+C Dx+Ey+F Gx+Hy+<a href='#SkMatrix_I'>I</a>| = ------- , -------
|G H <a href='#SkMatrix_I'>I</a>| |1|                               Gx+Hy+<a href='#SkMatrix_I'>I</a>   Gx+Hy+<a href='#SkMatrix_I'>I</a>

<a href='#SkMatrix_mapPoints_src'>src</a> and <a href='#SkMatrix_mapPoints_dst'>dst</a> may <a href='SkPoint_Reference#Point'>point</a> to the same storage.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapPoints_dst'><code><strong>dst</strong></code></a></td>
    <td>storage for mapped <a href='SkPoint_Reference#SkPoint'>SkPoint</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapPoints_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkPoint_Reference#SkPoint'>SkPoint</a> to transform</td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapPoints_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='SkPoint_Reference#SkPoint'>SkPoint</a> to transform</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_mapPoints"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapXY'>mapXY</a> <a href='#SkMatrix_mapHomogeneousPoints'>mapHomogeneousPoints</a> <a href='#SkMatrix_mapVectors'>mapVectors</a>

<a name='SkMatrix_mapPoints_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_mapPoints'>mapPoints</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> pts[], int count)const
</pre>

Maps <a href='#SkMatrix_mapPoints_2_pts'>pts</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> of length <a href='#SkMatrix_mapPoints_2_count'>count</a> in place. <a href='SkPoint_Reference#SkPoint'>SkPoint</a> are mapped by multiplying
each <a href='SkPoint_Reference#SkPoint'>SkPoint</a> by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. Given:

| A B C |        | x |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | D E F |,  pt = | y |
| G H <a href='#SkMatrix_I'>I</a> |        | 1 |

where

for (i = 0; i < <a href='#SkMatrix_mapPoints_2_count'>count</a>; ++i) {
x = <a href='#SkMatrix_mapPoints_2_pts'>pts</a>[i].fX
y = <a href='#SkMatrix_mapPoints_2_pts'>pts</a>[i].fY
}

each resulting <a href='#SkMatrix_mapPoints_2_pts'>pts</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> is computed as:

|A B C| |x|                               Ax+By+C   Dx+Ey+F
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * pt = |D E F| |y| = |Ax+By+C Dx+Ey+F Gx+Hy+<a href='#SkMatrix_I'>I</a>| = ------- , -------
|G H <a href='#SkMatrix_I'>I</a>| |1|                               Gx+Hy+<a href='#SkMatrix_I'>I</a>   Gx+Hy+<a href='#SkMatrix_I'>I</a>

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapPoints_2_pts'><code><strong>pts</strong></code></a></td>
    <td>storage for mapped <a href='SkPoint_Reference#SkPoint'>SkPoint</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapPoints_2_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='SkPoint_Reference#SkPoint'>SkPoint</a> to transform</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_mapPoints_2"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapXY'>mapXY</a> <a href='#SkMatrix_mapHomogeneousPoints'>mapHomogeneousPoints</a> <a href='#SkMatrix_mapVectors'>mapVectors</a>

<a name='SkMatrix_mapHomogeneousPoints'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_mapHomogeneousPoints'>mapHomogeneousPoints</a>(<a href='undocumented#SkPoint3'>SkPoint3</a> dst[], const <a href='undocumented#SkPoint3'>SkPoint3</a> src[], int count)const
</pre>

Maps <a href='#SkMatrix_mapHomogeneousPoints_src'>src</a> <a href='undocumented#SkPoint3'>SkPoint3</a> array of length <a href='#SkMatrix_mapHomogeneousPoints_count'>count</a> to <a href='#SkMatrix_mapHomogeneousPoints_dst'>dst</a> <a href='undocumented#SkPoint3'>SkPoint3</a> array, which must of length <a href='#SkMatrix_mapHomogeneousPoints_count'>count</a> or
greater. <a href='undocumented#SkPoint3'>SkPoint3</a> array is mapped by multiplying each <a href='undocumented#SkPoint3'>SkPoint3</a> by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. Given:

| A B C |         | x |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | D E F |,  <a href='#SkMatrix_mapHomogeneousPoints_src'>src</a> = | y |
| G H <a href='#SkMatrix_I'>I</a> |         | z |

each resulting <a href='#SkMatrix_mapHomogeneousPoints_dst'>dst</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> is computed as:

|A B C| |x|
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * <a href='#SkMatrix_mapHomogeneousPoints_src'>src</a> = |D E F| |y| = |Ax+By+Cz Dx+Ey+Fz Gx+Hy+Iz|
|G H <a href='#SkMatrix_I'>I</a>| |z|

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapHomogeneousPoints_dst'><code><strong>dst</strong></code></a></td>
    <td>storage for mapped <a href='undocumented#SkPoint3'>SkPoint3</a> array</td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapHomogeneousPoints_src'><code><strong>src</strong></code></a></td>
    <td><a href='undocumented#SkPoint3'>SkPoint3</a> array to transform</td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapHomogeneousPoints_count'><code><strong>count</strong></code></a></td>
    <td>items in <a href='undocumented#SkPoint3'>SkPoint3</a> array to transform</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_mapHomogeneousPoints"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapPoints'>mapPoints</a> <a href='#SkMatrix_mapXY'>mapXY</a> <a href='#SkMatrix_mapVectors'>mapVectors</a>

<a name='SkMatrix_mapXY'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_mapXY'>mapXY</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, <a href='SkPoint_Reference#SkPoint'>SkPoint</a>* result)const
</pre>

Maps <a href='SkPoint_Reference#SkPoint'>SkPoint</a> (<a href='#SkMatrix_mapXY_x'>x</a>, <a href='#SkMatrix_mapXY_y'>y</a>) to <a href='#SkMatrix_mapXY_result'>result</a>. <a href='SkPoint_Reference#SkPoint'>SkPoint</a> is mapped by multiplying by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. Given:

| A B C |        | <a href='#SkMatrix_mapXY_x'>x</a> |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | D E F |,  pt = | <a href='#SkMatrix_mapXY_y'>y</a> |
| G H <a href='#SkMatrix_I'>I</a> |        | 1 |

<a href='#SkMatrix_mapXY_result'>result</a> is computed as:

|A B C| |<a href='#SkMatrix_mapXY_x'>x</a>|                               Ax+By+C   Dx+Ey+F
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * pt = |D E F| |<a href='#SkMatrix_mapXY_y'>y</a>| = |Ax+By+C Dx+Ey+F Gx+Hy+<a href='#SkMatrix_I'>I</a>| = ------- , -------
|G H <a href='#SkMatrix_I'>I</a>| |1|                               Gx+Hy+<a href='#SkMatrix_I'>I</a>   Gx+Hy+<a href='#SkMatrix_I'>I</a>

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapXY_x'><code><strong>x</strong></code></a></td>
    <td>x-axis value of <a href='SkPoint_Reference#SkPoint'>SkPoint</a> to map</td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapXY_y'><code><strong>y</strong></code></a></td>
    <td>y-axis value of <a href='SkPoint_Reference#SkPoint'>SkPoint</a> to map</td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapXY_result'><code><strong>result</strong></code></a></td>
    <td>storage for mapped <a href='SkPoint_Reference#SkPoint'>SkPoint</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_mapXY"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapPoints'>mapPoints</a> <a href='#SkMatrix_mapVectors'>mapVectors</a>

<a name='SkMatrix_mapXY_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkMatrix_mapXY'>mapXY</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y)const
</pre>

Returns <a href='SkPoint_Reference#SkPoint'>SkPoint</a> (<a href='#SkMatrix_mapXY_2_x'>x</a>, <a href='#SkMatrix_mapXY_2_y'>y</a>) multiplied by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. Given:

| A B C |        | <a href='#SkMatrix_mapXY_2_x'>x</a> |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | D E F |,  pt = | <a href='#SkMatrix_mapXY_2_y'>y</a> |
| G H <a href='#SkMatrix_I'>I</a> |        | 1 |

result is computed as:

|A B C| |<a href='#SkMatrix_mapXY_2_x'>x</a>|                               Ax+By+C   Dx+Ey+F
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * pt = |D E F| |<a href='#SkMatrix_mapXY_2_y'>y</a>| = |Ax+By+C Dx+Ey+F Gx+Hy+<a href='#SkMatrix_I'>I</a>| = ------- , -------
|G H <a href='#SkMatrix_I'>I</a>| |1|                               Gx+Hy+<a href='#SkMatrix_I'>I</a>   Gx+Hy+<a href='#SkMatrix_I'>I</a>

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapXY_2_x'><code><strong>x</strong></code></a></td>
    <td>x-axis value of <a href='SkPoint_Reference#SkPoint'>SkPoint</a> to map</td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapXY_2_y'><code><strong>y</strong></code></a></td>
    <td>y-axis value of <a href='SkPoint_Reference#SkPoint'>SkPoint</a> to map</td>
  </tr>
</table>

### Return Value

mapped <a href='SkPoint_Reference#SkPoint'>SkPoint</a>

### Example

<div><fiddle-embed name="@Matrix_mapXY_2"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapPoints'>mapPoints</a> <a href='#SkMatrix_mapVectors'>mapVectors</a>

<a name='SkMatrix_mapVectors'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_mapVectors'>mapVectors</a>(<a href='SkPoint_Reference#SkVector'>SkVector</a> dst[], const <a href='SkPoint_Reference#SkVector'>SkVector</a> src[], int count)const
</pre>

Maps <a href='#SkMatrix_mapVectors_src'>src</a> <a href='SkPoint_Reference#Vector'>vector</a> array of length <a href='#SkMatrix_mapVectors_count'>count</a> to <a href='SkPoint_Reference#Vector'>vector</a>  <a href='SkPath_Reference#Point_Array'>SkPoint array</a> of equal or greater
length. <a href='SkPoint_Reference#Vector'>Vectors</a> are mapped by multiplying each <a href='SkPoint_Reference#Vector'>vector</a> by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, treating
<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> translation as zero. Given:

| A B 0 |         | x |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | D E 0 |,  <a href='#SkMatrix_mapVectors_src'>src</a> = | y |
| G H <a href='#SkMatrix_I'>I</a> |         | 1 |

where

for (i = 0; i < <a href='#SkMatrix_mapVectors_count'>count</a>; ++i) {
x = <a href='#SkMatrix_mapVectors_src'>src</a>[i].fX
y = <a href='#SkMatrix_mapVectors_src'>src</a>[i].fY
}

each <a href='#SkMatrix_mapVectors_dst'>dst</a> <a href='SkPoint_Reference#Vector'>vector</a> is computed as:

|A B 0| |x|                            Ax+By     Dx+Ey
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * <a href='#SkMatrix_mapVectors_src'>src</a> = |D E 0| |y| = |Ax+By Dx+Ey Gx+Hy+<a href='#SkMatrix_I'>I</a>| = ------- , -------
|G H <a href='#SkMatrix_I'>I</a>| |1|                           Gx+Hy+<a href='#SkMatrix_I'>I</a>   Gx+Hy+<a href='#SkMatrix_I'>I</a>

<a href='#SkMatrix_mapVectors_src'>src</a> and <a href='#SkMatrix_mapVectors_dst'>dst</a> may <a href='SkPoint_Reference#Point'>point</a> to the same storage.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapVectors_dst'><code><strong>dst</strong></code></a></td>
    <td>storage for mapped <a href='SkPoint_Reference#Vector'>vectors</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapVectors_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkPoint_Reference#Vector'>vectors</a> to transform</td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapVectors_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='SkPoint_Reference#Vector'>vectors</a> to transform</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_mapVectors"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapVector'>mapVector</a> <a href='#SkMatrix_mapPoints'>mapPoints</a> <a href='#SkMatrix_mapXY'>mapXY</a>

<a name='SkMatrix_mapVectors_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_mapVectors'>mapVectors</a>(<a href='SkPoint_Reference#SkVector'>SkVector</a> vecs[], int count)const
</pre>

Maps <a href='#SkMatrix_mapVectors_2_vecs'>vecs</a> <a href='SkPoint_Reference#Vector'>vector</a> array of length <a href='#SkMatrix_mapVectors_2_count'>count</a> in place, multiplying each <a href='SkPoint_Reference#Vector'>vector</a> by
<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, treating <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> translation as zero. Given:

| A B 0 |         | x |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | D E 0 |,  vec = | y |
| G H <a href='#SkMatrix_I'>I</a> |         | 1 |

where

for (i = 0; i < <a href='#SkMatrix_mapVectors_2_count'>count</a>; ++i) {
x = <a href='#SkMatrix_mapVectors_2_vecs'>vecs</a>[i].fX
y = <a href='#SkMatrix_mapVectors_2_vecs'>vecs</a>[i].fY
}

each result <a href='SkPoint_Reference#Vector'>vector</a> is computed as:

|A B 0| |x|                            Ax+By     Dx+Ey
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * vec = |D E 0| |y| = |Ax+By Dx+Ey Gx+Hy+<a href='#SkMatrix_I'>I</a>| = ------- , -------
|G H <a href='#SkMatrix_I'>I</a>| |1|                           Gx+Hy+<a href='#SkMatrix_I'>I</a>   Gx+Hy+<a href='#SkMatrix_I'>I</a>

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapVectors_2_vecs'><code><strong>vecs</strong></code></a></td>
    <td><a href='SkPoint_Reference#Vector'>vectors</a> to transform, and storage for mapped <a href='SkPoint_Reference#Vector'>vectors</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapVectors_2_count'><code><strong>count</strong></code></a></td>
    <td>number of <a href='SkPoint_Reference#Vector'>vectors</a> to transform</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_mapVectors_2"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapVector'>mapVector</a> <a href='#SkMatrix_mapPoints'>mapPoints</a> <a href='#SkMatrix_mapXY'>mapXY</a>

<a name='SkMatrix_mapVector'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_mapVector'>mapVector</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy, <a href='SkPoint_Reference#SkVector'>SkVector</a>* result)const
</pre>

Maps <a href='SkPoint_Reference#Vector'>vector</a> (<a href='#SkMatrix_mapVector_dx'>dx</a>, <a href='#SkMatrix_mapVector_dy'>dy</a>) to <a href='#SkMatrix_mapVector_result'>result</a>. <a href='SkPoint_Reference#Vector'>Vector</a> is mapped by multiplying by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>,
treating <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> translation as zero. Given:

| A B 0 |         | <a href='#SkMatrix_mapVector_dx'>dx</a> |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | D E 0 |,  vec = | <a href='#SkMatrix_mapVector_dy'>dy</a> |
| G H <a href='#SkMatrix_I'>I</a> |         |  1 |

each <a href='#SkMatrix_mapVector_result'>result</a> <a href='SkPoint_Reference#Vector'>vector</a> is computed as:

|A B 0| |<a href='#SkMatrix_mapVector_dx'>dx</a>|                                        A*<a href='#SkMatrix_mapVector_dx'>dx</a>+B*<a href='#SkMatrix_mapVector_dy'>dy</a>     D*<a href='#SkMatrix_mapVector_dx'>dx</a>+E*<a href='#SkMatrix_mapVector_dy'>dy</a>
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * vec = |D E 0| |<a href='#SkMatrix_mapVector_dy'>dy</a>| = |A*<a href='#SkMatrix_mapVector_dx'>dx</a>+B*<a href='#SkMatrix_mapVector_dy'>dy</a> D*<a href='#SkMatrix_mapVector_dx'>dx</a>+E*<a href='#SkMatrix_mapVector_dy'>dy</a> G*<a href='#SkMatrix_mapVector_dx'>dx</a>+H*<a href='#SkMatrix_mapVector_dy'>dy</a>+<a href='#SkMatrix_I'>I</a>| = ----------- , -----------
|G H <a href='#SkMatrix_I'>I</a>| | 1|                                       G*<a href='#SkMatrix_mapVector_dx'>dx</a>+H*<a href='#SkMatrix_mapVector_dy'>dy</a>+<a href='#SkMatrix_I'>I</a>   G*<a href='#SkMatrix_mapVector_dx'>dx</a>+*dHy+<a href='#SkMatrix_I'>I</a>

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapVector_dx'><code><strong>dx</strong></code></a></td>
    <td>x-axis value of <a href='SkPoint_Reference#Vector'>vector</a> to map</td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapVector_dy'><code><strong>dy</strong></code></a></td>
    <td>y-axis value of <a href='SkPoint_Reference#Vector'>vector</a> to map</td>
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
<a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='#SkMatrix_mapVector'>mapVector</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy)const
</pre>

Returns <a href='SkPoint_Reference#Vector'>vector</a> (<a href='#SkMatrix_mapVector_2_dx'>dx</a>, <a href='#SkMatrix_mapVector_2_dy'>dy</a>) multiplied by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>, treating <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> translation as zero.
Given:

| A B 0 |         | <a href='#SkMatrix_mapVector_2_dx'>dx</a> |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | D E 0 |,  vec = | <a href='#SkMatrix_mapVector_2_dy'>dy</a> |
| G H <a href='#SkMatrix_I'>I</a> |         |  1 |

each result <a href='SkPoint_Reference#Vector'>vector</a> is computed as:

|A B 0| |<a href='#SkMatrix_mapVector_2_dx'>dx</a>|                                        A*<a href='#SkMatrix_mapVector_2_dx'>dx</a>+B*<a href='#SkMatrix_mapVector_2_dy'>dy</a>     D*<a href='#SkMatrix_mapVector_2_dx'>dx</a>+E*<a href='#SkMatrix_mapVector_2_dy'>dy</a>
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * vec = |D E 0| |<a href='#SkMatrix_mapVector_2_dy'>dy</a>| = |A*<a href='#SkMatrix_mapVector_2_dx'>dx</a>+B*<a href='#SkMatrix_mapVector_2_dy'>dy</a> D*<a href='#SkMatrix_mapVector_2_dx'>dx</a>+E*<a href='#SkMatrix_mapVector_2_dy'>dy</a> G*<a href='#SkMatrix_mapVector_2_dx'>dx</a>+H*<a href='#SkMatrix_mapVector_2_dy'>dy</a>+<a href='#SkMatrix_I'>I</a>| = ----------- , -----------
|G H <a href='#SkMatrix_I'>I</a>| | 1|                                       G*<a href='#SkMatrix_mapVector_2_dx'>dx</a>+H*<a href='#SkMatrix_mapVector_2_dy'>dy</a>+<a href='#SkMatrix_I'>I</a>   G*<a href='#SkMatrix_mapVector_2_dx'>dx</a>+*dHy+<a href='#SkMatrix_I'>I</a>

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapVector_2_dx'><code><strong>dx</strong></code></a></td>
    <td>x-axis value of <a href='SkPoint_Reference#Vector'>vector</a> to map</td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapVector_2_dy'><code><strong>dy</strong></code></a></td>
    <td>y-axis value of <a href='SkPoint_Reference#Vector'>vector</a> to map</td>
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
bool <a href='#SkMatrix_mapRect'>mapRect</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* dst, const <a href='SkRect_Reference#SkRect'>SkRect</a>& src)const
</pre>

Sets <a href='#SkMatrix_mapRect_dst'>dst</a> to bounds of <a href='#SkMatrix_mapRect_src'>src</a> corners mapped by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
Returns true if mapped corners are <a href='#SkMatrix_mapRect_dst'>dst</a> corners.

Returned value is the same as calling <a href='#SkMatrix_rectStaysRect'>rectStaysRect</a>().

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapRect_dst'><code><strong>dst</strong></code></a></td>
    <td>storage for bounds of mapped <a href='SkPoint_Reference#SkPoint'>SkPoint</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapRect_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> to map</td>
  </tr>
</table>

### Return Value

true if <a href='#SkMatrix_mapRect_dst'>dst</a> is equivalent to mapped <a href='#SkMatrix_mapRect_src'>src</a>

### Example

<div><fiddle-embed name="@Matrix_mapRect"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapPoints'>mapPoints</a> <a href='#SkMatrix_rectStaysRect'>rectStaysRect</a>

<a name='SkMatrix_mapRect_2'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_mapRect'>mapRect</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* <a href='SkRect_Reference#Rect'>rect</a>)const
</pre>

Sets <a href='#SkMatrix_mapRect_2_rect'>rect</a> to bounds of <a href='#SkMatrix_mapRect_2_rect'>rect</a> corners mapped by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
Returns true if mapped corners are computed <a href='#SkMatrix_mapRect_2_rect'>rect</a> corners.

Returned value is the same as calling <a href='#SkMatrix_rectStaysRect'>rectStaysRect</a>().

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapRect_2_rect'><code><strong>rect</strong></code></a></td>
    <td>rectangle to map, and storage for bounds of mapped corners</td>
  </tr>
</table>

### Return Value

true if result is equivalent to mapped <a href='#SkMatrix_mapRect_2_rect'>rect</a>

### Example

<div><fiddle-embed name="@Matrix_mapRect_2"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapRectScaleTranslate'>mapRectScaleTranslate</a> <a href='#SkMatrix_mapPoints'>mapPoints</a> <a href='#SkMatrix_rectStaysRect'>rectStaysRect</a>

<a name='SkMatrix_mapRect_3'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkMatrix_mapRect'>mapRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& src)const
</pre>

Returns bounds of <a href='#SkMatrix_mapRect_3_src'>src</a> corners mapped by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapRect_3_src'><code><strong>src</strong></code></a></td>
    <td>rectangle to map</td>
  </tr>
</table>

### Return Value

mapped bounds

### Example

<div><fiddle-embed name="@Matrix_mapRect_3"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapRectToQuad'>mapRectToQuad</a> <a href='#SkMatrix_mapRectScaleTranslate'>mapRectScaleTranslate</a>

<a name='SkMatrix_mapRectToQuad'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_mapRectToQuad'>mapRectToQuad</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> dst[4], const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='SkRect_Reference#Rect'>rect</a>)const
</pre>

Maps four corners of <a href='#SkMatrix_mapRectToQuad_rect'>rect</a> to <a href='#SkMatrix_mapRectToQuad_dst'>dst</a>. <a href='SkPoint_Reference#SkPoint'>SkPoint</a> are mapped by multiplying each
<a href='#SkMatrix_mapRectToQuad_rect'>rect</a> corner by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. <a href='#SkMatrix_mapRectToQuad_rect'>rect</a> corner is processed in this order:
(<a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fLeft'>fLeft</a>, <a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fTop'>fTop</a>), (<a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fRight'>fRight</a>, <a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fTop'>fTop</a>), (<a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fRight'>fRight</a>, <a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fBottom'>fBottom</a>),
(<a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fLeft'>fLeft</a>, <a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fBottom'>fBottom</a>).

<a href='#SkMatrix_mapRectToQuad_rect'>rect</a> may be empty: <a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fLeft'>fLeft</a> may be greater than or equal to <a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fRight'>fRight</a>;
<a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fTop'>fTop</a> may be greater than or equal to <a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fBottom'>fBottom</a>.

Given:

| A B C |        | x |
<a href='SkMatrix_Reference#Matrix'>Matrix</a> = | D E F |,  pt = | y |
| G H <a href='#SkMatrix_I'>I</a> |        | 1 |

where pt is initialized from each of (<a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fLeft'>fLeft</a>, <a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fTop'>fTop</a>),
(<a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fRight'>fRight</a>, <a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fTop'>fTop</a>), (<a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fRight'>fRight</a>, <a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fBottom'>fBottom</a>), (<a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fLeft'>fLeft</a>, <a href='#SkMatrix_mapRectToQuad_rect'>rect</a>.<a href='#SkRect_fBottom'>fBottom</a>),
each <a href='#SkMatrix_mapRectToQuad_dst'>dst</a> <a href='SkPoint_Reference#SkPoint'>SkPoint</a> is computed as:

|A B C| |x|                               Ax+By+C   Dx+Ey+F
<a href='SkMatrix_Reference#Matrix'>Matrix</a> * pt = |D E F| |y| = |Ax+By+C Dx+Ey+F Gx+Hy+<a href='#SkMatrix_I'>I</a>| = ------- , -------
|G H <a href='#SkMatrix_I'>I</a>| |1|                               Gx+Hy+<a href='#SkMatrix_I'>I</a>   Gx+Hy+<a href='#SkMatrix_I'>I</a>

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapRectToQuad_dst'><code><strong>dst</strong></code></a></td>
    <td>storage for mapped corner <a href='SkPoint_Reference#SkPoint'>SkPoint</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapRectToQuad_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> to map</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_mapRectToQuad"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapRect'>mapRect</a> <a href='#SkMatrix_mapRectScaleTranslate'>mapRectScaleTranslate</a>

<a name='SkMatrix_mapRectScaleTranslate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_mapRectScaleTranslate'>mapRectScaleTranslate</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* dst, const <a href='SkRect_Reference#SkRect'>SkRect</a>& src)const
</pre>

Sets <a href='#SkMatrix_mapRectScaleTranslate_dst'>dst</a> to bounds of <a href='#SkMatrix_mapRectScaleTranslate_src'>src</a> corners mapped by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. If <a href='SkMatrix_Reference#Matrix'>matrix</a> contains
elements other than scale or translate: asserts if SK_DEBUG is defined;
otherwise, results are undefined.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapRectScaleTranslate_dst'><code><strong>dst</strong></code></a></td>
    <td>storage for bounds of mapped <a href='SkPoint_Reference#SkPoint'>SkPoint</a></td>
  </tr>
  <tr>    <td><a name='SkMatrix_mapRectScaleTranslate_src'><code><strong>src</strong></code></a></td>
    <td><a href='SkRect_Reference#SkRect'>SkRect</a> to map</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="@Matrix_mapRectScaleTranslate"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_mapRect'>mapRect</a> <a href='#SkMatrix_mapRectToQuad'>mapRectToQuad</a> <a href='#SkMatrix_isScaleTranslate'>isScaleTranslate</a> <a href='#SkMatrix_rectStaysRect'>rectStaysRect</a>

<a name='SkMatrix_mapRadius'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_mapRadius'>mapRadius</a>(<a href='undocumented#SkScalar'>SkScalar</a> radius)const
</pre>

Returns geometric mean <a href='#SkMatrix_mapRadius_radius'>radius</a> of ellipse formed by constructing <a href='undocumented#Circle'>circle</a> of
<a href='undocumented#Size'>size</a> <a href='#SkMatrix_mapRadius_radius'>radius</a>, and mapping constructed <a href='undocumented#Circle'>circle</a> with <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. The result squared is
equal to the major axis length times the minor axis length.
Result is not meaningful if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> contains perspective elements.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_mapRadius_radius'><code><strong>radius</strong></code></a></td>
    <td><a href='undocumented#Circle'>circle</a> <a href='undocumented#Size'>size</a> to map</td>
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
bool <a href='#SkMatrix_isFixedStepInX'>isFixedStepInX</a>()const
</pre>

Returns true if a unit step on x-axis at some y-axis value mapped through <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
can be represented by a constant <a href='SkPoint_Reference#Vector'>vector</a>. Returns true if <a href='#SkMatrix_getType'>getType</a>() returns
<a href='#SkMatrix_kIdentity_Mask'>kIdentity_Mask</a>, or combinations of: <a href='#SkMatrix_kTranslate_Mask'>kTranslate_Mask</a>, <a href='#SkMatrix_kScale_Mask'>kScale_Mask</a>, and <a href='#SkMatrix_kAffine_Mask'>kAffine_Mask</a>.

May return true if <a href='#SkMatrix_getType'>getType</a>() returns <a href='#SkMatrix_kPerspective_Mask'>kPerspective_Mask</a>, but only when <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
does not include rotation or skewing along the y-axis.

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> does not have complex perspective

### Example

<div><fiddle-embed name="@Matrix_isFixedStepInX">

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
<a href='SkPoint_Reference#SkVector'>SkVector</a> <a href='#SkMatrix_fixedStepInX'>fixedStepInX</a>(<a href='undocumented#SkScalar'>SkScalar</a> y)const
</pre>

Returns <a href='SkPoint_Reference#Vector'>vector</a> representing a unit step on x-axis at <a href='#SkMatrix_fixedStepInX_y'>y</a> mapped through <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>.
If <a href='#SkMatrix_isFixedStepInX'>isFixedStepInX</a>() is false, returned value is undefined.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_fixedStepInX_y'><code><strong>y</strong></code></a></td>
    <td>position of <a href='undocumented#Line'>line</a> parallel to x-axis</td>
  </tr>
</table>

### Return Value

<a href='SkPoint_Reference#Vector'>vector</a> advance of mapped unit step on x-axis

### Example

<div><fiddle-embed name="@Matrix_fixedStepInX"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_isFixedStepInX'>isFixedStepInX</a> <a href='#SkMatrix_getType'>getType</a>

<a name='SkMatrix_cheapEqualTo'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_cheapEqualTo'>cheapEqualTo</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& m)const
</pre>

Returns true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> equals <a href='#SkMatrix_cheapEqualTo_m'>m</a>, using an efficient comparison.

Returns false when the sign of zero values is the different; when one
<a href='SkMatrix_Reference#Matrix'>matrix</a> has positive zero value and the other has negative zero value.

Returns true even when both <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> contain NaN.

NaN never equals any value, including itself. To improve performance, NaN values
are treated as bit patterns that are equal if their bit patterns are equal.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_cheapEqualTo_m'><code><strong>m</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href='#SkMatrix_cheapEqualTo_m'>m</a> and <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> are represented by identical bit patterns

### Example

<div><fiddle-embed name="@Matrix_cheapEqualTo">

#### Example Output

~~~~
identity: a == b a.cheapEqualTo(b): true
neg zero: a == b a.cheapEqualTo(b): false
one NaN: a != b a.cheapEqualTo(b): false
both NaN: a != b a.cheapEqualTo(b): true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_equal_operator'>operator==</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& a, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& b)

<a name='SkMatrix_equal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_equal_operator'>operator==</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& a, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& b)
</pre>

Compares <a href='#SkMatrix_equal_operator_a'>a</a> and <a href='#SkMatrix_equal_operator_b'>b</a>; returns true if <a href='#SkMatrix_equal_operator_a'>a</a> and <a href='#SkMatrix_equal_operator_b'>b</a> are numerically equal. Returns true
even if sign of zero values are different. Returns false if either <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
contains NaN, even if the other <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> also contains NaN.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_equal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to compare</td>
  </tr>
  <tr>    <td><a name='SkMatrix_equal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_equal_operator_a'>a</a> and <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_equal_operator_b'>b</a> are numerically equal

### Example

<div><fiddle-embed name="@Matrix_equal_operator">

#### Example Output

~~~~
identity: a == b a.cheapEqualTo(b): true
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_cheapEqualTo'>cheapEqualTo</a> <a href='#SkMatrix_notequal_operator'>operator!=</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='#SkMatrix_equal_operator_a'>a</a>, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='#SkMatrix_equal_operator_b'>b</a>)

<a name='SkMatrix_notequal_operator'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkMatrix_notequal_operator'>operator!=</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& a, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& b)
</pre>

Compares <a href='#SkMatrix_notequal_operator_a'>a</a> and <a href='#SkMatrix_notequal_operator_b'>b</a>; returns true if <a href='#SkMatrix_notequal_operator_a'>a</a> and <a href='#SkMatrix_notequal_operator_b'>b</a> are not numerically equal. Returns false
even if sign of zero values are different. Returns true if either <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>
contains NaN, even if the other <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> also contains NaN.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_notequal_operator_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to compare</td>
  </tr>
  <tr>    <td><a name='SkMatrix_notequal_operator_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_notequal_operator_a'>a</a> and <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_notequal_operator_b'>b</a> are numerically not equal

### Example

<div><fiddle-embed name="@Matrix_notequal_operator"></fiddle-embed></div>

### See Also

<a href='#SkMatrix_cheapEqualTo'>cheapEqualTo</a> <a href='#SkMatrix_equal_operator'>operator==</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='#SkMatrix_notequal_operator_a'>a</a>, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& <a href='#SkMatrix_notequal_operator_b'>b</a>)

<a name='Utility'></a>

<a name='SkMatrix_dump'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_dump'>dump()</a>const
</pre>

Writes <a href='undocumented#Text'>text</a> representation of <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to standard output. Floating <a href='SkPoint_Reference#Point'>point</a> values
are written with limited precision; it may not be possible to reconstruct
original <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> from output.

### Example

<div><fiddle-embed name="@Matrix_dump">

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
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getMinScale'>getMinScale</a>()const
</pre>

Returns the minimum scaling factor of <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> by decomposing the scaling and
skewing elements.
Returns -1 if scale factor overflows or <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> contains perspective.

### Return Value

minimum scale factor

### Example

<div><fiddle-embed name="@Matrix_getMinScale">

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
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkMatrix_getMaxScale'>getMaxScale</a>()const
</pre>

Returns the maximum scaling factor of <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> by decomposing the scaling and
skewing elements.
Returns -1 if scale factor overflows or <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> contains perspective.

### Return Value

maximum scale factor

### Example

<div><fiddle-embed name="@Matrix_getMaxScale">

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
bool <a href='#SkMatrix_getMinMaxScales'>getMinMaxScales</a>(<a href='undocumented#SkScalar'>SkScalar</a> scaleFactors[2])const
</pre>

Sets <a href='#SkMatrix_getMinMaxScales_scaleFactors'>scaleFactors</a>[0] to the minimum scaling factor, and <a href='#SkMatrix_getMinMaxScales_scaleFactors'>scaleFactors</a>[1] to the
maximum scaling factor. Scaling factors are computed by decomposing
the <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> scaling and skewing elements.

Returns true if <a href='#SkMatrix_getMinMaxScales_scaleFactors'>scaleFactors</a> are found; otherwise, returns false and sets
<a href='#SkMatrix_getMinMaxScales_scaleFactors'>scaleFactors</a> to undefined values.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_getMinMaxScales_scaleFactors'><code><strong>scaleFactors</strong></code></a></td>
    <td>storage for minimum and maximum scale factors</td>
  </tr>
</table>

### Return Value

true if scale factors were computed correctly

### Example

<div><fiddle-embed name="@Matrix_getMinMaxScales">

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
bool <a href='#SkMatrix_decomposeScale'>decomposeScale</a>(<a href='undocumented#SkSize'>SkSize</a>* scale, <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>* remaining = nullptr)const
</pre>

Decomposes <a href='SkMatrix_Reference#Matrix'>Matrix</a> into <a href='#SkMatrix_decomposeScale_scale'>scale</a> components and whatever remains. Returns false if
<a href='SkMatrix_Reference#Matrix'>Matrix</a> could not be decomposed.

Sets <a href='#SkMatrix_decomposeScale_scale'>scale</a> to portion of <a href='SkMatrix_Reference#Matrix'>Matrix</a> that <a href='#SkMatrix_decomposeScale_scale'>scale</a> axes. Sets <a href='#SkMatrix_decomposeScale_remaining'>remaining</a> to <a href='SkMatrix_Reference#Matrix'>Matrix</a>
with scaling factored out. <a href='#SkMatrix_decomposeScale_remaining'>remaining</a> may be passed as nullptr
to determine if <a href='SkMatrix_Reference#Matrix'>Matrix</a> can be decomposed without computing remainder.

Returns true if <a href='#SkMatrix_decomposeScale_scale'>scale</a> components are found. <a href='#SkMatrix_decomposeScale_scale'>scale</a> and <a href='#SkMatrix_decomposeScale_remaining'>remaining</a> are
unchanged if <a href='SkMatrix_Reference#Matrix'>Matrix</a> contains perspective; <a href='#SkMatrix_decomposeScale_scale'>scale</a> factors are not finite, or
are nearly zero.

On success: <code><a href='SkMatrix_Reference#Matrix'>Matrix</a> = <a href='#SkMatrix_decomposeScale_scale'>scale</a> * Remaining</code>.

### Parameters

<table>  <tr>    <td><a name='SkMatrix_decomposeScale_scale'><code><strong>scale</strong></code></a></td>
    <td>axes scaling factors; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkMatrix_decomposeScale_remaining'><code><strong>remaining</strong></code></a></td>
    <td><a href='SkMatrix_Reference#Matrix'>Matrix</a> without scaling; may be nullptr</td>
  </tr>
</table>

### Return Value

true if <a href='#SkMatrix_decomposeScale_scale'>scale</a> can be computed

### Example

<div><fiddle-embed name="@Matrix_decomposeScale">

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

Returns reference to const identity <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>. Returned <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> is set to:

| 1 0 0 |
| 0 1 0 |
| 0 0 1 |

### Return Value

const identity <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>

### Example

<div><fiddle-embed name="@Matrix_I">

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

Returns reference to a const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> with invalid values. Returned <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> is set
to:

| <a href='undocumented#SK_ScalarMax'>SK_ScalarMax</a> <a href='undocumented#SK_ScalarMax'>SK_ScalarMax</a> <a href='undocumented#SK_ScalarMax'>SK_ScalarMax</a> |
| <a href='undocumented#SK_ScalarMax'>SK_ScalarMax</a> <a href='undocumented#SK_ScalarMax'>SK_ScalarMax</a> <a href='undocumented#SK_ScalarMax'>SK_ScalarMax</a> |
| <a href='undocumented#SK_ScalarMax'>SK_ScalarMax</a> <a href='undocumented#SK_ScalarMax'>SK_ScalarMax</a> <a href='undocumented#SK_ScalarMax'>SK_ScalarMax</a> |

### Return Value

const invalid <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>

### Example

<div><fiddle-embed name="@Matrix_InvalidMatrix">

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
static <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_Concat'>Concat</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& a, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& b)
</pre>

Returns <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_Concat_a'>a</a> multiplied by <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> <a href='#SkMatrix_Concat_b'>b</a>.

Given:

| A B C |      | J K L |
<a href='#SkMatrix_Concat_a'>a</a> = | D E F |, <a href='#SkMatrix_Concat_b'>b</a> = | M N O |
| G H <a href='#SkMatrix_I'>I</a> |      | P Q R |

sets <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> to:

| A B C |   | J K L |   | AJ+BM+CP AK+BN+CQ AL+BO+CR |
<a href='#SkMatrix_Concat_a'>a</a> * <a href='#SkMatrix_Concat_b'>b</a> = | D E F | * | M N O | = | DJ+EM+FP DK+EN+FQ DL+EO+FR |
| G H <a href='#SkMatrix_I'>I</a> |   | P Q R |   | GJ+HM+IP GK+HN+IQ GL+HO+IR |

### Parameters

<table>  <tr>    <td><a name='SkMatrix_Concat_a'><code><strong>a</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> on  left side of multiply expression</td>
  </tr>
  <tr>    <td><a name='SkMatrix_Concat_b'><code><strong>b</strong></code></a></td>
    <td><a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> on  right side of multiply expression</td>
  </tr>
</table>

### Return Value

<a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> computed from <a href='#SkMatrix_Concat_a'>a</a> times <a href='#SkMatrix_Concat_b'>b</a>

### Example

<div><fiddle-embed name="@Matrix_Concat"><div><a href='#SkMatrix_setPolyToPoly'>setPolyToPoly</a> creates perspective <a href='SkMatrix_Reference#Matrix'>matrices</a>, one the inverse of the other.
Multiplying the <a href='SkMatrix_Reference#Matrix'>matrix</a> by its inverse turns into an identity <a href='SkMatrix_Reference#Matrix'>matrix</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkMatrix_preConcat'>preConcat</a> <a href='#SkMatrix_postConcat'>postConcat</a>

<a name='SkMatrix_dirtyMatrixTypeCache'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_dirtyMatrixTypeCache'>dirtyMatrixTypeCache</a>()
</pre>

Sets internal cache to unknown state. Use to force update after repeated
modifications to <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> element reference returned by <a href='#SkMatrix_array1_operator'>operator[]</a>(int index).

### Example

<div><fiddle-embed name="@Matrix_dirtyMatrixTypeCache">

#### Example Output

~~~~
with identity matrix: x = 24
after skew x mod:     x = 24
after 2nd skew x mod: x = 24
after dirty cache:    x = 66
~~~~

</fiddle-embed></div>

### See Also

<a href='#SkMatrix_array1_operator'>operator[]</a>(int index) <a href='#SkMatrix_getType'>getType</a>

<a name='SkMatrix_setScaleTranslate'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkMatrix_setScaleTranslate'>setScaleTranslate</a>(<a href='undocumented#SkScalar'>SkScalar</a> sx, <a href='undocumented#SkScalar'>SkScalar</a> sy, <a href='undocumented#SkScalar'>SkScalar</a> tx, <a href='undocumented#SkScalar'>SkScalar</a> ty)
</pre>

Initializes <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a> with scale and translate elements.

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

<div><fiddle-embed name="@Matrix_setScaleTranslate">

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
bool <a href='#SkMatrix_isFinite'>isFinite</a>()const
</pre>

Returns true if all elements of the <a href='SkMatrix_Reference#Matrix'>matrix</a> are finite. Returns false if any
element is infinity, or NaN.

### Return Value

true if <a href='SkMatrix_Reference#Matrix'>matrix</a> has only finite elements

### Example

<div><fiddle-embed name="@Matrix_isFinite">

#### Example Output

~~~~
[  1.0000   0.0000      nan][  0.0000   1.0000   0.0000][  0.0000   0.0000   1.0000]
matrix is finite: false
matrix != matrix
~~~~

</fiddle-embed></div>

### See Also

operator==

