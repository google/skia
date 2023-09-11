### Compilation failed:

error: 20: value is out of range for type 'short': 99999
    cast_int_to_short(99999);
    ^^^^^^^^^^^^^^^^^^^^^^^^
error: 21: value is out of range for type 'short': 67890
    cast_int2_to_short2(int2(12345, 67890));
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 22: value is out of range for type 'int': 5000000000
    cast_float_to_int(5000000000.0);
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 23: value is out of range for type 'int': 3000000000
    cast_float3_to_int3(float3(3000000000, 2000000, 1000));
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 24: value is out of range for type 'short': 32768
    negate_short(-32768);
    ^^^^^^^^^^^^^^^^^^^^
error: 25: value is out of range for type 'int': 2147483648
    negate_int4(int4(-2147483648));
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
6 errors
