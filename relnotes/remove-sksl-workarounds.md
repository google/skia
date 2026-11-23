Removed the unused `emulate_abs_int_function` (`EMULATE_ABS_INT_FUNCTION`),
`rewrite_do_while_loops` (`REWRITE_DO_WHILE_LOOPS`), and
`unfold_short_circuit_as_ternary_operation` (`UNFOLD_SHORT_CIRCUIT_AS_TERNARY_OPERATION`)
driver bug workarounds from `GrDriverBugWorkarounds` and `GrDriverBugWorkaroundType`, along with
their corresponding SkSL GLSL code generation paths.
