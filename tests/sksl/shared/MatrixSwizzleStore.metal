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

thread bool operator==(const float4x4 left, const float4x4 right);
thread bool operator!=(const float4x4 left, const float4x4 right);

thread bool operator==(const float3x3 left, const float3x3 right);
thread bool operator!=(const float3x3 left, const float3x3 right);
thread bool operator==(const float4x4 left, const float4x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const float4x4 left, const float4x4 right) {
    return !(left == right);
}
thread bool operator==(const float3x3 left, const float3x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const float3x3 left, const float3x3 right) {
    return !(left == right);
}
bool test4x4_b(Uniforms _uniforms) {
    float4x4 matrix;
    float4 values = float4(4.0, 3.0, 2.0, 1.0);
    for (int index = 0;index < 4; ++index) {
        matrix[index].wx = values.xw;
        matrix[index].zy = values.yz;
        values += 4.0;
    }
    return matrix == _uniforms.testMatrix4x4;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float3x3 _0_matrix;
    float3 _1_values = float3(3.0, 2.0, 1.0);
    for (int _2_index = 0;_2_index < 3; ++_2_index) {
        _0_matrix[_2_index].zx = _1_values.xz;
        _0_matrix[_2_index].y = _1_values.y;
        _1_values += 3.0;
    }
    _out.sk_FragColor = _0_matrix == _uniforms.testMatrix3x3 && test4x4_b(_uniforms) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
