#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    float a;
    float b;
    float4 c;
    float4 d;
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{{}, {}, {}, {}};
    (void)_globals;
    Outputs _out;
    (void)_out;
    float _skTemp0;
    float _skTemp1;
    _out.sk_FragColor.x = (_skTemp0 = _globals.a, _skTemp1 = _globals.b, _skTemp0 - 2 * _skTemp1 * _skTemp0 * _skTemp1);
    _out.sk_FragColor = reflect(_globals.c, _globals.d);
    return _out;
}
