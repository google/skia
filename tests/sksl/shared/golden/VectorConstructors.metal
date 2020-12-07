#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    float2 v1;
    float2 v2;
    float2 v3;
    float3 v4;
    int2 v5;
    int2 v6;
    float2 v7;
};







fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals globalStruct{float2(1.0), float2(1.0, 2.0), float2(1.0), float3(float2(1.0), 1.0), int2(1), int2(float2(1.0, 2.0)), float2(int2(1, 2))};
    thread Globals* _globals = &globalStruct;
    (void)_globals;
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor.x = (((((_globals->v1.x + _globals->v2.x) + _globals->v3.x) + _globals->v4.x) + float(_globals->v5.x)) + float(_globals->v6.x)) + _globals->v7.x;
    return *_out;
}
