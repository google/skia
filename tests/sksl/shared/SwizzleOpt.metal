#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorRed;
    float4 colorGreen;
    float4 testInputs;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};



float fn(float4 v) {
    for (int x = 1;x <= 2; ++x) {
        return v.x;
    }
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 v = _uniforms.testInputs;
    v = float4(v.xyz, 0.0).wzyx;
    v = float3(v.xw, 0.0).zzxy;
    v = float3(float3(v.xxxw.xw, 0.0).zzxy.wz, 1.0).zzxy;
    v = float3(v.wzyw.yz, 1.0).xyzz;
    v = v.wzyx.wzyx;
    v = float4(v.xxxx.zz, 1.0, 1.0);
    v = v.zw.yxyx;
    v = float3(fn(v), 123.0, 456.0).yyzz;
    v = float3(fn(v), float2(123.0, 456.0)).yyzz;
    v = float3(fn(v), 123.0, 456.0).yzzx;
    v = float3(fn(v), float2(123.0, 456.0)).yzzx;
    v = float3(fn(v), 123.0, 456.0).yxxz;
    v = float3(fn(v), float2(123.0, 456.0)).yxxz;
    v = float4(1.0, 2.0, 3.0, 4.0).xxyz;
    v = float4(1.0, _uniforms.colorRed.xyz).yzwx;
    v = float4(1.0, _uniforms.colorRed.xyz).yxzw;
    v.wzyx = v;
    v.xw = v.yz;
    v.wzyx.yzw = float3(v.ww, 1.0);
    _out.sk_FragColor = all(v == float4(1.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
