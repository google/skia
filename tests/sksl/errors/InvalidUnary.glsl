### Compilation failed:

error: 3: '++' cannot operate on 'float4x4'
void preincrement_matrix()  { float4x4 x = float4x4(1); ++x; }
                                                        ^^^
error: 4: '--' cannot operate on 'float3'
void predecrement_vector()  { float3 x = float3(1); --x; }
                                                    ^^^
error: 5: '++' cannot operate on 'float4x4'
void postincrement_matrix() { float4x4 x = float4x4(1); x++; }
                                                        ^^^
error: 6: '--' cannot operate on 'float3'
void postdecrement_vector() { float3 x = float3(1); x--; }
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
