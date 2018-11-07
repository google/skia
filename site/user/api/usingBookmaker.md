usingBookmaker
===
<a href='usingBookmaker#Bookmaker'>Bookmaker</a> <a href='usingBookmaker#Bookmaker'>generates</a> <a href='usingBookmaker#Bookmaker'>markdown</a> <a href='usingBookmaker#Bookmaker'>files</a> <a href='usingBookmaker#Bookmaker'>to</a> <a href='usingBookmaker#Bookmaker'>view</a> <a href='usingBookmaker#Bookmaker'>documentation</a> <a href='usingBookmaker#Bookmaker'>on</a> <a href='usingBookmaker#Bookmaker'>skia</a>.<a href='usingBookmaker#Bookmaker'>org</a>, <a href='usingBookmaker#Bookmaker'>and</a> <a href='usingBookmaker#Bookmaker'>generates</a> <a href='usingBookmaker#Bookmaker'>includes</a> <a href='usingBookmaker#Bookmaker'>for</a> <a href='usingBookmaker#Bookmaker'>use</a> <a href='usingBookmaker#Bookmaker'>in</a> <a href='usingBookmaker#Bookmaker'>C</a>++.
<a href='usingBookmaker#Bookmaker'>Bookmaker</a> <a href='usingBookmaker#Bookmaker'>reads</a> <a href='usingBookmaker#Bookmaker'>canonical</a> <a href='usingBookmaker#Bookmaker'>documentation</a> <a href='usingBookmaker#Bookmaker'>from</a> <a href='usingBookmaker#Bookmaker'>files</a> <a href='usingBookmaker#Bookmaker'>suffixed</a> <a href='usingBookmaker#Bookmaker'>with</a> <a href='usingBookmaker#Bookmaker'>bmh</a> <a href='usingBookmaker#Bookmaker'>in</a> <a href='usingBookmaker#Bookmaker'>the</a> <a href='usingBookmaker#Bookmaker'>docs</a> <a href='usingBookmaker#Bookmaker'>directory</a>. <a href='usingBookmaker#Bookmaker'>These</a> <a href='usingBookmaker#Bookmaker'>bmh</a>
<a href='usingBookmaker#Bookmaker'>files</a> <a href='usingBookmaker#Bookmaker'>describe</a> <a href='usingBookmaker#Bookmaker'>how</a> <a href='usingBookmaker#Bookmaker'>public</a> <a href='usingBookmaker#Bookmaker'>interfaces</a> <a href='usingBookmaker#Bookmaker'>work</a>, <a href='usingBookmaker#Bookmaker'>and</a> <a href='usingBookmaker#Bookmaker'>generate</a> <a href='usingBookmaker#Bookmaker'>Skia</a> <a href='usingBookmaker#Bookmaker'>fiddle</a> <a href='usingBookmaker#Bookmaker'>examples</a> <a href='usingBookmaker#Bookmaker'>to</a> <a href='usingBookmaker#Bookmaker'>illustrate</a> <a href='usingBookmaker#Bookmaker'>them</a>.

<a href='usingBookmaker#Bookmaker'>The</a> <a href='usingBookmaker#Bookmaker'>docs</a> <a href='usingBookmaker#Bookmaker'>files</a> <a href='usingBookmaker#Bookmaker'>must</a> <a href='usingBookmaker#Bookmaker'>be</a> <a href='usingBookmaker#Bookmaker'>manually</a> <a href='usingBookmaker#Bookmaker'>edited</a> <a href='usingBookmaker#Bookmaker'>to</a> <a href='usingBookmaker#Bookmaker'>stay</a> <a href='usingBookmaker#Bookmaker'>current</a> <a href='usingBookmaker#Bookmaker'>with</a> <a href='usingBookmaker#Bookmaker'>Skia</a> <a href='usingBookmaker#Bookmaker'>as</a> <a href='usingBookmaker#Bookmaker'>it</a> <a href='usingBookmaker#Bookmaker'>evolves</a>.

