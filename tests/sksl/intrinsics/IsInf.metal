#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half2x2 testMatrix2x2;
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};

half4 half4_from_half2x2(half2x2 x) {
    return half4(x[0].xy, x[1].xy);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half4 infiniteValue = half4_from_half2x2(_uniforms.testMatrix2x2) / _uniforms.colorGreen.x;
    half4 finiteValue = half4_from_half2x2(_uniforms.testMatrix2x2) / _uniforms.colorGreen.y;
    _out.sk_FragColor = ((((((isinf(infiniteValue.x) && all(isinf(infiniteValue.xy))) && all(isinf(infiniteValue.xyz))) && all(isinf(infiniteValue))) && !isinf(finiteValue.x)) && !any(isinf(finiteValue.xy))) && !any(isinf(finiteValue.xyz))) && !any(isinf(finiteValue)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
