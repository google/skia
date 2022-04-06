### Compilation failed:

error: 1: function 'half4 blend_src_over(half4 src, half4 dst)' was already defined
half4 blend_src_over(half4 src, half4 dst) {
                                           ^...
error: 5: shader 'main' must be main() or main(float2)
half4 main(half4 src, half4 dst) {
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
error: 6: unknown identifier 'src'
    return blend_src_over(src, half4(1) - dst);
                          ^^^
error: 6: unknown identifier 'dst'
    return blend_src_over(src, half4(1) - dst);
                                          ^^^
4 errors
