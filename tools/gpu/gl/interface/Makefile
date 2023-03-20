generate:
	go run *.go --in_table "./interface.json5" --out_dir "../../../../src/gpu/ganesh/gl"

dryrun:
	go run *.go --in_table "./interface.json5" --out_dir "../../../../src/gpu/ganesh/gl" --dryrun

generate_bazel:
	bazelisk run //tools/gpu/gl/interface:generate_gl_interfaces

dryrun_bazel:
	bazelisk run //tools/gpu/gl/interface:generate_gl_interfaces -- --dryrun