SkPath Reference
===

# <a name='Path'>Path</a>
<a href='#Path'>Path</a> contains <a href='undocumented#Line'>Lines</a> and <a href='undocumented#Curve'>Curves</a> which can be stroked or filled. <a href='#Contour'>Contour</a> is
composed of a series of connected <a href='undocumented#Line'>Lines</a> and <a href='undocumented#Curve'>Curves</a>. <a href='#Path'>Path</a> may contain zero,
one, or more <a href='#Contour'>Contours</a>.
Each <a href='undocumented#Line'>Line</a> and <a href='undocumented#Curve'>Curve</a> are described by <a href='#SkPath_Verb'>Verb</a>, <a href='SkPoint_Reference#Point'>Points</a>, and optional <a href='#Conic_Weight'>Conic Weight</a>.

Each pair of connected <a href='undocumented#Line'>Lines</a> and <a href='undocumented#Curve'>Curves</a> share common <a href='SkPoint_Reference#Point'>Point</a>; for instance, <a href='#Path'>Path</a>
containing two connected <a href='undocumented#Line'>Lines</a> are described the <a href='#SkPath_Verb'>Verb</a> sequence:
<a href='#SkPath_kMove_Verb'>SkPath::kMove Verb</a>, <a href='#SkPath_kLine_Verb'>SkPath::kLine Verb</a>, <a href='#SkPath_kLine_Verb'>SkPath::kLine Verb</a>; and a <a href='SkPoint_Reference#Point'>Point</a> sequence
with three entries, sharing
the middle entry as the end of the first <a href='undocumented#Line'>Line</a> and the start of the second <a href='undocumented#Line'>Line</a>.

<a href='#Path'>Path</a> components <a href='#Arc'>Arc</a>, <a href='SkRect_Reference#Rect'>Rect</a>, <a href='SkRRect_Reference#RRect'>Round Rect</a>, <a href='undocumented#Circle'>Circle</a>, and <a href='undocumented#Oval'>Oval</a> are composed of
<a href='undocumented#Line'>Lines</a> and <a href='undocumented#Curve'>Curves</a> with as many <a href='#Verb'>Verbs</a> and <a href='SkPoint_Reference#Point'>Points</a> required
for an exact description. Once added to <a href='#Path'>Path</a>, these components may lose their
identity; although <a href='#Path'>Path</a> can be inspected to determine if it describes a single
<a href='SkRect_Reference#Rect'>Rect</a>, <a href='undocumented#Oval'>Oval</a>, <a href='SkRRect_Reference#RRect'>Round Rect</a>, and so on.

### Example

<div><fiddle-embed name="93887af0c1dac49521972698cf04069c"><div><a href='#Path'>Path</a> contains three <a href='#Contour'>Contours</a>: <a href='undocumented#Line'>Line</a>, <a href='undocumented#Circle'>Circle</a>, and <a href='#Quad'>Quad</a>. <a href='undocumented#Line'>Line</a> is stroked but
not filled. <a href='undocumented#Circle'>Circle</a> is stroked and filled; <a href='undocumented#Circle'>Circle</a> stroke forms a loop. <a href='#Quad'>Quad</a>
is stroked and filled, but since it is not closed, <a href='#Quad'>Quad</a> does not stroke a loop.
</div></fiddle-embed></div>

<a href='#Path'>Path</a> contains a <a href='#Fill_Type'>Fill Type</a> which determines whether overlapping <a href='#Contour'>Contours</a>
form fills or holes. <a href='#Fill_Type'>Fill Type</a> also determines whether area inside or outside
<a href='undocumented#Line'>Lines</a> and <a href='undocumented#Curve'>Curves</a> is filled.

### Example

<div><fiddle-embed name="36a995442c081ee779ecab2962d36e69"><div><a href='#Path'>Path</a> is drawn filled, then stroked, then stroked and filled.
</div></fiddle-embed></div>

<a href='#Path'>Path</a> contents are never shared. Copying <a href='#Path'>Path</a> by value effectively creates
a new <a href='#Path'>Path</a> independent of the original. Internally, the copy does not duplicate
its contents until it is edited, to reduce memory use and improve performance.

## <a name='Contour'>Contour</a>

<a href='#Contour'>Contour</a> contains one or more <a href='#Verb'>Verbs</a>, and as many <a href='SkPoint_Reference#Point'>Points</a> as
are required to satisfy <a href='#Verb_Array'>Verb Array</a>. First <a href='#SkPath_Verb'>Verb</a> in <a href='#Path'>Path</a> is always
<a href='#SkPath_kMove_Verb'>SkPath::kMove Verb</a>; each <a href='#SkPath_kMove_Verb'>SkPath::kMove Verb</a> that follows starts a new <a href='#Contour'>Contour</a>.

### Example

<div><fiddle-embed name="0374f2dcd7effeb1dd435205a6c2de6f"><div>Each <a href='#SkPath_moveTo'>SkPath::moveTo</a> starts a new <a href='#Contour'>Contour</a>, and content after <a href='#SkPath_close'>SkPath::close()</a>
also starts a new <a href='#Contour'>Contour</a>. Since <a href='#SkPath_conicTo'>SkPath::conicTo</a> is not preceded by
<a href='#SkPath_moveTo'>SkPath::moveTo</a>, the first <a href='SkPoint_Reference#Point'>Point</a> of the third <a href='#Contour'>Contour</a> starts at the last <a href='SkPoint_Reference#Point'>Point</a>
of the second <a href='#Contour'>Contour</a>.
</div></fiddle-embed></div>

If final <a href='#SkPath_Verb'>Verb</a> in <a href='#Contour'>Contour</a> is <a href='#SkPath_kClose_Verb'>SkPath::kClose Verb</a>, <a href='undocumented#Line'>Line</a> connects <a href='#Last_Point'>Last Point</a> in
<a href='#Contour'>Contour</a> with first <a href='SkPoint_Reference#Point'>Point</a>. A closed <a href='#Contour'>Contour</a>, stroked, draws
<a href='SkPaint_Reference#Stroke_Join'>Paint Stroke Join</a> at <a href='#Last_Point'>Last Point</a> and first <a href='SkPoint_Reference#Point'>Point</a>. Without <a href='#SkPath_kClose_Verb'>SkPath::kClose Verb</a>
as final <a href='#SkPath_Verb'>Verb</a>, <a href='#Last_Point'>Last Point</a> and first <a href='SkPoint_Reference#Point'>Point</a> are not connected; <a href='#Contour'>Contour</a>
remains open. An open <a href='#Contour'>Contour</a>, stroked, draws <a href='SkPaint_Reference#Stroke_Cap'>Paint Stroke Cap</a> at
<a href='#Last_Point'>Last Point</a> and first <a href='SkPoint_Reference#Point'>Point</a>.

### Example

<div><fiddle-embed name="7a1f39b12d2cd8b7f5b1190879259cb2"><div><a href='#Path'>Path</a> is drawn stroked, with an open <a href='#Contour'>Contour</a> and a closed <a href='#Contour'>Contour</a>.
</div></fiddle-embed></div>

## <a name='Contour_Zero_Length'>Contour Zero Length</a>

<a href='#Contour'>Contour</a> length is distance traveled from first <a href='SkPoint_Reference#Point'>Point</a> to <a href='#Last_Point'>Last Point</a>,
plus, if <a href='#Contour'>Contour</a> is closed, distance from <a href='#Last_Point'>Last Point</a> to first <a href='SkPoint_Reference#Point'>Point</a>.
Even if <a href='#Contour'>Contour</a> length is zero, stroked <a href='undocumented#Line'>Lines</a> are drawn if <a href='SkPaint_Reference#Stroke_Cap'>Paint Stroke Cap</a>
makes them visible.

### Example

<div><fiddle-embed name="62848df605af6258653d9e16b27d8f7f"></fiddle-embed></div>

# <a name='SkPath'>Class SkPath</a>
<a href='#Path'>Paths</a> contain geometry. <a href='#Path'>Paths</a> may be empty, or contain one or more <a href='#Verb'>Verbs</a> that
outline a figure. <a href='#Path'>Path</a> always starts with a move verb to a <a href='undocumented#Cartesian_Coordinate'>Cartesian Coordinate</a>,
and may be followed by additional verbs that add lines or curves.
Adding a close verb makes the geometry into a continuous loop, a closed contour.
<a href='#Path'>Paths</a> may contain any number of contours, each beginning with a move verb.

<a href='#Path'>Path</a> contours may contain only a move verb, or may also contain lines,
<a href='#Quad'>Quadratic Beziers</a>, <a href='#Conic'>Conics</a>, and <a href='#Cubic'>Cubic Beziers</a>. <a href='#Path'>Path</a> contours may be open or
closed.

When used to draw a filled area, <a href='#Path'>Path</a> describes whether the fill is inside or
outside the geometry. <a href='#Path'>Path</a> also describes the winding rule used to fill
overlapping contours.

Internally, <a href='#Path'>Path</a> lazily computes metrics likes bounds and convexity. Call
<a href='#SkPath_updateBoundsCache'>SkPath::updateBoundsCache</a> to make <a href='#Path'>Path</a> thread safe.

## Overview

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Class'>Class Declarations</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>embedded class members</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Constant'>Constants</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>enum and enum class, and their const values</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Constructor'>Constructors</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>functions that construct <a href='#SkPath'>SkPath</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Member_Function'>Functions</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>global and class member functions</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Operator'>Operators</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>operator overloading methods</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Related_Function'>Related Functions</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>similar member functions grouped together</td>
  </tr>
</table>


## <a name='Related_Function'>Related Function</a>


SkPath global, <code>struct</code>, and <code>class</code> related member functions share a topic.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Arc'>Arc</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>part of <a href='undocumented#Oval'>Oval</a> or <a href='undocumented#Circle'>Circle</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Build'>Build</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds points and verbs to path</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Conic'>Conic</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>conic section defined by three points and a weight</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Conic_Weight'>Conic Weight</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>strength of <a href='#Conic'>Conic</a> control <a href='SkPoint_Reference#Point'>Point</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Contour'>Contour</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>loop of lines and curves</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Contour_Zero_Length'>Contour Zero Length</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>consideration when contour has no length</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_Convexity'>Convexity</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>if <a href='#Path'>Path</a> is concave or convex</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Cubic'>Cubic</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>curve described by third-order polynomial</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_Direction'>Direction</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>contour orientation, clockwise or counterclockwise</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Fill_Type'>Fill Type</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>fill rule, normal and inverted</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Generation_ID'>Generation ID</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>value reflecting contents change</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Interpolate'>Interpolate</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>weighted average of <a href='#Path'>Path</a> pair</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Last_Point'>Last Point</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>final <a href='SkPoint_Reference#Point'>Point</a> in <a href='#Contour'>Contour</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Point_Array'>Point Array</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>end points and control points for lines and curves</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Property'>Property</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>metrics and attributes</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Quad'>Quad</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>curve described by second-order polynomial</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Transform'>Transform</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>modify all points</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Utility'>Utility</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>rarely called management functions</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_Verb'>Verb</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>line and curve type</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Verb_Array'>Verb Array</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>line and curve type for points</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Volatile'>Volatile</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>caching attribute</td>
  </tr>
</table>

## <a name='Constant'>Constant</a>


SkPath related constants are defined by <code>enum</code>, <code>enum class</code>,  <code>#define</code>, <code>const</code>, and <code>constexpr</code>.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_AddPathMode'>AddPathMode</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets <a href='#SkPath_addPath'>addPath</a> options</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_ArcSize'>ArcSize</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>used by <a href='#SkPath_arcTo'>arcTo</a> variation</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_Convexity'>Convexity</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if <a href='#Path'>Path</a> is convex or concave</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_Direction'>Direction</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets <a href='#Contour'>Contour</a> clockwise or counterclockwise</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_FillType'>FillType</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets winding rule and inverse fill</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_SegmentMask'>SegmentMask</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#SkPath_Verb'>Verb</a> types in <a href='#Path'>Path</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_Verb'>Verb</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>controls how <a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Points</a> are interpreted</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_kAppend_AddPathMode'>kAppend AddPathMode</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>appended to destination unaltered</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_kCCW_Direction'>kCCW Direction</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>contour travels counterclockwise</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_kCW_Direction'>kCW Direction</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>contour travels clockwise</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_kClose_Verb'>kClose Verb</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>closes <a href='#Contour'>Contour</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_kConcave_Convexity'>kConcave Convexity</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>more than one <a href='#Contour'>Contour</a>, or a geometry with indentations</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_kConic_SegmentMask'>kConic SegmentMask</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>contains one or more <a href='#Conic'>Conics</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_kConic_Verb'>kConic Verb</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds <a href='#Conic'>Conic</a> from <a href='#Last_Point'>Last Point</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_kConvex_Convexity'>kConvex Convexity</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>one <a href='#Contour'>Contour</a> made of a simple geometry without indentations</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_kCubic_SegmentMask'>kCubic SegmentMask</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>contains one or more <a href='#Cubic'>Cubics</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_kCubic_Verb'>kCubic Verb</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds <a href='#Cubic'>Cubic</a> from <a href='#Last_Point'>Last Point</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_kDone_Verb'>kDone Verb</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>terminates <a href='#Path'>Path</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_kEvenOdd_FillType'>kEvenOdd FillType</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>is enclosed by an odd number of <a href='#Contour'>Contours</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_kExtend_AddPathMode'>kExtend AddPathMode</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>add line if prior <a href='#Contour'>Contour</a> is not closed</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd FillType</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>is enclosed by an even number of <a href='#Contour'>Contours</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_kInverseWinding_FillType'>kInverseWinding FillType</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>is enclosed by a zero sum of <a href='#Contour'>Contour</a> <a href='#Direction'>Directions</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_kLarge_ArcSize'>kLarge ArcSize</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>larger of <a href='#Arc'>Arc</a> pair</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_kLine_SegmentMask'>kLine SegmentMask</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>contains one or more <a href='undocumented#Line'>Lines</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_kLine_Verb'>kLine Verb</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds <a href='undocumented#Line'>Line</a> from <a href='#Last_Point'>Last Point</a> to next <a href='SkPoint_Reference#Point'>Point</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_kMove_Verb'>kMove Verb</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>starts new <a href='#Contour'>Contour</a> at next <a href='SkPoint_Reference#Point'>Point</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_kQuad_SegmentMask'>kQuad SegmentMask</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>contains one or more <a href='#Quad'>Quads</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_kQuad_Verb'>kQuad Verb</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds <a href='#Quad'>Quad</a> from <a href='#Last_Point'>Last Point</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_kSmall_ArcSize'>kSmall ArcSize</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>smaller of <a href='#Arc'>Arc</a> pair</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_kUnknown_Convexity'>kUnknown Convexity</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>indicates <a href='#SkPath_Convexity'>Convexity</a> has not been determined</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_kWinding_FillType'>kWinding FillType</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>is enclosed by a non-zero sum of <a href='#Contour'>Contour</a> <a href='#Direction'>Directions</a></td>
  </tr>
</table>

## <a name='Class'>Class</a>


SkPath uses C++ classes to declare the public data structures and interfaces.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_Iter'>Iter</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>data iterator</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_RawIter'>RawIter</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>raw data iterator</td>
  </tr>
</table>

## <a name='Constructor'>Constructor</a>


SkPath can be constructed or initialized by these functions, including C++ class constructors.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_empty_constructor'>SkPath()</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>constructs with default values</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_copy_const_SkPath'>SkPath(const SkPath& path)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>makes a shallow copy</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_reset'>reset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>removes <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, and <a href='#Conic_Weight'>Weights</a>; frees memory</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_rewind'>rewind</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>removes <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, and <a href='#Conic_Weight'>Weights</a>, keeping memory</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_destructor'>~SkPath()</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>decreases <a href='undocumented#Reference_Count'>Reference Count</a> of owned objects</td>
  </tr>
</table>

## <a name='Operator'>Operator</a>


SkPath operators inline class member functions with arithmetic equivalents.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_notequal_operator'>operator!=(const SkPath& a, const SkPath& b)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>compares paths for inequality</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_copy_operator'>operator=(const SkPath& path)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>makes a shallow copy</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_equal_operator'>operator==(const SkPath& a, const SkPath& b)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>compares paths for equality</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_swap'>swap</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>exchanges <a href='#Path'>Path</a> pair</td>
  </tr>
</table>

## <a name='Member_Function'>Member Function</a>


SkPath member functions read and modify the structure properties.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_ConvertConicToQuads'>ConvertConicToQuads</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>approximates <a href='#Conic'>Conic</a> with <a href='#Quad'>Quad</a> array</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_ConvertToNonInverseFillType'>ConvertToNonInverseFillType</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#Fill_Type'>Fill Type</a> representing inside geometry</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_IsCubicDegenerate'>IsCubicDegenerate</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if <a href='#Cubic'>Cubic</a> is very small</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_IsInverseFillType'>IsInverseFillType</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if <a href='#Fill_Type'>Fill Type</a> represents outside geometry</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_IsLineDegenerate'>IsLineDegenerate</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if <a href='undocumented#Line'>Line</a> is very small</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_IsQuadDegenerate'>IsQuadDegenerate</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if <a href='#Quad'>Quad</a> is very small</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addArc'>addArc</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds one <a href='#Contour'>Contour</a> containing <a href='#Arc'>Arc</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addCircle'>addCircle</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds one <a href='#Contour'>Contour</a> containing <a href='undocumented#Circle'>Circle</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addOval'>addOval</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds one <a href='#Contour'>Contour</a> containing <a href='undocumented#Oval'>Oval</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addPath'>addPath</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds contents of <a href='#Path'>Path</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addPoly'>addPoly</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds one <a href='#Contour'>Contour</a> containing connected lines</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addRRect'>addRRect</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds one <a href='#Contour'>Contour</a> containing <a href='SkRRect_Reference#RRect'>Round Rect</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addRect'>addRect</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds one <a href='#Contour'>Contour</a> containing <a href='SkRect_Reference#Rect'>Rect</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addRoundRect'>addRoundRect</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds one <a href='#Contour'>Contour</a> containing <a href='SkRRect_Reference#RRect'>Round Rect</a> with common corner radii</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_arcTo'>arcTo</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>appends <a href='#Arc'>Arc</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_close'>close</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>makes last <a href='#Contour'>Contour</a> a loop</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_computeTightBounds'>computeTightBounds</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns extent of geometry</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_conicTo'>conicTo</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>appends <a href='#Conic'>Conic</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_conservativelyContainsRect'>conservativelyContainsRect</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if <a href='SkRect_Reference#Rect'>Rect</a> may be inside</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_contains'>contains</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if <a href='SkPoint_Reference#Point'>Point</a> is in fill area</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_countPoints'>countPoints</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#Point_Array'>Point Array</a> length</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_countVerbs'>countVerbs</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#Verb_Array'>Verb Array</a> length</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_cubicTo'>cubicTo</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>appends <a href='#Cubic'>Cubic</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_dump_2'>dump</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sends text representation to stream</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_dumpHex'>dumpHex</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sends text representation using hexadecimal to standard output</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_getBounds'>getBounds</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns maximum and minimum of <a href='#Point_Array'>Point Array</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_getConvexity'>getConvexity</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns geometry convexity, computing if necessary</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_getConvexityOrUnknown'>getConvexityOrUnknown</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns geometry convexity if known</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_getFillType'>getFillType</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#Fill_Type'>Fill Type</a>: winding, even-odd, inverse</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_getGenerationID'>getGenerationID</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns unique ID</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_getLastPt'>getLastPt</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#Last_Point'>Last Point</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_getPoint'>getPoint</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns entry from <a href='#Point_Array'>Point Array</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_getPoints'>getPoints</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#Point_Array'>Point Array</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_getSegmentMasks'>getSegmentMasks</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns types in <a href='#Verb_Array'>Verb Array</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_getVerbs'>getVerbs</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#Verb_Array'>Verb Array</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_incReserve'>incReserve</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>reserves space for additional data</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_interpolate'>interpolate</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>interpolates between <a href='#Path'>Path</a> pair</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_isConvex'>isConvex</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if geometry is convex</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_isEmpty'>isEmpty</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if verb count is zero</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_isFinite'>isFinite</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if all <a href='SkPoint_Reference#Point'>Point</a> values are finite</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_isInterpolatable'>isInterpolatable</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if pair contains equal counts of <a href='#Verb_Array'>Verb Array</a> and <a href='#Conic_Weight'>Weights</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_isInverseFillType'>isInverseFillType</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if <a href='#Fill_Type'>Fill Type</a> fills outside geometry</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_isLastContourClosed'>isLastContourClosed</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if final <a href='#Contour'>Contour</a> forms a loop</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_isLine'>isLine</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if describes <a href='undocumented#Line'>Line</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_isNestedFillRects'>isNestedFillRects</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if describes <a href='SkRect_Reference#Rect'>Rect</a> pair, one inside the other</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_isOval'>isOval</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if describes <a href='undocumented#Oval'>Oval</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_isRRect'>isRRect</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if describes <a href='SkRRect_Reference#RRect'>Round Rect</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_isRect'>isRect</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if describes <a href='SkRect_Reference#Rect'>Rect</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_isValid'>isValid</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if data is internally consistent</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_isVolatile'>isVolatile</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if <a href='undocumented#Device'>Device</a> should not cache</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_lineTo'>lineTo</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>appends <a href='undocumented#Line'>Line</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_moveTo'>moveTo</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>starts <a href='#Contour'>Contour</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_offset'>offset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>translates <a href='#Point_Array'>Point Array</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_quadTo'>quadTo</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>appends <a href='#Quad'>Quad</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_rArcTo'>rArcTo</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>appends <a href='#Arc'>Arc</a> relative to <a href='#Last_Point'>Last Point</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_rConicTo'>rConicTo</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>appends <a href='#Conic'>Conic</a> relative to <a href='#Last_Point'>Last Point</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_rCubicTo'>rCubicTo</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>appends <a href='#Cubic'>Cubic</a> relative to <a href='#Last_Point'>Last Point</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_rLineTo'>rLineTo</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>appends <a href='undocumented#Line'>Line</a> relative to <a href='#Last_Point'>Last Point</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_rMoveTo'>rMoveTo</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>starts <a href='#Contour'>Contour</a> relative to <a href='#Last_Point'>Last Point</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_rQuadTo'>rQuadTo</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>appends <a href='#Quad'>Quad</a> relative to <a href='#Last_Point'>Last Point</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_readFromMemory'>readFromMemory</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>initializes from buffer</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_reset'>reset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>removes <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, and <a href='#Conic_Weight'>Weights</a>; frees memory</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_reverseAddPath'>reverseAddPath</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds contents of <a href='#Path'>Path</a> back to front</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_rewind'>rewind</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>removes <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, and <a href='#Conic_Weight'>Weights</a>, keeping memory</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_serialize'>serialize</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>copies data to buffer</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_setConvexity'>setConvexity</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets if geometry is convex to avoid future computation</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_setFillType'>setFillType</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets <a href='#Fill_Type'>Fill Type</a>: winding, even-odd, inverse</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_setIsVolatile'>setIsVolatile</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets if <a href='undocumented#Device'>Device</a> should not cache</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_setLastPt'>setLastPt</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>replaces <a href='#Last_Point'>Last Point</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_swap'>swap</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>exchanges <a href='#Path'>Path</a> pair</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_toggleInverseFillType'>toggleInverseFillType</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>toggles <a href='#Fill_Type'>Fill Type</a> between inside and outside geometry</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_transform'>transform</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>applies <a href='SkMatrix_Reference#Matrix'>Matrix</a> to <a href='#Point_Array'>Point Array</a> and <a href='#Conic_Weight'>Weights</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_updateBoundsCache'>updateBoundsCache</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>refreshes result of <a href='#SkPath_getBounds'>getBounds</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_writeToMemory'>writeToMemory</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>copies data to buffer</td>
  </tr>
</table>

## <a name='Verb'>Verb</a>

## <a name='SkPath_Verb'>Enum SkPath::Verb</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPath_Verb'>Verb</a> {
        <a href='#SkPath_kMove_Verb'>kMove Verb</a>,
        <a href='#SkPath_kLine_Verb'>kLine Verb</a>,
        <a href='#SkPath_kQuad_Verb'>kQuad Verb</a>,
        <a href='#SkPath_kConic_Verb'>kConic Verb</a>,
        <a href='#SkPath_kCubic_Verb'>kCubic Verb</a>,
        <a href='#SkPath_kClose_Verb'>kClose Verb</a>,
        <a href='#SkPath_kDone_Verb'>kDone Verb</a>,
    };
</pre>

<a href='#SkPath_Verb'>Verb</a> instructs <a href='#Path'>Path</a> how to interpret one or more <a href='SkPoint_Reference#Point'>Point</a> and optional <a href='#Conic_Weight'>Conic Weight</a>;
manage <a href='#Contour'>Contour</a>, and terminate <a href='#Path'>Path</a>.

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
ignored. <a href='#SkPath_kMove_Verb'>kMove Verb</a> after other <a href='#Verb'>Verbs</a> implicitly closes the previous <a href='#Contour'>Contour</a>
if <a href='SkPaint_Reference#SkPaint_kFill_Style'>SkPaint::kFill Style</a> is set when drawn; otherwise, stroke is drawn open.
<a href='#SkPath_kMove_Verb'>kMove Verb</a> as the last <a href='#SkPath_Verb'>Verb</a> is preserved but ignored.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kLine_Verb'><code>SkPath::kLine_Verb</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='undocumented#Line'>Line</a> is a straight segment from <a href='SkPoint_Reference#Point'>Point</a> to <a href='SkPoint_Reference#Point'>Point</a>. Consecutive <a href='#SkPath_kLine_Verb'>kLine Verb</a>
extend <a href='#Contour'>Contour</a>. <a href='#SkPath_kLine_Verb'>kLine Verb</a> at same position as prior <a href='#SkPath_kMove_Verb'>kMove Verb</a> is
preserved, and draws <a href='SkPoint_Reference#Point'>Point</a> if <a href='SkPaint_Reference#SkPaint_kStroke_Style'>SkPaint::kStroke Style</a> is set, and
<a href='SkPaint_Reference#SkPaint_Cap'>SkPaint::Cap</a> is <a href='SkPaint_Reference#SkPaint_kSquare_Cap'>SkPaint::kSquare Cap</a> or <a href='SkPaint_Reference#SkPaint_kRound_Cap'>SkPaint::kRound Cap</a>. <a href='#SkPath_kLine_Verb'>kLine Verb</a>
at same position as prior line or curve <a href='#SkPath_Verb'>Verb</a> is preserved but is ignored.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kQuad_Verb'><code>SkPath::kQuad_Verb</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Adds <a href='#Quad'>Quad</a> from <a href='#Last_Point'>Last Point</a>, using control <a href='SkPoint_Reference#Point'>Point</a>, and end <a href='SkPoint_Reference#Point'>Point</a>.
<a href='#Quad'>Quad</a> is a parabolic section within tangents from <a href='#Last_Point'>Last Point</a> to control <a href='SkPoint_Reference#Point'>Point</a>,
and control <a href='SkPoint_Reference#Point'>Point</a> to end <a href='SkPoint_Reference#Point'>Point</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kConic_Verb'><code>SkPath::kConic_Verb</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>3</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Adds <a href='#Conic'>Conic</a> from <a href='#Last_Point'>Last Point</a>, using control <a href='SkPoint_Reference#Point'>Point</a>, end <a href='SkPoint_Reference#Point'>Point</a>, and <a href='#Conic_Weight'>Conic Weight</a>.
<a href='#Conic'>Conic</a> is a elliptical, parabolic, or hyperbolic section within tangents
from <a href='#Last_Point'>Last Point</a> to control <a href='SkPoint_Reference#Point'>Point</a>, and control <a href='SkPoint_Reference#Point'>Point</a> to end <a href='SkPoint_Reference#Point'>Point</a>, constrained
by <a href='#Conic_Weight'>Conic Weight</a>. <a href='#Conic_Weight'>Conic Weight</a> less than one is elliptical; equal to one is
parabolic (and identical to <a href='#Quad'>Quad</a>); greater than one hyperbolic.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kCubic_Verb'><code>SkPath::kCubic_Verb</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Adds <a href='#Cubic'>Cubic</a> from <a href='#Last_Point'>Last Point</a>, using two control <a href='SkPoint_Reference#Point'>Points</a>, and end <a href='SkPoint_Reference#Point'>Point</a>.
<a href='#Cubic'>Cubic</a> is a third-order <a href='undocumented#Bezier_Curve'>Bezier Curve</a> section within tangents from <a href='#Last_Point'>Last Point</a>
to first control <a href='SkPoint_Reference#Point'>Point</a>, and from second control <a href='SkPoint_Reference#Point'>Point</a> to end <a href='SkPoint_Reference#Point'>Point</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kClose_Verb'><code>SkPath::kClose_Verb</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>5</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Closes <a href='#Contour'>Contour</a>, connecting <a href='#Last_Point'>Last Point</a> to <a href='#SkPath_kMove_Verb'>kMove Verb</a> <a href='SkPoint_Reference#Point'>Point</a>. Consecutive
<a href='#SkPath_kClose_Verb'>kClose Verb</a> are preserved but only first has an effect. <a href='#SkPath_kClose_Verb'>kClose Verb</a> after
<a href='#SkPath_kMove_Verb'>kMove Verb</a> has no effect.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kDone_Verb'><code>SkPath::kDone_Verb</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>6</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Not in <a href='#Verb_Array'>Verb Array</a>, but returned by <a href='#Path'>Path</a> iterator.
</td>
  </tr>
Each <a href='#SkPath_Verb'>Verb</a> has zero or more <a href='SkPoint_Reference#Point'>Points</a> stored in <a href='#Path'>Path</a>.
<a href='#Path'>Path</a> iterator returns complete curve descriptions, duplicating shared <a href='SkPoint_Reference#Point'>Points</a>
for consecutive entries.

</table>

| <a href='#SkPath_Verb'>Verb</a> | Allocated <a href='SkPoint_Reference#Point'>Points</a> | Iterated <a href='SkPoint_Reference#Point'>Points</a> | <a href='#Conic_Weight'>Weights</a> |
| --- | --- | --- | ---  |
| <a href='#SkPath_kMove_Verb'>kMove Verb</a> | 1 | 1 | 0 |
| <a href='#SkPath_kLine_Verb'>kLine Verb</a> | 1 | 2 | 0 |
| <a href='#SkPath_kQuad_Verb'>kQuad Verb</a> | 2 | 3 | 0 |
| <a href='#SkPath_kConic_Verb'>kConic Verb</a> | 2 | 3 | 1 |
| <a href='#SkPath_kCubic_Verb'>kCubic Verb</a> | 3 | 4 | 0 |
| <a href='#SkPath_kClose_Verb'>kClose Verb</a> | 0 | 1 | 0 |
| <a href='#SkPath_kDone_Verb'>kDone Verb</a> | -- | 0 | 0 |

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
        <a href='#SkPath_kCW_Direction'>kCW Direction</a>,
        <a href='#SkPath_kCCW_Direction'>kCCW Direction</a>,
    };
