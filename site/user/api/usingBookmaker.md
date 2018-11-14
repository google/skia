usingBookmaker
===
<a href='usingBookmaker#Bookmaker'>Bookmaker</a> generates markdown files to view documentation on skia.org, and generates includes for use in C++.
<a href='usingBookmaker#Bookmaker'>Bookmaker</a> reads canonical documentation from files suffixed with bmh in the docs directory. These bmh
files describe how public interfaces work, and generate Skia fiddle examples to illustrate them.

The docs files must be manually edited to stay current with Skia as it evolves.

<a name='Installing'></a>

Install
<a href='https://golang.org/doc/install'>Go</a></a> if needed.
Check the version. The results should be 1.10 or greater.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ go version
</pre>

Get the fiddle command <a href='undocumented#Line'>line</a> interface tool.
By default this will appear in your home directory.

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

<a href='usingBookmaker#Bookmaker'>Bookmaker</a> extracts examples, generates example hashes with fiddle, and generates web markdown
and c++ includes.

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
<a href='https://status.skia.org/repo/skia?filter=search&search_value=Housekeeper-Nightly-Bookmaker'>Housekeeper-Nightly-Bookmaker</a></a> verify that <a href='usingBookmaker#Bookmaker'>Bookmaker</a> <a href='undocumented#Data'>data</a> in docs builds without error and is consistent with include files it documents.

Possible failures include:

<table>  <tr>
    <td>Public interface in include directory does not match documented interface in docs directory.</td>
  </tr>  <tr>
    <td>Example in <a href='usingBookmaker#Bookmaker'>bookmaker</a> bmh file does not compile, or does not produce expected output.</td>
  </tr>  <tr>
    <td>Undocumented but referenced interface is missing from undocumented <a href='usingBookmaker#Bookmaker'>bookmaker</a> file in docs directory.</td>
  </tr>
</table>

Editing comments in includes or editing private interfaces will not break the bots.
<a href='usingBookmaker#Bookmaker'>Bookmaker</a> detects that comments edited in includes do not match comments in docs; it will generate an updated include in the
directory where it is run.

If
<a href='https://status.skia.org/repo/skia?filter=search&search_value=Housekeeper-PerCommit-Bookmaker'>Housekeeper-PerCommit-Bookmaker</a></a> bot is red, the error is usually related to an edit to an include which has not been reflected in docs.

To fix this, edit the docs file corresponding to the changed include file.

For instance, if the change was made to <a href='SkIRect_Reference#SkIRect'>SkIRect</a>, edit docs/SkIRect_Reference.bmh.
Checking in the edited docs/SkIRect_Reference.bmh will fix the bot.

If the interface is deprecated, private, or experimental, documentation is not
required. Put the word "Deprecated", "Private", or "Experimental"; upper or lower
case, in a comment just before the symbol to be ignored.

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
where the error occurred.

To regenerate the documentation, follow the Installing and Regenerate steps below.

<a name='Editing_Comments'></a>

Edit docs instead of include/core files to update comments if possible.

The <a href='usingBookmaker#Bookmaker'>Bookmaker</a> bots do not complain if the docs file does not match the
corresponding include comments. Running <a href='usingBookmaker#Bookmaker'>Bookmaker</a> include generation will
report when docs and includes comments do not match.

For instance, if include/core/SkSurface.h comments do not match
docs/SkSurface_Reference.bmh, running:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -b docs -i include/core/SkSurface.h -p
</pre>

generates

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
wrote updated <a href='SkSurface_Reference#SkSurface'>SkSurface</a>.h
</pre>

The updated <a href='SkSurface_Reference#SkSurface'>SkSurface</a>.h is written to the root to avoid subsequent runs of
<a href='usingBookmaker#Bookmaker'>Bookmaker</a> from recompiling. if <a href='SkSurface_Reference#SkSurface'>SkSurface</a>.h was not changed, it is not written,
and <a href='usingBookmaker#Bookmaker'>Bookmaker</a> will not generate any output.

<a name='Broken_Example'></a>

An example may cause <a href='usingBookmaker#Bookmaker'>Bookmaker</a> or a bot running <a href='usingBookmaker#Bookmaker'>Bookmaker</a> to fail if it fails to compile.

Fix the example by pasting it into <a href='https://fiddle.skia.org'>Skia Fiddle</a></a> and editing it until it runs successfully.

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

Complete rebuilding of all <a href='usingBookmaker#Bookmaker'>bookmaker</a> output looks like:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -a docs/status.json -e fiddle.json
$ ~/go/bin/fiddlecli --input fiddle.json --output fiddleout.json
$ ./out/dir/bookmaker -a docs/status.json -f fiddleout.json -r site/user/api -c
$ ./out/dir/bookmaker -a docs/status.json -f fiddleout.json -r site/user/api
$ ./out/dir/bookmaker -a docs/status.json -x
$ ./out/dir/bookmaker -a docs/status.json -p
</pre>

<a name='New_Documentation'></a>

Generate an starter <a href='usingBookmaker#Bookmaker'>Bookmaker</a> file from an existing include.

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -i include/core/SkXXX.h -t docs
</pre>

If a method or function has an unnamed parameter, <a href='usingBookmaker#Bookmaker'>bookmaker</a> generates an error:

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
C:/puregit/include/core/SkPixmap.h(208): error: #Method missing param name
bool erase(const <a href='SkColor4f_Reference#SkColor4f'>SkColor4f</a>&, const <a href='SkIRect_Reference#SkIRect'>SkIRect</a>* subset = nullptr) const
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

Each <a href='undocumented#Document'>document</a> begins with one or more indices that include the contents of
that file. A class reference includes an index listing contained topics,
a separate listing for constructors, one for methods, and so on.

Class methods contain a description, any parameters, any return value,
an example, and any cross references.

Each method must contain either one or more examples or markup indicating
that there is no example.

After editing is complete, searching for "incomplete" should fail,
assuming "incomplete" is not the perfect word to use in a description or
example!

<a name='Adding_Documentation'></a>

Generate fiddle.json from all examples, including the ones you just wrote.
Error checking is syntatic: starting keywords are closed, keywords have the
correct parents.
If you run <a href='usingBookmaker#Bookmaker'>Bookmaker</a> inside Visual_Studio, you can click on errors and it
will take you to the source <a href='undocumented#Line'>line</a> in question.

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

<a href='usingBookmaker#Bookmaker'>Bookmaker</a> bugs are tracked
<a href='https://bug.skia.org/6898'>here</a></a> .

