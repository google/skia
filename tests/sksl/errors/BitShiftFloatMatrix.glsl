### Compilation failed:

error: 5: type mismatch: '>>=' cannot operate on 'float2x2', 'int'
void shr_eq() { x >>= 1; }
                ^^^^^^^
error: 6: type mismatch: '<<=' cannot operate on 'float2x2', 'int'
void shl_eq() { x <<= 1; }
                ^^^^^^^
error: 7: type mismatch: '&=' cannot operate on 'float2x2', 'int'
void and_eq() { x &=  1; }
                ^^^^^^^
error: 8: type mismatch: '|=' cannot operate on 'float2x2', 'int'
void or_eq()  { x |=  1; }
                ^^^^^^^
error: 9: type mismatch: '^=' cannot operate on 'float2x2', 'int'
void xor_eq() { x ^=  1; }
                ^^^^^^^
error: 11: type mismatch: '>>' cannot operate on 'float2x2', 'int'
void shr() { x = x >> 1; }
                 ^^^^^^
error: 12: type mismatch: '<<' cannot operate on 'float2x2', 'int'
void shl() { x = x << 1; }
                 ^^^^^^
error: 13: type mismatch: '&' cannot operate on 'float2x2', 'int'
void and() { x = x & 1; }
                 ^^^^^
error: 14: type mismatch: '|' cannot operate on 'float2x2', 'int'
void or()  { x = x | 1; }
                 ^^^^^
error: 15: type mismatch: '^' cannot operate on 'float2x2', 'int'
void xor() { x = x ^ 1; }
                 ^^^^^
10 errors
