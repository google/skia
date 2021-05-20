#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
    float2x2 testMatrix2x2;
    float3x3 testMatrix3x3;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool _0_ok = true;
    float2x2 _1_tempMatL;
    float2x2 _2_tempMatR;
    _0_ok = _0_ok && ((_1_tempMatL = _uniforms.testMatrix2x2 , _2_tempMatR = float2x2(float2(1.0, 2.0), float2(3.0, 4.0))) , all(_1_tempMatL[0] == _2_tempMatR[0]) && all(_1_tempMatL[1] == _2_tempMatR[1]));
    _out.sk_FragColor = _0_ok ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
