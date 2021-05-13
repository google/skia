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
thread bool operator==(const float4x4 left, const float4x4 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]) &&
           all(left[2] == right[2]) &&
           all(left[3] == right[3]);
}
thread bool operator!=(const float4x4 left, const float4x4 right) {
    return !(left == right);
}

bool test_half_b() {
    bool ok = true;
    float4x4 m10 = float4x4(11.0);
    ok = ok && m10 == float4x4(float4(11.0, 0.0, 0.0, 0.0), float4(0.0, 11.0, 0.0, 0.0), float4(0.0, 0.0, 11.0, 0.0), float4(0.0, 0.0, 0.0, 11.0));
    float4x4 m11 = float4x4(float4(20.0, 20.0, 20.0, 20.0), float4(20.0, 20.0, 20.0, 20.0), float4(20.0, 20.0, 20.0, 20.0), float4(20.0, 20.0, 20.0, 20.0));
    m11 -= m10;
    ok = ok && m11 == float4x4(float4(9.0, 20.0, 20.0, 20.0), float4(20.0, 9.0, 20.0, 20.0), float4(20.0, 20.0, 9.0, 20.0), float4(20.0, 20.0, 20.0, 9.0));
    return ok;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool _0_ok = true;
    float4x4 _1_m10 = float4x4(11.0);
    _0_ok = _0_ok && _1_m10 == float4x4(float4(11.0, 0.0, 0.0, 0.0), float4(0.0, 11.0, 0.0, 0.0), float4(0.0, 0.0, 11.0, 0.0), float4(0.0, 0.0, 0.0, 11.0));
    float4x4 _2_m11 = float4x4(float4(20.0, 20.0, 20.0, 20.0), float4(20.0, 20.0, 20.0, 20.0), float4(20.0, 20.0, 20.0, 20.0), float4(20.0, 20.0, 20.0, 20.0));
    _2_m11 -= _1_m10;
    _0_ok = _0_ok && _2_m11 == float4x4(float4(9.0, 20.0, 20.0, 20.0), float4(20.0, 9.0, 20.0, 20.0), float4(20.0, 20.0, 9.0, 20.0), float4(20.0, 20.0, 20.0, 9.0));
    _out.sk_FragColor = _0_ok && test_half_b() ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
