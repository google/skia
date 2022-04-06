### Compilation failed:

error: 3: expected '(' to begin constructor invocation
void expect_constructor_invocation()                   { int x = int; }
                                                                    ^
error: 4: expected '(' to begin constructor invocation
void expect_constructor_invocation_extra_initializer() { int x, y = int; }
                                                                       ^
error: 5: expected '(' to begin function call
void expect_function_call()                            { int x = func; }
                                                                     ^
error: 6: expected '(' to begin function call
void expect_function_call_extra_initializer()          { int x, y = func; }
                                                                        ^
error: 8: expected '(' to begin constructor invocation
int  g_expect_constructor_invocation                       = int;
                                                                ^
error: 9: expected '(' to begin constructor invocation
int  g_expect_constructor_invocation_extra_initializer, ix = int;
                                                                ^
error: 10: expected '(' to begin function call
int  g_expect_function_call                                = func;
                                                                 ^
error: 11: expected '(' to begin function call
int  g_expect_function_call_extra_initializer,          iy = func;
                                                                 ^
8 errors
