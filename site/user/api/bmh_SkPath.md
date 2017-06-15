# <a name="Path"></a> Path
<a href="bmh_SkPath?cl=9919#Path">Path</a> contains <a href="bmh_undocumented?cl=9919#Line">Lines</a> and <a href="bmh_undocumented?cl=9919#Curve">Curves</a> which can be stroked or filled. <a href="bmh_SkPath?cl=9919#Contour">Contour</a> is 
composed of a series of connected <a href="bmh_undocumented?cl=9919#Line">Lines</a> and <a href="bmh_undocumented?cl=9919#Curve">Curves</a>. <a href="bmh_SkPath?cl=9919#Path">Path</a> may contain zero, 
one, or more <a href="bmh_SkPath?cl=9919#Contour">Contours</a>.
Each <a href="bmh_undocumented?cl=9919#Line">Line</a> and <a href="bmh_undocumented?cl=9919#Curve">Curve</a> are described by <a href="bmh_SkPath?cl=9919#Verb">Verb</a>, <a href="bmh_undocumented?cl=9919#Point">Points</a>, and optional <a href="bmh_SkPath?cl=9919#Weight">Weight</a>.

Each pair of connected <a href="bmh_undocumented?cl=9919#Line">Lines</a> and <a href="bmh_undocumented?cl=9919#Curve">Curves</a> share common <a href="bmh_undocumented?cl=9919#Point">Point</a>; for instance, <a href="bmh_SkPath?cl=9919#Path">Path</a>
containing two connected <a href="bmh_undocumented?cl=9919#Line">Lines</a> are described the <a href="bmh_SkPath?cl=9919#Verb">Verb</a> sequence: 
move to, line to, line to; and a <a href="bmh_undocumented?cl=9919#Point">Point</a> sequence with three entries, sharing
the middle entry as the end of the first <a href="bmh_undocumented?cl=9919#Line">Line</a> and the start of the second <a href="bmh_undocumented?cl=9919#Line">Line</a>.

<a href="bmh_SkPath?cl=9919#Path">Path</a> components <a href="bmh_SkPath?cl=9919#Arc">Arc</a>, <a href="bmh_undocumented?cl=9919#Rect">Rect</a>, <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a>, <a href="bmh_undocumented?cl=9919#Circle">Circle</a>, and <a href="bmh_undocumented?cl=9919#Oval">Oval</a> are composed of
<a href="bmh_undocumented?cl=9919#Line">Lines</a> and <a href="bmh_undocumented?cl=9919#Curve">Curves</a> with as many <a href="bmh_SkPath?cl=9919#Verb">Verbs</a> and <a href="bmh_undocumented?cl=9919#Point">Points</a> required
for an exact description. Once added to <a href="bmh_SkPath?cl=9919#Path">Path</a>, these components may lose their
identity; although <a href="bmh_SkPath?cl=9919#Path">Path</a> can be inspected to determine if it decribes a single
<a href="bmh_undocumented?cl=9919#Rect">Rect</a>, <a href="bmh_undocumented?cl=9919#Oval">Oval</a>, <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a>, and so on.

### Example

<div><fiddle-embed name=""></fiddle-embed></div>

<a href="bmh_SkPath?cl=9919#Path">Path</a> contains a <a href="bmh_SkPath?cl=9919#Fill_Type">Fill Type</a> which determines whether overlapping <a href="bmh_SkPath?cl=9919#Contour">Contours</a>
form fills or holes. <a href="bmh_SkPath?cl=9919#Fill_Type">Fill Type</a> also determines whether area inside or outside
<a href="bmh_undocumented?cl=9919#Line">Lines</a> and <a href="bmh_undocumented?cl=9919#Curve">Curves</a> is filled.

### Example

<div><fiddle-embed name=""></fiddle-embed></div>

<a href="bmh_SkPath?cl=9919#Path">Path</a> contents are never shared. Copying <a href="bmh_SkPath?cl=9919#Path">Path</a> by value effectively creates
a new <a href="bmh_SkPath?cl=9919#Path">Path</a> independent of the original. Internally, the copy does not duplicate
its contents until it is edited, to reduce memory use and improve performance.

## <a name="Subtopics"></a> Subtopics

| topics | description |
| --- | ---  |


## <a name="Contour"></a> Contour

<a href="bmh_SkPath?cl=9919#Contour">Contour</a> contains one or more <a href="bmh_SkPath?cl=9919#Verb">Verbs</a>, and as many <a href="bmh_undocumented?cl=9919#Point">Points</a> as
are required to satisfy <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a>. First <a href="bmh_SkPath?cl=9919#Verb">Verb</a> in <a href="bmh_SkPath?cl=9919#Path">Path</a> is always
<a href="bmh_SkPath?cl=9919#kMove_Verb">SkPath::kMove Verb</a>; each <a href="bmh_SkPath?cl=9919#kMove_Verb">SkPath::kMove Verb</a> that follows starts new <a href="bmh_SkPath?cl=9919#Contour">Contour</a>.

### Example

<div><fiddle-embed name=""></fiddle-embed></div>

If final <a href="bmh_SkPath?cl=9919#Verb">Verb</a> in <a href="bmh_SkPath?cl=9919#Contour">Contour</a> is close, <a href="bmh_undocumented?cl=9919#Line">Line</a> connects <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> in
<a href="bmh_SkPath?cl=9919#Contour">Contour</a> with first <a href="bmh_undocumented?cl=9919#Point">Point</a>. A closed <a href="bmh_SkPath?cl=9919#Contour">Contour</a>, stroked, draws 
<a href="bmh_SkPaint?cl=9919#Stroke_Join">Paint Stroke Join</a> at <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> and first <a href="bmh_undocumented?cl=9919#Point">Point</a>. Without close
as final <a href="bmh_SkPath?cl=9919#Verb">Verb</a>, <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> and first <a href="bmh_undocumented?cl=9919#Point">Point</a> are not connected; <a href="bmh_SkPath?cl=9919#Contour">Contour</a>
remains open. An open <a href="bmh_SkPath?cl=9919#Contour">Contour</a>, stroked, draws <a href="bmh_SkPaint?cl=9919#Stroke_Cap">Paint Stroke Cap</a> at 
<a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> and first <a href="bmh_undocumented?cl=9919#Point">Point</a>.

### Example

<div><fiddle-embed name=""></fiddle-embed></div>

## <a name="Zero_Length"></a> Zero Length

<a href="bmh_SkPath?cl=9919#Contour">Contour</a> length is distance traveled from first <a href="bmh_undocumented?cl=9919#Point">Point</a> to <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a>,
plus, if <a href="bmh_SkPath?cl=9919#Contour">Contour</a> is closed, distance from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> to first <a href="bmh_undocumented?cl=9919#Point">Point</a>.
Even if <a href="bmh_SkPath?cl=9919#Contour">Contour</a> length is zero, stroked <a href="bmh_undocumented?cl=9919#Line">Lines</a> are drawn if <a href="bmh_SkPaint?cl=9919#Stroke_Cap">Paint Stroke Cap</a>
makes them visible.

### Example

<div><fiddle-embed name=""></fiddle-embed></div>

## <a name="Weight"></a> Weight

Each <a href="bmh_SkPath?cl=9919#Conic">Conic</a> in <a href="bmh_SkPath?cl=9919#Path">Path</a> adds two <a href="bmh_undocumented?cl=9919#Point">Points</a> and one <a href="bmh_SkPath?cl=9919#Weight">Weight</a>. <a href="bmh_SkPath?cl=9919#Weight">Weight</a> controls the influence
of <a href="bmh_SkPath?cl=9919#Conic">Conic</a> control <a href="bmh_undocumented?cl=9919#Point">Point</a>. <a href="bmh_SkPath?cl=9919#Weight">Weights</a> in <a href="bmh_SkPath?cl=9919#Path">Path</a> may be inspected with Iter, or with
RawIter.

# <a name="SkPath"></a> Class SkPath

# <a name="Overview"></a> Overview

## <a name="Constants"></a> Constants

| constants | description |
| --- | ---  |
incomplete

## <a name="Classes_and_Structs"></a> Classes and Structs

| class or struct | description |
| --- | ---  |
| <a href="bmh_SkPath?cl=9919#Iter">Iter</a> | Iterates through lines and curves, skipping degenerates. |
| <a href="bmh_SkPath?cl=9919#RawIter">RawIter</a> | Iterates through lines and curves, including degenerates. |


## <a name="Constructors"></a> Constructors

|  | description |
| --- | ---  |
| <a href="bmh_SkPath?cl=9919#empty_constructor">SkPath()</a> | Constructs with default values. |
| <a href="bmh_SkPath?cl=9919#copy_constructor">SkPath(const SkPath& path)</a> | Makes a shallow copy. |
|  | Decreases <a href="bmh_undocumented?cl=9919#Reference_Count">Reference Count</a> of owned objects. |


## <a name="Operators"></a> Operators

| operator | description |
| --- | ---  |
| <a href="bmh_SkPath?cl=9919#copy_assignment_operator">operator=(const SkPath& path)</a> | Makes a shallow copy. |
| <a href="bmh_SkPath?cl=9919#equal_operator">operator==(const SkPath& a, const SkPath& b)</a> | Compares paths for equality. |
| <a href="bmh_SkPath?cl=9919#not_equal_operator">operator!=(const SkPath& a, const SkPath& b)</a> | Compares paths for inequality. |


## <a name="Member_Functions"></a> Member Functions

| function | description |
| --- | ---  |
| <a href="bmh_SkPath?cl=9919#ConvertConicToQuads">ConvertConicToQuads</a> | Approximates <a href="bmh_SkPath?cl=9919#Conic">Conic</a> with <a href="bmh_SkPath?cl=9919#Quad">Quad</a> array. |
| <a href="bmh_SkPath?cl=9919#ConvertToNonInverseFillType">ConvertToNonInverseFillType</a> | Returns <a href="bmh_SkPath?cl=9919#Fill_Type">Fill Type</a> representing inside geometry. |
| <a href="bmh_SkPath?cl=9919#IsCubicDegenerate">IsCubicDegenerate</a> | Returns if <a href="bmh_SkPath?cl=9919#Cubic">Cubic</a> is very small. |
| <a href="bmh_SkPath?cl=9919#IsInverseFillType">IsInverseFillType</a> | Returns if <a href="bmh_SkPath?cl=9919#Fill_Type">Fill Type</a> represents outside geometry. |
| <a href="bmh_SkPath?cl=9919#IsLineDegenerate">IsLineDegenerate</a> | Returns if <a href="bmh_undocumented?cl=9919#Line">Line</a> is very small. |
| <a href="bmh_SkPath?cl=9919#IsQuadDegenerate">IsQuadDegenerate</a> | Returns if <a href="bmh_SkPath?cl=9919#Quad">Quad</a> is very small. |
| <a href="bmh_SkPath?cl=9919#addArc">addArc</a> | Adds one <a href="bmh_SkPath?cl=9919#Contour">Contour</a> containing <a href="bmh_SkPath?cl=9919#Arc">Arc</a>. |
| <a href="bmh_SkPath?cl=9919#addCircle">addCircle</a> | Adds one <a href="bmh_SkPath?cl=9919#Contour">Contour</a> containing <a href="bmh_undocumented?cl=9919#Circle">Circle</a>. |
| <a href="bmh_SkPath?cl=9919#addOval">addOval</a> | Adds one <a href="bmh_SkPath?cl=9919#Contour">Contour</a> containing <a href="bmh_undocumented?cl=9919#Oval">Oval</a>. |
| <a href="bmh_SkPath?cl=9919#addPath">addPath</a> | Adds contents of <a href="bmh_SkPath?cl=9919#Path">Path</a>. |
| <a href="bmh_SkPath?cl=9919#addPoly">addPoly</a> | Adds one <a href="bmh_SkPath?cl=9919#Contour">Contour</a> containing connected lines. |
| <a href="bmh_SkPath?cl=9919#addRRect">addRRect</a> | Adds one <a href="bmh_SkPath?cl=9919#Contour">Contour</a> containing <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a>. |
| <a href="bmh_SkPath?cl=9919#addRect">addRect</a> | Adds one <a href="bmh_SkPath?cl=9919#Contour">Contour</a> containing <a href="bmh_undocumented?cl=9919#Rect">Rect</a>. |
| <a href="bmh_SkPath?cl=9919#addRoundRect">addRoundRect</a> | Adds one <a href="bmh_SkPath?cl=9919#Contour">Contour</a> containing <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a> with common corner radii. |
| <a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> | Appends <a href="bmh_SkPath?cl=9919#Arc">Arc</a>. |
| close | Makes last <a href="bmh_SkPath?cl=9919#Contour">Contour</a> a loop. |
| <a href="bmh_SkPath?cl=9919#computeTightBounds">computeTightBounds</a> | Returns extent of geometry. |
| <a href="bmh_SkPath?cl=9919#conicTo">conicTo</a> | Appends <a href="bmh_SkPath?cl=9919#Conic">Conic</a>. |
| <a href="bmh_SkPath?cl=9919#conservativelyContainsRect">conservativelyContainsRect</a> | Returns true if <a href="bmh_undocumented?cl=9919#Rect">Rect</a> may be inside. |
| contains | Returns if <a href="bmh_undocumented?cl=9919#Point">Point</a> is in fill area. |
| <a href="bmh_SkPath?cl=9919#countPoints">countPoints</a> | Returns <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a> length. |
| <a href="bmh_SkPath?cl=9919#countVerbs">countVerbs</a> | Returns <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a> length. |
| <a href="bmh_SkPath?cl=9919#cubicTo">cubicTo</a> | Appends <a href="bmh_SkPath?cl=9919#Cubic">Cubic</a>. |
| dump | Sends text representation using floats to stdout. |
| <a href="bmh_SkPath?cl=9919#dumpHex">dumpHex</a> | Sends text representation using hexadecimal to stdout. |
| <a href="bmh_SkPath?cl=9919#experimentalValidateRef">experimentalValidateRef</a> | Experimental; debugging only. |
| <a href="bmh_SkPath?cl=9919#getBounds">getBounds</a> | Returns maximum and minimum of <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>. |
| <a href="bmh_SkPath?cl=9919#getConvexity">getConvexity</a> | Returns geometry convexity, computing if necessary. |
| <a href="bmh_SkPath?cl=9919#getConvexityOrUnknown">getConvexityOrUnknown</a> | Returns geometry convexity if known. |
| <a href="bmh_SkPath?cl=9919#getFillType">getFillType</a> | Returns <a href="bmh_SkPath?cl=9919#Fill_Type">Fill Type</a>: winding, even-odd, inverse. |
| <a href="bmh_SkPath?cl=9919#getGenerationID">getGenerationID</a> | Returns unique ID. |
| <a href="bmh_SkPath?cl=9919#getLastPt">getLastPt</a> | Returns <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a>. |
| <a href="bmh_SkPath?cl=9919#getPoint">getPoint</a> | Returns entry from <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>. |
| <a href="bmh_SkPath?cl=9919#getPoints">getPoints</a> | Returns <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>. |
| <a href="bmh_SkPath?cl=9919#getSegmentMasks">getSegmentMasks</a> | Returns types in <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a>. |
| <a href="bmh_SkPath?cl=9919#getVerbs">getVerbs</a> | Returns <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a>. |
| <a href="bmh_SkPath?cl=9919#incReserve">incReserve</a> | Hint to reserve space for additional data. |
| interpolate | Interpolates between <a href="bmh_SkPath?cl=9919#Path">Path</a> pair. |
| <a href="bmh_SkPath?cl=9919#isConvex">isConvex</a> | Returns if geometry is convex. |
| <a href="bmh_SkPath?cl=9919#isEmpty">isEmpty</a> | Returns if verb count is zero. |
| <a href="bmh_SkPath?cl=9919#isFinite">isFinite</a> | Returns if all <a href="bmh_undocumented?cl=9919#Point">Point</a> values are finite. |
| <a href="bmh_SkPath?cl=9919#isInterpolatable">isInterpolatable</a> | Returns if pair contains equal counts of <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a> and <a href="bmh_SkPath?cl=9919#Weight">Weights</a>. |
| <a href="bmh_SkPath?cl=9919#isInverseFillType">isInverseFillType</a> | Returns if <a href="bmh_SkPath?cl=9919#Fill_Type">Fill Type</a> fills outside geometry. |
| <a href="bmh_SkPath?cl=9919#isLastContourClosed">isLastContourClosed</a> | Returns if final <a href="bmh_SkPath?cl=9919#Contour">Contour</a> forms a loop. |
| <a href="bmh_SkPath?cl=9919#isLine">isLine</a> | Returns if describes <a href="bmh_undocumented?cl=9919#Line">Line</a>. |
| <a href="bmh_SkPath?cl=9919#isNestedFillRects">isNestedFillRects</a> | Returns if describes <a href="bmh_undocumented?cl=9919#Rect">Rect</a> pair, one inside the other. |
| <a href="bmh_SkPath?cl=9919#isOval">isOval</a> | Returns if describes <a href="bmh_undocumented?cl=9919#Oval">Oval</a>. |
| <a href="bmh_SkPath?cl=9919#isRRect">isRRect</a> | Returns if describes <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a>. |
| <a href="bmh_SkPath?cl=9919#isRect">isRect</a> | Returns if describes <a href="bmh_undocumented?cl=9919#Rect">Rect</a>. |
| <a href="bmh_SkPath?cl=9919#isVolatile">isVolatile</a> | Returns if <a href="bmh_undocumented?cl=9919#Device">Device</a> should not cache. |
| <a href="bmh_SkPath?cl=9919#lineTo">lineTo</a> | Appends <a href="bmh_undocumented?cl=9919#Line">Line</a>. |
| <a href="bmh_SkPath?cl=9919#moveTo">moveTo</a> | Starts <a href="bmh_SkPath?cl=9919#Contour">Contour</a>. |
| offset | Translates <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>. |
| <a href="bmh_SkPath?cl=9919#quadTo">quadTo</a> | Appends <a href="bmh_SkPath?cl=9919#Quad">Quad</a>. |
| <a href="bmh_SkPath?cl=9919#rArcTo">rArcTo</a> | Appends <a href="bmh_SkPath?cl=9919#Arc">Arc</a> relative to <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a>. |
| <a href="bmh_SkPath?cl=9919#rConicTo">rConicTo</a> | Appends <a href="bmh_SkPath?cl=9919#Conic">Conic</a> relative to <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a>. |
| <a href="bmh_SkPath?cl=9919#rCubicTo">rCubicTo</a> | Appends <a href="bmh_SkPath?cl=9919#Cubic">Cubic</a> relative to <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a>. |
| <a href="bmh_SkPath?cl=9919#rLineTo">rLineTo</a> | Appends <a href="bmh_undocumented?cl=9919#Line">Line</a> relative to <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a>. |
| <a href="bmh_SkPath?cl=9919#rMoveTo">rMoveTo</a> | Starts <a href="bmh_SkPath?cl=9919#Contour">Contour</a> relative to <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a>. |
| <a href="bmh_SkPath?cl=9919#rQuadTo">rQuadTo</a> | Appends <a href="bmh_SkPath?cl=9919#Quad">Quad</a> relative to <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a>. |
| <a href="bmh_SkPath?cl=9919#readFromMemory">readFromMemory</a> | Initialize from buffer. |
| reset | Removes <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a>, <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>, and <a href="bmh_SkPath?cl=9919#Weight">Weights</a>; frees memory. |
| <a href="bmh_SkPath?cl=9919#reverseAddPath">reverseAddPath</a> | Adds contents of <a href="bmh_SkPath?cl=9919#Path">Path</a> back to front. |
| rewind | Removes <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a>, <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>, and <a href="bmh_SkPath?cl=9919#Weight">Weights</a>; leaves memory allocated. |
| <a href="bmh_SkPath?cl=9919#setConvexity">setConvexity</a> | Sets if geometry is convex to avoid future computation. |
| <a href="bmh_SkPath?cl=9919#setFillType">setFillType</a> | Sets <a href="bmh_SkPath?cl=9919#Fill_Type">Fill Type</a>: winding, even-odd, inverse. |
| <a href="bmh_SkPath?cl=9919#setIsConvex">setIsConvex</a> | Deprecated. |
| <a href="bmh_SkPath?cl=9919#setIsVolatile">setIsVolatile</a> | Sets if <a href="bmh_undocumented?cl=9919#Device">Device</a> should not cache. |
| <a href="bmh_SkPath?cl=9919#setLastPt">setLastPt</a> | Replaces <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a>. |
| swap | Exchanges <a href="bmh_SkPath?cl=9919#Path">Path</a> pair. |
| <a href="bmh_SkPath?cl=9919#toggleInverseFillType">toggleInverseFillType</a> | Toggles <a href="bmh_SkPath?cl=9919#Fill_Type">Fill Type</a> between inside and outside geometry. |
| transform | Applies <a href="bmh_undocumented?cl=9919#Matrix">Matrix</a> to <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a> and <a href="bmh_SkPath?cl=9919#Weight">Weights</a>. |
| unique | Returns if data has single owner. |
| <a href="bmh_SkPath?cl=9919#updateBoundsCache">updateBoundsCache</a> | Refresh result of <a href="bmh_SkPath?cl=9919#getBounds">getBounds</a>. |
| <a href="bmh_SkPath?cl=9919#writeToMemory">writeToMemory</a> | Copy data to buffer. |


## <a name="Verb"></a> Verb

<a href="bmh_SkPath?cl=9919#Verb">Verb</a> instructs <a href="bmh_SkPath?cl=9919#Path">Path</a> how to interpret one or more <a href="bmh_undocumented?cl=9919#Point">Point</a> and optional <a href="bmh_SkPath?cl=9919#Weight">Weight</a>;
manage <a href="bmh_SkPath?cl=9919#Contour">Contour</a>, and terminate <a href="bmh_SkPath?cl=9919#Path">Path</a>.

## <a name="SkPath::Verb"></a> Enum SkPath::Verb

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="bmh_SkPath?cl=9919#Verb">Verb</a> {
    <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a> 
    <a href="bmh_SkPath?cl=9919#kLine_Verb">kLine Verb</a> 
    <a href="bmh_SkPath?cl=9919#kQuad_Verb">kQuad Verb</a> 
    <a href="bmh_SkPath?cl=9919#kConic_Verb">kConic Verb</a> 
    <a href="bmh_SkPath?cl=9919#kCubic_Verb">kCubic Verb</a> 
    <a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a> 
    <a href="bmh_SkPath?cl=9919#kDone_Verb">kDone Verb</a> 
};</pre>

### Constants

<table>
  <tr>
    <td><a name="SkPath::kMove_Verb"></a> <code><strong>SkPath::kMove_Verb </strong></code></td><td>0</td><td>Starts new <a href="bmh_SkPath?cl=9919#Contour">Contour</a> at next <a href="bmh_undocumented?cl=9919#Point">Point</a>.</td>
  </tr>
  <tr>
    <td><a name="SkPath::kLine_Verb"></a> <code><strong>SkPath::kLine_Verb </strong></code></td><td>1</td><td>Adds <a href="bmh_undocumented?cl=9919#Line">Line</a> from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> to next <a href="bmh_undocumented?cl=9919#Point">Point</a>.
<a href="bmh_undocumented?cl=9919#Line">Line</a> is a straight segment from <a href="bmh_undocumented?cl=9919#Point">Point</a> to <a href="bmh_undocumented?cl=9919#Point">Point</a>.</td>
  </tr>
  <tr>
    <td><a name="SkPath::kQuad_Verb"></a> <code><strong>SkPath::kQuad_Verb </strong></code></td><td>2</td><td>Adds <a href="bmh_SkPath?cl=9919#Quad">Quad</a> from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a>, using control <a href="bmh_undocumented?cl=9919#Point">Point</a>, and end <a href="bmh_undocumented?cl=9919#Point">Point</a>. 
<a href="bmh_SkPath?cl=9919#Quad">Quad</a> is a parabolic section within tangents from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> to control <a href="bmh_undocumented?cl=9919#Point">Point</a>,
and control <a href="bmh_undocumented?cl=9919#Point">Point</a> to end <a href="bmh_undocumented?cl=9919#Point">Point</a>.</td>
  </tr>
  <tr>
    <td><a name="SkPath::kConic_Verb"></a> <code><strong>SkPath::kConic_Verb </strong></code></td><td>3</td><td>Adds <a href="bmh_SkPath?cl=9919#Conic">Conic</a> from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a>, using control <a href="bmh_undocumented?cl=9919#Point">Point</a>, end <a href="bmh_undocumented?cl=9919#Point">Point</a>, and <a href="bmh_SkPath?cl=9919#Weight">Weight</a>.
<a href="bmh_SkPath?cl=9919#Conic">Conic</a> is a elliptical, parabolic, or hyperbolic section within tangents 
from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> to control <a href="bmh_undocumented?cl=9919#Point">Point</a>, and control <a href="bmh_undocumented?cl=9919#Point">Point</a> to end <a href="bmh_undocumented?cl=9919#Point">Point</a>, constrained
by <a href="bmh_SkPath?cl=9919#Weight">Weight</a>. <a href="bmh_SkPath?cl=9919#Weight">Weight</a> less than one is elliptical; equal to one is parabolic
(and identical to <a href="bmh_SkPath?cl=9919#Quad">Quad</a>); greater than one hyperbolic.</td>
  </tr>
  <tr>
    <td><a name="SkPath::kCubic_Verb"></a> <code><strong>SkPath::kCubic_Verb </strong></code></td><td>4</td><td>Adds <a href="bmh_SkPath?cl=9919#Cubic">Cubic</a> from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a>, using two control <a href="bmh_undocumented?cl=9919#Point">Points</a>, and end <a href="bmh_undocumented?cl=9919#Point">Point</a>. 
<a href="bmh_SkPath?cl=9919#Cubic">Cubic</a> is a third-order <a href="bmh_undocumented?cl=9919#Bezier">Bezier</a> section within tangents from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> to
first control <a href="bmh_undocumented?cl=9919#Point">Point</a>, and from second control <a href="bmh_undocumented?cl=9919#Point">Point</a> to end <a href="bmh_undocumented?cl=9919#Point">Point</a>.</td>
  </tr>
  <tr>
    <td><a name="SkPath::kClose_Verb"></a> <code><strong>SkPath::kClose_Verb </strong></code></td><td>5</td><td>Closes <a href="bmh_SkPath?cl=9919#Contour">Contour</a>, connecting <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> to <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a> <a href="bmh_undocumented?cl=9919#Point">Point</a>.</td>
  </tr>
  <tr>
    <td><a name="SkPath::kDone_Verb"></a> <code><strong>SkPath::kDone_Verb </strong></code></td><td>6</td><td>Terminates <a href="bmh_SkPath?cl=9919#Path">Path</a>.</td>
  </tr>
Each <a href="bmh_SkPath?cl=9919#Verb">Verb</a> has zero or more <a href="bmh_undocumented?cl=9919#Point">Points</a> stored in <a href="bmh_SkPath?cl=9919#Path">Path</a>.
<a href="bmh_SkPath?cl=9919#Path">Path</a> iterator returns complete curve descriptions, duplicating shared <a href="bmh_undocumented?cl=9919#Point">Points</a>
for consecutive entries.

</table>

| <a href="bmh_SkPath?cl=9919#Verb">Verb</a> | Allocated <a href="bmh_undocumented?cl=9919#Point">Points</a> | Iterated <a href="bmh_undocumented?cl=9919#Point">Points</a> | <a href="bmh_SkPath?cl=9919#Weight">Weights</a> |
| --- | --- | --- | ---  |
| <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a> | 1 | 1 | 0 |
| <a href="bmh_SkPath?cl=9919#kLine_Verb">kLine Verb</a> | 1 | 2 | 0 |
| <a href="bmh_SkPath?cl=9919#kQuad_Verb">kQuad Verb</a> | 2 | 3 | 0 |
| <a href="bmh_SkPath?cl=9919#kConic_Verb">kConic Verb</a> | 2 | 3 | 1 |
| <a href="bmh_SkPath?cl=9919#kCubic_Verb">kCubic Verb</a> | 3 | 4 | 0 |
| <a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a> | 0 | 0 | 0 |
| <a href="bmh_SkPath?cl=9919#kDone_Verb">kDone Verb</a> | -- | 0 | 0 |


### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>



## <a name="Direction"></a> Direction

## <a name="SkPath::Direction"></a> Enum SkPath::Direction

<a href="bmh_SkPath?cl=9919#Direction">Direction</a> describes whether <a href="bmh_SkPath?cl=9919#Contour">Contour</a> is clockwise or counterclockwise.
When <a href="bmh_SkPath?cl=9919#Path">Path</a> contains multiple overlapping <a href="bmh_SkPath?cl=9919#Contour">Contours</a>, <a href="bmh_SkPath?cl=9919#Direction">Direction</a> together with
<a href="bmh_SkPath?cl=9919#Fill_Type">Fill Type</a> determines whether overlaps are filled or form holes.

<a href="bmh_SkPath?cl=9919#Direction">Direction</a> also determines how <a href="bmh_SkPath?cl=9919#Contour">Contour</a> is measured. For instance, dashing
measures along <a href="bmh_SkPath?cl=9919#Path">Path</a> to determine where to start and stop stroke; <a href="bmh_SkPath?cl=9919#Direction">Direction</a>
will change dashed results as it steps clockwise or counterclockwise.

Closed <a href="bmh_SkPath?cl=9919#Contour">Contours</a> like <a href="bmh_undocumented?cl=9919#Rect">Rect</a>, <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a>, <a href="bmh_undocumented?cl=9919#Circle">Circle</a>, and <a href="bmh_undocumented?cl=9919#Oval">Oval</a> added with 
<a href="bmh_SkPath?cl=9919#kCW_Direction">kCW Direction</a> travel clockwise; the same added with <a href="bmh_SkPath?cl=9919#kCCW_Direction">kCCW Direction</a>
travel counterclockwise.

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="bmh_SkPath?cl=9919#Direction">Direction</a> {
    <a href="bmh_SkPath?cl=9919#kCW_Direction">kCW Direction</a> 
    <a href="bmh_SkPath?cl=9919#kCCW_Direction">kCCW Direction</a> 
};</pre>

### Constants

<table>
  <tr>
    <td><a name="SkPath::kCW_Direction"></a> <code><strong>SkPath::kCW_Direction </strong></code></td><td>Contour travels in a clockwise direction. </td><td></td>
  </tr>
  <tr>
    <td><a name="SkPath::kCCW_Direction"></a> <code><strong>SkPath::kCCW_Direction </strong></code></td><td>Contour travels in a counterclockwise direction. </td><td></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d2248ee98778f27e0273f52bdf4e75c8"></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> <a href="bmh_SkPath?cl=9919#rArcTo">rArcTo</a> <a href="bmh_SkPath?cl=9919#isRect">isRect</a> <a href="bmh_SkPath?cl=9919#isNestedFillRects">isNestedFillRects</a> <a href="bmh_SkPath?cl=9919#addRect">addRect</a> <a href="bmh_SkPath?cl=9919#addOval">addOval</a>



<a name="empty_constructor"></a>

## SkPath

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkPath()
</pre>

### Return Value

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<a name="copy_constructor"></a>

## SkPath

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkPath(const SkPath& path)
</pre>

### Parameters

<table>
  <tr>
    <td><code><strong>path </strong></code></td> <td></td>
  </tr>
</table>

### Return Value

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<a name="destructor"></a>

## ~SkPath

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
~SkPath()
</pre>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<a name="copy_assignment_operator"></a>

## operator=

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkPath& operator=(const SkPath& path)
</pre>

<a href="bmh_SkPath?cl=9919#Path">Path</a> assignment makes two paths identical by value. Internally, assignment
shares pointer values. The underlying <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a>, <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a> and <a href="bmh_SkPath?cl=9919#Weight">Weights</a>
are copied when modified.

Copying <a href="bmh_SkPath?cl=9919#Path">Paths</a> by assignment is very efficient and never allocates memory.
<a href="bmh_SkPath?cl=9919#Path">Paths</a> are always copied by value from the interface; the underlying shared
pointers are not exposed.

### Parameters

<table>
  <tr>
    <td><code><strong>path </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a>, <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>, <a href="bmh_SkPath?cl=9919#Weight">Weights</a>, amd <a href="bmh_SkPath?cl=9919#Fill_Type">Fill Type</a> to copy.</td>
  </tr>
</table>

### Return Value

<a href="bmh_SkPath?cl=9919#Path">Path</a> copied by value.

### Example

<div><fiddle-embed name="bba288f5f77fc8e37e89d2ec08e0ac60">#### Example Output

~~~~
path1 bounds = 10, 20, 30, 40
path2 bounds = 10, 20, 30, 40
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#swap">swap</a>

---

<a name="equal_operator"></a>

## operator==

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
friend SK_API bool operator==(const SkPath& a, const SkPath& b)
</pre>

Compares <a href="bmh_SkPath?cl=9919#a">a</a> and <a href="bmh_SkPath?cl=9919#b">b</a>; returns true if <a href="bmh_SkPath?cl=9919#Fill_Type">Fill Type</a>, <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a>, <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>, and <a href="bmh_SkPath?cl=9919#Weight">Weights</a>
are equivalent.

### Parameters

<table>
  <tr>
    <td><code><strong>a </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Path">Path</a> to compare.</td>
  </tr>
  <tr>
    <td><code><strong>b </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Path">Path</a> to compare.</td>
  </tr>
</table>

### Return Value

true if <a href="bmh_SkPath?cl=9919#Path">Path</a> pair are equivalent.

### Example

<div><fiddle-embed name="31883f51bb357f2ac5990d88f8b82e02"><p>Rewind removes <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a> but leaves storage; since storage is not compared,
<a href="bmh_SkPath?cl=9919#Path">Path</a> pair are equivalent.</p>

#### Example Output

~~~~
empty one == two
moveTo one != two
rewind one == two
reset one == two
~~~~

</fiddle-embed></div>

---

<a name="not_equal_operator"></a>

## operator!=

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
friend bool operator!=(const SkPath& a, const SkPath& b)
</pre>

Compares <a href="bmh_SkPath?cl=9919#a">a</a> and <a href="bmh_SkPath?cl=9919#b">b</a>; returns true if <a href="bmh_SkPath?cl=9919#Fill_Type">Fill Type</a>, <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a>, <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>, and <a href="bmh_SkPath?cl=9919#Weight">Weights</a>
are not equivalent.

### Parameters

<table>
  <tr>
    <td><code><strong>a </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Path">Path</a> to compare.</td>
  </tr>
  <tr>
    <td><code><strong>b </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Path">Path</a> to compare.</td>
  </tr>
</table>

### Return Value

true if <a href="bmh_SkPath?cl=9919#Path">Path</a> pair are not equivalent.

### Example

<div><fiddle-embed name="0c6870ba1cea85ce6da5abd489c23d83"><p><a href="bmh_SkPath?cl=9919#Path">Path</a> pair are equal though their convexity is not equal.</p>

#### Example Output

~~~~
empty one == two
addRect one == two
setConvexity one == two
convexity !=
~~~~

</fiddle-embed></div>

---

<a name="isInterpolatable"></a>

## isInterpolatable

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isInterpolatable(const SkPath& compare) const
</pre>

Return true if the paths contain an equal array of verbs and weights. <a href="bmh_SkPath?cl=9919#Path">Paths</a>
with equal verb counts can be readily interpolated. If the paths contain one
or more conics, the conics' weights must also match.

### Parameters

<table>
  <tr>
    <td><code><strong>compare </strong></code></td> <td>The path to <a href="bmh_SkPath?cl=9919#compare">compare</a>.</td>
  </tr>
</table>

### Return Value

true if the paths have the same verbs and weights.

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<a name="interpolate"></a>

## interpolate

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool interpolate(const SkPath& ending, SkScalar weight, SkPath* out) const
</pre>

Interpolate between two paths with same-sized point arrays.
The <a href="bmh_SkPath?cl=9919#out">out</a> path contains the verbs and weights of this path.
The <a href="bmh_SkPath?cl=9919#out">out</a> points are a weighted average of this path and the <a href="bmh_SkPath?cl=9919#ending">ending</a> path.

### Parameters

<table>
  <tr>
    <td><code><strong>ending </strong></code></td> <td>The path to interpolate between.</td>
  </tr>
  <tr>
    <td><code><strong>weight </strong></code></td> <td>The <a href="bmh_SkPath?cl=9919#interpolate">weight</a>, from 0 to 1. The output points are set to</td>
  </tr>
  <tr>
    <td><code><strong>out </strong></code></td> <td></td>
  </tr>
</table>

### Return Value

true if the paths could be interpolated.

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<a name="unique"></a>

## unique

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool unique() const
</pre>

Returns true if the caller is the only owner of the underlying path data *

### Return Value

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

## <a name="Fill_Type"></a> Fill Type

<a href="bmh_SkPath?cl=9919#Fill_Type">Fill Type</a> selects the rule used to fill <a href="bmh_SkPath?cl=9919#Path">Path</a>. <a href="bmh_SkPath?cl=9919#Path">Path</a> set to <a href="bmh_SkPath?cl=9919#kWinding_FillType">kWinding FillType</a> 
fills if the sum of <a href="bmh_SkPath?cl=9919#Contour">Contour</a> edges is not zero, where clockwise edges add one, and
counterclockwise edges subtract one. <a href="bmh_SkPath?cl=9919#Path">Path</a> set to <a href="bmh_SkPath?cl=9919#kEvenOdd_FillType">kEvenOdd FillType</a> fills if the
number of <a href="bmh_SkPath?cl=9919#Contour">Contour</a> edges is odd. Each <a href="bmh_SkPath?cl=9919#Fill_Type">Fill Type</a> has an inverse variant that 
reverses the rule:
<a href="bmh_SkPath?cl=9919#kInverseWinding_FillType">kInverseWinding FillType</a> fills where the sum of <a href="bmh_SkPath?cl=9919#Contour">Contour</a> edges is zero;
<a href="bmh_SkPath?cl=9919#kInverseEvenOdd_FillType">kInverseEvenOdd FillType</a> fills where the number of <a href="bmh_SkPath?cl=9919#Contour">Contour</a> edges is even.

### Example

<div><fiddle-embed name="525ed591c31960de23068dba8ea11a75"><p>The top row has two clockwise rectangles. The second row has one clockwise and
one counterclockwise rectangle. The even-odd variants draw the same. The
winding variants draw the top rectangle overlap, which has a winding of 2, the
same as the outer parts of the top rectangles, which have a winding of 1.</p></fiddle-embed></div>

## <a name="SkPath::FillType"></a> Enum SkPath::FillType

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="bmh_SkPath?cl=9919#FillType">FillType</a> {
    <a href="bmh_SkPath?cl=9919#kWinding_FillType">kWinding FillType</a> 
    <a href="bmh_SkPath?cl=9919#kEvenOdd_FillType">kEvenOdd FillType</a> 
    <a href="bmh_SkPath?cl=9919#kInverseWinding_FillType">kInverseWinding FillType</a> 
    <a href="bmh_SkPath?cl=9919#kInverseEvenOdd_FillType">kInverseEvenOdd FillType</a> 
};</pre>

### Constants

<table>
  <tr>
    <td><a name="SkPath::kWinding_FillType"></a> <code><strong>SkPath::kWinding_FillType </strong></code></td><td>Specifies fill as area is enclosed by a non-zero sum of Contour Directions.</td><td></td>
  </tr>
  <tr>
    <td><a name="SkPath::kEvenOdd_FillType"></a> <code><strong>SkPath::kEvenOdd_FillType </strong></code></td><td>Specifies fill as area enclosed by an odd number of Contours.</td><td></td>
  </tr>
  <tr>
    <td><a name="SkPath::kInverseWinding_FillType"></a> <code><strong>SkPath::kInverseWinding_FillType </strong></code></td><td>Specifies fill as area is enclosed by a zero sum of Contour Directions.</td><td></td>
  </tr>
  <tr>
    <td><a name="SkPath::kInverseEvenOdd_FillType"></a> <code><strong>SkPath::kInverseEvenOdd_FillType </strong></code></td><td>Specifies fill as area enclosed by an even number of Contours.</td><td></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6c58ded16b8eabd4c7d3d64cd2451652"></fiddle-embed></div>

### See Also

<a href="bmh_SkPaint?cl=9919#Style">SkPaint::Style</a> <a href="bmh_SkPath?cl=9919#Direction">Direction</a> <a href="bmh_SkPath?cl=9919#getFillType">getFillType</a> <a href="bmh_SkPath?cl=9919#setFillType">setFillType</a>



<a name="getFillType"></a>

## getFillType

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
FillType getFillType() const
</pre>

Returns <a href="bmh_SkPath?cl=9919#FillType">FillType</a>, the rule used to fill <a href="bmh_SkPath?cl=9919#Path">Path</a>. <a href="bmh_SkPath?cl=9919#FillType">FillType</a> of a new <a href="bmh_SkPath?cl=9919#Path">Path</a> is
<a href="bmh_SkPath?cl=9919#kWinding_FillType">kWinding FillType</a>.

### Return Value

one of: <a href="bmh_SkPath?cl=9919#kWinding_FillType">kWinding FillType</a>, <a href="bmh_SkPath?cl=9919#kEvenOdd_FillType">kEvenOdd FillType</a>,  <a href="bmh_SkPath?cl=9919#kInverseWinding_FillType">kInverseWinding FillType</a>, 
<a href="bmh_SkPath?cl=9919#kInverseEvenOdd_FillType">kInverseEvenOdd FillType</a>. 

### Example

<div><fiddle-embed name="2eb8f985d1e263e70b5c0aa4a8b68d8e">#### Example Output

~~~~
default path fill type is kWinding_FillType
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#FillType">FillType</a> <a href="bmh_SkPath?cl=9919#setFillType">setFillType</a> <a href="bmh_SkPath?cl=9919#isInverseFillType">isInverseFillType</a>

---

<a name="setFillType"></a>

## setFillType

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setFillType(FillType ft)
</pre>

Sets <a href="bmh_SkPath?cl=9919#FillType">FillType</a>, the rule used to fill <a href="bmh_SkPath?cl=9919#Path">Path</a>. While <a href="bmh_SkPath?cl=9919#setFillType">setFillType</a> does not check
that <a href="bmh_SkPath?cl=9919#setFillType">ft</a> is legal, values outside of <a href="bmh_SkPath?cl=9919#FillType">FillType</a> are not supported.

### Parameters

<table>
  <tr>
    <td><code><strong>ft </strong></code></td> <td>one of: <a href="bmh_SkPath?cl=9919#kWinding_FillType">kWinding FillType</a>, <a href="bmh_SkPath?cl=9919#kEvenOdd_FillType">kEvenOdd FillType</a>,  <a href="bmh_SkPath?cl=9919#kInverseWinding_FillType">kInverseWinding FillType</a>, 
<a href="bmh_SkPath?cl=9919#kInverseEvenOdd_FillType">kInverseEvenOdd FillType</a>.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="b4a91cd7f50b2a0a0d1bec6d0ac823d2"><p>If empty <a href="bmh_SkPath?cl=9919#Path">Path</a> is set to inverse <a href="bmh_SkPath?cl=9919#FillType">FillType</a>, it fills all pixels.</p></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#FillType">FillType</a> <a href="bmh_SkPath?cl=9919#getFillType">getFillType</a> <a href="bmh_SkPath?cl=9919#toggleInverseFillType">toggleInverseFillType</a>

---

<a name="isInverseFillType"></a>

## isInverseFillType

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isInverseFillType() const
</pre>

Returns if <a href="bmh_SkPath?cl=9919#FillType">FillType</a> describes area outside <a href="bmh_SkPath?cl=9919#Path">Path</a> geometry. The inverse fill area
extends indefinitely.

### Return Value

true if <a href="bmh_SkPath?cl=9919#FillType">FillType</a> is <a href="bmh_SkPath?cl=9919#kInverseWinding_FillType">kInverseWinding FillType</a> or <a href="bmh_SkPath?cl=9919#kInverseEvenOdd_FillType">kInverseEvenOdd FillType</a>.

### Example

<div><fiddle-embed name="2a2d39f5da611545caa18bbcea873ab2">#### Example Output

~~~~
default path fill type is inverse: false
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#FillType">FillType</a> <a href="bmh_SkPath?cl=9919#getFillType">getFillType</a> <a href="bmh_SkPath?cl=9919#setFillType">setFillType</a> <a href="bmh_SkPath?cl=9919#toggleInverseFillType">toggleInverseFillType</a>

---

<a name="toggleInverseFillType"></a>

## toggleInverseFillType

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void toggleInverseFillType()
</pre>

Replace <a href="bmh_SkPath?cl=9919#FillType">FillType</a> with its inverse. The inverse of <a href="bmh_SkPath?cl=9919#FillType">FillType</a> describes the area
unmodified by the original <a href="bmh_SkPath?cl=9919#FillType">FillType</a>.

| <a href="bmh_SkPath?cl=9919#FillType">FillType</a> | toggled <a href="bmh_SkPath?cl=9919#FillType">FillType</a> |
| --- | ---  |
| <a href="bmh_SkPath?cl=9919#kWinding_FillType">kWinding FillType</a> | <a href="bmh_SkPath?cl=9919#kInverseWinding_FillType">kInverseWinding FillType</a> |
| <a href="bmh_SkPath?cl=9919#kEvenOdd_FillType">kEvenOdd FillType</a> | <a href="bmh_SkPath?cl=9919#kInverseEvenOdd_FillType">kInverseEvenOdd FillType</a> |
| <a href="bmh_SkPath?cl=9919#kInverseWinding_FillType">kInverseWinding FillType</a> | <a href="bmh_SkPath?cl=9919#kWinding_FillType">kWinding FillType</a> |
| <a href="bmh_SkPath?cl=9919#kInverseEvenOdd_FillType">kInverseEvenOdd FillType</a> | <a href="bmh_SkPath?cl=9919#kEvenOdd_FillType">kEvenOdd FillType</a> |


### Example

<div><fiddle-embed name="400facce23d417bc5043c5f58404afbd"><p><a href="bmh_SkPath?cl=9919#Path">Path</a> drawn normally and through its inverse touches every pixel once.</p></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#FillType">FillType</a> <a href="bmh_SkPath?cl=9919#getFillType">getFillType</a> <a href="bmh_SkPath?cl=9919#setFillType">setFillType</a> <a href="bmh_SkPath?cl=9919#isInverseFillType">isInverseFillType</a>

---

## <a name="Convexity"></a> Convexity

<a href="bmh_SkPath?cl=9919#Path">Path</a> is convex if it contains one <a href="bmh_SkPath?cl=9919#Contour">Contour</a> and <a href="bmh_SkPath?cl=9919#Contour">Contour</a> loops no more than 
360 degrees, and <a href="bmh_SkPath?cl=9919#Contour">Contour</a> angles all have same <a href="bmh_SkPath?cl=9919#Direction">Direction</a>. Convex <a href="bmh_SkPath?cl=9919#Path">Path</a> 
may have better performance and require fewer resources on <a href="bmh_undocumented?cl=9919#GPU">GPU Surface</a>.

<a href="bmh_SkPath?cl=9919#Path">Path</a> is concave when either at least one <a href="bmh_SkPath?cl=9919#Direction">Direction</a> change is clockwise and
another is counterclockwise, or the sum of the changes in <a href="bmh_SkPath?cl=9919#Direction">Direction</a> is not 360
degrees.

## <a name="SkPath::Convexity"></a> Enum SkPath::Convexity

Initially <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_SkPath?cl=9919#Convexity">Convexity</a> is <a href="bmh_SkPath?cl=9919#kUnknown_Convexity">kUnknown Convexity</a>. <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_SkPath?cl=9919#Convexity">Convexity</a> is computed 
if needed by destination <a href="bmh_undocumented?cl=9919#Surface">Surface</a>.

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="bmh_SkPath?cl=9919#Convexity">Convexity</a> {
    <a href="bmh_SkPath?cl=9919#kUnknown_Convexity">kUnknown Convexity</a>, 
    <a href="bmh_SkPath?cl=9919#kConvex_Convexity">kConvex Convexity</a>,
    <a href="bmh_SkPath?cl=9919#kConcave_Convexity">kConcave Convexity</a> 
};</pre>

### Constants

<table>
  <tr>
    <td><a name="SkPath::kUnknown_Convexity"></a> <code><strong>SkPath::kUnknown_Convexity </strong></code></td><td>Indicates Convexity has not been determined.</td><td></td>
  </tr>
  <tr>
    <td><a name="SkPath::kConvex_Convexity"></a> <code><strong>SkPath::kConvex_Convexity </strong></code></td><td>Path has one Contour made of a simple geometry without indentations.</td><td></td>
  </tr>
  <tr>
    <td><a name="SkPath::kConcave_Convexity"></a> <code><strong>SkPath::kConcave_Convexity </strong></code></td><td>Path has more than one Contour, or a geometry with indentations.</td><td></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="b7d0c0732411db76fa37b05fc18712b3"></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#Contour">Contour</a> <a href="bmh_SkPath?cl=9919#Direction">Direction</a> <a href="bmh_SkPath?cl=9919#getConvexity">getConvexity</a> <a href="bmh_SkPath?cl=9919#getConvexityOrUnknown">getConvexityOrUnknown</a> <a href="bmh_SkPath?cl=9919#setConvexity">setConvexity</a> <a href="bmh_SkPath?cl=9919#isConvex">isConvex</a>



<a name="getConvexity"></a>

## getConvexity

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
Convexity getConvexity() const
</pre>

Computes <a href="bmh_SkPath?cl=9919#Convexity">Convexity</a> if required, and returns stored value. 
<a href="bmh_SkPath?cl=9919#Convexity">Convexity</a> is computed if stored value is <a href="bmh_SkPath?cl=9919#kUnknown_Convexity">kUnknown Convexity</a>,
or if <a href="bmh_SkPath?cl=9919#Path">Path</a> has been altered since <a href="bmh_SkPath?cl=9919#Convexity">Convexity</a> was computed or set.

### Return Value

Computed or stored <a href="bmh_SkPath?cl=9919#Convexity">Convexity</a>.

### Example

<div><fiddle-embed name="c8f5ac4040cb5026d234bf99e3f01e8e"></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#Convexity">Convexity</a> <a href="bmh_SkPath?cl=9919#Contour">Contour</a> <a href="bmh_SkPath?cl=9919#Direction">Direction</a> <a href="bmh_SkPath?cl=9919#getConvexityOrUnknown">getConvexityOrUnknown</a> <a href="bmh_SkPath?cl=9919#setConvexity">setConvexity</a> <a href="bmh_SkPath?cl=9919#isConvex">isConvex</a>

---

<a name="getConvexityOrUnknown"></a>

## getConvexityOrUnknown

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
Convexity getConvexityOrUnknown() const
</pre>

Returns last computed <a href="bmh_SkPath?cl=9919#Convexity">Convexity</a>, or <a href="bmh_SkPath?cl=9919#kUnknown_Convexity">kUnknown Convexity</a> if 
<a href="bmh_SkPath?cl=9919#Path">Path</a> has been altered since <a href="bmh_SkPath?cl=9919#Convexity">Convexity</a> was computed or set.

### Return Value

Stored <a href="bmh_SkPath?cl=9919#Convexity">Convexity</a>.

### Example

<div><fiddle-embed name="bc19da9de880e3f339707247686efc0a"><p><a href="bmh_SkPath?cl=9919#Convexity">Convexity</a> is unknown unless <a href="bmh_SkPath?cl=9919#getConvexity">getConvexity</a> is called without a subsequent call
that alters the path.</p></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#Convexity">Convexity</a> <a href="bmh_SkPath?cl=9919#Contour">Contour</a> <a href="bmh_SkPath?cl=9919#Direction">Direction</a> <a href="bmh_SkPath?cl=9919#getConvexity">getConvexity</a> <a href="bmh_SkPath?cl=9919#setConvexity">setConvexity</a> <a href="bmh_SkPath?cl=9919#isConvex">isConvex</a>

---

<a name="setConvexity"></a>

## setConvexity

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setConvexity(Convexity convexity)
</pre>

Stores <a href="bmh_SkPath?cl=9919#convexity">convexity</a> so that it is later returned by <a href="bmh_SkPath?cl=9919#getConvexity">getConvexity</a> or <a href="bmh_SkPath?cl=9919#getConvexityOrUnknown">getConvexityOrUnknown</a>.
<a href="bmh_SkPath?cl=9919#convexity">convexity</a> may differ from <a href="bmh_SkPath?cl=9919#getConvexity">getConvexity</a>, although setting an incorrect value may
cause incorrect or inefficient drawing.

If <a href="bmh_SkPath?cl=9919#convexity">convexity</a> is <a href="bmh_SkPath?cl=9919#kUnknown_Convexity">kUnknown Convexity</a>: <a href="bmh_SkPath?cl=9919#getConvexity">getConvexity</a> will
compute <a href="bmh_SkPath?cl=9919#Convexity">Convexity</a>, and <a href="bmh_SkPath?cl=9919#getConvexityOrUnknown">getConvexityOrUnknown</a> will return <a href="bmh_SkPath?cl=9919#kUnknown_Convexity">kUnknown Convexity</a>.

If <a href="bmh_SkPath?cl=9919#convexity">convexity</a> is <a href="bmh_SkPath?cl=9919#kConvex_Convexity">kConvex Convexity</a> or <a href="bmh_SkPath?cl=9919#kConcave_Convexity">kConcave Convexity</a>, <a href="bmh_SkPath?cl=9919#getConvexity">getConvexity</a>
and <a href="bmh_SkPath?cl=9919#getConvexityOrUnknown">getConvexityOrUnknown</a> will return <a href="bmh_SkPath?cl=9919#convexity">convexity</a> until the path is
altered.

### Parameters

<table>
  <tr>
    <td><code><strong>convexity </strong></code></td> <td>One of <a href="bmh_SkPath?cl=9919#kUnknown_Convexity">kUnknown Convexity</a>, <a href="bmh_SkPath?cl=9919#kConvex_Convexity">kConvex Convexity</a>, or <a href="bmh_SkPath?cl=9919#kConcave_Convexity">kConcave Convexity</a>.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6fe0d520507eeafe118b80f7f1d9b588"></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#Convexity">Convexity</a> <a href="bmh_SkPath?cl=9919#Contour">Contour</a> <a href="bmh_SkPath?cl=9919#Direction">Direction</a> <a href="bmh_SkPath?cl=9919#getConvexity">getConvexity</a> <a href="bmh_SkPath?cl=9919#getConvexityOrUnknown">getConvexityOrUnknown</a> <a href="bmh_SkPath?cl=9919#isConvex">isConvex</a>

---

<a name="isConvex"></a>

## isConvex

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isConvex() const
</pre>

Computes <a href="bmh_SkPath?cl=9919#Convexity">Convexity</a> if required, and returns true if value is <a href="bmh_SkPath?cl=9919#kConvex_Convexity">kConvex Convexity</a>.
If <a href="bmh_SkPath?cl=9919#setConvexity">setConvexity</a> was called with <a href="bmh_SkPath?cl=9919#kConvex_Convexity">kConvex Convexity</a> or <a href="bmh_SkPath?cl=9919#kConcave_Convexity">kConcave Convexity</a>, and
the path has not been altered, <a href="bmh_SkPath?cl=9919#Convexity">Convexity</a> is not recomputed.

### Return Value

true if <a href="bmh_SkPath?cl=9919#Convexity">Convexity</a> stored or computed is <a href="bmh_SkPath?cl=9919#kConvex_Convexity">kConvex Convexity</a>.

### Example

<div><fiddle-embed name="dfd2c40e1c2a7b539a94aec8d040d349"><p>Concave shape is erroneously considered convex after a forced call to 
<a href="bmh_SkPath?cl=9919#setConvexity">setConvexity</a>.</p></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#Convexity">Convexity</a> <a href="bmh_SkPath?cl=9919#Contour">Contour</a> <a href="bmh_SkPath?cl=9919#Direction">Direction</a> <a href="bmh_SkPath?cl=9919#getConvexity">getConvexity</a> <a href="bmh_SkPath?cl=9919#getConvexityOrUnknown">getConvexityOrUnknown</a> <a href="bmh_SkPath?cl=9919#setConvexity">setConvexity</a>

