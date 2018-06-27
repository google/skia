SkAutoCanvasRestore Reference
===

# <a name='Automatic_Canvas_Restore'>Automatic Canvas Restore</a>

# <a name='SkAutoCanvasRestore'>Class SkAutoCanvasRestore</a>
Stack helper class calls <a href='SkCanvas_Reference#SkCanvas_restoreToCount'>SkCanvas::restoreToCount</a> when <a href='#SkAutoCanvasRestore'>SkAutoCanvasRestore</a>
goes out of scope. Use this to guarantee that the canvas is restored to a known
state.

## Overview

<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Constructor'>Constructors</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>functions that construct <a href='#SkAutoCanvasRestore'>SkAutoCanvasRestore</a></td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#Member_Function'>Functions</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>global and class member functions</td>
  </tr>
</table>


## <a name='Constructor'>Constructor</a>


SkAutoCanvasRestore can be constructed or initialized by these functions, including C++ class constructors.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkAutoCanvasRestore_SkCanvas_star'>SkAutoCanvasRestore(SkCanvas* canvas, bool doSave)</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>preserves <a href='SkCanvas_Reference#Canvas'>Canvas</a> save count</td>
  </tr>
  <tr>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkAutoCanvasRestore_destructor'>~SkAutoCanvasRestore()</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>restores <a href='SkCanvas_Reference#Canvas'>Canvas</a> to saved state</td>
  </tr>
</table>

## <a name='Member_Function'>Member Function</a>


SkAutoCanvasRestore member functions read and modify the structure properties.
<table style='border-collapse: collapse; width: 62.5em'>
  <tr><th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Topic</th>
<th style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>Description</th></tr>
  <tr style='background-color: #f0f0f0; '>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '><a href='#SkAutoCanvasRestore_restore'>restore</a></td>
    <td style='text-align: left; border: 2px solid #dddddd; padding: 8px; '>restores <a href='SkCanvas_Reference#Canvas'>Canvas</a> to saved state</td>
  </tr>
</table>

<a name='SkAutoCanvasRestore_SkCanvas_star'></a>
## SkAutoCanvasRestore

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkAutoCanvasRestore'>SkAutoCanvasRestore</a>(<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* canvas, bool doSave)
</pre>

Preserves <a href='SkCanvas_Reference#Canvas'>Canvas</a> save count. Optionally saves <a href='SkCanvas_Reference#Clip'>Canvas Clip</a> and <a href='SkCanvas_Reference#Matrix'>Canvas Matrix</a>.

### Parameters

<table>  <tr>    <td><a name='SkAutoCanvasRestore_SkCanvas_star_canvas'><code><strong>canvas</strong></code></a></td>
    <td><a href='SkCanvas_Reference#Canvas'>Canvas</a> to guard</td>
  </tr>
  <tr>    <td><a name='SkAutoCanvasRestore_SkCanvas_star_doSave'><code><strong>doSave</strong></code></a></td>
    <td>call <a href='SkCanvas_Reference#SkCanvas_save'>SkCanvas::save()</a></td>
  </tr>
</table>

### Return Value

utility to restore <a href='SkCanvas_Reference#Canvas'>Canvas</a> state on destructor

### Example

<div><fiddle-embed name="466ef576b88e29d7252422db7adeed1c"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas_save'>SkCanvas::save</a> <a href='SkCanvas_Reference#SkCanvas_restore'>SkCanvas::restore</a>

---

<a name='SkAutoCanvasRestore_destructor'></a>
## ~SkAutoCanvasRestore

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkAutoCanvasRestore_destructor'>~SkAutoCanvasRestore</a>()
</pre>

Restores <a href='SkCanvas_Reference#Canvas'>Canvas</a> to saved state. <a href='undocumented#Destructor'>Destructor</a> is called when container goes out of
scope.

### See Also

<a href='SkCanvas_Reference#SkCanvas_save'>SkCanvas::save</a> <a href='SkCanvas_Reference#SkCanvas_restore'>SkCanvas::restore</a>

---

<a name='SkAutoCanvasRestore_restore'></a>
## restore

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkAutoCanvasRestore_restore'>restore</a>()
</pre>

Restores <a href='SkCanvas_Reference#Canvas'>Canvas</a> to saved state immediately. Subsequent calls and
<a href='#SkAutoCanvasRestore_destructor'>~SkAutoCanvasRestore</a> have no effect.

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

<a href='SkCanvas_Reference#SkCanvas_save'>SkCanvas::save</a> <a href='SkCanvas_Reference#SkCanvas_restore'>SkCanvas::restore</a>

---

