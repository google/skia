SkPath Overview
===
<a href='SkPath_Reference#Path'>Path</a> contains <a href='undocumented#Line'>Lines</a> and <a href='undocumented#Curve'>Curves</a> which can be stroked or filled. <a href='SkPath_Overview#Contour'>Contour</a> is
composed of a series of connected <a href='undocumented#Line'>Lines</a> and <a href='undocumented#Curve'>Curves</a>. <a href='SkPath_Reference#Path'>Path</a> may contain zero,
one, or more <a href='SkPath_Overview#Contour'>Contours</a>.
Each <a href='undocumented#Line'>Line</a> and <a href='undocumented#Curve'>Curve</a> are described by Verb, <a href='SkPoint_Reference#Point'>Points</a>, and optional <a href='#Path_Conic_Weight'>Path_Conic_Weight</a>.

Each pair of connected <a href='undocumented#Line'>Lines</a> and <a href='undocumented#Curve'>Curves</a> share common <a href='SkPoint_Reference#Point'>Point</a>; for instance, <a href='SkPath_Reference#Path'>Path</a>
containing two connected <a href='undocumented#Line'>Lines</a> are described the <a href='#Path_Verb'>Path_Verb</a> sequence:
<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_kMove_Verb'>kMove_Verb</a>, <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_kLine_Verb'>kLine_Verb</a>, <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_kLine_Verb'>kLine_Verb</a>; and a <a href='SkPoint_Reference#Point'>Point</a> sequence
with three entries, sharing
the middle entry as the end of the first <a href='undocumented#Line'>Line</a> and the start of the second <a href='undocumented#Line'>Line</a>.

<a href='SkPath_Reference#Path'>Path</a> components <a href='undocumented#Arc'>Arc</a>, <a href='SkRect_Reference#Rect'>Rect</a>, <a href='#RRect'>Round_Rect</a>, <a href='undocumented#Circle'>Circle</a>, and <a href='undocumented#Oval'>Oval</a> are composed of
<a href='undocumented#Line'>Lines</a> and <a href='undocumented#Curve'>Curves</a> with as many <a href='SkPath_Reference#Verb'>Verbs</a> and <a href='SkPoint_Reference#Point'>Points</a> required
for an exact description. Once added to <a href='SkPath_Reference#Path'>Path</a>, these components may lose their
identity; although <a href='SkPath_Reference#Path'>Path</a> can be inspected to determine if it describes a single
<a href='SkRect_Reference#Rect'>Rect</a>, <a href='undocumented#Oval'>Oval</a>, <a href='#RRect'>Round_Rect</a>, and so on.

### Example

<div><fiddle-embed name="93887af0c1dac49521972698cf04069c"><div><a href='SkPath_Reference#Path'>Path</a> contains three <a href='SkPath_Overview#Contour'>Contours</a>: <a href='undocumented#Line'>Line</a>, <a href='undocumented#Circle'>Circle</a>, and <a href='SkPath_Reference#Quad'>Quad</a>. <a href='undocumented#Line'>Line</a> is stroked but
not filled. <a href='undocumented#Circle'>Circle</a> is stroked and filled; <a href='undocumented#Circle'>Circle</a> stroke forms a loop. <a href='SkPath_Reference#Quad'>Quad</a>
is stroked and filled, but since it is not closed, <a href='SkPath_Reference#Quad'>Quad</a> does not stroke a loop.
</div></fiddle-embed></div>

<a href='SkPath_Reference#Path'>Path</a> contains a <a href='#Path_Fill_Type'>Path_Fill_Type</a> which determines whether overlapping <a href='SkPath_Overview#Contour'>Contours</a>
form fills or holes. <a href='#Path_Fill_Type'>Path_Fill_Type</a> also determines whether area inside or outside
<a href='undocumented#Line'>Lines</a> and <a href='undocumented#Curve'>Curves</a> is filled.

### Example

<div><fiddle-embed name="36a995442c081ee779ecab2962d36e69"><div><a href='SkPath_Reference#Path'>Path</a> is drawn filled, then stroked, then stroked and filled.
</div></fiddle-embed></div>

