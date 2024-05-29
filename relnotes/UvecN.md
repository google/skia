SkSL now properly recognizes the types `uvec2`, `uvec3` or `uvec4`.

Unsigned types are not supported in Runtime Effects, as they did not exist in GLSL ES2; however,
SkSL should still recognize these typenames and reject them if they are used in a program.
That is, we should not allow `uvec3` to be used as a variable or function name. We will now properly
detect and reject this as an error.
