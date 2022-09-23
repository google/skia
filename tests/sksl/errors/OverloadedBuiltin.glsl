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
error: 12: duplicate definition of 'half cos(half2 a)'
half cos(half2 a) { return 0; /* error: overloads a builtin (despite return type mismatch) */ }
^^^^^^^^^^^^^^^^^
error: 13: functions 'float2 cos(half2 a)' and '$pure $genHType cos($genHType angle)' differ only in return type
float2 cos(half2 a) { return 0; /* error: overloads a builtin (despite return type mismatch) */ }
^^^^^^^^^^^^^^^^^^^
error: 15: duplicate definition of 'float pow(float x, float y)'
float pow(float x, float y) { return 0; /* error: overloads a builtin */ }
^^^^^^^^^^^^^^^^^^^^^^^^^^^
6 errors
