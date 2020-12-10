#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    float3x3 a;
    float3x3 b;
    float4x4 c;
    float4x4 d;
};




fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals globalStruct{{}, {}, {}, {}};
    thread Globals* _globals = &globalStruct;
    (void)_globals;
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor.xyz = matrixCompMult(_globals->a, _globals->b)[0];
    _out->sk_FragColor = matrixCompMult(_globals->c, _globals->d)[0];
    return *_out;
}
