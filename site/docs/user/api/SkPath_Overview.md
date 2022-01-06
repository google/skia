---
title: 'SkPath Overview'
linkTitle: 'SkPath Overview'

weight: 270
---

<a href='https://api.skia.org/classSkPath.html'>Path</a> contains
<a href='undocumented#Line'>Lines</a> and
<a href='undocumented#Curve'>Curves</a> which can be stroked or filled.
<a href='#Contour'>Contour</a> is composed of a series of connected
<a href='undocumented#Line'>Lines</a> and
<a href='undocumented#Curve'>Curves</a>.
<a href='https://api.skia.org/classSkPath.html'>Path</a> may contain zero, one,
or more <a href='#Contour'>Contours</a>. Each
<a href='undocumented#Line'>Line</a> and <a href='undocumented#Curve'>Curve</a>
are described by Verb,
<a href='https://api.skia.org/structSkPoint.html'>Points</a>, and optional
<a href='#Path_Conic_Weight'>Path_Conic_Weight</a>.

Each pair of connected <a href='undocumented#Line'>Lines</a> and
<a href='undocumented#Curve'>Curves</a> share common
<a href='https://api.skia.org/structSkPoint.html'>Point</a>; for instance,
<a href='https://api.skia.org/classSkPath.html'>Path</a> containing two
connected <a href='undocumented#Line'>Lines</a> are described the
<a href='#Path_Verb'>Path_Verb</a> sequence:
<a href='https://api.skia.org/classSkPath.html'>SkPath</a>::<a href='#SkPath_kMove_Verb'>kMove_Verb</a>,
<a href='https://api.skia.org/classSkPath.html'>SkPath</a>::<a href='#SkPath_kLine_Verb'>kLine_Verb</a>,
<a href='https://api.skia.org/classSkPath.html'>SkPath</a>::<a href='#SkPath_kLine_Verb'>kLine_Verb</a>;
and a <a href='https://api.skia.org/structSkPoint.html'>Point</a> sequence with
three entries, sharing the middle entry as the end of the first
<a href='undocumented#Line'>Line</a> and the start of the second
<a href='undocumented#Line'>Line</a>.

<a href='https://api.skia.org/classSkPath.html'>Path</a> components
<a href='undocumented#Arc'>Arc</a>,
<a href='https://api.skia.org/classSkPath.html#af037025a1adad16072abbbcd83b621f2'>Rect</a>,
<a href='#RRect'>Round_Rect</a>, <a href='undocumented#Circle'>Circle</a>, and
<a href='undocumented#Oval'>Oval</a> are composed of
<a href='undocumented#Line'>Lines</a> and
<a href='undocumented#Curve'>Curves</a> with as many
<a href='https://api.skia.org/classSkPath.html#ac36f638ac96f3428626e993eacf84ff0'>Verbs</a>
and <a href='https://api.skia.org/structSkPoint.html'>Points</a> required for an
exact description. Once added to
<a href='https://api.skia.org/classSkPath.html'>Path</a>, these components may
lose their identity; although
<a href='https://api.skia.org/classSkPath.html'>Path</a> can be inspected to
determine if it describes a single
<a href='https://api.skia.org/classSkPath.html#af037025a1adad16072abbbcd83b621f2'>Rect</a>,
<a href='undocumented#Oval'>Oval</a>, <a href='#RRect'>Round_Rect</a>, and so
on.

### Example

<div><fiddle-embed-sk name="93887af0c1dac49521972698cf04069c"><div><a href='https://api.skia.org/classSkPath.html'>Path</a> contains three <a href='#Contour'>Contours</a>: <a href='undocumented#Line'>Line</a>, <a href='undocumented#Circle'>Circle</a>, and <a href='https://api.skia.org/classSkPath.html#ad75d5a934476ac6543d6d7ddd8dbb90a'>Quad</a>. <a href='undocumented#Line'>Line</a> is stroked but
not filled. <a href='undocumented#Circle'>Circle</a> is stroked and filled; <a href='undocumented#Circle'>Circle</a> stroke forms a loop. <a href='https://api.skia.org/classSkPath.html#ad75d5a934476ac6543d6d7ddd8dbb90a'>Quad</a>
is stroked and filled, but since it is not closed, <a href='https://api.skia.org/classSkPath.html#ad75d5a934476ac6543d6d7ddd8dbb90a'>Quad</a> does not stroke a loop.
</div></fiddle-embed-sk></div>

<a href='https://api.skia.org/classSkPath.html'>Path</a> contains a
<a href='#Path_Fill_Type'>Path_Fill_Type</a> which determines whether
overlapping <a href='#Contour'>Contours</a> form fills or holes.
<a href='#Path_Fill_Type'>Path_Fill_Type</a> also determines whether area inside
or outside <a href='undocumented#Line'>Lines</a> and
<a href='undocumented#Curve'>Curves</a> is filled.

### Example

<div><fiddle-embed-sk name="36a995442c081ee779ecab2962d36e69"><div><a href='https://api.skia.org/classSkPath.html'>Path</a> is drawn filled, then stroked, then stroked and filled.
</div></fiddle-embed-sk></div>

