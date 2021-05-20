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
    _0_ok = ((((((((_0_ok && _uniforms.testMatrix3x3[0].x == 1.0) && _uniforms.testMatrix3x3[0].y == 2.0) && _uniforms.testMatrix3x3[0].z == 3.0) && _uniforms.testMatrix3x3[1].x == 4.0) && _uniforms.testMatrix3x3[1].y == 5.0) && _uniforms.testMatrix3x3[1].z == 6.0) && _uniforms.testMatrix3x3[2].x == 7.0) && _uniforms.testMatrix3x3[2].y == 8.0) && _uniforms.testMatrix3x3[2].z == 9.0;
    _out.sk_FragColor = _0_ok ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
