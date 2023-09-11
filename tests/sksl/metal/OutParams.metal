#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
void out_half_vh(thread half& v) {
    v = 1.0h;
}
void out_half2_vh2(thread half2& v) {
    v = half2(2.0h);
}
void out_half3_vh3(thread half3& v) {
    v = half3(3.0h);
}
void out_half4_vh4(thread half4& v) {
    v = half4(4.0h);
}
void out_half2x2_vh22(thread half2x2& v) {
    v = half2x2(2.0h);
}
void out_half3x3_vh33(thread half3x3& v) {
    v = half3x3(3.0h);
}
void out_half4x4_vh44(thread half4x4& v) {
    v = half4x4(4.0h);
}
void out_int_vi(thread int& v) {
    v = 1;
}
void out_int2_vi2(thread int2& v) {
    v = int2(2);
}
void out_int3_vi3(thread int3& v) {
    v = int3(3);
}
void out_int4_vi4(thread int4& v) {
    v = int4(4);
}
void out_float_vf(thread float& v) {
    v = 1.0;
}
void out_float2_vf2(thread float2& v) {
    v = float2(2.0);
}
void out_float3_vf3(thread float3& v) {
    v = float3(3.0);
}
void out_float4_vf4(thread float4& v) {
    v = float4(4.0);
}
void out_float2x2_vf22(thread float2x2& v) {
    v = float2x2(2.0);
}
void out_float3x3_vf33(thread float3x3& v) {
    v = float3x3(3.0);
}
void out_float4x4_vf44(thread float4x4& v) {
    v = float4x4(4.0);
}
void out_bool_vb(thread bool& v) {
    v = true;
}
void out_bool2_vb2(thread bool2& v) {
    v = bool2(false);
}
void out_bool3_vb3(thread bool3& v) {
    v = bool3(true);
}
void out_bool4_vb4(thread bool4& v) {
    v = bool4(false);
}
void out_pair_vhh(thread half& v1, thread half& v2) {
    v1 = 1.0h;
    v2 = 2.0h;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half _skTemp0;
    half2 _skTemp1;
    half3 _skTemp2;
    half4 _skTemp3;
    half _skTemp4;
    half2 _skTemp5;
    half4 _skTemp6;
    half _skTemp7;
    half _skTemp8;
    half _skTemp9;
    half _skTemp10;
    half _skTemp11;
    half _skTemp12;
    half _skTemp13;
    half _skTemp14;
    half _skTemp15;
    half _skTemp16;
    half2x2 _skTemp17;
    half3x3 _skTemp18;
    half4x4 _skTemp19;
    half3 _skTemp20;
    half4 _skTemp21;
    half2 _skTemp22;
    int _skTemp23;
    int2 _skTemp24;
    int3 _skTemp25;
    int4 _skTemp26;
    int3 _skTemp27;
    float _skTemp28;
    float2 _skTemp29;
    float3 _skTemp30;
    float4 _skTemp31;
    float2 _skTemp32;
    float _skTemp33;
    float2x2 _skTemp34;
    float3x3 _skTemp35;
    float4x4 _skTemp36;
    float _skTemp37;
    float4 _skTemp38;
    bool _skTemp39;
    bool2 _skTemp40;
    bool3 _skTemp41;
    bool4 _skTemp42;
    bool2 _skTemp43;
    bool _skTemp44;
    half h;
    ((out_half_vh(_skTemp0)), (h = _skTemp0));
    half2 h2;
    ((out_half2_vh2(_skTemp1)), (h2 = _skTemp1));
    half3 h3;
    ((out_half3_vh3(_skTemp2)), (h3 = _skTemp2));
    half4 h4;
    ((out_half4_vh4(_skTemp3)), (h4 = _skTemp3));
    ((out_half_vh(_skTemp4)), (h3.y = _skTemp4));
    ((out_half2_vh2(_skTemp5)), (h3.xz = _skTemp5));
    ((out_half4_vh4(_skTemp6)), (h4.zwxy = _skTemp6));
    _out.sk_FragColor = half4(h, h2.x, h3.x, h4.x);
    half h1;
    ((out_pair_vhh(_skTemp7, _skTemp8)), (h = _skTemp7), (h1 = _skTemp8));
    ((out_pair_vhh(_skTemp9, _skTemp10)), (h = _skTemp9), (h = _skTemp10));
    ((out_pair_vhh(_skTemp11, _skTemp12)), (h2.x = _skTemp11), (h2.y = _skTemp12));
    ((out_pair_vhh(_skTemp13, _skTemp14)), (h2.x = _skTemp13), (h2.x = _skTemp14));
    ((out_pair_vhh(_skTemp15, _skTemp16)), (h2.x = _skTemp15), (h3.x = _skTemp16));
    half2x2 h2x2;
    ((out_half2x2_vh22(_skTemp17)), (h2x2 = _skTemp17));
    half3x3 h3x3;
    ((out_half3x3_vh33(_skTemp18)), (h3x3 = _skTemp18));
    half4x4 h4x4;
    ((out_half4x4_vh44(_skTemp19)), (h4x4 = _skTemp19));
    ((out_half3_vh3(_skTemp20)), (h3x3[1] = _skTemp20));
    ((out_half4_vh4(_skTemp21)), (h4x4[3].zwxy = _skTemp21));
    ((out_half2_vh2(_skTemp22)), (h2x2[0] = _skTemp22));
    _out.sk_FragColor = half4(h2x2[0].x, h3x3[0].x, h4x4[0].x, 1.0h);
    int i;
    ((out_int_vi(_skTemp23)), (i = _skTemp23));
    int2 i2;
    ((out_int2_vi2(_skTemp24)), (i2 = _skTemp24));
    int3 i3;
    ((out_int3_vi3(_skTemp25)), (i3 = _skTemp25));
    int4 i4;
    ((out_int4_vi4(_skTemp26)), (i4 = _skTemp26));
    ((out_int3_vi3(_skTemp27)), (i4.xyz = _skTemp27));
    _out.sk_FragColor = half4(half(i), half(i2.x), half(i3.x), half(i4.x));
    float f;
    ((out_float_vf(_skTemp28)), (f = _skTemp28));
    float2 f2;
    ((out_float2_vf2(_skTemp29)), (f2 = _skTemp29));
    float3 f3;
    ((out_float3_vf3(_skTemp30)), (f3 = _skTemp30));
    float4 f4;
    ((out_float4_vf4(_skTemp31)), (f4 = _skTemp31));
    ((out_float2_vf2(_skTemp32)), (f3.xy = _skTemp32));
    ((out_float_vf(_skTemp33)), (f2.x = _skTemp33));
    _out.sk_FragColor = half4(half(f), half(f2.x), half(f3.x), half(f4.x));
    float2x2 f2x2;
    ((out_float2x2_vf22(_skTemp34)), (f2x2 = _skTemp34));
    float3x3 f3x3;
    ((out_float3x3_vf33(_skTemp35)), (f3x3 = _skTemp35));
    float4x4 f4x4;
    ((out_float4x4_vf44(_skTemp36)), (f4x4 = _skTemp36));
    ((out_float_vf(_skTemp37)), (f2x2[0].x = _skTemp37));
    ((out_float4_vf4(_skTemp38)), (f4x4[1] = _skTemp38));
    _out.sk_FragColor = half4(half(f2x2[0].x), half(f3x3[0].x), half(f4x4[0].x), 1.0h);
    bool b;
    ((out_bool_vb(_skTemp39)), (b = _skTemp39));
    bool2 b2;
    ((out_bool2_vb2(_skTemp40)), (b2 = _skTemp40));
    bool3 b3;
    ((out_bool3_vb3(_skTemp41)), (b3 = _skTemp41));
    bool4 b4;
    ((out_bool4_vb4(_skTemp42)), (b4 = _skTemp42));
    ((out_bool2_vb2(_skTemp43)), (b4.xw = _skTemp43));
    ((out_bool_vb(_skTemp44)), (b3.z = _skTemp44));
    _out.sk_FragColor = half4(half(b), half(b2.x), half(b3.x), half(b4.x));
    return _out;
}