<a name='Installing'></a>

Install
<a href='https://golang.org/doc/install'>Go</a></a> if needed.
Check the version. The results should be 1.10 or greater.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ go version
</pre>

Get the fiddle command <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>interface</a> <a href='undocumented#Line'>tool</a>.
<a href='undocumented#Line'>By</a> <a href='undocumented#Line'>default</a> <a href='undocumented#Line'>this</a> <a href='undocumented#Line'>will</a> <a href='undocumented#Line'>appear</a> <a href='undocumented#Line'>in</a> <a href='undocumented#Line'>your</a> <a href='undocumented#Line'>home</a> <a href='undocumented#Line'>directory</a>.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ go get go.skia.org/infra/fiddlek/go/fiddlecli
</pre>

Check the version. The command should work and the result should be 1.0 or greater.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ~/go/bin/fiddlecli --version
</pre>

If fiddlecli is already installed but out of date, update with:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ go get -u go.skia.org/infra/fiddlek/go/fiddlecli
</pre>

Build <a href='usingBookmaker#Bookmaker'>Bookmaker</a>.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ninja -C out/skia <a href='usingBookmaker#Bookmaker'>bookmaker </a>
</pre>

<a name='Running'></a>

<a href='usingBookmaker#Bookmaker'>Bookmaker</a> <a href='usingBookmaker#Bookmaker'>extracts</a> <a href='usingBookmaker#Bookmaker'>examples</a>, <a href='usingBookmaker#Bookmaker'>generates</a> <a href='usingBookmaker#Bookmaker'>example</a> <a href='usingBookmaker#Bookmaker'>hashes</a> <a href='usingBookmaker#Bookmaker'>with</a> <a href='usingBookmaker#Bookmaker'>fiddle</a>, <a href='usingBookmaker#Bookmaker'>and</a> <a href='usingBookmaker#Bookmaker'>generates</a> <a href='usingBookmaker#Bookmaker'>web</a> <a href='usingBookmaker#Bookmaker'>markdown</a>
<a href='usingBookmaker#Bookmaker'>and</a> <a href='usingBookmaker#Bookmaker'>c</a>++ <a href='usingBookmaker#Bookmaker'>includes</a>.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/skia/bookmaker -E && ~/go/bin/fiddlecli --quiet && ./out/skia/bookmaker
</pre>

A successful run generates:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
cross-check...................
</pre>

<a name='Broken_Build'></a>

The bots
<a href='https://status.skia.org/repo/skia?filter=search&search_value=Housekeeper-PerCommit-Bookmaker'>Housekeeper-PerCommit-Bookmaker</a></a> and
<a href='https://status.skia.org/repo/skia?filter=search&search_value=Housekeeper-Nightly-Bookmaker'>Housekeeper-Nightly-Bookmaker</a></a> verify that <a href='usingBookmaker#Bookmaker'>Bookmaker</a> <a href='undocumented#Data'>data</a> <a href='undocumented#Data'>in</a> <a href='undocumented#Data'>docs</a> <a href='undocumented#Data'>builds</a> <a href='undocumented#Data'>without</a> <a href='undocumented#Data'>error</a> <a href='undocumented#Data'>and</a> <a href='undocumented#Data'>is</a> <a href='undocumented#Data'>consistent</a> <a href='undocumented#Data'>with</a> <a href='undocumented#Data'>include</a> <a href='undocumented#Data'>files</a> <a href='undocumented#Data'>it</a> <a href='undocumented#Data'>documents</a>.

<a href='undocumented#Data'>Possible</a> <a href='undocumented#Data'>failures</a> <a href='undocumented#Data'>include</a>:

