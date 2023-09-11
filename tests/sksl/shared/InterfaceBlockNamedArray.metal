#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
struct testBlock {
    float s;
    char pad0[12];
    float2x2 m;
    char pad1[16];
    array<float, 2> a;
    char pad2[24];
    array<float2x2, 2> am;
} test[2];
struct Globals {
    constant testBlock* test;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant array<testBlock, 2>& test [[buffer(123)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{&test};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = half4(half(_uniforms.test[0].s), half(_uniforms.test[1].m[1].x), half(_uniforms.test[0].a[1]), half(_uniforms.test[1].am[1][0].y));
    return _out;
}
