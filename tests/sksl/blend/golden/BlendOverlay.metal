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


fragment Outputs fragmentMain(Inputs _in [[stage_in]], bool _frontFacing [[front_facing]], float4 _fragCoord [[position]]) {
    Outputs _outputStruct;
    thread Outputs* _out = &_outputStruct;
    float4 _0_blend_overlay;
    {
        float _1_1_blend_overlay_component;
        {
            _1_1_blend_overlay_component = 2.0 * _in.dst.x <= _in.dst.w ? (2.0 * _in.src.x) * _in.dst.x : _in.src.w * _in.dst.w - (2.0 * (_in.dst.w - _in.dst.x)) * (_in.src.w - _in.src.x);
        }
        float _2_75_blend_overlay_component;
        {
            _2_75_blend_overlay_component = 2.0 * _in.dst.y <= _in.dst.w ? (2.0 * _in.src.y) * _in.dst.y : _in.src.w * _in.dst.w - (2.0 * (_in.dst.w - _in.dst.y)) * (_in.src.w - _in.src.y);
        }
        float _3_79_blend_overlay_component;
        {
            _3_79_blend_overlay_component = 2.0 * _in.dst.z <= _in.dst.w ? (2.0 * _in.src.z) * _in.dst.z : _in.src.w * _in.dst.w - (2.0 * (_in.dst.w - _in.dst.z)) * (_in.src.w - _in.src.z);
        }
        float4 _4_result = float4(_1_1_blend_overlay_component, _2_75_blend_overlay_component, _3_79_blend_overlay_component, _in.src.w + (1.0 - _in.src.w) * _in.dst.w);



        _4_result.xyz = _4_result.xyz + _in.dst.xyz * (1.0 - _in.src.w) + _in.src.xyz * (1.0 - _in.dst.w);
        _0_blend_overlay = _4_result;
    }
    _out->sk_FragColor = _0_blend_overlay;

    return *_out;
}