<table>  <tr>
    <td>Public interface in include directory does not match documented interface in docs directory.</td>
  </tr>  <tr>
    <td>Example in <a href='usingBookmaker#Bookmaker'>bookmaker</a> <a href='usingBookmaker#Bookmaker'>bmh</a> <a href='usingBookmaker#Bookmaker'>file</a> <a href='usingBookmaker#Bookmaker'>does</a> <a href='usingBookmaker#Bookmaker'>not</a> <a href='usingBookmaker#Bookmaker'>compile</a>, <a href='usingBookmaker#Bookmaker'>or</a> <a href='usingBookmaker#Bookmaker'>does</a> <a href='usingBookmaker#Bookmaker'>not</a> <a href='usingBookmaker#Bookmaker'>produce</a> <a href='usingBookmaker#Bookmaker'>expected</a> <a href='usingBookmaker#Bookmaker'>output</a>.</td>
  </tr>  <tr>
    <td>Undocumented but referenced interface is missing from undocumented <a href='usingBookmaker#Bookmaker'>bookmaker</a> <a href='usingBookmaker#Bookmaker'>file</a> <a href='usingBookmaker#Bookmaker'>in</a> <a href='usingBookmaker#Bookmaker'>docs</a> <a href='usingBookmaker#Bookmaker'>directory</a>.</td>
  </tr>
</table>

Editing comments in includes or editing private interfaces will not break the bots.
<a href='usingBookmaker#Bookmaker'>Bookmaker</a> <a href='usingBookmaker#Bookmaker'>detects</a> <a href='usingBookmaker#Bookmaker'>that</a> <a href='usingBookmaker#Bookmaker'>comments</a> <a href='usingBookmaker#Bookmaker'>edited</a> <a href='usingBookmaker#Bookmaker'>in</a> <a href='usingBookmaker#Bookmaker'>includes</a> <a href='usingBookmaker#Bookmaker'>do</a> <a href='usingBookmaker#Bookmaker'>not</a> <a href='usingBookmaker#Bookmaker'>match</a> <a href='usingBookmaker#Bookmaker'>comments</a> <a href='usingBookmaker#Bookmaker'>in</a> <a href='usingBookmaker#Bookmaker'>docs</a>; <a href='usingBookmaker#Bookmaker'>it</a> <a href='usingBookmaker#Bookmaker'>will</a> <a href='usingBookmaker#Bookmaker'>generate</a> <a href='usingBookmaker#Bookmaker'>an</a> <a href='usingBookmaker#Bookmaker'>updated</a> <a href='usingBookmaker#Bookmaker'>include</a> <a href='usingBookmaker#Bookmaker'>in</a> <a href='usingBookmaker#Bookmaker'>the</a>
<a href='usingBookmaker#Bookmaker'>directory</a> <a href='usingBookmaker#Bookmaker'>where</a> <a href='usingBookmaker#Bookmaker'>it</a> <a href='usingBookmaker#Bookmaker'>is</a> <a href='usingBookmaker#Bookmaker'>run</a>.

<a href='usingBookmaker#Bookmaker'>If</a>
<a href='https://status.skia.org/repo/skia?filter=search&search_value=Housekeeper-PerCommit-Bookmaker'>Housekeeper-PerCommit-Bookmaker</a></a> bot is red, the error is usually related to an edit to an include which has not been reflected in docs.

To fix this, edit the docs file corresponding to the changed include file.

For instance, if the change was made to <a href='SkIRect_Reference#SkIRect'>SkIRect</a>, <a href='SkIRect_Reference#SkIRect'>edit</a> <a href='SkIRect_Reference#SkIRect'>docs/SkIRect_Reference</a>.<a href='SkIRect_Reference#SkIRect'>bmh</a>.
<a href='SkIRect_Reference#SkIRect'>Checking</a> <a href='SkIRect_Reference#SkIRect'>in</a> <a href='SkIRect_Reference#SkIRect'>the</a> <a href='SkIRect_Reference#SkIRect'>edited</a> <a href='SkIRect_Reference#SkIRect'>docs/SkIRect_Reference</a>.<a href='SkIRect_Reference#SkIRect'>bmh</a> <a href='SkIRect_Reference#SkIRect'>will</a> <a href='SkIRect_Reference#SkIRect'>fix</a> <a href='SkIRect_Reference#SkIRect'>the</a> <a href='SkIRect_Reference#SkIRect'>bot</a>.

