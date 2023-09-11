PathKit WASM API
================

The [PathKit](https://skia.org/docs/user/modules/pathkit/) library lets you use
Skia's feature-rich PathOps API in the browser.

Compiling the source
--------------------

### Compiling with GN

Get a compiled [Enscriptem SDK](https://kripken.github.io/emscripten-site/docs/getting_started/downloads.html)
in your path. This is easiest done by running `//tools/git-sync-deps` which
downloads and then runs `//bin/activate-emsdk`.

To compile a production WASM build to `//out/pathkit`:

```sh
./compile.sh
```

Add `--help` for more options.

### Compiling with Bazel

To compile a production WASM build to `//bazel-bin/modules/pathkit/pathkit`:

```sh
bazelisk build //modules/pathkit:pathkit --config=ck_full_webgl2_release
```

A debug build:

```sh
bazelisk build //modules/pathkit:pathkit --config=ck_full_webgl2_debug
```

Testing
-------

### Running example page locally:

```sh
# First build PathKit
make release
# Then run a local test server
make local-example
```

Then follow messages to navigate the browser to the example page.

### Automated tests

**Testing the GN build**

```sh
make release
make npm
npm ci
make test-continuous
```

**Testing the Bazel build**

```sh
make debug-bazel
make npm-bazel
npm ci
make test-continuous
```

Deploying to npm
----------------

NOTE: The deployment steps use the GN build.

```sh
# Build the release version for both asmjs and WASM
# These binaries will be placed in the proper places of npm-*/bin
make npm

# In each npm- subdirectory, run:
npm version minor (or patch or major)
npm login --registry https://wombat-dressing-room.appspot.com
npm publish
```