SkMatrix Reference
===

# <a name="Matrix"></a> Matrix

## <a name="Overview"></a> Overview

## <a name="Overview_Subtopic"></a> Overview Subtopic

| name | description |
| --- | --- |
| Constant | enum and enum class, const values |
| <a href="#Constructor">Constructor</a> | functions that construct <a href="#SkMatrix">SkMatrix</a> |
| <a href="#Member_Function">Member Function</a> | static functions and member methods |
| <a href="#Operator">Operator</a> | operator overloading methods |
| <a href="#Related_Function">Related Function</a> | similar methods grouped together |

# <a name="SkMatrix"></a> Class SkMatrix
<a href="#Matrix">Matrix</a> holds a 3x3 matrix for transforming coordinates. This allows mapping
<a href="SkPoint_Reference#Point">Points</a> and <a href="SkPoint_Reference#Vector">Vectors</a> with translation, scaling, skewing, rotation, and
perspective.

<a href="#Matrix">Matrix</a> elements are in row major order. <a href="#Matrix">Matrix</a> does not have a constructor,
so it must be explicitly initialized. <a href="#SkMatrix_setIdentity">setIdentity</a> initializes <a href="#Matrix">Matrix</a>
so it has no effect. <a href="#SkMatrix_setTranslate">setTranslate</a>, <a href="#SkMatrix_setScale">setScale</a>, <a href="#SkMatrix_setSkew">setSkew</a>, <a href="#SkMatrix_setRotate">setRotate</a>, <a href="#SkMatrix_set9">set9</a> and <a href="#SkMatrix_setAll">setAll</a>
initializes all <a href="#Matrix">Matrix</a> elements with the corresponding mapping.

<a href="#Matrix">Matrix</a> includes a hidden variable that classifies the type of matrix to
improve performance. <a href="#Matrix">Matrix</a> is not thread safe unless <a href="#SkMatrix_getType">getType</a> is called first.

## <a name="Member_Function"></a> Member Function

| name | description |
| --- | --- |
| <a href="#SkMatrix_Concat">Concat</a> | returns the concatenation of <a href="#Matrix">Matrix</a> pair |
| I | returns a reference to a const identity <a href="#Matrix">Matrix</a> |
| <a href="#SkMatrix_InvalidMatrix">InvalidMatrix</a> | returns a reference to a const invalid <a href="#Matrix">Matrix</a> |
| <a href="#SkMatrix_MakeAll">MakeAll</a> | constructs all nine values |
| <a href="#SkMatrix_MakeRectToRect">MakeRectToRect</a> | constructs from source <a href="SkRect_Reference#Rect">Rect</a> to destination <a href="SkRect_Reference#Rect">Rect</a> |
| <a href="#SkMatrix_MakeScale">MakeScale</a> | constructs from scale in x and y |
| <a href="#SkMatrix_MakeTrans">MakeTrans</a> | constructs from translate in x and y |
| <a href="#SkMatrix_SetAffineIdentity">SetAffineIdentity</a> | sets 3x2 array to identity |
| <a href="#SkMatrix_asAffine">asAffine</a> | copies to 3x2 array |
| <a href="#SkMatrix_cheapEqualTo">cheapEqualTo</a> | compares <a href="#Matrix">Matrix</a> pair using memcmp() |
| <a href="#SkMatrix_decomposeScale">decomposeScale</a> | separates scale if possible |
| <a href="#SkMatrix_dirtyMatrixTypeCache">dirtyMatrixTypeCache</a> | sets internal cache to unknown state |
| <a href="#SkMatrix_dump">dump</a> | sends text representation using floats to standard output |
| <a href="#SkMatrix_fixedStepInX">fixedStepInX</a> | returns step in x for a position in y |
| <a href="#SkMatrix_get">get</a> | returns one of nine <a href="#Matrix">Matrix</a> values |
| <a href="#SkMatrix_get9">get9</a> | returns all nine <a href="#Matrix">Matrix</a> values |
| <a href="#SkMatrix_getMaxScale">getMaxScale</a> | returns maximum scaling, if possible |
| <a href="#SkMatrix_getMinMaxScales">getMinMaxScales</a> | returns minimum and maximum scaling, if possible |
| <a href="#SkMatrix_getMinScale">getMinScale</a> | returns minimum scaling, if possible |
| <a href="#SkMatrix_getPerspX">getPerspX</a> | returns input x perspective factor |
| <a href="#SkMatrix_getPerspY">getPerspY</a> | returns input y perspective factor |
| <a href="#SkMatrix_getScaleX">getScaleX</a> | returns horizontal scale factor |
| <a href="#SkMatrix_getScaleY">getScaleY</a> | returns vertical scale factor |
| <a href="#SkMatrix_getSkewX">getSkewX</a> | returns horizontal skew factor |
| <a href="#SkMatrix_getSkewY">getSkewY</a> | returns vertical skew factor |
| <a href="#SkMatrix_getTranslateX">getTranslateX</a> | returns horizontal translation |
| <a href="#SkMatrix_getTranslateY">getTranslateY</a> | returns vertical translation |
| <a href="#SkMatrix_getType">getType</a> | returns transform complexity |
| <a href="#SkMatrix_hasPerspective">hasPerspective</a> | returns if transform includes perspective |
| <a href="#SkMatrix_invert">invert</a> | returns inverse, if possible |
| <a href="#SkMatrix_isFinite">isFinite</a> | returns if all <a href="#Matrix">Matrix</a> values are not infinity, NaN |
| <a href="#SkMatrix_isFixedStepInX">isFixedStepInX</a> | returns if transformation supports fixed step in x |
| <a href="#SkMatrix_isIdentity">isIdentity</a> | returns if matrix equals the identity <a href="#Matrix">Matrix</a> |
| <a href="#SkMatrix_isScaleTranslate">isScaleTranslate</a> | returns if transform is limited to scale and translate |
| <a href="#SkMatrix_isSimilarity">isSimilarity</a> | returns if transform is limited to square scale and rotation |
| <a href="#SkMatrix_isTranslate">isTranslate</a> | returns if transform is limited to translate |
| <a href="#SkMatrix_mapHomogeneousPoints">mapHomogeneousPoints</a> | maps <a href="undocumented#Point3">Point3</a> array |
| <a href="#SkMatrix_mapPoints">mapPoints</a> | maps <a href="SkPoint_Reference#Point">Point</a> array |
| <a href="#SkMatrix_mapRadius">mapRadius</a> | returns mean radius of mapped <a href="undocumented#Circle">Circle</a> |
| <a href="#SkMatrix_mapRect">mapRect</a> | returns bounds of mapped <a href="SkRect_Reference#Rect">Rect</a> |
| <a href="#SkMatrix_mapRectScaleTranslate">mapRectScaleTranslate</a> | returns bounds of mapped <a href="SkRect_Reference#Rect">Rect</a> |
| <a href="#SkMatrix_mapRectToQuad">mapRectToQuad</a> | maps <a href="SkRect_Reference#Rect">Rect</a> to <a href="SkPoint_Reference#Point">Point</a> array |
| <a href="#SkMatrix_mapVector">mapVector</a> | maps <a href="SkPoint_Reference#Vector">Vector</a> |
| <a href="#SkMatrix_mapVectors">mapVectors</a> | maps <a href="SkPoint_Reference#Vector">Vector</a> array |
| <a href="#SkMatrix_mapXY">mapXY</a> | maps <a href="SkPoint_Reference#Point">Point</a> |
| <a href="#SkMatrix_postConcat">postConcat</a> | post-multiplies <a href="#Matrix">Matrix</a> by <a href="#Matrix">Matrix</a> parameter |
| <a href="#SkMatrix_postIDiv">postIDiv</a> | post-multiplies <a href="#Matrix">Matrix</a> by inverse scale |
| <a href="#SkMatrix_postRotate">postRotate</a> | post-multiplies <a href="#Matrix">Matrix</a> by rotation |
| <a href="#SkMatrix_postScale">postScale</a> | post-multiplies <a href="#Matrix">Matrix</a> by scale |
| <a href="#SkMatrix_postSkew">postSkew</a> | post-multiplies <a href="#Matrix">Matrix</a> by skew |
| <a href="#SkMatrix_postTranslate">postTranslate</a> | post-multiplies <a href="#Matrix">Matrix</a> by translation |
| <a href="#SkMatrix_preConcat">preConcat</a> | pre-multiplies <a href="#Matrix">Matrix</a> by <a href="#Matrix">Matrix</a> parameter |
| <a href="#SkMatrix_preRotate">preRotate</a> | pre-multiplies <a href="#Matrix">Matrix</a> by rotation |
| <a href="#SkMatrix_preScale">preScale</a> | pre-multiplies <a href="#Matrix">Matrix</a> by scale |
| <a href="#SkMatrix_preSkew">preSkew</a> | pre-multiplies <a href="#Matrix">Matrix</a> by skew |
| <a href="#SkMatrix_preTranslate">preTranslate</a> | pre-multiplies <a href="#Matrix">Matrix</a> by translation |
| <a href="#SkMatrix_preservesAxisAlignment">preservesAxisAlignment</a> | returns if mapping restricts to 90 degree multiples and mirroring |
| <a href="#SkMatrix_preservesRightAngles">preservesRightAngles</a> | returns if mapped 90 angle remains 90 degrees |
| <a href="#SkMatrix_rectStaysRect">rectStaysRect</a> | returns if mapped <a href="SkRect_Reference#Rect">Rect</a> can be represented by another <a href="SkRect_Reference#Rect">Rect</a> |
| <a href="#SkMatrix_reset">reset</a> | sets <a href="#Matrix">Matrix</a> to identity |
| <a href="#SkMatrix_set">set</a> | sets one value |
| <a href="#SkMatrix_set9">set9</a> | sets all values from <a href="undocumented#Scalar">Scalar</a> array |
| <a href="#SkMatrix_setAffine">setAffine</a> | sets left two columns |
| <a href="#SkMatrix_setAll">setAll</a> | sets all values from parameters |
| <a href="#SkMatrix_setConcat">setConcat</a> | sets to <a href="#Matrix">Matrix</a> parameter multiplied by <a href="#Matrix">Matrix</a> parameter |
| <a href="#SkMatrix_setIdentity">setIdentity</a> | sets <a href="#Matrix">Matrix</a> to identity |
| <a href="#SkMatrix_setPerspX">setPerspX</a> | sets input x perspective factor |
| <a href="#SkMatrix_setPerspY">setPerspY</a> | sets input y perspective factor |
| <a href="#SkMatrix_setPolyToPoly">setPolyToPoly</a> | sets to map one to four points to an equal array of points |
| <a href="#SkMatrix_setRSXform">setRSXform</a> | sets to rotate, scale, and translate |
| <a href="#SkMatrix_setRectToRect">setRectToRect</a> | sets to map one <a href="SkRect_Reference#Rect">Rect</a> to another |
| <a href="#SkMatrix_setRotate">setRotate</a> | sets to rotate about a point |
| <a href="#SkMatrix_setScale">setScale</a> | sets to scale about a point |
| <a href="#SkMatrix_setScaleTranslate">setScaleTranslate</a> | sets to scale and translate |
| <a href="#SkMatrix_setScaleX">setScaleX</a> | sets horizontal scale factor |
| <a href="#SkMatrix_setScaleY">setScaleY</a> | sets vertical scale factor |
| <a href="#SkMatrix_setSinCos">setSinCos</a> | sets to rotate and scale about a point |
| <a href="#SkMatrix_setSkew">setSkew</a> | sets to skew about a point |
| <a href="#SkMatrix_setSkewX">setSkewX</a> | sets horizontal skew factor |
| <a href="#SkMatrix_setSkewY">setSkewY</a> | sets vertical skew factor |
| <a href="#SkMatrix_setTranslate">setTranslate</a> | sets to translate in x and y |
| <a href="#SkMatrix_setTranslateX">setTranslateX</a> | sets horizontal translation |
| <a href="#SkMatrix_setTranslateY">setTranslateY</a> | sets vertical translation |
| <a href="#SkMatrix_toString">toString</a> | converts <a href="#Matrix">Matrix</a> to machine readable form |

## <a name="Related_Function"></a> Related Function

| name | description |
| --- | --- |
| <a href="#Property">Property</a> | values and attributes |
| <a href="#Set">Set</a> | set one or more matrix values |
| <a href="#Transform">Transform</a> | map points with <a href="#Matrix">Matrix</a> |
| <a href="#Utility">Utility</a> | rarely called management functions |

## <a name="Constructor"></a> Constructor

| name | description |
| --- | --- |
| I | returns a reference to a const identity <a href="#Matrix">Matrix</a> |
| <a href="#SkMatrix_InvalidMatrix">InvalidMatrix</a> | returns a reference to a const invalid <a href="#Matrix">Matrix</a> |
| <a href="#SkMatrix_MakeAll">MakeAll</a> | constructs all nine values |
| <a href="#SkMatrix_MakeRectToRect">MakeRectToRect</a> | constructs from source <a href="SkRect_Reference#Rect">Rect</a> to destination <a href="SkRect_Reference#Rect">Rect</a> |
| <a href="#SkMatrix_MakeScale">MakeScale</a> | constructs from scale in x and y |
|  | <a href="#SkMatrix_MakeScale">MakeScale(SkScalar sx, SkScalar sy)</a> |
|  | <a href="#SkMatrix_MakeScale_2">MakeScale(SkScalar scale)</a> |
| <a href="#SkMatrix_MakeTrans">MakeTrans</a> | constructs from translate in x and y |
| <a href="#SkMatrix_SetAffineIdentity">SetAffineIdentity</a> | sets 3x2 array to identity |
| <a href="#SkMatrix_asAffine">asAffine</a> | copies to 3x2 array |
| <a href="#SkMatrix_reset">reset</a> | sets <a href="#Matrix">Matrix</a> to identity |
| <a href="#SkMatrix_setAffine">setAffine</a> | sets left two columns |
| <a href="#SkMatrix_setConcat">setConcat</a> | sets to <a href="#Matrix">Matrix</a> parameter multiplied by <a href="#Matrix">Matrix</a> parameter |
| <a href="#SkMatrix_setIdentity">setIdentity</a> | sets <a href="#Matrix">Matrix</a> to identity |
| <a href="#SkMatrix_setRSXform">setRSXform</a> | sets to rotate, scale, and translate |
| <a href="#SkMatrix_setRotate">setRotate</a> | sets to rotate about a point |
|  | <a href="#SkMatrix_setRotate">setRotate(SkScalar degrees, SkScalar px, SkScalar py)</a> |
|  | <a href="#SkMatrix_setRotate_2">setRotate(SkScalar degrees)</a> |
| <a href="#SkMatrix_setScale">setScale</a> | sets to scale about a point |
|  | <a href="#SkMatrix_setScale">setScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py)</a> |
|  | <a href="#SkMatrix_setScale_2">setScale(SkScalar sx, SkScalar sy)</a> |
| <a href="#SkMatrix_setScaleTranslate">setScaleTranslate</a> | sets to scale and translate |
| <a href="#SkMatrix_setSinCos">setSinCos</a> | sets to rotate and scale about a point |
|  | <a href="#SkMatrix_setSinCos">setSinCos(SkScalar sinValue, SkScalar cosValue, SkScalar px, SkScalar py)</a> |
|  | <a href="#SkMatrix_setSinCos_2">setSinCos(SkScalar sinValue, SkScalar cosValue)</a> |
| <a href="#SkMatrix_setSkew">setSkew</a> | sets to skew about a point |
|  | <a href="#SkMatrix_setSkew">setSkew(SkScalar kx, SkScalar ky, SkScalar px, SkScalar py)</a> |
|  | <a href="#SkMatrix_setSkew_2">setSkew(SkScalar kx, SkScalar ky)</a> |
| <a href="#SkMatrix_setTranslate">setTranslate</a> | sets to translate in x and y |
|  | <a href="#SkMatrix_setTranslate">setTranslate(SkScalar dx, SkScalar dy)</a> |
|  | <a href="#SkMatrix_setTranslate_2">setTranslate(const SkVector& v)</a> |

<a name="SkMatrix_MakeScale"></a>
## MakeScale

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static SkMatrix SK_WARN_UNUSED_RESULT MakeScale(SkScalar sx, SkScalar sy)
</pre>

Sets <a href="#Matrix">Matrix</a> to scale by (<a href="#SkMatrix_MakeScale_sx">sx</a>, <a href="#SkMatrix_MakeScale_sy">sy</a>). Returned matrix is:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
| sx  0  0 |
|  0 sy  0 |
|  0  0  1 |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_MakeScale_sx"> <code><strong>sx </strong></code> </a></td> <td>
horizontal scale factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_MakeScale_sy"> <code><strong>sy </strong></code> </a></td> <td>
vertical scale factor</td>
  </tr>
</table>

### Return Value

<a href="#Matrix">Matrix</a> with scale

### Example

<div><fiddle-embed name="7ff17718111df6d6f95381d8a8f1b389"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setScale">setScale</a><sup><a href="#SkMatrix_setScale_2">[2]</a></sup> <a href="#SkMatrix_postScale">postScale</a><sup><a href="#SkMatrix_postScale_2">[2]</a></sup> <a href="#SkMatrix_preScale">preScale</a><sup><a href="#SkMatrix_preScale_2">[2]</a></sup>

---

<a name="SkMatrix_MakeScale_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static SkMatrix SK_WARN_UNUSED_RESULT MakeScale(SkScalar scale)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#SkMatrix_MakeScale_2_scale">scale</a> by (<a href="#SkMatrix_MakeScale_2_scale">scale</a>, <a href="#SkMatrix_MakeScale_2_scale">scale</a>). Returned matrix is:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
| scale   0   0 |
|   0   scale 0 |
|   0     0   1 |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_MakeScale_2_scale"> <code><strong>scale </strong></code> </a></td> <td>
horizontal and vertical <a href="#SkMatrix_MakeScale_2_scale">scale</a> factor</td>
  </tr>
</table>

### Return Value

<a href="#Matrix">Matrix</a> with <a href="#SkMatrix_MakeScale_2_scale">scale</a>

### Example

<div><fiddle-embed name="2956aeb50fa862cdb13995e1e56a4bc8"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setScale">setScale</a><sup><a href="#SkMatrix_setScale_2">[2]</a></sup> <a href="#SkMatrix_postScale">postScale</a><sup><a href="#SkMatrix_postScale_2">[2]</a></sup> <a href="#SkMatrix_preScale">preScale</a><sup><a href="#SkMatrix_preScale_2">[2]</a></sup>

---

<a name="SkMatrix_MakeTrans"></a>
## MakeTrans

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static SkMatrix SK_WARN_UNUSED_RESULT MakeTrans(SkScalar dx, SkScalar dy)
</pre>

Sets <a href="#Matrix">Matrix</a> to translate by (<a href="#SkMatrix_MakeTrans_dx">dx</a>, <a href="#SkMatrix_MakeTrans_dy">dy</a>). Returned matrix is:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
| 1 0 dx |
| 0 1 dy |
| 0 0  1 |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_MakeTrans_dx"> <code><strong>dx </strong></code> </a></td> <td>
horizontal translation</td>
  </tr>  <tr>    <td><a name="SkMatrix_MakeTrans_dy"> <code><strong>dy </strong></code> </a></td> <td>
vertical translation</td>
  </tr>
</table>

### Return Value

<a href="#Matrix">Matrix</a> with translation

### Example

<div><fiddle-embed name="b2479df0d9cf296ff64ac31e36684557"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setTranslate">setTranslate</a><sup><a href="#SkMatrix_setTranslate_2">[2]</a></sup> <a href="#SkMatrix_postTranslate">postTranslate</a> <a href="#SkMatrix_preTranslate">preTranslate</a>

---

<a name="SkMatrix_MakeAll"></a>
## MakeAll

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static SkMatrix SK_WARN_UNUSED_RESULT MakeAll(SkScalar scaleX, SkScalar skewX, SkScalar transX,
                                              SkScalar skewY, SkScalar scaleY, SkScalar transY,
                                              SkScalar pers0, SkScalar pers1, SkScalar pers2)
</pre>

Sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
| scaleX  skewX transX |
|  skewY scaleY transY |
|  pers0  pers1  pers2 |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_MakeAll_scaleX"> <code><strong>scaleX </strong></code> </a></td> <td>
horizontal scale factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_MakeAll_skewX"> <code><strong>skewX </strong></code> </a></td> <td>
horizontal skew factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_MakeAll_transX"> <code><strong>transX </strong></code> </a></td> <td>
horizontal translation</td>
  </tr>  <tr>    <td><a name="SkMatrix_MakeAll_skewY"> <code><strong>skewY </strong></code> </a></td> <td>
vertical skew factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_MakeAll_scaleY"> <code><strong>scaleY </strong></code> </a></td> <td>
vertical scale factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_MakeAll_transY"> <code><strong>transY </strong></code> </a></td> <td>
vertical translation</td>
  </tr>  <tr>    <td><a name="SkMatrix_MakeAll_pers0"> <code><strong>pers0 </strong></code> </a></td> <td>
input x perspective factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_MakeAll_pers1"> <code><strong>pers1 </strong></code> </a></td> <td>
input y perspective factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_MakeAll_pers2"> <code><strong>pers2 </strong></code> </a></td> <td>
perspective scale factor</td>
  </tr>
</table>

### Return Value

<a href="#Matrix">Matrix</a> constructed from parameters

### Example

<div><fiddle-embed name="6bad83b64de9266e323c29d550e04188"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setAll">setAll</a> <a href="#SkMatrix_set9">set9</a> <a href="#SkMatrix_postConcat">postConcat</a> <a href="#SkMatrix_preConcat">preConcat</a>

