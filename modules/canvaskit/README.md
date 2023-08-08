# Prerequisites

Node v14 or later is required to run tests. We use npm (the Node Package Manager) to install
test dependencies. Recent installations of Node have npm as well.
CanvasKit has no other external source dependencies.

## Compiling with GN
To build with GN, you need to have followed the instructions to download Skia and its deps
<https://skia.org/user/download>.

To compile CanvasKit, you will first need to [download and activate `emscripten`][1] using the
script in `//bin/activate-emsdk` (or `//tools/git-sync-deps` which also calls activate-emsdk).
This places the associated files in `//third_party/externals/emsdk` and the GN[2] build scripts
will use those by default.
The compile.sh script automates the default GN settings; users are free to set their own. If users
want to use their own version of emscripten, they should set the `skia_emsdk_dir` argument
(see `//skia/gn/toolchain/wasm.gni`). For other available arguments, see
`//modules/canvaskit/BUILD.gn`.

[1]: https://emscripten.org/
[2]: https://chromium.googlesource.com/chromium/src/tools/gn/+/48062805e19b4697c5fbd926dc649c78b6aaa138/README.md

### MacOS specific notes
Make sure you have Python3 installed, otherwise the downloading emscripten toolchain
can fail with errors about SSL certificates. <https://github.com/emscripten-core/emsdk/pull/273>

See also <https://github.com/emscripten-core/emscripten/issues/9036#issuecomment-532092743>
for a solution to Python3 using the wrong certificates.

# Compile and Run Local Example

```
# The following installs all npm dependencies and only needs to be when setting up
# or if our npm dependencies have changed (rarely).
npm ci

make release  # make debug is much faster and has better error messages
make local-example
```

This will print a local endpoint for viewing the example.  You can experiment
with the CanvasKit API by modifying `./npm_build/example.html` and refreshing
the page. For some more experimental APIs, there's also `./npm_build/extra.html`.

For other available build targets, see `Makefile` and `compile.sh`.
For example, building a stripped-down version of CanvasKit with no text support or
any of the "extras", one might run:

    ./compile.sh no_skottie no_font

Such a stripped-down version is about half the size of the default release build.

If CanvasKit fails to build and you are getting compile errors that don't look like Skia code,
you may need to do a fresh install of the npm modules. You can do this by finding the .dts file
mentioned in the error message, deleting it, and rerunning `npm ci`.

If you're using the correct modules plus the latest supported typescript and it still fails,
the module versions listed in package.json may need to be updated as well.

# Unit tests, performance tests, and coverage.

To run unit tests and compute test coverage on a debug gpu build

```
make debug
make test-continuous
```

This reads karma.conf.js, and opens a Chrome browser and begins running all the test
in `test/` it will detect changes to the tests in that directory and automatically
run again, however it will automatically rebuild and reload CanvasKit. Closing the
chrome window will just cause it to re-opened. Kill the karma process to stop continuous
monitoring for changes.

The tests are run with whichever build of CanvasKit you last made. be sure to also
test with `release`, `debug_cpu`, and `release_cpu`. testing with release builds will
expose problems in closure compilation and usually forgotten externs.

## Coverage

Coverage will be automatically computed when running test-continuous locally. Note that
the results will only be useful when testing a debug build. Open
`coverage/<browser version>/index.html` For a summary and detailed line-by-line result.

## Measuring Performance

We use puppeteer to run a Chrome browser to gather performance data in a consistent way.
See `//tools/perf-canvaskit-puppeteer` for more.

## Adding tests

The tests in `tests/` are grouped into files by topic.
Within each file there are `describe` blocks further organizing the tests, and within those
`it()` functions which test particular behaviors. `describe` and `it` are jasmine methods
which can both be temporarily renamed `fdescribe` and `fit`. Which causes jasmine to only those.

We have also defined `gm` which is a method for defining a test which draws something to a canvas
that is shapshotted and reported to gold.skia.org, where you can compare it with the snapshot at
head.

## Testing from Gerrit

When submitting a CL in gerrit, click "choose tryjobs" and type CanvasKit to filter them.
select all of them, which at the time of this writing is four jobs, for each combination
of perf/test gpu/cpu.

The performance results are reported to [perf.skia.org] and correctness results are reported to
[gold.skia.org].

Coverage is not measured while running tests this way.

# Inspecting output WASM

The `wasm2wat` tool from [the WebAssembly Binary Toolkit](https://github.com/WebAssembly/wabt)
can be used to produce a human-readable text version of a `.wasm` file.

The output of `wasm2wat --version` should be `1.0.13 (1.0.17)`. This version has been checked to
work with the tools in `wasm_tools/SIMD/`. These tools programmatically inspect the `.wasm` output
of a CanvasKit build to detect the presence of [wasm SIMD](https://github.com/WebAssembly/simd)
operations.

# Infrastructure Playbook

When dealing with CanvasKit (or PathKit) on our bots, we use Docker. Check out
$SKIA_ROOT/infra/wasm-common/docker/README.md for more on building/editing the
images used for building and testing.

## Updating the version of Emscripten we build/test with

This presumes you have updated emscripten locally to a newer version of the
sdk and verified/fixed any build issues that have arisen.

  1. Edit `//bin/activate-emsdk` to install and activate the desired version of Emscripten.
  2. Upload a CL with all the changes. Run all Test.+CanvasKit, Perf.+Puppeteer,
      Test.+PathKit, Perf.+PathKit jobs to make sure the new builds pass all
      tests and don't crash the perf harnesses.
  3. Send out CL for review. Feel free to point the reviewer at these steps.

## Running Skia's GMs and Unit Tests against wasm+WebGL ##

General Tips:
 - Make use of the skip lists and start indexes in the run-wasm-gm-tests.html to focus in on
   problematic tests.
 - `Uncaught (in promise) RuntimeError: function signature mismatch` tends to mean null was
   dereferenced somewhere. Add SkASSERT to verify.

### Debugging some GMs / Unit Tests
For faster cycle time, it is recommended to focus on specific GMs instead of re-compiling all
of them. This can be done by modifying the `compile_gm.sh` script (but not checking this in)
to set `GMS_TO_BUILD` and/or `TESTS_TO_BUILD` to a minimal set of files. There's an `if false`
that can be commented out to assist with this.

Run `make gm_tests` or `make_gm_tests_debug` from this folder. This will produce a .js and .wasm
in a (not checked in) `build` subfolder.

Run `make single-gm` and navigate to <http://localhost:8000/wasm_tools/gms.html>. This will load
that html file and the freshly built wasm_gm_tests binary and run a single GM and unit test that
was compiled in. Feel free to modify //modules/canvaskit/wasm_tools/gms.html to run the specific
GM/unit test or tests that you care about.

### Testing all GMs / Unit Tests
With the current GN build, this can take quite a while to compile and re-compile (the upcoming
Bazel build should alleviate this).

Run `make gm_tests` or `make_gm_tests_debug` from this folder. This will produce a .js and .wasm
in a (not checked in) `build` subfolder.

Change directory to `//tools/run-wasm-gm-tests`. Run `make run_local`, which will put all PNGs
produced by GMs into `/tmp/wasm-gmtests` and run all unit tests.
