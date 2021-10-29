#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
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
    _4_ok = _4_ok && all(-half4(half(_1_two)) == half4(half(-_1_two), half3(half(-_1_two))));
    _4_ok = _4_ok && all(half2(1.0h, -2.0h) == -half2(half(_0_one - _1_two), half(_1_two)));
    _out.sk_FragColor = (_4_ok && test_ivec_b()) && test_mat_b() ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
