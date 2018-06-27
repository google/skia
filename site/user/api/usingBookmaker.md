usingBookmaker
===

# <a name="Bookmaker"></a> Bookmaker
How to use the <a href="#Bookmaker">Bookmaker</a> utility.

## <a name="Broken_Build"></a> Broken Build

If the <a href="https://status.skia.org/repo/skia?filter=search&search_value=Housekeeper-PerCommit-Bookmaker">Housekeeper-PerCommit-Bookmaker</a> bot is red, the bot has detected that the files in docs and include/core differ.

The bot output describes what changed.

To fix this, edit the docs file corresponding to the changed include file.

For instance, if the change was made to <a href="SkIRect_Reference#SkIRect">SkIRect</a>, edit docs/SkIRect_Reference.bmh.
Checking in the edited docs/SkIRect_Reference.bmh will fix the bot.

If the interface is deprecated, but still present in the interface, mark-up the
documentation to be deprecated as well.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
# void someMethodThatIsNowDeprecated()
#Deprecated
##</pre>

Use

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
## soon</pre>

if the change is soon to be deprecated.

To regenerate the documentation, follow the <a href="#Installing">Installing</a> and <a href="#Regenerate">Regenerate</a> steps below.

If the <a href="https://status.skia.org/repo/skia?filter=search&search_value=Housekeeper-Nightly-Bookmaker">Housekeeper-Nightly-Bookmaker</a> bot is red, one of several things may have gone wrong:

<table>  <tr>
    <td>A change to include broke documentation examples.</td>  </tr>  <tr>
    <td>Something changed the examples that output text.</td>  </tr>  <tr>
    <td>Some interface was added, deleted, edited.</td>  </tr>  <tr>
    <td>Documentation is malformed.</td>  </tr>
</table>

The bot output describes what changed, and includes the file and line
where the error occurred.

To regenerate the documentation, follow the <a href="#Installing">Installing</a> and <a href="#Regenerate">Regenerate</a> steps below.

## <a name="Editing_Comments"></a> Editing Comments

Edit docs instead of include/core files to update comments if possible.

The <a href="#Bookmaker">Bookmaker</a> bots do not complain if the docs file does not match the
corresponding include comments. Running <a href="#Bookmaker">Bookmaker</a> include generation will
report when docs and includes comments do not match.

For instance, if include/core/SkSurface.h comments do not match
docs/SkSurface_Reference.bmh, running:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -b docs -i include/core/SkSurface.h -p</pre>

generates

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
wrote updated <a href="SkSurface_Reference#SkSurface">SkSurface</a>.h</pre>

The updated SkSurface.h is written to the root to avoid subsequent runs of
<a href="#Bookmaker">Bookmaker</a> from recompiling. if SkSurface.h was not changed, it is not written,
and <a href="#Bookmaker">Bookmaker</a> will not generate any output.

## <a name="Broken_Example"></a> Broken Example

An example may cause <a href="#Bookmaker">Bookmaker</a> or a bot running <a href="#Bookmaker">Bookmaker</a> to fail if it can't
be compiled by fiddle. If the example cannot be fixed, it can be commented out
by changing

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#Example</pre>

to

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#NoExample</pre>

.
The disabled example can contain additional markup,
which will be ignored.

## <a name="Installing"></a> Installing

Install <a href="https://golang.org/doc/install">Go</a> if needed.
Get the fiddle command line interface tool.
By default this will appear in your home directory.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ go get go.skia.org/infra/fiddle/go/fiddlecli</pre>

Build <a href="#Bookmaker">Bookmaker</a>.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ninja -C out/dir bookmaker</pre>

## <a name="Regenerate"></a> Regenerate

Complete rebuilding of all bookmaker output looks like:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/skia/bookmaker -a docs/status.json -e fiddle.json
$ ~/go/bin/fiddlecli --input fiddle.json --output fiddleout.json
$ ./out/skia/bookmaker -a docs/status.json -f fiddleout.json -r site/user/api -c
$ ./out/skia/bookmaker -a docs/status.json -x
$ ./out/skia/bookmaker -a docs/status.json -p</pre>

## <a name="New_Documentation"></a> New Documentation

Generate an starter <a href="#Bookmaker">Bookmaker</a> file from an existing include.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -i include/core/SkXXX.h -t docs</pre>

If a method or function has an unnamed parameter, bookmaker generates an error:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
C:/puregit/include/core/<a href="SkPixmap_Reference#SkPixmap">SkPixmap</a>.h(208): error: # missing param name
bool erase(const SkColor4f&, const SkIRect* subset = nullptr) const
^
</pre>

All parameters require names to allow markdown and doxygen documents to refer to
them. After naming all parameters, check in the include before continuing.

A successful run generates

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
docs/SkXXX_Reference.bmh</pre>

.

Next, use your favorite editor to fill out

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
docs/SkXXX_Reference.bmh</pre>

.

## <a name="Style"></a> Style

Documentation consists of cross references, descriptions, and examples.
All structs, classes, enums, their members and methods, functions, and so on,
require descriptions. Most also require examples.

All methods and functions should include examples if practical.
It's difficult to think of a meaningful example for class destructors.
In cases like these, change the placeholder:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#Example</pre>

to:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#NoExample</pre>

Descriptions start with an active verb. Descriptions are complete, punctuated
sentences unless they describe parameters or return values. Parameters and
returned values are described by phrases that start lower case and do not
include trailing punctuation.

Descriptions are not self-referential; they do not include the thing they
describe. Descriptions may contain upper case or camel case references to
definitions but otherwise should be free of jargon.

Descriptions may contain code and formulas, each bracketed by markup.

Similar items may be grouped into topics. Topics may include subtopics.

Each document begins with one or more indices that include the contents of
that file. A class reference includes an index listing contained topics,
a separate listing for constructors, one for methods, and so on.

Class methods contain a description, any parameters, any return value,
an example, and any cross references.

Each method must contain either one or more examples or markup indicating
that there is no example.

After editing is complete, searching for "" should fail,
assuming "" is not the perfect word to use in a description or
example!

## <a name="Adding_Documentation"></a> Adding Documentation

Generate fiddle.json from all examples, including the ones you just wrote.
Error checking is syntatic: starting keywords are closed, keywords have the
correct parents.
If you run <a href="#Bookmaker">Bookmaker</a> inside Visual_Studio, you can click on errors and it
will take you to the source line in question.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -e fiddle.json -b docs</pre>

Once complete, run fiddlecli to generate the example hashes.
Errors are contained by the output but aren't reported yet.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ $GOPATH/bin/fiddlecli --input fiddle.json --output fiddleout.json</pre>

Generate SkXXX.md from SkXXX.bmh and fiddleout.json.
Error checking includes: undefined references, fiddle compiler errors,
missing or mismatched printf output.
Again, you can click on any errors inside Visual_Studio.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -r site/user/api -b docs -f fiddleout.json</pre>

The original include may have changed since you started creating the markdown.
Check to see if it is up to date.
This reports if a method no longer exists or its parameters have changed.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -x -b docs/SkXXX.bmh -i include/core/SkXXX.h</pre>

Generate an updated include header. Run:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -p -b docs -i include/core/SkXXX.h</pre>

to write the updated SkXXX.h to the current directory.

Once adding the file is complete, add the file to status.json in the
Completed section. You may add it to the InProgress section during
development, or leave status.json unchanged.

If the new file has been added to status.json, you can run
any of the above commands with -a docs/status.json in place of
-b docs or -i includes.

## <a name="Bugs"></a> Bugs

<a href="#Bookmaker">Bookmaker</a> bugs are tracked <a href="bug.skia.org/6898">here</a> .
