SkAutoCanvasRestore Reference
===


<a name='SkAutoCanvasRestore'></a>

---

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
class <a href='SkAutoCanvasRestore_Reference#SkAutoCanvasRestore'>SkAutoCanvasRestore</a> {
<a href='SkAutoCanvasRestore_Reference#SkAutoCanvasRestore'>public</a>:
    <a href='SkAutoCanvasRestore_Reference#SkAutoCanvasRestore'>SkAutoCanvasRestore</a>(<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* <a href='SkCanvas_Reference#Canvas'>canvas</a>, <a href='SkCanvas_Reference#Canvas'>bool</a> <a href='SkCanvas_Reference#Canvas'>doSave</a>);
    ~<a href='SkAutoCanvasRestore_Reference#SkAutoCanvasRestore'>SkAutoCanvasRestore</a>();
    <a href='SkAutoCanvasRestore_Reference#SkAutoCanvasRestore'>void</a> <a href='#SkAutoCanvasRestore_restore'>restore()</a>;
};
</pre>

Stack helper class calls <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_restoreToCount'>restoreToCount</a> <a href='#SkCanvas_restoreToCount'>when</a> <a href='SkAutoCanvasRestore_Reference#SkAutoCanvasRestore'>SkAutoCanvasRestore</a>
<a href='SkAutoCanvasRestore_Reference#SkAutoCanvasRestore'>goes</a> <a href='SkAutoCanvasRestore_Reference#SkAutoCanvasRestore'>out</a> <a href='SkAutoCanvasRestore_Reference#SkAutoCanvasRestore'>of</a> <a href='SkAutoCanvasRestore_Reference#SkAutoCanvasRestore'>scope</a>. <a href='SkAutoCanvasRestore_Reference#SkAutoCanvasRestore'>Use</a> <a href='SkAutoCanvasRestore_Reference#SkAutoCanvasRestore'>this</a> <a href='SkAutoCanvasRestore_Reference#SkAutoCanvasRestore'>to</a> <a href='SkAutoCanvasRestore_Reference#SkAutoCanvasRestore'>guarantee</a> <a href='SkAutoCanvasRestore_Reference#SkAutoCanvasRestore'>that</a> <a href='SkAutoCanvasRestore_Reference#SkAutoCanvasRestore'>the</a> <a href='SkCanvas_Reference#Canvas'>canvas</a> <a href='SkCanvas_Reference#Canvas'>is</a> <a href='SkCanvas_Reference#Canvas'>restored</a> <a href='SkCanvas_Reference#Canvas'>to</a> <a href='SkCanvas_Reference#Canvas'>a</a> <a href='SkCanvas_Reference#Canvas'>known</a>
<a href='SkCanvas_Reference#Canvas'>state</a>.

<a name='SkAutoCanvasRestore_SkCanvas_star'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
<a href='SkAutoCanvasRestore_Reference#SkAutoCanvasRestore'>SkAutoCanvasRestore</a>(<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>* <a href='SkCanvas_Reference#Canvas'>canvas</a>, <a href='SkCanvas_Reference#Canvas'>bool</a> <a href='SkCanvas_Reference#Canvas'>doSave</a>)
</pre>

Preserves <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_save'>save()</a> <a href='#SkCanvas_save'>count</a>. <a href='#SkCanvas_save'>Optionally</a> <a href='#SkCanvas_save'>saves</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>clip</a> <a href='SkCanvas_Reference#SkCanvas'>and</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkMatrix_Reference#Matrix'>matrix</a>.

### Parameters

<table>  <tr>    <td><a name='SkAutoCanvasRestore_SkCanvas_star_canvas'><code><strong>canvas</strong></code></a></td>
    <td><a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>to</a> <a href='SkCanvas_Reference#SkCanvas'>guard</a></td>
  </tr>
  <tr>    <td><a name='SkAutoCanvasRestore_SkCanvas_star_doSave'><code><strong>doSave</strong></code></a></td>
    <td>call <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_save'>save()</a></td>
  </tr>
</table>

### Return Value

utility to restore <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>state</a> <a href='SkCanvas_Reference#SkCanvas'>on</a> <a href='SkCanvas_Reference#SkCanvas'>destructor</a>

### Example

<div><fiddle-embed name="466ef576b88e29d7252422db7adeed1c"></fiddle-embed></div>

### See Also

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_save'>save</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_restore'>restore</a>

<a name='SkAutoCanvasRestore_destructor'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
~<a href='SkAutoCanvasRestore_Reference#SkAutoCanvasRestore'>SkAutoCanvasRestore</a>()
</pre>

Restores <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>to</a> <a href='SkCanvas_Reference#SkCanvas'>saved</a> <a href='SkCanvas_Reference#SkCanvas'>state</a>. <a href='SkCanvas_Reference#SkCanvas'>Destructor</a> <a href='SkCanvas_Reference#SkCanvas'>is</a> <a href='SkCanvas_Reference#SkCanvas'>called</a> <a href='SkCanvas_Reference#SkCanvas'>when</a> <a href='SkCanvas_Reference#SkCanvas'>container</a> <a href='SkCanvas_Reference#SkCanvas'>goes</a> <a href='SkCanvas_Reference#SkCanvas'>out</a> <a href='SkCanvas_Reference#SkCanvas'>of</a>
scope.

### See Also

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_save'>save</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_restore'>restore</a>

<a name='SkAutoCanvasRestore_restore'></a>

---

<pre style="padding: 1em 1em 1em 1em; width: 62.5em;background-color: #f0f0f0">
void <a href='#SkAutoCanvasRestore_restore'>restore()</a>
</pre>

Restores <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a> <a href='SkCanvas_Reference#SkCanvas'>to</a> <a href='SkCanvas_Reference#SkCanvas'>saved</a> <a href='SkCanvas_Reference#SkCanvas'>state</a> <a href='SkCanvas_Reference#SkCanvas'>immediately</a>. <a href='SkCanvas_Reference#SkCanvas'>Subsequent</a> <a href='SkCanvas_Reference#SkCanvas'>calls</a> <a href='SkCanvas_Reference#SkCanvas'>and</a>
~<a href='SkAutoCanvasRestore_Reference#SkAutoCanvasRestore'>SkAutoCanvasRestore</a> <a href='SkAutoCanvasRestore_Reference#SkAutoCanvasRestore'>have</a> <a href='SkAutoCanvasRestore_Reference#SkAutoCanvasRestore'>no</a> <a href='SkAutoCanvasRestore_Reference#SkAutoCanvasRestore'>effect</a>.

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

<a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_save'>save</a> <a href='SkCanvas_Reference#SkCanvas'>SkCanvas</a>::<a href='#SkCanvas_restore'>restore</a>

