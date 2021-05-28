#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 N;
    float4 I;
    float4 NRef;
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 expectedPos = float4(1.0, 2.0, 3.0, 4.0);
    float4 expectedNeg = float4(-1.0, -2.0, -3.0, -4.0);
    _out.sk_FragColor = ((((((((((_uniforms.NRef.x) * (_uniforms.I.x) < 0) ? 1 : -1) * (_uniforms.N.x)) == expectedNeg.x && all(faceforward(_uniforms.N.xy, _uniforms.I.xy, _uniforms.NRef.xy) == expectedNeg.xy)) && all(faceforward(_uniforms.N.xyz, _uniforms.I.xyz, _uniforms.NRef.xyz) == expectedPos.xyz)) && all(faceforward(_uniforms.N, _uniforms.I, _uniforms.NRef) == expectedPos)) && -1.0 == expectedNeg.x) && all(float2(-1.0, -2.0) == expectedNeg.xy)) && all(float3(1.0, 2.0, 3.0) == expectedPos.xyz)) && all(float4(1.0, 2.0, 3.0, 4.0) == expectedPos) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
