#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
void out_half(thread float& v);
void _SwizzleHelper0_for_out_half(thread float3& h3) {
    float var0 = h3[1];
    out_half(var0);
    h3[1] = var0;
}
void out_half2(thread float2& v);
void _SwizzleHelper1_for_out_half2(thread float3& h3) {
    float2 var0 = h3.xz;
    out_half2(var0);
    h3.xz = var0;
}
void out_half4(thread float4& v);
void _SwizzleHelper2_for_out_half4(thread float4& h4) {
    float4 var0 = h4.zwxy;
    out_half4(var0);
    h4.zwxy = var0;
}
void out_half4(thread float4& v);
void _SwizzleHelper3_for_out_half4(thread float4x4& h4x4) {
    float4 var0 = h4x4[3].zwxy;
    out_half4(var0);
    h4x4[3].zwxy = var0;
}
void out_int3(thread int3& v);
void _SwizzleHelper4_for_out_int3(thread int4& i4) {
    int3 var0 = i4.xyz;
    out_int3(var0);
    i4.xyz = var0;
}
void out_float2(thread float2& v);
void _SwizzleHelper5_for_out_float2(thread float3& f3) {
    float2 var0 = f3.xy;
    out_float2(var0);
    f3.xy = var0;
}
void out_float(thread float& v);
void _SwizzleHelper6_for_out_float(thread float2& f2) {
    float var0 = f2[0];
    out_float(var0);
    f2[0] = var0;
}
void out_float(thread float& v);
void _SwizzleHelper7_for_out_float(thread float2x2& f2x2) {
    float var0 = f2x2[0][0];
    out_float(var0);
    f2x2[0][0] = var0;
}
void out_bool2(thread bool2& v);
void _SwizzleHelper8_for_out_bool2(thread bool4& b4) {
    bool2 var0 = b4.xw;
    out_bool2(var0);
    b4.xw = var0;
}
void out_bool(thread bool& v);
void _SwizzleHelper9_for_out_bool(thread bool3& b3) {
    bool var0 = b3[2];
    out_bool(var0);
    b3[2] = var0;
}
void out_half_with_extra_params(float4x4 unusedF4x4, thread float& v, int unusedInt);
void _SwizzleHelper10_for_out_half_with_extra_params(float4x4 var0, thread float4& h4, int var2) {
    float var1 = h4.z;
    out_half_with_extra_params(var0, var1, var2);
    h4.z = var1;
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
void out_half_with_extra_params(float4x4 unusedF4x4, thread float& v, int unusedInt) {
    v = 1.0;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float h;
    out_half(h);
    float2 h2;
    out_half2(h2);
    float3 h3;
    out_half3(h3);
    float4 h4;
    out_half4(h4);
    _SwizzleHelper0_for_out_half(h3);
    _SwizzleHelper1_for_out_half2(h3);
    _SwizzleHelper2_for_out_half4(h4);
    _out->sk_FragColor = float4(h, h2.x, h3.x, h4.x);
    float2x2 h2x2;
    out_half2x2(h2x2);
    float3x3 h3x3;
    out_half3x3(h3x3);
    float4x4 h4x4;
    out_half4x4(h4x4);
    out_half3(h3x3[1]);
    _SwizzleHelper3_for_out_half4(h4x4);
    out_half2(h2x2[0]);
    _out->sk_FragColor = float4(h2x2[0][0], h3x3[0][0], h4x4[0][0], 1.0);
    int i;
    out_int(i);
    int2 i2;
    out_int2(i2);
    int3 i3;
    out_int3(i3);
    int4 i4;
    out_int4(i4);
    _SwizzleHelper4_for_out_int3(i4);
    _out->sk_FragColor = float4(float(i), float(i2.x), float(i3.x), float(i4.x));
    float f;
    out_float(f);
    float2 f2;
    out_float2(f2);
    float3 f3;
    out_float3(f3);
    float4 f4;
    out_float4(f4);
    _SwizzleHelper5_for_out_float2(f3);
    _SwizzleHelper6_for_out_float(f2);
    _out->sk_FragColor = float4(f, f2.x, f3.x, f4.x);
    float2x2 f2x2;
    out_float2x2(f2x2);
    float3x3 f3x3;
    out_float3x3(f3x3);
    float4x4 f4x4;
    out_float4x4(f4x4);
    _SwizzleHelper7_for_out_float(f2x2);
    out_float4(f4x4[1]);
    _out->sk_FragColor = float4(f2x2[0][0], f3x3[0][0], f4x4[0][0], 1.0);
    bool b;
    out_bool(b);
    bool2 b2;
    out_bool2(b2);
    bool3 b3;
    out_bool3(b3);
    bool4 b4;
    out_bool4(b4);
    _SwizzleHelper8_for_out_bool2(b4);
    _SwizzleHelper9_for_out_bool(b3);
    _out->sk_FragColor = float4(b ? 1.0 : 0.0, b2.x ? 1.0 : 0.0, b3.x ? 1.0 : 0.0, b4.x ? 1.0 : 0.0);
    _SwizzleHelper10_for_out_half_with_extra_params(f4x4, h4, 123);
    _out->sk_FragColor = h4.zzzz;
    return *_out;
}