---

## <a name="SkMatrix_TypeMask"></a> Enum SkMatrix::TypeMask

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum <a href="#SkMatrix_TypeMask">TypeMask</a> {
<a href="#SkMatrix_kIdentity_Mask">kIdentity Mask</a> = 0,
<a href="#SkMatrix_kTranslate_Mask">kTranslate Mask</a> = 0x01,
<a href="#SkMatrix_kScale_Mask">kScale Mask</a> = 0x02,
<a href="#SkMatrix_kAffine_Mask">kAffine Mask</a> = 0x04,
<a href="#SkMatrix_kPerspective_Mask">kPerspective Mask</a> = 0x08,
};</pre>

Enum of bit fields for mask returned by <a href="#SkMatrix_getType">getType</a>.
Used to identify the complexity of <a href="#Matrix">Matrix</a>, to optimize performance.

### Constants

<table>
  <tr>
    <td><a name="SkMatrix_kIdentity_Mask"> <code><strong>SkMatrix::kIdentity_Mask </strong></code> </a></td><td>0</td><td>all bits clear if <a href="#Matrix">Matrix</a> is identity</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kTranslate_Mask"> <code><strong>SkMatrix::kTranslate_Mask </strong></code> </a></td><td>1</td><td>set if <a href="#Matrix">Matrix</a> has translation</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kScale_Mask"> <code><strong>SkMatrix::kScale_Mask </strong></code> </a></td><td>2</td><td>set if <a href="#Matrix">Matrix</a> has x or y scale</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kAffine_Mask"> <code><strong>SkMatrix::kAffine_Mask </strong></code> </a></td><td>4</td><td>set if <a href="#Matrix">Matrix</a> skews or rotates</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kPerspective_Mask"> <code><strong>SkMatrix::kPerspective_Mask </strong></code> </a></td><td>8</td><td>set if <a href="#Matrix">Matrix</a> has perspective</td>
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

<a href="#SkMatrix_getType">getType</a>



## <a name="Property"></a> Property

| name | description |
| --- | --- |
| <a href="#SkMatrix_decomposeScale">decomposeScale</a> | separates scale if possible |
| <a href="#SkMatrix_fixedStepInX">fixedStepInX</a> | returns step in x for a position in y |
| <a href="#SkMatrix_get">get</a> | returns one of nine <a href="#Matrix">Matrix</a> values |
| <a href="#SkMatrix_get9">get9</a> | returns all nine <a href="#Matrix">Matrix</a> values |
| <a href="#SkMatrix_getMaxScale">getMaxScale</a> | returns maximum scaling, if possible |
| <a href="#SkMatrix_getMinMaxScales">getMinMaxScales</a> | returns minimum and maximum scaling, if possible |
| <a href="#SkMatrix_getMinScale">getMinScale</a> | returns minimum scaling, if possible |
| <a href="#SkMatrix_getPerspX">getPerspX</a> | returns input x perspective factor |
| <a href="#SkMatrix_getPerspY">getPerspY</a> | returns input y perspective factor |
| <a href="#SkMatrix_getScaleX">getScaleX</a> | returns horizontal scale factor |
| <a href="#SkMatrix_getScaleY">getScaleY</a> | returns vertical scale factor |
| <a href="#SkMatrix_getSkewX">getSkewX</a> | returns horizontal skew factor |
| <a href="#SkMatrix_getSkewY">getSkewY</a> | returns vertical skew factor |
| <a href="#SkMatrix_getTranslateX">getTranslateX</a> | returns horizontal translation |
| <a href="#SkMatrix_getTranslateY">getTranslateY</a> | returns vertical translation |
| <a href="#SkMatrix_getType">getType</a> | returns transform complexity |
| <a href="#SkMatrix_hasPerspective">hasPerspective</a> | returns if transform includes perspective |
| <a href="#SkMatrix_isFinite">isFinite</a> | returns if all <a href="#Matrix">Matrix</a> values are not infinity, NaN |
| <a href="#SkMatrix_isFixedStepInX">isFixedStepInX</a> | returns if transformation supports fixed step in x |
| <a href="#SkMatrix_isIdentity">isIdentity</a> | returns if matrix equals the identity <a href="#Matrix">Matrix</a> |
| <a href="#SkMatrix_isScaleTranslate">isScaleTranslate</a> | returns if transform is limited to scale and translate |
| <a href="#SkMatrix_isSimilarity">isSimilarity</a> | returns if transform is limited to square scale and rotation |
| <a href="#SkMatrix_isTranslate">isTranslate</a> | returns if transform is limited to translate |
| <a href="#SkMatrix_preservesAxisAlignment">preservesAxisAlignment</a> | returns if mapping restricts to 90 degree multiples and mirroring |
| <a href="#SkMatrix_preservesRightAngles">preservesRightAngles</a> | returns if mapped 90 angle remains 90 degrees |
| <a href="#SkMatrix_rectStaysRect">rectStaysRect</a> | returns if mapped <a href="SkRect_Reference#Rect">Rect</a> can be represented by another <a href="SkRect_Reference#Rect">Rect</a> |

<a name="SkMatrix_getType"></a>
## getType

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
TypeMask getType() const
</pre>

Returns a bit field describing the transformations the matrix may
perform. The bit field is computed conservatively, so it may include
false positives. For example, when <a href="#SkMatrix_kPerspective_Mask">kPerspective Mask</a> is set, all
other bits are set.

### Return Value

<a href="#SkMatrix_kIdentity_Mask">kIdentity Mask</a>, or combinations of: <a href="#SkMatrix_kTranslate_Mask">kTranslate Mask</a>, <a href="#SkMatrix_kScale_Mask">kScale Mask</a>,
<a href="#SkMatrix_kAffine_Mask">kAffine Mask</a>, <a href="#SkMatrix_kPerspective_Mask">kPerspective Mask</a>

### Example

<div><fiddle-embed name="8e45fe2dd52731bb2d4318686257e1d7">

#### Example Output

~~~~
identity flags hex: 0 decimal: 0
set all  flags hex: f decimal: 15
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkMatrix_TypeMask">TypeMask</a>

---

<a name="SkMatrix_isIdentity"></a>
## isIdentity

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isIdentity() const
</pre>

Returns true if <a href="#Matrix">Matrix</a> is identity.  Identity matrix is:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
| 1 0 0 |
| 0 1 0 |
| 0 0 1 |</pre>

### Return Value

true if <a href="#Matrix">Matrix</a> has no effect

### Example

<div><fiddle-embed name="780ab376325b3cfa889ea26c0769ec11">

#### Example Output

~~~~
is identity: true
is identity: false
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkMatrix_reset">reset</a> <a href="#SkMatrix_setIdentity">setIdentity</a> <a href="#SkMatrix_getType">getType</a>

---

<a name="SkMatrix_isScaleTranslate"></a>
## isScaleTranslate

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isScaleTranslate() const
</pre>

Returns true if <a href="#Matrix">Matrix</a> at most scales and translates. <a href="#Matrix">Matrix</a> may be identity,
contain only scale elements, only translate elements, or both. <a href="#Matrix">Matrix</a> form is:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
| scale-x    0    translate-x |
|    0    scale-y translate-y |
|    0       0         1      |</pre>

### Return Value

true if <a href="#Matrix">Matrix</a> is identity; or scales, translates, or both

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

<a href="#SkMatrix_setScale">setScale</a><sup><a href="#SkMatrix_setScale_2">[2]</a></sup> <a href="#SkMatrix_isTranslate">isTranslate</a> <a href="#SkMatrix_setTranslate">setTranslate</a><sup><a href="#SkMatrix_setTranslate_2">[2]</a></sup> <a href="#SkMatrix_getType">getType</a>

---

<a name="SkMatrix_isTranslate"></a>
## isTranslate

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isTranslate() const
</pre>

Returns true if <a href="#Matrix">Matrix</a> is identity, or translates. <a href="#Matrix">Matrix</a> form is:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
| 1 0 translate-x |
| 0 1 translate-y |
| 0 0      1      |</pre>

### Return Value

true if <a href="#Matrix">Matrix</a> is identity, or translates

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

<a href="#SkMatrix_setTranslate">setTranslate</a><sup><a href="#SkMatrix_setTranslate_2">[2]</a></sup> <a href="#SkMatrix_getType">getType</a>

---

<a name="SkMatrix_rectStaysRect"></a>
## rectStaysRect

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool rectStaysRect() const
</pre>

Returns true <a href="#Matrix">Matrix</a> maps <a href="SkRect_Reference#Rect">Rect</a> to another <a href="SkRect_Reference#Rect">Rect</a>. If true, <a href="#Matrix">Matrix</a> is identity,
or scales, or rotates a multiple of 90 degrees, or mirrors in x or y. In all
cases, <a href="#Matrix">Matrix</a> may also have translation. <a href="#Matrix">Matrix</a> form is either:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
| scale-x    0    translate-x |
|    0    scale-y translate-y |
|    0       0         1      |</pre>

or

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
|    0     rotate-x translate-x |
| rotate-y    0     translate-y |
|    0        0          1      |</pre>

for non-zero values of scale-x, scale-y, rotate-x, and rotate-y.

Also called <a href="#SkMatrix_preservesAxisAlignment">preservesAxisAlignment</a>; use the one that provides better inline
documentation.

### Return Value

true if <a href="#Matrix">Matrix</a> maps one <a href="SkRect_Reference#Rect">Rect</a> into another

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

<a href="#SkMatrix_preservesAxisAlignment">preservesAxisAlignment</a> <a href="#SkMatrix_preservesRightAngles">preservesRightAngles</a>

---

<a name="SkMatrix_preservesAxisAlignment"></a>
## preservesAxisAlignment

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool preservesAxisAlignment() const
</pre>

Returns true <a href="#Matrix">Matrix</a> maps <a href="SkRect_Reference#Rect">Rect</a> to another <a href="SkRect_Reference#Rect">Rect</a>. If true, <a href="#Matrix">Matrix</a> is identity,
or scales, or rotates a multiple of 90 degrees, or mirrors in x or y. In all
cases, <a href="#Matrix">Matrix</a> may also have translation. <a href="#Matrix">Matrix</a> form is either:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
| scale-x    0    translate-x |
|    0    scale-y translate-y |
|    0       0         1      |</pre>

or

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
|    0     rotate-x translate-x |
| rotate-y    0     translate-y |
|    0        0          1      |</pre>

for non-zero values of scale-x, scale-y, rotate-x, and rotate-y.

Also called <a href="#SkMatrix_rectStaysRect">rectStaysRect</a>; use the one that provides better inline
documentation.

### Return Value

true if <a href="#Matrix">Matrix</a> maps one <a href="SkRect_Reference#Rect">Rect</a> into another

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

<a href="#SkMatrix_rectStaysRect">rectStaysRect</a> <a href="#SkMatrix_preservesRightAngles">preservesRightAngles</a>

---

<a name="SkMatrix_hasPerspective"></a>
## hasPerspective

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool hasPerspective() const
</pre>

Returns true if the matrix contains perspective elements. <a href="#Matrix">Matrix</a> form is:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
|       --            --              --          |
|       --            --              --          |
| perspective-x  perspective-y  perspective-scale |</pre>

where perspective-x or perspective-y is non-zero, or perspective-scale is
not one. All other elements may have any value.

### Return Value

true if <a href="#Matrix">Matrix</a> is in most general form

### Example

<div><fiddle-embed name="688123908c733169bbbfaf11f41ecff6"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setAll">setAll</a> <a href="#SkMatrix_set9">set9</a> <a href="#SkMatrix_MakeAll">MakeAll</a>

---

<a name="SkMatrix_isSimilarity"></a>
## isSimilarity

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isSimilarity(SkScalar tol = SK_ScalarNearlyZero) const
</pre>

Returns true if <a href="#Matrix">Matrix</a> contains only translation, rotation, reflection, and
uniform scale.
Returns false if <a href="#Matrix">Matrix</a> contains different scales, skewing, perspective, or
degenerate forms that collapse to a line or point.

Describes that the <a href="#Matrix">Matrix</a> makes rendering with and without the matrix are
visually alike; a transformed circle remains a circle. Mathematically, this is
referred to as similarity of a Euclidean_Space, or a similarity transformation.

Preserves right angles, keeping the arms of the angle equal lengths.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_isSimilarity_tol"> <code><strong>tol </strong></code> </a></td> <td>
to be deprecated</td>
  </tr>
</table>

### Return Value

true if <a href="#Matrix">Matrix</a> only rotates, uniformly scales, translates

### Example

<div><fiddle-embed name="98d60230ad633ae74d851de3a65d72d6"><div><a href="undocumented#String">String</a> is drawn four times through but only two are visible. Drawing the pair
with <a href="#SkMatrix_isSimilarity">isSimilarity</a> false reveals the pair not visible through the matrix.</div></fiddle-embed></div>

### See Also

<a href="#SkMatrix_isScaleTranslate">isScaleTranslate</a> <a href="#SkMatrix_preservesRightAngles">preservesRightAngles</a> <a href="#SkMatrix_rectStaysRect">rectStaysRect</a> <a href="#SkMatrix_isFixedStepInX">isFixedStepInX</a>

---

<a name="SkMatrix_preservesRightAngles"></a>
## preservesRightAngles

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool preservesRightAngles(SkScalar tol = SK_ScalarNearlyZero) const
</pre>

Returns true if <a href="#Matrix">Matrix</a> contains only translation, rotation, reflection, and
scale. Scale may differ along rotated axes.
Returns false if <a href="#Matrix">Matrix</a> skewing, perspective, or degenerate forms that collapse
to a line or point.

Preserves right angles, but not requiring that the arms of the angle
retain equal lengths.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_preservesRightAngles_tol"> <code><strong>tol </strong></code> </a></td> <td>
to be deprecated</td>
  </tr>
</table>

### Return Value

true if <a href="#Matrix">Matrix</a> only rotates, scales, translates

### Example

<div><fiddle-embed name="a3d5bfebc1c3423fb983d30aaf4ac5f4"><div>Equal scale is both similar and preserves right angles.
Unequal scale is not similar but preserves right angles.
Skews are not similar and do not preserve right angles.</div></fiddle-embed></div>

### See Also

<a href="#SkMatrix_isScaleTranslate">isScaleTranslate</a> <a href="#SkMatrix_isSimilarity">isSimilarity</a> <a href="#SkMatrix_rectStaysRect">rectStaysRect</a> <a href="#SkMatrix_isFixedStepInX">isFixedStepInX</a>

---

## <a name="SkMatrix__anonymous"></a> Enum SkMatrix::_anonymous

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum {
<a href="#SkMatrix_kMScaleX">kMScaleX</a>,
<a href="#SkMatrix_kMSkewX">kMSkewX</a>,
<a href="#SkMatrix_kMTransX">kMTransX</a>,
<a href="#SkMatrix_kMSkewY">kMSkewY</a>,
<a href="#SkMatrix_kMScaleY">kMScaleY</a>,
<a href="#SkMatrix_kMTransY">kMTransY</a>,
<a href="#SkMatrix_kMPersp0">kMPersp0</a>,
<a href="#SkMatrix_kMPersp1">kMPersp1</a>,
<a href="#SkMatrix_kMPersp2">kMPersp2</a>,
};</pre>

<a href="#Matrix">Matrix</a> organizes its values in row order. These members correspond to
each value in <a href="#Matrix">Matrix</a>.

### Constants

<table>
  <tr>
    <td><a name="SkMatrix_kMScaleX"> <code><strong>SkMatrix::kMScaleX </strong></code> </a></td><td>0</td><td>horizontal scale factor</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kMSkewX"> <code><strong>SkMatrix::kMSkewX </strong></code> </a></td><td>1</td><td>horizontal skew factor</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kMTransX"> <code><strong>SkMatrix::kMTransX </strong></code> </a></td><td>2</td><td>horizontal translation</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kMSkewY"> <code><strong>SkMatrix::kMSkewY </strong></code> </a></td><td>3</td><td>vertical skew factor</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kMScaleY"> <code><strong>SkMatrix::kMScaleY </strong></code> </a></td><td>4</td><td>vertical scale factor</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kMTransY"> <code><strong>SkMatrix::kMTransY </strong></code> </a></td><td>5</td><td>vertical translation</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kMPersp0"> <code><strong>SkMatrix::kMPersp0 </strong></code> </a></td><td>6</td><td>input x perspective factor</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kMPersp1"> <code><strong>SkMatrix::kMPersp1 </strong></code> </a></td><td>7</td><td>input y perspective factor</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kMPersp2"> <code><strong>SkMatrix::kMPersp2 </strong></code> </a></td><td>8</td><td>perspective bias</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3bbf75f4748420810aa2586e3c8548d9"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_get">get</a> <a href="#SkMatrix_set">set</a>



## <a name="SkMatrix__anonymous_2"></a> Enum SkMatrix::_anonymous_2

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum {
<a href="#SkMatrix_kAScaleX">kAScaleX</a>,
<a href="#SkMatrix_kASkewY">kASkewY</a>,
<a href="#SkMatrix_kASkewX">kASkewX</a>,
<a href="#SkMatrix_kAScaleY">kAScaleY</a>,
<a href="#SkMatrix_kATransX">kATransX</a>,
<a href="#SkMatrix_kATransY">kATransY</a>,
};</pre>

Affine arrays are in column major order to match the matrix used by
PDF and XPS.

### Constants

<table>
  <tr>
    <td><a name="SkMatrix_kAScaleX"> <code><strong>SkMatrix::kAScaleX </strong></code> </a></td><td>0</td><td>horizontal scale factor</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kASkewY"> <code><strong>SkMatrix::kASkewY </strong></code> </a></td><td>1</td><td>vertical skew factor</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kASkewX"> <code><strong>SkMatrix::kASkewX </strong></code> </a></td><td>2</td><td>horizontal skew factor</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kAScaleY"> <code><strong>SkMatrix::kAScaleY </strong></code> </a></td><td>3</td><td>vertical scale factor</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kATransX"> <code><strong>SkMatrix::kATransX </strong></code> </a></td><td>4</td><td>horizontal translation</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kATransY"> <code><strong>SkMatrix::kATransY </strong></code> </a></td><td>5</td><td>vertical translation</td>
  </tr>
</table>

### See Also

<a href="#SkMatrix_SetAffineIdentity">SetAffineIdentity</a> <a href="#SkMatrix_asAffine">asAffine</a> <a href="#SkMatrix_setAffine">setAffine</a>



## <a name="Operator"></a> Operator

| name | description |
| --- | --- |
| <a href="#SkMatrix_Concat">Concat</a> | returns the concatenation of <a href="#Matrix">Matrix</a> pair |
| <a href="#SkMatrix_cheapEqualTo">cheapEqualTo</a> | compares <a href="#Matrix">Matrix</a> pair using memcmp() |
| <a href="#SkMatrix_invert">invert</a> | returns inverse, if possible |
| <a href="#SkMatrix_notequal_operator">operator!=(const SkMatrix& a, const SkMatrix& b)</a> | returns true if members are unequal |
| <a href="#SkMatrix_equal_operator">operator==(const SkMatrix& a, const SkMatrix& b)</a> | returns true if members are equal |
| <a href="#SkMatrix_array1_operator">operator[](int index)</a> | returns writable reference to <a href="#Matrix">Matrix</a> value |
| <a href="#SkMatrix_array_operator">operator[](int index) const</a> | returns <a href="#Matrix">Matrix</a> value |

<a name="SkMatrix_array_operator"></a>
## operator[]

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkScalar operator[](int index) _const
</pre>

Returns one matrix value. Asserts if <a href="#SkMatrix_array_operator_index">index</a> is out of range and SK_DEBUG is
defined.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_array_operator_index"> <code><strong>index </strong></code> </a></td> <td>
one of: <a href="#SkMatrix_kMScaleX">kMScaleX</a>, <a href="#SkMatrix_kMSkewX">kMSkewX</a>, <a href="#SkMatrix_kMTransX">kMTransX</a>, <a href="#SkMatrix_kMSkewY">kMSkewY</a>, <a href="#SkMatrix_kMScaleY">kMScaleY</a>, <a href="#SkMatrix_kMTransY">kMTransY</a>,
<a href="#SkMatrix_kMPersp0">kMPersp0</a>, <a href="#SkMatrix_kMPersp1">kMPersp1</a>, <a href="#SkMatrix_kMPersp2">kMPersp2</a></td>
  </tr>
</table>

### Return Value

value corresponding to <a href="#SkMatrix_array_operator_index">index</a>

### Example

<div><fiddle-embed name="e8740493abdf0c6341762db9cee56b89">

#### Example Output

~~~~
matrix[SkMatrix::kMScaleX] == 42
matrix[SkMatrix::kMScaleY] == 24
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkMatrix_get">get</a> <a href="#SkMatrix_set">set</a>

---

<a name="SkMatrix_get"></a>
## get

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkScalar get(int index) const
</pre>

Returns one matrix value. Asserts if <a href="#SkMatrix_get_index">index</a> is out of range and SK_DEBUG is
defined.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_get_index"> <code><strong>index </strong></code> </a></td> <td>
one of: <a href="#SkMatrix_kMScaleX">kMScaleX</a>, <a href="#SkMatrix_kMSkewX">kMSkewX</a>, <a href="#SkMatrix_kMTransX">kMTransX</a>, <a href="#SkMatrix_kMSkewY">kMSkewY</a>, <a href="#SkMatrix_kMScaleY">kMScaleY</a>, <a href="#SkMatrix_kMTransY">kMTransY</a>,
<a href="#SkMatrix_kMPersp0">kMPersp0</a>, <a href="#SkMatrix_kMPersp1">kMPersp1</a>, <a href="#SkMatrix_kMPersp2">kMPersp2</a></td>
  </tr>
