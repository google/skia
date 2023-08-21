BAZEL?=bazelisk

.PHONY: build
build:
	$(BAZEL) run //infra/skottie:skottie_container \
	--config=skottie_container

# Review section in README.md before running this target
.PHONY: push_skottie_I_am_really_sure
push_skottie_I_am_really_sure:
	$(BAZEL) run //infra/skottie:push_skottie_container \
	--config=skottie_container
