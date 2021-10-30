#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    half4 sk_FragColor [[color(0)]];
};
struct Globals {
    half2 glob;
};
half4 fn_h4hh2h2h3(thread Outputs& _out, thread Globals& _globals, half a, thread half2& b, thread half2& c, thread half3& d);
half4 _skOutParamHelper0_fn_h4hh2h2h3(thread Outputs& _out, thread Globals& _globals, half _var0, thread half3& b, thread half2& glob, thread half3x3& d) {
    half2 _var1;
    half2 _var2 = glob.yx;
    half3 _var3 = d[1].zyx;
    half4 _skResult = fn_h4hh2h2h3(_out, _globals, _var0, _var1, _var2, _var3);
    b.yz = _var1;
    glob.yx = _var2;
    d[1].zyx = _var3;
    return _skResult;
}
half4 fn_h4hh2h2h3(thread Outputs& _out, thread Globals& _globals, half a, thread half2& b, thread half2& c, thread half3& d) {
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
    half2 a = half2(1.0h);
    half3 b = half3(2.0h);
    half3x3 d = half3x3(4.0h);
    _out.sk_FragColor =     _skOutParamHelper0_fn_h4hh2h2h3(_out, _globals, a.x, b, _globals.glob, d);
    return _out;
}
