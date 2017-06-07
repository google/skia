
# <a name="Bookmaker"></a> Bookmaker
How to use the <a href="bmh_usingBookmaker?cl=9919#Bookmaker">Bookmaker</a> utility.
Get the fiddle command line interface tool.

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
$ go get go.skia.org/infra/fiddle/go/fiddlecli</pre>

Get the <a href="bmh_usingBookmaker?cl=9919#Bookmaker">Bookmaker</a> <a href="bmh_usingBookmaker?cl=9919#CL">CL</a> and build it.

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
$ git cl patch 9919
$ ninja -<a href="bmh_usingBookmaker?cl=9919#C">C</a> out/dir bookmaker</pre>

Generate an starter <a href="bmh_usingBookmaker?cl=9919#Bookmaker">Bookmaker</a> file from an existing include.
This writes out/dir/obj/<a href="bmh_usingBookmaker?cl=9919#SkXXX">SkXXX</a>.bmh.
Known bugs: struct inside class isn't transcribed.
            @param @return aren't transcribed.
            doxygen //!< comments aren't associated with content
            lines of slashes should be removed

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -t -i include/core/<a href="bmh_usingBookmaker?cl=9919#SkXXX">SkXXX</a>.h</pre>

Use your favorite editor to fill out <a href="bmh_usingBookmaker?cl=9919#SkXXX">SkXXX</a>.bmh.
Generate a fiddle json from all examples, including the ones you just wrote.
Error checking is syntatic: starting keywords are closed, keywords have the
correct parents.
If you run <a href="bmh_usingBookmaker?cl=9919#Bookmaker">Bookmaker</a> inside <a href="bmh_usingBookmaker?cl=9919#Visual_Studio">Visual Studio</a>, you can click on errors and it
will take you to the source line in question.
This writes out/dir/obj/fiddle.json.

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -e -b out/dir/obj</pre>

Once complete, run fiddlecli to generate the example hashes.
Errors are contained by the output but aren't reported yet.

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
$ $GOPATH/bin/fiddlecli --input out/dir/obj/fiddle.json --output \
   experimental/docs/fiddleout.json</pre>

<a href="bmh_usingBookmaker?cl=9919#Bookmaker">Bookmaker</a> will take the fiddlecli output and generate markdown files.
writing out/dir/obj/<a href="bmh_usingBookmaker?cl=9919#bmh_SkXXX">bmh SkXXX</a>.md.
Error checking includes: undefined references, fiddle compiler errors,
missing or mismatched printf output.
Again, you can click on any errors inside <a href="bmh_usingBookmaker?cl=9919#Visual_Studio">Visual Studio</a>.

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -r -b out/dir/obj -f experimental/docs/fiddleout.json</pre>

The original include may have changed since you started working on this.
Check to see if it is up to date.
This reports if a method no longer exists or its parameters have changed.

<pre style="padding: 1em 1em 1em 1em;width: 44em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -x -b out/dir/obj/<a href="bmh_usingBookmaker?cl=9919#SkXXX">SkXXX</a>.bmh -i include/core/<a href="bmh_usingBookmaker?cl=9919#SkXXX">SkXXX</a>.h</pre>

# <a name="Bugs"></a> Bugs
overaggressive reference finding in code block
missing examples
redundant examples -- got tired so used the same one more than once
some examples need vertical resizing
list doesn't work (ironic, huh)

# <a name="To_Do"></a> To Do
check that all methods have one line descriptions in overview
see also -- anything that can be done automatically? maybe any ref shows up everywhere
index by example png
generate pdf or pdf-like out
generate b/w out instead of color -- have b/w versions of examples?
formalize voice / syntax for parts of topic and method
write bmh data back into include
    have a way to write one block that covers multiple nearly indentical methods?
    may want to do this for pdf view as well
write a one-method-per-page online view?