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
float fn_hh4(float4 v) {
    for (int x = 1;x <= 2; ++x) {
        return v.x;
    }
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 v = _uniforms.testInputs;
    v = float4(0.0, v.zyx);
    v = float4(0.0, 0.0, v.xw);
    v = float4(1.0, 1.0, v.wx);
    v = float4(v.zy, 1.0, 1.0);
    v = float4(v.xx, 1.0, 1.0);
    v = v.wzwz;
    v = float3(fn_hh4(v), 123.0, 456.0).yyzz;
    v = float3(fn_hh4(v), 123.0, 456.0).yyzz;
    v = float4(123.0, 456.0, 456.0, fn_hh4(v));
    v = float4(123.0, 456.0, 456.0, fn_hh4(v));
    v = float3(fn_hh4(v), 123.0, 456.0).yxxz;
    v = float3(fn_hh4(v), 123.0, 456.0).yxxz;
    v = float4(1.0, 1.0, 2.0, 3.0);
    v = float4(_uniforms.colorRed.xyz, 1.0);
    v = float4(_uniforms.colorRed.x, 1.0, _uniforms.colorRed.yz);
    v.wzyx = v;
    v.xw = v.yz;
    v.zyx = float3(v.ww, 1.0);
    _out.sk_FragColor = all(v == float4(1.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
