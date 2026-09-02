### Compilation failed:

error: 21: no match for textureRead(writeonlyTexture2D, uint2)
    textureRead(dest, sk_GlobalInvocationID.xy);           // BAD
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 22: no match for textureWrite(readonlyTexture2D, uint2, half4)
    textureWrite(src, sk_GlobalInvocationID.xy, half4(1)); // BAD
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 25: no match for overload(readonlyTexture2D, int)
    overload(src, 1);  // BAD: overload(readonly texture2D t, int) missing
    ^^^^^^^^^^^^^^^^
error: 26: no match for overload(writeonlyTexture2D)
    overload(dest);    // BAD: overload(writeonly texture2D t)      missing
    ^^^^^^^^^^^^^^
error: 30: expected argument of type 'readonlyTexture2D'
    takes_sampled_readonly(storageSrc);            // BAD: storage -> sampled
                           ^^^^^^^^^^
error: 33: expected argument of type 'layout (rgba32f) readonlyTexture2D'
    takes_storage_readonly(src);                   // BAD: sampled -> storage
                           ^^^
error: 34: expected argument of type 'layout (rgba32f) readonlyTexture2D'
    takes_storage_readonly(storageSrcRgba8);       // BAD: storage format mismatch
                           ^^^^^^^^^^^^^^^
error: 49: no match for textureWrite(readonlyTexture2D, uint2, half4)
    textureWrite(t, sk_GlobalInvocationID.xy, half4(1)); // BAD
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 55: no match for textureRead(writeonlyTexture2D, uint2)
    textureRead(t, sk_GlobalInvocationID.xy);            // BAD
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
9 errors
