#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
    float2x2 testMatrix2x2;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};

thread bool operator==(const float2x2 left, const float2x2 right);
thread bool operator!=(const float2x2 left, const float2x2 right);
thread bool operator==(const float2x2 left, const float2x2 right) {
    return all(left[0] == right[0]) &&
           all(left[1] == right[1]);
}
thread bool operator!=(const float2x2 left, const float2x2 right) {
    return !(left == right);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool ok = true;
    int i = 5;
    ++i;
    ok = ok && i == 6;
    ok = ok && ++i == 7;
    ok = ok && --i == 6;
    --i;
    ok = ok && i == 5;
    float f = 0.5;
    ++f;
    ok = ok && f == 1.5;
    ok = ok && ++f == 2.5;
    ok = ok && --f == 1.5;
    --f;
    ok = ok && f == 0.5;
    float2 f2 = float2(0.5);
    ++f2.x;
    ok = ok && f2.x == 1.5;
    ok = ok && ++f2.x == 2.5;
    ok = ok && --f2.x == 1.5;
    --f2.x;
    ok = ok && f2.x == 0.5;
    ok = ok && _uniforms.colorGreen.x != 1.0h;
    ok = ok && -1.0h == -_uniforms.colorGreen.y;
    ok = ok && all(half4(0.0h, -1.0h, 0.0h, -1.0h) == -_uniforms.colorGreen);
    ok = ok && float2x2(float2(-1.0, -2.0), float2(-3.0, -4.0)) == (-1.0 * _uniforms.testMatrix2x2);
    int2 iv = int2(i, -i);
    ok = ok && -i == -5;
    ok = ok && all(-iv == int2(-5, 5));
    _out.sk_FragColor = ok ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
