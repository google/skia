#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 pi;
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
    _out.sk_FragColor = ((abs(cos(_uniforms.pi.x)) < 0.0010000000474974513 && all((abs(cos(_uniforms.pi.xy) - float2(0.0, -1.0)) < float2(0.0010000000474974513)))) && all((abs(cos(_uniforms.pi.xyz) - float3(0.0, -1.0, 0.0)) < float3(0.0010000000474974513)))) && all((abs(cos(_uniforms.pi) - float4(0.0, -1.0, 0.0, 1.0)) < float4(0.0010000000474974513))) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
