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
the page.

For other available build targets, see `Makefile`.

[1]: https://emscripten.org/docs/getting_started/downloads.html
