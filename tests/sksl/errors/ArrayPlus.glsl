### Compilation failed:

error: 1: '+' cannot operate on 'int[123]'
void array_plus_int    () { int     a[123]; +a; }
                                            ^^
error: 2: '+' cannot operate on 'int4[123]'
void array_plus_int4   () { int4    a[123]; +a; }
                                            ^^
error: 3: '+' cannot operate on 'float[123]'
void array_plus_float  () { float   a[123]; +a; }
                                            ^^
error: 4: '+' cannot operate on 'half3[123]'
void array_plus_half3  () { half3   a[123]; +a; }
                                            ^^
error: 5: '+' cannot operate on 'bool2[123]'
void array_plus_bool2  () { bool2   a[123]; +a; }
                                            ^^
error: 6: '+' cannot operate on 'half4x4[123]'
void array_plus_half4x4() { half4x4 a[123]; +a; }
                                            ^^
6 errors
