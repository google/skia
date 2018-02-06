SkAutoCanvasRestore Reference
===

# <a name="Automatic_Canvas_Restore"></a> Automatic Canvas Restore

## <a name="Overview"></a> Overview

## <a name="Subtopic"></a> Subtopic

| name | description |
| --- | --- |
| &lt;a href="SkAutoCanvasRestore_Reference#Automatic_Canvas_Restore_Constructor"&gt;Constructor&lt;/a&gt; | functions that construct &lt;a href="SkAutoCanvasRestore_Reference#SkAutoCanvasRestore"&gt;SkAutoCanvasRestore&lt;/a&gt; |
| &lt;a href="SkAutoCanvasRestore_Reference#Automatic_Canvas_Restore_Member_Function"&gt;Member Function&lt;/a&gt; | static functions and member methods |

# <a name="SkAutoCanvasRestore"></a> Class SkAutoCanvasRestore
Stack helper class calls

## <a name="Constructor"></a> Constructor

| name | description |
| --- | --- |
| &lt;a href="#SkAutoCanvasRestore_SkCanvas_star"&gt;SkAutoCanvasRestore(SkCanvas* canvas, bool doSave)&lt;/a&gt; | preserves &lt;a href="SkCanvas_Reference#Canvas"&gt;Canvas&lt;/a&gt; save count |
| &lt;a href="#SkAutoCanvasRestore_destructor"&gt;~SkAutoCanvasRestore()&lt;/a&gt; | restores &lt;a href="SkCanvas_Reference#Canvas"&gt;Canvas&lt;/a&gt; to saved state |

## <a name="Member_Function"></a> Member Function

| name | description |
| --- | --- |
| &lt;a href="#SkAutoCanvasRestore_restore"&gt;restore&lt;/a&gt; | restores &lt;a href="SkCanvas_Reference#Canvas"&gt;Canvas&lt;/a&gt; to saved state |

<a name="SkAutoCanvasRestore_SkCanvas_star"></a>
## SkAutoCanvasRestore

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
SkAutoCanvasRestore(SkCanvas* canvas, bool doSave)
</pre>

Preserves <a href="SkCanvas_Reference#Canvas">Canvas</a> save count. Optionally saves <a href="#Clip">Canvas Clip</a> and <a href="#Matrix">Canvas Matrix</a>.

### Parameters

<table>  <tr>    <td><a name="SkAutoCanvasRestore_SkCanvas_star_canvas"> <code><strong>canvas </strong></code> </a></td> <td>
<a href="SkCanvas_Reference#Canvas">Canvas</a> to guard</td>
  </tr>  <tr>    <td><a name="SkAutoCanvasRestore_SkCanvas_star_doSave"> <code><strong>doSave </strong></code> </a></td> <td>
call <a href="#SkCanvas_save">SkCanvas::save()</a></td>
  </tr>
</table>

### Return Value

utility to restore <a href="SkCanvas_Reference#Canvas">Canvas</a> state on destructor

### Example

<div><fiddle-embed name="466ef576b88e29d7252422db7adeed1c"></fiddle-embed></div>

### See Also

<a href="#SkCanvas_save">SkCanvas::save</a> <a href="#SkCanvas_restore">SkCanvas::restore</a>

---

<a name="SkAutoCanvasRestore_destructor"></a>
## ~SkAutoCanvasRestore

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
~SkAutoCanvasRestore()
</pre>

Restores <a href="SkCanvas_Reference#Canvas">Canvas</a> to saved state. Destructor is called when container goes out of
scope.

### See Also

<a href="#SkCanvas_save">SkCanvas::save</a> <a href="#SkCanvas_restore">SkCanvas::restore</a>

---

<a name="SkAutoCanvasRestore_restore"></a>
## restore

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
void restore()
</pre>

Restores <a href="SkCanvas_Reference#Canvas">Canvas</a> to saved state immediately. Subsequent calls and
<a href="#SkAutoCanvasRestore_destructor">~SkAutoCanvasRestore</a> have no effect.

### Example

<div><fiddle-embed name="9f459b218ec079c1ada23f4412968f9a">

#### Example Output

~~~~
saveCanvas: false  before restore: 2
saveCanvas: false  after restore: 2
saveCanvas: true  before restore: 2
saveCanvas: true  after restore: 2
saveCanvas: false  before restore: 2
saveCanvas: false  after restore: 1
saveCanvas: true  before restore: 2
saveCanvas: true  after restore: 1
final count: 1
~~~~

</fiddle-embed></div>

### See Also

<a href="#SkCanvas_save">SkCanvas::save</a> <a href="#SkCanvas_restore">SkCanvas::restore</a>

---

