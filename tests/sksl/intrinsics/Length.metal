#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 inputVal;
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
    float4 expected = float4(3.0, 3.0, 5.0, 13.0);
    _out.sk_FragColor = ((((((abs(_uniforms.inputVal.x) == expected.x && length(_uniforms.inputVal.xy) == expected.y) && length(_uniforms.inputVal.xyz) == expected.z) && length(_uniforms.inputVal) == expected.w) && 3.0 == expected.x) && 3.0 == expected.y) && 5.0 == expected.z) && 13.0 == expected.w ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