</table>

### Return Value

value corresponding to <a href="#SkMatrix_get_index">index</a>

### Example

<div><fiddle-embed name="f5ed382bd04fa7d50b2398cce2fca23a">

#### Example Output

~~~~
matrix.get(SkMatrix::kMSkewX) == 42
matrix.get(SkMatrix::kMSkewY) == 24
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkMatrix_array1_operator">operator[](int index)</a> <a href="#SkMatrix_set">set</a>

---

<a name="SkMatrix_getScaleX"></a>
## getScaleX

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkScalar getScaleX() const
</pre>

Returns scale factor multiplied by x input, contributing to x output.
With <a href="#SkMatrix_mapPoints">mapPoints</a>, scales <a href="SkPoint_Reference#Point">Points</a> along the x-axis.

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

<a href="#SkMatrix_get">get</a> <a href="#SkMatrix_getScaleY">getScaleY</a> <a href="#SkMatrix_setScaleX">setScaleX</a> <a href="#SkMatrix_setScale">setScale</a><sup><a href="#SkMatrix_setScale_2">[2]</a></sup>

---

<a name="SkMatrix_getScaleY"></a>
## getScaleY

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkScalar getScaleY() const
</pre>

Returns scale factor multiplied by y input, contributing to y output.
With <a href="#SkMatrix_mapPoints">mapPoints</a>, scales <a href="SkPoint_Reference#Point">Points</a> along the y-axis.

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

<a href="#SkMatrix_get">get</a> <a href="#SkMatrix_getScaleX">getScaleX</a> <a href="#SkMatrix_setScaleY">setScaleY</a> <a href="#SkMatrix_setScale">setScale</a><sup><a href="#SkMatrix_setScale_2">[2]</a></sup>

---

<a name="SkMatrix_getSkewY"></a>
## getSkewY

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkScalar getSkewY() const
</pre>

Returns scale factor multiplied by x input, contributing to y output.
With <a href="#SkMatrix_mapPoints">mapPoints</a>, skews <a href="SkPoint_Reference#Point">Points</a> along the y-axis.
Skew x and y together can rotate <a href="SkPoint_Reference#Point">Points</a>.

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

<a href="#SkMatrix_get">get</a> <a href="#SkMatrix_getSkewX">getSkewX</a> <a href="#SkMatrix_setSkewY">setSkewY</a> <a href="#SkMatrix_setSkew">setSkew</a><sup><a href="#SkMatrix_setSkew_2">[2]</a></sup>

---

<a name="SkMatrix_getSkewX"></a>
## getSkewX

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkScalar getSkewX() const
</pre>

Returns scale factor multiplied by y input, contributing to x output.
With <a href="#SkMatrix_mapPoints">mapPoints</a>, skews <a href="SkPoint_Reference#Point">Points</a> along the x-axis.
Skew x and y together can rotate <a href="SkPoint_Reference#Point">Points</a>.

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

<a href="#SkMatrix_get">get</a> <a href="#SkMatrix_getSkewY">getSkewY</a> <a href="#SkMatrix_setSkewX">setSkewX</a> <a href="#SkMatrix_setSkew">setSkew</a><sup><a href="#SkMatrix_setSkew_2">[2]</a></sup>

---

<a name="SkMatrix_getTranslateX"></a>
## getTranslateX

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkScalar getTranslateX() const
</pre>

Returns translation contributing to x output.
With <a href="#SkMatrix_mapPoints">mapPoints</a>, moves <a href="SkPoint_Reference#Point">Points</a> along the x-axis.

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

<a href="#SkMatrix_get">get</a> <a href="#SkMatrix_getTranslateY">getTranslateY</a> <a href="#SkMatrix_setTranslateX">setTranslateX</a> <a href="#SkMatrix_setTranslate">setTranslate</a><sup><a href="#SkMatrix_setTranslate_2">[2]</a></sup>

---

<a name="SkMatrix_getTranslateY"></a>
## getTranslateY

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkScalar getTranslateY() const
</pre>

Returns translation contributing to y output.
With <a href="#SkMatrix_mapPoints">mapPoints</a>, moves <a href="SkPoint_Reference#Point">Points</a> along the y-axis.

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

<a href="#SkMatrix_get">get</a> <a href="#SkMatrix_getTranslateX">getTranslateX</a> <a href="#SkMatrix_setTranslateY">setTranslateY</a> <a href="#SkMatrix_setTranslate">setTranslate</a><sup><a href="#SkMatrix_setTranslate_2">[2]</a></sup>

---

<a name="SkMatrix_getPerspX"></a>
## getPerspX

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkScalar getPerspX() const
</pre>

Returns factor scaling input x relative to input y.

### Return Value

input x perspective factor

### Example

<div><fiddle-embed name="a0f5bf4b55e8c33bfda29bf67e34306f"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_kMPersp0">kMPersp0</a> <a href="#SkMatrix_getPerspY">getPerspY</a>

---

<a name="SkMatrix_getPerspY"></a>
## getPerspY

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkScalar getPerspY() const
</pre>

Returns factor scaling input y relative to input x.

### Return Value

input y perspective factor

### Example

<div><fiddle-embed name="424a00a73675dbd99ad20feb0267442b"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_kMPersp1">kMPersp1</a> <a href="#SkMatrix_getPerspX">getPerspX</a>

---

<a name="SkMatrix_array1_operator"></a>
## operator[]

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkScalar& operator[](int index)
</pre>

Returns writable <a href="#Matrix">Matrix</a> value. Asserts if <a href="#SkMatrix_array1_operator_index">index</a> is out of range and SK_DEBUG is
defined. Clears internal cache anticipating that caller will change <a href="#Matrix">Matrix</a> value.

Next call to read <a href="#Matrix">Matrix</a> state may recompute cache; subsequent writes to <a href="#Matrix">Matrix</a>
value must be followed by <a href="#SkMatrix_dirtyMatrixTypeCache">dirtyMatrixTypeCache</a>.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_array1_operator_index"> <code><strong>index </strong></code> </a></td> <td>
one of: <a href="#SkMatrix_kMScaleX">kMScaleX</a>, <a href="#SkMatrix_kMSkewX">kMSkewX</a>, <a href="#SkMatrix_kMTransX">kMTransX</a>, <a href="#SkMatrix_kMSkewY">kMSkewY</a>, <a href="#SkMatrix_kMScaleY">kMScaleY</a>, <a href="#SkMatrix_kMTransY">kMTransY</a>,
<a href="#SkMatrix_kMPersp0">kMPersp0</a>, <a href="#SkMatrix_kMPersp1">kMPersp1</a>, <a href="#SkMatrix_kMPersp2">kMPersp2</a></td>
  </tr>
</table>

### Return Value

writable value corresponding to <a href="#SkMatrix_array1_operator_index">index</a>

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

<a href="#SkMatrix_get">get</a> <a href="#SkMatrix_dirtyMatrixTypeCache">dirtyMatrixTypeCache</a> <a href="#SkMatrix_set">set</a>

---

## <a name="Set"></a> Set

| name | description |
| --- | --- |
| <a href="#SkMatrix_postConcat">postConcat</a> | post-multiplies <a href="#Matrix">Matrix</a> by <a href="#Matrix">Matrix</a> parameter |
| <a href="#SkMatrix_postIDiv">postIDiv</a> | post-multiplies <a href="#Matrix">Matrix</a> by inverse scale |
| <a href="#SkMatrix_postRotate">postRotate</a> | post-multiplies <a href="#Matrix">Matrix</a> by rotation |
|  | <a href="#SkMatrix_postRotate">postRotate(SkScalar degrees, SkScalar px, SkScalar py)</a> |
|  | <a href="#SkMatrix_postRotate_2">postRotate(SkScalar degrees)</a> |
| <a href="#SkMatrix_postScale">postScale</a> | post-multiplies <a href="#Matrix">Matrix</a> by scale |
|  | <a href="#SkMatrix_postScale">postScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py)</a> |
|  | <a href="#SkMatrix_postScale_2">postScale(SkScalar sx, SkScalar sy)</a> |
| <a href="#SkMatrix_postSkew">postSkew</a> | post-multiplies <a href="#Matrix">Matrix</a> by skew |
|  | <a href="#SkMatrix_postSkew">postSkew(SkScalar kx, SkScalar ky, SkScalar px, SkScalar py)</a> |
|  | <a href="#SkMatrix_postSkew_2">postSkew(SkScalar kx, SkScalar ky)</a> |
| <a href="#SkMatrix_postTranslate">postTranslate</a> | post-multiplies <a href="#Matrix">Matrix</a> by translation |
| <a href="#SkMatrix_preConcat">preConcat</a> | pre-multiplies <a href="#Matrix">Matrix</a> by <a href="#Matrix">Matrix</a> parameter |
| <a href="#SkMatrix_preRotate">preRotate</a> | pre-multiplies <a href="#Matrix">Matrix</a> by rotation |
|  | <a href="#SkMatrix_preRotate">preRotate(SkScalar degrees, SkScalar px, SkScalar py)</a> |
|  | <a href="#SkMatrix_preRotate_2">preRotate(SkScalar degrees)</a> |
| <a href="#SkMatrix_preScale">preScale</a> | pre-multiplies <a href="#Matrix">Matrix</a> by scale |
|  | <a href="#SkMatrix_preScale">preScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py)</a> |
|  | <a href="#SkMatrix_preScale_2">preScale(SkScalar sx, SkScalar sy)</a> |
| <a href="#SkMatrix_preSkew">preSkew</a> | pre-multiplies <a href="#Matrix">Matrix</a> by skew |
|  | <a href="#SkMatrix_preSkew">preSkew(SkScalar kx, SkScalar ky, SkScalar px, SkScalar py)</a> |
|  | <a href="#SkMatrix_preSkew_2">preSkew(SkScalar kx, SkScalar ky)</a> |
| <a href="#SkMatrix_preTranslate">preTranslate</a> | pre-multiplies <a href="#Matrix">Matrix</a> by translation |
| <a href="#SkMatrix_set">set</a> | sets one value |
| <a href="#SkMatrix_set9">set9</a> | sets all values from <a href="undocumented#Scalar">Scalar</a> array |
| <a href="#SkMatrix_setAll">setAll</a> | sets all values from parameters |
| <a href="#SkMatrix_setPerspX">setPerspX</a> | sets input x perspective factor |
| <a href="#SkMatrix_setPerspY">setPerspY</a> | sets input y perspective factor |
| <a href="#SkMatrix_setPolyToPoly">setPolyToPoly</a> | sets to map one to four points to an equal array of points |
| <a href="#SkMatrix_setRectToRect">setRectToRect</a> | sets to map one <a href="SkRect_Reference#Rect">Rect</a> to another |
| <a href="#SkMatrix_setScaleX">setScaleX</a> | sets horizontal scale factor |
| <a href="#SkMatrix_setScaleY">setScaleY</a> | sets vertical scale factor |
| <a href="#SkMatrix_setSkewX">setSkewX</a> | sets horizontal skew factor |
| <a href="#SkMatrix_setSkewY">setSkewY</a> | sets vertical skew factor |
| <a href="#SkMatrix_setTranslateX">setTranslateX</a> | sets horizontal translation |
| <a href="#SkMatrix_setTranslateY">setTranslateY</a> | sets vertical translation |

<a name="SkMatrix_set"></a>
## set

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void set(int index, SkScalar value)
</pre>

Sets <a href="#Matrix">Matrix</a> <a href="#SkMatrix_set_value">value</a>. Asserts if <a href="#SkMatrix_set_index">index</a> is out of range and SK_DEBUG is
defined. Safer than operator[]; internal cache is always maintained.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_set_index"> <code><strong>index </strong></code> </a></td> <td>
one of: <a href="#SkMatrix_kMScaleX">kMScaleX</a>, <a href="#SkMatrix_kMSkewX">kMSkewX</a>, <a href="#SkMatrix_kMTransX">kMTransX</a>, <a href="#SkMatrix_kMSkewY">kMSkewY</a>, <a href="#SkMatrix_kMScaleY">kMScaleY</a>, <a href="#SkMatrix_kMTransY">kMTransY</a>,
<a href="#SkMatrix_kMPersp0">kMPersp0</a>, <a href="#SkMatrix_kMPersp1">kMPersp1</a>, <a href="#SkMatrix_kMPersp2">kMPersp2</a></td>
  </tr>  <tr>    <td><a name="SkMatrix_set_value"> <code><strong>value </strong></code> </a></td> <td>
<a href="undocumented#Scalar">Scalar</a> to store in <a href="#Matrix">Matrix</a></td>
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

operator[] <a href="#SkMatrix_get">get</a>

---

<a name="SkMatrix_setScaleX"></a>
## setScaleX

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setScaleX(SkScalar v)
</pre>

Sets horizontal scale factor.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setScaleX_v"> <code><strong>v </strong></code> </a></td> <td>
horizontal scale factor to store</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a39dfed98c3c3c3a56be9ad59fe4e21e"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_set">set</a> <a href="#SkMatrix_setScale">setScale</a><sup><a href="#SkMatrix_setScale_2">[2]</a></sup> <a href="#SkMatrix_setScaleY">setScaleY</a>

---

<a name="SkMatrix_setScaleY"></a>
## setScaleY

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setScaleY(SkScalar v)
</pre>

Sets vertical scale factor.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setScaleY_v"> <code><strong>v </strong></code> </a></td> <td>
vertical scale factor to store</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f040c6dd85a02e94eaca00d5c2832604"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_set">set</a> <a href="#SkMatrix_setScale">setScale</a><sup><a href="#SkMatrix_setScale_2">[2]</a></sup> <a href="#SkMatrix_setScaleX">setScaleX</a>

---

<a name="SkMatrix_setSkewY"></a>
## setSkewY

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setSkewY(SkScalar v)
</pre>

Sets vertical skew factor.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setSkewY_v"> <code><strong>v </strong></code> </a></td> <td>
vertical skew factor to store</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="b418d15df9829aefcc6aca93a37428bb"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_set">set</a> <a href="#SkMatrix_setSkew">setSkew</a><sup><a href="#SkMatrix_setSkew_2">[2]</a></sup> <a href="#SkMatrix_setSkewX">setSkewX</a>

---

<a name="SkMatrix_setSkewX"></a>
## setSkewX

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setSkewX(SkScalar v)
</pre>

Sets horizontal skew factor.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setSkewX_v"> <code><strong>v </strong></code> </a></td> <td>
horizontal skew factor to store</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c7177a6fbc1545be95a5ebca87e0cd0d"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_set">set</a> <a href="#SkMatrix_setSkew">setSkew</a><sup><a href="#SkMatrix_setSkew_2">[2]</a></sup> <a href="#SkMatrix_setSkewX">setSkewX</a>

---

<a name="SkMatrix_setTranslateX"></a>
## setTranslateX

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setTranslateX(SkScalar v)
</pre>

Sets horizontal translation.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setTranslateX_v"> <code><strong>v </strong></code> </a></td> <td>
horizontal translation to store</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a18bc2e3607ac3a8e438bcb61fb13130"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_set">set</a> <a href="#SkMatrix_setTranslate">setTranslate</a><sup><a href="#SkMatrix_setTranslate_2">[2]</a></sup> <a href="#SkMatrix_setTranslateY">setTranslateY</a>

---

<a name="SkMatrix_setTranslateY"></a>
## setTranslateY

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setTranslateY(SkScalar v)
</pre>

Sets vertical translation.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setTranslateY_v"> <code><strong>v </strong></code> </a></td> <td>
vertical translation to store</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="34e3c70a72b836abf7f4858d35eecc98"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_set">set</a> <a href="#SkMatrix_setTranslate">setTranslate</a><sup><a href="#SkMatrix_setTranslate_2">[2]</a></sup> <a href="#SkMatrix_setTranslateX">setTranslateX</a>

---

<a name="SkMatrix_setPerspX"></a>
## setPerspX

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setPerspX(SkScalar v)
</pre>

Sets input x perspective factor, which causes <a href="#SkMatrix_mapXY">mapXY</a> to vary input x inversely
proportional to input y.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setPerspX_v"> <code><strong>v </strong></code> </a></td> <td>
perspective factor</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="830a9e4e4bb93d25afd83b2fea63929e"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_getPerspX">getPerspX</a> <a href="#SkMatrix_set">set</a> <a href="#SkMatrix_setAll">setAll</a> <a href="#SkMatrix_set9">set9</a> <a href="#SkMatrix_MakeAll">MakeAll</a>

---

<a name="SkMatrix_setPerspY"></a>
## setPerspY

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setPerspY(SkScalar v)
</pre>

Sets input y perspective factor, which causes <a href="#SkMatrix_mapXY">mapXY</a> to vary input y inversely
proportional to input x.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setPerspY_v"> <code><strong>v </strong></code> </a></td> <td>
perspective factor</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="aeb258b7922c1a11b698b00f562182ec"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_getPerspY">getPerspY</a> <a href="#SkMatrix_set">set</a> <a href="#SkMatrix_setAll">setAll</a> <a href="#SkMatrix_set9">set9</a> <a href="#SkMatrix_MakeAll">MakeAll</a>

---

<a name="SkMatrix_setAll"></a>
## setAll

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setAll(SkScalar scaleX, SkScalar skewX, SkScalar transX, SkScalar skewY, SkScalar scaleY,
            SkScalar transY, SkScalar persp0, SkScalar persp1, SkScalar persp2)
</pre>

Sets all values from parameters. Sets matrix to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
| scaleX  skewX transX |
|  skewY scaleY transY |
| persp0 persp1 persp2 |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setAll_scaleX"> <code><strong>scaleX </strong></code> </a></td> <td>
horizontal scale factor to store</td>
  </tr>  <tr>    <td><a name="SkMatrix_setAll_skewX"> <code><strong>skewX </strong></code> </a></td> <td>
horizontal skew factor to store</td>
  </tr>  <tr>    <td><a name="SkMatrix_setAll_transX"> <code><strong>transX </strong></code> </a></td> <td>
horizontal translation to store</td>
  </tr>  <tr>    <td><a name="SkMatrix_setAll_skewY"> <code><strong>skewY </strong></code> </a></td> <td>
vertical skew factor to store</td>
  </tr>  <tr>    <td><a name="SkMatrix_setAll_scaleY"> <code><strong>scaleY </strong></code> </a></td> <td>
vertical scale factor to store</td>
  </tr>  <tr>    <td><a name="SkMatrix_setAll_transY"> <code><strong>transY </strong></code> </a></td> <td>
vertical translation to store</td>
  </tr>  <tr>    <td><a name="SkMatrix_setAll_persp0"> <code><strong>persp0 </strong></code> </a></td> <td>
input x perspective factor to store</td>
  </tr>  <tr>    <td><a name="SkMatrix_setAll_persp1"> <code><strong>persp1 </strong></code> </a></td> <td>
input y perspective factor to store</td>
  </tr>  <tr>    <td><a name="SkMatrix_setAll_persp2"> <code><strong>persp2 </strong></code> </a></td> <td>
perspective scale factor to store</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="95ccfc2a89ce593e6b7a9f992a844bc0"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_set9">set9</a> <a href="#SkMatrix_MakeAll">MakeAll</a>

---

<a name="SkMatrix_get9"></a>
## get9

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void get9(SkScalar buffer[9]) const
</pre>

Copies nine <a href="undocumented#Scalar">Scalar</a> values contained by <a href="#Matrix">Matrix</a> into <a href="#SkMatrix_get9_buffer">buffer</a>, in member value
ascending order: <a href="#SkMatrix_kMScaleX">kMScaleX</a>, <a href="#SkMatrix_kMSkewX">kMSkewX</a>, <a href="#SkMatrix_kMTransX">kMTransX</a>, <a href="#SkMatrix_kMSkewY">kMSkewY</a>, <a href="#SkMatrix_kMScaleY">kMScaleY</a>, <a href="#SkMatrix_kMTransY">kMTransY</a>,
<a href="#SkMatrix_kMPersp0">kMPersp0</a>, <a href="#SkMatrix_kMPersp1">kMPersp1</a>, <a href="#SkMatrix_kMPersp2">kMPersp2</a>.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_get9_buffer"> <code><strong>buffer </strong></code> </a></td> <td>
storage for nine <a href="undocumented#Scalar">Scalar</a> values</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="df509d73b47cb98b0475e4465db7b246">

#### Example Output

~~~~
{4, 0, 3},
{0, 5, 4},
{0, 0, 1}
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkMatrix_set9">set9</a>

---

<a name="SkMatrix_set9"></a>
## set9

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void set9(const SkScalar buffer[9])
</pre>

Sets <a href="#Matrix">Matrix</a> to nine <a href="undocumented#Scalar">Scalar</a> values in <a href="#SkMatrix_set9_buffer">buffer</a>, in member value ascending order:
<a href="#SkMatrix_kMScaleX">kMScaleX</a>, <a href="#SkMatrix_kMSkewX">kMSkewX</a>, <a href="#SkMatrix_kMTransX">kMTransX</a>, <a href="#SkMatrix_kMSkewY">kMSkewY</a>, <a href="#SkMatrix_kMScaleY">kMScaleY</a>, <a href="#SkMatrix_kMTransY">kMTransY</a>, <a href="#SkMatrix_kMPersp0">kMPersp0</a>, <a href="#SkMatrix_kMPersp1">kMPersp1</a>,
<a href="#SkMatrix_kMPersp2">kMPersp2</a>.

Sets matrix to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
| buffer[0] buffer[1] buffer[2] |
| buffer[3] buffer[4] buffer[5] |
| buffer[6] buffer[7] buffer[8] |</pre>

