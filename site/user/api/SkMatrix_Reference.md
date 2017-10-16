SkMatrix Reference
===

# <a name="Matrix"></a> Matrix

# <a name="SkMatrix"></a> Class SkMatrix
<a href="#Matrix">Matrix</a> holds a 3x3 matrix for transforming coordinates. This allows mapping
<a href="#Point">Points</a> and <a href="#Vector">Vectors</a> with translation, scaling, skewing, rotation, and
perspective.

<a href="#Matrix">Matrix</a> elements are in row major order. <a href="#Matrix">Matrix</a> does not have a constructor,
so it must be explicitly initialized. <a href="#SkMatrix_setIdentity">setIdentity</a> initializes <a href="#Matrix">Matrix</a>
so it has no effect. <a href="#SkMatrix_setTranslate">setTranslate</a>, <a href="#SkMatrix_setScale">setScale</a>, <a href="#SkMatrix_setSkew">setSkew</a>, <a href="#SkMatrix_setRotate">setRotate</a>, <a href="#SkMatrix_set9">set9</a> and <a href="#SkMatrix_setAll">setAll</a>
initializes all <a href="#Matrix">Matrix</a> elements with the corresponding mapping.

<a href="#Matrix">Matrix</a> includes a hidden variable that classifies the type of matrix to 
improve performance. <a href="#Matrix">Matrix</a> is not thread safe unless <a href="#SkMatrix_getType">getType</a> is called first.

# <a name="Overview"></a> Overview

## <a name="Subtopics"></a> Subtopics

| topics | description |
| --- | ---  |

## <a name="Operators"></a> Operators

| function | description |
| --- | ---  |
| <a href="#SkMatrix_not_equal_operator">operator!=(const SkMatrix& a, const SkMatrix& b)</a> | Returns true if members are unequal. |
| <a href="#SkMatrix_equal_operator">operator==(const SkMatrix& a, const SkMatrix& b)</a> | Returns true if members are equal. |
| <a href="#SkMatrix_subscript_operator_const">operator[](int index)</a> const | Returns <a href="#Matrix">Matrix</a> value. |
| <a href="#SkMatrix_subscript_operator_const">operator[](int index)</a> | Returns writable reference to <a href="#Matrix">Matrix</a> value. |

## <a name="Member_Functions"></a> Member Functions

| function | description |
| --- | ---  |
| <a href="#SkMatrix_Concat">Concat</a> | Returns the concatenation of <a href="#Matrix">Matrix</a> pair. |
| <a href="#SkMatrix_GetMapPtsProc">GetMapPtsProc</a> | Returns optimal function to map <a href="undocumented#Point">Point</a> array. |
| <a href="#SkMatrix_GetMapXYProc">GetMapXYProc</a> | Returns optimal function to map one <a href="undocumented#Point">Point</a>. |
| <a href="#SkMatrix_I">I</a> | Returns a reference to a const identity <a href="#Matrix">Matrix</a>. |
| <a href="#SkMatrix_InvalidMatrix">InvalidMatrix</a> | Returns a reference to a const invalid <a href="#Matrix">Matrix</a>. |
| <a href="#SkMatrix_MakeRectToRect">MakeRectToRect</a> | Constructs from source <a href="SkRect_Reference#Rect">Rect</a> to destination <a href="SkRect_Reference#Rect">Rect</a>. |
| <a href="#SkMatrix_MakeScale">MakeScale</a> | Constructs from scale in x and y. |
| <a href="#SkMatrix_MakeTrans">MakeTrans</a> | Constructs from translate in x and y. |
| <a href="#SkMatrix_SetAffineIdentity">SetAffineIdentity</a> | Sets 2x3 array to identity. |
| <a href="#SkMatrix_asAffine">asAffine</a> | Copies to 2x3 array. |
| <a href="#SkMatrix_cheapEqualTo">cheapEqualTo</a> | Compares <a href="#Matrix">Matrix</a> pair using memcmp(). |
| <a href="#SkMatrix_decomposeScale">decomposeScale</a> | Separates scale if possible. |
| <a href="#SkMatrix_dirtyMatrixTypeCache">dirtyMatrixTypeCache</a> | Sets internal cache to unknown state. |
| <a href="#SkMatrix_dump">dump</a> | Sends text representation using floats to standard output. |
| <a href="#SkMatrix_fixedStepInX">fixedStepInX</a> | Returns step in x for a position in y. |
| <a href="#SkMatrix_get">get</a> | Returns one of nine <a href="#Matrix">Matrix</a> values. |
| <a href="#SkMatrix_get9">get9</a> | Returns all nine <a href="#Matrix">Matrix</a> values. |
| <a href="#SkMatrix_getMapPtsProc">getMapPtsProc</a> | Returns optimal function to map <a href="undocumented#Point">Point</a> array. |
| <a href="#SkMatrix_getMapXYProc">getMapXYProc</a> | Returns optimal function to map one <a href="undocumented#Point">Point</a>. |
| <a href="#SkMatrix_getMaxScale">getMaxScale</a> | Returns maximum scaling, if possible. |
| <a href="#SkMatrix_getMinMaxScales">getMinMaxScales</a> | Returns minimum and maximum scaling, if possible. |
| <a href="#SkMatrix_getMinScale">getMinScale</a> | Returns minimum scaling, if possible. |
| <a href="#SkMatrix_getPerspX">getPerspX</a> | Returns input x perspective factor. |
| <a href="#SkMatrix_getPerspY">getPerspY</a> | Returns input y perspective factor. |
| <a href="#SkMatrix_getScaleX">getScaleX</a> | Returns horizontal scale factor. |
| <a href="#SkMatrix_getScaleY">getScaleY</a> | Returns vertical scale factor. |
| <a href="#SkMatrix_getSkewX">getSkewX</a> | Returns horizontal skew factor. |
| <a href="#SkMatrix_getSkewY">getSkewY</a> | Returns vertical skew factor. |
| <a href="#SkMatrix_getTranslateX">getTranslateX</a> | Returns horizontal translation. |
| <a href="#SkMatrix_getTranslateY">getTranslateY</a> | Returns vertical translation. |
| <a href="#SkMatrix_getType">getType</a> | Returns transform complexity. |
| <a href="#SkMatrix_hasPerspective">hasPerspective</a> | Returns if transform includes perspective. |
| <a href="#SkMatrix_invert">invert</a> | Returns inverse, if possible. |
| <a href="#SkMatrix_isFinite">isFinite</a> | Returns if all <a href="#Matrix">Matrix</a> values are not infinity, <a href="undocumented#NaN">NaN</a>. |
| <a href="#SkMatrix_isFixedStepInX">isFixedStepInX</a> | Returns if transformation supports fixed step in x. |
| <a href="#SkMatrix_isIdentity">isIdentity</a> | Returns if matrix equals the identity <a href="#Matrix">Matrix</a> . |
| <a href="#SkMatrix_isScaleTranslate">isScaleTranslate</a> | Returns if transform is limited to scale and translate. |
| <a href="#SkMatrix_isSimilarity">isSimilarity</a> | Returns if transform is limited to square scale and rotation. |
| <a href="#SkMatrix_isTranslate">isTranslate</a> | Returns if transform is limited to translate. |
| <a href="#SkMatrix_mapHomogeneousPoints">mapHomogeneousPoints</a> | Maps <a href="undocumented#Point3">Point3</a> array. |
| <a href="#SkMatrix_mapPoints">mapPoints</a> | Maps <a href="undocumented#Point">Point</a> array. |
| <a href="#SkMatrix_mapPointsWithStride">mapPointsWithStride</a> | Maps <a href="undocumented#Point">Point</a> array with padding. |
| <a href="#SkMatrix_mapRadius">mapRadius</a> | Returns mean radius of mapped <a href="undocumented#Circle">Circle</a>. |
| <a href="#SkMatrix_mapRect">mapRect</a> | Returns bounds of mapped <a href="SkRect_Reference#Rect">Rect</a>. |
| <a href="#SkMatrix_mapRectScaleTranslate">mapRectScaleTranslate</a> | Returns bounds of mapped <a href="SkRect_Reference#Rect">Rect</a>. |
| <a href="#SkMatrix_mapRectToQuad">mapRectToQuad</a> | Maps <a href="SkRect_Reference#Rect">Rect</a> to <a href="undocumented#Point">Point</a> array. |
| <a href="#SkMatrix_mapVector">mapVector</a> | Maps <a href="undocumented#Vector">Vector</a>. |
| <a href="#SkMatrix_mapVectors">mapVectors</a> | Maps <a href="undocumented#Vector">Vector</a> array. |
| <a href="#SkMatrix_mapXY">mapXY</a> | Maps <a href="undocumented#Point">Point</a>. |
| <a href="#SkMatrix_postConcat">postConcat</a> | Post-multiplies <a href="#Matrix">Matrix</a> by <a href="#Matrix">Matrix</a> parameter. |
| <a href="#SkMatrix_postIDiv">postIDiv</a> | Post-multiplies <a href="#Matrix">Matrix</a> by inverse scale. |
| <a href="#SkMatrix_postRotate">postRotate</a> | Post-multiplies <a href="#Matrix">Matrix</a> by rotation. |
| <a href="#SkMatrix_postScale">postScale</a> | Post-multiplies <a href="#Matrix">Matrix</a> by scale. |
| <a href="#SkMatrix_postSkew">postSkew</a> | Post-multiplies <a href="#Matrix">Matrix</a> by skew. |
| <a href="#SkMatrix_postTranslate">postTranslate</a> | Post-multiplies <a href="#Matrix">Matrix</a> by translation. |
| <a href="#SkMatrix_preConcat">preConcat</a> | Pre-multiplies <a href="#Matrix">Matrix</a> by <a href="#Matrix">Matrix</a> parameter. |
| <a href="#SkMatrix_preRotate">preRotate</a> | Pre-multiplies <a href="#Matrix">Matrix</a> by rotation. |
| <a href="#SkMatrix_preScale">preScale</a> | Pre-multiplies <a href="#Matrix">Matrix</a> by scale. |
| <a href="#SkMatrix_preSkew">preSkew</a> | Pre-multiplies <a href="#Matrix">Matrix</a> by skew. |
| <a href="#SkMatrix_preTranslate">preTranslate</a> | Pre-multiplies <a href="#Matrix">Matrix</a> by translation. |
| <a href="#SkMatrix_preservesAxisAlignment">preservesAxisAlignment</a> | Returns if mapping restricts to 90 degree multiples and mirroring. |
| <a href="#SkMatrix_preservesRightAngles">preservesRightAngles</a> | Returns if mapped 90 angle remains 90 degrees. |
| <a href="#SkMatrix_readFromMemory">readFromMemory</a> | Sets <a href="#Matrix">Matrix</a> from buffer pointing to <a href="#Scalar">Scalar</a> array. |
| <a href="#SkMatrix_rectStaysRect">rectStaysRect</a> | Returns if mapped <a href="SkRect_Reference#Rect">Rect</a> can be represented by another <a href="SkRect_Reference#Rect">Rect</a>. |
| <a href="#SkMatrix_reset">reset</a> | Sets <a href="#Matrix">Matrix</a> to identity. |
| <a href="#SkMatrix_set">set</a> | Sets one value. |
| <a href="#SkMatrix_set9">set9</a> | Sets all values from <a href="#Scalar">Scalar</a> array. |
| <a href="#SkMatrix_setAffine">setAffine</a> | Sets left two columns. |
| <a href="#SkMatrix_setAll">setAll</a> | Sets all values from parameters. |
| <a href="#SkMatrix_setConcat">setConcat</a> | Sets to <a href="#Matrix">Matrix</a> parameter multiplied by <a href="#Matrix">Matrix</a> parameter. |
| <a href="#SkMatrix_setIDiv">setIDiv</a> | Sets to inverse scale. |
| <a href="#SkMatrix_setIdentity">setIdentity</a> | Sets <a href="#Matrix">Matrix</a> to identity. |
| <a href="#SkMatrix_setPerspX">setPerspX</a> | Sets input x perspective factor. |
| <a href="#SkMatrix_setPerspY">setPerspY</a> | Sets input y perspective factor. |
| <a href="#SkMatrix_setPolyToPoly">setPolyToPoly</a> | Sets to map one to four points to an equal array of points. |
| <a href="#SkMatrix_setRSXform">setRSXform</a> | Sets to rotate, scale, and translate. |
| <a href="#SkMatrix_setRectToRect">setRectToRect</a> | Sets to map one <a href="SkRect_Reference#Rect">Rect</a> to another. |
| <a href="#SkMatrix_setRotate">setRotate</a> | Sets to rotate about a point. |
| <a href="#SkMatrix_setScale">setScale</a> | Sets to scale about a point. |
| <a href="#SkMatrix_setScaleTranslate">setScaleTranslate</a> | Sets to scale and translate. |
| <a href="#SkMatrix_setScaleX">setScaleX</a> | Sets horizontal scale factor. |
| <a href="#SkMatrix_setScaleY">setScaleY</a> | Sets vertical scale factor |
| <a href="#SkMatrix_setSinCos">setSinCos</a> | Sets to rotate and scale about a point. |
| <a href="#SkMatrix_setSkew">setSkew</a> | Sets to skew about a point. |
| <a href="#SkMatrix_setSkewX">setSkewX</a> | Sets horizontal skew factor. |
| <a href="#SkMatrix_setSkewY">setSkewY</a> | Sets vertical skew factor. |
| <a href="#SkMatrix_setTranslate">setTranslate</a> | Sets to translate in x and y. |
| <a href="#SkMatrix_setTranslateX">setTranslateX</a> | Sets horizontal translation. |
| <a href="#SkMatrix_setTranslateY">setTranslateY</a> | Sets vertical translation. |
| <a href="#SkMatrix_toString">toString</a> | Converts <a href="#Matrix">Matrix</a> to machine readable form. |
| <a href="#SkMatrix_writeToMemory">writeToMemory</a> | Writes array of <a href="#Scalar">Scalar</a> to buffer. |

