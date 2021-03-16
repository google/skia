#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Uniforms {
    float a;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    int b;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor.x = ldexp(_uniforms.a, _globals.b);
    return _out;
}
