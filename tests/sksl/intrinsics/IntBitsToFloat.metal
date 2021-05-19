#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    int a;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = as_type<float>(_uniforms.a);
    return _out;
}