<a name="SkMatrix_MakeScale"></a>
## MakeScale

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static SkMatrix SK_WARN_UNUSED_RESULT MakeScale(SkScalar sx, SkScalar sy)
</pre>

Sets <a href="#Matrix">Matrix</a> to scale by (<a href="#SkMatrix_MakeScale_sx">sx</a>, <a href="#SkMatrix_MakeScale_sy">sy</a>). Returned matrix is:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
| <a href="#SkMatrix_MakeScale_sx">sx</a>  0  0 |
|  0 <a href="#SkMatrix_MakeScale_sy">sy</a>  0 |
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

<a href="#SkMatrix_setScale">setScale</a> <a href="#SkMatrix_postScale">postScale</a> <a href="#SkMatrix_preScale">preScale</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static SkMatrix SK_WARN_UNUSED_RESULT MakeScale(SkScalar scale)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#SkMatrix_MakeScale_2_scale">scale</a> by (<a href="#SkMatrix_MakeScale_2_scale">scale</a>, <a href="#SkMatrix_MakeScale_2_scale">scale</a>). Returned matrix is:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
| <a href="#SkMatrix_MakeScale_2_scale">scale</a>   0   0 |
|   0   <a href="#SkMatrix_MakeScale_2_scale">scale</a> 0 |
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

<a href="#SkMatrix_setScale">setScale</a> <a href="#SkMatrix_postScale">postScale</a> <a href="#SkMatrix_preScale">preScale</a>

---

<a name="SkMatrix_MakeTrans"></a>
## MakeTrans

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static SkMatrix SK_WARN_UNUSED_RESULT MakeTrans(SkScalar dx, SkScalar dy)
</pre>

Sets <a href="#Matrix">Matrix</a> to translate by (<a href="#SkMatrix_MakeTrans_dx">dx</a>, <a href="#SkMatrix_MakeTrans_dy">dy</a>). Returned matrix is:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  1  0 0 |
|  0  1 0 |
| <a href="#SkMatrix_MakeTrans_dx">dx</a> <a href="#SkMatrix_MakeTrans_dy">dy</a> 1 |</pre>

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

<a href="#SkMatrix_setTranslate">setTranslate</a> <a href="#SkMatrix_postTranslate">postTranslate</a> <a href="#SkMatrix_preTranslate">preTranslate</a>

---

## <a name="SkMatrix_TypeMask"></a> Enum SkMatrix::TypeMask

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
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
    <td><a name="SkMatrix_kTranslate_Mask"> <code><strong>SkMatrix::kTranslate_Mask </strong></code> </a></td><td>1</td><td><a href="#SkMatrix_set">set</a> if <a href="#Matrix">Matrix</a> has translation</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kScale_Mask"> <code><strong>SkMatrix::kScale_Mask </strong></code> </a></td><td>2</td><td><a href="#SkMatrix_set">set</a> if <a href="#Matrix">Matrix</a> has x or y scale</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kAffine_Mask"> <code><strong>SkMatrix::kAffine_Mask </strong></code> </a></td><td>4</td><td><a href="#SkMatrix_set">set</a> if <a href="#Matrix">Matrix</a> skews or rotates</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kPerspective_Mask"> <code><strong>SkMatrix::kPerspective_Mask </strong></code> </a></td><td>8</td><td><a href="#SkMatrix_set">set</a> if <a href="#Matrix">Matrix</a> has perspective</td>
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



<a name="SkMatrix_getType"></a>
## getType

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
TypeMask getType() const
</pre>

Returns a bit field describing the transformations the matrix may
perform. The bit field is computed conservatively, so it may include
false positives. For example, when <a href="#SkMatrix_kPerspective_Mask">kPerspective Mask</a> is <a href="#SkMatrix_set">set</a>, all
other bits are <a href="#SkMatrix_set">set</a>.

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

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isIdentity() const
</pre>

Returns true if <a href="#Matrix">Matrix</a> is identity.  Identity matrix is:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
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

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isScaleTranslate() const
</pre>

Returns true if <a href="#Matrix">Matrix</a> at most scales and translates. <a href="#Matrix">Matrix</a> may be identity,
contain only scale elements, only translate elements, or both. <a href="#Matrix">Matrix</a> form is:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
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

<a href="#SkMatrix_setScale">setScale</a> <a href="#SkMatrix_isTranslate">isTranslate</a> <a href="#SkMatrix_setTranslate">setTranslate</a> <a href="#SkMatrix_getType">getType</a>

---

<a name="SkMatrix_isTranslate"></a>
## isTranslate

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isTranslate() const
</pre>

Returns true if <a href="#Matrix">Matrix</a> is identity, or translates. <a href="#Matrix">Matrix</a> form is:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
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

<a href="#SkMatrix_setTranslate">setTranslate</a> <a href="#SkMatrix_getType">getType</a>

---

<a name="SkMatrix_rectStaysRect"></a>
## rectStaysRect

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool rectStaysRect() const
</pre>

Returns true <a href="#Matrix">Matrix</a> maps <a href="SkRect_Reference#Rect">Rect</a> to another <a href="SkRect_Reference#Rect">Rect</a>. If true, <a href="#Matrix">Matrix</a> is identity,
or scales, or rotates a multiple of 90 degrees, or mirrors in x or y. In all
cases, <a href="#Matrix">Matrix</a> may also have translation. <a href="#Matrix">Matrix</a> form is either:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
| scale-x    0    translate-x |
|    0    scale-y translate-y |
|    0       0         1      |</pre>

or

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
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

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool preservesAxisAlignment() const
</pre>

Returns true <a href="#Matrix">Matrix</a> maps <a href="SkRect_Reference#Rect">Rect</a> to another <a href="SkRect_Reference#Rect">Rect</a>. If true, <a href="#Matrix">Matrix</a> is identity,
or scales, or rotates a multiple of 90 degrees, or mirrors in x or y. In all
cases, <a href="#Matrix">Matrix</a> may also have translation. <a href="#Matrix">Matrix</a> form is either:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
| scale-x    0    translate-x |
|    0    scale-y translate-y |
|    0       0         1      |</pre>

or

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
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

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool hasPerspective() const
</pre>

Returns true if the matrix contains perspective elements. <a href="#Matrix">Matrix</a> form is:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|       --            --              --          |
|       --            --              --          |
| perspective-x  perspective-y  perspective-scale |</pre>

where perspective-x or perspective-y is non-zero, or perspective-scale is 
not one. All other elements may have any value.

### Return Value

true if <a href="#Matrix">Matrix</a> is in most general form

### Example

<div><fiddle-embed name="06a480fe7baeec99a49d256089e89a5b"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setAll">setAll</a> <a href="#SkMatrix_set9">set9</a>

---

<a name="SkMatrix_isSimilarity"></a>
## isSimilarity

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isSimilarity(SkScalar tol = SK_ScalarNearlyZero) const
</pre>

Returns true if <a href="#Matrix">Matrix</a> contains only translation, rotation, reflection, and
uniform scale.
Returns false if <a href="#Matrix">Matrix</a> contains different scales, skewing, perspective, or
degenerate forms that collapse to a line or point.

Describes that the <a href="#Matrix">Matrix</a> makes rendering with and without the matrix are 
visually alike; a transformed circle remains a circle. Mathematically, this is
referred to as similarity of a <a href="undocumented#Euclidean">Euclidean</a> space, or a similarity transformation.

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

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
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

<div><fiddle-embed name="d79fba194a2a136b8ad22ee765a7a983"><div>Equal scale is both similar and preserves right angles.
Unequal scale is not similar but preserves right angles.
Skews are not similar and do not preserve right angles.</div></fiddle-embed></div>

### See Also

<a href="#SkMatrix_isScaleTranslate">isScaleTranslate</a> <a href="#SkMatrix_isSimilarity">isSimilarity</a> <a href="#SkMatrix_rectStaysRect">rectStaysRect</a> <a href="#SkMatrix_isFixedStepInX">isFixedStepInX</a>

---

## <a name="SkMatrix__anonymous"></a> Enum SkMatrix::_anonymous

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
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

<a href="#Matrix">Matrix</a> organizes its values in row order. This enum members corresponds to
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

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum {
<a href="#SkMatrix_kAScaleX">kAScaleX</a>,
<a href="#SkMatrix_kASkewY">kASkewY</a>,
<a href="#SkMatrix_kASkewX">kASkewX</a>,
<a href="#SkMatrix_kAScaleY">kAScaleY</a>,
<a href="#SkMatrix_kATransX">kATransX</a>,
<a href="#SkMatrix_kATransY">kATransY</a>,
};</pre>

Affine arrays are in column major order to match the matrix used by
<a href="undocumented#PDF">PDF</a> and <a href="undocumented#XPS">XPS</a>.

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



<a name="SkMatrix_subscript_operator_const"></a>
## operator[]

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar operator[](int index) const
</pre>

Returns one matrix value. Asserts if <a href="#SkMatrix_subscript_operator_const_index">index</a> is out of range and <a href="undocumented#SK_DEBUG">SK DEBUG</a> is
defined.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_subscript_operator_const_index"> <code><strong>index </strong></code> </a></td> <td>
one of: <a href="#SkMatrix_kMScaleX">kMScaleX</a>, <a href="#SkMatrix_kMSkewX">kMSkewX</a>, <a href="#SkMatrix_kMTransX">kMTransX</a>, <a href="#SkMatrix_kMSkewY">kMSkewY</a>, <a href="#SkMatrix_kMScaleY">kMScaleY</a>, <a href="#SkMatrix_kMTransY">kMTransY</a>,
<a href="#SkMatrix_kMPersp0">kMPersp0</a>, <a href="#SkMatrix_kMPersp1">kMPersp1</a>, <a href="#SkMatrix_kMPersp2">kMPersp2</a></td>
  </tr>
</table>

### Return Value

value corresponding to <a href="#SkMatrix_subscript_operator_const_index">index</a>

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

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar get(int index) const
</pre>

Returns one matrix value. Asserts if <a href="#SkMatrix_get_index">index</a> is out of range and <a href="undocumented#SK_DEBUG">SK DEBUG</a> is
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

<a href="#SkMatrix_subscript_operator_const">operator[](int index)</a> <a href="#SkMatrix_set">set</a>

---

<a name="SkMatrix_getScaleX"></a>
## getScaleX

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar getScaleX() const
</pre>

Returns scale factor multiplied by x input, contributing to x output.
With <a href="#SkMatrix_mapPoints">mapPoints</a>, scales <a href="#Point">Points</a> along the x-axis.

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

<a href="#SkMatrix_get">get</a> <a href="#SkMatrix_getScaleY">getScaleY</a> <a href="#SkMatrix_setScaleX">setScaleX</a> <a href="#SkMatrix_setScale">setScale</a>

---

<a name="SkMatrix_getScaleY"></a>
## getScaleY

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar getScaleY() const
</pre>

Returns scale factor multiplied by y input, contributing to y output.
With <a href="#SkMatrix_mapPoints">mapPoints</a>, scales <a href="#Point">Points</a> along the y-axis.

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

<a href="#SkMatrix_get">get</a> <a href="#SkMatrix_getScaleX">getScaleX</a> <a href="#SkMatrix_setScaleY">setScaleY</a> <a href="#SkMatrix_setScale">setScale</a>

