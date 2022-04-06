### Compilation failed:

error: 2: type '$bvec' is private
    $bvec bv;
    ^^^^^
error: 5: type '$ivec' is private
void param_private($ivec iv) {}
                   ^^^^^
error: 6: type '$genType' is private
void vardecl_private()       { $genType g; }
                               ^^^^^^^^
error: 7: type '$mat' is private
void ctor_mat_private()      { $mat(0); }
                               ^^^^
error: 8: type '$floatLiteral' is private
void ctor_literal_private()  { $floatLiteral(0); }
                               ^^^^^^^^^^^^^
5 errors
