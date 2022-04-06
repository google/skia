### Compilation failed:

error: 7: cannot assign to this expression
void assign_to_literal()                      { 1 = 2; }
                                                ^
error: 8: cannot modify immutable variable 'u'
void assign_to_uniform()                      { u = 0; }
                                                ^
error: 9: cannot modify immutable variable 'x'
void assign_to_const()                        { const int x = 1; x = 0; }
                                                                 ^
error: 11: cannot assign to this expression
void assign_to_const_swizzle()                { const half4 x = half4(1); x.w = 0; }
                                                                          ^^^
error: 12: cannot write to the same swizzle field more than once
void assign_to_repeated_swizzle()             { half4 x; x.yy = half2(0); }
                                                         ^^^^
error: 14: cannot modify immutable variable 'l'
void assign_to_foldable_ternary_const_left()  { const float l = 1; float r; (true ? l : r) = 0; }
                                                                            ^^^^^^^^^^^^^^
error: 15: cannot modify immutable variable 'r'
void assign_to_foldable_ternary_const_right() { float l; const float r = 1; (false ? l : r) = 0; }
                                                                            ^^^^^^^^^^^^^^^
error: 16: cannot modify immutable variable 'l'
void assign_to_foldable_ternary_const_both()  { const float l = 1; const float r = 1; (true ? l : r) = 0; }
                                                                                      ^^^^^^^^^^^^^^
error: 17: cannot assign to this expression
void assign_to_unfoldable_ternary()           { float l, r; (u > 0 ? l : r) = 0; }
                                                            ^^^^^^^^^^^^^^^
error: 18: cannot assign to this expression
void assign_to_unary_minus()                  { float x; -x = 0; }
                                                         ^^
error: 21: cannot modify immutable variable 'x'
void assign_to_const_param(const int x)          { x = 0; }
                                                   ^
error: 22: cannot modify immutable variable 'x'
void assign_to_const_array_param(const int x[1]) { x[0] = 0; }
                                                   ^
error: 23: cannot modify immutable variable 's'
void assign_to_const_struct_param(const S s)     { s.f = 0; }
                                                   ^
13 errors