---

<a name="SkMatrix_getSkewY"></a>
## getSkewY

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar getSkewY() const
</pre>

Returns scale factor multiplied by x input, contributing to y output.
With <a href="#SkMatrix_mapPoints">mapPoints</a>, skews <a href="#Point">Points</a> along the y-axis.
Skew x and y together can rotate <a href="#Point">Points</a>.

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

<a href="#SkMatrix_get">get</a> <a href="#SkMatrix_getSkewX">getSkewX</a> <a href="#SkMatrix_setSkewY">setSkewY</a> <a href="#SkMatrix_setSkew">setSkew</a>

---

<a name="SkMatrix_getSkewX"></a>
## getSkewX

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar getSkewX() const
</pre>

Returns scale factor multiplied by y input, contributing to x output.
With <a href="#SkMatrix_mapPoints">mapPoints</a>, skews <a href="#Point">Points</a> along the x-axis.
Skew x and y together can rotate <a href="#Point">Points</a>.

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

<a href="#SkMatrix_get">get</a> <a href="#SkMatrix_getSkewY">getSkewY</a> <a href="#SkMatrix_setSkewX">setSkewX</a> <a href="#SkMatrix_setSkew">setSkew</a>

---

<a name="SkMatrix_getTranslateX"></a>
## getTranslateX

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar getTranslateX() const
</pre>

Returns translation contributing to x output.
With <a href="#SkMatrix_mapPoints">mapPoints</a>, moves <a href="#Point">Points</a> along the x-axis.

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

<a href="#SkMatrix_get">get</a> <a href="#SkMatrix_getTranslateY">getTranslateY</a> <a href="#SkMatrix_setTranslateX">setTranslateX</a> <a href="#SkMatrix_setTranslate">setTranslate</a>

---

<a name="SkMatrix_getTranslateY"></a>
## getTranslateY

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar getTranslateY() const
</pre>

Returns translation contributing to y output.
With <a href="#SkMatrix_mapPoints">mapPoints</a>, moves <a href="#Point">Points</a> along the y-axis.

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

<a href="#SkMatrix_get">get</a> <a href="#SkMatrix_getTranslateX">getTranslateX</a> <a href="#SkMatrix_setTranslateY">setTranslateY</a> <a href="#SkMatrix_setTranslate">setTranslate</a>

---

<a name="SkMatrix_getPerspX"></a>
## getPerspX

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
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

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
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

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar& operator[](int index)
</pre>

Returns writable <a href="#Matrix">Matrix</a> value. Asserts if <a href="#SkMatrix_subscript_operator_index">index</a> is out of range and <a href="undocumented#SK_DEBUG">SK DEBUG</a> is
defined. Clears internal cache anticipating that caller will change <a href="#Matrix">Matrix</a> value.

Next call to read <a href="#Matrix">Matrix</a> state may recompute cache; subsequent writes to <a href="#Matrix">Matrix</a>
value must be followed by <a href="#SkMatrix_dirtyMatrixTypeCache">dirtyMatrixTypeCache</a>.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_subscript_operator_index"> <code><strong>index </strong></code> </a></td> <td>
one of: <a href="#SkMatrix_kMScaleX">kMScaleX</a>, <a href="#SkMatrix_kMSkewX">kMSkewX</a>, <a href="#SkMatrix_kMTransX">kMTransX</a>, <a href="#SkMatrix_kMSkewY">kMSkewY</a>, <a href="#SkMatrix_kMScaleY">kMScaleY</a>, <a href="#SkMatrix_kMTransY">kMTransY</a>,
<a href="#SkMatrix_kMPersp0">kMPersp0</a>, <a href="#SkMatrix_kMPersp1">kMPersp1</a>, <a href="#SkMatrix_kMPersp2">kMPersp2</a></td>
  </tr>
</table>

### Return Value

writable value corresponding to <a href="#SkMatrix_subscript_operator_index">index</a>

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

<a name="SkMatrix_set"></a>
## set

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void set(int index, SkScalar value)
</pre>

Sets <a href="#Matrix">Matrix</a> <a href="#SkMatrix_set_value">value</a>. Asserts if <a href="#SkMatrix_set_index">index</a> is out of range and <a href="undocumented#SK_DEBUG">SK DEBUG</a> is
defined. Safer than operator[]; internal cache is always maintained.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_set_index"> <code><strong>index </strong></code> </a></td> <td>
one of: <a href="#SkMatrix_kMScaleX">kMScaleX</a>, <a href="#SkMatrix_kMSkewX">kMSkewX</a>, <a href="#SkMatrix_kMTransX">kMTransX</a>, <a href="#SkMatrix_kMSkewY">kMSkewY</a>, <a href="#SkMatrix_kMScaleY">kMScaleY</a>, <a href="#SkMatrix_kMTransY">kMTransY</a>,
<a href="#SkMatrix_kMPersp0">kMPersp0</a>, <a href="#SkMatrix_kMPersp1">kMPersp1</a>, <a href="#SkMatrix_kMPersp2">kMPersp2</a></td>
  </tr>  <tr>    <td><a name="SkMatrix_set_value"> <code><strong>value </strong></code> </a></td> <td>
<a href="#Scalar">Scalar</a> to store in <a href="#Matrix">Matrix</a></td>
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

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setScaleX(SkScalar v)
</pre>

Sets horizontal scale factor. 

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setScaleX_v"> <code><strong>v </strong></code> </a></td> <td>
horizontal scale factor to store</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="b7bedc0617d17236503bdc25abe2c9d6"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_set">set</a> <a href="#SkMatrix_setScale">setScale</a> <a href="#SkMatrix_setScaleY">setScaleY</a>

---

<a name="SkMatrix_setScaleY"></a>
## setScaleY

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setScaleY(SkScalar v)
</pre>

Sets vertical scale factor. 

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setScaleY_v"> <code><strong>v </strong></code> </a></td> <td>
vertical scale factor to store</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="142c8f45430ce9ab71b8f6602d0a2220"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_set">set</a> <a href="#SkMatrix_setScale">setScale</a> <a href="#SkMatrix_setScaleX">setScaleX</a>

---

<a name="SkMatrix_setSkewY"></a>
## setSkewY

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setSkewY(SkScalar v)
</pre>

Sets vertical skew factor. 

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setSkewY_v"> <code><strong>v </strong></code> </a></td> <td>
vertical skew factor to store</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="abf2a8c67e26fee5952d0f9d65619c37"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_set">set</a> <a href="#SkMatrix_setSkew">setSkew</a> <a href="#SkMatrix_setSkewX">setSkewX</a>

---

<a name="SkMatrix_setSkewX"></a>
## setSkewX

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
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

<a href="#SkMatrix_set">set</a> <a href="#SkMatrix_setSkew">setSkew</a> <a href="#SkMatrix_setSkewX">setSkewX</a>

---

<a name="SkMatrix_setTranslateX"></a>
## setTranslateX

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setTranslateX(SkScalar v)
</pre>

Sets horizontal translation.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setTranslateX_v"> <code><strong>v </strong></code> </a></td> <td>
horizontal translation to store</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="68a0f8b379278d2b68306a7dfc00c0a8"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_set">set</a> <a href="#SkMatrix_setTranslate">setTranslate</a> <a href="#SkMatrix_setTranslateY">setTranslateY</a>

---

<a name="SkMatrix_setTranslateY"></a>
## setTranslateY

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setTranslateY(SkScalar v)
</pre>

Sets vertical translation.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setTranslateY_v"> <code><strong>v </strong></code> </a></td> <td>
vertical translation to store</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1d5336c5e31aaba5dc4e64dd215a3f5a"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_set">set</a> <a href="#SkMatrix_setTranslate">setTranslate</a> <a href="#SkMatrix_setTranslateX">setTranslateX</a>

---

<a name="SkMatrix_setPerspX"></a>
## setPerspX

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
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

<div><fiddle-embed name="89bdf655e97f3836ab01d17274dafbca"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_getPerspX">getPerspX</a> <a href="#SkMatrix_set">set</a> <a href="#SkMatrix_setAll">setAll</a> <a href="#SkMatrix_set9">set9</a>

---

<a name="SkMatrix_setPerspY"></a>
## setPerspY

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
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

<div><fiddle-embed name="bde4d22aed5f60edabc999d4c510d2cc"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_getPerspY">getPerspY</a> <a href="#SkMatrix_set">set</a> <a href="#SkMatrix_setAll">setAll</a> <a href="#SkMatrix_set9">set9</a>

---

<a name="SkMatrix_setAll"></a>
## setAll

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setAll(SkScalar scaleX, SkScalar skewX, SkScalar transX, SkScalar skewY,
            SkScalar scaleY, SkScalar transY, SkScalar persp0, SkScalar persp1,
            SkScalar persp2)
</pre>

Sets all values from parameters. Sets matrix to:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
| <a href="#SkMatrix_setAll_scaleX">scaleX</a>  <a href="#SkMatrix_setAll_skewX">skewX</a> <a href="#SkMatrix_setAll_transX">transX</a> |
|  <a href="#SkMatrix_setAll_skewY">skewY</a> <a href="#SkMatrix_setAll_scaleY">scaleY</a> <a href="#SkMatrix_setAll_transY">transY</a> |
| <a href="#SkMatrix_setAll_persp0">persp0</a> <a href="#SkMatrix_setAll_persp1">persp1</a> <a href="#SkMatrix_setAll_persp2">persp2</a> |</pre>

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

<div><fiddle-embed name="c51d81b637b58da30fb9ae2d68877167"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_set9">set9</a>

---

<a name="SkMatrix_get9"></a>
## get9

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void get9(SkScalar buffer[9]) const
</pre>

Copies nine <a href="#Scalar">Scalar</a> values contained by <a href="#Matrix">Matrix</a> into <a href="#SkMatrix_get9_buffer">buffer</a>, in member value
ascending order: <a href="#SkMatrix_kMScaleX">kMScaleX</a>, <a href="#SkMatrix_kMSkewX">kMSkewX</a>, <a href="#SkMatrix_kMTransX">kMTransX</a>, <a href="#SkMatrix_kMSkewY">kMSkewY</a>, <a href="#SkMatrix_kMScaleY">kMScaleY</a>, <a href="#SkMatrix_kMTransY">kMTransY</a>,
<a href="#SkMatrix_kMPersp0">kMPersp0</a>, <a href="#SkMatrix_kMPersp1">kMPersp1</a>, <a href="#SkMatrix_kMPersp2">kMPersp2</a>.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_get9_buffer"> <code><strong>buffer </strong></code> </a></td> <td>
storage for nine <a href="#Scalar">Scalar</a> values</td>
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

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void set9(const SkScalar buffer[9])
</pre>

Sets <a href="#Matrix">Matrix</a> to nine <a href="#Scalar">Scalar</a> values in <a href="#SkMatrix_set9_buffer">buffer</a>, in member value ascending order:
<a href="#SkMatrix_kMScaleX">kMScaleX</a>, <a href="#SkMatrix_kMSkewX">kMSkewX</a>, <a href="#SkMatrix_kMTransX">kMTransX</a>, <a href="#SkMatrix_kMSkewY">kMSkewY</a>, <a href="#SkMatrix_kMScaleY">kMScaleY</a>, <a href="#SkMatrix_kMTransY">kMTransY</a>, <a href="#SkMatrix_kMPersp0">kMPersp0</a>, <a href="#SkMatrix_kMPersp1">kMPersp1</a>,
<a href="#SkMatrix_kMPersp2">kMPersp2</a>.

Sets matrix to:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
</pre>

In the future, <a href="#SkMatrix_set9">set9</a> followed by <a href="#SkMatrix_get9">get9</a> may not return the same values. Since <a href="#Matrix">Matrix</a>
maps non-homogeneous coordinates, scaling all nine values produces an equivalent
transformation, possibly improving precision.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_set9_buffer"> <code><strong>buffer </strong></code> </a></td> <td>
nine <a href="#Scalar">Scalar</a> values</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="ec5de0d23e5fe28ba7628625d1402e85"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setAll">setAll</a> <a href="#SkMatrix_get9">get9</a>

---

<a name="SkMatrix_reset"></a>
## reset

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void reset()
</pre>

Sets <a href="#Matrix">Matrix</a> to identity; which has no effect on mapped <a href="#Point">Points</a>. Sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
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

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setIdentity()
</pre>

Sets <a href="#Matrix">Matrix</a> to identity; which has no effect on mapped <a href="#Point">Points</a>. Sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
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

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
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

