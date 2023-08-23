#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorWhite;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half4 color = half4(0.0h);
    if (_uniforms.colorWhite.x == 1.0h) color.y = 1.0h;
    if (_uniforms.colorWhite.x == 2.0h) ; else color.w = 1.0h;
    while (_uniforms.colorWhite.x == 2.0h) ;
    do ; while (_uniforms.colorWhite.x == 2.0h);
    _out.sk_FragColor = color;
    return _out;
}
