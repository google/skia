#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float4 srcdst;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};

fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float4 _0_blend_dst_in;
    {
        float4 _2_blend_src_in;
        {
            _2_blend_src_in = _in.dst * _in.src.w;
        }
        _0_blend_dst_in = _2_blend_src_in;

    }

    _out->sk_FragColor = _0_blend_dst_in;

    return *_out;
}