---

<a name="setIsConvex"></a>

## setIsConvex

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setIsConvex(bool isConvex)
</pre>

Deprecated. Use <a href="bmh_SkPath?cl=9919#setConvexity">setConvexity</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>isConvex </strong></code></td> <td></td>
  </tr>

---

</table>

<a name="isOval"></a>

## isOval

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isOval(SkRect* rect, Direction* dir = nullptr, unsigned* start = nullptr) const
</pre>

<a href="bmh_SkPath?cl=9919#Path">Path</a> is <a href="bmh_undocumented?cl=9919#Oval">Oval</a> if constructed by <a href="bmh_SkPath?cl=9919#addCircle">addCircle</a>, <a href="bmh_SkPath?cl=9919#addOval">addOval</a>; and in some cases,
<a href="bmh_SkPath?cl=9919#addRoundRect">addRoundRect</a>, <a href="bmh_SkPath?cl=9919#addRRect">addRRect</a>.  <a href="bmh_SkPath?cl=9919#Path">Path</a> constructed with <a href="bmh_SkPath?cl=9919#conicTo">conicTo</a> or <a href="bmh_SkPath?cl=9919#rConicTo">rConicTo</a> will not
return true though <a href="bmh_SkPath?cl=9919#Path">Path</a> draws <a href="bmh_undocumented?cl=9919#Oval">Oval</a>.

<a href="bmh_SkPath?cl=9919#isOval">isOval</a> triggers performance optimizations on some <a href="bmh_undocumented?cl=9919#GPU">GPU Surface</a> implementations.

### Parameters

<table>
  <tr>
    <td><code><strong>rect </strong></code></td> <td>storage for bounding <a href="bmh_undocumented?cl=9919#Rect">Rect</a> of <a href="bmh_undocumented?cl=9919#Oval">Oval</a>. <a href="bmh_undocumented?cl=9919#Oval">Oval</a> is <a href="bmh_undocumented?cl=9919#Circle">Circle</a> if <a href="bmh_SkPath?cl=9919#isOval">rect</a> width
equals <a href="bmh_SkPath?cl=9919#isOval">rect</a> height. Unwritten if <a href="bmh_SkPath?cl=9919#Path">Path</a> is not <a href="bmh_undocumented?cl=9919#Oval">Oval</a>. May be nullptr.</td>
  </tr>
  <tr>
    <td><code><strong>dir </strong></code></td> <td>storage for <a href="bmh_SkPath?cl=9919#Direction">Direction</a>; <a href="bmh_SkPath?cl=9919#kCW_Direction">kCW Direction</a> if clockwise, <a href="bmh_SkPath?cl=9919#kCCW_Direction">kCCW Direction</a> if
counterclockwise. Unwritten if <a href="bmh_SkPath?cl=9919#Path">Path</a> is not <a href="bmh_undocumented?cl=9919#Oval">Oval</a>. May be nullptr.</td>
  </tr>
  <tr>
    <td><code><strong>start </strong></code></td> <td>storage for <a href="bmh_SkPath?cl=9919#isOval">start</a> of <a href="bmh_undocumented?cl=9919#Oval">Oval</a>: 0 for top,
1 for right, 2 for bottom, 3 for left. Unwritten if <a href="bmh_SkPath?cl=9919#Path">Path</a> is not <a href="bmh_undocumented?cl=9919#Oval">Oval</a>. May be nullptr.</td>
  </tr>
</table>

### Return Value

true if <a href="bmh_SkPath?cl=9919#Path">Path</a> was constructed by method that reduces to <a href="bmh_undocumented?cl=9919#Oval">Oval</a>.

### Example

<div><fiddle-embed name="4fc7b86c9b772c5e85af480524267bde"></fiddle-embed></div>

### See Also

<a href="bmh_undocumented?cl=9919#Oval">Oval</a> <a href="bmh_SkPath?cl=9919#addCircle">addCircle</a> <a href="bmh_SkPath?cl=9919#addOval">addOval</a>

---

<a name="isRRect"></a>

## isRRect

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isRRect(SkRRect* rrect, Direction* dir = nullptr, unsigned* start = nullptr) const
</pre>

<a href="bmh_SkPath?cl=9919#Path">Path</a> is <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a> if constructed by <a href="bmh_SkPath?cl=9919#addRoundRect">addRoundRect</a>, <a href="bmh_SkPath?cl=9919#addRRect">addRRect</a>; and if construction
is not empty, not <a href="bmh_undocumented?cl=9919#Rect">Rect</a>, and not <a href="bmh_undocumented?cl=9919#Oval">Oval</a>. <a href="bmh_SkPath?cl=9919#Path">Path</a> constructed with other other calls
will not return true though <a href="bmh_SkPath?cl=9919#Path">Path</a> draws <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a>.

<a href="bmh_SkPath?cl=9919#isRRect">isRRect</a> triggers performance optimizations on some <a href="bmh_undocumented?cl=9919#GPU">GPU Surface</a> implementations.

### Parameters

<table>
  <tr>
    <td><code><strong>rrect </strong></code></td> <td>storage for bounding <a href="bmh_undocumented?cl=9919#Rect">Rect</a> of <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a>. 
Unwritten if <a href="bmh_SkPath?cl=9919#Path">Path</a> is not <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a>. May be nullptr.</td>
  </tr>
  <tr>
    <td><code><strong>dir </strong></code></td> <td>storage for <a href="bmh_SkPath?cl=9919#Direction">Direction</a>; <a href="bmh_SkPath?cl=9919#kCW_Direction">kCW Direction</a> if clockwise, <a href="bmh_SkPath?cl=9919#kCCW_Direction">kCCW Direction</a> if
counterclockwise. Unwritten if <a href="bmh_SkPath?cl=9919#Path">Path</a> is not <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a>. May be nullptr.</td>
  </tr>
  <tr>
    <td><code><strong>start </strong></code></td> <td>storage for <a href="bmh_SkPath?cl=9919#isRRect">start</a> of <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a>: 0 for top,
1 for right, 2 for bottom, 3 for left. Unwritten if <a href="bmh_SkPath?cl=9919#Path">Path</a> is not <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a>. May be nullptr.</td>
  </tr>
</table>

### Return Value

true for <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a> <a href="bmh_SkPath?cl=9919#Path">Path</a> constructed by <a href="bmh_SkPath?cl=9919#addRoundRect">addRoundRect</a> or <a href="bmh_SkPath?cl=9919#addRRect">addRRect</a>.

### Example

<div><fiddle-embed name="f2b7e57a385e6604475c99ec8daa2697"></fiddle-embed></div>

### See Also

<a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a> <a href="bmh_SkPath?cl=9919#addRoundRect">addRoundRect</a> <a href="bmh_SkPath?cl=9919#addRRect">addRRect</a>

---

<a name="reset"></a>

## reset

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void reset()
</pre>

Sets <a href="bmh_SkPath?cl=9919#Path">Path</a> to its intial state.
Removes <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a>, <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>, and <a href="bmh_SkPath?cl=9919#Weight">Weights</a>, and sets <a href="bmh_SkPath?cl=9919#FillType">FillType</a> to <a href="bmh_SkPath?cl=9919#kWinding_FillType">kWinding FillType</a>.
Internal storage associated with <a href="bmh_SkPath?cl=9919#Path">Path</a> is released.

### Example

<div><fiddle-embed name="8cdca35d2964bbbecb93d79a13f71c65"></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#rewind">rewind</a>

---

<a name="rewind"></a>

## rewind

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void rewind()
</pre>

Sets <a href="bmh_SkPath?cl=9919#Path">Path</a> to its intial state, preserving internal storage.
Removes <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a>, <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>, and <a href="bmh_SkPath?cl=9919#Weight">Weights</a>, and sets <a href="bmh_SkPath?cl=9919#FillType">FillType</a> to <a href="bmh_SkPath?cl=9919#kWinding_FillType">kWinding FillType</a>.
Internal storage associated with <a href="bmh_SkPath?cl=9919#Path">Path</a> is retained.

Use <a href="bmh_SkPath?cl=9919#rewind">rewind</a> instead of <a href="bmh_SkPath?cl=9919#reset">reset</a> if <a href="bmh_SkPath?cl=9919#Path">Path</a> storage will be reused and performance
is critical. 

### Example

<div><fiddle-embed name="f1fedbb89da9c2a33a91805175663012"><p>Although path1 retains its internal storage, it is indistinguishable from
a newly initialized path.</p></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#reset">reset</a>

---

<a name="isEmpty"></a>

## isEmpty

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isEmpty() const
</pre>

Empty <a href="bmh_SkPath?cl=9919#Path">Path</a> may have <a href="bmh_SkPath?cl=9919#FillType">FillType</a> but has no <a href="bmh_undocumented?cl=9919#SkPoint">SkPoint</a>, <a href="bmh_SkPath?cl=9919#Verb">Verb</a>, or <a href="bmh_SkPath?cl=9919#Conic_Weight">Conic Weight</a>.
<a href="bmh_SkPath?cl=9919#empty_constructor">SkPath()</a> constructs empty <a href="bmh_SkPath?cl=9919#Path">Path</a>; <a href="bmh_SkPath?cl=9919#reset">reset</a> and (rewind) make <a href="bmh_SkPath?cl=9919#Path">Path</a> empty. 

### Return Value

true if the path contains no <a href="bmh_SkPath?cl=9919#Verb">Verb</a> array.

### Example

<div><fiddle-embed name="0b34e6d55d11586744adeb889d2a12f4">#### Example Output

~~~~
initial path is empty
after moveTo path is not empty
after rewind path is empty
after lineTo path is not empty
after reset path is empty
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#empty_constructor">SkPath()</a> <a href="bmh_SkPath?cl=9919#reset">reset</a> <a href="bmh_SkPath?cl=9919#rewind">rewind</a>

---

<a name="isLastContourClosed"></a>

## isLastContourClosed

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isLastContourClosed() const
</pre>

<a href="bmh_SkPath?cl=9919#Contour">Contour</a> is closed if <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_SkPath?cl=9919#Verb">Verb</a> array was last modified by <a href="bmh_SkPath?cl=9919#close">close</a>. When stroked,
closed <a href="bmh_SkPath?cl=9919#Contour">Contour</a> draws <a href="bmh_SkPaint?cl=9919#Stroke_Join">Paint Stroke Join</a> instead of <a href="bmh_SkPaint?cl=9919#Stroke_Cap">Paint Stroke Cap</a> at first and last <a href="bmh_undocumented?cl=9919#Point">Point</a>. 

### Return Value

true if the last <a href="bmh_SkPath?cl=9919#Contour">Contour</a> ends with a <a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a>.

### Example

<div><fiddle-embed name="03b740ab94b9017800a52e30b5e7fee7"><p><a href="bmh_SkPath?cl=9919#close">close</a> has no effect if <a href="bmh_SkPath?cl=9919#Path">Path</a> is empty; <a href="bmh_SkPath?cl=9919#isLastContourClosed">isLastContourClosed</a> returns
false until <a href="bmh_SkPath?cl=9919#Path">Path</a> has geometry followed by <a href="bmh_SkPath?cl=9919#close">close</a>.</p>

#### Example Output

~~~~
initial last contour is not closed
after close last contour is not closed
after lineTo last contour is not closed
after close last contour is closed
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#close">close</a>

---

<a name="isFinite"></a>

## isFinite

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isFinite() const
</pre>

Finite <a href="bmh_undocumented?cl=9919#Point">Point</a> array values are between negative <a href="bmh_undocumented?cl=9919#SK_ScalarMax">SK ScalarMax</a> and
positive <a href="bmh_undocumented?cl=9919#SK_ScalarMax">SK ScalarMax</a>. Any <a href="bmh_undocumented?cl=9919#Point">Point</a> array value of
<a href="bmh_undocumented?cl=9919#SK_ScalarInfinity">SK ScalarInfinity</a>, <a href="bmh_undocumented?cl=9919#SK_ScalarNegativeInfinity">SK ScalarNegativeInfinity</a>, or <a href="bmh_undocumented?cl=9919#SK_ScalarNaN">SK ScalarNaN</a>
cause <a href="bmh_SkPath?cl=9919#isFinite">isFinite</a> to return false.

### Return Value

true if all <a href="bmh_undocumented?cl=9919#Point">Point</a> values are finite.

### Example

<div><fiddle-embed name="dd4e4dd2aaa8039b2430729c6b3af817">#### Example Output

~~~~
initial path is finite
after line path is finite
after scale path is not finite
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a>

---

<a name="isVolatile"></a>

## isVolatile

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isVolatile() const
</pre>

Returns true if the path is volatile; it will not be altered or discarded
by the caller after it is drawn. <a href="bmh_SkPath?cl=9919#Path">Paths</a> by default have volatile set false, allowing 
<a href="bmh_undocumented?cl=9919#Surface">Surface</a> to attach a cache of data which speeds repeated drawing. If true, <a href="bmh_undocumented?cl=9919#Surface">Surface</a>
may not speed repeated drawing.

### Return Value

true if caller will alter <a href="bmh_SkPath?cl=9919#Path">Path</a> after drawing.

### Example

<div><fiddle-embed name="c722ebe8ac991d77757799ce29e509e1">#### Example Output

~~~~
volatile by default is false
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#setIsVolatile">setIsVolatile</a>

---

<a name="setIsVolatile"></a>

## setIsVolatile

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setIsVolatile(bool isVolatile)
</pre>

Specify whether <a href="bmh_SkPath?cl=9919#Path">Path</a> is volatile; whether it will be altered or discarded
by the caller after it is drawn. <a href="bmh_SkPath?cl=9919#Path">Paths</a> by default have volatile set false, allowing 
<a href="bmh_undocumented?cl=9919#Device">Device</a> to attach a cache of data which speeds repeated drawing.

Mark temporary paths, discarded or modified after use, as volatile
to inform <a href="bmh_undocumented?cl=9919#Device">Device</a> that the path need not be cached.

Mark animating <a href="bmh_SkPath?cl=9919#Path">Path</a> volatile to improve performance.
Mark unchanging <a href="bmh_SkPath?cl=9919#Path">Path</a> non-volative to improve repeated rendering.

<a href="bmh_undocumented?cl=9919#Raster">Raster Surface</a> <a href="bmh_SkPath?cl=9919#Path">Path</a> draws are affected by volatile for some shadows.
<a href="bmh_undocumented?cl=9919#GPU">GPU Surface</a> <a href="bmh_SkPath?cl=9919#Path">Path</a> draws are affected by volatile for some shadows and concave geometries.

### Parameters

<table>
  <tr>
    <td><code><strong>isVolatile </strong></code></td> <td>true if caller will alter <a href="bmh_SkPath?cl=9919#Path">Path</a> after drawing.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="2049ff5141f0c80aac497618622b28af"></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#isVolatile">isVolatile</a>

---

<a name="IsLineDegenerate"></a>

## IsLineDegenerate

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static bool IsLineDegenerate(const SkPoint& p1, const SkPoint& p2, bool exact)
</pre>

Test if <a href="bmh_undocumented?cl=9919#Line">Line</a> between <a href="bmh_undocumented?cl=9919#Point">Point</a> pair is degenerate.
<a href="bmh_undocumented?cl=9919#Line">Line</a> with no length or that moves a very short distance is degenerate; it is
treated as a point. 

### Parameters

<table>
  <tr>
    <td><code><strong>p1 </strong></code></td> <td><a href="bmh_undocumented?cl=9919#Line">Line</a> start point.</td>
  </tr>
  <tr>
    <td><code><strong>p2 </strong></code></td> <td><a href="bmh_undocumented?cl=9919#Line">Line</a> end point.</td>
  </tr>
  <tr>
    <td><code><strong>exact </strong></code></td> <td>If true, returns true only if <a href="bmh_SkPath?cl=9919#p1">p1</a> equals <a href="bmh_SkPath?cl=9919#p2">p2</a>. If false, returns true
              if <a href="bmh_SkPath?cl=9919#p1">p1</a> equals or nearly equals <a href="bmh_SkPath?cl=9919#p2">p2</a>.</td>
  </tr>
</table>

### Return Value

true if <a href="bmh_undocumented?cl=9919#Line">Line</a> is degenerate; its length is effectively zero.

### Example

<div><fiddle-embed name="97a031f9186ade586928563840ce9116"><p>As single precision floats, 100 and 100.000001f have the same bit representation,
and are exactly equal. 100 and 100.0001f have different bit representations, and
are not exactly equal, but are nearly equal.</p>

#### Example Output

~~~~
line from (100,100) to (100,100) is degenerate, nearly
line from (100,100) to (100,100) is degenerate, exactly
line from (100,100) to (100.0001,100.0001) is degenerate, nearly
line from (100,100) to (100.0001,100.0001) is not degenerate, exactly
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#IsQuadDegenerate">IsQuadDegenerate</a> <a href="bmh_SkPath?cl=9919#IsCubicDegenerate">IsCubicDegenerate</a> <a href="bmh_undocumented?cl=9919#equalsWithinTolerance">SkPoint::equalsWithinTolerance</a>

---

<a name="IsQuadDegenerate"></a>

## IsQuadDegenerate

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static bool IsQuadDegenerate(const SkPoint& p1, const SkPoint& p2,
                             const SkPoint& p3, bool exact)
</pre>

Test if <a href="bmh_SkPath?cl=9919#Quad">Quad</a> is degenerate.
<a href="bmh_SkPath?cl=9919#Quad">Quad</a> with no length or that moves a very short distance is degenerate; it is
treated as a point. 

### Parameters

<table>
  <tr>
    <td><code><strong>p1 </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Quad">Quad</a> start point.</td>
  </tr>
  <tr>
    <td><code><strong>p2 </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Quad">Quad</a> control point.</td>
  </tr>
  <tr>
    <td><code><strong>p3 </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Quad">Quad</a> end point.</td>
  </tr>
  <tr>
    <td><code><strong>exact </strong></code></td> <td>If true, returns true only if <a href="bmh_SkPath?cl=9919#p1">p1</a>, <a href="bmh_SkPath?cl=9919#p2">p2</a>, and <a href="bmh_SkPath?cl=9919#p3">p3</a> are equal. 
              If false, returns true if <a href="bmh_SkPath?cl=9919#p1">p1</a>, <a href="bmh_SkPath?cl=9919#p2">p2</a>, and <a href="bmh_SkPath?cl=9919#p3">p3</a> are equal or nearly equal.</td>
  </tr>
</table>

### Return Value

true if <a href="bmh_SkPath?cl=9919#Quad">Quad</a> is degenerate; its length is effectively zero.

### Example

<div><fiddle-embed name="1d50896c528cd4581966646b7d96acff"><p>As single precision floats: 100, 100.00001f, and 100.00002f have different bit representations
but nearly the same value. Translating all three by 1000 gives them the same bit representation;
the fractional portion of the number can't be represented by the float and is lost.</p>

#### Example Output

~~~~
quad (100,100), (100.00001,100.00001), (100.00002,100.00002) is degenerate, nearly
quad (1100,1100), (1100,1100), (1100,1100) is degenerate, nearly
quad (100,100), (100.00001,100.00001), (100.00002,100.00002) is not degenerate, exactly
quad (1100,1100), (1100,1100), (1100,1100) is degenerate, exactly
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#IsLineDegenerate">IsLineDegenerate</a> <a href="bmh_SkPath?cl=9919#IsCubicDegenerate">IsCubicDegenerate</a> <a href="bmh_undocumented?cl=9919#equalsWithinTolerance">SkPoint::equalsWithinTolerance</a>

---

<a name="IsCubicDegenerate"></a>

## IsCubicDegenerate

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static bool IsCubicDegenerate(const SkPoint& p1, const SkPoint& p2,
                              const SkPoint& p3, const SkPoint& p4, bool exact)
</pre>

Test if <a href="bmh_SkPath?cl=9919#Cubic">Cubic</a> is degenerate.
<a href="bmh_SkPath?cl=9919#Cubic">Cubic</a> with no length or that moves a very short distance is degenerate; it is
treated as a point. 

### Parameters

<table>
  <tr>
    <td><code><strong>p1 </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Cubic">Cubic</a> start point.</td>
  </tr>
  <tr>
    <td><code><strong>p2 </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Cubic">Cubic</a> control point 1.</td>
  </tr>
  <tr>
    <td><code><strong>p3 </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Cubic">Cubic</a> control point 2.</td>
  </tr>
  <tr>
    <td><code><strong>p4 </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Cubic">Cubic</a> end point.</td>
  </tr>
  <tr>
    <td><code><strong>exact </strong></code></td> <td>If true, returns true only if <a href="bmh_SkPath?cl=9919#p1">p1</a>, <a href="bmh_SkPath?cl=9919#p2">p2</a>, <a href="bmh_SkPath?cl=9919#p3">p3</a>, and <a href="bmh_SkPath?cl=9919#p4">p4</a> are equal. 
              If false, returns true if <a href="bmh_SkPath?cl=9919#p1">p1</a>, <a href="bmh_SkPath?cl=9919#p2">p2</a>, <a href="bmh_SkPath?cl=9919#p3">p3</a>, and <a href="bmh_SkPath?cl=9919#p4">p4</a> are equal or nearly equal.</td>
  </tr>
</table>

### Return Value

true if <a href="bmh_SkPath?cl=9919#Cubic">Cubic</a> is degenerate; its length is effectively zero.

### Example

<div><fiddle-embed name="c79d813f0b37062cb2f7a0c83f4a09f3">#### Example Output

~~~~
0.00024414062 is degenerate
0.00024414065 is length
~~~~

</fiddle-embed></div>

---

<a name="isLine"></a>

## isLine

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isLine(SkPoint line[2]) const
</pre>

Returns true if <a href="bmh_SkPath?cl=9919#Path">Path</a> contains only one <a href="bmh_undocumented?cl=9919#Line">Line</a>;
<a href="bmh_SkPath?cl=9919#Verb">Path Verb</a> array has two entries: <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a>, <a href="bmh_SkPath?cl=9919#kLine_Verb">kLine Verb</a>. 
If <a href="bmh_SkPath?cl=9919#Path">Path</a> contains one <a href="bmh_undocumented?cl=9919#Line">Line</a> and <a href="bmh_SkPath?cl=9919#line">line</a> is not nullptr, <a href="bmh_SkPath?cl=9919#line">line</a> is set to 
<a href="bmh_undocumented?cl=9919#Line">Line</a> start point and <a href="bmh_undocumented?cl=9919#Line">Line</a> end point.
Returns false if <a href="bmh_SkPath?cl=9919#Path">Path</a> is not one <a href="bmh_undocumented?cl=9919#Line">Line</a>; <a href="bmh_SkPath?cl=9919#line">line</a> is unaltered.

### Parameters

<table>
  <tr>
    <td><code><strong>line </strong></code></td> <td>storage for <a href="bmh_undocumented?cl=9919#Line">Line</a>. May be nullptr.</td>
  </tr>
</table>

### Return Value

true if <a href="bmh_SkPath?cl=9919#Path">Path</a> contains exactly one <a href="bmh_undocumented?cl=9919#Line">Line</a>.

### Example

<div><fiddle-embed name="1ad07d56e4258e041606d50cad969392">#### Example Output

~~~~
empty is not line
zero line is line (0,0) (0,0)
line is line (10,10) (20,20)
second move is not line
~~~~

</fiddle-embed></div>

---

## <a name="Point_Array"></a> Point Array

<a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a> contains <a href="bmh_undocumented?cl=9919#Point">Points</a> satisfying the allocated <a href="bmh_undocumented?cl=9919#Point">Points</a> for
each <a href="bmh_SkPath?cl=9919#Verb">Verb</a> in <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a>. For instance, <a href="bmh_SkPath?cl=9919#Path">Path</a> containing one <a href="bmh_SkPath?cl=9919#Contour">Contour</a> with <a href="bmh_undocumented?cl=9919#Line">Line</a>
and <a href="bmh_SkPath?cl=9919#Quad">Quad</a> is described by <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a>: move to, line to, quad to; and
one <a href="bmh_undocumented?cl=9919#Point">Point</a> for move, one <a href="bmh_undocumented?cl=9919#Point">Point</a> for <a href="bmh_undocumented?cl=9919#Line">Line</a>, two <a href="bmh_undocumented?cl=9919#Point">Points</a> for <a href="bmh_SkPath?cl=9919#Quad">Quad</a>; totaling four <a href="bmh_undocumented?cl=9919#Point">Points</a>.

<a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a> may be read directly from <a href="bmh_SkPath?cl=9919#Path">Path</a> with <a href="bmh_SkPath?cl=9919#getPoints">getPoints</a>, or inspected with
<a href="bmh_SkPath?cl=9919#getPoint">getPoint</a>, with <a href="bmh_SkPath?cl=9919#Iter">Iter</a>, or with <a href="bmh_SkPath?cl=9919#RawIter">RawIter</a>.

<a name="getPoints"></a>

## getPoints

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
int getPoints(SkPoint points[], int max) const
</pre>

Returns number of <a href="bmh_SkPath?cl=9919#points">points</a> in <a href="bmh_SkPath?cl=9919#Path">Path</a>. Up to <a href="bmh_SkPath?cl=9919#max">max</a> <a href="bmh_SkPath?cl=9919#points">points</a> are copied.
<a href="bmh_SkPath?cl=9919#points">points</a> may be nullptr; then, <a href="bmh_SkPath?cl=9919#max">max</a> must be zero.
If <a href="bmh_SkPath?cl=9919#max">max</a> is greater than number of <a href="bmh_SkPath?cl=9919#points">points</a>, excess <a href="bmh_SkPath?cl=9919#points">points</a> storage is unaltered. 

### Parameters

<table>
  <tr>
    <td><code><strong>points </strong></code></td> <td>storage for <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_undocumented?cl=9919#Point">Point</a> array. May be nullptr.</td>
  </tr>
  <tr>
    <td><code><strong>max </strong></code></td> <td>Number of <a href="bmh_SkPath?cl=9919#points">points</a> alloted in <a href="bmh_SkPath?cl=9919#points">points</a> storage; must be greater than or equal to zero.</td>
  </tr>
</table>

### Return Value

<a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_undocumented?cl=9919#Point">Point</a> array length.

### Example

<div><fiddle-embed name="9bc86efda08cbcd9c6f7c5f220294a24">#### Example Output

~~~~
no points point count: 3
zero max point count: 3
too small point count: 3  (0,0) (20,20)
just right point count: 3  (0,0) (20,20) (-10,-10)
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#countPoints">countPoints</a> <a href="bmh_SkPath?cl=9919#getPoint">getPoint</a>

---

<a name="countPoints"></a>

## countPoints

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
int countPoints() const
</pre>

Returns the number of points in <a href="bmh_SkPath?cl=9919#Path">Path</a>.
<a href="bmh_undocumented?cl=9919#Point">Point</a> count is initially zero. 

### Return Value

<a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_undocumented?cl=9919#Point">Point</a> array length.

### Example

<div><fiddle-embed name="bca6379ccef62cb081b10db7381deb27">#### Example Output

~~~~
empty point count: 0
zero line point count: 2
line point count: 2
second move point count: 3
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#getPoints">getPoints</a>

---

<a name="getPoint"></a>

## getPoint

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkPoint getPoint(int index) const
</pre>

Returns <a href="bmh_undocumented?cl=9919#Point">Point</a> at <a href="bmh_SkPath?cl=9919#index">index</a> in <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>. Valid range for <a href="bmh_SkPath?cl=9919#index">index</a> is
0 to <a href="bmh_SkPath?cl=9919#countPoints">countPoints</a> - 1.
If the <a href="bmh_SkPath?cl=9919#index">index</a> is out of range, <a href="bmh_SkPath?cl=9919#getPoint">getPoint</a> returns (0, 0). 

### Parameters

<table>
  <tr>
    <td><code><strong>index </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a> element selector.</td>
  </tr>
</table>

### Return Value

<a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a> value or (0, 0).

### Example

<div><fiddle-embed name="1cf6b8dd2994c4ca9a2d6887ff888017">#### Example Output

~~~~
point 0: (-10,-10)
point 1: (10,10)
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#countPoints">countPoints</a> <a href="bmh_SkPath?cl=9919#getPoints">getPoints</a>