</pre>

<a href='#SkPath_Direction'>Direction</a> describes whether <a href='#Contour'>Contour</a> is clockwise or counterclockwise.
When <a href='#Path'>Path</a> contains multiple overlapping <a href='#Contour'>Contours</a>, <a href='#SkPath_Direction'>Direction</a> together with
<a href='#Fill_Type'>Fill Type</a> determines whether overlaps are filled or form holes.

<a href='#SkPath_Direction'>Direction</a> also determines how <a href='#Contour'>Contour</a> is measured. For instance, dashing
measures along <a href='#Path'>Path</a> to determine where to start and stop stroke; <a href='#SkPath_Direction'>Direction</a>
will change dashed results as it steps clockwise or counterclockwise.

Closed <a href='#Contour'>Contours</a> like <a href='SkRect_Reference#Rect'>Rect</a>, <a href='SkRRect_Reference#RRect'>Round Rect</a>, <a href='undocumented#Circle'>Circle</a>, and <a href='undocumented#Oval'>Oval</a> added with
<a href='#SkPath_kCW_Direction'>kCW Direction</a> travel clockwise; the same added with <a href='#SkPath_kCCW_Direction'>kCCW Direction</a>
travel counterclockwise.

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
<a href='#SkPath'>SkPath</a>()
</pre>

Constucts an empty path. By default, <a href='#Path'>Path</a> has no <a href='#Verb'>Verbs</a>, no <a href='SkPoint_Reference#Point'>Points</a>, and no <a href='#Conic_Weight'>Weights</a>.
<a href='#Fill_Type'>Fill Type</a> is set to <a href='#SkPath_kWinding_FillType'>kWinding FillType</a>.

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
<a href='#SkPath'>SkPath</a>(const <a href='#SkPath'>SkPath</a>& path)
</pre>

Constructs a copy of an existing <a href='#SkPath_copy_const_SkPath_path'>path</a>.
Copy constructor makes two paths identical by value. Internally, <a href='#SkPath_copy_const_SkPath_path'>path</a> and
the returned result share pointer values. The underlying <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>
and <a href='#Conic_Weight'>Weights</a> are copied when modified.

Creating a <a href='#Path'>Path</a> copy is very efficient and never allocates memory.
<a href='#Path'>Paths</a> are always copied by value from the interface; the underlying shared
pointers are not exposed.

### Parameters

<table>  <tr>    <td><a name='SkPath_copy_const_SkPath_path'><code><strong>path</strong></code></a></td>
    <td><a href='#Path'>Path</a> to copy by value</td>
  </tr>
</table>

### Return Value

copy of <a href='#Path'>Path</a>

### Example

<div><fiddle-embed name="647312aacd946c8a6eabaca797140432"><div>Modifying one <a href='#SkPath_copy_const_SkPath_path'>path</a> does not effect another, even if they started as copies
of each other.
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

---

<a name='SkPath_destructor'></a>
## ~SkPath

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_destructor'>~SkPath</a>()
</pre>

Releases ownership of any shared data and deletes data if <a href='#Path'>Path</a> is sole owner.

### Example

<div><fiddle-embed name="01ad6be9b7d15a2217daea273eb3d466"><div>delete calls <a href='#Path'>Path</a> <a href='undocumented#Destructor'>Destructor</a>, but copy of original in path2 is unaffected.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_empty_constructor'>SkPath()</a> <a href='#SkPath_copy_const_SkPath'>SkPath(const SkPath& path)</a> <a href='#SkPath_copy_operator'>operator=(const SkPath& path)</a>

---

<a name='SkPath_copy_operator'></a>
## operator=

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath'>SkPath</a>& <a href='#SkPath_copy_operator'>operator=(const SkPath& path)</a>
</pre>

Constructs a copy of an existing <a href='#SkPath_copy_operator_path'>path</a>.
<a href='#Path'>Path</a> assignment makes two paths identical by value. Internally, assignment
shares pointer values. The underlying <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a> and <a href='#Conic_Weight'>Weights</a>
are copied when modified.

Copying <a href='#Path'>Paths</a> by assignment is very efficient and never allocates memory.
<a href='#Path'>Paths</a> are always copied by value from the interface; the underlying shared
pointers are not exposed.

### Parameters

<table>  <tr>    <td><a name='SkPath_copy_operator_path'><code><strong>path</strong></code></a></td>
    <td><a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, <a href='#Conic_Weight'>Weights</a>, and <a href='#Fill_Type'>Fill Type</a> to copy</td>
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

Compares <a href='#SkPath_equal_operator_a'>a</a> and <a href='#SkPath_equal_operator_b'>b</a>; returns true if <a href='#Fill_Type'>Fill Type</a>, <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, and <a href='#Conic_Weight'>Weights</a>
are equivalent.

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

<div><fiddle-embed name="31883f51bb357f2ac5990d88f8b82e02"><div>Rewind removes <a href='#Verb_Array'>Verb Array</a> but leaves storage; since storage is not compared,
<a href='#Path'>Path</a> pair are equivalent.
</div>

#### Example Output

~~~~
empty one == two
moveTo one != two
rewind one == two
reset one == two
~~~~

</fiddle-embed></div>

---

<a name='SkPath_notequal_operator'></a>
## operator!=

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_notequal_operator'>operator!=(const SkPath& a, const SkPath& b)</a>
</pre>

Compares <a href='#SkPath_notequal_operator_a'>a</a> and <a href='#SkPath_notequal_operator_b'>b</a>; returns true if <a href='#Fill_Type'>Fill Type</a>, <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, and <a href='#Conic_Weight'>Weights</a>
are not equivalent.

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

<div><fiddle-embed name="0c6870ba1cea85ce6da5abd489c23d83"><div><a href='#Path'>Path</a> pair are equal though their convexity is not equal.
</div>

#### Example Output

~~~~
empty one == two
addRect one == two
setConvexity one == two
convexity !=
~~~~

</fiddle-embed></div>

---

## <a name='Property'>Property</a>


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_IsCubicDegenerate'>IsCubicDegenerate</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if <a href='#Cubic'>Cubic</a> is very small</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_IsInverseFillType'>IsInverseFillType</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if <a href='#Fill_Type'>Fill Type</a> represents outside geometry</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_IsLineDegenerate'>IsLineDegenerate</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if <a href='undocumented#Line'>Line</a> is very small</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_IsQuadDegenerate'>IsQuadDegenerate</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if <a href='#Quad'>Quad</a> is very small</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_computeTightBounds'>computeTightBounds</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns extent of geometry</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_conservativelyContainsRect'>conservativelyContainsRect</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns true if <a href='SkRect_Reference#Rect'>Rect</a> may be inside</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_contains'>contains</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if <a href='SkPoint_Reference#Point'>Point</a> is in fill area</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_getBounds'>getBounds</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns maximum and minimum of <a href='#Point_Array'>Point Array</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_getLastPt'>getLastPt</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#Last_Point'>Last Point</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_isEmpty'>isEmpty</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if verb count is zero</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_isFinite'>isFinite</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if all <a href='SkPoint_Reference#Point'>Point</a> values are finite</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_isInterpolatable'>isInterpolatable</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if pair contains equal counts of <a href='#Verb_Array'>Verb Array</a> and <a href='#Conic_Weight'>Weights</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_isLastContourClosed'>isLastContourClosed</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if final <a href='#Contour'>Contour</a> forms a loop</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_isLine'>isLine</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if describes <a href='undocumented#Line'>Line</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_isNestedFillRects'>isNestedFillRects</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if describes <a href='SkRect_Reference#Rect'>Rect</a> pair, one inside the other</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_isOval'>isOval</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if describes <a href='undocumented#Oval'>Oval</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_isRRect'>isRRect</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if describes <a href='SkRRect_Reference#RRect'>Round Rect</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_isRect'>isRect</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if describes <a href='SkRect_Reference#Rect'>Rect</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_isValid'>isValid</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if data is internally consistent</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_isVolatile'>isVolatile</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns if <a href='undocumented#Device'>Device</a> should not cache</td>
  </tr>
</table>

<a name='SkPath_isInterpolatable'></a>
## isInterpolatable

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isInterpolatable'>isInterpolatable</a>(const <a href='#SkPath'>SkPath</a>& compare) const
</pre>

Returns true if <a href='#Path'>Paths</a> contain equal <a href='#Verb'>Verbs</a> and equal <a href='#Conic_Weight'>Weights</a>.
If <a href='#Path'>Paths</a> contain one or more <a href='#Conic'>Conics</a>, the <a href='#Conic_Weight'>Weights</a> must match.

<a href='#SkPath_conicTo'>conicTo</a> may add different <a href='#Verb'>Verbs</a> depending on <a href='#Conic_Weight'>Conic Weight</a>, so it is not
trivial to interpolate a pair of <a href='#Path'>Paths</a> containing <a href='#Conic'>Conics</a> with different
<a href='#Conic_Weight'>Conic Weight</a> values.

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


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_interpolate'>interpolate</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>interpolates between <a href='#Path'>Path</a> pair</td>
  </tr>
</table>

<a name='SkPath_interpolate'></a>
## interpolate

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_interpolate'>interpolate</a>(const <a href='#SkPath'>SkPath</a>& ending, <a href='undocumented#SkScalar'>SkScalar</a> weight, <a href='#SkPath'>SkPath</a>* out) const
</pre>

Interpolates between <a href='#Path'>Paths</a> with <a href='#Point_Array'>Point Array</a> of equal size.
Copy <a href='#Verb_Array'>Verb Array</a> and <a href='#Conic_Weight'>Weights</a> to <a href='#SkPath_interpolate_out'>out</a>, and set <a href='#SkPath_interpolate_out'>out</a> <a href='#Point_Array'>Point Array</a> to a weighted
average of this <a href='#Point_Array'>Point Array</a> and <a href='#SkPath_interpolate_ending'>ending</a> <a href='#Point_Array'>Point Array</a>, using the formula:
(<a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> * <a href='#SkPath_interpolate_weight'>weight</a>) + <a href='#SkPath_interpolate_ending'>ending</a> <a href='SkPoint_Reference#Point'>Point</a> * (1 - <a href='#SkPath_interpolate_weight'>weight</a>)
.

<a href='#SkPath_interpolate_weight'>weight</a> is most useful when between zero (<a href='#SkPath_interpolate_ending'>ending</a> <a href='#Point_Array'>Point Array</a>) and
one (this <a href='#Point_Array'>Point Array</a>); will work with values outside of this
range.

<a href='#SkPath_interpolate'>interpolate</a> returns false and leaves <a href='#SkPath_interpolate_out'>out</a> unchanged if <a href='#Point_Array'>Point Array</a> is not
the same size as <a href='#SkPath_interpolate_ending'>ending</a> <a href='#Point_Array'>Point Array</a>. Call <a href='#SkPath_isInterpolatable'>isInterpolatable</a> to check <a href='#Path'>Path</a>
compatibility prior to calling <a href='#SkPath_interpolate'>interpolate</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_interpolate_ending'><code><strong>ending</strong></code></a></td>
    <td><a href='#Point_Array'>Point Array</a> averaged with this <a href='#Point_Array'>Point Array</a></td>
  </tr>
  <tr>    <td><a name='SkPath_interpolate_weight'><code><strong>weight</strong></code></a></td>
    <td>contribution of this <a href='#Point_Array'>Point Array</a>, and
one minus contribution of <a href='#SkPath_interpolate_ending'>ending</a> <a href='#Point_Array'>Point Array</a></td>
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
bool <a href='#SkPath_unique'>unique</a>() const
</pre>

Deprecated.

soonOnly valid for Android framework.

---

## <a name='Fill_Type'>Fill Type</a>

## <a name='SkPath_FillType'>Enum SkPath::FillType</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPath_FillType'>FillType</a> {
        <a href='#SkPath_kWinding_FillType'>kWinding FillType</a>,
        <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd FillType</a>,
        <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding FillType</a>,
        <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd FillType</a>,
    };
</pre>

<a href='#Fill_Type'>Fill Type</a> selects the rule used to fill <a href='#Path'>Path</a>. <a href='#Path'>Path</a> set to <a href='#SkPath_kWinding_FillType'>kWinding FillType</a>
fills if the sum of <a href='#Contour'>Contour</a> edges is not zero, where clockwise edges add one, and
counterclockwise edges subtract one. <a href='#Path'>Path</a> set to <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd FillType</a> fills if the
number of <a href='#Contour'>Contour</a> edges is odd. Each <a href='#Fill_Type'>Fill Type</a> has an inverse variant that
reverses the rule:
<a href='#SkPath_kInverseWinding_FillType'>kInverseWinding FillType</a> fills where the sum of <a href='#Contour'>Contour</a> edges is zero;
<a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd FillType</a> fills where the number of <a href='#Contour'>Contour</a> edges is even.

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

<div><fiddle-embed name="d84cd32b0bfd9ad2714f753120ed0ee1"></fiddle-embed></div>

### See Also

<a href='SkPaint_Reference#SkPaint_Style'>SkPaint::Style</a> <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_getFillType'>getFillType</a> <a href='#SkPath_setFillType'>setFillType</a>

<a name='SkPath_getFillType'></a>
## getFillType

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_getFillType'>getFillType</a>() const
</pre>

Returns <a href='#SkPath_FillType'>FillType</a>, the rule used to fill <a href='#Path'>Path</a>. <a href='#SkPath_FillType'>FillType</a> of a new <a href='#Path'>Path</a> is
<a href='#SkPath_kWinding_FillType'>kWinding FillType</a>.

### Return Value

one of: <a href='#SkPath_kWinding_FillType'>kWinding FillType</a>, <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd FillType</a>,  <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding FillType</a>,
<a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd FillType</a>

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
void <a href='#SkPath_setFillType'>setFillType</a>(<a href='#SkPath_FillType'>FillType</a> ft)
</pre>

Sets <a href='#SkPath_FillType'>FillType</a>, the rule used to fill <a href='#Path'>Path</a>. While there is no check
that <a href='#SkPath_setFillType_ft'>ft</a> is legal, values outside of <a href='#SkPath_FillType'>FillType</a> are not supported.

### Parameters

<table>  <tr>    <td><a name='SkPath_setFillType_ft'><code><strong>ft</strong></code></a></td>
    <td>one of: <a href='#SkPath_kWinding_FillType'>kWinding FillType</a>, <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd FillType</a>,  <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding FillType</a>,
<a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd FillType</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="b4a91cd7f50b2a0a0d1bec6d0ac823d2"><div>If empty <a href='#Path'>Path</a> is set to inverse <a href='#SkPath_FillType'>FillType</a>, it fills all pixels.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_getFillType'>getFillType</a> <a href='#SkPath_toggleInverseFillType'>toggleInverseFillType</a>

---

<a name='SkPath_isInverseFillType'></a>
## isInverseFillType

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isInverseFillType'>isInverseFillType</a>() const
</pre>

Returns if <a href='#SkPath_FillType'>FillType</a> describes area outside <a href='#Path'>Path</a> geometry. The inverse fill area
extends indefinitely.

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
void <a href='#SkPath_toggleInverseFillType'>toggleInverseFillType</a>()
</pre>

Replaces <a href='#SkPath_FillType'>FillType</a> with its inverse. The inverse of <a href='#SkPath_FillType'>FillType</a> describes the area
unmodified by the original <a href='#SkPath_FillType'>FillType</a>.

| <a href='#SkPath_FillType'>FillType</a> | toggled <a href='#SkPath_FillType'>FillType</a> |
| --- | ---  |
| <a href='#SkPath_kWinding_FillType'>kWinding FillType</a> | <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding FillType</a> |
| <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd FillType</a> | <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd FillType</a> |
| <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding FillType</a> | <a href='#SkPath_kWinding_FillType'>kWinding FillType</a> |
| <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd FillType</a> | <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd FillType</a> |

### Example

<div><fiddle-embed name="400facce23d417bc5043c5f58404afbd"><div><a href='#Path'>Path</a> drawn normally and through its inverse touches every pixel once.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_getFillType'>getFillType</a> <a href='#SkPath_setFillType'>setFillType</a> <a href='#SkPath_isInverseFillType'>isInverseFillType</a>

---

## <a name='Convexity'>Convexity</a>

## <a name='SkPath_Convexity'>Enum SkPath::Convexity</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPath_Convexity'>Convexity</a> : uint8_t {
        <a href='#SkPath_kUnknown_Convexity'>kUnknown Convexity</a>,
        <a href='#SkPath_kConvex_Convexity'>kConvex Convexity</a>,
        <a href='#SkPath_kConcave_Convexity'>kConcave Convexity</a>,
    };
</pre>

<a href='#Path'>Path</a> is convex if it contains one <a href='#Contour'>Contour</a> and <a href='#Contour'>Contour</a> loops no more than
360 degrees, and <a href='#Contour'>Contour</a> angles all have same <a href='#SkPath_Direction'>Direction</a>. Convex <a href='#Path'>Path</a>
may have better performance and require fewer resources on <a href='undocumented#GPU_Surface'>GPU Surface</a>.

<a href='#Path'>Path</a> is concave when either at least one <a href='#SkPath_Direction'>Direction</a> change is clockwise and
another is counterclockwise, or the sum of the changes in <a href='#SkPath_Direction'>Direction</a> is not 360
degrees.

Initially <a href='#Path'>Path</a> <a href='#SkPath_Convexity'>Convexity</a> is <a href='#SkPath_kUnknown_Convexity'>kUnknown Convexity</a>. <a href='#Path'>Path</a> <a href='#SkPath_Convexity'>Convexity</a> is computed
if needed by destination <a href='SkSurface_Reference#Surface'>Surface</a>.

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

<a href='#Contour'>Contour</a> <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_getConvexity'>getConvexity</a> <a href='#SkPath_getConvexityOrUnknown'>getConvexityOrUnknown</a> <a href='#SkPath_setConvexity'>setConvexity</a> <a href='#SkPath_isConvex'>isConvex</a>

<a name='SkPath_getConvexity'></a>
## getConvexity

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_getConvexity'>getConvexity</a>() const
</pre>

Computes <a href='#SkPath_Convexity'>Convexity</a> if required, and returns stored value.
<a href='#SkPath_Convexity'>Convexity</a> is computed if stored value is <a href='#SkPath_kUnknown_Convexity'>kUnknown Convexity</a>,
or if <a href='#Path'>Path</a> has been altered since <a href='#SkPath_Convexity'>Convexity</a> was computed or set.

### Return Value

computed or stored <a href='#SkPath_Convexity'>Convexity</a>

### Example

<div><fiddle-embed name="a8f36f2fa90003e3691fd0da0bb0c243"></fiddle-embed></div>

### See Also

<a href='#SkPath_Convexity'>Convexity</a> <a href='#Contour'>Contour</a> <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_getConvexityOrUnknown'>getConvexityOrUnknown</a> <a href='#SkPath_setConvexity'>setConvexity</a> <a href='#SkPath_isConvex'>isConvex</a>

---

<a name='SkPath_getConvexityOrUnknown'></a>
## getConvexityOrUnknown

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_Convexity'>Convexity</a> <a href='#SkPath_getConvexityOrUnknown'>getConvexityOrUnknown</a>() const
</pre>

Returns last computed <a href='#SkPath_Convexity'>Convexity</a>, or <a href='#SkPath_kUnknown_Convexity'>kUnknown Convexity</a> if
<a href='#Path'>Path</a> has been altered since <a href='#SkPath_Convexity'>Convexity</a> was computed or set.

### Return Value

stored <a href='#SkPath_Convexity'>Convexity</a>

### Example

<div><fiddle-embed name="111c59e9afadb940ab8f41bdc25378a4"><div><a href='#SkPath_Convexity'>Convexity</a> is unknown unless <a href='#SkPath_getConvexity'>getConvexity</a> is called without a subsequent call
that alters the path.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_Convexity'>Convexity</a> <a href='#Contour'>Contour</a> <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_getConvexity'>getConvexity</a> <a href='#SkPath_setConvexity'>setConvexity</a> <a href='#SkPath_isConvex'>isConvex</a>

---

<a name='SkPath_setConvexity'></a>
## setConvexity

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_setConvexity'>setConvexity</a>(<a href='#SkPath_Convexity'>Convexity</a> convexity)
</pre>

Stores <a href='#SkPath_setConvexity_convexity'>convexity</a> so that it is later returned by <a href='#SkPath_getConvexity'>getConvexity</a> or <a href='#SkPath_getConvexityOrUnknown'>getConvexityOrUnknown</a>.
<a href='#SkPath_setConvexity_convexity'>convexity</a> may differ from <a href='#SkPath_getConvexity'>getConvexity</a>, although setting an incorrect value may
cause incorrect or inefficient drawing.

If <a href='#SkPath_setConvexity_convexity'>convexity</a> is <a href='#SkPath_kUnknown_Convexity'>kUnknown Convexity</a>: <a href='#SkPath_getConvexity'>getConvexity</a> will
compute <a href='#SkPath_Convexity'>Convexity</a>, and <a href='#SkPath_getConvexityOrUnknown'>getConvexityOrUnknown</a> will return <a href='#SkPath_kUnknown_Convexity'>kUnknown Convexity</a>.

If <a href='#SkPath_setConvexity_convexity'>convexity</a> is <a href='#SkPath_kConvex_Convexity'>kConvex Convexity</a> or <a href='#SkPath_kConcave_Convexity'>kConcave Convexity</a>, <a href='#SkPath_getConvexity'>getConvexity</a>
and <a href='#SkPath_getConvexityOrUnknown'>getConvexityOrUnknown</a> will return <a href='#SkPath_setConvexity_convexity'>convexity</a> until the path is
altered.

### Parameters

<table>  <tr>    <td><a name='SkPath_setConvexity_convexity'><code><strong>convexity</strong></code></a></td>
    <td>one of: <a href='#SkPath_kUnknown_Convexity'>kUnknown Convexity</a>, <a href='#SkPath_kConvex_Convexity'>kConvex Convexity</a>, or <a href='#SkPath_kConcave_Convexity'>kConcave Convexity</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="875e32b4b1cb48d739325705fc0fa42c"></fiddle-embed></div>

### See Also

<a href='#SkPath_Convexity'>Convexity</a> <a href='#Contour'>Contour</a> <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_getConvexity'>getConvexity</a> <a href='#SkPath_getConvexityOrUnknown'>getConvexityOrUnknown</a> <a href='#SkPath_isConvex'>isConvex</a>

---

<a name='SkPath_isConvex'></a>
## isConvex

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isConvex'>isConvex</a>() const
</pre>

Computes <a href='#SkPath_Convexity'>Convexity</a> if required, and returns true if value is <a href='#SkPath_kConvex_Convexity'>kConvex Convexity</a>.
If <a href='#SkPath_setConvexity'>setConvexity</a> was called with <a href='#SkPath_kConvex_Convexity'>kConvex Convexity</a> or <a href='#SkPath_kConcave_Convexity'>kConcave Convexity</a>, and
the path has not been altered, <a href='#SkPath_Convexity'>Convexity</a> is not recomputed.

### Return Value

true if <a href='#SkPath_Convexity'>Convexity</a> stored or computed is <a href='#SkPath_kConvex_Convexity'>kConvex Convexity</a>

### Example

<div><fiddle-embed name="d8be8b6e59de244e4cbf58ec9554557b"><div>Concave shape is erroneously considered convex after a forced call to
<a href='#SkPath_setConvexity'>setConvexity</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_Convexity'>Convexity</a> <a href='#Contour'>Contour</a> <a href='#SkPath_Direction'>Direction</a> <a href='#SkPath_getConvexity'>getConvexity</a> <a href='#SkPath_getConvexityOrUnknown'>getConvexityOrUnknown</a> <a href='#SkPath_setConvexity'>setConvexity</a>

---

<a name='SkPath_isOval'></a>
## isOval

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isOval'>isOval</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* bounds) const
</pre>

Returns true if this path is recognized as an oval or circle.

<a href='#SkPath_isOval_bounds'>bounds</a> receives <a href='#SkPath_isOval_bounds'>bounds</a> of <a href='undocumented#Oval'>Oval</a>.

<a href='#SkPath_isOval_bounds'>bounds</a> is unmodified if <a href='undocumented#Oval'>Oval</a> is not found.

### Parameters

<table>  <tr>    <td><a name='SkPath_isOval_bounds'><code><strong>bounds</strong></code></a></td>
    <td>storage for bounding <a href='SkRect_Reference#Rect'>Rect</a> of <a href='undocumented#Oval'>Oval</a>; may be nullptr</td>
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
bool <a href='#SkPath_isRRect'>isRRect</a>(<a href='SkRRect_Reference#SkRRect'>SkRRect</a>* rrect) const
</pre>

Returns true if this path is recognized as a <a href='SkRRect_Reference#SkRRect'>SkRRect</a> (but not an oval/circle or rect).

<a href='#SkPath_isRRect_rrect'>rrect</a> receives bounds of <a href='SkRRect_Reference#RRect'>Round Rect</a>.

<a href='#SkPath_isRRect_rrect'>rrect</a> is unmodified if <a href='SkRRect_Reference#RRect'>Round Rect</a> is not found.

### Parameters

<table>  <tr>    <td><a name='SkPath_isRRect_rrect'><code><strong>rrect</strong></code></a></td>
    <td>storage for bounding <a href='SkRect_Reference#Rect'>Rect</a> of <a href='SkRRect_Reference#RRect'>Round Rect</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

true if <a href='#Path'>Path</a> contains only <a href='SkRRect_Reference#RRect'>Round Rect</a>

### Example

<div><fiddle-embed name="2aa939b90d96aff436b145a96305132c"><div>Draw rounded rectangle and its bounds.
</div></fiddle-embed></div>

### See Also

<a href='SkRRect_Reference#RRect'>Round Rect</a> <a href='#SkPath_addRoundRect'>addRoundRect</a><sup><a href='#SkPath_addRoundRect_2'>[2]</a></sup> <a href='#SkPath_addRRect'>addRRect</a><sup><a href='#SkPath_addRRect_2'>[2]</a></sup>

---

<a name='SkPath_reset'></a>
## reset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_reset'>reset</a>()
</pre>

Sets <a href='#Path'>Path</a> to its initial state.
Removes <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, and <a href='#Conic_Weight'>Weights</a>, and sets <a href='#SkPath_FillType'>FillType</a> to <a href='#SkPath_kWinding_FillType'>kWinding FillType</a>.
Internal storage associated with <a href='#Path'>Path</a> is released.

### Example

<div><fiddle-embed name="8cdca35d2964bbbecb93d79a13f71c65"></fiddle-embed></div>

### See Also

<a href='#SkPath_rewind'>rewind</a>

---

<a name='SkPath_rewind'></a>
## rewind

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_rewind'>rewind</a>()
</pre>

Sets <a href='#Path'>Path</a> to its initial state, preserving internal storage.
Removes <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, and <a href='#Conic_Weight'>Weights</a>, and sets <a href='#SkPath_FillType'>FillType</a> to <a href='#SkPath_kWinding_FillType'>kWinding FillType</a>.
Internal storage associated with <a href='#Path'>Path</a> is retained.

Use <a href='#SkPath_rewind'>rewind</a> instead of <a href='#SkPath_reset'>reset</a> if <a href='#Path'>Path</a> storage will be reused and performance
is critical.

### Example

<div><fiddle-embed name="f1fedbb89da9c2a33a91805175663012"><div>Although path1 retains its internal storage, it is indistinguishable from
a newly initialized path.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_reset'>reset</a>

---

<a name='SkPath_isEmpty'></a>
## isEmpty

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isEmpty'>isEmpty</a>() const
</pre>

Returns if <a href='#Path'>Path</a> is empty.
Empty <a href='#Path'>Path</a> may have <a href='#SkPath_FillType'>FillType</a> but has no <a href='SkPoint_Reference#SkPoint'>SkPoint</a>, <a href='#SkPath_Verb'>Verb</a>, or <a href='#Conic_Weight'>Conic Weight</a>.
<a href='#SkPath_empty_constructor'>SkPath()</a> constructs empty <a href='#Path'>Path</a>; <a href='#SkPath_reset'>reset</a> and (rewind) make <a href='#Path'>Path</a> empty.

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

<a href='#SkPath_empty_constructor'>SkPath()</a> <a href='#SkPath_reset'>reset</a> <a href='#SkPath_rewind'>rewind</a>

---

<a name='SkPath_isLastContourClosed'></a>
## isLastContourClosed

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isLastContourClosed'>isLastContourClosed</a>() const
</pre>

Returns if <a href='#Contour'>Contour</a> is closed.
<a href='#Contour'>Contour</a> is closed if <a href='#Path'>Path</a> <a href='#SkPath_Verb'>Verb</a> array was last modified by <a href='#SkPath_close'>close</a>. When stroked,
closed <a href='#Contour'>Contour</a> draws <a href='SkPaint_Reference#Stroke_Join'>Paint Stroke Join</a> instead of <a href='SkPaint_Reference#Stroke_Cap'>Paint Stroke Cap</a> at first and last <a href='SkPoint_Reference#Point'>Point</a>.

### Return Value

true if the last <a href='#Contour'>Contour</a> ends with a <a href='#SkPath_kClose_Verb'>kClose Verb</a>

### Example

<div><fiddle-embed name="03b740ab94b9017800a52e30b5e7fee7"><div><a href='#SkPath_close'>close</a> has no effect if <a href='#Path'>Path</a> is empty; <a href='#SkPath_isLastContourClosed'>isLastContourClosed</a> returns
false until <a href='#Path'>Path</a> has geometry followed by <a href='#SkPath_close'>close</a>.
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

<a href='#SkPath_close'>close</a>

---

<a name='SkPath_isFinite'></a>
## isFinite

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isFinite'>isFinite</a>() const
</pre>

