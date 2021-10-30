#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    half4 inputA;
    half4 inputB;
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
    half4 expected = half4(5.0h, 17.0h, 38.0h, 70.0h);
    _out.sk_FragColor = (((((((_uniforms.inputA.x * _uniforms.inputB.x) == expected.x && dot(_uniforms.inputA.xy, _uniforms.inputB.xy) == expected.y) && dot(_uniforms.inputA.xyz, _uniforms.inputB.xyz) == expected.z) && dot(_uniforms.inputA, _uniforms.inputB) == expected.w) && 5.0h == expected.x) && 17.0h == expected.y) && 38.0h == expected.z) && 70.0h == expected.w ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
