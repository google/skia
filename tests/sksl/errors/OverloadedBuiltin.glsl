### Compilation failed:

error: 4: duplicate definition of 'half fma(half a, half b, half c)'
half fma(half a, half b, half c) { return 0; /* error: overloads a builtin */ }
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 7: duplicate definition of 'half2 sin(half2 a)'
half2 sin(half2 a) { return half2(0); /* error: overloads a builtin */ }
^^^^^^^^^^^^^^^^^^
error: 8: duplicate definition of 'float3 sin(float3 a)'
float3 sin(float3 a) { return float3(0); /* error: overloads a builtin */ }
^^^^^^^^^^^^^^^^^^^^
error: 9: modifiers on parameter 1 differ between declaration and definition
float4 sin(inout float4 a) { return float4(0); /* error: overloads a builtin */ }
           ^^^^^^^^^^^^^^
error: 13: duplicate definition of 'half cos(half2 a)'
half cos(half2 a) { return 0; /* error: overloads a builtin (despite return type mismatch) */ }
^^^^^^^^^^^^^^^^^
error: 14: functions 'float2 cos(half2 a)' and '$pure $genHType cos($genHType angle)' differ only in return type
float2 cos(half2 a) { return 0; /* error: overloads a builtin (despite return type mismatch) */ }
^^^^^^
error: 15: functions 'int cos(out half3 a)' and '$pure $genHType cos($genHType angle)' differ only in return type
int cos(out half3 a) { return 0; /* error: overloads a builtin (despite return type mismatch) */ }
^^^
error: 17: duplicate definition of 'float pow(float x, float y)'
float pow(float x, float y) { return 0; /* error: overloads a builtin */ }
^^^^^^^^^^^^^^^^^^^^^^^^^^^
8 errors
