#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct S {
    int i;
};
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
half4 fnGreen_h4bf2(Uniforms _uniforms, bool b, float2 ) {
    return _uniforms.colorGreen;
}
half4 fnRed_h4ifS(Uniforms _uniforms, int , float f, S ) {
    return _uniforms.colorRed;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = bool(_uniforms.colorGreen.y) ? fnGreen_h4bf2(_uniforms, true, coords) : fnRed_h4ifS(_uniforms, 123, 3.14, S{0});
    return _out;
}
