### Compilation failed:

error: 11: cannot assign to this expression
void assign_to_literal()                      { 1 = 2; }
                                                ^
error: 12: cannot modify immutable variable 'u'
void assign_to_uniform()                      { u = 0; }
                                                ^
error: 13: cannot modify immutable variable 'x'
void assign_to_const()                        { const int x = 1; x = 0; }
                                                                 ^
error: 15: cannot assign to this expression
void assign_to_const_swizzle()                { const half4 x = half4(1); x.w = 0; }
                                                                          ^^^
error: 16: cannot write to the same swizzle field more than once
void assign_to_repeated_swizzle()             { half4 x; x.yy = half2(0); }
                                                         ^^^^
error: 18: cannot modify immutable variable 'l'
void assign_to_foldable_ternary_const_left()  { const float l = 1; float r; (true ? l : r) = 0; }
                                                                            ^^^^^^^^^^^^^^
error: 19: cannot modify immutable variable 'r'
void assign_to_foldable_ternary_const_right() { float l; const float r = 1; (false ? l : r) = 0; }
                                                                            ^^^^^^^^^^^^^^^
error: 20: cannot modify immutable variable 'l'
void assign_to_foldable_ternary_const_both()  { const float l = 1; const float r = 1; (true ? l : r) = 0; }
                                                                                      ^^^^^^^^^^^^^^
error: 21: cannot assign to this expression
void assign_to_unfoldable_ternary()           { float l, r; (u > 0 ? l : r) = 0; }
                                                            ^^^^^^^^^^^^^^^
error: 22: cannot assign to this expression
void assign_to_unary_minus()                  { float x; -x = 0; }
                                                         ^^
error: 25: cannot modify immutable variable 'x'
void assign_to_const_param(const int x)             { x = 0; }
                                                      ^
error: 26: cannot modify immutable variable 'x'
void assign_to_const_array_param(const int x[1])    { x[0] = 0; }
                                                      ^
error: 27: cannot modify immutable variable 's.f'
void assign_to_const_struct_param(const S s)        { s.f = 0; }
                                                      ^
error: 28: cannot modify immutable variable 't.s'
void assign_to_const_nested_struct_param(const T t) { t.s.f = 0; }
                                                      ^
14 errors
