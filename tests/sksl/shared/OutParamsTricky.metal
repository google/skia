#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
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
    float4 result = float4(0.0, 1.0, 2.0, 3.0);
    result.xz.xy = float2(2.0, 0.0);

    result.yw = float2(3.0, 5.0);

    _out.sk_FragColor = all(result == float4(2.0, 3.0, 0.0, 5.0)) ? _uniforms.colorGreen : _uniforms.colorRed;
    return _out;
}
