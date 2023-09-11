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

thread bool operator==(const float3x3 left, const float3x3 right);
thread bool operator!=(const float3x3 left, const float3x3 right);

thread bool operator==(const float4x4 left, const float4x4 right);
thread bool operator!=(const float4x4 left, const float4x4 right);
thread bool operator==(const float3x3 left, const float3x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const float3x3 left, const float3x3 right) {
    return !(left == right);
}
thread bool operator==(const float4x4 left, const float4x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const float4x4 left, const float4x4 right) {
    return !(left == right);
}
bool test3x3_b(Uniforms _uniforms) {
    float3x3 matrix;
    float3 values = float3(1.0, 2.0, 3.0);
    for (int index = 0;index < 3; ++index) {
        matrix[index] = values;
        values += 3.0;
    }
    return matrix == _uniforms.testMatrix3x3;
}
bool test4x4_b(Uniforms _uniforms) {
    float4x4 matrix;
    float4 values = float4(1.0, 2.0, 3.0, 4.0);
    for (int index = 0;index < 4; ++index) {
        matrix[index] = values;
        values += 4.0;
    }
    return matrix == _uniforms.testMatrix4x4;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = test3x3_b(_uniforms) && test4x4_b(_uniforms) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
