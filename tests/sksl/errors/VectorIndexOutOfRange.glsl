### Compilation failed:

error: 1: index -1 out of range for 'half4'
void half4_neg1()              { half4 h;        half    v = h[-1]; }
                                                               ^^
error: 6: index 4 out of range for 'half4'
void half4_4()                 { half4 h;        half    v = h[4]; }
                                                               ^
error: 7: index 1000000000 out of range for 'half4'
void half4_huge()              { half4 h;        half    v = h[1000000000]; }
                                                               ^^^^^^^^^^
error: 9: index -1 out of range for 'half3'
void half3_neg1()              { half3 h;        half    v = h[-1]; }
                                                               ^^
error: 13: index 3 out of range for 'half3'
void half3_3()                 { half3 h;        half    v = h[3]; }
                                                               ^
error: 14: index 4 out of range for 'half3'
void half3_4()                 { half3 h;        half    v = h[4]; }
                                                               ^
error: 15: index 1000000000 out of range for 'half3'
void half3_huge()              { half3 h;        half    v = h[1000000000]; }
                                                               ^^^^^^^^^^
error: 17: index -1 out of range for 'half2'
void half2_neg1()              { half2 h;        half    v = h[-1]; }
                                                               ^^
error: 20: index 2 out of range for 'half2'
void half2_2()                 { half2 h;        half    v = h[2]; }
                                                               ^
error: 21: index 3 out of range for 'half2'
void half2_3()                 { half2 h;        half    v = h[3]; }
                                                               ^
error: 22: index 4 out of range for 'half2'
void half2_4()                 { half2 h;        half    v = h[4]; }
                                                               ^
error: 23: index 1000000000 out of range for 'half2'
void half2_huge()              { half2 h;        half    v = h[1000000000]; }
                                                               ^^^^^^^^^^
error: 25: index -1 out of range for 'half2'
void half2_neg1_constidx()     { half2 h;        const int INDEX = -1;         half v = h[INDEX]; }
                                                                                          ^^^^^
error: 28: index 2 out of range for 'half2'
void half2_2_constidx()        { half2 h;        const int INDEX = 2;          half v = h[INDEX]; }
                                                                                          ^^^^^
error: 29: index 1000000000 out of range for 'half2'
void half2_huge_constidx()     { half2 h;        const int INDEX = 1000000000; half v = h[INDEX]; }
                                                                                          ^^^^^
error: 31: index -1 out of range for 'half3'
void half3_neg1_constidx()     { half3 h;        const int INDEX = -1;         half v = h[INDEX]; }
                                                                                          ^^^^^
error: 35: index 3 out of range for 'half3'
void half3_3_constidx()        { half3 h;        const int INDEX = 3;          half v = h[INDEX]; }
                                                                                          ^^^^^
error: 36: index 1000000000 out of range for 'half3'
void half3_huge_constidx()     { half3 h;        const int INDEX = 1000000000; half v = h[INDEX]; }
                                                                                          ^^^^^
error: 38: index -1 out of range for 'half4'
void half4_neg1_constidx()     { half4 h;        const int INDEX = -1;         half v = h[INDEX]; }
                                                                                          ^^^^^
error: 43: index 4 out of range for 'half4'
void half4_4_constidx()        { half4 h;        const int INDEX = 4;          half v = h[INDEX]; }
                                                                                          ^^^^^
error: 44: index 1000000000 out of range for 'half4'
void half4_huge_constidx()     { half4 h;        const int INDEX = 1000000000; half v = h[INDEX]; }
                                                                                          ^^^^^
21 errors
