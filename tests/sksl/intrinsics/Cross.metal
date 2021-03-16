#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float2 a;
    float2 b;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = _uniforms.a.x * _uniforms.b.y - _uniforms.a.y * _uniforms.b.x;
    return _out;
}
