This folder is where we put `BUILD.bazel` files for external (e.g. third party) dependencies.

If a dependency supports Bazel, we should use those rules, but if the dependency does not, we
need to create our own rules in a subdirectory.

We generally compile third_party deps from source. If we do, we clone the repository and use
the given BUILD.bazel file to build it. This is specified in the `WORKSPACE.bazel` (e.g. 
`new_local_repository` or `new_git_repository`) and we refer to those targets using labels like
`@freetype`, or `@libpng`.

Some third_party deps we only link against prebuilt versions. For those, we do not involve
WORKSPACE.bazel and link to them directly, e.g. `//bazel/external/fontconfig`.

Notes
-----

Avoid strip_include_prefix
==========================
[strip_include_prefix](https://docs.bazel.build/versions/main/be/c-cpp.html#cc_library.strip_include_prefix)
causes the header path for the library to be added to the compiler include search path with `-I`,
which means Clang will treat it like a file in Skia proper. This means if those headers have
issues that Clang's diagnostic warnings catch (e.g. missing `override`), we will see those warnings
and the build will fail.

Generally, we do not want to have to fix third_party code's warnings, so instead of
using `strip_include_prefix`, use `includes` instead. This is more ergonomic, as it can let us
expose header files from multiple locations (e.g. `freetype` has its API in `includes` and the
customization headers in `builds`) and adds these to the search path with `-isystem`. Clang ignores
warnings in these "system" headers, which means our warnings will be focused to the Skia code base.