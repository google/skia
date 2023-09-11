#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct Uniforms {
    int a;
    uint b;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], constant Uniforms& _uniforms [[buffer(0)]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    int _skTemp0;
    uint _skTemp1;
    _out.sk_FragColor.x = half((_skTemp0 = (_uniforms.a), select(ctz(_skTemp0), int(-1), _skTemp0 == int(0))));
    _out.sk_FragColor.y = half((_skTemp1 = (_uniforms.b), select(ctz(_skTemp1), uint(-1), _skTemp1 == uint(0))));
    return _out;
}
