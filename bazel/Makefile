generate:
	bazelisk build @org_skia_go_infra//bazel/gazelle:gazelle_cpp
	cd .. && bazel-bin/external/org_skia_go_infra/bazel/gazelle/gazelle_cpp_/gazelle_cpp update \
		--third_party_file_map="third_party/file_map_for_bazel.json" \
		include src tools \
		modules/skshaper modules/svg modules/skresources modules/skparagraph modules/skunicode \
		experimental/bazel_test example

gazelle_update_repo:
	cd .. && bazelisk run //:gazelle -- update-repos -from_file=go.mod -to_macro=go_repositories.bzl%go_repositories

# Run this target to test all known working Bazel builds
known_good_builds:
	bazelisk build //experimental/bazel_test/... --config=clang
	bazelisk build //:skia_core --config=clang --sandbox_base=/dev/shm
	bazelisk build //src/sksl/lex:sksllex --config=clang