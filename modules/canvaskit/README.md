# Prerequisites

To compile CanvasKit, you will first need to [install `emscripten`][1].  This
will set the environment `EMSDK` (among others) which is required for
compilation.

# Compile and Test Locally

```
make release  # make debug is much faster and has better error messages
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
  6. In `$SKIA_ROOT/infra/canvaskit/docker/`, make `publish_canvaskit_emsdk`.
  7. In `$SKIA_ROOT/infra/bots/recipe_modules/build/`, update `canvaskit.py`
     and `pathkit.py` to have `DOCKER_IMAAGE` point to the desired tagged Docker
     containers from steps 2 and 5 (which should be the same).
  9. In `$SKIA_ROOT/infra/bots/`, run `make train` to re-train the recipes.
  10. Optional: Run something like `git grep 1\\.38\\.` in `$SKIA_ROOT` to see if
     there are any other references that need updating.
  11. Upload a CL with all the changes. Run all Test.+CanvasKit, Perf.+CanvasKit,
      Test.+PathKit, Perf.+PathKit jobs to make sure the new builds pass all
      tests and don't crash the perf harnesses.
  12. Send out CL for review. Feel free to point the reviewer at these steps.