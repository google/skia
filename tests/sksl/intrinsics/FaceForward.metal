#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 N;
    half4 I;
    half4 NRef;
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
    float huge = float(((((1e+30h) * (1e+30h) < 0) ? 1 : -1) * (1.0h)));
    float2 huge2 = faceforward(float2(1.0), float2(1e+30), float2(1e+30));
    float3 huge3 = faceforward(float3(1.0), float3(1e+30), float3(1e+30));
    float4 huge4 = faceforward(float4(1.0), float4(1e+30), float4(1e+30));
    half4 expectedPos = half4(float4(huge) + huge2.xxxx);
    half4 expectedNeg = half4(huge3.xxxx + huge4.xxxx);
    expectedPos = half4(1.0h, 2.0h, 3.0h, 4.0h);
    expectedNeg = half4(-1.0h, -2.0h, -3.0h, -4.0h);
    _out.sk_FragColor = ((((((((((_uniforms.NRef.x) * (_uniforms.I.x) < 0) ? 1 : -1) * (_uniforms.N.x)) == expectedNeg.x && all(faceforward(_uniforms.N.xy, _uniforms.I.xy, _uniforms.NRef.xy) == expectedNeg.xy)) && all(faceforward(_uniforms.N.xyz, _uniforms.I.xyz, _uniforms.NRef.xyz) == expectedPos.xyz)) && all(faceforward(_uniforms.N, _uniforms.I, _uniforms.NRef) == expectedPos)) && -1.0h == expectedNeg.x) && all(half2(-1.0h, -2.0h) == expectedNeg.xy)) && all(half3(1.0h, 2.0h, 3.0h) == expectedPos.xyz)) && all(half4(1.0h, 2.0h, 3.0h, 4.0h) == expectedPos) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
