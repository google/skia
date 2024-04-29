### Compilation failed:

error: 3: type 'S' does not have a field named 'missing'
void not_a_field()    { S s; s.missing = 123; }
                             ^^^^^^^^^
error: 4: not a function
void not_a_function() { S s; s.f(); }
                             ^^^^^
error: 5: type mismatch: '=' cannot operate on 'float', 'bool3'
void not_a_bvec()     { S s; s.f = bool3(true); }
                             ^^^^^^^^^^^^^^^^^
error: 6: too many components in swizzle mask
void not_a_struct()   { S s; s.f.missing; }
                                     ^^^
error: 7: expected array, but found 'float'
void not_an_array()   { S s; s.f[0]; }
                             ^^^
5 errors
