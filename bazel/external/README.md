This folder is where we put `BUILD.bazel` files for external (e.g. third party) dependencies.

If a dependency supports Bazel, we should use those rules, but if the dependency does not, we
need to create our own rules in a subdirectory.

We generally compile third_party deps from source. If we do, we clone the repository and use
the given BUILD.bazel file to build it. This is specified in the `WORKSPACE.bazel` (e.g. 
`new_local_repository` or `new_git_repository`) and we refer to those targets using labels like
`@freetype`, or `@libpng`.

Some third_party deps we only link against prebuilt versions. For those, we do not involve
WORKSPACE.bazel and link to them directly, e.g. `//bazel/external/fontconfig`.
