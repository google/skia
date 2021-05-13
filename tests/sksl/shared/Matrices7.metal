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
thread bool operator==(const float2x2 left, const float2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x2 left, const float2x2 right) {
    return !(left == right);
}

bool test_half_b() {
    bool ok = true;
    float2x2 m7 = float2x2(float2(5.0, 6.0), float2(7.0, 8.0));
    ok = ok && m7 == float2x2(float2(5.0, 6.0), float2(7.0, 8.0));
    return ok;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool _0_ok = true;
    float2x2 _1_m7 = float2x2(float2(5.0, 6.0), float2(7.0, 8.0));
    _0_ok = _0_ok && _1_m7 == float2x2(float2(5.0, 6.0), float2(7.0, 8.0));
    _out.sk_FragColor = _0_ok && test_half_b() ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