<div><fiddle-embed name="193d71489c653471d7faa6932d0b7352"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setTranslateX">setTranslateX</a> <a href="#SkMatrix_setTranslateY">setTranslateY</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setTranslate(const SkVector& v)
</pre>

Sets <a href="#Matrix">Matrix</a> to translate by (<a href="#SkMatrix_setTranslate_2_v">v</a>.fX, <a href="#SkMatrix_setTranslate_2_v">v</a>.fY).

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setTranslate_2_v"> <code><strong>v </strong></code> </a></td> <td>
<a href="undocumented#Vector">Vector</a> containing horizontal and vertical translation</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="dd63c8f0d37e7fad9e4e38dde58f524f"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setTranslateX">setTranslateX</a> <a href="#SkMatrix_setTranslateY">setTranslateY</a> <a href="#SkMatrix_MakeTrans">MakeTrans</a>

---

<a name="SkMatrix_setScale"></a>
## setScale

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
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

<div><fiddle-embed name="397db8da89bc92e7c576ad013d64ed24"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setScaleX">setScaleX</a> <a href="#SkMatrix_setScaleY">setScaleY</a> <a href="#SkMatrix_MakeScale">MakeScale</a> <a href="#SkMatrix_preScale">preScale</a> <a href="#SkMatrix_postScale">postScale</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
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

<div><fiddle-embed name="94a9ec11b994580dca14aa2159a796a9"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setScaleX">setScaleX</a> <a href="#SkMatrix_setScaleY">setScaleY</a> <a href="#SkMatrix_MakeScale">MakeScale</a> <a href="#SkMatrix_setIDiv">setIDiv</a> <a href="#SkMatrix_preScale">preScale</a> <a href="#SkMatrix_postScale">postScale</a>

---

<a name="SkMatrix_setIDiv"></a>
## setIDiv

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool setIDiv(int divx, int divy)
</pre>

Sets <a href="#Matrix">Matrix</a> to scale by inverse of <a href="#SkMatrix_setIDiv_divx">divx</a> and inverse of <a href="#SkMatrix_setIDiv_divy">divy</a>. Returns false if
either <a href="#SkMatrix_setIDiv_divx">divx</a> or <a href="#SkMatrix_setIDiv_divy">divy</a> is zero, leaving <a href="#Matrix">Matrix</a> unchanged.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setIDiv_divx"> <code><strong>divx </strong></code> </a></td> <td>
inverse horizontal scale factor</td>
  </tr>  <tr>    <td><a name="SkMatrix_setIDiv_divy"> <code><strong>divy </strong></code> </a></td> <td>
inverse vertical scale factor</td>
  </tr>
</table>

### Return Value

true if inverse factors applied

to be deprecated

### See Also

<a href="#SkMatrix_setScale">setScale</a> <a href="#SkMatrix_MakeScale">MakeScale</a>

---

<a name="SkMatrix_setRotate"></a>
## setRotate

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
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

<div><fiddle-embed name="5c393839fe4e3fdc499e3505e38ebeb3"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setSinCos">setSinCos</a> <a href="#SkMatrix_preRotate">preRotate</a> <a href="#SkMatrix_postRotate">postRotate</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
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

<div><fiddle-embed name="41f0d48c825d00df873cb2226420d666"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setSinCos">setSinCos</a> <a href="#SkMatrix_preRotate">preRotate</a> <a href="#SkMatrix_postRotate">postRotate</a>

---

<a name="SkMatrix_setSinCos"></a>
## setSinCos

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setSinCos(SkScalar sinValue, SkScalar cosValue, SkScalar px, SkScalar py)
</pre>

Sets <a href="#Matrix">Matrix</a> to rotate by <a href="#SkMatrix_setSinCos_sinValue">sinValue</a> and <a href="#SkMatrix_setSinCos_cosValue">cosValue</a>, about a pivot point at (<a href="#SkMatrix_setSinCos_px">px</a>, <a href="#SkMatrix_setSinCos_py">py</a>). 
The pivot point is unchanged when mapped with <a href="#Matrix">Matrix</a>.

<a href="undocumented#Vector">Vector</a> (<a href="#SkMatrix_setSinCos_sinValue">sinValue</a>, <a href="#SkMatrix_setSinCos_cosValue">cosValue</a>) describes the angle of rotation relative to (0, 1).
<a href="undocumented#Vector">Vector</a> length specifies scale.

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

<div><fiddle-embed name="55d5e1648c021552ab4bebd75248825d"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setRotate">setRotate</a> <a href="#SkMatrix_setScale">setScale</a> <a href="#SkMatrix_setRSXform">setRSXform</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setSinCos(SkScalar sinValue, SkScalar cosValue)
</pre>

Sets <a href="#Matrix">Matrix</a> to rotate by <a href="#SkMatrix_setSinCos_2_sinValue">sinValue</a> and <a href="#SkMatrix_setSinCos_2_cosValue">cosValue</a>, about a pivot point at (0, 0). 

<a href="undocumented#Vector">Vector</a> (<a href="#SkMatrix_setSinCos_2_sinValue">sinValue</a>, <a href="#SkMatrix_setSinCos_2_cosValue">cosValue</a>) describes the angle of rotation relative to (0, 1).
<a href="undocumented#Vector">Vector</a> length specifies scale.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setSinCos_2_sinValue"> <code><strong>sinValue </strong></code> </a></td> <td>
rotation vector x component</td>
  </tr>  <tr>    <td><a name="SkMatrix_setSinCos_2_cosValue"> <code><strong>cosValue </strong></code> </a></td> <td>
rotation vector y component</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="b03725cce159a83da58469d62ca7bf30"><div><a href="SkCanvas_Reference#Canvas">Canvas</a> needs offset after applying <a href="#Matrix">Matrix</a> to pivot about <a href="SkRect_Reference#Rect">Rect</a> center.</div></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setRotate">setRotate</a> <a href="#SkMatrix_setScale">setScale</a> <a href="#SkMatrix_setRSXform">setRSXform</a>

---

<a name="SkMatrix_setRSXform"></a>
## setRSXform

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkMatrix& setRSXform(const SkRSXform& rsxForm)
</pre>

Sets <a href="#Matrix">Matrix</a> to rotate, scale, and translate using a compressed matrix form.

<a href="undocumented#Vector">Vector</a> (<a href="#SkMatrix_setRSXform_rsxForm">rsxForm</a>.fSSin, <a href="#SkMatrix_setRSXform_rsxForm">rsxForm</a>.fSCos) describes the angle of rotation relative
to (0, 1). <a href="undocumented#Vector">Vector</a> length specifies scale. Mapped point is rotated and scaled
by <a href="undocumented#Vector">Vector</a>, then translated by (<a href="#SkMatrix_setRSXform_rsxForm">rsxForm</a>.fTx, <a href="#SkMatrix_setRSXform_rsxForm">rsxForm</a>.fTy).

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setRSXform_rsxForm"> <code><strong>rsxForm </strong></code> </a></td> <td>
compressed <a href="undocumented#RSXform">RSXform</a> matrix</td>
  </tr>
</table>

### Return Value

reference to <a href="#Matrix">Matrix</a>

### Example

<div><fiddle-embed name="0ef22feb5aa205db83513f81958fcfe7"><div><a href="SkCanvas_Reference#Canvas">Canvas</a> needs offset after applying <a href="#Matrix">Matrix</a> to pivot about <a href="SkRect_Reference#Rect">Rect</a> center.</div></fiddle-embed></div>

### See Also

<a href="#SkMatrix_setSinCos">setSinCos</a> <a href="#SkMatrix_setScale">setScale</a> <a href="#SkMatrix_setTranslate">setTranslate</a>

---

<a name="SkMatrix_setSkew"></a>
## setSkew

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setSkew(SkScalar kx, SkScalar ky, SkScalar px, SkScalar py)
</pre>

Sets <a href="#Matrix">Matrix</a> to skew by sx and sy, about a pivot point at (<a href="#SkMatrix_setSkew_px">px</a>, <a href="#SkMatrix_setSkew_py">py</a>).
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

<a href="#SkMatrix_setSkewX">setSkewX</a> <a href="#SkMatrix_setSkewY">setSkewY</a> <a href="#SkMatrix_preSkew">preSkew</a> <a href="#SkMatrix_postSkew">postSkew</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setSkew(SkScalar kx, SkScalar ky)
</pre>

Sets <a href="#Matrix">Matrix</a> to skew by sx and sy, about a pivot point at (0, 0).

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

<a href="#SkMatrix_setSkewX">setSkewX</a> <a href="#SkMatrix_setSkewY">setSkewY</a> <a href="#SkMatrix_preSkew">preSkew</a> <a href="#SkMatrix_postSkew">postSkew</a>

---

<a name="SkMatrix_setConcat"></a>
## setConcat

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setConcat(const SkMatrix& a, const SkMatrix& b)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> <a href="#SkMatrix_setConcat_a">a</a> multiplied by <a href="#Matrix">Matrix</a> <a href="#SkMatrix_setConcat_b">b</a>. Either <a href="#SkMatrix_setConcat_a">a</a> or <a href="#SkMatrix_setConcat_b">b</a> may be this.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  <a href="undocumented#C">C</a> |      |  |
<a href="#SkMatrix_setConcat_a">a</a> = |  |, <a href="#SkMatrix_setConcat_b">b</a> = |  |
|  <a href="#SkMatrix_I">I</a> |      |  |</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  <a href="undocumented#C">C</a> |   |  |   | AJ+BM+ AK+BN+ AL+BO+ |
<a href="#SkMatrix_setConcat_a">a</a> * <a href="#SkMatrix_setConcat_b">b</a> = |  | * |  | = | DJ+EM+ DK+EN+ DL+EO+ |
|  <a href="#SkMatrix_I">I</a> |   |  |   | GJ+HM+ GK+HN+ GL+HO+ |</pre>

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

<a href="#SkMatrix_Concat">Concat</a> <a href="#SkMatrix_preConcat">preConcat</a> <a href="#SkMatrix_postConcat">postConcat</a> <a href="#SkCanvas_concat">SkCanvas::concat</a>

---

<a name="SkMatrix_preTranslate"></a>
## preTranslate

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void preTranslate(SkScalar dx, SkScalar dy)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> multiplied by <a href="#Matrix">Matrix</a> constructed from translation (<a href="#SkMatrix_preTranslate_dx">dx</a>, <a href="#SkMatrix_preTranslate_dy">dy</a>).
This can be thought of as moving the point to be mapped before applying <a href="#Matrix">Matrix</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  <a href="undocumented#C">C</a> |               | 1 0 <a href="#SkMatrix_preTranslate_dx">dx</a> |
<a href="#Matrix">Matrix</a> = |  |,   = | 0 1 <a href="#SkMatrix_preTranslate_dy">dy</a> |
|  <a href="#SkMatrix_I">I</a> |               | 0 0  1 |</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  <a href="undocumented#C">C</a> | | 1 0 <a href="#SkMatrix_preTranslate_dx">dx</a> |   |  A*<a href="#SkMatrix_preTranslate_dx">dx</a>+B*<a href="#SkMatrix_preTranslate_dy">dy</a>+<a href="undocumented#C">C</a> |
<a href="#Matrix">Matrix</a> *  = |  | | 0 1 <a href="#SkMatrix_preTranslate_dy">dy</a> | = |  D*<a href="#SkMatrix_preTranslate_dx">dx</a>+E*<a href="#SkMatrix_preTranslate_dy">dy</a>+ |
|  <a href="#SkMatrix_I">I</a> | | 0 0  1 |   |  G*<a href="#SkMatrix_preTranslate_dx">dx</a>+H*<a href="#SkMatrix_preTranslate_dy">dy</a>+<a href="#SkMatrix_I">I</a> |</pre>

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

<a href="#SkMatrix_postTranslate">postTranslate</a> <a href="#SkMatrix_setTranslate">setTranslate</a> <a href="#SkMatrix_MakeTrans">MakeTrans</a>

---

<a name="SkMatrix_preScale"></a>
## preScale

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void preScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> multiplied by <a href="#Matrix">Matrix</a> constructed from scaling by (<a href="#SkMatrix_preScale_sx">sx</a>, <a href="#SkMatrix_preScale_sy">sy</a>)
about pivot point (<a href="#SkMatrix_preScale_px">px</a>, <a href="#SkMatrix_preScale_py">py</a>).
This can be thought of as scaling about a pivot point before applying <a href="#Matrix">Matrix</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  <a href="undocumented#C">C</a> |                       | <a href="#SkMatrix_preScale_sx">sx</a>  0 dx |
<a href="#Matrix">Matrix</a> = |  |,   = |  0 <a href="#SkMatrix_preScale_sy">sy</a> dy |
|  <a href="#SkMatrix_I">I</a> |                       |  0  0  1 |</pre>