---

## <a name="Verb_Array"></a> Verb Array

<a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a> always starts with <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a>.
If <a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a> is not the last entry, it is always followed by <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a>;
the quantity of <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a> equals the <a href="bmh_SkPath?cl=9919#Contour">Contour</a> count.
<a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a> does not include or count <a href="bmh_SkPath?cl=9919#kDone_Verb">kDone Verb</a>; it is a convenience
returned when iterating through <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a>.

<a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a> may be read directly from <a href="bmh_SkPath?cl=9919#Path">Path</a> with <a href="bmh_SkPath?cl=9919#getVerbs">getVerbs</a>, or inspected with <a href="bmh_SkPath?cl=9919#Iter">Iter</a>, 
or with <a href="bmh_SkPath?cl=9919#RawIter">RawIter</a>.

<a name="countVerbs"></a>

## countVerbs

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
int countVerbs() const
</pre>

Returns the number of <a href="bmh_SkPath?cl=9919#Verb">Verbs</a>: <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a>, <a href="bmh_SkPath?cl=9919#kLine_Verb">kLine Verb</a>, <a href="bmh_SkPath?cl=9919#kQuad_Verb">kQuad Verb</a>, <a href="bmh_SkPath?cl=9919#kConic_Verb">kConic Verb</a>, 
<a href="bmh_SkPath?cl=9919#kCubic_Verb">kCubic Verb</a>, and <a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a>; added to <a href="bmh_SkPath?cl=9919#Path">Path</a>.

### Return Value

Length of <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a>.

### Example

<div><fiddle-embed name="af0c66aea3ef81b709664c7007f48aae">#### Example Output

~~~~
empty verb count: 0
round rect verb count: 10
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#getVerbs">getVerbs</a> <a href="bmh_SkPath?cl=9919#Iter">Iter</a> <a href="bmh_SkPath?cl=9919#RawIter">RawIter</a>

---

<a name="getVerbs"></a>

## getVerbs

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
int getVerbs(uint8_t verbs[], int max) const
</pre>

Returns the number of <a href="bmh_SkPath?cl=9919#verbs">verbs</a> in the path. Up to <a href="bmh_SkPath?cl=9919#max">max</a> <a href="bmh_SkPath?cl=9919#verbs">verbs</a> are copied. The
<a href="bmh_SkPath?cl=9919#verbs">verbs</a> are copied as one byte per verb.

### Parameters

<table>
  <tr>
    <td><code><strong>verbs </strong></code></td> <td>If not null, receives up to <a href="bmh_SkPath?cl=9919#max">max</a> <a href="bmh_SkPath?cl=9919#verbs">verbs</a></td>
  </tr>
  <tr>
    <td><code><strong>max </strong></code></td> <td>The maximum number of <a href="bmh_SkPath?cl=9919#verbs">verbs</a> to copy into <a href="bmh_SkPath?cl=9919#verbs">verbs</a></td>
  </tr>
</table>

### Return Value

the actual number of <a href="bmh_SkPath?cl=9919#verbs">verbs</a> in the path

### Example

<div><fiddle-embed name="2ec66880966a6133ddd9331ce7323438">#### Example Output

~~~~
no verbs verb count: 3
zero max verb count: 3
too small verb count: 3  move line
just right verb count: 3  move line line
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#countVerbs">countVerbs</a> <a href="bmh_SkPath?cl=9919#getPoints">getPoints</a> <a href="bmh_SkPath?cl=9919#Iter">Iter</a> <a href="bmh_SkPath?cl=9919#RawIter">RawIter</a>

---

<a name="swap"></a>

## swap

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void swap(SkPath& other)
</pre>

Exchanges the <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a>, <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>, <a href="bmh_SkPath?cl=9919#Weight">Weights</a>, and <a href="bmh_SkPath?cl=9919#Fill_Type">Fill Type</a> with <a href="bmh_SkPath?cl=9919#other">other</a>.
Cached state is also exchanged. <a href="bmh_SkPath?cl=9919#swap">swap</a> internally exchanges pointers, so
it is lightweight and does not allocate memory.

<a href="bmh_SkPath?cl=9919#swap">swap</a> usage has largely been replaced by <a href="bmh_SkPath?cl=9919#copy_assignment_operator">operator=(const SkPath& path)</a>.
<a href="bmh_SkPath?cl=9919#Path">Paths</a> do not copy their content on assignment util they are written to,
making assignment as efficient as <a href="bmh_SkPath?cl=9919#swap">swap</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>other </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Path">Path</a> exchanged by value.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="4c5ebee2b5039e5faefa07ae63a15467">#### Example Output

~~~~
path1 bounds = 0, 0, 0, 0
path2 bounds = 10, 20, 30, 40
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#copy_assignment_operator">operator=(const SkPath& path)</a>

---

<a name="getBounds"></a>

## getBounds

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
const SkRect& getBounds() const
</pre>

Returns minimum and maximum x and y values of <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>. If <a href="bmh_SkPath?cl=9919#Path">Path</a> contains
no points, <a href="bmh_SkPath?cl=9919#getBounds">getBounds</a> returns (0, 0, 0, 0). Returned bounds width and height may
be larger or smaller than area affected when <a href="bmh_SkPath?cl=9919#Path">Path</a> is drawn.

<a href="bmh_SkPath?cl=9919#getBounds">getBounds</a> includes all <a href="bmh_undocumented?cl=9919#Point">Points</a> added to <a href="bmh_SkPath?cl=9919#Path">Path</a>, including <a href="bmh_undocumented?cl=9919#Point">Points</a> associated with
<a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a> that define empty <a href="bmh_SkPath?cl=9919#Contour">Contours</a>.

### Return Value

bounds of all <a href="bmh_undocumented?cl=9919#Point">Points</a> in <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>.

### Example

<div><fiddle-embed name="9160aa6d1476bd87d927cfc8a4bf25e7"><p>Bounds of upright <a href="bmh_undocumented?cl=9919#Circle">Circle</a> can be predicted from center and radius.
Bounds of rotated <a href="bmh_undocumented?cl=9919#Circle">Circle</a> includes control <a href="bmh_undocumented?cl=9919#Point">Points</a> outside of filled area.</p>

#### Example Output

~~~~
empty bounds = 0, 0, 0, 0
circle bounds = 25, 20, 75, 70
rotated circle bounds = 14.6447, 9.64466, 85.3553, 80.3553
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#computeTightBounds">computeTightBounds</a> <a href="bmh_SkPath?cl=9919#updateBoundsCache">updateBoundsCache</a>

---

<a name="updateBoundsCache"></a>

## updateBoundsCache

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void updateBoundsCache() const
</pre>

Update internal bounds so that subsequent calls to <a href="bmh_SkPath?cl=9919#getBounds">getBounds</a> are instantaneous.
Unaltered copies of <a href="bmh_SkPath?cl=9919#Path">Path</a> may also access cached bounds through <a href="bmh_SkPath?cl=9919#getBounds">getBounds</a>.

For now, <a href="bmh_SkPath?cl=9919#updateBoundsCache">updateBoundsCache</a> is identical to <a href="bmh_SkPath?cl=9919#getBounds">getBounds</a>, where the
returned value is ignored.

<a href="bmh_SkPath?cl=9919#updateBoundsCache">updateBoundsCache</a> prepares a <a href="bmh_SkPath?cl=9919#Path">Path</a> subsequently drawn from multiple threads, 
to avoid a race condition where each draw separately computes the bounds.

### Example

<div><fiddle-embed name="bb761cd858e6d0ca05627262cd22ff5e">#### Example Output

~~~~
uncached avg: 0.18048 ms
cached avg: 0.182784 ms
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#getBounds">getBounds</a>

---

<a name="computeTightBounds"></a>

## computeTightBounds

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkRect computeTightBounds() const
</pre>

Returns minimum and maximum x and y values of the lines and curves in <a href="bmh_SkPath?cl=9919#Path">Path</a>.
If <a href="bmh_SkPath?cl=9919#Path">Path</a> contains no points, <a href="bmh_SkPath?cl=9919#computeTightBounds">computeTightBounds</a> returns (0, 0, 0, 0). 
Returned bounds width and height may be larger or smaller than area affected 
when <a href="bmh_SkPath?cl=9919#Path">Path</a> is drawn.

<a href="bmh_SkPath?cl=9919#computeTightBounds">computeTightBounds</a> behaves identically to <a href="bmh_SkPath?cl=9919#getBounds">getBounds</a> when <a href="bmh_SkPath?cl=9919#Path">Path</a> contains
only lines. If <a href="bmh_SkPath?cl=9919#Path">Path</a> contains curves, compute <a href="bmh_SkPath?cl=9919#computeTightBounds">computeTightBounds</a> includes
the maximum extent of the <a href="bmh_SkPath?cl=9919#Quad">Quad</a>, <a href="bmh_SkPath?cl=9919#Conic">Conic</a>, or <a href="bmh_SkPath?cl=9919#Cubic">Cubic</a>; is slower,
and does not cache the result.

Like <a href="bmh_SkPath?cl=9919#getBounds">getBounds</a>, <a href="bmh_SkPath?cl=9919#computeTightBounds">computeTightBounds</a> includes <a href="bmh_undocumented?cl=9919#Point">Points</a> associated with
<a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a> that define empty <a href="bmh_SkPath?cl=9919#Contour">Contours</a>.

### Return Value

### Example

<div><fiddle-embed name="da34f02e69ec98d5681300aea9a2d0bf">#### Example Output

~~~~
empty bounds = 0, 0, 0, 0
circle bounds = 25, 20, 75, 70
rotated circle bounds = 25, 20, 75, 70
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#getBounds">getBounds</a>

---

<a name="conservativelyContainsRect"></a>

## conservativelyContainsRect

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool conservativelyContainsRect(const SkRect& rect) const
</pre>

Returns true if <a href="bmh_SkPath?cl=9919#rect">rect</a> is contained by <a href="bmh_SkPath?cl=9919#Path">Path</a>. 
May return false when <a href="bmh_SkPath?cl=9919#rect">rect</a> is contained by <a href="bmh_SkPath?cl=9919#Path">Path</a>.

For now, only returns true if <a href="bmh_SkPath?cl=9919#Path">Path</a> has one <a href="bmh_SkPath?cl=9919#Contour">Contour</a> and is convex.
<a href="bmh_SkPath?cl=9919#rect">rect</a> may share points and edges with <a href="bmh_SkPath?cl=9919#Path">Path</a> and be contained.
If <a href="bmh_SkPath?cl=9919#rect">rect</a> is empty, that is, it has zero width or height; <a href="bmh_SkPath?cl=9919#conservativelyContainsRect">conservativelyContainsRect</a>
returns true if the <a href="bmh_undocumented?cl=9919#Point">Point</a> or <a href="bmh_undocumented?cl=9919#Line">Line</a> described by <a href="bmh_SkPath?cl=9919#rect">rect</a> is contained by <a href="bmh_SkPath?cl=9919#Path">Path</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>rect </strong></code></td> <td><a href="bmh_undocumented?cl=9919#Rect">Rect</a>, <a href="bmh_undocumented?cl=9919#Line">Line</a>, or <a href="bmh_undocumented?cl=9919#Point">Point</a> checked for containment.</td>
  </tr>
</table>

### Return Value

true if <a href="bmh_SkPath?cl=9919#rect">rect</a> is contained.

### Example

<div><fiddle-embed name="41638d13e40fa449ece354dde5fb1941"><p><a href="bmh_undocumented?cl=9919#Rect">Rect</a> is drawn in blue if it is contained by red <a href="bmh_SkPath?cl=9919#Path">Path</a>.</p></fiddle-embed></div>

### See Also

Op <a href="bmh_undocumented?cl=9919#Rect">Rect</a> <a href="bmh_SkPath?cl=9919#Convexity">Convexity</a>

---

<a name="incReserve"></a>

## incReserve

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void incReserve(unsigned extraPtCount)
</pre>

grows <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a> and <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a> to contain <a href="bmh_SkPath?cl=9919#extraPtCount">extraPtCount</a> additional <a href="bmh_undocumented?cl=9919#Point">Points</a>.
<a href="bmh_SkPath?cl=9919#incReserve">incReserve</a> may improve performance and use less memory by
reducing the number and size of allocations when creating <a href="bmh_SkPath?cl=9919#Path">Path</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>extraPtCount </strong></code></td> <td>number of additional <a href="bmh_undocumented?cl=9919#Point">Points</a> to preallocate.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f2260f2a170a54aef5bafe5b91c121b3"></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>

---

<a name="moveTo"></a>

## moveTo

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void moveTo(SkScalar x, SkScalar y)
</pre>

Adds beginning of <a href="bmh_SkPath?cl=9919#Contour">Contour</a> at <a href="bmh_undocumented?cl=9919#Point">Point</a> (<a href="bmh_SkPath?cl=9919#x">x</a>, <a href="bmh_SkPath?cl=9919#y">y</a>).

### Parameters

<table>
  <tr>
    <td><code><strong>x </strong></code></td> <td>x-coordinate of <a href="bmh_SkPath?cl=9919#Contour">Contour</a> start.</td>
  </tr>
  <tr>
    <td><code><strong>y </strong></code></td> <td>y-coordinate of <a href="bmh_SkPath?cl=9919#Contour">Contour</a> start.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="84101d341e934a535a41ad6cf42218ce"></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#Contour">Contour</a> <a href="bmh_SkPath?cl=9919#lineTo">lineTo</a> <a href="bmh_SkPath?cl=9919#rMoveTo">rMoveTo</a> <a href="bmh_SkPath?cl=9919#quadTo">quadTo</a> <a href="bmh_SkPath?cl=9919#conicTo">conicTo</a> <a href="bmh_SkPath?cl=9919#cubicTo">cubicTo</a> <a href="bmh_SkPath?cl=9919#close">close</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void moveTo(const SkPoint& p)
</pre>

Adds beginning of <a href="bmh_SkPath?cl=9919#Contour">Contour</a> at <a href="bmh_undocumented?cl=9919#Point">Point</a> <a href="bmh_SkPath?cl=9919#p">p</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>p </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Contour">Contour</a> start.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="53b351d3fac667a4803418238e44a593"></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#Contour">Contour</a> <a href="bmh_SkPath?cl=9919#lineTo">lineTo</a> <a href="bmh_SkPath?cl=9919#rMoveTo">rMoveTo</a> <a href="bmh_SkPath?cl=9919#quadTo">quadTo</a> <a href="bmh_SkPath?cl=9919#conicTo">conicTo</a> <a href="bmh_SkPath?cl=9919#cubicTo">cubicTo</a> <a href="bmh_SkPath?cl=9919#close">close</a>

---

<a name="rMoveTo"></a>

## rMoveTo

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void rMoveTo(SkScalar dx, SkScalar dy)
</pre>

Adds beginning of <a href="bmh_SkPath?cl=9919#Contour">Contour</a> relative to <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a>.
If <a href="bmh_SkPath?cl=9919#Path">Path</a> is empty, starts <a href="bmh_SkPath?cl=9919#Contour">Contour</a> at (<a href="bmh_SkPath?cl=9919#dx">dx</a>, <a href="bmh_SkPath?cl=9919#dy">dy</a>).
Otherwise, start <a href="bmh_SkPath?cl=9919#Contour">Contour</a> at <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> offset by (<a href="bmh_SkPath?cl=9919#dx">dx</a>, <a href="bmh_SkPath?cl=9919#dy">dy</a>). 
<a href="bmh_SkPath?cl=9919#rMoveTo">rMoveTo</a> stands for relative move to.

### Parameters

<table>
  <tr>
    <td><code><strong>dx </strong></code></td> <td>offset from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> x to <a href="bmh_SkPath?cl=9919#Contour">Contour</a> start x.</td>
  </tr>
  <tr>
    <td><code><strong>dy </strong></code></td> <td>offset from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> y to <a href="bmh_SkPath?cl=9919#Contour">Contour</a> start y.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="63e32dec4b2d8440b427f368bf8313a4"></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#Contour">Contour</a> <a href="bmh_SkPath?cl=9919#lineTo">lineTo</a> <a href="bmh_SkPath?cl=9919#moveTo">moveTo</a> <a href="bmh_SkPath?cl=9919#quadTo">quadTo</a> <a href="bmh_SkPath?cl=9919#conicTo">conicTo</a> <a href="bmh_SkPath?cl=9919#cubicTo">cubicTo</a> <a href="bmh_SkPath?cl=9919#close">close</a>

---

<a name="lineTo"></a>

## lineTo

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void lineTo(SkScalar x, SkScalar y)
</pre>

Adds <a href="bmh_undocumented?cl=9919#Line">Line</a> from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> to (<a href="bmh_SkPath?cl=9919#x">x</a>,<a href="bmh_SkPath?cl=9919#y">y</a>). If <a href="bmh_SkPath?cl=9919#Path">Path</a> is empty, or last <a href="bmh_SkPath?cl=9919#Verb">Verb</a> is
<a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a>, <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> is set to (0, 0) before adding <a href="bmh_undocumented?cl=9919#Line">Line</a>.

<a href="bmh_SkPath?cl=9919#lineTo">lineTo</a> appends <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a> to <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a> and (0, 0) to <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>, if needed.
<a href="bmh_SkPath?cl=9919#lineTo">lineTo</a> then appends <a href="bmh_SkPath?cl=9919#kLine_Verb">kLine Verb</a> to <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a> and (<a href="bmh_SkPath?cl=9919#x">x</a>, <a href="bmh_SkPath?cl=9919#y">y</a>) to <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>.
 
### Parameters

<table>
  <tr>
    <td><code><strong>x </strong></code></td> <td>end of added <a href="bmh_undocumented?cl=9919#Line">Line</a> in <a href="bmh_SkPath?cl=9919#x">x</a>.</td>
  </tr>
  <tr>
    <td><code><strong>y </strong></code></td> <td>end of added <a href="bmh_undocumented?cl=9919#Line">Line</a> in <a href="bmh_SkPath?cl=9919#y">y</a>.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c4766d9f2ebe69dee895a6e4f6c7153b"></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#Contour">Contour</a> <a href="bmh_SkPath?cl=9919#moveTo">moveTo</a> <a href="bmh_SkPath?cl=9919#rLineTo">rLineTo</a> <a href="bmh_SkPath?cl=9919#addRect">addRect</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void lineTo(const SkPoint& p)
</pre>

Adds <a href="bmh_undocumented?cl=9919#Line">Line</a> from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> to <a href="bmh_undocumented?cl=9919#Point">Point</a> <a href="bmh_SkPath?cl=9919#p">p</a>. If <a href="bmh_SkPath?cl=9919#Path">Path</a> is empty, or last <a href="bmh_SkPath?cl=9919#Verb">Verb</a> is
<a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a>, <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> is set to (0, 0) before adding <a href="bmh_undocumented?cl=9919#Line">Line</a>.

<a href="bmh_SkPath?cl=9919#lineTo">lineTo</a> first appends <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a> to <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a> and (0, 0) to <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>, if needed.
<a href="bmh_SkPath?cl=9919#lineTo">lineTo</a> then appends <a href="bmh_SkPath?cl=9919#kLine_Verb">kLine Verb</a> to <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a> and <a href="bmh_undocumented?cl=9919#Point">Point</a> <a href="bmh_SkPath?cl=9919#p">p</a> to <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>p </strong></code></td> <td>end <a href="bmh_undocumented?cl=9919#Point">Point</a> of added <a href="bmh_undocumented?cl=9919#Line">Line</a>.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="41001546a7f7927d08e5a818bcc304f5"></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#Contour">Contour</a> <a href="bmh_SkPath?cl=9919#moveTo">moveTo</a> <a href="bmh_SkPath?cl=9919#rLineTo">rLineTo</a> <a href="bmh_SkPath?cl=9919#addRect">addRect</a>

---

<a name="rLineTo"></a>

## rLineTo

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void rLineTo(SkScalar dx, SkScalar dy)
</pre>

Adds <a href="bmh_undocumented?cl=9919#Line">Line</a> from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> to <a href="bmh_undocumented?cl=9919#Vector">Vector</a> (<a href="bmh_SkPath?cl=9919#dx">dx</a>, <a href="bmh_SkPath?cl=9919#dy">dy</a>). If <a href="bmh_SkPath?cl=9919#Path">Path</a> is empty, or last <a href="bmh_SkPath?cl=9919#Verb">Verb</a> is
<a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a>, <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> is set to (0, 0) before adding <a href="bmh_undocumented?cl=9919#Line">Line</a>.

<a href="bmh_SkPath?cl=9919#rLineTo">rLineTo</a> first appends <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a> to <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a> and (0, 0) to <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>, if needed.
<a href="bmh_SkPath?cl=9919#rLineTo">rLineTo</a> then appends <a href="bmh_SkPath?cl=9919#kLine_Verb">kLine Verb</a> to <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a> and <a href="bmh_undocumented?cl=9919#Line">Line</a> end to <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>.
<a href="bmh_undocumented?cl=9919#Line">Line</a> end is <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> plus <a href="bmh_undocumented?cl=9919#Vector">Vector</a> (<a href="bmh_SkPath?cl=9919#dx">dx</a>, <a href="bmh_SkPath?cl=9919#dy">dy</a>).
<a href="bmh_SkPath?cl=9919#rLineTo">rLineTo</a> stands for relative line to.

### Parameters

<table>
  <tr>
    <td><code><strong>dx </strong></code></td> <td>offset from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> x to <a href="bmh_undocumented?cl=9919#Line">Line</a> end x.</td>
  </tr>
  <tr>
    <td><code><strong>dy </strong></code></td> <td>offset from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> y to <a href="bmh_undocumented?cl=9919#Line">Line</a> end y.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="599374a59b6ffefeb583c08e20654bf3"></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#Contour">Contour</a> <a href="bmh_SkPath?cl=9919#moveTo">moveTo</a> <a href="bmh_SkPath?cl=9919#lineTo">lineTo</a> <a href="bmh_SkPath?cl=9919#addRect">addRect</a>

---

# <a name="Quad"></a> Quad
<a href="bmh_SkPath?cl=9919#Quad">Quad</a> describes a quadratic <a href="bmh_undocumented?cl=9919#Bezier">Bezier</a>, a second-order curve identical to a section
of a parabola. <a href="bmh_SkPath?cl=9919#Quad">Quad</a> begins at a start <a href="bmh_undocumented?cl=9919#Point">Point</a>, curves towards a control <a href="bmh_undocumented?cl=9919#Point">Point</a>,
and then curves to an end <a href="bmh_undocumented?cl=9919#Point">Point</a>.

### Example

<div><fiddle-embed name="bdc84127d6c6a2252bc8fd10812f8dbb"></fiddle-embed></div>

<a href="bmh_SkPath?cl=9919#Quad">Quad</a> is a special case of <a href="bmh_SkPath?cl=9919#Conic">Conic</a> where <a href="bmh_SkPath?cl=9919#Conic_Weight">Conic Weight</a> is set to one.

<a href="bmh_SkPath?cl=9919#Quad">Quad</a> is always contained by the triangle connecting its three <a href="bmh_undocumented?cl=9919#Point">Points</a>. <a href="bmh_SkPath?cl=9919#Quad">Quad</a>
begins tangent to the line between start <a href="bmh_undocumented?cl=9919#Point">Point</a> and control <a href="bmh_undocumented?cl=9919#Point">Point</a>, and ends
tangent to the line between control <a href="bmh_undocumented?cl=9919#Point">Point</a> and end <a href="bmh_undocumented?cl=9919#Point">Point</a>.

### Example

<div><fiddle-embed name="66b7cbae115958801383cb11013166ee"></fiddle-embed></div>

<a name="quadTo"></a>

## quadTo

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void quadTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2)
</pre>

Adds <a href="bmh_SkPath?cl=9919#Quad">Quad</a> from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> towards (<a href="bmh_SkPath?cl=9919#x1">x1</a>,<a href="bmh_SkPath?cl=9919#y1">y1</a>), to (<a href="bmh_SkPath?cl=9919#x2">x2</a>,<a href="bmh_SkPath?cl=9919#y2">y2</a>). 
If <a href="bmh_SkPath?cl=9919#Path">Path</a> is empty, or last <a href="bmh_SkPath?cl=9919#Verb">Verb</a> is <a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a>, <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> is set to (0, 0)
before adding <a href="bmh_SkPath?cl=9919#Quad">Quad</a>.

<a href="bmh_SkPath?cl=9919#quadTo">quadTo</a> appends <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a> to <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a> and (0, 0) to <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>, if needed.
<a href="bmh_SkPath?cl=9919#quadTo">quadTo</a> then appends <a href="bmh_SkPath?cl=9919#kQuad_Verb">kQuad Verb</a> to <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a>; and (<a href="bmh_SkPath?cl=9919#x1">x1</a>, <a href="bmh_SkPath?cl=9919#y1">y1</a>), (<a href="bmh_SkPath?cl=9919#x2">x2</a>, <a href="bmh_SkPath?cl=9919#y2">y2</a>)
to <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>x1 </strong></code></td> <td>control <a href="bmh_undocumented?cl=9919#Point">Point</a> of <a href="bmh_SkPath?cl=9919#Quad">Quad</a> in x.</td>
  </tr>
  <tr>
    <td><code><strong>y1 </strong></code></td> <td>control <a href="bmh_undocumented?cl=9919#Point">Point</a> of <a href="bmh_SkPath?cl=9919#Quad">Quad</a> in y.</td>
  </tr>
  <tr>
    <td><code><strong>x2 </strong></code></td> <td>end <a href="bmh_undocumented?cl=9919#Point">Point</a> of <a href="bmh_SkPath?cl=9919#Quad">Quad</a> in x.</td>
  </tr>
  <tr>
    <td><code><strong>y2 </strong></code></td> <td>end <a href="bmh_undocumented?cl=9919#Point">Point</a> of <a href="bmh_SkPath?cl=9919#Quad">Quad</a> in y.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="59d11a5a4de72f3b116b9a5b820ace68"></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#Contour">Contour</a> <a href="bmh_SkPath?cl=9919#moveTo">moveTo</a> <a href="bmh_SkPath?cl=9919#conicTo">conicTo</a> <a href="bmh_SkPath?cl=9919#rQuadTo">rQuadTo</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void quadTo(const SkPoint& p1, const SkPoint& p2)
</pre>

Adds <a href="bmh_SkPath?cl=9919#Quad">Quad</a> from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> towards <a href="bmh_undocumented?cl=9919#Point">Point</a> <a href="bmh_SkPath?cl=9919#p1">p1</a>, to <a href="bmh_undocumented?cl=9919#Point">Point</a> <a href="bmh_SkPath?cl=9919#p2">p2</a>. 
If <a href="bmh_SkPath?cl=9919#Path">Path</a> is empty, or last <a href="bmh_SkPath?cl=9919#Verb">Verb</a> is <a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a>, <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> is set to (0, 0)
before adding <a href="bmh_SkPath?cl=9919#Quad">Quad</a>.

<a href="bmh_SkPath?cl=9919#quadTo">quadTo</a> appends <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a> to <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a> and (0, 0) to <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>, if needed.
<a href="bmh_SkPath?cl=9919#quadTo">quadTo</a> then appends <a href="bmh_SkPath?cl=9919#kQuad_Verb">kQuad Verb</a> to <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a>; and <a href="bmh_undocumented?cl=9919#Point">Points</a> <a href="bmh_SkPath?cl=9919#p1">p1</a>, <a href="bmh_SkPath?cl=9919#p2">p2</a>
to <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>p1 </strong></code></td> <td>control <a href="bmh_undocumented?cl=9919#Point">Point</a> of added <a href="bmh_SkPath?cl=9919#Quad">Quad</a>.</td>
  </tr>
  <tr>
    <td><code><strong>p2 </strong></code></td> <td>end <a href="bmh_undocumented?cl=9919#Point">Point</a> of added <a href="bmh_SkPath?cl=9919#Quad">Quad</a>.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="9e8b422e63b2f9841b58a5a328840881"></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#Contour">Contour</a> <a href="bmh_SkPath?cl=9919#moveTo">moveTo</a> <a href="bmh_SkPath?cl=9919#conicTo">conicTo</a> <a href="bmh_SkPath?cl=9919#rQuadTo">rQuadTo</a>

