This folder is where we put BUILD.bazel files for external (e.g. third party) dependencies.

If a dependency supports Bazel, we should use those rules, but if the dependency does not, we
need to create our own rules in a subdirectory.

These BUILD.bazel files are used in WORKSPACE.bazel (e.g. new_local_repository or
new_git_repository).