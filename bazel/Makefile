# This target should be run when new files are added or includes changed.
generate:
	bazelisk build @org_skia_go_infra//bazel/gazelle:gazelle_cpp
	cd .. && bazel-bin/external/org_skia_go_infra/bazel/gazelle/gazelle_cpp_/gazelle_cpp update \
		--third_party_file_map="third_party/file_map_for_bazel.json" \
		include src tools gm tests \
		modules/skshaper modules/svg modules/skresources modules/skparagraph modules/skunicode \
		modules/skottie modules/skresources modules/sksg experimental/ffmpeg \
		modules/particles \
		experimental/bazel_test experimental/graphite example \
		modules/canvaskit/go \
		infra/bots/task_drivers

# This target should be run after the go.mod file is updated (e.g. version rolls or new updates)
gazelle_update_repo:
	cd .. && bazelisk run //:gazelle -- update-repos -from_file=go.mod -to_macro=go_repositories.bzl%go_repositories

# Run this target to test all known working Bazel builds
known_good_builds:
	bazelisk build //experimental/bazel_test/... --config=clang --sandbox_base=/dev/shm
	bazelisk run //experimental/bazel_test:bazel_test_exe --config=clang --sandbox_base=/dev/shm
	bazelisk build //:skia_core --config=clang --sandbox_base=/dev/shm
	bazelisk build //src/sksl/lex:sksllex --config=clang --sandbox_base=/dev/shm
	bazelisk build //tools/skdiff --config=clang --sandbox_base=/dev/shm
	bazelisk build //example:hello_world_gl --config=clang --sandbox_base=/dev/shm

	# Test the enforcement of include what you use
	bazelisk build //experimental/bazel_test/... --config=clang --sandbox_base=/dev/shm \
 		--features skia_enforce_iwyu
 	# Both with and without a GPU backend should be error free (i.e. IWYU should let us
 	# conditionally import things.
	bazelisk build //src/svg/... --config=clang --sandbox_base=/dev/shm \
		--features skia_enforce_iwyu --gpu_backend=gl_backend --include_decoder=jpeg_decode_codec
	bazelisk build //src/svg/... --config=clang --sandbox_base=/dev/shm \
		--features skia_enforce_iwyu