---

<a name="rQuadTo"></a>

## rQuadTo

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void rQuadTo(SkScalar dx1, SkScalar dy1, SkScalar dx2, SkScalar dy2)
</pre>

Adds <a href="bmh_SkPath?cl=9919#Quad">Quad</a> from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> towards <a href="bmh_undocumented?cl=9919#Vector">Vector</a> (<a href="bmh_SkPath?cl=9919#dx1">dx1</a>, <a href="bmh_SkPath?cl=9919#dy1">dy1</a>), to <a href="bmh_undocumented?cl=9919#Vector">Vector</a> (<a href="bmh_SkPath?cl=9919#dx2">dx2</a>, <a href="bmh_SkPath?cl=9919#dy2">dy2</a>).
If <a href="bmh_SkPath?cl=9919#Path">Path</a> is empty, or last <a href="bmh_SkPath?cl=9919#Verb">Verb</a>
is <a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a>, <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> is set to (0, 0) before adding <a href="bmh_SkPath?cl=9919#Quad">Quad</a>.

<a href="bmh_SkPath?cl=9919#rQuadTo">rQuadTo</a> first appends <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a> to <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a> and (0, 0) to <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>,
if needed. <a href="bmh_SkPath?cl=9919#rQuadTo">rQuadTo</a> then appends <a href="bmh_SkPath?cl=9919#kQuad_Verb">kQuad Verb</a> to <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a>; and appends <a href="bmh_SkPath?cl=9919#Quad">Quad</a>
control and <a href="bmh_SkPath?cl=9919#Quad">Quad</a> end to <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>.
<a href="bmh_SkPath?cl=9919#Quad">Quad</a> control is <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> plus <a href="bmh_undocumented?cl=9919#Vector">Vector</a> (<a href="bmh_SkPath?cl=9919#dx1">dx1</a>, <a href="bmh_SkPath?cl=9919#dy1">dy1</a>).
<a href="bmh_SkPath?cl=9919#Quad">Quad</a> end is <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> plus <a href="bmh_undocumented?cl=9919#Vector">Vector</a> (<a href="bmh_SkPath?cl=9919#dx2">dx2</a>, <a href="bmh_SkPath?cl=9919#dy2">dy2</a>).
<a href="bmh_SkPath?cl=9919#rQuadTo">rQuadTo</a> stands for relative quad to.

### Parameters

<table>
  <tr>
    <td><code><strong>dx1 </strong></code></td> <td>offset from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> x to <a href="bmh_SkPath?cl=9919#Quad">Quad</a> control x.</td>
  </tr>
  <tr>
    <td><code><strong>dy1 </strong></code></td> <td>offset from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> x to <a href="bmh_SkPath?cl=9919#Quad">Quad</a> control y.</td>
  </tr>
  <tr>
    <td><code><strong>dx2 </strong></code></td> <td>offset from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> x to <a href="bmh_SkPath?cl=9919#Quad">Quad</a> end x.</td>
  </tr>
  <tr>
    <td><code><strong>dy2 </strong></code></td> <td>offset from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> x to <a href="bmh_SkPath?cl=9919#Quad">Quad</a> end y.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="704483d3e4ddc66c39f055771d85ecd6"></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#Contour">Contour</a> <a href="bmh_SkPath?cl=9919#moveTo">moveTo</a> <a href="bmh_SkPath?cl=9919#conicTo">conicTo</a> <a href="bmh_SkPath?cl=9919#quadTo">quadTo</a>

---

# <a name="Conic"></a> Conic
<a href="bmh_SkPath?cl=9919#Conic">Conic</a> describes a conical section: a piece of an ellipse, or a piece of a
parabola, or a piece of a hyperbola. <a href="bmh_SkPath?cl=9919#Conic">Conic</a> begins at a start <a href="bmh_undocumented?cl=9919#Point">Point</a>, 
curves towards a control <a href="bmh_undocumented?cl=9919#Point">Point</a>, and then curves to an end <a href="bmh_undocumented?cl=9919#Point">Point</a>. The influence
of the control <a href="bmh_undocumented?cl=9919#Point">Point</a> is determined by <a href="bmh_SkPath?cl=9919#Conic_Weight">Conic Weight</a>.

## <a name="Weight"></a> Weight

<a href="bmh_SkPath?cl=9919#Conic_Weight">Weight</a> determines both the strength of the control <a href="bmh_undocumented?cl=9919#Point">Point</a> and the class of <a href="bmh_SkPath?cl=9919#Conic">Conic</a>.
If <a href="bmh_SkPath?cl=9919#Conic_Weight">Weight</a> is exactly one, then <a href="bmh_SkPath?cl=9919#Conic">Conic</a> is identical to <a href="bmh_SkPath?cl=9919#Quad">Quad</a>; it is always a
parabolic segment. 

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

If weight is less than one, <a href="bmh_SkPath?cl=9919#Conic">Conic</a> is an elliptical segment.

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

If weight is greater than one, <a href="bmh_SkPath?cl=9919#Conic">Conic</a> is a hyperbolic segment. As w gets large,
a hyperbolic segment can be approximated by straight lines connecting the
control <a href="bmh_undocumented?cl=9919#Point">Point</a> with the end <a href="bmh_undocumented?cl=9919#Point">Points</a>.

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

<a name="conicTo"></a>

## conicTo

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void conicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar w)
</pre>

Adds <a href="bmh_SkPath?cl=9919#Conic">Conic</a> from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> towards (<a href="bmh_SkPath?cl=9919#x1">x1</a>,<a href="bmh_SkPath?cl=9919#y1">y1</a>), to (<a href="bmh_SkPath?cl=9919#x2">x2</a>,<a href="bmh_SkPath?cl=9919#y2">y2</a>), weighted by <a href="bmh_SkPath?cl=9919#w">w</a>. 
If <a href="bmh_SkPath?cl=9919#Path">Path</a> is empty, or last <a href="bmh_SkPath?cl=9919#Verb">Verb</a> is <a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a>, <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> is set to (0, 0)
before adding <a href="bmh_SkPath?cl=9919#Conic">Conic</a>.

<a href="bmh_SkPath?cl=9919#conicTo">conicTo</a> appends <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a> to <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a> and (0, 0) to <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>, if needed.
<a href="bmh_SkPath?cl=9919#conicTo">conicTo</a> then appends <a href="bmh_SkPath?cl=9919#kConic_Verb">kConic Verb</a> to <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a>; and (<a href="bmh_SkPath?cl=9919#x1">x1</a>, <a href="bmh_SkPath?cl=9919#y1">y1</a>), (<a href="bmh_SkPath?cl=9919#x2">x2</a>, <a href="bmh_SkPath?cl=9919#y2">y2</a>)
to <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>; and <a href="bmh_SkPath?cl=9919#w">w</a> to <a href="bmh_SkPath?cl=9919#Weight">Weights</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>x1 </strong></code></td> <td>control <a href="bmh_undocumented?cl=9919#Point">Point</a> of <a href="bmh_SkPath?cl=9919#Conic">Conic</a> in x.</td>
  </tr>
  <tr>
    <td><code><strong>y1 </strong></code></td> <td>control <a href="bmh_undocumented?cl=9919#Point">Point</a> of <a href="bmh_SkPath?cl=9919#Conic">Conic</a> in y.</td>
  </tr>
  <tr>
    <td><code><strong>x2 </strong></code></td> <td>end <a href="bmh_undocumented?cl=9919#Point">Point</a> of <a href="bmh_SkPath?cl=9919#Conic">Conic</a> in x.</td>
  </tr>
  <tr>
    <td><code><strong>y2 </strong></code></td> <td>end <a href="bmh_undocumented?cl=9919#Point">Point</a> of <a href="bmh_SkPath?cl=9919#Conic">Conic</a> in y.</td>
  </tr>
  <tr>
    <td><code><strong>w </strong></code></td> <td>weight of added <a href="bmh_SkPath?cl=9919#Conic">Conic</a>.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void conicTo(const SkPoint& p1, const SkPoint& p2, SkScalar w)
</pre>

Adds <a href="bmh_SkPath?cl=9919#Conic">Conic</a> from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> towards <a href="bmh_undocumented?cl=9919#Point">Point</a> <a href="bmh_SkPath?cl=9919#p1">p1</a>, to <a href="bmh_undocumented?cl=9919#Point">Point</a> <a href="bmh_SkPath?cl=9919#p2">p2</a>, weighted by <a href="bmh_SkPath?cl=9919#w">w</a>. 
If <a href="bmh_SkPath?cl=9919#Path">Path</a> is empty, or last <a href="bmh_SkPath?cl=9919#Verb">Verb</a> is <a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a>, <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> is set to (0, 0)
before adding <a href="bmh_SkPath?cl=9919#Conic">Conic</a>.

<a href="bmh_SkPath?cl=9919#conicTo">conicTo</a> appends <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a> to <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a> and (0, 0) to <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>, if needed.
<a href="bmh_SkPath?cl=9919#conicTo">conicTo</a> then appends <a href="bmh_SkPath?cl=9919#kConic_Verb">kConic Verb</a> to <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a>; and <a href="bmh_undocumented?cl=9919#Point">Points</a> <a href="bmh_SkPath?cl=9919#p1">p1</a>, <a href="bmh_SkPath?cl=9919#p2">p2</a>
to <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>; and <a href="bmh_SkPath?cl=9919#w">w</a> to <a href="bmh_SkPath?cl=9919#Weight">Weights</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>p1 </strong></code></td> <td>control <a href="bmh_undocumented?cl=9919#Point">Point</a> of added <a href="bmh_SkPath?cl=9919#Conic">Conic</a>.</td>
  </tr>
  <tr>
    <td><code><strong>p2 </strong></code></td> <td>end <a href="bmh_undocumented?cl=9919#Point">Point</a> of added <a href="bmh_SkPath?cl=9919#Conic">Conic</a>.</td>
  </tr>
  <tr>
    <td><code><strong>w </strong></code></td> <td>weight of added <a href="bmh_SkPath?cl=9919#Conic">Conic</a>.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<a name="rConicTo"></a>

## rConicTo

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void rConicTo(SkScalar dx1, SkScalar dy1, SkScalar dx2, SkScalar dy2, SkScalar w)
</pre>

Adds <a href="bmh_SkPath?cl=9919#Conic">Conic</a> from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> towards <a href="bmh_undocumented?cl=9919#Vector">Vector</a> (<a href="bmh_SkPath?cl=9919#dx1">dx1</a>, <a href="bmh_SkPath?cl=9919#dy1">dy1</a>), to <a href="bmh_undocumented?cl=9919#Vector">Vector</a> (<a href="bmh_SkPath?cl=9919#dx2">dx2</a>, <a href="bmh_SkPath?cl=9919#dy2">dy2</a>),
weighted by <a href="bmh_SkPath?cl=9919#w">w</a>. If <a href="bmh_SkPath?cl=9919#Path">Path</a> is empty, or last <a href="bmh_SkPath?cl=9919#Verb">Verb</a>
is <a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a>, <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> is set to (0, 0) before adding <a href="bmh_SkPath?cl=9919#Conic">Conic</a>.

<a href="bmh_SkPath?cl=9919#rConicTo">rConicTo</a> first appends <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a> to <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a> and (0, 0) to <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>,
if needed. <a href="bmh_SkPath?cl=9919#rConicTo">rConicTo</a> then appends <a href="bmh_SkPath?cl=9919#kConic_Verb">kConic Verb</a> to <a href="bmh_SkPath?cl=9919#Verb_Array">Verb Array</a>; and appends <a href="bmh_SkPath?cl=9919#Conic">Conic</a>
control and <a href="bmh_SkPath?cl=9919#Conic">Conic</a> end to <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a>.
<a href="bmh_SkPath?cl=9919#Conic">Conic</a> control is <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> plus <a href="bmh_undocumented?cl=9919#Vector">Vector</a> (<a href="bmh_SkPath?cl=9919#dx1">dx1</a>, <a href="bmh_SkPath?cl=9919#dy1">dy1</a>).
<a href="bmh_SkPath?cl=9919#Conic">Conic</a> end is <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> plus <a href="bmh_undocumented?cl=9919#Vector">Vector</a> (<a href="bmh_SkPath?cl=9919#dx2">dx2</a>, <a href="bmh_SkPath?cl=9919#dy2">dy2</a>).
<a href="bmh_SkPath?cl=9919#Conic_Weight">Conic Weight</a> is set to <a href="bmh_SkPath?cl=9919#w">w</a>, which is not affected by the last weight.
<a href="bmh_SkPath?cl=9919#rConicTo">rConicTo</a> stands for relative conic to.

### Parameters

<table>
  <tr>
    <td><code><strong>dx1 </strong></code></td> <td>offset from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> x to <a href="bmh_SkPath?cl=9919#Conic">Conic</a> control x.</td>
  </tr>
  <tr>
    <td><code><strong>dy1 </strong></code></td> <td>offset from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> x to <a href="bmh_SkPath?cl=9919#Conic">Conic</a> control y.</td>
  </tr>
  <tr>
    <td><code><strong>dx2 </strong></code></td> <td>offset from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> x to <a href="bmh_SkPath?cl=9919#Conic">Conic</a> end x.</td>
  </tr>
  <tr>
    <td><code><strong>dy2 </strong></code></td> <td>offset from <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> x to <a href="bmh_SkPath?cl=9919#Conic">Conic</a> end y.</td>
  </tr>
  <tr>
    <td><code><strong>w </strong></code></td> <td>weight of added <a href="bmh_SkPath?cl=9919#Conic">Conic</a>.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

# <a name="Cubic"></a> Cubic

<a name="cubicTo"></a>

## cubicTo

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void cubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar x3,
             SkScalar y3)
</pre>

Add a cubic bezier from the last point, approaching control points
(<a href="bmh_SkPath?cl=9919#x1">x1</a>,<a href="bmh_SkPath?cl=9919#y1">y1</a>) and (<a href="bmh_SkPath?cl=9919#x2">x2</a>,<a href="bmh_SkPath?cl=9919#y2">y2</a>), and ending at (<a href="bmh_SkPath?cl=9919#x3">x3</a>,<a href="bmh_SkPath?cl=9919#y3">y3</a>). If no

### Parameters

<table>
  <tr>
    <td><code><strong>x1 </strong></code></td> <td>The x-coordinate of the 1st control point on a cubic curve</td>
  </tr>
  <tr>
    <td><code><strong>y1 </strong></code></td> <td>The y-coordinate of the 1st control point on a cubic curve</td>
  </tr>
  <tr>
    <td><code><strong>x2 </strong></code></td> <td>The x-coordinate of the 2nd control point on a cubic curve</td>
  </tr>
  <tr>
    <td><code><strong>y2 </strong></code></td> <td>The y-coordinate of the 2nd control point on a cubic curve</td>
  </tr>
  <tr>
    <td><code><strong>x3 </strong></code></td> <td>The x-coordinate of the end point on a cubic curve</td>
  </tr>
  <tr>
    <td><code><strong>y3 </strong></code></td> <td>The y-coordinate of the end point on a cubic curve</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void cubicTo(const SkPoint& p1, const SkPoint& p2, const SkPoint& p3)
</pre>

Add a cubic bezier from the last point, approaching control points <a href="bmh_SkPath?cl=9919#p1">p1</a>
and <a href="bmh_SkPath?cl=9919#p2">p2</a>, and ending at <a href="bmh_SkPath?cl=9919#p3">p3</a>. If no

### Parameters

<table>
  <tr>
    <td><code><strong>p1 </strong></code></td> <td>The 1st control point on a cubic curve</td>
  </tr>
  <tr>
    <td><code><strong>p2 </strong></code></td> <td>The 2nd control point on a cubic curve</td>
  </tr>
  <tr>
    <td><code><strong>p3 </strong></code></td> <td>The end point on a cubic curve</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<a name="rCubicTo"></a>

## rCubicTo

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void rCubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar x3,
              SkScalar y3)
</pre>

Same as <a href="bmh_SkPath?cl=9919#cubicTo">cubicTo</a>, but the coordinates are considered relative to the
current point on this contour. If there is no previous point, then a

### Parameters

<table>
  <tr>
    <td><code><strong>x1 </strong></code></td> <td>The amount to add to the x-coordinate of the last point on</td>
  </tr>
  <tr>
    <td><code><strong>y1 </strong></code></td> <td>The amount to add to the y-coordinate of the last point on</td>
  </tr>
  <tr>
    <td><code><strong>x2 </strong></code></td> <td>The amount to add to the x-coordinate of the last point on</td>
  </tr>
  <tr>
    <td><code><strong>y2 </strong></code></td> <td>The amount to add to the y-coordinate of the last point on</td>
  </tr>
  <tr>
    <td><code><strong>x3 </strong></code></td> <td>The amount to add to the x-coordinate of the last point on</td>
  </tr>
  <tr>
    <td><code><strong>y3 </strong></code></td> <td>The amount to add to the y-coordinate of the last point on</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

# <a name="Arc"></a> Arc
<a href="bmh_SkPath?cl=9919#Arc">Arc</a> can be constructed in a number of ways. <a href="bmh_SkPath?cl=9919#Arc">Arc</a> may be described by part of <a href="bmh_undocumented?cl=9919#Oval">Oval</a> and angles,
by start point and end point, and by radius and tangent lines. Each construction has advantages,
and some constructions correspond to <a href="bmh_SkPath?cl=9919#Arc">Arc</a> drawing in graphics standards.

All <a href="bmh_SkPath?cl=9919#Arc">Arc</a> draws are implemented by one or more <a href="bmh_SkPath?cl=9919#Conic">Conic</a> draws. When <a href="bmh_SkPath?cl=9919#Conic_Weight">Conic Weight</a> is less than one,
<a href="bmh_SkPath?cl=9919#Conic">Conic</a> describes an <a href="bmh_SkPath?cl=9919#Arc">Arc</a> of some <a href="bmh_undocumented?cl=9919#Oval">Oval</a> or <a href="bmh_undocumented?cl=9919#Circle">Circle</a>.

<a href="bmh_SkPath?cl=9919#arcTo">arcTo(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle, bool forceMoveTo)</a>
describes <a href="bmh_SkPath?cl=9919#Arc">Arc</a> as a piece of <a href="bmh_undocumented?cl=9919#Oval">Oval</a>, beginning at start angle, sweeping clockwise or counterclockwise,
which may continue <a href="bmh_SkPath?cl=9919#Contour">Contour</a> or start a new one. This construction is similar to <a href="bmh_undocumented?cl=9919#PostScript">PostScript</a> and 
<a href="bmh_undocumented?cl=9919#HTML_Canvas">HTML Canvas</a> arcs. Variation <a href="bmh_SkPath?cl=9919#addArc">addArc</a> always starts new <a href="bmh_SkPath?cl=9919#Contour">Contour</a>. Canvas::drawArc draws without
requiring <a href="bmh_SkPath?cl=9919#Path">Path</a>.

<a href="bmh_SkPath?cl=9919#arcTo_2">arcTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar radius)</a>
describes <a href="bmh_SkPath?cl=9919#Arc">Arc</a> as tangent to the line (x0, y0), (x1, y1) and tangent to the line (x1, y1), (x2, y2)
where (x0, y0) is the last <a href="bmh_undocumented?cl=9919#Point">Point</a> added to <a href="bmh_SkPath?cl=9919#Path">Path</a>. This construction is similar to <a href="bmh_undocumented?cl=9919#PostScript">PostScript</a> and
<a href="bmh_undocumented?cl=9919#HTML_Canvas">HTML Canvas</a> arcs.

<a href="bmh_SkPath?cl=9919#arcTo_4">arcTo(SkScalar rx, SkScalar ry, SkScalar xAxisRotate, ArcSize largeArc, Direction sweep,
      SkScalar x, SkScalar y)</a> 
describes <a href="bmh_SkPath?cl=9919#Arc">Arc</a> as part of <a href="bmh_undocumented?cl=9919#Oval">Oval</a> with radii (rx, ry), beginning at
last <a href="bmh_undocumented?cl=9919#Point">Point</a> added to <a href="bmh_SkPath?cl=9919#Path">Path</a> and ending at (x, y). More than one <a href="bmh_SkPath?cl=9919#Arc">Arc</a> satisfies this criteria,
so additional values choose a single solution. This construction is similar to <a href="bmh_undocumented?cl=9919#SVG">SVG</a> arcs.

<a href="bmh_SkPath?cl=9919#conicTo">conicTo</a> describes <a href="bmh_SkPath?cl=9919#Arc">Arc</a> of less than 180 degrees as a pair of tangent lines and <a href="bmh_SkPath?cl=9919#Conic_Weight">Conic Weight</a>.
<a href="bmh_SkPath?cl=9919#conicTo">conicTo</a> can represent any <a href="bmh_SkPath?cl=9919#Arc">Arc</a> with a sweep less than 180 degrees at any rotation. All <a href="bmh_SkPath?cl=9919#arcTo">arcTo</a>
constructions are converted to <a href="bmh_SkPath?cl=9919#Conic">Conic</a> data when added to <a href="bmh_SkPath?cl=9919#Path">Path</a>. 

### Example

<div><fiddle-embed name="fbbb044a09ecf6235f68ccdbb5ab6edd"><p><table>
  <tr>
    <td><sup>1</sup> <a href="bmh_SkPath?cl=9919#arcTo">arcTo(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle, bool forceMoveTo)</a></td>
  </tr>
  <tr>
    <td><sup>2</sup> parameter sets force MoveTo</td>
  </tr>
  <tr>
    <td><sup>3</sup> start angle must be multiple of 90 degrees.</td>
  </tr>
  <tr>
    <td><sup>4</sup> <a href="bmh_SkPath?cl=9919#arcTo_2">arcTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar radius)</a></td>
  </tr>
  <tr>
    <td><sup>5</sup> <a href="bmh_SkPath?cl=9919#arcTo_4">arcTo(SkScalar rx, SkScalar ry, SkScalar xAxisRotate, ArcSize largeArc,
               Direction sweep, SkScalar x, SkScalar y)</a></td>
  </tr>
</table>

</p></fiddle-embed></div>

### Example

<div><fiddle-embed name="c6889901eb9f32e671c18cde57c733d6"><p>1 describes an arc from an oval, a starting angle, and a sweep angle.
2 is similar to 1, but does not require building a path to draw.
3 is similar to 1, but always begins new <a href="bmh_SkPath?cl=9919#Contour">Contour</a>.
4 describes an arc from a pair of tangent lines and a radius.
5 describes an arc from <a href="bmh_undocumented?cl=9919#Oval">Oval</a> center, arc start <a href="bmh_undocumented?cl=9919#Point">Point</a> and arc end <a href="bmh_undocumented?cl=9919#Point">Point</a>.
6 describes an arc from a pair of tangent lines and a <a href="bmh_SkPath?cl=9919#Conic_Weight">Conic Weight</a>.</p></fiddle-embed></div>

<a name="arcTo"></a>

## arcTo

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void arcTo(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle,
           bool forceMoveTo)
</pre>

Append <a href="bmh_SkPath?cl=9919#Arc">Arc</a> to <a href="bmh_SkPath?cl=9919#Path">Path</a>. <a href="bmh_SkPath?cl=9919#Arc">Arc</a> added is part of ellipse
bounded by <a href="bmh_SkPath?cl=9919#oval">oval</a>, from <a href="bmh_SkPath?cl=9919#startAngle">startAngle</a> through <a href="bmh_SkPath?cl=9919#sweepAngle">sweepAngle</a>. Both <a href="bmh_SkPath?cl=9919#startAngle">startAngle</a> and
<a href="bmh_SkPath?cl=9919#sweepAngle">sweepAngle</a> are measured in degrees, where zero degrees is aligned with the
positive x-axis, and positive sweeps extends <a href="bmh_SkPath?cl=9919#Arc">Arc</a> clockwise.

<a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> adds <a href="bmh_undocumented?cl=9919#Line">Line</a> connecting <a href="bmh_SkPath?cl=9919#Path">Path</a> last <a href="bmh_undocumented?cl=9919#Point">Point</a> to initial <a href="bmh_SkPath?cl=9919#Arc">Arc</a> <a href="bmh_undocumented?cl=9919#Point">Point</a> if <a href="bmh_SkPath?cl=9919#forceMoveTo">forceMoveTo</a>
is false and <a href="bmh_SkPath?cl=9919#Path">Path</a> is not empty. Otherwise, added <a href="bmh_SkPath?cl=9919#Contour">Contour</a> begins with first point
of <a href="bmh_SkPath?cl=9919#Arc">Arc</a>. Angles greater than -360 and less than 360 are treated modulo 360.

### Parameters

<table>
  <tr>
    <td><code><strong>oval </strong></code></td> <td>bounds of ellipse containing <a href="bmh_SkPath?cl=9919#Arc">Arc</a>.</td>
  </tr>
  <tr>
    <td><code><strong>startAngle </strong></code></td> <td>starting angle of <a href="bmh_SkPath?cl=9919#Arc">Arc</a> in degrees.</td>
  </tr>
  <tr>
    <td><code><strong>sweepAngle </strong></code></td> <td>sweep, in degrees. Positive is clockwise; treated modulo 360.</td>
  </tr>
  <tr>
    <td><code><strong>forceMoveTo </strong></code></td> <td>true to start a new contour with <a href="bmh_SkPath?cl=9919#Arc">Arc</a>.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c6cd2e2a07fd231f6e9173809ebe44b8"><p><a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> continues a previous contour when <a href="bmh_SkPath?cl=9919#forceMoveTo">forceMoveTo</a> is false and when <a href="bmh_SkPath?cl=9919#Path">Path</a>
is not empty.</p></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#addArc">addArc</a> <a href="bmh_SkCanvas?cl=9919#drawArc">SkCanvas::drawArc</a> <a href="bmh_SkPath?cl=9919#conicTo">conicTo</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void arcTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar radius)
</pre>

Append <a href="bmh_SkPath?cl=9919#Arc">Arc</a> to <a href="bmh_SkPath?cl=9919#Path">Path</a>, after appending <a href="bmh_undocumented?cl=9919#Line">Line</a> if needed. <a href="bmh_SkPath?cl=9919#Arc">Arc</a> is implemented by <a href="bmh_SkPath?cl=9919#Conic">Conic</a>
weighted to describe part of <a href="bmh_undocumented?cl=9919#Circle">Circle</a>. <a href="bmh_SkPath?cl=9919#Arc">Arc</a> is contained by tangent from
last <a href="bmh_SkPath?cl=9919#Path">Path</a> point (x0, y0) to (<a href="bmh_SkPath?cl=9919#x1">x1</a>, <a href="bmh_SkPath?cl=9919#y1">y1</a>), and tangent from (<a href="bmh_SkPath?cl=9919#x1">x1</a>, <a href="bmh_SkPath?cl=9919#y1">y1</a>) to (<a href="bmh_SkPath?cl=9919#x2">x2</a>, <a href="bmh_SkPath?cl=9919#y2">y2</a>). <a href="bmh_SkPath?cl=9919#Arc">Arc</a>
is part of <a href="bmh_undocumented?cl=9919#Circle">Circle</a> sized to <a href="bmh_SkPath?cl=9919#radius">radius</a>, positioned so it touches both tangent lines. 

### Example

<div><fiddle-embed name="009b34d2a6165bb3e9a52e7dbb712c68"></fiddle-embed></div>

If last <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_undocumented?cl=9919#Point">Point</a> does not start <a href="bmh_SkPath?cl=9919#Arc">Arc</a>, <a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> appends connecting <a href="bmh_undocumented?cl=9919#Line">Line</a> to <a href="bmh_SkPath?cl=9919#Path">Path</a>.
The length of <a href="bmh_undocumented?cl=9919#Vector">Vector</a> from (<a href="bmh_SkPath?cl=9919#x1">x1</a>, <a href="bmh_SkPath?cl=9919#y1">y1</a>) to (<a href="bmh_SkPath?cl=9919#x2">x2</a>, <a href="bmh_SkPath?cl=9919#y2">y2</a>) does not affect <a href="bmh_SkPath?cl=9919#Arc">Arc</a>.

### Example

<div><fiddle-embed name="b6196ae54e5b9171c478185e5b7189e7"></fiddle-embed></div>

