#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 colorGreen;
    float4 colorRed;
    float2x2 testMatrix2x2;
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
    bool ok = true;
    ok = ok && all(float4_from_float2x2(_uniforms.testMatrix2x2) == float4(1.0, 2.0, 3.0, 4.0));
    ok = ok && all(float4_from_float2x2(_uniforms.testMatrix2x2) == float4(1.0, 2.0, 3.0, 4.0));
    ok = ok && all(int4(float4_from_float2x2(_uniforms.testMatrix2x2)) == int4(1, 2, 3, 4));
    ok = ok && all(bool4(float4_from_float2x2(_uniforms.testMatrix2x2)) == bool4(true, true, true, true));
    _out.sk_FragColor = ok ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
