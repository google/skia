#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 I;
    half4 N;
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half _skTemp0;
    half _skTemp1;
    half expectedX = -49.0h;
    half2 expectedXY = half2(-169.0h, 202.0h);
    half3 expectedXYZ = half3(-379.0h, 454.0h, -529.0h);
    half4 expectedXYZW = half4(-699.0h, 838.0h, -977.0h, 1116.0h);
    _out.sk_FragColor = (((((((_skTemp0 = _uniforms.I.x, _skTemp1 = _uniforms.N.x, _skTemp0 - 2 * _skTemp1 * _skTemp0 * _skTemp1) == expectedX && all(reflect(_uniforms.I.xy, _uniforms.N.xy) == expectedXY)) && all(reflect(_uniforms.I.xyz, _uniforms.N.xyz) == expectedXYZ)) && all(reflect(_uniforms.I, _uniforms.N) == expectedXYZW)) && -49.0h == expectedX) && all(half2(-169.0h, 202.0h) == expectedXY)) && all(half3(-379.0h, 454.0h, -529.0h) == expectedXYZ)) && all(half4(-699.0h, 838.0h, -977.0h, 1116.0h) == expectedXYZW) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
