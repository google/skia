#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
constant const int minus = 2;
constant const int star = 3;
constant const int slash = 4;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
    float4 testInputs;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};

thread bool operator==(const float2x2 left, const float2x2 right);
thread bool operator!=(const float2x2 left, const float2x2 right);
thread float2x2 operator/(const float2x2 left, const float2x2 right) {
    return float2x2(left[0] / right[0], left[1] / right[1]);
}
thread float2x2& operator/=(thread float2x2& left, thread const float2x2& right) {
    left = left / right;
    return left;
}
thread bool operator==(const float2x2 left, const float2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x2 left, const float2x2 right) {
    return !(left == right);
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
            m2 *= 0.5;
            break;
    }
    return ((m2[0].x == expected[0].x && m2[0].y == expected[0].y) && m2[1].x == expected[1].x) && m2[1].y == expected[1].y;
}
bool divisionTest_b(Uniforms _uniforms) {
    float ten = float(_uniforms.colorRed.x * 10.0h);
    float2x2 mat = float2x2(float2(ten), float2(ten));
    float2x2 div = mat / (float2x2(1.0, 1.0, 1.0, 1.0) * _uniforms.testInputs.x);
    mat /= (float2x2(1.0, 1.0, 1.0, 1.0) * _uniforms.testInputs.x);
    return div == float2x2(float2(-8.0, -8.0), float2(-8.0, -8.0)) && mat == float2x2(float2(-8.0, -8.0), float2(-8.0, -8.0));
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
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
    _out.sk_FragColor = ((((((_2_m2[0].x == _0_expected[0].x && _2_m2[0].y == _0_expected[0].y) && _2_m2[1].x == _0_expected[1].x) && _2_m2[1].y == _0_expected[1].y) && test_bifffff22(_uniforms, minus, f1, f2, f3, f4, float2x2(float2(f1 - 1.0, f2 - 1.0), float2(f3 - 1.0, f4 - 1.0)))) && test_bifffff22(_uniforms, star, f1, f2, f3, f4, float2x2(float2(f1 * 2.0, f2 * 2.0), float2(f3 * 2.0, f4 * 2.0)))) && test_bifffff22(_uniforms, slash, f1, f2, f3, f4, float2x2(float2(f1 * 0.5, f2 * 0.5), float2(f3 * 0.5, f4 * 0.5)))) && divisionTest_b(_uniforms) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
