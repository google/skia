#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorRed;
    half4 colorGreen;
    half unknownInput;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
bool test_int_b(Uniforms _uniforms) {
    bool ok = true;
    int4 inputRed = int4(_uniforms.colorRed);
    int4 inputGreen = int4(_uniforms.colorGreen);
    int4 x = inputRed + 2;
    ok = ok && all(x == int4(3, 2, 2, 3));
    x = inputGreen.ywxz - 2;
    ok = ok && all(x == int4(-1, -1, -2, -2));
    x = inputRed + inputGreen.y;
    ok = ok && all(x == int4(2, 1, 1, 2));
    x.xyz = inputGreen.wyw * 9;
    ok = ok && all(x == int4(9, 9, 9, 2));
    x.xy = x.zw / 4;
    ok = ok && all(x == int4(2, 0, 9, 2));
    x = (inputRed * 5).yxwz;
    ok = ok && all(x == int4(0, 5, 5, 0));
    x = 2 + inputRed;
    ok = ok && all(x == int4(3, 2, 2, 3));
    x = 10 - inputGreen.ywxz;
    ok = ok && all(x == int4(9, 9, 10, 10));
    x = inputRed.x + inputGreen;
    ok = ok && all(x == int4(1, 2, 1, 2));
    x.xyz = 8 * inputGreen.wyw;
    ok = ok && all(x == int4(8, 8, 8, 2));
    x.xy = 36 / x.zw;
    ok = ok && all(x == int4(4, 18, 8, 2));
    x = (37 / x).yxwz;
    ok = ok && all(x == int4(2, 9, 18, 4));
    x += 2;
    x *= 2;
    x -= 4;
    x /= 2;
    ok = ok && all(x == int4(2, 9, 18, 4));
    x = x + 2;
    x = x * 2;
    x = x - 4;
    x = x / 2;
    ok = ok && all(x == int4(2, 9, 18, 4));
    return ok;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool _0_ok = true;
    half4 _1_inputRed = _uniforms.colorRed;
    half4 _2_inputGreen = _uniforms.colorGreen;
    half4 _3_x = _1_inputRed + 2.0h;
    _0_ok = _0_ok && all(_3_x == half4(3.0h, 2.0h, 2.0h, 3.0h));
    _3_x = _2_inputGreen.ywxz - 2.0h;
    _0_ok = _0_ok && all(_3_x == half4(-1.0h, -1.0h, -2.0h, -2.0h));
    _3_x = _1_inputRed + _2_inputGreen.y;
    _0_ok = _0_ok && all(_3_x == half4(2.0h, 1.0h, 1.0h, 2.0h));
    _3_x.xyz = _2_inputGreen.wyw * 9.0h;
    _0_ok = _0_ok && all(_3_x == half4(9.0h, 9.0h, 9.0h, 2.0h));
    _3_x.xy = _3_x.zw * 2.0h;
    _0_ok = _0_ok && all(_3_x == half4(18.0h, 4.0h, 9.0h, 2.0h));
    _3_x = (_1_inputRed * 5.0h).yxwz;
    _0_ok = _0_ok && all(_3_x == half4(0.0h, 5.0h, 5.0h, 0.0h));
    _3_x = 2.0h + _1_inputRed;
    _0_ok = _0_ok && all(_3_x == half4(3.0h, 2.0h, 2.0h, 3.0h));
    _3_x = 10.0h - _2_inputGreen.ywxz;
    _0_ok = _0_ok && all(_3_x == half4(9.0h, 9.0h, 10.0h, 10.0h));
    _3_x = _1_inputRed.x + _2_inputGreen;
    _0_ok = _0_ok && all(_3_x == half4(1.0h, 2.0h, 1.0h, 2.0h));
    _3_x.xyz = 8.0h * _2_inputGreen.wyw;
    _0_ok = _0_ok && all(_3_x == half4(8.0h, 8.0h, 8.0h, 2.0h));
    _3_x.xy = 32.0h / _3_x.zw;
    _0_ok = _0_ok && all(_3_x == half4(4.0h, 16.0h, 8.0h, 2.0h));
    _3_x = (32.0h / _3_x).yxwz;
    _0_ok = _0_ok && all(_3_x == half4(2.0h, 8.0h, 16.0h, 4.0h));
    _3_x += 2.0h;
    _3_x *= 2.0h;
    _3_x -= 4.0h;
    _3_x *= 0.5h;
    _0_ok = _0_ok && all(_3_x == half4(2.0h, 8.0h, 16.0h, 4.0h));
    _3_x = _3_x + 2.0h;
    _3_x = _3_x * 2.0h;
    _3_x = _3_x - 4.0h;
    _3_x = _3_x * 0.5h;
    _0_ok = _0_ok && all(_3_x == half4(2.0h, 8.0h, 16.0h, 4.0h));
    _out.sk_FragColor = _0_ok && test_int_b(_uniforms) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
