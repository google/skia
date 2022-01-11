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
struct Globals {
    const int minus;
    const int star;
    const int slash;
};
thread float2x2 operator/(const float2x2 left, const float2x2 right) {
    return float2x2(left[0] / right[0], left[1] / right[1]);
}
thread float2x2& operator/=(thread float2x2& left, thread const float2x2& right) {
    left = left / right;
    return left;
}
bool test_bifffff22(Uniforms _uniforms, int op, float m11, float m12, float m21, float m22, float2x2 expected) {
    float one = float(_uniforms.colorRed.x);
    float2x2 m2 = float2x2(float2(m11 * one, m12 * one), float2(m21 * one, m22 * one));
    switch (op) {
        case 1:
            m2 += (float2x2(1.0, 1.0, 1.0, 1.0) * 1.0);
            break;
        case 2:
            m2 -= (float2x2(1.0, 1.0, 1.0, 1.0) * 1.0);
            break;
        case 3:
            m2 *= 2.0;
            break;
        case 4:
            m2 /= (float2x2(1.0, 1.0, 1.0, 1.0) * 2.0);
            break;
    }
    return ((m2[0].x == expected[0].x && m2[0].y == expected[0].y) && m2[1].x == expected[1].x) && m2[1].y == expected[1].y;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{2, 3, 4};
    (void)_globals;
    Outputs _out;
    (void)_out;
    float f1 = float(_uniforms.colorGreen.y);
    float f2 = float(2.0h * _uniforms.colorGreen.y);
    float f3 = float(3.0h * _uniforms.colorGreen.y);
    float f4 = float(4.0h * _uniforms.colorGreen.y);
    float2x2 _0_expected = float2x2(float2(f1 + 1.0, f2 + 1.0), float2(f3 + 1.0, f4 + 1.0));
    float _1_one = float(_uniforms.colorRed.x);
    float2x2 _2_m2 = float2x2(float2(f1 * _1_one, f2 * _1_one), float2(f3 * _1_one, f4 * _1_one));
    {
        _2_m2 += (float2x2(1.0, 1.0, 1.0, 1.0) * 1.0);
    }
    _out.sk_FragColor = (((((_2_m2[0].x == _0_expected[0].x && _2_m2[0].y == _0_expected[0].y) && _2_m2[1].x == _0_expected[1].x) && _2_m2[1].y == _0_expected[1].y) && test_bifffff22(_uniforms, _globals.minus, f1, f2, f3, f4, float2x2(float2(f1 - 1.0, f2 - 1.0), float2(f3 - 1.0, f4 - 1.0)))) && test_bifffff22(_uniforms, _globals.star, f1, f2, f3, f4, float2x2(float2(f1 * 2.0, f2 * 2.0), float2(f3 * 2.0, f4 * 2.0)))) && test_bifffff22(_uniforms, _globals.slash, f1, f2, f3, f4, float2x2(float2(f1 / 2.0, f2 / 2.0), float2(f3 / 2.0, f4 / 2.0))) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
