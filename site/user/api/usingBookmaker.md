usingBookmaker
===
<a href='#Bookmaker'>Bookmaker</a> generates markdown files to view documentation on skia

## <a name='Broken_Build'>Broken Build</a>

The bots <a href='https://status.skia.org/repo/skia?filter=search&search_value=Housekeeper-PerCommit-Bookmaker'>Housekeeper-PerCommit-Bookmaker</a></a> and <a href='https://status.skia.org/repo/skia?filter=search&search_value=Housekeeper-Nightly-Bookmaker'>Housekeeper-Nightly-Bookmaker</a></a> verify that <a href='#Bookmaker'>Bookmaker</a> data in docs builds without error and is consistent with include files it documents

<table>  <tr>
    <td>Public interface in include directory does not match documented interface in docs directory</td>
  </tr>  <tr>
    <td>Example in bookmaker bmh file does not compile</td>
  </tr>  <tr>
    <td>Undocumented but referenced interface is missing from undocumented bookmaker file in docs directory</td>
  </tr>
</table>

Editing comments in includes or editing private interfaces will not break the bots <a href='https://status.skia.org/repo/skia?filter=search&search_value=Housekeeper-PerCommit-Bookmaker'>Housekeeper-PerCommit-Bookmaker</a></a> bot is red

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#Method void someMethodThatIsNowDeprecated()
#Deprecated
##
</pre>

Use

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#Deprecated soon
</pre>

if the change is soon to be deprecatedIf <a href='https://status.skia.org/repo/skia?filter=search&search_value=Housekeeper-Nightly-Bookmaker'>Housekeeper-Nightly-Bookmaker</a></a> bot is red

<table>  <tr>
    <td>A change to include broke documentation examples</td>
  </tr>  <tr>
    <td>Something changed the examples that output text</td>
  </tr>  <tr>
    <td>Some interface was added</td>
  </tr>  <tr>
    <td>Documentation is malformed</td>
  </tr>
</table>

The bot output describes what changed

## <a name='Editing_Comments'>Editing Comments</a>

Edit docs instead of include

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -b docs -i include/core/SkSurface.h -p
</pre>

generates

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
wrote updated <a href='SkSurface_Reference#SkSurface'>SkSurface</a>.h
</pre>

The updated SkSurface.h is written to the root to avoid subsequent runs of
<a href='#Bookmaker'>Bookmaker</a> from recompiling

## <a name='Broken_Example'>Broken Example</a>

An example may cause <a href='#Bookmaker'>Bookmaker</a> or a bot running <a href='#Bookmaker'>Bookmaker</a> to fail if it fails to compile <a href='https://fiddle.skia.org'>Skia Fiddle</a></a> and editing it until it runs successfully

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#Example
</pre>

to

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#NoExample
</pre>

## <a name='Installing'>Installing</a>

Install <a href='https://golang.org/doc/install'>Go</a></a> if needed

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ go version
</pre>

Get the fiddle command line interface tool

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ go get go.skia.org/infra/fiddlek/go/fiddlecli
</pre>

Build <a href='#Bookmaker'>Bookmaker</a>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ninja -C out/dir bookmaker
</pre>

## <a name='Regenerate'>Regenerate</a>

Complete rebuilding of all bookmaker output looks like

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -a docs/status.json -e fiddle.json
$ ~/go/bin/fiddlecli --input fiddle.json --output fiddleout.json
$ ./out/dir/bookmaker -a docs/status.json -f fiddleout.json -r site/user/api -c
$ ./out/dir/bookmaker -a docs/status.json -f fiddleout.json -r site/user/api
$ ./out/dir/bookmaker -a docs/status.json -x
$ ./out/dir/bookmaker -a docs/status.json -p
</pre>

## <a name='New_Documentation'>New Documentation</a>

Generate an starter <a href='#Bookmaker'>Bookmaker</a> file from an existing include

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -i include/core/SkXXX.h -t docs
</pre>

If a method or function has an unnamed parameter

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
C:/puregit/include/core/<a href='SkPixmap_Reference#SkPixmap'>SkPixmap</a>.h(208): error: #Method missing param name
bool erase(const SkColor4f&, const SkIRect* subset = nullptr) const
           ^
</pre>

All parameters require names to allow markdown and doxygen documents to refer to
them

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
docs/SkXXX_Reference.bmh
</pre>

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
docs/SkXXX_Reference.bmh
</pre>

## <a name='Style'>Style</a>

Documentation consists of cross references

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#Example
</pre>

to

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
#NoExample
</pre>

Descriptions start with an active verb

## <a name='Adding_Documentation'>Adding Documentation</a>

Generate fiddle

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -e fiddle.json -b docs
</pre>

Once complete

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ $GOPATH/bin/fiddlecli --input fiddle.json --output fiddleout.json
</pre>

Generate SkXXX

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -r site/user/api -b docs -f fiddleout.json
</pre>

The original include may have changed since you started creating the markdown

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -x -b docs/SkXXX.bmh -i include/core/SkXXX.h
</pre>

Generate an updated include header

<pre style="padding: 1em 1em 1em 1em;width: 62.5em; background-color: #f0f0f0">
$ ./out/dir/bookmaker -p -b docs -i include/core/SkXXX.h
</pre>

to write the updated SkXXX.h to the current directory

## <a name='Bugs'>Bugs</a>

<a href='#Bookmaker'>Bookmaker</a> bugs are tracked <a href='https://bug.skia.org/6898'>here</a></a> 