#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float myHalf;
    float4 myHalf4;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = _uniforms.myHalf4 * _uniforms.myHalf;
    return _out;
}
