Skia Lua Bindings
=================

**Warning: The following has only been tested on Linux, but it will likely
work for any Unix.**

Prerequisites
-------------

This assumes one already has Skia building normally. If not, refer to the
quick start guides. In addition to that, you will need Lua 5.2 installed on
your system in order to use the bindings.

Building lua requires the readline development library. If missing this can be installed (on Ubuntu) by executing:

  * `apt-cache search libreadline` to see the available libreadline libraries
  * `sudo apt-get install libreadline6 libreadline6-dev` to actually install the libraries

Build
-----

The build process starts the same way as described in the quick starts, but
before using gyp or make, do this instead:

    $ export GYP_DEFINES="skia_shared_lib=1"
    $ make tools

This tells Skia to build as a shared library, which enables a build of another shared library called 'skia.so' that exposes Skia bindings to Lua.

Try It Out
----------

Once the build is complete, use the same terminal:

    $ cd out/Debug/
    $ lua

    Lua 5.2.0  Copyright (C) 1994-2011 Lua.org, PUC-Rio
    > require 'skia'
    > paint = Sk.newPaint()
    > paint:setColor{a=1, r=1, g=0, b=0}
    > doc = Sk.newDocumentPDF('test.pdf')
    > canvas = doc:beginPage(72*8.5, 72*11)
    > canvas:drawText('Hello Lua', 300, 300, paint)
    > doc:close()

The key part to loading the bindings is `require 'skia'` which tells lua to look
for 'skia.so' in the current directory (among many others) and provides the
bindings. 'skia.so' in turn will load 'libskia.so' from the current directory or
in our case the lib.target directory. 'libskia.so' is what contains the native
skia code. The script shown above uses skia to draw Hello Lua in red text onto
a pdf that will be outputted into the current folder as 'test.pdf'. Go ahead and
open 'test.pdf' to confirm that everything is working.

