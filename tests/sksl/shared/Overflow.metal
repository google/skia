#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorGreen;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    const half h = 1e+09h;
    half hugeH = ((((((((((1e+36h * h) * h) * h) * h) * h) * h) * h) * h) * h) * h) * h;
    const float f = 1e+09;
    float hugeF = ((((((((((1e+36 * f) * f) * f) * f) * f) * f) * f) * f) * f) * f) * f;
    int hugeI = int((((((((((((((((((((1073741824 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2);
    uint hugeU = ((((((((((((((((((2147483648u * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    short hugeS = ((((((((((((((((16384 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    ushort hugeUS = (((((((((((((((32768u * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    int hugeNI = int(((((((((((((((((((-2147483648 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2);
    short hugeNS = (((((((((((((((-32768 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    const int4 i4 = int4(2);
    int4 hugeIvec = ((((((((((((((int4(1073741824) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4;
    const uint4 u4 = uint4(2u);
    uint4 hugeUvec = (((((((((((((uint4(2147483648u) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4;
    float4x4 hugeMxM = float4x4(float4(1e+20, 1e+20, 1e+20, 1e+20), float4(1e+20, 1e+20, 1e+20, 1e+20), float4(1e+20, 1e+20, 1e+20, 1e+20), float4(1e+20, 1e+20, 1e+20, 1e+20)) * float4x4(float4(1e+20, 1e+20, 1e+20, 1e+20), float4(1e+20, 1e+20, 1e+20, 1e+20), float4(1e+20, 1e+20, 1e+20, 1e+20), float4(1e+20, 1e+20, 1e+20, 1e+20));
    float4 hugeMxV = float4x4(float4(1e+20, 1e+20, 1e+20, 1e+20), float4(1e+20, 1e+20, 1e+20, 1e+20), float4(1e+20, 1e+20, 1e+20, 1e+20), float4(1e+20, 1e+20, 1e+20, 1e+20)) * float4(1e+20);
    float4 hugeVxM = float4(1e+20) * float4x4(float4(1e+20, 1e+20, 1e+20, 1e+20), float4(1e+20, 1e+20, 1e+20, 1e+20), float4(1e+20, 1e+20, 1e+20, 1e+20), float4(1e+20, 1e+20, 1e+20, 1e+20));
    _out.sk_FragColor = ((((((((((((_uniforms.colorGreen * saturate(hugeH)) * saturate(half(hugeF))) * saturate(half(hugeI))) * saturate(half(hugeU))) * saturate(half(hugeS))) * saturate(half(hugeUS))) * saturate(half(hugeNI))) * saturate(half(hugeNS))) * saturate(half4(hugeIvec))) * saturate(half4(hugeUvec))) * saturate(half4(hugeMxM[0]))) * saturate(half4(hugeMxV))) * saturate(half4(hugeVxM));
    return _out;
}
