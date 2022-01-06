This allows us to experiment with making sk_app target the browser.

To build, make sure bazelisk is installed. (This is a wrapper around bazel).
https://github.com/google/skia-buildbot/blob/main/BAZEL_CHEATSHEET.md#install-bazelisk

Then, run `make debug` to build the binary. To see the result on the web, run `make serve`
and navigate to http://localhost:8000/hello_world.html

Presently, the //tools/sk_app/wasm/main_wasm.cpp only prints out hello world, but it should
set up the surface/window for wasm and then call into the sk_app code.