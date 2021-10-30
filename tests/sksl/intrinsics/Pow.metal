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
    half4 expected = half4(-1.5625h, 0.0h, 0.75h, 3.375h);
    const half4 exponents = half4(2.0h, 3.0h, 1.0h, 1.5h);
    _out.sk_FragColor = ((((((pow(_uniforms.testInputs.x, 2.0h) == expected.x && all(pow(_uniforms.testInputs.xy, half2(2.0h, 3.0h)) == expected.xy)) && all(pow(_uniforms.testInputs.xyz, half3(2.0h, 3.0h, 1.0h)) == expected.xyz)) && all(pow(_uniforms.testInputs, exponents) == expected)) && 1.5625h == expected.x) && all(half2(1.5625h, 0.0h) == expected.xy)) && all(half3(1.5625h, 0.0h, 0.75h) == expected.xyz)) && all(half4(1.5625h, 0.0h, 0.75h, 3.375h) == expected) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