Returns true for finite <a href='SkPoint_Reference#Point'>Point</a> array values between negative <a href='undocumented#SK_ScalarMax'>SK ScalarMax</a> and
positive <a href='undocumented#SK_ScalarMax'>SK ScalarMax</a>. Returns false for any <a href='SkPoint_Reference#Point'>Point</a> array value of
<a href='undocumented#SK_ScalarInfinity'>SK ScalarInfinity</a>, <a href='undocumented#SK_ScalarNegativeInfinity'>SK ScalarNegativeInfinity</a>, or <a href='undocumented#SK_ScalarNaN'>SK ScalarNaN</a>.

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
bool <a href='#SkPath_isVolatile'>isVolatile</a>() const
</pre>

Returns true if the path is volatile; it will not be altered or discarded
by the caller after it is drawn. <a href='#Path'>Paths</a> by default have volatile set false, allowing
<a href='SkSurface_Reference#Surface'>Surface</a> to attach a cache of data which speeds repeated drawing. If true, <a href='SkSurface_Reference#Surface'>Surface</a>
may not speed repeated drawing.

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


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_setIsVolatile'>setIsVolatile</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sets if <a href='undocumented#Device'>Device</a> should not cache</td>
  </tr>
</table>

<a name='SkPath_setIsVolatile'></a>
## setIsVolatile

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_setIsVolatile'>setIsVolatile</a>(bool <a href='#SkPath_isVolatile'>isVolatile</a>)
</pre>

Specifies whether <a href='#Path'>Path</a> is volatile; whether it will be altered or discarded
by the caller after it is drawn. <a href='#Path'>Paths</a> by default have volatile set false, allowing
<a href='undocumented#Device'>Device</a> to attach a cache of data which speeds repeated drawing.

Mark temporary paths, discarded or modified after use, as volatile
to inform <a href='undocumented#Device'>Device</a> that the path need not be cached.

Mark animating <a href='#Path'>Path</a> volatile to improve performance.
Mark unchanging <a href='#Path'>Path</a> non-volatile to improve repeated rendering.

<a href='undocumented#Raster_Surface'>Raster Surface</a> <a href='#Path'>Path</a> draws are affected by volatile for some shadows.
<a href='undocumented#GPU_Surface'>GPU Surface</a> <a href='#Path'>Path</a> draws are affected by volatile for some shadows and concave geometries.

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
static bool <a href='#SkPath_IsLineDegenerate'>IsLineDegenerate</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p1, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p2, bool exact)
</pre>

Tests if <a href='undocumented#Line'>Line</a> between <a href='SkPoint_Reference#Point'>Point</a> pair is degenerate.
<a href='undocumented#Line'>Line</a> with no length or that moves a very short distance is degenerate; it is
treated as a point.

<a href='#SkPath_IsLineDegenerate_exact'>exact</a> changes the equality test. If true, returns true only if <a href='#SkPath_IsLineDegenerate_p1'>p1</a> equals <a href='#SkPath_IsLineDegenerate_p2'>p2</a>.
If false, returns true if <a href='#SkPath_IsLineDegenerate_p1'>p1</a> equals or nearly equals <a href='#SkPath_IsLineDegenerate_p2'>p2</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_IsLineDegenerate_p1'><code><strong>p1</strong></code></a></td>
    <td>line start point</td>
  </tr>
  <tr>    <td><a name='SkPath_IsLineDegenerate_p2'><code><strong>p2</strong></code></a></td>
    <td>line end point</td>
  </tr>
  <tr>    <td><a name='SkPath_IsLineDegenerate_exact'><code><strong>exact</strong></code></a></td>
    <td>if false, allow nearly equals</td>
  </tr>
</table>

### Return Value

true if <a href='undocumented#Line'>Line</a> is degenerate; its length is effectively zero

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

---

<a name='SkPath_IsQuadDegenerate'></a>
## IsQuadDegenerate

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkPath_IsQuadDegenerate'>IsQuadDegenerate</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p1, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p2, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p3, bool exact)
</pre>

Tests if <a href='#Quad'>Quad</a> is degenerate.
<a href='#Quad'>Quad</a> with no length or that moves a very short distance is degenerate; it is
treated as a point.

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
    <td>if true, returns true only if <a href='#SkPath_IsQuadDegenerate_p1'>p1</a>, <a href='#SkPath_IsQuadDegenerate_p2'>p2</a>, and <a href='#SkPath_IsQuadDegenerate_p3'>p3</a> are equal;
if false, returns true if <a href='#SkPath_IsQuadDegenerate_p1'>p1</a>, <a href='#SkPath_IsQuadDegenerate_p2'>p2</a>, and <a href='#SkPath_IsQuadDegenerate_p3'>p3</a> are equal or nearly equal</td>
  </tr>
</table>

### Return Value

true if <a href='#Quad'>Quad</a> is degenerate; its length is effectively zero

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

---

<a name='SkPath_IsCubicDegenerate'></a>
## IsCubicDegenerate

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkPath_IsCubicDegenerate'>IsCubicDegenerate</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p1, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p2, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p3,
                              const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p4, bool exact)
</pre>

Tests if <a href='#Cubic'>Cubic</a> is degenerate.
<a href='#Cubic'>Cubic</a> with no length or that moves a very short distance is degenerate; it is
treated as a point.

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
    <td>if true, returns true only if <a href='#SkPath_IsCubicDegenerate_p1'>p1</a>, <a href='#SkPath_IsCubicDegenerate_p2'>p2</a>, <a href='#SkPath_IsCubicDegenerate_p3'>p3</a>, and <a href='#SkPath_IsCubicDegenerate_p4'>p4</a> are equal;
if false, returns true if <a href='#SkPath_IsCubicDegenerate_p1'>p1</a>, <a href='#SkPath_IsCubicDegenerate_p2'>p2</a>, <a href='#SkPath_IsCubicDegenerate_p3'>p3</a>, and <a href='#SkPath_IsCubicDegenerate_p4'>p4</a> are equal or nearly equal</td>
  </tr>
</table>

### Return Value

true if <a href='#Cubic'>Cubic</a> is degenerate; its length is effectively zero

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
bool <a href='#SkPath_isLine'>isLine</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> line[2]) const
</pre>

Returns true if <a href='#Path'>Path</a> contains only one <a href='undocumented#Line'>Line</a>;
<a href='#Verb'>Path Verb</a> array has two entries: <a href='#SkPath_kMove_Verb'>kMove Verb</a>, <a href='#SkPath_kLine_Verb'>kLine Verb</a>.
If <a href='#Path'>Path</a> contains one <a href='undocumented#Line'>Line</a> and <a href='#SkPath_isLine_line'>line</a> is not nullptr, <a href='#SkPath_isLine_line'>line</a> is set to
<a href='undocumented#Line'>Line</a> start point and <a href='undocumented#Line'>Line</a> end point.
Returns false if <a href='#Path'>Path</a> is not one <a href='undocumented#Line'>Line</a>; <a href='#SkPath_isLine_line'>line</a> is unaltered.

### Parameters

<table>  <tr>    <td><a name='SkPath_isLine_line'><code><strong>line</strong></code></a></td>
    <td>storage for <a href='undocumented#Line'>Line</a>. May be nullptr</td>
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
each <a href='#SkPath_Verb'>Verb</a> in <a href='#Verb_Array'>Verb Array</a>. For instance, <a href='#Path'>Path</a> containing one <a href='#Contour'>Contour</a> with <a href='undocumented#Line'>Line</a>
and <a href='#Quad'>Quad</a> is described by <a href='#Verb_Array'>Verb Array</a>: Verb::kMoveTo, Verb::kLineTo, Verb::kQuadTo; and
one <a href='SkPoint_Reference#Point'>Point</a> for move, one <a href='SkPoint_Reference#Point'>Point</a> for <a href='undocumented#Line'>Line</a>, two <a href='SkPoint_Reference#Point'>Points</a> for <a href='#Quad'>Quad</a>; totaling four <a href='SkPoint_Reference#Point'>Points</a>.

<a href='#Point_Array'>Point Array</a> may be read directly from <a href='#Path'>Path</a> with <a href='#SkPath_getPoints'>getPoints</a>, or inspected with
<a href='#SkPath_getPoint'>getPoint</a>, with <a href='#SkPath_Iter'>Iter</a>, or with <a href='#SkPath_RawIter'>RawIter</a>.

<a name='SkPath_getPoints'></a>
## getPoints

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPath_getPoints'>getPoints</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> points[], int max) const
</pre>

Returns number of <a href='#SkPath_getPoints_points'>points</a> in <a href='#Path'>Path</a>. Up to max <a href='#SkPath_getPoints_points'>points</a> are copied.
<a href='#SkPath_getPoints_points'>points</a> may be nullptr; then, max must be zero.
If max is greater than number of <a href='#SkPath_getPoints_points'>points</a>, excess <a href='#SkPath_getPoints_points'>points</a> storage is unaltered.

### Parameters

<table>  <tr>    <td><a name='SkPath_getPoints_points'><code><strong>points</strong></code></a></td>
    <td>storage for <a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> array. May be nullptr</td>
  </tr>
  <tr>    <td><a name='SkPath_getPoints_max'><code><strong>max</strong></code></a></td>
    <td>maximum to copy; must be greater than or equal to zero</td>
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
int <a href='#SkPath_countPoints'>countPoints</a>() const
</pre>

Returns the number of points in <a href='#Path'>Path</a>.
<a href='SkPoint_Reference#Point'>Point</a> count is initially zero.

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
<a href='SkPoint_Reference#SkPoint'>SkPoint</a> <a href='#SkPath_getPoint'>getPoint</a>(int index) const
</pre>

Returns <a href='SkPoint_Reference#Point'>Point</a> at <a href='#SkPath_getPoint_index'>index</a> in <a href='#Point_Array'>Point Array</a>. Valid range for <a href='#SkPath_getPoint_index'>index</a> is
0 to <a href='#SkPath_countPoints'>countPoints</a> - 1.
Returns (0, 0) if <a href='#SkPath_getPoint_index'>index</a> is out of range.

### Parameters

<table>  <tr>    <td><a name='SkPath_getPoint_index'><code><strong>index</strong></code></a></td>
    <td><a href='SkPoint_Reference#Point'>Point</a> array element selector</td>
  </tr>
</table>

### Return Value

<a href='SkPoint_Reference#Point'>Point</a> array value or (0, 0)

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

<a href='#Verb_Array'>Verb Array</a> always starts with <a href='#SkPath_kMove_Verb'>kMove Verb</a>.
If <a href='#SkPath_kClose_Verb'>kClose Verb</a> is not the last entry, it is always followed by <a href='#SkPath_kMove_Verb'>kMove Verb</a>;
the quantity of <a href='#SkPath_kMove_Verb'>kMove Verb</a> equals the <a href='#Contour'>Contour</a> count.
<a href='#Verb_Array'>Verb Array</a> does not include or count <a href='#SkPath_kDone_Verb'>kDone Verb</a>; it is a convenience
returned when iterating through <a href='#Verb_Array'>Verb Array</a>.

<a href='#Verb_Array'>Verb Array</a> may be read directly from <a href='#Path'>Path</a> with <a href='#SkPath_getVerbs'>getVerbs</a>, or inspected with <a href='#SkPath_Iter'>Iter</a>,
or with <a href='#SkPath_RawIter'>RawIter</a>.

<a name='SkPath_countVerbs'></a>
## countVerbs

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
int <a href='#SkPath_countVerbs'>countVerbs</a>() const
</pre>

Returns the number of <a href='#Verb'>Verbs</a>: <a href='#SkPath_kMove_Verb'>kMove Verb</a>, <a href='#SkPath_kLine_Verb'>kLine Verb</a>, <a href='#SkPath_kQuad_Verb'>kQuad Verb</a>, <a href='#SkPath_kConic_Verb'>kConic Verb</a>,
<a href='#SkPath_kCubic_Verb'>kCubic Verb</a>, and <a href='#SkPath_kClose_Verb'>kClose Verb</a>; added to <a href='#Path'>Path</a>.

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
int <a href='#SkPath_getVerbs'>getVerbs</a>(uint8_t verbs[], int max) const
</pre>

Returns the number of <a href='#SkPath_getVerbs_verbs'>verbs</a> in the path. Up to max <a href='#SkPath_getVerbs_verbs'>verbs</a> are copied. The
<a href='#SkPath_getVerbs_verbs'>verbs</a> are copied as one byte per verb.

### Parameters

<table>  <tr>    <td><a name='SkPath_getVerbs_verbs'><code><strong>verbs</strong></code></a></td>
    <td>storage for <a href='#SkPath_getVerbs_verbs'>verbs</a>, may be nullptr</td>
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
void <a href='#SkPath_swap'>swap</a>(<a href='#SkPath'>SkPath</a>& other)
</pre>

Exchanges the <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, <a href='#Conic_Weight'>Weights</a>, and <a href='#Fill_Type'>Fill Type</a> with <a href='#SkPath_swap_other'>other</a>.
Cached state is also exchanged. <a href='#SkPath_swap'>swap</a> internally exchanges pointers, so
it is lightweight and does not allocate memory.

<a href='#SkPath_swap'>swap</a> usage has largely been replaced by <a href='#SkPath_copy_operator'>operator=(const SkPath& path)</a>.
<a href='#Path'>Paths</a> do not copy their content on assignment until they are written to,
making assignment as efficient as <a href='#SkPath_swap'>swap</a>.

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
const <a href='SkRect_Reference#SkRect'>SkRect</a>& <a href='#SkPath_getBounds'>getBounds</a>() const
</pre>

Returns minimum and maximum axes values of <a href='#Point_Array'>Point Array</a>.
Returns (0, 0, 0, 0) if <a href='#Path'>Path</a> contains no points. Returned bounds width and height may
be larger or smaller than area affected when <a href='#Path'>Path</a> is drawn.

<a href='SkRect_Reference#Rect'>Rect</a> returned includes all <a href='SkPoint_Reference#Point'>Points</a> added to <a href='#Path'>Path</a>, including <a href='SkPoint_Reference#Point'>Points</a> associated with
<a href='#SkPath_kMove_Verb'>kMove Verb</a> that define empty <a href='#Contour'>Contours</a>.

### Return Value

bounds of all <a href='SkPoint_Reference#Point'>Points</a> in <a href='#Point_Array'>Point Array</a>

### Example

<div><fiddle-embed name="45c0fc3acb74fab99d544b80eadd10ad"><div>Bounds of upright <a href='undocumented#Circle'>Circle</a> can be predicted from center and radius.
Bounds of rotated <a href='undocumented#Circle'>Circle</a> includes control <a href='SkPoint_Reference#Point'>Points</a> outside of filled area.
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

---

## <a name='Utility'>Utility</a>


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_ConvertConicToQuads'>ConvertConicToQuads</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>approximates <a href='#Conic'>Conic</a> with <a href='#Quad'>Quad</a> array</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_ConvertToNonInverseFillType'>ConvertToNonInverseFillType</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns <a href='#Fill_Type'>Fill Type</a> representing inside geometry</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_dump_2'>dump</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sends text representation to stream</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_dump'>dump(SkWStream* stream, bool forceClose, bool dumpAsHex)</a> const</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_dump_2'>dump</a> const</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_dumpHex'>dumpHex</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>sends text representation using hexadecimal to standard output</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_getSegmentMasks'>getSegmentMasks</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>returns types in <a href='#Verb_Array'>Verb Array</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_incReserve'>incReserve</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>reserves space for additional data</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_readFromMemory'>readFromMemory</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>initializes from buffer</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_serialize'>serialize</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>copies data to buffer</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_setLastPt'>setLastPt</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>replaces <a href='#Last_Point'>Last Point</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_setLastPt'>setLastPt(SkScalar x, SkScalar y)</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_updateBoundsCache'>updateBoundsCache</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>refreshes result of <a href='#SkPath_getBounds'>getBounds</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_writeToMemory'>writeToMemory</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>copies data to buffer</td>
  </tr>
</table>

<a name='SkPath_updateBoundsCache'></a>
## updateBoundsCache

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_updateBoundsCache'>updateBoundsCache</a>() const
</pre>

Updates internal bounds so that subsequent calls to <a href='#SkPath_getBounds'>getBounds</a> are instantaneous.
Unaltered copies of <a href='#Path'>Path</a> may also access cached bounds through <a href='#SkPath_getBounds'>getBounds</a>.

For now, identical to calling <a href='#SkPath_getBounds'>getBounds</a> and ignoring the returned value.

Call to prepare <a href='#Path'>Path</a> subsequently drawn from multiple threads,
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

---

<a name='SkPath_computeTightBounds'></a>
## computeTightBounds

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkRect_Reference#SkRect'>SkRect</a> <a href='#SkPath_computeTightBounds'>computeTightBounds</a>() const
</pre>

Returns minimum and maximum axes values of the lines and curves in <a href='#Path'>Path</a>.
Returns (0, 0, 0, 0) if <a href='#Path'>Path</a> contains no points.
Returned bounds width and height may be larger or smaller than area affected
when <a href='#Path'>Path</a> is drawn.

Includes <a href='SkPoint_Reference#Point'>Points</a> associated with <a href='#SkPath_kMove_Verb'>kMove Verb</a> that define empty
<a href='#Contour'>Contours</a>.

Behaves identically to <a href='#SkPath_getBounds'>getBounds</a> when <a href='#Path'>Path</a> contains
only lines. If <a href='#Path'>Path</a> contains curves, computed bounds includes
the maximum extent of the <a href='#Quad'>Quad</a>, <a href='#Conic'>Conic</a>, or <a href='#Cubic'>Cubic</a>; is slower than <a href='#SkPath_getBounds'>getBounds</a>;
and unlike <a href='#SkPath_getBounds'>getBounds</a>, does not cache the result.

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
bool <a href='#SkPath_conservativelyContainsRect'>conservativelyContainsRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& rect) const
</pre>

Returns true if <a href='#SkPath_conservativelyContainsRect_rect'>rect</a> is contained by <a href='#Path'>Path</a>.
May return false when <a href='#SkPath_conservativelyContainsRect_rect'>rect</a> is contained by <a href='#Path'>Path</a>.

For now, only returns true if <a href='#Path'>Path</a> has one <a href='#Contour'>Contour</a> and is convex.
<a href='#SkPath_conservativelyContainsRect_rect'>rect</a> may share points and edges with <a href='#Path'>Path</a> and be contained.
Returns true if <a href='#SkPath_conservativelyContainsRect_rect'>rect</a> is empty, that is, it has zero width or height; and
the <a href='SkPoint_Reference#Point'>Point</a> or <a href='undocumented#Line'>Line</a> described by <a href='#SkPath_conservativelyContainsRect_rect'>rect</a> is contained by <a href='#Path'>Path</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_conservativelyContainsRect_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a>, <a href='undocumented#Line'>Line</a>, or <a href='SkPoint_Reference#Point'>Point</a> checked for containment</td>
  </tr>
</table>

### Return Value

true if <a href='#SkPath_conservativelyContainsRect_rect'>rect</a> is contained

### Example

<div><fiddle-embed name="41638d13e40fa449ece354dde5fb1941"><div><a href='SkRect_Reference#Rect'>Rect</a> is drawn in blue if it is contained by red <a href='#Path'>Path</a>.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_contains'>contains</a> <a href='undocumented#Op'>Op</a> <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkPath_Convexity'>Convexity</a>

---

<a name='SkPath_incReserve'></a>
## incReserve

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_incReserve'>incReserve</a>(unsigned extraPtCount)
</pre>

Grows <a href='#Path'>Path</a> <a href='#Verb_Array'>Verb Array</a> and <a href='#Point_Array'>Point Array</a> to contain <a href='#SkPath_incReserve_extraPtCount'>extraPtCount</a> additional <a href='SkPoint_Reference#Point'>Points</a>.
May improve performance and use less memory by
reducing the number and size of allocations when creating <a href='#Path'>Path</a>.

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

## <a name='Build'>Build</a>


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addArc'>addArc</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds one <a href='#Contour'>Contour</a> containing <a href='#Arc'>Arc</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addCircle'>addCircle</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds one <a href='#Contour'>Contour</a> containing <a href='undocumented#Circle'>Circle</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addOval'>addOval</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds one <a href='#Contour'>Contour</a> containing <a href='undocumented#Oval'>Oval</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addOval'>addOval(const SkRect& oval, Direction dir = kCW Direction)</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addOval_2'>addOval(const SkRect& oval, Direction dir, unsigned start)</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addPath'>addPath</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds contents of <a href='#Path'>Path</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addPath'>addPath(const SkPath& src, SkScalar dx, SkScalar dy, AddPathMode mode = kAppend AddPathMode)</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addPath_2'>addPath(const SkPath& src, AddPathMode mode = kAppend AddPathMode)</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addPath_3'>addPath(const SkPath& src, const SkMatrix& matrix, AddPathMode mode = kAppend AddPathMode)</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addPoly'>addPoly</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds one <a href='#Contour'>Contour</a> containing connected lines</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addRRect'>addRRect</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds one <a href='#Contour'>Contour</a> containing <a href='SkRRect_Reference#RRect'>Round Rect</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addRRect'>addRRect(const SkRRect& rrect, Direction dir = kCW Direction)</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addRRect_2'>addRRect(const SkRRect& rrect, Direction dir, unsigned start)</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addRect'>addRect</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds one <a href='#Contour'>Contour</a> containing <a href='SkRect_Reference#Rect'>Rect</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addRect'>addRect(const SkRect& rect, Direction dir = kCW Direction)</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addRect_2'>addRect(const SkRect& rect, Direction dir, unsigned start)</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addRect_3'>addRect(SkScalar left, SkScalar top, SkScalar right, SkScalar bottom, Direction dir = kCW Direction)</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addRoundRect'>addRoundRect</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds one <a href='#Contour'>Contour</a> containing <a href='SkRRect_Reference#RRect'>Round Rect</a> with common corner radii</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addRoundRect'>addRoundRect(const SkRect& rect, SkScalar rx, SkScalar ry, Direction dir = kCW Direction)</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_addRoundRect_2'>addRoundRect(const SkRect& rect, const SkScalar radii[], Direction dir = kCW Direction)</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_arcTo'>arcTo</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>appends <a href='#Arc'>Arc</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_arcTo'>arcTo(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle, bool forceMoveTo)</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_close'>close</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>makes last <a href='#Contour'>Contour</a> a loop</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_cubicTo'>cubicTo</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>appends <a href='#Cubic'>Cubic</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_cubicTo'>cubicTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar x3, SkScalar y3)</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_lineTo'>lineTo</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>appends <a href='undocumented#Line'>Line</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_lineTo'>lineTo(SkScalar x, SkScalar y)</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_lineTo_2'>lineTo(const SkPoint& p)</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_moveTo'>moveTo</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>starts <a href='#Contour'>Contour</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_moveTo'>moveTo(SkScalar x, SkScalar y)</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_moveTo_2'>moveTo(const SkPoint& p)</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_rArcTo'>rArcTo</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>appends <a href='#Arc'>Arc</a> relative to <a href='#Last_Point'>Last Point</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_rConicTo'>rConicTo</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>appends <a href='#Conic'>Conic</a> relative to <a href='#Last_Point'>Last Point</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_rCubicTo'>rCubicTo</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>appends <a href='#Cubic'>Cubic</a> relative to <a href='#Last_Point'>Last Point</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_rLineTo'>rLineTo</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>appends <a href='undocumented#Line'>Line</a> relative to <a href='#Last_Point'>Last Point</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_rMoveTo'>rMoveTo</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>starts <a href='#Contour'>Contour</a> relative to <a href='#Last_Point'>Last Point</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_rQuadTo'>rQuadTo</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>appends <a href='#Quad'>Quad</a> relative to <a href='#Last_Point'>Last Point</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_reverseAddPath'>reverseAddPath</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>adds contents of <a href='#Path'>Path</a> back to front</td>
  </tr>
</table>

<a name='SkPath_moveTo'></a>
## moveTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_moveTo'>moveTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y)
</pre>

Adds beginning of <a href='#Contour'>Contour</a> at <a href='SkPoint_Reference#Point'>Point</a> (<a href='#SkPath_moveTo_x'>x</a>, <a href='#SkPath_moveTo_y'>y</a>).

### Parameters

<table>  <tr>    <td><a name='SkPath_moveTo_x'><code><strong>x</strong></code></a></td>
    <td><a href='#SkPath_moveTo_x'>x</a>-axis value of <a href='#Contour'>Contour</a> start</td>
  </tr>
  <tr>    <td><a name='SkPath_moveTo_y'><code><strong>y</strong></code></a></td>
    <td><a href='#SkPath_moveTo_y'>y</a>-axis value of <a href='#Contour'>Contour</a> start</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="84101d341e934a535a41ad6cf42218ce"></fiddle-embed></div>

### See Also

<a href='#Contour'>Contour</a> <a href='#SkPath_lineTo'>lineTo</a><sup><a href='#SkPath_lineTo_2'>[2]</a></sup> <a href='#SkPath_rMoveTo'>rMoveTo</a> <a href='#SkPath_quadTo'>quadTo</a><sup><a href='#SkPath_quadTo_2'>[2]</a></sup> <a href='#SkPath_conicTo'>conicTo</a><sup><a href='#SkPath_conicTo_2'>[2]</a></sup> <a href='#SkPath_cubicTo'>cubicTo</a><sup><a href='#SkPath_cubicTo_2'>[2]</a></sup> <a href='#SkPath_close'>close</a>

---

<a name='SkPath_moveTo_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_moveTo'>moveTo</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p)
</pre>

Adds beginning of <a href='#Contour'>Contour</a> at <a href='SkPoint_Reference#Point'>Point</a> <a href='#SkPath_moveTo_2_p'>p</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_moveTo_2_p'><code><strong>p</strong></code></a></td>
    <td>contour start</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="cb8d37990f6e7df3bcc85e7240c81274"></fiddle-embed></div>

### See Also

<a href='#Contour'>Contour</a> <a href='#SkPath_lineTo'>lineTo</a><sup><a href='#SkPath_lineTo_2'>[2]</a></sup> <a href='#SkPath_rMoveTo'>rMoveTo</a> <a href='#SkPath_quadTo'>quadTo</a><sup><a href='#SkPath_quadTo_2'>[2]</a></sup> <a href='#SkPath_conicTo'>conicTo</a><sup><a href='#SkPath_conicTo_2'>[2]</a></sup> <a href='#SkPath_cubicTo'>cubicTo</a><sup><a href='#SkPath_cubicTo_2'>[2]</a></sup> <a href='#SkPath_close'>close</a>

---

<a name='SkPath_rMoveTo'></a>
## rMoveTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_rMoveTo'>rMoveTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy)
</pre>

Adds beginning of <a href='#Contour'>Contour</a> relative to <a href='#Last_Point'>Last Point</a>.
If <a href='#Path'>Path</a> is empty, starts <a href='#Contour'>Contour</a> at (<a href='#SkPath_rMoveTo_dx'>dx</a>, <a href='#SkPath_rMoveTo_dy'>dy</a>).
Otherwise, start <a href='#Contour'>Contour</a> at <a href='#Last_Point'>Last Point</a> offset by (<a href='#SkPath_rMoveTo_dx'>dx</a>, <a href='#SkPath_rMoveTo_dy'>dy</a>).
Function name stands for "relative move to".

### Parameters

<table>  <tr>    <td><a name='SkPath_rMoveTo_dx'><code><strong>dx</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='#Contour'>Contour</a> start on x-axis</td>
  </tr>
  <tr>    <td><a name='SkPath_rMoveTo_dy'><code><strong>dy</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='#Contour'>Contour</a> start on y-axis</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="63e32dec4b2d8440b427f368bf8313a4"></fiddle-embed></div>

### See Also

<a href='#Contour'>Contour</a> <a href='#SkPath_lineTo'>lineTo</a><sup><a href='#SkPath_lineTo_2'>[2]</a></sup> <a href='#SkPath_moveTo'>moveTo</a><sup><a href='#SkPath_moveTo_2'>[2]</a></sup> <a href='#SkPath_quadTo'>quadTo</a><sup><a href='#SkPath_quadTo_2'>[2]</a></sup> <a href='#SkPath_conicTo'>conicTo</a><sup><a href='#SkPath_conicTo_2'>[2]</a></sup> <a href='#SkPath_cubicTo'>cubicTo</a><sup><a href='#SkPath_cubicTo_2'>[2]</a></sup> <a href='#SkPath_close'>close</a>

---

<a name='SkPath_lineTo'></a>
## lineTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_lineTo'>lineTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y)
</pre>

Adds <a href='undocumented#Line'>Line</a> from <a href='#Last_Point'>Last Point</a> to (<a href='#SkPath_lineTo_x'>x</a>, <a href='#SkPath_lineTo_y'>y</a>). If <a href='#Path'>Path</a> is empty, or last <a href='#SkPath_Verb'>Verb</a> is
<a href='#SkPath_kClose_Verb'>kClose Verb</a>, <a href='#Last_Point'>Last Point</a> is set to (0, 0) before adding <a href='undocumented#Line'>Line</a>.

<a href='#SkPath_lineTo'>lineTo</a> appends <a href='#SkPath_kMove_Verb'>kMove Verb</a> to <a href='#Verb_Array'>Verb Array</a> and (0, 0) to <a href='#Point_Array'>Point Array</a>, if needed.
<a href='#SkPath_lineTo'>lineTo</a> then appends <a href='#SkPath_kLine_Verb'>kLine Verb</a> to <a href='#Verb_Array'>Verb Array</a> and (<a href='#SkPath_lineTo_x'>x</a>, <a href='#SkPath_lineTo_y'>y</a>) to <a href='#Point_Array'>Point Array</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_lineTo_x'><code><strong>x</strong></code></a></td>
    <td>end of added <a href='undocumented#Line'>Line</a> in <a href='#SkPath_lineTo_x'>x</a></td>
  </tr>
  <tr>    <td><a name='SkPath_lineTo_y'><code><strong>y</strong></code></a></td>
    <td>end of added <a href='undocumented#Line'>Line</a> in <a href='#SkPath_lineTo_y'>y</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="e311cdd451edacec33b50cc22a4dd5dc"></fiddle-embed></div>

### See Also

<a href='#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a><sup><a href='#SkPath_moveTo_2'>[2]</a></sup> <a href='#SkPath_rLineTo'>rLineTo</a> <a href='#SkPath_addRect'>addRect</a><sup><a href='#SkPath_addRect_2'>[2]</a></sup><sup><a href='#SkPath_addRect_3'>[3]</a></sup>

---

<a name='SkPath_lineTo_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_lineTo'>lineTo</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p)
</pre>

Adds <a href='undocumented#Line'>Line</a> from <a href='#Last_Point'>Last Point</a> to <a href='SkPoint_Reference#Point'>Point</a> <a href='#SkPath_lineTo_2_p'>p</a>. If <a href='#Path'>Path</a> is empty, or last <a href='#SkPath_Verb'>Verb</a> is
<a href='#SkPath_kClose_Verb'>kClose Verb</a>, <a href='#Last_Point'>Last Point</a> is set to (0, 0) before adding <a href='undocumented#Line'>Line</a>.