<a href='SkIRect_Reference#SkIRect'>If</a> <a href='SkIRect_Reference#SkIRect'>the</a> <a href='SkIRect_Reference#SkIRect'>interface</a> <a href='SkIRect_Reference#SkIRect'>is</a> <a href='SkIRect_Reference#SkIRect'>deprecated</a>, <a href='SkIRect_Reference#SkIRect'>but</a> <a href='SkIRect_Reference#SkIRect'>still</a> <a href='SkIRect_Reference#SkIRect'>present</a> <a href='SkIRect_Reference#SkIRect'>in</a> <a href='SkIRect_Reference#SkIRect'>the</a> <a href='SkIRect_Reference#SkIRect'>interface</a>, <a href='SkIRect_Reference#SkIRect'>mark-up</a> <a href='SkIRect_Reference#SkIRect'>the</a>
<a href='SkIRect_Reference#SkIRect'>documentation</a> <a href='SkIRect_Reference#SkIRect'>to</a> <a href='SkIRect_Reference#SkIRect'>be</a> <a href='SkIRect_Reference#SkIRect'>deprecated</a> <a href='SkIRect_Reference#SkIRect'>as</a> <a href='SkIRect_Reference#SkIRect'>well</a>.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#Method void someMethodThatIsNowDeprecated()
#Deprecated
##
</pre>

Use

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#Deprecated soon
</pre>

if the change is soon to be deprecated.

If
<a href='https://status.skia.org/repo/skia?filter=search&search_value=Housekeeper-Nightly-Bookmaker'>Housekeeper-Nightly-Bookmaker</a></a> bot is red, one of several things may have gone wrong:

<table>  <tr>
    <td>A change to include broke documentation examples.</td>
  </tr>  <tr>
    <td>Something changed the examples that output <a href='undocumented#Text'>text</a>.</td>
  </tr>  <tr>
    <td>Some interface was added, deleted, edited.</td>
  </tr>  <tr>
    <td>Documentation is malformed.</td>
  </tr>
</table>

The bot output describes what changed, and includes the file and <a href='undocumented#Line'>line</a>
<a href='undocumented#Line'>where</a> <a href='undocumented#Line'>the</a> <a href='undocumented#Line'>error</a> <a href='undocumented#Line'>occurred</a>.

<a href='undocumented#Line'>To</a> <a href='undocumented#Line'>regenerate</a> <a href='undocumented#Line'>the</a> <a href='undocumented#Line'>documentation</a>, <a href='undocumented#Line'>follow</a> <a href='undocumented#Line'>the</a> <a href='undocumented#Line'>Installing</a> <a href='undocumented#Line'>and</a> <a href='undocumented#Line'>Regenerate</a> <a href='undocumented#Line'>steps</a> <a href='undocumented#Line'>below</a>.

<a name='Editing_Comments'></a>

Edit docs instead of include/core files to update comments if possible.

