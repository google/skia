# This file will be copied into //third_party/externals/spirv-cross via the new_local_repository
# rule in WORKSPACE.bazel, so all files should be relative to that path.

cc_library(
    name = "spirv_cross",
    srcs = [
        "GLSL.std.450.h",
        "spirv.hpp",
        "spirv_cfg.cpp",
        "spirv_cfg.hpp",
        "spirv_common.hpp",
        "spirv_cpp.cpp",
        "spirv_cpp.hpp",
        "spirv_cross.cpp",
        "spirv_cross.hpp",
        "spirv_cross_containers.hpp",
        "spirv_cross_error_handling.hpp",
        "spirv_cross_parsed_ir.cpp",
        "spirv_cross_parsed_ir.hpp",
        "spirv_glsl.cpp",
        "spirv_glsl.hpp",
        "spirv_hlsl.cpp",
        "spirv_msl.cpp",
        "spirv_msl.hpp",
        "spirv_parser.cpp",
        "spirv_parser.hpp",
        "spirv_reflect.cpp",
        "spirv_reflect.hpp",
    ],
    hdrs = [
        "spirv_hlsl.hpp",
    ],
    defines = ["SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS"],
    includes = [
        # This allows #include <spirv_hlsl.hpp> to work
        ".",
    ],
    visibility = ["//visibility:public"],
)
