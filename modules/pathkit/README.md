PathKit WASM API
================

This library lets you use Skia's feature-rich PathOps API in the browser.


Compiling the source
--------------------

Download the [Enscriptem SDK](https://kripken.github.io/emscripten-site/docs/getting_started/downloads.html).

Set the EMSDK environment variable to the directory you installed it to.

Run `./compile.sh` to compile a production, WASM build to `$SKIA_HOME/out/pathkit`.
Add "--help" for more options.


Deploying to npm
----------------

    # Build all 3 versions (release, test, debug) for both asmjs and WASM
    # These binaries will be placed in the proper places of npm-*/bin
    # This takes 5-10 minutes.
    make npm

    # Update the package.json files of both npm-asmjs and npm-wasm
    make update-patch  # or update-minor or update-major

    # Publish both repos
    make publish
