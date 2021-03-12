#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float a;
    float b;
    float4 c;
    float4 d;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _out;
    (void)_out;
    float _skTemp0;
    float _skTemp1;
    float4 _skTemp2;
    float _skTemp3;
    float4 _skTemp4;
    float4 _skTemp5;
    _out.sk_FragColor.x = (_skTemp0 = _in.a, _skTemp1 = _in.b, _skTemp0 - _skTemp1 * floor(_skTemp0 / _skTemp1));
    _out.sk_FragColor = (_skTemp2 = _in.c, _skTemp3 = _in.b, _skTemp2 - _skTemp3 * floor(_skTemp2 / _skTemp3));
    _out.sk_FragColor = (_skTemp4 = _in.c, _skTemp5 = _in.d, _skTemp4 - _skTemp5 * floor(_skTemp4 / _skTemp5));
    return _out;
}
