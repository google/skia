#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    float2x2 testMatrix2x2;
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};

float4 float4_from_float2x2(float2x2 x) {
    return float4(x[0].xy, x[1].xy);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 inputVal = float4_from_float2x2(_uniforms.testMatrix2x2) * float4(1.0, 1.0, -1.0, -1.0);
    const int4 expectedB = int4(1065353216, 1073741824, -1069547520, -1065353216);
    _out.sk_FragColor = ((as_type<int>(inputVal.x) == 1065353216 && all(as_type<int2>(inputVal.xy) == int2(1065353216, 1073741824))) && all(as_type<int3>(inputVal.xyz) == int3(1065353216, 1073741824, -1069547520))) && all(as_type<int4>(inputVal) == expectedB) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
