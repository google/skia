BAZEL?=bazelisk

.PHONY: build
build:
	$(BAZEL) run //infra/debugger-app:debugger_container \
	--config=ck_full_webgl2_debug_debugger \
	--workspace_status_command=bazel/get_workspace_status.sh

# Review section in README.md before running this target
.PHONY: push_debugger_I_am_really_sure
push_debugger_I_am_really_sure:
	$(BAZEL) run //infra/debugger-app:push_debugger_container \
	--config=ck_full_webgl2_debug_debugger \
	--workspace_status_command=bazel/get_workspace_status.sh