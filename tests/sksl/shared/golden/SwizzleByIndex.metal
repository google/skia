#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    int4 _0_i = int4(int(sqrt(1.0)));
    float4 _1_v = float4(sqrt(1.0));
    float _2_x = _1_v[_0_i.x];
    float _3_y = _1_v[_0_i.y];
    float _4_z = _1_v[_0_i.z];
    float _5_w = _1_v[_0_i.w];
    _out->sk_FragColor = float4(_2_x, _3_y, _4_z, _5_w);

    float4 _6_v = float4(sqrt(1.0));
    float _7_x = _6_v.x;
    float _8_y = _6_v.y;
    float _9_z = _6_v.z;
    float _10_w = _6_v.w;
    _out->sk_FragColor = float4(_7_x, _8_y, _9_z, _10_w);

    _out->sk_FragColor = float4(2.0, 2.0, 2.0, 2.0);

    return *_out;
}
