generate:
	echo "You do not need to run this any more for C++ code, only if you modify go code"

generate_go:
	cd .. && bazelisk run //:gazelle -- update infra/bots/task_drivers modules/canvaskit/go

# This target should be run after the go.mod file is updated (e.g. version rolls or new updates)
gazelle_update_repo:
	cd .. && bazelisk run //:gazelle -- update-repos -from_file=go.mod -to_macro=go_repositories.bzl%go_repositories

# Run this target to test all known working Bazel builds
known_good_builds:
	#bazelisk build //experimental/bazel_test/... --config=clang_linux
	#bazelisk run //experimental/bazel_test:bazel_test_exe --config=clang_linux
	#bazelisk build //:skia_core --config=clang_linux
	#bazelisk build //src/sksl/lex:sksllex --config=clang_linux
	#bazelisk build //tools/skdiff --config=clang_linux
	#bazelisk build //tools/skslc --config=clang_linux
	#bazelisk build //modules/canvaskit:canvaskit_wasm --compilation_mode opt --sandbox_base=/dev/shm

rbe_known_good_builds:
	#bazelisk build //experimental/bazel_test/... --config=linux_rbe
	#bazelisk run //experimental/bazel_test:bazel_test_exe --config=linux_rbe
	#bazelisk build //:skia_core --config=linux_rbe
	#bazelisk build //src/sksl/lex:sksllex --config=linux_rbe
	#bazelisk build //tools/skdiff --config=linux_rbe
	#bazelisk build //tools/skslc --config=linux_rbe
	## TODO(kjlubick) CanvasKit in release mode (i.e. with Closure) requires
	## https://github.com/emscripten-core/emscripten/pull/16640 to land
	#bazelisk build //modules/canvaskit:canvaskit_wasm --compilation_mode dbg --config=linux_rbe \
	#	--jobs 100