where

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
dx = <a href="#SkMatrix_preScale_px">px</a> - <a href="#SkMatrix_preScale_sx">sx</a> * <a href="#SkMatrix_preScale_px">px</a>
dy = <a href="#SkMatrix_preScale_py">py</a> - <a href="#SkMatrix_preScale_sy">sy</a> * <a href="#SkMatrix_preScale_py">py</a></pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  <a href="undocumented#C">C</a> | | <a href="#SkMatrix_preScale_sx">sx</a>  0 dx |   | A*<a href="#SkMatrix_preScale_sx">sx</a> B*<a href="#SkMatrix_preScale_sy">sy</a> A*dx+B*dy+<a href="undocumented#C">C</a> |
<a href="#Matrix">Matrix</a> *  = |  | |  0 <a href="#SkMatrix_preScale_sy">sy</a> dy | = | D*<a href="#SkMatrix_preScale_sx">sx</a> E*<a href="#SkMatrix_preScale_sy">sy</a> D*dx+E*dy+ |
|  <a href="#SkMatrix_I">I</a> | |  0  0  1 |   | G*<a href="#SkMatrix_preScale_sx">sx</a> H*<a href="#SkMatrix_preScale_sy">sy</a> G*dx+H*dy+<a href="#SkMatrix_I">I</a> |</pre>

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

<a href="#SkMatrix_postScale">postScale</a> <a href="#SkMatrix_setScale">setScale</a> <a href="#SkMatrix_MakeScale">MakeScale</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void preScale(SkScalar sx, SkScalar sy)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> multiplied by <a href="#Matrix">Matrix</a> constructed from scaling by (<a href="#SkMatrix_preScale_2_sx">sx</a>, <a href="#SkMatrix_preScale_2_sy">sy</a>)
about pivot point (0, 0).
This can be thought of as scaling about the origin before applying <a href="#Matrix">Matrix</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  <a href="undocumented#C">C</a> |               | <a href="#SkMatrix_preScale_2_sx">sx</a>  0  0 |
<a href="#Matrix">Matrix</a> = |  |,   = |  0 <a href="#SkMatrix_preScale_2_sy">sy</a>  0 |
|  <a href="#SkMatrix_I">I</a> |               |  0  0  1 |</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  <a href="undocumented#C">C</a> | | <a href="#SkMatrix_preScale_2_sx">sx</a>  0  0 |   | A*<a href="#SkMatrix_preScale_2_sx">sx</a> B*<a href="#SkMatrix_preScale_2_sy">sy</a> <a href="undocumented#C">C</a> |
<a href="#Matrix">Matrix</a> *  = |  | |  0 <a href="#SkMatrix_preScale_2_sy">sy</a>  0 | = | D*<a href="#SkMatrix_preScale_2_sx">sx</a> E*<a href="#SkMatrix_preScale_2_sy">sy</a> |
|  <a href="#SkMatrix_I">I</a> | |  0  0  1 |   | G*<a href="#SkMatrix_preScale_2_sx">sx</a> H*<a href="#SkMatrix_preScale_2_sy">sy</a> <a href="#SkMatrix_I">I</a> |</pre>

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

<a href="#SkMatrix_postScale">postScale</a> <a href="#SkMatrix_setScale">setScale</a> <a href="#SkMatrix_MakeScale">MakeScale</a>

---

<a name="SkMatrix_preRotate"></a>
## preRotate

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void preRotate(SkScalar degrees, SkScalar px, SkScalar py)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> multiplied by <a href="#Matrix">Matrix</a> constructed from rotating by <a href="#SkMatrix_preRotate_degrees">degrees</a>
about pivot point (<a href="#SkMatrix_preRotate_px">px</a>, <a href="#SkMatrix_preRotate_py">py</a>).
This can be thought of as rotating about a pivot point before applying <a href="#Matrix">Matrix</a>.

Positive <a href="#SkMatrix_preRotate_degrees">degrees</a> rotates clockwise.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  <a href="undocumented#C">C</a> |                        | c -s dx |
<a href="#Matrix">Matrix</a> = |  |,   = | s  c dy |
|  <a href="#SkMatrix_I">I</a> |                        | 0  0  1 |</pre>

where

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
c  = cos(degrees)
s  = sin(degrees)
dx =  s * <a href="#SkMatrix_preRotate_py">py</a> + (1 - c) * <a href="#SkMatrix_preRotate_px">px</a>
dy = -s * <a href="#SkMatrix_preRotate_px">px</a> + (1 - c) * <a href="#SkMatrix_preRotate_py">py</a></pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  <a href="undocumented#C">C</a> | | c -s dx |   | Ac+ -As+ A*dx+B*dy+<a href="undocumented#C">C</a> |
<a href="#Matrix">Matrix</a> *  = |  | | s  c dy | = | Dc+ -Ds+ D*dx+E*dy+ |
|  <a href="#SkMatrix_I">I</a> | | 0  0  1 |   | Gc+ -Gs+ G*dx+H*dy+<a href="#SkMatrix_I">I</a> |</pre>

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

<div><fiddle-embed name="fa357b3ff6a8521a1fc3469c5f086474"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_postRotate">postRotate</a> <a href="#SkMatrix_setRotate">setRotate</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void preRotate(SkScalar degrees)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> multiplied by <a href="#Matrix">Matrix</a> constructed from rotating by <a href="#SkMatrix_preRotate_2_degrees">degrees</a>
about pivot point (0, 0).
This can be thought of as rotating about the origin before applying <a href="#Matrix">Matrix</a>.

Positive <a href="#SkMatrix_preRotate_2_degrees">degrees</a> rotates clockwise.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  <a href="undocumented#C">C</a> |                        | c -s 0 |
<a href="#Matrix">Matrix</a> = |  |,   = | s  c 0 |
|  <a href="#SkMatrix_I">I</a> |                        | 0  0 1 |</pre>

where

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
c  = cos(degrees)
s  = sin(degrees)</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  <a href="undocumented#C">C</a> | | c -s 0 |   | Ac+ -As+ <a href="undocumented#C">C</a> |
<a href="#Matrix">Matrix</a> *  = |  | | s  c 0 | = | Dc+ -Ds+ |
|  <a href="#SkMatrix_I">I</a> | | 0  0 1 |   | Gc+ -Gs+ <a href="#SkMatrix_I">I</a> |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_preRotate_2_degrees"> <code><strong>degrees </strong></code> </a></td> <td>
angle of axes relative to upright axes</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="2e95f8ae5c82ba02c770eacf60d5a798"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_postRotate">postRotate</a> <a href="#SkMatrix_setRotate">setRotate</a>

---

<a name="SkMatrix_preSkew"></a>
## preSkew

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void preSkew(SkScalar kx, SkScalar ky, SkScalar px, SkScalar py)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> multiplied by <a href="#Matrix">Matrix</a> constructed from skewing by (<a href="#SkMatrix_preSkew_kx">kx</a>, <a href="#SkMatrix_preSkew_ky">ky</a>)
about pivot point (<a href="#SkMatrix_preSkew_px">px</a>, <a href="#SkMatrix_preSkew_py">py</a>).
This can be thought of as skewing about a pivot point before applying <a href="#Matrix">Matrix</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  <a href="undocumented#C">C</a> |                       |  1 <a href="#SkMatrix_preSkew_kx">kx</a> dx |
<a href="#Matrix">Matrix</a> = |  |,   = | <a href="#SkMatrix_preSkew_ky">ky</a>  1 dy |
|  <a href="#SkMatrix_I">I</a> |                       |  0  0  1 |</pre>

where

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
dx = -<a href="#SkMatrix_preSkew_kx">kx</a> * <a href="#SkMatrix_preSkew_py">py</a>
dy = -<a href="#SkMatrix_preSkew_ky">ky</a> * <a href="#SkMatrix_preSkew_px">px</a></pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  <a href="undocumented#C">C</a> | |  1 <a href="#SkMatrix_preSkew_kx">kx</a> dx |   | A+B*<a href="#SkMatrix_preSkew_ky">ky</a> A*<a href="#SkMatrix_preSkew_kx">kx</a>+ A*dx+B*dy+<a href="undocumented#C">C</a> |
<a href="#Matrix">Matrix</a> *  = |  | | <a href="#SkMatrix_preSkew_ky">ky</a>  1 dy | = | D+E*<a href="#SkMatrix_preSkew_ky">ky</a> D*<a href="#SkMatrix_preSkew_kx">kx</a>+ D*dx+E*dy+ |
|  <a href="#SkMatrix_I">I</a> | |  0  0  1 |   | G+H*<a href="#SkMatrix_preSkew_ky">ky</a> G*<a href="#SkMatrix_preSkew_kx">kx</a>+ G*dx+H*dy+<a href="#SkMatrix_I">I</a> |</pre>

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

<a href="#SkMatrix_postSkew">postSkew</a> <a href="#SkMatrix_setSkew">setSkew</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void preSkew(SkScalar kx, SkScalar ky)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> multiplied by <a href="#Matrix">Matrix</a> constructed from skewing by (<a href="#SkMatrix_preSkew_2_kx">kx</a>, <a href="#SkMatrix_preSkew_2_ky">ky</a>)
about pivot point (0, 0).
This can be thought of as skewing about the origin before applying <a href="#Matrix">Matrix</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  <a href="undocumented#C">C</a> |               |  1 <a href="#SkMatrix_preSkew_2_kx">kx</a> 0 |
<a href="#Matrix">Matrix</a> = |  |,   = | <a href="#SkMatrix_preSkew_2_ky">ky</a>  1 0 |
|  <a href="#SkMatrix_I">I</a> |               |  0  0 1 |</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  <a href="undocumented#C">C</a> | |  1 <a href="#SkMatrix_preSkew_2_kx">kx</a> 0 |   | A+B*<a href="#SkMatrix_preSkew_2_ky">ky</a> A*<a href="#SkMatrix_preSkew_2_kx">kx</a>+ <a href="undocumented#C">C</a> |
<a href="#Matrix">Matrix</a> *  = |  | | <a href="#SkMatrix_preSkew_2_ky">ky</a>  1 0 | = | D+E*<a href="#SkMatrix_preSkew_2_ky">ky</a> D*<a href="#SkMatrix_preSkew_2_kx">kx</a>+ |
|  <a href="#SkMatrix_I">I</a> | |  0  0 1 |   | G+H*<a href="#SkMatrix_preSkew_2_ky">ky</a> G*<a href="#SkMatrix_preSkew_2_kx">kx</a>+ <a href="#SkMatrix_I">I</a> |</pre>

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

<a href="#SkMatrix_postSkew">postSkew</a> <a href="#SkMatrix_setSkew">setSkew</a>

---

<a name="SkMatrix_preConcat"></a>
## preConcat

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void preConcat(const SkMatrix& other)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> multiplied by <a href="#Matrix">Matrix</a> <a href="#SkMatrix_preConcat_other">other</a>.
This can be thought of mapping by <a href="#SkMatrix_preConcat_other">other</a> before applying <a href="#Matrix">Matrix</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  <a href="undocumented#C">C</a> |          |  |
<a href="#Matrix">Matrix</a> = |  |, <a href="#SkMatrix_preConcat_other">other</a> = |  |
|  <a href="#SkMatrix_I">I</a> |          |  |</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  <a href="undocumented#C">C</a> |   |  |   | AJ+BM+ AK+BN+ AL+BO+ |
<a href="#Matrix">Matrix</a> * <a href="#SkMatrix_preConcat_other">other</a> = |  | * |  | = | DJ+EM+ DK+EN+ DL+EO+ |
|  <a href="#SkMatrix_I">I</a> |   |  |   | GJ+HM+ GK+HN+ GL+HO+ |</pre>

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

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void postTranslate(SkScalar dx, SkScalar dy)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> constructed from translation (<a href="#SkMatrix_postTranslate_dx">dx</a>, <a href="#SkMatrix_postTranslate_dy">dy</a>) multiplied by <a href="#Matrix">Matrix</a>.
This can be thought of as moving the point to be mapped after applying <a href="#Matrix">Matrix</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  |               | 1 0 <a href="#SkMatrix_postTranslate_dx">dx</a> |
<a href="#Matrix">Matrix</a> = |  |,   = | 0 1 <a href="#SkMatrix_postTranslate_dy">dy</a> |
|  |               | 0 0  1 |</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|                      1 0 <a href="#SkMatrix_postTranslate_dx">dx</a> | |  |   | J+<a href="#SkMatrix_postTranslate_dx">dx</a>* K+<a href="#SkMatrix_postTranslate_dx">dx</a>* L+<a href="#SkMatrix_postTranslate_dx">dx</a>* |
* <a href="#Matrix">Matrix</a> = | 0 1 <a href="#SkMatrix_postTranslate_dy">dy</a> | |  | = | M+<a href="#SkMatrix_postTranslate_dy">dy</a>* N+<a href="#SkMatrix_postTranslate_dy">dy</a>* O+<a href="#SkMatrix_postTranslate_dy">dy</a>* |
| 0 0  1 | |  |   |       |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_postTranslate_dx"> <code><strong>dx </strong></code> </a></td> <td>
x translation after applying <a href="#Matrix">Matrix</a></td>
  </tr>  <tr>    <td><a name="SkMatrix_postTranslate_dy"> <code><strong>dy </strong></code> </a></td> <td>
