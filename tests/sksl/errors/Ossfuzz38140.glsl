### Compilation failed:

error: 1: functions 'half4 blend_src_over(half4 src, half4 dst)' and '$pure half4 blend_src_over(half4 src, half4 dst)' differ only in modifiers
half4 blend_src_over(half4 src, half4 dst) {
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 2: unknown identifier 'src'
    return src + (1 - src.a)*dst;
           ^^^
error: 2: unknown identifier 'src'
    return src + (1 - src.a)*dst;
                      ^^^
error: 2: unknown identifier 'dst'
    return src + (1 - src.a)*dst;
                             ^^^
error: 5: shader 'main' must be main() or main(float2)
half4 main(half4 src, half4 dst) {
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 6: unknown identifier 'src'
    return blend_src_over(src, half4(1) - dst);
                          ^^^
error: 6: unknown identifier 'dst'
    return blend_src_over(src, half4(1) - dst);
                                          ^^^
7 errors
