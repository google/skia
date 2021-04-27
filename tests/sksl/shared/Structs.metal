#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct A {
    int x;
    int y;
};
thread bool operator==(thread const A& left, thread const A& right) {
    return (left.x == right.x) &&
           (left.y == right.y);
}
thread bool operator!=(thread const A& left, thread const A& right) {
    return !(left == right);
}
struct B {
    float x;
    array<float, 2> y;
    A z;
};
thread bool operator==(thread const B& left, thread const B& right) {
    return (left.x == right.x) &&
           (left.y == right.y) &&
           (left.z == right.z);
}
thread bool operator!=(thread const B& left, thread const B& right) {
    return !(left == right);
}
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
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
    _out.sk_FragColor.x = float(_globals.a1.x) + _globals.b1.x;
    return _out;
}
