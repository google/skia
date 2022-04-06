### Compilation failed:

error: 1: index -1 out of range for 'half2x4'
void half2x4_neg1()            { half2x4 m;      half4   v = m[-1]; }
                                                               ^^
error: 4: index 2 out of range for 'half2x4'
void half2x4_2()               { half2x4 m;      half4   v = m[2]; }
                                                               ^
error: 5: index 3 out of range for 'half2x4'
void half2x4_3()               { half2x4 m;      half4   v = m[3]; }
                                                               ^
error: 6: index 4 out of range for 'half2x4'
void half2x4_4()               { half2x4 m;      half4   v = m[4]; }
                                                               ^
error: 7: index 1000000000 out of range for 'half2x4'
void half2x4_huge()            { half2x4 m;      half4   v = m[1000000000]; }
                                                               ^^^^^^^^^^
error: 9: index -1 out of range for 'half4x2'
void half4x2_neg1()            { half4x2 m;      half2   v = m[-1]; }
                                                               ^^
error: 14: index 4 out of range for 'half4x2'
void half4x2_4()               { half4x2 m;      half2   v = m[4]; }
                                                               ^
error: 15: index 1000000000 out of range for 'half4x2'
void half4x2_huge()            { half4x2 m;      half2   v = m[1000000000]; }
                                                               ^^^^^^^^^^
8 errors
