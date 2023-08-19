BAZEL?=bazelisk

.PHONY: build
build:
	$(BAZEL) run //infra/shaders:shaders_container \
	--config=shaders_container

# Review section in README.md before running this target
.PHONY: push_shaders_I_am_really_sure
push_shaders_I_am_really_sure:
	$(BAZEL) run //infra/shaders:push_shaders_container \
	--config=shaders_container
