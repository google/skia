#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
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
    out_half2(h3.xz);
    out_half4(h4.zwxy);
    _out->sk_FragColor = float4(h, h2.x, h3.x, h4.x);
    float2x2 h2x2;
    out_half2x2(h2x2);
    float3x3 h3x3;
    out_half3x3(h3x3);
    float4x4 h4x4;
    out_half4x4(h4x4);
    out_half3(h3x3[1]);
    out_half(h4x4[3].w);
    _out->sk_FragColor = float4(h2x2[0][0], h3x3[0][0], h4x4[0][0], 1.0);
    int i;
    out_int(i);
    int2 i2;
    out_int2(i2);
    int3 i3;
    out_int3(i3);
    int4 i4;
    out_int4(i4);
    out_int3(i4.xyz);
    _out->sk_FragColor = float4(float(i), float(i2.x), float(i3.x), float(i4.x));
    float f;
    out_float(f);
    float2 f2;
    out_float2(f2);
    float3 f3;
    out_float3(f3);
    float4 f4;
    out_float4(f4);
    out_float2(f3.xy);
    _out->sk_FragColor = float4(f, f2.x, f3.x, f4.x);
    float2x2 f2x2;
    out_float2x2(f2x2);
    float3x3 f3x3;
    out_float3x3(f3x3);
    float4x4 f4x4;
    out_float4x4(f4x4);
    out_float(f2x2[0][0]);
    _out->sk_FragColor = float4(f2x2[0][0], f3x3[0][0], f4x4[0][0], 1.0);
    bool b;
    out_bool(b);
    bool2 b2;
    out_bool2(b2);
    bool3 b3;
    out_bool3(b3);
    bool4 b4;
    out_bool4(b4);
    out_bool2(b4.xw);
    _out->sk_FragColor = float4(b ? 1.0 : 0.0, b2.x ? 1.0 : 0.0, b3.x ? 1.0 : 0.0, b4.x ? 1.0 : 0.0);
    return *_out;
}