y translation after applying <a href="#Matrix">Matrix</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="141c9197c3b725701a13132da258826c"><div>Compare with <a href="#SkMatrix_preTranslate">preTranslate</a> example.</div></fiddle-embed></div>

### See Also

<a href="#SkMatrix_preTranslate">preTranslate</a> <a href="#SkMatrix_setTranslate">setTranslate</a> <a href="#SkMatrix_MakeTrans">MakeTrans</a>

---

<a name="SkMatrix_postScale"></a>
## postScale

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void postScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> constructed from scaling by (<a href="#SkMatrix_postScale_sx">sx</a>, <a href="#SkMatrix_postScale_sy">sy</a>) about pivot point
(<a href="#SkMatrix_postScale_px">px</a>, <a href="#SkMatrix_postScale_py">py</a>), multiplied by <a href="#Matrix">Matrix</a>.
This can be thought of as scaling about a pivot point after applying <a href="#Matrix">Matrix</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  |                       | <a href="#SkMatrix_postScale_sx">sx</a>  0 dx |
<a href="#Matrix">Matrix</a> = |  |,   = |  0 <a href="#SkMatrix_postScale_sy">sy</a> dy |
|  |                       |  0  0  1 |</pre>

where

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
dx = <a href="#SkMatrix_postScale_px">px</a> - <a href="#SkMatrix_postScale_sx">sx</a> * <a href="#SkMatrix_postScale_px">px</a>
dy = <a href="#SkMatrix_postScale_py">py</a> - <a href="#SkMatrix_postScale_sy">sy</a> * <a href="#SkMatrix_postScale_py">py</a></pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|                              <a href="#SkMatrix_postScale_sx">sx</a>  0 dx | |  |   | <a href="#SkMatrix_postScale_sx">sx</a>*J+dx* <a href="#SkMatrix_postScale_sx">sx</a>*K+dx* <a href="#SkMatrix_postScale_sx">sx</a>*L+dx+ |
* <a href="#Matrix">Matrix</a> = |  0 <a href="#SkMatrix_postScale_sy">sy</a> dy | |  | = | <a href="#SkMatrix_postScale_sy">sy</a>*M+dy* <a href="#SkMatrix_postScale_sy">sy</a>*N+dy* <a href="#SkMatrix_postScale_sy">sy</a>*O+dy* |
|  0  0  1 | |  |   |          |</pre>

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

<a href="#SkMatrix_preScale">preScale</a> <a href="#SkMatrix_setScale">setScale</a> <a href="#SkMatrix_MakeScale">MakeScale</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void postScale(SkScalar sx, SkScalar sy)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> constructed from scaling by (<a href="#SkMatrix_postScale_2_sx">sx</a>, <a href="#SkMatrix_postScale_2_sy">sy</a>) about pivot point
(0, 0), multiplied by <a href="#Matrix">Matrix</a>.
This can be thought of as scaling about the origin after applying <a href="#Matrix">Matrix</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  |               | <a href="#SkMatrix_postScale_2_sx">sx</a>  0  0 |
<a href="#Matrix">Matrix</a> = |  |,   = |  0 <a href="#SkMatrix_postScale_2_sy">sy</a>  0 |
|  |               |  0  0  1 |</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|                      <a href="#SkMatrix_postScale_2_sx">sx</a>  0  0 | |  |   | <a href="#SkMatrix_postScale_2_sx">sx</a>* <a href="#SkMatrix_postScale_2_sx">sx</a>* <a href="#SkMatrix_postScale_2_sx">sx</a>* |
* <a href="#Matrix">Matrix</a> = |  0 <a href="#SkMatrix_postScale_2_sy">sy</a>  0 | |  | = | <a href="#SkMatrix_postScale_2_sy">sy</a>* <a href="#SkMatrix_postScale_2_sy">sy</a>* <a href="#SkMatrix_postScale_2_sy">sy</a>* |
|  0  0  1 | |  |   |     |</pre>

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

<a href="#SkMatrix_preScale">preScale</a> <a href="#SkMatrix_setScale">setScale</a> <a href="#SkMatrix_MakeScale">MakeScale</a>

---

<a name="SkMatrix_postIDiv"></a>
## postIDiv

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool postIDiv(int divx, int divy)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> constructed from scaling by(1/<a href="#SkMatrix_postIDiv_divx">divx</a>, 1/<a href="#SkMatrix_postIDiv_divy">divy</a>)about pivot point (px, py), multiplied by <a href="#Matrix">Matrix</a>.

Returns false if either <a href="#SkMatrix_postIDiv_divx">divx</a> or divyh is zero

Given:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  |                   | sx  0  0 |
<a href="#Matrix">Matrix</a> = |  |,</pre>

where

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
sx = 1 / <a href="#SkMatrix_postIDiv_divx">divx</a>
sy = 1 / <a href="#SkMatrix_postIDiv_divy">divy</a></pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|                          sx  0  0 | |  |   | sx* sx* sx* |
</pre>

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

<div><fiddle-embed name="bf45c127433865f1887bdef4a6420f11"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_postScale">postScale</a> <a href="#SkMatrix_MakeScale">MakeScale</a>

---

<a name="SkMatrix_postRotate"></a>
## postRotate

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void postRotate(SkScalar degrees, SkScalar px, SkScalar py)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> constructed from rotating by <a href="#SkMatrix_postRotate_degrees">degrees</a> about pivot point
(<a href="#SkMatrix_postRotate_px">px</a>, <a href="#SkMatrix_postRotate_py">py</a>), multiplied by <a href="#Matrix">Matrix</a>.
This can be thought of as rotating about a pivot point after applying <a href="#Matrix">Matrix</a>.

Positive <a href="#SkMatrix_postRotate_degrees">degrees</a> rotates clockwise.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  |                        | c -s dx |
<a href="#Matrix">Matrix</a> = |  |,   = | s  c dy |
|  |                        | 0  0  1 |</pre>

where

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
c  = cos(degrees)
s  = sin(degrees)
dx =  s * <a href="#SkMatrix_postRotate_py">py</a> + (1 - c) * <a href="#SkMatrix_postRotate_px">px</a>
dy = -s * <a href="#SkMatrix_postRotate_px">px</a> + (1 - c) * <a href="#SkMatrix_postRotate_py">py</a></pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|                               c -s dx | |  |   | cJ-sM+dx* cK-sN+dx* cL-sO+dx+ |
* <a href="#Matrix">Matrix</a> = | s  c dy | |  | = | sJ+cM+dy* sK+cN+dy* sL+cO+dy* |
| 0  0  1 | |  |   |           |</pre>

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

<div><fiddle-embed name="06a34bd00f3dedcdc7540cd346e7e77f"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_preRotate">preRotate</a> <a href="#SkMatrix_setRotate">setRotate</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void postRotate(SkScalar degrees)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> constructed from rotating by <a href="#SkMatrix_postRotate_2_degrees">degrees</a> about pivot point
(0, 0), multiplied by <a href="#Matrix">Matrix</a>.
This can be thought of as rotating about the origin after applying <a href="#Matrix">Matrix</a>.

Positive <a href="#SkMatrix_postRotate_2_degrees">degrees</a> rotates clockwise.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  |                        | c -s 0 |
<a href="#Matrix">Matrix</a> = |  |,   = | s  c 0 |
|  |                        | 0  0 1 |</pre>

where

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
c  = cos(degrees)
s  = sin(degrees)</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|                               c -s dx | |  |   | cJ-sM cK-sN cL-sO |
* <a href="#Matrix">Matrix</a> = | s  c dy | |  | = | sJ+cM sK+cN sL+cO |
| 0  0  1 | |  |   |      |</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_postRotate_2_degrees"> <code><strong>degrees </strong></code> </a></td> <td>
angle of axes relative to upright axes</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="64b435d61b271e5f79f7525d6e37a67d"></fiddle-embed></div>

### See Also

<a href="#SkMatrix_preRotate">preRotate</a> <a href="#SkMatrix_setRotate">setRotate</a>

---

<a name="SkMatrix_postSkew"></a>
## postSkew

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void postSkew(SkScalar kx, SkScalar ky, SkScalar px, SkScalar py)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> constructed from skewing by (<a href="#SkMatrix_postSkew_kx">kx</a>, <a href="#SkMatrix_postSkew_ky">ky</a>) about pivot point
(<a href="#SkMatrix_postSkew_px">px</a>, <a href="#SkMatrix_postSkew_py">py</a>), multiplied by <a href="#Matrix">Matrix</a>.
This can be thought of as skewing about a pivot point after applying <a href="#Matrix">Matrix</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  |                       |  1 <a href="#SkMatrix_postSkew_kx">kx</a> dx |
<a href="#Matrix">Matrix</a> = |  |,   = | <a href="#SkMatrix_postSkew_ky">ky</a>  1 dy |
|  |                       |  0  0  1 |</pre>

where

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
dx = -<a href="#SkMatrix_postSkew_kx">kx</a> * <a href="#SkMatrix_postSkew_py">py</a>
dy = -<a href="#SkMatrix_postSkew_ky">ky</a> * <a href="#SkMatrix_postSkew_px">px</a></pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|                               1 <a href="#SkMatrix_postSkew_kx">kx</a> dx | |  |   | J+<a href="#SkMatrix_postSkew_kx">kx</a>*M+dx* K+<a href="#SkMatrix_postSkew_kx">kx</a>*N+dx* L+<a href="#SkMatrix_postSkew_kx">kx</a>*O+dx+ |
* <a href="#Matrix">Matrix</a> = | <a href="#SkMatrix_postSkew_ky">ky</a>  1 dy | |  | = | <a href="#SkMatrix_postSkew_ky">ky</a>*J+M+dy* <a href="#SkMatrix_postSkew_ky">ky</a>*K+N+dy* <a href="#SkMatrix_postSkew_ky">ky</a>*L+O+dy* |
|  0  0  1 | |  |   |            |</pre>

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

<a href="#SkMatrix_preSkew">preSkew</a> <a href="#SkMatrix_setSkew">setSkew</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void postSkew(SkScalar kx, SkScalar ky)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> constructed from skewing by (<a href="#SkMatrix_postSkew_2_kx">kx</a>, <a href="#SkMatrix_postSkew_2_ky">ky</a>) about pivot point
(0, 0), multiplied by <a href="#Matrix">Matrix</a>.
This can be thought of as skewing about the origin after applying <a href="#Matrix">Matrix</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  |               |  1 <a href="#SkMatrix_postSkew_2_kx">kx</a> 0 |
<a href="#Matrix">Matrix</a> = |  |,   = | <a href="#SkMatrix_postSkew_2_ky">ky</a>  1 0 |
|  |               |  0  0 1 |</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|                       1 <a href="#SkMatrix_postSkew_2_kx">kx</a> 0 | |  |   | J+<a href="#SkMatrix_postSkew_2_kx">kx</a>* K+<a href="#SkMatrix_postSkew_2_kx">kx</a>* L+<a href="#SkMatrix_postSkew_2_kx">kx</a>* |
* <a href="#Matrix">Matrix</a> = | <a href="#SkMatrix_postSkew_2_ky">ky</a>  1 0 | |  | = | <a href="#SkMatrix_postSkew_2_ky">ky</a>*J+ <a href="#SkMatrix_postSkew_2_ky">ky</a>*K+ <a href="#SkMatrix_postSkew_2_ky">ky</a>*L+ |
|  0  0 1 | |  |   |       |</pre>

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

<a href="#SkMatrix_preSkew">preSkew</a> <a href="#SkMatrix_setSkew">setSkew</a>

---

<a name="SkMatrix_postConcat"></a>
## postConcat

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void postConcat(const SkMatrix& other)
</pre>