In the future, <a href="#SkMatrix_set9">set9</a> followed by <a href="#SkMatrix_get9">get9</a> may not return the same values. Since <a href="#Matrix">Matrix</a>
maps non-homogeneous coordinates, scaling all nine values produces an equivalent
transformation, possibly improving precision.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_set9_buffer"> <code><strong>buffer </strong></code> </a></td> <td>
nine <a href="undocumented#Scalar">Scalar</a> values</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ec5de0d23e5fe28ba7628625d1402e85"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setAll">setAll</a> <a href="#SkMatrix_get9">get9</a> <a href="#SkMatrix_MakeAll">MakeAll</a>

---

<a name="SkMatrix_reset"></a>
## reset

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void reset()
</pre>

Sets <a href="#Matrix">Matrix</a> to identity; which has no effect on mapped <a href="SkPoint_Reference#Point">Points</a>. Sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
| 1 0 0 |
| 0 1 0 |
| 0 0 1 |</pre>

Also called <a href="#SkMatrix_setIdentity">setIdentity</a>; use the one that provides better inline
documentation.

### Example

<div><fiddle-embed name="ca94f7922bc37ef03bbc51ad70536fcf">

#### Example Output

~~~~
m.isIdentity(): true
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkMatrix_isIdentity">isIdentity</a> <a href="#SkMatrix_setIdentity">setIdentity</a>

---

<a name="SkMatrix_setIdentity"></a>
## setIdentity

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setIdentity()
</pre>

Sets <a href="#Matrix">Matrix</a> to identity; which has no effect on mapped <a href="SkPoint_Reference#Point">Points</a>. Sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
| 1 0 0 |
| 0 1 0 |
| 0 0 1 |</pre>

Also called <a href="#SkMatrix_reset">reset</a>; use the one that provides better inline
documentation.

### Example

<div><fiddle-embed name="3979c865bb482e6ef1fafc71e56bbb91">

#### Example Output

~~~~
m.isIdentity(): true
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkMatrix_isIdentity">isIdentity</a> <a href="#SkMatrix_reset">reset</a>

---

<a name="SkMatrix_setTranslate"></a>
## setTranslate

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setTranslate(SkScalar dx, SkScalar dy)
</pre>

Sets <a href="#Matrix">Matrix</a> to translate by (<a href="#SkMatrix_setTranslate_dx">dx</a>, <a href="#SkMatrix_setTranslate_dy">dy</a>).

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setTranslate_dx"> <code><strong>dx </strong></code> </a></td> <td>
horizontal translation</td>
  </tr>  <tr>    <td><a name="SkMatrix_setTranslate_dy"> <code><strong>dy </strong></code> </a></td> <td>
vertical translation</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="63ca62985741b1bccb5e8b9cf734874e"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setTranslateX">setTranslateX</a> <a href="#SkMatrix_setTranslateY">setTranslateY</a>

---

<a name="SkMatrix_setTranslate_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setTranslate(const SkVector& v)
</pre>

Sets <a href="#Matrix">Matrix</a> to translate by (<a href="#SkMatrix_setTranslate_2_v">v</a>.fX, <a href="#SkMatrix_setTranslate_2_v">v</a>.fY).

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setTranslate_2_v"> <code><strong>v </strong></code> </a></td> <td>
<a href="SkPoint_Reference#Vector">Vector</a> containing horizontal and vertical translation</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ccfc734aff2ddea0b097c83f5621de5e"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setTranslateX">setTranslateX</a> <a href="#SkMatrix_setTranslateY">setTranslateY</a> <a href="#SkMatrix_MakeTrans">MakeTrans</a>

---

<a name="SkMatrix_setScale"></a>
## setScale

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py)
</pre>

Sets <a href="#Matrix">Matrix</a> to scale by <a href="#SkMatrix_setScale_sx">sx</a> and <a href="#SkMatrix_setScale_sy">sy</a>, about a pivot point at (<a href="#SkMatrix_setScale_px">px</a>, <a href="#SkMatrix_setScale_py">py</a>).
The pivot point is unchanged when mapped with <a href="#Matrix">Matrix</a>.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setScale_sx"> <code><strong>sx </strong></code> </a></td> <td>
horizontal scale factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_setScale_sy"> <code><strong>sy </strong></code> </a></td> <td>
vertical scale factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_setScale_px"> <code><strong>px </strong></code> </a></td> <td>
pivot x</td>
  </tr>  <tr>    <td><a name="SkMatrix_setScale_py"> <code><strong>py </strong></code> </a></td> <td>
pivot y</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4565a0792058178c88e0a129a87272d6"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setScaleX">setScaleX</a> <a href="#SkMatrix_setScaleY">setScaleY</a> <a href="#SkMatrix_MakeScale">MakeScale</a><sup><a href="#SkMatrix_MakeScale_2">[2]</a></sup> <a href="#SkMatrix_preScale">preScale</a><sup><a href="#SkMatrix_preScale_2">[2]</a></sup> <a href="#SkMatrix_postScale">postScale</a><sup><a href="#SkMatrix_postScale_2">[2]</a></sup>

---

<a name="SkMatrix_setScale_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setScale(SkScalar sx, SkScalar sy)
</pre>

Sets <a href="#Matrix">Matrix</a> to scale by <a href="#SkMatrix_setScale_2_sx">sx</a> and <a href="#SkMatrix_setScale_2_sy">sy</a> about at pivot point at (0, 0).

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setScale_2_sx"> <code><strong>sx </strong></code> </a></td> <td>
horizontal scale factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_setScale_2_sy"> <code><strong>sy </strong></code> </a></td> <td>
vertical scale factor</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1579d0cc109c26e69f66f73abd35fb0e"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setScaleX">setScaleX</a> <a href="#SkMatrix_setScaleY">setScaleY</a> <a href="#SkMatrix_MakeScale">MakeScale</a><sup><a href="#SkMatrix_MakeScale_2">[2]</a></sup> <a href="#SkMatrix_preScale">preScale</a><sup><a href="#SkMatrix_preScale_2">[2]</a></sup> <a href="#SkMatrix_postScale">postScale</a><sup><a href="#SkMatrix_postScale_2">[2]</a></sup>

---

<a name="SkMatrix_setRotate"></a>
## setRotate

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setRotate(SkScalar degrees, SkScalar px, SkScalar py)
</pre>

Sets <a href="#Matrix">Matrix</a> to rotate by <a href="#SkMatrix_setRotate_degrees">degrees</a> about a pivot point at (<a href="#SkMatrix_setRotate_px">px</a>, <a href="#SkMatrix_setRotate_py">py</a>).
The pivot point is unchanged when mapped with <a href="#Matrix">Matrix</a>.

Positive <a href="#SkMatrix_setRotate_degrees">degrees</a> rotates clockwise.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setRotate_degrees"> <code><strong>degrees </strong></code> </a></td> <td>
angle of axes relative to upright axes</td>
  </tr>  <tr>    <td><a name="SkMatrix_setRotate_px"> <code><strong>px </strong></code> </a></td> <td>
pivot x</td>
  </tr>  <tr>    <td><a name="SkMatrix_setRotate_py"> <code><strong>py </strong></code> </a></td> <td>
pivot y</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="8c28db3add9cd0177225088f6df6bbb5"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setSinCos">setSinCos</a><sup><a href="#SkMatrix_setSinCos_2">[2]</a></sup> <a href="#SkMatrix_preRotate">preRotate</a><sup><a href="#SkMatrix_preRotate_2">[2]</a></sup> <a href="#SkMatrix_postRotate">postRotate</a><sup><a href="#SkMatrix_postRotate_2">[2]</a></sup>

---

<a name="SkMatrix_setRotate_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setRotate(SkScalar degrees)
</pre>

Sets <a href="#Matrix">Matrix</a> to rotate by <a href="#SkMatrix_setRotate_2_degrees">degrees</a> about a pivot point at (0, 0).
Positive <a href="#SkMatrix_setRotate_2_degrees">degrees</a> rotates clockwise.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setRotate_2_degrees"> <code><strong>degrees </strong></code> </a></td> <td>
angle of axes relative to upright axes</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="93efb9d191bf1b9710c173513e014d6c"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setSinCos">setSinCos</a><sup><a href="#SkMatrix_setSinCos_2">[2]</a></sup> <a href="#SkMatrix_preRotate">preRotate</a><sup><a href="#SkMatrix_preRotate_2">[2]</a></sup> <a href="#SkMatrix_postRotate">postRotate</a><sup><a href="#SkMatrix_postRotate_2">[2]</a></sup>

---

<a name="SkMatrix_setSinCos"></a>
## setSinCos

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setSinCos(SkScalar sinValue, SkScalar cosValue, SkScalar px, SkScalar py)
</pre>

Sets <a href="#Matrix">Matrix</a> to rotate by <a href="#SkMatrix_setSinCos_sinValue">sinValue</a> and <a href="#SkMatrix_setSinCos_cosValue">cosValue</a>, about a pivot point at (<a href="#SkMatrix_setSinCos_px">px</a>, <a href="#SkMatrix_setSinCos_py">py</a>).
The pivot point is unchanged when mapped with <a href="#Matrix">Matrix</a>.

<a href="SkPoint_Reference#Vector">Vector</a> (<a href="#SkMatrix_setSinCos_sinValue">sinValue</a>, <a href="#SkMatrix_setSinCos_cosValue">cosValue</a>) describes the angle of rotation relative to (0, 1).
<a href="SkPoint_Reference#Vector">Vector</a> length specifies scale.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setSinCos_sinValue"> <code><strong>sinValue </strong></code> </a></td> <td>
rotation vector x component</td>
  </tr>  <tr>    <td><a name="SkMatrix_setSinCos_cosValue"> <code><strong>cosValue </strong></code> </a></td> <td>
rotation vector y component</td>
  </tr>  <tr>    <td><a name="SkMatrix_setSinCos_px"> <code><strong>px </strong></code> </a></td> <td>
pivot x</td>
  </tr>  <tr>    <td><a name="SkMatrix_setSinCos_py"> <code><strong>py </strong></code> </a></td> <td>
pivot y</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="187e1d9228e2e4341ef820bd77b6fda9"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setRotate">setRotate</a><sup><a href="#SkMatrix_setRotate_2">[2]</a></sup> <a href="#SkMatrix_setScale">setScale</a><sup><a href="#SkMatrix_setScale_2">[2]</a></sup> <a href="#SkMatrix_setRSXform">setRSXform</a>

---

<a name="SkMatrix_setSinCos_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setSinCos(SkScalar sinValue, SkScalar cosValue)
</pre>

Sets <a href="#Matrix">Matrix</a> to rotate by <a href="#SkMatrix_setSinCos_2_sinValue">sinValue</a> and <a href="#SkMatrix_setSinCos_2_cosValue">cosValue</a>, about a pivot point at (0, 0).

<a href="SkPoint_Reference#Vector">Vector</a> (<a href="#SkMatrix_setSinCos_2_sinValue">sinValue</a>, <a href="#SkMatrix_setSinCos_2_cosValue">cosValue</a>) describes the angle of rotation relative to (0, 1).
<a href="SkPoint_Reference#Vector">Vector</a> length specifies scale.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setSinCos_2_sinValue"> <code><strong>sinValue </strong></code> </a></td> <td>
rotation vector x component</td>
  </tr>  <tr>    <td><a name="SkMatrix_setSinCos_2_cosValue"> <code><strong>cosValue </strong></code> </a></td> <td>
rotation vector y component</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="e37a94a53c959951b059fcd624639ef6"><div><a href="SkCanvas_Reference#Canvas">Canvas</a> needs offset after applying <a href="#Matrix">Matrix</a> to pivot about <a href="SkRect_Reference#Rect">Rect</a> center.</div></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setRotate">setRotate</a><sup><a href="#SkMatrix_setRotate_2">[2]</a></sup> <a href="#SkMatrix_setScale">setScale</a><sup><a href="#SkMatrix_setScale_2">[2]</a></sup> <a href="#SkMatrix_setRSXform">setRSXform</a>

---

<a name="SkMatrix_setRSXform"></a>
## setRSXform

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkMatrix& setRSXform(const SkRSXform& rsxForm)
</pre>

Sets <a href="#Matrix">Matrix</a> to rotate, scale, and translate using a compressed matrix form.

<a href="SkPoint_Reference#Vector">Vector</a> (<a href="#SkMatrix_setRSXform_rsxForm">rsxForm</a>.fSSin, <a href="#SkMatrix_setRSXform_rsxForm">rsxForm</a>.fSCos) describes the angle of rotation relative
to (0, 1). <a href="SkPoint_Reference#Vector">Vector</a> length specifies scale. Mapped point is rotated and scaled
by <a href="SkPoint_Reference#Vector">Vector</a>, then translated by (<a href="#SkMatrix_setRSXform_rsxForm">rsxForm</a>.fTx, <a href="#SkMatrix_setRSXform_rsxForm">rsxForm</a>.fTy).

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setRSXform_rsxForm"> <code><strong>rsxForm </strong></code> </a></td> <td>
compressed <a href="undocumented#RSXform">RSXform</a> matrix</td>
  </tr>
</table>

### Return Value

reference to <a href="#Matrix">Matrix</a>

### Example

<div><fiddle-embed name="c3f5faddca466f78278b32b88fd5f5eb"><div><a href="SkCanvas_Reference#Canvas">Canvas</a> needs offset after applying <a href="#Matrix">Matrix</a> to pivot about <a href="SkRect_Reference#Rect">Rect</a> center.</div></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setSinCos">setSinCos</a><sup><a href="#SkMatrix_setSinCos_2">[2]</a></sup> <a href="#SkMatrix_setScale">setScale</a><sup><a href="#SkMatrix_setScale_2">[2]</a></sup> <a href="#SkMatrix_setTranslate">setTranslate</a><sup><a href="#SkMatrix_setTranslate_2">[2]</a></sup>

---

<a name="SkMatrix_setSkew"></a>
## setSkew

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setSkew(SkScalar kx, SkScalar ky, SkScalar px, SkScalar py)
</pre>

Sets <a href="#Matrix">Matrix</a> to skew by <a href="#SkMatrix_setSkew_kx">kx</a> and <a href="#SkMatrix_setSkew_ky">ky</a>, about a pivot point at (<a href="#SkMatrix_setSkew_px">px</a>, <a href="#SkMatrix_setSkew_py">py</a>).
The pivot point is unchanged when mapped with <a href="#Matrix">Matrix</a>.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setSkew_kx"> <code><strong>kx </strong></code> </a></td> <td>
horizontal skew factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_setSkew_ky"> <code><strong>ky </strong></code> </a></td> <td>
vertical skew factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_setSkew_px"> <code><strong>px </strong></code> </a></td> <td>
pivot x</td>
  </tr>  <tr>    <td><a name="SkMatrix_setSkew_py"> <code><strong>py </strong></code> </a></td> <td>
pivot y</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="55e0431adc6c5b1987ebb8123cc10342"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setSkewX">setSkewX</a> <a href="#SkMatrix_setSkewY">setSkewY</a> <a href="#SkMatrix_preSkew">preSkew</a><sup><a href="#SkMatrix_preSkew_2">[2]</a></sup> <a href="#SkMatrix_postSkew">postSkew</a><sup><a href="#SkMatrix_postSkew_2">[2]</a></sup>

---

<a name="SkMatrix_setSkew_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setSkew(SkScalar kx, SkScalar ky)
</pre>

Sets <a href="#Matrix">Matrix</a> to skew by <a href="#SkMatrix_setSkew_2_kx">kx</a> and <a href="#SkMatrix_setSkew_2_ky">ky</a>, about a pivot point at (0, 0).

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setSkew_2_kx"> <code><strong>kx </strong></code> </a></td> <td>
horizontal skew factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_setSkew_2_ky"> <code><strong>ky </strong></code> </a></td> <td>
vertical skew factor</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="05be7844e9afdd7b9bfc31c5423a70a2"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setSkewX">setSkewX</a> <a href="#SkMatrix_setSkewY">setSkewY</a> <a href="#SkMatrix_preSkew">preSkew</a><sup><a href="#SkMatrix_preSkew_2">[2]</a></sup> <a href="#SkMatrix_postSkew">postSkew</a><sup><a href="#SkMatrix_postSkew_2">[2]</a></sup>

---

<a name="SkMatrix_setConcat"></a>
## setConcat

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setConcat(const SkMatrix& a, const SkMatrix& b)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> <a href="#SkMatrix_setConcat_a">a</a> multiplied by <a href="#Matrix">Matrix</a> <a href="#SkMatrix_setConcat_b">b</a>. Either <a href="#SkMatrix_setConcat_a">a</a> or <a href="#SkMatrix_setConcat_b">b</a> may be this.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    | A B C |      | J K L |
a = | D E F |, b = | M N O |
    | G H I |      | P Q R |</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
        | A B C |   | J K L |   | AJ+BM+CP AK+BN+CQ AL+BO+CR |
a * b = | D E F | * | M N O | = | DJ+EM+FP DK+EN+FQ DL+EO+FR |
        | G H I |   | P Q R |   | GJ+HM+IP GK+HN+IQ GL+HO+IR |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setConcat_a"> <code><strong>a </strong></code> </a></td> <td>
<a href="#Matrix">Matrix</a> on left side of multiply expression</td>
  </tr>  <tr>    <td><a name="SkMatrix_setConcat_b"> <code><strong>b </strong></code> </a></td> <td>
<a href="#Matrix">Matrix</a> on right side of multiply expression</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0381a10ac69bdefdf9d15b47cbb9fefe"><div><a href="#SkMatrix_setPolyToPoly">setPolyToPoly</a> creates perspective matrices, one the inverse of the other.
Multiplying the matrix by its inverse turns into an identity matrix.</div></fiddle-embed></div>

### See Also

<a href="#SkMatrix_Concat">Concat</a> <a href="#SkMatrix_preConcat">preConcat</a> <a href="#SkMatrix_postConcat">postConcat</a> <a href="SkCanvas_Reference#SkCanvas_concat">SkCanvas::concat</a>

---

<a name="SkMatrix_preTranslate"></a>
## preTranslate

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void preTranslate(SkScalar dx, SkScalar dy)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> multiplied by <a href="#Matrix">Matrix</a> constructed from translation (<a href="#SkMatrix_preTranslate_dx">dx</a>, <a href="#SkMatrix_preTranslate_dy">dy</a>).
This can be thought of as moving the point to be mapped before applying <a href="#Matrix">Matrix</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | A B C |               | 1 0 dx |
Matrix = | D E F |,  T(dx, dy) = | 0 1 dy |
         | G H I |               | 0 0  1 |</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
                     | A B C | | 1 0 dx |   | A B A*dx+B*dy+C |
Matrix * T(dx, dy) = | D E F | | 0 1 dy | = | D E D*dx+E*dy+F |
                     | G H I | | 0 0  1 |   | G H G*dx+H*dy+I |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_preTranslate_dx"> <code><strong>dx </strong></code> </a></td> <td>
x translation before applying <a href="#Matrix">Matrix</a></td>
  </tr>  <tr>    <td><a name="SkMatrix_preTranslate_dy"> <code><strong>dy </strong></code> </a></td> <td>
y translation before applying <a href="#Matrix">Matrix</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="08f6749933f4ce541073077ab506fd9b"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_postTranslate">postTranslate</a> <a href="#SkMatrix_setTranslate">setTranslate</a><sup><a href="#SkMatrix_setTranslate_2">[2]</a></sup> <a href="#SkMatrix_MakeTrans">MakeTrans</a>

---

<a name="SkMatrix_preScale"></a>
## preScale

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void preScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> multiplied by <a href="#Matrix">Matrix</a> constructed from scaling by (<a href="#SkMatrix_preScale_sx">sx</a>, <a href="#SkMatrix_preScale_sy">sy</a>)
about pivot point (<a href="#SkMatrix_preScale_px">px</a>, <a href="#SkMatrix_preScale_py">py</a>).
This can be thought of as scaling about a pivot point before applying <a href="#Matrix">Matrix</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | A B C |                       | sx  0 dx |
Matrix = | D E F |,  S(sx, sy, px, py) = |  0 sy dy |
         | G H I |                       |  0  0  1 |</pre>

where

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
dx = px - sx * px
dy = py - sy * py</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
                             | A B C | | sx  0 dx |   | A*sx B*sy A*dx+B*dy+C |
Matrix * S(sx, sy, px, py) = | D E F | |  0 sy dy | = | D*sx E*sy D*dx+E*dy+F |
                             | G H I | |  0  0  1 |   | G*sx H*sy G*dx+H*dy+I |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_preScale_sx"> <code><strong>sx </strong></code> </a></td> <td>
horizontal scale factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_preScale_sy"> <code><strong>sy </strong></code> </a></td> <td>
vertical scale factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_preScale_px"> <code><strong>px </strong></code> </a></td> <td>
pivot x</td>
  </tr>  <tr>    <td><a name="SkMatrix_preScale_py"> <code><strong>py </strong></code> </a></td> <td>
pivot y</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="2531f8d1e05d7b6dc22f3efcd2fb84e4"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_postScale">postScale</a><sup><a href="#SkMatrix_postScale_2">[2]</a></sup> <a href="#SkMatrix_setScale">setScale</a><sup><a href="#SkMatrix_setScale_2">[2]</a></sup> <a href="#SkMatrix_MakeScale">MakeScale</a><sup><a href="#SkMatrix_MakeScale_2">[2]</a></sup>

---

