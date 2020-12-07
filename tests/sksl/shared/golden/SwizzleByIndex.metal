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
    {
        float4 _0_v = float4(sqrt(1.0));
        float _1_x = _0_v.x;
        float _2_y = _0_v.y;
        float _3_z = _0_v.z;
        float _4_w = _0_v.w;
        _out->sk_FragColor = float4(_1_x, _2_y, _3_z, _4_w);
    }

    {
        _out->sk_FragColor = float4(2.0, 2.0, 2.0, 2.0);
    }

    return *_out;
}
