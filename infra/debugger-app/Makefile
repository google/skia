BAZEL?=bazelisk

.PHONY: build
build:
	$(BAZEL) run //infra/debugger-app:debugger_container \
	--config=debugger_app_container

# Review section in README.md before running this target
.PHONY: push_debugger_I_am_really_sure
push_debugger_I_am_really_sure:
	$(BAZEL) run //infra/debugger-app:push_debugger_container \
	--config=debugger_app_container