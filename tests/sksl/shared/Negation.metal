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
bool test_ivec_b() {
    int one = 1;
    const int two = 2;
    bool ok = true;
    ok = ok && all(-int2(-one, one + one) == -int2(one - two, 2));
    return ok;
}
bool test_mat_b() {
    bool ok = true;
    return ok;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    const float _0_one = 1.0;
    float _1_two = 2.0;
    bool _4_ok = true;
    _4_ok = _4_ok && all(-float4(_1_two) == float4(-_1_two, float3(-_1_two)));
    _4_ok = _4_ok && all(float2(1.0, -2.0) == -float2(_0_one - _1_two, _1_two));
    _out.sk_FragColor = (_4_ok && test_ivec_b()) && test_mat_b() ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
