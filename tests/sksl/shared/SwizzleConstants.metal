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
    v = float4(v.x, 1.0, 1.0, 1.0);
    v = float4(0.0, v.y, 1.0, 1.0);
    v = float4(v.xyz, 1.0);
    v = float4(v.xy, 1.0, 1.0);
    v = float4(v.x, 0.0, v.z, 1.0);
    v = float4(v.x, 1.0, 0.0, 1.0);
    v = float4(1.0, v.yz, 1.0);
    v = float4(0.0, v.y, 1.0, 1.0);
    v = float4(1.0, 1.0, v.z, 1.0);
    v = float4(v.xyz, 1.0);
    v = float4(v.xy, 0.0, v.w);
    v = float4(v.xy, 1.0, 0.0);
    v = float4(v.x, 1.0, v.zw);
    v = float4(v.x, 0.0, v.z, 1.0);
    v = float4(v.x, 1.0, 1.0, v.w);
    v = float4(v.x, 1.0, 0.0, 1.0);
    v = float4(1.0, v.yzw);
    v = float4(0.0, v.yz, 1.0);
    v = float4(0.0, v.y, 1.0, v.w);
    v = float4(1.0, v.y, 1.0, 1.0);
    v = float4(0.0, 0.0, v.zw);
    v = float4(0.0, 0.0, v.z, 1.0);
    v = float4(0.0, 1.0, 1.0, v.w);
    _out.sk_FragColor = all(v == float4(0.0, 1.0, 1.0, 1.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
