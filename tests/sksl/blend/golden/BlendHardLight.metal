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
    float4 _0_blend_hard_light;
    {
        float4 _1_8_blend_overlay;
        {
            float _2_9_1_blend_overlay_component;
            {
                _2_9_1_blend_overlay_component = 2.0 * _in.src.x <= _in.src.w ? (2.0 * _in.dst.x) * _in.src.x : _in.dst.w * _in.src.w - (2.0 * (_in.src.w - _in.src.x)) * (_in.dst.w - _in.dst.x);
            }
            float _3_76_blend_overlay_component;
            {
                _3_76_blend_overlay_component = 2.0 * _in.src.y <= _in.src.w ? (2.0 * _in.dst.y) * _in.src.y : _in.dst.w * _in.src.w - (2.0 * (_in.src.w - _in.src.y)) * (_in.dst.w - _in.dst.y);
            }
            float _4_80_blend_overlay_component;
            {
                _4_80_blend_overlay_component = 2.0 * _in.src.z <= _in.src.w ? (2.0 * _in.dst.z) * _in.src.z : _in.dst.w * _in.src.w - (2.0 * (_in.src.w - _in.src.z)) * (_in.dst.w - _in.dst.z);
            }
            float4 _5_10_result = float4(_2_9_1_blend_overlay_component, _3_76_blend_overlay_component, _4_80_blend_overlay_component, _in.dst.w + (1.0 - _in.dst.w) * _in.src.w);



            _5_10_result.xyz = _5_10_result.xyz + _in.src.xyz * (1.0 - _in.dst.w) + _in.dst.xyz * (1.0 - _in.src.w);
            _1_8_blend_overlay = _5_10_result;
        }
        _0_blend_hard_light = _1_8_blend_overlay;

    }
    _out->sk_FragColor = _0_blend_hard_light;

    return *_out;
}