<a name="SkMatrix_preScale_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void preScale(SkScalar sx, SkScalar sy)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> multiplied by <a href="#Matrix">Matrix</a> constructed from scaling by (<a href="#SkMatrix_preScale_2_sx">sx</a>, <a href="#SkMatrix_preScale_2_sy">sy</a>)
about pivot point (0, 0).
This can be thought of as scaling about the origin before applying <a href="#Matrix">Matrix</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | A B C |               | sx  0  0 |
Matrix = | D E F |,  S(sx, sy) = |  0 sy  0 |
         | G H I |               |  0  0  1 |</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
                     | A B C | | sx  0  0 |   | A*sx B*sy C |
Matrix * S(sx, sy) = | D E F | |  0 sy  0 | = | D*sx E*sy F |
                     | G H I | |  0  0  1 |   | G*sx H*sy I |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_preScale_2_sx"> <code><strong>sx </strong></code> </a></td> <td>
horizontal scale factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_preScale_2_sy"> <code><strong>sy </strong></code> </a></td> <td>
vertical scale factor</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3edbdea8e43d06086abf33ec4a9b415b"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_postScale">postScale</a><sup><a href="#SkMatrix_postScale_2">[2]</a></sup> <a href="#SkMatrix_setScale">setScale</a><sup><a href="#SkMatrix_setScale_2">[2]</a></sup> <a href="#SkMatrix_MakeScale">MakeScale</a><sup><a href="#SkMatrix_MakeScale_2">[2]</a></sup>

---

<a name="SkMatrix_preRotate"></a>
## preRotate

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void preRotate(SkScalar degrees, SkScalar px, SkScalar py)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> multiplied by <a href="#Matrix">Matrix</a> constructed from rotating by <a href="#SkMatrix_preRotate_degrees">degrees</a>
about pivot point (<a href="#SkMatrix_preRotate_px">px</a>, <a href="#SkMatrix_preRotate_py">py</a>).
This can be thought of as rotating about a pivot point before applying <a href="#Matrix">Matrix</a>.

Positive <a href="#SkMatrix_preRotate_degrees">degrees</a> rotates clockwise.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | A B C |                        | c -s dx |
Matrix = | D E F |,  R(degrees, px, py) = | s  c dy |
         | G H I |                        | 0  0  1 |</pre>

where

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
c  = cos(degrees)
s  = sin(degrees)
dx =  s * py + (1 - c) * px
dy = -s * px + (1 - c) * py</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
                              | A B C | | c -s dx |   | Ac+Bs -As+Bc A*dx+B*dy+C |
Matrix * R(degrees, px, py) = | D E F | | s  c dy | = | Dc+Es -Ds+Ec D*dx+E*dy+F |
                              | G H I | | 0  0  1 |   | Gc+Hs -Gs+Hc G*dx+H*dy+I |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_preRotate_degrees"> <code><strong>degrees </strong></code> </a></td> <td>
angle of axes relative to upright axes</td>
  </tr>  <tr>    <td><a name="SkMatrix_preRotate_px"> <code><strong>px </strong></code> </a></td> <td>
pivot x</td>
  </tr>  <tr>    <td><a name="SkMatrix_preRotate_py"> <code><strong>py </strong></code> </a></td> <td>
pivot y</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a70bb18d67c06a20ab514e7a47924e5a"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_postRotate">postRotate</a><sup><a href="#SkMatrix_postRotate_2">[2]</a></sup> <a href="#SkMatrix_setRotate">setRotate</a><sup><a href="#SkMatrix_setRotate_2">[2]</a></sup>

---

<a name="SkMatrix_preRotate_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void preRotate(SkScalar degrees)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> multiplied by <a href="#Matrix">Matrix</a> constructed from rotating by <a href="#SkMatrix_preRotate_2_degrees">degrees</a>
about pivot point (0, 0).
This can be thought of as rotating about the origin before applying <a href="#Matrix">Matrix</a>.

Positive <a href="#SkMatrix_preRotate_2_degrees">degrees</a> rotates clockwise.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | A B C |                        | c -s 0 |
Matrix = | D E F |,  R(degrees, px, py) = | s  c 0 |
         | G H I |                        | 0  0 1 |</pre>

where

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
c  = cos(degrees)
s  = sin(degrees)</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
                              | A B C | | c -s 0 |   | Ac+Bs -As+Bc C |
Matrix * R(degrees, px, py) = | D E F | | s  c 0 | = | Dc+Es -Ds+Ec F |
                              | G H I | | 0  0 1 |   | Gc+Hs -Gs+Hc I |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_preRotate_2_degrees"> <code><strong>degrees </strong></code> </a></td> <td>
angle of axes relative to upright axes</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="5acd49bd931c79a808dd6c7cc0e92f72"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_postRotate">postRotate</a><sup><a href="#SkMatrix_postRotate_2">[2]</a></sup> <a href="#SkMatrix_setRotate">setRotate</a><sup><a href="#SkMatrix_setRotate_2">[2]</a></sup>

---

<a name="SkMatrix_preSkew"></a>
## preSkew

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void preSkew(SkScalar kx, SkScalar ky, SkScalar px, SkScalar py)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> multiplied by <a href="#Matrix">Matrix</a> constructed from skewing by (<a href="#SkMatrix_preSkew_kx">kx</a>, <a href="#SkMatrix_preSkew_ky">ky</a>)
about pivot point (<a href="#SkMatrix_preSkew_px">px</a>, <a href="#SkMatrix_preSkew_py">py</a>).
This can be thought of as skewing about a pivot point before applying <a href="#Matrix">Matrix</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | A B C |                       |  1 kx dx |
Matrix = | D E F |,  K(kx, ky, px, py) = | ky  1 dy |
         | G H I |                       |  0  0  1 |</pre>

where

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
dx = -kx * py
dy = -ky * px</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
                             | A B C | |  1 kx dx |   | A+B*ky A*kx+B A*dx+B*dy+C |
Matrix * K(kx, ky, px, py) = | D E F | | ky  1 dy | = | D+E*ky D*kx+E D*dx+E*dy+F |
                             | G H I | |  0  0  1 |   | G+H*ky G*kx+H G*dx+H*dy+I |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_preSkew_kx"> <code><strong>kx </strong></code> </a></td> <td>
horizontal skew factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_preSkew_ky"> <code><strong>ky </strong></code> </a></td> <td>
vertical skew factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_preSkew_px"> <code><strong>px </strong></code> </a></td> <td>
pivot x</td>
  </tr>  <tr>    <td><a name="SkMatrix_preSkew_py"> <code><strong>py </strong></code> </a></td> <td>
pivot y</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="199a18ad61d702664ce6df1d7037aa48"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_postSkew">postSkew</a><sup><a href="#SkMatrix_postSkew_2">[2]</a></sup> <a href="#SkMatrix_setSkew">setSkew</a><sup><a href="#SkMatrix_setSkew_2">[2]</a></sup>

---

<a name="SkMatrix_preSkew_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void preSkew(SkScalar kx, SkScalar ky)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> multiplied by <a href="#Matrix">Matrix</a> constructed from skewing by (<a href="#SkMatrix_preSkew_2_kx">kx</a>, <a href="#SkMatrix_preSkew_2_ky">ky</a>)
about pivot point (0, 0).
This can be thought of as skewing about the origin before applying <a href="#Matrix">Matrix</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | A B C |               |  1 kx 0 |
Matrix = | D E F |,  K(kx, ky) = | ky  1 0 |
         | G H I |               |  0  0 1 |</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
                     | A B C | |  1 kx 0 |   | A+B*ky A*kx+B C |
Matrix * K(kx, ky) = | D E F | | ky  1 0 | = | D+E*ky D*kx+E F |
                     | G H I | |  0  0 1 |   | G+H*ky G*kx+H I |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_preSkew_2_kx"> <code><strong>kx </strong></code> </a></td> <td>
horizontal skew factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_preSkew_2_ky"> <code><strong>ky </strong></code> </a></td> <td>
vertical skew factor</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="e100c543869fe8fd516ba69de79444ba"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_postSkew">postSkew</a><sup><a href="#SkMatrix_postSkew_2">[2]</a></sup> <a href="#SkMatrix_setSkew">setSkew</a><sup><a href="#SkMatrix_setSkew_2">[2]</a></sup>

---

<a name="SkMatrix_preConcat"></a>
## preConcat

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void preConcat(const SkMatrix& other)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> multiplied by <a href="#Matrix">Matrix</a> <a href="#SkMatrix_preConcat_other">other</a>.
This can be thought of mapping by <a href="#SkMatrix_preConcat_other">other</a> before applying <a href="#Matrix">Matrix</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | A B C |          | J K L |
Matrix = | D E F |, other = | M N O |
         | G H I |          | P Q R |</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
                 | A B C |   | J K L |   | AJ+BM+CP AK+BN+CQ AL+BO+CR |
Matrix * other = | D E F | * | M N O | = | DJ+EM+FP DK+EN+FQ DL+EO+FR |
                 | G H I |   | P Q R |   | GJ+HM+IP GK+HN+IQ GL+HO+IR |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_preConcat_other"> <code><strong>other </strong></code> </a></td> <td>
<a href="#Matrix">Matrix</a> on right side of multiply expression</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="b07e62298e7b0ab5683db199faffceb2"><div><a href="#SkMatrix_setPolyToPoly">setPolyToPoly</a> creates perspective matrices, one the inverse of the <a href="#SkMatrix_preConcat_other">other</a>.
Multiplying the matrix by its inverse turns into an identity matrix.</div></fiddle-embed></div>

### See Also

<a href="#SkMatrix_postConcat">postConcat</a> <a href="#SkMatrix_setConcat">setConcat</a> <a href="#SkMatrix_Concat">Concat</a>

---

<a name="SkMatrix_postTranslate"></a>
## postTranslate

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void postTranslate(SkScalar dx, SkScalar dy)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> constructed from translation (<a href="#SkMatrix_postTranslate_dx">dx</a>, <a href="#SkMatrix_postTranslate_dy">dy</a>) multiplied by <a href="#Matrix">Matrix</a>.
This can be thought of as moving the point to be mapped after applying <a href="#Matrix">Matrix</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | J K L |               | 1 0 dx |
Matrix = | M N O |,  T(dx, dy) = | 0 1 dy |
         | P Q R |               | 0 0  1 |</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
                     | 1 0 dx | | J K L |   | J+dx*P K+dx*Q L+dx*R |
T(dx, dy) * Matrix = | 0 1 dy | | M N O | = | M+dy*P N+dy*Q O+dy*R |
                     | 0 0  1 | | P Q R |   |      P      Q      R |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_postTranslate_dx"> <code><strong>dx </strong></code> </a></td> <td>
x translation after applying <a href="#Matrix">Matrix</a></td>
  </tr>  <tr>    <td><a name="SkMatrix_postTranslate_dy"> <code><strong>dy </strong></code> </a></td> <td>
y translation after applying <a href="#Matrix">Matrix</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="57e4cff302c0d754ac6c66050d741772"><div>Compare with <a href="#SkMatrix_preTranslate">preTranslate</a> example.</div></fiddle-embed></div>

### See Also

<a href="#SkMatrix_preTranslate">preTranslate</a> <a href="#SkMatrix_setTranslate">setTranslate</a><sup><a href="#SkMatrix_setTranslate_2">[2]</a></sup> <a href="#SkMatrix_MakeTrans">MakeTrans</a>

---

<a name="SkMatrix_postScale"></a>
## postScale

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void postScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> constructed from scaling by (<a href="#SkMatrix_postScale_sx">sx</a>, <a href="#SkMatrix_postScale_sy">sy</a>) about pivot point
(<a href="#SkMatrix_postScale_px">px</a>, <a href="#SkMatrix_postScale_py">py</a>), multiplied by <a href="#Matrix">Matrix</a>.
This can be thought of as scaling about a pivot point after applying <a href="#Matrix">Matrix</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | J K L |                       | sx  0 dx |
Matrix = | M N O |,  S(sx, sy, px, py) = |  0 sy dy |
         | P Q R |                       |  0  0  1 |</pre>

where

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
dx = px - sx * px
dy = py - sy * py</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
                             | sx  0 dx | | J K L |   | sx*J+dx*P sx*K+dx*Q sx*L+dx+R |
S(sx, sy, px, py) * Matrix = |  0 sy dy | | M N O | = | sy*M+dy*P sy*N+dy*Q sy*O+dy*R |
                             |  0  0  1 | | P Q R |   |         P         Q         R |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_postScale_sx"> <code><strong>sx </strong></code> </a></td> <td>
horizontal scale factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_postScale_sy"> <code><strong>sy </strong></code> </a></td> <td>
vertical scale factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_postScale_px"> <code><strong>px </strong></code> </a></td> <td>
pivot x</td>
  </tr>  <tr>    <td><a name="SkMatrix_postScale_py"> <code><strong>py </strong></code> </a></td> <td>
pivot y</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ed3aa18ba0ea95c85cc49aa3829fe384"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_preScale">preScale</a><sup><a href="#SkMatrix_preScale_2">[2]</a></sup> <a href="#SkMatrix_setScale">setScale</a><sup><a href="#SkMatrix_setScale_2">[2]</a></sup> <a href="#SkMatrix_MakeScale">MakeScale</a><sup><a href="#SkMatrix_MakeScale_2">[2]</a></sup>

---

<a name="SkMatrix_postScale_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void postScale(SkScalar sx, SkScalar sy)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> constructed from scaling by (<a href="#SkMatrix_postScale_2_sx">sx</a>, <a href="#SkMatrix_postScale_2_sy">sy</a>) about pivot point
(0, 0), multiplied by <a href="#Matrix">Matrix</a>.
This can be thought of as scaling about the origin after applying <a href="#Matrix">Matrix</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | J K L |               | sx  0  0 |
Matrix = | M N O |,  S(sx, sy) = |  0 sy  0 |
         | P Q R |               |  0  0  1 |</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
                     | sx  0  0 | | J K L |   | sx*J sx*K sx*L |
S(sx, sy) * Matrix = |  0 sy  0 | | M N O | = | sy*M sy*N sy*O |
                     |  0  0  1 | | P Q R |   |    P    Q    R |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_postScale_2_sx"> <code><strong>sx </strong></code> </a></td> <td>
horizontal scale factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_postScale_2_sy"> <code><strong>sy </strong></code> </a></td> <td>
vertical scale factor</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1931017698766a67d3a26423453b8095"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_preScale">preScale</a><sup><a href="#SkMatrix_preScale_2">[2]</a></sup> <a href="#SkMatrix_setScale">setScale</a><sup><a href="#SkMatrix_setScale_2">[2]</a></sup> <a href="#SkMatrix_MakeScale">MakeScale</a><sup><a href="#SkMatrix_MakeScale_2">[2]</a></sup>

---

<a name="SkMatrix_postIDiv"></a>
## postIDiv

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool postIDiv(int divx, int divy)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> constructed from scaling by(1/<a href="#SkMatrix_postIDiv_divx">divx</a>, 1/<a href="#SkMatrix_postIDiv_divy">divy</a>)about pivot point (px, py), multiplied by <a href="#Matrix">Matrix</a>.

Returns false if either <a href="#SkMatrix_postIDiv_divx">divx</a> or <a href="#SkMatrix_postIDiv_divy">divy</a> is zero.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | J K L |                   | sx  0  0 |
Matrix = | M N O |,  I(divx, divy) = |  0 sy  0 |
         | P Q R |                   |  0  0  1 |</pre>

where

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
sx = 1 / divx
sy = 1 / divy</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
                         | sx  0  0 | | J K L |   | sx*J sx*K sx*L |
I(divx, divy) * Matrix = |  0 sy  0 | | M N O | = | sy*M sy*N sy*O |
                         |  0  0  1 | | P Q R |   |    P    Q    R |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_postIDiv_divx"> <code><strong>divx </strong></code> </a></td> <td>
integer divisor for inverse scale in x</td>
  </tr>  <tr>    <td><a name="SkMatrix_postIDiv_divy"> <code><strong>divy </strong></code> </a></td> <td>
integer divisor for inverse scale in y</td>
  </tr>
</table>

### Return Value

true on successful scale

### Example

<div><fiddle-embed name="58c844b8f0c36acdbc8211e8c929c253"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_postScale">postScale</a><sup><a href="#SkMatrix_postScale_2">[2]</a></sup> <a href="#SkMatrix_MakeScale">MakeScale</a><sup><a href="#SkMatrix_MakeScale_2">[2]</a></sup>

---

<a name="SkMatrix_postRotate"></a>
## postRotate

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void postRotate(SkScalar degrees, SkScalar px, SkScalar py)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> constructed from rotating by <a href="#SkMatrix_postRotate_degrees">degrees</a> about pivot point
(<a href="#SkMatrix_postRotate_px">px</a>, <a href="#SkMatrix_postRotate_py">py</a>), multiplied by <a href="#Matrix">Matrix</a>.
This can be thought of as rotating about a pivot point after applying <a href="#Matrix">Matrix</a>.

Positive <a href="#SkMatrix_postRotate_degrees">degrees</a> rotates clockwise.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | J K L |                        | c -s dx |
Matrix = | M N O |,  R(degrees, px, py) = | s  c dy |
         | P Q R |                        | 0  0  1 |</pre>

where

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
c  = cos(degrees)
s  = sin(degrees)
dx =  s * py + (1 - c) * px
dy = -s * px + (1 - c) * py</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
                              |c -s dx| |J K L|   |cJ-sM+dx*P cK-sN+dx*Q cL-sO+dx+R|
R(degrees, px, py) * Matrix = |s  c dy| |M N O| = |sJ+cM+dy*P sK+cN+dy*Q sL+cO+dy*R|
                              |0  0  1| |P Q R|   |         P          Q          R|</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_postRotate_degrees"> <code><strong>degrees </strong></code> </a></td> <td>
angle of axes relative to upright axes</td>
  </tr>  <tr>    <td><a name="SkMatrix_postRotate_px"> <code><strong>px </strong></code> </a></td> <td>
pivot x</td>
  </tr>  <tr>    <td><a name="SkMatrix_postRotate_py"> <code><strong>py </strong></code> </a></td> <td>
pivot y</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="e09194ee48a81e7b375ade473d340f0d"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_preRotate">preRotate</a><sup><a href="#SkMatrix_preRotate_2">[2]</a></sup> <a href="#SkMatrix_setRotate">setRotate</a><sup><a href="#SkMatrix_setRotate_2">[2]</a></sup>

---

<a name="SkMatrix_postRotate_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void postRotate(SkScalar degrees)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> constructed from rotating by <a href="#SkMatrix_postRotate_2_degrees">degrees</a> about pivot point
(0, 0), multiplied by <a href="#Matrix">Matrix</a>.
This can be thought of as rotating about the origin after applying <a href="#Matrix">Matrix</a>.

Positive <a href="#SkMatrix_postRotate_2_degrees">degrees</a> rotates clockwise.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | J K L |                        | c -s 0 |
Matrix = | M N O |,  R(degrees, px, py) = | s  c 0 |
         | P Q R |                        | 0  0 1 |</pre>

where

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
c  = cos(degrees)
s  = sin(degrees)</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
                              | c -s dx | | J K L |   | cJ-sM cK-sN cL-sO |
R(degrees, px, py) * Matrix = | s  c dy | | M N O | = | sJ+cM sK+cN sL+cO |
                              | 0  0  1 | | P Q R |   |     P     Q     R |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_postRotate_2_degrees"> <code><strong>degrees </strong></code> </a></td> <td>
angle of axes relative to upright axes</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="52e4c53e26971af5576b30de60fa70c2"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_preRotate">preRotate</a><sup><a href="#SkMatrix_preRotate_2">[2]</a></sup> <a href="#SkMatrix_setRotate">setRotate</a><sup><a href="#SkMatrix_setRotate_2">[2]</a></sup>

---

<a name="SkMatrix_postSkew"></a>
## postSkew

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void postSkew(SkScalar kx, SkScalar ky, SkScalar px, SkScalar py)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> constructed from skewing by (<a href="#SkMatrix_postSkew_kx">kx</a>, <a href="#SkMatrix_postSkew_ky">ky</a>) about pivot point
(<a href="#SkMatrix_postSkew_px">px</a>, <a href="#SkMatrix_postSkew_py">py</a>), multiplied by <a href="#Matrix">Matrix</a>.
This can be thought of as skewing about a pivot point after applying <a href="#Matrix">Matrix</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | J K L |                       |  1 kx dx |
Matrix = | M N O |,  K(kx, ky, px, py) = | ky  1 dy |
         | P Q R |                       |  0  0  1 |</pre>

where

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
dx = -kx * py
dy = -ky * px</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
                             | 1 kx dx| |J K L|   |J+kx*M+dx*P K+kx*N+dx*Q L+kx*O+dx+R|
K(kx, ky, px, py) * Matrix = |ky  1 dy| |M N O| = |ky*J+M+dy*P ky*K+N+dy*Q ky*L+O+dy*R|
                             | 0  0  1| |P Q R|   |          P           Q           R|</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_postSkew_kx"> <code><strong>kx </strong></code> </a></td> <td>
horizontal skew factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_postSkew_ky"> <code><strong>ky </strong></code> </a></td> <td>
vertical skew factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_postSkew_px"> <code><strong>px </strong></code> </a></td> <td>
pivot x</td>
  </tr>  <tr>    <td><a name="SkMatrix_postSkew_py"> <code><strong>py </strong></code> </a></td> <td>
pivot y</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="8c34ae3a2b7e2742bb969819737365ec"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_preSkew">preSkew</a><sup><a href="#SkMatrix_preSkew_2">[2]</a></sup> <a href="#SkMatrix_setSkew">setSkew</a><sup><a href="#SkMatrix_setSkew_2">[2]</a></sup>

---

