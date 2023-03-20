### Compilation failed:

error: 1: array size must be positive
void a1() { float[-2]; }
                  ^^
error: 2: array size must be positive
void b1() { float[-1]; }
                  ^^
error: 3: array size must be positive
void c1() { float[0]; }
                  ^
error: 4: expected 'int', but found 'float'
void d1() { float[1.5]; }
                  ^^^
error: 5: value is out of range for type 'int': 4000000000
void e1() { float[4000000000]; }
                  ^^^^^^^^^^
error: 5: array size must be positive
void e1() { float[4000000000]; }
                  ^^^^^^^^^^
error: 6: expected 'int', but found 'bool'
void f1() { float[true]; }
                  ^^^^
error: 7: expected 'int', but found 'bool'
void g1() { float[false]; }
                  ^^^^^
error: 8: expected 'int', but found 'int2'
void h1() { float[int2(2, 2)]; }
                  ^^^^^^^^^^
error: 9: missing index in '[]'
void i1() { float[]; }
                 ^^
error: 10: value is out of range for type 'int': 4000000000
void j1() { float[int3(4000000000)]; }
                       ^^^^^^^^^^
error: 11: value is out of range for type 'int': 100000002004087734272
void k1() { float[int(1e20)]; }
                      ^^^^
error: 13: array size must be positive
void a2() { float x[-2]; }
                    ^^
error: 14: array size must be positive
void b2() { float x[-1]; }
                    ^^
error: 15: array size must be positive
void c2() { float x[0]; }
                    ^
error: 16: array size must be an integer
void d2() { float x[1.5]; }
                    ^^^
error: 17: array size out of bounds
void e2() { float x[4000000000]; }
                    ^^^^^^^^^^
error: 18: array size must be an integer
void f2() { float x[true]; }
                    ^^^^
error: 19: array size must be an integer
void g2() { float x[false]; }
                    ^^^^^
error: 20: array size must be an integer
void h2() { float x[int2(2, 2)]; }
                    ^^^^^^^^^^
error: 21: unsized arrays are not permitted here
void i2() { float x[]; }
            ^^^^^^^^^
error: 22: value is out of range for type 'int': 4000000000
void j2() { float x[int3(4000000000)]; }
                         ^^^^^^^^^^
error: 23: value is out of range for type 'int': 100000002004087734272
void k2() { float x[int(1e20)]; }
                        ^^^^
23 errors
