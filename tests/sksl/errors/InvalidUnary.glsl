### Compilation failed:

error: 3: '++' cannot operate on 'float[1]'
void preincrement_array()   { float    x[1]; ++x; }
                                             ^^^
error: 4: '--' cannot operate on 'int3[2]'
void predecrement_array()   { int3     x[2]; --x; }
                                             ^^^
error: 5: '++' cannot operate on 'float4x4[3]'
void postincrement_array()  { float4x4 x[3]; x++; }
                                             ^^^
error: 6: '--' cannot operate on 'bool'
void postdecrement_bool()   { bool     x = true; x--; }
                                                 ^^^
error: 7: '!' cannot operate on 'int'
void not_integer()          { int x = !12; }
                                      ^^^
error: 8: '+' cannot operate on 'Foo'
void positive_struct()      { Foo x = +bar; }
                                      ^^^^
error: 9: '-' cannot operate on 'Foo'
void negative_struct()      { Foo x = -bar; }
                                      ^^^^
7 errors
