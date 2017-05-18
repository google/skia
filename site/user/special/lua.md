Skia Lua Bindings
=================

**Warning: The following has only been tested on Mac and Linux, but it will
likely work for any Unix.**

Prerequisites
-------------

This assumes you already have Skia building normally. If not, refer to [How to
build Skia](../build).

Build
-----

To build Lua support into Skia tools, set the GN argument `skia_use_lua` to `true`.
Optionally, set `skia_use_system_lua`.  Then re-run GN.


Try It Out
----------

The tools `lua_app` and `lua_pictures` should now be available when you compile,
and `SampleApp` should now have a `Lua` sample.


To-Do
-----

Skia had a feature that let it be imported as an .so by Lua.
This feature is not yet supported by GN, but would have looked something like this:

    $ lua

    Lua 5.2.0  Copyright (C) 1994-2011 Lua.org, PUC-Rio
    > require 'skia'
    > paint = Sk.newPaint()
    > paint:setColor{a=1, r=1, g=0, b=0}
    > doc = Sk.newDocumentPDF('test.pdf')
    > canvas = doc:beginPage(72*8.5, 72*11)
    > canvas:drawText('Hello Lua', 300, 300, paint)
    > doc:close()
