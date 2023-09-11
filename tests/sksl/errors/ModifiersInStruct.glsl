### Compilation failed:

error: 7: type 'bool' does not support qualifier 'readonly'
    readonly bool f;
    ^^^^^^^^
error: 8: type 'bool' does not support qualifier 'writeonly'
    writeonly bool g;
    ^^^^^^^^^
error: 2: modifier 'const' is not permitted on a struct field
    const float a;
    ^^^^^^^^^^^^^
error: 3: modifier 'uniform' is not permitted on a struct field
    uniform int b;
    ^^^^^^^^^^^^^
error: 4: modifier 'flat' is not permitted on a struct field
    flat half4 c;
    ^^^^^^^^^^^^
error: 5: modifier 'noperspective' is not permitted on a struct field
    noperspective float4 d;
    ^^^^^^^^^^^^^^^^^^^^^^
error: 6: modifier 'inout' is not permitted on a struct field
    inout bool e;
    ^^^^^^^^^^^^
error: 9: modifier 'buffer' is not permitted on a struct field
    buffer int h;
    ^^^^^^^^^^^^
error: 10: modifier 'pixel_local' is not permitted on a struct field
    pixel_local float i;
    ^^^^^^^^^^^^^^^^^^^
9 errors
