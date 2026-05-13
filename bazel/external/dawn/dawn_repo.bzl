"""
This module defines the dawn_repo repository rule.

We have custom logic to generate Bazel rules to build Dawn because
1) Dawn doesn't support this natively
2) Dawn changes by adding/removing files
3) Dawn is autorolled into Skia for testing
"""

def _dawn_repo_impl(repo_ctx):
    repo_ctx.execute(["git", "init"])
    repo_ctx.execute(["git", "remote", "add", "origin", repo_ctx.attr.remote])
    res = repo_ctx.execute(["git", "fetch", "--depth", "1", "origin", repo_ctx.attr.commit])
    if res.return_code != 0:
        fail("Failed to fetch Dawn: " + res.stderr)
    repo_ctx.execute(["git", "reset", "--hard", "FETCH_HEAD"])

    # Apply patches
    for patch in repo_ctx.attr.patches:
        res = repo_ctx.execute(["patch", "-p1", "-i", repo_ctx.path(patch)])
        if res.return_code != 0:
            fail("Failed to apply patch {}: {}".format(patch, res.stderr))

    # Copy the BUILD.bazel and dawn_files.bzl from Skia
    # We remove existing files to avoid "File exists" errors when symlinking
    repo_ctx.execute(["rm", "-f", "BUILD.bazel", "dawn_files.bzl"])
    repo_ctx.symlink(repo_ctx.path(repo_ctx.attr.build_file), "BUILD.bazel")

    # TODO(kjlubick) generate this dawn_files.bzl
    repo_ctx.symlink(repo_ctx.path(repo_ctx.attr.files_bzl), "dawn_files.bzl")

# https://bazel.build/rules/lib/globals/bzl#repository_rule
dawn_repo = repository_rule(
    implementation = _dawn_repo_impl,
    attrs = {
        "commit": attr.string(mandatory = True),
        "remote": attr.string(mandatory = True),
        "patches": attr.label_list(),
        "build_file": attr.label(mandatory = True),
        "files_bzl": attr.label(mandatory = True),
    },
)
