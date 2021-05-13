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

bool test_half_b() {
    bool ok = true;
    float3 v1 = float3x3(2.0) * float3(3.0);
    ok = ok && all(v1 == float3(6.0));
    float3 v2 = float3(3.0) * float3x3(3.0);
    ok = ok && all(v2 == float3(9.0));
    return ok;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool _0_ok = true;
    float3 _1_v1 = float3x3(2.0) * float3(3.0);
    _0_ok = _0_ok && all(_1_v1 == float3(6.0));
    float3 _2_v2 = float3(3.0) * float3x3(3.0);
    _0_ok = _0_ok && all(_2_v2 == float3(9.0));
    _out.sk_FragColor = _0_ok && test_half_b() ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
