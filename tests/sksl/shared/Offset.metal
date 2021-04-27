#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Test {
    int x;
    int y;
    int z;
};
thread bool operator==(thread const Test& left, thread const Test& right) {
    return (left.x == right.x) &&
           (left.y == right.y) &&
           (left.z == right.z);
}
thread bool operator!=(thread const Test& left, thread const Test& right) {
    return !(left == right);
}
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    Test t;
    t.x = 0;
    _out.sk_FragColor.x = float(t.x);
    return _out;
}
