"""This module defines the download_config_files rule.

   This allows external clients to get the configuration files that our custom
   Bazel rules for repos like Freetype depend on.

   Clients don't have to use this rule if they build those libraries their own
   way or want to use their own custom configurations.

   Skia doesn't use this rule directly, instead it uses a local_repository rule
   to make those configs available while still being defined in the same repo.
   Clients cannot use local_repository to refer to something in the Skia source
   code though, thus the need for a way for them to get those independently.

   TODO(kjlubick) Make this more hermetic by having a place to download (and verify)
   the files from that is more robust.
   """

def _download_config_files_impl(repo_ctx):
    # This will create one or more files in the [bazel sandbox]/external/[name] folder.
    for dst, src in repo_ctx.attr.files.items():
        # I'd prefer to download a single zip that we can verify with sha256,
        # or at least download from https://skia.googlesource.com/skia, however
        # the latter only lets one download base64-encoded files and I would prefer
        # not to depend on something like https://docs.aspect.build/rulesets/aspect_bazel_lib/docs/base64
        url = "https://raw.githubusercontent.com/google/skia/{}/{}".format(repo_ctx.attr.skia_revision, src)

        # https://bazel.build/rules/lib/builtins/repository_ctx#download
        repo_ctx.download(url, output = dst)

# https://bazel.build/rules/lib/globals/bzl#repository_rule
download_config_files = repository_rule(
    implementation = _download_config_files_impl,
    local = False,
    attrs = {
        # Maps dst (in this created repo) to src (relative to the Skia root).
        "files": attr.string_dict(mandatory = True),
        # The git version of the Skia repo from which to extract files.
        "skia_revision": attr.string(mandatory = True),
    },
)
