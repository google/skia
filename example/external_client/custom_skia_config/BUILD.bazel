load("@rules_cc//cc:defs.bzl", "cc_library")

config_setting(
    name = "debug_build",
    values = {"compilation_mode": "dbg"},
)

cc_library(
    name = "user_config",
    hdrs = [
        "SkUserConfig.h",
    ],
    defines = [
        "SK_USE_BAZEL_CONFIG_HEADER",
    ] + select({
        ":debug_build": ["SK_DEBUG"],
        "//conditions:default": ["SK_RELEASE"],
    }),
    visibility = ["//visibility:public"],
)
