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
    int4 expectedB = int4(1065353216, 1073741824, -1069547520, -1065353216);
    _out.sk_FragColor = ((inputVal.x == as_type<float>(expectedB.x) && all(inputVal.xy == as_type<float2>(expectedB.xy))) && all(inputVal.xyz == as_type<float3>(expectedB.xyz))) && all(inputVal == as_type<float4>(expectedB)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
