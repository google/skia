#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};

thread bool operator==(const half2x4 left, const half2x4 right);
thread bool operator!=(const half2x4 left, const half2x4 right);

thread bool operator==(const half4x2 left, const half4x2 right);
thread bool operator!=(const half4x2 left, const half4x2 right);

thread bool operator==(const float4x3 left, const float4x3 right);
thread bool operator!=(const float4x3 left, const float4x3 right);

template <typename T, int C, int R>
matrix<T, C, R> matrixCompMult(matrix<T, C, R> a, const matrix<T, C, R> b) {
    for (int c = 0; c < C; ++c) {
        a[c] *= b[c];
    }
    return a;
}
half4x2 half4x2_from_half4_half4(half4 x0, half4 x1) {
    return half4x2(half2(x0.xy), half2(x0.zw), half2(x1.xy), half2(x1.zw));
}
thread bool operator==(const half2x4 left, const half2x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const half2x4 left, const half2x4 right) {
    return !(left == right);
}
thread bool operator==(const half4x2 left, const half4x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const half4x2 left, const half4x2 right) {
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
    half2x4 h24 = matrixCompMult(half2x4(half4(9.0h, 9.0h, 9.0h, 9.0h), half4(9.0h, 9.0h, 9.0h, 9.0h)), half2x4(_uniforms.colorRed, _uniforms.colorGreen));
    half4x2 h42 = matrixCompMult(half4x2(half2(1.0h, 2.0h), half2(3.0h, 4.0h), half2(5.0h, 6.0h), half2(7.0h, 8.0h)), half4x2_from_half4_half4(_uniforms.colorRed, _uniforms.colorGreen));
    float4x3 f43 = float4x3(float3(12.0, 22.0, 30.0), float3(36.0, 40.0, 42.0), float3(42.0, 40.0, 36.0), float3(30.0, 22.0, 12.0));
    _out.sk_FragColor = (h24 == half2x4(half4(9.0h, 0.0h, 0.0h, 9.0h), half4(0.0h, 9.0h, 0.0h, 9.0h)) && h42 == half4x2(half2(1.0h, 0.0h), half2(0.0h, 4.0h), half2(0.0h, 6.0h), half2(0.0h, 8.0h))) && f43 == float4x3(float3(12.0, 22.0, 30.0), float3(36.0, 40.0, 42.0), float3(42.0, 40.0, 36.0), float3(30.0, 22.0, 12.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