<a href="bmh_SkPath?cl=9919#Arc">Arc</a> sweep is always less than 180 degrees. If <a href="bmh_SkPath?cl=9919#radius">radius</a> is zero, or if
tangents are nearly parallel, <a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> appends <a href="bmh_undocumented?cl=9919#Line">Line</a> from last <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_undocumented?cl=9919#Point">Point</a> to (<a href="bmh_SkPath?cl=9919#x1">x1</a>, <a href="bmh_SkPath?cl=9919#y1">y1</a>).

<a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> appends at most one <a href="bmh_undocumented?cl=9919#Line">Line</a> and one <a href="bmh_SkPath?cl=9919#Conic">Conic</a>.
<a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> implements the functionality of <a href="bmh_undocumented?cl=9919#PostScript_arct">PostScript arct</a> and <a href="bmh_undocumented?cl=9919#HTML_Canvas_arcTo">HTML Canvas arcTo</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>x1 </strong></code></td> <td>x common to pair of tangents.</td>
  </tr>
  <tr>
    <td><code><strong>y1 </strong></code></td> <td>y common to pair of tangents.</td>
  </tr>
  <tr>
    <td><code><strong>x2 </strong></code></td> <td>x end of second tangent.</td>
  </tr>
  <tr>
    <td><code><strong>y2 </strong></code></td> <td>y end of second tangent.</td>
  </tr>
  <tr>
    <td><code><strong>radius </strong></code></td> <td>distance from <a href="bmh_SkPath?cl=9919#Arc">Arc</a> to <a href="bmh_undocumented?cl=9919#Circle">Circle</a> center.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="498360fa0a201cc5db04b1c27256358f"><p><a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> is represented by <a href="bmh_undocumented?cl=9919#Line">Line</a> and circular <a href="bmh_SkPath?cl=9919#Conic">Conic</a> in <a href="bmh_SkPath?cl=9919#Path">Path</a>.</p>

#### Example Output

~~~~
move to (156,20)
line (156,20),(79.2893,20)
conic (79.2893,20),(200,20),(114.645,105.355) weight 0.382683
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#conicTo">conicTo</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void arcTo(const SkPoint p1, const SkPoint p2, SkScalar radius)
</pre>

Append <a href="bmh_SkPath?cl=9919#Arc">Arc</a> to <a href="bmh_SkPath?cl=9919#Path">Path</a>, after appending <a href="bmh_undocumented?cl=9919#Line">Line</a> if needed. <a href="bmh_SkPath?cl=9919#Arc">Arc</a> is implemented by <a href="bmh_SkPath?cl=9919#Conic">Conic</a>
weighted to describe part of <a href="bmh_undocumented?cl=9919#Circle">Circle</a>. <a href="bmh_SkPath?cl=9919#Arc">Arc</a> is contained by tangent from
last <a href="bmh_SkPath?cl=9919#Path">Path</a> point to <a href="bmh_SkPath?cl=9919#p1">p1</a>, and tangent from <a href="bmh_SkPath?cl=9919#p1">p1</a> to <a href="bmh_SkPath?cl=9919#p2">p2</a>. <a href="bmh_SkPath?cl=9919#Arc">Arc</a>
is part of <a href="bmh_undocumented?cl=9919#Circle">Circle</a> sized to <a href="bmh_SkPath?cl=9919#radius">radius</a>, positioned so it touches both tangent lines. 

If last <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_undocumented?cl=9919#Point">Point</a> does not start <a href="bmh_SkPath?cl=9919#Arc">Arc</a>, <a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> appends connecting <a href="bmh_undocumented?cl=9919#Line">Line</a> to <a href="bmh_SkPath?cl=9919#Path">Path</a>.
The length of <a href="bmh_undocumented?cl=9919#Vector">Vector</a> from <a href="bmh_SkPath?cl=9919#p1">p1</a> to <a href="bmh_SkPath?cl=9919#p2">p2</a> does not affect <a href="bmh_SkPath?cl=9919#Arc">Arc</a>.

<a href="bmh_SkPath?cl=9919#Arc">Arc</a> sweep is always less than 180 degrees. If <a href="bmh_SkPath?cl=9919#radius">radius</a> is zero, or if
tangents are nearly parallel, <a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> appends <a href="bmh_undocumented?cl=9919#Line">Line</a> from last <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_undocumented?cl=9919#Point">Point</a> to <a href="bmh_SkPath?cl=9919#p1">p1</a>.

<a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> appends at most one <a href="bmh_undocumented?cl=9919#Line">Line</a> and one <a href="bmh_SkPath?cl=9919#Conic">Conic</a>.
<a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> implements the functionality of <a href="bmh_undocumented?cl=9919#PostScript_arct">PostScript arct</a> and <a href="bmh_undocumented?cl=9919#HTML_Canvas_arcTo">HTML Canvas arcTo</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>p1 </strong></code></td> <td><a href="bmh_undocumented?cl=9919#Point">Point</a> common to pair of tangents.</td>
  </tr>
  <tr>
    <td><code><strong>p2 </strong></code></td> <td>end of second tangent.</td>
  </tr>
  <tr>
    <td><code><strong>radius </strong></code></td> <td>distance from <a href="bmh_SkPath?cl=9919#Arc">Arc</a> to <a href="bmh_undocumented?cl=9919#Circle">Circle</a> center.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0c056264a361579c18e5d02d3172d4d4"><p>Because tangent lines are parallel, <a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> appends line from last <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_undocumented?cl=9919#Point">Point</a> to
<a href="bmh_SkPath?cl=9919#p1">p1</a>, but does not append a circular <a href="bmh_SkPath?cl=9919#Conic">Conic</a>.</p>

#### Example Output

~~~~
move to (156,20)
line (156,20),(200,20)
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#conicTo">conicTo</a>

---

## <a name="SkPath::ArcSize"></a> Enum SkPath::ArcSize

Four <a href="bmh_undocumented?cl=9919#Oval">Oval</a> parts with radii (rx, ry) start at last <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_undocumented?cl=9919#Point">Point</a> and ends at (x, y).
<a href="bmh_SkPath?cl=9919#ArcSize">ArcSize</a> and <a href="bmh_SkPath?cl=9919#Direction">Direction</a> select one of the four <a href="bmh_undocumented?cl=9919#Oval">Oval</a> parts.

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="bmh_SkPath?cl=9919#ArcSize">ArcSize</a> {
    <a href="bmh_SkPath?cl=9919#kSmall_ArcSize">kSmall ArcSize</a> 
    <a href="bmh_SkPath?cl=9919#kLarge_ArcSize">kLarge ArcSize</a> 
};</pre>

### Constants

<table>
  <tr>
    <td><a name="SkPath::kSmall_ArcSize"></a> <code><strong>SkPath::kSmall_ArcSize </strong></code></td><td>0</td><td>Smaller of <a href="bmh_SkPath?cl=9919#Arc">Arc</a> pair.</td>
  </tr>
  <tr>
    <td><a name="SkPath::kLarge_ArcSize"></a> <code><strong>SkPath::kLarge_ArcSize </strong></code></td><td>1</td><td>Larger of <a href="bmh_SkPath?cl=9919#Arc">Arc</a> pair.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3a02bae7f9764114154f9f2799649562"><p><a href="bmh_SkPath?cl=9919#Arc">Arc</a> begins at top of <a href="bmh_undocumented?cl=9919#Oval">Oval</a> pair and ends at bottom. <a href="bmh_SkPath?cl=9919#Arc">Arc</a> can take four routes to get there.
Two routes are large, and two routes are counterclockwise. The one route both large
and counterclockwise is blue.</p></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> <a href="bmh_SkPath?cl=9919#Direction">Direction</a>



<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void arcTo(SkScalar rx, SkScalar ry, SkScalar xAxisRotate, ArcSize largeArc,
           Direction sweep, SkScalar x, SkScalar y)
</pre>

Append <a href="bmh_SkPath?cl=9919#Arc">Arc</a> to <a href="bmh_SkPath?cl=9919#Path">Path</a>. <a href="bmh_SkPath?cl=9919#Arc">Arc</a> is implemented by one or more <a href="bmh_SkPath?cl=9919#Conic">Conic</a> weighted to describe part of <a href="bmh_undocumented?cl=9919#Oval">Oval</a>
with radii (<a href="bmh_SkPath?cl=9919#rx">rx</a>, <a href="bmh_SkPath?cl=9919#ry">ry</a>) rotated by <a href="bmh_SkPath?cl=9919#xAxisRotate">xAxisRotate</a> degrees. <a href="bmh_SkPath?cl=9919#Arc">Arc</a> curves from last <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_undocumented?cl=9919#Point">Point</a> to (<a href="bmh_SkPath?cl=9919#x">x</a>, <a href="bmh_SkPath?cl=9919#y">y</a>),
choosing one of four possible routes: clockwise or counterclockwise, and smaller or larger.

<a href="bmh_SkPath?cl=9919#Arc">Arc</a> <a href="bmh_SkPath?cl=9919#sweep">sweep</a> is always less than 360 degrees. <a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> appends <a href="bmh_undocumented?cl=9919#Line">Line</a> to (<a href="bmh_SkPath?cl=9919#x">x</a>, <a href="bmh_SkPath?cl=9919#y">y</a>) if either radii are zero,
or if last <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_undocumented?cl=9919#Point">Point</a> equals (<a href="bmh_SkPath?cl=9919#x">x</a>, <a href="bmh_SkPath?cl=9919#y">y</a>). <a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> scales radii (<a href="bmh_SkPath?cl=9919#rx">rx</a>, <a href="bmh_SkPath?cl=9919#ry">ry</a>) to fit last <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_undocumented?cl=9919#Point">Point</a> and
(<a href="bmh_SkPath?cl=9919#x">x</a>, <a href="bmh_SkPath?cl=9919#y">y</a>) if both are greater than zero but too small.

<a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> appends up to four <a href="bmh_SkPath?cl=9919#Conic">Conic</a> curves.
<a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> implements the functionatlity of <a href="bmh_undocumented?cl=9919#Arc">SVG Arc</a>, although <a href="bmh_undocumented?cl=9919#SVG">SVG</a> sweep-flag value is
opposite the integer value of <a href="bmh_SkPath?cl=9919#sweep">sweep</a>; <a href="bmh_undocumented?cl=9919#SVG">SVG</a> sweep-flag uses 1 for clockwise, while <a href="bmh_SkPath?cl=9919#kCW_Direction">kCW Direction</a> 
cast to int is zero.

### Parameters

<table>
  <tr>
    <td><code><strong>rx </strong></code></td> <td>radius in <a href="bmh_SkPath?cl=9919#x">x</a> before x-axis rotation.</td>
  </tr>
  <tr>
    <td><code><strong>ry </strong></code></td> <td>radius in <a href="bmh_SkPath?cl=9919#y">y</a> before x-axis rotation.</td>
  </tr>
  <tr>
    <td><code><strong>xAxisRotate </strong></code></td> <td>x-axis rotation in degrees; positve values are clockwise.</td>
  </tr>
  <tr>
    <td><code><strong>largeArc </strong></code></td> <td>chooses smaller or larger <a href="bmh_SkPath?cl=9919#Arc">Arc</a>.</td>
  </tr>
  <tr>
    <td><code><strong>sweep </strong></code></td> <td>chooses clockwise or counterclockwise <a href="bmh_SkPath?cl=9919#Arc">Arc</a>.</td>
  </tr>
  <tr>
    <td><code><strong>x </strong></code></td> <td>end of <a href="bmh_SkPath?cl=9919#Arc">Arc</a>.</td>
  </tr>
  <tr>
    <td><code><strong>y </strong></code></td> <td>end of <a href="bmh_SkPath?cl=9919#Arc">Arc</a>.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="aa638dc4e317ca01b0825d34a1c016d2"></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#rArcTo">rArcTo</a> <a href="bmh_SkPath?cl=9919#ArcSize">ArcSize</a> <a href="bmh_SkPath?cl=9919#Direction">Direction</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void arcTo(const SkPoint r, SkScalar xAxisRotate, ArcSize largeArc,
           Direction sweep, const SkPoint xy)
</pre>

Append <a href="bmh_SkPath?cl=9919#Arc">Arc</a> to <a href="bmh_SkPath?cl=9919#Path">Path</a>. <a href="bmh_SkPath?cl=9919#Arc">Arc</a> is implemented by one or more <a href="bmh_SkPath?cl=9919#Conic">Conic</a> weighted to describe part of <a href="bmh_undocumented?cl=9919#Oval">Oval</a>
with radii (<a href="bmh_SkPath?cl=9919#r">r</a>.fX, <a href="bmh_SkPath?cl=9919#r">r</a>.fY) rotated by <a href="bmh_SkPath?cl=9919#xAxisRotate">xAxisRotate</a> degrees. <a href="bmh_SkPath?cl=9919#Arc">Arc</a> curves from last <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_undocumented?cl=9919#Point">Point</a> to
(<a href="bmh_SkPath?cl=9919#xy">xy</a>.fX, <a href="bmh_SkPath?cl=9919#xy">xy</a>.fY), choosing one of four possible routes: clockwise or counterclockwise,
and smaller or larger.

<a href="bmh_SkPath?cl=9919#Arc">Arc</a> <a href="bmh_SkPath?cl=9919#sweep">sweep</a> is always less than 360 degrees. <a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> appends <a href="bmh_undocumented?cl=9919#Line">Line</a> to <a href="bmh_SkPath?cl=9919#xy">xy</a> if either radii are zero,
or if last <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_undocumented?cl=9919#Point">Point</a> equals (x, y). <a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> scales radii <a href="bmh_SkPath?cl=9919#r">r</a> to fit last <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_undocumented?cl=9919#Point">Point</a> and
<a href="bmh_SkPath?cl=9919#xy">xy</a> if both are greater than zero but too small.

<a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> appends up to four <a href="bmh_SkPath?cl=9919#Conic">Conic</a> curves.
<a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> implements the functionatlity of <a href="bmh_undocumented?cl=9919#Arc">SVG Arc</a>, although <a href="bmh_undocumented?cl=9919#SVG">SVG</a> sweep-flag value is
opposite the integer value of <a href="bmh_SkPath?cl=9919#sweep">sweep</a>; <a href="bmh_undocumented?cl=9919#SVG">SVG</a> sweep-flag uses 1 for clockwise, while <a href="bmh_SkPath?cl=9919#kCW_Direction">kCW Direction</a> 
cast to int is zero.

### Parameters

<table>
  <tr>
    <td><code><strong>r </strong></code></td> <td>radii in x and y before x-axis rotation.</td>
  </tr>
  <tr>
    <td><code><strong>xAxisRotate </strong></code></td> <td>x-axis rotation in degrees; positve values are clockwise.</td>
  </tr>
  <tr>
    <td><code><strong>largeArc </strong></code></td> <td>chooses smaller or larger <a href="bmh_SkPath?cl=9919#Arc">Arc</a>.</td>
  </tr>
  <tr>
    <td><code><strong>sweep </strong></code></td> <td>chooses clockwise or counterclockwise <a href="bmh_SkPath?cl=9919#Arc">Arc</a>.</td>
  </tr>
  <tr>
    <td><code><strong>xy </strong></code></td> <td>end of <a href="bmh_SkPath?cl=9919#Arc">Arc</a>.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="f8584ddb02078ceac39c841978400a40"></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#rArcTo">rArcTo</a> <a href="bmh_SkPath?cl=9919#ArcSize">ArcSize</a> <a href="bmh_SkPath?cl=9919#Direction">Direction</a>

---

<a name="rArcTo"></a>

## rArcTo

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void rArcTo(SkScalar rx, SkScalar ry, SkScalar xAxisRotate, ArcSize largeArc,
            Direction sweep, SkScalar dx, SkScalar dy)
</pre>

Append <a href="bmh_SkPath?cl=9919#Arc">Arc</a> to <a href="bmh_SkPath?cl=9919#Path">Path</a>, relative to last <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_undocumented?cl=9919#Point">Point</a>. <a href="bmh_SkPath?cl=9919#Arc">Arc</a> is implemented by one or 
more <a href="bmh_SkPath?cl=9919#Conic">Conic</a>, weighted to describe part of <a href="bmh_undocumented?cl=9919#Oval">Oval</a> with radii (r.fX, r.fY) rotated by
<a href="bmh_SkPath?cl=9919#xAxisRotate">xAxisRotate</a> degrees. <a href="bmh_SkPath?cl=9919#Arc">Arc</a> curves from last <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_undocumented?cl=9919#Point">Point</a> (x0, y0) to
(x0 + <a href="bmh_SkPath?cl=9919#dx">dx</a>, y0 + <a href="bmh_SkPath?cl=9919#dy">dy</a>), choosing one of four possible routes: clockwise or
counterclockwise, and smaller or larger. If <a href="bmh_SkPath?cl=9919#Path">Path</a> is empty, the start <a href="bmh_SkPath?cl=9919#Arc">Arc</a> <a href="bmh_undocumented?cl=9919#Point">Point</a>
is (0, 0).

<a href="bmh_SkPath?cl=9919#Arc">Arc</a> <a href="bmh_SkPath?cl=9919#sweep">sweep</a> is always less than 360 degrees. <a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> appends <a href="bmh_undocumented?cl=9919#Line">Line</a> to xy if either
radii are zero, or if last <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_undocumented?cl=9919#Point">Point</a> equals (x, y). <a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> scales radii r to fit
last <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_undocumented?cl=9919#Point">Point</a> and xy if both are greater than zero but too small.

<a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> appends up to four <a href="bmh_SkPath?cl=9919#Conic">Conic</a> curves.
<a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> implements the functionatlity of <a href="bmh_undocumented?cl=9919#Arc">SVG Arc</a>, although <a href="bmh_undocumented?cl=9919#SVG">SVG</a> sweep-flag value is
opposite the integer value of <a href="bmh_SkPath?cl=9919#sweep">sweep</a>; <a href="bmh_undocumented?cl=9919#SVG">SVG</a> sweep-flag uses 1 for clockwise, while
<a href="bmh_SkPath?cl=9919#kCW_Direction">kCW Direction</a> cast to int is zero.

### Parameters

<table>
  <tr>
    <td><code><strong>rx </strong></code></td> <td>radius in x before x-axis rotation.</td>
  </tr>
  <tr>
    <td><code><strong>ry </strong></code></td> <td>radius in y before x-axis rotation.</td>
  </tr>
  <tr>
    <td><code><strong>xAxisRotate </strong></code></td> <td>x-axis rotation in degrees; positve values are clockwise.</td>
  </tr>
  <tr>
    <td><code><strong>largeArc </strong></code></td> <td>chooses smaller or larger <a href="bmh_SkPath?cl=9919#Arc">Arc</a>.</td>
  </tr>
  <tr>
    <td><code><strong>sweep </strong></code></td> <td>chooses clockwise or counterclockwise <a href="bmh_SkPath?cl=9919#Arc">Arc</a>.</td>
  </tr>
  <tr>
    <td><code><strong>dx </strong></code></td> <td>x offset end of <a href="bmh_SkPath?cl=9919#Arc">Arc</a> from last <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_undocumented?cl=9919#Point">Point</a>.</td>
  </tr>
  <tr>
    <td><code><strong>dy </strong></code></td> <td>y offset end of <a href="bmh_SkPath?cl=9919#Arc">Arc</a> from last <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_undocumented?cl=9919#Point">Point</a>.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="b3e9314e1ebfae975fdde269debc6334"></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> <a href="bmh_SkPath?cl=9919#ArcSize">ArcSize</a> <a href="bmh_SkPath?cl=9919#Direction">Direction</a>

---

<a name="close"></a>

## close

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void close()
</pre>

Append <a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a> to <a href="bmh_SkPath?cl=9919#Path">Path</a>. A closed <a href="bmh_SkPath?cl=9919#Contour">Contour</a> connects the first and last <a href="bmh_undocumented?cl=9919#Point">Point</a>
with <a href="bmh_undocumented?cl=9919#Line">Line</a>, forming a continous loop. Open and closed <a href="bmh_SkPath?cl=9919#Contour">Contour</a> draw the same
with <a href="bmh_SkPaint?cl=9919#kFill_Style">SkPaint::kFill Style</a>. With <a href="bmh_SkPaint?cl=9919#kStroke_Style">SkPaint::kStroke Style</a>, open <a href="bmh_SkPath?cl=9919#Contour">Contour</a> draws
<a href="bmh_SkPaint?cl=9919#Stroke_Cap">Paint Stroke Cap</a> at <a href="bmh_SkPath?cl=9919#Contour">Contour</a> start and end; closed <a href="bmh_SkPath?cl=9919#Contour">Contour</a> draws 
<a href="bmh_SkPaint?cl=9919#Stroke_Join">Paint Stroke Join</a> at <a href="bmh_SkPath?cl=9919#Contour">Contour</a> start and end.

<a href="bmh_SkPath?cl=9919#close">close</a> has no effect if <a href="bmh_SkPath?cl=9919#Path">Path</a> is empty or last <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_SkPath?cl=9919#Verb">Verb</a> is <a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a>.

### Example

<div><fiddle-embed name="9235f6309271d6420fa5c45dc28664c5"></fiddle-embed></div>

### See Also

---

<a name="IsInverseFillType"></a>

## IsInverseFillType

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static bool IsInverseFillType(FillType fill)
</pre>

Returns true if <a href="bmh_SkPath?cl=9919#IsInverseFillType">fill</a> is inverted and <a href="bmh_SkPath?cl=9919#Path">Path</a> with <a href="bmh_SkPath?cl=9919#IsInverseFillType">fill</a> represents area outside
of its geometric bounds.

| <a href="bmh_SkPath?cl=9919#FillType">FillType</a> | is inverse |
| --- | ---  |
| <a href="bmh_SkPath?cl=9919#kWinding_FillType">kWinding FillType</a> | false |
| <a href="bmh_SkPath?cl=9919#kEvenOdd_FillType">kEvenOdd FillType</a> | false |
| <a href="bmh_SkPath?cl=9919#kInverseWinding_FillType">kInverseWinding FillType</a> | true |
| <a href="bmh_SkPath?cl=9919#kInverseEvenOdd_FillType">kInverseEvenOdd FillType</a> | true |


### Parameters

<table>
  <tr>
    <td><code><strong>fill </strong></code></td> <td>one of: <a href="bmh_SkPath?cl=9919#kWinding_FillType">kWinding FillType</a>, <a href="bmh_SkPath?cl=9919#kEvenOdd_FillType">kEvenOdd FillType</a>,
             <a href="bmh_SkPath?cl=9919#kInverseWinding_FillType">kInverseWinding FillType</a>, <a href="bmh_SkPath?cl=9919#kInverseEvenOdd_FillType">kInverseEvenOdd FillType</a>.</td>
  </tr>
</table>

### Return Value

true if <a href="bmh_SkPath?cl=9919#Path">Path</a> fills outside its bounds.

### Example

<div><fiddle-embed name="1453856a9d0c73e8192bf298c4143563">#### Example Output

~~~~
IsInverseFillType(kWinding_FillType) == false
IsInverseFillType(kEvenOdd_FillType) == false
IsInverseFillType(kInverseWinding_FillType) == true
IsInverseFillType(kInverseEvenOdd_FillType) == true
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#FillType">FillType</a> <a href="bmh_SkPath?cl=9919#getFillType">getFillType</a> <a href="bmh_SkPath?cl=9919#setFillType">setFillType</a> <a href="bmh_SkPath?cl=9919#ConvertToNonInverseFillType">ConvertToNonInverseFillType</a>

---

<a name="ConvertToNonInverseFillType"></a>

## ConvertToNonInverseFillType

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static FillType ConvertToNonInverseFillType(FillType fill)
</pre>

Returns equivalent <a href="bmh_SkPath?cl=9919#Fill_Type">Fill Type</a> representing <a href="bmh_SkPath?cl=9919#Path">Path</a> <a href="bmh_SkPath?cl=9919#ConvertToNonInverseFillType">fill</a> inside its bounds.
.
| <a href="bmh_SkPath?cl=9919#FillType">FillType</a> | inside <a href="bmh_SkPath?cl=9919#FillType">FillType</a> |
| --- | ---  |
| <a href="bmh_SkPath?cl=9919#kWinding_FillType">kWinding FillType</a> | <a href="bmh_SkPath?cl=9919#kWinding_FillType">kWinding FillType</a> |
| <a href="bmh_SkPath?cl=9919#kEvenOdd_FillType">kEvenOdd FillType</a> | <a href="bmh_SkPath?cl=9919#kEvenOdd_FillType">kEvenOdd FillType</a> |
| <a href="bmh_SkPath?cl=9919#kInverseWinding_FillType">kInverseWinding FillType</a> | <a href="bmh_SkPath?cl=9919#kWinding_FillType">kWinding FillType</a> |
| <a href="bmh_SkPath?cl=9919#kInverseEvenOdd_FillType">kInverseEvenOdd FillType</a> | <a href="bmh_SkPath?cl=9919#kEvenOdd_FillType">kEvenOdd FillType</a> |


### Parameters

<table>
  <tr>
    <td><code><strong>fill </strong></code></td> <td>one of: <a href="bmh_SkPath?cl=9919#kWinding_FillType">kWinding FillType</a>, <a href="bmh_SkPath?cl=9919#kEvenOdd_FillType">kEvenOdd FillType</a>,
             <a href="bmh_SkPath?cl=9919#kInverseWinding_FillType">kInverseWinding FillType</a>, <a href="bmh_SkPath?cl=9919#kInverseEvenOdd_FillType">kInverseEvenOdd FillType</a>.</td>
  </tr>
</table>

### Return Value

<a href="bmh_SkPath?cl=9919#ConvertToNonInverseFillType">fill</a>, or <a href="bmh_SkPath?cl=9919#kWinding_FillType">kWinding FillType</a> or <a href="bmh_SkPath?cl=9919#kEvenOdd_FillType">kEvenOdd FillType</a> if <a href="bmh_SkPath?cl=9919#ConvertToNonInverseFillType">fill</a> is inverted.

### Example

<div><fiddle-embed name="adfae398bbe9e37495f8220ad544c8f8">#### Example Output

~~~~
ConvertToNonInverseFillType(kWinding_FillType) == kWinding_FillType
ConvertToNonInverseFillType(kEvenOdd_FillType) == kEvenOdd_FillType
ConvertToNonInverseFillType(kInverseWinding_FillType) == kWinding_FillType
ConvertToNonInverseFillType(kInverseEvenOdd_FillType) == kEvenOdd_FillType
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#FillType">FillType</a> <a href="bmh_SkPath?cl=9919#getFillType">getFillType</a> <a href="bmh_SkPath?cl=9919#setFillType">setFillType</a> <a href="bmh_SkPath?cl=9919#IsInverseFillType">IsInverseFillType</a>

---

<a name="ConvertConicToQuads"></a>

## ConvertConicToQuads

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
static int ConvertConicToQuads(const SkPoint& p0, const SkPoint& p1,
                               const SkPoint& p2, SkScalar w, SkPoint pts[],
                               int pow2)
</pre>

Approximates <a href="bmh_SkPath?cl=9919#Conic">Conic</a> with <a href="bmh_SkPath?cl=9919#Quad">Quad</a> array. <a href="bmh_SkPath?cl=9919#Conic">Conic</a> is constructed from start <a href="bmh_undocumented?cl=9919#Point">Point</a> <a href="bmh_SkPath?cl=9919#p0">p0</a>,
control <a href="bmh_undocumented?cl=9919#Point">Point</a> <a href="bmh_SkPath?cl=9919#p1">p1</a>, end <a href="bmh_undocumented?cl=9919#Point">Point</a> <a href="bmh_SkPath?cl=9919#p2">p2</a>, and weight <a href="bmh_SkPath?cl=9919#w">w</a>. 
<a href="bmh_SkPath?cl=9919#Quad">Quad</a> array is stored in <a href="bmh_SkPath?cl=9919#pts">pts</a>; this storage is supplied by caller.
Maximum <a href="bmh_SkPath?cl=9919#Quad">Quad</a> count is 2 to the <a href="bmh_SkPath?cl=9919#pow2">pow2</a>.
Every third point in array shares last <a href="bmh_undocumented?cl=9919#Point">Point</a> of previous <a href="bmh_SkPath?cl=9919#Quad">Quad</a> and first <a href="bmh_undocumented?cl=9919#Point">Point</a> of 
next <a href="bmh_SkPath?cl=9919#Quad">Quad</a>. Maximum <a href="bmh_SkPath?cl=9919#pts">pts</a> storage size is given by: 
(1 + 2 * (1 << <a href="bmh_SkPath?cl=9919#pow2">pow2</a>)) * sizeof(SkPoint)<a href="bmh_SkPath?cl=9919#ConvertConicToQuads">ConvertConicToQuads</a> returns <a href="bmh_SkPath?cl=9919#Quad">Quad</a> count used the approximation, which may be smaller
than the number requested.
 <a href="bmh_SkPath?cl=9919#Conic_Weight">Conic Weight</a> determines the amount of influence <a href="bmh_SkPath?cl=9919#Conic">Conic</a> control point has on the curve.
