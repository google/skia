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
float2x2 float2x2_from_float4(float4 x0) {
    return float2x2(float2(x0[0], x0[1]), float2(x0[2], x0[3]));
}

bool test_half_b() {
    bool ok = true;
    float2x2 m1 = float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    ok = ok && m1 == float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    float2x2 m2 = float2x2_from_float4(float4(5.0));
    ok = ok && m2 == float2x2(float2(5.0, 5.0), float2(5.0, 5.0));
    float2x2 m3 = m1;
    ok = ok && m3 == float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    return ok;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool _0_ok = true;
    float2x2 _1_m1 = float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    _0_ok = _0_ok && _1_m1 == float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    float2x2 _2_m2 = float2x2_from_float4(float4(5.0));
    _0_ok = _0_ok && _2_m2 == float2x2(float2(5.0, 5.0), float2(5.0, 5.0));
    float2x2 _3_m3 = _1_m1;
    _0_ok = _0_ok && _3_m3 == float2x2(float2(1.0, 2.0), float2(3.0, 4.0));
    _out.sk_FragColor = _0_ok && test_half_b() ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
