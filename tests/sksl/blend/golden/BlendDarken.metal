#include <metal_stdlib>
#include <simd/simd.h>
using namespace metal;
struct Inputs {
    float4 src;
    float4 dst;
};
struct Outputs {
    float4 sk_FragColor [[color(0)]];
};


float4 blend_src_over(float4 src, float4 dst) {
    return src + (1.0 - src.w) * dst;
}
float4 blend_darken(float4 src, float4 dst) {
    float4 _2_blend_src_over;
    {
        _2_blend_src_over = src + (1.0 - src.w) * dst;
    }
    float4 result = _2_blend_src_over;

    result.xyz = min(result.xyz, (1.0 - dst.w) * src.xyz + dst.xyz);
    return result;
}
fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float4 _0_blend_darken;
    {
        float4 _3_blend_src_over;
        {
            _3_blend_src_over = _in.src + (1.0 - _in.src.w) * _in.dst;
        }
        float4 _1_result = _3_blend_src_over;

        _1_result.xyz = min(_1_result.xyz, (1.0 - _in.dst.w) * _in.src.xyz + _in.dst.xyz);
        _0_blend_darken = _1_result;
    }

    _out->sk_FragColor = _0_blend_darken;

    return *_out;
}
