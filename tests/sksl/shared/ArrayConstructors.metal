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
    array<float, 4> test1 = array<float, 4>{1.0, 2.0, 3.0, 4.0};
    array<float2, 2> test2 = array<float2, 2>{float2(1.0, 2.0), float2(3.0, 4.0)};
    array<float4x4, 1> test3 = array<float4x4, 1>{float4x4(16.0)};
    _out.sk_FragColor = (test1[3] + test2[1].y) + test3[0][3].w == 24.0 ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
