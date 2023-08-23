#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
    half2x2 testMatrix2x2;
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
    bool ok = true;
    ok = ok && all(half4_from_half2x2(_uniforms.testMatrix2x2) == half4(1.0h, 2.0h, 3.0h, 4.0h));
    ok = ok && all(float4(half4_from_half2x2(_uniforms.testMatrix2x2)) == float4(1.0, 2.0, 3.0, 4.0));
    ok = ok && all(int4(half4_from_half2x2(_uniforms.testMatrix2x2)) == int4(1, 2, 3, 4));
    ok = ok && all(bool4(half4_from_half2x2(_uniforms.testMatrix2x2)) == bool4(true));
    _out.sk_FragColor = ok ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
