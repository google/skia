#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
    float3x3 testMatrix3x3;
    float4x4 testMatrix4x4;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
bool test3x3_b(Uniforms _uniforms) {
    float3x3 matrix = _uniforms.testMatrix3x3;
    float3 expected = float3(1.0, 2.0, 3.0);
    for (int index = 0;index < 3; ++index) {
        if (any(matrix[index] != expected)) {
            return false;
        }
        expected += 3.0;
    }
    return true;
}
bool test4x4_b(Uniforms _uniforms) {
    float4x4 matrix = _uniforms.testMatrix4x4;
    float4 expected = float4(1.0, 2.0, 3.0, 4.0);
    for (int index = 0;index < 4; ++index) {
        if (any(matrix[index] != expected)) {
            return false;
        }
        expected += 4.0;
    }
    return true;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = test3x3_b(_uniforms) && test4x4_b(_uniforms) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
