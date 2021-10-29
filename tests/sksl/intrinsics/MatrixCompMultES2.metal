#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
    float2x2 testMatrix2x2;
    half3x3 testMatrix3x3;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};

thread bool operator==(const half2x2 left, const half2x2 right);
thread bool operator!=(const half2x2 left, const half2x2 right);

thread bool operator==(const float2x2 left, const float2x2 right);
thread bool operator!=(const float2x2 left, const float2x2 right);

thread bool operator==(const half3x3 left, const half3x3 right);
thread bool operator!=(const half3x3 left, const half3x3 right);

template <typename T, int C, int R>
matrix<T, C, R> matrixCompMult(matrix<T, C, R> a, const matrix<T, C, R> b) {
    for (int c = 0; c < C; ++c) {
        a[c] *= b[c];
    }
    return a;
}
thread bool operator==(const half2x2 left, const half2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const half2x2 left, const half2x2 right) {
    return !(left == right);
}
thread bool operator==(const float2x2 left, const float2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x2 left, const float2x2 right) {
    return !(left == right);
}
thread bool operator==(const half3x3 left, const half3x3 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]);
}
thread bool operator!=(const half3x3 left, const half3x3 right) {
    return !(left == right);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half2x2 h22 = half2x2(half2(0.0h, 5.0h), half2(10.0h, 15.0h));
    float2x2 f22 = matrixCompMult(_uniforms.testMatrix2x2, float2x2(1.0));
    half3x3 h33 = matrixCompMult(_uniforms.testMatrix3x3, half3x3(half3(2.0h, 2.0h, 2.0h), half3(2.0h, 2.0h, 2.0h), half3(2.0h, 2.0h, 2.0h)));
    _out.sk_FragColor = (h22 == half2x2(half2(0.0h, 5.0h), half2(10.0h, 15.0h)) && f22 == float2x2(float2(1.0, 0.0), float2(0.0, 4.0))) && h33 == half3x3(half3(2.0h, 4.0h, 6.0h), half3(8.0h, 10.0h, 12.0h), half3(14.0h, 16.0h, 18.0h)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
