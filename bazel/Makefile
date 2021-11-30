generate:
	bazel build @org_skia_go_infra//bazel/gazelle:gazelle_cpp
	cd .. && bazel-bin/external/org_skia_go_infra/bazel/gazelle/gazelle_cpp_/gazelle_cpp update \
		--third_party_file_map="third_party/file_map_for_bazel.json" \
		include src tools \
		modules/skshaper modules/svg modules/skresources modules/skparagraph modules/skunicode \
		experimental/bazel_test

gazelle_update_repo:
	cd .. && bazel run //:gazelle -- update-repos -from_file=go.mod -to_macro=go_repositories.bzl%go_repositories
