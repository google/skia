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
    half4 expected = half4(0.0h);
    _out.sk_FragColor = (((((((dfdx(_uniforms.testInputs.x) == expected.x && all(dfdx(_uniforms.testInputs.xy) == expected.xy)) && all(dfdx(_uniforms.testInputs.xyz) == expected.xyz)) && all(dfdx(_uniforms.testInputs) == expected)) && all(sign(fwidth(coords.xx)) == float2(1.0))) && all(sign(fwidth(float2(coords.x, 1.0))) == float2(1.0, 0.0))) && all(sign(fwidth(coords.yy)) == float2(1.0))) && all(sign(fwidth(float2(0.0, coords.y))) == float2(0.0, 1.0))) && all(sign(fwidth(coords)) == float2(1.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
