#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 testInputs;
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
    half4 v = _uniforms.testInputs;
    v = half4(v.x, 1.0h, 1.0h, 1.0h);
    v = half4(v.xy, 1.0h, 1.0h);
    v = half4(v.x, 1.0h, 1.0h, 1.0h);
    v = half4(0.0h, v.y, 1.0h, 1.0h);
    v = half4(v.xyz, 1.0h);
    v = half4(v.xy, 1.0h, 1.0h);
    v = half4(v.x, 0.0h, v.z, 1.0h);
    v = half4(v.x, 1.0h, 0.0h, 1.0h);
    v = half4(1.0h, v.yz, 1.0h);
    v = half4(0.0h, v.y, 1.0h, 1.0h);
    v = half4(1.0h, 1.0h, v.z, 1.0h);
    v = half4(v.xyz, 1.0h);
    v = half4(v.xy, 0.0h, v.w);
    v = half4(v.xy, 1.0h, 0.0h);
    v = half4(v.x, 1.0h, v.zw);
    v = half4(v.x, 0.0h, v.z, 1.0h);
    v = half4(v.x, 1.0h, 1.0h, v.w);
    v = half4(v.x, 1.0h, 0.0h, 1.0h);
    v = half4(1.0h, v.yzw);
    v = half4(0.0h, v.yz, 1.0h);
    v = half4(0.0h, v.y, 1.0h, v.w);
    v = half4(1.0h, v.y, 1.0h, 1.0h);
    v = half4(0.0h, 0.0h, v.zw);
    v = half4(0.0h, 0.0h, v.z, 1.0h);
    v = half4(0.0h, 1.0h, 1.0h, v.w);
    _out.sk_FragColor = all(v == half4(0.0h, 1.0h, 1.0h, 1.0h)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
