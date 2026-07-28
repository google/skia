"""
This module defines the dawn_repo repository rule.

We have custom logic to generate Bazel rules to build Dawn because
1) Dawn doesn't support this natively
2) Dawn changes often by adding/removing files
3) Dawn is autorolled into Skia for testing
"""

def _dawn_repo_impl(repo_ctx):
    # Fetch the Dawn source (this takes a while on Windows)
    repo_ctx.execute(["git", "init"])
    repo_ctx.execute(["git", "remote", "add", "origin", repo_ctx.attr.remote])

    res = repo_ctx.execute(["git", "fetch", "--depth", "1", "origin", repo_ctx.attr.commit])
    if res.return_code != 0:
        fail("Failed to fetch Dawn: " + res.stderr)

    repo_ctx.execute(["git", "reset", "--hard", "FETCH_HEAD"])

    # On Windows, the checked-in headers conflict with the generated headers (Windows doesn't
    # sandbox like Linux/Mac). Dawn is planning on deleting these anyway, so we do it until then.
    for h in [
        "src/tint/lang/core/enums.h",
        "src/tint/lang/core/intrinsic/ctor_conv.h",
        "src/tint/lang/glsl/builtin_fn.h",
        "src/tint/lang/hlsl/builtin_fn.h",
        "src/tint/lang/msl/builtin_fn.h",
        "src/tint/lang/spirv/builtin_fn.h",
        "src/tint/lang/wgsl/enums.h",
        "src/tint/lang/wgsl/intrinsic/ctor_conv.h",
    ]:
        repo_ctx.delete(h)

    # Detect the host system and resolve our hermetic CPython binary
    os_name = repo_ctx.os.name.lower()
    os_arch = repo_ctx.os.arch.lower()

    if "windows" in os_name:
        python_dir = repo_ctx.path(Label("@cpython_windows_amd64//:BUILD.bazel")).dirname
        python_bin = python_dir.get_child("bin").get_child("python3.exe")
    elif "mac" in os_name:
        if "arm" in os_arch or "aarch64" in os_arch:
            python_dir = repo_ctx.path(Label("@cpython_mac_arm64//:BUILD.bazel")).dirname
        else:
            python_dir = repo_ctx.path(Label("@cpython_mac_amd64//:BUILD.bazel")).dirname
        python_bin = python_dir.get_child("bin").get_child("python3")
    else:
        python_dir = repo_ctx.path(Label("@cpython_linux_amd64//:BUILD.bazel")).dirname
        python_bin = python_dir.get_child("bin").get_child("python3")

    # Copy the BUILD.bazel from Skia
    repo_ctx.delete("BUILD.bazel")
    repo_ctx.delete("dawn_files.bzl")
    repo_ctx.symlink(repo_ctx.path(repo_ctx.attr.build_file), "BUILD.bazel")

    # Run the generator to create dawn_files.bzl
    generator_path = repo_ctx.path(repo_ctx.attr.generator_py)

    # We must explicitly watch the generator script and BUILD.bazel file because Bazel doesn't
    # magically know they affect the resulting checkout.
    repo_ctx.watch(repo_ctx.attr.generator_py)
    repo_ctx.watch(repo_ctx.attr.build_file)

    # We do *not* have to rely on jinja2 and markupsafe (deps for Dawn's generator scripts)
    # being available. We can use the versions Bazel checks out by adding them to PYTHONPATH.
    jinja2_dir = str(repo_ctx.path(repo_ctx.attr.jinja2).dirname)
    markupsafe_dir = str(repo_ctx.path(repo_ctx.attr.markupsafe).dirname)
    path_sep = ";" if "windows" in repo_ctx.os.name.lower() else ":"
    python_path = "{}{}{}".format(jinja2_dir, path_sep, markupsafe_dir)

    res = repo_ctx.execute(
        [python_bin, generator_path, ".", "dawn_files.bzl"],
        environment = {"PYTHONPATH": python_path},
    )
    if res.return_code != 0:
        fail("Failed to generate dawn_files.bzl: " + res.stderr)

# https://bazel.build/rules/lib/globals/bzl#repository_rule
dawn_repo = repository_rule(
    implementation = _dawn_repo_impl,
    attrs = {
        "commit": attr.string(mandatory = True),
        "remote": attr.string(mandatory = True),
        "build_file": attr.label(mandatory = True),
        "generator_py": attr.label(mandatory = True),
        "jinja2": attr.label(default = "@jinja2//:BUILD.bazel"),
        "markupsafe": attr.label(default = "@markupsafe//:BUILD.bazel"),
    },
)
