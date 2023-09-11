#include <metal_stdlib>
#include <simd/simd.h>
#ifdef __clang__
#pragma clang diagnostic ignored "-Wall"
#endif
using namespace metal;
struct A {
    int x;
    int y;
};
struct B {
    float x;
    array<float, 2> y;
    A z;
};
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
struct Globals {
    A a1;
    B b1;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{}, {}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _globals.a1.x = 0;
    _globals.b1.x = 0.0;
    _out.sk_FragColor.x = half(_globals.a1.x) + half(_globals.b1.x);
    return _out;
}
