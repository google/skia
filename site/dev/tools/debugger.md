Skia Debugger
=============

Introduction
------------

The Skia Debugger is a graphical tool used to step through and analyze the
contents of the Skia picture format. The tool is available online at
[https://debugger.skia.org](https://debugger.skia.org/) or can be run locally.

Building and running locally
--------------------

Begin by following the instructions to
[download and build Skia](../../user/quick), then simply build and run the
`skiaserve` tool:

<!--?prettify lang=sh?-->

    # Build.
    ninja -C out/Release skiaserve

    # Run the debugger locally
    out/Release/skiaserve

After running `skiaserve`, follow the instructions to open the debugger in your
local browser. By default the address will be `http://127.0.0.1:8888`.

![Debugger interface](/dev/tools/onlinedebugger.png)
