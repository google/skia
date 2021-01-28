#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 minus1234;
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
    _out.sk_FragColor = ((abs(_uniforms.minus1234.x) == 1.0 && all(abs(_uniforms.minus1234.xy) == float2(1.0, 2.0))) && all(abs(_uniforms.minus1234.xyz) == float3(1.0, 2.0, 3.0))) && all(abs(_uniforms.minus1234) == float4(1.0, 2.0, 3.0, 4.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
