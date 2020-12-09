#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    int a;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
float _sk_bitcast_int_to_float(int x) { return as_type<float>(x); }

fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor.x = _sk_bitcast_int_to_float(_in.a);
    return *_out;
}
