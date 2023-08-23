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
    _out.sk_FragColor = ((((((dfdy(_uniforms.testInputs.x)) == expected.x && all((dfdy(_uniforms.testInputs.xy)) == expected.xy)) && all((dfdy(_uniforms.testInputs.xyz)) == expected.xyz)) && all((dfdy(_uniforms.testInputs)) == expected)) && all(sign((dfdy(coords.xx))) == float2(0.0))) && all(sign((dfdy(coords.yy))) == float2(1.0))) && all(sign((dfdy(coords))) == float2(0.0, 1.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
