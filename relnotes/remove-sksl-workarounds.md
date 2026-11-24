Removed the unused `disable_dual_source_blending_support` (`DISABLE_DUAL_SOURCE_BLENDING_SUPPORT`),
`emulate_abs_int_function` (`EMULATE_ABS_INT_FUNCTION`),
`remove_pow_with_constant_exponent` (`REMOVE_POW_WITH_CONSTANT_EXPONENT`),
`rewrite_do_while_loops` (`REWRITE_DO_WHILE_LOOPS`), and
`unfold_short_circuit_as_ternary_operation` (`UNFOLD_SHORT_CIRCUIT_AS_TERNARY_OPERATION`)
driver bug workarounds from `GrDriverBugWorkarounds` and `GrDriverBugWorkaroundType`, along with
their corresponding SkSL GLSL code generation paths.
