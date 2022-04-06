### Compilation failed:

error: 16: 'const' variable initializer must be a constant expression
void from_uniform()                      { const float x = u; }
                                                           ^
error: 17: 'const' variable initializer must be a constant expression
void from_parameter(float p)             { const float x = p; }
                                                           ^
error: 18: 'const' variable initializer must be a constant expression
void from_const_parameter(const float p) { const float x = p; }
                                                           ^
error: 19: 'const' variable initializer must be a constant expression
void from_non_const_local()              { float x = u; const float y = x; }
                                                                        ^
error: 20: 'const' variable initializer must be a constant expression
void from_non_const_expression()         { const float x = u + u; }
                                                           ^^^^^
error: 21: 'const' variable initializer must be a constant expression
void from_mixed_expression()             { const float x = c + u; }
                                                           ^^^^^
error: 22: 'const' variable initializer must be a constant expression
void from_non_const_struct_field()       { const float x = s.f; }
                                                           ^^^
7 errors
