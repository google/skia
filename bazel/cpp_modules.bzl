"""
This module extension creates git_repositories loaded from a json file.

It is important to separate the implementation from the actual data because
the MODULE.bazel.lock file has a checksum of the .bzl files

"""

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

# https://bazel.build/rules/lib/globals/bzl.html#tag_class
_from_file_tag = tag_class(
    doc = "A JSON file containing attributes for git_repository",
    attrs = {
        "deps_json": attr.label(
            doc = "The deps.json file to read and create repos from.",
        ),
    },
)

# ctx is https://bazel.build/rules/lib/builtins/module_ctx
def _cpp_modules_impl(ctx):
    all_deps = set()
    direct_deps = []

    # https://bazel.build/rules/lib/builtins/bazel_module.html
    for module in ctx.modules:
        #https://bazel.build/rules/lib/builtins/bazel_module_tags
        if len(module.tags.from_file) != 1:
            fail("Must have exactly one from_file attribute set")
        json_file_label = module.tags.from_file[0].deps_json

        # https://bazel.build/rules/lib/builtins/module_ctx#read
        json_content = ctx.read(ctx.path(json_file_label))
        data = json.decode(json_content)

        if "direct" not in data:
            fail("JSON file must contain a 'direct' list: {}".format(json_file_label))
        if "indirect" not in data:
            fail("JSON file must contain a 'indirect' list (even if it's empty): {}".format(json_file_label))

        for repo_data in data["direct"]:
            name = repo_data.get("name")
            if name not in all_deps:
                git_repository(
                    build_file = repo_data.get("build_file"),
                    commit = repo_data.get("commit"),
                    name = name,
                    patch_cmds = repo_data.get("patch_cmds"),
                    patch_cmds_win = repo_data.get("patch_cmds_win"),
                    patches = repo_data.get("patches"),
                    remote = repo_data.get("remote"),
                )
                direct_deps.append(name)
            all_deps.add(name)

        for repo_data in data["indirect"]:
            name = repo_data.get("name")
            if name not in all_deps:
                git_repository(
                    build_file = repo_data.get("build_file"),
                    commit = repo_data.get("commit"),
                    name = name,
                    patch_cmds = repo_data.get("patch_cmds"),
                    patch_cmds_win = repo_data.get("patch_cmds_win"),
                    patches = repo_data.get("patches"),
                    remote = repo_data.get("remote"),
                )
            all_deps.add(name)

    # https://bazel.build/rules/lib/builtins/module_ctx#extension_metadata
    return ctx.extension_metadata(
        # By specifying the direct dependencies, bazel mod tidy will automatically
        # update the use_repo call to add or remove dependencies to the list.
        root_module_direct_deps = direct_deps,
        root_module_direct_dev_deps = [],
        # By setting this line, we are telling Bazel that the generated rules are
        # hermetic all on their own and that Bazel doesn't need to track the deps.json
        # file (e.g. via a sha256 checksum hash). This *is* the case because we are
        # generating git_repository rules, which include a specific commit, which is
        # effectively a hash of all the input contents.
        # This is a big deal because it means our autorollers can update only the deps.json
        # file and don't have to update anything in the MODULE.bazel.lock file after.
        reproducible = True,
    )

cpp_modules = module_extension(
    implementation = _cpp_modules_impl,
    tag_classes = {
        "from_file": _from_file_tag,
    },
)