Sets <a href="#Matrix">Matrix</a> to <a href="#Matrix">Matrix</a> <a href="#SkMatrix_postConcat_other">other</a> multiplied by <a href="#Matrix">Matrix</a>.
This can be thought of mapping by <a href="#SkMatrix_postConcat_other">other</a> after applying <a href="#Matrix">Matrix</a>.

Given:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  |           |  <a href="undocumented#C">C</a> |
<a href="#Matrix">Matrix</a> = |  |,  <a href="#SkMatrix_postConcat_other">other</a> = |  |
|  |           |  <a href="#SkMatrix_I">I</a> |</pre>

sets <a href="#Matrix">Matrix</a> to:

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
|  <a href="undocumented#C">C</a> |   |  |   | AJ+BM+ AK+BN+ AL+BO+ |
<a href="#SkMatrix_postConcat_other">other</a> * <a href="#Matrix">Matrix</a> = |  | * |  | = | DJ+EM+ DK+EN+ DL+EO+ |
|  <a href="#SkMatrix_I">I</a> |   |  |   | GJ+HM+ GK+HN+ GL+HO+ |</pre>

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

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="#SkMatrix_ScaleToFit">ScaleToFit</a> {
<a href="#SkMatrix_kFill_ScaleToFit">kFill ScaleToFit</a>,
<a href="#SkMatrix_kStart_ScaleToFit">kStart ScaleToFit</a>,
<a href="#SkMatrix_kCenter_ScaleToFit">kCenter ScaleToFit</a>,
<a href="#SkMatrix_kEnd_ScaleToFit">kEnd ScaleToFit</a>,
};</pre>

### Constants

<table>
  <tr>
    <td><a name="SkMatrix_kFill_ScaleToFit"> <code><strong>SkMatrix::kFill_ScaleToFit </strong></code> </a></td><td>Scale in X and Y independently, so that src matches dst exactly.</td><td>This may change the aspect ratio of the src.</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kStart_ScaleToFit"> <code><strong>SkMatrix::kStart_ScaleToFit </strong></code> </a></td><td>Compute a scale that will maintain the original src aspect ratio,</td><td>but will also ensure that src fits entirely inside dst. At least one
axis (x or y) will fit exactly. Aligns the result to the
left and top edges of dst.</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kCenter_ScaleToFit"> <code><strong>SkMatrix::kCenter_ScaleToFit </strong></code> </a></td><td>Compute a scale that will maintain the original src aspect ratio,</td><td>but will also ensure that src fits entirely inside dst. At least one
axis (x or y) will fit exactly. The result is centered inside dst.</td>
  </tr>
  <tr>
    <td><a name="SkMatrix_kEnd_ScaleToFit"> <code><strong>SkMatrix::kEnd_ScaleToFit </strong></code> </a></td><td>Compute a scale that will maintain the original src aspect ratio,</td><td>but will also ensure that src fits entirely inside dst. At least one
axis (x or y) will fit exactly. Aligns the result to the
right and bottom edges of dst.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>



<a name="SkMatrix_setRectToRect"></a>
## setRectToRect

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool setRectToRect(const SkRect& src, const SkRect& dst, ScaleToFit stf)
</pre>

Set the matrix to the scale and translate values that map the source
rectangle to the destination rectangle, returning true if the the result
can be represented.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setRectToRect_src"> <code><strong>src </strong></code> </a></td> <td>
the source rectangle to map from</td>
  </tr>  <tr>    <td><a name="SkMatrix_setRectToRect_dst"> <code><strong>dst </strong></code> </a></td> <td>
the destination rectangle to map to</td>
  </tr>  <tr>    <td><a name="SkMatrix_setRectToRect_stf"> <code><strong>stf </strong></code> </a></td> <td>
the <a href="#SkMatrix_ScaleToFit">ScaleToFit</a> option</td>
  </tr>
</table>

### Return Value

true if the matrix can be represented by the rectangle mapping

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_MakeRectToRect"></a>
## MakeRectToRect

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static SkMatrix MakeRectToRect(const SkRect& src, const SkRect& dst,
                               ScaleToFit stf)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_MakeRectToRect_src"> <code><strong>src </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkMatrix_MakeRectToRect_dst"> <code><strong>dst </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkMatrix_MakeRectToRect_stf"> <code><strong>stf </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_setPolyToPoly"></a>
## setPolyToPoly

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool setPolyToPoly(const SkPoint src[], const SkPoint dst[], int count)
</pre>

Set the matrix such that the specified <a href="#SkMatrix_setPolyToPoly_src">src</a> points would map to the
specified <a href="#SkMatrix_setPolyToPoly_dst">dst</a> points. <a href="#SkMatrix_setPolyToPoly_count">count</a> must be within [0..4].

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setPolyToPoly_src"> <code><strong>src </strong></code> </a></td> <td>
array of <a href="#SkMatrix_setPolyToPoly_src">src</a> points</td>
  </tr>  <tr>    <td><a name="SkMatrix_setPolyToPoly_dst"> <code><strong>dst </strong></code> </a></td> <td>
array of <a href="#SkMatrix_setPolyToPoly_dst">dst</a> points</td>
  </tr>  <tr>    <td><a name="SkMatrix_setPolyToPoly_count"> <code><strong>count </strong></code> </a></td> <td>
number of points to use for the transformation</td>
  </tr>
</table>

### Return Value

true if the matrix was <a href="#SkMatrix_set">set</a> to the specified transformation

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_invert"></a>
## invert

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool SK_WARN_UNUSED_RESULT invert(SkMatrix* inverse) const
</pre>

If this matrix can be inverted, return true and if <a href="#SkMatrix_invert_inverse">inverse</a> is not null,
<a href="#SkMatrix_set">set</a> <a href="#SkMatrix_invert_inverse">inverse</a> to be the <a href="#SkMatrix_invert_inverse">inverse</a> of this matrix. If this matrix cannot be
inverted, ignore <a href="#SkMatrix_invert_inverse">inverse</a> and return false

### Parameters

<table>  <tr>    <td><a name="SkMatrix_invert_inverse"> <code><strong>inverse </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_SetAffineIdentity"></a>
## SetAffineIdentity

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static void SetAffineIdentity(SkScalar affine[6])
</pre>

Fills the passed array with <a href="#SkMatrix_SetAffineIdentity_affine">affine</a> identity values
in column major order.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_SetAffineIdentity_affine"> <code><strong>affine </strong></code> </a></td> <td>
array to fill with <a href="#SkMatrix_SetAffineIdentity_affine">affine</a> identity values; must not be nullptr</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_asAffine"></a>
## asAffine

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool SK_WARN_UNUSED_RESULT asAffine(SkScalar affine[6]) const
</pre>

Fills the passed array with the <a href="#SkMatrix_asAffine_affine">affine</a> values in column major order.
If the matrix is a perspective transform, returns false
and does not change the passed array.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_asAffine_affine"> <code><strong>affine </strong></code> </a></td> <td>
array to fill with <a href="#SkMatrix_asAffine_affine">affine</a> values; ignored if nullptr</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_setAffine"></a>
## setAffine

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setAffine(const SkScalar affine[6])
</pre>

Set the matrix to the specified <a href="#SkMatrix_setAffine_affine">affine</a> values.
Note: these are passed in column major order.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setAffine_affine"> <code><strong>affine </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_mapPoints"></a>
## mapPoints

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void mapPoints(SkPoint dst[], const SkPoint src[], int count) const
</pre>

Apply this matrix to the array of points specified by <a href="#SkMatrix_mapPoints_src">src</a>, and write
the transformed points into the array of points specified by <a href="#SkMatrix_mapPoints_dst">dst</a>.
<a href="#SkMatrix_mapPoints_dst">dst</a>[] =  * <a href="#SkMatrix_mapPoints_src">src</a>[]

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapPoints_dst"> <code><strong>dst </strong></code> </a></td> <td>
storage for transformed coordinates; must
allow <a href="#SkMatrix_mapPoints_count">count</a> entries</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapPoints_src"> <code><strong>src </strong></code> </a></td> <td>
original coordinates that are to be transformed;
must allow <a href="#SkMatrix_mapPoints_count">count</a> entries</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapPoints_count"> <code><strong>count </strong></code> </a></td> <td>
number of points in <a href="#SkMatrix_mapPoints_src">src</a> to read, and then transform
into <a href="#SkMatrix_mapPoints_dst">dst</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void mapPoints(SkPoint pts[], int count) const
</pre>

Apply this matrix to the array of points, overwriting it with the
transformed values.
dst[] =  * <a href="#SkMatrix_mapPoints_2_pts">pts</a>[]

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapPoints_2_pts"> <code><strong>pts </strong></code> </a></td> <td>
storage for transformed points; must allow <a href="#SkMatrix_mapPoints_2_count">count</a> entries</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapPoints_2_count"> <code><strong>count </strong></code> </a></td> <td>
number of points in <a href="#SkMatrix_mapPoints_2_pts">pts</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_mapPointsWithStride"></a>
## mapPointsWithStride

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void mapPointsWithStride(SkPoint pts[], size_t stride, int count) const
</pre>

Like <a href="#SkMatrix_mapPoints">mapPoints</a> but with custom byte <a href="#SkMatrix_mapPointsWithStride_stride">stride</a> between the points. Stride
should be a multiple ofsizeof(SkScalar).

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapPointsWithStride_pts"> <code><strong>pts </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapPointsWithStride_stride"> <code><strong>stride </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapPointsWithStride_count"> <code><strong>count </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void mapPointsWithStride(SkPoint dst[], const SkPoint src[], size_t stride,
                         int count) const
</pre>

Like <a href="#SkMatrix_mapPoints">mapPoints</a> but with custom byte <a href="#SkMatrix_mapPointsWithStride_2_stride">stride</a> between the points.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapPointsWithStride_2_dst"> <code><strong>dst </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapPointsWithStride_2_src"> <code><strong>src </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapPointsWithStride_2_stride"> <code><strong>stride </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapPointsWithStride_2_count"> <code><strong>count </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_mapHomogeneousPoints"></a>
## mapHomogeneousPoints

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void mapHomogeneousPoints(SkScalar dst[], const SkScalar src[], int count) const
</pre>

Apply this matrix to the array of homogeneous points, specified by <a href="#SkMatrix_mapHomogeneousPoints_src">src</a>,
where a homogeneous point is defined by 3 contiguous scalar values,
and write the transformed points into the array of scalars specified by <a href="#SkMatrix_mapHomogeneousPoints_dst">dst</a>.
<a href="#SkMatrix_mapHomogeneousPoints_dst">dst</a>[] =  * <a href="#SkMatrix_mapHomogeneousPoints_src">src</a>[]

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapHomogeneousPoints_dst"> <code><strong>dst </strong></code> </a></td> <td>
storage for transformed coordinates; must
allow 3 * <a href="#SkMatrix_mapHomogeneousPoints_count">count</a> entries</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapHomogeneousPoints_src"> <code><strong>src </strong></code> </a></td> <td>
original coordinates to be transformed;
must contain at least 3 * <a href="#SkMatrix_mapHomogeneousPoints_count">count</a> entries</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapHomogeneousPoints_count"> <code><strong>count </strong></code> </a></td> <td>
number of triples (homogeneous points) in <a href="#SkMatrix_mapHomogeneousPoints_src">src</a> to read,
and then transform into <a href="#SkMatrix_mapHomogeneousPoints_dst">dst</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_mapXY"></a>
## mapXY

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void mapXY(SkScalar x, SkScalar y, SkPoint* result) const
</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapXY_x"> <code><strong>x </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapXY_y"> <code><strong>y </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapXY_result"> <code><strong>result </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkPoint mapXY(SkScalar x, SkScalar y) const
</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapXY_2_x"> <code><strong>x </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapXY_2_y"> <code><strong>y </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_mapVectors"></a>
## mapVectors

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void mapVectors(SkVector dst[], const SkVector src[], int count) const
</pre>

Apply this matrix to the array of vectors specified by <a href="#SkMatrix_mapVectors_src">src</a>, and write
the transformed vectors into the array of vectors specified by <a href="#SkMatrix_mapVectors_dst">dst</a>.
This is similar to <a href="#SkMatrix_mapPoints">mapPoints</a>, but ignores any translation in the matrix.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapVectors_dst"> <code><strong>dst </strong></code> </a></td> <td>
storage for transformed coordinates; must
allow <a href="#SkMatrix_mapVectors_count">count</a> entries</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapVectors_src"> <code><strong>src </strong></code> </a></td> <td>
coordinates to be transformed;
must contain at least <a href="#SkMatrix_mapVectors_count">count</a> entries</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapVectors_count"> <code><strong>count </strong></code> </a></td> <td>
number of vectors in <a href="#SkMatrix_mapVectors_src">src</a> to read and transform
into <a href="#SkMatrix_mapVectors_dst">dst</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void mapVectors(SkVector vecs[], int count) const
</pre>

