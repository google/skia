generate:
	echo "You do not need to run this any more for C++ code, only if you modify go code"

generate_go:
	cd .. && bazelisk run //:gazelle -- update infra/bots/task_drivers modules/canvaskit/go

# This target should be run after the go.mod file is updated (e.g. version rolls or new updates)
gazelle_update_repo:
	cd .. && bazelisk run //:gazelle -- update-repos -from_file=go.mod -to_macro=go_repositories.bzl%go_repositories

# Run this target to test all known working Bazel builds
known_good_builds:
	bazelisk build //:skia_public --config=clang_linux
	bazelisk build //:skia_internal --config=clang_linux
	bazelisk build //experimental/bazel_test/... --config=clang_linux
	bazelisk run //experimental/bazel_test:bazel_test_exe --config=clang_linux
	bazelisk build //example:hello_world_gl --config=clang_linux
	bazelisk build //example:hello_world_vulkan --config=clang_linux
	bazelisk build //example:hello_world_dawn --config=clang_linux
	bazelisk build //example:vulkan_basic --config=clang_linux
	bazelisk build //src/sksl/lex:sksllex --config=clang_linux
	bazelisk build //tools/skdiff --config=clang_linux
	bazelisk build //tools/skslc --config=clang_linux
	bazelisk build //modules/canvaskit:canvaskit_wasm --config=ck_release

rbe_known_good_builds:
	bazelisk build //:skia_public --config=linux_rbe --remote_download_minimal
	bazelisk build //:skia_internal --config=linux_rbe --remote_download_minimal
	bazelisk build //experimental/bazel_test/... --config=linux_rbe --remote_download_minimal
	bazelisk run //experimental/bazel_test:bazel_test_exe --config=linux_rbe --remote_download_toplevel
	bazelisk build //example:hello_world_gl --config=linux_rbe --remote_download_minimal
	bazelisk build //example:hello_world_vulkan --config=linux_rbe --remote_download_minimal
	bazelisk build //example:hello_world_dawn --config=linux_rbe --remote_download_minimal
	bazelisk build //example:vulkan_basic --config=linux_rbe --remote_download_minimal
	bazelisk build //src/sksl/lex:sksllex --config=linux_rbe --remote_download_minimal
	bazelisk build //tools/skdiff --config=linux_rbe --remote_download_minimal
	bazelisk build //tools/skslc --config=linux_rbe --remote_download_minimal
	## TODO(kjlubick) CanvasKit in release mode (i.e. with Closure) requires
	## https://github.com/emscripten-core/emscripten/pull/16640 to land
	bazelisk build //modules/canvaskit:canvaskit_wasm --compilation_mode dbg --config=linux_rbe \
		--remote_download_minimal