<a href='SkPath_Reference#Path'>Path</a> contents are never shared. Copying <a href='SkPath_Reference#Path'>Path</a> by value effectively creates
a new <a href='SkPath_Reference#Path'>Path</a> independent of the original. Internally, the copy does not duplicate
its contents until it is edited, to reduce memory use and improve performance.

<a name='Contour'></a>

---

<a href='SkPath_Overview#Contour'>Contour</a> contains one or more <a href='SkPath_Reference#Verb'>Verbs</a>, and as many <a href='SkPoint_Reference#Point'>Points</a> as
are required to satisfy <a href='#Path_Verb_Array'>Path_Verb_Array</a>. First <a href='#Path_Verb'>Path_Verb</a> in <a href='SkPath_Reference#Path'>Path</a> is always
<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_kMove_Verb'>kMove_Verb</a>; each <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_kMove_Verb'>kMove_Verb</a> that follows starts a new <a href='SkPath_Overview#Contour'>Contour</a>.

### Example

<div><fiddle-embed name="0374f2dcd7effeb1dd435205a6c2de6f"><div>Each <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_moveTo'>moveTo</a> starts a new <a href='SkPath_Overview#Contour'>Contour</a>, and content after <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_close'>close()</a>
also starts a new <a href='SkPath_Overview#Contour'>Contour</a>. Since <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_conicTo'>conicTo</a> is not preceded by
<a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_moveTo'>moveTo</a>, the first <a href='SkPoint_Reference#Point'>Point</a> of the third <a href='SkPath_Overview#Contour'>Contour</a> starts at the last <a href='SkPoint_Reference#Point'>Point</a>
of the second <a href='SkPath_Overview#Contour'>Contour</a>.
</div></fiddle-embed></div>

If final <a href='#Path_Verb'>Path_Verb</a> in <a href='SkPath_Overview#Contour'>Contour</a> is <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_kClose_Verb'>kClose_Verb</a>, <a href='undocumented#Line'>Line</a> connects <a href='#Path_Last_Point'>Path_Last_Point</a> in
<a href='SkPath_Overview#Contour'>Contour</a> with first <a href='SkPoint_Reference#Point'>Point</a>. A closed <a href='SkPath_Overview#Contour'>Contour</a>, stroked, draws
<a href='#Paint_Stroke_Join'>Paint_Stroke_Join</a> at <a href='#Path_Last_Point'>Path_Last_Point</a> and first <a href='SkPoint_Reference#Point'>Point</a>. Without <a href='SkPath_Reference#SkPath'>SkPath</a>::<a href='#SkPath_kClose_Verb'>kClose_Verb</a>
as final Verb, <a href='#Path_Last_Point'>Path_Last_Point</a> and first <a href='SkPoint_Reference#Point'>Point</a> are not connected; <a href='SkPath_Overview#Contour'>Contour</a>
remains open. An open <a href='SkPath_Overview#Contour'>Contour</a>, stroked, draws <a href='#Paint_Stroke_Cap'>Paint_Stroke_Cap</a> at
<a href='#Path_Last_Point'>Path_Last_Point</a> and first <a href='SkPoint_Reference#Point'>Point</a>.

### Example

<div><fiddle-embed name="7a1f39b12d2cd8b7f5b1190879259cb2"><div><a href='SkPath_Reference#Path'>Path</a> is drawn stroked, with an open <a href='SkPath_Overview#Contour'>Contour</a> and a closed <a href='SkPath_Overview#Contour'>Contour</a>.
</div></fiddle-embed></div>

<a name='Contour_Zero_Length'></a>

---

<a href='SkPath_Overview#Contour'>Contour</a> length is distance traveled from first <a href='SkPoint_Reference#Point'>Point</a> to <a href='#Path_Last_Point'>Path_Last_Point</a>,
plus, if <a href='SkPath_Overview#Contour'>Contour</a> is closed, distance from <a href='#Path_Last_Point'>Path_Last_Point</a> to first <a href='SkPoint_Reference#Point'>Point</a>.
Even if <a href='SkPath_Overview#Contour'>Contour</a> length is zero, stroked <a href='undocumented#Line'>Lines</a> are drawn if <a href='#Paint_Stroke_Cap'>Paint_Stroke_Cap</a>
makes them visible.

### Example

<div><fiddle-embed name="62848df605af6258653d9e16b27d8f7f"></fiddle-embed></div>

