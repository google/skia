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
struct Globals {
    half2 glob;
};
half4 fn_h4hh2h2h3(thread Globals& _globals, half a, thread half2& b, thread half2& c, thread half3& d) {
    a = _out.sk_FragColor.x + a;
    b = _out.sk_FragColor.yz - _globals.glob.y;
    c *= a;
    d = _out.sk_FragColor.www / d;
    return half4(a, b.x, c.y, d.x);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{half2(1.0h)};
    (void)_globals;
    Outputs _out;
    (void)_out;
    half2 _skTemp0;
    half2 _skTemp1;
    half3 _skTemp2;
    half4 _skTemp3;
    half2 a = half2(1.0h);
    half3 b = half3(2.0h);
    half3x3 d = half3x3(4.0h);
    _out.sk_FragColor = ((_skTemp3 = fn_h4hh2h2h3(_globals, a.x, _skTemp0, (_skTemp1 = _globals.glob.yx), (_skTemp2 = d[1].zyx))), (b.yz = _skTemp0), (_globals.glob.yx = _skTemp1), (d[1].zyx = _skTemp2), _skTemp3);
    return _out;
}