<a href='#SkPath_lineTo'>lineTo</a> first appends <a href='#SkPath_kMove_Verb'>kMove Verb</a> to <a href='#Verb_Array'>Verb Array</a> and (0, 0) to <a href='#Point_Array'>Point Array</a>, if needed.
<a href='#SkPath_lineTo'>lineTo</a> then appends <a href='#SkPath_kLine_Verb'>kLine Verb</a> to <a href='#Verb_Array'>Verb Array</a> and <a href='SkPoint_Reference#Point'>Point</a> <a href='#SkPath_lineTo_2_p'>p</a> to <a href='#Point_Array'>Point Array</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_lineTo_2_p'><code><strong>p</strong></code></a></td>
    <td>end <a href='SkPoint_Reference#Point'>Point</a> of added <a href='undocumented#Line'>Line</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="41001546a7f7927d08e5a818bcc304f5"></fiddle-embed></div>

### See Also

<a href='#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a><sup><a href='#SkPath_moveTo_2'>[2]</a></sup> <a href='#SkPath_rLineTo'>rLineTo</a> <a href='#SkPath_addRect'>addRect</a><sup><a href='#SkPath_addRect_2'>[2]</a></sup><sup><a href='#SkPath_addRect_3'>[3]</a></sup>

---

<a name='SkPath_rLineTo'></a>
## rLineTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_rLineTo'>rLineTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy)
</pre>

Adds <a href='undocumented#Line'>Line</a> from <a href='#Last_Point'>Last Point</a> to <a href='SkPoint_Reference#Vector'>Vector</a> (<a href='#SkPath_rLineTo_dx'>dx</a>, <a href='#SkPath_rLineTo_dy'>dy</a>). If <a href='#Path'>Path</a> is empty, or last <a href='#SkPath_Verb'>Verb</a> is
<a href='#SkPath_kClose_Verb'>kClose Verb</a>, <a href='#Last_Point'>Last Point</a> is set to (0, 0) before adding <a href='undocumented#Line'>Line</a>.

Appends <a href='#SkPath_kMove_Verb'>kMove Verb</a> to <a href='#Verb_Array'>Verb Array</a> and (0, 0) to <a href='#Point_Array'>Point Array</a>, if needed;
then appends <a href='#SkPath_kLine_Verb'>kLine Verb</a> to <a href='#Verb_Array'>Verb Array</a> and <a href='undocumented#Line'>Line</a> end to <a href='#Point_Array'>Point Array</a>.
<a href='undocumented#Line'>Line</a> end is <a href='#Last_Point'>Last Point</a> plus <a href='SkPoint_Reference#Vector'>Vector</a> (<a href='#SkPath_rLineTo_dx'>dx</a>, <a href='#SkPath_rLineTo_dy'>dy</a>).
Function name stands for "relative line to".

### Parameters

<table>  <tr>    <td><a name='SkPath_rLineTo_dx'><code><strong>dx</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='undocumented#Line'>Line</a> end on x-axis</td>
  </tr>
  <tr>    <td><a name='SkPath_rLineTo_dy'><code><strong>dy</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='undocumented#Line'>Line</a> end on y-axis</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="6e0be0766b8ca320da51640326e608b3"></fiddle-embed></div>

### See Also

<a href='#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a><sup><a href='#SkPath_moveTo_2'>[2]</a></sup> <a href='#SkPath_lineTo'>lineTo</a><sup><a href='#SkPath_lineTo_2'>[2]</a></sup> <a href='#SkPath_addRect'>addRect</a><sup><a href='#SkPath_addRect_2'>[2]</a></sup><sup><a href='#SkPath_addRect_3'>[3]</a></sup>

---

## <a name='Quad'>Quad</a>

<a href='#Quad'>Quad</a> describes a quadratic Bezier, a second-order curve identical to a section
of a parabola. <a href='#Quad'>Quad</a> begins at a start <a href='SkPoint_Reference#Point'>Point</a>, curves towards a control <a href='SkPoint_Reference#Point'>Point</a>,
and then curves to an end <a href='SkPoint_Reference#Point'>Point</a>.

### Example

<div><fiddle-embed name="78ad51fa1cd33eb84a6f99061e56e067"></fiddle-embed></div>

<a href='#Quad'>Quad</a> is a special case of <a href='#Conic'>Conic</a> where <a href='#Conic_Weight'>Conic Weight</a> is set to one.

<a href='#Quad'>Quad</a> is always contained by the triangle connecting its three <a href='SkPoint_Reference#Point'>Points</a>. <a href='#Quad'>Quad</a>
begins tangent to the line between start <a href='SkPoint_Reference#Point'>Point</a> and control <a href='SkPoint_Reference#Point'>Point</a>, and ends
tangent to the line between control <a href='SkPoint_Reference#Point'>Point</a> and end <a href='SkPoint_Reference#Point'>Point</a>.

### Example

<div><fiddle-embed name="4082f66a42df11bb20462b232b156bb6"></fiddle-embed></div>

<a name='SkPath_quadTo'></a>
## quadTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_quadTo'>quadTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> x1, <a href='undocumented#SkScalar'>SkScalar</a> y1, <a href='undocumented#SkScalar'>SkScalar</a> x2, <a href='undocumented#SkScalar'>SkScalar</a> y2)
</pre>

Adds <a href='#Quad'>Quad</a> from <a href='#Last_Point'>Last Point</a> towards (<a href='#SkPath_quadTo_x1'>x1</a>, <a href='#SkPath_quadTo_y1'>y1</a>), to (<a href='#SkPath_quadTo_x2'>x2</a>, <a href='#SkPath_quadTo_y2'>y2</a>).
If <a href='#Path'>Path</a> is empty, or last <a href='#SkPath_Verb'>Verb</a> is <a href='#SkPath_kClose_Verb'>kClose Verb</a>, <a href='#Last_Point'>Last Point</a> is set to (0, 0)
before adding <a href='#Quad'>Quad</a>.

Appends <a href='#SkPath_kMove_Verb'>kMove Verb</a> to <a href='#Verb_Array'>Verb Array</a> and (0, 0) to <a href='#Point_Array'>Point Array</a>, if needed;
then appends <a href='#SkPath_kQuad_Verb'>kQuad Verb</a> to <a href='#Verb_Array'>Verb Array</a>; and (<a href='#SkPath_quadTo_x1'>x1</a>, <a href='#SkPath_quadTo_y1'>y1</a>), (<a href='#SkPath_quadTo_x2'>x2</a>, <a href='#SkPath_quadTo_y2'>y2</a>)
to <a href='#Point_Array'>Point Array</a>.

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

### Example

<div><fiddle-embed name="60ee3eb747474f5781b0f0dd3a17a866"></fiddle-embed></div>

### See Also

<a href='#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a><sup><a href='#SkPath_moveTo_2'>[2]</a></sup> <a href='#SkPath_conicTo'>conicTo</a><sup><a href='#SkPath_conicTo_2'>[2]</a></sup> <a href='#SkPath_rQuadTo'>rQuadTo</a>

---

<a name='SkPath_quadTo_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_quadTo'>quadTo</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p1, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p2)
</pre>

Adds <a href='#Quad'>Quad</a> from <a href='#Last_Point'>Last Point</a> towards <a href='SkPoint_Reference#Point'>Point</a> <a href='#SkPath_quadTo_2_p1'>p1</a>, to <a href='SkPoint_Reference#Point'>Point</a> <a href='#SkPath_quadTo_2_p2'>p2</a>.
If <a href='#Path'>Path</a> is empty, or last <a href='#SkPath_Verb'>Verb</a> is <a href='#SkPath_kClose_Verb'>kClose Verb</a>, <a href='#Last_Point'>Last Point</a> is set to (0, 0)
before adding <a href='#Quad'>Quad</a>.

Appends <a href='#SkPath_kMove_Verb'>kMove Verb</a> to <a href='#Verb_Array'>Verb Array</a> and (0, 0) to <a href='#Point_Array'>Point Array</a>, if needed;
then appends <a href='#SkPath_kQuad_Verb'>kQuad Verb</a> to <a href='#Verb_Array'>Verb Array</a>; and <a href='SkPoint_Reference#Point'>Points</a> <a href='#SkPath_quadTo_2_p1'>p1</a>, <a href='#SkPath_quadTo_2_p2'>p2</a>
to <a href='#Point_Array'>Point Array</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_quadTo_2_p1'><code><strong>p1</strong></code></a></td>
    <td>control <a href='SkPoint_Reference#Point'>Point</a> of added <a href='#Quad'>Quad</a></td>
  </tr>
  <tr>    <td><a name='SkPath_quadTo_2_p2'><code><strong>p2</strong></code></a></td>
    <td>end <a href='SkPoint_Reference#Point'>Point</a> of added <a href='#Quad'>Quad</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="82621c4df8da1e589d9e627494067826"></fiddle-embed></div>

### See Also

<a href='#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a><sup><a href='#SkPath_moveTo_2'>[2]</a></sup> <a href='#SkPath_conicTo'>conicTo</a><sup><a href='#SkPath_conicTo_2'>[2]</a></sup> <a href='#SkPath_rQuadTo'>rQuadTo</a>

---

<a name='SkPath_rQuadTo'></a>
## rQuadTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_rQuadTo'>rQuadTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx1, <a href='undocumented#SkScalar'>SkScalar</a> dy1, <a href='undocumented#SkScalar'>SkScalar</a> dx2, <a href='undocumented#SkScalar'>SkScalar</a> dy2)
</pre>

Adds <a href='#Quad'>Quad</a> from <a href='#Last_Point'>Last Point</a> towards <a href='SkPoint_Reference#Vector'>Vector</a> (<a href='#SkPath_rQuadTo_dx1'>dx1</a>, <a href='#SkPath_rQuadTo_dy1'>dy1</a>), to <a href='SkPoint_Reference#Vector'>Vector</a> (<a href='#SkPath_rQuadTo_dx2'>dx2</a>, <a href='#SkPath_rQuadTo_dy2'>dy2</a>).
If <a href='#Path'>Path</a> is empty, or last <a href='#SkPath_Verb'>Verb</a>
is <a href='#SkPath_kClose_Verb'>kClose Verb</a>, <a href='#Last_Point'>Last Point</a> is set to (0, 0) before adding <a href='#Quad'>Quad</a>.

Appends <a href='#SkPath_kMove_Verb'>kMove Verb</a> to <a href='#Verb_Array'>Verb Array</a> and (0, 0) to <a href='#Point_Array'>Point Array</a>,
if needed; then appends <a href='#SkPath_kQuad_Verb'>kQuad Verb</a> to <a href='#Verb_Array'>Verb Array</a>; and appends <a href='#Quad'>Quad</a>
control and <a href='#Quad'>Quad</a> end to <a href='#Point_Array'>Point Array</a>.
<a href='#Quad'>Quad</a> control is <a href='#Last_Point'>Last Point</a> plus <a href='SkPoint_Reference#Vector'>Vector</a> (<a href='#SkPath_rQuadTo_dx1'>dx1</a>, <a href='#SkPath_rQuadTo_dy1'>dy1</a>).
<a href='#Quad'>Quad</a> end is <a href='#Last_Point'>Last Point</a> plus <a href='SkPoint_Reference#Vector'>Vector</a> (<a href='#SkPath_rQuadTo_dx2'>dx2</a>, <a href='#SkPath_rQuadTo_dy2'>dy2</a>).
Function name stands for "relative quad to".

### Parameters

<table>  <tr>    <td><a name='SkPath_rQuadTo_dx1'><code><strong>dx1</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='#Quad'>Quad</a> control on x-axis</td>
  </tr>
  <tr>    <td><a name='SkPath_rQuadTo_dy1'><code><strong>dy1</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='#Quad'>Quad</a> control on y-axis</td>
  </tr>
  <tr>    <td><a name='SkPath_rQuadTo_dx2'><code><strong>dx2</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='#Quad'>Quad</a> end on x-axis</td>
  </tr>
  <tr>    <td><a name='SkPath_rQuadTo_dy2'><code><strong>dy2</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='#Quad'>Quad</a> end on y-axis</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1c1f4cdef1c572c9aa8fdf3e461191d0"></fiddle-embed></div>

### See Also

<a href='#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a><sup><a href='#SkPath_moveTo_2'>[2]</a></sup> <a href='#SkPath_conicTo'>conicTo</a><sup><a href='#SkPath_conicTo_2'>[2]</a></sup> <a href='#SkPath_quadTo'>quadTo</a><sup><a href='#SkPath_quadTo_2'>[2]</a></sup>

---

## <a name='Conic'>Conic</a>

<a href='#Conic'>Conic</a> describes a conical section: a piece of an ellipse, or a piece of a
parabola, or a piece of a hyperbola. <a href='#Conic'>Conic</a> begins at a start <a href='SkPoint_Reference#Point'>Point</a>,
curves towards a control <a href='SkPoint_Reference#Point'>Point</a>, and then curves to an end <a href='SkPoint_Reference#Point'>Point</a>. The influence
of the control <a href='SkPoint_Reference#Point'>Point</a> is determined by <a href='#Conic_Weight'>Conic Weight</a>.

Each <a href='#Conic'>Conic</a> in <a href='#Path'>Path</a> adds two <a href='SkPoint_Reference#Point'>Points</a> and one <a href='#Conic_Weight'>Conic Weight</a>. <a href='#Conic_Weight'>Conic Weights</a> in <a href='#Path'>Path</a>
may be inspected with <a href='#SkPath_Iter'>Iter</a>, or with <a href='#SkPath_RawIter'>RawIter</a>.

## <a name='Conic_Weight'>Conic Weight</a>

<a href='#Conic_Weight'>Weight</a> determines both the strength of the control <a href='SkPoint_Reference#Point'>Point</a> and the type of <a href='#Conic'>Conic</a>.
<a href='#Conic_Weight'>Weight</a> varies from zero to infinity. At zero, <a href='#Conic_Weight'>Weight</a> causes the control <a href='SkPoint_Reference#Point'>Point</a> to
have no effect; <a href='#Conic'>Conic</a> is identical to a line segment from start <a href='SkPoint_Reference#Point'>Point</a> to end
point. If <a href='#Conic_Weight'>Weight</a> is less than one, <a href='#Conic'>Conic</a> follows an elliptical arc.
If <a href='#Conic_Weight'>Weight</a> is exactly one, then <a href='#Conic'>Conic</a> is identical to <a href='#Quad'>Quad</a>; <a href='#Conic'>Conic</a> follows a
parabolic arc. If <a href='#Conic_Weight'>Weight</a> is greater than one, <a href='#Conic'>Conic</a> follows a hyperbolic
arc. If <a href='#Conic_Weight'>Weight</a> is infinity, <a href='#Conic'>Conic</a> is identical to two line segments, connecting
start <a href='SkPoint_Reference#Point'>Point</a> to control <a href='SkPoint_Reference#Point'>Point</a>, and control <a href='SkPoint_Reference#Point'>Point</a> to end <a href='SkPoint_Reference#Point'>Point</a>.

### Example

<div><fiddle-embed name="2aadded3d20dfef34d1c8abe28c7bc8d"><div>When <a href='#Conic_Weight'>Conic Weight</a> is one, <a href='#Quad'>Quad</a> is added to path; the two are identical.
</div>

#### Example Output

~~~~
move {0, 0},
quad {0, 0}, {20, 30}, {50, 60},
done
~~~~

</fiddle-embed></div>

If weight is less than one, <a href='#Conic'>Conic</a> is an elliptical segment.

### Example

<div><fiddle-embed name="e88f554efacfa9f75f270fb1c0add5b4"><div>A 90 degree circular arc has the weight1 / sqrt(2).
</div>

#### Example Output

~~~~
move {0, 0},
conic {0, 0}, {20, 0}, {20, 20}, weight = 0.707107
done
~~~~

</fiddle-embed></div>

If weight is greater than one, <a href='#Conic'>Conic</a> is a hyperbolic segment. As weight gets large,
a hyperbolic segment can be approximated by straight lines connecting the
control <a href='SkPoint_Reference#Point'>Point</a> with the end <a href='SkPoint_Reference#Point'>Points</a>.

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
void <a href='#SkPath_conicTo'>conicTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> x1, <a href='undocumented#SkScalar'>SkScalar</a> y1, <a href='undocumented#SkScalar'>SkScalar</a> x2, <a href='undocumented#SkScalar'>SkScalar</a> y2, <a href='undocumented#SkScalar'>SkScalar</a> w)
</pre>

Adds <a href='#Conic'>Conic</a> from <a href='#Last_Point'>Last Point</a> towards (<a href='#SkPath_conicTo_x1'>x1</a>, <a href='#SkPath_conicTo_y1'>y1</a>), to (<a href='#SkPath_conicTo_x2'>x2</a>, <a href='#SkPath_conicTo_y2'>y2</a>), weighted by <a href='#SkPath_conicTo_w'>w</a>.
If <a href='#Path'>Path</a> is empty, or last <a href='#SkPath_Verb'>Verb</a> is <a href='#SkPath_kClose_Verb'>kClose Verb</a>, <a href='#Last_Point'>Last Point</a> is set to (0, 0)
before adding <a href='#Conic'>Conic</a>.

Appends <a href='#SkPath_kMove_Verb'>kMove Verb</a> to <a href='#Verb_Array'>Verb Array</a> and (0, 0) to <a href='#Point_Array'>Point Array</a>, if needed.

If <a href='#SkPath_conicTo_w'>w</a> is finite and not one, appends <a href='#SkPath_kConic_Verb'>kConic Verb</a> to <a href='#Verb_Array'>Verb Array</a>;
and (<a href='#SkPath_conicTo_x1'>x1</a>, <a href='#SkPath_conicTo_y1'>y1</a>), (<a href='#SkPath_conicTo_x2'>x2</a>, <a href='#SkPath_conicTo_y2'>y2</a>) to <a href='#Point_Array'>Point Array</a>; and <a href='#SkPath_conicTo_w'>w</a> to <a href='#Conic_Weight'>Conic Weights</a>.

If <a href='#SkPath_conicTo_w'>w</a> is one, appends <a href='#SkPath_kQuad_Verb'>kQuad Verb</a> to <a href='#Verb_Array'>Verb Array</a>, and
(<a href='#SkPath_conicTo_x1'>x1</a>, <a href='#SkPath_conicTo_y1'>y1</a>), (<a href='#SkPath_conicTo_x2'>x2</a>, <a href='#SkPath_conicTo_y2'>y2</a>) to <a href='#Point_Array'>Point Array</a>.

If <a href='#SkPath_conicTo_w'>w</a> is not finite, appends <a href='#SkPath_kLine_Verb'>kLine Verb</a> twice to <a href='#Verb_Array'>Verb Array</a>, and
(<a href='#SkPath_conicTo_x1'>x1</a>, <a href='#SkPath_conicTo_y1'>y1</a>), (<a href='#SkPath_conicTo_x2'>x2</a>, <a href='#SkPath_conicTo_y2'>y2</a>) to <a href='#Point_Array'>Point Array</a>.

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

### Example

<div><fiddle-embed name="358d9b6060b528b0923c007420f09c13"><div>As weight increases, curve is pulled towards control point.
The bottom two curves are elliptical; the next is parabolic; the
top curve is hyperbolic.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_rConicTo'>rConicTo</a> <a href='#SkPath_arcTo'>arcTo</a><sup><a href='#SkPath_arcTo_2'>[2]</a></sup><sup><a href='#SkPath_arcTo_3'>[3]</a></sup><sup><a href='#SkPath_arcTo_4'>[4]</a></sup><sup><a href='#SkPath_arcTo_5'>[5]</a></sup> <a href='#SkPath_addArc'>addArc</a> <a href='#SkPath_quadTo'>quadTo</a><sup><a href='#SkPath_quadTo_2'>[2]</a></sup>

---

<a name='SkPath_conicTo_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_conicTo'>conicTo</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p1, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p2, <a href='undocumented#SkScalar'>SkScalar</a> w)
</pre>

Adds <a href='#Conic'>Conic</a> from <a href='#Last_Point'>Last Point</a> towards <a href='SkPoint_Reference#Point'>Point</a> <a href='#SkPath_conicTo_2_p1'>p1</a>, to <a href='SkPoint_Reference#Point'>Point</a> <a href='#SkPath_conicTo_2_p2'>p2</a>, weighted by <a href='#SkPath_conicTo_2_w'>w</a>.
If <a href='#Path'>Path</a> is empty, or last <a href='#SkPath_Verb'>Verb</a> is <a href='#SkPath_kClose_Verb'>kClose Verb</a>, <a href='#Last_Point'>Last Point</a> is set to (0, 0)
before adding <a href='#Conic'>Conic</a>.

Appends <a href='#SkPath_kMove_Verb'>kMove Verb</a> to <a href='#Verb_Array'>Verb Array</a> and (0, 0) to <a href='#Point_Array'>Point Array</a>, if needed.

If <a href='#SkPath_conicTo_2_w'>w</a> is finite and not one, appends <a href='#SkPath_kConic_Verb'>kConic Verb</a> to <a href='#Verb_Array'>Verb Array</a>;
and <a href='SkPoint_Reference#Point'>Points</a> <a href='#SkPath_conicTo_2_p1'>p1</a>, <a href='#SkPath_conicTo_2_p2'>p2</a> to <a href='#Point_Array'>Point Array</a>; and <a href='#SkPath_conicTo_2_w'>w</a> to <a href='#Conic_Weight'>Conic Weights</a>.

If <a href='#SkPath_conicTo_2_w'>w</a> is one, appends <a href='#SkPath_kQuad_Verb'>kQuad Verb</a> to <a href='#Verb_Array'>Verb Array</a>, and <a href='SkPoint_Reference#Point'>Points</a> <a href='#SkPath_conicTo_2_p1'>p1</a>, <a href='#SkPath_conicTo_2_p2'>p2</a>
to <a href='#Point_Array'>Point Array</a>.

If <a href='#SkPath_conicTo_2_w'>w</a> is not finite, appends <a href='#SkPath_kLine_Verb'>kLine Verb</a> twice to <a href='#Verb_Array'>Verb Array</a>, and
<a href='SkPoint_Reference#Point'>Points</a> <a href='#SkPath_conicTo_2_p1'>p1</a>, <a href='#SkPath_conicTo_2_p2'>p2</a> to <a href='#Point_Array'>Point Array</a>.

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

### Example

<div><fiddle-embed name="22d25e03b19d5bae92118877e462361b"><div><a href='#Conic'>Conics</a> and arcs use identical representations. As the arc sweep increases
the <a href='#Conic_Weight'>Conic Weight</a> also increases, but remains smaller than one.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_rConicTo'>rConicTo</a> <a href='#SkPath_arcTo'>arcTo</a><sup><a href='#SkPath_arcTo_2'>[2]</a></sup><sup><a href='#SkPath_arcTo_3'>[3]</a></sup><sup><a href='#SkPath_arcTo_4'>[4]</a></sup><sup><a href='#SkPath_arcTo_5'>[5]</a></sup> <a href='#SkPath_addArc'>addArc</a> <a href='#SkPath_quadTo'>quadTo</a><sup><a href='#SkPath_quadTo_2'>[2]</a></sup>

---

<a name='SkPath_rConicTo'></a>
## rConicTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_rConicTo'>rConicTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx1, <a href='undocumented#SkScalar'>SkScalar</a> dy1, <a href='undocumented#SkScalar'>SkScalar</a> dx2, <a href='undocumented#SkScalar'>SkScalar</a> dy2, <a href='undocumented#SkScalar'>SkScalar</a> w)
</pre>

Adds <a href='#Conic'>Conic</a> from <a href='#Last_Point'>Last Point</a> towards <a href='SkPoint_Reference#Vector'>Vector</a> (<a href='#SkPath_rConicTo_dx1'>dx1</a>, <a href='#SkPath_rConicTo_dy1'>dy1</a>), to <a href='SkPoint_Reference#Vector'>Vector</a> (<a href='#SkPath_rConicTo_dx2'>dx2</a>, <a href='#SkPath_rConicTo_dy2'>dy2</a>),
weighted by <a href='#SkPath_rConicTo_w'>w</a>. If <a href='#Path'>Path</a> is empty, or last <a href='#SkPath_Verb'>Verb</a>
is <a href='#SkPath_kClose_Verb'>kClose Verb</a>, <a href='#Last_Point'>Last Point</a> is set to (0, 0) before adding <a href='#Conic'>Conic</a>.

Appends <a href='#SkPath_kMove_Verb'>kMove Verb</a> to <a href='#Verb_Array'>Verb Array</a> and (0, 0) to <a href='#Point_Array'>Point Array</a>,
if needed.

If <a href='#SkPath_rConicTo_w'>w</a> is finite and not one, next appends <a href='#SkPath_kConic_Verb'>kConic Verb</a> to <a href='#Verb_Array'>Verb Array</a>,
and <a href='#SkPath_rConicTo_w'>w</a> is recorded as <a href='#Conic_Weight'>Conic Weight</a>; otherwise, if <a href='#SkPath_rConicTo_w'>w</a> is one, appends
<a href='#SkPath_kQuad_Verb'>kQuad Verb</a> to <a href='#Verb_Array'>Verb Array</a>; or if <a href='#SkPath_rConicTo_w'>w</a> is not finite, appends <a href='#SkPath_kLine_Verb'>kLine Verb</a>
twice to <a href='#Verb_Array'>Verb Array</a>.

In all cases appends <a href='SkPoint_Reference#Point'>Points</a> control and end to <a href='#Point_Array'>Point Array</a>.
control is <a href='#Last_Point'>Last Point</a> plus <a href='SkPoint_Reference#Vector'>Vector</a> (<a href='#SkPath_rConicTo_dx1'>dx1</a>, <a href='#SkPath_rConicTo_dy1'>dy1</a>).
end is <a href='#Last_Point'>Last Point</a> plus <a href='SkPoint_Reference#Vector'>Vector</a> (<a href='#SkPath_rConicTo_dx2'>dx2</a>, <a href='#SkPath_rConicTo_dy2'>dy2</a>).

Function name stands for "relative conic to".

### Parameters

<table>  <tr>    <td><a name='SkPath_rConicTo_dx1'><code><strong>dx1</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='#Conic'>Conic</a> control on x-axis</td>
  </tr>
  <tr>    <td><a name='SkPath_rConicTo_dy1'><code><strong>dy1</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='#Conic'>Conic</a> control on y-axis</td>
  </tr>
  <tr>    <td><a name='SkPath_rConicTo_dx2'><code><strong>dx2</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='#Conic'>Conic</a> end on x-axis</td>
  </tr>
  <tr>    <td><a name='SkPath_rConicTo_dy2'><code><strong>dy2</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='#Conic'>Conic</a> end on y-axis</td>
  </tr>
  <tr>    <td><a name='SkPath_rConicTo_w'><code><strong>w</strong></code></a></td>
    <td>weight of added <a href='#Conic'>Conic</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3d52763e7c0e20c0b1d484a0afa622d2"></fiddle-embed></div>

### See Also

<a href='#SkPath_conicTo'>conicTo</a><sup><a href='#SkPath_conicTo_2'>[2]</a></sup> <a href='#SkPath_arcTo'>arcTo</a><sup><a href='#SkPath_arcTo_2'>[2]</a></sup><sup><a href='#SkPath_arcTo_3'>[3]</a></sup><sup><a href='#SkPath_arcTo_4'>[4]</a></sup><sup><a href='#SkPath_arcTo_5'>[5]</a></sup> <a href='#SkPath_addArc'>addArc</a> <a href='#SkPath_quadTo'>quadTo</a><sup><a href='#SkPath_quadTo_2'>[2]</a></sup>

---

## <a name='Cubic'>Cubic</a>

<a href='#Cubic'>Cubic</a> describes a <a href='undocumented#Bezier_Curve'>Bezier Curve</a> segment described by a third-order polynomial.
<a href='#Cubic'>Cubic</a> begins at a start <a href='SkPoint_Reference#Point'>Point</a>, curving towards the first control <a href='SkPoint_Reference#Point'>Point</a>;
and curves from the end <a href='SkPoint_Reference#Point'>Point</a> towards the second control <a href='SkPoint_Reference#Point'>Point</a>.

### Example

<div><fiddle-embed name="466445ed991d86de08587066392d654a"></fiddle-embed></div>

<a name='SkPath_cubicTo'></a>
## cubicTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_cubicTo'>cubicTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> x1, <a href='undocumented#SkScalar'>SkScalar</a> y1, <a href='undocumented#SkScalar'>SkScalar</a> x2, <a href='undocumented#SkScalar'>SkScalar</a> y2, <a href='undocumented#SkScalar'>SkScalar</a> x3, <a href='undocumented#SkScalar'>SkScalar</a> y3)
</pre>

Adds <a href='#Cubic'>Cubic</a> from <a href='#Last_Point'>Last Point</a> towards (<a href='#SkPath_cubicTo_x1'>x1</a>, <a href='#SkPath_cubicTo_y1'>y1</a>), then towards (<a href='#SkPath_cubicTo_x2'>x2</a>, <a href='#SkPath_cubicTo_y2'>y2</a>), ending at
(<a href='#SkPath_cubicTo_x3'>x3</a>, <a href='#SkPath_cubicTo_y3'>y3</a>). If <a href='#Path'>Path</a> is empty, or last <a href='#SkPath_Verb'>Verb</a> is <a href='#SkPath_kClose_Verb'>kClose Verb</a>, <a href='#Last_Point'>Last Point</a> is set to
(0, 0) before adding <a href='#Cubic'>Cubic</a>.

Appends <a href='#SkPath_kMove_Verb'>kMove Verb</a> to <a href='#Verb_Array'>Verb Array</a> and (0, 0) to <a href='#Point_Array'>Point Array</a>, if needed;
then appends <a href='#SkPath_kCubic_Verb'>kCubic Verb</a> to <a href='#Verb_Array'>Verb Array</a>; and (<a href='#SkPath_cubicTo_x1'>x1</a>, <a href='#SkPath_cubicTo_y1'>y1</a>), (<a href='#SkPath_cubicTo_x2'>x2</a>, <a href='#SkPath_cubicTo_y2'>y2</a>), (<a href='#SkPath_cubicTo_x3'>x3</a>, <a href='#SkPath_cubicTo_y3'>y3</a>)
to <a href='#Point_Array'>Point Array</a>.

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

### Example

