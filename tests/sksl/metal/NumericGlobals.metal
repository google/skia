#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
struct Globals {
    half attr1;
    int attr2;
    float attr3;
    half4 attr4;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{}, 123, {}, half4(4.0h, 5.0h, 6.0h, 7.0h)};
    (void)_globals;
    Outputs _out;
    (void)_out;
    _out.sk_FragColor = half4(_globals.attr1, half(_globals.attr2), half(_globals.attr3), _globals.attr4.x);
    return _out;
}
