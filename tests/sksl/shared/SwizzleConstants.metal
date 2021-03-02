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
    v = float4(float2(v.x, 1.0), 1.0, 1.0);
    v = float4(float2(v.y, 0.0).yx, 1.0, 1.0);
    v = float4(v.xyz, 1.0);
    v = float4(float3(v.xy, 1.0), 1.0);
    v = float4(float3(v.xz, 0.0).xzy, 1.0);
    v = float4(float3(v.x, 1.0, 0.0), 1.0);
    v = float4(float3(v.yz, 1.0).zxy, 1.0);
    v = float4(float3(v.y, 0.0, 1.0).yxz, 1.0);
    v = float4(float2(v.z, 1.0).yyx, 1.0);
    v = float4(v.xyz, 1.0);
    v = float4(v.xyw, 0.0).xywz;
    v = float4(v.xy, 1.0, 0.0);
    v = float4(v.xzw, 1.0).xwyz;
    v = float4(v.xz, 0.0, 1.0).xzyw;
    v = float3(v.xw, 1.0).xzzy;
    v = float3(v.x, 1.0, 0.0).xyzy;
    v = float4(v.yzw, 1.0).wxyz;
    v = float4(v.yz, 0.0, 1.0).zxyw;
    v = float4(v.yw, 0.0, 1.0).zxwy;
    v = float2(v.y, 1.0).yxyy;
    v = float3(v.zw, 0.0).zzxy;
    v = float3(v.z, 0.0, 1.0).yyxz;
    v = float3(v.w, 0.0, 1.0).yzzx;
    _out.sk_FragColor = all(v == float4(0.0, 1.0, 1.0, 1.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
