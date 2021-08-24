#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
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
    float4 infiniteValue = float4_from_float2x2(_uniforms.testMatrix2x2) / _uniforms.colorGreen.x;
    float4 finiteValue = float4_from_float2x2(_uniforms.testMatrix2x2) / _uniforms.colorGreen.y;
    _out.sk_FragColor = ((((((isinf(infiniteValue.x) && all(isinf(infiniteValue.xy))) && all(isinf(infiniteValue.xyz))) && all(isinf(infiniteValue))) && !isinf(finiteValue.x)) && !any(isinf(finiteValue.xy))) && !any(isinf(finiteValue.xyz))) && !any(isinf(finiteValue)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
