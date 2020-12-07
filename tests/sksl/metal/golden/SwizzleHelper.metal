#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    float glob;
};
float4 fn(thread Outputs* _out, thread Globals* _globals, float a, thread float2& b, float2 c, thread float3& d);
float4 _skOutParamHelper0_fn(thread Outputs* _out, thread Globals* _globals, float var0, thread float3& b, float2 var2, thread float3x3& d) {
    float2 var1 = b.yz;
    float3 var3 = d[1].zyx;
    float4 _skResult = fn(_out, _globals, var0, var1, var2, var3);
    b.yz = var1;
    d[1].zyx = var3;
    return _skResult;
}

float4 fn(thread Outputs* _out, thread Globals* _globals, float a, thread float2& b, float2 c, thread float3& d) {
    a = _out->sk_FragColor.x + a;
    b = _out->sk_FragColor.yz - _globals->glob;
    c = _out->sk_FragColor.zz * c;
    d = _out->sk_FragColor.www / d;
    return float4(a, b.x, c.x, d.x);
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals globalStruct{1.0};
    thread Globals* _globals = &globalStruct;
    (void)_globals;
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float3 b = float3(2.0);
    float3x3 d = float3x3(4.0);
    _out->sk_FragColor =     _skOutParamHelper0_fn(_out, _globals, 1.0, b, float4x4(3.0)[0].zx, d);
    return *_out;
}