<a href='https://api.skia.org/classSkPath.html'>Path</a> contents are never
shared. Copying <a href='https://api.skia.org/classSkPath.html'>Path</a> by
value effectively creates a new
<a href='https://api.skia.org/classSkPath.html'>Path</a> independent of the
original. Internally, the copy does not duplicate its contents until it is
edited, to reduce memory use and improve performance.

<a name='Contour'></a>

---

<a href='#Contour'>Contour</a> contains one or more
<a href='https://api.skia.org/classSkPath.html#ac36f638ac96f3428626e993eacf84ff0'>Verbs</a>,
and as many <a href='https://api.skia.org/structSkPoint.html'>Points</a> as are
required to satisfy <a href='#Path_Verb_Array'>Path_Verb_Array</a>. First
<a href='#Path_Verb'>Path_Verb</a> in
<a href='https://api.skia.org/classSkPath.html'>Path</a> is always
<a href='https://api.skia.org/classSkPath.html'>SkPath</a>::<a href='#SkPath_kMove_Verb'>kMove_Verb</a>;
each
<a href='https://api.skia.org/classSkPath.html'>SkPath</a>::<a href='#SkPath_kMove_Verb'>kMove_Verb</a>
that follows starts a new <a href='#Contour'>Contour</a>.

### Example

<div><fiddle-embed-sk name="0374f2dcd7effeb1dd435205a6c2de6f"><div>Each <a href='https://api.skia.org/classSkPath.html'>SkPath</a>::<a href='#SkPath_moveTo'>moveTo</a> starts a new <a href='#Contour'>Contour</a>, and content after <a href='https://api.skia.org/classSkPath.html'>SkPath</a>::<a href='#SkPath_close'>close()</a>
also starts a new <a href='#Contour'>Contour</a>. Since <a href='https://api.skia.org/classSkPath.html'>SkPath</a>::<a href='#SkPath_conicTo'>conicTo</a> is not preceded by
<a href='https://api.skia.org/classSkPath.html'>SkPath</a>::<a href='#SkPath_moveTo'>moveTo</a>, the first <a href='https://api.skia.org/structSkPoint.html'>Point</a> of the third <a href='#Contour'>Contour</a> starts at the last <a href='https://api.skia.org/structSkPoint.html'>Point</a>
of the second <a href='#Contour'>Contour</a>.
</div></fiddle-embed-sk></div>

If final <a href='#Path_Verb'>Path_Verb</a> in <a href='#Contour'>Contour</a> is
<a href='https://api.skia.org/classSkPath.html'>SkPath</a>::<a href='#SkPath_kClose_Verb'>kClose_Verb</a>,
<a href='undocumented#Line'>Line</a> connects
<a href='#Path_Last_Point'>Path_Last_Point</a> in <a href='#Contour'>Contour</a>
with first <a href='https://api.skia.org/structSkPoint.html'>Point</a>. A closed
<a href='#Contour'>Contour</a>, stroked, draws
<a href='#Paint_Stroke_Join'>Paint_Stroke_Join</a> at
<a href='#Path_Last_Point'>Path_Last_Point</a> and first
<a href='https://api.skia.org/structSkPoint.html'>Point</a>. Without
<a href='https://api.skia.org/classSkPath.html'>SkPath</a>::<a href='#SkPath_kClose_Verb'>kClose_Verb</a>
as final Verb, <a href='#Path_Last_Point'>Path_Last_Point</a> and first
<a href='https://api.skia.org/structSkPoint.html'>Point</a> are not connected;
<a href='#Contour'>Contour</a> remains open. An open
<a href='#Contour'>Contour</a>, stroked, draws
<a href='#Paint_Stroke_Cap'>Paint_Stroke_Cap</a> at
<a href='#Path_Last_Point'>Path_Last_Point</a> and first
<a href='https://api.skia.org/structSkPoint.html'>Point</a>.

### Example

<div><fiddle-embed-sk name="7a1f39b12d2cd8b7f5b1190879259cb2"><div><a href='https://api.skia.org/classSkPath.html'>Path</a> is drawn stroked, with an open <a href='#Contour'>Contour</a> and a closed <a href='#Contour'>Contour</a>.
</div></fiddle-embed-sk></div>

<a name='Contour_Zero_Length'></a>

---

<a href='#Contour'>Contour</a> length is distance traveled from first
<a href='https://api.skia.org/structSkPoint.html'>Point</a> to
<a href='#Path_Last_Point'>Path_Last_Point</a>, plus, if
<a href='#Contour'>Contour</a> is closed, distance from
<a href='#Path_Last_Point'>Path_Last_Point</a> to first
<a href='https://api.skia.org/structSkPoint.html'>Point</a>. Even if
<a href='#Contour'>Contour</a> length is zero, stroked
<a href='undocumented#Line'>Lines</a> are drawn if
<a href='#Paint_Stroke_Cap'>Paint_Stroke_Cap</a> makes them visible.

### Example

<div><fiddle-embed-sk name="62848df605af6258653d9e16b27d8f7f"></fiddle-embed-sk></div>
