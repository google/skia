usingBookmaker
===

# <a name="Bookmaker"></a> Bookmaker
How to use the <a href="usingBookmaker#Bookmaker">Bookmaker</a> utility.

Get the fiddle command line interface tool.

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
$ go get go.skia.org/infra/fiddle/go/fiddlecli</pre>

Build <a href="usingBookmaker#Bookmaker">Bookmaker</a>.

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
$ ninja -<a href="usingBookmaker#C">C</a> out/dir bookmaker</pre>

Generate an starter <a href="usingBookmaker#Bookmaker">Bookmaker</a> file from an existing include.
This writes <a href="usingBookmaker#SkXXX">SkXXX</a>.bmh in the current directory, which is
out/dir/obj/ from an IDE.

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -t -i include/core/<a href="usingBookmaker#SkXXX">SkXXX</a>.h</pre>

Copy <a href="usingBookmaker#SkXXX">SkXXX</a>.bmh to docs.
Use your favorite editor to fill out docs/<a href="usingBookmaker#SkXXX">SkXXX</a>.bmh.

Generate fiddle.json from all examples, including the ones you just wrote.
Error checking is syntatic: starting keywords are closed, keywords have the
correct parents.
If you run <a href="usingBookmaker#Bookmaker">Bookmaker</a> inside <a href="usingBookmaker#Visual_Studio">Visual Studio</a>, you can click on errors and it
will take you to the source line in question.

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -e fiddle.json -b docs</pre>

Once complete, run fiddlecli to generate the example hashes.
Errors are contained by the output but aren't reported yet.

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
$ $GOPATH/bin/fiddlecli --input fiddle.json --output fiddleout.json</pre>

Generate <a href="usingBookmaker#bmh_SkXXX">bmh SkXXX</a>.md from <a href="usingBookmaker#SkXXX">SkXXX</a>.bmh and fiddleout.json.
Error checking includes: undefined references, fiddle compiler errors,
missing or mismatched printf output.
Again, you can click on any errors inside <a href="usingBookmaker#Visual_Studio">Visual Studio</a>.

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -r site/user/api -b docs -f fiddleout.json</pre>

The original include may have changed since you started creating the markdown.
Check to see if it is up to date.
This reports if a method no longer exists or its parameters have changed.

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -x -b docs/<a href="usingBookmaker#SkXXX">SkXXX</a>.bmh -i include/core/<a href="usingBookmaker#SkXXX">SkXXX</a>.h</pre>

Generate an updated include header.
This writes the updated <a href="undocumented#SkXXX.h">SkXXX.h</a> to the current directory.

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -p -b docs -i include/core/<a href="usingBookmaker#SkXXX">SkXXX</a>.h</pre>

## <a name="Bugs"></a> Bugs

<a href="usingBookmaker#Bookmaker">Bookmaker</a> bugs are trackedherebug.skia.org/6898.
