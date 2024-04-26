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

thread bool operator==(const float3x3 left, const float3x3 right);
thread bool operator!=(const float3x3 left, const float3x3 right);
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
    float3x3 _skTemp0;
    float3x3 _skTemp1;
    float3x3 _skTemp2;
    float3x3 _skTemp3;
    bool ok = true;
    int i = 5;
    i++;
    ok = ok && i++ == 6;
    ok = ok && i == 7;
    ok = ok && i-- == 7;
    ok = ok && i == 6;
    i--;
    ok = ok && i == 5;
    float f = 0.5;
    f++;
    ok = ok && f++ == 1.5;
    ok = ok && f == 2.5;
    ok = ok && f-- == 2.5;
    ok = ok && f == 1.5;
    f--;
    ok = ok && f == 0.5;
    float2 f2 = float2(0.5);
    f2.x++;
    ok = ok && f2.x++ == 1.5;
    ok = ok && f2.x == 2.5;
    ok = ok && f2.x-- == 2.5;
    ok = ok && f2.x == 1.5;
    f2.x--;
    ok = ok && f2.x == 0.5;
    f2++;
    ok = ok && all(f2++ == float2(1.5));
    ok = ok && all(f2 == float2(2.5));
    ok = ok && all(f2-- == float2(2.5));
    ok = ok && all(f2 == float2(1.5));
    f2--;
    ok = ok && all(f2 == float2(0.5));
    int4 i4 = int4(7, 8, 9, 10);
    i4++;
    ok = ok && all(i4++ == int4(8, 9, 10, 11));
    ok = ok && all(i4 == int4(9, 10, 11, 12));
    ok = ok && all(i4-- == int4(9, 10, 11, 12));
    ok = ok && all(i4 == int4(8, 9, 10, 11));
    i4--;
    ok = ok && all(i4 == int4(7, 8, 9, 10));
    float3x3 m3x3 = float3x3(float3(1.0, 2.0, 3.0), float3(4.0, 5.0, 6.0), float3(7.0, 8.0, 9.0));
    ((_skTemp0 = m3x3), (m3x3 += float3x3(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0)), _skTemp0);
    ok = ok && ((_skTemp1 = m3x3), (m3x3 += float3x3(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0)), _skTemp1) == float3x3(float3(2.0, 3.0, 4.0), float3(5.0, 6.0, 7.0), float3(8.0, 9.0, 10.0));
    ok = ok && m3x3 == float3x3(float3(3.0, 4.0, 5.0), float3(6.0, 7.0, 8.0), float3(9.0, 10.0, 11.0));
    ok = ok && ((_skTemp2 = m3x3), (m3x3 -= float3x3(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0)), _skTemp2) == float3x3(float3(3.0, 4.0, 5.0), float3(6.0, 7.0, 8.0), float3(9.0, 10.0, 11.0));
    ok = ok && m3x3 == float3x3(float3(2.0, 3.0, 4.0), float3(5.0, 6.0, 7.0), float3(8.0, 9.0, 10.0));
    ((_skTemp3 = m3x3), (m3x3 -= float3x3(1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0)), _skTemp3);
    ok = ok && m3x3 == float3x3(float3(1.0, 2.0, 3.0), float3(4.0, 5.0, 6.0), float3(7.0, 8.0, 9.0));
    _out.sk_FragColor = ok ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
