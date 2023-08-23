#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    float3x3 testMatrix3x3;
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
    const float3 expected1 = float3(-3.0, 6.0, -3.0);
    const float3 expected2 = float3(6.0, -12.0, 6.0);
    _out.sk_FragColor = all(cross(_uniforms.testMatrix3x3[0], _uniforms.testMatrix3x3[1]) == expected1) && all(cross(_uniforms.testMatrix3x3[2], _uniforms.testMatrix3x3[0]) == expected2) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