The <a href='usingBookmaker#Bookmaker'>Bookmaker</a> <a href='usingBookmaker#Bookmaker'>bots</a> <a href='usingBookmaker#Bookmaker'>do</a> <a href='usingBookmaker#Bookmaker'>not</a> <a href='usingBookmaker#Bookmaker'>complain</a> <a href='usingBookmaker#Bookmaker'>if</a> <a href='usingBookmaker#Bookmaker'>the</a> <a href='usingBookmaker#Bookmaker'>docs</a> <a href='usingBookmaker#Bookmaker'>file</a> <a href='usingBookmaker#Bookmaker'>does</a> <a href='usingBookmaker#Bookmaker'>not</a> <a href='usingBookmaker#Bookmaker'>match</a> <a href='usingBookmaker#Bookmaker'>the</a>
<a href='usingBookmaker#Bookmaker'>corresponding</a> <a href='usingBookmaker#Bookmaker'>include</a> <a href='usingBookmaker#Bookmaker'>comments</a>. <a href='usingBookmaker#Bookmaker'>Running</a> <a href='usingBookmaker#Bookmaker'>Bookmaker</a> <a href='usingBookmaker#Bookmaker'>include</a> <a href='usingBookmaker#Bookmaker'>generation</a> <a href='usingBookmaker#Bookmaker'>will</a>
<a href='usingBookmaker#Bookmaker'>report</a> <a href='usingBookmaker#Bookmaker'>when</a> <a href='usingBookmaker#Bookmaker'>docs</a> <a href='usingBookmaker#Bookmaker'>and</a> <a href='usingBookmaker#Bookmaker'>includes</a> <a href='usingBookmaker#Bookmaker'>comments</a> <a href='usingBookmaker#Bookmaker'>do</a> <a href='usingBookmaker#Bookmaker'>not</a> <a href='usingBookmaker#Bookmaker'>match</a>.

<a href='usingBookmaker#Bookmaker'>For</a> <a href='usingBookmaker#Bookmaker'>instance</a>, <a href='usingBookmaker#Bookmaker'>if</a> <a href='usingBookmaker#Bookmaker'>include/core/SkSurface</a>.<a href='usingBookmaker#Bookmaker'>h</a> <a href='usingBookmaker#Bookmaker'>comments</a> <a href='usingBookmaker#Bookmaker'>do</a> <a href='usingBookmaker#Bookmaker'>not</a> <a href='usingBookmaker#Bookmaker'>match</a>
<a href='usingBookmaker#Bookmaker'>docs/SkSurface_Reference</a>.<a href='usingBookmaker#Bookmaker'>bmh</a>, <a href='usingBookmaker#Bookmaker'>running</a>:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -b docs -i include/core/SkSurface.h -p
</pre>

generates

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
wrote updated <a href='SkSurface_Reference#SkSurface'>SkSurface</a>.<a href='SkSurface_Reference#SkSurface'>h</a>
</pre>

The updated <a href='SkSurface_Reference#SkSurface'>SkSurface</a>.<a href='SkSurface_Reference#SkSurface'>h</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>written</a> <a href='SkSurface_Reference#SkSurface'>to</a> <a href='SkSurface_Reference#SkSurface'>the</a> <a href='SkSurface_Reference#SkSurface'>root</a> <a href='SkSurface_Reference#SkSurface'>to</a> <a href='SkSurface_Reference#SkSurface'>avoid</a> <a href='SkSurface_Reference#SkSurface'>subsequent</a> <a href='SkSurface_Reference#SkSurface'>runs</a> <a href='SkSurface_Reference#SkSurface'>of</a>
<a href='usingBookmaker#Bookmaker'>Bookmaker</a> <a href='usingBookmaker#Bookmaker'>from</a> <a href='usingBookmaker#Bookmaker'>recompiling</a>. <a href='usingBookmaker#Bookmaker'>if</a> <a href='SkSurface_Reference#SkSurface'>SkSurface</a>.<a href='SkSurface_Reference#SkSurface'>h</a> <a href='SkSurface_Reference#SkSurface'>was</a> <a href='SkSurface_Reference#SkSurface'>not</a> <a href='SkSurface_Reference#SkSurface'>changed</a>, <a href='SkSurface_Reference#SkSurface'>it</a> <a href='SkSurface_Reference#SkSurface'>is</a> <a href='SkSurface_Reference#SkSurface'>not</a> <a href='SkSurface_Reference#SkSurface'>written</a>,
<a href='SkSurface_Reference#SkSurface'>and</a> <a href='usingBookmaker#Bookmaker'>Bookmaker</a> <a href='usingBookmaker#Bookmaker'>will</a> <a href='usingBookmaker#Bookmaker'>not</a> <a href='usingBookmaker#Bookmaker'>generate</a> <a href='usingBookmaker#Bookmaker'>any</a> <a href='usingBookmaker#Bookmaker'>output</a>.

