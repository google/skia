#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
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

thread bool operator==(const float2x2 left, const float2x2 right);
thread bool operator!=(const float2x2 left, const float2x2 right);

thread bool operator==(const float3x3 left, const float3x3 right);
thread bool operator!=(const float3x3 left, const float3x3 right);

thread bool operator==(const float4x4 left, const float4x4 right);
thread bool operator!=(const float4x4 left, const float4x4 right);
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
thread bool operator==(const float4x4 left, const float4x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const float4x4 left, const float4x4 right) {
    return !(left == right);
}

template <typename T>
matrix<T, 3, 3> mat3_inverse(matrix<T, 3, 3> m) {
T
 a00 = m[0].x, a01 = m[0].y, a02 = m[0].z,
 a10 = m[1].x, a11 = m[1].y, a12 = m[1].z,
 a20 = m[2].x, a21 = m[2].y, a22 = m[2].z,
 b01 =  a22*a11 - a12*a21,
 b11 = -a22*a10 + a12*a20,
 b21 =  a21*a10 - a11*a20,
 det = a00*b01 + a01*b11 + a02*b21;
return matrix<T, 3, 3>(
 b01, (-a22*a01 + a02*a21), ( a12*a01 - a02*a11),
 b11, ( a22*a00 - a02*a20), (-a12*a00 + a02*a10),
 b21, (-a21*a00 + a01*a20), ( a11*a00 - a01*a10)) * (1/det);
}

template <typename T>
matrix<T, 2, 2> mat2_inverse(matrix<T, 2, 2> m) {
return matrix<T, 2, 2>(m[1].y, -m[0].y, -m[1].x, m[0].x) * (1/determinant(m));
}

template <typename T>
matrix<T, 4, 4> mat4_inverse(matrix<T, 4, 4> m) {
T
 a00 = m[0].x, a01 = m[0].y, a02 = m[0].z, a03 = m[0].w,
 a10 = m[1].x, a11 = m[1].y, a12 = m[1].z, a13 = m[1].w,
 a20 = m[2].x, a21 = m[2].y, a22 = m[2].z, a23 = m[2].w,
 a30 = m[3].x, a31 = m[3].y, a32 = m[3].z, a33 = m[3].w,
 b00 = a00*a11 - a01*a10,
 b01 = a00*a12 - a02*a10,
 b02 = a00*a13 - a03*a10,
 b03 = a01*a12 - a02*a11,
 b04 = a01*a13 - a03*a11,
 b05 = a02*a13 - a03*a12,
 b06 = a20*a31 - a21*a30,
 b07 = a20*a32 - a22*a30,
 b08 = a20*a33 - a23*a30,
 b09 = a21*a32 - a22*a31,
 b10 = a21*a33 - a23*a31,
 b11 = a22*a33 - a23*a32,
 det = b00*b11 - b01*b10 + b02*b09 + b03*b08 - b04*b07 + b05*b06;
return matrix<T, 4, 4>(
 a11*b11 - a12*b10 + a13*b09,
 a02*b10 - a01*b11 - a03*b09,
 a31*b05 - a32*b04 + a33*b03,
 a22*b04 - a21*b05 - a23*b03,
 a12*b08 - a10*b11 - a13*b07,
 a00*b11 - a02*b08 + a03*b07,
 a32*b02 - a30*b05 - a33*b01,
 a20*b05 - a22*b02 + a23*b01,
 a10*b10 - a11*b08 + a13*b06,
 a01*b08 - a00*b10 - a03*b06,
 a30*b04 - a31*b02 + a33*b00,
 a21*b02 - a20*b04 - a23*b00,
 a11*b07 - a10*b09 - a12*b06,
 a00*b09 - a01*b07 + a02*b06,
 a31*b01 - a30*b03 - a32*b00,
 a20*b03 - a21*b01 + a22*b00) * (1/det);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    const float2x2 matrix2x2 = float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    float2x2 inv2x2 = float2x2(float2(-2.0, 1.0), float2(1.5, -0.5));
    float3x3 inv3x3 = float3x3(float3(-24.0, 18.0, 5.0), float3(20.0, -15.0, -4.0), float3(-5.0, 4.0, 1.0));
    float4x4 inv4x4 = float4x4(float4(-2.0, -0.5, 1.0, 0.5), float4(1.0, 0.5, 0.0, -0.5), float4(-8.0, -1.0, 2.0, 2.0), float4(3.0, 0.5, -1.0, -0.5));
    float Zero = float(_uniforms.colorGreen.z);
    _out.sk_FragColor = (((((float2x2(float2(-2.0, 1.0), float2(1.5, -0.5)) == inv2x2 && float3x3(float3(-24.0, 18.0, 5.0), float3(20.0, -15.0, -4.0), float3(-5.0, 4.0, 1.0)) == inv3x3) && float4x4(float4(-2.0, -0.5, 1.0, 0.5), float4(1.0, 0.5, 0.0, -0.5), float4(-8.0, -1.0, 2.0, 2.0), float4(3.0, 0.5, -1.0, -0.5)) == inv4x4) && mat3_inverse(float3x3(float3(1.0, 2.0, 3.0), float3(4.0, 5.0, 6.0), float3(7.0, 8.0, 9.0))) != inv3x3) && mat2_inverse(matrix2x2 + (float2x2(1.0, 1.0, 1.0, 1.0) * Zero)) == inv2x2) && mat3_inverse(float3x3(float3(1.0, 2.0, 3.0), float3(0.0, 1.0, 4.0), float3(5.0, 6.0, 0.0)) + (float3x3(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0) * Zero)) == inv3x3) && mat4_inverse(float4x4(float4(1.0, 0.0, 0.0, 1.0), float4(0.0, 2.0, 1.0, 2.0), float4(2.0, 1.0, 0.0, 1.0), float4(2.0, 0.0, 1.0, 4.0)) + (float4x4(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0) * Zero)) == inv4x4 ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
