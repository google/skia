#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float testInput;
    float2x2 testMatrix2x2;
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

float4 float4_from_float2x2(float2x2 x) {
    return float4(x[0].xy, x[1].xy);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 inputVal = float4_from_float2x2(_uniforms.testMatrix2x2) * float4(1.0, 1.0, -1.0, -1.0);
    const uint4 expectedB = uint4(1065353216u, 1073741824u, 3225419776u, 3229614080u);
    _out.sk_FragColor = ((as_type<uint>(inputVal.x) == 1065353216u && all(as_type<uint2>(inputVal.xy) == uint2(1065353216u, 1073741824u))) && all(as_type<uint3>(inputVal.xyz) == uint3(1065353216u, 1073741824u, 3225419776u))) && all(as_type<uint4>(inputVal) == expectedB) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