<a name='Broken_Example'></a>

An example may cause <a href='usingBookmaker#Bookmaker'>Bookmaker</a> <a href='usingBookmaker#Bookmaker'>or</a> <a href='usingBookmaker#Bookmaker'>a</a> <a href='usingBookmaker#Bookmaker'>bot</a> <a href='usingBookmaker#Bookmaker'>running</a> <a href='usingBookmaker#Bookmaker'>Bookmaker</a> <a href='usingBookmaker#Bookmaker'>to</a> <a href='usingBookmaker#Bookmaker'>fail</a> <a href='usingBookmaker#Bookmaker'>if</a> <a href='usingBookmaker#Bookmaker'>it</a> <a href='usingBookmaker#Bookmaker'>fails</a> <a href='usingBookmaker#Bookmaker'>to</a> <a href='usingBookmaker#Bookmaker'>compile</a>.

<a href='usingBookmaker#Bookmaker'>Fix</a> <a href='usingBookmaker#Bookmaker'>the</a> <a href='usingBookmaker#Bookmaker'>example</a> <a href='usingBookmaker#Bookmaker'>by</a> <a href='usingBookmaker#Bookmaker'>pasting</a> <a href='usingBookmaker#Bookmaker'>it</a> <a href='usingBookmaker#Bookmaker'>into</a> <a href='https://fiddle.skia.org'>Skia Fiddle</a></a> and editing it until it runs successfully.

If the example cannot be fixed, it can be commented out by changing

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#Example
</pre>

to

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#NoExample
</pre>

. The disabled example can contain additional markup, which will be ignored.

<a name='Regenerate'></a>

Complete rebuilding of all <a href='usingBookmaker#Bookmaker'>bookmaker</a> <a href='usingBookmaker#Bookmaker'>output</a> <a href='usingBookmaker#Bookmaker'>looks</a> <a href='usingBookmaker#Bookmaker'>like</a>:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -a docs/status.json -e fiddle.json
$ ~/go/bin/fiddlecli --input fiddle.json --output fiddleout.json
$ ./out/dir/bookmaker -a docs/status.json -f fiddleout.json -r site/user/api -c
$ ./out/dir/bookmaker -a docs/status.json -f fiddleout.json -r site/user/api
$ ./out/dir/bookmaker -a docs/status.json -x
$ ./out/dir/bookmaker -a docs/status.json -p
</pre>

<a name='New_Documentation'></a>

Generate an starter <a href='usingBookmaker#Bookmaker'>Bookmaker</a> <a href='usingBookmaker#Bookmaker'>file</a> <a href='usingBookmaker#Bookmaker'>from</a> <a href='usingBookmaker#Bookmaker'>an</a> <a href='usingBookmaker#Bookmaker'>existing</a> <a href='usingBookmaker#Bookmaker'>include</a>.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -i include/core/SkXXX.h -t docs
</pre>

If a method or function has an unnamed parameter, <a href='usingBookmaker#Bookmaker'>bookmaker</a> <a href='usingBookmaker#Bookmaker'>generates</a> <a href='usingBookmaker#Bookmaker'>an</a> <a href='usingBookmaker#Bookmaker'>error</a>:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
C:/puregit/include/core/SkPixmap.h(208): error: #Method missing param name
bool erase(const <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a>&, <a href='SkColor4f_Reference#SkColor4f'>const</a> <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* <a href='SkIRect_Reference#SkIRect'>subset</a> = <a href='SkIRect_Reference#SkIRect'>nullptr</a>) <a href='SkIRect_Reference#SkIRect'>const</a>
           ^
</pre>

All parameters require names to allow markdown and doxygen documents to refer to
them. After naming all parameters, check in the include before continuing.

