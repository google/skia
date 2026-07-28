load("//bazel:cipd_install.bzl", "cipd_install")

def _cipd_deps_impl(_ctx):
    # https://bazel.build/rules/lib/builtins/module_ctx

    cipd_install(
        name = "gn_linux_amd64",
        build_file_content = """
exports_files(
    ["gn"],
    visibility = ["//visibility:public"]
)
""",
        cipd_package = "gn/gn/linux-amd64",
        # From https://chrome-infra-packages.appspot.com/p/gn/gn/linux-amd64/+/git_revision:1c4151ff5c1d6fbf7fa800b8d4bb34d3abc03a41
        sha256 = "7195291488d08f3a10e85b85d8c4816e077015f1c5f196f770003a97aa42caf8",
        tag = "git_revision:1c4151ff5c1d6fbf7fa800b8d4bb34d3abc03a41",
    )

    cipd_install(
        name = "gn_mac_arm64",
        build_file_content = """
exports_files(
    ["gn"],
    visibility = ["//visibility:public"]
)
""",
        cipd_package = "gn/gn/mac-arm64",
        # From https://chrome-infra-packages.appspot.com/p/gn/gn/mac-arm64/+/git_revision:1c4151ff5c1d6fbf7fa800b8d4bb34d3abc03a41
        sha256 = "1123907ac3317530e9dd537d50cd83fd83e852aacc07d286f45753c8fc5287ed",
        tag = "git_revision:1c4151ff5c1d6fbf7fa800b8d4bb34d3abc03a41",
    )

    cipd_install(
        name = "gn_mac_amd64",
        build_file_content = """
exports_files(
    ["gn"],
    visibility = ["//visibility:public"]
)
""",
        cipd_package = "gn/gn/mac-amd64",
        # From https://chrome-infra-packages.appspot.com/p/gn/gn/mac-amd64/+/git_revision:1c4151ff5c1d6fbf7fa800b8d4bb34d3abc03a41
        sha256 = "ed96f7d2f49b83b016e4bdbed432e4734a5a133accb025d7c07685e01489ba93",
        tag = "git_revision:1c4151ff5c1d6fbf7fa800b8d4bb34d3abc03a41",
    )

    cipd_install(
        name = "git_linux_amd64",
        build_file_content = """
filegroup(
  name = "all_files",
  # The exclude pattern prevents files with spaces in their names from tripping up Bazel.
  srcs = glob(include=["**/*"], exclude=["**/* *"]),
  visibility = ["//visibility:public"],
)
""",
        cipd_package = "infra/3pp/tools/git/linux-amd64",
        # Based on
        # https://skia.googlesource.com/buildbot/+/f1d21dc58818cd6aba0a7822e59d37636aefe936/WORKSPACE#391.
        #
        # Note that the below "git config" commands do not affect the user's Git configuration. These
        # settings are only visible to Bazel targets that depend on @git_linux_amd64//:all_files via
        # the "data" attribute. The result of these commands can be examined as follows:
        #
        #     $ cat $(bazel info output_base)/external/git_linux_amd64/etc/gitconfig
        #     [user]
        #             name = Bazel Test User
        #             email = bazel-test-user@example.com
        postinstall_cmds_posix = [
            "mkdir etc",
            "bin/git config --system user.name \"Bazel Test User\"",
            "bin/git config --system user.email \"bazel-test-user@example.com\"",
        ],
        # From https://chrome-infra-packages.appspot.com/p/infra/3pp/tools/git/linux-amd64/+/version:2.29.2.chromium.6
        sha256 = "36cb96051827d6a3f6f59c5461996fe9490d997bcd2b351687d87dcd4a9b40fa",
        tag = "version:2.29.2.chromium.6",
    )

    cipd_install(
        name = "cpython_linux_amd64",
        build_file_content = """
exports_files(
    glob(["**/*"]),
    visibility = ["//visibility:public"]
)
""",
        cipd_package = "infra/3pp/tools/cpython3/linux-amd64",
        sha256 = "a06bad297267859aad818d2d6c9cc2865f4b0087c71dbf020f6b8e0cb92746c2",
        tag = "version:3@3.11.9.chromium.36",
    )

    cipd_install(
        name = "cpython_mac_amd64",
        build_file_content = """
exports_files(
    glob(["**/*"]),
    visibility = ["//visibility:public"]
)
""",
        cipd_package = "infra/3pp/tools/cpython3/mac-amd64",
        sha256 = "787e1e98b6220b4bf887ae04a8a80fb4939ad3c24ca6e2e7753541048c97224a",
        tag = "version:3@3.11.9.chromium.36",
    )

    cipd_install(
        name = "cpython_mac_arm64",
        build_file_content = """
exports_files(
    glob(["**/*"]),
    visibility = ["//visibility:public"]
)
""",
        cipd_package = "infra/3pp/tools/cpython3/mac-arm64",
        sha256 = "6c7fb45e6ac6363c00b9820b259132554190a197d65437a0912025fa9c27e161",
        tag = "version:3@3.11.9.chromium.36",
    )

    cipd_install(
        name = "cpython_windows_amd64",
        build_file_content = """
exports_files(
    glob(["**/*"]),
    visibility = ["//visibility:public"]
)
""",
        cipd_package = "infra/3pp/tools/cpython3/windows-amd64",
        sha256 = "cdc51dd7d25bc6a249b076e117cdfe29456a93cddd48d4193ca3742a20ea7699",
        tag = "version:3@3.11.9.chromium.36",
    )

cipd_deps = module_extension(
    implementation = _cipd_deps_impl,
)