<a name="SkMatrix_postSkew_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void postSkew(SkScalar kx, SkScalar ky)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> constructed from skewing by (<a href="#SkMatrix_postSkew_2_kx">kx</a>, <a href="#SkMatrix_postSkew_2_ky">ky</a>) about pivot point
(0, 0), multiplied by <a href="#Matrix">Matrix</a>.
This can be thought of as skewing about the origin after applying <a href="#Matrix">Matrix</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | J K L |               |  1 kx 0 |
Matrix = | M N O |,  K(kx, ky) = | ky  1 0 |
         | P Q R |               |  0  0 1 |</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
                     |  1 kx 0 | | J K L |   | J+kx*M K+kx*N L+kx*O |
K(kx, ky) * Matrix = | ky  1 0 | | M N O | = | ky*J+M ky*K+N ky*L+O |
                     |  0  0 1 | | P Q R |   |      P      Q      R |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_postSkew_2_kx"> <code><strong>kx </strong></code> </a></td> <td>
horizontal skew factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_postSkew_2_ky"> <code><strong>ky </strong></code> </a></td> <td>
vertical skew factor</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3aa2603225dff72ac53dd359f897f494"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_preSkew">preSkew</a><sup><a href="#SkMatrix_preSkew_2">[2]</a></sup> <a href="#SkMatrix_setSkew">setSkew</a><sup><a href="#SkMatrix_setSkew_2">[2]</a></sup>

---

<a name="SkMatrix_postConcat"></a>
## postConcat

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void postConcat(const SkMatrix& other)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> <a href="#SkMatrix_postConcat_other">other</a> multiplied by <a href="#Matrix">Matrix</a>.
This can be thought of mapping by <a href="#SkMatrix_postConcat_other">other</a> after applying <a href="#Matrix">Matrix</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | J K L |           | A B C |
Matrix = | M N O |,  other = | D E F |
         | P Q R |           | G H I |</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
                 | A B C |   | J K L |   | AJ+BM+CP AK+BN+CQ AL+BO+CR |
other * Matrix = | D E F | * | M N O | = | DJ+EM+FP DK+EN+FQ DL+EO+FR |
                 | G H I |   | P Q R |   | GJ+HM+IP GK+HN+IQ GL+HO+IR |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_postConcat_other"> <code><strong>other </strong></code> </a></td> <td>
<a href="#Matrix">Matrix</a> on left side of multiply expression</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0a4214289249c77f48d59227c4ac4d9e"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_preConcat">preConcat</a> <a href="#SkMatrix_setConcat">setConcat</a> <a href="#SkMatrix_Concat">Concat</a>

---

## <a name="SkMatrix_ScaleToFit"></a> Enum SkMatrix::ScaleToFit

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
enum <a href="#SkMatrix_ScaleToFit">ScaleToFit</a> {
<a href="#SkMatrix_kFill_ScaleToFit">kFill ScaleToFit</a>,
<a href="#SkMatrix_kStart_ScaleToFit">kStart ScaleToFit</a>,
<a href="#SkMatrix_kCenter_ScaleToFit">kCenter ScaleToFit</a>,
<a href="#SkMatrix_kEnd_ScaleToFit">kEnd ScaleToFit</a>,
};</pre>

<a href="#SkMatrix_ScaleToFit">ScaleToFit</a> describes how <a href="#Matrix">Matrix</a> is constructed to map one <a href="SkRect_Reference#Rect">Rect</a> to another.
<a href="#SkMatrix_ScaleToFit">ScaleToFit</a> may allow <a href="#Matrix">Matrix</a> to have unequal horizontal and vertical scaling,
or may restrict <a href="#Matrix">Matrix</a> to square scaling. If restricted, <a href="#SkMatrix_ScaleToFit">ScaleToFit</a> specifies
how <a href="#Matrix">Matrix</a> maps to the side or center of the destination <a href="SkRect_Reference#Rect">Rect</a>.

### Constants

<table>
  <tr>
    <td><a name="SkMatrix_kFill_ScaleToFit"> <code><strong>SkMatrix::kFill_ScaleToFit </strong></code> </a></td><td>0</td><td>Computes <a href="#Matrix">Matrix</a> that scales in x and y independently, so that source <a href="SkRect_Reference#Rect">Rect</a> is
mapped to completely fill destination <a href="SkRect_Reference#Rect">Rect</a>. The aspect ratio of source <a href="SkRect_Reference#Rect">Rect</a>
may change.</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kStart_ScaleToFit"> <code><strong>SkMatrix::kStart_ScaleToFit </strong></code> </a></td><td>1</td><td>Computes <a href="#Matrix">Matrix</a> that maintains source <a href="SkRect_Reference#Rect">Rect</a> aspect ratio, mapping source <a href="SkRect_Reference#Rect">Rect</a>
width or height to destination <a href="SkRect_Reference#Rect">Rect</a>. Aligns mapping to left and top edges
of destination <a href="SkRect_Reference#Rect">Rect</a>.</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kCenter_ScaleToFit"> <code><strong>SkMatrix::kCenter_ScaleToFit </strong></code> </a></td><td>2</td><td>Computes <a href="#Matrix">Matrix</a> that maintains source <a href="SkRect_Reference#Rect">Rect</a> aspect ratio, mapping source <a href="SkRect_Reference#Rect">Rect</a>
width or height to destination <a href="SkRect_Reference#Rect">Rect</a>. Aligns mapping to center of destination
<a href="SkRect_Reference#Rect">Rect</a>.</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kEnd_ScaleToFit"> <code><strong>SkMatrix::kEnd_ScaleToFit </strong></code> </a></td><td>3</td><td>Computes <a href="#Matrix">Matrix</a> that maintains source <a href="SkRect_Reference#Rect">Rect</a> aspect ratio, mapping source <a href="SkRect_Reference#Rect">Rect</a>
width or height to destination <a href="SkRect_Reference#Rect">Rect</a>. Aligns mapping to right and bottom
edges of destination <a href="SkRect_Reference#Rect">Rect</a>.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="17c3070b31b700ea8f52e48af9a66b6e"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setRectToRect">setRectToRect</a> <a href="#SkMatrix_MakeRectToRect">MakeRectToRect</a> <a href="#SkMatrix_setPolyToPoly">setPolyToPoly</a>



<a name="SkMatrix_setRectToRect"></a>
## setRectToRect

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool setRectToRect(const SkRect& src, const SkRect& dst, ScaleToFit stf)
</pre>

Sets <a href="#Matrix">Matrix</a> to scale and translate <a href="#SkMatrix_setRectToRect_src">src</a> <a href="SkRect_Reference#Rect">Rect</a> to <a href="#SkMatrix_setRectToRect_dst">dst</a> <a href="SkRect_Reference#Rect">Rect</a>. <a href="#SkMatrix_setRectToRect_stf">stf</a> selects whether
mapping completely fills <a href="#SkMatrix_setRectToRect_dst">dst</a> or preserves the aspect ratio, and how to align
<a href="#SkMatrix_setRectToRect_src">src</a> within <a href="#SkMatrix_setRectToRect_dst">dst</a>. Returns false if <a href="#SkMatrix_setRectToRect_src">src</a> is empty, and sets <a href="#Matrix">Matrix</a> to identity.
Returns true if <a href="#SkMatrix_setRectToRect_dst">dst</a> is empty, and sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
| 0 0 0 |
| 0 0 0 |
| 0 0 1 |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setRectToRect_src"> <code><strong>src </strong></code> </a></td> <td>
<a href="SkRect_Reference#Rect">Rect</a> to map from</td>
  </tr>  <tr>    <td><a name="SkMatrix_setRectToRect_dst"> <code><strong>dst </strong></code> </a></td> <td>
<a href="SkRect_Reference#Rect">Rect</a> to map to</td>
  </tr>  <tr>    <td><a name="SkMatrix_setRectToRect_stf"> <code><strong>stf </strong></code> </a></td> <td>
one of: <a href="#SkMatrix_kFill_ScaleToFit">kFill ScaleToFit</a>, <a href="#SkMatrix_kStart_ScaleToFit">kStart ScaleToFit</a>,
<a href="#SkMatrix_kCenter_ScaleToFit">kCenter ScaleToFit</a>, <a href="#SkMatrix_kEnd_ScaleToFit">kEnd ScaleToFit</a></td>
  </tr>
</table>

### Return Value

true if <a href="#Matrix">Matrix</a> can represent <a href="SkRect_Reference#Rect">Rect</a> mapping

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

<a href="#SkMatrix_MakeRectToRect">MakeRectToRect</a> <a href="#SkMatrix_ScaleToFit">ScaleToFit</a> <a href="#SkMatrix_setPolyToPoly">setPolyToPoly</a> <a href="SkRect_Reference#SkRect_isEmpty">SkRect::isEmpty</a>

---

<a name="SkMatrix_MakeRectToRect"></a>
## MakeRectToRect

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static SkMatrix MakeRectToRect(const SkRect& src, const SkRect& dst, ScaleToFit stf)
</pre>

Returns <a href="#Matrix">Matrix</a> set to scale and translate <a href="#SkMatrix_MakeRectToRect_src">src</a> <a href="SkRect_Reference#Rect">Rect</a> to <a href="#SkMatrix_MakeRectToRect_dst">dst</a> <a href="SkRect_Reference#Rect">Rect</a>. <a href="#SkMatrix_MakeRectToRect_stf">stf</a> selects
whether mapping completely fills <a href="#SkMatrix_MakeRectToRect_dst">dst</a> or preserves the aspect ratio, and how to
align <a href="#SkMatrix_MakeRectToRect_src">src</a> within <a href="#SkMatrix_MakeRectToRect_dst">dst</a>. Returns the identity <a href="#Matrix">Matrix</a> if <a href="#SkMatrix_MakeRectToRect_src">src</a> is empty. If <a href="#SkMatrix_MakeRectToRect_dst">dst</a> is
empty, returns <a href="#Matrix">Matrix</a> set to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
| 0 0 0 |
| 0 0 0 |
| 0 0 1 |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_MakeRectToRect_src"> <code><strong>src </strong></code> </a></td> <td>
<a href="SkRect_Reference#Rect">Rect</a> to map from</td>
  </tr>  <tr>    <td><a name="SkMatrix_MakeRectToRect_dst"> <code><strong>dst </strong></code> </a></td> <td>
<a href="SkRect_Reference#Rect">Rect</a> to map to</td>
  </tr>  <tr>    <td><a name="SkMatrix_MakeRectToRect_stf"> <code><strong>stf </strong></code> </a></td> <td>
one of: <a href="#SkMatrix_kFill_ScaleToFit">kFill ScaleToFit</a>, <a href="#SkMatrix_kStart_ScaleToFit">kStart ScaleToFit</a>,
<a href="#SkMatrix_kCenter_ScaleToFit">kCenter ScaleToFit</a>, <a href="#SkMatrix_kEnd_ScaleToFit">kEnd ScaleToFit</a></td>
  </tr>
</table>

### Return Value

<a href="#Matrix">Matrix</a> mapping <a href="#SkMatrix_MakeRectToRect_src">src</a> to <a href="#SkMatrix_MakeRectToRect_dst">dst</a>

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

<a href="#SkMatrix_setRectToRect">setRectToRect</a> <a href="#SkMatrix_ScaleToFit">ScaleToFit</a> <a href="#SkMatrix_setPolyToPoly">setPolyToPoly</a> <a href="SkRect_Reference#SkRect_isEmpty">SkRect::isEmpty</a>

---

<a name="SkMatrix_setPolyToPoly"></a>
## setPolyToPoly

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool setPolyToPoly(const SkPoint src[], const SkPoint dst[], int count)
</pre>

Sets <a href="#Matrix">Matrix</a> to map <a href="#SkMatrix_setPolyToPoly_src">src</a> to <a href="#SkMatrix_setPolyToPoly_dst">dst</a>. <a href="#SkMatrix_setPolyToPoly_count">count</a> must be zero or greater, and four or less.

If <a href="#SkMatrix_setPolyToPoly_count">count</a> is zero, sets <a href="#Matrix">Matrix</a> to identity and returns true.
If <a href="#SkMatrix_setPolyToPoly_count">count</a> is one, sets <a href="#Matrix">Matrix</a> to translate and returns true.
If <a href="#SkMatrix_setPolyToPoly_count">count</a> is two or more, sets <a href="#Matrix">Matrix</a> to map <a href="SkPoint_Reference#Point">Points</a> if possible; returns false
if <a href="#Matrix">Matrix</a> cannot be constructed. If <a href="#SkMatrix_setPolyToPoly_count">count</a> is four, <a href="#Matrix">Matrix</a> may include
perspective.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setPolyToPoly_src"> <code><strong>src </strong></code> </a></td> <td>
<a href="SkPoint_Reference#Point">Points</a> to map from</td>
  </tr>  <tr>    <td><a name="SkMatrix_setPolyToPoly_dst"> <code><strong>dst </strong></code> </a></td> <td>
<a href="SkPoint_Reference#Point">Points</a> to map to</td>
  </tr>  <tr>    <td><a name="SkMatrix_setPolyToPoly_count"> <code><strong>count </strong></code> </a></td> <td>
number of <a href="SkPoint_Reference#Point">Points</a> in <a href="#SkMatrix_setPolyToPoly_src">src</a> and <a href="#SkMatrix_setPolyToPoly_dst">dst</a></td>
  </tr>
</table>

### Return Value

true if <a href="#Matrix">Matrix</a> was constructed successfully

### Example

<div><fiddle-embed name="c851d1313e8909aaea4f0591699fdb7b"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setRectToRect">setRectToRect</a> <a href="#SkMatrix_MakeRectToRect">MakeRectToRect</a>

---

<a name="SkMatrix_invert"></a>
## invert

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool SK_WARN_UNUSED_RESULT invert(SkMatrix* inverse) const
</pre>

Sets <a href="#SkMatrix_invert_inverse">inverse</a> to reciprocal matrix, returning true if <a href="#Matrix">Matrix</a> can be inverted.
Geometrically, if <a href="#Matrix">Matrix</a> maps from source to destination, <a href="#SkMatrix_invert_inverse">inverse</a> <a href="#Matrix">Matrix</a>
maps from destination to source. If <a href="#Matrix">Matrix</a> can not be inverted, <a href="#SkMatrix_invert_inverse">inverse</a> is
unchanged.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_invert_inverse"> <code><strong>inverse </strong></code> </a></td> <td>
storage for inverted <a href="#Matrix">Matrix</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

true if <a href="#Matrix">Matrix</a> can be inverted

### Example

<div><fiddle-embed name="10a10c5bf2ac7ec88e84204441fc83b6"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_Concat">Concat</a>

---

<a name="SkMatrix_SetAffineIdentity"></a>
## SetAffineIdentity

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static void SetAffineIdentity(SkScalar affine[6])
</pre>

Fills <a href="#SkMatrix_SetAffineIdentity_affine">affine</a> with identity values in column major order.
Sets <a href="#SkMatrix_SetAffineIdentity_affine">affine</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
| 1 0 0 |
| 0 1 0 |</pre>

Affine 3x2 matrices in column major order are used by OpenGL and XPS.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_SetAffineIdentity_affine"> <code><strong>affine </strong></code> </a></td> <td>
storage for 3x2 <a href="#SkMatrix_SetAffineIdentity_affine">affine</a> matrix</td>
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

<a href="#SkMatrix_setAffine">setAffine</a> <a href="#SkMatrix_asAffine">asAffine</a>

---

<a name="SkMatrix_asAffine"></a>
## asAffine

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool SK_WARN_UNUSED_RESULT asAffine(SkScalar affine[6]) const
</pre>

Fills <a href="#SkMatrix_asAffine_affine">affine</a> in column major order. Sets <a href="#SkMatrix_asAffine_affine">affine</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
| scale-x  skew-x translate-x |
| skew-y  scale-y translate-y |</pre>

If <a href="#Matrix">Matrix</a> contains perspective, returns false and leaves <a href="#SkMatrix_asAffine_affine">affine</a> unchanged.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_asAffine_affine"> <code><strong>affine </strong></code> </a></td> <td>
storage for 3x2 <a href="#SkMatrix_asAffine_affine">affine</a> matrix; may be nullptr</td>
  </tr>
</table>

### Return Value

true if <a href="#Matrix">Matrix</a> does not contain perspective

### Example

<div><fiddle-embed name="752e4a48ed1dae05765a2499c390f277">

#### Example Output

~~~~
ScaleX: 2 SkewY: 5 SkewX: 3 ScaleY: 6 TransX: 4 TransY: 7
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkMatrix_setAffine">setAffine</a> <a href="#SkMatrix_SetAffineIdentity">SetAffineIdentity</a>

---

<a name="SkMatrix_setAffine"></a>
## setAffine

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setAffine(const SkScalar affine[6])
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#SkMatrix_setAffine_affine">affine</a> values, passed in column major order. Given <a href="#SkMatrix_setAffine_affine">affine</a>,
column, then row, as:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
| scale-x  skew-x translate-x |
|  skew-y scale-y translate-y |</pre>

<a href="#Matrix">Matrix</a> is set, row, then column, to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
| scale-x  skew-x translate-x |
|  skew-y scale-y translate-y |
|       0       0           1 |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setAffine_affine"> <code><strong>affine </strong></code> </a></td> <td>
3x2 <a href="#SkMatrix_setAffine_affine">affine</a> matrix</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="403370bd53526f59020a7141955d70b0">

#### Example Output

~~~~
ScaleX: 2 SkewY: 5 SkewX: 3 ScaleY: 6 TransX: 4 TransY: 7
[  2.0000   3.0000   4.0000][  5.0000   6.0000   7.0000][  0.0000   0.0000   1.0000]
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkMatrix_asAffine">asAffine</a> <a href="#SkMatrix_SetAffineIdentity">SetAffineIdentity</a>

---

## <a name="Transform"></a> Transform

| name | description |
| --- | --- |
| <a href="#SkMatrix_mapHomogeneousPoints">mapHomogeneousPoints</a> | maps <a href="undocumented#Point3">Point3</a> array |
| <a href="#SkMatrix_mapPoints">mapPoints</a> | maps <a href="SkPoint_Reference#Point">Point</a> array |
|  | <a href="#SkMatrix_mapPoints">mapPoints(SkPoint dst[], const SkPoint src[], int count)</a> const |
|  | <a href="#SkMatrix_mapPoints_2">mapPoints(SkPoint pts[], int count)</a> const |
| <a href="#SkMatrix_mapRadius">mapRadius</a> | returns mean radius of mapped <a href="undocumented#Circle">Circle</a> |
| <a href="#SkMatrix_mapRect">mapRect</a> | returns bounds of mapped <a href="SkRect_Reference#Rect">Rect</a> |
|  | <a href="#SkMatrix_mapRect">mapRect(SkRect* dst, const SkRect& src)</a> const |
|  | <a href="#SkMatrix_mapRect_2">mapRect(SkRect* rect)</a> const |
| <a href="#SkMatrix_mapRectScaleTranslate">mapRectScaleTranslate</a> | returns bounds of mapped <a href="SkRect_Reference#Rect">Rect</a> |
| <a href="#SkMatrix_mapRectToQuad">mapRectToQuad</a> | maps <a href="SkRect_Reference#Rect">Rect</a> to <a href="SkPoint_Reference#Point">Point</a> array |
| <a href="#SkMatrix_mapVector">mapVector</a> | maps <a href="SkPoint_Reference#Vector">Vector</a> |
|  | <a href="#SkMatrix_mapVector">mapVector(SkScalar dx, SkScalar dy, SkVector* result)</a> const |
|  | <a href="#SkMatrix_mapVector_2">mapVector(SkScalar dx, SkScalar dy)</a> const |
| <a href="#SkMatrix_mapVectors">mapVectors</a> | maps <a href="SkPoint_Reference#Vector">Vector</a> array |
|  | <a href="#SkMatrix_mapVectors">mapVectors(SkVector dst[], const SkVector src[], int count)</a> const |
|  | <a href="#SkMatrix_mapVectors_2">mapVectors(SkVector vecs[], int count)</a> const |
| <a href="#SkMatrix_mapXY">mapXY</a> | maps <a href="SkPoint_Reference#Point">Point</a> |
|  | <a href="#SkMatrix_mapXY">mapXY(SkScalar x, SkScalar y, SkPoint* result)</a> const |
|  | <a href="#SkMatrix_mapXY_2">mapXY(SkScalar x, SkScalar y)</a> const |

<a name="SkMatrix_mapPoints"></a>
## mapPoints

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void mapPoints(SkPoint dst[], const SkPoint src[], int count) const
</pre>

Maps <a href="#SkMatrix_mapPoints_src">src</a> <a href="SkPoint_Reference#Point">Point</a> array of length <a href="#SkMatrix_mapPoints_count">count</a> to <a href="#SkMatrix_mapPoints_dst">dst</a> <a href="SkPoint_Reference#Point">Point</a> array of equal or greater
length. <a href="SkPoint_Reference#Point">Points</a> are mapped by multiplying each <a href="SkPoint_Reference#Point">Point</a> by <a href="#Matrix">Matrix</a>. Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | A B C |        | x |
Matrix = | D E F |,  pt = | y |
         | G H I |        | 1 |</pre>

where

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
for (i = 0; i < count; ++i) {
    x = src[i].fX
    y = src[i].fY
}</pre>

each <a href="#SkMatrix_mapPoints_dst">dst</a> <a href="SkPoint_Reference#Point">Point</a> is computed as:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
              |A B C| |x|                               Ax+By+C   Dx+Ey+F
Matrix * pt = |D E F| |y| = |Ax+By+C Dx+Ey+F Gx+Hy+I| = ------- , -------
              |G H I| |1|                               Gx+Hy+I   Gx+Hy+I</pre>

