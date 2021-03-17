#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
    float4 colorWhite;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
void out_half(Uniforms _uniforms, thread float& v);
void _skOutParamHelper0_out_half(Uniforms _uniforms, thread float& h) {
    float _var0;
    out_half(_uniforms, _var0);
    h = _var0;
}
void out_half2(Uniforms _uniforms, thread float2& v);
void _skOutParamHelper1_out_half2(Uniforms _uniforms, thread float2& h2) {
    float2 _var0;
    out_half2(_uniforms, _var0);
    h2 = _var0;
}
void out_half3(Uniforms _uniforms, thread float3& v);
void _skOutParamHelper2_out_half3(Uniforms _uniforms, thread float3& h3) {
    float3 _var0;
    out_half3(_uniforms, _var0);
    h3 = _var0;
}
void out_half4(Uniforms _uniforms, thread float4& v);
void _skOutParamHelper3_out_half4(Uniforms _uniforms, thread float4& h4) {
    float4 _var0;
    out_half4(_uniforms, _var0);
    h4 = _var0;
}
void out_half(Uniforms _uniforms, thread float& v);
void _skOutParamHelper4_out_half(Uniforms _uniforms, thread float3& h3) {
    float _var0;
    out_half(_uniforms, _var0);
    h3.y = _var0;
}
void out_half2(Uniforms _uniforms, thread float2& v);
void _skOutParamHelper5_out_half2(Uniforms _uniforms, thread float3& h3) {
    float2 _var0;
    out_half2(_uniforms, _var0);
    h3.xz = _var0;
}
void out_half4(Uniforms _uniforms, thread float4& v);
void _skOutParamHelper6_out_half4(Uniforms _uniforms, thread float4& h4) {
    float4 _var0;
    out_half4(_uniforms, _var0);
    h4.zwxy = _var0;
}
void out_half2x2(Uniforms _uniforms, thread float2x2& v);
void _skOutParamHelper7_out_half2x2(Uniforms _uniforms, thread float2x2& h2x2) {
    float2x2 _var0;
    out_half2x2(_uniforms, _var0);
    h2x2 = _var0;
}
void out_half3x3(Uniforms _uniforms, thread float3x3& v);
void _skOutParamHelper8_out_half3x3(Uniforms _uniforms, thread float3x3& h3x3) {
    float3x3 _var0;
    out_half3x3(_uniforms, _var0);
    h3x3 = _var0;
}
void out_half4x4(Uniforms _uniforms, thread float4x4& v);
void _skOutParamHelper9_out_half4x4(Uniforms _uniforms, thread float4x4& h4x4) {
    float4x4 _var0;
    out_half4x4(_uniforms, _var0);
    h4x4 = _var0;
}
void out_half3(Uniforms _uniforms, thread float3& v);
void _skOutParamHelper10_out_half3(Uniforms _uniforms, thread float3x3& h3x3) {
    float3 _var0;
    out_half3(_uniforms, _var0);
    h3x3[1] = _var0;
}
void out_half(Uniforms _uniforms, thread float& v);
void _skOutParamHelper11_out_half(Uniforms _uniforms, thread float4x4& h4x4) {
    float _var0;
    out_half(_uniforms, _var0);
    h4x4[3].w = _var0;
}
void out_half(Uniforms _uniforms, thread float& v);
void _skOutParamHelper12_out_half(Uniforms _uniforms, thread float2x2& h2x2) {
    float _var0;
    out_half(_uniforms, _var0);
    h2x2[0].x = _var0;
}
void out_int(Uniforms _uniforms, thread int& v);
void _skOutParamHelper13_out_int(Uniforms _uniforms, thread int& i) {
    int _var0;
    out_int(_uniforms, _var0);
    i = _var0;
}
void out_int2(Uniforms _uniforms, thread int2& v);
void _skOutParamHelper14_out_int2(Uniforms _uniforms, thread int2& i2) {
    int2 _var0;
    out_int2(_uniforms, _var0);
    i2 = _var0;
}
void out_int3(Uniforms _uniforms, thread int3& v);
void _skOutParamHelper15_out_int3(Uniforms _uniforms, thread int3& i3) {
    int3 _var0;
    out_int3(_uniforms, _var0);
    i3 = _var0;
}
void out_int4(Uniforms _uniforms, thread int4& v);
void _skOutParamHelper16_out_int4(Uniforms _uniforms, thread int4& i4) {
    int4 _var0;
    out_int4(_uniforms, _var0);
    i4 = _var0;
}
void out_int3(Uniforms _uniforms, thread int3& v);
void _skOutParamHelper17_out_int3(Uniforms _uniforms, thread int4& i4) {
    int3 _var0;
    out_int3(_uniforms, _var0);
    i4.xyz = _var0;
}
void out_int(Uniforms _uniforms, thread int& v);
void _skOutParamHelper18_out_int(Uniforms _uniforms, thread int2& i2) {
    int _var0;
    out_int(_uniforms, _var0);
    i2.y = _var0;
}
void out_float(Uniforms _uniforms, thread float& v);
void _skOutParamHelper19_out_float(Uniforms _uniforms, thread float& f) {
    float _var0;
    out_float(_uniforms, _var0);
    f = _var0;
}
void out_float2(Uniforms _uniforms, thread float2& v);
void _skOutParamHelper20_out_float2(Uniforms _uniforms, thread float2& f2) {
    float2 _var0;
    out_float2(_uniforms, _var0);
    f2 = _var0;
}
void out_float3(Uniforms _uniforms, thread float3& v);
void _skOutParamHelper21_out_float3(Uniforms _uniforms, thread float3& f3) {
    float3 _var0;
    out_float3(_uniforms, _var0);
    f3 = _var0;
}
void out_float4(Uniforms _uniforms, thread float4& v);
void _skOutParamHelper22_out_float4(Uniforms _uniforms, thread float4& f4) {
    float4 _var0;
    out_float4(_uniforms, _var0);
    f4 = _var0;
}
void out_float2(Uniforms _uniforms, thread float2& v);
void _skOutParamHelper23_out_float2(Uniforms _uniforms, thread float3& f3) {
    float2 _var0;
    out_float2(_uniforms, _var0);
    f3.xy = _var0;
}
void out_float(Uniforms _uniforms, thread float& v);
void _skOutParamHelper24_out_float(Uniforms _uniforms, thread float2& f2) {
    float _var0;
    out_float(_uniforms, _var0);
    f2.x = _var0;
}
void out_float2x2(Uniforms _uniforms, thread float2x2& v);
void _skOutParamHelper25_out_float2x2(Uniforms _uniforms, thread float2x2& f2x2) {
    float2x2 _var0;
    out_float2x2(_uniforms, _var0);
    f2x2 = _var0;
}
void out_float3x3(Uniforms _uniforms, thread float3x3& v);
void _skOutParamHelper26_out_float3x3(Uniforms _uniforms, thread float3x3& f3x3) {
    float3x3 _var0;
    out_float3x3(_uniforms, _var0);
    f3x3 = _var0;
}
void out_float4x4(Uniforms _uniforms, thread float4x4& v);
void _skOutParamHelper27_out_float4x4(Uniforms _uniforms, thread float4x4& f4x4) {
    float4x4 _var0;
    out_float4x4(_uniforms, _var0);
    f4x4 = _var0;
}
void out_float(Uniforms _uniforms, thread float& v);
void _skOutParamHelper28_out_float(Uniforms _uniforms, thread float2x2& f2x2) {
    float _var0;
    out_float(_uniforms, _var0);
    f2x2[0].x = _var0;
}
void out_bool(Uniforms _uniforms, thread bool& v);
void _skOutParamHelper29_out_bool(Uniforms _uniforms, thread bool& b) {
    bool _var0;
    out_bool(_uniforms, _var0);
    b = _var0;
}
void out_bool2(Uniforms _uniforms, thread bool2& v);
void _skOutParamHelper30_out_bool2(Uniforms _uniforms, thread bool2& b2) {
    bool2 _var0;
    out_bool2(_uniforms, _var0);
    b2 = _var0;
}
void out_bool3(Uniforms _uniforms, thread bool3& v);
void _skOutParamHelper31_out_bool3(Uniforms _uniforms, thread bool3& b3) {
    bool3 _var0;
    out_bool3(_uniforms, _var0);
    b3 = _var0;
}
void out_bool4(Uniforms _uniforms, thread bool4& v);
void _skOutParamHelper32_out_bool4(Uniforms _uniforms, thread bool4& b4) {
    bool4 _var0;
    out_bool4(_uniforms, _var0);
    b4 = _var0;
}
void out_bool2(Uniforms _uniforms, thread bool2& v);
void _skOutParamHelper33_out_bool2(Uniforms _uniforms, thread bool4& b4) {
    bool2 _var0;
    out_bool2(_uniforms, _var0);
    b4.xw = _var0;
}
void out_bool(Uniforms _uniforms, thread bool& v);
void _skOutParamHelper34_out_bool(Uniforms _uniforms, thread bool3& b3) {
    bool _var0;
    out_bool(_uniforms, _var0);
    b3.z = _var0;
}