A successful run generates

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
docs/SkXXX_Reference.bmh
</pre>

.

Next, use your favorite editor to fill out

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
docs/SkXXX_Reference.bmh
</pre>

.

<a name='Style'></a>

Documentation consists of cross references, descriptions, and examples.
All structs, classes, enums, their members and methods, functions, and so on,
require descriptions. Most also require examples.

All methods and functions should include examples if practical.
It's difficult to think of a meaningful example for class destructors.
In cases like these, change the placeholder:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#Example
</pre>

to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#NoExample
</pre>

Descriptions start with an active verb. Descriptions are complete, punctuated
sentences unless they describe parameters or return values. Parameters and
returned values are described by phrases that start lower case and do not
include trailing punctuation.

Descriptions are not self-referential; they do not include the thing they
describe. Descriptions may contain upper case or camel case references to
definitions but otherwise should be free of jargon.

Descriptions may contain code and formulas, each bracketed by markup.

Similar items may be grouped into topics. Topics may include subtopics.

Each <a href='undocumented#Document'>document</a> <a href='undocumented#Document'>begins</a> <a href='undocumented#Document'>with</a> <a href='undocumented#Document'>one</a> <a href='undocumented#Document'>or</a> <a href='undocumented#Document'>more</a> <a href='undocumented#Document'>indices</a> <a href='undocumented#Document'>that</a> <a href='undocumented#Document'>include</a> <a href='undocumented#Document'>the</a> <a href='undocumented#Document'>contents</a> <a href='undocumented#Document'>of</a>
<a href='undocumented#Document'>that</a> <a href='undocumented#Document'>file</a>. <a href='undocumented#Document'>A</a> <a href='undocumented#Document'>class</a> <a href='undocumented#Document'>reference</a> <a href='undocumented#Document'>includes</a> <a href='undocumented#Document'>an</a> <a href='undocumented#Document'>index</a> <a href='undocumented#Document'>listing</a> <a href='undocumented#Document'>contained</a> <a href='undocumented#Document'>topics</a>,
<a href='undocumented#Document'>a</a> <a href='undocumented#Document'>separate</a> <a href='undocumented#Document'>listing</a> <a href='undocumented#Document'>for</a> <a href='undocumented#Document'>constructors</a>, <a href='undocumented#Document'>one</a> <a href='undocumented#Document'>for</a> <a href='undocumented#Document'>methods</a>, <a href='undocumented#Document'>and</a> <a href='undocumented#Document'>so</a> <a href='undocumented#Document'>on</a>.

<a href='undocumented#Document'>Class</a> <a href='undocumented#Document'>methods</a> <a href='undocumented#Document'>contain</a> <a href='undocumented#Document'>a</a> <a href='undocumented#Document'>description</a>, <a href='undocumented#Document'>any</a> <a href='undocumented#Document'>parameters</a>, <a href='undocumented#Document'>any</a> <a href='undocumented#Document'>return</a> <a href='undocumented#Document'>value</a>,
<a href='undocumented#Document'>an</a> <a href='undocumented#Document'>example</a>, <a href='undocumented#Document'>and</a> <a href='undocumented#Document'>any</a> <a href='undocumented#Document'>cross</a> <a href='undocumented#Document'>references</a>.

<a href='undocumented#Document'>Each</a> <a href='undocumented#Document'>method</a> <a href='undocumented#Document'>must</a> <a href='undocumented#Document'>contain</a> <a href='undocumented#Document'>either</a> <a href='undocumented#Document'>one</a> <a href='undocumented#Document'>or</a> <a href='undocumented#Document'>more</a> <a href='undocumented#Document'>examples</a> <a href='undocumented#Document'>or</a> <a href='undocumented#Document'>markup</a> <a href='undocumented#Document'>indicating</a>
<a href='undocumented#Document'>that</a> <a href='undocumented#Document'>there</a> <a href='undocumented#Document'>is</a> <a href='undocumented#Document'>no</a> <a href='undocumented#Document'>example</a>.

