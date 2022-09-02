### Compilation failed:

error: 11: expected 'texture2D', but found 'readonlyTexture2D'
    needs_all_access(src);                      // BAD
                     ^^^
error: 12: expected 'texture2D', but found 'writeonlyTexture2D'
    needs_all_access(dest);                     // BAD
                     ^^^^
error: 15: no match for read(writeonlyTexture2D, uint2)
    read(dest, sk_ThreadPosition.xy);           // BAD
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 16: no match for write(readonlyTexture2D, uint2, half4)
    write(src, sk_ThreadPosition.xy, half4(1)); // BAD
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 19: no match for overload(readonlyTexture2D, int)
    overload(src, 1);  // BAD: overload(readonly texture2D t, int) missing
    ^^^^^^^^^^^^^^^^
error: 20: no match for overload(writeonlyTexture2D)
    overload(dest);    // BAD: overload(writeonly texture2D t)      missing
    ^^^^^^^^^^^^^^
error: 32: expected 'texture2D', but found 'readonlyTexture2D'
    needs_all_access(t);                      // BAD
                     ^
error: 35: no match for write(readonlyTexture2D, uint2, half4)
    write(t, sk_ThreadPosition.xy, half4(1)); // BAD
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 39: expected 'texture2D', but found 'writeonlyTexture2D'
    needs_all_access(t);                      // BAD
                     ^
error: 41: no match for read(writeonlyTexture2D, uint2)
    read(t, sk_ThreadPosition.xy);            // BAD
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
10 errors
