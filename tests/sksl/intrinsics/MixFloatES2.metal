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
    half4 expectedBW = half4(0.5h, 0.5h, 0.5h, 1.0h);
    half4 expectedWT = half4(1.0h, 0.5h, 1.0h, 2.25h);
    _out.sk_FragColor = ((((((((((((((((((all(mix(_uniforms.colorGreen, _uniforms.colorRed, 0.0h) == half4(0.0h, 1.0h, 0.0h, 1.0h)) && all(mix(_uniforms.colorGreen, _uniforms.colorRed, 0.25h) == half4(0.25h, 0.75h, 0.0h, 1.0h))) && all(mix(_uniforms.colorGreen, _uniforms.colorRed, 0.75h) == half4(0.75h, 0.25h, 0.0h, 1.0h))) && all(mix(_uniforms.colorGreen, _uniforms.colorRed, 1.0h) == half4(1.0h, 0.0h, 0.0h, 1.0h))) && mix(_uniforms.colorBlack.x, _uniforms.colorWhite.x, 0.5h) == expectedBW.x) && all(mix(_uniforms.colorBlack.xy, _uniforms.colorWhite.xy, 0.5h) == expectedBW.xy)) && all(mix(_uniforms.colorBlack.xyz, _uniforms.colorWhite.xyz, 0.5h) == expectedBW.xyz)) && all(mix(_uniforms.colorBlack, _uniforms.colorWhite, 0.5h) == expectedBW)) && 0.5h == expectedBW.x) && all(half2(0.5h) == expectedBW.xy)) && all(half3(0.5h) == expectedBW.xyz)) && all(half4(0.5h, 0.5h, 0.5h, 1.0h) == expectedBW)) && mix(_uniforms.colorWhite.x, _uniforms.testInputs.x, 0.0h) == expectedWT.x) && all(mix(_uniforms.colorWhite.xy, _uniforms.testInputs.xy, half2(0.0h, 0.5h)) == expectedWT.xy)) && all(mix(_uniforms.colorWhite.xyz, _uniforms.testInputs.xyz, half3(0.0h, 0.5h, 0.0h)) == expectedWT.xyz)) && all(mix(_uniforms.colorWhite, _uniforms.testInputs, half4(0.0h, 0.5h, 0.0h, 1.0h)) == expectedWT)) && 1.0h == expectedWT.x) && all(half2(1.0h, 0.5h) == expectedWT.xy)) && all(half3(1.0h, 0.5h, 1.0h) == expectedWT.xyz)) && all(half4(1.0h, 0.5h, 1.0h, 2.25h) == expectedWT) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
