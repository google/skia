### Compilation failed:

error: 5: type mismatch: '==' cannot operate on 'float2', 'float'
    return v == f || v == m || m == f ||
           ^^^^^^
error: 5: type mismatch: '==' cannot operate on 'float2', 'float2x2'
    return v == f || v == m || m == f ||
                     ^^^^^^
error: 5: type mismatch: '==' cannot operate on 'float2x2', 'float'
    return v == f || v == m || m == f ||
                               ^^^^^^
error: 6: type mismatch: '==' cannot operate on 'float', 'float2'
           f == v || m == v || f == m ||
           ^^^^^^
error: 6: type mismatch: '==' cannot operate on 'float2x2', 'float2'
           f == v || m == v || f == m ||
                     ^^^^^^
error: 6: type mismatch: '==' cannot operate on 'float', 'float2x2'
           f == v || m == v || f == m ||
                               ^^^^^^
error: 7: type mismatch: '!=' cannot operate on 'float2', 'float'
           v != f || v != m || m != f ||
           ^^^^^^
error: 7: type mismatch: '!=' cannot operate on 'float2', 'float2x2'
           v != f || v != m || m != f ||
                     ^^^^^^
error: 7: type mismatch: '!=' cannot operate on 'float2x2', 'float'
           v != f || v != m || m != f ||
                               ^^^^^^
error: 8: type mismatch: '!=' cannot operate on 'float', 'float2'
           f != v || m != v || f != m ;
           ^^^^^^
error: 8: type mismatch: '!=' cannot operate on 'float2x2', 'float2'
           f != v || m != v || f != m ;
                     ^^^^^^
error: 8: type mismatch: '!=' cannot operate on 'float', 'float2x2'
           f != v || m != v || f != m ;
                               ^^^^^^
error: 14: type mismatch: '==' cannot operate on 'float2x2', 'float3x3'
    return m2 == m3 || m2 != m3;
           ^^^^^^^^
error: 14: type mismatch: '!=' cannot operate on 'float2x2', 'float3x3'
    return m2 == m3 || m2 != m3;
                       ^^^^^^^^
error: 20: type mismatch: '==' cannot operate on 'float2', 'float3'
    return v2 == v3 || v2 != v3;
           ^^^^^^^^
error: 20: type mismatch: '!=' cannot operate on 'float2', 'float3'
    return v2 == v3 || v2 != v3;
                       ^^^^^^^^
16 errors