<a href="bmh_SkPath?cl=9919#w">w</a> less than one represents an elliptical section. <a href="bmh_SkPath?cl=9919#w">w</a> greater than one represents
a hyperbolic section. <a href="bmh_SkPath?cl=9919#w">w</a> equal to one represents a parabolic section.

Two <a href="bmh_SkPath?cl=9919#Quad">Quad</a> curves are sufficient to approximate an elliptical <a href="bmh_SkPath?cl=9919#Conic">Conic</a> with a sweep
of up to 90 degrees; in this case, set <a href="bmh_SkPath?cl=9919#pow2">pow2</a> to one.

### Parameters

<table>
  <tr>
    <td><code><strong>p0 </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Conic">Conic</a> start <a href="bmh_undocumented?cl=9919#Point">Point</a>.</td>
  </tr>
  <tr>
    <td><code><strong>p1 </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Conic">Conic</a> control <a href="bmh_undocumented?cl=9919#Point">Point</a>.</td>
  </tr>
  <tr>
    <td><code><strong>p2 </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Conic">Conic</a> end <a href="bmh_undocumented?cl=9919#Point">Point</a>.</td>
  </tr>
  <tr>
    <td><code><strong>w </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Conic">Conic</a> weight.</td>
  </tr>
  <tr>
    <td><code><strong>pts </strong></code></td> <td>storage for <a href="bmh_SkPath?cl=9919#Quad">Quad</a> array.</td>
  </tr>
  <tr>
    <td><code><strong>pow2 </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Quad">Quad</a> count, as power of two, normally 0 to 5 (1 to 32 <a href="bmh_SkPath?cl=9919#Quad">Quad</a> curves).</td>
  </tr>
</table>

### Return Value

Number of <a href="bmh_SkPath?cl=9919#Quad">Quad</a> curves written to <a href="bmh_SkPath?cl=9919#pts">pts</a>.

### Example

<div><fiddle-embed name="460a3604a8bc37dcef031064dd4c046b"><p>A pair of <a href="bmh_SkPath?cl=9919#Quad">Quad</a> curves are drawn in red on top of the elliptical <a href="bmh_SkPath?cl=9919#Conic">Conic</a> curve in black.
The middle curve is nearly circular. The top-right curve is parabolic, which can
be drawn exactly with a single <a href="bmh_SkPath?cl=9919#Quad">Quad</a>.</p></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#Conic">Conic</a> <a href="bmh_SkPath?cl=9919#Quad">Quad</a>

---

<a name="isRect"></a>

## isRect

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isRect(SkRect* rect, bool* isClosed = NULL, Direction* direction = NULL) const
</pre>

Returns true if <a href="bmh_SkPath?cl=9919#Path">Path</a> is eqivalent to <a href="bmh_undocumented?cl=9919#Rect">Rect</a> when filled.
If <a href="bmh_SkPath?cl=9919#isRect">isRect</a> returns false: <a href="bmh_SkPath?cl=9919#rect">rect</a>, <a href="bmh_SkPath?cl=9919#isClosed">isClosed</a>, and <a href="bmh_SkPath?cl=9919#direction">direction</a> are unchanged.
If <a href="bmh_SkPath?cl=9919#isRect">isRect</a> returns true: <a href="bmh_SkPath?cl=9919#rect">rect</a>, <a href="bmh_SkPath?cl=9919#isClosed">isClosed</a>, and <a href="bmh_SkPath?cl=9919#direction">direction</a> are written to if not nullptr.

<a href="bmh_SkPath?cl=9919#rect">rect</a> may be smaller than the <a href="bmh_SkPath?cl=9919#Path">Path</a> bounds. <a href="bmh_SkPath?cl=9919#Path">Path</a> bounds may include <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a> points
that do not alter the area drawn by the returned <a href="bmh_SkPath?cl=9919#rect">rect</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>rect </strong></code></td> <td>storage for bounds of <a href="bmh_undocumented?cl=9919#Rect">Rect</a>; may be nullptr.</td>
  </tr>
  <tr>
    <td><code><strong>isClosed </strong></code></td> <td>storage set to true if <a href="bmh_SkPath?cl=9919#Path">Path</a> is closed; may be nullptr</td>
  </tr>
  <tr>
    <td><code><strong>direction </strong></code></td> <td>storage set to <a href="bmh_undocumented?cl=9919#Rect">Rect</a> <a href="bmh_SkPath?cl=9919#direction">direction</a>; may be nullptr.</td>
  </tr>
</table>

### Return Value

true if <a href="bmh_SkPath?cl=9919#Path">Path</a> contains <a href="bmh_undocumented?cl=9919#Rect">Rect</a>.

### Example

<div><fiddle-embed name="063a5f0a8de1fe998d227393e0866557"><p>After <a href="bmh_SkPath?cl=9919#addRect">addRect</a>, <a href="bmh_SkPath?cl=9919#isRect">isRect</a> returns true. Following <a href="bmh_SkPath?cl=9919#moveTo">moveTo</a> permits <a href="bmh_SkPath?cl=9919#isRect">isRect</a> to return true, but
following <a href="bmh_SkPath?cl=9919#lineTo">lineTo</a> does not. <a href="bmh_SkPath?cl=9919#addPoly">addPoly</a> returns true even though <a href="bmh_SkPath?cl=9919#rect">rect</a> is not closed, and one
side of <a href="bmh_SkPath?cl=9919#rect">rect</a> is made up of consecutive line segments.</p>

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

<a href="bmh_SkPath?cl=9919#computeTightBounds">computeTightBounds</a> <a href="bmh_SkPath?cl=9919#conservativelyContainsRect">conservativelyContainsRect</a> <a href="bmh_SkPath?cl=9919#getBounds">getBounds</a> <a href="bmh_SkPath?cl=9919#isConvex">isConvex</a> <a href="bmh_SkPath?cl=9919#isLastContourClosed">isLastContourClosed</a> <a href="bmh_SkPath?cl=9919#isNestedFillRects">isNestedFillRects</a>

---

<a name="isNestedFillRects"></a>

## isNestedFillRects

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isNestedFillRects(SkRect rect[2], Direction dirs[2] = NULL) const
</pre>

Returns true if <a href="bmh_SkPath?cl=9919#Path">Path</a> is equivalent to nested <a href="bmh_undocumented?cl=9919#Rect">Rect</a> pair when filled.
If <a href="bmh_SkPath?cl=9919#isNestedFillRects">isNestedFillRects</a> returns false, <a href="bmh_SkPath?cl=9919#rect">rect</a> and <a href="bmh_SkPath?cl=9919#dirs">dirs</a> are unchanged.
If <a href="bmh_SkPath?cl=9919#isNestedFillRects">isNestedFillRects</a> returns true, <a href="bmh_SkPath?cl=9919#rect">rect</a> and <a href="bmh_SkPath?cl=9919#dirs">dirs</a> are written to if not nullptr:
setting <a href="bmh_SkPath?cl=9919#rect">rect</a>[0] to outer <a href="bmh_undocumented?cl=9919#Rect">Rect</a>, and <a href="bmh_SkPath?cl=9919#rect">rect</a>[1] to inner <a href="bmh_undocumented?cl=9919#Rect">Rect</a>;
setting <a href="bmh_SkPath?cl=9919#dirs">dirs</a>[0] to <a href="bmh_SkPath?cl=9919#Direction">Direction</a> of outer <a href="bmh_undocumented?cl=9919#Rect">Rect</a>, and <a href="bmh_SkPath?cl=9919#dirs">dirs</a>[1] to <a href="bmh_SkPath?cl=9919#Direction">Direction</a> of inner
<a href="bmh_undocumented?cl=9919#Rect">Rect</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>rect </strong></code></td> <td>storage for <a href="bmh_undocumented?cl=9919#Rect">Rect</a> pair; may be nullptr.</td>
  </tr>
  <tr>
    <td><code><strong>dirs </strong></code></td> <td>storage for <a href="bmh_SkPath?cl=9919#Direction">Direction</a> pair; may be nullptr.</td>
  </tr>
</table>

### Return Value

true if <a href="bmh_SkPath?cl=9919#Path">Path</a> contains nested <a href="bmh_undocumented?cl=9919#Rect">Rect</a> pair.

### Example

<div><fiddle-embed name="77e4394caf9fa083c19c21c2462efe14">#### Example Output

~~~~
outer (7.5, 17.5, 32.5, 42.5); direction CW
inner (12.5, 22.5, 27.5, 37.5); direction CCW
~~~~

</fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#computeTightBounds">computeTightBounds</a> <a href="bmh_SkPath?cl=9919#conservativelyContainsRect">conservativelyContainsRect</a> <a href="bmh_SkPath?cl=9919#getBounds">getBounds</a> <a href="bmh_SkPath?cl=9919#isConvex">isConvex</a> <a href="bmh_SkPath?cl=9919#isLastContourClosed">isLastContourClosed</a> <a href="bmh_SkPath?cl=9919#isRect">isRect</a>

---

<a name="addRect"></a>

## addRect

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void addRect(const SkRect& rect, Direction dir = kCW_Direction)
</pre>

Add <a href="bmh_undocumented?cl=9919#Rect">Rect</a> to <a href="bmh_SkPath?cl=9919#Path">Path</a>, appending <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a>, three <a href="bmh_SkPath?cl=9919#kLine_Verb">kLine Verb</a>, and <a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a>,
starting with top-left corner of <a href="bmh_undocumented?cl=9919#Rect">Rect</a>; followed by top-right, bottom-right,
and bottom-left if <a href="bmh_SkPath?cl=9919#dir">dir</a> is <a href="bmh_SkPath?cl=9919#kCW_Direction">kCW Direction</a>; or followed by bottom-left,
bottom-right, and top-right if <a href="bmh_SkPath?cl=9919#dir">dir</a> is <a href="bmh_SkPath?cl=9919#kCCW_Direction">kCCW Direction</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>rect </strong></code></td> <td><a href="bmh_undocumented?cl=9919#Rect">Rect</a> to add as a closed contour.</td>
  </tr>
  <tr>
    <td><code><strong>dir </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Direction">Direction</a> to wind added contour.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0f841e4eaebb613b5069800567917c2d"><p>The left <a href="bmh_undocumented?cl=9919#Rect">Rect</a> dashes starting at the top-left corner, to the right.
The right <a href="bmh_undocumented?cl=9919#Rect">Rect</a> dashes starting at the top-left corner, towards the bottom.</p></fiddle-embed></div>

### See Also

<a href="bmh_SkCanvas?cl=9919#drawRect">SkCanvas::drawRect</a> <a href="bmh_SkPath?cl=9919#Direction">Direction</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void addRect(const SkRect& rect, Direction dir, unsigned start)
</pre>

Add <a href="bmh_undocumented?cl=9919#Rect">Rect</a> to <a href="bmh_SkPath?cl=9919#Path">Path</a>, appending <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a>, three <a href="bmh_SkPath?cl=9919#kLine_Verb">kLine Verb</a>, and <a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a>.
If <a href="bmh_SkPath?cl=9919#dir">dir</a> is <a href="bmh_SkPath?cl=9919#kCW_Direction">kCW Direction</a>, <a href="bmh_undocumented?cl=9919#Rect">Rect</a> corners are added clockwise; if <a href="bmh_SkPath?cl=9919#dir">dir</a> is
<a href="bmh_SkPath?cl=9919#kCCW_Direction">kCCW Direction</a>, <a href="bmh_undocumented?cl=9919#Rect">Rect</a> corners are added counterclockwise.
<a href="bmh_SkPath?cl=9919#start">start</a> determines the first corner added.

| <a href="bmh_SkPath?cl=9919#start">start</a> | first corner |
| --- | ---  |
| 0 | top-left |
| 1 | top-right |
| 2 | bottom-right |
| 3 | bottom-left |


### Parameters

<table>
  <tr>
    <td><code><strong>rect </strong></code></td> <td><a href="bmh_undocumented?cl=9919#Rect">Rect</a> to add as a closed contour.</td>
  </tr>
  <tr>
    <td><code><strong>dir </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Direction">Direction</a> to wind added contour.</td>
  </tr>
  <tr>
    <td><code><strong>start </strong></code></td> <td>Initial corner of <a href="bmh_undocumented?cl=9919#Rect">Rect</a> to add.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="81a5bf1262a7a4d2a87d5908c8de18a4"><p>The arrow is just after the initial corner and points towards the next
corner appended to <a href="bmh_SkPath?cl=9919#Path">Path</a>.</p></fiddle-embed></div>

### See Also

<a href="bmh_SkCanvas?cl=9919#drawRect">SkCanvas::drawRect</a> <a href="bmh_SkPath?cl=9919#Direction">Direction</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void addRect(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom,
             Direction dir = kCW_Direction)
</pre>

Add <a href="bmh_undocumented?cl=9919#Rect">Rect</a> (<a href="bmh_SkPath?cl=9919#left">left</a>, <a href="bmh_SkPath?cl=9919#top">top</a>, <a href="bmh_SkPath?cl=9919#right">right</a>, <a href="bmh_SkPath?cl=9919#bottom">bottom</a>) to <a href="bmh_SkPath?cl=9919#Path">Path</a>,
appending <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a>, three <a href="bmh_SkPath?cl=9919#kLine_Verb">kLine Verb</a>, and <a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a>,
starting with top-left corner of <a href="bmh_undocumented?cl=9919#Rect">Rect</a>; followed by top-right, bottom-right,
and bottom-left if <a href="bmh_SkPath?cl=9919#dir">dir</a> is <a href="bmh_SkPath?cl=9919#kCW_Direction">kCW Direction</a>; or followed by bottom-left,
bottom-right, and top-right if <a href="bmh_SkPath?cl=9919#dir">dir</a> is <a href="bmh_SkPath?cl=9919#kCCW_Direction">kCCW Direction</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>left </strong></code></td> <td>smaller x of <a href="bmh_undocumented?cl=9919#Rect">Rect</a>.</td>
  </tr>
  <tr>
    <td><code><strong>top </strong></code></td> <td>smaller y of <a href="bmh_undocumented?cl=9919#Rect">Rect</a>.</td>
  </tr>
  <tr>
    <td><code><strong>right </strong></code></td> <td>larger x of <a href="bmh_undocumented?cl=9919#Rect">Rect</a>.</td>
  </tr>
  <tr>
    <td><code><strong>bottom </strong></code></td> <td>larger y of <a href="bmh_undocumented?cl=9919#Rect">Rect</a>.</td>
  </tr>
  <tr>
    <td><code><strong>dir </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Direction">Direction</a> to wind added contour.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3837827310e8b88b8c2e128ef9fbbd65"><p>The <a href="bmh_SkPath?cl=9919#left">left</a> <a href="bmh_undocumented?cl=9919#Rect">Rect</a> dashes start at the top-left corner, and continue to the <a href="bmh_SkPath?cl=9919#right">right</a>.
The <a href="bmh_SkPath?cl=9919#right">right</a> <a href="bmh_undocumented?cl=9919#Rect">Rect</a> dashes start at the top-left corner, and continue down.</p></fiddle-embed></div>

### See Also

<a href="bmh_SkCanvas?cl=9919#drawRect">SkCanvas::drawRect</a> <a href="bmh_SkPath?cl=9919#Direction">Direction</a>

---

<a name="addOval"></a>

## addOval

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void addOval(const SkRect& oval, Direction dir = kCW_Direction)
</pre>

Add <a href="bmh_undocumented?cl=9919#Oval">Oval</a> to path, appending <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a>, four <a href="bmh_SkPath?cl=9919#kConic_Verb">kConic Verb</a>, and <a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a>.
<a href="bmh_undocumented?cl=9919#Oval">Oval</a> is upright ellipse bounded by <a href="bmh_undocumented?cl=9919#Rect">Rect</a> <a href="bmh_SkPath?cl=9919#oval">oval</a> with radii equal to half <a href="bmh_SkPath?cl=9919#oval">oval</a> width
and half <a href="bmh_SkPath?cl=9919#oval">oval</a> height. <a href="bmh_undocumented?cl=9919#Oval">Oval</a> begins at (<a href="bmh_SkPath?cl=9919#oval">oval</a>.fRight, <a href="bmh_SkPath?cl=9919#oval">oval</a>.centerY()) and continues
clockwise if <a href="bmh_SkPath?cl=9919#dir">dir</a> is <a href="bmh_SkPath?cl=9919#kCW_Direction">kCW Direction</a>, counterclockwise if <a href="bmh_SkPath?cl=9919#dir">dir</a> is <a href="bmh_SkPath?cl=9919#kCCW_Direction">kCCW Direction</a>.

This form is identical to

### Parameters

<table>
  <tr>
    <td><code><strong>oval </strong></code></td> <td>Bounds of ellipse added.</td>
  </tr>
  <tr>
    <td><code><strong>dir </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Direction">Direction</a> to wind ellipse.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

### See Also

<a href="bmh_SkCanvas?cl=9919#drawOval">SkCanvas::drawOval</a> <a href="bmh_SkPath?cl=9919#Direction">Direction</a> <a href="bmh_undocumented?cl=9919#Oval">Oval</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void addOval(const SkRect& oval, Direction dir, unsigned start)
</pre>

Add <a href="bmh_undocumented?cl=9919#Oval">Oval</a> to <a href="bmh_SkPath?cl=9919#Path">Path</a>, appending <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a>, four <a href="bmh_SkPath?cl=9919#kConic_Verb">kConic Verb</a>, and <a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a>.
<a href="bmh_undocumented?cl=9919#Oval">Oval</a> is upright ellipse bounded by <a href="bmh_undocumented?cl=9919#Rect">Rect</a> <a href="bmh_SkPath?cl=9919#oval">oval</a> with radii equal to half <a href="bmh_SkPath?cl=9919#oval">oval</a> width
and half <a href="bmh_SkPath?cl=9919#oval">oval</a> height. <a href="bmh_undocumented?cl=9919#Oval">Oval</a> begins at <a href="bmh_SkPath?cl=9919#start">start</a> and continues
clockwise if <a href="bmh_SkPath?cl=9919#dir">dir</a> is <a href="bmh_SkPath?cl=9919#kCW_Direction">kCW Direction</a>, counterclockwise if <a href="bmh_SkPath?cl=9919#dir">dir</a> is <a href="bmh_SkPath?cl=9919#kCCW_Direction">kCCW Direction</a>.

| <a href="bmh_SkPath?cl=9919#start">start</a> | <a href="bmh_undocumented?cl=9919#Point">Point</a> |
| --- | ---  |
| 0 | <a href="bmh_SkPath?cl=9919#oval">oval</a>.centerX(), <a href="bmh_SkPath?cl=9919#oval">oval</a>.fTop |
| 1 | <a href="bmh_SkPath?cl=9919#oval">oval</a>.fRight, <a href="bmh_SkPath?cl=9919#oval">oval</a>.centerY() |
| 2 | <a href="bmh_SkPath?cl=9919#oval">oval</a>.centerX(), <a href="bmh_SkPath?cl=9919#oval">oval</a>.fBottom |
| 3 | <a href="bmh_SkPath?cl=9919#oval">oval</a>.fLeft, <a href="bmh_SkPath?cl=9919#oval">oval</a>.centerY() |


### Parameters

<table>
  <tr>
    <td><code><strong>oval </strong></code></td> <td>Bounds of ellipse added.</td>
  </tr>
  <tr>
    <td><code><strong>dir </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Direction">Direction</a> to wind ellipse.</td>
  </tr>
  <tr>
    <td><code><strong>start </strong></code></td> <td>Index of initial point of ellipse.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="422ae5c7b5f30126e52fa090073f5361"></fiddle-embed></div>

### See Also

<a href="bmh_SkCanvas?cl=9919#drawOval">SkCanvas::drawOval</a> <a href="bmh_SkPath?cl=9919#Direction">Direction</a> <a href="bmh_undocumented?cl=9919#Oval">Oval</a>

---

<a name="addCircle"></a>

## addCircle

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void addCircle(SkScalar x, SkScalar y, SkScalar radius,
               Direction dir = kCW_Direction)
</pre>

Add <a href="bmh_undocumented?cl=9919#Circle">Circle</a> centered at (<a href="bmh_SkPath?cl=9919#x">x</a>, <a href="bmh_SkPath?cl=9919#y">y</a>) of size <a href="bmh_SkPath?cl=9919#radius">radius</a> to <a href="bmh_SkPath?cl=9919#Path">Path</a>, appending <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a>,
four <a href="bmh_SkPath?cl=9919#kConic_Verb">kConic Verb</a>, and <a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a>. <a href="bmh_undocumented?cl=9919#Circle">Circle</a> begins at (<a href="bmh_SkPath?cl=9919#x">x</a> + <a href="bmh_SkPath?cl=9919#radius">radius</a>, <a href="bmh_SkPath?cl=9919#y">y</a>) and
continues clockwise if <a href="bmh_SkPath?cl=9919#dir">dir</a> is <a href="bmh_SkPath?cl=9919#kCW_Direction">kCW Direction</a>, counterclockwise if <a href="bmh_SkPath?cl=9919#dir">dir</a> is
<a href="bmh_SkPath?cl=9919#kCCW_Direction">kCCW Direction</a>.

<a href="bmh_SkPath?cl=9919#addCircle">addCircle</a> has no effect if <a href="bmh_SkPath?cl=9919#radius">radius</a> is zero or negative.

### Parameters

<table>
  <tr>
    <td><code><strong>x </strong></code></td> <td>center of <a href="bmh_undocumented?cl=9919#Circle">Circle</a>.</td>
  </tr>
  <tr>
    <td><code><strong>y </strong></code></td> <td>center of <a href="bmh_undocumented?cl=9919#Circle">Circle</a>.</td>
  </tr>
  <tr>
    <td><code><strong>radius </strong></code></td> <td>distance from center to edge.</td>
  </tr>
  <tr>
    <td><code><strong>dir </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Direction">Direction</a> to wind <a href="bmh_undocumented?cl=9919#Circle">Circle</a>.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="51c5853030fbc2db917a03c3de472bc5"></fiddle-embed></div>

### See Also

<a href="bmh_SkCanvas?cl=9919#drawCircle">SkCanvas::drawCircle</a> <a href="bmh_SkPath?cl=9919#Direction">Direction</a> <a href="bmh_undocumented?cl=9919#Circle">Circle</a>

---

<a name="addArc"></a>

## addArc

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void addArc(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle)
</pre>

Append <a href="bmh_SkPath?cl=9919#Arc">Arc</a> to <a href="bmh_SkPath?cl=9919#Path">Path</a>, as the start of new <a href="bmh_SkPath?cl=9919#Contour">Contour</a>. <a href="bmh_SkPath?cl=9919#Arc">Arc</a> added is part of ellipse
bounded by <a href="bmh_SkPath?cl=9919#oval">oval</a>, from <a href="bmh_SkPath?cl=9919#startAngle">startAngle</a> through <a href="bmh_SkPath?cl=9919#sweepAngle">sweepAngle</a>. Both <a href="bmh_SkPath?cl=9919#startAngle">startAngle</a> and
<a href="bmh_SkPath?cl=9919#sweepAngle">sweepAngle</a> are measured in degrees, where zero degrees is aligned with the
positive x-axis, and positive sweeps extends <a href="bmh_SkPath?cl=9919#Arc">Arc</a> clockwise.

If <a href="bmh_SkPath?cl=9919#sweepAngle">sweepAngle</a> <= -360, or <a href="bmh_SkPath?cl=9919#sweepAngle">sweepAngle</a> >= 360; and <a href="bmh_SkPath?cl=9919#startAngle">startAngle</a> modulo 90 is nearly 
zero, append <a href="bmh_undocumented?cl=9919#Oval">Oval</a> instead of <a href="bmh_SkPath?cl=9919#Arc">Arc</a>. Otherwise, <a href="bmh_SkPath?cl=9919#sweepAngle">sweepAngle</a> values are treated 
modulo 360, and <a href="bmh_SkPath?cl=9919#Arc">Arc</a> may or may not draw depending on numeric rounding.

### Parameters

<table>
  <tr>
    <td><code><strong>oval </strong></code></td> <td>bounds of ellipse containing <a href="bmh_SkPath?cl=9919#Arc">Arc</a>.</td>
  </tr>
  <tr>
    <td><code><strong>startAngle </strong></code></td> <td>starting angle of <a href="bmh_SkPath?cl=9919#Arc">Arc</a> in degrees.</td>
  </tr>
  <tr>
    <td><code><strong>sweepAngle </strong></code></td> <td>sweep, in degrees. Positive is clockwise; treated modulo 360.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="9cf5122475624e4cf39f06c698f80b1a"><p>The middle row of the left and right columns draw differently from the entries
above and below because <a href="bmh_SkPath?cl=9919#sweepAngle">sweepAngle</a> is outside of the range of +/-360, 
and <a href="bmh_SkPath?cl=9919#startAngle">startAngle</a> modulo 90 is not zero.</p></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#Arc">Arc</a> <a href="bmh_SkPath?cl=9919#arcTo">arcTo</a> <a href="bmh_SkCanvas?cl=9919#drawArc">SkCanvas::drawArc</a>

---

<a name="addRoundRect"></a>

## addRoundRect

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void addRoundRect(const SkRect& rect, SkScalar rx, SkScalar ry,
                  Direction dir = kCW_Direction)
</pre>

Append <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a> to <a href="bmh_SkPath?cl=9919#Path">Path</a>, creating a new closed <a href="bmh_SkPath?cl=9919#Contour">Contour</a>. <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a> has bounds
equal to <a href="bmh_SkPath?cl=9919#rect">rect</a>; each corner is 90 degrees of an ellipse with radii (<a href="bmh_SkPath?cl=9919#rx">rx</a>, <a href="bmh_SkPath?cl=9919#ry">ry</a>). If
<a href="bmh_SkPath?cl=9919#dir">dir</a> is <a href="bmh_SkPath?cl=9919#kCW_Direction">kCW Direction</a>, <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a> starts at top-left of the lower-left corner and
winds clockwise. If <a href="bmh_SkPath?cl=9919#dir">dir</a> is <a href="bmh_SkPath?cl=9919#kCCW_Direction">kCCW Direction</a>, <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a> starts at the bottom-left
of the upper-left corner and winds counterclockwise.

If either <a href="bmh_SkPath?cl=9919#rx">rx</a> or <a href="bmh_SkPath?cl=9919#ry">ry</a> is too large, <a href="bmh_SkPath?cl=9919#rx">rx</a> and <a href="bmh_SkPath?cl=9919#ry">ry</a> are scaled uniformly until the
corners fit. If <a href="bmh_SkPath?cl=9919#rx">rx</a> or <a href="bmh_SkPath?cl=9919#ry">ry</a> is less than or equal to zero, <a href="bmh_SkPath?cl=9919#addRoundRect">addRoundRect</a> appends
<a href="bmh_undocumented?cl=9919#Rect">Rect</a> <a href="bmh_SkPath?cl=9919#rect">rect</a> to <a href="bmh_SkPath?cl=9919#Path">Path</a>.

After appending, <a href="bmh_SkPath?cl=9919#Path">Path</a> may be empty, or may contain: <a href="bmh_undocumented?cl=9919#Rect">Rect</a>, <a href="bmh_undocumented?cl=9919#Oval">Oval</a>, or RoundRect.

### Parameters

