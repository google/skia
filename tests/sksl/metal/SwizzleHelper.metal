#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    float2 glob;
};
float4 fn(thread Outputs& _out, thread Globals& _globals, float a, thread float2& b, thread float2& c, thread float3& d);
float4 _skOutParamHelper0_fn(thread Outputs& _out, thread Globals& _globals, float _var0, thread float3& b, thread float2& glob, thread float3x3& d) {
    float2 _var1;
    float2 _var2 = glob.yx;
    float3 _var3 = d[1].zyx;
    float4 _skResult = fn(_out, _globals, _var0, _var1, _var2, _var3);
    b.yz = _var1;
    glob.yx = _var2;
    d[1].zyx = _var3;
    return _skResult;
}
float4 fn(thread Outputs& _out, thread Globals& _globals, float a, thread float2& b, thread float2& c, thread float3& d) {
    a = _out.sk_FragColor.x + a;
    b = _out.sk_FragColor.yz - _globals.glob.y;
    c *= a;
    d = _out.sk_FragColor.www / d;
    return float4(a, b.x, c.y, d.x);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _globals{float2(1.0)};
    (void)_globals;
    Outputs _out;
    (void)_out;
    float2 a = float2(1.0);
    float3 b = float3(2.0);
    float3x3 d = float3x3(4.0);
    _out.sk_FragColor =     _skOutParamHelper0_fn(_out, _globals, a.x, b, _globals.glob, d);
    return _out;
}