<a href="#SkMatrix_mapPoints_src">src</a> and <a href="#SkMatrix_mapPoints_dst">dst</a> may point to the same storage.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapPoints_dst"> <code><strong>dst </strong></code> </a></td> <td>
storage for mapped <a href="SkPoint_Reference#Point">Points</a></td>
  </tr>  <tr>    <td><a name="SkMatrix_mapPoints_src"> <code><strong>src </strong></code> </a></td> <td>
<a href="SkPoint_Reference#Point">Points</a> to transform</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapPoints_count"> <code><strong>count </strong></code> </a></td> <td>
number of <a href="SkPoint_Reference#Point">Points</a> to transform</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f99dcb00296d0c56b6c0e178e94b3534"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_mapXY">mapXY</a><sup><a href="#SkMatrix_mapXY_2">[2]</a></sup> <a href="#SkMatrix_mapHomogeneousPoints">mapHomogeneousPoints</a> <a href="#SkMatrix_mapVectors">mapVectors</a><sup><a href="#SkMatrix_mapVectors_2">[2]</a></sup>

---

<a name="SkMatrix_mapPoints_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void mapPoints(SkPoint pts[], int count) const
</pre>

Maps <a href="#SkMatrix_mapPoints_2_pts">pts</a> <a href="SkPoint_Reference#Point">Point</a> array of length <a href="#SkMatrix_mapPoints_2_count">count</a> in place. <a href="SkPoint_Reference#Point">Points</a> are mapped by multiplying
each <a href="SkPoint_Reference#Point">Point</a> by <a href="#Matrix">Matrix</a>. Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | A B C |        | x |
Matrix = | D E F |,  pt = | y |
         | G H I |        | 1 |</pre>

where

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
for (i = 0; i < count; ++i) {
    x = pts[i].fX
    y = pts[i].fY
}</pre>

each resulting <a href="#SkMatrix_mapPoints_2_pts">pts</a> <a href="SkPoint_Reference#Point">Point</a> is computed as:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
              |A B C| |x|                               Ax+By+C   Dx+Ey+F
Matrix * pt = |D E F| |y| = |Ax+By+C Dx+Ey+F Gx+Hy+I| = ------- , -------
              |G H I| |1|                               Gx+Hy+I   Gx+Hy+I</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapPoints_2_pts"> <code><strong>pts </strong></code> </a></td> <td>
storage for mapped <a href="SkPoint_Reference#Point">Points</a></td>
  </tr>  <tr>    <td><a name="SkMatrix_mapPoints_2_count"> <code><strong>count </strong></code> </a></td> <td>
number of <a href="SkPoint_Reference#Point">Points</a> to transform</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="428ca171ae3bd0d3f992458ac598b97b"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_mapXY">mapXY</a><sup><a href="#SkMatrix_mapXY_2">[2]</a></sup> <a href="#SkMatrix_mapHomogeneousPoints">mapHomogeneousPoints</a> <a href="#SkMatrix_mapVectors">mapVectors</a><sup><a href="#SkMatrix_mapVectors_2">[2]</a></sup>

---

<a name="SkMatrix_mapHomogeneousPoints"></a>
## mapHomogeneousPoints

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void mapHomogeneousPoints(SkPoint3 dst[], const SkPoint3 src[], int count) const
</pre>

Maps <a href="#SkMatrix_mapHomogeneousPoints_src">src</a> <a href="undocumented#Point3">Point3</a> array of length <a href="#SkMatrix_mapHomogeneousPoints_count">count</a> to <a href="#SkMatrix_mapHomogeneousPoints_dst">dst</a> <a href="undocumented#Point3">Point3</a> array, which must of length <a href="#SkMatrix_mapHomogeneousPoints_count">count</a> or
greater. <a href="undocumented#Point3">Point3</a> array is mapped by multiplying each <a href="undocumented#Point3">Point3</a> by <a href="#Matrix">Matrix</a>. Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | A B C |         | x |
Matrix = | D E F |,  src = | y |
         | G H I |         | z |</pre>

each resulting <a href="#SkMatrix_mapHomogeneousPoints_dst">dst</a> <a href="SkPoint_Reference#Point">Point</a> is computed as:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
               |A B C| |x|
Matrix * src = |D E F| |y| = |Ax+By+Cz Dx+Ey+Fz Gx+Hy+Iz|
               |G H I| |z|</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapHomogeneousPoints_dst"> <code><strong>dst </strong></code> </a></td> <td>
storage for mapped <a href="undocumented#Point3">Point3</a> array</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapHomogeneousPoints_src"> <code><strong>src </strong></code> </a></td> <td>
<a href="undocumented#Point3">Point3</a> array to transform</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapHomogeneousPoints_count"> <code><strong>count </strong></code> </a></td> <td>
items in <a href="undocumented#Point3">Point3</a> array to transform</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d56f93e4bc763c7ba4914321ed07a8b5"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_mapPoints">mapPoints</a><sup><a href="#SkMatrix_mapPoints_2">[2]</a></sup> <a href="#SkMatrix_mapXY">mapXY</a><sup><a href="#SkMatrix_mapXY_2">[2]</a></sup> <a href="#SkMatrix_mapVectors">mapVectors</a><sup><a href="#SkMatrix_mapVectors_2">[2]</a></sup>

---

<a name="SkMatrix_mapXY"></a>
## mapXY

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void mapXY(SkScalar x, SkScalar y, SkPoint* result) const
</pre>

Maps <a href="SkPoint_Reference#Point">Point</a> (<a href="#SkMatrix_mapXY_x">x</a>, <a href="#SkMatrix_mapXY_y">y</a>) to <a href="#SkMatrix_mapXY_result">result</a>. <a href="SkPoint_Reference#Point">Point</a> is mapped by multiplying by <a href="#Matrix">Matrix</a>. Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | A B C |        | x |
Matrix = | D E F |,  pt = | y |
         | G H I |        | 1 |</pre>

<a href="#SkMatrix_mapXY_result">result</a> is computed as:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
              |A B C| |x|                               Ax+By+C   Dx+Ey+F
Matrix * pt = |D E F| |y| = |Ax+By+C Dx+Ey+F Gx+Hy+I| = ------- , -------
              |G H I| |1|                               Gx+Hy+I   Gx+Hy+I</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapXY_x"> <code><strong>x </strong></code> </a></td> <td>
<a href="#SkMatrix_mapXY_x">x</a>-coordinate of <a href="SkPoint_Reference#Point">Point</a> to map</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapXY_y"> <code><strong>y </strong></code> </a></td> <td>
<a href="#SkMatrix_mapXY_y">y</a>-coordinate of <a href="SkPoint_Reference#Point">Point</a> to map</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapXY_result"> <code><strong>result </strong></code> </a></td> <td>
storage for mapped <a href="SkPoint_Reference#Point">Point</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="9e50185d502dc6903783679a84106089"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_mapPoints">mapPoints</a><sup><a href="#SkMatrix_mapPoints_2">[2]</a></sup> <a href="#SkMatrix_mapVectors">mapVectors</a><sup><a href="#SkMatrix_mapVectors_2">[2]</a></sup>

---

<a name="SkMatrix_mapXY_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkPoint mapXY(SkScalar x, SkScalar y) const
</pre>

Returns <a href="SkPoint_Reference#Point">Point</a> (<a href="#SkMatrix_mapXY_2_x">x</a>, <a href="#SkMatrix_mapXY_2_y">y</a>) multiplied by <a href="#Matrix">Matrix</a>. Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | A B C |        | x |
Matrix = | D E F |,  pt = | y |
         | G H I |        | 1 |</pre>

result is computed as:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
              |A B C| |x|                               Ax+By+C   Dx+Ey+F
Matrix * pt = |D E F| |y| = |Ax+By+C Dx+Ey+F Gx+Hy+I| = ------- , -------
              |G H I| |1|                               Gx+Hy+I   Gx+Hy+I</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapXY_2_x"> <code><strong>x </strong></code> </a></td> <td>
<a href="#SkMatrix_mapXY_2_x">x</a>-coordinate of <a href="SkPoint_Reference#Point">Point</a> to map</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapXY_2_y"> <code><strong>y </strong></code> </a></td> <td>
<a href="#SkMatrix_mapXY_2_y">y</a>-coordinate of <a href="SkPoint_Reference#Point">Point</a> to map</td>
  </tr>
</table>

### Return Value

mapped <a href="SkPoint_Reference#Point">Point</a>

### Example

<div><fiddle-embed name="b1ead09c67a177ab8eace12b061610a7"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_mapPoints">mapPoints</a><sup><a href="#SkMatrix_mapPoints_2">[2]</a></sup> <a href="#SkMatrix_mapVectors">mapVectors</a><sup><a href="#SkMatrix_mapVectors_2">[2]</a></sup>

---

<a name="SkMatrix_mapVectors"></a>
## mapVectors

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void mapVectors(SkVector dst[], const SkVector src[], int count) const
</pre>

Maps <a href="#SkMatrix_mapVectors_src">src</a> <a href="SkPoint_Reference#Vector">Vector</a> array of length <a href="#SkMatrix_mapVectors_count">count</a> to <a href="SkPoint_Reference#Vector">Vector</a> <a href="SkPoint_Reference#Point">Point</a> array of equal or greater
length. <a href="SkPoint_Reference#Vector">Vectors</a> are mapped by multiplying each <a href="SkPoint_Reference#Vector">Vector</a> by <a href="#Matrix">Matrix</a>, treating
<a href="#Matrix">Matrix</a> translation as zero. Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | A B 0 |         | x |
Matrix = | D E 0 |,  src = | y |
         | G H I |         | 1 |</pre>

where

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
for (i = 0; i < count; ++i) {
    x = src[i].fX
    y = src[i].fY
}</pre>

each <a href="#SkMatrix_mapVectors_dst">dst</a> <a href="SkPoint_Reference#Vector">Vector</a> is computed as:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
               |A B 0| |x|                            Ax+By     Dx+Ey
Matrix * src = |D E 0| |y| = |Ax+By Dx+Ey Gx+Hy+I| = ------- , -------
               |G H I| |1|                           Gx+Hy+I   Gx+Hy+I</pre>

<a href="#SkMatrix_mapVectors_src">src</a> and <a href="#SkMatrix_mapVectors_dst">dst</a> may point to the same storage.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapVectors_dst"> <code><strong>dst </strong></code> </a></td> <td>
storage for mapped <a href="SkPoint_Reference#Vector">Vectors</a></td>
  </tr>  <tr>    <td><a name="SkMatrix_mapVectors_src"> <code><strong>src </strong></code> </a></td> <td>
<a href="SkPoint_Reference#Vector">Vectors</a> to transform</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapVectors_count"> <code><strong>count </strong></code> </a></td> <td>
number of <a href="SkPoint_Reference#Vector">Vectors</a> to transform</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="918a9778c3d7d5cb306692784399f6dc"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_mapVector">mapVector</a><sup><a href="#SkMatrix_mapVector_2">[2]</a></sup> <a href="#SkMatrix_mapPoints">mapPoints</a><sup><a href="#SkMatrix_mapPoints_2">[2]</a></sup> <a href="#SkMatrix_mapXY">mapXY</a><sup><a href="#SkMatrix_mapXY_2">[2]</a></sup>

---

<a name="SkMatrix_mapVectors_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void mapVectors(SkVector vecs[], int count) const
</pre>

Maps <a href="#SkMatrix_mapVectors_2_vecs">vecs</a> <a href="SkPoint_Reference#Vector">Vector</a> array of length <a href="#SkMatrix_mapVectors_2_count">count</a> in place, multiplying each <a href="SkPoint_Reference#Vector">Vector</a> by
<a href="#Matrix">Matrix</a>, treating <a href="#Matrix">Matrix</a> translation as zero. Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | A B 0 |         | x |
Matrix = | D E 0 |,  vec = | y |
         | G H I |         | 1 |</pre>

where

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
for (i = 0; i < count; ++i) {
    x = vecs[i].fX
    y = vecs[i].fY
}</pre>

each result <a href="SkPoint_Reference#Vector">Vector</a> is computed as:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
               |A B 0| |x|                            Ax+By     Dx+Ey
Matrix * vec = |D E 0| |y| = |Ax+By Dx+Ey Gx+Hy+I| = ------- , -------
               |G H I| |1|                           Gx+Hy+I   Gx+Hy+I</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapVectors_2_vecs"> <code><strong>vecs </strong></code> </a></td> <td>
<a href="SkPoint_Reference#Vector">Vectors</a> to transform, and storage for mapped <a href="SkPoint_Reference#Vector">Vectors</a></td>
  </tr>  <tr>    <td><a name="SkMatrix_mapVectors_2_count"> <code><strong>count </strong></code> </a></td> <td>
number of <a href="SkPoint_Reference#Vector">Vectors</a> to transform</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="5754501a00a1323e76353fb53153e939"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_mapVector">mapVector</a><sup><a href="#SkMatrix_mapVector_2">[2]</a></sup> <a href="#SkMatrix_mapPoints">mapPoints</a><sup><a href="#SkMatrix_mapPoints_2">[2]</a></sup> <a href="#SkMatrix_mapXY">mapXY</a><sup><a href="#SkMatrix_mapXY_2">[2]</a></sup>

---

<a name="SkMatrix_mapVector"></a>
## mapVector

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void mapVector(SkScalar dx, SkScalar dy, SkVector* result) const
</pre>

Maps <a href="SkPoint_Reference#Vector">Vector</a> (x, y) to <a href="#SkMatrix_mapVector_result">result</a>. <a href="SkPoint_Reference#Vector">Vector</a> is mapped by multiplying by <a href="#Matrix">Matrix</a>,
treating <a href="#Matrix">Matrix</a> translation as zero. Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | A B 0 |         | dx |
Matrix = | D E 0 |,  vec = | dy |
         | G H I |         |  1 |</pre>

each <a href="#SkMatrix_mapVector_result">result</a> <a href="SkPoint_Reference#Vector">Vector</a> is computed as:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
               |A B 0| |dx|                                        A*dx+B*dy     D*dx+E*dy
Matrix * vec = |D E 0| |dy| = |A*dx+B*dy D*dx+E*dy G*dx+H*dy+I| = ----------- , -----------
               |G H I| | 1|                                       G*dx+H*dy+I   G*dx+*dHy+I</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapVector_dx"> <code><strong>dx </strong></code> </a></td> <td>
x-coordinate of <a href="SkPoint_Reference#Vector">Vector</a> to map</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapVector_dy"> <code><strong>dy </strong></code> </a></td> <td>
y-coordinate of <a href="SkPoint_Reference#Vector">Vector</a> to map</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapVector_result"> <code><strong>result </strong></code> </a></td> <td>
storage for mapped <a href="SkPoint_Reference#Vector">Vector</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="aed143fc6cd0bce4ed029b98d1e61f2d"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_mapVectors">mapVectors</a><sup><a href="#SkMatrix_mapVectors_2">[2]</a></sup> <a href="#SkMatrix_mapPoints">mapPoints</a><sup><a href="#SkMatrix_mapPoints_2">[2]</a></sup> <a href="#SkMatrix_mapXY">mapXY</a><sup><a href="#SkMatrix_mapXY_2">[2]</a></sup>

---

<a name="SkMatrix_mapVector_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkVector mapVector(SkScalar dx, SkScalar dy) const
</pre>

Returns <a href="SkPoint_Reference#Vector">Vector</a> (x, y) multiplied by <a href="#Matrix">Matrix</a>, treating <a href="#Matrix">Matrix</a> translation as zero.
Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | A B 0 |         | dx |
Matrix = | D E 0 |,  vec = | dy |
         | G H I |         |  1 |</pre>

each result <a href="SkPoint_Reference#Vector">Vector</a> is computed as:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
               |A B 0| |dx|                                        A*dx+B*dy     D*dx+E*dy
Matrix * vec = |D E 0| |dy| = |A*dx+B*dy D*dx+E*dy G*dx+H*dy+I| = ----------- , -----------
               |G H I| | 1|                                       G*dx+H*dy+I   G*dx+*dHy+I</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapVector_2_dx"> <code><strong>dx </strong></code> </a></td> <td>
x-coordinate of <a href="SkPoint_Reference#Vector">Vector</a> to map</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapVector_2_dy"> <code><strong>dy </strong></code> </a></td> <td>
y-coordinate of <a href="SkPoint_Reference#Vector">Vector</a> to map</td>
  </tr>
</table>

### Return Value

mapped <a href="SkPoint_Reference#Vector">Vector</a>

### Example

<div><fiddle-embed name="8bf1518db3f369696cd3065b541a8bd7"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_mapVectors">mapVectors</a><sup><a href="#SkMatrix_mapVectors_2">[2]</a></sup> <a href="#SkMatrix_mapPoints">mapPoints</a><sup><a href="#SkMatrix_mapPoints_2">[2]</a></sup> <a href="#SkMatrix_mapXY">mapXY</a><sup><a href="#SkMatrix_mapXY_2">[2]</a></sup>

---

<a name="SkMatrix_mapRect"></a>
## mapRect

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool mapRect(SkRect* dst, const SkRect& src) const
</pre>

Sets <a href="#SkMatrix_mapRect_dst">dst</a> to bounds of <a href="#SkMatrix_mapRect_src">src</a> corners mapped by <a href="#Matrix">Matrix</a>.
Returns true if mapped corners are <a href="#SkMatrix_mapRect_dst">dst</a> corners.

Returned value is the same as calling <a href="#SkMatrix_rectStaysRect">rectStaysRect</a>.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapRect_dst"> <code><strong>dst </strong></code> </a></td> <td>
storage for bounds of mapped <a href="SkPoint_Reference#Point">Points</a></td>
  </tr>  <tr>    <td><a name="SkMatrix_mapRect_src"> <code><strong>src </strong></code> </a></td> <td>
<a href="SkRect_Reference#Rect">Rect</a> to map</td>
  </tr>
</table>

### Return Value

true if <a href="#SkMatrix_mapRect_dst">dst</a> is equivalent to mapped <a href="#SkMatrix_mapRect_src">src</a>

### Example

<div><fiddle-embed name="dbcf928b035a31ca69c99392e2e2cca9"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_mapPoints">mapPoints</a><sup><a href="#SkMatrix_mapPoints_2">[2]</a></sup> <a href="#SkMatrix_rectStaysRect">rectStaysRect</a>

---

<a name="SkMatrix_mapRect_2"></a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool mapRect(SkRect* rect) const
</pre>

Sets <a href="#SkMatrix_mapRect_2_rect">rect</a> to bounds of <a href="#SkMatrix_mapRect_2_rect">rect</a> corners mapped by <a href="#Matrix">Matrix</a>.
Returns true if mapped corners are computed <a href="#SkMatrix_mapRect_2_rect">rect</a> corners.

Returned value is the same as calling <a href="#SkMatrix_rectStaysRect">rectStaysRect</a>.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapRect_2_rect"> <code><strong>rect </strong></code> </a></td> <td>
rectangle to map, and storage for bounds of mapped corners</td>
  </tr>
</table>

### Return Value

true if result is equivalent to mapped src

### Example

<div><fiddle-embed name="5fafd0bd23d1ed37425b970b4a3c6cc9"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_mapRectScaleTranslate">mapRectScaleTranslate</a> <a href="#SkMatrix_mapPoints">mapPoints</a><sup><a href="#SkMatrix_mapPoints_2">[2]</a></sup> <a href="#SkMatrix_rectStaysRect">rectStaysRect</a>

---

<a name="SkMatrix_mapRectToQuad"></a>
## mapRectToQuad

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void mapRectToQuad(SkPoint dst[4], const SkRect& rect) const
</pre>

Maps four corners of <a href="#SkMatrix_mapRectToQuad_rect">rect</a> to <a href="#SkMatrix_mapRectToQuad_dst">dst</a>. <a href="SkPoint_Reference#Point">Points</a> are mapped by multiplying each
<a href="#SkMatrix_mapRectToQuad_rect">rect</a> corner by <a href="#Matrix">Matrix</a>. <a href="#SkMatrix_mapRectToQuad_rect">rect</a> corner is processed in this order:
(<a href="#SkMatrix_mapRectToQuad_rect">rect</a>.fLeft, <a href="#SkMatrix_mapRectToQuad_rect">rect</a>.fTop), (<a href="#SkMatrix_mapRectToQuad_rect">rect</a>.fRight, <a href="#SkMatrix_mapRectToQuad_rect">rect</a>.fTop), (<a href="#SkMatrix_mapRectToQuad_rect">rect</a>.fRight, <a href="#SkMatrix_mapRectToQuad_rect">rect</a>.fBottom),
(<a href="#SkMatrix_mapRectToQuad_rect">rect</a>.fLeft, <a href="#SkMatrix_mapRectToQuad_rect">rect</a>.fBottom).

<a href="#SkMatrix_mapRectToQuad_rect">rect</a> may be empty: <a href="#SkMatrix_mapRectToQuad_rect">rect</a>.fLeft may be greater than or equal to <a href="#SkMatrix_mapRectToQuad_rect">rect</a>.fRight;
<a href="#SkMatrix_mapRectToQuad_rect">rect</a>.fTop may be greater than or equal to <a href="#SkMatrix_mapRectToQuad_rect">rect</a>.fBottom.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
         | A B C |        | x |
Matrix = | D E F |,  pt = | y |
         | G H I |        | 1 |</pre>

