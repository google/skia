### Compilation failed:

error: 16: type mismatch: '=' cannot operate on 'int', 'float'
void   i_eq_float_literal_disallowed()     { i = 1.0; }
                                             ^^^^^^^
error: 17: type mismatch: '=' cannot operate on 'int', 'float'
void   i_eq_f_disallowed()                 { i = f; }
                                             ^^^^^
error: 18: type mismatch: '=' cannot operate on 'float', 'int'
void   f_eq_i_disallowed()                 { f = i; }
                                             ^^^^^
error: 19: type mismatch: '+' cannot operate on 'int', 'float'
void   i_plus_float_literal_disallowed()   { i + 1.0; }
                                             ^^^^^^^
error: 20: type mismatch: '-' cannot operate on 'int', 'float'
void   i_minus_float_literal_disallowed()  { i - 1.0; }
                                             ^^^^^^^
error: 21: type mismatch: '*' cannot operate on 'int', 'float'
void   i_mul_float_literal_disallowed()    { i * 1.0; }
                                             ^^^^^^^
error: 22: type mismatch: '/' cannot operate on 'int', 'float'
void   i_div_float_literal_disallowed()    { i / 1.0; }
                                             ^^^^^^^
error: 23: type mismatch: '+' cannot operate on 'float', 'int'
void   float_literal_plus_i_disallowed()   { 1.0 + i; }
                                             ^^^^^^^
error: 24: type mismatch: '-' cannot operate on 'float', 'int'
void   float_literal_minus_i_disallowed()  { 1.0 - i; }
                                             ^^^^^^^
error: 25: type mismatch: '*' cannot operate on 'float', 'int'
void   float_literal_mul_i_disallowed()    { 1.0 * i; }
                                             ^^^^^^^
error: 26: type mismatch: '/' cannot operate on 'float', 'int'
void   float_literal_div_i_disallowed()    { 1.0 / i; }
                                             ^^^^^^^
error: 27: type mismatch: '+' cannot operate on 'int', 'float'
void   i_plus_f_disallowed()               { i + f; }
                                             ^^^^^
error: 28: type mismatch: '-' cannot operate on 'int', 'float'
void   i_minus_f_disallowed()              { i - f; }
                                             ^^^^^
error: 29: type mismatch: '*' cannot operate on 'int', 'float'
void   i_mul_f_disallowed()                { i * f; }
                                             ^^^^^
error: 30: type mismatch: '/' cannot operate on 'int', 'float'
void   i_div_f_disallowed()                { i / f; }
                                             ^^^^^
error: 31: type mismatch: '+' cannot operate on 'float', 'int'
void   f_plus_i_disallowed()               { f + i; }
                                             ^^^^^
error: 32: type mismatch: '-' cannot operate on 'float', 'int'
void   f_minus_i_disallowed()              { f - i; }
                                             ^^^^^
error: 33: type mismatch: '*' cannot operate on 'float', 'int'
void   f_mul_i_disallowed()                { f * i; }
                                             ^^^^^
error: 34: type mismatch: '/' cannot operate on 'float', 'int'
void   f_div_i_disallowed()                { f / i; }
                                             ^^^^^
error: 35: unknown identifier 'u'
void   f_plus_u_disallowed()               { f + u; }
                                                 ^
error: 36: unknown identifier 'u'
void   f_minus_u_disallowed()              { f - u; }
                                                 ^
error: 37: unknown identifier 'u'
void   f_mul_u_disallowed()                { f * u; }
                                                 ^
error: 38: unknown identifier 'u'
void   f_div_u_disallowed()                { f / u; }
                                                 ^
error: 39: unknown identifier 'u'
void   i_plus_u_disallowed()               { i + u; }
                                                 ^
error: 40: unknown identifier 'u'
void   i_minus_u_disallowed()              { i - u; }
                                                 ^
error: 41: unknown identifier 'u'
void   i_mul_u_disallowed()                { i * u; }
                                                 ^
error: 42: unknown identifier 'u'
void   i_div_u_disallowed()                { i / u; }
                                                 ^
27 errors
