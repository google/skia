### Compilation failed:

error: 1: type mismatch: '+' cannot operate on 'bool', 'bool'
bool2 add_boolean()    { return false + true; }
                                ^^^^^^^^^^^^
error: 2: type mismatch: '-' cannot operate on 'bool', 'bool'
bool2 sub_boolean()    { return false - true; }
                                ^^^^^^^^^^^^
error: 3: type mismatch: '*' cannot operate on 'bool', 'bool'
bool2 mul_boolean()    { return false * true; }
                                ^^^^^^^^^^^^
error: 4: type mismatch: '/' cannot operate on 'bool', 'bool'
bool2 div_boolean()    { return false / true; }
                                ^^^^^^^^^^^^
error: 5: type mismatch: '%' cannot operate on 'bool', 'bool'
bool2 mod_boolean()    { return false % true; }
                                ^^^^^^^^^^^^
error: 6: type mismatch: '<<' cannot operate on 'bool', 'bool'
bool2 shl_boolean()    { return false << true; }
                                ^^^^^^^^^^^^^
error: 7: type mismatch: '>>' cannot operate on 'bool', 'bool'
bool2 shr_boolean()    { return false >> true; }
                                ^^^^^^^^^^^^^
error: 8: '-' cannot operate on 'bool'
bool2 neg_boolean()    { return -false; }
                                ^^^^^^
error: 9: '~' cannot operate on 'bool'
bool2 bitnot_boolean() { return ~false; }
                                ^^^^^^
error: 10: type mismatch: '&' cannot operate on 'bool', 'bool'
bool2 bitand_boolean() { return false & true; }
                                ^^^^^^^^^^^^
error: 11: type mismatch: '|' cannot operate on 'bool', 'bool'
bool2 bitor_boolean()  { return false | true; }
                                ^^^^^^^^^^^^
error: 12: type mismatch: '^' cannot operate on 'bool', 'bool'
bool2 bitxor_boolean() { return false ^ true; }
                                ^^^^^^^^^^^^
error: 14: type mismatch: '+' cannot operate on 'bool2', 'bool2'
bool2 add_boolean_vec()    { return bool2(false, false) + bool2(true, true); }
                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 15: type mismatch: '-' cannot operate on 'bool2', 'bool2'
bool2 sub_boolean_vec()    { return bool2(false, false) - bool2(true, true); }
                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 16: type mismatch: '*' cannot operate on 'bool2', 'bool2'
bool2 mul_boolean_vec()    { return bool2(false, false) * bool2(true, true); }
                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 17: type mismatch: '/' cannot operate on 'bool2', 'bool2'
bool2 div_boolean_vec()    { return bool2(false, false) / bool2(true, true); }
                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 18: type mismatch: '%' cannot operate on 'bool2', 'bool2'
bool2 mod_boolean_vec()    { return bool2(false, false) % bool2(true, true); }
                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 19: type mismatch: '<<' cannot operate on 'bool2', 'bool2'
bool2 shl_boolean_vec()    { return bool2(false, false) << bool2(true, true); }
                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 20: type mismatch: '>>' cannot operate on 'bool2', 'bool2'
bool2 shr_boolean_vec()    { return bool2(false, false) >> bool2(true, true); }
                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 21: '!' cannot operate on 'bool2'
bool2 not_boolean_vec()    { return !bool2(false, false); }
                                    ^^^^^^^^^^^^^^^^^^^^
error: 22: '-' cannot operate on 'bool2'
bool2 neg_boolean_vec()    { return -bool2(false, false); }
                                    ^^^^^^^^^^^^^^^^^^^^
error: 23: '~' cannot operate on 'bool2'
bool2 bitnot_boolean_vec() { return ~bool2(false, false); }
                                    ^^^^^^^^^^^^^^^^^^^^
error: 24: type mismatch: '&' cannot operate on 'bool2', 'bool2'
bool2 bitand_boolean_vec() { return bool2(false, false) & bool2(true, true); }
                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 25: type mismatch: '|' cannot operate on 'bool2', 'bool2'
bool2 bitor_boolean_vec()  { return bool2(false, false) | bool2(true, true); }
                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 26: type mismatch: '^' cannot operate on 'bool2', 'bool2'
bool2 bitxor_boolean_vec() { return bool2(false, false) ^ bool2(true, true); }
                                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
25 errors
