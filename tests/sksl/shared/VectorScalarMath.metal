#include <metal_stdlib>
#include <simd/simd.h>
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
bool test_half_b(Uniforms _uniforms) {
    bool ok = true;
    half4 inputRed = _uniforms.colorRed;
    half4 inputGreen = _uniforms.colorGreen;
    half4 x = inputRed + 2.0h;
    ok = ok && all(x == half4(3.0h, 2.0h, 2.0h, 3.0h));
    x = inputGreen.ywxz - 2.0h;
    ok = ok && all(x == half4(-1.0h, -1.0h, -2.0h, -2.0h));
    x = inputRed + inputGreen.y;
    ok = ok && all(x == half4(2.0h, 1.0h, 1.0h, 2.0h));
    x.xyz = inputGreen.wyw * 9.0h;
    ok = ok && all(x == half4(9.0h, 9.0h, 9.0h, 2.0h));
    x.xy = x.zw / 0.5h;
    ok = ok && all(x == half4(18.0h, 4.0h, 9.0h, 2.0h));
    x = (inputRed * 5.0h).yxwz;
    ok = ok && all(x == half4(0.0h, 5.0h, 5.0h, 0.0h));
    x = 2.0h + inputRed;
    ok = ok && all(x == half4(3.0h, 2.0h, 2.0h, 3.0h));
    x = 10.0h - inputGreen.ywxz;
    ok = ok && all(x == half4(9.0h, 9.0h, 10.0h, 10.0h));
    x = inputRed.x + inputGreen;
    ok = ok && all(x == half4(1.0h, 2.0h, 1.0h, 2.0h));
    x.xyz = 8.0h * inputGreen.wyw;
    ok = ok && all(x == half4(8.0h, 8.0h, 8.0h, 2.0h));
    x.xy = 32.0h / x.zw;
    ok = ok && all(x == half4(4.0h, 16.0h, 8.0h, 2.0h));
    x = (32.0h / x).yxwz;
    ok = ok && all(x == half4(2.0h, 8.0h, 16.0h, 4.0h));
    x += 2.0h;
    x *= 2.0h;
    x -= 4.0h;
    x /= 2.0h;
    ok = ok && all(x == half4(2.0h, 8.0h, 16.0h, 4.0h));
    x = x + 2.0h;
    x = x * 2.0h;
    x = x - 4.0h;
    x = x / 2.0h;
    ok = ok && all(x == half4(2.0h, 8.0h, 16.0h, 4.0h));
    return ok;
}
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
    _out.sk_FragColor = test_half_b(_uniforms) && test_int_b(_uniforms) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
