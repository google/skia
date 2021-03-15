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
    float4 _skTemp2;
    float _skTemp3;
    float4 _skTemp4;
    float4 _skTemp5;
    _out.sk_FragColor.x = (_skTemp0 = _globals.a, _skTemp1 = _globals.b, _skTemp0 - _skTemp1 * floor(_skTemp0 / _skTemp1));
    _out.sk_FragColor = (_skTemp2 = _globals.c, _skTemp3 = _globals.b, _skTemp2 - _skTemp3 * floor(_skTemp2 / _skTemp3));
    _out.sk_FragColor = (_skTemp4 = _globals.c, _skTemp5 = _globals.d, _skTemp4 - _skTemp5 * floor(_skTemp4 / _skTemp5));
    return _out;
}
