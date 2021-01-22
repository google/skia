#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct testBlock {
    float x;
} test;
struct Globals {
    constant testBlock* test;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant testBlock& test [[buffer(456)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{&test};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = float4(_uniforms.test.x);
    return _out;
}
