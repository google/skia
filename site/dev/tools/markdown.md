Markdown
========

This site can handle normal MarkDown and many common extensions. To learn
how the following is done please see the [source for this page](./markdown.md).
Any file you put under `/site/` that has the extension `.md` will be processed
as MarkDown. All other files will be served directly. For example, images
can be added and they will be served correctly and referenced from within MarkDown files.

When preparing for a code review of site docs you can get a preview of how the
page will render by visiting the skia.org site and add a query parameter `cl`
with the value of the Reitveld issue id:

    https://skia.org/path/to/markdown-file?cl=REITVELD_ISSUE_NUMBER

You can also run a local copy of the documentation server, which will allow
you to preview changes much quicker. You must have [Go](https://golang.org)
installed on your computer, which you will have if you are running on a Google
corporate workstation. Run:

    go get -u skia.googlesource.com/buildbot.git/doc/go/docserver

And then **from within** the directory of your local Git checkout of Skia run:

    docserver --preview

Then visit http://localhost:8000 to preview your changes. There is no need to
restart the server for file changes, but you will need to restart it if there
are changes to the navigation menu, i.e. you add or remove a file and want it
to appear in the navigation on the right hand side of the page.

If port 8000 is unavailable on your machine you can set the port to use via
the --port flag:

    docserver --preview --port=:8002

METADATA
--------

By default all files and directories that appear in the same level are sorted
alphabetically by file name in the navigation menu, with files appearing
before directories. You can override this default behavior by adding a
METADATA file to a directory. A METADATA file is a JSON file of the following
format:

~~~~
   {
     "dirOrder": ["sample", "quick", "special"],
     "fileOrder": ["download", "api"]
   }
~~~~

If a file or directory doesn't appear in `dirOrder` or `fileOrder` then it is sorted
to appear after the members of `dirOrder` or `fileOrder` respectively. All
files and directories that aren't controlled by a METADATA file are sorted in
alphabetical order by their filename.

Some Example MarkDown
---------------------

This is a [link](https://www.google.com). You can also create
ordered and unordered lists:

1. First
2. Second:
  * Fee
    * Fie
      * Foe
  * Fum
3. Third

Incorporate images:

![image](image.png)

Or go old school and use [ASCII art](http://asciiflow.com/):

~~~~

       +----------------+
       |  Should you    |
    +--+ use ASCII art? +--+
    |  +----------------+  |
    |                      |
+---v---+              +---v---+
|       |              |       |
|  Yes  |              |  No   |
|       |              |       |
+-------+              +-------+

~~~~

Format code snippets or other preformatted text. Just surround the code
with `~~~~`. You can also trigger syntax highlighting by putting in
the following HTML comment before the code section:

    <!--?prettify?-->


<!--?prettify?-->
~~~~
class SK_API SkPaint {
public:
    SkPaint();
    SkPaint(const SkPaint& paint);
    ~SkPaint();

    SkPaint& operator=(const SkPaint&);
~~~~


Tables

  Name    | Value    | Summary
  --------|----------|--------
  A       | 27       |  yes
  B       | 23       |  no


There are also inline styles for *emphasis*, **bold**,
~~strikethrough~~, and `inline code`. Also small fractions,
such as 1/2 are rendered nicely.

> # There are
> ## Headers
> ### Up To
> #### Six
> ##### Levels
> ###### Deep

And you can <b>always</b> use HTML, which is useful for features that can't be
done in MarkDown, such as iframes, but try to avoid using HTML outside of
sitations like that.
<svg viewBox="0 0 24 24" height="24px" width="24px" style="display: inline;">
  <g>
    <path d="M1
    21h4v-12h-4v12zm22-11c0-1.1-.9-2-2-2h-6.31l.95-4.57.03-.32c0-.41-.17-.79-.44-1.06l-1.06-1.05-6.58
    6.59c-.37.36-.59.86-.59 1.41v10c0 1.1.9 2 2 2h9c.83 0 1.54-.5
    1.84-1.22l3.02-7.05c.09-.23.14-.47.14-.73v-1.91l-.01-.01.01-.08z"> </path>
  </g>
</svg>

Reference
=========

Below is a longer reference on the MarkDown that docserver accepts.

Paragraphs
----------

A paragraph is simply one or more consecutive lines of text, separated
by one or more blank lines. (A blank line is any line that looks like a
blank line -- a line containing nothing spaces or tabs is considered
blank.) Normal paragraphs should not be intended with spaces or tabs.

Headers and Blockquotes
-----------------------

You can create headers by either "underlining" with equal signs (`=`) and hyphens (`-`),
or,you can put 1-6 hash marks (`#`) at the
beginning of the line -- the number of hashes equals the resulting
HTML header level.

Blockquotes are indicated using email-style '`>`' angle brackets.

Markdown:

    A First Level Header
    ====================
    
    A Second Level Header
    ---------------------

    Now is the time for all good men to come to
    the aid of their country. This is just a
    regular paragraph.

    The quick brown fox jumped over the lazy
    dog's back.
    
    ### Header 3

    > This is a blockquote.
    > 
    > This is the second paragraph in the blockquote.
    >
    > ## This is an H2 in a blockquote


Output:

    <h1>A First Level Header</h1>
    
    <h2>A Second Level Header</h2>
    
    <p>Now is the time for all good men to come to
    the aid of their country. This is just a
    regular paragraph.</p>
    
    <p>The quick brown fox jumped over the lazy
    dog's back.</p>
    
    <h3>Header 3</h3>
    
    <blockquote>
        <p>This is a blockquote.</p>
        
        <p>This is the second paragraph in the blockquote.</p>
        
        <h2>This is an H2 in a blockquote</h2>
    </blockquote>



### Phrase Emphasis ###

Markdown uses asterisks and underscores to indicate spans of emphasis.

Markdown:

    Some of these words *are emphasized*.
    Some of these words _are emphasized also_.
    
    Use two asterisks for **strong emphasis**.
    Or, if you prefer, __use two underscores instead__.

Output:

    <p>Some of these words <em>are emphasized</em>.
    Some of these words <em>are emphasized also</em>.</p>
    
    <p>Use two asterisks for <strong>strong emphasis</strong>.
    Or, if you prefer, <strong>use two underscores instead</strong>.</p>
   


## Lists ##

Unordered (bulleted) lists use asterisks, pluses, and hyphens (`*`,
`+`, and `-`) as list markers. These three markers are
interchangable; this:

    *   Candy.
    *   Gum.
    *   Booze.

this:

    +   Candy.
    +   Gum.
    +   Booze.

and this:

    -   Candy.
    -   Gum.
    -   Booze.

all produce the same output:

    <ul>
    <li>Candy.</li>
    <li>Gum.</li>
    <li>Booze.</li>
    </ul>

Ordered (numbered) lists use regular numbers, followed by periods, as
list markers:

    1.  Red
    2.  Green
    3.  Blue

Output:

    <ol>
    <li>Red</li>
    <li>Green</li>
    <li>Blue</li>
    </ol>

If you put blank lines between items, you'll get `<p>` tags for the
list item text. You can create multi-paragraph list items by indenting
the paragraphs by 4 spaces or 1 tab:

    *   A list item.
    
        With multiple paragraphs.

    *   Another item in the list.

Output:

    <ul>
    <li><p>A list item.</p>
    <p>With multiple paragraphs.</p></li>
    <li><p>Another item in the list.</p></li>
    </ul>
    


### Links ###

Markdown supports two styles for creating links: *inline* and
*reference*. With both styles, you use square brackets to delimit the
text you want to turn into a link.

Inline-style links use parentheses immediately after the link text.
For example:

    This is an [example link](http://example.com/).

Output:

    <p>This is an <a href="http://example.com/">
    example link</a>.</p>

Optionally, you may include a title attribute in the parentheses:

    This is an [example link](http://example.com/ "With a Title").

Output:

    <p>This is an <a href="http://example.com/" title="With a Title">
    example link</a>.</p>

Reference-style links allow you to refer to your links by names, which
you define elsewhere in your document:

    I get 10 times more traffic from [Google][1] than from
    [Yahoo][2] or [MSN][3].

    [1]: http://google.com/        "Google"
    [2]: http://search.yahoo.com/  "Yahoo Search"
    [3]: http://search.msn.com/    "MSN Search"

Output:

    <p>I get 10 times more traffic from <a href="http://google.com/"
    title="Google">Google</a> than from <a href="http://search.yahoo.com/"
    title="Yahoo Search">Yahoo</a> or <a href="http://search.msn.com/"
    title="MSN Search">MSN</a>.</p>

The title attribute is optional. Link names may contain letters,
numbers and spaces, but are *not* case sensitive:

    I start my morning with a cup of coffee and
    [The New York Times][NY Times].

    [ny times]: http://www.nytimes.com/

Output:

    <p>I start my morning with a cup of coffee and
    <a href="http://www.nytimes.com/">The New York Times</a>.</p>


### Images ###

Image syntax is very much like link syntax.

Inline (titles are optional):

    ![alt text](/path/to/img.jpg "Title")

Reference-style:

    ![alt text][id]

    [id]: /path/to/img.jpg "Title"

Both of the above examples produce the same output:

    <img src="/path/to/img.jpg" alt="alt text" title="Title" />



### Code ###

In a regular paragraph, you can create code span by wrapping text in
backtick quotes. Any ampersands (`&`) and angle brackets (`<` or
`>`) will automatically be translated into HTML entities. This makes
it easy to use Markdown to write about HTML example code:

    I strongly recommend against using any `<blink>` tags.

    I wish SmartyPants used named entities like `&mdash;`
    instead of decimal-encoded entites like `&#8212;`.

Output:

    <p>I strongly recommend against using any
    <code>&lt;blink&gt;</code> tags.</p>
    
    <p>I wish SmartyPants used named entities like
    <code>&amp;mdash;</code> instead of decimal-encoded
    entites like <code>&amp;#8212;</code>.</p>


To specify an entire block of pre-formatted code, indent every line of
the block by 4 spaces or 1 tab. Just like with code spans, `&`, `<`,
and `>` characters will be escaped automatically.

Markdown:

    If you want your page to validate under XHTML 1.0 Strict,
    you've got to put paragraph tags in your blockquotes:

        <blockquote>
            <p>For example.</p>
        </blockquote>

Output:

    <p>If you want your page to validate under XHTML 1.0 Strict,
    you've got to put paragraph tags in your blockquotes:</p>
    
    <pre><code>&lt;blockquote&gt;
        &lt;p&gt;For example.&lt;/p&gt;
    &lt;/blockquote&gt;
    </code></pre>