<table>
  <tr>
    <td><code><strong>rect </strong></code></td> <td>bounds of <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a>.</td>
  </tr>
  <tr>
    <td><code><strong>rx </strong></code></td> <td>x-radius of rounded corners on the <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a></td>
  </tr>
  <tr>
    <td><code><strong>ry </strong></code></td> <td>y-radius of rounded corners on the <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a></td>
  </tr>
  <tr>
    <td><code><strong>dir </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Direction">Direction</a> to wind <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a>.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d4dc8e1bc0c19b7271e5fae83e40888c"><p>If either radius is zero, path contains <a href="bmh_undocumented?cl=9919#Rect">Rect</a> and is drawn red.
If sides are only radii, path contains <a href="bmh_undocumented?cl=9919#Oval">Oval</a> and is drawn blue.
All remaining path draws are convex, and are drawn in gray; no
paths constructed from <a href="bmh_SkPath?cl=9919#addRoundRect">addRoundRect</a> are concave, so none are
drawn in green.</p></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#addRRect">addRRect</a> <a href="bmh_SkCanvas?cl=9919#drawRoundRect">SkCanvas::drawRoundRect</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void addRoundRect(const SkRect& rect, const SkScalar radii[],
                  Direction dir = kCW_Direction)
</pre>

Append <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a> to <a href="bmh_SkPath?cl=9919#Path">Path</a>, creating a new closed <a href="bmh_SkPath?cl=9919#Contour">Contour</a>. <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a> has bounds
equal to <a href="bmh_SkPath?cl=9919#rect">rect</a>; each corner is 90 degrees of an ellipse with <a href="bmh_SkPath?cl=9919#radii">radii</a> from the
array.

| <a href="bmh_SkPath?cl=9919#radii">radii</a> index | location |
| --- | ---  |
| 0 | x-radius of top-left corner |
| 1 | y-radius of top-left corner |
| 2 | x-radius of top-right corner |
| 3 | y-radius of top-right corner |
| 4 | x-radius of bottom-right corner |
| 5 | y-radius of bottom-right corner |
| 6 | x-radius of bottom-left corner |
| 7 | y-radius of bottom-left corner |


If <a href="bmh_SkPath?cl=9919#dir">dir</a> is <a href="bmh_SkPath?cl=9919#kCW_Direction">kCW Direction</a>, <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a> starts at top-left of the lower-left corner 
and winds clockwise. If <a href="bmh_SkPath?cl=9919#dir">dir</a> is <a href="bmh_SkPath?cl=9919#kCCW_Direction">kCCW Direction</a>, <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a> starts at the 
bottom-left of the upper-left corner and winds counterclockwise.

If both <a href="bmh_SkPath?cl=9919#radii">radii</a> on any side of <a href="bmh_SkPath?cl=9919#rect">rect</a> exceed its length, all <a href="bmh_SkPath?cl=9919#radii">radii</a> are scaled 
uniformly until the corners fit. If either radius of a corner is less than or
equal to zero, both are treated as zero.

After appending, <a href="bmh_SkPath?cl=9919#Path">Path</a> may be empty, or may contain: <a href="bmh_undocumented?cl=9919#Rect">Rect</a>, <a href="bmh_undocumented?cl=9919#Oval">Oval</a>, or RoundRect.

### Parameters

<table>
  <tr>
    <td><code><strong>rect </strong></code></td> <td>bounds of <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a>.</td>
  </tr>
  <tr>
    <td><code><strong>radii </strong></code></td> <td>array of 8 <a href="bmh_undocumented?cl=9919#SkScalar">SkScalar</a> values, a radius pair for each corner.</td>
  </tr>
  <tr>
    <td><code><strong>dir </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Direction">Direction</a> to wind <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a>.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="faedc32e7db9c668ae386f1d50be2499"></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#addRRect">addRRect</a> <a href="bmh_SkCanvas?cl=9919#drawRoundRect">SkCanvas::drawRoundRect</a>

---

<a name="addRRect"></a>

## addRRect

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void addRRect(const SkRRect& rrect, Direction dir = kCW_Direction)
</pre>

Add <a href="bmh_SkPath?cl=9919#rrect">rrect</a> to <a href="bmh_SkPath?cl=9919#Path">Path</a>, creating a new closed <a href="bmh_SkPath?cl=9919#Contour">Contour</a>. If
<a href="bmh_SkPath?cl=9919#dir">dir</a> is <a href="bmh_SkPath?cl=9919#kCW_Direction">kCW Direction</a>, <a href="bmh_SkPath?cl=9919#rrect">rrect</a> starts at top-left of the lower-left corner and
winds clockwise. If <a href="bmh_SkPath?cl=9919#dir">dir</a> is <a href="bmh_SkPath?cl=9919#kCCW_Direction">kCCW Direction</a>, <a href="bmh_SkPath?cl=9919#rrect">rrect</a> starts at the bottom-left
of the upper-left corner and winds counterclockwise.

After appending, <a href="bmh_SkPath?cl=9919#Path">Path</a> may be empty, or may contain: <a href="bmh_undocumented?cl=9919#Rect">Rect</a>, <a href="bmh_undocumented?cl=9919#Oval">Oval</a>, or RRect.

### Parameters

<table>
  <tr>
    <td><code><strong>rrect </strong></code></td> <td>bounds and radii of rounded rectangle.</td>
  </tr>
  <tr>
    <td><code><strong>dir </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Direction">Direction</a> to wind <a href="bmh_undocumented?cl=9919#Round_Rect">Round Rect</a>.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="bd53f1d80ec1a9ef00133cd4ee74f71b"></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#addRoundRect">addRoundRect</a> <a href="bmh_SkCanvas?cl=9919#drawRRect">SkCanvas::drawRRect</a>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void addRRect(const SkRRect& rrect, Direction dir, unsigned start)
</pre>

Add <a href="bmh_SkPath?cl=9919#rrect">rrect</a> to <a href="bmh_SkPath?cl=9919#Path">Path</a>, creating a new closed <a href="bmh_SkPath?cl=9919#Contour">Contour</a>. If <a href="bmh_SkPath?cl=9919#dir">dir</a> is <a href="bmh_SkPath?cl=9919#kCW_Direction">kCW Direction</a>, <a href="bmh_SkPath?cl=9919#rrect">rrect</a>
winds clockwise; if <a href="bmh_SkPath?cl=9919#dir">dir</a> is <a href="bmh_SkPath?cl=9919#kCCW_Direction">kCCW Direction</a>, <a href="bmh_SkPath?cl=9919#rrect">rrect</a> winds counterclockwise.
<a href="bmh_SkPath?cl=9919#start">start</a> determines the first point of <a href="bmh_SkPath?cl=9919#rrect">rrect</a> to add.

| <a href="bmh_SkPath?cl=9919#start">start</a> | location |
| --- | ---  |
| 0 | right of top-left corner |
| 1 | left of top-right corner |
| 2 | bottom of top-right corner |
| 3 | top of bottom-right corner |
| 4 | left of bottom-right corner |
| 5 | right of bottom-left corner |
| 6 | top of bottom-left corner |
| 7 | bottom of top-left corner |


After appending, <a href="bmh_SkPath?cl=9919#Path">Path</a> may be empty, or may contain: <a href="bmh_undocumented?cl=9919#Rect">Rect</a>, <a href="bmh_undocumented?cl=9919#Oval">Oval</a>, or RRect.

### Parameters

<table>
  <tr>
    <td><code><strong>rrect </strong></code></td> <td>bounds and radii of rounded rectangle.</td>
  </tr>
  <tr>
    <td><code><strong>dir </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Direction">Direction</a> to wind RRect.</td>
  </tr>
  <tr>
    <td><code><strong>start </strong></code></td> <td>Index of initial point of RRect.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="8e253691a5b9b536f634cfd675d0d1eb"></fiddle-embed></div>

### See Also

<a href="bmh_SkPath?cl=9919#addRoundRect">addRoundRect</a> <a href="bmh_SkCanvas?cl=9919#drawRRect">SkCanvas::drawRRect</a>

---

<a name="addPoly"></a>

## addPoly

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void addPoly(const SkPoint pts[], int count, bool close)
</pre>

Add <a href="bmh_SkPath?cl=9919#Contour">Contour</a> created from <a href="bmh_undocumented?cl=9919#Line">Line</a> Array. Given <a href="bmh_SkPath?cl=9919#count">count</a> <a href="bmh_SkPath?cl=9919#pts">pts</a>, <a href="bmh_SkPath?cl=9919#addPoly">addPoly</a> adds
<a href="bmh_SkPath?cl=9919#count">count</a> - 1 <a href="bmh_undocumented?cl=9919#Line">Line</a> segments. <a href="bmh_SkPath?cl=9919#Contour">Contour</a> added starts at pt[0], then adds a line
for every additional <a href="bmh_undocumented?cl=9919#Point">Point</a> in <a href="bmh_SkPath?cl=9919#pts">pts</a> array. If <a href="bmh_SkPath?cl=9919#close">close</a> is true, <a href="bmh_SkPath?cl=9919#addPoly">addPoly</a>
appends <a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a> to <a href="bmh_SkPath?cl=9919#Path">Path</a>, connecting <a href="bmh_SkPath?cl=9919#pts">pts</a>[<a href="bmh_SkPath?cl=9919#count">count</a> - 1] and <a href="bmh_SkPath?cl=9919#pts">pts</a>[0].

If <a href="bmh_SkPath?cl=9919#count">count</a> is zero, append <a href="bmh_SkPath?cl=9919#kMove_Verb">kMove Verb</a> to path.
<a href="bmh_SkPath?cl=9919#addPoly">addPoly</a> has no effect if <a href="bmh_SkPath?cl=9919#count">count</a> is less than one.
  
### Parameters

<table>
  <tr>
    <td><code><strong>pts </strong></code></td> <td>Array of <a href="bmh_undocumented?cl=9919#Line">Line</a> sharing end and start <a href="bmh_undocumented?cl=9919#Point">Point</a>.</td>
  </tr>
  <tr>
    <td><code><strong>count </strong></code></td> <td>Length of <a href="bmh_undocumented?cl=9919#Point">Point</a> array.</td>
  </tr>
  <tr>
    <td><code><strong>close </strong></code></td> <td>true to add <a href="bmh_undocumented?cl=9919#Line">Line</a> connecting <a href="bmh_SkPath?cl=9919#Contour">Contour</a> end and start.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="182b3999772f330f3b0b891b492634ae"></fiddle-embed></div>

### See Also

<a href="bmh_SkCanvas?cl=9919#drawPoints">SkCanvas::drawPoints</a>

---

## <a name="SkPath::AddPathMode"></a> Enum SkPath::AddPathMode

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="bmh_SkPath?cl=9919#AddPathMode">AddPathMode</a> {
    <a href="bmh_SkPath?cl=9919#kAppend_AddPathMode">kAppend AddPathMode</a> 
    <a href="bmh_SkPath?cl=9919#kExtend_AddPathMode">kExtend AddPathMode</a> 
};</pre>

### Constants

<table>
  <tr>
    <td><a name="SkPath::kAppend_AddPathMode"></a> <code><strong>SkPath::kAppend_AddPathMode </strong></code></td><td>Source path contours are added as new contours.</td><td></td>
  </tr>
  <tr>
    <td><a name="SkPath::kExtend_AddPathMode"></a> <code><strong>SkPath::kExtend_AddPathMode </strong></code></td><td>Path is added by extending the last contour of the destination path</td><td>with the first contour of the source path. If the last contour of
the destination path is closed, then it will not be extended.
Instead, the start of source path will be extended by a straight
line to the end point of the destination path.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>



<a name="addPath"></a>

## addPath

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void addPath(const SkPath& src, SkScalar dx, SkScalar dy,
             AddPathMode mode = kAppend_AddPathMode)
</pre>

Add a copy of <a href="bmh_SkPath?cl=9919#src">src</a> to the path, offset by (<a href="bmh_SkPath?cl=9919#dx">dx</a>,<a href="bmh_SkPath?cl=9919#dy">dy</a>)

### Parameters

<table>
  <tr>
    <td><code><strong>src </strong></code></td> <td><a href="bmh_SkPath?cl=9919#Path">Path</a> to add starting a new <a href="bmh_SkPath?cl=9919#Contour">Contour</a>.</td>
  </tr>
  <tr>
    <td><code><strong>dx </strong></code></td> <td>offset added to <a href="bmh_SkPath?cl=9919#src">src</a> <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a> x coordinates.</td>
  </tr>
  <tr>
    <td><code><strong>dy </strong></code></td> <td>offset added to <a href="bmh_SkPath?cl=9919#src">src</a> <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a> y coordinates.</td>
  </tr>
  <tr>
    <td><code><strong>mode </strong></code></td> <td></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void addPath(const SkPath& src, AddPathMode mode = kAppend_AddPathMode)
</pre>

Add a copy of <a href="bmh_SkPath?cl=9919#src">src</a> to the path

### Parameters

<table>
  <tr>
    <td><code><strong>src </strong></code></td> <td></td>
  </tr>
  <tr>
    <td><code><strong>mode </strong></code></td> <td></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void addPath(const SkPath& src, const SkMatrix& matrix,
             AddPathMode mode = kAppend_AddPathMode)
</pre>

Add a copy of <a href="bmh_SkPath?cl=9919#src">src</a> to the path, transformed by <a href="bmh_SkPath?cl=9919#matrix">matrix</a>

### Parameters

<table>
  <tr>
    <td><code><strong>src </strong></code></td> <td>The path to add as a new contour</td>
  </tr>
  <tr>
    <td><code><strong>matrix </strong></code></td> <td>Transform applied to <a href="bmh_SkPath?cl=9919#src">src</a></td>
  </tr>
  <tr>
    <td><code><strong>mode </strong></code></td> <td>Determines how path is added</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<a name="reverseAddPath"></a>

## reverseAddPath

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void reverseAddPath(const SkPath& src)
</pre>

Same as

### Parameters

<table>
  <tr>
    <td><code><strong>src </strong></code></td> <td></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<a name="offset"></a>

## offset

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void offset(SkScalar dx, SkScalar dy, SkPath* dst) const
</pre>

Offset the path by (<a href="bmh_SkPath?cl=9919#dx">dx</a>,<a href="bmh_SkPath?cl=9919#dy">dy</a>), returning true on success

### Parameters

<table>
  <tr>
    <td><code><strong>dx </strong></code></td> <td>offset added to <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a> x coordinates.</td>
  </tr>
  <tr>
    <td><code><strong>dy </strong></code></td> <td>offset added to <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a> y coordinates.</td>
  </tr>
  <tr>
    <td><code><strong>dst </strong></code></td> <td>overwritten, translated copy of <a href="bmh_SkPath?cl=9919#Path">Path</a>.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void offset(SkScalar dx, SkScalar dy)
</pre>

Offset the path by (<a href="bmh_SkPath?cl=9919#dx">dx</a>,<a href="bmh_SkPath?cl=9919#dy">dy</a>), returning true on success

### Parameters

<table>
  <tr>
    <td><code><strong>dx </strong></code></td> <td>offset added to <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a> x coordinates.</td>
  </tr>
  <tr>
    <td><code><strong>dy </strong></code></td> <td>offset added to <a href="bmh_SkPath?cl=9919#Point_Array">Point Array</a> y coordinates.</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<a name="transform"></a>

## transform

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void transform(const SkMatrix& matrix, SkPath* dst) const
</pre>

Transform the points in this path by <a href="bmh_SkPath?cl=9919#matrix">matrix</a>, and write the answer into
<a href="bmh_SkPath?cl=9919#dst">dst</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>matrix </strong></code></td> <td>The <a href="bmh_SkPath?cl=9919#matrix">matrix</a> to apply to the path</td>
  </tr>
  <tr>
    <td><code><strong>dst </strong></code></td> <td>The transformed path is written here</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void transform(const SkMatrix& matrix)
</pre>

Transform the points in this path by <a href="bmh_SkPath?cl=9919#matrix">matrix</a>

### Parameters

<table>
  <tr>
    <td><code><strong>matrix </strong></code></td> <td>The <a href="bmh_SkPath?cl=9919#matrix">matrix</a> to apply to the path</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

## <a name="Last_Point"></a> Last Point

<a href="bmh_SkPath?cl=9919#Path">Path</a> is defined cumulatively, often by adding a segment to the end of last
<a href="bmh_SkPath?cl=9919#Contour">Contour</a>. <a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> of <a href="bmh_SkPath?cl=9919#Contour">Contour</a> is shared as first <a href="bmh_undocumented?cl=9919#Point">Point</a> of added <a href="bmh_undocumented?cl=9919#Line">Line</a> or <a href="bmh_undocumented?cl=9919#Curve">Curve</a>.
<a href="bmh_SkPath?cl=9919#Last_Point">Last Point</a> can be read and written directly with <a href="bmh_SkPath?cl=9919#getLastPt">getLastPt</a> and <a href="bmh_SkPath?cl=9919#setLastPt">setLastPt</a>.

<a name="getLastPt"></a>

## getLastPt

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool getLastPt(SkPoint* lastPt) const
</pre>

Return the last point on the path. If no points have been added, (0,0)
is returned. If there are no points, this returns false, otherwise it
returns true.

### Parameters

<table>
  <tr>
    <td><code><strong>lastPt </strong></code></td> <td>The last point on the path is returned here</td>
  </tr>
</table>

### Return Value

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<a name="setLastPt"></a>

## setLastPt

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setLastPt(SkScalar x, SkScalar y)
</pre>

Set the last point on the path. If no points have been added,

### Parameters

<table>
  <tr>
    <td><code><strong>x </strong></code></td> <td>The new x-coordinate for the last point</td>
  </tr>
  <tr>
    <td><code><strong>y </strong></code></td> <td>The new y-coordinate for the last point</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setLastPt(const SkPoint& p)
</pre>

Set the last point on the path. If no points have been added,

### Parameters

<table>
  <tr>
    <td><code><strong>p </strong></code></td> <td>The new location for the last point</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

## <a name="SkPath::SegmentMask"></a> Enum SkPath::SegmentMask

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
enum <a href="bmh_SkPath?cl=9919#SegmentMask">SegmentMask</a> {
    <a href="bmh_SkPath?cl=9919#kLine_SegmentMask">kLine SegmentMask</a> = 1 << 0
    <a href="bmh_SkPath?cl=9919#kQuad_SegmentMask">kQuad SegmentMask</a> = 1 << 1
    <a href="bmh_SkPath?cl=9919#kConic_SegmentMask">kConic SegmentMask</a> = 1 << 2
    <a href="bmh_SkPath?cl=9919#kCubic_SegmentMask">kCubic SegmentMask</a> = 1 << 3
};</pre>

### Constants

<table>
  <tr>
    <td><a name="SkPath::kLine_SegmentMask"></a> <code><strong>SkPath::kLine_SegmentMask </strong></code></td><td>= 1 << 0</td><td></td>
  </tr>
  <tr>
    <td><a name="SkPath::kQuad_SegmentMask"></a> <code><strong>SkPath::kQuad_SegmentMask </strong></code></td><td>= 1 << 1</td><td></td>
  </tr>
  <tr>
    <td><a name="SkPath::kConic_SegmentMask"></a> <code><strong>SkPath::kConic_SegmentMask </strong></code></td><td>= 1 << 2</td><td></td>
  </tr>
  <tr>
    <td><a name="SkPath::kCubic_SegmentMask"></a> <code><strong>SkPath::kCubic_SegmentMask </strong></code></td><td>= 1 << 3</td><td></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>



<a name="getSegmentMasks"></a>

## getSegmentMasks

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
uint32_t getSegmentMasks() const
</pre>

Returns a mask, where each bit corresponding to a <a href="bmh_SkPath?cl=9919#SegmentMask">SegmentMask</a> is
set if the path contains 1 or more segments of that type.
Returns 0 for an empty path (no segments).

### Return Value

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<a name="contains"></a>

## contains

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool contains(SkScalar x, SkScalar y) const
</pre>

Returns true if the point { <a href="bmh_SkPath?cl=9919#x">x</a>, <a href="bmh_SkPath?cl=9919#y">y</a> } is contained by the path, taking into
account the <a href="bmh_SkPath?cl=9919#FillType">FillType</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>x </strong></code></td> <td></td>
  </tr>
  <tr>
    <td><code><strong>y </strong></code></td> <td></td>
  </tr>
</table>

### Return Value

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<a name="dump"></a>

## dump

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void dump(SkWStream* stream, bool forceClose, bool dumpAsHex) const
</pre>

### Parameters

<table>
  <tr>
    <td><code><strong>stream </strong></code></td> <td></td>
  </tr>
  <tr>
    <td><code><strong>forceClose </strong></code></td> <td></td>
  </tr>
  <tr>
    <td><code><strong>dumpAsHex </strong></code></td> <td></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void dump() const
</pre>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<a name="dumpHex"></a>

## dumpHex

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void dumpHex() const
</pre>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<a name="writeToMemory"></a>

## writeToMemory

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
size_t writeToMemory(void* buffer) const
</pre>

Write the path to the <a href="bmh_SkPath?cl=9919#buffer">buffer</a>, and return the number of bytes written.
If <a href="bmh_SkPath?cl=9919#buffer">buffer</a> is <a href="bmh_undocumented?cl=9919#NULL">NULL</a>, it still returns the number of bytes.

### Parameters

<table>
  <tr>
    <td><code><strong>buffer </strong></code></td> <td></td>
  </tr>
</table>

### Return Value

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<a name="readFromMemory"></a>

## readFromMemory

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
size_t readFromMemory(const void* buffer, size_t length)
</pre>

Initializes the path from the <a href="bmh_SkPath?cl=9919#buffer">buffer</a>

### Parameters

<table>
  <tr>
    <td><code><strong>buffer </strong></code></td> <td>Memory to read from</td>
  </tr>
  <tr>
    <td><code><strong>length </strong></code></td> <td>Amount of memory available in the <a href="bmh_SkPath?cl=9919#buffer">buffer</a></td>
  </tr>
</table>

### Return Value

number of bytes read (must be a multiple of 4) or

0 if there was not enough memory available

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

# <a name="Generation_ID"></a> Generation ID

<a name="getGenerationID"></a>

## getGenerationID

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
uint32_t getGenerationID() const
</pre>

Returns a non-zero, globally unique value corresponding to the set of verbs
and points in the path (but not the fill type [except on <a href="bmh_undocumented?cl=9919#Android">Android</a> skbug.com/1762]).
Each time the path is modified, a different <a href="bmh_SkPath?cl=9919#Generation_ID">Generation ID</a> will be returned.

### Return Value

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<a name="validate"></a>

## validate

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void validate() const
</pre>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

<a name="experimentalValidateRef"></a>

## experimentalValidateRef

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void experimentalValidateRef() const { fPathRef->validate() ; }
</pre>

### Example

<div><fiddle-embed name="08293376ae295e685d165a5c2d625957"></fiddle-embed></div>

---

# <a name="Iter"></a> Class Iter
Iterate through all of the segments (lines, quadratics, cubics) of
each contours in a path.

### Example

<div><fiddle-embed name="97e9b8256503d5ecd36e5f2a840e1dee"></fiddle-embed></div>

<a name="empty_constructor"></a>

## Iter

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
Iter()
</pre>

### Return Value

---

<a name="const_SkPath"></a>

## Iter

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
Iter(const SkPath& path, bool forceClose)
</pre>

The iterator cleans up the segments along the way, removing degenerate
segments and adding close verbs where necessary. When the <a href="bmh_SkPath?cl=9919#forceClose">forceClose</a>
argument is provided, each contour (as defined by a new starting
move command) will be completed with a close verb regardless of the
contour's contents.

### Parameters

<table>
  <tr>
    <td><code><strong>path </strong></code></td> <td></td>
  </tr>
  <tr>
    <td><code><strong>forceClose </strong></code></td> <td></td>
  </tr>
</table>

### Return Value

---

<a name="setPath"></a>

## setPath

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setPath(const SkPath& path, bool forceClose)
</pre>

### Parameters

<table>
  <tr>
    <td><code><strong>path </strong></code></td> <td></td>
  </tr>
  <tr>
    <td><code><strong>forceClose </strong></code></td> <td></td>
  </tr>

---

</table>

<a name="next"></a>

## next

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
Verb next(SkPoint pts[4], bool doConsumeDegenerates = true, bool exact = false)
</pre>

Return the next verb in this iteration of the path. When all
segments have been visited, return <a href="bmh_SkPath?cl=9919#kDone_Verb">kDone Verb</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>pts </strong></code></td> <td>The points representing the current verb and/or segment</td>
  </tr>
  <tr>
    <td><code><strong>doConsumeDegenerates </strong></code></td> <td>If true, first scan for segments that are
                deemed degenerate (too short) and skip those.</td>
  </tr>
  <tr>
    <td><code><strong>exact </strong></code></td> <td>if <a href="bmh_SkPath?cl=9919#next">doConsumeDegenerates</a> is true and <a href="bmh_SkPath?cl=9919#next">exact</a> is true, skip only
                degenerate elements with lengths exactly equal to zero. If <a href="bmh_SkPath?cl=9919#next">exact</a>
                is false, skip degenerate elements with lengths close to zero. If
                <a href="bmh_SkPath?cl=9919#next">doConsumeDegenerates</a> is false, <a href="bmh_SkPath?cl=9919#next">exact</a> has no effect.</td>
  </tr>
</table>

### Return Value

The verb for the current segment

---

<a name="conicWeight"></a>

## conicWeight

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar conicWeight() const
</pre>

Return the weight for the current conic. Only valid if the current
segment return by <a href="bmh_SkPath?cl=9919#next">next</a> was a conic.

### Return Value

---

<a name="isCloseLine"></a>

## isCloseLine

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isCloseLine() const
</pre>

If <a href="bmh_SkPath?cl=9919#next">next</a> returns <a href="bmh_SkPath?cl=9919#kLine_Verb">kLine Verb</a>, then this query returns true if the
    line was the result of a <a href="bmh_SkPath?cl=9919#close">close</a> command (i.e. the end point is the
    initial moveto for this contour). If <a href="bmh_SkPath?cl=9919#next">next</a> returned a different
    verb, this returns an undefined value.

If the last call to <a href="bmh_SkPath?cl=9919#next">next</a> returned <a href="bmh_SkPath?cl=9919#kLine_Verb">kLine Verb</a>, return true
        if it was the result of an explicit close command.

### Return Value

true if last <a href="bmh_SkPath?cl=9919#kLine_Verb">kLine Verb</a> was generated by <a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a>.

---

<a name="isClosedContour"></a>

## isClosedContour

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
bool isClosedContour() const
</pre>

Returns true if the current contour is closed (has a <a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a>)

### Return Value

true if the current contour is closed (has a <a href="bmh_SkPath?cl=9919#kClose_Verb">kClose Verb</a>)

---

# <a name="RawIter"></a> Class RawIter

<a name="empty_constructor"></a>

## RawIter

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
RawIter()
</pre>

### Return Value

---

<a name="copy_constructor"></a>

## RawIter

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
RawIter(const SkPath& path)
</pre>

### Parameters

<table>
  <tr>
    <td><code><strong>path </strong></code></td> <td></td>
  </tr>
</table>

### Return Value

---

<a name="setPath"></a>

## setPath

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
void setPath(const SkPath& path)
</pre>

### Parameters

<table>
  <tr>
    <td><code><strong>path </strong></code></td> <td></td>
  </tr>

---

</table>

<a name="next"></a>

## next

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
Verb next(SkPoint pts[4])
</pre>

Return the next verb in this iteration of the path. When all
            segments have been visited, return <a href="bmh_SkPath?cl=9919#kDone_Verb">kDone Verb</a>.

### Parameters

<table>
  <tr>
    <td><code><strong>pts </strong></code></td> <td>The points representing the current verb and/or segment.
                        This must not be <a href="bmh_undocumented?cl=9919#NULL">NULL</a>.</td>
  </tr>
</table>

### Return Value

The verb for the current segment

---

<a name="peek"></a>

## peek

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
Verb peek() const
</pre>

Return what the next verb will be, but do not visit the next segment.

### Return Value

The verb for the next segment

---

<a name="conicWeight"></a>

## conicWeight

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
SkScalar conicWeight() const
</pre>

### Return Value

---

