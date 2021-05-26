#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 inputA;
    float4 inputB;
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
    float4 expected = float4(5.0, 17.0, 38.0, 70.0);
    _out.sk_FragColor = (((((((_uniforms.inputA.x * _uniforms.inputB.x) == expected.x && dot(_uniforms.inputA.xy, _uniforms.inputB.xy) == expected.y) && dot(_uniforms.inputA.xyz, _uniforms.inputB.xyz) == expected.z) && dot(_uniforms.inputA, _uniforms.inputB) == expected.w) && 5.0 == expected.x) && 17.0 == expected.y) && 38.0 == expected.z) && 70.0 == expected.w ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
