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
    float x = 1.0;
    float y = 2.0;

    int z = 3;
    x = 2.0;
    y = 0.5;
    z = 8;
    x = 14.0;
    x = 2.0;
    x = 2.0 * (y = 0.05000000074505806);
    z = 8;
    z = 8;
    z = 8;
    z = 2;
    z = 32;
    z = 2;
    x = 6.0;
    y = 6.0;
    z = 6;
    _out.sk_FragColor = _uniforms.colorGreen;
    return _out;
}
