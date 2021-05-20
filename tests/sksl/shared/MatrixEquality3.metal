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
thread bool operator==(const float2x2 left, const float2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x2 left, const float2x2 right) {
    return !(left == right);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool _0_ok = true;
    _0_ok = _0_ok && _uniforms.testMatrix2x2 == float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    _out.sk_FragColor = _0_ok ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
