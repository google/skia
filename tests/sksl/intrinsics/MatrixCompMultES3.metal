#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

thread bool operator==(const float2x4 left, const float2x4 right);
thread bool operator!=(const float2x4 left, const float2x4 right);

thread bool operator==(const float4x2 left, const float4x2 right);
thread bool operator!=(const float4x2 left, const float4x2 right);

thread bool operator==(const float4x3 left, const float4x3 right);
thread bool operator!=(const float4x3 left, const float4x3 right);

template <int C, int R>
matrix<float, C, R> matrixCompMult(matrix<float, C, R> a, const matrix<float, C, R> b) {
    for (int c = 0; c < C; ++c) {
        a[c] *= b[c];
    }
    return a;
}
float4x2 float4x2_from_float4_float4(float4 x0, float4 x1) {
    return float4x2(float2(x0.xy), float2(x0.zw), float2(x1.xy), float2(x1.zw));
}
thread bool operator==(const float2x4 left, const float2x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x4 left, const float2x4 right) {
    return !(left == right);
}
thread bool operator==(const float4x2 left, const float4x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const float4x2 left, const float4x2 right) {
    return !(left == right);
}
thread bool operator==(const float4x3 left, const float4x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const float4x3 left, const float4x3 right) {
    return !(left == right);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float2x4 h24 = matrixCompMult(float2x4(float4(9.0, 9.0, 9.0, 9.0), float4(9.0, 9.0, 9.0, 9.0)), float2x4(_uniforms.colorRed, _uniforms.colorGreen));
    float4x2 h42 = matrixCompMult(float4x2(float2(1.0, 2.0), float2(3.0, 4.0), float2(5.0, 6.0), float2(7.0, 8.0)), float4x2_from_float4_float4(_uniforms.colorRed, _uniforms.colorGreen));
    float4x3 f43 = float4x3(float3(12.0, 22.0, 30.0), float3(36.0, 40.0, 42.0), float3(42.0, 40.0, 36.0), float3(30.0, 22.0, 12.0));
    _out.sk_FragColor = (h24 == float2x4(float4(9.0, 0.0, 0.0, 9.0), float4(0.0, 9.0, 0.0, 9.0)) && h42 == float4x2(float2(1.0, 0.0), float2(0.0, 4.0), float2(0.0, 6.0), float2(0.0, 8.0))) && f43 == float4x3(float3(12.0, 22.0, 30.0), float3(36.0, 40.0, 42.0), float3(42.0, 40.0, 36.0), float3(30.0, 22.0, 12.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
