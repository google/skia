#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 pos1;
    float4 pos2;
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
    _out.sk_FragColor = ((((((abs(_uniforms.pos1.x - _uniforms.pos2.x) == expected.x && distance(_uniforms.pos1.xy, _uniforms.pos2.xy) == expected.y) && distance(_uniforms.pos1.xyz, _uniforms.pos2.xyz) == expected.z) && distance(_uniforms.pos1, _uniforms.pos2) == expected.w) && 3.0 == expected.x) && 3.0 == expected.y) && 5.0 == expected.z) && 13.0 == expected.w ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
