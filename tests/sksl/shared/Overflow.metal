#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float huge = (((((((((9.0000007644071214e+35 * 1000000000.0) * 1000000000.0) * 1000000000.0) * 1000000000.0) * 1000000000.0) * 1000000000.0) * 1000000000.0) * 1000000000.0) * 1000000000.0) * 1000000000.0;
    int hugeI = int((((((((((((((((((((1073741824 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2);
    uint hugeU = ((((((((((((((((((2147483648u * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    int hugeS = ((((((((((((((((16384 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    uint hugeUS = (((((((((((((((32768u * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u) * 2u;
    int hugeNI = int(((((((((((((((((((-2147483648 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2);
    int hugeNS = (((((((((((((((-32768 * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2) * 2;
    const int4 i4 = int4(2, 2, 2, 2);
    int4 hugeIvec = ((((((((((((((int4(1073741824, 1073741824, 1073741824, 1073741824) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4) * i4;
    const uint4 u4 = uint4(2u, 2u, 2u, 2u);
    uint4 hugeUvec = (((((((((((((uint4(2147483648u, 2147483648u, 2147483648u, 2147483648u) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4) * u4;
    _out.sk_FragColor = ((((((((_uniforms.colorGreen * saturate(huge)) * saturate(float(hugeI))) * saturate(float(hugeU))) * saturate(float(hugeS))) * saturate(float(hugeUS))) * saturate(float(hugeNI))) * saturate(float(hugeNS))) * saturate(float4(hugeIvec))) * saturate(float4(hugeUvec));
    return _out;
}
