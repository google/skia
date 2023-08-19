BAZEL?=bazelisk

.PHONY: build
build:
	$(BAZEL) run //infra/jsfiddle:jsfiddle_container \
	--config=jsfiddle_container

# Review section in README.md before running this target
.PHONY: push_jsfiddle_I_am_really_sure
push_jsfiddle_I_am_really_sure:
	$(BAZEL) run //infra/jsfiddle:push_jsfiddle_container \
	--config=jsfiddle_container