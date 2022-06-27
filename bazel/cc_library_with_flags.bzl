"""
This file contains a way to set flags from BUILD.bazel instead of requiring users to set them from
the CLI.

For example, a user could create:

    cc_library_with_flags(
        name = "skia_with_jpeg_png_and_svg",
        set_flags = {
            "include_decoder": [
                "jpeg_decode_codec",
                "png_decode_codec",
            ],
            "enable_svg_canvas": ["True"],
        },
        target = "//:skia_public",
    )

Then, compiling with

    bazel build :skia_with_jpeg_png_and_svg

would produce the same output as

    bazel build //:skia_public --include_encoder=jpeg_decode_codec \
        --include_encoder=png_decode_codec \
        --enable_svg_canvas

which uses aliases defined in .bazelrc to be the same as

    bazel build //:skia_public --//bazel/common_config_settings:include_encoder=jpeg_decode_codec \
        --//bazel/common_config_settings:include_encoder=png_decode_codec \
        --//bazel/common_config_settings:enable_svg_canvas

Having the options be defined in the BUILD.bazel target is much more convenient.

See more here about Bazel transitions:
 - https://bazel.build/rules/config#user-defined-transitions
 - https://bazel.build/rules/lib/transition
 - https://github.com/bazelbuild/examples/tree/5a8696429e36090a75eb6fee4ef4e91a3413ef13/configurations
"""

load("//bazel:cc_binary_with_flags.bzl", "with_flags_transition")

# This transition implementation just forwards the outputs from the target as its own.
# We expect target to be a cc_library, so we return its CcInfo outputs, describing the compilation
# and linking of C++ code. That way, this rule can be used anywhere a cc_library could be used
# (e.g. in the deps of a cc_binary).
# https://bazel.build/rules/rules#default_outputs
def _transition_rule_impl(ctx):
    target = ctx.attr.target[0]
    return [
        # https://bazel.build/rules/lib/CcInfo
        target[CcInfo],
    ]

cc_library_with_flags = rule(
    implementation = _transition_rule_impl,
    attrs = {
        # set_flags is a dictionary with the keys being the short-form of a flag name
        # (e.g. the part that comes after the colon) and the value being a list of values
        # that the flag should be set to, regardless of the relevant CLI flags.
        # https://bazel.build/rules/lib/attr#string_list_dict
        "set_flags": attr.string_list_dict(),
        "target": attr.label(
            mandatory = True,
            allow_single_file = True,
            # Setting cfg to be a transition allows us to modify the build settings before
            # Bazel actually does the building. In this way, we can read the set_flags dictionary
            # and modify/update the appropriate build settings that we defined in
            # //bazel/common_config_settings. We share this transition with cc_binary_with_flags.
            cfg = with_flags_transition,
        ),
        # This is a stock Bazel requirement for any rule that uses Starlark
        # transitions. It's okay to copy the below verbatim for all such rules.
        "_allowlist_function_transition": attr.label(
            default = "@bazel_tools//tools/allowlists/function_transition_allowlist",
        ),
    },
)
