#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    half4 testInputs;
    half4 colorGreen;
    half4 colorRed;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    half4 expected = half4(-2.0h, 0.0h, 0.0h, 2.0h);
    _out.sk_FragColor = ((((((floor(_uniforms.testInputs.x) == expected.x && all(floor(_uniforms.testInputs.xy) == expected.xy)) && all(floor(_uniforms.testInputs.xyz) == expected.xyz)) && all(floor(_uniforms.testInputs) == expected)) && -2.0h == expected.x) && all(half2(-2.0h, 0.0h) == expected.xy)) && all(half3(-2.0h, 0.0h, 0.0h) == expected.xyz)) && all(half4(-2.0h, 0.0h, 0.0h, 2.0h) == expected) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
