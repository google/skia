#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float2 ah;
    float2 bh;
    float2 af;
    float2 bf;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = _uniforms.ah.x * _uniforms.bh.y - _uniforms.ah.y * _uniforms.bh.x;
    _out.sk_FragColor.y = _uniforms.af.x * _uniforms.bf.y - _uniforms.af.y * _uniforms.bf.x;
    _out.sk_FragColor.z = 12.0;
    _out.sk_FragColor.xyz = float3(-8.0, -8.0, 12.0);
    _out.sk_FragColor.yzw = float3(9.0, -18.0, -9.0);
    return _out;
}
