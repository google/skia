### Compilation failed:

error: 1: index -1 out of range for 'half3x3'
void half3x3_neg1()            { half3x3 m;  half3   v = m[-1]; }
                                                           ^^
error: 5: index 3 out of range for 'half3x3'
void half3x3_3()               { half3x3 m;  half3   v = m[3]; }
                                                           ^
error: 6: index 4 out of range for 'half3x3'
void half3x3_4()               { half3x3 m;  half3   v = m[4]; }
                                                           ^
error: 7: index 1000000000 out of range for 'half3x3'
void half3x3_huge()            { half3x3 m;  half3   v = m[1000000000]; }
                                                           ^^^^^^^^^^
error: 9: index -1 out of range for 'half3x3'
void half3x3_neg1_constidx()   { half3x3 m;  const int INDEX = -1;         half3 v = m[INDEX]; }
                                                                                       ^^^^^
error: 13: index 3 out of range for 'half3x3'
void half3x3_3_constidx()      { half3x3 m;  const int INDEX = 3;          half3 v = m[INDEX]; }
                                                                                       ^^^^^
error: 14: index 4 out of range for 'half3x3'
void half3x3_4_constidx()      { half3x3 m;  const int INDEX = 4;          half3 v = m[INDEX]; }
                                                                                       ^^^^^
error: 15: index 1000000000 out of range for 'half3x3'
void half3x3_huge_constidx()   { half3x3 m;  const int INDEX = 1000000000; half3 v = m[INDEX]; }
                                                                                       ^^^^^
8 errors
