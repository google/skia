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

    # Build the release version for both asmjs and WASM
    # These binaries will be placed in the proper places of npm-*/bin
    make npm

    # In each npm- subdirectory, run:
    npm version minor (or patch or major)
    npm login --registry https://wombat-dressing-room.appspot.com
    npm publish
