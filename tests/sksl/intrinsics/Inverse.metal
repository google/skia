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

thread bool operator==(const half2x2 left, const half2x2 right);
thread bool operator!=(const half2x2 left, const half2x2 right);

thread bool operator==(const half3x3 left, const half3x3 right);
thread bool operator!=(const half3x3 left, const half3x3 right);

thread bool operator==(const half4x4 left, const half4x4 right);
thread bool operator!=(const half4x4 left, const half4x4 right);
thread bool operator==(const half2x2 left, const half2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const half2x2 left, const half2x2 right) {
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
thread bool operator==(const half4x4 left, const half4x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const half4x4 left, const half4x4 right) {
    return !(left == right);
}

template <typename T>
matrix<T, 3, 3> mat3_inverse(matrix<T, 3, 3> m) {
    T a00 = m[0][0], a01 = m[0][1], a02 = m[0][2];
    T a10 = m[1][0], a11 = m[1][1], a12 = m[1][2];
    T a20 = m[2][0], a21 = m[2][1], a22 = m[2][2];
    T b01 =  a22*a11 - a12*a21;
    T b11 = -a22*a10 + a12*a20;
    T b21 =  a21*a10 - a11*a20;
    T det = a00*b01 + a01*b11 + a02*b21;
    return matrix<T, 3, 3>(b01, (-a22*a01 + a02*a21), ( a12*a01 - a02*a11),
                           b11, ( a22*a00 - a02*a20), (-a12*a00 + a02*a10),
                           b21, (-a21*a00 + a01*a20), ( a11*a00 - a01*a10)) * (1/det);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half2x2 inv2x2 = half2x2(half2(-2.0h, 1.0h), half2(1.5h, -0.5h));
    half3x3 inv3x3 = half3x3(half3(-24.0h, 18.0h, 5.0h), half3(20.0h, -15.0h, -4.0h), half3(-5.0h, 4.0h, 1.0h));
    half4x4 inv4x4 = half4x4(half4(-2.0h, -0.5h, 1.0h, 0.5h), half4(1.0h, 0.5h, 0.0h, -0.5h), half4(-8.0h, -1.0h, 2.0h, 2.0h), half4(3.0h, 0.5h, -1.0h, -0.5h));
    _out.sk_FragColor = ((half2x2(half2(-2.0h, 1.0h), half2(1.5h, -0.5h)) == inv2x2 && half3x3(half3(-24.0h, 18.0h, 5.0h), half3(20.0h, -15.0h, -4.0h), half3(-5.0h, 4.0h, 1.0h)) == inv3x3) && half4x4(half4(-2.0h, -0.5h, 1.0h, 0.5h), half4(1.0h, 0.5h, 0.0h, -0.5h), half4(-8.0h, -1.0h, 2.0h, 2.0h), half4(3.0h, 0.5h, -1.0h, -0.5h)) == inv4x4) && mat3_inverse(half3x3(half3(1.0h, 2.0h, 3.0h), half3(4.0h, 5.0h, 6.0h), half3(7.0h, 8.0h, 9.0h))) != inv3x3 ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
