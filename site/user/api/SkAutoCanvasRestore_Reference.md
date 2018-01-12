SkAutoCanvasRestore Reference
===

# <a name="Automatic_Canvas_Restore"></a> Automatic Canvas Restore

# <a name="SkAutoCanvasRestore"></a> Class SkAutoCanvasRestore
Stack helper class calls

# <a name="Overview"></a> Overview

## <a name="Subtopics"></a> Subtopics

| name | description |
| --- | ---  |
| <a href="SkAutoCanvasRestore_Reference#Automatic_Canvas_Restore_Overview_Constructors">Constructors</a> | functions that construct <a href="#SkAutoCanvasRestore">SkAutoCanvasRestore</a> |
| <a href="SkAutoCanvasRestore_Reference#Automatic_Canvas_Restore_Overview_Member_Functions">Member Functions</a> | static functions and member methods |

## <a name="Constructors"></a> Constructors

| name | description |
| --- | ---  |
| <a href="#SkAutoCanvasRestore_SkCanvas_star">SkAutoCanvasRestore(SkCanvas* canvas, bool doSave)</a> | Preserves <a href="SkCanvas_Reference#Canvas">Canvas</a> save count. |
|  | Restores <a href="SkCanvas_Reference#Canvas">Canvas</a> to saved state. |

## <a name="Member_Functions"></a> Member Functions

| name | description |
| --- | ---  |
| <a href="#SkAutoCanvasRestore_restore">restore</a> | Restores <a href="SkCanvas_Reference#Canvas">Canvas</a> to saved state. |

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

utility to <a href="#SkAutoCanvasRestore_restore">restore</a> <a href="SkCanvas_Reference#Canvas">Canvas</a> state on destructor

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