<div><fiddle-embed name="3e476378e3e0550ab134bbaf61112d98"></fiddle-embed></div>

### See Also

<a href='#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a><sup><a href='#SkPath_moveTo_2'>[2]</a></sup> <a href='#SkPath_rCubicTo'>rCubicTo</a> <a href='#SkPath_quadTo'>quadTo</a><sup><a href='#SkPath_quadTo_2'>[2]</a></sup>

---

<a name='SkPath_cubicTo_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_cubicTo'>cubicTo</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p1, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p2, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p3)
</pre>

Adds <a href='#Cubic'>Cubic</a> from <a href='#Last_Point'>Last Point</a> towards <a href='SkPoint_Reference#Point'>Point</a> <a href='#SkPath_cubicTo_2_p1'>p1</a>, then towards <a href='SkPoint_Reference#Point'>Point</a> <a href='#SkPath_cubicTo_2_p2'>p2</a>, ending at
<a href='SkPoint_Reference#Point'>Point</a> <a href='#SkPath_cubicTo_2_p3'>p3</a>. If <a href='#Path'>Path</a> is empty, or last <a href='#SkPath_Verb'>Verb</a> is <a href='#SkPath_kClose_Verb'>kClose Verb</a>, <a href='#Last_Point'>Last Point</a> is set to
(0, 0) before adding <a href='#Cubic'>Cubic</a>.

Appends <a href='#SkPath_kMove_Verb'>kMove Verb</a> to <a href='#Verb_Array'>Verb Array</a> and (0, 0) to <a href='#Point_Array'>Point Array</a>, if needed;
then appends <a href='#SkPath_kCubic_Verb'>kCubic Verb</a> to <a href='#Verb_Array'>Verb Array</a>; and <a href='SkPoint_Reference#Point'>Points</a> <a href='#SkPath_cubicTo_2_p1'>p1</a>, <a href='#SkPath_cubicTo_2_p2'>p2</a>, <a href='#SkPath_cubicTo_2_p3'>p3</a>
to <a href='#Point_Array'>Point Array</a>.

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

### Example

<div><fiddle-embed name="d38aaf12c6ff5b8d901a2201bcee5476"></fiddle-embed></div>

### See Also

<a href='#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a><sup><a href='#SkPath_moveTo_2'>[2]</a></sup> <a href='#SkPath_rCubicTo'>rCubicTo</a> <a href='#SkPath_quadTo'>quadTo</a><sup><a href='#SkPath_quadTo_2'>[2]</a></sup>

---

<a name='SkPath_rCubicTo'></a>
## rCubicTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_rCubicTo'>rCubicTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> x1, <a href='undocumented#SkScalar'>SkScalar</a> y1, <a href='undocumented#SkScalar'>SkScalar</a> x2, <a href='undocumented#SkScalar'>SkScalar</a> y2, <a href='undocumented#SkScalar'>SkScalar</a> x3, <a href='undocumented#SkScalar'>SkScalar</a> y3)
</pre>

Adds <a href='#Cubic'>Cubic</a> from <a href='#Last_Point'>Last Point</a> towards <a href='SkPoint_Reference#Vector'>Vector</a> (dx1, dy1), then towards
<a href='SkPoint_Reference#Vector'>Vector</a> (dx2, dy2), to <a href='SkPoint_Reference#Vector'>Vector</a> (dx3, dy3).
If <a href='#Path'>Path</a> is empty, or last <a href='#SkPath_Verb'>Verb</a>
is <a href='#SkPath_kClose_Verb'>kClose Verb</a>, <a href='#Last_Point'>Last Point</a> is set to (0, 0) before adding <a href='#Cubic'>Cubic</a>.

Appends <a href='#SkPath_kMove_Verb'>kMove Verb</a> to <a href='#Verb_Array'>Verb Array</a> and (0, 0) to <a href='#Point_Array'>Point Array</a>,
if needed; then appends <a href='#SkPath_kCubic_Verb'>kCubic Verb</a> to <a href='#Verb_Array'>Verb Array</a>; and appends <a href='#Cubic'>Cubic</a>
control and <a href='#Cubic'>Cubic</a> end to <a href='#Point_Array'>Point Array</a>.
<a href='#Cubic'>Cubic</a> control is <a href='#Last_Point'>Last Point</a> plus <a href='SkPoint_Reference#Vector'>Vector</a> (dx1, dy1).
<a href='#Cubic'>Cubic</a> end is <a href='#Last_Point'>Last Point</a> plus <a href='SkPoint_Reference#Vector'>Vector</a> (dx2, dy2).
Function name stands for "relative cubic to".

### Parameters

<table>  <tr>    <td><a name='SkPath_rCubicTo_x1'><code><strong>x1</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to first <a href='#Cubic'>Cubic</a> control on x-axis</td>
  </tr>
  <tr>    <td><a name='SkPath_rCubicTo_y1'><code><strong>y1</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to first <a href='#Cubic'>Cubic</a> control on y-axis</td>
  </tr>
  <tr>    <td><a name='SkPath_rCubicTo_x2'><code><strong>x2</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to second <a href='#Cubic'>Cubic</a> control on x-axis</td>
  </tr>
  <tr>    <td><a name='SkPath_rCubicTo_y2'><code><strong>y2</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to second <a href='#Cubic'>Cubic</a> control on y-axis</td>
  </tr>
  <tr>    <td><a name='SkPath_rCubicTo_x3'><code><strong>x3</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='#Cubic'>Cubic</a> end on x-axis</td>
  </tr>
  <tr>    <td><a name='SkPath_rCubicTo_y3'><code><strong>y3</strong></code></a></td>
    <td>offset from <a href='#Last_Point'>Last Point</a> to <a href='#Cubic'>Cubic</a> end on y-axis</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="19f0cfc7eeba8937fe19446ec0b5f932"></fiddle-embed></div>

### See Also

<a href='#Contour'>Contour</a> <a href='#SkPath_moveTo'>moveTo</a><sup><a href='#SkPath_moveTo_2'>[2]</a></sup> <a href='#SkPath_cubicTo'>cubicTo</a><sup><a href='#SkPath_cubicTo_2'>[2]</a></sup> <a href='#SkPath_quadTo'>quadTo</a><sup><a href='#SkPath_quadTo_2'>[2]</a></sup>

---

## <a name='Arc'>Arc</a>

<a href='#Arc'>Arc</a> can be constructed in a number of ways. <a href='#Arc'>Arc</a> may be described by part of <a href='undocumented#Oval'>Oval</a> and angles,
by start point and end point, and by radius and tangent lines. Each construction has advantages,
and some constructions correspond to <a href='#Arc'>Arc</a> drawing in graphics standards.

All <a href='#Arc'>Arc</a> draws are implemented by one or more <a href='#Conic'>Conic</a> draws. When <a href='#Conic_Weight'>Conic Weight</a> is less than one,
<a href='#Conic'>Conic</a> describes an <a href='#Arc'>Arc</a> of some <a href='undocumented#Oval'>Oval</a> or <a href='undocumented#Circle'>Circle</a>.

<a href='#SkPath_arcTo'>arcTo(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle, bool forceMoveTo)</a>
describes <a href='#Arc'>Arc</a> as a piece of <a href='undocumented#Oval'>Oval</a>, beginning at start angle, sweeping clockwise or counterclockwise,
which may continue <a href='#Contour'>Contour</a> or start a new one. This construction is similar to <a href='undocumented#PostScript'>PostScript</a> and
<a href='undocumented#HTML_Canvas'>HTML Canvas</a> arcs. Variation <a href='#SkPath_addArc'>addArc</a> always starts new <a href='#Contour'>Contour</a>. Canvas::drawArc draws without
requiring <a href='#Path'>Path</a>.

<a href='#SkPath_arcTo_2'>arcTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar radius)</a>
describes <a href='#Arc'>Arc</a> as tangent to the line (x0, y0), (x1, y1) and tangent to the line (x1, y1), (x2, y2)
where (x0, y0) is the last <a href='SkPoint_Reference#Point'>Point</a> added to <a href='#Path'>Path</a>. This construction is similar to <a href='undocumented#PostScript'>PostScript</a> and
<a href='undocumented#HTML_Canvas'>HTML Canvas</a> arcs.

<a href='#SkPath_arcTo_4'>arcTo(SkScalar rx, SkScalar ry, SkScalar xAxisRotate, ArcSize largeArc, Direction sweep,
SkScalar x, SkScalar y)</a>
describes <a href='#Arc'>Arc</a> as part of <a href='undocumented#Oval'>Oval</a> with radii (rx, ry), beginning at
last <a href='SkPoint_Reference#Point'>Point</a> added to <a href='#Path'>Path</a> and ending at (x, y). More than one <a href='#Arc'>Arc</a> satisfies this criteria,
so additional values choose a single solution. This construction is similar to <a href='undocumented#SVG'>SVG</a> arcs.

<a href='#SkPath_conicTo'>conicTo</a> describes <a href='#Arc'>Arc</a> of less than 180 degrees as a pair of tangent lines and <a href='#Conic_Weight'>Conic Weight</a>.
<a href='#SkPath_conicTo'>conicTo</a> can represent any <a href='#Arc'>Arc</a> with a sweep less than 180 degrees at any rotation. All <a href='#SkPath_arcTo'>arcTo</a>
constructions are converted to <a href='#Conic'>Conic</a> data when added to <a href='#Path'>Path</a>.

### Example

<div><fiddle-embed name="891ac93abd0cdb27c4156685d3b1bb4c"><div>

<table>  <tr>
    <td><sup>1</sup> <a href='#SkPath_arcTo'>arcTo(const SkRect& oval, SkScalar startAngle, SkScalar sweepAngle, bool forceMoveTo)</a></td>
  </tr>  <tr>
    <td><sup>2</sup> parameter sets force MoveTo</td>
  </tr>  <tr>
    <td><sup>3</sup> start angle must be multiple of 90 degrees</td>
  </tr>  <tr>
    <td><sup>4</sup> <a href='#SkPath_arcTo_2'>arcTo(SkScalar x1, SkScalar y1, SkScalar x2, SkScalar y2, SkScalar radius)</a></td>
  </tr>  <tr>
    <td><sup>5</sup> <a href='#SkPath_arcTo_4'>arcTo(SkScalar rx, SkScalar ry, SkScalar xAxisRotate, ArcSize largeArc,
Direction sweep, SkScalar x, SkScalar y)</a></td>
  </tr>
</table>

</div></fiddle-embed></div>

### Example

<div><fiddle-embed name="5acc77eba0cb4d00bbf3a8f4db0c0aee"><div>1 describes an arc from an oval, a starting angle, and a sweep angle.
2 is similar to 1, but does not require building a path to draw.
3 is similar to 1, but always begins new <a href='#Contour'>Contour</a>.
4 describes an arc from a pair of tangent lines and a radius.
5 describes an arc from <a href='undocumented#Oval'>Oval</a> center, arc start <a href='SkPoint_Reference#Point'>Point</a> and arc end <a href='SkPoint_Reference#Point'>Point</a>.
6 describes an arc from a pair of tangent lines and a <a href='#Conic_Weight'>Conic Weight</a>.
</div></fiddle-embed></div>

<a name='SkPath_arcTo'></a>
## arcTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_arcTo'>arcTo</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& oval, <a href='undocumented#SkScalar'>SkScalar</a> startAngle, <a href='undocumented#SkScalar'>SkScalar</a> sweepAngle, bool forceMoveTo)
</pre>

Appends <a href='#Arc'>Arc</a> to <a href='#Path'>Path</a>. <a href='#Arc'>Arc</a> added is part of ellipse
bounded by <a href='#SkPath_arcTo_oval'>oval</a>, from <a href='#SkPath_arcTo_startAngle'>startAngle</a> through <a href='#SkPath_arcTo_sweepAngle'>sweepAngle</a>. Both <a href='#SkPath_arcTo_startAngle'>startAngle</a> and
<a href='#SkPath_arcTo_sweepAngle'>sweepAngle</a> are measured in degrees, where zero degrees is aligned with the
positive x-axis, and positive sweeps extends <a href='#Arc'>Arc</a> clockwise.

<a href='#SkPath_arcTo'>arcTo</a> adds <a href='undocumented#Line'>Line</a> connecting <a href='#Path'>Path</a> last <a href='SkPoint_Reference#Point'>Point</a> to initial <a href='#Arc'>Arc</a> <a href='SkPoint_Reference#Point'>Point</a> if <a href='#SkPath_arcTo_forceMoveTo'>forceMoveTo</a>
is false and <a href='#Path'>Path</a> is not empty. Otherwise, added <a href='#Contour'>Contour</a> begins with first point
of <a href='#Arc'>Arc</a>. Angles greater than -360 and less than 360 are treated modulo 360.

### Parameters

<table>  <tr>    <td><a name='SkPath_arcTo_oval'><code><strong>oval</strong></code></a></td>
    <td>bounds of ellipse containing <a href='#Arc'>Arc</a></td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_startAngle'><code><strong>startAngle</strong></code></a></td>
    <td>starting angle of <a href='#Arc'>Arc</a> in degrees</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_sweepAngle'><code><strong>sweepAngle</strong></code></a></td>
    <td>sweep, in degrees. Positive is clockwise; treated modulo 360</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_forceMoveTo'><code><strong>forceMoveTo</strong></code></a></td>
    <td>true to start a new contour with <a href='#Arc'>Arc</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="5f02890edaa10cb5e1a4243a82b6a382"><div><a href='#SkPath_arcTo'>arcTo</a> continues a previous contour when <a href='#SkPath_arcTo_forceMoveTo'>forceMoveTo</a> is false and when <a href='#Path'>Path</a>
is not empty.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_addArc'>addArc</a> <a href='SkCanvas_Reference#SkCanvas_drawArc'>SkCanvas::drawArc</a> <a href='#SkPath_conicTo'>conicTo</a><sup><a href='#SkPath_conicTo_2'>[2]</a></sup>

---

<a name='SkPath_arcTo_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_arcTo'>arcTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> x1, <a href='undocumented#SkScalar'>SkScalar</a> y1, <a href='undocumented#SkScalar'>SkScalar</a> x2, <a href='undocumented#SkScalar'>SkScalar</a> y2, <a href='undocumented#SkScalar'>SkScalar</a> radius)
</pre>

Appends <a href='#Arc'>Arc</a> to <a href='#Path'>Path</a>, after appending <a href='undocumented#Line'>Line</a> if needed. <a href='#Arc'>Arc</a> is implemented by <a href='#Conic'>Conic</a>
weighted to describe part of <a href='undocumented#Circle'>Circle</a>. <a href='#Arc'>Arc</a> is contained by tangent from
last <a href='#Path'>Path</a> point (x0, y0) to (<a href='#SkPath_arcTo_2_x1'>x1</a>, <a href='#SkPath_arcTo_2_y1'>y1</a>), and tangent from (<a href='#SkPath_arcTo_2_x1'>x1</a>, <a href='#SkPath_arcTo_2_y1'>y1</a>) to (<a href='#SkPath_arcTo_2_x2'>x2</a>, <a href='#SkPath_arcTo_2_y2'>y2</a>). <a href='#Arc'>Arc</a>
is part of <a href='undocumented#Circle'>Circle</a> sized to <a href='#SkPath_arcTo_2_radius'>radius</a>, positioned so it touches both tangent lines.

### Example

<div><fiddle-embed name="d37540afd918506ac2594665ca63979b"></fiddle-embed></div>

If last <a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> does not start <a href='#Arc'>Arc</a>, <a href='#SkPath_arcTo'>arcTo</a> appends connecting <a href='undocumented#Line'>Line</a> to <a href='#Path'>Path</a>.
The length of <a href='SkPoint_Reference#Vector'>Vector</a> from (<a href='#SkPath_arcTo_2_x1'>x1</a>, <a href='#SkPath_arcTo_2_y1'>y1</a>) to (<a href='#SkPath_arcTo_2_x2'>x2</a>, <a href='#SkPath_arcTo_2_y2'>y2</a>) does not affect <a href='#Arc'>Arc</a>.

### Example

<div><fiddle-embed name="01d2ddfd539ab86a86989e210640dffc"></fiddle-embed></div>

<a href='#Arc'>Arc</a> sweep is always less than 180 degrees. If <a href='#SkPath_arcTo_2_radius'>radius</a> is zero, or if
tangents are nearly parallel, <a href='#SkPath_arcTo'>arcTo</a> appends <a href='undocumented#Line'>Line</a> from last <a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> to (<a href='#SkPath_arcTo_2_x1'>x1</a>, <a href='#SkPath_arcTo_2_y1'>y1</a>).

<a href='#SkPath_arcTo'>arcTo</a> appends at most one <a href='undocumented#Line'>Line</a> and one <a href='#Conic'>Conic</a>.
<a href='#SkPath_arcTo'>arcTo</a> implements the functionality of <a href='undocumented#Arct'>PostScript Arct</a> and <a href='undocumented#ArcTo'>HTML Canvas ArcTo</a>.

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
    <td>distance from <a href='#Arc'>Arc</a> to <a href='undocumented#Circle'>Circle</a> center</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="498360fa0a201cc5db04b1c27256358f"><div><a href='#SkPath_arcTo'>arcTo</a> is represented by <a href='undocumented#Line'>Line</a> and circular <a href='#Conic'>Conic</a> in <a href='#Path'>Path</a>.
</div>

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
void <a href='#SkPath_arcTo'>arcTo</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> p1, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> p2, <a href='undocumented#SkScalar'>SkScalar</a> radius)
</pre>

Appends <a href='#Arc'>Arc</a> to <a href='#Path'>Path</a>, after appending <a href='undocumented#Line'>Line</a> if needed. <a href='#Arc'>Arc</a> is implemented by <a href='#Conic'>Conic</a>
weighted to describe part of <a href='undocumented#Circle'>Circle</a>. <a href='#Arc'>Arc</a> is contained by tangent from
last <a href='#Path'>Path</a> point to <a href='#SkPath_arcTo_3_p1'>p1</a>, and tangent from <a href='#SkPath_arcTo_3_p1'>p1</a> to <a href='#SkPath_arcTo_3_p2'>p2</a>. <a href='#Arc'>Arc</a>
is part of <a href='undocumented#Circle'>Circle</a> sized to <a href='#SkPath_arcTo_3_radius'>radius</a>, positioned so it touches both tangent lines.

If last <a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> does not start <a href='#Arc'>Arc</a>, <a href='#SkPath_arcTo'>arcTo</a> appends connecting <a href='undocumented#Line'>Line</a> to <a href='#Path'>Path</a>.
The length of <a href='SkPoint_Reference#Vector'>Vector</a> from <a href='#SkPath_arcTo_3_p1'>p1</a> to <a href='#SkPath_arcTo_3_p2'>p2</a> does not affect <a href='#Arc'>Arc</a>.

<a href='#Arc'>Arc</a> sweep is always less than 180 degrees. If <a href='#SkPath_arcTo_3_radius'>radius</a> is zero, or if
tangents are nearly parallel, <a href='#SkPath_arcTo'>arcTo</a> appends <a href='undocumented#Line'>Line</a> from last <a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> to <a href='#SkPath_arcTo_3_p1'>p1</a>.

<a href='#SkPath_arcTo'>arcTo</a> appends at most one <a href='undocumented#Line'>Line</a> and one <a href='#Conic'>Conic</a>.
<a href='#SkPath_arcTo'>arcTo</a> implements the functionality of <a href='undocumented#Arct'>PostScript Arct</a> and <a href='undocumented#ArcTo'>HTML Canvas ArcTo</a>.

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

### Example

<div><fiddle-embed name="0c056264a361579c18e5d02d3172d4d4"><div>Because tangent lines are parallel, <a href='#SkPath_arcTo'>arcTo</a> appends line from last <a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> to
<a href='#SkPath_arcTo_3_p1'>p1</a>, but does not append a circular <a href='#Conic'>Conic</a>.
</div>

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
        <a href='#SkPath_kSmall_ArcSize'>kSmall ArcSize</a>,
        <a href='#SkPath_kLarge_ArcSize'>kLarge ArcSize</a>,
    };
</pre>

Four <a href='undocumented#Oval'>Oval</a> parts with radii (rx, ry) start at last <a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> and ends at (x, y).
<a href='#SkPath_ArcSize'>ArcSize</a> and <a href='#SkPath_Direction'>Direction</a> select one of the four <a href='undocumented#Oval'>Oval</a> parts.

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

<div><fiddle-embed name="8e40c546eecd9cc213200717240898ba"><div><a href='#Arc'>Arc</a> begins at top of <a href='undocumented#Oval'>Oval</a> pair and ends at bottom. <a href='#Arc'>Arc</a> can take four routes to get there.
Two routes are large, and two routes are counterclockwise. The one route both large
and counterclockwise is blue.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_arcTo'>arcTo</a><sup><a href='#SkPath_arcTo_2'>[2]</a></sup><sup><a href='#SkPath_arcTo_3'>[3]</a></sup><sup><a href='#SkPath_arcTo_4'>[4]</a></sup><sup><a href='#SkPath_arcTo_5'>[5]</a></sup> <a href='#SkPath_Direction'>Direction</a>

<a name='SkPath_arcTo_4'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_arcTo'>arcTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> rx, <a href='undocumented#SkScalar'>SkScalar</a> ry, <a href='undocumented#SkScalar'>SkScalar</a> xAxisRotate, <a href='#SkPath_ArcSize'>ArcSize</a> largeArc, <a href='#SkPath_Direction'>Direction</a> sweep,
           <a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y)
</pre>

Appends <a href='#Arc'>Arc</a> to <a href='#Path'>Path</a>. <a href='#Arc'>Arc</a> is implemented by one or more <a href='#Conic'>Conics</a> weighted to
describe part of <a href='undocumented#Oval'>Oval</a> with radii (<a href='#SkPath_arcTo_4_rx'>rx</a>, <a href='#SkPath_arcTo_4_ry'>ry</a>) rotated by <a href='#SkPath_arcTo_4_xAxisRotate'>xAxisRotate</a> degrees. <a href='#Arc'>Arc</a>
curves from last <a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> to (<a href='#SkPath_arcTo_4_x'>x</a>, <a href='#SkPath_arcTo_4_y'>y</a>), choosing one of four possible routes:
clockwise or counterclockwise, and smaller or larger.

<a href='#Arc'>Arc</a> <a href='#SkPath_arcTo_4_sweep'>sweep</a> is always less than 360 degrees. <a href='#SkPath_arcTo'>arcTo</a> appends <a href='undocumented#Line'>Line</a> to (<a href='#SkPath_arcTo_4_x'>x</a>, <a href='#SkPath_arcTo_4_y'>y</a>) if
either radii are zero, or if last <a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> equals (<a href='#SkPath_arcTo_4_x'>x</a>, <a href='#SkPath_arcTo_4_y'>y</a>). <a href='#SkPath_arcTo'>arcTo</a> scales radii
(<a href='#SkPath_arcTo_4_rx'>rx</a>, <a href='#SkPath_arcTo_4_ry'>ry</a>) to fit last <a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> and (<a href='#SkPath_arcTo_4_x'>x</a>, <a href='#SkPath_arcTo_4_y'>y</a>) if both are greater than zero but
too small.

<a href='#SkPath_arcTo'>arcTo</a> appends up to four <a href='#Conic'>Conic</a> curves.
<a href='#SkPath_arcTo'>arcTo</a> implements the functionality of <a href='undocumented#Arc'>SVG Arc</a>, although <a href='undocumented#SVG'>SVG</a> "<a href='#SkPath_arcTo_4_sweep'>sweep</a>-flag" value
is opposite the integer value of <a href='#SkPath_arcTo_4_sweep'>sweep</a>; <a href='undocumented#SVG'>SVG</a> "<a href='#SkPath_arcTo_4_sweep'>sweep</a>-flag" uses 1 for clockwise,
while <a href='#SkPath_kCW_Direction'>kCW Direction</a>  cast to int is zero.

### Parameters

<table>  <tr>    <td><a name='SkPath_arcTo_4_rx'><code><strong>rx</strong></code></a></td>
    <td>radius in <a href='#SkPath_arcTo_4_x'>x</a> before <a href='#SkPath_arcTo_4_x'>x</a>-axis rotation</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_4_ry'><code><strong>ry</strong></code></a></td>
    <td>radius in <a href='#SkPath_arcTo_4_y'>y</a> before <a href='#SkPath_arcTo_4_x'>x</a>-axis rotation</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_4_xAxisRotate'><code><strong>xAxisRotate</strong></code></a></td>
    <td><a href='#SkPath_arcTo_4_x'>x</a>-axis rotation in degrees; positive values are clockwise</td>
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

### Example

<div><fiddle-embed name="6b6ea44f659b27918f3a6fa621bf6173"></fiddle-embed></div>

### See Also

<a href='#SkPath_rArcTo'>rArcTo</a> <a href='#SkPath_ArcSize'>ArcSize</a> <a href='#SkPath_Direction'>Direction</a>

---

<a name='SkPath_arcTo_5'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_arcTo'>arcTo</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> r, <a href='undocumented#SkScalar'>SkScalar</a> xAxisRotate, <a href='#SkPath_ArcSize'>ArcSize</a> largeArc, <a href='#SkPath_Direction'>Direction</a> sweep, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> xy)
</pre>

Appends <a href='#Arc'>Arc</a> to <a href='#Path'>Path</a>. <a href='#Arc'>Arc</a> is implemented by one or more <a href='#Conic'>Conic</a> weighted to describe part of <a href='undocumented#Oval'>Oval</a>
with radii (<a href='#SkPath_arcTo_5_r'>r</a>.fX, <a href='#SkPath_arcTo_5_r'>r</a>.fY) rotated by <a href='#SkPath_arcTo_5_xAxisRotate'>xAxisRotate</a> degrees. <a href='#Arc'>Arc</a> curves from last <a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> to
(<a href='#SkPath_arcTo_5_xy'>xy</a>.fX, <a href='#SkPath_arcTo_5_xy'>xy</a>.fY), choosing one of four possible routes: clockwise or counterclockwise,
and smaller or larger.

<a href='#Arc'>Arc</a> <a href='#SkPath_arcTo_5_sweep'>sweep</a> is always less than 360 degrees. <a href='#SkPath_arcTo'>arcTo</a> appends <a href='undocumented#Line'>Line</a> to <a href='#SkPath_arcTo_5_xy'>xy</a> if either radii are zero,
or if last <a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> equals (x, y). <a href='#SkPath_arcTo'>arcTo</a> scales radii <a href='#SkPath_arcTo_5_r'>r</a> to fit last <a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> and
<a href='#SkPath_arcTo_5_xy'>xy</a> if both are greater than zero but too small to describe an arc.

<a href='#SkPath_arcTo'>arcTo</a> appends up to four <a href='#Conic'>Conic</a> curves.
<a href='#SkPath_arcTo'>arcTo</a> implements the functionality of <a href='undocumented#Arc'>SVG Arc</a>, although <a href='undocumented#SVG'>SVG</a> "<a href='#SkPath_arcTo_5_sweep'>sweep</a>-flag" value is
opposite the integer value of <a href='#SkPath_arcTo_5_sweep'>sweep</a>; <a href='undocumented#SVG'>SVG</a> "<a href='#SkPath_arcTo_5_sweep'>sweep</a>-flag" uses 1 for clockwise, while
<a href='#SkPath_kCW_Direction'>kCW Direction</a> cast to int is zero.

### Parameters

<table>  <tr>    <td><a name='SkPath_arcTo_5_r'><code><strong>r</strong></code></a></td>
    <td>radii on axes before x-axis rotation</td>
  </tr>
  <tr>    <td><a name='SkPath_arcTo_5_xAxisRotate'><code><strong>xAxisRotate</strong></code></a></td>
    <td>x-axis rotation in degrees; positive values are clockwise</td>
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

### Example

<div><fiddle-embed name="3f76a1007416181a4848c1a87fc81dbd"></fiddle-embed></div>

### See Also

<a href='#SkPath_rArcTo'>rArcTo</a> <a href='#SkPath_ArcSize'>ArcSize</a> <a href='#SkPath_Direction'>Direction</a>

---

<a name='SkPath_rArcTo'></a>
## rArcTo

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_rArcTo'>rArcTo</a>(<a href='undocumented#SkScalar'>SkScalar</a> rx, <a href='undocumented#SkScalar'>SkScalar</a> ry, <a href='undocumented#SkScalar'>SkScalar</a> xAxisRotate, <a href='#SkPath_ArcSize'>ArcSize</a> largeArc, <a href='#SkPath_Direction'>Direction</a> sweep,
            <a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy)
</pre>

Appends <a href='#Arc'>Arc</a> to <a href='#Path'>Path</a>, relative to last <a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a>. <a href='#Arc'>Arc</a> is implemented by one or
more <a href='#Conic'>Conic</a>, weighted to describe part of <a href='undocumented#Oval'>Oval</a> with radii (<a href='#SkPath_rArcTo_rx'>rx</a>, <a href='#SkPath_rArcTo_ry'>ry</a>) rotated by
<a href='#SkPath_rArcTo_xAxisRotate'>xAxisRotate</a> degrees. <a href='#Arc'>Arc</a> curves from last <a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> (x0, y0) to end <a href='SkPoint_Reference#Point'>Point</a>:

(x0 + <a href='#SkPath_rArcTo_dx'>dx</a>, y0 + <a href='#SkPath_rArcTo_dy'>dy</a>)
,
choosing one of four possible routes: clockwise or
counterclockwise, and smaller or larger. If <a href='#Path'>Path</a> is empty, the start <a href='#Arc'>Arc</a> <a href='SkPoint_Reference#Point'>Point</a>
is (0, 0).

<a href='#Arc'>Arc</a> <a href='#SkPath_rArcTo_sweep'>sweep</a> is always less than 360 degrees. <a href='#SkPath_arcTo'>arcTo</a> appends <a href='undocumented#Line'>Line</a> to end <a href='SkPoint_Reference#Point'>Point</a>
if either radii are zero, or if last <a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> equals end <a href='SkPoint_Reference#Point'>Point</a>.
<a href='#SkPath_arcTo'>arcTo</a> scales radii (<a href='#SkPath_rArcTo_rx'>rx</a>, <a href='#SkPath_rArcTo_ry'>ry</a>) to fit last <a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a> and end <a href='SkPoint_Reference#Point'>Point</a> if both are
greater than zero but too small to describe an arc.

