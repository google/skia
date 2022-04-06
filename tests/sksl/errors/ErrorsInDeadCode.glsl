### Compilation failed:

error: 3: unknown identifier 'x'
void error_in_dead_else_body()          { if (true) {} else x = 5; }
                                                            ^
error: 4: unknown identifier 'x'
void error_in_dead_if_body()            { if (false) x = 5; }
                                                     ^
error: 5: unknown identifier 'x'
void error_in_dead_ternary_true_expr()  { true ? 5 : x; }
                                                     ^
error: 6: unknown identifier 'x'
void error_in_dead_ternary_false_expr() { false ? x : 5; }
                                                  ^
4 errors
