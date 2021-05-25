#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 input;
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
    float3 expected = float3(3.0, 5.0, 13.0);
    _out.sk_FragColor = ((((length(_uniforms.input.xy) == expected.x && length(_uniforms.input.xyz) == expected.y) && length(_uniforms.input) == expected.z) && 3.0 == expected.x) && 5.0 == expected.y) && 13.0 == expected.z ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
