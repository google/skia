### Compilation failed:

error: 7: 'half4' is not a valid parameter to 'half3' constructor; use '.xyz' instead
const half3 h3 = half3(h4);
                 ^^^^^^^^^
error: 8: 'half4' is not a valid parameter to 'half2' constructor; use '.xy' instead
const half2 h2 = half2(h4);
                 ^^^^^^^^^
error: 9: 'half4' is not a valid parameter to 'half' constructor; use '.x' instead
const half  h  = half (h4);
                 ^^^^^^^^^
error: 12: 'int4' is not a valid parameter to 'int3' constructor; use '.xyz' instead
const int3  i3 = int3(i4);
                 ^^^^^^^^
error: 13: 'int4' is not a valid parameter to 'int2' constructor; use '.xy' instead
const int2  i2 = int2(i4);
                 ^^^^^^^^
error: 14: 'int4' is not a valid parameter to 'int' constructor; use '.x' instead
const int   i  = int (i4);
                 ^^^^^^^^
error: 17: 'bool4' is not a valid parameter to 'bool3' constructor; use '.xyz' instead
const bool3 b3 = bool3(b4);
                 ^^^^^^^^^
error: 18: 'bool4' is not a valid parameter to 'bool2' constructor; use '.xy' instead
const bool2 b2 = bool2(b4);
                 ^^^^^^^^^
error: 19: 'bool4' is not a valid parameter to 'bool' constructor; use '.x' instead
const bool  b  = bool (b4);
                 ^^^^^^^^^
9 errors
