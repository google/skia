#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float2 a;
    float4 b;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = float(packHalf2x16(_uniforms.a));
    _out.sk_FragColor.x = float(packUnorm2x16(_uniforms.a));
    _out.sk_FragColor.x = float(packSnorm2x16(_uniforms.a));
    _out.sk_FragColor.x = float(packUnorm4x8(_uniforms.b));
    _out.sk_FragColor.x = float(packSnorm4x8(_uniforms.b));
    return _out;
}
