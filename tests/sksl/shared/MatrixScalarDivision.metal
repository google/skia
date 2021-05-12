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
template <typename T, int C, int R>
thread matrix<T, C, R> operator/(matrix<T, C, R> left, T right) {
    T reciprocal = 1 / right;
    for (size_t index = 0; index < C; ++index) {
        left[index] *= reciprocal;
    }
    return left;
}
template <typename T, int C, int R>
thread matrix<T, C, R> operator/(T left, matrix<T, C, R> right) {
    for (size_t index = 0; index < C; ++index) {
        right[index] = left / right[index];
    }
    return right;
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
    bool ok = true;
    float3x3 matrixScalarDivide = _uniforms.testMatrix3x3 / 2.0;
    float2x2 scalarMatrixDivide = 12.0 / _uniforms.testMatrix2x2;
    ok = ok && matrixScalarDivide == float3x3(float3(0.5, 1.0, 1.5), float3(2.0, 2.5, 3.0), float3(3.5, 4.0, 4.5));
    float4 delta = float4(scalarMatrixDivide[0], scalarMatrixDivide[1]) - float4(12.0, 6.0, 4.0, 3.0);
    ok = ok && all((abs(delta) < float4(0.0099999997764825821)));
    _out.sk_FragColor = ok ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
