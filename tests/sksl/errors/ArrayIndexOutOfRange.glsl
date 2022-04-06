### Compilation failed:

error: 1: index -1 out of range for 'int[123]'
void array_neg1             () { int     a[123]; int     v = a[-1]; }
                                                               ^^
error: 4: index 123 out of range for 'half4x4[123]'
void array_123              () { half4x4 a[123]; half4x4 v = a[123]; }
                                                               ^^^
error: 5: index 1000000000 out of range for 'int4[123]'
void array_huge             () { int4    a[123]; int4    v = a[1000000000]; }
                                                               ^^^^^^^^^^
error: 6: index 3000000000 out of range for 'half3[123]'
void array_overflow         () { half3   a[123]; half3   v = a[3000000000]; }
                                                               ^^^^^^^^^^
error: 7: missing index in '[]'
void array_no_index         () { int     a[123]; int     v = a[]; }
                                                              ^^
5 errors
