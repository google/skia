SkSL now allows the ++ and -- operators on vector and matrix variables.

Previously, attempting to use these operators on a vector or matrix would lead to an error. This was
a violation of the GLSL expression rules (5.9): "The arithmetic unary operators negate (-), post-
and pre-increment and decrement (-- and ++) operate on integer or floating-point values (including
vectors and matrices)."
