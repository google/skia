# Prerequisites

Node v14 or later is required to run tests. We use npm (the Node Package Manager) to install
test dependencies. Recent installations of Node have npm as well.
CanvasKit has no other external source dependencies.

To compile CanvasKit, you will first need to [install `emscripten`][1].  This
will set the environment `EMSDK` (among others) which is required for
compilation. Which version should you use?  [`/infra/wasm-common/docker/emsdk-base/Dockerfile`][2]
shows the version we build and test with. We try to use as recent a version of emscripten as
is reasonable.

[1]: https://emscripten.org/docs/getting_started/downloads.html
[2]: https://github.com/google/skia/blob/main/infra/wasm-common/docker/emsdk-base/Dockerfile

Be sure to both install **and** activate the correct version. For example:
```
    ./emsdk install 2.0.20
    ./emsdk activate 2.0.20
```

This document also assumes you have followed the instructions to download Skia and its deps
<https://skia.org/user/download>.

## MacOS specific notes
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

    ./compile.sh no_skottie no_particles no_font

Such a stripped-down version is about half the size of the default release build.

# Unit tests, performance tests, and coverage.

To run unit tests and compute test coverage on a debug gpu build

```
make debug
make test-continuous
```

This reads karma.conf.js, and opens a chrome browser and begins running all the test
in `test/` it will detect changes to the tests in that directory and automatically
run again, however it will automatically rebuild and reload canvaskit. Closing the
chrome window will just cause it to re-opened. Kill the karma process to stop continuous
monitoring for changes.

The tests are run with whichever build of canvaskit you last made. be sure to also
test with `release`, `debug_cpu`, and `release_cpu`. testing with release builds will
expose problems in closure compilation and usually forgotten externs.

## Coverage

Coverage will be automatically computed when running test-continuous locally. Note that
the results will only be useful when testing a debug build. Open
`coverage/<browser version>/index.html` For a summary and detailed line-by-line result.

## Measuring Performance

We use puppeteer to run a Chrome browser to gather performance data in a consistent way.
See //tools/perf-canvaskit-puppeteer for more.

## Adding tests

The tests in `tests/` are grouped into files by topic.
Within each file there are `describe` blocks further organizing the tests, and within those
`it()` functions which test particular behaviors. `describe` and `it` are jasmine methods
which can both be temporarily renamed `fdescribe` and `fit`. Which causes jasmine to only those.

We have also defined `gm` which is a method for defining a test which draws something to a canvas
that is shapshotted and reported to gold.skia.org, where you can compare it with the snapshot at
head.

## Testing from Gerrit

When submitting a CL in gerrit, click "choose tryjobs" and type canvaskit to filter them.
select all of them, which at the time of this writing is four jobs, for each combination
of perf/test gpu/cpu.

The performance results are reported to perf.skia.org
gold results are reported to gold.skia.org

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

  1. Edit `$SKIA_ROOT/infra/wasm-common/docker/emsdk-base/Dockerfile` to install
     and activate the desired version of Emscripten.
  2. Edit `$SKIA_ROOT/infra/wasm-common/docker/Makefile` to have `EMSDK_VERSION` be
     set to that desired version. If there is a suffix that is not `_v1`, reset
     it to be `_v1`. If testing the image later does not work and edits are made
     to the emsdk-base Dockerfile to correct that, increment to `_v2`,`_v3`, etc
     to force the bots to pick up the new image.
  3. In `$SKIA_ROOT/infra/wasm-common/docker/`, run `make publish_emsdk_base`
  4. Edit `$SKIA_ROOT/infra/canvaskit/docker/canvaskit-emsdk/Dockerfile` to be based
     off the new version from step 2. CanvasKit has its own docker image because
     it needs a few extra dependencies to build with font support.
  5. Edit `$SKIA_ROOT/infra/canvaskit/docker/Makefile` to have the same version
     from step 2. It's easiest to keep the `emsdk-base` and `canvaskit-emsdk` versions
     be in lock-step.
  6. In `$SKIA_ROOT/infra/canvaskit/docker/`, run `make publish_canvaskit_emsdk`.
  7. In `$SKIA_ROOT/infra/bots/recipe_modules/build/`, update `canvaskit.py`
     and `pathkit.py` to have `DOCKER_IMAGE` point to the desired tagged Docker
     containers from steps 2 and 5 (which should be the same).
  8. In `$SKIA_ROOT/infra/bots/task_drivers/compile_wasm_gm_tests.go`, update dockerImage
     to refer to the desired Docker containers from steps 2 and 5.
  9. In `$SKIA_ROOT/infra/bots/`, run `make train` to re-train the recipes.
  10. Optional: Run something like `git grep 1\\.38\\.` in `$SKIA_ROOT` to see if
     there are any other references that need updating.
  11. Upload a CL with all the changes. Run all Test.+CanvasKit, Perf.+Puppeteer,
      Test.+PathKit, Perf.+PathKit jobs to make sure the new builds pass all
      tests and don't crash the perf harnesses.
  12. Send out CL for review. Feel free to point the reviewer at these steps.
