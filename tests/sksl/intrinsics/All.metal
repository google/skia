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
    bool4 inputVal = bool4(_uniforms.inputH4);
    bool4 expected = bool4(_uniforms.expectedH4);
    _out.sk_FragColor = ((((all(inputVal.xy) == expected.x && all(inputVal.xyz) == expected.y) && all(inputVal) == expected.z) && expected.x) && false == expected.y) && false == expected.z ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