<a href='#SkPath_arcTo'>arcTo</a> appends up to four <a href='#Conic'>Conic</a> curves.
<a href='#SkPath_arcTo'>arcTo</a> implements the functionality of <a href='undocumented#Arc'>SVG Arc</a>, although <a href='undocumented#SVG'>SVG</a> "<a href='#SkPath_rArcTo_sweep'>sweep</a>-flag" value is
opposite the integer value of <a href='#SkPath_rArcTo_sweep'>sweep</a>; <a href='undocumented#SVG'>SVG</a> "<a href='#SkPath_rArcTo_sweep'>sweep</a>-flag" uses 1 for clockwise, while
<a href='#SkPath_kCW_Direction'>kCW Direction</a> cast to int is zero.

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
    <td>chooses smaller or larger <a href='#Arc'>Arc</a></td>
  </tr>
  <tr>    <td><a name='SkPath_rArcTo_sweep'><code><strong>sweep</strong></code></a></td>
    <td>chooses clockwise or counterclockwise <a href='#Arc'>Arc</a></td>
  </tr>
  <tr>    <td><a name='SkPath_rArcTo_dx'><code><strong>dx</strong></code></a></td>
    <td>x-axis offset end of <a href='#Arc'>Arc</a> from last <a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a></td>
  </tr>
  <tr>    <td><a name='SkPath_rArcTo_dy'><code><strong>dy</strong></code></a></td>
    <td>y-axis offset end of <a href='#Arc'>Arc</a> from last <a href='#Path'>Path</a> <a href='SkPoint_Reference#Point'>Point</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3f76a1007416181a4848c1a87fc81dbd"></fiddle-embed></div>

### See Also

<a href='#SkPath_arcTo'>arcTo</a><sup><a href='#SkPath_arcTo_2'>[2]</a></sup><sup><a href='#SkPath_arcTo_3'>[3]</a></sup><sup><a href='#SkPath_arcTo_4'>[4]</a></sup><sup><a href='#SkPath_arcTo_5'>[5]</a></sup> <a href='#SkPath_ArcSize'>ArcSize</a> <a href='#SkPath_Direction'>Direction</a>

---

<a name='SkPath_close'></a>
## close

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_close'>close</a>()
</pre>

Appends <a href='#SkPath_kClose_Verb'>kClose Verb</a> to <a href='#Path'>Path</a>. A closed <a href='#Contour'>Contour</a> connects the first and last <a href='SkPoint_Reference#Point'>Point</a>
with <a href='undocumented#Line'>Line</a>, forming a continuous loop. Open and closed <a href='#Contour'>Contour</a> draw the same
with <a href='SkPaint_Reference#SkPaint_kFill_Style'>SkPaint::kFill Style</a>. With <a href='SkPaint_Reference#SkPaint_kStroke_Style'>SkPaint::kStroke Style</a>, open <a href='#Contour'>Contour</a> draws
<a href='SkPaint_Reference#Stroke_Cap'>Paint Stroke Cap</a> at <a href='#Contour'>Contour</a> start and end; closed <a href='#Contour'>Contour</a> draws
<a href='SkPaint_Reference#Stroke_Join'>Paint Stroke Join</a> at <a href='#Contour'>Contour</a> start and end.

<a href='#SkPath_close'>close</a> has no effect if <a href='#Path'>Path</a> is empty or last <a href='#Path'>Path</a> <a href='#SkPath_Verb'>Verb</a> is <a href='#SkPath_kClose_Verb'>kClose Verb</a>.

### Example

<div><fiddle-embed name="9235f6309271d6420fa5c45dc28664c5"></fiddle-embed></div>

### See Also

---

<a name='SkPath_IsInverseFillType'></a>
## IsInverseFillType

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
static bool <a href='#SkPath_IsInverseFillType'>IsInverseFillType</a>(<a href='#SkPath_FillType'>FillType</a> fill)
</pre>

Returns true if <a href='#SkPath_IsInverseFillType_fill'>fill</a> is inverted and <a href='#Path'>Path</a> with <a href='#SkPath_IsInverseFillType_fill'>fill</a> represents area outside
of its geometric bounds.

| <a href='#SkPath_FillType'>FillType</a> | is inverse |
| --- | ---  |
| <a href='#SkPath_kWinding_FillType'>kWinding FillType</a> | false |
| <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd FillType</a> | false |
| <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding FillType</a> | true |
| <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd FillType</a> | true |

### Parameters

<table>  <tr>    <td><a name='SkPath_IsInverseFillType_fill'><code><strong>fill</strong></code></a></td>
    <td>one of: <a href='#SkPath_kWinding_FillType'>kWinding FillType</a>, <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd FillType</a>,
<a href='#SkPath_kInverseWinding_FillType'>kInverseWinding FillType</a>, <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd FillType</a></td>
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
static <a href='#SkPath_FillType'>FillType</a> <a href='#SkPath_ConvertToNonInverseFillType'>ConvertToNonInverseFillType</a>(<a href='#SkPath_FillType'>FillType</a> fill)
</pre>

Returns equivalent <a href='#Fill_Type'>Fill Type</a> representing <a href='#Path'>Path</a> <a href='#SkPath_ConvertToNonInverseFillType_fill'>fill</a> inside its bounds.
.

| <a href='#SkPath_FillType'>FillType</a> | inside <a href='#SkPath_FillType'>FillType</a> |
| --- | ---  |
| <a href='#SkPath_kWinding_FillType'>kWinding FillType</a> | <a href='#SkPath_kWinding_FillType'>kWinding FillType</a> |
| <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd FillType</a> | <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd FillType</a> |
| <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding FillType</a> | <a href='#SkPath_kWinding_FillType'>kWinding FillType</a> |
| <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd FillType</a> | <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd FillType</a> |

### Parameters

<table>  <tr>    <td><a name='SkPath_ConvertToNonInverseFillType_fill'><code><strong>fill</strong></code></a></td>
    <td>one of: <a href='#SkPath_kWinding_FillType'>kWinding FillType</a>, <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd FillType</a>,
<a href='#SkPath_kInverseWinding_FillType'>kInverseWinding FillType</a>, <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd FillType</a></td>
  </tr>
</table>

### Return Value

<a href='#SkPath_ConvertToNonInverseFillType_fill'>fill</a>, or <a href='#SkPath_kWinding_FillType'>kWinding FillType</a> or <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd FillType</a> if <a href='#SkPath_ConvertToNonInverseFillType_fill'>fill</a> is inverted

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
static int <a href='#SkPath_ConvertConicToQuads'>ConvertConicToQuads</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p0, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p1, const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p2, <a href='undocumented#SkScalar'>SkScalar</a> w,
                               <a href='SkPoint_Reference#SkPoint'>SkPoint</a> pts[], int pow2)
</pre>

Approximates <a href='#Conic'>Conic</a> with <a href='#Quad'>Quad</a> array. <a href='#Conic'>Conic</a> is constructed from start <a href='SkPoint_Reference#Point'>Point</a> <a href='#SkPath_ConvertConicToQuads_p0'>p0</a>,
control <a href='SkPoint_Reference#Point'>Point</a> <a href='#SkPath_ConvertConicToQuads_p1'>p1</a>, end <a href='SkPoint_Reference#Point'>Point</a> <a href='#SkPath_ConvertConicToQuads_p2'>p2</a>, and weight <a href='#SkPath_ConvertConicToQuads_w'>w</a>.
<a href='#Quad'>Quad</a> array is stored in <a href='#SkPath_ConvertConicToQuads_pts'>pts</a>; this storage is supplied by caller.
Maximum <a href='#Quad'>Quad</a> count is 2 to the <a href='#SkPath_ConvertConicToQuads_pow2'>pow2</a>.
Every third point in array shares last <a href='SkPoint_Reference#Point'>Point</a> of previous <a href='#Quad'>Quad</a> and first <a href='SkPoint_Reference#Point'>Point</a> of
next <a href='#Quad'>Quad</a>. Maximum <a href='#SkPath_ConvertConicToQuads_pts'>pts</a> storage size is given by:
(1 + 2 * (1 << <a href='#SkPath_ConvertConicToQuads_pow2'>pow2</a>)) * sizeof(SkPoint).

Returns <a href='#Quad'>Quad</a> count used the approximation, which may be smaller
than the number requested.

<a href='#Conic_Weight'>Conic Weight</a> determines the amount of influence <a href='#Conic'>Conic</a> control point has on the curve.
<a href='#SkPath_ConvertConicToQuads_w'>w</a> less than one represents an elliptical section. <a href='#SkPath_ConvertConicToQuads_w'>w</a> greater than one represents
a hyperbolic section. <a href='#SkPath_ConvertConicToQuads_w'>w</a> equal to one represents a parabolic section.

Two <a href='#Quad'>Quad</a> curves are sufficient to approximate an elliptical <a href='#Conic'>Conic</a> with a sweep
of up to 90 degrees; in this case, set <a href='#SkPath_ConvertConicToQuads_pow2'>pow2</a> to one.

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
    <td><a href='#Quad'>Quad</a> count, as power of two, normally 0 to 5 (1 to 32 <a href='#Quad'>Quad</a> curves)</td>
  </tr>
</table>

### Return Value

number of <a href='#Quad'>Quad</a> curves written to <a href='#SkPath_ConvertConicToQuads_pts'>pts</a>

### Example

<div><fiddle-embed name="3ba94448a4ba48f926e643baeb5b1016"><div>A pair of <a href='#Quad'>Quad</a> curves are drawn in red on top of the elliptical <a href='#Conic'>Conic</a> curve in black.
The middle curve is nearly circular. The top-right curve is parabolic, which can
be drawn exactly with a single <a href='#Quad'>Quad</a>.
</div></fiddle-embed></div>

### See Also

<a href='#Conic'>Conic</a> <a href='#Quad'>Quad</a>

---

<a name='SkPath_isRect'></a>
## isRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isRect'>isRect</a>(<a href='SkRect_Reference#SkRect'>SkRect</a>* rect, bool* isClosed = nullptr, <a href='#SkPath_Direction'>Direction</a>* direction = nullptr) const
</pre>

Returns true if <a href='#Path'>Path</a> is equivalent to <a href='SkRect_Reference#Rect'>Rect</a> when filled.
If false: <a href='#SkPath_isRect_rect'>rect</a>, <a href='#SkPath_isRect_isClosed'>isClosed</a>, and <a href='#SkPath_isRect_direction'>direction</a> are unchanged.
If true: <a href='#SkPath_isRect_rect'>rect</a>, <a href='#SkPath_isRect_isClosed'>isClosed</a>, and <a href='#SkPath_isRect_direction'>direction</a> are written to if not nullptr.

<a href='#SkPath_isRect_rect'>rect</a> may be smaller than the <a href='#Path'>Path</a> bounds. <a href='#Path'>Path</a> bounds may include <a href='#SkPath_kMove_Verb'>kMove Verb</a> points
that do not alter the area drawn by the returned <a href='#SkPath_isRect_rect'>rect</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_isRect_rect'><code><strong>rect</strong></code></a></td>
    <td>storage for bounds of <a href='SkRect_Reference#Rect'>Rect</a>; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkPath_isRect_isClosed'><code><strong>isClosed</strong></code></a></td>
    <td>storage set to true if <a href='#Path'>Path</a> is closed; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkPath_isRect_direction'><code><strong>direction</strong></code></a></td>
    <td>storage set to <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkPath_isRect_direction'>direction</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

true if <a href='#Path'>Path</a> contains <a href='SkRect_Reference#Rect'>Rect</a>

### Example

<div><fiddle-embed name="81a2aac1b8f0ff3d4c8d35ccb9149b16"><div>After <a href='#SkPath_addRect'>addRect</a>, <a href='#SkPath_isRect'>isRect</a> returns true. Following <a href='#SkPath_moveTo'>moveTo</a> permits <a href='#SkPath_isRect'>isRect</a> to return true, but
following <a href='#SkPath_lineTo'>lineTo</a> does not. <a href='#SkPath_addPoly'>addPoly</a> returns true even though <a href='#SkPath_isRect_rect'>rect</a> is not closed, and one
side of <a href='#SkPath_isRect_rect'>rect</a> is made up of consecutive line segments.
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

---

<a name='SkPath_isNestedFillRects'></a>
## isNestedFillRects

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isNestedFillRects'>isNestedFillRects</a>(<a href='SkRect_Reference#SkRect'>SkRect</a> rect[2], <a href='#SkPath_Direction'>Direction</a> dirs[2] = nullptr) const
</pre>

Returns true if <a href='#Path'>Path</a> is equivalent to nested <a href='SkRect_Reference#Rect'>Rect</a> pair when filled.
If false, <a href='#SkPath_isNestedFillRects_rect'>rect</a> and <a href='#SkPath_isNestedFillRects_dirs'>dirs</a> are unchanged.
If true, <a href='#SkPath_isNestedFillRects_rect'>rect</a> and <a href='#SkPath_isNestedFillRects_dirs'>dirs</a> are written to if not nullptr:
setting <a href='#SkPath_isNestedFillRects_rect'>rect</a>[0] to outer <a href='SkRect_Reference#Rect'>Rect</a>, and <a href='#SkPath_isNestedFillRects_rect'>rect</a>[1] to inner <a href='SkRect_Reference#Rect'>Rect</a>;
setting <a href='#SkPath_isNestedFillRects_dirs'>dirs</a>[0] to <a href='#SkPath_Direction'>Direction</a> of outer <a href='SkRect_Reference#Rect'>Rect</a>, and <a href='#SkPath_isNestedFillRects_dirs'>dirs</a>[1] to <a href='#SkPath_Direction'>Direction</a> of inner
<a href='SkRect_Reference#Rect'>Rect</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_isNestedFillRects_rect'><code><strong>rect</strong></code></a></td>
    <td>storage for <a href='SkRect_Reference#Rect'>Rect</a> pair; may be nullptr</td>
  </tr>
  <tr>    <td><a name='SkPath_isNestedFillRects_dirs'><code><strong>dirs</strong></code></a></td>
    <td>storage for <a href='#SkPath_Direction'>Direction</a> pair; may be nullptr</td>
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
void <a href='#SkPath_addRect'>addRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& rect, <a href='#SkPath_Direction'>Direction</a> dir = <a href='#SkPath_kCW_Direction'>kCW Direction</a>)
</pre>

Adds <a href='SkRect_Reference#Rect'>Rect</a> to <a href='#Path'>Path</a>, appending <a href='#SkPath_kMove_Verb'>kMove Verb</a>, three <a href='#SkPath_kLine_Verb'>kLine Verb</a>, and <a href='#SkPath_kClose_Verb'>kClose Verb</a>,
starting with top-left corner of <a href='SkRect_Reference#Rect'>Rect</a>; followed by top-right, bottom-right,
and bottom-left if <a href='#SkPath_addRect_dir'>dir</a> is <a href='#SkPath_kCW_Direction'>kCW Direction</a>; or followed by bottom-left,
bottom-right, and top-right if <a href='#SkPath_addRect_dir'>dir</a> is <a href='#SkPath_kCCW_Direction'>kCCW Direction</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_addRect_rect'><code><strong>rect</strong></code></a></td>
    <td><a href='SkRect_Reference#Rect'>Rect</a> to add as a closed contour</td>
  </tr>
  <tr>    <td><a name='SkPath_addRect_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='#SkPath_Direction'>Direction</a> to wind added contour</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="0f841e4eaebb613b5069800567917c2d"><div>The left <a href='SkRect_Reference#Rect'>Rect</a> dashes starting at the top-left corner, to the right.
The right <a href='SkRect_Reference#Rect'>Rect</a> dashes starting at the top-left corner, towards the bottom.
</div></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas_drawRect'>SkCanvas::drawRect</a> <a href='#SkPath_Direction'>Direction</a>

---

<a name='SkPath_addRect_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_addRect'>addRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& rect, <a href='#SkPath_Direction'>Direction</a> dir, unsigned start)
</pre>

Adds <a href='SkRect_Reference#Rect'>Rect</a> to <a href='#Path'>Path</a>, appending <a href='#SkPath_kMove_Verb'>kMove Verb</a>, three <a href='#SkPath_kLine_Verb'>kLine Verb</a>, and <a href='#SkPath_kClose_Verb'>kClose Verb</a>.
If <a href='#SkPath_addRect_2_dir'>dir</a> is <a href='#SkPath_kCW_Direction'>kCW Direction</a>, <a href='SkRect_Reference#Rect'>Rect</a> corners are added clockwise; if <a href='#SkPath_addRect_2_dir'>dir</a> is
<a href='#SkPath_kCCW_Direction'>kCCW Direction</a>, <a href='SkRect_Reference#Rect'>Rect</a> corners are added counterclockwise.
<a href='#SkPath_addRect_2_start'>start</a> determines the first corner added.

| <a href='#SkPath_addRect_2_start'>start</a> | first corner |
| --- | ---  |
| 0 | top-left |
| 1 | top-right |
| 2 | bottom-right |
| 3 | bottom-left |

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

### Example

<div><fiddle-embed name="9202430b3f4f5275af8eec5cc9d7baa8"><div>The arrow is just after the initial corner and points towards the next
corner appended to <a href='#Path'>Path</a>.
</div></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas_drawRect'>SkCanvas::drawRect</a> <a href='#SkPath_Direction'>Direction</a>

---

<a name='SkPath_addRect_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_addRect'>addRect</a>(<a href='undocumented#SkScalar'>SkScalar</a> left, <a href='undocumented#SkScalar'>SkScalar</a> top, <a href='undocumented#SkScalar'>SkScalar</a> right, <a href='undocumented#SkScalar'>SkScalar</a> bottom,
             <a href='#SkPath_Direction'>Direction</a> dir = <a href='#SkPath_kCW_Direction'>kCW Direction</a>)
</pre>

Adds <a href='SkRect_Reference#Rect'>Rect</a> (<a href='#SkPath_addRect_3_left'>left</a>, <a href='#SkPath_addRect_3_top'>top</a>, <a href='#SkPath_addRect_3_right'>right</a>, <a href='#SkPath_addRect_3_bottom'>bottom</a>) to <a href='#Path'>Path</a>,
appending <a href='#SkPath_kMove_Verb'>kMove Verb</a>, three <a href='#SkPath_kLine_Verb'>kLine Verb</a>, and <a href='#SkPath_kClose_Verb'>kClose Verb</a>,
starting with <a href='#SkPath_addRect_3_top'>top</a>-<a href='#SkPath_addRect_3_left'>left</a> corner of <a href='SkRect_Reference#Rect'>Rect</a>; followed by <a href='#SkPath_addRect_3_top'>top</a>-<a href='#SkPath_addRect_3_right'>right</a>, <a href='#SkPath_addRect_3_bottom'>bottom</a>-<a href='#SkPath_addRect_3_right'>right</a>,
and <a href='#SkPath_addRect_3_bottom'>bottom</a>-<a href='#SkPath_addRect_3_left'>left</a> if <a href='#SkPath_addRect_3_dir'>dir</a> is <a href='#SkPath_kCW_Direction'>kCW Direction</a>; or followed by <a href='#SkPath_addRect_3_bottom'>bottom</a>-<a href='#SkPath_addRect_3_left'>left</a>,
<a href='#SkPath_addRect_3_bottom'>bottom</a>-<a href='#SkPath_addRect_3_right'>right</a>, and <a href='#SkPath_addRect_3_top'>top</a>-<a href='#SkPath_addRect_3_right'>right</a> if <a href='#SkPath_addRect_3_dir'>dir</a> is <a href='#SkPath_kCCW_Direction'>kCCW Direction</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_addRect_3_left'><code><strong>left</strong></code></a></td>
    <td>smaller x-axis value of <a href='SkRect_Reference#Rect'>Rect</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addRect_3_top'><code><strong>top</strong></code></a></td>
    <td>smaller y-axis value of <a href='SkRect_Reference#Rect'>Rect</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addRect_3_right'><code><strong>right</strong></code></a></td>
    <td>larger x-axis value of <a href='SkRect_Reference#Rect'>Rect</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addRect_3_bottom'><code><strong>bottom</strong></code></a></td>
    <td>larger y-axis value of <a href='SkRect_Reference#Rect'>Rect</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addRect_3_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='#SkPath_Direction'>Direction</a> to wind added contour</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3837827310e8b88b8c2e128ef9fbbd65"><div>The <a href='#SkPath_addRect_3_left'>left</a> <a href='SkRect_Reference#Rect'>Rect</a> dashes start at the <a href='#SkPath_addRect_3_top'>top</a>-<a href='#SkPath_addRect_3_left'>left</a> corner, and continue to the <a href='#SkPath_addRect_3_right'>right</a>.
The <a href='#SkPath_addRect_3_right'>right</a> <a href='SkRect_Reference#Rect'>Rect</a> dashes start at the <a href='#SkPath_addRect_3_top'>top</a>-<a href='#SkPath_addRect_3_left'>left</a> corner, and continue down.
</div></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas_drawRect'>SkCanvas::drawRect</a> <a href='#SkPath_Direction'>Direction</a>

---

<a name='SkPath_addOval'></a>
## addOval

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_addOval'>addOval</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& oval, <a href='#SkPath_Direction'>Direction</a> dir = <a href='#SkPath_kCW_Direction'>kCW Direction</a>)
</pre>

Adds <a href='undocumented#Oval'>Oval</a> to path, appending <a href='#SkPath_kMove_Verb'>kMove Verb</a>, four <a href='#SkPath_kConic_Verb'>kConic Verb</a>, and <a href='#SkPath_kClose_Verb'>kClose Verb</a>.
<a href='undocumented#Oval'>Oval</a> is upright ellipse bounded by <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkPath_addOval_oval'>oval</a> with radii equal to half <a href='#SkPath_addOval_oval'>oval</a> width
and half <a href='#SkPath_addOval_oval'>oval</a> height. <a href='undocumented#Oval'>Oval</a> begins at (<a href='#SkPath_addOval_oval'>oval</a>.fRight, <a href='#SkPath_addOval_oval'>oval</a>.centerY()) and continues
clockwise if <a href='#SkPath_addOval_dir'>dir</a> is <a href='#SkPath_kCW_Direction'>kCW Direction</a>, counterclockwise if <a href='#SkPath_addOval_dir'>dir</a> is <a href='#SkPath_kCCW_Direction'>kCCW Direction</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_addOval_oval'><code><strong>oval</strong></code></a></td>
    <td>bounds of ellipse added</td>
  </tr>
  <tr>    <td><a name='SkPath_addOval_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='#SkPath_Direction'>Direction</a> to wind ellipse</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="cac84cf68e63a453c2a8b64c91537704"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas_drawOval'>SkCanvas::drawOval</a> <a href='#SkPath_Direction'>Direction</a> <a href='undocumented#Oval'>Oval</a>

---

<a name='SkPath_addOval_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_addOval'>addOval</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& oval, <a href='#SkPath_Direction'>Direction</a> dir, unsigned start)
</pre>

Adds <a href='undocumented#Oval'>Oval</a> to <a href='#Path'>Path</a>, appending <a href='#SkPath_kMove_Verb'>kMove Verb</a>, four <a href='#SkPath_kConic_Verb'>kConic Verb</a>, and <a href='#SkPath_kClose_Verb'>kClose Verb</a>.
<a href='undocumented#Oval'>Oval</a> is upright ellipse bounded by <a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkPath_addOval_2_oval'>oval</a> with radii equal to half <a href='#SkPath_addOval_2_oval'>oval</a> width
and half <a href='#SkPath_addOval_2_oval'>oval</a> height. <a href='undocumented#Oval'>Oval</a> begins at <a href='#SkPath_addOval_2_start'>start</a> and continues
clockwise if <a href='#SkPath_addOval_2_dir'>dir</a> is <a href='#SkPath_kCW_Direction'>kCW Direction</a>, counterclockwise if <a href='#SkPath_addOval_2_dir'>dir</a> is <a href='#SkPath_kCCW_Direction'>kCCW Direction</a>.

| <a href='#SkPath_addOval_2_start'>start</a> | <a href='SkPoint_Reference#Point'>Point</a> |
| --- | ---  |
| 0 | <a href='#SkPath_addOval_2_oval'>oval</a>.centerX(), <a href='#SkPath_addOval_2_oval'>oval</a>.fTop |
| 1 | <a href='#SkPath_addOval_2_oval'>oval</a>.fRight, <a href='#SkPath_addOval_2_oval'>oval</a>.centerY() |
| 2 | <a href='#SkPath_addOval_2_oval'>oval</a>.centerX(), <a href='#SkPath_addOval_2_oval'>oval</a>.fBottom |
| 3 | <a href='#SkPath_addOval_2_oval'>oval</a>.fLeft, <a href='#SkPath_addOval_2_oval'>oval</a>.centerY() |

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

### Example

<div><fiddle-embed name="ab9753174060e4a551727ef3af12924d"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas_drawOval'>SkCanvas::drawOval</a> <a href='#SkPath_Direction'>Direction</a> <a href='undocumented#Oval'>Oval</a>

---

<a name='SkPath_addCircle'></a>
## addCircle

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_addCircle'>addCircle</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y, <a href='undocumented#SkScalar'>SkScalar</a> radius, <a href='#SkPath_Direction'>Direction</a> dir = <a href='#SkPath_kCW_Direction'>kCW Direction</a>)
</pre>

Adds <a href='undocumented#Circle'>Circle</a> centered at (<a href='#SkPath_addCircle_x'>x</a>, <a href='#SkPath_addCircle_y'>y</a>) of size <a href='#SkPath_addCircle_radius'>radius</a> to <a href='#Path'>Path</a>, appending <a href='#SkPath_kMove_Verb'>kMove Verb</a>,
four <a href='#SkPath_kConic_Verb'>kConic Verb</a>, and <a href='#SkPath_kClose_Verb'>kClose Verb</a>. <a href='undocumented#Circle'>Circle</a> begins at:
(<a href='#SkPath_addCircle_x'>x</a> + <a href='#SkPath_addCircle_radius'>radius</a>, <a href='#SkPath_addCircle_y'>y</a>)
,
continuing
clockwise if <a href='#SkPath_addCircle_dir'>dir</a> is <a href='#SkPath_kCW_Direction'>kCW Direction</a>, and counterclockwise if <a href='#SkPath_addCircle_dir'>dir</a> is <a href='#SkPath_kCCW_Direction'>kCCW Direction</a>.

Has no effect if <a href='#SkPath_addCircle_radius'>radius</a> is zero or negative.

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

### Example

<div><fiddle-embed name="bd5286cb9a5e5c32cd980f72b8f400fb"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas_drawCircle'>SkCanvas::drawCircle</a><sup><a href='SkCanvas_Reference#SkCanvas_drawCircle_2'>[2]</a></sup> <a href='#SkPath_Direction'>Direction</a> <a href='undocumented#Circle'>Circle</a>

---

<a name='SkPath_addArc'></a>
## addArc

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_addArc'>addArc</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& oval, <a href='undocumented#SkScalar'>SkScalar</a> startAngle, <a href='undocumented#SkScalar'>SkScalar</a> sweepAngle)
</pre>

Appends <a href='#Arc'>Arc</a> to <a href='#Path'>Path</a>, as the start of new <a href='#Contour'>Contour</a>. <a href='#Arc'>Arc</a> added is part of ellipse
bounded by <a href='#SkPath_addArc_oval'>oval</a>, from <a href='#SkPath_addArc_startAngle'>startAngle</a> through <a href='#SkPath_addArc_sweepAngle'>sweepAngle</a>. Both <a href='#SkPath_addArc_startAngle'>startAngle</a> and
<a href='#SkPath_addArc_sweepAngle'>sweepAngle</a> are measured in degrees, where zero degrees is aligned with the
positive x-axis, and positive sweeps extends <a href='#Arc'>Arc</a> clockwise.

If <a href='#SkPath_addArc_sweepAngle'>sweepAngle</a> <= -360, or <a href='#SkPath_addArc_sweepAngle'>sweepAngle</a> >= 360; and <a href='#SkPath_addArc_startAngle'>startAngle</a> modulo 90 is nearly
zero, append <a href='undocumented#Oval'>Oval</a> instead of <a href='#Arc'>Arc</a>. Otherwise, <a href='#SkPath_addArc_sweepAngle'>sweepAngle</a> values are treated
modulo 360, and <a href='#Arc'>Arc</a> may or may not draw depending on numeric rounding.

### Parameters

<table>  <tr>    <td><a name='SkPath_addArc_oval'><code><strong>oval</strong></code></a></td>
    <td>bounds of ellipse containing <a href='#Arc'>Arc</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addArc_startAngle'><code><strong>startAngle</strong></code></a></td>
    <td>starting angle of <a href='#Arc'>Arc</a> in degrees</td>
  </tr>
  <tr>    <td><a name='SkPath_addArc_sweepAngle'><code><strong>sweepAngle</strong></code></a></td>
    <td>sweep, in degrees. Positive is clockwise; treated modulo 360</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="9cf5122475624e4cf39f06c698f80b1a"><div>The middle row of the left and right columns draw differently from the entries
above and below because <a href='#SkPath_addArc_sweepAngle'>sweepAngle</a> is outside of the range of +/-360,
and <a href='#SkPath_addArc_startAngle'>startAngle</a> modulo 90 is not zero.
</div></fiddle-embed></div>

### See Also

<a href='#Arc'>Arc</a> <a href='#SkPath_arcTo'>arcTo</a><sup><a href='#SkPath_arcTo_2'>[2]</a></sup><sup><a href='#SkPath_arcTo_3'>[3]</a></sup><sup><a href='#SkPath_arcTo_4'>[4]</a></sup><sup><a href='#SkPath_arcTo_5'>[5]</a></sup> <a href='SkCanvas_Reference#SkCanvas_drawArc'>SkCanvas::drawArc</a>

---

<a name='SkPath_addRoundRect'></a>
## addRoundRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_addRoundRect'>addRoundRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& rect, <a href='undocumented#SkScalar'>SkScalar</a> rx, <a href='undocumented#SkScalar'>SkScalar</a> ry, <a href='#SkPath_Direction'>Direction</a> dir = <a href='#SkPath_kCW_Direction'>kCW Direction</a>)
</pre>

Appends <a href='SkRRect_Reference#RRect'>Round Rect</a> to <a href='#Path'>Path</a>, creating a new closed <a href='#Contour'>Contour</a>. <a href='SkRRect_Reference#RRect'>Round Rect</a> has bounds
equal to <a href='#SkPath_addRoundRect_rect'>rect</a>; each corner is 90 degrees of an ellipse with radii (<a href='#SkPath_addRoundRect_rx'>rx</a>, <a href='#SkPath_addRoundRect_ry'>ry</a>). If
<a href='#SkPath_addRoundRect_dir'>dir</a> is <a href='#SkPath_kCW_Direction'>kCW Direction</a>, <a href='SkRRect_Reference#RRect'>Round Rect</a> starts at top-left of the lower-left corner and
winds clockwise. If <a href='#SkPath_addRoundRect_dir'>dir</a> is <a href='#SkPath_kCCW_Direction'>kCCW Direction</a>, <a href='SkRRect_Reference#RRect'>Round Rect</a> starts at the bottom-left
of the upper-left corner and winds counterclockwise.

