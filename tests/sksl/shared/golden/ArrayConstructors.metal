#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    float[] test1;
    float2[] test2;
    float4x4[] test3;
};


fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals globalStruct{float[](1.0, 2.0, 3.0, 4.0), float2[](float2(1.0, 2.0), float2(3.0, 4.0)), float4x4[]()};
    thread Globals* _globals = &globalStruct;
    (void)_globals;
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor.x = (_globals->test1[0] + _globals->test2[0].x) + _globals->test3[0][0][0];
    return *_out;
}
