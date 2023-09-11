#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
    half4 colorWhite;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
void out_half_vh(Uniforms _uniforms, thread half& v) {
    v = _uniforms.colorWhite.x;
}
void out_half2_vh2(Uniforms _uniforms, thread half2& v) {
    v = half2(_uniforms.colorWhite.y);
}
void out_half3_vh3(Uniforms _uniforms, thread half3& v) {
    v = half3(_uniforms.colorWhite.z);
}
void out_half4_vh4(Uniforms _uniforms, thread half4& v) {
    v = half4(_uniforms.colorWhite.w);
}
void out_half2x2_vh22(Uniforms _uniforms, thread half2x2& v) {
    v = half2x2(_uniforms.colorWhite.x);
}
void out_half3x3_vh33(Uniforms _uniforms, thread half3x3& v) {
    v = half3x3(_uniforms.colorWhite.y);
}
void out_half4x4_vh44(Uniforms _uniforms, thread half4x4& v) {
    v = half4x4(_uniforms.colorWhite.z);
}
void out_int_vi(Uniforms _uniforms, thread int& v) {
    v = int(_uniforms.colorWhite.x);
}
void out_int2_vi2(Uniforms _uniforms, thread int2& v) {
    v = int2(int(_uniforms.colorWhite.y));
}
void out_int3_vi3(Uniforms _uniforms, thread int3& v) {
    v = int3(int(_uniforms.colorWhite.z));
}
void out_int4_vi4(Uniforms _uniforms, thread int4& v) {
    v = int4(int(_uniforms.colorWhite.w));
}
void out_float_vf(Uniforms _uniforms, thread float& v) {
    v = float(_uniforms.colorWhite.x);
}
void out_float2_vf2(Uniforms _uniforms, thread float2& v) {
    v = float2(float(_uniforms.colorWhite.y));
}
void out_float3_vf3(Uniforms _uniforms, thread float3& v) {
    v = float3(float(_uniforms.colorWhite.z));
}
void out_float4_vf4(Uniforms _uniforms, thread float4& v) {
    v = float4(float(_uniforms.colorWhite.w));
}
void out_float2x2_vf22(Uniforms _uniforms, thread float2x2& v) {
    v = float2x2(float(_uniforms.colorWhite.x));
}
void out_float3x3_vf33(Uniforms _uniforms, thread float3x3& v) {
    v = float3x3(float(_uniforms.colorWhite.y));
}
void out_float4x4_vf44(Uniforms _uniforms, thread float4x4& v) {
    v = float4x4(float(_uniforms.colorWhite.z));
}
void out_bool_vb(Uniforms _uniforms, thread bool& v) {
    v = bool(_uniforms.colorWhite.x);
}
void out_bool2_vb2(Uniforms _uniforms, thread bool2& v) {
    v = bool2(bool(_uniforms.colorWhite.y));
}
void out_bool3_vb3(Uniforms _uniforms, thread bool3& v) {
    v = bool3(bool(_uniforms.colorWhite.z));
}
void out_bool4_vb4(Uniforms _uniforms, thread bool4& v) {
    v = bool4(bool(_uniforms.colorWhite.w));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half _skTemp0;
    half2 _skTemp1;
    half3 _skTemp2;
    half4 _skTemp3;
    half _skTemp4;
    half2 _skTemp5;
    half4 _skTemp6;
    half2x2 _skTemp7;
    half3x3 _skTemp8;
    half4x4 _skTemp9;
    half3 _skTemp10;
    half _skTemp11;
    half _skTemp12;
    int _skTemp13;
    int2 _skTemp14;
    int3 _skTemp15;
    int4 _skTemp16;
    int3 _skTemp17;
    int _skTemp18;
    float _skTemp19;
    float2 _skTemp20;
    float3 _skTemp21;
    float4 _skTemp22;
    float2 _skTemp23;
    float _skTemp24;
    float2x2 _skTemp25;
    float3x3 _skTemp26;
    float4x4 _skTemp27;
    float _skTemp28;
    bool _skTemp29;
    bool2 _skTemp30;
    bool3 _skTemp31;
    bool4 _skTemp32;
    bool2 _skTemp33;
    bool _skTemp34;
    half h;
    ((out_half_vh(_uniforms, _skTemp0)), (h = _skTemp0));
    half2 h2;
    ((out_half2_vh2(_uniforms, _skTemp1)), (h2 = _skTemp1));
    half3 h3;
    ((out_half3_vh3(_uniforms, _skTemp2)), (h3 = _skTemp2));
    half4 h4;
    ((out_half4_vh4(_uniforms, _skTemp3)), (h4 = _skTemp3));
    ((out_half_vh(_uniforms, _skTemp4)), (h3.y = _skTemp4));
    ((out_half2_vh2(_uniforms, _skTemp5)), (h3.xz = _skTemp5));
    ((out_half4_vh4(_uniforms, _skTemp6)), (h4.zwxy = _skTemp6));
    half2x2 h2x2;
    ((out_half2x2_vh22(_uniforms, _skTemp7)), (h2x2 = _skTemp7));
    half3x3 h3x3;
    ((out_half3x3_vh33(_uniforms, _skTemp8)), (h3x3 = _skTemp8));
    half4x4 h4x4;
    ((out_half4x4_vh44(_uniforms, _skTemp9)), (h4x4 = _skTemp9));
    ((out_half3_vh3(_uniforms, _skTemp10)), (h3x3[1] = _skTemp10));
    ((out_half_vh(_uniforms, _skTemp11)), (h4x4[3].w = _skTemp11));
    ((out_half_vh(_uniforms, _skTemp12)), (h2x2[0].x = _skTemp12));
    int i;
    ((out_int_vi(_uniforms, _skTemp13)), (i = _skTemp13));
    int2 i2;
    ((out_int2_vi2(_uniforms, _skTemp14)), (i2 = _skTemp14));
    int3 i3;
    ((out_int3_vi3(_uniforms, _skTemp15)), (i3 = _skTemp15));
    int4 i4;
    ((out_int4_vi4(_uniforms, _skTemp16)), (i4 = _skTemp16));
    ((out_int3_vi3(_uniforms, _skTemp17)), (i4.xyz = _skTemp17));
    ((out_int_vi(_uniforms, _skTemp18)), (i2.y = _skTemp18));
    float f;
    ((out_float_vf(_uniforms, _skTemp19)), (f = _skTemp19));
    float2 f2;
    ((out_float2_vf2(_uniforms, _skTemp20)), (f2 = _skTemp20));
    float3 f3;
    ((out_float3_vf3(_uniforms, _skTemp21)), (f3 = _skTemp21));
    float4 f4;
    ((out_float4_vf4(_uniforms, _skTemp22)), (f4 = _skTemp22));
    ((out_float2_vf2(_uniforms, _skTemp23)), (f3.xy = _skTemp23));
    ((out_float_vf(_uniforms, _skTemp24)), (f2.x = _skTemp24));
    float2x2 f2x2;
    ((out_float2x2_vf22(_uniforms, _skTemp25)), (f2x2 = _skTemp25));
    float3x3 f3x3;
    ((out_float3x3_vf33(_uniforms, _skTemp26)), (f3x3 = _skTemp26));
    float4x4 f4x4;
    ((out_float4x4_vf44(_uniforms, _skTemp27)), (f4x4 = _skTemp27));
    ((out_float_vf(_uniforms, _skTemp28)), (f2x2[0].x = _skTemp28));
    bool b;
    ((out_bool_vb(_uniforms, _skTemp29)), (b = _skTemp29));
    bool2 b2;
    ((out_bool2_vb2(_uniforms, _skTemp30)), (b2 = _skTemp30));
    bool3 b3;
    ((out_bool3_vb3(_uniforms, _skTemp31)), (b3 = _skTemp31));
    bool4 b4;
    ((out_bool4_vb4(_uniforms, _skTemp32)), (b4 = _skTemp32));
    ((out_bool2_vb2(_uniforms, _skTemp33)), (b4.xw = _skTemp33));
    ((out_bool_vb(_uniforms, _skTemp34)), (b3.z = _skTemp34));
    bool ok = true;
    ok = ok && 1.0h == (((((h * h2.x) * h3.x) * h4.x) * h2x2[0].x) * h3x3[0].x) * h4x4[0].x;
    ok = ok && 1.0 == (((((f * f2.x) * f3.x) * f4.x) * f2x2[0].x) * f3x3[0].x) * f4x4[0].x;
    ok = ok && 1 == ((i * i2.x) * i3.x) * i4.x;
    ok = ok && (((b && b2.x) && b3.x) && b4.x);
    _out.sk_FragColor = ok ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
