#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 testInputs;
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};



fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 v = _uniforms.testInputs;
    v = float4(v.x, 1.0, 1.0, 1.0);
    v = float4(v.xy, 1.0, 1.0);
    v = float4(float2(v.x, float(1)), 1.0, 1.0);
    v = float4(float2(v.y, float(0)).yx, 1.0, 1.0);
    v = float4(v.xyz, 1.0);
    v = float4(float3(v.xy, float(1)), 1.0);
    v = float4(float3(v.xz, float(0)).xzy, 1.0);
    v = float4(float3(v.x, float(1), float(0)), 1.0);
    v = float4(float3(v.yz, float(1)).zxy, 1.0);
    v = float4(float3(v.y, float(0), float(1)).yxz, 1.0);
    v = float4(float2(v.z, float(1)).yyx, 1.0);
    v = v.xyzw;
    v = float4(v.xyz, float(1));
    v = float4(v.xyw, float(0)).xywz;
    v = float4(v.xy, float(1), float(0));
    v = float4(v.xzw, float(1)).xwyz;
    v = float4(v.xz, float(0), float(1)).xzyw;
    v = float3(v.xw, float(1)).xzzy;
    v = float3(v.x, float(1), float(0)).xyzy;
    v = float4(v.yzw, float(1)).wxyz;
    v = float4(v.yz, float(0), float(1)).zxyw;
    v = float4(v.yw, float(0), float(1)).zxwy;
    v = float2(v.y, float(1)).yxyy;
    v = float3(v.zw, float(0)).zzxy;
    v = float3(v.z, float(0), float(1)).yyxz;
    v = float3(v.w, float(0), float(1)).yzzx;
    _out.sk_FragColor = all(v == float4(0.0, 1.0, 1.0, 1.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
