#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float a;
    float b;
    float4 c;
    float4 d;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = atan(_uniforms.a);
    _out.sk_FragColor.x = atan2(_uniforms.a, _uniforms.b);
    _out.sk_FragColor = atan(_uniforms.c);
    _out.sk_FragColor = atan2(_uniforms.c, _uniforms.d);
    return _out;
}
