Notes about Bazel Builds
========================

Skia cannot be built with Bazel yet.

But you should be able to build and run the trivial `tools/bazel_test.cc`:

    $ bazel test ...

Dependencies
------------

`WORKSPACE.bazel` acts like `DEPS`, listing external dependencies and how to
fetch them.  You can call `bazel sync`, or just let `bazel {build,test,run}`
handle it as needed on its own.  The easiest way to add a new dependency is to
start using `tag="..."` or `branch="..."` and then follow the advice of Bazel
to pin that to the `commit` and `shallow_since` it suggests.

We must provide Bazel build configuration for dependencies like `libpng` that
don't provide their own.  For `libpng` that's `bazel/libpng.bazel`, linked by
the `new_git_repository()` `build_file` argument, written relative to that
fetched Git repo's root.  Its resemblance to `third_party/libpng/BUILD.gn` is
no coincidence... it's pretty much a 1:1 translation between GN and Bazel.

Everything that's checked in builds external dependencies from source.  I've
not written an integrated system for substituting prebuilt versions of these
dependencies (e.g. `/usr/include/png.h` and `/usr/lib/libpng.so`), instead
leaving that up to users who want it.  The process is not exactly trivial, but
closer to tedious than difficult.  Here's an example, overriding `libpng` to
point to prebuilts from Homebrew in ~/brew:

Each overridden dependency will need its own directory with a few files.

    $ find overrides
    overrides
    overrides/libpng
    overrides/libpng/include
    overrides/libpng/WORKSPACE.bazel
    overrides/libpng/BUILD.bazel

`WORKSPACE.bazel` must be present, but in this case can be empty.

    $ cat overrides/libpng/WORKSPACE.bazel

`BUILD.bazel` is where it all happens:

    $ cat overrides/libpng/BUILD.bazel
    cc_library(
        name = "libpng",
        hdrs = ["include/png.h"],
        srcs = ["include/pngconf.h", "include/pnglibconf.h"],
        includes = ["include"],
        linkopts = ["-lpng", "-L/Users/mtklein/brew/lib"],
        visibility = ["//visibility:public"],
    )

`include` is a symlink I've made to `~/brew/include` because Bazel doesn't like
absolute paths in `hdrs` or `includes`.  On the other hand, a symlink to
`~/brew/lib` doesn't work here, though `-L/Users/mtklein/brew/lib` works fine.

    $ readlink overrides/libpng/include
    /Users/mtklein/brew/include/

Finally, we point Bazel at all that using `--override_repository`:

    $ bazel test ... --override_repository libpng=/Users/mtklein/overrides/libpng

I expect building from source to be the most common use case, and it's more or
less enough to simply know that we can substitute prebuilts this way.  The most
interesting part to me is that we don't need to provide this mechanism... it's
all there in stock Bazel.  This plan may all want some rethinking in the future
if we want to add the option to trim the dependency entirely and make this
tristate (build it, use it prebuilt, or trim).

.bazelrc
--------

I have not (yet?) checked in a .bazelrc to the Skia repo, but have found it
handy to write my own in ~/.bazelrc:

    $ cat ~/.bazelrc
    # Print more information on failures.
    build --verbose_failures
    test --test_output errors

    # Create an ASAN config, try `bazel test --config asan ...`.
    build:asan --copt -fsanitize=address
    build:asan --copt -Wno-macro-redefined   # (_FORTIFY_SOURCE redefined.)
    build:asan --linkopt -fsanitize=address

    # Flip on and off prebuilt overrides easily.
    build --override_repository libpng=/Users/mtklein/overrides/libpng

I'm impressed by how much you can configure via bazelrc, and I think this
should let our Bazel build configuration stay mostly focused on the structure
of the project, less cluttered by build settings.
