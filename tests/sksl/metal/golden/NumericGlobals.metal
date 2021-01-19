#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
struct Globals {
    float attr1;
    int attr2;
    float attr3;
    float4 attr4;
};




fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Globals _skGlobals{{}, 123, {}, float4(4.0, 5.0, 6.0, 7.0)};
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor = float4(_skGlobals.attr1, float(_skGlobals.attr2), _skGlobals.attr3, _skGlobals.attr4.x);
    return *_out;
}