where pt is initialized from each of (<a href="#SkMatrix_mapRectToQuad_rect">rect</a>.fLeft, <a href="#SkMatrix_mapRectToQuad_rect">rect</a>.fTop),
(<a href="#SkMatrix_mapRectToQuad_rect">rect</a>.fRight, <a href="#SkMatrix_mapRectToQuad_rect">rect</a>.fTop), (<a href="#SkMatrix_mapRectToQuad_rect">rect</a>.fRight, <a href="#SkMatrix_mapRectToQuad_rect">rect</a>.fBottom), (<a href="#SkMatrix_mapRectToQuad_rect">rect</a>.fLeft, <a href="#SkMatrix_mapRectToQuad_rect">rect</a>.fBottom),
each <a href="#SkMatrix_mapRectToQuad_dst">dst</a> <a href="SkPoint_Reference#Point">Point</a> is computed as:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
              |A B C| |x|                               Ax+By+C   Dx+Ey+F
Matrix * pt = |D E F| |y| = |Ax+By+C Dx+Ey+F Gx+Hy+I| = ------- , -------
              |G H I| |1|                               Gx+Hy+I   Gx+Hy+I</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapRectToQuad_dst"> <code><strong>dst </strong></code> </a></td> <td>
storage for mapped corner <a href="SkPoint_Reference#Point">Points</a></td>
  </tr>  <tr>    <td><a name="SkMatrix_mapRectToQuad_rect"> <code><strong>rect </strong></code> </a></td> <td>
<a href="SkRect_Reference#Rect">Rect</a> to map</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c69cd2a590b5733c3cbc92cb9ceed3f5"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_mapRect">mapRect</a><sup><a href="#SkMatrix_mapRect_2">[2]</a></sup> <a href="#SkMatrix_mapRectScaleTranslate">mapRectScaleTranslate</a>

---

<a name="SkMatrix_mapRectScaleTranslate"></a>
## mapRectScaleTranslate

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void mapRectScaleTranslate(SkRect* dst, const SkRect& src) const
</pre>

Sets <a href="#SkMatrix_mapRectScaleTranslate_dst">dst</a> to bounds of <a href="#SkMatrix_mapRectScaleTranslate_src">src</a> corners mapped by <a href="#Matrix">Matrix</a>. If matrix contains
elements other than scale or translate: asserts if SK_DEBUG is defined;
otherwise, results are undefined.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapRectScaleTranslate_dst"> <code><strong>dst </strong></code> </a></td> <td>
storage for bounds of mapped <a href="SkPoint_Reference#Point">Points</a></td>
  </tr>  <tr>    <td><a name="SkMatrix_mapRectScaleTranslate_src"> <code><strong>src </strong></code> </a></td> <td>
<a href="SkRect_Reference#Rect">Rect</a> to map</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="62bc26989c2b4c2a54d516596a71dd97"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_mapRect">mapRect</a><sup><a href="#SkMatrix_mapRect_2">[2]</a></sup> <a href="#SkMatrix_mapRectToQuad">mapRectToQuad</a> <a href="#SkMatrix_isScaleTranslate">isScaleTranslate</a> <a href="#SkMatrix_rectStaysRect">rectStaysRect</a>

---

<a name="SkMatrix_mapRadius"></a>
## mapRadius

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkScalar mapRadius(SkScalar radius) const
</pre>

Returns geometric mean <a href="#SkMatrix_mapRadius_radius">radius</a> of ellipse formed by constructing <a href="undocumented#Circle">Circle</a> of
size <a href="#SkMatrix_mapRadius_radius">radius</a>, and mapping constructed <a href="undocumented#Circle">Circle</a> with <a href="#Matrix">Matrix</a>. The result squared is
equal to the major axis length times the minor axis length.
Result is not meaningful if <a href="#Matrix">Matrix</a> contains perspective elements.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapRadius_radius"> <code><strong>radius </strong></code> </a></td> <td>
<a href="undocumented#Circle">Circle</a> size to map</td>
  </tr>
</table>

### Return Value

average mapped <a href="#SkMatrix_mapRadius_radius">radius</a>

### Example

<div><fiddle-embed name="6d6f2082fcf59d9f02bfb1758b87db69"><div>The area enclosed by a square with sides equal to mappedRadius is the same as
the area enclosed by the ellipse major and minor axes.</div></fiddle-embed></div>

### See Also

<a href="#SkMatrix_mapVector">mapVector</a><sup><a href="#SkMatrix_mapVector_2">[2]</a></sup>

---

<a name="SkMatrix_isFixedStepInX"></a>
## isFixedStepInX

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isFixedStepInX() const
</pre>

Returns true if a unit step in x at some y mapped through <a href="#Matrix">Matrix</a> can be
represented by a constant <a href="SkPoint_Reference#Vector">Vector</a>. Returns true if <a href="#SkMatrix_getType">getType</a> returns <a href="#SkMatrix_kIdentity_Mask">kIdentity Mask</a>,
or combinations of: <a href="#SkMatrix_kTranslate_Mask">kTranslate Mask</a>, <a href="#SkMatrix_kScale_Mask">kScale Mask</a>, and <a href="#SkMatrix_kAffine_Mask">kAffine Mask</a>.

May return true if <a href="#SkMatrix_getType">getType</a> returns <a href="#SkMatrix_kPerspective_Mask">kPerspective Mask</a>, but only when <a href="#Matrix">Matrix</a>
does not include rotation or skewing along the y-axis.

### Return Value

true if <a href="#Matrix">Matrix</a> does not have complex perspective

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

<a href="#SkMatrix_fixedStepInX">fixedStepInX</a> <a href="#SkMatrix_getType">getType</a>

---

<a name="SkMatrix_fixedStepInX"></a>
## fixedStepInX

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkVector fixedStepInX(SkScalar y) const
</pre>

Returns <a href="SkPoint_Reference#Vector">Vector</a> representing a unit step in x at <a href="#SkMatrix_fixedStepInX_y">y</a> mapped through <a href="#Matrix">Matrix</a>.
If <a href="#SkMatrix_isFixedStepInX">isFixedStepInX</a> is false, returned value is undefined.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_fixedStepInX_y"> <code><strong>y </strong></code> </a></td> <td>
position of line parallel to x-axis</td>
  </tr>
</table>

### Return Value

<a href="SkPoint_Reference#Vector">Vector</a> advance of mapped unit step in x

### Example

<div><fiddle-embed name="fad6b92b21b1e1deeae61978cec2d232"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_isFixedStepInX">isFixedStepInX</a> <a href="#SkMatrix_getType">getType</a>

---

<a name="SkMatrix_cheapEqualTo"></a>
## cheapEqualTo

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool cheapEqualTo(const SkMatrix& m) const
</pre>

Returns true if <a href="#Matrix">Matrix</a> equals <a href="#SkMatrix_cheapEqualTo_m">m</a>, using an efficient comparison.

Returns false when the sign of zero values is the different; when one
matrix has positive zero value and the other has negative zero value.

Returns true even when both <a href="#Matrix">Matrices</a> contain NaN.

NaN never equals any value, including itself. To improve performance, NaN values
are treated as bit patterns that are equal if their bit patterns are equal.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_cheapEqualTo_m"> <code><strong>m </strong></code> </a></td> <td>
<a href="#Matrix">Matrix</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href="#SkMatrix_cheapEqualTo_m">m</a> and <a href="#Matrix">Matrix</a> are represented by identical bit patterns

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

<a href="#SkMatrix_equal_operator">operator==(const SkMatrix& a, const SkMatrix& b)</a>

---

<a name="SkMatrix_equal_operator"></a>
## operator==

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool operator==(const SkMatrix& a, const SkMatrix& b)
</pre>

Compares <a href="#SkMatrix_equal_operator_a">a</a> and <a href="#SkMatrix_equal_operator_b">b</a>; returns true if <a href="#SkMatrix_equal_operator_a">a</a> and <a href="#SkMatrix_equal_operator_b">b</a> are numerically equal. Returns true
even if sign of zero values are different. Returns false if either <a href="#Matrix">Matrix</a>
contains NaN, even if the other <a href="#Matrix">Matrix</a> also contains NaN.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_equal_operator_a"> <code><strong>a </strong></code> </a></td> <td>
<a href="#Matrix">Matrix</a> to compare</td>
  </tr>  <tr>    <td><a name="SkMatrix_equal_operator_b"> <code><strong>b </strong></code> </a></td> <td>
<a href="#Matrix">Matrix</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href="#Matrix">Matrix</a> <a href="#SkMatrix_equal_operator_a">a</a> and <a href="#Matrix">Matrix</a> <a href="#SkMatrix_equal_operator_b">b</a> are numerically equal

### Example

<div><fiddle-embed name="3902859150b0f0c4aeb1f25d00434baa">

#### Example Output

~~~~
identity: a == b a.cheapEqualTo(b): true
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkMatrix_cheapEqualTo">cheapEqualTo</a> <a href="#SkMatrix_notequal_operator">operator!=(const SkMatrix& a, const SkMatrix& b)</a>

---

<a name="SkMatrix_notequal_operator"></a>
## operator!=

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool operator!=(const SkMatrix& a, const SkMatrix& b)
</pre>

Compares <a href="#SkMatrix_notequal_operator_a">a</a> and <a href="#SkMatrix_notequal_operator_b">b</a>; returns true if <a href="#SkMatrix_notequal_operator_a">a</a> and <a href="#SkMatrix_notequal_operator_b">b</a> are not numerically equal. Returns false
even if sign of zero values are different. Returns true if either <a href="#Matrix">Matrix</a>
contains NaN, even if the other <a href="#Matrix">Matrix</a> also contains NaN.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_notequal_operator_a"> <code><strong>a </strong></code> </a></td> <td>
<a href="#Matrix">Matrix</a> to compare</td>
  </tr>  <tr>    <td><a name="SkMatrix_notequal_operator_b"> <code><strong>b </strong></code> </a></td> <td>
<a href="#Matrix">Matrix</a> to compare</td>
  </tr>
</table>

### Return Value

true if <a href="#Matrix">Matrix</a> <a href="#SkMatrix_notequal_operator_a">a</a> and <a href="#Matrix">Matrix</a> <a href="#SkMatrix_notequal_operator_b">b</a> are numerically not equal

### Example

<div><fiddle-embed name="8a8fadf5fd294daa4ee152833cc0dc0e"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_cheapEqualTo">cheapEqualTo</a> <a href="#SkMatrix_equal_operator">operator==(const SkMatrix& a, const SkMatrix& b)</a>

---

## <a name="Utility"></a> Utility

| name | description |
| --- | --- |
| <a href="#SkMatrix_dirtyMatrixTypeCache">dirtyMatrixTypeCache</a> | sets internal cache to unknown state |
| <a href="#SkMatrix_dump">dump</a> | sends text representation using floats to standard output |
| <a href="#SkMatrix_toString">toString</a> | converts <a href="#Matrix">Matrix</a> to machine readable form |

<a name="SkMatrix_dump"></a>
## dump

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void dump() const
</pre>

Writes text representation of <a href="#Matrix">Matrix</a> to standard output. Floating point values
are written with limited precision; it may not be possible to reconstruct
original <a href="#Matrix">Matrix</a> from output.

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

<a href="#SkMatrix_toString">toString</a>

---

<a name="SkMatrix_toString"></a>
## toString

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void toString(SkString* str) const
</pre>

Creates string representation of <a href="#Matrix">Matrix</a>. Floating point values
are written with limited precision; it may not be possible to reconstruct
original <a href="#Matrix">Matrix</a> from output.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_toString_str"> <code><strong>str </strong></code> </a></td> <td>
storage for string representation of <a href="#Matrix">Matrix</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1d86e43958e42b8eaaa9b16df1baa4c8">

#### Example Output

~~~~
mStr  [  0.7071  -0.7071   0.0000][  0.7071   0.7071   0.0000][  0.0000   0.0000   1.0000]
neStr [  0.7071  -0.7071   0.0000][  0.7071   0.7071   0.0000][  0.0000   0.0000   1.0000]
matrix != nearlyEqual
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkMatrix_dump">dump</a>

---

<a name="SkMatrix_getMinScale"></a>
## getMinScale

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkScalar getMinScale() const
</pre>

Returns the minimum scaling factor of <a href="#Matrix">Matrix</a> by decomposing the scaling and
skewing elements.
Returns -1 if scale factor overflows or <a href="#Matrix">Matrix</a> contains perspective.

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

<a href="#SkMatrix_getMaxScale">getMaxScale</a> <a href="#SkMatrix_getMinMaxScales">getMinMaxScales</a>

---

<a name="SkMatrix_getMaxScale"></a>
## getMaxScale

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkScalar getMaxScale() const
</pre>

Returns the maximum scaling factor of <a href="#Matrix">Matrix</a> by decomposing the scaling and
skewing elements.
Returns -1 if scale factor overflows or <a href="#Matrix">Matrix</a> contains perspective.

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

<a href="#SkMatrix_getMinScale">getMinScale</a> <a href="#SkMatrix_getMinMaxScales">getMinMaxScales</a>

---

<a name="SkMatrix_getMinMaxScales"></a>
## getMinMaxScales

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool SK_WARN_UNUSED_RESULT getMinMaxScales(SkScalar scaleFactors[2]) const
</pre>

Sets <a href="#SkMatrix_getMinMaxScales_scaleFactors">scaleFactors</a>[0] to the minimum scaling factor, and <a href="#SkMatrix_getMinMaxScales_scaleFactors">scaleFactors</a>[1] to the
maximum scaling factor. Scaling factors are computed by decomposing
the <a href="#Matrix">Matrix</a> scaling and skewing elements.

Returns true if <a href="#SkMatrix_getMinMaxScales_scaleFactors">scaleFactors</a> are found; otherwise, returns false and sets
<a href="#SkMatrix_getMinMaxScales_scaleFactors">scaleFactors</a> to undefined values.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_getMinMaxScales_scaleFactors"> <code><strong>scaleFactors </strong></code> </a></td> <td>
storage for minimum and maximum scale factors</td>
  </tr>
</table>

### Return Value

true if scale factors were computed correctly

### Example

<div><fiddle-embed name="cd4dc63d3e04226f0b5861ba8925e223">

#### Example Output

~~~~
matrix.getMinMaxScales() false 2 2
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkMatrix_getMinScale">getMinScale</a> <a href="#SkMatrix_getMaxScale">getMaxScale</a>

---

<a name="SkMatrix_decomposeScale"></a>
## decomposeScale

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool decomposeScale(SkSize* scale, SkMatrix* remaining = nullptr) const
</pre>

Decomposes <a href="#Matrix">Matrix</a> into <a href="#SkMatrix_decomposeScale_scale">scale</a> components and whatever remains. Returns false if
<a href="#Matrix">Matrix</a> could not be decomposed.

Sets <a href="#SkMatrix_decomposeScale_scale">scale</a> to portion of <a href="#Matrix">Matrix</a> that scales in x and y. Sets <a href="#SkMatrix_decomposeScale_remaining">remaining</a> to <a href="#Matrix">Matrix</a>
with x and y scaling factored out. <a href="#SkMatrix_decomposeScale_remaining">remaining</a> may be passed as nullptr
to determine if <a href="#Matrix">Matrix</a> can be decomposed without computing remainder.

Returns true if <a href="#SkMatrix_decomposeScale_scale">scale</a> components are found. <a href="#SkMatrix_decomposeScale_scale">scale</a> and <a href="#SkMatrix_decomposeScale_remaining">remaining</a> are
unchanged if <a href="#Matrix">Matrix</a> contains perspective; <a href="#SkMatrix_decomposeScale_scale">scale</a> factors are not finite, or
are nearly zero.

On success<a href="#Matrix">Matrix</a> = <a href="#SkMatrix_decomposeScale_scale">scale</a> * Remaining

### Parameters

<table>  <tr>    <td><a name="SkMatrix_decomposeScale_scale"> <code><strong>scale </strong></code> </a></td> <td>
x and y scaling factors; may be nullptr</td>
  </tr>  <tr>    <td><a name="SkMatrix_decomposeScale_remaining"> <code><strong>remaining </strong></code> </a></td> <td>
<a href="#Matrix">Matrix</a> without scaling; may be nullptr</td>
  </tr>
</table>

### Return Value

true if <a href="#SkMatrix_decomposeScale_scale">scale</a> can be computed

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

<a href="#SkMatrix_setScale">setScale</a><sup><a href="#SkMatrix_setScale_2">[2]</a></sup> <a href="#SkMatrix_MakeScale">MakeScale</a><sup><a href="#SkMatrix_MakeScale_2">[2]</a></sup>

---

<a name="SkMatrix_I"></a>
## I

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static const SkMatrix& I()
</pre>

Returns reference to const identity <a href="#Matrix">Matrix</a>. Returned <a href="#Matrix">Matrix</a> is set to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
| 1 0 0 |
| 0 1 0 |
| 0 0 1 |</pre>

### Return Value

const identity <a href="#Matrix">Matrix</a>

### Example

<div><fiddle-embed name="d961d91020f19037204a8c3fd8cb1060">

#### Example Output

~~~~
m1 == m2
m2 == m3
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkMatrix_reset">reset</a> <a href="#SkMatrix_setIdentity">setIdentity</a>

---

<a name="SkMatrix_InvalidMatrix"></a>
## InvalidMatrix

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static const SkMatrix& InvalidMatrix()
</pre>

Returns reference to a const <a href="#Matrix">Matrix</a> with invalid values. Returned <a href="#Matrix">Matrix</a> is set
to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
| SK_ScalarMax SK_ScalarMax SK_ScalarMax |
| SK_ScalarMax SK_ScalarMax SK_ScalarMax |
| SK_ScalarMax SK_ScalarMax SK_ScalarMax |</pre>

### Return Value

const invalid <a href="#Matrix">Matrix</a>

### Example

<div><fiddle-embed name="af0b72360c1c7a25b4754bfa47011dd5">

#### Example Output

~~~~
scaleX 3.40282e+38
~~~~

</fiddle-embed></div>

### See Also

SeeAlso <a href="#SkMatrix_getType">getType</a>

---

<a name="SkMatrix_Concat"></a>
## Concat

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
static SkMatrix Concat(const SkMatrix& a, const SkMatrix& b)
</pre>

Returns <a href="#Matrix">Matrix</a> <a href="#SkMatrix_Concat_a">a</a> multiplied by <a href="#Matrix">Matrix</a> <a href="#SkMatrix_Concat_b">b</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    | A B C |      | J K L |
a = | D E F |, b = | M N O |
    | G H I |      | P Q R |</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
        | A B C |   | J K L |   | AJ+BM+CP AK+BN+CQ AL+BO+CR |
a * b = | D E F | * | M N O | = | DJ+EM+FP DK+EN+FQ DL+EO+FR |
        | G H I |   | P Q R |   | GJ+HM+IP GK+HN+IQ GL+HO+IR |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_Concat_a"> <code><strong>a </strong></code> </a></td> <td>
<a href="#Matrix">Matrix</a> on left side of multiply expression</td>
  </tr>  <tr>    <td><a name="SkMatrix_Concat_b"> <code><strong>b </strong></code> </a></td> <td>
<a href="#Matrix">Matrix</a> on right side of multiply expression</td>
  </tr>
</table>

### Return Value

<a href="#Matrix">Matrix</a> computed from <a href="#SkMatrix_Concat_a">a</a> times <a href="#SkMatrix_Concat_b">b</a>

### Example

<div><fiddle-embed name="6b4562c7052da94f3d5b2412dca41946"><div><a href="#SkMatrix_setPolyToPoly">setPolyToPoly</a> creates perspective matrices, one the inverse of the other.
Multiplying the matrix by its inverse turns into an identity matrix.</div></fiddle-embed></div>

### See Also

<a href="#SkMatrix_preConcat">preConcat</a> <a href="#SkMatrix_postConcat">postConcat</a>

---

<a name="SkMatrix_dirtyMatrixTypeCache"></a>
## dirtyMatrixTypeCache

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void dirtyMatrixTypeCache()
</pre>

Sets internal cache to unknown state. Use to force update after repeated
modifications to <a href="#Matrix">Matrix</a> element reference returned by <a href="#SkMatrix_array1_operator">operator[](int index)</a>.

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

<a href="#SkMatrix_array1_operator">operator[](int index)</a> <a href="#SkMatrix_getType">getType</a>

---

<a name="SkMatrix_setScaleTranslate"></a>
## setScaleTranslate

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void setScaleTranslate(SkScalar sx, SkScalar sy, SkScalar tx, SkScalar ty)
</pre>

Initializes <a href="#Matrix">Matrix</a> with scale and translate elements.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
| sx  0 tx |
|  0 sy ty |
|  0  0  1 |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setScaleTranslate_sx"> <code><strong>sx </strong></code> </a></td> <td>
horizontal scale factor to store</td>
  </tr>  <tr>    <td><a name="SkMatrix_setScaleTranslate_sy"> <code><strong>sy </strong></code> </a></td> <td>
vertical scale factor to store</td>
  </tr>  <tr>    <td><a name="SkMatrix_setScaleTranslate_tx"> <code><strong>tx </strong></code> </a></td> <td>
horizontal translation to store</td>
  </tr>  <tr>    <td><a name="SkMatrix_setScaleTranslate_ty"> <code><strong>ty </strong></code> </a></td> <td>
vertical translation to store</td>
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

<a href="#SkMatrix_setScale">setScale</a><sup><a href="#SkMatrix_setScale_2">[2]</a></sup> <a href="#SkMatrix_preTranslate">preTranslate</a> <a href="#SkMatrix_postTranslate">postTranslate</a>

---

<a name="SkMatrix_isFinite"></a>
## isFinite

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
bool isFinite() const
</pre>

Returns true if all elements of the matrix are finite. Returns false if any
element is infinity, or NaN.

### Return Value

true if matrix has only finite elements

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

---

