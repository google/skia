- SkSL will now properly reject sequence-expressions containing arrays, or sequence-expressions
  containing structures of arrays. Previously, the left-side expression of a sequence was checked,
  but the right-side was not. In GLSL ES 1.0, and therefore in SkSL, the only operator which is
  allowed to operate on arrays is the array subscript operator (`[]`).
