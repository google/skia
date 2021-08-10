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

float3x3 float3x3_inverse(float3x3 m) {
    float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2];
    float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2];
    float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2];
    float b01 =  a22*a11 - a12*a21;
    float b11 = -a22*a10 + a12*a20;
    float b21 =  a21*a10 - a11*a20;
    float det = a00*b01 + a01*b11 + a02*b21;
    return float3x3(b01, (-a22*a01 + a02*a21), ( a12*a01 - a02*a11),
                    b11, ( a22*a00 - a02*a20), (-a12*a00 + a02*a10),
                    b21, (-a21*a00 + a01*a20), ( a11*a00 - a01*a10)) * (1/det);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float2x2 inv2x2 = float2x2(float2(-2.0, 1.0), float2(1.5, -0.5));
    float3x3 inv3x3 = float3x3(float3(-24.0, 18.0, 5.0), float3(20.0, -15.0, -4.0), float3(-5.0, 4.0, 1.0));
    float4x4 inv4x4 = float4x4(float4(-2.0, -0.5, 1.0, 0.5), float4(1.0, 0.5, 0.0, -0.5), float4(-8.0, -1.0, 2.0, 2.0), float4(3.0, 0.5, -1.0, -0.5));
    _out.sk_FragColor = ((float2x2(float2(-2.0, 1.0), float2(1.5, -0.5)) == inv2x2 && float3x3(float3(-24.0, 18.0, 5.0), float3(20.0, -15.0, -4.0), float3(-5.0, 4.0, 1.0)) == inv3x3) && float4x4(float4(-2.0, -0.5, 1.0, 0.5), float4(1.0, 0.5, 0.0, -0.5), float4(-8.0, -1.0, 2.0, 2.0), float4(3.0, 0.5, -1.0, -0.5)) == inv4x4) && float3x3_inverse(float3x3(float3(1.0, 2.0, 3.0), float3(4.0, 5.0, 6.0), float3(7.0, 8.0, 9.0))) != inv3x3 ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
