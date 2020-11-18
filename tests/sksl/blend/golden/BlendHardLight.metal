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
    float4 _3_blend_hard_light;
    {
        float4 _5_blend_overlay;
        {
            float _7_blend_overlay_component;
            {
                _7_blend_overlay_component = 2.0 * _in.src.x <= _in.src.w ? (2.0 * _in.dst.x) * _in.src.x : _in.dst.w * _in.src.w - (2.0 * (_in.src.w - _in.src.x)) * (_in.dst.w - _in.dst.x);
            }
            float _8_blend_overlay_component;
            {
                _8_blend_overlay_component = 2.0 * _in.src.y <= _in.src.w ? (2.0 * _in.dst.y) * _in.src.y : _in.dst.w * _in.src.w - (2.0 * (_in.src.w - _in.src.y)) * (_in.dst.w - _in.dst.y);
            }
            float _9_blend_overlay_component;
            {
                _9_blend_overlay_component = 2.0 * _in.src.z <= _in.src.w ? (2.0 * _in.dst.z) * _in.src.z : _in.dst.w * _in.src.w - (2.0 * (_in.src.w - _in.src.z)) * (_in.dst.w - _in.dst.z);
            }
            float4 _6_result = float4(_7_blend_overlay_component, _8_blend_overlay_component, _9_blend_overlay_component, _in.dst.w + (1.0 - _in.dst.w) * _in.src.w);



            _6_result.xyz = _6_result.xyz + _in.src.xyz * (1.0 - _in.dst.w) + _in.dst.xyz * (1.0 - _in.src.w);
            _5_blend_overlay = _6_result;
        }
        _3_blend_hard_light = _5_blend_overlay;

    }
    _out->sk_FragColor = _3_blend_hard_light;

    return *_out;
}
