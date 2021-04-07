#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct A {
    int x;
    float y;
};
struct B {
    float x;
    array<float, 2> y;
    A z;
};
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    A a1;
    A a4;
    B b1;
    B b4;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{}, A{1, 2.0}, {}, B{1.0, array<float, 2>{2.0, 3.0}, A{4, 5.0}}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _globals.a1.x = 0;
    _globals.b1.x = 0.0;
    _out.sk_FragColor.x = (((float(_globals.a1.x) + _globals.b1.x) + _globals.a4.y) + _globals.b4.x) + A{1, 2.0}.y;
    return _out;
}