<a href='undocumented#Document'>After</a> <a href='undocumented#Document'>editing</a> <a href='undocumented#Document'>is</a> <a href='undocumented#Document'>complete</a>, <a href='undocumented#Document'>searching</a> <a href='undocumented#Document'>for</a> "<a href='undocumented#Document'>incomplete</a>" <a href='undocumented#Document'>should</a> <a href='undocumented#Document'>fail</a>,
<a href='undocumented#Document'>assuming</a> "<a href='undocumented#Document'>incomplete</a>" <a href='undocumented#Document'>is</a> <a href='undocumented#Document'>not</a> <a href='undocumented#Document'>the</a> <a href='undocumented#Document'>perfect</a> <a href='undocumented#Document'>word</a> <a href='undocumented#Document'>to</a> <a href='undocumented#Document'>use</a> <a href='undocumented#Document'>in</a> <a href='undocumented#Document'>a</a> <a href='undocumented#Document'>description</a> <a href='undocumented#Document'>or</a>
<a href='undocumented#Document'>example</a>!

<a name='Adding_Documentation'></a>

Generate fiddle.json from all examples, including the ones you just wrote.
Error checking is syntatic: starting keywords are closed, keywords have the
correct parents.
If you run <a href='usingBookmaker#Bookmaker'>Bookmaker</a> <a href='usingBookmaker#Bookmaker'>inside</a> <a href='usingBookmaker#Bookmaker'>Visual_Studio</a>, <a href='usingBookmaker#Bookmaker'>you</a> <a href='usingBookmaker#Bookmaker'>can</a> <a href='usingBookmaker#Bookmaker'>click</a> <a href='usingBookmaker#Bookmaker'>on</a> <a href='usingBookmaker#Bookmaker'>errors</a> <a href='usingBookmaker#Bookmaker'>and</a> <a href='usingBookmaker#Bookmaker'>it</a>
<a href='usingBookmaker#Bookmaker'>will</a> <a href='usingBookmaker#Bookmaker'>take</a> <a href='usingBookmaker#Bookmaker'>you</a> <a href='usingBookmaker#Bookmaker'>to</a> <a href='usingBookmaker#Bookmaker'>the</a> <a href='usingBookmaker#Bookmaker'>source</a> <a href='undocumented#Line'>line</a> <a href='undocumented#Line'>in</a> <a href='undocumented#Line'>question</a>.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -e fiddle.json -b docs
</pre>

Once complete, run fiddlecli to generate the example hashes.
Errors are contained by the output but aren't reported yet.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ $GOPATH/bin/fiddlecli --input fiddle.json --output fiddleout.json
</pre>

Generate SkXXX.md from SkXXX.bmh and fiddleout.json.
Error checking includes: undefined references, fiddle compiler errors,
missing or mismatched printf output.
Again, you can click on any errors inside Visual_Studio.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -r site/user/api -b docs -f fiddleout.json
</pre>

The original include may have changed since you started creating the markdown.
Check to see if it is up to date.
This reports if a method no longer exists or its parameters have changed.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -x -b docs/SkXXX.bmh -i include/core/SkXXX.h
</pre>

Generate an updated include header. Run:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -p -b docs -i include/core/SkXXX.h
</pre>

to write the updated SkXXX.h to the current directory.

Once adding the file is complete, add the file to status.json in the
Completed section. You may add it to the InProgress section during
development, or leave status.json unchanged.

If the new file has been added to status.json, you can run
any of the above commands with -a docs/status.json in place of
-b docs or -i includes.

<a name='Bugs'></a>

<a href='usingBookmaker#Bookmaker'>Bookmaker</a> <a href='usingBookmaker#Bookmaker'>bugs</a> <a href='usingBookmaker#Bookmaker'>are</a> <a href='usingBookmaker#Bookmaker'>tracked</a>
<a href='https://bug.skia.org/6898'>here</a></a> .

