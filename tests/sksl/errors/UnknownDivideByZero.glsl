### Compilation failed:

error: 5: division by zero
    float x = unknownInput / 0;
              ^^^^^^^^^^^^^^^^
error: 6: division by zero
    x = (float2(unknownInput) / 0).x;
         ^^^^^^^^^^^^^^^^^^^^^^^^
error: 7: division by zero
    x = (float2(unknownInput) / float2(zero)).x;
         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 8: division by zero
    x = (float2(unknownInput) / float2(unknownInput, 0)).x;
         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 9: division by zero
    x = (float3(unknownInput) / float3(float(0), unknownInput, unknownInput)).x;
         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 10: division by zero
    x = (float4(unknownInput) / float4(float2(unknownInput, float(zero)), 1, 1)).x;
         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 11: division by zero
    x /= 0;
    ^^^^^^
error: 13: division by zero
    int y = int(unknownInput) / 0;
            ^^^^^^^^^^^^^^^^^^^^^
error: 14: division by zero
    y = int(unknownInput) % 0;
        ^^^^^^^^^^^^^^^^^^^^^
error: 15: division by zero
    y = (int2(unknownInput) / 0).x;
         ^^^^^^^^^^^^^^^^^^^^^^
error: 16: division by zero
    y = (int2(unknownInput) / int2(zero)).x;
         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 17: division by zero
    y = (int2(unknownInput) / int2(unknownInput, 0)).x;
         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 18: division by zero
    y = (int3(unknownInput) / int3(0, unknownInput, unknownInput)).x;
         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 19: division by zero
    y = (int4(unknownInput) % int4(int2(unknownInput, int(zero)), 1, 1)).x;
         ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 20: division by zero
    y /= 0;
    ^^^^^^
error: 21: division by zero
    y %= 0;
    ^^^^^^
16 errors
