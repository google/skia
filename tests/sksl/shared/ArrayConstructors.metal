#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
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
    _out.sk_FragColor = (array<float, 4>{1.0, 2.0, 3.0, 4.0}[3] + array<float2, 2>{float2(1.0, 2.0), float2(3.0, 4.0)}[1].y) + array<float4x4, 1>{float4x4(16.0)}[0][3].w == 24.0 ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
