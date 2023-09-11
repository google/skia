#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorWhite;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
bool IsEqual_bh4h4(half4 x, half4 y) {
    return all(x == y);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half4 colorBlue = half4(0.0h, 0.0h, _uniforms.colorWhite.zw);
    half4 colorGreen = half4(0.0h, _uniforms.colorWhite.y, 0.0h, _uniforms.colorWhite.w);
    half4 colorRed = half4(_uniforms.colorWhite.x, 0.0h, 0.0h, _uniforms.colorWhite.w);
    half4 result = !IsEqual_bh4h4(_uniforms.colorWhite, colorBlue) ? (IsEqual_bh4h4(colorGreen, colorRed) ? colorRed : colorGreen) : (!IsEqual_bh4h4(colorRed, colorGreen) ? colorBlue : _uniforms.colorWhite);
    _out.sk_FragColor = IsEqual_bh4h4(colorRed, colorBlue) ? _uniforms.colorWhite : (!IsEqual_bh4h4(colorRed, colorGreen) ? result : (IsEqual_bh4h4(colorRed, _uniforms.colorWhite) ? colorBlue : colorRed));
    return _out;
}
