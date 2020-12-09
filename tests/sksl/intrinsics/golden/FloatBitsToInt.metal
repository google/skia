#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float a;
    float4 b;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
int _sk_bitcast_float_to_int(float x) { return as_type<int>(x); }
int4 _sk_bitcast_float4_to_int4(float4 x) { return as_type<int4>(x); }


fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    _out->sk_FragColor.x = float(_sk_bitcast_float_to_int(_in.a));
    _out->sk_FragColor = float4(_sk_bitcast_float4_to_int4(_in.b));
    return *_out;
}
