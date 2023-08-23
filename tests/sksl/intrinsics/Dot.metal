#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    float4x4 testMatrix4x4;
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
    float4 inputA = _uniforms.testMatrix4x4[0];
    float4 inputB = _uniforms.testMatrix4x4[1];
    float4 expected = float4(5.0, 17.0, 38.0, 70.0);
    _out.sk_FragColor = (((((((inputA.x * inputB.x) == expected.x && dot(inputA.xy, inputB.xy) == expected.y) && dot(inputA.xyz, inputB.xyz) == expected.z) && dot(inputA, inputB) == expected.w) && 5.0 == expected.x) && 17.0 == expected.y) && 38.0 == expected.z) && 70.0 == expected.w ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
