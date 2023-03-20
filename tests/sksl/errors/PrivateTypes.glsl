### Compilation failed:

error: 2: name '$bvec' is reserved
    $bvec bv;
    ^^^^^
error: 2: type '$bvec' is generic
    $bvec bv;
    ^^^^^
error: 5: name '$ivec' is reserved
void param_private($ivec iv) {}
                   ^^^^^
error: 5: type '$ivec' is generic
void param_private($ivec iv) {}
                   ^^^^^
error: 6: name '$genType' is reserved
void vardecl_private()       { $genType g; }
                               ^^^^^^^^
error: 6: type '$genType' is generic
void vardecl_private()       { $genType g; }
                               ^^^^^^^^
error: 7: name '$mat' is reserved
void ctor_mat_private()      { $mat(0); }
                               ^^^^
error: 7: type '$mat' is generic
void ctor_mat_private()      { $mat(0); }
                               ^^^^
error: 8: name '$floatLiteral' is reserved
void ctor_literal_private()  { $floatLiteral(0); }
                               ^^^^^^^^^^^^^
error: 8: type '$floatLiteral' is generic
void ctor_literal_private()  { $floatLiteral(0); }
                               ^^^^^^^^^^^^^
10 errors
