### Compilation failed:

error: 17: type mismatch: '=' cannot operate on 'uint', 'float'
void   u_eq_float_literal_disallowed()     { u = 1.0; }
                                             ^^^^^^^
error: 18: type mismatch: '=' cannot operate on 'int', 'uint'
void   i_eq_u_disallowed()                 { i = u; }
                                             ^^^^^
error: 19: type mismatch: '=' cannot operate on 'float', 'uint'
void   f_eq_u_disallowed()                 { f = u; }
                                             ^^^^^
error: 20: type mismatch: '=' cannot operate on 'uint', 'int'
void   u_eq_i_disallowed()                 { u = i; }
                                             ^^^^^
error: 21: type mismatch: '=' cannot operate on 'uint', 'float'
void   u_eq_f_disallowed()                 { u = f; }
                                             ^^^^^
error: 22: type mismatch: '+' cannot operate on 'uint', 'float'
void   u_plus_float_literal_disallowed()   { u + 1.0; }
                                             ^^^^^^^
error: 23: type mismatch: '-' cannot operate on 'uint', 'float'
void   u_minus_float_literal_disallowed()  { u - 1.0; }
                                             ^^^^^^^
error: 24: type mismatch: '*' cannot operate on 'uint', 'float'
void   u_mul_float_literal_disallowed()    { u * 1.0; }
                                             ^^^^^^^
error: 25: type mismatch: '/' cannot operate on 'uint', 'float'
void   u_div_float_literal_disallowed()    { u / 1.0; }
                                             ^^^^^^^
error: 26: type mismatch: '+' cannot operate on 'float', 'uint'
void   float_literal_plus_u_disallowed()   { 1.0 + u; }
                                             ^^^^^^^
error: 27: type mismatch: '-' cannot operate on 'float', 'uint'
void   float_literal_minus_u_disallowed()  { 1.0 - u; }
                                             ^^^^^^^
error: 28: type mismatch: '*' cannot operate on 'float', 'uint'
void   float_literal_mul_u_disallowed()    { 1.0 * u; }
                                             ^^^^^^^
error: 29: type mismatch: '/' cannot operate on 'float', 'uint'
void   float_literal_div_u_disallowed()    { 1.0 / u; }
                                             ^^^^^^^
error: 30: type mismatch: '+' cannot operate on 'uint', 'float'
void   u_plus_f_disallowed()               { u + f; }
                                             ^^^^^
error: 31: type mismatch: '-' cannot operate on 'uint', 'float'
void   u_minus_f_disallowed()              { u - f; }
                                             ^^^^^
error: 32: type mismatch: '*' cannot operate on 'uint', 'float'
void   u_mul_f_disallowed()                { u * f; }
                                             ^^^^^
error: 33: type mismatch: '/' cannot operate on 'uint', 'float'
void   u_div_f_disallowed()                { u / f; }
                                             ^^^^^
error: 34: type mismatch: '+' cannot operate on 'float', 'uint'
void   f_plus_u_disallowed()               { f + u; }
                                             ^^^^^
error: 35: type mismatch: '-' cannot operate on 'float', 'uint'
void   f_minus_u_disallowed()              { f - u; }
                                             ^^^^^
error: 36: type mismatch: '*' cannot operate on 'float', 'uint'
void   f_mul_u_disallowed()                { f * u; }
                                             ^^^^^
error: 37: type mismatch: '/' cannot operate on 'float', 'uint'
void   f_div_u_disallowed()                { f / u; }
                                             ^^^^^
error: 38: type mismatch: '+' cannot operate on 'int', 'uint'
void   i_plus_u_disallowed()               { i + u; }
                                             ^^^^^
error: 39: type mismatch: '-' cannot operate on 'int', 'uint'
void   i_minus_u_disallowed()              { i - u; }
                                             ^^^^^
error: 40: type mismatch: '*' cannot operate on 'int', 'uint'
void   i_mul_u_disallowed()                { i * u; }
                                             ^^^^^
error: 41: type mismatch: '/' cannot operate on 'int', 'uint'
void   i_div_u_disallowed()                { i / u; }
                                             ^^^^^
error: 42: type mismatch: '+' cannot operate on 'uint', 'int'
void   u_plus_i_disallowed()               { u + i; }
                                             ^^^^^
error: 43: type mismatch: '-' cannot operate on 'uint', 'int'
void   u_minus_i_disallowed()              { u - i; }
                                             ^^^^^
error: 44: type mismatch: '*' cannot operate on 'uint', 'int'
void   u_mul_i_disallowed()                { u * i; }
                                             ^^^^^
error: 45: type mismatch: '/' cannot operate on 'uint', 'int'
void   u_div_i_disallowed()                { u / i; }
                                             ^^^^^
29 errors
