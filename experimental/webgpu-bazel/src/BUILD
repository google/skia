load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@emsdk//emscripten_toolchain:wasm_rules.bzl", "wasm_cc_binary")

BASE_LINKOPTS = [
    #"-flto",  # https://github.com/emscripten-core/emsdk/issues/807
    "--bind",  # Compiles the source code using the Embind bindings to connect C/C++ and JavaScript
    "-s ALLOW_MEMORY_GROWTH=1",
    "-s USE_PTHREADS=0",  # Disable pthreads
    "-s ASSERTIONS=0",  # Turn off assertions
    "-s MODULARIZE=1",
    "-s EXPORT_NAME=WebGPUKitInit",
    "-s DISABLE_EXCEPTION_CATCHING=1",  # Disable all exception catching
    "-s NODEJS_CATCH_EXIT=0",  # We don't have a 'main' so disable exit() catching
    "-s WASM=1",
    "-s USE_WEBGPU=1",
]

RELEASE_OPTS = [
    "--closure 1",  # Run the closure compiler
]

DEBUG_OPTS = [
    "--closure 0",  # Do not use closure
]

config_setting(
    name = "release_opts",
    values = {"compilation_mode": "opt"},
)

config_setting(
    name = "debug_opts",
    values = {"compilation_mode": "dbg"},
)

cc_binary(
    name = "hello-world",
    srcs = ["bindings.cpp"],
    linkopts = select({
        ":debug_opts": BASE_LINKOPTS + DEBUG_OPTS,
        ":release_opts": BASE_LINKOPTS + RELEASE_OPTS,
        "//conditions:default": BASE_LINKOPTS + RELEASE_OPTS,
    }),
    # This target won't build successfully on its own because of missing emscripten
    # headers etc. Therefore, we hide it from wildcards.
    tags = ["manual"],
)

wasm_cc_binary(
    name = "hello-world-wasm",
    cc_target = ":hello-world",
)