void out_half(Uniforms _uniforms, thread float& v) {
    v = _uniforms.colorWhite.x;
}
void out_half2(Uniforms _uniforms, thread float2& v) {
    v = float2(_uniforms.colorWhite.y);
}
void out_half3(Uniforms _uniforms, thread float3& v) {
    v = float3(_uniforms.colorWhite.z);
}
void out_half4(Uniforms _uniforms, thread float4& v) {
    v = float4(_uniforms.colorWhite.w);
}
void out_half2x2(Uniforms _uniforms, thread float2x2& v) {
    v = float2x2(_uniforms.colorWhite.x);
}
void out_half3x3(Uniforms _uniforms, thread float3x3& v) {
    v = float3x3(_uniforms.colorWhite.y);
}
void out_half4x4(Uniforms _uniforms, thread float4x4& v) {
    v = float4x4(_uniforms.colorWhite.z);
}
void out_int(Uniforms _uniforms, thread int& v) {
    v = int(_uniforms.colorWhite.x);
}
void out_int2(Uniforms _uniforms, thread int2& v) {
    v = int2(int(_uniforms.colorWhite.y));
}
void out_int3(Uniforms _uniforms, thread int3& v) {
    v = int3(int(_uniforms.colorWhite.z));
}
void out_int4(Uniforms _uniforms, thread int4& v) {
    v = int4(int(_uniforms.colorWhite.w));
}
void out_float(Uniforms _uniforms, thread float& v) {
    v = _uniforms.colorWhite.x;
}
void out_float2(Uniforms _uniforms, thread float2& v) {
    v = float2(_uniforms.colorWhite.y);
}
void out_float3(Uniforms _uniforms, thread float3& v) {
    v = float3(_uniforms.colorWhite.z);
}
void out_float4(Uniforms _uniforms, thread float4& v) {
    v = float4(_uniforms.colorWhite.w);
}
void out_float2x2(Uniforms _uniforms, thread float2x2& v) {
    v = float2x2(_uniforms.colorWhite.x);
}
void out_float3x3(Uniforms _uniforms, thread float3x3& v) {
    v = float3x3(_uniforms.colorWhite.y);
}
void out_float4x4(Uniforms _uniforms, thread float4x4& v) {
    v = float4x4(_uniforms.colorWhite.z);
}
void out_bool(Uniforms _uniforms, thread bool& v) {
    v = bool(_uniforms.colorWhite.x);
}
void out_bool2(Uniforms _uniforms, thread bool2& v) {
    v = bool2(bool(_uniforms.colorWhite.y));
}
void out_bool3(Uniforms _uniforms, thread bool3& v) {
    v = bool3(bool(_uniforms.colorWhite.z));
}
void out_bool4(Uniforms _uniforms, thread bool4& v) {
    v = bool4(bool(_uniforms.colorWhite.w));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float h;
    _skOutParamHelper0_out_half(_uniforms, h);
    float2 h2;
    _skOutParamHelper1_out_half2(_uniforms, h2);
    float3 h3;
    _skOutParamHelper2_out_half3(_uniforms, h3);
    float4 h4;
    _skOutParamHelper3_out_half4(_uniforms, h4);
    _skOutParamHelper4_out_half(_uniforms, h3);
    _skOutParamHelper5_out_half2(_uniforms, h3);
    _skOutParamHelper6_out_half4(_uniforms, h4);
    float2x2 h2x2;
    _skOutParamHelper7_out_half2x2(_uniforms, h2x2);
    float3x3 h3x3;
    _skOutParamHelper8_out_half3x3(_uniforms, h3x3);
    float4x4 h4x4;
    _skOutParamHelper9_out_half4x4(_uniforms, h4x4);
    _skOutParamHelper10_out_half3(_uniforms, h3x3);
    _skOutParamHelper11_out_half(_uniforms, h4x4);
    _skOutParamHelper12_out_half(_uniforms, h2x2);
    int i;
    _skOutParamHelper13_out_int(_uniforms, i);
    int2 i2;
    _skOutParamHelper14_out_int2(_uniforms, i2);
    int3 i3;
    _skOutParamHelper15_out_int3(_uniforms, i3);
    int4 i4;
    _skOutParamHelper16_out_int4(_uniforms, i4);
    _skOutParamHelper17_out_int3(_uniforms, i4);
    _skOutParamHelper18_out_int(_uniforms, i2);
    float f;
    _skOutParamHelper19_out_float(_uniforms, f);
    float2 f2;
    _skOutParamHelper20_out_float2(_uniforms, f2);
    float3 f3;
    _skOutParamHelper21_out_float3(_uniforms, f3);
    float4 f4;
    _skOutParamHelper22_out_float4(_uniforms, f4);
    _skOutParamHelper23_out_float2(_uniforms, f3);
    _skOutParamHelper24_out_float(_uniforms, f2);
    float2x2 f2x2;
    _skOutParamHelper25_out_float2x2(_uniforms, f2x2);
    float3x3 f3x3;
    _skOutParamHelper26_out_float3x3(_uniforms, f3x3);
    float4x4 f4x4;
    _skOutParamHelper27_out_float4x4(_uniforms, f4x4);
    _skOutParamHelper28_out_float(_uniforms, f2x2);
    bool b;
    _skOutParamHelper29_out_bool(_uniforms, b);
    bool2 b2;
    _skOutParamHelper30_out_bool2(_uniforms, b2);
    bool3 b3;
    _skOutParamHelper31_out_bool3(_uniforms, b3);
    bool4 b4;
    _skOutParamHelper32_out_bool4(_uniforms, b4);
    _skOutParamHelper33_out_bool2(_uniforms, b4);
    _skOutParamHelper34_out_bool(_uniforms, b3);
    bool ok = true;
    ok = ok && 1.0 == (((((h * h2.x) * h3.x) * h4.x) * h2x2[0].x) * h3x3[0].x) * h4x4[0].x;
    ok = ok && 1.0 == (((((f * f2.x) * f3.x) * f4.x) * f2x2[0].x) * f3x3[0].x) * f4x4[0].x;
    ok = ok && 1 == ((i * i2.x) * i3.x) * i4.x;
    ok = ok && (((b && b2.x) && b3.x) && b4.x);
    _out.sk_FragColor = ok ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