Apply this matrix to <a href="#SkMatrix_mapVectors_2_count">count</a> vectors in array <a href="#SkMatrix_mapVectors_2_vecs">vecs</a>.
This is similar to <a href="#SkMatrix_mapPoints">mapPoints</a>, but ignores any translation in the matrix.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapVectors_2_vecs"> <code><strong>vecs </strong></code> </a></td> <td>
vectors to transform; must contain at least
<a href="#SkMatrix_mapVectors_2_count">count</a> entries</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapVectors_2_count"> <code><strong>count </strong></code> </a></td> <td>
number of vectors in <a href="#SkMatrix_mapVectors_2_vecs">vecs</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_mapVector"></a>
## mapVector

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void mapVector(SkScalar dx, SkScalar dy, SkVector* result) const
</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapVector_dx"> <code><strong>dx </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapVector_dy"> <code><strong>dy </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapVector_result"> <code><strong>result </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkVector mapVector(SkScalar dx, SkScalar dy) const
</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapVector_2_dx"> <code><strong>dx </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapVector_2_dy"> <code><strong>dy </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_mapRect"></a>
## mapRect

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool mapRect(SkRect* dst, const SkRect& src) const
</pre>

Apply this matrix to the <a href="#SkMatrix_mapRect_src">src</a> rectangle, and write the transformed
rectangle into <a href="#SkMatrix_mapRect_dst">dst</a>. This is accomplished by transforming the 4 corners
of <a href="#SkMatrix_mapRect_src">src</a>, and then setting <a href="#SkMatrix_mapRect_dst">dst</a> to the bounds of those points.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapRect_dst"> <code><strong>dst </strong></code> </a></td> <td>
storage for transformed rectangle</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapRect_src"> <code><strong>src </strong></code> </a></td> <td>
rectangle to transform</td>
  </tr>
</table>

### Return Value

result of calling <a href="#SkMatrix_rectStaysRect">rectStaysRect</a>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool mapRect(SkRect* rect) const
</pre>

Apply this matrix to the rectangle, and write the transformed rectangle
back into it. This is accomplished by transforming the 4 corners of
<a href="#SkMatrix_mapRect_2_rect">rect</a>, and then setting it to the bounds of those points

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapRect_2_rect"> <code><strong>rect </strong></code> </a></td> <td>
rectangle to transform</td>
  </tr>
</table>

### Return Value

the result of calling <a href="#SkMatrix_rectStaysRect">rectStaysRect</a>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_mapRectToQuad"></a>
## mapRectToQuad

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void mapRectToQuad(SkPoint dst[4], const SkRect& rect) const
</pre>

Applies <a href="#Matrix">Matrix</a> to <a href="#SkMatrix_mapRectToQuad_rect">rect</a>, and write the four transformed
points into <a href="#SkMatrix_mapRectToQuad_dst">dst</a>. The points written to <a href="#SkMatrix_mapRectToQuad_dst">dst</a> will be the original top-left, top-right,
bottom-right, and bottom-left points transformed by <a href="#Matrix">Matrix</a>.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapRectToQuad_dst"> <code><strong>dst </strong></code> </a></td> <td>
storage for transformed quad</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapRectToQuad_rect"> <code><strong>rect </strong></code> </a></td> <td>
rectangle to transform</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_mapRectScaleTranslate"></a>
## mapRectScaleTranslate

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void mapRectScaleTranslate(SkRect* dst, const SkRect& src) const
</pre>

Maps a rectangle to another rectangle, asserting (in debug mode) that the matrix only contains
scale and translate elements. If it contains other elements, the results are undefined.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapRectScaleTranslate_dst"> <code><strong>dst </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkMatrix_mapRectScaleTranslate_src"> <code><strong>src </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_mapRadius"></a>
## mapRadius

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar mapRadius(SkScalar radius) const
</pre>

Return the mean <a href="#SkMatrix_mapRadius_radius">radius</a> of a circle after it has been mapped by
this matrix. NOTE: in perspective this value assumes the circle
has its center at the origin.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_mapRadius_radius"> <code><strong>radius </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_GetMapXYProc"></a>
## GetMapXYProc

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static MapXYProc GetMapXYProc(TypeMask mask)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_GetMapXYProc_mask"> <code><strong>mask </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_getMapXYProc"></a>
## getMapXYProc

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
MapXYProc getMapXYProc() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_GetMapPtsProc"></a>
## GetMapPtsProc

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static MapPtsProc GetMapPtsProc(TypeMask mask)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_GetMapPtsProc_mask"> <code><strong>mask </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_getMapPtsProc"></a>
## getMapPtsProc

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
MapPtsProc getMapPtsProc() const
</pre>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_isFixedStepInX"></a>
## isFixedStepInX

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isFixedStepInX() const
</pre>

Returns true if the matrix can be stepped in x (not complex
perspective).

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_fixedStepInX"></a>
## fixedStepInX

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkVector fixedStepInX(SkScalar y) const
</pre>

If the matrix can be stepped in x (not complex perspective)
then return the step value.
If it cannot, behavior is undefined.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_fixedStepInX_y"> <code><strong>y </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_cheapEqualTo"></a>
## cheapEqualTo

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool cheapEqualTo(const SkMatrix& m) const
</pre>

Returns true if <a href="#Matrix">Matrix</a> equals <a href="#SkMatrix_cheapEqualTo_m">m</a>, using an efficient comparison.

Return false when the sign of zero values is the different, that is, one
matrix has positive zero value and the other has negative zero value.

Normally, comparing <a href="undocumented#NaN">NaN</a> prevents the value from equaling any other value,
including itself. To improve performance, <a href="undocumented#NaN">NaN</a> values are treated as bit patterns
that are equal if their bit patterns are equal.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_cheapEqualTo_m"> <code><strong>m </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_equal_operator"></a>
## operator==

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
friend SK_API bool operator==(const SkMatrix& a, const SkMatrix& b)
</pre>

mac chromium debug build requires <a href="undocumented#SK_API">SK API</a> to make operator== visible

### Parameters

<table>  <tr>    <td><a name="SkMatrix_equal_operator_a"> <code><strong>a </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkMatrix_equal_operator_b"> <code><strong>b </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_not_equal_operator"></a>
## operator!=

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
friend SK_API bool operator!=(const SkMatrix& a, const SkMatrix& b)
</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_not_equal_operator_a"> <code><strong>a </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkMatrix_not_equal_operator_b"> <code><strong>b </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

## <a name="SkMatrix__anonymous_3"></a> Enum SkMatrix::_anonymous_3

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum {
<a href="#SkMatrix_kMaxFlattenSize">kMaxFlattenSize</a> = 9 * sizeof(SkScalar) + sizeof(uint32_t),
};</pre>

### Constants

<table>
  <tr>
    <td><a name="SkMatrix_kMaxFlattenSize"> <code><strong>SkMatrix::kMaxFlattenSize </strong></code> </a></td><td>= 9 * sizeof(SkScalar) + sizeof(uint32_t)</td><td><a href="#SkMatrix_writeToMemory">writeToMemory</a> and <a href="#SkMatrix_readFromMemory">readFromMemory</a> will never return a value larger than this</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>



<a name="SkMatrix_writeToMemory"></a>
## writeToMemory

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
size_t writeToMemory(void* buffer) const
</pre>

return the number of bytes written, whether or not <a href="#SkMatrix_writeToMemory_buffer">buffer</a> is null

### Parameters

<table>  <tr>    <td><a name="SkMatrix_writeToMemory_buffer"> <code><strong>buffer </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_readFromMemory"></a>
## readFromMemory

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
size_t readFromMemory(const void* buffer, size_t length)
</pre>

Reads data from the <a href="#SkMatrix_readFromMemory_buffer">buffer</a> parameter

### Parameters

<table>  <tr>    <td><a name="SkMatrix_readFromMemory_buffer"> <code><strong>buffer </strong></code> </a></td> <td>
memory to read from</td>
  </tr>  <tr>    <td><a name="SkMatrix_readFromMemory_length"> <code><strong>length </strong></code> </a></td> <td>
amount of memory available in the <a href="#SkMatrix_readFromMemory_buffer">buffer</a></td>
  </tr>
</table>

### Return Value

number of bytes read (must be a multiple of 4) or
0 if there was not enough memory available

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_dump"></a>
## dump

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void dump() const
</pre>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_toString"></a>
## toString

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void toString(SkString* str) const
</pre>

### Parameters

<table>  <tr>    <td><a name="SkMatrix_toString_str"> <code><strong>str </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_getMinScale"></a>
## getMinScale

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar getMinScale() const
</pre>

Calculates the minimum scaling factor of the matrix as computed from the 
singular value decomposition of the upper
left 2x2. If the max scale factor cannot be computed (for example overflow or perspective)
-1 is returned.

### Return Value

minimum scale factor

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_getMaxScale"></a>
## getMaxScale

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar getMaxScale() const
</pre>

Calculates the maximum scaling factor of the matrix as computed from the 
singular value decomposition of the upper
left 2x2. If the max scale factor cannot be computed (for example overflow or perspective)
-1 is returned.

### Return Value

maximum scale factor

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_getMinMaxScales"></a>
## getMinMaxScales

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool SK_WARN_UNUSED_RESULT getMinMaxScales(SkScalar scaleFactors[2]) const
</pre>

Gets both the min and max scale factors. The min scale factor is <a href="#SkMatrix_getMinMaxScales_scaleFactors">scaleFactors</a>[0] and the max
is <a href="#SkMatrix_getMinMaxScales_scaleFactors">scaleFactors</a>[1]. If the min/max scale factors cannot be computed false is returned and the
values of <a href="#SkMatrix_getMinMaxScales_scaleFactors">scaleFactors</a>[] are undefined.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_getMinMaxScales_scaleFactors"> <code><strong>scaleFactors </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_decomposeScale"></a>
## decomposeScale

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool decomposeScale(SkSize* scale, SkMatrix* remaining = nullptr) const
</pre>

Attempt to decompose this matrix into a scale-only component and whatever remains, where
the <a href="#SkMatrix_decomposeScale_scale">scale</a> component is to be applied first.
M ->  * ScaleOn success, return true and assign the <a href="#SkMatrix_decomposeScale_scale">scale</a> and <a href="#SkMatrix_decomposeScale_remaining">remaining</a> components (assuming their
respective parameters are not null). On failure return false and ignore the parameters.
Possible reasons to fail: perspective, one or more <a href="#SkMatrix_decomposeScale_scale">scale</a> factors are zero.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_decomposeScale_scale"> <code><strong>scale </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkMatrix_decomposeScale_remaining"> <code><strong>remaining </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_I"></a>
## I

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static const SkMatrix& I()
</pre>

Return a reference to a const identity matrix

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_InvalidMatrix"></a>
## InvalidMatrix

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static const SkMatrix& InvalidMatrix()
</pre>

Return a reference to a const matrix that is "", one that could
never be used.

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_Concat"></a>
## Concat

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static SkMatrix Concat(const SkMatrix& a, const SkMatrix& b)
</pre>

Return the concatenation of two matrices, <a href="#SkMatrix_Concat_a">a</a> * <a href="#SkMatrix_Concat_b">b</a>.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_Concat_a"> <code><strong>a </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkMatrix_Concat_b"> <code><strong>b </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_dirtyMatrixTypeCache"></a>
## dirtyMatrixTypeCache

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void dirtyMatrixTypeCache()
</pre>

Testing routine; the matrix type cache should never need to be
manually invalidated during normal use.

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_setScaleTranslate"></a>
## setScaleTranslate

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setScaleTranslate(SkScalar sx, SkScalar sy, SkScalar tx, SkScalar ty)
</pre>

Initialize the matrix to be scale + post-translate.

### Parameters

<table>  <tr>    <td><a name="SkMatrix_setScaleTranslate_sx"> <code><strong>sx </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkMatrix_setScaleTranslate_sy"> <code><strong>sy </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkMatrix_setScaleTranslate_tx"> <code><strong>tx </strong></code> </a></td> <td>
incomplete</td>
  </tr>  <tr>    <td><a name="SkMatrix_setScaleTranslate_ty"> <code><strong>ty </strong></code> </a></td> <td>
incomplete</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

<a name="SkMatrix_isFinite"></a>
## isFinite

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isFinite() const
</pre>

Are all elements of the matrix finite?

### Return Value

incomplete

### Example

<div><fiddle-embed name="882e8e0103048009a25cfc20400492f7"></fiddle-embed></div>

---

