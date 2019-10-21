SkAutoCanvasRestore Reference
===


<a name='SkAutoCanvasRestore'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkAutoCanvasRestore_Reference#SkAutoCanvasRestore'>SkAutoCanvasRestore</a> {

    <a href='#SkAutoCanvasRestore_SkCanvas_star'>SkAutoCanvasRestore</a>(<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* <a href='SkCanvas_Reference#Canvas'>canvas</a>, bool doSave);
    <a href='#SkAutoCanvasRestore_destructor'>~SkAutoCanvasRestore()</a>;
    void <a href='#SkAutoCanvasRestore_restore'>restore()</a>;
};

</pre>

Stack helper class calls <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_restoreToCount'>restoreToCount</a> when <a href='SkAutoCanvasRestore_Reference#SkAutoCanvasRestore'>SkAutoCanvasRestore</a>
goes out of scope. Use this to guarantee that the <a href='SkCanvas_Reference#Canvas'>canvas</a> is restored to a known
state.

<a name='SkAutoCanvasRestore_SkCanvas_star'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkAutoCanvasRestore_SkCanvas_star'>SkAutoCanvasRestore</a>(<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* <a href='SkCanvas_Reference#Canvas'>canvas</a>, bool doSave)
</pre>

Preserves <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_save'>save()</a> count. Optionally saves <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> clip and <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkMatrix_Reference#Matrix'>matrix</a>.

### Parameters

<table>  <tr>    <td><a name='SkAutoCanvasRestore_SkCanvas_star_canvas'><code><strong>canvas</strong></code></a></td>
    <td><a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> to guard</td>
  </tr>
  <tr>    <td><a name='SkAutoCanvasRestore_SkCanvas_star_doSave'><code><strong>doSave</strong></code></a></td>
    <td>call <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_save'>save()</a></td>
  </tr>
</table>

### Return Value

utility to restore <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> state on destructor

### Example

<div><fiddle-embed name="@AutoCanvasRestore_SkCanvas_star"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_save'>save</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_restore'>restore</a>

<a name='SkAutoCanvasRestore_destructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='#SkAutoCanvasRestore_destructor'>~SkAutoCanvasRestore()</a>
</pre>

Restores <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> to saved state. Destructor is called when container goes out of
scope.

### See Also

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_save'>save</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_restore'>restore</a>

<a name='SkAutoCanvasRestore_restore'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkAutoCanvasRestore_restore'>restore()</a>
</pre>

Restores <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> to saved state immediately. Subsequent calls and
<a href='#SkAutoCanvasRestore_destructor'>~SkAutoCanvasRestore()</a> have no effect.

### Example

<div><fiddle-embed name="@AutoCanvasRestore_restore">

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

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_save'>save</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_restore'>restore</a>

