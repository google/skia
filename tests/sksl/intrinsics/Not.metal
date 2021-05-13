#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 inputH4;
    float4 expectedH4;
    float4 colorGreen;
    float4 colorRed;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    bool4 input = bool4(_uniforms.inputH4);
    bool4 expected = bool4(_uniforms.expectedH4);
    _out.sk_FragColor = ((((all(not(input.xy) == expected.xy) && all(not(input.xyz) == expected.xyz)) && all(not(input) == expected)) && all(bool2(false, true) == expected.xy)) && all(bool3(false, true, false) == expected.xyz)) && all(bool4(false, true, false, true) == expected) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
