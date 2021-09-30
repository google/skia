#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 inputVal;
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
    float4 expectedVec = float4(1.0, 0.0, 0.0, 0.0);
    _out.sk_FragColor = ((((((sign(_uniforms.inputVal.x) == expectedVec.x && all(normalize(_uniforms.inputVal.xy) == expectedVec.xy)) && all(normalize(_uniforms.inputVal.xyz) == expectedVec.xyz)) && all(normalize(_uniforms.inputVal) == expectedVec)) && 1.0 == expectedVec.x) && all(float2(0.0, 1.0) == expectedVec.yx)) && all(float3(0.0, 1.0, 0.0) == expectedVec.zxy)) && all(float4(1.0, 0.0, 0.0, 0.0) == expectedVec) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
