To run these demos with a local build of CanvasKit, go into the CanvasKit folder
`//modules/canvaskit` and run `make debug` or similar to build it.

Then, run `make local` from this directory to spin up a local web server.

You will need to modify your demos to load from the local web server and not
the CDN. See ./demos/hello_world for an example.