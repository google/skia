#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
void out_half(thread float& v);
void _skOutParamHelper0_out_half(thread float& h) {
    float _var0;
    out_half(_var0);
    h = _var0;
}
void out_half2(thread float2& v);
void _skOutParamHelper1_out_half2(thread float2& h2) {
    float2 _var0;
    out_half2(_var0);
    h2 = _var0;
}
void out_half3(thread float3& v);
void _skOutParamHelper2_out_half3(thread float3& h3) {
    float3 _var0;
    out_half3(_var0);
    h3 = _var0;
}
void out_half4(thread float4& v);
void _skOutParamHelper3_out_half4(thread float4& h4) {
    float4 _var0;
    out_half4(_var0);
    h4 = _var0;
}
void out_half(thread float& v);
void _skOutParamHelper4_out_half(thread float3& h3) {
    float _var0;
    out_half(_var0);
    h3.y = _var0;
}
void out_half2(thread float2& v);
void _skOutParamHelper5_out_half2(thread float3& h3) {
    float2 _var0;
    out_half2(_var0);
    h3.xz = _var0;
}
void out_half4(thread float4& v);
void _skOutParamHelper6_out_half4(thread float4& h4) {
    float4 _var0;
    out_half4(_var0);
    h4.zwxy = _var0;
}
void out_pair(thread float& v1, thread float& v2);
void _skOutParamHelper7_out_pair(thread float& h, thread float& h1) {
    float _var0;
    float _var1;
    out_pair(_var0, _var1);
    h = _var0;
    h1 = _var1;
}
void out_pair(thread float& v1, thread float& v2);
void _skOutParamHelper8_out_pair(thread float& h, thread float&) {
    float _var0;
    float _var1;
    out_pair(_var0, _var1);
    h = _var0;
    h = _var1;
}
void out_pair(thread float& v1, thread float& v2);
void _skOutParamHelper9_out_pair(thread float2& h2, thread float2&) {
    float _var0;
    float _var1;
    out_pair(_var0, _var1);
    h2.x = _var0;
    h2.y = _var1;
}
void out_pair(thread float& v1, thread float& v2);
void _skOutParamHelper10_out_pair(thread float2& h2, thread float2&) {
    float _var0;
    float _var1;
    out_pair(_var0, _var1);
    h2.x = _var0;
    h2.x = _var1;
}
void out_pair(thread float& v1, thread float& v2);
void _skOutParamHelper11_out_pair(thread float2& h2, thread float3& h3) {
    float _var0;
    float _var1;
    out_pair(_var0, _var1);
    h2.x = _var0;
    h3.x = _var1;
}
void out_half2x2(thread float2x2& v);
void _skOutParamHelper12_out_half2x2(thread float2x2& h2x2) {
    float2x2 _var0;
    out_half2x2(_var0);
    h2x2 = _var0;
}
void out_half3x3(thread float3x3& v);
void _skOutParamHelper13_out_half3x3(thread float3x3& h3x3) {
    float3x3 _var0;
    out_half3x3(_var0);
    h3x3 = _var0;
}
void out_half4x4(thread float4x4& v);
void _skOutParamHelper14_out_half4x4(thread float4x4& h4x4) {
    float4x4 _var0;
    out_half4x4(_var0);
    h4x4 = _var0;
}
void out_half3(thread float3& v);
void _skOutParamHelper15_out_half3(thread float3x3& h3x3) {
    float3 _var0;
    out_half3(_var0);
    h3x3[1] = _var0;
}
void out_half4(thread float4& v);
void _skOutParamHelper16_out_half4(thread float4x4& h4x4) {
    float4 _var0;
    out_half4(_var0);
    h4x4[3].zwxy = _var0;
}
void out_half2(thread float2& v);
void _skOutParamHelper17_out_half2(thread float2x2& h2x2) {
    float2 _var0;
    out_half2(_var0);
    h2x2[0] = _var0;
}
void out_int(thread int& v);
void _skOutParamHelper18_out_int(thread int& i) {
    int _var0;
    out_int(_var0);
    i = _var0;
}
void out_int2(thread int2& v);
void _skOutParamHelper19_out_int2(thread int2& i2) {
    int2 _var0;
    out_int2(_var0);
    i2 = _var0;
}
void out_int3(thread int3& v);
void _skOutParamHelper20_out_int3(thread int3& i3) {
    int3 _var0;
    out_int3(_var0);
    i3 = _var0;
}
void out_int4(thread int4& v);
void _skOutParamHelper21_out_int4(thread int4& i4) {
    int4 _var0;
    out_int4(_var0);
    i4 = _var0;
}
void out_int3(thread int3& v);
void _skOutParamHelper22_out_int3(thread int4& i4) {
    int3 _var0;
    out_int3(_var0);
    i4.xyz = _var0;
}
void out_float(thread float& v);
void _skOutParamHelper23_out_float(thread float& f) {
    float _var0;
    out_float(_var0);
    f = _var0;
}
void out_float2(thread float2& v);
void _skOutParamHelper24_out_float2(thread float2& f2) {
    float2 _var0;
    out_float2(_var0);
    f2 = _var0;
}
void out_float3(thread float3& v);
void _skOutParamHelper25_out_float3(thread float3& f3) {
    float3 _var0;
    out_float3(_var0);
    f3 = _var0;
}
void out_float4(thread float4& v);
void _skOutParamHelper26_out_float4(thread float4& f4) {
    float4 _var0;
    out_float4(_var0);
    f4 = _var0;
}
void out_float2(thread float2& v);
void _skOutParamHelper27_out_float2(thread float3& f3) {
    float2 _var0;
    out_float2(_var0);
    f3.xy = _var0;
}
void out_float(thread float& v);
void _skOutParamHelper28_out_float(thread float2& f2) {
    float _var0;
    out_float(_var0);
    f2.x = _var0;
}
void out_float2x2(thread float2x2& v);
void _skOutParamHelper29_out_float2x2(thread float2x2& f2x2) {
    float2x2 _var0;
    out_float2x2(_var0);
    f2x2 = _var0;
}
void out_float3x3(thread float3x3& v);
void _skOutParamHelper30_out_float3x3(thread float3x3& f3x3) {
    float3x3 _var0;
    out_float3x3(_var0);
    f3x3 = _var0;
}
void out_float4x4(thread float4x4& v);
void _skOutParamHelper31_out_float4x4(thread float4x4& f4x4) {
    float4x4 _var0;
    out_float4x4(_var0);
    f4x4 = _var0;
}
void out_float(thread float& v);
void _skOutParamHelper32_out_float(thread float2x2& f2x2) {
    float _var0;
    out_float(_var0);
    f2x2[0].x = _var0;
}
void out_float4(thread float4& v);
void _skOutParamHelper33_out_float4(thread float4x4& f4x4) {
    float4 _var0;
    out_float4(_var0);
    f4x4[1] = _var0;
}
void out_bool(thread bool& v);
void _skOutParamHelper34_out_bool(thread bool& b) {
    bool _var0;
    out_bool(_var0);
    b = _var0;
}
void out_bool2(thread bool2& v);
void _skOutParamHelper35_out_bool2(thread bool2& b2) {
    bool2 _var0;
    out_bool2(_var0);
    b2 = _var0;
}
void out_bool3(thread bool3& v);
void _skOutParamHelper36_out_bool3(thread bool3& b3) {
    bool3 _var0;
    out_bool3(_var0);
    b3 = _var0;
}
void out_bool4(thread bool4& v);
void _skOutParamHelper37_out_bool4(thread bool4& b4) {
    bool4 _var0;
    out_bool4(_var0);
    b4 = _var0;
}
void out_bool2(thread bool2& v);
void _skOutParamHelper38_out_bool2(thread bool4& b4) {
    bool2 _var0;
    out_bool2(_var0);
    b4.xw = _var0;
}
void out_bool(thread bool& v);
void _skOutParamHelper39_out_bool(thread bool3& b3) {
    bool _var0;
    out_bool(_var0);
    b3.z = _var0;
}
void out_half(thread float& v) {
    v = 1.0;
}
void out_half2(thread float2& v) {
    v = float2(2.0);
}
void out_half3(thread float3& v) {
    v = float3(3.0);
}
void out_half4(thread float4& v) {
    v = float4(4.0);
}
void out_half2x2(thread float2x2& v) {
    v = float2x2(2.0);
}
void out_half3x3(thread float3x3& v) {
    v = float3x3(3.0);
}
void out_half4x4(thread float4x4& v) {
    v = float4x4(4.0);
}
void out_int(thread int& v) {
    v = 1;
}
void out_int2(thread int2& v) {
    v = int2(2);
}
void out_int3(thread int3& v) {
    v = int3(3);
}
void out_int4(thread int4& v) {
    v = int4(4);
}
void out_float(thread float& v) {
    v = 1.0;
}
void out_float2(thread float2& v) {
    v = float2(2.0);
}
void out_float3(thread float3& v) {
    v = float3(3.0);
}
void out_float4(thread float4& v) {
    v = float4(4.0);
}
void out_float2x2(thread float2x2& v) {
    v = float2x2(2.0);
}
void out_float3x3(thread float3x3& v) {
    v = float3x3(3.0);
}
void out_float4x4(thread float4x4& v) {
    v = float4x4(4.0);
}
void out_bool(thread bool& v) {
    v = true;
}
void out_bool2(thread bool2& v) {
    v = bool2(false);
}
void out_bool3(thread bool3& v) {
    v = bool3(true);
}
void out_bool4(thread bool4& v) {
    v = bool4(false);
}
void out_pair(thread float& v1, thread float& v2) {
    v1 = 1.0;
    v2 = 2.0;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float h;
    _skOutParamHelper0_out_half(h);
    float2 h2;
    _skOutParamHelper1_out_half2(h2);
    float3 h3;
    _skOutParamHelper2_out_half3(h3);
    float4 h4;
    _skOutParamHelper3_out_half4(h4);
    _skOutParamHelper4_out_half(h3);
    _skOutParamHelper5_out_half2(h3);
    _skOutParamHelper6_out_half4(h4);
    _out.sk_FragColor = float4(h, h2.x, h3.x, h4.x);
    float h1;
    _skOutParamHelper7_out_pair(h, h1);
    _skOutParamHelper8_out_pair(h, h);
    _skOutParamHelper9_out_pair(h2, h2);
    _skOutParamHelper10_out_pair(h2, h2);
    _skOutParamHelper11_out_pair(h2, h3);
    float2x2 h2x2;
    _skOutParamHelper12_out_half2x2(h2x2);
    float3x3 h3x3;
    _skOutParamHelper13_out_half3x3(h3x3);
    float4x4 h4x4;
    _skOutParamHelper14_out_half4x4(h4x4);
    _skOutParamHelper15_out_half3(h3x3);
    _skOutParamHelper16_out_half4(h4x4);
    _skOutParamHelper17_out_half2(h2x2);
    _out.sk_FragColor = float4(h2x2[0].x, h3x3[0].x, h4x4[0].x, 1.0);
    int i;
    _skOutParamHelper18_out_int(i);
    int2 i2;
    _skOutParamHelper19_out_int2(i2);
    int3 i3;
    _skOutParamHelper20_out_int3(i3);
    int4 i4;
    _skOutParamHelper21_out_int4(i4);
    _skOutParamHelper22_out_int3(i4);
    _out.sk_FragColor = float4(float(i), float(i2.x), float(i3.x), float(i4.x));
    float f;
    _skOutParamHelper23_out_float(f);
    float2 f2;
    _skOutParamHelper24_out_float2(f2);
    float3 f3;
    _skOutParamHelper25_out_float3(f3);
    float4 f4;
    _skOutParamHelper26_out_float4(f4);
    _skOutParamHelper27_out_float2(f3);
    _skOutParamHelper28_out_float(f2);
    _out.sk_FragColor = float4(f, f2.x, f3.x, f4.x);
    float2x2 f2x2;
    _skOutParamHelper29_out_float2x2(f2x2);
    float3x3 f3x3;
    _skOutParamHelper30_out_float3x3(f3x3);
    float4x4 f4x4;
    _skOutParamHelper31_out_float4x4(f4x4);
    _skOutParamHelper32_out_float(f2x2);
    _skOutParamHelper33_out_float4(f4x4);
    _out.sk_FragColor = float4(f2x2[0].x, f3x3[0].x, f4x4[0].x, 1.0);
    bool b;
    _skOutParamHelper34_out_bool(b);
    bool2 b2;
    _skOutParamHelper35_out_bool2(b2);
    bool3 b3;
    _skOutParamHelper36_out_bool3(b3);
    bool4 b4;
    _skOutParamHelper37_out_bool4(b4);
    _skOutParamHelper38_out_bool2(b4);
    _skOutParamHelper39_out_bool(b3);
    _out.sk_FragColor = float4(float(b), float(b2.x), float(b3.x), float(b4.x));
    return _out;
}
