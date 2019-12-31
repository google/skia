# Prerequisites

To compile CanvasKit, you will first need to [install `emscripten`][1].  This
will set the environment `EMSDK` (among others) which is required for
compilation.

# Compile and Test Locally

```
make release
make local-example
```

This will print a local endpoint for viewing the example.  You can experiment
with the CanvasKit API by modifying `./canvaskit/example.html` and refreshing
the page. For some more experimental APIs, there's also `./canvaskit/extra.html`.

For other available build targets, see `Makefile` and `compile.sh`.
For example, building a stripped-down version of CanvasKit with no text support or
any of the "extras", one might run:

    ./compile.sh no_skottie no_particles no_font

Such a stripped-down version is about half the size of the default release build.

[1]: https://emscripten.org/docs/getting_started/downloads.html
