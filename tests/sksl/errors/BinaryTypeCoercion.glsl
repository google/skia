### Compilation failed:

error: 11: type mismatch: '=' cannot operate on 'half', 'float'
void scalar_times_scalar_disallowed_1() { H = F * H; }
                                          ^^^^^^^^^
error: 12: type mismatch: '=' cannot operate on 'half', 'float'
void scalar_times_scalar_disallowed_2() { H = H * F; }
                                          ^^^^^^^^^
error: 13: type mismatch: '*=' cannot operate on 'half', 'float'
void scalar_times_scalar_disallowed_3() { H *= F;    }
                                          ^^^^^^
error: 18: type mismatch: '=' cannot operate on 'half4', 'float4'
void vector_times_vector_disallowed_1() { H4 = F4 * H4; }
                                          ^^^^^^^^^^^^
error: 19: type mismatch: '=' cannot operate on 'half4', 'float4'
void vector_times_vector_disallowed_2() { H4 = H4 * F4; }
                                          ^^^^^^^^^^^^
error: 20: type mismatch: '*=' cannot operate on 'half4', 'float4'
void vector_times_vector_disallowed_3() { H4 *= F4;     }
                                          ^^^^^^^^
error: 24: type mismatch: '=' cannot operate on 'half4', 'float4'
void scalar_times_vector_disallowed_1() { H4 = F * H4; }
                                          ^^^^^^^^^^^
error: 25: type mismatch: '=' cannot operate on 'half4', 'float4'
void scalar_times_vector_disallowed_2() { H4 = H * F4; }
                                          ^^^^^^^^^^^
error: 30: type mismatch: '=' cannot operate on 'half4', 'float4'
void vector_times_scalar_disallowed_1() { H4 = F4 * H; }
                                          ^^^^^^^^^^^
error: 31: type mismatch: '=' cannot operate on 'half4', 'float4'
void vector_times_scalar_disallowed_2() { H4 = H4 * F; }
                                          ^^^^^^^^^^^
error: 32: type mismatch: '*=' cannot operate on 'half4', 'float'
void vector_times_scalar_disallowed_3() { H4 *= F;     }
                                          ^^^^^^^
error: 36: type mismatch: '=' cannot operate on 'half4', 'float4'
void matrix_times_vector_disallowed_1() { H4 = F4x4 * H4; }
                                          ^^^^^^^^^^^^^^
error: 37: type mismatch: '=' cannot operate on 'half4', 'float4'
void matrix_times_vector_disallowed_2() { H4 = H4x4 * F4; }
                                          ^^^^^^^^^^^^^^
error: 42: type mismatch: '=' cannot operate on 'half4', 'float4'
void vector_times_matrix_disallowed_1() { H4 = F4 * H4x4; }
                                          ^^^^^^^^^^^^^^
error: 43: type mismatch: '=' cannot operate on 'half4', 'float4'
void vector_times_matrix_disallowed_2() { H4 = H4 * F4x4; }
                                          ^^^^^^^^^^^^^^
error: 44: type mismatch: '*=' cannot operate on 'half4', 'float4x4'
void vector_times_matrix_disallowed_3() { H4 *= F4x4;     }
                                          ^^^^^^^^^^
error: 49: type mismatch: '=' cannot operate on 'half4x4', 'float4x4'
void matrix_times_matrix_disallowed_1() { H4x4 = F4x4 * H4x4; }
                                          ^^^^^^^^^^^^^^^^^^
error: 50: type mismatch: '=' cannot operate on 'half4x4', 'float4x4'
void matrix_times_matrix_disallowed_2() { H4x4 = H4x4 * F4x4; }
                                          ^^^^^^^^^^^^^^^^^^
error: 51: type mismatch: '*=' cannot operate on 'half4x4', 'float4x4'
void matrix_times_matrix_disallowed_3() { H4x4 *= F4x4;       }
                                          ^^^^^^^^^^^^
error: 55: type mismatch: '*=' cannot operate on 'float4x4', 'float4'
void dimensions_disallowed_1()          { F4x4 *= F4;   }
                                          ^^^^^^^^^^
20 errors
