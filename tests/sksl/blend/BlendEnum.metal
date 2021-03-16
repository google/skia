#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float4 src;
    float4 dst;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float4 _0_blend;
    for (int _1_loop = 0;_1_loop < 1; _1_loop++) {
        {
            {
                _0_blend = _uniforms.src * _uniforms.dst;
                continue;
            }
        }
    }
    _out.sk_FragColor = _0_blend;
    return _out;
}
