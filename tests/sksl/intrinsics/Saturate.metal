#include <metal_stdlib>
#include <simd/simd.h>
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
    half4 expected = half4(0.0h, 0.0h, 0.75h, 1.0h);
    _out.sk_FragColor = ((((((saturate(_uniforms.testInputs.x) == expected.x && all(saturate(_uniforms.testInputs.xy) == expected.xy)) && all(saturate(_uniforms.testInputs.xyz) == expected.xyz)) && all(saturate(_uniforms.testInputs) == expected)) && 0.0h == expected.x) && all(half2(0.0h, 0.0h) == expected.xy)) && all(half3(0.0h, 0.0h, 0.75h) == expected.xyz)) && all(half4(0.0h, 0.0h, 0.75h, 1.0h) == expected) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