If either <a href='#SkPath_addRoundRect_rx'>rx</a> or <a href='#SkPath_addRoundRect_ry'>ry</a> is too large, <a href='#SkPath_addRoundRect_rx'>rx</a> and <a href='#SkPath_addRoundRect_ry'>ry</a> are scaled uniformly until the
corners fit. If <a href='#SkPath_addRoundRect_rx'>rx</a> or <a href='#SkPath_addRoundRect_ry'>ry</a> is less than or equal to zero, <a href='#SkPath_addRoundRect'>addRoundRect</a> appends
<a href='SkRect_Reference#Rect'>Rect</a> <a href='#SkPath_addRoundRect_rect'>rect</a> to <a href='#Path'>Path</a>.

After appending, <a href='#Path'>Path</a> may be empty, or may contain: <a href='SkRect_Reference#Rect'>Rect</a>, <a href='undocumented#Oval'>Oval</a>, or RoundRect.

### Parameters

<table>  <tr>    <td><a name='SkPath_addRoundRect_rect'><code><strong>rect</strong></code></a></td>
    <td>bounds of <a href='SkRRect_Reference#RRect'>Round Rect</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addRoundRect_rx'><code><strong>rx</strong></code></a></td>
    <td>x-axis radius of rounded corners on the <a href='SkRRect_Reference#RRect'>Round Rect</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addRoundRect_ry'><code><strong>ry</strong></code></a></td>
    <td>y-axis radius of rounded corners on the <a href='SkRRect_Reference#RRect'>Round Rect</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addRoundRect_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='#SkPath_Direction'>Direction</a> to wind <a href='SkRRect_Reference#RRect'>Round Rect</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="24736f685f265cf533f1700c042db353"><div>If either radius is zero, path contains <a href='SkRect_Reference#Rect'>Rect</a> and is drawn red.
If sides are only radii, path contains <a href='undocumented#Oval'>Oval</a> and is drawn blue.
All remaining path draws are convex, and are drawn in gray; no
paths constructed from <a href='#SkPath_addRoundRect'>addRoundRect</a> are concave, so none are
drawn in green.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_addRRect'>addRRect</a><sup><a href='#SkPath_addRRect_2'>[2]</a></sup> <a href='SkCanvas_Reference#SkCanvas_drawRoundRect'>SkCanvas::drawRoundRect</a>

---

<a name='SkPath_addRoundRect_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_addRoundRect'>addRoundRect</a>(const <a href='SkRect_Reference#SkRect'>SkRect</a>& rect, const <a href='undocumented#SkScalar'>SkScalar</a> radii[], <a href='#SkPath_Direction'>Direction</a> dir = <a href='#SkPath_kCW_Direction'>kCW Direction</a>)
</pre>

Appends <a href='SkRRect_Reference#RRect'>Round Rect</a> to <a href='#Path'>Path</a>, creating a new closed <a href='#Contour'>Contour</a>. <a href='SkRRect_Reference#RRect'>Round Rect</a> has bounds
equal to <a href='#SkPath_addRoundRect_2_rect'>rect</a>; each corner is 90 degrees of an ellipse with <a href='#SkPath_addRoundRect_2_radii'>radii</a> from the
array.

| <a href='#SkPath_addRoundRect_2_radii'>radii</a> index | location |
| --- | ---  |
| 0 | x-axis radius of top-left corner |
| 1 | y-axis radius of top-left corner |
| 2 | x-axis radius of top-right corner |
| 3 | y-axis radius of top-right corner |
| 4 | x-axis radius of bottom-right corner |
| 5 | y-axis radius of bottom-right corner |
| 6 | x-axis radius of bottom-left corner |
| 7 | y-axis radius of bottom-left corner |

If <a href='#SkPath_addRoundRect_2_dir'>dir</a> is <a href='#SkPath_kCW_Direction'>kCW Direction</a>, <a href='SkRRect_Reference#RRect'>Round Rect</a> starts at top-left of the lower-left corner
and winds clockwise. If <a href='#SkPath_addRoundRect_2_dir'>dir</a> is <a href='#SkPath_kCCW_Direction'>kCCW Direction</a>, <a href='SkRRect_Reference#RRect'>Round Rect</a> starts at the
bottom-left of the upper-left corner and winds counterclockwise.

If both <a href='#SkPath_addRoundRect_2_radii'>radii</a> on any side of <a href='#SkPath_addRoundRect_2_rect'>rect</a> exceed its length, all <a href='#SkPath_addRoundRect_2_radii'>radii</a> are scaled
uniformly until the corners fit. If either radius of a corner is less than or
equal to zero, both are treated as zero.

After appending, <a href='#Path'>Path</a> may be empty, or may contain: <a href='SkRect_Reference#Rect'>Rect</a>, <a href='undocumented#Oval'>Oval</a>, or RoundRect.

### Parameters

<table>  <tr>    <td><a name='SkPath_addRoundRect_2_rect'><code><strong>rect</strong></code></a></td>
    <td>bounds of <a href='SkRRect_Reference#RRect'>Round Rect</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addRoundRect_2_radii'><code><strong>radii</strong></code></a></td>
    <td>array of 8 <a href='undocumented#SkScalar'>SkScalar</a> values, a radius pair for each corner</td>
  </tr>
  <tr>    <td><a name='SkPath_addRoundRect_2_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='#SkPath_Direction'>Direction</a> to wind <a href='SkRRect_Reference#RRect'>Round Rect</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c43d70606b4ee464d2befbcf448c5e73"></fiddle-embed></div>

### See Also

<a href='#SkPath_addRRect'>addRRect</a><sup><a href='#SkPath_addRRect_2'>[2]</a></sup> <a href='SkCanvas_Reference#SkCanvas_drawRoundRect'>SkCanvas::drawRoundRect</a>

---

<a name='SkPath_addRRect'></a>
## addRRect

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_addRRect'>addRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& rrect, <a href='#SkPath_Direction'>Direction</a> dir = <a href='#SkPath_kCW_Direction'>kCW Direction</a>)
</pre>

Adds <a href='#SkPath_addRRect_rrect'>rrect</a> to <a href='#Path'>Path</a>, creating a new closed <a href='#Contour'>Contour</a>. If
<a href='#SkPath_addRRect_dir'>dir</a> is <a href='#SkPath_kCW_Direction'>kCW Direction</a>, <a href='#SkPath_addRRect_rrect'>rrect</a> starts at top-left of the lower-left corner and
winds clockwise. If <a href='#SkPath_addRRect_dir'>dir</a> is <a href='#SkPath_kCCW_Direction'>kCCW Direction</a>, <a href='#SkPath_addRRect_rrect'>rrect</a> starts at the bottom-left
of the upper-left corner and winds counterclockwise.

After appending, <a href='#Path'>Path</a> may be empty, or may contain: <a href='SkRect_Reference#Rect'>Rect</a>, <a href='undocumented#Oval'>Oval</a>, or <a href='SkRRect_Reference#RRect'>Round Rect</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_addRRect_rrect'><code><strong>rrect</strong></code></a></td>
    <td>bounds and radii of rounded rectangle</td>
  </tr>
  <tr>    <td><a name='SkPath_addRRect_dir'><code><strong>dir</strong></code></a></td>
    <td><a href='#SkPath_Direction'>Direction</a> to wind <a href='SkRRect_Reference#RRect'>Round Rect</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="d9ecd58081b5bc77a157636fcb345dc6"></fiddle-embed></div>

### See Also

<a href='#SkPath_addRoundRect'>addRoundRect</a><sup><a href='#SkPath_addRoundRect_2'>[2]</a></sup> <a href='SkCanvas_Reference#SkCanvas_drawRRect'>SkCanvas::drawRRect</a>

---

<a name='SkPath_addRRect_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_addRRect'>addRRect</a>(const <a href='SkRRect_Reference#SkRRect'>SkRRect</a>& rrect, <a href='#SkPath_Direction'>Direction</a> dir, unsigned start)
</pre>

Adds <a href='#SkPath_addRRect_2_rrect'>rrect</a> to <a href='#Path'>Path</a>, creating a new closed <a href='#Contour'>Contour</a>. If <a href='#SkPath_addRRect_2_dir'>dir</a> is <a href='#SkPath_kCW_Direction'>kCW Direction</a>, <a href='#SkPath_addRRect_2_rrect'>rrect</a>
winds clockwise; if <a href='#SkPath_addRRect_2_dir'>dir</a> is <a href='#SkPath_kCCW_Direction'>kCCW Direction</a>, <a href='#SkPath_addRRect_2_rrect'>rrect</a> winds counterclockwise.
<a href='#SkPath_addRRect_2_start'>start</a> determines the first point of <a href='#SkPath_addRRect_2_rrect'>rrect</a> to add.

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

After appending, <a href='#Path'>Path</a> may be empty, or may contain: <a href='SkRect_Reference#Rect'>Rect</a>, <a href='undocumented#Oval'>Oval</a>, or <a href='SkRRect_Reference#RRect'>Round Rect</a>.

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

### Example

<div><fiddle-embed name="f18740ffcb10a499007488948c2cd60d"></fiddle-embed></div>

### See Also

<a href='#SkPath_addRoundRect'>addRoundRect</a><sup><a href='#SkPath_addRoundRect_2'>[2]</a></sup> <a href='SkCanvas_Reference#SkCanvas_drawRRect'>SkCanvas::drawRRect</a>

---

<a name='SkPath_addPoly'></a>
## addPoly

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_addPoly'>addPoly</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a> pts[], int count, bool close)
</pre>

Adds <a href='#Contour'>Contour</a> created from <a href='undocumented#Line'>Line</a> array, adding (<a href='#SkPath_addPoly_count'>count</a> - 1) <a href='undocumented#Line'>Line</a> segments.
<a href='#Contour'>Contour</a> added starts at <a href='#SkPath_addPoly_pts'>pts</a>[0], then adds a line for every additional <a href='SkPoint_Reference#Point'>Point</a>
in <a href='#SkPath_addPoly_pts'>pts</a> array. If close is true,appends <a href='#SkPath_kClose_Verb'>kClose Verb</a> to <a href='#Path'>Path</a>, connecting
<a href='#SkPath_addPoly_pts'>pts</a>[<a href='#SkPath_addPoly_count'>count</a> - 1] and <a href='#SkPath_addPoly_pts'>pts</a>[0].

If <a href='#SkPath_addPoly_count'>count</a> is zero, append <a href='#SkPath_kMove_Verb'>kMove Verb</a> to path.
Has no effect if <a href='#SkPath_addPoly_count'>count</a> is less than one.

### Parameters

<table>  <tr>    <td><a name='SkPath_addPoly_pts'><code><strong>pts</strong></code></a></td>
    <td>array of <a href='undocumented#Line'>Line</a> sharing end and start <a href='SkPoint_Reference#Point'>Point</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addPoly_count'><code><strong>count</strong></code></a></td>
    <td>length of <a href='SkPoint_Reference#Point'>Point</a> array</td>
  </tr>
  <tr>    <td><a name='SkPath_addPoly_close'><code><strong>close</strong></code></a></td>
    <td>true to add <a href='undocumented#Line'>Line</a> connecting <a href='#Contour'>Contour</a> end and start</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="182b3999772f330f3b0b891b492634ae"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas_drawPoints'>SkCanvas::drawPoints</a>

---

## <a name='SkPath_AddPathMode'>Enum SkPath::AddPathMode</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    enum <a href='#SkPath_AddPathMode'>AddPathMode</a> {
        <a href='#SkPath_kAppend_AddPathMode'>kAppend AddPathMode</a>,
        <a href='#SkPath_kExtend_AddPathMode'>kExtend AddPathMode</a>,
    };
</pre>

<a href='#SkPath_AddPathMode'>AddPathMode</a> chooses how <a href='#SkPath_addPath'>addPath</a> appends. Adding one <a href='#Path'>Path</a> to another can extend
the last <a href='#Contour'>Contour</a> or start a new <a href='#Contour'>Contour</a>.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kAppend_AddPathMode'><code>SkPath::kAppend_AddPathMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#Line # appended to destination unaltered ##</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
<a href='#Path'>Path</a> <a href='#Verb'>Verbs</a>, <a href='SkPoint_Reference#Point'>Points</a>, and <a href='#Conic_Weight'>Conic Weights</a> are appended to destination unaltered.
Since <a href='#Path'>Path</a> <a href='#Verb_Array'>Verb Array</a> begins with <a href='#SkPath_kMove_Verb'>kMove Verb</a> if src is not empty, this
starts a new <a href='#Contour'>Contour</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kExtend_AddPathMode'><code>SkPath::kExtend_AddPathMode</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>#Line # add line if prior Contour is not closed ##</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
If destination is closed or empty, start a new <a href='#Contour'>Contour</a>. If destination
is not empty, add <a href='undocumented#Line'>Line</a> from <a href='#Last_Point'>Last Point</a> to added <a href='#Path'>Path</a> first <a href='SkPoint_Reference#Point'>Point</a>. Skip added
<a href='#Path'>Path</a> initial <a href='#SkPath_kMove_Verb'>kMove Verb</a>, then append remining <a href='#Verb'>Verbs</a>, <a href='SkPoint_Reference#Point'>Points</a>, and <a href='#Conic_Weight'>Conic Weights</a>.
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="801b02e74c64aafdb734f2e5cf3e5ab0"><div>test is built from path, open on the top row, and closed on the bottom row.
The left column uses <a href='#SkPath_kAppend_AddPathMode'>kAppend AddPathMode</a>; the right uses <a href='#SkPath_kExtend_AddPathMode'>kExtend AddPathMode</a>.
The top right composition is made up of one contour; the other three have two.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_addPath'>addPath</a><sup><a href='#SkPath_addPath_2'>[2]</a></sup><sup><a href='#SkPath_addPath_3'>[3]</a></sup> <a href='#SkPath_reverseAddPath'>reverseAddPath</a>

<a name='SkPath_addPath'></a>
## addPath

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_addPath'>addPath</a>(const <a href='#SkPath'>SkPath</a>& src, <a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy, <a href='#SkPath_AddPathMode'>AddPathMode</a> mode = <a href='#SkPath_kAppend_AddPathMode'>kAppend AddPathMode</a>)
</pre>

Appends <a href='#SkPath_addPath_src'>src</a> to <a href='#Path'>Path</a>, offset by (<a href='#SkPath_addPath_dx'>dx</a>, <a href='#SkPath_addPath_dy'>dy</a>).

If <a href='#SkPath_addPath_mode'>mode</a> is <a href='#SkPath_kAppend_AddPathMode'>kAppend AddPathMode</a>, <a href='#SkPath_addPath_src'>src</a> <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, and <a href='#Conic_Weight'>Conic Weights</a> are
added unaltered. If <a href='#SkPath_addPath_mode'>mode</a> is <a href='#SkPath_kExtend_AddPathMode'>kExtend AddPathMode</a>, add <a href='undocumented#Line'>Line</a> before appending
<a href='#Verb'>Verbs</a>, <a href='SkPoint_Reference#Point'>Points</a>, and <a href='#Conic_Weight'>Conic Weights</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_addPath_src'><code><strong>src</strong></code></a></td>
    <td><a href='#Path'>Path</a> <a href='#Verb'>Verbs</a>, <a href='SkPoint_Reference#Point'>Points</a>, and <a href='#Conic_Weight'>Conic Weights</a> to add</td>
  </tr>
  <tr>    <td><a name='SkPath_addPath_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#SkPath_addPath_src'>src</a> <a href='#Point_Array'>Point Array</a> x-axis coordinates</td>
  </tr>
  <tr>    <td><a name='SkPath_addPath_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#SkPath_addPath_src'>src</a> <a href='#Point_Array'>Point Array</a> y-axis coordinates</td>
  </tr>
  <tr>    <td><a name='SkPath_addPath_mode'><code><strong>mode</strong></code></a></td>
    <td><a href='#SkPath_kAppend_AddPathMode'>kAppend AddPathMode</a> or <a href='#SkPath_kExtend_AddPathMode'>kExtend AddPathMode</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c416bddfe286628974e1c7f0fd66f3f4"></fiddle-embed></div>

### See Also

<a href='#SkPath_AddPathMode'>AddPathMode</a> <a href='#SkPath_offset'>offset</a><sup><a href='#SkPath_offset_2'>[2]</a></sup> <a href='#SkPath_reverseAddPath'>reverseAddPath</a>

---

<a name='SkPath_addPath_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_addPath'>addPath</a>(const <a href='#SkPath'>SkPath</a>& src, <a href='#SkPath_AddPathMode'>AddPathMode</a> mode = <a href='#SkPath_kAppend_AddPathMode'>kAppend AddPathMode</a>)
</pre>

Appends <a href='#SkPath_addPath_2_src'>src</a> to <a href='#Path'>Path</a>.

If <a href='#SkPath_addPath_2_mode'>mode</a> is <a href='#SkPath_kAppend_AddPathMode'>kAppend AddPathMode</a>, <a href='#SkPath_addPath_2_src'>src</a> <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, and <a href='#Conic_Weight'>Conic Weights</a> are
added unaltered. If <a href='#SkPath_addPath_2_mode'>mode</a> is <a href='#SkPath_kExtend_AddPathMode'>kExtend AddPathMode</a>, add <a href='undocumented#Line'>Line</a> before appending
<a href='#Verb'>Verbs</a>, <a href='SkPoint_Reference#Point'>Points</a>, and <a href='#Conic_Weight'>Conic Weights</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_addPath_2_src'><code><strong>src</strong></code></a></td>
    <td><a href='#Path'>Path</a> <a href='#Verb'>Verbs</a>, <a href='SkPoint_Reference#Point'>Points</a>, and <a href='#Conic_Weight'>Conic Weights</a> to add</td>
  </tr>
  <tr>    <td><a name='SkPath_addPath_2_mode'><code><strong>mode</strong></code></a></td>
    <td><a href='#SkPath_kAppend_AddPathMode'>kAppend AddPathMode</a> or <a href='#SkPath_kExtend_AddPathMode'>kExtend AddPathMode</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="84b2d1c0fc29f1b35e855b6fc6672f9e"></fiddle-embed></div>

### See Also

<a href='#SkPath_AddPathMode'>AddPathMode</a> <a href='#SkPath_reverseAddPath'>reverseAddPath</a>

---

<a name='SkPath_addPath_3'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_addPath'>addPath</a>(const <a href='#SkPath'>SkPath</a>& src, const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& matrix, <a href='#SkPath_AddPathMode'>AddPathMode</a> mode = <a href='#SkPath_kAppend_AddPathMode'>kAppend AddPathMode</a>)
</pre>

Appends <a href='#SkPath_addPath_3_src'>src</a> to <a href='#Path'>Path</a>, transformed by <a href='#SkPath_addPath_3_matrix'>matrix</a>. Transformed curves may have different
<a href='#Verb'>Verbs</a>, <a href='SkPoint_Reference#Point'>Points</a>, and <a href='#Conic_Weight'>Conic Weights</a>.

If <a href='#SkPath_addPath_3_mode'>mode</a> is <a href='#SkPath_kAppend_AddPathMode'>kAppend AddPathMode</a>, <a href='#SkPath_addPath_3_src'>src</a> <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, and <a href='#Conic_Weight'>Conic Weights</a> are
added unaltered. If <a href='#SkPath_addPath_3_mode'>mode</a> is <a href='#SkPath_kExtend_AddPathMode'>kExtend AddPathMode</a>, add <a href='undocumented#Line'>Line</a> before appending
<a href='#Verb'>Verbs</a>, <a href='SkPoint_Reference#Point'>Points</a>, and <a href='#Conic_Weight'>Conic Weights</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_addPath_3_src'><code><strong>src</strong></code></a></td>
    <td><a href='#Path'>Path</a> <a href='#Verb'>Verbs</a>, <a href='SkPoint_Reference#Point'>Points</a>, and <a href='#Conic_Weight'>Conic Weights</a> to add</td>
  </tr>
  <tr>    <td><a name='SkPath_addPath_3_matrix'><code><strong>matrix</strong></code></a></td>
    <td>transform applied to <a href='#SkPath_addPath_3_src'>src</a></td>
  </tr>
  <tr>    <td><a name='SkPath_addPath_3_mode'><code><strong>mode</strong></code></a></td>
    <td><a href='#SkPath_kAppend_AddPathMode'>kAppend AddPathMode</a> or <a href='#SkPath_kExtend_AddPathMode'>kExtend AddPathMode</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="3a90a91030f7289d5df0671d342dbbad"></fiddle-embed></div>

### See Also

<a href='#SkPath_AddPathMode'>AddPathMode</a> <a href='#SkPath_transform'>transform</a><sup><a href='#SkPath_transform_2'>[2]</a></sup> <a href='#SkPath_offset'>offset</a><sup><a href='#SkPath_offset_2'>[2]</a></sup> <a href='#SkPath_reverseAddPath'>reverseAddPath</a>

---

<a name='SkPath_reverseAddPath'></a>
## reverseAddPath

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_reverseAddPath'>reverseAddPath</a>(const <a href='#SkPath'>SkPath</a>& src)
</pre>

Appends <a href='#SkPath_reverseAddPath_src'>src</a> to <a href='#Path'>Path</a>, from back to front.
Reversed <a href='#SkPath_reverseAddPath_src'>src</a> always appends a new <a href='#Contour'>Contour</a> to <a href='#Path'>Path</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_reverseAddPath_src'><code><strong>src</strong></code></a></td>
    <td><a href='#Path'>Path</a> <a href='#Verb'>Verbs</a>, <a href='SkPoint_Reference#Point'>Points</a>, and <a href='#Conic_Weight'>Conic Weights</a> to add</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="5e8513f073db09acde3ff616f6426e3d"></fiddle-embed></div>

### See Also

<a href='#SkPath_AddPathMode'>AddPathMode</a> <a href='#SkPath_transform'>transform</a><sup><a href='#SkPath_transform_2'>[2]</a></sup> <a href='#SkPath_offset'>offset</a><sup><a href='#SkPath_offset_2'>[2]</a></sup> <a href='#SkPath_addPath'>addPath</a><sup><a href='#SkPath_addPath_2'>[2]</a></sup><sup><a href='#SkPath_addPath_3'>[3]</a></sup>

---

<a name='SkPath_offset'></a>
## offset

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_offset'>offset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy, <a href='#SkPath'>SkPath</a>* dst) const
</pre>

Offsets <a href='#Point_Array'>Point Array</a> by (<a href='#SkPath_offset_dx'>dx</a>, <a href='#SkPath_offset_dy'>dy</a>). Offset <a href='#Path'>Path</a> replaces <a href='#SkPath_offset_dst'>dst</a>.
If <a href='#SkPath_offset_dst'>dst</a> is nullptr, <a href='#Path'>Path</a> is replaced by offset data.

### Parameters

<table>  <tr>    <td><a name='SkPath_offset_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#Point_Array'>Point Array</a> x-axis coordinates</td>
  </tr>
  <tr>    <td><a name='SkPath_offset_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#Point_Array'>Point Array</a> y-axis coordinates</td>
  </tr>
  <tr>    <td><a name='SkPath_offset_dst'><code><strong>dst</strong></code></a></td>
    <td>overwritten, translated copy of <a href='#Path'>Path</a>; may be nullptr</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="1d1892196ba5bda257df4f3351abd084"></fiddle-embed></div>

### See Also

<a href='#SkPath_addPath'>addPath</a><sup><a href='#SkPath_addPath_2'>[2]</a></sup><sup><a href='#SkPath_addPath_3'>[3]</a></sup> <a href='#SkPath_transform'>transform</a><sup><a href='#SkPath_transform_2'>[2]</a></sup>

---

## <a name='Transform'>Transform</a>


<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_offset'>offset</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>translates <a href='#Point_Array'>Point Array</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_offset'>offset(SkScalar dx, SkScalar dy, SkPath* dst)</a> const</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_offset_2'>offset(SkScalar dx, SkScalar dy)</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_transform'>transform</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>applies <a href='SkMatrix_Reference#Matrix'>Matrix</a> to <a href='#Point_Array'>Point Array</a> and <a href='#Conic_Weight'>Weights</a></td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_transform'>transform(const SkMatrix& matrix, SkPath* dst)</a> const</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkPath_transform_2'>transform(const SkMatrix& matrix)</a></td>
  </tr>
</table>

<a name='SkPath_offset_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_offset'>offset</a>(<a href='undocumented#SkScalar'>SkScalar</a> dx, <a href='undocumented#SkScalar'>SkScalar</a> dy)
</pre>

Offsets <a href='#Point_Array'>Point Array</a> by (<a href='#SkPath_offset_2_dx'>dx</a>, <a href='#SkPath_offset_2_dy'>dy</a>). <a href='#Path'>Path</a> is replaced by offset data.

### Parameters

<table>  <tr>    <td><a name='SkPath_offset_2_dx'><code><strong>dx</strong></code></a></td>
    <td>offset added to <a href='#Point_Array'>Point Array</a> x-axis coordinates</td>
  </tr>
  <tr>    <td><a name='SkPath_offset_2_dy'><code><strong>dy</strong></code></a></td>
    <td>offset added to <a href='#Point_Array'>Point Array</a> y-axis coordinates</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="5188d77585715db30bef228f2dfbcccd"></fiddle-embed></div>

### See Also

<a href='#SkPath_addPath'>addPath</a><sup><a href='#SkPath_addPath_2'>[2]</a></sup><sup><a href='#SkPath_addPath_3'>[3]</a></sup> <a href='#SkPath_transform'>transform</a><sup><a href='#SkPath_transform_2'>[2]</a></sup> <a href='SkCanvas_Reference#SkCanvas_translate'>SkCanvas::translate()</a>

---

<a name='SkPath_transform'></a>
## transform

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_transform'>transform</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& matrix, <a href='#SkPath'>SkPath</a>* dst) const
</pre>

Transforms <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, and weight by <a href='#SkPath_transform_matrix'>matrix</a>.
transform may change <a href='#Verb'>Verbs</a> and increase their number.
Transformed <a href='#Path'>Path</a> replaces <a href='#SkPath_transform_dst'>dst</a>; if <a href='#SkPath_transform_dst'>dst</a> is nullptr, original data
is replaced.

### Parameters

<table>  <tr>    <td><a name='SkPath_transform_matrix'><code><strong>matrix</strong></code></a></td>
    <td><a href='SkMatrix_Reference#Matrix'>Matrix</a> to apply to <a href='#Path'>Path</a></td>
  </tr>
  <tr>    <td><a name='SkPath_transform_dst'><code><strong>dst</strong></code></a></td>
    <td>overwritten, transformed copy of <a href='#Path'>Path</a>; may be nullptr</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="99761add116ce3b0730557224c1b0105"></fiddle-embed></div>

### See Also

<a href='#SkPath_addPath'>addPath</a><sup><a href='#SkPath_addPath_2'>[2]</a></sup><sup><a href='#SkPath_addPath_3'>[3]</a></sup> <a href='#SkPath_offset'>offset</a><sup><a href='#SkPath_offset_2'>[2]</a></sup> <a href='SkCanvas_Reference#SkCanvas_concat'>SkCanvas::concat()</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>

---

<a name='SkPath_transform_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_transform'>transform</a>(const <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>& matrix)
</pre>

Transforms <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, and weight by <a href='#SkPath_transform_2_matrix'>matrix</a>.
transform may change <a href='#Verb'>Verbs</a> and increase their number.
<a href='#Path'>Path</a> is replaced by transformed data.

### Parameters

<table>  <tr>    <td><a name='SkPath_transform_2_matrix'><code><strong>matrix</strong></code></a></td>
    <td><a href='SkMatrix_Reference#Matrix'>Matrix</a> to apply to <a href='#Path'>Path</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="c40979a3b92a30cfb7bae36abcc1d805"></fiddle-embed></div>

### See Also

<a href='#SkPath_addPath'>addPath</a><sup><a href='#SkPath_addPath_2'>[2]</a></sup><sup><a href='#SkPath_addPath_3'>[3]</a></sup> <a href='#SkPath_offset'>offset</a><sup><a href='#SkPath_offset_2'>[2]</a></sup> <a href='SkCanvas_Reference#SkCanvas_concat'>SkCanvas::concat()</a> <a href='SkMatrix_Reference#SkMatrix'>SkMatrix</a>

---

## <a name='Last_Point'>Last Point</a>

<a href='#Path'>Path</a> is defined cumulatively, often by adding a segment to the end of last
<a href='#Contour'>Contour</a>. <a href='#Last_Point'>Last Point</a> of <a href='#Contour'>Contour</a> is shared as first <a href='SkPoint_Reference#Point'>Point</a> of added <a href='undocumented#Line'>Line</a> or <a href='undocumented#Curve'>Curve</a>.
<a href='#Last_Point'>Last Point</a> can be read and written directly with <a href='#SkPath_getLastPt'>getLastPt</a> and <a href='#SkPath_setLastPt'>setLastPt</a>.

<a name='SkPath_getLastPt'></a>
## getLastPt

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_getLastPt'>getLastPt</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a>* lastPt) const
</pre>

Returns <a href='#Last_Point'>Last Point</a> on <a href='#Path'>Path</a> in <a href='#SkPath_getLastPt_lastPt'>lastPt</a>. Returns false if <a href='#Point_Array'>Point Array</a> is empty,
storing (0, 0) if <a href='#SkPath_getLastPt_lastPt'>lastPt</a> is not nullptr.

### Parameters

<table>  <tr>    <td><a name='SkPath_getLastPt_lastPt'><code><strong>lastPt</strong></code></a></td>
    <td>storage for final <a href='SkPoint_Reference#Point'>Point</a> in <a href='#Point_Array'>Point Array</a>; may be nullptr</td>
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
void <a href='#SkPath_setLastPt'>setLastPt</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y)
</pre>

Sets <a href='#Last_Point'>Last Point</a> to (<a href='#SkPath_setLastPt_x'>x</a>, <a href='#SkPath_setLastPt_y'>y</a>). If <a href='#Point_Array'>Point Array</a> is empty, append <a href='#SkPath_kMove_Verb'>kMove Verb</a> to
<a href='#Verb_Array'>Verb Array</a> and append (<a href='#SkPath_setLastPt_x'>x</a>, <a href='#SkPath_setLastPt_y'>y</a>) to <a href='#Point_Array'>Point Array</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_setLastPt_x'><code><strong>x</strong></code></a></td>
    <td>set <a href='#SkPath_setLastPt_x'>x</a>-axis value of <a href='#Last_Point'>Last Point</a></td>
  </tr>
  <tr>    <td><a name='SkPath_setLastPt_y'><code><strong>y</strong></code></a></td>
    <td>set <a href='#SkPath_setLastPt_y'>y</a>-axis value of <a href='#Last_Point'>Last Point</a></td>
  </tr>
