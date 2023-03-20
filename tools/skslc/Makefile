# kjlubick@ could not find a good way to run multiple bazel tasks using bazel itself
compile_all_tests:
	bazel run //tools/skslc:compile_glsl_tests --config=release
	bazel run //tools/skslc:compile_glsl_nosettings_tests --config=release
	bazel run //tools/skslc:compile_metal_tests --config=release
	bazel run //tools/skslc:compile_skrp_tests --config=release
	bazel run //tools/skslc:compile_skvm_tests --config=release
	bazel run //tools/skslc:compile_stage_tests --config=release
	bazel run //tools/skslc:compile_spirv_tests --config=release
	bazel run //tools/skslc:compile_wgsl_tests --config=release
	bazel run //tools/skslc:compile_hlsl_tests --config=release
