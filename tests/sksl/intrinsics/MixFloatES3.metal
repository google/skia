#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 colorGreen;
    half4 colorRed;
    half4 colorBlack;
    half4 colorWhite;
    half4 testInputs;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool4 FTFT = bool4(_uniforms.colorGreen);
    bool4 TFTF = FTFT.wzyx;
    _out.sk_FragColor = ((((((select(_uniforms.colorBlack.x, _uniforms.colorWhite.x, FTFT.x) == _uniforms.colorBlack.x && all(select(_uniforms.colorBlack.xy, _uniforms.colorWhite.xy, FTFT.xy) == half2(_uniforms.colorBlack.x, 1.0h))) && all(select(_uniforms.colorBlack.xyz, _uniforms.colorWhite.xyz, FTFT.xyz) == half3(_uniforms.colorBlack.x, 1.0h, _uniforms.colorBlack.z))) && all(select(_uniforms.colorBlack, _uniforms.colorWhite, FTFT) == half4(_uniforms.colorBlack.x, 1.0h, _uniforms.colorBlack.z, 1.0h))) && select(_uniforms.colorWhite.x, _uniforms.testInputs.x, TFTF.x) == _uniforms.testInputs.x) && all(select(_uniforms.colorWhite.xy, _uniforms.testInputs.xy, TFTF.xy) == half2(_uniforms.testInputs.x, 1.0h))) && all(select(_uniforms.colorWhite.xyz, _uniforms.testInputs.xyz, TFTF.xyz) == half3(_uniforms.testInputs.x, 1.0h, _uniforms.testInputs.z))) && all(select(_uniforms.colorWhite, _uniforms.testInputs, TFTF) == half4(_uniforms.testInputs.x, 1.0h, _uniforms.testInputs.z, 1.0h)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