</table>

### Example

<div><fiddle-embed name="542c5afaea5f57baa11d0561dd402e18"></fiddle-embed></div>

### See Also

<a href='#SkPath_getLastPt'>getLastPt</a>

---

<a name='SkPath_setLastPt_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_setLastPt'>setLastPt</a>(const <a href='SkPoint_Reference#SkPoint'>SkPoint</a>& p)
</pre>

Sets the last point on the path. If <a href='#Point_Array'>Point Array</a> is empty, append <a href='#SkPath_kMove_Verb'>kMove Verb</a> to
<a href='#Verb_Array'>Verb Array</a> and append <a href='#SkPath_setLastPt_2_p'>p</a> to <a href='#Point_Array'>Point Array</a>.

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
        <a href='#SkPath_kLine_SegmentMask'>kLine SegmentMask</a> = 1 << 0,
        <a href='#SkPath_kQuad_SegmentMask'>kQuad SegmentMask</a> = 1 << 1,
        <a href='#SkPath_kConic_SegmentMask'>kConic SegmentMask</a> = 1 << 2,
        <a href='#SkPath_kCubic_SegmentMask'>kCubic SegmentMask</a> = 1 << 3,
    };
</pre>

<a href='#SkPath_SegmentMask'>SegmentMask</a> constants correspond to each drawing <a href='#SkPath_Verb'>Verb</a> type in <a href='#Path'>Path</a>; for
instance, if <a href='#Path'>Path</a> only contains <a href='undocumented#Line'>Lines</a>, only the <a href='#SkPath_kLine_SegmentMask'>kLine SegmentMask</a> bit is set.

### Constants

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Const</th>
<th style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>Value</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kLine_SegmentMask'><code>SkPath::kLine_SegmentMask</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>1</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Set if <a href='#Verb_Array'>Verb Array</a> contains <a href='#SkPath_kLine_Verb'>kLine Verb</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kQuad_SegmentMask'><code>SkPath::kQuad_SegmentMask</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>2</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Set if <a href='#Verb_Array'>Verb Array</a> contains <a href='#SkPath_kQuad_Verb'>kQuad Verb</a>. Note that <a href='#SkPath_conicTo'>conicTo</a> may add a <a href='#Quad'>Quad</a>.
</td>
  </tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kConic_SegmentMask'><code>SkPath::kConic_SegmentMask</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>4</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Set if <a href='#Verb_Array'>Verb Array</a> contains <a href='#SkPath_kConic_Verb'>kConic Verb</a>.
</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a name='SkPath_kCubic_SegmentMask'><code>SkPath::kCubic_SegmentMask</code></a></td>
    <td style='text-align: center; border: 2px solid #dddddd; padding: 8px; '>8</td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>
Set if <a href='#Verb_Array'>Verb Array</a> contains <a href='#SkPath_kCubic_Verb'>kCubic Verb</a>.
</td>
  </tr>
</table>

### Example

<div><fiddle-embed name="a61e5758574e28190ec4ed8c4ae7e7fa"><div>When <a href='#SkPath_conicTo'>conicTo</a> has a weight of one, <a href='#Quad'>Quad</a> is added to <a href='#Path'>Path</a>.
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
## getSegmentMasks

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkPath_getSegmentMasks'>getSegmentMasks</a>() const
</pre>

Returns a mask, where each set bit corresponds to a <a href='#SkPath_SegmentMask'>SegmentMask</a> constant
if <a href='#Path'>Path</a> contains one or more <a href='#Verb'>Verbs</a> of that type.
Returns zero if <a href='#Path'>Path</a> contains no <a href='undocumented#Line'>Lines</a>, or <a href='undocumented#Curve'>Curves</a>: <a href='#Quad'>Quads</a>, <a href='#Conic'>Conics</a>, or <a href='#Cubic'>Cubics</a>.

<a href='#SkPath_getSegmentMasks'>getSegmentMasks</a> returns a cached result; it is very fast.

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
bool <a href='#SkPath_contains'>contains</a>(<a href='undocumented#SkScalar'>SkScalar</a> x, <a href='undocumented#SkScalar'>SkScalar</a> y) const
</pre>

Returns true if the point (<a href='#SkPath_contains_x'>x</a>, <a href='#SkPath_contains_y'>y</a>) is contained by <a href='#Path'>Path</a>, taking into
account <a href='#SkPath_FillType'>FillType</a>.

| <a href='#SkPath_FillType'>FillType</a> | <a href='#SkPath_contains'>contains</a> returns true if <a href='SkPoint_Reference#Point'>Point</a> is enclosed by |
| --- | ---  |
| <a href='#SkPath_kWinding_FillType'>kWinding FillType</a> | a non-zero sum of <a href='#Contour'>Contour</a> <a href='#Direction'>Directions</a>. |
| <a href='#SkPath_kEvenOdd_FillType'>kEvenOdd FillType</a> | an odd number of <a href='#Contour'>Contours</a>. |
| <a href='#SkPath_kInverseWinding_FillType'>kInverseWinding FillType</a> | a zero sum of <a href='#Contour'>Contour</a> <a href='#Direction'>Directions</a>. |
| <a href='#SkPath_kInverseEvenOdd_FillType'>kInverseEvenOdd FillType</a> | and even number of <a href='#Contour'>Contours</a>. |

### Parameters

<table>  <tr>    <td><a name='SkPath_contains_x'><code><strong>x</strong></code></a></td>
    <td><a href='#SkPath_contains_x'>x</a>-axis value of containment test</td>
  </tr>
  <tr>    <td><a name='SkPath_contains_y'><code><strong>y</strong></code></a></td>
    <td><a href='#SkPath_contains_y'>y</a>-axis value of containment test</td>
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
void <a href='#SkPath_dump'>dump</a>(<a href='undocumented#SkWStream'>SkWStream</a>* stream, bool forceClose, bool dumpAsHex) const
</pre>

Writes text representation of <a href='#Path'>Path</a> to <a href='#SkPath_dump_stream'>stream</a>. If <a href='#SkPath_dump_stream'>stream</a> is nullptr, writes to
standard output. Set <a href='#SkPath_dump_forceClose'>forceClose</a> to true to get edges used to fill <a href='#Path'>Path</a>.
Set <a href='#SkPath_dump_dumpAsHex'>dumpAsHex</a> true to generate exact binary representations
of floating point numbers used in <a href='#Point_Array'>Point Array</a> and <a href='#Conic_Weight'>Conic Weights</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_dump_stream'><code><strong>stream</strong></code></a></td>
    <td>writable <a href='undocumented#WStream'>WStream</a> receiving <a href='#Path'>Path</a> text representation; may be nullptr</td>
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

<a href='#SkPath_dumpHex'>dumpHex</a> <a href='SkRect_Reference#SkRect_dump'>SkRect::dump()</a><sup><a href='SkRect_Reference#SkRect_dump_2'>[2]</a></sup> <a href='SkRRect_Reference#SkRRect_dump'>SkRRect::dump()</a><sup><a href='SkRRect_Reference#SkRRect_dump_2'>[2]</a></sup> <a href='undocumented#SkPathMeasure_dump'>SkPathMeasure::dump()</a>

---

<a name='SkPath_dump_2'></a>

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_dump'>dump</a>() const
</pre>

Writes text representation of <a href='#Path'>Path</a> to standard output. The representation may be
directly compiled as C++ code. Floating point values are written
with limited precision; it may not be possible to reconstruct original <a href='#Path'>Path</a>
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

<a href='#SkPath_dumpHex'>dumpHex</a> <a href='SkRect_Reference#SkRect_dump'>SkRect::dump()</a><sup><a href='SkRect_Reference#SkRect_dump_2'>[2]</a></sup> <a href='SkRRect_Reference#SkRRect_dump'>SkRRect::dump()</a><sup><a href='SkRRect_Reference#SkRRect_dump_2'>[2]</a></sup> <a href='undocumented#SkPathMeasure_dump'>SkPathMeasure::dump()</a> <a href='#SkPath_writeToMemory'>writeToMemory</a>

---

<a name='SkPath_dumpHex'></a>
## dumpHex

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_dumpHex'>dumpHex</a>() const
</pre>

Writes text representation of <a href='#Path'>Path</a> to standard output. The representation may be
directly compiled as C++ code. Floating point values are written
in hexadecimal to preserve their exact bit pattern. The output reconstructs the
original <a href='#Path'>Path</a>.

Use instead of <a href='#SkPath_dump_2'>dump</a> when submitting <a href='https://bug.skia.org'>bug reports against Skia</a></a> .

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
size_t <a href='#SkPath_writeToMemory'>writeToMemory</a>(void* buffer) const
</pre>

Writes <a href='#Path'>Path</a> to <a href='#SkPath_writeToMemory_buffer'>buffer</a>, returning the number of bytes written.
Pass nullptr to obtain the storage size.

Writes <a href='#Fill_Type'>Fill Type</a>, <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, <a href='#Conic_Weight'>Conic Weight</a>, and
additionally writes computed information like <a href='#SkPath_Convexity'>Convexity</a> and bounds.

Use only be used in concert with <a href='#SkPath_readFromMemory'>readFromMemory</a>;
the format used for <a href='#Path'>Path</a> in memory is not guaranteed.

### Parameters

<table>  <tr>    <td><a name='SkPath_writeToMemory_buffer'><code><strong>buffer</strong></code></a></td>
    <td>storage for <a href='#Path'>Path</a>; may be nullptr</td>
  </tr>
</table>

### Return Value

size of storage required for <a href='#Path'>Path</a>; always a multiple of 4

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
<a href='undocumented#sk_sp'>sk sp</a>&lt;<a href='undocumented#SkData'>SkData</a>&gt; <a href='#SkPath_serialize'>serialize</a>() const
</pre>

Writes <a href='#Path'>Path</a> to buffer, returning the buffer written to, wrapped in <a href='undocumented#Data'>Data</a>.

<a href='#SkPath_serialize'>serialize</a> writes <a href='#Fill_Type'>Fill Type</a>, <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, <a href='#Conic_Weight'>Conic Weight</a>, and
additionally writes computed information like <a href='#SkPath_Convexity'>Convexity</a> and bounds.

<a href='#SkPath_serialize'>serialize</a> should only be used in concert with <a href='#SkPath_readFromMemory'>readFromMemory</a>.
The format used for <a href='#Path'>Path</a> in memory is not guaranteed.

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
size_t <a href='#SkPath_readFromMemory'>readFromMemory</a>(const void* buffer, size_t length)
</pre>

Initializes <a href='#Path'>Path</a> from <a href='#SkPath_readFromMemory_buffer'>buffer</a> of size <a href='#SkPath_readFromMemory_length'>length</a>. Returns zero if the <a href='#SkPath_readFromMemory_buffer'>buffer</a> is
data is inconsistent, or the <a href='#SkPath_readFromMemory_length'>length</a> is too small.

Reads <a href='#Fill_Type'>Fill Type</a>, <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, <a href='#Conic_Weight'>Conic Weight</a>, and
additionally reads computed information like <a href='#SkPath_Convexity'>Convexity</a> and bounds.

Used only in concert with <a href='#SkPath_writeToMemory'>writeToMemory</a>;
the format used for <a href='#Path'>Path</a> in memory is not guaranteed.

### Parameters

<table>  <tr>    <td><a name='SkPath_readFromMemory_buffer'><code><strong>buffer</strong></code></a></td>
    <td>storage for <a href='#Path'>Path</a></td>
  </tr>
  <tr>    <td><a name='SkPath_readFromMemory_length'><code><strong>length</strong></code></a></td>
    <td><a href='#SkPath_readFromMemory_buffer'>buffer</a> size in bytes; must be multiple of 4</td>
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

---

## <a name='Generation_ID'>Generation ID</a>

<a href='#Generation_ID'>Generation ID</a> provides a quick way to check if <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, or
<a href='#Conic_Weight'>Conic Weight</a> has changed. <a href='#Generation_ID'>Generation ID</a> is not a hash; identical <a href='#Path'>Paths</a> will
not necessarily have matching <a href='#Generation_ID'>Generation IDs</a>.

Empty <a href='#Path'>Paths</a> have a <a href='#Generation_ID'>Generation ID</a> of one.

<a name='SkPath_getGenerationID'></a>
## getGenerationID

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
uint32_t <a href='#SkPath_getGenerationID'>getGenerationID</a>() const
</pre>

Returns a non-zero, globally unique value. A different value is returned
if <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, or <a href='#Conic_Weight'>Conic Weight</a> changes.

Setting <a href='#Fill_Type'>Fill Type</a> does not change <a href='#Generation_ID'>Generation ID</a>.

Each time the path is modified, a different <a href='#Generation_ID'>Generation ID</a> will be returned.

<a href='#Fill_Type'>Fill Type</a> does affect <a href='#Generation_ID'>Generation ID</a> on Android framework.

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

---

<a name='SkPath_isValid'></a>
## isValid

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_isValid'>isValid</a>() const
</pre>

Returns if <a href='#Path'>Path</a> data is consistent. Corrupt <a href='#Path'>Path</a> data is detected if
internal values are out of range or internal storage does not match
array dimensions.

### Return Value

true if <a href='#Path'>Path</a> data is consistent

---

<a name='SkPath_pathRefIsValid'></a>
## pathRefIsValid

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_pathRefIsValid'>pathRefIsValid</a>() const
</pre>

Deprecated.

soon

---

# <a name='SkPath_Iter'>Class SkPath::Iter</a>

## <a name='Constructor'>Constructor</a>


SkPath can be constructed or initialized by these functions, including C++ class constructors.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
</table>

## <a name='Member_Function'>Member_Function</a>


SkPath member functions read and modify the structure properties.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
</table>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='#SkPath_Iter_empty_constructor'>Iter</a> {
public:
    <a href='#SkPath_Iter_empty_constructor'>Iter()</a>;
    <a href='#SkPath_const_SkPath'>Iter(const SkPath& path, bool forceClose)</a>;
    void <a href='#SkPath_Iter_setPath'>setPath(const SkPath& path, bool forceClose)</a>;
    <a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Iter_next'>next(SkPoint pts[4], bool doConsumeDegenerates = true, bool exact = false)</a>;
    <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPath_Iter_conicWeight'>conicWeight</a> const;
    bool <a href='#SkPath_Iter_isCloseLine'>isCloseLine</a> const;
    bool <a href='#SkPath_Iter_isClosedContour'>isClosedContour</a> const;
};
</pre>

Iterates through <a href='#Verb_Array'>Verb Array</a>, and associated <a href='#Point_Array'>Point Array</a> and <a href='#Conic_Weight'>Conic Weight</a>.
Provides options to treat open <a href='#Contour'>Contours</a> as closed, and to ignore
degenerate data.

### Example

<div><fiddle-embed name="2f53df9201769ab7e7c0e164a1334309"><div>Ignoring the actual <a href='#Verb'>Verbs</a> and replacing them with <a href='#Quad'>Quads</a> rounds the
path of the glyph.
</div></fiddle-embed></div>

### See Also

<a href='#SkPath_RawIter'>RawIter</a>

<a name='SkPath_Iter_empty_constructor'></a>
## Iter

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_Iter_empty_constructor'>Iter</a>()
</pre>

Initializes <a href='#SkPath_Iter_empty_constructor'>Iter</a> with an empty <a href='#Path'>Path</a>. <a href='#SkPath_Iter_next'>next</a> on <a href='#SkPath_Iter_empty_constructor'>Iter</a> returns <a href='#SkPath_kDone_Verb'>kDone Verb</a>.
Call <a href='#SkPath_Iter_setPath'>setPath</a> to initialize <a href='#SkPath_Iter_empty_constructor'>Iter</a> at a later time.

### Return Value

<a href='#SkPath_Iter_empty_constructor'>Iter</a> of empty <a href='#Path'>Path</a>

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

<a name='SkPath_const_SkPath'></a>
## Iter

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_Iter_empty_constructor'>Iter</a>(const <a href='#SkPath'>SkPath</a>& path, bool forceClose)
</pre>

Sets <a href='#SkPath_Iter_empty_constructor'>Iter</a> to return elements of <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, and <a href='#Conic_Weight'>Conic Weight</a> in <a href='#SkPath_const_SkPath_path'>path</a>.
If <a href='#SkPath_const_SkPath_forceClose'>forceClose</a> is true, <a href='#SkPath_Iter_empty_constructor'>Iter</a> will add <a href='#SkPath_kLine_Verb'>kLine Verb</a> and <a href='#SkPath_kClose_Verb'>kClose Verb</a> after each
open <a href='#Contour'>Contour</a>. <a href='#SkPath_const_SkPath_path'>path</a> is not altered.

### Parameters

<table>  <tr>    <td><a name='SkPath_const_SkPath_path'><code><strong>path</strong></code></a></td>
    <td><a href='#Path'>Path</a> to iterate</td>
  </tr>
  <tr>    <td><a name='SkPath_const_SkPath_forceClose'><code><strong>forceClose</strong></code></a></td>
    <td>true if open <a href='#Contour'>Contours</a> generate <a href='#SkPath_kClose_Verb'>kClose Verb</a></td>
  </tr>
</table>

### Return Value

<a href='#SkPath_Iter_empty_constructor'>Iter</a> of <a href='#SkPath_const_SkPath_path'>path</a>

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
void <a href='#SkPath_Iter_setPath'>setPath</a>(const <a href='#SkPath'>SkPath</a>& path, bool forceClose)
</pre>

Sets <a href='#SkPath_Iter_empty_constructor'>Iter</a> to return elements of <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, and <a href='#Conic_Weight'>Conic Weight</a> in <a href='#SkPath_Iter_setPath_path'>path</a>.
If <a href='#SkPath_Iter_setPath_forceClose'>forceClose</a> is true, <a href='#SkPath_Iter_empty_constructor'>Iter</a> will add <a href='#SkPath_kLine_Verb'>kLine Verb</a> and <a href='#SkPath_kClose_Verb'>kClose Verb</a> after each
open <a href='#Contour'>Contour</a>. <a href='#SkPath_Iter_setPath_path'>path</a> is not altered.

### Parameters

<table>  <tr>    <td><a name='SkPath_Iter_setPath_path'><code><strong>path</strong></code></a></td>
    <td><a href='#Path'>Path</a> to iterate</td>
  </tr>
  <tr>    <td><a name='SkPath_Iter_setPath_forceClose'><code><strong>forceClose</strong></code></a></td>
    <td>true if open <a href='#Contour'>Contours</a> generate <a href='#SkPath_kClose_Verb'>kClose Verb</a></td>
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

<a href='#SkPath_const_SkPath'>Iter(const SkPath& path, bool forceClose)</a>

---

<a name='SkPath_Iter_next'></a>
## next

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_Iter_next'>next</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> pts[4], bool doConsumeDegenerates = true, bool exact = false)
</pre>

Returns next <a href='#SkPath_Verb'>Verb</a> in <a href='#Verb_Array'>Verb Array</a>, and advances <a href='#SkPath_Iter_empty_constructor'>Iter</a>.
When <a href='#Verb_Array'>Verb Array</a> is exhausted, returns <a href='#SkPath_kDone_Verb'>kDone Verb</a>.

Zero to four <a href='SkPoint_Reference#Point'>Points</a> are stored in <a href='#SkPath_Iter_next_pts'>pts</a>, depending on the returned <a href='#SkPath_Verb'>Verb</a>.

If <a href='#SkPath_Iter_next_doConsumeDegenerates'>doConsumeDegenerates</a> is true, skip consecutive <a href='#SkPath_kMove_Verb'>kMove Verb</a> entries, returning
only the last in the series; and skip very small <a href='undocumented#Line'>Lines</a>, <a href='#Quad'>Quads</a>, and <a href='#Conic'>Conics</a>; and
skip <a href='#SkPath_kClose_Verb'>kClose Verb</a> following <a href='#SkPath_kMove_Verb'>kMove Verb</a>.
if <a href='#SkPath_Iter_next_doConsumeDegenerates'>doConsumeDegenerates</a> is true and <a href='#SkPath_Iter_next_exact'>exact</a> is true, only skip <a href='undocumented#Line'>Lines</a>, <a href='#Quad'>Quads</a>, and
<a href='#Conic'>Conics</a> with zero lengths.

### Parameters

<table>  <tr>    <td><a name='SkPath_Iter_next_pts'><code><strong>pts</strong></code></a></td>
    <td>storage for <a href='SkPoint_Reference#Point'>Point</a> data describing returned <a href='#SkPath_Verb'>Verb</a></td>
  </tr>
  <tr>    <td><a name='SkPath_Iter_next_doConsumeDegenerates'><code><strong>doConsumeDegenerates</strong></code></a></td>
    <td>if true, skip degenerate <a href='#Verb'>Verbs</a></td>
  </tr>
  <tr>    <td><a name='SkPath_Iter_next_exact'><code><strong>exact</strong></code></a></td>
    <td>skip zero length curves</td>
  </tr>
</table>

### Return Value

next <a href='#SkPath_Verb'>Verb</a> from <a href='#Verb_Array'>Verb Array</a>

### Example

<div><fiddle-embed name="00ae8984856486bdb626d0ed6587855a"><div>skip degenerate skips the first in a <a href='#SkPath_kMove_Verb'>kMove Verb</a> pair, the <a href='#SkPath_kMove_Verb'>kMove Verb</a>
followed by the <a href='#SkPath_kClose_Verb'>kClose Verb</a>, the zero length <a href='undocumented#Line'>Line</a> and the very small <a href='undocumented#Line'>Line</a>.

skip degenerate if <a href='#SkPath_Iter_next_exact'>exact</a> skips the same as skip degenerate, but shows
the very small <a href='undocumented#Line'>Line</a>.

skip none shows all of the <a href='#Verb'>Verbs</a> and <a href='SkPoint_Reference#Point'>Points</a> in <a href='#Path'>Path</a>.
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

---

<a name='SkPath_Iter_conicWeight'></a>
## conicWeight

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPath_Iter_conicWeight'>conicWeight</a>() const
</pre>

Returns <a href='#Conic_Weight'>Conic Weight</a> if <a href='#SkPath_Iter_next'>next</a> returned <a href='#SkPath_kConic_Verb'>kConic Verb</a>.

If <a href='#SkPath_Iter_next'>next</a> has not been called, or <a href='#SkPath_Iter_next'>next</a> did not return <a href='#SkPath_kConic_Verb'>kConic Verb</a>,
result is undefined.

### Return Value

<a href='#Conic_Weight'>Conic Weight</a> for <a href='#Conic'>Conic</a> <a href='SkPoint_Reference#Point'>Points</a> returned by <a href='#SkPath_Iter_next'>next</a>

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
bool <a href='#SkPath_Iter_isCloseLine'>isCloseLine</a>() const
</pre>

Returns true if last <a href='#SkPath_kLine_Verb'>kLine Verb</a> returned by <a href='#SkPath_Iter_next'>next</a> was generated
by <a href='#SkPath_kClose_Verb'>kClose Verb</a>. When true, the end point returned by <a href='#SkPath_Iter_next'>next</a> is
also the start point of <a href='#Contour'>Contour</a>.

If <a href='#SkPath_Iter_next'>next</a> has not been called, or <a href='#SkPath_Iter_next'>next</a> did not return <a href='#SkPath_kLine_Verb'>kLine Verb</a>,
result is undefined.

### Return Value

true if last <a href='#SkPath_kLine_Verb'>kLine Verb</a> was generated by <a href='#SkPath_kClose_Verb'>kClose Verb</a>

### Example

<div><fiddle-embed name="11f8f0efe6291019fee0ac17844f6c1a">

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

<a href='#SkPath_close'>close</a>

---

<a name='SkPath_Iter_isClosedContour'></a>
## isClosedContour

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
bool <a href='#SkPath_Iter_isClosedContour'>isClosedContour</a>() const
</pre>

Returns true if subsequent calls to <a href='#SkPath_Iter_next'>next</a> return <a href='#SkPath_kClose_Verb'>kClose Verb</a> before returning
<a href='#SkPath_kMove_Verb'>kMove Verb</a>. if true, <a href='#Contour'>Contour</a> <a href='#SkPath_Iter_empty_constructor'>Iter</a> is processing may end with <a href='#SkPath_kClose_Verb'>kClose Verb</a>, or
<a href='#SkPath_Iter_empty_constructor'>Iter</a> may have been initialized with force close set to true.

### Return Value

true if <a href='#Contour'>Contour</a> is closed

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

<a href='#SkPath_const_SkPath'>Iter(const SkPath& path, bool forceClose)</a>

---

# <a name='SkPath_RawIter'>Class SkPath::RawIter</a>

## <a name='Constructor'>Constructor</a>


SkPath can be constructed or initialized by these functions, including C++ class constructors.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
</table>

## <a name='Member_Function'>Member_Function</a>


SkPath member functions read and modify the structure properties.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
</table>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
    class <a href='#SkPath_RawIter_empty_constructor'>RawIter</a> {
    public:
        <a href='#SkPath_RawIter_empty_constructor'>RawIter()</a>;
        <a href='#SkPath_copy_const_SkPath'>RawIter(const SkPath& path)</a>;
        void <a href='#SkPath_RawIter_setPath'>setPath(const SkPath& path)</a>;
        <a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_RawIter_next'>next(SkPoint pts[4])</a>;
        <a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_RawIter_peek'>peek</a> const;
        <a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPath_RawIter_conicWeight'>conicWeight</a> const;
    }
</pre>

Iterates through <a href='#Verb_Array'>Verb Array</a>, and associated <a href='#Point_Array'>Point Array</a> and <a href='#Conic_Weight'>Conic Weight</a>.
<a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, and <a href='#Conic_Weight'>Conic Weight</a> are returned unaltered.

<a name='SkPath_RawIter_empty_constructor'></a>
## RawIter

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_RawIter_empty_constructor'>RawIter</a>()
</pre>

Initializes <a href='#SkPath_RawIter_empty_constructor'>RawIter</a> with an empty <a href='#Path'>Path</a>. <a href='#SkPath_RawIter_next'>next</a> on <a href='#SkPath_RawIter_empty_constructor'>RawIter</a> returns <a href='#SkPath_kDone_Verb'>kDone Verb</a>.
Call <a href='#SkPath_RawIter_setPath'>setPath</a> to initialize <a href='#SkPath_Iter'>SkPath::Iter</a> at a later time.

### Return Value

<a href='#SkPath_RawIter_empty_constructor'>RawIter</a> of empty <a href='#Path'>Path</a>

---

<a name='SkPath_copy_const_SkPath'></a>
## RawIter

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_RawIter_empty_constructor'>RawIter</a>(const <a href='#SkPath'>SkPath</a>& path)
</pre>

Sets <a href='#SkPath_RawIter_empty_constructor'>RawIter</a> to return elements of <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, and <a href='#Conic_Weight'>Conic Weight</a> in <a href='#SkPath_copy_const_SkPath_path'>path</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_copy_const_SkPath_path'><code><strong>path</strong></code></a></td>
    <td><a href='#Path'>Path</a> to iterate</td>
  </tr>
</table>

### Return Value

<a href='#SkPath_RawIter_empty_constructor'>RawIter</a> of <a href='#SkPath_copy_const_SkPath_path'>path</a>

---

<a name='SkPath_RawIter_setPath'></a>
## setPath

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkPath_RawIter_setPath'>setPath</a>(const <a href='#SkPath'>SkPath</a>& path)
</pre>

Sets <a href='#SkPath_Iter'>SkPath::Iter</a> to return elements of <a href='#Verb_Array'>Verb Array</a>, <a href='#Point_Array'>Point Array</a>, and <a href='#Conic_Weight'>Conic Weight</a> in <a href='#SkPath_RawIter_setPath_path'>path</a>.

### Parameters

<table>  <tr>    <td><a name='SkPath_RawIter_setPath_path'><code><strong>path</strong></code></a></td>
    <td><a href='#Path'>Path</a> to iterate</td>
  </tr>
</table>

---

<a name='SkPath_RawIter_next'></a>
## next

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_RawIter_next'>next</a>(<a href='SkPoint_Reference#SkPoint'>SkPoint</a> pts[4])
</pre>

Returns next <a href='#SkPath_Verb'>Verb</a> in <a href='#Verb_Array'>Verb Array</a>, and advances <a href='#SkPath_RawIter_empty_constructor'>RawIter</a>.
When <a href='#Verb_Array'>Verb Array</a> is exhausted, returns <a href='#SkPath_kDone_Verb'>kDone Verb</a>.
Zero to four <a href='SkPoint_Reference#Point'>Points</a> are stored in <a href='#SkPath_RawIter_next_pts'>pts</a>, depending on the returned <a href='#SkPath_Verb'>Verb</a>.

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

<a href='#SkPath_RawIter_peek'>peek</a>

---

<a name='SkPath_RawIter_peek'></a>
## peek

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkPath_Verb'>Verb</a> <a href='#SkPath_RawIter_peek'>peek</a>() const
</pre>

Returns next <a href='#SkPath_Verb'>Verb</a>, but does not advance <a href='#SkPath_RawIter_empty_constructor'>RawIter</a>.

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

StdOut is not really volatile, it just produces the wrong result.
A simple fix changes the output of hairlines and needs to be
investigated to see if the change is correct or not.
see change 21340 (abandoned for now)

### See Also

<a href='#SkPath_RawIter_next'>next</a>

---

<a name='SkPath_RawIter_conicWeight'></a>
## conicWeight

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='undocumented#SkScalar'>SkScalar</a> <a href='#SkPath_RawIter_conicWeight'>conicWeight</a>() const
</pre>

Returns <a href='#Conic_Weight'>Conic Weight</a> if <a href='#SkPath_RawIter_next'>next</a> returned <a href='#SkPath_kConic_Verb'>kConic Verb</a>.

If <a href='#SkPath_RawIter_next'>next</a> has not been called, or <a href='#SkPath_RawIter_next'>next</a> did not return <a href='#SkPath_kConic_Verb'>kConic Verb</a>,
result is undefined.

### Return Value

<a href='#Conic_Weight'>Conic Weight</a> for <a href='#Conic'>Conic</a> <a href='SkPoint_Reference#Point'>Points</a> returned by <a href='#SkPath_RawIter_next'>next</a>

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

