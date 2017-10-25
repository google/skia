usingBookmaker
===

# <a name="Bookmaker"></a> Bookmaker
How to use the <a href="#Bookmaker">Bookmaker</a> utility.

Install<a href="usingBookmaker#Go">Go</a>if needed.  
Get the fiddle command line interface tool.
By default this will appear in your home directory.

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
$ go get go.skia.org/infra/fiddle/go/fiddlecli</pre>

Build <a href="#Bookmaker">Bookmaker</a>.

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
$ ninja -<a href="undocumented#C">C</a> out/dir bookmaker</pre>

Generate an starter <a href="#Bookmaker">Bookmaker</a> file from an existing include.

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -i include/core/<a href="undocumented#SkXXX.h">SkXXX.h</a> -t docs</pre>

If a method or function has an unnamed parameter, bookmaker generates an error:

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
<a href="undocumented#C">C</a>:/puregit/include/core/<a href="SkPixmap_Reference#SkPixmap">SkPixmap</a>.h(208): error: # missing param name
bool erase(const SkColor4f&, const SkIRect* subset = nullptr) const
^
</pre>

All parameters require names to allow markdown and doxygen documents to refer to
them. After naming all parameters, check in the include before continuing.

A successful run generates

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
docs/<a href="undocumented#SkXXX_Reference">SkXXX Reference</a>.bmh</pre>

.

Next, use your favorite editor to fill out

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
docs/<a href="undocumented#SkXXX_Reference">SkXXX Reference</a>.bmh</pre>

.

## <a name="Style"></a> Style

Documentation consists of cross references, descriptions, and examples.
All structs, classes, enums, their members and methods, functions, and so on,
require descriptions. Most also require examples.

All methods and functions should include examples if practical.

Descriptions start with an active verb. Descriptions are complete, punctuated
sentences unless they describe parameters or return values. Parameters and
returned values are described by phrases that start lower case and do not
include trailing punctuation.

Descriptions are not self-referential; they do not include the thing they
describe. Descriptions may contain upper case references to definitions
but otherwise should be free of jargon.

Descriptions may contain code and formulas, each bracketed by markup.

Similar items may be grouped into topics. Topics may include subtopics.

Each document begins with one or more indices that include the contents of
that file. A class reference includes an index listing contained topics, 
a separate listing for constructors, one for methods, and so on.

Class methods contain a description, any parameters, any return value,
an example, and any cross references.

Each method must contain either one or more examples or markup indicating
that there is no example.
Generate fiddle.json from all examples, including the ones you just wrote.
Error checking is syntatic: starting keywords are closed, keywords have the
correct parents.
If you run <a href="#Bookmaker">Bookmaker</a> inside <a href="usingBookmaker#Visual_Studio">Visual Studio</a>, you can click on errors and it
will take you to the source line in question.

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -e fiddle.json -b docs</pre>

Once complete, run fiddlecli to generate the example hashes.
Errors are contained by the output but aren't reported yet.

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
$ $GOPATH/bin/fiddlecli --input fiddle.json --output fiddleout.json</pre>

Generate <a href="usingBookmaker#bmh_SkXXX">bmh SkXXX</a>.md from <a href="usingBookmaker#SkXXX">SkXXX</a>.bmh and fiddleout.json.
Error checking includes: undefined references, fiddle compiler errors,
missing or mismatched printf output.
Again, you can click on any errors inside <a href="usingBookmaker#Visual_Studio">Visual Studio</a>.

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -r site/user/api -b docs -f fiddleout.json</pre>

The original include may have changed since you started creating the markdown.
Check to see if it is up to date.
This reports if a method no longer exists or its parameters have changed.

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -x -b docs/<a href="usingBookmaker#SkXXX">SkXXX</a>.bmh -i include/core/<a href="usingBookmaker#SkXXX">SkXXX</a>.h</pre>

Generate an updated include header. Run:

<pre style="padding: 1em 1em 1em 1em;width: 50em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -p -b docs -i include/core/<a href="usingBookmaker#SkXXX">SkXXX</a>.h</pre>

to write the updated <a href="undocumented#SkXXX.h">SkXXX.h</a> to the current directory.

## <a name="Bugs"></a> Bugs

<a href="#Bookmaker">Bookmaker</a> bugs are trackedhere.
