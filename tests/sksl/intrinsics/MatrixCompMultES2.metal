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

thread bool operator==(const float2x2 left, const float2x2 right);
thread bool operator!=(const float2x2 left, const float2x2 right);

thread bool operator==(const float3x3 left, const float3x3 right);
thread bool operator!=(const float3x3 left, const float3x3 right);

template <typename T, int C, int R>
matrix<T, C, R> matrixCompMult(matrix<T, C, R> a, const matrix<T, C, R> b) {
    for (int c = 0; c < C; ++c) {
        a[c] *= b[c];
    }
    return a;
}
thread bool operator==(const float2x2 left, const float2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x2 left, const float2x2 right) {
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
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float2x2 h22 = float2x2(float2(0.0, 5.0), float2(10.0, 15.0));
    float2x2 f22 = matrixCompMult(_uniforms.testMatrix2x2, float2x2(1.0));
    float3x3 h33 = matrixCompMult(_uniforms.testMatrix3x3, float3x3(float3(2.0, 2.0, 2.0), float3(2.0, 2.0, 2.0), float3(2.0, 2.0, 2.0)));
    _out.sk_FragColor = (h22 == float2x2(float2(0.0, 5.0), float2(10.0, 15.0)) && f22 == float2x2(float2(1.0, 0.0), float2(0.0, 4.0))) && h33 == float3x3(float3(2.0, 4.0, 6.0), float3(8.0, 10.0, 12.0), float3(14.0, 16.0, 18.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
