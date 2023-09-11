### Compilation failed:

error: 16: no match for textureRead(writeonlyTexture2D, uint2)
    textureRead(dest, sk_GlobalInvocationID.xy);           // BAD
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 17: no match for textureWrite(readonlyTexture2D, uint2, half4)
    textureWrite(src, sk_GlobalInvocationID.xy, half4(1)); // BAD
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 20: no match for overload(readonlyTexture2D, int)
    overload(src, 1);  // BAD: overload(readonly texture2D t, int) missing
    ^^^^^^^^^^^^^^^^
error: 21: no match for overload(writeonlyTexture2D)
    overload(dest);    // BAD: overload(writeonly texture2D t)      missing
    ^^^^^^^^^^^^^^
error: 37: no match for textureWrite(readonlyTexture2D, uint2, half4)
    textureWrite(t, sk_GlobalInvocationID.xy, half4(1)); // BAD
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 43: no match for textureRead(writeonlyTexture2D, uint2)
    textureRead(t, sk_GlobalInvocationID.xy);            // BAD
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
6 errors
